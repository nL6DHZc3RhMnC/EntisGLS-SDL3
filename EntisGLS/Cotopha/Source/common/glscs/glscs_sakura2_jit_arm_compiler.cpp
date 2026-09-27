
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/


#include <sakuraglx/sakuraglx.h>
#include <sakura/ssys_module.h>
#include <glscs/glscs_sakura2_jit_arm_compiler.h>

using	namespace ECSSakura2JIT ;
using	namespace ECSSakura2Processor ;

#if	defined(__PROCESSOR_ARM__)

//////////////////////////////////////////////////////////////////////////////
// ARM コードバッファ出力オブジェクト (ARMv4 以降)
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ECSSakura2JIT::ARMCodeBuffer, CodeBuffer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ARMCodeBuffer::ARMCodeBuffer( void )
{
	m_bytesBlockUnit = 0x800 ;
	m_pxmChain = NULL ;
	m_armVersion = 4 ;
	m_modeThumb = false ;

}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ARMCodeBuffer::~ARMCodeBuffer( void )
{
	ExecuatbleMemory *	pxmNext = m_pxmChain ;
	while ( pxmNext != NULL )
	{
		ExecuatbleMemory *	pxmMem = pxmNext ;
		pxmNext = pxmNext->pNext ;
		//
		CodeBuffer::FreeCodeBuffer( pxmMem->pbytBuf, pxmMem->nBufBytes ) ;
	}
	m_pxmChain = NULL ;
}

// 次の命令を出力アドレスを取得する
//////////////////////////////////////////////////////////////////////////////
void * ARMCodeBuffer::GetNext( void )
{
	void *	ptrCode = CodeBuffer::GetNext() ;
	if ( m_modeThumb )
	{
		ptrCode = (void*) (((ulong_ptr_t)ptrCode) | 0x01) ;
	}
	return	ptrCode ;
}

// データを確定する
//////////////////////////////////////////////////////////////////////////////
void ARMCodeBuffer::CommitAllCodes( void )
{
	ExecuatbleMemory *	pxmMem = m_pxmChain ;
	while ( pxmMem != NULL )
	{
		CodeBuffer::FlushCodeBlock( pxmMem->pbytBuf, pxmMem->nBufBytes ) ;
		pxmMem = pxmMem->pNext ;
	}
	//
	// キャッシュの内容をメモリに書き出すために大量のメモリへアクセスする
	// ※cacheflush 関数などによるキャッシュフラッシュ動作の保険のため
	//
	using namespace SSystem ;
	SArray<uint8_t>	bufTemp1, bufTemp2 ;
	size_t	bytesCache = 0x400000 ;			// 4MB
	for ( ; ; )
	{
		bufTemp1.SetLength( bytesCache ) ;
		bufTemp2.SetLength( bytesCache ) ;
		if ( bufTemp1.GetConstArray() && bufTemp2.GetConstArray() )
		{
			break ;
		}
		bufTemp1.FreeArray() ;
		bufTemp2.FreeArray() ;
		bytesCache >>= 1 ;
	}
	for ( int i = 0; i < 2; i ++ )
	{
		bufTemp1.SetLength( bytesCache ) ;
		bufTemp2.SetLength( bytesCache ) ;
		memset( bufTemp1.GetArray(), 1, bytesCache ) ;
		memset( bufTemp2.GetArray(), 2, bytesCache ) ;
		memmove( bufTemp1.GetArray(), bufTemp2.GetArray(), bytesCache ) ;
		memmove( bufTemp2.GetArray(), bufTemp1.GetArray(), bytesCache ) ;
		bufTemp1.FinishArray() ;
		bufTemp2.FinishArray() ;
		bufTemp1.FreeArray() ;
		bufTemp2.FreeArray() ;
	}
}

// ARM 命令選択
//////////////////////////////////////////////////////////////////////////////
void ARMCodeBuffer::SelectARMInstruction( int armVersion, bool modeThumb )
{
	m_armVersion = armVersion ;
	m_modeThumb = modeThumb ;
	ESLAssert( m_armVersion >= 4 ) ;
	ESLAssert( !m_modeThumb || (m_armVersion >= 6) ) ;
}

// 指定バイト数分は最低限度分割されないことを保証する
//////////////////////////////////////////////////////////////////////////////
void ARMCodeBuffer::PreserveContinuousCodes( size_t nBytes )
{
	if ( m_pLastBlock->nBufSize
			- (m_pLastBlock->nCodeUsed
				+ m_pLastBlock->nDataUsed) <= m_maxInstructionSize + nBytes )
	{
		ExtendBlock() ;
	}
}

// 実行・読み書き可能メモリの確保
//////////////////////////////////////////////////////////////////////////////
BYTE * ARMCodeBuffer::AllocateCodeBuffer( size_t& nSize )
{
	ExecuatbleMemory *	pxmMem = m_pxmChain ;
	if ( (pxmMem == NULL)
		|| (pxmMem->nUsedBytes + nSize > pxmMem->nBufBytes) )
	{
		m_pxmChain = new ExecuatbleMemory ;
		m_pxmChain->pNext = pxmMem ;
		pxmMem = m_pxmChain ;
		//
		pxmMem->nBufBytes = 0x10000 ;
		pxmMem->nUsedBytes = 0 ;
		//
		pxmMem->pbytBuf =
			CodeBuffer::AllocateCodeBuffer( pxmMem->nBufBytes ) ;
	}
	BYTE *	pbytAlloc = pxmMem->pbytBuf + pxmMem->nUsedBytes ;
	pxmMem->nUsedBytes += nSize ;
	return	pbytAlloc ;
}

// 実行・読み書き可能メモリの解放
//////////////////////////////////////////////////////////////////////////////
void ARMCodeBuffer::FreeCodeBuffer( BYTE * pbytBuf, size_t nSize )
{
}

// ジャンプ命令を追加する
//////////////////////////////////////////////////////////////////////////////
void ARMCodeBuffer::WriteJump( Block * pBlock, const void * pTarget )
{
	BYTE *		pbytCode = pBlock->pbytBufAligned16 + pBlock->nCodeUsed ;
	long_ptr_t	pcCodeBase = (long_ptr_t) pbytCode ;
	if ( m_modeThumb )
	{
		uint16_t *	pnCode = (uint16_t*) pbytCode ;
		long_ptr_t	nRel11 =
			((long_ptr_t) pTarget - (pcCodeBase + 4)) >> 1 ;
		if ( (-(long_ptr_t)(1 << 10) <= nRel11) && (nRel11 < (1 << 10)) )
		{
			// B rel11  (ARMv4T)
			pnCode[0] = 0xE000 | ((uint16_t) nRel11 & 0x07FF) ;
			pBlock->nCodeUsed += 2 ;
			ESLAssert( pBlock->nCodeUsed + pBlock->nDataUsed <= pBlock->nBufSize ) ;
			return ;
		}
		long_ptr_t	nRel24 =
			((long_ptr_t) pTarget - (pcCodeBase + 4)) >> 1 ;
		if ( (m_armVersion >= 6)
			&& (-(long_ptr_t)(1 << 23) <= nRel11) && (nRel11 < (1 << 23)) )
		{
			// B rel24  (ARMv6T2)
			uint16_t	imm11 = (uint16_t) nRel24 & 0x07FF ;
			uint16_t	imm10 = (uint16_t) (nRel24 >> 1) & 0x03FF ;
			uint16_t	s = (uint16_t) (nRel24 >> 23) & 0x01 ;
			uint16_t	i2 = (uint16_t) (nRel24 >> 21) & 0x01 ;
			uint16_t	i1 = (uint16_t) (nRel24 >> 22) & 0x01 ;
			uint16_t	j2 = (i2 ^ 0x01) ^ s ;
			uint16_t	j1 = (i1 ^ 0x01) ^ s ;
			pnCode[0] = 0xF000 | (s << 10) | imm10 ;
			pnCode[1] = 0x9000 | (j1 << 13) | (j2 << 11) | imm11 ;
			pBlock->nCodeUsed += 4 ;
			ESLAssert( pBlock->nCodeUsed + pBlock->nDataUsed <= pBlock->nBufSize ) ;
			return ;
		}
		long_ptr_t	lptTarget = (long_ptr_t) pTarget ;
		lptTarget |= 0x0001 ;
		if ( m_armVersion >= 6 )
		{
			uint16_t	i1, imm4, imm3, imm8 ;
			uint16_t	plTarget = (uint16_t) lptTarget ;
			uint16_t	phTarget = (uint16_t) (lptTarget >> 16) ;
			//
			// MOVW r6, imm16  (ARMv6T2)
			imm8 = plTarget & 0xFF ;
			imm3 = (plTarget >> 4) & 0x07 ;
			i1 = (plTarget >> 11) & 0x01 ;
			imm4 = (plTarget >> 12) & 0x0F ;
			pnCode[0] = 0xF240 | (i1 << 10) | imm4 ;
			pnCode[1] = (imm3 << 12) | (ARMGenericAssembler::ARM_r6 << 8) | imm8 ;
			//
			// MOVT r6, imm16  (ARMv6T2)
			imm8 = phTarget & 0xFF ;
			imm3 = (phTarget >> 4) & 0x07 ;
			i1 = (phTarget >> 11) & 0x01 ;
			imm4 = (phTarget >> 12) & 0x0F ;
			pnCode[2] = 0xF2C0 | (i1 << 10) | imm4 ;
			pnCode[3] = (imm3 << 12) | (ARMGenericAssembler::ARM_r6 << 8) | imm8 ;
			//
			pnCode += 4 ;
			pBlock->nCodeUsed += 8 ;
		}
		else
		{
			pBlock->nDataUsed = (pBlock->nDataUsed + 4 + 3) & ~0x03 ;
			uint32_t *	pData =
				(uint32_t*) (pBlock->pbytBufAligned16
							+ (pBlock->nBufSize - pBlock->nDataUsed)) ;
			*pData = (uint32_t) lptTarget ;
			//
			// LDR r6, [PC+rel8*4]  (ARMv4T)
			uint32_t	imm8 = ((uint32_t) ((ulong_ptr_t) pData)
					- ((uint32_t) ((ulong_ptr_t) (pnCode + 1)) & ~0x03)) >> 2 ;
			ESLAssert( imm8 < 0x100 ) ;
			pnCode[0] = 0x4800 | (ARMGenericAssembler::ARM_r6 << 8)
											| (uint16_t) (imm8 & 0xFF) ;
			//
			pnCode += 1 ;
			pBlock->nCodeUsed += 2 ;
		}
		//
		// BX r6  (ARMv4T)
		pnCode[0] = 0x4700 | (ARMGenericAssembler::ARM_r6 << 3) ;
		pBlock->nCodeUsed += 2 ;
	}
	else
	{
		uint32_t *	pnCode = (uint32_t*) pbytCode ;
		long_ptr_t	nRel24 =
			((long_ptr_t) pTarget - (pcCodeBase + 8)) >> 2 ;
		if ( (-(long_ptr_t)(1 << 23) <= nRel24) && (nRel24 < (1 << 23)) )
		{
			// B rel24  (ARMv4*)
			*pnCode = 0xEA000000 | ((uint32_t) nRel24 & 0x00FFFFFF) ;
			pBlock->nCodeUsed += 4 ;
			ESLAssert( pBlock->nCodeUsed + pBlock->nDataUsed <= pBlock->nBufSize ) ;
			return ;
		}
		long_ptr_t	lptTarget = (long_ptr_t) pTarget ;
		ESLAssert( !(lptTarget & 0x03) ) ;
		if ( m_armVersion >= 6 )
		{
			uint32_t	imm4, imm12 ;
			uint32_t	plTarget = (uint32_t) (lptTarget & 0xFFFF) ;
			uint32_t	phTarget = (uint32_t) ((lptTarget >> 16) & 0xFFFF) ;
			//
			// MOVW r6, imm16  (ARMv6T2)
			imm12 = plTarget & 0x3FF ;
			imm4 = (plTarget >> 12) & 0x0F ;
			pnCode[0] = 0xE3000000 | (imm4 << 16) | (ARMGenericAssembler::ARM_r6 << 12) | imm12 ;
			//
			// MOVT r6, imm16  (ARMv6T2)
			imm12 = phTarget & 0x3FF ;
			imm4 = (phTarget >> 12) & 0x0F ;
			pnCode[1] = 0xE3400000 | (imm4 << 16) | (ARMGenericAssembler::ARM_r6 << 12) | imm12 ;
			//
			pnCode += 2 ;
			pBlock->nCodeUsed += 8 ;
		}
		else
		{
			pBlock->nDataUsed = (pBlock->nDataUsed + 4 + 3) & ~0x03 ;
			uint32_t *	pData =
				(uint32_t*) (pBlock->pbytBufAligned16
							+ (pBlock->nBufSize - pBlock->nDataUsed)) ;
			*pData = (uint32_t) lptTarget ;
			//
			// LDR r6, [PC+rel12]  (ARMv4T)
			uint32_t	imm12 = (uint32_t) ((ulong_ptr_t) pData)
							- ((uint32_t) ((ulong_ptr_t) (pnCode + 2)) & ~0x03) ;
			ESLAssert( imm12 < 0x1000 ) ;
			pnCode[0] = 0xE59F0000 | (ARMGenericAssembler::ARM_r6 << 12)
														| (imm12 & 0x0FFF) ;
			//
			pnCode += 1 ;
			pBlock->nCodeUsed += 4 ;
		}
		//
		// BX r6  (ARMv4T)
		pnCode[0] = 0xE12FFF10 | ARMGenericAssembler::ARM_r6 ;
		pBlock->nCodeUsed += 4 ;
	}
	ESLAssert( pBlock->nCodeUsed + pBlock->nDataUsed <= pBlock->nBufSize ) ;
}

// CPU キャッシュサイズを取得する（目安）
//////////////////////////////////////////////////////////////////////////////
size_t	ARMCodeBuffer::m_timesEstimatedCPUDataCache = 0 ;
size_t	ARMCodeBuffer::m_bytesEstimatedCPUDataCache = 0 ;

size_t ARMCodeBuffer::GetCPUDataCacheSize( bool fForceEstimate )
{
	if ( !fForceEstimate && (m_timesEstimatedCPUDataCache >= 2) )
	{
		return	m_bytesEstimatedCPUDataCache ;
	}
	size_t	nBytes = EstimateCPUDataCacheSize() ;
	SSystem::Trace( "CPU cache size may be %d[KB]\n", nBytes / 1024 ) ;
	if ( m_timesEstimatedCPUDataCache == 0 )
	{
		m_bytesEstimatedCPUDataCache = nBytes ;
		m_timesEstimatedCPUDataCache = 1 ;
	}
	else
	{
		m_bytesEstimatedCPUDataCache =
			(m_bytesEstimatedCPUDataCache + nBytes) >> 1 ;
		m_timesEstimatedCPUDataCache ++ ;
	}
	return	m_bytesEstimatedCPUDataCache ;
}

size_t ARMCodeBuffer::EstimateCPUDataCacheSize( void )
{
	//
	// キャッシュが効くと思われる小さなメモリ操作を繰り返し
	// キャッシュありの場合の速度を計測する
	//
	using namespace SSystem ;
	int64_t			msecStart, msecPastCached ;
	SArray<uint8_t>	buf1024 ;
	SArray<uint8_t>	bufTemp ;
	size_t			i, j, countBase = 0 ;
	buf1024.SetLength( 1024 ) ;
	uint8_t *	pbyt1024 = buf1024.GetArray() ;
	for ( i = 0; i < 1024; i ++ )
	{
		pbyt1024[i] = i ;
	}
	for ( i = 0, j = 0; i < 1024; i += 16 )
	{
		j += pbyt1024[i] ;
	}
	buf1024.FinishArray() ;
	Trace( "start EstimateCPUDataCacheSize\n", j ) ;
	//
	msecStart = CurrentMilliSec() ;
	for ( i = 0; i < 1000; i ++ )
	{
		for ( j = 0; j < 128; j ++ )
		{
			memset( pbyt1024, 0xFF, 1024 ) ;
		}
		msecPastCached = CurrentMilliSec() - msecStart ;
		countBase ++ ;
		if ( msecPastCached >= 20 )
		{
			break ;
		}
	}
	Trace( "%d[KB] / %d[ms]\n", countBase * 128, (int) msecPastCached ) ;
	//
	// メモリサイズを大きくしていき
	// 極端に速度が遅くなるサイズをキャッシュサイズと見なす
	// 但し正確なキャッシュサイズが知りたい訳ではないので
	// 最小キャッシュサイズを 1MB とし、それ以下はテストしない
	//
	size_t	bytesCache = 0x200000 ;		// 2MB
	size_t	bytesMaxCache = 0x2000000 ;	// 32MB
	MEMORY_STATUS	mstatus ;
	GetMemoryStatus( mstatus ) ;
	if ( mstatus.nAvailPhys < bytesMaxCache )
	{
		bytesMaxCache = (size_t) mstatus.nAvailPhys >> 1 ;
		if ( mstatus.nAvailPhys < 0x400000 )
		{
			bytesMaxCache = 0x400000 ;
		}
	}
	while ( bytesCache < bytesMaxCache )
	{
		SArray<uint8_t>	bufTemp ;
		bufTemp.SetLength( bytesCache ) ;
		uint8_t *	pbytTemp = bufTemp.GetArray() ;
		if ( pbytTemp == NULL )
		{
			break ;
		}
		size_t	countInterLoop = countBase * 128 / (bytesCache / 1024) ;
		if ( countInterLoop == 0 )
		{
			countInterLoop = 1 ;
		}
		int64_t	msecPast = 0 ;
		size_t	countLoop = 0 ;
		memset( pbytTemp, 0, bytesCache ) ;
		msecStart = CurrentMilliSec() ;
		for ( i = 0; i < 2048; i ++ )
		{
			for ( j = 0; j < countInterLoop; j ++ )
			{
				memset( pbytTemp, 0xFF, bytesCache ) ;
			}
			msecPast = CurrentMilliSec() - msecStart ;
			countLoop ++ ;
			if ( msecPast >= 20 )
			{
				break ;
			}
		}
		bufTemp.FinishArray() ;
		//
		size_t	kbytesBase = countBase * 128 ;
		size_t	kbytesTest =
					countLoop * countInterLoop * (bytesCache / 1024) ;
		Trace( "%d[KB] block: %d[KB] / %d[ms]\n",
				(bytesCache / 1024), kbytesTest, (int) msecPast ) ;
		if ( kbytesTest * 2 * msecPastCached < kbytesBase * msecPast )
		{
			bytesCache >>= 1 ;
			break ;
		}
		bytesCache <<= 1 ;
	}
	return	bytesCache ;
}


//////////////////////////////////////////////////////////////////////////////
// データレジスタ・割り当てコンテキスト
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ARMGenericAssembler::DataRegisterContext::DataRegisterContext( void )
{
	lruRegARM[0].regPhy = ARM_r0 ;
	lruRegARM[1].regPhy = ARM_r2 ;
	lruRegARM[2].regPhy = ARM_r4 ;
	//
	int	i ;
	for ( i = 0; i < lruRegVFP.SlotCount; i ++ )
	{
		lruRegVFP[i].regPhy = VFP_d0 + i ;
	}
	for ( i = 0; i < lruRegNEON.SlotCount; i ++ )
	{
		lruRegNEON[i].regPhy = VFP_q8 + i ;
	}
	for ( i = 0; i < 0x100; i ++ )
	{
		dprSakura[i].regClass = regClassNothing ;
		dprSakura[i].regPhy = -1 ;
	}
}

// 代入
//////////////////////////////////////////////////////////////////////////////
const ARMGenericAssembler::DataRegisterContext&
	ARMGenericAssembler::DataRegisterContext::operator =
			( const ARMGenericAssembler::DataRegisterContext& drc )
{
	int	i ;
	for ( i = 0; i < lruRegARM.SlotCount; i ++ )
	{
		lruRegARM[i] = drc.lruRegARM[i] ;
	}
	for ( i = 0; i < lruRegVFP.SlotCount; i ++ )
	{
		lruRegVFP[i] = drc.lruRegVFP[i] ;
	}
	for ( i = 0; i < lruRegNEON.SlotCount; i ++ )
	{
		lruRegNEON[i] = drc.lruRegNEON[i] ;
	}
	for ( i = 0; i < 0x100; i ++ )
	{
		dprSakura[i] = drc.dprSakura[i] ;
	}
	return	*this ;
}

// 現在の割り当てレジスタ取得
//////////////////////////////////////////////////////////////////////////////
bool ARMGenericAssembler::DataRegisterContext::GetLoadedPhysicalRegister
	( ARMGenericAssembler::DataPhysicalRegister& dpr, int regSakura ) const
{
	dpr = dprSakura[regSakura & 0xFF] ;
	return	(dpr.regClass != regClassNothing) & (dpr.regPhy != -1) ;
}



//////////////////////////////////////////////////////////////////////////////
// ARM ネイティブコード化アセンブラ (ARMv4 以降 / thumb では ARMv6以降)
//////////////////////////////////////////////////////////////////////////////

// ARM 命令選択
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::SelectARMInstruction
	( int armVersion, int vfpVersion, bool vfpNEON, bool modeThumb )
{
	ESLAssert( (armVersion >= 6) || ((armVersion >= 4) && !modeThumb) ) ;
	m_armVersion = armVersion ;
	m_vfpVersion = vfpVersion ;
	m_vfpNEON = vfpNEON ;
	m_modeThumb = modeThumb ;
}

// 指定バイト数分は最低限度分割されないことを保証する
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::PreserveContinuousCodes( size_t nBytes )
{
	ARMCodeBuffer *	buf = ESLTypeCast<ARMCodeBuffer>( m_buf ) ;
	buf->PreserveContinuousCodes( nBytes ) ;
}

#if	defined(__DEBUG__)
// デバッグ用（関数コンパイル開始時の処理）
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::OnDebugBeforeFunction( DWORD dwFuncAddr, DWORD dwSize )
{
	m_dwFuncAddr = dwFuncAddr ;
}

// デバッグ用（関数コンパイル終了時の処理）
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::OnDebugAfterFunction( DWORD dwFuncAddr, DWORD dwSize )
{
}

// デバッグ用（命令コンパイル前の処理）
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::OnDebugBeforeInstruction( BYTE bytInst, DWORD ip )
{
	m_pCurCode = (ulong_ptr_t) m_bufMain->GetNext() ;
	m_ipCurrent = ip ;
/*
	if ( (ip >= 0x98A03) && (ip <= 0x98ABA) )
	{
		SSystem::SString	strDump = L"ip=" ;
		strDump += SSystem::SString( ip, 8, 16 ) ;
		SSystem::SArray<char> *	pTemp = new SSystem::SArray<char> ;
		strDump.EncodeDefaultTo( *pTemp ) ;
		//
		TraceDebugString( pTemp->GetConstArray() ) ;
	}
*/
}

// デバッグ用（命令コンパイル後のテスト）
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::OnDebugAfterInstruction( BYTE bytInst, DWORD ip )
{
/*
	ulong_ptr_t	pNextCode = (ulong_ptr_t) m_bufMain->GetNext() ;
	if ( (ip >= 0x00111861) && (ip <= 0x00111881) )
	{
		SSystem::SString	strDump = L"ip=" ;
		strDump += SSystem::SString( ip, 8, 16 ) ;
		strDump += L"(" ;
		strDump += SSystem::SString( bytInst, 8, 16 ) ;
		strDump += L"):" ;
		strDump += SSystem::SString( m_pCurCode, 8, 16 ) ;
		strDump += L":" ;
		//
		DumpARMCode( strDump, m_pCurCode, pNextCode ) ;
		//
		SSystem::Trace( strDump.ToCharArray() ) ;
	}
*/
}

// メモリダンプ
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::DumpMemory32
	( SSystem::SString& strDump, ulong_ptr_t pFirst, ulong_ptr_t pEnd )
{
	while ( pFirst < pEnd )
	{
		const uint32_t *	pdwMem = (const uint32_t*) pFirst ;
		strDump += SSystem::SString( *pdwMem, 8, 16 ) ;
		strDump += L" " ;
		pFirst += 4 ;
	}
}

void ARMGenericAssembler::DumpARMCode
	( SSystem::SString& strDump, ulong_ptr_t pFirst, ulong_ptr_t pEnd )
{
	DumpMemory32( strDump, (pFirst & ~0x03), (pEnd & ~0x03) ) ;
}
#endif

// ldr reg, [reg+imm12]
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMLoadMemOffsetImm12
	( ARMRegister regDst, ARMRegister regBase,
		int offsetAddr, ECSSakura2Processor::DataType type )
{
	uint16_t	U = 1 ;
	if ( offsetAddr < 0 )
	{
		offsetAddr = - offsetAddr ;
		U = 0 ;
	}
	if ( m_modeThumb )
	{
		uint16_t	code[2] ;
		if ( U == 1 )
		{
			static const uint16_t	codeLDRimm12[] =
			{
				0xF8D0, 0xF8D0, 0xF9B0, 0xF990,
				0xF8D0, 0xF8D0, 0xF8B0, 0xF890,
			} ;
			// LDR<type> Rt, Rn, imm12  (ARMv6T2)
			ESLAssert( (uint32_t) offsetAddr < 0x1000 ) ;
			code[0] = codeLDRimm12[type] | (regBase) ;
			code[1] = (regDst << 12) | (offsetAddr & 0xFFF) ;
		}
		else
		{
			static const uint16_t	codeLDRimm8[] =
			{
				0xF850, 0xF850, 0xF930, 0xF910,
				0xF850, 0xF850, 0xF830, 0xF810,
			} ;
			// LDR<type> Rt, Rn, -imm8  (ARMv6T2)
			ESLAssert( (uint32_t) offsetAddr < 0x100 ) ;
			code[0] = codeLDRimm8[type] | (regBase) ;
			code[1] = 0x0C00 | (regDst << 12) | (offsetAddr & 0xFF) ;
		}
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		static const uint16_t		codeEncImm8[] =
		{
			0, 0, 0x00F0, 0x00D0,
			0, 0, 0x00B0, 0,
		} ;
		static const uint32_t	codeLDRimm12[] =
		{
			0xE5100000, 0xE5100000, 0xE1500000, 0xE1500000,
			0xE5100000, 0xE5100000, 0xE1500000, 0xE5500000,
		} ;
		// LDR<type> Rt, Rn, imm12|imm8  (ARMv4*)
		if ( codeEncImm8[type] != 0 )
		{
			ESLAssert( (uint32_t) offsetAddr < 0x100 ) ;
			offsetAddr = ((offsetAddr & 0xF0) << 4)
							| codeEncImm8[type] | (offsetAddr & 0x0F) ;
		}
		else
		{
			ESLAssert( (uint32_t) offsetAddr < 0x1000 ) ;
		}
		uint32_t	code[1] ;
		code[0] = codeLDRimm12[type] | ((uint32_t) U << 23)
				| (regBase << 16) | (regDst << 12) | (offsetAddr & 0x0FFF) ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// str [reg+imm12], reg
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMStoreMemOffsetImm12
	( ARMRegister regSrc, ARMRegister regBase,
		int offsetAddr, ECSSakura2Processor::DataType type )
{
	uint16_t	U = 1 ;
	if ( offsetAddr < 0 )
	{
		offsetAddr = - offsetAddr ;
		U = 0 ;
	}
	if ( m_modeThumb )
	{
		uint16_t	code[2] ;
		if ( U == 1 )
		{
			static const uint16_t	codeSTRimm12[] =
			{
				0xF8C0, 0xF8C0, 0xF8A0, 0xF880,
				0xF8C0, 0xF8C0, 0xF8A0, 0xF880,
			} ;
			// STR<type> Rt, Rn, imm12  (ARMv6T2)
			ESLAssert( (uint32_t) offsetAddr < 0x1000 ) ;
			code[0] = codeSTRimm12[type] | (regBase) ;
			code[1] = (regSrc << 12) | (offsetAddr & 0xFFF) ;
		}
		else
		{
			static const uint16_t	codeSTRimm12[] =
			{
				0xF840, 0xF840, 0xF820, 0xF800,
				0xF840, 0xF840, 0xF820, 0xF800,
			} ;
			// STR<type> Rt, Rn, -imm8  (ARMv6T2)
			ESLAssert( (uint32_t) offsetAddr < 0x100 ) ;
			code[0] = codeSTRimm12[type] | (regBase) ;
			code[1] = 0x0C00 | (regSrc << 12) | (offsetAddr & 0xFF) ;
		}
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		static const uint16_t		codeEncImm8[] =
		{
			0, 0, 0x00B0, 0,
			0, 0, 0x00B0, 0,
		} ;
		static const uint32_t	codeSTRimm12[] =
		{
			0xE5000000, 0xE5000000, 0xE1400000, 0xE5400000,
			0xE5000000, 0xE5000000, 0xE1400000, 0xE5400000,
		} ;
		// STR<type> Rt, Rn, imm12  (ARMv4*)
		if ( codeEncImm8[type] != 0 )
		{
			ESLAssert( (uint32_t) offsetAddr < 0x100 ) ;
			offsetAddr = ((offsetAddr & 0xF0) << 4)
							| codeEncImm8[type] | (offsetAddr & 0x0F) ;
		}
		else
		{
			ESLAssert( (uint32_t) offsetAddr < 0x1000 ) ;
		}
		uint32_t	code[1] ;
		code[0] = codeSTRimm12[type] | ((uint32_t) U << 23)
				| (regBase << 16) | (regSrc << 12) | (offsetAddr & 0x0FFF) ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// ldrd reg, [reg+imm8]  (must be aligned)
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMLoadDoubleMemOffsetImm8
	( ARMRegister regDst, ARMRegister regBase, int offsetAddr )
{
	if ( m_modeThumb )
	{
		// LDRD Rt, Rt2, Rn, imm8  (ARMv6T2)
		ESLAssert( !(offsetAddr & 0x03) ) ;
		ESLAssert( offsetAddr < 0x3FC ) ;
		uint16_t	code[2] ;
		ESLAssert( (uint32_t) offsetAddr < 0x100 ) ;
		code[0] = 0xE9D0 | (regBase) ;
		code[1] = (regDst << 12) | ((regDst + 1) << 8)
							| ((offsetAddr >> 2) & 0xFF) ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		if ( m_armVersion >= 5 )
		{
			// LDRD Rt, Rt2, Rn, imm8  (ARMv5TE*)
			ESLAssert( offsetAddr < 0xFF ) ;
			uint32_t	code[1] ;
			code[0] = 0xE1C000D0 | (regBase << 16) | (regDst << 12)
					| ((offsetAddr & 0xF0) << 4) | (offsetAddr & 0x0F) ;
			m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
		}
		else
		{
			ESLAssert( offsetAddr < 0xFFF ) ;
			WriteARMLoadMemOffsetImm12( regDst, regBase, offsetAddr ) ;
			WriteARMLoadMemOffsetImm12
				( (ARMRegister) (regDst + 1), regBase, offsetAddr + 4 ) ;
		}
	}
}

// strd [reg+imm8], reg  (must be aligned)
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMStoreDoubleMemOffsetImm8
	( ARMRegister regSrc, ARMRegister regBase, int offsetAddr )
{
	if ( m_modeThumb )
	{
		// STRD Rt, Rt2, Rn, imm8  (ARMv6T2)
		ESLAssert( !(offsetAddr & 0x03) ) ;
		ESLAssert( offsetAddr < 0x3FC ) ;
		uint16_t	code[2] ;
		ESLAssert( (uint32_t) offsetAddr < 0x100 ) ;
		code[0] = 0xE9C0 | (regBase) ;
		code[1] = (regSrc << 12) | ((regSrc + 1) << 8)
							| ((offsetAddr >> 2) & 0xFF) ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		if ( m_armVersion >= 5 )
		{
			// STRD Rt, Rt2, Rn, imm8  (ARMv5TE*)
			ESLAssert( offsetAddr <= 0xFF ) ;
			uint32_t	code[1] ;
			code[0] = 0xE1C000F0 | (regBase << 16) | (regSrc << 12)
					| ((offsetAddr & 0xF0) << 4) | (offsetAddr & 0x0F) ;
			m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
		}
		else
		{
			WriteARMStoreMemOffsetImm12( regSrc, regBase, offsetAddr ) ;
			WriteARMStoreMemOffsetImm12
				( (ARMRegister) (regSrc + 1), regBase, offsetAddr + 4 ) ;
		}
	}
}

// ldrex reg, [reg]
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMLoadMemEx
	( ARMRegister regDst, ARMRegister regBase )
{
	if ( m_modeThumb )
	{
		// LDREX Rt, Rn, imm  (ARMv6T2)
		uint16_t	code[2] ;
		code[0] = 0xE850 | (regBase) ;
		code[1] = 0x0F00 | (regDst << 12) ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		if ( m_armVersion >= 6 )
		{
			// LDREX Rt, Rn  (ARMv6*)
			uint32_t	code[1] ;
			code[0] = 0xE1900F9F | (regBase << 16) | (regDst << 12) ;
			m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
		}
		else
		{
			WriteARMLoadMemOffsetImm12( regDst, regBase, 0 ) ;
		}
	}
}

// strex reg, [reg], reg
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMStoreMemEx
	( ARMRegister regDst, ARMRegister regSrc, ARMRegister regBase )
{
	if ( m_modeThumb )
	{
		// STREX Rd, Rt, Rn, imm  (ARMv6T2)
		uint16_t	code[2] ;
		code[0] = 0xE840 | (regBase) ;
		code[1] = (regSrc << 12) | (regDst << 8) ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		if ( m_armVersion >= 6 )
		{
			// STREX Rd, Rt, Rn  (ARMv6*)
			uint32_t	code[1] ;
			code[0] = 0xE1800F90 | (regBase << 16) | (regDst << 12) | regSrc ;
			m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
		}
		else
		{
			WriteARMStoreMemOffsetImm12( regSrc, regBase, 0 ) ;
			WriteARMMoveRegImm( regDst, 0 ) ;
		}
	}
}

// clrex
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMClrEx( void )
{
	if ( m_modeThumb )
	{
		if ( m_armVersion >= 7 )
		{
			// CLREX  (ARMv7)
			uint16_t	code[2] ;
			code[0] = 0xF3BF ;
			code[1] = 0x8F2F ;
			m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
		}
	}
	else
	{
		if ( m_armVersion >= 6 )
		{
			// CLREX  (ARMv6K)
			uint32_t	code[1] ;
			code[0] = 0xF57FF01F ;
			m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
		}
	}
}

// lea reg, context->m_regset[x]
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMLeaSakura2Register
					( ARMRegister regDst, int regSakura2 )
{
	size_t	offsetReg = Context::OffsetOfReg(regSakura2) ;
	WriteARMAddRegRegImm( regDst, ARM_r10, offsetReg, regDst ) ;
}

// op[s] reg, reg, imm8
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMOpRegRegImm12
	( uint32_t armOpCode, uint32_t thumbOpCode,
		ARMRegister regDst, ARMRegister regSrc,
		int imm12, ARMCondition condARM, bool fSetFlags )
{
	if ( m_modeThumb )
	{
		if ( condARM != cond_AL )
		{
			// B imm8  (ARMv4T)
			PreserveContinuousCodes( 0x10 ) ;
			uint16_t	code[1] ;
			code[0] = 0xD000 | (uint16_t) ((condARM ^ 0x01) << 8)
								| (uint16_t) (((6 - 4) >> 1) & 0xFF) ;
			m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(uint16_t) ) ;
		}
		// OP Rd, Rn, imm12  (ARMv6T2)
		uint16_t	code[2] ;
		ESLAssert( m_armVersion >= 6 ) ;
		ESLAssert( (uint32_t) imm12 < 0x100 ) ;
		uint16_t	i = (uint16_t) (imm12 >> 11) & 0x01 ;
		uint16_t	imm3 = (uint16_t) (imm12 >> 8) & 0x07 ;
		uint16_t	imm8 = (uint16_t) imm12 & 0xFF ;
		code[0] = ((thumbOpCode >> 16) & 0xFFFF) | (i << 10) | regSrc ;
		code[1] = (thumbOpCode & 0xFFFF) | (imm3 << 12) | (regDst << 8) | imm8 ;
		if ( fSetFlags )
		{
			code[0] |= (1 << 4) ;
		}
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		// OP Rd, Rn, imm12  (ARMv4*)
		ESLAssert( (uint32_t) imm12 < 0x100 ) ;
		uint32_t	code[1] ;
		code[0] = armOpCode | (condARM << 28)
				| (regSrc << 16)
				| (regDst << 12) | ((uint32_t) imm12 & 0x0FFF) ;
		if ( fSetFlags )
		{
			code[0] |= (1 << 20) ;
		}
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// op reg, reg, reg, shift
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMOpRegRegRegShift
	( uint32_t armOpCode, uint32_t thumbOpCode,
		ARMRegister regDst, ARMRegister regSrc1,
			ARMRegister regSrc2, int nShift,
			ARMCondition condARM, bool fSetFlags )
{
	uint16_t	imm5 = 0, type = 0 ;
	if ( nShift > 0 )
	{
		// LSL
		imm5 = (uint16_t) nShift & 0x1F ;
	}
	else if ( nShift < 0 )
	{
		if ( nShift >= -31 )
		{
			type = 1 ;		// LSR
			imm5 = (uint16_t) -nShift ;
		}
		else
		{
			type = 2 ;		// ASR
			imm5 = (uint16_t) (-nShift - 32) & 0x1F ;
		}
	}
	if ( m_modeThumb )
	{
		if ( condARM != cond_AL )
		{
			// B imm8  (ARMv4T)
			PreserveContinuousCodes( 0x10 ) ;
			uint16_t	code[1] ;
			code[0] = 0xD000 | (uint16_t) ((condARM ^ 0x01) << 8)
								| (uint16_t) (((6 - 4) >> 1) & 0xFF) ;
			m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(uint16_t) ) ;
		}
		// OP Rd, Rn, Rm, shift  (ARMv6T2)
		ESLAssert( m_armVersion >= 6 ) ;
		uint16_t	code[2] ;
		uint16_t	imm3 = (imm5 >> 2) & 0x07 ;
		uint16_t	imm2 = (imm5 & 0x03) ;
		code[0] = ((thumbOpCode >> 16) & 0xFFFF) | regSrc1 ;
		code[1] = (thumbOpCode & 0xFFFF)
					| (imm3 << 12) | (regDst << 8)
					| (imm2 << 6) | (type << 4) | regSrc2 ;
		if ( fSetFlags )
		{
			code[0] |= (1 << 4) ;
		}
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		// OP Rd, Rn, Rm, shift  (ARMv4*)
		uint32_t	code[1] ;
		code[0] = armOpCode | (condARM << 28)
					| (regSrc1 << 16) | (regDst << 12)
					| (imm5 << 7) | (type << 5) | regSrc2 ;
		if ( fSetFlags )
		{
			code[0] |= (1 << 20) ;
		}
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// op reg, reg, reg
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMOpRegRegReg
	( uint32_t armOpCode, uint32_t thumbOpCode,
		ARMRegister regDst, ARMRegister regSrc1, ARMRegister regSrc2,
			ARMCondition condARM, bool fSetFlags )
{
	uint16_t	S = fSetFlags ? 1 : 0 ;
	if ( m_modeThumb )
	{
		uint16_t	code[2] ;
		code[0] = (uint16_t) ((thumbOpCode >> 16) & 0xFFFF) | (S << 4) | regSrc1 ;
		code[0] = (uint16_t) (thumbOpCode & 0xFFFF) | (regDst << 8) | regSrc2 ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		uint32_t	code[1] ;
		code[0] = armOpCode | (condARM << 28) | ((uint32_t) S << 20)
							| (regDst << 12) | (regSrc2 << 8) | regSrc1 ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// and reg, reg, imm8
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMAndRegRegImm8
	( ARMRegister regDst, ARMRegister regSrc, int imm12,
			ARMCondition condARM, bool fSetFlags )
{
	WriteARMOpRegRegImm12
		( armOpAndImm12, thumbOpAndImm12,
			regDst, regSrc, imm12, condARM, fSetFlags ) ;
}

// add reg, reg, imm8
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMAddRegRegImm8
		( ARMRegister regDst, ARMRegister regSrc, int imm12,
			ARMCondition condARM, bool fSetFlags )
{
	WriteARMOpRegRegImm12
		( armOpAddImm12, thumbOpAddImm12,
			regDst, regSrc, imm12, condARM, fSetFlags ) ;
}

// adc reg, reg, imm8
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMAdcRegRegImm8
	( ARMRegister regDst, ARMRegister regSrc, int imm12,
		ARMCondition condARM, bool fSetFlags )
{
	WriteARMOpRegRegImm12
		( armOpAdcImm12, thumbOpAdcImm12,
			regDst, regSrc, imm12, condARM, fSetFlags ) ;
}

// macro: add reg, reg, imm  (use regTemp if imm >= 0x100)
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMAddRegRegImm
	( ARMRegister regDst,
		ARMRegister regSrc, int imm32, ARMRegister regTemp )
{
	if ( (imm32 >= 0) & (imm32 <= 0xFF) )
	{
		WriteARMAddRegRegImm8( regDst, regSrc, imm32 ) ;
	}
	else
	{
		ESLAssert( (regTemp != regDst) || (regTemp != regSrc) ) ;
		PreserveContinuousCodes( 0x10 ) ;
		WriteARMMoveRegImm( regTemp, imm32 ) ;
		WriteARMAddRegRegRegShift( regDst, regSrc, regTemp ) ;
	}
}

// sub reg, reg, imm8
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMSubRegRegImm8
		( ARMRegister regDst, ARMRegister regSrc, int imm12,
			ARMCondition condARM, bool fSetFlags )
{
	WriteARMOpRegRegImm12
		( armOpSubImm12, thumbOpSubImm12,
			regDst, regSrc, imm12, condARM, fSetFlags ) ;
}

// sbc reg, reg, imm8
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMSbcRegRegImm8
	( ARMRegister regDst, ARMRegister regSrc, int imm12,
		ARMCondition condARM, bool fSetFlags )
{
	WriteARMOpRegRegImm12
		( armOpSbcImm12, thumbOpSbcImm12,
			regDst, regSrc, imm12, condARM, fSetFlags ) ;
}

// macro: sub reg, reg, imm
// (use regTemp if imm >= 0x100,
//  possibility regDst==regTemp,
//  but impossible regDst==regSrc==regTemp)
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMSubRegRegImm
	( ARMRegister regDst,
			ARMRegister regSrc, int imm32, ARMRegister regTemp )
{
	if ( (imm32 >= 0) & (imm32 <= 0xFF) )
	{
		WriteARMSubRegRegImm8( regDst, regSrc, imm32 ) ;
	}
	else
	{
		ESLAssert( regTemp != regSrc ) ;
		PreserveContinuousCodes( 0x10 ) ;
		WriteARMMoveRegImm( regTemp, imm32 ) ;
		WriteARMSubRegRegRegShift( regDst, regSrc, regTemp ) ;
	}
}

// add reg, reg, reg, shift
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMAddRegRegRegShift
	( ARMRegister regDst,
		ARMRegister regSrc1, ARMRegister regSrc2,
		int nShift, ARMCondition condARM, bool fSetFlags )
{
	WriteARMOpRegRegRegShift
		( armOpADD, thumbOpADD,
			regDst, regSrc1, regSrc2, nShift, condARM, fSetFlags ) ;
}

// sub reg, reg, reg, shift
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMSubRegRegRegShift
	( ARMRegister regDst,
		ARMRegister regSrc1, ARMRegister regSrc2,
		int nShift, ARMCondition condARM, bool fSetFlags )
{
	WriteARMOpRegRegRegShift
		( armOpSUB, thumbOpSUB,
			regDst, regSrc1, regSrc2, nShift, condARM, fSetFlags ) ;
}

// and reg, reg, reg, shift
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMAndRegRegRegShift
	( ARMRegister regDst,
		ARMRegister regSrc1, ARMRegister regSrc2, int nShift )
{
	WriteARMOpRegRegRegShift
		( armOpAND, thumbOpAND, regDst, regSrc1, regSrc2, nShift ) ;
}

// or reg, reg, reg, shift
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMOrRegRegRegShift
	( ARMRegister regDst,
		ARMRegister regSrc1, ARMRegister regSrc2, int nShift,
		ARMCondition condARM, bool fSetFlags )
{
	WriteARMOpRegRegRegShift
		( armOpORR, thumbOpORR,
			regDst, regSrc1, regSrc2, nShift, condARM, fSetFlags ) ;
}

// xor reg, reg, reg, shift
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMXorRegRegRegShift
	( ARMRegister regDst, ARMRegister regSrc1,
		ARMRegister regSrc2, int nShift,
		ARMCondition condARM, bool fSetFlags )
{
	WriteARMOpRegRegRegShift
		( armOpEOR, thumbOpEOR,
			regDst, regSrc1, regSrc2, nShift, condARM, fSetFlags ) ;
}

// not reg, reg, shift
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMNotRegRegShift
	( ARMRegister regDst,
		ARMRegister regSrc, int nShift,
		ARMCondition condARM, bool fSetFlags )
{
	WriteARMOpRegRegRegShift
		( armOpMVN, thumbOpMVN,
			regDst, ARM_r0, regSrc, nShift, condARM, fSetFlags ) ;
}

// cmp reg, imm
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMCmpRegImm8( ARMRegister regSrc, int immData )
{
	if ( m_modeThumb )
	{
		// CMP Rn, imm  (ARMv6T2)
		ESLAssert( (immData >= 0) && (immData <= 0xFF) ) ;
		uint16_t	code[2] ;
		code[0] = 0xF1B0 | regSrc ;
		code[1] = 0x0F00 | (uint16_t) (immData & 0xFF) ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		// CMP Rn, imm  (ARMv4*)
		ESLAssert( (immData >= 0) && (immData <= 0xFF) ) ;
		uint32_t	code[1] ;
		code[0] = 0xE3500000 | (regSrc << 16) | (immData & 0xFF) ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// cmp reg, reg, shift
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMCmpRegRegShift
	( ARMRegister regSrc1, ARMRegister regSrc2, int nShift )
{
	uint16_t	imm5 = 0, type = 0 ;
	if ( nShift > 0 )
	{
		// LSL
		imm5 = (uint16_t) nShift & 0x1F ;
	}
	else if ( nShift < 0 )
	{
		if ( nShift >= -31 )
		{
			type = 1 ;		// LSR
			imm5 = (uint16_t) -nShift ;
		}
		else
		{
			type = 2 ;		// ASR
			imm5 = (uint16_t) (-nShift - 32) & 0x1F ;
		}
	}
	if ( m_modeThumb )
	{
		// CMP Rn, Rm, shift  (ARMv6T2)
		ESLAssert( m_armVersion >= 6 ) ;
		uint16_t	code[2] ;
		uint16_t	imm3 = (imm5 >> 2) & 0x07 ;
		uint16_t	imm2 = (imm5 & 0x03) ;
		code[0] = 0xEBB0 | regSrc1 ;
		code[1] = 0x0F00 | (imm3 << 12)
					| (imm2 << 6) | (type << 4) | regSrc2 ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		// CMP Rn, Rm, shift  (ARMv4*)
		uint32_t	code[1] ;
		code[0] = 0xE1500000 | (regSrc1 << 16)
						| (imm5 << 7) | (type << 5) | regSrc2 ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// test reg, imm
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMTestRegImm8( ARMRegister regSrc, int immData )
{
	if ( m_modeThumb )
	{
		// TST Rn, imm  (ARMv6T2)
		ESLAssert( (immData >= 0) && (immData <= 0xFF) ) ;
		uint16_t	code[2] ;
		code[0] = 0xF010 | regSrc ;
		code[1] = 0x0F00 | (uint16_t) (immData & 0xFF) ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		// TST Rn, imm  (ARMv4*)
		ESLAssert( (immData >= 0) && (immData <= 0xFF) ) ;
		uint32_t	code[1] ;
		code[0] = 0xE3100000 | (regSrc << 16) | (immData & 0xFF) ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// shift reg, reg, imm5
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMShiftRegRegImm
	( ARMRegister regDst, ARMRegister regSrc,
			int nShift, bool fRight, bool fArithmetic )
{
	if ( nShift == 0 )
	{
		return ;
	}
	uint16_t	imm5 = (uint16_t) (nShift & 0x1F) ;
	int	type = 0 ;
	if ( fRight )
	{
		type = fArithmetic ? 2 : 1 ;
	}
	if ( m_modeThumb )
	{
		if ( (regDst < 8) & (regSrc < 8) )
		{
			static const uint16_t	codeShiftImm[] =
			{
				0x0000, 0x0800, 0x1000,
			} ;
			// SHIFT Rd, Rm, imm  (ARM4T)
			uint16_t	code[1] ;
			code[0] = codeShiftImm[type]
						| (imm5 << 6) | (regSrc << 3) | regDst ;
			m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
		}
		else
		{
			static const uint16_t	codeShiftImm[] =
			{
				0x0000, 0x0010, 0x0020,
			} ;
			// SHIFT Rd, Rm, imm  (ARM6T2)
			uint16_t	code[2] ;
			uint16_t	imm3 = (imm5 >> 2) ;
			uint16_t	imm2 = (imm5 & 0x03) ;
			code[0] = 0xEA4F ;
			code[1] = codeShiftImm[type] | (imm3 << 12)
						| (regDst << 8) | (imm2 << 6) | regSrc ;
			m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
		}
	}
	else
	{
		static const uint32_t	codeShiftImm[] =
		{
			0xE1A00000, 0xE1A00020, 0xE1A00040,
		} ;
		// SHIFT Rd, Rm, imm  (ARMv4*)
		uint32_t	code[1] ;
		code[0] = codeShiftImm[type] | (regDst << 12) | (imm5 << 7) | regSrc ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// lsl reg, reg, imm5
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMShiftLeftImm
	( ARMRegister regDst, ARMRegister regSrc, int nShift )
{
	WriteARMShiftRegRegImm( regDst, regSrc, nShift, false, false ) ;
}

// lsr reg, reg, imm5
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMShiftRightImm
	( ARMRegister regDst, ARMRegister regSrc, int nShift )
{
	WriteARMShiftRegRegImm( regDst, regSrc, nShift, true, false ) ;
}

// asr reg, reg, imm5
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMShiftARightImm
	( ARMRegister regDst, ARMRegister regSrc, int nShift )
{
	WriteARMShiftRegRegImm( regDst, regSrc, nShift, true, true ) ;
}

// mul reg, reg, reg
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMMulInt32
	( ARMRegister regDst,
		ARMRegister regSrc1, ARMRegister regSrc2 )
{
	if ( m_modeThumb )
	{
		// MUL Rd, Rn, Rm  (ARMv6T2)
		uint16_t	code[2] ;
		code[0] = 0xFB00 | regSrc1 ;
		code[1] = 0xF000 | (regDst << 8) | regSrc2 ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		// MUL Rd, Rn, Rm  (ARMv4*)
		uint32_t	code[1] ;
		code[0] = 0xE0000090
					| (regDst << 16) | (regSrc2 << 8) | regSrc1 ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// mla reg, reg, reg, reg
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMMulAddInt32
	( ARMRegister regDst,
		ARMRegister regSrc1, ARMRegister regSrc2, ARMRegister regSrcAdd )
{
	if ( m_modeThumb )
	{
		// MLA Rd, Rn, Rm, Ra  (ARMv6T2)
		uint16_t	code[2] ;
		code[0] = 0xFB00 | regSrc1 ;
		code[1] = 0x0000 | (regSrcAdd << 12) | (regDst << 8) | regSrc2 ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		// MLA Rd, Rn, Rm, Ra  (ARMv4*)
		uint32_t	code[1] ;
		code[0] = 0xE0200090
					| (regDst << 16) | (regSrcAdd << 12)
					| (regSrc2 << 8) | regSrc1 ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// smull reg, reg, reg, reg
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMMulSInt64
	( ARMRegister regDst0, ARMRegister regDst1,
		ARMRegister regSrc1, ARMRegister regSrc2 )
{
	if ( m_modeThumb )
	{
		// SMULL RdLo, RdHi, Rn, Rm  (ARMv6T2)
		uint16_t	code[2] ;
		code[0] = 0xFB80 | regSrc1 ;
		code[1] = 0x0000 | (regDst0 << 12) | (regDst1 << 8) | regSrc2 ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		// SMULL RdLo, RdHi, Rn, Rm  (ARMv4*)
		uint32_t	code[1] ;
		code[0] = 0xE0C00090
					| (regDst1 << 16) | (regDst0 << 12)
					| (regSrc2 << 8) | regSrc1 ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// umull reg, reg, reg, reg
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMMulUInt64
	( ARMRegister regDst0, ARMRegister regDst1,
		ARMRegister regSrc1, ARMRegister regSrc2 )
{
	if ( m_modeThumb )
	{
		// UMULL RdLo, RdHi, Rn, Rm  (ARMv6T2)
		uint16_t	code[2] ;
		code[0] = 0xFBA0 | regSrc1 ;
		code[1] = 0x0000 | (regDst0 << 12) | (regDst1 << 8) | regSrc2 ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		// UMULL RdLo, RdHi, Rn, Rm  (ARMv4*)
		uint32_t	code[1] ;
		code[0] = 0xE0800090
					| (regDst1 << 16) | (regDst0 << 12)
					| (regSrc2 << 8) | regSrc1 ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// macro: mov reg, imm
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMMoveRegImm
	( ARMRegister regDst, int32_t immData, ARMCondition condARM )
{
	if ( m_modeThumb )
	{
		if ( (regDst < 8) & (immData >= 0) & (immData <= 0xFF) )
		{
			// MOVS Rd, imm8  (ARMv4T)
			uint16_t	code[1] ;
			code[0] = 0x2000 | (regDst << 8) | (immData & 0xFF) ;
			m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
		}
		else
		{
			WriteARMMoveRegImm32( regDst, (uint32_t) immData, condARM ) ;
		}
	}
	else
	{
		if ( (m_armVersion >= 6) & (immData >= 0) & (immData <= 0xFFFF) )
		{
			// MOV Rd, imm16  (ARMv6T2)
			uint32_t	code[1] ;
			uint32_t	imm4 = (immData >> 12) & 0x0F ;
			uint32_t	imm12 = immData & 0x0FFF ;
			code[0] = 0x03000000 | (condARM << 28)
						| (imm4 << 16) | (regDst << 12) | imm12 ;
			m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
		}
		else
		{
			WriteARMMoveRegImm32( regDst, (uint32_t) immData, condARM ) ;
		}
	}
}

// mov reg, imm32
//////////////////////////////////////////////////////////////////////////////
void * ARMGenericAssembler::WriteARMMoveRegImm32
	( ARMRegister regDst, uint32_t immData, ARMCondition condARM )
{
	PreserveContinuousCodes( 0x08 ) ;
	//
	uint8_t *	pbytCode = (uint8_t*) m_buf->GetNext() ;
	uint32_t *	ptrData =
			(uint32_t*) m_buf->AllocateData
							( sizeof(uint32_t), sizeof(uint32_t) ) ;
	*ptrData = immData ;
	//
	if ( m_modeThumb )
	{
		ESLAssert( condARM == cond_AL ) ;
		uint32_t	imm8 = ((uint32_t) ((ulong_ptr_t) ptrData)
					- ((uint32_t) ((ulong_ptr_t) (pbytCode + 4)) & ~0x03)) >> 2 ;
		if ( (regDst < 8) && (imm8 < 0x100) )
		{
			// LDR reg, [PC+rel8*4]  (ARMv4T)
			ESLAssert( imm8 < 0x100 ) ;
			uint16_t	code[1] ;
			code[0] = 0x4800 | (regDst << 8) | (uint16_t) (imm8 & 0xFF) ;
			m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
		}
		else
		{
			// LDR reg, [PC+rel12]  (ARMv6T2)
			uint32_t	imm12 = (uint32_t) ((ulong_ptr_t) ptrData)
						- ((uint32_t) ((ulong_ptr_t) (pbytCode + 4)) & ~0x03) ;
			ESLAssert( imm12 < 0x1000 ) ;
			uint16_t	code[2] ;
			code[0] = 0xF8DF ;
			code[1] = (regDst << 12) | (uint16_t) (imm12 & 0x0FFF) ;
			m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
		}
	}
	else
	{
		// LDR reg, [PC+rel12]  (ARMv4*)
		uint32_t	imm12 = (uint32_t) ((ulong_ptr_t) ptrData)
						- ((uint32_t) ((ulong_ptr_t) (pbytCode + 8)) & ~0x03) ;
		ESLAssert( imm12 < 0x1000 ) ;
		uint32_t	code[1] ;
		code[0] = 0x059F0000 | (condARM << 28)
						| (regDst << 12) | (imm12 & 0x0FFF) ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	return	ptrData ;
}

// mov reg, reg
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMMoveRegReg
	( ARMRegister regDst, ARMRegister regSrc, ARMCondition condARM )
{
	if ( m_modeThumb )
	{
		/*
		// MOV Rd, Rm  (ARMv4T | Rd>=8 の時 ARMv6*)
		uint16_t	code[1] ;
		uint16_t	D = (uint16_t) (regDst >> 3) & 0x01 ;
		code[0] = 0x4600 | (D << 7) | (regSrc << 3) | (regDst & 0x07) ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
		*/
		// MOV Rd, Rm  (ARMv6T2)
		uint16_t	code[2] ;
		uint16_t	D = (uint16_t) (regDst >> 3) & 0x01 ;
		code[0] = 0xEA4F ;
		code[1] = (regDst << 8) | regSrc ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		// MOV Rd, Rm  (ARMv4*)
		uint32_t	code[1] ;
		code[0] = 0x01A00000 | (condARM << 28) | (regDst << 12) | (regSrc) ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// jump reg
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMJumpReg( ARMRegister reg )
{
	if ( m_modeThumb )
	{
		// BX Rm  (ARMv4T)
		uint16_t	code[1] ;
		code[0] = 0x4700 | (reg << 3) ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		// BX Rm  (ARMv4T*)
		uint32_t	code[1] ;
		code[0] = 0xE12FFF10 | reg ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// jump imm32
//////////////////////////////////////////////////////////////////////////////
void * ARMGenericAssembler::WriteARMJumpImm32
	( const void * pfnJmpTarget, ARMCondition cond )
{
	if ( m_modeThumb )
	{
		PreserveContinuousCodes( 0x20 ) ;
		//
		// LDR imm32
		void *	ptrData =
			WriteARMMoveRegImm32
				( ARM_r6, (uint32_t) ((ulong_ptr_t) pfnJmpTarget) ) ;
		//
		// B imm8  (ARMv4T)
		uint16_t	code[1] ;
		if ( cond != cond_AL )
		{
			code[0] = 0xD000 | ((cond ^ 0x01) << 8) ;
			m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
		}
		//
		// BX Rm  (ARMv4T)
		code[0] = 0x4700 | (ARM_r6 << 3) ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
		//
		return	ptrData ;
	}
	else
	{
		PreserveContinuousCodes( 0x20 ) ;
		//
		// LDR imm32
		void *	ptrData =
			WriteARMMoveRegImm32
				( ARM_r6, (uint32_t) ((ulong_ptr_t) pfnJmpTarget), cond ) ;
		//
		// BXc Rm  (ARMv4T*)
		uint32_t	code[1] ;
		code[0] = 0x012FFF10 | (cond << 28) | ARM_r6 ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
		//
		return	ptrData ;
	}
}

// jump offset
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMJumpOffsetImm
				( int immPrevOffset, ARMCondition cond )
{
	if ( m_modeThumb )
	{
		uint16_t	code[1] ;
		const int	immOffset = immPrevOffset - 4 ;
		if ( cond != cond_AL )
		{
			// Bc imm8  (ARMv4T)
			ESLAssert( (immOffset >= -0xFF) && (immOffset <= 0xFF) ) ;
			code[0] = 0xD000 | (cond << 8) | ((immOffset >> 1) & 0xFF) ;
		}
		else
		{
			// B imm11  (ARMv4T)
			ESLAssert( (immOffset >= -0x7FF) && (immOffset <= 0x7FF) ) ;
			code[0] = 0xE000 | ((immOffset >> 1) & 0x7FF) ;
		}
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		// Bc imm24
		const int	immOffset = immPrevOffset - 8 ;
		ESLAssert( (immOffset >= -0x1FFFFFF) && (immOffset <= 0x1FFFFFF) ) ;
		uint32_t	code[1] ;
		code[0] = 0x0A000000 | (cond << 28) | ((immOffset >> 2) & 0x00FFFFFF) ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// macro: jump imm32
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMJumpImm
	( const void * pfnJmpTarget, ARMCondition cond )
{
	long_ptr_t	nJmpTarget = (long_ptr_t) pfnJmpTarget ;
	long_ptr_t	pcCodePtr = (long_ptr_t) GetNextAddress() ;
	if ( m_modeThumb )
	{
		WriteARMJumpImm32( pfnJmpTarget, cond ) ;
	}
	else
	{
		long int	immOffset = nJmpTarget - (pcCodePtr + 8) ;
		if ( (immOffset >= -0x1FFFFFF) && (immOffset <= 0x1FFFFFF) )
		{
			WriteARMJumpOffsetImm( nJmpTarget - pcCodePtr, cond ) ;
		}
		else
		{
			WriteARMJumpImm32( pfnJmpTarget, cond ) ;
		}
	}
}

// call reg
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMCallReg( ARMRegister reg )
{
	if ( m_modeThumb )
	{
		// BLX Rm  (ARMv5T*)
		uint16_t	code[1] ;
		code[0] = 0x4780 | (reg << 3) ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		if ( m_armVersion < 5 )
		{
			// ADD lr, pc, 0
			WriteARMAddRegRegImm8( ARM_LR, ARM_PC, 0 ) ;
			//
			// BX Rm
			WriteARMJumpReg( reg ) ;
		}
		else
		{
			// BLX Rm  (ARMv5T*)
			uint32_t	code[1] ;
			code[0] = 0xE12FFF30 | reg ;
			m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
		}
	}
}

// macro: call imm32
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMCallImm( const void * pfnJmpTarget )
{
	long_ptr_t	pcCodePtr = (long_ptr_t) GetNextAddress() ;
	long_ptr_t	nJmpTarget = (long_ptr_t) pfnJmpTarget ;
	if ( m_modeThumb )
	{
		PreserveContinuousCodes( 0x10 ) ;
		WriteARMMoveRegImm32( ARM_r6, (uint32_t) nJmpTarget ) ;
		WriteARMCallReg( ARM_r6 ) ;
	}
	else
	{
		int32_t	imm32 = (nJmpTarget & ~0x01) - (pcCodePtr + 8) ;
		if ( (m_armVersion >= 5)
			&& (imm32 >= -0x1FFFFFE) && (imm32 <= 0x1FFFFFE) )
		{
			uint32_t	code[1] ;
			if ( nJmpTarget & 0x01 )
			{
				// BLX label  (ARMv5T*)
				code[0] = 0xFA000000
							| (((imm32 >> 1) & 0x01) << 24)
							| ((imm32 >> 2) & 0x00FFFFFF) ;
			}
			else
			{
				// BLc label  (ARMv4*)
				code[0] = 0x0B000000 | (cond_AL << 28)
								| ((imm32 >> 2) & 0x00FFFFFF) ;
			}
			m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
		}
		else
		{
			PreserveContinuousCodes( 0x10 ) ;
			WriteARMMoveRegImm32( ARM_r6, (uint32_t) nJmpTarget ) ;
			WriteARMCallReg( ARM_r6 ) ;
		}
	}
}

// push reg,...
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMPushReg( ARMRegister reg )
{
	WriteARMPushRegs( &reg, 1 ) ;
}

void ARMGenericAssembler::WriteARMPushRegs
			( const ARMRegister * regs, size_t count )
{
	uint32_t	maskRegs = 0 ;
	for ( size_t i = 0; i < count; i ++ )
	{
		maskRegs |= (1 << regs[i]) ;
	}
	if ( maskRegs == 0 )
	{
		return ;
	}
	if ( m_modeThumb )
	{
		if ( maskRegs & 0x5F00 )
		{
			// PUSH registers  (ARMv6T2)
			ESLAssert( m_armVersion >= 6 ) ;
			uint16_t	code[2] ;
			code[0] = 0xE92D ;
			code[1] = (uint16_t) maskRegs & 0x5FFF ;
			m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
		}
		else
		{
			// PUSH registers  (ARMv4T)
			uint16_t	code[1] ;
			code[0] = 0xB400 | (((maskRegs >> 14) & 0x01) << 8)
									| ((uint16_t) maskRegs & 0x00FF) ;
			m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
		}
	}
	else
	{
		// PUSH registers  (ARMv4*)
		uint32_t	code[1] ;
		code[0] = 0xE92D0000 | (maskRegs & 0xFFFF) ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// pop reg,...
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMPopReg( ARMRegister reg )
{
	WriteARMPopRegs( &reg, 1 ) ;
}

void ARMGenericAssembler::WriteARMPopRegs( const ARMRegister * regs, size_t count )
{
	uint32_t	maskRegs = 0 ;
	for ( size_t i = 0; i < count; i ++ )
	{
		maskRegs |= (1 << regs[i]) ;
	}
	if ( maskRegs == 0 )
	{
		return ;
	}
	if ( m_modeThumb )
	{
		if ( maskRegs & 0x5F00 )
		{
			// POP registers  (ARMv6T2)
			ESLAssert( m_armVersion >= 6 ) ;
			uint16_t	code[2] ;
			code[0] = 0xE8BD ;
			code[1] = (uint16_t) maskRegs & 0x5FFF ;
			m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
		}
		else
		{
			// POP registers  (ARMv4T)
			uint16_t	code[1] ;
			code[0] = 0xBC00 | (((maskRegs >> 15) & 0x01) << 8)
									| ((uint16_t) maskRegs & 0x00FF) ;
			m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
		}
	}
	else
	{
		// POP registers  (ARMv4*)
		uint32_t	code[1] ;
		code[0] = 0xE8BD0000 | (maskRegs & 0xFFFF) ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// MSR APSR_nzcvq, Rn
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMtoAPSR_nzcvq( ARMRegister reg )
{
	uint16_t	mask = 2 ;
	if ( m_modeThumb )
	{
		// MSR APSR_nzcvq, Rn  (ARMv6T2)
		ESLAssert( m_armVersion >= 6 ) ;
		uint16_t	code[2] ;
		code[0] = 0xF380 | reg ;
		code[1] = 0x8000 | (mask << 10) ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		// MSR APSR_nzcvq, Rn  (ARMv4*)
		uint32_t	code[1] ;
		code[0] = 0xE120F000 | ((uint32_t) mask << 18) | reg ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// clamp macro
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMClampValueImm8
	( ARMRegister reg, int minImm8, int maxImm8, ARMRegister regTemp )
{
	if ( minImm8 != 0 )
	{
		if ( minImm8 < 0 )
		{
			WriteARMAddRegRegImm8( reg, reg, -minImm8 ) ;
		}
		else
		{
			WriteARMSubRegRegImm8( reg, reg, minImm8 ) ;
		}
	}
	PreserveContinuousCodes( 0x20 ) ;
	WriteARMNotRegRegShift( regTemp, reg, -31-32 ) ;
	WriteARMAndRegRegImm8( regTemp, regTemp, (maxImm8 - minImm8) ) ;
	WriteARMCmpRegImm8( reg, (maxImm8 - minImm8) ) ;
	//
	if ( m_modeThumb )
	{
		WriteARMJumpOffsetImm( 2, cond_HI ) ;
		WriteARMMoveRegReg( reg, regTemp ) ;
	}
	else
	{
		WriteARMMoveRegReg( reg, regTemp, cond_HI ) ;
	}
	if ( minImm8 != 0 )
	{
		if ( minImm8 < 0 )
		{
			WriteARMSubRegRegImm8( reg, reg, -minImm8 ) ;
		}
		else
		{
			WriteARMAddRegRegImm8( reg, reg, minImm8 ) ;
		}
	}
}

void ARMGenericAssembler::WriteARMClampValueToSigned16
	( ARMRegister reg, ARMRegister regTemp )
{
	if ( m_armVersion >= 6 )
	{
		WriteARMSatRegImmRegShift( reg, 15, reg, 0, false ) ;
	}
	else
	{
		PreserveContinuousCodes( 0x20 ) ;
		WriteARMMoveRegImm( regTemp, 0x8000 ) ;
		WriteARMAddRegRegRegShift( reg, reg, regTemp ) ;
		PreserveContinuousCodes( 0x20 ) ;
		WriteARMMoveRegImm( regTemp, 0xFFFF ) ;
		WriteARMCmpRegRegShift( reg, regTemp ) ;
		WriteARMNotRegRegShift( regTemp, reg, -15-32 ) ;
		WriteARMShiftRightImm( regTemp, regTemp, 16 ) ;
		//
		if ( m_modeThumb )
		{
			WriteARMJumpOffsetImm( 2, cond_HI ) ;
			WriteARMMoveRegReg( reg, regTemp ) ;
		}
		else
		{
			WriteARMMoveRegReg( reg, regTemp, cond_HI ) ;
		}
		PreserveContinuousCodes( 0x20 ) ;
		WriteARMMoveRegImm( regTemp, 0x8000 ) ;
		WriteARMSubRegRegRegShift( reg, reg, regTemp ) ;
	}
}

// ARM SIMD 命令
// op reg, reg, reg, imm5
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteSIMDOpARMRegRegRegImm5
	( uint32_t armOpCode, uint32_t thumbOpCode,
		ARMRegister regDst,
		ARMRegister regSrc1, ARMRegister regSrc2, int imm5, bool fShiftFlag )
{
	uint16_t	s = fShiftFlag ? 1 : 0 ;
	imm5 &= 0x1F ;
	if ( m_modeThumb )
	{
		uint16_t	code[2] ;
		uint16_t	imm3 = (imm5 >> 2) & 0x07 ;
		uint16_t	imm2 = imm5 & 0x03 ;
		code[0] = (uint16_t) ((thumbOpCode >> 16) & 0xFFFF) | regSrc1 ;
		code[0] = (uint16_t) (thumbOpCode & 0xFFFF)
						| (imm3 << 12) | (regDst << 8)
						| (imm2 << 6) | (s << 5) | regSrc2 ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		uint32_t	code[1] ;
		code[0] = armOpCode | (cond_AL << 28)
					| (regSrc1 << 16) | (regDst << 12)
					| (imm5 << 7) | (s << 6) | regSrc2 ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// op reg, reg, reg
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteSIMDOpARMRegRegReg
	( uint32_t armOpCode, uint32_t thumbOpCode,
		ARMRegister regDst, ARMRegister regSrc1, ARMRegister regSrc2 )
{
	WriteSIMDOpARMRegRegRegImm5
		( armOpCode, thumbOpCode, regDst, regSrc1, regSrc2, 0, false ) ; 
}

// SAT reg, imm5, reg, shift
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMSatRegImmRegShift
	( ARMRegister regDst, int nSatBits,
			ARMRegister regSrc, int nShift, bool fUnsigned )
{
	uint32_t	armCode, thumbCode ;
	if ( fUnsigned )
	{
		armCode = armOpUSAT ;
		thumbCode = thumbOpUSAT ;
	}
	else
	{
		armCode = armOpSSAT ;
		thumbCode = thumbOpSSAT ;
	}
	uint16_t	sh = 0 ;
	if ( nShift < 0 )
	{
		sh = 1 ;
		nShift = - nShift ;
	}
	nShift &= 0x1F ;
	nSatBits &= 0x1F ;
	//
	if ( m_modeThumb )
	{
		uint16_t	code[2] ;
		uint16_t	imm3 = (nShift >> 2) & 0x07 ;
		uint16_t	imm2 = nShift & 0x03 ;
		code[0] = ((thumbCode >> 16) & 0xFFFF) | (sh << 5) | regSrc ;
		code[0] = (thumbCode & 0xFFFF)
					| (imm3 << 12) | (regDst << 8) | (imm2 << 6) | nSatBits ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		uint32_t	code[1] ;
		code[0] = armCode | (cond_AL << 28)
				| (nSatBits << 16) | (regDst << 12) | (sh << 6) | regSrc ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// SAT16 reg, imm5, reg
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMSat16RegImmReg
	( ARMRegister regDst, int nSatBits, ARMRegister regSrc, bool fUnsigned )
{
	uint32_t	armCode, thumbCode ;
	if ( fUnsigned )
	{
		armCode = armOpUSAT16 ;
		thumbCode = thumbOpUSAT16 ;
	}
	else
	{
		armCode = armOpSSAT16 ;
		thumbCode = thumbOpSSAT16 ;
	}
	nSatBits &= 0x0F ;
	if ( m_modeThumb )
	{
		uint16_t	code[2] ;
		code[0] = ((thumbCode >> 16) & 0xFFFF) | regSrc ;
		code[0] = (thumbCode & 0xFFFF) | (regDst << 8) | nSatBits ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		uint32_t	code[1] ;
		code[0] = armCode | (cond_AL << 28)
					| (nSatBits << 16) | (regDst << 12) | regSrc ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// PKH reg, reg, reg, imm
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMPack16RegRegRegImm
	( ARMRegister regDst, ARMRegister regSrc1,
		ARMRegister regSrc2, bool fShiftRight, int nShift )
{
	WriteSIMDOpARMRegRegRegImm5
		( armOpPKH, thumbOpPKH, regDst, regSrc1, regSrc2, nShift, fShiftRight ) ;
}

// BFop reg, reg, lsb, width
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMBFRegRegImmImm
	( uint32_t armOpCode, uint32_t thumbOpCode,
		ARMRegister regDst, ARMRegister regSrc, int lsb, int width )
{
	lsb &= 0x1F ;
	width &= 0x1F ;
	if ( m_modeThumb )
	{
		uint16_t	code[2] ;
		uint16_t	imm3 = (lsb >> 2) & 0x07 ;
		uint16_t	imm2 = lsb & 0x03 ;
		code[0] = ((thumbOpBFI >> 16) & 0xFFFF) | regSrc ;
		code[0] = (thumbOpBFI & 0xFFFF)
					| (imm3 << 12) | (regDst << 8) | (imm2 << 6) | width ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		uint32_t	code[1] ;
		code[0] = armOpBFI | (cond_AL << 28)
				| (width << 16) | (regDst << 12) | (lsb << 7) | regSrc ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// BFI reg, reg, lsb, width
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMBFIRegRegImmImm
	( ARMRegister regDst, ARMRegister regSrc, int lsb, int width )
{
	WriteARMBFRegRegImmImm
		( armOpBFI, thumbOpBFI, regDst, regSrc, lsb, lsb + width - 1 ) ;
}

// UBFX reg, reg, lsb, width
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMUBFXRegRegImmImm
	( ARMRegister regDst, ARMRegister regSrc, int lsb, int width )
{
	WriteARMBFRegRegImmImm
		( armOpUBFX, thumbOpUBFX, regDst, regSrc, lsb, width - 1 ) ;
}

// UXTop reg, reg, imm
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMUXTRegRegImm
	( uint32_t armOpCode, uint32_t thumbOpCode,
		ARMRegister regDst, ARMRegister regSrc, int pos )
{
	pos &= 0x03 ;
	if ( m_modeThumb )
	{
		uint16_t	code[2] ;
		code[0] = ((thumbOpCode >> 16) & 0xFFFF) ;
		code[0] = (thumbOpCode & 0xFFFF)
					| (regDst << 8) | (pos << 4) | regSrc ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		uint32_t	code[1] ;
		code[0] = armOpCode | (cond_AL << 28)
					| (regDst << 12) | (pos << 10) | regSrc ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// UXTB reg, reg, imm
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMUXTBRegRegImm
	( ARMRegister regDst, ARMRegister regSrc, int pos )
{
	WriteARMUXTRegRegImm( armOpUXTB, thumbOpUXTB, regDst, regSrc, pos ) ;
}

// UXTB16 reg, reg, imm
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMUXTB16RegRegImm
	( ARMRegister regDst, ARMRegister regSrc, int pos )
{
	WriteARMUXTRegRegImm( armOpUXTB16, thumbOpUXTB16, regDst, regSrc, pos ) ;
}

// UXTH reg, reg, imm
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteARMUXTHRegRegImm
	( ARMRegister regDst, ARMRegister regSrc, int pos )
{
	WriteARMUXTRegRegImm( armOpUXTH, thumbOpUXTH, regDst, regSrc, pos ) ;
}

// op reg, reg, reg
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteVFPOpRegRegReg
	( uint32_t armOpCode, uint32_t thumbOpCode,
		int vregDst, int vregSrc1, int vregSrc2, bool fDoubleFlag )
{
	uint16_t	D, Vd, N, Vn, M, Vm, sz ;
	if ( fDoubleFlag )
	{
		D = (uint16_t) (vregDst >> 4) & 0x01 ;
		Vd = (uint16_t) vregDst & 0x0F ;
		N = (uint16_t) (vregSrc1 >> 4) & 0x01 ;
		Vn = (uint16_t) vregSrc1 & 0x0F ;
		M = (uint16_t) (vregSrc2 >> 4) & 0x01 ;
		Vm = (uint16_t) vregSrc2 & 0x0F ;
		sz = 1 ;
	}
	else
	{
		D = (uint16_t) vregDst & 0x01 ;
		Vd = (uint16_t) (vregDst >> 1) & 0x0F ;
		N = (uint16_t) vregSrc1 & 0x01 ;
		Vn = (uint16_t) (vregSrc1 >> 1) & 0x0F ;
		M = (uint16_t) vregSrc2 & 0x01 ;
		Vm = (uint16_t) (vregSrc2 >> 1) & 0x0F ;
		sz = 0 ;
	}
	if ( m_modeThumb )
	{
		uint16_t	code[2] ;
		code[0] = (uint16_t) ((thumbOpCode >> 16) & 0xFFFF) | (D << 6) | Vn ;
		code[0] = (uint16_t) (thumbOpCode & 0xFFFF)
					| (Vd << 12) | (sz << 8) | (N << 7) | (M << 5) | Vm ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		uint32_t	code[1] ;
		code[0] = armOpCode | (cond_AL << 28)
					| ((uint32_t) D << 22) | ((uint32_t) Vn << 16)
					| (Vd << 12) | (sz << 8) | (N << 7) | (M << 5) | Vm ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// op reg, reg
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteVFPOpRegReg
	( uint32_t armOpCode, uint32_t thumbOpCode,
		int vregDst, int vregSrc, bool fDoubleFlag )
{
	WriteVFPOpRegRegReg
		( armOpCode, thumbOpCode, vregDst, 0, vregSrc, fDoubleFlag ) ;
}

// vpush reg,...
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteVFPPushReg32( int vreg, int count )
{
	// VPUSH registers  (VFPv2)
	uint16_t	D = vreg & 0x01 ;
	uint16_t	Vd = (vreg >> 1) & 0x0F ;
	if ( m_modeThumb )
	{
		uint16_t	code[2] ;
		code[0] = 0xED2D | (D << 6) ;
		code[1] = 0x0A00 | (Vd << 12) | (count & 0xFF) ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		uint32_t	code[1] ;
		code[0] = 0xED2D0A00 | ((uint32_t) D << 22)
						| ((uint32_t) Vd << 12) | (count & 0xFF) ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// vpop reg,...
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteVFPPopReg32( int vreg, int count )
{
	// VPOP registers  (VFPv2)
	uint16_t	D = vreg & 0x01 ;
	uint16_t	Vd = (vreg >> 1) & 0x0F ;
	if ( m_modeThumb )
	{
		uint16_t	code[2] ;
		code[0] = 0xECBD | (D << 6) ;
		code[1] = 0x0A00 | (Vd << 12) | (count & 0xFF) ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		uint32_t	code[1] ;
		code[0] = 0xECBD0A00 | ((uint32_t) D << 22)
					| ((uint32_t) Vd << 12) | (count & 0xFF) ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// vldr vreg, [reg]  (must be aligned)
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteVFPLoad64OffsetImm8
	( int vreg, ARMRegister regAddr, int offsetAddr )
{
	// VLDR Dd, [Rn+imm8]  (VFPv2)
	ESLAssert( (-0x3FC <= offsetAddr) && (offsetAddr <= 0x3FC) ) ;
	uint16_t	U = 1 ;
	if ( offsetAddr < 0 )
	{
		offsetAddr = - offsetAddr ;
		U = 0 ;
	}
	uint16_t	D = (vreg >> 4) & 0x01 ;
	uint16_t	Vd = (vreg & 0x0F) ;
	uint16_t	imm8 = (uint16_t) ((offsetAddr >> 2) & 0xFF) ;
	if ( m_modeThumb )
	{
		uint16_t	code[2] ;
		code[0] = 0xED10 | (U << 7) | (D << 6) | (regAddr) ;
		code[0] = 0x0B00 | (Vd << 12) | imm8 ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		uint32_t	code[1] ;
		code[0] = 0xED100B00
				| ((uint32_t) U << 23) | ((uint32_t) D << 22)
				| (regAddr << 16) | ((uint32_t) Vd << 12) | imm8 ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

void ARMGenericAssembler::WriteVFPLoad32OffsetImm8
	( int vreg, ARMRegister regAddr, int offsetAddr )
{
	// VLDR Sd, [Rn+imm8]  (VFPv2)
	ESLAssert( (-0x3FC <= offsetAddr) && (offsetAddr <= 0x3FC) ) ;
	uint16_t	U = 1 ;
	if ( offsetAddr < 0 )
	{
		offsetAddr = - offsetAddr ;
		U = 0 ;
	}
	uint16_t	D = (vreg >> 4) & 0x01 ;
	uint16_t	Vd = (vreg & 0x0F) ;
	uint16_t	imm8 = (uint16_t) ((offsetAddr >> 2) & 0xFF) ;
	if ( m_modeThumb )
	{
		uint16_t	code[2] ;
		code[0] = 0xED10 | (U << 7) | (D << 6) | (regAddr) ;
		code[0] = 0x0A00 | (Vd << 12) | imm8 ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		uint32_t	code[1] ;
		code[0] = 0xED100A00
				| ((uint32_t) U << 23) | ((uint32_t) D << 22)
				| (regAddr << 16) | ((uint32_t) Vd << 12) | imm8 ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// macro: vldr sreg, imm32
//////////////////////////////////////////////////////////////////////////////
float32_t * ARMGenericAssembler::WriteVFPLoadImm32( int sreg, float32_t imm32 )
{
	PreserveContinuousCodes( 0x20 ) ;
	//
	uint8_t *	pbytCode = (uint8_t*) m_buf->GetNext() ;
	float32_t *	ptrData =
			(float32_t*) m_buf->AllocateData
							( sizeof(float32_t), sizeof(float32_t) ) ;
	*ptrData = imm32 ;
	//
	int	offsetAddr ;
	if ( m_modeThumb )
	{
		offsetAddr =
			(int) ((long_ptr_t) ptrData - (long_ptr_t) (pbytCode + 4)) ;
	}
	else
	{
		offsetAddr =
			(int) ((long_ptr_t) ptrData - (long_ptr_t) (pbytCode + 8)) ;
	}
	if ( (-0x3FC <= offsetAddr) && (offsetAddr <= 0x3FC) )
	{
		WriteVFPLoad32OffsetImm8( sreg, ARM_PC, offsetAddr ) ;
	}
	else if ( (0 <= offsetAddr) && (offsetAddr <= 0x3FC + 0xFF) )
	{
		WriteARMAddRegRegImm8( ARM_r6, ARM_PC, offsetAddr - 0x3FC ) ;
		WriteVFPLoad32OffsetImm8( sreg, ARM_r6, 0x3FC ) ;
	}
	else
	{
		WriteARMMoveRegImm( ARM_r6, (int32_t) ((ulong_ptr_t) ptrData) ) ;
		WriteVFPLoad32OffsetImm8( sreg, ARM_r6, 0 ) ;
	}
	return	ptrData ;
}

// macro: vldr vreg, imm64
//////////////////////////////////////////////////////////////////////////////
int64_t * ARMGenericAssembler::WriteVFPLoadImm64( int vreg, int64_t imm64 )
{
	PreserveContinuousCodes( 0x20 ) ;
	//
	uint8_t *	pbytCode = (uint8_t*) m_buf->GetNext() ;
	int64_t *	ptrData =
			(int64_t*) m_buf->AllocateData
							( sizeof(int64_t), sizeof(int64_t) ) ;
	*ptrData = imm64 ;
	//
	int	offsetAddr ;
	if ( m_modeThumb )
	{
		offsetAddr =
			(int) ((long_ptr_t) ptrData - (long_ptr_t) (pbytCode + 4)) ;
	}
	else
	{
		offsetAddr =
			(int) ((long_ptr_t) ptrData - (long_ptr_t) (pbytCode + 8)) ;
	}
	if ( (-0x3FC <= offsetAddr) && (offsetAddr <= 0x3FC) )
	{
		WriteVFPLoad64OffsetImm8( vreg, ARM_PC, offsetAddr ) ;
	}
	else if ( (0 <= offsetAddr) && (offsetAddr <= 0x3FC + 0xFF) )
	{
		WriteARMAddRegRegImm8( ARM_r6, ARM_PC, offsetAddr - 0x3FC ) ;
		WriteVFPLoad64OffsetImm8( vreg, ARM_r6, 0x3FC ) ;
	}
	else
	{
		WriteARMMoveRegImm( ARM_r6, (int32_t) ((ulong_ptr_t) ptrData) ) ;
		WriteVFPLoad64OffsetImm8( vreg, ARM_r6, 0 ) ;
	}
	return	ptrData ;
}

// vstr [reg], vreg  (must be aligned)
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteVFPStore64OffsetImm8
	( int vreg, ARMRegister regAddr, int offsetAddr )
{
	// VSTR Dd, [Rt+imm8]  (VFPv2)
	ESLAssert( (-0x3FC <= offsetAddr) && (offsetAddr <= 0x3FC) ) ;
	uint16_t	U = 1 ;
	if ( offsetAddr < 0 )
	{
		offsetAddr = - offsetAddr ;
		U = 0 ;
	}
	uint16_t	D = (vreg >> 4) & 0x01 ;
	uint16_t	Vd = (vreg & 0x0F) ;
	uint16_t	imm8 = (uint16_t) ((offsetAddr >> 2) & 0xFF) ;
	if ( m_modeThumb )
	{
		uint16_t	code[2] ;
		code[0] = 0xED00 | (U << 7) | (D << 6) | (regAddr) ;
		code[0] = 0x0B00 | (Vd << 12) | imm8 ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		uint32_t	code[1] ;
		code[0] = 0xED000B00
				| ((uint32_t) U << 23) | ((uint32_t) D << 22)
				| (regAddr << 16) | ((uint32_t) Vd << 12) | imm8 ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// vmov.f32 Dd, Dm
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteMoveVFP32
		( int sregDst, int sregSrc, ARMCondition condARM )
{
	uint16_t	D = (uint16_t) sregDst & 0x01 ;
	uint16_t	Vd = (uint16_t) (sregDst >> 1) & 0x0F ;
	uint16_t	M = (uint16_t) sregSrc & 0x01 ;
	uint16_t	Vm = (uint16_t) (sregSrc >> 1) & 0x0F ;
	if ( m_modeThumb )
	{
		if ( condARM != cond_AL )
		{
			// B imm8  (ARMv4T)
			PreserveContinuousCodes( 0x10 ) ;
			uint16_t	code[1] ;
			code[0] = 0xD000 | (uint16_t) ((condARM ^ 0x01) << 8)
								| (uint16_t) (((6 - 4) >> 1) & 0xFF) ;
			m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(uint16_t) ) ;
		}
		uint16_t	code[2] ;
		code[0] = 0xEEB0 | (D << 6) ;
		code[0] = 0x0A40 | (Vd << 12) | (M << 5) | Vm ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		uint32_t	code[1] ;
		code[0] = 0x0EB00A40 | (condARM << 28)
					| ((uint32_t) D << 22)
					| ((uint32_t) Vd << 12) | (M << 5) | Vm ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// vmov.f64 Dd, Dm
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteMoveVFP64
		( int vregDst, int vregSrc, ARMCondition condARM )
{
	uint16_t	D = (uint16_t) (vregDst >> 4) & 0x01 ;
	uint16_t	Vd = (uint16_t) vregDst & 0x0F ;
	uint16_t	M = (uint16_t) (vregSrc >> 4) & 0x01 ;
	uint16_t	Vm = (uint16_t) vregSrc & 0x0F ;
	if ( m_modeThumb )
	{
		if ( condARM != cond_AL )
		{
			// B imm8  (ARMv4T)
			PreserveContinuousCodes( 0x10 ) ;
			uint16_t	code[1] ;
			code[0] = 0xD000 | (uint16_t) ((condARM ^ 0x01) << 8)
								| (uint16_t) (((6 - 4) >> 1) & 0xFF) ;
			m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(uint16_t) ) ;
		}
		uint16_t	code[2] ;
		code[0] = 0xEEB0 | (D << 6) ;
		code[0] = 0x0B40 | (Vd << 12) | (M << 5) | Vm ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		uint32_t	code[1] ;
		code[0] = 0x0EB00B40 | (condARM << 28)
					| ((uint32_t) D << 22)
					| ((uint32_t) Vd << 12) | (M << 5) | Vm ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// vmov Sm, Rt
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteMoveARMtoVFP32( int sreg, ARMRegister reg )
{
	uint16_t	N = sreg & 0x01 ;
	uint16_t	Vn = (sreg >> 1) & 0x0F ;
	if ( m_modeThumb )
	{
		uint16_t	code[2] ;
		code[0] = 0xEE00 | Vn ;
		code[0] = 0x0A10 | (reg << 12) | (N << 7) ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		uint32_t	code[1] ;
		code[0] = 0xEE000A10
				| (((uint32_t)Vn) << 16) | (reg << 12) | (N << 7) ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// vmov Rt, Sm
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteMoveVFPtoARM32( ARMRegister reg, int sreg )
{
	uint16_t	N = sreg & 0x01 ;
	uint16_t	Vn = (sreg >> 1) & 0x0F ;
	if ( m_modeThumb )
	{
		uint16_t	code[2] ;
		code[0] = 0xEE10 | Vn ;
		code[0] = 0x0A10 | (reg << 12) | (N << 7) ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		uint32_t	code[1] ;
		code[0] = 0xEE100A10
				| (((uint32_t)Vn) << 16) | (reg << 12) | (N << 7) ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// vmov Dm, Rt, Rt2
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteMoveARMtoVFP64
	( int vreg, ARMRegister regLow, ARMRegister regHigh )
{
	uint16_t	M = (uint16_t) (vreg >> 4) & 0x01 ;
	uint16_t	Vm = (uint16_t) vreg & 0x0F ;
	if ( m_modeThumb )
	{
		uint16_t	code[2] ;
		code[0] = 0xEC40 | regHigh ;
		code[0] = 0x0B10 | (regLow << 12) | (M << 5) | Vm ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		uint32_t	code[1] ;
		code[0] = 0xEC400B10 | ((uint32_t) regHigh << 16)
						| (regLow << 12) | (M << 5) | Vm ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// vmov Rt, Rt2, Dm
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteMoveVFPtoARM64
	( ARMRegister regLow, ARMRegister regHigh, int vreg )
{
	uint16_t	M = (uint16_t) (vreg >> 4) & 0x01 ;
	uint16_t	Vm = (uint16_t) vreg & 0x0F ;
	if ( m_modeThumb )
	{
		uint16_t	code[2] ;
		code[0] = 0xEC50 | regHigh ;
		code[0] = 0x0B10 | (regLow << 12) | (M << 5) | Vm ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		uint32_t	code[1] ;
		code[0] = 0xEC500B10 | ((uint32_t) regHigh << 16)
						| (regLow << 12) | (M << 5) | Vm ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// vcvt.f64.f32 Dd, Sm
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteCvtVFP32to64( int vreg, int sreg )
{
	uint16_t	D = (uint16_t) (vreg >> 4) & 0x01 ;
	uint16_t	Vd = (uint16_t) vreg & 0x0F ;
	uint16_t	M = (uint16_t) sreg & 0x01 ;
	uint16_t	Vm = (uint16_t) (sreg >> 1) & 0x0F ;
	if ( m_modeThumb )
	{
		uint16_t	code[2] ;
		code[0] = 0xEEB7 | (D << 6) ;
		code[0] = 0x0AC0 | (Vd << 12) | (M << 5) | Vm ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		uint32_t	code[1] ;
		code[0] = 0xEEB70AC0 | ((uint32_t) D << 22)
						| (Vd << 12) | (M << 5) | Vm ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// vcvt.f32.f64 Sd, Dm
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteCvtVFP64to32( int sreg, int vreg )
{
	uint16_t	D = (uint16_t) sreg & 0x01 ;
	uint16_t	Vd = (uint16_t) (sreg >> 1) & 0x0F ;
	uint16_t	M = (uint16_t) (vreg >> 4) & 0x01 ;
	uint16_t	Vm = (uint16_t) vreg & 0x0F ;
	if ( m_modeThumb )
	{
		uint16_t	code[2] ;
		code[0] = 0xEEB7 | (D << 6) ;
		code[0] = 0x0BC0 | (Vd << 12) | (M << 5) | Vm ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		uint32_t	code[1] ;
		code[0] = 0xEEB70BC0 | ((uint32_t) D << 22)
						| (Vd << 12) | (M << 5) | Vm ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// vcvt.s32.f64 Sd, Dm
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteCvtVFPtoInt32
	( int sreg, int vreg, bool fDoubleFloat,
				bool fUnsignedInt, bool fRoundZero )
{
	uint16_t	D, Vd, M, Vm, opc2, sz, op ;
	if ( fDoubleFloat )
	{
		M = (uint16_t) (vreg >> 4) & 0x01 ;
		Vm = (uint16_t) vreg & 0x0F ;
		sz = 1 ;
	}
	else
	{
		M = (uint16_t) vreg & 0x01 ;
		Vm = (uint16_t) (vreg >> 1) & 0x0F ;
		sz = 0 ;
	}
	D = sreg & 0x01 ;
	Vd = (sreg >> 1) & 0x0F ;
	op = fRoundZero ? 1 : 0 ;
	opc2 = 0x04 ;
	if ( !fUnsignedInt )
	{
		opc2 |= 0x01 ;
	}
	//
	if ( m_modeThumb )
	{
		uint16_t	code[2] ;
		code[0] = 0xEEB8 | (D << 6) | opc2 ;
		code[0] = 0x0A40 | (Vd << 12) | (sz << 8) | (op << 7) | (M << 5) | Vm ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		uint32_t	code[1] ;
		code[0] = 0xEEB80A40
				| ((uint32_t) D << 22)
				| ((uint32_t) opc2 << 16)
				| (Vd << 12) | (sz << 8) | (op << 7) | (M << 5) | Vm ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// vcvt.f64.s32 Dd, Sm
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteCvtVFPInt32toFloat
	( int vreg, int sreg, bool fDoubleFloat, bool fUnsignedInt )
{
	uint16_t	D, Vd, M, Vm, opc2, sz, op ;
	if ( fDoubleFloat )
	{
		D = (uint16_t) (vreg >> 4) & 0x01 ;
		Vd = (uint16_t) vreg & 0x0F ;
		sz = 1 ;
	}
	else
	{
		D = (uint16_t) vreg & 0x01 ;
		Vd = (uint16_t) (vreg >> 1) & 0x0F ;
		sz = 0 ;
	}
	M = sreg & 0x01 ;
	Vm = (sreg >> 1) & 0x0F ;
	op = fUnsignedInt ? 0 : 1 ;
	opc2 = 0 ;
	//
	if ( m_modeThumb )
	{
		uint16_t	code[2] ;
		code[0] = 0xEEB8 | (D << 6) | opc2 ;
		code[0] = 0x0A40 | (Vd << 12) | (sz << 8) | (op << 7) | (M << 5) | Vm ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		uint32_t	code[1] ;
		code[0] = 0xEEB80A40
				| ((uint32_t) D << 22)
				| ((uint32_t) opc2 << 16)
				| (Vd << 12) | (sz << 8) | (op << 7) | (M << 5) | Vm ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// vneg.{f64|f32} Dd, Dm
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteNegVFPRegReg
	( int vregDst, int vregSrc, bool fDoubleFlag )
{
	uint16_t	D, Vd, M, Vm, sz ;
	if ( fDoubleFlag )
	{
		D = (uint16_t) (vregDst >> 4) & 0x01 ;
		Vd = (uint16_t) vregDst & 0x0F ;
		M = (uint16_t) (vregSrc >> 4) & 0x01 ;
		Vm = (uint16_t) vregSrc & 0x0F ;
		sz = 1 ;
	}
	else
	{
		D = (uint16_t) vregDst & 0x01 ;
		Vd = (uint16_t) (vregDst >> 1) & 0x0F ;
		M = (uint16_t) vregSrc & 0x01 ;
		Vm = (uint16_t) (vregSrc >> 1) & 0x0F ;
		sz = 0 ;
	}
	if ( m_modeThumb )
	{
		uint16_t	code[2] ;
		code[0] = (uint16_t) ((thumbOpFNEG >> 16) & 0xFFFF) | (D << 6) ;
		code[0] = (uint16_t) (thumbOpFNEG & 0xFFFF)
						| (Vd << 12) | (sz << 8) | (M << 5) | Vm ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		uint32_t	code[1] ;
		code[0] = armOpFNEG | (cond_AL << 28)
					| ((uint32_t) D << 22)
					| (Vd << 12) | (sz << 8) | (M << 5) | Vm ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// vcmp.{f64|f32} Dd, Dm
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteCmpVFPRegReg
	( int vregDst, int vregSrc, bool fDoubleFlag )
{
	WriteVFPOpRegReg( armOpFCMP, thumbOpFCMP, vregDst, vregSrc, fDoubleFlag ) ;
}

// vmrs Rt, FPSCR
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteFPSCRtoARMReg( ARMRegister regDst )
{
	if ( m_modeThumb )
	{
		uint16_t	code[2] ;
		code[0] = (uint16_t) ((thumbOpVMRS >> 16) & 0xFFFF) ;
		code[0] = (uint16_t) (thumbOpVMRS & 0xFFFF) | (regDst << 12) ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		uint32_t	code[1] ;
		code[0] = armOpVMRS | (cond_AL << 28) | (regDst << 12) ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// op reg, reg, reg
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteSIMDOpRegRegReg
	( uint32_t armOpCode, uint32_t thumbOpCode,
		int vregDst, int vregSrc1, int vregSrc2, bool fQuadFlag )
{
	uint16_t	D = (uint16_t) (vregDst >> 4) & 0x01 ;
	uint16_t	Vd = (uint16_t) vregDst & 0x0F ;
	uint16_t	N = (uint16_t) (vregSrc1 >> 4) & 0x01 ;
	uint16_t	Vn = (uint16_t) vregSrc1 & 0x0F ;
	uint16_t	M = (uint16_t) (vregSrc2 >> 4) & 0x01 ;
	uint16_t	Vm = (uint16_t) vregSrc2 & 0x0F ;
	uint16_t	Q = fQuadFlag ? 1 : 0 ;
	if ( m_modeThumb )
	{
		uint16_t	code[2] ;
		code[0] = ((thumbOpCode >> 16) & 0xFFFF) | (D << 6) | Vn ;
		code[0] = (thumbOpCode & 0xFFFF) | (Vd << 12)
						| (N << 7) | (Q << 6) | (M << 5) | Vm ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		uint32_t	code[1] ;
		code[0] = armOpCode | ((uint32_t) D << 22)
					| ((uint32_t) Vn << 16)
					| ((uint32_t) Vd << 12)
					| (N << 7) | (Q << 6) | (M << 5) | Vm ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// op reg, reg, reg
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteSIMDIntOpRegRegReg
	( uint32_t armOpCode, uint32_t thumbOpCode,
		int vregDst, int vregSrc1, int vregSrc2,
		int nLog2Size, bool fQuadFlag, bool fUnsigned )
{
	uint16_t	D = (uint16_t) (vregDst >> 4) & 0x01 ;
	uint16_t	Vd = (uint16_t) vregDst & 0x0F ;
	uint16_t	N = (uint16_t) (vregSrc1 >> 4) & 0x01 ;
	uint16_t	Vn = (uint16_t) vregSrc1 & 0x0F ;
	uint16_t	M = (uint16_t) (vregSrc2 >> 4) & 0x01 ;
	uint16_t	Vm = (uint16_t) vregSrc2 & 0x0F ;
	uint16_t	Q = fQuadFlag ? 1 : 0 ;
	uint16_t	U = fUnsigned ? 1 : 0 ;
	nLog2Size &= 0x03 ;
	if ( m_modeThumb )
	{
		uint16_t	code[2] ;
		code[0] = ((thumbOpCode >> 16) & 0xFFFF)
						| (U << 12) | (D << 6) | (nLog2Size << 4) | Vn ;
		code[0] = (thumbOpCode & 0xFFFF) | (Vd << 12)
						| (N << 7) | (Q << 6) | (M << 5) | Vm ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		uint32_t	code[1] ;
		code[0] = armOpCode
					| ((uint32_t) U << 24)
					| ((uint32_t) D << 22) | (nLog2Size << 20)
					| ((uint32_t) Vn << 16)
					| ((uint32_t) Vd << 12)
					| (N << 7) | (Q << 6) | (M << 5) | Vm ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// vmul reg, reg, reg
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteSIMDIntMulRegRegReg
	( int vregDst, int vregSrc1, int vregSrc2,
								int nLog2Size, bool fQuadFlag )
{
	WriteSIMDIntOpRegRegReg
		( armOpVMUL, thumbOpVMUL,
			vregDst, vregSrc1, vregSrc2, nLog2Size, fQuadFlag ) ;
}

// vmull reg, reg, reg
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteSIMDIntMulLongRegRegReg
	( int qregDst, int vregSrc1, int vregSrc2,
					int nLog2Size, bool fUnsigned )
{
	WriteSIMDIntOpRegRegReg
		( armOpVMULL, thumbOpVMULL,
			(qregDst << 1), vregSrc1, vregSrc2,
						nLog2Size, false, fUnsigned ) ;
}

// vmvn reg, reg
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteSIMDIntNotRegReg
	( int vregDst, int vregSrc, int nLog2Size, bool fQuadFlag )
{
	uint16_t	D = (uint16_t) (vregDst >> 4) & 0x01 ;
	uint16_t	Vd = (uint16_t) vregDst & 0x0F ;
	uint16_t	M = (uint16_t) (vregSrc >> 4) & 0x01 ;
	uint16_t	Vm = (uint16_t) vregSrc & 0x0F ;
	uint16_t	Q = fQuadFlag ? 1 : 0 ;
	nLog2Size = typeNEONInt8 ;
	if ( m_modeThumb )
	{
		uint16_t	code[2] ;
		code[0] = ((thumbOpVMVN >> 16) & 0xFFFF)
						| (D << 6) | (nLog2Size << 2) ;
		code[0] = (thumbOpVMVN & 0xFFFF) | (Vd << 12)
						| (Q << 6) | (M << 5) | Vm ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		uint32_t	code[1] ;
		code[0] = armOpVMVN
					| ((uint32_t) D << 22) | (nLog2Size << 18)
					| ((uint32_t) Vd << 12)
					| (Q << 6) | (M << 5) | Vm ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// vneg reg, reg
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteSIMDIntNegRegReg
	( int vregDst, int vregSrc,
		int nLog2Size, bool fFloatFlag, bool fQuadFlag )
{
	uint16_t	D = (uint16_t) (vregDst >> 4) & 0x01 ;
	uint16_t	Vd = (uint16_t) vregDst & 0x0F ;
	uint16_t	M = (uint16_t) (vregSrc >> 4) & 0x01 ;
	uint16_t	Vm = (uint16_t) vregSrc & 0x0F ;
	uint16_t	Q = fQuadFlag ? 1 : 0 ;
	uint16_t	F = fFloatFlag ? 1 : 0 ;
	nLog2Size &= 0x03 ;
	ESLAssert( nLog2Size != 0x03 ) ;
	if ( m_modeThumb )
	{
		uint16_t	code[2] ;
		code[0] = ((thumbOpVINEG >> 16) & 0xFFFF)
						| (D << 6) | (nLog2Size << 2) ;
		code[0] = (thumbOpVINEG & 0xFFFF) | (Vd << 12)
						| (F << 10) | (Q << 6) | (M << 5) | Vm ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		uint32_t	code[1] ;
		code[0] = armOpVINEG
					| ((uint32_t) D << 22) | (nLog2Size << 18)
					| ((uint32_t) Vd << 12)
					| (F << 10) | (Q << 6) | (M << 5) | Vm ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// shift reg, reg, imm
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteSIMDShiftRegRegImm
	( uint32_t armOpCode, uint32_t thumbOpCode,
		int vregDst, int vregSrc, int imm,
		int nLog2Size, bool fSigned, bool fShiftRight, bool fQuadFlag )
{
	uint16_t	D = (uint16_t) (vregDst >> 4) & 0x01 ;
	uint16_t	Vd = (uint16_t) vregDst & 0x0F ;
	uint16_t	M = (uint16_t) (vregSrc >> 4) & 0x01 ;
	uint16_t	Vm = (uint16_t) vregSrc & 0x0F ;
	uint16_t	imm6 ;
	if ( fShiftRight )
	{
		imm6 = ((0x40 >> (3 - nLog2Size)) & 0x3F)
							| (~(imm - 1) & ((0x08 << nLog2Size) - 1)) ;
	}
	else
	{
		imm6 = ((0x40 >> (3 - nLog2Size)) & 0x3F)
							| (imm & ((0x08 << nLog2Size) - 1)) ;
	}
	uint16_t	L = (nLog2Size == 3) ? 1 : 0 ;
	uint16_t	U = fSigned ? 0 : 1 ;
	uint16_t	Q = fQuadFlag ? 1 : 0 ;
	if ( m_modeThumb )
	{
		uint16_t	code[2] ;
		code[0] = ((thumbOpCode >> 16) & 0xFFFF) | (U << 12) | (D << 6) | imm6 ;
		code[0] = (thumbOpCode & 0xFFFF) | (Vd << 12)
						| (L << 7) | (Q << 6) | (M << 5) | Vm ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		uint32_t	code[1] ;
		code[0] = armOpCode
					| ((uint32_t) U << 24)
					| ((uint32_t) D << 22)
					| ((uint32_t) imm6 << 16)
					| ((uint32_t) Vd << 12)
					| (L << 7) | (Q << 6) | (M << 5) | Vm ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// qshift reg, reg, imm
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteSIMDQShiftRegRegImm
	( int vregDst, int vregSrc, int imm,
		int nLog2Size, bool fDstSigned, bool fSrcSigned, bool fQuadFlag )
{
	uint32_t	armOpCode = armOpVQSHLImm ;
	uint32_t	thumbOpCode = thumbOpVQSHLImm ;
	if ( fSrcSigned )
	{
		if ( fDstSigned )
		{
			armOpCode |= (1 << 8) ;
			thumbOpCode |= (1 << 8) ;
		}
	}
	else
	{
		fDstSigned = false ;
	}
	WriteSIMDShiftRegRegImm
		( armOpCode, thumbOpCode,
			vregDst, vregSrc, imm, nLog2Size, fDstSigned, false, fQuadFlag ) ;
}

// vshl reg, reg, reg
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteSIMDShiftRegRegReg
	( int vregDst, int vregSrc1, int vregSrc2,
		int nLog2Size, bool fSigned, bool fQuadFlag )
{
	uint16_t	D = (uint16_t) (vregDst >> 4) & 0x01 ;
	uint16_t	Vd = (uint16_t) vregDst & 0x0F ;
	uint16_t	M = (uint16_t) (vregSrc1 >> 4) & 0x01 ;
	uint16_t	Vm = (uint16_t) vregSrc1 & 0x0F ;
	uint16_t	N = (uint16_t) (vregSrc2 >> 4) & 0x01 ;
	uint16_t	Vn = (uint16_t) vregSrc2 & 0x0F ;
	uint16_t	U = fSigned ? 0 : 1 ;
	uint16_t	Q = fQuadFlag ? 1 : 0 ;
	if ( m_modeThumb )
	{
		uint16_t	code[2] ;
		code[0] = ((thumbOpVSHL >> 16) & 0xFFFF) | (U << 12)
						| (D << 6) | ((nLog2Size & 0x03) << 4) | vregSrc2 ;
		code[0] = (thumbOpVSHL & 0xFFFF) | (Vd << 12)
						| (N << 7) | (Q << 6) | (M << 5) | Vm ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		uint32_t	code[1] ;
		code[0] = armOpVSHL
					| ((uint32_t) U << 24)
					| ((uint32_t) D << 22)
					| ((nLog2Size & 0x03) << 20)
					| ((uint32_t) Vn << 16)
					| ((uint32_t) Vd << 12)
					| (N << 7) | (Q << 6) | (M << 5) | Vm ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// vrev reg, reg
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteSIMDRevRegReg
	( int vregDst, int vregSrc,
		int nLog2Size, int nLog2ElSize, bool fQuadFlag )
{
	uint16_t	D = (uint16_t) (vregDst >> 4) & 0x01 ;
	uint16_t	Vd = (uint16_t) vregDst & 0x0F ;
	uint16_t	M = (uint16_t) (vregSrc >> 4) & 0x01 ;
	uint16_t	Vm = (uint16_t) vregSrc & 0x0F ;
	uint16_t	Q = fQuadFlag ? 1 : 0 ;
	ESLAssert( nLog2Size > nLog2ElSize ) ;
	uint16_t	nSize = nLog2ElSize & 0x03 ;
	uint16_t	op = (nLog2Size + 1) & 0x03 ;
	if ( m_modeThumb )
	{
		uint16_t	code[2] ;
		code[0] = ((thumbOpVREV >> 16) & 0xFFFF)
						| (D << 6) | (nSize << 2) ;
		code[0] = (thumbOpVREV & 0xFFFF) | (Vd << 12)
						| (op << 7) | (Q << 6) | (M << 5) | Vm ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		uint32_t	code[1] ;
		code[0] = armOpVREV
					| ((uint32_t) D << 22)
					| (nSize << 18) | ((uint32_t) Vd << 12)
					| (op << 7) | (Q << 6) | (M << 5) | Vm ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// vdup.s32 Dd, Rt
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteDupARM32toVFP
			( int vreg, ARMRegister regARM, bool fQuadFlag )
{
	uint16_t	D, Vd, Q ;
	D = (vreg >> 4) & 0x01 ;
	Vd = (vreg & 0x0F) ;
	Q = fQuadFlag ? 1 : 0 ;
	//
	if ( m_modeThumb )
	{
		uint16_t	code[2] ;
		code[0] = 0xEE80 | (Q << 5) | Vd ;
		code[0] = 0x0B10 | (regARM << 12) | (D << 7) ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		uint32_t	code[1] ;
		code[0] = 0xEE800B10
				| ((uint32_t) Q << 21)
				| ((uint32_t) Vd << 16)
				| (regARM << 12) | (D << 7) ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// vdup reg, reg, imm
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteSIMDDupRegRegImm
	( int vregDst, int vregSrc, int imm, int nLog2Size, bool fQuadFlag )
{
	uint16_t	D = (uint16_t) (vregDst >> 4) & 0x01 ;
	uint16_t	Vd = (uint16_t) vregDst & 0x0F ;
	uint16_t	M = (uint16_t) (vregSrc >> 4) & 0x01 ;
	uint16_t	Vm = (uint16_t) vregSrc & 0x0F ;
	uint16_t	Q = fQuadFlag ? 1 : 0 ;
	uint16_t	imm4 = (((imm << 1) | 1) << nLog2Size) & 0x0F ;
	if ( m_modeThumb )
	{
		uint16_t	code[2] ;
		code[0] = ((thumbOpVDUPImm >> 16) & 0xFFFF) | (D << 6) | imm4 ;
		code[0] = (thumbOpVDUPImm & 0xFFFF)
							| (Vd << 12) | (Q << 6) | (M << 5) | Vm ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		uint32_t	code[1] ;
		code[0] = armOpVDUPImm
					| ((uint32_t) D << 22)
					| ((uint32_t) imm4 << 16)
					| (Vd << 12) | (Q << 6) | (M << 5) | Vm ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// vext reg, reg, reg, imm
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteSIMDExtRegRegRegImm
	( int vregDst, int vregSrc1, int vregSrc2, int imm4, bool fQuadFlag )
{
	uint16_t	D = (uint16_t) (vregDst >> 4) & 0x01 ;
	uint16_t	Vd = (uint16_t) vregDst & 0x0F ;
	uint16_t	N = (uint16_t) (vregSrc1 >> 4) & 0x01 ;
	uint16_t	Vn = (uint16_t) vregSrc1 & 0x0F ;
	uint16_t	M = (uint16_t) (vregSrc2 >> 4) & 0x01 ;
	uint16_t	Vm = (uint16_t) vregSrc2 & 0x0F ;
	uint16_t	Q = fQuadFlag ? 1 : 0 ;
	imm4 &= 0x0F ;
	if ( m_modeThumb )
	{
		uint16_t	code[2] ;
		code[0] = ((thumbOpVEXTImm >> 16) & 0xFFFF) | (D << 6) | Vn ;
		code[0] = (thumbOpVEXTImm & 0xFFFF) | (Vd << 12)
					| (imm4 << 8) | (N << 7) | (Q << 6) | (M << 5) | Vm ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		uint32_t	code[1] ;
		code[0] = armOpVEXTImm
					| ((uint32_t) D << 22)
					| ((uint32_t) Vn << 16)
					| (Vd << 12) | (imm4 << 8)
					| (N << 7) | (Q << 6) | (M << 5) | Vm ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

// op reg, reg
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteSIMDIntOpRegReg
	( uint32_t armOpCode, uint32_t thumbOpCode,
		int vregDst, int vregSrc, int nLog2Size, bool fQuadFlag )
{
	uint16_t	D = (uint16_t) (vregDst >> 4) & 0x01 ;
	uint16_t	Vd = (uint16_t) vregDst & 0x0F ;
	uint16_t	M = (uint16_t) (vregSrc >> 4) & 0x01 ;
	uint16_t	Vm = (uint16_t) vregSrc & 0x0F ;
	uint16_t	Q = fQuadFlag ? 1 : 0 ;
	nLog2Size &= 0x03 ;
	if ( m_modeThumb )
	{
		uint16_t	code[2] ;
		code[0] = ((thumbOpCode >> 16) & 0xFFFF)
						| (D << 6) | (nLog2Size << 2) ;
		code[0] = (thumbOpCode & 0xFFFF)
						| (Vd << 12) | (Q << 6) | (M << 5) | Vm ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
	else
	{
		uint32_t	code[1] ;
		code[0] = armOpCode
					| ((uint32_t) D << 22)
					| (nLog2Size << 18)
					| (Vd << 12) | (Q << 6) | (M << 5) | Vm ;
		m_buf->WriteInstruction( (BYTE*) &code[0], sizeof(code) ) ;
	}
}

void ARMGenericAssembler::WriteSIMDFloatOpRegReg
	( uint32_t armOpCode, uint32_t thumbOpCode,
				int vregDst, int vregSrc, bool fQuadFlag )
{
	WriteSIMDIntOpRegReg
		( armOpCode, thumbOpCode, vregDst, vregSrc, 0, fQuadFlag ) ;
}

// vtrn reg, reg
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteSIMDTrnRegReg
	( int vregDst, int vregSrc, int nLog2Size, bool fQuadFlag )
{
	WriteSIMDIntOpRegReg
		( armOpVTRN, thumbOpVTRN, vregDst, vregSrc, nLog2Size, fQuadFlag ) ;
}

// vzip reg, reg
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteSIMDZipRegReg
	( int vregDst, int vregSrc, int nLog2Size, bool fQuadFlag )
{
	if ( (nLog2Size == typeNEONInt32) && !fQuadFlag )
	{
		WriteSIMDIntOpRegReg
			( armOpVTRN, thumbOpVTRN, vregDst, vregSrc, nLog2Size, fQuadFlag ) ;
	}
	else
	{
		WriteSIMDIntOpRegReg
			( armOpVZIP, thumbOpVZIP, vregDst, vregSrc, nLog2Size, fQuadFlag ) ;
	}
}

// vuzp reg, reg
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteSIMDUnZipRegReg
	( int vregDst, int vregSrc, int nLog2Size, bool fQuadFlag )
{
	if ( (nLog2Size == typeNEONInt32) && !fQuadFlag )
	{
		WriteSIMDIntOpRegReg
			( armOpVTRN, thumbOpVTRN, vregDst, vregSrc, nLog2Size, fQuadFlag ) ;
	}
	else
	{
		WriteSIMDIntOpRegReg
			( armOpVUZP, thumbOpVUZP, vregDst, vregSrc, nLog2Size, fQuadFlag ) ;
	}
}

// vswp regm reg
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteSIMDSwapRegReg
	( int vregDst, int vregSrc, bool fQuadFlag )
{
	WriteSIMDIntOpRegReg
		( armOpVSWP, thumbOpVSWP, vregDst, vregSrc, 0, fQuadFlag ) ;
}

// vmov Qd, Qm
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteMoveVFP128( int qregDst, int qregSrc )
{
	WriteSIMDOpRegRegReg
		( armOpVMOV, thumbOpVMOV,
			(qregDst << 1), (qregSrc << 1), (qregSrc << 1), true ) ;
}


// Sakura2 レジスタを物理レジスタに割り当て／ロード
//////////////////////////////////////////////////////////////////////////////
int ARMGenericAssembler::WriteRealizeDataRegister
	( int regSakura, ARMGenericAssembler::DataRegisterClass regClass, bool fLoad )
{
	//
	// 割り当て済み物理レジスタを取得する
	//
	int	regPhy = GetRealizedDataRegister( regSakura, regClass, fLoad ) ;
	if ( regPhy != ARM_Nothing )
	{
		return	regPhy ;
	}
	//
	// 新規に物理レジスタを割り当てる
	//
	regPhy = AllocateDataRegister( regClass ) ;
	m_lruDataReg.dprSakura[regSakura].regClass = regClass ;
	m_lruDataReg.dprSakura[regSakura].regPhy = regPhy ;
	//
	int	iSlot, regOddSakura ;
	switch ( regClass )
	{
	case	regClassARM:
		iSlot = (regPhy >> 1) ;
		ESLAssert( (iSlot >= 0) & (iSlot < 3) ) ;
		ESLAssert( m_lruDataReg.lruRegARM.IsSlotAssigned(iSlot) ) ;
		ESLAssert( m_lruDataReg.lruRegARM[iSlot].regPhy == regPhy ) ;
		m_lruDataReg.lruRegARM[iSlot].regSakura = regSakura ;
		if ( fLoad )
		{
			const size_t
				offsetReg = Context::OffsetOfReg(regSakura) ;
			if ( offsetReg <= 0xFF )
			{
				WriteARMLoadDoubleMemOffsetImm8
					( (ARMRegister) regPhy, ARM_r10, offsetReg ) ;
			}
			else
			{
				WriteARMLoadMemOffsetImm12
					( (ARMRegister) regPhy, ARM_r10, offsetReg ) ;
				WriteARMLoadMemOffsetImm12
					( (ARMRegister) (regPhy + 1), ARM_r10, offsetReg + 4 ) ;
			}
		}
		break ;

	case	regClassVFP:
		iSlot = regPhy ;
		ESLAssert( (iSlot >= 0) & (iSlot < 0x10) ) ;
		ESLAssert( m_lruDataReg.lruRegVFP.IsSlotAssigned(iSlot) ) ;
		ESLAssert( m_lruDataReg.lruRegVFP[iSlot].regPhy == regPhy ) ;
		m_lruDataReg.lruRegVFP[iSlot].regSakura = regSakura ;
		if ( fLoad )
		{
			const size_t
				offsetReg = Context::OffsetOfReg(regSakura) ;
			if ( offsetReg <= 0x3FC )
			{
				WriteVFPLoad64OffsetImm8( regPhy, ARM_r10, offsetReg ) ;
			}
			else
			{
				PreserveContinuousCodes( 0x20 ) ;
				WriteARMLeaSakura2Register( ARM_r6, regSakura ) ;
				WriteVFPLoad64OffsetImm8( regPhy, ARM_r6, 0 ) ;
			}
		}
		break ;

	case	regClassNEON:
		ESLAssert( !(regSakura & 0x01) ) ;
		regOddSakura = regSakura ^ 0x01 ;
		if ( (m_lruDataReg.dprSakura[regOddSakura].regClass != regClassNothing)
			& (m_lruDataReg.dprSakura[regOddSakura].regPhy != -1) )
		{
			DataRegisterClass
				regClassOdd = m_lruDataReg.dprSakura[regOddSakura].regClass ;
			int	regPhyOdd = m_lruDataReg.dprSakura[regOddSakura].regPhy ;
			ESLAssert( regClassOdd != regClassNEON ) ;
			if ( fLoad )
			{
				WriteBackDataRegister( regClassOdd, regPhyOdd ) ;
			}
			FreeDataRegister( regClassOdd, regPhyOdd ) ;
		}
		iSlot = regPhy - 8 ;
		ESLAssert( (iSlot >= 0) & (iSlot < 8) ) ;
		ESLAssert( m_lruDataReg.lruRegNEON.IsSlotAssigned(iSlot) ) ;
		ESLAssert( m_lruDataReg.lruRegNEON[iSlot].regPhy == regPhy ) ;
		m_lruDataReg.lruRegNEON[iSlot].regSakura = regSakura ;
		if ( fLoad )
		{
			const size_t
				offsetReg = Context::OffsetOfReg(regSakura+1) ;
			if ( offsetReg <= 0x3FC )
			{
				WriteVFPLoad64OffsetImm8( (regPhy << 1), ARM_r10, offsetReg - 8 ) ;
				WriteVFPLoad64OffsetImm8( (regPhy << 1) + 1, ARM_r10, offsetReg ) ;
			}
			else
			{
				PreserveContinuousCodes( 0x40 ) ;
				WriteARMLeaSakura2Register( ARM_r6, regSakura ) ;
				WriteVFPLoad64OffsetImm8( (regPhy << 1), ARM_r6, 0 ) ;
				WriteVFPLoad64OffsetImm8( (regPhy << 1) + 1, ARM_r6, 8 ) ;
			}
		}
		m_lruDataReg.dprSakura[regOddSakura].regClass = regClass ;
		m_lruDataReg.dprSakura[regOddSakura].regPhy = regPhy ;
		break ;

	default:
		break ;
	}
	return	regPhy ;
}

// Sakura2 レジスタを割り当て済みの物理レジスタを取得
// 取得データ型によっては正規化
//////////////////////////////////////////////////////////////////////////////
int ARMGenericAssembler::GetRealizedDataRegister
	( int regSakura, ARMGenericAssembler::DataRegisterClass regClass, bool fNormalize )
{
	ESLAssert( (regSakura >= 0) && (regSakura < 0x100) ) ;
	regSakura &= 0xFF ;
	//
	DataRegisterClass
		classLoaded = m_lruDataReg.dprSakura[regSakura].regClass ;
	int	regPhy = m_lruDataReg.dprSakura[regSakura].regPhy ;
	if ( (classLoaded == regClassNothing) | (regPhy == -1) )
	{
		ESLAssert( (classLoaded == regClassNothing) & (regPhy == -1) ) ;
		return	-1 ;
	}
	if ( classLoaded == regClass )
	{
		#if	defined(__DEBUG__)
		int	iSlot ;
		switch ( classLoaded )
		{
		case	regClassARM:
			iSlot = (regPhy >> 1) ;
			ESLAssert( (iSlot >= 0) & (iSlot < 3) ) ;
			ESLAssert( m_lruDataReg.lruRegARM.IsSlotAssigned(iSlot) ) ;
			ESLAssert( m_lruDataReg.lruRegARM[iSlot].regSakura == regSakura ) ;
			break ;
		case	regClassVFP:
			iSlot = regPhy ;
			ESLAssert( (iSlot >= 0) & (iSlot < 0x10) ) ;
			ESLAssert( m_lruDataReg.lruRegVFP.IsSlotAssigned(iSlot) ) ;
			ESLAssert( m_lruDataReg.lruRegVFP[iSlot].regSakura == regSakura ) ;
			break ;
		case	regClassNEON:
			iSlot = regPhy - 8 ;
			ESLAssert( (iSlot >= 0) & (iSlot < 8) ) ;
			ESLAssert( m_lruDataReg.lruRegNEON.IsSlotAssigned(iSlot) ) ;
			ESLAssert( m_lruDataReg.lruRegNEON[iSlot].regSakura == regSakura ) ;
			break ;
		default:
			break ;
		}
		#endif
		LockDataRegister( regClass, regPhy ) ;
		return	regPhy ;
	}
	int	iSlot, regPhyNew ;
	switch ( regClass )
	{
	case	regClassARM:
		regPhyNew = AllocateDataRegister( regClassARM ) ;
		iSlot = (regPhyNew >> 1) ;
		if ( RealizeFreeARMRegister
				( regPhyNew, regSakura, fNormalize, true ) )
		{
			m_lruDataReg.lruRegARM.SetModified( iSlot ) ;
		}
		m_lruDataReg.lruRegARM[iSlot].regSakura = regSakura ;
		m_lruDataReg.dprSakura[regSakura].regClass = regClass ;
		m_lruDataReg.dprSakura[regSakura].regPhy = regPhyNew ;
		return	regPhyNew ;

	case	regClassVFP:
		regPhyNew = AllocateDataRegister( regClassVFP ) ;
		iSlot = regPhyNew ;
		if ( RealizeFreeVFPRegister
				( regPhyNew, regSakura, fNormalize, true ) )
		{
			m_lruDataReg.lruRegVFP.SetModified( iSlot ) ;
		}
		m_lruDataReg.lruRegVFP[iSlot].regSakura = regSakura ;
		m_lruDataReg.dprSakura[regSakura].regClass = regClass ;
		m_lruDataReg.dprSakura[regSakura].regPhy = regPhyNew ;
		return	regPhyNew ;

	case	regClassNEON:
		regPhyNew = AllocateDataRegister( regClassNEON ) ;
		iSlot = regPhyNew - 8 ;
		if ( RealizeFreeNEONRegister
				( regPhyNew, regSakura, fNormalize, true ) )
		{
			m_lruDataReg.lruRegNEON.SetModified( iSlot ) ;
		}
		m_lruDataReg.lruRegNEON[iSlot].regSakura = regSakura ;
		m_lruDataReg.dprSakura[regSakura].regClass = regClass ;
		m_lruDataReg.dprSakura[regSakura].regPhy = regPhyNew ;
		m_lruDataReg.dprSakura[regSakura^0x01].regClass = regClass ;
		m_lruDataReg.dprSakura[regSakura^0x01].regPhy = regPhyNew ;
		return	regPhyNew ;

	default:
		break ;
	}
	ESLAssert( regClass == regClassNothing ) ;
	return	-1 ;
}

// Sakura2 レジスタを指定 ARM レジスタにロードし、以前の割り当ては解除する
//////////////////////////////////////////////////////////////////////////////
bool ARMGenericAssembler::RealizeFreeARMRegister
		( int regDst, int regSakura, bool fLoad, bool fFree )
{
	DataRegisterClass
			classLoaded = m_lruDataReg.dprSakura[regSakura].regClass ;
	int		regPhy = m_lruDataReg.dprSakura[regSakura].regPhy ;
	int		iSlot ;
	bool	fModified = false ;
	switch ( classLoaded )
	{
	case	regClassARM:
		iSlot = (regPhy >> 1) ;
		ESLAssert( (iSlot >= 0) & (iSlot < 3) ) ;
		ESLAssert( m_lruDataReg.lruRegARM.IsSlotAssigned(iSlot) ) ;
		ESLAssert( m_lruDataReg.lruRegARM[iSlot].regPhy == regPhy ) ;
		if ( fFree )
		{
			fModified = m_lruDataReg.lruRegARM[iSlot].fModified ;
			FreeDataRegister( classLoaded, regPhy ) ;
		}
		if ( fLoad )
		{
			WriteARMMoveRegReg
				( (ARMRegister) regDst, (ARMRegister) regPhy ) ;
			WriteARMMoveRegReg
				( (ARMRegister) (regDst + 1), (ARMRegister) (regPhy + 1) ) ;
		}
		break ;

	case	regClassVFP:
		iSlot = regPhy ;
		ESLAssert( (iSlot >= 0) & (iSlot < 0x10) ) ;
		ESLAssert( m_lruDataReg.lruRegVFP.IsSlotAssigned(iSlot) ) ;
		ESLAssert( m_lruDataReg.lruRegVFP[iSlot].regPhy == regPhy ) ;
		if ( fFree )
		{
			fModified = m_lruDataReg.lruRegVFP[iSlot].fModified ;
			FreeDataRegister( classLoaded, regPhy ) ;
		}
		if ( fLoad )
		{
			WriteMoveVFPtoARM64
				( (ARMRegister) regDst, (ARMRegister) (regDst + 1), regPhy ) ;
		}
		break ;

	case	regClassNEON:
		iSlot = regPhy - 8 ;
		ESLAssert( (iSlot >= 0) & (iSlot < 8) ) ;
		ESLAssert( m_lruDataReg.lruRegNEON.IsSlotAssigned(iSlot) ) ;
		ESLAssert( m_lruDataReg.lruRegNEON[iSlot].regPhy == regPhy ) ;
		if ( fFree )
		{
			if ( m_lruDataReg.lruRegNEON[iSlot].fModified )
			{
				const size_t
					offsetReg = Context::OffsetOfReg(regSakura^0x01) ;
				if ( offsetReg <= 0x3FC )
				{
					WriteVFPStore64OffsetImm8
						( (regPhy << 1) + (~regSakura & 0x01), ARM_r10, offsetReg ) ;
				}
				else
				{
					PreserveContinuousCodes( 0x20 ) ;
					WriteARMLeaSakura2Register( ARM_r6, (regSakura ^ 0x01) ) ;
					WriteVFPStore64OffsetImm8
						( (regPhy << 1) + (~regSakura & 0x01), ARM_r6, 0 ) ;
				}
				fModified = true ;
			}
			FreeDataRegister( classLoaded, regPhy ) ;
		}
		if ( fLoad )
		{
			WriteMoveVFPtoARM64
				( (ARMRegister) regDst,
					(ARMRegister) (regDst + 1),
					(regPhy << 1) + (regSakura & 0x01) ) ;
		}
		break ;

	default:
		ESLAssert( classLoaded == regClassNothing ) ;
		ESLAssert( regPhy == -1 ) ;
		if ( fLoad )
		{
			const size_t
				offsetReg = Context::OffsetOfReg(regSakura) ;
			if ( offsetReg <= 0xFF )
			{
				WriteARMLoadDoubleMemOffsetImm8
					( (ARMRegister) regDst, ARM_r10, offsetReg ) ;
			}
			else
			{
				WriteARMLoadMemOffsetImm12
					( (ARMRegister) regDst, ARM_r10, offsetReg ) ;
				WriteARMLoadMemOffsetImm12
					( (ARMRegister) (regDst + 1), ARM_r10, offsetReg + 4 ) ;
			}
		}
		break ;
	}
	return	fModified ;
}

// Sakura2 レジスタを指定 VFP にロードし、以前の割り当ては解除する
//////////////////////////////////////////////////////////////////////////////
bool ARMGenericAssembler::RealizeFreeVFPRegister
		( int vregDst, int regSakura, bool fLoad, bool fFree )
{
	DataRegisterClass
			classLoaded = m_lruDataReg.dprSakura[regSakura].regClass ;
	int		regPhy = m_lruDataReg.dprSakura[regSakura].regPhy ;
	int		iSlot ;
	bool	fModified = false ;
	switch ( classLoaded )
	{
	case	regClassARM:
		iSlot = (regPhy >> 1) ;
		ESLAssert( (iSlot >= 0) & (iSlot < 3) ) ;
		ESLAssert( m_lruDataReg.lruRegARM.IsSlotAssigned(iSlot) ) ;
		ESLAssert( m_lruDataReg.lruRegARM[iSlot].regPhy == regPhy ) ;
		if ( fFree )
		{
			fModified = m_lruDataReg.lruRegARM[iSlot].fModified ;
			FreeDataRegister( classLoaded, regPhy ) ;
		}
		if ( fLoad )
		{
			WriteMoveARMtoVFP64
				( vregDst, (ARMRegister) regPhy,
								(ARMRegister) (regPhy + 1) ) ;
		}
		break ;

	case	regClassVFP:
		iSlot = regPhy ;
		ESLAssert( (iSlot >= 0) & (iSlot < 0x10) ) ;
		ESLAssert( m_lruDataReg.lruRegVFP.IsSlotAssigned(iSlot) ) ;
		ESLAssert( m_lruDataReg.lruRegVFP[iSlot].regPhy == regPhy ) ;
		if ( fFree )
		{
			fModified = m_lruDataReg.lruRegVFP[iSlot].fModified ;
			FreeDataRegister( classLoaded, regPhy ) ;
		}
		if ( fLoad )
		{
			WriteMoveVFP64( vregDst, regPhy ) ;
		}
		break ;

	case	regClassNEON:
		iSlot = regPhy - 8 ;
		ESLAssert( (iSlot >= 0) & (iSlot < 8) ) ;
		ESLAssert( m_lruDataReg.lruRegNEON.IsSlotAssigned(iSlot) ) ;
		ESLAssert( m_lruDataReg.lruRegNEON[iSlot].regPhy == regPhy ) ;
		if ( fFree )
		{
			if ( m_lruDataReg.lruRegNEON[iSlot].fModified )
			{
				const size_t
					offsetReg = Context::OffsetOfReg(regSakura^0x01) ;
				if ( offsetReg <= 0x3FC )
				{
					WriteVFPStore64OffsetImm8
						( (regPhy << 1) + (~regSakura & 0x01), ARM_r10, offsetReg ) ;
				}
				else
				{
					PreserveContinuousCodes( 0x20 ) ;
					WriteARMLeaSakura2Register( ARM_r6, (regSakura ^ 0x01) ) ;
					WriteVFPStore64OffsetImm8
						( (regPhy << 1) + (~regSakura & 0x01), ARM_r6, 0 ) ;
				}
				fModified = true ;
			}
			FreeDataRegister( classLoaded, regPhy ) ;
		}
		if ( fLoad )
		{
			WriteMoveVFP64( vregDst, (regPhy << 1) + (regSakura & 0x01) ) ;
		}
		break ;

	default:
		ESLAssert( classLoaded == regClassNothing ) ;
		ESLAssert( regPhy == -1 ) ;
		if ( fLoad )
		{
			const size_t
				offsetReg = Context::OffsetOfReg(regSakura) ;
			if ( offsetReg <= 0x3FC )
			{
				WriteVFPLoad64OffsetImm8( vregDst, ARM_r10, offsetReg ) ;
			}
			else
			{
				PreserveContinuousCodes( 0x20 ) ;
				WriteARMLeaSakura2Register( ARM_r6, regSakura ) ;
				WriteVFPLoad64OffsetImm8( vregDst, ARM_r6, 0 ) ;
			}
		}
		break ;
	}
	return	fModified ;
}

// Sakura2 レジスタを指定 NEON レジスタにロードし、以前の割り当ては解除する
//////////////////////////////////////////////////////////////////////////////
bool ARMGenericAssembler::RealizeFreeNEONRegister
	( int qregDst, int regSakura, bool fLoad, bool fFree )
{
	DataRegisterClass
			classLoaded = m_lruDataReg.dprSakura[regSakura].regClass ;
	int		regPhy = m_lruDataReg.dprSakura[regSakura].regPhy ;
	bool	fModified = false ;
	if ( classLoaded == regClassNEON )
	{
		int	iSlot = regPhy - 8 ;
		ESLAssert( (iSlot >= 0) & (iSlot < 8) ) ;
		ESLAssert( m_lruDataReg.lruRegNEON.IsSlotAssigned(iSlot) ) ;
		ESLAssert( m_lruDataReg.lruRegNEON[iSlot].regPhy == regPhy ) ;
		if ( fFree )
		{
			fModified = m_lruDataReg.lruRegNEON[iSlot].fModified ;
			FreeDataRegister( classLoaded, regPhy ) ;
		}
		if ( fLoad )
		{
			WriteMoveVFP128( qregDst, regPhy ) ;
		}
		return	fModified ;
	}
	ESLAssert( !(regSakura & 0x01) ) ;
	fModified |= RealizeFreeVFPRegister
		( (qregDst << 1), (regSakura & ~0x01), fLoad, fFree ) ;
	fModified |= RealizeFreeVFPRegister
		( ((qregDst << 1) + 1), (regSakura | 0x01), fLoad, fFree ) ;
	return	fModified ;
}

// 一時処理のための物理レジスタを確保
//////////////////////////////////////////////////////////////////////////////
int ARMGenericAssembler::AllocateDataRegister
	( ARMGenericAssembler::DataRegisterClass regClass )
{
	int	iSlot, regPhy ;
	switch ( regClass )
	{
	case	regClassARM:
		iSlot = m_lruDataReg.lruRegARM.FindFreeSlot() ;
		if ( iSlot < 0 )
		{
			iSlot = m_lruDataReg.lruRegARM.FindMostOldSlot() ;
			ESLAssert( iSlot >= 0 ) ;
			WriteBackDataRegister
				( regClass, m_lruDataReg.lruRegARM[iSlot].regPhy ) ;
			FreeDataRegister
				( regClass, m_lruDataReg.lruRegARM[iSlot].regPhy ) ;
		}
		m_lruDataReg.lruRegARM.AddStale( 1 ) ;
		m_lruDataReg.lruRegARM.NewSlot( iSlot, -1 ) ;
		//
		regPhy = m_lruDataReg.lruRegARM[iSlot].regPhy ;
		LockDataRegister( regClass, regPhy ) ;
		return	regPhy ;

	case	regClassVFP:
		iSlot = m_lruDataReg.lruRegVFP.FindFreeSlot() ;
		if ( iSlot < 0 )
		{
			iSlot = m_lruDataReg.lruRegVFP.FindMostOldSlot() ;
			ESLAssert( iSlot >= 0 ) ;
			WriteBackDataRegister
				( regClass, m_lruDataReg.lruRegVFP[iSlot].regPhy ) ;
			FreeDataRegister
				( regClass, m_lruDataReg.lruRegVFP[iSlot].regPhy ) ;
		}
		m_lruDataReg.lruRegVFP.AddStale( 1 ) ;
		m_lruDataReg.lruRegVFP.NewSlot( iSlot, -1 ) ;
		//
		regPhy = m_lruDataReg.lruRegVFP[iSlot].regPhy ;
		LockDataRegister( regClass, regPhy ) ;
		return	regPhy ;

	case	regClassNEON:
		iSlot = m_lruDataReg.lruRegNEON.FindFreeSlot() ;
		if ( iSlot < 0 )
		{
			iSlot = m_lruDataReg.lruRegNEON.FindMostOldSlot() ;
			ESLAssert( iSlot >= 0 ) ;
			WriteBackDataRegister
				( regClass, m_lruDataReg.lruRegNEON[iSlot].regPhy ) ;
			FreeDataRegister
				( regClass, m_lruDataReg.lruRegNEON[iSlot].regPhy ) ;
		}
		m_lruDataReg.lruRegNEON.AddStale( 1 ) ;
		m_lruDataReg.lruRegNEON.NewSlot( iSlot, -1 ) ;
		//
		regPhy = m_lruDataReg.lruRegNEON[iSlot].regPhy ;
		LockDataRegister( regClass, regPhy ) ;
		return	regPhy ;

	default:
		break ;
	}
	return	-1 ;
}

// レジスタの値更新フラグ設定
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::SetDataRegisterModified
	( ARMGenericAssembler::DataRegisterClass regClass, int regPhy )
{
	int	iSlot ;
	switch ( regClass )
	{
	case	regClassARM:
		iSlot = (regPhy >> 1) ;
		ESLAssert( (iSlot >= 0) & (iSlot < 3) ) ;
		ESLAssert( m_lruDataReg.lruRegARM.IsSlotAssigned(iSlot) ) ;
		ESLAssert( m_lruDataReg.lruRegARM[iSlot].regPhy == regPhy ) ;
		m_lruDataReg.lruRegARM.SetModified( iSlot ) ;
		break ;

	case	regClassVFP:
		iSlot = regPhy ;
		ESLAssert( (iSlot >= 0) & (iSlot < 0x10) ) ;
		ESLAssert( m_lruDataReg.lruRegVFP.IsSlotAssigned(iSlot) ) ;
		ESLAssert( m_lruDataReg.lruRegVFP[iSlot].regPhy == regPhy ) ;
		m_lruDataReg.lruRegVFP.SetModified( iSlot ) ;
		break ;

	case	regClassNEON:
		iSlot = regPhy - 8 ;
		ESLAssert( (iSlot >= 0) & (iSlot < 8) ) ;
		ESLAssert( m_lruDataReg.lruRegNEON.IsSlotAssigned(iSlot) ) ;
		ESLAssert( m_lruDataReg.lruRegNEON[iSlot].regPhy == regPhy ) ;
		m_lruDataReg.lruRegNEON.SetModified( iSlot ) ;
		break ;

	default:
		break ;
	}
}

// 物理レジスタの変更をライトバック
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteBackDataRegister
	( ARMGenericAssembler::DataRegisterClass regClass,
								int regPhy, bool fKeepModified )
{
	int		iSlot, regSakura ;
	switch ( regClass )
	{
	case	regClassARM:
		iSlot = (regPhy >> 1) ;
		ESLAssert( (iSlot >= 0) & (iSlot < 3) ) ;
		ESLAssert( m_lruDataReg.lruRegARM[iSlot].regPhy == regPhy ) ;
		if ( m_lruDataReg.lruRegARM.IsSlotAssigned( iSlot )
				&& m_lruDataReg.lruRegARM[iSlot].fModified )
		{
			if ( !fKeepModified )
			{
				m_lruDataReg.lruRegARM[iSlot].fModified = false ;
			}
			regSakura = m_lruDataReg.lruRegARM[iSlot].regSakura ;
			size_t	dispOffset = Context::OffsetOfReg(regSakura) ;
			if ( dispOffset <= 0xFF )
			{
				WriteARMStoreDoubleMemOffsetImm8
					( (ARMRegister) regPhy, ARM_r10, dispOffset ) ;
			}
			else
			{
				WriteARMStoreMemOffsetImm12
					( (ARMRegister) regPhy, ARM_r10, dispOffset ) ;
				WriteARMStoreMemOffsetImm12
					( (ARMRegister) (regPhy + 1), ARM_r10, dispOffset + 4 ) ;
			}
		}
		break ;

	case	regClassVFP:
		iSlot = regPhy ;
		ESLAssert( (iSlot >= 0) & (iSlot < 0x10) ) ;
		ESLAssert( m_lruDataReg.lruRegVFP[iSlot].regPhy == regPhy ) ;
		if ( m_lruDataReg.lruRegVFP.IsSlotAssigned( iSlot )
				&& m_lruDataReg.lruRegVFP[iSlot].fModified )
		{
			if ( !fKeepModified )
			{
				m_lruDataReg.lruRegVFP[iSlot].fModified = false ;
			}
			regSakura = m_lruDataReg.lruRegVFP[iSlot].regSakura ;
			size_t	dispOffset = Context::OffsetOfReg(regSakura) ;
			if ( dispOffset <= 0x3FC )
			{
				WriteVFPStore64OffsetImm8( regPhy, ARM_r10, dispOffset ) ;
			}
			else
			{
				PreserveContinuousCodes( 0x20 ) ;
				WriteARMLeaSakura2Register( ARM_r6, regSakura ) ;
				WriteVFPStore64OffsetImm8( regPhy, ARM_r6, 0 ) ;
			}
		}
		break ;

	case	regClassNEON:
		iSlot = regPhy - 8 ;
		ESLAssert( (iSlot >= 0) & (iSlot < 8) ) ;
		ESLAssert( m_lruDataReg.lruRegNEON[iSlot].regPhy == regPhy ) ;
		if ( m_lruDataReg.lruRegNEON.IsSlotAssigned( iSlot )
				&& m_lruDataReg.lruRegNEON[iSlot].fModified )
		{
			if ( !fKeepModified )
			{
				m_lruDataReg.lruRegNEON[iSlot].fModified = false ;
			}
			regSakura = m_lruDataReg.lruRegNEON[iSlot].regSakura ;
			size_t	dispOffset = Context::OffsetOfReg(regSakura+1) ;
			if ( dispOffset <= 0x3FC )
			{
				WriteVFPStore64OffsetImm8( (regPhy << 1), ARM_r10, dispOffset - 8 ) ;
				WriteVFPStore64OffsetImm8( (regPhy << 1) + 1, ARM_r10, dispOffset ) ;
			}
			else
			{
				PreserveContinuousCodes( 0x20 ) ;
				WriteARMLeaSakura2Register( ARM_r6, regSakura ) ;
				WriteVFPStore64OffsetImm8( (regPhy << 1), ARM_r6, 0 ) ;
				WriteVFPStore64OffsetImm8( (regPhy << 1) + 1, ARM_r6, 8 ) ;
			}
		}
		break ;

	default:
		break ;
	}
}

// 物理レジスタの内容をリロード
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::ReloadDataRegister
	( ARMGenericAssembler::DataRegisterClass regClass, int regPhy )
{
	int		iSlot, regSakura ;
	switch ( regClass )
	{
	case	regClassARM:
		iSlot = (regPhy >> 1) ;
		ESLAssert( (iSlot >= 0) & (iSlot < 3) ) ;
		ESLAssert( m_lruDataReg.lruRegARM[iSlot].regPhy == regPhy ) ;
		if ( m_lruDataReg.lruRegARM.IsSlotAssigned( iSlot ) )
		{
			regSakura = m_lruDataReg.lruRegARM[iSlot].regSakura ;
			if ( regSakura >= 0 )
			{
				ESLAssert( (regSakura >= 0) && (regSakura < 0x100) ) ;
				size_t	dispOffset = Context::OffsetOfReg(regSakura) ;
				if ( dispOffset <= 0xFF )
				{
					WriteARMLoadDoubleMemOffsetImm8
						( (ARMRegister) regPhy, ARM_r10, dispOffset ) ;
				}
				else
				{
					WriteARMLoadMemOffsetImm12
						( (ARMRegister) regPhy, ARM_r10, dispOffset ) ;
					WriteARMLoadMemOffsetImm12
						( (ARMRegister) (regPhy + 1), ARM_r10, dispOffset + 4 ) ;
				}
			}
		}
		break ;

	case	regClassVFP:
		iSlot = regPhy ;
		ESLAssert( (iSlot >= 0) & (iSlot < 0x10) ) ;
		ESLAssert( m_lruDataReg.lruRegVFP[iSlot].regPhy == regPhy ) ;
		if ( m_lruDataReg.lruRegVFP.IsSlotAssigned( iSlot ) )
		{
			regSakura = m_lruDataReg.lruRegVFP[iSlot].regSakura ;
			if ( regSakura >= 0 )
			{
				ESLAssert( (regSakura >= 0) && (regSakura < 0x100) ) ;
				size_t	dispOffset = Context::OffsetOfReg(regSakura) ;
				if ( dispOffset <= 0x3FC )
				{
					WriteVFPLoad64OffsetImm8( regPhy, ARM_r10, dispOffset ) ;
				}
				else
				{
					PreserveContinuousCodes( 0x20 ) ;
					WriteARMLeaSakura2Register( ARM_r6, regSakura ) ;
					WriteVFPLoad64OffsetImm8( regPhy, ARM_r6, 0 ) ;
				}
			}
		}
		break ;

	case	regClassNEON:
		iSlot = regPhy - 8 ;
		ESLAssert( (iSlot >= 0) & (iSlot < 8) ) ;
		ESLAssert( m_lruDataReg.lruRegNEON[iSlot].regPhy == regPhy ) ;
		if ( m_lruDataReg.lruRegNEON.IsSlotAssigned( iSlot ) )
		{
			regSakura = m_lruDataReg.lruRegNEON[iSlot].regSakura ;
			if ( regSakura >= 0 )
			{
				ESLAssert( (regSakura >= 0) && (regSakura < 0x100) ) ;
				size_t	dispOffset = Context::OffsetOfReg(regSakura+1) ;
				if ( dispOffset <= 0x3FC )
				{
					WriteVFPLoad64OffsetImm8
						( (regPhy << 1), ARM_r10, dispOffset - 8 ) ;
					WriteVFPLoad64OffsetImm8
						( (regPhy << 1) + 1, ARM_r10, dispOffset ) ;
				}
				else
				{
					PreserveContinuousCodes( 0x20 ) ;
					WriteARMLeaSakura2Register( ARM_r6, regSakura ) ;
					WriteVFPLoad64OffsetImm8( (regPhy << 1), ARM_r6, 0 ) ;
					WriteVFPLoad64OffsetImm8( (regPhy << 1) + 1, ARM_r6, 8 ) ;
				}
			}
		}
		break ;

	default:
		break ;
	}
}

// Sakura2 レジスタの割り当てをスワップ
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::SwapDataRegisterAssignation
	( ARMGenericAssembler::DataRegisterClass regClass, int regPhy1, int regPhy2 )
{
	int	iSlot1, iSlot2 ;
	int	regTemp1, regTemp2 ;
	switch ( regClass )
	{
	case	regClassARM:
		iSlot1 = (regPhy1 >> 1) ;
		iSlot2 = (regPhy2 >> 1) ;
		ESLAssert( (iSlot1 >= 0) & (iSlot1 < 3) ) ;
		ESLAssert( (iSlot2 >= 0) & (iSlot2 < 3) ) ;
		ESLAssert( m_lruDataReg.lruRegARM.IsSlotAssigned(iSlot1) ) ;
		ESLAssert( m_lruDataReg.lruRegARM.IsSlotAssigned(iSlot2) ) ;
		ESLAssert( m_lruDataReg.lruRegARM[iSlot1].regPhy == regPhy1 ) ;
		ESLAssert( m_lruDataReg.lruRegARM[iSlot2].regPhy == regPhy2 ) ;
		regTemp1 = m_lruDataReg.lruRegARM[iSlot1].regSakura ;
		regTemp2 = m_lruDataReg.lruRegARM[iSlot2].regSakura ;
		m_lruDataReg.lruRegARM[iSlot1].regSakura = regTemp2 ;
		m_lruDataReg.lruRegARM[iSlot2].regSakura = regTemp1 ;
		if ( regTemp2 >= 0 )
		{
			m_lruDataReg.dprSakura[regTemp2].regClass = regClass ;
			m_lruDataReg.dprSakura[regTemp2].regPhy =
								m_lruDataReg.lruRegARM[iSlot1].regPhy ;
		}
		if ( regTemp1 >= 0 )
		{
			m_lruDataReg.dprSakura[regTemp1].regClass = regClass ;
			m_lruDataReg.dprSakura[regTemp1].regPhy =
								m_lruDataReg.lruRegARM[iSlot2].regPhy ;
		}
		break ;

	case	regClassVFP:
		iSlot1 = regPhy1 ;
		iSlot2 = regPhy2 ;
		ESLAssert( (iSlot1 >= 0) & (iSlot1 < 0x10) ) ;
		ESLAssert( (iSlot2 >= 0) & (iSlot2 < 0x10) ) ;
		ESLAssert( m_lruDataReg.lruRegVFP.IsSlotAssigned(iSlot1) ) ;
		ESLAssert( m_lruDataReg.lruRegVFP.IsSlotAssigned(iSlot2) ) ;
		ESLAssert( m_lruDataReg.lruRegVFP[iSlot1].regPhy == regPhy1 ) ;
		ESLAssert( m_lruDataReg.lruRegVFP[iSlot2].regPhy == regPhy2 ) ;
		regTemp1 = m_lruDataReg.lruRegVFP[iSlot1].regSakura ;
		regTemp2 = m_lruDataReg.lruRegVFP[iSlot2].regSakura ;
		m_lruDataReg.lruRegVFP[iSlot1].regSakura = regTemp2 ;
		m_lruDataReg.lruRegVFP[iSlot2].regSakura = regTemp1 ;
		if ( regTemp2 >= 0 )
		{
			m_lruDataReg.dprSakura[regTemp2].regClass = regClass ;
			m_lruDataReg.dprSakura[regTemp2].regPhy =
								m_lruDataReg.lruRegVFP[iSlot1].regPhy ;
		}
		if ( regTemp1 >= 0 )
		{
			m_lruDataReg.dprSakura[regTemp1].regClass = regClass ;
			m_lruDataReg.dprSakura[regTemp1].regPhy =
								m_lruDataReg.lruRegVFP[iSlot2].regPhy ;
		}
		break ;

	case	regClassNEON:
		iSlot1 = regPhy1 - 8 ;
		iSlot2 = regPhy2 - 8 ;
		ESLAssert( (iSlot1 >= 0) & (iSlot1 < 8) ) ;
		ESLAssert( (iSlot2 >= 0) & (iSlot2 < 8) ) ;
		ESLAssert( m_lruDataReg.lruRegNEON.IsSlotAssigned(iSlot1) ) ;
		ESLAssert( m_lruDataReg.lruRegNEON.IsSlotAssigned(iSlot2) ) ;
		ESLAssert( m_lruDataReg.lruRegNEON[iSlot1].regPhy == regPhy1 ) ;
		ESLAssert( m_lruDataReg.lruRegNEON[iSlot2].regPhy == regPhy2 ) ;
		regTemp1 = m_lruDataReg.lruRegNEON[iSlot1].regSakura ;
		regTemp2 = m_lruDataReg.lruRegNEON[iSlot2].regSakura ;
		m_lruDataReg.lruRegNEON[iSlot1].regSakura = regTemp2 ;
		m_lruDataReg.lruRegNEON[iSlot2].regSakura = regTemp1 ;
		if ( regTemp2 >= 0 )
		{
			ESLAssert( !(regTemp2 & 0x01) ) ;
			m_lruDataReg.dprSakura[regTemp2].regClass = regClass ;
			m_lruDataReg.dprSakura[regTemp2].regPhy =
								m_lruDataReg.lruRegNEON[iSlot1].regPhy ;
			m_lruDataReg.dprSakura[regTemp2^0x01].regClass = regClass ;
			m_lruDataReg.dprSakura[regTemp2^0x01].regPhy =
								m_lruDataReg.lruRegNEON[iSlot1].regPhy ;
		}
		if ( regTemp1 >= 0 )
		{
			ESLAssert( !(regTemp1 & 0x01) ) ;
			m_lruDataReg.dprSakura[regTemp1].regClass = regClass ;
			m_lruDataReg.dprSakura[regTemp1].regPhy =
								m_lruDataReg.lruRegNEON[iSlot2].regPhy ;
			m_lruDataReg.dprSakura[regTemp1^0x01].regClass = regClass ;
			m_lruDataReg.dprSakura[regTemp1^0x01].regPhy =
								m_lruDataReg.lruRegNEON[iSlot2].regPhy ;
		}
		break ;

	default:
		break ;
	}
}

// 物理レジスタの割り当てを一時的にロック
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::LockDataRegister
	( ARMGenericAssembler::DataRegisterClass regClass, int regPhy )
{
	int		iSlot ;
	switch ( regClass )
	{
	case	regClassARM:
		iSlot = (regPhy >> 1) ;
		ESLAssert( (iSlot >= 0) & (iSlot < 3) ) ;
		ESLAssert( m_lruDataReg.lruRegARM.IsSlotAssigned(iSlot) ) ;
		ESLAssert( m_lruDataReg.lruRegARM[iSlot].regPhy == regPhy ) ;
		m_lruDataReg.lruRegARM.RecentlyAccess( iSlot ) ;
		m_lruDataReg.lruRegARM.LockSlot( iSlot ) ;
		break ;

	case	regClassVFP:
		iSlot = regPhy ;
		ESLAssert( (iSlot >= 0) & (iSlot < 0x10) ) ;
		ESLAssert( m_lruDataReg.lruRegVFP.IsSlotAssigned(iSlot) ) ;
		ESLAssert( m_lruDataReg.lruRegVFP[iSlot].regPhy == regPhy ) ;
		m_lruDataReg.lruRegVFP.RecentlyAccess( iSlot ) ;
		m_lruDataReg.lruRegVFP.LockSlot( iSlot ) ;
		break ;

	case	regClassNEON:
		iSlot = regPhy - 8 ;
		ESLAssert( (iSlot >= 0) & (iSlot < 8) ) ;
		ESLAssert( m_lruDataReg.lruRegNEON.IsSlotAssigned(iSlot) ) ;
		ESLAssert( m_lruDataReg.lruRegNEON[iSlot].regPhy == regPhy ) ;
		m_lruDataReg.lruRegNEON.RecentlyAccess( iSlot ) ;
		m_lruDataReg.lruRegNEON.LockSlot( iSlot ) ;
		break ;

	default:
		break ;
	}
}

// 物理レジスタの割り当てを解放可能にアンロック
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::UnlockDataRegister
	( ARMGenericAssembler::DataRegisterClass regClass, int regPhy )
{
	int		iSlot ;
	switch ( regClass )
	{
	case	regClassARM:
		iSlot = (regPhy >> 1) ;
		ESLAssert( (iSlot >= 0) & (iSlot < 3) ) ;
		ESLAssert( m_lruDataReg.lruRegARM.IsSlotAssigned(iSlot) ) ;
		ESLAssert( m_lruDataReg.lruRegARM[iSlot].regPhy == regPhy ) ;
		m_lruDataReg.lruRegARM.UnlockSlot( iSlot ) ;
		if ( (m_lruDataReg.lruRegARM[iSlot].numLocked == 0)
			&& (m_lruDataReg.lruRegARM[iSlot].regSakura == -1) )
		{
			FreeDataRegister( regClass, regPhy ) ;
		}
		break ;

	case	regClassVFP:
		iSlot = regPhy ;
		ESLAssert( (iSlot >= 0) & (iSlot < 0x10) ) ;
		ESLAssert( m_lruDataReg.lruRegVFP.IsSlotAssigned(iSlot) ) ;
		ESLAssert( m_lruDataReg.lruRegVFP[iSlot].regPhy == regPhy ) ;
		m_lruDataReg.lruRegVFP.UnlockSlot( iSlot ) ;
		if ( (m_lruDataReg.lruRegVFP[iSlot].numLocked == 0)
			&& (m_lruDataReg.lruRegVFP[iSlot].regSakura == -1) )
		{
			FreeDataRegister( regClass, regPhy ) ;
		}
		break ;

	case	regClassNEON:
		iSlot = regPhy - 8 ;
		ESLAssert( (iSlot >= 0) & (iSlot < 8) ) ;
		ESLAssert( m_lruDataReg.lruRegNEON.IsSlotAssigned(iSlot) ) ;
		ESLAssert( m_lruDataReg.lruRegNEON[iSlot].regPhy == regPhy ) ;
		m_lruDataReg.lruRegNEON.UnlockSlot( iSlot ) ;
		if ( (m_lruDataReg.lruRegNEON[iSlot].numLocked == 0)
			&& (m_lruDataReg.lruRegNEON[iSlot].regSakura == -1) )
		{
			FreeDataRegister( regClass, regPhy ) ;
		}
		break ;

	default:
		break ;
	}
}

// 物理レジスタの割り当てを解放
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::FreeDataRegister
	( ARMGenericAssembler::DataRegisterClass regClass, int regPhy )
{
	int	iSlot, regSakura = -1 ;
	switch ( regClass )
	{
	case	regClassARM:
		iSlot = (regPhy >> 1) ;
		ESLAssert( (iSlot >= 0) & (iSlot < 3) ) ;
		ESLAssert( m_lruDataReg.lruRegARM.IsSlotAssigned(iSlot) ) ;
		ESLAssert( m_lruDataReg.lruRegARM[iSlot].regPhy == regPhy ) ;
		regSakura = m_lruDataReg.lruRegARM[iSlot].regSakura ;
		m_lruDataReg.lruRegARM.FreeSlot( iSlot ) ;
		break ;

	case	regClassVFP:
		iSlot = regPhy ;
		ESLAssert( (iSlot >= 0) & (iSlot < 0x10) ) ;
		ESLAssert( m_lruDataReg.lruRegVFP.IsSlotAssigned(iSlot) ) ;
		ESLAssert( m_lruDataReg.lruRegVFP[iSlot].regPhy == regPhy ) ;
		regSakura = m_lruDataReg.lruRegVFP[iSlot].regSakura ;
		m_lruDataReg.lruRegVFP.FreeSlot( iSlot ) ;
		break ;

	case	regClassNEON:
		iSlot = regPhy - 8 ;
		ESLAssert( (iSlot >= 0) & (iSlot < 8) ) ;
		ESLAssert( m_lruDataReg.lruRegNEON.IsSlotAssigned(iSlot) ) ;
		ESLAssert( m_lruDataReg.lruRegNEON[iSlot].regPhy == regPhy ) ;
		regSakura = m_lruDataReg.lruRegNEON[iSlot].regSakura ;
		m_lruDataReg.lruRegNEON.FreeSlot( iSlot ) ;
		break ;

	default:
		break ;
	}
	if ( regSakura >= 0 )
	{
		ESLAssert( regSakura < 0x100 ) ;
		ESLAssert( m_lruDataReg.dprSakura[regSakura].regClass == regClass ) ;
		regSakura &= 0xFF ;
		m_lruDataReg.dprSakura[regSakura].regClass = regClassNothing ;
		m_lruDataReg.dprSakura[regSakura].regPhy = -1 ;
		if ( regClass == regClassNEON )
		{
			ESLAssert( !(regSakura & 0x01) ) ;
			m_lruDataReg.dprSakura[regSakura^0x01].regClass = regClassNothing ;
			m_lruDataReg.dprSakura[regSakura^0x01].regPhy = -1 ;
		}
	}
}

// 物理レジスタの妥当性をチェック（デバッグ用）
//////////////////////////////////////////////////////////////////////////////
bool ARMGenericAssembler::VerifyDataRegisterAssignation( void )
{
	int		i, iSlot, regSakura, regPhy ;
	bool	fVerify = true ;
	for ( i = 0; i < m_lruDataReg.lruRegARM.SlotCount; i ++ )
	{
		if ( m_lruDataReg.lruRegARM.IsSlotAssigned(i) )
		{
			regPhy = m_lruDataReg.lruRegARM[i].regPhy ;
			regSakura = m_lruDataReg.lruRegARM[i].regSakura ;
			if ( regSakura >= 0 )
			{
				ESLAssert( regSakura < 0x100 ) ;
				ESLAssert( m_lruDataReg.dprSakura[regSakura].regClass == regClassARM ) ;
				ESLAssert( m_lruDataReg.dprSakura[regSakura].regPhy == regPhy ) ;
				fVerify &= (regSakura < 0x100)
					& (m_lruDataReg.dprSakura[regSakura].regClass == regClassARM)
					& (m_lruDataReg.dprSakura[regSakura].regPhy == regPhy) ;
			}
		}
	}
	for ( i = 0; i < m_lruDataReg.lruRegVFP.SlotCount; i ++ )
	{
		if ( m_lruDataReg.lruRegVFP.IsSlotAssigned(i) )
		{
			regPhy = m_lruDataReg.lruRegVFP[i].regPhy ;
			regSakura = m_lruDataReg.lruRegVFP[i].regSakura ;
			if ( regSakura >= 0 )
			{
				ESLAssert( regSakura < 0x100 ) ;
				ESLAssert( m_lruDataReg.dprSakura[regSakura].regClass == regClassVFP ) ;
				ESLAssert( m_lruDataReg.dprSakura[regSakura].regPhy == regPhy ) ;
				fVerify &= (regSakura < 0x100)
					& (m_lruDataReg.dprSakura[regSakura].regClass == regClassVFP)
					& (m_lruDataReg.dprSakura[regSakura].regPhy == regPhy) ;
			}
		}
	}
	for ( i = 0; i < m_lruDataReg.lruRegNEON.SlotCount; i ++ )
	{
		if ( m_lruDataReg.lruRegNEON.IsSlotAssigned(i) )
		{
			regPhy = m_lruDataReg.lruRegNEON[i].regPhy ;
			regSakura = m_lruDataReg.lruRegNEON[i].regSakura ;
			if ( regSakura >= 0 )
			{
				ESLAssert( regSakura < 0x100 ) ;
				ESLAssert( !(regSakura & 0x01) ) ;
				ESLAssert( m_lruDataReg.dprSakura[regSakura].regClass == regClassNEON ) ;
				ESLAssert( m_lruDataReg.dprSakura[regSakura].regPhy == regPhy ) ;
				ESLAssert( m_lruDataReg.dprSakura[regSakura^0x01].regClass == regClassNEON ) ;
				ESLAssert( m_lruDataReg.dprSakura[regSakura^0x01].regPhy == regPhy ) ;
				fVerify &= (regSakura < 0x100)
					& !(regSakura & 0x01)
					& (m_lruDataReg.dprSakura[regSakura].regClass == regClassNEON)
					& (m_lruDataReg.dprSakura[regSakura].regPhy == regPhy)
					& (m_lruDataReg.dprSakura[regSakura^0x01].regClass == regClassNEON)
					& (m_lruDataReg.dprSakura[regSakura^0x01].regPhy == regPhy) ;
			}
		}
	}
	for ( i = 0; i < 0x100; i ++ )
	{
		DataRegisterClass	regClass = m_lruDataReg.dprSakura[i].regClass ;
		if ( regClass != regClassNothing )
		{
			regPhy = m_lruDataReg.dprSakura[i].regPhy ;
			switch ( regClass )
			{
			case	regClassARM:
				iSlot = (regPhy >> 1) ;
				ESLAssert( m_lruDataReg.lruRegARM.IsSlotAssigned(iSlot) ) ;
				ESLAssert( m_lruDataReg.lruRegARM[iSlot].regSakura == i ) ;
				fVerify &= m_lruDataReg.lruRegARM.IsSlotAssigned(iSlot)
						& (m_lruDataReg.lruRegARM[iSlot].regSakura == i) ;
				break ;
			case	regClassVFP:
				iSlot = regPhy ;
				ESLAssert( m_lruDataReg.lruRegVFP.IsSlotAssigned(iSlot) ) ;
				ESLAssert( m_lruDataReg.lruRegVFP[iSlot].regSakura == i ) ;
				fVerify &= m_lruDataReg.lruRegVFP.IsSlotAssigned(iSlot)
						& (m_lruDataReg.lruRegVFP[iSlot].regSakura == i) ;
				break ;
			case	regClassNEON:
				iSlot = regPhy - 8 ;
				ESLAssert( m_lruDataReg.lruRegNEON.IsSlotAssigned(iSlot) ) ;
				ESLAssert( m_lruDataReg.lruRegNEON[iSlot].regSakura == (i & ~0x01) ) ;
				fVerify &= m_lruDataReg.lruRegNEON.IsSlotAssigned(iSlot)
						& (m_lruDataReg.lruRegNEON[iSlot].regSakura == i) ;
				break ;
			default:
				break ;
			}
		}
	}
	return	fVerify ;
}

// デバッグ文字列出力コード生成
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::TraceDebugString
			( const char * pszText, int reg1, int reg2, int reg3 )
{
	const ARMRegister	regPushList[] =
	{
		ARM_r0, ARM_r1, ARM_r2, ARM_r3,
		ARM_r4, ARM_r5, ARM_r6, ARM_r7,
		ARM_r8, ARM_r9, ARM_r10, ARM_r11,
		ARM_r12, ARM_LR,
	} ;
	WriteARMPushRegs( &regPushList[0], 14 ) ;

	if ( !((reg1 == ARM_r3) || (reg2 == ARM_r3)) )
	{
		WriteARMMoveRegReg( ARM_r3, (ARMRegister) reg3 ) ;
	}
	if ( reg2 == ARM_r1 )
	{
		if ( reg1 == ARM_r2 )
		{
			WriteARMMoveRegReg( ARM_r1, (ARMRegister) reg1 ) ;
			WriteARMMoveRegReg( ARM_r2, ARM_r3 ) ;
		}
		else
		{
			WriteARMMoveRegReg( ARM_r2, (ARMRegister) reg2 ) ;
			WriteARMMoveRegReg( ARM_r1, (ARMRegister) reg1 ) ;
		}
	}
	else
	{
		WriteARMMoveRegReg( ARM_r1, (ARMRegister) reg1 ) ;
		WriteARMMoveRegReg( ARM_r2, (ARMRegister) reg2 ) ;
	}
	if ( (reg1 == ARM_r3) || (reg2 == ARM_r3) )
	{
		WriteARMMoveRegReg( ARM_r3, (ARMRegister) reg3 ) ;
	}
	WriteARMMoveRegImm( ARM_r0, (int32_t) ((ulong_ptr_t) pszText) ) ;

	PreserveContinuousCodes( 0x10 ) ;
	WriteARMCallImm( (const void*) &SSystem::Trace ) ;

	WriteARMPopRegs( &regPushList[0], 14 ) ;
}


// 汎用演算命令出力（複雑な演算命令は外部関数を呼び出して解決する）
// 関数呼び出しコード出力 : reg 形式
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteToCallInstructionReg
	( OPERATION_DST_SRC_PROC pfnGen, int reg )
{
	FlushAllRegisters() ;
	//
	WriteARMLeaSakura2Register( ARM_r0, reg ) ;
	WriteARMMoveRegReg( ARM_r1, ARM_r0 ) ;
	WriteARMCallImm( (const void*) pfnGen ) ;
	//
	ResetRegisterAfterCall() ;
}

// 関数呼び出しコード出力 : reg, reg 形式
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteToCallInstructionRegReg
	( OPERATION_DST_SRC_PROC pfnGen, int dstreg, int srcreg )
{
	FlushAllRegisters() ;
	//
	WriteARMLeaSakura2Register( ARM_r0, dstreg ) ;
	WriteARMLeaSakura2Register( ARM_r1, srcreg ) ;
	WriteARMCallImm( (const void*) pfnGen ) ;
	//
	ResetRegisterAfterCall() ;
}

void ARMGenericAssembler::WriteToCallSIMD128InstructionRegReg
	( OPERATION_SIMD128_DST_SRC_PROC pfnGen, int dstreg, int srcreg )
{
	FlushAllRegisters() ;
	//
	WriteARMLeaSakura2Register( ARM_r0, dstreg ) ;
	WriteARMLeaSakura2Register( ARM_r1, srcreg ) ;
	WriteARMCallImm( (const void*) pfnGen ) ;
	//
	ResetRegisterAfterCall() ;
}

// 関数呼び出しコード出力 : reg, reg, reg 形式
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteToCallInstructionRegRegReg
	( OPERATION_DST_SRC_SRC2_PROC pfnGen,
				int dstreg, int srcreg, int srcreg2 )
{
	FlushAllRegisters() ;
	//
	WriteARMLeaSakura2Register( ARM_r0, dstreg ) ;
	WriteARMLeaSakura2Register( ARM_r1, srcreg ) ;
	WriteARMLeaSakura2Register( ARM_r2, srcreg2 ) ;
	WriteARMCallImm( (const void*) pfnGen ) ;
	//
	ResetRegisterAfterCall() ;
}

// 関数呼び出しコード出力 : reg, reg, imm 形式
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteToCallInstructionRegRegImm
	( OPERATION_DST_SRC_IMM_PROC pfnGen, int dstreg, int srcreg, int imm )
{
	FlushAllRegisters() ;
	//
	WriteARMLeaSakura2Register( ARM_r0, dstreg ) ;
	WriteARMLeaSakura2Register( ARM_r1, srcreg ) ;
	WriteARMMoveRegImm32( ARM_r2, (uint32_t) imm ) ;
	WriteARMCallImm( (const void*) pfnGen ) ;
	//
	ResetRegisterAfterCall() ;
}

void ARMGenericAssembler::WriteToCallSIMD128InstructionRegRegImm
	( OPERATION_SIMD128_DST_SRC_IMM_PROC pfnGen, int dstreg, int srcreg, int imm )
{
	FlushAllRegisters() ;
	//
	WriteARMMoveRegReg( ARM_r0, ARM_r10 ) ;
	WriteARMLeaSakura2Register( ARM_r1, dstreg ) ;
	WriteARMLeaSakura2Register( ARM_r2, srcreg ) ;
	WriteARMMoveRegImm32( ARM_r3, (uint32_t) imm ) ;
	WriteARMCallImm( (const void*) pfnGen ) ;
	//
	ResetRegisterAfterCall() ;
}

// 関数呼び出し後のレジスタ処理
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::ResetRegisterAfterCall( void )
{
	ResetDataRegisters() ;
	//
	ESLAssert( m_lruPointer[regPtrPhyR9].regPhy == ARM_r9 ) ;
	ESLAssert( m_lruPointer[regPtrPhyR12].regPhy == ARM_r12 ) ;
	m_lruPointer.FreeSlot( regPtrPhyR9 ) ;
	m_lruPointer.FreeSlot( regPtrPhyR12 ) ;
}

// プロローグコード出力
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WritePrologue( void )
{
	const ARMRegister	regPushList[] =
	{
		ARM_r4, ARM_r5, ARM_r6, ARM_r7,
		ARM_r8, ARM_r9, ARM_r10, ARM_r11,
		ARM_r12, ARM_LR,
	} ;
	// PUSH r4-r12, LR
	WriteARMPushRegs( &regPushList[0], 10 ) ;
	//
	if ( m_vfpVersion >= 2 )
	{
		// VPUSH s16-s31
		WriteVFPPushReg32( 16, 16 ) ;
	}
	// MOV r10, r0
	WriteARMMoveRegReg( ARM_r10, ARM_r0 ) ;
}

void ARMGenericAssembler::WriteSubPrologue( void )
{
	if ( m_flagNoBoundary )
	{
		for ( int i = 0; i < 2; i ++ )
		{
			if ( m_regTLBFetched[i] >= 0 )
			{
				WritePrefetchTLB( i, m_regTLBFetched[i] ) ;
			}
		}
	}
}

// エピローグコード出力
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteEpilogue( int ipExit )
{
	if ( ipExit >= 0 )
	{
		PreserveContinuousCodes( 0x20 ) ;
		WriteARMMoveRegImm32( ARM_r6, ipExit ) ;
		WriteARMStoreMemOffsetImm12
			( ARM_r6, ARM_r10, offsetof(Context,m_ip) ) ;
	}
	if ( m_vfpVersion >= 2 )
	{
		// VPOP s16-s31
		WriteVFPPopReg32( 16, 16 ) ;
	}
	const ARMRegister	regPopList[] =
	{
		ARM_r4, ARM_r5, ARM_r6, ARM_r7,
		ARM_r8, ARM_r9, ARM_r10, ARM_r11,
		ARM_r12, ARM_PC,
	} ;
	// POP r4-r12, PC
	WriteARMPopRegs( &regPopList[0], 10 ) ;
	//
	// nop
	WriteARMMoveRegReg( ARM_r0, ARM_r0 ) ;
}

// レジスタへの変更をコンテキストに書き出し
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::FlushAllRegisters( void )
{
	int	i ;
	for ( i = 0; i < m_lruDataReg.lruRegARM.SlotCount; i ++ )
	{
		WriteBackDataRegister
			( regClassARM, m_lruDataReg.lruRegARM[i].regPhy ) ;
	}
	for ( i = 0; i < m_lruDataReg.lruRegVFP.SlotCount; i ++ )
	{
		WriteBackDataRegister
			( regClassVFP, m_lruDataReg.lruRegVFP[i].regPhy ) ;
	}
	for ( i = 0; i < m_lruDataReg.lruRegNEON.SlotCount; i ++ )
	{
		WriteBackDataRegister
			( regClassNEON, m_lruDataReg.lruRegNEON[i].regPhy ) ;
	}
}

// レジスタへの変更をコンテキストに書き出し
//（レジスタコンテキストを変更しない）
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteBackAllRegisters( void )
{
	int	i ;
	for ( i = 0; i < m_lruDataReg.lruRegARM.SlotCount; i ++ )
	{
		WriteBackDataRegister
			( regClassARM, m_lruDataReg.lruRegARM[i].regPhy, true ) ;
	}
	for ( i = 0; i < m_lruDataReg.lruRegVFP.SlotCount; i ++ )
	{
		WriteBackDataRegister
			( regClassVFP, m_lruDataReg.lruRegVFP[i].regPhy, true ) ;
	}
	for ( i = 0; i < m_lruDataReg.lruRegNEON.SlotCount; i ++ )
	{
		WriteBackDataRegister
			( regClassNEON, m_lruDataReg.lruRegNEON[i].regPhy, true ) ;
	}
}

// レジスタへの変更をコンテキストに書き出し
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::FlushRegister( int regSakura )
{
	DataRegisterClass
		classLoaded = m_lruDataReg.dprSakura[regSakura].regClass ;
	int	regPhy = m_lruDataReg.dprSakura[regSakura].regPhy ;
	if ( (classLoaded != regClassNothing) & (regPhy != -1) )
	{
		WriteBackDataRegister( classLoaded, regPhy ) ;
	}
}

// レジスタの値を物理レジスタに復元する
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::ReloadRegisters( void )
{
	int	i ;
	for ( i = 0; i < m_lruDataReg.lruRegARM.SlotCount; i ++ )
	{
		ReloadDataRegister
			( regClassARM, m_lruDataReg.lruRegARM[i].regPhy ) ;
	}
	for ( i = 0; i < m_lruDataReg.lruRegVFP.SlotCount; i ++ )
	{
		ReloadDataRegister
			( regClassVFP, m_lruDataReg.lruRegVFP[i].regPhy ) ;
	}
	for ( i = 0; i < m_lruDataReg.lruRegNEON.SlotCount; i ++ )
	{
		ReloadDataRegister
			( regClassNEON, m_lruDataReg.lruRegNEON[i].regPhy ) ;
	}
}

// レジスタコンテキストをリセット
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::ResetAllRegisters( void )
{
	ARMGenericAssembler::ResetDataRegisters() ;

	m_lruPointer.FreeAllUnlockedSlot() ;
}

// レジスタコンテキストをリセット
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::ResetRegister( int regSakura )
{
	DataRegisterClass
		classLoaded = m_lruDataReg.dprSakura[regSakura].regClass ;
	int	regPhy = m_lruDataReg.dprSakura[regSakura].regPhy ;
	if ( (classLoaded != regClassNothing) & (regPhy != -1) )
	{
		FreeDataRegister( classLoaded, regPhy ) ;
	}
	ModifiedRegister( regSakura ) ;
}

// レジスタコンテキストをリセット（ポインタレジスタ以外）
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::ResetDataRegisters( void )
{
	int	i, regSakura ;
	for ( i = 0; i < m_lruDataReg.lruRegARM.SlotCount; i ++ )
	{
		if ( m_lruDataReg.lruRegARM.IsSlotAssigned(i) )
		{
			regSakura = m_lruDataReg.lruRegARM[i].regSakura ;
			if ( regSakura >= 0 )
			{
				ESLAssert( regSakura < 0x100 ) ;
				ESLAssert( m_lruDataReg.dprSakura[regSakura].regClass == regClassARM ) ;
				m_lruDataReg.dprSakura[regSakura].regClass = regClassNothing ;
				m_lruDataReg.dprSakura[regSakura].regPhy = -1 ;
			}
		}
	}
	for ( i = 0; i < m_lruDataReg.lruRegVFP.SlotCount; i ++ )
	{
		if ( m_lruDataReg.lruRegVFP.IsSlotAssigned(i) )
		{
			regSakura = m_lruDataReg.lruRegVFP[i].regSakura ;
			if ( regSakura >= 0 )
			{
				ESLAssert( regSakura < 0x100 ) ;
				ESLAssert( m_lruDataReg.dprSakura[regSakura].regClass == regClassVFP ) ;
				m_lruDataReg.dprSakura[regSakura].regClass = regClassNothing ;
				m_lruDataReg.dprSakura[regSakura].regPhy = -1 ;
			}
		}
	}
	for ( i = 0; i < m_lruDataReg.lruRegNEON.SlotCount; i ++ )
	{
		if ( m_lruDataReg.lruRegNEON.IsSlotAssigned(i) )
		{
			regSakura = m_lruDataReg.lruRegNEON[i].regSakura ;
			if ( regSakura >= 0 )
			{
				ESLAssert( regSakura < 0x100 ) ;
				ESLAssert( m_lruDataReg.dprSakura[regSakura].regClass == regClassNEON ) ;
				ESLAssert( m_lruDataReg.dprSakura[regSakura^0x01].regClass == regClassNEON ) ;
				m_lruDataReg.dprSakura[regSakura].regClass = regClassNothing ;
				m_lruDataReg.dprSakura[regSakura].regPhy = -1 ;
				m_lruDataReg.dprSakura[regSakura^0x01].regClass = regClassNothing ;
				m_lruDataReg.dprSakura[regSakura^0x01].regPhy = -1 ;
			}
		}
	}
	m_lruDataReg.lruRegARM.FreeAllSlot() ;
	m_lruDataReg.lruRegVFP.FreeAllSlot() ;
	m_lruDataReg.lruRegNEON.FreeAllSlot() ;

#if	defined(__DEBUG__)
	for ( i = 0; i < 0x100; i ++ )
	{
		ESLAssert( m_lruDataReg.dprSakura[i].regClass == regClassNothing ) ;
		ESLAssert( m_lruDataReg.dprSakura[i].regPhy == -1 ) ;
		m_lruDataReg.dprSakura[i].regClass = regClassNothing ;
		m_lruDataReg.dprSakura[i].regPhy = -1 ;
	}
#endif
}

// BP ポインタ用物理レジスタ識別子取得
//////////////////////////////////////////////////////////////////////////////
int ARMGenericAssembler::GetFramePointerPhysicalRegister( void ) const
{
	return	regPtrPhyR11Index ;
}

// レジスタ値変更通知（BP 以外ポインタ用）
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::ModifiedRegister( int reg )
{
	for ( int i = 0; i < regPtrPhyCount; i ++ )
	{
		int	regCode = m_lruPointer[i].regSakura ;
		if ( ((regCode & 0xFF) == reg)
			|| (((regCode >> 8) & 0xFF) == reg) )
		{
			if ( !m_lruPointer[i].fTLBFetched )
			{
				m_lruPointer.FreeSlot( i ) ;
			}
		}
	}
}

// メモリ参照で使用する TLB スロットを取得する
//////////////////////////////////////////////////////////////////////////////
int ARMGenericAssembler::SelectTLBSlotFromMemoryOperand
	( int regBase, int regIndex, int scaleIndex )
{
	int	slotTLB = (regBase & 0x03) ;
	if ( (regIndex < 0) && (m_tlbFetch[regBase] != 0xFF) )
	{
		slotTLB = m_tlbFetch[regBase] & 0x03 ;
		ESLAssert( !(slotTLB & 0xFE) ) ;
	}
	else if ( !(slotTLB & ~0x01)
			&& m_lruPointer.IsSlotAssigned(slotTLB)
			&& m_lruPointer[slotTLB].fTLBFetched )
	{
		slotTLB |= 0x02 ;
		if ( slotTLB < regPtrPhyCount )
		{
			ESLAssert( !m_lruPointer.IsSlotAssigned(slotTLB)
						|| !m_lruPointer[slotTLB].fTLBFetched ) ;
		}
	}
	else
	{
		if ( slotTLB < regPtrPhyCount )
		{
			ESLAssert( !m_lruPointer.IsSlotAssigned(slotTLB)
						|| !m_lruPointer[slotTLB].fTLBFetched ) ;
		}
	}
	return	slotTLB ;
}

// アドレス変換して物理レジスタにアドレスをロードするコード出力
//////////////////////////////////////////////////////////////////////////////
void * ARMGenericAssembler::WriteRealizePointerRegister
	( int regPhy, int regSakura,
		RealizePointerBoundary& rpb, const void * ptrEpilogue )
{
	//
	// レジスタの値を物理レジスタにロード
	//
	int	regPhyTemp = AllocateDataRegister( regClassARM ) ;
	ESLAssert( regPhyTemp >= 0 ) ;
	LockDataRegister( regClassARM, regPhyTemp ) ;
	//
	RealizeFreeARMRegister( regPhyTemp, regSakura, true, false ) ;
	//
	// アドレス変換
	//
	void *	ptrEscJumpFrom = NULL ;
	if ( regPhy == regPtrPhyR11Index )
	{
		WriteARMLoadMemOffsetImm12
			( ARM_r11, ARM_r10,
				offsetof(Context,m_segStack.pbytBuffer) ) ;
		PreserveContinuousCodes( 0x20 ) ;
		WriteARMLoadMemOffsetImm12
			( ARM_r6, ARM_r10,
				offsetof(Context,m_segStack.baseOffset) ) ;
		WriteARMSubRegRegRegShift
			( (ARMRegister) regPhyTemp, (ARMRegister) regPhyTemp, ARM_r6 ) ;
		WriteARMAddRegRegRegShift
			( ARM_r11, ARM_r11, (ARMRegister) regPhyTemp ) ;
		//
		// メモリ境界判定
		//
		/*
		// push bp / move bp,sp / add.sp の順で実行した場合
		// add.sp でスタックの拡張が行われる条件で
		// move bp,sp がメモリアクセス例外になってしまうため除外
		//
		if ( !m_flagNoBoundary )
		{
			ptrEscJumpFrom =
				WriteToCheckBoundaryAddress
					( rpb, regPhyTemp,
						offsetof(Context,m_segStack), false ) ;
		}
		*/
	}
	else
	{
		//
		// アドレス変換
		//
		int	slotTLB = SelectTLBSlotFromMemoryOperand( regSakura ) ;
		ESLAssert( !m_lruPointer[regPhy].fTLBFetched ) ;
		ptrEscJumpFrom =
			WriteToTranslateAddress
				( m_lruPointer[regPhy],
					(ARMRegister) m_lruPointer[regPhy].regPhy,
					(ARMRegister) regPhyTemp, slotTLB ) ;
		//
		// LRU 更新
		//
		m_lruPointer.AddStale( 1 ) ;
		m_lruPointer.NewSlot( regPhy, (regSakura | 0xFF00) ) ;
	}
	if ( (ptrEscJumpFrom != NULL) && (ptrEpilogue != NULL) )
	{
		CommitJumpTarget( ptrEscJumpFrom, ptrEpilogue ) ;
	}
	UnlockDataRegister( regClassARM, regPhyTemp ) ;
	FreeDataRegister( regClassARM, regPhyTemp ) ;
	return	ptrEscJumpFrom ;
}

// 物理レジスタにロードしたポインタの境界チェックコードを完成させる
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::CommitRealizePointerRegister
	( RealizePointerBoundary& rpb, int offsetFirst, int offsetEnd )
{
	if ( rpb.offsetFirst > offsetFirst )
	{
		rpb.offsetFirst = offsetFirst ;
	}
	if ( rpb.offsetEnd < offsetEnd )
	{
		rpb.offsetEnd = offsetEnd ;
	}
	if ( !m_flagNoBoundary )
	{
		if ( rpb.pdwFirst != NULL )
		{
			*(rpb.pdwFirst) = rpb.offsetFirst ;
		}
		if ( rpb.pdwLimit != NULL )
		{
			*(rpb.pdwLimit) = rpb.offsetEnd - rpb.offsetFirst ;
		}
	}
}

// ポインタアクセス用レジスタ割り当て処理コード出力
//（物理ポインタ用レジスタ番号を返却）
//////////////////////////////////////////////////////////////////////////////
int ARMGenericAssembler::WriteAssignPointerRegister
	( int regPtr, int regIndex, int scale,
		int offsetFirst, int offsetEnd, void *& ptrEscJumpFrom )
{
	ptrEscJumpFrom = NULL ;
	if ( (regPtr == regZeroPtr) && (scale == 0) )
	{
		regPtr = regIndex ;
		regIndex = -1 ;
	}
	if ( (regPtr == regBP) && (regIndex < 0) )
	{
		return	regPtrPhyR11Index ;
	}
	const int	regCode =
		(regPtr & 0xFF) | ((regIndex & 0xFF) << 8) | (scale << 16) ;
	int	iPtrReg = m_lruPointer.FindAssignedSlot( regCode ) ;
	if ( iPtrReg < 0 )
	{
		//
		// 新規レジスタ割り当て
		//
		iPtrReg = m_lruPointer.FindFreeSlot() ;
		if ( iPtrReg < 0 )
		{
			iPtrReg = m_lruPointer.FindMostOldSlot() ;
			ESLAssert( iPtrReg >= 0 ) ;
		}
		ESLAssert( !m_lruPointer[iPtrReg].fTLBFetched ) ;
		//
		// 仮想アドレスを ARM レジスタにロード
		//
		int	regPhyTemp = AllocateDataRegister( regClassARM ) ;
		ESLAssert( regPhyTemp >= 0 ) ;
		LockDataRegister( regClassARM, regPhyTemp ) ;
		//
		WriteToLoadSakura2AddressRegister
			( (ARMRegister) regPhyTemp, regPtr, regIndex, scale ) ;
		//
		// アドレス変換
		//
		int	slotTLB = SelectTLBSlotFromMemoryOperand
									( regPtr, regIndex, scale ) ;
		ptrEscJumpFrom =
			WriteToTranslateAddress
				( m_lruPointer[iPtrReg],
					(ARMRegister) m_lruPointer[iPtrReg].regPhy,
					(ARMRegister) regPhyTemp, slotTLB ) ;
		//
		// LRU 更新
		//
		m_lruPointer.AddStale( 1 ) ;
		m_lruPointer.NewSlot( iPtrReg, regCode ) ;
		//
		// 範囲設定
		//
		m_lruPointer[iPtrReg].offsetFirst = offsetFirst ;
		m_lruPointer[iPtrReg].offsetEnd = offsetEnd ;
		if ( m_lruPointer[iPtrReg].pdwFirst != NULL )
		{
			*(m_lruPointer[iPtrReg].pdwFirst) = offsetFirst ;
		}
		if ( m_lruPointer[iPtrReg].pdwLimit != NULL )
		{
			*(m_lruPointer[iPtrReg].pdwLimit) = offsetEnd - offsetFirst ;
		}
		UnlockDataRegister( regClassARM, regPhyTemp ) ;
		FreeDataRegister( regClassARM, regPhyTemp ) ;
	}
	else
	{
		//
		// 既に割り当てられたレジスタの境界チェックコード更新
		//
		m_lruPointer.RecentlyAccess( iPtrReg ) ;
		if ( !m_lruPointer[iPtrReg].fTLBFetched )
		{
			CommitRealizePointerRegister
				( m_lruPointer[iPtrReg], offsetFirst, offsetEnd ) ;
		}
		else
		{
			// TLB にプリフェッチされている場合
			ESLAssert( !(iPtrReg & 0xFE) ) ;
			ESLAssert( regIndex < 0 ) ;
			PreserveContinuousCodes( 0x40 ) ;
			WriteToLoadSakura2Register( ARM_r6, regPtr, true ) ;
			m_lruPointer[iPtrReg].regPhyIndex = ARM_r6 ;
		}
	}
	return	iPtrReg ;
}

// Sakura2 汎用レジスタを ARM 汎用レジスタにロードするコードを出力
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteToLoadSakura2Register
	( ARMRegister regPhyARM, int regSakura, bool fOnlyLow )
{
	DataRegisterClass
		classLoaded = m_lruDataReg.dprSakura[regSakura].regClass ;
	int	regPhy = m_lruDataReg.dprSakura[regSakura].regPhy ;
	int	iSlot ;
	switch ( classLoaded )
	{
	case	regClassARM:
		iSlot = (regPhy >> 1) ;
		ESLAssert( (iSlot >= 0) & (iSlot < 3) ) ;
		ESLAssert( m_lruDataReg.lruRegARM[iSlot].regPhy == regPhy ) ;
		WriteARMMoveRegReg
			( regPhyARM, (ARMRegister) regPhy ) ;
		if ( !fOnlyLow )
		{
			WriteARMMoveRegReg
				( (ARMRegister) (regPhyARM + 1), (ARMRegister) (regPhy + 1) ) ;
		}
		break ;

	case	regClassVFP:
		iSlot = regPhy ;
		ESLAssert( (iSlot >= 0) & (iSlot < 0x10) ) ;
		ESLAssert( m_lruDataReg.lruRegVFP[iSlot].regPhy == regPhy ) ;
		if ( !fOnlyLow )
		{
			WriteMoveVFPtoARM64
				( regPhyARM, (ARMRegister) (regPhyARM + 1), regPhy ) ;
		}
		else
		{
			WriteMoveVFPtoARM32( regPhyARM, (regPhy << 1) ) ;
		}
		break ;

	case	regClassNEON:
		iSlot = regPhy - 8 ;
		ESLAssert( (iSlot >= 0) & (iSlot < 8) ) ;
		ESLAssert( m_lruDataReg.lruRegNEON[iSlot].regPhy == regPhy ) ;
		if ( !fOnlyLow )
		{
			WriteMoveVFPtoARM64
				( regPhyARM,
					(ARMRegister) (regPhyARM + 1),
					(regPhy << 1) + (regSakura & 0x01) ) ;
		}
		else
		{
			WriteBackDataRegister( classLoaded, regPhy ) ;
			//
			const size_t
				offsetReg = Context::OffsetOfReg(regSakura) ;
			WriteARMLoadMemOffsetImm12
				( regPhyARM, ARM_r10, offsetReg ) ;
			if ( !fOnlyLow )
			{
				WriteARMLoadMemOffsetImm12
					( (ARMRegister) (regPhyARM + 1), ARM_r10, offsetReg + 4 ) ;
			}
		}
		break ;

	default:
		ESLAssert( classLoaded == regClassNothing ) ;
		ESLAssert( regPhy == -1 ) ;
		{
			const size_t
				offsetReg = Context::OffsetOfReg(regSakura) ;
			if ( (offsetReg <= 0xFF) && !fOnlyLow )
			{
				WriteARMLoadDoubleMemOffsetImm8
					( regPhyARM, ARM_r10, offsetReg ) ;
			}
			else
			{
				WriteARMLoadMemOffsetImm12
					( regPhyARM, ARM_r10, offsetReg ) ;
				if ( !fOnlyLow )
				{
					WriteARMLoadMemOffsetImm12
						( (ARMRegister) (regPhyARM + 1), ARM_r10, offsetReg + 4 ) ;
				}
			}
		}
		break ;
	}
}

void ARMGenericAssembler::WriteToLoadSakura2AddressRegister
	( ARMRegister regPhyAddr, int regBasePtr, int regIndex, int scale )
{
	if ( regIndex >= 0 )
	{
		PreserveContinuousCodes( 0x20 ) ;
		WriteToLoadSakura2Register( ARM_r6, regIndex, true ) ;
		WriteToLoadSakura2Register( regPhyAddr, regBasePtr ) ;
		WriteARMAddRegRegRegShift( regPhyAddr, regPhyAddr, ARM_r6, scale ) ;
	}
	else
	{
		WriteToLoadSakura2Register( regPhyAddr, regBasePtr ) ;
	}
}

// ARM 汎用レジスタから Sakura2 汎用レジスタへストアするコードを出力
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteToStoreSakura2Register
	( int regSakura, ARMRegister regPhyARM, bool fOnlyLow )
{
	DataRegisterClass
		classLoaded = m_lruDataReg.dprSakura[regSakura].regClass ;
	int	regPhyLoaded = m_lruDataReg.dprSakura[regSakura].regPhy ;
	if ( classLoaded == regClassARM )
	{
		int	iSlot = (regPhyLoaded >> 1) ;
		ESLAssert( (iSlot >= 0) & (iSlot < 3) ) ;
		ESLAssert( m_lruDataReg.lruRegARM[iSlot].regPhy == regPhyLoaded ) ;
		//
		WriteARMMoveRegReg( (ARMRegister) regPhyLoaded, regPhyARM ) ;
		if ( !fOnlyLow )
		{
			WriteARMMoveRegReg
				( (ARMRegister) (regPhyLoaded + 1),
						(ARMRegister) (regPhyARM + 1) ) ;
		}
		SetDataRegisterModified( classLoaded, regPhyLoaded ) ;
	}
	else if ( classLoaded == regClassVFP )
	{
		int	iSlot = regPhyLoaded ;
		ESLAssert( (iSlot >= 0) & (iSlot < 0x10) ) ;
		ESLAssert( m_lruDataReg.lruRegVFP[iSlot].regPhy == regPhyLoaded ) ;
		//
		if ( !fOnlyLow )
		{
			WriteMoveARMtoVFP64
				( regPhyLoaded,
					(ARMRegister) regPhyARM,
					(ARMRegister) (regPhyARM + 1) ) ;
		}
		else
		{
			WriteMoveARMtoVFP32
				( (regPhyLoaded << 1), (ARMRegister) regPhyARM ) ;
		}
		SetDataRegisterModified( classLoaded, regPhyLoaded ) ;
	}
	else if ( (classLoaded == regClassNEON) && !fOnlyLow )
	{
		int	iSlot = regPhyLoaded - 8 ;
		ESLAssert( (iSlot >= 0) & (iSlot < 8) ) ;
		ESLAssert( m_lruDataReg.lruRegNEON[iSlot].regPhy == regPhyLoaded ) ;
		//
		WriteMoveARMtoVFP64
			( (regPhyLoaded << 1),
				(ARMRegister) regPhyARM,
				(ARMRegister) (regPhyARM + 1) ) ;
	}
	else
	{
		if ( classLoaded != regClassNothing )
		{
			WriteBackDataRegister( classLoaded, regPhyLoaded ) ;
			FreeDataRegister( classLoaded, regPhyLoaded ) ;
		}
		const size_t
			offsetReg = Context::OffsetOfReg(regSakura) ;
		if ( (offsetReg <= 0xFF) && !fOnlyLow )
		{
			WriteARMStoreDoubleMemOffsetImm8
				( regPhyARM, ARM_r10, offsetReg ) ;
		}
		else
		{
			WriteARMStoreMemOffsetImm12( regPhyARM, ARM_r10, offsetReg ) ;
			if ( !fOnlyLow )
			{
				WriteARMStoreMemOffsetImm12
					( (ARMRegister) (regPhyARM + 1), ARM_r10, offsetReg + 4 ) ;
			}
		}
	}
}

// 仮想アドレスを実アドレスに変換するコードを出力
//////////////////////////////////////////////////////////////////////////////
void * ARMGenericAssembler::WriteToTranslateAddress
	( RealizePointerBoundary& rpb,
		ARMRegister regPhyBase, ARMRegister regPhyAddr, int slotTLB )
{
	//
	// TLB 比較
	//
	PreserveContinuousCodes( 0x40 ) ;
	WriteARMLoadMemOffsetImm12
		( ARM_r6, ARM_r10,
			Context::OffsetofStoreCache_baseOffset( slotTLB ) ) ;
	WriteARMLoadMemOffsetImm12
		( regPhyBase, ARM_r10,
			Context::OffsetofStoreCache_pbytBuffer(slotTLB) ) ;
	WriteARMSubRegRegRegShift( regPhyAddr, regPhyAddr, ARM_r6 ) ;
	WriteARMLoadMemOffsetImm12
		( ARM_r6, ARM_r10,
			Context::OffsetofStoreCache_highAddress(slotTLB) ) ;
	WriteARMCmpRegRegShift( (ARMRegister) (regPhyAddr + 1), ARM_r6 ) ;
	ESLAssert( m_bufSub != NULL ) ;
	WriteARMJumpImm( m_bufSub->GetNext(), cond_NE ) ;
	void *	pNextAddr = GetNextAddress() ;
	m_buf = m_bufSub ;
	//
	// 第二 TLB 比較
	//
	PreserveContinuousCodes( 0x40 ) ;
	WriteARMLoadMemOffsetImm12
		( ARM_r6, ARM_r10,
			Context::OffsetofStoreCache_baseOffset( slotTLB ) ) ;
	WriteARMMoveRegImm( regPhyBase, 0x03 ) ;
	WriteARMAddRegRegRegShift( regPhyAddr, regPhyAddr, ARM_r6 ) ;
	WriteARMAndRegRegRegShift
		( regPhyBase, regPhyBase, (ARMRegister) (regPhyAddr + 1) ) ;
	ESLAssert( (1 << 4) == sizeof(LinearAddressCache) ) ;
	WriteARMAddRegRegRegShift( regPhyBase, ARM_r10, regPhyBase, 4 ) ;
	WriteARMLoadMemOffsetImm12
		( ARM_r6, regPhyBase,
				offsetof(Context,m_segLoadCache[0].highAddress) ) ;
	WriteARMCmpRegRegShift( (ARMRegister) (regPhyAddr + 1), ARM_r6 ) ;
	void *	pJeTLB2 = WriteARMJumpImm32( NULL, cond_EQ ) ;
	//
	// アドレス変換処理
	//
	WriteBackAllRegisters() ;
	//
	ARMRegister	regsPush[16] ;
	int			iPushNext = 0 ;
	if ( m_lruPointer.IsSlotAssigned( regPtrPhyR9 )
								&& (regPhyBase != ARM_r9) )
	{
		regsPush[iPushNext ++] = ARM_r9 ;
	}
	if ( m_lruPointer.IsSlotAssigned( regPtrPhyR12 )
								&& (regPhyBase != ARM_r12) )
	{
		regsPush[iPushNext ++] = ARM_r12 ;
	}
	regsPush[iPushNext ++] = regPhyAddr ;
	regsPush[iPushNext ++] = (ARMRegister) (regPhyAddr + 1) ;
	WriteARMPushRegs( regsPush, iPushNext ) ;
	//
	WriteARMMoveRegReg( ARM_r2, regPhyAddr ) ;
	WriteARMMoveRegReg( ARM_r3, (ARMRegister) (regPhyAddr + 1) ) ;
	WriteARMMoveRegReg( ARM_r0, ARM_r10 ) ;
	WriteARMAddRegRegImm
		( ARM_r1, regPhyBase,
			offsetof(Context,m_segLoadCache[0]), ARM_r6 ) ;
	//
	PreserveContinuousCodes( 0x10 ) ;
	WriteARMLoadMemOffsetImm12
		( ARM_r6, ARM_r10, offsetof(Context,m_pfnTranslate) ) ;
	WriteARMCallReg( ARM_r6 ) ;
	//
	WriteARMPopRegs( regsPush, iPushNext ) ;
	//
	ReloadRegisters() ;
	//
	// 第二 TLB 複製
	//
	CommitJumpTarget( pJeTLB2, GetNextAddress() ) ;
	//
	WriteToCopyMemory
		( true, ARM_r10, Context::OffsetofStoreCache(slotTLB),
			true, regPhyBase, offsetof(Context,m_segLoadCache[0]),
			sizeof(LinearAddressCache) / sizeof(DWORD),
			(ARMRegister) (regPhyAddr + 1) ) ;
	//
	// TLB ベースアドレスをロード
	//
	WriteARMLoadMemOffsetImm12
		( regPhyBase, ARM_r10,
			Context::OffsetofStoreCache_pbytBuffer(slotTLB) ) ;
	PreserveContinuousCodes( 0x20 ) ;
	WriteARMLoadMemOffsetImm12
		( ARM_r6, ARM_r10,
			Context::OffsetofStoreCache_baseOffset(slotTLB) ) ;
	WriteARMSubRegRegRegShift( regPhyAddr, regPhyAddr, ARM_r6 ) ;
	WriteToJump( pNextAddr ) ;
	m_buf = m_bufMain ;
	//
	WriteARMAddRegRegRegShift( regPhyBase, regPhyBase, regPhyAddr ) ;
	//
	// メモリ境界判定
	//
	return	WriteToCheckBoundaryAddress
		( rpb, regPhyAddr, (ARMRegister) (regPhyAddr + 1),
			(int) Context::OffsetofStoreCache(slotTLB), false ) ;
}

// メモリ境界を判定するコードを出力
//////////////////////////////////////////////////////////////////////////////
void * ARMGenericAssembler::WriteToCheckBoundaryAddress
	( RealizePointerBoundary& rpb,
		ARMRegister regPhyLow, ARMRegister regPhyTemp,
		int offsetTLB, bool fBaseOffset )
{
	void *	pEscFrom = NULL ;
	rpb.pdwFirst = NULL ;
	rpb.pdwLimit = NULL ;
	if ( !m_flagNoBoundary )
	{
		FlushAllRegisters() ;
		//
		if ( fBaseOffset )
		{
			PreserveContinuousCodes( 0x10 ) ;
			WriteARMLoadMemOffsetImm12
				( ARM_r6, ARM_r10,
					offsetTLB + offsetof(LinearAddressCache,baseOffset) ) ;
			WriteARMSubRegRegRegShift( regPhyLow, regPhyLow, ARM_r6 ) ;
		}
		PreserveContinuousCodes( 0x40 ) ;
		rpb.pdwFirst = (SDWORD*) WriteARMMoveRegImm32( ARM_r6, 0 ) ;
		WriteARMAddRegRegRegShift( regPhyLow, regPhyLow, ARM_r6 ) ;
		rpb.pdwLimit = (SDWORD*) WriteARMMoveRegImm32( ARM_r6, 0 ) ;
		WriteARMMoveRegReg( regPhyTemp, regPhyLow ) ;
		WriteARMShiftARightImm( regPhyLow, regPhyLow, 31 ) ;
		WriteARMAddRegRegRegShift( regPhyTemp, regPhyTemp, ARM_r6 ) ;
		PreserveContinuousCodes( 0x20 ) ;
		WriteARMLoadMemOffsetImm12
			( ARM_r6, ARM_r10,
				offsetTLB + offsetof(LinearAddressCache,limitSegment) ) ;
		WriteARMOrRegRegRegShift( regPhyLow, regPhyLow, regPhyTemp ) ;
		WriteARMCmpRegRegShift( regPhyLow, ARM_r6 ) ;
		pEscFrom = WriteARMJumpImm32( NULL, cond_HI ) ;
	}
	return	pEscFrom ;
}

// メモリを複製するコードを出力
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteToCopyMemory
	( bool fDstAligned,
		ARMRegister regPhyDst, INT_PTR dispDst,
		bool fSrcAligned,
			ARMRegister regPhySrc, INT_PTR dispSrc,
		int	sizeInDWord,  ARMRegister regPhyTemp )
{
	for ( int i = 0, iOffset = 0; i < sizeInDWord; i ++, iOffset += 4 )
	{
		PreserveContinuousCodes( 0x10 ) ;
		WriteARMLoadMemOffsetImm12
			( regPhyTemp, regPhySrc, dispSrc + iOffset ) ;
		WriteARMStoreMemOffsetImm12
			( regPhyTemp, regPhyDst, dispDst + iOffset ) ;
	}
}


// 例外マスクに追加するコードを出力
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteToAtomicOrExceptionMask( DWORD dwException )
{
	if ( dwException != 0 )
	{
		const int	regTemp = AllocateDataRegister( regClassARM ) ;
		ARMRegister	regTemp1 = (ARMRegister) regTemp ;
		ARMRegister	regTemp2 = (ARMRegister) (regTemp + 1) ;
		//
		PreserveContinuousCodes( 0x40 ) ;
		long_ptr_t	ptrLoopLabel = (long_ptr_t) GetNextAddress() ;
		//
		WriteARMAddRegRegImm
			( ARM_r6, ARM_r10, offsetof(Context,m_maskException), ARM_r6 ) ;
		WriteARMMoveRegImm( regTemp2, dwException ) ;
		WriteARMLoadMemEx( regTemp1, ARM_r6 ) ;
		WriteARMOrRegRegRegShift( regTemp1, regTemp1, regTemp2 ) ;
		WriteARMStoreMemEx( regTemp2, regTemp1, ARM_r6 ) ;
		WriteARMClrEx() ;
		WriteARMCmpRegImm8( regTemp2, 0 ) ;
		//
		long_ptr_t	ptrJumpPrev = (long_ptr_t) GetNextAddress() ;
		WriteARMJumpOffsetImm( (ptrLoopLabel - ptrJumpPrev), cond_NE ) ;
		//
		FreeDataRegister( regClassARM, regTemp ) ;
	}
}

// スタック拡張例外判定
//////////////////////////////////////////////////////////////////////////////
void * ARMGenericAssembler::WriteToStackException
	( int& regPhyNewLowSP, int nOffsetSP, const void * ptrEpilogue )
{
	FlushAllRegisters() ;
	//
	const int	regTemp = AllocateDataRegister( regClassARM ) ;
	ARMRegister	regLowSP = (ARMRegister) regTemp ;
	ARMRegister	regTemp2 = (ARMRegister) (regTemp + 1) ;
	regPhyNewLowSP = regTemp ;
	//
	WriteToLoadSakura2Register( regLowSP, regSP, true ) ;
	if ( nOffsetSP != 0 )
	{
		if ( (nOffsetSP > 0) && (nOffsetSP < 0xFF) )
		{
			WriteARMAddRegRegImm8( regLowSP, regLowSP, nOffsetSP ) ;
		}
		else if ( (nOffsetSP < 0) && (nOffsetSP > -0xFF) )
		{
			WriteARMSubRegRegImm8( regLowSP, regLowSP, -nOffsetSP ) ;
		}
		else
		{
			WriteARMMoveRegImm( regTemp2, nOffsetSP ) ;
			WriteARMAddRegRegRegShift( regLowSP, regLowSP, regTemp2 ) ;
		}
	}
	WriteARMLoadMemOffsetImm12
		( regTemp2, ARM_r10, offsetof(Context,m_segStack.baseOffset) ) ;
	WriteARMCmpRegRegShift( regTemp2, regLowSP ) ;
	void *	pEscJumpFrom = WriteARMJumpImm32( NULL, cond_HI ) ;
	if ( ptrEpilogue != NULL )
	{
		CommitJumpTarget( pEscJumpFrom, ptrEpilogue ) ;
	}
	return	pEscJumpFrom ;
}

// ゼロ除算例外用ゼロ比較脱出コード出力
//////////////////////////////////////////////////////////////////////////////
void * ARMGenericAssembler::WriteToZeroDivisionException
	( int regDiv, const void * ptrEpilogue )
{
	const int	regTemp = AllocateDataRegister( regClassARM ) ;
	WriteToLoadSakura2Register( (ARMRegister) regTemp, regDiv ) ;
	WriteARMOrRegRegRegShift
		( (ARMRegister) regTemp,
			(ARMRegister) regTemp,
			(ARMRegister) (regTemp + 1), 0, cond_AL, true ) ;
	void *	pJneNoEsc = WriteARMJumpImm32( NULL, cond_NE ) ;
	//
	WriteBackAllRegisters() ;
	void *	pEscJumpFrom = WriteARMJumpImm32( ptrEpilogue ) ;
	//
	CommitJumpTarget( pJneNoEsc, GetNextAddress() ) ;
	FreeDataRegister( regClassARM, regTemp ) ;
	return	pEscJumpFrom ;
}

void * ARMGenericAssembler::WriteToZeroDivisionException32
	( int regDiv, const void * ptrEpilogue )
{
	const int	regTemp = AllocateDataRegister( regClassARM ) ;
	WriteToLoadSakura2Register( (ARMRegister) regTemp, regDiv, true ) ;
	WriteARMOrRegRegRegShift
		( (ARMRegister) regTemp,
			(ARMRegister) regTemp,
			(ARMRegister) regTemp, 0, cond_AL, true ) ;
	void *	pJneNoEsc = WriteARMJumpImm32( NULL, cond_NE ) ;
	//
	WriteBackAllRegisters() ;
	void *	pEscJumpFrom = WriteARMJumpImm32( ptrEpilogue ) ;
	//
	CommitJumpTarget( pJneNoEsc, GetNextAddress() ) ;
	FreeDataRegister( regClassARM, regTemp ) ;
	return	pEscJumpFrom ;
}

// メモリ読み込み命令出力
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteToLoadPhysicalMemory
	( int regDst, int regPhyPtr,
			int offset, DataType type, bool fPair )
{
	//
	// ARM 命令 [base+offset] 形式への正規化
	//
	const int	sizeOfData = sizeof_prim_data[type] ;
	int			sizeOfLoadData = sizeOfData ;
	if ( fPair && (type == dataInt64) )
	{
		sizeOfLoadData *= 2 ;
	}
	ARMRegister	regTempAddr = ARM_Nothing ;
	ARMRegister	regTemp2 ;
	ARMRegister	regPhyBase = ARM_r11 ;
	ARMRegister	regPhyIndex = ARM_Nothing ;
	bool		fOffsetOverflow =
					(fPair && (type == dataInt64))
						| ((offset <= -0x100) & m_modeThumb)
						| (offset <= -0x1000)
						| (offset + sizeOfLoadData >= 0x1000) ;
	if ( regPhyPtr != regPtrPhyR11Index )
	{
		// バウンダリチェック用数値更新
		CommitRealizePointerRegister
			( m_lruPointer[regPhyPtr],
				offset, offset + sizeOfLoadData ) ;
		//
		// ベースレジスタ正規化
		regPhyBase = (ARMRegister) m_lruPointer[regPhyPtr].regPhy ;
		if ( fOffsetOverflow )
		{
			regTempAddr = (ARMRegister) AllocateDataRegister( regClassARM ) ;
			WriteARMMoveRegImm( regTempAddr, offset ) ;
			WriteARMAddRegRegRegShift( regTempAddr, regTempAddr, regPhyBase ) ;
			regPhyBase = regTempAddr ;
			offset = 0 ;
		}
		if ( m_lruPointer[regPhyPtr].fTLBFetched )
		{
			regPhyIndex = (ARMRegister) m_lruPointer[regPhyPtr].regPhyIndex ;
			m_lruPointer[regPhyPtr].regPhyIndex = ARM_Nothing ;
			//
			if ( regPhyIndex != ARM_Nothing )
			{
				if ( regTempAddr == ARM_Nothing )
				{
					regTempAddr =
						(ARMRegister) AllocateDataRegister( regClassARM ) ;
				}
				WriteARMAddRegRegRegShift
					( regTempAddr, regPhyBase, regPhyIndex ) ;
				regPhyBase = regTempAddr ;
				regPhyIndex = ARM_Nothing ;
			}
		}
	}
	else
	{
		// ベースレジスタ正規化
		if ( fOffsetOverflow )
		{
			regTempAddr = (ARMRegister) AllocateDataRegister( regClassARM ) ;
			WriteARMMoveRegImm( regTempAddr, offset ) ;
			WriteARMAddRegRegRegShift( regTempAddr, regTempAddr, regPhyBase ) ;
			regPhyBase = regTempAddr ;
			offset = 0 ;
		}
	}
	if ( fPair && (type == dataInt64) )
	{
		//
		// 128bit ロード
		//
		ESLAssert( offset == 0 ) ;
		if ( m_vfpNEON )
		{
			int	qreg0 =
				WriteRealizeDataRegister( regDst, regClassNEON, false ) ;
			//
			int	regARMTemp =
				(ARMRegister) AllocateDataRegister( regClassARM ) ;
			ARMRegister	regPhy1 = (ARMRegister) regARMTemp ;
			ARMRegister	regPhy2 = (ARMRegister) (regARMTemp + 1) ;
			//
			for ( size_t i = 0; i < 2; i ++ )
			{
				WriteARMLoadMemOffsetImm12
					( regPhy1, regPhyBase, offset + (i * 8) ) ;
				WriteARMLoadMemOffsetImm12
					( regPhy2, regPhyBase, offset + (i * 8) + 4 ) ;
				WriteMoveARMtoVFP64
					( (qreg0 << 1) + i, regPhy1, regPhy2 ) ;
			}
			FreeDataRegister( regClassARM, regARMTemp ) ;
			//
			SetDataRegisterModified( regClassNEON, qreg0 ) ;
			UnlockDataRegister( regClassNEON, qreg0 ) ;
		}
		else if ( m_vfpVersion >= 2 )
		{
			int	regARMTemp =
				(ARMRegister) AllocateDataRegister( regClassARM ) ;
			ARMRegister	regPhy1 = (ARMRegister) regARMTemp ;
			ARMRegister	regPhy2 = (ARMRegister) (regARMTemp + 1) ;
			//
			for ( size_t i = 0; i < 2; i ++ )
			{
				int	vreg =
					WriteRealizeDataRegister
						( regDst + i, regClassVFP, false ) ;
				WriteARMLoadMemOffsetImm12
					( regPhy1, regPhyBase, offset + (i * 8) ) ;
				WriteARMLoadMemOffsetImm12
					( regPhy2, regPhyBase, offset + (i * 8) + 4 ) ;
				WriteMoveARMtoVFP64
					( vreg, regPhy1, regPhy2 ) ;
				SetDataRegisterModified( regClassVFP, vreg ) ;
				UnlockDataRegister( regClassVFP, vreg ) ;
			}
			FreeDataRegister( regClassARM, regARMTemp ) ;
		}
		else
		{
			for ( size_t i = 0; i < 2; i ++ )
			{
				int	reg =
					WriteRealizeDataRegister
						( regDst + i, regClassARM, false ) ;
				//
				WriteARMLoadMemOffsetImm12
					( (ARMRegister) reg,
						regPhyBase, offset + (i * 8) ) ;
				WriteARMLoadMemOffsetImm12
					( (ARMRegister) (reg + 1),
						regPhyBase, offset + (i * 8) + 4 ) ;
				//
				SetDataRegisterModified( regClassARM, reg ) ;
				UnlockDataRegister( regClassARM, reg ) ;
			}
		}
	}
	else if ( fPair && (type == dataUint32) )
	{
		//
		// 32bit -> 128bit ゼロ拡張ロード
		//
		if ( m_vfpVersion >= 2 )
		{
			if ( regTempAddr == ARM_Nothing )
			{
				regTempAddr =
					(ARMRegister) AllocateDataRegister( regClassARM ) ;
			}
			regTemp2 = (ARMRegister) (regTempAddr + 1) ;
			//
			WriteARMLoadMemOffsetImm12
				( regTempAddr, regPhyBase, offset ) ;
			WriteARMXorRegRegRegShift
				( regTemp2, regTemp2, regTemp2 ) ;
			//
			if ( m_vfpNEON )
			{
				int	qregPhy =
					WriteRealizeDataRegister( regDst, regClassNEON, false ) ;
				int	vregPhyLow = (qregPhy << 1) ;
				int	vregPhyHigh = vregPhyLow | 0x01 ;
				//
				WriteSIMDOpRegRegReg
					( armOpVEOR, thumbOpVEOR,
						vregPhyHigh, vregPhyHigh, vregPhyHigh, false ) ;
				WriteMoveARMtoVFP64
					( vregPhyLow, regTempAddr, regTemp2 ) ;
				//
				SetDataRegisterModified( regClassNEON, qregPhy ) ;
				UnlockDataRegister( regClassNEON, qregPhy ) ;
			}
			else
			{
				int	vregPhy1 =
					WriteRealizeDataRegister( regDst, regClassVFP, false ) ;
				int	vregPhy2 =
					WriteRealizeDataRegister( regDst + 1, regClassVFP, false ) ;
				//
				WriteMoveARMtoVFP64( vregPhy1, regTempAddr, regTemp2 ) ;
				//
				WriteARMXorRegRegRegShift
					( regTempAddr, regTempAddr, regTempAddr ) ;
				WriteMoveARMtoVFP64( vregPhy2, regTempAddr, regTemp2 ) ;
				//
				SetDataRegisterModified( regClassVFP, vregPhy1 ) ;
				SetDataRegisterModified( regClassVFP, vregPhy2 ) ;
				UnlockDataRegister( regClassVFP, vregPhy1 ) ;
				UnlockDataRegister( regClassVFP, vregPhy2 ) ;
			}
		}
		else
		{
			int	regARM =
				WriteRealizeDataRegister( regDst, regClassARM, false ) ;
			//
			WriteARMLoadMemOffsetImm12
				( (ARMRegister) regARM, regPhyBase, offset ) ;
			WriteARMXorRegRegRegShift
				( (ARMRegister) (regARM + 1),
					(ARMRegister) (regARM + 1), (ARMRegister) (regARM + 1) ) ;
			//
			SetDataRegisterModified( regClassARM, regARM ) ;
			UnlockDataRegister( regClassARM, regARM ) ;
			//
			regARM =
				WriteRealizeDataRegister( regDst + 1, regClassARM, false ) ;
			//
			WriteARMXorRegRegRegShift
				( (ARMRegister) regARM,
					(ARMRegister) regARM, (ARMRegister) regARM ) ;
			WriteARMXorRegRegRegShift
				( (ARMRegister) (regARM + 1),
					(ARMRegister) (regARM + 1), (ARMRegister) (regARM + 1) ) ;
			//
			SetDataRegisterModified( regClassARM, regARM ) ;
			UnlockDataRegister( regClassARM, regARM ) ;
		}
	}
	else
	{
		//
		// 64bit 以下サイズロード
		//
		ESLAssert( !fPair ) ;
		if ( (type == dataInt16)
				| (type == dataInt8) | (type == dataUint16) )
		{
			if ( (offset <= -0x100) | (offset >= 0x100) )
			{
				if ( regTempAddr == ARM_Nothing )
				{
					regTempAddr =
						(ARMRegister) AllocateDataRegister( regClassARM ) ;
				}
				if ( offset > 0 )
				{
					WriteARMAddRegRegImm
						( regTempAddr, regPhyBase, offset, ARM_r6 ) ;
				}
				else
				{
					WriteARMSubRegRegImm
						( regTempAddr, regPhyBase, -offset, ARM_r6 ) ;
				}
				regPhyBase = regTempAddr ;
				offset = 0 ;
			}
		}
		if ( type != dataFloat )
		{
			//
			// 整数のロード／符号の拡張
			//
			int	regARM ;
			if ( m_vfpNEON )
			{
				regARM = AllocateDataRegister( regClassARM ) ;
			}
			else
			{
				regARM = WriteRealizeDataRegister
								( regDst, regClassARM, false ) ;
			}
			ARMRegister	regPhy1 = (ARMRegister) regARM ;
			ARMRegister	regPhy2 = (ARMRegister) (regARM + 1) ;
			//
			switch ( type )
			{
			case	dataInt64:
			default:
				WriteARMLoadMemOffsetImm12( regPhy1, regPhyBase, offset ) ;
				WriteARMLoadMemOffsetImm12( regPhy2, regPhyBase, offset + 4 ) ;
				break ;
			case	dataInt32:
			case	dataInt16:
			case	dataInt8:
				WriteARMLoadMemOffsetImm12( regPhy1, regPhyBase, offset, type ) ;
				WriteARMShiftARightImm( regPhy2, regPhy1, 31 ) ;
				break ;
			case	dataFloat:
			case	dataUint32:
			case	dataUint16:
			case	dataUint8:
				WriteARMLoadMemOffsetImm12( regPhy1, regPhyBase, offset, type ) ;
				WriteARMXorRegRegRegShift( regPhy2, regPhy2, regPhy2 ) ;
				break ;
			}
			if ( m_vfpNEON )
			{
				const int	vreg =
					WriteRealizeDataRegister( regDst, regClassVFP, false ) ;
				//
				WriteMoveARMtoVFP64( vreg, regPhy1, regPhy2 ) ;
				//
				SetDataRegisterModified( regClassVFP, vreg ) ;
				UnlockDataRegister( regClassVFP, vreg ) ;
				FreeDataRegister( regClassARM, regARM ) ;
			}
			else
			{
				SetDataRegisterModified( regClassARM, regARM ) ;
				UnlockDataRegister( regClassARM, regARM ) ;
			}
		}
		else
		{
			//
			// 単精度浮動小数点から倍精度浮動小数点へ変換
			//
			if ( regTempAddr == ARM_Nothing )
			{
				regTempAddr =
					(ARMRegister) AllocateDataRegister( regClassARM ) ;
			}
			regTemp2 = (ARMRegister) (regTempAddr + 1) ;
			//
			WriteARMLoadMemOffsetImm12
				( regTempAddr, regPhyBase, offset, dataUint32 ) ;
			//
			if ( m_vfpVersion >= 2 )
			{
				ESLAssert( regDst >= 0 ) ;
				const int	vreg =
					WriteRealizeDataRegister( regDst, regClassVFP, false ) ;
				//
				WriteMoveARMtoVFP32
					( (vreg << 1), (ARMRegister) regTempAddr ) ;
				WriteCvtVFP32to64( vreg, (vreg << 1) ) ;
				//
				SetDataRegisterModified( regClassVFP, vreg ) ;
				UnlockDataRegister( regClassVFP, vreg ) ;
			}
			else
			{
				const int	regARM =
					WriteRealizeDataRegister( regDst, regClassARM, false ) ;
				ARMRegister	regPhy1 = (ARMRegister) regARM ;
				ARMRegister	regPhy2 = (ARMRegister) (regARM + 1) ;
				//
				WriteARMXorRegRegRegShift( regPhy1, regPhy1, regPhy1 ) ;
				WriteARMXorRegRegRegShift( regPhy2, regPhy2, regPhy2 ) ;
				WriteARMCmpRegImm8( regTempAddr, 0 ) ;
				void *	pJmpIfZero = WriteARMJumpImm32( NULL, cond_EQ ) ;
				//
				WriteARMMoveRegImm( regPhy2, 0xFF ) ;		// 指数部
				WriteARMMoveRegImm( regPhy1, 0x7FFFFF ) ;	// 仮数部
				WriteARMAndRegRegRegShift
					( regPhy2, regPhy2, regTempAddr, -23 ) ;
				WriteARMAndRegRegRegShift
					( regPhy1, regPhy1, regTempAddr ) ;
				WriteARMShiftRightImm
					( regTempAddr, regTempAddr, 31 ) ;		// 符号部
				WriteARMAddRegRegImm( regPhy2, regPhy2, 1023 - 127, ARM_r6 ) ;
				WriteARMShiftLeftImm( regTempAddr, regTempAddr, 31 ) ;
				WriteARMShiftLeftImm( regPhy2, regPhy2, 32 - 11 ) ;
				//
				WriteARMOrRegRegRegShift( regPhy2, regTempAddr, regPhy2, -1 ) ;
				WriteARMOrRegRegRegShift( regPhy2, regPhy2, regPhy1, -3 ) ;
				WriteARMShiftLeftImm( regPhy1, regPhy1, 32 - 3 ) ;
				//
				CommitJumpTarget( pJmpIfZero, GetNextAddress() ) ;
				//
				SetDataRegisterModified( regClassARM, regARM ) ;
				UnlockDataRegister( regClassARM, regARM ) ;
			}
		}
	}
	if ( regTempAddr != ARM_Nothing )
	{
		FreeDataRegister( regClassARM, regTempAddr ) ;
	}
}


// メモリ書き出し命令出力
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WriteToStorePhysicalMemory
	( int regSrc, int regPhyPtr,
			int offset, DataType type, bool fPair )
{
	//
	// ARM 命令 [base+offset] 形式への正規化
	//
	const int	sizeOfData = sizeof_prim_data[type] ;
	int			sizeOfLoadData = sizeOfData ;
	if ( fPair && (type == dataInt64) )
	{
		sizeOfLoadData *= 2 ;
	}
	ARMRegister	regTempAddr = ARM_Nothing ;
	ARMRegister	regTemp2 ;
	ARMRegister	regPhyBase = ARM_r11 ;
	ARMRegister	regPhyIndex = ARM_Nothing ;
	bool		fOffsetOverflow =
					((offset <= -0x100) & m_modeThumb)
						| (offset <= -0x1000)
						| (offset + sizeOfLoadData >= 0x1000) ;
	if ( regPhyPtr != regPtrPhyR11Index )
	{
		// バウンダリチェック用数値更新
		CommitRealizePointerRegister
			( m_lruPointer[regPhyPtr],
				offset, offset + sizeOfLoadData ) ;
		//
		// ベースレジスタ正規化
		regPhyBase = (ARMRegister) m_lruPointer[regPhyPtr].regPhy ;
		if ( fOffsetOverflow )
		{
			regTempAddr = (ARMRegister) AllocateDataRegister( regClassARM ) ;
			WriteARMMoveRegImm( regTempAddr, offset ) ;
			WriteARMAddRegRegRegShift( regTempAddr, regTempAddr, regPhyBase ) ;
			regPhyBase = regTempAddr ;
			offset = 0 ;
		}
		if ( m_lruPointer[regPhyPtr].fTLBFetched )
		{
			regPhyIndex = (ARMRegister) m_lruPointer[regPhyPtr].regPhyIndex ;
			m_lruPointer[regPhyPtr].regPhyIndex = ARM_Nothing ;
			//
			if ( regPhyIndex != ARM_Nothing )
			{
				if ( regTempAddr == ARM_Nothing )
				{
					regTempAddr =
						(ARMRegister) AllocateDataRegister( regClassARM ) ;
				}
				WriteARMAddRegRegRegShift
					( regTempAddr, regPhyBase, regPhyIndex ) ;
				regPhyBase = regTempAddr ;
				regPhyIndex = ARM_Nothing ;
			}
		}
	}
	else
	{
		// ベースレジスタ正規化
		if ( fOffsetOverflow )
		{
			regTempAddr = (ARMRegister) AllocateDataRegister( regClassARM ) ;
			WriteARMMoveRegImm( regTempAddr, offset ) ;
			WriteARMAddRegRegRegShift( regTempAddr, regTempAddr, regPhyBase ) ;
			regPhyBase = regTempAddr ;
			offset = 0 ;
		}
	}
	//
	// オフセットアドレスの正規化
	//
	if ( (type == dataInt16) | (type == dataUint16) )
	{
		if ( (offset <= -0x100) | (offset >= 0x100) )
		{
			if ( regTempAddr == ARM_Nothing )
			{
				regTempAddr =
					(ARMRegister) AllocateDataRegister( regClassARM ) ;
			}
			if ( offset > 0 )
			{
				WriteARMAddRegRegImm
					( regTempAddr, regPhyBase, offset, ARM_r6 ) ;
			}
			else
			{
				WriteARMSubRegRegImm
					( regTempAddr, regPhyBase, -offset, ARM_r6 ) ;
			}
			regPhyBase = regTempAddr ;
			offset = 0 ;
		}
	}
	if ( type != dataFloat )
	{
		//
		// 整数ストア
		//
		const int	countPair = fPair ? 2 : 1 ;
		for ( int i = 0; i < countPair; i ++ )
		{
			ARMRegister	regARMData ;
			ARMRegister	regARMTemp = ARM_Nothing ;
			DataRegisterClass
				classLoaded = m_lruDataReg.dprSakura[regSrc+i].regClass ;
			int	regPhy = m_lruDataReg.dprSakura[regSrc+i].regPhy ;
			if ( classLoaded == regClassARM )
			{
				regARMData = (ARMRegister) regPhy ;
			}
			else
			{
				regARMTemp =
					(ARMRegister) AllocateDataRegister( regClassARM ) ;
				regARMData = regARMTemp ;
				//
				WriteToLoadSakura2Register
					( regARMTemp, regSrc + i, (type != dataInt64) ) ;
			}
			//
			WriteARMStoreMemOffsetImm12
				( regARMData, regPhyBase, offset + (i * 8), type ) ;
			if ( type == dataInt64 )
			{
				WriteARMStoreMemOffsetImm12
					( (ARMRegister) (regARMData + 1),
							regPhyBase, offset + (i * 8) + 4 ) ;
			}
			if ( regARMTemp != ARM_Nothing )
			{
				FreeDataRegister( regClassARM, regARMTemp ) ;
			}
		}
	}
	else
	{
		//
		// 倍精度浮動小数点から単精度浮動小数点へ変換
		//
		ESLAssert( !fPair ) ;
		if ( m_vfpVersion >= 2 )
		{
			int	vreg = WriteRealizeDataRegister( regSrc, regClassVFP ) ;
			int	vregTemp = AllocateDataRegister( regClassVFP ) ;
			int	regARMTemp = AllocateDataRegister( regClassARM ) ;
			//
			WriteCvtVFP64to32( (vregTemp << 1), vreg ) ;
			WriteMoveVFPtoARM32( (ARMRegister) regARMTemp, (vregTemp << 1) ) ;
			WriteARMStoreMemOffsetImm12
				( (ARMRegister) regARMTemp, regPhyBase, offset, type ) ;
			//
			UnlockDataRegister( regClassVFP, vreg ) ;
			FreeDataRegister( regClassVFP, vregTemp ) ;
			FreeDataRegister( regClassARM, regARMTemp ) ;
		}
		else
		{
			if ( regTempAddr == ARM_Nothing )
			{
				regTempAddr =
					(ARMRegister) AllocateDataRegister( regClassARM ) ;
			}
			regTemp2 = (ARMRegister) (regTempAddr + 1) ;
			//
			ARMRegister	regSrc1 =
				(ARMRegister) AllocateDataRegister( regClassARM ) ;
			ARMRegister	regSrc2 = (ARMRegister) (regSrc1 + 1) ;
			WriteToLoadSakura2Register( regSrc1, regSrc ) ;
			//
			WriteARMShiftRightImm( regSrc1, regSrc1, 32-12 ) ;	// 仮数部
			WriteARMOrRegRegRegShift( regSrc1, regSrc1, regSrc2, 12 ) ;
			WriteARMShiftLeftImm( regTemp2, regSrc2, 1 ) ;		// 指数部
			WriteARMShiftRightImm( regTemp2, regTemp2, 32-11 ) ;
			WriteARMSubRegRegImm( regTemp2, regTemp2, 1023 - 127, ARM_r6 ) ;
			WriteARMClampValueImm8( regTemp2, 0, 0xFF, ARM_r6 ) ;
			WriteARMShiftRightImm( regSrc2, regSrc2, 31 ) ;		// 符号部
			WriteARMShiftLeftImm( regSrc2, regSrc2, 31 ) ;
			WriteARMOrRegRegRegShift( regSrc2, regSrc2, regTemp2, 23 ) ;
			WriteARMOrRegRegRegShift( regSrc2, regSrc2, regSrc1, -9 ) ;
			//
			WriteARMStoreMemOffsetImm12( regSrc2, regPhyBase, offset ) ;
			//
			FreeDataRegister( regClassARM, regSrc1 ) ;
		}
	}
	if ( regTempAddr != ARM_Nothing )
	{
		FreeDataRegister( regClassARM, regTempAddr ) ;
	}
}

// 無条件ジャンプコード出力
//////////////////////////////////////////////////////////////////////////////
void * ARMGenericAssembler::WriteToJump( const void * ptrTarget )
{
	return	WriteARMJumpImm32( ptrTarget ) ;
}

// 条件ジャンプコード出力
//////////////////////////////////////////////////////////////////////////////
void * ARMGenericAssembler::WriteToConditionalJump
		( int reg, bool fLogic, const void * ptrTarget )
{
	DataRegisterClass
		classLoaded = m_lruDataReg.dprSakura[reg].regClass ;
	int	regPhy = m_lruDataReg.dprSakura[reg].regPhy ;
	if ( classLoaded == regClassARM )
	{
		WriteARMTestRegImm8( (ARMRegister) regPhy, 1 ) ;
	}
	else
	{
		ARMRegister	regTemp =
			(ARMRegister) AllocateDataRegister( regClassARM ) ;
		WriteToLoadSakura2Register( regTemp, reg, true ) ;
		WriteARMTestRegImm8( regTemp, 1 ) ;
		FreeDataRegister( regClassARM, regTemp ) ;
	}
	void *	pJumpFrom = NULL ;
	if ( fLogic )
	{
		pJumpFrom = WriteARMJumpImm32( ptrTarget, cond_NE ) ;
	}
	else
	{
		pJumpFrom = WriteARMJumpImm32( ptrTarget, cond_EQ ) ;
	}
	return	pJumpFrom ;
}

// 例外判定離脱コード出力
//////////////////////////////////////////////////////////////////////////////
void * ARMGenericAssembler::WriteToEscapeByException( const void * ptrEpilogue )
{
	ARMRegister	regTemp1 =
		(ARMRegister) AllocateDataRegister( regClassARM ) ;
	ARMRegister	regTemp2 = (ARMRegister) (regTemp1 + 1) ;
	ESLAssert( sizeof(uint32_t) == sizeof(void*) ) ;
	WriteARMMoveRegImm32
		( regTemp1, (uint32_t) ((ulong_ptr_t) &maskGlobalInterrupt) ) ;
	WriteARMLoadMemOffsetImm12
			( regTemp2, ARM_r10, offsetof(Context,m_maskException) ) ;
	WriteARMLoadMemOffsetImm12( regTemp1, regTemp1, 0 ) ;
	WriteARMOrRegRegRegShift
			( regTemp1, regTemp1, regTemp2, 0, cond_AL, true ) ;
	//
	FreeDataRegister( regClassARM, regTemp1 ) ;
	//
	return	WriteARMJumpImm32( ptrEpilogue, cond_NE ) ;
}

// ジャンプコード完成（２パス用）
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::CommitJumpTarget
		( void * ptrJumpFrom, const void * ptrFixedTarget )
{
	*((uint32_t*)ptrJumpFrom) = (uint32_t) ((ulong_ptr_t) ptrFixedTarget) ;
}

// データ移動命令
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::write_move_reg_reg( int regDst, int regSrc, bool fPair )
{
	if ( regDst == regSrc )
	{
		return ;
	}
	if ( m_vfpNEON & fPair )
	{
		int	qreg = WriteRealizeDataRegister( regDst, regClassNEON, false ) ;
		RealizeFreeNEONRegister( qreg, regSrc, true, false ) ;
		SetDataRegisterModified( regClassNEON, qreg ) ;
		UnlockDataRegister( regClassNEON, qreg ) ;
	}
	else if ( m_vfpVersion >= 2 )
	{
		DataRegisterClass
				classLoaded = m_lruDataReg.dprSakura[regDst].regClass ;
		int		regPhy = m_lruDataReg.dprSakura[regDst].regPhy ;
		if ( classLoaded == regClassNEON )
		{
			if ( fPair )
			{
				RealizeFreeNEONRegister( regPhy, regSrc, true, false ) ;
			}
			else
			{
				RealizeFreeVFPRegister
					( (regPhy << 1) + (regDst & 0x01), regSrc, true, false ) ;
			}
		}
		else
		{
			int	vreg0 = WriteRealizeDataRegister( regDst, regClassVFP, false ) ;
			RealizeFreeVFPRegister( vreg0, regSrc, true, false ) ;
			SetDataRegisterModified( regClassVFP, vreg0 ) ;
			UnlockDataRegister( regClassVFP, vreg0 ) ;
			//
			if ( fPair )
			{
				int	vreg1 = WriteRealizeDataRegister( regDst + 1, regClassVFP, false ) ;
				RealizeFreeVFPRegister( vreg1, regSrc + 1, true, false ) ;
				SetDataRegisterModified( regClassVFP, vreg1 ) ;
				UnlockDataRegister( regClassVFP, vreg1 ) ;
			}
		}
	}
	else
	{
		int	regARM = WriteRealizeDataRegister( regDst, regClassARM, false ) ;
		RealizeFreeARMRegister( regARM, regSrc, true, false ) ;
		SetDataRegisterModified( regClassARM, regARM ) ;
		UnlockDataRegister( regClassARM, regARM ) ;
		//
		if ( fPair )
		{
			regARM = WriteRealizeDataRegister( regDst + 1, regClassARM, false ) ;
			RealizeFreeARMRegister( regARM, regSrc + 1, true, false ) ;
			SetDataRegisterModified( regClassARM, regARM ) ;
			UnlockDataRegister( regClassARM, regARM ) ;
		}
	}
}

void ARMGenericAssembler::write_maskmove_reg_reg_reg( int regDst, int regSrc, int regSrc2, bool fPair )
{
	if ( m_vfpNEON & fPair )
	{
		int	qregDst = WriteRealizeDataRegister( regDst, regClassNEON, true ) ;
		int	qregSrc = WriteRealizeDataRegister( regSrc, regClassNEON, true ) ;
		int	qregMask = WriteRealizeDataRegister( regDst, regClassNEON, true ) ;
		//
		WriteSIMDOpRegRegReg
			( armOpVBIT, thumbOpVBIT,
				(qregSrc << 1), (qregSrc << 1), (qregMask << 1), true ) ;
		//
		SetDataRegisterModified( regClassNEON, qregDst ) ;
		UnlockDataRegister( regClassNEON, qregMask ) ;
		UnlockDataRegister( regClassNEON, qregSrc ) ;
		UnlockDataRegister( regClassNEON, qregDst ) ;
	}
	else if ( m_vfpNEON )
	{
		ESLAssert( !fPair ) ;
		int	vregDst = WriteRealizeDataRegister( regDst, regClassVFP, true ) ;
		int	vregSrc = WriteRealizeDataRegister( regSrc, regClassVFP, true ) ;
		int	vregMask = WriteRealizeDataRegister( regSrc2, regClassVFP, true ) ;
		//
		WriteSIMDOpRegRegReg
			( armOpVBIT, thumbOpVBIT,
				vregDst, vregSrc, vregMask, false ) ;
		//
		SetDataRegisterModified( regClassVFP, vregDst ) ;
		UnlockDataRegister( regClassVFP, vregMask ) ;
		UnlockDataRegister( regClassVFP, vregSrc ) ;
		UnlockDataRegister( regClassVFP, vregDst ) ;
	}
	else
	{
		ARMRegister	armDst0 =
			(ARMRegister) WriteRealizeDataRegister( regDst, regClassARM ) ;
		ARMRegister	armSrc0 =
			(ARMRegister) AllocateDataRegister( regClassARM ) ;
		ARMRegister	armMask0 =
			(ARMRegister) AllocateDataRegister( regClassARM ) ;
		RealizeFreeARMRegister( armSrc0, regSrc, true, false ) ;
		RealizeFreeARMRegister( armMask0, regSrc2, true, false ) ;
		//
		ARMRegister	armDst1 = (ARMRegister) (armDst0 + 1) ;
		ARMRegister	armSrc1 = (ARMRegister) (armSrc0 + 1) ;
		ARMRegister	armMask1 = (ARMRegister) (armMask0 + 1) ;
		//
		WriteARMAndRegRegRegShift( armSrc0, armSrc0, armMask0 ) ;
		WriteARMAndRegRegRegShift( armSrc1, armSrc1, armMask1 ) ;
		//
		WriteARMNotRegRegShift( armMask0, armMask0 ) ;
		WriteARMNotRegRegShift( armMask1, armMask1 ) ;
		//
		WriteARMAndRegRegRegShift( armDst0, armDst0, armMask0 ) ;
		WriteARMAndRegRegRegShift( armDst1, armDst1, armMask1 ) ;
		//
		WriteARMOrRegRegRegShift( armDst0, armDst0, armSrc0 ) ;
		WriteARMOrRegRegRegShift( armDst1, armDst1, armSrc1 ) ;
		//
		FreeDataRegister( regClassARM, armMask0 ) ;
		FreeDataRegister( regClassARM, armSrc0 ) ;
		UnlockDataRegister( regClassARM, armDst0 ) ;
		//
		if ( fPair )
		{
			write_maskmove_reg_reg_reg( regDst+1, regSrc+1, regSrc2+1, false ) ;
		}
	}
}

// 整数・実数変換命令
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::write_cvt_float2int( int regDst, int regSrc )
{
	if ( m_vfpVersion >= 2 )
	{
		ARMRegister	armDst0 =
			(ARMRegister) WriteRealizeDataRegister
					( regDst, regClassARM, (regDst == regSrc) ) ;
		if ( regDst != regSrc )
		{
			RealizeFreeARMRegister( armDst0, regSrc, true, false ) ;
		}
		int	vregTemp1 = AllocateDataRegister( regClassVFP ) ;
		ARMRegister	armDst1 = (ARMRegister) (armDst0 + 1) ;
		//
		WriteMoveARMtoVFP64( vregTemp1, armDst0, armDst1 ) ;
		WriteARMMoveRegImm( armDst0, 1023+30 ) ;
		PreserveContinuousCodes( 0x10 ) ;
		WriteARMMoveRegImm( ARM_r6, 0x07FF ) ;
		WriteARMAndRegRegRegShift( armDst1, ARM_r6, armDst1, -(52-32) ) ;
		WriteARMCmpRegRegShift( armDst1, armDst0 ) ;
		WriteARMJumpImm( m_bufSub->GetNext(), cond_HI ) ;
		//
		// 絶対値が 2^30 以下の場合
		WriteCvtVFPtoInt32( (vregTemp1 << 1), vregTemp1, true, false, false ) ;
		WriteMoveVFPtoARM32( armDst0, (vregTemp1 << 1) ) ;
		WriteARMShiftARightImm( armDst1, armDst0, 31 ) ;
		void *	pNextAddr = GetNextAddress() ;
		//
		// 絶対値が 2^30 より大きい場合
		m_buf = m_bufSub ;
		//
		WriteMoveVFPtoARM64( armDst0, armDst1, vregTemp1 ) ;
		WriteBackAllRegisters() ;
		//
		WriteARMLeaSakura2Register( ARM_r0, regDst ) ;
		WriteARMLeaSakura2Register( ARM_r1, regSrc ) ;
		WriteARMCallImm( (const void*) m_gi->cvt_float2int ) ;
		//
		ReloadRegisters() ;
		WriteARMJumpImm( pNextAddr, cond_AL ) ;
		//
		m_buf = m_bufMain ;
		//
		SetDataRegisterModified( regClassARM, armDst0 ) ;
		UnlockDataRegister( regClassARM, armDst0 ) ;
		FreeDataRegister( regClassVFP, vregTemp1 ) ;
	}
	else
	{
		Sakura2Assembler::write_cvt_float2int( regDst, regSrc ) ;
	}
}

void ARMGenericAssembler::write_cvt_int2float( int regDst, int regSrc )
{
	if ( m_vfpVersion >= 2 )
	{
		int	vregDst =
			WriteRealizeDataRegister
					( regDst, regClassVFP, (regDst == regSrc) ) ;
		if ( regDst != regSrc )
		{
			RealizeFreeVFPRegister( vregDst, regSrc, true, false ) ;
		}
		ARMRegister	armTemp0 = (ARMRegister) AllocateDataRegister( regClassARM ) ;
		ARMRegister	armTemp1 = (ARMRegister) (armTemp0 + 1) ;
		//
		WriteMoveVFPtoARM64( armTemp0, armTemp1, vregDst ) ;
		WriteARMCmpRegRegShift( armTemp1, armTemp0, -31-32 ) ;
		WriteARMJumpImm( m_bufSub->GetNext(), cond_NE ) ;
		//
		// 符号有り32ビット範囲の場合
		WriteCvtVFPInt32toFloat( vregDst, (vregDst << 1), true, false ) ;
		void *	pNextAddr = GetNextAddress() ;
		//
		// 符号有り32ビット範囲外の場合
		m_buf = m_bufSub ;
		//
		WriteBackAllRegisters() ;
		//
		WriteARMLeaSakura2Register( ARM_r0, regDst ) ;
		WriteARMLeaSakura2Register( ARM_r1, regSrc ) ;
		PreserveContinuousCodes( 0x10 ) ;
		WriteARMCallImm( (const void*) m_gi->cvt_int2float ) ;
		//
		ReloadRegisters() ;
		WriteARMJumpImm( pNextAddr, cond_AL ) ;
		//
		m_buf = m_bufMain ;
		//
		SetDataRegisterModified( regClassVFP, vregDst ) ;
		UnlockDataRegister( regClassVFP, vregDst ) ;
		FreeDataRegister( regClassARM, armTemp0 ) ;
	}
	else
	{
		Sakura2Assembler::write_cvt_int2float( regDst, regSrc ) ;
	}
}

// シフト命令
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::write_srl_reg_reg_imm8( int regDst, int regSrc, int imm8, bool fPair )
{
	imm8 &= 0x3F ;
	if ( imm8 == 0 )
	{
		return ;
	}
	if ( m_vfpNEON & fPair )
	{
		int	qregDst = WriteRealizeDataRegister
						( regDst, regClassNEON, (regDst == regSrc) ) ;
		if ( regDst != regSrc )
		{
			RealizeFreeNEONRegister( qregDst, regSrc, true, false ) ;
		}
		//
		WriteSIMDShiftRegRegImm
			( armOpVSHRImm, thumbOpVSHRImm,
				(qregDst << 1), (qregDst << 1),
				(imm8 & 0x3F), typeNEONInt64, false, true, true ) ;
		//
		SetDataRegisterModified( regClassNEON, qregDst ) ;
		UnlockDataRegister( regClassNEON, qregDst ) ;
	}
	else if ( m_vfpNEON )
	{
		ESLAssert( !fPair ) ;
		int	vregDst = WriteRealizeDataRegister
						( regDst, regClassVFP, (regDst == regSrc) ) ;
		if ( regDst != regSrc )
		{
			RealizeFreeVFPRegister( vregDst, regSrc, true, false ) ;
		}
		//
		WriteSIMDShiftRegRegImm
			( armOpVSHRImm, thumbOpVSHRImm,
				vregDst, vregDst,
				(imm8 & 0x3F), typeNEONInt64, false, true, false ) ;
		//
		SetDataRegisterModified( regClassVFP, vregDst ) ;
		UnlockDataRegister( regClassVFP, vregDst ) ;
	}
	else
	{
		int	armDst = WriteRealizeDataRegister
						( regDst, regClassARM, (regDst == regSrc) ) ;
		if ( regDst != regSrc )
		{
			RealizeFreeARMRegister( armDst, regSrc, true, false ) ;
		}
		ARMRegister	regDst0 = (ARMRegister) armDst ;
		ARMRegister	regDst1 = (ARMRegister) (armDst + 1) ;
		//
		if ( imm8 < 32 )
		{
			WriteARMShiftRightImm( regDst0, regDst0, imm8 ) ;
			WriteARMOrRegRegRegShift( regDst0, regDst0, regDst1, (32 - imm8) ) ;
			WriteARMShiftRightImm( regDst1, regDst1, imm8 ) ;
		}
		else if ( imm8 > 32 )
		{
			WriteARMShiftLeftImm( regDst0, regDst1, imm8 - 32 ) ;
			WriteARMXorRegRegRegShift( regDst1, regDst1, regDst1 ) ;
		}
		else
		{
			ESLAssert( imm8 == 32 ) ;
			WriteARMMoveRegReg( regDst0, regDst1 ) ;
			WriteARMXorRegRegRegShift( regDst1, regDst1, regDst1 ) ;
		}
		//
		SetDataRegisterModified( regClassARM, armDst ) ;
		UnlockDataRegister( regClassARM, armDst ) ;
		//
		if ( fPair )
		{
			write_srl_reg_reg_imm8( regDst + 1, regSrc + 1, imm8, false ) ;
		}
	}
}

void ARMGenericAssembler::write_sra_reg_reg_imm8( int regDst, int regSrc, int imm8, bool fPair )
{
	imm8 &= 0x3F ;
	if ( imm8 == 0 )
	{
		return ;
	}
	if ( m_vfpNEON & fPair )
	{
		int	qregDst = WriteRealizeDataRegister
						( regDst, regClassNEON, (regDst == regSrc) ) ;
		if ( regDst != regSrc )
		{
			RealizeFreeNEONRegister( qregDst, regSrc, true, false ) ;
		}
		//
		WriteSIMDShiftRegRegImm
			( armOpVSHRImm, thumbOpVSHRImm,
				(qregDst << 1), (qregDst << 1),
				(imm8 & 0x3F), typeNEONInt64, true, true, true ) ;
		//
		SetDataRegisterModified( regClassNEON, qregDst ) ;
		UnlockDataRegister( regClassNEON, qregDst ) ;
	}
	else if ( m_vfpNEON )
	{
		ESLAssert( !fPair ) ;
		int	vregDst = WriteRealizeDataRegister
						( regDst, regClassVFP, (regDst == regSrc) ) ;
		if ( regDst != regSrc )
		{
			RealizeFreeVFPRegister( vregDst, regSrc, true, false ) ;
		}
		//
		WriteSIMDShiftRegRegImm
			( armOpVSHRImm, thumbOpVSHRImm,
				vregDst, vregDst,
				(imm8 & 0x3F), typeNEONInt64, true, true, false ) ;
		//
		SetDataRegisterModified( regClassVFP, vregDst ) ;
		UnlockDataRegister( regClassVFP, vregDst ) ;
	}
	else
	{
		int	armDst = WriteRealizeDataRegister
						( regDst, regClassARM, (regDst == regSrc) ) ;
		if ( regDst != regSrc )
		{
			RealizeFreeARMRegister( armDst, regSrc, true, false ) ;
		}
		ARMRegister	regDst0 = (ARMRegister) armDst ;
		ARMRegister	regDst1 = (ARMRegister) (armDst + 1) ;
		//
		if ( imm8 < 32 )
		{
			WriteARMShiftRightImm( regDst0, regDst0, imm8 ) ;
			WriteARMOrRegRegRegShift( regDst0, regDst0, regDst1, (32 - imm8) ) ;
			WriteARMShiftARightImm( regDst1, regDst1, imm8 ) ;
		}
		else if ( imm8 > 32 )
		{
			WriteARMShiftARightImm( regDst0, regDst1, imm8 - 32 ) ;
			WriteARMShiftARightImm( regDst1, regDst1, 31 ) ;
		}
		else
		{
			ESLAssert( imm8 == 32 ) ;
			WriteARMMoveRegReg( regDst1, regDst0 ) ;
			WriteARMShiftARightImm( regDst1, regDst1, 31 ) ;
		}
		//
		SetDataRegisterModified( regClassARM, armDst ) ;
		UnlockDataRegister( regClassARM, armDst ) ;
		//
		if ( fPair )
		{
			write_sra_reg_reg_imm8( regDst + 1, regSrc + 1, imm8, false ) ;
		}
	}
}

void ARMGenericAssembler::write_sll_reg_reg_imm8( int regDst, int regSrc, int imm8, bool fPair )
{
	imm8 &= 0x3F ;
	if ( imm8 == 0 )
	{
		return ;
	}
	if ( m_vfpNEON & fPair )
	{
		int	qregDst = WriteRealizeDataRegister
						( regDst, regClassNEON, (regDst == regSrc) ) ;
		if ( regDst != regSrc )
		{
			RealizeFreeNEONRegister( qregDst, regSrc, true, false ) ;
		}
		//
		WriteSIMDShiftRegRegImm
			( armOpVSHLImm, thumbOpVSHLImm,
				(qregDst << 1), (qregDst << 1),
				(imm8 & 0x3F), typeNEONInt64, true, false, true ) ;
		//
		SetDataRegisterModified( regClassNEON, qregDst ) ;
		UnlockDataRegister( regClassNEON, qregDst ) ;
	}
	else if ( m_vfpNEON )
	{
		ESLAssert( !fPair ) ;
		int	vregDst = WriteRealizeDataRegister
						( regDst, regClassVFP, (regDst == regSrc) ) ;
		if ( regDst != regSrc )
		{
			RealizeFreeVFPRegister( vregDst, regSrc, true, false ) ;
		}
		//
		WriteSIMDShiftRegRegImm
			( armOpVSHLImm, thumbOpVSHLImm,
				vregDst, vregDst,
				(imm8 & 0x3F), typeNEONInt64, true, false, false ) ;
		//
		SetDataRegisterModified( regClassVFP, vregDst ) ;
		UnlockDataRegister( regClassVFP, vregDst ) ;
	}
	else
	{
		int	armDst = WriteRealizeDataRegister
						( regDst, regClassARM, (regDst == regSrc) ) ;
		if ( regDst != regSrc )
		{
			RealizeFreeARMRegister( armDst, regSrc, true, false ) ;
		}
		ARMRegister	regDst0 = (ARMRegister) armDst ;
		ARMRegister	regDst1 = (ARMRegister) (armDst + 1) ;
		//
		if ( imm8 < 32 )
		{
			WriteARMShiftLeftImm( regDst1, regDst1, imm8 ) ;
			WriteARMOrRegRegRegShift( regDst1, regDst1, regDst0, -(32 - imm8) ) ;
			WriteARMShiftLeftImm( regDst0, regDst0, imm8 ) ;
		}
		else if ( imm8 > 32 )
		{
			WriteARMShiftLeftImm( regDst1, regDst0, imm8 - 32 ) ;
			WriteARMXorRegRegRegShift( regDst0, regDst0, regDst0 ) ;
		}
		else
		{
			ESLAssert( imm8 == 32 ) ;
			WriteARMMoveRegReg( regDst1, regDst0 ) ;
			WriteARMXorRegRegRegShift( regDst0, regDst0, regDst0 ) ;
		}
		//
		SetDataRegisterModified( regClassARM, armDst ) ;
		UnlockDataRegister( regClassARM, armDst ) ;
		//
		if ( fPair )
		{
			write_sll_reg_reg_imm8( regDst + 1, regSrc + 1, imm8, false ) ;
		}
	}
}

// 32ビット即値命令
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::write_add_reg_reg_imm32( int regDst, int regSrc, int imm32, bool fPair )
{
	if ( imm32 == 0 )
	{
		write_move_reg_reg( regDst, regSrc, fPair ) ;
		return ;
	}
	if ( m_vfpNEON )
	{
		int	vregDst = WriteRealizeDataRegister
						( regDst, regClassVFP, (regDst == regSrc) ) ;
		int	vregTemp = AllocateDataRegister( regClassVFP ) ;
		if ( regDst != regSrc )
		{
			RealizeFreeVFPRegister( vregDst, regSrc, true, false ) ;
		}
		//
		WriteVFPLoadImm64( vregTemp, imm32 ) ;
		WriteSIMDIntOpRegRegReg
			( armOpVIADD, thumbOpVIADD,
				vregDst, vregDst, vregTemp, typeNEONInt64, false ) ;
		//
		SetDataRegisterModified( regClassVFP, vregDst ) ;
		UnlockDataRegister( regClassVFP, vregDst ) ;
		FreeDataRegister( regClassVFP, vregTemp ) ;
	}
	else
	{
		const int	armDst = WriteRealizeDataRegister
						( regDst, regClassARM, (regDst == regSrc) ) ;
		if ( regDst != regSrc )
		{
			RealizeFreeARMRegister( armDst, regSrc, true, false ) ;
		}
		ARMRegister	armDst0 = (ARMRegister) armDst ;
		ARMRegister	armDst1 = (ARMRegister) (armDst + 1) ;
		//
		if ( imm32 > 0 )
		{
			if ( imm32 < 0x100 )
			{
				WriteARMAddRegRegImm8
					( armDst0, armDst0, imm32, cond_AL, true ) ;
			}
			else
			{
				ARMRegister	armTemp =
					(ARMRegister) AllocateDataRegister( regClassARM ) ;
				WriteARMMoveRegImm( armTemp, imm32 ) ;
				WriteARMAddRegRegRegShift
					( armDst0, armDst0, armTemp, 0, cond_AL, true ) ;
				FreeDataRegister( regClassARM, armTemp ) ;
			}
			WriteARMAdcRegRegImm8( armDst1, armDst1, 0 ) ;
		}
		else
		{
			if ( imm32 > -0x100 )
			{
				WriteARMSubRegRegImm8
					( armDst0, armDst0, -imm32, cond_AL, true ) ;
			}
			else
			{
				ARMRegister	armTemp =
					(ARMRegister) AllocateDataRegister( regClassARM ) ;
				WriteARMMoveRegImm( armTemp, -imm32 ) ;
				WriteARMSubRegRegRegShift
					( armDst0, armDst0, armTemp, 0, cond_AL, true ) ;
				FreeDataRegister( regClassARM, armTemp ) ;
			}
			WriteARMSbcRegRegImm8( armDst1, armDst1, 0 ) ;
		}
		//
		SetDataRegisterModified( regClassARM, armDst ) ;
		UnlockDataRegister( regClassARM, armDst ) ;
	}
	if ( fPair )
	{
		write_add_reg_reg_imm32( regDst + 1, regSrc + 1, imm32, false ) ;
	}
}

void ARMGenericAssembler::write_mul_reg_reg_imm32( int regDst, int regSrc, int imm32, bool fPair )
{
	if ( imm32 == 0 )
	{
		if ( m_vfpNEON )
		{
			int	vregDst = WriteRealizeDataRegister( regDst, regClassVFP, false ) ;
			WriteSIMDOpRegRegReg
				( armOpVEOR, thumbOpVEOR, vregDst, vregDst, vregDst, false ) ;
			SetDataRegisterModified( regClassVFP, vregDst ) ;
			UnlockDataRegister( regClassVFP, vregDst ) ;
		}
		else
		{
			ARMRegister	armDst =
				(ARMRegister) WriteRealizeDataRegister
								( regDst, regClassARM, false ) ;
			ARMRegister	armDst1 = (ARMRegister) (armDst + 1) ;
			WriteARMXorRegRegRegShift
				( armDst, armDst, armDst, false ) ;
			WriteARMXorRegRegRegShift
				( armDst1, armDst1, armDst1, false ) ;
			SetDataRegisterModified( regClassARM, armDst ) ;
			UnlockDataRegister( regClassARM, armDst ) ;
		}
	}
	else if ( imm32 == 1 )
	{
		if ( m_vfpNEON )
		{
			int	vregDst =
				WriteRealizeDataRegister
					( regDst, regClassVFP, (regDst == regSrc) ) ;
			if ( regDst != regSrc )
			{
				RealizeFreeVFPRegister( vregDst, regSrc, true, false ) ;
			}
			SetDataRegisterModified( regClassVFP, vregDst ) ;
			UnlockDataRegister( regClassVFP, vregDst ) ;
		}
		else
		{
			ARMRegister	armDst =
				(ARMRegister) WriteRealizeDataRegister
							( regDst, regClassARM, (regDst == regSrc) ) ;
			if ( regDst != regSrc )
			{
				RealizeFreeARMRegister( armDst, regSrc, true, false ) ;
			}
			SetDataRegisterModified( regClassARM, armDst ) ;
			UnlockDataRegister( regClassARM, armDst ) ;
		}
	}
	else if ( imm32 == -1 )
	{
		if ( m_vfpNEON )
		{
			int	vregTemp = AllocateDataRegister( regClassVFP ) ;
			RealizeFreeVFPRegister( vregTemp, regSrc, true, false ) ;
			int	vregDst =
				WriteRealizeDataRegister( regDst, regClassVFP, false ) ;
			//
			WriteSIMDOpRegRegReg
				( armOpVEOR, thumbOpVEOR, vregDst, vregDst, vregDst, false ) ;
			WriteSIMDIntOpRegRegReg
				( armOpVISUB, thumbOpVISUB,
					vregDst, vregDst, vregTemp, typeNEONInt64, false ) ;
			//
			SetDataRegisterModified( regClassVFP, vregDst ) ;
			UnlockDataRegister( regClassVFP, vregDst ) ;
			FreeDataRegister( regClassVFP, vregTemp ) ;
		}
		else
		{
			ARMRegister	armTemp0 =
				(ARMRegister) AllocateDataRegister( regClassARM ) ;
			ARMRegister	armTemp1 = (ARMRegister) (armTemp0 + 1) ;
			RealizeFreeARMRegister( armTemp0, regSrc, true, false ) ;
			//
			ARMRegister	armDst0 =
				(ARMRegister) WriteRealizeDataRegister
								( regDst, regClassARM, false ) ;
			ARMRegister	armDst1 = (ARMRegister) (armDst0 + 1) ;
			//
			WriteARMXorRegRegRegShift( armDst0, armDst0, armDst0 ) ;
			WriteARMXorRegRegRegShift( armDst1, armDst1, armDst1 ) ;
			WriteARMSubRegRegRegShift
				( armDst0, armDst0, armTemp0, 0, cond_AL, true ) ;
			WriteARMOpRegRegRegShift
				( armOpSBC, thumbOpSBC,
					armDst1, armDst1, armTemp1 ) ;
			//
			SetDataRegisterModified( regClassARM, armDst0 ) ;
			UnlockDataRegister( regClassARM, armDst0 ) ;
			FreeDataRegister( regClassARM, armTemp0 ) ;
		}
	}
	else
	{
		ARMRegister	armDst =
			(ARMRegister) WriteRealizeDataRegister
						( regDst, regClassARM, (regDst == regSrc) ) ;
		if ( regDst != regSrc )
		{
			RealizeFreeARMRegister( armDst, regSrc, true, false ) ;
		}
		ARMRegister	armSrc0 =
			(ARMRegister) AllocateDataRegister( regClassARM ) ;
		ARMRegister	armTemp0 =
			(ARMRegister) AllocateDataRegister( regClassARM ) ;
		ARMRegister	armDst1 = (ARMRegister) (armDst + 1) ;
		ARMRegister	armSrc1 = (ARMRegister) (armSrc0 + 1) ;
		ARMRegister	armTemp1 = (ARMRegister) (armTemp0 + 1) ;
		//
		WriteARMMoveRegImm( armSrc0, imm32 ) ;
		WriteARMMoveRegImm( armSrc1, (imm32 >> 31) ) ;
		//
		write_mul_int64xint64
			( armDst, armDst1, armSrc0, armSrc1, armTemp0, armTemp1 ) ;
		//
		SetDataRegisterModified( regClassARM, armDst ) ;
		UnlockDataRegister( regClassARM, armDst ) ;
		FreeDataRegister( regClassARM, armSrc0 ) ;
		FreeDataRegister( regClassARM, armTemp0 ) ;
	}
	if ( fPair )
	{
		write_mul_reg_reg_imm32( regDst + 1, regSrc + 1, imm32, false ) ;
	}
}

// スタックレジスタ加算命令
//////////////////////////////////////////////////////////////////////////////
void * ARMGenericAssembler::write_add_sp_imm32( int ip, int imm32 )
{
	FlushAllRegisters() ;
	//
	write_add_reg_reg_imm32( regSP, regSP, imm32, false ) ;
	//
	if ( imm32 < 0 )
	{
		int		regPhy ;
		void *	ptrEsc = WriteToStackException( regPhy, 0, NULL ) ;
		FreeDataRegister( regClassARM, regPhy ) ;
		return	ptrEsc ;
	}
	else
	{
		return	NULL ;
	}
}

// 64ビット即値命令
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::write_move_reg_imm64( int regDst, INT64 imm64 )
{
	if ( m_vfpVersion >= 2 )
	{
		int	vregPhy =
			WriteRealizeDataRegister( regDst, regClassVFP, false ) ;
		//
		WriteVFPLoadImm64( vregPhy, imm64 ) ;
		//
		SetDataRegisterModified( regClassVFP, vregPhy ) ;
		UnlockDataRegister( regClassVFP, vregPhy ) ;
	}
	else
	{
		int	regPhy =
			WriteRealizeDataRegister( regDst, regClassARM, false ) ;
		//
		WriteARMMoveRegImm32
			( (ARMRegister) regPhy, (uint32_t) imm64 ) ;
		WriteARMMoveRegImm32
			( (ARMRegister) (regPhy + 1), (uint32_t) (imm64 >> 32) ) ;
		//
		SetDataRegisterModified( regClassARM, regPhy ) ;
		UnlockDataRegister( regClassARM, regPhy ) ;
	}
}

// 1 OP 演算命令
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::write_neg_int( int regDst )
{
	if ( m_vfpNEON )
	{
		int	vregDst = WriteRealizeDataRegister( regDst, regClassVFP ) ;
		int	vregTemp = AllocateDataRegister( regClassVFP ) ;
		//
		WriteSIMDOpRegRegReg
			( armOpVEOR, thumbOpVEOR,
				vregTemp, vregTemp, vregTemp, false ) ;
		WriteSIMDIntOpRegRegReg
			( armOpVISUB, thumbOpVISUB,
				vregDst, vregTemp, vregDst, typeNEONInt64, false ) ;
		//
		SetDataRegisterModified( regClassVFP, vregDst ) ;
		UnlockDataRegister( regClassVFP, vregDst ) ;
		FreeDataRegister( regClassVFP, vregTemp ) ;
	}
	else
	{
		ARMRegister	armTemp0 =
			(ARMRegister) AllocateDataRegister( regClassARM ) ;
		ARMRegister	armTemp1 = (ARMRegister) (armTemp0 + 1) ;
		//
		WriteARMXorRegRegRegShift( armTemp0, armTemp0, armTemp0 ) ;
		//
		ARMRegister	armDst0 =
			(ARMRegister) WriteRealizeDataRegister( regDst, regClassARM ) ;
		ARMRegister	armDst1 = (ARMRegister) (armDst0 + 1) ;
		//
		WriteARMSubRegRegRegShift
			( armDst0, armTemp0, armDst0, 0, cond_AL, true ) ;
		WriteARMOpRegRegRegShift
			( armOpSBC, thumbOpSBC,
				armDst1, armTemp0, armDst1 ) ;
		//
		SetDataRegisterModified( regClassARM, armDst0 ) ;
		UnlockDataRegister( regClassARM, armDst0 ) ;
		FreeDataRegister( regClassARM, armTemp0 ) ;
	}
}

void ARMGenericAssembler::write_not_int( int regDst )
{
	if ( m_vfpNEON )
	{
		int	vregDst = WriteRealizeDataRegister( regDst, regClassVFP ) ;
		//
		WriteSIMDIntNotRegReg
			( vregDst, vregDst, typeNEONInt64, false ) ;
		//
		SetDataRegisterModified( regClassVFP, vregDst ) ;
		UnlockDataRegister( regClassVFP, vregDst ) ;
	}
	else
	{
		ARMRegister	armDst0 =
			(ARMRegister) WriteRealizeDataRegister( regDst, regClassARM ) ;
		ARMRegister	armDst1 = (ARMRegister) (armDst0 + 1) ;
		//
		WriteARMNotRegRegShift( armDst0, armDst0 ) ;
		WriteARMNotRegRegShift( armDst1, armDst1 ) ;
		//
		SetDataRegisterModified( regClassARM, armDst0 ) ;
		UnlockDataRegister( regClassARM, armDst0 ) ;
	}
}

void ARMGenericAssembler::write_neg_float( int regDst )
{
	if ( m_vfpVersion >= 2 )
	{
		int	vregDst = WriteRealizeDataRegister( regDst, regClassVFP ) ;
		//
		WriteNegVFPRegReg( vregDst, vregDst, true ) ;
		//
		SetDataRegisterModified( regClassVFP, vregDst ) ;
		UnlockDataRegister( regClassVFP, vregDst ) ;
	}
	else
	{
		ARMRegister	armDst0 =
			(ARMRegister) WriteRealizeDataRegister( regDst, regClassARM ) ;
		ARMRegister	armDst1 = (ARMRegister) (armDst0 + 1) ;
		//
		PreserveContinuousCodes( 0x10 ) ;
		WriteARMMoveRegImm( ARM_r6, 0x80000000 ) ;
		WriteARMXorRegRegRegShift( armDst1, armDst1, ARM_r6 ) ;
		//
		SetDataRegisterModified( regClassARM, armDst0 ) ;
		UnlockDataRegister( regClassARM, armDst0 ) ;
	}
}

// 2 OP 整数演算命令
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::write_add_reg_reg( int regDst, int regSrc, bool fPair )
{
	if ( m_vfpNEON )
	{
		DataRegisterClass	regClass = fPair ? regClassNEON : regClassVFP ;
		int	vregDst = WriteRealizeDataRegister( regDst, regClass ) ;
		int	vregSrc = WriteRealizeDataRegister( regSrc, regClass ) ;
		if ( regClass == regClassNEON )
		{
			WriteSIMDIntOpRegRegReg
				( armOpVIADD, thumbOpVIADD,
					(vregDst << 1), (vregDst << 1),
					(vregSrc << 1), typeNEONInt64, fPair ) ;
		}
		else
		{
			WriteSIMDIntOpRegRegReg
				( armOpVIADD, thumbOpVIADD,
					vregDst, vregDst, vregSrc, typeNEONInt64, fPair ) ;
		}
		SetDataRegisterModified( regClass, vregDst ) ;
		UnlockDataRegister( regClass, vregDst ) ;
		UnlockDataRegister( regClass, vregSrc ) ;
	}
	else
	{
		ARMRegister	armDst0 =
			(ARMRegister) WriteRealizeDataRegister( regDst, regClassARM ) ;
		ARMRegister	armSrc0 =
			(ARMRegister) WriteRealizeDataRegister( regSrc, regClassARM ) ;
		ARMRegister	armDst1 = (ARMRegister) (armDst0 + 1) ;
		ARMRegister	armSrc1 = (ARMRegister) (armSrc0 + 1) ;
		//
		WriteARMAddRegRegRegShift
			( armDst0, armDst0, armSrc0, 0, cond_AL, true ) ;
		WriteARMOpRegRegRegShift
			( armOpADC, thumbOpADC, armDst1, armDst1, armSrc1 ) ;
		//
		SetDataRegisterModified( regClassARM, armDst0 ) ;
		UnlockDataRegister( regClassARM, armDst0 ) ;
		UnlockDataRegister( regClassARM, armSrc0 ) ;
	}
}

void ARMGenericAssembler::write_sub_reg_reg( int regDst, int regSrc, bool fPair )
{
	if ( m_vfpNEON )
	{
		DataRegisterClass	regClass = fPair ? regClassNEON : regClassVFP ;
		int	vregDst = WriteRealizeDataRegister( regDst, regClass ) ;
		int	vregSrc = WriteRealizeDataRegister( regSrc, regClass ) ;
		if ( regClass == regClassNEON )
		{
			WriteSIMDIntOpRegRegReg
				( armOpVISUB, thumbOpVISUB,
					(vregDst << 1), (vregDst << 1),
					(vregSrc << 1), typeNEONInt64, fPair ) ;
		}
		else
		{
			WriteSIMDIntOpRegRegReg
				( armOpVISUB, thumbOpVISUB,
					vregDst, vregDst, vregSrc, typeNEONInt64, fPair ) ;
		}
		SetDataRegisterModified( regClass, vregDst ) ;
		UnlockDataRegister( regClass, vregDst ) ;
		UnlockDataRegister( regClass, vregSrc ) ;
	}
	else
	{
		ARMRegister	armDst0 =
			(ARMRegister) WriteRealizeDataRegister( regDst, regClassARM ) ;
		ARMRegister	armSrc0 =
			(ARMRegister) WriteRealizeDataRegister( regSrc, regClassARM ) ;
		ARMRegister	armDst1 = (ARMRegister) (armDst0 + 1) ;
		ARMRegister	armSrc1 = (ARMRegister) (armSrc0 + 1) ;
		//
		WriteARMSubRegRegRegShift
			( armDst0, armDst0, armSrc0, 0, cond_AL, true ) ;
		WriteARMOpRegRegRegShift
			( armOpSBC, thumbOpSBC, armDst1, armDst1, armSrc1 ) ;
		//
		SetDataRegisterModified( regClassARM, armDst0 ) ;
		UnlockDataRegister( regClassARM, armDst0 ) ;
		UnlockDataRegister( regClassARM, armSrc0 ) ;
	}
}

void ARMGenericAssembler::write_mul_reg_reg( int regDst, int regSrc, bool fPair )
{
	ARMRegister	armDst0 =
		(ARMRegister) WriteRealizeDataRegister( regDst, regClassARM ) ;
	ARMRegister	armSrc0 =
		(ARMRegister) WriteRealizeDataRegister( regSrc, regClassARM ) ;
	ARMRegister	armTemp0 = (ARMRegister) AllocateDataRegister( regClassARM ) ;
	ARMRegister	armDst1 = (ARMRegister) (armDst0 + 1) ;
	ARMRegister	armSrc1 = (ARMRegister) (armSrc0 + 1) ;
	ARMRegister	armTemp1 = (ARMRegister) (armTemp0 + 1) ;
	//
	write_mul_int64xint64
		( armDst0, armDst1, armSrc0, armSrc1, armTemp0, armTemp1 ) ;
	//
	SetDataRegisterModified( regClassARM, armDst0 ) ;
	UnlockDataRegister( regClassARM, armDst0 ) ;
	UnlockDataRegister( regClassARM, armSrc0 ) ;
	FreeDataRegister( regClassARM, armTemp0 ) ;
}

void ARMGenericAssembler::write_and_reg_reg( int regDst, int regSrc, bool fPair )
{
	if ( m_vfpNEON )
	{
		DataRegisterClass	regClass = fPair ? regClassNEON : regClassVFP ;
		int	vregDst = WriteRealizeDataRegister( regDst, regClass ) ;
		int	vregSrc = WriteRealizeDataRegister( regSrc, regClass ) ;
		if ( regClass == regClassNEON )
		{
			WriteSIMDOpRegRegReg
				( armOpVAND, thumbOpVAND,
					(vregDst << 1), (vregDst << 1), (vregSrc << 1), fPair ) ;
		}
		else
		{
			WriteSIMDOpRegRegReg
				( armOpVAND, thumbOpVAND,
					vregDst, vregDst, vregSrc, fPair ) ;
		}
		SetDataRegisterModified( regClass, vregDst ) ;
		UnlockDataRegister( regClass, vregDst ) ;
		UnlockDataRegister( regClass, vregSrc ) ;
	}
	else
	{
		ARMRegister	armDst0 =
			(ARMRegister) WriteRealizeDataRegister( regDst, regClassARM ) ;
		ARMRegister	armSrc0 =
			(ARMRegister) WriteRealizeDataRegister( regSrc, regClassARM ) ;
		ARMRegister	armDst1 = (ARMRegister) (armDst0 + 1) ;
		ARMRegister	armSrc1 = (ARMRegister) (armSrc0 + 1) ;
		//
		WriteARMAndRegRegRegShift( armDst0, armDst0, armSrc0 ) ;
		WriteARMAndRegRegRegShift( armDst1, armDst1, armSrc1 ) ;
		//
		SetDataRegisterModified( regClassARM, armDst0 ) ;
		UnlockDataRegister( regClassARM, armDst0 ) ;
		UnlockDataRegister( regClassARM, armSrc0 ) ;
	}
}

void ARMGenericAssembler::write_or_reg_reg( int regDst, int regSrc, bool fPair )
{
	if ( m_vfpNEON )
	{
		DataRegisterClass	regClass = fPair ? regClassNEON : regClassVFP ;
		int	vregDst = WriteRealizeDataRegister( regDst, regClass ) ;
		int	vregSrc = WriteRealizeDataRegister( regSrc, regClass ) ;
		if ( regClass == regClassNEON )
		{
			WriteSIMDOpRegRegReg
				( armOpVORR, thumbOpVORR,
					(vregDst << 1), (vregDst << 1), (vregSrc << 1), fPair ) ;
		}
		else
		{
			WriteSIMDOpRegRegReg
				( armOpVORR, thumbOpVORR,
					vregDst, vregDst, vregSrc, fPair ) ;
		}
		SetDataRegisterModified( regClass, vregDst ) ;
		UnlockDataRegister( regClass, vregDst ) ;
		UnlockDataRegister( regClass, vregSrc ) ;
	}
	else
	{
		ARMRegister	armDst0 =
			(ARMRegister) WriteRealizeDataRegister( regDst, regClassARM ) ;
		ARMRegister	armSrc0 =
			(ARMRegister) WriteRealizeDataRegister( regSrc, regClassARM ) ;
		ARMRegister	armDst1 = (ARMRegister) (armDst0 + 1) ;
		ARMRegister	armSrc1 = (ARMRegister) (armSrc0 + 1) ;
		//
		WriteARMOrRegRegRegShift( armDst0, armDst0, armSrc0 ) ;
		WriteARMOrRegRegRegShift( armDst1, armDst1, armSrc1 ) ;
		//
		SetDataRegisterModified( regClassARM, armDst0 ) ;
		UnlockDataRegister( regClassARM, armDst0 ) ;
		UnlockDataRegister( regClassARM, armSrc0 ) ;
	}
}

void ARMGenericAssembler::write_xor_reg_reg( int regDst, int regSrc, bool fPair )
{
	if ( m_vfpNEON )
	{
		DataRegisterClass	regClass = fPair ? regClassNEON : regClassVFP ;
		int	vregDst = WriteRealizeDataRegister( regDst, regClass ) ;
		int	vregSrc = WriteRealizeDataRegister( regSrc, regClass ) ;
		if ( regClass == regClassNEON )
		{
			WriteSIMDOpRegRegReg
				( armOpVEOR, thumbOpVEOR,
					(vregDst << 1), (vregDst << 1), (vregSrc << 1), fPair ) ;
		}
		else
		{
			WriteSIMDOpRegRegReg
				( armOpVEOR, thumbOpVEOR,
					vregDst, vregDst, vregSrc, fPair ) ;
		}
		SetDataRegisterModified( regClass, vregDst ) ;
		UnlockDataRegister( regClass, vregDst ) ;
		UnlockDataRegister( regClass, vregSrc ) ;
	}
	else
	{
		ARMRegister	armDst0 =
			(ARMRegister) WriteRealizeDataRegister( regDst, regClassARM ) ;
		ARMRegister	armSrc0 =
			(ARMRegister) WriteRealizeDataRegister( regSrc, regClassARM ) ;
		ARMRegister	armDst1 = (ARMRegister) (armDst0 + 1) ;
		ARMRegister	armSrc1 = (ARMRegister) (armSrc0 + 1) ;
		//
		WriteARMXorRegRegRegShift( armDst0, armDst0, armSrc0 ) ;
		WriteARMXorRegRegRegShift( armDst1, armDst1, armSrc1 ) ;
		//
		SetDataRegisterModified( regClassARM, armDst0 ) ;
		UnlockDataRegister( regClassARM, armDst0 ) ;
		UnlockDataRegister( regClassARM, armSrc0 ) ;
	}
}

void ARMGenericAssembler::write_srl_reg_reg( int regDst, int regSrc, bool fPair )
{
	if ( m_vfpNEON )
	{
		ARMRegister	armTemp0 = (ARMRegister) AllocateDataRegister( regClassARM ) ;
		ARMRegister	armTemp1 = (ARMRegister) (armTemp0 + 1) ;
		WriteToLoadSakura2Register( armTemp0, regSrc, true ) ;
		WriteARMXorRegRegRegShift( armTemp1, armTemp1, armTemp1 ) ;
		WriteARMAndRegRegImm8( armTemp0, armTemp0, 0x3F ) ;
		WriteARMSubRegRegImm8( armTemp0, armTemp1, armTemp0 ) ;
		//
		int	vregTemp = AllocateDataRegister( regClassVFP ) ;
		WriteMoveARMtoVFP64( vregTemp, armTemp0, armTemp1 ) ;
		//
		int	vregDst = WriteRealizeDataRegister( regDst, regClassVFP ) ;
		WriteSIMDShiftRegRegReg
			( vregDst, vregDst, vregTemp, typeNEONInt64, false, false ) ;
		SetDataRegisterModified( regClassVFP, vregDst ) ;
		UnlockDataRegister( regClassVFP, vregDst ) ;
		//
		if ( fPair )
		{
			vregDst = WriteRealizeDataRegister( regDst + 1, regClassVFP ) ;
			WriteSIMDShiftRegRegReg
				( vregDst, vregDst, vregTemp, typeNEONInt64, false, false ) ;
			SetDataRegisterModified( regClassVFP, vregDst ) ;
			UnlockDataRegister( regClassVFP, vregDst ) ;
		}
		//
		FreeDataRegister( regClassVFP, vregTemp ) ;
		FreeDataRegister( regClassARM, armTemp0 ) ;
	}
	else
	{
		Sakura2Assembler::write_srl_reg_reg( regDst, regSrc, fPair ) ;
	}
}

void ARMGenericAssembler::write_sra_reg_reg( int regDst, int regSrc, bool fPair )
{
	if ( m_vfpNEON )
	{
		ARMRegister	armTemp0 = (ARMRegister) AllocateDataRegister( regClassARM ) ;
		ARMRegister	armTemp1 = (ARMRegister) (armTemp0 + 1) ;
		WriteToLoadSakura2Register( armTemp0, regSrc, true ) ;
		WriteARMXorRegRegRegShift( armTemp1, armTemp1, armTemp1 ) ;
		WriteARMAndRegRegImm8( armTemp0, armTemp0, 0x3F ) ;
		WriteARMSubRegRegImm8( armTemp0, armTemp1, armTemp0 ) ;
		//
		int	vregTemp = AllocateDataRegister( regClassVFP ) ;
		WriteMoveARMtoVFP64( vregTemp, armTemp0, armTemp1 ) ;
		//
		int	vregDst = WriteRealizeDataRegister( regDst, regClassVFP ) ;
		WriteSIMDShiftRegRegReg
			( vregDst, vregDst, vregTemp, typeNEONInt64, true, false ) ;
		SetDataRegisterModified( regClassVFP, vregDst ) ;
		UnlockDataRegister( regClassVFP, vregDst ) ;
		//
		if ( fPair )
		{
			vregDst = WriteRealizeDataRegister( regDst + 1, regClassVFP ) ;
			WriteSIMDShiftRegRegReg
				( vregDst, vregDst, vregTemp, typeNEONInt64, true, false ) ;
			SetDataRegisterModified( regClassVFP, vregDst ) ;
			UnlockDataRegister( regClassVFP, vregDst ) ;
		}
		//
		FreeDataRegister( regClassVFP, vregTemp ) ;
		FreeDataRegister( regClassARM, armTemp0 ) ;
	}
	else
	{
		Sakura2Assembler::write_sra_reg_reg( regDst, regSrc, fPair ) ;
	}
}

void ARMGenericAssembler::write_sll_reg_reg( int regDst, int regSrc, bool fPair )
{
	if ( m_vfpNEON )
	{
		ARMRegister	armTemp0 = (ARMRegister) AllocateDataRegister( regClassARM ) ;
		ARMRegister	armTemp1 = (ARMRegister) (armTemp0 + 1) ;
		WriteToLoadSakura2Register( armTemp0, regSrc, true ) ;
		WriteARMXorRegRegRegShift( armTemp1, armTemp1, armTemp1 ) ;
		WriteARMAndRegRegImm8( armTemp0, armTemp0, 0x3F ) ;
		//
		int	vregTemp = AllocateDataRegister( regClassVFP ) ;
		WriteMoveARMtoVFP64( vregTemp, armTemp0, armTemp1 ) ;
		//
		int	vregDst = WriteRealizeDataRegister( regDst, regClassVFP ) ;
		WriteSIMDShiftRegRegReg
			( vregDst, vregDst, vregTemp, typeNEONInt64, false, false ) ;
		SetDataRegisterModified( regClassVFP, vregDst ) ;
		UnlockDataRegister( regClassVFP, vregDst ) ;
		//
		if ( fPair )
		{
			vregDst = WriteRealizeDataRegister( regDst + 1, regClassVFP ) ;
			WriteSIMDShiftRegRegReg
				( vregDst, vregDst, vregTemp, typeNEONInt64, false, false ) ;
			SetDataRegisterModified( regClassVFP, vregDst ) ;
			UnlockDataRegister( regClassVFP, vregDst ) ;
		}
		//
		FreeDataRegister( regClassVFP, vregTemp ) ;
		FreeDataRegister( regClassARM, armTemp0 ) ;
	}
	else
	{
		Sakura2Assembler::write_sll_reg_reg( regDst, regSrc, fPair ) ;
	}
}

// 整数符号拡張命令
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::write_move_sx32_reg_reg( int regDst, int regSrc )
{
	if ( m_vfpNEON )
	{
		int	vregDst =
			WriteRealizeDataRegister( regDst, regClassVFP, (regDst == regSrc) ) ;
		if ( regDst != regSrc )
		{
			RealizeFreeVFPRegister( vregDst, regSrc, true, false ) ;
		}
		WriteSIMDShiftRegRegImm
			( armOpVSHLImm, thumbOpVSHLImm,
				vregDst, vregDst, 32, typeNEONInt64, false, false, false ) ;
		WriteSIMDShiftRegRegImm
			( armOpVSHRImm, thumbOpVSHRImm,
				vregDst, vregDst, 32, typeNEONInt64, true, true, false ) ;
		SetDataRegisterModified( regClassVFP, vregDst ) ;
		UnlockDataRegister( regClassVFP, vregDst ) ;
	}
	else
	{
		ARMRegister	armDst0 =
			(ARMRegister) WriteRealizeDataRegister
					( regDst, regClassARM, (regDst == regSrc) ) ;
		if ( regDst != regSrc )
		{
			RealizeFreeARMRegister( armDst0, regSrc, true, false ) ;
		}
		ARMRegister	armDst1 = (ARMRegister) (armDst0 + 1) ;
		WriteARMShiftARightImm( armDst1, armDst0, 31 ) ;
		SetDataRegisterModified( regClassARM, armDst0 ) ;
		UnlockDataRegister( regClassARM, armDst0 ) ;
	}
}

void ARMGenericAssembler::write_move_sx16_reg_reg( int regDst, int regSrc )
{
	if ( m_vfpNEON )
	{
		int	vregDst =
			WriteRealizeDataRegister( regDst, regClassVFP, (regDst == regSrc) ) ;
		if ( regDst != regSrc )
		{
			RealizeFreeVFPRegister( vregDst, regSrc, true, false ) ;
		}
		WriteSIMDShiftRegRegImm
			( armOpVSHLImm, thumbOpVSHLImm,
				vregDst, vregDst, 64-16, typeNEONInt64, false, false, false ) ;
		WriteSIMDShiftRegRegImm
			( armOpVSHRImm, thumbOpVSHRImm,
				vregDst, vregDst, 64-16, typeNEONInt64, true, true, false ) ;
		SetDataRegisterModified( regClassVFP, vregDst ) ;
		UnlockDataRegister( regClassVFP, vregDst ) ;
	}
	else
	{
		ARMRegister	armDst0 =
			(ARMRegister) WriteRealizeDataRegister
					( regDst, regClassARM, (regDst == regSrc) ) ;
		if ( regDst != regSrc )
		{
			RealizeFreeARMRegister( armDst0, regSrc, true, false ) ;
		}
		ARMRegister	armDst1 = (ARMRegister) (armDst0 + 1) ;
		WriteARMShiftLeftImm( armDst0, armDst0, 16 ) ;
		WriteARMShiftARightImm( armDst1, armDst0, 31 ) ;
		WriteARMShiftARightImm( armDst0, armDst0, 16 ) ;
		SetDataRegisterModified( regClassARM, armDst0 ) ;
		UnlockDataRegister( regClassARM, armDst0 ) ;
	}
}

void ARMGenericAssembler::write_move_sx8_reg_reg( int regDst, int regSrc )
{
	if ( m_vfpNEON )
	{
		int	vregDst =
			WriteRealizeDataRegister( regDst, regClassVFP, (regDst == regSrc) ) ;
		if ( regDst != regSrc )
		{
			RealizeFreeVFPRegister( vregDst, regSrc, true, false ) ;
		}
		WriteSIMDShiftRegRegImm
			( armOpVSHLImm, thumbOpVSHLImm,
				vregDst, vregDst, 64-8, typeNEONInt64, false, false, false ) ;
		WriteSIMDShiftRegRegImm
			( armOpVSHRImm, thumbOpVSHRImm,
				vregDst, vregDst, 64-8, typeNEONInt64, true, true, false ) ;
		SetDataRegisterModified( regClassVFP, vregDst ) ;
		UnlockDataRegister( regClassVFP, vregDst ) ;
	}
	else
	{
		ARMRegister	armDst0 =
			(ARMRegister) WriteRealizeDataRegister
					( regDst, regClassARM, (regDst == regSrc) ) ;
		if ( regDst != regSrc )
		{
			RealizeFreeARMRegister( armDst0, regSrc, true, false ) ;
		}
		ARMRegister	armDst1 = (ARMRegister) (armDst0 + 1) ;
		WriteARMShiftLeftImm( armDst0, armDst0, 32-8 ) ;
		WriteARMShiftARightImm( armDst1, armDst0, 31 ) ;
		WriteARMShiftARightImm( armDst0, armDst0, 32-8 ) ;
		SetDataRegisterModified( regClassARM, armDst0 ) ;
		UnlockDataRegister( regClassARM, armDst0 ) ;
	}
}

// 2 OP 実数演算命令
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::write_fadd_reg_reg( int regDst, int regSrc, bool fPair )
{
	if ( m_vfpVersion >= 2 )
	{
		int	vregDst = WriteRealizeDataRegister( regDst, regClassVFP ) ;
		int	vregSrc = WriteRealizeDataRegister( regSrc, regClassVFP ) ;
		WriteVFPOpRegRegReg
			( armOpFADD, thumbOpFADD,
				vregDst, vregDst, vregSrc, true ) ;
		SetDataRegisterModified( regClassVFP, vregDst ) ;
		UnlockDataRegister( regClassVFP, vregDst ) ;
		UnlockDataRegister( regClassVFP, vregSrc ) ;
		//
		if ( fPair )
		{
			write_fadd_reg_reg( regDst + 1, regSrc + 1, false ) ;
		}
	}
	else
	{
		Sakura2Assembler::write_fadd_reg_reg( regDst, regSrc, fPair ) ;
	}
}

void ARMGenericAssembler::write_fsub_reg_reg( int regDst, int regSrc, bool fPair )
{
	if ( m_vfpVersion >= 2 )
	{
		int	vregDst = WriteRealizeDataRegister( regDst, regClassVFP ) ;
		int	vregSrc = WriteRealizeDataRegister( regSrc, regClassVFP ) ;
		WriteVFPOpRegRegReg
			( armOpFSUB, thumbOpFSUB,
				vregDst, vregDst, vregSrc, true ) ;
		SetDataRegisterModified( regClassVFP, vregDst ) ;
		UnlockDataRegister( regClassVFP, vregDst ) ;
		UnlockDataRegister( regClassVFP, vregSrc ) ;
		//
		if ( fPair )
		{
			write_fadd_reg_reg( regDst + 1, regSrc + 1, false ) ;
		}
	}
	else
	{
		Sakura2Assembler::write_fsub_reg_reg( regDst, regSrc, fPair ) ;
	}
}

void ARMGenericAssembler::write_fmul_reg_reg( int regDst, int regSrc, bool fPair )
{
	if ( m_vfpVersion >= 2 )
	{
		int	vregDst = WriteRealizeDataRegister( regDst, regClassVFP ) ;
		int	vregSrc = WriteRealizeDataRegister( regSrc, regClassVFP ) ;
		WriteVFPOpRegRegReg
			( armOpFMUL, thumbOpFMUL,
				vregDst, vregDst, vregSrc, true ) ;
		SetDataRegisterModified( regClassVFP, vregDst ) ;
		UnlockDataRegister( regClassVFP, vregDst ) ;
		UnlockDataRegister( regClassVFP, vregSrc ) ;
		//
		if ( fPair )
		{
			write_fadd_reg_reg( regDst + 1, regSrc + 1, false ) ;
		}
	}
	else
	{
		Sakura2Assembler::write_fmul_reg_reg( regDst, regSrc, fPair ) ;
	}
}

void ARMGenericAssembler::write_fdiv_reg_reg( int regDst, int regSrc, bool fPair )
{
	if ( m_vfpVersion >= 2 )
	{
		int	vregDst = WriteRealizeDataRegister( regDst, regClassVFP ) ;
		int	vregSrc = WriteRealizeDataRegister( regSrc, regClassVFP ) ;
		WriteVFPOpRegRegReg
			( armOpFDIV, thumbOpFDIV,
				vregDst, vregDst, vregSrc, true ) ;
		SetDataRegisterModified( regClassVFP, vregDst ) ;
		UnlockDataRegister( regClassVFP, vregDst ) ;
		UnlockDataRegister( regClassVFP, vregSrc ) ;
		//
		if ( fPair )
		{
			write_fadd_reg_reg( regDst + 1, regSrc + 1, false ) ;
		}
	}
	else
	{
		Sakura2Assembler::write_fdiv_reg_reg( regDst, regSrc, fPair ) ;
	}
}

// 特殊精度整数演算命令
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::write_mul32_reg_reg( int regDst, int regSrc )
{
	ARMRegister	armDst0 =
		(ARMRegister) WriteRealizeDataRegister( regDst, regClassARM ) ;
	ARMRegister	armSrc0 =
		(ARMRegister) WriteRealizeDataRegister( regSrc, regClassARM ) ;
	ARMRegister	armDst1 = (ARMRegister) (armDst0 + 1) ;
	//
	WriteARMMulUInt64( armDst0, armDst1, armDst0, armSrc0 ) ;
	//
	SetDataRegisterModified( regClassARM, armDst0 ) ;
	UnlockDataRegister( regClassARM, armDst0 ) ;
	UnlockDataRegister( regClassARM, armSrc0 ) ;
}

void ARMGenericAssembler::write_imul32_reg_reg( int regDst, int regSrc )
{
	ARMRegister	armDst0 =
		(ARMRegister) WriteRealizeDataRegister( regDst, regClassARM ) ;
	ARMRegister	armSrc0 =
		(ARMRegister) WriteRealizeDataRegister( regSrc, regClassARM ) ;
	ARMRegister	armDst1 = (ARMRegister) (armDst0 + 1) ;
	//
	WriteARMMulSInt64( armDst0, armDst1, armDst0, armSrc0 ) ;
	//
	SetDataRegisterModified( regClassARM, armDst0 ) ;
	UnlockDataRegister( regClassARM, armDst0 ) ;
	UnlockDataRegister( regClassARM, armSrc0 ) ;
}

// 整数比較命令
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::write_cmp_ne( int regDst, int regSrc, bool fPair )
{
	if ( m_vfpNEON )
	{
		DataRegisterClass	regClass = fPair ? regClassNEON : regClassVFP ;
		int	vregDst = WriteRealizeDataRegister( regDst, regClass ) ;
		int	vregSrc = WriteRealizeDataRegister( regSrc, regClass ) ;
		int	vregTemp = AllocateDataRegister( regClass ) ;
		int	vfpDst = vregDst, vfpSrc = vregSrc, vfpTemp = vregTemp ;
		if ( regClass == regClassNEON )
		{
			vfpDst <<= 1 ;
			vfpSrc <<= 1 ;
			vfpTemp <<= 1 ;
		}
		WriteSIMDIntOpRegRegReg
			( armOpVICmpEQ, thumbOpVICmpEQ,
				vfpDst, vfpDst, vfpSrc, typeNEONInt32, fPair ) ;
		WriteSIMDIntNotRegReg( vfpDst, vfpDst, typeNEONInt64, fPair ) ;
		WriteSIMDRevRegReg
			( vfpTemp, vfpDst, typeNEONInt64, typeNEONInt32, fPair ) ;
		WriteSIMDOpRegRegReg
			( armOpVORR, thumbOpVORR, vfpDst, vfpDst, vfpTemp, fPair ) ;
		//
		SetDataRegisterModified( regClass, vregDst ) ;
		UnlockDataRegister( regClass, vregDst ) ;
		UnlockDataRegister( regClass, vregSrc ) ;
		FreeDataRegister( regClass, vregTemp ) ;
	}
	else
	{
		ARMRegister	armDst0 =
			(ARMRegister) WriteRealizeDataRegister( regDst, regClassARM ) ;
		ARMRegister	armSrc0 =
			(ARMRegister) WriteRealizeDataRegister( regSrc, regClassARM ) ;
		ARMRegister	armDst1 = (ARMRegister) (armDst0 + 1) ;
		ARMRegister	armSrc1 = (ARMRegister) (armSrc0 + 1) ;
		//
		WriteARMCmpRegRegShift( armDst0, armSrc0 ) ;
		WriteARMXorRegRegRegShift
			( armDst0, armDst0, armDst0, 0, cond_AL, false ) ;
		WriteARMSubRegRegImm8( armDst0, armDst0, 1, cond_NE ) ;
		//
		WriteARMCmpRegRegShift( armDst1, armSrc1 ) ;
		WriteARMXorRegRegRegShift
			( armDst1, armDst1, armDst1, 0, cond_AL, false ) ;
		WriteARMSubRegRegImm8( armDst1, armDst1, 1, cond_NE ) ;
		//
		WriteARMOrRegRegRegShift( armDst0, armDst0, armDst1 ) ;
		WriteARMMoveRegReg( armDst1, armDst0 ) ;
		//
		SetDataRegisterModified( regClassARM, armDst0 ) ;
		UnlockDataRegister( regClassARM, armDst0 ) ;
		UnlockDataRegister( regClassARM, armSrc0 ) ;
		//
		if ( fPair )
		{
			write_cmp_ne( regDst + 1, regSrc + 1, false ) ;
		}
	}
}

void ARMGenericAssembler::write_cmp_eq( int regDst, int regSrc, bool fPair )
{
	if ( m_vfpNEON )
	{
		DataRegisterClass	regClass = fPair ? regClassNEON : regClassVFP ;
		int	vregDst = WriteRealizeDataRegister( regDst, regClass ) ;
		int	vregSrc = WriteRealizeDataRegister( regSrc, regClass ) ;
		int	vregTemp = AllocateDataRegister( regClass ) ;
		int	vfpDst = vregDst, vfpSrc = vregSrc, vfpTemp = vregTemp ;
		if ( regClass == regClassNEON )
		{
			vfpDst <<= 1 ;
			vfpSrc <<= 1 ;
			vfpTemp <<= 1 ;
		}
		WriteSIMDIntOpRegRegReg
			( armOpVICmpEQ, thumbOpVICmpEQ,
				vfpDst, vfpDst, vfpSrc, typeNEONInt32, fPair ) ;
		WriteSIMDRevRegReg
			( vfpTemp, vfpDst, typeNEONInt64, typeNEONInt32, fPair ) ;
		WriteSIMDOpRegRegReg
			( armOpVAND, thumbOpVAND, vfpDst, vfpDst, vfpTemp, fPair ) ;
		//
		SetDataRegisterModified( regClass, vregDst ) ;
		UnlockDataRegister( regClass, vregDst ) ;
		UnlockDataRegister( regClass, vregSrc ) ;
		FreeDataRegister( regClass, vregTemp ) ;
	}
	else
	{
		ARMRegister	armDst0 =
			(ARMRegister) WriteRealizeDataRegister( regDst, regClassARM ) ;
		ARMRegister	armSrc0 =
			(ARMRegister) WriteRealizeDataRegister( regSrc, regClassARM ) ;
		ARMRegister	armDst1 = (ARMRegister) (armDst0 + 1) ;
		ARMRegister	armSrc1 = (ARMRegister) (armSrc0 + 1) ;
		//
		WriteARMCmpRegRegShift( armDst0, armSrc0 ) ;
		WriteARMXorRegRegRegShift
			( armDst0, armDst0, armDst0, 0, cond_AL, false ) ;
		WriteARMSubRegRegImm8( armDst0, armDst0, 1, cond_EQ ) ;
		//
		WriteARMCmpRegRegShift( armDst1, armSrc1 ) ;
		WriteARMXorRegRegRegShift
			( armDst1, armDst1, armDst1, 0, cond_AL, false ) ;
		WriteARMSubRegRegImm8( armDst1, armDst1, 1, cond_EQ ) ;
		//
		WriteARMAndRegRegRegShift( armDst0, armDst0, armDst1 ) ;
		WriteARMMoveRegReg( armDst1, armDst0 ) ;
		//
		SetDataRegisterModified( regClassARM, armDst0 ) ;
		UnlockDataRegister( regClassARM, armDst0 ) ;
		UnlockDataRegister( regClassARM, armSrc0 ) ;
		//
		if ( fPair )
		{
			write_cmp_eq( regDst + 1, regSrc + 1, false ) ;
		}
	}
}

void ARMGenericAssembler::write_cmp_lt( int regDst, int regSrc, bool fPair )
{
	ARMRegister	armDst0 =
		(ARMRegister) WriteRealizeDataRegister( regDst, regClassARM ) ;
	ARMRegister	armSrc0 =
		(ARMRegister) WriteRealizeDataRegister( regSrc, regClassARM ) ;
	ARMRegister	armTemp = (ARMRegister) AllocateDataRegister( regClassARM ) ;
	//
	write_arm_cmp_int64_gt
		( armDst0, armTemp, armSrc0, armDst0, false, false ) ;
	//
	SetDataRegisterModified( regClassARM, armDst0 ) ;
	UnlockDataRegister( regClassARM, armDst0 ) ;
	UnlockDataRegister( regClassARM, armSrc0 ) ;
	FreeDataRegister( regClassARM, armTemp ) ;
	//
	if ( fPair )
	{
		write_cmp_lt( regDst + 1, regSrc + 1, false ) ;
	}
}

void ARMGenericAssembler::write_cmp_le( int regDst, int regSrc, bool fPair )
{
	ARMRegister	armDst0 =
		(ARMRegister) WriteRealizeDataRegister( regDst, regClassARM ) ;
	ARMRegister	armSrc0 =
		(ARMRegister) WriteRealizeDataRegister( regSrc, regClassARM ) ;
	ARMRegister	armTemp = (ARMRegister) AllocateDataRegister( regClassARM ) ;
	//
	write_arm_cmp_int64_gt
		( armDst0, armTemp, armDst0, armSrc0, true, false ) ;
	//
	SetDataRegisterModified( regClassARM, armDst0 ) ;
	UnlockDataRegister( regClassARM, armDst0 ) ;
	UnlockDataRegister( regClassARM, armSrc0 ) ;
	FreeDataRegister( regClassARM, armTemp ) ;
	//
	if ( fPair )
	{
		write_cmp_le( regDst + 1, regSrc + 1, false ) ;
	}
}

void ARMGenericAssembler::write_cmp_gt( int regDst, int regSrc, bool fPair )
{
	ARMRegister	armDst0 =
		(ARMRegister) WriteRealizeDataRegister( regDst, regClassARM ) ;
	ARMRegister	armSrc0 =
		(ARMRegister) WriteRealizeDataRegister( regSrc, regClassARM ) ;
	ARMRegister	armTemp = (ARMRegister) AllocateDataRegister( regClassARM ) ;
	//
	write_arm_cmp_int64_gt
		( armDst0, armTemp, armDst0, armSrc0, false, false ) ;
	//
	SetDataRegisterModified( regClassARM, armDst0 ) ;
	UnlockDataRegister( regClassARM, armDst0 ) ;
	UnlockDataRegister( regClassARM, armSrc0 ) ;
	FreeDataRegister( regClassARM, armTemp ) ;
	//
	if ( fPair )
	{
		write_cmp_gt( regDst + 1, regSrc + 1, false ) ;
	}
}

void ARMGenericAssembler::write_cmp_ge( int regDst, int regSrc, bool fPair )
{
	ARMRegister	armDst0 =
		(ARMRegister) WriteRealizeDataRegister( regDst, regClassARM ) ;
	ARMRegister	armSrc0 =
		(ARMRegister) WriteRealizeDataRegister( regSrc, regClassARM ) ;
	ARMRegister	armTemp = (ARMRegister) AllocateDataRegister( regClassARM ) ;
	//
	write_arm_cmp_int64_gt
		( armDst0, armTemp, armSrc0, armDst0, true, false ) ;
	//
	SetDataRegisterModified( regClassARM, armDst0 ) ;
	UnlockDataRegister( regClassARM, armDst0 ) ;
	UnlockDataRegister( regClassARM, armSrc0 ) ;
	FreeDataRegister( regClassARM, armTemp ) ;
	//
	if ( fPair )
	{
		write_cmp_ge( regDst + 1, regSrc + 1, false ) ;
	}
}

void ARMGenericAssembler::write_cmp_c( int regDst, int regSrc, bool fPair )
{
	ARMRegister	armDst0 =
		(ARMRegister) WriteRealizeDataRegister( regDst, regClassARM ) ;
	ARMRegister	armSrc0 =
		(ARMRegister) WriteRealizeDataRegister( regSrc, regClassARM ) ;
	ARMRegister	armTemp = (ARMRegister) AllocateDataRegister( regClassARM ) ;
	//
	write_arm_cmp_int64_gt
		( armDst0, armTemp, armSrc0, armDst0, false, true ) ;
	//
	SetDataRegisterModified( regClassARM, armDst0 ) ;
	UnlockDataRegister( regClassARM, armDst0 ) ;
	UnlockDataRegister( regClassARM, armSrc0 ) ;
	FreeDataRegister( regClassARM, armTemp ) ;
	//
	if ( fPair )
	{
		write_cmp_c( regDst + 1, regSrc + 1, false ) ;
	}
}

void ARMGenericAssembler::write_cmp_cz( int regDst, int regSrc, bool fPair )
{
	ARMRegister	armDst0 =
		(ARMRegister) WriteRealizeDataRegister( regDst, regClassARM ) ;
	ARMRegister	armSrc0 =
		(ARMRegister) WriteRealizeDataRegister( regSrc, regClassARM ) ;
	ARMRegister	armTemp = (ARMRegister) AllocateDataRegister( regClassARM ) ;
	//
	write_arm_cmp_int64_gt
		( armDst0, armTemp, armDst0, armSrc0, true, true ) ;
	//
	SetDataRegisterModified( regClassARM, armDst0 ) ;
	UnlockDataRegister( regClassARM, armDst0 ) ;
	UnlockDataRegister( regClassARM, armSrc0 ) ;
	FreeDataRegister( regClassARM, armTemp ) ;
	//
	if ( fPair )
	{
		write_cmp_cz( regDst + 1, regSrc + 1, false ) ;
	}
}

// 実数比較命令
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::write_fcmp_ne( int regDst, int regSrc, bool fPair )
{
	if ( m_vfpVersion >= 2 )
	{
		write_vfp_cmp_float64( regDst, regSrc, cond_NE ) ;
		//
		if ( fPair )
		{
			write_vfp_cmp_float64( regDst + 1, regSrc + 1, cond_NE ) ;
		}
	}
	else
	{
		Sakura2Assembler::write_fcmp_ne( regDst, regSrc, fPair ) ;
	}
}

void ARMGenericAssembler::write_fcmp_eq( int regDst, int regSrc, bool fPair )
{
	if ( m_vfpVersion >= 2 )
	{
		write_vfp_cmp_float64( regDst, regSrc, cond_EQ ) ;
		//
		if ( fPair )
		{
			write_vfp_cmp_float64( regDst + 1, regSrc + 1, cond_EQ ) ;
		}
	}
	else
	{
		Sakura2Assembler::write_fcmp_eq( regDst, regSrc, fPair ) ;
	}
}

void ARMGenericAssembler::write_fcmp_lt( int regDst, int regSrc, bool fPair )
{
	if ( m_vfpVersion >= 2 )
	{
		write_vfp_cmp_float64( regDst, regSrc, cond_LT ) ;
		//
		if ( fPair )
		{
			write_vfp_cmp_float64( regDst + 1, regSrc + 1, cond_LT ) ;
		}
	}
	else
	{
		Sakura2Assembler::write_fcmp_lt( regDst, regSrc, fPair ) ;
	}
}

void ARMGenericAssembler::write_fcmp_le( int regDst, int regSrc, bool fPair )
{
	if ( m_vfpVersion >= 2 )
	{
		write_vfp_cmp_float64( regDst, regSrc, cond_LE ) ;
		//
		if ( fPair )
		{
			write_vfp_cmp_float64( regDst + 1, regSrc + 1, cond_LE ) ;
		}
	}
	else
	{
		Sakura2Assembler::write_fcmp_le( regDst, regSrc, fPair ) ;
	}
}

void ARMGenericAssembler::write_fcmp_gt( int regDst, int regSrc, bool fPair )
{
	if ( m_vfpVersion >= 2 )
	{
		write_vfp_cmp_float64( regDst, regSrc, cond_GT ) ;
		//
		if ( fPair )
		{
			write_vfp_cmp_float64( regDst + 1, regSrc + 1, cond_GT ) ;
		}
	}
	else
	{
		Sakura2Assembler::write_fcmp_gt( regDst, regSrc, fPair ) ;
	}
}

void ARMGenericAssembler::write_fcmp_ge( int regDst, int regSrc, bool fPair )
{
	if ( m_vfpVersion >= 2 )
	{
		write_vfp_cmp_float64( regDst, regSrc, cond_GE ) ;
		//
		if ( fPair )
		{
			write_vfp_cmp_float64( regDst + 1, regSrc + 1, cond_GE ) ;
		}
	}
	else
	{
		Sakura2Assembler::write_fcmp_ge( regDst, regSrc, fPair ) ;
	}
}

// 浮動小数点演算 EXTENSION
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::write_float_extension( int code, int regDst, int regSrc )
{
	int	vregSrc, vregDst ;
	if ( m_vfpVersion >= 2 )
	switch ( code )
	{
	case	fcodeFabs:
		vregSrc = WriteRealizeDataRegister( regSrc, regClassVFP, true ) ;
		vregDst = WriteRealizeDataRegister( regDst, regClassVFP, false ) ;
		//
		WriteVFPOpRegReg
			( armOpFABS, thumbOpFABS, vregDst, vregSrc, true ) ;
		//
		SetDataRegisterModified( regClassVFP, vregDst ) ;
		UnlockDataRegister( regClassVFP, vregDst ) ;
		UnlockDataRegister( regClassVFP, vregSrc ) ;
		return ;

	case	fcodeSqrt:
		vregSrc = WriteRealizeDataRegister( regSrc, regClassVFP, true ) ;
		vregDst = WriteRealizeDataRegister( regDst, regClassVFP, false ) ;
		//
		WriteVFPOpRegReg
			( armOpFSQRT, thumbOpFSQRT, vregDst, vregSrc, true ) ;
		//
		SetDataRegisterModified( regClassVFP, vregDst ) ;
		UnlockDataRegister( regClassVFP, vregDst ) ;
		UnlockDataRegister( regClassVFP, vregSrc ) ;
		return ;

	default:
		break ;
	}
	Sakura2Assembler::write_float_extension( code, regDst, regSrc ) ;
}

// 64bit SIMD
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::write_simd64_extension( int code, int regDst, int regSrc, bool fPair )
{
	if ( m_armVersion < 6 )
	{
		Sakura2Assembler::write_simd64_extension( code, regDst, regSrc, fPair ) ;
		return ;
	}
	if ( m_vfpNEON )
	{
		DataRegisterClass	regClassDst = fPair ? regClassNEON : regClassVFP ;
		DataRegisterClass	regClassSrc = regClassDst ;
		DataRegisterClass	regClassTemp = regClassDst ;
		ARMRegister	armTemp0 = ARM_Nothing, armTemp1 = ARM_Nothing ;
		int			vregTemp = ARM_Nothing, dregTemp = ARM_Nothing ;
		bool		fSrcPair = fPair ;
		switch ( code )
		{
		case	simdPsrlw:
		case	simdPsrld:
		case	simdPsraw:
		case	simdPsrad:
		case	simdPsllw:
		case	simdPslld:
			fSrcPair = false ;
			regClassSrc = regClassVFP ;
			vregTemp = AllocateDataRegister( regClassTemp ) ;
			dregTemp = (regClassTemp == regClassNEON) ? (vregTemp << 1) : vregTemp ;
			armTemp0 = (ARMRegister) AllocateDataRegister( regClassARM ) ;
			armTemp1 = (ARMRegister) (armTemp0 + 1) ;
			break ;
		}
		int	vregSrc = WriteRealizeDataRegister( regSrc, regClassSrc ) ;
		int	vregDst = WriteRealizeDataRegister( regDst, regClassDst ) ;
		int	dregSrc = (regClassSrc == regClassNEON) ? (vregSrc << 1) : vregSrc ;
		int	dregDst = (regClassDst == regClassNEON) ? (vregDst << 1) : vregDst ;
		//
		switch ( code )
		{
		case	simdPaddub:
			WriteSIMDIntOpRegRegReg
				( armOpVQADD, thumbOpVQADD,
					dregDst, dregDst, dregSrc, typeNEONInt8, fPair, true ) ;
			break ;
		case	simdPaddsb:
			WriteSIMDIntOpRegRegReg
				( armOpVQADD, thumbOpVQADD,
					dregDst, dregDst, dregSrc, typeNEONInt8, fPair, false ) ;
			break ;
		case	simdPaddb:
			WriteSIMDIntOpRegRegReg
				( armOpVIADD, thumbOpVIADD,
					dregDst, dregDst, dregSrc, typeNEONInt8, fPair, false ) ;
			break ;
		case	simdPadduw:
			WriteSIMDIntOpRegRegReg
				( armOpVQADD, thumbOpVQADD,
					dregDst, dregDst, dregSrc, typeNEONInt16, fPair, true ) ;
			break ;
		case	simdPaddsw:
			WriteSIMDIntOpRegRegReg
				( armOpVQADD, thumbOpVQADD,
					dregDst, dregDst, dregSrc, typeNEONInt16, fPair, false ) ;
			break ;
		case	simdPaddw:
			WriteSIMDIntOpRegRegReg
				( armOpVIADD, thumbOpVIADD,
					dregDst, dregDst, dregSrc, typeNEONInt16, fPair, false ) ;
			break ;
		case	simdPaddd:
			WriteSIMDIntOpRegRegReg
				( armOpVIADD, thumbOpVIADD,
					dregDst, dregDst, dregSrc, typeNEONInt32, fPair, false ) ;
			break ;

		case	simdPsubub:
			WriteSIMDIntOpRegRegReg
				( armOpVQSUB, thumbOpVQSUB,
					dregDst, dregDst, dregSrc, typeNEONInt8, fPair, true ) ;
			break ;
		case	simdPsubsb:
			WriteSIMDIntOpRegRegReg
				( armOpVQSUB, thumbOpVQSUB,
					dregDst, dregDst, dregSrc, typeNEONInt8, fPair, false ) ;
			break ;
		case	simdPsubb:
			WriteSIMDIntOpRegRegReg
				( armOpVISUB, thumbOpVISUB,
					dregDst, dregDst, dregSrc, typeNEONInt8, fPair, false ) ;
			break ;
		case	simdPsubuw:
			WriteSIMDIntOpRegRegReg
				( armOpVQSUB, thumbOpVQSUB,
					dregDst, dregDst, dregSrc, typeNEONInt16, fPair, true ) ;
			break ;
		case	simdPsubsw:
			WriteSIMDIntOpRegRegReg
				( armOpVQSUB, thumbOpVQSUB,
					dregDst, dregDst, dregSrc, typeNEONInt16, fPair, false ) ;
			break ;
		case	simdPsubw:
			WriteSIMDIntOpRegRegReg
				( armOpVISUB, thumbOpVISUB,
					dregDst, dregDst, dregSrc, typeNEONInt16, fPair, false ) ;
			break ;
		case	simdPsubd:
			WriteSIMDIntOpRegRegReg
				( armOpVISUB, thumbOpVISUB,
					dregDst, dregDst, dregSrc, typeNEONInt32, fPair, false ) ;
			break ;

		case	simdPsrlw:
			WriteARMMoveRegImm( armTemp0, 0x0f ) ;
			WriteMoveARMtoVFP64( dregTemp, armTemp0, armTemp1 ) ;
			WriteSIMDOpRegRegReg
				( armOpVAND, thumbOpVAND, dregTemp, dregTemp, dregSrc, false ) ;
			WriteSIMDIntNegRegReg
				( dregTemp, dregTemp, typeNEONInt16, false, fPair ) ;
			WriteSIMDDupRegRegImm
				( dregTemp, dregTemp, 0, typeNEONInt16, fPair ) ;
			WriteSIMDIntOpRegRegReg
				( armOpVSHL, thumbOpVSHL,
					dregDst, dregDst, dregTemp, typeNEONInt16, fPair, true ) ;
			break ;
		case	simdPsrld:
			WriteARMMoveRegImm( armTemp0, 0x1f ) ;
			WriteMoveARMtoVFP64( dregTemp, armTemp0, armTemp1 ) ;
			WriteSIMDOpRegRegReg
				( armOpVAND, thumbOpVAND, dregTemp, dregTemp, dregSrc, false ) ;
			WriteSIMDDupRegRegImm
				( dregTemp, dregTemp, 0, typeNEONInt32, fPair ) ;
			WriteSIMDIntOpRegRegReg
				( armOpVSHL, thumbOpVSHL,
					dregDst, dregDst, dregTemp, typeNEONInt32, fPair, true ) ;
			break ;
		case	simdPsraw:
			WriteARMMoveRegImm( armTemp0, 0x0f ) ;
			WriteMoveARMtoVFP64( dregTemp, armTemp0, armTemp1 ) ;
			WriteSIMDOpRegRegReg
				( armOpVAND, thumbOpVAND, dregTemp, dregTemp, dregSrc, false ) ;
			WriteSIMDIntNegRegReg
				( dregTemp, dregTemp, typeNEONInt16, false, fPair ) ;
			WriteSIMDDupRegRegImm
				( dregTemp, dregTemp, 0, typeNEONInt16, fPair ) ;
			WriteSIMDIntOpRegRegReg
				( armOpVSHL, thumbOpVSHL,
					dregDst, dregDst, dregTemp, typeNEONInt16, fPair, false ) ;
			break ;
		case	simdPsrad:
			WriteARMMoveRegImm( armTemp0, 0x1f ) ;
			WriteMoveARMtoVFP64( dregTemp, armTemp0, armTemp1 ) ;
			WriteSIMDOpRegRegReg
				( armOpVAND, thumbOpVAND, dregTemp, dregTemp, dregSrc, false ) ;
			WriteSIMDIntNegRegReg
				( dregTemp, dregTemp, typeNEONInt32, false, fPair ) ;
			WriteSIMDDupRegRegImm
				( dregTemp, dregTemp, 0, typeNEONInt32, fPair ) ;
			WriteSIMDIntOpRegRegReg
				( armOpVSHL, thumbOpVSHL,
					dregDst, dregDst, dregTemp, typeNEONInt32, fPair, false ) ;
			break ;
		case	simdPsllw:
			WriteARMMoveRegImm( armTemp0, 0x0f ) ;
			WriteMoveARMtoVFP64( dregTemp, armTemp0, armTemp1 ) ;
			WriteSIMDOpRegRegReg
				( armOpVAND, thumbOpVAND, dregTemp, dregTemp, dregSrc, false ) ;
			WriteSIMDDupRegRegImm
				( dregTemp, dregTemp, 0, typeNEONInt16, fPair ) ;
			WriteSIMDIntOpRegRegReg
				( armOpVSHL, thumbOpVSHL,
					dregDst, dregDst, dregTemp, typeNEONInt16, fPair ) ;
			break ;
		case	simdPslld:
			WriteARMMoveRegImm( armTemp0, 0x1f ) ;
			WriteMoveARMtoVFP64( dregTemp, armTemp0, armTemp1 ) ;
			WriteSIMDOpRegRegReg
				( armOpVAND, thumbOpVAND, dregTemp, dregTemp, dregSrc, false ) ;
			WriteSIMDDupRegRegImm
				( dregTemp, dregTemp, 0, typeNEONInt32, fPair ) ;
			WriteSIMDIntOpRegRegReg
				( armOpVSHL, thumbOpVSHL,
					dregDst, dregDst, dregTemp, typeNEONInt32, fPair ) ;
			break ;

		case	simdPcmpnesb:
			WriteSIMDIntOpRegRegReg
				( armOpVICmpEQ, thumbOpVICmpEQ,
					dregDst, dregDst, dregSrc, typeNEONInt8, fPair ) ;
			WriteSIMDIntNotRegReg
				( dregDst, dregDst, typeNEONInt8, fPair ) ;
			break ;
		case	simdPcmpnesw:
			WriteSIMDIntOpRegRegReg
				( armOpVICmpEQ, thumbOpVICmpEQ,
					dregDst, dregDst, dregSrc, typeNEONInt16, fPair ) ;
			WriteSIMDIntNotRegReg
				( dregDst, dregDst, typeNEONInt16, fPair ) ;
			break ;
		case	simdPcmpnesd:
			WriteSIMDIntOpRegRegReg
				( armOpVICmpEQ, thumbOpVICmpEQ,
					dregDst, dregDst, dregSrc, typeNEONInt32, fPair ) ;
			WriteSIMDIntNotRegReg
				( dregDst, dregDst, typeNEONInt32, fPair ) ;
			break ;

		case	simdPcmpeqsb:
			WriteSIMDIntOpRegRegReg
				( armOpVICmpEQ, thumbOpVICmpEQ,
					dregDst, dregDst, dregSrc, typeNEONInt8, fPair ) ;
			break ;
		case	simdPcmpeqsw:
			WriteSIMDIntOpRegRegReg
				( armOpVICmpEQ, thumbOpVICmpEQ,
					dregDst, dregDst, dregSrc, typeNEONInt16, fPair ) ;
			break ;
		case	simdPcmpeqsd:
			WriteSIMDIntOpRegRegReg
				( armOpVICmpEQ, thumbOpVICmpEQ,
					dregDst, dregDst, dregSrc, typeNEONInt32, fPair ) ;
			break ;

		case	simdPcmpltsb:
			WriteSIMDIntOpRegRegReg
				( armOpVICmpGT, thumbOpVICmpGT,
					dregDst, dregSrc, dregDst, typeNEONInt8, fPair ) ;
			break ;
		case	simdPcmpltsw:
			WriteSIMDIntOpRegRegReg
				( armOpVICmpGT, thumbOpVICmpGT,
					dregDst, dregSrc, dregDst, typeNEONInt16, fPair ) ;
			break ;
		case	simdPcmpltsd:
			WriteSIMDIntOpRegRegReg
				( armOpVICmpGT, thumbOpVICmpGT,
					dregDst, dregSrc, dregDst, typeNEONInt32, fPair ) ;
			break ;

		case	simdPcmplesb:
			WriteSIMDIntOpRegRegReg
				( armOpVICmpGE, thumbOpVICmpGE,
					dregDst, dregSrc, dregDst, typeNEONInt8, fPair ) ;
			break ;
		case	simdPcmplesw:
			WriteSIMDIntOpRegRegReg
				( armOpVICmpGE, thumbOpVICmpGE,
					dregDst, dregSrc, dregDst, typeNEONInt16, fPair ) ;
			break ;
		case	simdPcmplesd:
			WriteSIMDIntOpRegRegReg
				( armOpVICmpGE, thumbOpVICmpGE,
					dregDst, dregSrc, dregDst, typeNEONInt32, fPair ) ;
			break ;

		case	simdPcmpgtsb:
			WriteSIMDIntOpRegRegReg
				( armOpVICmpGT, thumbOpVICmpGT,
					dregDst, dregDst, dregSrc, typeNEONInt8, fPair ) ;
			break ;
		case	simdPcmpgtsw:
			WriteSIMDIntOpRegRegReg
				( armOpVICmpGT, thumbOpVICmpGT,
					dregDst, dregDst, dregSrc, typeNEONInt16, fPair ) ;
			break ;
		case	simdPcmpgtsd:
			WriteSIMDIntOpRegRegReg
				( armOpVICmpGT, thumbOpVICmpGT,
					dregDst, dregDst, dregSrc, typeNEONInt32, fPair ) ;
			break ;

		case	simdPcmpgesb:
			WriteSIMDIntOpRegRegReg
				( armOpVICmpGT, thumbOpVICmpGE,
					dregDst, dregDst, dregSrc, typeNEONInt8, fPair ) ;
			break ;
		case	simdPcmpgesw:
			WriteSIMDIntOpRegRegReg
				( armOpVICmpGT, thumbOpVICmpGE,
					dregDst, dregDst, dregSrc, typeNEONInt16, fPair ) ;
			break ;
		case	simdPcmpgesd:
			WriteSIMDIntOpRegRegReg
				( armOpVICmpGT, thumbOpVICmpGE,
					dregDst, dregDst, dregSrc, typeNEONInt32, fPair ) ;
			break ;

		case	simdPmullw:
			WriteSIMDIntMulRegRegReg
				( dregDst, dregDst, dregSrc, typeNEONInt16, fPair ) ;
			break ;
		case	simdPmulhsw:
			regClassTemp = regClassNEON ;
			vregTemp = AllocateDataRegister( regClassTemp ) ;
			dregTemp = (vregTemp << 1) ;
			if ( fPair )
			{
				WriteSIMDIntMulLongRegRegReg
					( vregTemp, dregDst, dregSrc, typeNEONInt16, false ) ;
				WriteSIMDIntMulLongRegRegReg
					( vregDst, dregDst + 1, dregSrc + 1, typeNEONInt16, false ) ;
				WriteSIMDUnZipRegReg
					( dregTemp, dregTemp + 1, typeNEONInt16, false ) ;
				WriteSIMDUnZipRegReg
					( dregDst, dregDst + 1, typeNEONInt16, false ) ;
				WriteMoveVFP64( dregDst + 1, dregTemp + 1 ) ;
			}
			else
			{
				WriteSIMDIntMulLongRegRegReg
					( vregTemp, dregDst, dregSrc, typeNEONInt16, false ) ;
				WriteSIMDUnZipRegReg
					( dregTemp, dregTemp + 1, typeNEONInt16, false ) ;
				WriteMoveVFP64( dregDst, dregTemp + 1 ) ;
			}
			break ;
		case	simdPmulhuw:
			regClassTemp = regClassNEON ;
			vregTemp = AllocateDataRegister( regClassTemp ) ;
			dregTemp = (vregTemp << 1) ;
			if ( fPair )
			{
				WriteSIMDIntMulLongRegRegReg
					( vregTemp, dregDst, dregSrc, typeNEONInt16, true ) ;
				WriteSIMDIntMulLongRegRegReg
					( vregDst, dregDst + 1, dregSrc + 1, typeNEONInt16, true ) ;
				WriteSIMDUnZipRegReg
					( dregTemp, dregTemp + 1, typeNEONInt16, false ) ;
				WriteSIMDUnZipRegReg
					( dregDst, dregDst + 1, typeNEONInt16, false ) ;
				WriteMoveVFP64( dregDst + 1, dregTemp + 1 ) ;
			}
			else
			{
				WriteSIMDIntMulLongRegRegReg
					( vregTemp, dregDst, dregSrc, typeNEONInt16, true ) ;
				WriteSIMDUnZipRegReg
					( dregTemp, dregTemp + 1, typeNEONInt16, false ) ;
				WriteMoveVFP64( dregDst, dregTemp + 1 ) ;
			}
			break ;
		case	simdPmaddwd:
			regClassTemp = regClassNEON ;
			vregTemp = AllocateDataRegister( regClassTemp ) ;
			dregTemp = (vregTemp << 1) ;
			if ( fPair )
			{
				WriteSIMDIntMulLongRegRegReg
					( vregTemp, dregDst, dregSrc, typeNEONInt16, false ) ;
				WriteSIMDIntMulLongRegRegReg
					( vregDst, dregDst + 1, dregSrc + 1, typeNEONInt16, false ) ;
				WriteSIMDUnZipRegReg
					( dregTemp, dregTemp + 1, typeNEONInt32, false ) ;
				WriteSIMDUnZipRegReg
					( dregDst, dregDst + 1, typeNEONInt32, false ) ;
				WriteSIMDIntOpRegRegReg
					( armOpVIADD, thumbOpVIADD,
						dregDst + 1, dregDst, dregDst + 1, typeNEONInt32, false ) ;
				WriteSIMDIntOpRegRegReg
					( armOpVIADD, thumbOpVIADD,
						dregDst, dregTemp, dregTemp + 1, typeNEONInt32, false ) ;
			}
			else
			{
				WriteSIMDIntMulLongRegRegReg
					( vregTemp, dregDst, dregSrc, typeNEONInt16, false ) ;
				WriteSIMDUnZipRegReg
					( dregTemp, dregTemp + 1, typeNEONInt32, false ) ;
				WriteSIMDIntOpRegRegReg
					( armOpVIADD, thumbOpVIADD,
						dregDst, dregTemp, dregTemp + 1, typeNEONInt32, false ) ;
			}
			break ;

		case	simdPunpacklbw:
			vregTemp = AllocateDataRegister( regClassTemp ) ;
			dregTemp = (regClassTemp == regClassNEON) ? (vregTemp << 1) : vregTemp ;
			if ( fPair )
			{
				WriteMoveVFP128( vregTemp, vregSrc ) ;
			}
			else
			{
				WriteMoveVFP64( vregTemp, vregSrc ) ;
			}
			WriteSIMDZipRegReg( dregDst, dregTemp, typeNEONInt8, fPair ) ;
			break ;
		case	simdPunpacklwd:
			vregTemp = AllocateDataRegister( regClassTemp ) ;
			dregTemp = (regClassTemp == regClassNEON) ? (vregTemp << 1) : vregTemp ;
			if ( fPair )
			{
				WriteMoveVFP128( vregTemp, vregSrc ) ;
			}
			else
			{
				WriteMoveVFP64( vregTemp, vregSrc ) ;
			}
			WriteSIMDZipRegReg( dregDst, dregTemp, typeNEONInt16, fPair ) ;
			break ;
		case	simdPunpackldq:
			vregTemp = AllocateDataRegister( regClassTemp ) ;
			dregTemp = (regClassTemp == regClassNEON) ? (vregTemp << 1) : vregTemp ;
			if ( fPair )
			{
				WriteMoveVFP128( vregTemp, vregSrc ) ;
			}
			else
			{
				WriteMoveVFP64( vregTemp, vregSrc ) ;
			}
			WriteSIMDZipRegReg( dregDst, dregTemp, typeNEONInt32, fPair ) ;
			break ;

		case	simdPcvtswsb:
			vregTemp = AllocateDataRegister( regClassTemp ) ;
			dregTemp = (regClassTemp == regClassNEON) ? (vregTemp << 1) : vregTemp ;
			if ( fPair )
			{
				WriteMoveVFP128( vregTemp, vregSrc ) ;
			}
			else
			{
				WriteMoveVFP64( vregTemp, vregSrc ) ;
			}
			WriteSIMDQShiftRegRegImm
				( dregDst, dregDst, 8, typeNEONInt16, true, true, fPair ) ;
			WriteSIMDQShiftRegRegImm
				( dregTemp, dregTemp, 8, typeNEONInt16, true, true, fPair ) ;
			WriteSIMDUnZipRegReg
				( dregDst, dregTemp, typeNEONInt8, fPair ) ;
			if ( fPair )
			{
				WriteMoveVFP128( vregDst, vregTemp ) ;
			}
			else
			{
				WriteMoveVFP64( vregDst, vregTemp ) ;
			}
			break ;
		case	simdPcvtswub:
			vregTemp = AllocateDataRegister( regClassTemp ) ;
			dregTemp = (regClassTemp == regClassNEON) ? (vregTemp << 1) : vregTemp ;
			if ( fPair )
			{
				WriteMoveVFP128( vregTemp, vregSrc ) ;
			}
			else
			{
				WriteMoveVFP64( vregTemp, vregSrc ) ;
			}
			WriteSIMDQShiftRegRegImm
				( dregDst, dregDst, 8, typeNEONInt16, false, true, fPair ) ;
			WriteSIMDQShiftRegRegImm
				( dregTemp, dregTemp, 8, typeNEONInt16, false, true, fPair ) ;
			WriteSIMDUnZipRegReg
				( dregDst, dregTemp, typeNEONInt8, fPair ) ;
			if ( fPair )
			{
				WriteMoveVFP128( vregDst, vregTemp ) ;
			}
			else
			{
				WriteMoveVFP64( vregDst, vregTemp ) ;
			}
			break ;
		case	simdPcvtsdsw:
			vregTemp = AllocateDataRegister( regClassTemp ) ;
			dregTemp = (regClassTemp == regClassNEON) ? (vregTemp << 1) : vregTemp ;
			if ( fPair )
			{
				WriteMoveVFP128( vregTemp, vregSrc ) ;
			}
			else
			{
				WriteMoveVFP64( vregTemp, vregSrc ) ;
			}
			WriteSIMDQShiftRegRegImm
				( dregDst, dregDst, 16, typeNEONInt32, true, true, fPair ) ;
			WriteSIMDQShiftRegRegImm
				( dregTemp, dregTemp, 16, typeNEONInt32, true, true, fPair ) ;
			WriteSIMDUnZipRegReg
				( dregDst, dregTemp, typeNEONInt16, fPair ) ;
			if ( fPair )
			{
				WriteMoveVFP128( vregDst, vregTemp ) ;
			}
			else
			{
				WriteMoveVFP64( vregDst, vregTemp ) ;
			}
			break ;

		default:
			ESLTrace( "bad instruction packed 64bit SIMD %02X %02X\n",
						codeSIMD64Extension2Op, code ) ;
			WriteToAtomicOrExceptionMask( exceptionBadInstruction ) ;
			break ;
		}
		SetDataRegisterModified( regClassDst, vregDst ) ;
		UnlockDataRegister( regClassDst, vregDst ) ;
		UnlockDataRegister( regClassSrc, vregSrc ) ;
		if ( vregTemp != ARM_Nothing )
		{
			FreeDataRegister( regClassTemp, vregTemp ) ;
		}
		if ( armTemp0 != ARM_Nothing )
		{
			FreeDataRegister( regClassARM, armTemp0 ) ;
		}
		return ;
	}
	bool	fLoadDst = true, fLoadSrcAsTemp = false ;
	int		stepSrcReg = 1 ;
	switch ( code )
	{
	case	simdPsrlw:
	case	simdPsrld:
	case	simdPsraw:
	case	simdPsrad:
	case	simdPsllw:
	case	simdPslld:
		stepSrcReg = 0 ;
		break ;

	case	simdPcmpeqsb:
	case	simdPcmpeqsw:
		if ( regDst == regSrc )
		{
			break ;
		}
	case	simdPcmpnesb:
	case	simdPcmpnesw:
	case	simdPcmpltsb:
	case	simdPcmpltsw:
	case	simdPcmplesb:
	case	simdPcmplesw:
	case	simdPcmpgtsb:
	case	simdPcmpgtsw:
	case	simdPcmpgesb:
	case	simdPcmpgesw:
		Sakura2Assembler::write_simd64_extension( code, regDst, regSrc, fPair ) ;
		return ;

	case	simdPcvtswsb:
	case	simdPcvtswub:
	case	simdPcvtsdsw:
		fLoadSrcAsTemp = true ;
		break ;

	case	simdPmullw:
	case	simdPmulhsw:
	case	simdPmulhuw:
	case	simdPmaddwd:
	case	simdPunpacklbw:
	case	simdPunpacklwd:
		fLoadSrcAsTemp = true ;
		break ;
	}
	ARMRegister	armTemp0 = ARM_Nothing, armTemp1 = ARM_Nothing ;
	ARMRegister	armSrc0, armDst0 ;
	if ( fLoadSrcAsTemp )
	{
		armSrc0 = (ARMRegister) AllocateDataRegister( regClassARM ) ;
		WriteToLoadSakura2Register( armSrc0, regSrc ) ;
		armDst0 = (ARMRegister)
			WriteRealizeDataRegister( regDst, regClassARM, fLoadDst ) ;
	}
	else
	{
		armSrc0 = (ARMRegister)
			WriteRealizeDataRegister( regSrc, regClassARM ) ;
		armDst0 = (ARMRegister)
			WriteRealizeDataRegister( regDst, regClassARM, fLoadDst ) ;
	}
	ARMRegister	armSrc1 = (ARMRegister) (armSrc0 + 1) ;
	ARMRegister	armDst1 = (ARMRegister) (armDst0 + 1) ;
	//
	switch ( code )
	{
	case	simdPaddub:
		WriteSIMDOpARMRegRegReg
			( armOpUQADD8, thumbOpUQADD8, armDst0, armDst0, armSrc0 ) ;
		WriteSIMDOpARMRegRegReg
			( armOpUQADD8, thumbOpUQADD8, armDst1, armDst1, armSrc1 ) ;
		break ;
	case	simdPaddsb:
		WriteSIMDOpARMRegRegReg
			( armOpQADD8, thumbOpQADD8, armDst0, armDst0, armSrc0 ) ;
		WriteSIMDOpARMRegRegReg
			( armOpQADD8, thumbOpQADD8, armDst1, armDst1, armSrc1 ) ;
		break ;
	case	simdPaddb:
		WriteSIMDOpARMRegRegReg
			( armOpSADD8, thumbOpSADD8, armDst0, armDst0, armSrc0 ) ;
		WriteSIMDOpARMRegRegReg
			( armOpSADD8, thumbOpSADD8, armDst1, armDst1, armSrc1 ) ;
		break ;
	case	simdPadduw:
		WriteSIMDOpARMRegRegReg
			( armOpUQADD16, thumbOpUQADD16, armDst0, armDst0, armSrc0 ) ;
		WriteSIMDOpARMRegRegReg
			( armOpUQADD16, thumbOpUQADD16, armDst1, armDst1, armSrc1 ) ;
		break ;
	case	simdPaddsw:
		WriteSIMDOpARMRegRegReg
			( armOpQADD16, thumbOpQADD16, armDst0, armDst0, armSrc0 ) ;
		WriteSIMDOpARMRegRegReg
			( armOpQADD16, thumbOpQADD16, armDst1, armDst1, armSrc1 ) ;
		break ;
	case	simdPaddw:
		WriteSIMDOpARMRegRegReg
			( armOpSADD16, thumbOpSADD16, armDst0, armDst0, armSrc0 ) ;
		WriteSIMDOpARMRegRegReg
			( armOpSADD16, thumbOpSADD16, armDst1, armDst1, armSrc1 ) ;
		break ;
	case	simdPaddd:
		WriteARMAddRegRegRegShift( armDst0, armDst0, armSrc0 ) ;
		WriteARMAddRegRegRegShift( armDst1, armDst1, armSrc1 ) ;
		break ;

	case	simdPsubub:
		WriteSIMDOpARMRegRegReg
			( armOpUQSUB8, thumbOpUQSUB8, armDst0, armDst0, armSrc0 ) ;
		WriteSIMDOpARMRegRegReg
			( armOpUQSUB8, thumbOpUQSUB8, armDst1, armDst1, armSrc1 ) ;
		break ;
	case	simdPsubsb:
		WriteSIMDOpARMRegRegReg
			( armOpQSUB8, thumbOpQSUB8, armDst0, armDst0, armSrc0 ) ;
		WriteSIMDOpARMRegRegReg
			( armOpQSUB8, thumbOpQSUB8, armDst1, armDst1, armSrc1 ) ;
		break ;
	case	simdPsubb:
		WriteSIMDOpARMRegRegReg
			( armOpSSUB8, thumbOpSSUB8, armDst0, armDst0, armSrc0 ) ;
		WriteSIMDOpARMRegRegReg
			( armOpSSUB8, thumbOpSSUB8, armDst1, armDst1, armSrc1 ) ;
		break ;
	case	simdPsubuw:
		WriteSIMDOpARMRegRegReg
			( armOpUQSUB16, thumbOpUQSUB16, armDst0, armDst0, armSrc0 ) ;
		WriteSIMDOpARMRegRegReg
			( armOpUQSUB16, thumbOpUQSUB16, armDst1, armDst1, armSrc1 ) ;
		break ;
	case	simdPsubsw:
		WriteSIMDOpARMRegRegReg
			( armOpQSUB16, thumbOpQSUB16, armDst0, armDst0, armSrc0 ) ;
		WriteSIMDOpARMRegRegReg
			( armOpQSUB16, thumbOpQSUB16, armDst1, armDst1, armSrc1 ) ;
		break ;
	case	simdPsubw:
		WriteSIMDOpARMRegRegReg
			( armOpSSUB16, thumbOpSSUB16, armDst0, armDst0, armSrc0 ) ;
		WriteSIMDOpARMRegRegReg
			( armOpSSUB16, thumbOpSSUB16, armDst1, armDst1, armSrc1 ) ;
		break ;
	case	simdPsubd:
		WriteARMSubRegRegRegShift( armDst0, armDst0, armSrc0 ) ;
		WriteARMSubRegRegRegShift( armDst1, armDst1, armSrc1 ) ;
		break ;

	case	simdPsrlw:
		armTemp0 = (ARMRegister) AllocateDataRegister( regClassARM ) ;
		armTemp1 = (ARMRegister) (armTemp0 + 1) ;
		WriteARMMoveRegImm( armTemp1, 0xFFFF ) ;
		WriteARMAndRegRegImm8( armTemp0, armSrc0, 0x0f ) ;
		WriteARMOpRegRegReg( armOpLSR, thumbOpLSR, armTemp1, armTemp1, armTemp0 ) ;
		WriteARMOpRegRegReg( armOpLSR, thumbOpLSR, armDst0, armDst0, armTemp0 ) ;
		WriteARMOpRegRegReg( armOpLSR, thumbOpLSR, armDst1, armDst1, armTemp0 ) ;
		WriteARMOrRegRegRegShift( armTemp1, armTemp1, armTemp1, 16 ) ;
		WriteARMAndRegRegRegShift( armDst0, armDst0, armTemp1 ) ;
		WriteARMAndRegRegRegShift( armDst1, armDst1, armTemp1 ) ;
		break ;
	case	simdPsrld:
		armTemp0 = (ARMRegister) AllocateDataRegister( regClassARM ) ;
		WriteARMAndRegRegImm8( armTemp0, armSrc0, 0x1f ) ;
		WriteARMOpRegRegReg( armOpLSR, thumbOpLSR, armDst0, armDst0, armTemp0 ) ;
		WriteARMOpRegRegReg( armOpLSR, thumbOpLSR, armDst1, armDst1, armTemp0 ) ;
		break ;
	case	simdPsraw:
		armTemp0 = (ARMRegister) AllocateDataRegister( regClassARM ) ;
		armTemp1 = (ARMRegister) (armTemp0 + 1) ;
		WriteARMAndRegRegImm8( armTemp0, armSrc0, 0x0f ) ;
		WriteARMShiftLeftImm( armTemp1, armDst0, 16 ) ;
		WriteARMOpRegRegReg( armOpASR, thumbOpASR, armDst0, armDst0, armTemp0 ) ;
		WriteARMOpRegRegReg( armOpASR, thumbOpASR, armTemp1, armTemp1, armTemp0 ) ;
		WriteARMShiftRightImm( armDst0, armDst0, 16 ) ;
		WriteARMShiftRightImm( armTemp1, armTemp1, 16 ) ;
		WriteARMOrRegRegRegShift( armDst0, armTemp1, armDst0, 16 ) ;
		//
		WriteARMShiftLeftImm( armTemp1, armDst1, 16 ) ;
		WriteARMOpRegRegReg( armOpASR, thumbOpASR, armDst1, armDst1, armTemp0 ) ;
		WriteARMOpRegRegReg( armOpASR, thumbOpASR, armTemp1, armTemp1, armTemp0 ) ;
		WriteARMShiftRightImm( armDst1, armDst1, 16 ) ;
		WriteARMShiftRightImm( armTemp1, armTemp1, 16 ) ;
		WriteARMOrRegRegRegShift( armDst1, armTemp1, armDst1, 16 ) ;
		break ;
	case	simdPsrad:
		armTemp0 = (ARMRegister) AllocateDataRegister( regClassARM ) ;
		WriteARMAndRegRegImm8( armTemp0, armSrc0, 0x1f ) ;
		WriteARMOpRegRegReg( armOpASR, thumbOpASR, armDst0, armDst0, armTemp0 ) ;
		WriteARMOpRegRegReg( armOpASR, thumbOpASR, armDst1, armDst1, armTemp0 ) ;
		break ;
	case	simdPsllw:
		armTemp0 = (ARMRegister) AllocateDataRegister( regClassARM ) ;
		armTemp1 = (ARMRegister) (armTemp0 + 1) ;
		WriteARMMoveRegImm( armTemp1, 0xFFFF0000 ) ;
		WriteARMAndRegRegImm8( armTemp0, armSrc0, 0x0f ) ;
		WriteARMOpRegRegReg( armOpLSL, thumbOpLSL, armTemp1, armTemp1, armTemp0 ) ;
		WriteARMOpRegRegReg( armOpLSL, thumbOpLSL, armDst0, armDst0, armTemp0 ) ;
		WriteARMOpRegRegReg( armOpLSL, thumbOpLSL, armDst1, armDst1, armTemp0 ) ;
		WriteARMOrRegRegRegShift( armTemp1, armTemp1, armTemp1, -16 ) ;
		WriteARMAndRegRegRegShift( armDst0, armDst0, armTemp1 ) ;
		WriteARMAndRegRegRegShift( armDst1, armDst1, armTemp1 ) ;
		break ;
	case	simdPslld:
		armTemp0 = (ARMRegister) AllocateDataRegister( regClassARM ) ;
		WriteARMAndRegRegImm8( armTemp0, armSrc0, 0x1f ) ;
		WriteARMOpRegRegReg( armOpLSL, thumbOpLSL, armDst0, armDst0, armTemp0 ) ;
		WriteARMOpRegRegReg( armOpLSL, thumbOpLSL, armDst1, armDst1, armTemp0 ) ;
		break ;

	case	simdPcmpeqsb:
	case	simdPcmpeqsw:
		ESLAssert( regDst == regSrc ) ;
		WriteARMMoveRegImm( armDst0, 0xFFFFFFFF ) ;
		WriteARMMoveRegReg( armDst1, armDst0 ) ;
		break ;

	case	simdPcmpnesd:
		write_arm_cmp_int32( armDst0, armSrc0, cond_NE ) ;
		write_arm_cmp_int32( armDst1, armSrc1, cond_NE ) ;
		break ;

	case	simdPcmpeqsd:
		write_arm_cmp_int32( armDst0, armSrc0, cond_EQ ) ;
		write_arm_cmp_int32( armDst1, armSrc1, cond_EQ ) ;
		break ;

	case	simdPcmpltsd:
		write_arm_cmp_int32( armDst0, armSrc0, cond_LT ) ;
		write_arm_cmp_int32( armDst1, armSrc1, cond_LT ) ;
		break ;

	case	simdPcmplesd:
		write_arm_cmp_int32( armDst0, armSrc0, cond_LE ) ;
		write_arm_cmp_int32( armDst1, armSrc1, cond_LE ) ;
		break ;

	case	simdPcmpgtsd:
		write_arm_cmp_int32( armDst0, armSrc0, cond_GT ) ;
		write_arm_cmp_int32( armDst1, armSrc1, cond_GT ) ;
		break ;

	case	simdPcmpgesd:
		write_arm_cmp_int32( armDst0, armSrc0, cond_GE ) ;
		write_arm_cmp_int32( armDst1, armSrc1, cond_GE ) ;
		break ;

	case	simdPcvtswsb:
		WriteARMSat16RegImmReg( armDst0, 7, armDst0, false ) ;
		WriteARMSat16RegImmReg( armDst1, 7, armDst1, false ) ;
		WriteARMSat16RegImmReg( armSrc0, 7, armSrc0, false ) ;
		WriteARMSat16RegImmReg( armSrc1, 7, armSrc1, false ) ;
		WriteARMUXTB16RegRegImm( armDst0, armDst0, 0 ) ;
		WriteARMUXTB16RegRegImm( armDst1, armDst1, 0 ) ;
		WriteARMUXTB16RegRegImm( armSrc0, armSrc0, 0 ) ;
		WriteARMUXTB16RegRegImm( armSrc1, armSrc1, 0 ) ;
		WriteARMOrRegRegRegShift( armDst0, armDst0, armDst0, -8 ) ;
		WriteARMOrRegRegRegShift( armDst1, armDst1, armDst1, -8 ) ;
		WriteARMOrRegRegRegShift( armSrc0, armSrc0, armSrc0, -8 ) ;
		WriteARMOrRegRegRegShift( armSrc1, armSrc1, armSrc1, -8 ) ;
		WriteARMBFIRegRegImmImm( armDst0, armDst1, 16, 16 ) ;
		WriteARMBFIRegRegImmImm( armSrc0, armSrc1, 16, 16 ) ;
		WriteARMMoveRegReg( armDst1, armSrc0 ) ;
		break ;

	case	simdPcvtswub:
		WriteARMSat16RegImmReg( armDst0, 8, armDst0, true ) ;
		WriteARMSat16RegImmReg( armDst1, 8, armDst1, true ) ;
		WriteARMSat16RegImmReg( armSrc0, 8, armSrc0, true ) ;
		WriteARMSat16RegImmReg( armSrc1, 8, armSrc1, true ) ;
		WriteARMOrRegRegRegShift( armDst0, armDst0, armDst0, -8 ) ;
		WriteARMOrRegRegRegShift( armDst1, armDst1, armDst1, -8 ) ;
		WriteARMOrRegRegRegShift( armSrc0, armSrc0, armSrc0, -8 ) ;
		WriteARMOrRegRegRegShift( armSrc1, armSrc1, armSrc1, -8 ) ;
		WriteARMBFIRegRegImmImm( armDst0, armDst1, 16, 16 ) ;
		WriteARMBFIRegRegImmImm( armSrc0, armSrc1, 16, 16 ) ;
		WriteARMMoveRegReg( armDst1, armSrc0 ) ;
		break ;

	case	simdPcvtsdsw:
		WriteARMSatRegImmRegShift( armDst0, 15, armDst0, 0, false ) ;
		WriteARMSatRegImmRegShift( armDst1, 15, armDst1, 0, false ) ;
		WriteARMSatRegImmRegShift( armSrc0, 15, armSrc0, 0, false ) ;
		WriteARMSatRegImmRegShift( armSrc1, 15, armSrc1, 0, false ) ;
		WriteARMBFIRegRegImmImm( armDst0, armDst1, 16, 16 ) ;
		WriteARMBFIRegRegImmImm( armSrc0, armSrc1, 16, 16 ) ;
		WriteARMMoveRegReg( armDst1, armSrc0 ) ;
		break ;

	case	simdPmullw:
		armTemp0 = (ARMRegister) AllocateDataRegister( regClassARM ) ;
		armTemp1 = (ARMRegister) (armTemp0 + 1) ;
		WriteARMShiftRightImm( armTemp0, armDst0, 16 ) ;
		WriteARMShiftRightImm( armTemp1, armSrc0, 16 ) ;
		WriteARMMulInt32( armTemp0, armTemp0, armTemp1 ) ;
		WriteARMMoveRegImm( armTemp1, 0xFFFF ) ;
		WriteARMAndRegRegRegShift( armDst0, armDst0, armTemp1 ) ;
		WriteARMAndRegRegRegShift( armSrc0, armSrc0, armTemp1 ) ;
		WriteARMMulInt32( armDst0, armDst0, armSrc0 ) ;
		WriteARMAndRegRegRegShift( armDst0, armDst0, armTemp1 ) ;
		WriteARMOrRegRegRegShift( armDst0, armDst0, armTemp0, 16 ) ;
		//
		WriteARMShiftRightImm( armTemp0, armDst1, 16 ) ;
		WriteARMShiftRightImm( armTemp1, armSrc1, 16 ) ;
		WriteARMMulInt32( armTemp0, armTemp0, armTemp1 ) ;
		WriteARMMoveRegImm( armTemp1, 0xFFFF ) ;
		WriteARMAndRegRegRegShift( armDst1, armDst1, armTemp1 ) ;
		WriteARMAndRegRegRegShift( armSrc1, armSrc1, armTemp1 ) ;
		WriteARMMulInt32( armDst1, armDst1, armSrc1 ) ;
		WriteARMAndRegRegRegShift( armDst1, armDst1, armTemp1 ) ;
		WriteARMOrRegRegRegShift( armDst1, armDst1, armTemp0, 16 ) ;
		break ;
	case	simdPmulhsw:
		armTemp0 = (ARMRegister) AllocateDataRegister( regClassARM ) ;
		armTemp1 = (ARMRegister) (armTemp0 + 1) ;
		WriteARMShiftARightImm( armTemp0, armDst0, 16 ) ;
		WriteARMShiftARightImm( armTemp1, armSrc0, 16 ) ;
		WriteARMMulInt32( armTemp0, armTemp0, armTemp1 ) ;
		WriteARMShiftLeftImm( armDst0, armDst0, 16 ) ;
		WriteARMShiftLeftImm( armSrc0, armSrc0, 16 ) ;
		WriteARMShiftARightImm( armDst0, armDst0, 16 ) ;
		WriteARMShiftARightImm( armSrc0, armSrc0, 16 ) ;
		WriteARMMulInt32( armDst0, armDst0, armSrc0 ) ;
		WriteARMMoveRegImm( armTemp1, 0xFFFF0000 ) ;
		WriteARMAndRegRegRegShift( armTemp0, armTemp0, armTemp1 ) ;
		WriteARMOrRegRegRegShift( armDst0, armTemp0, armDst0, -16 ) ;
		//
		WriteARMShiftARightImm( armTemp0, armDst1, 16 ) ;
		WriteARMShiftARightImm( armTemp1, armSrc1, 16 ) ;
		WriteARMMulInt32( armTemp0, armTemp0, armTemp1 ) ;
		WriteARMShiftLeftImm( armDst1, armDst1, 16 ) ;
		WriteARMShiftLeftImm( armSrc1, armSrc1, 16 ) ;
		WriteARMShiftARightImm( armDst1, armDst1, 16 ) ;
		WriteARMShiftARightImm( armSrc1, armSrc1, 16 ) ;
		WriteARMMulInt32( armDst1, armDst1, armSrc1 ) ;
		WriteARMMoveRegImm( armTemp1, 0xFFFF0000 ) ;
		WriteARMAndRegRegRegShift( armTemp0, armTemp0, armTemp1 ) ;
		WriteARMOrRegRegRegShift( armDst1, armTemp0, armDst1, -16 ) ;
		break ;
	case	simdPmulhuw:
		armTemp0 = (ARMRegister) AllocateDataRegister( regClassARM ) ;
		armTemp1 = (ARMRegister) (armTemp0 + 1) ;
		WriteARMShiftRightImm( armTemp0, armDst0, 16 ) ;
		WriteARMShiftRightImm( armTemp1, armSrc0, 16 ) ;
		WriteARMMulInt32( armTemp0, armTemp0, armTemp1 ) ;
		WriteARMMoveRegImm( armTemp1, 0xFFFF ) ;
		WriteARMAndRegRegRegShift( armDst0, armDst0, armTemp1 ) ;
		WriteARMAndRegRegRegShift( armSrc0, armSrc0, armTemp1 ) ;
		WriteARMMulInt32( armDst0, armDst0, armSrc0 ) ;
		WriteARMMoveRegImm( armTemp1, 0xFFFF0000 ) ;
		WriteARMAndRegRegRegShift( armTemp0, armTemp0, armTemp1 ) ;
		WriteARMOrRegRegRegShift( armDst0, armTemp0, armDst0, -16 ) ;
		//
		WriteARMShiftRightImm( armTemp0, armDst1, 16 ) ;
		WriteARMShiftRightImm( armTemp1, armSrc1, 16 ) ;
		WriteARMMulInt32( armTemp0, armTemp0, armTemp1 ) ;
		WriteARMMoveRegImm( armTemp1, 0xFFFF ) ;
		WriteARMAndRegRegRegShift( armDst1, armDst1, armTemp1 ) ;
		WriteARMAndRegRegRegShift( armSrc1, armSrc1, armTemp1 ) ;
		WriteARMMulInt32( armDst1, armDst1, armSrc1 ) ;
		WriteARMMoveRegImm( armTemp1, 0xFFFF0000 ) ;
		WriteARMAndRegRegRegShift( armTemp0, armTemp0, armTemp1 ) ;
		WriteARMOrRegRegRegShift( armDst1, armTemp0, armDst1, -16 ) ;
		break ;
	case	simdPmaddwd:
		armTemp0 = (ARMRegister) AllocateDataRegister( regClassARM ) ;
		armTemp1 = (ARMRegister) (armTemp0 + 1) ;
		WriteARMShiftARightImm( armTemp0, armDst0, 16 ) ;
		WriteARMShiftARightImm( armTemp1, armSrc0, 16 ) ;
		WriteARMMulInt32( armTemp0, armTemp0, armTemp1 ) ;
		WriteARMShiftLeftImm( armDst0, armDst0, 16 ) ;
		WriteARMShiftLeftImm( armSrc0, armSrc0, 16 ) ;
		WriteARMShiftARightImm( armDst0, armDst0, 16 ) ;
		WriteARMShiftARightImm( armSrc0, armSrc0, 16 ) ;
		WriteARMMulInt32( armDst0, armDst0, armSrc0 ) ;
		WriteARMAddRegRegRegShift( armDst0, armDst0, armTemp0 ) ;
		//
		WriteARMShiftARightImm( armTemp0, armDst1, 16 ) ;
		WriteARMShiftARightImm( armTemp1, armSrc1, 16 ) ;
		WriteARMMulInt32( armTemp0, armTemp0, armTemp1 ) ;
		WriteARMShiftLeftImm( armDst1, armDst1, 16 ) ;
		WriteARMShiftLeftImm( armSrc1, armSrc1, 16 ) ;
		WriteARMShiftARightImm( armDst1, armDst1, 16 ) ;
		WriteARMShiftARightImm( armSrc1, armSrc1, 16 ) ;
		WriteARMMulInt32( armDst1, armDst1, armSrc1 ) ;
		WriteARMAddRegRegRegShift( armDst1, armDst1, armTemp0 ) ;
		break ;

	case	simdPunpacklbw:
		armTemp0 = (ARMRegister) AllocateDataRegister( regClassARM ) ;
		armTemp1 = (ARMRegister) (armTemp0 + 1) ;
		WriteARMShiftRightImm( armDst1, armDst0, 16 ) ;
		WriteARMShiftRightImm( armSrc1, armSrc0, 16 ) ;
		WriteARMMoveRegImm( armTemp1, 0xFF00 ) ;
		WriteARMAndRegRegRegShift( armTemp0, armTemp1, armDst1 ) ;
		WriteARMAndRegRegRegShift( armTemp1, armTemp1, armSrc1 ) ;
		WriteARMAndRegRegImm8( armDst1, armDst1, 0xFF ) ;
		WriteARMAndRegRegImm8( armSrc1, armSrc1, 0xFF ) ;
		WriteARMOrRegRegRegShift( armTemp0, armTemp0, armTemp1, 8 ) ;
		WriteARMOrRegRegRegShift( armDst1, armDst1, armSrc1, 8 ) ;
		WriteARMOrRegRegRegShift( armDst1, armDst1, armTemp0, 8 ) ;
		//
		WriteARMMoveRegImm( armTemp1, 0xFF00 ) ;
		WriteARMAndRegRegRegShift( armTemp0, armTemp1, armDst0 ) ;
		WriteARMAndRegRegRegShift( armTemp1, armTemp1, armSrc0 ) ;
		WriteARMAndRegRegImm8( armDst0, armDst0, 0xFF ) ;
		WriteARMAndRegRegImm8( armSrc0, armSrc0, 0xFF ) ;
		WriteARMOrRegRegRegShift( armTemp0, armTemp0, armTemp1, 8 ) ;
		WriteARMOrRegRegRegShift( armDst0, armDst0, armSrc0, 8 ) ;
		WriteARMOrRegRegRegShift( armDst0, armDst0, armTemp0, 8 ) ;
		break ;
	case	simdPunpacklwd:
		WriteARMShiftRightImm( armDst1, armDst0, 16 ) ;
		WriteARMShiftRightImm( armSrc1, armSrc0, 16 ) ;
		WriteARMOrRegRegRegShift( armDst1, armDst1, armSrc1, 16 ) ;
		//
		WriteARMShiftLeftImm( armDst0, armDst0, 16 ) ;
		WriteARMShiftLeftImm( armSrc0, armSrc0, 16 ) ;
		WriteARMOrRegRegRegShift( armDst0, armSrc0, armDst0, -16 ) ;
		break ;
	case	simdPunpackldq:
		WriteARMMoveRegReg( armDst1, armSrc0 ) ;
		break ;

	default:
		ESLTrace( "bad instruction packed 64bit SIMD %02X %02X\n",
					codeSIMD64Extension2Op, code ) ;
		WriteToAtomicOrExceptionMask( exceptionBadInstruction ) ;
		break ;
	}
	SetDataRegisterModified( regClassARM, armDst0 ) ;
	UnlockDataRegister( regClassARM, armDst0 ) ;
	if ( fLoadSrcAsTemp )
	{
		FreeDataRegister( regClassARM, armSrc0 ) ;
	}
	else
	{
		UnlockDataRegister( regClassARM, armSrc0 ) ;
	}
	if ( armTemp0 != ARM_Nothing )
	{
		FreeDataRegister( regClassARM, armTemp0 ) ;
	}
	if ( fPair )
	{
		write_simd64_extension( code, regDst + 1, regSrc + stepSrcReg, false ) ;
	}
}

void ARMGenericAssembler::write_simd64_imm_extension( int code, int regDst, int regSrc, int imm8, bool fPair )
{
	bool	fLoadSrcAsTemp = false ;
	bool	vfpNEON = m_vfpNEON ;
	switch ( code )
	{
	case	simdPshufwImm8:
		fLoadSrcAsTemp = true ;
		if ( vfpNEON )
		{
			vfpNEON = (imm8 == 0x00) || (imm8 == 0x55)
					|| (imm8 == 0xAA) || (imm8 == 0xFF)
					|| (imm8 == 0x1B) || (imm8 == 0x4E)
					|| (imm8 == 0x39) || (imm8 == 0x93)
					|| (imm8 == 0xD8) || (imm8 == 0x72) || (imm8 == 0xE4) ;
		}
		break ;
	}
	if ( vfpNEON )
	{
		DataRegisterClass	regClass = fPair ? regClassNEON : regClassVFP ;
		int	vregDst, vregSrc, dregDst, dregSrc ;
		vregSrc = WriteRealizeDataRegister( regSrc, regClass ) ;
		vregDst = WriteRealizeDataRegister( regDst, regClass, false ) ;
		dregSrc = vregSrc ;
		dregDst = vregDst ;
		if ( regClass == regClassNEON )
		{
			dregSrc = (vregSrc << 1) ;
			dregDst = (vregDst << 1) ;
		}
		switch ( code )
		{
		case	simdPsrlwImm8:
			WriteSIMDShiftRegRegImm
				( armOpVSHRImm, thumbOpVSHRImm,
					dregDst, dregSrc, (imm8 & 0x0F),
					typeNEONInt16, false, true, fPair ) ;
			break ;
		case	simdPsrldImm8:
			WriteSIMDShiftRegRegImm
				( armOpVSHRImm, thumbOpVSHRImm,
					dregDst, dregSrc, (imm8 & 0x1F),
					typeNEONInt32, false, true, fPair ) ;
			break ;
		case	simdPsrawImm8:
			WriteSIMDShiftRegRegImm
				( armOpVSHRImm, thumbOpVSHRImm,
					dregDst, dregSrc, (imm8 & 0x0F),
					typeNEONInt16, true, true, fPair ) ;
			break ;
		case	simdPsradImm8:
			WriteSIMDShiftRegRegImm
				( armOpVSHRImm, thumbOpVSHRImm,
					dregDst, dregSrc, (imm8 & 0x1F),
					typeNEONInt32, true, true, fPair ) ;
			break ;
		case	simdPsllwImm8:
			WriteSIMDShiftRegRegImm
				( armOpVSHLImm, thumbOpVSHLImm,
					dregDst, dregSrc, (imm8 & 0x0F),
					typeNEONInt16, false, false, fPair ) ;
			break ;
		case	simdPslldImm8:
			WriteSIMDShiftRegRegImm
				( armOpVSHLImm, thumbOpVSHLImm,
					dregDst, dregSrc, (imm8 & 0x1F),
					typeNEONInt32, false, false, fPair ) ;
			break ;

		case	simdPshufwImm8:
			if ( (imm8 == 0x00) || (imm8 == 0x55)
				|| (imm8 == 0xAA) || (imm8 == 0xFF) )
			{
				WriteSIMDDupRegRegImm
					( dregDst, dregSrc, (imm8 & 0x03), typeNEONInt16, false ) ;
				if ( fPair )
					WriteSIMDDupRegRegImm
						( dregDst+1, dregSrc+1, (imm8 & 0x03), typeNEONInt16, false ) ;
			}
			else if ( imm8 == 0x1B )
			{
				WriteSIMDRevRegReg
					( dregDst, dregSrc, typeNEONInt64, typeNEONInt16, fPair ) ;
			}
			else if ( imm8 == 0x4E )
			{
				WriteSIMDRevRegReg
					( dregDst, dregSrc, typeNEONInt64, typeNEONInt32, fPair ) ;
			}
			else if ( imm8 == 0x39 )
			{
				WriteSIMDExtRegRegRegImm
					( dregDst, dregSrc, dregSrc, 2, false ) ;
				if ( fPair )
					WriteSIMDExtRegRegRegImm
						( dregDst+1, dregSrc+1, dregSrc+1, 2, false ) ;
			}
			else if ( imm8 == 0x93 )
			{
				WriteSIMDExtRegRegRegImm
					( dregDst, dregSrc, dregSrc, 6, false ) ;
				if ( fPair )
					WriteSIMDExtRegRegRegImm
						( dregDst+1, dregSrc+1, dregSrc+1, 6, false ) ;
			}
			else
			{
				if ( fPair )
				{
					WriteMoveVFP128( vregDst, vregSrc ) ;
				}
				else
				{
					WriteMoveVFP64( vregDst, vregSrc ) ;
				}
				if ( imm8 == 0xD8 )
				{
					int	vregTemp = AllocateDataRegister( regClass ) ;
					int	dregTemp = (regClass == regClassNEON)
											? (vregTemp << 1) : vregTemp ;
					WriteSIMDShiftRegRegImm
						( armOpVSHRImm, thumbOpVSHRImm,
							dregTemp, dregSrc, 32,
							typeNEONInt64, false, true, fPair ) ;
					WriteSIMDZipRegReg( dregDst, dregTemp, typeNEONInt16, false ) ;
					if ( fPair )
					{
						WriteSIMDZipRegReg( dregDst+1, dregTemp+1, typeNEONInt16, false ) ;
					}
					FreeDataRegister( regClass, vregTemp ) ;
				}
				else if ( imm8 == 0x72 )
				{
					int	vregTemp = AllocateDataRegister( regClass ) ;
					int	dregTemp = (regClass == regClassNEON)
											? (vregTemp << 1) : vregTemp ;
					WriteSIMDShiftRegRegImm
						( armOpVSHLImm, thumbOpVSHLImm,
							dregTemp, dregSrc, 32,
							typeNEONInt64, false, false, fPair ) ;
					WriteSIMDZipRegReg( dregDst, dregTemp, typeNEONInt16, false ) ;
					if ( fPair )
					{
						WriteSIMDZipRegReg( dregDst+1, dregTemp+1, typeNEONInt16, false ) ;
					}
					if ( fPair )
					{
						WriteMoveVFP128( vregDst, vregTemp ) ;
					}
					else
					{
						WriteMoveVFP64( vregDst, vregTemp ) ;
					}
					FreeDataRegister( regClass, vregTemp ) ;
				}
				else
				{
					ESLAssert( imm8 == 0xE4 ) ;
				}
			}
			break ;

		default:
			ESLTrace( "bad instruction packed 64bit SIMD %02X %02X\n",
						codeSIMD64Extension3Op, code ) ;
			WriteToAtomicOrExceptionMask( exceptionBadInstruction ) ;
			break ;
		}
		SetDataRegisterModified( regClass, vregDst ) ;
		UnlockDataRegister( regClass, vregDst ) ;
		UnlockDataRegister( regClass, vregSrc ) ;
	}
	else
	{
		ARMRegister	armTemp0 = ARM_Nothing, armTemp1 = ARM_Nothing ;
		ARMRegister	armSrc0, armDst0 ;
		if ( fLoadSrcAsTemp )
		{
			armSrc0 = (ARMRegister) AllocateDataRegister( regClassARM ) ;
			WriteToLoadSakura2Register( armSrc0, regSrc ) ;
			armDst0 = (ARMRegister)
				WriteRealizeDataRegister( regDst, regClassARM, false ) ;
		}
		else
		{
			armSrc0 = (ARMRegister)
				WriteRealizeDataRegister( regSrc, regClassARM ) ;
			armDst0 = (ARMRegister)
				WriteRealizeDataRegister( regDst, regClassARM, false ) ;
		}
		ARMRegister	armSrc1 = (ARMRegister) (armSrc0 + 1) ;
		ARMRegister	armDst1 = (ARMRegister) (armDst0 + 1) ;
		//
		uint32_t	nTemp ;
		int			i ;
		switch ( code )
		{
		case	simdPsrlwImm8:
			nTemp = 0xFFFF >> (imm8 & 0x0F) ;
			WriteARMShiftRightImm( armDst0, armSrc0, (imm8 & 0x0F) ) ;
			WriteARMShiftRightImm( armDst1, armSrc1, (imm8 & 0x0F) ) ;
			PreserveContinuousCodes( 0x20 ) ;
			WriteARMMoveRegImm( ARM_r6, nTemp | (nTemp << 16) ) ;
			WriteARMAndRegRegRegShift( armDst0, armDst0, ARM_r6 ) ;
			WriteARMAndRegRegRegShift( armDst1, armDst1, ARM_r6 ) ;
			break ;
		case	simdPsrldImm8:
			WriteARMShiftRightImm( armDst0, armSrc0, (imm8 & 0x1F) ) ;
			WriteARMShiftRightImm( armDst1, armSrc1, (imm8 & 0x1F) ) ;
			break ;
		case	simdPsrawImm8:
			armTemp0 = (ARMRegister) AllocateDataRegister( regClassARM ) ;
			armTemp1 = (ARMRegister) (armTemp0 + 1) ;
			WriteARMShiftLeftImm( armTemp0, armSrc0, 16 ) ;
			WriteARMShiftLeftImm( armTemp1, armSrc1, 16 ) ;
			WriteARMShiftARightImm( armDst0, armSrc0, (imm8 & 0x0F) + 16 ) ;
			WriteARMShiftARightImm( armDst1, armSrc1, (imm8 & 0x0F) + 16 ) ;
			WriteARMShiftARightImm( armTemp0, armTemp0, (imm8 & 0x0F) ) ;
			WriteARMShiftARightImm( armTemp1, armTemp1, (imm8 & 0x0F) ) ;
			WriteARMShiftLeftImm( armDst0, armDst0, 16 ) ;
			WriteARMShiftLeftImm( armDst1, armDst1, 16 ) ;
			WriteARMOrRegRegRegShift( armDst0, armDst0, armTemp0, -16 ) ;
			WriteARMOrRegRegRegShift( armDst1, armDst1, armTemp1, -16 ) ;
			break ;
		case	simdPsradImm8:
			WriteARMShiftARightImm( armDst0, armSrc0, (imm8 & 0x1F) ) ;
			WriteARMShiftARightImm( armDst1, armSrc1, (imm8 & 0x1F) ) ;
			break ;
		case	simdPsllwImm8:
			nTemp = (0xFFFF << (imm8 & 0x0F)) & 0xFFFF ;
			WriteARMShiftLeftImm( armDst0, armSrc0, (imm8 & 0x0F) ) ;
			WriteARMShiftLeftImm( armDst1, armSrc1, (imm8 & 0x0F) ) ;
			PreserveContinuousCodes( 0x20 ) ;
			WriteARMMoveRegImm( ARM_r6, nTemp | (nTemp << 16) ) ;
			WriteARMAndRegRegRegShift( armDst0, armDst0, ARM_r6 ) ;
			WriteARMAndRegRegRegShift( armDst1, armDst1, ARM_r6 ) ;
			break ;
		case	simdPslldImm8:
			WriteARMShiftLeftImm( armDst0, armSrc0, (imm8 & 0x1F) ) ;
			WriteARMShiftLeftImm( armDst1, armSrc1, (imm8 & 0x1F) ) ;
			break ;

		case	simdPshufwImm8:
			nTemp = imm8 ;
			if ( nTemp & 0x44 )
			{
				armTemp0 = (ARMRegister) AllocateDataRegister( regClassARM ) ;
				armTemp1 = (ARMRegister) (armTemp0 + 1) ;
			}
			for ( i = 0; i < 2; i ++ )
			{
				ARMRegister	armDstCur = (ARMRegister) (armDst0 + i) ;
				ARMRegister	armSrcLow = (nTemp & 0x02) ? armSrc1 : armSrc0 ;
				if ( nTemp & 0x01 )
				{
					WriteARMShiftRightImm( armDstCur, armSrcLow, -16 ) ;
				}
				else
				{
					WriteARMShiftLeftImm( armDstCur, armSrcLow, 16 ) ;
					WriteARMShiftRightImm( armDstCur, armDstCur, 16 ) ;
				}
				ARMRegister	armSrcHigh = (nTemp & 0x08) ? armSrc1 : armSrc0 ;
				if ( nTemp & 0x04 )
				{
					ESLAssert( armTemp0 != ARM_Nothing ) ;
					WriteARMShiftRightImm( armTemp0, armSrcHigh, 16 ) ;
					WriteARMOrRegRegRegShift( armDstCur, armDstCur, armTemp0, 16 ) ;
				}
				else
				{
					WriteARMOrRegRegRegShift( armDstCur, armDstCur, armSrcHigh, 16 ) ;
				}
				nTemp >>= 4 ;
			}
			break ;

		default:
			ESLTrace( "bad instruction packed 64bit SIMD %02X %02X\n",
						codeSIMD64Extension3Op, code ) ;
			WriteToAtomicOrExceptionMask( exceptionBadInstruction ) ;
			break ;
		}
		SetDataRegisterModified( regClassARM, armDst0 ) ;
		UnlockDataRegister( regClassARM, armDst0 ) ;
		if ( fLoadSrcAsTemp )
		{
			FreeDataRegister( regClassARM, armSrc0 ) ;
		}
		else
		{
			UnlockDataRegister( regClassARM, armSrc0 ) ;
		}
		if ( armTemp0 != ARM_Nothing )
		{
			FreeDataRegister( regClassARM, armTemp0 ) ;
		}
		if ( fPair )
		{
			write_simd64_imm_extension( code, regDst + 1, regSrc + 1, imm8, false ) ;
		}
	}
}

// 128bit SIMD
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::write_simd128_extension( int code, int regDst, int regSrc )
{
	if ( m_vfpVersion < 2 )
	{
		Sakura2Assembler::write_simd128_extension( code, regDst, regSrc ) ;
		return ;
	}
	bool	fLoadDstValue = true ;
	switch ( code )
	{
	case	simdVmove:
	case	simdFsqrt:
	case	simdFrcp:
	case	simdFrsqrt:
	case	simdFabs:
	case	simdVsqrt:
	case	simdVrcp:
	case	simdVrsqrt:
	case	simdVabs:
	case	simdDcvtd2f:
	case	simdDcvtf2d:
	case	simdDcvtf2i:
	case	simdVcvtf2w:
	case	simdVcvtf2i:
	case	simdDcvti2f:
	case	simdVcvtw2f:
	case	simdVcvti2f:
		fLoadDstValue = false ;
		break ;
	}
	if ( m_vfpNEON )
	{
		int	vregSrc = WriteRealizeDataRegister( regSrc, regClassNEON ) ;
		int	vregDst = WriteRealizeDataRegister( regDst, regClassNEON, fLoadDstValue ) ;
		int	dregSrc = (vregSrc << 1) ;
		int	dregDst = (vregDst << 1) ;
		int	vregTemp0 = -1 ;
		int	vregTemp1 = -1 ;
		DataRegisterClass	regClassTemp = regClassVFP ;
		ARMRegister	armTemp0 = ARM_Nothing ;
		ARMRegister	armTemp1 = ARM_Nothing ;
		float32_t	f32Half = 0.5f ;
		//
		switch ( code )
		{
		case	simdFadd:
			vregTemp0 = AllocateDataRegister( regClassVFP ) ;
			WriteSIMDRevRegReg
				( vregTemp0, dregDst, typeNEONInt64, typeNEONInt32, false ) ;
			WriteSIMDOpRegRegReg
				( armOpVFADD, thumbOpVFADD, dregDst, dregDst, dregSrc, false ) ;
			WriteSIMDZipRegReg
				( dregDst, vregTemp0, typeNEONInt32, false ) ;
			break ;
		case	simdFsub:
			vregTemp0 = AllocateDataRegister( regClassVFP ) ;
			WriteSIMDRevRegReg
				( vregTemp0, dregDst, typeNEONInt64, typeNEONInt32, false ) ;
			WriteSIMDOpRegRegReg
				( armOpVFSUB, thumbOpVFSUB, dregDst, dregDst, dregSrc, false ) ;
			WriteSIMDZipRegReg
				( dregDst, vregTemp0, typeNEONInt32, false ) ;
			break ;
		case	simdFmul:
			vregTemp0 = AllocateDataRegister( regClassVFP ) ;
			WriteSIMDRevRegReg
				( vregTemp0, dregDst, typeNEONInt64, typeNEONInt32, false ) ;
			WriteSIMDOpRegRegReg
				( armOpVFMUL, thumbOpVFMUL, dregDst, dregDst, dregSrc, false ) ;
			WriteSIMDZipRegReg
				( dregDst, vregTemp0, typeNEONInt32, false ) ;
			break ;
		case	simdFdiv:
			vregTemp0 = AllocateDataRegister( regClassVFP ) ;
			vregTemp1 = AllocateDataRegister( regClassVFP ) ;
			WriteMoveVFP64( vregTemp0, dregDst ) ;
			WriteMoveVFP64( vregTemp1, dregSrc ) ;
			WriteVFPOpRegRegReg
				( armOpFDIV, thumbOpFDIV,
					(vregTemp0 << 1),
					(vregTemp0 << 1), (vregTemp1 << 1), false ) ;
			WriteMoveVFP64( dregDst, vregTemp0 ) ;
			break ;
		case	simdFsqrt:
			vregTemp0 = AllocateDataRegister( regClassVFP ) ;
			vregTemp1 = AllocateDataRegister( regClassVFP ) ;
			WriteMoveVFP64( vregTemp0, dregDst ) ;
			WriteMoveVFP64( vregTemp1, dregSrc ) ;
			WriteVFPOpRegReg
				( armOpFSQRT, thumbOpFSQRT,
					(vregTemp0 << 1), (vregTemp1 << 1), false ) ;
			WriteMoveVFP64( dregDst, vregTemp0 ) ;
			break ;
		case	simdFrcp:
			vregTemp0 = AllocateDataRegister( regClassVFP ) ;
			WriteSIMDRevRegReg
				( vregTemp0, dregDst, typeNEONInt64, typeNEONInt32, false ) ;
			WriteSIMDFloatOpRegReg
				( armOpVFRECPE, thumbOpVFRECPE, dregDst, dregSrc, false ) ;
			WriteSIMDZipRegReg
				( dregDst, vregTemp0, typeNEONInt32, false ) ;
			break ;
		case	simdFrsqrt:
			vregTemp0 = AllocateDataRegister( regClassVFP ) ;
			WriteSIMDRevRegReg
				( vregTemp0, dregDst, typeNEONInt64, typeNEONInt32, false ) ;
			WriteSIMDFloatOpRegReg
				( armOpVFRSQRTE, thumbOpVFRSQRTE, dregDst, dregSrc, false ) ;
			WriteSIMDZipRegReg
				( dregDst, vregTemp0, typeNEONInt32, false ) ;
			break ;
		case	simdFabs:
			vregTemp0 = AllocateDataRegister( regClassVFP ) ;
			WriteSIMDRevRegReg
				( vregTemp0, dregDst, typeNEONInt64, typeNEONInt32, false ) ;
			WriteSIMDFloatOpRegReg
				( armOpVFABS, thumbOpVFABS, dregDst, dregSrc, false ) ;
			WriteSIMDZipRegReg
				( dregDst, vregTemp0, typeNEONInt32, false ) ;
			break ;
		case	simdFmax:
			vregTemp0 = AllocateDataRegister( regClassVFP ) ;
			WriteSIMDRevRegReg
				( vregTemp0, dregDst, typeNEONInt64, typeNEONInt32, false ) ;
			WriteSIMDOpRegRegReg
				( armOpVFMAX, thumbOpVFMAX, dregDst, dregDst, dregSrc, false ) ;
			WriteSIMDZipRegReg
				( dregDst, vregTemp0, typeNEONInt32, false ) ;
			break ;
		case	simdFmin:
			vregTemp0 = AllocateDataRegister( regClassVFP ) ;
			WriteSIMDRevRegReg
				( vregTemp0, dregDst, typeNEONInt64, typeNEONInt32, false ) ;
			WriteSIMDOpRegRegReg
				( armOpVFMIN, thumbOpVFMIN, dregDst, dregDst, dregSrc, false ) ;
			WriteSIMDZipRegReg
				( dregDst, vregTemp0, typeNEONInt32, false ) ;
			break ;

		case	simdVadd:
			WriteSIMDOpRegRegReg
				( armOpVFADD, thumbOpVFADD, dregDst, dregDst, dregSrc, true ) ;
			break ;
		case	simdVsub:
			WriteSIMDOpRegRegReg
				( armOpVFSUB, thumbOpVFSUB, dregDst, dregDst, dregSrc, true ) ;
			break ;
		case	simdVmul:
			WriteSIMDOpRegRegReg
				( armOpVFMUL, thumbOpVFMUL, dregDst, dregDst, dregSrc, true ) ;
			break ;
		case	simdVdiv:
			vregTemp0 = AllocateDataRegister( regClassVFP ) ;
			vregTemp1 = AllocateDataRegister( regClassVFP ) ;
			WriteMoveVFP64( vregTemp0, dregDst ) ;
			WriteMoveVFP64( vregTemp1, dregSrc ) ;
			WriteVFPOpRegRegReg
				( armOpFDIV, thumbOpFDIV,
					(vregTemp0 << 1),
					(vregTemp0 << 1), (vregTemp1 << 1), false ) ;
			WriteVFPOpRegRegReg
				( armOpFDIV, thumbOpFDIV,
					(vregTemp0 << 1) + 1,
					(vregTemp0 << 1) + 1, (vregTemp1 << 1) + 1, false ) ;
			WriteMoveVFP64( dregDst, vregTemp0 ) ;
			//
			WriteMoveVFP64( vregTemp0, dregDst+1 ) ;
			WriteMoveVFP64( vregTemp1, dregSrc+1 ) ;
			WriteVFPOpRegRegReg
				( armOpFDIV, thumbOpFDIV,
					(vregTemp0 << 1),
					(vregTemp0 << 1), (vregTemp1 << 1), false ) ;
			WriteVFPOpRegRegReg
				( armOpFDIV, thumbOpFDIV,
					(vregTemp0 << 1) + 1,
					(vregTemp0 << 1) + 1, (vregTemp1 << 1) + 1, false ) ;
			WriteMoveVFP64( dregDst+1, vregTemp0 ) ;
			break ;
		case	simdVsqrt:
			vregTemp0 = AllocateDataRegister( regClassVFP ) ;
			WriteMoveVFP64( vregTemp0, dregSrc ) ;
			WriteVFPOpRegReg
				( armOpFSQRT, thumbOpFSQRT,
					(vregTemp0 << 1), (vregTemp1 << 1), false ) ;
			WriteVFPOpRegReg
				( armOpFSQRT, thumbOpFSQRT,
					(vregTemp0 << 1) + 1, (vregTemp1 << 1) + 1, false ) ;
			WriteMoveVFP64( dregDst, vregTemp0 ) ;
			//
			WriteMoveVFP64( vregTemp0, dregSrc+1 ) ;
			WriteVFPOpRegReg
				( armOpFSQRT, thumbOpFSQRT,
					(vregTemp0 << 1), (vregTemp1 << 1), false ) ;
			WriteVFPOpRegReg
				( armOpFSQRT, thumbOpFSQRT,
					(vregTemp0 << 1) + 1, (vregTemp1 << 1) + 1, false ) ;
			WriteMoveVFP64( dregDst+1, vregTemp0 ) ;
			break ;
		case	simdVrcp:
			WriteSIMDFloatOpRegReg
				( armOpVFRECPE, thumbOpVFRECPE, dregDst, dregSrc, true ) ;
			break ;
		case	simdVrsqrt:
			WriteSIMDFloatOpRegReg
				( armOpVFRSQRTE, thumbOpVFRSQRTE, dregDst, dregSrc, true ) ;
			break ;
		case	simdVabs:
			WriteSIMDFloatOpRegReg
				( armOpVFABS, thumbOpVFABS, dregDst, dregSrc, true ) ;
			break ;
		case	simdVmax:
			WriteSIMDOpRegRegReg
				( armOpVFMAX, thumbOpVFMAX, dregDst, dregDst, dregSrc, true ) ;
			break ;
		case	simdVmin:
			WriteSIMDOpRegRegReg
				( armOpVFMIN, thumbOpVFMIN, dregDst, dregDst, dregSrc, true ) ;
			break ;

		case	simdVcmpne:
			WriteSIMDOpRegRegReg
				( armOpVFCmpEQ, thumbOpVFCmpEQ,
					dregDst, dregDst, dregSrc, true ) ;
			WriteSIMDIntNotRegReg( dregDst, dregDst, typeNEONInt32, true ) ;
			break ;
		case	simdVcmpeq:
			WriteSIMDOpRegRegReg
				( armOpVFCmpEQ, thumbOpVFCmpEQ,
					dregDst, dregDst, dregSrc, true ) ;
			break ;
		case	simdVcmplt:
			WriteSIMDOpRegRegReg
				( armOpVFCmpGT, thumbOpVFCmpGT,
					dregDst, dregSrc, dregDst, true ) ;
			break ;
		case	simdVcmple:
			WriteSIMDOpRegRegReg
				( armOpVFCmpGE, thumbOpVFCmpGE,
					dregDst, dregSrc, dregDst, true ) ;
			break ;
		case	simdVcmpgt:
			WriteSIMDOpRegRegReg
				( armOpVFCmpGT, thumbOpVFCmpGT,
					dregDst, dregDst, dregSrc, true ) ;
			break ;
		case	simdVcmpge:
			WriteSIMDOpRegRegReg
				( armOpVFCmpGE, thumbOpVFCmpGE,
					dregDst, dregDst, dregSrc, true ) ;
			break ;

		case	simdVmove:
			WriteMoveVFP128( vregDst, vregSrc ) ;
			break ;
		case	simdVand:
			WriteSIMDOpRegRegReg
				( armOpVAND, thumbOpVAND,
					dregDst, dregDst, dregSrc, true ) ;
			break ;
		case	simdVor:
			WriteSIMDOpRegRegReg
				( armOpVORR, thumbOpVORR,
					dregDst, dregDst, dregSrc, true ) ;
			break ;
		case	simdVxor:
			WriteSIMDOpRegRegReg
				( armOpVEOR, thumbOpVEOR,
					dregDst, dregDst, dregSrc, true ) ;
			break ;

		case	simdDcvtf2i:
			vregTemp0 = AllocateDataRegister( regClassVFP ) ;
			vregTemp1 = AllocateDataRegister( regClassVFP ) ;
			armTemp0 = (ARMRegister) AllocateDataRegister( regClassARM ) ;
			armTemp1 = (ARMRegister) (armTemp0 + 1) ;
			WriteARMMoveRegImm( armTemp0, 0x80000000 ) ;
			WriteARMMoveRegImm( armTemp1, *((int32_t*)&f32Half) ) ;
			WriteDupARM32toVFP( vregTemp0, armTemp0, false ) ;
			WriteDupARM32toVFP( vregTemp1, armTemp1, false ) ;
			WriteSIMDOpRegRegReg
				( armOpVBIT, thumbOpVBIT,
						vregTemp1, dregSrc, vregTemp0, false ) ;
			WriteSIMDOpRegRegReg
				( armOpVFADD, thumbOpVFADD, dregDst, dregSrc, vregTemp1, false ) ;
			WriteSIMDOpRegRegReg
				( armOpVEOR, thumbOpVEOR, dregDst+1, dregDst+1, dregDst+1, false ) ;
			WriteSIMDFloatOpRegReg
				( armOpVCVT_S32F32, thumbOpVCVT_S32F32, dregDst, dregDst, false ) ;
			break ;
		case	simdDcvti2f:
			WriteSIMDFloatOpRegReg
				( armOpVCVT_F32S32, thumbOpVCVT_F32S32, dregDst, dregSrc, false ) ;
			WriteSIMDOpRegRegReg
				( armOpVEOR, thumbOpVEOR, dregDst+1, dregDst+1, dregDst+1, false ) ;
			break ;
		case	simdDcvtd2f:
			vregTemp0 = AllocateDataRegister( regClassVFP ) ;
			WriteCvtVFP64to32( (vregTemp0 << 1), dregSrc ) ;
			WriteCvtVFP64to32( (vregTemp0 << 1) + 1, dregSrc + 1 ) ;
			WriteSIMDOpRegRegReg
				( armOpVEOR, thumbOpVEOR, dregDst+1, dregDst+1, dregDst+1, false ) ;
			WriteMoveVFP64( dregDst, vregTemp0 ) ;
			break ;
		case	simdDcvtf2d:
			vregTemp0 = AllocateDataRegister( regClassVFP ) ;
			WriteMoveVFP64( vregTemp0, dregSrc ) ;
			WriteCvtVFP32to64( dregDst, (vregTemp0 << 1) ) ;
			WriteCvtVFP32to64( dregDst + 1, (vregTemp0 << 1) + 1 ) ;
			break ;

		case	simdVcvtf2w:
			regClassTemp = regClassNEON ;
			vregTemp0 = AllocateDataRegister( regClassNEON ) ;
			vregTemp1 = AllocateDataRegister( regClassNEON ) ;
			armTemp0 = (ARMRegister) AllocateDataRegister( regClassARM ) ;
			armTemp1 = (ARMRegister) (armTemp0 + 1) ;
			WriteARMMoveRegImm( armTemp0, 0x80000000 ) ;
			WriteARMMoveRegImm( armTemp1, *((int32_t*)&f32Half) ) ;
			WriteDupARM32toVFP( (vregTemp0 << 1), armTemp0, true ) ;
			WriteDupARM32toVFP( (vregTemp1 << 1), armTemp1, true ) ;
			WriteSIMDOpRegRegReg
				( armOpVBIT, thumbOpVBIT,
					(vregTemp1 << 1), dregSrc, (vregTemp0 << 1), true ) ;
			WriteSIMDOpRegRegReg
				( armOpVFADD, thumbOpVFADD, dregDst, dregSrc, (vregTemp1 << 1), true ) ;
			WriteSIMDFloatOpRegReg
				( armOpVCVT_S32F32, thumbOpVCVT_S32F32, dregDst, dregDst, true ) ;
			//
			WriteSIMDOpRegRegReg
				( armOpVEOR, thumbOpVEOR,
					(vregTemp0 << 1), (vregTemp0 << 1), (vregTemp0 << 1), true ) ;
			WriteSIMDQShiftRegRegImm
				( dregDst, dregDst, 16, typeNEONInt32, true, true, true ) ;
			WriteSIMDUnZipRegReg
				( dregDst, (vregTemp0 << 1), typeNEONInt16, true ) ;
			WriteMoveVFP64( dregDst, (vregTemp0 << 1) ) ;
			break ;
		case	simdVcvtw2f:
			if ( dregDst != dregSrc )
			{
				WriteMoveVFP64( dregDst+1, dregSrc ) ;
			}
			WriteSIMDZipRegReg( dregDst, dregDst+1, typeNEONInt16, false ) ;
			WriteSIMDShiftRegRegImm
				( armOpVSHRImm, thumbOpVSHRImm,
					dregDst, dregDst, 16, typeNEONInt32, true, true, true ) ;
			WriteSIMDFloatOpRegReg
				( armOpVCVT_F32S32, thumbOpVCVT_F32S32, dregDst, dregDst, true ) ;
			break ;
		case	simdVcvtf2i:
			regClassTemp = regClassNEON ;
			vregTemp0 = AllocateDataRegister( regClassNEON ) ;
			vregTemp1 = AllocateDataRegister( regClassNEON ) ;
			armTemp0 = (ARMRegister) AllocateDataRegister( regClassARM ) ;
			armTemp1 = (ARMRegister) (armTemp0 + 1) ;
			WriteARMMoveRegImm( armTemp0, 0x80000000 ) ;
			WriteARMMoveRegImm( armTemp1, *((int32_t*)&f32Half) ) ;
			WriteDupARM32toVFP( (vregTemp0 << 1), armTemp0, true ) ;
			WriteDupARM32toVFP( (vregTemp1 << 1), armTemp1, true ) ;
			WriteSIMDOpRegRegReg
				( armOpVBIT, thumbOpVBIT,
					(vregTemp1 << 1), dregSrc, (vregTemp0 << 1), true ) ;
			WriteSIMDOpRegRegReg
				( armOpVFADD, thumbOpVFADD, dregDst, dregSrc, (vregTemp1 << 1), true ) ;
			WriteSIMDFloatOpRegReg
				( armOpVCVT_S32F32, thumbOpVCVT_S32F32, dregDst, dregDst, true ) ;
			break ;
		case	simdVcvti2f:
			WriteSIMDFloatOpRegReg
				( armOpVCVT_F32S32, thumbOpVCVT_F32S32, dregDst, dregSrc, true ) ;
			break ;

		default:
			ESLTrace( "bad instruction packed 128bit SIMD %02X %02X\n",
						codeSIMD128Extension2Op, code ) ;
			WriteToAtomicOrExceptionMask( exceptionBadInstruction ) ;
			break ;
		}
		if ( vregTemp0 >= 0 )
		{
			FreeDataRegister( regClassTemp, vregTemp0 ) ;
		}
		if ( vregTemp1 >= 0 )
		{
			FreeDataRegister( regClassTemp, vregTemp1 ) ;
		}
		if ( armTemp0 != ARM_Nothing )
		{
			FreeDataRegister( regClassARM, armTemp0 ) ;
		}
		SetDataRegisterModified( regClassNEON, vregDst ) ;
		UnlockDataRegister( regClassNEON, vregDst ) ;
		UnlockDataRegister( regClassNEON, vregSrc ) ;
	}
	else if ( m_vfpVersion >= 2 )
	{
		switch ( code )
		{
		case	simdVmove:
			write_move_reg_reg( regDst, regSrc, true ) ;
			return ;
		case	simdVand:
			write_and_reg_reg( regDst, regSrc, true ) ;
			return ;
		case	simdVor:
			write_or_reg_reg( regDst, regSrc, true ) ;
			return ;
		case	simdVxor:
			write_xor_reg_reg( regDst, regSrc, true ) ;
			return ;
		}
		ARMRegister	armTemp0 = ARM_Nothing ;
		ARMRegister	armTemp1 = ARM_Nothing ;
		int	vregSrc0 = WriteRealizeDataRegister( regSrc, regClassVFP ) ;
		int	vregDst0 = WriteRealizeDataRegister( regDst, regClassVFP, fLoadDstValue ) ;
		int	vregSrc1 = -1 ;
		int	vregDst1 = -1 ;
		int	vregTemp0 = -1 ;
		switch ( code )
		{
		case	simdFadd:
			WriteVFPOpRegRegReg
				( armOpFADD, thumbOpFADD,
					(vregDst0 << 1), (vregDst0 << 1), (vregSrc0 << 1), false ) ;
			break ;
		case	simdFsub:
			WriteVFPOpRegRegReg
				( armOpFSUB, thumbOpFSUB,
					(vregDst0 << 1), (vregDst0 << 1), (vregSrc0 << 1), false ) ;
			break ;
		case	simdFmul:
			WriteVFPOpRegRegReg
				( armOpFMUL, thumbOpFMUL,
					(vregDst0 << 1), (vregDst0 << 1), (vregSrc0 << 1), false ) ;
			break ;
		case	simdFdiv:
			WriteVFPOpRegRegReg
				( armOpFDIV, thumbOpFDIV,
					(vregDst0 << 1), (vregDst0 << 1), (vregSrc0 << 1), false ) ;
			break ;
		case	simdFsqrt:
			WriteVFPOpRegReg
				( armOpFSQRT, thumbOpFSQRT,
					(vregDst0 << 1), (vregSrc0 << 1), false ) ;
			break ;
		case	simdFrcp:
			WriteVFPLoadImm32( (vregDst0 << 1), 1.0f ) ;
			WriteVFPOpRegRegReg
				( armOpFDIV, thumbOpFDIV,
					(vregDst0 << 1), (vregDst0 << 1), (vregSrc0 << 1), false ) ;
			break ;
		case	simdFrsqrt:
			vregTemp0 = AllocateDataRegister( regClassVFP ) ;
			WriteVFPLoadImm32( (vregDst0 << 1), 1.0f ) ;
			WriteVFPOpRegReg
				( armOpFSQRT, thumbOpFSQRT,
					(vregTemp0 << 1), (vregSrc0 << 1), false ) ;
			WriteVFPOpRegRegReg
				( armOpFDIV, thumbOpFDIV,
					(vregDst0 << 1), (vregDst0 << 1), (vregTemp0 << 1), false ) ;
			break ;
		case	simdFabs:
			WriteVFPOpRegReg
				( armOpFABS, thumbOpFABS,
					(vregDst0 << 1), (vregSrc0 << 1), false ) ;
			break ;
		case	simdFmax:
			write_vfp_cmove_float32
				( (vregDst0 << 1), (vregSrc0 << 1), cond_LT ) ;
			break ;
		case	simdFmin:
			write_vfp_cmove_float32
				( (vregDst0 << 1), (vregSrc0 << 1), cond_GT ) ;
			break ;

		case	simdVadd:
			vregSrc1 = WriteRealizeDataRegister( regSrc+1, regClassVFP ) ;
			vregDst1 = WriteRealizeDataRegister( regDst+1, regClassVFP, fLoadDstValue ) ;
			WriteVFPOpRegRegReg
				( armOpFADD, thumbOpFADD,
					(vregDst0 << 1),
					(vregDst0 << 1), (vregSrc0 << 1), false ) ;
			WriteVFPOpRegRegReg
				( armOpFADD, thumbOpFADD,
					(vregDst0 << 1) + 1,
					(vregDst0 << 1) + 1, (vregSrc0 << 1) + 1, false ) ;
			WriteVFPOpRegRegReg
				( armOpFADD, thumbOpFADD,
					(vregDst1 << 1),
					(vregDst1 << 1), (vregSrc1 << 1), false ) ;
			WriteVFPOpRegRegReg
				( armOpFADD, thumbOpFADD,
					(vregDst1 << 1) + 1,
					(vregDst1 << 1) + 1, (vregSrc1 << 1) + 1, false ) ;
			break ;
		case	simdVsub:
			vregSrc1 = WriteRealizeDataRegister( regSrc+1, regClassVFP ) ;
			vregDst1 = WriteRealizeDataRegister( regDst+1, regClassVFP, fLoadDstValue ) ;
			WriteVFPOpRegRegReg
				( armOpFSUB, thumbOpFSUB,
					(vregDst0 << 1),
					(vregDst0 << 1), (vregSrc0 << 1), false ) ;
			WriteVFPOpRegRegReg
				( armOpFSUB, thumbOpFSUB,
					(vregDst0 << 1) + 1,
					(vregDst0 << 1) + 1, (vregSrc0 << 1) + 1, false ) ;
			WriteVFPOpRegRegReg
				( armOpFSUB, thumbOpFSUB,
					(vregDst1 << 1),
					(vregDst1 << 1), (vregSrc1 << 1), false ) ;
			WriteVFPOpRegRegReg
				( armOpFSUB, thumbOpFSUB,
					(vregDst1 << 1) + 1,
					(vregDst1 << 1) + 1, (vregSrc1 << 1) + 1, false ) ;
			break ;
		case	simdVmul:
			vregSrc1 = WriteRealizeDataRegister( regSrc+1, regClassVFP ) ;
			vregDst1 = WriteRealizeDataRegister( regDst+1, regClassVFP, fLoadDstValue ) ;
			WriteVFPOpRegRegReg
				( armOpFMUL, thumbOpFMUL,
					(vregDst0 << 1),
					(vregDst0 << 1), (vregSrc0 << 1), false ) ;
			WriteVFPOpRegRegReg
				( armOpFMUL, thumbOpFMUL,
					(vregDst0 << 1) + 1,
					(vregDst0 << 1) + 1, (vregSrc0 << 1) + 1, false ) ;
			WriteVFPOpRegRegReg
				( armOpFMUL, thumbOpFMUL,
					(vregDst1 << 1),
					(vregDst1 << 1), (vregSrc1 << 1), false ) ;
			WriteVFPOpRegRegReg
				( armOpFMUL, thumbOpFMUL,
					(vregDst1 << 1) + 1,
					(vregDst1 << 1) + 1, (vregSrc1 << 1) + 1, false ) ;
			break ;
		case	simdVdiv:
			vregSrc1 = WriteRealizeDataRegister( regSrc+1, regClassVFP ) ;
			vregDst1 = WriteRealizeDataRegister( regDst+1, regClassVFP, fLoadDstValue ) ;
			WriteVFPOpRegRegReg
				( armOpFDIV, thumbOpFDIV,
					(vregDst0 << 1),
					(vregDst0 << 1), (vregSrc0 << 1), false ) ;
			WriteVFPOpRegRegReg
				( armOpFDIV, thumbOpFDIV,
					(vregDst0 << 1) + 1,
					(vregDst0 << 1) + 1, (vregSrc0 << 1) + 1, false ) ;
			WriteVFPOpRegRegReg
				( armOpFDIV, thumbOpFDIV,
					(vregDst1 << 1),
					(vregDst1 << 1), (vregSrc1 << 1), false ) ;
			WriteVFPOpRegRegReg
				( armOpFDIV, thumbOpFDIV,
					(vregDst1 << 1) + 1,
					(vregDst1 << 1) + 1, (vregSrc1 << 1) + 1, false ) ;
			break ;
		case	simdVsqrt:
			vregSrc1 = WriteRealizeDataRegister( regSrc+1, regClassVFP ) ;
			vregDst1 = WriteRealizeDataRegister( regDst+1, regClassVFP, fLoadDstValue ) ;
			WriteVFPOpRegReg
				( armOpFSQRT, thumbOpFSQRT,
					(vregDst0 << 1), (vregSrc0 << 1), false ) ;
			WriteVFPOpRegReg
				( armOpFSQRT, thumbOpFSQRT,
					(vregDst0 << 1) + 1, (vregSrc0 << 1) + 1, false ) ;
			WriteVFPOpRegReg
				( armOpFSQRT, thumbOpFSQRT,
					(vregDst1 << 1), (vregSrc1 << 1), false ) ;
			WriteVFPOpRegReg
				( armOpFSQRT, thumbOpFSQRT,
					(vregDst1 << 1) + 1, (vregSrc1 << 1) + 1, false ) ;
			break ;
		case	simdVrcp:
			vregSrc1 = WriteRealizeDataRegister( regSrc+1, regClassVFP ) ;
			vregDst1 = WriteRealizeDataRegister( regDst+1, regClassVFP, fLoadDstValue ) ;
			WriteVFPLoadImm32( (vregDst0 << 1), 1.0f ) ;
			WriteVFPLoadImm32( (vregDst0 << 1) + 1, 1.0f ) ;
			WriteVFPLoadImm32( (vregDst1 << 1), 1.0f ) ;
			WriteVFPLoadImm32( (vregDst1 << 1) + 1, 1.0f ) ;
			WriteVFPOpRegRegReg
				( armOpFDIV, thumbOpFDIV,
					(vregDst0 << 1),
					(vregDst0 << 1), (vregSrc0 << 1), false ) ;
			WriteVFPOpRegRegReg
				( armOpFDIV, thumbOpFDIV,
					(vregDst0 << 1) + 1,
					(vregDst0 << 1) + 1, (vregSrc0 << 1) + 1, false ) ;
			WriteVFPOpRegRegReg
				( armOpFDIV, thumbOpFDIV,
					(vregDst1 << 1),
					(vregDst1 << 1), (vregSrc1 << 1), false ) ;
			WriteVFPOpRegRegReg
				( armOpFDIV, thumbOpFDIV,
					(vregDst1 << 1) + 1,
					(vregDst1 << 1) + 1, (vregSrc1 << 1) + 1, false ) ;
			break ;
		case	simdVrsqrt:
			vregTemp0 = AllocateDataRegister( regClassVFP ) ;
			vregSrc1 = WriteRealizeDataRegister( regSrc+1, regClassVFP ) ;
			vregDst1 = WriteRealizeDataRegister( regDst+1, regClassVFP, fLoadDstValue ) ;
			WriteVFPLoadImm32( (vregDst0 << 1), 1.0f ) ;
			WriteVFPLoadImm32( (vregDst0 << 1) + 1, 1.0f ) ;
			WriteVFPLoadImm32( (vregDst1 << 1), 1.0f ) ;
			WriteVFPLoadImm32( (vregDst1 << 1) + 1, 1.0f ) ;
			WriteVFPOpRegReg
				( armOpFSQRT, thumbOpFSQRT,
					(vregTemp0 << 1), (vregSrc0 << 1), false ) ;
			WriteVFPOpRegReg
				( armOpFSQRT, thumbOpFSQRT,
					(vregTemp0 << 1) + 1, (vregSrc0 << 1) + 1, false ) ;
			WriteVFPOpRegRegReg
				( armOpFDIV, thumbOpFDIV,
					(vregDst0 << 1),
					(vregDst0 << 1), (vregTemp0 << 1), false ) ;
			WriteVFPOpRegRegReg
				( armOpFDIV, thumbOpFDIV,
					(vregDst0 << 1) + 1,
					(vregDst0 << 1) + 1, (vregTemp0 << 1) + 1, false ) ;
			WriteVFPOpRegReg
				( armOpFSQRT, thumbOpFSQRT,
					(vregTemp0 << 1), (vregSrc1 << 1), false ) ;
			WriteVFPOpRegReg
				( armOpFSQRT, thumbOpFSQRT,
					(vregTemp0 << 1) + 1, (vregSrc1 << 1) + 1, false ) ;
			WriteVFPOpRegRegReg
				( armOpFDIV, thumbOpFDIV,
					(vregDst1 << 1),
					(vregDst1 << 1), (vregTemp0 << 1), false ) ;
			WriteVFPOpRegRegReg
				( armOpFDIV, thumbOpFDIV,
					(vregDst1 << 1) + 1,
					(vregDst1 << 1) + 1, (vregTemp0 << 1) + 1, false ) ;
			break ;
		case	simdVabs:
			vregSrc1 = WriteRealizeDataRegister( regSrc+1, regClassVFP ) ;
			vregDst1 = WriteRealizeDataRegister( regDst+1, regClassVFP, fLoadDstValue ) ;
			WriteVFPOpRegReg
				( armOpFABS, thumbOpFABS,
					(vregDst0 << 1), (vregSrc0 << 1), false ) ;
			WriteVFPOpRegReg
				( armOpFABS, thumbOpFABS,
					(vregDst0 << 1) + 1, (vregSrc0 << 1) + 1, false ) ;
			WriteVFPOpRegReg
				( armOpFABS, thumbOpFABS,
					(vregDst1 << 1), (vregSrc1 << 1), false ) ;
			WriteVFPOpRegReg
				( armOpFABS, thumbOpFABS,
					(vregDst1 << 1) + 1, (vregSrc1 << 1) + 1, false ) ;
			break ;
		case	simdVmax:
			vregSrc1 = WriteRealizeDataRegister( regSrc+1, regClassVFP ) ;
			vregDst1 = WriteRealizeDataRegister( regDst+1, regClassVFP, fLoadDstValue ) ;
			write_vfp_cmove_float32
				( (vregDst0 << 1), (vregSrc0 << 1), cond_LT ) ;
			write_vfp_cmove_float32
				( (vregDst0 << 1) + 1, (vregSrc0 << 1) + 1, cond_LT ) ;
			write_vfp_cmove_float32
				( (vregDst1 << 1), (vregSrc1 << 1), cond_LT ) ;
			write_vfp_cmove_float32
				( (vregDst1 << 1) + 1, (vregSrc1 << 1) + 1, cond_LT ) ;
			break ;
		case	simdVmin:
			vregSrc1 = WriteRealizeDataRegister( regSrc+1, regClassVFP ) ;
			vregDst1 = WriteRealizeDataRegister( regDst+1, regClassVFP, fLoadDstValue ) ;
			write_vfp_cmove_float32
				( (vregDst0 << 1), (vregSrc0 << 1), cond_GT ) ;
			write_vfp_cmove_float32
				( (vregDst0 << 1) + 1, (vregSrc0 << 1) + 1, cond_GT ) ;
			write_vfp_cmove_float32
				( (vregDst1 << 1), (vregSrc1 << 1), cond_GT ) ;
			write_vfp_cmove_float32
				( (vregDst1 << 1) + 1, (vregSrc1 << 1) + 1, cond_GT ) ;
			break ;

		case	simdVcmpne:
			vregSrc1 = WriteRealizeDataRegister( regSrc+1, regClassVFP ) ;
			vregDst1 = WriteRealizeDataRegister( regDst+1, regClassVFP, fLoadDstValue ) ;
			write_vfp_cmpxx_float32( (vregDst0 << 1), (vregSrc0 << 1), cond_NE ) ;
			write_vfp_cmpxx_float32( (vregDst0 << 1) + 1, (vregSrc0 << 1) + 1, cond_NE ) ;
			write_vfp_cmpxx_float32( (vregDst1 << 1), (vregSrc1 << 1), cond_NE ) ;
			write_vfp_cmpxx_float32( (vregDst1 << 1) + 1, (vregSrc1 << 1) + 1, cond_NE ) ;
			break ;
		case	simdVcmpeq:
			vregSrc1 = WriteRealizeDataRegister( regSrc+1, regClassVFP ) ;
			vregDst1 = WriteRealizeDataRegister( regDst+1, regClassVFP, fLoadDstValue ) ;
			write_vfp_cmpxx_float32( (vregDst0 << 1), (vregSrc0 << 1), cond_EQ ) ;
			write_vfp_cmpxx_float32( (vregDst0 << 1) + 1, (vregSrc0 << 1) + 1, cond_EQ ) ;
			write_vfp_cmpxx_float32( (vregDst1 << 1), (vregSrc1 << 1), cond_EQ ) ;
			write_vfp_cmpxx_float32( (vregDst1 << 1) + 1, (vregSrc1 << 1) + 1, cond_EQ ) ;
			break ;
		case	simdVcmplt:
			vregSrc1 = WriteRealizeDataRegister( regSrc+1, regClassVFP ) ;
			vregDst1 = WriteRealizeDataRegister( regDst+1, regClassVFP, fLoadDstValue ) ;
			write_vfp_cmpxx_float32( (vregDst0 << 1), (vregSrc0 << 1), cond_LT ) ;
			write_vfp_cmpxx_float32( (vregDst0 << 1) + 1, (vregSrc0 << 1) + 1, cond_LT ) ;
			write_vfp_cmpxx_float32( (vregDst1 << 1), (vregSrc1 << 1), cond_LT ) ;
			write_vfp_cmpxx_float32( (vregDst1 << 1) + 1, (vregSrc1 << 1) + 1, cond_LT ) ;
			break ;
		case	simdVcmple:
			vregSrc1 = WriteRealizeDataRegister( regSrc+1, regClassVFP ) ;
			vregDst1 = WriteRealizeDataRegister( regDst+1, regClassVFP, fLoadDstValue ) ;
			write_vfp_cmpxx_float32( (vregDst0 << 1), (vregSrc0 << 1), cond_LE ) ;
			write_vfp_cmpxx_float32( (vregDst0 << 1) + 1, (vregSrc0 << 1) + 1, cond_LE ) ;
			write_vfp_cmpxx_float32( (vregDst1 << 1), (vregSrc1 << 1), cond_LE ) ;
			write_vfp_cmpxx_float32( (vregDst1 << 1) + 1, (vregSrc1 << 1) + 1, cond_LE ) ;
			break ;
		case	simdVcmpgt:
			vregSrc1 = WriteRealizeDataRegister( regSrc+1, regClassVFP ) ;
			vregDst1 = WriteRealizeDataRegister( regDst+1, regClassVFP, fLoadDstValue ) ;
			write_vfp_cmpxx_float32( (vregDst0 << 1), (vregSrc0 << 1), cond_GT ) ;
			write_vfp_cmpxx_float32( (vregDst0 << 1) + 1, (vregSrc0 << 1) + 1, cond_GT ) ;
			write_vfp_cmpxx_float32( (vregDst1 << 1), (vregSrc1 << 1), cond_GT ) ;
			write_vfp_cmpxx_float32( (vregDst1 << 1) + 1, (vregSrc1 << 1) + 1, cond_GT ) ;
			break ;
		case	simdVcmpge:
			vregSrc1 = WriteRealizeDataRegister( regSrc+1, regClassVFP ) ;
			vregDst1 = WriteRealizeDataRegister( regDst+1, regClassVFP, fLoadDstValue ) ;
			write_vfp_cmpxx_float32( (vregDst0 << 1), (vregSrc0 << 1), cond_GE ) ;
			write_vfp_cmpxx_float32( (vregDst0 << 1) + 1, (vregSrc0 << 1) + 1, cond_GE ) ;
			write_vfp_cmpxx_float32( (vregDst1 << 1), (vregSrc1 << 1), cond_GE ) ;
			write_vfp_cmpxx_float32( (vregDst1 << 1) + 1, (vregSrc1 << 1) + 1, cond_GE ) ;
			break ;

		case	simdDcvtf2i:
			vregDst1 = WriteRealizeDataRegister( regDst+1, regClassVFP, fLoadDstValue ) ;
			WriteCvtVFPtoInt32
				( (vregDst0 << 1), (vregSrc0 << 1), false, false, false ) ;
			WriteCvtVFPtoInt32
				( (vregDst0 << 1) + 1, (vregSrc0 << 1) + 1, false, false, false ) ;
			WriteVFPLoadImm64( vregDst1, 0 ) ;
			break ;
		case	simdDcvti2f:
			vregDst1 = WriteRealizeDataRegister( regDst+1, regClassVFP, fLoadDstValue ) ;
			WriteCvtVFPInt32toFloat
				( (vregDst0 << 1), (vregSrc0 << 1), false, false ) ;
			WriteCvtVFPInt32toFloat
				( (vregDst0 << 1) + 1, (vregSrc0 << 1) + 1, false, false ) ;
			WriteVFPLoadImm64( vregDst1, 0 ) ;
			break ;
		case	simdDcvtd2f:
			vregSrc1 = WriteRealizeDataRegister( regSrc+1, regClassVFP ) ;
			vregDst1 = WriteRealizeDataRegister( regDst+1, regClassVFP, fLoadDstValue ) ;
			WriteCvtVFP64to32( (vregDst0 << 1), vregSrc0 ) ;
			WriteCvtVFP64to32( (vregDst0 << 1) + 1, vregSrc1 ) ;
			WriteVFPLoadImm64( vregDst1, 0 ) ;
			break ;
		case	simdDcvtf2d:
			vregDst1 = WriteRealizeDataRegister( regDst+1, regClassVFP, fLoadDstValue ) ;
			WriteCvtVFP32to64( vregDst1, (vregSrc0 << 1) + 1 ) ;
			WriteCvtVFP32to64( vregDst0, (vregSrc0 << 1) ) ;
			break ;

		case	simdVcvtf2w:
			vregSrc1 = WriteRealizeDataRegister( regSrc+1, regClassVFP ) ;
			vregDst1 = WriteRealizeDataRegister( regDst+1, regClassVFP, fLoadDstValue ) ;
			armTemp0 = (ARMRegister) AllocateDataRegister( regClassARM ) ;
			armTemp1 = (ARMRegister) (armTemp0 + 1) ;
			WriteCvtVFPtoInt32
				( (vregDst0 << 1), (vregSrc0 << 1), false, false, false ) ;
			WriteCvtVFPtoInt32
				( (vregDst0 << 1) + 1, (vregSrc0 << 1) + 1, false, false, false ) ;
			WriteCvtVFPtoInt32
				( (vregDst1 << 1), (vregSrc1 << 1), false, false, false ) ;
			WriteCvtVFPtoInt32
				( (vregDst1 << 1) + 1, (vregSrc1 << 1) + 1, false, false, false ) ;
			WriteMoveVFPtoARM64( armTemp0, armTemp1, vregDst0 ) ;
			WriteARMClampValueToSigned16( armTemp0, ARM_r6 ) ;
			WriteARMClampValueToSigned16( armTemp1, ARM_r6 ) ;
			if ( m_armVersion >= 6 )
			{
				WriteARMBFIRegRegImmImm( armTemp0, armTemp1, 16, 16 ) ;
			}
			else
			{
				WriteARMShiftLeftImm( armTemp0, armTemp1, 16 ) ;
				WriteARMShiftLeftImm( armTemp1, armTemp1, 16 ) ;
				WriteARMOrRegRegRegShift( armTemp0, armTemp1, armTemp0, -16 ) ;
			}
			WriteMoveARMtoVFP32( (vregDst0 << 1), armTemp0 ) ;
			//
			WriteMoveVFPtoARM64( armTemp0, armTemp1, vregDst1 ) ;
			WriteARMClampValueToSigned16( armTemp0, ARM_r6 ) ;
			WriteARMClampValueToSigned16( armTemp1, ARM_r6 ) ;
			if ( m_armVersion >= 6 )
			{
				WriteARMBFIRegRegImmImm( armTemp0, armTemp1, 16, 16 ) ;
			}
			else
			{
				WriteARMShiftLeftImm( armTemp0, armTemp1, 16 ) ;
				WriteARMShiftLeftImm( armTemp1, armTemp1, 16 ) ;
				WriteARMOrRegRegRegShift( armTemp0, armTemp1, armTemp0, -16 ) ;
			}
			WriteMoveARMtoVFP32( (vregDst0 << 1) + 1, armTemp0 ) ;
			WriteVFPLoadImm64( vregDst1, 0 ) ;
			break ;
		case	simdVcvtw2f:
			vregDst1 = WriteRealizeDataRegister( regDst+1, regClassVFP, fLoadDstValue ) ;
			armTemp0 = (ARMRegister) AllocateDataRegister( regClassARM ) ;
			armTemp1 = (ARMRegister) (armTemp0 + 1) ;
			WriteMoveVFPtoARM32( armTemp0, (vregSrc0 << 1) + 1 ) ;
			WriteARMShiftARightImm( armTemp1, armTemp0, 16 ) ;
			WriteARMShiftLeftImm( armTemp0, armTemp0, 16 ) ;
			WriteARMShiftARightImm( armTemp0, armTemp0, 16 ) ;
			WriteMoveARMtoVFP64( vregDst1, armTemp0, armTemp1 ) ;
			WriteCvtVFPInt32toFloat
				( (vregDst1 << 1), (vregDst1 << 1), false, false ) ;
			WriteCvtVFPInt32toFloat
				( (vregDst1 << 1) + 1, (vregDst1 << 1) + 1, false, false ) ;
			//
			WriteMoveVFPtoARM32( armTemp0, (vregSrc0 << 1) ) ;
			WriteARMShiftARightImm( armTemp1, armTemp0, 16 ) ;
			WriteARMShiftLeftImm( armTemp0, armTemp0, 16 ) ;
			WriteARMShiftARightImm( armTemp0, armTemp0, 16 ) ;
			WriteMoveARMtoVFP64( vregDst0, armTemp0, armTemp1 ) ;
			WriteCvtVFPInt32toFloat
				( (vregDst0 << 1), (vregDst0 << 1), false, false ) ;
			WriteCvtVFPInt32toFloat
				( (vregDst0 << 1) + 1, (vregDst0 << 1) + 1, false, false ) ;
			break ;
		case	simdVcvtf2i:
			vregSrc1 = WriteRealizeDataRegister( regSrc+1, regClassVFP ) ;
			vregDst1 = WriteRealizeDataRegister( regDst+1, regClassVFP, fLoadDstValue ) ;
			WriteCvtVFPtoInt32
				( (vregDst0 << 1), (vregSrc0 << 1), false, false, false ) ;
			WriteCvtVFPtoInt32
				( (vregDst0 << 1) + 1, (vregSrc0 << 1) + 1, false, false, false ) ;
			WriteCvtVFPtoInt32
				( (vregDst1 << 1), (vregSrc1 << 1), false, false, false ) ;
			WriteCvtVFPtoInt32
				( (vregDst1 << 1) + 1, (vregSrc1 << 1) + 1, false, false, false ) ;
			break ;
		case	simdVcvti2f:
			vregSrc1 = WriteRealizeDataRegister( regSrc+1, regClassVFP ) ;
			vregDst1 = WriteRealizeDataRegister( regDst+1, regClassVFP, fLoadDstValue ) ;
			WriteCvtVFPInt32toFloat
				( (vregDst0 << 1), (vregSrc0 << 1), false, false ) ;
			WriteCvtVFPInt32toFloat
				( (vregDst0 << 1) + 1, (vregSrc0 << 1) + 1, false, false ) ;
			WriteCvtVFPInt32toFloat
				( (vregDst1 << 1), (vregSrc1 << 1), false, false ) ;
			WriteCvtVFPInt32toFloat
				( (vregDst1 << 1) + 1, (vregSrc1 << 1) + 1, false, false ) ;
			break ;

		default:
			ESLTrace( "bad instruction packed 128bit SIMD %02X %02X\n",
						codeSIMD128Extension2Op, code ) ;
			WriteToAtomicOrExceptionMask( exceptionBadInstruction ) ;
			break ;
		}
		if ( vregTemp0 >= 0 )
		{
			FreeDataRegister( regClassVFP, vregTemp0 ) ;
		}
		if ( armTemp0 != ARM_Nothing )
		{
			FreeDataRegister( regClassARM, armTemp0 ) ;
		}
		SetDataRegisterModified( regClassVFP, vregDst0 ) ;
		UnlockDataRegister( regClassVFP, vregDst0 ) ;
		UnlockDataRegister( regClassVFP, vregSrc0 ) ;
		if ( vregDst1 >= 0 )
		{
			SetDataRegisterModified( regClassVFP, vregDst1 ) ;
			UnlockDataRegister( regClassVFP, vregDst1 ) ;
		}
		if ( vregSrc1 >= 0 )
		{
			UnlockDataRegister( regClassVFP, vregSrc1 ) ;
		}
	}
	else
	{
		Sakura2Assembler::write_simd128_extension( code, regDst, regSrc ) ;
	}
}

void ARMGenericAssembler::write_simd128_imm_extension( int code, int regDst, int regSrc, int imm8 )
{
	int	vregDst, vregSrc, dregDst, dregSrc ;
	switch ( code )
	{
	case	simdVmaskmove:
		write_maskmove_reg_reg_reg( regDst, regSrc, imm8, true ) ;
		break ;

	case	simdVshuf32:
		if ( !m_vfpNEON )
		{
			if ( m_vfpVersion < 2 )
			{
				Sakura2Assembler::write_simd128_imm_extension( code, regDst, regSrc, imm8 ) ;
				break ;
			}
			int	vregDst1, vregSrc1 ;
			vregSrc = AllocateDataRegister( regClassVFP ) ;
			vregSrc1 = AllocateDataRegister( regClassVFP ) ;
			RealizeFreeVFPRegister( vregSrc, regSrc, true, false ) ;
			RealizeFreeVFPRegister( vregSrc1, regSrc+1, true, false ) ;
			vregDst = WriteRealizeDataRegister( regDst, regClassVFP, false ) ;
			vregDst1 = WriteRealizeDataRegister( regDst+1, regClassVFP, false ) ;
			//
			WriteMoveVFP32
				( (vregDst << 1),
					((imm8 & 0x02) ? vregSrc : vregSrc1) + (imm8 & 0x01) ) ;
			WriteMoveVFP32
				( (vregDst << 1) + 1,
					((imm8 & 0x08) ? vregSrc : vregSrc1) + ((imm8 >> 2) & 0x01) ) ;
			WriteMoveVFP32
				( (vregDst1 << 1),
					((imm8 & 0x20) ? vregSrc : vregSrc1) + ((imm8 >> 4) & 0x01) ) ;
			WriteMoveVFP32
				( (vregDst1 << 1) + 1,
					((imm8 & 0x80) ? vregSrc : vregSrc1) + ((imm8 >> 6) & 0x01) ) ;
			//
			SetDataRegisterModified( regClassVFP, vregDst ) ;
			SetDataRegisterModified( regClassVFP, vregDst1 ) ;
			UnlockDataRegister( regClassVFP, vregDst ) ;
			UnlockDataRegister( regClassVFP, vregDst1 ) ;
			FreeDataRegister( regClassVFP, vregSrc ) ;
			FreeDataRegister( regClassVFP, vregSrc1 ) ;
			break ;
		}
		vregSrc = WriteRealizeDataRegister( regSrc, regClassNEON ) ;
		vregDst = WriteRealizeDataRegister( regDst, regClassNEON, false ) ;
		dregSrc = (vregSrc << 1) ;
		dregDst = (vregDst << 1) ;
		if ( (imm8 == 0x00) || (imm8 == 0x55)
			|| (imm8 == 0xAA) || (imm8 == 0xFF) )
		{
			WriteSIMDDupRegRegImm
				( dregDst, dregSrc + ((imm8 & 0x02) >> 1),
							(imm8 & 0x01), typeNEONInt32, true ) ;
		}
		else if ( imm8 == 0xD8 )
		{
			if ( vregDst != vregSrc )
			{
				WriteMoveVFP128( vregDst, vregSrc ) ;
			}
			WriteSIMDZipRegReg( dregDst, dregDst+1, typeNEONInt32, false ) ;
		}
		else if ( imm8 == 0x72 )
		{
			if ( vregDst != vregSrc )
			{
				WriteMoveVFP128( vregDst, vregSrc ) ;
			}
			WriteSIMDZipRegReg( dregDst+1, dregDst, typeNEONInt32, false ) ;
			WriteSIMDSwapRegReg( dregDst, dregDst+1, false ) ;
		}
		else if ( imm8 == 0x1B )
		{
			WriteSIMDRevRegReg
				( dregDst, dregSrc, typeNEONInt64, typeNEONInt32, true ) ;
			WriteSIMDSwapRegReg( dregDst, dregDst+1, false ) ;
		}
		else if ( imm8 == 0xB1 )
		{
			WriteSIMDRevRegReg
				( dregDst, dregSrc, typeNEONInt64, typeNEONInt32, true ) ;
		}
		else if ( imm8 == 0x4E )
		{
			if ( dregDst == dregSrc )
			{
				WriteSIMDSwapRegReg( dregDst, dregDst+1, false ) ;
			}
			else
			{
				WriteMoveVFP64( dregDst, dregSrc+1 ) ;
				WriteMoveVFP64( dregDst+1, dregSrc ) ;
			}
		}
		else if ( imm8 == 0x39 )
		{
			WriteSIMDExtRegRegRegImm
				( dregDst, dregSrc, dregSrc, 4, true ) ;
		}
		else if ( imm8 == 0x93 )
		{
			WriteSIMDExtRegRegRegImm
				( dregDst, dregSrc, dregSrc, 12, true ) ;
		}
		else
		{
			int	vregTemp = -1 ;
			for ( int i = 0; i < 2; i ++ )
			{
				switch ( imm8 & 0x0F )
				{
				case	0x00:
					WriteSIMDDupRegRegImm
						( dregDst+i, dregSrc, 0, typeNEONInt32, false ) ;
					break ;
				case	0x01:
					WriteSIMDRevRegReg
						( dregDst+i, dregSrc, typeNEONInt64, typeNEONInt32, false ) ;
					break ;
				case	0x04:
					WriteMoveVFP64( dregDst+i, dregSrc ) ;
					break ;
				case	0x05:
					WriteSIMDDupRegRegImm
						( dregDst+i, dregSrc, 1, typeNEONInt32, false ) ;
					break ;
				case	0x0A:
					WriteSIMDDupRegRegImm
						( dregDst+i, dregSrc+1, 0, typeNEONInt32, false ) ;
					break ;
				case	0x0B:
					WriteSIMDRevRegReg
						( dregDst+i, dregSrc+1, typeNEONInt64, typeNEONInt32, false ) ;
					break ;
				case	0x0E:
					WriteMoveVFP64( dregDst+i, dregSrc+1 ) ;
					break ;
				case	0x0F:
					WriteSIMDDupRegRegImm
						( dregDst+i, dregSrc+1, 1, typeNEONInt32, false ) ;
					break ;
				default:
					vregTemp = AllocateDataRegister( regClassVFP ) ;
					if ( imm8 & 0x01 )
					{
						WriteSIMDRevRegReg
							( dregDst+i, dregSrc+((imm8 >> 1) & 0x01),
								typeNEONInt64, typeNEONInt32, false ) ;
					}
					else
					{
						WriteMoveVFP64
							( dregDst+i, dregSrc+((imm8 >> 1) & 0x01) ) ;
					}
					if ( imm8 & 0x04 )
					{
						WriteSIMDRevRegReg
							( vregTemp, dregSrc+((imm8 >> 3) & 0x01),
								typeNEONInt64, typeNEONInt32, false ) ;
					}
					else
					{
						WriteMoveVFP64
							( vregTemp, dregSrc+((imm8 >> 3) & 0x01) ) ;
					}
					WriteSIMDZipRegReg
						( dregDst+i, vregTemp, typeNEONInt32, false ) ;
					FreeDataRegister( regClassVFP, vregTemp ) ;
				}
				imm8 >>= 4 ;
			}
		}
		SetDataRegisterModified( regClassNEON, vregDst ) ;
		UnlockDataRegister( regClassNEON, vregDst ) ;
		UnlockDataRegister( regClassNEON, vregSrc ) ;
		break ;

	default:
		ESLTrace( "bad instruction packed 128bit SIMD %02X %02X\n",
					codeSIMD128Extension3Op, code ) ;
		WriteToAtomicOrExceptionMask( exceptionBadInstruction ) ;
		break ;
	}
}

// 間接無条件ジャンプ命令（write_push_ip と組み合わせればコール）
//（exceptionFarJump 例外の判定と設定を含む）
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::write_jump_reg( int reg )
{
	FlushAllRegisters() ;
	//
	int	regTemp = AllocateDataRegister( regClassARM ) ;
	WriteToLoadSakura2Register( (ARMRegister) regTemp, reg ) ;
	PreserveContinuousCodes( 0x10 ) ;
	WriteARMLoadMemOffsetImm12
		( ARM_r6, ARM_r10, offsetof(Context,m_ipSegment) ) ;
	WriteARMCmpRegRegShift( (ARMRegister) (regTemp + 1), ARM_r6 ) ;
	void *	pJeIPSeg = WriteARMJumpImm32( NULL, cond_EQ ) ;
	//
	WriteToAtomicOrExceptionMask( exceptionFarJump ) ;
	WriteARMStoreMemOffsetImm12
		( (ARMRegister) (regTemp + 1), ARM_r10, offsetof(Context,m_ipSegment) ) ;
	//
	CommitJumpTarget( pJeIPSeg, GetNextAddress() ) ;
	//
	WriteARMStoreMemOffsetImm12
		( (ARMRegister) regTemp, ARM_r10, offsetof(Context,m_ip) ) ;
	FreeDataRegister( regClassARM, regTemp ) ;
}

// システムコール命令
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::write_syscall_imm( int imm32 )
{
	WriteToAtomicOrExceptionMask( exceptionSystemCall ) ;
	PreserveContinuousCodes( 0x10 ) ;
	WriteARMMoveRegImm( ARM_r6, imm32 ) ;
	WriteARMStoreMemOffsetImm12
		( ARM_r6, ARM_r10, offsetof(Context,m_idSystemCall) ) ;
}

void ARMGenericAssembler::write_syscall_reg( int reg )
{
	WriteToAtomicOrExceptionMask( exceptionSystemCall ) ;
	PreserveContinuousCodes( 0x10 ) ;
	WriteToLoadSakura2Register( ARM_r6, reg, true ) ;
	WriteARMStoreMemOffsetImm12
		( ARM_r6, ARM_r10, offsetof(Context,m_idSystemCall) ) ;
}

// リターン命令（exceptionFarJump 例外の判定と設定を含む）
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::write_return( void )
{
	void *	pEscJumpFrom1 = NULL ;
	void *	pEscJumpFrom2 = NULL ;
	//
	FlushAllRegisters() ;
	ResetAllRegisters() ;
	//
	int	regTemp = AllocateDataRegister( regClassARM ) ;
	WriteToLoadSakura2Register( (ARMRegister) regTemp, regSP, true ) ;
	//
	// バウンダリチェック
	//
	WriteARMLoadMemOffsetImm12
		( (ARMRegister) (regTemp + 1),
			ARM_r10, offsetof(Context,m_segStack.baseOffset) ) ;
	WriteARMSubRegRegRegShift
		( (ARMRegister) regTemp,
			(ARMRegister) regTemp,
			(ARMRegister) (regTemp + 1), 0, cond_AL, true ) ;
	if ( !m_flagNoBoundary )
	{
		pEscJumpFrom1 = WriteARMJumpImm32( NULL, cond_CC ) ;
		WriteARMAddRegRegImm8
			( (ARMRegister) (regTemp + 1), (ARMRegister) regTemp, 8 ) ;
		PreserveContinuousCodes( 0x10 ) ;
		WriteARMLoadMemOffsetImm12
			( ARM_r6, ARM_r10, offsetof(Context,m_segStack.limitSegment) ) ;
		WriteARMCmpRegRegShift
			( (ARMRegister) (regTemp + 1), ARM_r6 ) ;
		pEscJumpFrom2 = WriteARMJumpImm32( NULL, cond_HI ) ;
	}
	else
	{
		WriteARMAddRegRegImm8
			( (ARMRegister) (regTemp + 1), (ARMRegister) regTemp, 8 ) ;
	}
	//
	// スタック POP & SP レジスタ更新
	//
	PreserveContinuousCodes( 0x10 ) ;
	WriteARMLoadMemOffsetImm12
		( ARM_r6, ARM_r10, offsetof(Context,m_segStack.pbytBuffer) ) ;
	WriteARMAddRegRegRegShift
		( (ARMRegister) regTemp, (ARMRegister) regTemp, ARM_r6 ) ;
	WriteToStoreSakura2Register
		( regSP, (ARMRegister) (regTemp + 1), true ) ;
	WriteARMLoadMemOffsetImm12
		( (ARMRegister) (regTemp + 1), (ARMRegister) regTemp, 4 ) ;
	WriteARMLoadMemOffsetImm12
		( (ARMRegister) regTemp, (ARMRegister) regTemp, 0 ) ;
	//
	// far jump 例外判定
	//
	PreserveContinuousCodes( 0x20 ) ;
	WriteARMLoadMemOffsetImm12
		( ARM_r6, ARM_r10, offsetof(Context,m_ipSegment) ) ;
	WriteARMStoreMemOffsetImm12
		( (ARMRegister) regTemp, ARM_r10, offsetof(Context,m_ip) ) ;
	WriteARMCmpRegRegShift
		( (ARMRegister) (regTemp + 1), ARM_r6 ) ;
	void *	pJneCodeFF = WriteARMJumpImm32( NULL, cond_EQ ) ;
	void *	pJneIPSeg = WriteARMJumpImm32( NULL ) ;
	//
	// バウンダリ例外
	//
	if ( !m_flagNoBoundary )
	{
		if ( pEscJumpFrom1 != NULL )
		{
			CommitJumpTarget( pEscJumpFrom1, GetNextAddress() ) ;
		}
		if ( pEscJumpFrom2 != NULL )
		{
			CommitJumpTarget( pEscJumpFrom2, GetNextAddress() ) ;
		}
		WriteToAtomicOrExceptionMask( exceptionReadMemory ) ;
		WriteToStoreSakura2Register
			( regSP, (ARMRegister) (regTemp + 1), true ) ;
	}
	void *	pElseJumpFrom = WriteToJump( NULL ) ;
	//
	// far jump 時
	//
	CommitJumpTarget( pJneIPSeg, GetNextAddress() ) ;
	WriteToAtomicOrExceptionMask( exceptionFarJump ) ;
	CommitJumpTarget( pJneCodeFF, GetNextAddress() ) ;
	WriteARMStoreMemOffsetImm12
		( (ARMRegister) (regTemp + 1), ARM_r10, offsetof(Context,m_ipSegment) ) ;
	//
	CommitJumpTarget( pElseJumpFrom, GetNextAddress() ) ;

	FreeDataRegister( regClassARM, regTemp ) ;
}

// スタック処理
//////////////////////////////////////////////////////////////////////////////
void * ARMGenericAssembler::write_push_ip( int imm32 )
{
	int	regLowSP ;
	void *	pEscJumpFrom = WriteToStackException( regLowSP, -8, NULL ) ;
	//
	// アドレス変換
	//
	WriteToStoreSakura2Register( regSP, (ARMRegister) regLowSP, true ) ;
	//
	PreserveContinuousCodes( 0x10 ) ;
	WriteARMLoadMemOffsetImm12
		( ARM_r6, ARM_r10, offsetof(Context,m_segStack.baseOffset) ) ;
	WriteARMSubRegRegRegShift
		( (ARMRegister) regLowSP, (ARMRegister) regLowSP, ARM_r6 ) ;
	PreserveContinuousCodes( 0x10 ) ;
	WriteARMLoadMemOffsetImm12
		( ARM_r6, ARM_r10, offsetof(Context,m_segStack.pbytBuffer) ) ;
	WriteARMAddRegRegRegShift
		( (ARMRegister) regLowSP, (ARMRegister) regLowSP, ARM_r6 ) ;
	//
	// PUSH
	//
	PreserveContinuousCodes( 0x10 ) ;
	WriteARMMoveRegImm( ARM_r6, imm32 ) ;
	WriteARMStoreMemOffsetImm12
		( ARM_r6, (ARMRegister) regLowSP, 0 ) ;
	PreserveContinuousCodes( 0x10 ) ;
	WriteARMLoadMemOffsetImm12
		( ARM_r6, ARM_r10, offsetof(Context,m_ipSegment) ) ;
	WriteARMStoreMemOffsetImm12
		( ARM_r6, (ARMRegister) regLowSP, 4 ) ;
	//
	FreeDataRegister( regClassARM, regLowSP ) ;
	return	pEscJumpFrom ;
}

void * ARMGenericAssembler::write_push_reg( int regFirst, int nCount )
{
	int	regLowSP ;
	FlushAllRegisters() ;
	void *	pEscJumpFrom = WriteToStackException( regLowSP, -8 * nCount, NULL ) ;
	//
	// アドレス変換
	//
	WriteToStoreSakura2Register( regSP, (ARMRegister) regLowSP, true ) ;
	//
	ARMRegister	regTemp = (ARMRegister) AllocateDataRegister( regClassARM ) ;
	ARMRegister	regTemp1 = (ARMRegister) (regTemp + 1) ;
	WriteARMLoadMemOffsetImm12
		( regTemp, ARM_r10, offsetof(Context,m_segStack.baseOffset) ) ;
	WriteARMSubRegRegRegShift
		( (ARMRegister) regLowSP, (ARMRegister) regLowSP, regTemp ) ;
	WriteARMLoadMemOffsetImm12
		( regTemp1, ARM_r10, offsetof(Context,m_segStack.pbytBuffer) ) ;
	WriteARMAddRegRegRegShift
		( (ARMRegister) regLowSP, (ARMRegister) regLowSP, regTemp1 ) ;
	//
	// PUSH
	//
	for ( int i = 0; i < nCount; i ++ )
	{
		DataRegisterClass
			classLoaded = m_lruDataReg.dprSakura[regFirst+i].regClass ;
		int	regPhyLoaded = m_lruDataReg.dprSakura[regFirst+i].regPhy ;
		if ( classLoaded == regClassARM )
		{
			WriteARMStoreMemOffsetImm12
				( (ARMRegister) regPhyLoaded,
					(ARMRegister) regLowSP, i * 8 ) ;
			WriteARMStoreMemOffsetImm12
				( (ARMRegister) (regPhyLoaded + 1),
					(ARMRegister) regLowSP, i * 8 + 4 ) ;
		}
		else
		{
			WriteToLoadSakura2Register( regTemp, regFirst + i ) ;
			WriteARMStoreMemOffsetImm12
				( regTemp, (ARMRegister) regLowSP, i * 8 ) ;
			WriteARMStoreMemOffsetImm12
				( regTemp1, (ARMRegister) regLowSP, i * 8 + 4 ) ;
		}
	}
	//
	FreeDataRegister( regClassARM, regTemp ) ;
	FreeDataRegister( regClassARM, regLowSP ) ;
	return	pEscJumpFrom ;
}

void * ARMGenericAssembler::write_pop_reg( int regFirst, int nCount )
{
	ARMRegister	regTemp = (ARMRegister) AllocateDataRegister( regClassARM ) ;
	ARMRegister	regTemp1 = (ARMRegister) (regTemp + 1) ;
	ARMRegister	regTemp2 = (ARMRegister) AllocateDataRegister( regClassARM ) ;
	ARMRegister	regTemp3 = (ARMRegister) (regTemp + 2) ;
	//
	// アドレス変換
	//
	WriteToLoadSakura2Register( regTemp2, regSP, true ) ;
	WriteARMLoadMemOffsetImm12
		( regTemp, ARM_r10, offsetof(Context,m_segStack.baseOffset) ) ;
	WriteARMSubRegRegRegShift
		( regTemp2, regTemp2, regTemp ) ;
	WriteARMLoadMemOffsetImm12
		( regTemp1, ARM_r10, offsetof(Context,m_segStack.pbytBuffer) ) ;
	WriteARMAddRegRegRegShift
		( regTemp2, regTemp2, regTemp1 ) ;
	//
	// POP
	//
	for ( int i = 0; i < nCount; i ++ )
	{
		DataRegisterClass
			classLoaded = m_lruDataReg.dprSakura[regFirst+i].regClass ;
		int	regPhyLoaded = m_lruDataReg.dprSakura[regFirst+i].regPhy ;
		if ( classLoaded == regClassARM )
		{
			WriteARMLoadMemOffsetImm12
				( (ARMRegister) regPhyLoaded,
					(ARMRegister) regTemp2, i * 8 ) ;
			WriteARMLoadMemOffsetImm12
				( (ARMRegister) (regPhyLoaded + 1),
					(ARMRegister) regTemp2, i * 8 + 4 ) ;
			SetDataRegisterModified( classLoaded, regPhyLoaded ) ;
		}
		else
		{
			WriteARMLoadMemOffsetImm12
				( regTemp, (ARMRegister) regTemp2, i * 8 ) ;
			WriteARMLoadMemOffsetImm12
				( regTemp1, (ARMRegister) regTemp2, i * 8 + 4 ) ;
			WriteToStoreSakura2Register( regFirst + i, regTemp ) ;
		}
		ModifiedRegister( regFirst + i ) ;
	}
	//
	// SP 更新
	//
	WriteToLoadSakura2Register( regTemp, regSP, true ) ;
	if ( nCount * 8 < 0xFF )
	{
		WriteARMAddRegRegImm8( regTemp, regTemp, nCount * 8 ) ;
	}
	else
	{
		WriteARMMoveRegImm( regTemp2, nCount * 8 ) ;
		WriteARMAddRegRegRegShift( regTemp, regTemp, regTemp2 ) ;
	}
	WriteToStoreSakura2Register( regSP, regTemp, true ) ;
	//
	FreeDataRegister( regClassARM, regTemp ) ;
	FreeDataRegister( regClassARM, regTemp2 ) ;
	return	NULL ;
}

// メモリヒント
//////////////////////////////////////////////////////////////////////////////
void * ARMGenericAssembler::write_prefetch_tlb( int tlb, int reg )
{
	void *	ptrEscJump = NULL ;
	bool	fPrefetchTLB = false ;
	ESLAssert( !(tlb & ~0x01) ) ;
	tlb &= 0x01 ;
	if ( m_flagNoBoundary )
	{
		if ( (m_regTLBFetched[tlb] < 0)
				&& !m_lruPointer[tlb].fTLBFetched )
		{
			m_lruPointer.NewSlot( tlb, (reg | 0xFF00) ) ;
			m_lruPointer.LockSlot( tlb ) ;
			m_lruPointer[tlb].fTLBFetched = true ;
			m_lruPointer[tlb].regPhyIndex = ARM_Nothing ;
			fPrefetchTLB = true ;
		}
	}
	ptrEscJump = Sakura2Assembler::write_prefetch_tlb( tlb, reg ) ;
	if ( fPrefetchTLB )
	{
		//
		// TLB に変換アドレスをロードし
		// 物理レジスタにベースアドレスを設定する
		//
		WritePrefetchTLB( tlb, reg ) ;
	}
	return	ptrEscJump ;
}

void ARMGenericAssembler::write_unfetch_tlb( int tlb, int reg )
{
	ESLAssert( !(tlb & ~0x01) ) ;
	tlb &= 0x01 ;
	if ( (m_regTLBFetched[tlb] >= 0) && m_lruPointer[tlb].fTLBFetched )
	{
		m_lruPointer.UnlockSlot( tlb ) ;
		m_lruPointer.FreeSlot( tlb ) ;
		m_lruPointer[tlb].fTLBFetched = false ;
		m_lruPointer[tlb].regPhyIndex = ARM_Nothing ;
	}
	Sakura2Assembler::write_unfetch_tlb( tlb, reg ) ;
}

// TLB を準備し物理レジスタにロードする
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::WritePrefetchTLB( int tlb, int reg )
{
	if ( !m_flagNoBoundary )
	{
		return ;
	}
	ARMRegister	regTemp0 = (ARMRegister) AllocateDataRegister( regClassARM ) ;
	ARMRegister	regTemp1 = (ARMRegister) (regTemp0 + 1) ;
	//
	// TLB に変換アドレスをロードし
	// 物理レジスタにベースアドレスを設定する
	//
	WriteToLoadSakura2Register( regTemp0, reg ) ;
	//
	ARMRegister	regPhyBase = (ARMRegister) m_lruPointer[tlb].regPhy ;
	ARMRegister	regPhyHigh = regTemp1 ;
	ARMRegister	regPhyLow = regTemp0 ;
	//
	PreserveContinuousCodes( 0x10 ) ;
	WriteARMLoadMemOffsetImm12
		( ARM_r6, ARM_r10,
			Context::OffsetofStoreCache_highAddress(tlb) ) ;
	WriteARMCmpRegRegShift( regPhyHigh, ARM_r6 ) ;
	ESLAssert( m_bufSub != NULL ) ;
	WriteARMJumpImm( m_bufSub->GetNext(), cond_NE ) ;
	void *	pNextAddr = GetNextAddress() ;
	m_buf = m_bufSub ;
	//
	// 第二 TLB 比較
	//
	WriteARMAndRegRegImm8( regPhyBase, regPhyHigh, 0x03 ) ;
	PreserveContinuousCodes( 0x20 ) ;
	ESLAssert( sizeof(LinearAddressCache) == (1 << 4) ) ;
	WriteARMAddRegRegRegShift( regPhyBase, ARM_r10, regPhyBase, 4 ) ;
	WriteARMLoadMemOffsetImm12
		( ARM_r6, regPhyBase, offsetof(Context,m_segLoadCache[0].highAddress) ) ;
	WriteARMCmpRegRegShift( regPhyHigh, ARM_r6 ) ;
	void *	pJeTLB2 = WriteARMJumpImm32( NULL, cond_EQ ) ;
	//
	// アドレス変換処理
	//
	WriteBackAllRegisters() ;
	//
	ARMRegister	regsPush[16] ;
	int			iPushNext = 0 ;
	if ( m_lruPointer.IsSlotAssigned( regPtrPhyR9 )
								&& (regPhyBase != ARM_r9) )
	{
		regsPush[iPushNext ++] = ARM_r9 ;
	}
	if ( m_lruPointer.IsSlotAssigned( regPtrPhyR12 )
								&& (regPhyBase != ARM_r12) )
	{
		regsPush[iPushNext ++] = ARM_r12 ;
	}
	regsPush[iPushNext ++] = regPhyLow ;
	regsPush[iPushNext ++] = regPhyHigh ;
	regsPush[iPushNext ++] = regPhyBase ;
	WriteARMPushRegs( regsPush, iPushNext ) ;
	//
	WriteARMMoveRegReg( ARM_r2, regPhyLow ) ;
	WriteARMMoveRegReg( ARM_r3, regPhyHigh ) ;
	WriteARMMoveRegReg( ARM_r0, ARM_r10 ) ;
	WriteARMAddRegRegImm
		( ARM_r1, regPhyBase,
			offsetof(Context,m_segLoadCache[0]), ARM_r6 ) ;
	//
	PreserveContinuousCodes( 0x10 ) ;
	WriteARMLoadMemOffsetImm12
		( ARM_r6, ARM_r10, offsetof(Context,m_pfnTranslate) ) ;
	WriteARMCallReg( ARM_r6 ) ;
	//
	WriteARMPopRegs( regsPush, iPushNext ) ;
	//
	ReloadRegisters() ;
	//
	// 第二 TLB 複製
	//
	CommitJumpTarget( pJeTLB2, GetNextAddress() ) ;
	//
	WriteToCopyMemory
		( true, ARM_r10, Context::OffsetofStoreCache(tlb),
			true, regPhyBase, offsetof(Context,m_segLoadCache[0]),
			sizeof(LinearAddressCache) / sizeof(DWORD), regPhyHigh ) ;
	//
	// TLB ベースアドレスをロード
	//
	WriteToJump( pNextAddr ) ;
	m_buf = m_bufMain ;
	//
	WriteARMLoadMemOffsetImm12
		( regPhyBase, ARM_r10,
			Context::OffsetofStoreCache_pbytBuffer(tlb) ) ;
	WriteARMLoadMemOffsetImm12
		( regTemp0, ARM_r10,
			Context::OffsetofStoreCache_baseOffset(tlb) ) ;
	WriteARMSubRegRegRegShift
		( regPhyBase, regPhyBase, regTemp0 ) ;
	//
	FreeDataRegister( regClassARM, regTemp0 ) ;
}

// 32ビット符号有り比較命令生成
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::write_arm_cmp_int32
	( ARMRegister regDst, ARMRegister regSrc, ARMCondition condARM )
{
	WriteARMCmpRegRegShift( regDst, regSrc, 0 ) ;
	WriteARMXorRegRegRegShift
		( regDst, regDst, regDst, 0, cond_AL, false ) ;
	WriteARMSubRegRegImm8( regDst, regDst, 1, condARM, false ) ;
}

// 32ビット浮動小数点比較移動命令生成
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::write_vfp_cmove_float32
	( int sregDst, int sregSrc, ARMCondition condARM )
{
	ARMRegister	armTemp = (ARMRegister) AllocateDataRegister( regClassARM ) ;
	//
	WriteCmpVFPRegReg( sregDst, sregSrc, false ) ;
	WriteFPSCRtoARMReg( armTemp ) ;
	WriteARMtoAPSR_nzcvq( armTemp ) ;
	WriteMoveVFP32( sregDst, sregSrc, condARM ) ;
	//
	FreeDataRegister( regClassARM, armTemp ) ;
}

// 32ビット浮動小数点比較命令生成
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::write_vfp_cmpxx_float32
	( int sregDst, int sregSrc, ARMCondition condARM )
{
	ARMRegister	armTemp = (ARMRegister) AllocateDataRegister( regClassARM ) ;
	ARMRegister	armDst = (ARMRegister) (armTemp + 1) ;
	//
	WriteCmpVFPRegReg( sregDst, sregSrc, false ) ;
	WriteFPSCRtoARMReg( armTemp ) ;
	WriteARMXorRegRegRegShift( armDst, armDst, armDst ) ;
	WriteARMtoAPSR_nzcvq( armTemp ) ;
	WriteARMSubRegRegImm8( armDst, armDst, 1, condARM ) ;
	WriteMoveARMtoVFP32( sregDst, armDst ) ;
	//
	FreeDataRegister( regClassARM, armTemp ) ;
}

// 64ビット浮動小数点比較命令生成
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::write_vfp_cmp_float64
	( int regDst, int regSrc, ARMCondition condARM )
{
	int	vregDst = WriteRealizeDataRegister( regDst, regClassVFP ) ;
	int	vregSrc = WriteRealizeDataRegister( regSrc, regClassVFP ) ;
	//
	WriteCmpVFPRegReg( vregDst, vregSrc, true ) ;
	//
	UnlockDataRegister( regClassVFP, vregSrc ) ;
	UnlockDataRegister( regClassVFP, vregDst ) ;
	//
	ARMRegister	armDst =
		(ARMRegister) WriteRealizeDataRegister( regDst, regClassARM, false ) ;
	ARMRegister	armDst1 = (ARMRegister) (armDst + 1) ;
	//
	WriteFPSCRtoARMReg( armDst1 ) ;
	WriteARMXorRegRegRegShift( armDst, armDst, armDst ) ;
	WriteARMtoAPSR_nzcvq( armDst1 ) ;
	WriteARMSubRegRegImm8( armDst, armDst, 1, condARM ) ;
	WriteARMMoveRegReg( armDst1, armDst ) ;
	//
	SetDataRegisterModified( regClassARM, armDst ) ;
	UnlockDataRegister( regClassARM, armDst ) ;
}

// 64bit 整数比較命令生成
//	regDst <- (regCmp1 > regCmp2) ^ fLogicalNot
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::write_arm_cmp_int64_gt
	( ARMRegister regDst, ARMRegister regTemp0,
		ARMRegister regCmp1, ARMRegister regCmp2,
		bool fLogicalNot, bool fUnsigned )
{
	// regDst[Low] := unsigned(regCmp1[Low] > regCmp2[Low])
	WriteARMCmpRegRegShift( regCmp1, regCmp2, 0 ) ;
	WriteARMXorRegRegRegShift
		( regDst, regDst, regDst, 0, cond_AL, false ) ;
	WriteARMSubRegRegImm8( regDst, regDst, 1, cond_HI, false ) ;
	//
	// regTemp0[Low] := signed(regCmp1[High] > regCmp2[High])
	// regTemp0[High] := regCmp1[High] == regCmp2[High]
	WriteARMXorRegRegRegShift
		( regTemp0, regTemp0, regTemp0, 0, cond_AL, false ) ;
	WriteARMXorRegRegRegShift
		( (ARMRegister) (regTemp0 + 1),
			(ARMRegister) (regTemp0 + 1),
			(ARMRegister) (regTemp0 + 1), 0, cond_AL, false ) ;
	WriteARMCmpRegRegShift
		( (ARMRegister) (regCmp1 + 1), (ARMRegister) (regCmp2 + 1), 0 ) ;
	if ( fUnsigned )
	{
		WriteARMSubRegRegImm8( regTemp0, regTemp0, 1, cond_HI, false ) ;
	}
	else
	{
		WriteARMSubRegRegImm8( regTemp0, regTemp0, 1, cond_GT, false ) ;
	}
	WriteARMSubRegRegImm8
		( (ARMRegister) (regTemp0 + 1),
				(ARMRegister) (regTemp0 + 1), 1, cond_EQ, false ) ;
	//
	// GT = signed(regCmp1[High] > regCmp2[High])
	//		|| ((regCmp1[High] == regCmp2[High])
	//			&& (unsigned(regCmp1[Low] > regCmp2[Low])))
	WriteARMAndRegRegRegShift
		( regDst, regDst, (ARMRegister) (regTemp0 + 1) ) ;
	WriteARMOrRegRegRegShift( regDst, regDst, regTemp0 ) ;
	//
	if ( fLogicalNot )
	{
		WriteARMNotRegRegShift( (ARMRegister) (regDst + 1), regDst ) ;
		WriteARMNotRegRegShift( regDst, regDst ) ;
	}
	else
	{
		WriteARMMoveRegReg( (ARMRegister) (regDst + 1), regDst ) ;
	}
}

// 64bit x 64bit -> 64bit 乗算命令生成
//////////////////////////////////////////////////////////////////////////////
void ARMGenericAssembler::write_mul_int64xint64
	( ARMRegister regDst0, ARMRegister regDst1,
		ARMRegister regSrc0, ARMRegister regSrc1,
		ARMRegister regTemp0, ARMRegister regTemp1 )
{
	WriteARMMulInt32( regTemp0, regDst0, regSrc1 ) ;
	WriteARMMulUInt64( regDst0, regTemp1, regDst0, regSrc0 ) ;
	WriteARMMulAddInt32( regDst1, regDst1, regSrc0, regTemp1 ) ;
	WriteARMAddRegRegRegShift( regDst1, regDst1, regTemp0 ) ;
}


#endif

