
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/


#include <sakuraglx/sakuraglx.h>
#include <sakura/ssys_module.h>
#include <glscs/glscs_sakura2_jit_x86_compiler.h>
#include <glscs/glscs_sakura2_jit_sse2_compiler.h>

using	namespace ECSSakura2JIT ;
using	namespace ECSSakura2Processor ;

#if	defined(__PROCESSOR_INTEL_X86__)

//////////////////////////////////////////////////////////////////////////////
// x86 SSE2 ネイティブコード化アセンブラ (IA32)
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
X86SSE2Assembler::X86SSE2Assembler( void )
{
	int	i ;
	for ( i = 0; i < 0x100; i ++ )
	{
		m_rdtSakura[i] = regTypeQWord ;
	}
	for ( i = 0; i < 0x10; i ++ )
	{
		m_lruDataReg[i].regPhy = i ;
		m_lruDataReg[i].typeRegData = regTypeQWord ;
	}
	m_pConst64PairSignMask = NULL ;
	m_pConst64PairNoSignMask = NULL ;
	m_pConst64Pair80000000 = NULL ;
	m_pConst128Mask0F = NULL ;
	m_pConst128Mask1F = NULL ;
	m_pConst128Mask3F = NULL ;
	m_pConst32NoSignMask = NULL ;
	m_pConst32PackNoSignMask = NULL ;
}

// 定数値 64bit 最上位ビット 8000000000000000H ペア
//////////////////////////////////////////////////////////////////////////////
void * X86SSE2Assembler::GetConstantPair8000000000000000( void )
{
	if ( m_pConst64PairSignMask == NULL )
	{
		UINT64 * pDataSignMask = (UINT64*) m_buf->AllocateData( 0x10, 0x10 ) ;
		pDataSignMask[0] = 0x8000000000000000 ;
		pDataSignMask[1] = 0x8000000000000000 ;
		m_pConst64PairSignMask = pDataSignMask ;
	}
	return	m_pConst64PairSignMask ;
}

// 定数値 64bit 7FFFFFFFFFFFFFFFH ペア
//////////////////////////////////////////////////////////////////////////////
void * X86SSE2Assembler::GetConstantPair7FFFFFFFFFFFFFFF( void )
{
	if ( m_pConst64PairNoSignMask == NULL )
	{
		UINT64 * pDataNoSignMask = (UINT64*) m_buf->AllocateData( 0x10, 0x10 ) ;
		pDataNoSignMask[0] = 0x7FFFFFFFFFFFFFFF ;
		pDataNoSignMask[1] = 0x7FFFFFFFFFFFFFFF ;
		m_pConst64PairNoSignMask = pDataNoSignMask ;
	}
	return	m_pConst64PairNoSignMask ;
}

// 定数値 64bit 80000000H ペア
//////////////////////////////////////////////////////////////////////////////
void * X86SSE2Assembler::GetConstantPair80000000( void )
{
	if ( m_pConst64Pair80000000 == NULL )
	{
		UINT64 * pDataSignMask = (UINT64*) m_buf->AllocateData( 0x10, 0x10 ) ;
		pDataSignMask[0] = 0x80000000UL ;
		pDataSignMask[1] = 0x80000000UL ;
		m_pConst64Pair80000000 = pDataSignMask ;
	}
	return	m_pConst64Pair80000000 ;
}

// 定数値 128bit 0FH
//////////////////////////////////////////////////////////////////////////////
void * X86SSE2Assembler::GetConstantDQWord0F( void )
{
	if ( m_pConst128Mask0F == NULL )
	{
		UINT64 * pData0F = (UINT64*) m_buf->AllocateData( 0x10, 0x10 ) ;
		pData0F[0] = 0x0F ;
		pData0F[1] = 0 ;
		m_pConst128Mask0F = pData0F ;
	}
	return	m_pConst128Mask0F ;
}

// 定数値 128bit 1FH
//////////////////////////////////////////////////////////////////////////////
void * X86SSE2Assembler::GetConstantDQWord1F( void )
{
	if ( m_pConst128Mask1F == NULL )
	{
		UINT64 * pData1F = (UINT64*) m_buf->AllocateData( 0x10, 0x10 ) ;
		pData1F[0] = 0x1F ;
		pData1F[1] = 0 ;
		m_pConst128Mask1F = pData1F ;
	}
	return	m_pConst128Mask1F ;
}

// 定数値 128bit 3FH
//////////////////////////////////////////////////////////////////////////////
void * X86SSE2Assembler::GetConstantDQWord3F( void )
{
	if ( m_pConst128Mask3F == NULL )
	{
		UINT64 * pData3F = (UINT64*) m_buf->AllocateData( 0x10, 0x10 ) ;
		pData3F[0] = 0x3F ;
		pData3F[1] = 0 ;
		m_pConst128Mask3F = pData3F ;
	}
	return	m_pConst128Mask3F ;
}

// 定数値 32bit 7FFFFFFFH : -1 : -1 : -1
//////////////////////////////////////////////////////////////////////////////
void * X86SSE2Assembler::GetConstantLow32NoSignMask( void )
{
	if ( m_pConst32NoSignMask == NULL )
	{
		DWORD * pDataMask = (DWORD*) m_buf->AllocateData( 0x10, 0x10 ) ;
		pDataMask[0] = 0x7FFFFFFF ;
		pDataMask[1] = 0xFFFFFFFF ;
		pDataMask[2] = 0xFFFFFFFF ;
		pDataMask[3] = 0xFFFFFFFF ;
		m_pConst32NoSignMask = pDataMask ;
	}
	return	m_pConst32NoSignMask ;
}

// 定数値 32bit 7FFFFFFFH ペア
//////////////////////////////////////////////////////////////////////////////
void * X86SSE2Assembler::GetConstantPack32NoSignMask( void )
{
	if ( m_pConst32PackNoSignMask == NULL )
	{
		DWORD * pDataMask = (DWORD*) m_buf->AllocateData( 0x10, 0x10 ) ;
		pDataMask[0] = 0x7FFFFFFF ;
		pDataMask[1] = 0x7FFFFFFF ;
		pDataMask[2] = 0x7FFFFFFF ;
		pDataMask[3] = 0x7FFFFFFF ;
		m_pConst32PackNoSignMask = pDataMask ;
	}
	return	m_pConst32PackNoSignMask ;
}

// データ型を64ビットに正規化
//////////////////////////////////////////////////////////////////////////////
X86SSE2Assembler::RegisterDataType
		X86SSE2Assembler::NormalizeDataTypeTo64
				( X86SSE2Assembler::RegisterDataType regType )
{
	switch ( regType )
	{
	case	regTypeQWord:
	case	regTypeDQWord:
	case	regTypeQFloat32:
	default:
		regType = regTypeQWord ;
		break ;
	case	regTypeFloat64:
	case	regTypeDFloat64:
		regType = regTypeFloat64 ;
		break ;
		break ;
	}
	return	regType ;
}

// データ型を128ビットに正規化
//////////////////////////////////////////////////////////////////////////////
X86SSE2Assembler::RegisterDataType
		X86SSE2Assembler::NormalizeDataTypeTo128
				( X86SSE2Assembler::RegisterDataType regType )
{
	switch ( regType )
	{
	case	regTypeQWord:
	case	regTypeDQWord:
		regType = regTypeDQWord ;
		break ;
	case	regTypeFloat64:
	case	regTypeDFloat64:
		regType = regTypeDFloat64 ;
		break ;
	case	regTypeQFloat32:
	default:
		regType = regTypeQFloat32 ;
		break ;
	}
	return	regType ;
}

// Sakura2 レジスタを SSE 物理レジスタに割り当て／ロード
//////////////////////////////////////////////////////////////////////////////
X86SSE2Assembler::SSERegister
	 X86SSE2Assembler::WriteRealizeDataRegister
		( int regSakura, X86SSE2Assembler::RegisterDataType regType, bool fLoad )
{
	//
	// 割り当て済み物理レジスタを取得する
	//
	SSERegister	xmmReg = GetRealizedDataRegister( regSakura, regType, fLoad ) ;
	if ( xmmReg != XMM_Nothing )
	{
		return	xmmReg ;
	}
	//
	// 新規に物理レジスタを割り当てる
	//
	xmmReg = AllocateDataRegister( regType ) ;
	m_rdtSakura[regSakura] = regType ;
	m_lruDataReg[xmmReg << 1].regSakura = regSakura ;
	if ( regType >= regTypeFirst128 )
	{
		m_rdtSakura[regSakura & ~0x01] = regType ;
		m_rdtSakura[regSakura | 0x01] = regType ;
		m_lruDataReg[(xmmReg << 1) + 1].regSakura = regSakura + 1 ;
	}
	if ( fLoad )
	{
		switch ( regType )
		{
		case	regTypeQWord:
			WriteSSERegMemOperand
				( sseop_MOVQ_LOAD, 3, xmmReg,
					x86_EBX, Context::OffsetOfReg(regSakura) ) ;
			break ;
		case	regTypeFloat64:
			WriteSSERegMemOperand
				( sseop_MOVSD_LOAD, 3, xmmReg,
					x86_EBX, Context::OffsetOfReg(regSakura) ) ;
			break ;
		case	regTypeDQWord:
			WriteSSERegMemOperand
				( sseop_MOVDQA_LOAD, 3, xmmReg,
					x86_EBX, Context::OffsetOfReg(regSakura & ~0x01) ) ;
			break ;
		case	regTypeDFloat64:
			WriteSSERegMemOperand
				( sseop_MOVAPD_LOAD, 3, xmmReg,
					x86_EBX, Context::OffsetOfReg(regSakura & ~0x01) ) ;
			break ;
		case	regTypeQFloat32:
			WriteSSERegMemOperand
				( sseop_MOVAPS_LOAD, 2, xmmReg,
					x86_EBX, Context::OffsetOfReg(regSakura & ~0x01) ) ;
			break ;
		}
	}
	return	xmmReg ;
}

// Sakura2 レジスタを割り当て済みの SSE 物理レジスタを取得
// 取得データ型によっては正規化
//////////////////////////////////////////////////////////////////////////////
X86SSE2Assembler::SSERegister
	X86SSE2Assembler::GetRealizedDataRegister
		( int regSakura, X86SSE2Assembler::RegisterDataType regType, bool fNormalize )
{
	const bool	fPair = (regType >= regTypeDQWord) ;
	ESLAssert( !(regSakura & 0x01) || !fPair ) ;
	//
	// 割り当て済み物理レジスタ検索
	//
	int	iSlot = m_lruDataReg.FindAssignedSlot( regSakura ) ;
	if ( iSlot < 0 )
	{
		if ( fPair )
		{
			int	iSlotHigh =
					m_lruDataReg.FindAssignedSlot( regSakura | 0x01 ) ;
			if ( iSlotHigh >= 0 )
			{
				//
				// 128ビットレジスタの要求に対して、
				// 既に奇数（上位）レジスタのみ割り当てられている場合
				//
				SSERegister	xmmReg = (SSERegister) (iSlotHigh >> 1) ;
				SSERegister	xmmLowTemp ;
				if ( fNormalize )
				{
					switch ( regType )
					{
					case	regTypeQWord:
					case	regTypeDQWord:
						m_lruDataReg.LockSlot( iSlotHigh ) ;
						xmmLowTemp =
							WriteRealizeDataRegister
								( regSakura, regTypeQWord ) ;
						WriteSSERegRegOperand
							( sseop_PUNPCKLQDQ, 3, xmmLowTemp, xmmReg ) ;
						WriteSSERegRegOperand
							( sseop_MOVDQA_LOAD, 3, xmmReg, xmmLowTemp ) ;
						m_lruDataReg.UnlockSlot( iSlotHigh ) ;
						FreeDataRegister( xmmLowTemp, regTypeQWord ) ;
						break ;
					case	regTypeFloat64:
					case	regTypeDFloat64:
						WriteSSERegMemOperand
							( sseop_MOVLPD_STORE, 3, xmmReg,
								x86_EBX, Context::OffsetOfReg(regSakura | 0x01) ) ;
						WriteSSERegMemOperand
							( sseop_MOVAPD_LOAD, 3, xmmReg,
								x86_EBX, Context::OffsetOfReg(regSakura) ) ;
						break ;
					default:
					case	regTypeQFloat32:
						WriteSSERegRegOperand
							( sseop_MOVLHPS, 2, xmmReg, xmmReg ) ;
						WriteSSERegMemOperand
							( sseop_MOVLPS_LOAD, 2, xmmReg,
								x86_EBX, Context::OffsetOfReg(regSakura) ) ;
						break ;
					}
				}
				ESLAssert( !(iSlotHigh & 0x01) ) ;
				ESLAssert( !m_lruDataReg.IsSlotAssigned(iSlotHigh | 0x01) ) ;
				m_lruDataReg[iSlotHigh].regSakura = regSakura ;
				m_lruDataReg.NewSlot( iSlotHigh | 0x01, regSakura | 0x01 ) ;
				m_rdtSakura[regSakura] = regType ;
				m_rdtSakura[regSakura | 0x01] = regType ;
				m_lruDataReg[iSlotHigh & ~0x01].typeRegData = regType;
				m_lruDataReg[iSlotHigh | 0x01].typeRegData = regType;
				LockDataRegister( xmmReg, regType ) ;
				return	xmmReg ;
			}
		}
		return	XMM_Nothing ;
	}
	SSERegister	xmmReg = (SSERegister) (iSlot >> 1) ;
	const bool	fRegPair =
		(m_lruDataReg[iSlot].typeRegData >= regTypeFirst128) ;
	if ( (fPair && fRegPair) || (!fPair && !fRegPair) )
	{
		//
		// 既に物理レジスタに割り当てられていて
		// データサイズも同一の場合
		// データ形式を更新してレジスタ番号を返却
		//
		m_rdtSakura[regSakura] = regType ;
		if ( fPair )
		{
			ESLAssert( !(regSakura & 0x01) ) ;
			m_rdtSakura[regSakura | 0x01] = regType ;
		}
		LockDataRegister( xmmReg, regType ) ;
		return	xmmReg ;
	}
	if ( fPair )
	{
		ESLAssert( !fRegPair ) ;
		ESLAssert( !(iSlot & 0x01) ) ;
		ESLAssert( !m_lruDataReg.IsSlotAssigned( iSlot | 0x01 ) ) ;
		int	iSlotHigh =
			m_lruDataReg.FindAssignedSlot( regSakura | 0x01 ) ;
		if ( iSlotHigh >= 0 )
		{
			ESLAssert( !(iSlotHigh & 0x01) ) ;
			//
			// 割り当て済みの下位64ビットと上位64ビットを組み合わせ
			// 一つの128ビットレジスタとして返却する
			//
			if ( fNormalize )
			{
				SSERegister	xmmRegHigh = (SSERegister) (iSlotHigh >> 1) ;
				switch ( regType )
				{
				case	regTypeQWord:
				case	regTypeDQWord:
					WriteSSERegRegOperand
						( sseop_PUNPCKLQDQ, 3, xmmReg, xmmRegHigh ) ;
					break ;
				case	regTypeFloat64:
				case	regTypeDFloat64:
					WriteSSERegMemOperand
						( sseop_MOVLPD_STORE, 3, xmmRegHigh,
							x86_EBX, Context::OffsetOfReg(regSakura | 0x01) ) ;
					WriteSSERegMemOperand
						( sseop_MOVHPD_LOAD, 3, xmmReg,
							x86_EBX, Context::OffsetOfReg(regSakura | 0x01) ) ;
					break ;
				default:
				case	regTypeQFloat32:
					WriteSSERegRegOperand
						( sseop_MOVLHPS, 2, xmmReg, xmmRegHigh ) ;
					break ;
				}
			}
			m_lruDataReg.FreeSlot( iSlotHigh ) ;
			m_lruDataReg.FreeSlot( iSlotHigh | 0x01 ) ;
		}
		else
		{
			//
			// 割り当て済みの下位64ビットに
			// 上位64ビットをロードし
			// 一つの128ビットレジスタとして返却する
			//
			SSERegister	xmmHighTemp ;
			if ( fNormalize )
			{
				switch ( regType )
				{
				case	regTypeQWord:
				case	regTypeDQWord:
					m_lruDataReg.LockSlot( iSlot ) ;
					xmmHighTemp =
						WriteRealizeDataRegister
							( regSakura | 0x01, regTypeQWord ) ;
					WriteSSERegRegOperand
						( sseop_PUNPCKLQDQ, 3, xmmReg, xmmHighTemp ) ;
					m_lruDataReg.UnlockSlot( iSlot ) ;
					FreeDataRegister( xmmHighTemp, regTypeQWord ) ;
					break ;
				case	regTypeFloat64:
				case	regTypeDFloat64:
					WriteSSERegMemOperand
						( sseop_MOVHPD_LOAD, 3, xmmReg,
							x86_EBX, Context::OffsetOfReg(regSakura | 0x01) ) ;
					break ;
				default:
				case	regTypeQFloat32:
					WriteSSERegMemOperand
						( sseop_MOVHPS_LOAD, 2, xmmReg,
							x86_EBX, Context::OffsetOfReg(regSakura | 0x01) ) ;
					break ;
				}
			}
		}
		m_lruDataReg.NewSlot( iSlot | 0x01, regSakura | 0x01 ) ;
		m_rdtSakura[regSakura] = regType ;
		m_rdtSakura[regSakura | 0x01] = regType ;
		m_lruDataReg[iSlot & ~0x01].typeRegData = regType;
		m_lruDataReg[iSlot | 0x01].typeRegData = regType;
		LockDataRegister( xmmReg, regType ) ;
		return	xmmReg ;
	}
	else
	{
		ESLAssert( fRegPair ) ;
		ESLAssert( m_lruDataReg.IsSlotAssigned( iSlot | 0x01 ) ) ;
		ESLAssert( m_lruDataReg.IsSlotAssigned( iSlot & ~0x01 ) ) ;
		if ( iSlot & 0x01 )
		{
			//
			// 128bit XMM の下位を64ビットを保存し
			// 上位を下位に移動し64ビットレジスタとして返却
			//
			ESLAssert( regSakura & 0x01 ) ;
			ESLAssert( m_lruDataReg[iSlot].numLocked == 0 ) ;
			ESLAssert( m_lruDataReg[iSlot | 0x01].numLocked == 0 ) ;
			switch ( m_lruDataReg[iSlot].typeRegData )
			{
			case	regTypeQWord:
			case	regTypeDQWord:
				WriteSSERegMemOperand
					( sseop_MOVQ_STORE, 3, xmmReg,
						x86_EBX, Context::OffsetOfReg(regSakura & ~0x01) ) ;
				if ( fNormalize )
				{
					WriteSSERegRegImm8Operand
						( sseop_PSHIFTDQ_IMM, 3, mmxop2nd_SRLDQ,
							xmmReg, 64 / 8 ) ;
				}
				break ;
			case	regTypeFloat64:
			case	regTypeDFloat64:
				WriteSSERegMemOperand
					( sseop_MOVLPD_STORE, 3, xmmReg,
						x86_EBX, Context::OffsetOfReg(regSakura & ~0x01) ) ;
				if ( fNormalize )
				{
					WriteSSERegRegOperand
						( sseop_SHUFPD, 3, xmmReg, xmmReg, 0x01, 1 ) ;
				}
				break ;
			default:
			case	regTypeQFloat32:
				WriteSSERegMemOperand
					( sseop_MOVLPS_STORE, 2, xmmReg,
						x86_EBX, Context::OffsetOfReg(regSakura & ~0x01) ) ;
				if ( fNormalize )
				{
					WriteSSERegRegOperand
						( sseop_MOVHLPS, 2, xmmReg, xmmReg ) ;
				}
				break ;
			}
			m_lruDataReg[iSlot & ~0x01].regSakura
						= m_lruDataReg[iSlot | 0x01].regSakura ;
			m_lruDataReg.FreeSlot( iSlot | 0x01 ) ;
			m_lruDataReg[iSlot & ~0x01].typeRegData = regType;
			m_rdtSakura[regSakura] = regType ;
			LockDataRegister( xmmReg, regType ) ;
			return	xmmReg ;
		}
		else
		{
			//
			// 128bit XMM の上位を64ビットを保存し
			// 下位を64ビットレジスタとして返却
			//
			ESLAssert( !(regSakura & 0x01) ) ;
			ESLAssert( m_lruDataReg[iSlot].numLocked == 0 ) ;
			ESLAssert( m_lruDataReg[iSlot | 0x01].numLocked == 0 ) ;
			switch ( m_lruDataReg[iSlot].typeRegData )
			{
			case	regTypeQWord:
			case	regTypeDQWord:
				WriteSSERegMemOperand
					( sseop_MOVDQA_STORE, 3, xmmReg,
						x86_EBX, Context::OffsetOfReg(regSakura & ~0x01) ) ;
				break ;
			case	regTypeFloat64:
			case	regTypeDFloat64:
				WriteSSERegMemOperand
					( sseop_MOVHPD_STORE, 3, xmmReg,
						x86_EBX, Context::OffsetOfReg(regSakura | 0x01) ) ;
				break ;
			default:
			case	regTypeQFloat32:
				WriteSSERegMemOperand
					( sseop_MOVHPS_STORE, 2, xmmReg,
						x86_EBX, Context::OffsetOfReg(regSakura | 0x01) ) ;
				break ;
			}
			m_lruDataReg.FreeSlot( iSlot | 0x01 ) ;
			m_lruDataReg[iSlot & ~0x01].typeRegData = regType;
			m_rdtSakura[regSakura] = regType ;
			LockDataRegister( xmmReg, regType ) ;
			return	xmmReg ;
		}
	}
}

