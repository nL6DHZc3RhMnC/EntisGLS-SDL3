
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/


#include <sakuraglx/sakuraglx.h>
#include <sakura/ssys_module.h>

#if	defined(__PLATFORM_UNIX_LIKE__)
#include <sys/mman.h>

#if	defined(__POINTER64__) && defined(__PROCESSOR_ARM__)
int cacheflush( long_ptr_t start, long_ptr_t end, long flags )
{
	__clear_cache( (void*) start, (void*) end ) ;
	return	0 ;
}
#endif
#endif

using	namespace ECSSakura2JIT ;
using	namespace ECSSakura2Processor ;


//////////////////////////////////////////////////////////////////////////////
// コードバッファ出力オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ECSSakura2JIT::CodeBuffer, ESLObject )

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
CodeBuffer::~CodeBuffer( void )
{
	if ( m_pFirstBlock != NULL )
	{
		FreeBlockList( m_pFirstBlock ) ;
	}
}

// 次の命令を出力アドレスを取得する
//////////////////////////////////////////////////////////////////////////////
void * CodeBuffer::GetNext( void )
{
	if ( m_pLastBlock == NULL )
	{
		ExtendBlock() ;
	}
	ESLAssert( m_pLastBlock != NULL ) ;
	return	m_pLastBlock->pbytBufAligned16 + m_pLastBlock->nCodeUsed ;
}

// 命令を出力する
//////////////////////////////////////////////////////////////////////////////
void CodeBuffer::WriteInstruction( const BYTE * pbytSrc, size_t nBytes )
{
	if ( m_pLastBlock == NULL )
	{
		ExtendBlock() ;
	}
	ESLAssert( m_pLastBlock != NULL ) ;
	ESLAssert( m_pLastBlock->nBufSize
			- (m_pLastBlock->nCodeUsed + m_pLastBlock->nDataUsed) > nBytes ) ;
	memmove
		( m_pLastBlock->pbytBufAligned16
			+ m_pLastBlock->nCodeUsed, pbytSrc, nBytes ) ;
	m_pLastBlock->nCodeUsed += nBytes ;
	//
	if ( m_pLastBlock->nBufSize
			- (m_pLastBlock->nCodeUsed
				+ m_pLastBlock->nDataUsed) <= m_maxInstructionSize )
	{
		ExtendBlock() ;
	}
}

// データ領域を確保する
//////////////////////////////////////////////////////////////////////////////
void * CodeBuffer::AllocateData( size_t nBytes, size_t nAlign )
{
	if ( m_pLastBlock == NULL )
	{
		ExtendBlock() ;
	}
	ESLAssert( m_pLastBlock != NULL ) ;
	ESLAssert( m_pLastBlock->nBufSize
			- (m_pLastBlock->nCodeUsed + m_pLastBlock->nDataUsed) > nBytes ) ;
	m_pLastBlock->nDataUsed += nBytes ;
	//
	size_t	modAlign = m_pLastBlock->nDataUsed % nAlign ;
	if ( modAlign > 0 )
	{
		m_pLastBlock->nDataUsed += nAlign - modAlign ;
	}
	//
	void *	pData = m_pLastBlock->pbytBufAligned16
					+ (m_pLastBlock->nBufSize - m_pLastBlock->nDataUsed) ;
	ESLAssert( (((ULONG_PTR) pData) % nAlign) == 0 ) ;
	//
	if ( m_pLastBlock->nBufSize
			- (m_pLastBlock->nCodeUsed
				+ m_pLastBlock->nDataUsed) <= m_maxInstructionSize )
	{
		ExtendBlock() ;
	}
	return	pData ;
}

// データを確定する
//////////////////////////////////////////////////////////////////////////////
void CodeBuffer::CommitAllCodes( void )
{
	Block *	pBlock = m_pFirstBlock ;
	while ( pBlock != NULL )
	{
		FlushCodeBlock( pBlock->pbytBuf, pBlock->nBufSize ) ;
		pBlock = pBlock->pNextBlock ;
	}
	#if	defined(__PLATFORM_ANDROID__)
//		flush_cache_all() ;
	#endif
}

// 実行・読み書き可能メモリの確保
//////////////////////////////////////////////////////////////////////////////
BYTE * CodeBuffer::AllocateCodeBuffer( size_t& nSize )
{
#if	defined(__PLATFORM_WINDOWS__)
	SYSTEM_INFO	sysinf ;
	::eslFillMemory( &sysinf, 0, sizeof(SYSTEM_INFO) ) ;
	::GetSystemInfo( &sysinf ) ;
	if ( sysinf.dwPageSize > 0 )
	{
		size_t	sizeMap = nSize + sysinf.dwPageSize - 1 ;
		sizeMap -= sizeMap % sysinf.dwPageSize ;
		nSize = sizeMap ;
	}
	return	(BYTE*) VirtualAlloc
				( NULL, nSize, MEM_COMMIT, PAGE_EXECUTE_READWRITE ) ;
#else
	size_t	sizePageUnit = (size_t) sysconf( _SC_PAGE_SIZE ) ;
	if ( (sizePageUnit == 0) || (sizePageUnit == (size_t) -1) )
	{
		sizePageUnit = PAGE_SIZE ;
	}
	size_t	sizeMap = nSize + sizePageUnit - 1 ;
	sizeMap -= sizeMap % sizePageUnit ;
	void *	ptrMapped =
		mmap( NULL, sizeMap,
			PROT_READ | PROT_WRITE | PROT_EXEC,
			MAP_PRIVATE | MAP_ANONYMOUS, -1, 0) ;
	if ( ptrMapped == MAP_FAILED )
	{
		return NULL ;
	}
	nSize = sizeMap ;
	return	(BYTE*) ptrMapped ;
#endif
}

// 実行・読み書き可能メモリの解放
//////////////////////////////////////////////////////////////////////////////
void CodeBuffer::FreeCodeBuffer( BYTE * pbytBuf, size_t nSize )
{
#if	defined(__PLATFORM_WINDOWS__)
	VirtualFree( pbytBuf, 0, MEM_RELEASE ) ;
#else
	munmap( pbytBuf, nSize ) ;
#endif
}

// 実行メモリの内容確定
//////////////////////////////////////////////////////////////////////////////
void CodeBuffer::FlushCodeBlock( BYTE * pbytBuf, size_t nSize )
{
#if	defined(__PLATFORM_WINDOWS__)

#else
	#if	defined(__PLATFORM_ANDROID__)
		#if	defined(__PROCESSOR_ARM__) || defined(__PROCESSOR_MIPS__)
			ESLVerify
				( cacheflush
					( (long_ptr_t) pbytBuf,
						(long_ptr_t) (pbytBuf + nSize), 0 ) == 0 ) ;
			#if	!defined(ICACHE)
			enum
			{
				ICACHE	= (1<<0),
				DCACHE	= (1<<1),
				BCACHE	= (ICACHE|DCACHE),
			} ;
			#endif
			ESLVerify
				( cacheflush
					( (long_ptr_t) pbytBuf,
						(long_ptr_t) (pbytBuf + nSize), ICACHE | DCACHE ) == 0 ) ;
		#endif
	#endif
	ESLVerify( mprotect( pbytBuf, nSize, PROT_READ | PROT_EXEC ) == 0 ) ;
	ESLVerify( msync( pbytBuf, nSize, MS_SYNC ) == 0 ) ;
#endif
}

// メモリブロック確保
//////////////////////////////////////////////////////////////////////////////
CodeBuffer::Block * CodeBuffer::NewBlock( void )
{
	Block *	pBlock = new Block ;
	pBlock->pNextBlock = NULL ;
	pBlock->nBufAllocSize = m_bytesBlockUnit ;
	pBlock->pbytBuf = AllocateCodeBuffer( pBlock->nBufAllocSize ) ;
	pBlock->nBufSize = pBlock->nBufAllocSize ;
	pBlock->pbytBufAligned16 = pBlock->pbytBuf ;
	//
	if ( ((ULONG_PTR) pBlock->pbytBufAligned16) & 0x0F )
	{
		ULONG_PTR	ulOffset =
			0x10 - (((ULONG_PTR) pBlock->pbytBufAligned16) & 0x0F) ;
		pBlock->pbytBufAligned16 += ulOffset ;
		pBlock->nBufSize -= (size_t) ulOffset ;
	}
	pBlock->nCodeUsed = 0 ;
	pBlock->nDataUsed = 0 ;
	return	pBlock ;
}

// メモリブロックリスト解放
//////////////////////////////////////////////////////////////////////////////
void CodeBuffer::FreeBlockList( Block * pBlock )
{
	while ( pBlock != NULL )
	{
		Block * pNext = pBlock->pNextBlock ;
		FreeCodeBuffer( pBlock->pbytBuf, pBlock->nBufAllocSize ) ;
		delete	pBlock ;
		pBlock = pNext ;
	}
}

