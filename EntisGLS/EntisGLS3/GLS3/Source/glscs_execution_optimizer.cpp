
/*****************************************************************************
             Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
	Copyright (c) 2002-2012 Leshade Entis, Entis-soft. All rights reserved.
 ****************************************************************************/


#include <gls.h>


//////////////////////////////////////////////////////////////////////////////
// Entis Cotopha Script 実行イメージ・最適化
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ECSExecutionOptimizer, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ECSExecutionOptimizer::ECSExecutionOptimizer( ECSExecutionImageCompiler * pcsxi )
{
	m_pcsxi = pcsxi ;
	m_addrStart = 0 ;
	m_addrEnd = 0 ;
	m_regTempFirst = 0 ;
	m_regTempEnd = 0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ECSExecutionOptimizer::~ECSExecutionOptimizer( void )
{
}

// 最適化開始
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionOptimizer::BeginOptimize
	( DWORD dwStart, DWORD dwEnd,
			int regTempFirst, int regTempEnd )
{
	m_addrStart = dwStart ;
	m_addrEnd = dwEnd ;
	m_regTempFirst = regTempFirst ;
	m_regTempEnd = regTempEnd ;
	//
	InitializeInstructionList() ;
}

// 最適化処理
//////////////////////////////////////////////////////////////////////////////
int ECSExecutionOptimizer::PerformOptimize( void )
{
	int	nProcessed = 0 ;
	for ( int i = 0; i < (int) m_lstInstruction.GetSize(); i ++ )
	{
		OptimizeInfo	optinf ;
		TestOptimize( optinf, i ) ;
		if ( optinf.type != optimizeNothing )
		{
			switch ( optinf.type )
			{
			case	optimizeNothing:
				break ;
			case	optimizeOmission:
				RemoveInstruction( optinf.iTarget ) ;
				i -- ;
				nProcessed ++ ;
				break ;
			case	optimizeDstRegister:
				if ( OptimizeDstRegister( optinf ) )
				{
					nProcessed ++ ;
				}
				break ;
			case	optimizeSrcRegister:
				if ( OptimizeSrcRegister( optinf ) )
				{
					i -- ;
					nProcessed ++ ;
				}
				break ;
			case	optimizeAddrRegister:
				OptimizeAddrRegister( optinf ) ;
				nProcessed ++ ;
				break ;
			}
		}
	}
	return	nProcessed ;
}

// 最適化完了
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionOptimizer::FinishOptimize( void )
{
	CommitJumpAddress() ;
}

// 命令リスト生成
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionOptimizer::InitializeInstructionList( void )
{
	const BYTE *	pbytCode =
		(const BYTE *) m_pcsxi->m_bufImage.ModifyBuffer
							( m_addrStart, m_addrEnd - m_addrStart ) ;
	//
	ECSExecutionReverseAssembler	xra( m_pcsxi ) ;
	DWORD dwAddr = m_addrStart ;
	while ( dwAddr < m_addrEnd )
	{
		ECSExecutionReverseAssembler::InstructionInfo *
			pinf = new ECSExecutionReverseAssembler::InstructionInfo ;
		m_lstInstruction.Add( pinf ) ;
		//
		const DWORD	dwOffset = dwAddr - m_addrStart ;
		xra.ReverseAssemble
			( *pinf, pbytCode + dwOffset, dwAddr, false ) ;
		//
		bool	fNearJump = false ;
		DWORD	dwJumpTarget, dwJumpRefAddr ;
		switch ( pbytCode[dwOffset] )
		{
		case	csicJump:
		case	ECSSakura2Processor::codeJumpOffset32:
			fNearJump = true ;
			dwJumpRefAddr = dwAddr + 1 ;
			dwJumpTarget =
				dwAddr + pinf->nBytes
					+ *((SDWORD*)(pbytCode + (dwOffset + 1))) ;
			break ;
		case	csicCJump:
		case	ECSSakura2Processor::codeCNJumpOffset32:
		case	ECSSakura2Processor::codeCJumpOffset32:
			fNearJump = true ;
			dwJumpRefAddr = dwAddr + 2 ;
			dwJumpTarget =
				dwAddr + pinf->nBytes
					+ *((SDWORD*)(pbytCode + (dwOffset + 2))) ;
			break ;
		}
		if ( fNearJump )
		{
			ENumArray<DWORD> *	pList = m_tsaLabel.GetAs( dwJumpTarget ) ;
			if ( pList == NULL )
			{
				pList = new ENumArray<DWORD> ;
				m_tsaLabel.SetAs( dwJumpTarget, pList ) ;
			}
			pList->Add( dwJumpRefAddr ) ;
		}
		dwAddr += pinf->nBytes ;
	}
}

// ジャンプアドレス確定
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionOptimizer::CommitJumpAddress( void )
{
	const int	nCount = m_tsaLabel.GetSize() ;
	for ( int i = 0; i < nCount; i ++ )
	{
		DWORD *				pTargetAddr = m_tsaLabel.GetTagAt( i ) ;
		ENumArray<DWORD> *	pList = m_tsaLabel.GetObjectAt( i ) ;
		ESLAssert( (pTargetAddr != NULL) && (pList != NULL) ) ;
		if ( (pTargetAddr != NULL) && (pList != NULL) )
		{
			const DWORD	dwTargetAddr = *pTargetAddr ;
			const int	nListSize = pList->GetSize() ;
			for ( int j = 0; j < nListSize; j ++ )
			{
				SDWORD *	pdwJumpOffset =
					(SDWORD*) m_pcsxi->m_bufImage.ModifyBuffer
								( pList->GetAt(j), sizeof(DWORD) ) ;
				*pdwJumpOffset =
					(SDWORD) dwTargetAddr
								- (pList->GetAt(j) + sizeof(DWORD)) ;
			}
		}
	}
}

// コード省略判定
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionOptimizer::TestOptimize
		( ECSExecutionOptimizer::OptimizeInfo & optinf, int iTarget )
{
	ECSExecutionReverseAssembler::InstructionInfo *
						pinf = m_lstInstruction.GetAt( iTarget ) ;
	ESLAssert( pinf != NULL ) ;
	optinf.iTarget = iTarget ;
	if ( pinf == NULL )
	{
		optinf.type = optimizeOmission ;
		return ;
	}
	const int	regDst = pinf->regDst ;
	int			regMoveSrc = -1 ;
	bool		fMove = false ;
	switch ( pinf->nType )
	{
	case	ECSSakura2Processor::typeLoadMem:
	case	ECSSakura2Processor::typeLoadImm64:
		fMove = true ;
		break ;
	case	ECSSakura2Processor::typeStoreMem:
		TestOptimizeMemAddress( optinf, iTarget ) ;
		return ;
	case	ECSSakura2Processor::typeMoveReg:
		if ( IsSourceRegister( pinf, regDst ) )
		{
			// 入力と出力が同じレジスタの move 命令
			optinf.type = optimizeOmission ;
			return ;
		}
		regMoveSrc = pinf->regSrc1 ;
		fMove = true ;
		break ;
	case	ECSSakura2Processor::typeCvtMoveReg:
	case	ECSSakura2Processor::typeOpRegRegImm:
		fMove = true ;
		break ;
	case	ECSSakura2Processor::typeOpRegReg:
	case	ECSSakura2Processor::typeOpReg:
		break ;
	default:
		optinf.type = optimizeNothing ;
		return ;
	}
	if ( regDst < 0 )
	{
		optinf.type = optimizeNothing ;
		return ;
	}
	bool	fCodeEscape = true ;			// 途中で依存チェーンを中断する
	bool	fChangeMoveSource = false ;		// ソースレジスタは変更される
	bool	fModifyRegister = false ;		// 出力レジスタは依存性を持って変更される
	bool	fRefRegExceptMove = false ;		// move 以外の命令で出力レジスタが参照される
	int		nRefRegCount = 0 ;				// 出力レジスタが参照される回数
	int		regMoveChainDst = -1 ;			// move ソースに出力レジスタが使用されるときの出力レジスタ
	int		iLastMoveChain = -1 ;			// regMoveChainDst の move 命令指標
	int		iTerminate = -1 ;				// 終端指標
	for ( int i = iTarget + 1; i < (int) m_lstInstruction.GetSize(); i ++ )
	{
		const ECSExecutionReverseAssembler::InstructionInfo *
								pinfNext = m_lstInstruction.GetAt( i ) ;
		if ( pinfNext == NULL )
		{
			continue ;
		}
		if ( m_tsaLabel.GetAs( (DWORD) pinfNext->nAddress ) != NULL )
		{
			fCodeEscape = false ;
			break ;
		}
		if ( (regMoveSrc >= 0) && (regMoveSrc == pinfNext->regDst) )
		{
			fChangeMoveSource = true ;
		}
		if ( (pinfNext->nType == ECSSakura2Processor::typeLoadMem)
			|| (pinfNext->nType == ECSSakura2Processor::typeLoadImm64)
			|| (pinfNext->nType == ECSSakura2Processor::typeCvtMoveReg)
			|| (pinfNext->nType == ECSSakura2Processor::typeOpRegRegImm) )
		{
			if ( IsSourceRegister( pinfNext, regDst ) )
			{
				nRefRegCount ++ ;
				fRefRegExceptMove = true ;
				//
				if ( pinfNext->regDst == regDst )
				{
					iTerminate = i + 1 ;
					break ;
				}
			}
			else if ( pinfNext->regDst == regDst )
			{
				iTerminate = i ;
				break ;
			}
		}
		else if ( pinfNext->nType == ECSSakura2Processor::typeStoreMem )
		{
			if ( IsSourceRegister( pinfNext, regDst ) )
			{
				nRefRegCount ++ ;
				fRefRegExceptMove = true ;
			}
		}
		else if ( pinfNext->nType == ECSSakura2Processor::typeMoveReg )
		{
			if ( IsSourceRegister( pinfNext, regDst ) )
			{
				nRefRegCount ++ ;
				regMoveChainDst = pinfNext->regDst ;
				iLastMoveChain = i ;
			}
			else if ( pinfNext->regDst == regDst )
			{
				iTerminate = i ;
				break ;
			}
		}
		else if ( (pinfNext->nType == ECSSakura2Processor::typeOpRegReg)
				|| (pinfNext->nType == ECSSakura2Processor::typeOpReg) )
		{
			if ( IsSourceRegister( pinfNext, regDst ) )
			{
				nRefRegCount ++ ;
				fRefRegExceptMove = true ;
				//
				if ( pinfNext->regDst == regDst )
				{
					fModifyRegister = true ;
					break ;
				}
			}
			else if ( pinfNext->regDst == regDst )
			{
				iTerminate = i ;
				break ;
			}
		}
		else
		{
			fCodeEscape = false ;
			break ;
		}
	}
	do
	{
		if ( fModifyRegister || !fCodeEscape )
		{
			// 出力レジスタは参照されるし、変更されて出力される
			// あるいは、判別できない命令を発見したので
			// これ以上参照チェーンを追跡できず最適化出来ない
			optinf.type = optimizeNothing ;
			break ;
		}
		if ( iTerminate < 0 )
		{
			if ( (m_regTempFirst <= regDst)
				&& (regDst <= m_regTempEnd) )
			{
				if ( nRefRegCount == 0 )
				{
					// 出力されたレジスタは使用されなかったし、使用されない
					optinf.type = optimizeOmission ;
					return ;
				}
			}
			else
			{
				// これ以上参照チェーンを追跡できず最適化出来ない
				optinf.type = optimizeNothing ;
				break ;
			}
		}
		else
		{
			if ( nRefRegCount == 0 )
			{
				// 出力レジスタは使用されることなく上書きされた
				optinf.type = optimizeOmission ;
				return ;
			}
		}
		if ( fMove && !fRefRegExceptMove
			&& (nRefRegCount == 1) && (regMoveChainDst >= 0) )
		{
			bool	fDstRef = false ;
			for ( int i = iTarget + 1; i <= iLastMoveChain; i ++ )
			{
				const ECSExecutionReverseAssembler::InstructionInfo *
										pinfNext = m_lstInstruction.GetAt( i ) ;
				if ( pinfNext == NULL )
				{
					continue ;
				}
				if ( IsSourceRegister( pinfNext, regMoveChainDst ) )
				{
					fDstRef = true ;
					break ;
				}
				else if ( (pinfNext->nType == ECSSakura2Processor::typeComplex)
						|| (pinfNext->nType == ECSSakura2Processor::typeObject)
						|| (pinfNext->nFlags & ECSSakura2Processor::flagComplexSourceRegister) )
				{
					fDstRef = true ;
					break ;
				}
			}
			if ( !fDstRef )
			{
				// move or load 命令で出力レジスタは
				// 一度だけ move 命令で使用された
				optinf.type = optimizeDstRegister ;
				optinf.iStart = iLastMoveChain ;
				optinf.regDstReplace = regMoveChainDst ;
				return ;
			}
		}
		if ( (regMoveSrc >= 0) && !fChangeMoveSource )
		{
			// move 命令の入力レジスタは
			// 出力レジスタが上書きされるまでの間変更されないので
			// 置き変えることが出来る
			optinf.type = optimizeSrcRegister ;
			optinf.iStart = iTarget + 1 ;
			optinf.iTerminate = iTerminate ;
			optinf.regSrcReplace = regDst ;
			optinf.regDstReplace = regMoveSrc ;
			return ;
		}
	}
	while ( false ) ;
	//
	TestOptimizeMemAddress( optinf, iTarget ) ;
}

void ECSExecutionOptimizer::TestOptimizeMemAddress( OptimizeInfo & optinf, int iTarget )
{
	ECSExecutionReverseAssembler::InstructionInfo *
						pinf = m_lstInstruction.GetAt( iTarget ) ;
	ESLAssert( pinf != NULL ) ;
	optinf.iTarget = iTarget ;
	if ( pinf == NULL )
	{
		optinf.type = optimizeOmission ;
		return ;
	}
	if ( ((pinf->nType == ECSSakura2Processor::typeLoadMem)
		|| (pinf->nType == ECSSakura2Processor::typeStoreMem))
		&& (m_tsaLabel.GetAs( (DWORD) pinf->nAddress ) == NULL) )
	{
		//
		// メモリ命令で [zp+rx*1] 又は [zp+rx*1+offset] 形式の場合
		// zp を使用しないシンプルなエンコーディングに変更できないかテスト
		//
		BYTE *	pbytCode =
			(BYTE *) m_pcsxi->m_bufImage.ModifyBuffer
									( pinf->nAddress, pinf->nBytes ) ;
		if ( (pbytCode[0] == ECSSakura2Processor::codeLoadMemBaseIndex)
			|| (pbytCode[0] == ECSSakura2Processor::codeLoadMemBaseIndexImm32)
			|| (pbytCode[0] == ECSSakura2Processor::codeStoreMemBaseIndex)
			|| (pbytCode[0] == ECSSakura2Processor::codeStoreMemBaseIndexImm32) )
		{
			if ( ((pbytCode[1] >> 3) == ECSSakura2Processor::regZeroPtr)
				&& ((pbytCode[2] & 0x80) == 0) )
			{
				const int	regPtr = pbytCode[2] & 0x7F ;
				int			iOrgRegLoaded = -1 ;
				int			regReplace = -1 ;
				int			iOffset = 0 ;
				if ( (regPtr >= 0) && (regPtr < 0x10) )
				{
					regReplace = regPtr ;
				}
				else
				{
					for ( int i = iTarget - 1; i >= 0; i -- )
					{
						const ECSExecutionReverseAssembler::InstructionInfo *
												pinfPrev = m_lstInstruction.GetAt( i ) ;
						if ( pinfPrev == NULL )
						{
							continue ;
						}
						if ( pinfPrev->nType == ECSSakura2Processor::typeMoveReg )
						{
							if ( pinfPrev->regDst == regPtr )
							{
								if ( pinfPrev->regSrc1 < 0x10 )
								{
									iOrgRegLoaded = i ;
									regReplace = pinfPrev->regSrc1 ;
								}
								break ;
							}
						}
						else if ( pinfPrev->nType == ECSSakura2Processor::typeOpRegRegImm )
						{
							if ( pinfPrev->regDst == regPtr )
							{
								BYTE *	pbytCode =
									(BYTE *) m_pcsxi->m_bufImage.ModifyBuffer
												( pinfPrev->nAddress, pinfPrev->nBytes ) ;
								if ( (pinfPrev->regSrc1 < 0x10)
									&& (pbytCode[0] == ECSSakura2Processor::codeAddImm32) )
								{
									iOrgRegLoaded = i ;
									regReplace = pinfPrev->regSrc1 ;
									iOffset = *((SDWORD*)(pbytCode + 3)) ;
								}
								break ;
							}
						}
						else if ( pinfPrev->regDst == regPtr )
						{
							break ;
						}
						else if ( pinfPrev->nFlags & ECSSakura2Processor::flagComplexDestinationRegister )
						{
							break ;
						}
						if ( m_tsaLabel.GetAs( (DWORD) pinfPrev->nAddress ) != NULL )
						{
							break ;
						}
					}
				}
				if ( iOrgRegLoaded >= 0 )
				{
					for ( int i = iOrgRegLoaded + 1; i < iTarget; i ++ )
					{
						const ECSExecutionReverseAssembler::InstructionInfo *
												pinfPrev = m_lstInstruction.GetAt( i ) ;
						if ( (pinfPrev == NULL) || (pinfPrev->regDst == regReplace)
							|| (pinfPrev->nFlags & ECSSakura2Processor::flagComplexDestinationRegister) )
						{
							regReplace = -1 ;
							break ;
						}
					}
				}
				if ( regReplace >= 0 )
				{
					optinf.type = optimizeAddrRegister ;
					optinf.iTarget = iTarget ;
					optinf.iStart = iTarget ;
					optinf.iTerminate = iTarget + 1 ;
					optinf.regSrcReplace = regReplace ;
					optinf.iOffset = iOffset ;
					return ;
				}
			}
		}
	}
	// その他（最適化しない）
	optinf.type = optimizeNothing ;
	return ;
}

// optimizeDstRegister 最適化実行
//////////////////////////////////////////////////////////////////////////////
bool ECSExecutionOptimizer::OptimizeDstRegister( const OptimizeInfo & optinf )
{
	ECSExecutionReverseAssembler::InstructionInfo *
						pinf = m_lstInstruction.GetAt( optinf.iTarget ) ;
	ESLAssert( pinf != NULL ) ;
	//
	BYTE *	pbytCode =
		(BYTE *) m_pcsxi->m_bufImage.ModifyBuffer
								( pinf->nAddress, pinf->nBytes ) ;
	//
	const int	regDst = optinf.regDstReplace ;
	ESLAssert( (regDst >= 0) && (regDst < 0x100) ) ;
	//
	switch ( pbytCode[0] )
	{
	case	ECSSakura2Processor::codeLoadMemBase:
	case	ECSSakura2Processor::codeLoadMemBaseImm32:
	case	ECSSakura2Processor::codeLoadLocalImm32:
		pbytCode[2] = (BYTE) regDst ;
		pinf->regDst = regDst ;
		break ;
	case	ECSSakura2Processor::codeLoadMemBaseIndex:
	case	ECSSakura2Processor::codeLoadMemBaseIndexImm32:
	case	ECSSakura2Processor::codeLoadLocalIndexImm32:
		pbytCode[3] = (BYTE) regDst ;
		pinf->regDst = regDst ;
		break ;
	case	ECSSakura2Processor::codeMoveReg:
	case	ECSSakura2Processor::codeMoveSx32Reg:
	case	ECSSakura2Processor::codeMoveSx16Reg:
	case	ECSSakura2Processor::codeMoveSx8Reg:
	case	ECSSakura2Processor::codeCvtFloat2Int:
	case	ECSSakura2Processor::codeCvtInt2Float:
	case	ECSSakura2Processor::codeLoadImm64:
	case	ECSSakura2Processor::codeSrlImm8:
	case	ECSSakura2Processor::codeSraImm8:
	case	ECSSakura2Processor::codeSllImm8:
	case	ECSSakura2Processor::codeMaskMove:
	case	ECSSakura2Processor::codeAddImm32:
	case	ECSSakura2Processor::codeMulImm32:
	case	ECSSakura2Processor::codePopReg:
		pbytCode[1] = (BYTE) regDst ;
		pinf->regDst = regDst ;
		break ;
	default:
		return	false ;
	}
	//
	RemoveInstruction( optinf.iStart ) ;
	//
	return	true ;
}

// optimizeSrcRegister 最適化実行
//////////////////////////////////////////////////////////////////////////////
bool ECSExecutionOptimizer::OptimizeSrcRegister( const OptimizeInfo & optinf )
{
	int	iTerminate = optinf.iTerminate ;
	if ( iTerminate < 0 )
	{
		iTerminate = m_lstInstruction.GetSize() ;
	}
	const int	regSrc = optinf.regSrcReplace ;
	const int	regDst = optinf.regDstReplace ;
	ESLAssert( (regSrc >= 0) && (regSrc < 0x100) ) ;
	ESLAssert( (regDst >= 0) && (regDst < 0x100) ) ;
	bool	fRemovable = true ;
	//
	for ( int i = optinf.iStart; i < iTerminate; i ++ )
	{
		ECSExecutionReverseAssembler::InstructionInfo *
							pinf = m_lstInstruction.GetAt( i ) ;
		ESLAssert( pinf != NULL ) ;
		//
		BYTE *	pbytCode =
			(BYTE *) m_pcsxi->m_bufImage.ModifyBuffer
									( pinf->nAddress, pinf->nBytes ) ;
		switch ( pinf->nType )
		{
		case	ECSSakura2Processor::typeLoadMem:
		case	ECSSakura2Processor::typeStoreMem:
			if ( (ECSSakura2Processor::codeLoadMem <= pbytCode[0])
				&& (pbytCode[0] < ECSSakura2Processor::codeLoadLocal) )
			{
				bool		fProcessed = false ;
				const int	regBase = (pbytCode[1] >> 3) & 0x0F ;
				if ( regBase == regSrc )
				{
					if ( regDst <= 0x0F )
					{
						pbytCode[1] =
							(BYTE) ((pbytCode[1] & 0x87) | (regDst << 3)) ;
						fProcessed = true ;
					}
					else
					{
						fRemovable = false ;
					}
				}
				if ( (pbytCode[0] == ECSSakura2Processor::codeLoadMemBaseIndex)
					|| (pbytCode[0] == ECSSakura2Processor::codeLoadMemBaseIndexImm32)
					|| (pbytCode[0] == ECSSakura2Processor::codeStoreMemBaseIndex)
					|| (pbytCode[0] == ECSSakura2Processor::codeStoreMemBaseIndexImm32) )
				{
					if ( (ECSSakura2Processor::codeStoreMem <= pbytCode[0])
						&& (pbytCode[0] < ECSSakura2Processor::codeLoadLocal) )
					{
						if ( pbytCode[3] == regSrc )
						{
							pbytCode[3] = (BYTE) regDst ;
							fProcessed = true ;
						}
					}
					const int	regIndex = pbytCode[2] & 0x7F ;
					const int	scaleIndex =
							((pbytCode[1] >> 6) & 0x02) | (pbytCode[2] >> 7) ;
					if ( regIndex == regSrc )
					{
						if ( (scaleIndex == 0) && (regDst <= 0x0F)
							&& (regBase == ECSSakura2Processor::regZeroPtr) )
						{
							bool	fOffset = false ;
							switch ( pbytCode[0] )
							{
							case	ECSSakura2Processor::codeLoadMemBaseIndex:
								pbytCode[0] = ECSSakura2Processor::codeLoadMemBase ;
								break ;
							case	ECSSakura2Processor::codeLoadMemBaseIndexImm32:
								pbytCode[0] = ECSSakura2Processor::codeLoadMemBaseImm32 ;
								fOffset = true ;
								break ;
							case	ECSSakura2Processor::codeStoreMemBaseIndex:
								pbytCode[0] = ECSSakura2Processor::codeStoreMemBase ;
								break ;
							case	ECSSakura2Processor::codeStoreMemBaseIndexImm32:
								pbytCode[0] = ECSSakura2Processor::codeStoreMemBaseImm32 ;
								fOffset = true ;
								break ;
							}
							pbytCode[1] =
								(BYTE) ((pbytCode[1] & 0x07) | (regDst << 3)) ;
							pbytCode[2] = pbytCode[3] ;
							if ( fOffset )
							{
								*((DWORD*)(pbytCode + 3)) = *((DWORD*)(pbytCode + 4)) ;
								RemoveCodeImage( pinf->nAddress + 7, 1 ) ;
							}
							else
							{
								RemoveCodeImage( pinf->nAddress + 3, 1 ) ;
							}
							pbytCode = (BYTE *)
									m_pcsxi->m_bufImage.ModifyBuffer
										( pinf->nAddress, pinf->nBytes - 1 ) ;
							fProcessed = true ;
						}
						else if ( regDst <= 0x7F )
						{
							pbytCode[2] =
								(BYTE) ((pbytCode[2] & 0x80) | regDst) ;
							fProcessed = true ;
						}
						else
						{
							fRemovable = false ;
						}
					}
				}
				else
				{
					if ( (ECSSakura2Processor::codeStoreMem <= pbytCode[0])
						&& (pbytCode[0] < ECSSakura2Processor::codeLoadLocal) )
					{
						if ( pbytCode[2] == regSrc )
						{
							pbytCode[2] = (BYTE) regDst ;
							fProcessed = true ;
						}
					}
				}
				if ( fProcessed )
				{
					ECSExecutionReverseAssembler	xra( m_pcsxi ) ;
					xra.ReverseAssemble
						( *pinf, pbytCode, pinf->nAddress, false ) ;
				}
			}
			else
			{
				bool		fProcessed = false ;
				switch ( pbytCode[0] )
				{
				case	ECSSakura2Processor::codeLoadLocalIndexImm32:
				case	ECSSakura2Processor::codeStoreLocalIndexImm32:
					if ( pbytCode[2] == regSrc )
					{
						pbytCode[2] = (BYTE) regDst ;
						fProcessed = true ;
					}
					break ;
				}
				switch ( pbytCode[0] )
				{
				case	ECSSakura2Processor::codeStoreLocalImm32:
					if ( pbytCode[2] == regSrc )
					{
						pbytCode[2] = (BYTE) regDst ;
						fProcessed = true ;
					}
					break ;
				case	ECSSakura2Processor::codeStoreLocalIndexImm32:
					if ( pbytCode[3] == regSrc )
					{
						pbytCode[3] = (BYTE) regDst ;
						fProcessed = true ;
					}
					break ;
				}
				if ( fProcessed )
				{
					ECSExecutionReverseAssembler	xra( m_pcsxi ) ;
					xra.ReverseAssemble
						( *pinf, pbytCode, pinf->nAddress, false ) ;
				}
			}
			break ;
		case	ECSSakura2Processor::typeMoveReg:
		case	ECSSakura2Processor::typeCvtMoveReg:
		case	ECSSakura2Processor::typeOpRegRegImm:
			if ( pbytCode[2] == regSrc )
			{
				ESLAssert( pinf->regSrc1 == regSrc ) ;
				pbytCode[2] = (BYTE) regDst ;
				pinf->regSrc1 = regDst ;
			}
			break ;
		case	ECSSakura2Processor::typeOpRegReg:
		case	ECSSakura2Processor::typeOpRegRegReg:
			if ( pbytCode[2] == regSrc )
			{
				ESLAssert( (pinf->regSrc1 == regSrc)
							|| (pinf->regSrc2 == regSrc) ) ;
				pbytCode[2] = (BYTE) regDst ;
				//
				if ( pinf->regSrc1 == regSrc )
				{
					pinf->regSrc1 = regDst ;
				}
				else
				{
					pinf->regSrc2 = regDst ;
				}
			}
			else if ( (pbytCode[3] == regSrc)
				&& (pinf->nType == ECSSakura2Processor::typeOpRegRegReg) )
			{
				ESLAssert( pinf->regSrc2 == regSrc ) ;
				pbytCode[3] = (BYTE) regDst ;
				pinf->regSrc2 = regDst ;
			}
			break ;
		case	ECSSakura2Processor::typeOpReg:
			ESLAssert( pinf->regDst != regSrc ) ;
			if ( pbytCode[1] == regSrc )
			{
				ESLAssert( pinf->regSrc1 == regSrc ) ;
				pbytCode[1] = (BYTE) regDst ;
				pinf->regSrc1 = regDst ;
			}
			break ;
		default:
			ESLAssert( !IsSourceRegister( pinf, regSrc ) ) ;
			break ;
		}
	}
	if ( fRemovable )
	{
		RemoveInstruction( optinf.iTarget ) ;
	}
	return	fRemovable ;
}

// optimizeAddrRegister 最適化実行
//////////////////////////////////////////////////////////////////////////////
bool ECSExecutionOptimizer::OptimizeAddrRegister( const OptimizeInfo & optinf )
{
	ECSExecutionReverseAssembler::InstructionInfo *
						pinf = m_lstInstruction.GetAt( optinf.iTarget ) ;
	ESLAssert( pinf != NULL ) ;
	//
	BYTE *	pbytCode =
		(BYTE *) m_pcsxi->m_bufImage.ModifyBuffer
								( pinf->nAddress, pinf->nBytes ) ;
	//
	const int	regAddr = optinf.regSrcReplace ;
	const int	iOffset = optinf.iOffset ;
	ESLAssert( (regAddr >= 0) && (regAddr < 0x100) ) ;
	//
	DWORD	dwOffset ;
	switch ( pbytCode[0] )
	{
	case	ECSSakura2Processor::codeLoadMemBaseIndex:
		if ( iOffset == 0 )
		{
			pbytCode[0] = ECSSakura2Processor::codeLoadMemBase ;
			pbytCode[1] = (BYTE) (regAddr << 3) | (pbytCode[1] & 0x07) ;
			pbytCode[2] = pbytCode[3] ;
			//
			pinf->nBytes = 3 ;
			pinf->regSrc1 = regAddr ;
			pinf->regSrc2 = -1 ;
			pinf->regSrc3 = -1 ;
			//
			RemoveCodeImage( pinf->nAddress + pinf->nBytes, 1 ) ;
		}
		else
		{
			RemoveCodeImage( pinf->nAddress + pinf->nBytes, pinf->nBytes - 7 ) ;
			pbytCode = (BYTE *) m_pcsxi->m_bufImage.ModifyBuffer
										( pinf->nAddress, pinf->nBytes ) ;
			//
			if ( regAddr != ECSSakura2Processor::regBP )
			{
				pbytCode[0] = ECSSakura2Processor::codeLoadMemBaseImm32 ;
				pbytCode[1] = (BYTE) (regAddr << 3) | (pbytCode[1] & 0x07) ;
			}
			else
			{
				pbytCode[0] = ECSSakura2Processor::codeLoadLocalImm32 ;
				pbytCode[1] = (pbytCode[1] & 0x07) ;
			}
			pbytCode[2] = pbytCode[3] ;
			*((DWORD*)(pbytCode + 3)) = iOffset ;
			//
			pinf->nBytes = 7 ;
			pinf->regSrc1 = regAddr ;
			pinf->regSrc2 = -1 ;
			pinf->regSrc3 = -1 ;
		}
		break ;

	case	ECSSakura2Processor::codeLoadMemBaseIndexImm32:
		if ( regAddr != ECSSakura2Processor::regBP )
		{
			pbytCode[0] = ECSSakura2Processor::codeLoadMemBaseImm32 ;
			pbytCode[1] = (BYTE) (regAddr << 3) | (pbytCode[1] & 0x07) ;
		}
		else
		{
			pbytCode[0] = ECSSakura2Processor::codeLoadLocalImm32 ;
			pbytCode[1] = (pbytCode[1] & 0x07) ;
		}
		pbytCode[2] = pbytCode[3] ;
		dwOffset = *((DWORD*)(pbytCode + 4)) ;
		*((DWORD*)(pbytCode + 3)) = dwOffset + iOffset ;
		//
		pinf->nBytes = 7 ;
		pinf->regSrc1 = regAddr ;
		pinf->regSrc2 = -1 ;
		pinf->regSrc3 = -1 ;
		//
		RemoveCodeImage( pinf->nAddress + pinf->nBytes, 1 ) ;
		break ;

	case	ECSSakura2Processor::codeStoreMemBaseIndex:
		if ( iOffset == 0 )
		{
			pbytCode[0] = ECSSakura2Processor::codeStoreMemBase ;
			pbytCode[1] = (BYTE) (regAddr << 3) | (pbytCode[1] & 0x07) ;
			pbytCode[2] = pbytCode[3] ;
			//
			pinf->nBytes = 3 ;
			pinf->regSrc1 = regAddr ;
			pinf->regSrc2 = pbytCode[2] ;
			pinf->regSrc3 = -1 ;
			//
			RemoveCodeImage( pinf->nAddress + pinf->nBytes, 1 ) ;
		}
		else
		{
			RemoveCodeImage( pinf->nAddress + pinf->nBytes, pinf->nBytes - 7 ) ;
			pbytCode = (BYTE *) m_pcsxi->m_bufImage.ModifyBuffer
										( pinf->nAddress, pinf->nBytes ) ;
			//
			if ( regAddr != ECSSakura2Processor::regBP )
			{
				pbytCode[0] = ECSSakura2Processor::codeStoreMemBaseImm32 ;
				pbytCode[1] = (BYTE) (regAddr << 3) | (pbytCode[1] & 0x07) ;
			}
			else
			{
				pbytCode[0] = ECSSakura2Processor::codeStoreLocalImm32 ;
				pbytCode[1] = (pbytCode[1] & 0x07) ;
			}
			pbytCode[2] = pbytCode[3] ;
			*((DWORD*)(pbytCode + 3)) = iOffset ;
			//
			pinf->nBytes = 7 ;
			pinf->regSrc1 = regAddr ;
			pinf->regSrc2 = pbytCode[2] ;
			pinf->regSrc3 = -1 ;
		}
		break ;

	case	ECSSakura2Processor::codeStoreMemBaseIndexImm32:
		if ( regAddr != ECSSakura2Processor::regBP )
		{
			pbytCode[0] = ECSSakura2Processor::codeStoreMemBaseImm32 ;
			pbytCode[1] = (BYTE) (regAddr << 3) | (pbytCode[1] & 0x07) ;
		}
		else
		{
			pbytCode[0] = ECSSakura2Processor::codeStoreLocalImm32 ;
			pbytCode[1] = (pbytCode[1] & 0x07) ;
		}
		pbytCode[2] = pbytCode[3] ;
		dwOffset = *((DWORD*)(pbytCode + 4)) ;
		*((DWORD*)(pbytCode + 3)) = dwOffset + iOffset ;
		//
		pinf->nBytes = 7 ;
		pinf->regSrc1 = regAddr ;
		pinf->regSrc2 = pbytCode[2] ;
		pinf->regSrc3 = -1 ;
		//
		RemoveCodeImage( pinf->nAddress + pinf->nBytes, 1 ) ;
		break ;

	default:
		return	false ;
	}
	return	true ;
}

// コードの命令単位での削除
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionOptimizer::RemoveInstruction( int iInst )
{
	ECSExecutionReverseAssembler::InstructionInfo *
						pinf = m_lstInstruction.GetAt( iInst ) ;
	if ( pinf != NULL )
	{
		RemoveCodeImage( pinf->nAddress, pinf->nBytes ) ;
	}
}

// コードのバイト単位での削除
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionOptimizer::RemoveCodeImage( DWORD dwAddress, int nRange )
{
	//
	// ジャンプ命令の位置修正・削除
	//
	int	i ;
	for ( i = 0; i < (int) m_tsaLabel.GetSize(); i ++ )
	{
		ENumArray<DWORD> *	pList = m_tsaLabel.GetObjectAt( i ) ;
		if ( pList != NULL )
		{
			for ( int j = 0; j < (int) pList->GetSize(); j ++ )
			{
				DWORD	dwRefAddr = pList->GetAt( j ) ;
				if ( dwAddress <= dwRefAddr )
				{
					if ( dwRefAddr < dwAddress + nRange )
					{
						pList->RemoveAt( j -- ) ;
					}
					else
					{
						pList->SetAt( j, dwRefAddr - nRange ) ;
					}
				}
			}
		}
	}
	//
	// ジャンプ先位置修正
	//
	for ( i = 0; i < (int) m_tsaLabel.GetSize(); i ++ )
	{
		DWORD *				pdwJumpTarget = m_tsaLabel.GetTagAt( i ) ;
		ENumArray<DWORD> *	pList = m_tsaLabel.GetObjectAt( i ) ;
		if ( (pdwJumpTarget != NULL) && (pList != NULL)
			&& (dwAddress <= *pdwJumpTarget) )
		{
			DWORD	dwModifiedJumpTarget = *pdwJumpTarget - nRange ;
			if ( *pdwJumpTarget < dwAddress + nRange )
			{
				dwModifiedJumpTarget = dwAddress ;
			}
			m_tsaLabel.DetachAs( *pdwJumpTarget ) ;
			//
			ENumArray<DWORD> *	pListMerge =
						m_tsaLabel.GetAs( dwModifiedJumpTarget ) ;
			if ( pListMerge != NULL )
			{
				pListMerge->Merge( pListMerge->GetSize(), *pList ) ;
				delete	pList ;
				pList = pListMerge ;
			}
			else
			{
				m_tsaLabel.Add( dwModifiedJumpTarget, pList ) ;
			}
		}
	}
	//
	// 命令情報リスト修正
	//
	for ( i = 0; i < (int) m_lstInstruction.GetSize(); i ++ )
	{
		ECSExecutionReverseAssembler::InstructionInfo *
							pinf = m_lstInstruction.GetAt( i ) ;
		if ( pinf == NULL )
		{
			continue ;
		}
		if ( (DWORD) pinf->nAddress >= dwAddress )
		{
			if ( (DWORD) pinf->nAddress < dwAddress + nRange )
			{
				ESLAssert( (DWORD) pinf->nAddress + pinf->nBytes <= dwAddress + nRange ) ;
				m_lstInstruction.RemoveAt( i ) ;
				i -- ;
			}
			else
			{
				pinf->nAddress -= nRange ;
			}
		}
	}
	//
	// コード情報から削除
	//
	if ( nRange > 0 )
	{
		m_pcsxi->RemoveCodeImage( dwAddress, nRange ) ;
	}
	else if ( nRange < 0 )
	{
		m_pcsxi->ShiftCodeImage( dwAddress, - nRange ) ;
	}
}