// 一時処理のための物理レジスタを確保
//////////////////////////////////////////////////////////////////////////////
X86SSE2Assembler::SSERegister
	X86SSE2Assembler::AllocateDataRegister
		( X86SSE2Assembler::RegisterDataType regType )
{
	int	iSlot = m_lruDataReg.FindFreeSlot( 2 ) ;
	if ( iSlot < 0 )
	{
		iSlot = m_lruDataReg.FindMostOldSlot( 2 ) ;
		ESLAssert( iSlot >= 0 ) ;
		WriteBackDataRegister( (SSERegister) (iSlot >> 1) ) ;
	}
	m_lruDataReg.AddStale( 1 ) ;
	m_lruDataReg.NewSlot( iSlot, -1 ) ;
	m_lruDataReg[iSlot].typeRegData = regType ;
	//
	if ( regType >= regTypeFirst128 )
	{
		m_lruDataReg.NewSlot( iSlot + 1, -1 ) ;
		m_lruDataReg[iSlot + 1].typeRegData = regType ;
	}
	else
	{
		m_lruDataReg.FreeSlot( iSlot + 1 ) ;
	}
	SSERegister	xmmReg = (SSERegister) (iSlot >> 1) ;
	LockDataRegister( xmmReg, regType ) ;
	return	xmmReg ;
}

// レジスタの値更新フラグ設定
//////////////////////////////////////////////////////////////////////////////
void X86SSE2Assembler::SetDataRegisterModified( SSERegister xmmReg )
{
	m_lruDataReg.SetModified( (xmmReg << 1) ) ;
	m_lruDataReg.SetModified( (xmmReg << 1) + 1 ) ;
}

// 物理レジスタの変更をライトバック
//////////////////////////////////////////////////////////////////////////////
void X86SSE2Assembler::WriteBackDataRegister
		( SSERegister xmmReg, bool fKeepModified )
{
	const int	iSlot = (xmmReg << 1) ;
	if ( !m_lruDataReg.IsSlotAssigned( iSlot ) )
	{
		return ;
	}
	RegisterDataType	regType = m_lruDataReg[iSlot].typeRegData ;
	bool				fModified = m_lruDataReg[iSlot].fModified ;
	if ( !fKeepModified )
	{
		m_lruDataReg[iSlot].fModified = false ;
	}
	if ( m_lruDataReg.IsSlotAssigned( iSlot + 1 ) )
	{
		if ( m_lruDataReg[iSlot + 1].fModified )
		{
			fModified = true ;
			if ( !fKeepModified )
			{
				m_lruDataReg[iSlot + 1].fModified = false ;
			}
		}
		switch ( regType )
		{
		case	regTypeQWord:
			regType = regTypeDQWord ;
			break ;
		case	regTypeFloat64:
			regType = regTypeDFloat64 ;
			break ;
		default:
			break ;
		}
	}
	if ( !fModified )
	{
		return ;
	}
	const size_t	dispOffset =
		Context::OffsetOfReg(m_lruDataReg[iSlot].regSakura) ;
	switch ( regType )
	{
	case	regTypeQWord:
	default:
		WriteSSERegMemOperand
			( sseop_MOVQ_STORE, 3, xmmReg, x86_EBX, dispOffset ) ;
		break;
	case	regTypeFloat64:
		WriteSSERegMemOperand
			( sseop_MOVSD_STORE, 3, xmmReg, x86_EBX, dispOffset ) ;
		break;
	case	regTypeDQWord:
		WriteSSERegMemOperand
			( sseop_MOVDQA_STORE, 3, xmmReg, x86_EBX, dispOffset ) ;
		break;
	case	regTypeDFloat64:
		WriteSSERegMemOperand
			( sseop_MOVAPD_STORE, 3, xmmReg, x86_EBX, dispOffset ) ;
		break ;
	case	regTypeQFloat32:
		WriteSSERegMemOperand
			( sseop_MOVAPS_STORE, 2, xmmReg, x86_EBX, dispOffset ) ;
		break ;
	}
}

// 物理レジスタの内容をリロード
//////////////////////////////////////////////////////////////////////////////
void X86SSE2Assembler::ReloadDataRegister( SSERegister xmmReg )
{
	const int	iSlot = (xmmReg << 1) ;
	if ( !m_lruDataReg.IsSlotAssigned( iSlot ) )
	{
		return ;
	}
	RegisterDataType	regType = m_lruDataReg[iSlot].typeRegData ;
	if ( m_lruDataReg.IsSlotAssigned( iSlot + 1 ) )
	{
		switch ( regType )
		{
		case	regTypeQWord:
			regType = regTypeDQWord ;
			break ;
		case	regTypeFloat64:
			regType = regTypeDFloat64 ;
			break ;
		default:
			break ;
		}
	}
	const size_t	dispOffset =
		Context::OffsetOfReg(m_lruDataReg[iSlot].regSakura) ;
	switch ( regType )
	{
	case	regTypeQWord:
	default:
		WriteSSERegMemOperand
			( sseop_MOVQ_LOAD, 3, xmmReg, x86_EBX, dispOffset ) ;
		break;
	case	regTypeFloat64:
		WriteSSERegMemOperand
			( sseop_MOVSD_LOAD, 3, xmmReg, x86_EBX, dispOffset ) ;
		break;
	case	regTypeDQWord:
		WriteSSERegMemOperand
			( sseop_MOVDQA_LOAD, 3, xmmReg, x86_EBX, dispOffset ) ;
		break;
	case	regTypeDFloat64:
		WriteSSERegMemOperand
			( sseop_MOVAPD_LOAD, 3, xmmReg, x86_EBX, dispOffset ) ;
		break ;
	case	regTypeQFloat32:
		WriteSSERegMemOperand
			( sseop_MOVAPS_LOAD, 2, xmmReg, x86_EBX, dispOffset ) ;
		break ;
	}
}

// Sakura2 レジスタの割り当てをスワップ
//////////////////////////////////////////////////////////////////////////////
void X86SSE2Assembler::SwapDataRegisterAssignation
	( X86SSE2Assembler::SSERegister xmmReg1,
		X86SSE2Assembler::SSERegister xmmReg2 )
{
	const int	iSlot1 = (xmmReg1 << 1) ;
	const int	iSlot2 = (xmmReg2 << 1) ;
	for ( int i = 0; i < 2; i ++ )
	{
		int	regTemp1 = m_lruDataReg[iSlot1 + i].regSakura ;
		int	regTemp2 = m_lruDataReg[iSlot2 + i].regSakura ;
		m_lruDataReg[iSlot1 + i].regSakura = regTemp2 ;
		m_lruDataReg[iSlot2 + i].regSakura = regTemp1 ;
	}
}

// 物理レジスタの割り当てを一時的にロック
//////////////////////////////////////////////////////////////////////////////
void X86SSE2Assembler::LockDataRegister
	( X86SSE2Assembler::SSERegister xmmReg,
		X86SSE2Assembler::RegisterDataType regType )
{
	const int	iSlot = (xmmReg << 1) ;
	m_lruDataReg.RecentlyAccess( iSlot ) ;
	m_lruDataReg.LockSlot( iSlot ) ;
	if ( regType >= regTypeFirst128 )
	{
		ESLAssert( m_lruDataReg.IsSlotAssigned( iSlot + 1 ) ) ;
		m_lruDataReg.RecentlyAccess( iSlot + 1 ) ;
		m_lruDataReg.LockSlot( iSlot + 1 ) ;
	}
}

// 物理レジスタの割り当てを解放可能にアンロック
//////////////////////////////////////////////////////////////////////////////
void X86SSE2Assembler::UnlockDataRegister
	( X86SSE2Assembler::SSERegister xmmReg,
		X86SSE2Assembler::RegisterDataType regType )
{
	const int	iSlot = (xmmReg << 1) ;
	if ( m_lruDataReg[iSlot].regSakura == -1 )
	{
		FreeDataRegister( xmmReg, regType ) ;
	}
	else
	{
		m_lruDataReg.UnlockSlot( iSlot ) ;
		if ( regType >= regTypeFirst128 )
		{
			m_lruDataReg.UnlockSlot( iSlot + 1 ) ;
		}
		else if ( m_lruDataReg.IsSlotAssigned( iSlot + 1 ) )
		{
			m_lruDataReg.UnlockSlot( iSlot + 1 ) ;
		}
	}
}

// 物理レジスタの割り当てを解放
//////////////////////////////////////////////////////////////////////////////
void X86SSE2Assembler::FreeDataRegister
	( X86SSE2Assembler::SSERegister xmmReg,
		X86SSE2Assembler::RegisterDataType regType )
{
	const int	iSlot = (xmmReg << 1) ;
	m_lruDataReg.FreeSlot( iSlot ) ;
	m_lruDataReg.FreeSlot( iSlot + 1 ) ;
}

// 物理レジスタのペア割り当ての妥当性をチェック（デバッグ用）
//////////////////////////////////////////////////////////////////////////////
void X86SSE2Assembler::VerifyPairDataRegister( void )
{
	for ( int i = 0; i < 0x10; i += 2 )
	{
		if ( m_lruDataReg.IsSlotAssigned(i)
			&& m_lruDataReg.IsSlotAssigned(i+1) )
		{
			ESLAssert( m_lruDataReg[i].typeRegData >= regTypeFirst128 ) ;
			ESLAssert( m_lruDataReg[i+1].typeRegData >= regTypeFirst128 ) ;
		}
		else
		{
			if ( !m_lruDataReg.IsSlotAssigned(i) )
			{
				ESLAssert( m_lruDataReg[i].numLocked == 0 ) ;
			}
			if ( !m_lruDataReg.IsSlotAssigned(i+1) )
			{
				ESLAssert( m_lruDataReg[i+1].numLocked == 0 ) ;
			}
		}
	}
}

// レジスタへの変更をコンテキストに書き出し
//////////////////////////////////////////////////////////////////////////////
void X86SSE2Assembler::FlushAllRegisters( void )
{
	for ( int i = 0; i < x86_XMM_Count; i ++ )
	{
		WriteBackDataRegister( (SSERegister) i ) ;
	}
	X86GenericAssembler::FlushAllRegisters() ;
}

// レジスタへの変更をコンテキストに書き出し
//（レジスタコンテキストを変更しない）
//////////////////////////////////////////////////////////////////////////////
void X86SSE2Assembler::WriteBackAllRegisters( void )
{
	for ( int i = 0; i < x86_XMM_Count; i ++ )
	{
		WriteBackDataRegister( (SSERegister) i, true ) ;
	}
	X86GenericAssembler::WriteBackAllRegisters() ;
}

// レジスタへの変更をコンテキストに書き出し
//////////////////////////////////////////////////////////////////////////////
void X86SSE2Assembler::FlushRegister( int regSakura )
{
	int	iSlot = m_lruDataReg.FindAssignedSlot( regSakura ) ;
	if ( iSlot >= 0 )
	{
		WriteBackDataRegister( (SSERegister) (iSlot >> 1) ) ;
	}
	X86GenericAssembler::FlushRegister( regSakura ) ;
}

// レジスタの値を物理レジスタに復元する
//////////////////////////////////////////////////////////////////////////////
void X86SSE2Assembler::ReloadRegisters( void )
{
	for ( int i = 0; i < x86_XMM_Count; i ++ )
	{
		ReloadDataRegister( (SSERegister) i ) ;
	}
	X86GenericAssembler::ReloadRegisters() ;
}

// レジスタコンテキストをリセット
//////////////////////////////////////////////////////////////////////////////
void X86SSE2Assembler::ResetAllRegisters( void )
{
	m_lruDataReg.FreeAllSlot() ;
	X86GenericAssembler::ResetAllRegisters() ;
}

// レジスタコンテキストをリセット
//////////////////////////////////////////////////////////////////////////////
void X86SSE2Assembler::ResetRegister( int regSakura )
{
	int	iSlot = m_lruDataReg.FindAssignedSlot( regSakura ) ;
	if ( iSlot >= 0 )
	{
		FreeDataRegister
			( (SSERegister) (iSlot >> 1),
				m_lruDataReg[iSlot].typeRegData ) ;
	}
	X86GenericAssembler::ResetRegister( regSakura ) ;
}

// レジスタコンテキストをリセット（ポインタレジスタ以外）
//////////////////////////////////////////////////////////////////////////////
void X86SSE2Assembler::ResetDataRegisters( void )
{
	m_lruDataReg.FreeAllSlot() ;
	X86GenericAssembler::ResetDataRegisters() ;
}

// Sakura2 汎用レジスタを x86 汎用レジスタにロードするコードを出力
//////////////////////////////////////////////////////////////////////////////
void X86SSE2Assembler::WriteToLoadSakura2Register
	( X86Register regPhyLow, X86Register regPhyHigh,
			int regSakura, bool fOnlyLow )
{
	const int	iSlot = m_lruDataReg.FindAssignedSlot( regSakura ) ;
	if ( iSlot >= 0 )
	{
		SSERegister	xmmReg = (SSERegister) (iSlot >> 1) ;
		if ( iSlot & 0x01 )
		{
			WriteBackDataRegister( xmmReg ) ;
		}
		else if ( fOnlyLow )
		{
			WriteX86RegMemOperand
				( sseop_MOVD_STORE, 3,
					xmmReg, false, regPhyLow ) ;
			return ;
		}
		else
		{
			SSERegister	xmmRegTemp = AllocateDataRegister( regTypeQWord ) ;
			WriteSSERegRegOperand
				( sseop_MOVQ_LOAD, 3, xmmRegTemp, xmmReg ) ;
			WriteX86RegMemOperand
				( sseop_MOVD_STORE, 3, xmmReg, false, regPhyLow ) ;
			WriteSSERegRegImm8Operand
				( sseop_PSHIFTQ_IMM, 3, mmxop2nd_SRL, xmmRegTemp, 32 ) ;
			WriteX86RegMemOperand
				( sseop_MOVD_STORE, 3, xmmRegTemp, false, regPhyHigh ) ;
			FreeDataRegister( xmmRegTemp, regTypeQWord ) ;
			return ;
		}
	}
	X86GenericAssembler::WriteToLoadSakura2Register
		( regPhyLow, regPhyHigh, regSakura, fOnlyLow ) ;
}

void X86SSE2Assembler::WriteToLoadSakura2AddressRegister
	( X86Register regPhyLow, X86Register regPhyHigh,
		int regBasePtr, int regIndex, int scale )
{
	if ( regIndex >= 0 )
	{
		SSERegister	xmmRegTemp = AllocateDataRegister( regTypeQWord ) ;
		SSERegister	xmmRegIndex =
			WriteRealizeDataRegister( regIndex, regTypeQWord ) ;
		WriteSSERegRegOperand
			( sseop_MOVQ_LOAD, 3, xmmRegTemp, xmmRegIndex ) ;
		if ( scale > 0 )
		{
			WriteSSERegRegImm8Operand
				( sseop_PSHIFTQ_IMM, 3, mmxop2nd_SLL, xmmRegTemp, scale ) ;
		}
		SSERegister	xmmRegBase =
			WriteRealizeDataRegister( regBasePtr, regTypeQWord ) ;
		WriteSSERegRegOperand
			( sseop_PADDQ, 3, xmmRegTemp, xmmRegBase ) ;
		WriteX86RegMemOperand
			( sseop_MOVD_STORE, 3,
				xmmRegTemp, false, regPhyLow ) ;
		WriteSSERegRegImm8Operand
			( sseop_PSHIFTQ_IMM, 3, mmxop2nd_SRL, xmmRegTemp, 32 ) ;
		WriteX86RegMemOperand
			( sseop_MOVD_STORE, 3,
				xmmRegTemp, false, regPhyHigh ) ;
		//
		UnlockDataRegister( xmmRegBase, regTypeQWord ) ;
		UnlockDataRegister( xmmRegIndex, regTypeQWord ) ;
		FreeDataRegister( xmmRegTemp, regTypeQWord ) ;
	}
	else
	{
		WriteToLoadSakura2Register( regPhyLow, regPhyHigh, regBasePtr ) ;
	}
}

// x86 汎用レジスタから Sakura2 汎用レジスタへストアするコードを出力
//////////////////////////////////////////////////////////////////////////////
void X86SSE2Assembler::WriteToStoreSakura2Register
	( int regSakura, X86Register regPhyLow,
			X86Register regPhyHigh, bool fOnlyLow )
{
	const int	iSlot = m_lruDataReg.FindAssignedSlot( regSakura ) ;
	if ( iSlot >= 0 )
	{
		SSERegister	xmmRegTemp = AllocateDataRegister( regTypeQWord ) ;
		SSERegister	xmmRegSakura =
			WriteRealizeDataRegister( regSakura, regTypeQWord, fOnlyLow ) ;
		//
		if ( fOnlyLow )
		{
			WriteSSERegRegOperand
				( sseop_MOVQ_LOAD, 3, xmmRegTemp, xmmRegSakura ) ;
		}
		WriteSSERegRegOperand
			( sseop_MOVD_LOAD, 3, xmmRegSakura, (SSERegister) regPhyLow ) ;
		if ( fOnlyLow )
		{
			WriteSSERegRegImm8Operand
				( sseop_PSHIFTQ_IMM, 3, mmxop2nd_SRL, xmmRegTemp, 32 ) ;
		}
		else
		{
			WriteSSERegRegOperand
				( sseop_MOVD_LOAD, 3, xmmRegTemp, (SSERegister) regPhyHigh ) ;
		}
		WriteSSERegRegOperand
			( sseop_PUNPCKLDQ, 3, xmmRegSakura, xmmRegTemp ) ;
		//
		SetDataRegisterModified( xmmRegSakura ) ;
		UnlockDataRegister( xmmRegSakura, regTypeQWord ) ;
		FreeDataRegister( xmmRegTemp, regTypeQWord ) ;
	}
	else
	{
		X86GenericAssembler::WriteToStoreSakura2Register
			( regSakura, regPhyLow, regPhyHigh, fOnlyLow ) ;
	}
}

// メモリを複製するコードを出力
//////////////////////////////////////////////////////////////////////////////
void X86SSE2Assembler::WriteToCopyMemory
	( bool fDstAligned,
		X86Register regPhyDst, INT_PTR dispDst,
			X86Register regPhyDstIndex, int scaleDstIndex,
		bool fSrcAligned,
			X86Register regPhySrc, INT_PTR dispSrc,
			X86Register regPhySrcIndex, int scaleSrcIndex,
		int	sizeInDWord,  X86Register regPhyTemp )
{
	//
	// 128bits (16bytes) 転送
	//
//	SSERegister	xmmTemp = AllocateDataRegister( regTypeQFloat32 ) ;
	int	iSlot = m_lruDataReg.FindFreeSlot( 2 ) ;
	if ( iSlot < 0 )
	{
		iSlot = m_lruDataReg.FindMostOldSlot( 2 ) ;
		ESLAssert( iSlot >= 0 ) ;
		WriteBackDataRegister( (SSERegister) (iSlot >> 1), true ) ;
	}
	SSERegister	xmmTemp = (SSERegister) (iSlot >> 1) ;
	int	iInDWord = 0 ;
	for ( iInDWord = 0; iInDWord + 4 <= sizeInDWord; iInDWord += 4 )
	{
		if ( fSrcAligned && !(dispSrc & 0x0F)
					&& (regPhySrcIndex == x86_Nothing) )
		{
			WriteSSERegMemOperand
				( sseop_MOVAPS_LOAD, 2, xmmTemp,
					regPhySrc, dispSrc + iInDWord * sizeof(DWORD),
					regPhySrcIndex, scaleSrcIndex ) ;
		}
		else
		{
			WriteSSERegMemOperand
				( sseop_MOVUPS_LOAD, 2, xmmTemp,
					regPhySrc, dispSrc + iInDWord * sizeof(DWORD),
					regPhySrcIndex, scaleSrcIndex ) ;
		}
		if ( fDstAligned && !(dispDst & 0x0F)
					&& (regPhyDstIndex == x86_Nothing) )
		{
			WriteSSERegMemOperand
				( sseop_MOVAPS_STORE, 2, xmmTemp,
					regPhyDst, dispDst + iInDWord * sizeof(DWORD),
					regPhyDstIndex, scaleDstIndex ) ;
		}
		else
		{
			WriteSSERegMemOperand
				( sseop_MOVUPS_STORE, 2, xmmTemp,
					regPhyDst, dispDst + iInDWord * sizeof(DWORD),
					regPhyDstIndex, scaleDstIndex ) ;
		}
	}
	//
	// 64bits (8bytes) 転送
	//
	for ( ; iInDWord + 2 <= sizeInDWord; iInDWord += 2 )
	{
		WriteSSERegMemOperand
			( sseop_MOVLPS_LOAD, 2, xmmTemp,
				regPhySrc, dispSrc + iInDWord * sizeof(DWORD),
				regPhySrcIndex, scaleSrcIndex ) ;
		WriteSSERegMemOperand
			( sseop_MOVLPS_STORE, 2, xmmTemp,
				regPhyDst, dispDst + iInDWord * sizeof(DWORD),
				regPhyDstIndex, scaleDstIndex ) ;
	}
	//
	// 32bits (4bytes) 転送
	//
	for ( ; iInDWord < sizeInDWord; iInDWord ++ )
	{
		WriteSSERegMemOperand
			( sseop_MOVSS_LOAD, 3, xmmTemp,
				regPhySrc, dispSrc + iInDWord * sizeof(DWORD),
				regPhySrcIndex, scaleSrcIndex ) ;
		WriteSSERegMemOperand
			( sseop_MOVSS_STORE, 3, xmmTemp,
				regPhyDst, dispDst + iInDWord * sizeof(DWORD),
				regPhyDstIndex, scaleDstIndex ) ;
	}
//	FreeDataRegister( xmmTemp, regTypeQFloat32 ) ;
	ReloadDataRegister( xmmTemp ) ;
}

