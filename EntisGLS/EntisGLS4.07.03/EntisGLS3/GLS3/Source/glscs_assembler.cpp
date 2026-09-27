
/*****************************************************************************
             Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
	Copyright (c) 2013 Leshade Entis, Entis-soft. All rights reserved.
 ****************************************************************************/

#include <gls.h>

using namespace SSystem ;
using namespace ECSSakura2Processor ;
using namespace ECSSakura2Assember ;


//////////////////////////////////////////////////////////////////////////////
// Entis Cotopha Script ver.3 Sakura2 アセンブラ
//////////////////////////////////////////////////////////////////////////////

const wchar_t * ECSAssembler::m_pwszDirectiveName[ECSAssembler::dirCount] =
{
	L".IF", L".ELSEIF", L".ELSE", L".ENDIF",
	L".WHILE", L".ENDW", L".REPEAT", L".UNTIL",
	L".BREAK", L".CONTINUE",
	L"REG", L"ASSUME", L"INVOKE",
} ;

const ECSAssembler::PFUNC_ASSEMBLE	ECSAssembler::m_pfnDirective[dirCount] =
{
	&ECSAssembler::AssembleIf,
	&ECSAssembler::AssembleElseIf,
	&ECSAssembler::AssembleElse,
	&ECSAssembler::AssembleEndIf,
	&ECSAssembler::AssembleWhile,
	&ECSAssembler::AssembleEndWhile,
	&ECSAssembler::AssembleRepeat,
	&ECSAssembler::AssembleUntil,
	&ECSAssembler::AssembleBreak,
	&ECSAssembler::AssembleContinue,
	&ECSAssembler::AssembleRegister,
	&ECSAssembler::AssembleAssume,
	&ECSAssembler::AssembleInvoke,
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ECSAssembler, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ECSAssembler::ECSAssembler( void )
{
	m_maskTempReg = 0 ;
	m_compiler = NULL ;
	m_pcsxi = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ECSAssembler::~ECSAssembler( void )
{
}

// コンパイラ設定
//////////////////////////////////////////////////////////////////////////////
void ECSAssembler::AttachCompiler( ECSCompiler * compiler )
{
	m_compiler = compiler ;
}

// 出力先設定
//////////////////////////////////////////////////////////////////////////////
void ECSAssembler::AttachOutputImage( ECSExecutionImageCompiler * pcsxi )
{
	m_pcsxi = pcsxi ;
}

// 出力完了処理
//////////////////////////////////////////////////////////////////////////////
void ECSAssembler::FinishOutputImage( void )
{
	if ( m_pcsxi != NULL )
	{
		for ( size_t i = 0; i < m_ssoaLabel.GetLength(); i ++ )
		{
			LabelEntry *	pLabel = m_ssoaLabel.GetAt( i ) ;
			if ( pLabel != NULL )
			{
				CommitLabelReference( *pLabel ) ;
			}
		}
	}
	m_compiler = NULL ;
	m_pcsxi = NULL ;
}

// ディレクティブ判定
//////////////////////////////////////////////////////////////////////////////
ECSAssembler::DirectiveType
	ECSAssembler::IsDirective( const wchar_t * pwszName )
{
	SString	strName = pwszName ;
	for ( int i = 0; i < dirCount; i ++ )
	{
		if ( strName.CompareNoCase( m_pwszDirectiveName[i] ) == 0 )
		{
			return	(ECSAssembler::DirectiveType) i ;
		}
	}
	return	dirInvalid ;
}

// １行アセンブル
//////////////////////////////////////////////////////////////////////////////
SError ECSAssembler::AssembleLine
		( SSystem::SStringParser & sparsLine, int nPass )
{
	SError	err ;
	m_strErrMsg.FreeArray() ;
	if ( !sparsLine.PassSpace() )
	{
		return	errSuccess ;
	}
	SString	strMnemonic ;
	sparsLine.NextString( strMnemonic ) ;
	if ( strMnemonic.GetAt( strMnemonic.GetLength() - 1 ) == L':' )
	{
		//
		// ラベル判定
		//
		SStringParser	sparsLabel ;
		sparsLabel.AttachString
			( strMnemonic.GetConstArray(), strMnemonic.GetLength() ) ;
		err = CompileLabel( sparsLabel, nPass ) ;
		if ( err )
		{
			return	err ;
		}
		if ( !sparsLine.PassSpace() )
		{
			return	errSuccess ;
		}
		sparsLine.NextString( strMnemonic ) ;
	}
	if ( nPass == 0 )
	{
		return	errSuccess ;
	}
	//
	// ディレクティブ
	//
	DirectiveType	typeDir = IsDirective( strMnemonic ) ;
	if ( typeDir != dirInvalid )
	{
		return	CompileDirective( typeDir, sparsLine ) ;
	}
	//
	// ニーモニックアセンブル
	//
	return	AssembleMnemonic( strMnemonic, sparsLine ) ;
}

// ディレクティブコンパイル
//////////////////////////////////////////////////////////////////////////////
SError ECSAssembler::CompileDirective
	( ECSAssembler::DirectiveType dirType, SSystem::SStringParser & sparsLine )
{
	SError	err = (this->*(m_pfnDirective[dirType]))( sparsLine ) ;
	if ( err )
	{
		return	err ;
	}
	if ( sparsLine.PassSpace() )
	{
		m_strErrMsg = L"\'" ;
		m_strErrMsg +=
			sparsLine.SubString
				( sparsLine.GetIndex(),
					sparsLine.GetLength() - sparsLine.GetIndex() ) ;
		m_strErrMsg += L"\' は処理されません" ;
		return	errFailed ;
	}
	return	errSuccess ;
}

// ラベル定義
//////////////////////////////////////////////////////////////////////////////
SError ECSAssembler::CompileLabel
	( SSystem::SStringParser & sparsLine, int nPass )
{
	SString	strLabel ;
	SStringParser::TokenType
			typeToken = sparsLine.NextToken( strLabel ) ;
	if ( typeToken != SStringParser::tokenNormal )
	{
		m_strErrMsg = L"不正なラベル名です" ;
		return	errFailed ;
	}
	if ( sparsLine.HasToComeChar( L":" ) != L':' )
	{
		m_strErrMsg = L"不正なラベル名です" ;
		return	errFailed ;
	}
	LabelEntry *	pLabel = m_ssoaLabel.GetAs( strLabel ) ;
	if ( pLabel != NULL )
	{
		if ( nPass == 0 )
		{
			m_strErrMsg = strLabel + L" を二重に定義しています" ;
			return	errFailed ;
		}
		pLabel->m_addrLabel = m_pcsxi->m_bufImage.GetLength() ;
	}
	else
	{
		pLabel = new LabelEntry ;
		pLabel->m_addrLabel = m_pcsxi->m_bufImage.GetLength() ;
		m_ssoaLabel.SetAs( strLabel, pLabel ) ;
	}
	return	errSuccess ;
}

// オペランドアセンブル
//////////////////////////////////////////////////////////////////////////////
SSystem::SError ECSAssembler::AssembleOperand
	( ECSSakura2Assember::Operand& opTerm,
		ECSAssembler::SymbolReference& symRef, SSystem::SStringParser & sparsLine )
{
	opTerm.m_type = Operand::typeInvalid ;
	symRef.m_typeSymbol = symbolInvalid ;
	//
	// レジスタ判定
	//
	int		numReg ;
	SError	err = ParseRegister( numReg, sparsLine ) ;
	if ( err )
	{
		return	err ;
	}
	if ( numReg >= 0 )
	{
		opTerm.m_type = ECSSakura2Assember::Operand::typeRegister ;
		opTerm.m_reg = numReg ;
		return	errSuccess ;
	}
	//
	// メモリ参照判定
	//
	ECSTypeInfo	typeMem ;
	opTerm.m_type = Operand::typeInvalid ;
	err = ParseMemoryOperand( typeMem, opTerm, sparsLine ) ;
	if ( err )
	{
		return	err ;
	}
	if ( opTerm.m_type != Operand::typeInvalid )
	{
		return	errSuccess ;
	}
	//
	// シンボル参照判定
	//
	err = CompileSymbol( opTerm, symRef, sparsLine ) ;
	if ( err )
	{
		return	err ;
	}
	if ( opTerm.m_type != Operand::typeInvalid )
	{
		return	errSuccess ;
	}
	//
	// 定数値判定
	//
	ECSObject *	pValue = NULL ;
	err = CompileImmediate( pValue, sparsLine, L"," ) ;
	if ( err )
	{
		return	err ;
	}
	if ( pValue == NULL )
	{
		m_strErrMsg = L"不正なオペランドです" ;
		return	errFailed ;
	}
	if ( pValue->m_vtType == csvtInteger )
	{
		opTerm.m_type = Operand::typeInteger ;
		opTerm.m_int = ((ECSInteger*)pValue)->GetValue() ;
	}
	else if ( pValue->m_vtType == csvtReal )
	{
		opTerm.m_type = Operand::typeReal ;
		opTerm.m_real = ((ECSReal*)pValue)->m_varReal ;
	}
	else
	{
		delete	pValue ;
		m_strErrMsg = L"不正なオペランドです" ;
		return	errFailed ;
	}
	delete	pValue ;
	return	errSuccess ;
}

// レジスタ判定
//////////////////////////////////////////////////////////////////////////////
SSystem::SError ECSAssembler::ParseRegister
	( int& numReg, SSystem::SStringParser & sparsLine )
{
	numReg = ECSSakura2Assember::ParseRegister( sparsLine ) ;
	if ( numReg >= 0 )
	{
		if ( numReg >= 0x100 )
		{
			m_strErrMsg = L"レジスタ番号が不正です" ;
			return	errFailed ;
		}
		return	errSuccess ;
	}
	SString	strToken ;
	sparsLine.MarkIndex() ;
	sparsLine.NextToken( strToken ) ;
	RegisterAssign *	pRegAssign = m_assignsRegName.GetAs( strToken ) ;
	if ( pRegAssign != NULL )
	{
		numReg = pRegAssign->numReg ;
		if ( sparsLine.HasToComeChar( L"(" ) == L'(' )
		{
			ECSObject *	pValue = NULL ;
			SError	err = CompileImmediate( pValue, sparsLine, L")" ) ;
			if ( err )
			{
				return	err ;
			}
			if ( (pValue == NULL) || (pValue->m_vtType != csvtInteger) )
			{
				delete	pValue ;
				m_strErrMsg = L"レジスタ参照番号が整数ではありません" ;
				return	errFailed ;
			}
			INT64	numRegIndex ;
			pValue->OperateInteger( numRegIndex ) ;
			delete	pValue ;
			//
			if ( sparsLine.HasToComeChar( L")" ) != L')' )
			{
				m_strErrMsg = L"\'(\' に対応する \')\' が見つかりません" ;
				return	errFailed ;
			}
			if ( (numRegIndex < 0)
				|| (numRegIndex >= pRegAssign->numCount) )
			{
				m_strErrMsg = L"レジスタ参照番号が範囲外です" ;
				return	errFailed ;
			}
			numReg += (int) numRegIndex ;
		}
		return	errSuccess ;
	}
	sparsLine.SeekToMark() ;
	numReg = -1 ;
	return	errSuccess ;
}

// メモリオペランド解釈
//////////////////////////////////////////////////////////////////////////////
SSystem::SError ECSAssembler::ParseMemoryOperand
	( ECSTypeInfo& typeMem,
			ECSSakura2Assember::Operand& opTerm,
			SSystem::SStringParser & sparsLine )
{
	//
	// ローカル変数判定
	//
	SSystem::SString			strToken ;
	SStringParser::TokenType	typeToken ;
	sparsLine.MarkIndex() ;
	typeToken = sparsLine.NextToken( strToken ) ;
	if ( m_compiler->IsLocalVariableName( strToken, &typeMem ) >= 0 )
	{
		MemoryOperandFromTypeInfo( opTerm, typeMem ) ;
	}
	else
	{
		sparsLine.SeekToMark() ;
	}
	//
	// メモリ式修飾解釈
	//
	for ( ; ; )
	{
		sparsLine.MarkIndex() ;
		if ( !sparsLine.PassSpace() )
		{
			break ;
		}
		if ( sparsLine.CurrentCharacter() == L'[' )
		{
			//
			// メモリ式 : [expr]
			//
			if ( opTerm.m_type == Operand::typeInvalid )
			{
//				opTerm.m_type = Operand::typeMemory ;
			}
			else if ( opTerm.m_type != Operand::typeMemory )
			{
				m_strErrMsg = L"メモリオペランドの記述が不正です" ;
				return	errFailed ;
			}
			SError	err =
				ParseMemoryExpression( typeMem, opTerm, sparsLine ) ;
			if ( err )
			{
				return	err ;
			}
		}
		else if ( sparsLine.CurrentCharacter() == L'.' )
		{
			//
			// メンバ : <mem-expr>.member
			//
			const ECSClassInfo *	pClassInf = NULL ;
			if ( typeMem.IsTypeArray() )
			{
				pClassInf = m_compiler->GetTypeClassInfo
									( typeMem.GetArrayElementType() ) ;
			}
			else
			{
				pClassInf = m_compiler->GetNakedTypeClassInfo( typeMem ) ;
			}
			if ( pClassInf == NULL )
			{
				m_strErrMsg = L"メンバ参照にクラス情報が見つかりません" ;
				return	errFailed ;
			}
			SString	strMember ;
			sparsLine.GetCharacter() ;
			sparsLine.NextToken( strMember ) ;
			int	iVarIndex = pClassInf->GetVariableIndex( strMember ) ;
			ECSTypeInfo *
				pVarType = pClassInf->GetVariableAt( iVarIndex ) ;
			if ( pVarType == NULL )
			{
				m_strErrMsg = L"\'" ;
				m_strErrMsg += strMember ;
				m_strErrMsg += L"\' は " ;
				m_strErrMsg += pClassInf->GetGlobalName() ;
				m_strErrMsg += L" のメンバではありません" ;
				return	errFailed ;
			}
			switch ( opTerm.m_mem.modeAddr )
			{
			case	ECSSakura2Processor::addrBase:
				opTerm.m_mem.modeAddr = ECSSakura2Processor::addrBaseOffset32 ;
				opTerm.m_mem.offsetAddr = 0 ;
				break ;
			case	ECSSakura2Processor::addrBaseIndex:
				opTerm.m_mem.modeAddr = ECSSakura2Processor::addrBaseIndexOffset32 ;
				opTerm.m_mem.offsetAddr = 0 ;
				break ;
			case	ECSSakura2Processor::addrBaseOffset32:
			case	ECSSakura2Processor::addrBaseIndexOffset32:
				break ;
			}
			opTerm.m_mem.offsetAddr +=
				pClassInf->GetVariableNakedOffsetAt( iVarIndex ) ;
			opTerm.m_mem.typeData = DataTypeFromTypeInfo( *pVarType ) ;
			typeMem = *pVarType ;
		}
		else
		{
			break ;
		}
	}
	if ( opTerm.m_type == Operand::typeMemory )
	{
		switch ( opTerm.m_mem.modeAddr )
		{
		case	ECSSakura2Processor::addrBase:
		case	ECSSakura2Processor::addrBaseIndex:
			break ;
		case	ECSSakura2Processor::addrBaseOffset32:
			if ( opTerm.m_mem.offsetAddr == 0 )
			{
				opTerm.m_mem.modeAddr = ECSSakura2Processor::addrBase ;
			}
			break ;
		case	ECSSakura2Processor::addrBaseIndexOffset32:
			if ( opTerm.m_mem.offsetAddr == 0 )
			{
				opTerm.m_mem.modeAddr = ECSSakura2Processor::addrBaseIndex ;
			}
			break ;
		}
	}
	return	errSuccess ;
}

// メモリ式解釈
//////////////////////////////////////////////////////////////////////////////
SSystem::SError ECSAssembler::ParseMemoryExpression
	( ECSTypeInfo& typeMem,
		ECSSakura2Assember::Operand& opTerm,
		SSystem::SStringParser & sparsLine )
{
	if ( sparsLine.HasToComeChar( L"[" ) != L'[' )
	{
		m_strErrMsg = L"\'[\' が見つかりません" ;
		return	errFailed ;
	}
	if ( opTerm.m_type != Operand::typeMemory )
	{
		opTerm.m_type = Operand::typeMemory ;
		opTerm.m_mem.typeData = ECSSakura2Processor::dataInt64 ;
		opTerm.m_mem.modeAddr = ECSSakura2Processor::addrBase ;
		opTerm.m_mem.regBase = -1 ;
		opTerm.m_mem.regIndex = -1 ;
		opTerm.m_mem.scaleIndex = 0 ;
		opTerm.m_mem.offsetAddr = 0 ;
	}
	for ( ; ; )
	{
		int		numReg ;
		SError	err = ParseRegister( numReg, sparsLine ) ;
		if ( err )
		{
			return	err ;
		}
		if ( numReg >= 0 )
		{
			if ( numReg >= 0x80 )
			{
				m_strErrMsg = L"メモリ参照に使用できないレジスタです" ;
				return	errFailed ;
			}
			if ( sparsLine.HasToComeChar( L"*" ) == L'*' )
			{
				//
				// インデックスレジスタ＋スケール指定
				//
				switch ( opTerm.m_mem.modeAddr )
				{
				case	ECSSakura2Processor::addrBaseIndex:
				case	ECSSakura2Processor::addrBaseIndexOffset32:
					m_strErrMsg = L"インデックスレジスタが二重に指定されています" ;
					return	errFailed ;
				case	ECSSakura2Processor::addrBase:
					opTerm.m_mem.modeAddr = ECSSakura2Processor::addrBaseIndex ;
					break ;
				case	ECSSakura2Processor::addrBaseOffset32:
					opTerm.m_mem.modeAddr = ECSSakura2Processor::addrBaseIndexOffset32 ;
					break ;
				}
				opTerm.m_mem.regIndex = numReg ;
				//
				ECSCompiler::OPERATOR_INFO	opinf ;
				ECSObject*	pValue = NULL ;
				m_compiler->GetOperatorInfo( opinf, L"*" ) ;
				err = CompileImmediate
					( pValue, sparsLine, L"],",
						m_compiler->GetOperatorPriority( opinf ) - 1 ) ;
				if ( err )
				{
					return	err ;
				}
				if ( (pValue == NULL) || (pValue->m_vtType != csvtInteger) )
				{
					delete	pValue ;
					m_strErrMsg = L"インデックススケールが整数ではありません" ;
					return	errFailed ;
				}
				INT64	numScale ;
				pValue->OperateInteger( numScale ) ;
				delete	pValue ;
				int		scaleIndex = 0 ;
				if ( numScale != 0 )
				{
					while ( !(numScale & 0x01) && (scaleIndex < 8) )
					{
						numScale >>= 1 ;
						scaleIndex ++ ;
					}
				}
				int		scaleLimit = 3 ;
				if ( opTerm.m_mem.regBase == ECSSakura2Processor::regBP )
				{
					scaleLimit = 7 ;
				}
				if ( (numScale != 1) || (scaleIndex > scaleLimit) )
				{
					m_strErrMsg = L"インデックススケールが不正です" ;
					return	errFailed ;
				}
				opTerm.m_mem.scaleIndex = scaleIndex ;
			}
			else
			{
				//
				// ベース又はインデックスレジスタ指定
				//
				bool	fBaseAddr = false ;
				if ( numReg >= 0x10 )
				{
					switch ( opTerm.m_mem.modeAddr )
					{
					case	ECSSakura2Processor::addrBaseIndex:
					case	ECSSakura2Processor::addrBaseIndexOffset32:
						m_strErrMsg = L"インデックスレジスタが二重に指定されています" ;
						return	errFailed ;
					case	ECSSakura2Processor::addrBase:
						opTerm.m_mem.modeAddr = ECSSakura2Processor::addrBaseIndex ;
						break ;
					case	ECSSakura2Processor::addrBaseOffset32:
						opTerm.m_mem.modeAddr = ECSSakura2Processor::addrBaseIndexOffset32 ;
						break ;
					}
					if ( opTerm.m_mem.regBase < 0 )
					{
						opTerm.m_mem.regBase = ECSSakura2Processor::regZeroPtr ;
						fBaseAddr = true ;
					}
					opTerm.m_mem.regIndex = numReg ;
					opTerm.m_mem.scaleIndex = 0 ;
				}
				else
				{
					switch ( opTerm.m_mem.modeAddr )
					{
					case	ECSSakura2Processor::addrBaseIndex:
					case	ECSSakura2Processor::addrBaseIndexOffset32:
						m_strErrMsg = L"インデックスレジスタが二重に指定されています" ;
						return	errFailed ;
					case	ECSSakura2Processor::addrBase:
					case	ECSSakura2Processor::addrBaseOffset32:
						if ( opTerm.m_mem.regBase >= 0 )
						{
							opTerm.m_mem.modeAddr =
								(opTerm.m_mem.modeAddr == ECSSakura2Processor::addrBase)
									? ECSSakura2Processor::addrBaseIndex 
										: ECSSakura2Processor::addrBaseIndexOffset32 ;
							opTerm.m_mem.regIndex = numReg ;
							opTerm.m_mem.scaleIndex = 0 ;
						}
						else
						{
							opTerm.m_mem.regBase = numReg ;
							fBaseAddr = true ;
						}
						break ;
					}
				}
				if ( fBaseAddr )
				{
					if ( numReg == ECSSakura2Processor::regTP )
					{
						ECSClassInfo *	pThisClass =
								m_compiler->GetCurrentThisClass() ;
						if ( pThisClass != NULL )
						{
							typeMem = ECSTypeInfo
								( new ECSStructure(pThisClass) ) ;
						}
					}
					else
					{
						ECSTypeInfo *
							pRegType = m_assignsRegType.GetAt( numReg ) ;
						if ( pRegType != NULL )
						{
							typeMem.MakeNakedPointerOf( *pRegType ) ;
							opTerm.m_mem.typeData = DataTypeFromTypeInfo( typeMem ) ;
						}
					}
				}
			}
		}
		else
		{
			ECSObject*	pValue = NULL ;
			err = CompileImmediate
				( pValue, sparsLine, L"],", ECSCompiler::oppAdd ) ;
			if ( err )
			{
				return	err ;
			}
			if ( (pValue == NULL) || (pValue->m_vtType != csvtInteger) )
			{
				delete	pValue ;
				m_strErrMsg = L"アドレスオフセットが整数ではありません" ;
				return	errFailed ;
			}
			INT64	numOffset ;
			pValue->OperateInteger( numOffset ) ;
			delete	pValue ;
			if ( (numOffset < -(INT64)0x80000000UL)
						|| (numOffset > 0x7FFFFFFF) )
			{
				delete	pValue ;
				m_strErrMsg = L"アドレスオフセットが範囲外です" ;
				return	errFailed ;
			}
			switch ( opTerm.m_mem.modeAddr )
			{
			case	ECSSakura2Processor::addrBase:
				opTerm.m_mem.modeAddr = ECSSakura2Processor::addrBaseOffset32 ;
				opTerm.m_mem.offsetAddr = 0 ;
				break ;
			case	ECSSakura2Processor::addrBaseIndex:
				opTerm.m_mem.modeAddr = ECSSakura2Processor::addrBaseIndexOffset32 ;
				opTerm.m_mem.offsetAddr = 0 ;
				break ;
			case	ECSSakura2Processor::addrBaseOffset32:
			case	ECSSakura2Processor::addrBaseIndexOffset32:
				break ;
			}
			opTerm.m_mem.offsetAddr += (int32_t) numOffset ;
		}
		if ( sparsLine.HasToComeChar( L"+" ) == L'+' )
		{
		}
		else if ( sparsLine.HasToComeChar( L"]" ) == L']' )
		{
			break ;
		}
		else
		{
			sparsLine.PassSpace() ;
			if ( sparsLine.CurrentCharacter() != L'-' )
			{
				m_strErrMsg = L"アドレス式が不正です" ;
				return	errFailed ;
			}
		}
	}
	if ( opTerm.m_mem.regBase < 0 )
	{
		opTerm.m_mem.regBase = regZeroPtr ;
	}
	else if ( opTerm.m_mem.regBase >= 0x10 )
	{
		m_strErrMsg = L"ベースレジスタが不正です" ;
		return	errFailed ;
	}
	return	errSuccess ;
}

// 変数型情報からメモリオペランド情報へ変換
//////////////////////////////////////////////////////////////////////////////
void ECSAssembler::MemoryOperandFromTypeInfo
	( ECSSakura2Assember::Operand& opTerm, const ECSTypeInfo& typeMem )
{
	opTerm.m_type = Operand::typeMemory ;
	opTerm.m_mem.typeData = DataTypeFromTypeInfo( typeMem ) ;
	//
	if ( typeMem.IsAddressingInfo() )
	{
		opTerm.m_mem.regBase = typeMem.m_regBase ;
		opTerm.m_mem.regIndex = typeMem.m_regIndex ;
		opTerm.m_mem.scaleIndex = typeMem.m_scaleIndex ;
		opTerm.m_mem.offsetAddr = typeMem.m_addrOffset ;
		//
		if ( opTerm.m_mem.regIndex >= 0 )
		{
			if ( opTerm.m_mem.offsetAddr != 0 )
			{
				opTerm.m_mem.modeAddr =
					ECSSakura2Processor::addrBaseIndexOffset32 ;
			}
			else
			{
				opTerm.m_mem.modeAddr =
					ECSSakura2Processor::addrBaseIndex ;
			}
		}
		else
		{
			if ( opTerm.m_mem.offsetAddr != 0 )
			{
				opTerm.m_mem.modeAddr =
					ECSSakura2Processor::addrBaseOffset32 ;
			}
			else
			{
				opTerm.m_mem.modeAddr =
					ECSSakura2Processor::addrBase ;
			}
		}
	}
}

// 変数型情報からメモリアクセスデータ型へ変換
//////////////////////////////////////////////////////////////////////////////
ECSSakura2Processor::DataType
	ECSAssembler::DataTypeFromTypeInfo( const ECSTypeInfo& typeMem )
{
	CSVariableType	csvtType ;
	if ( typeMem.IsTypeArray() )
	{
		csvtType = ECSTypeInfo::GetNakedMemoryType
							( typeMem.GetArrayElementType() ) ;
	}
	else
	{
		csvtType = typeMem.GetNakedMemoryType() ;
	}
	switch ( csvtType )
	{
	case	csvtInteger:
	case	csvtInteger64:
	case	csvtReal:
	case	csvtReal64:
	default:
		break ;
	case	csvtBoolean:
	case	csvtInt8:
		return	ECSSakura2Processor::dataInt8 ;
	case	csvtUint8:
		return	ECSSakura2Processor::dataUint8 ;
	case	csvtInt16:
		return	ECSSakura2Processor::dataInt16 ;
	case	csvtUint16:
		return	ECSSakura2Processor::dataUint16 ;
	case	csvtInt32:
		return	ECSSakura2Processor::dataInt32 ;
	case	csvtUint32:
		return	ECSSakura2Processor::dataUint32 ;
	case	csvtReal32:
		return	ECSSakura2Processor::dataFloat ;
	}
	return	ECSSakura2Processor::dataInt64 ;
}

// シンボル解釈
//////////////////////////////////////////////////////////////////////////////
SSystem::SError ECSAssembler::CompileSymbol
	( ECSSakura2Assember::Operand& opTerm,
		SymbolReference& symRef, SSystem::SStringParser & sparsLine )
{
	opTerm.m_type = Operand::typeInvalid ;
	//
	SString	strSymbol ;
	sparsLine.MarkIndex() ;
	sparsLine.NextToken( strSymbol ) ;
	for ( ; ; )
	{
		int	nClassIndex = m_pcsxi->GetClassInfoIndex( strSymbol ) ;
		if ( nClassIndex < 0 )
		{
			break ;
		}
		if ( sparsLine.HasToComeToken( L"::" ) )
		{
			SString	strMember ;
			sparsLine.NextToken( strMember ) ;
			//
			ECSClassInfo *	pClassInf =
						m_pcsxi->GetClassInfoAt( nClassIndex ) ;
			if ( pClassInf != NULL )
			{
				int	iFunc = pClassInf->FindFunctionAs( strMember ) ;
				if ( iFunc >= 0 )
				{
					ECSPrototypeInfo *
						pFunc = pClassInf->GetFunctionAt( iFunc ) ;
					if ( pFunc != NULL )
					{
						// メンバ関数
						opTerm.m_type = Operand::typeAddress ;
						opTerm.m_int = 0 ;
						symRef.m_strSymbol = pFunc->GetGlobalName() ;
						if ( pFunc->GetAttribute()
									& ECSTypeInfo::flagNativeObject )
						{
							opTerm.m_int =
								m_pcsxi->MakeNakedNativeFunctionIndex
													( symRef.m_strSymbol ) ;
							symRef.m_typeSymbol = symbolSysCall ;
						}
						else
						{
							symRef.m_typeSymbol = symbolFunction ;
						}
						symRef.m_pProto = pFunc ;
						return	errSuccess ;
					}
				}
			}
			strSymbol += L"::" ;
			strSymbol += strMember ;
		}
		else
		{
			// クラス ID
			opTerm.m_type = Operand::typeAddress ;
			opTerm.m_int = nClassIndex ;
			symRef.m_strSymbol = strSymbol ;
			symRef.m_typeSymbol = symbolClass ;
			return	errSuccess ;
		}
	}
	ECSPrototypeInfo *	pProto =
			m_compiler->m_wstaNakedPrototype.GetAs( strSymbol ) ;
	if ( pProto != NULL )
	{
		// 関数
		opTerm.m_type = Operand::typeAddress ;
		opTerm.m_int = 0 ;
		symRef.m_strSymbol = pProto->GetGlobalName() ;
		if ( pProto->GetAttribute() & ECSTypeInfo::flagNativeObject )
		{
			opTerm.m_int =
				m_pcsxi->MakeNakedNativeFunctionIndex( symRef.m_strSymbol ) ;
			symRef.m_typeSymbol = symbolSysCall ;
		}
		else
		{
			symRef.m_typeSymbol = symbolFunction ;
		}
		symRef.m_pProto = pProto ;
		return	errSuccess ;
	}
	if ( m_ssoaLabel.GetAs( strSymbol ) != NULL )
	{
		// ラベル
		opTerm.m_type = Operand::typeLabel ;
		opTerm.m_int = 0 ;
		symRef.m_strSymbol = strSymbol ;
		symRef.m_typeSymbol = symbolLocalLabel ;
		return	errSuccess ;
	}
	SString	strGlobalVarName =
		m_compiler->TranslateGlobalVariableName( strSymbol ) ;
	ECSExecutionImage::NAKED_SYMBOL_INFO *
		pSymInf = m_pcsxi->m_wstaSymbols.GetAs( strGlobalVarName ) ;
	if ( pSymInf != NULL )
	{
		// 変数アドレス
		opTerm.m_int = pSymInf->nAddress ;
		symRef.m_strSymbol = strGlobalVarName ;
		//
		switch ( (int) (pSymInf->nAddress >> 56) & 0xFF )
		{
		case	ECSSakura2::VirtualMachine::roasNakedGlobal:
			opTerm.m_type = Operand::typeAddress ;
			symRef.m_typeSymbol = symbolGlobal ;
			return	errSuccess ;
		case	ECSSakura2::VirtualMachine::roasNakedConst:
			opTerm.m_type = Operand::typeAddress ;
			symRef.m_typeSymbol = symbolConst ;
			return	errSuccess ;
		case	ECSSakura2::VirtualMachine::roasNakedShared:
			opTerm.m_type = Operand::typeAddress ;
			symRef.m_typeSymbol = symbolShared ;
			return	errSuccess ;
		}
	}
	sparsLine.SeekToMark() ;
	//
	return	errSuccess ;
}

// 即値解釈
//////////////////////////////////////////////////////////////////////////////
SSystem::SError ECSAssembler::CompileImmediate
	( ECSObject*& pValue, SSystem::SStringParser & sparsLine,
		const wchar_t * pwszExit, int nPriority )
{
	ECSSourceStream	cssLine =
		sparsLine.SubString
			( sparsLine.GetIndex(),
				sparsLine.GetLength() - sparsLine.GetIndex() ) ;
	ESLError	err =
		m_compiler->CalculateExpression
					( pValue, cssLine, nPriority, pwszExit ) ;
	if ( err )
	{
		m_strErrMsg = GetESLErrorMsg( err ) ;
		return	errFailed ;
	}
	sparsLine.SeekIndex( sparsLine.GetIndex() + cssLine.GetIndex() ) ;
	return	errSuccess ;
}

// ニーモニックアセンブル
//////////////////////////////////////////////////////////////////////////////
SError ECSAssembler::AssembleMnemonic
	( const SString& strMnemonic, SSystem::SStringParser & sparsLine )
{
	//
	// オペランド解釈
	//
	SError	err ;
	SArray<ECSSakura2Assember::Operand>	aryOperands ;
	SObjectArray<SymbolReference>		arySymbols ;
	size_t	numOperands = 0 ;
	while ( sparsLine.PassSpace() )
	{
		SymbolReference *	pSymbol = new SymbolReference ;
		arySymbols.Add( pSymbol ) ;
		//
		aryOperands.SetLength( ++ numOperands ) ;
		err = AssembleOperand
			( aryOperands.At(numOperands - 1), *pSymbol, sparsLine ) ;
		if ( err )
		{
			return	err ;
		}
		if ( sparsLine.HasToComeChar( L"," ) != L',' )
		{
			if ( sparsLine.PassSpace() )
			{
				m_strErrMsg = L"オペランドの区切りが不正です" ;
				return	errFailed ;
			}
			break ;
		}
	}
	//
	// ニーモニック解釈
	//
	InstructionBuffer	ibuf ;
	err = AssembleInstruction
		( ibuf, m_strErrMsg, strMnemonic, aryOperands, numOperands ) ;
	if ( err )
	{
		return	err ;
	}
	if ( ibuf.bufCode[0] == codeReturn )
	{
		if ( m_compiler->CountOfNakedFunctionDestruction() > 0 )
		{
			m_strErrMsg = L"ローカル変数のデストラクタが呼び出されません" ;
			return	errFailed ;
		}
		bool	fThis = (m_compiler->GetCurrentThisClass() != NULL)
						&& (m_compiler->IsLocalVariableName(L"this") == 0) ;
		m_pcsxi->WriteSakuraMoveRegReg
			( ECSSakura2Processor::regSP, ECSSakura2Processor::regBP ) ;
		if ( fThis )
		{
			m_pcsxi->WriteSakuraPopRegsImm8( ECSSakura2Processor::regBP, 2 ) ;
		}
		else
		{
			m_pcsxi->WriteSakuraPopReg( ECSSakura2Processor::regBP ) ;
		}
	}
	//
	// コードバッファへ出力
	//
	DWORD	dwCodeAddr = m_pcsxi->m_bufImage.GetLength() ;
	m_pcsxi->WriteCodeData( ibuf.bufCode, ibuf.nTotalBytes ) ;
	//
	for ( size_t i = 0; i < numOperands; i ++ )
	{
		SymbolReference *	pSymbol = arySymbols.GetAt( i ) ;
		if ( pSymbol == NULL )
		{
			continue ;
		}
		if ( pSymbol->m_typeSymbol == symbolInvalid )
		{
			continue ;
		}
		size_t	nRefDataBytes = 4 ;
		switch ( pSymbol->m_typeSymbol )
		{
		case	symbolLocalLabel:
			{
				LabelEntry *	pLabel =
					m_ssoaLabel.GetAs( pSymbol->m_strSymbol ) ;
				if ( pLabel == NULL )
				{
					pLabel = new LabelEntry ;
					m_ssoaLabel.SetAs( pSymbol->m_strSymbol, pLabel ) ;
				}
				pLabel->Add( dwCodeAddr + ibuf.nCodeBytes ) ;
			}
			break ;
		case	symbolFunction:
			nRefDataBytes = 8 ;
			m_pcsxi->AddCodeRefFunctionAddress64
				( pSymbol->m_strSymbol, dwCodeAddr + ibuf.nCodeBytes ) ;
			break ;
		case	symbolGlobal:
			nRefDataBytes = 8 ;
			m_pcsxi->AddCodeRefNakedGlobalAddress
				( pSymbol->m_strSymbol, dwCodeAddr + ibuf.nCodeBytes ) ;
			break ;
		case	symbolConst:
			nRefDataBytes = 8 ;
			m_pcsxi->AddCodeRefNakedConstAddress
				( pSymbol->m_strSymbol, dwCodeAddr + ibuf.nCodeBytes ) ;
			break ;
		case	symbolShared:
			nRefDataBytes = 8 ;
			m_pcsxi->AddCodeRefNakedSharedAddress
				( pSymbol->m_strSymbol, dwCodeAddr + ibuf.nCodeBytes ) ;
			break ;
		case	symbolSysCall:
			nRefDataBytes = 4 ;
			m_pcsxi->AddCodeRefNakedSystemCallID( dwCodeAddr + ibuf.nCodeBytes ) ;
			break ;
		case	symbolClass:
			nRefDataBytes = 4 ;
			m_pcsxi->AddCodeRefClassID( dwCodeAddr + ibuf.nCodeBytes ) ;
			break ;
		}
		if ( ibuf.nTotalBytes < ibuf.nCodeBytes + nRefDataBytes )
		{
			m_strErrMsg =
				L"オペランドに指定出来ないシンボル参照を含んでいます" ;
			return	errFailed ;
		}
	}
	return	errSuccess ;
}

// ラベル相対ジャンプ確定
//////////////////////////////////////////////////////////////////////////////
void ECSAssembler::CommitLabelReference( const LabelEntry& label )
{
	const size_t	nCount = label.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		uint32_t	addrRef = label.At( i ) ;
		DWORD *	pdwRef = (DWORD*)
			m_pcsxi->m_bufImage.ModifyBuffer( addrRef, sizeof(DWORD) ) ;
		if ( pdwRef != NULL )
		{
			*pdwRef = label.m_addrLabel - (addrRef + sizeof(DWORD)) ;
		}
	}
}

// ディレクティブネスト検索
//////////////////////////////////////////////////////////////////////////////
ECSAssembler::DirectiveNest *
	ECSAssembler::FindDirectiveNest
			( int dirFirst, int dirEnd, int iNest ) const
{
	const size_t	countNest = m_nestDirective.GetLength() ;
	for ( size_t i = iNest; i < countNest; i ++ )
	{
		DirectiveNest *	pNest = m_nestDirective.GetLastAt( i ) ;
		if ( pNest == NULL )
		{
			continue ;
		}
		if ( (pNest->m_typeDirective >= dirFirst)
					&& (pNest->m_typeDirective <= dirEnd) )
		{
			return	pNest ;
		}
	}
	return	NULL ;
}

// .IF <runtime-conditional-expression>
//////////////////////////////////////////////////////////////////////////////
SSystem::SError ECSAssembler::AssembleIf( SSystem::SStringParser & sparsLine )
{
	DirectiveNest *	pdnIf = new DirectiveNest ;
	pdnIf->m_typeDirective = dirIf ;
	m_nestDirective.Add( pdnIf ) ;
	//
	FreeAllTemporaryRegister() ;
	//
	LabelEntry	labelTrue ;
	SError	err =
		CompileRuntimeConditionalExpression
			( pdnIf->m_labelElse, labelTrue, false, sparsLine ) ;
	if ( err )
	{
		return	err ;
	}
	labelTrue.m_addrLabel = m_pcsxi->m_bufImage.GetLength() ;
	CommitLabelReference( labelTrue ) ;
	//
	return	errSuccess ;
}

// .ELSEIF <runtime-conditional-expression>
//////////////////////////////////////////////////////////////////////////////
SSystem::SError ECSAssembler::AssembleElseIf( SSystem::SStringParser & sparsLine )
{
	//
	// 直前 .IF ブロック終了
	//
	DirectiveNest *	pdnIf = m_nestDirective.GetLastAt( 0 ) ;
	if ( (pdnIf == NULL)
		|| ((pdnIf->m_typeDirective != dirIf)
				&& (pdnIf->m_typeDirective != dirElseIf)) )
	{
		m_strErrMsg = L".ELSEIF が .IF と対応していません" ;
		return	errFailed ;
	}
	pdnIf->m_labelBreak.Add( m_pcsxi->WriteSakuraJumpOffset32(0) ) ;
	//
	pdnIf->m_labelElse.m_addrLabel = m_pcsxi->m_bufImage.GetLength() ;
	CommitLabelReference( pdnIf->m_labelElse ) ;
	pdnIf->m_labelElse.RemoveAll() ;
	//
	// 条件式
	//
	pdnIf->m_typeDirective = dirElseIf ;
	//
	FreeAllTemporaryRegister() ;
	//
	LabelEntry	labelTrue ;
	SError	err =
		CompileRuntimeConditionalExpression
			( pdnIf->m_labelElse, labelTrue, false, sparsLine ) ;
	if ( err )
	{
		return	err ;
	}
	labelTrue.m_addrLabel = m_pcsxi->m_bufImage.GetLength() ;
	//
	CommitLabelReference( labelTrue ) ;
	//
	return	errSuccess ;
}

// .ELSE
//////////////////////////////////////////////////////////////////////////////
SSystem::SError ECSAssembler::AssembleElse( SSystem::SStringParser & sparsLine )
{
	//
	// 直前 .IF ブロック終了
	//
	DirectiveNest *	pdnIf = m_nestDirective.GetLastAt( 0 ) ;
	if ( (pdnIf == NULL)
		|| ((pdnIf->m_typeDirective != dirIf)
				&& (pdnIf->m_typeDirective != dirElseIf)) )
	{
		m_strErrMsg = L".ELSE が .IF と対応していません" ;
		return	errFailed ;
	}
	pdnIf->m_labelBreak.Add( m_pcsxi->WriteSakuraJumpOffset32(0) ) ;
	//
	pdnIf->m_labelElse.m_addrLabel = m_pcsxi->m_bufImage.GetLength() ;
	CommitLabelReference( pdnIf->m_labelElse ) ;
	pdnIf->m_labelElse.RemoveAll() ;
	pdnIf->m_typeDirective = dirElse ;
	//
	return	errSuccess ;
}

// .ENDIF
//////////////////////////////////////////////////////////////////////////////
SSystem::SError ECSAssembler::AssembleEndIf( SSystem::SStringParser & sparsLine )
{
	DirectiveNest *	pdnIf = m_nestDirective.GetLastAt( 0 ) ;
	if ( (pdnIf == NULL)
		|| ((pdnIf->m_typeDirective != dirIf)
				&& (pdnIf->m_typeDirective != dirElseIf)
				&& (pdnIf->m_typeDirective != dirElse)) )
	{
		m_strErrMsg = L".ENDIF が .IF と対応していません" ;
		return	errFailed ;
	}
	if ( (pdnIf->m_typeDirective == dirIf)
		|| (pdnIf->m_typeDirective == dirElseIf) )
	{
		pdnIf->m_labelElse.m_addrLabel = m_pcsxi->m_bufImage.GetLength() ;
		CommitLabelReference( pdnIf->m_labelElse ) ;
		pdnIf->m_labelElse.RemoveAll() ;
	}
	pdnIf->m_labelBreak.m_addrLabel = m_pcsxi->m_bufImage.GetLength() ;
	CommitLabelReference( pdnIf->m_labelBreak ) ;
	//
	delete	m_nestDirective.Pop() ;
	//
	return	errSuccess ;
}

// .WHILE <runtime-conditional-expression>
//////////////////////////////////////////////////////////////////////////////
SSystem::SError ECSAssembler::AssembleWhile( SSystem::SStringParser & sparsLine )
{
	DirectiveNest *	pdnWhile = new DirectiveNest ;
	pdnWhile->m_typeDirective = dirWhile ;
	m_nestDirective.Add( pdnWhile ) ;
	//
	pdnWhile->m_labelContinue.m_addrLabel = m_pcsxi->m_bufImage.GetLength() ;
	//
	FreeAllTemporaryRegister() ;
	//
	LabelEntry	labelTrue ;
	SError	err =
		CompileRuntimeConditionalExpression
			( pdnWhile->m_labelBreak, labelTrue, false, sparsLine ) ;
	if ( err )
	{
		return	err ;
	}
	labelTrue.m_addrLabel = m_pcsxi->m_bufImage.GetLength() ;
	CommitLabelReference( labelTrue ) ;
	//
	return	errSuccess ;
}

// .ENDW
//////////////////////////////////////////////////////////////////////////////
SSystem::SError ECSAssembler::AssembleEndWhile( SSystem::SStringParser & sparsLine )
{
	DirectiveNest *	pdnWhile = m_nestDirective.GetLastAt( 0 ) ;
	if ( (pdnWhile == NULL)
		|| (pdnWhile->m_typeDirective != dirWhile) )
	{
		m_strErrMsg = L".ENDW が .WHILE と対応していません" ;
		return	errFailed ;
	}
	pdnWhile->m_labelContinue.Add( m_pcsxi->WriteSakuraJumpOffset32(0) ) ;
	pdnWhile->m_labelBreak.m_addrLabel = m_pcsxi->m_bufImage.GetLength() ;
	CommitLabelReference( pdnWhile->m_labelBreak ) ;
	CommitLabelReference( pdnWhile->m_labelContinue ) ;
	//
	delete	m_nestDirective.Pop() ;
	//
	return	errSuccess ;
}

// .REPEAT
//////////////////////////////////////////////////////////////////////////////
SSystem::SError ECSAssembler::AssembleRepeat( SSystem::SStringParser & sparsLine )
{
	DirectiveNest *	pdnRepeat = new DirectiveNest ;
	pdnRepeat->m_typeDirective = dirRepeat ;
	m_nestDirective.Add( pdnRepeat ) ;
	//
	pdnRepeat->m_labelContinue.m_addrLabel = m_pcsxi->m_bufImage.GetLength() ;
	//
	return	errSuccess ;
}

// .UNTIL <runtime-conditional-expression>
//////////////////////////////////////////////////////////////////////////////
SSystem::SError ECSAssembler::AssembleUntil( SSystem::SStringParser & sparsLine )
{
	DirectiveNest *	pdnRepeat = m_nestDirective.GetLastAt( 0 ) ;
	if ( (pdnRepeat == NULL)
		|| (pdnRepeat->m_typeDirective != dirRepeat) )
	{
		m_strErrMsg = L".UNTIL が .REPEAT と対応していません" ;
		return	errFailed ;
	}
	FreeAllTemporaryRegister() ;
	//
	SError	err =
		CompileRuntimeConditionalExpression
			( pdnRepeat->m_labelContinue,
				pdnRepeat->m_labelBreak, false, sparsLine ) ;
	if ( err )
	{
		return	err ;
	}
	pdnRepeat->m_labelBreak.m_addrLabel = m_pcsxi->m_bufImage.GetLength() ;
	CommitLabelReference( pdnRepeat->m_labelBreak ) ;
	CommitLabelReference( pdnRepeat->m_labelContinue ) ;
	//
	delete	m_nestDirective.Pop() ;
	//
	return	errSuccess ;
}

// .BREAK [.IF <runtime-conditional-expression>]
//////////////////////////////////////////////////////////////////////////////
SSystem::SError ECSAssembler::AssembleBreak( SSystem::SStringParser & sparsLine )
{
	DirectiveNest *	pdnNest = FindDirectiveNest( dirWhile, dirUntil ) ;
	if ( pdnNest == NULL )
	{
		m_strErrMsg = L".BREAK がループブロック内ではありません" ;
		return	errFailed ;
	}
	SString	strIf ;
	sparsLine.MarkIndex() ;
	sparsLine.NextString( strIf ) ;
	if ( strIf.CompareNoCase( L".IF" ) == 0 )
	{
		FreeAllTemporaryRegister() ;
		//
		LabelEntry	labelFalse ;
		SError	err =
			CompileRuntimeConditionalExpression
				( pdnNest->m_labelBreak, labelFalse, true, sparsLine ) ;
		if ( err )
		{
			return	err ;
		}
		labelFalse.m_addrLabel = m_pcsxi->m_bufImage.GetLength() ;
		CommitLabelReference( labelFalse ) ;
	}
	else
	{
		sparsLine.SeekToMark() ;
		pdnNest->m_labelBreak.Add( m_pcsxi->WriteSakuraJumpOffset32(0) ) ;
	}
	return	errSuccess ;
}

// .CONTINUE [.IF <runtime-conditional-expression>]
//////////////////////////////////////////////////////////////////////////////
SSystem::SError ECSAssembler::AssembleContinue( SSystem::SStringParser & sparsLine )
{
	DirectiveNest *	pdnNest = FindDirectiveNest( dirWhile, dirUntil ) ;
	if ( pdnNest == NULL )
	{
		m_strErrMsg = L".CONTINUE がループブロック内ではありません" ;
		return	errFailed ;
	}
	SString	strIf ;
	sparsLine.MarkIndex() ;
	sparsLine.NextString( strIf ) ;
	if ( strIf.CompareNoCase( L".IF" ) == 0 )
	{
		FreeAllTemporaryRegister() ;
		//
		LabelEntry	labelFalse ;
		SError	err =
			CompileRuntimeConditionalExpression
				( pdnNest->m_labelContinue, labelFalse, true, sparsLine ) ;
		if ( err )
		{
			return	err ;
		}
		labelFalse.m_addrLabel = m_pcsxi->m_bufImage.GetLength() ;
		CommitLabelReference( labelFalse ) ;
	}
	else
	{
		sparsLine.SeekToMark() ;
		pdnNest->m_labelContinue.Add( m_pcsxi->WriteSakuraJumpOffset32(0) ) ;
	}
	return	errSuccess ;
}

// REG LOAD <name> [, <register>] [: <memory-operand>]
// REG ALLOC <name>[(<count>)] [, <register>] : <type-expression>
// REG FREE <name> [, ...]
// REG RELOAD <name> [, ...]
// REG FLUSH <name> [, ...]
//////////////////////////////////////////////////////////////////////////////
SSystem::SError ECSAssembler::AssembleRegister( SSystem::SStringParser & sparsLine )
{
	SString	strCommand ;
	sparsLine.NextToken( strCommand ) ;
	if ( (strCommand.CompareNoCase( L"LOAD" ) == 0)
		|| (strCommand.CompareNoCase( L"ALLOC" ) == 0) )
	{
		//
		// 定義名解釈
		//
		SString	strName ;
		SStringParser::TokenType
				typeToken = sparsLine.NextToken( strName ) ;
		if ( typeToken != SStringParser::tokenNormal )
		{
			m_strErrMsg = L"REG 文の定義名が不正です" ;
			return	errFailed ;
		}
		//
		// 割り当てレジスタ数解釈
		//
		int	numCount = 1 ;
		if ( sparsLine.HasToComeChar( L"(" ) == L'(' )
		{
			ECSObject *	pValue = NULL ;
			SError	err = CompileImmediate( pValue, sparsLine, L")" ) ;
			if ( err )
			{
				return	err ;
			}
			if ( (pValue == NULL) || (pValue->m_vtType != csvtInteger) )
			{
				delete	pValue ;
				m_strErrMsg = L"REG 文での確保数が整数でありません" ;
				return	errFailed ;
			}
			numCount = (int) ((ECSInteger*)pValue)->GetValue() ;
			delete	pValue ;
			if ( (numCount < 0) || (numCount >= 100) )
			{
				m_strErrMsg = L"REG 文での確保数が範囲外です" ;
				return	errFailed ;
			}
			if ( sparsLine.HasToComeChar( L")" ) != L')' )
			{
				m_strErrMsg = L"REG 文で \'(\' に対応する \')\' が見つかりません" ;
				return	errFailed ;
			}
		}
		//
		// 割り当てレジスタ解釈
		//
		int	numReg = -1 ;
		if ( sparsLine.HasToComeChar( L"," ) == L',' )
		{
			numReg = ECSSakura2Assember::ParseRegister( sparsLine ) ;
			if ( (numReg < 0) || (numReg >= 112) )
			{
				m_strErrMsg = L"REG 文のレジスタ番号の指定が不正です" ;
				return	errFailed ;
			}
			RegisterAssign *	pRegAssign = m_assignsRegName.GetAs( strName ) ;
			if ( pRegAssign == NULL )
			{
				for ( int j = 0; j < numCount; j ++ )
				{
					if ( m_assignsRegType.GetAt(numReg+j) != NULL )
					{
						m_strErrMsg = L"レジスタ番号が二重定義になります" ;
						return	errFailed ;
					}
				}
			}
			else
			{
				if( (pRegAssign->numReg != numReg)
						|| (pRegAssign->numCount != numCount) )
				{
					m_strErrMsg = L"REG 文のレジスタ番号が再定義されています" ;
					return	errFailed ;
				}
			}
		}
		else
		{
			RegisterAssign *	pRegAssign = m_assignsRegName.GetAs( strName ) ;
			if ( pRegAssign == NULL )
			{
				int	stepCount = 1 ;
				if ( numCount >= 2 )
				{
					stepCount = 2 ;
				}
				for ( int i = 16; i < 112; i += stepCount )
				{
					numReg = i ;
					for ( int j = 0; j < numCount; j ++ )
					{
						if ( m_assignsRegType.GetAt(i+j) != NULL )
						{
							numReg = -1 ;
							break ;
						}
					}
					if ( numReg >= 0 )
					{
						break ;
					}
				}
				if ( numReg < 0 )
				{
					m_strErrMsg = L"REG 文でレジスタを割り当てられませんでした" ;
					return	errFailed ;
				}
			}
			else
			{
				numReg = pRegAssign->numReg ;
			}
		}
		if ( strCommand.CompareNoCase( L"LOAD" ) == 0 )
		{
			//
			// メモリオペランド解釈
			//
			ECSTypeInfo	typeMem ;
			Operand		opTerm[2] ;
			SError		err ;
			if ( sparsLine.HasToComeChar( L":" ) == L':' )
			{
				err = ParseMemoryOperand( typeMem, opTerm[1], sparsLine ) ;
			}
			else
			{
				SStringParser	sparsTemp ;
				sparsTemp.AttachString
						( strName.GetConstArray(), strName.GetLength() ) ;
				err = ParseMemoryOperand( typeMem, opTerm[1], sparsTemp ) ;
			}
			if ( err )
			{
				return	err ;
			}
			if ( opTerm[1].m_type != Operand::typeMemory )
			{
				m_strErrMsg = L"REG LOAD 文にメモリオペランドが指定されていません" ;
				return	errFailed ;
			}
			//
			// レジスタ割り当て定義
			//
			ECSTypeInfo *	pRegType = new ECSTypeInfo( typeMem ) ;
			switch ( opTerm[1].m_mem.modeAddr )
			{
			case	addrBase:
				pRegType->SetAddressingInfo
					( opTerm[1].m_mem.regBase, 0 ) ;
				break ;
			case	addrBaseOffset32:
				pRegType->SetAddressingInfo
					( opTerm[1].m_mem.regBase,
						opTerm[1].m_mem.offsetAddr ) ;
				break ;
			case	addrBaseIndex:
				pRegType->SetAddressingInfo
					( opTerm[1].m_mem.regBase, 0,
						opTerm[1].m_mem.regIndex,
						opTerm[1].m_mem.scaleIndex ) ;
				break ;
			case	addrBaseIndexOffset32:
			default:
				pRegType->SetAddressingInfo
					( opTerm[1].m_mem.regBase,
						opTerm[1].m_mem.offsetAddr,
						opTerm[1].m_mem.regIndex,
						opTerm[1].m_mem.scaleIndex ) ;
				break ;
			}
			RegisterAssign *	pRegAssign = new RegisterAssign ;
			pRegAssign->numReg = numReg ;
			pRegAssign->numCount = 1 ;
			m_assignsRegName.SetAs( strName, pRegAssign ) ;
			m_assignsRegType.SetAt( numReg, pRegType ) ;
			//
			// メモリから読み込み
			//
			opTerm[0].m_type = Operand::typeRegister ;
			opTerm[0].m_reg = numReg ;
			//
			InstructionBuffer	ibuf ;
			err = AssembleInstructionMove
				( ibuf, m_strErrMsg, L"load", codeLoadMem, opTerm, 2 ) ;
			if ( err )
			{
				return	err ;
			}
			m_pcsxi->WriteCodeData( &ibuf.bufCode[0], ibuf.nTotalBytes ) ;
		}
		else
		{
			//
			// 型式解釈
			//
			if ( sparsLine.HasToComeChar( L":" ) != L':' )
			{
				m_strErrMsg = L"REG ALLOC 文に型が指定されていません" ;
				return	errFailed ;
			}
			ECSTypeInfo	typeMem ;
			ECSSourceStream
					cssLine = sparsLine.SubString( sparsLine.GetIndex() ) ;
			ESLError	err =
				m_compiler->ParseTypeDescription( typeMem, cssLine ) ;
			if ( err )
			{
				m_strErrMsg = GetESLErrorMsg( err ) ;
				return	errFailed ;
			}
			sparsLine.SeekIndex
				( sparsLine.GetIndex() + cssLine.GetIndex() ) ;
			//
			// レジスタ割り当て定義
			//
			RegisterAssign *	pRegAssign = new RegisterAssign ;
			pRegAssign->numReg = numReg ;
			pRegAssign->numCount = numCount ;
			m_assignsRegName.SetAs( strName, pRegAssign ) ;
			for ( int j = 0; j < numCount; j ++ )
			{
				m_assignsRegType.SetAt
					( numReg + j, new ECSTypeInfo( typeMem ) ) ;
			}
		}
	}
	else if ( (strCommand.CompareNoCase( L"FREE" ) == 0)
			|| (strCommand.CompareNoCase( L"RELOAD" ) == 0)
			|| (strCommand.CompareNoCase( L"FLUSH" ) == 0) )
	{
		//
		// 定義名解釈
		//
		for ( ; ; )
		{
			SString	strName ;
			sparsLine.NextToken( strName ) ;
			//
			RegisterAssign *	pRegAssign = m_assignsRegName.GetAs( strName ) ;
			if ( pRegAssign == NULL )
			{
				m_strErrMsg = L"REG 文の定義名が不正です" ;
				return	errFailed ;
			}
			if ( strCommand.CompareNoCase( L"FREE" ) == 0 )
			{
				for ( int i = 0; i < pRegAssign->numCount; i ++ )
				{
					m_assignsRegType.SetAt( pRegAssign->numReg + i, NULL ) ;
				}
				m_assignsRegName.RemoveAs( strName ) ;
			}
			else if ( strCommand.CompareNoCase( L"RELOAD" ) == 0 )
			{
				for ( int i = 0; i < pRegAssign->numCount; i ++ )
				{
					ECSTypeInfo *	pRegType =
							m_assignsRegType.GetAt( pRegAssign->numReg + i ) ;
					if ( (pRegType != NULL) && pRegType->IsAddressingInfo() )
					{
						int	regDst = pRegAssign->numReg + i ;
						m_pcsxi->WriteSakuraLoadMemory
							( regDst, *pRegType, false, 0, true ) ;
					}
					else
					{
						m_strErrMsg = L"REG 文で型とアドレスが定義されていない"
										L"レジスタを RELOAD しようとしています" ;
						return	errFailed ;
					}
				}
			}
			else
			{
				for ( int i = 0; i < pRegAssign->numCount; i ++ )
				{
					ECSTypeInfo *	pRegType =
							m_assignsRegType.GetAt( pRegAssign->numReg + i ) ;
					if ( (pRegType != NULL) && pRegType->IsAddressingInfo() )
					{
						int	regSrc = pRegAssign->numReg + i ;
						m_pcsxi->WriteSakuraStoreMemory
							( regSrc, *pRegType, false, 0, true ) ;
					}
					else
					{
						m_strErrMsg = L"REG 文で型とアドレスが定義されていない"
										L"レジスタを FLUSH しようとしています" ;
						return	errFailed ;
					}
				}
			}
			if ( sparsLine.HasToComeChar( L"," ) != L',' )
			{
				break ;
			}
		}
	}
	else
	{
		m_strErrMsg = L"REG " ;
		m_strErrMsg += strCommand ;
		m_strErrMsg += L" は不正な REG 文です" ;
		return	errFailed ;
	}
	return	errSuccess ;
}

// ASSUME <register> : <type-expression>
//////////////////////////////////////////////////////////////////////////////
SSystem::SError ECSAssembler::AssembleAssume( SSystem::SStringParser & sparsLine )
{
	int	numReg = ECSSakura2Assember::ParseRegister( sparsLine ) ;
	if ( (numReg < 0) || (numReg >= 0x100) )
	{
		m_strErrMsg = L"ASSUME 文のレジスタ指定が不正です" ;
		return	errFailed ;
	}
	if ( sparsLine.HasToComeChar( L":" ) != L':' )
	{
		m_strErrMsg = L"ASSUME 文に型指定がありません" ;
		return	errFailed ;
	}
	ECSTypeInfo		typeReg ;
	ECSSourceStream	cssLine = sparsLine.SubString( sparsLine.GetIndex() ) ;
	ESLError	err = m_compiler->ParseTypeDescription( typeReg, cssLine ) ;
	if ( err )
	{
		m_strErrMsg = GetESLErrorMsg( err ) ;
		return	errFailed ;
	}
	sparsLine.SeekIndex( sparsLine.GetIndex() + cssLine.GetIndex() ) ;
	//
	if ( typeReg.IsVoid() )
	{
		m_assignsRegType.SetAt( numReg, NULL ) ;
	}
	else
	{
		m_assignsRegType.SetAt( numReg, new ECSTypeInfo( typeReg ) ) ;
	}
	return	errSuccess ;
}

// INVOKE <function> , <argument-list>, ...
//////////////////////////////////////////////////////////////////////////////
SSystem::SError ECSAssembler::AssembleInvoke( SSystem::SStringParser & sparsLine )
{
	FreeAllTemporaryRegister() ;
	//
	// 関数解釈
	//
	Operand			opFunc ;
	int				regFunc = -1 ;
	SymbolReference	symFuncRef ;
	ECSTypeInfo		typeFunc ;
	SError	err = CompileSymbol( opFunc, symFuncRef, sparsLine ) ;
	if ( err )
	{
		return	err ;
	}
	if ( opFunc.m_type != Operand::typeInvalid )
	{
		if ( (opFunc.m_type != Operand::typeAddress)
			|| ((symFuncRef.m_typeSymbol != symbolFunction)
				&& (symFuncRef.m_typeSymbol != symbolSysCall)) )
		{
			m_strErrMsg = L"INVOKE に関数名が指定されていません" ;
			return	errFailed ;
		}
	}
	else
	{
		err = CompileRuntimeTerm( regFunc, typeFunc, sparsLine ) ;
		if ( err )
		{
			return	err ;
		}
		ECSFunction *	pFunc = typeFunc.GetTypeFunctionPointer() ;
		if ( pFunc == NULL )
		{
			m_strErrMsg = L"関数ポインタではありません" ;
			return	errFailed ;
		}
		symFuncRef.m_pProto = &(pFunc->m_prototype) ;
	}
	//
	// 引数解釈
	//
	int		regFirstArg = -1 ;
	size_t	nArgCount = 0 ;
	for ( ; ; )
	{
		if ( sparsLine.HasToComeChar( L"," ) != L',' )
		{
			break ;
		}
		//
		// 引数をレジスタにロード
		//
		ECSTypeInfo	typeArg ;
		int			regArg = -1 ;
		err = CompileRuntimeTerm( regArg, typeArg, sparsLine ) ;
		if ( err )
		{
			return	err ;
		}
		if ( !IsTemporaryRegister( regArg ) )
		{
			int	regTemp = AllocateTemporaryRegister() ;
			m_pcsxi->WriteSakuraMoveRegReg( regTemp, regArg ) ;
			regArg = regTemp ;
		}
		ECSTypeInfo	typeProtoArg ;
		if ( nArgCount == 0 )
		{
			regFirstArg = regArg ;
		}
		else if ( regFirstArg + (int) nArgCount != regArg )
		{
			m_strErrMsg = L"レジスタの割り当てに失敗しました" ;
			return	errFailed ;
		}
		if ( symFuncRef.m_pProto->IsThisCall() )
		{
			if ( nArgCount == 0 )
			{
				typeProtoArg.SetTypeValue( new ECSPointer(), 0 ) ;
			}
			else
			{
				ECSTypeInfo *	pArgType =
					symFuncRef.m_pProto->GetArgumentAt( nArgCount - 1 ) ;
				if ( pArgType != NULL )
				{
					typeProtoArg = *pArgType ;
				}
				else
				{
					m_strErrMsg = L"引数が多すぎます" ;
					return	errFailed ;
				}
			}
		}
		else
		{
			ECSTypeInfo *	pArgType =
				symFuncRef.m_pProto->GetArgumentAt( nArgCount ) ;
			if ( pArgType != NULL )
			{
				typeProtoArg = *pArgType ;
			}
			else
			{
				m_strErrMsg = L"引数が多すぎます" ;
				return	errFailed ;
			}
		}
		//
		// 型チェック
		//
		if ( typeProtoArg.IsTypeReference() )
		{
			if ( !typeArg.IsTypeInteger()
				&& !typeArg.IsTypePointer()
				&& !typeArg.IsTypeReference() )
			{
				m_strErrMsg = L"参照型引数に型が一致しません" ;
				return	errFailed ;
			}
		}
		else
		{
			if ( typeArg.IsTypeReference() )
			{
				ECSTypeInfo	typeTemp = typeArg ;
				int	regLoad = regArg ;
				typeArg.MakeNakedOf( typeTemp ) ;
				m_pcsxi->WriteSakuraLoadMemory
					( addrBase, DataTypeFromTypeInfo( typeArg ),
								regLoad, regArg, 0, -1, 0, true ) ;
			}
			bool	fMatchArg = false ;
			if ( typeProtoArg.IsTypeInteger() )
			{
				fMatchArg = typeArg.IsVoid() || typeArg.IsTypeInteger() ;
			}
			else if ( typeProtoArg.IsTypeReal() )
			{
				fMatchArg = typeArg.IsVoid() || typeArg.IsTypeReal() ;
			}
			else if ( typeProtoArg.IsTypePointer() )
			{
				fMatchArg = typeArg.IsVoid() || typeArg.IsTypePointer() ;
			}
			else
			{
				fMatchArg = true ;
			}
			if ( !fMatchArg )
			{
				m_strErrMsg = L"引数型が一致しません" ;
				return	errFailed ;
			}
		}
		nArgCount ++ ;
	}
	bool	fMatchArgCount = false ;
	if ( symFuncRef.m_pProto->IsThisCall() )
	{
		fMatchArgCount =
			(symFuncRef.m_pProto->GetArgumentCount() == nArgCount - 1) ;
	}
	else
	{
		fMatchArgCount =
			(symFuncRef.m_pProto->GetArgumentCount() == nArgCount) ;
	}
	if ( !fMatchArgCount )
	{
		m_strErrMsg = L"引数の数が一致しません" ;
		return	errFailed ;
	}
	//
	// 引数プッシュ
	//
	if ( nArgCount > 0 )
	{
		if ( nArgCount == 1 )
		{
			m_pcsxi->WriteSakuraPushReg( regFirstArg ) ;
		}
		else
		{
			m_pcsxi->WriteSakuraPushRegsImm8( regFirstArg, nArgCount ) ;
		}
		for ( size_t i = 0; i < nArgCount; i ++ )
		{
			FreeTemporaryRegister( regFirstArg + i ) ;
		}
	}
	//
	// 関数呼び出し
	//
	if ( opFunc.m_type == Operand::typeAddress )
	{
		if ( symFuncRef.m_typeSymbol == symbolFunction )
		{
			m_pcsxi->WriteSakuraCallFunction( symFuncRef.m_strSymbol ) ;
		}
		else if ( symFuncRef.m_typeSymbol == symbolSysCall )
		{
			m_pcsxi->WriteSakuraSysCallFunction( symFuncRef.m_strSymbol ) ;
		}
		else
		{
			m_strErrMsg = L"関数が不正です" ;
			return	errFailed ;
		}
	}
	else
	{
		if ( regFunc < 0 )
		{
			m_strErrMsg = L"実行時式関数ポインタ"
							L"レジスタが割り当てられていません" ;
			return	errFailed ;
		}
		if ( symFuncRef.m_pProto->GetAttribute()
							& ECSTypeInfo::flagNativeObject )
		{
			m_pcsxi->WriteSakuraSysCallIndirect( regFunc ) ;
		}
		else
		{
			m_pcsxi->WriteSakuraCallReg( regFunc ) ;
		}
		FreeTemporaryRegister( regFunc ) ;
	}
	//
	// 引数スタック解放
	//
	if ( nArgCount > 0 )
	{
		m_pcsxi->WriteSakuraAddSP( nArgCount * 8 ) ;
	}
	return	errSuccess ;
}

// 実行時式用一時レジスタ割り当て
//////////////////////////////////////////////////////////////////////////////
int ECSAssembler::AllocateTemporaryRegister( void )
{
	for ( int i = 0; i < 16; i ++ )
	{
		if ( !(m_maskTempReg & (1 << i)) )
		{
			m_maskTempReg |= (1 << i) ;
			return	112 + i ;
		}
	}
	m_strErrMsg = L"実行時式が複雑すぎます" ;
	return	-1 ;
}

// 一時レジスタ解放
//////////////////////////////////////////////////////////////////////////////
void ECSAssembler::FreeTemporaryRegister( int reg )
{
	if ( (reg >= 112) && (reg < 128) )
	{
		m_maskTempReg &= ~(1 << (reg - 112)) ;
	}
}

void ECSAssembler::FreeAllTemporaryRegister( void )
{
	m_maskTempReg = 0 ;
}

// 一時レジスタか？
//////////////////////////////////////////////////////////////////////////////
bool ECSAssembler::IsTemporaryRegister( int reg ) const
{
	return	(reg >= 112) && (reg < 128) ;
}

// 実行時式の1項を計算しレジスタを返す
//////////////////////////////////////////////////////////////////////////////
SSystem::SError ECSAssembler::CompileRuntimeTerm
	( int& numReg, ECSTypeInfo& typeTerm,
			SSystem::SStringParser & sparsLine )
{
	SError	err ;
	//
	// 型キャスト解釈
	//
	wchar_t		wchOp = sparsLine.HasToComeChar( L"(" ) ;
	if ( wchOp == L'(' )
	{
		ECSSourceStream	cssLine =
			sparsLine.SubString( sparsLine.GetIndex() ) ;
		if ( !m_compiler->ParseTypeDescription( typeTerm, cssLine ) )
		{
			sparsLine.SeekIndex( sparsLine.GetIndex() + cssLine.GetIndex() ) ;
			if ( sparsLine.HasToComeChar( L")" ) != L')' )
			{
				m_strErrMsg = L"実行時式で \'(\' に対応する \')\' が見つかりません" ;
				return	errFailed ;
			}
			ECSTypeInfo	typeTemp ;
			err = CompileRuntimeTerm
				( numReg, typeTemp, sparsLine ) ;
			if ( err )
			{
				return	err ;
			}
			return	errSuccess ;
		}
	}
	//
	// レジスタ判定
	//
	typeTerm = ECSTypeInfo() ;
	numReg = -1 ;
	err = ParseRegister( numReg, sparsLine ) ;
	if ( err )
	{
		return	err ;
	}
	if ( numReg >= 0 )
	{
		ECSTypeInfo *	pRegType = m_assignsRegType.GetAt( numReg ) ;
		if ( pRegType != NULL )
		{
			typeTerm = *pRegType ;
		}
		else
		{
			typeTerm.SetTypeValue( NULL, 0 ) ;
		}
		return	errSuccess ;
	}
	//
	// メモリ参照判定
	//
	ECSSakura2Assember::Operand	opTerm ;
	opTerm.m_type = Operand::typeInvalid ;
	err = ParseMemoryOperand( typeTerm, opTerm, sparsLine ) ;
	if ( err )
	{
		return	err ;
	}
	if ( opTerm.m_type != Operand::typeInvalid )
	{
		numReg = AllocateTemporaryRegister() ;
		if ( numReg < 0 )
		{
			return	errFailed ;
		}
		m_pcsxi->WriteSakuraLoadMemory
			( opTerm.m_mem.modeAddr, opTerm.m_mem.typeData, numReg,
				opTerm.m_mem.regBase, opTerm.m_mem.offsetAddr,
				opTerm.m_mem.regIndex, opTerm.m_mem.scaleIndex, true ) ;
		return	errSuccess ;
	}
	//
	// 定数値判定
	//
	ECSObject *	pValue = NULL ;
	err = CompileImmediate
		( pValue, sparsLine, L",)", ECSCompiler::oppCompare ) ;
	if ( err )
	{
		return	err ;
	}
	if ( pValue == NULL )
	{
		m_strErrMsg = L"不正な実行時式です" ;
		return	errFailed ;
	}
	if ( pValue->m_vtType == csvtInteger )
	{
		numReg = AllocateTemporaryRegister() ;
		typeTerm.SetTypeValue( pValue, 0 ) ;
		m_pcsxi->WriteSakuraLoadInt64
			( numReg, ((ECSInteger*)pValue)->GetValue() ) ;
	}
	else if ( pValue->m_vtType == csvtReal )
	{
		typeTerm.SetTypeValue( pValue, 0 ) ;
		m_pcsxi->WriteSakuraLoadReal64
			( numReg, ((ECSReal*)pValue)->m_varReal ) ;
	}
	else
	{
		delete	pValue ;
		m_strErrMsg = L"不正な実行時式です" ;
		return	errFailed ;
	}
	return	errSuccess ;
}

// 条件実行時式を評価し条件分岐コードを出力する
//////////////////////////////////////////////////////////////////////////////
SSystem::SError ECSAssembler::CompileRuntimeExpression
	( int& regRuntime,
		LabelEntry& labelTrue,
		LabelEntry& labelFalse, bool fPositive,
		SSystem::SStringParser & sparsLine,
		const wchar_t * pwszExit, int nPriority )
{
	//
	// 終了判定
	//
	regRuntime = -1 ;
	if ( !sparsLine.PassSpace() )
	{
		return	errSuccess ;
	}
	if ( pwszExit != NULL )
	{
		sparsLine.MarkIndex() ;
		wchar_t	wchExit = sparsLine.HasToComeChar( pwszExit ) ;
		if ( wchExit != 0 )
		{
			sparsLine.SeekToMark() ;
			return	errSuccess ;
		}
	}
	//
	// 前置演算子／項判定
	//
	SError		err ;
	ECSTypeInfo	typeExpr ;
	wchar_t		wchOp = sparsLine.HasToComeChar( L"!(" ) ;
	if ( wchOp == L'!' )
	{
		err = CompileRuntimeConditionalExpression
			( labelTrue, labelFalse, !fPositive,
				sparsLine, pwszExit, ECSCompiler::oppUnary ) ;
		if ( err )
		{
			return	err ;
		}
	}
	else if ( wchOp == L'(' )
	{
		ECSSourceStream	cssLine =
			sparsLine.SubString( sparsLine.GetIndex() ) ;
		if ( !m_compiler->ParseTypeDescription( typeExpr, cssLine ) )
		{
			sparsLine.SeekIndex( sparsLine.GetIndex() + cssLine.GetIndex() ) ;
			if ( sparsLine.HasToComeChar( L")" ) != L')' )
			{
				m_strErrMsg = L"実行時式で \'(\' に対応する \')\' が見つかりません" ;
				return	errFailed ;
			}
			ECSTypeInfo	typeTemp ;
			err = CompileRuntimeTerm
				( regRuntime, typeTemp, sparsLine ) ;
			if ( err )
			{
				return	err ;
			}
		}
		else
		{
			err = CompileRuntimeExpression
				( regRuntime, labelTrue, labelFalse,
								fPositive, sparsLine, L")" ) ;
			if ( err )
			{
				return	err ;
			}
			if ( sparsLine.HasToComeChar( L")" ) != L')' )
			{
				m_strErrMsg = L"実行時式で \'(\' に対応する \')\' が見つかりません" ;
				return	errFailed ;
			}
		}
	}
	else
	{
		SString	strToken ;
		sparsLine.MarkIndex() ;
		sparsLine.NextToken( strToken ) ;
		if ( strToken.CompareNoCase( L"ADDR" ) == 0 )
		{
			ECSSakura2Assember::Operand	opTerm[2] ;
			err = ParseMemoryOperand( typeExpr, opTerm[1], sparsLine ) ;
			if ( err )
			{
				return	err ;
			}
			if ( opTerm[1].m_type != Operand::typeMemory )
			{
				m_strErrMsg = L"メモリでないオペランドに"
								L" ADDR 演算子が指定されています" ;
				return	errFailed ;
			}
			//
			InstructionBuffer	ibuf ;
			regRuntime = AllocateTemporaryRegister() ;
			opTerm[0].m_type = Operand::typeRegister ;
			opTerm[0].m_reg = regRuntime ;
			//
			if ( AssembleMacroInstructionLea
				( ibuf, m_strErrMsg, L"lea", 0, opTerm, 2 ) )
			{
				return	errFailed ;
			}
			typeExpr.SetTypeValue( new ECSInteger, 0 ) ;
		}
		else
		{
			sparsLine.SeekToMark() ;
			err = CompileRuntimeTerm( regRuntime, typeExpr, sparsLine ) ;
			if ( err )
			{
				return	err ;
			}
		}
	}
	//
	// 演算子と第二項以降処理
	//
	err = CompileRuntimeConditionalOperators
		( labelTrue, labelFalse, fPositive,
			typeExpr, regRuntime, sparsLine, pwszExit, nPriority ) ;
	if ( err )
	{
		return	err ;
	}
	return	errSuccess ;
}

SSystem::SError ECSAssembler::CompileRuntimeConditionalExpression
	( ECSAssembler::LabelEntry& labelTrue,
		ECSAssembler::LabelEntry& labelFalse, bool fPositive,
		SSystem::SStringParser & sparsLine,
		const wchar_t * pwszExit, int nPriority )
{
	//
	// 実行時式
	//
	int		regRuntime = -1 ;
	SError	err = CompileRuntimeExpression
		( regRuntime, labelTrue, labelFalse,
			fPositive, sparsLine, pwszExit, nPriority ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// 最終的に結果がレジスタに残っている場合にはジャンプ命令生成
	//
	if ( regRuntime >= 0 )
	{
		DWORD	dwJumpRef ;
		if ( fPositive )
		{
			dwJumpRef =
				m_pcsxi->WriteSakuraCJumpOffset32( regRuntime, 0 ) ;
		}
		else
		{
			dwJumpRef =
				m_pcsxi->WriteSakuraCNJumpOffset32( regRuntime, 0 ) ;
		}
		FreeTemporaryRegister( regRuntime ) ;
		labelTrue.Add( dwJumpRef ) ;
	}
	return	errSuccess ;
}

// 条件実行時式を評価し条件分岐コードを出力する（二項演算子継続処理）
//////////////////////////////////////////////////////////////////////////////
SSystem::SError ECSAssembler::CompileRuntimeConditionalOperators
	( ECSAssembler::LabelEntry& labelTrue,
		ECSAssembler::LabelEntry& labelFalse, bool fPositive,
		ECSTypeInfo& typeExpr, int& regRuntime,
		SSystem::SStringParser & sparsLine,
		const wchar_t * pwszExit, int nPriority )
{
	for ( ; ; )
	{
		//
		// 終了判定
		//
		if ( !sparsLine.PassSpace() )
		{
			return	errSuccess ;
		}
		if ( pwszExit != NULL )
		{
			sparsLine.MarkIndex() ;
			wchar_t	wchExit = sparsLine.HasToComeChar( pwszExit ) ;
			if ( wchExit != 0 )
			{
				sparsLine.SeekToMark() ;
				return	errSuccess ;
			}
		}
		//
		// 演算子判定
		//
		SString	strOperator ;
		sparsLine.MarkIndex() ;
		if ( ((sparsLine.CurrentCharacter() == L'<')
				|| (sparsLine.CurrentCharacter() == L'>'))
			&& (sparsLine.OffsetAt(1) == L'=') )
		{
			strOperator += sparsLine.GetCharacter() ;
			strOperator += sparsLine.GetCharacter() ;
		}
		else
		{
			sparsLine.NextToken( strOperator ) ;
		}
		//
		ECSCompiler::OPERATOR_INFO	opinf ;
		if ( m_compiler->GetOperatorInfo( opinf, strOperator ) )
		{
			m_strErrMsg = L"実行時式に解釈不能な \'" ;
			m_strErrMsg += strOperator ;
			m_strErrMsg += L"\' を発見しました" ;
			return	errFailed ;
		}
		int	nOpPriority = m_compiler->GetOperatorPriority( opinf ) ;
		if ( nOpPriority <= nPriority )
		{
			sparsLine.SeekToMark() ;
			return	errSuccess ;
		}
		ECSTypeInfo	typeTerm2 ;
		int			regTerm2 = -1 ;
		SError		err ;
		if ( opinf.opiType == ECSCompiler::optCompare )
		{
			//
			// 比較演算子
			//
			err = CompileRuntimeTerm( regTerm2, typeTerm2, sparsLine ) ;
			if ( err )
			{
				return	err ;
			}
			err = CompileRuntimeConditionalComparator
				( opinf.cptCompare, strOperator,
					typeExpr, regRuntime, typeTerm2, regTerm2 ) ;
			if ( err )
			{
				return	err ;
			}
		}
		else if ( opinf.opiType == ECSCompiler::optGeneral )
		{
			//
			// 二項演算子
			//
			err = CompileRuntimeConditionalOperator
				( opinf.optOperator, strOperator,
					labelTrue, labelFalse, fPositive,
					typeExpr, regRuntime,
					sparsLine, pwszExit, nOpPriority ) ;
			if ( err )
			{
				return	err ;
			}
			if ( regRuntime < 0 )
			{
				err = CompileRuntimeExpression
					( regRuntime, labelTrue, labelFalse,
						fPositive, sparsLine, pwszExit, nPriority ) ;
				if ( err )
				{
					return	err ;
				}
			}
		}
		else
		{
			m_strErrMsg = L"実行時式に \'" ;
			m_strErrMsg += strOperator ;
			m_strErrMsg += L"\' 演算子は使用出来ません" ;
			return	errFailed ;
		}
	}
}

// 実行時式比較演算子
//////////////////////////////////////////////////////////////////////////////
SSystem::SError ECSAssembler::CompileRuntimeConditionalComparator
	( CSCompareType csctType, const SSystem::SString& strOperator,
		ECSTypeInfo& typeTerm1, int& regTerm1,
		const ECSTypeInfo& typeTerm2, int regTerm2 )
{
	bool	fTypeReal = typeTerm1.IsTypeReal()
							|| typeTerm2.IsTypeReal() ;
	bool	fUnsignedInt = false ;
	bool	fCompareInt32 = false ;
	bool	fCompareUInt32 = false ;
	if ( (typeTerm1.IsTypeReal() && !typeTerm2.IsTypeReal())
		|| (!typeTerm1.IsTypeReal() && typeTerm2.IsTypeReal()) )
	{
		m_strErrMsg = L"実行時式の比較で型が一致しません" ;
		return	errFailed ;
	}
	if ( !fTypeReal )
	{
		int		sizeTerm1 = 64, sizeTerm2 = 64 ;
		bool	signTerm1 = true, signTerm2 = true ;
		if ( (typeTerm1.m_pValue != NULL)
			&& (typeTerm1.m_pValue->m_vtType == csvtInteger) )
		{
			sizeTerm1 = ((ECSInteger*)typeTerm1.m_pValue)->SizeOf() ;
			signTerm1 = ((ECSInteger*)typeTerm1.m_pValue)->IsSign() ;
			fUnsignedInt |= (sizeTerm1 == 64) && !signTerm1 ;
		}
		if ( (typeTerm2.m_pValue != NULL)
			&& (typeTerm2.m_pValue->m_vtType == csvtInteger) )
		{
			sizeTerm2 = ((ECSInteger*)typeTerm2.m_pValue)->SizeOf() ;
			signTerm2 = ((ECSInteger*)typeTerm2.m_pValue)->IsSign() ;
			fUnsignedInt |= (sizeTerm2 == 64) && !signTerm2 ;
		}
		if ( ((sizeTerm1 < 32) || (signTerm1 && (sizeTerm1 == 32)))
			&& ((sizeTerm2 < 32) || (signTerm2 && (sizeTerm2 == 32))) )
		{
			fCompareInt32 = true ;
		}
		if ( ((sizeTerm1 <= 32) && !signTerm1)
			|| ((sizeTerm2 <= 32) && !signTerm2) )
		{
			fCompareUInt32 = true ;
		}
	}
	bool	fSwap = false ;
	if ( fUnsignedInt
		&& ((csctType == csctGreaterThan)
				|| (csctType == csctGreaterEqual)) )
	{
		if ( !IsTemporaryRegister( regTerm2 ) )
		{
			int	regTemp = AllocateTemporaryRegister() ;
			m_pcsxi->WriteSakuraMoveRegReg( regTemp, regTerm2 ) ;
			regTerm2 = regTemp ;
		}
		fSwap = true ;
	}
	else
	{
		if ( !IsTemporaryRegister( regTerm1 ) )
		{
			int	regTemp = AllocateTemporaryRegister() ;
			m_pcsxi->WriteSakuraMoveRegReg( regTemp, regTerm1 ) ;
			regTerm1 = regTemp ;
		}
	}
	switch ( csctType )
	{
	case	csctNotEqual:
		if ( fTypeReal )
		{
			m_pcsxi->WriteSakuraOperandRegReg
				( codeFCmpNeReg, regTerm1, regTerm2 ) ;
		}
		else if ( fCompareInt32 || fCompareUInt32 )
		{
			m_pcsxi->WriteSakuraSIMD64OperandRegReg
				( simdPcmpnesd, regTerm1, regTerm2 ) ;
		}
		else
		{
			m_pcsxi->WriteSakuraOperandRegReg
				( codeCmpNeReg, regTerm1, regTerm2 ) ;
		}
		break ;
	case	csctEqual:
		if ( fTypeReal )
		{
			m_pcsxi->WriteSakuraOperandRegReg
				( codeFCmpEqReg, regTerm1, regTerm2 ) ;
		}
		else if ( fCompareInt32 || fCompareUInt32 )
		{
			m_pcsxi->WriteSakuraSIMD64OperandRegReg
				( simdPcmpeqsd, regTerm1, regTerm2 ) ;
		}
		else
		{
			m_pcsxi->WriteSakuraOperandRegReg
				( codeCmpEqReg, regTerm1, regTerm2 ) ;
		}
		break ;
	case	csctLessThan:
		if ( fTypeReal )
		{
			m_pcsxi->WriteSakuraOperandRegReg
				( codeFCmpLtReg, regTerm1, regTerm2 ) ;
		}
		else if ( fCompareInt32 )
		{
			m_pcsxi->WriteSakuraSIMD64OperandRegReg
				( simdPcmpltsd, regTerm1, regTerm2 ) ;
		}
		else if ( fUnsignedInt )
		{
			m_pcsxi->WriteSakuraOperandRegReg
				( codeCmpCReg, regTerm1, regTerm2 ) ;
		}
		else
		{
			m_pcsxi->WriteSakuraOperandRegReg
				( codeCmpLtReg, regTerm1, regTerm2 ) ;
		}
		break ;
	case	csctLessEqual:
		if ( fTypeReal )
		{
			m_pcsxi->WriteSakuraOperandRegReg
				( codeFCmpLeReg, regTerm1, regTerm2 ) ;
		}
		else if ( fCompareInt32 )
		{
			m_pcsxi->WriteSakuraSIMD64OperandRegReg
				( simdPcmplesd, regTerm1, regTerm2 ) ;
		}
		else if ( fUnsignedInt )
		{
			m_pcsxi->WriteSakuraOperandRegReg
				( codeCmpCZReg, regTerm1, regTerm2 ) ;
		}
		else
		{
			m_pcsxi->WriteSakuraOperandRegReg
				( codeCmpLeReg, regTerm1, regTerm2 ) ;
		}
		break ;
	case	csctGreaterThan:
		if ( fTypeReal )
		{
			m_pcsxi->WriteSakuraOperandRegReg
				( codeFCmpGtReg, regTerm1, regTerm2 ) ;
		}
		else if ( fCompareInt32 )
		{
			m_pcsxi->WriteSakuraSIMD64OperandRegReg
				( simdPcmpgtsd, regTerm1, regTerm2 ) ;
		}
		else if ( fUnsignedInt )
		{
			m_pcsxi->WriteSakuraOperandRegReg
				( codeCmpCReg, regTerm2, regTerm1 ) ;
			ESLAssert( fSwap ) ;
		}
		else
		{
			m_pcsxi->WriteSakuraOperandRegReg
				( codeCmpGtReg, regTerm1, regTerm2 ) ;
		}
		break ;
	case	csctGreaterEqual:
		if ( fTypeReal )
		{
			m_pcsxi->WriteSakuraOperandRegReg
				( codeFCmpGeReg, regTerm1, regTerm2 ) ;
		}
		else if ( fCompareInt32 )
		{
			m_pcsxi->WriteSakuraSIMD64OperandRegReg
				( simdPcmpgesd, regTerm1, regTerm2 ) ;
		}
		else if ( fUnsignedInt )
		{
			m_pcsxi->WriteSakuraOperandRegReg
				( codeCmpCZReg, regTerm2, regTerm1 ) ;
			ESLAssert( fSwap ) ;
		}
		else
		{
			m_pcsxi->WriteSakuraOperandRegReg
				( codeCmpGeReg, regTerm1, regTerm2 ) ;
		}
		break ;
	default:
		m_strErrMsg = L"実行時式に \'" ;
		m_strErrMsg += strOperator ;
		m_strErrMsg += L"\' 演算子は使用出来ません" ;
		return	errFailed ;
	}
	typeTerm1.SetTypeValue
		( new ECSInteger( 0, ECSInteger::m_maskBoolean ), 0 ) ;
	if ( fSwap )
	{
		if ( IsTemporaryRegister( regTerm1 ) )
		{
			FreeTemporaryRegister( regTerm1 ) ;
		}
		regTerm1 = regTerm2 ;
	}
	else
	{
		if ( IsTemporaryRegister( regTerm2 ) )
		{
			FreeTemporaryRegister( regTerm2 ) ;
		}
	}
	return	errSuccess ;
}

// 実行時式演算子
//////////////////////////////////////////////////////////////////////////////
SSystem::SError ECSAssembler::CompileRuntimeConditionalOperator
	( CSOperatorType csotType,
		const SSystem::SString& strOperator,
		ECSAssembler::LabelEntry& labelTrue,
		ECSAssembler::LabelEntry& labelFalse, bool fPositive,
		ECSTypeInfo& typeExpr, int& regRuntime,
		SSystem::SStringParser & sparsLine,
		const wchar_t * pwszExit, int nPriority )
{
	if ( regRuntime < 0 )
	{
		m_strErrMsg = L"実行時式で複雑すぎる論理演算子の組み合わせです" ;
		return	errFailed ;
	}
	if ( csotType == csotLogicalAnd )
	{
		DWORD	dwJumpRef ;
		if ( fPositive )
		{
			dwJumpRef =
				m_pcsxi->WriteSakuraCNJumpOffset32( regRuntime, 0 ) ;
			labelFalse.Add( dwJumpRef ) ;
		}
		else
		{
			dwJumpRef =
				m_pcsxi->WriteSakuraCNJumpOffset32( regRuntime, 0 ) ;
			labelTrue.Add( dwJumpRef ) ;
		}
		FreeTemporaryRegister( regRuntime ) ;
		regRuntime = -1 ;
	}
	else if ( csotType == csoutLogicalOr )
	{
		DWORD	dwJumpRef ;
		if ( fPositive )
		{
			dwJumpRef =
				m_pcsxi->WriteSakuraCJumpOffset32( regRuntime, 0 ) ;
			labelTrue.Add( dwJumpRef ) ;
		}
		else
		{
			dwJumpRef =
				m_pcsxi->WriteSakuraCJumpOffset32( regRuntime, 0 ) ;
			labelFalse.Add( dwJumpRef ) ;
		}
		FreeTemporaryRegister( regRuntime ) ;
		regRuntime = -1 ;
	}
	else if ( (csotType == csotAnd)
				|| (csotType == csotOr) || (csotType == csotXor) )
	{
		LabelEntry	labelTempTrue, labelTempFalse ;
		int	regRuntime2 = -1 ;
		SError	err =
			CompileRuntimeExpression
				( regRuntime2, labelTempTrue, labelTempFalse,
					fPositive, sparsLine, pwszExit, nPriority ) ;
		if ( err )
		{
			return	err ;
		}
		if ( (regRuntime2 < 0)
			|| (labelTempTrue.GetLength() > 0)
			|| (labelTempFalse.GetLength() > 0) )
		{
			m_strErrMsg = L"実行時式で複雑すぎる論理演算子の組み合わせです" ;
			return	errFailed ;
		}
		if ( !IsTemporaryRegister( regRuntime ) )
		{
			if ( !IsTemporaryRegister( regRuntime2 ) )
			{
				int	regTemp = AllocateTemporaryRegister() ;
				m_pcsxi->WriteSakuraMoveRegReg( regTemp, regRuntime ) ;
				regRuntime = regTemp ;
			}
			else
			{
				int	regTemp = regRuntime ;
				regRuntime = regRuntime2 ;
				regRuntime2 = regTemp ;
			}
		}
		switch ( csotType )
		{
		case	csotAnd:
			m_pcsxi->WriteSakuraOperandRegReg
				( codeAndReg, regRuntime, regRuntime2 ) ;
			break ;
		case	csotOr:
			m_pcsxi->WriteSakuraOperandRegReg
				( codeOrReg, regRuntime, regRuntime2 ) ;
			break ;
		case	csotXor:
			m_pcsxi->WriteSakuraOperandRegReg
				( codeXorReg, regRuntime, regRuntime2 ) ;
			break ;
		}
		if ( IsTemporaryRegister( regRuntime2 ) )
		{
			FreeTemporaryRegister( regRuntime2 ) ;
		}
	}
	else
	{
		m_strErrMsg = L"実行時式に \'" ;
		m_strErrMsg += strOperator ;
		m_strErrMsg += L"\' 演算子は使用出来ません" ;
		return	errFailed ;
	}
	return	errSuccess ;
}