// バッファの残りが少なくなった場合追加する
//////////////////////////////////////////////////////////////////////////////
void CodeBuffer::ExtendBlock( void )
{
	if ( m_pFirstBlock == NULL )
	{
		m_pLastBlock = m_pFirstBlock = NewBlock() ;
	}
	else
	{
		Block *	pLast = m_pLastBlock ;
		//
		m_pLastBlock = NewBlock() ;
		pLast->pNextBlock = m_pLastBlock ;
		//
		WriteJump( pLast, m_pLastBlock->pbytBufAligned16 ) ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// 汎用演算関数
//////////////////////////////////////////////////////////////////////////////

const GENERIC_IMPLEMENTS	ECSSakura2JIT::giDefGenericImplements =
{
	// ムーブ・変換演算
	gen_move_reg_reg,
	gen_maskmove_reg_reg_reg,
	gen_cvt_float2int,
	gen_cvt_int2float,
	// 即値シフト演算
	gen_srl_reg_reg_imm8,
	gen_sra_reg_reg_imm8,
	gen_sll_reg_reg_imm8,
	// 即値加算・乗算
	gen_add_reg_reg_imm32,
	gen_mul_reg_reg_imm32,
	// 単項演算
	gen_neg_int,
	gen_not_int,
	gen_neg_float,
	// 整数演算
	gen_add_reg_reg,
	gen_sub_reg_reg,
	gen_mul_reg_reg,
	gen_div_reg_reg,
	gen_mod_reg_reg,
	gen_and_reg_reg,
	gen_or_reg_reg,
	gen_xor_reg_reg,
	gen_srl_reg_reg,
	gen_sra_reg_reg,
	gen_sll_reg_reg,
	// 整数符号拡張
	gen_move_sx32_reg_reg,
	gen_move_sx16_reg_reg,
	gen_move_sx8_reg_reg,
	// 実数演算
	gen_fadd_reg_reg,
	gen_fsub_reg_reg,
	gen_fmul_reg_reg,
	gen_fdiv_reg_reg,
	// 特殊演算精度演算
	gen_mul32_reg_reg,
	gen_imul32_reg_reg,
	gen_div32_reg_reg,
	gen_idiv32_reg_reg,
	gen_mod32_reg_reg,
	gen_imod32_reg_reg,
	// 整数比較
	gen_cmp_ne,
	gen_cmp_eq,
	gen_cmp_lt,
	gen_cmp_le,
	gen_cmp_gt,
	gen_cmp_ge,
	gen_cmp_c,
	gen_cmp_cz,
	// 実数比較
	gen_fcmp_ne,
	gen_fcmp_eq,
	gen_fcmp_lt,
	gen_fcmp_le,
	gen_fcmp_gt,
	gen_fcmp_ge,
} ;

// ムーブ・変換演算
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2JIT::gen_move_reg_reg
		( Register * dst, const Register * src )
{
	dst->i = src->i ;
}

void __fastcall ECSSakura2JIT::gen_maskmove_reg_reg_reg
		( Register * dst, const Register * src, const Register * src2 )
{
	INT64	mask = src2->i ;
	dst->i = (dst->i & ~mask) | (src->i & mask) ;
}

void __fastcall ECSSakura2JIT::gen_cvt_float2int
		( Register * dst, const Register * src )
{
	dst->i = eslRoundR64ToLInt( src->f ) ;
}

void __fastcall ECSSakura2JIT::gen_cvt_int2float
		( Register * dst, const Register * src )
{
	dst->f = (REAL64) src->i ;
}

// 即値シフト演算
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2JIT::gen_srl_reg_reg_imm8
		( Register * dst, const Register * src, int imm )
{
	dst->i = ((UINT64) src->i) >> (imm & 0x3F) ;
}

void __fastcall ECSSakura2JIT::gen_sra_reg_reg_imm8
		( Register * dst, const Register * src, int imm )
{
	dst->i = ((INT64) src->i) >> (imm & 0x3F) ;
}

void __fastcall ECSSakura2JIT::gen_sll_reg_reg_imm8
		( Register * dst, const Register * src, int imm )
{
	dst->i = ((INT64) src->i) << (imm & 0x3F) ;
}

// 即値加算・乗算
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2JIT::gen_add_reg_reg_imm32
		( Register * dst, const Register * src, int imm )
{
	dst->i = src->i + imm ;
}

void __fastcall ECSSakura2JIT::gen_mul_reg_reg_imm32
		( Register * dst, const Register * src, int imm )
{
	dst->i = src->i * imm ;
}

// 単項演算
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2JIT::gen_neg_int
		( Register * dst, const Register * src )
{
	dst->i = - src->i ;
}

void __fastcall ECSSakura2JIT::gen_not_int
		( Register * dst, const Register * src )
{
	dst->i = ~src->i ;
}

void __fastcall ECSSakura2JIT::gen_neg_float
		( Register * dst, const Register * src )
{
	dst->f = - src->f ;
}

// 整数演算
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2JIT::gen_add_reg_reg
		( Register * dst, const Register * src )
{
	dst->i += src->i ;
}

void __fastcall ECSSakura2JIT::gen_sub_reg_reg
		( Register * dst, const Register * src )
{
	dst->i -= src->i ;
}

void __fastcall ECSSakura2JIT::gen_mul_reg_reg
		( Register * dst, const Register * src )
{
	dst->i *= src->i ;
}

void __fastcall ECSSakura2JIT::gen_div_reg_reg
		( Register * dst, const Register * src )
{
	dst->i /= src->i ;
}

void __fastcall ECSSakura2JIT::gen_mod_reg_reg
		( Register * dst, const Register * src )
{
	dst->i %= src->i ;
}

void __fastcall ECSSakura2JIT::gen_and_reg_reg
		( Register * dst, const Register * src )
{
	dst->i &= src->i ;
}

void __fastcall ECSSakura2JIT::gen_or_reg_reg
		( Register * dst, const Register * src )
{
	dst->i |= src->i ;
}

void __fastcall ECSSakura2JIT::gen_xor_reg_reg
		( Register * dst, const Register * src )
{
	dst->i ^= src->i ;
}

void __fastcall ECSSakura2JIT::gen_srl_reg_reg
		( Register * dst, const Register * src )
{
	dst->i = ((UINT64)dst->i) >> (src->l32 & 0x3F) ;
}

void __fastcall ECSSakura2JIT::gen_sra_reg_reg
		( Register * dst, const Register * src )
{
	dst->i = ((INT64)dst->i) >> (src->l32 & 0x3F) ;
}

void __fastcall ECSSakura2JIT::gen_sll_reg_reg
		( Register * dst, const Register * src )
{
	dst->i = ((INT64)dst->i) << (src->l32 & 0x3F) ;
}

// 整数符号拡張
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2JIT::gen_move_sx32_reg_reg
		( Register * dst, const Register * src )
{
	dst->i = src->li32 ;
}

void __fastcall ECSSakura2JIT::gen_move_sx16_reg_reg
		( Register * dst, const Register * src )
{
	dst->i = (SWORD) src->li32 ;
}

void __fastcall ECSSakura2JIT::gen_move_sx8_reg_reg
		( Register * dst, const Register * src )
{
	dst->i = (SBYTE) src->li32 ;
}

// 実数演算
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2JIT::gen_fadd_reg_reg
		( Register * dst, const Register * src )
{
	dst->f += src->f ;
}

void __fastcall ECSSakura2JIT::gen_fsub_reg_reg
		( Register * dst, const Register * src )
{
	dst->f -= src->f ;
}

void __fastcall ECSSakura2JIT::gen_fmul_reg_reg
		( Register * dst, const Register * src )
{
	dst->f *= src->f ;
}

void __fastcall ECSSakura2JIT::gen_fdiv_reg_reg
		( Register * dst, const Register * src )
{
	dst->f /= src->f ;
}

// 特殊演算精度演算
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2JIT::gen_mul32_reg_reg
		( Register * dst, const Register * src )
{
	dst->i = (UINT64) dst->l32 * src->l32 ;
}

void __fastcall ECSSakura2JIT::gen_imul32_reg_reg
		( Register * dst, const Register * src )
{
	dst->i = (INT64) dst->li32 * src->li32 ;
}

void __fastcall ECSSakura2JIT::gen_div32_reg_reg
		( Register * dst, const Register * src )
{
	dst->i = ((UINT64) dst->i) / src->l32 ;
}

void __fastcall ECSSakura2JIT::gen_idiv32_reg_reg
		( Register * dst, const Register * src )
{
	dst->i = ((INT64) dst->i) / src->li32 ;
}

void __fastcall ECSSakura2JIT::gen_mod32_reg_reg
		( Register * dst, const Register * src )
{
	dst->i = ((UINT64) dst->i) % src->l32 ;
}

void __fastcall ECSSakura2JIT::gen_imod32_reg_reg
		( Register * dst, const Register * src )
{
	dst->i = ((INT64) dst->i) % src->li32 ;
}

// 整数比較
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2JIT::gen_cmp_ne
		( Register * dst, const Register * src )
{
	dst->i = - (int) (dst->i != src->i) ;
}

void __fastcall ECSSakura2JIT::gen_cmp_eq
		( Register * dst, const Register * src )
{
	dst->i = - (int) (dst->i == src->i) ;
}

void __fastcall ECSSakura2JIT::gen_cmp_lt
		( Register * dst, const Register * src )
{
	dst->i = - (int) (dst->i < src->i) ;
}

void __fastcall ECSSakura2JIT::gen_cmp_le
		( Register * dst, const Register * src )
{
	dst->i = - (int) (dst->i <= src->i) ;
}

void __fastcall ECSSakura2JIT::gen_cmp_gt
		( Register * dst, const Register * src )
{
	dst->i = - (int) (dst->i > src->i) ;
}

void __fastcall ECSSakura2JIT::gen_cmp_ge
		( Register * dst, const Register * src )
{
	dst->i = - (int) (dst->i >= src->i) ;
}

void __fastcall ECSSakura2JIT::gen_cmp_c
		( Register * dst, const Register * src )
{
	dst->i = - (int) ((UINT64) dst->i < (UINT64) src->i) ;
}

void __fastcall ECSSakura2JIT::gen_cmp_cz
		( Register * dst, const Register * src )
{
	dst->i = - (int) ((UINT64) dst->i <= (UINT64) src->i) ;
}

// 実数比較
//////////////////////////////////////////////////////////////////////////////
void __fastcall ECSSakura2JIT::gen_fcmp_ne
		( Register * dst, const Register * src )
{
	dst->i = - (int) (dst->f != src->f) ;
}

void __fastcall ECSSakura2JIT::gen_fcmp_eq
		( Register * dst, const Register * src )
{
	dst->i = - (int) (dst->f == src->f) ;
}

void __fastcall ECSSakura2JIT::gen_fcmp_lt
		( Register * dst, const Register * src )
{
	dst->i = - (int) (dst->f < src->f) ;
}

void __fastcall ECSSakura2JIT::gen_fcmp_le
		( Register * dst, const Register * src )
{
	dst->i = - (int) (dst->f <= src->f) ;
}

void __fastcall ECSSakura2JIT::gen_fcmp_gt
		( Register * dst, const Register * src )
{
	dst->i = - (int) (dst->f > src->f) ;
}

void __fastcall ECSSakura2JIT::gen_fcmp_ge
		( Register * dst, const Register * src )
{
	dst->i = - (int) (dst->f >= src->f) ;
}


//////////////////////////////////////////////////////////////////////////////
// Sakura2 processor ネイティブコード化抽象アセンブラ
//////////////////////////////////////////////////////////////////////////////

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ECSSakura2JIT::Sakura2Assembler::~Sakura2Assembler( void )
{
}

// 全てのコード出力を完了し確定する
//////////////////////////////////////////////////////////////////////////////
void ECSSakura2JIT::Sakura2Assembler::CommitAllCodes( void )
{
	if ( m_bufMain )
	{
		m_bufMain->CommitAllCodes() ;
	}
	if ( m_bufSub )
	{
		m_bufSub->CommitAllCodes() ;
	}
}

// デバッグ用（関数コンパイル開始時の処理）
//////////////////////////////////////////////////////////////////////////////
void ECSSakura2JIT::Sakura2Assembler::OnDebugBeforeFunction( DWORD dwFuncAddr, DWORD dwSize )
{
}

// デバッグ用（関数コンパイル終了時の処理）
//////////////////////////////////////////////////////////////////////////////
void ECSSakura2JIT::Sakura2Assembler::OnDebugAfterFunction( DWORD dwFuncAddr, DWORD dwSize )
{
}

// デバッグ用（命令コンパイル前の処理）
//////////////////////////////////////////////////////////////////////////////
void ECSSakura2JIT::Sakura2Assembler::OnDebugBeforeInstruction( BYTE bytInst, DWORD ip )
{
}

// デバッグ用（命令コンパイル後のテスト）
//////////////////////////////////////////////////////////////////////////////
void ECSSakura2JIT::Sakura2Assembler::OnDebugAfterInstruction( BYTE bytInst, DWORD ip )
{
}

// ローカル変数用フレームポインタ割り当て処理コード出力
//////////////////////////////////////////////////////////////////////////////
void * ECSSakura2JIT::Sakura2Assembler::WriteAssignLocalBasePointer
	( int offsetFirst, int offsetEnd, const void * ptrEpilogue )
{
	RealizePointerBoundary	rpb ;
	rpb.offsetFirst = offsetFirst ;
	rpb.offsetEnd = offsetEnd ;
	//
	const int	regPhyBP = GetFramePointerPhysicalRegister() ;
	void *	pEscJumpFrom =
		WriteRealizePointerRegister( regPhyBP, regBP, rpb, ptrEpilogue ) ;
	CommitRealizePointerRegister( rpb, offsetFirst, offsetEnd ) ;
	//
	return	pEscJumpFrom ;
}

// メモリ読み込み処理コード出力
//////////////////////////////////////////////////////////////////////////////
void * ECSSakura2JIT::Sakura2Assembler::WriteToLoadMemory
	( int regDst, int regPtr,
		int regIndex, int scale,
		int offset, DataType type, bool fPair )
{
	const int	sizeOfData = sizeof_prim_data[type] ;
	int	sizeOfLoad = sizeOfData ;
	if ( fPair && (type == dataInt64) )
	{
		sizeOfLoad *= 2 ;
	}
	void *	ptrEscJumpFrom = NULL ;
	int	regPhyPtr =
		WriteAssignPointerRegister
			( regPtr, regIndex, scale,
				offset, offset + sizeOfLoad, ptrEscJumpFrom ) ;
//	if ( !fPair || (type == dataInt64) )
	{
		WriteToLoadPhysicalMemory
			( regDst, regPhyPtr, offset, type, fPair ) ;
	}
/*	else
	{
		WriteToLoadPhysicalMemory
			( regDst, regPhyPtr, offset, type, false ) ;
		if ( fPair )
		{
			WriteToLoadPhysicalMemory
				( regDst + 1, regPhyPtr, offset + sizeOfData, type, false ) ;
		}
	}
*/	return	ptrEscJumpFrom ;
}

// メモリ読み込み処理コード出力
//////////////////////////////////////////////////////////////////////////////
void * ECSSakura2JIT::Sakura2Assembler::WriteToStoreMemory
	( int regSrc, int regPtr,
		int regIndex, int scale,
		int offset, DataType type, bool fPair )
{
	const int	sizeOfData = sizeof_prim_data[type] ;
	int	sizeOfLoad = sizeOfData ;
	if ( fPair && (type == dataInt64) )
	{
		sizeOfLoad *= 2 ;
	}
	void *	ptrEscJumpFrom = NULL ;
	int	regPhyPtr =
		WriteAssignPointerRegister
			( regPtr, regIndex, scale,
				offset, offset + sizeOfLoad, ptrEscJumpFrom ) ;
//	if ( !fPair || (type == dataInt64) )
	{
		WriteToStorePhysicalMemory
			( regSrc, regPhyPtr, offset, type, fPair ) ;
	}
/*	else
	{
		WriteToStorePhysicalMemory
			( regSrc, regPhyPtr, offset, type, false ) ;
		if ( fPair )
		{
			WriteToStorePhysicalMemory
				( regSrc + 1, regPhyPtr, offset + sizeOfData, type, false ) ;
		}
	}
*/	return	ptrEscJumpFrom ;
}

// メモリアクセス命令
//////////////////////////////////////////////////////////////////////////////
void * ECSSakura2JIT::Sakura2Assembler::write_load_memory
	( const void * pEscCode, AddressingMode mode, DataType type,
		int regDst, int regBase, int regIndex, int scale, int offset, bool fPair )
{
	switch ( mode )
	{
	case	addrBase:
		regIndex = -1 ;
		scale = 0 ;
		offset = 0 ;
		break ;
	case	addrBaseOffset32:
		regIndex = -1 ;
		scale = 0 ;
		break ;
	case	addrBaseIndex:
		offset = 0 ;
		break ;
	case	addrBaseIndexOffset32:
		break ;
	}
	void *	pEscJumpFrom =
		WriteToLoadMemory
			( regDst, regBase, regIndex, scale, offset, type, fPair ) ;
	if ( (pEscJumpFrom != NULL) && (pEscCode != NULL) )
	{
		CommitJumpTarget( pEscJumpFrom, pEscCode ) ;
	}
	return	pEscJumpFrom ;
}

void * ECSSakura2JIT::Sakura2Assembler::write_store_memory
	( const void * pEscCode, AddressingMode mode, DataType type,
		int regSrc, int regBase, int regIndex, int scale, int offset, bool fPair )
{
	switch ( mode )
	{
	case	addrBase:
		regIndex = -1 ;
		scale = 0 ;
		offset = 0 ;
		break ;
	case	addrBaseOffset32:
		regIndex = -1 ;
		scale = 0 ;
		break ;
	case	addrBaseIndex:
		offset = 0 ;
		break ;
	case	addrBaseIndexOffset32:
		break ;
	}
	void *	pEscJumpFrom =
		WriteToStoreMemory
			( regSrc, regBase, regIndex, scale, offset, type, fPair ) ;
	if ( (pEscJumpFrom != NULL) && (pEscCode != NULL) )
	{
		CommitJumpTarget( pEscJumpFrom, pEscCode ) ;
	}
	return	pEscJumpFrom ;
}

void * ECSSakura2JIT::Sakura2Assembler::write_load_local
	( const void * pEscCode, LocalAddressingMode mode, DataType type,
		int regDst, int regIndex, int scale, int offset, bool fPair )
{
	switch ( mode )
	{
	case	addrLocalOffset32:
		regIndex = -1 ;
		scale = 0 ;
		break ;
	case	addrLocalIndexOffset32:
		break ;
	}
	void *	pEscJumpFrom =
		WriteToLoadMemory
			( regDst, regBP, regIndex, scale, offset, type, fPair ) ;
	if ( (pEscJumpFrom != NULL) && (pEscCode != NULL) )
	{
		CommitJumpTarget( pEscJumpFrom, pEscCode ) ;
	}
	return	pEscJumpFrom ;
}

void * ECSSakura2JIT::Sakura2Assembler::write_store_local
	( const void * pEscCode, LocalAddressingMode mode, DataType type,
		int regSrc, int regIndex, int scale, int offset, bool fPair )
{
	switch ( mode )
	{
	case	addrLocalOffset32:
		regIndex = -1 ;
		scale = 0 ;
		break ;
	case	addrLocalIndexOffset32:
		break ;
	}
	void *	pEscJumpFrom =
		WriteToStoreMemory
			( regSrc, regBP, regIndex, scale, offset, type, fPair ) ;
	if ( (pEscJumpFrom != NULL) && (pEscCode != NULL) )
	{
		CommitJumpTarget( pEscJumpFrom, pEscCode ) ;
	}
	return	pEscJumpFrom ;
}

// データ移動命令
//////////////////////////////////////////////////////////////////////////////
void ECSSakura2JIT::Sakura2Assembler::write_move_reg_reg
	( int regDst, int regSrc, bool fPair )
{
	FlushAllRegisters() ;
	WriteToCallInstructionRegReg( m_gi->move_reg_reg, regDst, regSrc ) ;
	if ( fPair )
	{
		WriteToCallInstructionRegReg( m_gi->move_reg_reg, regDst + 1, regSrc + 1 ) ;
	}
}

void ECSSakura2JIT::Sakura2Assembler::write_maskmove_reg_reg_reg
	( int regDst, int regSrc, int regSrc2, bool fPair )
{
	FlushAllRegisters() ;
	WriteToCallInstructionRegRegReg
		( m_gi->maskmove_reg_reg_reg, regDst, regSrc, regSrc2 ) ;
	if ( fPair )
	{
		WriteToCallInstructionRegRegReg
			( m_gi->maskmove_reg_reg_reg, regDst + 1, regSrc + 1, regSrc2 + 1 ) ;
	}
}

// 整数・実数変換命令
//////////////////////////////////////////////////////////////////////////////
void ECSSakura2JIT::Sakura2Assembler::write_cvt_float2int
	( int regDst, int regSrc )
{
	FlushAllRegisters() ;
	WriteToCallInstructionRegReg( m_gi->cvt_float2int, regDst, regSrc ) ;
}

void ECSSakura2JIT::Sakura2Assembler::write_cvt_int2float
	( int regDst, int regSrc )
{
	FlushAllRegisters() ;
	WriteToCallInstructionRegReg( m_gi->cvt_int2float, regDst, regSrc ) ;
}

// シフト命令
//////////////////////////////////////////////////////////////////////////////
void ECSSakura2JIT::Sakura2Assembler::write_srl_reg_reg_imm8
	( int regDst, int regSrc, int imm8, bool fPair )
{
	FlushAllRegisters() ;
	WriteToCallInstructionRegRegImm
		( m_gi->srl_reg_reg_imm8, regDst, regSrc, imm8 ) ;
	if ( fPair )
	{
		WriteToCallInstructionRegRegImm
			( m_gi->srl_reg_reg_imm8, regDst + 1, regSrc + 1, imm8 ) ;
	}
}

void ECSSakura2JIT::Sakura2Assembler::write_sra_reg_reg_imm8
	( int regDst, int regSrc, int imm8, bool fPair )
{
	FlushAllRegisters() ;
	WriteToCallInstructionRegRegImm
		( m_gi->sra_reg_reg_imm8, regDst, regSrc, imm8 ) ;
	if ( fPair )
	{
		WriteToCallInstructionRegRegImm
			( m_gi->sra_reg_reg_imm8, regDst + 1, regSrc + 1, imm8 ) ;
	}
}

void ECSSakura2JIT::Sakura2Assembler::write_sll_reg_reg_imm8
	( int regDst, int regSrc, int imm8, bool fPair )
{
	FlushAllRegisters() ;
	WriteToCallInstructionRegRegImm
		( m_gi->sll_reg_reg_imm8, regDst, regSrc, imm8 ) ;
	if ( fPair )
	{
		WriteToCallInstructionRegRegImm
			( m_gi->sll_reg_reg_imm8, regDst + 1, regSrc + 1, imm8 ) ;
	}
}

// 32ビット即値命令
//////////////////////////////////////////////////////////////////////////////
void ECSSakura2JIT::Sakura2Assembler::write_add_reg_reg_imm32
	( int regDst, int regSrc, int imm32, bool fPair )
{
	FlushAllRegisters() ;
	WriteToCallInstructionRegRegImm
		( m_gi->add_reg_reg_imm32, regDst, regSrc, imm32 ) ;
	if ( fPair )
	{
		WriteToCallInstructionRegRegImm
			( m_gi->add_reg_reg_imm32, regDst + 1, regSrc + 1, imm32 ) ;
	}
}

void ECSSakura2JIT::Sakura2Assembler::write_mul_reg_reg_imm32
	( int regDst, int regSrc, int imm32, bool fPair )
{
	FlushAllRegisters() ;
	WriteToCallInstructionRegRegImm
		( m_gi->mul_reg_reg_imm32, regDst, regSrc, imm32 ) ;
	if ( fPair )
	{
		WriteToCallInstructionRegRegImm
			( m_gi->mul_reg_reg_imm32, regDst + 1, regSrc + 1, imm32 ) ;
	}
}

// スタックレジスタ加算命令
//////////////////////////////////////////////////////////////////////////////
void * ECSSakura2JIT::Sakura2Assembler::write_add_sp_imm32( int ip, int imm32 )
{
	FlushAllRegisters() ;
	WriteToCallInstructionRegRegImm
		( m_gi->add_reg_reg_imm32, regSP, regSP, imm32 ) ;
	if ( imm32 < 0 )
	{
		int	regPhy ;
		return	WriteToStackException( regPhy, 0, NULL ) ;
	}
	else
	{
		return	NULL ;
	}
}

// 1 OP 演算命令
//////////////////////////////////////////////////////////////////////////////
void ECSSakura2JIT::Sakura2Assembler::write_neg_int( int regDst )
{
	FlushAllRegisters() ;
	WriteToCallInstructionRegReg( m_gi->neg_int, regDst, regDst ) ;
}

void ECSSakura2JIT::Sakura2Assembler::write_not_int( int regDst )
{
	FlushAllRegisters() ;
	WriteToCallInstructionRegReg( m_gi->not_int, regDst, regDst ) ;
}

void ECSSakura2JIT::Sakura2Assembler::write_neg_float( int regDst )
{
	FlushAllRegisters() ;
	WriteToCallInstructionRegReg( m_gi->neg_float, regDst, regDst ) ;
}

// 2 OP 整数演算命令
//////////////////////////////////////////////////////////////////////////////
void ECSSakura2JIT::Sakura2Assembler::write_add_reg_reg
	( int regDst, int regSrc, bool fPair )
{
	FlushAllRegisters() ;
	WriteToCallInstructionRegReg( m_gi->add_reg_reg, regDst, regSrc ) ;
	if ( fPair )
	{
		WriteToCallInstructionRegReg
				( m_gi->add_reg_reg, regDst + 1, regSrc + 1 ) ;
	}
}

void ECSSakura2JIT::Sakura2Assembler::write_sub_reg_reg
	( int regDst, int regSrc, bool fPair )
{
	FlushAllRegisters() ;
	WriteToCallInstructionRegReg( m_gi->sub_reg_reg, regDst, regSrc ) ;
	if ( fPair )
	{
		WriteToCallInstructionRegReg
				( m_gi->sub_reg_reg, regDst + 1, regSrc + 1 ) ;
	}
}

void ECSSakura2JIT::Sakura2Assembler::write_mul_reg_reg
	( int regDst, int regSrc, bool fPair )
{
	FlushAllRegisters() ;
	WriteToCallInstructionRegReg( m_gi->mul_reg_reg, regDst, regSrc ) ;
	if ( fPair )
	{
		WriteToCallInstructionRegReg
				( m_gi->mul_reg_reg, regDst + 1, regSrc + 1 ) ;
	}
}

void ECSSakura2JIT::Sakura2Assembler::write_div_reg_reg
	( const void * pEscCode, int regDst, int regSrc )
{
	FlushAllRegisters() ;
	WriteToZeroDivisionException( regSrc, pEscCode ) ;
	WriteToCallInstructionRegReg( m_gi->div_reg_reg, regDst, regSrc ) ;
}

void ECSSakura2JIT::Sakura2Assembler::write_mod_reg_reg
	( const void * pEscCode, int regDst, int regSrc )
{
	FlushAllRegisters() ;
	WriteToZeroDivisionException( regSrc, pEscCode ) ;
	WriteToCallInstructionRegReg( m_gi->mod_reg_reg, regDst, regSrc ) ;
}

void ECSSakura2JIT::Sakura2Assembler::write_and_reg_reg
	( int regDst, int regSrc, bool fPair )
{
	FlushAllRegisters() ;
	WriteToCallInstructionRegReg( m_gi->and_reg_reg, regDst, regSrc ) ;
	if ( fPair )
	{
		WriteToCallInstructionRegReg
				( m_gi->and_reg_reg, regDst + 1, regSrc + 1 ) ;
	}
}

void ECSSakura2JIT::Sakura2Assembler::write_or_reg_reg
	( int regDst, int regSrc, bool fPair )
{
	FlushAllRegisters() ;
	WriteToCallInstructionRegReg( m_gi->or_reg_reg, regDst, regSrc ) ;
	if ( fPair )
	{
		WriteToCallInstructionRegReg
				( m_gi->or_reg_reg, regDst + 1, regSrc + 1 ) ;
	}
}

void ECSSakura2JIT::Sakura2Assembler::write_xor_reg_reg
	( int regDst, int regSrc, bool fPair )
{
	FlushAllRegisters() ;
	WriteToCallInstructionRegReg( m_gi->xor_reg_reg, regDst, regSrc ) ;
	if ( fPair )
	{
		WriteToCallInstructionRegReg
				( m_gi->xor_reg_reg, regDst + 1, regSrc + 1 ) ;
	}
}

void ECSSakura2JIT::Sakura2Assembler::write_srl_reg_reg
	( int regDst, int regSrc, bool fPair )
{
	FlushAllRegisters() ;
	WriteToCallInstructionRegReg( m_gi->srl_reg_reg, regDst, regSrc ) ;
	if ( fPair )
	{
		WriteToCallInstructionRegReg
				( m_gi->srl_reg_reg, regDst + 1, regSrc ) ;
	}
}

void ECSSakura2JIT::Sakura2Assembler::write_sra_reg_reg
	( int regDst, int regSrc, bool fPair )
{
	FlushAllRegisters() ;
	WriteToCallInstructionRegReg( m_gi->sra_reg_reg, regDst, regSrc ) ;
	if ( fPair )
	{
		WriteToCallInstructionRegReg
				( m_gi->sra_reg_reg, regDst + 1, regSrc ) ;
	}
}

void ECSSakura2JIT::Sakura2Assembler::write_sll_reg_reg
	( int regDst, int regSrc, bool fPair )
{
	FlushAllRegisters() ;
	WriteToCallInstructionRegReg( m_gi->sll_reg_reg, regDst, regSrc ) ;
	if ( fPair )
	{
		WriteToCallInstructionRegReg
				( m_gi->sll_reg_reg, regDst + 1, regSrc ) ;
	}
}

// 整数符号拡張命令
//////////////////////////////////////////////////////////////////////////////
void ECSSakura2JIT::Sakura2Assembler::write_move_sx32_reg_reg( int regDst, int regSrc )
{
	FlushAllRegisters() ;
	WriteToCallInstructionRegReg( m_gi->move_sx32_reg_reg, regDst, regSrc ) ;
}

void ECSSakura2JIT::Sakura2Assembler::write_move_sx16_reg_reg( int regDst, int regSrc )
{
	FlushAllRegisters() ;
	WriteToCallInstructionRegReg( m_gi->move_sx16_reg_reg, regDst, regSrc ) ;
}

void ECSSakura2JIT::Sakura2Assembler::write_move_sx8_reg_reg( int regDst, int regSrc )
{
	FlushAllRegisters() ;
	WriteToCallInstructionRegReg( m_gi->move_sx8_reg_reg, regDst, regSrc ) ;
}

// 2 OP 実数演算命令
//////////////////////////////////////////////////////////////////////////////
void ECSSakura2JIT::Sakura2Assembler::write_fadd_reg_reg
	( int regDst, int regSrc, bool fPair )
{
	FlushAllRegisters() ;
	WriteToCallInstructionRegReg( m_gi->fadd_reg_reg, regDst, regSrc ) ;
	if ( fPair )
	{
		WriteToCallInstructionRegReg
				( m_gi->fadd_reg_reg, regDst + 1, regSrc + 1 ) ;
	}
}

void ECSSakura2JIT::Sakura2Assembler::write_fsub_reg_reg
	( int regDst, int regSrc, bool fPair )
{
	FlushAllRegisters() ;
	WriteToCallInstructionRegReg( m_gi->fsub_reg_reg, regDst, regSrc ) ;
	if ( fPair )
	{
		WriteToCallInstructionRegReg
				( m_gi->fsub_reg_reg, regDst + 1, regSrc + 1 ) ;
	}
}

void ECSSakura2JIT::Sakura2Assembler::write_fmul_reg_reg
	( int regDst, int regSrc, bool fPair )
{
	FlushAllRegisters() ;
	WriteToCallInstructionRegReg( m_gi->fmul_reg_reg, regDst, regSrc ) ;
	if ( fPair )
	{
		WriteToCallInstructionRegReg
				( m_gi->fmul_reg_reg, regDst + 1, regSrc + 1 ) ;
	}
}

void ECSSakura2JIT::Sakura2Assembler::write_fdiv_reg_reg
	( int regDst, int regSrc, bool fPair )
{
	FlushAllRegisters() ;
	WriteToCallInstructionRegReg( m_gi->fdiv_reg_reg, regDst, regSrc ) ;
	if ( fPair )
	{
		WriteToCallInstructionRegReg
				( m_gi->fdiv_reg_reg, regDst + 1, regSrc + 1 ) ;
	}
}

// 特殊精度整数演算命令
//////////////////////////////////////////////////////////////////////////////
void ECSSakura2JIT::Sakura2Assembler::write_mul32_reg_reg( int regDst, int regSrc )
{
	FlushAllRegisters() ;
	WriteToCallInstructionRegReg( m_gi->mul32_reg_reg, regDst, regSrc ) ;
}

void ECSSakura2JIT::Sakura2Assembler::write_imul32_reg_reg( int regDst, int regSrc )
{
	FlushAllRegisters() ;
	WriteToCallInstructionRegReg( m_gi->imul32_reg_reg, regDst, regSrc ) ;
}

void ECSSakura2JIT::Sakura2Assembler::write_div32_reg_reg( const void * pEscCode, int regDst, int regSrc )
{
	FlushAllRegisters() ;
	WriteToZeroDivisionException32( regSrc, pEscCode ) ;
	WriteToCallInstructionRegReg( m_gi->div32_reg_reg, regDst, regSrc ) ;
}

void ECSSakura2JIT::Sakura2Assembler::write_idiv32_reg_reg( const void * pEscCode, int regDst, int regSrc )
{
	FlushAllRegisters() ;
	WriteToZeroDivisionException32( regSrc, pEscCode ) ;
	WriteToCallInstructionRegReg( m_gi->idiv32_reg_reg, regDst, regSrc ) ;
}

void ECSSakura2JIT::Sakura2Assembler::write_mod32_reg_reg( const void * pEscCode, int regDst, int regSrc )
{
	FlushAllRegisters() ;
	WriteToZeroDivisionException32( regSrc, pEscCode ) ;
	WriteToCallInstructionRegReg( m_gi->mod32_reg_reg, regDst, regSrc ) ;
}

void ECSSakura2JIT::Sakura2Assembler::write_imod32_reg_reg( const void * pEscCode, int regDst, int regSrc )
{
	FlushAllRegisters() ;
	WriteToZeroDivisionException32( regSrc, pEscCode ) ;
	WriteToCallInstructionRegReg( m_gi->imod32_reg_reg, regDst, regSrc ) ;
}

// 整数比較命令
//////////////////////////////////////////////////////////////////////////////
void ECSSakura2JIT::Sakura2Assembler::write_cmp_ne( int regDst, int regSrc, bool fPair )
{
	FlushAllRegisters() ;
	WriteToCallInstructionRegReg( m_gi->cmp_ne, regDst, regSrc ) ;
	if ( fPair )
	{
		WriteToCallInstructionRegReg( m_gi->cmp_ne, regDst + 1, regSrc + 1 ) ;
	}
}

void ECSSakura2JIT::Sakura2Assembler::write_cmp_eq( int regDst, int regSrc, bool fPair )
{
	FlushAllRegisters() ;
	WriteToCallInstructionRegReg( m_gi->cmp_eq, regDst, regSrc ) ;
	if ( fPair )
	{
		WriteToCallInstructionRegReg( m_gi->cmp_eq, regDst + 1, regSrc + 1 ) ;
	}
}

void ECSSakura2JIT::Sakura2Assembler::write_cmp_lt( int regDst, int regSrc, bool fPair )
{
	FlushAllRegisters() ;
	WriteToCallInstructionRegReg( m_gi->cmp_lt, regDst, regSrc ) ;
	if ( fPair )
	{
		WriteToCallInstructionRegReg( m_gi->cmp_lt, regDst + 1, regSrc + 1 ) ;
	}
}

void ECSSakura2JIT::Sakura2Assembler::write_cmp_le( int regDst, int regSrc, bool fPair )
{
	FlushAllRegisters() ;
	WriteToCallInstructionRegReg( m_gi->cmp_le, regDst, regSrc ) ;
	if ( fPair )
	{
		WriteToCallInstructionRegReg( m_gi->cmp_le, regDst + 1, regSrc + 1 ) ;
	}
}

void ECSSakura2JIT::Sakura2Assembler::write_cmp_gt( int regDst, int regSrc, bool fPair )
{
	FlushAllRegisters() ;
	WriteToCallInstructionRegReg( m_gi->cmp_gt, regDst, regSrc ) ;
	if ( fPair )
	{
		WriteToCallInstructionRegReg( m_gi->cmp_gt, regDst + 1, regSrc + 1 ) ;
	}
}

void ECSSakura2JIT::Sakura2Assembler::write_cmp_ge( int regDst, int regSrc, bool fPair )
{
	FlushAllRegisters() ;
	WriteToCallInstructionRegReg( m_gi->cmp_ge, regDst, regSrc ) ;
	if ( fPair )
	{
		WriteToCallInstructionRegReg( m_gi->cmp_ge, regDst + 1, regSrc + 1 ) ;
	}
}

void ECSSakura2JIT::Sakura2Assembler::write_cmp_c( int regDst, int regSrc, bool fPair )
{
	FlushAllRegisters() ;
	WriteToCallInstructionRegReg( m_gi->cmp_c, regDst, regSrc ) ;
	if ( fPair )
	{
		WriteToCallInstructionRegReg( m_gi->cmp_c, regDst + 1, regSrc + 1 ) ;
	}
}

void ECSSakura2JIT::Sakura2Assembler::write_cmp_cz( int regDst, int regSrc, bool fPair )
{
	FlushAllRegisters() ;
	WriteToCallInstructionRegReg( m_gi->cmp_cz, regDst, regSrc ) ;
	if ( fPair )
	{
		WriteToCallInstructionRegReg( m_gi->cmp_cz, regDst + 1, regSrc + 1 ) ;
	}
}

// 実数比較命令
//////////////////////////////////////////////////////////////////////////////
void ECSSakura2JIT::Sakura2Assembler::write_fcmp_ne( int regDst, int regSrc, bool fPair )
{
	FlushAllRegisters() ;
	WriteToCallInstructionRegReg( m_gi->fcmp_ne, regDst, regSrc ) ;
	if ( fPair )
	{
		WriteToCallInstructionRegReg( m_gi->fcmp_ne, regDst + 1, regSrc + 1 ) ;
	}
}

void ECSSakura2JIT::Sakura2Assembler::write_fcmp_eq( int regDst, int regSrc, bool fPair )
{
	FlushAllRegisters() ;
	WriteToCallInstructionRegReg( m_gi->fcmp_eq, regDst, regSrc ) ;
	if ( fPair )
	{
		WriteToCallInstructionRegReg( m_gi->fcmp_eq, regDst + 1, regSrc + 1 ) ;
	}
}

void ECSSakura2JIT::Sakura2Assembler::write_fcmp_lt( int regDst, int regSrc, bool fPair )
{
	FlushAllRegisters() ;
	WriteToCallInstructionRegReg( m_gi->fcmp_lt, regDst, regSrc ) ;
	if ( fPair )
	{
		WriteToCallInstructionRegReg( m_gi->fcmp_lt, regDst + 1, regSrc + 1 ) ;
	}
}

void ECSSakura2JIT::Sakura2Assembler::write_fcmp_le( int regDst, int regSrc, bool fPair )
{
	FlushAllRegisters() ;
	WriteToCallInstructionRegReg( m_gi->fcmp_le, regDst, regSrc ) ;
	if ( fPair )
	{
		WriteToCallInstructionRegReg( m_gi->fcmp_le, regDst + 1, regSrc + 1 ) ;
	}
}

void ECSSakura2JIT::Sakura2Assembler::write_fcmp_gt( int regDst, int regSrc, bool fPair )
{
	FlushAllRegisters() ;
	WriteToCallInstructionRegReg( m_gi->fcmp_gt, regDst, regSrc ) ;
	if ( fPair )
	{
		WriteToCallInstructionRegReg( m_gi->fcmp_gt, regDst + 1, regSrc + 1 ) ;
	}
}

void ECSSakura2JIT::Sakura2Assembler::write_fcmp_ge( int regDst, int regSrc, bool fPair )
{
	FlushAllRegisters() ;
	WriteToCallInstructionRegReg( m_gi->fcmp_ge, regDst, regSrc ) ;
	if ( fPair )
	{
		WriteToCallInstructionRegReg( m_gi->fcmp_ge, regDst + 1, regSrc + 1 ) ;
	}
}

// 相対無条件ジャンプ命令
//////////////////////////////////////////////////////////////////////////////
void * ECSSakura2JIT::Sakura2Assembler::write_jump_offset32( void )
{
	FlushAllRegisters() ;
	return	WriteToJump( NULL ) ;
}

// 相対条件ジャンプ命令 0xD2～
//////////////////////////////////////////////////////////////////////////////
void * ECSSakura2JIT::Sakura2Assembler::write_cnjump_reg_offset32( int reg )
{
	FlushAllRegisters() ;
	return	WriteToConditionalJump( reg, false, NULL ) ;
}

void * ECSSakura2JIT::Sakura2Assembler::write_cjump_reg_offset32( int reg )
{
	FlushAllRegisters() ;
	return	WriteToConditionalJump( reg, true, NULL ) ;
}

// メモリヒント
//////////////////////////////////////////////////////////////////////////////
void * ECSSakura2JIT::Sakura2Assembler::write_prefetch_tlb( int tlb, int reg )
{
	m_tlbFetch[reg & 0xFF] = tlb & 0x01 ;
	m_regTLBFetched[tlb & 0x01] = reg & 0xFF ;
	return	NULL ;
}

void ECSSakura2JIT::Sakura2Assembler::write_unfetch_tlb( int tlb, int reg )
{
	m_tlbFetch[reg & 0xFF] = 0xFF ;
	m_regTLBFetched[tlb & 0x01] = -1 ;
}

// 浮動小数点演算 EXTENSION
//////////////////////////////////////////////////////////////////////////////
void ECSSakura2JIT::Sakura2Assembler::write_float_extension
	( int code, int regDst, int regSrc )
{
	FlushAllRegisters() ;
	WriteToCallInstructionRegReg( m_fx[code & 0xFF], regDst, regSrc ) ;
}

// 64bit SIMD
//////////////////////////////////////////////////////////////////////////////
void ECSSakura2JIT::Sakura2Assembler::write_simd64_extension
	( int code, int regDst, int regSrc, bool fPair )
{
	FlushAllRegisters() ;
	WriteToCallInstructionRegReg( m_simd64op2[code & 0xFF], regDst, regSrc ) ;
	if ( fPair )
	{
		switch ( code & 0xFF )
		{
		case	simdPsrlw:
		case	simdPsraw:
		case	simdPsllw:
			WriteToCallInstructionRegReg
				( m_simd64op2[code & 0xFF], regDst + 1, regSrc ) ;
			break ;

		default:
			WriteToCallInstructionRegReg
				( m_simd64op2[code & 0xFF], regDst + 1, regSrc + 1 ) ;
			break ;
		}
	}
}

void ECSSakura2JIT::Sakura2Assembler::write_simd64_imm_extension
	( int code, int regDst, int regSrc, int imm8, bool fPair )
{
	FlushAllRegisters() ;
	WriteToCallInstructionRegRegImm
		( m_simd64op3[code & 0xFF], regDst, regSrc, imm8 ) ;
	if ( fPair )
	{
		WriteToCallInstructionRegRegImm
			( m_simd64op3[code & 0xFF], regDst + 1, regSrc + 1, imm8 ) ;
	}
}

// 128bit SIMD
//////////////////////////////////////////////////////////////////////////////
void ECSSakura2JIT::Sakura2Assembler::write_simd128_extension
	( int code, int regDst, int regSrc )
{
	FlushAllRegisters() ;
	WriteToCallSIMD128InstructionRegReg
		( m_simd128op2[code & 0xFF], regDst, regSrc ) ;
}

void ECSSakura2JIT::Sakura2Assembler::write_simd128_imm_extension
	( int code, int regDst, int regSrc, int imm8 )
{
	FlushAllRegisters() ;
	WriteToCallSIMD128InstructionRegRegImm
		( m_simd128op3[code & 0xFF], regDst, regSrc, imm8 ) ;
}




//////////////////////////////////////////////////////////////////////////////
// Sakura2 processor ネイティブコード化コンパイラ
//////////////////////////////////////////////////////////////////////////////

// メモリオフセット領域追加
//////////////////////////////////////////////////////////////////////////////
void ECSSakura2JIT::NativeCompiler::MemoryOffsetRange::AddRange( int first, int end )
{
	if ( !flagUsed )
	{
		flagUsed = true ;
		minOffset = first ;
		maxOffset = end ;
	}
	else
	{
		if ( minOffset > first )
		{
			minOffset = first ;
		}
		if ( maxOffset < end )
		{
			maxOffset = end ;
		}
	}
}

// コード出力オブジェクト設定
//////////////////////////////////////////////////////////////////////////////
void ECSSakura2JIT::NativeCompiler::AttachCodeAssembler
	( Sakura2Assembler * pCodes, Sakura2Assembler * pGates )
{
	m_pAssemberCodes = pCodes ;
	m_pAssemberGates = pGates ;
}

// 詞葉 naked 関数設定
//////////////////////////////////////////////////////////////////////////////
void ECSSakura2JIT::NativeCompiler::AttachFunction
	( BYTE * pbytCode, BYTE * pbytTrickBuf,
			DWORD dwFuncAddr, DWORD dwFuncSize )
{
	m_pbytCode = pbytCode ;
	m_pbytTrickBuf = pbytTrickBuf ;
	m_dwFuncAddr = dwFuncAddr ;
	m_dwFuncSize = dwFuncSize ;
}

// 関数全体の基本情報を収集する（第１パス）
//////////////////////////////////////////////////////////////////////////////
bool ECSSakura2JIT::NativeCompiler::PreprocessFunction( void )
{
	m_lstLabelAddr.RemoveAll() ;
	m_mbRangeBP.flagUsed = false ;
	m_mbRangeTP.flagUsed = false ;
	m_fObjectComplex = false ;
	//
	int	iNextCode = 0 ;
	while ( (DWORD) iNextCode < m_dwFuncSize )
	{
		InstructionInfo	inf ;
		inf.nFlags = 0 ;
		GetInstructionInfo( &inf, m_pbytCode + iNextCode ) ;
		//
		if ( (inf.nType == typeLoadMem)
			|| (inf.nType == typeStoreMem) )
		{
			//
			// [reg+offset32] 形式のメモリアクセスを集計
			//
			BYTE	bytCode = m_pbytCode[iNextCode] ;
			BYTE	bytMem = m_pbytCode[iNextCode + 1] ;
			int		typeData = bytMem & 0x07 ;
			int		regBase = (bytMem >> 3) & 0x0F ;
			int		sizeData = sizeof_prim_data[typeData] ;
			SDWORD	dwOffset ;
			//
			switch ( bytCode )
			{
			case	codeLoadMemBase:
			case	codeStoreMemBase:
				if ( regBase == regTP )
				{
					m_mbRangeTP.AddRange( 0, sizeData ) ;
				}
				break ;
			case	codeLoadMemBaseImm32:
			case	codeStoreMemBaseImm32:
				dwOffset = *((SDWORD*)(m_pbytCode + iNextCode + 3)) ;
				if ( regBase == regTP )
				{
					m_mbRangeTP.AddRange
						( dwOffset, dwOffset + sizeData ) ;
				}
				break ;
			case	codeLoadMemBaseIndex:
			case	codeStoreMemBaseIndex:
			case	codeLoadMemBaseIndexImm32:
			case	codeStoreMemBaseIndexImm32:
				break ;
			case	codeLoadLocalImm32:
			case	codeStoreLocalImm32:
				dwOffset = *((SDWORD*)(m_pbytCode + iNextCode + 3)) ;
				m_mbRangeBP.AddRange( dwOffset, dwOffset + sizeData ) ;
				break ;
			case	codeLoadLocalIndexImm32:
			case	codeStoreLocalIndexImm32:
				break ;
			}
		}
		else if ( inf.nType == typeJump )
		{
			//
			// ジャンプ先／リターン先アドレスを登録
			//
			BYTE	bytCode = m_pbytCode[iNextCode] ;
			SDWORD	dwOffset ;
			DWORD	dwJumpTarget ;
			switch ( bytCode )
			{
			case	codeCNJumpOffset32:
			case	codeCJumpOffset32:
				dwOffset = *((SDWORD*)(m_pbytCode + iNextCode + 2)) ;
				dwJumpTarget = iNextCode + inf.nBytes + dwOffset ;
				if ( m_lstLabelAddr.GetAs( (int) dwJumpTarget ) == NULL )
				{
					m_lstLabelAddr.Add( dwJumpTarget, new JumpAddress() ) ;
				}
				break ;
			case	codeJumpOffset32:
				dwOffset = *((SDWORD*)(m_pbytCode + iNextCode + 1)) ;
				dwJumpTarget = iNextCode + inf.nBytes + dwOffset ;
				if ( m_lstLabelAddr.GetAs( (int) dwJumpTarget ) == NULL )
				{
					m_lstLabelAddr.Add( dwJumpTarget, new JumpAddress() ) ;
				}
			case	codeJumpReg:
			case	codeCallImm32:
			case	codeCallReg:
			case	codeSysCallImm32:
			case	codeSysCallReg:
			case	codeReturn:
				dwJumpTarget = iNextCode + inf.nBytes ;
				if ( m_lstLabelAddr.GetAs( (int) dwJumpTarget ) == NULL )
				{
					m_lstLabelAddr.Add( dwJumpTarget, new JumpAddress() ) ;
				}
				break ;
			}
		}
		else if ( inf.nType == typeComplex )
		{
		}
		else if ( inf.nType == typeObject )
		{
			m_fObjectComplex = true ;
			break ;
		}
		iNextCode += inf.nBytes ;
	}
	return	!m_fObjectComplex ;
}

// ネイティブコードを出力する（第２パス）
//////////////////////////////////////////////////////////////////////////////
void ECSSakura2JIT::NativeCompiler::CompileFunction( void )
{
	if ( m_fObjectComplex )
	{
		return ;
	}
	m_pAssemberCodes->ResetAllRegisters() ;
	m_pAssemberGates->ResetAllRegisters() ;
	//
	#if	defined(__DEBUG__)
	m_pAssemberCodes->OnDebugBeforeFunction( m_dwFuncAddr, m_dwFuncSize ) ;
	m_pAssemberGates->OnDebugBeforeFunction( m_dwFuncAddr, m_dwFuncSize ) ;
	#endif
	//
	m_fInitBP = false ;
	m_fInitTP = false ;
	//
	bool	fDelayEnter = true ;
	int		iLastEnterPos = -1 ;
	int		iNextCode = 0 ;
	while ( (DWORD) iNextCode < m_dwFuncSize )
	{
		//
		// 進入位置判定：ラベル位置ならレジスタ依存を解消
		//
		bool	fCurrentEnter = fDelayEnter ;
		fDelayEnter = false ;
		//
		JumpAddress *	pLabel = m_lstLabelAddr.GetAs( iNextCode ) ;
		if ( pLabel != NULL )
		{
			m_pAssemberCodes->FlushAllRegisters() ;
			m_pAssemberCodes->ResetAllRegisters() ;
			//
			pLabel->ptrNative = m_pAssemberCodes->GetNextAddress() ;
			//
			fCurrentEnter = true ;
		}
		const void *	ptrCurrentCodePos = m_pAssemberCodes->GetNextAddress() ;
		//
		// 命令判定
		//
		InstructionInfo	inf ;
		inf.nFlags = 0 ;
		GetInstructionInfo( &inf, m_pbytCode + iNextCode ) ;
		//
		bool		fPair = ((DWORD)(iNextCode + inf.nBytes * 2) <= m_dwFuncSize)
						&& (m_lstLabelAddr.GetAs( iNextCode + inf.nBytes ) == NULL)
						&& IsPairInstruction( inf, m_pbytCode + iNextCode ) ;
		bool		fDoubleRegister = false ;
		BYTE		bytCode = m_pbytCode[iNextCode] ;
		BYTE		xbbbbddd, yiiiiiii, xxx00ddd, iiiiiiii ;
		INT64		imm64 ;
		int			regDst, regSrc, regSrc2 ;
		void *		pEscJumpFrom = NULL ;
		const int	ipEscCurrent = m_dwFuncAddr + iNextCode ;
		int			ipEscException = m_dwFuncAddr + iNextCode + inf.nBytes ;
		DWORD		dwEscException = 0 ;
		int			nJumpTarget ;
		void *		pJumpFrom = NULL ;
		//
		#if	defined(__DEBUG__)
		m_pAssemberCodes->OnDebugBeforeInstruction( bytCode, ipEscCurrent ) ;
		#endif
		//
		switch ( bytCode )
		{
		// メモリロード命令
		////////////////////////////////////////////////////////////////////////////
		case	codeLoadMemBase:
			xbbbbddd = m_pbytCode[iNextCode + 1] ;
			pEscJumpFrom =
				m_pAssemberCodes->write_load_memory
					( NULL, addrBase,
						(DataType) (xbbbbddd & 0x07),
						m_pbytCode[iNextCode + 2],
						((xbbbbddd >> 3) & 0x0F), -1, 0, 0, fPair ) ;
			dwEscException = exceptionReadMemory ;
			break ;
		case	codeLoadMemBaseImm32:
			xbbbbddd = m_pbytCode[iNextCode + 1] ;
			pEscJumpFrom =
				m_pAssemberCodes->write_load_memory
					( NULL, addrBaseOffset32,
						(DataType) (xbbbbddd & 0x07),
						m_pbytCode[iNextCode + 2],
						((xbbbbddd >> 3) & 0x0F), -1, 0,
						*((SDWORD*)(m_pbytCode + iNextCode + 3)), fPair ) ;
			dwEscException = exceptionReadMemory ;
			break ;
		case	codeLoadMemBaseIndex:
			xbbbbddd = m_pbytCode[iNextCode + 1] ;
			yiiiiiii = m_pbytCode[iNextCode + 2] ;
			pEscJumpFrom =
				m_pAssemberCodes->write_load_memory
					( NULL, addrBaseIndex,
						(DataType) (xbbbbddd & 0x07),
						m_pbytCode[iNextCode + 3],
						((xbbbbddd >> 3) & 0x0F),
						(yiiiiiii & 0x7F),
						((xbbbbddd >> 7) << 1) + (yiiiiiii >> 7), 0, fPair ) ;
			dwEscException = exceptionReadMemory ;
			break ;
		case	codeLoadMemBaseIndexImm32:
			xbbbbddd = m_pbytCode[iNextCode + 1] ;
			yiiiiiii = m_pbytCode[iNextCode + 2] ;
			pEscJumpFrom =
				m_pAssemberCodes->write_load_memory
					( NULL, addrBaseIndexOffset32,
						(DataType) (xbbbbddd & 0x07),
						m_pbytCode[iNextCode + 3],
						((xbbbbddd >> 3) & 0x0F),
						(yiiiiiii & 0x7F),
						((xbbbbddd >> 7) << 1) + (yiiiiiii >> 7),
						*((SDWORD*)(m_pbytCode + iNextCode + 4)), fPair ) ;
			dwEscException = exceptionReadMemory ;
			break ;

		// メモリストア命令
		////////////////////////////////////////////////////////////////////////////
		case	codeStoreMemBase:
			xbbbbddd = m_pbytCode[iNextCode + 1] ;
			pEscJumpFrom =
				m_pAssemberCodes->write_store_memory
					( NULL, addrBase,
						(DataType) (xbbbbddd & 0x07),
						m_pbytCode[iNextCode + 2],
						((xbbbbddd >> 3) & 0x0F), -1, 0, 0, fPair ) ;
			dwEscException = exceptionWriteMemory ;
			break ;
		case	codeStoreMemBaseImm32:
			xbbbbddd = m_pbytCode[iNextCode + 1] ;
			pEscJumpFrom =
				m_pAssemberCodes->write_store_memory
					( NULL, addrBaseOffset32,
						(DataType) (xbbbbddd & 0x07),
						m_pbytCode[iNextCode + 2],
						((xbbbbddd >> 3) & 0x0F), -1, 0,
						*((SDWORD*)(m_pbytCode + iNextCode + 3)), fPair ) ;
			dwEscException = exceptionWriteMemory ;
			break ;
		case	codeStoreMemBaseIndex:
			xbbbbddd = m_pbytCode[iNextCode + 1] ;
			yiiiiiii = m_pbytCode[iNextCode + 2] ;
			pEscJumpFrom =
				m_pAssemberCodes->write_store_memory
					( NULL, addrBaseIndex,
						(DataType) (xbbbbddd & 0x07),
						m_pbytCode[iNextCode + 3],
						((xbbbbddd >> 3) & 0x0F),
						(yiiiiiii & 0x7F),
						((xbbbbddd >> 7) << 1) + (yiiiiiii >> 7), 0, fPair ) ;
			dwEscException = exceptionWriteMemory ;
			break ;
		case	codeStoreMemBaseIndexImm32:
			xbbbbddd = m_pbytCode[iNextCode + 1] ;
			yiiiiiii = m_pbytCode[iNextCode + 2] ;
			pEscJumpFrom =
				m_pAssemberCodes->write_store_memory
					( NULL, addrBaseIndexOffset32,
						(DataType) (xbbbbddd & 0x07),
						m_pbytCode[iNextCode + 3],
						((xbbbbddd >> 3) & 0x0F),
						(yiiiiiii & 0x7F),
						((xbbbbddd >> 7) << 1) + (yiiiiiii >> 7),
						*((SDWORD*)(m_pbytCode + iNextCode + 4)), fPair ) ;
			dwEscException = exceptionWriteMemory ;
			break ;

		// ローカルメモリロード命令
		////////////////////////////////////////////////////////////////////////////
		case	codeLoadLocalImm32:
			xxx00ddd = m_pbytCode[iNextCode + 1] ;
			regDst = m_pbytCode[iNextCode + 2] ;
			pEscJumpFrom =
				m_pAssemberCodes->write_load_local
					( NULL, addrLocalOffset32,
						(DataType) (xxx00ddd & 0x07),
						regDst, -1, 0,
						*((SDWORD*)(m_pbytCode + iNextCode + 3)), fPair ) ;
			dwEscException = exceptionReadMemory ;
			break ;
		case	codeLoadLocalIndexImm32:
			xxx00ddd = m_pbytCode[iNextCode + 1] ;
			iiiiiiii = m_pbytCode[iNextCode + 2] ;
			pEscJumpFrom =
				m_pAssemberCodes->write_load_local
					( NULL, addrLocalIndexOffset32,
						(DataType) (xxx00ddd & 0x07),
						m_pbytCode[iNextCode + 3],
						iiiiiiii, ((xxx00ddd >> 5) & 0x07),
						*((SDWORD*)(m_pbytCode + iNextCode + 4)), fPair ) ;
			dwEscException = exceptionReadMemory ;
			break ;

		// ローカルメモリストア命令
		////////////////////////////////////////////////////////////////////////////
		case	codeStoreLocalImm32:
			xxx00ddd = m_pbytCode[iNextCode + 1] ;
			pEscJumpFrom =
				m_pAssemberCodes->write_store_local
					( NULL, addrLocalOffset32,
						(DataType) (xxx00ddd & 0x07),
						m_pbytCode[iNextCode + 2], -1, 0,
						*((SDWORD*)(m_pbytCode + iNextCode + 3)), fPair ) ;
			dwEscException = exceptionWriteMemory ;
			break ;
		case	codeStoreLocalIndexImm32:
			xxx00ddd = m_pbytCode[iNextCode + 1] ;
			iiiiiiii = m_pbytCode[iNextCode + 2] ;
			pEscJumpFrom =
				m_pAssemberCodes->write_store_local
					( NULL, addrLocalIndexOffset32,
						(DataType) (xxx00ddd & 0x07),
						m_pbytCode[iNextCode + 3],
						iiiiiiii, ((xxx00ddd >> 5) & 0x07),
						*((SDWORD*)(m_pbytCode + iNextCode + 4)), fPair ) ;
			dwEscException = exceptionWriteMemory ;
			break ;

		// データ移動命令
		////////////////////////////////////////////////////////////////////////////
		case	codeMoveReg:
			regDst = m_pbytCode[iNextCode + 1] ;
			regSrc = m_pbytCode[iNextCode + 2] ;
			m_pAssemberCodes->write_move_reg_reg( regDst, regSrc, fPair ) ;
			break ;
		case	codeMaskMove:
			regDst = m_pbytCode[iNextCode + 1] ;
			regSrc = m_pbytCode[iNextCode + 2] ;
			regSrc2 = m_pbytCode[iNextCode + 3] ;
			m_pAssemberCodes->write_maskmove_reg_reg_reg
							( regDst, regSrc, regSrc2, fPair ) ;
			break ;

		// 整数・実数変換命令
		////////////////////////////////////////////////////////////////////////////
		case	codeCvtFloat2Int:
			m_pAssemberCodes->write_cvt_float2int
					( m_pbytCode[iNextCode + 1], m_pbytCode[iNextCode + 2] ) ;
			fPair = false ;
			break ;
		case	codeCvtInt2Float:
			m_pAssemberCodes->write_cvt_int2float
					( m_pbytCode[iNextCode + 1], m_pbytCode[iNextCode + 2] ) ;
			fPair = false ;
			break ;

		// 即値シフト命令
		////////////////////////////////////////////////////////////////////////////
		case	codeSrlImm8:
			m_pAssemberCodes->write_srl_reg_reg_imm8
					( m_pbytCode[iNextCode + 1],
						m_pbytCode[iNextCode + 2],
						(SBYTE) m_pbytCode[iNextCode + 3] & 0x3F, fPair ) ;
			break ;
		case	codeSraImm8:
			m_pAssemberCodes->write_sra_reg_reg_imm8
					( m_pbytCode[iNextCode + 1],
						m_pbytCode[iNextCode + 2],
						(SBYTE) m_pbytCode[iNextCode + 3] & 0x3F, fPair ) ;
			break ;
		case	codeSllImm8:
			m_pAssemberCodes->write_sll_reg_reg_imm8
					( m_pbytCode[iNextCode + 1],
						m_pbytCode[iNextCode + 2],
						(SBYTE) m_pbytCode[iNextCode + 3] & 0x3F, fPair ) ;
			break ;

		// 32ビット即値命令
		////////////////////////////////////////////////////////////////////////////
		case	codeAddImm32:
			m_pAssemberCodes->write_add_reg_reg_imm32
					( m_pbytCode[iNextCode + 1],
						m_pbytCode[iNextCode + 2],
						*((SDWORD*)(m_pbytCode + iNextCode + 3)), fPair ) ;
			break ;
		case	codeMulImm32:
			m_pAssemberCodes->write_mul_reg_reg_imm32
					( m_pbytCode[iNextCode + 1],
						m_pbytCode[iNextCode + 2],
						*((SDWORD*)(m_pbytCode + iNextCode + 3)), fPair ) ;
			break ;

		// スタックレジスタ加算命令
		////////////////////////////////////////////////////////////////////////////
		case	codeAddSPImm32:
			pEscJumpFrom =
				m_pAssemberCodes->write_add_sp_imm32
					( ipEscCurrent, *((SDWORD*)(m_pbytCode + iNextCode + 1)) ) ;
			dwEscException = exceptionExtendStack ;
			fPair = false ;
			break ;

		// 64ビット即値命令
		////////////////////////////////////////////////////////////////////////////
		case	codeLoadImm64:
			#if	defined(PLATFORM_ANDROID)
				imm64 = ((INT64) *((DWORD*)(m_pbytCode + iNextCode + 6)) << 32)
									| *((DWORD*)(m_pbytCode + iNextCode + 2)) ;
			#else
				imm64 = *((INT64*)(m_pbytCode + iNextCode + 2)) ;
			#endif
			m_pAssemberCodes->write_move_reg_imm64
 					( m_pbytCode[iNextCode + 1], imm64 ) ;
			fPair = false ;
			break ;

		// 1 OP 演算命令
		////////////////////////////////////////////////////////////////////////////
		case	codeNegInt:
			m_pAssemberCodes->write_neg_int( m_pbytCode[iNextCode + 1] ) ;
			fPair = false ;
			break ;
		case	codeNotInt:
			m_pAssemberCodes->write_not_int( m_pbytCode[iNextCode + 1] ) ;
			fPair = false ;
			break ;
		case	codeNegFloat:
			m_pAssemberCodes->write_neg_float( m_pbytCode[iNextCode + 1] ) ;
			fPair = false ;
			break ;

		// 2 OP 整数演算命令
		////////////////////////////////////////////////////////////////////////////
		case	codeAddReg:
			m_pAssemberCodes->write_add_reg_reg
					( m_pbytCode[iNextCode + 1], m_pbytCode[iNextCode + 2], fPair ) ;
			break ;
		case	codeSubReg:
			m_pAssemberCodes->write_sub_reg_reg
					( m_pbytCode[iNextCode + 1], m_pbytCode[iNextCode + 2], fPair ) ;
			break ;
		case	codeMulReg:
			m_pAssemberCodes->write_mul_reg_reg
					( m_pbytCode[iNextCode + 1], m_pbytCode[iNextCode + 2], fPair ) ;
			break ;
		case	codeDivReg:
			m_pAssemberCodes->write_div_reg_reg
				( m_pAssemberGates->GetNextAddress(),
					m_pbytCode[iNextCode + 1], m_pbytCode[iNextCode + 2] ) ;
			m_pAssemberGates->WriteToAtomicOrExceptionMask( exceptionZeroDivision ) ;
			m_pAssemberGates->WriteEpilogue( ipEscCurrent ) ;
			fPair = false ;
			break ;
		case	codeModReg:
			m_pAssemberCodes->write_mod_reg_reg
				( m_pAssemberGates->GetNextAddress(),
					m_pbytCode[iNextCode + 1], m_pbytCode[iNextCode + 2] ) ;
			m_pAssemberGates->WriteToAtomicOrExceptionMask( exceptionZeroDivision ) ;
			m_pAssemberGates->WriteEpilogue( ipEscCurrent ) ;
			fPair = false ;
			break ;
		case	codeAndReg:
			m_pAssemberCodes->write_and_reg_reg
					( m_pbytCode[iNextCode + 1], m_pbytCode[iNextCode + 2], fPair ) ;
			break ;
		case	codeOrReg:
			m_pAssemberCodes->write_or_reg_reg
					( m_pbytCode[iNextCode + 1], m_pbytCode[iNextCode + 2], fPair ) ;
			break ;
		case	codeXorReg:
			m_pAssemberCodes->write_xor_reg_reg
					( m_pbytCode[iNextCode + 1], m_pbytCode[iNextCode + 2], fPair ) ;
			break ;
		case	codeSrlReg:
			m_pAssemberCodes->write_srl_reg_reg
					( m_pbytCode[iNextCode + 1], m_pbytCode[iNextCode + 2], fPair ) ;
			break ;
		case	codeSraReg:
			m_pAssemberCodes->write_sra_reg_reg
					( m_pbytCode[iNextCode + 1], m_pbytCode[iNextCode + 2], fPair ) ;
			break ;
		case	codeSllReg:
			m_pAssemberCodes->write_sll_reg_reg
					( m_pbytCode[iNextCode + 1], m_pbytCode[iNextCode + 2], fPair ) ;
			break ;

		// 整数符号拡張命令
		////////////////////////////////////////////////////////////////////////////
		case	codeMoveSx32Reg:
			m_pAssemberCodes->write_move_sx32_reg_reg
					( m_pbytCode[iNextCode + 1], m_pbytCode[iNextCode + 2] ) ;
			fPair = false ;
			break ;
		case	codeMoveSx16Reg:
			m_pAssemberCodes->write_move_sx16_reg_reg
					( m_pbytCode[iNextCode + 1], m_pbytCode[iNextCode + 2] ) ;
			fPair = false ;
			break ;
		case	codeMoveSx8Reg:
			m_pAssemberCodes->write_move_sx8_reg_reg
					( m_pbytCode[iNextCode + 1], m_pbytCode[iNextCode + 2] ) ;
			fPair = false ;
			break ;

		// 2 OP 実数演算命令
		////////////////////////////////////////////////////////////////////////////
		case	codeFAddReg:
			m_pAssemberCodes->write_fadd_reg_reg
					( m_pbytCode[iNextCode + 1], m_pbytCode[iNextCode + 2], fPair ) ;
			break ;
		case	codeFSubReg:
			m_pAssemberCodes->write_fsub_reg_reg
					( m_pbytCode[iNextCode + 1], m_pbytCode[iNextCode + 2], fPair ) ;
			break ;
		case	codeFMulReg:
			m_pAssemberCodes->write_fmul_reg_reg
					( m_pbytCode[iNextCode + 1], m_pbytCode[iNextCode + 2], fPair ) ;
			break ;
		case	codeFDivReg:
			m_pAssemberCodes->write_fdiv_reg_reg
					( m_pbytCode[iNextCode + 1], m_pbytCode[iNextCode + 2], fPair ) ;
			break ;

		// 特殊精度整数演算命令
		////////////////////////////////////////////////////////////////////////////
		case	codeMul32Reg:
			m_pAssemberCodes->write_mul32_reg_reg
					( m_pbytCode[iNextCode + 1], m_pbytCode[iNextCode + 2] ) ;
			fPair = false ;
			break ;
		case	codeIMul32Reg:
			m_pAssemberCodes->write_imul32_reg_reg
					( m_pbytCode[iNextCode + 1], m_pbytCode[iNextCode + 2] ) ;
			fPair = false ;
			break ;
		case	codeDiv32Reg:
			m_pAssemberCodes->write_div32_reg_reg
				( m_pAssemberGates->GetNextAddress(),
					m_pbytCode[iNextCode + 1], m_pbytCode[iNextCode + 2] ) ;
			m_pAssemberGates->WriteToAtomicOrExceptionMask( exceptionZeroDivision ) ;
			m_pAssemberGates->WriteEpilogue( ipEscCurrent ) ;
			fPair = false ;
			break ;
		case	codeIDiv32Reg:
			m_pAssemberCodes->write_idiv32_reg_reg
				( m_pAssemberGates->GetNextAddress(),
					m_pbytCode[iNextCode + 1], m_pbytCode[iNextCode + 2] ) ;
			m_pAssemberGates->WriteToAtomicOrExceptionMask( exceptionZeroDivision ) ;
			m_pAssemberGates->WriteEpilogue( ipEscCurrent ) ;
			fPair = false ;
			break ;
		case	codeMod32Reg:
			m_pAssemberCodes->write_mod32_reg_reg
				( m_pAssemberGates->GetNextAddress(),
					m_pbytCode[iNextCode + 1], m_pbytCode[iNextCode + 2] ) ;
			m_pAssemberGates->WriteToAtomicOrExceptionMask( exceptionZeroDivision ) ;
			m_pAssemberGates->WriteEpilogue( ipEscCurrent ) ;
			fPair = false ;
			break ;
		case	codeIMod32Reg:
			m_pAssemberCodes->write_imod32_reg_reg
				( m_pAssemberGates->GetNextAddress(),
					m_pbytCode[iNextCode + 1], m_pbytCode[iNextCode + 2] ) ;
			m_pAssemberGates->WriteToAtomicOrExceptionMask( exceptionZeroDivision ) ;
			m_pAssemberGates->WriteEpilogue( ipEscCurrent ) ;
			fPair = false ;
			break ;

		// 整数比較命令
		////////////////////////////////////////////////////////////////////////////
		case	codeCmpNeReg:
			m_pAssemberCodes->write_cmp_ne
				( m_pbytCode[iNextCode + 1], m_pbytCode[iNextCode + 2], fPair ) ;
			break ;
		case	codeCmpEqReg:
			m_pAssemberCodes->write_cmp_eq
				( m_pbytCode[iNextCode + 1], m_pbytCode[iNextCode + 2], fPair ) ;
			break ;
		case	codeCmpLtReg:
			m_pAssemberCodes->write_cmp_lt
				( m_pbytCode[iNextCode + 1], m_pbytCode[iNextCode + 2], fPair ) ;
			break ;
		case	codeCmpLeReg:
			m_pAssemberCodes->write_cmp_le
				( m_pbytCode[iNextCode + 1], m_pbytCode[iNextCode + 2], fPair ) ;
			break ;
		case	codeCmpGtReg:
			m_pAssemberCodes->write_cmp_gt
				( m_pbytCode[iNextCode + 1], m_pbytCode[iNextCode + 2], fPair ) ;
			break ;
		case	codeCmpGeReg:
			m_pAssemberCodes->write_cmp_ge
				( m_pbytCode[iNextCode + 1], m_pbytCode[iNextCode + 2], fPair ) ;
			break ;
		case	codeCmpCReg:
			m_pAssemberCodes->write_cmp_c
				( m_pbytCode[iNextCode + 1], m_pbytCode[iNextCode + 2], fPair ) ;
			break ;
		case	codeCmpCZReg:
			m_pAssemberCodes->write_cmp_cz
				( m_pbytCode[iNextCode + 1], m_pbytCode[iNextCode + 2], fPair ) ;
			break ;

		// 実数比較命令
		////////////////////////////////////////////////////////////////////////////
		case	codeFCmpNeReg:
			m_pAssemberCodes->write_fcmp_ne
					( m_pbytCode[iNextCode + 1], m_pbytCode[iNextCode + 2], fPair ) ;
			break ;
		case	codeFCmpEqReg:
			m_pAssemberCodes->write_fcmp_eq
					( m_pbytCode[iNextCode + 1], m_pbytCode[iNextCode + 2], fPair ) ;
			break ;
		case	codeFCmpLtReg:
			m_pAssemberCodes->write_fcmp_lt
					( m_pbytCode[iNextCode + 1], m_pbytCode[iNextCode + 2], fPair ) ;
			break ;
		case	codeFCmpLeReg:
			m_pAssemberCodes->write_fcmp_le
					( m_pbytCode[iNextCode + 1], m_pbytCode[iNextCode + 2], fPair ) ;
			break ;
		case	codeFCmpGtReg:
			m_pAssemberCodes->write_fcmp_gt
					( m_pbytCode[iNextCode + 1], m_pbytCode[iNextCode + 2], fPair ) ;
			break ;
		case	codeFCmpGeReg:
			m_pAssemberCodes->write_fcmp_ge
					( m_pbytCode[iNextCode + 1], m_pbytCode[iNextCode + 2], fPair ) ;
			break ;

		// 相対無条件ジャンプ命令
		////////////////////////////////////////////////////////////////////////////
		case	codeJumpOffset32:
			m_pAssemberCodes->FlushAllRegisters() ;
			//
			if ( *((SDWORD*)(m_pbytCode + iNextCode + 1)) < 0 )
			{
				m_pAssemberCodes->WriteToEscapeByException
								( m_pAssemberGates->GetNextAddress() ) ;
				m_pAssemberGates->WriteEpilogue( ipEscCurrent ) ;
			}
			nJumpTarget = iNextCode + inf.nBytes
					+ *((SDWORD*)(m_pbytCode + iNextCode + 1)) ;
			pLabel = m_lstLabelAddr.GetAs( nJumpTarget ) ;
			ESLAssert( pLabel != NULL ) ;
			if ( pLabel == NULL )
			{
				size_t	nIndex =
					m_lstLabelAddr.Add( nJumpTarget, new JumpAddress() ) ;
				pLabel = m_lstLabelAddr.GetAt( nIndex ) ;
			}
			pJumpFrom = m_pAssemberCodes->write_jump_offset32() ;
			pLabel->lstJumpFrom.Add( pJumpFrom ) ;
			fPair = false ;
			break ;

		// 間接無条件ジャンプ命令
		////////////////////////////////////////////////////////////////////////////
		case	codeJumpReg:
			m_pAssemberCodes->FlushAllRegisters() ;
			//
			m_pAssemberCodes->WriteToEscapeByException
								( m_pAssemberGates->GetNextAddress() ) ;
			m_pAssemberGates->WriteEpilogue( ipEscCurrent ) ;
			//
			m_pAssemberCodes->write_jump_reg( m_pbytCode[iNextCode + 1] ) ;
			ipEscException = -1 ;
			//
			m_pAssemberCodes->WriteEpilogue() ;
			fPair = false ;
			break ;

		// 相対条件ジャンプ命令
		////////////////////////////////////////////////////////////////////////////
		case	codeCNJumpOffset32:
		case	codeCJumpOffset32:
			m_pAssemberCodes->FlushAllRegisters() ;
			//
			if ( *((SDWORD*)(m_pbytCode + iNextCode + 2)) < 0 )
			{
				m_pAssemberCodes->WriteToEscapeByException
							( m_pAssemberGates->GetNextAddress() ) ;
				m_pAssemberGates->WriteEpilogue( ipEscCurrent ) ;
			}
			nJumpTarget = iNextCode + inf.nBytes
					+ *((SDWORD*)(m_pbytCode + iNextCode + 2)) ;
			pLabel = m_lstLabelAddr.GetAs( nJumpTarget ) ;
			ESLAssert( pLabel != NULL ) ;
			if ( pLabel == NULL )
			{
				size_t	nIndex =
					m_lstLabelAddr.Add( nJumpTarget, new JumpAddress() ) ;
				pLabel = m_lstLabelAddr.GetAt( nIndex ) ;
			}
			if ( bytCode == codeCNJumpOffset32 )
			{
				pJumpFrom =
					m_pAssemberCodes->write_cnjump_reg_offset32
									( m_pbytCode[iNextCode + 1] ) ;
			}
			else
			{
				pJumpFrom =
					m_pAssemberCodes->write_cjump_reg_offset32
									( m_pbytCode[iNextCode + 1] ) ;
			}
			pLabel->lstJumpFrom.Add( pJumpFrom ) ;
			fPair = false ;
			break ;

		// コール命令
		////////////////////////////////////////////////////////////////////////////
		case	codeCallImm32:
			m_pAssemberCodes->FlushAllRegisters() ;
			pEscJumpFrom =
				m_pAssemberCodes->write_push_ip
					( m_dwFuncAddr + iNextCode + inf.nBytes ) ;
			if ( pEscJumpFrom != NULL )
			{
				m_pAssemberCodes->CommitJumpTarget
					( pEscJumpFrom, m_pAssemberGates->GetNextAddress() ) ;
				m_pAssemberGates->WriteToAtomicOrExceptionMask( exceptionExtendStack ) ;
				m_pAssemberGates->WriteEpilogue( ipEscCurrent ) ;
				pEscJumpFrom = NULL ;
			}
			ipEscException = -1 ;
			//
			m_pAssemberCodes->FlushAllRegisters() ;
			m_pAssemberCodes->WriteEpilogue
				( *((SDWORD*)(m_pbytCode + iNextCode + 1)) ) ;
			//
			m_pAssemberCodes->ResetAllRegisters() ;
			fDelayEnter = true ;
			fPair = false ;
			break ;

		// 間接コール命令
		////////////////////////////////////////////////////////////////////////////
		case	codeCallReg:
			m_pAssemberCodes->FlushAllRegisters() ;
			pEscJumpFrom =
				m_pAssemberCodes->write_push_ip
					( m_dwFuncAddr + iNextCode + inf.nBytes ) ;
			if ( pEscJumpFrom != NULL )
			{
				m_pAssemberCodes->CommitJumpTarget
					( pEscJumpFrom, m_pAssemberGates->GetNextAddress() ) ;
				m_pAssemberGates->WriteToAtomicOrExceptionMask( exceptionExtendStack ) ;
				m_pAssemberGates->WriteEpilogue( ipEscCurrent ) ;
			}
			m_pAssemberCodes->write_jump_reg( m_pbytCode[iNextCode + 1] ) ;
			//
			ipEscException = -1 ;
			//
			m_pAssemberCodes->FlushAllRegisters() ;
			m_pAssemberCodes->WriteEpilogue() ;
			//
			m_pAssemberCodes->ResetAllRegisters() ;
			fDelayEnter = true ;
			fPair = false ;
			break ;

		// システムコール命令
		////////////////////////////////////////////////////////////////////////////
		case	codeSysCallImm32:
			m_pAssemberCodes->write_syscall_imm
				( *((SDWORD*)(m_pbytCode + iNextCode + 1)) ) ;
			m_pAssemberCodes->FlushAllRegisters() ;
			m_pAssemberCodes->WriteEpilogue
				( m_dwFuncAddr + iNextCode + inf.nBytes ) ;
			m_pAssemberCodes->ResetAllRegisters() ;
			fDelayEnter = true ;
			fPair = false ;
			break ;

		case	codeSysCallReg:
			m_pAssemberCodes->write_syscall_reg
				( m_pbytCode[iNextCode + 1] ) ;
			m_pAssemberCodes->FlushAllRegisters() ;
			m_pAssemberCodes->WriteEpilogue
				( m_dwFuncAddr + iNextCode + inf.nBytes ) ;
			m_pAssemberCodes->ResetAllRegisters() ;
			fDelayEnter = true ;
			fPair = false ;
			break ;

		// リターン命令
		////////////////////////////////////////////////////////////////////////////
		case	codeReturn:
			m_pAssemberCodes->write_return() ;
			m_pAssemberCodes->FlushAllRegisters() ;
			m_pAssemberCodes->WriteEpilogue() ;
			m_pAssemberCodes->ResetAllRegisters() ;
			fDelayEnter = true ;
			fPair = false ;
			break ;

		// スタック命令
		////////////////////////////////////////////////////////////////////////////
		case	codePushReg:
			pEscJumpFrom =
				m_pAssemberCodes->write_push_reg( m_pbytCode[iNextCode + 1], 1 ) ;
			if ( pEscJumpFrom != NULL )
			{
				m_pAssemberCodes->CommitJumpTarget
					( pEscJumpFrom, m_pAssemberGates->GetNextAddress() ) ;
				m_pAssemberGates->WriteToAtomicOrExceptionMask( exceptionExtendStack ) ;
				m_pAssemberGates->WriteEpilogue( ipEscCurrent ) ;
				pEscJumpFrom = NULL ;
			}
			fPair = false ;
			break ;
		case	codePopReg:
			pEscJumpFrom =
				m_pAssemberCodes->write_pop_reg( m_pbytCode[iNextCode + 1], 1 ) ;
			if ( pEscJumpFrom != NULL )
			{
				m_pAssemberCodes->CommitJumpTarget
					( pEscJumpFrom, m_pAssemberGates->GetNextAddress() ) ;
				m_pAssemberGates->WriteToAtomicOrExceptionMask( exceptionExtendStack ) ;
				m_pAssemberGates->WriteEpilogue( ipEscCurrent ) ;
				pEscJumpFrom = NULL ;
			}
			fPair = false ;
			break ;
		case	codePushRegs:
			pEscJumpFrom =
				m_pAssemberCodes->write_push_reg
					( m_pbytCode[iNextCode + 1], m_pbytCode[iNextCode + 2] ) ;
			if ( pEscJumpFrom != NULL )
			{
				m_pAssemberCodes->CommitJumpTarget
					( pEscJumpFrom, m_pAssemberGates->GetNextAddress() ) ;
				m_pAssemberGates->WriteToAtomicOrExceptionMask( exceptionExtendStack ) ;
				m_pAssemberGates->WriteEpilogue( ipEscCurrent ) ;
				pEscJumpFrom = NULL ;
			}
			fPair = false ;
			break ;
		case	codePopRegs:
			pEscJumpFrom =
				m_pAssemberCodes->write_pop_reg
					( m_pbytCode[iNextCode + 1], m_pbytCode[iNextCode + 2] ) ;
			if ( pEscJumpFrom != NULL )
			{
				m_pAssemberCodes->CommitJumpTarget
					( pEscJumpFrom, m_pAssemberGates->GetNextAddress() ) ;
				m_pAssemberGates->WriteToAtomicOrExceptionMask( exceptionExtendStack ) ;
				m_pAssemberGates->WriteEpilogue( ipEscCurrent ) ;
				pEscJumpFrom = NULL ;
			}
			fPair = false ;
			break ;

		// 特殊命令
		////////////////////////////////////////////////////////////////////////////
		case	codeMemoryHint:
			switch ( m_pbytCode[iNextCode+1] )
			{
			case	mhcodePrefetchTLB0:
				pEscJumpFrom =
					m_pAssemberCodes->write_prefetch_tlb
								( 0, m_pbytCode[iNextCode+2] ) ;
				dwEscException = exceptionReadMemory ;
				break ;
			case	mhcodePrefetchTLB1:
				pEscJumpFrom =
					m_pAssemberCodes->write_prefetch_tlb
								( 1, m_pbytCode[iNextCode+2] ) ;
				dwEscException = exceptionReadMemory ;
				break ;
			case	mhcodeUnfetchTLB0:
				m_pAssemberCodes->write_unfetch_tlb
							( 0, m_pbytCode[iNextCode+2] ) ;
				break ;
			case	mhcodeUnfetchTLB1:
				m_pAssemberCodes->write_unfetch_tlb
							( 1, m_pbytCode[iNextCode+2] ) ;
				break ;
			}
			fPair = false ;
			break ;

		case	codeFloatExtension:
			m_pAssemberCodes->write_float_extension
				( m_pbytCode[iNextCode + 1],
					m_pbytCode[iNextCode + 2], m_pbytCode[iNextCode + 3] ) ;
			fPair = false ;
			break ;

		case	codeSIMD64Extension2Op:
			m_pAssemberCodes->write_simd64_extension
				( m_pbytCode[iNextCode + 1],
					m_pbytCode[iNextCode + 2], m_pbytCode[iNextCode + 3], fPair ) ;
			break ;

		case	codeSIMD64Extension3Op:
			m_pAssemberCodes->write_simd64_imm_extension
				( m_pbytCode[iNextCode + 1], m_pbytCode[iNextCode + 2],
					m_pbytCode[iNextCode + 3], m_pbytCode[iNextCode + 4], fPair ) ;
			break ;

		case	codeSIMD128Extension2Op:
			m_pAssemberCodes->write_simd128_extension
				( m_pbytCode[iNextCode + 1],
					m_pbytCode[iNextCode + 2], m_pbytCode[iNextCode + 3] ) ;
			fPair = false ;
			fDoubleRegister = true ;
			break ;

		case	codeSIMD128Extension3Op:
			m_pAssemberCodes->write_simd128_imm_extension
				( m_pbytCode[iNextCode + 1], m_pbytCode[iNextCode + 2],
					m_pbytCode[iNextCode + 3], m_pbytCode[iNextCode + 4] ) ;
			fPair = false ;
			fDoubleRegister = true ;
			break ;

		case	codeNoOperation:
			fPair = false ;
			break ;

		default:
			m_pAssemberCodes->FlushAllRegisters() ;
			m_pAssemberCodes->WriteEpilogue( ipEscCurrent ) ;
			m_pAssemberCodes->ResetAllRegisters() ;
			fDelayEnter = true ;
			fPair = false ;
			break ;
		}
		#if	defined(__DEBUG__)
		m_pAssemberCodes->OnDebugAfterInstruction( bytCode, ipEscCurrent ) ;
		#endif
		//
		// 特定機能レジスタ更新
		////////////////////////////////////////////////////////////////////////////
		const bool	fInitBP = m_fInitBP ;
		if ( ((inf.regDst == regBP)
				|| ((fPair | fDoubleRegister)
						&& ((inf.regDst + 1) == regBP))) && m_mbRangeBP.flagUsed )
		{
			if ( m_pbytCode[iNextCode + inf.nBytes] != codeReturn )
			{
				m_pAssemberCodes->WriteAssignLocalBasePointer
					( m_mbRangeBP.minOffset,
						m_mbRangeBP.maxOffset, m_pAssemberGates->GetNextAddress() ) ;
				m_pAssemberGates->WriteToAtomicOrExceptionMask
									( dwEscException | exceptionReadMemory ) ;
				m_pAssemberGates->WriteEpilogue( ipEscException ) ;
				m_fInitBP = true ;
			}
		}
		else if ( inf.regDst != -1 )
		{
			m_pAssemberCodes->ModifiedRegister( inf.regDst ) ;
			//
			if ( fPair | fDoubleRegister )
			{
				m_pAssemberCodes->ModifiedRegister( inf.regDst + 1 ) ;
			}
		}
		//
		// 脱出コード生成
		////////////////////////////////////////////////////////////////////////////
		if ( pEscJumpFrom != NULL )
		{
			m_pAssemberCodes->CommitJumpTarget
				( pEscJumpFrom, m_pAssemberGates->GetNextAddress() ) ;
			if ( dwEscException != 0 )
			{
				m_pAssemberGates->WriteToAtomicOrExceptionMask( dwEscException ) ;
				dwEscException = 0 ;
			}
			m_pAssemberGates->WriteEpilogue( ipEscException ) ;
		}
		//
		// 現在の命令にジャンプインする
		////////////////////////////////////////////////////////////////////////////
		if ( fCurrentEnter )
		{
			if ( ((iLastEnterPos < 0)
					|| ((iNextCode - iLastEnterPos) >= sizeof(void*)))
				&& ((iNextCode + sizeof(void*)) <= m_dwFuncSize) )
			{
				m_pbytCode[iNextCode] = codeSystemReserved ;
				*((void**)(m_pbytTrickBuf + iNextCode)) =
									m_pAssemberGates->GetNextAddress() ;
				iLastEnterPos = iNextCode ;
				//
				void *	pEscBPJumpFrom = NULL ;
				m_pAssemberGates->WritePrologue() ;
				if ( fInitBP && m_mbRangeBP.flagUsed )
				{
					pEscBPJumpFrom =
						m_pAssemberGates->WriteAssignLocalBasePointer
							( m_mbRangeBP.minOffset,
								m_mbRangeBP.maxOffset, NULL ) ;
				}
				m_pAssemberGates->WriteToJump( ptrCurrentCodePos ) ;
				//
				if ( pEscBPJumpFrom != NULL )
				{
					m_pAssemberGates->CommitJumpTarget
						( pEscBPJumpFrom, m_pAssemberGates->GetNextAddress() ) ;
					m_pAssemberGates->WriteToAtomicOrExceptionMask( exceptionReadMemory ) ;
					m_pAssemberGates->WriteEpilogue( ipEscCurrent ) ;
				}
			}
			else
			{
				m_pAssemberCodes->FlushAllRegisters() ;
				m_pAssemberCodes->ResetAllRegisters() ;
				fDelayEnter = true ;
			}
		}
		//
		// 次の命令へ
		////////////////////////////////////////////////////////////////////////////
		iNextCode += inf.nBytes ;
		if ( fPair )
		{
			inf.nFlags = 0 ;
			GetInstructionInfo( &inf, m_pbytCode + iNextCode ) ;
			iNextCode += inf.nBytes ;
		}
	}
	//
	// 関数内ジャンプ完成
	//
	const int	nLabelCount = (int) m_lstLabelAddr.GetLength() ;
	for ( int i = 0; i < nLabelCount; i ++ )
	{
		JumpAddress *	pLabel = m_lstLabelAddr.GetAt( i ) ;
		if ( pLabel != NULL )
		{
			const int	nRefCount = (int) pLabel->lstJumpFrom.GetLength() ;
			for ( int j = 0; j < nRefCount; j ++ )
			{
				m_pAssemberCodes->CommitJumpTarget
					( pLabel->lstJumpFrom[j], pLabel->ptrNative ) ;
			}
		}
	}
	#if	defined(__DEBUG__)
	m_pAssemberCodes->OnDebugAfterFunction( m_dwFuncAddr, m_dwFuncSize ) ;
	m_pAssemberGates->OnDebugAfterFunction( m_dwFuncAddr, m_dwFuncSize ) ;
	#endif
}

// ペア命令を判定する
//////////////////////////////////////////////////////////////////////////////
bool ECSSakura2JIT::NativeCompiler::IsPairInstruction
	( const InstructionInfo& inf, const BYTE * pbytCode )
{
	const BYTE *	pbytNext = pbytCode + inf.nBytes ;
	if ( pbytCode[0] == pbytNext[0] )
	{
		switch ( pbytCode[0] )
		{
		case	codeLoadMemBase:
		case	codeStoreMemBase:
			break ;

		case	codeLoadMemBaseImm32:
		case	codeLoadLocalImm32:
		case	codeStoreMemBaseImm32:
		case	codeStoreLocalImm32:
			if ( (pbytCode[1] & 0x07) == dataInt64 )
			{
				return	(pbytCode[1] == pbytNext[1])
						&& !(pbytCode[2] & 0x01)
						&& ((pbytCode[2] + 1) == pbytNext[2])
						&& ((*((DWORD*)(pbytCode + 3)) + 8)
									== *((DWORD*)(pbytNext + 3))) ;
			}
			break ;

		case	codeLoadMemBaseIndex:
		case	codeStoreMemBaseIndex:
			break ;

		case	codeLoadMemBaseIndexImm32:
		case	codeLoadLocalIndexImm32:
		case	codeStoreMemBaseIndexImm32:
		case	codeStoreLocalIndexImm32:
			return	((pbytCode[1] & 0x07) == dataInt64)
					&& (pbytCode[1] == pbytNext[1])
					&& !(pbytCode[3] & 0x01)
					&& (pbytCode[2] == pbytNext[2])
					&& ((pbytCode[3] + 1) == pbytNext[3])
					&& ((*((DWORD*)(pbytCode + 4)) + 8)
								== *((DWORD*)(pbytNext + 4))) ;

		case	codeMoveReg:
		case	codeAddReg:
		case	codeSubReg:
		case	codeMulReg:
		case	codeAndReg:
		case	codeOrReg:
		case	codeXorReg:
		case	codeFAddReg:
		case	codeFSubReg:
		case	codeFMulReg:
		case	codeFDivReg:
		case	codeCmpNeReg:
		case	codeCmpEqReg:
		case	codeCmpLtReg:
		case	codeCmpLeReg:
		case	codeCmpGtReg:
		case	codeCmpGeReg:
		case	codeCmpCReg:
		case	codeCmpCZReg:
		case	codeFCmpNeReg:
		case	codeFCmpEqReg:
		case	codeFCmpLtReg:
		case	codeFCmpLeReg:
		case	codeFCmpGtReg:
		case	codeFCmpGeReg:
			return	!(pbytCode[1] & 0x01) && !(pbytCode[2] & 0x01)
						&& ((pbytCode[1] + 1) == pbytNext[1])
						&& ((pbytCode[2] + 1) == pbytNext[2]) ;

		case	codeSrlImm8:
		case	codeSraImm8:
		case	codeSllImm8:
			return	!(pbytCode[1] & 0x01) && !(pbytCode[2] & 0x01)
						&& ((pbytCode[1] + 1) == pbytNext[1])
						&& ((pbytCode[2] + 1) == pbytNext[2])
						&& (pbytCode[3] == pbytNext[3]) ;

		case	codeMaskMove:
			return	!(pbytCode[1] & 0x01)
						&& !(pbytCode[2] & 0x01)
						&& !(pbytCode[3] & 0x01)
						&& ((pbytCode[1] + 1) == pbytNext[1])
						&& ((pbytCode[2] + 1) == pbytNext[2])
						&& ((pbytCode[3] + 1) == pbytNext[3]) ;

		case	codeSrlReg:
		case	codeSraReg:
		case	codeSllReg:
			return	!(pbytCode[1] & 0x01)
						&& ((pbytCode[1] + 1) == pbytNext[1])
						&& (pbytCode[2] == pbytNext[2]) ;

		case	codeAddImm32:
		case	codeMulImm32:
			return	!(pbytCode[1] & 0x01) && !(pbytCode[2] & 0x01)
						&& ((pbytCode[1] + 1) == pbytNext[1])
						&& ((pbytCode[2] + 1) == pbytNext[2])
						&& (pbytCode[3] == pbytNext[3])
						&& (pbytCode[4] == pbytNext[4])
						&& (pbytCode[5] == pbytNext[5])
						&& (pbytCode[6] == pbytNext[6]) ;

		case	codeSIMD64Extension2Op:
			if ( pbytCode[1] == pbytNext[1] )
			{
				switch ( pbytCode[1] & 0xF8 )
				{
				case	simdPadd:
				case	simdPadd | 0x08:
				case	simdPsub:
				case	simdPsub | 0x08:
				case	simdPcmpnesb:
				case	simdPcmpeqsb:
				case	simdPcmpltsb:
				case	simdPcmplesb:
				case	simdPcmpgtsb:
				case	simdPcmpgesb:
				case	simdPmullw:
					return	!(pbytCode[2] & 0x01) && !(pbytCode[3] & 0x01)
							&& ((pbytCode[2] + 1) == pbytNext[2])
							&& ((pbytCode[3] + 1) == pbytNext[3]) ;

				case	simdPsrlw:
				case	simdPsraw:
				case	simdPsllw:
					return	!(pbytCode[2] & 0x01) 
								&& ((pbytCode[2] + 1) == pbytNext[2])
								&& (pbytCode[3] == pbytNext[3]) ;
				}
			}
			break ;

		case	codeSIMD64Extension3Op:
			return	(pbytCode[1] == pbytNext[1])
					&& 	!(pbytCode[2] & 0x01) && !(pbytCode[3] & 0x01)
					&& ((pbytCode[2] + 1) == pbytNext[2])
					&& ((pbytCode[3] + 1) == pbytNext[3])
					&& (pbytCode[4] == pbytNext[4]) ;
		}
	}
	switch ( pbytCode[0] )
	{
	case	codeLoadMemBase:
		if ( (pbytCode[1] & 0x07) == dataInt64 )
		{
			return	(pbytNext[0] == codeLoadMemBaseImm32)
					&& (pbytCode[1] == pbytNext[1])
					&& !(pbytCode[2] & 0x01)
					&& ((pbytCode[2] + 1) == pbytNext[2])
					&& (*((DWORD*)(pbytNext + 3)) == 8) ;
		}
	case	codeLoadMemBaseImm32:
	case	codeLoadLocalImm32:
		if ( (pbytCode[1] & 0x07) == dataUint32 )
		{
			return	(pbytNext[0] == codeMoveReg)
					&& (pbytNext[2] == regIntZero)
					&& !(pbytCode[2] & 0x01)
					&& ((pbytCode[2] + 1) == pbytNext[1]) ;
		}
		break ;

	case	codeStoreMemBase:
		if ( (pbytCode[1] & 0x07) == dataInt64 )
		{
			return	(pbytNext[0] == codeStoreMemBaseImm32)
					&& (pbytCode[1] == pbytNext[1])
					&& !(pbytCode[2] & 0x01)
					&& ((pbytCode[2] + 1) == pbytNext[2])
					&& (*((DWORD*)(pbytNext + 3)) == 8) ;
		}
		break ;

	case	codeLoadMemBaseIndex:
		if ( (pbytCode[1] & 0x07) == dataInt64 )
		{
			return	(pbytNext[0] == codeLoadMemBaseIndexImm32)
					&& (pbytCode[1] == pbytNext[1])
					&& (pbytCode[2] == pbytNext[2])
					&& !(pbytCode[3] & 0x01)
					&& ((pbytCode[3] + 1) == pbytNext[3])
					&& (*((DWORD*)(pbytNext + 4)) == 8) ;
		}
	case	codeLoadMemBaseIndexImm32:
	case	codeLoadLocalIndexImm32:
		if ( (pbytCode[1] & 0x07) == dataUint32 )
		{
			return	(pbytNext[0] == codeMoveReg)
					&& (pbytNext[2] == regIntZero)
					&& !(pbytCode[3] & 0x01)
					&& ((pbytCode[3] + 1) == pbytNext[1]) ;
		}
		break ;

	case	codeStoreMemBaseIndex:
		if ( (pbytCode[1] & 0x07) == dataInt64 )
		{
			return	(pbytNext[0] == codeStoreMemBaseIndexImm32)
					&& (pbytCode[1] == pbytNext[1])
					&& (pbytCode[2] == pbytNext[2])
					&& !(pbytCode[3] & 0x01)
					&& ((pbytCode[3] + 1) == pbytNext[3])
					&& (*((DWORD*)(pbytNext + 4)) == 8) ;
		}
		break ;
	}
	return	false ;
}