// メモリ読み込み命令出力
//////////////////////////////////////////////////////////////////////////////
void X86SSE2Assembler::WriteToLoadPhysicalMemory
	( int regDst, int regPhyPtr,
			int offset, DataType type, bool fPair )
{
	const int	sizeOfData = sizeof_prim_data[type] ;
	int			sizeOfLoadData = sizeOfData ;
	if ( fPair && (type == dataInt64) )
	{
		sizeOfLoadData *= 2 ;
	}
	X86Register	regPhyBase = x86_EBP ;
	X86Register	regPhyIndex = x86_Nothing ;
	if ( regPhyPtr != regPtrPhyBPIndex )
	{
		CommitRealizePointerRegister
			( m_lruPointer[regPhyPtr],
				offset, offset + sizeOfLoadData ) ;
		regPhyBase = (X86Register) m_lruPointer[regPhyPtr].regPhy ;
		if ( m_lruPointer[regPhyPtr].fTLBFetched )
		{
			regPhyIndex = (X86Register) m_lruPointer[regPhyPtr].regPhyIndex ;
			m_lruPointer[regPhyPtr].regPhyIndex = x86_Nothing ;
		}
	}
	if ( fPair && (type == dataInt64) )
	{
		//
		// 128bit ロード
		//
		RegisterDataType	regType = m_rdtSakura[regDst] ;
		SSERegister	xmmReg ;
		switch ( regType )
		{
		case	regTypeQWord:
		case	regTypeDQWord:
			xmmReg = WriteRealizeDataRegister
						( regDst, regTypeDQWord, false ) ;
			WriteSSERegMemOperand
				( sseop_MOVDQU_LOAD, 3,
					xmmReg, regPhyBase, offset, regPhyIndex ) ;
			regType = regTypeDQWord ;
			break ;
		case	regTypeFloat64:
		case	regTypeDFloat64:
			xmmReg = WriteRealizeDataRegister
						( regDst, regTypeDFloat64, false ) ;
			WriteSSERegMemOperand
				( sseop_MOVUPD_LOAD, 3,
					xmmReg, regPhyBase, offset, regPhyIndex ) ;
			regType = regTypeDFloat64 ;
			break ;
		default:
		case	regTypeQFloat32:
			xmmReg = WriteRealizeDataRegister
						( regDst, regTypeQFloat32, false ) ;
			WriteSSERegMemOperand
				( sseop_MOVUPS_LOAD, 2,
					xmmReg, regPhyBase, offset, regPhyIndex ) ;
			regType = regTypeQFloat32 ;
			break ;
		}
		SetDataRegisterModified( xmmReg ) ;
		UnlockDataRegister( xmmReg, regType ) ;
	}
	else if ( fPair && (type == dataUint32) )
	{
		RegisterDataType	regType = m_rdtSakura[regDst] ;
		SSERegister	xmmReg ;
		switch ( regType )
		{
		case	regTypeQWord:
		case	regTypeDQWord:
			xmmReg = WriteRealizeDataRegister
						( regDst, regTypeDQWord, false ) ;
			WriteSSERegMemOperand
				( sseop_MOVD_LOAD, 3,
					xmmReg, regPhyBase, offset, regPhyIndex ) ;
			regType = regTypeDQWord ;
			break ;
		case	regTypeFloat64:
		case	regTypeDFloat64:
		case	regTypeQFloat32:
		default:
			xmmReg = WriteRealizeDataRegister
						( regDst, regTypeQFloat32, false ) ;
			WriteSSERegMemOperand
				( sseop_MOVSS_LOAD, 3,
					xmmReg, regPhyBase, offset, regPhyIndex ) ;
			regType = regTypeQFloat32 ;
			break ;
		}
		SetDataRegisterModified( xmmReg ) ;
		UnlockDataRegister( xmmReg, regType ) ;
	}
	else
	{
		//
		// 64bit 以下サイズロード
		//
		RegisterDataType	regType = m_rdtSakura[regDst] ;
		SSERegister	xmmReg ;
		bool		fConvertReg = false ;
		switch ( type )
		{
		case	dataInt64:
		default:
			ESLAssert( !fPair ) ;
			switch ( regType )
			{
			case	regTypeQWord:
			case	regTypeDQWord:
				xmmReg = WriteRealizeDataRegister
							( regDst, regTypeQWord, false ) ;
				WriteSSERegMemOperand
					( sseop_MOVQ_LOAD, 3,
						xmmReg, regPhyBase, offset, regPhyIndex ) ;
				regType = regTypeQWord ;
				break ;
			case	regTypeFloat64:
			case	regTypeDFloat64:
				xmmReg = WriteRealizeDataRegister
							( regDst, regTypeFloat64, false ) ;
				WriteSSERegMemOperand
					( sseop_MOVSD_LOAD, 3,
						xmmReg, regPhyBase, offset, regPhyIndex ) ;
				regType = regTypeFloat64 ;
				break ;
			default:
			case	regTypeQFloat32:
				xmmReg = WriteRealizeDataRegister
					( (regDst & ~0x01), regTypeQFloat32, true ) ;
				if ( regDst & 0x01 )
				{
					WriteSSERegMemOperand
						( sseop_MOVHPS_LOAD, 2,
							xmmReg, regPhyBase, offset, regPhyIndex ) ;
				}
				else
				{
					WriteSSERegMemOperand
						( sseop_MOVLPS_LOAD, 2,
							xmmReg, regPhyBase, offset, regPhyIndex ) ;
				}
				regType = regTypeQFloat32 ;
				break ;
			}
			SetDataRegisterModified( xmmReg ) ;
			UnlockDataRegister( xmmReg, regType ) ;
			break ;
		case	dataInt32:
			WriteX86LoadRegMem
				( x86_EAX, regPhyBase, offset, regPhyIndex ) ;
			WriteX86ImmediateOperand( x86op1_CDQ, 1 ) ;
			fConvertReg = true ;
			break ;
		case	dataInt16:
			WriteX86RegMemOperand
				( x86op2_MOVSX_16, 2,
					x86_EAX, true, regPhyBase, offset, regPhyIndex ) ;
			WriteX86ImmediateOperand( x86op1_CDQ, 1 ) ;
			fConvertReg = true ;
			break ;
		case	dataInt8:
			WriteX86RegMemOperand
				( x86op2_MOVSX_8, 2,
					x86_EAX, true, regPhyBase, offset, regPhyIndex ) ;
			WriteX86ImmediateOperand( x86op1_CDQ, 1 ) ;
			fConvertReg = true ;
			break ;
		case	dataFloat:
			xmmReg = WriteRealizeDataRegister
						( regDst, regTypeFloat64, false ) ;
			WriteSSERegMemOperand
				( sseop_CVT_SS2SD, 3,
					xmmReg, regPhyBase, offset, regPhyIndex ) ;
			SetDataRegisterModified( xmmReg ) ;
			UnlockDataRegister( xmmReg, regTypeFloat64 ) ;
			break ;
		case	dataUint32:
			xmmReg = WriteRealizeDataRegister
						( regDst, regTypeQWord, false ) ;
			WriteSSERegMemOperand
				( sseop_MOVD_LOAD, 3,
					xmmReg, regPhyBase, offset, regPhyIndex ) ;
			SetDataRegisterModified( xmmReg ) ;
			UnlockDataRegister( xmmReg, regTypeQWord ) ;
			break ;
		case	dataUint16:
			xmmReg = WriteRealizeDataRegister
						( regDst, regTypeQWord, false ) ;
			WriteX86RegMemOperand
				( x86op2_MOVZX_16, 2,
					x86_EAX, true, regPhyBase, offset, regPhyIndex ) ;
			WriteSSERegRegOperand
				( sseop_MOVD_LOAD, 3, xmmReg, (SSERegister) x86_EAX ) ;
			SetDataRegisterModified( xmmReg ) ;
			UnlockDataRegister( xmmReg, regTypeQWord ) ;
			break ;
		case	dataUint8:
			xmmReg = WriteRealizeDataRegister
						( regDst, regTypeQWord, false ) ;
			WriteX86RegMemOperand
				( x86op2_MOVZX_8, 2,
					x86_EAX, true, regPhyBase, offset, regPhyIndex ) ;
			WriteSSERegRegOperand
				( sseop_MOVD_LOAD, 3, xmmReg, (SSERegister) x86_EAX ) ;
			SetDataRegisterModified( xmmReg ) ;
			UnlockDataRegister( xmmReg, regTypeQWord ) ;
			break ;
		}
		if ( fConvertReg )
		{
			SSERegister	xmmTemp = AllocateDataRegister( regTypeQWord ) ;
			xmmReg = WriteRealizeDataRegister
						( regDst, regTypeQWord, false ) ;
			WriteSSERegRegOperand
				( sseop_MOVD_LOAD, 3, xmmReg, (SSERegister) x86_EAX ) ;
			WriteSSERegRegOperand
				( sseop_MOVD_LOAD, 3, xmmTemp, (SSERegister) x86_EDX ) ;
			WriteSSERegRegOperand
				( sseop_PUNPCKLDQ, 3, xmmReg, xmmTemp ) ;
			SetDataRegisterModified( xmmReg ) ;
			UnlockDataRegister( xmmReg, regTypeQWord ) ;
			FreeDataRegister( xmmTemp, regTypeQWord ) ;
		}
	}
}

// メモリ書き出し命令出力
//////////////////////////////////////////////////////////////////////////////
void X86SSE2Assembler::WriteToStorePhysicalMemory
	( int regSrc, int regPhyPtr,
			int offset, DataType type, bool fPair )
{
	const int	sizeOfData = sizeof_prim_data[type] ;
	const int	countPair = fPair ? 2 : 1 ;
	X86Register	regPhyBase = x86_EBP ;
	X86Register	regPhyIndex = x86_Nothing ;
	if ( regPhyPtr != regPtrPhyBPIndex )
	{
		CommitRealizePointerRegister
			( m_lruPointer[regPhyPtr], offset,
					offset + sizeOfData * countPair ) ;
		regPhyBase = (X86Register) m_lruPointer[regPhyPtr].regPhy ;
		if ( m_lruPointer[regPhyPtr].fTLBFetched )
		{
			regPhyIndex = (X86Register) m_lruPointer[regPhyPtr].regPhyIndex ;
			m_lruPointer[regPhyPtr].regPhyIndex = x86_Nothing ;
		}
	}
	if ( fPair && (type == dataInt64) )
	{
		//
		// 128bit ストア
		//
		RegisterDataType	regType = m_rdtSakura[regSrc] ;
		SSERegister	xmmReg ;
		switch ( regType )
		{
		case	regTypeQWord:
		case	regTypeDQWord:
			xmmReg = WriteRealizeDataRegister
						( regSrc, regTypeDQWord, true ) ;
			WriteSSERegMemOperand
				( sseop_MOVDQU_STORE, 3,
					xmmReg, regPhyBase, offset, regPhyIndex ) ;
			regType = regTypeDQWord ;
			break ;
		case	regTypeFloat64:
		case	regTypeDFloat64:
			xmmReg = WriteRealizeDataRegister
						( regSrc, regTypeDFloat64, true ) ;
			WriteSSERegMemOperand
				( sseop_MOVUPD_STORE, 3,
					xmmReg, regPhyBase, offset, regPhyIndex ) ;
			regType = regTypeDFloat64 ;
			break ;
		default:
		case	regTypeQFloat32:
			xmmReg = WriteRealizeDataRegister
						( regSrc, regTypeQFloat32, true ) ;
			WriteSSERegMemOperand
				( sseop_MOVUPS_STORE, 2,
					xmmReg, regPhyBase, offset, regPhyIndex ) ;
			regType = regTypeQFloat32 ;
			break ;
		}
		UnlockDataRegister( xmmReg, regType ) ;
	}
	else for ( int i = 0; i < countPair; i ++ )
	{
		//
		// 64bit 以下サイズストア
		//
		RegisterDataType	regType = m_rdtSakura[regSrc + i] ;
		SSERegister	xmmReg, xmmTemp ;
		switch ( type )
		{
		case	dataInt64:
		default:
			ESLAssert( !fPair ) ;
			switch ( regType )
			{
			case	regTypeQWord:
			case	regTypeDQWord:
				xmmReg = WriteRealizeDataRegister
							( regSrc + i, regTypeQWord, true ) ;
				WriteSSERegMemOperand
					( sseop_MOVQ_STORE, 3,
						xmmReg, regPhyBase,
						offset + i * sizeOfData, regPhyIndex ) ;
				regType = regTypeQWord ;
				break ;
			case	regTypeFloat64:
			case	regTypeDFloat64:
				xmmReg = WriteRealizeDataRegister
							( regSrc + i, regTypeFloat64, true ) ;
				WriteSSERegMemOperand
					( sseop_MOVSD_STORE, 3,
						xmmReg, regPhyBase,
						offset + i * sizeOfData, regPhyIndex ) ;
				regType = regTypeFloat64 ;
				break ;
			default:
			case	regTypeQFloat32:
				xmmReg = WriteRealizeDataRegister
					( ((regSrc + i) & ~0x01), regTypeQFloat32, true ) ;
				if ( (regSrc + i) & 0x01 )
				{
					WriteSSERegMemOperand
						( sseop_MOVHPS_STORE, 2,
							xmmReg, regPhyBase,
							offset + i * sizeOfData, regPhyIndex ) ;
				}
				else
				{
					WriteSSERegMemOperand
						( sseop_MOVLPS_STORE, 2,
							xmmReg, regPhyBase,
							offset + i * sizeOfData, regPhyIndex ) ;
				}
				regType = regTypeQFloat32 ;
				break ;
			}
			UnlockDataRegister( xmmReg, regType ) ;
			break ;
		case	dataInt32:
		case	dataUint32:
			if ( (regType == regTypeQFloat32) && !((regSrc + i) & 0x01) )
			{
				xmmReg = WriteRealizeDataRegister
							( regSrc + i, regTypeQFloat32, true ) ;
				WriteSSERegMemOperand
					( sseop_MOVSS_STORE, 3,
						xmmReg, regPhyBase,
						offset + i * sizeOfData, regPhyIndex ) ;
				UnlockDataRegister( xmmReg, regTypeQFloat32 ) ;
			}
			else
			{
				xmmReg = WriteRealizeDataRegister
							( regSrc + i, regTypeQWord, true ) ;
				WriteSSERegMemOperand
					( sseop_MOVD_STORE, 3,
						xmmReg, regPhyBase,
						offset + i * sizeOfData, regPhyIndex ) ;
				UnlockDataRegister( xmmReg, regTypeQWord ) ;
			}
			break ;
		case	dataInt16:
		case	dataUint16:
			WriteToLoadSakura2Register
				( x86_EAX, x86_EDX, regSrc + i, true ) ;
			WriteX86RegMemOperand
				( x86op1_MOV_STORE | 0x6600, 2,
					x86_EAX, true, regPhyBase,
					offset + i * sizeOfData, regPhyIndex ) ;
			break ;
		case	dataInt8:
		case	dataUint8:
			WriteToLoadSakura2Register
				( x86_EAX, x86_EDX, regSrc + i, true ) ;
			WriteX86RegMemOperand
				( x86op1_MOV_STORE_8, 1,
					x86_AL, true, regPhyBase,
					offset + i * sizeOfData, regPhyIndex ) ;
			break ;
		case	dataFloat:
			xmmReg = GetRealizedDataRegister
						( regSrc + i, regTypeFloat64, true ) ;
			xmmTemp = AllocateDataRegister( regTypeFloat64 ) ;
			if ( xmmReg != XMM_Nothing )
			{
				WriteSSERegRegOperand
					( sseop_CVT_SD2SS, 3, xmmTemp, xmmReg ) ;
				UnlockDataRegister( xmmReg, regTypeFloat64 ) ;
			}
			else
			{
				WriteSSERegMemOperand
					( sseop_CVT_SD2SS, 3, xmmTemp,
						x86_EBX, Context::OffsetOfReg(regSrc + i) ) ;
			}
			WriteSSERegMemOperand
				( sseop_MOVSS_STORE, 3,
					xmmTemp, regPhyBase,
					offset + i * sizeOfData, regPhyIndex ) ;
			FreeDataRegister( xmmTemp, regTypeFloat64 ) ;
			break ;
		}
	}
}

// データ移動命令
//////////////////////////////////////////////////////////////////////////////
void X86SSE2Assembler::write_move_reg_reg( int regDst, int regSrc, bool fPair )
{
	if ( regDst != regSrc )
	{
		int		iSlotDst = m_lruDataReg.FindAssignedSlot( regDst ) ;
		int		iSlotSrc = m_lruDataReg.FindAssignedSlot( regSrc ) ;
		bool	fProcessed = false ;
		if ( !fPair && (iSlotDst >= 0)
			&& (m_lruDataReg[iSlotDst].typeRegData >= regTypeDFloat64) )
		{
			//
			// ロード済み128ビットレジスタへの部分移動処理
			//
			const bool	fDstHigh = ((iSlotDst & 0x01) != 0) ;
			bool		fSrcHigh = ((iSlotSrc & 0x01) != 0) ;
			SSERegister	xmmDstReg = (SSERegister) (iSlotDst >> 1) ;
			SSERegister	xmmSrcReg = (SSERegister) (iSlotSrc >> 1) ;
			RegisterDataType
					regDstType = m_lruDataReg[iSlotDst].typeRegData ;
			LockDataRegister( xmmDstReg, regDstType ) ;
			if ( iSlotSrc >= 0 )
			{
				LockDataRegister
					( xmmSrcReg, m_lruDataReg[iSlotSrc].typeRegData ) ;
			}
			RegisterDataType	regSrcType = m_rdtSakura[regSrc] ;
			regDstType = NormalizeDataTypeTo128( regDstType ) ;
			regSrcType = NormalizeDataTypeTo128( regSrcType ) ;
			if ( regDstType < regSrcType )
			{
				regDstType = regSrcType ;
			}
			switch ( regDstType )
			{
			case	regTypeFloat64:
			case	regTypeDFloat64:
				if ( (iSlotSrc >= 0) && fDstHigh && !fSrcHigh )
				{
					WriteSSERegRegOperand
						( sseop_UNPCKLPD, 3, xmmDstReg, xmmSrcReg ) ;
				}
				else if ( (iSlotSrc >= 0) && fDstHigh && fSrcHigh )
				{
					WriteSSERegRegOperand
						( sseop_SHUFPD, 3, xmmDstReg, xmmSrcReg, 0x02, 1 ) ;
				}
				else
				{
					if ( iSlotSrc >= 0 )
					{
						WriteBackDataRegister( xmmSrcReg ) ;
					}
					if ( !fDstHigh )
					{
						WriteSSERegMemOperand
							( sseop_MOVLPD_LOAD, 3, xmmDstReg,
								x86_EBX, Context::OffsetOfReg(regSrc) ) ;
					}
					else
					{
						WriteSSERegMemOperand
							( sseop_MOVHPD_LOAD, 3, xmmDstReg,
								x86_EBX, Context::OffsetOfReg(regSrc) ) ;
					}
				}
				regDstType = regTypeDFloat64 ;
				break ;
			default:
			case	regTypeQWord:
			case	regTypeDQWord:
			case	regTypeQFloat32:
				if ( (iSlotSrc >= 0) && fDstHigh && !fSrcHigh )
				{
					WriteSSERegRegOperand
						( sseop_MOVLHPS, 2, xmmDstReg, xmmSrcReg ) ;
				}
				else if ( (iSlotSrc >= 0) && !fDstHigh && fSrcHigh )
				{
					WriteSSERegRegOperand
						( sseop_MOVHLPS, 2, xmmDstReg, xmmSrcReg ) ;
				}
				else if ( (iSlotSrc >= 0) && fDstHigh && fSrcHigh )
				{
					WriteSSERegRegOperand
						( sseop_SHUFPS, 2, xmmDstReg, xmmSrcReg, 0xE4, 1 ) ;
				}
				else
				{
					if ( iSlotSrc >= 0 )
					{
						WriteBackDataRegister( xmmSrcReg ) ;
					}
					if ( !fDstHigh )
					{
						WriteSSERegMemOperand
							( sseop_MOVLPS_LOAD, 2, xmmDstReg,
								x86_EBX, Context::OffsetOfReg(regSrc) ) ;
					}
					else
					{
						WriteSSERegMemOperand
							( sseop_MOVHPS_LOAD, 2, xmmDstReg,
								x86_EBX, Context::OffsetOfReg(regSrc) ) ;
					}
				}
				regDstType = regTypeQFloat32 ;
				break ;
			}
			m_lruDataReg[iSlotDst & ~0x01].typeRegData = regDstType ;
			m_lruDataReg[iSlotDst | 0x01].typeRegData = regDstType ;
			UnlockDataRegister( xmmDstReg, regDstType ) ;
			if ( iSlotSrc >= 0 )
			{
				UnlockDataRegister
					( xmmSrcReg, m_lruDataReg[iSlotSrc].typeRegData ) ;
			}
			fProcessed = true ;
		}
		if ( !fProcessed )
		{
			//
			// 汎用処理
			//
			RegisterDataType	regType = m_rdtSakura[regSrc] ;
			if ( fPair )
			{
				regType = NormalizeDataTypeTo128( regType ) ;
			}
			else
			{
				regType = NormalizeDataTypeTo64( regType ) ;
			}
			SSERegister	xmmSrc =
				WriteRealizeDataRegister( regSrc, regType, true ) ;
			SSERegister	xmmDst =
				WriteRealizeDataRegister( regDst, regType, false ) ;
			switch ( regType )
			{
			case	regTypeQWord:
				WriteSSERegRegOperand
					( sseop_MOVQ_LOAD, 3, xmmDst, xmmSrc ) ;
				break ;
			case	regTypeFloat64:
				WriteSSERegRegOperand
					( sseop_MOVSD_LOAD, 3, xmmDst, xmmSrc ) ;
				break ;
			case	regTypeDQWord:
				WriteSSERegRegOperand
					( sseop_MOVDQA_LOAD, 3, xmmDst, xmmSrc ) ;
				break ;
			case	regTypeDFloat64:
				WriteSSERegRegOperand
					( sseop_MOVAPD_LOAD, 3, xmmDst, xmmSrc ) ;
				break ;
			default:
			case	regTypeQFloat32:
				WriteSSERegRegOperand
					( sseop_MOVAPS_LOAD, 2, xmmDst, xmmSrc ) ;
				break ;
			}
			SetDataRegisterModified( xmmDst ) ;
			UnlockDataRegister( xmmSrc, regType ) ;
			UnlockDataRegister( xmmDst, regType ) ;
		}
	}
}

void X86SSE2Assembler::write_maskmove_reg_reg_reg( int regDst, int regSrc, int regSrc2, bool fPair )
{
	RegisterDataType	regType = m_rdtSakura[regSrc] ;
	if ( fPair )
	{
		regType = NormalizeDataTypeTo128( regType ) ;
	}
	else
	{
		regType = NormalizeDataTypeTo64( regType ) ;
	}
	SSERegister	xmmTemp = AllocateDataRegister( regType ) ;
	SSERegister	xmmMask =
		WriteRealizeDataRegister( regSrc2, regType, true ) ;
	SSERegister	xmmDst =
		WriteRealizeDataRegister( regDst, regType, true ) ;
	SSERegister	xmmSrc =
		WriteRealizeDataRegister( regSrc, regType, true ) ;
	WriteBackDataRegister( xmmMask ) ;
	switch ( regType )
	{
	case	regTypeQWord:
	case	regTypeDQWord:
		WriteSSERegRegOperand
			( sseop_MOVDQA_LOAD, 3, xmmTemp, xmmMask ) ;
		WriteSSERegRegOperand
			( sseop_PAND, 3, xmmMask, xmmSrc ) ;
		WriteSSERegRegOperand
			( sseop_PANDN, 3, xmmTemp, xmmDst ) ;
		WriteSSERegRegOperand
			( sseop_MOVDQA_LOAD, 3, xmmDst, xmmMask ) ;
		WriteSSERegRegOperand
			( sseop_POR, 3, xmmDst, xmmTemp ) ;
		break ;
	case	regTypeFloat64:
	case	regTypeDFloat64:
		WriteSSERegRegOperand
			( sseop_MOVAPD_LOAD, 3, xmmTemp, xmmMask ) ;
		WriteSSERegRegOperand
			( sseop_ANDPD, 3, xmmMask, xmmSrc ) ;
		WriteSSERegRegOperand
			( sseop_ANDNPD, 3, xmmTemp, xmmDst ) ;
		WriteSSERegRegOperand
			( sseop_MOVAPD_LOAD, 3, xmmDst, xmmMask ) ;
		WriteSSERegRegOperand
			( sseop_ORPD, 3, xmmDst, xmmTemp ) ;
		break ;
	default:
	case	regTypeQFloat32:
		WriteSSERegRegOperand
			( sseop_MOVAPS_LOAD, 2, xmmTemp, xmmMask ) ;
		WriteSSERegRegOperand
			( sseop_ANDPS, 2, xmmMask, xmmSrc ) ;
		WriteSSERegRegOperand
			( sseop_ANDNPS, 2, xmmTemp, xmmDst ) ;
		WriteSSERegRegOperand
			( sseop_MOVAPS_LOAD, 2, xmmDst, xmmMask ) ;
		WriteSSERegRegOperand
			( sseop_ORPS, 2, xmmDst, xmmTemp ) ;
		break ;
	}
	SetDataRegisterModified( xmmDst ) ;
	FreeDataRegister( xmmTemp, regType ) ;
	UnlockDataRegister( xmmDst, regType ) ;
	UnlockDataRegister( xmmSrc, regType ) ;
	FreeDataRegister( xmmMask, regType ) ;
}

// 整数・実数変換命令
//////////////////////////////////////////////////////////////////////////////
void X86SSE2Assembler::write_cvt_float2int( int regDst, int regSrc )
{
	FlushRegister( regSrc ) ;
	ResetRegister( regDst ) ;
	//
	WriteX86RegMemOperand
		( x86op1_FLD_FP64, 1, x86op2nd_FLD,
			true, x86_EBX, Context::OffsetOfReg(regSrc) ) ;
	WriteX86RegMemOperand
		( x86op1_FIST_FP64, 1, x86op2nd_FISTP64,
			true, x86_EBX, Context::OffsetOfReg(regDst) ) ;
}

void X86SSE2Assembler::write_cvt_int2float( int regDst, int regSrc )
{
	FlushRegister( regSrc ) ;
	ResetRegister( regDst ) ;
	//
	WriteX86RegMemOperand
		( x86op1_FILD_FP64, 1, x86op2nd_FILD64,
			true, x86_EBX, Context::OffsetOfReg(regSrc) ) ;
	WriteX86RegMemOperand
		( x86op1_FST_FP64, 1, x86op2nd_FSTP,
			true, x86_EBX, Context::OffsetOfReg(regDst) ) ;
}

// シフト命令
//////////////////////////////////////////////////////////////////////////////
void X86SSE2Assembler::write_srl_reg_reg_imm8( int regDst, int regSrc, int imm8, bool fPair )
{
	RegisterDataType	regType = fPair ? regTypeDQWord : regTypeQWord ;
	SSERegister	xmmDst =
		WriteRealizeDataRegister( regDst, regType, (regDst == regSrc) ) ;
	SSERegister	xmmSrc =
		WriteRealizeDataRegister( regSrc, regType, true ) ;
	//
	if ( regDst == regSrc )
	{
		ESLAssert( xmmDst == xmmSrc ) ;
	}
	else
	{
		WriteSSERegRegOperand
			( sseop_MOVDQA_LOAD, 3, xmmDst, xmmSrc ) ;
	}
	if ( imm8 > 0 )
	{
		WriteSSERegRegImm8Operand
			( sseop_PSHIFTQ_IMM, 3, mmxop2nd_SRL, xmmDst, imm8 ) ;
	}
	//
	SetDataRegisterModified( xmmDst ) ;
	UnlockDataRegister( xmmDst, regType ) ;
	UnlockDataRegister( xmmSrc, regType ) ;
}

void X86SSE2Assembler::write_sra_reg_reg_imm8( int regDst, int regSrc, int imm8, bool fPair )
{
	RegisterDataType	regType = fPair ? regTypeDQWord : regTypeQWord ;
	SSERegister	xmmDst =
		WriteRealizeDataRegister( regDst, regType, (regDst == regSrc) ) ;
	SSERegister	xmmSrc =
		WriteRealizeDataRegister( regSrc, regType, true ) ;
	SSERegister	xmmTemp = AllocateDataRegister( regType ) ;
	//
	void * pDataSignMask = GetConstantPair8000000000000000() ;
	//
	WriteSSERegMemOperand
		( sseop_MOVDQA_LOAD, 3,
			xmmTemp, x86_Nothing, (INT_PTR) pDataSignMask ) ;
	if ( regDst == regSrc )
	{
		ESLAssert( xmmDst == xmmSrc ) ;
	}
	else
	{
		WriteSSERegRegOperand
			( sseop_MOVDQA_LOAD, 3, xmmDst, xmmSrc ) ;
	}
	if ( imm8 > 0 )
	{
		SSERegister	xmmZero = AllocateDataRegister( regType ) ;
		//
		WriteSSERegRegOperand
			( sseop_PAND, 3, xmmTemp, xmmSrc ) ;
		WriteSSERegRegOperand( sseop_PXOR, 3, xmmZero, xmmZero ) ;
		WriteSSERegRegImm8Operand
			( sseop_PSHIFTQ_IMM, 3, mmxop2nd_SRL, xmmTemp, imm8 ) ;
		WriteSSERegRegImm8Operand
			( sseop_PSHIFTQ_IMM, 3, mmxop2nd_SRL, xmmDst, imm8 ) ;
		WriteSSERegRegOperand( sseop_PSUBQ, 3, xmmZero, xmmTemp ) ;
		WriteSSERegRegOperand( sseop_POR, 3, xmmDst, xmmZero ) ;
		//
		FreeDataRegister( xmmZero, regType ) ;
	}
	//
	SetDataRegisterModified( xmmDst ) ;
	UnlockDataRegister( xmmDst, regType ) ;
	UnlockDataRegister( xmmSrc, regType ) ;
	FreeDataRegister( xmmTemp, regType ) ;
}

void X86SSE2Assembler::write_sll_reg_reg_imm8( int regDst, int regSrc, int imm8, bool fPair )
{
	RegisterDataType	regType = fPair ? regTypeDQWord : regTypeQWord ;
	SSERegister	xmmDst =
		WriteRealizeDataRegister( regDst, regType, (regDst == regSrc) ) ;
	SSERegister	xmmSrc =
		WriteRealizeDataRegister( regSrc, regType, true ) ;
	//
	if ( regDst == regSrc )
	{
		ESLAssert( xmmDst == xmmSrc ) ;
	}
	else
	{
		WriteSSERegRegOperand
			( sseop_MOVDQA_LOAD, 3, xmmDst, xmmSrc ) ;
	}
	if ( imm8 > 0 )
	{
		WriteSSERegRegImm8Operand
			( sseop_PSHIFTQ_IMM, 3, mmxop2nd_SLL, xmmDst, imm8 ) ;
	}
	//
	SetDataRegisterModified( xmmDst ) ;
	UnlockDataRegister( xmmDst, regType ) ;
	UnlockDataRegister( xmmSrc, regType ) ;
}

// 32ビット即値命令
//////////////////////////////////////////////////////////////////////////////
void X86SSE2Assembler::write_add_reg_reg_imm32( int regDst, int regSrc, int imm32, bool fPair )
{
	RegisterDataType	regType = fPair ? regTypeDQWord : regTypeQWord ;
	if ( !fPair && (regSrc == regIntZero) )
	{
		INT64 * pDataImm = (INT64*) m_buf->AllocateData( 8, 8 ) ;
		*pDataImm = imm32 ;
		//
		SSERegister	xmmDst =
			WriteRealizeDataRegister( regDst, regType, false ) ;
		//
		WriteSSERegMemOperand
			( sseop_MOVQ_LOAD, 3,
				xmmDst, x86_Nothing, (INT_PTR) pDataImm ) ;
		//
		SetDataRegisterModified( xmmDst ) ;
		UnlockDataRegister( xmmDst, regType ) ;
	}
	else
	{
		SSERegister	xmmDst =
			WriteRealizeDataRegister( regDst, regType, (regDst == regSrc) ) ;
		SSERegister	xmmSrc =
			WriteRealizeDataRegister( regSrc, regType, true ) ;
		if ( regDst == regSrc )
		{
			ESLAssert( xmmDst == xmmSrc ) ;
		}
		else
		{
			WriteSSERegRegOperand
				( sseop_MOVDQA_LOAD, 3, xmmDst, xmmSrc ) ;
		}
		if ( imm32 != 0 )
		{
			INT64 * pDataImm = (INT64*) m_buf->AllocateData( 0x10, 0x10 ) ;
			pDataImm[0] = imm32 ;
			pDataImm[1] = fPair ? imm32 : 0 ;
			//
			WriteSSERegMemOperand
				( sseop_PADDQ, 3, xmmDst, x86_Nothing, (INT_PTR) pDataImm ) ;
		}
		UnlockDataRegister( xmmSrc, regType ) ;
		SetDataRegisterModified( xmmDst ) ;
		UnlockDataRegister( xmmDst, regType ) ;
	}
}

void X86SSE2Assembler::write_mul_reg_reg_imm32( int regDst, int regSrc, int imm32, bool fPair )
{
	if ( (imm32 == 0) || (!fPair && (regSrc == regIntZero)) )
	{
		RegisterDataType	regType = fPair ? regTypeDQWord : regTypeQWord ;
		SSERegister	xmmDst =
			WriteRealizeDataRegister( regDst, regType, false ) ;
		//
		WriteSSERegRegOperand( sseop_PXOR, 3, xmmDst, xmmDst ) ;
		//
		SetDataRegisterModified( xmmDst ) ;
		UnlockDataRegister( xmmDst, regType ) ;
		return ;
	}
	else if ( imm32 == 1 )
	{
		RegisterDataType	regType = fPair ? regTypeDQWord : regTypeQWord ;
		SSERegister	xmmDst =
			WriteRealizeDataRegister( regDst, regType, false ) ;
		SSERegister	xmmSrc =
			WriteRealizeDataRegister( regSrc, regType, true ) ;
		//
		WriteSSERegRegOperand( sseop_MOVDQA_LOAD, 3, xmmDst, xmmSrc ) ;
		//
		SetDataRegisterModified( xmmDst ) ;
		UnlockDataRegister( xmmDst, regType ) ;
		UnlockDataRegister( xmmSrc, regType ) ;
		return ;
	}
	else if ( imm32 == -1 )
	{
		RegisterDataType	regType = fPair ? regTypeDQWord : regTypeQWord ;
		SSERegister	xmmDst =
			WriteRealizeDataRegister( regDst, regType, false ) ;
		SSERegister	xmmSrc =
			WriteRealizeDataRegister( regSrc, regType, true ) ;
		SSERegister	xmmTemp = AllocateDataRegister( regType ) ;
		//
		WriteSSERegRegOperand( sseop_PCMPEQD, 3, xmmTemp, xmmTemp ) ;
		WriteSSERegRegOperand( sseop_MOVDQA_LOAD, 3, xmmDst, xmmSrc ) ;
		WriteSSERegRegOperand( sseop_PXOR, 3, xmmDst, xmmTemp ) ;
		WriteSSERegRegOperand( sseop_PSUBQ, 3, xmmDst, xmmTemp ) ;
		//
		SetDataRegisterModified( xmmDst ) ;
		UnlockDataRegister( xmmDst, regType ) ;
		UnlockDataRegister( xmmSrc, regType ) ;
		FreeDataRegister( xmmTemp, regType ) ;
		return ;
	}
	INT64 * pDataImm = (INT64*) m_buf->AllocateData( 8, 8 ) ;
	*pDataImm = imm32 ;
	//
	const int	countPair = fPair ? 2 : 1 ;
	for ( int i = 0; i < countPair; i ++ )
	{
		SSERegister	xmmDst =
			WriteRealizeDataRegister( regDst + i, regTypeQWord, (regDst == regSrc) ) ;
		SSERegister	xmmSrc =
			WriteRealizeDataRegister( regSrc + i, regTypeQWord, true ) ;
		SSERegister	xmmTemp1 = AllocateDataRegister( regTypeQWord ) ;
		SSERegister	xmmTemp2 = AllocateDataRegister( regTypeQWord ) ;
		//
		if ( regDst != regSrc )
		{
			WriteSSERegRegOperand( sseop_MOVQ_LOAD, 3, xmmDst, xmmSrc ) ;
		}
		UnlockDataRegister( xmmSrc, regTypeQWord ) ;
		//
		SSERegister	xmmImm = AllocateDataRegister( regTypeQWord ) ;
		WriteSSERegMemOperand
			( sseop_MOVQ_LOAD, 3, xmmImm, x86_Nothing, (INT_PTR) pDataImm ) ;
		//
		write_mul_int64xint64( xmmDst, xmmImm, xmmTemp1, xmmTemp2 ) ;
		//
		SetDataRegisterModified( xmmDst ) ;
		UnlockDataRegister( xmmDst, regTypeQWord ) ;
		FreeDataRegister( xmmImm, regTypeQWord ) ;
		FreeDataRegister( xmmTemp1, regTypeQWord ) ;
		FreeDataRegister( xmmTemp2, regTypeQWord ) ;
	}
}

// 64ビット即値命令
//////////////////////////////////////////////////////////////////////////////
void X86SSE2Assembler::write_move_reg_imm64( int regDst, INT64 imm64 )
{
	SSERegister	xmmDst =
		WriteRealizeDataRegister( regDst, regTypeQWord, false ) ;
	INT64 * pDataImm = (INT64*) m_buf->AllocateData( 8, 8 ) ;
	*pDataImm = imm64 ;
	//
	switch ( m_rdtSakura[regDst] )
	{
	case	regTypeQWord:
	case	regTypeDQWord:
		WriteSSERegMemOperand
			( sseop_MOVQ_LOAD, 3,
				xmmDst, x86_Nothing, (INT_PTR) pDataImm ) ;
		break ;
	case	regTypeFloat64:
	case	regTypeDFloat64:
		WriteSSERegMemOperand
			( sseop_MOVSD_LOAD, 3,
				xmmDst, x86_Nothing, (INT_PTR) pDataImm ) ;
		break ;
	case	regTypeQFloat32:
		WriteSSERegMemOperand
			( sseop_MOVLPS_LOAD, 2,
				xmmDst, x86_Nothing, (INT_PTR) pDataImm ) ;
		break ;
	}
	SetDataRegisterModified( xmmDst ) ;
	UnlockDataRegister( xmmDst, regTypeQWord ) ;
}

// 1 OP 演算命令
//////////////////////////////////////////////////////////////////////////////
void X86SSE2Assembler::write_neg_int( int regDst )
{
	SSERegister	xmmFill =
		WriteRealizeDataRegister( regFillBit, regTypeQWord, true ) ;
	SSERegister	xmmDst =
		WriteRealizeDataRegister( regDst, regTypeQWord, true ) ;
	WriteSSERegRegOperand
		( sseop_PXOR, 3, xmmDst, xmmFill ) ;
	WriteSSERegRegOperand
		( sseop_PSUBQ, 3, xmmDst, xmmFill ) ;
	SetDataRegisterModified( xmmDst ) ;
	UnlockDataRegister( xmmFill, regTypeQWord ) ;
	UnlockDataRegister( xmmDst, regTypeQWord ) ;
}

void X86SSE2Assembler::write_not_int( int regDst )
{
	SSERegister	xmmFill =
		WriteRealizeDataRegister( regFillBit, regTypeQWord, true ) ;
	SSERegister	xmmDst =
		WriteRealizeDataRegister( regDst, regTypeQWord, true ) ;
	WriteSSERegRegOperand
		( sseop_PXOR, 3, xmmDst, xmmFill ) ;
	SetDataRegisterModified( xmmDst ) ;
	UnlockDataRegister( xmmFill, regTypeQWord ) ;
	UnlockDataRegister( xmmDst, regTypeQWord ) ;
}

void X86SSE2Assembler::write_neg_float( int regDst )
{
	void * pDataSignMask = GetConstantPair8000000000000000() ;
	//
	SSERegister	xmmDst =
		WriteRealizeDataRegister( regDst, regTypeFloat64, true ) ;
	WriteSSERegMemOperand
		( sseop_XORPD, 3, xmmDst, x86_Nothing, (INT_PTR) pDataSignMask ) ;
	SetDataRegisterModified( xmmDst ) ;
	UnlockDataRegister( xmmDst, regTypeFloat64 ) ;
}

// 2 OP 整数演算命令
//////////////////////////////////////////////////////////////////////////////
void X86SSE2Assembler::write_add_reg_reg( int regDst, int regSrc, bool fPair )
{
	RegisterDataType	regType = fPair ? regTypeDQWord : regTypeQWord ;
	SSERegister	xmmDst =
		WriteRealizeDataRegister( regDst, regType, true ) ;
	SSERegister	xmmSrc =
		WriteRealizeDataRegister( regSrc, regType, true ) ;
	//
	WriteSSERegRegOperand
		( sseop_PADDQ, 3, xmmDst, xmmSrc ) ;
	//
	SetDataRegisterModified( xmmDst ) ;
	UnlockDataRegister( xmmDst, regType ) ;
	UnlockDataRegister( xmmSrc, regType ) ;
}

void X86SSE2Assembler::write_sub_reg_reg( int regDst, int regSrc, bool fPair )
{
	RegisterDataType	regType = fPair ? regTypeDQWord : regTypeQWord ;
	SSERegister	xmmDst =
		WriteRealizeDataRegister( regDst, regType, true ) ;
	SSERegister	xmmSrc =
		WriteRealizeDataRegister( regSrc, regType, true ) ;
	//
	WriteSSERegRegOperand
		( sseop_PSUBQ, 3, xmmDst, xmmSrc ) ;
	//
	SetDataRegisterModified( xmmDst ) ;
	UnlockDataRegister( xmmDst, regType ) ;
	UnlockDataRegister( xmmSrc, regType ) ;
}

void X86SSE2Assembler::write_mul_reg_reg( int regDst, int regSrc, bool fPair )
{
	const int	countPair = fPair ? 2 : 1 ;
	for ( int i = 0; i < countPair; i ++ )
	{
		SSERegister	xmmDst =
			WriteRealizeDataRegister( regDst, regTypeQWord, true ) ;
		SSERegister	xmmSrc =
			WriteRealizeDataRegister( regSrc, regTypeQWord, true ) ;
		SSERegister	xmmTemp1 = AllocateDataRegister( regTypeQWord ) ;
		SSERegister	xmmTemp2 = AllocateDataRegister( regTypeQWord ) ;
		//
		write_mul_int64xint64( xmmDst, xmmSrc, xmmTemp1, xmmTemp2 ) ;
		//
		SetDataRegisterModified( xmmDst ) ;
		UnlockDataRegister( xmmDst, regTypeQWord ) ;
		UnlockDataRegister( xmmSrc, regTypeQWord ) ;
		FreeDataRegister( xmmTemp1, regTypeQWord ) ;
		FreeDataRegister( xmmTemp2, regTypeQWord ) ;
	}
}

//void X86SSE2Assembler::write_div_reg_reg( const void * pEscCode, int regDst, int regSrc ) ;
//void X86SSE2Assembler::write_mod_reg_reg( const void * pEscCode, int regDst, int regSrc ) ;

void X86SSE2Assembler::write_and_reg_reg( int regDst, int regSrc, bool fPair )
{
	RegisterDataType	regType = m_rdtSakura[regDst] ;
	if ( fPair )
	{
		regType = NormalizeDataTypeTo128( regType ) ;
	}
	else
	{
		regType = NormalizeDataTypeTo64( regType ) ;
	}
	SSERegister	xmmDst =
		WriteRealizeDataRegister( regDst, regType, true ) ;
	SSERegister	xmmSrc =
		WriteRealizeDataRegister( regSrc, regType, true ) ;
	//
	switch ( regType )
	{
	case	regTypeQWord:
	case	regTypeDQWord:
		WriteSSERegRegOperand
			( sseop_PAND, 3, xmmDst, xmmSrc ) ;
		break ;
	case	regTypeFloat64:
	case	regTypeDFloat64:
		WriteSSERegRegOperand
			( sseop_ANDPD, 3, xmmDst, xmmSrc ) ;
		break ;
	default:
	case	regTypeQFloat32:
		WriteSSERegRegOperand
			( sseop_ANDPS, 2, xmmDst, xmmSrc ) ;
		break ;
	}
	//
	SetDataRegisterModified( xmmDst ) ;
	UnlockDataRegister( xmmDst, regType ) ;
	UnlockDataRegister( xmmSrc, regType ) ;
}

void X86SSE2Assembler::write_or_reg_reg( int regDst, int regSrc, bool fPair )
{
	RegisterDataType	regType = m_rdtSakura[regDst] ;
	if ( fPair )
	{
		regType = NormalizeDataTypeTo128( regType ) ;
	}
	else
	{
		regType = NormalizeDataTypeTo64( regType ) ;
	}
	SSERegister	xmmDst =
		WriteRealizeDataRegister( regDst, regType, true ) ;
	SSERegister	xmmSrc =
		WriteRealizeDataRegister( regSrc, regType, true ) ;
	//
	switch ( regType )
	{
	case	regTypeQWord:
	case	regTypeDQWord:
		WriteSSERegRegOperand
			( sseop_POR, 3, xmmDst, xmmSrc ) ;
		break ;
	case	regTypeFloat64:
	case	regTypeDFloat64:
		WriteSSERegRegOperand
			( sseop_ORPD, 3, xmmDst, xmmSrc ) ;
		break ;
	default:
	case	regTypeQFloat32:
		WriteSSERegRegOperand
			( sseop_ORPS, 2, xmmDst, xmmSrc ) ;
		break ;
	}
	//
	SetDataRegisterModified( xmmDst ) ;
	UnlockDataRegister( xmmDst, regType ) ;
	UnlockDataRegister( xmmSrc, regType ) ;
}

void X86SSE2Assembler::write_xor_reg_reg( int regDst, int regSrc, bool fPair )
{
	RegisterDataType	regType = m_rdtSakura[regDst] ;
	if ( fPair )
	{
		regType = NormalizeDataTypeTo128( regType ) ;
	}
	else
	{
		regType = NormalizeDataTypeTo64( regType ) ;
	}
	SSERegister	xmmDst =
		WriteRealizeDataRegister( regDst, regType, true ) ;
	SSERegister	xmmSrc =
		WriteRealizeDataRegister( regSrc, regType, true ) ;
	//
	switch ( regType )
	{
	case	regTypeQWord:
	case	regTypeDQWord:
		WriteSSERegRegOperand
			( sseop_PXOR, 3, xmmDst, xmmSrc ) ;
		break ;
	case	regTypeFloat64:
	case	regTypeDFloat64:
		WriteSSERegRegOperand
			( sseop_XORPD, 3, xmmDst, xmmSrc ) ;
		break ;
	default:
	case	regTypeQFloat32:
		WriteSSERegRegOperand
			( sseop_XORPS, 2, xmmDst, xmmSrc ) ;
		break ;
	}
	//
	SetDataRegisterModified( xmmDst ) ;
	UnlockDataRegister( xmmDst, regType ) ;
	UnlockDataRegister( xmmSrc, regType ) ;
}

void X86SSE2Assembler::write_srl_reg_reg( int regDst, int regSrc, bool fPair )
{
	RegisterDataType	regType = fPair ? regTypeDQWord : regTypeQWord ;
	SSERegister	xmmSrc =
		WriteRealizeDataRegister( regSrc, regTypeQWord, true ) ;
	SSERegister	xmmTemp = AllocateDataRegister( regTypeQWord ) ;
	//
	void *	pMask3F = GetConstantDQWord3F() ;
	//
	WriteSSERegMemOperand
		( sseop_MOVDQA_LOAD, 3, xmmTemp, x86_Nothing, (INT_PTR) pMask3F ) ;
	WriteSSERegRegOperand
		( sseop_PAND, 3, xmmTemp, xmmSrc ) ;
	//
	UnlockDataRegister( xmmSrc, regTypeQWord ) ;
	//
	SSERegister	xmmDst =
		WriteRealizeDataRegister( regDst, regType, true ) ;
	//
	WriteSSERegRegOperand
		( sseop_PSRLQ, 3, xmmDst, xmmTemp ) ;
	//
	SetDataRegisterModified( xmmDst ) ;
	UnlockDataRegister( xmmDst, regType ) ;
	FreeDataRegister( xmmTemp, regTypeQWord ) ;
}

void X86SSE2Assembler::write_sra_reg_reg( int regDst, int regSrc, bool fPair )
{
	RegisterDataType	regType = fPair ? regTypeDQWord : regTypeQWord ;
	SSERegister	xmmSrc =
		WriteRealizeDataRegister( regSrc, regTypeQWord, true ) ;
	SSERegister	xmmSrcTemp = AllocateDataRegister( regTypeQWord ) ;
	SSERegister	xmmTemp = AllocateDataRegister( regTypeQWord ) ;
	SSERegister	xmmZero = AllocateDataRegister( regType ) ;
	//
	void *	pMask3F = GetConstantDQWord3F() ;
	void * pDataSignMask = GetConstantPair8000000000000000() ;
	//
	WriteSSERegMemOperand
		( sseop_MOVDQA_LOAD, 3, xmmSrcTemp, x86_Nothing, (INT_PTR) pMask3F ) ;
	WriteSSERegMemOperand
		( sseop_MOVDQA_LOAD, 3, xmmTemp, x86_Nothing, (INT_PTR) pDataSignMask ) ;
	WriteSSERegRegOperand( sseop_PAND, 3, xmmSrcTemp, xmmSrc ) ;
	//
	UnlockDataRegister( xmmSrc, regTypeQWord ) ;
	//
	SSERegister	xmmDst =
		WriteRealizeDataRegister( regDst, regType, true ) ;
	//
	WriteSSERegRegOperand( sseop_PAND, 3, xmmTemp, xmmDst ) ;
	WriteSSERegRegOperand( sseop_PXOR, 3, xmmZero, xmmZero ) ;
	WriteSSERegRegOperand( sseop_PSRLQ, 3, xmmTemp, xmmSrcTemp ) ;
	WriteSSERegRegOperand( sseop_PSRLQ, 3, xmmDst, xmmSrcTemp ) ;
	WriteSSERegRegOperand( sseop_PSUBQ, 3, xmmZero, xmmTemp ) ;
	WriteSSERegRegOperand( sseop_POR, 3, xmmDst, xmmZero ) ;
	//
	SetDataRegisterModified( xmmDst ) ;
	UnlockDataRegister( xmmDst, regType ) ;
	FreeDataRegister( xmmZero, regType ) ;
	FreeDataRegister( xmmTemp, regTypeQWord ) ;
	FreeDataRegister( xmmSrcTemp, regTypeQWord ) ;
}

void X86SSE2Assembler::write_sll_reg_reg( int regDst, int regSrc, bool fPair )
{
	RegisterDataType	regType = fPair ? regTypeDQWord : regTypeQWord ;
	SSERegister	xmmSrc =
		WriteRealizeDataRegister( regSrc, regTypeQWord, true ) ;
	SSERegister	xmmTemp = AllocateDataRegister( regTypeQWord ) ;
	//
	void *	pMask3F = GetConstantDQWord3F() ;
	//
	WriteSSERegMemOperand
		( sseop_MOVDQA_LOAD, 3, xmmTemp, x86_Nothing, (INT_PTR) pMask3F ) ;
	WriteSSERegRegOperand
		( sseop_PAND, 3, xmmTemp, xmmSrc ) ;
	//
	UnlockDataRegister( xmmSrc, regTypeQWord ) ;
	//
	SSERegister	xmmDst =
		WriteRealizeDataRegister( regDst, regType, true ) ;
	//
	WriteSSERegRegOperand
		( sseop_PSLLQ, 3, xmmDst, xmmTemp ) ;
	//
	SetDataRegisterModified( xmmDst ) ;
	UnlockDataRegister( xmmDst, regType ) ;
	FreeDataRegister( xmmTemp, regTypeQWord ) ;
}

// 整数符号拡張命令
//////////////////////////////////////////////////////////////////////////////
void X86SSE2Assembler::write_move_sx32_reg_reg( int regDst, int regSrc )
{
	WriteToLoadSakura2Register( x86_EAX, x86_EDX, regSrc, true ) ;
	WriteX86ImmediateOperand( x86op1_CDQ, 1 ) ;
	WriteToStoreSakura2Register( regDst, x86_EAX, x86_EDX ) ;
}

void X86SSE2Assembler::write_move_sx16_reg_reg( int regDst, int regSrc )
{
	WriteToLoadSakura2Register( x86_EAX, x86_EDX, regSrc, true ) ;
	WriteX86RegMemOperand
		( x86op2_MOVSX_16, 2, x86_EAX, false, x86_EAX ) ;
	WriteX86ImmediateOperand( x86op1_CDQ, 1 ) ;
	WriteToStoreSakura2Register( regDst, x86_EAX, x86_EDX ) ;
}

void X86SSE2Assembler::write_move_sx8_reg_reg( int regDst, int regSrc )
{
	WriteToLoadSakura2Register( x86_EAX, x86_EDX, regSrc, true ) ;
	WriteX86RegMemOperand
		( x86op2_MOVSX_8, 2, x86_EAX, false, x86_EAX ) ;
	WriteX86ImmediateOperand( x86op1_CDQ, 1 ) ;
	WriteToStoreSakura2Register( regDst, x86_EAX, x86_EDX ) ;
}

// 2 OP 実数演算命令
//////////////////////////////////////////////////////////////////////////////
void X86SSE2Assembler::write_fadd_reg_reg( int regDst, int regSrc, bool fPair )
{
	RegisterDataType	regType = fPair ? regTypeDFloat64 : regTypeFloat64 ;
	if ( regSrc != regIntZero )
	{
		SSERegister	xmmDst =
			WriteRealizeDataRegister( regDst, regType, true ) ;
		SSERegister	xmmSrc =
			WriteRealizeDataRegister( regSrc, regType, true ) ;
		//
		if ( fPair )
		{
			WriteSSERegRegOperand( sseop_ADDPD, 3, xmmDst, xmmSrc ) ;
		}
		else
		{
			WriteSSERegRegOperand( sseop_ADDSD, 3, xmmDst, xmmSrc ) ;
		}
		//
		SetDataRegisterModified( xmmDst ) ;
		UnlockDataRegister( xmmDst, regType ) ;
		UnlockDataRegister( xmmSrc, regType ) ;
	}
	else
	{
		if ( fPair )
		{
			m_rdtSakura[regDst & ~0x01] = regTypeDFloat64 ;
			m_rdtSakura[regDst | 0x01] = regTypeDFloat64 ;
			m_rdtSakura[regSrc & ~0x01] = regTypeDFloat64 ;
			m_rdtSakura[regSrc | 0x01] = regTypeDFloat64 ;
		}
		else
		{
			m_rdtSakura[regDst] = regTypeFloat64 ;
			m_rdtSakura[regSrc] = regTypeFloat64 ;
		}
	}
}

void X86SSE2Assembler::write_fsub_reg_reg( int regDst, int regSrc, bool fPair )
{
	RegisterDataType	regType = fPair ? regTypeDFloat64 : regTypeFloat64 ;
	SSERegister	xmmDst =
		WriteRealizeDataRegister( regDst, regType, true ) ;
	SSERegister	xmmSrc =
		WriteRealizeDataRegister( regSrc, regType, true ) ;
	//
	if ( fPair )
	{
		WriteSSERegRegOperand( sseop_SUBPD, 3, xmmDst, xmmSrc ) ;
	}
	else
	{
		WriteSSERegRegOperand( sseop_SUBSD, 3, xmmDst, xmmSrc ) ;
	}
	//
	SetDataRegisterModified( xmmDst ) ;
	UnlockDataRegister( xmmDst, regType ) ;
	UnlockDataRegister( xmmSrc, regType ) ;
}

void X86SSE2Assembler::write_fmul_reg_reg( int regDst, int regSrc, bool fPair )
{
	RegisterDataType	regType = fPair ? regTypeDFloat64 : regTypeFloat64 ;
	SSERegister	xmmDst =
		WriteRealizeDataRegister( regDst, regType, true ) ;
	SSERegister	xmmSrc =
		WriteRealizeDataRegister( regSrc, regType, true ) ;
	//
	if ( fPair )
	{
		WriteSSERegRegOperand( sseop_MULPD, 3, xmmDst, xmmSrc ) ;
	}
	else
	{
		WriteSSERegRegOperand( sseop_MULSD, 3, xmmDst, xmmSrc ) ;
	}
	//
	SetDataRegisterModified( xmmDst ) ;
	UnlockDataRegister( xmmDst, regType ) ;
	UnlockDataRegister( xmmSrc, regType ) ;
}

void X86SSE2Assembler::write_fdiv_reg_reg( int regDst, int regSrc, bool fPair )
{
	RegisterDataType	regType = fPair ? regTypeDFloat64 : regTypeFloat64 ;
	SSERegister	xmmDst =
		WriteRealizeDataRegister( regDst, regType, true ) ;
	SSERegister	xmmSrc =
		WriteRealizeDataRegister( regSrc, regType, true ) ;
	//
	if ( fPair )
	{
		WriteSSERegRegOperand( sseop_DIVPD, 3, xmmDst, xmmSrc ) ;
	}
	else
	{
		WriteSSERegRegOperand( sseop_DIVSD, 3, xmmDst, xmmSrc ) ;
	}
	//
	SetDataRegisterModified( xmmDst ) ;
	UnlockDataRegister( xmmDst, regType ) ;
	UnlockDataRegister( xmmSrc, regType ) ;
}

// 特殊精度整数演算命令
//////////////////////////////////////////////////////////////////////////////
void X86SSE2Assembler::write_mul32_reg_reg( int regDst, int regSrc )
{
	SSERegister	xmmDst =
		WriteRealizeDataRegister( regDst, regTypeQWord, true ) ;
	SSERegister	xmmSrc =
		WriteRealizeDataRegister( regSrc, regTypeQWord, true ) ;
	//
	WriteSSERegRegOperand( sseop_PMULUDQ, 3, xmmDst, xmmSrc ) ;
	//
	SetDataRegisterModified( xmmDst ) ;
	UnlockDataRegister( xmmDst, regTypeQWord ) ;
	UnlockDataRegister( xmmSrc, regTypeQWord ) ;
}

void X86SSE2Assembler::write_imul32_reg_reg( int regDst, int regSrc )
{
	WriteToLoadSakura2Register( x86_EAX, x86_EAX, regDst, true ) ;
	WriteToLoadSakura2Register( x86_EDX, x86_EDX, regSrc, true ) ;
	//
	WriteX86RegMemOperand
		( x86op1_IMUL_RM, 1, x86op2nd_IMUL_RM, false, x86_EDX ) ;
	//
	WriteToStoreSakura2Register( regDst, x86_EAX, x86_EDX ) ;
}

void X86SSE2Assembler::write_div32_reg_reg( const void * pEscCode, int regDst, int regSrc )
{
	bool	fAssignPtrECX = m_lruPointer.IsSlotAssigned( regPtrPhyECX ) ;
	if ( fAssignPtrECX )
	{
		WriteX86PushReg( x86_ECX ) ;
	}
	WriteToLoadSakura2Register( x86_ECX, x86_EDX, regSrc, true ) ;
	WriteToLoadSakura2Register( x86_EAX, x86_EDX, regDst ) ;
	WriteX86CmpRegMem( x86_ECX, false, x86_EDX ) ;
	void *	pJaNoEsc =
		WriteX86ImmediateOperand( x86op2_JA, 2, 0, 4 ) ;
	//
	if ( fAssignPtrECX )
	{
		WriteX86PopReg( x86_ECX ) ;
	}
	WriteBackAllRegisters() ;
	WriteX86JmpImm32( pEscCode ) ;
	//
	CommitJumpTarget( pJaNoEsc, GetNextAddress() ) ;
	//
	WriteX86RegMemOperand
		( x86op1_DIV_RM, 1, x86op2nd_DIV_RM, false, x86_ECX ) ;
	if ( fAssignPtrECX )
	{
		WriteX86PopReg( x86_ECX ) ;
	}
	WriteX86XorRegMem( x86_EDX, false, x86_EDX ) ;
	//
	WriteToStoreSakura2Register( regDst, x86_EAX, x86_EDX ) ;
}

void X86SSE2Assembler::write_idiv32_reg_reg( const void * pEscCode, int regDst, int regSrc )
{
	bool	fAssignPtrECX = m_lruPointer.IsSlotAssigned( regPtrPhyECX ) ;
	if ( fAssignPtrECX )
	{
		WriteX86PushReg( x86_ECX ) ;
	}
	WriteToLoadSakura2Register( x86_ECX, x86_EDX, regSrc, true ) ;
	WriteToLoadSakura2Register( x86_EAX, x86_EDX, regDst ) ;
	WriteX86OrRegMem( x86_ECX, false, x86_ECX ) ;
	void *	pJneNoEsc =
		WriteX86ImmediateOperand( x86op2_JNE, 2, 0, 4 ) ;
	//
	if ( fAssignPtrECX )
	{
		WriteX86PopReg( x86_ECX ) ;
	}
	WriteBackAllRegisters() ;
	WriteX86JmpImm32( pEscCode ) ;
	//
	CommitJumpTarget( pJneNoEsc, GetNextAddress() ) ;
	//
	WriteX86RegMemOperand
		( x86op1_IDIV_RM, 1, x86op2nd_IDIV_RM, false, x86_ECX ) ;
	if ( fAssignPtrECX )
	{
		WriteX86PopReg( x86_ECX ) ;
	}
	WriteX86ImmediateOperand( x86op1_CDQ, 1 ) ;
	//
	WriteToStoreSakura2Register( regDst, x86_EAX, x86_EDX ) ;
}

void X86SSE2Assembler::write_mod32_reg_reg( const void * pEscCode, int regDst, int regSrc )
{
	bool	fAssignPtrECX = m_lruPointer.IsSlotAssigned( regPtrPhyECX ) ;
	if ( fAssignPtrECX )
	{
		WriteX86PushReg( x86_ECX ) ;
	}
	WriteToLoadSakura2Register( x86_ECX, x86_EDX, regSrc, true ) ;
	WriteToLoadSakura2Register( x86_EAX, x86_EDX, regDst ) ;
	WriteX86CmpRegMem( x86_ECX, false, x86_EDX ) ;
	void *	pJaNoEsc =
		WriteX86ImmediateOperand( x86op2_JA, 2, 0, 4 ) ;
	//
	if ( fAssignPtrECX )
	{
		WriteX86PopReg( x86_ECX ) ;
	}
	WriteBackAllRegisters() ;
	WriteX86JmpImm32( pEscCode ) ;
	//
	CommitJumpTarget( pJaNoEsc, GetNextAddress() ) ;
	//
	WriteX86RegMemOperand
		( x86op1_DIV_RM, 1, x86op2nd_DIV_RM, false, x86_ECX ) ;
	if ( fAssignPtrECX )
	{
		WriteX86PopReg( x86_ECX ) ;
	}
	WriteX86XorRegMem( x86_EAX, false, x86_EAX ) ;
	//
	WriteToStoreSakura2Register( regDst, x86_EDX, x86_EAX ) ;
}

void X86SSE2Assembler::write_imod32_reg_reg( const void * pEscCode, int regDst, int regSrc )
{
	bool	fAssignPtrECX = m_lruPointer.IsSlotAssigned( regPtrPhyECX ) ;
	if ( fAssignPtrECX )
	{
		WriteX86PushReg( x86_ECX ) ;
	}
	WriteToLoadSakura2Register( x86_ECX, x86_EDX, regSrc, true ) ;
	WriteToLoadSakura2Register( x86_EAX, x86_EDX, regDst ) ;
	WriteX86OrRegMem( x86_ECX, false, x86_ECX ) ;
	void *	pJneNoEsc =
		WriteX86ImmediateOperand( x86op2_JNE, 2, 0, 4 ) ;
	//
	if ( fAssignPtrECX )
	{
		WriteX86PopReg( x86_ECX ) ;
	}
	WriteBackAllRegisters() ;
	WriteX86JmpImm32( pEscCode ) ;
	//
	CommitJumpTarget( pJneNoEsc, GetNextAddress() ) ;
	//
	WriteX86RegMemOperand
		( x86op1_IDIV_RM, 1, x86op2nd_IDIV_RM, false, x86_ECX ) ;
	if ( fAssignPtrECX )
	{
		WriteX86PopReg( x86_ECX ) ;
	}
	WriteX86MoveRegReg( x86_EAX, x86_EDX ) ;
	WriteX86ImmediateOperand( x86op1_CDQ, 1 ) ;
	//
	WriteToStoreSakura2Register( regDst, x86_EAX, x86_EDX ) ;
}

// 整数比較命令
//////////////////////////////////////////////////////////////////////////////
void X86SSE2Assembler::write_cmp_ne( int regDst, int regSrc, bool fPair )
{
	RegisterDataType	regType = fPair ? regTypeDQWord : regTypeQWord ;
	if ( regDst != regSrc )
	{
		SSERegister	xmmDst =
			WriteRealizeDataRegister( regDst, regType, true ) ;
		SSERegister	xmmSrc =
			WriteRealizeDataRegister( regSrc, regType, true ) ;
		SSERegister	xmmFill = AllocateDataRegister( regType ) ;
		SSERegister	xmmTemp = AllocateDataRegister( regType ) ;
		//
		WriteSSERegRegOperand( sseop_PCMPEQD, 3, xmmDst, xmmSrc ) ;
		WriteSSERegRegOperand( sseop_PCMPEQD, 3, xmmFill, xmmFill ) ;
		WriteSSERegRegOperand( sseop_PSHUFD, 3, xmmTemp, xmmDst, 0xB1, 1 ) ;
		WriteSSERegRegOperand( sseop_PAND, 3, xmmDst, xmmTemp ) ;
		WriteSSERegRegOperand( sseop_PXOR, 3, xmmDst, xmmFill ) ;
		//
		SetDataRegisterModified( xmmDst ) ;
		FreeDataRegister( xmmTemp, regType ) ;
		FreeDataRegister( xmmFill, regType ) ;
		UnlockDataRegister( xmmDst, regType ) ;
		UnlockDataRegister( xmmSrc, regType ) ;
	}
	else
	{
		SSERegister	xmmDst =
			WriteRealizeDataRegister( regDst, regType, false ) ;
		//
		WriteSSERegRegOperand( sseop_PXOR, 3, xmmDst, xmmDst ) ;
		//
		SetDataRegisterModified( xmmDst ) ;
		UnlockDataRegister( xmmDst, regType ) ;
	}
}

void X86SSE2Assembler::write_cmp_eq( int regDst, int regSrc, bool fPair )
{
	RegisterDataType	regType = fPair ? regTypeDQWord : regTypeQWord ;
	if ( regDst != regSrc )
	{
		SSERegister	xmmDst =
			WriteRealizeDataRegister( regDst, regType, true ) ;
		SSERegister	xmmSrc =
			WriteRealizeDataRegister( regSrc, regType, true ) ;
		SSERegister	xmmTemp = AllocateDataRegister( regType ) ;
		//
		WriteSSERegRegOperand( sseop_PCMPEQD, 3, xmmDst, xmmSrc ) ;
		WriteSSERegRegOperand( sseop_PSHUFD, 3, xmmTemp, xmmDst, 0xB1, 1 ) ;
		WriteSSERegRegOperand( sseop_PAND, 3, xmmDst, xmmTemp ) ;
		//
		SetDataRegisterModified( xmmDst ) ;
		FreeDataRegister( xmmTemp, regType ) ;
		UnlockDataRegister( xmmDst, regType ) ;
		UnlockDataRegister( xmmSrc, regType ) ;
	}
	else
	{
		SSERegister	xmmDst =
			WriteRealizeDataRegister( regDst, regType, false ) ;
		//
		WriteSSERegRegOperand( sseop_PCMPEQD, 3, xmmDst, xmmDst ) ;
		//
		SetDataRegisterModified( xmmDst ) ;
		UnlockDataRegister( xmmDst, regType ) ;
	}
}

void X86SSE2Assembler::write_cmp_lt( int regDst, int regSrc, bool fPair )
{
	RegisterDataType	regType = fPair ? regTypeDQWord : regTypeQWord ;
	if ( regDst != regSrc )
	{
		SSERegister	xmmSrc =
			WriteRealizeDataRegister( regSrc, regType, true ) ;
		SSERegister	xmmDst =
			WriteRealizeDataRegister( regDst, regType, true ) ;
		SSERegister	xmmTemp1 = AllocateDataRegister( regType ) ;
		SSERegister	xmmTemp2 = AllocateDataRegister( regType ) ;
		//
		write_cmp_int64_gt
			( xmmDst, xmmTemp1, xmmTemp2, xmmSrc, xmmDst, false, fPair ) ;
		//
		SetDataRegisterModified( xmmDst ) ;
		FreeDataRegister( xmmTemp1, regType ) ;
		FreeDataRegister( xmmTemp2, regType ) ;
		UnlockDataRegister( xmmDst, regType ) ;
		UnlockDataRegister( xmmSrc, regType ) ;
	}
	else
	{
		SSERegister	xmmDst =
			WriteRealizeDataRegister( regDst, regType, false ) ;
		//
		WriteSSERegRegOperand( sseop_PXOR, 3, xmmDst, xmmDst ) ;
		//
		SetDataRegisterModified( xmmDst ) ;
		UnlockDataRegister( xmmDst, regType ) ;
	}
}

void X86SSE2Assembler::write_cmp_le( int regDst, int regSrc, bool fPair )
{
	RegisterDataType	regType = fPair ? regTypeDQWord : regTypeQWord ;
	if ( regDst != regSrc )
	{
		SSERegister	xmmSrc =
			WriteRealizeDataRegister( regSrc, regType, true ) ;
		SSERegister	xmmDst =
			WriteRealizeDataRegister( regDst, regType, true ) ;
		SSERegister	xmmTemp1 = AllocateDataRegister( regType ) ;
		SSERegister	xmmTemp2 = AllocateDataRegister( regType ) ;
		//
		write_cmp_int64_gt
			( xmmDst, xmmTemp1, xmmTemp2, xmmDst, xmmSrc, true, fPair ) ;
		//
		SetDataRegisterModified( xmmDst ) ;
		FreeDataRegister( xmmTemp1, regType ) ;
		FreeDataRegister( xmmTemp2, regType ) ;
		UnlockDataRegister( xmmDst, regType ) ;
		UnlockDataRegister( xmmSrc, regType ) ;
	}
	else
	{
		SSERegister	xmmDst =
			WriteRealizeDataRegister( regDst, regType, false ) ;
		//
		WriteSSERegRegOperand( sseop_PCMPEQD, 3, xmmDst, xmmDst ) ;
		//
		SetDataRegisterModified( xmmDst ) ;
		UnlockDataRegister( xmmDst, regType ) ;
	}
}

void X86SSE2Assembler::write_cmp_gt( int regDst, int regSrc, bool fPair )
{
	RegisterDataType	regType = fPair ? regTypeDQWord : regTypeQWord ;
	if ( regDst != regSrc )
	{
		SSERegister	xmmSrc =
			WriteRealizeDataRegister( regSrc, regType, true ) ;
		SSERegister	xmmDst =
			WriteRealizeDataRegister( regDst, regType, true ) ;
		SSERegister	xmmTemp1 = AllocateDataRegister( regType ) ;
		SSERegister	xmmTemp2 = AllocateDataRegister( regType ) ;
		//
		write_cmp_int64_gt
			( xmmDst, xmmTemp1, xmmTemp2, xmmDst, xmmSrc, false, fPair ) ;
		//
		SetDataRegisterModified( xmmDst ) ;
		FreeDataRegister( xmmTemp1, regType ) ;
		FreeDataRegister( xmmTemp2, regType ) ;
		UnlockDataRegister( xmmDst, regType ) ;
		UnlockDataRegister( xmmSrc, regType ) ;
	}
	else
	{
		SSERegister	xmmDst =
			WriteRealizeDataRegister( regDst, regType, false ) ;
		//
		WriteSSERegRegOperand( sseop_PXOR, 3, xmmDst, xmmDst ) ;
		//
		SetDataRegisterModified( xmmDst ) ;
		UnlockDataRegister( xmmDst, regType ) ;
	}
}

void X86SSE2Assembler::write_cmp_ge( int regDst, int regSrc, bool fPair )
{
	RegisterDataType	regType = fPair ? regTypeDQWord : regTypeQWord ;
	if ( regDst != regSrc )
	{
		SSERegister	xmmSrc =
			WriteRealizeDataRegister( regSrc, regType, true ) ;
		SSERegister	xmmDst =
			WriteRealizeDataRegister( regDst, regType, true ) ;
		SSERegister	xmmTemp1 = AllocateDataRegister( regType ) ;
		SSERegister	xmmTemp2 = AllocateDataRegister( regType ) ;
		//
		write_cmp_int64_gt
			( xmmDst, xmmTemp1, xmmTemp2, xmmSrc, xmmDst, true, fPair ) ;
		//
		SetDataRegisterModified( xmmDst ) ;
		FreeDataRegister( xmmTemp1, regType ) ;
		FreeDataRegister( xmmTemp2, regType ) ;
		UnlockDataRegister( xmmDst, regType ) ;
		UnlockDataRegister( xmmSrc, regType ) ;
	}
	else
	{
		SSERegister	xmmDst =
			WriteRealizeDataRegister( regDst, regType, false ) ;
		//
		WriteSSERegRegOperand( sseop_PCMPEQD, 3, xmmDst, xmmDst ) ;
		//
		SetDataRegisterModified( xmmDst ) ;
		UnlockDataRegister( xmmDst, regType ) ;
	}
}

void X86SSE2Assembler::write_cmp_c( int regDst, int regSrc, bool fPair )
{
	RegisterDataType	regType = fPair ? regTypeDQWord : regTypeQWord ;
	if ( regDst != regSrc )
	{
		const int	countPair = fPair ? 2 : 1 ;
		for ( int i = 0; i < countPair; i ++ )
		{
			FlushRegister( regSrc + i ) ;
			//
			WriteToLoadSakura2Register( x86_EAX, x86_EDX, regDst + i ) ;
			//
			SSERegister	xmmDst =
				WriteRealizeDataRegister( regDst + i, regTypeQWord, false ) ;
			//
			WriteX86RegMemOperand
				( x86op1_SUB_REG_RM, 1, x86_EAX,
					true, x86_EBX, Context::OffsetOfReg(regSrc + i) ) ;
			WriteX86RegMemOperand
				( x86op1_SBB_REG_RM, 1, x86_EDX,
					true, x86_EBX, Context::OffsetOfReg(regSrc + i) + 4 ) ;
			WriteX86RegMemOperand
				( x86op1_SBB_REG_RM, 1, x86_EAX, false, x86_EAX ) ;
			//
			WriteSSERegRegOperand
				( sseop_MOVD_LOAD, 3, xmmDst, (SSERegister) x86_EAX ) ;
			WriteSSERegRegOperand
				( sseop_PUNPCKLDQ, 3, xmmDst, xmmDst ) ;
			//
			SetDataRegisterModified( xmmDst ) ;
			UnlockDataRegister( xmmDst, regType ) ;
		}
	}
	else
	{
		SSERegister	xmmDst =
			WriteRealizeDataRegister( regDst, regType, false ) ;
		//
		WriteSSERegRegOperand( sseop_PXOR, 3, xmmDst, xmmDst ) ;
		//
		SetDataRegisterModified( xmmDst ) ;
		UnlockDataRegister( xmmDst, regType ) ;
	}
}

void X86SSE2Assembler::write_cmp_cz( int regDst, int regSrc, bool fPair )
{
	RegisterDataType	regType = fPair ? regTypeDQWord : regTypeQWord ;
	if ( regDst != regSrc )
	{
		const int	countPair = fPair ? 2 : 1 ;
		for ( int i = 0; i < countPair; i ++ )
		{
			FlushRegister( regDst + i ) ;
			//
			WriteToLoadSakura2Register( x86_EAX, x86_EDX, regSrc + i ) ;
			//
			SSERegister	xmmDst =
				WriteRealizeDataRegister( regDst + i, regTypeQWord, false ) ;
			//
			WriteX86RegMemOperand
				( x86op1_SUB_REG_RM, 1, x86_EAX,
					true, x86_EBX, Context::OffsetOfReg(regDst + i) ) ;
			WriteX86RegMemOperand
				( x86op1_SBB_REG_RM, 1, x86_EDX,
					true, x86_EBX, Context::OffsetOfReg(regDst + i) + 4 ) ;
			WriteX86RegMemOperand
				( x86op1_SBB_REG_RM, 1, x86_EAX, false, x86_EAX ) ;
			WriteX86RegMemOperand
				( x86op1_NOT_RM, 1, x86op2nd_NOT_RM, false, x86_EAX ) ;
			WriteX86RegMemOperand
				( x86op2_MOVZX_8, 2, x86_EAX, false, x86_AL ) ;
			WriteX86RegMemOperand
				( x86op1_NEG_RM, 1, x86op2nd_NEG_RM, false, x86_EAX ) ;
			//
			WriteSSERegRegOperand
				( sseop_MOVD_LOAD, 3, xmmDst, (SSERegister) x86_EAX ) ;
			WriteSSERegRegOperand
				( sseop_PUNPCKLDQ, 3, xmmDst, xmmDst ) ;
			//
			SetDataRegisterModified( xmmDst ) ;
			UnlockDataRegister( xmmDst, regType ) ;
		}
	}
	else
	{
		SSERegister	xmmDst =
			WriteRealizeDataRegister( regDst, regType, false ) ;
		//
		WriteSSERegRegOperand( sseop_PCMPEQD, 3, xmmDst, xmmDst ) ;
		//
		SetDataRegisterModified( xmmDst ) ;
		UnlockDataRegister( xmmDst, regType ) ;
	}
}

// 実数比較命令
//////////////////////////////////////////////////////////////////////////////
void X86SSE2Assembler::write_fcmp_ne( int regDst, int regSrc, bool fPair )
{
	RegisterDataType	regType = fPair ? regTypeDFloat64 : regTypeFloat64 ;
	SSERegister	xmmSrc = WriteRealizeDataRegister( regSrc, regType, true ) ;
	SSERegister	xmmDst = WriteRealizeDataRegister( regDst, regType, true ) ;
	if ( fPair )
	{
		WriteSSERegRegOperand
			( sseop_CMPPD, 3, xmmDst, xmmSrc, sseimm_CMP_NEQ, 1 ) ;
	}
	else
	{
		WriteSSERegRegOperand
			( sseop_CMPSD, 3, xmmDst, xmmSrc, sseimm_CMP_NEQ, 1 ) ;
	}
	SetDataRegisterModified( xmmDst ) ;
	UnlockDataRegister( xmmDst, regType ) ;
	UnlockDataRegister( xmmSrc, regType ) ;
}

void X86SSE2Assembler::write_fcmp_eq( int regDst, int regSrc, bool fPair )
{
	RegisterDataType	regType = fPair ? regTypeDFloat64 : regTypeFloat64 ;
	SSERegister	xmmSrc = WriteRealizeDataRegister( regSrc, regType, true ) ;
	SSERegister	xmmDst = WriteRealizeDataRegister( regDst, regType, true ) ;
	if ( fPair )
	{
		WriteSSERegRegOperand
			( sseop_CMPPD, 3, xmmDst, xmmSrc, sseimm_CMP_EQ, 1 ) ;
	}
	else
	{
		WriteSSERegRegOperand
			( sseop_CMPSD, 3, xmmDst, xmmSrc, sseimm_CMP_EQ, 1 ) ;
	}
	SetDataRegisterModified( xmmDst ) ;
	UnlockDataRegister( xmmDst, regType ) ;
	UnlockDataRegister( xmmSrc, regType ) ;
}

void X86SSE2Assembler::write_fcmp_lt( int regDst, int regSrc, bool fPair )
{
	RegisterDataType	regType = fPair ? regTypeDFloat64 : regTypeFloat64 ;
	SSERegister	xmmSrc = WriteRealizeDataRegister( regSrc, regType, true ) ;
	SSERegister	xmmDst = WriteRealizeDataRegister( regDst, regType, true ) ;
	if ( fPair )
	{
		WriteSSERegRegOperand
			( sseop_CMPPD, 3, xmmDst, xmmSrc, sseimm_CMP_LT, 1 ) ;
	}
	else
	{
		WriteSSERegRegOperand
			( sseop_CMPSD, 3, xmmDst, xmmSrc, sseimm_CMP_LT, 1 ) ;
	}
	SetDataRegisterModified( xmmDst ) ;
	UnlockDataRegister( xmmDst, regType ) ;
	UnlockDataRegister( xmmSrc, regType ) ;
}

void X86SSE2Assembler::write_fcmp_le( int regDst, int regSrc, bool fPair )
{
	RegisterDataType	regType = fPair ? regTypeDFloat64 : regTypeFloat64 ;
	SSERegister	xmmSrc = WriteRealizeDataRegister( regSrc, regType, true ) ;
	SSERegister	xmmDst = WriteRealizeDataRegister( regDst, regType, true ) ;
	if ( fPair )
	{
		WriteSSERegRegOperand
			( sseop_CMPPD, 3, xmmDst, xmmSrc, sseimm_CMP_LE, 1 ) ;
	}
	else
	{
		WriteSSERegRegOperand
			( sseop_CMPSD, 3, xmmDst, xmmSrc, sseimm_CMP_LE, 1 ) ;
	}
	SetDataRegisterModified( xmmDst ) ;
	UnlockDataRegister( xmmDst, regType ) ;
	UnlockDataRegister( xmmSrc, regType ) ;
}

void X86SSE2Assembler::write_fcmp_gt( int regDst, int regSrc, bool fPair )
{
	RegisterDataType	regType = fPair ? regTypeDFloat64 : regTypeFloat64 ;
	SSERegister	xmmSrc = WriteRealizeDataRegister( regSrc, regType, true ) ;
	SSERegister	xmmDst = WriteRealizeDataRegister( regDst, regType, true ) ;
	if ( fPair )
	{
		WriteSSERegRegOperand
			( sseop_CMPPD, 3, xmmDst, xmmSrc, sseimm_CMP_GT, 1 ) ;
	}
	else
	{
		WriteSSERegRegOperand
			( sseop_CMPSD, 3, xmmDst, xmmSrc, sseimm_CMP_GT, 1 ) ;
	}
	SetDataRegisterModified( xmmDst ) ;
	UnlockDataRegister( xmmDst, regType ) ;
	UnlockDataRegister( xmmSrc, regType ) ;
}

void X86SSE2Assembler::write_fcmp_ge( int regDst, int regSrc, bool fPair )
{
	RegisterDataType	regType = fPair ? regTypeDFloat64 : regTypeFloat64 ;
	SSERegister	xmmSrc = WriteRealizeDataRegister( regSrc, regType, true ) ;
	SSERegister	xmmDst = WriteRealizeDataRegister( regDst, regType, true ) ;
	if ( fPair )
	{
		WriteSSERegRegOperand
			( sseop_CMPPD, 3, xmmDst, xmmSrc, sseimm_CMP_GE, 1 ) ;
	}
	else
	{
		WriteSSERegRegOperand
			( sseop_CMPSD, 3, xmmDst, xmmSrc, sseimm_CMP_GE, 1 ) ;
	}
	SetDataRegisterModified( xmmDst ) ;
	UnlockDataRegister( xmmDst, regType ) ;
	UnlockDataRegister( xmmSrc, regType ) ;
}

// 64bit 整数比較命令生成
//	xmmDst <- (xmmCmp1 > xmmCmp2) ^ fLogicalNot
//////////////////////////////////////////////////////////////////////////////
void X86SSE2Assembler::write_cmp_int64_gt
	( X86SSE2Assembler::SSERegister xmmDst,
		X86SSE2Assembler::SSERegister xmmTemp1,
		X86SSE2Assembler::SSERegister xmmTemp2,
		X86SSE2Assembler::SSERegister xmmCmp1,
		X86SSE2Assembler::SSERegister xmmCmp2,
		bool fLogicalNot, bool fPair )
{
	void *	pDataPair80000000 = GetConstantPair80000000() ;
	if ( fPair )
	{
		WriteSSERegMemOperand
			( sseop_MOVDQA_LOAD, 3,
				xmmTemp1, x86_Nothing, (INT_PTR) pDataPair80000000 ) ;
	}
	else
	{
		WriteSSERegMemOperand
			( sseop_MOVQ_LOAD, 3,
				xmmTemp1, x86_Nothing, (INT_PTR) pDataPair80000000 ) ;
	}
	if ( (xmmDst != xmmCmp1) && (xmmDst != xmmCmp2) )
	{
		WriteSSERegRegOperand
			( sseop_MOVDQA_LOAD, 3, xmmDst, xmmCmp1 ) ;
		xmmCmp1 = xmmDst ;
	}
	WriteSSERegRegOperand( sseop_MOVDQA_LOAD, 3, xmmTemp2, xmmCmp1 ) ;
	WriteSSERegRegOperand( sseop_PXOR, 3, xmmDst, xmmTemp1 ) ;
	WriteSSERegRegOperand( sseop_PCMPEQD, 3, xmmTemp2, xmmCmp2 ) ;
	if ( xmmDst == xmmCmp1 )
	{
		WriteSSERegRegOperand( sseop_PXOR, 3, xmmTemp1, xmmCmp2 ) ;
		xmmCmp1 = xmmDst ;
		xmmCmp2 = xmmTemp1 ;
	}
	else
	{
		ESLAssert( xmmDst == xmmCmp2 ) ;
		WriteSSERegRegOperand( sseop_PXOR, 3, xmmTemp1, xmmCmp1 ) ;
		xmmCmp1 = xmmTemp1 ;
		xmmCmp2 = xmmDst ;
	}
	WriteSSERegRegImm8Operand
		( sseop_PSHIFTQ_IMM, 3, mmxop2nd_SRL, xmmTemp2, 32 ) ;
	WriteSSERegRegOperand( sseop_PCMPGTD, 3, xmmCmp1, xmmCmp2 ) ;
	WriteSSERegRegOperand( sseop_PAND, 3, xmmTemp2, xmmCmp1 ) ;
	//
	// xmmDst := signed(xmmCmp1[High] > xmmCmp2[High])
	WriteSSERegRegOperand( sseop_PSHUFD, 3, xmmDst, xmmCmp1, 0xF5, 1 ) ;
	//
	if ( fLogicalNot )
	{
		WriteSSERegRegOperand( sseop_PCMPEQD, 3, xmmTemp1, xmmTemp1 ) ;
	}
	//
	// xmmTemp2 := unsigned(xmmCmp1[Low] > xmmCmp2[Low])
	//					&& (xmmCmp1[High] == xmmCmp2[High])
	WriteSSERegRegOperand( sseop_PSHUFD, 3, xmmTemp2, xmmTemp2, 0xA0, 1 ) ;
	//
	WriteSSERegRegOperand( sseop_POR, 3, xmmDst, xmmTemp2 ) ;
	//
	if ( fLogicalNot )
	{
		WriteSSERegRegOperand( sseop_PXOR, 3, xmmDst, xmmTemp1 ) ;
	}
}

// 浮動小数点演算 EXTENSION
//////////////////////////////////////////////////////////////////////////////
void X86SSE2Assembler::write_float_extension( int code, int regDst, int regSrc )
{
	SSERegister	xmmSrc, xmmDst ;
	switch ( code )
	{
	case	fcodeFabs:
		xmmSrc = WriteRealizeDataRegister( regSrc, regTypeFloat64, true ) ;
		xmmDst = WriteRealizeDataRegister( regDst, regTypeFloat64, false ) ;
		//
		WriteSSERegMemOperand
			( sseop_MOVSD_LOAD, 3, 
				xmmDst, x86_Nothing,
				(INT_PTR) GetConstantPair7FFFFFFFFFFFFFFF() ) ;
		WriteSSERegRegOperand
			( sseop_ANDPD, 3, xmmDst, xmmSrc ) ;
		//
		SetDataRegisterModified( xmmDst ) ;
		UnlockDataRegister( xmmSrc, regTypeFloat64 ) ;
		UnlockDataRegister( xmmDst, regTypeFloat64 ) ;
		return ;

	case	fcodeSqrt:
		xmmSrc = WriteRealizeDataRegister( regSrc, regTypeFloat64, true ) ;
		xmmDst = WriteRealizeDataRegister( regDst, regTypeFloat64, false ) ;
		//
		WriteSSERegRegOperand
			( sseop_SQRTPD, 3, xmmDst, xmmSrc ) ;
		//
		SetDataRegisterModified( xmmDst ) ;
		UnlockDataRegister( xmmSrc, regTypeFloat64 ) ;
		UnlockDataRegister( xmmDst, regTypeFloat64 ) ;
		return ;

	case	fcodeRound:
		FlushRegister( regSrc ) ;
		ResetRegister( regDst ) ;
		//
		WriteX86RegMemOperand
			( x86op1_FLD_FP64, 1, x86op2nd_FLD,
				true, x86_EBX, Context::OffsetOfReg(regSrc) ) ;
		WriteX86RegMemOperand
			( x86op1_FIST_FP64, 1, x86op2nd_FISTP64,
				true, x86_EBX, Context::OffsetOfReg(regDst) ) ;
		//
		m_rdtSakura[regDst] = regTypeQWord ;
		return ;

	default:
		break ;
	}
	X86GenericAssembler::write_float_extension( code, regDst, regSrc ) ;
}

// 64bit SIMD
//////////////////////////////////////////////////////////////////////////////
void X86SSE2Assembler::write_simd64_extension
	( int code, int regDst, int regSrc, bool fPair )
{
	RegisterDataType	regType = fPair ? regTypeDQWord : regTypeQWord ;
	RegisterDataType	regDstType = regType ;
	RegisterDataType	regSrcType = regType ;
	bool				fPackDst = false ;
	int					countPair = 1 ;
	int					stepSrcReg = 1 ;
	switch ( code )
	{
	case	simdPsrlw:
	case	simdPsrld:
	case	simdPsraw:
	case	simdPsrad:
	case	simdPsllw:
	case	simdPslld:
		regSrcType = regTypeQWord ;
		stepSrcReg = 0 ;
		break ;
	case	simdPunpacklbw:
	case	simdPunpacklwd:
	case	simdPunpackldq:
		regDstType = regTypeQWord ;
		regSrcType = regTypeQWord ;
		countPair = fPair ? 2 : 1 ;
		break ;
	case	simdPcvtswsb:
	case	simdPcvtswub:
	case	simdPcvtsdsw:
		regDstType = regTypeQWord ;
		regSrcType = regTypeQWord ;
		fPackDst = (((regDst + 1) == regSrc) & !(regDst & 0x01)) ;
		countPair = fPair ? 2 : 1 ;
		break ;
	}
	//
	for ( int i = 0; i < countPair; i ++ )
	{
		SSERegister	xmmDst, xmmSrc, xmmTemp ;
		bool	fTempSrc = false ;
		if ( fPackDst )
		{
			xmmSrc = XMM_Nothing ;
		}
		else if ( (regSrc == regDst) && (regSrcType != regDstType) )
		{
			SSERegister	xmmSrcTemp =
				WriteRealizeDataRegister
						( regSrc + i * stepSrcReg, regSrcType, true ) ;
			xmmSrc = AllocateDataRegister( regSrcType ) ;
			WriteSSERegRegOperand
				( sseop_MOVDQA_LOAD, 3, xmmSrc, xmmSrcTemp ) ;
			UnlockDataRegister( xmmSrcTemp, regSrcType ) ;
			fTempSrc = true ;
		}
		else
		{
			xmmSrc = WriteRealizeDataRegister
						( regSrc + i * stepSrcReg, regSrcType, true ) ;
		}
		xmmDst = WriteRealizeDataRegister
			( regDst + i, (fPackDst ? regTypeDQWord : regDstType), true ) ;
		switch ( code )
		{
		case	simdPaddub:
			WriteSSERegRegOperand( sseop_PADDUSB, 3, xmmDst, xmmSrc ) ;
			break ;
		case	simdPaddsb:
			WriteSSERegRegOperand( sseop_PADDSB, 3, xmmDst, xmmSrc ) ;
			break ;
		case	simdPaddb:
			WriteSSERegRegOperand( sseop_PADDB, 3, xmmDst, xmmSrc ) ;
			break ;
		case	simdPadduw:
			WriteSSERegRegOperand( sseop_PADDUSW, 3, xmmDst, xmmSrc ) ;
			break ;
		case	simdPaddsw:
			WriteSSERegRegOperand( sseop_PADDSW, 3, xmmDst, xmmSrc ) ;
			break ;
		case	simdPaddw:
			WriteSSERegRegOperand( sseop_PADDW, 3, xmmDst, xmmSrc ) ;
			break ;
		case	simdPaddd:
			WriteSSERegRegOperand( sseop_PADDD, 3, xmmDst, xmmSrc ) ;
			break ;

		case	simdPsubub:
			WriteSSERegRegOperand( sseop_PSUBUSB, 3, xmmDst, xmmSrc ) ;
			break ;
		case	simdPsubsb:
			WriteSSERegRegOperand( sseop_PSUBSB, 3, xmmDst, xmmSrc ) ;
			break ;
		case	simdPsubb:
			WriteSSERegRegOperand( sseop_PSUBB, 3, xmmDst, xmmSrc ) ;
			break ;
		case	simdPsubuw:
			WriteSSERegRegOperand( sseop_PSUBUSW, 3, xmmDst, xmmSrc ) ;
			break ;
		case	simdPsubsw:
			WriteSSERegRegOperand( sseop_PSUBSW, 3, xmmDst, xmmSrc ) ;
			break ;
		case	simdPsubw:
			WriteSSERegRegOperand( sseop_PSUBW, 3, xmmDst, xmmSrc ) ;
			break ;
		case	simdPsubd:
			WriteSSERegRegOperand( sseop_PSUBD, 3, xmmDst, xmmSrc ) ;
			break ;

		case	simdPsrlw:
			xmmTemp = AllocateDataRegister( regDstType ) ;
			WriteSSERegMemOperand
				( sseop_MOVD_LOAD, 3, xmmTemp,
					x86_Nothing, (INT_PTR) GetConstantDQWord0F() ) ;
			WriteSSERegRegOperand( sseop_PAND, 3, xmmTemp, xmmSrc ) ;
			WriteSSERegRegOperand( sseop_PSRLW, 3, xmmDst, xmmTemp ) ;
			FreeDataRegister( xmmTemp, regDstType ) ;
			break ;
		case	simdPsrld:
			xmmTemp = AllocateDataRegister( regDstType ) ;
			WriteSSERegMemOperand
				( sseop_MOVD_LOAD, 3, xmmTemp,
					x86_Nothing, (INT_PTR) GetConstantDQWord1F() ) ;
			WriteSSERegRegOperand( sseop_PAND, 3, xmmTemp, xmmSrc ) ;
			WriteSSERegRegOperand( sseop_PSRLD, 3, xmmDst, xmmTemp ) ;
			FreeDataRegister( xmmTemp, regDstType ) ;
			break ;
		case	simdPsraw:
			xmmTemp = AllocateDataRegister( regDstType ) ;
			WriteSSERegMemOperand
				( sseop_MOVD_LOAD, 3, xmmTemp,
					x86_Nothing, (INT_PTR) GetConstantDQWord0F() ) ;
			WriteSSERegRegOperand( sseop_PAND, 3, xmmTemp, xmmSrc ) ;
			WriteSSERegRegOperand( sseop_PSRAW, 3, xmmDst, xmmTemp ) ;
			FreeDataRegister( xmmTemp, regDstType ) ;
			break ;
		case	simdPsrad:
			xmmTemp = AllocateDataRegister( regDstType ) ;
			WriteSSERegMemOperand
				( sseop_MOVD_LOAD, 3, xmmTemp,
					x86_Nothing, (INT_PTR) GetConstantDQWord1F() ) ;
			WriteSSERegRegOperand( sseop_PAND, 3, xmmTemp, xmmSrc ) ;
			WriteSSERegRegOperand( sseop_PSRAD, 3, xmmDst, xmmTemp ) ;
			FreeDataRegister( xmmTemp, regDstType ) ;
			break ;
		case	simdPsllw:
			xmmTemp = AllocateDataRegister( regDstType ) ;
			WriteSSERegMemOperand
				( sseop_MOVD_LOAD, 3, xmmTemp,
					x86_Nothing, (INT_PTR) GetConstantDQWord0F() ) ;
			WriteSSERegRegOperand( sseop_PAND, 3, xmmTemp, xmmSrc ) ;
			WriteSSERegRegOperand( sseop_PSLLW, 3, xmmDst, xmmTemp ) ;
			FreeDataRegister( xmmTemp, regDstType ) ;
			break ;
		case	simdPslld:
			xmmTemp = AllocateDataRegister( regDstType ) ;
			WriteSSERegMemOperand
				( sseop_MOVD_LOAD, 3, xmmTemp,
					x86_Nothing, (INT_PTR) GetConstantDQWord1F() ) ;
			WriteSSERegRegOperand( sseop_PAND, 3, xmmTemp, xmmSrc ) ;
			WriteSSERegRegOperand( sseop_PSLLD, 3, xmmDst, xmmTemp ) ;
			FreeDataRegister( xmmTemp, regDstType ) ;
			break ;

		case	simdPcmpnesb:
			xmmTemp = AllocateDataRegister( regDstType ) ;
			WriteSSERegRegOperand( sseop_PCMPEQB, 3, xmmTemp, xmmTemp ) ;
			WriteSSERegRegOperand( sseop_PCMPEQB, 3, xmmDst, xmmSrc ) ;
			WriteSSERegRegOperand( sseop_PXOR, 3, xmmDst, xmmTemp ) ;
			FreeDataRegister( xmmTemp, regDstType ) ;
			break ;
		case	simdPcmpnesw:
			xmmTemp = AllocateDataRegister( regDstType ) ;
			WriteSSERegRegOperand( sseop_PCMPEQW, 3, xmmTemp, xmmTemp ) ;
			WriteSSERegRegOperand( sseop_PCMPEQW, 3, xmmDst, xmmSrc ) ;
			WriteSSERegRegOperand( sseop_PXOR, 3, xmmDst, xmmTemp ) ;
			FreeDataRegister( xmmTemp, regDstType ) ;
			break ;
		case	simdPcmpnesd:
			xmmTemp = AllocateDataRegister( regDstType ) ;
			WriteSSERegRegOperand( sseop_PCMPEQD, 3, xmmTemp, xmmTemp ) ;
			WriteSSERegRegOperand( sseop_PCMPEQD, 3, xmmDst, xmmSrc ) ;
			WriteSSERegRegOperand( sseop_PXOR, 3, xmmDst, xmmTemp ) ;
			FreeDataRegister( xmmTemp, regDstType ) ;
			break ;

		case	simdPcmpeqsb:
			WriteSSERegRegOperand( sseop_PCMPEQB, 3, xmmDst, xmmSrc ) ;
			break ;
		case	simdPcmpeqsw:
			WriteSSERegRegOperand( sseop_PCMPEQW, 3, xmmDst, xmmSrc ) ;
			break ;
		case	simdPcmpeqsd:
			WriteSSERegRegOperand( sseop_PCMPEQD, 3, xmmDst, xmmSrc ) ;
			break ;

		case	simdPcmpltsb:
			xmmTemp = AllocateDataRegister( regDstType ) ;
			WriteSSERegRegOperand( sseop_MOVDQA_LOAD, 3, xmmTemp, xmmSrc ) ;
			WriteSSERegRegOperand( sseop_PCMPGTB, 3, xmmTemp, xmmDst ) ;
			WriteSSERegRegOperand( sseop_MOVDQA_LOAD, 3, xmmDst, xmmTemp ) ;
			FreeDataRegister( xmmTemp, regDstType ) ;
			break ;
		case	simdPcmpltsw:
			xmmTemp = AllocateDataRegister( regDstType ) ;
			WriteSSERegRegOperand( sseop_MOVDQA_LOAD, 3, xmmTemp, xmmSrc ) ;
			WriteSSERegRegOperand( sseop_PCMPGTW, 3, xmmTemp, xmmDst ) ;
			WriteSSERegRegOperand( sseop_MOVDQA_LOAD, 3, xmmDst, xmmTemp ) ;
			FreeDataRegister( xmmTemp, regDstType ) ;
			break ;
		case	simdPcmpltsd:
			xmmTemp = AllocateDataRegister( regDstType ) ;
			WriteSSERegRegOperand( sseop_MOVDQA_LOAD, 3, xmmTemp, xmmSrc ) ;
			WriteSSERegRegOperand( sseop_PCMPGTD, 3, xmmTemp, xmmDst ) ;
			WriteSSERegRegOperand( sseop_MOVDQA_LOAD, 3, xmmDst, xmmTemp ) ;
			FreeDataRegister( xmmTemp, regDstType ) ;
			break ;

		case	simdPcmplesb:
			xmmTemp = AllocateDataRegister( regDstType ) ;
			WriteSSERegRegOperand( sseop_PCMPEQB, 3, xmmTemp, xmmTemp ) ;
			WriteSSERegRegOperand( sseop_PCMPGTB, 3, xmmDst, xmmSrc ) ;
			WriteSSERegRegOperand( sseop_PXOR, 3, xmmDst, xmmTemp ) ;
			FreeDataRegister( xmmTemp, regDstType ) ;
			break ;
		case	simdPcmplesw:
			xmmTemp = AllocateDataRegister( regDstType ) ;
			WriteSSERegRegOperand( sseop_PCMPEQW, 3, xmmTemp, xmmTemp ) ;
			WriteSSERegRegOperand( sseop_PCMPGTW, 3, xmmDst, xmmSrc ) ;
			WriteSSERegRegOperand( sseop_PXOR, 3, xmmDst, xmmTemp ) ;
			FreeDataRegister( xmmTemp, regDstType ) ;
			break ;
		case	simdPcmplesd:
			xmmTemp = AllocateDataRegister( regDstType ) ;
			WriteSSERegRegOperand( sseop_PCMPEQW, 3, xmmTemp, xmmTemp ) ;
			WriteSSERegRegOperand( sseop_PCMPGTD, 3, xmmDst, xmmSrc ) ;
			WriteSSERegRegOperand( sseop_PXOR, 3, xmmDst, xmmTemp ) ;
			FreeDataRegister( xmmTemp, regDstType ) ;
			break ;

		case	simdPcmpgtsb:
			WriteSSERegRegOperand( sseop_PCMPGTB, 3, xmmDst, xmmSrc ) ;
			break ;
		case	simdPcmpgtsw:
			WriteSSERegRegOperand( sseop_PCMPGTW, 3, xmmDst, xmmSrc ) ;
			break ;
		case	simdPcmpgtsd:
			WriteSSERegRegOperand( sseop_PCMPGTD, 3, xmmDst, xmmSrc ) ;
			break ;

		case	simdPcmpgesb:
			xmmTemp = AllocateDataRegister( regDstType ) ;
			WriteSSERegRegOperand( sseop_MOVDQA_LOAD, 3, xmmTemp, xmmSrc ) ;
			WriteSSERegRegOperand( sseop_PCMPGTB, 3, xmmTemp, xmmDst ) ;
			WriteSSERegRegOperand( sseop_PCMPEQB, 3, xmmDst, xmmDst ) ;
			WriteSSERegRegOperand( sseop_PXOR, 3, xmmDst, xmmTemp ) ;
			FreeDataRegister( xmmTemp, regDstType ) ;
			break ;
		case	simdPcmpgesw:
			xmmTemp = AllocateDataRegister( regDstType ) ;
			WriteSSERegRegOperand( sseop_MOVDQA_LOAD, 3, xmmTemp, xmmSrc ) ;
			WriteSSERegRegOperand( sseop_PCMPGTW, 3, xmmTemp, xmmDst ) ;
			WriteSSERegRegOperand( sseop_PCMPEQW, 3, xmmDst, xmmDst ) ;
			WriteSSERegRegOperand( sseop_PXOR, 3, xmmDst, xmmTemp ) ;
			FreeDataRegister( xmmTemp, regDstType ) ;
			break ;
		case	simdPcmpgesd:
			xmmTemp = AllocateDataRegister( regDstType ) ;
			WriteSSERegRegOperand( sseop_MOVDQA_LOAD, 3, xmmTemp, xmmSrc ) ;
			WriteSSERegRegOperand( sseop_PCMPGTD, 3, xmmTemp, xmmDst ) ;
			WriteSSERegRegOperand( sseop_PCMPEQD, 3, xmmDst, xmmDst ) ;
			WriteSSERegRegOperand( sseop_PXOR, 3, xmmDst, xmmTemp ) ;
			FreeDataRegister( xmmTemp, regDstType ) ;
			break ;

		case	simdPmullw:
			WriteSSERegRegOperand( sseop_PMULLW, 3, xmmDst, xmmSrc ) ;
			break ;
		case	simdPmulhsw:
			WriteSSERegRegOperand( sseop_PMULHW, 3, xmmDst, xmmSrc ) ;
			break ;
		case	simdPmulhuw:
			WriteSSERegRegOperand( sseop_PMULHUW, 3, xmmDst, xmmSrc ) ;
			break ;
		case	simdPmaddwd:
			WriteSSERegRegOperand( sseop_PMADDWD, 3, xmmDst, xmmSrc ) ;
			break ;

		case	simdPunpacklbw:
			WriteSSERegRegOperand( sseop_PUNPCKLBW, 3, xmmDst, xmmSrc ) ;
			break ;
		case	simdPunpacklwd:
			WriteSSERegRegOperand( sseop_PUNPCKLWD, 3, xmmDst, xmmSrc ) ;
			break ;
		case	simdPunpackldq:
			WriteSSERegRegOperand( sseop_PUNPCKLDQ, 3, xmmDst, xmmSrc ) ;
			break ;

		case	simdPcvtswsb:
			if ( !fPackDst )
			{
				WriteSSERegRegOperand
					( sseop_PUNPCKLQDQ, 3, xmmDst, xmmSrc ) ;
			}
			WriteSSERegRegOperand( sseop_PACKSSWB, 3, xmmDst, xmmDst ) ;
			break ;
		case	simdPcvtswub:
			if ( !fPackDst )
			{
				WriteSSERegRegOperand
					( sseop_PUNPCKLQDQ, 3, xmmDst, xmmSrc ) ;
			}
			WriteSSERegRegOperand( sseop_PACKUSWB, 3, xmmDst, xmmDst ) ;
			break ;
		case	simdPcvtsdsw:
			if ( !fPackDst )
			{
				WriteSSERegRegOperand
					( sseop_PUNPCKLQDQ, 3, xmmDst, xmmSrc ) ;
			}
			WriteSSERegRegOperand( sseop_PACKSSDW, 3, xmmDst, xmmDst ) ;
			break ;

		default:
			ESLTrace( "bad instruction packed 64bit SIMD %02X %02X\n",
						codeSIMD64Extension2Op, code ) ;
			WriteToAtomicOrExceptionMask( exceptionBadInstruction ) ;
			break ;
		}
		SetDataRegisterModified( xmmDst ) ;
		UnlockDataRegister( xmmDst, regDstType ) ;
		//
		if ( xmmSrc != XMM_Nothing )
		{
			if ( fTempSrc )
			{
				FreeDataRegister( xmmSrc, regSrcType ) ;
			}
			else
			{
				UnlockDataRegister( xmmSrc, regSrcType ) ;
			}
		}
	}
}

void X86SSE2Assembler::write_simd64_imm_extension
	( int code, int regDst, int regSrc, int imm8, bool fPair )
{
	RegisterDataType
		regType = fPair ? regTypeDQWord : regTypeQWord ;
	int	countPair = 1 ;
	if ( code == simdPshufwImm8 )
	{
		regType = regTypeQWord ;
		countPair = fPair ? 2 : 1 ;
	}
	for ( int i = 0; i < countPair; i ++ )
	{
		SSERegister	xmmSrc =
			WriteRealizeDataRegister( regSrc + i, regType, true ) ;
		SSERegister	xmmDst =
			WriteRealizeDataRegister( regDst + i, regType, false ) ;
		if ( regSrc != regDst )
		{
			WriteSSERegRegOperand
				( sseop_MOVDQA_LOAD, 3, xmmDst, xmmSrc ) ;
		}
		UnlockDataRegister( xmmSrc, regType ) ;
		//
		switch ( code )
		{
		case	simdPsrlwImm8:
			WriteSSERegRegImm8Operand
				( sseop_PSHIFTW_IMM, 3,
					mmxop2nd_SRL, xmmDst, (imm8 & 0x0F) ) ;
			break ;
		case	simdPsrldImm8:
			WriteSSERegRegImm8Operand
				( sseop_PSHIFTD_IMM, 3,
					mmxop2nd_SRL, xmmDst, (imm8 & 0x1F) ) ;
			break ;
		case	simdPsrawImm8:
			WriteSSERegRegImm8Operand
				( sseop_PSHIFTW_IMM, 3,
					mmxop2nd_SRA, xmmDst, (imm8 & 0x0F) ) ;
			break ;
		case	simdPsradImm8:
			WriteSSERegRegImm8Operand
				( sseop_PSHIFTD_IMM, 3,
					mmxop2nd_SRA, xmmDst, (imm8 & 0x1F) ) ;
			break ;
		case	simdPsllwImm8:
			WriteSSERegRegImm8Operand
				( sseop_PSHIFTW_IMM, 3,
					mmxop2nd_SLL, xmmDst, (imm8 & 0x0F) ) ;
			break ;
		case	simdPslldImm8:
			WriteSSERegRegImm8Operand
				( sseop_PSHIFTD_IMM, 3,
					mmxop2nd_SLL, xmmDst, (imm8 & 0x1F) ) ;
			break ;

		case	simdPshufwImm8:
			WriteSSERegRegOperand
				( sseop_PSHUFLW, 3, xmmDst, xmmDst, imm8, 1 ) ;
			break ;

		default:
			ESLTrace( "bad instruction packed 64bit SIMD %02X %02X\n",
						codeSIMD64Extension3Op, code ) ;
			WriteToAtomicOrExceptionMask( exceptionBadInstruction ) ;
			break ;
		}
		SetDataRegisterModified( xmmDst ) ;
		UnlockDataRegister( xmmDst, regType ) ;
	}
}

// 128bit SIMD
//////////////////////////////////////////////////////////////////////////////
void X86SSE2Assembler::write_simd128_extension
	( int code, int regDst, int regSrc )
{
	//
	// 入出力レジスタ情報整理
	//
	RegisterDataType	regDstType = regTypeQFloat32 ;
	RegisterDataType	regSrcType = regTypeQFloat32 ;
	bool				fLoadDstValue = true ;
	switch ( code )
	{
	case	simdVmove:
		fLoadDstValue = false ;
		if ( regDst == regSrc )
		{
			m_rdtSakura[regDst & ~0x01] = regTypeQFloat32 ;
			m_rdtSakura[regDst | 0x01] = regTypeQFloat32 ;
			return ;
		}
		break ;

	case	simdFsqrt:
	case	simdFrcp:
	case	simdFrsqrt:
	case	simdFabs:
	case	simdVsqrt:
	case	simdVrcp:
	case	simdVrsqrt:
	case	simdVabs:
		fLoadDstValue = false ;
		break ;

	case	simdDcvtd2f:
		fLoadDstValue = false ;
		regSrcType = regTypeDFloat64 ;
		break ;

	case	simdDcvtf2d:
		fLoadDstValue = false ;
		regDstType = regTypeDFloat64 ;
		break ;

	case	simdDcvtf2i:
	case	simdVcvtf2w:
	case	simdVcvtf2i:
		fLoadDstValue = false ;
		regDstType = regTypeDQWord ;
		break ;

	case	simdDcvti2f:
	case	simdVcvtw2f:
	case	simdVcvti2f:
		fLoadDstValue = false ;
		regSrcType = regTypeDQWord ;
		break ;
	}
	//
	// 物理レジスタ割り当て
	//
	SSERegister	xmmDst, xmmSrc, xmmTemp ;
	bool	fTempSrc = false ;
	if ( (regSrc == regDst) && (regSrcType != regDstType) )
	{
		SSERegister	xmmSrcTemp =
			WriteRealizeDataRegister( regSrc, regSrcType, true ) ;
		xmmSrc = AllocateDataRegister( regSrcType ) ;
		switch ( regSrcType )
		{
		case	regTypeQWord:
			WriteSSERegRegOperand
				( sseop_MOVQ_LOAD, 3, xmmSrc, xmmSrcTemp ) ;
			break ;
		default:
			WriteSSERegRegOperand
				( sseop_MOVAPS_LOAD, 2, xmmSrc, xmmSrcTemp ) ;
		}
		UnlockDataRegister( xmmSrcTemp, regSrcType ) ;
		fTempSrc = true ;
	}
	else
	{
		xmmSrc = WriteRealizeDataRegister( regSrc, regSrcType, true ) ;
	}
	xmmDst = WriteRealizeDataRegister( regDst, regDstType, fLoadDstValue ) ;
	//
	// 処理コード生成
	//
	switch ( code )
	{
	case	simdFadd:
		WriteSSERegRegOperand( sseop_ADDSS, 3, xmmDst, xmmSrc ) ;
		break ;
	case	simdFsub:
		WriteSSERegRegOperand( sseop_SUBSS, 3, xmmDst, xmmSrc ) ;
		break ;
	case	simdFmul:
		WriteSSERegRegOperand( sseop_MULSS, 3, xmmDst, xmmSrc ) ;
		break ;
	case	simdFdiv:
		WriteSSERegRegOperand( sseop_DIVSS, 3, xmmDst, xmmSrc ) ;
		break ;
	case	simdFsqrt:
		WriteSSERegRegOperand( sseop_SQRTSS, 3, xmmDst, xmmSrc ) ;
		break ;
	case	simdFrcp:
		WriteSSERegRegOperand( sseop_RCPSS, 3, xmmDst, xmmSrc ) ;
		break ;
	case	simdFrsqrt:
		WriteSSERegRegOperand( sseop_RSQRTSS, 3, xmmDst, xmmSrc ) ;
		break ;
	case	simdFabs:
		if ( xmmDst != xmmSrc )
		{
			WriteSSERegMemOperand
				( sseop_MOVAPS_LOAD, 2, xmmDst,
					x86_Nothing, (INT_PTR) GetConstantLow32NoSignMask() ) ;
			WriteSSERegRegOperand( sseop_ANDPS, 2, xmmDst, xmmSrc ) ;
		}
		else
		{
			WriteSSERegMemOperand
				( sseop_ANDPS, 2, xmmDst,
					x86_Nothing, (INT_PTR) GetConstantLow32NoSignMask() ) ;
		}
		break ;
	case	simdFmax:
		WriteSSERegRegOperand( sseop_MAXSS, 3, xmmDst, xmmSrc ) ;
		break ;
	case	simdFmin:
		WriteSSERegRegOperand( sseop_MINSS, 3, xmmDst, xmmSrc ) ;
		break ;

	case	simdVadd:
		WriteSSERegRegOperand( sseop_ADDPS, 2, xmmDst, xmmSrc ) ;
		break ;
	case	simdVsub:
		WriteSSERegRegOperand( sseop_SUBPS, 2, xmmDst, xmmSrc ) ;
		break ;
	case	simdVmul:
		WriteSSERegRegOperand( sseop_MULPS, 2, xmmDst, xmmSrc ) ;
		break ;
	case	simdVdiv:
		WriteSSERegRegOperand( sseop_DIVPS, 2, xmmDst, xmmSrc ) ;
		break ;
	case	simdVsqrt:
		WriteSSERegRegOperand( sseop_SQRTPS, 2, xmmDst, xmmSrc ) ;
		break ;
	case	simdVrcp:
		WriteSSERegRegOperand( sseop_RCPPS, 2, xmmDst, xmmSrc ) ;
		break ;
	case	simdVrsqrt:
		WriteSSERegRegOperand( sseop_RSQRTPS, 2, xmmDst, xmmSrc ) ;
		break ;
	case	simdVabs:
		if ( xmmDst != xmmSrc )
		{
			WriteSSERegMemOperand
				( sseop_MOVAPS_LOAD, 2, xmmDst,
					x86_Nothing, (INT_PTR) GetConstantPack32NoSignMask() ) ;
			WriteSSERegRegOperand( sseop_ANDPS, 2, xmmDst, xmmSrc ) ;
		}
		else
		{
			WriteSSERegMemOperand
				( sseop_ANDPS, 2, xmmDst,
					x86_Nothing, (INT_PTR) GetConstantPack32NoSignMask() ) ;
		}
		break ;
	case	simdVmax:
		WriteSSERegRegOperand( sseop_MAXPS, 2, xmmDst, xmmSrc ) ;
		break ;
	case	simdVmin:
		WriteSSERegRegOperand( sseop_MINPS, 2, xmmDst, xmmSrc ) ;
		break ;

	case	simdVcmpne:
		WriteSSERegRegOperand
			( sseop_CMPPS, 2, xmmDst, xmmSrc, sseimm_CMP_NEQ, 1 ) ;
		break ;
	case	simdVcmpeq:
		WriteSSERegRegOperand
			( sseop_CMPPS, 2, xmmDst, xmmSrc, sseimm_CMP_EQ, 1 ) ;
		break ;
	case	simdVcmplt:
		WriteSSERegRegOperand
			( sseop_CMPPS, 2, xmmDst, xmmSrc, sseimm_CMP_LT, 1 ) ;
		break ;
	case	simdVcmple:
		WriteSSERegRegOperand
			( sseop_CMPPS, 2, xmmDst, xmmSrc, sseimm_CMP_LE, 1 ) ;
		break ;
	case	simdVcmpgt:
		WriteSSERegRegOperand
			( sseop_CMPPS, 2, xmmDst, xmmSrc, sseimm_CMP_GT, 1 ) ;
		break ;
	case	simdVcmpge:
		WriteSSERegRegOperand
			( sseop_CMPPS, 2, xmmDst, xmmSrc, sseimm_CMP_GE, 1 ) ;
		break ;

	case	simdVmove:
		WriteSSERegRegOperand( sseop_MOVAPS_LOAD, 2, xmmDst, xmmSrc ) ;
		break ;
	case	simdVand:
		WriteSSERegRegOperand( sseop_ANDPS, 2, xmmDst, xmmSrc ) ;
		break ;
	case	simdVor:
		WriteSSERegRegOperand( sseop_ORPS, 2, xmmDst, xmmSrc ) ;
		break ;
	case	simdVxor:
		WriteSSERegRegOperand( sseop_XORPS, 2, xmmDst, xmmSrc ) ;
		break ;

	case	simdDcvtf2i:
		xmmTemp = AllocateDataRegister( regTypeQFloat32 ) ;
		//
		WriteSSERegRegOperand( sseop_XORPS, 2, xmmTemp, xmmTemp ) ;
		WriteSSERegRegOperand( sseop_CVT_PS2DQ, 3, xmmDst, xmmSrc ) ;
		WriteSSERegRegOperand( sseop_MOVLHPS, 2, xmmDst, xmmTemp ) ;
		//
		FreeDataRegister( xmmTemp, regTypeQFloat32 ) ;
		break ;
	case	simdDcvti2f:
		xmmTemp = AllocateDataRegister( regTypeQFloat32 ) ;
		//
		WriteSSERegRegOperand( sseop_XORPS, 2, xmmTemp, xmmTemp ) ;
		WriteSSERegRegOperand( sseop_CVT_DQ2PS, 2, xmmDst, xmmSrc ) ;
		WriteSSERegRegOperand( sseop_MOVLHPS, 2, xmmDst, xmmTemp ) ;
		//
		FreeDataRegister( xmmTemp, regTypeQFloat32 ) ;
		break ;
	case	simdDcvtd2f:
		WriteSSERegRegOperand( sseop_CVT_PD2PS, 3, xmmDst, xmmSrc ) ;
		break ;
	case	simdDcvtf2d:
		WriteSSERegRegOperand( sseop_CVT_PS2PD, 2, xmmDst, xmmSrc ) ;
		break ;

	case	simdVcvtf2w:
		xmmTemp = AllocateDataRegister( regTypeDQWord ) ;
		//
		WriteSSERegRegOperand( sseop_CVT_PS2DQ, 3, xmmDst, xmmSrc ) ;
		WriteSSERegRegOperand( sseop_PXOR, 3, xmmTemp, xmmTemp ) ;
		WriteSSERegRegOperand( sseop_PACKSSDW, 3, xmmDst, xmmTemp ) ;
		//
		FreeDataRegister( xmmTemp, regTypeDQWord ) ;
		break ;
	case	simdVcvtw2f:
		xmmTemp = AllocateDataRegister( regTypeDQWord ) ;
		//
		WriteSSERegRegOperand( sseop_PUNPCKLWD, 3, xmmTemp, xmmSrc ) ;
		WriteSSERegRegImm8Operand
			( sseop_PSHIFTD_IMM, 3, mmxop2nd_SRA, xmmTemp, 16 ) ;
		WriteSSERegRegOperand( sseop_CVT_DQ2PS, 2, xmmDst, xmmTemp ) ;
		//
		FreeDataRegister( xmmTemp, regTypeDQWord ) ;
		break ;
	case	simdVcvtf2i:
		WriteSSERegRegOperand( sseop_CVT_PS2DQ, 3, xmmDst, xmmSrc ) ;
		break ;
	case	simdVcvti2f:
		WriteSSERegRegOperand( sseop_CVT_DQ2PS, 2, xmmDst, xmmSrc ) ;
		break ;

	default:
		ESLTrace( "bad instruction packed 128bit SIMD %02X %02X\n",
					codeSIMD128Extension2Op, code ) ;
		WriteToAtomicOrExceptionMask( exceptionBadInstruction ) ;
		break ;
	}
	//
	SetDataRegisterModified( xmmDst ) ;
	UnlockDataRegister( xmmDst, regDstType ) ;
	if ( fTempSrc )
	{
		FreeDataRegister( xmmSrc, regSrcType ) ;
	}
	else
	{
		UnlockDataRegister( xmmSrc, regSrcType ) ;
	}
}

void X86SSE2Assembler::write_simd128_imm_extension
	( int code, int regDst, int regSrc, int imm8 )
{
	SSERegister	xmmDst, xmmSrc, xmmMask, xmmTemp ;
	switch ( code )
	{
	case	simdVmaskmove:
		xmmMask = WriteRealizeDataRegister( imm8, regTypeQFloat32 ) ;
		xmmSrc = WriteRealizeDataRegister( regSrc, regTypeQFloat32 ) ;
		xmmDst = WriteRealizeDataRegister( regDst, regTypeQFloat32 ) ;
		xmmTemp = AllocateDataRegister( regTypeQFloat32 ) ;
		WriteBackDataRegister( xmmMask ) ;
		//
		WriteSSERegRegOperand
			( sseop_MOVAPS_LOAD, 2, xmmTemp, xmmMask ) ;
		WriteSSERegRegOperand
			( sseop_ANDPS, 2, xmmMask, xmmSrc ) ;
		WriteSSERegRegOperand
			( sseop_ANDNPS, 2, xmmTemp, xmmDst ) ;
		WriteSSERegRegOperand
			( sseop_MOVAPS_LOAD, 2, xmmDst, xmmMask ) ;
		WriteSSERegRegOperand
			( sseop_ORPS, 2, xmmDst, xmmTemp ) ;
		//
		SetDataRegisterModified( xmmDst ) ;
		FreeDataRegister( xmmTemp, regTypeQFloat32 ) ;
		UnlockDataRegister( xmmDst, regTypeQFloat32 ) ;
		UnlockDataRegister( xmmSrc, regTypeQFloat32 ) ;
		FreeDataRegister( xmmMask, regTypeQFloat32 ) ;
		break ;

	case	simdVshuf32:
		if ( m_rdtSakura[regDst] == regTypeDQWord )
		{
			xmmSrc = WriteRealizeDataRegister( regSrc, regTypeDQWord ) ;
			xmmDst = WriteRealizeDataRegister( regDst, regTypeDQWord, false ) ;
			//
			WriteSSERegRegOperand
				( sseop_PSHUFD, 3, xmmDst, xmmSrc, imm8, 1 ) ;
			//
			SetDataRegisterModified( xmmDst ) ;
			UnlockDataRegister( xmmDst, regTypeDQWord ) ;
			UnlockDataRegister( xmmSrc, regTypeDQWord ) ;
		}
		else
		{
			xmmSrc = WriteRealizeDataRegister( regSrc, regTypeQFloat32 ) ;
			xmmDst = WriteRealizeDataRegister( regDst, regTypeQFloat32, false ) ;
			//
			if ( xmmSrc != xmmDst )
			{
				WriteSSERegRegOperand
					( sseop_MOVAPS_LOAD, 2, xmmDst, xmmSrc ) ;
			}
			WriteSSERegRegOperand
				( sseop_SHUFPS, 2, xmmDst, xmmDst, imm8, 1 ) ;
			//
			SetDataRegisterModified( xmmDst ) ;
			UnlockDataRegister( xmmDst, regTypeQFloat32 ) ;
			UnlockDataRegister( xmmSrc, regTypeQFloat32 ) ;
		}
		break ;

	default:
		ESLTrace( "bad instruction packed 128bit SIMD %02X %02X\n",
					codeSIMD128Extension3Op, code ) ;
		WriteToAtomicOrExceptionMask( exceptionBadInstruction ) ;
		break ;
	}
}

// 64bit x 64bit -> 64bit 乗算命令生成
//////////////////////////////////////////////////////////////////////////////
void X86SSE2Assembler::write_mul_int64xint64
	( X86SSE2Assembler::SSERegister xmmDst,
		X86SSE2Assembler::SSERegister xmmSrc,
		X86SSE2Assembler::SSERegister xmmTemp1,
		X86SSE2Assembler::SSERegister xmmTemp2 )
{
	//
	// xmmTemp1 <- high[dst.h32 * src.l32] : low[dst.l32 * src.h32]
	// xmmDst <- high[dst.h32 * src.h32] : low[dst.l32 * src.l32]
	//
	WriteSSERegRegOperand( sseop_PSHUFD, 3, xmmTemp1, xmmSrc, 0xE1, 1 ) ;
	WriteSSERegRegOperand( sseop_PXOR, 3, xmmTemp2, xmmTemp2 ) ;
	WriteSSERegRegOperand( sseop_PUNPCKLDQ, 3, xmmDst, xmmTemp2 ) ;
	WriteSSERegRegOperand( sseop_PUNPCKLDQ, 3, xmmTemp1, xmmTemp2 ) ;
	WriteSSERegRegOperand( sseop_PUNPCKLDQ, 3, xmmTemp2, xmmSrc ) ;
	//
	WriteSSERegRegOperand( sseop_PMULUDQ, 3, xmmTemp1, xmmDst ) ;
	//
	WriteSSERegRegImm8Operand
			( sseop_PSHIFTQ_IMM, 3, mmxop2nd_SRL, xmmTemp2, 32 ) ;
	WriteSSERegRegOperand( sseop_PMULUDQ, 3, xmmDst, xmmTemp2 ) ;
	//
	// xmmDst <- high[(dst.h32 * src.l32) << 32]
	//			: low[((dst.l32 * src.h32) << 32) + (dst.l32 * src.l32)]
	//
	WriteSSERegRegImm8Operand
			( sseop_PSHIFTQ_IMM, 3, mmxop2nd_SLL, xmmTemp1, 32 ) ;
	WriteSSERegRegOperand( sseop_MOVQ_LOAD, 3, xmmDst, xmmDst ) ;
	//
	WriteSSERegRegOperand( sseop_PADDQ, 3, xmmDst, xmmTemp1 ) ;
	//
	// xmmDst <- low[((dst.h32 * src.l32) << 32)
	//				+ ((dst.l32 * src.h32) << 32) + (dst.l32 * src.l32)]
	//
	WriteSSERegRegOperand( sseop_PSHUFD, 3, xmmTemp1, xmmDst, 0x4E, 1 ) ;
	WriteSSERegRegOperand( sseop_PADDQ, 3, xmmDst, xmmTemp1 ) ;
}

#endif
