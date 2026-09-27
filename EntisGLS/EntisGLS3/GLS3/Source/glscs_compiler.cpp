
/*****************************************************************************
             Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
	Copyright (c) 2002-2014 Leshade Entis, Entis-soft. All rights reserved.
 ****************************************************************************/


#include <gls.h>


//////////////////////////////////////////////////////////////////////////////
// Entis Cotopha Script コンパイラ
//////////////////////////////////////////////////////////////////////////////

const ECSCompiler::PFN_COMPILE_RESERVED_WORD
	ECSCompiler::m_pfnCompileReservedWord[ECSCompiler::rwMax] =
{
	&ECSCompiler::CompileInclude,
	&ECSCompiler::CompileOption,
	&ECSCompiler::CompileDeclareType,
	&ECSCompiler::CompileDeclareDef,
	&ECSCompiler::CompileExternDef,
	&ECSCompiler::CompileTypeDef,
	&ECSCompiler::CompileVariable,
	&ECSCompiler::CompileConstant,
	&ECSCompiler::CompileData,
	&ECSCompiler::CompileEndData,
	&ECSCompiler::CompileEnumerator,
	&ECSCompiler::CompileEndEnum,
	&ECSCompiler::CompileStructure,
	&ECSCompiler::CompileEndStruct,
	&ECSCompiler::CompileClass,
	&ECSCompiler::CompileEndClass,
	&ECSCompiler::CompileNamespace,
	&ECSCompiler::CompileEndNamespace,
	&ECSCompiler::CompileUnion,
	&ECSCompiler::CompileEndUnion,
	&ECSCompiler::CompileAssembler,
	&ECSCompiler::CompileEndAssembler,
	&ECSCompiler::CompilePublic,
	&ECSCompiler::CompileProtected,
	&ECSCompiler::CompilePrivate,
	&ECSCompiler::CompilePrototype,
	&ECSCompiler::CompileFunction,
	&ECSCompiler::CompileEndFunc,
	&ECSCompiler::CompileIf,
	&ECSCompiler::CompileElseIf,
	&ECSCompiler::CompileElse,
	&ECSCompiler::CompileEndIf,
	&ECSCompiler::CompileBegin,
	&ECSCompiler::CompileEnd,
	&ECSCompiler::CompileBreak,
	&ECSCompiler::CompileContinue,
	&ECSCompiler::CompileReturn,
	&ECSCompiler::CompileGoto,
	&ECSCompiler::CompileLabel,
	&ECSCompiler::CompileTry,
	&ECSCompiler::CompileCatch,
	&ECSCompiler::CompileEndTry,
	&ECSCompiler::CompileThrow,
	&ECSCompiler::CompileFor,
	&ECSCompiler::CompileNext,
	&ECSCompiler::CompileWhile,
	&ECSCompiler::CompileEndWhile,
	&ECSCompiler::CompileRepeat,
	&ECSCompiler::CompileUntil,
	&ECSCompiler::CompileSwitch,
	&ECSCompiler::CompileEndSwitch,
	&ECSCompiler::CompileCase,
	&ECSCompiler::CompileDefault,
	&ECSCompiler::CompileMemoryFence,
	&ECSCompiler::CompileTemplate,
	&ECSCompiler::CompileEndTemplate,
	&ECSCompiler::CompileUsing,
	&ECSCompiler::CompileFriend,
} ;

const ECSCompiler::PFN_COMPILE_RESERVED_WORD
	ECSCompiler::m_pfnCompileMacroWord[ECSCompiler::mwMax] =
{
	&ECSCompiler::CompileMacroError,
	&ECSCompiler::CompileMacroWarning,
	&ECSCompiler::CompileMacroCompile,
	&ECSCompiler::CompileMacroIf,
	&ECSCompiler::CompileMacroElseIf,
	&ECSCompiler::CompileMacroElse,
	&ECSCompiler::CompileMacroEndIf,
	&ECSCompiler::CompileMacroLet,
	&ECSCompiler::CompileMacroLocal,
	&ECSCompiler::CompileMacroFor,
	&ECSCompiler::CompileMacroNext,
	&ECSCompiler::CompileMacroLiteral,
	&ECSCompiler::CompileMacroDefMacro,
	&ECSCompiler::CompileMacroEndMacro,
	&ECSCompiler::CompileMacroExitMacro,
	&ECSCompiler::CompileMacroUndefMacro,
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ECSCompiler, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ECSCompiler::ECSCompiler( void )
{
	m_rwCtrlType = rwInvalid ;
	m_modeNakedCode = false ;
	m_flagOptimize = true ;
	m_flagThrowable = false ;
	m_flagFarCall = false ;
	//
	m_pcsxiDst = NULL ;
	//
	m_nMacroMode.SetValue( 0 ) ;
	m_dwModeFlags = flagStrictStyle | flagDefualtNakedFunc ;
	m_dwImplementFlags = implementAll ;
	//
	m_pStatementCache = NULL ;
	//
	m_pPreprocessMacro = NULL ;
	m_pPreprocessFlag = NULL ;
	//
	m_modeTemporary = false ;
	m_regExprAlloc = 0 ;
	m_stackExprAlloc = 0 ;
	m_objExprAlloc = 0 ;
	//
	m_nLineNum = 0 ;
	m_nErrorCount = 0 ;
	m_nWarningCount = 0 ;
	m_nWarningLevel = 2 ;
	//
	m_pRefMacroVariable = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ECSCompiler::~ECSCompiler( void )
{
}

// 出力先を設定してコンパイラを初期化する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::Initialize
	( ECSExecutionImageCompiler * pcsxiDst, const char * pszScriptName )
{
	static const wchar_t *	pwszTypeName[csvtMax] =
	{
		NULL, L"Reference", L"Array", L"Hash",
		L"Integer", L"Real", L"String",
	} ;
	static const wchar_t *	pwszMemoryClass[csomMax] =
	{
		NULL, L"stack", L"this", L"global", L"static", L"auto"
	} ;
	//
	m_staExternName.RemoveAll( ) ;
	m_staTypeName.RemoveAll( ) ;
	m_staMemoryClass.RemoveAll( ) ;
	m_staConstant.RemoveAll( ) ;
	//
	m_pPreprocessMacro = NULL ;
	m_pPreprocessFlag = NULL ;
	//
	m_nestCtrl.RemoveAll( ) ;
	m_rwCtrlType = rwInvalid ;
	//
	m_pcsxiDst = pcsxiDst ;
	m_csxiInitFunc.DeleteImage( ) ;
	m_pcsxi = &m_csxiInitFunc ;
	DWORD	dwDummy = 0 ;
	m_csxiInitFunc.WriteInstructionCode( csicEnter ) ;
	m_csxiInitFunc.WriteConstantString
			( ECSWideString(L"@Initialize ") + ECSWideString(pszScriptName) ) ;
	m_csxiInitFunc.WriteCodeData( &dwDummy, sizeof(dwDummy) ) ;
	m_dwInitPrologueSize = m_csxiInitFunc.m_bufImage.GetLength() ;
	//
	m_nMacroMode.SetValue( 0 ) ;
	m_csstrFileName.m_varStr = pszScriptName ;
	m_nErrorCount = 0 ;
	m_nWarningCount = 0 ;
	//
	int	i ;
	for ( i = 0; i < csvtMax; i ++ )
	{
		if ( pwszTypeName[i] != NULL )
		{
			m_staTypeName.Add( pwszTypeName[i] ) ;
		}
		else
		{
			m_staTypeName.Add( NULL ) ;
		}
	}
	for ( i = 0; i < csomMax; i ++ )
	{
		if ( pwszMemoryClass[i] != NULL )
		{
			m_staMemoryClass.Add( pwszMemoryClass[i] ) ;
		}
		else
		{
			m_staMemoryClass.Add( NULL ) ;
		}
	}
	return	eslErrSuccess ;
}

// スクリプトをコンパイルする
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileScript
	( ECSSourceStream & cssScript, const char * pszFilePath )
{
	int	nLineNum = 1 ;
	m_strFilePath = pszFilePath ;
	if ( m_csstrFileName.m_varStr.IsEmpty() )
	{
		m_csstrFileName.m_varStr = pszFilePath ;
	}
	for ( ; ; )
	{
		//
		// １行取得
		//
		ECSSourceStream	cssLine ;
		int		nFirstLine = nLineNum ;
		GetNextScriptLine( cssLine, cssScript, nLineNum ) ;
		//
		// 1 行コンパイル
		//
		ESLError	err = CompileScriptLine( cssLine, nFirstLine ) ;
		if ( err )
		{
			err = OutputError
				( GetESLErrorMsg(err), pszFilePath, nFirstLine ) ;
			if ( err )
				return	err ;
		}
		//
		if ( cssScript.IsIndexOverflow() )
			break ;
	}
	return	eslErrSuccess ;
}

// １行取得する
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::GetNextScriptLine
	( ECSSourceStream & cssLine, ECSSourceStream & cssScript, int & nLineNum )
{
	while ( !cssScript.IsIndexOverflow() )
	{
		int	iLineFirst = cssScript.GetIndex( ) ;
		cssScript.MoveToNextLine( ) ;
		nLineNum ++ ;
		//
		cssLine += cssScript.Middle
			( iLineFirst, cssScript.GetIndex() - iLineFirst ) ;
		//
		if ( !(m_nMacroMode.GetValue() & mmodeDisableComment) )
		{
			cssLine.PassEnclosedString( L';', m_dwModeFlags ) ;
			if ( cssLine.CurrentCharacter() == L';' )
			{
				cssLine = cssLine.Left( cssLine.GetIndex() ) ;
			}
		}
		//
		cssLine.TrimRight( ) ;
		int		nLength = cssLine.GetLength( ) ;
		if ( nLength < 1 )
		{
			break ;
		}
		wchar_t	wchLast = cssLine.GetAt(nLength - 1) ;
		if ( (wchLast != L'\\') && (wchLast != L',')
			&& (wchLast != L'+') && (wchLast != L'(') )
		{
			break ;
		}
		if ( wchLast == L'\\' )
		{
			cssLine = cssLine.Left( nLength - 1 ) ;
		}
	}
	cssLine.MoveIndex( 0 ) ;
}

// １行コンパイルする
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileScriptLine
	( ECSSourceStream & cssLine,
		int nLineNum, const char * pszFilePath, bool fEnableUserMacro )
{
	//
	// 現在の処理行の更新
	//
	m_nLineNum = nLineNum ;
	if ( (pszFilePath != NULL) && (pszFilePath[0] != '\0') )
	{
		m_strFilePath = pszFilePath ;
		if ( m_csstrFileName.m_varStr.IsEmpty() )
		{
			m_csstrFileName.m_varStr = pszFilePath ;
		}
	}
	//
	// テキストマクロの事前処理（デフォルト動作）
	//
	EMacroNest *	pmnNest = m_nestMacro.GetLastAt( ) ;
	bool	fDefMacroMode = false ;
	if ( (pmnNest != NULL) &&
		((pmnNest->m_mwType == mwDefMacro) || (pmnNest->m_mwType == mwFor)) )
	{
		fDefMacroMode = true ;
	}
	//
	if ( !fDefMacroMode
		&& !(m_nMacroMode.GetValue()
				& (mmodePriorityTextMacro | mmodeDisableTextMacro)) )
	{
		ESLError	err = ProcessTextMacro( cssLine ) ;
		if ( err )
		{
			return	err ;
		}
	}
	//
	// ユーザー定義プリプロセッサマクロ
	//
	if ( (m_pPreprocessMacro != NULL)
		&& (m_pPreprocessFlag != NULL)
		&& (m_pPreprocessFlag->GetValue() != 0)
		&& fEnableUserMacro && (m_nMacroMode.GetValue() & mmodeEnableUserMacro)
			&& ((pmnNest == NULL)
					|| (pmnNest->m_nCondition == EMacroNest::condNormal)) )
	{
		EObjArray<EWideString>	lstParam ;
		lstParam.SetAt
			( 0, new EWideString( cssLine.Middle( cssLine.GetIndex() ) ) ) ;
		//
		ECSObject *	pRetValue = NULL ;
		ESLError	err =
			CompileUserMacro( m_pPreprocessMacro, lstParam, &pRetValue ) ;
		if ( err )
		{
			delete	pRetValue ;
			return	err ;
		}
		if ( (pRetValue == NULL)
			|| (pRetValue->m_vtType != csvtString) )
		{
			return	eslErrSuccess ;
		}
		cssLine = ((ECSString*)pRetValue)->m_varStr ;
	}
	//
	int		iLineFirst = cssLine.GetIndex( ) ;
	if ( !cssLine.DisregardSpace() )
	{
		//
		// ユーザー定義マクロ書式の判定
		//
		if ( fEnableUserMacro && (m_nMacroMode.GetValue() & mmodeEnableUserMacro)
				&& ((pmnNest == NULL)
						|| (pmnNest->m_nCondition == EMacroNest::condNormal)) )
		{
			int	i, nCount ;
			nCount = m_staMacro.GetSize( ) ;
			for ( i = 0; i < nCount; i ++ )
			{
				//
				// 書式一致判定
				//
				ETaggedElement<ECSWideString,EMacroBlock> * pElement ;
				pElement = m_staMacro.GetAt( i ) ;
				if ( pElement == NULL )
				{
					continue ;
				}
				EMacroBlock *	pmbMacro = pElement->GetObject( ) ;
				if ( (pmbMacro == NULL)
					|| (pmbMacro->m_usage.GetSize() == 0)
					|| (pElement->Tag() == L"@epilogue")
					|| (pElement->Tag() == L"@preprocess"))
				{
					continue ;
				}
				ECSObject *	pMacroFlag = m_staConstant.GetAs( pElement->Tag() ) ;
				if ( (pMacroFlag != NULL)
					&& (pMacroFlag->m_vtType == csvtInteger)
					&& (((ECSInteger*)pMacroFlag)->GetValue() == 0) )
				{
					continue ;
				}
				EObjArray<EWideString>	lstParam ;
				cssLine.MoveIndex( iLineFirst ) ;
				ESLError	err =
					cssLine.IsMatchUsageList
						( pmbMacro->m_usage, 0, m_strErrMsg, &lstParam ) ;
				if ( err )
				{
					continue ;
				}
				//
				// マクロ処理
				//
				ECSObject *	pRetValue = NULL ;
				err = CompileUserMacro( pmbMacro, lstParam, &pRetValue ) ;
				if ( err )
				{
					delete	pRetValue ;
					return	err ;
				}
				if ( (pRetValue == NULL)
					|| (pRetValue->m_vtType != csvtInteger)
					|| (((ECSInteger*)pRetValue)->GetValue() == 0) )
				{
					return	eslErrSuccess ;
				}
			}
		}
	}
	//
	// テキストマクロの事後処理
	//
	if ( !fDefMacroMode
		&& !(m_nMacroMode.GetValue() & mmodeDisableTextMacro)
		&& (m_nMacroMode.GetValue() & mmodePriorityTextMacro) )
	{
		ESLError	err = ProcessTextMacro( cssLine ) ;
		if ( err )
		{
			return	err ;
		}
		iLineFirst = 0 ;
	}
	if ( m_nMacroMode.GetValue() & mmodeDisableComment )
	{
		//
		// ※プリプロセスでコメント処理が無効化されている場合、
		// マクロ処理後の文に対してコメント処理を行う
		//
		cssLine.MoveIndex( iLineFirst ) ;
		cssLine.PassEnclosedString( L';', m_dwModeFlags ) ;
		if ( cssLine.CurrentCharacter() == L';' )
		{
			cssLine = cssLine.Left( cssLine.GetIndex() ) ;
		}
		iLineFirst = 0 ;
	}
	//
	// マクロ文の判定
	//
	cssLine.MoveIndex( iLineFirst ) ;
	ECSWideString	wstrToken = cssLine.GetAToken( ) ;
	MacroWord		mwIndex = IsMacroWord( wstrToken ) ;
	if ( pmnNest != NULL )
	{
		if ( pmnNest->m_mwType == mwDefMacro )
		{
			if ( mwIndex != mwEndMacro )
			{
				if ( pmnNest->m_nCondition == EMacroNest::condNormal )
				{
					pmnNest->m_lstCodeBuf.Add( cssLine.Duplicate() ) ;
				}
				return	eslErrSuccess ;
			}
		}
		else if ( pmnNest->m_mwType == mwFor )
		{
			if ( mwIndex == mwFor )
			{
				pmnNest->m_nNestCounter ++ ;
			}
			else if ( mwIndex == mwNext )
			{
				pmnNest->m_nNestCounter -- ;
			}
			if ( pmnNest->m_nNestCounter >= 0 )
			{
				if ( pmnNest->m_nCondition == EMacroNest::condNormal )
				{
					pmnNest->m_lstCodeBuf.Add( cssLine.Duplicate() ) ;
				}
				return	eslErrSuccess ;
			}
		}
		else if ( (pmnNest->m_mwType >= mwIf)
					&& (pmnNest->m_mwType <= mwEndIf) )
		{
			if ( pmnNest->m_nCondition != EMacroNest::condNormal )
			{
				if ( (mwIndex < mwIf) || (mwIndex > mwEndIf) )
				{
					return	eslErrSuccess ;
				}
			}
		}
	}
	if ( wstrToken.IsEmpty() )
	{
		return	eslErrSuccess ;
	}
	if ( mwIndex != mwInvalid )
	{
		ESLAssert( (mwIndex >= 0) && (mwIndex < mwMax) ) ;
		ESLError	err =
			(this->*m_pfnCompileMacroWord[mwIndex])( cssLine ) ;
		if ( !err )
		{
			wstrToken = cssLine.GetAToken( ) ;
			if ( !wstrToken.IsEmpty() )
			{
				m_strErrMsg = "ステートメントの末尾に謎の構文 \'"
						+ EString(wstrToken) + "\' を発見しました。" ;
				err = ESLErrorMsg( m_strErrMsg ) ;
			}
		}
		return	err ;
	}
	for ( unsigned int iNest = 0; iNest < m_nestMacro.GetSize(); iNest ++ )
	{
		EMacroNest *	pmnNest = m_nestMacro.GetLastAt( iNest ) ;
		if ( (pmnNest != NULL) && pmnNest->m_fMacroFunction )
		{
			return	ESLErrorMsg
				( "マクロ関数内では擬似命令以外は使用できません。" ) ;
		}
	}
	//
	// 予約語の判定
	//
	ReservedWord	rwIndex = IsReservedWord( wstrToken ) ;
	if ( (m_rwCtrlType == rwAssembler) && (rwIndex != rwEndAssembler) )
	{
		//
		// インラインアセンブラ
		//
		cssLine.MoveIndex( iLineFirst ) ;
		return	CompileAssembleLine1( cssLine ) ;
	}
	else if ( (m_rwCtrlType == rwData) && (rwIndex != rwEndData) )
	{
		//
		// 定数定義文の処理
		//
		cssLine.MoveIndex( iLineFirst ) ;
		return	CompileConstant( cssLine ) ;
	}
	else if ( (m_rwCtrlType == rwEnumerator) && (rwIndex != rwEndEnum) )
	{
		//
		// 列挙子文の処理
		//
		cssLine.MoveIndex( iLineFirst ) ;
		return	CompileEnumerate( cssLine ) ;
	}
	if ( rwIndex != rwInvalid )
	{
		ESLAssert( (rwIndex >= 0) && (rwIndex < rwMax) ) ;
		ESLError	err =
			(this->*m_pfnCompileReservedWord[rwIndex])( cssLine ) ;
		if ( !err )
		{
			wstrToken = cssLine.GetAToken( ) ;
			if ( !wstrToken.IsEmpty() )
			{
				m_strErrMsg = "ステートメントの末尾に謎の構文 \'"
						+ EString(wstrToken) + "\' を発見しました。" ;
				err = ESLErrorMsg( m_strErrMsg ) ;
			}
		}
		return	err ;
	}
	//
	// 変数定義の判定
	//
	ECSTypeInfo	typeTemp ;
	cssLine.MoveIndex( iLineFirst ) ;
	if ( !ParseTypeDescription( typeTemp, cssLine ) )
	{
		OPERATOR_INFO	opinf ;
		if ( GetOperatorInfo( opinf, cssLine.GetAToken(), false ) )
		{
			cssLine.MoveIndex( iLineFirst ) ;
			return	CompileVariable( cssLine ) ;
		}
	}
	cssLine.MoveIndex( iLineFirst ) ;
	//
	if ( (m_rwCtrlType == rwStructure) || (m_rwCtrlType == rwClass) )
	{
		//
		// クラス定義ブロック内での式
		//
		if ( m_rwCtrlType == rwStructure )
		{
			return	ESLErrorMsg( "Structure ブロック内で不正な文です。" ) ;
		}
		return	ESLErrorMsg( "Class ブロック内で不正な文です。" ) ;
	}
	else
	{
		//
		// 式文の処理
		//
		return	CompileExpressionStatement( cssLine ) ;
	}
}

// コンパイルを完了する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::FinishCompile( DWORD dwFlags )
{
	//
	// エピローグマクロ展開
	//
	EMacroBlock *	pmbMacro = m_staMacro.GetAs( L"@epilogue" ) ;
	if ( pmbMacro != NULL )
	{
		EObjArray<EWideString>	lstParam ;
		ESLError	err = CompileUserMacro( pmbMacro, lstParam ) ;
		if ( err )
		{
			err = OutputError
				( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
			if ( err )
				return	err ;
		}
	}
	//
	// 制御ブロックの終了チェック
	//
	if ( m_rwCtrlType != rwInvalid )
	{
		const char *	pszErrMsg ;
		switch ( m_rwCtrlType )
		{
		case	rwData:
			pszErrMsg = "Data ブロックが閉じられていません。" ;
			break ;
		case	rwStructure:
			pszErrMsg = "Structure ブロックが閉じられていません。" ;
			break ;
		case	rwFunction:
			pszErrMsg = "Function ブロックが閉じられていません。" ;
			break ;
		case	rwIf:
		case	rwElseIf:
		case	rwElse:
			pszErrMsg = "If ブロックが閉じられていません。" ;
			break ;
		case	rwBegin:
			pszErrMsg = "Begin ブロックが閉じられていません。" ;
			break ;
		case	rwFor:
			pszErrMsg = "For ブロックが閉じられていません。" ;
			break ;
		case	rwWhile:
			pszErrMsg = "While ブロックが閉じられていません。" ;
			break ;
		case	rwRepeat:
			pszErrMsg = "Repeat ブロックが閉じられていません。" ;
			break ;
		default:
			pszErrMsg = "制御ブロックが閉じられていません。" ;
			break ;
		}
		ESLError	err =
			OutputError( pszErrMsg, m_strFilePath, m_nLineNum ) ;
		if ( err )
			return	err ;
	}
	//
	// 大域初期化関数を設定
	//
	if ( m_csxiInitFunc.m_bufImage.GetLength() > m_dwInitPrologueSize )
	{
		m_csxiInitFunc.WriteInstructionCode( csicReturn ) ;
		m_csxiInitFunc.WriteByteCode( 1 ) ;
		m_csxiInitFunc.CommitImage( ) ;
		//
		DWORD	dwPrologueFunc = m_pcsxiDst->AlignCodeBuffer() ;
		m_pcsxiDst->CommitImage( ) ;
		//
		ESLError	err =
			m_pcsxiDst->MergeImage
				( m_csxiInitFunc, ECSExecutionImageLinker::mfMirrorClassInf ) ;
		if ( err )
		{
			return	ESLErrorMsg
				( "コンパイルコードのマージ処理でエラーが発生しました。" ) ;
		}
		m_pcsxiDst->m_pifPrologue.Add( dwPrologueFunc ) ;
	}
	if ( m_csxiNakedPrologue.m_bufImage.GetLength() > 0 )
	{
		m_csxiNakedPrologue.WriteSakuraReturn() ;
		m_csxiNakedPrologue.CommitImage( ) ;
		//
		DWORD	dwPrologueFunc = m_pcsxiDst->AlignCodeBuffer() ;
		m_pcsxiDst->CommitImage( ) ;
		//
		ESLError	err =
			m_pcsxiDst->MergeImage
				( m_csxiNakedPrologue, ECSExecutionImageLinker::mfMirrorClassInf ) ;
		if ( err )
		{
			return	ESLErrorMsg
				( "コンパイルコードのマージ処理でエラーが発生しました。" ) ;
		}
		m_pcsxiDst->m_pifNakedPrologue.Add( dwPrologueFunc ) ;
	}
	if ( m_csxiNakedEpilogue.m_bufImage.GetLength() > 0 )
	{
		m_csxiNakedEpilogue.WriteSakuraReturn() ;
		m_csxiNakedEpilogue.CommitImage( ) ;
		//
		DWORD	dwEpilogueFunc = m_pcsxiDst->AlignCodeBuffer() ;
		m_pcsxiDst->CommitImage( ) ;
		//
		ESLError	err =
			m_pcsxiDst->MergeImage
				( m_csxiNakedEpilogue, ECSExecutionImageLinker::mfMirrorClassInf ) ;
		if ( err )
		{
			return	ESLErrorMsg
				( "コンパイルコードのマージ処理でエラーが発生しました。" ) ;
		}
		m_pcsxiDst->m_pifNakedEpilogue.Add( dwEpilogueFunc ) ;
	}
	//
	// インライン関数を選択して結合
	//
	while ( CommitInlineFunctions( dwFlags ) > 0 )
	{
	}
	m_pcsxiDst->CommitImage( ) ;
	//
	return	eslErrSuccess ;
}

// インライン関数を結合する
//////////////////////////////////////////////////////////////////////////////
int ECSCompiler::CommitInlineFunctions( DWORD dwFlags )
{
	int	nMergeCount = 0 ;
	for ( int i = 0; i < (int) m_wstaInlineFuncs.GetSize(); i ++ )
	{
		EWideString *	pwstrFuncName = m_wstaInlineFuncs.GetTagAt( i ) ;
		if ( pwstrFuncName == NULL )
		{
			continue ;
		}
		if ( !(dwFlags & flagForceImplementAll)
			&& (m_pcsxiDst->m_impFuncRef.GetAs( *pwstrFuncName ) == NULL)
			&& (m_pcsxiDst->m_impNakedFuncRef.GetAs( *pwstrFuncName ) == NULL) )
		{
			continue ;
		}
		ECSExecutionImageCompiler *
			pcsxiFunc = m_wstaInlineFuncs.GetObjectAt( i ) ;
		if ( pcsxiFunc == NULL )
		{
			continue ;
		}
		//
		LinkInlineFunction( pcsxiFunc ) ;
		//
		nMergeCount ++ ;
		m_wstaInlineFuncs.RemoveAt( i -- ) ;
	}
	return	nMergeCount ;
}

void ECSCompiler::LinkInlineFunction( ECSExecutionImageCompiler * pcsxiFunc )
{
	pcsxiFunc->AlignCodeBuffer() ;
	pcsxiFunc->CommitImage() ;
	m_pcsxiDst->AlignCodeBuffer() ;
	m_pcsxiDst->CommitImage( ) ;
	//
	ESLError	err =
		m_pcsxiDst->MergeImage
			( *pcsxiFunc, ECSExecutionImageLinker::mfMirrorClassInf ) ;
	if ( err )
	{
		OutputError
			( "インライン関数のマージ処理でエラーが発生しました。" ) ;
	}
}

// 指定のパスのファイルを開く
//////////////////////////////////////////////////////////////////////////////
ESLFileObject * ECSCompiler::OpenScriptFile( const char * pszFilePath )
{
	ERawFile *	pfile = new ERawFile ;
	if ( pfile->Open( pszFilePath,
			ESLFileObject::modeRead | ESLFileObject::shareRead ) )
	{
		delete	pfile ;
		return	NULL ;
	}
	return	pfile ;
}

// エラーを出力する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::OutputError
	( const char * pszErrMsg, const char * pszFilePath, int nLineNum )
{
	m_nErrorCount ++ ;
	return	eslErrSuccess ;
}

// 警告を出力する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::OutputWarning
	( const char * pszErrMsg, const char * pszFilePath, int nLineNum )
{
	m_nWarningCount ++ ;
	return	eslErrSuccess ;
}

ESLError ECSCompiler::OutputWarning0
	( const char * pszErrMsg,
		const char * pszFilePath, int nLineNum )
{
	if ( m_nWarningLevel <= 0 )
	{
		return	eslErrSuccess ;
	}
	if ( pszFilePath == NULL )
	{
		pszFilePath = m_strFilePath ;
	}
	if ( nLineNum <= 0 )
	{
		nLineNum = m_nLineNum ;
	}
	return	OutputWarning( pszErrMsg, pszFilePath, nLineNum ) ;
}

ESLError ECSCompiler::OutputWarning1
	( const char * pszErrMsg,
		const char * pszFilePath, int nLineNum )
{
	if ( m_nWarningLevel <= 1 )
	{
		return	eslErrSuccess ;
	}
	if ( pszFilePath == NULL )
	{
		pszFilePath = m_strFilePath ;
	}
	if ( nLineNum <= 0 )
	{
		nLineNum = m_nLineNum ;
	}
	return	OutputWarning( pszErrMsg, pszFilePath, nLineNum ) ;
}

ESLError ECSCompiler::OutputWarning2
	( const char * pszErrMsg,
		const char * pszFilePath, int nLineNum )
{
	if ( m_nWarningLevel <= 2 )
	{
		return	eslErrSuccess ;
	}
	if ( pszFilePath == NULL )
	{
		pszFilePath = m_strFilePath ;
	}
	if ( nLineNum <= 0 )
	{
		nLineNum = m_nLineNum ;
	}
	return	OutputWarning( pszErrMsg, pszFilePath, nLineNum ) ;
}

ESLError ECSCompiler::OutputWarning3
	( const char * pszErrMsg,
		const char * pszFilePath, int nLineNum )
{
	if ( m_nWarningLevel <= 3 )
	{
		return	eslErrSuccess ;
	}
	if ( pszFilePath == NULL )
	{
		pszFilePath = m_strFilePath ;
	}
	if ( nLineNum <= 0 )
	{
		nLineNum = m_nLineNum ;
	}
	return	OutputWarning( pszErrMsg, pszFilePath, nLineNum ) ;
}

// テキストマクロを処理する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::ProcessTextMacro( ECSSourceStream & cssLine )
{
	EWideString		wstrLine ;
	ECSWideString	wstrToken ;
	const wchar_t *	pwszLine = cssLine ;
	int				iLast = cssLine.GetIndex() ;
	int				nLength = cssLine.GetLength( ) ;
	while ( !cssLine.DisregardSpace() )
	{
		int	iCurrent = cssLine.GetIndex( ) ;
		if ( (iCurrent + 2 <= nLength)
			&& (pwszLine[iCurrent] == L'@')
			&& (pwszLine[iCurrent + 1] == L'<') )
		{
			//
			// @<> 演算子
			//
			wstrLine += cssLine.Middle( iLast, iCurrent - iLast ) ;
			//
			ECSObject *	pMacro = NULL ;
			cssLine.MoveIndex( iCurrent + 2 ) ;
			ESLError	err =
				CalculateExpression( pMacro, cssLine, 0, L">" ) ;
			if ( err )
			{
				return	err ;
			}
			if ( cssLine.HasToComeChar( L">" ) != L'>' )
			{
				m_strErrMsg +=
					"テキストマクロ演算子が \'>\' 括弧で閉じられていません。" ;
				return	ESLErrorMsg( m_strErrMsg ) ;
			}
			if ( pMacro->m_vtType != csvtString )
			{
				m_strErrMsg +=
					"テキストマクロ演算子のパラメータが文字列型でありません。" ;
				return	ESLErrorMsg( m_strErrMsg ) ;
			}
			//
			wstrLine += ((ECSString*)pMacro)->m_varStr ;
			delete	pMacro ;
			//
			iLast = cssLine.GetIndex( ) ;
			continue ;
		}
		else
		{
			int	nTokenType ;
			cssLine.PassAToken( &nTokenType ) ;
			if ( nTokenType == 0 )
			{
				const wchar_t *	pwszLine = cssLine ;
				unsigned int	nLength = cssLine.GetIndex() - iCurrent ;
				pwszLine += iCurrent ;
				wchar_t *		pwszBuf = wstrToken.GetBuffer( nLength ) ;
				for ( unsigned int i = 0; i < nLength; i ++ )
				{
					pwszBuf[i] = pwszLine[i] ;
				}
				wstrToken.ReleaseBuffer( nLength ) ;
				//
				ECSSourceStream *
					pcssExpr = m_staLiteral.GetAsPtr( &wstrToken ) ;
				if ( pcssExpr != NULL )
				{
					//
					// リテラル置き換え
					//
					ECSObject *	pMacro = NULL ;
					pcssExpr->MoveIndex( 0 ) ;
					ESLError	err =
						CalculateExpression( pMacro, *pcssExpr ) ;
					if ( err )
					{
						return	err ;
					}
					if ( pMacro->m_vtType != csvtString )
					{
						m_strErrMsg +=
							"リテラルの評価値が文字列型でありません。" ;
						return	ESLErrorMsg( m_strErrMsg ) ;
					}
					//
					wstrLine += ((ECSString*)pMacro)->m_varStr ;
					delete	pMacro ;
					//
					iLast = cssLine.GetIndex( ) ;
					continue ;
				}
			}
			cssLine.MoveIndex( iCurrent ) ;
		}
		wchar_t	wch = pwszLine[iCurrent] ;
		if ( (wch == L'\"') | (wch == L'\'') )
		{
			cssLine.PassEnclosedString( wch, m_dwModeFlags ) ;
			if ( cssLine.CurrentCharacter() == wch )
			{
				cssLine.GetCharacter( ) ;
			}
		}
		else
		{
			cssLine.PassAToken( ) ;
		}
	}
	wstrLine += cssLine.Middle( iLast ) ;
	cssLine = wstrLine ;
	return	eslErrSuccess ;
}

// 予約語比較（第一パラメータは予約語）
//////////////////////////////////////////////////////////////////////////////
int ECSCompiler::CompareReservedWord
	( const wchar_t * pwszReservedWord,
				const wchar_t * pwszSymbol ) const
{
	if ( m_dwModeFlags & flagRealReservedWord )
	{
		return	EWideString::Compare( pwszReservedWord, pwszSymbol ) ;
	}
	else if ( m_dwModeFlags & flagSmallReservedWord )
	{
		return	CompareStringSmallCase( pwszReservedWord, pwszSymbol ) ;
	}
	return	EWideString::CompareNoCase( pwszReservedWord, pwszSymbol ) ;
}

// 文字列比較（第一パラメータを強制的に小文字アルファベットとして）
//////////////////////////////////////////////////////////////////////////////
int ECSCompiler::CompareStringSmallCase
	( const wchar_t * pwszSmall, const wchar_t * pwszCase )
{
	int	i ;
	if ( pwszCase == NULL )
	{
		if ( (pwszSmall == NULL)
			|| (pwszSmall[0] == L'\0') )
		{
			return	0 ;
		}
		return	1 ;
	}
	for ( i = 0; pwszSmall[i]; i ++ )
	{
		wchar_t	wchSmall = pwszSmall[i] ;
		if ( (wchSmall >= L'A') && (wchSmall <= L'Z') )
		{
			wchSmall += L'a' - L'A' ;
		}
		int	nCompare = (int) wchSmall - (int) pwszCase[i] ;
		if ( nCompare != 0 )
		{
			return	nCompare ;
		}
	}
	return	- (int) pwszCase[i] ;
}

// 予約語判定
//////////////////////////////////////////////////////////////////////////////
ECSCompiler::ReservedWord
	ECSCompiler::IsReservedWord( const wchar_t * pwszToken ) const
{
	static const wchar_t *	pwszReservedWord[rwMax] =
	{
		L"Include",		L"Option",	L"DeclareType",
		L"DeclareDef",	L"ExternDef",	L"TypeDef",
		L"Variable",	L"Constant",
		L"Data",		L"EndData",
		L"Enumerator",	L"EndEnum",
		L"Structure",	L"EndStruct",
		L"Class",	L"EndClass",
		L"Namespace",	L"EndNamespace",
		L"Union",	L"EndUnion",
		L"Assembler", L"EndAssembler",
		L"Public",	L"Protected",	L"Private",
		L"Prototype",	L"Function",	L"EndFunc",
		L"If",	L"ElseIf",	L"Else",	L"EndIf",
		L"Begin",	L"End",
		L"Break",	L"Continue",	L"Return",
		L"Goto",	L"Label",
		L"Try",		L"Catch",	L"EndTry",	L"Throw",
		L"For",		L"Next",
		L"While",	L"EndWhile",
		L"Repeat",	L"Until",
		L"Switch",	L"EndSwitch",
		L"Case",	L"Default",
		L"_m_fence",
		L"Template",	L"EndTemplate", 
		L"Using",	L"Friend",
	} ;
	for ( int i = 0; i < rwMax; i ++ )
	{
		if ( !CompareReservedWord
				( pwszReservedWord[i], pwszToken ) )
		{
			return	(ECSCompiler::ReservedWord) i ;
		}
	}
	return	rwInvalid ;
}

// マクロ予約語判定
//////////////////////////////////////////////////////////////////////////////
ECSCompiler::MacroWord
	ECSCompiler::IsMacroWord( const wchar_t * pwszToken ) const
{
	static const wchar_t *	pwszMacroWord[mwMax] =
	{
		L"@Error",	L"@Warning",	L"@Compile",
		L"@If",		L"@ElseIf",	L"@Else",	L"@EndIf",
		L"@Let",	L"@Local",	L"@For",	L"@Next",
		L"@Literal",	L"@Macro",	L"@EndMacro",
		L"@ExitMacro",	L"@UndefMacro",
	} ;
	for ( int i = 0; i < mwMax; i ++ )
	{
		if ( !CompareReservedWord
				( pwszMacroWord[i], pwszToken ) )
		{
			return	(ECSCompiler::MacroWord) i ;
		}
	}
	return	mwInvalid ;
}

// 予約語文処理
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileReservedWord
	( ECSCompiler::ReservedWord rwIndex, ECSSourceStream & cssLine )
{
	ESLAssert( rwIndex < rwMax ) ;
	return	(this->*m_pfnCompileReservedWord[rwIndex])( cssLine ) ;
}

// 式文処理
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileExpressionStatement( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwInvalid, stExpression ) )
	{
		return	eslErrSuccess ;
	}
	ESLError	err ;
	ECSTypeInfo	typeTemp ;
	BeginSakura2Optimize() ;
	err = CompileExpression( typeTemp, cssLine ) ;
	CompileCodeFreeStack( typeTemp ) ;
	FinishSakura2Optimize() ;
	FreeExpressionTemporary() ;
	return	err ;
}

// 式文処理
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileExpression
	( ECSTypeInfo & typeinf,
		ECSSourceStream & cssLine,
		DWORD dwFlags, int nPriotiy, const wchar_t * pwszExit )
{
	//
	// 第一項処理
	//////////////////////////////////////////////////////////////////////////
	if ( cssLine.DisregardSpace() )
	{
		return	ESLErrorMsg( "式の解析中に行末に到達しました。" ) ;
	}
	if ( pwszExit != NULL )
	{
		wchar_t	wchExit = cssLine.HasToComeChar( pwszExit ) ;
		if ( wchExit != L'\0' )
		{
			m_strErrMsg = "式の解析中に \'" ;
			m_strErrMsg += (char) wchExit ;
			m_strErrMsg += "\' 記号を発見しました" ;
			return	ESLErrorMsg( m_strErrMsg ) ;
		}
	}
	ESLError	err ;
	OPERATOR_INFO	opinf ;
	do
	{
		//
		// 即値判定
		//////////////////////////////////////////////////////////////////////
		bool	fRealNumber ;
		int		nRadix = GetNumberLiteralRadix( cssLine, fRealNumber ) ;
		if ( nRadix >= 0 )
		{
			//
			// 整数・実数値リテラル
			//////////////////////////////////////////////////////////////////
			if ( !fRealNumber )
			{
				INT64	nValue ;
				err = GetIntegerLiteral( nValue, cssLine, nRadix ) ;
				if ( err )
				{
					return	err ;
				}
				if ( !m_modeNakedCode )
				{
					CompileImmediateInteger( nValue ) ;
				}
				typeinf = ECSTypeInfo
							( new ECSInteger( nValue ),
								ECSTypeInfo::flagDeterministic ) ;
				typeinf.NormalzieImmediateIntegerType() ;
			}
			else
			{
				REAL64	rValue ;
				err = GetRealLiteral( rValue, cssLine, nRadix ) ;
				if ( err )
				{
					return	err ;
				}
				if ( !m_modeNakedCode )
				{
					CompileImmediateReal( rValue ) ;
				}
				typeinf = ECSTypeInfo
							( new ECSReal( rValue ),
								ECSTypeInfo::flagDeterministic ) ;
			}
			break ;
		}
		wchar_t	wch ;
		wch = cssLine.CurrentCharacter( ) ;
		if ( (wch == L'L')
			&& (m_dwModeFlags & flagCStyleNumberLiteral) )
		{
			wchar_t	wchNext = cssLine.GetAt( cssLine.GetIndex() + 1 ) ;
			if ( (wchNext == L'\"') || (wchNext == L'\'') )
			{
				cssLine.GetCharacter() ;
				wch = cssLine.CurrentCharacter( ) ;
			}
		}
		if ( (wch == L'\"') || (wch == L'\'') )
		{
			//
			// 文字列リテラル
			//////////////////////////////////////////////////////////////////
			EWideString	wstrStr ;
			err = GetStringLiteral( wstrStr, cssLine, wch ) ;
			if ( err )
			{
				return	err ;
			}
			if ( (wch == L'\'')
				&& (m_dwModeFlags & flagQuoteCharactorCode) )
			{
				INT64	nCode = 0 ;
				for ( unsigned int i = 0; i < 4; i ++ )
				{
					if ( i >= wstrStr.GetLength() )
					{
						break ;
					}
					nCode = (nCode << 16) | (wstrStr.GetAt(i) & 0xFFFF) ;
				}
				if ( !m_modeNakedCode )
				{
					CompileImmediateInteger( nCode ) ;
				}
				typeinf = ECSTypeInfo
					( new ECSInteger( nCode, ECSInteger::m_maskUint16 ),
											ECSTypeInfo::flagDeterministic ) ;
			}
			else
			{
				CompileImmediateString( wstrStr ) ;
				//
				if ( m_modeNakedCode )
				{
					typeinf.MakePointerOf
						( ECSTypeInfo
							( new ECSInteger( 0, ECSInteger::m_maskUint16 ),
												ECSTypeInfo::flagConstant ) ) ;
					typeinf.m_dwFlags |= ECSTypeInfo::flagNakedBuffer ;
					typeinf.SetLoadedRegister
							( GetExpressionRegister() ) ;
				}
				else
				{
					typeinf = ECSTypeInfo
								( new ECSString( wstrStr ),
									ECSTypeInfo::flagDeterministic ) ;
				}
			}
			break ;
		}
		else if ( wch == L'(' )
		{
			//
			// 括弧記号
			//////////////////////////////////////////////////////////////////
			cssLine.GetCharacter( ) ;
			if ( m_dwModeFlags & flagCStyleCast )
			{
				int	nIndex = cssLine.GetIndex() ;
				ECSTypeInfo	typeCast ;
				err = ParseTypeDescription( typeCast, cssLine ) ;
				if ( !err && (cssLine.HasToComeChar( L")" ) == L')') )
				{
					//
					// C 言語互換キャスト記述
					//
					ECSTypeInfo	typeExpr ;
					err = CompileExpression
						( typeExpr, cssLine,
							(dwFlags & exprInheritedFlags),
							oppCast, pwszExit ) ;
					if ( err )
					{
						return	err ;
					}
					err = CompileTypeCast
						( typeinf, typeCast, typeExpr,
							castStatic, ECSTypeInfo::flagPublic ) ;
					if ( err )
					{
						return	err ;
					}
					break ;
				}
				cssLine.MoveIndex( nIndex ) ;
			}
			err = CompileExpression
				( typeinf, cssLine, (dwFlags & exprInheritedFlags), 0, L")" ) ;
			if ( err )
			{
				return	err ;
			}
			if ( cssLine.HasToComeChar( L")" ) != L')' )
			{
				return	ESLErrorMsg
					( "\'(\' に対応する \')\' が見つかりません。" ) ;
			}
			break ;
		}
		else if ( wch == L'[' )
		{
			//
			// 間接参照
			//////////////////////////////////////////////////////////////////
			if ( m_modeNakedCode )
			{
				return	ESLErrorMsg
					( "naked モードで動的変数参照はサポートされません" ) ;
			}
			m_pcsxi->WriteInstructionCode( csicLoad ) ;
			m_pcsxi->WriteObjectModeCode( csomAuto ) ;
			m_pcsxi->WriteVariableTypeCode( csvtReference ) ;
			//
			cssLine.GetCharacter( ) ;
			//
			ECSTypeInfo	typeTemp ;
			err = CompileExpression
				( typeTemp, cssLine,
					(dwFlags & exprInheritedFlags), 0, L"]" ) ;
			if ( err )
			{
				return	err ;
			}
			if ( typeTemp.IsVoid() )
			{
				return	ESLErrorMsg( "void データでの間接参照です。" ) ;
			}
			if ( cssLine.HasToComeChar( L"]" ) != L']' )
			{
				return	ESLErrorMsg
					( "\'[\' に対応する \']\' が見つかりません。" ) ;
			}
			//
			m_pcsxi->WriteInstructionCode( csicElementIndirect ) ;
			//
			typeinf = ECSTypeInfo( new ECSReference ) ;
			break ;
		}
		else if ( wch == L'{' )
		{
			//
			// 配列構築
			//////////////////////////////////////////////////////////////////
			if ( m_modeNakedCode )
			{
				return	ESLErrorMsg
					( "naked モードで配列即値はサポートされません" ) ;
			}
			cssLine.GetCharacter( ) ;
			//
			CompileImmediateBasicVariable( csvtArray ) ;
			//
			ECSArray *	pArrayType = new ECSArray ;
			ECSTypeInfo	typeElement ;
			bool		fElementType = false ;
			bool		fElementCompleted = false ;
			//
			wch = cssLine.HasToComeChar( L"}" ) ;
			while ( wch != L'}' )
			{
				if ( cssLine.HasToComeChar( L"}" ) == L'}' )
				{
					break ;
				}
				ECSTypeInfo	typeTemp ;
				err = CompileExpression
					( typeTemp, cssLine,
						(dwFlags & exprInheritedFlags), 0, L",}" ) ;
				if ( err )
				{
					delete	pArrayType ;
					return	err ;
				}
				err = VerifyTypeNoVoid( typeTemp ) ;
				if ( err )
				{
					delete	pArrayType ;
					return	err ;
				}
				CompileCodeOperate( csotAdd ) ;
				//
				ECSTypeInfo	typeNakedExpr ;
				typeNakedExpr.MakeNakedOf( typeTemp ) ;
				//
				if ( fElementType )
				{
					if ( fElementCompleted )
					{
						if ( !typeNakedExpr.IsTypeEqual( typeElement ) )
						{
							fElementCompleted = false ;
						}
					}
				}
				else
				{
					typeElement = typeNakedExpr ;
					fElementType = true ;
					fElementCompleted = true ;
				}
				//
				wch = cssLine.HasToComeChar( L",}" ) ;
				if ( wch == L'\0' )
				{
					delete	pArrayType ;
					return	ESLErrorMsg
						( "\'{\' に対応する \'}\' が見つかりません。" ) ;
				}
			}
			if ( fElementCompleted )
			{
				pArrayType->SetDefaultElement
						( typeElement.DuplicateType() ) ;
			}
			typeinf = ECSTypeInfo
				( pArrayType, ECSTypeInfo::flagDeterministic ) ;
			break ;
		}
		//
		// トークン取得（名前空間記述）
		//////////////////////////////////////////////////////////////////////
		EWideString	wstrToken = cssLine.GetAToken() ;
		bool		fGlobalScope = false ;
		if ( wstrToken == L"::" )
		{
			wstrToken = cssLine.GetAToken() ;
			fGlobalScope = true ;
		}
		SYMBOL_NAMESPACE	snsSymbol = wstrToken ;
		err = ParseFullNameSymbol( snsSymbol, cssLine, false, true, true ) ;
		if ( err )
		{
			return	err ;
		}
		wstrToken = snsSymbol.wstrName ;
		EWideString	wstrGlobalName = snsSymbol.wstrFullName ;
		EWideString	wstrNameSpace = snsSymbol.wstrNamespace ;
		//
		// 定数・マクロ判定
		//////////////////////////////////////////////////////////////////////
		MacroExpression	mxprResult ;
		err = CompileMacroExpression
				( wstrGlobalName, cssLine, typeinf, mxprResult ) ;
		if ( err )
		{
			return	err ;
		}
		if ( mxprResult.fValidMacro )
		{
			if ( mxprResult.pValue != NULL )
			{
				err = CompileImmediateObject( typeinf, mxprResult.pValue ) ;
				if ( mxprResult.fOwnValue )
				{
					delete	mxprResult.pValue ;
				}
				if ( err )
				{
					return	err ;
				}
			}
			break ;
		}
		//
		// 単項演算子判定
		//////////////////////////////////////////////////////////////////////
		if ( !GetOperatorInfo( opinf, wstrGlobalName, true ) )
		{
			if ( opinf.opiType == optNew )
			{
				err = CompileNewExpression( typeinf, cssLine ) ;
			}
			else
			{
				if ( (opinf.opiType == optExtraUniary)
					&& (opinf.xuoptExUnary == csxuotDeselect) )
				{
					//
					// deselect [] 判定
					//
					int	nIndex = cssLine.GetIndex() ;
					if ( cssLine.HasToComeChar( L"[" ) == L'[' )
					{
						if ( cssLine.HasToComeChar( L"]" ) == L']' )
						{
							opinf.xuoptExUnary = csxuotDeleteArray ;
						}
						else
						{
							cssLine.MoveIndex( nIndex ) ;
						}
					}
				}
				else if ( (opinf.opiType == optExtraUniary)
						&& (opinf.xuoptExUnary == csxuotSizeOf) )
				{
					//
					// sizeof(type) 判定
					//
					bool	fSizeOf = false ;
					int		nNakedSize = 0 ;
					err = ParseNakedSizeOfTypeOperator
								( cssLine, fSizeOf, nNakedSize ) ;
					if ( fSizeOf )
					{
						if ( err )
						{
							return	err ;
						}
						if ( !m_modeNakedCode )
						{
							CompileImmediateInteger( nNakedSize ) ;
						}
						typeinf = ECSTypeInfo
							( new ECSInteger( nNakedSize, ECSInteger::m_maskUint32 ),
												ECSTypeInfo::flagDeterministic ) ;
						break ;
					}
				}
				//
				// 単項計算
				//
				ECSTypeInfo	typeTemp ;
				err = CompileExpression
					( typeTemp, cssLine,
						(dwFlags & exprInheritedFlags),
						GetUnaryOperatorPriority(opinf), pwszExit ) ;
				if ( err )
				{
					return	err ;
				}
				//
				// 演算子処理
				//
				if ( opinf.opiType == optUnary )
				{
					err = CompileTypeUnaryOperate
							( typeinf, typeTemp, opinf.uoptUnary ) ;
				}
				else if ( opinf.opiType == optExtraUniary )
				{
					err = CompileTypeExUnaryOperate
							( typeinf, typeTemp, opinf.xuoptExUnary ) ;
				}
				else
				{
					return	ESLErrorMsg( "単項演算子で内部エラーが発生しました。" ) ;
				}
			}
			if ( err )
			{
				return	err ;
			}
			break ;
		}
		//
		// 型キャスト
		//////////////////////////////////////////////////////////////////////
		if ( (wstrGlobalName == L"static_cast")
			|| (wstrGlobalName == L"dynamic_cast") )
		{
			if ( cssLine.HasToComeChar( L"<" ) != L'<' )
			{
				m_strErrMsg =
					EString( wstrGlobalName )
						+ " に \'<\' 記号が見つかりません。" ;
				return	ESLErrorMsg( m_strErrMsg ) ;
			}
			ECSTypeInfo	typeCast ;
			if ( wstrGlobalName == L"static_cast" )
			{
				err = ParseTypeDescription( typeCast, cssLine ) ;
			}
			else
			{
				EWideString	wstrTypeName = cssLine.GetAToken() ;
				err = GetSimpleTypeInfoAs( typeCast, wstrTypeName ) ;
			}
			if ( err )
			{
				return	err ;
			}
			if ( cssLine.HasToComeChar( L">" ) != L'>' )
			{
				m_strErrMsg =
					EString( wstrGlobalName )
						+ " に閉じ \'>\' 記号が見つかりません。" ;
				return	ESLErrorMsg( m_strErrMsg ) ;
			}
			if ( cssLine.HasToComeChar( L"(" ) != L'(' )
			{
				m_strErrMsg =
					EString( wstrGlobalName )
						+ " に \'(\' 記号が見つかりません。" ;
				return	ESLErrorMsg( m_strErrMsg ) ;
			}
			ECSTypeInfo	typeExpr ;
			err = CompileExpression
				( typeExpr, cssLine,
					(dwFlags & exprInheritedFlags), 0, L")" ) ;
			if ( err )
			{
				return	err ;
			}
			if ( cssLine.HasToComeChar( L")" ) != L')' )
			{
				return	ESLErrorMsg
					( "\'(\' に対応する \')\' が見つかりません。" ) ;
			}
			if ( wstrGlobalName == L"static_cast" )
			{
				err = CompileTypeCast
					( typeinf, typeCast, typeExpr,
						castStatic, ECSTypeInfo::flagPublic ) ;
				if ( err )
				{
					return	err ;
				}
			}
			else
			{
				if ( m_modeNakedCode )
				{
					return	ESLErrorMsg
						( "naked モードで dynamic_cast はサポートされません" ) ;
				}
				EWideString	wstrCastTypeName ;
				typeCast.FormatTypeString( wstrCastTypeName ) ;
				m_pcsxi->WriteInstructionCode( csicExUniOperate ) ;
				m_pcsxi->WriteExUniOperatorTypeCode( csxuotDynamicCast ) ;
				m_pcsxi->WriteConstantString( wstrCastTypeName ) ;
				typeinf.MakeReferenceOf( typeCast ) ;
			}
			break ;
		}
		//
		// オブジェクト構築か？
		//////////////////////////////////////////////////////////////////////
		bool	fTypeConstruction ;
		err = CompileTypeConstruction
			( cssLine, typeinf,
				fTypeConstruction, wstrNameSpace, wstrToken ) ;
		if ( err )
		{
			return	err ;
		}
		if ( fTypeConstruction )
		{
			break ;
		}
		//
		// 記憶クラス取得
		//////////////////////////////////////////////////////////////////////
		const ECSClassInfo *	pClassInf ;
		ECSTypeInfo		typeVar ;
		CSObjectMode	csomClass = IsMemoryClass( wstrGlobalName ) ;
		if ( csomClass != csomImmediate )
		{
			if ( csomClass == csomThis )
			{
				int	iThisVarIndex = IsLocalVariableName( L"this", &typeinf ) ;
				if ( iThisVarIndex < 0 )
				{
					return	ESLErrorMsg
						( "この関数には this オブジェクトは存在しません。" ) ;
				}
				if ( m_modeNakedCode )
				{
					int	regTemp = AllocateExpressionRegister() ;
					m_pcsxi->WriteSakuraMoveRegReg
						( regTemp, ECSSakura2Processor::regTP ) ;
					//
					typeinf.ClearAddressingInfo() ;
					typeinf.SetLoadedRegister( regTemp ) ;
					break ;
				}
			}
			wch = cssLine.HasToComeChar( L"." ) ;
			if ( wch != L'.' )
			{
				//
				// 変数間接参照
				//
				if ( m_modeNakedCode )
				{
					return	ESLErrorMsg
						( "naked モードで記憶クラスへの"
								"動的参照はサポートされません" ) ;
				}
				m_pcsxi->WriteInstructionCode( csicLoad ) ;
				m_pcsxi->WriteObjectModeCode( csomClass ) ;
				m_pcsxi->WriteVariableTypeCode( csvtReference ) ;
				if ( csomClass == csomThis )
				{
//					typeinf.m_dwFlags &= ~ECSTypeInfo::flagProtectedMask ;
//					typeinf.m_dwFlags |= ECSTypeInfo::flagPrivate ;
				}
				else
				{
					typeinf = ECSTypeInfo( new ECSReference ) ;
				}
				break ;
			}
			wstrGlobalName = wstrToken = cssLine.GetAToken( ) ;
			wstrNameSpace = L"" ;
			snsSymbol.wstrFullName = wstrGlobalName ;
			snsSymbol.wstrName = snsSymbol.wstrFullName ;
			snsSymbol.wstrNamespace = L"" ;
		}
		else if ( wstrToken == L"parent" )
		{
			if ( IsLocalVariableName( L"this", &typeinf ) < 0 )
			{
				return	ESLErrorMsg( "parent が見つかりません。" ) ;
			}
			//
			// 親クラス参照
			//
			pClassInf = GetNakedTypeClassInfo( typeinf ) ;
			if ( pClassInf == NULL )
			{
				if ( m_modeNakedCode )
				{
					return	ESLErrorMsg
						( "naked モードでクラス情報のない parent 指定です" ) ;
				}
				CompileImmediateString( wstrToken ) ;
				//
				typeinf = ECSTypeInfo( new ECSReference ) ;
				break ;
			}
			if ( m_modeNakedCode && !pClassInf->IsNakedMemoryClass() )
			{
				return	ESLErrorMsg
					( "naked モードで naked でないクラスの parent 指定です" ) ;
			}
			if ( pClassInf->GetParentClassCount() == 0 )
			{
				return	ESLErrorMsg( "parent が見つかりません。" ) ;
			}
			else if ( pClassInf->GetParentClassCount() != 1 )
			{
				return	ESLErrorMsg
					( "複数派生のクラスで parent は使用できません。" ) ;
			}
			ECSClassInfo::ParentClass *
				pParentClass = pClassInf->GetParentClassAt( 0 ) ;
			if ( pParentClass == NULL )
			{
				return	ESLErrorMsg( "parent が見つかりません。" ) ;
			}
			if ( !m_modeNakedCode )
			{
				m_pcsxi->WriteInstructionCode( csicLoad ) ;
				m_pcsxi->WriteObjectModeCode( csomThis ) ;
				m_pcsxi->WriteVariableTypeCode( csvtReference ) ;
			}
			ECSClassInfo::CastInfo *
				pCastParent =
					pClassInf->GetCastParentClassAs
							( pParentClass->pClassInf->GetGlobalName() ) ;
			if ( pCastParent == NULL )
			{
				return	ESLErrorMsg( "親クラスへのキャスト情報が見つかりません" ) ;
			}
			if ( !m_modeNakedCode )
			{
				ECSTypeInfo	typeTemp ;
				err = CompileCastToParentClass
					( typeTemp, *pCastParent,
						*pClassInf, typeinf, ECSTypeInfo::flagPublic ) ;
				if ( err )
				{
					return	err ;
				}
			}
			typeVar.SetTypeValue
				( new ECSStructure
					( pParentClass->pClassInf ), typeinf.m_dwFlags ) ;
			if ( !m_modeNakedCode )
			{
				typeinf.MakeReferenceOf( typeVar ) ;
			}
			else
			{
				typeinf.MakePointerOf( typeVar ) ;
			}
			typeinf.m_dwFlags &= ~ECSTypeInfo::flagProtectedMask ;
			typeinf.m_dwFlags |= ECSTypeInfo::flagPrivate ;
			//
			if ( m_modeNakedCode )
			{
				typeinf.SetAddressingInfo
					( ECSSakura2Processor::regTP,
							pCastParent->nNakedOffset ) ;
			}
			break ;
		}
		else if ( fGlobalScope )
		{
			csomClass = csomGlobal ;
		}
		else
		{
			csomClass = csomAuto ;
		}
		//
		// 関数呼び出し判定
		//////////////////////////////////////////////////////////////////////
		if ( cssLine.HasToComeChar( L"(" ) == L'(' )
		{
			err = CompileVariableReference
				( typeinf, csomClass, snsSymbol, true ) ;
			if ( err )
			{
				err = CompileArgumentAndCallFunction
					( cssLine, typeinf, csomClass, wstrNameSpace, wstrToken ) ;
				if ( err )
				{
					return	err ;
				}
			}
			else
			{
				ECSFunction *	pFuncPtr = typeinf.GetTypeFunctionPointer() ;
				if ( pFuncPtr == NULL )
				{
					return	ESLErrorMsg
						( "\'(\' の左項が関数ポインタではありません" ) ;
				}
				ECSTypeInfo	typeFunc = typeinf ;
				err = CompileArgumentAndIndirectCallFunction
									( cssLine, typeinf, typeFunc ) ;
				if ( err )
				{
					return	err ;
				}
			}
			break ;
		}
		//
		// 変数判定
		//////////////////////////////////////////////////////////////////////
		err = CompileVariableReference
			( typeinf, csomClass, snsSymbol, false ) ;
		if ( err )
		{
			return	err ;
		}
	}
	while ( false ) ;
	//
	// 第二項処理
	//////////////////////////////////////////////////////////////////////////
	while ( !cssLine.DisregardSpace() )
	{
		//
		// 終了判定
		//
		int		iOpIndex = cssLine.GetIndex( ) ;
		if ( pwszExit != NULL )
		{
			if ( cssLine.HasToComeChar( pwszExit ) != L'\0' )
			{
				cssLine.MoveIndex( iOpIndex ) ;
				break ;
			}
		}
		//
		// 演算子取得
		//
		ECSWideString	wstrToken = cssLine.GetAToken( ) ;
		if ( GetOperatorInfo( opinf, wstrToken, false ) )
		{
			m_strErrMsg = "「" + EString(wstrToken)
							+ "」は有効な演算子ではありません。" ;
			cssLine.MoveIndex( iOpIndex ) ;
			return	ESLErrorMsg( m_strErrMsg ) ;
		}
		int		nCurrentPriotity = GetOperatorPriority( opinf ) ;
		if ( nCurrentPriotity <= nPriotiy )
		{
			cssLine.MoveIndex( iOpIndex ) ;
			break ;
		}
		//
		if ( (opinf.opiType == optCompareSelector)
			&& (opinf.cstSelector == cstEvaluation) )
		{
			//
			// 比較選択演算子
			//
			NormalizePointerFromLinearAddress( typeinf ) ;
			err = VerifyTypeBoolean( typeinf ) ;
			if ( err )
			{
				return	err ;
			}
			DWORD	dwCmpJumpPos ;
			dwCmpJumpPos = CompileCodeConditionalJump( false, false ) ;
			//
			ECSExecutionImageCompiler::RegisterContext	regContext ;
			m_pcsxi->GetRegisterContext( regContext ) ;
			const int	nSaveExprAlloc = m_lstExprAlloc.GetSize() ;
			const int	nSaveObjAlloc = m_objExprAlloc ;
			//
			ECSTypeInfo	typeTerm1 ;
			err = CompileExpression
				( typeTerm1, cssLine,
					(dwFlags & exprInheritedFlags),
					nCurrentPriotity, pwszExit ) ;
			if ( err )
			{
				return	err ;
			}
			MakeCommitValueToNakedRegister( typeTerm1 ) ;
			FreeExpressionRegister( typeTerm1 ) ;
			//
			if ( !cssLine.HasToComeToken( L":" ) )
			{
				return	ESLErrorMsg( "\'?\' に対応する \':\' が見つかりません" ) ;
			}
			//
			DWORD	dwEndExprJumpPos ;
			dwEndExprJumpPos = CompileCodeJump( ) ;
			//
			CompileCodeCommitJumpAddress
				( dwCmpJumpPos, CompileCodeGetCurrent() ) ;
			m_pcsxi->RestoreRegisterContext( regContext ) ;
			//
			ECSTypeInfo	typeTerm2 ;
			err = CompileExpression
					( typeTerm2, cssLine,
						(dwFlags & exprInheritedFlags),
						nCurrentPriotity, pwszExit ) ;
			if ( err )
			{
				return	err ;
			}
			MakeCommitValueToNakedRegister( typeTerm2 ) ;
			FenceInstruction() ;
			//
			CompileCodeCommitJumpAddress
				( dwEndExprJumpPos, CompileCodeGetCurrent() ) ;
			//
			bool	fEquType = (typeTerm1 == typeTerm2) ;
			if ( !fEquType
				&& typeTerm1.IsPureType() && typeTerm2.IsPureType()
				&& typeTerm1.m_pValue && typeTerm2.m_pValue )
			{
				fEquType = (typeTerm1.m_pValue->m_vtType
								== typeTerm2.m_pValue->m_vtType) ;
			}
			if ( fEquType )
			{
				typeinf = typeTerm1 ;
				//
				if ( m_modeNakedCode )
				{
					if ( typeTerm1.m_regLoaded != typeTerm2.m_regLoaded )
					{
						return	ESLErrorMsg
							( "内部エラー： ? : 演算子でレジスタが一致しません" ) ;
					}
				}
			}
			else if ( m_modeNakedCode )
			{
				err = ESLErrorMsg( " ? : 演算子で型が一致しません" ) ;
				if ( !typeTerm1.IsTypeReference()
						&& !typeTerm2.IsTypeReference() )
				{
					if ( (typeTerm1.IsTypeInteger() && typeTerm2.IsTypeInteger())
						|| (typeTerm1.IsTypeReal() && typeTerm2.IsTypeReal()) )
					{
						int	nSize1 = typeTerm1.SizeOfOnNakedMemory() ;
						int	nSize2 = typeTerm2.SizeOfOnNakedMemory() ;
						if ( nSize1 < nSize2 )
						{
							typeinf = typeTerm2 ;
						}
						else
						{
							typeinf = typeTerm1 ;
						}
						err = eslErrSuccess ;
					}
				}
				if ( err )
				{
					return	err ;
				}
			}
			else
			{
				OutputWarning0( " ? : 演算子で型が一致しません" ) ;
				typeinf = ECSTypeInfo( new ECSReference ) ;
			}
			if ( (nSaveExprAlloc != (int) m_lstExprAlloc.GetSize())
				|| (nSaveObjAlloc != m_objExprAlloc) )
			{
				return	ESLErrorMsg
					( "分岐式の中で一時オブジェクトが生成されています" ) ;
			}
		}
		else if ( opinf.opiType == optGeneral )
		{
			//
			// 二項演算子
			//
			NormalizePointerFromLinearAddress( typeinf ) ;
			if ( typeinf.IsTypeArray() )
			{
				return	ESLErrorMsg( "配列への不正な演算子です。" ) ;
			}
			ECSTypeInfo	typeTemp1, typeTemp2 ;
			if ( (opinf.optOperator == csotLogicalAnd)
				|| (opinf.optOperator == csoutLogicalOr) )
			{
				err = CompileTypeExUnaryOperate
							( typeTemp1, typeinf, csxuotBoolean ) ;
				if ( err )
				{
					return	err ;
				}
				MakeCommitValueToNakedRegister( typeTemp1 ) ;
//				FenceInstruction() ;
				m_pcsxi->FlushAllRegisterAssigns() ;
				//
				DWORD	dwCmpJumpPos ;
				dwCmpJumpPos =
					CompileCodeConditionalJump
						( (opinf.optOperator == csoutLogicalOr),
							!m_modeNakedCode ) ;
				//
				const int	nSaveExprAlloc = m_lstExprAlloc.GetSize() ;
				const int	nSaveObjAlloc = m_objExprAlloc ;
				//
				err = CompileExpression
						( typeTemp2, cssLine,
							(dwFlags & exprInheritedFlags),
							nCurrentPriotity, pwszExit ) ;
				if ( err )
				{
					return	err ;
				}
				NormalizePointerFromLinearAddress( typeTemp2 ) ;
				//
				err = CompileTypeExUnaryOperate
							( typeTemp2, typeTemp2, csxuotBoolean ) ;
				if ( err )
				{
					return	err ;
				}
				MakeCommitValueToNakedRegister( typeTemp2 ) ;
				FenceInstruction() ;
				//
				CompileCodeCommitJumpAddress
					( dwCmpJumpPos, CompileCodeGetCurrent() ) ;
				//
				typeinf = typeTemp1 ;
				//
				if ( m_modeNakedCode )
				{
					if ( typeTemp1.m_regLoaded != typeTemp2.m_regLoaded )
					{
						return	ESLErrorMsg
							( "内部エラー： && 又は || 演算子でレジスタが一致しません" ) ;
					}
				}
				if ( (nSaveExprAlloc != (int) m_lstExprAlloc.GetSize())
					|| (nSaveObjAlloc != m_objExprAlloc) )
				{
					return	ESLErrorMsg
						( "分岐式の中で一時オブジェクトが生成されています" ) ;
				}
			}
			else
			{
				err = CompileExpression
					( typeTemp2, cssLine,
						(dwFlags & exprInheritedFlags),
							nCurrentPriotity, pwszExit ) ;
				if ( err )
				{
					return	err ;
				}
				NormalizePointerFromLinearAddress( typeTemp2 ) ;
				//
				typeTemp1 = typeinf ;
				err = CompileTypeOperate
						( typeinf, typeTemp1, typeTemp2, opinf.optOperator ) ;
				if ( err )
				{
					return	err ;
				}
			}
		}
		else if ( opinf.opiType == optMember )
		{
			//
			// メンバ参照
			//
			int		nTokenType ;
			wstrToken = cssLine.GetAToken( &nTokenType ) ;
			if ( nTokenType != 0 )
			{
				return	ESLErrorMsg( "不正なメンバ名が指定されています。" ) ;
			}
			NormalizePointerFromLinearAddress( typeinf ) ;
			if ( typeinf.IsTypePointer() )
			{
				return	ESLErrorMsg
					( "ポインタに対して \'.\' でメンバが指定されています" ) ;
			}
			if ( cssLine.HasToComeChar( L"(" ) == L'(' )
			{
				//
				// クラスメンバ関数呼び出し
				//
				const ECSClassInfo *
					pClassInf = GetNakedTypeClassInfo( typeinf ) ;
				//
				if ( pClassInf != NULL )
				{
					if ( m_modeNakedCode )
					{
						while ( typeinf.IsTypeReference2() )
						{
							CompileCodeNakedUncoverReference( typeinf ) ;
						}
						MakeCommitValueToNakedRegister( typeinf ) ;
					}
					//
					const ECSClassInfo::MemberFunction *	pFunc = NULL ;
					err = CompileArgumentAndCallMemberFunction
						( cssLine, *pClassInf, typeinf, wstrToken,
								false, pFunc, ECSTypeInfo::flagPublic ) ;
					if ( err )
					{
						return	err ;
					}
					if ( pFunc != NULL )
					{
						GetFunctionReturnType( typeinf, *pFunc ) ;
					}
					else if ( m_modeNakedCode )
					{
						return	ESLErrorMsg
							( "naked モードでプロトタイプ情報がない関数呼び出しです" ) ;
					}
					else
					{
						typeinf.SetTypeValue( new ECSReference, 0 ) ;
					}
				}
				else
				{
					if ( m_dwModeFlags & flagStrictStyle )
					{
						err = OutputWarning1
							( "抽象 Reference のメンバ関数を呼び出そうとしています" ) ;
						if ( err )
						{
							return	err ;
						}
					}
					if ( m_modeNakedCode )
					{
						return	ESLErrorMsg
							( "naked モードでクラス情報のない関数呼び出しです" ) ;
					}
					EObjArray<ECSTypeInfo>	lstArgType ;
					err = CompileArgument( lstArgType, NULL, cssLine ) ;
					if ( err )
					{
						return	err ;
					}
					int	nArgCount = lstArgType.GetSize() ;
					nArgCount ++ ;
					m_pcsxi->WriteInstructionCode( csicCall ) ;
					m_pcsxi->WriteObjectModeCode( csomThis ) ;
					m_pcsxi->WriteCodeData( &nArgCount, sizeof(nArgCount) ) ;
					m_pcsxi->WriteConstantString( wstrToken ) ;
					//
					typeinf.SetTypeValue( new ECSReference, 0 ) ;
				}
			}
			else
			{
				//
				// メンバ変数
				//
				ECSTypeInfo	typeSrc = typeinf ;
				err = CompileTypeMemberVariable
						( typeinf, typeSrc, wstrToken ) ;
				if ( err )
				{
					return	err ;
				}
			}
		}
		else if ( opinf.opiType == optPtrMember )
		{
			//
			// ポインタメンバ参照
			//
			err = CompileTypeCallPointerOperator( typeinf ) ;
			if ( err )
			{
				return	err ;
			}
			int		nTokenType ;
			wstrToken = cssLine.GetAToken( &nTokenType ) ;
			if ( nTokenType != 0 )
			{
				return	ESLErrorMsg( "不正なメンバ名が指定されています。" ) ;
			}
			NormalizePointerFromLinearAddress( typeinf ) ;
			if ( !typeinf.IsTypePointer() )
			{
				return	ESLErrorMsg
					( "ポインタでないオブジェクトに対し"
						" \'->\' でメンバが指定されています" ) ;
			}
			const ECSClassInfo *
				pClassInf = GetNakedPtrTypeClassInfo( typeinf ) ;
			if ( pClassInf == NULL )
			{
				return	ESLErrorMsg
					( "\'->\' で参照されるクラス情報が見つかりません" ) ;
			}
			if ( cssLine.HasToComeChar( L"(" ) == L'(' )
			{
				//
				// クラスメンバ関数呼び出し
				//
				ECSTypeInfo	typeThis ;
				err = CompileReferencePointer( typeThis, typeinf ) ;
				if ( err )
				{
					return	err ;
				}
				NormalizePointerFromLinearAddress( typeThis ) ;
				//
				if ( m_modeNakedCode )
				{
					while ( typeThis.IsTypeReference2() )
					{
						CompileCodeNakedUncoverReference( typeThis ) ;
					}
					MakeCommitValueToNakedRegister( typeThis ) ;
				}
				//
				const ECSClassInfo::MemberFunction *	pFunc = NULL ;
				err = CompileArgumentAndCallMemberFunction
					( cssLine, *pClassInf, typeThis, wstrToken,
							false, pFunc, ECSTypeInfo::flagPublic ) ;
				if ( err )
				{
					return	err ;
				}
				if ( pFunc != NULL )
				{
					GetFunctionReturnType( typeinf, *pFunc ) ;
				}
				else if ( m_modeNakedCode )
				{
					return	ESLErrorMsg
						( "naked モードでプロトタイプ情報がない関数呼び出しです" ) ;
				}
				else
				{
					typeinf.SetTypeValue( new ECSReference, 0 ) ;
				}
			}
			else
			{
				//
				// メンバ変数
				//
				ECSTypeInfo	typeSrc = typeinf ;
				err = CompilePointerTypeMemberVariable
						( typeinf, typeSrc, wstrToken ) ;
				if ( err )
				{
					return	err ;
				}
			}
		}
		else if ( (opinf.opiType == optMemberFunc)
				|| (opinf.opiType == optPtrMemberFunc) )
		{
			//
			// ".*" | "->*" メンバ関数参照
			//
			ECSTypeInfo	typeThis = typeinf ;
			err = CompileTypeMemberFunctionPointer
				( typeinf, typeThis, opinf.opiType,
					cssLine, (dwFlags & exprInheritedFlags),
					nCurrentPriotity - 1, pwszExit ) ;
			if ( err )
			{
				return	err ;
			}
		}
		else if ( opinf.opiType == optUnary )
		{
			//
			// 後置単項演算子
			//
			switch ( opinf.uoptUnary )
			{
			case	csuotIncrement:
				opinf.uoptUnary = csuotIncrementAfter ;
				break; 
			case	csuotDecrement:
				opinf.uoptUnary = csuotDecrementAfter ;
				break; 
			default:
				return	ESLErrorMsg( "不正な後置単項演算子です。" ) ;
			}
			ECSTypeInfo	typeSrc = typeinf ;
			err = CompileTypeUnaryOperate
					( typeinf, typeSrc, opinf.uoptUnary ) ;
			if ( err )
			{
				return	err ;
			}
		}
		else if ( opinf.opiType == optReference )
		{
			//
			// 要素間接参照
			//
			NormalizePointerFromLinearAddress( typeinf ) ;
			//
			ECSTypeInfo	typeIndex ;
			err = CompileExpression
				( typeIndex, cssLine,
					(dwFlags & exprInheritedFlags), 0, L"]" ) ;
			if ( err )
			{
				return	err ;
			}
			if ( cssLine.HasToComeChar( L"]" ) != L']' )
			{
				return	ESLErrorMsg
					( "\'[\' に対応する \']\' が見つかりません。" ) ;
			}
			ECSTypeInfo	typeSrc = typeinf ;
			err = CompileTypeReferenceElement
						( typeinf, typeSrc, typeIndex ) ;
			if ( err )
			{
				return	err ;
			}
		}
		else if ( opinf.opiType == optMove )
		{
			//
			// 代入操作
			//
			ECSTypeInfo	typeSrc1 = typeinf ;
			ECSTypeInfo	typeSrc2 ;
			err = CompileExpression
				( typeSrc2, cssLine,
					(dwFlags & exprInheritedFlags),
					nCurrentPriotity - 1, pwszExit ) ;
			if ( err )
			{
				return	err ;
			}
			err = CompileTypeMoveOperate
					( typeinf, typeSrc1, typeSrc2, opinf.optOperator ) ;
			if ( err )
			{
				return	err ;
			}
		}
		else if ( opinf.opiType == optExtraOperator )
		{
			//
			// 特殊演算子
			//
			if ( opinf.xoptExOperator != csxotMoveReference )
			{
				return	ESLErrorMsg
					( "内部エラー：不正な特殊演算子です。" ) ;
			}
			if ( !typeinf.IsTypeReference2()
					&& !typeinf.IsAbstractType() )
			{
				return	ESLErrorMsg
					( "::= の左辺式が参照型変数ではありません。" ) ;
			}
			ECSTypeInfo	typeSrc ;
			err = CompileExpression
				( typeSrc, cssLine,
					(dwFlags & exprInheritedFlags),
					nCurrentPriotity - 1, pwszExit ) ;
			if ( err )
			{
				return	err ;
			}
			ECSTypeInfo	typeDstCast ;
			typeDstCast.MakeReferenceOf
				( ECSTypeInfo( ECSTypeInfo::DuplicateType
					( typeinf.GetNakedType() ), typeinf.m_dwFlags ) ) ;
			ECSTypeInfo	typeSrcCast ;
			err = CompileTypeCast
				( typeSrcCast, typeDstCast,
					typeSrc, 0, ECSTypeInfo::flagPublic ) ;
			if ( err )
			{
				return	err ;
			}
			if ( m_modeNakedCode )
			{
				MakeCommitValueToNakedRegister( typeSrcCast ) ;
				//
				int	regSrc = typeSrcCast.IsLoadedRegister()
								? typeSrcCast.GetLoadedRegister()
									: GetExpressionRegister() ;
				WriteSakuraStoreMemory( regSrc, typeinf ) ;
				//
				FreeExpressionRegister( typeSrcCast ) ;
			}
			else
			{
				m_pcsxi->WriteInstructionCode( csicExOperate ) ;
				m_pcsxi->WriteExOperatorTypeCode( csxotMoveReference ) ;
			}
		}
		else if ( opinf.opiType == optCompare )
		{
			//
			// 比較演算
			//
			ECSTypeInfo	typeTemp1 = typeinf ;
			ECSTypeInfo	typeTemp2 ;
			err = CompileExpression
				( typeTemp2, cssLine,
					(dwFlags & exprInheritedFlags),
					nCurrentPriotity, pwszExit ) ;
			if ( err )
			{
				return	err ;
			}
			err = CompileTypeCompare
				( typeinf, typeTemp1, typeTemp2, opinf.cptCompare ) ;
			if ( err )
			{
				return	err ;
			}
		}
		else if ( opinf.opiType == optListExpression )
		{
			//
			// 式列挙
			//
			CompileCodeFreeStack( typeinf ) ;
			//
			err = CompileExpression
				( typeinf, cssLine,
					(dwFlags & exprInheritedFlags),
					nCurrentPriotity, pwszExit ) ;
			if ( err )
			{
				return	err ;
			}
		}
		else if ( opinf.opiType == optArgument )
		{
			//
			// 関数呼び出し
			//
			ECSFunction *	pFuncPtr = typeinf.GetTypeFunctionPointer() ;
			if ( pFuncPtr == NULL )
			{
				return	ESLErrorMsg
					( "\'(\' の左項が関数ポインタではありません" ) ;
			}
			ECSTypeInfo	typeFunc = typeinf ;
			err = CompileArgumentAndIndirectCallFunction
								( cssLine, typeinf, typeFunc ) ;
			if ( err )
			{
				return	err ;
			}
		}
		else
		{
			return	ESLErrorMsg( "不正な演算子です。" ) ;
		}
	}
	//
	return	eslErrSuccess ;
}

// 数値リテラル判定
//////////////////////////////////////////////////////////////////////////////
int ECSCompiler::GetNumberLiteralRadix
	( ECSSourceStream & cssLine, bool & fRealNumber )
{
	const int	iFirst = cssLine.GetIndex( ) ;
	int			nRadix = -1 ;
	//
	if ( !(m_dwModeFlags & flagCStyleNumberLiteral) )
	{
		nRadix = cssLine.GetNumberRadix( ) ;
		if ( nRadix >= 0 )
		{
			REAL64	rValue = cssLine.GetRealNumber( nRadix ) ;
			int		iNext = cssLine.GetIndex( ) ;
			int		iFindComma = cssLine.Find( L'.', iFirst ) ;
			fRealNumber = ((iFindComma >= 0) && (iFindComma < iNext)) ;
		}
	}
	else
	{
		wchar_t	wchFirst ;
		for ( ; ; )
		{
			cssLine.DisregardSpace() ;
			wchFirst = cssLine.GetCharacter() ;
			if ( (wchFirst != L'+') && (wchFirst != L'-') )
			{
				break ;
			}
		}
		fRealNumber = false ;
		if ( wchFirst == L'0' )
		{
			wchar_t	wchNext = cssLine.CurrentCharacter() ;
			if ( (wchNext == L'x') || (wchNext == L'X') )
			{
				nRadix = 16 ;
			}
			else
			{
				nRadix = 10 ;
			}
		}
		else if ( (L'0' <= wchFirst) && (wchFirst <= L'9') )
		{
			nRadix = 10 ;
		}
		if ( nRadix == 10 )
		{
			for ( ; ; )
			{
				wchar_t	wchNext = cssLine.GetCharacter() ;
				if ( (wchNext == L'.')
					|| (wchNext == L'E') || (wchNext == L'e') )
				{
					fRealNumber = true ;
					break ;
				}
				if ( (wchNext < L'0') || (L'9' < wchNext) )
				{
					break;
				}
			}
			if ( (nRadix >= 0) && !fRealNumber && (wchFirst == L'0') )
			{
				nRadix = 8 ;
			}
		}
	}
	cssLine.MoveIndex( iFirst ) ;
	return	nRadix ;
}

// 整数リテラル解釈
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::GetIntegerLiteral
	( INT64 & intLiteral, ECSSourceStream & cssLine, int nRadix )
{
	if ( !(m_dwModeFlags & flagCStyleNumberLiteral) )
	{
		intLiteral = cssLine.GetLargeInteger( nRadix ) ;
	}
	else
	{
		bool	fSign = false ;
		for ( ; ; )
		{
			cssLine.DisregardSpace() ;
			wchar_t	wch = cssLine.CurrentCharacter() ;
			if ( wch == L'-' )
			{
				fSign = !fSign ;
			}
			else if ( wch != L'+' )
			{
				break ;
			}
			cssLine.GetCharacter() ;
		}
		INT64	num = 0 ;
		wchar_t	wch ;
		if ( cssLine.CurrentCharacter() == L'0' )
		{
			cssLine.GetCharacter() ;
			wch = cssLine.CurrentCharacter() ;
			if ( (wch == L'X') || (wch == L'x') )
			{
				cssLine.GetCharacter() ;
			}
		}
		for ( ; ; )
		{
			wch = cssLine.CurrentCharacter() ;
			int	c = 0 ;
			if ( (L'0' <= wch) && (wch <= L'9') )
			{
				c = (wch - L'0') ;
			}
			else if ( (L'A' <= wch) && (wch <= L'F') )
			{
				c = (wch - L'A') + 10 ;
			}
			else if ( (L'a' <= wch) && (wch <= L'f') )
			{
				c = (wch - L'a') + 10 ;
			}
			else if ( (wch == L'U') || (c == L'u') )
			{
				cssLine.GetCharacter() ;
				wch = cssLine.CurrentCharacter() ;
				if ( (wch == L'L') || (wch == L'l') )
				{
					cssLine.GetCharacter() ;
				}
				break ;
			}
			else if ( (wch == L'L') || (wch == L'l') )
			{
				cssLine.GetCharacter() ;
				break ;
			}
			else
			{
				break ;
			}
			if ( c >= nRadix )
			{
				return	ESLErrorMsg
					( "整数リテラルに不正な文字が含まれています。" ) ;
			}
			cssLine.GetCharacter() ;
			//
			num = num * nRadix + c ;
		}
		intLiteral = (fSign ? - num : num) ;
	}
	return	eslErrSuccess ;
}

// 実数リテラル解釈
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::GetRealLiteral
	( REAL64 & realLiteral, ECSSourceStream & cssLine, int nRadix )
{
	if ( !(m_dwModeFlags & flagCStyleNumberLiteral) )
	{
		realLiteral = cssLine.GetRealNumber( nRadix ) ;
	}
	else
	{
		bool	fSign = false ;
		for ( ; ; )
		{
			cssLine.DisregardSpace() ;
			wchar_t	wch = cssLine.CurrentCharacter() ;
			if ( wch == L'-' )
			{
				fSign = !fSign ;
			}
			else if ( wch != L'+' )
			{
				break ;
			}
			cssLine.GetCharacter() ;
		}
		double	num = 0.0 ;
		double	decimal = 1.0 ;
		bool	p = false ;
		for ( ; ; )
		{
			wchar_t	wch = cssLine.CurrentCharacter() ;
			if ( (L'0' <= wch) && (wch <= L'9') )
			{
				num = (num * nRadix) + (int) (wch - L'0') ;
				if ( p )
				{
					decimal *= nRadix ;
				}
			}
			else if ( !p && (wch == L'.') )
			{
				p = true ;
			}
			else if ( (wch == L'e') || (wch == L'E') )
			{
				bool	fExpSign = false ;
				cssLine.GetCharacter() ;
				wch = cssLine.CurrentCharacter() ;
				if ( wch == L'-' )
				{
					fExpSign = true ;
					cssLine.GetCharacter() ;
				}
				else if ( wch == L'+' )
				{
					cssLine.GetCharacter() ;
				}
				int		nExp = 0 ;
				for ( ; ; )
				{
					wch = cssLine.CurrentCharacter() ;
					if ( (L'0' <= wch) && (wch <= L'9') )
					{
						nExp = (nExp * 10) + (int) (wch - L'0') ;
						cssLine.GetCharacter() ;
					}
					else if ( (wch == L'F') || (wch == L'f') )
					{
						cssLine.GetCharacter() ;
						break ;
					}
					else
					{
						break ;
					}
				}
				if ( fExpSign )
				{
					nExp = - nExp ;
				}
				num *= pow( 10.0, nExp ) ;
				break ;
			}
			else if ( (wch == L'F') || (wch == L'f') )
			{
				cssLine.GetCharacter() ;
				break ;
			}
			else
			{
				break ;
			}
			cssLine.GetCharacter() ;
		}
		realLiteral = num / decimal ;
		if ( fSign )
		{
			realLiteral = - realLiteral ;
		}
	}
	return	eslErrSuccess ;
}

// 文字列リテラル解釈
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::GetStringLiteral
	( EWideString & wstrLiteral, ECSSourceStream & cssLine, wchar_t wchCloser )
{
	wstrLiteral = L"" ;
	for ( ; ; )
	{
		if ( cssLine.GetCharacter() != wchCloser )
		{
			return	ESLErrorMsg
				( "コンパイル内部エラー："
					"文字列リテラルの開始記号が一致しません。" ) ;
		}
		int	iFirstStr = cssLine.GetIndex( ) ;
		cssLine.PassEnclosedString( wchCloser, m_dwModeFlags ) ;
		int	iEndStr = cssLine.GetIndex( ) ;
		if ( cssLine.GetCharacter() != wchCloser )
		{
			return	ESLErrorMsg
				( "文字列がクォーテーションで閉じられていません。" ) ;
		}
		wstrLiteral += cssLine.Middle( iFirstStr, iEndStr - iFirstStr ) ;
		//
		cssLine.DisregardSpace() ;
		if ( (cssLine.CurrentCharacter() == L'L')
			&& (cssLine.GetAt( cssLine.GetIndex() + 1 ) == wchCloser) )
		{
			cssLine.GetCharacter() ;
			continue ;
		}
		if ( cssLine.CurrentCharacter() != wchCloser )
		{
			break ;
		}
	}
	//
	if ( (wchCloser != L'\'') || !(m_dwModeFlags & flagQuoteNakedString) )
	{
		EDescription::DecodeTextCEscSequence( wstrLiteral ) ;
	}
	return	eslErrSuccess ;
}

// 変数参照
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileVariableReference
	( ECSTypeInfo & typeinf, CSObjectMode csomClass,
		const SYMBOL_NAMESPACE& snsSymbol, bool fWithoutFunction )
{
	EWideString	wstrToken = snsSymbol.wstrFullName ;
	const ECSClassInfo *	pClassInf ;
	ECSTypeInfo		typeVar ;
	ECSTypeInfo *	pVarType ;
	ECSTypeInfo		typeThis ;
	DWORD		fdwTypeFlags ;
	int			iVarIndex = -1 ;
	int			iThisVarIndex = -1 ;
	bool		fFuncPtrType = false ;
	bool		fVarType = false ;
	bool		fStrLiteral = false ;
	bool		fNakedThisVar = false ;
	bool		fNakedGlobalVar = false ;
	DWORD		dwRefAddr = CompileCodeGetCurrent() + 3 ;
	if ( !snsSymbol.wstrNamespace.IsEmpty() )
	{
		if ( (csomClass == csomAuto)
			|| (csomClass == csomGlobal) || (csomClass == csomThis) )
		{
			csomClass = csomThis ;
		}
		else
		{
			return	ESLErrorMsg( "名前空間の指定が不正です" ) ;
		}
	}
	switch ( csomClass )
	{
	case	csomThis:
		// this メンバ、又は親クラスメンバ
		//////////////////////////////////////////////////////////////////////
		if ( !snsSymbol.wstrNamespace.IsEmpty() )
		{
			// static なメンバ変数の検索
			if ( !fWithoutFunction )
			{
				fFuncPtrType =
					CompileCodeGlobalFunctionPointer
							( typeVar, snsSymbol.wstrFullName ) ;
				if ( fFuncPtrType )
				{
					break ;
				}
			}
			fNakedGlobalVar =
				CompileCodeSearchNakedGlobalVariable
						( typeVar, snsSymbol.wstrFullName ) ;
			if ( fNakedGlobalVar )
			{
				break ;
			}
			iVarIndex = IsGlobalVariableName
					( snsSymbol.wstrFullName, dwRefAddr, &typeVar ) ;
			fVarType = (iVarIndex >= 0) ;
			if ( fVarType )
			{
				break ;
			}
			iVarIndex = IsDataTableName
					( snsSymbol.wstrFullName, dwRefAddr, &typeVar ) ;
			if ( iVarIndex >= 0 )
			{
				fVarType = true ;
				csomClass = csomData ;
				break ;
			}
			wstrToken = snsSymbol.wstrName ;
		}
		// this ポインタ検索
		iThisVarIndex = IsLocalVariableName( L"this", &typeThis ) ;
		if ( iThisVarIndex < 0 )
		{
			return	ESLErrorMsg
				( "この関数には this オブジェクトは存在しません。" ) ;
		}
		if ( m_modeNakedCode )
		{
			pClassInf = GetNakedPtrTypeClassInfo( typeThis ) ;
		}
		else
		{
			pClassInf = GetNakedTypeClassInfo( typeThis ) ;
		}
		if ( typeThis.IsAbstractType() || (pClassInf == NULL) )
		{
			fStrLiteral = true ;
		}
		else
		{
			int	iCastOffset = 0 ;
			if ( !snsSymbol.wstrNamespace.IsEmpty() )
			{
				// 親クラスにキャスト
				EPtrObjArray<const wchar_t>	lstNamespace ;
				EWideString	wstrTempNamespace ;
				GetUsingNamespaceList( lstNamespace ) ;
				ECSClassInfo::CastInfo *	pCast = NULL ;
				for ( unsigned int i = 0; i < lstNamespace.GetSize(); i ++ )
				{
					const wchar_t *	pwszNamespace = lstNamespace.GetAt( i ) ;
					if ( pwszNamespace != NULL )
					{
						wstrTempNamespace = pwszNamespace ;
						wstrTempNamespace += L"::" ;
						wstrTempNamespace += snsSymbol.wstrNamespace ;
						pCast = pClassInf->GetCastClassInfoAs( wstrTempNamespace ) ;
						if ( pCast != NULL )
						{
							break ;
						}
					}
				}
				if ( pCast == NULL )
				{
					pCast = pClassInf->GetCastClassInfoAs( snsSymbol.wstrNamespace ) ;
				}
				if ( pCast == NULL )
				{
					m_strErrMsg =
						EString( snsSymbol.wstrNamespace )
									+ " は親クラスではありません" ;
					return	ESLErrorMsg( m_strErrMsg ) ;
				}
				iCastOffset = pCast->nNakedOffset ;
				pClassInf = pCast->pClassInf ;
				if ( !pClassInf->IsNakedMemoryClass() )
				{
					return	ESLErrorMsg
						( "objected なクラスのメンバ指定に"
							"名前空間を指定する事は出来ません" ) ;
				}
				if ( (pCast->dwFlags & ECSTypeInfo::flagProtectedMask)
											> ECSTypeInfo::flagPrivate )
				{
					return	ESLErrorMsg
						( "private な親クラスへアクセスできません。" ) ;
				}
			}
			// メンバ変数検索
			iVarIndex = pClassInf->GetVariableIndex( wstrToken ) ;
			pVarType = pClassInf->GetVariableAt( iVarIndex ) ;
			if ( (iVarIndex < 0) || (pVarType == NULL) )
			{
				m_strErrMsg =
					EString( pClassInf->GetGlobalName() )
						+ " クラスに " + EString( wstrToken )
						+ " メンバ変数が見つかりません。" ;
				return	ESLErrorMsg( m_strErrMsg ) ;
			}
			if ( pVarType->GetProtectedAttribute()
								> ECSTypeInfo::flagPrivate )
			{
				return	ESLErrorMsg
					( "private なメンバへアクセスできません。" ) ;
			}
			fVarType = true ;
			fdwTypeFlags = (typeThis.m_dwFlags & ECSTypeInfo::flagConstant) ;
			typeVar = *pVarType ;
			typeVar.m_dwFlags |= fdwTypeFlags ;
			//
			if ( pClassInf->IsNakedMemoryClass() )
			{
				fNakedThisVar = true ;
				iVarIndex = pClassInf->GetVariableNakedOffsetAt( iVarIndex ) ;
				//
				if ( m_modeNakedCode )
				{
					typeinf.MakeReferenceOf( typeVar ) ;
					typeinf.SetAddressingInfo
						( ECSSakura2Processor::regTP, iVarIndex + iCastOffset ) ;
					return	eslErrSuccess ;
				}
				else if ( !snsSymbol.wstrNamespace.IsEmpty() )
				{
					m_strErrMsg =
						EString(snsSymbol.wstrFullName)
						+ " は naked モードでなければ許可されない記述です" ;
					return	ESLErrorMsg( m_strErrMsg ) ;
				}
			}
			else if ( m_modeNakedCode )
			{
				return	ESLErrorMsg
					( "naked モードで object の"
						"メンバを参照しようとしています" ) ;
			}
		}
		break ;

	case	csomGlobal:
		// グローバル変数
		//////////////////////////////////////////////////////////////////////
		if ( !fWithoutFunction )
		{
			fFuncPtrType =
				CompileCodeGlobalFunctionPointer( typeVar, wstrToken, true ) ;
			if ( fFuncPtrType )
			{
				break ;
			}
		}
		fNakedGlobalVar =
			CompileCodeSearchNakedGlobalVariable( typeVar, wstrToken ) ;
		if ( fNakedGlobalVar )
		{
			break ;
		}
		iVarIndex = IsGlobalVariableName( wstrToken, dwRefAddr, &typeVar ) ;
		fVarType = (iVarIndex >= 0) ;
		break ;

	case	csomData:
		// shared 変数
		//////////////////////////////////////////////////////////////////////
		if ( m_modeNakedCode )
		{
			return	ESLErrorMsg( "naked モードで 未対応の static 指定です" ) ;
		}
		iVarIndex = IsDataTableName( wstrToken, dwRefAddr, &typeVar ) ;
		fVarType = (iVarIndex >= 0) ;
		break ;

	case	csomStack:
		// ローカル変数
		//////////////////////////////////////////////////////////////////////
		iVarIndex = IsLocalVariableName( wstrToken, &typeVar ) ;
		fVarType = (iVarIndex >= 0) ;
		break ;

	case	csomAuto:
		// 自動スコープ
		//////////////////////////////////////////////////////////////////////
		iVarIndex = IsLocalVariableName( wstrToken, &typeVar ) ;
		if ( iVarIndex >= 0 )
		{
			csomClass = csomStack ;
			fVarType = true ;
			if ( !m_modeNakedCode )
			{
				fStrLiteral = (m_rwCtrlType == rwCatch) ;
				if ( !fStrLiteral )
				{
					fStrLiteral =
						(GetMostInnerNest( rwCatch, rwCatch ) != NULL) ;
				}
			}
			break ;
		}
		if ( !fWithoutFunction )
		{
			fFuncPtrType =
				CompileCodeAutoFunctionPointer( typeVar, snsSymbol ) ;
			if ( fFuncPtrType )
			{
				break ;
			}
		}
		iThisVarIndex = IsLocalVariableName( L"this", &typeThis ) ;
		if ( iThisVarIndex >= 0 )
		{
			if ( m_modeNakedCode )
			{
				pClassInf = GetNakedPtrTypeClassInfo( typeThis ) ;
			}
			else
			{
				pClassInf = GetNakedTypeClassInfo( typeThis ) ;
			}
			if ( typeThis.IsAbstractType() || (pClassInf == NULL) )
			{
				m_strErrMsg = "定義されていない変数名 \'"
					+ EString(wstrToken)
					+ "\' が指定されていますが、動的に名前を解決します。"
						"this オブジェクトのメンバであれば明示してください。" ;
				ESLError	err = OutputWarning
					( m_strErrMsg, m_strFilePath, m_nLineNum ) ;
				if ( err )
				{
					return	err ;
				}
				csomClass = csomThis ;
				fStrLiteral = true ;
				break ;
			}
			else
			{
				iVarIndex = pClassInf->GetVariableIndex( wstrToken ) ;
				pVarType = pClassInf->GetVariableAt( iVarIndex ) ;
				if ( (iVarIndex >= 0) && (pVarType != NULL) )
				{
					if ( pVarType->GetProtectedAttribute()
										> ECSTypeInfo::flagPrivate )
					{
						return	ESLErrorMsg
							( "private なメンバへアクセスできません。" ) ;
					}
					csomClass = csomThis ;
					fVarType = true ;
					fdwTypeFlags =
						(typeThis.m_dwFlags & ECSTypeInfo::flagConstant) ;
					typeVar = *pVarType ;
					typeVar.m_dwFlags |= fdwTypeFlags ;
					//
					if ( pClassInf->IsNakedMemoryClass() )
					{
						fNakedThisVar = true ;
						iVarIndex = pClassInf->GetVariableNakedOffsetAt( iVarIndex ) ;
					}
					break ;
				}
			}
		}
		fNakedGlobalVar =
			CompileCodeSearchNakedGlobalVariable( typeVar, wstrToken ) ;
		if ( fNakedGlobalVar )
		{
			break ;
		}
		iVarIndex = IsGlobalVariableName( wstrToken, dwRefAddr, &typeVar ) ;
		if ( iVarIndex >= 0 )
		{
			fVarType = true ;
			csomClass = csomGlobal ;
			break ;
		}
		iVarIndex = IsDataTableName( wstrToken, dwRefAddr, &typeVar ) ;
		if ( iVarIndex >= 0 )
		{
			fVarType = true ;
			csomClass = csomData ;
			break ;
		}
		if ( m_staExternName.FindIndex( wstrToken ) >= 0 )
		{
			csomClass = csomGlobal ;
			fStrLiteral = true ;
			break ;
		}
		break ;
	}
	if ( fNakedThisVar )
	{
		ESLAssert( csomClass == csomThis ) ;
		ESLError	err ;
		if ( m_modeNakedCode )
		{
			typeThis.SetAddressingInfo
					( ECSSakura2Processor::regTP, 0 ) ;
			err = CompilePointerTypeMemberVariable( typeinf, typeThis, wstrToken ) ;
		}
		else
		{
			m_pcsxi->WriteInstructionCode( csicLoad ) ;
			m_pcsxi->WriteObjectModeCode( csomClass ) ;
			m_pcsxi->WriteVariableTypeCode( csvtReference ) ;
			err = CompileTypeMemberVariable( typeinf, typeThis, wstrToken ) ;
		}
		if ( err )
		{
			return	err ;
		}
		return	eslErrSuccess ;
	}
	if ( fFuncPtrType )
	{
		typeinf = typeVar ;
		return	eslErrSuccess ;
	}
	if ( fStrLiteral )
	{
		if ( m_modeNakedCode )
		{
			return	ESLErrorMsg( "naked モードで動的メンバ参照です" ) ;
		}
		CompileCodeLoadRefVariable( csomClass, wstrToken ) ;
	}
	else if ( iVarIndex >= 0 )
	{
		if ( !m_modeNakedCode )
		{
			CompileCodeLoadRefVariable( csomClass, iVarIndex ) ;
		}
	}
	else if ( !fNakedGlobalVar )
	{
		m_strErrMsg = "定義されていない変数名 \'"
			+ EString(wstrToken) + "\' が指定されています。" ;
		return	ESLErrorMsg( m_strErrMsg ) ;
	}
	if ( fVarType || fNakedGlobalVar )
	{
		typeinf.MakeReferenceOf( typeVar ) ;
		typeinf.m_dwFlags &= ~ECSTypeInfo::flagDeterministic ;
		//
		if ( m_modeNakedCode )
		{
			bool	fObjectVar =
				!typeVar.IsNakedMemoryObject( m_modeNakedCode ) ;
			bool	fRefObjectVar =
				typeVar.IsTypeReference()
					&& !ECSTypeInfo::IsNakedMemoryObject
						( typeVar.GetNakedType(), m_modeNakedCode ) ;
			if ( fRefObjectVar )
			{
				typeinf = typeVar ;
			}
			//
			typeinf.MoveRegisterAndAddressingFrom( typeVar ) ;
			typeinf.m_dwFlags |= ECSTypeInfo::flagNakedBuffer ;
			//
			if ( fObjectVar || fRefObjectVar )
			{
				CompileCodeNakedUncoverReference( typeinf ) ;
				//
				ECSTypeInfo	typeTemp = typeinf ;
				typeinf.MakeReferenceOf( typeTemp ) ;
				typeinf.MoveRegisterAndAddressingFrom( typeTemp ) ;
			}
		}
	}
	else
	{
		if ( m_modeNakedCode )
		{
			return	ESLErrorMsg
				( "naked モードで型情報のない変数参照です" ) ;
		}
		typeinf = ECSTypeInfo( new ECSReference ) ;
	}
	return	eslErrSuccess ;
}

// 最適化領域開始
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::BeginSakura2Optimize( void )
{
//	if ( m_flagOptimize )
	{
		m_pcsxi->BeginSakura2Optimize() ;
	}
}

// 最適化領域開始
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::FinishSakura2Optimize( void )
{
	if ( m_flagOptimize )
	{
		m_pcsxi->FinishSakura2Optimize
			( ECSSakura2Processor::regExpr0 + m_regExprAlloc ) ;
	}
}

// 数式の中で生成されたオブジェクトを解放する
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::FreeExpressionTemporary( int regSave )
{
	if ( m_modeNakedCode )
	{
		FreeExpressionTemporaryStack() ;
	}
	ESLAssert( m_regExprAlloc == 0 ) ;
	/*
	if ( m_regExprAlloc != 0 )
	{
		OutputWarning
			( "内部エラー：レジスタの割り当て処理に異常があります",
										m_strFilePath, m_nLineNum ) ;
	}
	*/
	m_regExprAlloc = 0 ;
}

void ECSCompiler::FreeExpressionTemporaryStack( int regSave )
{
	if ( !m_modeNakedCode )
	{
		return ;
	}
	ESLAssert( (regSave < 0) || (regSave == GetExpressionRegister()) ) ;
	if ( m_lstExprAlloc.GetSize() > 0 )
	{
		//
		// 一時 naked オブジェクトのデストラクタ呼び出し
		//
		if ( regSave >= 0 )
		{
			m_pcsxi->WriteSakuraPushReg( regSave ) ;
		}
		ESLAssert( m_modeNakedCode ) ;
		for ( int i = m_lstExprAlloc.GetSize() - 1; i >= 0; i -- )
		{
			ECSTypeInfo *	pTypeInf = m_lstExprAlloc.GetAt( i ) ;
			if ( (pTypeInf == NULL)
				|| !pTypeInf->IsAddressingInfo() )
			{
				continue ;
			}
			// this ポインタ計算
			int	regTemp = AllocateExpressionRegister() ;
			if ( pTypeInf->m_regIndex >= 0 )
			{
				m_pcsxi->WriteSakuraMulRegRegImm32
					( regTemp, pTypeInf->m_regIndex,
							(1 << pTypeInf->m_scaleIndex) ) ;
				m_pcsxi->WriteSakuraAddRegReg
					( regTemp, pTypeInf->m_regBase ) ;
			}
			else
			{
				m_pcsxi->WriteSakuraAddRegRegImm32
					( regTemp, pTypeInf->m_regBase,
								pTypeInf->m_addrOffset ) ;
			}
			// デストラクタ呼び出し
			ESLError	err =
				CompileCodeNakedVariableDestruction( *pTypeInf ) ;
			if ( err )
			{
				OutputError
					( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
			}
		}
		m_lstExprAlloc.RemoveAll() ;
		//
		if ( regSave >= 0 )
		{
			m_pcsxi->WriteSakuraPopReg( regSave ) ;
		}
	}
	if ( m_stackExprAlloc > 0 )
	{
		//
		// 一時 naked スタック解放
		//
		ESLAssert( m_modeNakedCode ) ;
		m_pcsxi->WriteSakuraAddSP( m_stackExprAlloc ) ;
		m_stackExprAlloc = 0 ;
	}
	if ( m_objExprAlloc > 0 )
	{
		//
		// 一時 object スタック解放
		//
		if ( regSave >= 0 )
		{
			m_pcsxi->WriteSakuraPushReg( regSave ) ;
		}
		ESLAssert( m_modeNakedCode ) ;
		int	regTemp = AllocateExpressionRegister() ;
		m_pcsxi->WriteSakuraLoadInt64( regTemp, (INT64) m_objExprAlloc ) ;
		//
		CompileNakedSystemCall( L"object_stack_free", 1 ) ;
		//
		if ( regSave >= 0 )
		{
			m_pcsxi->WriteSakuraPopReg( regSave ) ;
		}
		m_objExprAlloc = 0 ;
	}
	if ( m_lstLockedRegister.GetSize() > 0 )
	{
		for ( int i = 0; i < (int) m_lstLockedRegister.GetSize(); i ++ )
		{
			m_pcsxi->UnlockAssignedRegister( m_lstLockedRegister[i] ) ;
		}
		m_lstLockedRegister.RemoveAll() ;
	}
}

// レジスタ依存などの最適化をフェンスする
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::FenceInstruction( void )
{
	m_pcsxi->FenceInstruction() ;
	m_lstLockedRegister.RemoveAll() ;
}

// naked mode 計算用レジスタを割り当てる
//////////////////////////////////////////////////////////////////////////////
int ECSCompiler::AllocateExpressionRegister( void )
{
	int	regExpr = ECSSakura2Processor::regExpr0 + m_regExprAlloc ;
	if ( m_modeNakedCode )
	{
		m_regExprAlloc ++ ;
		if ( regExpr >= 0x80 )
		{
			OutputError
				( "計算式が複雑すぎます。"
					"レジスタが割り当てられませんでした。",
									m_strFilePath, m_nLineNum ) ;
		}
	}
	return	regExpr ;
}

// naked mode 計算用レジスタの最後に割り当てられた番号を取得する
//////////////////////////////////////////////////////////////////////////////
int ECSCompiler::GetExpressionRegister( int last )
{
	int	regExpr =
		ECSSakura2Processor::regExpr0 + m_regExprAlloc - last - 1 ;
	if ( m_modeNakedCode )
	{
		if ( (regExpr < ECSSakura2Processor::regExpr0) || (regExpr >= 0x80) )
		{
			OutputError
				( "内部エラー：レジスタの割り当が不正です",
									m_strFilePath, m_nLineNum ) ;
		}
	}
	return	regExpr ;
}

// naked mode 計算用レジスタを解放する
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::FreeExpressionRegister( void )
{
	if ( m_modeNakedCode )
	{
		if ( m_regExprAlloc > 0 )
		{
			m_regExprAlloc -- ;
		}
		else
		{
			OutputError
				( "内部エラー：レジスタの割り当が不正です",
									m_strFilePath, m_nLineNum ) ;
		}
	}
}

void ECSCompiler::FreeExpressionRegister( const ECSTypeInfo & typeExpr )
{
	if ( typeExpr.IsLoadedRegister() )
	{
		FreeExpressionRegister() ;
	}
	if ( typeExpr.IsLoadedThisCallRegister() )
	{
		FreeExpressionRegister() ;
	}
}

// ローカル変数割り当てを一時的にロックする
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::LockExpressionWorkRegister( int regWork )
{
	if ( regWork < 0x10 )
	{
		m_lstLockedRegister.Add( regWork ) ;
		//
		m_pcsxi->LockAssignedRegister( regWork ) ;
	}
}

// 一時割り当て情報をセーブする
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::SaveNakedExpressionTemporary
	( ECSCompiler::NakedExpressionTemporary & work )
{
	work.modeTemporary = m_modeTemporary ;
	work.regExprAlloc = m_regExprAlloc ;
	work.stackExprAlloc = m_stackExprAlloc ;
	work.objExprAlloc = m_objExprAlloc ;
	work.lstExprAlloc = m_lstExprAlloc ;
	work.lstLockedRegister = m_lstLockedRegister ;
}

// 一時割り当て情報をリストアする
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::RestoreNakedExpressionTemporary
	( const ECSCompiler::NakedExpressionTemporary & work )
{
	m_modeTemporary = work.modeTemporary ;
	m_regExprAlloc = work.regExprAlloc ;
	m_stackExprAlloc = work.stackExprAlloc ;
	m_objExprAlloc = work.objExprAlloc ;
	m_lstExprAlloc = work.lstExprAlloc ;
	m_lstLockedRegister = work.lstLockedRegister ;
	//
	if ( !m_modeTemporary )
	{
		m_lstLocalVarTemporary.RemoveAll() ;
	}
}

// 定数式情報のみでレジスタに値がロードされていない場合
// 評価値をレジスタにロードする
//（アドレスのインデックスのみロードされている場合には
// フルアドレスをロードする）
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::MakeCommitValueToNakedRegister( ECSTypeInfo & typeinf )
{
	if ( !m_modeNakedCode )
	{
		return ;
	}
	if ( typeinf.IsAddressingInfo() )
	{
		if ( !typeinf.IsLoadedRegister() )
		{
			typeinf.SetLoadedRegister
				( AllocateExpressionRegister() ) ;
		}
		LoadCommitValueToNakedRegister
				( typeinf.GetLoadedRegister(), typeinf ) ;
		typeinf.ClearAddressingInfo() ;
	}
	else if ( !typeinf.IsLoadedRegister() )
	{
		if ( typeinf.m_pValue != NULL )
		{
			int	regExpr = AllocateExpressionRegister() ;
			LoadCommitValueToNakedRegister( regExpr, typeinf ) ;
			typeinf.m_dwFlags &= ~ECSTypeInfo::flagDeterministic ;
			typeinf.SetLoadedRegister( regExpr ) ;
		}
	}
}

void ECSCompiler::LoadCommitValueToNakedRegister( int regDst, const ECSTypeInfo & typeinf )
{
	if ( !m_modeNakedCode )
	{
		return ;
	}
	if ( typeinf.IsAddressingInfo() )
	{
		m_pcsxi->FlushAllRegisterAssigns() ;
		if ( typeinf.m_regIndex >= 0 )
		{
			if ( typeinf.m_scaleIndex > 0 )
			{
				if ( regDst != typeinf.m_regBase )
				{
					m_pcsxi->WriteSakuraSllRegRegImm8
						( regDst, typeinf.m_regIndex, typeinf.m_scaleIndex ) ;
					m_pcsxi->WriteSakuraAddRegReg
						( regDst, typeinf.m_regBase ) ;
				}
				else
				{
					int	regTemp = AllocateExpressionRegister() ;
					m_pcsxi->WriteSakuraSllRegRegImm8
						( regTemp, typeinf.m_regIndex, typeinf.m_scaleIndex ) ;
					m_pcsxi->WriteSakuraAddRegReg( regDst, regTemp ) ;
					FreeExpressionRegister() ;
				}
				if ( typeinf.m_addrOffset != 0 )
				{
					m_pcsxi->WriteSakuraAddRegRegImm32
						( regDst, regDst, typeinf.m_addrOffset ) ;
				}
			}
			else
			{
				m_pcsxi->WriteSakuraAddRegRegImm32
					( regDst, typeinf.m_regIndex, typeinf.m_addrOffset ) ;
				m_pcsxi->WriteSakuraAddRegReg
					( regDst, typeinf.m_regBase ) ;
			}
		}
		else
		{
			m_pcsxi->WriteSakuraAddRegRegImm32
				( regDst, typeinf.m_regBase, typeinf.m_addrOffset ) ;
		}
	}
	else if ( !typeinf.IsLoadedRegister() )
	{
		if ( typeinf.m_pValue != NULL )
		{
			switch ( typeinf.m_pValue->m_vtType )
			{
			case	csvtInteger:
				{
					INT64	nValue = ((ECSInteger*)typeinf.m_pValue)->GetValue() ;
					switch ( nValue )
					{
					case	0:
						m_pcsxi->WriteSakuraMoveRegReg
							( regDst, ECSSakura2Processor::regIntZero ) ;
						break ;
					case	1:
						m_pcsxi->WriteSakuraMoveRegReg
							( regDst, ECSSakura2Processor::regIntOne ) ;
						break ;
					case	-1:
						m_pcsxi->WriteSakuraMoveRegReg
							( regDst, ECSSakura2Processor::regFillBit ) ;
						break ;
					case	0xFFFFFFFFUL:
						m_pcsxi->WriteSakuraMoveRegReg
							( regDst, ECSSakura2Processor::regMaskLow32 ) ;
						break ;
					case	0xFFFF:
						m_pcsxi->WriteSakuraMoveRegReg
							( regDst, ECSSakura2Processor::regMaskLow16 ) ;
						break ;
					case	0xFF:
						m_pcsxi->WriteSakuraMoveRegReg
							( regDst, ECSSakura2Processor::regMaskLow8 ) ;
						break ;
					default:
						if ( (-0x7FFFFFFF <= nValue) && (nValue <= 0x7FFFFFFF) )
						{
							m_pcsxi->WriteSakuraAddRegRegImm32
								( regDst, ECSSakura2Processor::regIntZero, (int) nValue ) ;
						}
						else
						{
							m_pcsxi->WriteSakuraLoadInt64( regDst, nValue ) ;
						}
						break ;
					}
				}
				break ;
			case	csvtReal:
				{
					double	nValue = ((ECSReal*)typeinf.m_pValue)->m_varReal ;
					if ( nValue == 0.0 )
					{
						m_pcsxi->WriteSakuraMoveRegReg
							( regDst, ECSSakura2Processor::regIntZero ) ;
					}
					else if ( nValue == 1.0 )
					{
						m_pcsxi->WriteSakuraMoveRegReg
							( regDst, ECSSakura2Processor::regFloatOne ) ;
					}
					else if ( fabs( nValue - 3.1415926535897932384626433832795 ) < 1.0e-15 )
					{
						m_pcsxi->WriteSakuraMoveRegReg
							( regDst, ECSSakura2Processor::regFloatPI ) ;
					}
					else
					{
						m_pcsxi->WriteSakuraLoadReal64( regDst, nValue ) ;
					}
				}
				break ;
			case	csvtReference:
				if ( ((ECSReference*)(typeinf.m_pValue))->m_pRef == NULL )
				{
					m_pcsxi->WriteSakuraMoveRegReg
						( regDst, ECSSakura2Processor::regIntZero ) ;
					break ;
				}
			default:
				OutputError
					( "naked モードで評価できない型です",
							m_strFilePath, m_nLineNum ) ;
				break ;
			}
		}
	}
	else if ( regDst != typeinf.GetLoadedRegister() )
	{
		m_pcsxi->WriteSakuraMoveRegReg
			( regDst, typeinf.GetLoadedRegister() ) ;
	}
}

// naked モードで参照型を展開する
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::CompileCodeNakedUncoverReference
		( ECSTypeInfo & typeinf, int regLoad )
{
	if ( !m_modeNakedCode )
	{
		return ;
	}
	if ( !typeinf.IsTypeReference() )
	{
		return ;
	}
	ESLAssert( typeinf.m_pValue != NULL ) ;
	ESLAssert( typeinf.m_pValue->m_vtType == csvtReference ) ;
	ECSReference *	pRefType = (ECSReference*) typeinf.m_pValue ;
	//
	ECSTypeInfo	typeTemp = typeinf ;
	if ( typeinf.IsAddressingInfo() )
	{
		int		regDst ;
		bool	fAllocReg = false ;
		if ( regLoad >= 0 )
		{
			regDst = regLoad ;
		}
		else if ( typeinf.IsLoadedRegister() )
		{
			regDst = typeinf.GetLoadedRegister() ;
		}
		else
		{
			regDst = AllocateExpressionRegister() ;
			fAllocReg = true ;
		}
		int	regTemp = regDst ;
		WriteSakuraLoadMemory( regTemp, typeinf ) ;
		typeinf.SetTypeValue
			( ECSTypeInfo::DuplicateType
					( pRefType->m_pRef ), typeinf.m_dwFlags ) ;
		if ( (regTemp != regDst) && (regTemp < 0x10)
			&& (typeinf.IsTypeReference() /*|| typeinf.IsTypePointer()*/) )
		{
			LockExpressionWorkRegister( regTemp ) ;
			typeinf.ClearLoadedRegister() ;
			typeinf.SetAddressingInfo( regTemp, 0 ) ;
			//
			if ( fAllocReg )
			{
				FreeExpressionRegister() ;
			}
		}
		else
		{
			if ( regTemp != regDst )
			{
				m_pcsxi->WriteSakuraMoveRegReg( regDst, regTemp ) ;
			}
			typeinf.ClearAddressingInfo() ;
			typeinf.SetLoadedRegister( regDst ) ;
		}
	}
	else
	{
		if ( ECSTypeInfo::GetObjectClassInfo
					( pRefType->m_pRef, *this ) != NULL )
		{
			typeinf.SetTypeValue
				( ECSTypeInfo::DuplicateType
					( pRefType->m_pRef ), typeinf.m_dwFlags ) ;
			typeinf.MoveRegisterAndAddressingFrom( typeTemp ) ;
			return ;
		}
		int	regDst, regAddr ;
		if ( typeinf.IsLoadedRegister() )
		{
			regAddr = typeinf.GetLoadedRegister() ;
		}
		else if ( regLoad >= 0 )
		{
			regAddr = regLoad ;
		}
		else
		{
			regAddr = AllocateExpressionRegister() ;
		}
		regDst = regAddr ;
		if ( regLoad >= 0 )
		{
			regDst = regLoad ;
		}
		typeinf.SetTypeValue
			( ECSTypeInfo::DuplicateType
				( typeTemp.GetSingleNakedType() ), typeTemp.m_dwFlags ) ;
		//
		ECSSakura2Processor::DataType
			type = ECSExecutionImageCompiler::DataTypeFromVariableType
					( ECSTypeInfo::GetNakedMemoryType( typeinf.m_pValue ) ) ;
		//
		int	regTemp = regDst ;
		WriteSakuraLoadMemory
			( ECSSakura2Processor::addrBaseIndex, type, regTemp,
				ECSSakura2Processor::regZeroPtr, 0, regAddr, 0 ) ;
		if ( regTemp != regDst )
		{
			m_pcsxi->WriteSakuraMoveRegReg( regDst, regTemp ) ;
		}
		typeinf.SetLoadedRegister( regDst ) ;
	}
}

// naked モードで列挙型を整数型か実数型に変換する
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::CompileCodeNakedUncoverEnumerator( ECSTypeInfo & typeinf )
{
	if ( !m_modeNakedCode )
	{
		return ;
	}
	ECSObject *	pType = typeinf.m_pValue ;
	if ( (pType != NULL) && (pType->m_vtType == csvtObject) )
	{
		const ECSClassInfo *	pClassInf = pType->m_pClassInf ;
		if ( pClassInf != NULL )
		{
			if ( pClassInf->IsIntegerEnumeratorType() )
			{
				ECSTypeInfo	typeAddr ;
				typeAddr.MoveRegisterAndAddressingFrom( typeinf ) ;
				typeinf.SetTypeValue( new ECSInteger, typeinf.m_dwFlags ) ;
				typeinf.MoveRegisterAndAddressingFrom( typeAddr ) ;
			}
			else if ( pClassInf->IsRealEnumeratorType() )
			{
				ECSTypeInfo	typeAddr ;
				typeAddr.MoveRegisterAndAddressingFrom( typeinf ) ;
				typeinf.SetTypeValue( new ECSReal, typeinf.m_dwFlags ) ;
				typeinf.MoveRegisterAndAddressingFrom( typeAddr ) ;
			}
		}
	}
}

// naked モードで整数型に変換する
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::CompileCodeNakedConvertToInt64( ECSTypeInfo & typeinf )
{
	ESLAssert( m_modeNakedCode ) ;
	if ( !m_modeNakedCode )
	{
		return ;
	}
	if ( typeinf.IsTypeInteger() || typeinf.IsRuntimeIntegerType() )
	{
		CompileCodeNakedUncoverEnumerator( typeinf ) ;
	}
	else if ( typeinf.IsTypeReal() )
	{
		if ( typeinf.IsLoadedRegister() )
		{
			int	regLoaded = typeinf.GetLoadedRegister() ;
			m_pcsxi->WriteSakuraCvt2IntRegReg( regLoaded, regLoaded ) ;
			//
			typeinf.SetTypeValue
				( new ECSInteger,
					(typeinf.m_dwFlags
						& ~ECSTypeInfo::flagDeterministic) ) ;
			typeinf.SetLoadedRegister( regLoaded ) ;
		}
		else
		{
			ECSObject *	pObj = typeinf.GetNakedType() ;
			if ( (pObj != NULL) && (pObj->m_vtType == csvtReal) )
			{
				INT64	nValue =
					eriRoundR64ToLInt( ((ECSReal*)pObj)->m_varReal ) ;
				typeinf.SetTypeValue
					( new ECSInteger( nValue ),
						(typeinf.m_dwFlags
							| ECSTypeInfo::flagDeterministic) ) ;
			}
		}
	}
}

ESLError ECSCompiler::CompileCodeNakedConvertToBoolean( ECSTypeInfo & typeinf )
{
	ESLAssert( m_modeNakedCode ) ;
	if ( !m_modeNakedCode )
	{
		return	eslErrSuccess ;
	}
	if ( typeinf.IsTypeBoolean() )
	{
		return	eslErrSuccess ;
	}
	else if ( typeinf.IsTypeInteger()
			|| typeinf.IsTypePointer()
			|| typeinf.IsRuntimeIntegerType() )
	{
		CompileCodeNakedUncoverEnumerator( typeinf ) ;
		if ( typeinf.IsTypePointer() )
		{
			MakeCommitValueToNakedRegister( typeinf ) ;
		}
		if ( typeinf.IsLoadedRegister() )
		{
			int	regLoaded = typeinf.GetLoadedRegister() ;
			m_pcsxi->WriteSakuraCmpNeRegReg
				( regLoaded, ECSSakura2Processor::regIntZero ) ;
			//
			typeinf.SetTypeValue
				( new ECSInteger( 0, ECSInteger::m_maskBoolean ),
					(typeinf.m_dwFlags & ~ECSTypeInfo::flagDeterministic) ) ;
			typeinf.SetLoadedRegister( regLoaded ) ;
		}
		else
		{
			ECSObject *	pObj = typeinf.GetNakedType() ;
			if ( (pObj != NULL) && (pObj->m_vtType == csvtInteger) )
			{
				INT64	nValue = ((ECSInteger*)pObj)->GetValue() ;
				nValue = (nValue != 0) ? -1 : 0 ;
				typeinf.SetTypeValue
					( new ECSInteger( nValue, ECSInteger::m_maskBoolean ),
						(typeinf.m_dwFlags | ECSTypeInfo::flagDeterministic) ) ;
			}
		}
		return	eslErrSuccess ;
	}
	else if ( typeinf.IsTypeReal() )
	{
		OutputWarning
			( "実数型をブール型に変換しようとしています",
								m_strFilePath, m_nLineNum ) ;
		if ( typeinf.IsLoadedRegister() )
		{
			int	regLoaded = typeinf.GetLoadedRegister() ;
			m_pcsxi->WriteSakuraCmpNeRegReg
				( regLoaded, ECSSakura2Processor::regIntZero ) ;
			//
			typeinf.SetTypeValue
				( new ECSInteger( 0, ECSInteger::m_maskBoolean ),
					(typeinf.m_dwFlags & ~ECSTypeInfo::flagDeterministic) ) ;
			typeinf.SetLoadedRegister( regLoaded ) ;
		}
		else
		{
			ECSObject *	pObj = typeinf.GetNakedType() ;
			if ( (pObj != NULL) && (pObj->m_vtType == csvtReal) )
			{
				INT64	nValue = (((ECSReal*)pObj)->m_varReal != 0) ? -1 : 0 ;
				typeinf.SetTypeValue
					( new ECSInteger( nValue, ECSInteger::m_maskBoolean ),
						(typeinf.m_dwFlags | ECSTypeInfo::flagDeterministic) ) ;
			}
		}
		return	eslErrSuccess ;
	}
	return	ESLErrorMsg( "boolean に変換出来ません" ) ;
}

// naked モードで実数型に変換する
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::CompileCodeNakedConvertToReal( ECSTypeInfo & typeinf )
{
	ESLAssert( m_modeNakedCode ) ;
	if ( !m_modeNakedCode )
	{
		return ;
	}
	if ( typeinf.IsTypeReal() )
	{
	}
	else if ( typeinf.IsTypeInteger() || typeinf.IsRuntimeIntegerType() )
	{
		CompileCodeNakedUncoverEnumerator( typeinf ) ;
		if ( typeinf.IsLoadedRegister() )
		{
			int	regLoaded = typeinf.GetLoadedRegister() ;
			m_pcsxi->WriteSakuraCvt2FloatRegReg( regLoaded, regLoaded ) ;
			//
			typeinf.SetTypeValue
				( new ECSReal( 0 ),
					(typeinf.m_dwFlags & ~ECSTypeInfo::flagDeterministic) ) ;
			typeinf.SetLoadedRegister( regLoaded ) ;
		}
		else
		{
			ECSObject *	pObj = typeinf.GetNakedType() ;
			if ( (pObj != NULL) && (pObj->m_vtType == csvtInteger) )
			{
				INT64	nValue = ((ECSInteger*)pObj)->GetValue() ;
				typeinf.SetTypeValue
					( new ECSReal( (double) nValue ),
						(typeinf.m_dwFlags | ECSTypeInfo::flagDeterministic) ) ;
			}
		}
	}
}

// 自然数を 2^n に変換できるか判定し n を取得
//////////////////////////////////////////////////////////////////////////////
int ECSCompiler::GetNumberScale( INT64 nValue )
{
	if ( nValue > 0 )
	{
		INT64	nBit = 1 ;
		for ( int i = 0; i < 64; i ++, nBit <<= 1 )
		{
			if ( nValue == nBit )
			{
				return	i ;
			}
			else if ( nValue & nBit )
			{
				break ;
			}
		}
	}
	return	-1 ;
}

// 多次元配列オブジェクト構築命令を出力
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileArrayDimension( ECSArray * pArray )
{
	if ( m_modeNakedCode )
	{
		return	ESLErrorMsg( "object 配列は naked モードでは生成出来ません" ) ;
	}
	ECSObject *	pElementType = pArray->GetEndDefaultElement() ;
	if ( pElementType != NULL )
	{
		ECSTypeInfo	typeTemp ;
		ESLError	err =
			CompileImmediateObject( typeTemp, pElementType ) ;
		if ( err )
		{
			return	err ;
		}
		m_pcsxi->WriteInstructionCode( csicExOperate ) ;
		m_pcsxi->WriteExOperatorTypeCode( csxotArrayDim ) ;
		//
		DWORD	dwDimension = pArray->GetDimension() ;
		unsigned int *	pBounds = new unsigned int [dwDimension] ;
		dwDimension =
			pArray->GetDimensionSize( pBounds, dwDimension ) ;
		//
		m_pcsxi->WriteCodeData( &dwDimension, sizeof(DWORD) ) ;
		m_pcsxi->WriteCodeData
				( pBounds, dwDimension * sizeof(unsigned int) ) ;
		//
		delete []	pBounds ;
	}
	unsigned int	i, nCount ;
	nCount = pArray->m_varArray.GetSize() ;
	for ( i = 0; i < nCount; i ++ )
	{
		ECSObject *	pElement = pArray->m_varArray.GetAt( i ) ;
		if ( pElement != NULL )
		{
			ECSTypeInfo	typeTemp ;
			ESLError	err =
				CompileImmediateObject( typeTemp, pElement ) ;
			if ( err )
			{
				return	err ;
			}
			CompileCodeStore( csotAdd ) ;
		}
	}
	return	eslErrSuccess ;
}

// ハッシュコンテナオブジェクト構築命令を出力
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileHashContainer( ECSHash * pHash )
{
	if ( m_modeNakedCode )
	{
		return	ESLErrorMsg( "object 配列は naked モードでは生成出来ません" ) ;
	}
	ECSObject *	pElementType = pHash->m_pDefObj ;
	if ( pElementType != NULL )
	{
		ECSTypeInfo	typeTemp ;
		ESLError	err =
			CompileImmediateObject( typeTemp, pElementType ) ;
		if ( err )
		{
			return	err ;
		}
		m_pcsxi->WriteInstructionCode( csicExOperate ) ;
		m_pcsxi->WriteExOperatorTypeCode( csxotHashContainer ) ;
	}
	unsigned int	i, nCount ;
	nCount = pHash->m_varArray.GetSize() ;
	for ( i = 0; i < nCount; i ++ )
	{
		ECSWideString *	pTag = pHash->m_varArray.GetTagAt( i ) ;
		ECSObject *		pElement = pHash->m_varArray.GetObjectAt( i ) ;
		if ( (pTag != NULL) && (pElement != NULL) )
		{
			CompileLoadStackObject( 0 ) ;
			//
			m_pcsxi->WriteInstructionCode( csicElement ) ;
			m_pcsxi->WriteVariableTypeCode( csvtString ) ;
			m_pcsxi->WriteConstantString( *pTag ) ;
			//
			ECSTypeInfo	typeTemp ;
			ESLError	err =
				CompileImmediateObject( typeTemp, pElement ) ;
			if ( err )
			{
				return	err ;
			}
			CompileCodeStore( csotNop ) ;
			CompileCodeFreeStack() ;
		}
	}
	return	eslErrSuccess ;
}

// マクロ式解釈
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileMacroExpression
	( const EWideString & wstrMacroName,
		ECSSourceStream & cssLine,
		ECSTypeInfo & typeinf,
		ECSCompiler::MacroExpression & mxprResult )
{
	ESLError	err ;
	mxprResult.pValue = NULL ;
	mxprResult.fOwnValue = false ;
	mxprResult.fValidMacro = false ;
	//
	// 定数値（マクロ変数）判定
	//
	ECSObject *	pObj = GetMacroVariable( wstrMacroName ) ;
	if ( pObj == NULL )
	{
		SYMBOL_NAMESPACE	snsSymbol ;
		snsSymbol.ParseSymbol( wstrMacroName ) ;
		if ( !snsSymbol.wstrNamespace.IsEmpty() )
		{
			ECSClassInfo *	pClassInf =
				GetClassInfoAs( snsSymbol.wstrNamespace ) ;
			if ( pClassInf != NULL )
			{
				EPtrObjArray<const wchar_t>	lstNamespace ;
				AddUsingParentClassList( lstNamespace, pClassInf ) ;
				for ( unsigned int i = 0; i < lstNamespace.GetSize(); i ++ )
				{
					const wchar_t *	pszNamespace = lstNamespace.GetAt( i ) ;
					EWideString	wstrTempName = pszNamespace ;
					wstrTempName += L"::" ;
					wstrTempName += snsSymbol.wstrName ;
					pObj = GetMacroVariable( wstrTempName ) ;
					if ( pObj != NULL )
					{
						break ;
					}
				}
			}
		}
	}
	if ( pObj == NULL )
	{
		//
		// マクロ関数判定
		//
		EMacroBlock *	pmbMacro = m_staMacro.GetAs( wstrMacroName ) ;
		if ( (pmbMacro != NULL) && pmbMacro->m_fMacroFunc )
		{
			EObjArray<EWideString>	lstParam ;
			err = CompileMacroArgument( lstParam, cssLine ) ;
			if ( err )
			{
				return	err ;
			}
			err = CompileUserMacro( pmbMacro, lstParam, &pObj ) ;
			if ( err )
			{
				return	err ;
			}
			if ( pObj == NULL )
			{
				return	ESLErrorMsg( "マクロ関数に返り値がありません。" ) ;
			}
		}
		else if ( (wstrMacroName == L"@")
			|| !CompareReservedWord( L"@Compile", wstrMacroName ) )
		{
			wchar_t	wch = cssLine.HasToComeChar( L"(" ) ;
			if ( wch != L'(' )
			{
				return	ESLErrorMsg
					( "@compile マクロ関数に引数がありません。" ) ;
			}
			ECSObject *	pMacro = NULL ;
			err = CalculateExpression( pMacro, cssLine, 0, L")" ) ;
			if ( err )
			{
				return	err ;
			}
			if ( cssLine.HasToComeChar( L")" ) != L')' )
			{
				return	ESLErrorMsg
					( "@compile マクロ関数の引数が \')\' 括弧で閉じられていません。" ) ;
			}
			if ( pMacro->m_vtType != csvtString )
			{
				return	ESLErrorMsg
					( "@compile マクロ関数の引数が文字列型でありません。" ) ;
			}
			ECSSourceStream	cssExpr = ((ECSString*)pMacro)->m_varStr ;
			delete	pMacro ;
			if ( cssExpr.DisregardSpace() )
			{
				pObj = new ECSInteger( 0 ) ;
				mxprResult.fOwnValue = true ;
			}
			else
			{
				err = CompileExpression( typeinf, cssExpr ) ;
				if ( err )
				{
					return	err ;
				}
				mxprResult.fValidMacro = true ;
				return	eslErrSuccess ;
			}
		}
		else if ( wstrMacroName == L"null" )
		{
			pObj = new ECSReference ;
			mxprResult.fOwnValue = true ;
		}
		else if ( wstrMacroName == L"true" )
		{
			pObj = new ECSInteger( -1, ECSInteger::m_maskBoolean ) ;
			mxprResult.fOwnValue = true ;
		}
		else if ( wstrMacroName == L"false" )
		{
			pObj = new ECSInteger( 0, ECSInteger::m_maskBoolean ) ;
			mxprResult.fOwnValue = true ;
		}
		else
		{
			return	eslErrSuccess ;
		}
	}
	else
	{
		//
		// マクロ変数発見
		//
	}
	ESLAssert( pObj != NULL ) ;
	mxprResult.pValue = pObj ;
	mxprResult.fValidMacro = true ;
	//
	return	CompileConstantArrayElement( mxprResult, cssLine ) ;
}

// マクロ配列変数（定数配列オブジェクト）の要素参照文の解釈
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileConstantArrayElement
	( ECSCompiler::MacroExpression & mxprResult, ECSSourceStream & cssLine )
{
	if ( !mxprResult.fValidMacro || (mxprResult.pValue == NULL) )
	{
		return	eslErrSuccess ;
	}
	//
	// 定数配列要素指定
	//
	ESLError	err ;
	ECSObject *	pObj = mxprResult.pValue ;
	while ( (pObj != NULL)
		/*&& ((pObj->m_vtType == csvtArray)
				|| (pObj->m_vtType == csvtHash))*/ )
	{
		if ( cssLine.HasToComeChar( L"[" ) != L'[' )
		{
			break ;
		}
		ECSObject *	pIndex = NULL ;
		err = CalculateExpression( pIndex, cssLine, 0, L"]" ) ;
		if ( err )
		{
			return	err ;
		}
		if ( cssLine.HasToComeChar( L"]" ) != L']' )
		{
			return	ESLErrorMsg( "\']\' 閉じ括弧がありません。" ) ;
		}
		int	nIndex ;
		if ( pIndex->m_vtType == csvtInteger )
		{
			err = pObj->GetVariableIndex
				( nIndex, ((ECSInteger*)pIndex)->GetInt() ) ;
		}
		else if ( pIndex->m_vtType == csvtString )
		{
			err = pObj->GetVariableIndex
				( nIndex, ((ECSString*)pIndex)->m_varStr ) ;
		}
		else
		{
			err = ESLErrorMsg
				( "指標に整数でも文字列でもないオブジェクトが指定されています。" ) ;
		}
		delete	pIndex ;
		if ( err )
		{
			return	err ;
		}
		ECSObject *	pElement = pObj->GetVariableAt( nIndex ) ;
		if ( pElement == NULL )
		{
			return	ESLErrorMsg
				( "定数式の参照している配列要素がありません。" ) ;
		}
		if ( mxprResult.fOwnValue )
		{
			pElement = pElement->Duplicate() ;
			delete	mxprResult.pValue ;
		}
		pObj = mxprResult.pValue = pElement ;
	}
	return	eslErrSuccess ;
}

// オブジェクト構築式の解釈
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileTypeConstruction
	( ECSSourceStream & cssLine,
		ECSTypeInfo & typeinf, bool & fTypeConstruction,
		const EWideString & wstrNameSpace, const EWideString & wstrTypeName )
{
	//
	// 型式を解釈
	//
	EPtrObjArray<const wchar_t>	lstNamespace ;
	EWideString	wstrThisClass = GetCurrentThisClassName() ;
	GetUsingNamespaceList( lstNamespace ) ;
	//
	EWideString	wstrGlobalName ;
	EWideString	wstrThisSpace ;
	ESLError	err = eslErrGeneral ;
	for ( int i = 0; i < (int) lstNamespace.GetSize(); i ++ )
	{
		const wchar_t *	pwszNamespace = lstNamespace.GetAt( i ) ;
		if ( pwszNamespace != NULL )
		{
			wstrThisSpace = pwszNamespace ;
			ECSTypeInfo::Flags	flagScope = ECSTypeInfo::flagProtected ;
			if ( wstrThisClass != pwszNamespace )
			{
				flagScope = ECSTypeInfo::flagPublic ;
			}
			if ( !wstrNameSpace.IsEmpty() )
			{
				wstrThisSpace += L"::" ;
				wstrThisSpace += wstrNameSpace ;
				flagScope = ECSTypeInfo::flagPublic ;
			}
			SYMBOL_NAMESPACE	snsSymbol ;
			snsSymbol.wstrName = wstrTypeName ;
			snsSymbol.wstrNamespace = wstrThisSpace ;
			//
			err = SearchTypeName( snsSymbol, flagScope ) ;
			if ( !err )
			{
				wstrGlobalName = snsSymbol.wstrFullName ;
				break ;
			}
		}
	}
	if ( err )
	{
		SYMBOL_NAMESPACE	snsSymbol ;
		snsSymbol.wstrName = wstrTypeName ;
		snsSymbol.wstrNamespace = wstrNameSpace ;
		//
		err = SearchTypeName( snsSymbol, ECSTypeInfo::flagPublic ) ;
		//
		wstrGlobalName = snsSymbol.wstrFullName ;
	}
	if ( err )
	{
		fTypeConstruction = false ;
		return	eslErrSuccess ;
	}
	//
	// 型情報取得
	//
	err = GetSimpleTypeInfoAs( typeinf, wstrGlobalName ) ;
	if ( err )
	{
		fTypeConstruction = false ;
		return	eslErrSuccess ;
	}
	//
	// オブジェクト構築命令
	//
	err = CompileNewTypeConstruction( typeinf, cssLine ) ;
	if ( err )
	{
		return	err ;
	}
	fTypeConstruction = true ;
	return	eslErrSuccess ;
}

ESLError ECSCompiler::CompileNewTypeConstruction
	( ECSTypeInfo & typeinf, ECSSourceStream & cssLine )
{
	if ( m_modeNakedCode )
	{
		return	CompileNakedTypeConstruction( typeinf, cssLine ) ;
	}
	ECSTypeInfo	typeTemp ;
	ESLError	err = CompileImmediateObject( typeTemp, typeinf.m_pValue ) ;
	if ( err )
	{
		return	err ;
	}
	if ( typeinf.IsPureType() )
	{
		const ECSClassInfo *	pClassInf = typeinf.GetClassInfo() ;
		if ( (pClassInf != NULL)
			&& (pClassInf->GetAttribute() & ECSTypeInfo::flagAbstract) )
		{
			m_strErrMsg = EString( pClassInf->GetGlobalName() )
							+ " 抽象クラスを生成しようとしています。" ;
			return	ESLErrorMsg( m_strErrMsg ) ;
		}
		if ( cssLine.HasToComeChar( L"(" ) == L'(' )
		{
			if ( (pClassInf != NULL)
				&& !(pClassInf->GetAttribute()
							& ECSTypeInfo::flagNativeObject) )
			{
				//
				// 構築関数の引数型取得と呼び出し
				//
				const ECSClassInfo::MemberFunction *	pFunc ;
				err = CompileArgumentAndCallMemberFunction
					( cssLine, *pClassInf, typeinf,
							pClassInf->GetName(), true,
							pFunc, ECSTypeInfo::flagPublic ) ;
				if ( err )
				{
					return	err ;
				}
				if ( (pFunc != NULL)
					&& !pFunc->GetReturnType().IsVoid() )
				{
					err = OutputWarning0
						( "構築関数の返り値が定義されています。",
										m_strFilePath, m_nLineNum ) ;
					if ( err )
					{
						return	err ;
					}
					CompileCodeFreeStack() ;
				}
			}
			else
			{
				//
				// 代入処理
				//
				ECSTypeInfo	typeTemp ;
				ESLError	err =
					CompileExpression( typeTemp, cssLine, 0, 0, L")" ) ;
				if ( err )
				{
					return	err ;
				}
				if ( cssLine.HasToComeChar( L")" ) != L')' )
				{
					return	ESLErrorMsg
						( "\'(\' に対応する \')\' が見つかりません。" ) ;
				}
				if ( pClassInf != NULL )
				{
					ECSClassInfo::ListMemberFunction	lstFunc ;
					EPtrObjArray<ECSTypeInfo>			lstArg ;
					lstArg.Add( &typeTemp ) ;
					//
					if ( !pClassInf->SearchFunctinoAs
							( lstFunc, pClassInf->GetName(),
								lstArg, 0, true, false, m_modeNakedCode )
						&& (lstFunc.GetSize() == 0) )
					{
						m_strErrMsg = EString(pClassInf->GetGlobalName())
							+ " 型の適合する構築関数がみつかりません。" ;
						return	ESLErrorMsg( m_strErrMsg ) ;
					}
				}
				CompileCodeStore( csotNop ) ;
			}
		}
		else if ( pClassInf != NULL )
		{
			//
			// デフォルトのコンストラクタ呼び出し
			//
			if ( IsDefaultConstructor( *pClassInf ) )
			{
				ESLError	err =
					CompileCallDefaultConstructor( *pClassInf, true ) ;
				if ( err )
				{
					return	err ;
				}
			}
		}
	}
	return	eslErrSuccess ;
}

ESLError ECSCompiler::CompileNakedTypeConstruction
	( ECSTypeInfo & typeinf, ECSSourceStream & cssLine )
{
	ECSObject *	pType = typeinf.m_pValue ;
	if ( (pType != NULL)
		&& ((pType->m_vtType == csvtInteger)
			|| (pType->m_vtType == csvtReal)) )
	{
		if ( cssLine.HasToComeChar( L"(" ) != L'(' )
		{
			return	ESLErrorMsg
				( "naked モードでオブジェクトの構築構文が指定されています" ) ;
		}
		//
		// 基本型のキャスト
		//
		ECSTypeInfo	typeExpr ;
		ESLError	err =
			CompileExpression( typeExpr, cssLine, 0, 0, L")" ) ;
		if ( err )
		{
			return	err ;
		}
		if ( cssLine.HasToComeChar( L")" ) != L')' )
		{
			return	ESLErrorMsg
				( "\'(\' に対応する \')\' が見つかりません。" ) ;
		}
		ECSTypeInfo	typeCast = typeinf ;
		return	CompileTypeCast
					( typeinf, typeCast, typeExpr,
						castStatic, ECSTypeInfo::flagPublic ) ;
	}
	//
	// クラス情報取得
	//
	const ECSClassInfo *	pClassInf = GetTypeClassInfo( pType ) ;
	if ( (pClassInf == NULL)
		|| !pClassInf->IsNakedMemoryClass() )
	{
		return	ESLErrorMsg
			( "naked モードで naked クラスでない"
				"一時オブジェクトを生成しようとしています" ) ;
	}
	//
	// 一時オブジェクト生成
	//
	ECSTypeInfo	typeObj = typeinf ;
	ECSTypeInfo	typeTemp ;
	ECSTypeInfo *	pVarTemp =
		CompilePrepareNakedTemporaryObject( typeTemp, typeObj ) ;
	if ( pVarTemp == NULL )
	{
		return	ESLErrorMsg( "一時オブジェクトの生成に失敗しました" ) ;
	}
	//
	// 構築関数の引数型取得と呼び出し
	//
	if ( cssLine.HasToComeChar( L"(" ) == L'(' )
	{
		const ECSClassInfo::MemberFunction *	pFunc ;
		ESLError	err =
			CompileArgumentAndCallMemberFunction
				( cssLine, *pClassInf, typeObj,
						pClassInf->GetName(), false,
						pFunc, ECSTypeInfo::flagPublic ) ;
		if ( err )
		{
			return	err ;
		}
		if ( (pFunc != NULL)
			&& !pFunc->GetReturnType().IsVoid() )
		{
			err = OutputWarning0
				( "構築関数の返り値が定義されています。",
								m_strFilePath, m_nLineNum ) ;
			if ( err )
			{
				return	err ;
			}
			CompileCodeFreeStack() ;
		}
	}
	else
	{
		if ( IsDefaultConstructor( *pVarTemp ) )
		{
			ESLError	err =
				CompileCallDefaultConstructor( *pVarTemp, false ) ;
			if ( err )
			{
				return	err ;
			}
		}
		else
		{
			FreeExpressionRegister( typeTemp ) ;
		}
	}
	//
	// 一時オブジェクトへの参照
	//
	typeinf.MakeReferenceOf( *pVarTemp ) ;
	typeinf.SetAddressingInfo( *pVarTemp ) ;
	return	eslErrSuccess ;
}

ESLError ECSCompiler::CompileNewExpression
	( ECSTypeInfo & typeinf, ECSSourceStream & cssLine )
{
	bool	fNaked = m_modeNakedCode
					|| ((m_dwModeFlags & flagDefaultNakedNew) != 0) ;
	bool	fExpresslyNaked = false ;
	bool	fExpresslyShared = false ;
	for ( ; ; )
	{
		if ( cssLine.HasToComeToken( L"naked" ) )
		{
			fNaked = true ;
			fExpresslyNaked = true ;
		}
		else if ( cssLine.HasToComeToken( L"shared" ) )
		{
			fExpresslyShared = true ;
		}
		else
		{
			break ;
		}
	}
	ESLError	err ;
	if ( !fNaked )
	{
		//
		// 通常 new 演算子
		//
		err = ParseTypeDescription( typeinf, cssLine ) ;
		if ( !err )
		{
			err = CompileNewTypeConstruction( typeinf, cssLine ) ;
		}
		return	err ;
	}
	//
	// naked 型取得
	//
	ECSTypeInfo			typeNakedType ;
	SYMBOL_NAMESPACE	snsSymbol = cssLine.GetAToken() ;
	err = ParseFullNameSymbol( snsSymbol, cssLine ) ;
	if ( err )
	{
		return	err ;
	}
	err = GetSimpleTypeInfoAs( typeNakedType, snsSymbol.wstrFullName ) ;
	if ( err )
	{
		return	err ;
	}
	if ( !typeNakedType.IsPureType() )
	{
		return	ESLErrorMsg( "new naked できない型指定です" ) ;
	}
	err = ParseTypePointerDecoration( typeNakedType, cssLine ) ;
	if ( err )
	{
		return	err ;
	}
	ECSObject *	pType = typeNakedType.m_pValue ;
	if ( pType == NULL )
	{
		return	ESLErrorMsg( "new naked できない型指定です" ) ;
	}
	//
	// naked サイズ取得
	//
	DWORD					dwElementBytes = 0 ;
	const ECSClassInfo *	pClassInf = NULL ;
	switch ( pType->m_vtType )
	{
	case	csvtInteger:
		dwElementBytes = ((ECSInteger*)pType)->SizeOf() / 8 ;
		break ;
	case	csvtReal:
		dwElementBytes = ((ECSReal*)pType)->SizeOf() / 8 ;
		break ;
	case	csvtPointer:
		dwElementBytes = 8 ;
		break ;
	case	csvtObject:
		pClassInf = pType->m_pClassInf ;
		if ( pClassInf != NULL )
		{
			if ( !(pClassInf->GetAttribute() & ECSTypeInfo::flagNakedBuffer) )
			{
				if ( fExpresslyNaked )
				{
					return	ESLErrorMsg
						( "naked でない型を new naked しようとしています" ) ;
				}
				fNaked = false ;
			}
			else
			{
				dwElementBytes = pClassInf->GetNakedMemorySize() ;
			}
			break ;
		}
	case	csvtHash:
	case	csvtString:
		pClassInf = GetClassInfoAs( pType->GetTypeName() ) ;
		fNaked = false ;
		break ;
	default:
		return	ESLErrorMsg( "new naked できない型指定です" ) ;
	}
	if ( m_modeNakedCode )
	{
		typeNakedType.m_dwFlags |= ECSTypeInfo::flagNakedBuffer ;
	}
	if ( cssLine.HasToComeChar( L"[" ) != L'[' )
	{
		//
		// 配列でない new 演算子
		//
		if ( m_modeNakedCode )
		{
			if ( fNaked || (pClassInf == NULL) )
			{
				m_pcsxi->WriteSakuraLoadInt64
					( AllocateExpressionRegister(), dwElementBytes ) ;
				if ( fExpresslyShared )
				{
					CompileNakedSystemCall( L"shared_malloc", 1 ) ;
				}
				else
				{
					CompileNakedSystemCall( L"malloc", 1 ) ;
				}
			}
			else
			{
				m_pcsxi->WriteSakuraLoadInt64_ClassID
					( AllocateExpressionRegister(),
								pClassInf->GetGlobalName() ) ;
				if ( fExpresslyShared )
				{
					CompileNakedSystemCall( L"object_shared_new", 1 ) ;
				}
				else
				{
					CompileNakedSystemCall( L"object_new", 1 ) ;
				}
			}
			m_pcsxi->WriteSakuraMoveRegReg
				( AllocateExpressionRegister(),
							ECSSakura2Processor::regAcc ) ;
		}
		else
		{
			if ( fNaked )
			{
				m_pcsxi->WriteInstructionCode( csicCreateBuffer ) ;
				m_pcsxi->WriteCodeData( &dwElementBytes, sizeof(DWORD) ) ;
			}
			else
			{
				err = CompileNewTypeConstruction( typeNakedType, cssLine ) ;
				if ( err )
				{
					return	err ;
				}
			}
			CompileCodePointerToObject( 0 ) ;
		}
		//
		// メモリ初期化・構築関数呼び出し
		//
		if ( fNaked && (pClassInf != NULL) )
		{
			CompileCodeNakedClassInitialize( pClassInf ) ;
			//
			if ( m_modeNakedCode )
			{
				int	regNewPtr = GetExpressionRegister() ;
				m_pcsxi->WriteSakuraMoveRegReg
					( AllocateExpressionRegister(),regNewPtr ) ;
			}
			else
			{
				CompileLoadStackObject( 0 ) ;
				CompileCodeReferenceForPointer( csvtObject ) ;
			}
			if ( cssLine.HasToComeChar( L"(" ) == L'(' )
			{
				//
				// 構築関数の引数型取得と呼び出し
				//
				const ECSClassInfo::MemberFunction *	pFunc ;
				err = CompileArgumentAndCallMemberFunction
					( cssLine, *pClassInf, typeNakedType,
							pClassInf->GetName(), false,
							pFunc, ECSTypeInfo::flagPublic ) ;
				if ( err )
				{
					return	err ;
				}
				if ( (pFunc != NULL)
					&& !pFunc->GetReturnType().IsVoid() )
				{
					err = OutputWarning0
						( "構築関数の返り値が定義されています。",
										m_strFilePath, m_nLineNum ) ;
					if ( err )
					{
						return	err ;
					}
					CompileCodeFreeStack() ;
				}
			}
			else if ( IsDefaultConstructor( *pClassInf ) )
			{
				//
				// デフォルトコンストラクタ
				//
				err = CompileCallDefaultConstructor( *pClassInf, false ) ;
				if ( err )
				{
					return	err ;
				}
			}
			else
			{
				CompileCodeFreeStack() ;
			}
		}
		typeinf.MakePointerOf( typeNakedType ) ;
		//
		if ( m_modeNakedCode )
		{
			typeinf.SetLoadedRegister( GetExpressionRegister() ) ;
		}
		return	eslErrSuccess ;
	}
	else if ( !fNaked )
	{
		return	ESLErrorMsg
			( "naked でないクラスの配列を new naked しようとしています" ) ;
	}
	//
	// 配列サイズ取得
	//
	int			nIndex = cssLine.GetIndex() ;
	ECSObject *	pValue = NULL ;
	err = CalculateExpression( pValue, cssLine, 0, L"]" ) ;
	if ( !err )
	{
		if ( (pValue == NULL) || (pValue->m_vtType != csvtInteger) )
		{
			return	ESLErrorMsg( "配列サイズが整数ではありません" ) ;
		}
		if ( cssLine.HasToComeChar( L"]" ) != L']' )
		{
			return	ESLErrorMsg
				( "new naked 配列の \'[\' に対応する \']\' が見つかりません" ) ;
		}
		err = ParseTypeArrayDecoration( typeNakedType, cssLine ) ;
		if ( err )
		{
			return	err ;
		}
		int	nNakedSize, nObjCount ;
		err = typeNakedType.GetNakedMemorySize( nNakedSize, nObjCount ) ;
		if ( err )
		{
			return	err ;
		}
		INT64	nArraySize = ((ECSInteger*)pValue)->GetValue() ;
		DWORD	dwBufBytes = nNakedSize * (int) nArraySize + 8 ;
		delete	pValue ;
		//
		nArraySize *= typeNakedType.CalcNakedMemoryArraySize() ;
		//
		if ( m_modeNakedCode )
		{
			m_pcsxi->WriteSakuraLoadInt64
				( AllocateExpressionRegister(), dwBufBytes ) ;
			if ( fExpresslyShared )
			{
				CompileNakedSystemCall( L"shared_malloc", 1 ) ;
			}
			else
			{
				CompileNakedSystemCall( L"malloc", 1 ) ;
			}
			//
			m_pcsxi->WriteSakuraMoveRegReg
				( AllocateExpressionRegister(),
							ECSSakura2Processor::regAcc ) ;
			m_pcsxi->WriteSakuraLoadInt64
				( AllocateExpressionRegister(), nArraySize ) ;
		}
		else
		{
			m_pcsxi->WriteInstructionCode( csicCreateBuffer ) ;
			m_pcsxi->WriteCodeData( &dwBufBytes, sizeof(DWORD) ) ;
			//
			CompileCodePointerToObject( 0 ) ;
			//
			CompileImmediateInteger( nArraySize ) ;
		}
	}
	else
	{
		ECSTypeInfo	typeSize ;
		cssLine.MoveIndex( nIndex ) ;
		err = CompileExpression( typeSize, cssLine, 0, 0, L"]" ) ;
		if ( err )
		{
			return	err ;
		}
		pValue = ECSObject::GetEntity( typeSize.m_pValue ) ;
		if ( (pValue == NULL) || (pValue->m_vtType != csvtInteger) )
		{
			return	ESLErrorMsg( "配列サイズが整数ではありません" ) ;
		}
		if ( cssLine.HasToComeChar( L"]" ) != L']' )
		{
			return	ESLErrorMsg
				( "new naked 配列の \'[\' に対応する \']\' が見つかりません" ) ;
		}
		err = ParseTypeArrayDecoration( typeNakedType, cssLine ) ;
		if ( err )
		{
			return	err ;
		}
		const int	nArraySize = typeNakedType.CalcNakedMemoryArraySize() ;
		int	nNakedSize, nObjCount ;
		err = typeNakedType.GetNakedMemorySize( nNakedSize, nObjCount ) ;
		if ( err )
		{
			return	err ;
		}
		if ( m_modeNakedCode )
		{
			CompileCodeNakedUncoverReference( typeSize ) ;
			MakeCommitValueToNakedRegister( typeSize ) ;
			ESLAssert( typeSize.IsLoadedRegister() ) ;
			//
			int	regArrayCount =
					typeSize.IsLoadedRegister()
						? typeSize.GetLoadedRegister()
								: GetExpressionRegister() ;
			if ( nArraySize > 1 )
			{
				m_pcsxi->WriteSakuraMulRegRegImm32
					( regArrayCount, regArrayCount, nArraySize ) ;
			}
			int	regMemSize = AllocateExpressionRegister() ;
			m_pcsxi->WriteSakuraMulRegRegImm32
				( regMemSize, regArrayCount, nNakedSize / nArraySize ) ;
			m_pcsxi->WriteSakuraAddRegRegImm32( regMemSize, regMemSize, 8 ) ;
			if ( fExpresslyShared )
			{
				CompileNakedSystemCall( L"shared_malloc", 1 ) ;
			}
			else
			{
				CompileNakedSystemCall( L"malloc", 1 ) ;
			}
			//
			m_pcsxi->WriteSakuraMoveRegReg
				( AllocateExpressionRegister(), regArrayCount ) ;
			m_pcsxi->WriteSakuraMoveRegReg
				( regArrayCount, ECSSakura2Processor::regAcc ) ;
		}
		else
		{
			if ( nArraySize > 1 )
			{
				CompileImmediateInteger( nArraySize ) ;
				CompileCodeOperate( csotMul ) ;
			}
			CompileLoadStackObject( 0 ) ;
			//
			CompileImmediateInteger( nNakedSize / nArraySize ) ;
			CompileCodeOperate( csotMul ) ;
			//
			CompileImmediateInteger( 8 ) ;
			CompileCodeOperate( csotAdd ) ;
			//
			m_pcsxi->WriteInstructionCode( csicCreateBufferVSize ) ;
			//
			CompileCodePointerToObject( 0 ) ;
			//
			CompileCodeSwap( 0, 1 ) ;
		}
	}
	//
	// 配列の全体サイズを計算
	//
	/*
	if ( (typeNakedType.m_pValue != NULL)
		&& (typeNakedType.m_pValue->m_vtType == csvtArray) )
	{
		ECSArray *		pArray = (ECSArray*) typeNakedType.m_pValue ;
		int				nDim = pArray->GetDimension() ;
		unsigned int *	pBounds = new unsigned int[nDim] ;
		int				nMultipleElements = 1 ;
		nDim = pArray->GetDimensionSize( pBounds, nDim ) ;
		for ( int i = 0; i < nDim; i ++ )
		{
			nMultipleElements *= pBounds[i] ;
		}
		delete []	pBounds ;
		//
		if ( nMultipleElements != 1 )
		{
			CompileImmediateInteger( nMultipleElements ) ;
			CompileCodeOperate( csotMul ) ;
		}
	}
	*/
	//
	// アドレスの先頭に配列サイズをセットし、ポインタを補正
	//
	int	regNewPtr, regCounter ;
	if ( m_modeNakedCode )
	{
		regNewPtr = GetExpressionRegister( 1 ) ;
		regCounter = GetExpressionRegister( 0 ) ;
		//
		int	regDst = regCounter ;
		WriteSakuraStoreMemory
			( ECSSakura2Processor::addrBase,
				ECSSakura2Processor::dataInt64,
				regDst, regNewPtr, 0 ) ;
		//
		m_pcsxi->WriteSakuraAddRegRegImm32( regNewPtr, regNewPtr, 8 ) ;
	}
	else
	{
		CompileLoadStackObject( 1 ) ;					// pointer
		CompileCodeReferenceForPointer( csvtInteger ) ;
		//
		CompileLoadStackObject( 1 ) ;					// size
		CompileCodeStore( csotNop ) ;
		//
		CompileCodeFreeStack() ;
		CompileCodeSwap( 0, 1 ) ;
		//
		CompileImmediateInteger( 8 ) ;
		CompileCodeOperate( csotAdd ) ;
		CompileCodeSwap( 0, 1 ) ;
	}
	//
	// 初期値の設定と構築関数呼び出し
	//
	if ( pClassInf != NULL )
	{
		int	regNextPtr ;
		if ( m_modeNakedCode )
		{
			regNextPtr = AllocateExpressionRegister() ;
			m_pcsxi->WriteSakuraMoveRegReg( regNextPtr, regNewPtr ) ;
		}
		else
		{
			CompileLoadStackObject( 1 ) ;				// pointer
			CompileCodeSwap( 0, 1 ) ;					// counter
		}
		m_pcsxi->FenceInstruction() ;
		//
		DWORD	dwLoopBegin = CompileCodeGetCurrent() ;
		//
		// ループ脱出判定
		//
		if ( m_modeNakedCode )
		{
			int	regCmp = AllocateExpressionRegister() ;
			m_pcsxi->WriteSakuraMoveRegReg( regCmp, regCounter ) ;
			m_pcsxi->WriteSakuraCmpGtRegReg
					( regCmp, ECSSakura2Processor::regIntZero ) ;
		}
		else
		{
			CompileLoadStackObject( 0 ) ;				// counter > 0 ?
			CompileImmediateInteger( 0 ) ;
			CompileCodeCompare( csctGreaterThan ) ;
		}
		DWORD	dwJumpBreakRef ;
		dwJumpBreakRef = CompileCodeConditionalJump( false, false ) ;
		//
		// メモリ初期化・構築関数呼び出し
		//
		if ( !m_modeNakedCode )
		{
			CompileLoadStackObject( 1 ) ;				// pointer
		}
		CompileCodeNakedClassInitialize( pClassInf ) ;
		//
		if ( IsDefaultConstructor( *pClassInf ) )
		{
			if ( m_modeNakedCode )
			{
				m_pcsxi->WriteSakuraMoveRegReg
					( AllocateExpressionRegister(), regNextPtr ) ;
			}
			else
			{
				CompileCodeReferenceForPointer( csvtObject ) ;
			}
			err = CompileCallDefaultConstructor( *pClassInf, false ) ;
			if ( err )
			{
				return	err ;
			}
		}
		else
		{
			if ( !m_modeNakedCode )
			{
				CompileCodeFreeStack() ;
			}
		}
		//
		// ループカウンタ更新
		//
		if ( m_modeNakedCode )
		{
			m_pcsxi->WriteSakuraAddRegRegImm32
						( regCounter, regCounter, -1 ) ;
		}
		else
		{
			CompileImmediateInteger( 1 ) ;
			CompileCodeOperate( csotSub ) ;
		}
		//
		// ポインタ更新
		//
		if ( m_modeNakedCode )
		{
			m_pcsxi->WriteSakuraAddRegRegImm32
				( regNextPtr, regNextPtr,
						pClassInf->GetNakedMemorySize() ) ;
		}
		else
		{
			CompileCodeSwap( 0, 1 ) ;
			//
			CompileImmediateInteger( pClassInf->GetNakedMemorySize() ) ;
			CompileCodeOperate( csotAdd ) ;
			//
			CompileCodeSwap( 0, 1 ) ;
		}
		//
		// 次のループへ
		//
		CompileCodeJump( dwLoopBegin ) ;
		//
		// ループ完了
		//
		CompileCodeCommitJumpAddress
				( dwJumpBreakRef, CompileCodeGetCurrent() ) ;
		//
		CompileCodeFreeStack() ;
		CompileCodeFreeStack() ;
	}
	else
	{
		CompileCodeFreeStack() ;
	}
	//
	typeinf.MakePointerOf( typeNakedType ) ;
	//
	if ( m_modeNakedCode )
	{
		ESLAssert( regNewPtr == GetExpressionRegister() ) ;
		typeinf.SetLoadedRegister( GetExpressionRegister() ) ;
	}
	return	eslErrSuccess ;
}

// 関数間接呼び出し
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileArgumentAndIndirectCallFunction
	( ECSSourceStream & cssLine,
		ECSTypeInfo & typeinf, ECSTypeInfo & typeFunc )
{
	CompileCodeNakedUncoverReference( typeFunc ) ;
	if ( !typeFunc.IsLoadedRegister() )
	{
		return	ESLErrorMsg
			( "関数の間接呼び出しで関数ポインタが見つかりません" ) ;
	}
	ECSFunction *	pFuncPtr = typeFunc.GetTypeFunctionPointer() ;
	if ( pFuncPtr == NULL )
	{
		return	ESLErrorMsg( "関数ポインタでない間接呼び出しです" ) ;
	}
	ESLAssert( pFuncPtr != NULL ) ;
	//
	if ( pFuncPtr->m_prototype.IsThisCall() )
	{
		if ( !typeFunc.IsLoadedThisCallRegister() )
		{
			return	ESLErrorMsg
				( "thiscall 関数の間接呼び出しで"
					"オブジェクトポインタが指定されていません" ) ;
		}
		int	regThisPtr = typeFunc.GetLoadedThisCallRegister() ;
		int	regFuncPtr = typeFunc.GetLoadedRegister() ;
		if ( (regThisPtr == (regFuncPtr + 1))
			&& (regThisPtr == GetExpressionRegister()) )
		{
		}
		else if ( (regFuncPtr == (regThisPtr + 1))
			&& (regFuncPtr == GetExpressionRegister()) )
		{
			int	regTemp = AllocateExpressionRegister() ;
			m_pcsxi->WriteSakuraMoveRegReg( regTemp, regThisPtr ) ;
			m_pcsxi->WriteSakuraMoveRegReg( regThisPtr, regFuncPtr ) ;
			m_pcsxi->WriteSakuraMoveRegReg( regFuncPtr, regTemp ) ;
			FreeExpressionRegister() ;
			//
			typeFunc.SetLoadedRegister( regThisPtr ) ;
		}
		else
		{
			return	ESLErrorMsg
				( "thiscall 関数の間接呼び出しでレジスタ割り当てが不正です" ) ;
		}
	}
	//
	ESLError	err ;
	int	nArgAddition = 0 ;
	err = CompileNakedFuncPrepareToReturnObject
					( &(pFuncPtr->m_prototype), nArgAddition ) ;
	if ( err )
	{
		return	err ;
	}
	if ( pFuncPtr->m_prototype.IsThisCall() )
	{
		nArgAddition ++ ;
	}
	EObjArray<ECSTypeInfo>	lstArgType ;
	err = CompileArgument
		( lstArgType, &(pFuncPtr->m_prototype), cssLine ) ;
	if ( err )
	{
		return	err ;
	}
	err = CompileCallIndirectFunction
		( typeFunc.GetLoadedRegister(),
			pFuncPtr->m_prototype, lstArgType.GetSize() + nArgAddition ) ;
	if ( err )
	{
		return	err ;
	}
	GetFunctionReturnType( typeinf, pFuncPtr->m_prototype ) ;
	return	eslErrSuccess ;
}

// 明示的オブジェクト指定の無い関数の呼び出し
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileArgumentAndCallFunction
	( ECSSourceStream & cssLine,
		ECSTypeInfo & typeinf, CSObjectMode csomClass,
		const EWideString & wstrNameSpace, const EWideString & wstrFuncName )
{
	//
	// 引数フォーマット解釈
	//
	ESLError						err ;
	ECSExecutionImageCompiler		csxiTemp ;
	ECSExecutionImageCompiler *		pcsxiLast = m_pcsxi ;
	ECSClassInfo::MemberFunction *	pFunc ;
	EObjArray<ECSTypeInfo>			lstArgType ;
	int								iArgBegin = cssLine.GetIndex() ;
	//
	NakedExpressionTemporary	tempExpr ;
	SaveNakedExpressionTemporary( tempExpr ) ;
	m_modeTemporary = true ;
	m_pcsxi = &csxiTemp ;
	//
	err = CompileArgument( lstArgType, NULL, cssLine ) ;
	//
	m_pcsxi = pcsxiLast ;
	RestoreNakedExpressionTemporary( tempExpr ) ;
	//
	if ( err )
	{
		return	err ;
	}
	cssLine.MoveIndex( iArgBegin ) ;
	//
	EWideString	wstrGlobalName = wstrNameSpace ;
	if ( !wstrGlobalName.IsEmpty() )
	{
		wstrGlobalName += L"::" ;
	}
	wstrGlobalName += wstrFuncName ;
	//
	if ( (csomClass == csomThis) || (csomClass == csomAuto) )
	do
	{
		//
		// this メンバ関数
		//////////////////////////////////////////////////////////////////////
		//
		// this 取得
		//
		const ECSClassInfo *	pThisClassInf = NULL ;
		ECSTypeInfo	typeThis ;
		long int	iThisVar = IsLocalVariableName( L"this", &typeThis ) ;
		bool		fStaticFunc = false ;
		if ( iThisVar >= 0 )
		{
			if ( m_modeNakedCode )
			{
				pThisClassInf = GetNakedPtrTypeClassInfo( typeThis ) ;
				//
				ECSTypeInfo	typeTempThis ;
				ECSTypeInfo	typeAddrThis ;
				typeAddrThis.MoveRegisterAndAddressingFrom( typeThis ) ;
				typeTempThis.MakeNakedPointerOf( typeThis ) ;
				typeThis.MakeReferenceOf( typeTempThis ) ;
				typeThis.MoveRegisterAndAddressingFrom( typeAddrThis ) ;
			}
			else
			{
				pThisClassInf = GetNakedTypeClassInfo( typeThis ) ;
			}
		}
		else if ( csomClass == csomThis )
		{
			return	ESLErrorMsg
				( "この関数には this オブジェクトは存在しません。" ) ;
		}
		else
		{
			pThisClassInf = GetCurrentThisClass() ;
			fStaticFunc = true ;
		}
		if ( pThisClassInf == NULL )
		{
			break ;
		}
		//
		// 親クラスキャスト判定
		//
		ECSTypeInfo				typeCallTarget = typeThis ;
		const ECSClassInfo *	pTargetClassInf = pThisClassInf ;
		const ECSClassInfo::CastInfo *	pCastInf = NULL ;
		if ( !wstrNameSpace.IsEmpty() /*&& !fStaticFunc*/ )
		{
			pCastInf = pThisClassInf->GetCastParentClassAs( wstrNameSpace ) ;
			if ( (pCastInf == NULL) || (pCastInf->pClassInf == NULL) )
			{
				break ;
			}
			ECSTypeInfo	typeTarget
				( new ECSStructure( pCastInf->pClassInf ), typeThis.m_dwFlags ) ;
			typeCallTarget.MakeReferenceOf( typeTarget ) ;
			pTargetClassInf = pCastInf->pClassInf ;
		}
		bool	fBreak = false ;
		for ( ; ; )
		{
			//
			// メンバ関数検索
			//
			ECSClassInfo::ListMemberFunction	lstFunc ;
			if ( !pTargetClassInf->SearchFunctinoAs
				( lstFunc, wstrFuncName, lstArgType,
					(typeCallTarget.m_dwFlags & ECSTypeInfo::flagConstant),
					false, false, m_modeNakedCode ) )
			{
				if ( lstFunc.GetSize() == 0 )
				{
					if ( csomClass == csomThis )
					{
						return	MakeNotFoundCallErrorMsg
							( "適合する "
								+ EString(pTargetClassInf->GetGlobalName()) + "::"
								+ EString(wstrFuncName)
								+ " 関数が見つかりません。", &lstArgType ) ;
					}
					fBreak = true ;
					break ;
				}
				else
				{
					MakeAmbiguousWarningMsg1
						( EString(wstrFuncName)
							+ " 関数の呼び出しが曖昧です。",
										&lstArgType, &lstFunc ) ;
				}
			}
			pFunc = lstFunc.GetAt( 0 ) ;
			if ( pFunc == NULL )
			{
				m_strErrMsg = "内部エラー：適合する "
					+ EString(pTargetClassInf->GetGlobalName()) + "::"
					+ EString(wstrFuncName) + " 関数が見つかりません。" ;
				return	ESLErrorMsg( m_strErrMsg ) ;
			}
			if ( pFunc->GetProtectedAttribute() > ECSTypeInfo::flagPrivate )
			{
				m_strErrMsg =
					EString(pTargetClassInf->GetGlobalName()) + "::"
					+ EString(wstrFuncName) + " 関数は private な関数です。" ;
				return	ESLErrorMsg( m_strErrMsg ) ;
			}
			//
			// 暗黙キャスト判定
			//
			if ( !fStaticFunc
				&& (pFunc->m_pClassCast->pClassInf->GetAttribute()
										& ECSTypeInfo::flagNativeObject)
				&& !(pTargetClassInf->GetAttribute()
										& ECSTypeInfo::flagNativeObject) )
			{
				pCastInf = pFunc->m_pClassCast ;
				pTargetClassInf = pCastInf->pClassInf ;
				//
				ECSTypeInfo	typeTarget
					( new ECSStructure( pTargetClassInf ), typeThis.m_dwFlags ) ;
				typeCallTarget.MakeReferenceOf( typeTarget ) ;
				//
				continue ;
			}
			break ;
		}
		if ( fBreak )
		{
			break ;
		}
		//
		// メンバ関数呼び出し
		//
		if ( pFunc->GetAttribute() & ECSTypeInfo::flagStatic )
		{
			//
			// static なメンバ関数
			//
			int	nArgAddition = 0 ;
			err = CompileNakedFuncPrepareToReturnObject
									( pFunc, nArgAddition ) ;
			if ( err )
			{
				return	err ;
			}
			lstArgType.RemoveAll() ;
			err = CompileArgument( lstArgType, pFunc, cssLine ) ;
			if ( err )
			{
				return	err ;
			}
			err = CompileCallGlobalFunction
					( *pFunc, lstArgType, lstArgType.GetSize() + nArgAddition ) ;
			if ( err )
			{
				return	err ;
			}
			GetFunctionReturnType( typeinf, *pFunc ) ;
			return	eslErrSuccess ;
		}
		if ( fStaticFunc )
		{
			break ;
		}
		//
		// this ポインタロード
		//
		if ( m_modeNakedCode )
		{
			m_pcsxi->WriteSakuraMoveRegReg
				( AllocateExpressionRegister(),
					ECSSakura2Processor::regTP ) ;
			typeThis.SetLoadedRegister( GetExpressionRegister() ) ;
			typeThis.SetAddressingInfo( GetExpressionRegister(), 0 ) ;
		}
		else
		{
			CompileCodeLoadRefVariable( csomStack, iThisVar ) ;
		}
		if ( pTargetClassInf->GetGlobalName() != pThisClassInf->GetGlobalName() )
		{
			//
			// 親クラスへのキャスト
			//
			ECSTypeInfo	typeAddressing ;
			err = CompileCastToParentClass
				( typeAddressing, *pCastInf,
					*pThisClassInf, typeThis, ECSTypeInfo::flagPrivate ) ;
			if ( err )
			{
				return	err ;
			}
			typeThis.MoveRegisterAndAddressingFrom( typeAddressing ) ;
			MakeCommitValueToNakedRegister( typeThis ) ;
		}
		//
		// naked モードで関数返り値オブジェクト領域準備
		//
		int	nArgAddition = 0 ;
		err = CompileNakedFuncPrepareToReturnObject( pFunc, nArgAddition ) ;
		if ( err )
		{
			return	err ;
		}
		//
		// 引数ロード
		//
		lstArgType.RemoveAll() ;
		err = CompileArgument( lstArgType, pFunc, cssLine ) ;
		if ( err )
		{
			return	err ;
		}
		//
		// メンバ関数呼び出し
		//
		err = CompileCallMemberFunction
			( *pTargetClassInf, typeThis,
					*pFunc, lstArgType,
					lstArgType.GetSize() + nArgAddition,
					ECSTypeInfo::flagPrivate, !wstrNameSpace.IsEmpty() ) ;
		if ( err )
		{
			return	err ;
		}
		GetFunctionReturnType( typeinf, *pFunc ) ;
		return	eslErrSuccess ;
	}
	while ( false ) ;
	//
	// static なクラスメンバ関数
	//////////////////////////////////////////////////////////////////////
	if ( !wstrNameSpace.IsEmpty() && (csomClass == csomAuto) )
	{
		const ECSClassInfo *
			pClassInf = GetClassInfoAs( wstrNameSpace ) ;
		if ( pClassInf == NULL )
		{
			m_strErrMsg =
				EString( wstrNameSpace ) + " クラスが見つかりません。" ;
			return	ESLErrorMsg( m_strErrMsg ) ;
		}
		ECSClassInfo::ListMemberFunction	lstFunc ;
		if ( !pClassInf->SearchFunctinoAs
				( lstFunc, wstrFuncName, lstArgType,
					ECSTypeInfo::flagStatic, false, false, m_modeNakedCode ) )
		{
			if ( lstFunc.GetSize() == 0 )
			{
				return	MakeNotFoundCallErrorMsg
					( "適合する "
						+ EString(pClassInf->GetGlobalName()) + "::"
						+ EString(wstrFuncName)
						+ " 関数が見つかりません。", &lstArgType ) ;
			}
			else
			{
				return	MakeAmbiguousErrorMsg
					( EString(wstrFuncName)
						+ " 関数の呼び出しが曖昧です。",
								&lstArgType, &lstFunc ) ;
			}
		}
		pFunc = lstFunc.GetAt( 0 ) ;
		if ( pFunc == NULL )
		{
			m_strErrMsg = "内部エラー：適合する "
				+ EString(pClassInf->GetGlobalName()) + "::"
				+ EString(wstrFuncName) + " 関数が見つかりません。" ;
			return	ESLErrorMsg( m_strErrMsg ) ;
		}
		//
		// メンバ関数呼び出し
		//
		if ( !(pFunc->GetAttribute() & ECSTypeInfo::flagStatic) )
		{
			return	ESLErrorMsg
				( "this ポインタのないメンバ関数の呼び出しです。" ) ;
		}
		//
		// static なメンバ関数
		//
		int	nArgAddition = 0 ;
		err = CompileNakedFuncPrepareToReturnObject( pFunc, nArgAddition ) ;
		if ( err )
		{
			return	err ;
		}
		lstArgType.RemoveAll() ;
		err = CompileArgument( lstArgType, pFunc, cssLine ) ;
		if ( err )
		{
			return	err ;
		}
		err = CompileCallGlobalFunction
				( *pFunc, lstArgType, lstArgType.GetSize() + nArgAddition ) ;
		if ( err )
		{
			return	err ;
		}
		GetFunctionReturnType( typeinf, *pFunc ) ;
		return	eslErrSuccess ;
	}
	//
	// グローバル関数・又はクラスメンバの static 関数
	//////////////////////////////////////////////////////////////////////
	ECSPrototypeInfo * pPrototype =
		SearchGlobalFunctionAs( wstrGlobalName, (csomClass == csomGlobal) ) ;
	if ( pPrototype != NULL )
	{
		ECSTypeInfo::TypeMatchResult
			matchResult = pPrototype->IsMatchArgument( lstArgType ) ;
		if ( matchResult != ECSTypeInfo::typeMatch )
		{
			if ( (matchResult == ECSTypeInfo::typeNoMatch)
				/* || (m_dwModeFlags & flagStrictStyle) */ )
			{
				return	MakeNotFoundCallErrorMsg
					( EString( wstrGlobalName )
							+ " 関数の引数が適合しません", &lstArgType ) ;
			}
		}
		int	nArgAddition = 0 ;
		err = CompileNakedFuncPrepareToReturnObject
							( pPrototype, nArgAddition ) ;
		if ( err )
		{
			return	err ;
		}
		lstArgType.RemoveAll() ;
		err = CompileArgument( lstArgType, pPrototype, cssLine ) ;
		if ( err )
		{
			return	err ;
		}
		err = CompileCallGlobalFunction
				( *pPrototype, lstArgType, lstArgType.GetSize() + nArgAddition ) ;
		if ( err )
		{
			return	err ;
		}
		GetFunctionReturnType( typeinf, *pPrototype ) ;
		return	eslErrSuccess ;
	}
	//
	// 動的解釈
	//////////////////////////////////////////////////////////////////////////
	if ( m_dwModeFlags & flagStrictStyle )
	{
		m_strErrMsg = EString( wstrFuncName )
				+ " 関数は定義されていない関数です。" ;
		return	ESLErrorMsg( m_strErrMsg ) ;
	}
	if ( (csomClass != csomThis)
		&& (csomClass != csomAuto) && (csomClass != csomGlobal) )
	{
		return	ESLErrorMsg
			( "関数の呼び出しが不正な記憶クラスを伴っています。" ) ;
	}
	if ( m_modeNakedCode )
	{
		return	ESLErrorMsg
			( "naked モードでは関数呼び出しを動的に解決できません" ) ;
	}
	lstArgType.RemoveAll() ;
	err = CompileArgument( lstArgType, NULL, cssLine ) ;
	if ( err )
	{
		return	err ;
	}
	int	nArgCount = lstArgType.GetSize() ;
	if ( csomClass == csomThis )
	{
		nArgCount ++ ;
	}
	//
	m_pcsxi->WriteInstructionCode( csicCall ) ;
	m_pcsxi->WriteObjectModeCode( csomClass ) ;
	m_pcsxi->WriteCodeData( &nArgCount, sizeof(nArgCount) ) ;
	m_pcsxi->WriteConstantString( wstrGlobalName ) ;
	//
	typeinf = ECSTypeInfo( new ECSReference ) ;
	return	eslErrSuccess ;
}

// 引数を解釈して適合するクラスメンバ関数呼び出し
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileArgumentAndCallMemberFunction
	( ECSSourceStream & cssLine,
		const ECSClassInfo & clsinfThis,
		const ECSTypeInfo & typeThis,
		const wchar_t * pwszFuncName, bool fLoadThisRef,
		const ECSClassInfo::MemberFunction *& pFunc,
		ECSTypeInfo::Flags flagScope )
{
	if ( typeThis.m_dwFlags & ECSTypeInfo::flagNakedBuffer )
	{
		if ( typeThis.IsTypeInteger() )
		{
			return	ESLErrorMsg
				( "naked int へメンバ関数を呼び出そうとしています" ) ;
		}
		else if ( typeThis.IsTypeReal() )
		{
			return	ESLErrorMsg
				( "naked double へメンバ関数を呼び出そうとしています" ) ;
		}
	}
	if ( (DWORD) flagScope
			< (typeThis.m_dwFlags & ECSTypeInfo::flagProtectedMask) )
	{
		flagScope = (ECSTypeInfo::Flags)
			(typeThis.m_dwFlags & ECSTypeInfo::flagProtectedMask) ;
	}
	const DWORD	dwAccessScope = GetAccessClassTo( &clsinfThis ) ;
	if ( dwAccessScope > (DWORD) flagScope )
	{
		flagScope = (ECSTypeInfo::Flags) dwAccessScope ;
	}
	//
	// メンバ関数の引数型取得
	//
	ESLError					err ;
	ECSExecutionImageCompiler	csxiTemp ;
	ECSExecutionImageCompiler *	pcsxiLast = m_pcsxi ;
	EObjArray<ECSTypeInfo>		lstArgType ;
	int							iArgBegin = cssLine.GetIndex() ;
	//
	NakedExpressionTemporary	tempExpr ;
	SaveNakedExpressionTemporary( tempExpr ) ;
	m_modeTemporary = true ;
	m_pcsxi = &csxiTemp ;
	//
	err = CompileArgument( lstArgType, NULL, cssLine ) ;
	//
	m_pcsxi = pcsxiLast ;
	RestoreNakedExpressionTemporary( tempExpr ) ;
	if ( err )
	{
		return	err ;
	}
	cssLine.MoveIndex( iArgBegin ) ;
	//
	// メンバ関数検索
	//
	ECSClassInfo::ListMemberFunction	lstFunc ;
	if ( !clsinfThis.SearchFunctinoAs
			( lstFunc, pwszFuncName, lstArgType,
				(typeThis.m_dwFlags & ECSTypeInfo::flagConstant),
				false, false, m_modeNakedCode ) )
	{
		if ( (lstFunc.GetSize() > 0)
			&& (clsinfThis.GetAttribute() & ECSTypeInfo::flagNativeObject) )
		{
			if ( m_dwModeFlags & flagStrictStyle )
			{
				err = OutputWarning1
					( "複数の適合する関数があります。",
								m_strFilePath, m_nLineNum ) ;
				if ( err )
				{
					return	err ;
				}
			}
		}
		else if ( !(m_dwModeFlags & flagStrictStyle) && !m_modeNakedCode )
		{
			//
			// ルーズな関数呼び出し
			//
			if ( fLoadThisRef )
			{
				CompileLoadStackObject( 0 ) ;
			}
			lstArgType.RemoveAll() ;
			err = CompileArgument( lstArgType, NULL, cssLine ) ;
			if ( err )
			{
				return	err ;
			}
			int	nArgCount = lstArgType.GetSize() ;
			nArgCount ++ ;
			m_pcsxi->WriteInstructionCode( csicCall ) ;
			m_pcsxi->WriteObjectModeCode( csomThis ) ;
			m_pcsxi->WriteCodeData( &nArgCount, sizeof(nArgCount) ) ;
			m_pcsxi->WriteConstantString( pwszFuncName ) ;
			//
			pFunc = NULL ;
			//
			return	eslErrSuccess ;
		}
		else if ( lstFunc.GetSize() > 0 )
		{
			MakeAmbiguousWarningMsg1
				( "複数の適合する "
					+ EString(pwszFuncName) + " 関数があります。",
											&lstArgType, &lstFunc ) ;
		}
		else
		{
			ECSTypeInfo *	pArgType = lstArgType.GetAt(0) ;
			if ( m_modeNakedCode
				&& clsinfThis.IsNakedMemoryClass()
				&& (clsinfThis.GetName() == pwszFuncName)
				&& (clsinfThis.GetVirtualFunctionCount() == 0)
				&& (lstArgType.GetSize() == 1)
				&& (pArgType != NULL) )
			{
				const ECSClassInfo *
					pSrcClassInf = GetNakedPtrTypeClassInfo( *pArgType ) ;
				if ( (pSrcClassInf != NULL)
					&& (clsinfThis.GetGlobalName() == pSrcClassInf->GetGlobalName()) )
				{
					ECSPrototypeInfo	protoTemp ;
					ECSTypeInfo *	pTypeRef = new ECSTypeInfo ;
					pTypeRef->MakeReferenceOf
						( ECSTypeInfo( new ECSStructure(pSrcClassInf), ECSTypeInfo::flagConstant ) ) ;
					protoTemp.AddArgument( pTypeRef, L"@" ) ;
					//
					lstArgType.RemoveAll() ;
					err = CompileArgument( lstArgType, &protoTemp, cssLine ) ;
					if ( err )
					{
						return	err ;
					}
					//
					m_pcsxi->WriteSakuraLoadInt64
						( AllocateExpressionRegister(),
								clsinfThis.GetNakedMemorySize() ) ;
					CompileNakedSystemCall( L"memmove", 3 ) ;
					pFunc = NULL ;
					return	eslErrSuccess ;
				}
			}
			return	MakeNotFoundCallErrorMsg
				( "適合する "
					+ EString(clsinfThis.GetGlobalName()) + "::"
					+ EString(pwszFuncName)
					+ " 関数が見つかりません。", &lstArgType ) ;
		}
	}
	pFunc = lstFunc.GetAt( 0 ) ;
	if ( pFunc == NULL )
	{
		m_strErrMsg = "内部エラー：適合する "
			+ EString(clsinfThis.GetGlobalName()) + "::"
			+ EString(pwszFuncName) + " 関数が見つかりません。" ;
		return	ESLErrorMsg( m_strErrMsg ) ;
	}
	//
	// メンバ関数の引数
	//
	if ( fLoadThisRef )
	{
		CompileLoadStackObject( 0 ) ;
	}
	int	nArgAddition = 0 ;
	err = CompileNakedFuncPrepareToReturnObject( pFunc, nArgAddition ) ;
	if ( err )
	{
		return	err ;
	}
	lstArgType.RemoveAll() ;
	err = CompileArgument( lstArgType, pFunc, cssLine ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// メンバ関数の呼び出し
	//
	err = CompileCallMemberFunction
		( clsinfThis, typeThis, *pFunc, lstArgType,
			lstArgType.GetSize() + nArgAddition, flagScope ) ;
	if ( err )
	{
		return	err ;
	}
	return	eslErrSuccess ;
}

// 関数返り値型取得
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::GetFunctionReturnType
	( ECSTypeInfo & typeinf, const ECSPrototypeInfo & proto )
{
	const ECSTypeInfo &	typeReturn = proto.GetReturnType() ;
	if ( m_modeNakedCode && !typeReturn.IsVoid() )
	{
		if ( proto.IsNakedCall() )
		{
			typeinf = proto.GetReturnType() ;
		}
		else
		{
			ECSObject *	pType = typeReturn.GetNakedType() ;
			if ( typeReturn.IsTypeReference()
				|| !ECSTypeInfo::IsNakedPrimitiveDataType( pType ) )
			{
				if ( typeReturn.IsTypeReference()
					&& ECSTypeInfo::IsNakedPrimitiveDataType( pType ) )
				{
					OutputError
						( "naked モードから object 関数呼び出しで、"
							"返り値が正常に処理できる型ではありません",
											m_strFilePath, m_nLineNum ) ;
				}
				ECSTypeInfo	typeTemp
					( ECSTypeInfo::DuplicateType( pType ),
									typeReturn.m_dwFlags ) ;
				typeinf.MakePointerOf( typeTemp ) ;
			}
			else
			{
				if ( typeReturn.IsTypePointer()
					&& !(typeReturn.m_dwFlags & ECSTypeInfo::flagNakedBuffer) )
				{
					if ( typeReturn.GetObjectClassInfo( *this ) != NULL )
					{
						OutputWarning3
							( "naked モードから object 関数呼び出しで、"
								"返り値が naked ポインタではありません",
											m_strFilePath, m_nLineNum ) ;
					}
					else
					{
						OutputError
							( "naked モードから object 関数呼び出しで、"
								"返り値が naked クラスポインタは"
								"正常に処理できる型ではありません",
											m_strFilePath, m_nLineNum ) ;
					}
				}
				typeinf = proto.GetReturnType() ;
			}
		}
		typeinf.SetLoadedRegister( GetExpressionRegister() ) ;
	}
	else
	{
		typeinf = proto.GetReturnType() ;
	}
}

// 型情報検索
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::SearchTypeName
	( SYMBOL_NAMESPACE& snsSymbol,
			/* in wstrName, wstrNamespace
				/ out wstrFullName, wstrName, wstrNamespace */
		ECSTypeInfo::Flags flagScope )
{
	if ( !snsSymbol.wstrNamespace.IsEmpty() )
	{
		snsSymbol.wstrFullName =
			snsSymbol.wstrNamespace + L"::" + snsSymbol.wstrName ;
		if ( IsTypeName( snsSymbol.wstrFullName ) >= 0 )
		{
			return	eslErrSuccess ;
		}
		//
		// 親クラス名前空間判定
		//
		ECSClassInfo *
			pClassInf = GetClassInfoAs( snsSymbol.wstrNamespace ) ;
		if ( pClassInf != NULL )
		{
			unsigned int	i, nCount ;
			nCount = pClassInf->GetParentClassCount() ;
			for ( i = 0; i < nCount; i ++ )
			{
				ECSClassInfo::ParentClass *
					pParentClass = pClassInf->GetParentClassAt( i ) ;
				if ( pParentClass == NULL )
				{
					continue ;
				}
				if ( (pParentClass->dwFlags
						& ECSTypeInfo::flagProtectedMask)
					> (DWORD) (flagScope & ECSTypeInfo::flagProtectedMask) )
				{
					continue ;
				}
				SYMBOL_NAMESPACE	snsTemp = snsSymbol ;
				snsTemp.wstrNamespace =
					pParentClass->pClassInf->GetGlobalName() ;
				snsTemp.wstrFullName =
					snsTemp.wstrNamespace + L"::" + snsSymbol.wstrName ;
				//
				ESLError	err = SearchTypeName( snsTemp, flagScope ) ;
				if ( !err )
				{
					snsSymbol = snsTemp ;
					return	eslErrSuccess ;
				}
			}
		}
	}
	ECSObject *	pObj = ParseBsaicType( snsSymbol.wstrName ) ;
	if ( pObj != NULL )
	{
		delete	pObj ;
		snsSymbol.wstrNamespace = L"" ;
		snsSymbol.wstrFullName = snsSymbol.wstrName ;
		return	eslErrSuccess ;
	}
	if ( IsTypeName( snsSymbol.wstrName ) >= 0 )
	{
		snsSymbol.wstrNamespace = L"" ;
		snsSymbol.wstrFullName = snsSymbol.wstrName ;
		return	eslErrSuccess ;
	}
	int	i, nCount = m_nestCtrl.GetSize() ;
	for ( i = 0; i < nCount; i ++ )
	{
		EControlNest *	pNest = m_nestCtrl.GetLastAt( i ) ;
		if ( pNest == NULL )
		{
			continue ;
		}
		ECSTypeInfo *	pTypeDef =
			pNest->m_wstaTypeDef.GetAs( snsSymbol.wstrFullName ) ;
		if ( pTypeDef != NULL )
		{
			return	eslErrSuccess ;
		}
		pTypeDef = pNest->m_wstaTypeDef.GetAs( snsSymbol.wstrName ) ;
		if ( pTypeDef != NULL )
		{
			snsSymbol.wstrNamespace = L"" ;
			snsSymbol.wstrFullName = snsSymbol.wstrName ;
			return	eslErrSuccess ;
		}
		if ( pNest->m_rwType == rwTemplate )
		{
			break ;
		}
	}
	return	eslErrGeneral ;
}

// 適合関数が見つからないエラーメッセージ生成
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::MakeNotFoundCallErrorMsg
	( const char * pszErrMsg,
		EObjArray<ECSTypeInfo> * pCallParam )
{
	m_strErrMsg = pszErrMsg ;
	m_strErrMsg += "\n\t呼び出し引数:(" ;
	//
	unsigned int	i ;
	for ( i = 0; i < pCallParam->GetSize(); i ++ )
	{
		EWideString	wstrArg ;
		pCallParam->GetAt(i)->FormatTypeString( wstrArg ) ;
		if ( i > 0 )
		{
			m_strErrMsg += "," ;
		}
		m_strErrMsg += EString( wstrArg ) ;
	}
	m_strErrMsg += ")" ;
	//
	return	ESLErrorMsg( m_strErrMsg ) ;
}

// 曖昧な関数呼び出しエラーメッセージ生成
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::MakeAmbiguousErrorMsg
	( const char * pszErrMsg,
		EPtrObjArray<ECSTypeInfo> * pCallParam,
		ECSClassInfo::ListMemberFunction * pFuncList )
{
	m_strErrMsg = pszErrMsg ;
	m_strErrMsg += "\n\t呼び出し引数:(" ;
	//
	unsigned int	i ;
	for ( i = 0; i < pCallParam->GetSize(); i ++ )
	{
		EWideString	wstrArg ;
		pCallParam->GetAt(i)->FormatTypeString( wstrArg ) ;
		if ( i > 0 )
		{
			m_strErrMsg += "," ;
		}
		m_strErrMsg += EString( wstrArg ) ;
	}
	m_strErrMsg += ")\n" ;
	//
	for ( i = 0; i < pFuncList->GetSize(); i ++ )
	{
		m_strErrMsg += "\t候補" + EString((int)i) + ":"
					+ EString(pFuncList->GetAt(i)->FormatPrototype()) + "\n" ;
	}
	return	ESLErrorMsg( m_strErrMsg ) ;
}

// 曖昧な関数呼び出し警告メッセージ生成
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::MakeAmbiguousWarningMsg1
	( const char * pszErrMsg,
		EPtrObjArray<ECSTypeInfo> * pCallParam,
		ECSClassInfo::ListMemberFunction * pFuncList )
{
	MakeAmbiguousErrorMsg( pszErrMsg, pCallParam, pFuncList ) ;
	//
	return	OutputWarning1( m_strErrMsg, m_strFilePath, m_nLineNum ) ;
}

ESLError ECSCompiler::MakeAmbiguousWarningMsg2
	( const char * pszErrMsg,
		EPtrObjArray<ECSTypeInfo> * pCallParam,
		ECSClassInfo::ListMemberFunction * pFuncList )
{
	MakeAmbiguousErrorMsg( pszErrMsg, pCallParam, pFuncList ) ;
	//
	return	OutputWarning2( m_strErrMsg, m_strFilePath, m_nLineNum ) ;
}

// 型二項演算
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileTypeOperate
	( ECSTypeInfo & typeDst, const ECSTypeInfo & typeSrc1,
		const ECSTypeInfo & typeSrc2, CSOperatorType optOperator )
{
	const ECSClassInfo *			pClassInf = NULL ;
	ECSClassInfo::MemberFunction *	pFunc = NULL ;
	//
	if ( typeSrc1.IsVoid() || typeSrc2.IsVoid() )
	{
		return	ESLErrorMsg( "void への不正な二項演算子です。" ) ;
	}
	if ( typeSrc1.IsTypePointer() )
	{
		return	CompileTypePointerOperate
					( typeDst, typeSrc1, typeSrc2, optOperator ) ;
	}
	pClassInf = GetNakedTypeClassInfo( typeSrc1 ) ;
	if ( pClassInf != NULL )
	{
		ECSClassInfo::ListMemberFunction	lstFunc ;
		EObjArray<ECSTypeInfo>				lstArg ;
		lstArg.Add( new ECSTypeInfo( typeSrc2 ) ) ;
		//
		bool	fOperator =
			pClassInf->SearchOperatorAs
				( lstFunc, optOperator, lstArg, m_modeNakedCode ) ;
		if ( !fOperator )
		{
			if ( lstFunc.GetSize() >= 1 )
			{
				if ( m_dwModeFlags & flagStrictStyle )
				{
					MakeAmbiguousWarningMsg2
						( "複数の適合する演算子オーバーロードが存在します",
														&lstArg, &lstFunc ) ;
				}
				fOperator = true ;
			}
		}
		if ( fOperator )
		{
			pFunc = lstFunc.GetAt( 0 ) ;
			typeDst = pFunc->GetReturnType() ;
			//
			if ( pClassInf->GetAttribute()
						& ECSTypeInfo::flagNativeObject )
			{
				if ( m_modeNakedCode )
				{
					return	CompileTypeNakedOperate
						( typeDst, typeSrc1, typeSrc2, optOperator ) ;
				}
				CompileCodeOperate( optOperator ) ;
				return	eslErrSuccess ;
			}
			ESLError	err ;
			DWORD		dwArgCount = 1 ;
			ECSTypeInfo	typeThis = typeSrc1 ;
			if ( m_modeNakedCode )
			{
				/*
				LoadCommitValueToNakedRegister
					( AllocateExpressionRegister(), typeSrc1 ) ;
				LoadCommitValueToNakedRegister
					( AllocateExpressionRegister(), typeSrc2 ) ;
				*/
				EObjArray<ECSTypeInfo>	lstArgType ;
				lstArgType[0] = typeSrc1 ;
				lstArgType[1] = typeSrc2 ;
				//
				err = NormalizeNakedArgument
						( pFunc, lstArgType, dwArgCount ) ;
				if ( err )
				{
					return	err ;
				}
				typeThis = lstArgType[0] ;
			}
			//
			err = CompileCallMemberFunction
					( *pClassInf, typeThis, *pFunc,
						lstArg, dwArgCount, ECSTypeInfo::flagPublic ) ;
			int	regReturn = GetExpressionRegister() ;
			//
			FreeExpressionRegister( typeSrc2 ) ;
			FreeExpressionRegister( typeSrc1 ) ;
			//
			if ( !typeDst.IsVoid() && m_modeNakedCode )
			{
				if ( regReturn != GetExpressionRegister() )
				{
					m_pcsxi->WriteSakuraMoveRegReg
							( GetExpressionRegister(), regReturn ) ;
				}
				GetFunctionReturnType( typeDst, *pFunc ) ;
			}
			return	err ;
		}
		else if ( m_dwModeFlags & flagStrictStyle )
		{
			return	ESLErrorMsg
				( "適合する演算子オーバーロードが見つかりませんでした。" ) ;
		}
	}
	if ( m_modeNakedCode )
	{
		return	ESLErrorMsg
			( "naked モードで object の操作でクラス情報がありません" ) ;
	}
	CompileCodeOperate( optOperator ) ;
	//
	typeDst = typeSrc1 ;
	//
	if ( (typeSrc1.m_pValue->m_vtType == csvtInteger)
			&& (typeSrc2.m_pValue->m_vtType == csvtReal) )
	{
		typeDst = ECSTypeInfo( new ECSReal ) ;
	}
	//
	return	eslErrSuccess ;
}

ESLError ECSCompiler::CompileTypeNakedOperate
	( ECSTypeInfo & typeDst, const ECSTypeInfo & typeSrc1,
		const ECSTypeInfo & typeSrc2, CSOperatorType optOperator )
{
	//
	// メモリ参照を展開
	//
	ESLAssert( m_modeNakedCode ) ;
	ECSTypeInfo	typeSrc1Temp = typeSrc1 ;
	ECSTypeInfo	typeSrc2Temp = typeSrc2 ;
	while ( typeSrc1Temp.IsTypeReference() )
	{
		CompileCodeNakedUncoverReference( typeSrc1Temp ) ;
	}
	while ( typeSrc2Temp.IsTypeReference() )
	{
		CompileCodeNakedUncoverReference( typeSrc2Temp ) ;
	}
	CompileCodeNakedUncoverEnumerator( typeSrc1Temp ) ;
	CompileCodeNakedUncoverEnumerator( typeSrc2Temp ) ;
	//
	if ( (!typeSrc1Temp.IsTypeInteger() && !typeSrc1Temp.IsTypeReal())
		|| (!typeSrc2Temp.IsTypeInteger() && !typeSrc2Temp.IsTypeReal()) )
	{
		return	ESLErrorMsg
			( "naked モードで整数でも実数でもない演算は定義されません" ) ;
	}
	//
	// 一方が実数型の場合には両者を実数に変換
	//
	bool	fFloat = false ;
	if ( typeSrc1Temp.IsTypeReal()
		|| typeSrc2Temp.IsTypeReal() )
	{
		CompileCodeNakedConvertToReal( typeSrc1Temp ) ;
		CompileCodeNakedConvertToReal( typeSrc2Temp ) ;
		fFloat = true ;
	}
	int	regSrc1 = typeSrc1Temp.IsLoadedRegister()
						? typeSrc1Temp.GetLoadedRegister() : -1 ;
	int	regSrc2 = typeSrc2Temp.IsLoadedRegister()
						? typeSrc2Temp.GetLoadedRegister() : -1 ;
	if ( (regSrc1 < 0) && (regSrc2 < 0) )
	{
		//
		// 定数計算
		//
		ESLAssert( typeSrc1Temp.m_pValue != NULL ) ;
		ESLAssert( typeSrc2Temp.m_pValue != NULL ) ;
		if ( !fFloat )
		{
			ESLAssert( typeSrc1Temp.m_pValue->m_vtType == csvtInteger ) ;
			ECSInteger *	pSrcInt1 = (ECSInteger*) typeSrc1Temp.m_pValue ;
			pSrcInt1->SetValueMask( ECSInteger::m_maskInt64 ) ;
			ESLError	err =
				pSrcInt1->Operate
					( m_ctxExpr, optOperator, *(typeSrc2Temp.m_pValue) ) ;
			if ( err )
			{
				return	err ;
			}
		}
		else
		{
			ESLAssert( typeSrc1Temp.m_pValue->m_vtType == csvtReal ) ;
			ECSReal *	pSrcReal1 = (ECSReal*) typeSrc1Temp.m_pValue ;
			ESLError	err =
				pSrcReal1->Operate
					( m_ctxExpr, optOperator, *(typeSrc2Temp.m_pValue) ) ;
			if ( err )
			{
				return	err ;
			}
		}
		ECSObject *	pResult = typeSrc1Temp.m_pValue ;
		if ( pResult->m_pResult != NULL )
		{
			pResult = pResult->m_pResult ;
		}
		typeDst.SetTypeValue
			( ECSTypeInfo::DuplicateType( pResult ),
						ECSTypeInfo::flagDeterministic ) ;
		typeDst.NormalzieImmediateIntegerType() ;
		return	eslErrSuccess ;
	}
	else if ( !fFloat && ((regSrc1 < 0) || (regSrc2 < 0)) )
	{
		//
		// 整数且つ、一方が定数値の場合の最適化
		//
		if ( (optOperator == csotAdd) || (optOperator == csotMul) )
		{
			//
			// reg * imm32 or imm32 * reg
			//
			const int	regLoaded = (regSrc1 >= 0) ? regSrc1 : regSrc2 ;
			const INT64	nValue = (regSrc1 < 0)
								? typeSrc1Temp.GetValueInteger()
									: typeSrc2Temp.GetValueInteger() ;
			ESLAssert( regLoaded >= 0 ) ;
			if ( (nValue >= - (INT64) 0x80000000)
							&& (nValue <= 0x7FFFFFFF) )
			{
				if ( optOperator == csotAdd )
				{
					m_pcsxi->WriteSakuraAddRegRegImm32
							( regLoaded, regLoaded, (int) nValue ) ;
				}
				else
				{
					m_pcsxi->WriteSakuraMulRegRegImm32
							( regLoaded, regLoaded, (int) nValue ) ;
				}
				typeDst = typeSrc1Temp ;
				typeDst.SetLoadedRegister( regLoaded ) ;
				return	eslErrSuccess ;
			}
			else if ( optOperator == csotMul )
			{
				int	nScale = GetNumberScale( nValue ) ;
				if ( nScale >= 0 )
				{
					m_pcsxi->WriteSakuraSllRegRegImm8
							( regLoaded, regLoaded, nScale ) ;
					typeDst = typeSrc1Temp ;
					typeDst.SetLoadedRegister( regLoaded ) ;
					return	eslErrSuccess ;
				}
			}
		}
		else if ( (regSrc2 < 0) && (optOperator == csotDiv) )
		{
			//
			// reg / imm64 --> reg >> imm8
			//
			const int	regLoaded = regSrc1 ;
			const INT64	nValue = typeSrc2Temp.GetValueInteger() ;
			ESLAssert( regLoaded >= 0 ) ;
			int	nScale = GetNumberScale( nValue ) ;
			if ( nScale >= 0 )
			{
				ECSInteger *	pSrcInt1 =
						(ECSInteger*) typeSrc1Temp.m_pValue ;
				ESLAssert( pSrcInt1->m_vtType == csvtInteger ) ;
				if ( pSrcInt1->IsSign() )
				{
					int	regTemp = AllocateExpressionRegister() ;
					ESLAssert( regTemp > regLoaded ) ;
					//
					m_pcsxi->WriteSakuraSraRegRegImm8( regTemp, regLoaded, 63 ) ;
					m_pcsxi->WriteSakuraSrlRegRegImm8( regTemp, regTemp, 64 - nScale ) ;
					m_pcsxi->WriteSakuraAddRegReg( regLoaded, regTemp ) ;
					//
					FreeExpressionRegister() ;
				}
				m_pcsxi->WriteSakuraSraRegRegImm8( regLoaded, regLoaded, nScale ) ;
				//
				typeDst = typeSrc1Temp ;
				typeDst.SetLoadedRegister( regLoaded ) ;
				return	eslErrSuccess ;
			}
		}
		else if ( (regSrc2 < 0) && (optOperator == csotMod) )
		{
			//
			// reg % imm64 --> reg & imm64
			//
			const int	regLoaded = regSrc1 ;
			const INT64	nValue = typeSrc2Temp.GetValueInteger() ;
			ESLAssert( regLoaded >= 0 ) ;
			int	nScale = GetNumberScale( nValue ) ;
			if ( nScale >= 0 )
			{
				ECSInteger *	pSrcInt1 =
						(ECSInteger*) typeSrc1Temp.m_pValue ;
				ESLAssert( pSrcInt1->m_vtType == csvtInteger ) ;
				if ( !pSrcInt1->IsSign() )
				{
					int	regTemp = AllocateExpressionRegister() ;
					ESLAssert( regTemp > regLoaded ) ;
					//
					m_pcsxi->WriteSakuraSrlRegRegImm8
						( regTemp,
							ECSSakura2Processor::regFillBit, 64 - nScale ) ;
					m_pcsxi->WriteSakuraAndRegReg( regLoaded, regTemp ) ;
					//
					FreeExpressionRegister() ;
				}
				else
				{
					int	regTempSign = AllocateExpressionRegister() ;
					ESLAssert( regTempSign > regLoaded ) ;
					int	regTempMask = AllocateExpressionRegister() ;
					ESLAssert( regTempMask > regLoaded ) ;
					//
					m_pcsxi->WriteSakuraSraRegRegImm8( regTempSign, regLoaded, 63 ) ;
					m_pcsxi->WriteSakuraSrlRegRegImm8
						( regTempMask,
							ECSSakura2Processor::regFillBit, 64 - nScale ) ;
					//
					m_pcsxi->WriteSakuraAddRegReg( regLoaded, regTempSign ) ;
					m_pcsxi->WriteSakuraXorRegReg( regLoaded, regTempSign ) ;
					//
					m_pcsxi->WriteSakuraAndRegReg( regLoaded, regTempMask ) ;
					//
					m_pcsxi->WriteSakuraAddRegReg( regLoaded, regTempSign ) ;
					m_pcsxi->WriteSakuraXorRegReg( regLoaded, regTempSign ) ;
					//
					FreeExpressionRegister() ;
					FreeExpressionRegister() ;
				}
				typeDst = typeSrc1Temp ;
				typeDst.SetLoadedRegister( regLoaded ) ;
				return	eslErrSuccess ;
			}
		}
		else if ( (regSrc2 < 0) &&
				((optOperator == csotShiftRight)
					|| (optOperator == csotShiftLeft)) )
		{
			//
			// reg >> imm8
			//
			const int	regLoaded = regSrc1 ;
			const INT64	nValue = typeSrc2Temp.GetValueInteger() ;
			ESLAssert( regLoaded >= 0 ) ;
			if ( (-0x80 <= nValue) && (nValue <= 0x7F) )
			{
				ECSInteger *	pSrcInt1 =
						(ECSInteger*) typeSrc1Temp.m_pValue ;
				ESLAssert( pSrcInt1->m_vtType == csvtInteger ) ;
				if ( optOperator == csotShiftLeft )
				{
					m_pcsxi->WriteSakuraSllRegRegImm8
							( regLoaded, regLoaded, (int) nValue ) ;
				}
				else if ( !pSrcInt1->IsSign() )
				{
					m_pcsxi->WriteSakuraSrlRegRegImm8
							( regLoaded, regLoaded, (int) nValue ) ;
				}
				else
				{
					m_pcsxi->WriteSakuraSraRegRegImm8
							( regLoaded, regLoaded, (int) nValue ) ;
				}
				typeDst = typeSrc1Temp ;
				typeDst.SetLoadedRegister( regLoaded ) ;
				return	eslErrSuccess ;
			}
		}
	}
	//
	// レジスタに値をロードする
	//
	MakeCommitValueToNakedRegister( typeSrc1Temp ) ;
	MakeCommitValueToNakedRegister( typeSrc2Temp ) ;
	//
	// 演算命令出力
	//
	return	CompileTypeNakedOperateCode
		( typeDst, typeSrc1Temp, typeSrc2Temp, optOperator ) ;
}

ESLError ECSCompiler::CompileTypeNakedOperateCode
	( ECSTypeInfo & typeDst, const ECSTypeInfo & typeSrc1,
		const ECSTypeInfo & typeSrc2, CSOperatorType optOperator )
{
	const int	regSrc1 = typeSrc1.GetLoadedRegister() ;
	const int	regSrc2 = typeSrc2.GetLoadedRegister() ;
	ESLAssert( regSrc1 >= 0 ) ;
	ESLAssert( regSrc2 >= 0 ) ;
	typeDst = typeSrc1 ;
	//
	if ( typeSrc1.IsTypeInteger() )
	{
		ESLAssert( typeSrc2.IsTypeInteger() ) ;
		ECSInteger *	pIntSrc1 = (ECSInteger*) typeSrc1.m_pValue ;
		ECSInteger *	pIntSrc2 = (ECSInteger*) typeSrc2.m_pValue ;
		ESLAssert( pIntSrc1->IsKindOf( ESL_RUNTIME_CLASS(ECSInteger) ) ) ;
		ESLAssert( pIntSrc2->IsKindOf( ESL_RUNTIME_CLASS(ECSInteger) ) ) ;
		int	nBitSizeSrc1 = pIntSrc1->SizeOf() ;
		int	nBitSizeSrc2 = pIntSrc2->SizeOf() ;
		if ( nBitSizeSrc1 < nBitSizeSrc2 )
		{
			typeDst.SetTypeValue
				( new ECSInteger( 0, pIntSrc2->GetValueMask() ), typeDst.m_dwFlags ) ;
		}
		switch ( optOperator )
		{
		case	csotAdd:
			m_pcsxi->WriteSakuraOperandRegReg
				( ECSSakura2Processor::codeAddReg, regSrc1, regSrc2 ) ;
			break ;
		case	csotSub:
			m_pcsxi->WriteSakuraOperandRegReg
				( ECSSakura2Processor::codeSubReg, regSrc1, regSrc2 ) ;
			break ;
		case	csotMul:
			if ( (nBitSizeSrc1 <= 32) && (nBitSizeSrc2 <= 32) )
			{
				ECSSakura2Processor::InstructionCode	codeMul ;
				if ( !pIntSrc1->IsSign() && !pIntSrc2->IsSign() )
				{
					// uint * uint
					codeMul = ECSSakura2Processor::codeMul32Reg ;
				}
				else if ( pIntSrc1->IsSign() && pIntSrc2->IsSign() )
				{
					// int * int
					codeMul = ECSSakura2Processor::codeIMul32Reg ;
				}
				else if ( (!pIntSrc1->IsSign()
							&& (pIntSrc1->SizeOf() == 32))
						|| (!pIntSrc2->IsSign()
							&& (pIntSrc2->SizeOf() == 32)) )
				{
					// int * uint, or uint * int
					codeMul = ECSSakura2Processor::codeMulReg ;
				}
				else
				{
					// int * under 16 bits
					codeMul = ECSSakura2Processor::codeIMul32Reg ;
				}
				m_pcsxi->WriteSakuraOperandRegReg
							( codeMul, regSrc1, regSrc2 ) ;
				//
				int	nBiggerBits =
					(nBitSizeSrc1 > nBitSizeSrc2)
							? nBitSizeSrc1 : nBitSizeSrc2 ;
				bool	fSign = pIntSrc1->IsSign() | pIntSrc2->IsSign() ;
				INT64	nMask = ECSInteger::m_maskInt64 ;
				if ( IsCStyleCompatibleMode() )
				{
					if ( nBiggerBits <= 8 )
					{
						nMask = fSign ? ECSInteger::m_maskInt8
										: ECSInteger::m_maskUint8 ;
					}
					else if ( nBiggerBits <= 16 )
					{
						nMask = fSign ? ECSInteger::m_maskInt16
										: ECSInteger::m_maskUint16 ;
					}
					else if ( nBiggerBits <= 32 )
					{
						nMask = fSign ? ECSInteger::m_maskInt32
										: ECSInteger::m_maskUint32 ;
					}
					else
					{
						nMask = fSign ? ECSInteger::m_maskInt64
										: ECSInteger::m_maskUint64 ;
					}
				}
				else
				{
					if ( nBiggerBits <= 8 )
					{
						nMask = fSign ? ECSInteger::m_maskInt16
										: ECSInteger::m_maskUint16 ;
					}
					else if ( nBiggerBits <= 16 )
					{
						nMask = fSign ? ECSInteger::m_maskInt32
										: ECSInteger::m_maskUint32 ;
					}
				}
				typeDst.SetTypeValue
					( new ECSInteger( 0, nMask ), typeDst.m_dwFlags ) ;
			}
			else
			{
				m_pcsxi->WriteSakuraOperandRegReg
					( ECSSakura2Processor::codeMulReg, regSrc1, regSrc2 ) ;
			}
			break ;
		case	csotDiv:
			if ( nBitSizeSrc2 <= 32 )
			{
				ECSSakura2Processor::InstructionCode	codeDiv ;
				if ( (nBitSizeSrc2 == 32) && !pIntSrc2->IsSign() )
				{
					if ( (nBitSizeSrc1 <= 32) && !pIntSrc1->IsSign() )
					{
						// uint / uint32
						codeDiv = ECSSakura2Processor::codeDiv32Reg ;
					}
					else
					{
						// int / uint32
						codeDiv = ECSSakura2Processor::codeDivReg ;
					}
				}
				else if ( !pIntSrc2->IsSign() )
				{
					ESLAssert( nBitSizeSrc2 < 32 ) ;
					if ( pIntSrc1->IsSign() )
					{
						// int / {uint16|uint8}
						codeDiv = ECSSakura2Processor::codeIDiv32Reg ;
					}
					else
					{
						// uint / uint16
						codeDiv = ECSSakura2Processor::codeDiv32Reg ;
					}
				}
				else
				{
					codeDiv = ECSSakura2Processor::codeIDiv32Reg ;
				}
				m_pcsxi->WriteSakuraOperandRegReg( codeDiv, regSrc1, regSrc2 ) ;
				//
				bool	fSign = pIntSrc1->IsSign() || pIntSrc2->IsSign() ;
				bool	fExSign = !pIntSrc1->IsSign() && pIntSrc2->IsSign() ;
				INT64	nMask = ECSInteger::m_maskInt64 ;
				if ( (nBitSizeSrc2 == 64)
					|| ((nBitSizeSrc2 == 32) && fExSign) )
				{
					nMask = ECSInteger::m_maskInt64 ;
				}
				else if ( (nBitSizeSrc2 >= 32)
						|| ((nBitSizeSrc2 == 16) && fExSign) )
				{
					nMask = fSign ? ECSInteger::m_maskInt32
									: ECSInteger::m_maskUint32 ;
				}
				else if ( (nBitSizeSrc2 >= 16)
						|| ((nBitSizeSrc2 == 8) && fExSign) )
				{
					nMask = fSign ? ECSInteger::m_maskInt16
									: ECSInteger::m_maskUint16 ;
				}
				else
				{
					nMask = fSign ? ECSInteger::m_maskInt8
									: ECSInteger::m_maskUint8 ;
				}
				typeDst.SetTypeValue
					( new ECSInteger( 0, nMask ), typeDst.m_dwFlags ) ;
			}
			else
			{
				m_pcsxi->WriteSakuraOperandRegReg
					( ECSSakura2Processor::codeDivReg, regSrc1, regSrc2 ) ;
			}
			break ;
		case	csotMod:
			if ( nBitSizeSrc2 <= 32 )
			{
				ECSSakura2Processor::InstructionCode	codeMod ;
				if ( (nBitSizeSrc2 == 32) && !pIntSrc2->IsSign() )
				{
					if ( (nBitSizeSrc1 <= 32) && !pIntSrc1->IsSign() )
					{
						// uint / uint32
						codeMod = ECSSakura2Processor::codeMod32Reg ;
					}
					else
					{
						// int / uint32
						codeMod = ECSSakura2Processor::codeModReg ;
					}
				}
				else if ( !pIntSrc2->IsSign() )
				{
					ESLAssert( nBitSizeSrc2 < 32 ) ;
					if ( pIntSrc1->IsSign() )
					{
						// int / {uint16|uint8}
						codeMod = ECSSakura2Processor::codeIMod32Reg ;
					}
					else
					{
						// uint / uint16
						codeMod = ECSSakura2Processor::codeMod32Reg ;
					}
				}
				else
				{
					codeMod = ECSSakura2Processor::codeIMod32Reg ;
				}
				m_pcsxi->WriteSakuraOperandRegReg( codeMod, regSrc1, regSrc2 ) ;
				//
				bool	fSign = pIntSrc1->IsSign() || pIntSrc2->IsSign() ;
				bool	fExSign = pIntSrc1->IsSign() && !pIntSrc2->IsSign() ;
				INT64	nMask = ECSInteger::m_maskInt64 ;
				if ( (nBitSizeSrc2 == 64)
					|| ((nBitSizeSrc2 == 32) && fExSign) )
				{
					nMask = ECSInteger::m_maskInt64 ;
				}
				else if ( (nBitSizeSrc2 >= 32)
						|| ((nBitSizeSrc2 == 16) && fExSign) )
				{
					nMask = fSign ? ECSInteger::m_maskInt32
									: ECSInteger::m_maskUint32 ;
				}
				else if ( (nBitSizeSrc2 >= 16)
						|| ((nBitSizeSrc2 == 8) && fExSign) )
				{
					nMask = fSign ? ECSInteger::m_maskInt16
									: ECSInteger::m_maskUint16 ;
				}
				else
				{
					nMask = fSign ? ECSInteger::m_maskInt8
									: ECSInteger::m_maskUint8 ;
				}
				typeDst.SetTypeValue
					( new ECSInteger( 0, nMask ), typeDst.m_dwFlags ) ;
			}
			else
			{
				m_pcsxi->WriteSakuraOperandRegReg
					( ECSSakura2Processor::codeModReg, regSrc1, regSrc2 ) ;
			}
			break ;
		case	csotAnd:
			m_pcsxi->WriteSakuraOperandRegReg
				( ECSSakura2Processor::codeAndReg, regSrc1, regSrc2 ) ;
			break ;
		case	csotOr:
			m_pcsxi->WriteSakuraOperandRegReg
				( ECSSakura2Processor::codeOrReg, regSrc1, regSrc2 ) ;
			break ;
		case	csotXor:
			m_pcsxi->WriteSakuraOperandRegReg
				( ECSSakura2Processor::codeXorReg, regSrc1, regSrc2 ) ;
			break ;
		case	csotShiftRight:
			ESLAssert( typeSrc1.m_pValue->m_vtType == csvtInteger ) ;
			if ( ((ECSInteger*)(typeSrc1.m_pValue))->IsSign() )
			{
				m_pcsxi->WriteSakuraOperandRegReg
					( ECSSakura2Processor::codeSraReg, regSrc1, regSrc2 ) ;
			}
			else
			{
				m_pcsxi->WriteSakuraOperandRegReg
					( ECSSakura2Processor::codeSrlReg, regSrc1, regSrc2 ) ;
			}
			typeDst = typeSrc1 ;
			break ;
		case	csotShiftLeft:
			m_pcsxi->WriteSakuraOperandRegReg
				( ECSSakura2Processor::codeSllReg, regSrc1, regSrc2 ) ;
			typeDst = typeSrc1 ;
			break ;
		case	csotLogicalAnd:
		case	csoutLogicalOr:
		default:
			return	ESLErrorMsg
				( "naked モードで定義されない整数の演算子です" ) ;
		}
	}
	else if ( typeSrc1.IsTypeReal() )
	{
		ESLAssert( typeSrc2.IsTypeReal() ) ;
		switch ( optOperator )
		{
		case	csotAdd:
			m_pcsxi->WriteSakuraOperandRegReg
				( ECSSakura2Processor::codeFAddReg, regSrc1, regSrc2 ) ;
			break ;
		case	csotSub:
			m_pcsxi->WriteSakuraOperandRegReg
				( ECSSakura2Processor::codeFSubReg, regSrc1, regSrc2 ) ;
			break ;
		case	csotMul:
			m_pcsxi->WriteSakuraOperandRegReg
				( ECSSakura2Processor::codeFMulReg, regSrc1, regSrc2 ) ;
			break ;
		case	csotDiv:
			m_pcsxi->WriteSakuraOperandRegReg
				( ECSSakura2Processor::codeFDivReg, regSrc1, regSrc2 ) ;
			break ;
		case	csotMod:
		case	csotAnd:
		case	csotOr:
		case	csotXor:
		case	csotShiftRight:
		case	csotShiftLeft:
		case	csotLogicalAnd:
		case	csoutLogicalOr:
		default:
			return	ESLErrorMsg
				( "naked モードで定義されない実数の演算子です" ) ;
		}
	}
	else
	{
		return	ESLErrorMsg( "naked モードで定義されない演算です" ) ;
	}
	int	regLoaded = regSrc1 ;
	if ( regSrc1 > regSrc2 )
	{
		regLoaded = regSrc2 ;
		m_pcsxi->WriteSakuraMoveRegReg( regSrc2, regSrc1 ) ;
	}
	FreeExpressionRegister() ;
//	ESLAssert( regLoaded == GetExpressionRegister() ) ;
	//
	typeDst.SetLoadedRegister( regLoaded ) ;
	return	eslErrSuccess ;
}

ESLError ECSCompiler::CompileTypePointerOperate
	( ECSTypeInfo & typeDst, const ECSTypeInfo & typeSrc1,
		const ECSTypeInfo & typeSrc2,
		CSOperatorType optOperator, CSInstructionCode icCode )
{
	ESLAssert( typeSrc1.IsTypePointer() ) ;
	switch ( optOperator )
	{
	case	csotAdd:
	case	csotSub:
		break ;
	default:
		return	ESLErrorMsg( "ポインタへの不正な演算子です" ) ;
	}
	if ( !typeSrc2.IsTypeInteger() )
	{
		return	ESLErrorMsg( "ポインタへの演算が整数ではありません" ) ;
	}
	//
	int			nPitch, nObjCount ;
	ESLError	err =
		ECSTypeInfo::GetNakedMemorySize
			( nPitch, nObjCount, typeSrc1.GetNakedPointerType() ) ;
	if ( err )
	{
		return	err ;
	}
	if ( m_modeNakedCode )
	{
		if ( optOperator == csotSub )
		{
			nPitch = - nPitch ;
		}
		return	CompileTypeNakedPointerAdd
					( typeDst, typeSrc1, typeSrc2, nPitch ) ;
	}
	else
	{
		if ( nPitch != 1 )
		{
			CompileImmediateInteger( nPitch ) ;
			CompileCodeOperate( csotMul ) ;
		}
		m_pcsxi->WriteInstructionCode( icCode ) ;
		m_pcsxi->WriteOperatorTypeCode( optOperator ) ;
		//
		typeDst.MakeNakedOf( typeSrc1 ) ;
	}
	return	eslErrSuccess ;
}

ESLError ECSCompiler::CompileTypeNakedPointerAdd
	( ECSTypeInfo & typeDst,
		const ECSTypeInfo & typeSrc1,
		const ECSTypeInfo & typeSrc2, int nPitch )
{
	ESLAssert( m_modeNakedCode ) ;
	ECSTypeInfo	typeSrc1Temp = typeSrc1 ;
	ECSTypeInfo	typeSrc2Temp = typeSrc2 ;
	while ( typeSrc1Temp.IsTypeReference() )
	{
		CompileCodeNakedUncoverReference( typeSrc1Temp ) ;
	}
	while ( typeSrc2Temp.IsTypeReference() )
	{
		CompileCodeNakedUncoverReference( typeSrc2Temp ) ;
	}
	ESLAssert( typeSrc1Temp.IsLoadedRegister()
				|| typeSrc1Temp.IsAddressingInfo() ) ;
	//
	int	regLoaded = -1 ;
	if ( !typeSrc2Temp.IsLoadedRegister() )
	{
		INT64	nValue = typeSrc2Temp.GetValueInteger() ;
		if ( typeSrc1Temp.IsAddressingInfo() )
		{
			typeSrc1Temp.m_addrOffset += (int) nValue * nPitch ;
		}
		else
		{
			regLoaded = typeSrc1Temp.GetLoadedRegister() ;
			m_pcsxi->WriteSakuraAddRegRegImm32
				( regLoaded, regLoaded, (int) nValue * nPitch ) ;
		}
	}
	else
	{
		if ( typeSrc1Temp.IsAddressingInfo()
			&& !typeSrc1Temp.IsLoadedRegister()
			&& (typeSrc1Temp.m_regIndex < 0) )
		{
			regLoaded = typeSrc2Temp.GetLoadedRegister() ;
			switch ( nPitch )
			{
			case	1:
				typeSrc1Temp.m_scaleIndex = 0 ;
				break ;
			case	2:
				typeSrc1Temp.m_scaleIndex = 1 ;
				break ;
			case	4:
				typeSrc1Temp.m_scaleIndex = 2 ;
				break ;
			case	8:
				typeSrc1Temp.m_scaleIndex = 3 ;
				break ;
			default:
				m_pcsxi->WriteSakuraMulRegRegImm32
							( regLoaded, regLoaded, nPitch ) ;
				break ;
			}
			typeSrc1Temp.m_regIndex = regLoaded ;
			typeSrc1Temp.SetLoadedRegister( regLoaded ) ;
		}
		else
		{
			MakeCommitValueToNakedRegister( typeSrc1Temp ) ;
			ESLAssert( !typeSrc1Temp.IsAddressingInfo() ) ;
			ESLAssert( typeSrc1Temp.IsLoadedRegister() ) ;
			//
			if ( nPitch != 1 )
			{
				regLoaded = typeSrc2Temp.GetLoadedRegister() ;
				m_pcsxi->WriteSakuraMulRegRegImm32
							( regLoaded, regLoaded, nPitch ) ;
			}
			int	regDst = typeSrc1Temp.GetLoadedRegister() ;
			int	regSrc = typeSrc2Temp.GetLoadedRegister() ;
			if ( regDst > regSrc )
			{
				int	regTemp = regSrc ;
				regSrc = regDst ;
				regDst = regTemp ;
			}
			m_pcsxi->WriteSakuraAddRegReg( regDst, regSrc ) ;
			ESLAssert( regSrc == GetExpressionRegister() ) ;
			FreeExpressionRegister() ;
			//
			typeSrc1Temp.ClearAddressingInfo() ;
			typeSrc1Temp.SetLoadedRegister( regDst ) ;
		}
	}
	typeDst = typeSrc1Temp ;
	return	eslErrSuccess ;
}

// 型代入演算
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileTypeMoveOperate
	( ECSTypeInfo & typeDst, const ECSTypeInfo & typeSrc1,
		const ECSTypeInfo & typeSrc2,
		CSOperatorType optOperator, bool fInitMove )
{
	if ( typeSrc2.IsVoid() )
	{
		return	ESLErrorMsg( "右辺式に void が指定されています。" ) ;
	}
	if ( (typeSrc1.m_dwFlags & ECSTypeInfo::flagConstant) && !fInitMove )
	{
		return	ESLErrorMsg( "不変オブジェクトへ変更を加えようとしています。" ) ;
	}
	if ( typeSrc1.IsVoid() || typeSrc2.IsVoid() )
	{
		return	ESLErrorMsg( "不正な代入演算子です。" ) ;
	}
	if ( typeSrc1.IsTypePointer() )
	{
		return	CompileTypeMovePointerOperate
					( typeDst, typeSrc1, typeSrc2, optOperator, fInitMove ) ;
	}
	ESLError	err ;
	if ( fInitMove )
	{
		if ( typeSrc1.IsTypeReference() )
		{
			//
			// 参照型の初期化
			//
			ECSTypeInfo	typeCast ;
			err = CompileTypeCast
				( typeCast, typeSrc1, typeSrc2, 0, ECSTypeInfo::flagPublic ) ;
			if ( err )
			{
				return	err ;
			}
			if ( m_modeNakedCode )
			{
				MakeCommitValueToNakedRegister( typeCast ) ;
				//
				ECSTypeInfo	typeVarRef ;
				typeVarRef.MakeReferenceOf( typeSrc1 ) ;
				typeVarRef.MoveRegisterAndAddressingFrom( typeSrc1 ) ;
				//
				int	regSrc = typeCast.GetLoadedRegister() ;
				WriteSakuraStoreMemory( regSrc, typeVarRef ) ;
				//
				FreeExpressionRegister( typeCast ) ;
				typeDst = typeVarRef ;
			}
			else
			{
				CompileCodeStore( csotNop ) ;
				typeDst = typeSrc1 ;
			}
			return	eslErrSuccess ;
		}
	}
	else
	{
		if ( !typeSrc1.IsAbstractType() && !typeSrc1.IsTypeReference() )
		{
			return	ESLErrorMsg( "不正な左辺式です。" ) ;
		}
	}
	if ( !m_modeNakedCode && typeSrc1.IsTypeArray() )
	{
		//
		// 配列の代入
		//
		if ( optOperator == csotNop )
		{
			if ( typeSrc1.IsMatchType( typeSrc2 ) != ECSTypeInfo::typeMatch )
			{
				return	ESLErrorMsg( "配列への代入で型が一致しません。" ) ;
			}
			CompileCodeStore( optOperator ) ;
			//
			typeDst = typeSrc1 ;
		}
		else if ( optOperator == csotAdd )
		{
			ECSObject *	pElementType = typeSrc1.GetArrayElementType() ;
			if ( pElementType != NULL )
			{
				ECSTypeInfo	typeTemp ;
				ECSTypeInfo	typeCast
					( ECSTypeInfo::DuplicateType( pElementType ), 0 ) ;
				err = CompileTypeCast
					( typeTemp, typeCast, typeSrc2,
							0, ECSTypeInfo::flagPublic ) ;
				if ( err )
				{
					return	err ;
				}
			}
			CompileCodeStore( optOperator ) ;
			//
			typeDst = typeSrc1 ;
		}
		else
		{
			return	ESLErrorMsg( "配列への不正な演算です。" ) ;
		}
		return	eslErrSuccess ;
	}
	else if ( !m_modeNakedCode && typeSrc1.IsTypeHashArray() )
	{
		//
		// Hash 配列の代入
		//
		if ( optOperator != csotNop )
		{
			return	ESLErrorMsg( "ハッシュ配列への不正な演算です。" ) ;
		}
		if ( typeSrc1.IsMatchType( typeSrc2 ) != ECSTypeInfo::typeMatch )
		{
			return	ESLErrorMsg( "ハッシュ配列への代入で型が一致しません。" ) ;
		}
		CompileCodeStore( optOperator ) ;
		//
		typeDst = typeSrc1 ;
		return	eslErrSuccess ;
	} 
	//
	// その他の代入
	//
	const ECSClassInfo *
		pClassInf = GetNakedTypeClassInfo( typeSrc1 ) ;
	if ( pClassInf != NULL )
	{
		if ( pClassInf->GetAttribute() & ECSTypeInfo::flagEnumerator )
		{
			if ( optOperator != csotNop )
			{
				return	ESLErrorMsg( "列挙型への不正な代入演算子です。" ) ;
			}
			const ECSClassInfo *
				pSrcClassInf = GetNakedTypeClassInfo( typeSrc2 ) ;
			if ( (typeSrc2.m_dwFlags & ECSTypeInfo::flagDeterministic)
				&& (typeSrc2.m_pValue != NULL) )
			{
				if ( !pClassInf->IsMatchEnumeratorValue( typeSrc2.m_pValue ) )
				{
					OutputWarning
						( EString(pClassInf->GetGlobalName())
							+ " 列挙型へ適合しない値の代入です",
											m_strFilePath, m_nLineNum ) ;
				}
			}
			else if ( (pClassInf == NULL)
					|| (pSrcClassInf == NULL)
					|| (pSrcClassInf->GetGlobalName()
							!= pClassInf->GetGlobalName()) )
			{
				OutputWarning
					( EString(pClassInf->GetGlobalName())
						+ " 列挙型へ定数値でないオブジェクトの代入です",
												m_strFilePath, m_nLineNum ) ;
			}
		}
		ECSClassInfo::ListMemberFunction	lstFunc ;
		EObjArray<ECSTypeInfo>				lstArg ;
		lstArg.Add( new ECSTypeInfo( typeSrc2 ) ) ;
		//
		bool	fOperator = false ;
		if ( fInitMove )
		{
			fOperator =
				pClassInf->SearchFunctinoAs
					( lstFunc, pClassInf->GetName(), lstArg,
						0, true, false, m_modeNakedCode ) ;
			if ( !fOperator )
			{
				if ( (lstFunc.GetSize() > 1)
						&& !(m_dwModeFlags & flagStrictStyle) )
				{
					fOperator = true ;
				}
			}
		}
		if ( !fOperator )
		{
			fOperator =
				pClassInf->SearchMoveOperatorAs
					( lstFunc, optOperator,
						lstArg, true, m_modeNakedCode ) ;
			if ( !fOperator )
			{
				if ( lstFunc.GetSize() == 0 )
				{
					fOperator =
						pClassInf->SearchMoveOperatorAs
							( lstFunc, optOperator,
								lstArg, false, m_modeNakedCode ) ;
					if ( fOperator && (m_dwModeFlags & flagStrictStyle) )
					{
						OutputWarning
							( "ルーズな型変換を伴う代入演算子です",
											m_strFilePath, m_nLineNum ) ;
					}
				}
				if ( lstFunc.GetSize() >= 2 )
				{
					if ( m_dwModeFlags & flagStrictStyle )
					{
						MakeAmbiguousWarningMsg2
							( "曖昧な代入演算子です", &lstArg, &lstFunc ) ;
					}
					fOperator = true ;
				}
			}
		}
		if ( fOperator )
		{
			typeDst = lstFunc[0].GetReturnType() ;
			//
			if ( pClassInf->GetAttribute() & ECSTypeInfo::flagNativeObject )
			{
				//
				// native クラスへの代入
				//
				if ( typeSrc1.IsTypeInteger() && typeSrc2.IsTypeInteger() )
				{
					ECSTypeInfo	typeTempDst, typeTempSrc ;
					typeTempDst.MakeNakedOf( typeSrc1 ) ;
					typeTempSrc.MakeNakedOf( typeSrc2 ) ;
					typeTempDst.m_dwFlags &= ~ECSTypeInfo::flagConstant ;
					typeTempSrc.m_dwFlags &= ~ECSTypeInfo::flagConstant ;
					//
					ECSTypeInfo::TypeMatchResult
						matchResult = typeTempDst.IsMatchType( typeTempSrc ) ;
					if ( matchResult > ECSTypeInfo::typeNatualMatch )
					{
						EWideString	wstrDstType, wstrSrcType ;
						typeTempDst.FormatTypeString( wstrDstType ) ;
						typeTempSrc.FormatTypeString( wstrSrcType ) ;
						//
						err = OutputWarning0
							( EString(wstrSrcType) + " を "
								+ EString(wstrDstType)
								+ " に代入しています。",
									m_strFilePath, m_nLineNum ) ;
						if ( err )
						{
							return	err ;
						}
					}
				}
				if ( m_modeNakedCode )
				{
					ECSTypeInfo	typeDstRef = typeSrc1 ;
					while ( typeDstRef.IsTypeReference2() )
					{
						CompileCodeNakedUncoverReference( typeDstRef ) ;
					}
					ECSTypeInfo	typeDstLoaded = typeDstRef ;
					int			regLoaded ;
					if ( optOperator != csotNop )
					{
						regLoaded = AllocateExpressionRegister() ;
						CompileCodeNakedUncoverReference( typeDstLoaded, regLoaded ) ;
						LoadCommitValueToNakedRegister( regLoaded, typeDstLoaded ) ;
//						typeDstLoaded.SetLoadedRegister( regLoaded ) ;
					}
					ECSTypeInfo	typeCast ;
					err = CompileTypeCast
						( typeCast, typeSrc1, typeSrc2,
							castCastNoRef, ECSTypeInfo::flagPublic ) ;
					if ( err )
					{
						return	err ;
					}
					while ( typeCast.IsTypeReference() )
					{
						CompileCodeNakedUncoverReference( typeCast ) ;
					}
					MakeCommitValueToNakedRegister( typeCast ) ;
					//
					if ( optOperator != csotNop )
					{
						ECSTypeInfo	typeDstTemp ;
						err = CompileTypeNakedOperateCode
							( typeDstTemp, typeDstLoaded,
										typeCast, optOperator ) ;
						if ( err )
						{
							return	err ;
						}
						int	regSrc = typeDstTemp.GetLoadedRegister() ;
						WriteSakuraStoreMemory( regSrc, typeDstRef ) ;
						//
						FreeExpressionRegister() ;
					}
					else
					{
						int	regSrc = typeCast.GetLoadedRegister() ;
						WriteSakuraStoreMemory( regSrc, typeDstRef ) ;
						FreeExpressionRegister( typeCast ) ;
					}
					if ( typeDstRef.IsLoadedRegister() )
					{
						if ( GetExpressionRegister()
								!= typeDstRef.GetLoadedRegister() )
						{
							m_pcsxi->WriteSakuraMoveRegReg
								( GetExpressionRegister(),
									typeDstRef.GetLoadedRegister() ) ;
							typeDstRef.SetLoadedRegister
									( GetExpressionRegister() ) ;
						}
					}
					typeDst = typeDstRef ;
				}
				else
				{
					CompileCodeStore( optOperator ) ;
				}
				return	eslErrSuccess ;
			}
			//
			// クラスメンバ呼び出し
			//
			DWORD		dwArgCount = 1 ;
			ECSTypeInfo	typeThis = typeSrc1 ;
			if ( m_modeNakedCode )
			{
				/*
				LoadCommitValueToNakedRegister
					( AllocateExpressionRegister(), typeSrc1 ) ;
				LoadCommitValueToNakedRegister
					( AllocateExpressionRegister(), typeSrc2 ) ;
				*/
				EObjArray<ECSTypeInfo>	lstArgType ;
				lstArgType[0] = typeSrc1 ;
				lstArgType[1] = typeSrc2 ;
				//
				ESLError	err =
					NormalizeNakedArgument
						( lstFunc.GetAt(0), lstArgType, dwArgCount ) ;
				if ( err )
				{
					return	err ;
				}
				typeThis = lstArgType[0] ;
			}
			else if ( fInitMove )
			{
				CompileLoadStackObject( 1 ) ;
				CompileCodeSwap( 0, 1 ) ;
			}
			err = CompileCallMemberFunction
					( *pClassInf, typeThis, lstFunc[0],
							lstArg, dwArgCount, ECSTypeInfo::flagPublic ) ;
			if ( m_modeNakedCode )
			{
				CompileCodeFreeStack( typeSrc2 ) ;
				CompileCodeFreeStack( typeSrc1 ) ;
				//
				if ( !typeDst.IsVoid() )
				{
					typeDst.SetLoadedRegister( GetExpressionRegister() ) ;
					m_pcsxi->WriteSakuraMoveRegReg
						( typeDst.GetLoadedRegister(),
								ECSSakura2Processor::regAcc ) ;
					GetFunctionReturnType( typeDst, lstFunc[0] ) ;
				}
			}
			else if ( fInitMove )
			{
				CompileCodeFreeStack( typeDst ) ;
			}
			return	err ;
		}
		else if ( lstFunc.GetSize() >= 2 )
		{
			return	ESLErrorMsg
				( "複数の適合する代入演算子オーバーロードが存在します。" ) ;
		}
		else if ( (optOperator == csotNop)
				&& (pClassInf->GetAttribute()
							& ECSTypeInfo::flagStructure) )
		{
			if ( pClassInf == GetNakedTypeClassInfo( typeSrc2 ) )
			{
				if ( m_modeNakedCode )
				{
					if ( !pClassInf->IsNakedMemoryClass() )
					{
						return	ESLErrorMsg
							( "naked モードで naked でない構造体の代入です" ) ;
					}
					if ( pClassInf->GetVirtualFunctionCount() > 0 )
					{
						OutputWarning
							( "仮想関数をメンバに持つ構造体の"
								"デフォルトの複製処理を行っています"
								"（構築関数の記述が推奨されます）",
									m_strFilePath, m_nLineNum ) ;
					}
					const int	regArg1 = AllocateExpressionRegister() ;
					const int	regArg2 = AllocateExpressionRegister() ;
					const int	regArg3 = AllocateExpressionRegister() ;
					ECSTypeInfo	typeTemp1 = typeSrc1 ;
					ECSTypeInfo	typeTemp2 = typeSrc2 ;
					//
					LoadCommitValueToNakedRegister( regArg1, typeTemp1 ) ;
					typeTemp1.ClearAddressingInfo() ;
					typeTemp1.SetLoadedRegister( regArg1 ) ;
					while ( typeTemp1.IsTypeReference2() )
					{
						CompileCodeNakedUncoverReference( typeTemp1 ) ;
						MakeCommitValueToNakedRegister( typeTemp1 ) ;
					}
					LoadCommitValueToNakedRegister( regArg2, typeTemp2 ) ;
					typeTemp2.ClearAddressingInfo() ;
					typeTemp2.SetLoadedRegister( regArg2 ) ;
					while ( typeTemp2.IsTypeReference2() )
					{
						CompileCodeNakedUncoverReference( typeTemp2 ) ;
						MakeCommitValueToNakedRegister( typeTemp2 ) ;
					}
					m_pcsxi->WriteSakuraLoadInt64
						( regArg3, pClassInf->GetNakedMemorySize() ) ;
					CompileNakedSystemCall( L"memmove", 3 ) ;
					//
					if ( fInitMove )
					{
						FreeExpressionRegister( typeSrc1 ) ;
					}
					FreeExpressionRegister( typeSrc2 ) ;
				}
				else
				{
					CompileCodeStore( optOperator ) ;
				}
				return	eslErrSuccess ;
			}
			else if ( !m_modeNakedCode
						&& !(m_dwModeFlags & flagStrictStyle)
						&& typeSrc2.IsAbstractType() )
			{
				CompileCodeStore( optOperator ) ;
				return	eslErrSuccess ;
			}
		}
	}
	else if ( !m_modeNakedCode &&
			(!(m_dwModeFlags & flagStrictStyle)
					|| typeSrc1.IsAbstractType()) )
	{
		CompileCodeStore( optOperator ) ;
		//
		typeDst = typeSrc1 ;
		return	eslErrSuccess ;
	}
	if ( fInitMove )
	{
		return	ESLErrorMsg( "適合する構築関数が見つかりません" ) ;
	}
	return	ESLErrorMsg( "不正な代入演算子です。" ) ;
}

ESLError ECSCompiler::CompileTypeMovePointerOperate
	( ECSTypeInfo & typeDst,
		const ECSTypeInfo & typeSrc1,
		const ECSTypeInfo & typeSrc2,
		CSOperatorType optOperator, bool fInitMove )
{
	ESLError	err ;
	ECSTypeInfo	typePtrRef ;
	if ( !fInitMove )
	{
		if ( !typeSrc1.IsTypeReference() )
		{
			return	ESLErrorMsg( "不正な左辺式です。" ) ;
		}
		typePtrRef = typeSrc1 ;
	}
	else if ( !typeSrc1.IsTypeReference() )
	{
		typePtrRef.MakeReferenceOf( typeSrc1 ) ;
		typePtrRef.MoveRegisterAndAddressingFrom( typeSrc1 ) ;
	}
	else
	{
		typePtrRef = typeSrc1 ;
	}
	if ( optOperator != csotNop )
	{
		//
		// ポインタへの演算
		//
		ECSTypeInfo	typeSrc2Temp = typeSrc2 ;
		NormalizePointerFromLinearAddress( typeSrc2Temp ) ;
		//
		ECSTypeInfo	typeTempPtr = typePtrRef ;
		int	regTempPtr = -1 ;
		while ( m_modeNakedCode && typeTempPtr.IsTypeReference() )
		{
			if ( regTempPtr < 0 )
			{
				regTempPtr = AllocateExpressionRegister() ;
			}
			CompileCodeNakedUncoverReference( typeTempPtr, regTempPtr ) ;
		}
		ECSTypeInfo	typeTemp ;
		err = CompileTypePointerOperate
			( typeTemp, typeTempPtr, typeSrc2Temp, optOperator, csicStore ) ;
		if ( err )
		{
			return	err ;
		}
		if ( m_modeNakedCode )
		{
			MakeCommitValueToNakedRegister( typeTemp ) ;
			WriteSakuraStoreMemory
				( typeTemp.GetLoadedRegister(), typePtrRef ) ;
			if ( regTempPtr > 0 )
			{
				FreeExpressionRegister() ;
			}
		}
		typeDst = typePtrRef ;
		return	eslErrSuccess ;
	}
	//
	// ポインタへの代入
	//
	ECSTypeInfo	typeCast ;
	err = CompileTypeCast
		( typeCast, typeSrc1, typeSrc2,
			castCastNoRef, ECSTypeInfo::flagPublic ) ;
	if ( err )
	{
		return	err ;
	}
	if ( m_modeNakedCode )
	{
		MakeCommitValueToNakedRegister( typeCast ) ;
		WriteSakuraStoreMemory
			( typeCast.GetLoadedRegister(), typePtrRef ) ;
		FreeExpressionRegister( typeCast ) ;
	}
	else
	{
		CompileCodeStore( csotNop ) ;
	}
	typeDst = typePtrRef ;
	return	eslErrSuccess ;
}

// 型比較演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileTypeCompare
	( ECSTypeInfo & typeDst, const ECSTypeInfo & typeSrc1,
		const ECSTypeInfo & typeSrc2, CSCompareType cptCompare )
{
	if ( (cptCompare >= csctPointerComparatorFirst)
		&& (cptCompare < csctMax) )
	{
		//
		// 参照ポインタ比較
		//
		if ( !typeSrc1.IsTypeReference() && !typeSrc1.IsAbstractType()
			&& !typeSrc2.IsTypeReference() && !typeSrc2.IsAbstractType() )
		{
			ESLError	err =
				OutputWarning0
					( "ポインタ比較演算子で不正な比較です。",
									m_strFilePath, m_nLineNum ) ;
			if ( err )
			{
				return	err ;
			}
		}
		typeDst = ECSTypeInfo
			( new ECSInteger( 0, ECSInteger::m_maskBoolean ) ) ;
		if ( m_modeNakedCode )
		{
			ECSTypeInfo	typeSrcTemp1 = typeSrc1 ;
			ECSTypeInfo	typeSrcTemp2 = typeSrc2 ;
			if ( typeSrcTemp1.IsTypeReference2() )
			{
				CompileCodeNakedUncoverReference( typeSrcTemp1 ) ;
			}
			MakeCommitValueToNakedRegister( typeSrcTemp1 ) ;
			//
			if ( typeSrcTemp2.IsTypeReference2() )
			{
				CompileCodeNakedUncoverReference( typeSrcTemp2 ) ;
			}
			MakeCommitValueToNakedRegister( typeSrcTemp2 ) ;
			//
			int	regDst = typeSrcTemp1.GetLoadedRegister() ;
			int	regSrc = typeSrcTemp2.GetLoadedRegister() ;
			if ( regDst > regSrc )
			{
				int	regTemp = regDst ;
				regDst = regSrc ;
				regSrc = regTemp ;
			}
			if ( cptCompare == csctNotEqualPointer )
			{
				m_pcsxi->WriteSakuraCmpNeRegReg( regDst, regSrc ) ;
			}
			else
			{
				m_pcsxi->WriteSakuraCmpEqRegReg( regDst, regSrc ) ;
			}
			FreeExpressionRegister() ;
			typeDst.SetLoadedRegister( regDst ) ;
			ESLAssert( GetExpressionRegister() == regDst ) ;
		}
		else
		{
			CompileCodeCompare( cptCompare ) ;
		}
		return	eslErrSuccess ;
	}
	if ( typeSrc1.IsVoid() || typeSrc2.IsVoid()
		|| typeSrc1.IsTypeArray() || typeSrc1.IsTypeHashArray() )
	{
		return	ESLErrorMsg( "不正な比較演算子です。" ) ;
	}
	if ( typeSrc1.IsTypePointer() )
	{
		return	CompileTypePointerCompare
					( typeDst, typeSrc1, typeSrc2, cptCompare ) ;
	}
	const ECSClassInfo *
		pClassInf = GetNakedTypeClassInfo( typeSrc1 ) ;
	if ( pClassInf != NULL )
	{
		ECSClassInfo::ListMemberFunction	lstFunc ;
		EObjArray<ECSTypeInfo>				lstArg ;
		lstArg.Add( new ECSTypeInfo( typeSrc2 ) ) ;
		//
		bool	fComparator =
			pClassInf->SearchComparatorAs
					( lstFunc, cptCompare, lstArg, m_modeNakedCode ) ;
		if ( !fComparator )
		{
			if ( (lstFunc.GetSize() >= 2)
					&& (m_dwModeFlags & flagStrictStyle) )
			{
				MakeAmbiguousWarningMsg2
					( "複数の適合する比較演算子オーバーロードが存在します。",
														&lstArg, &lstFunc ) ;
			}
			fComparator = (lstFunc.GetSize() >= 1) ;
		}
		if ( fComparator )
		{
			typeDst = lstFunc[0].GetReturnType() ;
			//
			if ( pClassInf->GetAttribute()
						& ECSTypeInfo::flagNativeObject )
			{
				if ( m_modeNakedCode )
				{
					return	CompileTypeNakedCompare
							( typeDst, typeSrc1, typeSrc2, cptCompare ) ;
				}
				CompileCodeCompare( cptCompare ) ;
				return	eslErrSuccess ;
			}
			ESLError	err ;
			DWORD		dwArgCount = 1 ;
			ECSTypeInfo	typeThis = typeSrc1 ;
			if ( m_modeNakedCode )
			{
				/*
				LoadCommitValueToNakedRegister
					( AllocateExpressionRegister(), typeSrc1 ) ;
				LoadCommitValueToNakedRegister
					( AllocateExpressionRegister(), typeSrc2 ) ;
				*/
				EObjArray<ECSTypeInfo>	lstArgType ;
				lstArgType[0] = typeSrc1 ;
				lstArgType[1] = typeSrc2 ;
				//
				err = NormalizeNakedArgument
						( lstFunc.GetAt(0), lstArgType, dwArgCount ) ;
				if ( err )
				{
					return	err ;
				}
				typeThis = lstArgType[0] ;
			}
			err = CompileCallMemberFunction
					( *pClassInf, typeThis, lstFunc[0],
						lstArg, dwArgCount, ECSTypeInfo::flagPublic ) ;
			int	regReturn = GetExpressionRegister() ;
			//
			FreeExpressionRegister( typeSrc2 ) ;
			FreeExpressionRegister( typeSrc1 ) ;
			//
			if ( !typeDst.IsVoid() && m_modeNakedCode )
			{
				if ( regReturn != GetExpressionRegister() )
				{
					m_pcsxi->WriteSakuraMoveRegReg
							( GetExpressionRegister(), regReturn ) ;
				}
				GetFunctionReturnType( typeDst, lstFunc[0] ) ;
//				typeDst.SetLoadedRegister( GetExpressionRegister() ) ;
			}
			return	err ;
		}
	}
	else
	{
		if ( m_modeNakedCode )
		{
			return	ESLErrorMsg
				( "naked モードでクラス情報の無い比較です" ) ;
		}
		CompileCodeCompare( cptCompare ) ;
		//
		typeDst = ECSTypeInfo
			( new ECSInteger( 0, ECSInteger::m_maskBoolean ) ) ;
		return	eslErrSuccess ;
	}
	return	ESLErrorMsg( "不正な比較演算子です。" ) ;
}

ESLError ECSCompiler::CompileTypeNakedCompare
	( ECSTypeInfo & typeDst, const ECSTypeInfo & typeSrc1,
		const ECSTypeInfo & typeSrc2, CSCompareType cptCompare )
{
	//
	// メモリ参照を展開
	//
	ESLAssert( m_modeNakedCode ) ;
	ECSTypeInfo	typeSrc1Temp = typeSrc1 ;
	ECSTypeInfo	typeSrc2Temp = typeSrc2 ;
	while ( typeSrc1Temp.IsTypeReference() )
	{
		CompileCodeNakedUncoverReference( typeSrc1Temp ) ;
	}
	while ( typeSrc2Temp.IsTypeReference() )
	{
		CompileCodeNakedUncoverReference( typeSrc2Temp ) ;
	}
	CompileCodeNakedUncoverEnumerator( typeSrc1Temp ) ;
	CompileCodeNakedUncoverEnumerator( typeSrc2Temp ) ;
	//
	if ( (!typeSrc1Temp.IsTypeInteger() && !typeSrc1Temp.IsTypeReal())
		|| (!typeSrc2Temp.IsTypeInteger() && !typeSrc2Temp.IsTypeReal()) )
	{
		return	ESLErrorMsg
			( "naked モードで整数でも実数でもない演算は定義されません" ) ;
	}
	//
	// 一方が実数型の場合には両者を実数に変換
	//
	bool	fFloat = false ;
	if ( typeSrc1Temp.IsTypeReal()
		|| typeSrc2Temp.IsTypeReal() )
	{
		CompileCodeNakedConvertToReal( typeSrc1Temp ) ;
		CompileCodeNakedConvertToReal( typeSrc2Temp ) ;
		fFloat = true ;
	}
	int	regSrc1 = typeSrc1Temp.IsLoadedRegister()
						? typeSrc1Temp.GetLoadedRegister() : -1 ;
	int	regSrc2 = typeSrc2Temp.IsLoadedRegister()
						? typeSrc2Temp.GetLoadedRegister() : -1 ;
	if ( (regSrc1 < 0) && (regSrc2 < 0) )
	{
		//
		// 定数計算
		//
		ESLAssert( typeSrc1Temp.m_pValue != NULL ) ;
		ESLAssert( typeSrc2Temp.m_pValue != NULL ) ;
		int			nCompareResult ;
		ESLError	err =
			typeSrc1Temp.m_pValue->Compare
				( m_ctxExpr, nCompareResult,
					cptCompare, *(typeSrc2Temp.m_pValue) ) ;
		if ( err )
		{
			return	err ;
		}
		typeDst.SetTypeValue
			( new ECSInteger( nCompareResult, ECSInteger::m_maskBoolean ),
										ECSTypeInfo::flagDeterministic ) ;
		return	eslErrSuccess ;
	}
	//
	// レジスタに値をロードする
	//
	MakeCommitValueToNakedRegister( typeSrc1Temp ) ;
	MakeCommitValueToNakedRegister( typeSrc2Temp ) ;
	//
	// 演算命令出力
	//
	return	CompileTypeNakedCompareCode
		( typeDst, typeSrc1Temp, typeSrc2Temp, cptCompare ) ;
}

ESLError ECSCompiler::CompileTypeNakedCompareCode
	( ECSTypeInfo & typeDst, const ECSTypeInfo & typeSrc1,
		const ECSTypeInfo & typeSrc2, CSCompareType cptCompare )
{
	const int	regSrc1 = typeSrc1.GetLoadedRegister() ;
	const int	regSrc2 = typeSrc2.GetLoadedRegister() ;
	ESLAssert( regSrc1 >= 0 ) ;
	ESLAssert( regSrc2 >= 0 ) ;
	//
	if ( typeSrc1.IsTypeInteger() || typeSrc1.IsTypePointer() )
	{
		ESLAssert( typeSrc2.IsTypeInteger() || typeSrc2.IsTypePointer() ) ;
		bool	fSrc1Uint64 = false, fSrc2Uint64 = false ;
		ECSObject *	pSrc1 = typeSrc1.GetNakedType() ;
		if ( (pSrc1 != NULL) && (pSrc1->m_vtType == csvtInteger) )
		{
			fSrc1Uint64 = (((ECSInteger*)pSrc1)->GetValueMask()
										== ECSInteger::m_maskUint64) ;
		}
		ECSObject *	pSrc2 = typeSrc2.GetNakedType() ;
		if ( (pSrc2 != NULL) && (pSrc2->m_vtType == csvtInteger) )
		{
			fSrc2Uint64 = (((ECSInteger*)pSrc2)->GetValueMask()
										== ECSInteger::m_maskUint64) ;
		}
		switch ( cptCompare )
		{
		case	csctNotEqual:
			m_pcsxi->WriteSakuraOperandRegReg
				( ECSSakura2Processor::codeCmpNeReg, regSrc1, regSrc2 ) ;
			break ;
		case	csctEqual:
			m_pcsxi->WriteSakuraOperandRegReg
				( ECSSakura2Processor::codeCmpEqReg, regSrc1, regSrc2 ) ;
			break ;
		case	csctLessThan:
			if ( !fSrc1Uint64 || !fSrc2Uint64 )
			{
				m_pcsxi->WriteSakuraOperandRegReg
					( ECSSakura2Processor::codeCmpLtReg, regSrc1, regSrc2 ) ;
			}
			else
			{
				m_pcsxi->WriteSakuraOperandRegReg
					( ECSSakura2Processor::codeCmpCReg, regSrc1, regSrc2 ) ;
			}
			break ;
		case	csctLessEqual:
			if ( !fSrc1Uint64 || !fSrc2Uint64 )
			{
				m_pcsxi->WriteSakuraOperandRegReg
					( ECSSakura2Processor::codeCmpLeReg, regSrc1, regSrc2 ) ;
			}
			else
			{
				m_pcsxi->WriteSakuraOperandRegReg
					( ECSSakura2Processor::codeCmpCZReg, regSrc1, regSrc2 ) ;
			}
			break ;
		case	csctGreaterThan:
			if ( !fSrc1Uint64 || !fSrc2Uint64 )
			{
				m_pcsxi->WriteSakuraOperandRegReg
					( ECSSakura2Processor::codeCmpGtReg, regSrc1, regSrc2 ) ;
			}
			else
			{
				m_pcsxi->WriteSakuraOperandRegReg
					( ECSSakura2Processor::codeCmpCReg, regSrc2, regSrc1 ) ;
				m_pcsxi->WriteSakuraMoveRegReg( regSrc1, regSrc2 ) ;
			}
			break ;
		case	csctGreaterEqual:
			if ( !fSrc1Uint64 || !fSrc2Uint64 )
			{
				m_pcsxi->WriteSakuraOperandRegReg
					( ECSSakura2Processor::codeCmpGeReg, regSrc1, regSrc2 ) ;
			}
			else
			{
				m_pcsxi->WriteSakuraOperandRegReg
					( ECSSakura2Processor::codeCmpCZReg, regSrc2, regSrc1 ) ;
				m_pcsxi->WriteSakuraMoveRegReg( regSrc1, regSrc2 ) ;
			}
			break ;
		default:
			return	ESLErrorMsg
				( "naked モードで定義されない比較演算子です" ) ;
		}
	}
	else if ( typeSrc1.IsTypeReal() )
	{
		ESLAssert( typeSrc2.IsTypeReal() ) ;
		switch ( cptCompare )
		{
		case	csctNotEqual:
			m_pcsxi->WriteSakuraOperandRegReg
				( ECSSakura2Processor::codeFCmpNeReg, regSrc1, regSrc2 ) ;
			break ;
		case	csctEqual:
			m_pcsxi->WriteSakuraOperandRegReg
				( ECSSakura2Processor::codeFCmpEqReg, regSrc1, regSrc2 ) ;
			break ;
		case	csctLessThan:
			m_pcsxi->WriteSakuraOperandRegReg
				( ECSSakura2Processor::codeFCmpLtReg, regSrc1, regSrc2 ) ;
			break ;
		case	csctLessEqual:
			m_pcsxi->WriteSakuraOperandRegReg
				( ECSSakura2Processor::codeFCmpLeReg, regSrc1, regSrc2 ) ;
			break ;
		case	csctGreaterThan:
			m_pcsxi->WriteSakuraOperandRegReg
				( ECSSakura2Processor::codeFCmpGtReg, regSrc1, regSrc2 ) ;
			break ;
		case	csctGreaterEqual:
			m_pcsxi->WriteSakuraOperandRegReg
				( ECSSakura2Processor::codeFCmpGeReg, regSrc1, regSrc2 ) ;
			break ;
		default:
			return	ESLErrorMsg
				( "naked モードで定義されない比較演算子です" ) ;
		}
	}
	else
	{
		return	ESLErrorMsg( "naked モードで定義されない比較です" ) ;
	}
	int	regLoaded = regSrc1 ;
	if ( regSrc1 > regSrc2 )
	{
		regLoaded = regSrc2 ;
		m_pcsxi->WriteSakuraMoveRegReg( regSrc2, regSrc1 ) ;
	}
	FreeExpressionRegister() ;
	ESLAssert( regLoaded == GetExpressionRegister() ) ;
	//
	typeDst.SetTypeValue
		( new ECSInteger( 0, ECSInteger::m_maskBoolean ), 0 ) ;
	typeDst.SetLoadedRegister( regLoaded ) ;
	return	eslErrSuccess ;
}

ESLError ECSCompiler::CompileTypePointerCompare
	( ECSTypeInfo & typeDst, const ECSTypeInfo & typeSrc1,
		const ECSTypeInfo & typeSrc2, CSCompareType cptCompare )
{
	bool	fSrcNullPtr = false ;
	ESLAssert( typeSrc1.IsTypePointer() ) ;
	if ( !typeSrc2.IsTypePointer() )
	{
		ESLError	err =
			ESLErrorMsg( "ポインタの比較対象がポインタでありません" ) ;
		ECSObject *	pType = typeSrc2.m_pValue ;
		if ( (pType != NULL)
			&& (typeSrc2.m_dwFlags & ECSTypeInfo::flagDeterministic) )
		{
			if ( pType->m_vtType == csvtInteger )
			{
				if ( ((ECSInteger*)pType)->GetValue() != 0 )
				{
					return	err ;
				}
				if ( !m_modeNakedCode )
				{
					CompileCodePointerToAddress() ;
				}
				fSrcNullPtr = true ;
			}
			else if ( pType->m_vtType == csvtReference )
			{
				if ( ((ECSReference*)pType)->m_pRef != NULL )
				{
					return	err ;
				}
				if ( !m_modeNakedCode )
				{
					CompileCodePointerToObject( 0 ) ;
				}
				fSrcNullPtr = true ;
			}
			else
			{
				return	err ;
			}
		}
		else
		{
			return	err ;
		}
	}
	/*
	ESLError	err =
		CompileTypeCast( typeSrc1, typeSrc2, 0, ECSTypeInfo::flagPublic ) ;
	if ( err )
	{
		return	err ;
	}
	*/
	switch ( cptCompare )
	{
	case	csctNotEqual:
	case	csctEqual:
		break ;
	default:
		return	ESLErrorMsg( "不正なポインタの比較演算子です" ) ;
	}
	typeDst = ECSTypeInfo
		( new ECSInteger( 0, ECSInteger::m_maskBoolean ) ) ;
	if ( m_modeNakedCode )
	{
		ESLAssert( m_modeNakedCode ) ;
		ECSTypeInfo	typeSrc1Temp = typeSrc1 ;
		ECSTypeInfo	typeSrc2Temp = typeSrc2 ;
		//
		while ( typeSrc1Temp.IsTypeReference() )
		{
			CompileCodeNakedUncoverReference( typeSrc1Temp ) ;
		}
		if ( !fSrcNullPtr )
		{
			while ( typeSrc2Temp.IsTypeReference() )
			{
				CompileCodeNakedUncoverReference( typeSrc2Temp ) ;
			}
		}
		MakeCommitValueToNakedRegister( typeSrc1Temp ) ;
		if ( !fSrcNullPtr )
		{
			MakeCommitValueToNakedRegister( typeSrc2Temp ) ;
		}
		int	regDst = typeSrc1Temp.GetLoadedRegister() ;
		int	regSrc = (!fSrcNullPtr || typeSrc2Temp.IsLoadedRegister())
						? typeSrc2Temp.GetLoadedRegister()
							: ECSSakura2Processor::regIntZero ;
		switch ( cptCompare )
		{
		case	csctNotEqual:
			m_pcsxi->WriteSakuraCmpNeRegReg( regDst, regSrc ) ;
			break ;
		case	csctEqual:
			m_pcsxi->WriteSakuraCmpEqRegReg( regDst, regSrc ) ;
			break ;
		}
		int	regLoaded = regDst ;
		if ( typeSrc1Temp.IsLoadedRegister()
			&& typeSrc2Temp.IsLoadedRegister() && (regDst > regSrc) )
		{
			regLoaded = regSrc ;
			m_pcsxi->WriteSakuraMoveRegReg( regSrc, regDst ) ;
		}
		if ( !fSrcNullPtr || typeSrc2.IsLoadedRegister() )
		{
			FreeExpressionRegister() ;
			ESLAssert( regLoaded == GetExpressionRegister() ) ;
		}
		typeDst.SetLoadedRegister( regLoaded ) ;
	}
	else
	{
		CompileCodeCompare( cptCompare ) ;
	}
	return	eslErrSuccess ;
}

// メンバ変数参照
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileTypeMemberVariable
	( ECSTypeInfo & typeDst,
		const ECSTypeInfo & typeSrc, const EWideString & wstrMember )
{
	const ECSClassInfo *
		pClassInf = GetNakedTypeClassInfo( typeSrc ) ;
	//
	if ( pClassInf != NULL )
	{
		//
		// メンバ変数参照
		//
		if ( m_modeNakedCode && !pClassInf->IsNakedMemoryClass() )
		{
			return	ESLErrorMsg
				( "naked モードで object クラスのメンバ変数の参照は出来ません" ) ;
		}
		long int	nVarIndex =
			pClassInf->GetVariableIndex( wstrMember ) ;
		ECSTypeInfo *
			pVarType = pClassInf->GetVariableAt( nVarIndex ) ;
		if ( pVarType != NULL )
		{
			const DWORD	dwVarAccess = pVarType->GetProtectedAttribute() ;
			if ( (dwVarAccess > (typeSrc.m_dwFlags & ECSTypeInfo::flagProtectedMask))
				&& (dwVarAccess > GetAccessClassTo(pClassInf)) )
			{
				if ( pVarType->GetProtectedAttribute()
									== ECSTypeInfo::flagPrivate )
				{
					return	ESLErrorMsg
						( "private なメンバ変数へアクセスできません。" ) ;
				}
				return	ESLErrorMsg
					( "protected なメンバ変数へアクセスできません。" ) ;
			}
			if ( pClassInf->GetAttribute() & ECSTypeInfo::flagNakedBuffer )
			{
				const DWORD	dwOffset =
					pClassInf->GetVariableNakedOffsetAt( nVarIndex ) ;
				if ( m_modeNakedCode )
				{
					ECSTypeInfo	typeSrcTemp = typeSrc ;
					if ( typeSrcTemp.IsTypeReference2() )
					{
						CompileCodeNakedUncoverReference( typeSrcTemp ) ;
					}
					typeDst.MakeReferenceOf( *pVarType ) ;
					typeDst.m_dwFlags &=
							~(ECSTypeInfo::flagDeterministic
										| ECSTypeInfo::flagProtectedMask) ;
					typeDst.m_dwFlags |=
						ECSTypeInfo::flagNakedBuffer
							| (typeSrcTemp.m_dwFlags & ECSTypeInfo::flagConstant) ;
					//
					typeDst.ClearLoadedRegister() ;
					typeDst.SetLoadedRegister( typeSrcTemp ) ;
					if ( typeSrcTemp.IsAddressingInfo() )
					{
						typeDst.SetAddressingInfo( typeSrcTemp ) ;
						typeDst.m_addrOffset += dwOffset ;
					}
					else
					{
						int	regExpr = typeSrcTemp.IsLoadedRegister()
										? typeSrcTemp.GetLoadedRegister()
												: GetExpressionRegister() ;
						typeDst.SetLoadedRegister( regExpr ) ;
						//
						m_pcsxi->WriteSakuraAddRegRegImm32
									( regExpr, regExpr, dwOffset ) ;
					}
					return	eslErrSuccess ;
				}
				else
				{
					CompileCodePointerToObject( dwOffset ) ;
					//
					return	CompileReferenceMemberPointer( typeDst, *pVarType ) ;
				}
			}
			else if ( typeSrc.m_dwFlags & ECSTypeInfo::flagNakedBuffer )
			{
				return	ESLErrorMsg( "naked メモリへの不正なメンバ参照です" ) ;
			}
			else if ( pClassInf->GetAttribute()
						& ECSTypeInfo::flagNativeObject )
			{
				m_pcsxi->WriteInstructionCode( csicElement ) ;
				m_pcsxi->WriteVariableTypeCode( csvtString ) ;
				m_pcsxi->WriteConstantString( wstrMember ) ;
			}
			else
			{
				m_pcsxi->WriteInstructionCode( csicElement ) ;
				m_pcsxi->WriteVariableTypeCode( csvtInteger ) ;
				m_pcsxi->WriteCodeData
						( &nVarIndex, sizeof(nVarIndex) ) ;
			}
			DWORD	fdwConst =
				typeDst.m_dwFlags & ECSTypeInfo::flagConstant ;
			typeDst.MakeReferenceOf( *pVarType ) ;
			typeDst.m_dwFlags |= fdwConst ;
			typeDst.m_dwFlags &=
					~(ECSTypeInfo::flagDeterministic
								| ECSTypeInfo::flagProtectedMask) ;
		}
		else if ( m_modeNakedCode || pClassInf->IsNakedMemoryClass()
				|| (typeSrc.m_dwFlags & ECSTypeInfo::flagNakedBuffer) )
		{
			return	ESLErrorMsg
				( "naked メモリ上のメンバ参照でメンバ変数情報が見つかりません" ) ;
		}
		else if ( pClassInf->IsBasicType() == csvtHash )
		{
			m_pcsxi->WriteInstructionCode( csicElement ) ;
			m_pcsxi->WriteVariableTypeCode( csvtString ) ;
			m_pcsxi->WriteConstantString( wstrMember ) ;
			//
			ECSObject *	pTypeValue = typeSrc.GetNakedType() ;
			if ( (pTypeValue != NULL)
				&& (pTypeValue->m_vtType == csvtHash) )
			{
				if ( ((ECSHash*)pTypeValue)->m_pDefObj != NULL )
				{
					ECSReference *	pRefElement = new ECSReference ;
					pRefElement->SetOwnObject
						( ECSTypeInfo::DuplicateType
							( ((ECSHash*)pTypeValue)->m_pDefObj ) ) ;
					typeDst.SetTypeValue
						( pRefElement,
							(typeSrc.m_dwFlags
									& ECSTypeInfo::flagConstant) ) ;
				}
				else
				{
					typeDst.SetTypeValue( new ECSReference, 0 ) ;
				}
			}
			else
			{
				typeDst.SetTypeValue( new ECSReference, 0 ) ;
			}
		}
		else if ( wstrMember == L"parent" )
		{
			if ( pClassInf->GetParentClassCount() == 0 )
			{
				m_strErrMsg =
					EString( pClassInf->GetGlobalName() )
							+ " の親クラスはありません。" ;
				return	ESLErrorMsg( m_strErrMsg ) ;
			}
			if ( pClassInf->GetParentClassCount() > 1 )
			{
				m_strErrMsg =
					EString( pClassInf->GetGlobalName() )
							+ " は複数の親クラスがあります。" ;
				return	ESLErrorMsg( m_strErrMsg ) ;
			}
			ECSClassInfo::ParentClass *
				pParentClass = pClassInf->GetParentClassAt( 0 ) ;
			if ( (pParentClass == NULL)
				|| (pParentClass->pClassInf == NULL) )
			{
				m_strErrMsg =
					EString( pClassInf->GetGlobalName() )
							+ " の親クラスが見つかりません。" ;
				return	ESLErrorMsg( m_strErrMsg ) ;
			}
			const ECSClassInfo::CastInfo *
				pParentCast =
					pClassInf->GetCastClassInfoAs
						( pParentClass->pClassInf->GetGlobalName() ) ;
			if ( pParentCast == NULL )
			{
				m_strErrMsg =
					EString( pClassInf->GetGlobalName() )
							+ " の親クラスキャストが見つかりません。" ;
				return	ESLErrorMsg( m_strErrMsg ) ;
			}
			ECSTypeInfo	typeAddressing ;
			ESLError	err =
				CompileCastToParentClass
					( typeAddressing, *pParentCast,
						*pClassInf, typeSrc,
						(ECSTypeInfo::Flags)
							(typeSrc.m_dwFlags
								& ECSTypeInfo::flagProtectedMask) ) ;
			if ( err )
			{
				return	err ;
			}
			ECSReference *	pRefParent = new ECSReference ;
			pRefParent->SetOwnObject
				( new ECSStructure( pParentClass->pClassInf ) ) ;
			typeDst.SetTypeValue
				( pRefParent,
					(typeSrc.m_dwFlags & ECSTypeInfo::flagConstant) ) ;
			//
			typeDst.MoveRegisterAndAddressingFrom( typeAddressing ) ;
		}
		else
		{
			m_strErrMsg =
				EString( pClassInf->GetGlobalName() )
					+ " のメンバに " + EString( wstrMember )
					+ " 変数が見つかりません。" ;
			return	ESLErrorMsg( m_strErrMsg ) ;
		}
	}
	else if ( m_modeNakedCode
			|| (typeSrc.m_dwFlags & ECSTypeInfo::flagNakedBuffer) )
	{
		return	ESLErrorMsg
			( "naked メモリ上のメンバ参照でクラス情報が見つかりません" ) ;
	}
	else
	{
		//
		// メンバ変数参照
		//
		if ( m_dwModeFlags & flagStrictStyle )
		{
			ESLError	err =
				OutputWarning1
					( "抽象 Reference のメンバ変数を参照しようとしています" ) ;
			if ( err )
			{
				return	err ;
			}
		}
		m_pcsxi->WriteInstructionCode( csicElement ) ;
		m_pcsxi->WriteVariableTypeCode( csvtString ) ;
		m_pcsxi->WriteConstantString( wstrMember ) ;
		//
		typeDst.SetTypeValue( new ECSReference, 0 ) ;
	}
	return	eslErrSuccess ;
}

// ポインタメンバ変数参照
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompilePointerTypeMemberVariable
	( ECSTypeInfo & typeDst,
		const ECSTypeInfo & typeSrc, const EWideString & wstrMember )
{
	const ECSClassInfo *
		pClassInf = GetNakedPtrTypeClassInfo( typeSrc ) ;
	if ( pClassInf == NULL )
	{
		return	ESLErrorMsg( "\'->\' ポインタ参照でクラス情報が見つかりません" ) ;
	}
	if ( !(pClassInf->GetAttribute() & ECSTypeInfo::flagNakedBuffer) )
	{
		if ( m_modeNakedCode )
		{
			return	ESLErrorMsg
				( "naked モードで naked でないクラスのメンバを参照しています" ) ;
		}
		ECSTypeInfo	typeThis ;
		ESLError	err = CompileReferencePointer( typeThis, typeSrc ) ;
		if ( err )
		{
			return	err ;
		}
		NormalizePointerFromLinearAddress( typeThis ) ;
		return	CompileTypeMemberVariable( typeDst, typeThis, wstrMember ) ;
	}
	if ( m_modeNakedCode )
	{
		ECSTypeInfo	typeSrcTemp = typeSrc ;
		while ( typeSrcTemp.IsTypeReference() )
		{
			CompileCodeNakedUncoverReference( typeSrcTemp ) ;
		}
		ECSTypeInfo	typeTemp ;
		typeTemp.MakeNakedPointerOf( typeSrcTemp ) ;
		//
		ECSTypeInfo	typeSrcRef ;
		typeSrcRef.MakeReferenceOf( typeTemp ) ;
		typeSrcRef.MoveRegisterAndAddressingFrom( typeSrcTemp ) ;
		//
		return	CompileTypeMemberVariable( typeDst, typeSrcRef, wstrMember ) ;
	}
	//
	// メンバ変数参照
	//
	long int	nVarIndex =
		pClassInf->GetVariableIndex( wstrMember ) ;
	ECSTypeInfo *
		pVarType = pClassInf->GetVariableAt( nVarIndex ) ;
	if ( pVarType == NULL )
	{
		m_strErrMsg =
			EString( pClassInf->GetGlobalName() )
				+ " のメンバに " + EString( wstrMember )
				+ " 変数が見つかりません。" ;
		return	ESLErrorMsg( m_strErrMsg ) ;
	}
	const DWORD	dwVarAccess = pVarType->GetProtectedAttribute() ;
	if ( (dwVarAccess > (typeSrc.m_dwFlags & ECSTypeInfo::flagProtectedMask))
					&& (dwVarAccess > GetAccessClassTo(pClassInf)) )
	{
		if ( pVarType->GetProtectedAttribute()
							== ECSTypeInfo::flagPrivate )
		{
			return	ESLErrorMsg
				( "private なメンバ変数へアクセスできません。" ) ;
		}
		return	ESLErrorMsg
			( "protected なメンバ変数へアクセスできません。" ) ;
	}
	//
	// ポインタを移動
	//
	int	iOffset = pClassInf->GetVariableNakedOffsetAt( nVarIndex ) ;
	CompileImmediateInteger( iOffset ) ;
	CompileCodeOperate( csotAdd ) ;
	//
	// ポインタ参照
	//
	return	CompileReferenceMemberPointer( typeDst, *pVarType ) ;
}

// メンバ関数ポインタ参照 operator .* , operator ->*
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileTypeMemberFunctionPointer
	( ECSTypeInfo & typeDst,
		const ECSTypeInfo & typeThisSrc, OperatorType optype,
		ECSSourceStream & cssLine, DWORD dwFlags,
		int nPriority, const wchar_t * pwszExit )
{
	if ( !m_modeNakedCode )
	{
		return	ESLErrorMsg
			( "object モードで関数の間接呼び出しはできません" ) ;
	}
	//
	// this アドレス確定
	//
	ESLError	err ;
	ECSTypeInfo	typeThis ;
	if ( optype == optPtrMemberFunc )
	{
		err = CompileReferencePointer( typeThis, typeThisSrc ) ;
		if ( err )
		{
			return	err ;
		}
		NormalizePointerFromLinearAddress( typeThis ) ;
	}
	else
	{
		typeThis = typeThisSrc ;
	}
	while ( typeThis.IsTypeReference2() )
	{
		CompileCodeNakedUncoverReference( typeThis ) ;
	}
	MakeCommitValueToNakedRegister( typeThis ) ;
	//
	int	regFuncPtr = typeThis.GetLoadedRegister() ;
	int	regThis = AllocateExpressionRegister() ;
	m_pcsxi->WriteSakuraMoveRegReg
		( regThis, typeThis.GetLoadedRegister() ) ;
	typeThis.SetLoadedRegister( regThis ) ;
	//
	// 関数ポインタ確定
	//
	ECSTypeInfo	typeFuncPtr ;
	err = CompileExpression
		( typeFuncPtr, cssLine, dwFlags, nPriority, pwszExit ) ;
	if ( err )
	{
		return	err ;
	}
	while ( typeFuncPtr.IsTypeReference() )
	{
		CompileCodeNakedUncoverReference( typeFuncPtr ) ;
	}
	MakeCommitValueToNakedRegister( typeFuncPtr ) ;
	//
	m_pcsxi->WriteSakuraMoveRegReg
		( regFuncPtr, typeFuncPtr.GetLoadedRegister() ) ;
	typeFuncPtr.SetLoadedRegister( regFuncPtr ) ;
	//
	FreeExpressionRegister() ;
	//
	// 関数ポインタ型判定
	//
	ECSFunction *	pFuncPtr = typeFuncPtr.GetTypeFunctionPointer() ;
	if ( pFuncPtr == NULL )
	{
		if ( optype == optPtrMemberFunc )
		{
			return	ESLErrorMsg
				( "\'->*\' 演算子の右項が関数ポインタではありません" ) ;
		}
		else
		{
			return	ESLErrorMsg
				( "\'.*\' 演算子の右項が関数ポインタではありません" ) ;
		}
	}
	if ( pFuncPtr->m_pThisCall == NULL )
	{
		return	ESLErrorMsg( "thiscall でない関数ポインタです" ) ;
	}
	const ECSClassInfo *
			pThisType = GetNakedTypeClassInfo( typeThis ) ;
	if ( pThisType == NULL )
	{
		return	ESLErrorMsg
			( "thiscall のクラス情報を取得できませんでした" ) ;
	}
	if ( pThisType->GetGlobalName()
				!= pFuncPtr->m_pThisCall->GetGlobalName() )
	{
		ECSClassInfo::CastInfo * pCast =
			pThisType->GetCastClassInfoAs
					( pFuncPtr->m_pThisCall->GetGlobalName() ) ;
		if ( pCast != NULL )
		{
			if ( pCast->nNakedOffset != 0 )
			{
				m_pcsxi->WriteSakuraAddRegRegImm32
					( regThis, regThis, pCast->nNakedOffset ) ;
			}
		}
		else
		{
			return	ESLErrorMsg( "thiscall のクラスが一致しません" ) ;
		}
	}
	//
	// 評価型設定
	//
	typeDst = typeFuncPtr ;
	typeDst.SetLoadedRegister( regFuncPtr, regThis ) ;
	return	eslErrSuccess ;
}

// 要素参照 operator []
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileTypeReferenceElement
	( ECSTypeInfo & typeDst,
		const ECSTypeInfo & typeSrc, const ECSTypeInfo & typeOrgIndex )
{
	if ( typeSrc.IsVoid() || typeOrgIndex.IsVoid() )
	{
		return	ESLErrorMsg( "不正な要素参照です。" ) ;
	}
	ECSTypeInfo	typeIndex = typeOrgIndex ;
	if ( m_modeNakedCode )
	{
		while ( typeIndex.IsTypeReference() )
		{
			CompileCodeNakedUncoverReference( typeIndex ) ;
		}
		CompileCodeNakedUncoverEnumerator( typeIndex ) ;
	}
	//
	ECSObject *	pType = typeSrc.GetNakedType() ;
	ECSObject *	pIndex = typeIndex.GetNakedType() ;
	ECSObject *	pElementType = NULL ;
	if ( pType != NULL )
	{
		bool	fElement = false ;
		if ( pType->m_vtType == csvtPointer )
		{
			//
			// ポインタ型処理
			//
			if ( !typeIndex.IsAbstractType()
				&& ((pIndex == NULL)
					|| (pIndex->m_vtType != csvtInteger)) )
			{
				return	ESLErrorMsg
					( "ポインタへの指標が Integer ではありません。" ) ;
			}
			ECSObject *	pPtrType = typeSrc.GetNakedPointerType() ;
			if ( pPtrType == NULL )
			{
				return	ESLErrorMsg( "void ポインタを参照しています" ) ;
			}
			int	nNakedSize, nObjCount ;
			ESLError	err ;
			err = ECSTypeInfo::GetNakedMemorySize
						( nNakedSize, nObjCount, pPtrType ) ;
			if ( err )
			{
				return	err ;
			}
			if ( m_modeNakedCode )
			{
				ECSTypeInfo	typeDstTemp ;
				err = CompileTypeNakedPointerAdd
					( typeDstTemp, typeSrc, typeIndex, nNakedSize ) ;
				if ( err )
				{
					return	err ;
				}
				return	CompileReferencePointer( typeDst, typeDstTemp ) ;
			}
			else
			{
				if ( nNakedSize != 1 )
				{
					CompileImmediateInteger( nNakedSize ) ;
					CompileCodeOperate( csotMul ) ;
				}
				CompileCodeOperate( csotAdd ) ;
				return	CompileReferencePointer( typeDst, typeSrc ) ;
			}
		}
		else if ( pType->m_vtType == csvtArray )
		{
			//
			// 配列型処理
			//
			pElementType = ((ECSArray*)pType)->m_pDefObj ;
			fElement = true ;
			//
			if ( !typeIndex.IsAbstractType()
				&& ((pIndex == NULL)
					|| (pIndex->m_vtType != csvtInteger)) )
			{
				return	ESLErrorMsg
					( "Array への指標が Integer ではありません。" ) ;
			}
			if ( typeSrc.m_dwFlags
						& ECSTypeInfo::flagDeterministic )
			{
				if ( ((ECSInteger*)pIndex)->GetValue()
							>= ((ECSArray*)pType)->GetBounds() )
				{
					return	ESLErrorMsg( "配列の指標が範囲外です。" ) ;
				}
			}
			if ( m_modeNakedCode )
			{
				//
				// naked 配列
				//
				if ( !(typeSrc.m_dwFlags & ECSTypeInfo::flagNakedBuffer) )
				{
					return	ESLErrorMsg
						( "naked モードで Array 要素を参照できません" ) ;
				}
				if ( pElementType == NULL )
				{
					OutputWarning
						( "void 配列を参照しています",
								m_strFilePath, m_nLineNum ) ;
				}
				ECSTypeInfo	typeDstTemp ;
				ECSTypeInfo	typeSrcPtr ;
				typeSrcPtr.MakePointerOf
					( ECSTypeInfo
						( ECSTypeInfo::DuplicateType
							( pElementType ), typeSrc.m_dwFlags ) ) ;
				typeSrcPtr.MoveRegisterAndAddressingFrom( typeSrc ) ;
				//
				int	nNakedSize, nObjCount ;
				ESLError	err ;
				err = ECSTypeInfo::GetNakedMemorySize
							( nNakedSize, nObjCount, pElementType ) ;
				if ( err )
				{
					return	err ;
				}
				err = CompileTypeNakedPointerAdd
					( typeDstTemp, typeSrcPtr, typeIndex, nNakedSize ) ;
				if ( err )
				{
					return	err ;
				}
				return	CompileReferencePointer( typeDst, typeDstTemp ) ;
			}
		}
		else if ( pType->m_vtType == csvtHash )
		{
			//
			// ハッシュ配列型処理
			//
			pElementType = ((ECSHash*)pType)->m_pDefObj ;
			fElement = true ;
			//
			if ( !typeIndex.IsAbstractType()
				&& ((pIndex == NULL)
					|| ((pIndex->m_vtType != csvtInteger)
						&& (pIndex->m_vtType != csvtString))) )
			{
				return	ESLErrorMsg
					( "Hash への指標が Integer/String "
								"いずれでもありません。" ) ;
			}
			if ( m_modeNakedCode )
			{
				return	ESLErrorMsg
					( "naked モードで Hash 要素を参照できません" ) ;
			}
		}
		else if ( !EWideString::Compare( pType->GetTypeName(), L"Global" ) )
		do
		{
			//
			// Data テーブル
			//
			if ( m_modeNakedCode )
			{
				return	ESLErrorMsg( "naked モードで object 要素参照です" ) ;
			}
			ECSGlobal *	pgDataType = ESLTypeCast<ECSGlobal>( pType ) ;
			if ( (pgDataType != NULL)
				&& (pgDataType->m_pDefObj != NULL) )
			{
				pElementType = pgDataType->m_pDefObj ;
				fElement = true ;
				//
				if ( !typeIndex.IsAbstractType()
					&& ((pIndex == NULL)
						|| ((pIndex->m_vtType != csvtInteger)
							&& (pIndex->m_vtType != csvtString))) )
				{
					return	ESLErrorMsg
						( "Data テーブルへの指標が "
							"Integer/String いずれでもありません。" ) ;
				}
				break ;
			}
			ECSStructure *	psDataType = ESLTypeCast<ECSStructure>( pType ) ;
			if ( (psDataType != NULL)
				&& (psDataType->m_pDefObj != NULL) )
			{
				pElementType = psDataType->m_pDefObj ;
				fElement = true ;
				//
				if ( !typeIndex.IsAbstractType()
					&& ((pIndex == NULL)
						|| ((pIndex->m_vtType != csvtInteger)
							&& (pIndex->m_vtType != csvtString))) )
				{
					return	ESLErrorMsg
						( "Data テーブルへの指標が "
							"Integer/String いずれでもありません。" ) ;
				}
				break ;
			}
		}
		while ( false ) ;
		//
		if ( fElement )
		{
			if ( pElementType == NULL )
			{
				typeDst.SetTypeValue
					( new ECSReference, typeSrc.m_dwFlags ) ;
			}
			else
			{
				ECSReference *	pRefType = new ECSReference ;
				pRefType->SetOwnObject
					( ECSTypeInfo::DuplicateType( pElementType ) ) ;
				typeDst.SetTypeValue( pRefType, typeSrc.m_dwFlags ) ;
			}
			m_pcsxi->WriteInstructionCode( csicElementIndirect ) ;
			return	eslErrSuccess ;
		}
	}
	//
	// その他汎用処理
	//
	const ECSClassInfo *
		pClassInf = GetNakedTypeClassInfo( typeSrc ) ;
	if ( (pClassInf != NULL)
		&& (pClassInf->IsBasicType() == csvtObject) )
	{
		ECSClassInfo::ListMemberFunction	lstFunc ;
		EObjArray<ECSTypeInfo>				lstArg ;
		lstArg.Add( new ECSTypeInfo( typeIndex ) ) ;
		//
		if ( pClassInf->SearchFunctinoAs
			( lstFunc, L"operator []", lstArg,
				(typeSrc.m_dwFlags & ECSTypeInfo::flagConstant),
				false, false, m_modeNakedCode ) )
		{
			typeDst = lstFunc[0].GetReturnType() ;
			//
			if ( pClassInf->GetAttribute()
						& ECSTypeInfo::flagNativeObject )
			{
				if ( m_modeNakedCode )
				{
					return	ESLErrorMsg
						( "naked モードで native オブジェクト"
									"の要素を参照しています" ) ;
				}
				m_pcsxi->WriteInstructionCode( csicElementIndirect ) ;
				return	eslErrSuccess ;
			}
			else
			{
				ECSTypeInfo	typeThis = typeSrc ;
				if ( m_modeNakedCode )
				{
					while ( typeThis.IsTypeReference2() )
					{
						CompileCodeNakedUncoverReference( typeThis ) ;
					}
				}
				LoadCommitValueToNakedRegister
						( AllocateExpressionRegister(), typeThis ) ;
				LoadCommitValueToNakedRegister
						( AllocateExpressionRegister(), typeIndex ) ;
				//
				ESLError	err = CompileCallMemberFunction
					( *pClassInf, typeSrc, lstFunc[0],
						lstArg, 1, ECSTypeInfo::flagPublic ) ;
				int	regReturn = GetExpressionRegister() ;
				//
				FreeExpressionRegister( typeIndex ) ;
				FreeExpressionRegister( typeThis ) ;
				//
				if ( m_modeNakedCode )
				{
					if ( regReturn != GetExpressionRegister() )
					{
						m_pcsxi->WriteSakuraMoveRegReg
								( GetExpressionRegister(), regReturn ) ;
					}
					GetFunctionReturnType( typeDst, lstFunc[0] ) ;
//					typeDst.SetLoadedRegister( GetExpressionRegister() ) ;
				}
				return	err ;
			}
		}
		else if ( lstFunc.GetSize() >= 2 )
		{
			return	ESLErrorMsg
				( "複数の適合する [] 演算子オーバーロードが存在します。" ) ;
		}
		else if ( !m_modeNakedCode
			&& (pClassInf->GetAttribute() & ECSTypeInfo::flagStructure) )
		{
			m_pcsxi->WriteInstructionCode( csicElementIndirect ) ;
			//
			if ( (typeIndex.m_dwFlags & ECSTypeInfo::flagDeterministic)
				&& (pIndex != NULL) && (pIndex->m_vtType == csvtInteger) )
			{
				if ( ((ECSInteger*)pIndex)->GetValue()
								>= pClassInf->GetVariableCount() )
				{
					return	ESLErrorMsg( "構造体への指標が範囲外です。" ) ;
				}
				ECSTypeInfo *	pVarType =
					pClassInf->GetVariableAt
						( (int) ((ECSInteger*)pIndex)->GetValue() ) ;
				if ( pVarType != NULL )
				{
					typeDst.MakeReferenceOf( *pVarType ) ;
					typeDst.m_dwFlags &= ~ECSTypeInfo::flagProtectedMask ;
					return	eslErrSuccess ;
				}
			}
			typeDst = ECSTypeInfo( new ECSReference ) ;
			return	eslErrSuccess ;
		}
	}
	else if ( !m_modeNakedCode
			&& (!(m_dwModeFlags & flagStrictStyle)
						|| typeSrc.IsAbstractType()) )
	{
		m_pcsxi->WriteInstructionCode( csicElementIndirect ) ;
		//
		typeDst.SetTypeValue
			( new ECSReference, typeSrc.m_dwFlags ) ;
		return	eslErrSuccess ;
	}
	return	ESLErrorMsg( "不正な要素参照です。" ) ;
}

// 型単項演算
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileTypeUnaryOperate
	( ECSTypeInfo & typeDst,
		const ECSTypeInfo & typeSrc, CSUnaryOperatorType uoptUnary )
{
	if ( !typeSrc.IsVoid() )
	{
		const ECSClassInfo *
			pClassInf = GetNakedTypeClassInfo( typeSrc ) ;
		bool	fDstType = false ;
		if ( pClassInf != NULL )
		{
			ECSClassInfo::ListMemberFunction	lstFunc ;
			EPtrObjArray<ECSTypeInfo>			lstArg ;
			ECSTypeInfo							typeArgDef ;
			if ( (uoptUnary == csuotIncrementAfter)
				|| (uoptUnary == csuotDecrementAfter) )
			{
				typeArgDef.SetTypeValue( new ECSInteger, 0 ) ;
				lstArg.Add( &typeArgDef ) ; 
			}
			bool	fOperator =
				pClassInf->SearchUnaryOperatorAs
					( lstFunc, uoptUnary, lstArg, m_modeNakedCode ) ;
			if ( !fOperator )
			{
				if ( lstFunc.GetSize() >= 2 )
				{
					if ( m_dwModeFlags & flagStrictStyle )
					{
						MakeAmbiguousWarningMsg2
							( "複数の適合する単項演算子オーバーロードが存在します",
														&lstArg, &lstFunc ) ;
					}
					fOperator = true ;
				}
			}
			if ( fOperator )
			{
				typeDst = lstFunc[0].GetReturnType() ;
				fDstType = true ;
				//
				if ( !(pClassInf->GetAttribute()
							& ECSTypeInfo::flagNativeObject) )
				{
					ESLError	err ;
					DWORD		dwArgCount = 0 ;
					ECSTypeInfo	typeThis = typeSrc ;
					if ( m_modeNakedCode )
					{
						/*
						LoadCommitValueToNakedRegister
							( AllocateExpressionRegister(), typeSrc ) ;
						*/
						EObjArray<ECSTypeInfo>	lstArgType ;
						lstArgType[0] = typeSrc ;
						//
						err = NormalizeNakedArgument
								( lstFunc.GetAt(0), lstArgType, dwArgCount ) ;
						if ( err )
						{
							return	err ;
						}
						typeThis = lstArgType[0] ;
					}
					err = CompileCallMemberFunction
							( *pClassInf, typeThis, lstFunc[0],
									lstArg, dwArgCount, ECSTypeInfo::flagPublic ) ;
					int	regReturn = GetExpressionRegister() ;
					//
					FreeExpressionRegister( typeSrc ) ;
					//
					if ( !typeDst.IsVoid() && m_modeNakedCode )
					{
						if ( regReturn != GetExpressionRegister() )
						{
							m_pcsxi->WriteSakuraMoveRegReg
									( GetExpressionRegister(), regReturn ) ;
						}
						GetFunctionReturnType( typeDst, lstFunc[0] ) ;
//						typeDst.SetLoadedRegister( GetExpressionRegister() ) ;
					}
					return	err ;
				}
			}
		}
		else if ( m_modeNakedCode && !typeSrc.IsTypePointer() )
		{
			return	ESLErrorMsg
				( "naked モードでクラス情報の見つからない単項演算子です" ) ;
		}
		if ( (uoptUnary == csuotIncrement)
			|| (uoptUnary == csuotDecrement)
			|| (uoptUnary == csuotIncrementAfter)
			|| (uoptUnary == csuotDecrementAfter) )
		{
			//
			// インクリメント・デクリメント演算子
			//
			if ( !typeSrc.IsTypeReference() )
			{
				return	ESLErrorMsg
					( "インクリメント・デクリメント演算子が"
						"左辺値として不正な型に指定されています" ) ;
			}
			bool	fAfterOp = (uoptUnary == csuotIncrementAfter)
								|| (uoptUnary == csuotDecrementAfter) ;
			int		regExpr = -1, regValue = -1 ;
			if ( m_modeNakedCode )
			{
				regExpr = AllocateExpressionRegister() ;
				regValue = regExpr ;
				WriteSakuraLoadMemory( regValue, typeSrc ) ;
				if ( fAfterOp && (regExpr != regValue) )
				{
					m_pcsxi->WriteSakuraMoveRegReg( regExpr, regValue ) ;
					regValue = regExpr ;
				}
			}
			if ( (uoptUnary == csuotIncrementAfter)
				|| (uoptUnary == csuotDecrementAfter) )
			{
				if ( !m_modeNakedCode )
				{
					CompileLoadStackObject( 0 ) ;
					//
					m_pcsxi->WriteInstructionCode( csicExUniOperate ) ;
					m_pcsxi->WriteExUniOperatorTypeCode( csxuotDuplicate ) ;
					//
					CompileCodeSwap( 0, 1 ) ;
				}
			}
			int	nPitch ;
			if ( typeSrc.IsTypePointer() )
			{
				int	nObjCount ;
				ESLError	err =
					ECSTypeInfo::GetNakedMemorySize
						( nPitch, nObjCount, typeSrc.GetNakedPointerType() ) ;
				if ( err )
				{
					return	err ;
				}
				if ( (uoptUnary == csuotDecrement)
					|| (uoptUnary == csuotDecrementAfter) )
				{
					nPitch = - nPitch ;
				}
			}
			else
			{
				nPitch = -1 ;
				if ( (uoptUnary == csuotIncrement)
					|| (uoptUnary == csuotIncrementAfter) )
				{
					nPitch = 1 ;
				}
			}
			if ( m_modeNakedCode )
			{
				int	regTemp = regExpr ;
				if ( fAfterOp )
				{
					regTemp = AllocateExpressionRegister() ;
				}
				m_pcsxi->WriteSakuraAddRegRegImm32
							( regTemp, regValue, nPitch ) ;
				WriteSakuraStoreMemory( regTemp, typeSrc ) ;
				//
				if ( fAfterOp )
				{
					FreeExpressionRegister() ;
					FreeExpressionRegister() ;
					FreeExpressionRegister( typeSrc ) ;
					//
					regExpr = AllocateExpressionRegister() ;
					if ( regExpr != regValue )
					{
						m_pcsxi->WriteSakuraMoveRegReg( regExpr, regValue ) ;
					}
				}
				else
				{
					FreeExpressionRegister() ;
					FreeExpressionRegister( typeSrc ) ;
					//
					regExpr = AllocateExpressionRegister() ;
					if ( regExpr != regTemp )
					{
						m_pcsxi->WriteSakuraMoveRegReg( regExpr, regTemp ) ;
					}
				}
				typeDst.MakeNakedOf( typeSrc ) ;
				typeDst.SetLoadedRegister( regExpr ) ;
				return	eslErrSuccess ;
			}
			else
			{
				CompileImmediateInteger( nPitch ) ;
				CompileCodeStore( csotAdd ) ;
				if ( fAfterOp )
				{
					CompileCodeFreeStack() ;
				}
			}
		}
		else
		{
			//
			// 通常の単項演算子
			//
			if ( m_modeNakedCode )
			{
				return	CompileTypeNakedUnaryOperate
								( typeDst, typeSrc, uoptUnary ) ;
			}
			m_pcsxi->WriteInstructionCode( csicUniOperate ) ;
			m_pcsxi->WriteUniOperatorTypeCode( uoptUnary ) ;
		}
		if ( !fDstType )
		{
			if ( uoptUnary == csuotLogicalNot )
			{
				typeDst.SetTypeValue
					( new ECSInteger( 0, ECSInteger::m_maskBoolean ), 0 ) ;
			}
			else
			{
				typeDst = typeSrc ;
			}
		}
		return	eslErrSuccess ;
	}
	return	ESLErrorMsg( "不正な単項演算子です。" ) ;
}

ESLError ECSCompiler::CompileTypeNakedUnaryOperate
	( ECSTypeInfo & typeDst,
		const ECSTypeInfo & typeSrc, CSUnaryOperatorType uoptUnary )
{
	ESLAssert( m_modeNakedCode ) ;
	ECSTypeInfo	typeSrcTemp = typeSrc ;
	while ( typeSrcTemp.IsTypeReference() )
	{
		CompileCodeNakedUncoverReference( typeSrcTemp ) ;
	}
	CompileCodeNakedUncoverEnumerator( typeSrcTemp ) ;
	//
	if ( !typeSrcTemp.IsLoadedRegister() )
	{
		//
		// 定数計算
		//
		if ( typeSrc.m_pValue == NULL )
		{
			return	ESLErrorMsg( "void への単項演算子です" ) ;
		}
		if ( typeSrc.m_pValue->m_vtType == csvtInteger )
		{
			((ECSInteger*)typeSrc.m_pValue)->
					SetValueMask( ECSInteger::m_maskInt64 ) ;
		}
		ESLError	err =
			typeSrc.m_pValue->UnaryOperate( m_ctxExpr, uoptUnary ) ;
		if ( err )
		{
			return	err ;
		}
		ECSObject *	pResult = typeSrc.m_pValue ;
		if ( pResult->m_pResult != NULL )
		{
			pResult = pResult->m_pResult ;
		}
		typeDst.SetTypeValue
			( ECSTypeInfo::DuplicateType( pResult ),
							ECSTypeInfo::flagDeterministic ) ;
		typeDst.NormalzieImmediateIntegerType() ;
		return	eslErrSuccess ;
	}
	else if ( typeSrcTemp.IsTypeInteger() )
	{
		const int	regExpr = typeSrcTemp.GetLoadedRegister() ;
		typeDst.MakeNakedOf( typeSrcTemp ) ;
		switch ( uoptUnary )
		{
		case	csuotPlus:
			break ;
		case	csuotNegate:
			m_pcsxi->WriteSakuraOperandReg
				( ECSSakura2Processor::codeNegInt, regExpr ) ;
			break ;
		case	csuotBitNot:
			m_pcsxi->WriteSakuraOperandReg
				( ECSSakura2Processor::codeNotInt, regExpr ) ;
			break ;
		case	csuotLogicalNot:
			m_pcsxi->WriteSakuraCmpEqRegReg
				( regExpr, ECSSakura2Processor::regIntZero ) ;
			typeDst.SetTypeValue
				( new ECSInteger( 0, ECSInteger::m_maskBoolean ), 0 ) ;
			break ;
		case	csuotIncrement:
			m_pcsxi->WriteSakuraAddRegRegImm32( regExpr, regExpr, 1 ) ;
			break ;
		case	csuotDecrement:
			m_pcsxi->WriteSakuraAddRegRegImm32( regExpr, regExpr, -1 ) ;
			break ;
		default:
			return	ESLErrorMsg( "整数型に未定義の単項演算子です。" ) ;
		}
		typeDst.SetLoadedRegister( regExpr ) ;
		return	eslErrSuccess ;
	}
	else if ( typeSrcTemp.IsTypeReal() )
	{
		const int	regExpr = typeSrcTemp.GetLoadedRegister() ;
		typeDst.MakeNakedOf( typeSrcTemp ) ;
		switch ( uoptUnary )
		{
		case	csuotPlus:
			break ;
		case	csuotNegate:
			m_pcsxi->WriteSakuraOperandReg
				( ECSSakura2Processor::codeNegFloat, regExpr ) ;
			break ;
		default:
			return	ESLErrorMsg( "実数型に未定義の単項演算子です。" ) ;
		}
		typeDst.SetLoadedRegister( regExpr ) ;
		return	eslErrSuccess ;
	}
	else if ( typeSrcTemp.IsTypePointer() )
	{
		const int	regExpr = typeSrcTemp.GetLoadedRegister() ;
		typeDst.MakeNakedOf( typeSrcTemp ) ;
		switch ( uoptUnary )
		{
		case	csuotLogicalNot:
			m_pcsxi->WriteSakuraCmpEqRegReg
				( regExpr, ECSSakura2Processor::regIntZero ) ;
			typeDst.SetTypeValue
				( new ECSInteger( 0, ECSInteger::m_maskBoolean ), 0 ) ;
			break ;
		default:
			return	ESLErrorMsg( "実数型に未定義の単項演算子です。" ) ;
		}
		typeDst.SetLoadedRegister( regExpr ) ;
		return	eslErrSuccess ;
	}
	return	ESLErrorMsg
		( "naked モードで定義されない単項演算子です" ) ;
}

// 特殊型単項演算
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileTypeExUnaryOperate
	( ECSTypeInfo & typeDst,
		const ECSTypeInfo & typeSrc,
		CSExtraUniOperatorType xuoptExUnary )
{
	if ( typeSrc.IsVoid() )
	{
		return	ESLErrorMsg( "不正な void への単項演算子です。" ) ;
	}
	if ( typeSrc.IsTypePointer() )
	{
		return	CompileTypePointerExUnaryOperate
					( typeDst, typeSrc, xuoptExUnary ) ;
	}
	const ECSClassInfo *	pClassInf = GetNakedTypeClassInfo( typeSrc ) ;
	if ( (pClassInf != NULL)
		&& !(pClassInf->GetAttribute() & ECSTypeInfo::flagNativeObject) )
	{
		ECSClassInfo::ListMemberFunction	lstFunc ;
		EPtrObjArray<ECSTypeInfo>			lstArg ;
		if ( pClassInf->SearchExUnaryOperatorAs
					( lstFunc, xuoptExUnary, lstArg, m_modeNakedCode ) )
		{
			typeDst = lstFunc[0].GetReturnType() ;
			//
			ESLError	err ;
			DWORD		dwArgCount = 0 ;
			ECSTypeInfo	typeThis = typeSrc ;
			if ( m_modeNakedCode )
			{
				/*
				LoadCommitValueToNakedRegister
					( AllocateExpressionRegister(), typeSrc ) ;
				*/
				EObjArray<ECSTypeInfo>	lstArgType ;
				lstArgType[0] = typeSrc ;
				//
				err = NormalizeNakedArgument
						( lstFunc.GetAt(0), lstArgType, dwArgCount ) ;
				if ( err )
				{
					return	err ;
				}
				typeThis = lstArgType[0] ;
			}
			err = CompileCallMemberFunction
					( *pClassInf, typeThis, lstFunc[0],
						lstArg, dwArgCount, ECSTypeInfo::flagPublic ) ;
			int	regReturn = GetExpressionRegister() ;
			//
			FreeExpressionRegister( typeSrc ) ;
			//
			if ( !typeDst.IsVoid() && m_modeNakedCode )
			{
				if ( regReturn != GetExpressionRegister() )
				{
					m_pcsxi->WriteSakuraMoveRegReg
							( GetExpressionRegister(), regReturn ) ;
				}
				GetFunctionReturnType( typeDst, lstFunc[0] ) ;
//				typeDst.SetLoadedRegister( GetExpressionRegister() ) ;
			}
			return	err ;
		}
		else if ( lstFunc.GetSize() >= 2 )
		{
			return	ESLErrorMsg
				( "複数の適合する単項演算子オーバーロードが存在します。" ) ;
		}
	}
	ESLError	err ;
	if ( (xuoptExUnary == csxuotBoolean)
		&& (pClassInf != NULL)
		&& !(pClassInf->GetAttribute() & ECSTypeInfo::flagNativeObject) )
	{
		ECSTypeInfo	typeCast
			( new ECSInteger( 0, ECSInteger::m_maskBoolean ) ) ;
		return	CompileTypeCast
			( typeDst, typeCast, typeSrc, 0, ECSTypeInfo::flagPublic ) ;
	}
	//
	switch ( xuoptExUnary )
	{
	case	csxuotDeselect:
		if ( m_modeNakedCode )
		{
			return	ESLErrorMsg
				( "naked モードでポインタ以外への deselect 演算子です" ) ;
		}
		m_pcsxi->WriteInstructionCode( csicExUniOperate ) ;
		m_pcsxi->WriteExUniOperatorTypeCode( xuoptExUnary ) ;
		//
		if ( !typeSrc.IsAbstractType() && !typeSrc.IsTypeReference2() )
		{
			const ECSClassInfo *	pClassInfo = typeSrc.GetClassInfo() ;
			if ( (pClassInfo == NULL)
				|| !(pClassInfo->GetAttribute() & ECSTypeInfo::flagEnumerator) )
			{
				return	ESLErrorMsg
					( "参照型でないオブジェクトは Deselect できません。" ) ;
			}
		}
		typeDst.SetTypeValue
			( ECSTypeInfo::DuplicateType( typeSrc.GetNakedType() ), 0 ) ;
		return	eslErrSuccess ;

	case	csxuotBoolean:
		if ( m_modeNakedCode )
		{
			typeDst = typeSrc ;
			while ( typeDst.IsTypeReference() )
			{
				CompileCodeNakedUncoverReference( typeDst ) ;
			}
			return	CompileCodeNakedConvertToBoolean( typeDst ) ;
		}
		else
		{
			m_pcsxi->WriteInstructionCode( csicExUniOperate ) ;
			m_pcsxi->WriteExUniOperatorTypeCode( xuoptExUnary ) ;
			//
			err = VerifyTypeBoolean( typeSrc, true ) ;
			if ( !err )
			{
				typeDst = ECSTypeInfo
					( new ECSInteger( 0, ECSInteger::m_maskBoolean ) ) ;
			}
		}
		return	err ;

	case	csxuotSizeOf:
		typeDst = ECSTypeInfo( new ECSInteger ) ;
		if ( m_modeNakedCode
			|| (typeSrc.m_dwFlags & ECSTypeInfo::flagNakedBuffer) )
		{
			ECSObject *	pType = typeSrc.GetNakedType() ;
			int	nNakedSize, nObjCount ;
			err = ECSTypeInfo::GetNakedMemorySize
						( nNakedSize, nObjCount, pType ) ;
			if ( err )
			{
				return	err ;
			}
			if ( m_modeNakedCode )
			{
				FreeExpressionRegister( typeSrc ) ;
				typeDst = ECSTypeInfo
					( new ECSInteger( nNakedSize ),
						ECSTypeInfo::flagDeterministic ) ;
				typeDst.NormalzieImmediateIntegerType() ;
			}
			else
			{
				CompileCodeFreeStack() ;
				CompileImmediateInteger( nNakedSize ) ;
			}
		}
		else
		{
			m_pcsxi->WriteInstructionCode( csicExUniOperate ) ;
			m_pcsxi->WriteExUniOperatorTypeCode( xuoptExUnary ) ;
		}
		return	eslErrSuccess ;

	case	csxuotTypeOf:
		if ( m_modeNakedCode )
		{
			return	ESLErrorMsg
				( "naked モードで動的な typeof 演算子です" ) ;
		}
		m_pcsxi->WriteInstructionCode( csicExUniOperate ) ;
		m_pcsxi->WriteExUniOperatorTypeCode( xuoptExUnary ) ;
		//
		typeDst = ECSTypeInfo( new ECSString ) ;
		return	eslErrSuccess ;

	case	csxuotLoadAddress:
		{
			ECSTypeInfo	typeNakedSrc ;
			ECSTypeInfo	typeTempSrc = typeSrc ;
			while ( typeTempSrc.IsTypeReference2() )
			{
				CompileCodeNakedUncoverReference( typeTempSrc ) ;
			}
			typeNakedSrc.MakeNakedOf( typeTempSrc ) ;
			//
			if ( m_modeNakedCode
				|| !(typeTempSrc.m_dwFlags & ECSTypeInfo::flagNakedBuffer) )
			{
				if ( !typeTempSrc.IsTypeReference() )
				{
					return	ESLErrorMsg
						( "一時オブジェクトへのポインタは評価できません" ) ;
				}
				ECSObject *	pRefType = typeTempSrc.GetNakedType() ;
				if ( pRefType == NULL )
				{
					return	ESLErrorMsg
						( "抽象オブジェクトへのポインタは評価できません" ) ;
				}
				switch ( pRefType->m_vtType )
				{
				case	csvtInteger:
					break ;
				case	csvtReal:
					/*
					typeNakedSrc.SetTypeValue
						( new ECSReal
							( ((ECSReal*)pRefType)->m_varReal ),
											typeTempSrc.m_dwFlags ) ;
					*/
					break ;
				case	csvtArray:
					return	ESLErrorMsg
						( "Array オブジェクトへのポインタは評価できません" ) ;
				}
			}
			if ( !m_modeNakedCode )
			{
				CompileCodePointerToObject( 0 ) ;
			}
			typeDst.MakePointerOf( typeNakedSrc ) ;
			typeDst.MoveRegisterAndAddressingFrom( typeTempSrc ) ;
		}
		return	eslErrSuccess ;

	case	csxuotRefAddress:
		return	ESLErrorMsg( "ポインタでない型への \'*\' アドレス参照です" ) ;
	}
	return	ESLErrorMsg( "不正な特殊演算子です。" ) ;
}

ESLError ECSCompiler::CompileTypePointerExUnaryOperate
	( ECSTypeInfo & typeDst,
		const ECSTypeInfo & typeSrc,
		CSExtraUniOperatorType xuoptExUnary )
{
	ESLAssert( typeSrc.IsTypePointer() ) ;
	//
	ECSTypeInfo	typeSrcTemp = typeSrc ;
	NormalizePointerFromLinearAddress( typeSrcTemp ) ;
	//
	ESLError	err ;
	const ECSClassInfo *	pClassInf = NULL ;
	ECSObject *	pPtrType = typeSrcTemp.GetNakedPointerType() ;
	bool		fObjectPtr = false ;
	bool		fPtrDestruction = false ;
	DWORD		dwJumpRefAddr ;
	//
	switch ( xuoptExUnary )
	{
	case	csxuotDeselect:
		//
		// deselect 演算子
		//
		dwJumpRefAddr = (DWORD) -1 ;
		fObjectPtr = false ;
		if ( m_modeNakedCode )
		{
			//
			// 参照を解除しクラス情報取得
			//
			while ( typeSrcTemp.IsTypeReference() )
			{
				CompileCodeNakedUncoverReference( typeSrcTemp ) ;
			}
			pPtrType = typeSrcTemp.GetNakedPointerType() ;
			//
			pClassInf = pPtrType->m_pClassInf ;
			if ( pClassInf == NULL )
			{
				pClassInf = GetClassInfoAs( pPtrType->GetTypeName() ) ;
			}
			if ( pClassInf != NULL )
			{
				fObjectPtr = !pClassInf->IsNakedMemoryClass() ;
			}
			//
			// ヌルポインタへの delete 判定
			//
			int	regPtr = AllocateExpressionRegister() ;
			LoadCommitValueToNakedRegister( regPtr, typeSrcTemp ) ;
			//
			m_pcsxi->FlushAllRegisterAssigns() ;
			m_pcsxi->WriteSakuraCmpEqRegReg
					( regPtr, ECSSakura2Processor::regIntZero ) ;
			dwJumpRefAddr =
				m_pcsxi->WriteSakuraCJumpOffset32( regPtr, 0 ) ;
			FreeExpressionRegister() ;
		}
		if ( pPtrType->m_vtType == csvtObject )
		{
			pClassInf = pPtrType->m_pClassInf ;
			if ( !fObjectPtr && (pClassInf != NULL)
				&& pClassInf->IsNeedsNakedClassDestruction( m_modeNakedCode ) )
			{
				//
				// デストラクタ呼び出し
				//
				if ( m_modeNakedCode )
				{
					LoadCommitValueToNakedRegister
						( AllocateExpressionRegister(), typeSrcTemp ) ;
				}
				else
				{
					CompileLoadStackObject( 0 ) ;
					CompileCodeReferenceForPointer( csvtObject ) ;
				}
				err = CompileCodeNakedClassPointerDestruction
								( pClassInf, false, fPtrDestruction ) ;
				if ( err )
				{
					return	err ;
				}
			}
		}
		else if ( pPtrType->m_vtType == csvtArray )
		{
			OutputWarning
				( "配列ポインタに対して "
					"deselect が使用されています",
						m_strFilePath, m_nLineNum ) ;
		}
		if ( m_modeNakedCode )
		{
			//
			// ポインタ解放処理
			//
			if ( fObjectPtr )
			{
				LoadCommitValueToNakedRegister
						( AllocateExpressionRegister(), typeSrcTemp ) ;
				CompileNakedSystemCall( L"object_delete", 1 ) ;
//				FreeExpressionRegister() ;
			}
			else if ( !fPtrDestruction )
			{
				LoadCommitValueToNakedRegister
						( AllocateExpressionRegister(), typeSrcTemp ) ;
				CompileNakedSystemCall( L"free", 1 ) ;
//				FreeExpressionRegister() ;
			}
			//
			// ヌルポインタ判定時ジャンプ先確定
			//
			if ( dwJumpRefAddr != (DWORD) -1 )
			{
				CompileCodeCommitJumpAddress
					( dwJumpRefAddr, CompileCodeGetCurrent() ) ;
			}
			FreeExpressionRegister( typeSrcTemp ) ;
			typeDst = ECSTypeInfo() ;
		}
		else
		{
			m_pcsxi->WriteInstructionCode( csicExUniOperate ) ;
			m_pcsxi->WriteExUniOperatorTypeCode( csxuotDelete ) ;
			//
			typeDst = typeSrcTemp ;
		}
		return	eslErrSuccess ;

	case	csxuotDeleteArray:
		//
		// deselect [] 演算子
		//
		if ( m_modeNakedCode )
		{
			while ( typeSrcTemp.IsTypeReference() )
			{
				CompileCodeNakedUncoverReference( typeSrcTemp ) ;
			}
			pPtrType = typeSrcTemp.GetNakedPointerType() ;
		}
		if ( pPtrType->m_vtType == csvtArray )
		{
			pPtrType = ((ECSArray*)pPtrType)->GetEndDefaultElement() ;
		}
		dwJumpRefAddr = (DWORD) -1 ;
		if ( (pPtrType != NULL)
			&& (pPtrType->m_vtType == csvtObject) )
		{
			pClassInf = pPtrType->m_pClassInf ;
			if ( (pClassInf != NULL)
				&& pClassInf->IsNeedsNakedClassDestruction( m_modeNakedCode ) )
			{
				//
				// デストラクタ呼び出し
				//
				if ( m_modeNakedCode )
				{
					int	regPtr = AllocateExpressionRegister() ;
					int	regCount = AllocateExpressionRegister() ;
					int	regLoaded = regCount ;
					//
					LoadCommitValueToNakedRegister( regPtr, typeSrcTemp ) ;
					//
					m_pcsxi->FlushAllRegisterAssigns() ;
					m_pcsxi->WriteSakuraMoveRegReg( regCount, regPtr ) ;
					m_pcsxi->WriteSakuraCmpEqRegReg
							( regCount, ECSSakura2Processor::regIntZero ) ;
					dwJumpRefAddr =
						m_pcsxi->WriteSakuraCJumpOffset32( regCount, 0 ) ;
					//
					WriteSakuraLoadMemory
						( ECSSakura2Processor::addrBaseIndexOffset32,
							ECSSakura2Processor::dataInt64, regLoaded,
							ECSSakura2Processor::regZeroPtr, -8, regPtr, 0 ) ;
					if ( regLoaded != regCount )
					{
						m_pcsxi->WriteSakuraMoveRegReg( regCount, regLoaded ) ;
					}
				}
				else
				{
					CompileLoadStackObject( 0 ) ;
					CompileCodeReferenceForPointer( csvtObject ) ;
					//
					CompileLoadStackObject( 1 ) ;
					CompileImmediateInteger( 8 ) ;
					CompileCodeOperate( csotSub ) ;
					CompileCodeReferenceForPointer( csvtInteger ) ;
				}
				err = CompileCodeNakedVariableDestructionList( pClassInf ) ;
				if ( err )
				{
					return	err ;
				}
			}
		}
		if ( m_modeNakedCode )
		{
			if ( dwJumpRefAddr == (DWORD) -1 )
			{
				int	regPtr = AllocateExpressionRegister() ;
				int	regTemp = AllocateExpressionRegister() ;
				//
				LoadCommitValueToNakedRegister( regPtr, typeSrcTemp ) ;
				//
				m_pcsxi->FlushAllRegisterAssigns() ;
				m_pcsxi->WriteSakuraMoveRegReg( regTemp, regPtr ) ;
				m_pcsxi->WriteSakuraCmpEqRegReg
						( regTemp, ECSSakura2Processor::regIntZero ) ;
				dwJumpRefAddr =
					m_pcsxi->WriteSakuraCJumpOffset32( regTemp, 0 ) ;
				//
				FreeExpressionRegister() ;
				FreeExpressionRegister() ;
			}
			int	regFreePtr = AllocateExpressionRegister() ;
			LoadCommitValueToNakedRegister( regFreePtr, typeSrcTemp ) ;
			m_pcsxi->WriteSakuraAddRegRegImm32( regFreePtr, regFreePtr, -8 ) ;
			//
			CompileNakedSystemCall( L"free", 1 ) ;
			//
			FreeExpressionRegister( typeSrcTemp ) ;
			//
			m_pcsxi->FlushAllRegisterAssigns() ;
			m_pcsxi->ResetAllRegisterAssigns() ;
			//
			if ( dwJumpRefAddr != (DWORD) -1 )
			{
				CompileCodeCommitJumpAddress
					( dwJumpRefAddr, CompileCodeGetCurrent() ) ;
			}
			//
			typeDst = ECSTypeInfo() ;
		}
		else
		{
			m_pcsxi->WriteInstructionCode( csicExUniOperate ) ;
			m_pcsxi->WriteExUniOperatorTypeCode( csxuotDelete ) ;
			//
			typeDst = typeSrcTemp ;
		}
		return	eslErrSuccess ;

	case	csxuotBoolean:
		if ( m_modeNakedCode )
		{
			typeDst = typeSrc ;
			while ( typeDst.IsTypeReference() )
			{
				CompileCodeNakedUncoverReference( typeDst ) ;
			}
			return	CompileCodeNakedConvertToBoolean( typeDst ) ;
		}
		else
		{
			m_pcsxi->WriteInstructionCode( csicExUniOperate ) ;
			m_pcsxi->WriteExUniOperatorTypeCode( xuoptExUnary ) ;
			//
			typeDst = ECSTypeInfo
				( new ECSInteger( 0, ECSInteger::m_maskBoolean ) ) ;
		}
		return	eslErrSuccess ;

	case	csxuotSizeOf:
		if ( m_modeNakedCode )
		{
			FreeExpressionRegister( typeSrc ) ;
		}
		else
		{
			CompileCodeFreeStack() ;
			CompileImmediateInteger( 8 ) ;
		}
//		m_pcsxi->WriteInstructionCode( csicExUniOperate ) ;
//		m_pcsxi->WriteExUniOperatorTypeCode( xuoptExUnary ) ;
		//
		typeDst = ECSTypeInfo
			( new ECSInteger( 8 ), ECSTypeInfo::flagDeterministic ) ;
		return	eslErrSuccess ;

	case	csxuotTypeOf:
		return	ESLErrorMsg( "ポインタへの typeof です" ) ;

	case	csxuotLoadAddress:
		if ( !typeSrcTemp.IsTypeReference() )
		{
			if ( typeSrcTemp.IsTypeFunctionPointer() )
			{
				typeDst = typeSrcTemp ;
				return	eslErrSuccess ;
			}
			else
			{
				return	ESLErrorMsg
					( "一時オブジェクトへのポインタは評価できません" ) ;
			}
		}
		if ( !m_modeNakedCode )
		{
			CompileCodePointerToObject( 0 ) ;
		}
		typeDst.MakePointerOf( typeSrc ) ;
		typeDst.MoveRegisterAndAddressingFrom( typeSrc ) ;
		//
		typeDst.m_dwFlags &= ~ECSTypeInfo::flagNakedBuffer ;
		return	eslErrSuccess ;

	case	csxuotRefAddress:
		err = CompileReferencePointer( typeDst, typeSrcTemp ) ;
		return	err ;
	}
	return	ESLErrorMsg( "不正な特殊演算子です。" ) ;
}

// sizeof(type) 演算子解釈
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::ParseNakedSizeOfTypeOperator
	( ECSSourceStream & cssLine, bool& fSizeOf, int& nNakedSize )
{
	int	iBeginIndex = cssLine.GetIndex() ;
	if ( cssLine.HasToComeChar( L"(" ) == L'(' )
	{
		ECSTypeInfo	typeSizeOf ;
		ESLError	err = ParseTypeDescription( typeSizeOf, cssLine ) ;
		if ( !err )
		{
			if ( cssLine.HasToComeChar( L")" ) == L')' )
			{
				int	nObjCount ;
				fSizeOf = true ;
				err = typeSizeOf.GetNakedMemorySize( nNakedSize, nObjCount ) ;
				if ( err )
				{
					return	ESLErrorMsg
						( "sizeof の型指定が naked な型ではありません" ) ;
				}
				return	eslErrSuccess ;
			}
		}
	}
	cssLine.MoveIndex( iBeginIndex ) ;
	//
	fSizeOf = false ;
	nNakedSize = 0 ;
	return	eslErrSuccess ;
}

// -> 演算子処理
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileTypeCallPointerOperator( ECSTypeInfo & typeinf )
{
	for ( ; ; )
	{
		ECSObject *	pObj = typeinf.GetNakedType() ;
		if ( (pObj == NULL) || (pObj->m_vtType == csvtPointer) )
		{
			break ;
		}
		if ( m_modeNakedCode )
		{
			while ( typeinf.IsTypeReference2() )
			{
				CompileCodeNakedUncoverReference( typeinf ) ;
			}
		}
		ECSClassInfo::ListMemberFunction	lstFunc ;
		EPtrObjArray<ECSTypeInfo>			lstArg ;
		const ECSClassInfo *
				pClassInf = GetNakedTypeClassInfo( typeinf ) ;
		if ( pClassInf->SearchFunctinoAs
			( lstFunc, L"operator ->",
				lstArg, 0, false, false, m_modeNakedCode ) )
		{
			if ( lstFunc.GetSize() > 1 )
			{
				OutputWarning
					( "-> 演算子が曖昧です", m_strFilePath, m_nLineNum ) ;
			}
			MakeCommitValueToNakedRegister( typeinf ) ;
			//
			ESLError	err =
				CompileCallMemberFunction
					( *pClassInf, typeinf,
						lstFunc[0], lstArg, 0, ECSTypeInfo::flagPublic ) ;
			if ( err )
			{
				return	err ;
			}
			GetFunctionReturnType( typeinf, lstFunc[0] ) ;
		}
		else
		{
			break ;
		}
	}
	return	eslErrSuccess ;
}

// 変数作成命令出力
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileNewVariable
	( CSObjectMode csomType,
		const wchar_t * pwszVarName, ECSObject * pTypeObj )
{
	ESLAssert( !m_modeNakedCode ) ;
	m_pcsxi->WriteInstructionCode( csicNew ) ;
	m_pcsxi->WriteObjectModeCode( csomType ) ;
	//
	CSVariableType	csvtType = pTypeObj->m_vtType ;
	if ( csvtType == csvtInteger )
	{
		csvtType = ((ECSInteger*)pTypeObj)->GetIntegerType() ;
	}
	else if ( csvtType == csvtObject )
	{
		int	nClassIndex =
			m_pcsxiDst->GetClassInfoIndex( pTypeObj->GetTypeName() ) ;
		if ( nClassIndex >= 0 )
		{
			m_pcsxi->WriteVariableTypeCode( csvtClassObject ) ;
			m_pcsxi->WriteClassIndex( nClassIndex ) ;
			m_pcsxi->WriteConstantString( ECSWideString( pwszVarName ) ) ;
			return	eslErrSuccess ;
		}
	}
	m_pcsxi->WriteVariableTypeCode( csvtType ) ;
	if ( csvtType == csvtObject )
	{
		m_pcsxi->WriteConstantString
				( ECSWideString( pTypeObj->GetTypeName() ) ) ;
	}
	m_pcsxi->WriteConstantString( ECSWideString( pwszVarName ) ) ;
	//
	if ( csomType == csomStack )
	{
		if ( csvtType == csvtArray )
		{
			return	CompileArrayDimension( (ECSArray*) pTypeObj ) ;
		}
		else if ( csvtType == csvtHash )
		{
			return	CompileHashContainer( (ECSHash*) pTypeObj ) ;
		}
	}
	return	eslErrSuccess ;
}

// ポインタを参照型へ変換する命令出力
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileReferencePointer
	( ECSTypeInfo & typeDst, const ECSTypeInfo & typeSrc )
{
	ECSTypeInfo	typeSrcTemp = typeSrc ;
	if ( m_modeNakedCode )
	{
		while ( typeSrcTemp.IsTypeReference() )
		{
			CompileCodeNakedUncoverReference( typeSrcTemp ) ;
		}
	}
	ESLAssert( typeSrcTemp.IsTypePointer() ) ;
	if ( !typeSrcTemp.IsTypePointer() )
	{
		return	ESLErrorMsg( "参照がポインタに対するものではありません" ) ;
	}
	ECSObject *		pPtrType = typeSrcTemp.GetNakedPointerType() ;
	if ( pPtrType == NULL )
	{
		return	ESLErrorMsg( "void ポインタを参照しています" ) ;
	}
	if ( pPtrType->m_vtType == csvtArray )
	{
		//
		// 配列ポインタへの参照
		//
		ECSObject *	pElementType = ((ECSArray*)pPtrType)->m_pDefObj ;
		if ( pElementType == NULL )
		{
			OutputWarning
				( "void 配列を参照しています",
						m_strFilePath, m_nLineNum ) ;
		}
		ECSTypeInfo	typeRefArray ;
		typeRefArray.SetTypeValue
			( ECSTypeInfo::DuplicateType( pElementType ),
				(typeDst.m_dwFlags & ~ECSTypeInfo::flagNakedBuffer) ) ;
		typeDst.MakePointerOf( typeRefArray ) ;
		typeDst.MoveRegisterAndAddressingFrom( typeSrcTemp ) ;
	}
	else
	{
		//
		// 配列以外のポインタへの参照
		//
		typeDst.MakeNakedPointerOf( typeSrcTemp ) ;
		typeDst.MoveRegisterAndAddressingFrom( typeSrcTemp ) ;
		//
		if ( typeDst.IsNakedMemoryObject
			( (typeSrc.m_dwFlags & ECSTypeInfo::flagNakedBuffer) != 0 ) )
		{
			if ( !m_modeNakedCode )
			{
				CSVariableType	csvtNakedType = typeDst.GetNakedMemoryType() ;
				CompileCodeReferenceForPointer( csvtNakedType ) ;
			}
			typeDst.m_dwFlags |= ECSTypeInfo::flagNakedBuffer ;
			NormalizeObjectPointerRefForNaked( typeDst ) ;
		}
		else
		{
			if ( !m_modeNakedCode )
			{
				CompileCodeReferenceForPointer( csvtObject ) ;
			}
			typeDst.m_dwFlags &= ~ECSTypeInfo::flagNakedBuffer ;
		}
	}
	return	eslErrSuccess ;
}

// ポインタを参照型へ変換する命令出力（型指定）
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileReferenceMemberPointer
	( ECSTypeInfo & typeDst, const ECSTypeInfo & typeRefType )
{
	ESLAssert( !m_modeNakedCode ) ;
	CompileCodeReferenceForPointer( typeRefType.GetNakedMemoryType() ) ;
	//
	DWORD	fdwConst = typeDst.m_dwFlags & ECSTypeInfo::flagConstant ;
	typeDst = typeRefType ;
	typeDst.m_dwFlags |= ECSTypeInfo::flagNakedBuffer ;
	//
	NormalizeObjectPointerRefForNaked( typeDst ) ;
	//
	typeDst.m_dwFlags |= fdwConst ;
	typeDst.m_dwFlags &=
			~(ECSTypeInfo::flagDeterministic
						| ECSTypeInfo::flagProtectedMask) ;
	return	eslErrSuccess ;
}

// naked buffer 上の変数参照をオブジェクトスタック上に正規化する
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::NormalizeObjectPointerRefForNaked( ECSTypeInfo & typeinf )
{
	if ( typeinf.m_dwFlags & ECSTypeInfo::flagNakedBuffer )
	{
		if ( typeinf.IsTypeReference() || typeinf.IsTypePointer() )
		{
			/*
			NormalizePointerFromLinearAddress( typeinf ) ;
			*/
			ECSTypeInfo	typeTemp = typeinf ;
			typeinf.MakeReferenceOf( typeTemp ) ;
			typeinf.MoveRegisterAndAddressingFrom( typeTemp ) ;
		}
		else if ( typeinf.IsTypeArray() )
		{
			ECSObject *	pType = typeinf.GetNakedType() ;
			if ( pType->m_vtType == csvtArray )
			{
				if ( !m_modeNakedCode )
				{
					CompileCodePointerToObject( 0 ) ;
				}
				ECSObject *	pElementType = ((ECSArray*)pType)->m_pDefObj ;
				if ( pElementType == NULL )
				{
					OutputWarning
						( "void 配列を参照しています",
								m_strFilePath, m_nLineNum ) ;
				}
				ECSTypeInfo	typeTemp = typeinf ;
				ECSTypeInfo	typeRefArray ;
				typeRefArray.SetTypeValue
					( ECSTypeInfo::DuplicateType( pElementType ),
						(typeinf.m_dwFlags & ~ECSTypeInfo::flagNakedBuffer) ) ;
				typeinf.MakePointerOf( typeRefArray ) ;
				//
				if ( m_modeNakedCode )
				{
					typeinf.m_dwFlags |= ECSTypeInfo::flagNakedBuffer ;
					typeinf.MoveRegisterAndAddressingFrom( typeTemp ) ;
				}
			}
		}
		else
		{
			// 関数ポインタの解除は naked モードでは型情報のみの操作
			ECSObject *	pType = typeinf.m_pValue ;
			if ( (pType == NULL) || (pType->m_vtType != csvtFunction) )
			{
				ECSTypeInfo	typeTemp = typeinf ;
				typeinf.MakeReferenceOf( typeTemp ) ;
				typeinf.MoveRegisterAndAddressingFrom( typeTemp ) ;
			}
		}
	}
}

// *naked, &naked（リニアアドレス）をポインタオブジェクトに正規化する
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::NormalizePointerFromLinearAddress( ECSTypeInfo & typeinf )
{
	if ( !m_modeNakedCode
		&& (typeinf.m_dwFlags & ECSTypeInfo::flagNakedBuffer) )
	{
		if ( typeinf.IsTypeReference() )
		{
			ECSTypeInfo	typeTemp = typeinf ;
			typeinf.MakeNakedOf( typeTemp ) ;
		}
		if ( typeinf.IsTypeReference() || typeinf.IsTypePointer() )
		{
			CompileCodePointerToAddress() ;
			//
			if ( typeinf.IsTypeReference() )
			{
				ECSTypeInfo	typeTemp ;
				typeTemp.MakeNakedOf( typeinf ) ;
				if ( typeTemp.IsNakedMemoryObject
					( (typeinf.m_dwFlags & ECSTypeInfo::flagNakedBuffer) != 0 ) )
				{
					CSVariableType	csvtNakedType = typeTemp.GetNakedMemoryType() ;
					CompileCodeReferenceForPointer( csvtNakedType ) ;
//					typeinf = typeTemp ;
					typeinf.m_dwFlags |= ECSTypeInfo::flagNakedBuffer ;
				}
				else
				{
					CompileCodeReferenceForPointer( csvtObject ) ;
					typeinf.m_dwFlags &= ~ECSTypeInfo::flagNakedBuffer ;
				}
			}
			else
			{
				typeinf.m_dwFlags &= ~ECSTypeInfo::flagNakedBuffer ;
			}
		}
	}
}

// naked 参照ポインタにオフセット加算
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::CompileCodeOffsetPointerReference( int iOffset )
{
	if ( m_modeNakedCode )
	{
		int	regExpr = GetExpressionRegister() ;
		m_pcsxi->WriteSakuraAddRegRegImm32( regExpr, regExpr, iOffset ) ;
	}
	else
	{
		CompileCodePointerToObject( iOffset ) ;
		CompileCodeReferenceForPointer( csvtObject ) ;
	}
}

// 型キャスト命令出力
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileTypeCast
	( ECSTypeInfo & typeResult,
		const ECSTypeInfo & typeDstOrg,
		const ECSTypeInfo & typeSrcOrg,
		DWORD dwFlags, ECSTypeInfo::Flags flagScope )
{
	ESLError	err ;
	if ( (DWORD) flagScope < typeSrcOrg.GetProtectedAttribute() )
	{
		flagScope = (ECSTypeInfo::Flags) typeSrcOrg.GetProtectedAttribute() ;
	}
	ECSTypeInfo	typeDst, typeSrc ;
	typeSrc = typeSrcOrg ;
	if ( m_modeNakedCode && (dwFlags & castCastNoRef) )
	{
		typeDst.MakeNakedOf( typeDstOrg ) ;
	}
	else
	{
		typeDst = typeDstOrg ;
	}
	if ( m_modeNakedCode && typeSrc.IsTypePointer() )
	{
		typeSrc.m_dwFlags |= ECSTypeInfo::flagNakedBuffer ;
	}
	if ( m_modeNakedCode && typeDst.IsTypePointer() )
	{
		typeDst.m_dwFlags |= ECSTypeInfo::flagNakedBuffer ;
	}
	typeResult = typeDst ;
	typeResult.MoveRegisterAndAddressingFrom( typeSrc ) ;
	//
	// void 判定
	//
	if ( typeDst.IsVoid() )
	{
		if ( typeSrc.IsVoid() )
		{
			typeResult = typeDst ;
			return	eslErrSuccess ;
		}
		if ( dwFlags & castStatic )
		{
			if ( typeSrcOrg.IsLoadedRegister() )
			{
				ESLAssert( GetExpressionRegister() == typeSrcOrg.GetLoadedRegister() ) ;
				FreeExpressionRegister( typeSrcOrg ) ;
			}
			typeResult = typeDst ;
			return	eslErrSuccess ;
		}
		return	ESLErrorMsg( "void 型へは型変換できません" ) ;
	}
	if ( typeSrc.IsVoid() )
	{
		return	ESLErrorMsg( "void 型から型変換できません" ) ;
	}
	//
	// 動的変換判定
	//
	if ( typeDst.IsAbstractType() )
	{
		if ( m_modeNakedCode )
		{
			return	ESLErrorMsg( "naked モードで抽象型へ変換出来ません" ) ;
		}
		// 出力型が純粋 Reference 型の場合には常にランタイム処理
		typeResult = typeDst ;
		return	eslErrSuccess ;
	}
	if ( dwFlags & castDynamic )
	{
		if ( !(dwFlags & castAcceptRef)
				&& !typeDst.IsTypeReference() )
		{
			return	ESLErrorMsg
				( "dynamic_cast は参照型で受け取らなければなりません。" ) ;
		}
		if ( typeDst.IsTypeArray() )
		{
			return	ESLErrorMsg
				( "配列型へ dynamic_cast することは出来ません。" ) ;
		}
		if ( typeSrc.IsTypeArray() )
		{
			return	ESLErrorMsg
				( "配列型を dynamic_cast することは出来ません。" ) ;
		}
		if ( m_modeNakedCode )
		{
			return	ESLErrorMsg
				( "naked モードで dynamic_cast することは出来ません" ) ;
		}
		ECSObject *	pPureType = typeDst.GetPureType() ;
		if ( pPureType != NULL )
		{
			m_pcsxi->WriteInstructionCode( csicExUniOperate ) ;
			m_pcsxi->WriteExUniOperatorTypeCode( csxuotDynamicCast ) ;
			m_pcsxi->WriteConstantString
					( ECSWideString( pPureType->OperateTypeOf() ) ) ;
		}
		return	eslErrSuccess ;
	}
	if ( typeSrc.IsAbstractType() )
	{
		// 入力型が純粋 Reference 型の場合
		if ( typeSrc.m_dwFlags & ECSTypeInfo::flagDeterministic )
		{
			if ( typeDst.IsAbstractType()
				|| typeDst.IsTypeReference()
				|| typeDst.IsTypePointer() )
			{
				return	eslErrSuccess ;	// null から参照型への変換
			}
		}
		if ( (!(dwFlags & castStatic)
			&& (m_dwModeFlags & flagStrictStyle)) || m_modeNakedCode )
		{
			EWideString	wstrDstType ;
			typeDst.FormatTypeString( wstrDstType ) ;
			m_strErrMsg = "純粋 Reference 型から "
					+ EString(wstrDstType) + " へ変換しています。" ;
			return	ESLErrorMsg( m_strErrMsg ) ;
		}
		else
		{
			return	eslErrSuccess ;
		}
	}
	//
	// 変換判定
	//
	ESLError	errVerifyType = eslErrSuccess ;
	ECSTypeInfo::TypeMatchResult
		matchResult = typeDst.IsMatchType( typeSrc ) ;
	if ( matchResult != ECSTypeInfo::typeMatch )
	{
		if ( matchResult == ECSTypeInfo::typeNoMatch )
		{
			errVerifyType = ErrorMsgTypeCast( typeDst, typeSrc ) ;
		}
		if ( (matchResult == ECSTypeInfo::typeLooseMatch)
			&& !(dwFlags & (castStatic | castDynamic))
				&& (m_dwModeFlags & flagStrictStyle) )
		{
			EWideString	wstrDstType, wstrSrcType ;
			typeDst.FormatTypeString( wstrDstType ) ;
			typeSrc.FormatTypeString( wstrSrcType ) ;
			err = OutputWarning1
				( EString( wstrSrcType )
					+ " から " + EString( wstrDstType )
					+ " への型変換はルーズな型変換です。",
									m_strFilePath, m_nLineNum ) ;
			if ( err )
			{
				return	err ;
			}
		}
	}
	if ( typeDst.IsTypeEqual( typeSrc ) )
	{
		if ( m_modeNakedCode
			&& (typeSrc.m_dwFlags & ECSTypeInfo::flagDeterministic) )
		{
			bool	fProcessed = false ;
			return	CompileNakedBasicTypeCast
				( fProcessed, errVerifyType,
					typeResult, typeDst, typeSrc, dwFlags, flagScope ) ;
		}
		return	eslErrSuccess ;
	}
	if ( !m_modeNakedCode
		&& !(typeDst.m_dwFlags & ECSTypeInfo::flagNakedBuffer)
		&& !(typeSrc.m_dwFlags & ECSTypeInfo::flagNakedBuffer) )
	{
		if ( (typeDst.IsTypeArray() && typeSrc.IsTypeArray())
			|| (typeDst.IsTypeHashArray() && typeSrc.IsTypeHashArray()) )
		{
			//
			// 配列型の判定
			//
			if ( !(dwFlags & castStatic) )
			{
				if ( !ECSTypeInfo::IsTypeEqual
					( typeDst.GetNakedType(), typeSrc.GetNakedType() ) )
				{
					return	ErrorMsgTypeCast( typeDst, typeSrc ) ;
				}
				if ( !typeDst.IsTypeReference() )
				do
				{
					ECSObject *	pDstElement = typeDst.GetArrayElementType() ;
					ECSObject *	pSrcElement = typeSrc.GetArrayElementType() ;
					if ( (pDstElement != NULL) && (pSrcElement != NULL) )
					{
						if ( (pDstElement->m_vtType == csvtReference)
							&& (pSrcElement->m_vtType == csvtReference) )
						{
							break ;
						}
					}
					ECSObject *	pDstPure = typeDst.GetPureType() ;
					ECSObject *	pSrcPure = typeSrc.GetPureType() ;
					const ECSClassInfo *	pDstClassInf = typeDst.GetClassInfo() ;
					const ECSClassInfo *	pSrcClassInf = typeSrc.GetClassInfo() ;
					if ( (pDstPure == NULL) || (pSrcPure == NULL)
						|| (pDstClassInf == NULL) || (pSrcClassInf == NULL) )
					{
						break; 
					}
					if ( (pDstClassInf->GetAttribute()
										& ECSTypeInfo::flagNativeObject)
						&& (pSrcClassInf->GetAttribute()
										& ECSTypeInfo::flagNativeObject) )
					{
						if ( pDstClassInf->CanMoveTypeFrom( pSrcPure ) )
						{
							// ネイティブ型の代入適合
							break ;
						}
					}
					if ( (pDstClassInf->GetAttribute()
										& ECSTypeInfo::flagStructure)
						&& (pSrcClassInf->GetAttribute()
										& ECSTypeInfo::flagStructure) )
					{
						// ユーザー構造体に operator := が
						// 定義されている場合には代入できない
						if ( !pDstClassInf->CanMoveTypeFrom( pSrcPure ) )
						{
							break ;
						}
					}
					return	ErrorMsgTypeCast( typeDst, typeSrc ) ;
				}
				while ( false ) ;
			}
			else
			{
				errVerifyType = eslErrSuccess ;
			}
			//
			// 出力先を参照型でなくする
			//
			if ( !typeDst.IsTypeReference()
				&& typeSrc.IsTypeReference()
				&& !(dwFlags & castAcceptRef) )
			{
				m_pcsxi->WriteInstructionCode( csicExUniOperate ) ;
				m_pcsxi->WriteExUniOperatorTypeCode( csxuotDuplicate ) ;
			}
			return	errVerifyType ;
		}
		else if ( typeSrc.IsTypeArray() || typeSrc.IsTypeHashArray() )
		{
			return	ErrorMsgTypeCast( typeDst, typeSrc ) ;
		}
		else if ( typeDst.IsTypeArray() )
		{
			if ( typeSrc.IsTypeBasicEqual( csvtArray ) )
			{
				//
				// 出力先を参照型でなくする
				//
				if ( !typeDst.IsTypeReference()
					&& typeSrc.IsTypeReference()
					&& !(dwFlags & castAcceptRef) )
				{
					m_pcsxi->WriteInstructionCode( csicExUniOperate ) ;
					m_pcsxi->WriteExUniOperatorTypeCode( csxuotDuplicate ) ;
				}
				return	errVerifyType ;
			}
		}
		else if ( typeDst.IsTypeHashArray() )
		{
			if ( typeSrc.IsTypeBasicEqual( csvtHash ) )
			{
				//
				// 出力先を参照型でなくする
				//
				if ( !typeDst.IsTypeReference()
					&& typeSrc.IsTypeReference()
					&& !(dwFlags & castAcceptRef) )
				{
					m_pcsxi->WriteInstructionCode( csicExUniOperate ) ;
					m_pcsxi->WriteExUniOperatorTypeCode( csxuotDuplicate ) ;
				}
				return	errVerifyType ;
			}
		}
	}
	if ( typeDst.IsTypePointer() )
	{
		//
		// ポインタへの変換
		//
		if ( typeSrc.IsTypeInteger()
			&& typeDst.IsNakedMemoryObject( false )
			&& typeSrc.IsNakedMemoryObject( false ) )
		{
			if ( (typeSrc.m_dwFlags & ECSTypeInfo::flagDeterministic)
				&& (typeSrc.m_pValue != NULL)
				&& (typeSrc.m_pValue->m_vtType == csvtInteger)
				&& (((ECSInteger*)(typeSrc.m_pValue))->GetValue() == 0) )
			{
				if ( !typeResult.IsLoadedRegister() )
				{
					if ( m_modeNakedCode )
					{
						int	regExpr = AllocateExpressionRegister() ;
						m_pcsxi->WriteSakuraMoveRegReg
							( regExpr, ECSSakura2Processor::regIntZero ) ;
						typeResult.ClearAddressingInfo() ;
						typeResult.SetLoadedRegister( regExpr ) ;
					}
				}
				return	eslErrSuccess ;
			}
			if ( dwFlags & castStatic )
			{
				if ( m_modeNakedCode )
				{
					ECSTypeInfo	typeSrcTemp = typeSrc ;
					CompileCodeNakedUncoverReference( typeSrcTemp ) ;
					if ( typeSrcTemp.IsLoadedRegister() )
					{
						typeDst.ClearAddressingInfo() ;
						typeDst.SetLoadedRegister
								( typeSrcTemp.GetLoadedRegister() ) ;
					}
					typeResult.MoveRegisterAndAddressingFrom( typeSrc ) ;
				}
				else if ( !(typeDst.m_dwFlags & ECSTypeInfo::flagNakedBuffer) )
				{
					CompileCodePointerToAddress() ;
				}
				return	errVerifyType ;
			}
			return	ESLErrorMsg( "整数からポインタ型へ変換できません" ) ;
		}
		else if ( m_modeNakedCode
				&& typeSrc.IsTypeArray()
				&& (typeSrc.m_dwFlags & ECSTypeInfo::flagNakedBuffer) )
		{
			const ECSPointer *	pDstPtr =
					(const ECSPointer *) typeDst.GetNakedType() ;
			const ECSObject *	pSrcObj =
					(const ECSObject *) typeSrc.GetNakedType() ;
			if ( !(pDstPtr->m_fReadOnly)
				&& (typeSrc.m_dwFlags & ECSTypeInfo::flagConstant) )
			{
				return	ESLErrorMsg( "const な配列をポインタに変換出来ません" ) ;
			}
			const ECSObject *	pDstPtrType = pDstPtr->m_pRef ;
			if ( pDstPtrType == NULL )
			{
				return	errVerifyType ;
			}
			if ( ECSTypeInfo::IsTypeNakedArrayEqual( pDstPtrType, pSrcObj ) )
			{
				return	errVerifyType ;
			}
		}
		else if ( !typeSrc.IsTypePointer() )
		{
			err = CompileCastByOperator
				( typeResult, typeDst, typeSrc, dwFlags, flagScope ) ;
			/*
			if ( !err )
			{
				err = errVerifyType ;
			}
			*/
			return	err ;
		}
		//
		ECSTypeInfo	typeSrcTemp = typeSrc ;
		CompileCodeNakedUncoverReference( typeSrcTemp ) ;
		//
		err = CompileTypeCastFromPointer
					( typeResult, typeDst, typeSrcTemp, dwFlags, flagScope ) ;
		if ( !err && !(dwFlags & castStatic) )
		{
			err = errVerifyType ;
		}
		return	err ;
	}
	else if ( typeSrc.IsTypePointer() )
	{
		//
		// ポインタからの変換
		//
		ECSTypeInfo	typeSrcTemp = typeSrc ;
		CompileCodeNakedUncoverReference( typeSrcTemp ) ;
		//
		err = CompileTypeCastFromPointer
			( typeResult, typeDst, typeSrcTemp, dwFlags, flagScope ) ;
		if ( !err && !(dwFlags & castStatic) )
		{
			err = errVerifyType ;
		}
		return	err ;
	}
	//
	// naked モードでの基本型（整数・実数）変換
	//
	bool	fProcessed = false ;
	err = CompileNakedBasicTypeCast
		( fProcessed, errVerifyType,
			typeResult, typeDst, typeSrc, dwFlags, flagScope ) ;
	if ( err || fProcessed )
	{
		return	err ;
	}
	//
	// 基本クラスの変換
	//
	const ECSClassInfo *	pDstClassInf = NULL ;
	const ECSClassInfo *	pSrcClassInf = GetNakedTypeClassInfo( typeSrc ) ;
	//
	if ( !typeDst.IsTypeArray() && !typeDst.IsTypeHashArray() )
	{
		pDstClassInf = GetNakedTypeClassInfo( typeDst ) ;
		//
		// 列挙型のテスト
		//
		if ( (pDstClassInf != NULL)
			&& (pDstClassInf->GetAttribute() & ECSTypeInfo::flagEnumerator) )
		{
			if ( (typeSrc.m_dwFlags & ECSTypeInfo::flagDeterministic)
				&& (typeSrc.m_pValue != NULL) )
			{
				if ( !pDstClassInf->IsMatchEnumeratorValue( typeSrc.m_pValue ) )
				{
					OutputWarning
						( EString(pDstClassInf->GetGlobalName())
							+ " 列挙型へ適合しない値の変換です",
											m_strFilePath, m_nLineNum ) ;
				}
			}
			else if ( (pSrcClassInf == NULL)
					|| (pSrcClassInf->GetGlobalName()
							!= pDstClassInf->GetGlobalName()) )
			{
				OutputWarning
					( EString(pDstClassInf->GetGlobalName())
						+ " 列挙型へ定数値でないオブジェクトの変換です",
												m_strFilePath, m_nLineNum ) ;
			}
		}
	}
	if ( !m_modeNakedCode
		&& ((pSrcClassInf == NULL)
			|| (pSrcClassInf->IsBasicType() != csvtObject)) )
	{
		//
		// 出力先を参照型でなくする
		//
		if ( (matchResult != ECSTypeInfo::typeMatch)
			|| (!typeDst.IsTypeReference()
					&& typeSrc.IsTypeReference()
					&& !(dwFlags & castAcceptRef)) )
		{
			if ( typeDst.IsTypeBoolean() )
			{
				m_pcsxi->WriteInstructionCode( csicExUniOperate ) ;
				m_pcsxi->WriteExUniOperatorTypeCode( csxuotBoolean ) ;
			}
			else if ( pDstClassInf != NULL )
			{
				err = CompileObjectConstruction( *pDstClassInf, typeSrc ) ;
				if ( err )
				{
					return	err ;
				}
			}
			else
			{
				m_pcsxi->WriteInstructionCode( csicExUniOperate ) ;
				m_pcsxi->WriteExUniOperatorTypeCode( csxuotDuplicate ) ;
			}
		}
		if ( !m_modeNakedCode && (dwFlags & castStatic) )
		{
			return	eslErrSuccess ;
		}
		return	errVerifyType ;
	}
	//
	// 親クラスへの変換
	//
	if ( pDstClassInf != NULL )
	{
		ECSClassInfo::CastInfo *
			pCastParent =
				pSrcClassInf->GetCastParentClassAs
							( pDstClassInf->GetGlobalName() ) ;
		if ( pCastParent != NULL )
		{
			ECSTypeInfo	typeAddressing ;
			err = CompileCastToParentClass
				( typeAddressing,
					*pCastParent, *pSrcClassInf, typeSrc, flagScope ) ;
			if ( err )
			{
				return	err ;
			}
			typeResult.MoveRegisterAndAddressingFrom( typeAddressing ) ;
			//
			if ( !(dwFlags & castAcceptRef)
					&& !typeDst.IsTypeReference() )
			{
				if ( m_modeNakedCode )
				{
					return	ESLErrorMsg
						( "naked モードでの型変換で"
							"オブジェクトの構築は出来ません" ) ;
				}
				//
				// 出力先を参照型でなくする
				//
				ECSTypeInfo	typeCastSrc
					( new ECSStructure( pCastParent->pClassInf ),
													typeSrc.m_dwFlags ) ;
				ECSTypeInfo	typeRefCastSrc ;
				typeRefCastSrc.MakeReferenceOf( typeCastSrc ) ;
				//
				err = CompileObjectConstruction
					( *pDstClassInf, typeRefCastSrc ) ;
				if ( err )
				{
					return	err ;
				}
			}
			if ( !m_modeNakedCode && (dwFlags & castStatic) )
			{
				return	eslErrSuccess ;
			}
			return	errVerifyType ;
		}
	}
	//
	// 型キャストオペレータオーバーロード
	//
	return	CompileCastByOperator
				( typeResult, typeDst, typeSrc, dwFlags, flagScope ) ;
}

// ポインタ型からのキャスト命令出力
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileTypeCastFromPointer
	( ECSTypeInfo & typeResult,
		const ECSTypeInfo & typeDstOrg,
		const ECSTypeInfo & typeSrcOrg,
		DWORD dwFlags, ECSTypeInfo::Flags flagScope )
{
	ESLAssert( typeSrcOrg.IsTypePointer() ) ;
	ECSTypeInfo	typeSrc = typeSrcOrg ;
	ECSTypeInfo	typeDst = typeDstOrg ;
	if ( !typeDst.IsTypePointer() )
	{
		if ( typeDst.IsTypeBoolean() && (dwFlags & castStatic) )
		{
			//
			// boolean への変換
			//
			if ( m_modeNakedCode )
			{
				CompileCodeNakedUncoverReference( typeSrc ) ;
				MakeCommitValueToNakedRegister( typeSrc ) ;
				//
				int	regExpr = GetExpressionRegister() ;
				if ( typeSrc.IsLoadedRegister() )
				{
					regExpr = typeSrc.GetLoadedRegister() ;
				}
				m_pcsxi->WriteSakuraCmpNeRegReg
					( regExpr, ECSSakura2Processor::regIntZero ) ;
				//
				typeResult =
					ECSTypeInfo
						( new ECSInteger( 0, ECSInteger::m_maskBoolean ) ) ;
				typeResult.SetLoadedRegister( regExpr ) ;
			}
			else
			{
				NormalizePointerFromLinearAddress( typeSrc ) ;
				m_pcsxi->WriteInstructionCode( csicExUniOperate ) ;
				m_pcsxi->WriteExUniOperatorTypeCode( csxuotBoolean ) ;
				typeResult = typeDst ;
			}
			return	eslErrSuccess ;
		}
		else if ( typeDst.IsTypeInteger() && (dwFlags & castStatic) )
		{
			//
			// int への変換
			//
			if ( m_modeNakedCode
				|| (typeSrc.m_dwFlags & ECSTypeInfo::flagNakedBuffer) )
			{
				CompileCodeNakedUncoverReference( typeSrc ) ;
				MakeCommitValueToNakedRegister( typeSrc ) ;
				//
				typeResult = typeDst ;
				typeResult.SetLoadedRegister( typeSrc ) ;
				return	eslErrSuccess ;
			}
			return	ESLErrorMsg( "ポインタをリニアアドレスに変換出来ません" ) ;
		}
		return	ESLErrorMsg( "ポインタ型から変換出来ません" ) ;
	}
	if ( typeDst.m_dwFlags & ECSTypeInfo::flagNakedBuffer )
	{
		if ( !m_modeNakedCode
			&& !(typeSrc.m_dwFlags & ECSTypeInfo::flagNakedBuffer) )
		{
			if ( !(dwFlags & castNoNakedPointer) )
			{
				return	ESLErrorMsg( "ポインタをリニアアドレスに変換出来ません" ) ;
			}
			typeDst.m_dwFlags &= ~ECSTypeInfo::flagNakedBuffer ;
		}
	}
	else
	{
		NormalizePointerFromLinearAddress( typeSrc ) ;
	}
	CompileCodeNakedUncoverReference( typeSrc ) ;
	//
	typeResult = typeDst ;
	typeResult.MoveRegisterAndAddressingFrom( typeSrc ) ;
	//
	ECSObject *	pDstPtrType = typeDst.GetNakedPointerType() ;
	ECSObject *	pSrcPtrType = typeSrc.GetNakedPointerType() ;
	if ( (pDstPtrType == NULL) || (pSrcPtrType == NULL) )
	{
		return	eslErrSuccess ;
	}
	if ( ECSTypeInfo::IsTypeNakedArrayPointerMatch( pDstPtrType, pSrcPtrType ) )
	{
		return	eslErrSuccess ;
	}
	//
	// 派生クラスから親クラスポインタへの変換判定
	//
	if ( (pSrcPtrType->m_vtType == csvtObject)
		&& (pSrcPtrType->m_pClassInf != NULL) )
	{
		const ECSClassInfo *	pSrcClassInf = pSrcPtrType->m_pClassInf ;
		ECSClassInfo::CastInfo *	pCastInf =
			pSrcClassInf->GetCastParentClassAs
					( pDstPtrType->OperateTypeOf() ) ;
		if ( (pCastInf != NULL) && (pCastInf->pClassInf != NULL) )
		{
			if ( pSrcClassInf->IsNakedMemoryClass()
				&& pCastInf->pClassInf->IsNakedMemoryClass() )
			{
				const int	nOffset = pCastInf->nNakedOffset ;
				if ( m_modeNakedCode )
				{
					if ( typeSrc.IsAddressingInfo() )
					{
						typeResult.MoveRegisterAndAddressingFrom( typeSrc ) ;
						typeResult.m_addrOffset += nOffset ;
					}
					else
					{
						int	regExpr = typeSrc.IsLoadedRegister()
										? typeSrc.GetLoadedRegister()
											: AllocateExpressionRegister() ;
						int	regTemp0 = AllocateExpressionRegister() ;
						int	regTemp1 = AllocateExpressionRegister() ;
						//
						typeResult.SetLoadedRegister( regExpr ) ;
						//
						if ( nOffset != 0 )
						{
							m_pcsxi->WriteSakuraMoveRegReg( regTemp0, regExpr ) ;
							m_pcsxi->WriteSakuraCmpNeRegReg
								( regExpr, ECSSakura2Processor::regIntZero ) ;
							m_pcsxi->WriteSakuraAddRegRegImm32
								( regTemp1, ECSSakura2Processor::regIntZero, nOffset ) ;
							m_pcsxi->WriteSakuraAndRegReg( regExpr, regTemp1 ) ;
							m_pcsxi->WriteSakuraAddRegReg( regExpr, regTemp0 ) ;
						}
						//
						FreeExpressionRegister() ;
						FreeExpressionRegister() ;
					}
				}
				else
				{
					CompileImmediateInteger( nOffset ) ;
					CompileCodeOperate( csotAdd ) ;
				}
				return	eslErrSuccess ;
			}
			else if ( !pSrcClassInf->IsNakedMemoryClass()
					&& !pCastInf->pClassInf->IsNakedMemoryClass() )
			{
				ECSTypeInfo	typeTemp
					( ECSTypeInfo::DuplicateType( pSrcPtrType ), typeSrc.m_dwFlags ) ;
				ECSTypeInfo	typeSrcRef ;
				typeSrcRef.MakeReferenceOf( typeTemp ) ;
				//
				if ( m_modeNakedCode )
				{
					typeSrcRef.MoveRegisterAndAddressingFrom( typeSrc ) ;
				}
				else
				{
					m_pcsxi->WriteInstructionCode( csicReferenceForObjPointer ) ;
				}
				ECSTypeInfo	typeAddressing ;
				ESLError	err =
					CompileCastToParentClass
						( typeAddressing, *pCastInf,
							*pSrcClassInf, typeSrcRef, flagScope ) ;
				if ( err )
				{
					return	err ;
				}
				if ( !m_modeNakedCode )
				{
					CompileCodePointerToObject( 0 ) ;
				}
				typeResult = typeDst ;
				typeResult.MoveRegisterAndAddressingFrom( typeAddressing ) ;
				return	eslErrSuccess ;
			}
			else
			{
				return	ESLErrorMsg
					( "naked なポインタと object ポインタを"
							"相互に変換することはできません" ) ;
			}
		}
	}
	//
	// 親クラスから派生クラスへの変換判定
	//
	if ( (pDstPtrType->m_vtType == csvtObject)
		&& (pDstPtrType->m_pClassInf != NULL) )
	{
		const ECSClassInfo *	pDstClassInf = pDstPtrType->m_pClassInf ;
		ECSClassInfo::CastInfo *	pCastInf =
			pDstClassInf->GetCastParentClassAs
					( pSrcPtrType->OperateTypeOf() ) ;
		if ( (pCastInf != NULL) && (pCastInf->pClassInf != NULL) )
		{
			if ( pDstClassInf->IsNakedMemoryClass()
				&& pCastInf->pClassInf->IsNakedMemoryClass() )
			{
				const int	nOffset = - pCastInf->nNakedOffset ;
				if ( m_modeNakedCode )
				{
					if ( typeSrc.IsAddressingInfo() )
					{
						typeResult.MoveRegisterAndAddressingFrom( typeSrc ) ;
						typeResult.m_addrOffset += nOffset ;
					}
					else
					{
						int	regExpr = typeSrc.IsLoadedRegister()
										? typeSrc.GetLoadedRegister()
											: AllocateExpressionRegister() ;
						int	regTemp0 = AllocateExpressionRegister() ;
						int	regTemp1 = AllocateExpressionRegister() ;
						//
						typeResult.SetLoadedRegister( regExpr ) ;
						//
						if ( nOffset != 0 )
						{
							m_pcsxi->WriteSakuraMoveRegReg( regTemp0, regExpr ) ;
							m_pcsxi->WriteSakuraCmpNeRegReg
								( regExpr, ECSSakura2Processor::regIntZero ) ;
							m_pcsxi->WriteSakuraAddRegRegImm32
								( regTemp1, ECSSakura2Processor::regIntZero, nOffset ) ;
							m_pcsxi->WriteSakuraAndRegReg( regExpr, regTemp1 ) ;
							m_pcsxi->WriteSakuraAddRegReg( regExpr, regTemp0 ) ;
						}
						//
						FreeExpressionRegister() ;
						FreeExpressionRegister() ;
					}
				}
				else
				{
					CompileImmediateInteger( nOffset ) ;
					CompileCodeOperate( csotAdd ) ;
				}
				return	eslErrSuccess ;
			}
			else if ( !pDstClassInf->IsNakedMemoryClass()
					&& !pCastInf->pClassInf->IsNakedMemoryClass() )
			{
				if ( m_modeNakedCode && (dwFlags & castStatic) )
				{
					typeResult.MoveRegisterAndAddressingFrom( typeSrc ) ;
					return	eslErrSuccess ;
				}
				return	ESLErrorMsg
					( "naked でない派生クラスへのポインタ変換はできません" ) ;
			}
			else
			{
				return	ESLErrorMsg
					( "naked なポインタと object ポインタを"
							"相互に変換することはできません" ) ;
			}
		}
	}
	if ( dwFlags & castStatic )
	{
		return	eslErrSuccess ;
	}
	else
	{
		return	ESLErrorMsg( "ポインタを変換できません" ) ;
	}
}

// 型キャストオペレーターオーバーロード処理出力
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileCastByOperator
	( ECSTypeInfo & typeResult,
		const ECSTypeInfo & typeDst,
		const ECSTypeInfo & typeSrc,
		DWORD dwFlags, ECSTypeInfo::Flags flagScope )
{
	if ( typeSrc.IsTypePointer() )
	{
		return	CompileTypeCastFromPointer
					( typeResult, typeDst, typeSrc, dwFlags, flagScope ) ;
	}
	const ECSClassInfo *	pSrcClassInf = GetNakedTypeClassInfo( typeSrc ) ;
	if ( m_modeNakedCode
		&& ((pSrcClassInf == NULL) || !pSrcClassInf->IsNakedMemoryClass()) )
	{
		return	ESLErrorMsg
			( "naked モードで object クラスに対するキャストです" ) ;
	}
	//
	// 型キャストオペレータオーバーロード
	//
	EWideString	wstrDstTypeName ;
	typeDst.FormatTypeString( wstrDstTypeName ) ;
	EWideString	wstrFuncName = L"operator " + wstrDstTypeName ;
	//
	EPtrObjArray<ECSTypeInfo>			lstArg ;
	ECSClassInfo::ListMemberFunction	lstFunc ;
	if ( !pSrcClassInf->SearchFunctinoAs
			( lstFunc, wstrFuncName, lstArg,
				(typeSrc.m_dwFlags & ECSTypeInfo::flagConstant),
				false, false, m_modeNakedCode ) )
	{
		if ( lstFunc.GetSize() == 0 )
		{
			if ( !typeDst.IsTypeReference()
				&& (dwFlags & castAcceptRef) )
			{
				wstrFuncName += L"&" ;
				pSrcClassInf->SearchFunctinoAs
					( lstFunc, wstrFuncName, lstArg,
						(typeSrc.m_dwFlags & ECSTypeInfo::flagConstant),
						false, false, m_modeNakedCode ) ;
			}
		}
		if ( lstFunc.GetSize() == 0 )
		{
			return	ErrorMsgTypeCast( typeDst, typeSrc ) ;
		}
		else if ( lstFunc.GetSize() > 1 )
		{
			return	ESLErrorMsg
				( "適合する型変換オーバーロード関数が複数あります。" ) ;
		}
	}
	ECSClassInfo::MemberFunction *	pMemberFunc = lstFunc.GetAt(0) ;
	if ( pMemberFunc == NULL )
	{
		return	ESLErrorMsg
			( "内部エラー：型変換オーバーロード関数が見つかりません。" ) ;
	}
	typeResult = pMemberFunc->GetReturnType() ;
	//
	if ( !m_modeNakedCode
		&& (pSrcClassInf->GetAttribute() & ECSTypeInfo::flagNativeObject) )
	{
		if ( !(dwFlags & castDynamic) )
		{
			return	ESLErrorMsg
				( "ネイティブ型の型変換は"
					" dynamic_cast でなければなりません。" ) ;
		}
		if ( typeDst.IsTypeBoolean() )
		{
			m_pcsxi->WriteInstructionCode( csicExUniOperate ) ;
			m_pcsxi->WriteExUniOperatorTypeCode( csxuotBoolean ) ;
		}
		else
		{
			m_pcsxi->WriteInstructionCode( csicExUniOperate ) ;
			m_pcsxi->WriteExUniOperatorTypeCode( csxuotDynamicCast ) ;
			m_pcsxi->WriteConstantString( wstrDstTypeName ) ;
		}
		return	eslErrSuccess ;
	}
	ECSTypeInfo	typeThis = typeSrc ;
	if ( m_modeNakedCode )
	{
		while ( typeThis.IsTypeReference2() )
		{
			CompileCodeNakedUncoverReference( typeThis ) ;
		}
		MakeCommitValueToNakedRegister( typeThis ) ;
	}
	ESLError	err =
		CompileCallMemberFunction
			( *pSrcClassInf, typeThis,
				*pMemberFunc, lstArg, 0, flagScope ) ;
	if ( m_modeNakedCode && !typeResult.IsVoid() )
	{
		GetFunctionReturnType( typeResult, *pMemberFunc ) ;
	}
	return	err ;
}

// 親クラスへのキャスト命令出力
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileCastToParentClass
	( ECSTypeInfo & typeAddressing,
		const ECSClassInfo::CastInfo & castParent,
		const ECSClassInfo & clsinfSrc,
		const ECSTypeInfo & typeSrc, ECSTypeInfo::Flags flagScope )
{
	const DWORD	dwProtectedMask = ECSTypeInfo::flagProtectedMask ;
	if ( (castParent.dwFlags & dwProtectedMask)
						> (DWORD) (flagScope & dwProtectedMask) )
	{
		if ( castParent.dwFlags & ECSTypeInfo::flagPrivate )
		{
			return	ESLErrorMsg
				( "private な親クラスへの型キャストです。" ) ;
		}
		return	ESLErrorMsg
			( "protected な親クラスへの型キャストです。" ) ;
	}
	//
	typeAddressing = typeSrc ;
	if ( m_modeNakedCode )
	{
		while ( typeAddressing.IsTypeReference2() )
		{
			CompileCodeNakedUncoverReference( typeAddressing ) ;
		}
	}
	//
	if ( !(clsinfSrc.GetAttribute() & ECSTypeInfo::flagNativeObject) )
	{
		if ( castParent.pClassInf->GetAttribute() & ECSTypeInfo::flagNativeObject )
		{
			if ( !typeSrc.IsTypeReference() )
			{
				ESLError	err = OutputWarning0
					( "Native 型親クラスへのキャストが、"
						"クラスの参照型からの変換ではありません。",
							m_strFilePath, m_nLineNum ) ;
				if ( err )
				{
					return	err ;
				}
			}
			if ( m_modeNakedCode )
			{
				return	ESLErrorMsg
					( "naked モードで object クラスの"
						" native へのキャストは未対応です" ) ;
			}
			long int	nIndex = castParent.iNativeParent ;
			m_pcsxi->WriteInstructionCode( csicElement ) ;
			m_pcsxi->WriteVariableTypeCode( csvtInteger ) ;
			m_pcsxi->WriteCodeData( &nIndex, sizeof(long int) ) ;
		}
		else if ( clsinfSrc.IsNakedMemoryClass() )
		{
			if ( m_modeNakedCode )
			{
				if ( typeAddressing.IsAddressingInfo() )
				{
					typeAddressing.m_addrOffset += castParent.nNakedOffset ;
				}
				else
				{
					int	regExpr = typeAddressing.IsLoadedRegister()
									? typeAddressing.GetLoadedRegister()
									: AllocateExpressionRegister() ;
					int	regTemp0 = AllocateExpressionRegister() ;
					int	regTemp1 = AllocateExpressionRegister() ;
					//
					typeAddressing.SetLoadedRegister( regExpr ) ;
					//
					if ( castParent.nNakedOffset != 0 )
					{
						m_pcsxi->WriteSakuraMoveRegReg( regTemp0, regExpr ) ;
						m_pcsxi->WriteSakuraCmpNeRegReg
							( regExpr, ECSSakura2Processor::regIntZero ) ;
						m_pcsxi->WriteSakuraAddRegRegImm32
							( regTemp1, ECSSakura2Processor::regIntZero, castParent.nNakedOffset ) ;
						m_pcsxi->WriteSakuraAndRegReg( regExpr, regTemp1 ) ;
						m_pcsxi->WriteSakuraAddRegReg( regExpr, regTemp0 ) ;
					}
					//
					FreeExpressionRegister() ;
					FreeExpressionRegister() ;
				}
			}
			else
			{
				CompileCodeOffsetPointerReference( castParent.nNakedOffset ) ;
			}
		}
		else
		{
			if ( (castParent.iVarOffset != 0)
				|| (castParent.iFuncOffset != 0)
				|| (castParent.nVarBounds >= 0) )
			{
				if ( m_modeNakedCode )
				{
					return	ESLErrorMsg
						( "naked モードで object クラスのキャストは未対応です" ) ;
				}
				SDWORD	dwCast[3] =
				{
					castParent.iVarOffset,
					castParent.nVarBounds,
					castParent.iFuncOffset
				} ;
				m_pcsxi->WriteInstructionCode( csicExUniOperate ) ;
				m_pcsxi->WriteExUniOperatorTypeCode( csxuotStaticCast ) ;
				m_pcsxi->WriteCodeData( dwCast, sizeof(dwCast) ) ;
			}
		}
	}
	return	eslErrSuccess ;
}

// naked モードでの基本型（整数・実数）変換
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileNakedBasicTypeCast
	( bool& fProcessed, ESLError errVerifyType,
		ECSTypeInfo & typeResult,
		const ECSTypeInfo & typeDst, const ECSTypeInfo & typeSrc,
		DWORD dwFlags, ECSTypeInfo::Flags flagScope )
{
	fProcessed = false ;
	if ( !m_modeNakedCode )
	{
		return	eslErrSuccess ;
	}
	if ( typeDst.IsTypeReference2() )
	{
		fProcessed = true ;
		ECSTypeInfo	typeSrcTemp = typeSrc ;
		for ( ; ; )
		{
			if ( !typeSrcTemp.IsTypeReference2() )
			{
				return	ESLErrorMsg( "naked モードで参照型へ変換出来ません" ) ;
			}
			if ( ECSTypeInfo::IsTypeEqual( typeDst.m_pValue, typeSrcTemp.m_pValue ) )
			{
				return	errVerifyType ;
			}
			if ( dwFlags & castStatic )
			{
				return	errVerifyType ;
			}
			CompileCodeNakedUncoverReference( typeSrcTemp ) ;
			typeResult.MoveRegisterAndAddressingFrom( typeSrcTemp ) ;
		}
		return	ESLErrorMsg( "naked モードで変換できない参照型です" ) ;
	}
	if ( (typeDst.IsTypeInteger()
				|| typeDst.IsRuntimeIntegerType() || typeDst.IsTypeReal())
		&& (typeSrc.IsTypeInteger()
				|| typeSrc.IsRuntimeIntegerType() || typeSrc.IsTypeReal()) )
	{
		fProcessed = true ;
		if ( typeDst.IsTypeReference() )
		{
			//
			// naked モードでの参照型への変換
			//
			ECSTypeInfo	typeSrcTemp = typeSrc ;
			for ( ; ; )
			{
				if ( !typeSrcTemp.IsTypeReference() )
				{
					return	ESLErrorMsg( "naked モードで参照型へ変換出来ません" ) ;
				}
				if ( ECSTypeInfo::IsTypeEqual( typeDst.m_pValue, typeSrcTemp.m_pValue ) )
				{
					return	errVerifyType ;
				}
				if ( dwFlags & castStatic )
				{
					return	errVerifyType ;
				}
				CompileCodeNakedUncoverReference( typeSrcTemp ) ;
				typeResult.MoveRegisterAndAddressingFrom( typeSrcTemp ) ;
			}
			return	ESLErrorMsg( "naked モードで変換できない参照型です" ) ;
		}
		ECSTypeInfo	typeSrcTemp = typeSrc ;
		while ( typeSrcTemp.IsTypeReference() )
		{
			CompileCodeNakedUncoverReference( typeSrcTemp ) ;
		}
//		MakeCommitValueToNakedRegister( typeSrcTemp ) ;
//		CompileCodeNakedUncoverEnumerator( typeSrcTemp ) ;
		ESLAssert( !typeSrcTemp.IsTypeReference() ) ;
		//
		if ( typeDst.IsTypeBoolean() )
		{
			CompileCodeNakedConvertToBoolean( typeSrcTemp ) ;
			typeResult.SetTypeValue
				( ECSTypeInfo::DuplicateType
					( typeDst.GetNakedType() ), typeDst.m_dwFlags )  ;
			typeResult.MoveFromImmediateValue( m_ctxExpr, typeSrcTemp ) ;
			typeResult.SetLoadedRegister( typeSrcTemp ) ;
			return	errVerifyType ;
		}
		else if ( typeDst.IsTypeInteger() )
		{
			CompileCodeNakedConvertToInt64( typeSrcTemp ) ;
			typeResult.SetTypeValue
				( ECSTypeInfo::DuplicateType
					( typeDst.GetNakedType() ), typeDst.m_dwFlags )  ;
			typeResult.MoveFromImmediateValue( m_ctxExpr, typeSrcTemp ) ;
			typeResult.SetLoadedRegister( typeSrcTemp ) ;
			//
			ESLError	err = CompileNakedCastIntegerToInteger
							( typeResult, typeSrcTemp ) ;
			if ( err )
			{
				return	err ;
			}
			return	errVerifyType ;
		}
		else if ( typeDst.IsTypeReal() )
		{
			CompileCodeNakedConvertToReal( typeSrcTemp ) ;
			typeResult.SetTypeValue
				( ECSTypeInfo::DuplicateType
					( typeDst.GetNakedType() ), typeDst.m_dwFlags )  ;
			typeResult.MoveFromImmediateValue( m_ctxExpr, typeSrcTemp ) ;
			typeResult.SetLoadedRegister( typeSrcTemp ) ;
			return	errVerifyType ;
		}
		else if ( typeDst.IsRuntimeIntegerType() )
		{
			if ( !(dwFlags & castStatic) )
			{
				ECSObject *	pSrcValue = typeSrcTemp.GetNakedType() ;
				ECSObject *	pDstValue = typeDst.GetNakedType() ;
				if ( (pSrcValue != NULL)
					&& (pDstValue != NULL)
					&& (pDstValue->m_vtType == csvtObject) )
				{
					const ECSClassInfo *	pSrcClassInf = pSrcValue->m_pClassInf ;
					const ECSClassInfo *	pDstClassInf = pDstValue->m_pClassInf ;
					if ( pDstClassInf->GetAttribute()
									& ECSTypeInfo::flagEnumerator )
					{
						if ( (pSrcClassInf != NULL)
							&& (pSrcClassInf->GetAttribute()
										& ECSTypeInfo::flagEnumerator) )
						{
							if ( pDstClassInf->GetGlobalName()
										!= pSrcClassInf->GetGlobalName() )
							{
								m_strErrMsg =
									EString(pDstClassInf->GetGlobalName())
										+ " 列挙型へ適合しない列挙型の代入です" ;
								return	ESLErrorMsg( m_strErrMsg ) ;
							}
						}
						else if ( !(typeSrcTemp.m_dwFlags & ECSTypeInfo::flagDeterministic)
							|| !pDstClassInf->IsMatchEnumeratorValue( pSrcValue ) )
						{
							m_strErrMsg =
								EString(pDstClassInf->GetGlobalName())
									+ " 列挙型へ適合しない値の代入です" ;
							return	ESLErrorMsg( m_strErrMsg ) ;
						}
					}
				}
			}
			else
			{
				errVerifyType = eslErrSuccess ;
			}
			MakeCommitValueToNakedRegister( typeSrcTemp ) ;
			CompileCodeNakedConvertToInt64( typeSrcTemp ) ;
			typeResult = typeDst ;
			typeResult.SetLoadedRegister( typeSrcTemp ) ;
			return	errVerifyType ;
		}
		return	ESLErrorMsg
			( "内部エラー：基本型の変換でエラーが発生しました" ) ;
	}
	return	eslErrSuccess ;
}

// 整数型のキャスト命令出力（nakedモード）
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileNakedCastIntegerToInteger
	( ECSTypeInfo & typeDst, const ECSTypeInfo & typeSrc )
{
	if ( !m_modeNakedCode )
	{
		return	eslErrSuccess ;
	}
	if ( (typeDst.m_pValue == NULL)
		|| (typeDst.m_pValue->m_vtType != csvtInteger)
		|| (typeSrc.m_pValue == NULL)
		|| (typeSrc.m_pValue->m_vtType != csvtInteger) )
	{
		return	ESLErrorMsg
			( "内部エラー：整数型キャストの対象が整数でありません" ) ;
	}
	//
	ECSInteger *	pIntDst = (ECSInteger*) typeDst.m_pValue ;
	ECSInteger *	pIntSrc = (ECSInteger*) typeSrc.m_pValue ;
	//
	if ( typeSrc.m_dwFlags & ECSTypeInfo::flagDeterministic )
	{
		INT64	nValue = pIntDst->GetValue() ;
		switch ( pIntDst->GetIntegerType() )
		{
		case	csvtInteger:
			break ;
		case	csvtBoolean:
			pIntDst->SetValue( (nValue != 0) ? -1 : 0 ) ;
			break ;
		case	csvtInt32:
			pIntDst->SetValue( (SDWORD) nValue ) ;
			break ;
		case	csvtUint32:
			pIntDst->SetValue( (DWORD) nValue ) ;
			break ;
		case	csvtInt16:
			pIntDst->SetValue( (SWORD) nValue ) ;
			break ;
		case	csvtUint16:
			pIntDst->SetValue( (WORD) nValue ) ;
			break ;
		case	csvtInt8:
			pIntDst->SetValue( (SBYTE) nValue ) ;
			break ;
		case	csvtUint8:
			pIntDst->SetValue( (BYTE) nValue ) ;
			break ;
		}
	}
	else if ( typeDst.IsLoadedRegister() )
	{
		int	regNum = typeDst.GetLoadedRegister() ;
		CSVariableType	csvtDst = pIntDst->GetIntegerType() ;
		CSVariableType	csvtSrc = pIntSrc->GetIntegerType() ;
		if ( csvtDst != csvtSrc )
		{
			switch ( csvtDst )
			{
			case	csvtInteger:
				break ;
			case	csvtBoolean:
				break ;
			case	csvtInt32:
				if ( (pIntSrc->SizeOf() == 64) || !pIntSrc->IsSign() )
				{
					m_pcsxi->WriteSakuraOperandRegReg
						( ECSSakura2Processor::codeMoveSx32Reg, regNum, regNum ) ;
				}
				break ;
			case	csvtUint32:
				if ( (pIntSrc->SizeOf() == 64) || pIntSrc->IsSign() )
				{
					m_pcsxi->WriteSakuraAndRegReg
						( regNum, ECSSakura2Processor::regMaskLow32 ) ;
				}
				break ;
			case	csvtInt16:
				if ( (pIntSrc->SizeOf() >= 32) || !pIntSrc->IsSign() )
				{
					m_pcsxi->WriteSakuraOperandRegReg
						( ECSSakura2Processor::codeMoveSx16Reg, regNum, regNum ) ;
				}
				break ;
			case	csvtUint16:
				if ( (pIntSrc->SizeOf() >= 32) || pIntSrc->IsSign() )
				{
					m_pcsxi->WriteSakuraAndRegReg
						( regNum, ECSSakura2Processor::regMaskLow16 ) ;
				}
				break ;
			case	csvtInt8:
				if ( (pIntSrc->SizeOf() >= 16) || !pIntSrc->IsSign() )
				{
					m_pcsxi->WriteSakuraOperandRegReg
						( ECSSakura2Processor::codeMoveSx8Reg, regNum, regNum ) ;
				}
				break ;
			case	csvtUint8:
				if ( (pIntSrc->SizeOf() >= 16) || pIntSrc->IsSign() )
				{
					m_pcsxi->WriteSakuraAndRegReg
						( regNum, ECSSakura2Processor::regMaskLow8 ) ;
				}
				break ;
			}
		}
	}
	return	eslErrSuccess ;
}

// 即値のキャスト
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileImmidiateCast
	( ECSObject*& pResult,
		const ECSTypeInfo & typeCast, ECSObject * pValue, DWORD dwFlags )
{
	pResult = NULL ;
	//
	if ( typeCast.IsTypeReference() )
	{
		return	ESLErrorMsg( "即値は参照型へ変換できません" ) ;
	}
	if ( typeCast.IsTypePointer() )
	{
		return	ESLErrorMsg( "即値はポインタ型へ変換できません" ) ;
	}
	ECSTypeInfo	typeSrc
		( pValue->Duplicate(), ECSTypeInfo::flagDeterministic ) ;
	ECSTypeInfo::TypeMatchResult	typeMatch = typeCast.IsMatchType( typeSrc ) ;
	if ( !(dwFlags & castStatic)
		&& (typeMatch > ECSTypeInfo::typeNatualMatch) )
	{
		EWideString	wstrCastType ;
		typeCast.FormatTypeString( wstrCastType ) ;
		if ( typeMatch == ECSTypeInfo::typeNoMatch )
		{
			m_strErrMsg = EString(wstrCastType) + " へ変換できません" ;
		}
		else
		{
			m_strErrMsg = EString(wstrCastType) + " へのルーズな変換です" ;
		}
		return	ESLErrorMsg( m_strErrMsg ) ;
	}
	if ( typeCast.IsTypeInteger() )
	{
		//
		// 整数型への変換
		//
		ECSInteger *	pIntCast = (ECSInteger*) typeCast.m_pValue ;
		ESLAssert( pIntCast != NULL ) ;
		ESLAssert( pIntCast->m_vtType == csvtInteger ) ;
		INT64	nValue ;
		if ( pValue->m_vtType == csvtInteger )
		{
			nValue = ((ECSInteger*)pValue)->GetValue() ;
		}
		else if ( pValue->m_vtType == csvtReal )
		{
			nValue = eslRoundR64ToLInt( ((ECSReal*)pValue)->m_varReal ) ;
		}
		else
		{
			return	ESLErrorMsg( "整数型への不正な変換です" ) ;
		}
		CSVariableType	csvtType = pIntCast->GetIntegerType() ;
		switch ( csvtType )
		{
		case	csvtBoolean:
			nValue = (nValue != 0) ? -1 : 0 ;
			break ;
		case	csvtInt8:
			nValue = (SBYTE) nValue ;
			break ;
		case	csvtUint8:
			nValue = (BYTE) nValue ;
			break ;
		case	csvtInt16:
			nValue = (SWORD) nValue ;
			break ;
		case	csvtUint16:
			nValue = (WORD) nValue ;
			break ;
		case	csvtInt32:
			nValue = (SDWORD) nValue ;
			break ;
		case	csvtUint32:
			nValue = (DWORD) nValue ;
			break ;
		case	csvtInteger:
		default:
			break ;
		}
		pResult = new ECSInteger( nValue, pIntCast->GetValueMask() ) ;
		return	eslErrSuccess ;
	}
	else if ( typeCast.IsTypeReal() )
	{
		//
		// 実数型への変換
		//
		ECSReal *	pRealCast = (ECSReal*) typeCast.m_pValue ;
		ESLAssert( pRealCast != NULL ) ;
		ESLAssert( pRealCast->m_vtType == csvtReal ) ;
		double	nValue ;
		if ( pValue->m_vtType == csvtInteger )
		{
			nValue = (double) ((ECSInteger*)pValue)->GetValue() ;
		}
		else if ( pValue->m_vtType == csvtReal )
		{
			nValue = ((ECSReal*)pValue)->m_varReal ;
		}
		else
		{
			return	ESLErrorMsg( "実数型への不正な変換です" ) ;
		}
		ECSReal *	pRealResult = new ECSReal( nValue ) ;
		pRealResult->m_vtRealType = pRealCast->m_vtRealType ;
		pResult = pRealResult ;
		return	eslErrSuccess ;
	}
	else if ( typeCast.IsTypeString() )
	{
		//
		// 文字列型への変換
		//
		ECSString *	pStrCast = (ECSString*) typeCast.m_pValue ;
		ESLAssert( pStrCast != NULL ) ;
		ESLAssert( pStrCast->m_vtType == csvtString ) ;
		if ( pValue->m_vtType != csvtString )
		{
			return	ESLErrorMsg( "文字列型への不正な変換です" ) ;
		}
		pResult = new ECSString( ((ECSString*)pValue)->m_varStr ) ;
		return	eslErrSuccess ;
	}
	return	ESLErrorMsg( "即値の不正な型変換です" ) ;
}

// クラスオブジェクトを構築する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileObjectConstruction
	( const ECSClassInfo & clsinf, const ECSTypeInfo & typeSrc )
{
	//
	// 構築関数を検索
	//
	EObjArray<ECSTypeInfo>				lstArg ;
	ECSClassInfo::ListMemberFunction	lstFunc ;
	if ( !typeSrc.IsVoid() )
	{
		lstArg.Add( new ECSTypeInfo( typeSrc ) ) ;
	}
	ECSClassInfo::MemberFunction *	pMemberFunc = NULL ;
	if ( !clsinf.SearchFunctinoAs
			( lstFunc, clsinf.GetName(), lstArg, 0,
				true, false, m_modeNakedCode ) )
	{
		if ( lstArg.GetSize() > 0 )
		{
			EWideString	wstrTypeForm ;
			typeSrc.FormatTypeString( wstrTypeForm ) ;
			//
			m_strErrMsg = EString( clsinf.GetGlobalName() )
				+ " クラスには " + EString( wstrTypeForm )
				+ " に一致する構築関数が見つかりませんでした。" ;
			return	ESLErrorMsg( m_strErrMsg ) ;
		}
	}
	else
	{
		pMemberFunc = lstFunc.GetAt(0) ;
		if ( pMemberFunc == NULL )
		{
			return	ESLErrorMsg
				( "内部エラー：適合する構築関数が見つかりません。" ) ;
		}
	}
	//
	// 構築オブジェクト生成
	//
	CSVariableType	csvtSrcType = clsinf.IsBasicType() ;
	//
	if ( csvtSrcType != csvtObject )
	{
		switch ( csvtSrcType )
		{
		case	csvtInteger:
			CompileImmediateInteger( 0 ) ;
			break ;
		case	csvtReal:
			CompileImmediateReal( 0.0 ) ;
			break ;
		case	csvtString:
			CompileImmediateString( L"" ) ;
			break ;
		default:
			return	ESLErrorMsg( "内部エラー：生成できない基本型です" ) ;
		}
	}
	else
	{
		ESLError	err =
			CompilerCodeCreateClassObject( clsinf.GetGlobalName() ) ;
		if ( err )
		{
			return	err ;
		}
	}
	if ( pMemberFunc == NULL )
	{
		return	eslErrSuccess ;
	}
	//
	CompileCodeSwap( 0, 1 ) ;
	//
	if ( clsinf.GetAttribute() & ECSTypeInfo::flagNativeObject )
	{
		CompileCodeStore( csotNop ) ;
		return	eslErrSuccess ;
	}
	//
	// 構築関数呼び出し
	//
	CompileLoadStackObject( lstArg.GetSize() ) ;
	//
	if ( lstArg.GetSize() > 0 )
	{
		CompileCodeSwap( 0, 1 ) ;
	}
	//
	ECSTypeInfo	typeThis( new ECSStructure( &clsinf ), 0 ) ;
	ECSTypeInfo	typeThisRef ;
	typeThisRef.MakeReferenceOf( typeThis ) ;
	//
	return	CompileCallMemberFunction
		( clsinf, typeThisRef, *pMemberFunc,
			lstArg, lstArg.GetSize(), ECSTypeInfo::flagPublic ) ;
}

// デフォルトコンストラクタの有無
//////////////////////////////////////////////////////////////////////////////
bool ECSCompiler::IsDefaultConstructor( const ECSTypeInfo & typeinf )
{
	if ( typeinf.IsTypeArray() || typeinf.IsTypeHashArray()
		|| typeinf.IsTypeReference() || typeinf.IsAbstractType() )
	{
		return	false ;
	}
	const ECSClassInfo *
		pClassInf = GetNakedTypeClassInfo( typeinf ) ;
	if ( pClassInf == NULL )
	{
		return	false ;
	}
	return	IsDefaultConstructor( *pClassInf ) ;
}

bool ECSCompiler::IsDefaultConstructor( const ECSClassInfo & clsinf )
{
	if ( clsinf.GetAttribute() & ECSTypeInfo::flagNativeObject )
	{
		return	false ;
	}
	if ( clsinf.IsBasicType() != csvtObject )
	{
		return	false ;
	}
	EObjArray<ECSTypeInfo>				lstArg ;
	ECSClassInfo::ListMemberFunction	lstFunc ;
	return	clsinf.SearchFunctinoAs
			( lstFunc, clsinf.GetName(),
				lstArg, 0, true, false, m_modeNakedCode ) ;
}

// デフォルトコンストラクタ呼び出し
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileCallDefaultConstructor
				( const ECSTypeInfo & typeinf, bool fLoadThisRef )
{
	if ( !typeinf.IsPureType() )
	{
		return	eslErrSuccess ;
	}
	const ECSClassInfo *
		pClassInf = GetNakedTypeClassInfo( typeinf ) ;
	if ( pClassInf == NULL )
	{
		return	eslErrSuccess ;
	}
	if ( fLoadThisRef && m_modeNakedCode )
	{
		if ( pClassInf->GetAttribute() & ECSTypeInfo::flagNativeObject )
		{
			return	eslErrSuccess ;
		}
		if ( pClassInf->IsBasicType() != csvtObject )
		{
			return	eslErrSuccess ;
		}
		if ( !typeinf.IsLoadedRegister() )
		{
			ECSTypeInfo	typeTemp = typeinf ;
			MakeCommitValueToNakedRegister( typeTemp ) ;
			fLoadThisRef = false ;
		}
	}
	return	CompileCallDefaultConstructor( *pClassInf, fLoadThisRef ) ;
}

ESLError ECSCompiler::CompileCallDefaultConstructor
				( const ECSClassInfo & clsinf, bool fLoadThisRef )
{
	if ( clsinf.GetAttribute() & ECSTypeInfo::flagNativeObject )
	{
		return	eslErrSuccess ;
	}
	if ( clsinf.IsBasicType() != csvtObject )
	{
		return	eslErrSuccess ;
	}
	EObjArray<ECSTypeInfo>				lstArg ;
	ECSClassInfo::ListMemberFunction	lstFunc ;
	if ( !clsinf.SearchFunctinoAs
			( lstFunc, clsinf.GetName(),
				lstArg, 0, true, false, m_modeNakedCode ) )
	{
		m_strErrMsg =
			EString( clsinf.GetGlobalName() )
				+ " クラスのデフォルトコンストラクタは定義されていません" ;
		return	ESLErrorMsg( m_strErrMsg ) ;
	}
	if ( fLoadThisRef )
	{
		CompileLoadStackObject( lstArg.GetSize() ) ;
	}
	return	CompileCallObjectConstructor( clsinf, lstArg ) ;
}

// コンストラクタ呼び出し
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileCallObjectConstructor
	( const ECSClassInfo & clsinf,
				const EPtrObjArray<ECSTypeInfo> & lstArg )
{
	ECSClassInfo::ListMemberFunction	lstFunc ;
	ECSClassInfo::MemberFunction *		pMemberFunc = NULL ;
	if ( !clsinf.SearchFunctinoAs
			( lstFunc, clsinf.GetName(),
				lstArg, 0, true, false, m_modeNakedCode ) )
	{
		ECSTypeInfo *	pArgType = lstArg.GetAt(0) ;
		if ( (lstArg.GetSize() == 1) && (pArgType != NULL)
			&& (clsinf.GetVirtualFunctionCount() == 0)
			&& clsinf.IsNakedMemoryClass() && m_modeNakedCode )
		{
			const ECSClassInfo *
				pSrcClassInf = GetNakedPtrTypeClassInfo( *pArgType ) ;
			if ( (pSrcClassInf != NULL)
				&& (clsinf.GetGlobalName() == pSrcClassInf->GetGlobalName()) )
			{
				while ( pArgType->IsTypeReference2() )
				{
					CompileCodeNakedUncoverReference( *pArgType ) ;
				}
				m_pcsxi->WriteSakuraLoadInt64
					( AllocateExpressionRegister(),
							clsinf.GetNakedMemorySize() ) ;
				CompileNakedSystemCall( L"memmove", 3 ) ;
				return	eslErrSuccess ;
			}
		}
		m_strErrMsg =
			EString( clsinf.GetGlobalName() )
				+ " クラスに適合する構築関数が見つかりませんでした。" ;
		return	ESLErrorMsg( m_strErrMsg ) ;
	}
	pMemberFunc = lstFunc.GetAt(0) ;
	if ( pMemberFunc == NULL )
	{
		return	ESLErrorMsg
			( "内部エラー：構築関数が見つかりません。" ) ;
	}
	if ( clsinf.GetAttribute() & ECSTypeInfo::flagNativeObject )
	{
		if ( lstArg.GetSize() == 0 )
		{
			return	eslErrSuccess ;
		}
		if ( lstArg.GetSize() != 0 )
		{
			return	ESLErrorMsg
				( "Native なクラスの構築関数の引数が複数指定されています。" ) ;
		}
		if ( m_modeNakedCode )
		{
			return	ESLErrorMsg( "naked モードで不正な処理です" ) ;
		}
		CompileCodeStore( csotNop ) ;
		return	eslErrSuccess ;
	}
	ECSTypeInfo	typeThisRef ;
	typeThisRef.MakeReferenceOf
			( ECSTypeInfo( ( new ECSStructure( &clsinf ) ) ) ) ;
	//
	DWORD	dwArgCount ;
	NormalizeLoadedNakedArgument( pMemberFunc, lstArg, dwArgCount ) ;
	//
	ESLError	err = CompileCallMemberFunction
		( clsinf, typeThisRef, *pMemberFunc,
			lstArg, dwArgCount, ECSTypeInfo::flagPublic ) ;
	if ( err )
	{
		return	err ;
	}
	if ( !pMemberFunc->GetReturnType().IsVoid() )
	{
		if ( !m_modeNakedCode )
		{
			CompileCodeFreeStack() ;
		}
		return	OutputWarning
			( "デフォルトの構築関数に返り値が定義されています。",
				m_strFilePath, m_nLineNum ) ;
	}
	return	eslErrSuccess ;
}

// 型キャストできないエラーメッセージ生成
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::ErrorMsgTypeCast
	( const ECSTypeInfo & typeDst, const ECSTypeInfo & typeSrc )
{
	EWideString	wstrDstType, wstrSrcType ;
	typeDst.FormatTypeString( wstrDstType ) ;
	typeSrc.FormatTypeString( wstrSrcType ) ;
	m_strErrMsg = EString( wstrSrcType )
		+ " から " + EString( wstrDstType )
						+ " へ型変換できません。" ;
	return	ESLErrorMsg( m_strErrMsg ) ;
}

// メンバ関数呼出し命令生成
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileCallMemberFunction
	( const ECSClassInfo & clsinfThis,
		const ECSTypeInfo & typeThis,
		const ECSClassInfo::MemberFunction & func,
		const EPtrObjArray<ECSTypeInfo> & lstArg,
		DWORD dwArgCount, ECSTypeInfo::Flags flagScope, bool fNoVirtualCall )
{
	//
	// 関数修飾子・アクセススコープ判定
	//
	if ( (typeThis.m_dwFlags & ECSTypeInfo::flagConstant)
		&& !(func.GetAttribute()
				& (ECSTypeInfo::flagConstant | ECSTypeInfo::flagStatic)) )
	{
		return	ESLErrorMsg
			( "constなオブジェクトの"
					"非constなメンバ関数を呼び出しています。" ) ;
	}
	if ( func.GetAttribute() & ECSTypeInfo::flagStatic )
	{
		return	ESLErrorMsg
			( "内部エラー：static なメンバ関数を"
				"this オブジェクトを通じて呼び出しています。" ) ;
	}
	const DWORD	dwProtectedMask = ECSTypeInfo::flagProtectedMask ;
	DWORD	dwProtectedFlag = func.GetAttribute() & dwProtectedMask ;
	if ( dwProtectedFlag > (DWORD) (flagScope & dwProtectedMask) )
	{
		if ( dwProtectedFlag & ECSTypeInfo::flagPrivate )
		{
			return	ESLErrorMsg
				( "private なメンバ関数は呼び出すことが出来ません。" ) ;
		}
		return	ESLErrorMsg
			( "protected なメンバ関数は呼び出すことが出来ません。" ) ;
	}
	//
	// メンバ関数呼び出し
	//
	int	nFuncIndex = clsinfThis.FindFunctionIndex( &func ) ;
	if ( nFuncIndex < 0 )
	{
		m_strErrMsg = EString( "内部エラー：メンバ関数 " )
						+ EString( func.GetGlobalName() )
						+ " インデックスが見つかりません。" ;
		return	ESLErrorMsg( m_strErrMsg ) ;
	}
	int	nClassIndex =
		GetClassInfoIndex( clsinfThis.GetGlobalName() ) ;
	if ( nClassIndex < 0 )
	{
		m_strErrMsg = EString( "内部エラー：" )
			+ EString( clsinfThis.GetGlobalName() )
			+ " クラスインデックスが見つかりません。" ;
		return	ESLErrorMsg( m_strErrMsg ) ;
	}
	DWORD	dwFuncIndex = nFuncIndex ;
	DWORD	dwAllArgCount = dwArgCount + 1 ;
	//
	if ( !(func.GetAttribute() & ECSTypeInfo::flagVirtual) || fNoVirtualCall )
	{
		if ( clsinfThis.GetAttribute() & ECSTypeInfo::flagNativeObject )
		{
			if ( func.GetName().CompareLeft( L"operator " ) == 0 )
			{
				if ( m_modeNakedCode )
				{
					return	ESLErrorMsg
						( "naked モードで未対応の native オブジェクト操作です" ) ;
				}
				OPERATOR_INFO	opinf ;
				ESLError	err =
					GetOperatorInfo
						( opinf, (func.GetName().CharPtr() + 9),
												(dwArgCount == 0) ) ;
				if ( err )
				{
					return	ESLErrorMsg
						( "Native なクラスの不正な operator を"
									"呼び出そうとしています。" ) ;
				}
				switch ( opinf.opiType )
				{
				case	optMove:
					CompileCodeStore( opinf.optOperator ) ;
					if ( dwArgCount != 1 )
					{
						return	ESLErrorMsg
							( "内部エラー：Native なオブジェクトの"
									"不正な代入 operator 呼び出しです。" ) ;
					}
					break ;
				case	optGeneral:
					CompileCodeOperate( opinf.optOperator ) ;
					if ( dwArgCount != 1 )
					{
						return	ESLErrorMsg
							( "内部エラー：Native なオブジェクトの"
									"不正な２項 operator 呼び出しです。" ) ;
					}
					break ;
				case	optCompare:
					CompileCodeCompare( opinf.cptCompare ) ;
					if ( dwArgCount != 1 )
					{
						return	ESLErrorMsg
							( "内部エラー：Native なオブジェクトの"
									"不正な比較 operator 呼び出しです。" ) ;
					}
					break ;
				case	optUnary:
					m_pcsxi->WriteInstructionCode( csicUniOperate ) ;
					m_pcsxi->WriteUniOperatorTypeCode( opinf.uoptUnary ) ;
					if ( dwArgCount != 0 )
					{
						return	ESLErrorMsg
							( "内部エラー：Native なオブジェクトの"
									"不正な単項 operator 呼び出しです。" ) ;
					}
					break ;
				case	optReference:
					m_pcsxi->WriteInstructionCode( csicElementIndirect ) ;
					if ( dwArgCount != 1 )
					{
						return	ESLErrorMsg
							( "内部エラー：Native なオブジェクトの"
									"不正な operator [] 呼び出しです。" ) ;
					}
					break ;
				default:
					return	ESLErrorMsg
						( "内部エラー：Native なオブジェクトの"
									"不正な operator 呼び出しです。" ) ;
				}
				return	eslErrSuccess ;
			}
			else if ( (func.GetName() == clsinfThis.GetName())
											&& (dwArgCount == 1) )
			{
				if ( m_modeNakedCode )
				{
					return	ESLErrorMsg
						( "naked モードで未対応の native オブジェクト操作です" ) ;
				}
				CompileCodeStore( csotNop ) ;
				CompileCodeFreeStack() ;
				return	eslErrSuccess ;
			}
			else if ( func.GetAttribute() & ECSTypeInfo::flagNakedCall )
			{
				return	CompileCallGlobalFunction( func, lstArg, dwAllArgCount ) ;
			}
		}
		else
		{
			if ( func.m_wstrClass != clsinfThis.GetGlobalName() )
			{
				//
				// virtual でないメンバ関数呼び出しでの
				// this の親クラスへのキャスト
				//
				ECSTypeInfo	typeThisTemp = typeThis ;
				int	regThis = -1 ;
				if ( m_modeNakedCode )
				{
					regThis = GetExpressionRegister( dwArgCount ) ;
					if ( dwArgCount > 0 )
					{
						m_pcsxi->WriteSakuraMoveRegReg
								( AllocateExpressionRegister(), regThis ) ;
						typeThisTemp.SetLoadedRegister( regThis ) ;
						typeThisTemp.ClearAddressingInfo() ;
					}
				}
				else if ( dwArgCount > 0 )
				{
					CompileCodeSwap( 0, dwArgCount ) ;
				}
				ECSTypeInfo	typeCast ;
				ECSTypeInfo	typeParent
					( new ECSStructure
						( func.m_pClassCast->pClassInf ),
							(func.GetAttribute()
								& ECSTypeInfo::flagConstant) ) ;
				ESLError	err =
					CompileTypeCast
						( typeCast, typeParent,
							typeThisTemp, castAcceptRef, flagScope ) ;
				if ( err )
				{
					OutputError
						( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
				}
				if ( m_modeNakedCode )
				{
					MakeCommitValueToNakedRegister( typeCast ) ;
					if ( dwArgCount > 0 )
					{
						m_pcsxi->WriteSakuraMoveRegReg
							( regThis, typeCast.GetLoadedRegister() ) ;
						FreeExpressionRegister() ;
					}
					else if ( !typeThis.IsLoadedRegister() )
					{
						m_pcsxi->WriteSakuraMoveRegReg
							( regThis, typeCast.GetLoadedRegister() ) ;
						FreeExpressionRegister() ;
					}
				}
				else if ( dwArgCount > 0 )
				{
					CompileCodeSwap( 0, dwArgCount ) ;
				}
			}
			return	CompileCallGlobalFunction( func, lstArg, dwAllArgCount ) ;
		}
	}
	if ( /*func.m_pClassCast->pClassInf->*/ clsinfThis.GetAttribute()
								& ECSTypeInfo::flagNativeObject )
	{
		//
		// Native オブジェクトのメンバ呼び出し
		//
		if ( m_modeNakedCode )
		{
			ESLError	err =
				CompileNakedPushToObjectArgument( func, dwAllArgCount ) ;
			if ( err )
			{
				return	err ;
			}
		}
		m_pcsxi->WriteInstructionCode( csicCallNativeMember ) ;
		m_pcsxi->WriteCodeData( &dwAllArgCount, sizeof(DWORD) ) ;
		m_pcsxi->WriteClassIndex( nClassIndex ) ;
		m_pcsxi->WriteCodeData( &dwFuncIndex, sizeof(DWORD) ) ;
		//
		if ( m_modeNakedCode )
		{
			ESLError	err =
				CompileNakedPopFromReturnedObject( func, dwAllArgCount ) ;
			if ( err )
			{
				return	err ;
			}
		}
	}
	else if ( clsinfThis.IsNakedMemoryClass() )
	{
		//
		// Naked クラスオブジェクトのメンバ呼び出し
		//
		int	nVirtIndex = clsinfThis.GetVirtualFunctionCount( nFuncIndex ) ;
		//
		int	regFuncAddr = ECSSakura2Processor::regAcc ;
		if ( m_modeNakedCode || func.IsNakedCall() )
		{
			int	regVirtVec = ECSSakura2Processor::regAcc ;
			if ( m_modeNakedCode )
			{
				WriteSakuraLoadMemory
					( ECSSakura2Processor::addrBaseIndex,
						ECSSakura2Processor::dataInt64, regVirtVec,
							ECSSakura2Processor::regZeroPtr, 0,
							GetExpressionRegister( dwArgCount ), 0 ) ;
				WriteSakuraLoadMemory
					( ECSSakura2Processor::addrBaseOffset32,
						ECSSakura2Processor::dataInt64,
						regFuncAddr, regVirtVec, nVirtIndex * 8 ) ;
			}
			else
			{
				CompileLoadStackObject( dwArgCount ) ;
				CompileCodePointerToObject( 0 ) ;
				CompileCodeReferenceForPointer( csvtInteger ) ;
				//
				NakedModeSaver	saver( *this ) ;
				m_modeNakedCode = true ;
				//
				regFuncAddr = AllocateExpressionRegister() ;
				//
				m_pcsxi->WriteSakuraSysCallFunction( L"object_pop_int" ) ;
				//
				WriteSakuraLoadMemory
					( ECSSakura2Processor::addrBaseOffset32,
						ECSSakura2Processor::dataInt64,
						regFuncAddr, regVirtVec, nVirtIndex * 8 ) ;
				//
				if ( regFuncAddr != GetExpressionRegister() )
				{
					m_pcsxi->WriteSakuraMoveRegReg
						( GetExpressionRegister(), regFuncAddr ) ;
					regFuncAddr = GetExpressionRegister() ;
				}
			}
		}
		else
		{
			CompileLoadStackObject( dwArgCount ) ;
///			CompileLoadStackObject( 0 ) ;
			CompileCodePointerToObject( 0 ) ;
			CompileCodeReferenceForPointer( csvtInteger ) ;
			CompileCodePointerToAddress() ;
			CompileImmediateInteger( nVirtIndex * 8 ) ;
			CompileCodeOperate( csotAdd ) ;
			CompileCodeReferenceForPointer( csvtInteger ) ;
		}
		if ( func.IsNakedCall() )
		{
			const bool	modeNakedCode = m_modeNakedCode ;
			NakedModeSaver	saver( *this ) ;
			m_modeNakedCode = true ;
			//
			int	nTempPointerCount = 0 ;
			if ( !modeNakedCode )
			{
				ESLError	err =
					CompileNakedPushFromObjectArgument
						( func, lstArg, dwAllArgCount, nTempPointerCount ) ;
				if ( err )
				{
					return	err ;
				}
			}
			CompileNakedPushBeforeCall( dwAllArgCount ) ;
			m_pcsxi->WriteSakuraCallReg( regFuncAddr ) ;
			CompileNakedPopAfterCall( dwAllArgCount ) ;
			//
			if ( !modeNakedCode )
			{
				ESLError	err =
					CompileNakedPushObjectReturned
							( func, dwAllArgCount, nTempPointerCount ) ;
				if ( err )
				{
					return	err ;
				}
				FreeExpressionRegister() ;
			}
		}
		else
		{
			ESLError	err ;
			if ( m_modeNakedCode )
			{
				err = CompileNakedPushToObjectArgument
									( func, dwAllArgCount ) ;
				if ( err )
				{
					return	err ;
				}
				err = CompileNakedPushObject
					( ECSTypeInfo( new ECSInteger() ), regFuncAddr ) ;
				if ( err )
				{
					return	err ;
				}
			}
			m_pcsxi->WriteInstructionCode( csicCallFunctionPointer ) ;
			m_pcsxi->WriteCodeData( &dwAllArgCount, sizeof(DWORD) ) ;
			//
			if ( m_modeNakedCode )
			{
				err = CompileNakedPopFromReturnedObject
									( func, dwAllArgCount ) ;
				if ( err )
				{
					return	err ;
				}
			}
		}
	}
	else
	{
		//
		// ユーザークラスオブジェクトのメンバ呼び出し
		//
		if ( m_modeNakedCode )
		{
			ESLError	err =
				CompileNakedPushToObjectArgument( func, dwAllArgCount ) ;
			if ( err )
			{
				return	err ;
			}
		}
		m_pcsxi->WriteInstructionCode( csicCallMember ) ;
		m_pcsxi->WriteCodeData( &dwAllArgCount, sizeof(DWORD) ) ;
		m_pcsxi->WriteClassIndex( nClassIndex ) ;
		m_pcsxi->WriteCodeData( &dwFuncIndex, sizeof(DWORD) ) ;
		//
		if ( m_modeNakedCode )
		{
			ESLError	err =
				CompileNakedPopFromReturnedObject( func, dwAllArgCount ) ;
			if ( err )
			{
				return	err ;
			}
		}
	}
	if ( m_modeNakedCode && !func.GetReturnType().IsVoid() )
	{
		m_pcsxi->WriteSakuraMoveRegReg
			( AllocateExpressionRegister(), ECSSakura2Processor::regAcc ) ;
	}
	return	eslErrSuccess ;
}

// グローバル関数呼出し命令生成
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileCallGlobalFunction
	( const ECSPrototypeInfo & func,
		const EPtrObjArray<ECSTypeInfo> & lstArg, DWORD dwArgCount )
{
	if ( !func.IsNakedCall() )
	{
		//
		// object モード関数
		//
		if ( m_modeNakedCode )
		{
			ESLError	err =
				CompileNakedPushToObjectArgument( func, dwArgCount ) ;
			if ( err )
			{
				return	err ;
			}
		}
		if ( func.GetAttribute() & ECSTypeInfo::flagNativeObject )
		{
			//
			// native 関数呼び出し
			//
			/*
			m_pcsxi->WriteInstructionCode( csicCall ) ;
			m_pcsxi->WriteObjectModeCode( csomGlobal ) ;
			m_pcsxi->WriteCodeData( &dwArgCount, sizeof(DWORD) ) ;
			m_pcsxi->WriteConstantString( func.GetGlobalName() ) ;
			*/
			m_pcsxi->WriteInstructionCode( csicCallNativeFunction ) ;
			m_pcsxi->WriteCodeData( &dwArgCount, sizeof(DWORD) ) ;
			m_pcsxi->WriteNativeFunctionIndex( func.GetGlobalName() ) ;
		}
		else 
		{
			//
			// object モード関数呼び出し
			//
			m_pcsxi->WriteInstructionCode( csicExCall ) ;
			m_pcsxi->WriteCodeData( &dwArgCount, sizeof(DWORD) ) ;
			m_pcsxi->WriteObjectModeCode( csomImmediate ) ;
			m_pcsxi->WriteVariableTypeCode( csvtInteger ) ;
			m_pcsxi->WriteFunctionAddress( func.GetGlobalName() ) ;
		}
		if ( m_modeNakedCode )
		{
			ESLError	err =
				CompileNakedPopFromReturnedObject( func, dwArgCount ) ;
			if ( err )
			{
				return	err ;
			}
		}
	}
	else
	{
		//
		// naked モード関数
		//
		int	nTempPointerCount = 0 ;
		if ( !m_modeNakedCode )
		{
			ESLError	err =
				CompileNakedPushFromObjectArgument
					( func, lstArg, dwArgCount, nTempPointerCount ) ;
			if ( err )
			{
				return	err ;
			}
		}
		if ( func.GetAttribute() & ECSTypeInfo::flagNativeObject )
		{
			//
			// naked native 関数呼び出し
			//
			CompileNakedSystemCall( func.GetGlobalName(), dwArgCount ) ;
		}
		else
		{
			//
			// naked 関数呼び出し
			//
			CompileNakedCallGlobal( func.GetGlobalName(), dwArgCount ) ;
		}
		if ( !m_modeNakedCode )
		{
			ESLError	err =
				CompileNakedPushObjectReturned
					( func, dwArgCount, nTempPointerCount ) ;
			if ( err )
			{
				return	err ;
			}
		}
	}
	if ( m_modeNakedCode )
	{
		AddThrowListToCurrentNest( func ) ;
	}
	if ( m_modeNakedCode && !func.GetReturnType().IsVoid() )
	{
		m_pcsxi->WriteSakuraMoveRegReg
			( AllocateExpressionRegister(), ECSSakura2Processor::regAcc ) ;
	}
	return	eslErrSuccess ;
}

// 間接関数呼出し命令生成
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileCallIndirectFunction
	( int regFunc, const ECSPrototypeInfo & func, DWORD dwArgCount )
{
	if ( !func.IsNakedCall() )
	{
		return	ESLErrorMsg( "objected な関数は間接呼び出し出来ません" ) ;
	}
	else if ( !m_modeNakedCode || (regFunc < 0) )
	{
		ESLAssert( !m_modeNakedCode && (regFunc < 0) ) ;
		return	ESLErrorMsg( "object モードで間接呼び出しはできません" ) ;
	}
	else
	{
		//
		// naked モード関数
		//
		int	nTempPointerCount = 0 ;
		if ( func.GetAttribute() & ECSTypeInfo::flagNativeObject )
		{
			//
			// naked native 関数呼び出し
			//
			CompileNakedIndirectSystemCall( regFunc, dwArgCount ) ;
		}
		else
		{
			//
			// naked 関数呼び出し
			//
			CompileNakedIndirectCallGlobal( regFunc, dwArgCount ) ;
		}
	}
	if ( m_modeNakedCode )
	{
		AddThrowListToCurrentNest( func ) ;
	}
	if ( m_modeNakedCode && !func.GetReturnType().IsVoid() )
	{
		m_pcsxi->WriteSakuraMoveRegReg
			( AllocateExpressionRegister(), ECSSakura2Processor::regAcc ) ;
	}
	return	eslErrSuccess ;
}

// クラス情報取得
//////////////////////////////////////////////////////////////////////////////
ECSClassInfo * ECSCompiler::GetClassInfoAs( const wchar_t * pwszClassName ) const
{
	int	i, nCount ;
	nCount = m_nestCtrl.GetSize() ;
	for ( i = 0; i < nCount; i ++ )
	{
		EControlNest *	pNest = m_nestCtrl.GetLastAt( i ) ;
		if ( pNest != NULL )
		{
			ECSClassInfo *	pClassInf =
					pNest->m_wstaClassDef.GetAs( pwszClassName ) ;
			if ( pClassInf != NULL )
			{
				return	pClassInf ;
			}
			if ( pNest->m_rwType == rwTemplate )
			{
				break ;
			}
		}
	}
	//
	EPtrObjArray<const wchar_t>	lstNamespace ;
	GetUsingNamespaceList( lstNamespace ) ;
	//
	EWideString	wstrClassName ;
	nCount = lstNamespace.GetSize() ;
	for ( i = 0; i < nCount; i ++ )
	{
		const wchar_t *	pwszNamespace = lstNamespace.GetAt( i ) ;
		if ( pwszNamespace != NULL )
		{
			wstrClassName = pwszNamespace ;
			wstrClassName += L"::" ;
			wstrClassName += pwszClassName ;
			//
			ECSClassInfo *
				pClassInf = m_pcsxiDst->GetClassInfoAs( wstrClassName ) ;
			if ( pClassInf != NULL )
			{
				return	pClassInf ;
			}
		}
	}
	return	m_pcsxiDst->GetClassInfoAs( pwszClassName ) ;
}

// クラス情報取得
//////////////////////////////////////////////////////////////////////////////
const ECSClassInfo *
	ECSCompiler::GetNakedTypeClassInfo( const ECSTypeInfo & typeinf ) const
{
	return	GetTypeClassInfo( typeinf.GetNakedType() ) ;
}

const ECSClassInfo *
	ECSCompiler::GetNakedPtrTypeClassInfo( const ECSTypeInfo & typeinf ) const
{
	return	GetTypeClassInfo( typeinf.GetNakedPointerType() ) ;
}

const ECSClassInfo * ECSCompiler::GetTypeClassInfo
							( const ECSObject *	pType ) const
{
	if ( pType != NULL )
	{
		switch ( pType->m_vtType )
		{
		case	csvtObject:
			{
				const ECSStructure *
					pStruct = ESLTypeCast<ECSStructure,ECSObject>( pType ) ;
				if ( pStruct != NULL )
				{
					if ( pStruct->m_pClassInf != NULL )
					{
						return	pStruct->m_pClassInf ;
					}
				}
			}
			return	GetClassInfoAs( pType->GetTypeName() ) ;
		case	csvtReference:
			break ;
		case	csvtArray:
			return	GetClassInfoAs( L"Array" ) ;
		case	csvtHash:
			return	GetClassInfoAs( L"Hash" ) ;
		case	csvtInteger:
			return	GetClassInfoAs( L"Integer" ) ;
		case	csvtReal:
			return	GetClassInfoAs( L"Real" ) ;
		case	csvtString:
			return	GetClassInfoAs( L"String" ) ;
		}
	}
	return	NULL ;
}

// Boolean 型判定
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::VerifyTypeBoolean
					( const ECSTypeInfo & typeinf, bool fNoWarning )
{
	if ( !m_modeNakedCode && !(m_dwModeFlags & flagStrictStyle) )
	{
		return	eslErrSuccess ;
	}
	ECSTypeInfo	typeTemp = typeinf ;
	ECSObject *	pValue = typeTemp.GetNakedType() ;
	if ( pValue == NULL )
	{
		if ( (m_modeNakedCode
			|| (m_dwModeFlags & flagStrictStyle)) && !fNoWarning )
		{
			return	OutputWarning
				( "動的な変換：Reference を Boolean に変換します。",
								m_strFilePath, m_nLineNum ) ;
		}
		return	eslErrSuccess ;		// Reference の動的評価
	}
	if ( (pValue->m_vtType == csvtInteger)
		&& (typeinf.m_dwFlags & ECSTypeInfo::flagDeterministic) )
	{
		ECSInteger *	pIntValue = (ECSInteger*) pValue ;
		INT64	nValue = pIntValue->GetValue() ;
		if ( (nValue != 0) && (nValue != -1) )
		{
			pIntValue->SetValueMask( ECSInteger::m_maskBoolean ) ;
			pIntValue->SetValue( ((nValue == 0) ? 0 : -1) ) ;
			OutputWarning
				( "暗黙の変換：Integer を Boolean に変換します。",
									m_strFilePath, m_nLineNum ) ;
		}
	}
	if ( m_modeNakedCode )
	{
		CompileCodeNakedUncoverReference( typeTemp ) ;
		MakeCommitValueToNakedRegister( typeTemp ) ;
	}
	pValue = typeTemp.GetNakedType() ;
	if ( pValue != NULL )
	{
		switch ( pValue->m_vtType )
		{
		case	csvtInteger:
			if ( m_modeNakedCode || (m_dwModeFlags & flagStrictStyle) )
			{
				ECSInteger *	pIntValue = (ECSInteger*) pValue ;
				if ( typeinf.m_dwFlags & ECSTypeInfo::flagDeterministic )
				{
					INT64	nValue = pIntValue->GetValue() ;
					if ( (nValue == 0) || (nValue == -1) )
					{
						return	eslErrSuccess ;
					}
				}
				else if ( ((ECSInteger*)pValue)->GetValueMask()
										== ECSInteger::m_maskBoolean )
				{
					return	eslErrSuccess ;
				}
				else
				{
					m_pcsxi->WriteSakuraCmpNeRegReg
						( typeTemp.GetLoadedRegister(),
								ECSSakura2Processor::regIntZero ) ;
				}
				if ( !fNoWarning && !IsCStyleCompatibleMode() )
				{
					return	OutputWarning
						( "暗黙の変換：Integer を Boolean に変換します。",
										m_strFilePath, m_nLineNum ) ;
				}
			}
			return	eslErrSuccess ;

		case	csvtString:
			if ( (m_modeNakedCode
				|| (m_dwModeFlags & flagStrictStyle)) && !fNoWarning )
			{
				return	OutputWarning
					( "暗黙の変換：String を Boolean に変換します。",
									m_strFilePath, m_nLineNum ) ;
			}
			return	eslErrSuccess ;

		case	csvtObject:
			{
				const ECSClassInfo *
					pClassInf = GetNakedTypeClassInfo( typeTemp ) ;
				if ( pClassInf != NULL )
				{
					ECSTypeInfo	typeBoolean
						( new ECSInteger
							( 0, ECSInteger::m_maskBoolean ), 0 ) ;
					ECSTypeInfo	typeCast ;
					return	CompileTypeCast
						( typeCast, typeBoolean,
								typeTemp, 0, ECSTypeInfo::flagPublic ) ;
				}
				else if ( !m_modeNakedCode && !(m_dwModeFlags & flagStrictStyle) )
				{
					return	eslErrSuccess ;
				}
			}
			break ;

		case	csvtReference:
			if ( ((ECSReference*)pValue)->m_pRef == NULL )
			{
				if ( (m_modeNakedCode
					|| (m_dwModeFlags & flagStrictStyle)) && !fNoWarning )
				{
					return	OutputWarning
						( "動的な変換：Reference を Boolean に変換します。",
										m_strFilePath, m_nLineNum ) ;
				}
				return	eslErrSuccess ;
			}
			break ;

		case	csvtPointer:
			if ( m_modeNakedCode )
			{
				m_pcsxi->WriteSakuraCmpNeRegReg
					( typeTemp.GetLoadedRegister(),
							ECSSakura2Processor::regIntZero ) ;
			}
			else
			{
				m_pcsxi->WriteInstructionCode( csicExUniOperate ) ;
				m_pcsxi->WriteExUniOperatorTypeCode( csxuotBoolean ) ;
			}
			return	eslErrSuccess ;

		default:
			break ;
		}
	}
	return	ESLErrorMsg( "Boolean に変換できません。" ) ;
}

// naked モードグローバルスクリプト関数呼び出し命令生成
//（レジスタ保存＆引数渡し＆コール）
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::CompileNakedCallGlobal
	( const wchar_t * pwszFuncName, DWORD dwArgCount )
{
	CompileNakedPushBeforeCall( dwArgCount ) ;
	//
	if ( m_flagFarCall )
	{
		m_pcsxi->WriteSakuraLoadInt64_FuncPtr
				( ECSSakura2Processor::regAcc, pwszFuncName ) ;
		m_pcsxi->WriteSakuraCallReg( ECSSakura2Processor::regAcc ) ;
	}
	else
	{
		m_pcsxi->WriteSakuraCallFunction( pwszFuncName ) ;
	}
	//
	CompileNakedPopAfterCall( dwArgCount ) ;
}

void ECSCompiler::CompileNakedIndirectCallGlobal
	( int regFunc, DWORD dwArgCount )
{
	CompileNakedPushBeforeCall( dwArgCount, regFunc ) ;
	//
	m_pcsxi->WriteSakuraCallReg( regFunc ) ;
	//
	FreeExpressionRegister() ;
	CompileNakedPopAfterCall( dwArgCount, regFunc ) ;
}

// naked モード・グローバル native 関数呼び出し命令生成
//（引数渡し＆コール＆後始末）
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::CompileNakedSystemCall
	( const wchar_t * pwszFuncName, DWORD dwArgCount )
{
	CompileNakedPushArgumentToCall( dwArgCount ) ;
	//
	m_pcsxi->WriteSakuraSysCallFunction( pwszFuncName ) ;
	//
	CompileNakedFreeArgumentAfterCall( dwArgCount ) ;
}

void ECSCompiler::CompileNakedIndirectSystemCall
		( int regSysId, DWORD dwArgCount )
{
	CompileNakedPushArgumentToCall( dwArgCount ) ;
	//
	m_pcsxi->WriteSakuraSysCallIndirect( regSysId ) ;
	//
	FreeExpressionRegister() ;
	CompileNakedFreeArgumentAfterCall( dwArgCount ) ;
}

// naked モード関数引数のプッシュ
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::CompileNakedPushArgumentToCall( DWORD dwArgCount )
{
	NakedModeSaver	saver( *this ) ;
	m_modeNakedCode = true ;
	//
	// 引数をプッシュする
	//
	int	regArgFirst = GetExpressionRegister( dwArgCount - 1 ) ;
	if ( dwArgCount > 0 )
	{
		if ( dwArgCount == 1 )
		{
			m_pcsxi->WriteSakuraPushReg( regArgFirst ) ;
		}
		else
		{
			m_pcsxi->WriteSakuraPushRegsImm8( regArgFirst, dwArgCount ) ;
		}
	}
	for ( DWORD i = 0; i < dwArgCount; i ++ )
	{
		FreeExpressionRegister() ;
	}
}

// naked モード関数呼び出しの為に一時レジスタと関数引数のプッシュ
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::CompileNakedPushBeforeCall( DWORD dwArgCount, int regExcept )
{
	NakedModeSaver	saver( *this ) ;
	m_modeNakedCode = true ;
	//
	// 一時レジスタをプッシュする
	//
	CompileNakedPushExpressionTemporary( dwArgCount, regExcept ) ;
	//
	// 引数をプッシュする
	//
	CompileNakedPushArgumentToCall( dwArgCount ) ;
}

// naked モード関数引数スタック解放
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::CompileNakedFreeArgumentAfterCall( DWORD dwArgCount )
{
	NakedModeSaver	saver( *this ) ;
	m_modeNakedCode = true ;
	//
	// 引数スタックを解放する
	//
	if ( dwArgCount > 0 )
	{
		m_pcsxi->WriteSakuraAddSP( dwArgCount * 8 ) ;
	}
}

// naked モード関数呼び出し後に引数スタックの解放と一時レジスタをプッシュ
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::CompileNakedPopAfterCall( DWORD dwArgCount, int regExcept )
{
	NakedModeSaver	saver( *this ) ;
	m_modeNakedCode = true ;
	//
	// 引数スタックを解放する
	//
	CompileNakedFreeArgumentAfterCall( dwArgCount ) ;
	//
	// 一時レジスタをポップする
	//
	CompileNakedPopExpressionTemporary( dwArgCount, regExcept ) ;
}

// naked モード式計算用一時レジスタをプッシュ
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::CompileNakedPushExpressionTemporary
		( DWORD dwArgCount, int regExcept )
{
	if ( m_regExprAlloc > 0 )
	{
		int	regArgFirst = GetExpressionRegister( dwArgCount - 1 ) ;
		int	nCount = regArgFirst - ECSSakura2Processor::regExpr0 ;
		if ( nCount > 0 )
		{
			if ( nCount == 1 )
			{
				if ( (regExcept < 0)
					&& (regExcept != ECSSakura2Processor::regExpr0) )
				{
					m_pcsxi->WriteSakuraPushReg
						( ECSSakura2Processor::regExpr0 ) ;
				}
			}
			else if ( (regExcept >= 0)
				&& (regExcept == ECSSakura2Processor::regExpr0) )
			{
				m_pcsxi->WriteSakuraPushRegsImm8
					( ECSSakura2Processor::regExpr0 + 1, nCount - 1 ) ;
			}
			else if ( (regExcept >= 0)
				&& (regExcept == (ECSSakura2Processor::regExpr0 + nCount - 1)) )
			{
				m_pcsxi->WriteSakuraPushRegsImm8
					( ECSSakura2Processor::regExpr0, nCount - 1 ) ;
			}
			else
			{
				m_pcsxi->WriteSakuraPushRegsImm8
					( ECSSakura2Processor::regExpr0, nCount ) ;
			}
		}
	}
}

// naked モード式計算用一時レジスタをポップ
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::CompileNakedPopExpressionTemporary
							( DWORD dwArgCount, int regExcept )
{
	if ( m_regExprAlloc > 0 )
	{
		int	regArgFirst = GetExpressionRegister( /*dwArgCount*/ - 1 ) ;
		int	nCount = regArgFirst - ECSSakura2Processor::regExpr0 ;
		if ( nCount > 0 )
		{
			if ( nCount == 1 )
			{
				if ( (regExcept < 0)
					|| (regExcept != ECSSakura2Processor::regExpr0) )
				{
					m_pcsxi->WriteSakuraPopReg
						( ECSSakura2Processor::regExpr0 ) ;
				}
			}
			else if ( (regExcept >= 0)
				&& (regExcept == ECSSakura2Processor::regExpr0) )
			{
				m_pcsxi->WriteSakuraPopRegsImm8
					( ECSSakura2Processor::regExpr0 + 1, nCount - 1 ) ;
			}
			else if ( (regExcept >= 0)
				&& (regExcept == (ECSSakura2Processor::regExpr0 + nCount - 1)) )
			{
				m_pcsxi->WriteSakuraPopRegsImm8
					( ECSSakura2Processor::regExpr0, nCount - 1 ) ;
			}
			else
			{
				m_pcsxi->WriteSakuraPopRegsImm8
					( ECSSakura2Processor::regExpr0, nCount ) ;
			}
		}
	}
	m_pcsxi->ReloadAllRegisterAssigns() ;
}

// naked モードから object モード関数を呼び出すために
// naked 引数を object スタックにプッシュ
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileNakedPushToObjectArgument
	( const ECSPrototypeInfo & func, DWORD dwArgCount )
{
	int	nArgCount = (int) dwArgCount ;
	if ( func.IsThisCall() )
	{
		m_pcsxi->WriteSakuraPushReg
			( GetExpressionRegister( dwArgCount - 1 ) ) ;
		m_pcsxi->WriteSakuraSysCallFunction( L"object_push_reference" ) ;
		m_pcsxi->WriteSakuraAddSP( 8 ) ;
		nArgCount -- ;
	}
	/*
	if ( !func.GetReturnType().IsVoid()
		&& !func.GetReturnType().IsNakedPrimitiveDataType() )
	{
		return	ESLErrorMsg
			( "naked モードから呼び出すことの"
				"できない object 関数の返り値型です" ) ;
	}
	*/
	if ( nArgCount != (int) func.GetArgumentCount() )
	{
		return	ESLErrorMsg( "内部エラー：関数の引数の数が一致しません" ) ;
	}
	int	i ;
	for ( i = 0; i < nArgCount; i ++ )
	{
		ECSTypeInfo *	pArgType = func.GetArgumentAt( i ) ;
		ESLAssert( pArgType != NULL ) ;
		if ( (pArgType == NULL)
			|| (pArgType->m_pValue == NULL) )
		{
			return	ESLErrorMsg( "内部エラー：関数の引数型がありません" ) ;
		}
		ESLError	err =
			CompileNakedPushObject
				( *pArgType, GetExpressionRegister( nArgCount - i - 1 ) ) ;
		if ( err )
		{
			return	err ;
		}
	}
	for ( i = 0; i < (int) dwArgCount; i ++ )
	{
		FreeExpressionRegister() ;
	}
	return	eslErrSuccess ;
}

// object モード関数の返り値を naked モードに変換
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileNakedPopFromReturnedObject
	( const ECSPrototypeInfo & func, DWORD dwArgCount )
{
	return	CompileNakedPopObject
				( func.GetReturnType(), ECSSakura2Processor::regAcc ) ;
}

// object モードから naked モード関数を呼び出すために
// object 引数を naked スタックにプッシュ
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileNakedPushFromObjectArgument
	( const ECSPrototypeInfo & func,
		const EPtrObjArray<ECSTypeInfo> & lstArg,
		DWORD dwArgCount, int & nPointerCount )
{
	NakedModeSaver	saver( *this ) ;
	m_modeNakedCode = true ;
	//
	int	i ;
	int	nArgCount = (int) dwArgCount ;
	int	nArgOffset = 0 ;
	for ( i = 0; i < nArgCount; i ++ )
	{
		AllocateExpressionRegister() ;
	}
	if ( func.IsThisCall() )
	{
//		nArgOffset = 1 ;	// lstArg[0] に this は入らないのでコメントアウト
		nArgCount -- ;
	}
	if ( func.IsArgumentToReturnObject() )
	{
		return	ESLErrorMsg
			( "object モードから呼び出すことの"
				"できない naked 関数の返り値型です" ) ;
	}
	nPointerCount = 0 ;
	for ( i = 0; i < nArgCount; i ++ )
	{
		ECSTypeInfo *	pArgType = func.GetArgumentAt( nArgCount - i - 1 ) ;
		ECSTypeInfo *	pArgObj = lstArg.GetAt( nArgCount - i - 1 - nArgOffset ) ;
		ESLAssert( pArgType != NULL ) ;
		ESLAssert( pArgObj != NULL ) ;
		if ( (pArgType == NULL)
			|| (pArgType->m_pValue == NULL)
			|| (pArgObj == NULL)
			|| (pArgObj->m_pValue == NULL) )
		{
			return	ESLErrorMsg( "内部エラー：関数の引数型がありません" ) ;
		}
//		if ( !pArgType->IsTypeReference()
//			&& pArgType->IsTypePointer()
//			&& !(pArgType->m_dwFlags & ECSTypeInfo::flagNakedBuffer) )
		if ( (pArgType->IsTypeReference() || pArgType->IsTypePointer())
			&& (pArgObj->IsTypeReference() || pArgObj->IsTypePointer())
			&& !(pArgObj->m_dwFlags & ECSTypeInfo::flagNakedBuffer) )
		{
			m_pcsxi->WriteSakuraSysCallFunction( L"object_pop_new" ) ;
			m_pcsxi->WriteSakuraPushReg( ECSSakura2Processor::regAcc ) ;
			m_pcsxi->WriteSakuraMoveRegReg
				( GetExpressionRegister( i ),
						ECSSakura2Processor::regAcc ) ;
			nPointerCount ++ ;
		}
		else
		{
			ESLError	err =
				CompileNakedPopObject
					( *pArgType, GetExpressionRegister( i ) ) ;
			if ( err )
			{
				return	err ;
			}
		}
	}
	if ( func.IsThisCall() )
	{
		m_pcsxi->WriteSakuraPushReg( ECSSakura2Processor::regIntZero ) ;
		m_pcsxi->WriteSakuraSysCallFunction( L"object_get_stack_last" ) ;
		m_pcsxi->WriteSakuraAddSP( 8 ) ;
		//
		m_pcsxi->WriteSakuraMoveRegReg
			( GetExpressionRegister( dwArgCount - 1 ),
								ECSSakura2Processor::regAcc ) ;
	}
	return	eslErrSuccess ;
}

// naked モード関数の返り値を object モードに変換
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileNakedPushObjectReturned
	( const ECSPrototypeInfo & func, DWORD dwArgCount, int nPointerCount )
{
	NakedModeSaver	saver( *this ) ;
	m_modeNakedCode = true ;
	//
	if ( func.IsThisCall() )
	{
		m_pcsxi->WriteSakuraPushReg( ECSSakura2Processor::regIntOne ) ;
		m_pcsxi->WriteSakuraSysCallFunction( L"object_stack_free" ) ;
		m_pcsxi->WriteSakuraAddSP( 8 ) ;
	}
	if ( !func.GetReturnType().IsVoid() )
	{
		ESLError	err =
			CompileNakedPushObject
				( func.GetReturnType(), ECSSakura2Processor::regAcc ) ;
		if ( err )
		{
			return	err ;
		}
	}
	for ( int i = 0; i < nPointerCount; i ++ )
	{
		m_pcsxi->WriteSakuraSysCallFunction( L"object_delete" ) ;
		m_pcsxi->WriteSakuraAddSP( 8 ) ;
	}
	return	eslErrSuccess ;
}

// naked レジスタを object スタックにプッシュ
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileNakedPushObject
	( const ECSTypeInfo & typeinf, int regSrc )
{
	if ( typeinf.IsVoid() || (typeinf.m_pValue == NULL) )
	{
		return	ESLErrorMsg( "void を object に変換しようとしました" ) ;
	}
	switch ( typeinf.m_pValue->m_vtType )
	{
	case	csvtInteger:
		m_pcsxi->WriteSakuraPushReg( regSrc ) ;
		m_pcsxi->WriteSakuraSysCallFunction( L"object_push_int" ) ;
		m_pcsxi->WriteSakuraAddSP( 8 ) ;
		break ;
	case	csvtReal:
		m_pcsxi->WriteSakuraPushReg( regSrc ) ;
		m_pcsxi->WriteSakuraSysCallFunction( L"object_push_double" ) ;
		m_pcsxi->WriteSakuraAddSP( 8 ) ;
		break ;
	case	csvtString:
		m_pcsxi->WriteSakuraPushReg( regSrc ) ;
		m_pcsxi->WriteSakuraSysCallFunction( L"object_push_string" ) ;
		m_pcsxi->WriteSakuraAddSP( 8 ) ;
		break ;
	case	csvtReference:
		if ( typeinf.m_dwFlags & ECSTypeInfo::flagNakedBuffer )
		{
			m_pcsxi->WriteSakuraPushReg( regSrc ) ;
			m_pcsxi->WriteSakuraSysCallFunction( L"object_push_int" ) ;
			m_pcsxi->WriteSakuraAddSP( 8 ) ;
		}
		else
		{
			CSVariableType	csvtNakedType =
				ECSTypeInfo::GetNakedMemoryType
					( ((ECSReference*)typeinf.m_pValue)->m_pRef ) ;
			if ( csvtNakedType == csvtObject )
			{
				m_pcsxi->WriteSakuraPushReg( regSrc ) ;
				m_pcsxi->WriteSakuraSysCallFunction( L"object_push_reference" ) ;
				m_pcsxi->WriteSakuraAddSP( 8 ) ;
			}
			else
			{
				m_pcsxi->WriteSakuraPushReg( regSrc ) ;
				m_pcsxi->WriteSakuraSysCallFunction( L"object_push_pointer" ) ;
				m_pcsxi->WriteSakuraAddSP( 8 ) ;
				//
				m_pcsxi->WriteInstructionCode( csicReferenceForPointer ) ;
				m_pcsxi->WriteVariableTypeCode( csvtNakedType ) ;
			}
		}
		break ;
	case	csvtPointer:
		if ( typeinf.m_dwFlags & ECSTypeInfo::flagNakedBuffer )
		{
			m_pcsxi->WriteSakuraPushReg( regSrc ) ;
			m_pcsxi->WriteSakuraSysCallFunction( L"object_push_int" ) ;
			m_pcsxi->WriteSakuraAddSP( 8 ) ;
		}
		else
		{
			m_pcsxi->WriteSakuraPushReg( regSrc ) ;
			m_pcsxi->WriteSakuraSysCallFunction( L"object_push_pointer" ) ;
			m_pcsxi->WriteSakuraAddSP( 8 ) ;
		}
		break ;
	default:
		m_pcsxi->WriteSakuraPushReg( regSrc ) ;
		m_pcsxi->WriteSakuraSysCallFunction( L"object_push_reference" ) ;
		m_pcsxi->WriteSakuraAddSP( 8 ) ;
		break ;
//		return	ESLErrorMsg( "object モードに変換できない naked 型です" ) ;
	}
	return	eslErrSuccess ;
}

// object スタックから naked レジスタにポップ
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileNakedPopObject
	( const ECSTypeInfo & typeinf, int regDst )
{
	if ( typeinf.IsVoid() || (typeinf.m_pValue == NULL) )
	{
		return	eslErrSuccess ;
	}
	CSVariableType	csvtType = typeinf.m_pValue->m_vtType ;
	if ( (csvtType != csvtInteger) && (csvtType != csvtReal) )
	{
		const ECSClassInfo *
			pClassInf = typeinf.m_pValue->m_pClassInf ;
		if ( pClassInf != NULL )
		{
			if ( pClassInf->IsIntegerEnumeratorType() )
			{
				csvtType = csvtInteger ;
			}
			else if ( pClassInf->IsRealEnumeratorType() )
			{
				csvtType = csvtReal ;
			}
		}
	}
	switch ( csvtType )
	{
	case	csvtInteger:
		m_pcsxi->WriteSakuraSysCallFunction( L"object_pop_int" ) ;
		break ;
	case	csvtReal:
		m_pcsxi->WriteSakuraSysCallFunction( L"object_pop_double" ) ;
		break ;
	case	csvtReference:
		if ( !(typeinf.m_dwFlags & ECSTypeInfo::flagNakedBuffer) )
		{
			OutputWarning3( "object 参照型を naked モードで変換しています" ) ;
			m_pcsxi->WriteSakuraSysCallFunction( L"object_pop_new" ) ;
		}
		else
		{
			m_pcsxi->WriteSakuraSysCallFunction( L"object_pop_int" ) ;
		}
		break ;
	case	csvtPointer:
		if ( !(typeinf.m_dwFlags & ECSTypeInfo::flagNakedBuffer) )
		{
			OutputWarning3( "object ポインタ型を naked モードで変換しています" ) ;
			m_pcsxi->WriteSakuraSysCallFunction( L"object_pop_new" ) ;
		}
		else
		{
			m_pcsxi->WriteSakuraSysCallFunction( L"object_pop_int" ) ;
		}
		break ;
	default:
		OutputWarning3( "object を naked モードで変換しています" ) ;
		m_pcsxi->WriteSakuraSysCallFunction( L"object_pop_new" ) ;
//		return	ESLErrorMsg( "naked モードに変換できない object 型です" ) ;
		break ;
	}
	m_pcsxi->WriteSakuraMoveRegReg( regDst, ECSSakura2Processor::regAcc ) ;
	return	eslErrSuccess ;
}

// 関数の引数を処理
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileArgument
	( EObjArray<ECSTypeInfo> & lstArgType,
		const ECSPrototypeInfo * pProto, ECSSourceStream & cssLine )
{
	ESLError	err ;
	int			iArgument = 0 ;
	int			regArgFirst = GetExpressionRegister( -1 ) ;
	wchar_t		wchNext ;
	for ( ; ; )
	{
		wchNext = cssLine.HasToComeChar( L",)" ) ;
		if ( wchNext == L')' )
		{
			break ;
		}
		ECSTypeInfo		typeArg ;
		ECSTypeInfo *	pArgType = NULL ;
		if ( pProto != NULL )
		{
			if ( (pProto->GetArgumentCount() <= (unsigned int) iArgument)
				&& !(pProto->GetAttribute()
									& ECSTypeInfo::flagVarArgument) )
			{
				return	ESLErrorMsg( "引数の数が多すぎます。" ) ;
			}
			pArgType = pProto->GetArgumentAt( iArgument ) ;
		}
		if ( wchNext == L',' )
		{
			//
			// 引数省略
			//
			ECSObject *	pDefaultValue = NULL ;
			if ( pProto != NULL )
			{
				pDefaultValue = pProto->GetArgumentDefaultAt( iArgument ) ;
				if ( pDefaultValue == NULL )
				{
					return	ESLErrorMsg
						( "デフォルト値の無い引数を省略しています。" ) ;
				}
				err = CompileImmediateObject( typeArg, pDefaultValue ) ;
				if ( err )
				{
					return	err ;
				}
				if ( pArgType != NULL )
				{
					ECSTypeInfo	typeArgCast ;
					DWORD	dwFlags = m_modeNakedCode ? 0 : castAcceptRef ;
					err = CompileTypeCast
						( typeArgCast, *pArgType, typeArg,
							dwFlags, ECSTypeInfo::flagPublic ) ;
					if ( err )
					{
						return	err ;
					}
					if ( m_modeNakedCode )
					{
						MakeCommitValueToNakedRegister( typeArgCast ) ;
						if ( typeArgCast.GetLoadedRegister()
										!= regArgFirst + iArgument )
						{
							int	regArg = AllocateExpressionRegister() ;
							ESLAssert( regArg == regArgFirst + iArgument ) ;
							if ( regArg != regArgFirst + iArgument )
							{
								OutputWarning
									( "内部エラー：関数の省略引数に"
										"割り当てるレジスタ番号が一致しません",
											m_strFilePath, m_nLineNum ) ;
							}
							m_pcsxi->WriteSakuraMoveRegReg
								( regArg, typeArgCast.GetLoadedRegister() ) ;
						}
					}
					//
					lstArgType.Add( new ECSTypeInfo( *pArgType ) ) ;
				}
				else
				{
					lstArgType.Add( new ECSTypeInfo( typeArg ) ) ;
				}
				iArgument ++ ;
				continue ;
			}
			CompileImmediateBasicVariable( csvtReference ) ;
			lstArgType.Add
				( new ECSTypeInfo
					( new ECSReference,
							ECSTypeInfo::flagDeterministic ) ) ;
			iArgument ++ ;
			continue ;
		}
		//
		// 引数式解釈
		//
		err = CompileExpression( typeArg, cssLine, 0, 0, L",)" ) ;
		if ( err )
		{
			return	err ;
		}
		if ( typeArg.IsVoid() )
		{
			return	ESLErrorMsg
				( "関数の引数として void データを渡しています。" ) ;
		}
		if ( pArgType != NULL )
		{
			ECSTypeInfo	typeArgType = *pArgType ;
			ECSTypeInfo	typeArgCast ;
			DWORD	dwFlags = castAcceptRef ;
			if ( m_modeNakedCode )
			{
				if ( ECSTypeInfo::GetObjectClassInfo
							( pArgType->m_pValue, *this ) != NULL )
				{
					typeArgType.MakeReferenceOf( *pArgType ) ;
				}
				dwFlags = 0 ;
			}
			else
			{
				typeArgType.m_dwFlags &= ~ECSTypeInfo::flagNakedBuffer ;
			}
			err = CompileTypeCast
				( typeArgCast, typeArgType, typeArg,
					dwFlags | castNoNakedPointer, ECSTypeInfo::flagPublic ) ;
			if ( err )
			{
				return	err ;
			}
			if ( m_modeNakedCode )
			{
				MakeCommitValueToNakedRegister( typeArgCast ) ;
				if ( typeArgCast.GetLoadedRegister()
								!= regArgFirst + iArgument )
				{
					int	regArg = AllocateExpressionRegister() ;
					ESLAssert( regArg == regArgFirst + iArgument ) ;
					if ( regArg != regArgFirst + iArgument )
					{
						OutputWarning
							( "内部エラー：関数の省略引数に"
								"割り当てるレジスタ番号が一致しません",
									m_strFilePath, m_nLineNum ) ;
					}
					m_pcsxi->WriteSakuraMoveRegReg
						( regArg, typeArgCast.GetLoadedRegister() ) ;
				}
				typeArgCast = *pArgType ;
			}
			lstArgType.Add( new ECSTypeInfo( typeArgCast ) ) ;
		}
		else
		{
			if ( m_modeNakedCode )
			{
				if ( (pProto != NULL)
					&& (pProto->GetAttribute() & ECSTypeInfo::flagVarArgument) )
				{
					while ( typeArg.IsTypeReference() )
					{
						CompileCodeNakedUncoverReference( typeArg ) ;
					}
				}
				else
				{
					while ( typeArg.IsTypeReference2() )
					{
						CompileCodeNakedUncoverReference( typeArg ) ;
					}
				}
				MakeCommitValueToNakedRegister( typeArg ) ;
			}
			lstArgType.Add( new ECSTypeInfo( typeArg ) ) ;
		}
		iArgument ++ ;
		//
		// 次の引数へ
		//
		wchNext = cssLine.HasToComeChar( L",)" ) ;
		if ( wchNext == L')' )
		{
			break ;
		}
		if ( wchNext != L',' )
		{
			return	ESLErrorMsg
				( "関数の引数が、, 記号で区切られていません。" ) ;
		}
	}
	if ( pProto != NULL )
	{
		//
		// 引数の数検証
		//
		int	nArgCount = pProto->GetArgumentCount() ;
		for ( ; iArgument < nArgCount; iArgument ++ )
		{
			ECSObject *
				pDefaultValue = pProto->GetArgumentDefaultAt( iArgument ) ;
			if ( pDefaultValue == NULL )
			{
				return	ESLErrorMsg
					( "デフォルト値の無い引数を省略しています。" ) ;
			}
			ECSTypeInfo		typeArg ;
			err = CompileImmediateObject( typeArg, pDefaultValue ) ;
			if ( err )
			{
				return	err ;
			}
			ECSTypeInfo *
				pArgType = pProto->GetArgumentAt( iArgument ) ;
			if ( pArgType != NULL )
			{
				ECSTypeInfo	typeArgCast ;
				DWORD	dwFlags = m_modeNakedCode ? 0 : castAcceptRef ;
				err = CompileTypeCast
					( typeArgCast, *pArgType, typeArg,
						dwFlags, ECSTypeInfo::flagPublic ) ;
				if ( err )
				{
					return	err ;
				}
				if ( m_modeNakedCode )
				{
					MakeCommitValueToNakedRegister( typeArgCast ) ;
					if ( typeArgCast.GetLoadedRegister()
									!= regArgFirst + iArgument )
					{
						int	regArg = AllocateExpressionRegister() ;
						ESLAssert( regArg == regArgFirst + iArgument ) ;
						if ( regArg != regArgFirst + iArgument )
						{
							OutputWarning
								( "内部エラー：関数の省略引数に"
									"割り当てるレジスタ番号が一致しません",
										m_strFilePath, m_nLineNum ) ;
						}
						m_pcsxi->WriteSakuraMoveRegReg
							( regArg, typeArgCast.GetLoadedRegister() ) ;
					}
				}
				lstArgType.Add( new ECSTypeInfo( *pArgType ) ) ;
			}
			else
			{
				MakeCommitValueToNakedRegister( typeArg ) ;
				lstArgType.Add( new ECSTypeInfo( typeArg ) ) ;
			}
		}
	}
	return	eslErrSuccess ;
}

// 既にレジスタにロードされた関数の引数を正規化（主にコンストラクタ用）
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::NormalizeLoadedNakedArgument
	( const ECSPrototypeInfo * pProto,
		const EPtrObjArray<ECSTypeInfo> & lstArgType, DWORD& dwArgCount )
{
	dwArgCount = lstArgType.GetSize() ;
	if ( !m_modeNakedCode )
	{
		return	eslErrSuccess ;
	}
	int	regArgFirst = GetExpressionRegister( lstArgType.GetSize() - 1 ) ;
	int	iArg ;
	for ( iArg = 0; iArg < (int) lstArgType.GetSize(); iArg ++ )
	{
		const ECSTypeInfo * pArgType = pProto->GetArgumentAt( iArg ) ;
		if ( pArgType == NULL )
		{
			return	ESLErrorMsg( "関数の引数が多すぎます" ) ;
		}
		ECSTypeInfo	typeArgType ;
		ECSTypeInfo	typeArgCast ;
		if ( ECSTypeInfo::GetObjectClassInfo( pArgType->m_pValue, *this ) != NULL )
		{
			typeArgType.MakeReferenceOf( *pArgType ) ;
		}
		else
		{
			typeArgType = *pArgType ;
		}
		ECSTypeInfo	typeArgSrc = lstArgType[iArg] ;
		typeArgSrc.ClearAddressingInfo() ;
		typeArgSrc.SetLoadedRegister( regArgFirst + iArg ) ;
		//
		ESLError	err = CompileTypeCast
			( typeArgCast, typeArgType,
				typeArgSrc, 0, ECSTypeInfo::flagPublic ) ;
		if ( err )
		{
			return	err ;
		}
		MakeCommitValueToNakedRegister( typeArgCast ) ;
		if ( typeArgCast.GetLoadedRegister() != regArgFirst + iArg )
		{
			ESLAssert( typeArgCast.GetLoadedRegister() == regArgFirst + iArg ) ;
			if ( typeArgCast.GetLoadedRegister() != regArgFirst + iArg )
			{
				OutputWarning
					( "内部エラー：関数の引数に"
						"割り当てるレジスタ番号が一致しません",
							m_strFilePath, m_nLineNum ) ;
			}
			m_pcsxi->WriteSakuraMoveRegReg
				( regArgFirst + iArg, typeArgCast.GetLoadedRegister() ) ;
			FreeExpressionRegister( typeArgCast ) ;
			typeArgCast.SetLoadedRegister( regArgFirst + iArg ) ;
		}
//		lstArgType[iArg] = typeArgCast ;
	}
	//
	// デフォルト値
	//
	int	nDefArg = 0 ;
	for ( iArg = lstArgType.GetSize();
					iArg < (int) pProto->GetArgumentCount(); iArg ++ )
	{
		ECSObject *	pDefArg = pProto->GetArgumentDefaultAt( iArg ) ;
		if ( pDefArg == NULL )
		{
			return	ESLErrorMsg( "関数の引数が少なすぎます" ) ;
		}
		const ECSTypeInfo *	pArgType = pProto->GetArgumentAt( iArg ) ;
		if ( pArgType == NULL )
		{
			continue ;
		}
		ECSTypeInfo	typeDefArg
			( ECSTypeInfo::DuplicateType( pDefArg ),
						ECSTypeInfo::flagDeterministic ) ;
		ECSTypeInfo	typeArgType ;
		ECSTypeInfo	typeArgCast ;
		if ( ECSTypeInfo::GetObjectClassInfo( pArgType->m_pValue, *this ) != NULL )
		{
			typeArgType.MakeReferenceOf( *pArgType ) ;
		}
		else
		{
			typeArgType = *pArgType ;
		}
		ESLError	err = CompileTypeCast
			( typeArgCast, typeArgType, typeDefArg,
				0, ECSTypeInfo::flagPublic ) ;
		if ( err )
		{
			return	err ;
		}
		MakeCommitValueToNakedRegister( typeArgCast ) ;
		if ( typeArgCast.GetLoadedRegister() != regArgFirst + iArg )
		{
			ESLAssert( typeArgCast.GetLoadedRegister() == regArgFirst + iArg ) ;
			if ( typeArgCast.GetLoadedRegister() != regArgFirst + iArg )
			{
				OutputWarning
					( "内部エラー：関数の引数に"
						"割り当てるレジスタ番号が一致しません",
							m_strFilePath, m_nLineNum ) ;
			}
			m_pcsxi->WriteSakuraMoveRegReg
				( regArgFirst + iArg, typeArgCast.GetLoadedRegister() ) ;
			typeArgCast.SetLoadedRegister( regArgFirst + iArg ) ;
		}
//		lstArgType[iArg] = typeArgCast ;
		dwArgCount ++ ;
	}
	return	eslErrSuccess ;
}

// 関数の引数を正規化（主に operator 用）
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::NormalizeNakedArgument
	( const ECSPrototypeInfo * pProto,
			EPtrObjArray<ECSTypeInfo> & lstArgType, DWORD& dwArgCount )
{
	if ( !m_modeNakedCode )
	{
		return	eslErrSuccess ;
	}
	ESLError	err ;
	int			iArgFirst = 0 ;
	int			regArgFirst = GetExpressionRegister( -1 ) ;
	dwArgCount = pProto->GetArgumentCount() ;
	//
	// this ポインタ準備
	//
	if ( pProto->IsThisCall() )
	{
		const ECSClassInfo *
			pClassInf = GetClassInfoAs( pProto->GetNameSpace() ) ;
		if ( pClassInf != NULL )
		{
			ECSTypeInfo	typeThis( new ECSStructure( pClassInf ) ) ;
			ECSTypeInfo	typeRefThis ;
			typeRefThis.MakeReferenceOf( typeThis ) ;
			typeRefThis.m_dwFlags |= ECSTypeInfo::flagConstant ;
			//
			ECSTypeInfo	typeArgCast ;
			DWORD	dwFlags = m_modeNakedCode ? 0 : castAcceptRef ;
			err = CompileTypeCast
				( typeArgCast, typeRefThis, lstArgType[0],
					dwFlags, ECSTypeInfo::flagPublic ) ;
			if ( err )
			{
				return	err ;
			}
			if ( m_modeNakedCode )
			{
				MakeCommitValueToNakedRegister( typeArgCast ) ;
				if ( typeArgCast.GetLoadedRegister() != regArgFirst )
				{
					int	regArg = AllocateExpressionRegister() ;
					ESLAssert( regArg == regArgFirst ) ;
					if ( regArg != regArgFirst )
					{
						OutputWarning
							( "内部エラー：関数の this 引数に"
								"割り当てるレジスタ番号が一致しません",
									m_strFilePath, m_nLineNum ) ;
					}
					m_pcsxi->WriteSakuraMoveRegReg
						( regArg, typeArgCast.GetLoadedRegister() ) ;
				}
			}
			if ( !(lstArgType[0].m_dwFlags & ECSTypeInfo::flagConstant) )
			{
				typeArgCast.m_dwFlags &= ~ECSTypeInfo::flagConstant ;
			}
			lstArgType[0] = typeArgCast ;
		}
		else
		{
			LoadCommitValueToNakedRegister
				( AllocateExpressionRegister(), lstArgType[0] ) ;
		}
		iArgFirst ++ ;
	}
	//
	// オブジェクトを返却する naked 関数のための処理
	//
	int	nArgAddition = 0 ;
	err = CompileNakedFuncPrepareToReturnObject( pProto, nArgAddition ) ;
	if ( err )
	{
		return	err ;
	}
	dwArgCount += nArgAddition ;
	regArgFirst += nArgAddition ;
	//
	// 引数準備
	//
	int	iArg ;
	for ( iArg = iArgFirst; iArg < (int) lstArgType.GetSize(); iArg ++ )
	{
		const ECSTypeInfo *
				pArgType = pProto->GetArgumentAt( iArg - iArgFirst ) ;
		if ( pArgType == NULL )
		{
			return	ESLErrorMsg( "関数の引数が多すぎます" ) ;
		}
		ECSTypeInfo	typeArgType ;
		ECSTypeInfo	typeArgCast ;
		if ( ECSTypeInfo::GetObjectClassInfo( pArgType->m_pValue, *this ) != NULL )
		{
			typeArgType.MakeReferenceOf( *pArgType ) ;
		}
		else
		{
			typeArgType = *pArgType ;
		}
		err = CompileTypeCast
			( typeArgCast, typeArgType, lstArgType[iArg],
				0, ECSTypeInfo::flagPublic ) ;
		if ( err )
		{
			return	err ;
		}
		MakeCommitValueToNakedRegister( typeArgCast ) ;
		if ( typeArgCast.GetLoadedRegister() != regArgFirst + iArg )
		{
			int	regArg = AllocateExpressionRegister() ;
			ESLAssert( regArg == regArgFirst + iArg ) ;
			if ( regArg != regArgFirst + iArg )
			{
				OutputWarning
					( "内部エラー：関数の引数に"
						"割り当てるレジスタ番号が一致しません",
							m_strFilePath, m_nLineNum ) ;
			}
			m_pcsxi->WriteSakuraMoveRegReg
				( regArg, typeArgCast.GetLoadedRegister() ) ;
		}
	}
	//
	// デフォルト値
	//
	int	nDefArg = 0 ;
	for ( iArg = lstArgType.GetSize() - iArgFirst;
					iArg < (int) pProto->GetArgumentCount(); iArg ++ )
	{
		ECSObject *	pDefArg = pProto->GetArgumentDefaultAt( iArg ) ;
		if ( pDefArg == NULL )
		{
			return	ESLErrorMsg( "関数の引数が少なすぎます" ) ;
		}
		const ECSTypeInfo *
				pArgType = pProto->GetArgumentAt( iArg ) ;
		if ( pArgType == NULL )
		{
			continue ;
		}
		ECSTypeInfo	typeDefArg
			( ECSTypeInfo::DuplicateType( pDefArg ),
						ECSTypeInfo::flagDeterministic ) ;
		ECSTypeInfo	typeArgType ;
		ECSTypeInfo	typeArgCast ;
		if ( ECSTypeInfo::GetObjectClassInfo( pArgType->m_pValue, *this ) != NULL )
		{
			typeArgType.MakeReferenceOf( *pArgType ) ;
		}
		else
		{
			typeArgType = *pArgType ;
		}
		err = CompileTypeCast
			( typeArgCast, typeArgType, typeDefArg,
				0, ECSTypeInfo::flagPublic ) ;
		if ( err )
		{
			return	err ;
		}
		MakeCommitValueToNakedRegister( typeArgCast ) ;
		if ( typeArgCast.GetLoadedRegister()
						!= regArgFirst + iArgFirst + iArg )
		{
			int	regArg = AllocateExpressionRegister() ;
			ESLAssert( regArg == regArgFirst + iArgFirst + iArg ) ;
			if ( regArg != regArgFirst + iArgFirst + iArg )
			{
				OutputWarning
					( "内部エラー：関数の引数に"
						"割り当てるレジスタ番号が一致しません",
							m_strFilePath, m_nLineNum ) ;
			}
			m_pcsxi->WriteSakuraMoveRegReg
				( regArg, typeArgCast.GetLoadedRegister() ) ;
		}
	}
	return	eslErrSuccess ;
}

// 直接オブジェクトを返す naked 関数の返り値を準備する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileNakedFuncPrepareToReturnObject
		( const ECSPrototypeInfo * pProto, int & nArgAddition )
{
	nArgAddition = 0 ;
	if ( pProto->IsNakedCall()
		&& pProto->IsArgumentToReturnObject() )
	{
		if ( !m_modeNakedCode )
		{
			return	ESLErrorMsg
				( "object モードから直接オブジェクトを"
					"返す naked 関数を呼び出すことは出来ません" ) ;
		}
		EControlNest *	pNest = m_nestCtrl.GetLastAt() ;
		ESLAssert( pNest != NULL ) ;
		if ( pNest == NULL )
		{
			return	ESLErrorMsg
				( "naked 関数内部でなければ、オブジェクトを"
					"直接返す naked 関数を呼び出すことは出来ません" ) ;
		}
		const ECSTypeInfo &	typeReturn = pProto->GetReturnType() ;
		//
		ECSTypeInfo	typeTemp ;
		CompilePrepareNakedTemporaryObject( typeTemp, typeReturn ) ;
		//
		nArgAddition ++ ;
	}
	return	eslErrSuccess ;
}

// 一時オブジェクトの領域を確保してメモリを初期化する
//////////////////////////////////////////////////////////////////////////////
ECSTypeInfo * ECSCompiler::CompilePrepareNakedTemporaryObject
					( ECSTypeInfo & typeTemp, const ECSTypeInfo & typeObj )
{
	ECSTypeInfo *	pTempVarType =
			CompileAllocateNakedTemporaryObject( typeObj ) ;
	if ( pTempVarType == NULL )
	{
		return	NULL ;
	}
	if ( m_modeTemporary )
	{
		int	regThis = AllocateExpressionRegister() ;
		typeTemp.MakeReferenceOf( typeObj ) ;
		typeTemp.ClearAddressingInfo() ;
		typeTemp.SetLoadedRegister( regThis ) ;
		return	pTempVarType ;
	}
	ESLError	err =
		CompilePrepareNakedLocalVariable( typeTemp, pTempVarType ) ;
	if ( err )
	{
		OutputError( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
	}
	return	pTempVarType ;
}

// 一時オブジェクトの領域を確保する（ローカル変数情報に一時領域を確保する）
//////////////////////////////////////////////////////////////////////////////
ECSTypeInfo * ECSCompiler::CompileAllocateNakedTemporaryObject
		( const ECSTypeInfo & typeObj )
{
	EControlNest *	pNest = m_nestCtrl.GetLastAt() ;
	ESLAssert( pNest != NULL ) ;
	if ( pNest == NULL )
	{
		OutputError
			( "naked 関数内部でなければ、"
				"一時オブジェクトを生成することは出来ません",
									m_strFilePath, m_nLineNum ) ;
		return	NULL ;
	}
	if ( m_modeTemporary )
	{
		ECSTypeInfo *	pTempVarType = new ECSTypeInfo( typeObj ) ;
		pTempVarType->SetAddressingInfo( ECSSakura2Processor::regBP, 0 ) ;
		m_lstLocalVarTemporary.Add( pTempVarType ) ;
		return	pTempVarType ;
	}
	return	CompileAllocateNakedLocalVariable( pNest, typeObj, L"<temp>" ) ;
}

// ローカル変数の領域を確保してメモリを初期化する（構築関数は呼び出さない）
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompilePrepareNakedLocalVariable
	( ECSTypeInfo & typeTemp, const ECSTypeInfo * pLocalVar, bool fNoGarbage )
{
	ESLAssert( m_modeNakedCode ) ;
	const int	regThis = AllocateExpressionRegister() ;
	LoadCommitValueToNakedRegister( regThis, *pLocalVar ) ;
	//
	const ECSClassInfo * pClassInf = pLocalVar->GetNakedMemoryClassInfo() ;
	if ( pClassInf != NULL )
	{
		const int	nElementCount = pLocalVar->CalcNakedMemoryArraySize() ;
		if ( nElementCount == 1 )
		{
			CompileCodeNakedClassInitialize( pClassInf ) ;
		}
		else if ( nElementCount > 1 )
		{
			const int	regCounter = regThis ;
			const int	regNextThis = AllocateExpressionRegister() ;
			const int	regTemp = AllocateExpressionRegister() ;
			//
			m_pcsxi->WriteSakuraMoveRegReg( regNextThis, regThis ) ;
			m_pcsxi->WriteSakuraAddRegRegImm32
				( regCounter, ECSSakura2Processor::regIntZero, nElementCount ) ;
			//
			DWORD	addrLoopBegin = CompileCodeGetCurrent() ;
			//
			m_pcsxi->WriteSakuraMoveRegReg( regTemp, regNextThis ) ;
			//
			CompileCodeNakedClassInitialize( pClassInf ) ;
			//
			m_pcsxi->WriteSakuraMoveRegReg
					( regTemp, ECSSakura2Processor::regIntZero ) ;
			m_pcsxi->WriteSakuraAddRegRegImm32
					( regCounter, regCounter, -1 ) ;
			m_pcsxi->WriteSakuraAddRegRegImm32
					( regNextThis, regNextThis, pClassInf->GetNakedMemorySize() ) ;
			m_pcsxi->WriteSakuraCmpNeRegReg( regTemp, regCounter ) ;
			//
			CompileCodeCommitJumpAddress
				( m_pcsxi->WriteSakuraCJumpOffset32( regTemp, 0 ), addrLoopBegin ) ;
			//
			FreeExpressionRegister() ;
			FreeExpressionRegister() ;
			//
			LoadCommitValueToNakedRegister( regThis, *pLocalVar ) ;
		}
	}
	else
	{
	}
	//
	typeTemp.MakeReferenceOf( *pLocalVar ) ;
	typeTemp.ClearAddressingInfo() ;
	typeTemp.SetLoadedRegister( regThis ) ;
	//
	if ( fNoGarbage )
	{
		return	eslErrSuccess ;
	}
	return	CompileCodeNakedAddGarbageList( pLocalVar ) ;
}

// ローカル変数の領域を確保する（ローカル変数情報を登録しアドレスを確定する）
//////////////////////////////////////////////////////////////////////////////
ECSTypeInfo * ECSCompiler::CompileAllocateNakedLocalVariable
		( EControlNest * pNest,
			const ECSTypeInfo & typeObj, const wchar_t * pwszVarName )
{
	ESLAssert( m_modeNakedCode ) ;
	UpdateNestLocalFrameBase() ;
	//
	pNest->m_nLocalSize =
		(pNest->m_nLocalSize
			+ typeObj.SizeOfOnNakedMemory() + 0x07) & ~0x07 ;
	//
	ECSTypeInfo *	pTempVarType = new ECSTypeInfo( typeObj ) ;
	//
	pNest->m_lstLocalObj.Add( pTempVarType ) ;
	pNest->m_lstLocalName.Add( pwszVarName ) ;
	//
	UpdateNestLocalFrameBase() ;
	//
	pTempVarType->SetAddressingInfo
		( ECSSakura2Processor::regBP,
			pNest->m_baseLocalFrame - pNest->m_nLocalSize ) ;
	//
	return	pTempVarType ;
}

// 一時領域 int64[n] を確保する（主にガベージリスト用）
//////////////////////////////////////////////////////////////////////////////
ECSTypeInfo * ECSCompiler::CompileAllocateNakedTemporaryBuffer( int nCount )
{
	ECSArray *	pArray = new ECSArray ;
	pArray->SetDefaultElement( new ECSInteger ) ;
	pArray->SetBounds( nCount ) ;
	//
	ECSTypeInfo	typeIntArray( pArray ) ;
	return	CompileAllocateNakedTemporaryObject( typeIntArray ) ;
}

// ユーザー定義マクロを展開
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileUserMacro
	( EMacroBlock * pmbMacro,
		const EObjArray<EWideString> & lstParam, ECSObject ** pRetValue )
{
	//
	// マクロ引数設定
	//
	EObjArray<ECSObject>	lstArg ;
	ESLError	err = eslErrSuccess ;
	int			i, nCount ;
	nCount = lstParam.GetSize( ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		if ( pmbMacro->m_fMacroFunc )
		{
			if ( (unsigned int) i >= pmbMacro->m_lstArgName.GetSize() )
			{
				err = ESLErrorMsg( "マクロ関数の引数の数が多すぎます。" ) ;
				break ;
			}
			ECSObject *		pValue = NULL ;
			ECSSourceStream	cssExpr = lstParam[i] ;
			err = CalculateExpression( pValue, cssExpr, 0, NULL, false ) ;
			if ( err )
			{
				break ;
			}
			/*
			if ( (pValue != NULL)
					&& (pValue->m_vtType == csvtReference) )
			{
				ECSObject *	pEntity = pValue->GetObjectEntity() ;
				if ( (pEntity != NULL)
					&& ((pEntity->m_vtType == csvtInteger)
						|| (pEntity->m_vtType == csvtReal)
						|| (pEntity->m_vtType == csvtString)) )
				{
					ECSObject *	pTemp = pValue ;
					pValue = pValue->Duplicate() ;
					delete	pTemp ;
				}
			}
			*/
			lstArg.Add( pValue ) ;
		}
		else
		{
			lstArg.Add( new ECSString( lstParam[i] ) ) ;
		}
	}
	if ( err )
	{
		if ( pRetValue != NULL )
		{
			*pRetValue = NULL ;
		}
		return	err ;
	}
	return	CompileUserMacro( pmbMacro, lstArg, pRetValue ) ;
}

ESLError ECSCompiler::CompileUserMacro
	( EMacroBlock * pmbMacro,
		EObjArray<ECSObject> & lstParam, ECSObject ** pRetValue )
{
	if ( m_nestMacro.GetSize() >= 256 )
	{
		return	ESLErrorMsg( "マクロ展開の入れ子が深すぎます。" ) ;
	}
	ESLError		err = eslErrSuccess ;
	int				i, nCount ;
	EMacroNest *	pmnMacroFunc = new EMacroNest( mwMacro ) ;
	pmnMacroFunc->m_fMacroFunction = pmbMacro->m_fMacroFunc ;
	pmnMacroFunc->m_fMacroStatement = !pmbMacro->m_fMacroFunc ;
	m_nestMacro.Add( pmnMacroFunc ) ;
	//
	EMacroNest *	pmnMacro = new EMacroNest( mwMacro ) ;
	unsigned int	nNestCount = m_nestMacro.GetSize( ) ;
	m_nestMacro.Add( pmnMacro ) ;
	//
	// マクロ引数設定
	//
	ECSArray *	pArg = NULL ;
	if ( !pmbMacro->m_fMacroFunc )
	{
		pArg = new ECSArray ;
		pmnMacro->m_staLocal.Add( L"@arg", pArg ) ;
	}
	nCount = lstParam.GetSize( ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		ECSObject *	pArgObj = lstParam.GetAt( 0 ) ;
		lstParam.DetachAt( 0 ) ;
		if ( pmbMacro->m_fMacroFunc )
		{
			pmnMacro->m_staLocal.SetAs
				( ECSWideString(pmbMacro->m_lstArgName[i]), pArgObj ) ;
		}
		else
		{
			pArg->m_varArray.Add( pArgObj ) ;
		}
	}
	//
	// マクロ展開
	//
	if ( !err )
	{
		nCount = pmbMacro->GetSize( ) ;
		for ( i = 0; i < nCount; i ++ )
		{
			ECSSourceStream *	pcssCode = pmbMacro->GetAt( i ) ;
			if ( pcssCode != NULL )
			{
				ECSSourceStream	cssCode = *pcssCode ;
				cssCode.MoveIndex( 0 ) ;
				err = CompileScriptLine( cssCode, m_nLineNum, NULL, false ) ;
				if ( err )
				{
					m_strErrMsg =
						EString( pmbMacro->m_wstrName )
							+ "(" + EString(i + 1) + "): "
							+ EString( GetESLErrorMsg(err) ) ;
					err = ESLErrorMsg( m_strErrMsg ) ;
					break ;
				}
			}
			if ( m_nestMacro.GetSize() == nNestCount )
			{
				break ;
			}
		}
	}
	m_nestMacro.SetSize( nNestCount ) ;
	//
	// マクロ関数返り値取得
	//
	ESLAssert( pmnMacroFunc == m_nestMacro.GetLastAt() ) ;
	if ( !err && (pRetValue != NULL) )
	{
		ECSObject *	pValue = pmnMacroFunc->m_staLocal.GetAs( L"@return" ) ;
		if ( pValue != NULL )
		{
			*pRetValue = pValue->Duplicate() ;
		}
		else
		{
			*pRetValue = NULL ;
		}
	}
	m_nestMacro.RemoveAt( m_nestMacro.GetSize() - 1 ) ;
	return	err ;
}

// マクロ関数引数を取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileMacroArgument
	( EObjArray<EWideString> & lstParam, ECSSourceStream & cssExpr )
{
	wchar_t	wch = cssExpr.HasToComeChar( L"(" ) ;
	if ( wch != L'(' )
	{
		return	ESLErrorMsg( "マクロ引数が指定されていません。" ) ;
	}
	wch = cssExpr.HasToComeChar( L")" ) ;
	while ( wch != L')' )
	{
		if ( cssExpr.IsIndexOverflow() )
		{
			return	ESLErrorMsg
				( "マクロ引数が丸括弧で閉じられていません。" ) ;
		}
		int	iBegin = cssExpr.GetIndex( ) ;
		int	iEnd = iBegin ;
		while ( !cssExpr.IsIndexOverflow() )
		{
			cssExpr.DisregardSpace( ) ;
			cssExpr.PassAExpressionTerm( m_dwModeFlags ) ;
			iEnd = cssExpr.GetIndex( ) ;
			wch = cssExpr.HasToComeChar( L",)" ) ;
			if ( wch )
			{
				break ;
			}
		}
		lstParam.Add( new EWideString
			( cssExpr.Middle( iBegin, iEnd - iBegin ) ) ) ;
	}
	return	eslErrSuccess ;
}

// シンボルを解釈（::結合とテンプレート）
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::ParseFullNameSymbol
	( SYMBOL_NAMESPACE& snsSymbol,
		ECSSourceStream & cssLine,
		bool fNoTemplateInstance, bool fAutoParseOperator,
		bool fAutoNormalizeNamespace )
{
	if ( m_dwModeFlags & flagCStyleBasicType )
	{
		bool	fSigned = false, fUnsigned = false ;
		int		nIndex = cssLine.GetIndex() ;
		if ( snsSymbol.wstrName == L"signed" )
		{
			fSigned = true ;
			snsSymbol.wstrName = cssLine.GetAToken() ;
			snsSymbol.wstrFullName = snsSymbol.wstrName ;
		}
		else if ( snsSymbol.wstrName == L"unsigned" )
		{
			fUnsigned = true ;
			snsSymbol.wstrName = cssLine.GetAToken() ;
			snsSymbol.wstrFullName = snsSymbol.wstrName ;
		}
		if ( IsCStyleBasicIntegerType( snsSymbol.wstrName ) )
		{
			if ( snsSymbol.wstrName == L"char" )
			{
				if ( fSigned )
				{
					snsSymbol.wstrName = L"int16" ;
				}
				else
				{
					snsSymbol.wstrName = L"uint16" ;
				}
				snsSymbol.wstrFullName = snsSymbol.wstrName ;
			}
			else if ( snsSymbol.wstrName == L"short" )
			{
				if ( fUnsigned )
				{
					snsSymbol.wstrName = L"uint16" ;
				}
				else
				{
					snsSymbol.wstrName = L"int16" ;
				}
				snsSymbol.wstrFullName = snsSymbol.wstrName ;
			}
			else if ( snsSymbol.wstrName == L"long" )
			{
				if ( fUnsigned )
				{
					snsSymbol.wstrName = L"uint64" ;
				}
				else
				{
					snsSymbol.wstrName = L"int64" ;
				}
				snsSymbol.wstrFullName = snsSymbol.wstrName ;
			}
			else if ( snsSymbol.wstrName == L"int" )
			{
				if ( m_dwModeFlags & flagCompatibleInt32 )
				{
					if ( fUnsigned )
					{
						snsSymbol.wstrName = L"uint32" ;
					}
					else
					{
						snsSymbol.wstrName = L"int32" ;
					}
				}
				else
				{
					if ( fUnsigned )
					{
						snsSymbol.wstrName = L"uint64" ;
					}
					else
					{
						snsSymbol.wstrName = L"int64" ;
					}
				}
				snsSymbol.wstrFullName = snsSymbol.wstrName ;
			}
			cssLine.HasToComeToken( L"int" ) ;
		}
		else if ( fSigned )
		{
			cssLine.MoveIndex( nIndex ) ;
			if ( m_dwModeFlags & flagCompatibleInt32 )
			{
				snsSymbol.wstrName = L"int32" ;
			}
			else
			{
				snsSymbol.wstrName = L"int64" ;
			}
			snsSymbol.wstrFullName = snsSymbol.wstrName ;
		}
		else if ( fUnsigned )
		{
			cssLine.MoveIndex( nIndex ) ;
			if ( m_dwModeFlags & flagCompatibleInt32 )
			{
				snsSymbol.wstrName = L"uint32" ;
			}
			else
			{
				snsSymbol.wstrName = L"uint64" ;
			}
			snsSymbol.wstrFullName = snsSymbol.wstrName ;
		}
	}
	bool	fTypename = false ;
	bool	fTypeNormalize = false ;
	if ( snsSymbol.wstrName == L"typename" )
	{
		snsSymbol.wstrFullName = snsSymbol.wstrName = cssLine.GetAToken() ;
		//
		EControlNest *	pTempNest =
			GetMostInnerNest( rwTemplate, rwEndTemplate ) ;
		if ( (pTempNest != NULL) && !pTempNest->m_fCommitBlock )
		{
			fTypename = true ;
		}
		else
		{
			fTypeNormalize = true ;
		}
	}
	for ( ; ; )
	{
		if ( fAutoParseOperator && (snsSymbol.wstrName == L"operator") )
		{
			const int		nIndex = cssLine.GetIndex() ;
			EWideString		wstrToken = cssLine.GetAToken() ;
			OPERATOR_INFO	opinf ;
			if ( GetOperatorInfo( opinf, wstrToken ) == eslErrSuccess )
			{
				wstrToken = L" " ;
				wstrToken += GetOperatorString( opinf ) ;
				//
				snsSymbol.wstrName += wstrToken ;
				snsSymbol.wstrFullName += wstrToken ;
			}
			else
			{
				ECSTypeInfo	typeinf ;
				cssLine.MoveIndex( nIndex ) ;
				if ( ParseTypeDescription( typeinf, cssLine ) != eslErrSuccess )
				{
					return	ESLErrorMsg( "operator が不正です" ) ;
				}
				EWideString	wstrTypeFormat ;
				typeinf.FormatTypeString( wstrTypeFormat ) ;
				//
				snsSymbol.wstrName += L" " + wstrTypeFormat ;
				snsSymbol.wstrFullName += L" " + wstrTypeFormat ;
			}
		}
		//
		ParseTemplateArgument( snsSymbol, cssLine, fNoTemplateInstance ) ;
		//
		if ( !cssLine.HasToComeToken( L"::" ) )
		{
			break ;
		}
		EWideString	wstrSymbol = cssLine.GetAToken() ;
		snsSymbol.wstrNamespace = snsSymbol.wstrFullName ;
		snsSymbol.wstrName = wstrSymbol ;
		if ( wstrSymbol == L"~" )
		{
			wstrSymbol = cssLine.GetAToken() ;
			snsSymbol.wstrName += wstrSymbol ;
		}
		if ( fTypeNormalize || fAutoNormalizeNamespace )
		{
			ECSTypeInfo	typeinf ;
			if ( GetSimpleTypeInfoAs
				( typeinf, snsSymbol.wstrNamespace ) == eslErrSuccess )
			{
				typeinf.FormatTypeString( snsSymbol.wstrNamespace ) ;
			}
		}
		snsSymbol.wstrFullName =
			snsSymbol.wstrNamespace + L"::" + snsSymbol.wstrName ;
		//
		if ( snsSymbol.wstrName != L"operator" )
		{
			ESLError	err = VerifyUserSymbol( wstrSymbol ) ;
			if ( err )
			{
				return	err ;
			}
		}
	}
	if ( fTypename )
	{
		EControlNest *	pTempNest =
			GetMostInnerNest( rwTemplate, rwEndTemplate ) ;
		if ( (pTempNest != NULL) && !pTempNest->m_fCommitBlock )
		{
			bool	fDefined = false ;
			ECSTypeInfo	typeinf ;
			if ( GetSimpleTypeInfoAs
				( typeinf, snsSymbol.wstrFullName ) == eslErrSuccess )
			{
				fDefined = true ;
			}
			if ( !fDefined )
			{
				pTempNest->m_wstaTypeDef.SetAs
					( snsSymbol.wstrFullName, new ECSTypeInfo() ) ;
			}
		}
	}
	else if ( fAutoNormalizeNamespace && !snsSymbol.wstrNamespace.IsEmpty() )
	{
		ECSTypeInfo	typeinf ;
		if ( GetSimpleTypeInfoAs
			( typeinf, snsSymbol.wstrNamespace ) == eslErrSuccess )
		{
			typeinf.FormatTypeString( snsSymbol.wstrNamespace ) ;
			snsSymbol.wstrFullName =
					snsSymbol.wstrNamespace + L"::" + snsSymbol.wstrName ;
		}
	}
	return	eslErrSuccess ;
}

void ECSCompiler::SYMBOL_NAMESPACE::ParseSymbol( const wchar_t * pwszName )
{
	int	iLastSep = 0 ;
	int	i = 0 ;
	int	nTemplateNest = 0 ;
	if ( pwszName != NULL )
	{
		while ( pwszName[i] != 0 )
		{
			if ( (nTemplateNest == 0)
					&& (pwszName[i] == L':') && (pwszName[i + 1] == L':') )
			{
				i += 2 ;
				iLastSep = i ;
			}
			else if ( pwszName[i] == L'<' )
			{
				nTemplateNest ++ ;
				i ++ ;
			}
			else if ( (pwszName[i] == L'>') && (nTemplateNest > 0) )
			{
				nTemplateNest -- ;
				i ++ ;
			}
			else
			{
				i ++ ;
			}
		}
		wstrFullName = pwszName ;
		wstrName = pwszName + iLastSep ;
		wstrNamespace = L"" ;
	}
	else
	{
		wstrFullName = pwszName ;
		wstrName = pwszName ;
		wstrNamespace = L"" ;
	}
	if ( iLastSep >= 2 )
	{
		wstrNamespace = wstrFullName.Left( iLastSep - 2 ) ;
	}
}

bool ECSCompiler::IsCStyleBasicIntegerType( const wchar_t * pwszName ) const
{
	static const wchar_t *	pwszBasicTypes[] =
	{
		L"char", L"short", L"int", L"long", NULL,
	} ;
	for ( int i = 0; pwszBasicTypes[i] != NULL; i ++ )
	{
		if ( EWideString::Compare( pwszBasicTypes[i], pwszName ) == 0 )
		{
			return	true ;
		}
	}
	return	false ;
}

const wchar_t *
	ECSCompiler::TranslateCStyleBasicIntegerType( const wchar_t * pwszName ) const
{
	static const wchar_t *	pwszBasicTypes[] =
	{
		L"char", L"short", L"int", L"long", NULL,
	} ;
	static const wchar_t *	pwszTranslatedTypes[] =
	{
		L"uint16", L"int16", L"Integer", L"int64", NULL,
	} ;
	for ( int i = 0; pwszBasicTypes[i] != NULL; i ++ )
	{
		if ( EWideString::Compare( pwszBasicTypes[i], pwszName ) == 0 )
		{
			if ( (m_dwModeFlags & flagCompatibleInt32)
				&& (EWideString::Compare( pwszBasicTypes[i], L"int" ) == 0) )
			{
				return	L"int32" ;
			}
			else
			{
				return	pwszTranslatedTypes[i] ;
			}
		}
	}
	return	NULL ;
}

// 型情報を取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::ParseTypeDescription
	( ECSTypeInfo & typeinf, ECSSourceStream & cssLine )
{
	EWideString	wstrToken ;
	const int	nIndex = cssLine.GetIndex() ;
	ESLError	err = ESLErrorMsg( "不正な型記述です。" ) ;
	do
	{
		//
		// 初期化
		//
		typeinf = ECSTypeInfo( NULL, 0 ) ;
		//
		// 型修飾子判別
		//
		DWORD	dwFlags = 0 ;
		wstrToken = cssLine.GetAToken( ) ;
		if ( CompareReservedWord( L"const", wstrToken ) == 0 )
		{
			dwFlags |= ECSTypeInfo::flagConstant ;
			wstrToken = cssLine.GetAToken( ) ;
		}
		if ( CompareReservedWord( L"void", wstrToken ) == 0 )
		{
			wchar_t	wch = cssLine.HasToComeChar( L"&*[" ) ;
			if ( wch == L'&' )
			{
				err = ESLErrorMsg( "void 型の参照型は不正な型記述です。" ) ;
				break ;
			}
			else if ( wch == L'[' )
			{
				err = ESLErrorMsg( "void 型の配列型は不正な型記述です。" ) ;
				break ;
			}
			else if ( wch == L'*' )
			{
				ECSPointer *	pPtr = new ECSPointer ;
				pPtr->m_fReadOnly = ((dwFlags & ECSTypeInfo::flagConstant) != 0) ;
				typeinf.m_pValue = pPtr ;
				err = ParseTypePointerDecoration( typeinf, cssLine ) ;
				if ( err )
				{
					break ;
				}
			}
			else
			{
				err = eslErrSuccess ;
				break ;
			}
		}
		else
		{
			//
			// 名前空間判定
			//
			SYMBOL_NAMESPACE	snsSymbol( wstrToken ) ;
			err = ParseFullNameSymbol( snsSymbol, cssLine ) ;
			if ( err )
			{
				break ;
			}
			//
			// 型名判別
			//
			err = SearchTypeName( snsSymbol, ECSTypeInfo::flagPublic ) ;
			if ( !err )
			{
				err = GetSimpleTypeInfoAs( typeinf, snsSymbol.wstrFullName ) ;
			}
			else
			{
				EPtrObjArray<const wchar_t>	lstNamespace ;
				EWideString	wstrThisClass = GetCurrentThisClassName() ;
				GetUsingNamespaceList( lstNamespace ) ;
				err = ESLErrorMsg( "不正な型指定です" ) ;
				//
				EWideString	wstrThisSpace ;
				for ( int i = 0; i < (int) lstNamespace.GetSize(); i ++ )
				{
					const wchar_t *	pwszNamespace = lstNamespace.GetAt( i ) ;
					if ( pwszNamespace != NULL )
					{
						wstrThisSpace = pwszNamespace ;
						ECSTypeInfo::Flags	flagScope = ECSTypeInfo::flagProtected ;
						if ( wstrThisClass != pwszNamespace )
						{
							flagScope = ECSTypeInfo::flagPublic ;
						}
						if ( !snsSymbol.wstrNamespace.IsEmpty() )
						{
							wstrThisSpace += L"::" ;
							wstrThisSpace += snsSymbol.wstrNamespace ;
							flagScope = ECSTypeInfo::flagPublic ;
						}
						SYMBOL_NAMESPACE	snsTemp ;
						snsTemp.wstrName = snsSymbol.wstrName ;
						snsTemp.wstrNamespace = wstrThisSpace ;
						//
						err = SearchTypeName( snsTemp, flagScope ) ;
						if ( !err )
						{
							err = GetSimpleTypeInfoAs
									( typeinf, snsTemp.wstrFullName ) ;
							if ( !err )
							{
								break ;
							}
						}
						err = ESLErrorMsg( "不正な型指定です" ) ;
					}
				}
			}
			if ( err )
			{
				break ;
			}
			typeinf.m_dwFlags |= dwFlags ;
			//
			// 型コンテナ判定
			//
			if ( cssLine.HasToComeChar( L"<" ) == L'<' )
			{
				if ( (typeinf.m_pValue->m_vtType == csvtReference)
					|| (typeinf.m_pValue->m_vtType == csvtPointer)
					|| (typeinf.m_pValue->m_vtType == csvtArray)
					|| (typeinf.m_pValue->m_vtType == csvtHash) )
				{
					ECSTypeInfo	typeContainer ;
					err = ParseTypeDescription( typeContainer, cssLine ) ;
					if ( err )
					{
						return	err ;
					}
					if ( cssLine.HasToComeChar( L">" ) != L'>' )
					{
						return	ESLErrorMsg
							( "型コンテナの閉じ括弧 > が見つかりません。" ) ;
					}
					//
					typeinf.m_dwFlags |= typeContainer.m_dwFlags ;
					//
					switch ( typeinf.m_pValue->m_vtType )
					{
					case	csvtReference:
						((ECSReference*)typeinf.m_pValue)->
								SetOwnObject( typeContainer.DetachValue() ) ;
						break ;
					case	csvtPointer:
						((ECSPointer*)typeinf.m_pValue)->
								SetOwnObject( typeContainer.DetachValue() ) ;
						((ECSPointer*)typeinf.m_pValue)->m_fReadOnly =
							((typeContainer.m_dwFlags & ECSTypeInfo::flagConstant) != 0) ;
						typeinf.m_dwFlags &= ~ECSTypeInfo::flagConstant ;
						break ;
					case	csvtArray:
						((ECSArray*)typeinf.m_pValue)->
							SetDefaultElement( typeContainer.DetachValue() ) ;
						break ;
					case	csvtHash:
						((ECSHash*)typeinf.m_pValue)->
							SetDefaultElement( typeContainer.DetachValue() ) ;
						break ;
					}
				}
				else
				{
					return	ESLErrorMsg
						( "型コンテナの記述が不正です。" ) ;
				}
			}
		}
		for ( ; ; )
		{
			//
			// ポインタ型判定
			//
			err = ParseTypePointerDecoration( typeinf, cssLine ) ;
			if ( err )
			{
				return	err ;
			}
			//
			// 参照型判定
			//
			if ( cssLine.HasToComeChar( L"&" ) == L'&' )
			{
				if ( typeinf.m_pValue->m_vtType == csvtReference )
				{
					OutputWarning0( "Reference 型の参照 & を指定しています。" ) ;
				}
				ECSReference *	pRef = new ECSReference ;
				pRef->SetOwnObject( typeinf.m_pValue ) ;
				typeinf.m_pValue = pRef ;
			}
			//
			// 配列判定
			//
			err = ParseTypeArrayDecoration( typeinf, cssLine ) ;
			if ( err )
			{
				break ;
			}
			//
			wchar_t	wchNext ;
			cssLine.DisregardSpace() ;
			wchNext = cssLine.CurrentCharacter() ;
			if ( (wchNext != L'[') && (wchNext != L'&') && (wchNext != L'*') )
			{
				break ;
			}
		}
		/*
		//
		// 参照型判定
		//
		while ( cssLine.HasToComeChar( L"&" ) == L'&' )
		{
			if ( typeinf.m_pValue->m_vtType == csvtReference )
			{
				OutputWarning( "Reference 型の参照 & を指定しています。" ) ;
			}
			ECSReference *	pRef = new ECSReference ;
			pRef->SetOwnObject( typeinf.m_pValue ) ;
			typeinf.m_pValue = pRef ;
		}
		*/
	}
	while ( false ) ;
	do
	{
		//
		// 関数型判定
		//
		int	iFuncTypeBegin = cssLine.GetIndex() ;
		if ( cssLine.HasToComeChar( L"(" ) == L'(' )
		do
		{
			ECSSourceStream	cssPtrDecr =
				cssLine.GetEnclosedString( L')', m_dwModeFlags ) ;
			if ( cssLine.GetAt( cssLine.GetIndex() - 1 ) != L')' )
			{
				cssLine.MoveIndex( iFuncTypeBegin ) ;
				break ;
			}
			if ( cssLine.HasToComeChar( L"(" ) != L'(' )
			{
				cssLine.MoveIndex( iFuncTypeBegin ) ;
				break ;
			}
			ECSFunction *	pFunc = new ECSFunction ;
			cssPtrDecr.DisregardSpace() ;
			wchar_t	wchNext = cssPtrDecr.CurrentCharacter() ;
			if ( !cssPtrDecr.IsCharacterSpace( wchNext )
				&& !cssPtrDecr.IsPunctuation( wchNext )
				&& !cssPtrDecr.IsSpecialPunctuaion( wchNext ) )
			{
				SYMBOL_NAMESPACE	snsSymbol( cssPtrDecr.GetAToken() ) ;
				for ( ; ; )
				{
					ParseTemplateArgument( snsSymbol, cssPtrDecr ) ;
					if ( err )
					{
						break ;
					}
					if ( !cssPtrDecr.HasToComeToken( L"::" ) )
					{
						break ;
					}
					cssPtrDecr.DisregardSpace() ;
					wchar_t	wchNext = cssPtrDecr.CurrentCharacter() ;
					if ( cssPtrDecr.IsCharacterSpace( wchNext )
						|| cssPtrDecr.IsPunctuation( wchNext )
						|| cssPtrDecr.IsSpecialPunctuaion( wchNext ) )
					{
						break ;
					}
					EWideString	wstrSymbol = cssPtrDecr.GetAToken() ;
					snsSymbol.wstrNamespace = snsSymbol.wstrFullName ;
					snsSymbol.wstrName = wstrSymbol ;
					snsSymbol.wstrFullName =
						snsSymbol.wstrNamespace + L"::" + snsSymbol.wstrName ;
				}
				pFunc->m_pThisCall =
					GetClassInfoAs( snsSymbol.wstrFullName ) ;
				if ( pFunc->m_pThisCall == NULL )
				{
					return	ESLErrorMsg( "関数型の this 型が不正です。" ) ;
				}
				pFunc->m_prototype.SetAttribute( ECSTypeInfo::flagThisCall ) ;
			}
			pFunc->m_prototype.SetReturnType( typeinf ) ;
			typeinf.SetTypeValue( pFunc, ECSTypeInfo::flagNakedBuffer ) ;
			//
			err = ParseArgumentDescription( pFunc->m_prototype, cssLine ) ;
			if ( err )
			{
				return	err ;
			}
			DWORD	dwFuncAttr = pFunc->m_prototype.GetAttribute() ;
			for ( ; ; )
			{
				if ( cssLine.HasToComeToken( L"const" ) )
				{
					dwFuncAttr |= ECSTypeInfo::flagConstant ;
				}
				else if ( cssLine.HasToComeToken( L"naked" ) )
				{
					dwFuncAttr |= ECSTypeInfo::flagNakedCall ;
				}
				else if ( cssLine.HasToComeToken( L"objected" ) )
				{
					dwFuncAttr |= ECSTypeInfo::flagObjectedCall ;
				}
				else if ( cssLine.HasToComeToken( L"native" ) )
				{
					dwFuncAttr |= ECSTypeInfo::flagNativeObject ;
				}
				else
				{
					break ;
				}
			}
			if ( !(dwFuncAttr & ECSTypeInfo::flagObjectedCall) )
			{
				if ( (m_dwModeFlags & flagDefaultNakedAll)
					|| ((m_dwModeFlags & flagDefualtNakedFunc)
						&& (pFunc->m_pThisCall != NULL)
						&& pFunc->m_pThisCall->IsNakedMemoryClass()) )
				{
					dwFuncAttr |= ECSTypeInfo::flagNakedCall ;
				}
			}
			pFunc->m_prototype.SetAttribute( dwFuncAttr ) ;
			//
			if ( dwFuncAttr & ECSTypeInfo::flagNakedCall )
			{
				pFunc->m_prototype.MakeNakedAttribute() ;
			}
			//
			err = ParseTypePointerDecoration( typeinf, cssPtrDecr ) ;
			if ( err )
			{
				return	err ;
			}
			if ( !cssPtrDecr.DisregardSpace() )
			{
				return	ESLErrorMsg( "関数型の修飾子が不正です" ) ;
			}
		}
		while ( false ) ;
	}
	while ( false ) ;
	//
	if ( err )
	{
		cssLine.MoveIndex( nIndex ) ;
	}
	return	err ;
}

ESLError ECSCompiler::ParseTypePointerDecoration
	( ECSTypeInfo & typeinf, ECSSourceStream & cssLine )
{
	if ( cssLine.HasToComeToken( L"const" ) )
	{
		typeinf.m_dwFlags |= ECSTypeInfo::flagConstant ;
	}
	while ( cssLine.HasToComeChar( L"*" ) == L'*' )
	{
		if ( typeinf.m_pValue->m_vtType == csvtReference )
		{
			OutputWarning0( "参照型へのポインタを指定しています。" ) ;
		}
		ECSPointer *	pPtr = new ECSPointer ;
		pPtr->SetOwnObject( typeinf.m_pValue ) ;
		pPtr->m_fReadOnly =
			((typeinf.m_dwFlags & ECSTypeInfo::flagConstant) != 0) ;
		typeinf.m_pValue = pPtr ;
		typeinf.m_dwFlags &= ~ECSTypeInfo::flagConstant ;
		//
		for ( ; ; )
		{
			if ( cssLine.HasToComeToken( L"const" ) )
			{
				typeinf.m_dwFlags |= ECSTypeInfo::flagConstant ;
			}
			else if ( cssLine.HasToComeToken( L"naked" ) )
			{
				typeinf.m_dwFlags |= ECSTypeInfo::flagNakedBuffer ;
			}
			else
			{
				break ;
			}
		}
	}
	return	eslErrSuccess ;
}

ESLError ECSCompiler::ParseTypeArrayDecoration
	( ECSTypeInfo & typeinf, ECSSourceStream & cssLine )
{
	ESLError		err = eslErrSuccess ;
	ENumArray<int>	lstDim ;
	while ( cssLine.HasToComeChar( L"[" ) == L'[' )
	{
		/*
		if ( typeinf.m_pValue->m_vtType == csvtArray )
		{
			OutputWarning( "Array 型の配列 [] を指定しています。" ) ;
		}
		*/
		if ( cssLine.HasToComeChar( L"]" ) != L']')
		{
			ECSObject *	pObjDim = NULL ;
			err = CalculateExpression( pObjDim, cssLine, 0, L"]" ) ;
			if ( err )
			{
				break ;
			}
			if ( (pObjDim == NULL)
				|| (pObjDim->m_vtType != csvtInteger) )
			{
				delete	pObjDim ;
				err = ESLErrorMsg
					( "配列要素数 [] に Integer 型以外の値が指定されています。" ) ;
				break ;
			}
			lstDim.Add( ((ECSInteger*)pObjDim)->GetInt() ) ;
			delete	pObjDim ;
			//
			if ( cssLine.HasToComeChar( L"]" ) != L']')
			{
				err = ESLErrorMsg
					( "配列要素指定に閉じ括弧 \']\' が見つかりません。" ) ;
				break ;
			}
		}
		else
		{
			lstDim.Add( 0x80000000 ) ;
		}
	}
	if ( err )
	{
		return	err ;
	}
	if ( lstDim.GetSize() > 0 )
	{
		unsigned int	nDim = lstDim.GetSize() ;
		unsigned int *	pBounds = new unsigned int [nDim] ;
		for ( unsigned int i = 0; i < nDim; i ++ )
		{
			pBounds[i] = lstDim[i] ;
		}
		//
		ECSArray *	pArray = new ECSArray ;
		pArray->MakeDimension( pBounds, nDim, typeinf.m_pValue ) ;
		//
		delete []	pBounds ;
		typeinf.m_pValue = pArray ;
	}
	return	eslErrSuccess ;
}

ECSObject * ECSCompiler::ParseBsaicType( const wchar_t * pwszName ) const
{
	struct	BASIC_TYPE_INFO
	{
		const wchar_t *	pwszTypeName ;
		CSVariableType	csvtType ;
		INT64			nSizeMask ;
		bool			fRealName ;
		bool			fNoCCompatible ;
	} ;
	static const BASIC_TYPE_INFO	btiTypeInOrder[] =
	{
		{ L"Array", csvtArray, 0, true, true },
		{ L"Boolean", csvtInteger, (INT64) 0x8000000000000000, false, false },
		{ L"Hash", csvtHash, 0, true, true },
		{ L"Int16", csvtInteger, (INT64) 0x8000000000007FFF, false, false },
		{ L"Int32", csvtInteger, (INT64) 0x800000007FFFFFFF, false, false },
		{ L"Int64", csvtInteger, -1, false, false },
		{ L"Int8", csvtInteger, (INT64) 0x800000000000007F, false, false },
		{ L"Integer", csvtInteger, -1, true, true },
		{ L"Pointer", csvtPointer, 0, true, true },
		{ L"Real", csvtReal, 0, true, true },
		{ L"Reference", csvtReference, 0, true, true },
		{ L"String", csvtString, 0, true, false },
		{ L"Uint16", csvtInteger, 0xFFFF, false, false },
		{ L"Uint32", csvtInteger, 0xFFFFFFFF, false, false },
		{ L"Uint64", csvtInteger, 0x7FFFFFFFFFFFFFFF, false, false },
		{ L"Uint8", csvtInteger, 0xFF, false, false },
		{ L"double", csvtReal, 0, false, false },
		{ L"float", csvtReal32, 0, false, false },
	} ;
	const int	nTypeCountInOrder =
		sizeof(btiTypeInOrder) / sizeof(btiTypeInOrder[0]) ;
	//
	if ( (m_dwModeFlags & flagCStyleBasicType)
			&& IsCStyleBasicIntegerType(pwszName) )
	{
		pwszName = TranslateCStyleBasicIntegerType( pwszName ) ;
	}
	CSVariableType	csvtType = csvtObject ;
	INT64			nSizeMask = -1 ;
	for ( int i = 0; i < nTypeCountInOrder; i ++ )
	{
		if ( IsCStyleCompatibleMode()
			&& btiTypeInOrder[i].fNoCCompatible )
		{
			continue ;
		}
		int	nCompare ;
		if ( btiTypeInOrder[i].fRealName )
		{
			nCompare =
				EWideString::Compare
					( btiTypeInOrder[i].pwszTypeName, pwszName ) ;
			if ( nCompare != 0 )
			{
				nCompare = CompareReservedWord
						( btiTypeInOrder[i].pwszTypeName, pwszName ) ;
			}
		}
		else
		{
			nCompare = CompareReservedWord
					( btiTypeInOrder[i].pwszTypeName, pwszName ) ;
		}
		if ( nCompare == 0 )
		{
			csvtType = btiTypeInOrder[i].csvtType ;
			nSizeMask = btiTypeInOrder[i].nSizeMask ;
			break ;
		}
	}
	switch ( csvtType )
	{
	case	csvtReference:
		return	new ECSReference() ;
	case	csvtInteger:
	case	csvtInteger64:
		return	new ECSInteger( 0, nSizeMask ) ;
	case	csvtReal:
	case	csvtReal64:
		return	new ECSReal() ;
	case	csvtReal32:
		{
			ECSReal *	pReal = new ECSReal ;
			pReal->m_vtRealType = csvtReal32 ;
			return	pReal ;
		}
	case	csvtString:
		return	new ECSString() ;
	case	csvtArray:
		return	new ECSArray() ;
	case	csvtHash:
		return	new ECSHash() ;
	case	csvtObject:
		break ;
	case	csvtPointer:
		return	new ECSPointer() ;
//	case	csvtFunction:
//		return	new ECSFunction() ;
	case	csvtBoolean:
		return	new ECSInteger( 0, 0x8000000000000000 ) ;
	case	csvtInt8:
		return	new ECSInteger( 0, 0x800000000000007F ) ;
	case	csvtUint8:
		return	new ECSInteger( 0, 0xFF ) ;
	case	csvtInt16:
		return	new ECSInteger( 0, 0x8000000000007FFF ) ;
	case	csvtUint16:
		return	new ECSInteger( 0, 0xFFFF ) ;
	case	csvtInt32:
		return	new ECSInteger( 0, 0x800000007FFFFFFF ) ;
	case	csvtUint32:
		return	new ECSInteger( 0, 0xFFFFFFFF ) ;
	}
	return	NULL ;
}

ESLError ECSCompiler::GetSimpleTypeInfoAs
		( ECSTypeInfo & typeinf, const wchar_t * pwszName )
{
	ECSObject *	pBasicType = ParseBsaicType( pwszName ) ;
	if ( pBasicType != NULL )
	{
		typeinf.SetTypeValue( pBasicType, 0 ) ;
		return	eslErrSuccess ;
	}
	int	iType = m_staTypeName.FindIndex( pwszName ) ;
	if ( (iType > csvtObject) && (iType < csvtMax) )
	{
		if ( !(m_dwModeFlags & flagCStyleBasicType) )
		{
			switch ( iType )
			{
			case	csvtReference:
				pBasicType = new ECSReference() ;
				break ;
			case	csvtInteger:
				pBasicType = new ECSInteger() ;
				break ;
			case	csvtReal:
				pBasicType = new ECSReal() ;
				break ;
			case	csvtString:
				pBasicType = new ECSString() ;
				break ;
			case	csvtArray:
				pBasicType = new ECSArray() ;
				break ;
			case	csvtHash:
				pBasicType = new ECSHash() ;
				break ;
			}
			if ( pBasicType != NULL )
			{
				typeinf.SetTypeValue( pBasicType, 0 ) ;
				return	eslErrSuccess ;
			}
		}
	}
	int	i, nCount = m_nestCtrl.GetSize() ;
	for ( i = 0; i < nCount; i ++ )
	{
		EControlNest *	pNest = m_nestCtrl.GetLastAt( i ) ;
		if ( pNest == NULL )
		{
			continue ;
		}
		ECSTypeInfo *	pTypeDef =
			pNest->m_wstaTypeDef.GetAs( pwszName ) ;
		if ( pTypeDef != NULL )
		{
			typeinf = *pTypeDef ;
			return	eslErrSuccess ;
		}
		if ( pNest->m_rwType == rwTemplate )
		{
			break ;
		}
	}
	ECSTypeInfo *	pTypeDef = m_wstaTypeDef.GetAs( pwszName ) ;
	if ( pTypeDef != NULL )
	{
		typeinf = *pTypeDef ;
		return	eslErrSuccess ;
	}
	ECSClassInfo *	pClassInf = GetClassInfoAs( pwszName ) ;
	if ( pClassInf != NULL )
	{
		ECSStructure *	pStruct = new ECSStructure ;
		pStruct->m_pwszTag = pClassInf->GetGlobalName() ;
		pStruct->m_pClassInf = pClassInf ;
		typeinf.SetTypeValue( pStruct, 0 ) ;
		return	eslErrSuccess ;
	}
	if ( iType >= 0 )
	{
		if ( (iType < csvtMax) && (m_dwModeFlags & flagCStyleBasicType) )
		{
			m_strErrMsg = "\'" + EString( pwszName )
						+ "\' は C 互換モードで使用できない型名です。" ;
			return	ESLErrorMsg( m_strErrMsg ) ;
		}
		ECSStructure *	pStruct = new ECSStructure ;
		pStruct->m_pwszTag = m_staTypeName.GetAt( iType ) ;
		typeinf.SetTypeValue( pStruct, 0 ) ;
		return	eslErrSuccess ;
	}
	m_strErrMsg = "\'" + EString( pwszName ) + "\' は不正な型名です。" ;
	return	ESLErrorMsg( m_strErrMsg ) ;
}

// テンプレート引数を解釈しインスタンス化
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::ParseTemplateArgument
	( SYMBOL_NAMESPACE& snsSymbol,
		ECSSourceStream & cssLine,
		bool fNotMakeInstance, bool fMustImplement, DWORD dwImplement )
{
	//
	// 名前空間検索
	//
	ETemplateDefinition *	pTemplate = NULL ;
	EPtrObjArray<const wchar_t>	lstNamespace ;
	GetUsingNamespaceList( lstNamespace ) ;
	//
	EWideString	wstrTempNamespace ;
	for ( int iNamespace = 0; iNamespace < (int) lstNamespace.GetSize(); iNamespace ++ )
	{
		const wchar_t *	pwszNamespace = lstNamespace.GetAt( iNamespace ) ;
		if ( pwszNamespace != NULL )
		{
			wstrTempNamespace = pwszNamespace ;
			wstrTempNamespace += L"::" ;
			wstrTempNamespace += snsSymbol.wstrFullName ;
			pTemplate = m_staTemplate.GetAs( wstrTempNamespace ) ;
			if ( pTemplate != NULL )
			{
				snsSymbol.wstrFullName = wstrTempNamespace ;
				if ( snsSymbol.wstrNamespace.IsEmpty() )
				{
					snsSymbol.wstrNamespace = pwszNamespace ;
				}
				else
				{
					snsSymbol.wstrNamespace =
						EWideString(pwszNamespace)
							+ L"::" + snsSymbol.wstrNamespace ;
				}
				break ;
			}
		}
	}
	if ( pTemplate == NULL )
	{
		pTemplate = m_staTemplate.GetAs( snsSymbol.wstrFullName ) ;
	}
	if ( pTemplate == NULL )
	{
		return ;
	}
	//
	// class/struct ブロック内でクラス自身をテンプレート引数に
	// 含んでいる場合のための準備
	//
	EControlNest *	pClassNest =
		GetMostInnerNest( rwStructure, rwEndUnion ) ;
	ECSClassInfo *	pNestClassInfo = NULL ;
	if ( pClassNest != NULL )
	{
		pNestClassInfo = GetClassInfoAs( pClassNest->m_wstrCurSpaceName ) ;
	}
	bool	fNestClass = false ;
	//
	// テンプレート引数を解釈
	//
	if ( cssLine.HasToComeChar( L"<" ) != L'<' )
	{
		return ;
	}
	EWideString	wstrTemplate ;
	wstrTemplate += L'<' ;
	//
	EControlNest *	pNestTemplate = new EControlNest( m_dwImplementFlags ) ;
	ESLError	err ;
	bool	fError = false ;
	for ( int iArg = 0; iArg < (int) pTemplate->m_arguments.GetSize(); iArg ++ )
	{
		ETemplateArgument *	pArg = pTemplate->m_arguments.GetAt( iArg ) ;
		ESLAssert( pArg != NULL ) ;
		if ( pArg == NULL )
		{
			continue ;
		}
		if ( iArg != 0 )
		{
			wstrTemplate += L',' ;
		}
		if ( pArg->m_type == templateClassArgument )
		{
			ECSTypeInfo	typeArg ;
			err = ParseTypeDescription( typeArg, cssLine ) ;
			if ( err )
			{
				fError = true ;
				OutputError
					( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
			}
			else
			{
				EWideString	wstrTypeArg ;
				typeArg.FormatTypeString( wstrTypeArg ) ;
				wstrTemplate += wstrTypeArg ;
				//
				pNestTemplate->m_wstaTypeDef.SetAs
					( pArg->m_name, new ECSTypeInfo(typeArg) ) ;
				//
				if ( (pNestClassInfo != NULL)
					&& (GetNakedTypeClassInfo( typeArg ) == pNestClassInfo) )
				{
					fNestClass = true ;
				}
			}
		}
		else if ( pArg->m_type == templateIntArgument )
		{
			ECSObject *	pValue = NULL ;
			const int	iArgFrom = cssLine.GetIndex() ;
			err = CalculateExpression( pValue, cssLine, 0, L",>" ) ;
			if ( err )
			{
				fError = true ;
				OutputError
					( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
			}
			else
			{
				if ( (pValue == NULL)
					|| (pValue->m_vtType != csvtInteger) )
				{
					fError = true ;
					OutputError
						( "テンプレート引数が整数ではありません",
										m_strFilePath, m_nLineNum ) ;
				}
				else
				{
					INT64	nValue = ((ECSInteger*)pValue)->GetValue() ;
					if ( IsInTemplateDeclaration() )
					{
						wstrTemplate +=
							cssLine.Middle( iArgFrom, cssLine.GetIndex() - iArgFrom ) ;
					}
					else
					{
						EWideString	wstrIntArg ;
						wstrIntArg.FromInteger( nValue ) ;
						wstrTemplate += wstrIntArg ;
					}
					pNestTemplate->m_staConstant.SetAs
						( pArg->m_name, new ECSInteger( nValue ) ) ;
				}
				delete	pValue ;
			}
		}
		else
		{
			fError = true ;
			OutputError
				( "内部エラー：テンプレート引数の型が不正です",
										m_strFilePath, m_nLineNum ) ;
		}
		if ( iArg + 1 < (int) pTemplate->m_arguments.GetSize() )
		{
			if ( cssLine.HasToComeChar( L"," ) != L',' )
			{
				fError = true ;
				OutputError
					( "テンプレート引数が \',\' で区切られていません",
											m_strFilePath, m_nLineNum ) ;
			}
		}
	}
	if ( cssLine.HasToComeChar( L">" ) != L'>' )
	{
		fError = true ;
		OutputError
			( "テンプレート引数が \'>\' で閉じられていません",
									m_strFilePath, m_nLineNum ) ;
	}
	wstrTemplate += L'>' ;
	if ( fError )
	{
		delete	pNestTemplate ;
		return ;
	}
	snsSymbol.wstrFullName += wstrTemplate ;
	snsSymbol.wstrName += wstrTemplate ;
	if ( fNotMakeInstance )
	{
		delete	pNestTemplate ;
		return ;
	}
	if ( m_pStatementCache != NULL )
	{
		EControlNest *	pCurTemplate =
			GetMostInnerNest( rwTemplate, rwEndTemplate ) ;
		if ( pCurTemplate != NULL )
		{
			//
			// テンプレート定義中の一時テンプレート型
			//
			ECSTypeInfo *	pTempType = new ECSTypeInfo ;
			unsigned int	nIndex ;
			pCurTemplate->m_wstaTypeDef.SetAs
						( snsSymbol.wstrFullName, pTempType ) ;
			if ( pCurTemplate->m_wstaTypeDef.GetAs
						( snsSymbol.wstrFullName, &nIndex ) != NULL )
			{
				EWideString *	pwstrName =
					pCurTemplate->m_wstaTypeDef.GetTagAt( nIndex ) ;
				if ( pwstrName != NULL )
				{
					ECSStructure *	pStruct = new ECSStructure ;
					pStruct->m_pwszTag = *pwstrName ;
					pTempType->SetTypeValue( pStruct, 0 ) ;
				}
			}
			delete	pNestTemplate ;
			return ;
		}
	}
	//
	// テンプレートインスタンスの定義済み判定
	//
	if ( !fMustImplement )
	{
		switch ( pTemplate->m_type )
		{
		case	templateFunction:
			if ( (m_wstaInlineFuncs.GetAs( snsSymbol.wstrFullName ) != NULL)
				|| (m_wstaInlineFuncs.GetAs
						( L"naked " + snsSymbol.wstrFullName ) != NULL) )
			{
				delete	pNestTemplate ;
				return ;
			}
			break ;
		case	templateClass:
		case	templateStruct:
			if ( GetClassInfoAs( snsSymbol.wstrFullName ) != NULL )
			{
				delete	pNestTemplate ;
				return ;
			}
			break ;
		}
	}
	//
	// class/struct ブロック内でクラス自身をテンプレート引数に
	// 含んでいる場合の処置
	//
	if ( fNestClass && !fMustImplement )
	{
		int	nCount = pClassNest->m_lstPostImplTemplate.GetSize() ;
		for ( int i = 0; i < nCount; i ++ )
		{
			EWideString *	pwstrInstance =
				pClassNest->m_lstPostImplTemplate.GetAt( i ) ;
			if ( (pwstrInstance != NULL)
				&& (*pwstrInstance == snsSymbol.wstrFullName) )
			{
				delete	pNestTemplate ;
				return ;
			}
		}
		pClassNest->m_lstPostImplTemplate.Add
				( new EWideString( snsSymbol.wstrFullName ) ) ;
		dwImplement = implementDeclaration ;
	}
	//
	// インスタンス化
	//
	pNestTemplate->m_rwType = rwTemplate ;
	pNestTemplate->m_fCommitBlock = true ;
	m_nestCtrl.Add( pNestTemplate ) ;
	//
	if ( !pTemplate->m_namespace.IsEmpty() )
	{
		pNestTemplate->m_lstUsingNamespace.Add
			( new EWideString( pTemplate->m_namespace ) ) ;
		//
		EWideString	wstrNamespace = pTemplate->m_namespace ;
		for ( ; ; )
		{
			SYMBOL_NAMESPACE	snsSymbol ;
			snsSymbol.ParseSymbol( wstrNamespace ) ;
			if ( snsSymbol.wstrNamespace.IsEmpty() )
			{
				break ;
			}
			pNestTemplate->m_lstUsingNamespace.Add
				( new EWideString( snsSymbol.wstrNamespace ) ) ;
			wstrNamespace = snsSymbol.wstrNamespace ;
		}
	}
	//
	ECSExecutionImageCompiler *	pcsxiTemp = m_pcsxi ;
	const DWORD	dwCodePos = m_pcsxiDst->m_bufImage.GetLength() ;
	const DWORD	dwCurCodePos = pcsxiTemp->m_bufImage.GetLength() ;
	const DWORD	dwInitCodePos = m_csxiInitFunc.m_bufImage.GetLength() ;
	const DWORD	dwGlobalSize = m_pcsxiDst->m_bufNakedGlobal.GetLength() ;
	const DWORD	dwSharedSize = m_pcsxiDst->m_bufNakedShared.GetLength() ;
	const DWORD	dwConstSize = m_pcsxiDst->m_bufNakedConst.GetLength() ;
	//
	EString	strMsgForErr = "テンプレートのインスタンス化 : " ;
	strMsgForErr += EString(snsSymbol.wstrFullName) ;
	//
	ImplementStatementCache( *pTemplate, strMsgForErr, dwImplement ) ;
	//
	for ( ; ; )
	{
		EControlNest *	pNest = m_nestCtrl.GetLastAt() ;
		if ( pNest == NULL )
		{
			break ;
		}
		if ( pNest->m_rwType == rwTemplate )
		{
			LeaveControlNest() ;
			break ;
		}
		else
		{
			LeaveControlNest() ;
		}
	}
	if ( err )
	{
		OutputError( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
	}
	if ( (m_pcsxiDst->m_bufImage.GetLength() != dwCodePos)
		|| (pcsxiTemp->m_bufImage.GetLength() != dwCurCodePos) )
	{
		OutputError
			( "テンプレートのインスタンス化でコードが干渉しています",
											m_strFilePath, m_nLineNum ) ;
	}
	if ( (pcsxiTemp == &m_csxiInitFunc)
		&& (m_csxiInitFunc.m_bufImage.GetLength() != dwInitCodePos) )
	{
		OutputError
			( "テンプレートのインスタンス化でコードが干渉しています",
											m_strFilePath, m_nLineNum ) ;
	}
	m_pcsxi = pcsxiTemp ;
}

// 関数プロトタイプ構文を解釈
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::ParsePrototypeDescription
		( ECSPrototypeInfo & prototype, ECSSourceStream & cssLine )
{
	EWideString	wstrName ;
	EWideString	wstrGlobalName ;
	EWideString	wstrNameSpace ;
	ESLError	err ;
	int			iFirst = cssLine.GetIndex() ;
	wstrName = cssLine.GetAToken() ;
	//
	prototype.SetAttribute( 0 ) ;
	//
	// virtual/native/static/inline/__jit_native__ 修飾判定
	//
	for ( ; ; )
	{
		if ( CompareReservedWord( L"virtual", wstrName ) == 0 )
		{
			prototype.SetAttribute
				( prototype.GetAttribute() | ECSTypeInfo::flagVirtual ) ;
		}
		else if ( CompareReservedWord( L"native", wstrName ) == 0 )
		{
			prototype.SetAttribute
				( prototype.GetAttribute() | ECSTypeInfo::flagNativeObject ) ;
		}
		else if ( CompareReservedWord( L"static", wstrName ) == 0 )
		{
			prototype.SetAttribute
				( prototype.GetAttribute() | ECSTypeInfo::flagStatic ) ;
		}
		else if ( CompareReservedWord( L"inline", wstrName ) == 0 )
		{
			prototype.SetAttribute
				( prototype.GetAttribute() | ECSTypeInfo::flagInline ) ;
		}
		else if ( CompareReservedWord( L"__jit_native__", wstrName ) == 0 )
		{
			prototype.SetAttribute
				( prototype.GetAttribute() | ECSTypeInfo::flagNakedJITNative ) ;
		}
		else
		{
			break ;
		}
		iFirst = cssLine.GetIndex() ;
		wstrName = cssLine.GetAToken() ;
	}
	//
	// 返り値省略判定
	//
	if ( wstrName == L"~" )
	{
		wstrName += cssLine.GetAToken() ;
	}
	SYMBOL_NAMESPACE	snsSymbol = wstrName ;
	err = ParseFullNameSymbol( snsSymbol, cssLine, true, false, true ) ;
	if ( err )
	{
		return	err ;
	}
	wstrName = snsSymbol.wstrName ;
	wstrGlobalName = snsSymbol.wstrFullName ;
	//
	if ( wstrName == L"operator" )
	{
		//
		// 型キャスト演算子
		//
		ECSTypeInfo	typeCast ;
		err = ParseTypeDescription( typeCast, cssLine ) ;
		if ( err )
		{
			return	err ;
		}
		EWideString	wstrTypeFormat ;
		typeCast.FormatTypeString( wstrTypeFormat ) ;
		wstrName += L" " + wstrTypeFormat ;
		wstrGlobalName += L" " + wstrTypeFormat ;
		//
		prototype.SetReturnType( typeCast ) ;
		//
		if ( cssLine.HasToComeChar( L"(" ) != L'(' )
		{
			return	ESLErrorMsg
				( "関数プロトタイプ構文に引数が見つかりません。" ) ;
		}
		if ( prototype.GetAttribute() & ECSTypeInfo::flagVirtual )
		{
			OutputWarning
				( "operator 関数に virtual 指定されています",
										m_strFilePath, m_nLineNum ) ;
			//
			prototype.SetAttribute
				( prototype.GetAttribute() & ~ECSTypeInfo::flagVirtual ) ;
		}
	}
	else if ( cssLine.HasToComeChar( L"(" ) != L'(' )
	{
		//
		// 返り値
		//
		ECSTypeInfo	typeReturn ;
		cssLine.MoveIndex( iFirst ) ;
		err = ParseTypeDescription( typeReturn, cssLine ) ;
		if ( err )
		{
			return	err ;
		}
		prototype.SetReturnType( typeReturn ) ;
		//
		// 関数名解釈
		//
		snsSymbol.wstrName = cssLine.GetAToken() ;
		snsSymbol.wstrNamespace = L"" ;
		if ( snsSymbol.wstrName == L"~" )
		{
			snsSymbol.wstrName += cssLine.GetAToken() ;
		}
		snsSymbol.wstrFullName = snsSymbol.wstrName ;
		//
		err = ParseFullNameSymbol( snsSymbol, cssLine, true, false, true ) ;
		if ( err )
		{
			return	err ;
		}
		wstrName = snsSymbol.wstrName ;
		wstrNameSpace = snsSymbol.wstrNamespace ;
		wstrGlobalName = snsSymbol.wstrFullName ;
		//
		if ( wstrName == L"operator" )
		{
			//
			// 演算子オーバーロード
			//
			OPERATOR_INFO	opinf ;
			EWideString	wstrOperator = cssLine.GetAToken() ;
			err = GetOperatorInfo( opinf, wstrOperator ) ;
			if ( err )
			{
				return	err ;
			}
			if ( opinf.opiType == optCompareSelector )
			{
				return	ESLErrorMsg
					( "operator として使用できない演算子が指定されています。" ) ;
			}
			if ( opinf.opiType == optReference )
			{
				if ( cssLine.HasToComeChar( L"]" ) != L']' )
				{
					return	ESLErrorMsg
						( "[] 演算子の閉じ括弧 ] が見つかりません。" ) ;
				}
			}
			const wchar_t *	pwszOperator = GetOperatorString( opinf ) ;
			if ( pwszOperator == NULL )
			{
				return	ESLErrorMsg( "不正なオペレーターです。" ) ;
			}
			wstrOperator = L" " ;
			wstrOperator += pwszOperator ;
			wstrName += wstrOperator ;
			wstrGlobalName += wstrOperator ;
			//
			if ( prototype.GetAttribute() & ECSTypeInfo::flagVirtual )
			{
				OutputWarning
					( "operator 関数に virtual 指定されています",
											m_strFilePath, m_nLineNum ) ;
				//
				prototype.SetAttribute
					( prototype.GetAttribute() & ~ECSTypeInfo::flagVirtual ) ;
			}
		}
		else
		{
			if ( !(m_dwModeFlags & flagNoDefaultVirtual) )
			{
				prototype.SetAttribute
					( prototype.GetAttribute() | ECSTypeInfo::flagVirtual ) ;
			}
		}
		if ( cssLine.HasToComeChar( L"(" ) != L'(' )
		{
			return	ESLErrorMsg
				( "関数プロトタイプ構文に引数が見つかりません。" ) ;
		}
	}
	else
	{
		//
		// 返り値記述省略
		//
		prototype.SetReturnType
			( ECSTypeInfo( new ECSReference, 0 ) ) ;
		//
		if ( m_dwModeFlags & flagStrictStyle )
		{
			err = OutputWarning1( "関数の返り値型が省略されています" ) ;
			if ( err )
			{
				return	err ;
			}
		}
	}
	prototype.SetName( wstrName ) ;
	prototype.SetGlobalName( wstrGlobalName ) ;
	//
	// 関数引数構文
	//
	EControlNest *	pNest = NULL ;
	if ( !wstrNameSpace.IsEmpty() )
	{
		pNest = new EControlNest( m_dwImplementFlags ) ;
		m_rwCtrlType = rwNamespace ;
		pNest->m_rwType = rwNamespace ;
		pNest->m_wstrName = wstrNameSpace ;
		pNest->m_wstrCurSpaceName = wstrNameSpace ;
		pNest->m_dwProtectedScope = ECSTypeInfo::flagPublic ;
		AddNecessaryUsingNamespace( pNest ) ;
		m_nestCtrl.Add( pNest ) ;
	}
	err = ParseArgumentDescription( prototype, cssLine ) ;
	if ( pNest != NULL )
	{
		ESLVerify( pNest == m_nestCtrl.Pop() ) ;
		LeaveControlNest( pNest ) ;
	}
	if ( err )
	{
		return	err ;
	}
	//
	// 関数修飾
	//
	int	iLastIndex = cssLine.GetIndex() ;
	EWideString	wstrToken = cssLine.GetAToken() ;
	while ( !wstrToken.IsEmpty() )
	{
		if ( CompareReservedWord( L"const", wstrToken ) == 0 )
		{
			prototype.SetAttribute
				( prototype.GetAttribute() | ECSTypeInfo::flagConstant ) ;
		}
		else if ( CompareReservedWord( L"static", wstrToken ) == 0 )
		{
			prototype.SetAttribute
				( prototype.GetAttribute() | ECSTypeInfo::flagStatic ) ;
		}
		else if ( CompareReservedWord( L"abstract", wstrToken ) == 0 )
		{
			prototype.SetAttribute
				( prototype.GetAttribute() | ECSTypeInfo::flagAbstract ) ;
		}
		else if ( CompareReservedWord( L"naked", wstrToken ) == 0 )
		{
			prototype.SetAttribute
				( prototype.GetAttribute() | ECSTypeInfo::flagNakedCall ) ;
		}
		else if ( CompareReservedWord( L"objected", wstrToken ) == 0 )
		{
			prototype.SetAttribute
				( prototype.GetAttribute() | ECSTypeInfo::flagObjectedCall ) ;
		}
		else if ( CompareReservedWord( L"native", wstrToken ) == 0 )
		{
			prototype.SetAttribute
				( prototype.GetAttribute() | ECSTypeInfo::flagNativeObject ) ;
		}
		else if ( CompareReservedWord( L"inline", wstrToken ) == 0 )
		{
			prototype.SetAttribute
				( prototype.GetAttribute() | ECSTypeInfo::flagInline ) ;
		}
		else if ( CompareReservedWord( L"throw", wstrToken ) == 0 )
		{
			if ( cssLine.HasToComeChar( L"(" ) != L'(' )
			{
				return	ESLErrorMsg( "throw 指定に \'(\' が見つかりません" ) ;
			}
			for ( ; ; )
			{
				EWideString	wstrThrowType = cssLine.GetAToken() ;
				if ( wstrThrowType == L")" )
				{
					break ;
				}
				if ( wstrThrowType.IsEmpty() )
				{
					return	ESLErrorMsg
						( "throw 指定に \'(\' に対応する \')\' が見つかりません" ) ;
				}
				if ( GetClassInfoAs( wstrThrowType ) == NULL )
				{
					m_strErrMsg = EString(wstrThrowType) + " は不正な型指定です" ;
					return	ESLErrorMsg( m_strErrMsg ) ;
				}
				prototype.AddThrowType( wstrThrowType ) ;
				//
				wchar_t	wchNext = cssLine.HasToComeChar( L",)" ) ;
				if ( wchNext == L')' )
				{
					break ;
				}
				else if ( wchNext != L',' )
				{
					return	ESLErrorMsg
						( "throw 要素が \',\' で区切られていません" ) ;
				}
			}
		}
		else if ( wstrToken == L":" )
		{
			cssLine.MoveIndex( iLastIndex ) ;
			break ;
		}
		else
		{
			return	ESLErrorMsg( "不正な関数修飾子です。" ) ;
		}
		iLastIndex = cssLine.GetIndex() ;
		wstrToken = cssLine.GetAToken() ;
	}
	const DWORD	dwConstStaticMask =
			ECSTypeInfo::flagConstant | ECSTypeInfo::flagStatic ;
	if ( (prototype.GetAttribute()
			& dwConstStaticMask) == dwConstStaticMask )
	{
		return	ESLErrorMsg
			( "関数修飾子として const と static は"
					"同時に指定することは出来ません。" ) ;
	}
	const DWORD	dwAbstractInlineMask =
			ECSTypeInfo::flagAbstract | ECSTypeInfo::flagInline ;
	if ( (prototype.GetAttribute()
			& dwAbstractInlineMask) == dwAbstractInlineMask )
	{
		return	ESLErrorMsg
			( "関数修飾子として abstract と inline は"
					"同時に指定することは出来ません。" ) ;
	}
	const DWORD	dwNakedObjectedMask =
			ECSTypeInfo::flagNakedCall | ECSTypeInfo::flagObjectedCall ;
	if ( (prototype.GetAttribute()
			& dwNakedObjectedMask) == dwNakedObjectedMask )
	{
		return	ESLErrorMsg
			( "関数修飾子として naked と objected は"
					"同時に指定することは出来ません。" ) ;
	}
	if ( (m_dwModeFlags & flagDefaultNakedAll)
		&& !(prototype.GetAttribute() & ECSTypeInfo::flagObjectedCall) )
	{
		prototype.SetAttribute
			( prototype.GetAttribute() | ECSTypeInfo::flagNakedCall ) ;
	}
	return	eslErrSuccess ;
}

// 関数引数構文を解釈
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::ParseArgumentDescription
		( ECSPrototypeInfo & prototype, ECSSourceStream & cssLine )
{
	if ( cssLine.HasToComeChar( L")" ) != L')' )
	for ( ; ; )
	{
		//
		// 可変長引数判定
		//
		if ( cssLine.HasToComeToken( L"..." ) )
		{
			prototype.SetAttribute
				( prototype.GetAttribute() | ECSTypeInfo::flagVarArgument ) ;
			if ( cssLine.HasToComeChar( L")" ) != L')' )
			{
				return	ESLErrorMsg
							( "関数引数の閉じ括弧 ) が見つかりません。" ) ;
			}
			break ;
		}
		//
		// 引数型解釈
		//
		ESLError	err ;
		ECSTypeInfo *	pArgType = new ECSTypeInfo ;
		err = ParseTypeDescription( *pArgType, cssLine ) ;
		if ( err )
		{
			delete	pArgType ;
			return	err ;
		}
		//
		// void 判定
		//
		if ( pArgType->IsVoid() )
		{
			delete	pArgType ;
			//
			if ( prototype.GetArgumentCount() == 0 )
			{
				if ( cssLine.HasToComeChar( L")" ) == L')' )
				{
					break ;
				}
				return	ESLErrorMsg
					( "関数の引数構文の閉じ括弧 ) が見つかりません。" ) ;
			}
			else
			{
				return	ESLErrorMsg
					( "関数の引数に void が指定されています。" ) ;
			}
		}
		//
		// クラスの非参照渡しチェック
		//
		if ( !pArgType->IsAbstractType() && !pArgType->IsTypeReference() )
		{
			const ECSClassInfo *
				pClassInf = GetNakedTypeClassInfo( *pArgType ) ;
			if ( pClassInf != NULL )
			{
				if ( !(pClassInf->GetAttribute()
								& ECSTypeInfo::flagNativeObject)
					&& !(pClassInf->GetAttribute()
								& ECSTypeInfo::flagStructure) )
				{
					err = OutputWarning3
						( "クラスオブジェクトは"
							"参照型で渡すことが推奨されます。",
										m_strFilePath, m_nLineNum ) ;
					if ( err )
					{
						delete	pArgType ;
						return	err ;
					}
				}
			}
		}
		//
		// 引数名取得
		//
		int			nTokenType ;
		EWideString	wstrArgName = cssLine.GetAToken( &nTokenType ) ;
		EWideString	wstrNext ;
		int			iArgIndex ;
		if ( nTokenType == 0 )
		{
			iArgIndex = prototype.AddArgument( pArgType, wstrArgName ) ;
			wstrNext = cssLine.GetAToken() ;
		}
		else
		{
			iArgIndex = prototype.AddArgument( pArgType, L"" ) ;
			wstrNext = wstrArgName ;
		}
		if ( (wstrNext == L":=") || (wstrNext == L"=") )
		{
			//
			// デフォルト値解釈
			//
			if ( (wstrNext == L"=")
				&& !(m_dwModeFlags & flagNoWarningEquMove) )
			{
				err = OutputWarning1
					( "引数のデフォルト値に = が用いられています。"
						":= を使ってください（推奨）。",
						m_strFilePath, m_nLineNum ) ;
				if ( err )
				{
					return	err ;
				}
			}
			ECSObject *	pInitObj ;
			err = CalculateExpression( pInitObj, cssLine, 0, L",)" ) ;
			if ( err )
			{
				return	err ;
			}
			ECSTypeInfo::TypeMatchResult
				typeMatch = pArgType->IsMatchType
					( ECSTypeInfo
						( ECSTypeInfo::DuplicateType( pInitObj ),
									ECSTypeInfo::flagDeterministic ) ) ;
			if ( typeMatch > ECSTypeInfo::typeNatualMatch )
			{
				delete	pInitObj ;
				return	ESLErrorMsg( "引数初期値の型が適合しません。" ) ;
			}
			prototype.SetArgumentDefaultAt( iArgIndex, pInitObj ) ;
			//
			wstrNext = cssLine.GetAToken() ;
		}
		//
		// 終了判定
		//
		if ( wstrNext == L")" )
		{
			break ;
		}
		if ( wstrNext != L"," )
		{
			return	ESLErrorMsg
						( "関数の引数が , 記号で区切られていません。" ) ;
		}
	}
	return	eslErrSuccess ;
}

// 型名か？
//////////////////////////////////////////////////////////////////////////////
int ECSCompiler::IsTypeName( const wchar_t * pwszToken ) const
{
	int	iType = m_staTypeName.FindIndex( pwszToken ) ;
	if ( iType >= 0 )
	{
		return	iType ;
	}
	return	-1 ;
}

// 記憶クラス判定
//////////////////////////////////////////////////////////////////////////////
CSObjectMode ECSCompiler::IsMemoryClass( const wchar_t * pwszToken ) const
{
	ECSWideString	wstrToken = pwszToken ;
//	if ( !(m_dwModeFlags & flagReservedWordCaseMask) )
//	{
//		wstrToken.MakeLower( ) ;
//	}
	int	index = m_staMemoryClass.FindIndex( wstrToken ) ;
	if ( index < 0 )
	{
		return	csomImmediate ;
	}
	return	(CSObjectMode) index ;
}

// 関数ローカル変数判定
//////////////////////////////////////////////////////////////////////////////
int ECSCompiler::IsLocalVariableName
		( const wchar_t * pwszToken, ECSTypeInfo * pVarType ) const
{
	int		i = 0, nVarIndex = -1 ;
	for ( i = 0; i < (int) m_nestCtrl.GetSize(); i ++ )
	{
		EControlNest *	pNest = m_nestCtrl.GetLastAt( i ) ;
		ESLAssert( pNest != NULL ) ;
		if ( pNest == NULL )
		{
			continue ;
		}
		nVarIndex = pNest->m_lstLocalName.FindIndex( pwszToken ) ;
		if ( nVarIndex >= 0 )
		{
			if ( pVarType != NULL )
			{
				ECSTypeInfo *
					pType = pNest->m_lstLocalObj.GetAt( nVarIndex ) ;
				if ( pType != NULL )
				{
					*pVarType = *pType ;
				}
			}
			break ;
		}
		if ( pNest->m_rwType == rwFunction )
		{
			break ;
		}
	}
	if ( nVarIndex >= 0 )
	{
		EControlNest *	pNest = m_nestCtrl.GetLastAt( i ++ ) ;
		if ( pNest->m_rwType != rwFunction )
		{
			while ( i < (int) m_nestCtrl.GetSize() )
			{
				EControlNest *	pNest = m_nestCtrl.GetLastAt( i ++ ) ;
				ESLAssert( pNest != NULL ) ;
				if ( pNest == NULL )
				{
					continue ;
				}
				nVarIndex += pNest->m_lstLocalObj.GetSize( ) ;
				if ( pNest->m_rwType == rwFunction )
				{
					break ;
				}
			}
		}
	}
	return	nVarIndex ;
}

// 定数テーブル名判定
//////////////////////////////////////////////////////////////////////////////
int ECSCompiler::IsDataTableName
	( const wchar_t * pwszToken,
			DWORD dwRefAddr, ECSTypeInfo * pVarType ) const
{
	pwszToken = TranslateGlobalVariableName( pwszToken ) ;
	int	nIndex = m_pcsxiDst->m_csgDataType.m_staObjName.FindIndex( pwszToken ) ;
	if ( nIndex >= 0 )
	{
		if ( pVarType != NULL )
		{
			ECSObject *	pDataType =
				m_pcsxiDst->m_csgDataType.GetVariableAt( nIndex ) ;
			if ( pDataType != NULL )
			{
				ECSGlobal *
					pgDataType = ESLTypeCast<ECSGlobal>( pDataType ) ;
				if ( pgDataType == NULL )
				{
					pVarType->SetTypeValue
						( ECSTypeInfo::DuplicateType( pDataType ), 0 ) ;
				}
				else
				{
					const ECSClassInfo *
						pGlobalType = GetClassInfoAs( L"Global" ) ;
					if ( pGlobalType != NULL )
					{
						ECSStructure *
							psDataType = new ECSStructure( pGlobalType ) ;
						if ( pgDataType->m_pDefObj != NULL )
						{
							psDataType->SetDefaultElement
								( ECSTypeInfo::DuplicateType
										( pgDataType->m_pDefObj ) ) ;
						}
						pVarType->SetTypeValue( psDataType, 0 ) ;
					}
					else
					{
						pVarType->SetTypeValue( new ECSReference, 0 ) ;
					}
				}
			}
			else
			{
				pVarType->SetTypeValue( new ECSReference, 0 ) ;
			}
		}
		if ( m_pcsxiDst == m_pcsxi )
		{
			m_pcsxi->m_extDataRef.Add( dwRefAddr ) ;
		}
		else
		{
			ENumArray<DWORD> *	pList =
				m_pcsxi->m_impDataRef.GetAs( pwszToken ) ;
			if ( pList == NULL )
			{
				pList = new ENumArray<DWORD> ;
				m_pcsxi->m_impDataRef.Add( pwszToken, pList ) ;
			}
			pList->Add( dwRefAddr ) ;
		}
		return	nIndex ;
	}
	ENumArray<DWORD> *	pList =
		m_pcsxiDst->m_impDataRef.GetAs( pwszToken ) ;
	if ( pList != NULL )
	{
		pList = m_pcsxi->m_impDataRef.GetAs( pwszToken ) ;
		if ( pList == NULL )
		{
			pList = new ENumArray<DWORD> ;
			m_pcsxi->m_impDataRef.Add( pwszToken, pList ) ;
		}
		pList->Add( dwRefAddr ) ;
		//
		if ( pVarType != NULL )
		{
			ECSTypeInfo *
				pDefType = m_wstaExternVarType.GetAs( pwszToken ) ;
			if ( pDefType != NULL )
			{
				*pVarType = *pDefType ;
			}
			else
			{
				const ECSClassInfo *
					pGlobalType = GetClassInfoAs( L"Global" ) ;
				if ( pGlobalType != NULL )
				{
					pVarType->SetTypeValue( new ECSStructure( pGlobalType ), 0 ) ;
				}
				else
				{
					pVarType->SetTypeValue( new ECSReference, 0 ) ;
				}
			}
		}
		return	0 ;
	}
	return	-1 ;
}

// 関数ポインタ判定
//////////////////////////////////////////////////////////////////////////////
bool ECSCompiler::CompileCodeAutoFunctionPointer
	( ECSTypeInfo & typeVar, const SYMBOL_NAMESPACE& snsSymbol )
{
	EPtrObjArray<const wchar_t>	lstNamespace ;
	GetUsingNamespaceList( lstNamespace ) ;
	//
	if ( !snsSymbol.wstrNamespace.IsEmpty() )
	{
		lstNamespace.Add( NULL ) ;
	}
	EWideString	wstrThisSpace ;
	for ( int i = 0; i < (int) lstNamespace.GetSize(); i ++ )
	{
		const wchar_t *	pwszNamespace = lstNamespace.GetAt( i ) ;
		wstrThisSpace = pwszNamespace ;
		ECSTypeInfo::Flags	flagScope = ECSTypeInfo::flagProtected ;
		if ( wstrThisSpace != pwszNamespace )
		{
			flagScope = ECSTypeInfo::flagPublic ;
		}
		if ( !snsSymbol.wstrNamespace.IsEmpty() )
		{
			if ( !wstrThisSpace.IsEmpty() )
			{
				wstrThisSpace += L"::" ;
			}
			wstrThisSpace += snsSymbol.wstrNamespace ;
			flagScope = ECSTypeInfo::flagPublic ;
		}
		if ( wstrThisSpace.IsEmpty() )
		{
			continue ;
		}
		ECSClassInfo *	pClassInf = GetClassInfoAs( wstrThisSpace ) ;
		if ( pClassInf == NULL )
		{
			continue ;
		}
		int	iFunc = pClassInf->FindFunctionAs( snsSymbol.wstrName ) ;
		if ( iFunc < 0 )
		{
			continue ;
		}
		ECSClassInfo::MemberFunction *
				pFunc = pClassInf->GetFunctionAt( iFunc ) ;
		if ( pFunc == NULL )
		{
			continue ;
		}
		if ( pFunc->GetProtectedAttribute() > (DWORD) flagScope )
		{
			OutputWarning
				( "プライベートなメンバ関数を参照しています",
									m_strFilePath, m_nLineNum ) ;
			return	false ;
		}
		return	CompileCodeLoadFunctionPointer( typeVar, pFunc ) ;
	}
	return	CompileCodeGlobalFunctionPointer
						( typeVar, snsSymbol.wstrFullName ) ;
}

// 大域関数判定
//////////////////////////////////////////////////////////////////////////////
ECSPrototypeInfo *
	ECSCompiler::SearchGlobalFunctionAs
				( const wchar_t * pwszToken, bool fGlobalScope )
{
	ECSPrototypeInfo *	pFunc = NULL ;
	if ( !fGlobalScope )
	{
		SYMBOL_NAMESPACE	snsSymbol ;
		snsSymbol.ParseSymbol( pwszToken ) ;
		//
		EPtrObjArray<const wchar_t>	lstNamespace ;
		EWideString	wstrNamespace ;
		GetUsingNamespaceList( lstNamespace ) ;
		for ( unsigned int i = 0; i < lstNamespace.GetSize(); i ++ )
		{
			wstrNamespace = lstNamespace.GetAt( i ) ;
			if ( !wstrNamespace.IsEmpty() && !snsSymbol.wstrNamespace.IsEmpty() )
			{
				wstrNamespace += L"::" ;
				wstrNamespace += snsSymbol.wstrNamespace ;
			}
			if ( wstrNamespace.IsEmpty() )
			{
				continue ;
			}
			ECSClassInfo *	pClassInf = GetClassInfoAs( wstrNamespace ) ;
			if ( pClassInf == NULL )
			{
				continue ;
			}
			int	iFunc = pClassInf->FindFunctionAs( snsSymbol.wstrName ) ;
			if ( iFunc >= 0 )
			{
				pFunc = pClassInf->GetFunctionAt( iFunc ) ;
				if ( pFunc != NULL )
				{
					return	pFunc ;
				}
			}
		}
		if ( !snsSymbol.wstrNamespace.IsEmpty() )
		{
			ECSClassInfo *	pClassInf = GetClassInfoAs( snsSymbol.wstrNamespace ) ;
			if ( pClassInf != NULL )
			{
				int	iFunc = pClassInf->FindFunctionAs( snsSymbol.wstrName ) ;
				if ( iFunc >= 0 )
				{
					pFunc = pClassInf->GetFunctionAt( iFunc ) ;
					if ( pFunc != NULL )
					{
						return	pFunc ;
					}
				}
			}
		}
	}
	if ( m_modeNakedCode )
	{
		pFunc = m_wstaNakedPrototype.GetAs( pwszToken ) ;
		if ( pFunc == NULL )
		{
			pFunc = m_wstaPrototype.GetAs( pwszToken ) ;
		}
	}
	else
	{
		pFunc = m_wstaPrototype.GetAs( pwszToken ) ;
		if ( pFunc == NULL )
		{
			pFunc = m_wstaNakedPrototype.GetAs( pwszToken ) ;
		}
	}
	return	pFunc ;
}

// 大域関数ポインタ判定
//////////////////////////////////////////////////////////////////////////////
bool ECSCompiler::CompileCodeGlobalFunctionPointer
	( ECSTypeInfo & typeVar, const wchar_t * pwszToken, bool fGlobalScope )
{
	ECSPrototypeInfo *	pFunc =
			SearchGlobalFunctionAs( pwszToken, fGlobalScope ) ;
	if ( pFunc == NULL )
	{
		return	false ;
	}
	return	CompileCodeLoadFunctionPointer( typeVar, pFunc ) ;
}

// 関数ポインタロードコード出力
//////////////////////////////////////////////////////////////////////////////
bool ECSCompiler::CompileCodeLoadFunctionPointer
	( ECSTypeInfo & typeVar, const ECSPrototypeInfo * pFunc )
{
	ECSFunction *	pFuncType = new ECSFunction ;
	pFuncType->m_prototype = *pFunc ;
	//
	if ( pFunc->GetAttribute() & ECSTypeInfo::flagThisCall )
	{
		EWideString	wstrClass = pFunc->GetNameSpace() ;
		if ( !wstrClass.IsEmpty() )
		{
			pFuncType->m_pThisCall = GetClassInfoAs( wstrClass ) ;
		}
	}
	ECSPointer *	pPtr = new ECSPointer ;
	pPtr->SetOwnObject( pFuncType ) ;
	typeVar.SetTypeValue( pPtr, 0 ) ;
	//
	if ( pFunc->GetAttribute() & ECSTypeInfo::flagNativeObject )
	{
		CompileImmediateNakedNativeFunctionID( pFunc->GetGlobalName() ) ;
	}
	else
	{
		CompileImmediateFunctionPointer( pFunc->GetGlobalName() ) ;
	}
	if ( m_modeNakedCode )
	{
		typeVar.SetLoadedRegister( GetExpressionRegister() ) ;
	}
	if ( pFunc->GetAttribute() & ECSTypeInfo::flagNativeObject )
	{
		OutputError
			( "native 関数のポインタは取得出来ません",
							m_strFilePath, m_nLineNum ) ;
	}
	return	true ;
}

// 大域変数名判定
//////////////////////////////////////////////////////////////////////////////
int ECSCompiler::IsGlobalVariableName
	( const wchar_t * pwszToken, DWORD dwRefAddr, ECSTypeInfo * pVarType ) const
{
	pwszToken = TranslateGlobalVariableName( pwszToken ) ;
	int	nIndex =
		m_pcsxiDst->m_csgGlobalType.m_staObjName.FindIndex( pwszToken ) ;
	if ( nIndex >= 0 )
	{
		if ( pVarType != NULL )
		{
			ECSObject *	pDataType =
				m_pcsxiDst->m_csgGlobalType.GetVariableAt( nIndex ) ;
			if ( pDataType != NULL )
			{
				pVarType->SetTypeValue
					( ECSTypeInfo::DuplicateType( pDataType ), 0 ) ;
			}
			else
			{
				pVarType->SetTypeValue( new ECSReference, 0 ) ;
			}
		}
		if ( m_pcsxiDst == m_pcsxi )
		{
			m_pcsxi->m_extGlobalRef.Add( dwRefAddr ) ;
		}
		else
		{
			ENumArray<DWORD> *	pList =
				m_pcsxi->m_impGlobalRef.GetAs( pwszToken ) ;
			if ( pList == NULL )
			{
				pList = new ENumArray<DWORD> ;
				m_pcsxi->m_impGlobalRef.Add( pwszToken, pList ) ;
			}
			pList->Add( dwRefAddr ) ;
		}
		return	nIndex ;
	}
	ENumArray<DWORD> *	pList =
		m_pcsxiDst->m_impGlobalRef.GetAs( pwszToken ) ;
	if ( pList != NULL )
	{
		pList = m_pcsxi->m_impGlobalRef.GetAs( pwszToken ) ;
		if ( pList == NULL )
		{
			pList = new ENumArray<DWORD> ;
			m_pcsxi->m_impGlobalRef.Add( pwszToken, pList ) ;
		}
		pList->Add( dwRefAddr ) ;
		//
		if ( pVarType != NULL )
		{
			ECSTypeInfo *
				pDefType = m_wstaExternVarType.GetAs( pwszToken ) ;
			if ( pDefType != NULL )
			{
				*pVarType = *pDefType ;
			}
			else
			{
				pVarType->SetTypeValue( new ECSReference, 0 ) ;
			}
		}
		return	0 ;
	}
	EWideString	wstrNameSpace = GetCurrentSpaceName() ;
	EWideString	wstrVarName = pwszToken ;
	if ( !wstrNameSpace.IsEmpty()
		&& (wstrVarName.Find( L"::" ) < 0) )
	{
		wstrVarName = wstrNameSpace + L"::" + pwszToken ;
		int	iVarIndex =
			IsGlobalVariableName( wstrVarName, dwRefAddr, pVarType ) ;
		if ( iVarIndex >= 0 )
		{
			return	iVarIndex ;
		}
		wstrNameSpace = GetCurrentThisClassName() ;
		if ( !wstrNameSpace.IsEmpty() )
		{
			wstrVarName = wstrNameSpace + L"::" + pwszToken ;
			int	iVarIndex =
				IsGlobalVariableName( wstrVarName, dwRefAddr, pVarType ) ;
			if ( iVarIndex >= 0 )
			{
				return	iVarIndex ;
			}
		}
	}
	return	-1 ;
}

// naked 大域変数参照
//////////////////////////////////////////////////////////////////////////////
bool ECSCompiler::CompileCodeSearchNakedGlobalVariable
		( ECSTypeInfo & typeVar, const wchar_t * pwszToken )
{
	EPtrObjArray<const wchar_t>	lstNamespace ;
	GetUsingNamespaceList( lstNamespace ) ;
	//
	EWideString	wstrNamespace ;
	int	i, nCount = lstNamespace.GetSize() ;
	for ( i = 0; i < nCount; i ++ )
	{
		const wchar_t * pwszNamespace = lstNamespace.GetAt( i ) ;
		if ( pwszNamespace != NULL )
		{
			wstrNamespace = pwszNamespace ;
			wstrNamespace += L"::" ;
			wstrNamespace += pwszToken ;
			if ( CompileCodeFindNakedGlobalVariable( typeVar, wstrNamespace ) )
			{
				return	true ;
			}
		}
	}
	return	CompileCodeFindNakedGlobalVariable( typeVar, pwszToken ) ;
}

bool ECSCompiler::CompileCodeFindNakedGlobalVariable
		( ECSTypeInfo & typeVar, const wchar_t * pwszToken )
{
	//
	// 変数宣言確認
	//
	pwszToken = TranslateGlobalVariableName( pwszToken ) ;
	ECSTypeInfo *	pVarType = m_wstaExternNakedVarType.GetAs( pwszToken ) ;
	if ( pVarType == NULL )
	{
		return	false ;
	}
	//
	// 記憶クラス判定
	//
	if ( m_pcsxiDst->m_impNakedGlobalRef.GetAs( pwszToken ) != NULL )
	{
		if ( m_modeNakedCode )
		{
			int	regAddr = AllocateExpressionRegister() ;
			m_pcsxi->WriteSakuraLoadInt64_GlobalVarAddr( regAddr, pwszToken ) ;
			typeVar = *pVarType ;
			typeVar.SetLoadedRegister( regAddr ) ;
			typeVar.SetAddressingInfo( regAddr, 0 ) ;
		}
		else
		{
			DWORD	dwRefAddr ;
			INT64	nValue = 0 ;
			m_pcsxi->WriteInstructionCode( csicLoad ) ;
			m_pcsxi->WriteObjectModeCode( csomImmediate ) ;
			m_pcsxi->WriteVariableTypeCode( csvtInteger64 ) ;
			dwRefAddr = m_pcsxi->m_bufImage.GetLength() ;
			m_pcsxi->WriteCodeData( &nValue, sizeof(INT64) ) ;
			m_pcsxi->AddCodeRefNakedGlobalAddress( pwszToken, dwRefAddr ) ;
			//
			CompileCodePointerToAddress() ;
			CompileCodeReferenceForPointer( csvtObject ) ;
			//
			typeVar = *pVarType ;
		}
		return	true ;
	}
	if ( m_pcsxiDst->m_impNakedSharedRef.GetAs( pwszToken ) != NULL )
	{
		if ( m_modeNakedCode )
		{
			int	regAddr = AllocateExpressionRegister() ;
			m_pcsxi->WriteSakuraLoadInt64_SharedVarAddr( regAddr, pwszToken ) ;
			typeVar = *pVarType ;
			typeVar.SetLoadedRegister( regAddr ) ;
			typeVar.SetAddressingInfo( regAddr, 0 ) ;
		}
		else
		{
			DWORD	dwRefAddr ;
			INT64	nValue = 0 ;
			m_pcsxi->WriteInstructionCode( csicLoad ) ;
			m_pcsxi->WriteObjectModeCode( csomImmediate ) ;
			m_pcsxi->WriteVariableTypeCode( csvtInteger64 ) ;
			dwRefAddr = m_pcsxi->m_bufImage.GetLength() ;
			m_pcsxi->WriteCodeData( &nValue, sizeof(INT64) ) ;
			m_pcsxi->AddCodeRefNakedSharedAddress( pwszToken, dwRefAddr ) ;
			//
			CompileCodePointerToAddress() ;
			CompileCodeReferenceForPointer( csvtObject ) ;
			//
			typeVar = *pVarType ;
		}
		return	true ;
	}
	if ( m_pcsxiDst->m_impNakedConstRef.GetAs( pwszToken ) != NULL )
	{
		if ( m_modeNakedCode )
		{
			int	regAddr = AllocateExpressionRegister() ;
			m_pcsxi->WriteSakuraLoadInt64_ConstVarAddr( regAddr, pwszToken ) ;
			typeVar = *pVarType ;
			typeVar.SetLoadedRegister( regAddr ) ;
			typeVar.SetAddressingInfo( regAddr, 0 ) ;
		}
		else
		{
			DWORD	dwRefAddr ;
			INT64	nValue = 0 ;
			m_pcsxi->WriteInstructionCode( csicLoad ) ;
			m_pcsxi->WriteObjectModeCode( csomImmediate ) ;
			m_pcsxi->WriteVariableTypeCode( csvtInteger64 ) ;
			dwRefAddr = m_pcsxi->m_bufImage.GetLength() ;
			m_pcsxi->WriteCodeData( &nValue, sizeof(INT64) ) ;
			m_pcsxi->AddCodeRefNakedConstAddress( pwszToken, dwRefAddr ) ;
			//
			CompileCodePointerToAddress() ;
			CompileCodeReferenceForPointer( csvtObject ) ;
			//
			typeVar = *pVarType ;
		}
		return	true ;
	}
	return	false ;
}

// 大域変数リンケージ名変換
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSCompiler::TranslateGlobalVariableName
							( const wchar_t * pwszVarName ) const
{
	EWideString *	pwstrVarName = m_wstaVarGlobalName.GetAs( pwszVarName ) ;
	if ( pwstrVarName == NULL )
	{
		return	pwszVarName ;
	}
	return	*pwstrVarName ;
}

// 定数オブジェクトを即値データ命令として出力
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileImmediateObject
				( ECSTypeInfo & typeinf, ECSObject * pObj )
{
	if ( m_modeNakedCode )
	{
		//
		// naked モード
		//
		switch ( pObj->m_vtType )
		{
		case	csvtInteger:
//			m_pcsxi->WriteSakuraLoadInt64
//					( AllocateExpressionRegister(),
//							((ECSInteger*)pObj)->GetValue() ) ;
			typeinf.SetTypeValue
				( new ECSInteger( ((ECSInteger*)pObj)->GetValue() ),
					ECSTypeInfo::flagDeterministic ) ;
			typeinf.NormalzieImmediateIntegerType() ;
			return	eslErrSuccess ;
		case	csvtReal:
//			m_pcsxi->WriteSakuraLoadReal64
//					( AllocateExpressionRegister(),
//							((ECSReal*)pObj)->m_varReal ) ;
			typeinf.SetTypeValue
				( new ECSReal( ((ECSReal*)pObj)->m_varReal ),
					ECSTypeInfo::flagDeterministic ) ;
			return	eslErrSuccess ;
		case	csvtString:
			m_pcsxi->WriteSakuraLoadInt64_CStrPtr
					( AllocateExpressionRegister(),
							((ECSString*)pObj)->m_varStr ) ;
			typeinf.MakePointerOf
				( ECSTypeInfo( new ECSInteger
						( 0, ECSInteger::m_maskUint16 ) ) ) ;
			typeinf.SetLoadedRegister( GetExpressionRegister() ) ;
			return	eslErrSuccess ;
		case	csvtReference:
			if ( ((ECSReference*)pObj)->m_pRef == NULL )
			{
				m_pcsxi->WriteSakuraLoadInt64
						( AllocateExpressionRegister(), 0 ) ;
				typeinf.SetTypeValue
					( new ECSReference, ECSTypeInfo::flagDeterministic ) ;
				typeinf.SetLoadedRegister( GetExpressionRegister() ) ;
				return	eslErrSuccess ;
			}
			break ;
		case	csvtPointer:
			if ( ((ECSPointer*)pObj)->m_pRef == NULL )
			{
				m_pcsxi->WriteSakuraLoadInt64
						( AllocateExpressionRegister(), 0 ) ;
				typeinf.SetTypeValue
					( new ECSPointer, ECSTypeInfo::flagDeterministic ) ;
				typeinf.SetLoadedRegister( GetExpressionRegister() ) ;
				return	eslErrSuccess ;
			}
			break ;
		}
		return	ESLErrorMsg( "naked モードで不正な定数型です。" ) ;
	}
	//
	// object モード
	//
	ESLError	err ;
	m_pcsxi->WriteInstructionCode( csicLoad ) ;
	m_pcsxi->WriteObjectModeCode( csomImmediate ) ;
	switch ( pObj->m_vtType )
	{
	case	csvtInteger:
		if ( ( ((ECSInteger*)pObj)->SizeOf() == 64)
			&& ((((ECSInteger*)pObj)->GetValue() < -0x7FFFFFFF)
				|| (((ECSInteger*)pObj)->GetValue() > 0x7FFFFFFF)) )
		{
			INT64	nValue = ((ECSInteger*)pObj)->GetValue( ) ;
			m_pcsxi->WriteVariableTypeCode( csvtInteger64 ) ;
			m_pcsxi->WriteCodeData( &nValue, sizeof(INT64) ) ;
			//
			typeinf.SetTypeValue
				( new ECSInteger( nValue ),
					ECSTypeInfo::flagDeterministic ) ;
		}
		else
		{
			long int	nValue = ((ECSInteger*)pObj)->GetInt( ) ;
			INT64		nMask = ((ECSInteger*)pObj)->GetValueMask( ) ;
			if ( nMask == ECSInteger::m_maskBoolean )
			{
				m_pcsxi->WriteVariableTypeCode( csvtBoolean ) ;
				m_pcsxi->WriteCodeData( &nValue, sizeof(SBYTE) ) ;
			}
			else if ( nMask == ECSInteger::m_maskInt8 )
			{
				m_pcsxi->WriteVariableTypeCode( csvtInt8 ) ;
				m_pcsxi->WriteCodeData( &nValue, sizeof(SBYTE) ) ;
			}
			else if ( nMask == ECSInteger::m_maskUint8 )
			{
				m_pcsxi->WriteVariableTypeCode( csvtUint8 ) ;
				m_pcsxi->WriteCodeData( &nValue, sizeof(BYTE) ) ;
			}
			else if ( nMask == ECSInteger::m_maskInt16 )
			{
				m_pcsxi->WriteVariableTypeCode( csvtInt16 ) ;
				m_pcsxi->WriteCodeData( &nValue, sizeof(SWORD) ) ;
			}
			else if ( nMask == ECSInteger::m_maskUint16 )
			{
				m_pcsxi->WriteVariableTypeCode( csvtUint16 ) ;
				m_pcsxi->WriteCodeData( &nValue, sizeof(WORD) ) ;
			}
			else if ( nMask == ECSInteger::m_maskInt32 )
			{
				m_pcsxi->WriteVariableTypeCode( csvtInt32 ) ;
				m_pcsxi->WriteCodeData( &nValue, sizeof(long int) ) ;
			}
			else if ( nMask == ECSInteger::m_maskUint32 )
			{
				m_pcsxi->WriteVariableTypeCode( csvtUint32 ) ;
				m_pcsxi->WriteCodeData( &nValue, sizeof(long int) ) ;
			}
			else
			{
				m_pcsxi->WriteVariableTypeCode( csvtInteger ) ;
				m_pcsxi->WriteCodeData( &nValue, sizeof(long int) ) ;
			}
			typeinf.SetTypeValue
				( new ECSInteger( nValue, nMask ),
					ECSTypeInfo::flagDeterministic ) ;
		}
		break ;

	case	csvtReal:
		m_pcsxi->WriteVariableTypeCode( csvtReal ) ;
		m_pcsxi->WriteCodeData
			( &(((ECSReal*)pObj)->m_varReal), sizeof(double) ) ;
		//
		typeinf.SetTypeValue
			( new ECSReal( ((ECSReal*)pObj)->m_varReal ),
							ECSTypeInfo::flagDeterministic ) ;
		break ;

	case	csvtString:
		m_pcsxi->WriteVariableTypeCode( csvtString ) ;
		m_pcsxi->WriteConstantString( ((ECSString*)pObj)->m_varStr ) ;
		//
		typeinf.SetTypeValue
			( new ECSString( ((ECSString*)pObj)->m_varStr ),
								ECSTypeInfo::flagDeterministic ) ;
		break ;

	case	csvtArray:
		m_pcsxi->WriteVariableTypeCode( csvtArray ) ;
		//
		err = CompileArrayDimension( (ECSArray*) pObj ) ;
		if ( err )
		{
			return	err ;
		}
		//
		typeinf.SetTypeValue( pObj->Duplicate(), 0 ) ;
		break ;

	case	csvtHash:
		m_pcsxi->WriteVariableTypeCode( csvtHash ) ;
		//
		err = CompileHashContainer( (ECSHash*) pObj ) ;
		if ( err )
		{
			return	err ;
		}
		//
		typeinf.SetTypeValue( pObj->Duplicate(), 0 ) ;
		break ;

	case	csvtReference:
		m_pcsxi->WriteVariableTypeCode( csvtReference ) ;
		//
		typeinf.SetTypeValue
			( new ECSReference, ECSTypeInfo::flagDeterministic ) ;
		break ;

	case	csvtObject:
		{
			int	nClassIndex =
				m_pcsxiDst->GetClassInfoIndex( pObj->GetTypeName() ) ;
			if ( nClassIndex >= 0 )
			{
				m_pcsxi->WriteVariableTypeCode( csvtClassObject ) ;
				m_pcsxi->WriteClassIndex( nClassIndex ) ;
			}
			else
			{
				m_pcsxi->WriteVariableTypeCode( csvtObject ) ;
				m_pcsxi->WriteConstantString
							( ECSWideString(pObj->GetTypeName()) ) ;
			}
		}
		typeinf.SetTypeValue( pObj->Duplicate(), 0 ) ;
		break ;

	default:
		return	ESLErrorMsg( "不正な定数型です。" ) ;
	}
	return	eslErrSuccess ;
}

void ECSCompiler::CompileImmediateInteger( INT64 nValue )
{
	if ( m_modeNakedCode )
	{
		m_pcsxi->WriteSakuraLoadInt64
			( AllocateExpressionRegister(), nValue ) ;
	}
	else
	{
		m_pcsxi->WriteInstructionCode( csicLoad ) ;
		m_pcsxi->WriteObjectModeCode( csomImmediate ) ;
		//
		if ( (nValue < -0x7FFFFFFF) || (nValue > 0x7FFFFFFF) )
		{
			m_pcsxi->WriteVariableTypeCode( csvtInteger64 ) ;
			m_pcsxi->WriteCodeData( &nValue, sizeof(INT64) ) ;
		}
		else
		{
			m_pcsxi->WriteVariableTypeCode( csvtInteger ) ;
			m_pcsxi->WriteCodeData( &nValue, sizeof(int) ) ;
		}
	}
}

void ECSCompiler::CompileImmediateReal( REAL64 nValue )
{
	if ( m_modeNakedCode )
	{
		m_pcsxi->WriteSakuraLoadReal64
			( AllocateExpressionRegister(), nValue ) ;
	}
	else
	{
		m_pcsxi->WriteInstructionCode( csicLoad ) ;
		m_pcsxi->WriteObjectModeCode( csomImmediate ) ;
		m_pcsxi->WriteVariableTypeCode( csvtReal ) ;
		m_pcsxi->WriteCodeData( &nValue, sizeof(REAL64) ) ;
	}
}

void ECSCompiler::CompileImmediateString( const wchar_t * pwszValue )
{
	if ( m_modeNakedCode )
	{
		m_pcsxi->WriteSakuraLoadInt64_CStrPtr
				( AllocateExpressionRegister(), pwszValue ) ;
	}
	else
	{
		ECSWideString	wstrValue ;
		wstrValue.AttachString( pwszValue ) ;
		//
		m_pcsxi->WriteInstructionCode( csicLoad ) ;
		m_pcsxi->WriteObjectModeCode( csomImmediate ) ;
		m_pcsxi->WriteVariableTypeCode( csvtString ) ;
		m_pcsxi->WriteConstantString( wstrValue ) ;
	}
}

// 基本型オブジェクト生成命令出力
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::CompileImmediateBasicVariable( CSVariableType csvtType )
{
	ESLAssert( !m_modeNakedCode ) ;
	m_pcsxi->WriteInstructionCode( csicLoad ) ;
	m_pcsxi->WriteObjectModeCode( csomImmediate ) ;
	m_pcsxi->WriteVariableTypeCode( csvtType ) ;
}

// クラスオブジェクト生成命令出力
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompilerCodeCreateClassObject
							( const wchar_t * pwszGlobalName )
{
	ESLAssert( !m_modeNakedCode ) ;
	m_pcsxi->WriteInstructionCode( csicLoad ) ;
	m_pcsxi->WriteObjectModeCode( csomImmediate ) ;
	//
	int	nClassIndex = m_pcsxiDst->GetClassInfoIndex( pwszGlobalName ) ;
	if ( nClassIndex >= 0 )
	{
		m_pcsxi->WriteVariableTypeCode( csvtClassObject ) ;
		m_pcsxi->WriteClassIndex( nClassIndex ) ;
		return	eslErrSuccess ;
	}
	else
	{
		return	ESLErrorMsg
			( "内部エラー：構築するオブジェクトの"
						"クラス情報が見つかりません。" ) ;
	}
}

// 関数ポインタをスタック上にロードする命令を生成
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileImmediateFunctionPointer
							( const wchar_t * pwszFuncName )
{
	if ( m_modeNakedCode )
	{
		int	regExpr = AllocateExpressionRegister() ;
		m_pcsxi->WriteSakuraLoadInt64_FuncPtr( regExpr, pwszFuncName ) ;
		m_pcsxi->FlushLastWrittenInstruction() ;
	}
	else
	{
		m_pcsxi->WriteInstructionCode( csicLoad ) ;
		m_pcsxi->WriteObjectModeCode( csomImmediate ) ;
		m_pcsxi->WriteVariableTypeCode( csvtInteger64 ) ;
		//
		m_pcsxi->AddCodeRefFunctionAddress64
				( pwszFuncName, m_pcsxi->m_bufImage.GetLength() ) ;
		DWORD	dwCodeLowAddr = 0 ;
		DWORD	dwCodeHighAddr = (ECSExecutionImage::roasCode << 24) ;
		m_pcsxi->WriteCodeData( &dwCodeLowAddr, sizeof(DWORD) ) ;
		m_pcsxi->WriteCodeData( &dwCodeHighAddr, sizeof(DWORD) ) ;
		m_pcsxi->FlushLastWrittenInstruction() ;
	}
	return	eslErrSuccess ;
}

// naked ネイティブ関数 ID をスタック上にロードする命令を生成
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileImmediateNakedNativeFunctionID( const wchar_t * pwszFuncName )
{
	if ( m_modeNakedCode )
	{
		int	regExpr = AllocateExpressionRegister() ;
		m_pcsxi->WriteSakuraInstructionCode( ECSSakura2Processor::codeLoadImm64 ) ;
		m_pcsxi->WriteByteCode( (BYTE) regExpr ) ;
		//
		m_pcsxi->WriteNakedNativeFunctionIndex( pwszFuncName ) ;
		//
		DWORD	dwCodeHighAddr = 0 ;
		m_pcsxi->WriteCodeData( &dwCodeHighAddr, sizeof(DWORD) ) ;
		//
		m_pcsxi->FlushLastWrittenInstruction() ;
	}
	else
	{
		m_pcsxi->WriteInstructionCode( csicLoad ) ;
		m_pcsxi->WriteObjectModeCode( csomImmediate ) ;
		m_pcsxi->WriteVariableTypeCode( csvtInteger64 ) ;
		//
		m_pcsxi->WriteNakedNativeFunctionIndex( pwszFuncName ) ;
		//
		DWORD	dwCodeHighAddr = 0 ;
		m_pcsxi->WriteCodeData( &dwCodeHighAddr, sizeof(DWORD) ) ;
		//
		m_pcsxi->FlushLastWrittenInstruction() ;
	}
	return	eslErrSuccess ;
}

// スタック上のオブジェクトへの一時参照ロード命令生成
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileLoadStackObject( long int nStackTop )
{
	if ( m_modeNakedCode )
	{
		int	regSrc = GetExpressionRegister( nStackTop ) ;
		m_pcsxi->WriteSakuraMoveRegReg
			( AllocateExpressionRegister(), regSrc ) ;
	}
	else
	{
		CompileCodeLoadRefVariable( csomStack, -1 - nStackTop ) ;
	}
	return	eslErrSuccess ;
}

// 変数参照命令出力
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::CompileCodeLoadRefVariable
		( CSObjectMode objmode, int nVarIndex )
{
	ESLAssert( !m_modeNakedCode ) ;
	DWORD	dwIndex = nVarIndex ;
	m_pcsxi->WriteInstructionCode( csicLoad ) ;
	m_pcsxi->WriteObjectModeCode( objmode ) ;
	m_pcsxi->WriteVariableTypeCode( csvtInteger ) ;
	m_pcsxi->WriteCodeData( &dwIndex, sizeof(DWORD) ) ;
}

void ECSCompiler::CompileCodeLoadRefVariable
		( CSObjectMode objmode, const wchar_t * pwszVarName )
{
	ESLAssert( !m_modeNakedCode ) ;
	m_pcsxi->WriteInstructionCode( csicLoad ) ;
	m_pcsxi->WriteObjectModeCode( objmode ) ;
	m_pcsxi->WriteVariableTypeCode( csvtString ) ;
	m_pcsxi->WriteConstantString( ECSWideString( pwszVarName ) ) ;
}

// スタック解放命令出力
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::CompileCodeFreeStack( void )
{
	if ( m_modeNakedCode )
	{
		FreeExpressionRegister() ;
	}
	else
	{
		m_pcsxi->WriteInstructionCode( csicFree ) ;
	}
}

void ECSCompiler::CompileCodeFreeStack( const ECSTypeInfo & typeinf )
{
	if ( m_modeNakedCode )
	{
		if ( typeinf.IsLoadedRegister() )
		{
			if ( typeinf.IsLoadedThisCallRegister() )
			{
				FreeExpressionRegister() ;
				FreeExpressionRegister() ;
			}
			else
			{
				FreeExpressionRegister() ;
			}
		}
	}
	else
	{
		if ( !typeinf.IsVoid() )
		{
			m_pcsxi->WriteInstructionCode( csicFree ) ;
		}
	}
}

// naked メモリポインタ変換命令出力
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::CompileCodePointerToObject( int iOffset )
{
	if ( m_modeNakedCode )
	{
		int	regExpr = GetExpressionRegister() ;
		if ( iOffset != 0 )
		{
			m_pcsxi->WriteSakuraAddRegRegImm32( regExpr, regExpr, iOffset ) ;
		}
		else
		{
			m_pcsxi->WriteSakuraMoveRegReg( regExpr, regExpr ) ;
		}
	}
	else
	{
		DWORD	dwOffset = iOffset ;
		m_pcsxi->WriteInstructionCode( csicPointerToObject ) ;
		m_pcsxi->WriteCodeData( &dwOffset, sizeof(DWORD) ) ;
	}
}

void ECSCompiler::CompileCodePointerToAddress( void )
{
	if ( m_modeNakedCode )
	{
	}
	else
	{
		m_pcsxi->WriteInstructionCode( csicPointerToAddress ) ;
	}
}

// ポインタ参照命令出力
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::CompileCodeReferenceForPointer( CSVariableType csvtRefType )
{
	if ( m_modeNakedCode )
	{
	}
	else
	{
		m_pcsxi->WriteInstructionCode( csicReferenceForPointer ) ;
		m_pcsxi->WriteVariableTypeCode( csvtRefType ) ;
	}
}

// ストア命令出力
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::CompileCodeStore( CSOperatorType optype )
{
	ESLAssert( !m_modeNakedCode ) ;
	m_pcsxi->WriteInstructionCode( csicStore ) ;
	m_pcsxi->WriteOperatorTypeCode( optype ) ;
}

// 演算命令出力
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::CompileCodeOperate( CSOperatorType optype )
{
	ESLAssert( !m_modeNakedCode ) ;
	m_pcsxi->WriteInstructionCode( csicOperate ) ;
	m_pcsxi->WriteOperatorTypeCode( optype ) ;
}

// 比較命令出力
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::CompileCodeCompare( CSCompareType cmptype )
{
	ESLAssert( !m_modeNakedCode ) ;
	m_pcsxi->WriteInstructionCode( csicCompare ) ;
	m_pcsxi->WriteCompareTypeCode( cmptype ) ;
}

// ジャンプ命令出力
//////////////////////////////////////////////////////////////////////////////
DWORD ECSCompiler::CompileCodeJump( void )
{
	if ( m_modeNakedCode )
	{
		return	m_pcsxi->WriteSakuraJumpOffset32( 0 ) ;
	}
	else
	{
		m_pcsxi->FenceInstruction() ;
		//
		DWORD	dwJumpRefAddr ;
		DWORD	dwDummy = 0 ;
		//
		m_pcsxi->WriteInstructionCode( csicJump ) ;
		//
		dwJumpRefAddr = m_pcsxi->m_bufImage.GetLength() ;
		//
		m_pcsxi->WriteCodeData( &dwDummy, sizeof(DWORD) ) ;
		//
		return	dwJumpRefAddr ;
	}
}

DWORD ECSCompiler::CompileCodeJump( DWORD dwTargetAddr )
{
	DWORD	dwJumpRefAddr = CompileCodeJump() ;
	CompileCodeCommitJumpAddress( dwJumpRefAddr, dwTargetAddr ) ;
	return	dwJumpRefAddr ;
}

DWORD ECSCompiler::CompileCodeConditionalJump( bool fLogic, bool fPushAfter )
{
	if ( m_modeNakedCode )
	{
		int	regExpr = GetExpressionRegister() ;
		ESLAssert( !fPushAfter ) ;
		DWORD	dwJumpRefAddr ;
		if ( fLogic )
		{
			dwJumpRefAddr =
				m_pcsxi->WriteSakuraCJumpOffset32( regExpr, 0 ) ;
		}
		else
		{
			dwJumpRefAddr =
				m_pcsxi->WriteSakuraCNJumpOffset32( regExpr, 0 ) ;
		}
		FreeExpressionRegister() ;
		return	dwJumpRefAddr ;
	}
	else
	{
		m_pcsxi->FenceInstruction() ;
		//
		DWORD	dwJumpRefAddr ;
		BYTE	bytCondition = 0 ;
		DWORD	dwDummy = 0 ;
		//
		if ( fLogic )
		{
			bytCondition |= 0x01 ;
		}
		if ( fPushAfter )
		{
			bytCondition |= 0x02 ;
		}
		m_pcsxi->WriteInstructionCode( csicCJump ) ;
		m_pcsxi->WriteByteCode( bytCondition ) ;
		//
		dwJumpRefAddr = m_pcsxi->m_bufImage.GetLength() ;
		//
		m_pcsxi->WriteCodeData( &dwDummy, sizeof(DWORD) ) ;
		//
		return	dwJumpRefAddr ;
	}
}

DWORD ECSCompiler::CompileCodeConditionalJump
	( DWORD dwTargetAddr, bool fLogic, bool fPushAfter )
{
	DWORD	dwJumpRefAddr = CompileCodeConditionalJump( fLogic, fPushAfter ) ;
	CompileCodeCommitJumpAddress( dwJumpRefAddr, dwTargetAddr ) ;
	return	dwJumpRefAddr ;
}

void ECSCompiler::CompileCodeCommitJumpAddress
			( DWORD dwJumpRefAddr, DWORD dwTargetAddr )
{
	DWORD *	pdwJumpAddr =
		(DWORD*) m_pcsxi->m_bufImage.ModifyBuffer
							( dwJumpRefAddr, sizeof(DWORD) ) ;
	ESLAssert( pdwJumpAddr != NULL ) ;
	*pdwJumpAddr = dwTargetAddr - (dwJumpRefAddr + sizeof(DWORD)) ;
}

DWORD ECSCompiler::CompileCodeGetCurrent( void ) const
{
	return	m_pcsxi->m_bufImage.GetLength() ;
}

// スワップ命令出力
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::CompileCodeSwap( int nIndex1, int nIndex2 )
{
	ESLAssert( !m_modeNakedCode ) ;
	DWORD	dwIndex1 = nIndex1 ;
	DWORD	dwIndex2 = nIndex2 ;
	//
	m_pcsxi->WriteInstructionCode( csicSwap ) ;
	m_pcsxi->WriteByteCode( 0 ) ;
	m_pcsxi->WriteCodeData( &dwIndex1, sizeof(DWORD) ) ;
	m_pcsxi->WriteCodeData( &dwIndex2, sizeof(DWORD) ) ;
}

// naked モードロード・ストア命令
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::WriteSakuraMoveMemory
	( bool fStore,
		ECSSakura2Processor::AddressingMode mode,
		ECSSakura2Processor::DataType type, int& regDst,
		int regBase, int offset32,
		int regIndex, int scaleIndex, bool fNoRegCache )
{
	ESLError	err =
		m_pcsxi->WriteSakuraMoveMemory
			( fStore, mode, type, regDst,
				regBase, offset32, regIndex, scaleIndex, fNoRegCache ) ;
	if ( err )
	{
		return	OutputError
			( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
	}
	return	eslErrSuccess ;
}

ESLError ECSCompiler::WriteSakuraLoadMemory
	( int& regDst, const ECSTypeInfo & typeVar,
		bool fRefType, int iOffset, bool fNoRegCache )
{
	ESLError	err =
		m_pcsxi->WriteSakuraLoadMemory
			( regDst, typeVar, fRefType, iOffset, fNoRegCache ) ;
	if ( err )
	{
		return	OutputError
			( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
	}
	return	eslErrSuccess ;
}

ESLError ECSCompiler::WriteSakuraStoreMemory
	( int regSrc, const ECSTypeInfo & typeVar,
		bool fRefType, int iOffset, bool fNoRegCache )
{
	ESLError	err =
		m_pcsxi->WriteSakuraStoreMemory
			( regSrc, typeVar, fRefType, iOffset, fNoRegCache ) ;
	if ( err )
	{
		return	OutputError
			( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
	}
	return	eslErrSuccess ;
}

// ローカルメモリ命令
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::WriteSakuraMoveLocal
	( bool fStore,
		ECSSakura2Processor::LocalAddressingMode mode,
		ECSSakura2Processor::DataType type, int& regDst,
		int offset32, int regIndex, int scaleIndex, bool fNoRegCache )
{
	ESLError	err =
		m_pcsxi->WriteSakuraMoveLocal
			( fStore, mode, type, regDst,
				offset32, regIndex, scaleIndex, fNoRegCache ) ;
	if ( err )
	{
		return	OutputError
			( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
	}
	return	eslErrSuccess ;
}

// @Error 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileMacroError( ECSSourceStream & cssLine )
{
	ECSObject *	pExpr = NULL ;
	ESLError	err = CalculateExpression( pExpr, cssLine ) ;
	if ( err )
	{
		return	err ;
	}
	ESLAssert( pExpr != NULL ) ;
	if ( pExpr == NULL )
	{
		return	ESLErrorMsg( "エラーメッセージが指定されていません。" ) ;
	}
	if ( pExpr->m_vtType != csvtString )
	{
		return	ESLErrorMsg
			( "エラーメッセージに文字列が指定されていません。" ) ;
	}
	EString	strExpr = ((ECSString*)pExpr)->m_varStr ;
	delete	pExpr ;
	return	OutputError( strExpr, m_strFilePath, m_nLineNum ) ;
}

// @Warning 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileMacroWarning( ECSSourceStream & cssLine )
{
	ECSObject *	pExpr = NULL ;
	ESLError	err = CalculateExpression( pExpr, cssLine ) ;
	if ( err )
	{
		return	err ;
	}
	ESLAssert( pExpr != NULL ) ;
	if ( pExpr == NULL )
	{
		return	ESLErrorMsg( "警告メッセージが指定されていません。" ) ;
	}
	if ( pExpr->m_vtType != csvtString )
	{
		return	ESLErrorMsg
			( "警告メッセージに文字列が指定されていません。" ) ;
	}
	EString	strExpr = ((ECSString*)pExpr)->m_varStr ;
	delete	pExpr ;
	return	OutputWarning( strExpr, m_strFilePath, m_nLineNum ) ;
}

// @Compile 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileMacroCompile( ECSSourceStream & cssLine )
{
	ECSObject *	pExpr = NULL ;
	ESLError	err = CalculateExpression( pExpr, cssLine ) ;
	if ( err )
	{
		return	err ;
	}
	ESLAssert( pExpr != NULL ) ;
	if ( pExpr == NULL )
	{
		return	eslErrSuccess ;
	}
	if ( pExpr->m_vtType != csvtString )
	{
		return	ESLErrorMsg( "@Compile 文に文字列が指定されていません。" ) ;
	}
	ECSSourceStream	cssStatement = ((ECSString*)pExpr)->m_varStr ;
	delete	pExpr ;
	return	CompileScriptLine( cssStatement, m_nLineNum, NULL, false ) ;
}

// @If 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileMacroIf( ECSSourceStream & cssLine )
{
	EObjArray<EWideString>	lstParam ;
	EStreamWideString	swsUsage = L" (%x) [(&!then&) (%x)] [(&!else&) (%x)]\\" ;
	ESLError	err =
		cssLine.IsMatchUsage( swsUsage, m_strErrMsg, &lstParam ) ;
	if ( err )
	{
		return	ESLErrorMsg( "@If 文の書式が不正です。" ) ;
	}
	bool	fEnabledNest = true ;
	EMacroNest *	pmnNest = m_nestMacro.GetLastAt( ) ;
	if ( (pmnNest != NULL)
		&& (pmnNest->m_nCondition != EMacroNest::condNormal) )
	{
		fEnabledNest = false ;
	}
	bool	fExpr = false ;
	ECSSourceStream	cssExpr = lstParam[0] ;
	ECSObject *	pExpr = NULL ;
	if ( fEnabledNest )
	{
		err = CalculateExpression( pExpr, cssExpr ) ;
		if ( err )
		{
			return	err ;
		}
		err = EvaluateVariable( pExpr ) ;
		delete	pExpr ;
		if ( err == eslErrInvalidParam )
		{
			return	ESLErrorMsg( "条件式の型が不正です。" ) ;
		}
		fExpr = (err == eslErrSuccess) ;
	}
	if ( lstParam[1].IsEmpty() && lstParam[3].IsEmpty() )
	{
		//
		// @If ブロックの作成
		//
		EMacroNest *	pmnIf = new EMacroNest( mwIf ) ;
		if ( !fEnabledNest )
		{
			pmnIf->m_nCondition = EMacroNest::condPass ;
		}
		else if ( fExpr )
		{
			pmnIf->m_nCondition = EMacroNest::condNormal ;
		}
		else
		{
			pmnIf->m_nCondition = EMacroNest::condNext ;
		}
		m_nestMacro.Add( pmnIf ) ;
	}
	else if ( fEnabledNest )
	{
		//
		// インライン実行
		//
		EMacroNest *	pmnNest = m_nestMacro.GetLastAt( ) ;
		cssExpr = L"" ;
		if ( fExpr )
		{
			if ( !lstParam[1].IsEmpty() )
			{
				cssExpr = lstParam[2] ;
			}
		}
		else
		{
			if ( !lstParam[3].IsEmpty() )
			{
				cssExpr = lstParam[4] ;
			}
		}
		if ( !cssExpr.IsEmpty() )
		{
			//
			// マクロ変数への代入
			//
			err = CalculateExpression( pExpr, cssExpr, 0, NULL, false ) ;
			if ( err )
			{
				return	err ;
			}
			delete	pExpr ;
		}
	}
	return	eslErrSuccess ;
}

// @ElseIf 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileMacroElseIf( ECSSourceStream & cssLine )
{
	EMacroNest *	pmnNest = m_nestMacro.GetLastAt( ) ;
	if ( (pmnNest == NULL)
		|| ((pmnNest->m_mwType != mwIf)
			&& (pmnNest->m_mwType != mwElseIf)) )
	{
		return	ESLErrorMsg( "@ElseIf に対応する @If がありません。" ) ;
	}
	if ( pmnNest->m_nCondition == EMacroNest::condNext )
	{
		ECSObject *	pExpr = NULL ;
		ESLError	err = CalculateExpression( pExpr, cssLine ) ;
		if ( err )
		{
			return	err ;
		}
		err = EvaluateVariable( pExpr ) ;
		delete	pExpr ;
		if ( err == eslErrInvalidParam )
		{
			return	ESLErrorMsg( "条件式の型が不正です。" ) ;
		}
		if ( err == eslErrSuccess )
		{
			pmnNest->m_nCondition = EMacroNest::condNormal ;
		}
	}
	else if ( pmnNest->m_nCondition == EMacroNest::condNormal )
	{
		pmnNest->m_nCondition = EMacroNest::condPass ;
	}
	cssLine.MoveIndex( cssLine.GetLength() ) ;
	return	eslErrSuccess ;
}

// @Else
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileMacroElse( ECSSourceStream & cssLine )
{
	EMacroNest *	pmnNest = m_nestMacro.GetLastAt( ) ;
	if ( (pmnNest == NULL)
		|| ((pmnNest->m_mwType != mwIf)
			&& (pmnNest->m_mwType != mwElseIf)) )
	{
		return	ESLErrorMsg( "@Else に対応する @If がありません。" ) ;
	}
	pmnNest->m_mwType = mwElse ;
	//
	if ( pmnNest->m_nCondition == EMacroNest::condNext )
	{
		pmnNest->m_nCondition = EMacroNest::condNormal ;
	}
	else if ( pmnNest->m_nCondition == EMacroNest::condNormal )
	{
		pmnNest->m_nCondition = EMacroNest::condPass ;
	}
	return	eslErrSuccess ;
}

// @EndIf 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileMacroEndIf( ECSSourceStream & cssLine )
{
	EMacroNest *	pmnNest = m_nestMacro.GetLastAt( ) ;
	if ( (pmnNest == NULL)
		|| ((pmnNest->m_mwType != mwIf)
			&& (pmnNest->m_mwType != mwElseIf)
			&& (pmnNest->m_mwType != mwElse)) )
	{
		return	ESLErrorMsg( "@EndIf に対応する @If がありません。" ) ;
	}
	m_nestMacro.RemoveAt( m_nestMacro.GetSize() - 1 ) ;
	return	eslErrSuccess ;
}

// @Let 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileMacroLet( ECSSourceStream & cssLine )
{
	ECSObject *	pExpr = NULL ;
	ESLError	err = CalculateExpression( pExpr, cssLine, 0, NULL, false ) ;
	if ( !err )
	{
		delete	pExpr ;
	}
	return	err ;
}

// @Local 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileMacroLocal( ECSSourceStream & cssLine )
{
	EMacroNest *	pmnNest = m_nestMacro.GetLastAt( ) ;
	if ( pmnNest == NULL )
	{
		return	ESLErrorMsg( "@Local 文はマクロブロック内で有効です" ) ;
	}
	while ( !cssLine.DisregardSpace() )
	{
		//
		// マクロ変数名取得
		//
		int				nTokenType ;
		ECSWideString	wstrName = cssLine.GetAToken( &nTokenType ) ;
		if ( nTokenType != 0 )
		{
			return	ESLErrorMsg( "マクロ変数名が不正です。" ) ;
		}
		if ( pmnNest->m_staLocal.GetAs( wstrName ) != NULL )
		{
			m_strErrMsg = "マクロ変数 \'"
				+ EString( wstrName ) + "\' はすでに定義されています。" ;
			return	ESLErrorMsg( m_strErrMsg ) ;
		}
		//
		// 初期値取得
		//
		ECSObject *	pVar = NULL ;
		if ( cssLine.HasToComeToken( L":=" ) )
		{
			ESLError	err = CalculateExpression( pVar, cssLine, 0, L"," ) ;
			if ( err )
			{
				return	err ;
			}
		}
		else
		{
			pVar = new ECSInteger ;
		}
		//
		// マクロ変数登録
		//
		pmnNest->m_staLocal.SetAs( wstrName, pVar ) ;
		//
		// 終了判定
		//
		if ( cssLine.HasToComeChar( L"," ) != L',' )
		{
			break ;
		}
	}
	return	eslErrSuccess ;
}

// @For 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileMacroFor( ECSSourceStream & cssLine )
{
	//
	// @For ネスト作成
	//
	EMacroNest *	pmnNext = new EMacroNest( mwFor ) ;
	//
	// 構文解析
	//
	EObjArray<EWideString>	lstParam ;
	EStreamWideString		swsUsage =
		L" [(%t) | (%x)] [&!as& (%t) [:= (%x)]]"
			L" [{&!to& (%x)|&!while& (%x)}] [{&!step& (%x)|&!by& (%x)}]\\" ;
	ESLError	err =
		cssLine.IsMatchUsage( swsUsage, m_strErrMsg, &lstParam ) ;
	if ( err )
	{
		delete	pmnNext ;
		return	ESLErrorMsg( "@For 構文が不正です。" ) ;
	}
	pmnNext->m_wstrElementVar = lstParam[0] ;
	pmnNext->m_wstrArrayExpr = lstParam[1] ;
	if ( pmnNext->m_wstrElementVar.IsEmpty()
			&& !pmnNext->m_wstrArrayExpr.IsEmpty() )
	{
		delete	pmnNext ;
		return	ESLErrorMsg( "配列参照変数名が指定されていません。" ) ;
	}
	else if ( !pmnNext->m_wstrElementVar.IsEmpty()
			&& pmnNext->m_wstrArrayExpr.IsEmpty() )
	{
		delete	pmnNext ;
		return	ESLErrorMsg( "配列式が指定されていません。" ) ;
	}
	pmnNext->m_wstrLoopVar = lstParam[2] ;
	if ( pmnNext->m_wstrLoopVar.IsEmpty() )
	{
		for ( int i = 0; i < (int) lstParam.GetSize(); i ++ )
		{
			if ( !lstParam[i].IsEmpty() )
			{
				pmnNext->m_wstrLoopVar = L"@I" ;
				break ;
			}
		}
	}
	pmnNext->m_wstrFrom = lstParam[3] ;
	if ( pmnNext->m_wstrFrom.IsEmpty() )
	{
		pmnNext->m_wstrFrom = L"0" ;
	}
	pmnNext->m_wstrTo = lstParam[4] ;
	pmnNext->m_wstrWhile = lstParam[5] ;
	if ( pmnNext->m_wstrTo.IsEmpty()
			&& pmnNext->m_wstrWhile.IsEmpty()
			&& !pmnNext->m_wstrArrayExpr.IsEmpty() )
	{
		pmnNext->m_wstrWhile =
			pmnNext->m_wstrLoopVar + L" < ("
				+ pmnNext->m_wstrArrayExpr + L").GetLength()" ;
	}
	if ( lstParam[6].IsEmpty() )
	{
		if ( lstParam[7].IsEmpty() )
		{
			if ( !pmnNext->m_wstrLoopVar.IsEmpty() )
			{
				pmnNext->m_wstrStep = pmnNext->m_wstrLoopVar + L" += 1" ;
			}
		}
		else
		{
			pmnNext->m_wstrStep = lstParam[7] ;
		}
	}
	else
	{
		pmnNext->m_wstrStep =
			pmnNext->m_wstrLoopVar + L" += " + ECSWideString( lstParam[6] ) ;
	}
	m_nestMacro.Add( pmnNext ) ;
	return	eslErrSuccess ;
}

// @Next 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileMacroNext( ECSSourceStream & cssLine )
{
	EMacroNest *	pmnNext = m_nestMacro.GetLastAt( ) ;
	if ( (pmnNext == NULL) || (pmnNext->m_mwType != mwFor) )
	{
		return	ESLErrorMsg( "@Next に対応する @For 文がありません。" ) ;
	}
	//
	// ループ変数作成
	//
	ESLError		err ;
	ECSObject *		pExpr ;
	ECSObject *		pLoopVar = NULL ;
	ECSSourceStream	cssExpr ;
	if ( !pmnNext->m_wstrLoopVar.IsEmpty() )
	{
		cssExpr = pmnNext->m_wstrFrom ;
		err = CalculateExpression( pLoopVar, cssExpr ) ;
		if ( err )
		{
			return	err ;
		}
		pmnNext->m_staLocal.SetAs( pmnNext->m_wstrLoopVar, pLoopVar ) ;
	}
	for ( ; ; )
	{
		//
		// ループ判定
		//
		if ( !pmnNext->m_wstrWhile.IsEmpty() )
		{
			cssExpr = pmnNext->m_wstrWhile ;
			err = CalculateExpression( pExpr, cssExpr ) ;
			if ( err )
			{
				break ;
			}
			err = EvaluateVariable( pExpr ) ;
			delete	pExpr ;
			if ( err != eslErrSuccess )
			{
				err = eslErrSuccess ;
				break ;
			}
		}
		//
		// ループ用マクロネスト作成
		//
		EMacroNest *	pmnLoop = new EMacroNest( mwMacro ) ;
		if ( !pmnNext->m_wstrElementVar.IsEmpty()
			&& !pmnNext->m_wstrArrayExpr.IsEmpty() )
		{
			cssExpr = L"(" + pmnNext->m_wstrArrayExpr
							+ L")[" + pmnNext->m_wstrLoopVar + L"]" ;
			err = CalculateExpression( pExpr, cssExpr ) ;
			if ( err )
			{
				delete	pmnLoop ;
				break ;
			}
			pmnLoop->m_staLocal.Add( pmnNext->m_wstrElementVar, pExpr ) ;
		}
		unsigned int	nNestCount = m_nestMacro.GetSize( ) ;
		bool			fBreakLoop = false ;
		m_nestMacro.Add( pmnLoop ) ;
		//
		for ( int i = 0; i < (int) pmnNext->m_lstCodeBuf.GetSize(); i ++ )
		{
			//
			// １行コンパイル
			//
			ECSSourceStream *	pcssCode = pmnNext->m_lstCodeBuf.GetAt( i ) ;
			if ( pcssCode != NULL )
			{
				pcssCode->MoveIndex( 0 ) ;
				err = CompileScriptLine( *pcssCode, m_nLineNum, NULL, false ) ;
				if ( err )
				{
					m_strErrMsg =
						"@For(" + EString(i+1) + "): "
							+ EString( GetESLErrorMsg( err ) ) ;
					err = ESLErrorMsg( m_strErrMsg ) ;
					break ;
				}
			}
			if ( m_nestMacro.GetSize() == nNestCount )
			{
				fBreakLoop = true ;
				break ;
			}
		}
		m_nestMacro.SetSize( nNestCount ) ;
		if ( err || fBreakLoop )
		{
			break ;
		}
		//
		// 継続処理
		//
		if ( !pmnNext->m_wstrLoopVar.IsEmpty()
				&& !pmnNext->m_wstrTo.IsEmpty() )
		{
			cssExpr =
				pmnNext->m_wstrLoopVar + L" == (" + pmnNext->m_wstrTo + L")" ;
			err = CalculateExpression( pExpr, cssExpr ) ;
			if ( err )
			{
				break ;
			}
			err = EvaluateVariable( pExpr ) ;
			delete	pExpr ;
			if ( err == eslErrSuccess )
			{
				err = eslErrSuccess ;
				break ;
			}
		}
		if ( !pmnNext->m_wstrStep.IsEmpty() )
		{
			cssExpr = pmnNext->m_wstrStep ;
			err = CalculateExpression( pExpr, cssExpr, 0, NULL, false ) ;
			if ( err )
			{
				break ;
			}
			delete	pExpr ;
		}
	}
	//
	// @Next ネスト削除
	//
	ESLAssert( pmnNext == m_nestMacro.GetLastAt() ) ;
	m_nestMacro.RemoveAt( m_nestMacro.GetSize() - 1 ) ;
	//
	return	err ;
}

// @Literal 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileMacroLiteral( ECSSourceStream & cssLine )
{
	//
	// リテラル文字取得
	//
	if ( cssLine.HasToComeChar( L"\"" ) != L'\"' )
	{
		return	ESLErrorMsg
			( "リテラル定数名が \" 記号で囲まれていません。" ) ;
	}
	int	nTokenType ;
	int	nIndex = cssLine.GetIndex( ) ;
	cssLine.PassAToken( &nTokenType ) ;
	if ( nTokenType != 0 )
	{
		return	ESLErrorMsg( "リテラル定数名が不正です。" ) ;
	}
	ECSWideString	wstrName
		( ((const wchar_t *) cssLine) + nIndex, cssLine.GetIndex() - nIndex ) ;
	//
	if ( cssLine.HasToComeChar( L"\"" ) != L'\"' )
	{
		return	ESLErrorMsg
			( "リテラル定数名が \" 記号で閉じられていません。" ) ;
	}
	//
	// 置き換え定数式取得
	//
	if ( !cssLine.HasToComeToken( L":=" ) )
	{
		return	ESLErrorMsg
			( "文字列式が := 演算子で結び付けられていません。" ) ;
	}
	cssLine.DisregardSpace( ) ;
	//
	ECSSourceStream *	pcssExpr =
		new ECSSourceStream( ((const wchar_t *) cssLine) + cssLine.GetIndex() ) ;
	cssLine.MoveIndex( cssLine.GetLength() ) ;
	//
	m_staLiteral.SetAs( wstrName, pcssExpr ) ;
	//
	return	eslErrSuccess ;
}

// @Macro 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileMacroDefMacro( ECSSourceStream & cssLine )
{
	//
	// マクロ名取得
	//
	int				nTokenType ;
	ECSWideString	wstrName = cssLine.GetAToken( &nTokenType ) ;
	if ( nTokenType != 0 )
	{
		return	ESLErrorMsg( "マクロ名が不正です。" ) ;
	}
	if ( m_staMacro.GetAs( wstrName ) != NULL )
	{
		m_strErrMsg = "マクロ \'"
			+ EString( wstrName ) + "\' はすでに定義されています。" ;
		return	ESLErrorMsg( m_strErrMsg ) ;
	}
	//
	// マクロ書式取得
	//
	wchar_t		wch = cssLine.HasToComeChar( L":\"\'(" ) ;
	bool		fMacroFunc = false ;
	EObjArray<EWideString>	lstArgName ;
	EWideString	wstrUsage ;
	if ( wch == L':' )
	{
		wstrUsage = cssLine.Middle( cssLine.GetIndex() ) ;
		cssLine.MoveIndex( cssLine.GetLength() ) ;
	}
	else if ( (wch == L'\"') || (wch == L'\'') )
	{
		wstrUsage = cssLine.GetEnclosedString( wch, m_dwModeFlags ) ;
		EDescription::DecodeTextCEscSequence( wstrUsage ) ;
	}
	else if ( wch == L'(' )
	{
		wch = cssLine.HasToComeChar( L")" ) ;
		while ( wch != L')' )
		{
			int			nTokenType ;
			EWideString	wstrToken = cssLine.GetAToken( &nTokenType ) ;
			if ( nTokenType != 0 )
			{
				return	ESLErrorMsg( "マクロ関数の引数名が不正です。" ) ;
			}
			lstArgName.Add( new EWideString( wstrToken ) ) ;
			//
			wch = cssLine.HasToComeChar( L",)" ) ;
			if ( wch == L'\0' )
			{
				return	ESLErrorMsg
					( "マクロ関数の引数が丸括弧で閉じられていません。" ) ;
			}
		}
		fMacroFunc = true ;
	}
	else if ( !cssLine.DisregardSpace() )
	{
		return	ESLErrorMsg( "@Macro 文の書式が不正です。" ) ;
	}
	//
	// マクロネスト作成
	//
	EMacroNest *	pmnMacro = new EMacroNest( mwDefMacro ) ;
	pmnMacro->m_fMacroFunction = fMacroFunc ;
	pmnMacro->m_wstrMacroName = wstrName ;
	if ( !fMacroFunc )
	{
		if ( wstrUsage.Right(1) != L"\\" )
		{
			wstrUsage += L'\\' ;
		}
		pmnMacro->m_wstrMacroUsage = wstrUsage ;
	}
	else
	{
		pmnMacro->m_lstMacroArgName.Merge( 0, lstArgName ) ;
	}
	m_nestMacro.Add( pmnMacro ) ;
	//
	return	eslErrSuccess ;
}

// @EndMacro 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileMacroEndMacro( ECSSourceStream & cssLine )
{
	EMacroNest *	pmnMacro = m_nestMacro.GetLastAt( ) ;
	if ( (pmnMacro == NULL) || (pmnMacro->m_mwType != mwDefMacro) )
	{
		return	ESLErrorMsg( "@EndMacro に対応する @Macro 文がありません。" ) ;
	}
	ESLError	err = eslErrSuccess ;
	EMacroBlock *	pMacroBlock = new EMacroBlock ;
	pMacroBlock->m_fMacroFunc = pmnMacro->m_fMacroFunction ;
	pMacroBlock->m_wstrName = pmnMacro->m_wstrMacroName ;
	if ( !pmnMacro->m_fMacroFunction )
	{
		ECSSourceStream	cssUsage = pmnMacro->m_wstrMacroUsage ;
		err = cssUsage.ParseUsage( pMacroBlock->m_usage, cssUsage, m_strErrMsg ) ;
		if ( err )
		{
			pMacroBlock->m_usage.RemoveAll( ) ;
			err = ESLErrorMsg( "マクロの書式構文が不正です。" ) ;
		}
		//
		if ( m_staConstant.GetAs( pmnMacro->m_wstrMacroName ) == NULL )
		{
			m_staConstant.SetAs
				( pmnMacro->m_wstrMacroName, new ECSInteger( -1 ) ) ;
		}
	}
	else
	{
		pMacroBlock->m_lstArgName.Merge( 0, pmnMacro->m_lstMacroArgName ) ;
	}
	pMacroBlock->Merge( 0, pmnMacro->m_lstCodeBuf ) ;
	m_staMacro.SetAs( pmnMacro->m_wstrMacroName, pMacroBlock ) ;
	//
	if ( pmnMacro->m_wstrMacroName == L"@preprocess" )
	{
		m_pPreprocessMacro = pMacroBlock ;
		m_pPreprocessFlag =
			ESLTypeCast<ECSInteger>
				( m_staConstant.GetAs( pmnMacro->m_wstrMacroName ) ) ;
	}
	//
	m_nestMacro.RemoveAt( m_nestMacro.GetSize() - 1 ) ;
	//
	return	err ;
}

// @ExitMacro 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileMacroExitMacro( ECSSourceStream & cssLine )
{
	ECSObject *	pRetValue = NULL ;
	if ( !cssLine.DisregardSpace() )
	{
		//
		// マクロ関数返り値取得
		//
		ESLError	err = CalculateExpression( pRetValue, cssLine ) ;
		if ( err )
		{
			return	err ;
		}
	}
	//
	// マクロネスト削除
	//
	EMacroNest *	pmnMacro ;
	int	i = m_nestMacro.GetSize() - 1 ;
	while ( i >= 0 )
	{
		pmnMacro = m_nestMacro.GetAt( i ) ;
		if ( (pmnMacro != NULL) && (pmnMacro->m_mwType == mwMacro) )
		{
			break ;
		}
		i -- ;
	}
	if ( (pmnMacro == NULL) || (pmnMacro->m_mwType != mwMacro) )
	{
		return	ESLErrorMsg
			( "@ExitMacro 文に対応するマクロブロックがありません。" ) ;
	}
	m_nestMacro.SetSize( i ) ;
	//
	// マクロ関数返り値設定
	//
	pmnMacro = m_nestMacro.GetLastAt( ) ;
	if ( (pmnMacro != NULL) &&
		(pmnMacro->m_fMacroFunction | pmnMacro->m_fMacroStatement) )
	{
		pmnMacro->m_staLocal.SetAs( L"@return", pRetValue ) ;
	}
	else
	{
		delete	pRetValue ;
	}
	return	eslErrSuccess ;
}

// @UndefMacro 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileMacroUndefMacro( ECSSourceStream & cssLine )
{
	ECSWideString	wstrName = cssLine.GetAToken( ) ;
	m_staMacro.RemoveAs( wstrName ) ;
	return	eslErrSuccess ;
}

// Include 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileInclude( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwInclude ) )
	{
		return	eslErrSuccess ;
	}
	wchar_t	wch = cssLine.HasToComeChar( L"\"<" ) ;
	EString	strFilePath ;
	if ( wch == L'\"' )
	{
		strFilePath =
			cssLine.GetEnclosedString
				( L'\"', cssLine.flagDisableExpression ) ;
	}
	else if ( wch == L'<' )
	{
		strFilePath =
			cssLine.GetEnclosedString
				( L'>', cssLine.flagDisableExpression ) ;
	}
	else
	{
		return	ESLErrorMsg
			( "インクルードするファイル名が指定されていません。" ) ;
	}
	//
	ESLFileObject *	pfile = OpenScriptFile( strFilePath ) ;
	if ( pfile == NULL )
	{
		m_strErrMsg = "\'" + strFilePath + "\' ファイルを開けませんでした。" ;
		return	ESLErrorMsg( m_strErrMsg ) ;
	}
	ERawFile *	pRawFile = ESLTypeCast<ERawFile>( pfile ) ;
	if ( pRawFile != NULL )
	{
		strFilePath = pRawFile->GetFilePath() ;
	}
	//
	ECSSourceStream	cssScript ;
	cssScript.ReadTextFile( *pfile ) ;
	delete	pfile ;
	//
	EString	strCurrentFile = m_strFilePath ;
	int		nCurrentLineNum = m_nLineNum ;
	ESLError	err = CompileScript( cssScript, strFilePath ) ;
	m_strFilePath = strCurrentFile ;
	m_nLineNum = nCurrentLineNum ;
	return	err ;
}

// Option 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileOption( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwOption ) )
	{
		return	eslErrSuccess ;
	}
	enum	OptionUsageIndex
	{
		optionStrictType,
		optionLooseType,
		optionNoWarningMoveByEqual,
		optionWarningMoveByEqual,
		optionCStyleNumber,
		optionBasicStyleNumber,
		optionNoEscapeInSingleQuote,
		optionEscapeInSingleQuote,
		optionCharacterCodeBySingleQuote,
		optionStringBySingleQuote,
		optionRealReservedWord,
		optionSmallReservedWord,
		optionNoCaseReservedWord,
		optionExpresslyVirtual,
		optionTacitlyVirtual,
		optionObjectNew,
		optionNakedNew,
		optionTacitlyNakedCall,
		optionExpresslyNakedCall,
		optionDefaultNaked,
		optionDefaultObjected,
		optionCStyleCast,
		optionNoCStyleCast,
		optionCStyleBasicType,
		optionNoCStyleBasicType,
		optionCompatibleInt32,
		optionDefaultInt64,
	} ;
	static const wchar_t *	pwszOptionUsages[] =
	{
		L"strict type",
		L"loose type",
		L"no warning move by equal",
		L"warning move by equal",
		L"c style number",
		L"basic style number",
		L"no esc in single quote",
		L"esc in single quote",
		L"character code by single quote",
		L"string by single quote",
		L"real reserved word",
		L"small reserved word",
		L"no case reserved word",
		L"expressly virtual",
		L"tacitly virtual",
		L"object new",
		L"naked new",
		L"tacitly naked call",
		L"expressly naked call",
		L"default naked",
		L"default objected",
		L"c style cast",
		L"no c style cast",
		L"c style basic type",
		L"no c style basic type",
		L"compatible int32",
		L"default int64",
		NULL
	} ;
	int	iBeginIndex = cssLine.GetIndex() ;
	int	iUsage = -1 ;
	for ( int i = 0; pwszOptionUsages[i] != NULL; i ++ )
	{
		EStreamWideString	swsUsage = pwszOptionUsages[i] ;
		cssLine.MoveIndex( iBeginIndex ) ;
		//
		for ( ; ; )
		{
			EWideString	wstrToken = swsUsage.GetAToken() ;
			if ( wstrToken.CompareNoCase( cssLine.GetAToken() ) )
			{
				break ;
			}
			if ( wstrToken.IsEmpty() )
			{
				iUsage = i ;
				break ;
			}
		}
		if ( iUsage >= 0 )
		{
			break ;
		}
	}
	switch ( iUsage )
	{
	case	optionStrictType:
		m_dwModeFlags |= flagStrictStyle ;
		break ;
	case	optionLooseType:
		m_dwModeFlags &= ~flagStrictStyle ;
		break ;
	case	optionNoWarningMoveByEqual:
		m_dwModeFlags |= flagNoWarningEquMove ;
		break ;
	case	optionWarningMoveByEqual:
		m_dwModeFlags &= ~flagNoWarningEquMove ;
		break ;
	case	optionCStyleNumber:
		m_dwModeFlags |= flagCStyleNumberLiteral ;
		break ;
	case	optionBasicStyleNumber:
		m_dwModeFlags &= ~flagCStyleNumberLiteral ;
		break ;
	case	optionNoEscapeInSingleQuote:
		m_dwModeFlags |= flagQuoteNakedString ;
		break ;
	case	optionEscapeInSingleQuote:
		m_dwModeFlags &= ~flagQuoteNakedString ;
		break ;
	case	optionCharacterCodeBySingleQuote:
		m_dwModeFlags |= flagQuoteCharactorCode ;
		break ;
	case	optionStringBySingleQuote:
		m_dwModeFlags &= ~flagQuoteCharactorCode ;
		break ;
	case	optionRealReservedWord:
		m_dwModeFlags =
			(m_dwModeFlags
				& ~flagReservedWordCaseMask) | flagRealReservedWord ;
		break ;
	case	optionSmallReservedWord:
		m_dwModeFlags =
			(m_dwModeFlags
				& ~flagReservedWordCaseMask) | flagSmallReservedWord ;
		break ;
	case	optionNoCaseReservedWord:
		m_dwModeFlags &= ~flagReservedWordCaseMask ;
		break ;
	case	optionExpresslyVirtual:
		m_dwModeFlags |= flagNoDefaultVirtual ;
		break ;
	case	optionTacitlyVirtual:
		m_dwModeFlags &= ~flagNoDefaultVirtual ;
		break ;
	case	optionObjectNew:
		m_dwModeFlags &= ~flagDefaultNakedNew ;
		break ;
	case	optionNakedNew:
		m_dwModeFlags |= flagDefaultNakedNew ;
		break ;
	case	optionTacitlyNakedCall:
		m_dwModeFlags |= flagDefualtNakedFunc ;
		break ;
	case	optionExpresslyNakedCall:
		m_dwModeFlags &= ~flagDefualtNakedFunc ;
		break ;
	case	optionDefaultNaked:
		m_dwModeFlags |= flagDefaultNakedAll ;
		break ;
	case	optionDefaultObjected:
		m_dwModeFlags &= ~flagDefaultNakedAll ;
		break ;
	case	optionCStyleCast:
		m_dwModeFlags |= flagCStyleCast ;
		break ;
	case	optionNoCStyleCast:
		m_dwModeFlags &= ~flagCStyleCast ;
		break ;
	case	optionCStyleBasicType:
		m_dwModeFlags |= flagCStyleBasicType ;
		break ;
	case	optionNoCStyleBasicType:
		m_dwModeFlags &= ~flagCStyleBasicType ;
		break ;
	case	optionCompatibleInt32:
		m_dwModeFlags |= flagCompatibleInt32 ;
		break ;
	case	optionDefaultInt64:
		m_dwModeFlags &= ~flagCompatibleInt32 ;
		break ;
	default:
		return	ESLErrorMsg( "不正な Option 引数です。" ) ;
	}
	return	eslErrSuccess ;
}

// DeclareType 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileDeclareType( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwDeclareType ) )
	{
		return	eslErrSuccess ;
	}
	while ( !cssLine.DisregardSpace() )
	{
		int		nTokenType ;
		ECSWideString	wstrTypeName = cssLine.GetAToken( &nTokenType ) ;
		if ( nTokenType != 0 )
		{
			return	ESLErrorMsg( "型名に不正な文字が使われています。" ) ;
		}
		EWideString	wstrSpaceName = GetCurrentSpaceName() ;
		if ( !wstrSpaceName.IsEmpty() )
		{
			wstrTypeName = wstrSpaceName + L"::" + wstrTypeName ;
		}
		if ( m_staTypeName.FindIndex( wstrTypeName ) < 0 )
		{
			m_staTypeName.Add( wstrTypeName ) ;
		}
		EWideString	wstrNext = cssLine.GetAToken() ;
		if ( CompareReservedWord( L"As", wstrNext ) == 0 )
		{
			wstrNext = cssLine.GetAToken() ;
			if ( (CompareReservedWord( L"Class", wstrNext ) == 0)
				|| (CompareReservedWord( L"Structure", wstrNext ) == 0)
				|| (CompareReservedWord( L"Enumerator", wstrNext ) == 0)
				|| (CompareReservedWord( L"Union", wstrNext ) == 0) )
			{
				ECSClassInfo *	pClassInf = GetClassInfoAs( wstrTypeName ) ;
				if ( pClassInf == NULL )
				{
					pClassInf = new ECSClassInfo ;
					pClassInf->SetName( wstrTypeName ) ;
					pClassInf->SetGlobalName( wstrTypeName ) ;
					m_pcsxiDst->AddClassInfo( pClassInf ) ;
				}
				wstrNext = cssLine.GetAToken() ;
			}
			else
			{
				return	ESLErrorMsg( "型宣言の書式が不正です。" ) ;
			}
		}
		if ( wstrNext.IsEmpty() )
		{
			break ;
		}
		if ( wstrNext != L"," )
		{
			return	ESLErrorMsg( "型名が , 記号で区切られていません。" ) ;
		}
	}
	return	eslErrSuccess ;
}

// DeclareDef 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileDeclareDef( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwDeclareDef ) )
	{
		return	eslErrSuccess ;
	}
	bool	fDataDef = false ;
	bool	fSharedDef = false ;
	bool	fNakedDef = false ;
	bool	fObjectedDef = false ;
	for ( ; ; )
	{
		int				nIndex = cssLine.GetIndex( ) ;
		ECSWideString	wstrToken = cssLine.GetAToken( ) ;
		if ( CompareReservedWord( L"Data", wstrToken ) == 0 )
		{
			fDataDef = true ;
			fSharedDef = true ;
		}
		else if ( wstrToken == L"shared" )
		{
			fSharedDef = true ;
		}
		else if ( wstrToken == L"naked" )
		{
			fNakedDef = true ;
		}
		else if ( wstrToken == L"objected" )
		{
			fObjectedDef = true ;
		}
		else
		{
			cssLine.MoveIndex( nIndex ) ;
			break ;
		}
	}
	if ( !fObjectedDef && (m_dwModeFlags & flagDefaultNakedAll) )
	{
		fNakedDef = true ;
	}
	while ( !cssLine.DisregardSpace() )
	{
		//
		// 定義名取得
		//
		int		nTokenType ;
		ECSWideString	wstrToken = cssLine.GetAToken( &nTokenType ) ;
		if ( nTokenType != 0 )
		{
			return	ESLErrorMsg( "変数名に不正な文字が使われています。" ) ;
		}
		ESLError	err = VerifyUserSymbol( wstrToken ) ;
		if ( err )
		{
			return	err ;
		}
		EWideString	wstrSpaceName = GetCurrentSpaceName() ;
		if ( !wstrSpaceName.IsEmpty() )
		{
			wstrToken = wstrSpaceName + L"::" + wstrToken ;
		}
		//
		// 型記述
		//
		bool		fDeclareType = false ;
		ECSTypeInfo	typeVar ;
		if ( cssLine.HasToComeChar( L":" ) == L':' )
		{
			err = ParseTypeDescription( typeVar, cssLine ) ;
			if ( err )
			{
				return	err ;
			}
			fDeclareType = true ;
		}
		else if ( fNakedDef )
		{
			return	ESLErrorMsg( "naked 変数の宣言に型の指定がありません" ) ;
		}
		else if ( fDataDef && (cssLine.HasToComeChar( L"<" ) == L'<') )
		{
			ECSTypeInfo	typeElement ;
			err = ParseTypeDescription( typeElement, cssLine ) ;
			if ( err )
			{
				return	err ;
			}
			if ( cssLine.HasToComeChar( L">" ) != L'>' )
			{
				m_strErrMsg =
					"Data テーブル要素型の指定が"
					" \'>\' 記号で閉じられていません。" ;
				return	ESLErrorMsg( m_strErrMsg ) ;
			}
			const ECSClassInfo *
					pGlobalType = GetClassInfoAs( L"Global" ) ;
			if ( pGlobalType == NULL )
			{
				m_strErrMsg = "Data 型の利用には Global 型の宣言が必要です" ;
				return	ESLErrorMsg( m_strErrMsg ) ;
			}
			ECSStructure *	psDataType = new ECSStructure( pGlobalType ) ;
			psDataType->SetDefaultElement( typeElement.DetachValue() ) ;
			typeVar.SetTypeValue( psDataType, 0 ) ;
			fDeclareType = true ;
		}
		//
		// 参照リスト登録
		//
		ENumArray<DWORD> *	pList =
			DeclareVariableReferenceDef
				( wstrToken, fNakedDef, fSharedDef,
						((typeVar.m_dwFlags & ECSTypeInfo::flagConstant) != 0) ) ;
		//
		if ( fDeclareType )
		{
			err = ExternVariableType( wstrToken, typeVar, fNakedDef ) ;
			if ( err )
			{
				return	err ;
			}
		}
		if ( cssLine.DisregardSpace() )
		{
			break ;
		}
		if ( cssLine.GetCharacter() != L',' )
		{
			return	ESLErrorMsg( "変数名が , 記号で区切られていません。" ) ;
		}
	}
	return	eslErrSuccess ;
}

// 変数参照リスト登録
//////////////////////////////////////////////////////////////////////////////
ENumArray<DWORD> *
	ECSCompiler::DeclareVariableReferenceDef
		( const wchar_t * pwszVarName,
			bool fNakedDef, bool fSharedDef, bool fConstDef )
{
	ENumArray<DWORD> *	pList = NULL ;
	if ( !fNakedDef )
	{
		if ( fSharedDef )
		{
			pList = m_pcsxiDst->m_impDataRef.GetAs( pwszVarName ) ;
			if ( pList == NULL )
			{
				pList = new ENumArray<DWORD> ;
				m_pcsxiDst->m_impDataRef.SetAs( pwszVarName, pList ) ;
			}
			pList = m_csxiInitFunc.m_impDataRef.GetAs( pwszVarName ) ;
			if ( pList == NULL )
			{
				pList = new ENumArray<DWORD> ;
				m_csxiInitFunc.m_impDataRef.SetAs( pwszVarName, pList ) ;
			}
		}
		else
		{
			pList = m_pcsxiDst->m_impGlobalRef.GetAs( pwszVarName ) ;
			if ( pList == NULL )
			{
				pList = new ENumArray<DWORD> ;
				m_pcsxiDst->m_impGlobalRef.SetAs( pwszVarName, pList ) ;
			}
			pList = m_csxiInitFunc.m_impGlobalRef.GetAs( pwszVarName ) ;
			if ( pList == NULL )
			{
				pList = new ENumArray<DWORD> ;
				m_csxiInitFunc.m_impGlobalRef.SetAs( pwszVarName, pList ) ;
			}
		}
	}
	else
	{
		if ( fSharedDef )
		{
			pList = m_pcsxiDst->m_impNakedSharedRef.GetAs( pwszVarName ) ;
			if ( pList == NULL )
			{
				pList = new ENumArray<DWORD> ;
				m_pcsxiDst->m_impNakedSharedRef.SetAs( pwszVarName, pList ) ;
			}
		}
		else if ( fConstDef )
		{
			pList = m_pcsxiDst->m_impNakedConstRef.GetAs( pwszVarName ) ;
			if ( pList == NULL )
			{
				pList = new ENumArray<DWORD> ;
				m_pcsxiDst->m_impNakedConstRef.SetAs( pwszVarName, pList ) ;
			}
		}
		else
		{
			pList = m_pcsxiDst->m_impNakedGlobalRef.GetAs( pwszVarName ) ;
			if ( pList == NULL )
			{
				pList = new ENumArray<DWORD> ;
				m_pcsxiDst->m_impNakedGlobalRef.SetAs( pwszVarName, pList ) ;
			}
		}
	}
	return	pList ;
}

// 変数型宣言
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::ExternVariableType
	( const wchar_t * pwszVarName,
		ECSTypeInfo& typeVar, bool fNakedDef )
{
	ECSTypeInfo *	pVarType = NULL ;
	if ( !fNakedDef )
	{
		pVarType = m_wstaExternVarType.GetAs( pwszVarName ) ;
	}
	else
	{
		pVarType = m_wstaExternNakedVarType.GetAs( pwszVarName ) ;
	}
	if ( pVarType != NULL )
	{
		if ( *pVarType != typeVar )
		{
			m_strErrMsg =
				EString( pwszVarName )
					+ " が二重に定義されています。" ;
			return	ESLErrorMsg( m_strErrMsg ) ;
		}
	}
	else
	{
		if ( !fNakedDef )
		{
			m_wstaExternVarType.SetAs
				( pwszVarName, new ECSTypeInfo( typeVar ) ) ;
		}
		else
		{
			typeVar.m_dwFlags |= ECSTypeInfo::flagNakedBuffer ;
			m_wstaExternNakedVarType.SetAs
				( pwszVarName, new ECSTypeInfo( typeVar ) ) ;
		}
	}
	return	eslErrSuccess ;
}

// ExternDef 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileExternDef( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwExternDef ) )
	{
		return	eslErrSuccess ;
	}
	while ( !cssLine.DisregardSpace() )
	{
		int		nTokenType ;
		ECSWideString	wstrToken = cssLine.GetAToken( &nTokenType ) ;
		if ( nTokenType != 0 )
		{
			return	ESLErrorMsg( "変数名に不正な文字が使われています。" ) ;
		}
		ESLError	err = VerifyUserSymbol( wstrToken ) ;
		if ( err )
		{
			return	err ;
		}
		if ( m_staExternName.FindIndex( wstrToken ) < 0 )
		{
			m_staExternName.Add( wstrToken ) ;
		}
		if ( cssLine.DisregardSpace() )
		{
			break ;
		}
		if ( cssLine.HasToComeChar( L":" ) == L':' )
		{
			ECSTypeInfo	typeVar ;
			err = ParseTypeDescription( typeVar, cssLine ) ;
			if ( err )
			{
				return	err ;
			}
			ECSTypeInfo *
				pVarType = m_wstaExternVarType.GetAs( wstrToken ) ;
			if ( pVarType != NULL )
			{
				if ( *pVarType == typeVar )
				{
					m_strErrMsg =
						EString( wstrToken )
							+ " が二重に定義されています。" ;
					return	ESLErrorMsg( m_strErrMsg ) ;
				}
			}
			else
			{
				m_wstaExternVarType.SetAs
					( wstrToken, new ECSTypeInfo( typeVar ) ) ;
			}
		}
		if ( cssLine.GetCharacter() != L',' )
		{
			return	ESLErrorMsg( "変数名が , 記号で区切られていません。" ) ;
		}
	}
	return	eslErrSuccess ;
}

// TypeDef 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileTypeDef( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwTypeDef ) )
	{
		return	eslErrSuccess ;
	}
	//
	// 定義名取得
	//
	int		nTokenType ;
	ECSWideString	wstrTypeName = cssLine.GetAToken( &nTokenType ) ;
	if ( nTokenType != 0 )
	{
		return	ESLErrorMsg( "型名に不正な文字が使われています。" ) ;
	}
	EWideString	wstrSpaceName = GetCurrentSpaceName() ;
	if ( !wstrSpaceName.IsEmpty() )
	{
		wstrTypeName = wstrSpaceName + L"::" + wstrTypeName ;
	}
	if ( (m_staTypeName.FindIndex( wstrTypeName ) >= 0)
		|| (m_wstaTypeDef.GetAs( wstrTypeName ) != NULL)
		|| (GetClassInfoAs( wstrTypeName ) != NULL) )
	{
		m_strErrMsg =
			"\'" + EString(wstrTypeName) + "\' は既に定義されています。" ;
		return	ESLErrorMsg( m_strErrMsg ) ;
	}
	if ( !cssLine.HasToComeToken( L":=" ) )
	{
		return	ESLErrorMsg( "TypeDef で := 記号が見つかりません。" ) ;
	}
	//
	// 定義型取得
	//
	ECSTypeInfo *	pTypeInfo = new ECSTypeInfo ;
	ESLError	err = ParseTypeDescription( *pTypeInfo, cssLine ) ;
	if ( err )
	{
		delete	pTypeInfo ;
		return	err ;
	}
	m_wstaTypeDef.SetAs( wstrTypeName, pTypeInfo ) ;
	m_staTypeName.Add( wstrTypeName ) ;
	//
	return	eslErrSuccess ;
}

// Variable 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileVariable( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwVariable ) )
	{
		return	eslErrSuccess ;
	}
	//
	// static | shared | naked 修飾指定
	//////////////////////////////////////////////////////////////////////////
	bool			fStatic = false ;
	bool			fShared = false ;
	bool			fNaked = false ;
	bool			fObjected = false ;
	int				nIndex = cssLine.GetIndex() ;
	ECSWideString	wstrToken = cssLine.GetAToken( ) ;
	for ( ; ; )
	{
		if ( CompareReservedWord( L"static", wstrToken ) == 0 )
		{
			fStatic = true ;
		}
		else if ( CompareReservedWord( L"shared", wstrToken ) == 0 )
		{
			fShared = true ;
		}
		else if ( CompareReservedWord( L"naked", wstrToken ) == 0 )
		{
			fNaked = true ;
		}
		else if ( CompareReservedWord( L"objected", wstrToken ) == 0 )
		{
			fObjected = true ;
		}
		else
		{
			cssLine.MoveIndex( nIndex ) ;
			break ;
		}
		nIndex = cssLine.GetIndex() ;
		wstrToken = cssLine.GetAToken() ;
	}
	if ( !fObjected && (m_dwModeFlags & flagDefaultNakedAll) )
	{
		fNaked = true ;
	}
	//
	// 記憶クラス確定
	//////////////////////////////////////////////////////////////////////////
	CSObjectMode	csomClass ;
	if ( fShared )
	{
		csomClass = csomGlobal ;
		fStatic = (m_rwCtrlType != rwInvalid) ;
	}
	else if ( (m_rwCtrlType == rwInvalid) || fStatic )
	{
		csomClass = csomGlobal ;
	}
	else if ( (m_rwCtrlType == rwStructure)
				|| (m_rwCtrlType == rwClass)
				|| (m_rwCtrlType == rwNamespace)
				|| (m_rwCtrlType == rwUnion) )
	{
		if ( m_rwCtrlType == rwNamespace )
		{
			fStatic = true ;
			csomClass = csomGlobal ;
		}
		else
		{
			csomClass = csomThis ;
			fNaked = false ;
		}
	}
	else if ( m_rwCtrlType != rwData )
	{
		if ( fNaked && !m_modeNakedCode )
		{
			OutputWarning
				( "ローカル変数の宣言に naked が指定されています",
										m_strFilePath, m_nLineNum ) ;
		}
		csomClass = csomStack ;
		fNaked = m_modeNakedCode ;
	}
	else
	{
		return	ESLErrorMsg
			( "Variable 文が不正な制御ブロックの中で指定されました。" ) ;
	}
	//
	// 型名取得
	//////////////////////////////////////////////////////////////////////////
	ECSTypeInfo	typeVarBase ;
	ESLError	err ;
	err = ParseTypeDescription( typeVarBase, cssLine ) ;
	if ( err )
	{
		return	err ;
	}
	if ( typeVarBase.IsVoid() )
	{
		return	ESLErrorMsg( "void 変数を定義しようとしています。" ) ;
	}
	if ( typeVarBase.IsPureType() )
	{
		const ECSClassInfo *	pClassInf = typeVarBase.GetClassInfo() ;
		if ( pClassInf != NULL )
		{
			if ( pClassInf->GetAttribute() & ECSTypeInfo::flagAbstract )
			{
				return	ESLErrorMsg( "抽象クラスを生成しようとしています。" ) ;
			}
		}
	}
	while ( !cssLine.DisregardSpace() )
	{
		//
		// 変数名取得
		//////////////////////////////////////////////////////////////////////
		int				nTokenType ;
		ECSWideString	wstrVarName = cssLine.GetAToken( &nTokenType ) ;
		if ( nTokenType != 0 )
		{
			return	ESLErrorMsg( "変数名に不正な文字が使われています。" ) ;
		}
		ESLError	err = VerifyUserSymbol( wstrVarName ) ;
		if ( err )
		{
			return	err ;
		}
		SYMBOL_NAMESPACE	snsSymbol = wstrVarName ;
		err = ParseFullNameSymbol( snsSymbol, cssLine, false, false, true ) ;
		if ( err )
		{
			return	err ;
		}
		wstrVarName = snsSymbol.wstrFullName ;
		//
		if ( !snsSymbol.wstrNamespace.IsEmpty() )
		{
			const ECSClassInfo *
				pClassInf = GetClassInfoAs( snsSymbol.wstrNamespace ) ;
			if ( pClassInf == NULL )
			{
				m_strErrMsg =
					EString( snsSymbol.wstrNamespace )
							+ " は無効な名前空間の指定です" ;
				return	ESLErrorMsg( m_strErrMsg ) ;
			}
			ECSTypeInfo *	pObjectType =
					m_wstaExternVarType.GetAs( wstrVarName ) ;
			ECSTypeInfo *	pNakedType =
					m_wstaExternNakedVarType.GetAs( wstrVarName ) ;
			if ( !fObjected && !fNaked )
			{
				fNaked = (pNakedType != NULL) ;
			}
			if ( fNaked )
			{
				if ( pNakedType == NULL )
				{
					m_strErrMsg =
						EString( wstrVarName )
								+ " は未定義のメンバ変数です" ;
					return	ESLErrorMsg( m_strErrMsg ) ;
				}
			}
			else
			{
				if ( pObjectType == NULL )
				{
					m_strErrMsg =
						EString( wstrVarName )
								+ " は未定義のメンバ変数です" ;
					return	ESLErrorMsg( m_strErrMsg ) ;
				}
			}
		}
		//
		// 配列指定解釈
		//
		ECSTypeInfo	typeVar = typeVarBase ;
		err = ParseTypeArrayDecoration( typeVar, cssLine ) ;
		if ( err )
		{
			return	err ;
		}
		if ( fNaked )
		{
			typeVar.m_dwFlags |= ECSTypeInfo::flagNakedBuffer ;
		}
		//
		// 初期値構文の有無
		//
		ECSObject *		pObjInit = NULL ;
		bool			fInitExpr = false ;
		wstrToken = cssLine.GetAToken( ) ;
		if ( (wstrToken == L":=") || (wstrToken == L"=") )
		{
			if ( (wstrToken == L"=")
				&& !(m_dwModeFlags & flagNoWarningEquMove) )
			{
				err = OutputWarning1
					( "変数の初期値の指定に = が用いられています。"
						":= を使ってください（推奨）。",
						m_strFilePath, m_nLineNum ) ;
				if ( err )
				{
					return	err ;
				}
			}
			//
			// 定数式初期値
			//////////////////////////////////////////////////////////////////
			ECSObject *	pValue ;
			int	iInitExprIndex = cssLine.GetIndex() ;
			err = CalculateExpression( pValue, cssLine, 0, L"," ) ;
			if ( err )
			{
				if ( /*(csomClass == csomGlobal) ||*/ (csomClass == csomThis) )
				{
					return	err ;
				}
				else
				{
					cssLine.MoveIndex( iInitExprIndex ) ;
					fInitExpr = true ;
				}
			}
			else
			{
				if ( !fNaked )
				{
					err = NormalizeObjectInitialValue
								( pObjInit, typeVar, pValue ) ;
				}
				else
				{
					err = NormalizeNakedVariableInitialValue
								( pObjInit, typeVar, pValue, false ) ;
				}
				if ( err )
				{
					if ( (csomClass == csomGlobal) || (csomClass == csomThis) )
					{
						return	err ;
					}
					delete	pObjInit ;
					pObjInit = NULL ;
				}
				if ( pObjInit != NULL )
				{
					wstrToken = cssLine.GetAToken( ) ;
				}
				else
				{
					cssLine.MoveIndex( iInitExprIndex ) ;
					fInitExpr = true ;
				}
			}
		}
		if ( csomClass == csomGlobal )
		{
			//
			// 大域変数定義
			//////////////////////////////////////////////////////////////////
			bool	fExternDef = false ;
			bool	fConstant = ((typeVar.m_dwFlags & ECSTypeInfo::flagConstant) != 0) ;
			EWideString	wstrExternName = wstrVarName ;
			if ( fStatic )
			{
				if ( !fShared && !fNaked && (m_rwCtrlType == rwInvalid) )
				{
					OutputWarning
						( "グローバルな変数に static 修飾されています",
												m_strFilePath, m_nLineNum ) ;
				}
				EWideString	wstrSpaceName = GetCurrentSpaceName() ;
				const ECSClassInfo *
						pClassInf = GetClassInfoAs( wstrSpaceName ) ;
				if ( !wstrSpaceName.IsEmpty() )
				{
					wstrSpaceName += L"::" ;
				}
				wstrVarName = wstrSpaceName + wstrVarName ;
				//
				if ( wstrSpaceName.IsEmpty() )
				{
					wstrExternName = L"<" ;
					wstrExternName += EWideString( m_strFilePath.GetFileNamePart() ) ;
					wstrExternName += L">::" ;
					wstrExternName += wstrVarName ;
				}
				else
				{
					wstrExternName = wstrVarName ;
				}
				if ( pClassInf != NULL )
				{
					bool	fNamespace = pClassInf->IsNamespace() ;
					if ( !fNamespace && (fInitExpr || (pObjInit != NULL)) )
					{
						return	ESLErrorMsg
							( "static メンバの宣言に初期値が指定されています" ) ;
					}
					if ( !fNaked && !fObjected )
					{
						if ( fNamespace || pClassInf->IsNakedMemoryClass() )
						{
							fNaked = true ;
						}
						else
						{
							fObjected = true ;
						}
					}
					DeclareVariableReferenceDef
							( wstrExternName, fNaked, fShared, fConstant ) ;
					err = ExternVariableType
							( wstrExternName, typeVar, fNaked ) ;
					if ( err )
					{
						return	err ;
					}
					fExternDef = !fNamespace ;
				}
				else if ( !fNaked && !fObjected )
				{
					if ( m_modeNakedCode )
					{
						fNaked = true ;
					}
					else
					{
						fObjected = true ;
					}
				}
			}
			if ( !fExternDef )
			{
				if ( !fNaked || fObjected )
				{
					if ( fShared )
					{
						err = DeclareSharedGlobalObjectVariable
									( wstrExternName, typeVar, pObjInit ) ;
					}
					else
					{
						err = DeclareGlobalObjectVariable
									( wstrExternName, typeVar, pObjInit ) ;
					}
				}
				else
				{
					err = DeclareGlobalNakedVariable
							( wstrExternName, typeVar, pObjInit, fShared, fConstant ) ;
				}
				if ( fInitExpr )
				{
					if ( fNaked )
					{
						NakedModeSaver	saver( *this ) ;
						m_pcsxi = &m_csxiNakedPrologue ;
						m_modeNakedCode = true ;
						//
						EControlNest *	pNest = NULL ;
						if ( wstrExternName.Find( L"::" ) >= 0 )
						{
							SYMBOL_NAMESPACE	snsExternName ;
							snsExternName.ParseSymbol( wstrExternName ) ;
							//
							ECSClassInfo *	pClassInf =
								GetClassInfoAs( snsExternName.wstrNamespace ) ;
							if ( pClassInf != NULL )
							{
								pNest = new EControlNest( m_dwImplementFlags ) ;
								pNest->m_rwType = rwVariable ;
								//
								AddNecessaryUsingNamespacePath
										( pNest, pClassInf->GetGlobalName() ) ;
								AddNecessaryUsingParentClassList( pNest, pClassInf ) ;
								m_nestCtrl.Add( pNest ) ;
							}
						}
						//
						int	regPtr = AllocateExpressionRegister() ;
						if ( fShared )
						{
							m_pcsxi->WriteSakuraLoadInt64_SharedVarAddr
												( regPtr, wstrExternName ) ;
						}
						else if ( fConstant )
						{
							m_pcsxi->WriteSakuraLoadInt64_ConstVarAddr
												( regPtr, wstrExternName ) ;
						}
						else
						{
							m_pcsxi->WriteSakuraLoadInt64_GlobalVarAddr
												( regPtr, wstrExternName ) ;
						}
						ECSTypeInfo	typeTempVar = typeVar ;
						typeTempVar.SetAddressingInfo( regPtr, 0 ) ;
						//
						err = CompileInitValuesForNakedVariable
											( typeTempVar, cssLine ) ;
						//
						FreeExpressionRegister() ;
						//
						if ( pNest != NULL )
						{
							ESLVerify( pNest == m_nestCtrl.Pop() ) ;
							LeaveControlNest( pNest ) ;
						}
						//
						if ( err )
						{
							return	err ;
						}
						wstrToken = cssLine.GetAToken( ) ;
					}
					else
					{
						return	ESLErrorMsg
							( "objected モードで初期値式が指定されています" ) ;
					}
				}
			}
			delete	pObjInit ;
			if ( err )
			{
				return	err ;
			}
			if ( wstrVarName != wstrExternName )
			{
				m_wstaVarGlobalName.SetAs
					( wstrVarName, new EWideString(wstrExternName) ) ;
			}
		}
		else if ( csomClass == csomThis )
		{
			//
			// 構造体・クラスメンバ変数定義
			//////////////////////////////////////////////////////////////////
			EControlNest *	pNest = m_nestCtrl.GetLastAt( 0 ) ;
			ESLAssert( pNest != NULL ) ;
			ESLAssert( pNest->m_rwType == m_rwCtrlType ) ;
			//
			ECSClassInfo *
				pClassInf = GetClassInfoAs( GetCurrentSpaceName() ) ;
			if ( pClassInf != NULL )
			{
				if ( fNaked && !pClassInf->IsNakedMemoryClass() )
				{
					OutputWarning
						( "メンバ変数の宣言に naked が指定されています",
												m_strFilePath, m_nLineNum ) ;
				}
				if ( fObjected && pClassInf->IsNakedMemoryClass() )
				{
					OutputWarning
						( "メンバ変数の宣言に objected が指定されています",
												m_strFilePath, m_nLineNum ) ;
				}
				int	iVarIndex = pClassInf->GetVariableIndex( wstrVarName ) ;
				if ( iVarIndex >= 0 )
				{
					m_strErrMsg = EString(wstrVarName)
								+ " は既に定義されているメンバ変数です。" ;
					return	ESLErrorMsg( m_strErrMsg ) ;
				}
				if ( pObjInit != NULL )
				{
					pClassInf->AddVariable
						( wstrVarName,
							new ECSTypeInfo
								( pObjInit,
									(typeVar.m_dwFlags
										| pNest->m_dwProtectedScope
										| ECSTypeInfo::flagDeterministic ) ) ) ;
				}
				else
				{
					ECSTypeInfo *	pNewVar = new ECSTypeInfo( typeVar ) ;
					pNewVar->m_dwFlags |= pNest->m_dwProtectedScope ;
					pClassInf->AddVariable( wstrVarName, pNewVar ) ;
				}
			}
			else
			{
				err = CompileNewVariable
							( csomThis, wstrVarName, typeVar.m_pValue ) ;
				if ( err )
				{
					return	err ;
				}
				if ( pObjInit != NULL )
				{
					CompileCodeLoadRefVariable( csomThis, wstrVarName ) ;
					//
					ECSTypeInfo	typeTemp ;
					err = CompileImmediateObject( typeTemp, pObjInit ) ;
					if ( err )
					{
						return	err ;
					}
					CompileCodeStore( csotNop ) ;
					CompileCodeFreeStack() ;
					delete	pObjInit ;
				}
			}
		}
		else if ( csomClass == csomStack )
		{
			//
			// ローカル変数定義
			//////////////////////////////////////////////////////////////////
			EControlNest *	pNest = m_nestCtrl.GetLastAt( 0 ) ;
			ESLAssert( pNest != NULL ) ;
			ESLAssert( pNest->m_rwType == m_rwCtrlType ) ;
			if ( m_rwCtrlType == rwSwitch )
			{
				return	ESLErrorMsg
					( "Switch ブロック内で変数を生成しようとしています" ) ;
			}
			if ( pNest->m_lstLocalName.FindIndex( wstrVarName ) >= 0 )
			{
				m_strErrMsg = "局所変数 \'"
					+ EString(wstrVarName) + "\' は既に定義されています。" ;
				return	ESLErrorMsg( m_strErrMsg ) ;
			}
			if ( pNest->m_fAutoEnter && !m_modeNakedCode )
			{
				if ( pNest->m_lstLocalName.GetSize() == 0 )
				{
					DWORD	dwDummy = 0 ;
					m_pcsxi->WriteInstructionCode( csicEnter ) ;
					m_pcsxi->WriteConstantString( ECSWideString(L"") ) ;
					m_pcsxi->WriteCodeData( &dwDummy, sizeof(dwDummy) ) ;
				}
			}
			ECSObject *	pType = typeVar.DuplicateType() ;
			if ( (pObjInit != NULL) && !m_modeNakedCode )
			{
				err = pType->Move
					( m_ctxExpr, ECSTypeInfo::DuplicateType( pObjInit ) ) ;
				if ( err )
				{
					return	err ;
				}
			}
			//
			// 変数定義
			//
			ECSTypeInfo *	pVarType =
				new ECSTypeInfo
					( pType, typeVar.m_dwFlags
									| ECSTypeInfo::flagDeterministic ) ;
			//
			AddVariableToControlNest( pNest, wstrVarName, pVarType ) ;
			//
			if ( pNest->m_fGotoOccured )
			{
				return	ESLErrorMsg
					( "局所名前空間の中で Goto 文の後に"
						"局所変数が定義されています。" ) ;
			}
			//
			// 変数生成・初期化命令出力
			//
			if ( !m_modeNakedCode )
			{
				if ( typeVar.IsRuntimeIntegerType() )
				{
					ECSInteger	typePtrNaked ;
					err = CompileNewVariable
								( csomStack, wstrVarName, &typePtrNaked ) ;
				}
				else
				{
					err = CompileNewVariable
								( csomStack, wstrVarName, typeVar.m_pValue ) ;
				}
				if ( err )
				{
					return	err ;
				}
			}
			BeginSakura2Optimize() ;
			//
			bool	fNoGarbage = false ;
			if ( pObjInit != NULL )
			{
				//
				// 定数初期値設定命令出力
				//
				if ( m_modeNakedCode )
				{
					err = CompileCodeNakedLocalInitialConstValue
											( *pVarType, pObjInit ) ;
				}
				else
				{
					ECSTypeInfo	typeInit ;
					err = CompileImmediateObject( typeInit, pObjInit ) ;
					if ( err )
					{
						return	err ;
					}
					ECSTypeInfo	typeTemp ;
					err = CompileTypeMoveOperate
						( typeTemp, typeVar, typeInit, csotNop, true ) ;
				}
				delete	pObjInit ;
				if ( err )
				{
					return	err ;
				}
			}
			else if ( fInitExpr )
			{
				//
				// 初期値式命令出力
				//
				bool	fInitialized = false ;
				if ( m_modeNakedCode )
				{
					ECSTypeInfo	typeNakedVarLoaded ;
					err = CompilePrepareNakedLocalVariable
									( typeNakedVarLoaded, pVarType ) ;
					if ( err )
					{
						return	err ;
					}
					int	iSaveIndex = cssLine.GetIndex() ;
					if ( cssLine.HasToComeChar( L"{" ) == L'{' )
					{
						cssLine.MoveIndex( iSaveIndex ) ;
						//
						ECSTypeInfo	typeTempVar = *pVarType ;
						typeTempVar.SetAddressingInfo
							( typeNakedVarLoaded.GetLoadedRegister(), 0 ) ;
						//
						err = CompileInitValuesForNakedVariable
											( typeTempVar, cssLine ) ;
						if ( err )
						{
							return	err ;
						}
						fInitialized = true ;
					}
					FreeExpressionRegister( typeNakedVarLoaded ) ;
					fNoGarbage = true ;
				}
				if ( !fInitialized )
				{
					ECSTypeInfo	typeExpr ;
					err = CompileExpression( typeExpr, cssLine, 0, 0, L"," ) ;
					if ( err )
					{
						return	err ;
					}
					if ( typeExpr.IsVoid() )
					{
						return	ESLErrorMsg
							( "void データを初期値をして指定しています。" ) ;
					}
					ECSTypeInfo	typeTemp ;
					err = CompileTypeMoveOperate
						( typeTemp, *pVarType, typeExpr, csotNop, true ) ;
					if ( err )
					{
						return	err ;
					}
				}
				wstrToken = cssLine.GetAToken( ) ;
			}
			else if ( (wstrToken == L"(") && typeVar.IsPureType() )
			{
				//
				// 構築関数呼び出し
				//
				const ECSClassInfo *
					pClassInf = GetNakedTypeClassInfo( typeVar ) ;
				if ( pClassInf == NULL )
				{
					return	ESLErrorMsg
						( "クラス情報の無いオブジェクトの"
							"構築関数を呼び出そうとしています。" ) ;
				}
				if ( m_modeNakedCode )
				{
					int	regThis = AllocateExpressionRegister() ;
					ESLAssert( pVarType->IsAddressingInfo() ) ;
					m_pcsxi->WriteSakuraAddRegRegImm32
						( regThis, pVarType->m_regBase,
										pVarType->m_addrOffset ) ;
					CompileCodeNakedClassInitialize( pClassInf ) ;
				}
				else
				{
					CompileLoadStackObject( 0 ) ;
				}
				EObjArray<ECSTypeInfo>	lstArgType ;
				err = CompileArgument( lstArgType, NULL, cssLine ) ;
				if ( err )
				{
					return	err ;
				}
				err = CompileCallObjectConstructor( *pClassInf, lstArgType ) ;
				if ( err )
				{
					return	err ;
				}
				//
				wstrToken = cssLine.GetAToken( ) ;
			}
			else if ( m_modeNakedCode )
			{
				//
				// naked モード初期化処理
				//
				err = CompileCodeNakedLocalInitialDefault( *pVarType ) ;
				if ( err )
				{
					return	err ;
				}
			}
			else if ( IsDefaultConstructor( typeVar ) )
			{
				//
				// デフォルトコンストラクタ呼び出し
				//
				err = CompileCallDefaultConstructor( *pVarType, true ) ;
				if ( err )
				{
					return	err ;
				}
			}
			if ( m_modeNakedCode && !fNoGarbage
				/*&& IsNakedThrowableNest()
				&& (pVarType != NULL)
				&& (pVarType->m_pValue != NULL)
				&& ECSClassInfo::IsNeedsNakedVariableDestruction
										( pVarType->m_pValue, true )*/ )
			{
				//
				// ガベージ・リスト登録
				//
				err = CompileCodeNakedAddGarbageList( pVarType ) ;
				if ( err )
				{
					return	err ;
				}
			}
			FinishSakura2Optimize() ;
		}
		else
		{
			return	ESLErrorMsg
				( "Variable 文が不正な制御ブロックの中で指定されました。" ) ;
		}
		if ( wstrToken.IsEmpty() )
		{
			break ;
		}
		if ( wstrToken != L"," )
		{
			return	ESLErrorMsg( "変数が , 記号で区切られていません。" ) ;
		}
	}
	return	eslErrSuccess ;
}

// object モード変数初期値正規化
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::NormalizeObjectInitialValue
	( ECSObject*& pObjInit,
		const ECSTypeInfo & typeVar, ECSObject * pValue )
{
	ESLError err ;
	pObjInit = NULL ;
	if ( pValue->m_vtType == typeVar.m_pValue->m_vtType )
	{
		ECSTypeInfo::TypeMatchResult	matchResult =
			typeVar.IsMatchType
				( ECSTypeInfo
					( pValue->Duplicate(),
						ECSTypeInfo::flagDeterministic ) ) ;
		if ( matchResult > ECSTypeInfo::typeNatualMatch )
		{
			err = OutputWarning0( "初期値が変数の型に適合しません。" ) ;
			if ( err )
			{
				return	err ;
			}
		}
		pObjInit = typeVar.DuplicateType() ;
		//
		err = pObjInit->Move( m_ctxExpr, pValue ) ;
		if ( err )
		{
			return	err ;
		}
	}
	else
	{
		if ( (m_rwCtrlType == rwInvalid)
			|| (m_rwCtrlType == rwStructure)
			|| (m_rwCtrlType == rwClass) )
		{
			delete	pValue ;
			return	ESLErrorMsg
				( "参照型変数に不正な初期値が指定されています。" ) ;
		}
		ECSObject *	pNakedVarType = typeVar.GetNakedType() ;
		if ( (pNakedVarType == NULL)
			|| (pValue->m_vtType != pNakedVarType->m_vtType) )
		{
			delete	pValue ;
			return	ESLErrorMsg
				( "初期値が変数の型と一致しません。" ) ;
		}
		delete	pValue ;
	}
	return	eslErrSuccess ;
}

// naked モード変数初期値・変数配列長正規化
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::NormalizeNakedVariableInitialValue
	( ECSObject*& pObjInit,
		ECSTypeInfo & typeVar, ECSObject * pValue, bool fVarTypeFixed )
{
	pObjInit = NULL ;
	if ( pValue == NULL )
	{
		return	eslErrSuccess ;
	}
	if ( typeVar.IsTypeReference() )
	{
		delete	pValue ;
		return	ESLErrorMsg
			( "naked モードで参照型の初期値に即値が指定されています" ) ;
	}
	if ( pValue->m_vtType == csvtString )
	{
		if ( typeVar.IsTypePointer() )
		{
			//
			// String -> const uint16 *
			//
			ECSObject *	pPtrType = typeVar.GetNakedPointerType() ;
			if ( (pPtrType != NULL)
				&& (pPtrType->m_vtType == csvtInteger)
				&& (((ECSInteger*)pPtrType)->GetValueMask()
									== ECSInteger::m_maskUint16) )
			{
				if ( typeVar.m_dwFlags & ECSTypeInfo::flagConstant )
				{
					OutputWarning
						( "文字列の初期値が const uint16 * 型で"
								"ない変数に指定されています",
									m_strFilePath, m_nLineNum ) ;
				}
				pObjInit = pValue ;
				return	eslErrSuccess ;
			}
			else
			{
				delete	pValue ;
				return	ESLErrorMsg
					( "文字列の初期値が const uint16 * 型で"
									"ない変数に指定されています" ) ;
			}
		}
		else if ( typeVar.IsTypeArray() )
		{
			//
			// String -> uint16[]
			//
			ECSObject *	pElementType = typeVar.GetArrayElementType() ;
			if ( (pElementType != NULL)
				&& (pElementType->m_vtType == csvtInteger)
				&& (((ECSInteger*)pElementType)->GetValueMask()
									== ECSInteger::m_maskUint16) )
			{
				ESLAssert( typeVar.m_pValue != NULL ) ;
				ESLAssert( typeVar.m_pValue->m_vtType == csvtArray ) ;
				ECSString *	pString = (ECSString*) pValue ;
				ECSArray *	pArray = (ECSArray*) typeVar.m_pValue ;
				if ( !pArray->IsBounds() )
				{
					if ( fVarTypeFixed )
					{
						delete	pValue ;
						return	ESLErrorMsg
							( "naked モードで不正な可変長配列は不正な変数宣言です" ) ;
					}
					pArray->SetBounds
						( pString->m_varStr.GetLength() + 1 ) ;
				}
				else if ( pArray->GetBounds() < pString->m_varStr.GetLength() )
				{
					if ( fVarTypeFixed )
					{
						delete	pValue ;
						return	ESLErrorMsg
							( "配列長以上の文字列が初期値に指定されています" ) ;
					}
					OutputWarning
						( "配列長以上の文字列が初期値に指定されています",
											m_strFilePath, m_nLineNum ) ;
					pArray->SetBounds
						( pString->m_varStr.GetLength() + 1 ) ;
				}
				ECSArray *	pInitArray = new ECSArray ;
				for ( int i = 0; i < (int) pString->m_varStr.GetLength(); i ++ )
				{
					pInitArray->m_varArray.SetAt
						( i, new ECSInteger
								( pString->m_varStr.GetAt( i ),
										ECSInteger::m_maskUint16 ) ) ;
				}
				delete	pValue ;
				pObjInit = pInitArray ;
				return	eslErrSuccess ;
			}
			else
			{
				delete	pValue ;
				return	ESLErrorMsg
					( "文字列の初期値が uint16[] 型で"
									"ない変数に指定されています" ) ;
			}
		}
		delete	pValue ;
		return	ESLErrorMsg
			( "文字列の初期値が const uint16 * でも"
				" uint16[] でもない変数に指定されています" ) ;
	}
	if ( pValue->m_vtType == csvtArray )
	{
		ECSArray *	pInitArray = (ECSArray*) pValue ;
		if ( typeVar.IsTypeArray() )
		{
			//
			// Array -> type[]
			//
			ECSArray *	pVarArray = (ECSArray*) typeVar.m_pValue ;
			if ( !pVarArray->IsBounds() )
			{
				if ( fVarTypeFixed )
				{
					delete	pValue ;
					return	ESLErrorMsg
						( "naked モードで不正な可変長配列は不正な変数宣言です" ) ;
				}
				pVarArray->SetBounds( pInitArray->m_varArray.GetSize() ) ;
			}
			else if ( pVarArray->GetBounds()
							< pInitArray->m_varArray.GetSize() )
			{
				if ( fVarTypeFixed )
				{
					delete	pValue ;
					return	ESLErrorMsg
						( "初期値配列が境界値を超えているため"
									"配列長定義を自動延長します" ) ;
				}
				OutputWarning
					( "初期値配列が境界値を超えているため"
							"配列長定義を自動延長します",
									m_strFilePath, m_nLineNum ) ;
				pVarArray->SetBounds( pInitArray->m_varArray.GetSize() ) ;
			}
			//
			// 配列要素を正規化
			//
			ECSTypeInfo	typeVarElement
				( ECSTypeInfo::DuplicateType
						( pVarArray->m_pDefObj ), typeVar.m_dwFlags ) ;
			//
			ECSArray *	pArrayInit = new ECSArray ;
			int	nArraySize = pInitArray->m_varArray.GetSize() ;
			for ( int i = 0; i < nArraySize; i ++ )
			{
				ECSObject *	pInitElement = NULL ;
				ESLError	err =
					NormalizeNakedVariableInitialValue
						( pInitElement,
							typeVarElement,
							ECSTypeInfo::DuplicateType
								( pInitArray->m_varArray.GetAt( i ) ),
							fVarTypeFixed ) ;
				if ( err )
				{
					delete	pArrayInit ;
					delete	pValue ;
					return	err ;
				}
				pArrayInit->m_varArray.SetAt( i, pInitElement ) ;
			}
			pObjInit = pArrayInit ;
			//
			// 修正された要素型を反映
			//
			if ( !fVarTypeFixed )
			{
				pVarArray->SetDefaultElement
					( ECSTypeInfo::DuplicateType
								( typeVarElement.m_pValue ) ) ;
			}
			delete	pValue ;
			return	eslErrSuccess ;
		}
		const ECSClassInfo *
			pClassInf = typeVar.GetNakedMemoryClassInfo() ;
		if ( (pClassInf == NULL)
			|| !pClassInf->IsNakedMemoryClass() )
		{
			delete	pValue ;
			return	ESLErrorMsg
				( "配列でない変数の初期値に配列が指定されています" ) ;
		}
		//
		// Array -> struct
		//
		if ( pClassInf->IsNeedsNakedClassConstruction( false ) )
		{
			delete	pValue ;
			return	ESLErrorMsg
				( "コンストラクタの定義されたクラスに初期値が指定されています" ) ;
		}
		if ( pClassInf->IsNeedsNakedClassDestruction() )
		{
			delete	pValue ;
			return	ESLErrorMsg
				( "デストラクタの定義されたクラスに初期値が指定されています" ) ;
		}
		if ( pClassInf->GetVirtualFunctionCount() > 0 )
		{
			delete	pValue ;
			return	ESLErrorMsg
				( "仮想関数をメンバに持つクラスの初期値が指定されています" ) ;
		}
		int	nMemberCount = pClassInf->GetVariableCount() ;
		if ( (int) pInitArray->m_varArray.GetSize() > nMemberCount )
		{
			delete	pValue ;
			return	ESLErrorMsg( "クラスの初期値要素が多すぎます" ) ;
		}
		ECSArray *	pArrayInit = new ECSArray ;
		for ( int i = 0; i < nMemberCount; i ++ )
		{
			ECSTypeInfo *	pMemberType = pClassInf->GetVariableAt( i ) ;
			if ( pMemberType == NULL )
			{
				delete	pValue ;
				delete	pObjInit ;
				pObjInit = NULL ;
				return	ESLErrorMsg
					( "内部エラー：メンバ変数の型情報が見つかりません" ) ;
			}
			ECSTypeInfo	typeMember = *pMemberType ;
			ECSObject *	pMemberInit = NULL ;
			ESLError	err =
				NormalizeNakedVariableInitialValue
					( pMemberInit, typeMember,
						ECSTypeInfo::DuplicateType
							( pInitArray->m_varArray.GetAt( i ) ),
						fVarTypeFixed ) ;
			if ( err )
			{
				delete	pValue ;
				delete	pArrayInit ;
				return	err ;
			}
			pArrayInit->m_varArray.SetAt( i, pMemberInit ) ;
		}
		pObjInit = pArrayInit ;
		delete	pValue ;
		return	eslErrSuccess ;
	}
	if ( (pValue->m_vtType == typeVar.m_pValue->m_vtType)
		|| (typeVar.IsTypeInteger() && (pValue->m_vtType == csvtReal))
		|| (typeVar.IsTypeReal() && (pValue->m_vtType == csvtInteger)) )
	{
		ESLError	err ;
		ECSTypeInfo::TypeMatchResult	matchResult =
			typeVar.IsMatchType
				( ECSTypeInfo
					( pValue->Duplicate(),
						ECSTypeInfo::flagDeterministic ) ) ;
		if ( matchResult > ECSTypeInfo::typeNatualMatch )
		{
			err = OutputWarning0( "初期値が変数の型に適合しません。" ) ;
			if ( err )
			{
				return	err ;
			}
		}
		pObjInit = typeVar.DuplicateType() ;
		//
		err = pObjInit->Move( m_ctxExpr, pValue ) ;
		if ( err )
		{
			return	err ;
		}
		return	eslErrSuccess ;
	}
	else if ( typeVar.IsTypePointer() )
	{
		if ( pValue->m_vtType == csvtReference )
		{
			if ( ((ECSReference*)pValue)->m_pRef == NULL )
			{
				pObjInit = new ECSReference ;
				delete	pValue ;
				return	eslErrSuccess ;
			}
		}
		else if ( pValue->m_vtType == csvtInteger )
		{
			if ( ((ECSInteger*)pValue)->GetValue() == 0 )
			{
				pObjInit = new ECSInteger( 0 ) ;
				delete	pValue ;
				return	eslErrSuccess ;
			}
		}
	}
	const ECSClassInfo *
		pClassInf = GetNakedTypeClassInfo( typeVar ) ;
	if ( pClassInf != NULL )
	{
		if ( pClassInf->GetAttribute() & ECSTypeInfo::flagEnumerator )
		{
			if ( !pClassInf->IsMatchEnumeratorValue( pValue ) )
			{
				OutputWarning
					( EString(pClassInf->GetGlobalName())
						+ " 列挙型へ適合しない初期値です",
										m_strFilePath, m_nLineNum ) ;
			}
		}
		pObjInit = pValue ;
		return	eslErrSuccess ;
	}
	delete	pValue ;
	return	ESLErrorMsg
		( "初期値が naked 変数の型と一致しません。" ) ;
}

// object モードグローバル変数定義
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::DeclareGlobalObjectVariable
	( const EWideString & wstrVarName,
		const ECSTypeInfo & typeVar, ECSObject * pObjInit )
{
	ESLError	err ;
	if ( m_pcsxiDst->m_csgGlobalType.
			m_staObjName.FindIndex( wstrVarName ) >= 0 )
	{
		m_strErrMsg = "大域変数 \'"
			+ EString(wstrVarName) + "\' は既に定義されています。" ;
		return	ESLErrorMsg( m_strErrMsg ) ;
	}
	if ( pObjInit == NULL )
	{
		if ( typeVar.IsRuntimeIntegerType() )
		{
			pObjInit = new ECSInteger ;
		}
		else
		{
			pObjInit = typeVar.DuplicateType() ;
		}
	}
	m_pcsxiDst->m_csgGlobalType.AddVariable
		( wstrVarName, ECSTypeInfo::DuplicateType( pObjInit ) ) ;
	m_csxiInitFunc.m_impGlobalRef.Add
		( wstrVarName, new ENumArray<DWORD> ) ;
	//
	if ( IsDefaultConstructor( typeVar ) )
	{
		NakedModeSaver	saver( *this ) ;
		m_pcsxi = &m_csxiInitFunc ;
		m_modeNakedCode = ((m_dwModeFlags & flagDefaultNakedAll) != 0) ;
		//
		DWORD	dwRefAddr = CompileCodeGetCurrent() + 3 ;
		int		iVarIndex =
					IsGlobalVariableName( wstrVarName, dwRefAddr ) ;
		if ( iVarIndex >= 0 )
		{
			CompileCodeLoadRefVariable( csomGlobal, iVarIndex ) ;
			//
			err = CompileCallDefaultConstructor( typeVar, false ) ;
			if ( err )
			{
				return	err ;
			}
		}
	}
	//
	ECSTypeInfo *
		pVarType = m_wstaExternVarType.GetAs( wstrVarName ) ;
	if ( pVarType != NULL )
	{
		if ( *pVarType != typeVar )
		{
			m_strErrMsg = "大域変数 \'"
				+ EString(wstrVarName) + "\' の型が宣言と異なっています。" ;
			return	ESLErrorMsg( m_strErrMsg ) ;
		}
	}
	return	eslErrSuccess ;
}

// object モード shared グローバル変数定義
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::DeclareSharedGlobalObjectVariable
	( const EWideString & wstrVarName,
		const ECSTypeInfo & typeVar, ECSObject * pObjInit )
{
	ESLError	err ;
	if ( typeVar.IsVoid() )
	{
		return	ESLErrorMsg( "void 変数が定義されています。" ) ;
	}
	m_pcsxiDst->m_csgDataType.AddVariable
				( wstrVarName, typeVar.DuplicateType() ) ;
	m_csxiInitFunc.m_impDataRef.Add( wstrVarName, new ENumArray<DWORD> ) ;
	//
	if ( IsDefaultConstructor( typeVar ) )
	{
		NakedModeSaver	saver( *this ) ;
		m_pcsxi = &m_csxiInitFunc ;
		m_modeNakedCode = ((m_dwModeFlags & flagDefaultNakedAll) != 0) ;
		//
		DWORD	dwRefAddr = CompileCodeGetCurrent() + 3 ;
		int		iVarIndex =
					IsDataTableName( wstrVarName, dwRefAddr ) ;
		if ( iVarIndex >= 0 )
		{
			CompileCodeLoadRefVariable( csomData, iVarIndex ) ;
			//
			err = CompileCallDefaultConstructor( typeVar, false ) ;
			if ( err )
			{
				return	err ;
			}
		}
	}
	//
	ECSTypeInfo *
		pVarType = m_wstaExternVarType.GetAs( wstrVarName ) ;
	if ( pVarType != NULL )
	{
		if ( *pVarType != typeVar )
		{
			m_strErrMsg = "大域変数 \'"
				+ EString(wstrVarName) + "\' の型が宣言と異なっています。" ;
			return	ESLErrorMsg( m_strErrMsg ) ;
		}
	}
	if ( pObjInit != NULL )
	{
		NakedModeSaver	saver( *this ) ;
		m_pcsxi = &m_csxiInitFunc ;
		m_modeNakedCode = ((m_dwModeFlags & flagDefaultNakedAll) != 0) ;
		//
		DWORD	dwRefAddr = CompileCodeGetCurrent() + 3 ;
		int		iVarIndex =
					IsGlobalVariableName( wstrVarName, dwRefAddr ) ;
		if ( iVarIndex >= 0 )
		{
			CompileCodeLoadRefVariable( csomGlobal, iVarIndex ) ;
			//
			ECSTypeInfo	typeTemp ;
			ESLError	err = CompileImmediateObject( typeTemp, pObjInit ) ;
			if ( err )
			{
				return	err ;
			}
			CompileCodeStore( csotNop ) ;
			CompileCodeFreeStack() ;
		}
	}
	return	eslErrSuccess ;
}

// naked モードグローバル変数定義
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::DeclareGlobalNakedVariable
	( const EWideString & wstrVarName,
		const ECSTypeInfo & typeVar, ECSObject * pObjInit,
		bool fShared, bool fConstant )
{
	ESLError	err ;
	if ( typeVar.IsVoid() )
	{
		return	ESLErrorMsg( "void 変数が定義されています。" ) ;
	}
	fConstant |= ((typeVar.m_dwFlags & ECSTypeInfo::flagConstant) != 0) ;
	//
	// 変数アドレス確定
	//
	ECSBuffer*	pbufNakedImage =
		(fShared ? &m_pcsxiDst->m_bufNakedShared
			: (fConstant ? &m_pcsxiDst->m_bufNakedConst
							: &m_pcsxiDst->m_bufNakedGlobal)) ;
	ECSBuffer&	bufNakedImage = *pbufNakedImage ;
	DWORD	dwVarAddr = bufNakedImage.GetLength() ;
	if ( dwVarAddr & 0x07 )
	{
		dwVarAddr = (dwVarAddr + 0x07) & ~0x07 ;
		bufNakedImage.ResizeBuffer( dwVarAddr ) ;
	}
	//
	// シンボル登録
	//
	ECSExecutionImage::NAKED_SYMBOL_INFO *
		pSymbol = new ECSExecutionImage::NAKED_SYMBOL_INFO ;
	pSymbol->dwFlags = 0 ;
	pSymbol->nAddress = dwVarAddr ;
	if ( fShared )
	{
		if ( m_pcsxiDst->m_impNakedSharedRef.GetAs( wstrVarName ) == NULL )
		{
			m_pcsxiDst->m_impNakedSharedRef.SetAs
						( wstrVarName, new ENumArray<DWORD> ) ;
		}
		pSymbol->nAddress |= (INT64) ECSExecutionImage::roasNakedShared << 56 ;
	}
	else if ( fConstant )
	{
		if ( m_pcsxiDst->m_impNakedConstRef.GetAs( wstrVarName ) == NULL )
		{
			m_pcsxiDst->m_impNakedConstRef.SetAs
						( wstrVarName, new ENumArray<DWORD> ) ;
		}
		pSymbol->nAddress |= (INT64) ECSExecutionImage::roasNakedConst << 56 ;
	}
	else
	{
		if ( m_pcsxiDst->m_impNakedGlobalRef.GetAs( wstrVarName ) == NULL )
		{
			m_pcsxiDst->m_impNakedGlobalRef.SetAs
						( wstrVarName, new ENumArray<DWORD> ) ;
		}
		pSymbol->nAddress |= (INT64) ECSExecutionImage::roasNakedGlobal << 56 ;
	}
	m_pcsxiDst->m_wstaSymbols.Add( wstrVarName, pSymbol ) ;
	//
	// 変数領域確保
	//
	int	nNakedSize = 0, nObjCount = 0 ;
	typeVar.GetNakedMemorySize( nNakedSize, nObjCount ) ;
	//
	BYTE *	pbytVarBuf = (BYTE*) bufNakedImage.PutBuffer( nNakedSize ) ;
	::eslFillMemory( pbytVarBuf, 0, nNakedSize ) ;
	bufNakedImage.Flush( nNakedSize ) ;
	//
	// 初期値設定
	//
	pbytVarBuf = (BYTE*) bufNakedImage.ModifyBuffer( dwVarAddr, nNakedSize ) ;
	//
	const ECSClassInfo *
		pNakedClassInf = typeVar.GetNakedMemoryClassInfo() ;
	if ( (pNakedClassInf != NULL)
		&& (IsDefaultConstructor(typeVar)
			|| (pNakedClassInf->GetVirtualFunctionCount() > 0)) )
	{
		//
		// 仮想関数ベクタのある変数の初期化は常にランタイムコード
		//
		NakedModeSaver	saver( *this ) ;
		m_pcsxi = &m_csxiNakedPrologue ;
		m_modeNakedCode = true ;
		//
		m_pcsxi->WriteSakuraPushReg( ECSSakura2Processor::regTP ) ;
		if ( fShared )
		{
			m_pcsxi->WriteSakuraLoadInt64_SharedVarAddr
				( ECSSakura2Processor::regTP, wstrVarName ) ;
		}
		else if ( fConstant )
		{
			m_pcsxi->WriteSakuraLoadInt64_ConstVarAddr
				( ECSSakura2Processor::regTP, wstrVarName ) ;
		}
		else
		{
			m_pcsxi->WriteSakuraLoadInt64_GlobalVarAddr
				( ECSSakura2Processor::regTP, wstrVarName ) ;
		}
		//
		ECSTypeInfo	typeVarTemp = typeVar ;
		typeVarTemp.SetAddressingInfo( ECSSakura2Processor::regTP, 0 ) ;
		typeVarTemp.ClearLoadedRegister() ;
		//
		err = CompileCodeNakedLocalInitialConstValue
								( typeVarTemp, pObjInit ) ;
		//
		m_pcsxi->WriteSakuraPopReg( ECSSakura2Processor::regTP ) ;
		//
		if ( err )
		{
			return	err ;
		}
	}
	else
	{
		//
		// メモリ上に初期値イメージを生成
		//
		NakedModeSaver	saver( *this ) ;
		m_pcsxi = &m_csxiNakedPrologue ;
		m_modeNakedCode = true ;
		//
		ECSTypeInfo	typeVarTemp = typeVar ;
		if ( MakeVariableInitImage
				( pbytVarBuf, typeVarTemp, pObjInit, false ) )
		{
			BeginSakura2Optimize() ;
			m_pcsxi->WriteSakuraPushReg( ECSSakura2Processor::regTP ) ;
			if ( fShared )
			{
				m_pcsxi->WriteSakuraLoadInt64_SharedVarAddr
					( ECSSakura2Processor::regTP, wstrVarName ) ;
			}
			else if ( fConstant )
			{
				m_pcsxi->WriteSakuraLoadInt64_ConstVarAddr
					( ECSSakura2Processor::regTP, wstrVarName ) ;
			}
			else
			{
				m_pcsxi->WriteSakuraLoadInt64_GlobalVarAddr
					( ECSSakura2Processor::regTP, wstrVarName ) ;
			}
			//
			typeVarTemp.SetAddressingInfo( ECSSakura2Processor::regTP, 0 ) ;
			typeVarTemp.ClearLoadedRegister() ;
			//
			MakeVariableInitImage
				( pbytVarBuf, typeVarTemp, pObjInit, true ) ;
			//
			m_pcsxi->WriteSakuraPopReg( ECSSakura2Processor::regTP ) ;
			FenceInstruction() ;
			FinishSakura2Optimize() ;
		}
	}
	//
	// エピローグにデストラクタ呼び出しコード追加
	//
	if ( ECSClassInfo::IsNeedsNakedVariableDestruction
								( typeVar.m_pValue, true ) )
	{
		NakedModeSaver	saver( *this ) ;
		m_pcsxi = &m_csxiNakedEpilogue ;
		m_modeNakedCode = true ;
		//
		BeginSakura2Optimize() ;
		int	regThis = AllocateExpressionRegister() ;
		if ( fShared )
		{
			m_pcsxi->WriteSakuraLoadInt64_SharedVarAddr( regThis, wstrVarName ) ;
		}
		else if ( fConstant )
		{
			m_pcsxi->WriteSakuraLoadInt64_ConstVarAddr( regThis, wstrVarName ) ;
		}
		else
		{
			m_pcsxi->WriteSakuraLoadInt64_GlobalVarAddr( regThis, wstrVarName ) ;
		}
		//
		err = CompileCodeNakedVariableDestruction( typeVar ) ;
		if ( err )
		{
			return	err ;
		}
		FenceInstruction() ;
		FinishSakura2Optimize() ;
	}
	//
	// 変数型定義
	//
	ECSTypeInfo *	pVarType =
		m_wstaExternNakedVarType.GetAs( wstrVarName ) ;
	if ( pVarType != NULL )
	{
		if ( *pVarType != typeVar )
		{
			m_strErrMsg = "大域変数 \'"
				+ EString(wstrVarName) + "\' の型が宣言と異なっています。" ;
			return	ESLErrorMsg( m_strErrMsg ) ;
		}
	}
	else
	{
		ECSTypeInfo *	pVarType = new ECSTypeInfo( typeVar ) ;
		pVarType->m_dwFlags |= ECSTypeInfo::flagNakedBuffer ;
		m_wstaExternNakedVarType.Add( wstrVarName, pVarType ) ;
	}
	return	eslErrSuccess ;
}

// 変数初期値設定
//////////////////////////////////////////////////////////////////////////////
bool ECSCompiler::MakeVariableInitImage
	( BYTE * pbytInitBuf, const ECSTypeInfo & typeVar,
				ECSObject * pObjInit, bool fInitRuntime )
{
	ECSObject *	pType = typeVar.m_pValue ;
	if ( pType == NULL )
	{
		return	false ;
	}
	if ( pType->m_vtType == csvtInteger )
	{
		if ( (pObjInit != NULL)
			&& (pObjInit->m_vtType == csvtInteger) )
		{
			ECSInteger *	pIntType = (ECSInteger*) pType ;
			ECSInteger *	pInitValue = (ECSInteger*) pObjInit ;
			ECSPointerReference::StoreBufferInteger
				( pbytInitBuf /*+ iOffset*/,
					pInitValue->GetValue(),
					pIntType->GetIntegerType() ) ;
		}
	}
	else if ( pType->m_vtType == csvtReal )
	{
		if ( (pObjInit != NULL)
			&& (pObjInit->m_vtType == csvtReal) )
		{
			ECSReal *	pRealType = (ECSReal*) pType ;
			ECSReal *	pInitValue = (ECSReal*) pObjInit ;
			ECSPointerReference::StoreBufferReal
				( pbytInitBuf /*+ iOffset*/,
					pInitValue->m_varReal,
					pRealType->m_vtRealType ) ;
		}
	}
	else if ( pType->m_vtType == csvtPointer )
	{
		if ( (pObjInit != NULL)
			&& (pObjInit->m_vtType == csvtString) )
		{
			if ( fInitRuntime )
			{
				ECSString *	pStrInit = (ECSString*) pObjInit ;
				m_pcsxi->WriteSakuraLoadInt64_CStrPtr
					( ECSSakura2Processor::regAcc, pStrInit->m_varStr ) ;
				WriteSakuraStoreMemory
					( ECSSakura2Processor::regAcc,
								typeVar, true, 0, true ) ;
			}
			return	true ;
		}
	}
	else if ( (pType->m_vtType == csvtArray)
			&& ((pObjInit == NULL) || (pObjInit->m_vtType == csvtArray)) )
	{
		ECSArray *	pTypeArray = (ECSArray*) pType ;
		ECSArray *	pInitArray = NULL ;
		if ( pObjInit != NULL )
		{
			pInitArray = (ECSArray*) pObjInit ;
		}
		ECSObject *	pElementType = pTypeArray->m_pDefObj ;
		if ( (pElementType != NULL) && pTypeArray->IsBounds() )
		{
			int	nElementPitch, nObjCount ;
			int	nElementCount = pTypeArray->GetBounds() ;
			ECSTypeInfo::GetNakedMemorySize
					( nElementPitch, nObjCount, pElementType ) ;
			//
			bool		fNeedsRuntime = false ;
			ECSTypeInfo	typeTemp
				( ECSTypeInfo::DuplicateType(pElementType),
											typeVar.m_dwFlags ) ;
			typeTemp.MoveRegisterAndAddressingFrom( typeVar ) ;
			//
			for ( int i = 0; i < nElementCount; i ++ )
			{
				ECSObject *	pInitElement = NULL ;
				if ( pInitArray != NULL )
				{
					pInitElement = pInitArray->m_varArray.GetAt( i ) ;
				}
				if ( pInitElement != NULL )
				{
					if ( MakeVariableInitImage
						( pbytInitBuf, typeTemp, pInitElement, fInitRuntime ) )
					{
						fNeedsRuntime = true ;
					}
				}
				pbytInitBuf += nElementPitch ;
				typeTemp.m_addrOffset += nElementPitch ;
			}
			return	fNeedsRuntime ;
		}
	}
	else if ( (pType->m_vtType == csvtObject)
			&& ((pObjInit == NULL) || (pObjInit->m_vtType == csvtArray)) )
	{
		const ECSClassInfo *	pClassInf = pType->m_pClassInf ;
		ECSArray *	pInitArray = NULL ;
		if ( pObjInit != NULL )
		{
			pInitArray = (ECSArray*) pObjInit ;
		}
		if ( (pClassInf != NULL) && pClassInf->IsNakedMemoryClass() )
		{
			bool	fNeedsRuntime = false ;
			int		nVarCount = pClassInf->GetVariableCount() ;
			for ( int i = 0; i < nVarCount; i ++ )
			{
				ECSTypeInfo *	pVarType = pClassInf->GetVariableAt( i ) ;
				if ( pVarType == NULL )
				{
					continue ;
				}
				ECSObject *	pInitValue = NULL ;
				if ( pInitArray != NULL )
				{
					pInitValue = pInitArray->m_varArray.GetAt( i ) ;
				}
				if ( pInitValue == NULL )
				{
					pInitValue = pVarType->m_pValue ;
				}
				ECSTypeInfo	typeVarMember = *pVarType ;
				int	iOffset = pClassInf->GetVariableNakedOffsetAt( i ) ;
				typeVarMember.MoveRegisterAndAddressingFrom( typeVar ) ;
				typeVarMember.m_addrOffset += iOffset ;
				//
				if ( MakeVariableInitImage
					( pbytInitBuf + iOffset,
						typeVarMember, pInitValue, fInitRuntime ) )
				{
					fNeedsRuntime = true ;
				}
			}
			return	fNeedsRuntime ;
		}
	}
	return	false ;
}

// naked モード初期値解釈
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileInitValuesForNakedVariable
	( const ECSTypeInfo & typeVar, ECSSourceStream & cssLine )
{
	ECSObject *	pType = typeVar.m_pValue ;
	if ( pType == NULL )
	{
		return	ESLErrorMsg( "void 変数に初期値を指定しています" ) ;
	}
	const ECSClassInfo *	pClassInf = pType->m_pClassInf ;
	ESLError	err ;
	if ( (pType->m_vtType == csvtInteger)
			|| (pType->m_vtType == csvtReal)
			|| (pType->m_vtType == csvtPointer)
			|| ((pClassInf != NULL)
				&& (pClassInf->IsIntegerEnumeratorType()
					|| pClassInf->IsRealEnumeratorType())) )
	{
		//
		// 整数・実数・ポインタ初期化
		//
		ECSTypeInfo	typeExpr ;
		err = CompileExpression( typeExpr, cssLine, 0, 0, L",}" ) ;
		if ( err )
		{
			return	err ;
		}
		if ( typeExpr.IsVoid() )
		{
			return	ESLErrorMsg
				( "void データを初期値をして指定しています。" ) ;
		}
		ECSTypeInfo	typeTemp ;
		err = CompileTypeMoveOperate
			( typeTemp, typeVar, typeExpr, csotNop, true ) ;
		if ( err )
		{
			return	err ;
		}
	}
	else if ( pType->m_vtType == csvtArray )
	{
		//
		// 配列初期化
		//
		if ( cssLine.HasToComeChar( L"{" ) != L'{' )
		{
			return	ESLErrorMsg
				( "配列の初期値が \'{}\' 括弧で記述されていません" ) ;
		}
		ECSArray *	pTypeArray = (ECSArray*) pType ;
		ECSObject *	pElementType = pTypeArray->m_pDefObj ;
		if ( (pElementType == NULL) || !pTypeArray->IsBounds() )
		{
			return	ESLErrorMsg
				( "naked モードで抽象配列は初期化出来ません" ) ;
		}
		bool		fClosedArray = false ;
		const int	nElementCount = pTypeArray->GetBounds() ;
		int			nElementPitch, nObjCount ;
		ECSTypeInfo::GetNakedMemorySize
				( nElementPitch, nObjCount, pElementType ) ;
		//
		ECSTypeInfo	typeTemp
			( ECSTypeInfo::DuplicateType(pElementType),
										typeVar.m_dwFlags ) ;
		typeTemp.MoveRegisterAndAddressingFrom( typeVar ) ;
		//
		for ( int i = 0; i < nElementCount; i ++ )
		{
			if ( cssLine.HasToComeChar( L"}" ) == L'}' )
			{
				fClosedArray = true ;
				break ;
			}
			err = CompileInitValuesForNakedVariable( typeTemp, cssLine ) ;
			if ( err )
			{
				return	err ;
			}
			wchar_t	wchNext = cssLine.HasToComeChar( L",}" ) ;
			if ( wchNext == L'}' )
			{
				fClosedArray = true ;
				break ;
			}
			if ( wchNext != L',' )
			{
				return	ESLErrorMsg
					( "配列の初期子が \',\' で区切られていません" ) ;
			}
			typeTemp.m_addrOffset += nElementPitch ;
		}
		if ( !fClosedArray )
		{
			if ( cssLine.HasToComeChar( L"}" ) != L'}' )
			{
				return	ESLErrorMsg
					( "配列の初期子が \'}\' で閉じられていません" ) ;
			}
		}
	}
	else if ( pType->m_vtType == csvtObject )
	{
		//
		// 構造体初期化
		//
		if ( (pClassInf == NULL) || !pClassInf->IsNakedMemoryClass() )
		{
			return	ESLErrorMsg( "naked モードで初期化出来ない型の変数です" ) ;
		}
		if ( cssLine.HasToComeChar( L"{" ) != L'{' )
		{
			return	ESLErrorMsg
				( "構造体の初期値が \'{}\' 括弧で記述されていません" ) ;
		}
		bool	fClosedStruct = false ;
		int		nVarCount = pClassInf->GetVariableCount() ;
		for ( int i = 0; i < nVarCount; i ++ )
		{
			if ( cssLine.HasToComeChar( L"}" ) == L'}' )
			{
				fClosedStruct = true ;
				break ;
			}
			ECSTypeInfo *	pVarType = pClassInf->GetVariableAt( i ) ;
			if ( pVarType == NULL )
			{
				continue ;
			}
			ECSTypeInfo	typeVarMember = *pVarType ;
			typeVarMember.MoveRegisterAndAddressingFrom( typeVar ) ;
			typeVarMember.m_addrOffset +=
						pClassInf->GetVariableNakedOffsetAt( i ) ;
			//
			err = CompileInitValuesForNakedVariable( typeVarMember, cssLine ) ;
			if ( err )
			{
				return	err ;
			}
			wchar_t	wchNext = cssLine.HasToComeChar( L",}" ) ;
			if ( wchNext == L'}' )
			{
				fClosedStruct = true ;
				break ;
			}
			if ( wchNext != L',' )
			{
				return	ESLErrorMsg
					( "構造体の初期子が \',\' で区切られていません" ) ;
			}
		}
		if ( !fClosedStruct )
		{
			if ( cssLine.HasToComeChar( L"}" ) != L'}' )
			{
				return	ESLErrorMsg
					( "構造体の初期子が \'}\' で閉じられていません" ) ;
			}
		}
	}
	else
	{
		return	ESLErrorMsg( "naked モードで初期化出来ない型の変数です" ) ;
	}
	return	eslErrSuccess ;
}

// Constant 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileConstant( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwConstant ) )
	{
		return	eslErrSuccess ;
	}
	while ( !cssLine.DisregardSpace() )
	{
		//
		// 定数名を取得
		//
		int		nTokenType ;
		ESLError	err ;
		ECSWideString	wstrName = cssLine.GetAToken( &nTokenType ) ;
		ECSWideString	wstrToken ;
		if ( (wstrName == L":=") || (wstrName == L"=") )
		{
			wstrToken = wstrName ;
			wstrName = L"" ;
		}
		else if ( nTokenType != 0 )
		{
			return	ESLErrorMsg( "定数名に不正な文字が使用されています。" ) ;
		}
		else
		{
			err = VerifyUserSymbol( wstrName ) ;
			if ( err )
			{
				return	err ;
			}
			EWideString		wstrNameSpace = GetCurrentSpaceName() ;
			if ( !wstrNameSpace.IsEmpty() )
			{
				wstrName = wstrNameSpace + L"::" + wstrName ;
			}
			wstrToken = cssLine.GetAToken( ) ;
		}
		if ( wstrToken == L"=" )
		{
			if ( !(m_dwModeFlags & flagNoWarningEquMove) )
			{
				err = OutputWarning1
					( "定数値の定義に = が使われています。"
						":= を使用してください（推奨）。",
						m_strFilePath, m_nLineNum ) ;
				if ( err )
				{
					return	err ;
				}
			}
		}
		else if ( wstrToken != L":=" )
		{
			return	ESLErrorMsg( "定数値を := で結び付けてください。" ) ;
		}
		//
		// 定数値を取得
		//
		ECSObject *	pValue ;
		err = CalculateExpression( pValue, cssLine, 0, L"," ) ;
		if ( err )
		{
			return	err ;
		}
		//
		if ( m_rwCtrlType == rwData )
		{
			//
			// Data 定数項目
			//
			EControlNest *	pNest = m_nestCtrl.GetLastAt( 0 ) ;
			ESLAssert( pNest != NULL ) ;
			ESLAssert( pNest->m_rwType == m_rwCtrlType ) ;
			//
			int	iData =
				m_pcsxiDst->m_csgDataType.m_staObjName.
						FindIndex( pNest->m_wstrName ) ;
			if ( iData < 0 )
			{
				delete	pValue ;
				return	ESLErrorMsg( "Data 文コンパイラー内部エラー" ) ;
			}
			ECSGlobal *	pData = ESLTypeCast<ECSGlobal>
				( m_pcsxiDst->m_csgDataType.m_varArray.GetAt( iData ) ) ;
			if ( pData == NULL )
			{
				delete	pValue ;
				return	ESLErrorMsg( "Data 文コンパイラー内部エラー" ) ;
			}
			if ( (pData->m_pDefObj != NULL)
				&& (m_dwModeFlags & flagStrictStyle) )
			{
				if ( !ECSTypeInfo::IsTypeEqual( pData->m_pDefObj, pValue ) )
				{
					err = OutputWarning0
						( "Data テーブル要素の型が一致しません",
										m_strFilePath, m_nLineNum ) ;
					if ( err )
					{
						return	err ;
					}
				}
			}
			pData->AddVariable( wstrName, pValue ) ;
		}
		else
		{
			//
			// 大域定数
			//
			if ( wstrName.IsEmpty() )
			{
				delete	pValue ;
				return	ESLErrorMsg
					( "Data ブロックの外の Constant 文では、"
								"定数名を省略できません。" ) ;
			}
			if ( m_staConstant.GetAs( wstrName ) != NULL )
			{
				delete	pValue ;
				m_strErrMsg = "大域定数 \'"
					+ EString(wstrName) + "\' は既に定義されています。" ;
				return	ESLErrorMsg( m_strErrMsg ) ;
			}
			m_staConstant.SetAs( wstrName, pValue ) ;
		}
		//
		wstrToken = cssLine.GetAToken( ) ;
		if ( wstrToken.IsEmpty() )
		{
			break ;
		}
		if ( wstrToken != L"," )
		{
			return	ESLErrorMsg( "定数数が , 記号で区切られていません。" ) ;
		}
	}
	return	eslErrSuccess ;
}

// Data 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileData( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwData ) )
	{
		return	eslErrSuccess ;
	}
	//
	// 定数テーブル名取得
	//
	int	nTokenType ;
	ECSWideString	wstrName = cssLine.GetAToken( &nTokenType ) ;
	if ( nTokenType != 0 )
	{
		return	ESLErrorMsg
			( "定数テーブル名に不正な文字が使用されています。" ) ;
	}
	SYMBOL_NAMESPACE	snsSymbol = wstrName ;
	ESLError	err = ParseFullNameSymbol( snsSymbol, cssLine ) ;
	if ( err )
	{
		return	err ;
	}
	wstrName = snsSymbol.wstrFullName ;
	//
	// データテーブル登録
	//
	if ( m_pcsxiDst->m_csgDataType.m_staObjName.FindIndex( wstrName ) >= 0 )
	{
		m_strErrMsg = "定数テーブル名 \'"
			+ EString(wstrName) + "\' は既に定義されています。" ;
		return	ESLErrorMsg( m_strErrMsg ) ;
	}
	//
	// 共有データか定数テーブルか判定
	//
	ECSWideString	wstrToken = cssLine.GetAToken( ) ;
	if ( wstrToken.IsEmpty() || (wstrToken == L"<") )
	{
		ECSGlobal *	pGlobal = new ECSGlobal ;
		if ( wstrToken == L"<" )
		{
			ECSTypeInfo	typeVar ;
			err = ParseTypeDescription( typeVar, cssLine ) ;
			if ( err )
			{
				return	err ;
			}
			pGlobal->SetDefaultElement( typeVar.DetachValue() ) ;
			//
			if ( cssLine.HasToComeChar( L">" ) != L'>' )
			{
				return	ESLErrorMsg
					( "Data テーブル要素型が \'>\' で閉じられていません。" ) ;
			}
		}
		m_pcsxiDst->m_csgDataType.AddVariable( wstrName, pGlobal ) ;
		m_csxiInitFunc.m_impDataRef.Add( wstrName, new ENumArray<DWORD> ) ;
		//
		// データセクション開始
		//
		EControlNest *	pNest = new EControlNest( m_dwImplementFlags ) ;
		pNest->m_rwType = rwData ;
		pNest->m_wstrName = wstrName ;
		m_rwCtrlType = rwData ;
		m_nestCtrl.Add( pNest ) ;
		//
		if ( m_nestCtrl.GetSize() > 1 )
		{
			return	ESLErrorMsg
				( "Data 構文が他の制御ブロックの中に記述されています。" ) ;
		}
	}
	else if ( wstrToken == L":" )
	{
		//
		// 共用データ
		//
		ECSTypeInfo	typeVar ;
		bool	fNaked = cssLine.HasToComeToken( L"naked" ) ;
		err = ParseTypeDescription( typeVar, cssLine ) ;
		if ( err )
		{
			return	err ;
		}
		if ( !fNaked )
		{
			err = DeclareSharedGlobalObjectVariable( wstrName, typeVar, NULL ) ;
		}
		else
		{
			err = DeclareGlobalNakedVariable
						( wstrName, typeVar, NULL, true, false ) ;
		}
		if ( err )
		{
			return	err ;
		}
	}
	else
	{
		m_strErrMsg = "Data 文に不正なトークン \'"
			+ EString(wstrToken) + "\' が記述されています。" ;
		return	ESLErrorMsg( m_strErrMsg ) ;
	}
	return	eslErrSuccess ;
}

// EndData 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileEndData( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwEndData ) )
	{
		return	eslErrSuccess ;
	}
	if ( m_rwCtrlType != rwData )
	{
		return	ESLErrorMsg( "EndData 文が Data 文と対応していません。" ) ;
	}
	EControlNest *	pNest = m_nestCtrl.GetLastAt( 0 ) ;
	ESLAssert( pNest != NULL ) ;
	ESLAssert( pNest->m_rwType == m_rwCtrlType ) ;
	//
	LeaveControlNest() ;
	return	eslErrSuccess ;
}

// Enumerator 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileEnumerator( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwEnumerator ) )
	{
		return	eslErrSuccess ;
	}
	//
	// 列挙型名取得
	//
	ESLError	err ;
	int			nTokenType ;
	ECSWideString	wstrName = cssLine.GetAToken( &nTokenType ) ;
	if ( nTokenType != 0 )
	{
		return	ESLErrorMsg
			( "列挙型名に不正な文字が使用されています。" ) ;
	}
	err = VerifyUserSymbol( wstrName ) ;
	if ( err )
	{
		return	err ;
	}
	EWideString	wstrSpaceName = GetCurrentSpaceName() ;
	EWideString	wstrEnumSpaceName = wstrSpaceName ;
	if ( !wstrSpaceName.IsEmpty() )
	{
		wstrSpaceName += L"::" ;
	}
	EWideString	wstrGlobalName = wstrSpaceName + wstrName ;
	//
	ECSClassInfo *	pClassInf = GetClassInfoAs( wstrGlobalName ) ;
	bool			fAddEnumInf = true ;
	if ( pClassInf != NULL )
	{
		if ( !pClassInf->IsEmpty() )
		{
			return	ESLErrorMsg
				( "既に定義されている列挙型を再定義しようとしています。" ) ;
		}
		fAddEnumInf = false ;
	}
	else
	{
		pClassInf = new ECSClassInfo ;
	}
	pClassInf->SetAttribute
		( ECSTypeInfo::flagEnumerator | ECSTypeInfo::flagNativeObject ) ;
	pClassInf->SetName( wstrName ) ;
	pClassInf->SetGlobalName( wstrGlobalName ) ;
	//
	if ( fAddEnumInf )
	{
		m_pcsxiDst->AddClassInfo( pClassInf ) ;
	}
	if ( m_staTypeName.FindIndex( wstrGlobalName ) < 0 )
	{
		m_staTypeName.Add( wstrGlobalName ) ;
	}
	//
	// 列挙子型
	//
	ECSClassInfo *	pEnumTypeInf = NULL ;
	CSVariableType	vtEnumType = csvtReference ;
	if ( cssLine.HasToComeChar( L"<" ) == L'<' )
	{
		ECSObject *	pType = ParseBsaicType( cssLine.GetAToken() ) ;
		if ( pType != NULL )
		{
			vtEnumType = pType->m_vtType ;
			delete	pType ;
			switch ( vtEnumType )
			{
			case	csvtInteger:
				pEnumTypeInf = GetClassInfoAs( L"Integer" ) ;
				break ;
			case	csvtReal:
				pEnumTypeInf = GetClassInfoAs( L"Real" ) ;
				break ;
			case	csvtString:
				pEnumTypeInf = GetClassInfoAs( L"String" ) ;
				break ;
			default:
				break ;
			}
			if ( pEnumTypeInf == NULL )
			{
				return	ESLErrorMsg
					( "列挙子型に指定することのできない型が指定されています" ) ;
			}
			if ( cssLine.HasToComeChar( L">" ) != L'>' )
			{
				return	ESLErrorMsg( "列挙子型が \'>\' で閉じられていません" ) ;
			}
		}
		else
		{
			return	ESLErrorMsg( "列挙子型の指定が不正です" ) ;
		}
	}
	/*
	if ( pEnumTypeInf == NULL )
	{
		pEnumTypeInf = GetClassInfoAs( L"Reference" ) ;
		if ( pEnumTypeInf == NULL )
		{
			return	ESLErrorMsg
				( "列挙子型に純粋 Reference 型を指定することができません" ) ;
		}
	}
	*/
	if ( pEnumTypeInf != NULL )
	{
		ECSClassInfo::ParentClass *
			pParentClass = new ECSClassInfo::ParentClass ;
		pParentClass->dwFlags = ECSTypeInfo::flagPublic ;
		pParentClass->pClassInf = pEnumTypeInf ;
		pClassInf->AddParentClassInfo( pParentClass ) ;
	}
	pClassInf->BuildAllClassCast() ;
	//
	// C 互換名前空間判定
	//
	static const wchar_t * pwszCNamespace[] =
	{
		L"As", L"C", L"Namespace", NULL
	} ;
	bool	fAsCNamespace = false ;
	err = eslErrSuccess ;
	for ( int i = 0; pwszCNamespace[i] != NULL; i ++ )
	{
		EWideString	wstrToken = cssLine.GetAToken() ;
		if ( !wstrToken.IsEmpty() )
		{
			if ( CompareReservedWord( pwszCNamespace[i], wstrToken ) == 0 )
			{
				fAsCNamespace = true ;
			}
			else
			{
				m_strErrMsg = EString(wstrToken) + " は不正な書式です" ;
				err = ESLErrorMsg( m_strErrMsg ) ;
				fAsCNamespace = false ;
				break ;
			}
		}
		else if ( fAsCNamespace )
		{
			err = ESLErrorMsg( "列挙型構文が不正です" ) ;
			fAsCNamespace = false ;
			break ;
		}
		else
		{
			break ;
		}
	}
	if ( !fAsCNamespace )
	{
		wstrEnumSpaceName = wstrGlobalName ;
	}
	//
	// 構築関数定義
	//
	ECSPrototypeInfo	prototype ;
	prototype.SetAttribute( 0 ) ;
	prototype.SetName( wstrName ) ;
	prototype.SetGlobalName( wstrGlobalName + L"::" + wstrName ) ;
	prototype.SetReturnType( ECSTypeInfo() ) ;
	switch ( vtEnumType )
	{
	case	csvtInteger:
		prototype.AddArgument( new ECSTypeInfo( new ECSInteger() ) ) ;
		break ;
	case	csvtReal:
		prototype.AddArgument( new ECSTypeInfo( new ECSReal() ) ) ;
		break ;
	case	csvtString:
		prototype.AddArgument( new ECSTypeInfo( new ECSString() ) ) ;
		break ;
	case	csvtReference:
		prototype.AddArgument( new ECSTypeInfo( new ECSReference() ) ) ;
		break ;
	}
	pClassInf->AddOverrideFunction( prototype ) ;
	//
	// ネスト設定
	//
	EControlNest *	pNest = new EControlNest( m_dwImplementFlags ) ;
	m_rwCtrlType = rwEnumerator ;
	pNest->m_rwType = rwEnumerator ;
	pNest->m_wstrName = wstrGlobalName ;
	pNest->m_wstrCurSpaceName = wstrEnumSpaceName ;
	pNest->m_wstrThisSpaceName = wstrGlobalName ;
	pNest->m_nEnumeratorValue = 0 ;
	AddNecessaryUsingNamespace( pNest ) ;
	m_nestCtrl.Add( pNest ) ;
	//
	return	err ;
}

// EndEnum 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileEndEnum( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwEndEnum ) )
	{
		return	eslErrSuccess ;
	}
	if ( m_rwCtrlType != rwEnumerator )
	{
		return	ESLErrorMsg
			( "EndEnum 文が Enumerator 文と対応していません。" ) ;
	}
	EControlNest *	pNest = m_nestCtrl.GetLastAt( 0 ) ;
	ESLAssert( pNest != NULL ) ;
	ESLAssert( pNest->m_rwType == m_rwCtrlType ) ;
	//
	// 列挙型取得
	//
	EWideString		wstrEnumTypeName = GetCurrentThisClassName() ;
	ECSClassInfo *	pClassInf = GetClassInfoAs( wstrEnumTypeName ) ;
	if ( pClassInf != NULL )
	{
		ECSClassInfo::ParentClass *
			pParentType = pClassInf->GetParentClassAt( 0 ) ;
		if ( (pParentType == NULL) || (pParentType->pClassInf == NULL) )
		{
			//
			// 型指定の無い列挙型の時は、デフォルトで代入演算子を定義
			//
			static const wchar_t *	pwszDefPrototype[] =
			{
				L"Integer operator := ( Integer num )",
				L"Real operator := ( Real num )",
				L"String operator := ( String str )",
				NULL
			} ;
			EObjArray<EWideString>	lstPrototype ;
			int	i ;
			for ( i = 0; pwszDefPrototype[i] != NULL; i ++ )
			{
				lstPrototype[i] = pwszDefPrototype[i] ;
			}
			lstPrototype.InsertAt
				( 0, new EWideString
					( wstrEnumTypeName + L"& operator := ( const "
									+ wstrEnumTypeName + L"& obj )" ) ) ;
			//
			for ( i = 0; i < (int) lstPrototype.GetSize(); i ++ )
			{
				ECSPrototypeInfo	prototype ;
				ECSSourceStream		cssLine = lstPrototype[i] ;
				ESLError	err = ParsePrototypeDescription( prototype, cssLine ) ;
				if ( err )
				{
					LeaveControlNest() ;
					return	ESLErrorMsg
						( "内部エラー："
							"列挙型のデフォルト構文の定義でエラーが発生しました" ) ;
				}
				prototype.SetGlobalName
					( pClassInf->GetGlobalName() + L"::" + prototype.GetName() ) ;
				pClassInf->AddOverrideFunction( prototype ) ;
			}
		}
	}
	//
	LeaveControlNest() ;
	return	eslErrSuccess ;
}

// 列挙子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileEnumerate( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwInvalid, stEnumeration ) )
	{
		return	eslErrSuccess ;
	}
	for ( ; ; )
	{
		EWideString	wstrName = cssLine.GetAToken() ;
		EWideString	wstrLocalName = wstrName ;
		ESLError	err = VerifyUserSymbol( wstrName ) ;
		if ( err )
		{
			return	err ;
		}
		EWideString	wstrToken = cssLine.GetAToken() ;
		bool	fEnumValue = true ;
		if ( wstrToken == L"=" )
		{
			if ( !(m_dwModeFlags & flagNoWarningEquMove) )
			{
				err = OutputWarning1
					( "列挙子の定義に = が使われています。"
						":= を使用してください（推奨）。",
						m_strFilePath, m_nLineNum ) ;
				if ( err )
				{
					return	err ;
				}
			}
		}
		else if ( wstrToken != L":=" )
		{
			fEnumValue = false ;
		}
		EWideString		wstrNameSpace = GetCurrentSpaceName() ;
		if ( !wstrNameSpace.IsEmpty() )
		{
			wstrName = wstrNameSpace + L"::" + wstrName ;
		}
		//
		// 列挙型取得
		//
		CSVariableType	vtEnumerator = csvtReference ;
		ECSClassInfo *
			pClassInf = GetClassInfoAs( GetCurrentThisClassName() ) ;
		if ( pClassInf != NULL )
		{
			ECSClassInfo::ParentClass *
				pParentType = pClassInf->GetParentClassAt( 0 ) ;
			if ( (pParentType != NULL) && (pParentType->pClassInf != NULL) )
			{
				const EWideString &	wstrType =
						pParentType->pClassInf->GetGlobalName() ;
				if ( wstrType == L"Integer" )
				{
					vtEnumerator = csvtInteger ;
				}
				else if ( wstrType == L"Real" )
				{
					vtEnumerator = csvtReal ;
				}
				else if ( wstrType == L"String" )
				{
					vtEnumerator = csvtString ;
				}
			}
		}
		else
		{
			return	ESLErrorMsg( "内部エラー：列挙型情報が見つかりません" ) ;
		}
		//
		// 定数値を取得
		//
		ECSObject *	pValue = NULL ;
		if ( fEnumValue )
		{
			err = CalculateExpression( pValue, cssLine, 0, L"," ) ;
			if ( err )
			{
				return	err ;
			}
			wstrToken = cssLine.GetAToken() ;
		}
		else
		{
			if ( vtEnumerator != csvtInteger )
			{
				if ( wstrToken == L"" )
				{
					return	ESLErrorMsg( "整数でない列挙子の値が省略されています" ) ;
				}
				else if ( wstrToken != L"," )
				{
					return	ESLErrorMsg( "列挙子の値を := で結び付けてください。" ) ;
				}
			}
			EControlNest *	pNest = m_nestCtrl.GetLastAt( 0 ) ;
			ESLAssert( pNest != NULL ) ;
			ESLAssert( pNest->m_rwType == rwEnumerator ) ;
			pValue = new ECSInteger( pNest->m_nEnumeratorValue ) ;
		}
		//
		// 型判定
		//
		if ( vtEnumerator != csvtReference )
		{
			if ( vtEnumerator != pValue->m_vtType )
			{
				delete	pValue ;
				return	ESLErrorMsg( "列挙子の型が一致しません" ) ;
			}
			if ( vtEnumerator == csvtInteger )
			{
				EControlNest *	pNest = m_nestCtrl.GetLastAt( 0 ) ;
				ESLAssert( pNest != NULL ) ;
				ESLAssert( pNest->m_rwType == rwEnumerator ) ;
				pNest->m_nEnumeratorValue = ((ECSInteger*)pValue)->GetValue() + 1 ;
			}
		}
		else
		{
			switch ( pValue->m_vtType )
			{
			case	csvtInteger:
			case	csvtReal:
			case	csvtString:
				break ;
			default:
				delete	pValue ;
				return	ESLErrorMsg( "列挙子に使用できない型が記述されています" ) ;
			}
		}
		//
		// メンバ変数定義（ラインタイム用情報）
		//
		if ( pClassInf != NULL )
		{
			pClassInf->AddVariable
				( wstrLocalName,
					new ECSTypeInfo
						( pValue->Duplicate(),
							ECSTypeInfo::flagDeterministic ) ) ;
		}
		//
		// 定数定義
		//
		if ( m_staConstant.GetAs( wstrName ) != NULL )
		{
			delete	pValue ;
			m_strErrMsg = "列挙子 \'"
				+ EString(wstrName) + "\' は既に定義されています。" ;
			return	ESLErrorMsg( m_strErrMsg ) ;
		}
		m_staConstant.SetAs( wstrName, pValue ) ;
		//
		if ( wstrToken == L"" )
		{
			break ;
		}
		else if ( wstrToken != L"," )
		{
			return	ESLErrorMsg( "列挙子が , で区切られていません" ) ;
		}
	}
	return	eslErrSuccess ;
}

// Structure 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileStructure( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwStructure ) )
	{
		return	eslErrSuccess ;
	}
	//
	// 構造体名取得
	//
	ESLError	err ;
	int			nTokenType ;
	DWORD		dwFlags = ECSTypeInfo::flagStructure ;
	ECSWideString	wstrName = cssLine.GetAToken( &nTokenType ) ;
	if ( CompareReservedWord( L"Naked", wstrName ) == 0 )
	{
		dwFlags |= ECSTypeInfo::flagNakedBuffer ;
		wstrName = cssLine.GetAToken( &nTokenType ) ;
	}
	if ( m_dwModeFlags & flagDefaultNakedAll )
	{
		dwFlags |= ECSTypeInfo::flagNakedBuffer ;
	}
	if ( nTokenType != 0 )
	{
		return	ESLErrorMsg
			( "構造体名に不正な文字が使用されています。" ) ;
	}
	err = VerifyUserSymbol( wstrName ) ;
	if ( err )
	{
		return	err ;
	}
	SYMBOL_NAMESPACE	snsSymbol = wstrName ;
	ParseTemplateArgument( snsSymbol, cssLine, true ) ;
	//
	EWideString	wstrSpaceName = GetCurrentSpaceName() ;
	if ( !wstrSpaceName.IsEmpty() )
	{
		wstrSpaceName += L"::" ;
	}
	EWideString	wstrGlobalName = wstrSpaceName + snsSymbol.wstrFullName ;
	wstrName = snsSymbol.wstrName ;
	int	iTempArg = wstrName.Find( L'<' ) ;
	if ( iTempArg >= 0 )
	{
		wstrName = wstrName.Left( iTempArg ) ;
	}
	//
	ECSClassInfo *	pClassInf = GetClassInfoAs( wstrGlobalName ) ;
	bool			fAddClassInf = true ;
	bool			fFirstPhase = true ;
	if ( pClassInf != NULL )
	{
		if ( !pClassInf->IsEmpty() )
		{
			if ( !(m_dwImplementFlags & implementDeclaration)
				&& (pClassInf->m_phase == ECSClassInfo::phaseImplement) )
			{
				fFirstPhase = false ;
			}
			else
			{
				return	ESLErrorMsg
					( "既に定義されているクラスを再定義しようとしています。" ) ;
			}
		}
		fAddClassInf = false ;
	}
	else
	{
		pClassInf = new ECSClassInfo ;
		if ( !(m_dwImplementFlags & implementFunction) )
		{
			pClassInf->m_phase = ECSClassInfo::phaseDeclaration ;
		}
	}
	pClassInf->SetAttribute( dwFlags ) ;
	pClassInf->SetName( wstrName ) ;
	pClassInf->SetGlobalName( wstrGlobalName ) ;
	//
	if ( fAddClassInf )
	{
		m_pcsxiDst->AddClassInfo( pClassInf ) ;
	}
	if ( m_staTypeName.FindIndex( wstrGlobalName ) < 0 )
	{
		m_staTypeName.Add( wstrGlobalName ) ;
	}
	//
	// 構造体の派生
	//
	if ( fFirstPhase )
	{
		if ( cssLine.HasToComeChar( L":" ) == L':' )
		do
		{
			SYMBOL_NAMESPACE	snsParent = cssLine.GetAToken() ;
			err = ParseFullNameSymbol( snsParent, cssLine ) ;
			if ( err )
			{
				return	err ;
			}
			ECSClassInfo *	pParentClassInf =
					GetClassInfoAs( snsParent.wstrFullName ) ;
			if ( pParentClassInf == NULL )
			{
				m_strErrMsg =
					EString( snsParent.wstrFullName )
						+ " は定義されていない構造体です。" ;
				return	ESLErrorMsg( m_strErrMsg ) ;
			}
			if ( !(pParentClassInf->GetAttribute()
						& ECSTypeInfo::flagStructure) )
			{
				m_strErrMsg =
					EString( snsParent.wstrFullName )
						+ " は構造体ではありません。" ;
				return	ESLErrorMsg( m_strErrMsg ) ;
			}
			ECSClassInfo::ParentClass *
				pParentClass = new ECSClassInfo::ParentClass ;
			pParentClass->dwFlags = ECSTypeInfo::flagPublic ;
			pParentClass->pClassInf = pParentClassInf ;
			pClassInf->AddParentClassInfo( pParentClass ) ;
			//
			if ( cssLine.HasToComeChar( L"," ) != L',' )
			{
				break ;
			}
		}
		while ( !cssLine.DisregardSpace() ) ;
		//
		pClassInf->BuildAllClassCast() ;
	}
	else
	{
		cssLine.MoveIndex( cssLine.GetLength() ) ;
	}
	//
	// ネスト設定
	//
	EControlNest *	pNest = new EControlNest( m_dwImplementFlags ) ;
	m_rwCtrlType = rwStructure ;
	pNest->m_rwType = rwStructure ;
	pNest->m_wstrName = wstrGlobalName ;
	pNest->m_wstrCurSpaceName = wstrGlobalName ;
	AddNecessaryUsingNamespace( pNest ) ;
	m_nestCtrl.Add( pNest ) ;
	//
	return	eslErrSuccess ;
}

// EndStruct 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileEndStruct( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwEndStruct ) )
	{
		return	eslErrSuccess ;
	}
	if ( m_rwCtrlType != rwStructure )
	{
		return	ESLErrorMsg
			( "EndStruct 文が Structure 文と対応していません。" ) ;
	}
	EControlNest *	pNest = m_nestCtrl.GetLastAt( 0 ) ;
	ESLAssert( pNest != NULL ) ;
	ESLAssert( pNest->m_rwType == m_rwCtrlType ) ;
	//
	ESLError		err = eslErrSuccess ;
	ECSClassInfo *	pClassInf = GetClassInfoAs( pNest->m_wstrCurSpaceName ) ;
	if ( pClassInf != NULL )
	{
		err = CompileFinalizeClassInfo( pClassInf ) ;
	}
	LeaveControlNest() ;
	return	err ;
}

// Class 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileClass( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwClass ) )
	{
		return	eslErrSuccess ;
	}
	//
	// クラス名取得
	//
	ESLError	err ;
	int			nTokenType ;
	DWORD		dwFlags = 0 ;
	ECSWideString	wstrName = cssLine.GetAToken( &nTokenType ) ;
	if ( CompareReservedWord( L"Native", wstrName ) == 0 )
	{
		dwFlags |= ECSTypeInfo::flagNativeObject ;
		wstrName = cssLine.GetAToken( &nTokenType ) ;
	}
	else if ( CompareReservedWord( L"Naked", wstrName ) == 0 )
	{
		dwFlags |= ECSTypeInfo::flagNakedBuffer ;
		wstrName = cssLine.GetAToken( &nTokenType ) ;
	}
	else if ( m_dwModeFlags & flagDefaultNakedAll )
	{
		dwFlags |= ECSTypeInfo::flagNakedBuffer ;
	}
	if ( nTokenType != 0 )
	{
		return	ESLErrorMsg
			( "クラス名に不正な文字が使用されています。" ) ;
	}
	if ( !(dwFlags & ECSTypeInfo::flagNativeObject) )
	{
		err = VerifyUserSymbol( wstrName ) ;
		if ( err )
		{
			return	err ;
		}
	}
	EWideString			wstrClassName = wstrName ;
	SYMBOL_NAMESPACE	snsSymbol = wstrName ;
	ParseTemplateArgument( snsSymbol, cssLine, true ) ;
	//
	EWideString	wstrSpaceName = GetCurrentSpaceName() ;
	if ( !wstrSpaceName.IsEmpty() )
	{
		wstrSpaceName += L"::" ;
	}
	EWideString	wstrGlobalName = wstrSpaceName + snsSymbol.wstrFullName ;
	wstrClassName = snsSymbol.wstrName ;
	int	iTempArg = wstrClassName.Find( L'<' ) ;
	if ( iTempArg >= 0 )
	{
		wstrClassName = wstrName.Left( iTempArg ) ;
	}
	//
	ECSClassInfo *	pClassInf = GetClassInfoAs( wstrGlobalName ) ;
	bool			fAddClassInf = true ;
	bool			fFirstPhase = true ;
	if ( pClassInf != NULL )
	{
		if ( !pClassInf->IsEmpty() )
		{
			if ( !(m_dwImplementFlags & implementDeclaration)
				&& (pClassInf->m_phase == ECSClassInfo::phaseImplement) )
			{
				fFirstPhase = false ;
			}
			else
			{
				return	ESLErrorMsg
					( "既に定義されているクラスを再定義しようとしています。" ) ;
			}
		}
		fAddClassInf = false ;
	}
	else
	{
		pClassInf = new ECSClassInfo ;
		if ( !(m_dwImplementFlags & implementFunction) )
		{
			pClassInf->m_phase = ECSClassInfo::phaseDeclaration ;
		}
	}
	pClassInf->SetAttribute( dwFlags ) ;
	pClassInf->SetName( wstrClassName ) ;
	pClassInf->SetGlobalName( wstrGlobalName ) ;
	//
	if ( fAddClassInf )
	{
		m_pcsxiDst->AddClassInfo( pClassInf ) ;
	}
	if ( m_staTypeName.FindIndex( wstrGlobalName ) < 0 )
	{
		m_staTypeName.Add( wstrGlobalName ) ;
	}
	//
	// クラスの派生
	//
	if ( fFirstPhase )
	{
		if ( (cssLine.HasToComeChar( L":" ) == L':') )
		do
		{
			EWideString	wstrScope = cssLine.GetAToken() ;
			DWORD	dwScope = ECSTypeInfo::flagPublic ;
			if ( CompareReservedWord( L"Public", wstrScope ) == 0 )
			{
				dwScope = ECSTypeInfo::flagPublic ;
			}
			else if ( CompareReservedWord( L"Protected", wstrScope ) == 0 )
			{
				dwScope = ECSTypeInfo::flagProtected ;
			}
			else if ( CompareReservedWord( L"Private", wstrScope ) == 0 )
			{
				dwScope = ECSTypeInfo::flagPrivate ;
			}
			else
			{
				m_strErrMsg = EString( wstrScope )
					+ " は親クラスの不正なアクセススコープ指定です。" ;
				return	ESLErrorMsg( m_strErrMsg ) ;
			}
			SYMBOL_NAMESPACE	snsParent = cssLine.GetAToken() ;
			err = ParseFullNameSymbol( snsParent, cssLine ) ;
			if ( err )
			{
				return	err ;
			}
			ECSClassInfo *	pParentClassInf = GetClassInfoAs( snsParent.wstrFullName ) ;
			if ( pParentClassInf == NULL )
			{
				m_strErrMsg =
					EString( snsParent.wstrFullName )
						+ " は定義されていないクラスです。" ;
				return	ESLErrorMsg( m_strErrMsg ) ;
			}
			ECSClassInfo::ParentClass *
				pParentClass = new ECSClassInfo::ParentClass ;
			pParentClass->dwFlags = dwScope ;
			pParentClass->pClassInf = pParentClassInf ;
			pClassInf->AddParentClassInfo( pParentClass ) ;
			//
			if ( cssLine.HasToComeChar( L"," ) != L',' )
			{
				break ;
			}
		}
		while ( !cssLine.DisregardSpace() ) ;
		//
		pClassInf->BuildAllClassCast() ;
	}
	else
	{
		cssLine.MoveIndex( cssLine.GetLength() ) ;
	}
	//
	// ネスト設定
	//
	EControlNest *	pNest = new EControlNest( m_dwImplementFlags ) ;
	m_rwCtrlType = rwClass ;
	pNest->m_rwType = rwClass ;
	pNest->m_wstrName = wstrGlobalName ;
	pNest->m_wstrCurSpaceName = wstrGlobalName ;
	pNest->m_dwProtectedScope = ECSTypeInfo::flagPrivate ;
	AddNecessaryUsingNamespace( pNest ) ;
	m_nestCtrl.Add( pNest ) ;
	//
	return	eslErrSuccess ;
}

// EndClass 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileEndClass( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwEndClass ) )
	{
		return	eslErrSuccess ;
	}
	if ( m_rwCtrlType != rwClass )
	{
		return	ESLErrorMsg
			( "EndClass 文が Class 文と対応していません。" ) ;
	}
	EControlNest *	pNest = m_nestCtrl.GetLastAt( 0 ) ;
	ESLAssert( pNest != NULL ) ;
	ESLAssert( pNest->m_rwType == m_rwCtrlType ) ;
	//
	ESLError		err = eslErrSuccess ;
	ECSClassInfo *	pClassInf = GetClassInfoAs( pNest->m_wstrCurSpaceName ) ;
	if ( pClassInf != NULL )
	{
		err = CompileFinalizeClassInfo( pClassInf ) ;
	}
	LeaveControlNest() ;
	return	err ;
}

// Namespace 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileNamespace( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwNamespace ) )
	{
		return	eslErrSuccess ;
	}
	//
	// 名前空間取得
	//
	ESLError	err ;
	ECSWideString	wstrName = cssLine.GetAToken() ;
	err = VerifyUserSymbol( wstrName ) ;
	if ( err )
	{
		return	err ;
	}
	EWideString	wstrSpaceName = GetCurrentSpaceName() ;
	if ( !wstrSpaceName.IsEmpty() )
	{
		wstrSpaceName += L"::" ;
	}
	EWideString	wstrGlobalName = wstrSpaceName + wstrName ;
	//
	ECSClassInfo *	pClassInf = GetClassInfoAs( wstrGlobalName ) ;
	bool			fAddClassInf = true ;
	if ( pClassInf != NULL )
	{
		fAddClassInf = false ;
	}
	else
	{
		pClassInf = new ECSClassInfo ;
	}
	pClassInf->SetAttribute
		( ECSTypeInfo::flagNamespace | ECSTypeInfo::flagAbstract ) ;
	pClassInf->SetName( wstrName ) ;
	pClassInf->SetGlobalName( wstrGlobalName ) ;
	//
	if ( fAddClassInf )
	{
		m_pcsxiDst->AddClassInfo( pClassInf ) ;
		pClassInf->BuildAllClassCast() ;
	}
	//
	// ネスト設定
	//
	EControlNest *	pNest = new EControlNest( m_dwImplementFlags ) ;
	m_rwCtrlType = rwNamespace ;
	pNest->m_rwType = rwNamespace ;
	pNest->m_wstrName = wstrGlobalName ;
	pNest->m_wstrCurSpaceName = wstrGlobalName ;
	pNest->m_dwProtectedScope = ECSTypeInfo::flagPublic ;
	AddNecessaryUsingNamespace( pNest ) ;
	m_nestCtrl.Add( pNest ) ;
	//
	return	eslErrSuccess ;
}

// EndNamespace 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileEndNamespace( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwEndNamespace ) )
	{
		return	eslErrSuccess ;
	}
	if ( m_rwCtrlType != rwNamespace )
	{
		return	ESLErrorMsg
			( "EndNamespace 文が Namespace 文と対応していません。" ) ;
	}
	EControlNest *	pNest = m_nestCtrl.GetLastAt( 0 ) ;
	ESLAssert( pNest != NULL ) ;
	ESLAssert( pNest->m_rwType == m_rwCtrlType ) ;
	//
	ESLError		err = eslErrSuccess ;
	ECSClassInfo *	pClassInf = GetClassInfoAs( pNest->m_wstrCurSpaceName ) ;
	if ( pClassInf != NULL )
	{
		err = CompileFinalizeClassInfo( pClassInf ) ;
	}
	LeaveControlNest() ;
	return	err ;
}

// Union 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileUnion( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwUnion ) )
	{
		return	eslErrSuccess ;
	}
	//
	// 共用体名取得
	//
	ESLError	err ;
	ECSWideString	wstrName = cssLine.GetAToken() ;
	err = VerifyUserSymbol( wstrName ) ;
	if ( err )
	{
		return	err ;
	}
	EWideString	wstrSpaceName = GetCurrentSpaceName() ;
	if ( !wstrSpaceName.IsEmpty() )
	{
		wstrSpaceName += L"::" ;
	}
	EWideString	wstrGlobalName = wstrSpaceName + wstrName ;
	//
	ECSClassInfo *	pClassInf = GetClassInfoAs( wstrGlobalName ) ;
	bool			fAddClassInf = true ;
	bool			fFirstPhase = true ;
	if ( pClassInf != NULL )
	{
		if ( !pClassInf->IsEmpty() )
		{
			if ( !(m_dwImplementFlags & implementDeclaration)
				&& (pClassInf->m_phase == ECSClassInfo::phaseImplement) )
			{
				fFirstPhase = false ;
			}
			else
			{
				return	ESLErrorMsg
					( "既に定義されている共用体を再定義しようとしています。" ) ;
			}
		}
		fAddClassInf = false ;
	}
	else
	{
		pClassInf = new ECSClassInfo ;
		if ( !(m_dwImplementFlags & implementFunction) )
		{
			pClassInf->m_phase = ECSClassInfo::phaseDeclaration ;
		}
	}
	pClassInf->SetAttribute
		( ECSTypeInfo::flagUnion | ECSTypeInfo::flagNakedBuffer ) ;
	pClassInf->SetName( wstrName ) ;
	pClassInf->SetGlobalName( wstrGlobalName ) ;
	//
	if ( fAddClassInf )
	{
		m_pcsxiDst->AddClassInfo( pClassInf ) ;
	}
	if ( m_staTypeName.FindIndex( wstrGlobalName ) < 0 )
	{
		m_staTypeName.Add( wstrGlobalName ) ;
	}
	if ( !fAddClassInf )
	{
		pClassInf->BuildAllClassCast() ;
	}
	//
	// ネスト設定
	//
	EControlNest *	pNest = new EControlNest( m_dwImplementFlags ) ;
	m_rwCtrlType = rwUnion ;
	pNest->m_rwType = rwUnion ;
	pNest->m_wstrName = wstrGlobalName ;
	pNest->m_wstrCurSpaceName = wstrGlobalName ;
	pNest->m_dwProtectedScope = ECSTypeInfo::flagPublic ;
	AddNecessaryUsingNamespace( pNest ) ;
	m_nestCtrl.Add( pNest ) ;
	//
	return	eslErrSuccess ;
}

// EndUnion 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileEndUnion( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwEndUnion ) )
	{
		return	eslErrSuccess ;
	}
	if ( m_rwCtrlType != rwUnion )
	{
		return	ESLErrorMsg
			( "EndUnion 文が Union 文と対応していません。" ) ;
	}
	EControlNest *	pNest = m_nestCtrl.GetLastAt( 0 ) ;
	ESLAssert( pNest != NULL ) ;
	ESLAssert( pNest->m_rwType == m_rwCtrlType ) ;
	//
	ESLError		err = eslErrSuccess ;
	ECSClassInfo *	pClassInf = GetClassInfoAs( pNest->m_wstrCurSpaceName ) ;
	if ( pClassInf != NULL )
	{
		err = CompileFinalizeClassInfo( pClassInf ) ;
	}
	LeaveControlNest() ;
	return	err ;
}

// Public 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompilePublic( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwPublic ) )
	{
		return	eslErrSuccess ;
	}
	if ( (m_rwCtrlType != rwStructure) && (m_rwCtrlType != rwClass) )
	{
		return	ESLErrorMsg
			( "Public 文が Class 外で記述されています。" ) ;
	}
	EControlNest *	pNest = m_nestCtrl.GetLastAt( 0 ) ;
	ESLAssert( pNest != NULL ) ;
	ESLAssert( pNest->m_rwType == m_rwCtrlType ) ;
	//
	pNest->m_dwProtectedScope = ECSTypeInfo::flagPublic ;
	//
	return	eslErrSuccess ;
}

// Protected 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileProtected( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwProtected ) )
	{
		return	eslErrSuccess ;
	}
	if ( (m_rwCtrlType != rwStructure) && (m_rwCtrlType != rwClass) )
	{
		return	ESLErrorMsg
			( "Protected 文が Class 外で記述されています。" ) ;
	}
	EControlNest *	pNest = m_nestCtrl.GetLastAt( 0 ) ;
	ESLAssert( pNest != NULL ) ;
	ESLAssert( pNest->m_rwType == m_rwCtrlType ) ;
	//
	pNest->m_dwProtectedScope = ECSTypeInfo::flagProtected ;
	//
	return	eslErrSuccess ;
}

// Private 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompilePrivate( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwPrivate ) )
	{
		return	eslErrSuccess ;
	}
	if ( (m_rwCtrlType != rwStructure) && (m_rwCtrlType != rwClass) )
	{
		return	ESLErrorMsg
			( "Private 文が Class 外で記述されています。" ) ;
	}
	EControlNest *	pNest = m_nestCtrl.GetLastAt( 0 ) ;
	ESLAssert( pNest != NULL ) ;
	ESLAssert( pNest->m_rwType == m_rwCtrlType ) ;
	//
	pNest->m_dwProtectedScope = ECSTypeInfo::flagPrivate ;
	//
	return	eslErrSuccess ;
}

// Prototype 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompilePrototype( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwPrototype ) )
	{
		return	eslErrSuccess ;
	}
	ECSPrototypeInfo	prototype ;
	ESLError	err = ParsePrototypeDescription( prototype, cssLine ) ;
	if ( err )
	{
		return	err ;
	}
	EControlNest *
		pClassNest = GetMostInnerNest( rwStructure, rwUnion ) ;
	if ( pClassNest == NULL )
	{
		if ( prototype.GetName() != prototype.GetGlobalName() )
		{
			return	ESLErrorMsg
				( "メンバ関数を宣言する場合は Class 構文"
							"ブロック内でなければなりません。" ) ;
		}
		//
		// グローバル関数
		//
		prototype.SetAttribute
			( prototype.GetAttribute() & ~ECSTypeInfo::flagVirtual ) ;
		if ( !prototype.IsNakedCall() )
		{
			if ( m_wstaPrototype.GetAs( prototype.GetName() ) != NULL )
			{
				return	ESLErrorMsg( "既に同名の関数が定義されています。" ) ;
			}
		}
		else
		{
			if ( m_wstaNakedPrototype.GetAs( prototype.GetName() ) != NULL )
			{
				return	ESLErrorMsg( "既に同名の関数が定義されています。" ) ;
			}
		}
		err = VerifyUserSymbol( prototype.GetName() ) ;
		if ( err )
		{
			return	err ;
		}
		if ( (prototype.GetAttribute() & ECSTypeInfo::flagVarArgument)
			&& !(prototype.GetAttribute() & ECSTypeInfo::flagNativeObject) )
		{
			return	ESLErrorMsg( "可変長引数が定義されています。" ) ;
		}
		if ( prototype.GetAttribute() & ECSTypeInfo::flagStatic )
		{
			EWideString	wstrExternName = L"<" ;
			wstrExternName += EWideString( m_strFilePath.GetFileNamePart() ) ;
			wstrExternName += L">::" ;
			wstrExternName += prototype.GetName() ;
			prototype.SetGlobalName( wstrExternName ) ;
		}
		if ( !prototype.IsNakedCall() )
		{
			m_wstaPrototype.Add
				( prototype.GetName(), new ECSPrototypeInfo( prototype ) ) ;
		}
		else
		{
			prototype.MakeNakedAttribute() ;
			m_wstaNakedPrototype.Add
				( prototype.GetName(), new ECSPrototypeInfo( prototype ) ) ;
		}
	}
	else
	{
		if ( prototype.GetName() != prototype.GetGlobalName() )
		{
			SYMBOL_NAMESPACE	snsFuncName ;
			snsFuncName.ParseSymbol( prototype.GetGlobalName() ) ;
			//
			SYMBOL_NAMESPACE	snsClassName ;
			snsClassName.ParseSymbol( pClassNest->m_wstrCurSpaceName ) ;
			//
			for ( ; ; )
			{
				if ( snsClassName.wstrNamespace.IsEmpty() )
				{
					return	ESLErrorMsg
						( "メンバ関数を宣言する場合は Class 構文"
									"ブロック内でなければなりません。" ) ;
				}
				if ( snsClassName.wstrNamespace == snsFuncName.wstrNamespace )
				{
					prototype.SetGlobalName( snsFuncName.wstrName ) ;
					break ;
				}
				snsClassName.ParseSymbol
					( EWideString( snsClassName.wstrNamespace ) ) ;
			}
		}
		//
		// メンバ関数
		//
		err = CompileEffectPrototype( prototype, pClassNest, false ) ;
		if ( err )
		{
			return	err ;
		}
		ECSClassInfo *	pClassInf =
					GetClassInfoAs( pClassNest->m_wstrCurSpaceName ) ;
		ESLAssert( pClassInf != NULL ) ;
		err = pClassInf->AddOverrideFunction( prototype ) ;
		if ( err )
		{
			return	err ;
		}
	}
	EWideString	wstrToken = cssLine.GetAToken( ) ;
	if ( !wstrToken.IsEmpty() )
	{
		m_strErrMsg = "ステートメントの末尾に謎の構文 \'"
				+ EString(wstrToken) + "\' を発見しました。" ;
		err = ESLErrorMsg( m_strErrMsg ) ;
	}
	return	eslErrSuccess ;
}

// Function 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileFunction( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwFunction ) )
	{
		return	eslErrSuccess ;
	}
	const int	iFirstIndex = cssLine.GetIndex() ;
	//
	// プロトタイプ解釈
	//
	ECSPrototypeInfo	prototype ;
	ESLError	err = ParsePrototypeDescription( prototype, cssLine ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// インライン記述判定
	//
	if ( IsInFunctionNest() )
	{
		return	ESLErrorMsg( "関数が関数内に記述されています" ) ;
	}
	EControlNest *
		pClassNest = GetMostInnerNest( rwStructure, rwUnion ) ;
	if ( pClassNest != NULL )
	{
		ECSClassInfo *	pClassInf =
					GetClassInfoAs( pClassNest->m_wstrCurSpaceName ) ;
		if ( pClassInf == NULL )
		{
			return	ESLErrorMsg
				( "内部エラー：インライン関数の"
					"クラス情報を取得出来ませんでした" ) ;
		}
		if ( prototype.GetName() != prototype.GetGlobalName() )
		{
			if ( pClassInf->GetGlobalName().Find( L'<' ) < 0 )
			{
				return	ESLErrorMsg
					( "インライン関数名に名前空間が含まれています" ) ;
			}
		}
		if ( prototype.GetAttribute() & ECSTypeInfo::flagAbstract )
		{
			return	ESLErrorMsg
				( "インライン関数に abstract が指定されています" ) ;
		}
		err = CompileEffectPrototype( prototype, pClassNest, true ) ;
		if ( err )
		{
			return	err ;
		}
		prototype.SetAttribute
				( prototype.GetAttribute() | ECSTypeInfo::flagInline ) ;
		//
		ECSPrototypeInfo * pPrototype =
				pClassInf->GetThisFunctionAs( prototype ) ;
		if ( pPrototype == NULL )
		{
			err = pClassInf->AddOverrideFunction( prototype ) ;
			if ( err )
			{
				return	err ;
			}
		}
		else
		{
			pPrototype->SetAttribute
				( pPrototype->GetAttribute() | ECSTypeInfo::flagInline ) ;
		}
		if ( !pClassNest->m_fCommitBlock )
		{
			EDelayImplementation *	pImplementation = new EDelayImplementation ;
			pImplementation->m_wstrName = prototype.GetGlobalName() ;
			pImplementation->m_rwEndOfImplementation = rwEndFunc ;
			pImplementation->m_pPrevStatementCache = m_pStatementCache ;
			pClassNest->m_lstDelayImplement.Add( pImplementation ) ;
			//
			m_pStatementCache = pImplementation ;
			m_rwEndOfStatementCache = rwEndFunc ;
			//
			cssLine.MoveIndex( iFirstIndex ) ;
			IsCacheStatement( cssLine, rwFunction ) ;
			//
			return	eslErrSuccess ;
		}
		/*
		if ( pClassInf->IsNakedMemoryClass() )
		{
			OutputError
				( "naked クラスブロック内で関数をインライン記述しています",
					m_strFilePath, m_nLineNum ) ;
		}
		*/
	}
	else
	{
		prototype.SetAttribute
			( prototype.GetAttribute() & ~ECSTypeInfo::flagVirtual ) ;
	}
	//
	// プロトタイプ検証
	//
	EWideString	wstrClassName = prototype.GetNameSpace() ;
	ECSPrototypeInfo *	pPrototype ;
	ECSClassInfo *		pClassInf = NULL ;
	if ( wstrClassName.IsEmpty() )
	{
		if ( (pClassNest== NULL)
			&& (prototype.GetAttribute() & ECSTypeInfo::flagStatic) )
		{
			EWideString	wstrExternName = L"<" ;
			wstrExternName += EWideString( m_strFilePath.GetFileNamePart() ) ;
			wstrExternName += L">::" ;
			wstrExternName += prototype.GetName() ;
			prototype.SetGlobalName( wstrExternName ) ;
		}
		if ( !prototype.IsNakedCall() )
		{
			pPrototype = m_wstaPrototype.GetAs( prototype.GetName() ) ;
			if ( pPrototype == NULL )
			{
				m_wstaPrototype.Add
					( prototype.GetName(),
						new ECSPrototypeInfo( prototype ) ) ;
				pPrototype =
					m_wstaPrototype.GetAs( prototype.GetName() ) ;
			}
		}
		else
		{
			prototype.MakeNakedAttribute() ;
			pPrototype = m_wstaNakedPrototype.GetAs( prototype.GetName() ) ;
			if ( pPrototype == NULL )
			{
				m_wstaNakedPrototype.Add
					( prototype.GetName(),
						new ECSPrototypeInfo( prototype ) ) ;
				pPrototype =
					m_wstaNakedPrototype.GetAs( prototype.GetName() ) ;
			}
		}
		if ( (pPrototype != NULL)
			&& !prototype.IsPrototypeEqual( *pPrototype ) )
		{
			return	ESLErrorMsg
				( "関数のプロトタイプが宣言と異なります。" ) ;
		}
	}
	else
	{
		pClassInf = GetClassInfoAs( wstrClassName ) ;
		if ( pClassInf == NULL )
		{
			m_strErrMsg = EString( wstrClassName )
						+ " は定義されていないクラス名です。" ;
			return	ESLErrorMsg( m_strErrMsg ) ;
		}
		err = CompileEffectPrototype( prototype, pClassInf ) ;
		if ( err )
		{
			return	err ;
		}
		pPrototype = pClassInf->GetThisFunctionAs( prototype ) ;
		if ( pPrototype == NULL )
		{
			if ( (m_dwModeFlags & flagStrictStyle)
						&& !pClassInf->IsNamespace() )
			{
				err = OutputWarning1( "定義されていないメンバ関数です。" ) ;
				if ( err )
				{
					return	err ;
				}
			}
			err = pClassInf->AddOverrideFunction( prototype ) ;
			if ( err )
			{
				return	err ;
			}
			pPrototype = pClassInf->GetThisFunctionAs( prototype ) ;
		}
		else
		{
			if ( pPrototype->GetAttribute() & ECSTypeInfo::flagAbstract )
			{
				return	ESLErrorMsg
					( "abstract 関数の実体を定義しようとしています。" ) ;
			}
			const DWORD	dwFuncFlags =
				ECSTypeInfo::flagConstant
					| ECSTypeInfo::flagAbstract | ECSTypeInfo::flagVarArgument ;
			if ( (pPrototype->GetAttribute() & dwFuncFlags)
					!= (prototype.GetAttribute() & dwFuncFlags) )
			{
				return	ESLErrorMsg( "関数修飾子が一致しません。" ) ;
			}
			if ( pPrototype->GetReturnType() != prototype.GetReturnType() )
			{
				return	ESLErrorMsg( "返り値の型が一致しません。" ) ;
			}
		}
		if ( pClassInf->GetAttribute() & ECSTypeInfo::flagNativeObject )
		{
			return	ESLErrorMsg
				( "Native クラスのメンバ関数を実装しようとしています。" ) ;
		}
	}
	if ( pPrototype == NULL )
	{
		return	ESLErrorMsg( "内部エラー：プロトタイプが取得できません。" ) ;
	}
	//
	// 関数エントリゲート生成
	//
	err = CompileFunctionEntryGate
			( pClassInf, pPrototype, prototype.GetArgumentName() ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// 構築関数
	//
	if ( (pClassInf != NULL)
		&& (pClassInf->GetName() == prototype.GetName()) )
	{
		err = CompileCodeConstructor( pClassInf, cssLine ) ;
		if ( err )
		{
			return	err ;
		}
	}
	/*
	if ( m_nestCtrl.GetSize() > 1 )
	{
		return	ESLErrorMsg
			( "Function 構文が他の制御ブロックの中に記述されています。" ) ;
	}
	*/
	EWideString	wstrToken = cssLine.GetAToken( ) ;
	if ( !wstrToken.IsEmpty() )
	{
		m_strErrMsg = "ステートメントの末尾に謎の構文 \'"
				+ EString(wstrToken) + "\' を発見しました。" ;
		return	ESLErrorMsg( m_strErrMsg ) ;
	}
	return	eslErrSuccess ;
}

// EndFunc 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileEndFunc( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwEndFunc ) )
	{
		return	eslErrSuccess ;
	}
	//
	// 直前のラベルローカル領域を削除
	//
	if ( (m_rwCtrlType != rwFunction) && (m_rwCtrlType != rwLabel) )
	{
		return	ESLErrorMsg
			( "EndFunc 文が Function 文と対応していません。" ) ;
	}
	EControlNest *	pNest = m_nestCtrl.GetLastAt( 0 ) ;
	const bool	fControlReturned = pNest->m_fReturned ;
	//
	if ( m_rwCtrlType == rwLabel )
	{
		if ( m_modeNakedCode )
		{
			ESLError	err =
				CompileCodeNakedLocalDestruction( m_nestCtrl.GetLastAt(), 0 ) ;
			if ( err )
			{
				OutputError
					( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
			}
		}
		delete	m_nestCtrl.Pop( ) ;
	}
	pNest = m_nestCtrl.GetLastAt( 0 ) ;
	ESLAssert( pNest != NULL ) ;
	ESLAssert( pNest->m_rwType == rwFunction ) ;
	//
	// naked クラスのデストラクタ判定
	//
	ECSClassInfo *
		pClassInf = GetClassInfoAs( pNest->m_wstrThisSpaceName ) ;
	if ( (pClassInf != NULL)
		&& pClassInf->IsNakedMemoryClass()
		&& (pNest->m_wstrCurSpaceName ==
				pNest->m_wstrThisSpaceName
						+ L"::~" + pClassInf->GetName()) )
	{
		if ( fControlReturned )
		{
			OutputWarning
				( "naked クラスのデストラクタが"
					"途中で return されています",
						m_strFilePath, m_nLineNum ) ;
		}
		if ( pClassInf->IsNeedsNakedClassDestruction( m_modeNakedCode ) )
		{
			if ( m_modeNakedCode )
			{
				m_pcsxi->WriteSakuraMoveRegReg
					( AllocateExpressionRegister(),
						ECSSakura2Processor::regTP ) ;
			}
			else
			{
				CompileCodeLoadRefVariable( csomStack, 0 ) ;
			}
			ESLError	err =
				CompileCodeNakedClassAfterDestruction( pClassInf ) ;
			if ( err )
			{
				OutputError
					( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
			}
			CompileCodeFreeStack() ;
		}
	}
	//
	// ラベル解決
	//
	for ( int i = 0; i < (int) pNest->m_staLabel.GetSize(); i ++ )
	{
		ETaggedElement<ECSWideString,ELabelEntries> *	pElement ;
		pElement = pNest->m_staLabel.GetAt( i ) ;
		ESLAssert( pElement != NULL ) ;
		if ( pElement == NULL )
			continue ;
		ELabelEntries *	pLabel = pElement->GetObject( ) ;
		ESLAssert( pLabel != NULL ) ;
		if ( pLabel == NULL )
			continue ;
		if ( pLabel->m_dwAddr == (DWORD) -1 )
		{
			OutputError
				( m_strErrMsg =
					"ラベル \'" + EString(pElement->Tag())
						+ "\' は定義されていないラベルです。",
					m_strFilePath, m_nLineNum ) ;
		}
		for ( int j = 0; j < (int) pLabel->m_lstRef.GetSize(); j ++ )
		{
			DWORD	dwRefAddr = pLabel->m_lstRef.GetAt( j ) ;
			DWORD *	pdwJumpAddr =
				(DWORD*) m_pcsxi->m_bufImage.
					ModifyBuffer( dwRefAddr, sizeof(DWORD) ) ;
			ESLAssert( pdwJumpAddr != NULL ) ;
			*pdwJumpAddr = pLabel->m_dwAddr - (dwRefAddr + sizeof(DWORD)) ;
		}
	}
	//
	// 関数のローカルフレーム確定
	//
	if ( m_modeNakedCode )
	{
		SDWORD *	pdwLocalSize =
			(SDWORD*) m_pcsxi->m_bufImage.ModifyBuffer
				( pNest->m_addrLocalFrame, sizeof(SDWORD) ) ;
		ESLAssert( pdwLocalSize != NULL ) ;
		if ( pdwLocalSize != NULL )
		{
			*pdwLocalSize = - (SDWORD)  pNest->m_maxLocalSize ;
		}
	}
	//
	// 返り値判定
	//
	if ( !fControlReturned )
	{
		if ( pNest->m_pFuncPrototype != NULL )
		{
			const ECSTypeInfo &	typeReturn =
					pNest->m_pFuncPrototype->GetReturnType() ;
			if ( m_modeNakedCode )
			{
				if ( !typeReturn.IsVoid() )
				{
					LeaveControlNest() ;
					return	ESLErrorMsg
						( "関数の終端に Return 文が見つかりません。" ) ;
				}
				ESLError	err =
					CompileCodeNakedFunctionReturn( typeReturn ) ;
				if ( err )
				{
					LeaveControlNest() ;
					return	err ;
				}
			}
			else if ( typeReturn.IsVoid() )
			{
				m_pcsxi->FenceInstruction() ;
				m_pcsxi->WriteInstructionCode( csicExReturn ) ;
				m_pcsxi->WriteByteCode( 0 ) ;
			}
			else if ( typeReturn.IsAbstractType()
						&& !(m_dwModeFlags & flagStrictStyle) )
			{
				m_pcsxi->FenceInstruction() ;
				CompileImmediateInteger( 0 ) ;
				m_pcsxi->WriteInstructionCode( csicReturn ) ;
				m_pcsxi->WriteByteCode( 0 ) ;
			}
			else
			{
				LeaveControlNest() ;
				return	ESLErrorMsg
					( "関数の終端に Return 文が見つかりません。" ) ;
			}
		}
		else
		{
			if ( m_modeNakedCode )
			{
				LeaveControlNest() ;
				return	ESLErrorMsg
					( "内部エラー：naked モードの関数のプロトタイプがありません" ) ;
			}
			m_pcsxi->FenceInstruction() ;
			CompileImmediateInteger( 0 ) ;
			m_pcsxi->WriteInstructionCode( csicReturn ) ;
			m_pcsxi->WriteByteCode( 0 ) ;
		}
	}
	//
	// 関数終端設定
	//
	m_pcsxi->SetEndOfFunctionAddress
		( pNest->m_wstrCurSpaceName, CompileCodeGetCurrent() ) ;
	//
	// 例外判定
	//
	if ( (pNest->m_pFuncPrototype != NULL) && !m_flagThrowable )
	{
		const int	nThrows = pNest->m_lstThrows.GetSize() ;
		for ( int i = 0; i < nThrows; i ++ )
		{
			if ( !pNest->m_pFuncPrototype->IsThrowable( pNest->m_lstThrows[i] ) )
			{
				OutputError
					( "例外 \'" + EString(pNest->m_lstThrows[i])
							+ "\' は処理されません",
						m_strFilePath, m_nLineNum ) ;
			}
		}
	}
	//
	LeaveControlNest() ;
	return	eslErrSuccess ;
}

// Assembler 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileAssembler( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwAssembler ) )
	{
		return	eslErrSuccess ;
	}
	if ( !m_modeNakedCode )
	{
		OutputWarning0
			( "Assembler が objected モードで指定されています",
									m_strFilePath, m_nLineNum ) ;
	}
	m_pcsxi->FenceInstruction() ;
	//
	EControlNest *	pNest = new EControlNest( m_dwImplementFlags ) ;
	m_rwCtrlType = rwAssembler ;
	pNest->m_rwType = rwAssembler ;
	pNest->m_fAutoEnter = true ;
	m_nestCtrl.Add( pNest ) ;
	//
	pNest->m_pAsm = new ECSAssembler ;
	pNest->m_pAsm->AttachCompiler( this ) ;
	pNest->m_pAsm->AttachOutputImage( m_pcsxi ) ;
	//
	EDelayImplementation *	pdiAsm = new EDelayImplementation ;
	pdiAsm->m_wstrName = L"<asm>" ;
	pdiAsm->m_rwEndOfImplementation = rwInvalid ;
	//
	pNest->m_lstDelayImplement.Add( pdiAsm ) ;
	//
	return	eslErrSuccess ;
}

// EndAssembler 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileEndAssembler( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwEndAssembler ) )
	{
		return	eslErrSuccess ;
	}
	EControlNest *	pNest = m_nestCtrl.GetLastAt( 0 ) ;
	ESLAssert( pNest != NULL ) ;
	ESLAssert( pNest->m_rwType == m_rwCtrlType ) ;
	//
	if ( pNest->m_rwType != rwAssembler )
	{
		return	ESLErrorMsg( "EndAssembler が Assembler 文と対応していません" ) ;
	}
	EDelayImplementation *	pdiAsm = pNest->m_lstDelayImplement.GetAt(0) ;
	if ( pdiAsm != NULL )
	{
		ImplementStatementCache( *pdiAsm, NULL ) ;
	}
	if ( pNest->m_pAsm != NULL )
	{
		pNest->m_pAsm->FinishOutputImage() ;
	}
	LeaveControlNest() ;
	m_pcsxi->FenceInstruction() ;
	//
	return	eslErrSuccess ;
}

// インラインアセンブラ
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileAssembleLine1( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwInvalid, stAssembler1 ) )
	{
		return	eslErrSuccess ;
	}
	EControlNest *	pNest = m_nestCtrl.GetLastAt( 0 ) ;
	ESLAssert( pNest != NULL ) ;
	ESLAssert( pNest->m_rwType == m_rwCtrlType ) ;
	//
	if ( pNest->m_rwType != rwAssembler )
	{
		return	ESLErrorMsg( "アセンブラが Assembler 内でありません" ) ;
	}
	EDelayImplementation *	pdiAsm = pNest->m_lstDelayImplement.GetAt(0) ;
	if ( pdiAsm != NULL )
	{
		ETemplateStatement *	pStatement = new ETemplateStatement ;
		pStatement->m_stType = stAssembler2 ;
		pStatement->m_rwType = rwInvalid ;
		pStatement->m_wstrStatement = cssLine.Middle( cssLine.GetIndex() ) ;
		pStatement->m_strFilePath = m_strFilePath ;
		pStatement->m_nLineNum = m_nLineNum ;
		//
		pdiAsm->m_statements.Add( pStatement ) ;
	}
	if ( pNest->m_pAsm != NULL )
	{
		SSystem::SStringParser	sparsLine ;
		ESLAssert( sizeof(uint16_t) == sizeof(wchar_t) ) ;
		sparsLine.AttachString
			( (const uint16_t *) cssLine.CharPtr() + cssLine.GetIndex() ) ;
		//
		SSystem::SError	err = pNest->m_pAsm->AssembleLine( sparsLine, 0 ) ;
		if ( err )
		{
			m_strErrMsg = (const wchar_t*) pNest->m_pAsm->GetErrorMessage() ;
			return	ESLErrorMsg( m_strErrMsg ) ;
		}
	}
	return	eslErrSuccess ;
}

ESLError ECSCompiler::CompileAssembleLine2( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwInvalid, stAssembler2 ) )
	{
		return	eslErrSuccess ;
	}
	EControlNest *	pNest = m_nestCtrl.GetLastAt( 0 ) ;
	ESLAssert( pNest != NULL ) ;
	ESLAssert( pNest->m_rwType == m_rwCtrlType ) ;
	//
	if ( pNest->m_rwType != rwAssembler )
	{
		return	ESLErrorMsg( "アセンブラが Assembler 内でありません" ) ;
	}
	if ( pNest->m_pAsm != NULL )
	{
		SSystem::SStringParser	sparsLine ;
		ESLAssert( sizeof(uint16_t) == sizeof(wchar_t) ) ;
		sparsLine.AttachString
			( (const uint16_t *) cssLine.CharPtr() + cssLine.GetIndex() ) ;
		//
		SSystem::SError	err = pNest->m_pAsm->AssembleLine( sparsLine, 1 ) ;
		if ( err )
		{
			m_strErrMsg = (const wchar_t*) pNest->m_pAsm->GetErrorMessage() ;
			return	ESLErrorMsg( m_strErrMsg ) ;
		}
	}
	return	eslErrSuccess ;
}

// If 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileIf( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwIf ) )
	{
		return	eslErrSuccess ;
	}
	if ( m_rwCtrlType == rwData )
	{
		return	ESLErrorMsg
			( "Data ブロックの中で If 文が記述されています。" ) ;
	}
	else if ( m_rwCtrlType == rwStructure )
	{
		return	ESLErrorMsg
			( "Structure ブロックの中で If 文が記述されています。" ) ;
	}
	else if ( m_rwCtrlType == rwClass )
	{
		return	ESLErrorMsg
			( "Class ブロックの中で If 文が記述されています。" ) ;
	}
	else if ( m_rwCtrlType == rwEnumerator )
	{
		return	ESLErrorMsg
			( "Enumerator ブロックの中で If 文が記述されています。" ) ;
	}
	EStreamWideString	swsUsage = L" (%x) [&!Then& (%x)] [&!Else& (%x)]\\" ;
	EObjArray<EWideString>	lstParam ;
	EString				strErrMsg ;
	ESLError	err = cssLine.IsMatchUsage( swsUsage, strErrMsg, &lstParam ) ;
	if ( err )
	{
		return	ESLErrorMsg( "If 文の書式が不正です。" ) ;
	}
	//
	EControlNest *	pNest = new EControlNest( m_dwImplementFlags ) ;
	m_rwCtrlType = rwIf ;
	pNest->m_rwType = rwIf ;
	pNest->m_fAutoEnter = true ;
	m_nestCtrl.Add( pNest ) ;
	//
	ECSSourceStream	cssExpr = lstParam[0] ;
	ECSTypeInfo		typeExpr ;
	BeginSakura2Optimize() ;
	err = CompileExpression( typeExpr, cssExpr, 0, NULL ) ;
	if ( err )
	{
		LeaveControlNest() ;
		return	err ;
	}
	err = VerifyTypeBoolean( typeExpr ) ;
	if ( err )
	{
		LeaveControlNest() ;
		return	err ;
	}
	FinishSakura2Optimize() ;
	//
	ESLAssert( !typeExpr.IsLoadedRegister() || (typeExpr.GetLoadedRegister() == GetExpressionRegister()) ) ;
	FreeExpressionTemporaryStack
		( typeExpr.IsLoadedRegister() ? typeExpr.GetLoadedRegister() : -1 ) ;
	m_pcsxi->GetRegisterContext( pNest->m_regContext ) ;
	//
	if ( m_modeNakedCode )
	{
		int	regFirst = 0 ;
		int regCount = 0 ;
		if ( typeExpr.IsLoadedRegister() )
		{
			regFirst = typeExpr.GetLoadedRegister() ;
			regCount = 1 ;
		}
		CompileCodeNakedLocalDestructionSaveRegister( pNest, regFirst, regCount ) ;
		pNest->m_lstLocalName.RemoveAll( ) ;
		pNest->m_lstLocalObj.RemoveAll( ) ;
	}
	//
	pNest->m_dwBeginPos = CompileCodeConditionalJump( false, false ) ;
	//
	if ( !lstParam[1].IsEmpty() || !lstParam[2].IsEmpty() )
	{
		do
		{
			if ( !lstParam[1].IsEmpty() )
			{
				ECSTypeInfo	typeTemp ;
				cssExpr = lstParam[1] ;
				BeginSakura2Optimize() ;
				err = CompileExpression( typeTemp, cssExpr, 0, NULL ) ;
				if ( err )
				{
					break ;
				}
				if ( typeTemp.m_pValue != NULL )
				{
					CompileCodeFreeStack() ;
				}
				FinishSakura2Optimize() ;
				FreeExpressionTemporaryStack() ;
			}
			if ( !lstParam[2].IsEmpty() )
			{
				err = CompileElse( cssLine ) ;
				if ( err )
				{
					break ;
				}
				ECSTypeInfo	typeTemp ;
				cssExpr = lstParam[2] ;
				BeginSakura2Optimize() ;
				err = CompileExpression( typeTemp, cssExpr, 0, NULL ) ;
				if ( err )
				{
					break ;
				}
				if ( typeTemp.m_pValue != NULL )
				{
					CompileCodeFreeStack() ;
				}
				FinishSakura2Optimize() ;
				FreeExpressionTemporaryStack() ;
			}
		}
		while ( false ) ;
		//
		ESLError	errEndIf = CompileEndIf( cssLine ) ;
		return	err ? err : errEndIf ;
	}
	return	eslErrSuccess ;
}

// ElseIf 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileElseIf( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwElseIf ) )
	{
		return	eslErrSuccess ;
	}
	if ( (m_rwCtrlType != rwIf) && (m_rwCtrlType != rwElseIf) )
	{
		return	ESLErrorMsg( "ElseIf 文が If 文と対応していません。" ) ;
	}
	EControlNest *	pNest = m_nestCtrl.GetLastAt( 0 ) ;
	ESLAssert( pNest != NULL ) ;
	ESLAssert( pNest->m_rwType == m_rwCtrlType ) ;
	//
	m_pcsxi->FenceInstruction() ;
	//
	if ( m_modeNakedCode )
	{
		ESLError	err =
			CompileCodeNakedLocalDestruction( pNest, 0 ) ;
		if ( err )
		{
			return	err ;
		}
	}
	else
	{
		if ( pNest->m_lstLocalName.GetSize() > 0 )
		{
			m_pcsxi->WriteInstructionCode( csicLeave ) ;
		}
	}
	pNest->m_lstBreak.Add( CompileCodeJump() ) ;
	pNest->m_lstLocalName.RemoveAll( ) ;
	pNest->m_lstLocalObj.RemoveAll( ) ;
	//
	CompileCodeCommitJumpAddress
		( pNest->m_dwBeginPos, CompileCodeGetCurrent() ) ;
	//
	m_pcsxi->RestoreRegisterContext( pNest->m_regContext ) ;
	//
	ECSTypeInfo	typeExpr ;
	BeginSakura2Optimize() ;
	ESLError	err =
		CompileExpression( typeExpr, cssLine, 0, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	err = VerifyTypeBoolean( typeExpr ) ;
	if ( err )
	{
		return	err ;
	}
	FinishSakura2Optimize() ;
	FreeExpressionTemporaryStack( ECSSakura2Processor::regExpr0 ) ;
	m_pcsxi->GetRegisterContext( pNest->m_regContext ) ;
	//
	if ( m_modeNakedCode )
	{
		CompileCodeNakedLocalDestructionSaveRegister
							( pNest, ECSSakura2Processor::regExpr0 ) ;
	}
	if ( pNest->m_rwType == rwIf )
	{
		pNest->m_fEachReturned = pNest->m_fReturned ;
	}
	else
	{
		pNest->m_fEachReturned =
			(pNest->m_fEachReturned && pNest->m_fReturned) ;
	}
	m_rwCtrlType = rwElseIf ;
	pNest->m_fAutoEnter = true ;
	pNest->m_fReturned = false ;
	pNest->m_rwType = rwElseIf ;
	pNest->m_lstLocalName.RemoveAll( ) ;
	pNest->m_lstLocalObj.RemoveAll( ) ;
	//
	pNest->m_dwBeginPos = CompileCodeConditionalJump( false, false ) ;
	//
	return	eslErrSuccess ;
}

// Else 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileElse( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwElse ) )
	{
		return	eslErrSuccess ;
	}
	if ( (m_rwCtrlType != rwIf) && (m_rwCtrlType != rwElseIf) )
	{
		return	ESLErrorMsg( "Else 文が If 文と対応していません。" ) ;
	}
	EControlNest *	pNest = m_nestCtrl.GetLastAt( 0 ) ;
	ESLAssert( pNest != NULL ) ;
	ESLAssert( pNest->m_rwType == m_rwCtrlType ) ;
	//
	m_pcsxi->FenceInstruction() ;
	//
	if ( m_modeNakedCode )
	{
		ESLError	err =
			CompileCodeNakedLocalDestruction( pNest, 0 ) ;
		if ( err )
		{
			return	err ;
		}
	}
	else
	{
		if ( pNest->m_lstLocalName.GetSize() > 0 )
		{
			m_pcsxi->WriteInstructionCode( csicLeave ) ;
		}
	}
	pNest->m_lstBreak.Add( CompileCodeJump() ) ;
	//
	CompileCodeCommitJumpAddress
		( pNest->m_dwBeginPos, CompileCodeGetCurrent() ) ;
	//
	m_pcsxi->RestoreRegisterContext( pNest->m_regContext ) ;
	//
	if ( pNest->m_rwType == rwIf )
	{
		pNest->m_fEachReturned = pNest->m_fReturned ;
	}
	else
	{
		pNest->m_fEachReturned =
			(pNest->m_fEachReturned && pNest->m_fReturned) ;
	}
	m_rwCtrlType = rwElse ;
	pNest->m_fAutoEnter = true ;
	pNest->m_fReturned = false ;
	pNest->m_rwType = rwElse ;
	pNest->m_lstLocalName.RemoveAll( ) ;
	pNest->m_lstLocalObj.RemoveAll( ) ;
	//
	return	eslErrSuccess ;
}

// EndIf 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileEndIf( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwEndIf ) )
	{
		return	eslErrSuccess ;
	}
	if ( (m_rwCtrlType != rwIf) && (m_rwCtrlType != rwElseIf)
		&& (m_rwCtrlType != rwElse) && (m_rwCtrlType != rwEndIf) )
	{
		return	ESLErrorMsg( "EndIf 文が If 文と対応していません。" ) ;
	}
	EControlNest *	pNest = m_nestCtrl.GetLastAt( 0 ) ;
	ESLAssert( pNest != NULL ) ;
	ESLAssert( pNest->m_rwType == m_rwCtrlType ) ;
	//
	bool	fMergeRegContext = false ;
	if ( m_modeNakedCode )
	{
		ESLError	err =
			CompileCodeNakedLocalDestruction( pNest, 0 ) ;
		if ( err )
		{
			return	err ;
		}
		if ( m_rwCtrlType == rwIf )
		{
			fMergeRegContext =
				m_pcsxi->FlushMergableContextTo( pNest->m_regContext ) ;
		}
		if ( (m_rwCtrlType != rwElse) && !fMergeRegContext )
		{
			if ( pNest->m_regContext.IsModifiedRegister() )
			{
				//
				// if ～ endif 形式で、if 条件不成立時に
				// レジスタのフラッシュ処理
				//
				m_pcsxi->FenceInstruction() ;
				pNest->m_lstBreak.Add( CompileCodeJump() ) ;
				//
				pNest->m_rwType = rwElse ;
				pNest->m_fReturned = false ;
				pNest->m_lstLocalName.RemoveAll( ) ;
				pNest->m_lstLocalObj.RemoveAll( ) ;
				//
				CompileCodeCommitJumpAddress
					( pNest->m_dwBeginPos, CompileCodeGetCurrent() ) ;
				//
				m_pcsxi->RestoreRegisterContext( pNest->m_regContext ) ;
				m_pcsxi->FenceInstruction() ;
			}
		}
		if ( fMergeRegContext )
		{
			m_pcsxi->RestoreRegisterContext( pNest->m_regContext ) ;
		}
		else
		{
			m_pcsxi->FenceInstruction() ;
		}
	}
	else
	{
		m_pcsxi->FenceInstruction() ;
		if ( pNest->m_lstLocalName.GetSize() > 0 )
		{
			m_pcsxi->WriteInstructionCode( csicLeave ) ;
		}
	}
	bool	fAllControlReturned = false ;
	//
	if ( pNest->m_rwType != rwElse )
	{
		CompileCodeCommitJumpAddress
			( pNest->m_dwBeginPos, CompileCodeGetCurrent() ) ;
	}
	else
	{
		fAllControlReturned =
			(pNest->m_fReturned && pNest->m_fEachReturned) ;
	}
	CommitBreakAddressOnNest( pNest, CompileCodeGetCurrent() ) ;
	//
	LeaveControlNest() ;
	//
	if ( fAllControlReturned )
	{
		pNest = m_nestCtrl.GetLastAt( 0 ) ;
		if ( pNest != NULL )
		{
			pNest->m_fReturned = true ;
		}
	}
	return	eslErrSuccess ;
}

// Begin 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileBegin( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwBegin ) )
	{
		return	eslErrSuccess ;
	}
	if ( m_rwCtrlType == rwData )
	{
		return	ESLErrorMsg
			( "Data ブロックの中で Begin 文が記述されています。" ) ;
	}
	if ( m_rwCtrlType == rwStructure )
	{
		return	ESLErrorMsg
			( "Structure ブロックの中で Begin 文が記述されています。" ) ;
	}
	else if ( m_rwCtrlType == rwClass )
	{
		return	ESLErrorMsg
			( "Class ブロックの中で Begin 文が記述されています。" ) ;
	}
	else if ( m_rwCtrlType == rwEnumerator )
	{
		return	ESLErrorMsg
			( "Enumerator ブロックの中で Begin 文が記述されています。" ) ;
	}
	ECSWideString	wstrName( L"" ) ;
	if ( !cssLine.DisregardSpace() )
	{
		ECSObject *	pObjName ;
		ESLError	err = CalculateExpression( pObjName, cssLine ) ;
		if ( err )
		{
			return	err ;
		}
		if ( pObjName->m_vtType != csvtString )
		{
			return	ESLErrorMsg( "Begin 名前空間名が文字列ではありません。" ) ;
		}
		wstrName = ((ECSString*)pObjName)->m_varStr ;
	}
	//
	EControlNest *	pNest = new EControlNest( m_dwImplementFlags ) ;
	m_rwCtrlType = rwBegin ;
	pNest->m_rwType = rwBegin ;
	m_nestCtrl.Add( pNest ) ;
	//
	if ( !m_modeNakedCode )
	{
		DWORD	dwDummy = 0 ;
		m_pcsxi->WriteInstructionCode( csicEnter ) ;
		m_pcsxi->WriteConstantString( wstrName ) ;
		m_pcsxi->WriteCodeData( &dwDummy, sizeof(dwDummy) ) ;
	}
	return	eslErrSuccess ;
}

// End 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileEnd( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwEnd ) )
	{
		return	eslErrSuccess ;
	}
	if ( m_rwCtrlType != rwBegin )
	{
		return	ESLErrorMsg( "End 文が Begin 文と対応していません。" ) ;
	}
	EControlNest *	pNest = m_nestCtrl.GetLastAt( 0 ) ;
	bool	fControlReturned =
				((pNest != NULL) && (pNest->m_fReturned)) ;
	ESLError	err = eslErrSuccess ;
	//
	if ( m_modeNakedCode )
	{
		if ( pNest != NULL )
		{
			err = CompileCodeNakedLocalDestruction( pNest, 0 ) ;
		}
	}
	else
	{
		m_pcsxi->WriteInstructionCode( csicLeave ) ;
	}
	LeaveControlNest() ;
	//
	if ( fControlReturned )
	{
		pNest = m_nestCtrl.GetLastAt( 0 ) ;
		if ( pNest != NULL )
		{
			pNest->m_fReturned = true ;
		}
	}
	return	err ;
}

// Break 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileBreak( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwBreak ) )
	{
		return	eslErrSuccess ;
	}
	//
	// 最も内側の反復ブロックを取得
	//
	int		nNestCount ;
	EControlNest *	pNest =
		GetMostInnerNest( rwLoopFirst, rwLoopLast, &nNestCount ) ;
	if ( pNest == NULL )
	{
		return	ESLErrorMsg( "Break 文に対応する反復文が見つかりません。" ) ;
	}
	//
	// ジャンプコード出力
	//
	ECSWideString	wstrToken = cssLine.GetAToken( ) ;
	int		i, nLeaveCount = 0 ;
	if ( m_modeNakedCode )
	{
		nLeaveCount = nNestCount + 1 ;
	}
	else
	{
		for ( i = 0; i <= nNestCount; i ++ )
		{
			EControlNest *	pBreakNest = m_nestCtrl.GetLastAt( i ) ;
			nLeaveCount += pBreakNest->ShuldBreakLocalBlock( ) ;
		}
	}
	bool	fNoBreakJump = false ;
	DWORD	dwNoBreakRef ;
	if ( !wstrToken.IsEmpty() )
	{
		if ( wstrToken.CompareNoCase( L"If" ) )
		{
			return	ESLErrorMsg
				( "Break 文に不正な構文が指定されています。" ) ;
		}
		ECSTypeInfo	typeExpr ;
		BeginSakura2Optimize() ;
		ESLError	err = CompileExpression( typeExpr, cssLine, 0, NULL ) ;
		if ( err )
		{
			return	err ;
		}
		err = VerifyTypeBoolean( typeExpr ) ;
		if ( err )
		{
			return	err ;
		}
		FinishSakura2Optimize() ;
		FreeExpressionTemporaryStack( ECSSakura2Processor::regExpr0 ) ;
		fNoBreakJump = true ;
		dwNoBreakRef = CompileCodeConditionalJump( false, false ) ;
	}
	m_pcsxi->FenceInstruction() ;
	//
	for ( i = 0; i < nLeaveCount; i ++ )
	{
		if ( m_modeNakedCode )
		{
			EControlNest *	pNestLocal = m_nestCtrl.GetLastAt( i ) ;
			if ( pNestLocal != NULL )
			{
				ESLError	err =
					CompileCodeNakedLocalDestruction( pNestLocal, i ) ;
				if ( err )
				{
					OutputError
						( GetESLErrorMsg( err ),
								m_strFilePath, m_nLineNum ) ;
				}
			}
		}
		else
		{
			m_pcsxi->WriteInstructionCode( csicLeave ) ;
		}
	}
	pNest->m_lstBreak.Add( CompileCodeJump() ) ;
	//
	if ( fNoBreakJump )
	{
		CompileCodeCommitJumpAddress
				( dwNoBreakRef, CompileCodeGetCurrent() ) ;
	}
	return	eslErrSuccess ;
}

// Continue 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileContinue( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwContinue ) )
	{
		return	eslErrSuccess ;
	}
	//
	// 最も内側の反復ブロックを取得
	//
	int		nNestCount ;
	EControlNest *	pNest =
		GetMostInnerNest( rwLoopFirst, rwLoopLast, &nNestCount ) ;
	if ( pNest == NULL )
	{
		return	ESLErrorMsg
			( "Continue 文に対応する反復文が見つかりません。" ) ;
	}
	if ( pNest->m_rwType == rwSwitch )
	{
		return	ESLErrorMsg
			( "Switch ブロック内で Continue 文が記述されています。" ) ;
	}
	//
	// ジャンプコード出力
	//
	ECSWideString	wstrToken = cssLine.GetAToken( ) ;
	int		i, nLeaveCount = 0 ;
	if ( m_modeNakedCode )
	{
		nLeaveCount = nNestCount + 1 ;
	}
	else
	{
		for ( i = 0; i <= nNestCount; i ++ )
		{
			EControlNest *	pBreakNest = m_nestCtrl.GetLastAt( i ) ;
			nLeaveCount += pBreakNest->ShuldBreakLocalBlock( false ) ;
		}
	}
	bool	fNoContinue = false ;
	DWORD	dwNoContinueRef ;
	if ( !wstrToken.IsEmpty() )
	{
		if ( wstrToken.CompareNoCase( L"If" ) )
		{
			return	ESLErrorMsg
				( "Continue 文に不正な構文が指定されています。" ) ;
		}
		ECSTypeInfo	typeExpr ;
		BeginSakura2Optimize() ;
		ESLError	err = CompileExpression( typeExpr, cssLine, 0, NULL ) ;
		if ( err )
		{
			return	err ;
		}
		err = VerifyTypeBoolean( typeExpr ) ;
		if ( err )
		{
			return	err ;
		}
		FinishSakura2Optimize() ;
		FreeExpressionTemporaryStack( ECSSakura2Processor::regExpr0 ) ;
		fNoContinue = true ;
		dwNoContinueRef = CompileCodeConditionalJump( false, false ) ;
	}
	m_pcsxi->FenceInstruction() ;
	//
	for ( i = 0; i < nLeaveCount; i ++ )
	{
		if ( m_modeNakedCode )
		{
			EControlNest *	pNestLocal = m_nestCtrl.GetLastAt( i ) ;
			if ( pNestLocal != NULL )
			{
				ESLError	err =
					CompileCodeNakedLocalDestruction( pNestLocal, i ) ;
				if ( err )
				{
					OutputError
						( GetESLErrorMsg( err ),
								m_strFilePath, m_nLineNum ) ;
				}
			}
		}
		else
		{
			m_pcsxi->WriteInstructionCode( csicLeave ) ;
		}
	}
	CompileCodeJump( pNest->m_dwBeginPos ) ;
	//
	if ( fNoContinue )
	{
		CompileCodeCommitJumpAddress
				( dwNoContinueRef, CompileCodeGetCurrent() ) ;
	}
	return	eslErrSuccess ;
}

// Return 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileReturn( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwReturn ) )
	{
		return	eslErrSuccess ;
	}
	//
	// 対応する Function ブロックを検索
	//
	EControlNest *	pNest = GetMostInnerNest( rwFunction, rwFunction ) ;
	if ( pNest == NULL )
	{
		return	ESLErrorMsg
			( "Return 文に対応する Function 文が見つかりません。" ) ;
	}
	//
	// Return Leave 文判定
	//
	bool			fLeaveReturn = false ;
	int				nCode = 0 ;
	int				nIndex = cssLine.GetIndex( ) ;
	ECSWideString	wstrToken = cssLine.GetAToken( ) ;
	if ( !CompareReservedWord( L"Leave", wstrToken ) )
	{
		if ( m_modeNakedCode )
		{
			return	ESLErrorMsg
				( "naked モードで return leave が指定されています" ) ;
		}
		fLeaveReturn = true ;
		nCode = 2 ;
	}
	else
	{
		cssLine.MoveIndex( nIndex ) ;
	}
	//
	// 返り値設定
	//
	if ( !cssLine.DisregardSpace() )
	{
		ECSTypeInfo	typeExpr ;
		BeginSakura2Optimize() ;
		ESLError	err =
			CompileExpression( typeExpr, cssLine, 0, NULL ) ;
//		FinishSakura2Optimize() ;
		//
		if ( m_modeNakedCode )
		{
			if ( typeExpr.IsLoadedRegister() )
			{
				ESLAssert( typeExpr.GetLoadedRegister() == GetExpressionRegister() ) ;
				FreeExpressionTemporaryStack
					( GetExpressionRegister() /*ECSSakura2Processor::regExpr0*/ ) ;
			}
			else
			{
				FreeExpressionTemporaryStack() ;
			}
		}
		if ( err )
		{
			return	err ;
		}
		if ( pNest->m_pFuncPrototype != NULL )
		{
			const ECSTypeInfo &	typeReturn =
					pNest->m_pFuncPrototype->GetReturnType() ;
			if ( !typeReturn.IsAbstractType()
					|| (m_dwModeFlags & flagStrictStyle) )
			{
				if ( m_modeNakedCode
					&& !typeReturn.IsNakedPrimitiveDataType() )
				{
					//
					// naked モードで関数返り値を構築
					//
					MakeCommitValueToNakedRegister( typeExpr ) ;
					ESLAssert( !m_modeNakedCode
							|| (typeExpr.GetLoadedRegister()
										== GetExpressionRegister()) ) ;
					if ( m_modeNakedCode
						&& (typeExpr.GetLoadedRegister() != GetExpressionRegister()) )
					{
						OutputError
							( "内部エラー：リターン文でレジスタの割り当てが異常です",
														m_strFilePath, m_nLineNum ) ;
					}
					//
					const ECSClassInfo *
							pRetClassInf = typeReturn.GetClassInfo() ;
					ECSTypeInfo	typeRetObjPtr ;
					int		iRetObjPtr =
						IsLocalVariableName( L"<return>", &typeRetObjPtr ) ;
					//
					if ( (pRetClassInf != NULL) && (iRetObjPtr >= 0) )
					{
						EPtrObjArray<ECSTypeInfo>			lstArg ;
						ECSClassInfo::ListMemberFunction	lstFunc ;
						lstArg.Add( &typeExpr ) ;
						if ( pRetClassInf->SearchFunctinoAs
								( lstFunc, pRetClassInf->GetName(),
									lstArg, 0, true, false, m_modeNakedCode ) )
						{
							ECSTypeInfo *	pArgType = lstFunc[0].GetArgumentAt( 0 ) ;
							if ( pArgType != NULL )
							{
								ECSTypeInfo	typeResult ;
								err = CompileTypeCast
									( typeResult, *pArgType,
										typeExpr, 0, ECSTypeInfo::flagPublic ) ;
								if ( err )
								{
									return	err ;
								}
								typeExpr = typeResult ;
							}
						}
						int	regArg = AllocateExpressionRegister() ;
						int	regThis = GetExpressionRegister( 1 ) ;
						//
						m_pcsxi->WriteSakuraMoveRegReg( regArg, regThis ) ;
						//
						int	regLoaded = regThis ;
						WriteSakuraLoadMemory( regLoaded, typeRetObjPtr, true ) ;
						if ( regLoaded != regThis )
						{
							m_pcsxi->WriteSakuraMoveRegReg( regThis, regLoaded ) ;
						}
						err = CompileCallObjectConstructor
											( *pRetClassInf, lstArg ) ;
						//
						int	regReturn = AllocateExpressionRegister() ;
						regLoaded = regReturn ;
						WriteSakuraLoadMemory( regLoaded, typeRetObjPtr, true ) ;
						if ( regLoaded != regReturn )
						{
							m_pcsxi->WriteSakuraMoveRegReg( regReturn, regLoaded ) ;
						}
					}
				}
				else
				{
					//
					// 関数返り値をキャスト
					//
//					BeginSakura2Optimize() ;
					//
					ECSTypeInfo	typeCast ;
					err = CompileTypeCast
						( typeCast, typeReturn,
							typeExpr, 0, ECSTypeInfo::flagPublic ) ;
					//
					MakeCommitValueToNakedRegister( typeCast ) ;
					ESLAssert( !m_modeNakedCode
							|| (typeCast.GetLoadedRegister()
										== GetExpressionRegister()) ) ;
					if ( m_modeNakedCode
						&& (typeCast.GetLoadedRegister() != GetExpressionRegister()) )
					{
						OutputError
							( "内部エラー：リターン文でレジスタの割り当てが異常です",
														m_strFilePath, m_nLineNum ) ;
					}
//					FinishSakura2Optimize() ;
				}
				if ( err )
				{
					return	err ;
				}
			}
			else if ( typeExpr.IsTypeReference() )
			{
				if ( m_modeNakedCode )
				{
					return	ESLErrorMsg
						( "naked モードで関数の返り値が純粋 Reference 型です" ) ;
				}
				else
				{
					if ( typeExpr.IsTypeInteger()
						|| typeExpr.IsTypeReal()
						|| typeExpr.IsTypeString() )
					{
						m_pcsxi->WriteInstructionCode( csicExUniOperate ) ;
						m_pcsxi->WriteExUniOperatorTypeCode( csxuotDuplicate ) ;
					}
				}
			}
		}
		else if ( typeExpr.IsVoid() )
		{
			return	ESLErrorMsg( "void が Return されています。" ) ;
		}
		FinishSakura2Optimize() ;
	}
	else
	{
		if ( pNest->m_pFuncPrototype != NULL )
		{
			const ECSTypeInfo &	typeReturn =
					pNest->m_pFuncPrototype->GetReturnType() ;
			if ( typeReturn.IsAbstractType()
				&& !(m_dwModeFlags & flagStrictStyle) && !m_modeNakedCode )
			{
				CompileImmediateInteger( 0 ) ;
			}
			else if ( !typeReturn.IsVoid() )
			{
				return	ESLErrorMsg( "返り値が記述されていません。" ) ;
			}
		}
		else
		{
			CompileImmediateInteger( 0 ) ;
		}
	}
//	m_pcsxi->ResetAllRegisterAssigns() ;
//	m_pcsxi->FenceInstruction() ;
	//
	if ( !fLeaveReturn && (pNest->m_pFuncPrototype != NULL) )
	{
		const ECSTypeInfo	typeRetrun =
				pNest->m_pFuncPrototype->GetReturnType() ;
		if ( m_modeNakedCode )
		{
			ESLError	err = CompileCodeNakedFunctionReturn( typeRetrun ) ;
			if ( err )
			{
				return	err ;
			}
		}
		else
		{
			m_pcsxi->WriteInstructionCode( csicExReturn ) ;
			//
			if ( typeRetrun.IsVoid() )
			{
				m_pcsxi->WriteByteCode( 0 ) ;
			}
			else
			{
				m_pcsxi->WriteByteCode( 1 ) ;
			}
		}
	}
	else if ( m_modeNakedCode )
	{
		return	ESLErrorMsg( "naked モードでルーズなリターン文です" ) ;
	}
	else
	{
		m_pcsxi->WriteInstructionCode( csicReturn ) ;
		m_pcsxi->WriteByteCode( nCode ) ;
	}
	m_pcsxi->ResetAllRegisterAssigns() ;
	m_pcsxi->FenceInstruction() ;
	//
	// 制御ブロックにリターンフラグ設定
	//
	pNest = m_nestCtrl.GetLastAt( 0 ) ;
	if ( pNest != NULL )
	{
		pNest->m_fReturned = true ;
	}
	return	eslErrSuccess ;
}

// Goto 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileGoto( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwGoto ) )
	{
		return	eslErrSuccess ;
	}
	//
	// Function ブロックを取得
	//
	int		nNestCount = -1, nFuncNestCount = 0 ;
	EControlNest *	pNest =
		GetMostInnerNest( rwLabel, rwLabel, &nNestCount ) ;
	EControlNest *	pFuncNest =
		GetMostInnerNest( rwFunction, rwFunction, &nFuncNestCount ) ;
	if ( (pNest == NULL) || (pFuncNest == NULL) )
	{
		pNest = pFuncNest ;
		nNestCount = nFuncNestCount - 1 ;
		if ( pNest == NULL )
		{
			return	ESLErrorMsg
				( "Goto 文は Function ブロック内でしか使用できません。" ) ;
		}
//		nNestCount = -1 ;
	}
	//
	// ラベル検索
	//
	ECSWideString	wstrLabel = cssLine.GetAToken( ) ;
	ELabelEntries *	pLabel = pFuncNest->m_staLabel.GetAs( wstrLabel ) ;
	if ( pLabel == NULL )
	{
		pLabel = new ELabelEntries ;
		pLabel->m_dwAddr = (DWORD) (-1) ;
		pFuncNest->m_staLabel.SetAs( wstrLabel, pLabel ) ;
	}
	//
	// ジャンプコード出力
	//
	m_pcsxi->FenceInstruction() ;
	//
	pNest->m_fGotoOccured = true ;
	for ( int i = 0; i <= nNestCount; i ++ )
	{
		EControlNest *	pBreakNest = m_nestCtrl.GetLastAt( i ) ;
		if ( m_modeNakedCode )
		{
			ESLError	err =
				CompileCodeNakedLocalDestruction( pBreakNest, i ) ;
			if ( err )
			{
				OutputError
					( GetESLErrorMsg( err ),
							m_strFilePath, m_nLineNum ) ;
			}
		}
		else
		{
			int	nLeaveCount = pBreakNest->ShuldBreakLocalBlock( ) ;
			for ( int j = 0; j < nLeaveCount; j ++ )
			{
				m_pcsxi->WriteInstructionCode( csicLeave ) ;
			}
		}
	}
	pLabel->m_lstRef.Add( CompileCodeJump() ) ;
	//
	return	eslErrSuccess ;
}

// Label 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileLabel( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwLabel ) )
	{
		return	eslErrSuccess ;
	}
	//
	// Function ブロックを取得
	//
	EControlNest *	pNestFunc = GetMostInnerNest( rwFunction, rwFunction ) ;
	if ( pNestFunc == NULL )
	{
		return	ESLErrorMsg
			( "Label 文は Function ブロック内でしか使用できません。" ) ;
	}
	if ( (m_rwCtrlType != rwFunction) && (m_rwCtrlType != rwLabel) )
	{
		return	ESLErrorMsg
			( "Label 文が制御ブロックの中に記述されています。" ) ;
	}
	//
	// ラベル名取得
	//
	bool	fPublicLabel = false ;
	int		nTokenType ;
	ECSWideString	wstrLabelName = cssLine.GetAToken( &nTokenType ) ;
	if ( !wstrLabelName.CompareNoCase( L"Public" ) )
	{
		wstrLabelName = cssLine.GetAToken( &nTokenType ) ;
		fPublicLabel = true ;
	}
	else if ( !wstrLabelName.CompareNoCase( L"Private" ) )
	{
		wstrLabelName = cssLine.GetAToken( &nTokenType ) ;
	}
	if ( nTokenType != 0 )
	{
		return	ESLErrorMsg( "ラベル名に不正な文字が使用されています。" ) ;
	}
	//
	// 直前のラベルローカル領域を削除
	//
	m_pcsxi->FenceInstruction() ;
	//
	if ( m_rwCtrlType == rwLabel )
	{
		if ( m_modeNakedCode )
		{
			ESLError	err =
				CompileCodeNakedLocalDestruction( m_nestCtrl.GetLastAt(), 0 ) ;
			if ( err )
			{
				OutputError
					( GetESLErrorMsg( err ),
							m_strFilePath, m_nLineNum ) ;
			}
		}
		else
		{
			m_pcsxi->WriteInstructionCode( csicLeave ) ;
		}
		delete	m_nestCtrl.Pop( ) ;
		m_rwCtrlType = rwFunction ;
	}
	//
	// ラベル名を大域関数名に変換
	//
	if ( fPublicLabel )
	{
		if ( m_modeNakedCode )
		{
			OutputError
				( "naked モードで public ラベルが"
					"定義されています", m_strFilePath, m_nLineNum ) ;
		}
		//
		// 関数エントリ登録
		//
		int		i ;
		DWORD	dwJumpAddr ;
		dwJumpAddr = CompileCodeJump( ) ;
		//
		ECSWideString	wstrFuncName =
			wstrLabelName + L"@"
				+ (const wchar_t *) pNestFunc->m_wstrName ;
		m_pcsxi->AddFunctionEntry
			( wstrFuncName, CompileCodeGetCurrent() ) ;
		//
		// 関数の引数設定
		//
		m_pcsxi->WriteInstructionCode( csicEnter ) ;
		m_pcsxi->WriteConstantString( wstrFuncName ) ;
		m_pcsxi->WriteCodeData
			( &(pNestFunc->m_nArgCount), sizeof(long int) ) ;
		for ( i = 0; i < pNestFunc->m_nArgCount; i ++ )
		{
			ECSTypeInfo *	pType = pNestFunc->m_lstLocalObj.GetAt( i ) ;
			ESLAssert( pType != NULL ) ;
			ECSObject *	pObj = pType->m_pValue ;
			ESLAssert( pObj != NULL ) ;
			ECSStructure *	pStruct = ESLTypeCast<ECSStructure>( pObj ) ;
			if ( pStruct != NULL )
			{
				m_pcsxi->WriteVariableTypeCode( csvtObject ) ;
				m_pcsxi->WriteConstantString
					( ECSWideString(pStruct->GetTypeName()) ) ;
			}
			else
			{
				m_pcsxi->WriteVariableTypeCode( pObj->m_vtType ) ;
			}
			const wchar_t *	pwszName = pNestFunc->m_lstLocalName.GetAt( i ) ;
			ESLAssert( pwszName != NULL ) ;
			m_pcsxi->WriteConstantString( pwszName ) ;
		}
		//
		// 関数の共通局所変数設定
		//
		for ( i = pNestFunc->m_nArgCount;
				i < (int) pNestFunc->m_lstLocalObj.GetSize(); i ++ )
		{
			m_pcsxi->WriteInstructionCode( csicNew ) ;
			m_pcsxi->WriteObjectModeCode( csomStack ) ;
			//
			ECSTypeInfo *	pType = pNestFunc->m_lstLocalObj.GetAt( i ) ;
			ESLAssert( pType != NULL ) ;
			ECSObject *	pObj = pType->m_pValue ;
			ESLAssert( pObj != NULL ) ;
			ECSStructure *	pStruct = ESLTypeCast<ECSStructure>( pObj ) ;
			if ( pStruct != NULL )
			{
				m_pcsxi->WriteVariableTypeCode( csvtObject ) ;
				m_pcsxi->WriteConstantString
					( ECSWideString(pStruct->GetTypeName()) ) ;
			}
			else
			{
				m_pcsxi->WriteVariableTypeCode( pObj->m_vtType ) ;
			}
			const wchar_t *	pwszName = pNestFunc->m_lstLocalName.GetAt( i ) ;
			ESLAssert( pwszName != NULL ) ;
			m_pcsxi->WriteConstantString( ECSWideString( pwszName ) ) ;
			//
			bool	fStore = true ;
			switch ( pObj->m_vtType )
			{
			case	csvtInteger:
				CompileImmediateInteger
						( ((ECSInteger*)pObj)->GetValue() ) ;
				break ;
			case	csvtReal:
				CompileImmediateReal( ((ECSReal*)pObj)->m_varReal ) ;
				break ;
			case	csvtString:
				CompileImmediateString( ((ECSString*)pObj)->m_varStr ) ;
				break ;
			default:
				fStore = false ;
				break ;
			}
			if ( fStore )
			{
				CompileCodeStore( csotNop ) ;
			}
		}
		//
		CompileCodeCommitJumpAddress
			( dwJumpAddr, CompileCodeGetCurrent() ) ;
	}
	//
	// ラベル設定
	//
	ELabelEntries *	pLabel = pNestFunc->m_staLabel.GetAs( wstrLabelName ) ;
	if ( pLabel != NULL )
	{
		if ( pLabel->m_dwAddr != (DWORD) -1 )
		{
			m_strErrMsg = "ラベル \'"
				+ EString(wstrLabelName) + "\' は既に定義されています。" ;
			return	ESLErrorMsg( m_strErrMsg ) ;
		}
	}
	else
	{
		pLabel = new ELabelEntries ;
		pNestFunc->m_staLabel.SetAs( wstrLabelName, pLabel ) ;
	}
	pLabel->m_dwAddr = CompileCodeGetCurrent( ) ;
	//
	EControlNest *	pNest = new EControlNest( m_dwImplementFlags ) ;
	m_rwCtrlType = rwLabel ;
	pNest->m_rwType = rwLabel ;
	pNest->m_wstrName = wstrLabelName ;
	pNest->m_fGotoOccured = false ;
	pNest->m_dwBeginPos = pLabel->m_dwAddr ;
	m_nestCtrl.Add( pNest ) ;
	//
	// 名前空間作成
	//
	m_pcsxi->FenceInstruction() ;
	//
	if ( !m_modeNakedCode )
	{
		DWORD	dwDummy = 0 ;
		m_pcsxi->WriteInstructionCode( csicEnter ) ;
		m_pcsxi->WriteConstantString( ECSWideString(L"") ) ;
		m_pcsxi->WriteCodeData( &dwDummy, sizeof(dwDummy) ) ;
	}
	return	eslErrSuccess ;
}

// Try 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileTry( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwTry ) )
	{
		return	eslErrSuccess ;
	}
	if ( m_rwCtrlType == rwData )
	{
		return	ESLErrorMsg
			( "Try 文は Data ブロックの中では記述できません。" ) ;
	}
	else if ( m_rwCtrlType == rwStructure )
	{
		return	ESLErrorMsg
			( "Structure ブロックの中で Try 文が記述されています。" ) ;
	}
	else if ( m_rwCtrlType == rwClass )
	{
		return	ESLErrorMsg
			( "Class ブロックの中で Try 文が記述されています。" ) ;
	}
	else if ( m_rwCtrlType == rwNamespace )
	{
		return	ESLErrorMsg
			( "Namespace ブロックの中で Try 文が記述されています。" ) ;
	}
	else if ( m_rwCtrlType == rwUnion )
	{
		return	ESLErrorMsg
			( "Union ブロックの中で Try 文が記述されています。" ) ;
	}
	else if ( m_rwCtrlType == rwEnumerator )
	{
		return	ESLErrorMsg
			( "Enumerator ブロックの中で Try 文が記述されています。" ) ;
	}
	//
	// Try ブロック作成
	//
	EControlNest *	pNest = new EControlNest( m_dwImplementFlags ) ;
	m_rwCtrlType = rwTry ;
	pNest->m_rwType = rwTry ;
	pNest->m_dwBeginPos = CompileCodeGetCurrent( ) ;
	m_nestCtrl.Add( pNest ) ;
	//
	// 名前空間を生成
	//
	m_pcsxi->FenceInstruction() ;
	if ( m_modeNakedCode )
	{
		//
		// naked モード構造化例外チェインを追加
		//
		ECSExecutionImageCompiler *	pcsxiHandler = NULL ;
		EWideString	wstrCurSpaceName = GetCurrentSpaceName() ;
		for ( int i = 1; ; i ++ )
		{
			pNest->m_wstrExceptionHandler = wstrCurSpaceName ;
			pNest->m_wstrExceptionHandler += L"::<try_handler@" ;
			pNest->m_wstrExceptionHandler += EWideString( i ) ;
			pNest->m_wstrExceptionHandler += L">" ;
			//
			pcsxiHandler =
				CreateTemporaryInlineFunction
					( pNest->m_wstrExceptionHandler, ECSTypeInfo::flagNakedCall ) ;
			if ( pcsxiHandler != NULL )
			{
				pNest->m_wstrCatchEnterLabel = wstrCurSpaceName ;
				pNest->m_wstrCatchEnterLabel += L"::<try_catch@" ;
				pNest->m_wstrCatchEnterLabel += EWideString( i ) ;
				pNest->m_wstrCatchEnterLabel += L">" ;
				pNest->m_wstrTryExitLabel = wstrCurSpaceName ;
				pNest->m_wstrTryExitLabel += L"::<try_final_exit@" ;
				pNest->m_wstrTryExitLabel += EWideString( i ) ;
				pNest->m_wstrTryExitLabel += L">" ;
				break ;
			}
		}
		pNest->m_pcsxiExceptionHandler = pcsxiHandler ;
		//
		const int	regHandler = AllocateExpressionRegister() ;
		m_pcsxi->WriteSakuraLoadInt64_FuncPtr
					( regHandler, pNest->m_wstrExceptionHandler ) ;
		m_pcsxi->WriteSakuraPushRegsImm8( ECSSakura2Processor::regBP, 2 ) ;
		m_pcsxi->WriteSakuraPushRegsImm8( ECSSakura2Processor::regXP, 2 ) ;
		m_pcsxi->WriteSakuraPushReg( regHandler ) ;
		m_pcsxi->WriteSakuraMoveRegReg
			( ECSSakura2Processor::regYP, ECSSakura2Processor::regIntZero ) ;
		m_pcsxi->WriteSakuraMoveRegReg
			( ECSSakura2Processor::regXP, ECSSakura2Processor::regSP ) ;
		FreeExpressionRegister() ;
	}
	else
	{
		//
		// object モード名前空間を生成
		//
		DWORD	dwDummy = -1 ;
		DWORD	dwCatchAddr = 0 ;
		m_pcsxi->WriteInstructionCode( csicEnter ) ;
		m_pcsxi->WriteConstantString( ECSWideString(L"@TRY") ) ;
		m_pcsxi->WriteCodeData( &dwDummy, sizeof(dwDummy) ) ;
		m_pcsxi->WriteByteCode( 0 ) ;
		pNest->m_lstBreak.Add( CompileCodeGetCurrent() ) ;
		m_pcsxi->WriteCodeData( &dwCatchAddr, sizeof(dwCatchAddr) ) ;
	}
	return	eslErrSuccess ;
}

// Catch 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileCatch( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwCatch ) )
	{
		return	eslErrSuccess ;
	}
	if ( m_modeNakedCode )
	{
		if ( (m_rwCtrlType != rwTry)
			&& (m_rwCtrlType != rwCatch) )
		{
			return	ESLErrorMsg
				( "Catch 文に対応する Try 文がありません。" ) ;
		}
		EControlNest *	pNest = m_nestCtrl.GetLastAt( 0 ) ;
		ESLAssert( pNest != NULL ) ;
		ESLAssert( pNest->m_rwType == m_rwCtrlType ) ;
		//
		bool	fTryBlock = (m_rwCtrlType == rwTry) ;
		EWideString	wstrExceptionHandler = pNest->m_wstrExceptionHandler ;
		EWideString	wstrCatchEnterLabel = pNest->m_wstrCatchEnterLabel ;
		EWideString	wstrTryExitLabel = pNest->m_wstrTryExitLabel ;
		int			nCatchCount = pNest->m_nCatchCount ;
		DWORD		dwBiasTryLocalSize = pNest->m_dwBiasTryLocalSize ;
		ECSExecutionImageCompiler *
					pcsxiExceptionHandler = pNest->m_pcsxiExceptionHandler ;
		if ( wstrExceptionHandler.IsEmpty()
				|| (pcsxiExceptionHandler == NULL) )
		{
			return	ESLErrorMsg
				( "例外エラーハンドラを取得できませんでした" ) ;
		}
		//
		// Try ブロックの脱出処理
		//
		ESLError	err ;
		m_pcsxi->FenceInstruction() ;
		err = CompileCodeNakedLocalDestruction( pNest, 0 ) ;
		if ( err )
		{
			OutputError
				( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
		}
		LeaveControlNest() ;
		//
		if ( fTryBlock )
		{
			m_pcsxi->WriteSakuraAddRegRegImm32
				( ECSSakura2Processor::regSP,
					ECSSakura2Processor::regXP, 8 ) ;
			m_pcsxi->WriteSakuraPopRegsImm8
				( ECSSakura2Processor::regXP, 2 ) ;
			m_pcsxi->WriteSakuraPopRegsImm8
				( ECSSakura2Processor::regBP, 2 ) ;
			//
			dwBiasTryLocalSize = pNest->m_nLocalSize ;
		}
		m_pcsxi->WriteSakuraLoadInt64_FuncPtr
			( ECSSakura2Processor::regAcc, wstrTryExitLabel ) ;
		m_pcsxi->WriteSakuraJumpReg( ECSSakura2Processor::regAcc ) ;
		//
		// Catch ブロック生成
		//
		EControlNest *	pParentNest = m_nestCtrl.GetLastAt(0) ;
		pNest = new EControlNest( m_dwImplementFlags ) ;
		m_rwCtrlType = rwCatch ;
		pNest->m_rwType = rwCatch ;
		pNest->m_dwBeginPos = CompileCodeGetCurrent() ;
		pNest->m_wstrExceptionHandler = wstrExceptionHandler ;
		pNest->m_wstrCatchEnterLabel = wstrCatchEnterLabel ;
		pNest->m_wstrTryExitLabel = wstrTryExitLabel ;
		pNest->m_pcsxiExceptionHandler = pcsxiExceptionHandler ;
		pNest->m_nLocalSize = dwBiasTryLocalSize ;
		pNest->m_dwBiasTryLocalSize = dwBiasTryLocalSize ;
		pNest->m_nCatchCount = ++ nCatchCount ;
		m_nestCtrl.Add( pNest ) ;
		//
		wstrCatchEnterLabel += EWideString(nCatchCount) ;
		m_pcsxi->AddFunctionEntry
			( wstrCatchEnterLabel,
				CompileCodeGetCurrent(), ECSTypeInfo::flagInline ) ;
		//
		// Catch 構文解釈
		//
		if ( cssLine.HasToComeChar( L"(" ) != L'(' )
		{
			return	ESLErrorMsg( "Catch 文に引数リストが指定されていません。" ) ;
		}
		bool					fCatchAll = cssLine.HasToComeToken( L"..." ) ;
		bool					fCatchErrMsg = false ;
		const ECSClassInfo *	pCatchClassInf = NULL ;
		ECSTypeInfo				typeCatch ;
		EWideString				wstrVarName ;
		if ( !fCatchAll )
		{
			err = ParseTypeDescription( typeCatch, cssLine ) ;
			if ( err )
			{
				return	err ;
			}
			if ( typeCatch.IsTypeReference() )
			{
				return	ESLErrorMsg( "参照型を受け取ることはできません" ) ;
			}
			if ( typeCatch.IsTypePointer() )
			{
				ECSObject *	pPtrType = typeCatch.GetNakedPointerType() ;
				if ( (pPtrType == NULL)
					|| (pPtrType->m_vtType != csvtInteger)
					|| (((ECSInteger*)pPtrType)->GetIntegerType() != csvtUint16) )
				{
					return	ESLErrorMsg( "ポインタ型を受け取ることはできません" ) ;
				}
				fCatchErrMsg = true ;
			}
			else if ( typeCatch.IsTypeArray() )
			{
				return	ESLErrorMsg( "配列型を受け取ることはできません" ) ;
			}
			else
			{
				pCatchClassInf = typeCatch.GetNakedMemoryClassInfo() ;
				if ( pCatchClassInf == NULL )
				{
					return	ESLErrorMsg( "naked クラスしか Catch できません" ) ;
				}
			}
			wstrVarName = cssLine.GetAToken() ;
			if ( wstrVarName == L")" )
			{
				wstrVarName = L"" ;
			}
			else if ( cssLine.HasToComeChar( L")" ) != L')' )
			{
				return	ESLErrorMsg
					( "Catch 文の \'(\' に対応する \')\' が見つかりません" ) ;
			}
			//
			if ( pParentNest != NULL )
			{
				int	iThrow = pParentNest->m_lstThrows.Find
								( pCatchClassInf->GetGlobalName() ) ;
				if ( iThrow >= 0 )
				{
					pParentNest->m_lstThrows.RemoveAt( iThrow ) ;
				}
			}
		}
		//
		// 例外捕捉判定・進入処理
		//
		ECSExecutionImageCompiler *	pcsxiTemp = m_pcsxi ;
		m_pcsxi = pcsxiExceptionHandler ;
		if ( fCatchAll )
		{
			m_pcsxi->WriteSakuraSllRegRegImm8
				( ECSSakura2Processor::regAcc,
					ECSSakura2Processor::regFillBit, 31 ) ;
			m_pcsxi->WriteSakuraCmpNeRegReg
				( ECSSakura2Processor::regAcc,
					ECSSakura2Processor::regException0 ) ;
			const DWORD	dwJumpNeReqFree =
				m_pcsxi->WriteSakuraCJumpOffset32
							( ECSSakura2Processor::regAcc, 0 ) ;
			//
			m_pcsxi->WriteSakuraPushReg
				( ECSSakura2Processor::regException1 ) ;
			m_pcsxi->WriteSakuraSysCallFunction( L"free" ) ;
			m_pcsxi->WriteSakuraAddSP( 8 ) ;
			//
			CompileCodeCommitJumpAddress
				( dwJumpNeReqFree, CompileCodeGetCurrent() ) ;
			m_pcsxi->WriteSakuraLoadInt64_FuncPtr
				( ECSSakura2Processor::regAcc, wstrCatchEnterLabel ) ;
			m_pcsxi->WriteSakuraJumpReg( ECSSakura2Processor::regAcc ) ;
			//
			if ( pParentNest != NULL )
			{
				pParentNest->m_lstThrows.RemoveAll() ;
			}
		}
		else
		{
			m_pcsxi->WriteSakuraSllRegRegImm8
				( ECSSakura2Processor::regAcc,
					ECSSakura2Processor::regFillBit, 31 ) ;
			if ( fCatchErrMsg )
			{
				m_pcsxi->WriteSakuraCmpNeRegReg
					( ECSSakura2Processor::regAcc,
						ECSSakura2Processor::regException0 ) ;
			}
			else
			{
				m_pcsxi->WriteSakuraCmpEqRegReg
					( ECSSakura2Processor::regAcc,
						ECSSakura2Processor::regException0 ) ;
			}
			const DWORD	dwJumpNextRefAddr =
				m_pcsxi->WriteSakuraCJumpOffset32
							( ECSSakura2Processor::regAcc, 0 ) ;
			if ( fCatchErrMsg )
			{
				m_pcsxi->WriteSakuraLoadInt64_FuncPtr( 1, wstrCatchEnterLabel ) ;
				m_pcsxi->WriteSakuraMoveRegReg
					( ECSSakura2Processor::regAcc, ECSSakura2Processor::regException1 ) ;
				m_pcsxi->WriteSakuraJumpReg( 1 ) ;
			}
			else
			{
				const int	regClassId = 1 ;
				const int	regCounter = 2 ;
				const int	regNextPtr = 3 ;
				const int	regTemp = 4 ;
				int	regLoaded ;
				m_pcsxi->WriteSakuraLoadInt64_ClassID
					( regClassId, pCatchClassInf->GetGlobalName() ) ;
				regLoaded = regCounter ;
				m_pcsxi->WriteSakuraMoveRegReg
					( regNextPtr, ECSSakura2Processor::regException3 ) ;
				m_pcsxi->WriteSakuraLoadMemory
					( ECSSakura2Processor::addrBase,
						ECSSakura2Processor::dataUint32, regLoaded,
						regNextPtr, 0, -1, 0, true ) ;
				m_pcsxi->WriteSakuraAddRegRegImm32( regNextPtr, regNextPtr, 8 ) ;
				//
				DWORD	dwLoopStart = CompileCodeGetCurrent() ;
				//
				m_pcsxi->WriteSakuraMoveRegReg
					( regTemp, ECSSakura2Processor::regIntZero ) ;
				m_pcsxi->WriteSakuraCmpEqRegReg( regTemp, regCounter ) ;
				const DWORD	dwJumpExitLoop =
					m_pcsxi->WriteSakuraCJumpOffset32( regTemp, 0 ) ;
				//
				regLoaded = regTemp ;
				m_pcsxi->WriteSakuraLoadMemory
					( ECSSakura2Processor::addrBase,
						ECSSakura2Processor::dataUint32, regLoaded,
						regNextPtr, 0, -1, 0, true ) ;
				m_pcsxi->WriteSakuraAddRegRegImm32( regNextPtr, regNextPtr, 8 ) ;
				m_pcsxi->WriteSakuraAddRegRegImm32( regCounter, regCounter, -1 ) ;
				//
				m_pcsxi->WriteSakuraCmpNeRegReg( regLoaded, regClassId ) ;
				CompileCodeCommitJumpAddress
					( m_pcsxi->WriteSakuraCJumpOffset32( regLoaded, 0 ), dwLoopStart ) ;
				//
				regLoaded = ECSSakura2Processor::regAcc ;
				m_pcsxi->WriteSakuraLoadMemory
					( ECSSakura2Processor::addrBaseOffset32,
						ECSSakura2Processor::dataInt32, regLoaded,
						regNextPtr, -4, -1, 0, true ) ;
				m_pcsxi->WriteSakuraAddRegReg
					( regLoaded, ECSSakura2Processor::regException1 ) ;
				//
				m_pcsxi->WriteSakuraLoadInt64_FuncPtr( regTemp, wstrCatchEnterLabel ) ;
				m_pcsxi->WriteSakuraJumpReg( regTemp ) ;
				//
				CompileCodeJump( dwLoopStart ) ;
				//
				CompileCodeCommitJumpAddress
					( dwJumpExitLoop, CompileCodeGetCurrent() ) ;
			}
			CompileCodeCommitJumpAddress
				( dwJumpNextRefAddr, CompileCodeGetCurrent() ) ;
		}
		m_pcsxi = pcsxiTemp ;
		//
		// bp レジスタの内容は先に復元
		//
		int	regLoadedBP = ECSSakura2Processor::regBP ;
		m_pcsxi->WriteSakuraLoadMemory
			( ECSSakura2Processor::addrBaseOffset32,
				ECSSakura2Processor::dataInt64,
				regLoadedBP,
				ECSSakura2Processor::regXP, 24, -1, 0, true ) ;
		if ( regLoadedBP != ECSSakura2Processor::regBP )
		{
			m_pcsxi->WriteSakuraMoveRegReg
				( ECSSakura2Processor::regBP, regLoadedBP ) ;
		}
		//
		// 受け取った例外オブジェクトをローカルに複製構築
		// （オブジェクトを受け取った場合 acc レジスタにポインタを受け取る）
		//
		ECSTypeInfo *	pVarType = NULL ;
		m_pcsxi->ResetAllRegisterAssigns() ;
		if ( !wstrVarName.IsEmpty() )
		{
			m_pcsxi->WriteSakuraPushReg( ECSSakura2Processor::regAcc ) ;
			//
			pVarType = CompileAllocateNakedLocalVariable
								( pNest, typeCatch, wstrVarName ) ;
			ESLAssert( pVarType != NULL ) ;
			ECSTypeInfo	typeVar ;
			err = CompilePrepareNakedLocalVariable( typeVar, pVarType, true ) ;
			if ( err )
			{
				OutputError
					( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
			}
			FreeExpressionRegister( typeVar ) ;
			//
			ECSTypeInfo	typeTemp ;
			ECSTypeInfo	typeSrc ;
			if ( fCatchErrMsg )
			{
				typeSrc = *pVarType ;
				typeSrc.ClearLoadedRegister() ;
				typeSrc.ClearAddressingInfo() ;
			}
			else
			{
				typeSrc.MakeReferenceOf( *pVarType ) ;
			}
			typeSrc.SetLoadedRegister( AllocateExpressionRegister() ) ;
			m_pcsxi->WriteSakuraPopReg( typeSrc.m_regLoaded ) ;
			//
			err = CompileTypeMoveOperate
				( typeTemp, *pVarType, typeSrc, csotNop, true ) ;
			if ( err )
			{
				OutputError
					( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
			}
		}
		//
		// ガベージ処理
		//
		const int	regTemp1 = AllocateExpressionRegister() ;
		const int	regTemp2 = AllocateExpressionRegister() ;
		DWORD	dwGCLoopBegin = CompileCodeGetCurrent() ;
		//
		m_pcsxi->WriteSakuraMoveRegReg
			( regTemp1, ECSSakura2Processor::regIntZero ) ;
		m_pcsxi->WriteSakuraCmpEqRegReg
			( regTemp1, ECSSakura2Processor::regYP ) ;
		const DWORD	dwJumpExitRef =
			m_pcsxi->WriteSakuraCJumpOffset32( regTemp1, 0 ) ;
		//
		int	regLoadedDestructor = regTemp1 ;
		int	regLoadedObjPtr = regTemp2 ;
		m_pcsxi->WriteSakuraLoadMemory
			( ECSSakura2Processor::addrBaseOffset32,
				ECSSakura2Processor::dataInt64,
				regLoadedDestructor,
				ECSSakura2Processor::regYP, 8, -1, 0, true ) ;
		m_pcsxi->WriteSakuraLoadMemory
			( ECSSakura2Processor::addrBaseOffset32,
				ECSSakura2Processor::dataInt64,
				regLoadedObjPtr,
				ECSSakura2Processor::regYP, 16, -1, 0, true ) ;
		m_pcsxi->WriteSakuraPushReg( regLoadedObjPtr ) ;
		m_pcsxi->WriteSakuraCallReg( regLoadedDestructor ) ;
		m_pcsxi->WriteSakuraAddSP( 8 ) ;
		//
		int	regLoadedYP = ECSSakura2Processor::regYP ;
		m_pcsxi->WriteSakuraLoadMemory
			( ECSSakura2Processor::addrBase,
				ECSSakura2Processor::dataInt64,
				regLoadedYP, ECSSakura2Processor::regYP, 0, -1, 0, true ) ;
		if ( regLoadedYP != ECSSakura2Processor::regYP )
		{
			m_pcsxi->WriteSakuraMoveRegReg
				( ECSSakura2Processor::regYP, regLoadedYP ) ;
		}
		CompileCodeCommitJumpAddress
			( m_pcsxi->WriteSakuraJumpOffset32(0), dwGCLoopBegin ) ;
		//
		CompileCodeCommitJumpAddress
			( dwJumpExitRef, CompileCodeGetCurrent() ) ;
		FreeExpressionRegister() ;
		FreeExpressionRegister() ;
		//
		// 構造化例外ブロック解放
		//
		m_pcsxi->WriteSakuraAddRegRegImm32
			( ECSSakura2Processor::regSP,
				ECSSakura2Processor::regXP, 8 ) ;
		m_pcsxi->WriteSakuraPopRegsImm8
			( ECSSakura2Processor::regXP, 2 ) ;
		m_pcsxi->WriteSakuraPopRegsImm8
			( ECSSakura2Processor::regBP, 2 ) ;
		//
		if ( pVarType != NULL )
		{
			err = CompileCodeNakedAddGarbageList( pVarType ) ;
			if ( err )
			{
				OutputError
					( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
			}
		}
	}
	else
	{
		if ( m_rwCtrlType != rwTry )
		{
			return	ESLErrorMsg
				( "Catch 文に対応する Try 文がありません。" ) ;
		}
		EControlNest *	pNest = m_nestCtrl.GetLastAt( 0 ) ;
		ESLAssert( pNest != NULL ) ;
		ESLAssert( pNest->m_rwType == m_rwCtrlType ) ;
		//
		// Try ブロックの終端へのジャンプ
		//
		m_pcsxi->FenceInstruction() ;
		//
		DWORD	dwDummy = 0 ;
		DWORD	dwBreakAddr = CompileCodeJump() ;
		//
		// Try ブロックの Catch アドレス確定
		//
		CommitBreakAddressOnNest( pNest, CompileCodeGetCurrent() ) ;
		pNest->m_lstBreak.RemoveAll( ) ;
		pNest->m_lstBreak.Add( dwBreakAddr ) ;
		//
		// Catch 文の引数解析
		//
		if ( cssLine.HasToComeChar( L"(" ) != L'(' )
		{
			return	ESLErrorMsg( "Catch 文に引数リストが指定されていません。" ) ;
		}
		ESLError			err ;
		ECSPrototypeInfo	prototype ;
		err = ParseArgumentDescription( prototype, cssLine ) ;
		if ( err )
		{
			return	err ;
		}
		//
		// ネスト設定
		//
		pNest = new EControlNest( m_dwImplementFlags ) ;
		m_rwCtrlType = rwCatch ;
		pNest->m_rwType = rwCatch ;
		pNest->m_dwBeginPos = CompileCodeGetCurrent( ) ;
		m_nestCtrl.Add( pNest ) ;
		//
		// 名前空間生成
		//
		EObjArray<ECSTypeInfo>	lstArgType ;
		EObjArray<EWideString>	lstArgName ;
		lstArgType = prototype.GetArgument() ;
		lstArgName = prototype.GetArgumentName() ;
		//
		DWORD	dwArgCount = lstArgType.GetSize( ) ;
		m_pcsxi->WriteInstructionCode( csicEnter ) ;
		m_pcsxi->WriteConstantString( ECSWideString(L"@CATCH") ) ;
		m_pcsxi->WriteCodeData( &dwArgCount, sizeof(dwArgCount) ) ;
		//
		EncodeArgumentList( pNest, lstArgType, lstArgName ) ;
		//
		m_pcsxi->FenceInstruction() ;
	}
	return	eslErrSuccess ;
}

// EndTry 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileEndTry( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwEndTry ) )
	{
		return	eslErrSuccess ;
	}
	if ( m_rwCtrlType != rwCatch )
	{
		return	ESLErrorMsg
			( "EndTry 文に対応する Catch 文がありません。" ) ;
	}
	EControlNest *	pNestCatch = m_nestCtrl.GetLastAt( 0 ) ;
	ESLAssert( pNestCatch != NULL ) ;
	ESLAssert( pNestCatch->m_rwType == m_rwCtrlType ) ;
	//
	m_pcsxi->FenceInstruction() ;
	//
	bool	fControlReturned = pNestCatch->m_fReturned ;
	if ( m_modeNakedCode )
	{
		//
		// catch ブロックの終了処理
		//
		EWideString	wstrExceptionHandler = pNestCatch->m_wstrExceptionHandler ;
		EWideString	wstrTryExitLabel = pNestCatch->m_wstrTryExitLabel ;
		ECSExecutionImageCompiler *
					pcsxiExceptionHandler = pNestCatch->m_pcsxiExceptionHandler ;
		//
		ESLError	err = CompileCodeNakedLocalDestruction( pNestCatch, 0 ) ;
		if ( err )
		{
			OutputError
				( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
		}
		LeaveControlNest() ;
		m_pcsxi->FenceInstruction() ;
		//
		// try 脱出アドレス登録
		//
		m_pcsxi->AddFunctionEntry
			( wstrTryExitLabel,
				CompileCodeGetCurrent(), ECSTypeInfo::flagInline ) ;
		//
		// 例外ハンドラ・処理されなかった例外の場合
		// ガベージリストの終端に
		// 上位の例外ブロックのガベージリストを接続
		//
		ECSExecutionImageCompiler *	pcsxiTemp = m_pcsxi ;
		m_pcsxi = pcsxiExceptionHandler ;
		//
		const int	regTemp1 = AllocateExpressionRegister() ;
		const int	regTemp2 = AllocateExpressionRegister() ;
		const int	regTemp3 = AllocateExpressionRegister() ;
		int	regLoadedPrevYP = regTemp2 ;
		m_pcsxi->WriteSakuraLoadMemory
			( ECSSakura2Processor::addrBaseOffset32,
				ECSSakura2Processor::dataInt64,
				regLoadedPrevYP,
				ECSSakura2Processor::regXP, 16, -1, 0, true ) ;
		m_pcsxi->WriteSakuraMoveRegReg
			( regTemp3, ECSSakura2Processor::regYP ) ;
		m_pcsxi->WriteSakuraCmpEqRegReg
			( regTemp3, ECSSakura2Processor::regIntZero ) ;
		m_pcsxi->WriteSakuraMaskMoveRegRegReg
			( ECSSakura2Processor::regYP, regLoadedPrevYP, regTemp3 ) ;
		//
		DWORD	dwJumpToGCListExit =
			m_pcsxi->WriteSakuraCJumpOffset32( regTemp3, 0 ) ;
		//
		// if yp != #zero
		m_pcsxi->WriteSakuraMoveRegReg
			( ECSSakura2Processor::regAcc, ECSSakura2Processor::regYP ) ;
		//
		DWORD	dwGCLoopBegin = CompileCodeGetCurrent() ;
		//
		int	regLoadedNextYP = regTemp1 ;
		m_pcsxi->WriteSakuraLoadMemory
			( ECSSakura2Processor::addrBase,
				ECSSakura2Processor::dataInt64,
				regLoadedNextYP,
				ECSSakura2Processor::regAcc, 0, -1, 0, true ) ;
		m_pcsxi->WriteSakuraMoveRegReg( regTemp3, regLoadedNextYP ) ;
		m_pcsxi->WriteSakuraCmpNeRegReg
				( regTemp3, ECSSakura2Processor::regIntZero ) ;
		m_pcsxi->WriteSakuraMaskMoveRegRegReg
			( ECSSakura2Processor::regAcc, regLoadedNextYP, regTemp3 ) ;
		//
		CompileCodeCommitJumpAddress
			( m_pcsxi->WriteSakuraCJumpOffset32( regTemp3, 0 ), dwGCLoopBegin ) ;
		//
		m_pcsxi->WriteSakuraStoreMemory
			( ECSSakura2Processor::addrBase,
				ECSSakura2Processor::dataInt64,
				regLoadedPrevYP,
				ECSSakura2Processor::regAcc, 0, -1, 0, true ) ;
		//
		CompileCodeCommitJumpAddress
			( dwJumpToGCListExit, CompileCodeGetCurrent() ) ;
		//
		FreeExpressionRegister() ;
		FreeExpressionRegister() ;
		FreeExpressionRegister() ;
		//
		// 上位の構造化例外ブロックへ移動
		//
		int	regLoadedPrevXP = ECSSakura2Processor::regXP ;
		m_pcsxi->WriteSakuraLoadMemory
			( ECSSakura2Processor::addrBaseOffset32,
				ECSSakura2Processor::dataInt64,
				regLoadedPrevXP,
				ECSSakura2Processor::regXP, 8, -1, 0, true ) ;
		//
		// 例外ハンドラ関数完成
		//
		m_pcsxi->WriteSakuraPushRegsImm8
			( ECSSakura2Processor::regException0, 4 ) ;
		m_pcsxi->WriteSakuraSysCallFunction( L"throw_exception" ) ;
		//
		SetEndOfTemporaryInlineFunction
			( pcsxiExceptionHandler, wstrExceptionHandler ) ;
		m_pcsxi = pcsxiTemp ;
	}
	else
	{
		//
		// Catch ブロック解放
		//
		m_pcsxi->WriteInstructionCode( csicLeave ) ;
		delete	m_nestCtrl.Pop( ) ;
		//
		// Try ブロック解放
		//
		EControlNest *	pNestTry = m_nestCtrl.GetLastAt( 0 ) ;
		ESLAssert( pNestTry != NULL ) ;
		if ( (pNestTry == NULL) || (pNestTry->m_rwType != rwTry) )
		{
			return	ESLErrorMsg
				( "EndTry 文に対応する Try 文がありません。" ) ;
		}
		CommitBreakAddressOnNest( pNestTry, CompileCodeGetCurrent() ) ;
		if ( m_modeNakedCode )
		{
			ESLError	err = CompileCodeNakedLocalDestruction( pNestTry, 0 ) ;
			if ( err )
			{
				OutputError
					( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
			}
		}
		else
		{
			m_pcsxi->WriteInstructionCode( csicLeave ) ;
		}
		fControlReturned = (fControlReturned && pNestTry->m_fReturned) ;
		//
		LeaveControlNest() ;
		//
		m_pcsxi->FenceInstruction() ;
		//
		if ( fControlReturned )
		{
			EControlNest *	pNest = m_nestCtrl.GetLastAt( 0 ) ;
			if ( pNest != NULL )
			{
				pNest->m_fReturned = true ;
			}
		}
	}
	return	eslErrSuccess ;
}

// Throw 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileThrow( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwThrow ) )
	{
		return	eslErrSuccess ;
	}
	if ( m_modeNakedCode )
	{
		ECSTypeInfo	typeThrow ;
		ESLError	err = CompileExpression( typeThrow, cssLine ) ;
		if ( err )
		{
			return	err ;
		}
		const ECSClassInfo *
			pNakedClass = GetNakedTypeClassInfo( typeThrow ) ;
		if ( (pNakedClass == NULL)
			|| !pNakedClass->IsNakedMemoryClass()
			|| typeThrow.IsTypeArray() || typeThrow.IsTypeReference2() )
		{
			EWideString	wstrType ;
			typeThrow.FormatTypeString( wstrType ) ;
			m_strErrMsg = EString(wstrType) + " は throw 出来ない型です" ;
			return	ESLErrorMsg( m_strErrMsg ) ;
		}
		MakeCommitValueToNakedRegister( typeThrow ) ;
		//
		m_pcsxi->WriteSakuraMoveRegReg
			( ECSSakura2Processor::regException0,
						ECSSakura2Processor::regIntZero ) ;
		m_pcsxi->WriteSakuraMoveRegReg
			( ECSSakura2Processor::regException1,
						typeThrow.GetLoadedRegister() ) ;
		m_pcsxi->WriteSakuraMoveRegReg
			( ECSSakura2Processor::regException2,
						ECSSakura2Processor::regIntZero ) ;
		m_pcsxi->WriteSakuraLoadInt64_FuncPtr
			( ECSSakura2Processor::regException3,
				pNakedClass->GetGlobalName() + L"::<rtcvector>" ) ;
		m_pcsxi->WriteSakuraPushRegsImm8
			( ECSSakura2Processor::regException0, 4 ) ;
		m_pcsxi->WriteSakuraSysCallFunction( L"throw_exception" ) ;
		//
		FreeExpressionRegister( typeThrow ) ;
		//
		EControlNest *	pNest = m_nestCtrl.GetLastAt( 0 ) ;
		if ( (pNest == NULL) || !IsInFunctionNest() )
		{
			return	ESLErrorMsg
				( "グローバルスコープでは throw 出来ません" ) ;
		}
		pNest->m_lstThrows.Add
			( new EWideString(pNakedClass->GetGlobalName()) ) ;
	}
	else
	{
		if ( cssLine.HasToComeChar( L"(" ) != L'(' )
		{
			return	ESLErrorMsg( "Throw 文に引数が指定されていません" ) ;
		}
		EObjArray<ECSTypeInfo>	lstArgType ;
		ESLError	err = CompileArgument( lstArgType, NULL, cssLine ) ;
		if ( err )
		{
			return	err ;
		}
		int	nArgCount = lstArgType.GetSize() ;
		//
		m_pcsxi->FenceInstruction() ;
		//
		m_pcsxi->WriteInstructionCode( csicCall ) ;
		m_pcsxi->WriteObjectModeCode( csomImmediate ) ;
		ESLAssert( sizeof(int) >= sizeof(DWORD) ) ;
		m_pcsxi->WriteCodeData( &nArgCount, sizeof(DWORD) ) ;
		m_pcsxi->WriteConstantString( ECSWideString( L"@CATCH" ) ) ;
		m_pcsxi->WriteInstructionCode( csicReturn ) ;
		m_pcsxi->WriteByteCode( 3 ) ;
	}
	EControlNest *	pNest = m_nestCtrl.GetLastAt( 0 ) ;
	if ( pNest != NULL )
	{
		pNest->m_fReturned = true ;
	}
	return	eslErrSuccess ;
}

// For 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileFor( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwFor ) )
	{
		return	eslErrSuccess ;
	}
	if ( m_rwCtrlType == rwData )
	{
		return	ESLErrorMsg
			( "For 文は Data ブロックの中では記述できません。" ) ;
	}
	else if ( m_rwCtrlType == rwStructure )
	{
		return	ESLErrorMsg
			( "Structure ブロックの中で For 文が記述されています。" ) ;
	}
	else if ( m_rwCtrlType == rwClass )
	{
		return	ESLErrorMsg
			( "Class ブロックの中で For 文が記述されています。" ) ;
	}
	else if ( m_rwCtrlType == rwEnumerator )
	{
		return	ESLErrorMsg
			( "Enumerator ブロックの中で For 文が記述されています。" ) ;
	}
	//
	// キーワード
	//////////////////////////////////////////////////////////////////////////
	enum	KeywordIndex
	{
		kwiFrom, kwiTo, kwiUntil, kwiWhile, kwiStep, kwiBy, kwiMax
	} ;
	static const wchar_t *	pwszKeyword[kwiMax] =
	{
		L"From", L"To", L"Until", L"While", L"Step", L"By"
	} ;
	//
	// 構文解析：参照変数取得
	//////////////////////////////////////////////////////////////////////////
	ECSWideString	wstrRefName, wstrIndexName ;
	ECSWideString	wstrArrayExpr, wstrType ;
	int				nTokenType ;
	int				iLast = cssLine.GetIndex( ) ;
	ESLError		err ;
	wstrRefName = cssLine.GetAToken( &nTokenType ) ;
	if ( !wstrRefName.IsEmpty() )
	do
	{
		bool	fKeyword = false ;
		for ( int i = 0; i < kwiMax; i ++ )
		{
			if ( !CompareReservedWord( pwszKeyword[i], wstrRefName ) )
			{
				fKeyword = true ;
				break ;
			}
		}
		if ( fKeyword )
		{
			wstrRefName = L"" ;
			cssLine.MoveIndex( iLast ) ;
			break ;
		}
		if ( nTokenType != 0 )
		{
			return	ESLErrorMsg
				( "For 文の中で参照変数名に不正な文字が用いられています。" ) ;
		}
		wstrType = cssLine.GetAToken( ) ;
		if ( wstrType == L"|" )
		{
			err = VerifyUserSymbol( wstrRefName ) ;
			if ( err )
			{
				return	err ;
			}
		}
		else if ( (wstrType == L":=") | (wstrType == L"=") )
		{
			if ( (wstrType == L"=")
				&& !(m_dwModeFlags & flagNoWarningEquMove) )
			{
				err = OutputWarning1
					( "変数の初期値の指定に = が用いられています。"
						":= を使ってください（推奨）。",
						m_strFilePath, m_nLineNum ) ;
				if ( err )
				{
					return	err ;
				}
			}
			err = VerifyUserSymbol( wstrRefName ) ;
			if ( err )
			{
				return	err ;
			}
			wstrIndexName = wstrRefName ;
			wstrRefName.FreeString( ) ;
		}
		else
		{
			return	ESLErrorMsg
				( "For 文の参照変数と配列は | 記号、"
						"初期値とは := で区切ってください。" ) ;
		}
	}
	while ( false ) ;
	//
	// 構文解析：キーワードで行を分解
	//////////////////////////////////////////////////////////////////////////
	ECSWideString	wstrParam[kwiMax + 1] ;
	int				iParam = kwiMax ;
	iLast = cssLine.GetIndex( ) ;
	while ( !cssLine.DisregardSpace() )
	{
		ECSWideString	wstrToken ;
		int		iCurrent = cssLine.GetIndex( ) ;
		wstrToken = cssLine.GetAToken( ) ;
		if ( (wstrToken == L"\"") || (wstrToken == L"\'") )
		{
			cssLine.PassEnclosedString( wstrToken.GetAt(0), m_dwModeFlags ) ;
			cssLine.HasToComeChar( wstrToken ) ;
		}
		else if ( wstrToken == L"(" )
		{
			cssLine.PassEnclosedString( L')', m_dwModeFlags ) ;
			cssLine.HasToComeChar( L")" ) ;
		}
		else
		for ( int i = 0; i < kwiMax; i ++ )
		{
			if ( !CompareReservedWord( pwszKeyword[i], wstrToken ) )
			{
				if ( !wstrParam[iParam].IsEmpty() )
				{
					m_strErrMsg = EString(pwszKeyword[i])
							+ " キーワードが複数指定されています。" ;
					return	ESLErrorMsg( m_strErrMsg ) ;
				}
				wstrParam[iParam] = cssLine.Middle( iLast, iCurrent - iLast ) ;
				iParam = i ;
				iLast = cssLine.GetIndex( ) ;
				break ;
			}
		}
	}
	if ( !wstrParam[iParam].IsEmpty() )
	{
		m_strErrMsg = EString(pwszKeyword[iParam - 1])
					+ " キーワードが複数指定されています。" ;
		return	ESLErrorMsg( m_strErrMsg ) ;
	}
	wstrParam[iParam] = cssLine.Middle( iLast ) ;
	//
	// 最初のキーワードまでの構文を解釈＆構文の整合性を検証
	//////////////////////////////////////////////////////////////////////////
	if ( wstrType == L"|" )
	{
		int	iLastIndex = -1 ;
		ECSSourceStream	cssFirst ;
		cssFirst = wstrParam[kwiMax] ;
		while ( !cssFirst.DisregardSpace() )
		{
			cssFirst.PassEnclosedString( L'[', m_dwModeFlags ) ;
			if ( cssFirst.CurrentCharacter() != L'[' )
			{
				break ;
			}
			iLastIndex = cssFirst.GetIndex( ) ;
			cssFirst.GetCharacter( ) ;
			cssFirst.PassEnclosedString( L']', m_dwModeFlags ) ;
			cssFirst.GetCharacter( ) ;
		}
		if ( iLastIndex < 0 )
		{
			return	ESLErrorMsg
				( "For 文に配列の指標変数名が指定されていません。" ) ;
		}
		wstrArrayExpr = wstrParam[kwiMax].Left( iLastIndex ) ;
		//
		cssFirst.MoveIndex( iLastIndex + 1 ) ;
		wstrIndexName = cssFirst.GetAToken( &nTokenType ) ;
		if ( nTokenType != 0 )
		{
			return	ESLErrorMsg
				( "For 文の中で指標変数名に不正な文字が用いられています。" ) ;
		}
		err = VerifyUserSymbol( wstrIndexName ) ;
		if ( err )
		{
			return	err ;
		}
		if ( cssFirst.GetAToken() != L"]" )
		{
			return	ESLErrorMsg( "指標が ']' 記号で閉じられていません。" ) ;
		}
	}
	else
	{
		if ( !wstrParam[kwiFrom].IsEmpty() )
		{
			return	ESLErrorMsg
				( "配列を参照しない For 文では "
					"From キーワードを指定することは出来ません。" ) ;
		}
		wstrParam[kwiFrom] = wstrParam[kwiMax] ;
	}
	if ( (!wstrParam[kwiTo].IsEmpty() && !wstrParam[kwiUntil].IsEmpty())
		|| (!wstrParam[kwiTo].IsEmpty() && !wstrParam[kwiWhile].IsEmpty())
		|| (!wstrParam[kwiUntil].IsEmpty() && !wstrParam[kwiWhile].IsEmpty()) )
	{
		return	ESLErrorMsg
			( "To, Unti, While 構文は同時に指定することは出来ません。" ) ;
	}
	if ( !wstrParam[kwiStep].IsEmpty() && !wstrParam[kwiBy].IsEmpty() )
	{
		return	ESLErrorMsg
			( "Step, By 構文は同時に指定することは出来ません。" ) ;
	}
	//
	// For ブロック作成
	//////////////////////////////////////////////////////////////////////////
	m_pcsxi->FenceInstruction() ;
	//
	EControlNest *	pNest = new EControlNest( m_dwImplementFlags ) ;
	m_rwCtrlType = rwFor ;
	pNest->m_rwType = rwFor ;
	pNest->m_dwBeginPos = CompileCodeGetCurrent( ) ;
	m_nestCtrl.Add( pNest ) ;
	//
	// 反復変数のための名前空間を生成
	//////////////////////////////////////////////////////////////////////////
	if ( !m_modeNakedCode )
	{
		DWORD	dwDummy = 0 ;
		m_pcsxi->WriteInstructionCode( csicEnter ) ;
		m_pcsxi->WriteConstantString( ECSWideString(L"") ) ;
		m_pcsxi->WriteCodeData( &dwDummy, sizeof(dwDummy) ) ;
	}
	//
	// 反復変数を生成
	//////////////////////////////////////////////////////////////////////////
	if ( !wstrIndexName.IsEmpty() )
	{
		ECSTypeInfo *	pIndexVarType = new ECSTypeInfo( new ECSInteger ) ;
		AddVariableToControlNest
			( pNest, wstrIndexName, pIndexVarType ) ;
		//
		if ( !m_modeNakedCode )
		{
			m_pcsxi->WriteInstructionCode( csicNew ) ;
			m_pcsxi->WriteObjectModeCode( csomStack ) ;
			m_pcsxi->WriteVariableTypeCode( csvtInteger ) ;
			m_pcsxi->WriteConstantString( wstrIndexName ) ;
		}
		if ( !wstrParam[kwiFrom].IsEmpty() )
		{
			ECSSourceStream	cssFrom ;
			ECSTypeInfo		typeExpr ;
			cssFrom = wstrParam[kwiFrom] ;
			BeginSakura2Optimize() ;
			err = CompileExpression( typeExpr, cssFrom ) ;
			FinishSakura2Optimize() ;
			if ( err )
			{
				LeaveControlNest() ;
				return	err ;
			}
			if ( typeExpr.IsVoid() )
			{
				LeaveControlNest() ;
				return	ESLErrorMsg
					( "反復変数の初期値に void が指定されています。" ) ;
			}
			if ( !m_modeNakedCode )
			{
				CompileCodeStore( csotNop ) ;
			}
			else
			{
				CompileCodeNakedUncoverReference( typeExpr ) ;
				MakeCommitValueToNakedRegister( typeExpr ) ;
				//
				WriteSakuraStoreMemory
					( typeExpr.GetLoadedRegister(), *pIndexVarType ) ;
			}
			FreeExpressionTemporaryStack() ;
		}
	}
	//
	// 反復順次処理実装
	//////////////////////////////////////////////////////////////////////////
	ECSSourceStream	cssForBy ;
	bool	fDefaultStep = false ;
	if ( !wstrParam[kwiStep].IsEmpty() && !wstrIndexName.IsEmpty() )
	{
		cssForBy = wstrIndexName + L" += " + wstrParam[kwiStep] ;
	}
	else if ( !wstrParam[kwiBy].IsEmpty() )
	{
		cssForBy = wstrParam[kwiBy] ;
	}
	else if ( !wstrIndexName.IsEmpty() )
	{
		cssForBy = wstrIndexName + L" += 1" ;
		fDefaultStep = true ;
	}
	if ( !cssForBy.IsEmpty() && (fDefaultStep || wstrParam[kwiTo].IsEmpty()) )
	{
		DWORD	dwJumpAddr ;
		dwJumpAddr = CompileCodeJump( ) ;
		//
		pNest->m_dwBeginPos = CompileCodeGetCurrent( ) ;
		//
//		ESLAssert( pNest->m_lstLocalObj.GetSize() == 0 ) ;
		//
		ECSTypeInfo	typeBy ;
		BeginSakura2Optimize() ;
		err = CompileExpression( typeBy, cssForBy ) ;
		if ( err )
		{
			LeaveControlNest() ;
			return	err ;
		}
		if ( typeBy.m_pValue != NULL )
		{
			CompileCodeFreeStack( typeBy ) ;
		}
		FinishSakura2Optimize() ;
		FreeExpressionTemporaryStack() ;
		m_pcsxi->FenceInstruction() ;
		//
		if ( m_modeNakedCode )
		{
			CompileCodeNakedLocalDestructionSaveRegister
					( pNest, ECSSakura2Processor::regExpr0, 0 ) ;
			pNest->m_lstLocalName.RemoveAll( ) ;
			pNest->m_lstLocalObj.RemoveAll( ) ;
		}
		CompileCodeCommitJumpAddress
			( dwJumpAddr, CompileCodeGetCurrent() ) ;
	}
	else
	{
		m_pcsxi->FenceInstruction() ;
		pNest->m_dwBeginPos = CompileCodeGetCurrent( ) ;
	}
	//
	// 反復条件判定
	//////////////////////////////////////////////////////////////////////////
	int	nIterLogic = 0 ;
	ECSSourceStream	cssIter ;
	if ( !wstrParam[kwiTo].IsEmpty() && !wstrIndexName.IsEmpty() )
	{
		//
		// To 構文
		//
		if ( fDefaultStep )
		{
			cssIter = wstrIndexName + L" > (" + wstrParam[kwiTo] + L")" ;
		}
		else
		{
			cssIter = wstrIndexName + L" == (" + wstrParam[kwiTo] + L")" ;
		}
		nIterLogic = 1 ;
	}
	else if ( !wstrParam[kwiUntil].IsEmpty() )
	{
		//
		// Until 構文
		//
		cssIter = wstrParam[kwiUntil] ;
		nIterLogic = 1 ;
	}
	else if ( !wstrParam[kwiWhile].IsEmpty() )
	{
		//
		// While 構文
		//
		cssIter = wstrParam[kwiWhile] ;
		nIterLogic = 0 ;
	}
	else if ( !wstrArrayExpr.IsEmpty() )
	{
		//
		// 省略
		//
		cssIter = wstrIndexName + L" >= sizeof(" + wstrArrayExpr + L")" ;
		nIterLogic = 1 ;
	}
	if ( !cssIter.IsEmpty() )
	{
		DWORD	dwJumpAddrBegin ;
		if ( !fDefaultStep && !wstrParam[kwiTo].IsEmpty() )
		{
			dwJumpAddrBegin = CompileCodeJump( ) ;
			pNest->m_dwBeginPos = CompileCodeGetCurrent( ) ;
			m_pcsxi->FenceInstruction() ;
		}
		ECSTypeInfo	typeIter ;
		BeginSakura2Optimize() ;
		err = CompileExpression( typeIter, cssIter ) ;
		if ( err )
		{
			LeaveControlNest() ;
			return	err ;
		}
		err = VerifyTypeBoolean( typeIter ) ;
		if ( err )
		{
			LeaveControlNest() ;
			return	err ;
		}
		FinishSakura2Optimize() ;
		FreeExpressionTemporaryStack( ECSSakura2Processor::regExpr0 ) ;
		//
		DWORD	dwNoBreakJumpRef =
					CompileCodeConditionalJump( !nIterLogic, false ) ;
		//
		m_pcsxi->FenceInstruction() ;
		//
		if ( m_modeNakedCode )
		{
			err = CompileCodeNakedLocalDestruction( pNest, 0 ) ;
			if ( err )
			{
				LeaveControlNest() ;
				return	err ;
			}
		}
		else
		{
			m_pcsxi->WriteInstructionCode( csicLeave ) ;
		}
		pNest->m_lstBreak.Add( CompileCodeJump() ) ;
		//
		CompileCodeCommitJumpAddress
				( dwNoBreakJumpRef, CompileCodeGetCurrent() ) ;
		//
		m_pcsxi->FenceInstruction() ;
		//
		if ( !fDefaultStep && !wstrParam[kwiTo].IsEmpty() )
		{
			if ( !cssForBy.IsEmpty() )
			{
				ECSTypeInfo	typeBy ;
				BeginSakura2Optimize() ;
				err = CompileExpression( typeBy, cssForBy ) ;
				if ( err )
				{
					LeaveControlNest() ;
					return	err ;
				}
				if ( typeBy.m_pValue != NULL )
				{
					CompileCodeFreeStack() ;
				}
				FinishSakura2Optimize() ;
				FreeExpressionTemporaryStack() ;
			}
			CompileCodeCommitJumpAddress
				( dwJumpAddrBegin, CompileCodeGetCurrent() ) ;
		}
	}
	m_pcsxi->FenceInstruction() ;
	//
	// 配列の要素参照
	//////////////////////////////////////////////////////////////////////////
	if ( !m_modeNakedCode )
	{
		DWORD	dwDummy = 0 ;
		m_pcsxi->WriteInstructionCode( csicEnter ) ;
		m_pcsxi->WriteConstantString( ECSWideString(L"") ) ;
		m_pcsxi->WriteCodeData( &dwDummy, sizeof(dwDummy) ) ;
	}
	if ( !wstrArrayExpr.IsEmpty() )
	{
		if ( m_modeNakedCode )
		{
			LeaveControlNest() ;
			return	ESLErrorMsg( "naked モードで Array は使用出来ません" ) ;
		}
		ECSReference *	pRefVar = new ECSReference ;
		ECSTypeInfo *	pRefVarType = new ECSTypeInfo( pRefVar ) ;
		pNest->m_lstLocalObj.Add( pRefVarType ) ;
		pNest->m_lstLocalName.Add( wstrRefName ) ;
		//
		m_pcsxi->WriteInstructionCode( csicNew ) ;
		m_pcsxi->WriteObjectModeCode( csomStack ) ;
		m_pcsxi->WriteVariableTypeCode( csvtReference ) ;
		m_pcsxi->WriteConstantString( wstrRefName ) ;
		//
		ECSSourceStream	cssElement ;
		ECSTypeInfo		typeElement ;
		cssElement = wstrArrayExpr + L"[" + wstrIndexName + L"]" ;
		err = CompileExpression( typeElement, cssElement ) ;
		FreeExpressionTemporaryStack() ;
		if ( err )
		{
			LeaveControlNest() ;
			return	err ;
		}
		if ( typeElement.IsVoid() )
		{
			LeaveControlNest() ;
			return	ESLErrorMsg( "void な要素参照です。" ) ;
		}
		ECSObject *	pElementType = typeElement.GetNakedType() ;
		if ( (pElementType != NULL)
			&& (pElementType->m_vtType != csvtReference) )
		{
			pRefVar->SetOwnObject
				( ECSTypeInfo::DuplicateType( pElementType ) ) ;
			pRefVarType->m_dwFlags |=
				(typeElement.m_dwFlags & ECSTypeInfo::flagConstant) ;
		}
		CompileCodeStore( csotNop ) ;
	}
	return	eslErrSuccess ;
}

// Next 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileNext( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwNext ) )
	{
		return	eslErrSuccess ;
	}
	if ( m_rwCtrlType != rwFor )
	{
		return	ESLErrorMsg
			( "Next 文に対応する For 文がありません。" ) ;
	}
	EControlNest *	pNest = m_nestCtrl.GetLastAt( 0 ) ;
	ESLAssert( pNest != NULL ) ;
	ESLAssert( pNest->m_rwType == m_rwCtrlType ) ;
	//
	m_pcsxi->FenceInstruction() ;
	//
	ESLError	err = eslErrSuccess ;
	if ( m_modeNakedCode )
	{
		err = CompileCodeNakedLocalDestruction( pNest, 0 ) ;
	}
	else
	{
		m_pcsxi->WriteInstructionCode( csicLeave ) ;
	}
	//
	CompileCodeJump( pNest->m_dwBeginPos ) ;
	//
	CommitBreakAddressOnNest( pNest, CompileCodeGetCurrent() ) ;
	//
	LeaveControlNest() ;
	return	err ;
}

// While 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileWhile( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwWhile ) )
	{
		return	eslErrSuccess ;
	}
	if ( m_rwCtrlType == rwData )
	{
		return	ESLErrorMsg
			( "While 文は Data ブロックの中では記述できません。" ) ;
	}
	else if ( m_rwCtrlType == rwStructure )
	{
		return	ESLErrorMsg
			( "Structure ブロックの中で While 文が記述されています。" ) ;
	}
	else if ( m_rwCtrlType == rwClass )
	{
		return	ESLErrorMsg
			( "Class ブロックの中で While 文が記述されています。" ) ;
	}
	else if ( m_rwCtrlType == rwEnumerator )
	{
		return	ESLErrorMsg
			( "Enumerator ブロックの中で While 文が記述されています。" ) ;
	}
	//
	// While ブロック作成
	//
	m_pcsxi->FenceInstruction() ;
	//
	EControlNest *	pNest = new EControlNest( m_dwImplementFlags ) ;
	m_rwCtrlType = rwWhile ;
	pNest->m_fAutoEnter = true ;
	pNest->m_rwType = rwWhile ;
	pNest->m_dwBeginPos = CompileCodeGetCurrent( ) ;
	m_nestCtrl.Add( pNest ) ;
	//
	// 反復条件判定
	//
	ECSTypeInfo	typeExpr ;
	BeginSakura2Optimize() ;
	ESLError	err =
		CompileExpression( typeExpr, cssLine, 0, NULL ) ;
	if ( err )
	{
		LeaveControlNest() ;
		return	err ;
	}
	err = VerifyTypeBoolean( typeExpr ) ;
	if ( err )
	{
		LeaveControlNest() ;
		return	err ;
	}
	FinishSakura2Optimize() ;
	FreeExpressionTemporaryStack( ECSSakura2Processor::regExpr0 ) ;
	//
	if ( m_modeNakedCode )
	{
		CompileCodeNakedLocalDestructionSaveRegister
					( pNest, ECSSakura2Processor::regExpr0 ) ;
		pNest->m_lstLocalName.RemoveAll( ) ;
		pNest->m_lstLocalObj.RemoveAll( ) ;
	}
	m_pcsxi->FlushAllRegisterAssigns() ;
	//
	pNest->m_lstBreak.Add( CompileCodeConditionalJump( false, false ) ) ;
	//
/*	m_pcsxi->WriteByteCode( csicEnter ) ;
	m_pcsxi->WriteStringData( ECSWideString( L"" ) ) ;
	m_pcsxi->WriteCodeData( &dwDummy, sizeof(dwDummy) ) ;
*/	//
	return	eslErrSuccess ;
}

// EndWhile 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileEndWhile( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwEndWhile ) )
	{
		return	eslErrSuccess ;
	}
	if ( m_rwCtrlType != rwWhile )
	{
		return	ESLErrorMsg
			( "EndWhile 文に対応する While 文がありません。" ) ;
	}
	EControlNest *	pNest = m_nestCtrl.GetLastAt( 0 ) ;
	ESLAssert( pNest != NULL ) ;
	ESLAssert( pNest->m_rwType == m_rwCtrlType ) ;
	//
	m_pcsxi->FenceInstruction() ;
	//
	ESLError	err = eslErrSuccess ;
	if ( m_modeNakedCode )
	{
		err = CompileCodeNakedLocalDestruction( pNest, 0 ) ;
	}
	else
	{
		if ( pNest->m_lstLocalName.GetSize() > 0 )
		{
			m_pcsxi->WriteInstructionCode( csicLeave ) ;
		}
	}
	CompileCodeJump( pNest->m_dwBeginPos ) ;
	//
	CommitBreakAddressOnNest( pNest, CompileCodeGetCurrent() ) ;
	//
	LeaveControlNest() ;
	return	err ;
}

// Repeat 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileRepeat( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwRepeat ) )
	{
		return	eslErrSuccess ;
	}
	if ( m_rwCtrlType == rwData )
	{
		return	ESLErrorMsg
			( "Repeat 文は Data ブロックの中では記述できません。" ) ;
	}
	else if ( m_rwCtrlType == rwStructure )
	{
		return	ESLErrorMsg
			( "Structure ブロックの中で Repeat 文が記述されています。" ) ;
	}
	else if ( m_rwCtrlType == rwClass )
	{
		return	ESLErrorMsg
			( "Class ブロックの中で Repeat 文が記述されています。" ) ;
	}
	else if ( m_rwCtrlType == rwEnumerator )
	{
		return	ESLErrorMsg
			( "Enumerator ブロックの中で Repeat 文が記述されています。" ) ;
	}
	//
	// Repeat ブロック作成
	//
	m_pcsxi->FenceInstruction() ;
	//
	EControlNest *	pNest = new EControlNest( m_dwImplementFlags ) ;
	m_rwCtrlType = rwRepeat ;
	pNest->m_fAutoEnter = true ;
	pNest->m_rwType = rwRepeat ;
	pNest->m_dwBeginPos = CompileCodeGetCurrent( ) ;
	m_nestCtrl.Add( pNest ) ;
	//
	DWORD	dwDummy = 0 ;
/*	m_pcsxi->WriteByteCode( csicEnter ) ;
	m_pcsxi->WriteStringData( ECSWideString( L"" ) ) ;
	m_pcsxi->WriteCodeData( &dwDummy, sizeof(dwDummy) ) ;
*/	//
	return	eslErrSuccess ;
}

// Until 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileUntil( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwUntil ) )
	{
		return	eslErrSuccess ;
	}
	if ( m_rwCtrlType != rwRepeat )
	{
		return	ESLErrorMsg
			( "Until 文に対応する Repeat 文がありません。" ) ;
	}
	EControlNest *	pNest = m_nestCtrl.GetLastAt( 0 ) ;
	ESLAssert( pNest != NULL ) ;
	ESLAssert( pNest->m_rwType == m_rwCtrlType ) ;
	//
	// 反復条件判定
	//
	m_pcsxi->FenceInstruction() ;
	//
	ESLError	err = eslErrSuccess ;
	if ( m_modeNakedCode )
	{
		err = CompileCodeNakedLocalDestruction( pNest, 0 ) ;
	}
	else
	{
		if ( pNest->m_lstLocalName.GetSize() > 0 )
		{
			m_pcsxi->WriteInstructionCode( csicLeave ) ;
		}
	}
	pNest->m_lstLocalName.RemoveAll( ) ;
	pNest->m_lstLocalObj.RemoveAll( ) ;
	//
	ECSTypeInfo	typeExpr ;
	BeginSakura2Optimize() ;
	err = CompileExpression( typeExpr, cssLine, 0, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	err = VerifyTypeBoolean( typeExpr ) ;
	if ( err )
	{
		return	err ;
	}
	FinishSakura2Optimize() ;
	FreeExpressionTemporaryStack( ECSSakura2Processor::regExpr0 ) ;
	//
	if ( m_modeNakedCode )
	{
		CompileCodeNakedLocalDestructionSaveRegister
					( pNest, ECSSakura2Processor::regExpr0 ) ;
		pNest->m_lstLocalName.RemoveAll( ) ;
		pNest->m_lstLocalObj.RemoveAll( ) ;
	}
	m_pcsxi->FlushAllRegisterAssigns() ;
	//
	pNest = m_nestCtrl.Pop( ) ;
	CompileCodeConditionalJump( pNest->m_dwBeginPos, false, false ) ;
	//
	// ネスト脱出
	//
	CommitBreakAddressOnNest( pNest, CompileCodeGetCurrent() ) ;
	//
	LeaveControlNest( pNest ) ;
	return	eslErrSuccess ;
}

// Switch 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileSwitch( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwSwitch ) )
	{
		return	eslErrSuccess ;
	}
	if ( m_rwCtrlType == rwData )
	{
		return	ESLErrorMsg
			( "Data ブロックの中で Switch 文が記述されています。" ) ;
	}
	else if ( m_rwCtrlType == rwStructure )
	{
		return	ESLErrorMsg
			( "Structure ブロックの中で Switch 文が記述されています。" ) ;
	}
	else if ( m_rwCtrlType == rwClass )
	{
		return	ESLErrorMsg
			( "Class ブロックの中で Switch 文が記述されています。" ) ;
	}
	else if ( m_rwCtrlType == rwEnumerator )
	{
		return	ESLErrorMsg
			( "Enumerator ブロックの中で Switch 文が記述されています。" ) ;
	}
	//
//	m_pcsxi->FenceInstruction() ;
	//
	EControlNest *	pNest = new EControlNest( m_dwImplementFlags ) ;
	m_rwCtrlType = rwSwitch ;
	pNest->m_rwType = rwSwitch ;
	pNest->m_fAutoEnter = true ;
	m_nestCtrl.Add( pNest ) ;
	//
	// 比較値計算
	//
	BeginSakura2Optimize() ;
	ESLError	err =
		CompileExpression( pNest->m_typeSwitch, cssLine, 0, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	if ( pNest->m_typeSwitch.IsVoid() )
	{
		return	ESLErrorMsg( "Switch の評価値が void です" ) ;
	}
	if ( m_modeNakedCode )
	{
		while ( pNest->m_typeSwitch.IsTypeReference() )
		{
			CompileCodeNakedUncoverReference( pNest->m_typeSwitch ) ;
		}
		MakeCommitValueToNakedRegister( pNest->m_typeSwitch ) ;
	}
	FinishSakura2Optimize() ;
	FreeExpressionTemporaryStack( ECSSakura2Processor::regExpr0 ) ;
	FreeExpressionRegister() ;
	//
	// 先頭の Case へ
	//
	pNest->m_dwBeginPos = CompileCodeJump( ) ;
	//
	return	eslErrSuccess ;
}

// EndSwitch 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileEndSwitch( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwEndSwitch ) )
	{
		return	eslErrSuccess ;
	}
	if ( m_rwCtrlType != rwSwitch )
	{
		return	ESLErrorMsg
			( "EndSwitch 文に対応する Switch 文がありません。" ) ;
	}
	EControlNest *	pNest = m_nestCtrl.GetLastAt( 0 ) ;
	ESLAssert( pNest != NULL ) ;
	ESLAssert( pNest->m_rwType == m_rwCtrlType ) ;
	//
	// 終端の暗黙 break 処理
	//
	pNest = m_nestCtrl.Pop( ) ;
	//
	pNest->m_lstBreak.Add( CompileCodeJump() ) ;
	//
	// Switch 文からのジャンプを完成
	//
	CompileCodeCommitJumpAddress
		( pNest->m_dwBeginPos, CompileCodeGetCurrent() ) ;
	//
	// 各 Case 値との比較ジャンプ
	//
	if ( m_modeNakedCode )
	{
		AllocateExpressionRegister() ;
	}
	const int	nCaseCount = pNest->m_lstCaseValues.GetSize() ;
	for ( int i = 0; i < nCaseCount; i ++ )
	{
		ECSObject *	pValue = pNest->m_lstCaseValues.GetAt( i ) ;
		if ( pValue == NULL )
		{
			continue ;
		}
		//
		// Switch 値
		//
		CompileLoadStackObject( 0 ) ;
		//
		ECSTypeInfo	typeSwitch = pNest->m_typeSwitch ;
		if ( m_modeNakedCode )
		{
			typeSwitch.SetLoadedRegister( GetExpressionRegister() ) ;
		}
		//
		// Case 値
		//
		ECSTypeInfo	typeCase ;
		ESLError	err = CompileImmediateObject( typeCase, pValue ) ;
		if ( err )
		{
			OutputError
				( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
		}
		//
		// 比較
		//
		ECSTypeInfo	typeResult ;
		err = CompileTypeCompare
			( typeResult, typeSwitch, typeCase, csctEqual ) ;
		if ( err )
		{
			OutputError
				( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
		}
		if ( !m_modeNakedCode )
		{
			//
			// 一致しない場合は次の Case へジャンプ
			//
			DWORD	dwNextJumpPos ;
			dwNextJumpPos = CompileCodeConditionalJump( false, false ) ;
			//
			// Case アドレスへジャンプ
			//
			CompileCodeFreeStack() ;
			CompileCodeJump( pNest->m_lstCaseAddress.GetAt(i) ) ;
			//
			// 一致しない場合のジャンプ完成
			//
			CompileCodeCommitJumpAddress
				( dwNextJumpPos, CompileCodeGetCurrent() ) ;
		}
		else
		{
			//
			// 一致する場合は Case アドレスへジャンプ
			//
			CompileCodeConditionalJump
				( pNest->m_lstCaseAddress.GetAt(i), true, false ) ;
		}
	}
	//
	// デフォルト時の処理
	//
	int	iDefaultCase = pNest->m_lstCaseValues.FindPtr( NULL ) ;
	//
	CompileCodeFreeStack() ;
	//
	if ( iDefaultCase >= 0 )
	{
		CompileCodeJump( pNest->m_lstCaseAddress.GetAt(iDefaultCase) ) ;
	}
	//
	// ネスト脱出
	//
	CommitBreakAddressOnNest( pNest, CompileCodeGetCurrent() ) ;
	//
	bool	fVarNest = (pNest->m_lstLocalName.GetSize() > 0) ;
	LeaveControlNest( pNest ) ;
	//
	if ( fVarNest )
	{
		return	ESLErrorMsg
			( "Switch ブロックに変数が定義されています" ) ;
	}
	return	eslErrSuccess ;
}

// Case 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileCase( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwCase ) )
	{
		return	eslErrSuccess ;
	}
	if ( m_rwCtrlType != rwSwitch )
	{
		return	ESLErrorMsg
			( "Case 文に対応する Switch 文がありません。" ) ;
	}
	EControlNest *	pNest = m_nestCtrl.GetLastAt( 0 ) ;
	ESLAssert( pNest != NULL ) ;
	ESLAssert( pNest->m_rwType == m_rwCtrlType ) ;
	//
	m_pcsxi->FenceInstruction() ;
	//
	// Case 値計算
	//
	ECSObject *	pValue = NULL ;
	ESLError	err = CalculateExpression( pValue, cssLine ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// 重複チェック
	//
	const int	nCaseCount = pNest->m_lstCaseValues.GetSize() ;
	for ( int i = 0; i < nCaseCount; i ++ )
	{
		ECSObject *	pCase = pNest->m_lstCaseValues.GetAt( i ) ;
		if ( (pCase != NULL) && (pCase->m_vtType == pValue->m_vtType) )
		{
			int	nResult ;
			if ( !pValue->Compare
				( m_ctxExpr, nResult, csctEqual, *pCase ) )
			{
				if ( nResult != 0 )
				{
					delete	pValue ;
					return	ESLErrorMsg
						( "Case に同じ値が重複して記述されています" ) ;
				}
			}
		}
	}
	//
	// Case 追加
	//
	pNest->m_lstCaseValues.SetAt( nCaseCount, pValue ) ;
	pNest->m_lstCaseAddress.SetAt
			( nCaseCount, CompileCodeGetCurrent() ) ;
	//
	m_pcsxi->FenceInstruction() ;
	//
	return	eslErrSuccess ;
}

// Default 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileDefault( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwDefault ) )
	{
		return	eslErrSuccess ;
	}
	if ( m_rwCtrlType != rwSwitch )
	{
		return	ESLErrorMsg
			( "Default 文に対応する Switch 文がありません。" ) ;
	}
	EControlNest *	pNest = m_nestCtrl.GetLastAt( 0 ) ;
	ESLAssert( pNest != NULL ) ;
	ESLAssert( pNest->m_rwType == m_rwCtrlType ) ;
	//
	m_pcsxi->FenceInstruction() ;
	//
	// 重複チェック
	//
	if ( pNest->m_lstCaseValues.FindPtr( NULL ) >= 0 )
	{
		return	ESLErrorMsg
			( "Default が重複して記述されています" ) ;
	}
	//
	// Default 追加
	//
	const int	nCaseCount = pNest->m_lstCaseValues.GetSize() ;
	pNest->m_lstCaseValues.SetAt( nCaseCount, NULL ) ;
	pNest->m_lstCaseAddress.SetAt
			( nCaseCount, CompileCodeGetCurrent() ) ;
	//
	m_pcsxi->FenceInstruction() ;
	//
	return	eslErrSuccess ;
}

// _m_fence 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileMemoryFence( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwMemoryFence ) )
	{
		return	eslErrSuccess ;
	}
	m_pcsxi->FenceInstruction() ;
	//
	m_pcsxi->WriteSakuraInstructionCode
			( ECSSakura2Processor::codeMemoryHint ) ;
	m_pcsxi->WriteByteCode
			( ECSSakura2Processor::mhcodeMemoryFence ) ;
	m_pcsxi->WriteByteCode( 0 ) ;
	//
	return	eslErrSuccess ;
}

// Template 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileTemplate( ECSSourceStream & cssLine )
{
	if ( m_pStatementCache != NULL )
	{
		return	ESLErrorMsg
			( "テンプレート内でテンプレートが定義されています" ) ;
	}
	//
	// テンプレート引数解釈
	//
	if ( cssLine.HasToComeChar( L"<" ) != L'<' )
	{
		return	ESLErrorMsg( "テンプレート引数が見つかりません" ) ;
	}
	if ( cssLine.HasToComeChar( L">" ) == L'>' )
	{
		return	ESLErrorMsg( "テンプレート引数がありません" ) ;
	}
	ETemplateDefinition *	pTemplate = new ETemplateDefinition ;
	ESLError	err ;
	for ( ; ; )
	{
		EWideString				wstrType = cssLine.GetAToken() ;
		TemplateArgumentType	tatType ;
		if ( wstrType == L"class" )
		{
			tatType = templateClassArgument ;
		}
		else if ( wstrType == L"int" )
		{
			tatType = templateIntArgument ;
		}
		else
		{
			m_strErrMsg =
				"\"" + EString( wstrType )
					+ "\" は不正なテンプレート引数型です" ;
			delete	pTemplate ;
			return	ESLErrorMsg( m_strErrMsg ) ;
		}
		EWideString	wstrTypeName = cssLine.GetAToken() ;
		err = VerifyUserSymbol( wstrTypeName ) ;
		if ( err )
		{
			delete	pTemplate ;
			return	err ;
		}
		ETemplateArgument *	pArg = new ETemplateArgument ;
		pArg->m_type = tatType ;
		pArg->m_name = wstrTypeName ;
		//
		pTemplate->m_arguments.Add( pArg ) ;
		//
		wchar_t	wchNext = cssLine.HasToComeChar( L",>" ) ;
		if ( wchNext == L'>' )
		{
			break ;
		}
		if ( wchNext != L',' )
		{
			delete	pTemplate ;
			return	ESLErrorMsg
				( "テンプレート引数が \'>\' で閉じられていません" ) ;
		}
	}
	//
	// テンプレート名取得
	//
	EWideString	wstrTemplateType = cssLine.GetAToken() ;
	int			iStatementIndex = cssLine.GetIndex() ;
	EWideString	wstrTemplateName ;
	int			iEndOfTemplateName = cssLine.GetIndex() ;
	if ( CompareReservedWord( L"Function", wstrTemplateType ) == 0 )
	{
		pTemplate->m_type = templateFunction ;
		pTemplate->m_rwEndOfImplementation = rwEndFunc ;
		m_modeNakedCode = IsCStyleCompatibleMode() ;
		//
		for ( ; ; )
		{
			if ( cssLine.HasToComeChar( L"(" ) == L'(' )
			{
				break ;
			}
			wstrTemplateName = cssLine.GetAToken() ;
			iEndOfTemplateName = cssLine.GetIndex() ;
			//
			if ( wstrTemplateName.IsEmpty() )
			{
				return	ESLErrorMsg
					( "テンプレート関数名が見つかりません" ) ;
			}
		}
	}
	else if ( CompareReservedWord( L"Class", wstrTemplateType ) == 0 )
	{
		pTemplate->m_type = templateClass ;
		pTemplate->m_rwEndOfImplementation = rwEndClass ;
		//
		for ( ; ; )
		{
			wstrTemplateName = cssLine.GetAToken() ;
			iEndOfTemplateName = cssLine.GetIndex() ;
			//
			if ( CompareReservedWord( L"Naked", wstrTemplateName ) != 0 )
			{
				break ;
			}
		}
	}
	else if ( CompareReservedWord( L"Structure", wstrTemplateType ) == 0 )
	{
		pTemplate->m_type = templateStruct ;
		pTemplate->m_rwEndOfImplementation = rwEndStruct ;
		//
		for ( ; ; )
		{
			wstrTemplateName = cssLine.GetAToken() ;
			iEndOfTemplateName = cssLine.GetIndex() ;
			//
			if ( CompareReservedWord( L"Naked", wstrTemplateName ) != 0 )
			{
				break ;
			}
		}
	}
	else
	{
		m_strErrMsg =
			"\'" + EString( wstrTemplateType )
				+ "\' は不正なテンプレート構文です" ;
		delete	pTemplate ;
		return	ESLErrorMsg( m_strErrMsg ) ;
	}
	//
	// テンプレート登録
	//
	err = VerifyUserSymbol( wstrTemplateName ) ;
	if ( err )
	{
		delete	pTemplate ;
		return	err ;
	}
	if ( iStatementIndex > iEndOfTemplateName )
	{
		delete	pTemplate ;
		return	ESLErrorMsg
			( "テンプレート構文を正常に解釈出来ませんでした" ) ;
	}
	EWideString	wstrSpaceName = GetCurrentSpaceName() ;
	EWideString	wstrGlobalName ;
	if ( !wstrSpaceName.IsEmpty() )
	{
		wstrGlobalName = wstrSpaceName + L"::" + wstrTemplateName ;
		pTemplate->m_namespace = wstrSpaceName ;
	}
	else
	{
		wstrGlobalName = wstrTemplateName ;
	}
	if ( m_staTemplate.GetAs( wstrGlobalName ) != NULL )
	{
		m_strErrMsg =
			"\'" + EString( wstrGlobalName )
				+ "\' が重複してテンプレート宣言されています" ;
		delete	pTemplate ;
		return	ESLErrorMsg( m_strErrMsg ) ;
	}
	m_staTemplate.SetAs( wstrGlobalName, pTemplate ) ;
	//
	// ブロック開始ステートメント設定
	//
	ETemplateStatement *	pStatement = new ETemplateStatement ;
	pStatement->m_stType = stReservedWord ;
	pStatement->m_strFilePath = m_strFilePath ;
	pStatement->m_nLineNum = m_nLineNum ;
	//
	pStatement->m_wstrStatement =
		cssLine.Middle( iStatementIndex, iEndOfTemplateName - iStatementIndex ) ;
	pStatement->m_wstrStatement += L"<" ;
	//
	int	i, nCount ;
	nCount = pTemplate->m_arguments.GetSize() ;
	for ( i = 0; i < nCount; i ++ )
	{
		ETemplateArgument *	pArg = pTemplate->m_arguments.GetAt( i ) ;
		if ( pArg != NULL )
		{
			if ( i > 0 )
			{
				pStatement->m_wstrStatement += L"," ;
			}
			pStatement->m_wstrStatement += pArg->m_name ;
		}
	}
	pStatement->m_wstrStatement += L">" ;
	pStatement->m_wstrStatement += cssLine.Middle( iEndOfTemplateName ) ;
	//
	switch ( pTemplate->m_type )
	{
	case	templateFunction:
		pStatement->m_rwType = rwFunction ;
		pStatement->m_wstrStatement += L" inline" ;
		break ;
	case	templateClass:
		pStatement->m_rwType = rwClass ;
		break ;
	case	templateStruct:
		pStatement->m_rwType = rwStructure ;
		break ;
	}
	pTemplate->m_statements.Add( pStatement ) ;
	//
	pTemplate->m_pPrevStatementCache = m_pStatementCache ;
	m_pStatementCache = pTemplate ;
	m_rwEndOfStatementCache = rwEndTemplate ;
	//
	// テンプレート引数一時定義（主に C スタイルモード用）
	//
	EControlNest *	pNest = new EControlNest( m_dwImplementFlags ) ;
	pNest->m_rwType = rwTemplate ;
	pNest->m_flagNakedMode = m_modeNakedCode ;
	//
	nCount = pTemplate->m_arguments.GetSize() ;
	for ( i = 0; i < nCount; i ++ )
	{
		ETemplateArgument *	pArg = pTemplate->m_arguments.GetAt( i ) ;
		if ( pArg == NULL )
		{
			continue ;
		}
		switch ( pArg->m_type )
		{
		case	templateClassArgument:
			{
				ECSStructure *	pStruct = new ECSStructure ;
				pStruct->m_pwszTag = pArg->m_name ;
				//
				ECSTypeInfo *	pTypeArg = new ECSTypeInfo( pStruct ) ;
				pNest->m_wstaTypeDef.Add( pArg->m_name, pTypeArg ) ;
			}
			break ;
		case	templateIntArgument:
			pNest->m_staConstant.SetAs
				( pArg->m_name, new ECSInteger( 1 ) ) ;
			break ;
		}
	}
	m_nestCtrl.Add( pNest ) ;
	//
	if ( !wstrSpaceName.IsEmpty() )
	{
		ENumArray<DWORD>	lstScope ;
		int	iLast = 0 ;
		for ( ; ; )
		{
			int	iNext = wstrSpaceName.Find( L"::", iLast ) ;
			if ( iNext < iLast )
			{
				break ;
			}
			lstScope.InsertAt( 0, iNext ) ;
			iLast = iNext + 2 ;
		}
		unsigned int	nScopeCount = lstScope.GetSize() ;
		pNest->m_lstUsingNamespace.Add( new EWideString( wstrSpaceName ) ) ;
		for ( unsigned int i = 0; i < nScopeCount; i ++ )
		{
			pNest->m_lstUsingNamespace.Add
				( new EWideString( wstrSpaceName.Left(lstScope[i]) ) ) ;
		}
	}
	//
	if ( (pTemplate->m_type == templateClass)
		|| (pTemplate->m_type == templateStruct) )
	{
		EControlNest *	pTempNest = new EControlNest( m_dwImplementFlags ) ;
		pTempNest->m_rwType =
			(pTemplate->m_type == templateClass) ? rwClass : rwStructure ;
		pTempNest->m_wstrName = wstrGlobalName ;
		pTempNest->m_wstrCurSpaceName = wstrGlobalName ;
		AddNecessaryUsingNamespace( pTempNest ) ;
		m_nestCtrl.Add( pTempNest ) ;
		//
		ECSClassInfo *	pClassTemplate = new ECSClassInfo ;
		pClassTemplate->SetName( wstrTemplateName ) ;
		pClassTemplate->SetGlobalName( wstrGlobalName ) ;
		//
		pTempNest->m_wstaClassDef.SetAs( wstrGlobalName, pClassTemplate ) ;
	}
	//
	return	eslErrSuccess ;
}

// EndTemplate 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileEndTemplate( ECSSourceStream & cssLine )
{
	ESLAssert( m_pStatementCache != NULL ) ;
	if ( m_pStatementCache != NULL )
	{
		EStatementCache *	pStatementCache = m_pStatementCache ;
		m_pStatementCache = pStatementCache->m_pPrevStatementCache ;
		pStatementCache->m_pPrevStatementCache = NULL ;
	}
	//
	EControlNest *	pNest = GetMostInnerNest( rwTemplate, rwTemplate ) ;
	if ( (pNest == NULL) || (pNest->m_rwType != rwTemplate) )
	{
		return	ESLErrorMsg
			( "EndTemplate 文が Template 文と対応していません" ) ;
	}
	for ( ; ; )
	{
		pNest = m_nestCtrl.Pop() ;
		if ( pNest == NULL )
		{
			break ;
		}
		ReservedWord	rwType = pNest->m_rwType ;
		LeaveControlNest( pNest ) ;
		if ( rwType == rwTemplate )
		{
			break ;
		}
	}
	return	eslErrSuccess ;
}

// Using 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileUsing( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwUsing ) )
	{
		return	eslErrSuccess ;
	}
	EWideString	wstrToken = cssLine.GetAToken() ;
	if ( CompareReservedWord( L"Namespace", wstrToken ) != 0 )
	{
		return	ESLErrorMsg
			( "Using 文は Namespace 指定のみ対応しています" ) ;
	}
	SYMBOL_NAMESPACE	snsSymbol = cssLine.GetAToken() ;
	ESLError	err = ParseFullNameSymbol( snsSymbol, cssLine ) ;
	if ( err )
	{
		return	err ;
	}
	ECSClassInfo *
		pClassInf = GetClassInfoAs( snsSymbol.wstrFullName ) ;
	if ( pClassInf == NULL )
	{
		m_strErrMsg = EString(snsSymbol.wstrFullName)
						+ " は定義されていない名前空間です" ;
		return	ESLErrorMsg( m_strErrMsg ) ;
	}
	if ( !IsUsingNamespaceNest( pClassInf->GetGlobalName() ) )
	{
		EControlNest *	pNest = m_nestCtrl.GetLastAt() ;
		if ( pNest != NULL )
		{
			pNest->m_lstUsingNamespace.Add
				( new EWideString( pClassInf->GetGlobalName() ) ) ;
		}
		else
		{
			m_lstUsingNamespace.Add
				( new EWideString( pClassInf->GetGlobalName() ) ) ;
		}
	}
	return	eslErrSuccess ;
}

// Friend 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileFriend( ECSSourceStream & cssLine )
{
	if ( IsCacheStatement( cssLine, rwUsing ) )
	{
		return	eslErrSuccess ;
	}
	if ( (m_rwCtrlType != rwStructure) && (m_rwCtrlType != rwClass) )
	{
		return	ESLErrorMsg
			( "Friend 文が Class 外で記述されています。" ) ;
	}
	EWideString	wstrToken = cssLine.GetAToken() ;
	if ( CompareReservedWord( L"Class", wstrToken ) != 0 )
	{
		wstrToken = cssLine.GetAToken() ;
	}
	SYMBOL_NAMESPACE	snsSymbol = cssLine.GetAToken() ;
	ESLError	err =
		ParseFullNameSymbol( snsSymbol, cssLine, false, false, true ) ;
	if ( err )
	{
		return	err ;
	}
	ECSClassInfo *
		pClassInf = GetClassInfoAs( snsSymbol.wstrFullName ) ;
	if ( pClassInf == NULL )
	{
		m_strErrMsg = EString(snsSymbol.wstrFullName)
							+ " は定義されていないクラス名です" ;
		return	ESLErrorMsg( m_strErrMsg ) ;
	}
	EControlNest *	pNest = m_nestCtrl.GetLastAt( 0 ) ;
	ESLAssert( pNest != NULL ) ;
	ESLAssert( pNest->m_rwType == m_rwCtrlType ) ;
	//
	ECSClassInfo *	pThisClass = GetClassInfoAs( pNest->m_wstrCurSpaceName ) ;
	if ( pThisClass == NULL )
	{
		m_strErrMsg = EString(pNest->m_wstrCurSpaceName)
								+ " クラス情報が見つかりません" ;
		return	ESLErrorMsg( m_strErrMsg ) ;
	}
	pThisClass->AddFriendClass( pClassInf->GetGlobalName() ) ;
	//
	return	eslErrSuccess ;
}

// ステートメント・キャッシュ
//////////////////////////////////////////////////////////////////////////////
bool ECSCompiler::IsCacheStatement
	( ECSSourceStream & cssLine, ReservedWord rwType, StatementType stType )
{
	if ( m_pStatementCache != NULL )
	{
		if ( rwType != m_rwEndOfStatementCache )
		{
			//
			// キャッシュ
			//
			ETemplateStatement *	pStatement = new ETemplateStatement ;
			pStatement->m_stType = stType ;
			pStatement->m_rwType = rwType ;
			pStatement->m_wstrStatement =
					cssLine.Middle( cssLine.GetIndex() ) ;
			pStatement->m_strFilePath = m_strFilePath ;
			pStatement->m_nLineNum = m_nLineNum ;
			//
			m_pStatementCache->m_statements.Add( pStatement ) ;
			//
			if ( (rwType == rwStructure)
				|| (rwType == rwClass) || (rwType == rwUnion) )
			{
				//
				// クラス・ブロック開始
				//
				EWideString	wstrName = cssLine.GetAToken() ;
				for ( ; ; )
				{
					if ( (CompareReservedWord( L"Naked", wstrName ) == 0)
						|| (CompareReservedWord( L"Native", wstrName ) == 0) )
					{
						wstrName = cssLine.GetAToken() ;
					}
					else
					{
						break ;
					}
				}
				EWideString	wstrSpaceName = GetCurrentSpaceName() ;
				if ( !wstrSpaceName.IsEmpty() )
				{
					wstrSpaceName += L"::" ;
				}
				EWideString	wstrGlobalName = wstrSpaceName + wstrName ;
				//
				EControlNest *	pTempNest = new EControlNest( m_dwImplementFlags ) ;
				pTempNest->m_rwType = rwType ;
				pTempNest->m_wstrName = wstrGlobalName ;
				pTempNest->m_wstrCurSpaceName = wstrGlobalName ;
				AddNecessaryUsingNamespace( pTempNest ) ;
				m_nestCtrl.Add( pTempNest ) ;
				//
				ECSClassInfo *	pClassTemplate = new ECSClassInfo ;
				pClassTemplate->SetName( wstrName ) ;
				pClassTemplate->SetGlobalName( wstrGlobalName ) ;
				//
				pTempNest->m_wstaClassDef.SetAs( wstrGlobalName, pClassTemplate ) ;
			}
			else if ( rwType == rwTypeDef )
			{
				//
				// TypeDef
				//
				EWideString	wstrName = cssLine.GetAToken() ;
				//
				EControlNest *	pNest = m_nestCtrl.GetLastAt(0) ;
				if ( pNest != NULL )
				{
					// 型の仮登録
					pNest->m_wstaTypeDef.SetAs( wstrName, new ECSTypeInfo() ) ;
				}
			}
			else if ( (rwType == rwEndStruct)
				|| (rwType == rwEndClass) || (rwType == rwEndUnion) )
			{
				//
				// クラス・ブロック終了
				//
				ReservedWord rwEndType ;
				switch ( rwType )
				{
				case	rwEndStruct:
					rwEndType = rwStructure ;
					break ;
				case	rwEndClass:
				default:
					rwEndType = rwClass ;
					break ;
				case	rwEndUnion:
					rwEndType = rwUnion ;
					break ;
				}
				EControlNest *	pNest = GetMostInnerNest( rwEndType, rwEndType ) ;
				if ( pNest != NULL )
				{
					for ( ; ; )
					{
						pNest = m_nestCtrl.Pop() ;
						if ( pNest == NULL )
						{
							break ;
						}
						ReservedWord	rwType = pNest->m_rwType ;
						LeaveControlNest( pNest ) ;
						if ( rwType == rwEndType )
						{
							break ;
						}
					}
				}
			}
		}
		else
		{
			EStatementCache *	pStatementCache = m_pStatementCache ;
			m_pStatementCache = pStatementCache->m_pPrevStatementCache ;
			pStatementCache->m_pPrevStatementCache = NULL ;
		}
		return	true ;
	}
	return	false ;
}

// ステートメント・キャッシュ・展開
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::ImplementStatementCache
	( const EStatementCache & statements,
		const char * pszNameForError, DWORD dwImplements )
{
	static const ReservedWord	rwDeclAll[] =
	{
		rwStructure,	rwEndStruct,
		rwClass,		rwEndClass,
		rwNamespace,	rwEndNamespace,
		rwUnion,		rwEndUnion,
		rwPublic,	rwProtected,	rwPrivate,
		rwInvalid
	} ;
	EString	strSaveFilePath = m_strFilePath ;
	int		nSaveLineNum = m_nLineNum ;
	DWORD	dwSaveImpleFlags = m_dwImplementFlags ;
	m_dwImplementFlags = dwImplements ;
	//
	int	nInFunction = 0 ;
	for ( int i = 0; i < (int) statements.m_statements.GetSize(); i ++ )
	{
		ETemplateStatement *
				pStatement = statements.m_statements.GetAt( i ) ;
		ESLAssert( pStatement != NULL ) ;
		if ( pStatement != NULL )
		{
			m_strFilePath = pStatement->m_strFilePath ;
			m_nLineNum = pStatement->m_nLineNum ;
			//
			ESLError		err = eslErrSuccess ;
			ECSSourceStream	cssLine = pStatement->m_wstrStatement ;
			cssLine.MoveIndex( 0 ) ;
			//
			ReservedWord	rwType ;
			bool			fDeclAll = false ;
			int				j ;
			switch ( pStatement->m_stType )
			{
			case	stReservedWord:
				rwType = pStatement->m_rwType ;
				for ( j = 0; rwDeclAll[j] != rwInvalid; j ++ )
				{
					if ( rwDeclAll[j] == rwType )
					{
						fDeclAll = true ;
						break ;
					}
				}
				if ( rwType == rwFunction )
				{
					if ( !(dwImplements & implementFunction) )
					{
						rwType = rwPrototype ;
					}
					else
					{
						nInFunction ++ ;
					}
				}
				if ( (nInFunction && (dwImplements & implementFunction))
					|| (!nInFunction && (dwImplements & implementDeclaration))
					|| (!nInFunction && fDeclAll) )
				{
					err = CompileReservedWord( rwType, cssLine ) ;
				}
				if ( (rwType == rwPrototype)
					&& (pStatement->m_rwType == rwFunction) )
				{
					nInFunction ++ ;
				}
				else if ( pStatement->m_rwType == rwEndFunc )
				{
					if ( nInFunction > 0 )
					{
						nInFunction -- ;
					}
				}
				break ;
			case	stExpression:
				if ( (nInFunction && (dwImplements & implementFunction))
					|| (!nInFunction && (dwImplements & implementDeclaration)) )
				{
					err = CompileExpressionStatement( cssLine ) ;
				}
				break ;
			case	stEnumeration:
				if ( (nInFunction && (dwImplements & implementFunction))
					|| (!nInFunction && (dwImplements & implementDeclaration)) )
				{
					err = CompileEnumerate( cssLine ) ;
				}
				break ;
			case	stAssembler1:
				if ( (nInFunction && (dwImplements & implementFunction))
					|| (!nInFunction && (dwImplements & implementDeclaration)) )
				{
					err = CompileAssembleLine1( cssLine ) ;
				}
				break ;
			case	stAssembler2:
				if ( (nInFunction && (dwImplements & implementFunction))
					|| (!nInFunction && (dwImplements & implementDeclaration)) )
				{
					err = CompileAssembleLine2( cssLine ) ;
				}
				break ;
			}
			if ( err )
			{
				if ( pszNameForError != NULL )
				{
					OutputError
						( pszNameForError, strSaveFilePath, nSaveLineNum ) ;
				}
				OutputError
					( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
			}
			FreeExpressionTemporary() ;
		}
	}
	if ( statements.m_rwEndOfImplementation != rwInvalid )
	{
		bool	fDeclAll = false ;
		for ( int j = 0; rwDeclAll[j] != rwInvalid; j ++ )
		{
			if ( rwDeclAll[j] == statements.m_rwEndOfImplementation )
			{
				fDeclAll = true ;
				break ;
			}
		}
		if ( (nInFunction && (dwImplements & implementFunction))
			|| (!nInFunction && (dwImplements & implementDeclaration))
			|| (!nInFunction && fDeclAll) )
		{
			ECSSourceStream	cssLine ;
			ESLError	err = CompileReservedWord
				( statements.m_rwEndOfImplementation, cssLine ) ;
			if ( err )
			{
				if ( pszNameForError != NULL )
				{
					OutputError
						( pszNameForError, strSaveFilePath, nSaveLineNum ) ;
				}
				OutputError
					( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
			}
			FreeExpressionTemporary() ;
		}
	}
	//
	m_strFilePath = strSaveFilePath ;
	m_nLineNum = nSaveLineNum ;
	m_dwImplementFlags = dwSaveImpleFlags ;
}

// インライン関数生成・登録
//////////////////////////////////////////////////////////////////////////////
ECSExecutionImageCompiler * ECSCompiler::CreateTemporaryInlineFunction
	( const wchar_t * pwszFuncName, DWORD dwFuncFlags, bool fNoGetDefined )
{
	//
	// 初期値イメージアドレスを登録
	//
	ECSExecutionImageCompiler *	pcsxiTemp ;
	pcsxiTemp = m_wstaInlineFuncs.GetAs( pwszFuncName ) ;
	if ( pcsxiTemp != NULL )
	{
		if ( fNoGetDefined )
		{
			return	NULL ;
		}
		else
		{
			return	pcsxiTemp ;
		}
	}
	pcsxiTemp = new ECSExecutionImageCompiler( m_pcsxiDst ) ;
	m_wstaInlineFuncs.SetAs( pwszFuncName, pcsxiTemp ) ;
	//
	if ( pcsxiTemp->GetFunctionAddress( pwszFuncName ) != NULL )
	{
		if ( fNoGetDefined )
		{
			return	NULL ;
		}
		else
		{
			return	pcsxiTemp ;
		}
	}
	const DWORD	dwBaseAddr = pcsxiTemp->AlignCodeBuffer( 8 ) ;
	pcsxiTemp->AddFunctionEntry
		( pwszFuncName, dwBaseAddr,
			dwFuncFlags | ECSTypeInfo::flagInline ) ;
	//
	return	pcsxiTemp ;
}

// インライン関数終端設定
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::SetEndOfTemporaryInlineFunction
	( ECSExecutionImageCompiler * pcsxiInline, const wchar_t * pwszFuncName )
{
	pcsxiInline->SetEndOfFunctionAddress
		( pwszFuncName, pcsxiInline->m_bufImage.GetLength() ) ;
}

// naked クラス初期値イメージ生成
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileCodeNakedClassInitImage( ECSClassInfo * pClassInf )
{
	ECSExecutionImageCompiler *	pcsxiTemp = m_pcsxi ;
	//
	// 初期値イメージアドレスを登録
	//
	EWideString	wstrNakedImageName = pClassInf->GetGlobalName() + L"::<image>" ;
	ECSExecutionImageCompiler *
		pcsxiImage = CreateTemporaryInlineFunction
						( wstrNakedImageName, ECSTypeInfo::flagConstant ) ;
	if ( pcsxiImage == NULL )
	{
		return	eslErrSuccess ;
	}
	m_pcsxi = pcsxiImage ;
	//
	// 初期値イメージ設定
	//
	const DWORD	dwBaseAddr = m_pcsxi->m_bufImage.GetLength() ;
	BYTE *	pbytImage =
		(BYTE*) m_pcsxi->m_bufImage.PutBuffer
							( pClassInf->GetNakedMemorySize() ) ;
	::eslFillMemory( pbytImage, 0, pClassInf->GetNakedMemorySize() ) ;
	//
	CompileCodeNakedClassInitImage( pClassInf, pbytImage, dwBaseAddr ) ;
	//
	m_pcsxi->m_bufImage.Flush( pClassInf->GetNakedMemorySize() ) ;
	//
	// 初期値イメージの終端アドレスを設定
	//
	m_pcsxi->SetEndOfFunctionAddress
		( wstrNakedImageName, CompileCodeGetCurrent() ) ;
	m_pcsxi = pcsxiTemp ; // &m_csxiInitFunc ;
	//
	return	eslErrSuccess ;
}

ESLError ECSCompiler::CompileCodeNakedClassInitImage
	( const ECSClassInfo * pClassInf,
					BYTE * pbytInitBuf, DWORD dwBaseAddr )
{
	//
	// 変数の初期値設定
	//
	int	i ;
	int	nVarCount = pClassInf->GetVariableCount() ;
	for ( i = 0; i < nVarCount; i ++ )
	{
		ECSTypeInfo *	pVarType = pClassInf->GetVariableAt( i ) ;
		if ( pVarType == NULL )
		{
			continue ;
		}
		ECSObject *	pType = pVarType->m_pValue ;
		if ( pType != NULL )
		{
			int	iOffset = pClassInf->GetVariableNakedOffsetAt( i ) ;
			if ( pType->m_vtType == csvtInteger )
			{
				ECSInteger *	pIntType = (ECSInteger*) pType ;
				ECSPointerReference::StoreBufferInteger
					( pbytInitBuf + iOffset,
						pIntType->GetValue(),
						pIntType->GetIntegerType() ) ;
			}
			else if ( pType->m_vtType == csvtReal )
			{
				ECSReal *	pRealType = (ECSReal*) pType ;
				ECSPointerReference::StoreBufferReal
					( pbytInitBuf + iOffset,
						pRealType->m_varReal,
						pRealType->m_vtRealType ) ;
			}
			else if ( pType->m_vtType == csvtArray )
			{
				ECSArray *	pArray = (ECSArray*) pType ;
				ECSObject *	pElementType = pArray->GetEndDefaultElement() ;
				if ( (pElementType != NULL)
					&& (pElementType->m_vtType == csvtObject)
					&& (pElementType->m_pClassInf != NULL) )
				{
					int	nElementPitch =
						pElementType->m_pClassInf->GetNakedMemorySize() ;
					int				nDim = pArray->GetDimension() ;
					unsigned int *	pBounds = new unsigned int[nDim] ;
					//
					nDim = pArray->GetDimensionSize( pBounds, nDim ) ;
					//
					int	j ;
					int	nElementCount = 1 ;
					for ( j = 0; j < nDim; j ++ )
					{
						nElementCount *= pBounds[j] ;
					}
					delete []	pBounds ;
					//
					for ( j = 0; j < nElementCount; j ++ )
					{
						CompileCodeNakedClassInitImage
							( pElementType->m_pClassInf,
								pbytInitBuf + iOffset + nElementPitch * j,
								dwBaseAddr + iOffset + nElementPitch * j ) ;
					}
				}
			}
			else if ( pType->m_vtType == csvtObject )
			{
				if ( pType->m_pClassInf != NULL )
				{
					if ( pType->m_pClassInf->IsIntegerEnumeratorType() )
					{
						ECSPointerReference::StoreBufferInteger
							( pbytInitBuf + iOffset, 0, csvtInteger ) ;
					}
					else if ( pType->m_pClassInf->IsRealEnumeratorType() )
					{
						ECSPointerReference::StoreBufferReal
							( pbytInitBuf + iOffset, 0.0, csvtReal ) ;
					}
					else
					{
						CompileCodeNakedClassInitImage
							( pType->m_pClassInf,
								pbytInitBuf + iOffset, dwBaseAddr + iOffset ) ;
					}
				}
			}
		}
	}
	//
	// 仮想関数ベクタ設定
	//
	EWideString	wstrVirtFuncVector =
					pClassInf->GetGlobalName() + L"::<vfvector>" ;
	int	nVirtFuncCount = pClassInf->GetVirtualFunctionCount() ;
	if ( nVirtFuncCount > 0 )
	{
		m_pcsxi->AddCodeRefFunctionAddress64
						( wstrVirtFuncVector, dwBaseAddr ) ;
		*((DWORD*)(pbytInitBuf + sizeof(DWORD)))
						= (DWORD) (ECSExecutionImage::roasCode << 24) ;
	}
	//
	CompileCodeNakedClassInitImage_VirtualVector
		( wstrVirtFuncVector, nVirtFuncCount,
			false, pClassInf, pbytInitBuf, dwBaseAddr ) ;
	//
	return	eslErrSuccess ;
}

// naked クラス初期値イメージに仮想関数ベクタ設定
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::CompileCodeNakedClassInitImage_VirtualVector
	( const wchar_t * pwszVectorName, int& iVirtFunc, bool fRootVector,
		const ECSClassInfo * pClassInf,
		BYTE * pbytInitBuf, DWORD dwVecBaseAddr )
{
	const int	nParentClassCount = pClassInf->GetParentClassCount() ;
	for ( int i = 0; i < nParentClassCount; i ++ )
	{
		ECSClassInfo::ParentClass *
			pParentClass = pClassInf->GetParentClassAt( i ) ;
		if ( (pParentClass == NULL) || (pParentClass->pClassInf == NULL) )
		{
			continue ;
		}
		ECSClassInfo *	pCastClassInf = pParentClass->pClassInf ;
		if ( pCastClassInf->GetVirtualFunctionCount() == 0 )
		{
			continue ;
		}
		ECSClassInfo::CastInfo *
			pCastInf = pClassInf->GetCastClassInfoAs
							( pCastClassInf->GetGlobalName() ) ;
		if ( (i != 0) || fRootVector )
		{
			EWideString	wstrVectorName = pwszVectorName ;
			wstrVectorName += L"+" ;
			wstrVectorName += EWideString( iVirtFunc * 8 ) ;
			//
			m_pcsxi->AddCodeRefFunctionAddress64
				( wstrVectorName, dwVecBaseAddr + pCastInf->nNakedOffset ) ;
			*((DWORD*)(pbytInitBuf + pCastInf->nNakedOffset + sizeof(DWORD)))
									= (DWORD) (ECSExecutionImage::roasCode << 24) ;
		}
		iVirtFunc += pCastClassInf->GetVirtualFunctionCount() ;
		//
		CompileCodeNakedClassInitImage_VirtualVector
			( pwszVectorName, iVirtFunc, false /*(i != 0)*/, pCastClassInf,
				pbytInitBuf + pCastInf->nNakedOffset,
				dwVecBaseAddr + pCastInf->nNakedOffset ) ;
	}
}

// naked クラス実行時キャストベクタ生成
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileCodeNakedClassCastVector( ECSClassInfo * pClassInf )
{
	ECSExecutionImageCompiler *	pcsxiTemp = m_pcsxi ;
	//
	// 実行時型キャストベクタを登録
	//
	EWideString	wstrVectorName = pClassInf->GetGlobalName() + L"::<rtcvector>" ;
	ECSExecutionImageCompiler *
		pcsxiVF = CreateTemporaryInlineFunction
						( wstrVectorName, ECSTypeInfo::flagConstant ) ;
	if ( pcsxiVF == NULL )
	{
		return	eslErrSuccess ;
	}
	m_pcsxi = pcsxiVF ;
	//
	// キャストベクタ・ヘッダ
	//
	const EWStrTagArray<ECSClassInfo::CastInfo> &
				wstaCast = pClassInf->GetCastClassArray() ;
	const int	nCastCount = wstaCast.GetSize() ;
	//
	DWORD *	pdwCastHeader = (DWORD*) m_pcsxi->m_bufImage.PutBuffer( 8 ) ;
	pdwCastHeader[0] = (DWORD) nCastCount ;
	pdwCastHeader[1] = 0 ;
	m_pcsxi->m_bufImage.Flush( 8 ) ;
	//
	// キャスタベクタ
	//
	for ( int i = 0; i < nCastCount; i ++ )
	{
		ECSClassInfo::CastInfo *	pCastInf = wstaCast.GetObjectAt( i ) ;
		ESLAssert( pCastInf != NULL ) ;
		ESLAssert( pCastInf->pClassInf != NULL ) ;
		if ( (pCastInf == NULL)
			|| (pCastInf->pClassInf == NULL) )
		{
			return	ESLErrorMsg( "実行時型キャスト情報の生成に失敗しました" ) ;
		}
		DWORD	dwCastOffset = pCastInf->nNakedOffset ;
		DWORD	dwClassIndex =
				GetClassInfoIndex( pCastInf->pClassInf->GetGlobalName() ) ;
		ESLAssert( (SDWORD) dwClassIndex >= 0 ) ;
		m_pcsxi->WriteClassIndex( dwClassIndex ) ;
		m_pcsxi->WriteCodeData( &dwCastOffset, sizeof(DWORD) ) ;
	}
	//
	// 実行時型キャストベクタの終端アドレスを設定
	//
	SetEndOfTemporaryInlineFunction( pcsxiVF, wstrVectorName ) ;
	//
	m_pcsxi = pcsxiTemp ;
	return	eslErrSuccess ;
}

// naked クラス仮想関数ベクタ生成
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileCodeNakedVirtualFuncVector
	( ECSClassInfo * pClassInf )
{
	ECSExecutionImageCompiler *	pcsxiTemp = m_pcsxi ;
	//
	// 仮想関数ベクタアドレスを登録
	//
	EWideString	wstrVectorName = pClassInf->GetGlobalName() + L"::<vfvector>" ;
	ECSExecutionImageCompiler *
		pcsxiVF = CreateTemporaryInlineFunction
						( wstrVectorName, ECSTypeInfo::flagConstant ) ;
	if ( pcsxiVF == NULL )
	{
		return	eslErrSuccess ;
	}
	m_pcsxi = pcsxiVF ;
	//
	// 仮想関数をベクタに登録
	//
	EObjArray<VIRT_OFFSET_GATE>	lstOffsetOverride ;
	const int	nParentClassCount = pClassInf->GetParentClassCount() ;
	const DWORD	dwBaseAddr = CompileCodeGetCurrent() ;
	//
	int	i, nFuncCount = pClassInf->GetFunctionCount() ;
	int	iVirtFunc = 0 ;
	for ( i = 0; i < nFuncCount; i ++ )
	{
		ECSClassInfo::MemberFunction *
					pFunc = pClassInf->GetFunctionAt( i ) ;
		if ( (pFunc == NULL)
			|| !(pFunc->GetAttribute() & ECSTypeInfo::flagVirtual) )
		{
			continue ;
		}
		ECSClassInfo::CastInfo *	pCastInf = pFunc->m_pClassCast ;
		ESLAssert( pCastInf != NULL ) ;
		if ( pCastInf == NULL )
		{
			continue ;
		}
		if ( !(pFunc->GetAttribute() & ECSTypeInfo::flagAbstract) )
		{
			const DWORD	dwAttr = pFunc->GetAttribute() ;
			if ( ((pCastInf->pClassInf != pClassInf)
						&& (pCastInf->nNakedOffset > 0))
				|| (dwAttr & (ECSTypeInfo::flagNakedCallGate
								| ECSTypeInfo::flagObjectCallGate
								| ECSTypeInfo::flagNativeObject)) )
			{
				VIRT_OFFSET_GATE *	pvog = new VIRT_OFFSET_GATE ;
				pvog->nIndex = iVirtFunc ;
				pvog->pPrototype = pFunc ;
				pvog->nThisOffset = pCastInf->nNakedOffset ;
				//
				lstOffsetOverride.Add( pvog ) ;
			}
			else
			{
				m_pcsxi->AddCodeRefFunctionAddress64
					( pFunc->GetGlobalName(), dwBaseAddr + iVirtFunc * 8 ) ;
			}
		}
		DWORD *	pdwVirtFuncAddr =
				(DWORD*) m_pcsxi->m_bufImage.PutBuffer( 8 ) ;
		pdwVirtFuncAddr[0] = 0 ;
		pdwVirtFuncAddr[1] = (DWORD) (ECSExecutionImage::roasCode << 24) ;
		m_pcsxi->m_bufImage.Flush( 8 ) ;
		//
		iVirtFunc ++ ;
	}
	//
	// 多重派生クラスの派生元クラス仮想関数ベクタ
	//
	CompileCodeNakedVirtualFuncVector_SuperClass
		( lstOffsetOverride, iVirtFunc, wstrVectorName,
				dwBaseAddr, pClassInf, pClassInf, 0, 0 ) ;
	//
	// 仮想関数ベクタの終端アドレスを設定
	//
	SetEndOfTemporaryInlineFunction( pcsxiVF, wstrVectorName ) ;
	//
	// 多重派生クラスでのオーバーライド関数のゲート関数生成
	//
	int	nCount = lstOffsetOverride.GetSize() ;
	for ( i = 0; i < nCount; i ++ )
	{
		VIRT_OFFSET_GATE *	pvog = lstOffsetOverride.GetAt( i ) ;
		ESLAssert( pvog != NULL ) ;
		//
		DWORD	dwRefAddr = dwBaseAddr + pvog->nIndex * 8 ;
		DWORD *	pdwFuncAddr =
			(DWORD*) m_pcsxi->m_bufImage.ModifyBuffer( dwRefAddr, 8 ) ;
		*pdwFuncAddr = m_pcsxi->m_bufImage.GetLength() ;
		//
		m_pcsxi->m_extCodeRef.Add( dwRefAddr ) ;
		//
		ESLError	err =
			CompileCodeNakedVirtualOfffsetGateFunc
				( pClassInf, pvog->pPrototype, pvog->nThisOffset ) ;
		if ( err )
		{
			return	err ;
		}
	}
//	m_pcsxi = m_pcsxiDst ;
	//
	m_pcsxi = pcsxiTemp ;
	return	eslErrSuccess ;
}

// naked 多重派生クラスの派生元クラス仮想関数ベクタ情報生成
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::CompileCodeNakedVirtualFuncVector_SuperClass
	( EObjArray<VIRT_OFFSET_GATE>& lstOffsetOverride, int& iVirtFunc,
		const wchar_t * pwszVectorName,
		DWORD dwVecBaseAddr, ECSClassInfo * pClassInf,
		ECSClassInfo * pSuperClassInf, int nNakedOffset, int iFuncOffset )
{
	const int	nParentClassCount = pSuperClassInf->GetParentClassCount() ;
	for ( int iParent = 0; iParent < nParentClassCount; iParent ++ )
	{
		ECSClassInfo::ParentClass *
			pParentClass = pSuperClassInf->GetParentClassAt( iParent ) ;
		if ( (pParentClass == NULL)
			|| (pParentClass->pClassInf == NULL) )
		{
			continue ;
		}
		ECSClassInfo *	pParentClassInf = pParentClass->pClassInf ;
		ECSClassInfo::CastInfo *
			pParentCastInf = pSuperClassInf->GetCastClassInfoAs
								( pParentClassInf->GetGlobalName() ) ;
		if ( pParentCastInf == NULL )
		{
			continue ;
		}
		if ( pParentClassInf->GetVirtualFunctionCount() == 0 )
		{
			continue ;
		}
		//
		EWideString	wstrVirtFuncVector = pwszVectorName ;
		wstrVirtFuncVector += L"+" ;
		wstrVirtFuncVector += EWideString( iVirtFunc * 8 ) ;
		m_pcsxi->AddFunctionEntry
			( wstrVirtFuncVector, dwVecBaseAddr + iVirtFunc * 8,
				ECSTypeInfo::flagConstant | ECSTypeInfo::flagInline ) ;
		//
		const int	nFuncCount = pParentClassInf->GetFunctionCount() ;
		for ( int i = 0; i < nFuncCount; i ++ )
		{
			ECSClassInfo::MemberFunction *
				pFunc = pClassInf->GetFunctionAt
							( iFuncOffset + pParentCastInf->iFuncOffset + i ) ;
			if ( (pFunc == NULL)
				|| !(pFunc->GetAttribute() & ECSTypeInfo::flagVirtual) )
			{
				continue ;
			}
			ECSClassInfo::CastInfo *	pCastInf = pFunc->m_pClassCast ;
			ESLAssert( pCastInf != NULL ) ;
			if ( pCastInf == NULL )
			{
				continue ;
			}
			if ( !(pFunc->GetAttribute() & ECSTypeInfo::flagAbstract) )
			{
				const DWORD	dwAttr = pFunc->GetAttribute() ;
				if ( ((pCastInf->pClassInf != pParentClassInf)
					&& (pCastInf->nNakedOffset
							!= nNakedOffset + pParentCastInf->nNakedOffset))
					|| (dwAttr & (ECSTypeInfo::flagNakedCallGate
								| ECSTypeInfo::flagObjectCallGate
								| ECSTypeInfo::flagNativeObject)) )
				{
					VIRT_OFFSET_GATE *	pvog = new VIRT_OFFSET_GATE ;
					pvog->nIndex = iVirtFunc ;
					pvog->pPrototype = pFunc ;
					pvog->nThisOffset =
						pCastInf->nNakedOffset
							- (nNakedOffset + pParentCastInf->nNakedOffset) ;
					//
					lstOffsetOverride.Add( pvog ) ;
				}
				else
				{
					m_pcsxi->AddCodeRefFunctionAddress64
						( pFunc->GetGlobalName(), dwVecBaseAddr + iVirtFunc * 8 ) ;
				}
			}
			DWORD *	pdwVirtFuncAddr =
					(DWORD*) m_pcsxi->m_bufImage.PutBuffer( 8 ) ;
			pdwVirtFuncAddr[0] = 0 ;
			pdwVirtFuncAddr[1] = (DWORD) (ECSExecutionImage::roasCode << 24) ;
			m_pcsxi->m_bufImage.Flush( 8 ) ;
			//
			iVirtFunc ++ ;
		}
		//
		CompileCodeNakedVirtualFuncVector_SuperClass
			( lstOffsetOverride, iVirtFunc, pwszVectorName,
				dwVecBaseAddr, pClassInf, pParentClassInf,
				nNakedOffset + pParentCastInf->nNakedOffset,
				iFuncOffset + pParentCastInf->iFuncOffset ) ;
	}
}

// naked クラスオーバーライド関数 this オフセットゲート関数生成
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileCodeNakedVirtualOfffsetGateFunc
	( ECSClassInfo * pClassInf,
		ECSPrototypeInfo * pPrototype, int iThisOffset )
{
	if ( pPrototype->IsNakedCall() )
	{
		NakedModeSaver	saver( *this ) ;
		m_modeNakedCode = true ;
		//
		m_pcsxi->InitializeAllRegisterAssigns() ;
		//
		int	regThis = AllocateExpressionRegister() ;
		int	regAddr = AllocateExpressionRegister() ;
		int	regDst = regThis ;
		if ( iThisOffset != 0 )
		{
			WriteSakuraLoadMemory
				( ECSSakura2Processor::addrBaseOffset32,
					ECSSakura2Processor::dataInt64,
					regDst, ECSSakura2Processor::regSP, 8 ) ;
			m_pcsxi->WriteSakuraAddRegRegImm32( regThis, regDst, iThisOffset ) ;
			WriteSakuraStoreMemory
				( ECSSakura2Processor::addrBaseOffset32,
					ECSSakura2Processor::dataInt64,
					regThis, ECSSakura2Processor::regSP, 8 ) ;
		}
		//
		if ( !(pPrototype->GetAttribute() & ECSTypeInfo::flagObjectCallGate) )
		{
			if ( !(pPrototype->GetAttribute() & ECSTypeInfo::flagNativeObject) )
			{
				m_pcsxi->WriteSakuraLoadInt64_FuncPtr
						( regAddr, pPrototype->GetGlobalName() ) ;
				m_pcsxi->WriteSakuraJumpReg( regAddr ) ;
			}
			else
			{
				m_pcsxi->WriteSakuraPopReg( regAddr ) ;
				m_pcsxi->WriteSakuraSysCallFunction( pPrototype->GetGlobalName() ) ;
				m_pcsxi->WriteSakuraJumpReg( regAddr ) ;
			}
			FreeExpressionRegister() ;
			FreeExpressionRegister() ;
		}
		else
		{
			FreeExpressionRegister() ;
			FreeExpressionRegister() ;
			//
			return	ESLErrorMsg
				( "naked モードから object モードへの"
					"コールゲートは現在対応していない機能です" ) ;
		}
		return	eslErrSuccess ;
	}
	//
	// 関数エントリ
	//
	ESLError	err ;
	err = CompileFunctionEntryGate
		( pClassInf, pPrototype, pPrototype->GetArgumentName(), true ) ;
	if ( err )
	{
		return	err ;
	}
	if ( pPrototype->GetAttribute() & ECSTypeInfo::flagVarArgument )
	{
		return	ESLErrorMsg( "仮想関数に可変長引数が指定されています" ) ;
	}
	//
	// this ポインタ修正
	//
	CompileCodeLoadRefVariable( csomStack, 0 ) ;
	CompileCodeOffsetPointerReference( iThisOffset ) ;
	//
	// 関数引数
	//
	EObjArray<ECSTypeInfo>	lstArg ;
	const int	nArgCount = pPrototype->GetArgumentCount() ;
	for ( int i = 0; i < nArgCount; i ++ )
	{
		lstArg.Add( new ECSTypeInfo( *pPrototype->GetArgumentAt(i) ) ) ;
		CompileCodeLoadRefVariable( csomStack, i + 1 ) ;
	}
	//
	// 関数呼び出し命令
	//
	CompileCallGlobalFunction( *pPrototype, lstArg, nArgCount + 1 ) ;
	//
	// return 命令
	//
	m_pcsxi->WriteInstructionCode( csicExReturn ) ;
	if ( pPrototype->GetReturnType().IsVoid() )
	{
		m_pcsxi->WriteByteCode( 0 ) ;
	}
	else
	{
		m_pcsxi->WriteByteCode( 1 ) ;
	}
	EControlNest *	pNest = m_nestCtrl.GetLastAt( 0 ) ;
	if ( pNest != NULL )
	{
		if ( (pClassInf != NULL)
			&& pClassInf->IsNakedMemoryClass()
			&& (pNest->m_wstrCurSpaceName ==
					pNest->m_wstrThisSpaceName
							+ L"::~" + pClassInf->GetName()) )
		{
		}
		else
		{
			pNest->m_fReturned = true ;
		}
	}
	//
	// 関数終了
	//
	ECSSourceStream	cssLine ;
	err = CompileEndFunc( cssLine ) ;
	if ( err )
	{
		return	err ;
	}
	return	eslErrSuccess ;
}

// syscall ゲート関数生成
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileCodeNakedNativeFuncGate( const wchar_t * pwszSysCall )
{
	ECSExecutionImageCompiler *	pcsxiTemp = m_pcsxi ;
	//
	// ゲート関数を登録
	//
	EWideString	wstrGateName = L"syscall " ;
	wstrGateName += pwszSysCall ;
	//
	ECSExecutionImageCompiler *
		pcsxiGate = CreateTemporaryInlineFunction
						( wstrGateName, ECSTypeInfo::flagNakedCall ) ;
	if ( pcsxiGate == NULL )
	{
		return	eslErrSuccess ;
	}
	m_pcsxi = pcsxiGate ;
	//
	// コード出力
	//
	const int	regRetAddr = AllocateExpressionRegister() ;
	//
	m_pcsxi->WriteSakuraPopReg( regRetAddr ) ;
	m_pcsxi->WriteSakuraSysCallFunction( pwszSysCall ) ;
	m_pcsxi->WriteSakuraJumpReg( regRetAddr ) ;
	//
	FreeExpressionRegister() ;
	//
	// ゲート関数の終端アドレスを設定
	//
	SetEndOfTemporaryInlineFunction( pcsxiGate, wstrGateName ) ;
	//
	m_pcsxi = pcsxiTemp ;
	return	eslErrSuccess ;
}

// 関数ネスト生成
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileFunctionEntryGate
	( ECSClassInfo * pClassInf,
		ECSPrototypeInfo * pPrototype,
		const EObjArray<EWideString> & lstArguments, bool fLocalFunc )
{
	//
	// 関数を登録
	//
	if ( pPrototype->GetAttribute() & ECSTypeInfo::flagInline )
	{
		ECSExecutionImageCompiler *	pcsxiFunc =
			m_wstaInlineFuncs.GetAs( pPrototype->GetGlobalName() ) ;
		if ( pcsxiFunc != NULL )
		{
			m_strErrMsg = "関数 \'"
				+ EString(pPrototype->GetGlobalName())
								+ "\' は既に定義されています。" ;
			return	ESLErrorMsg( m_strErrMsg ) ;
		}
		pcsxiFunc = new ECSExecutionImageCompiler( m_pcsxiDst ) ;
		m_wstaInlineFuncs.SetAs( pPrototype->GetGlobalName(), pcsxiFunc ) ;
		m_pcsxi = pcsxiFunc ;
	}
	else
	{
		m_pcsxi = m_pcsxiDst ;
	}
	m_pcsxi->InitializeAllRegisterAssigns() ;
	if ( !fLocalFunc )
	{
		if ( m_pcsxi->GetFunctionAddress
					( pPrototype->GetGlobalName() ) != NULL )
		{
			m_strErrMsg = "関数 \'"
				+ EString(pPrototype->GetGlobalName())
								+ "\' は既に定義されています。" ;
			return	ESLErrorMsg( m_strErrMsg ) ;
		}
		DWORD	dwFlags = pPrototype->GetAttribute() ;
		dwFlags &= ~(ECSTypeInfo::flagNakedCall
					| ECSTypeInfo::flagNakedCallGate
					| ECSTypeInfo::flagObjectCallGate) ;
		if ( pPrototype->IsNakedCodeMode() )
		{
			dwFlags |= ECSTypeInfo::flagNakedCall ;
		}
		m_pcsxi->AddFunctionEntry
			( pPrototype->GetGlobalName(),
				CompileCodeGetCurrent(), dwFlags ) ;
	}
	//
	// 関数引数
	//
	bool	fHasThisPointer = false ;
	bool	fNakedCodeMode = pPrototype->IsNakedCodeMode() ;
	EObjArray<ECSTypeInfo>	lstArgType ;
	EObjArray<EWideString>	lstArgName ;
	lstArgType = pPrototype->GetArgument() ;
	lstArgName = lstArguments ;
	if ( (pClassInf != NULL)
		&& !(pPrototype->GetAttribute() & ECSTypeInfo::flagStatic) )
	{
		if ( !fNakedCodeMode )
		{
			ECSReference *	pRefThis = new ECSReference ;
			pRefThis->SetOwnObject( new ECSStructure( pClassInf ) ) ;
			DWORD	dwFlags = pPrototype->GetAttribute()
										& ECSTypeInfo::flagConstant ;
			dwFlags |= ECSTypeInfo::flagPrivate ;
			lstArgType.InsertAt( 0, new ECSTypeInfo( pRefThis, dwFlags ) ) ;
			lstArgName.InsertAt( 0, new EWideString( L"this" ) ) ;
		}
		else
		{
			ECSPointer *	pPtrThis = new ECSPointer ;
			pPtrThis->SetOwnObject( new ECSStructure( pClassInf ) ) ;
			if ( pPrototype->GetAttribute() & ECSTypeInfo::flagConstant )
			{
				pPtrThis->m_fReadOnly = true ;
			}
			DWORD	dwFlags = pPrototype->GetAttribute()
										& ECSTypeInfo::flagConstant ;
			dwFlags |= ECSTypeInfo::flagPrivate ;
			lstArgType.InsertAt( 0, new ECSTypeInfo( pPtrThis, dwFlags ) ) ;
			lstArgName.InsertAt( 0, new EWideString( L"this" ) ) ;
		}
		fHasThisPointer = true ;
	}
	//
	// ネスト設定
	//
	EControlNest *	pNest = new EControlNest( m_dwImplementFlags ) ;
	const DWORD		dwFuncAttr = pPrototype->GetAttribute() ;
	m_rwCtrlType = rwFunction ;
	pNest->m_rwType = rwFunction ;
	pNest->m_wstrName = pPrototype->GetGlobalName() ;
	pNest->m_fGotoOccured = false ;
	pNest->m_nArgCount = lstArgType.GetSize( ) ;
	pNest->m_wstrThisSpaceName = pPrototype->GetNameSpace() ;
	pNest->m_wstrCurSpaceName = pPrototype->GetGlobalName() ;
	pNest->m_pFuncPrototype = pPrototype ;
	pNest->m_flagNakedMode = fNakedCodeMode ;
	AddNecessaryUsingNamespace( pNest ) ;
	m_nestCtrl.Add( pNest ) ;
	m_dwImplementFlags = implementAll ;
	//
	m_modeNakedCode = fNakedCodeMode ;
	if ( !m_modeNakedCode )
	{
		//
		// 関数の開始エントリ作成
		//
		DWORD	dwArgCount = lstArgType.GetSize( ) ;
		m_pcsxi->WriteInstructionCode( csicEnter ) ;
		m_pcsxi->WriteConstantString( pPrototype->GetGlobalName() ) ;
		m_pcsxi->WriteCodeData( &dwArgCount, sizeof(dwArgCount) ) ;
		//
		EncodeArgumentList( pNest, lstArgType, lstArgName ) ;
	}
	else
	{
		//
		// 関数のローカルフレーム生成
		//
		int	nFirstArg = 2 ;		// [bp] [ret ip] [arg0]
		if ( fHasThisPointer )
		{
			nFirstArg ++ ;		// [bp] [tp] [ret ip] [arg0]
			//
			m_pcsxi->WriteSakuraPushRegsImm8
				( ECSSakura2Processor::regBP, 2 ) ;
		}
		else
		{
			m_pcsxi->WriteSakuraPushReg
				( ECSSakura2Processor::regBP ) ;
		}
		m_pcsxi->WriteSakuraMoveRegReg
			( ECSSakura2Processor::regBP,
				ECSSakura2Processor::regSP ) ;
		pNest->m_addrLocalFrame = m_pcsxi->WriteSakuraAddSP( 0 ) ;
		//
		if ( fHasThisPointer )
		{
			int	regThis = ECSSakura2Processor::regTP ;
			WriteSakuraMoveLocal
				( false, ECSSakura2Processor::addrLocalOffset32,
					ECSSakura2Processor::dataInt64,
					regThis, nFirstArg * 8, 0, 0, true ) ;
		}
		m_pcsxi->ResetAllRegisterAssigns() ;
		m_pcsxi->FenceInstruction() ;
		//
		// 返り値受け渡し補正
		//
		if ( pPrototype->IsNakedCodeMode()
			&& pPrototype->IsArgumentToReturnObject() )
		{
			const ECSTypeInfo &	typeRet = pPrototype->GetReturnType() ;
			ECSReference *	pRetRet = new ECSReference ;
			pRetRet->SetOwnObject( typeRet.DuplicateType() ) ;
			//
			if ( fHasThisPointer )
			{
				lstArgType.InsertAt( 1, new ECSTypeInfo( pRetRet, 0 ) ) ;
				lstArgName.InsertAt( 1, new EWideString( "<return>" ) ) ;
			}
			else
			{
				lstArgType.InsertAt( 0, new ECSTypeInfo( pRetRet, 0 ) ) ;
				lstArgName.InsertAt( 0, new EWideString( "<return>" ) ) ;
			}
		}
		//
		// naked 関数引数アドレス決定
		//
		DWORD	dwArgCount = lstArgType.GetSize( ) ;
		DWORD	i ;
		bool	flagInvalidArgType = false ;
		for ( i = 0; i < dwArgCount; i ++ )
		{
			ECSTypeInfo *	pArgType = lstArgType.GetAt( i ) ;
			EWideString *	pwstrName = lstArgName.GetAt( i ) ;
			ESLAssert( pArgType != NULL ) ;
			ESLAssert( pwstrName != NULL ) ;
			//
			flagInvalidArgType |= !pArgType->IsNakedPrimitiveDataType() ;
			//
			ECSTypeInfo *	pTypeInf = new ECSTypeInfo( *pArgType ) ;
			if ( (pTypeInf->m_pValue != NULL)
				&& (pTypeInf->m_pValue->m_vtType == csvtReal) )
			{
				ECSReal *	pTypeReal = (ECSReal*) pTypeInf->m_pValue ;
				if ( pTypeReal->m_vtRealType == csvtReal32 )
				{
					pTypeReal->m_vtRealType = csvtReal64 ;
				}
			}
			pTypeInf->SetAddressingInfo
				( ECSSakura2Processor::regBP, (nFirstArg + i) * 8 ) ;
			//
			pNest->m_lstLocalObj.Add( pTypeInf ) ;
			pNest->m_lstLocalName.Add( *pwstrName ) ;
		}
		if ( flagInvalidArgType )
		{
			return	ESLErrorMsg
				( "関数の引数にオブジェクト型が直接指定されています" ) ;
		}
	}
	return	eslErrSuccess ;
}

// 構造体・クラス完成処理
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileFinalizeClassInfo( ECSClassInfo * pClassInf )
{
	ESLError	err ;
	EControlNest *	pNest = m_nestCtrl.GetLastAt( 0 ) ;
	ESLAssert( pNest != NULL ) ;
	pNest->m_fCommitBlock = true ;
	//
	bool	fDeclaration = true ;
	bool	fImplementation = true ;
	if ( pClassInf->m_phase != ECSClassInfo::phaseEmpty )
	{
		fDeclaration = (pClassInf->m_phase == ECSClassInfo::phaseDeclaration) ;
		fImplementation = (pClassInf->m_phase == ECSClassInfo::phaseImplement) ;
	}
	//
	if ( pClassInf->TestAbstractFunction() )
	{
		//
		// 抽象クラス判定
		//
		pClassInf->SetAttribute
			( pClassInf->GetAttribute()
					| ECSTypeInfo::flagAbstract ) ;
	}
	if ( pClassInf->IsNamespace() )
	{
		//
		// 名前空間の完了処理
		//
	}
	else if ( pClassInf->IsNakedMemoryClass() )
	{
		//
		// アライメントによってサイズ調整
		//
		if ( fDeclaration )
		{
			pClassInf->NormalizeNakedClassSize() ;
		}
		//
		// naked クラスに object が含まれているか？
		//
		if ( pClassInf->GetUnnakedObjectCount() > 0 )
		{
			pClassInf->SetAttribute
				( pClassInf->GetAttribute()
						& ~ECSTypeInfo::flagNakedBuffer ) ;
			return	ESLErrorMsg
				( "naked クラスに object が含まれています" ) ;
		}
		//
		// 親クラスに naked でないクラスが含まれていないかチェック
		//
		const int	nParentClassCount = pClassInf->GetParentClassCount() ;
		for ( int iParentClass = 0;
				iParentClass < nParentClassCount; iParentClass ++ )
		{
			ECSClassInfo::ParentClass *
				pParentClass = pClassInf->GetParentClassAt( iParentClass ) ;
			if ( (pParentClass != NULL)
				&& (pParentClass->pClassInf != NULL) )
			{
				if ( !pParentClass->pClassInf->IsNakedMemoryClass() )
				{
					OutputWarning
						( "naked クラスが object クラスから派生しています",
												m_strFilePath, m_nLineNum ) ;
				}
			}
		}
		//
		// デストラクタの判定
		//
		ECSPrototypeInfo *	pPrototypeDestructor = NULL ;
		ECSPrototypeInfo *	pPrototypeDelete = NULL ;
		if ( fDeclaration && pClassInf->IsNeedsNakedClassDestruction() )
		{
			ECSClassInfo::ListMemberFunction	lstFunc ;
			EPtrObjArray<ECSTypeInfo>			lstArg ;
			pClassInf->SearchFunctinoAs
				( lstFunc, L"<destructor>",
					lstArg, 0, true, true, m_modeNakedCode ) ;
			if ( lstFunc.GetSize() == 0 )
			{
				EString	strWarning =
					EString( pClassInf->GetGlobalName() )
						+ " クラスに消滅関数がありません。"
							"デフォルトの消滅関数が生成されます。" ;
				if ( IsCStyleCompatibleMode() )
				{
					OutputWarning3( strWarning, m_strFilePath, m_nLineNum ) ;
				}
				else
				{
					OutputWarning( strWarning, m_strFilePath, m_nLineNum ) ;
				}
				ECSPrototypeInfo	protoDestructor ;
				DWORD	dwFlags = ECSTypeInfo::flagInline ;
//				if ( m_dwModeFlags & flagDefualtNakedFunc )
				{
					dwFlags |= ECSTypeInfo::flagNakedCall ;
				}
				protoDestructor.SetReturnType( ECSTypeInfo() ) ;
				protoDestructor.SetAttribute( dwFlags ) ;
				protoDestructor.SetName( L"~" + pClassInf->GetName() ) ;
				protoDestructor.SetGlobalName
					( pClassInf->GetGlobalName()
								+ L"::~" + pClassInf->GetName() ) ;
				//
				pClassInf->AddOverrideFunction( protoDestructor ) ;
				//
				pPrototypeDestructor =
					pClassInf->GetThisFunctionAs( protoDestructor ) ;
				if ( pPrototypeDestructor == NULL )
				{
					return	ESLErrorMsg
						( "naked クラスのデフォルト消滅関数の定義に失敗しました" ) ;
				}
			}
			else if ( lstFunc[0].IsNakedCall()
				&& (lstFunc[0].GetAttribute() & ECSTypeInfo::flagVirtual) )
			{
				//
				// デストラクタが仮想関数の場合
				// delete も仮想関数として自動的に定義する
				//
				ECSPrototypeInfo	protoDelete ;
				DWORD	dwFlags = ECSTypeInfo::flagInline
									| ECSTypeInfo::flagVirtual
									| ECSTypeInfo::flagNakedCall ;
				protoDelete.SetReturnType( ECSTypeInfo() ) ;
				protoDelete.SetAttribute( dwFlags ) ;
				protoDelete.SetName( L"<delete>" ) ;
				protoDelete.SetGlobalName
					( pClassInf->GetGlobalName() + L"::<delete>" ) ;
				//
				pClassInf->AddOverrideFunction( protoDelete ) ;
				//
				pPrototypeDelete = pClassInf->GetThisFunctionAs( protoDelete ) ;
				if ( pPrototypeDelete == NULL )
				{
					return	ESLErrorMsg
						( "naked クラスのデフォルト delete 関数の定義に失敗しました" ) ;
				}
			}
		}
		//
		// 実行時型キャストベクタを生成
		//
		if ( fDeclaration )
		{
			err = CompileCodeNakedClassCastVector( pClassInf ) ;
			if ( err )
			{
				return	err ;
			}
		}
		//
		// 仮想関数ベクタ生成
		//
		if ( fDeclaration && (pClassInf->GetVirtualFunctionCount() > 0) )
		{
			pClassInf->NormalizeNakedOffsetForVirtualVector() ;
			//
			err = CompileCodeNakedVirtualFuncVector( pClassInf ) ;
			if ( err )
			{
				return	err ;
			}
		}
		//
		// クラス初期値イメージ生成
		//
		if ( fDeclaration )
		{
			CompileCodeNakedClassInitImage( pClassInf ) ;
		}
		//
		// naked クラスのデフォルトコンストラクタ定義
		//
		if ( fDeclaration
			&& pClassInf->IsNeedsNakedClassConstruction( true ) )
		{
			EObjArray<ECSTypeInfo>				lstArg ;
			ECSClassInfo::ListMemberFunction	lstFunc ;
			pClassInf->SearchFunctinoAs
					( lstFunc, pClassInf->GetName(),
						lstArg, 0, true, false, m_modeNakedCode ) ;
			if ( lstFunc.GetSize() == 0 )
			{
				EString	strWarning =
					EString( pClassInf->GetGlobalName() )
						+ " クラスに構築関数がありません。"
							"デフォルトの構築関数が生成されます。" ;
				if ( IsCStyleCompatibleMode() )
				{
					OutputWarning3( strWarning, m_strFilePath, m_nLineNum ) ;
				}
				else
				{
					OutputWarning( strWarning, m_strFilePath, m_nLineNum ) ;
				}
				ECSPrototypeInfo	protoConstructor ;
				DWORD	dwFlags = ECSTypeInfo::flagInline ;
//				if ( m_dwModeFlags & flagDefualtNakedFunc )
				{
					dwFlags |= ECSTypeInfo::flagNakedCall ;
				}
				protoConstructor.SetReturnType( ECSTypeInfo() ) ;
				protoConstructor.SetAttribute( dwFlags ) ;
				protoConstructor.SetName( pClassInf->GetName() ) ;
				protoConstructor.SetGlobalName
					( pClassInf->GetGlobalName()
								+ L"::" + pClassInf->GetName() ) ;
				//
				pClassInf->AddOverrideFunction( protoConstructor ) ;
				//
				ECSPrototypeInfo *	pPrototype =
					pClassInf->GetThisFunctionAs( protoConstructor ) ;
				if ( pPrototype == NULL )
				{
					return	ESLErrorMsg
						( "naked クラスのデフォルト構築関数の定義に失敗しました" ) ;
				}
				//
				err = CompileFunctionEntryGate
					( pClassInf, pPrototype, pPrototype->GetArgumentName() ) ;
				if ( err )
				{
					return	err ;
				}
				ECSSourceStream	cssLine ;
				err = CompileCodeConstructor( pClassInf, cssLine ) ;
				if ( err )
				{
					CompileEndFunc( cssLine ) ;
					return	err ;
				}
				err = CompileEndFunc( cssLine ) ;
				if ( err )
				{
					return	err ;
				}
			}
		}
		//
		// naked クラスのデフォルトデストラクタ定義
		//
		if ( pPrototypeDestructor != NULL )
		{
			err = CompileFunctionEntryGate
				( pClassInf, pPrototypeDestructor,
					pPrototypeDestructor->GetArgumentName() ) ;
			if ( err )
			{
				return	err ;
			}
			ECSSourceStream	cssLine ;
			if ( err )
			{
				CompileEndFunc( cssLine ) ;
				return	err ;
			}
			err = CompileEndFunc( cssLine ) ;
			if ( err )
			{
				return	err ;
			}
		}
		//
		// naked クラスのデフォルト delete 演算子定義
		//
		if ( pPrototypeDelete != NULL )
		{
			err = CompileCodeNakedClassDeleteOperator
							( pClassInf, pPrototypeDelete ) ;
			if ( err )
			{
				return	err ;
			}
		}
	}
	else
	{
		//
		// 親クラスに naked クラスが含まれていないかチェック
		//
		const int	nParentClassCount = pClassInf->GetParentClassCount() ;
		for ( int iParentClass = 0;
				iParentClass < nParentClassCount; iParentClass ++ )
		{
			ECSClassInfo::ParentClass *
				pParentClass = pClassInf->GetParentClassAt( iParentClass ) ;
			if ( (pParentClass != NULL)
				&& (pParentClass->pClassInf != NULL) )
			{
				if ( pParentClass->pClassInf->IsNakedMemoryClass() )
				{
					OutputWarning
						( "object クラスが naked クラスから派生しています",
												m_strFilePath, m_nLineNum ) ;
				}
			}
		}
	}
	//
	// クラス定義中の、クラス自身をテンプレート引数に持つ
	// テンプレート・インスタンスの実装
	//
	int	i, nCount ;
	if ( fImplementation )
	{
		nCount = pNest->m_lstPostImplTemplate.GetSize() ;
		for ( i = 0; i < nCount; i ++ )
		{
			EWideString *	pTemplateName =
						pNest->m_lstPostImplTemplate.GetAt( i ) ;
			if ( pTemplateName != NULL )
			{
				SYMBOL_NAMESPACE	snsSymbol ;
				snsSymbol.ParseSymbol( *pTemplateName ) ;
				int	iTempArg = snsSymbol.wstrName.Find( L'<' ) ;
				if ( iTempArg >= 0 )
				{
					ECSSourceStream		cssName =
							snsSymbol.wstrName.Middle( iTempArg ) ;
					snsSymbol.wstrName =
								snsSymbol.wstrName.Left( iTempArg ) ;
					if ( snsSymbol.wstrNamespace.IsEmpty() )
					{
						snsSymbol.wstrFullName = snsSymbol.wstrName ;
					}
					else
					{
						snsSymbol.wstrFullName =
							snsSymbol.wstrNamespace + L"::" + snsSymbol.wstrName ;
					}
					ParseTemplateArgument
						( snsSymbol, cssName, false, true, implementFunction ) ;
				}
			}
		}
		pNest->m_lstPostImplTemplate.RemoveAll() ;
	}
	//
	// クラスメンバの2パス実装
	//
	if ( fImplementation )
	{
		EString	strMsgForErr = EString(pClassInf->GetGlobalName()) ;
		strMsgForErr += " の実装" ;
		//
		nCount = pNest->m_lstDelayImplement.GetSize() ;
		for ( i = 0; i < nCount; i ++ )
		{
			EDelayImplementation *
				pImplementation = pNest->m_lstDelayImplement.GetAt( i ) ;
			if ( pImplementation != NULL )
			{
				ImplementStatementCache( *pImplementation, strMsgForErr ) ;
			}
		}
//		pNest->m_lstDelayImplement.RemoveAll() ;
	}
	//
	if ( pClassInf->m_phase == ECSClassInfo::phaseDeclaration )
	{
		pClassInf->m_phase = ECSClassInfo::phaseImplement ;
	}
	else if ( pClassInf->m_phase == ECSClassInfo::phaseImplement )
	{
		pClassInf->m_phase = ECSClassInfo::phaseCompleted ;
	}
	return	eslErrSuccess ;
}

// コンストラクタコード出力
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileCodeConstructor
	( ECSClassInfo * pClassInf, ECSSourceStream & cssLine )
{
	//
	// 構築パラメータ解釈
	//
	ESLError	err ;
	EWStrTagArray<EWideString>	wstaConstructorArg ;
	EWStrTagArray<int>			wstaValidSet ;
	if ( cssLine.HasToComeChar( L":" ) == L':' )
	{
		for ( ; ; )
		{
			EWideString	wstrName = cssLine.GetAToken() ;
			if ( wstrName == L"(" )
			{
				return	ESLErrorMsg
					( "構築関数のパラメータに名前がありません" ) ;
			}
			else if ( wstrName.IsEmpty() )
			{
				break ;
			}
			SYMBOL_NAMESPACE	snsSymbol = wstrName ;
			err = ParseFullNameSymbol( snsSymbol, cssLine ) ;
			if ( err )
			{
				return	err ;
			}
			wstrName = snsSymbol.wstrFullName ;
			if ( wstaConstructorArg.GetAs( wstrName ) != NULL )
			{
				m_strErrMsg = EString(wstrName)
								+ " が二重に指定されています" ;
				return	ESLErrorMsg( m_strErrMsg ) ;
			}
			if ( cssLine.HasToComeChar( L"(" ) != L'(' )
			{
				return	ESLErrorMsg( "構築関数のパラメータが見つかりません" ) ;
			}
			const ECSClassInfo *	pParentClassInf = GetClassInfoAs( wstrName ) ;
			if ( pParentClassInf != NULL )
			{
				wstrName = pParentClassInf->GetGlobalName() ;
			}
			EWideString	wstrArg =
				cssLine.GetEnclosedString( L')', m_dwModeFlags ) ;
			if ( cssLine.GetAt( cssLine.GetIndex() - 1 ) != L')' )
			{
				return	ESLErrorMsg( "\'(\' に対応する \')\' が見つかりません" ) ;
			}
			wstaConstructorArg.SetAs( wstrName, new EWideString(wstrArg) ) ;
			//
			if ( cssLine.HasToComeChar( L"," ) != L',' )
			{
				break ;
			}
		}
	}
	//
	// 親クラスの構築関数
	//
	unsigned int	i, nCount ;
	nCount = pClassInf->GetParentClassCount() ;
	for ( i = 0; i < nCount; i ++ )
	{
		ECSClassInfo::ParentClass *
			pParentClass = pClassInf->GetParentClassAt( i ) ;
		if ( (pParentClass == NULL)
			|| (pParentClass->pClassInf == NULL) )
		{
			continue ;
		}
		const EWideString &	wstrParentClassName =
				pParentClass->pClassInf->GetGlobalName() ;
		EWideString *	pwstrArg =
			wstaConstructorArg.GetAs( wstrParentClassName ) ;
		if ( pwstrArg == NULL )
		{
			int	iTempArg = wstrParentClassName.Find( L'<' ) ;
			if ( iTempArg >= 0 )
			{
				pwstrArg = wstaConstructorArg.GetAs
								( wstrParentClassName.Left(iTempArg) ) ;
				if ( pwstrArg != NULL )
				{
					wstaValidSet.Add( wstrParentClassName.Left(iTempArg), new int ) ;
				}
			}
			if ( (pwstrArg == NULL)
				&& !IsDefaultConstructor( *(pParentClass->pClassInf) ) )
			{
				continue ;
			}
		}
		wstaValidSet.Add( wstrParentClassName, new int ) ;
		//
		ECSClassInfo::CastInfo *	pCastInf =
			pClassInf->GetCastClassInfoAs( wstrParentClassName ) ;
		if ( pCastInf == NULL )
		{
			continue ;
		}
		BeginSakura2Optimize() ;
		//
		ECSTypeInfo	typeRefThis ;
		typeRefThis.MakeReferenceOf
			( ECSTypeInfo( new ECSStructure( pClassInf ) ) ) ;
		if ( m_modeNakedCode )
		{
			int	regThis = AllocateExpressionRegister() ;
			m_pcsxi->WriteSakuraMoveRegReg
				( regThis, ECSSakura2Processor::regTP ) ;
			//
			typeRefThis.SetLoadedRegister( regThis ) ;
		}
		else
		{
			CompileCodeLoadRefVariable( csomStack, 0 ) ;
		}
		ECSTypeInfo	typeAddressing ;
		err = CompileCastToParentClass
			( typeAddressing, *pCastInf, *pClassInf,
				typeRefThis, ECSTypeInfo::flagPrivate ) ;
		if ( err )
		{
			return	err ;
		}
		if ( pwstrArg == NULL )
		{
			err = CompileCallDefaultConstructor
					( *(pParentClass->pClassInf), false ) ;
		}
		else
		{
			ECSSourceStream	cssArg = *pwstrArg + L" )" ;
			const ECSClassInfo::MemberFunction *	pFunc ;
			err = CompileArgumentAndCallMemberFunction
				( cssArg, *(pParentClass->pClassInf), typeRefThis,
					pParentClass->pClassInf->GetName(), false,
					pFunc, ECSTypeInfo::flagPrivate2 ) ;
		}
		FinishSakura2Optimize() ;
		FreeExpressionTemporary() ;
		if ( err )
		{
			return	err ;
		}
	}
	//
	// メンバ変数の構築関数
	//
	unsigned int	nParentVarCount = pClassInf->GetParentVariableCount() ;
	nCount = pClassInf->GetVariableCount() ;
	for ( i = nParentVarCount; i < nCount; i ++ )
	{
		ECSTypeInfo *	pVarType = pClassInf->GetVariableAt( i ) ;
		if ( pVarType == NULL )
		{
			continue ;
		}
		EWideString *	pwstrArg =
			wstaConstructorArg.GetAs
				( pClassInf->GetVariableNameAt( i ) ) ;
		if ( (pwstrArg == NULL)
			&& !IsDefaultConstructor( *pVarType ) )
		{
			continue ;
		}
		wstaValidSet.Add( pClassInf->GetVariableNameAt( i ), new int ) ;
		//
		BeginSakura2Optimize() ;
		if ( pClassInf->IsNakedMemoryClass() )
		{
			if ( m_modeNakedCode )
			{
				int	regThis = AllocateExpressionRegister() ;
				m_pcsxi->WriteSakuraAddRegRegImm32
					( regThis, ECSSakura2Processor::regTP,
							pClassInf->GetVariableNakedOffsetAt( i ) ) ;
			}
			else
			{
				CompileCodeLoadRefVariable( csomStack, 0 ) ;
				//
				CompileCodeOffsetPointerReference
					( pClassInf->GetVariableNakedOffsetAt( i ) ) ;
			}
		}
		else
		{
			CompileCodeLoadRefVariable( csomThis, i ) ;
		}
		if ( pwstrArg == NULL )
		{
			err = CompileCallDefaultConstructor( *pVarType, false ) ;
		}
		else
		{
			if ( pVarType->m_pValue == NULL )
			{
				return	ESLErrorMsg( "void 変数に初期値を指定しています" ) ;
			}
			CSVariableType	csvtVarType =
				ECSTypeInfo::GetNakedMemoryType( pVarType->m_pValue ) ;
			if ( csvtVarType != csvtObject )
			{
				ECSTypeInfo	typeVar = *pVarType ;
				if ( m_modeNakedCode )
				{
					const int	regThis = GetExpressionRegister() ;
					typeVar.SetAddressingInfo( regThis, 0 ) ;
				}
				ECSTypeInfo	typeInit ;
				ECSSourceStream	cssInit = *pwstrArg ;
				err = CompileExpression( typeInit, cssInit ) ;
				if ( err )
				{
					return	err ;
				}
				ECSTypeInfo	typeTemp ;
				err = CompileTypeMoveOperate
					( typeTemp, typeVar, typeInit, csotNop, true ) ;
				if ( err )
				{
					return	err ;
				}
				FreeExpressionRegister() ;
			}
			else
			{
				const ECSClassInfo *
					pVarClassInf = GetTypeClassInfo( pVarType->m_pValue ) ;
				if ( pVarClassInf != NULL )
				{
					ECSSourceStream	cssArg = *pwstrArg + L" )" ;
					const ECSClassInfo::MemberFunction *	pFunc ;
					err = CompileArgumentAndCallMemberFunction
						( cssArg, *pVarClassInf, *pVarType,
							pVarClassInf->GetName/*GetGlobalName*/(), false,
							pFunc, ECSTypeInfo::flagPrivate2 ) ;
				}
				else
				{
					m_strErrMsg =
						EString(pVarClassInf->GetGlobalName())
									+ " クラス情報が見つかりません" ;
					return	ESLErrorMsg( m_strErrMsg ) ;
				}
			}
		}
		FinishSakura2Optimize() ;
		FreeExpressionTemporary() ;
		if ( err )
		{
			return	err ;
		}
	}
	//
	// 使用されなかった要素をチェック
	//
	nCount = wstaConstructorArg.GetSize() ;
	for ( i = 0; i < nCount; i ++ )
	{
		EWideString *	pwstrName = wstaConstructorArg.GetTagAt( i ) ;
		if ( pwstrName != NULL )
		{
			if ( wstaValidSet.GetAs( *pwstrName ) == NULL )
			{
				m_strErrMsg = EString(*pwstrName)
								+ " は不正な構築要素名です" ;
				return	ESLErrorMsg( m_strErrMsg ) ;
			}
		}
	}
	return	eslErrSuccess ;
}

// naked クラスメモリ初期化コード生成
//（スタックのトップはポインタで、処理後に破棄されない）
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileCodeNakedClassInitialize
	( const ECSClassInfo * pClassInf )
{
	if ( m_modeNakedCode )
	{
		const int	regThis = GetExpressionRegister() ;
		const int	nNakedSize = pClassInf->GetNakedMemorySize() ;
		if ( (nNakedSize <= 8*8) && ((nNakedSize % 8) == 0) && (nNakedSize > 0) )
		{
			ENumArray<int>	lstReg ;
			const int	regAcc = ECSSakura2Processor::regAcc ;
			int	i, j ;
			m_pcsxi->WriteSakuraLoadInt64_FuncPtr
					( regAcc, pClassInf->GetGlobalName() + L"::<image>" ) ;
			const int	nLoopStep = 8 * 4 ;
			if ( !(regThis & 0x01) )
			{
				AllocateExpressionRegister() ;
			}
			for ( j = 0; j < nNakedSize; j += nLoopStep )
			{
				for ( i = 0; (i < nLoopStep) && (i+j < nNakedSize); i += 8 )
				{
					int	regDst = AllocateExpressionRegister() ;
					lstReg.SetAt( ((i + j) / 8), regDst ) ;
					m_pcsxi->WriteSakuraLoadMemory
						( (((i + j) == 0) ? ECSSakura2Processor::addrBase
								: ECSSakura2Processor::addrBaseOffset32),
							ECSSakura2Processor::dataInt64,
							regDst, regAcc, i + j, 0, 0, true ) ;
				}
				for ( i = 0; (i < nLoopStep) && (i+j < nNakedSize); i += 8 )
				{
					int	regSrc = lstReg[(i + j) / 8] ;
					m_pcsxi->WriteSakuraStoreMemory
						( (((i + j) == 0) ? ECSSakura2Processor::addrBase
								: ECSSakura2Processor::addrBaseOffset32),
							ECSSakura2Processor::dataInt64,
							regSrc, regThis, i + j, 0, 0, true ) ;
				}
				for ( i = 0; (i < nLoopStep) && (i+j < nNakedSize); i += 8 )
				{
					FreeExpressionRegister() ;
				}
			}
			if ( !(regThis & 0x01) )
			{
				FreeExpressionRegister() ;
			}
		}
		else if ( nNakedSize > 0 )
		{
			m_pcsxi->WriteSakuraMoveRegReg
					( AllocateExpressionRegister(), regThis ) ;
			m_pcsxi->WriteSakuraLoadInt64_FuncPtr
					( AllocateExpressionRegister(),
						pClassInf->GetGlobalName() + L"::<image>" ) ;
			m_pcsxi->WriteSakuraAddRegRegImm32
					( AllocateExpressionRegister(),
						ECSSakura2Processor::regIntZero, nNakedSize ) ;
			//
			CompileNakedSystemCall( L"memmove", 3 ) ;
		}
	}
	else
	{
		CompileLoadStackObject( 0 ) ;
		CompileImmediateFunctionPointer
			( pClassInf->GetGlobalName() + L"::<image>" ) ;
		CompileCodePointerToAddress() ;
		CompileImmediateInteger( pClassInf->GetNakedMemorySize() ) ;
		//
		DWORD	dwArgCount = 3 ;
		m_pcsxi->WriteInstructionCode( csicCallNativeFunction ) ;
		m_pcsxi->WriteCodeData( &dwArgCount, sizeof(DWORD) ) ;
		m_pcsxi->WriteNativeFunctionIndex( L"memmove" ) ;
		//
		CompileCodeFreeStack() ;
	}
	return	eslErrSuccess ;
}

// naked クラスデストラクタ呼び出しコード生成
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileCodeNakedClassDestruction
			( const ECSClassInfo * pClassInf, bool fNoVirtual )
{
	if ( pClassInf == NULL )
	{
		return	ESLErrorMsg
			( "naked クラスの破棄処理でクラス情報が見つかりません" ) ;
	}
	if ( m_modeNakedCode && !pClassInf->IsNakedMemoryClass() )
	{
		return	CompileCodeNakedNativeObjectDestruction() ;
	}
	ECSTypeInfo	typeThis( new ECSStructure( pClassInf ), 0 ) ;
	//
	ECSClassInfo::ListMemberFunction	lstFunc ;
	EPtrObjArray<ECSTypeInfo>			lstArg ;
	pClassInf->SearchFunctinoAs
		( lstFunc, L"<destructor>", lstArg, 0, true, true, m_modeNakedCode ) ;
	if ( lstFunc.GetSize() == 0 )
	{
		OutputWarning
			( EString( pClassInf->GetGlobalName() )
				+ " クラスにデストラクタが見つかりませんでした",
										m_strFilePath, m_nLineNum ) ;
		CompileCodeNakedClassAfterDestruction( pClassInf ) ;
		CompileCodeFreeStack() ;
		return	eslErrSuccess ;
	}
	else if ( lstFunc.GetSize() > 1 )
	{
		m_strErrMsg =
				EString( pClassInf->GetGlobalName() )
					+ " クラスにデストラクタが複数あります" ;
		return	ESLErrorMsg( m_strErrMsg ) ;
	}
	if ( !lstFunc[0].GetReturnType().IsVoid() )
	{
		OutputWarning
			( EString( pClassInf->GetGlobalName() )
				+ " クラスのデストラクタに返り値が void ではありません",
											m_strFilePath, m_nLineNum ) ;
	}
	return	CompileCallMemberFunction
		( *pClassInf, typeThis,
			lstFunc[0], lstArg, 0, ECSTypeInfo::flagPrivate, fNoVirtual ) ;
}

ESLError ECSCompiler::CompileCodeNakedVariableDestruction
			( const ECSTypeInfo & typeVar )
{
	ECSObject *	pType = typeVar.m_pValue ;
	if ( pType != NULL )
	{
		if ( pType->m_vtType == csvtObject )
		{
			//
			// naked クラスオブジェクトのデストラクション
			//
			return	CompileCodeNakedClassDestruction
								( pType->m_pClassInf, false ) ;
		}
		else if ( pType->m_vtType == csvtArray )
		{
			//
			// 配列へのデストラクション
			//
			ECSArray *		pArray = (ECSArray*) pType ;
			ECSObject *		pElementType = pArray->GetEndDefaultElement() ;
			int				nDim = pArray->GetDimension() ;
			unsigned int *	pBounds = new unsigned int[nDim] ;
			nDim = pArray->GetDimensionSize( pBounds, nDim ) ;
			//
			int	nElementCount = 1 ;
			for ( int i = 0; i < nDim; i ++ )
			{
				nElementCount *= pBounds[i] ;
			}
			delete []	pBounds ;
			//
			CompileImmediateInteger( nElementCount ) ;
			//
			return	CompileCodeNakedVariableDestructionList
									( pElementType->m_pClassInf ) ;
		}
		else if ( (pType->m_vtType == csvtString)
				|| (pType->m_vtType == csvtHash) )
		{
			//
			// native オブジェクトのデストラクション
			//
			return	CompileCodeNakedNativeObjectDestruction() ;
		}
	}
	OutputWarning
		( "内部エラー：naked クラスのメンバ変数デストラクタはありません",
												m_strFilePath, m_nLineNum ) ;
	CompileCodeFreeStack() ;
	return	eslErrSuccess ;
}

ESLError ECSCompiler::CompileCodeNakedNativeObjectDestruction( void )
{
	if ( !m_modeNakedCode )
	{
		OutputWarning
			( "内部エラー：object モードから"
				" object のデストラクタを呼び出しています",
										m_strFilePath, m_nLineNum ) ;
		CompileCodeFreeStack() ;
		return	eslErrSuccess ;
	}
	int	regObjPtrRef = GetExpressionRegister() ;
	int	regObjPtr = regObjPtrRef ;
	WriteSakuraLoadMemory
		( ECSSakura2Processor::addrBase,
			ECSSakura2Processor::dataInt64, regObjPtr, regObjPtrRef, 0 ) ;
	if ( regObjPtr != regObjPtrRef )
	{
		m_pcsxi->WriteSakuraMoveRegReg( regObjPtrRef, regObjPtr ) ;
	}
	CompileNakedSystemCall( L"object_delete", 1 ) ;
	//
	return	eslErrSuccess ;
}

// naked クラスデストラクタ呼び出しコード生成（delete）
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileCodeNakedClassPointerDestruction
		( const ECSClassInfo * pClassInf,
			bool fNoVirtual, bool& fPointerDestruction )
{
	fPointerDestruction = false ;
	if ( !m_modeNakedCode || !pClassInf->IsNakedMemoryClass() )
	{
		return	CompileCodeNakedClassDestruction( pClassInf, fNoVirtual ) ;
	}
	ECSTypeInfo	typeThis( new ECSStructure( pClassInf ), 0 ) ;
	ECSClassInfo::ListMemberFunction	lstFunc ;
	EPtrObjArray<ECSTypeInfo>			lstArg ;
	pClassInf->SearchFunctinoAs
		( lstFunc, L"<delete>", lstArg, 0, false, true, m_modeNakedCode ) ;
	if ( lstFunc.GetSize() == 0 )
	{
		return	CompileCodeNakedClassDestruction( pClassInf, fNoVirtual ) ;
	}
	else if ( lstFunc.GetSize() > 1 )
	{
		m_strErrMsg =
				EString( pClassInf->GetGlobalName() )
					+ " クラスにデストラクタが複数あります" ;
		return	ESLErrorMsg( m_strErrMsg ) ;
	}
	fPointerDestruction = true ;
	return	CompileCallMemberFunction
		( *pClassInf, typeThis,
			lstFunc[0], lstArg, 0, ECSTypeInfo::flagPrivate, fNoVirtual ) ;
}

// デフォルトの delete 演算子関数生成
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileCodeNakedClassDeleteOperator
		( ECSClassInfo * pClassInf, ECSPrototypeInfo * pPrototype )
{
	ESLError	err ;
	err = CompileFunctionEntryGate
		( pClassInf, pPrototype, pPrototype->GetArgumentName() ) ;
	if ( err )
	{
		return	err ;
	}
	m_pcsxi->WriteSakuraMoveRegReg
		( AllocateExpressionRegister(), ECSSakura2Processor::regTP ) ;
	err = CompileCodeNakedClassDestruction( pClassInf, false ) ;
	if ( err )
	{
		ECSSourceStream	cssLine ;
		CompileEndFunc( cssLine ) ;
		return	err ;
	}
	m_pcsxi->WriteSakuraMoveRegReg
		( AllocateExpressionRegister(), ECSSakura2Processor::regTP ) ;
	CompileNakedSystemCall( L"free", 1 ) ;
	//
	ECSSourceStream	cssLine ;
	err = CompileEndFunc( cssLine ) ;
	if ( err )
	{
		return	err ;
	}
	return	eslErrSuccess ;
}

// naked クラス配列デストラクタ呼び出しコード生成
// object スタックには先頭ポインタ参照と個数がプッシュされている
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileCodeNakedVariableDestructionList
									( const ECSClassInfo * pClassInf )
{
	m_pcsxi->FenceInstruction() ;
	//
	int	regThis = GetExpressionRegister( 1 ) ;
	int	regCounter = GetExpressionRegister( 0 ) ;
	//
	ESLError	err ;
	DWORD	dwBeginLoop = CompileCodeGetCurrent() ;
	//
	// ループ終了判定
	//
	if ( m_modeNakedCode )
	{
		int	regCondition = AllocateExpressionRegister() ;
		m_pcsxi->WriteSakuraXorRegReg( regCondition, regCondition ) ;
		m_pcsxi->WriteSakuraCmpLtRegReg( regCondition, regCounter ) ;	// 0 < counter ?
	}
	else
	{
		CompileLoadStackObject( 0 ) ;			// counter > 0 ?
		CompileImmediateInteger( 0 ) ;
		CompileCodeCompare( csctGreaterThan ) ;
	}
	DWORD	dwJumpBreakRef ;
	dwJumpBreakRef = CompileCodeConditionalJump( false, false ) ;
	//
	// デストラクタ呼び出し
	//
	CompileLoadStackObject( 1 ) ;			// pointer
	err = CompileCodeNakedClassDestruction( pClassInf, false ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// ループカウンタ更新
	//
	if ( m_modeNakedCode )
	{
		m_pcsxi->WriteSakuraAddRegRegImm32( regCounter, regCounter, -1 ) ;
	}
	else
	{
		CompileImmediateInteger( 1 ) ;
		CompileCodeOperate( csotSub ) ;
	}
	//
	// ポインタ更新
	//
	if ( m_modeNakedCode )
	{
		m_pcsxi->WriteSakuraAddRegRegImm32
				( regThis, regThis, pClassInf->GetNakedMemorySize() ) ;
	}
	else
	{
		CompileCodeSwap( 0, 1 ) ;
		CompileCodeOffsetPointerReference( pClassInf->GetNakedMemorySize() ) ;
		CompileCodeSwap( 0, 1 ) ;
	}
	//
	// 次のループへ
	//
	m_pcsxi->FenceInstruction() ;
	CompileCodeJump( dwBeginLoop ) ;
	//
	// ループ完了
	//
	CompileCodeCommitJumpAddress
			( dwJumpBreakRef, CompileCodeGetCurrent() ) ;
	//
	CompileCodeFreeStack() ;
	CompileCodeFreeStack() ;
	//
	return	eslErrSuccess ;
}

// naked クラスデストラクタ呼び出しコード生成
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileCodeNakedClassAfterDestruction
	( const ECSClassInfo * pClassInf )
{
	//
	// メンバ変数のデストラクタ呼び出し
	//
	const int	nParentVarCount = pClassInf->GetParentVariableCount() ;
	const int	nVarCount = pClassInf->GetVariableCount() ;
	ESLError	err ;
	int			i ;
	for ( i = nParentVarCount; i < nVarCount; i ++ )
	{
		ECSTypeInfo *	pVarType = pClassInf->GetVariableAt( i ) ;
		if ( (pVarType == NULL)
			|| (pVarType->m_pValue == NULL)
			|| !ECSClassInfo::IsNeedsNakedVariableDestruction
								( pVarType->m_pValue, m_modeNakedCode ) )
		{
			continue ;
		}
		const int	nNakedOffset = pClassInf->GetVariableNakedOffsetAt( i ) ;
		if ( m_modeNakedCode )
		{
			int	regThis = GetExpressionRegister() ;
			int	regMember = AllocateExpressionRegister() ;
			m_pcsxi->WriteSakuraAddRegRegImm32
						( regMember, regThis, nNakedOffset ) ;
		}
		else
		{
			CompileLoadStackObject( 0 ) ;
			if ( nNakedOffset != 0 )
			{
				CompileCodeOffsetPointerReference( nNakedOffset ) ;
			}
		}
		err = CompileCodeNakedVariableDestruction( *pVarType ) ;
		if ( err )
		{
			return	err ;
		}
	}
	//
	// 親クラスデストラクタ呼び出し
	//
	const int	nParentClassCount = pClassInf->GetParentClassCount() ;
	for ( i = 0; i < nParentClassCount; i ++ )
	{
		ECSClassInfo::ParentClass *
			pParentClass = pClassInf->GetParentClassAt( i ) ;
		if ( (pParentClass == NULL)
			|| (pParentClass->pClassInf == NULL)
			|| !pParentClass->pClassInf->
				IsNeedsNakedClassDestruction( m_modeNakedCode ) )
		{
			continue ;
		}
		ECSClassInfo::CastInfo *
			pCastInf = pClassInf->GetCastClassInfoAs
							( pParentClass->pClassInf->GetGlobalName() ) ;
		if ( pCastInf == NULL )
		{
			continue ;
		}
		if ( m_modeNakedCode )
		{
			int	regThis = GetExpressionRegister() ;
			int	regParent = AllocateExpressionRegister() ;
			m_pcsxi->WriteSakuraAddRegRegImm32
						( regParent, regThis, pCastInf->nNakedOffset ) ;
		}
		else
		{
			CompileLoadStackObject( 0 ) ;
			if ( pCastInf->nNakedOffset != 0 )
			{
				CompileCodeOffsetPointerReference( pCastInf->nNakedOffset ) ;
			}
		}
		err = CompileCodeNakedClassDestruction
							( pParentClass->pClassInf, true ) ;
		if ( err )
		{
			return	err ;
		}
	}
	return	eslErrSuccess ;
}

// ガベージ・リスト登録処理
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileCodeNakedAddGarbageList( const ECSTypeInfo * pVarType )
{
	if ( !m_modeNakedCode || !IsNakedThrowableNest()
		|| (pVarType == NULL) || (pVarType->m_pValue == NULL)
		|| !ECSClassInfo::IsNeedsNakedVariableDestruction
										( pVarType->m_pValue, true ) )
	{
		return	eslErrSuccess ;
	}
	const int	nReleaseCount = pVarType->CalcNakedMemoryArraySize() ;
	if ( nReleaseCount == 0 )
	{
		return	eslErrSuccess ;
	}
	//
	// 解放情報の準備
	//
	const ECSClassInfo *
					pObjectClassInf = pVarType->GetObjectClassInfo( *this ) ;
	const ECSClassInfo *
					pNakedClassInf = pVarType->GetNakedMemoryClassInfo() ;
	ECSClassInfo::MemberFunction *
					pDestructor = NULL ;
	ECSTypeInfo *	pGarbageBuf = NULL ;
	//
	bool		fRuntimeLoop = false ;
	DWORD		dwLoopBegin ;
	int			nRuntimeCount = 1 ;
	int			nNakedElementSize ;
	int			nCastOffset = 0 ;
	if ( pObjectClassInf != NULL )
	{
		nNakedElementSize = 8 ;
		CompileCodeNakedNativeFuncGate( L"object_delete" ) ;
	}
	else if ( pNakedClassInf != NULL )
	{
		ECSClassInfo::ListMemberFunction	lstFunc ;
		EPtrObjArray<ECSTypeInfo>			lstArg ;
		pNakedClassInf->SearchFunctinoAs
			( lstFunc, L"<destructor>", lstArg, 0, true, true, m_modeNakedCode ) ;
		if ( lstFunc.GetSize() == 0 )
		{
			OutputWarning
				( EString( pNakedClassInf->GetGlobalName() )
					+ " クラスにデストラクタが見つかりませんでした",
											m_strFilePath, m_nLineNum ) ;
			return	eslErrSuccess ;
		}
		pDestructor = lstFunc.GetAt(0) ;
		//
		if ( pDestructor->m_wstrClass != pNakedClassInf->GetGlobalName() )
		{
			ECSClassInfo::CastInfo *
				pCast = pNakedClassInf->GetCastParentClassAs
										( pDestructor->m_wstrClass ) ;
			if ( pCast != NULL )
			{
				nCastOffset = pCast->nNakedOffset ;
			}
		}
		//
		nNakedElementSize = pNakedClassInf->GetNakedMemorySize() ;
	}
	else
	{
		return	eslErrSuccess ;
	}
	const int	regCounter = AllocateExpressionRegister() ;
	const int	regPtrNext = AllocateExpressionRegister() ;
	const int	regTemp = AllocateExpressionRegister() ;
	if ( nReleaseCount > 4 )
	{
		m_pcsxi->WriteSakuraMoveRegReg
			( regCounter, ECSSakura2Processor::regIntZero ) ;
		dwLoopBegin = CompileCodeGetCurrent() ;
		fRuntimeLoop = true ;
	}
	else
	{
		nRuntimeCount = nReleaseCount ;
	}
	LoadCommitValueToNakedRegister( regPtrNext, *pVarType ) ;
	pGarbageBuf = CompileAllocateNakedTemporaryBuffer( nReleaseCount * 3 ) ;
	//
	for ( int i = 0; i < nRuntimeCount; i ++ )
	{
		//
		// 解放関数アドレスとオブジェクトポインタをロード
		//
		const int	regDestructor = AllocateExpressionRegister() ;
		const int	regObjPtr = AllocateExpressionRegister() ;
		if ( pObjectClassInf != NULL )
		{
			// native object
			int	regLoadPtr = regObjPtr ;
			m_pcsxi->WriteSakuraLoadInt64_FuncPtr
				( regDestructor, L"syscall object_delete" ) ;
			if ( fRuntimeLoop )
			{
				WriteSakuraLoadMemory
					( ECSSakura2Processor::addrBaseIndex,
						ECSSakura2Processor::dataInt64, regLoadPtr,
						ECSSakura2Processor::regZeroPtr, 0, regPtrNext, 0, true ) ;
			}
			else
			{
				WriteSakuraLoadMemory
					( regLoadPtr, *pVarType,
						false, i * nNakedElementSize, true ) ;
			}
			if ( regLoadPtr != regObjPtr )
			{
				m_pcsxi->WriteSakuraMoveRegReg( regObjPtr, regLoadPtr ) ;
			}
		}
		else
		{
			// naked object
			m_pcsxi->WriteSakuraLoadInt64_FuncPtr
				( regDestructor, pDestructor->GetGlobalName() ) ;
			if ( fRuntimeLoop )
			{
				if ( nCastOffset != 0 )
				{
					m_pcsxi->WriteSakuraAddRegRegImm32
							( regObjPtr, regPtrNext, nCastOffset ) ;
				}
				else
				{
					m_pcsxi->WriteSakuraMoveRegReg( regObjPtr, regPtrNext ) ;
				}
			}
			else
			{
				m_pcsxi->WriteSakuraAddRegRegImm32
					( regObjPtr, regPtrNext,
						i * nNakedElementSize + nCastOffset ) ;
			}
		}
		//
		// チェインに追加
		//
		const int	regGarbagePtr = AllocateExpressionRegister() ;
		ECSTypeInfo	typeBuf( new ECSInteger ) ;
		typeBuf.SetAddressingInfo( *pGarbageBuf ) ;
		if ( fRuntimeLoop )
		{
			m_pcsxi->WriteSakuraMulRegRegImm32
						( regGarbagePtr, regCounter, 24 ) ;
			ESLAssert( typeBuf.m_regIndex == -1 ) ;
			typeBuf.m_regIndex = regGarbagePtr ;
			typeBuf.m_scaleIndex = 0 ;
		}
		else
		{
			typeBuf.m_addrOffset += i * 24 ;
		}
		if ( pGarbageBuf != NULL )
		{
			WriteSakuraStoreMemory
				( ECSSakura2Processor::regYP, typeBuf, true, 0, true ) ;
			WriteSakuraStoreMemory
				( regDestructor, typeBuf, true, 8, true ) ;
			WriteSakuraStoreMemory
				( regObjPtr, typeBuf, true, 16, true ) ;
			LoadCommitValueToNakedRegister
				( ECSSakura2Processor::regYP, typeBuf ) ;
		}
		FreeExpressionRegister() ;
		FreeExpressionRegister() ;
		FreeExpressionRegister() ;
	}
	if ( fRuntimeLoop )
	{
		m_pcsxi->WriteSakuraAddRegRegImm32
			( regTemp, ECSSakura2Processor::regIntZero, nReleaseCount ) ;
		m_pcsxi->WriteSakuraAddRegRegImm32( regCounter, regCounter, 1 ) ;
		m_pcsxi->WriteSakuraAddRegRegImm32
			( regPtrNext, regPtrNext, nNakedElementSize ) ;
		m_pcsxi->WriteSakuraCmpNeRegReg( regTemp, regCounter ) ;
		CompileCodeCommitJumpAddress
			( m_pcsxi->WriteSakuraCJumpOffset32( regTemp, 0 ), dwLoopBegin ) ;
	}
	FreeExpressionRegister() ;
	FreeExpressionRegister() ;
	FreeExpressionRegister() ;
	return	eslErrSuccess ;
}

// naked 関数ローカル変数のデストラクタ呼び出しコード生成
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileCodeNakedFunctionDestruction( void )
{
	for ( int i = 0; i < (int) m_nestCtrl.GetSize(); i ++ )
	{
		EControlNest *	pNest = m_nestCtrl.GetLastAt( i ) ;
		if ( pNest == NULL )
		{
			continue ;
		}
		ESLError	err = CompileCodeNakedLocalDestruction( pNest, i ) ;
		if ( err )
		{
			OutputError
				( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
		}
		if ( pNest->m_rwType == rwTry )
		{
			m_pcsxi->WriteSakuraAddRegRegImm32
				( ECSSakura2Processor::regSP,
					ECSSakura2Processor::regXP, 8 ) ;
			m_pcsxi->WriteSakuraPopRegsImm8
				( ECSSakura2Processor::regXP, 2 ) ;
			m_pcsxi->WriteSakuraPopRegsImm8
				( ECSSakura2Processor::regBP, 2 ) ;
		}
		else if ( pNest->m_rwType == rwFunction )
		{
			break ;
		}
	}
	return	eslErrSuccess ;
}

ESLError ECSCompiler::CompileCodeNakedLocalDestruction( EControlNest * pNest, int iNest )
{
	UpdateNestLocalFrameBase() ;
	//
	const bool	fNakedThrowableNest = IsNakedThrowableNest( iNest ) ;
	int			nReleaseCount = 0 ;
	for ( int i = 0; i < (int) pNest->m_lstLocalObj.GetSize(); i ++ )
	{
		ECSTypeInfo *	pTypeVar = pNest->m_lstLocalObj.GetLastAt( i ) ;
		if ( pTypeVar == NULL )
		{
			continue ;
		}
		if ( (pTypeVar->m_pValue == NULL)
			|| !pTypeVar->IsAddressingInfo() )
		{
			continue ;
		}
		if ( !ECSClassInfo::IsNeedsNakedVariableDestruction
									( pTypeVar->m_pValue, true ) )
		{
			continue ;
		}
		if ( fNakedThrowableNest )
		{
			nReleaseCount += pTypeVar->CalcNakedMemoryArraySize() ;
		}
		else
		{
			int	regThis = AllocateExpressionRegister() ;
			ESLAssert( pTypeVar->m_regBase >= 0 ) ;
			ESLAssert( pTypeVar->m_regIndex < 0 ) ;
			m_pcsxi->FlushLocalBoundsAssignedRegisters
				( pTypeVar->m_addrOffset,
					pTypeVar->m_addrOffset + pTypeVar->SizeOfOnNakedMemory() ) ;
			m_pcsxi->WriteSakuraAddRegRegImm32
				( regThis, pTypeVar->m_regBase, pTypeVar->m_addrOffset ) ;
			//
			ESLError	err = CompileCodeNakedVariableDestruction( *pTypeVar ) ;
			if ( err )
			{
				return	err ;
			}
		}
	}
	if ( fNakedThrowableNest && (nReleaseCount > 0) )
	{
		m_pcsxi->FlushAllRegisterAssigns() ;
		CompileNakedPushExpressionTemporary() ;
		//
		bool		fRuntimeLoop = false ;
		DWORD		dwLoopBegin ;
		int			nRuntimeCount = 1 ;
		const int	regCounter = AllocateExpressionRegister() ;
		const int	regTemp = AllocateExpressionRegister() ;
		if ( nReleaseCount > 4 )
		{
			m_pcsxi->WriteSakuraAddRegRegImm32
				( regCounter, ECSSakura2Processor::regIntZero, nReleaseCount ) ;
			dwLoopBegin = CompileCodeGetCurrent() ;
			m_pcsxi->WriteSakuraPushReg( regCounter ) ;
			fRuntimeLoop = true ;
		}
		else
		{
			nRuntimeCount = nReleaseCount ;
		}
		for ( int i = 0; i < nRuntimeCount; i ++ )
		{
			int	regDestructor = AllocateExpressionRegister() ;
			int	regObjectPtr = AllocateExpressionRegister() ;
			m_pcsxi->WriteSakuraLoadMemory
				( ECSSakura2Processor::addrBaseOffset32,
					ECSSakura2Processor::dataInt64, regDestructor,
					ECSSakura2Processor::regYP, 8, 0, 0, true ) ;
			m_pcsxi->WriteSakuraLoadMemory
				( ECSSakura2Processor::addrBaseOffset32,
					ECSSakura2Processor::dataInt64, regObjectPtr,
					ECSSakura2Processor::regYP, 16, 0, 0, true ) ;
			//
			int	regNextYP = ECSSakura2Processor::regYP ;
			m_pcsxi->WriteSakuraLoadMemory
				( ECSSakura2Processor::addrBase,
					ECSSakura2Processor::dataInt64, regNextYP,
					ECSSakura2Processor::regYP, 0, 0, 0, true ) ;
			if ( regNextYP != ECSSakura2Processor::regYP )
			{
				m_pcsxi->WriteSakuraMoveRegReg
					( ECSSakura2Processor::regYP, regNextYP ) ;
			}
			m_pcsxi->WriteSakuraPushReg( regObjectPtr ) ;
			m_pcsxi->WriteSakuraCallReg( regDestructor ) ;
			m_pcsxi->WriteSakuraAddSP( 8 ) ;
			//
			FreeExpressionRegister() ;
			FreeExpressionRegister() ;
		}
		if ( fRuntimeLoop )
		{
			m_pcsxi->WriteSakuraPopReg( regCounter ) ;
			m_pcsxi->WriteSakuraMoveRegReg
				( regTemp, ECSSakura2Processor::regIntZero ) ;
			m_pcsxi->WriteSakuraAddRegRegImm32( regCounter, regCounter, -1 ) ;
			m_pcsxi->WriteSakuraCmpNeRegReg( regTemp, regCounter ) ;
			CompileCodeCommitJumpAddress
				( m_pcsxi->WriteSakuraCJumpOffset32( regTemp, 0 ), dwLoopBegin ) ;
		}
		FreeExpressionRegister() ;
		FreeExpressionRegister() ;
		//
		CompileNakedPopExpressionTemporary() ;
	}
	m_pcsxi->ResetLocalBoundsAssignedRegisters
		( pNest->m_baseLocalFrame
			- pNest->m_nLocalSize, pNest->m_baseLocalFrame ) ;
	return	eslErrSuccess ;
}

int ECSCompiler::CountOfNakedFunctionDestruction( void ) const
{
	int	nCount = 0 ;
	for ( int i = 0; i < (int) m_nestCtrl.GetSize(); i ++ )
	{
		EControlNest *	pNest = m_nestCtrl.GetLastAt( i ) ;
		if ( pNest == NULL )
		{
			continue ;
		}
		nCount += CountOfNakedLocalDestruction( pNest ) ;
		if ( pNest->m_rwType == rwFunction )
		{
			break ;
		}
	}
	return	nCount ;
}

int ECSCompiler::CountOfNakedLocalDestruction( EControlNest * pNest ) const
{
	int	nCount = 0 ;
	for ( int i = 0; i < (int) pNest->m_lstLocalObj.GetSize(); i ++ )
	{
		ECSTypeInfo *	pTypeVar = pNest->m_lstLocalObj.GetLastAt( i ) ;
		if ( pTypeVar == NULL )
		{
			continue ;
		}
		if ( (pTypeVar->m_pValue == NULL)
			|| !pTypeVar->IsAddressingInfo() )
		{
			continue ;
		}
		if ( !ECSClassInfo::IsNeedsNakedVariableDestruction
									( pTypeVar->m_pValue, true ) )
		{
			continue ;
		}
		nCount += pTypeVar->CalcNakedMemoryArraySize() ;
	}
	return	nCount ;
}

// naked 関数ローカル変数のデストラクタ呼び出しコード生成
//（主に数式内で生成した一時変数のデストラクタを呼び出すため）
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileCodeNakedLocalDestructionSaveRegister
					( EControlNest * pNest, int regFirst, int regCount )
{
	if ( !m_modeNakedCode )
	{
		return	eslErrSuccess ;
	}
	int	nDestructions = CountOfNakedLocalDestruction( pNest ) ;
	if ( nDestructions == 0 )
	{
		return	eslErrSuccess ;
	}
	if ( regCount != 0 )
	{
		if ( regCount == 1 )
		{
			m_pcsxi->WriteSakuraPushReg( regFirst ) ;
		}
		else
		{
			m_pcsxi->WriteSakuraPushRegsImm8( regFirst, regCount ) ;
		}
	}
	ESLError	err = CompileCodeNakedLocalDestruction( pNest, 0 ) ;
	if ( regCount != 0 )
	{
		if ( regCount == 1 )
		{
			m_pcsxi->WriteSakuraPopReg( regFirst ) ;
		}
		else
		{
			m_pcsxi->WriteSakuraPopRegsImm8( regFirst, regCount ) ;
		}
	}
	return	err ;
}

// naked 関数のリーブ＆リターンコード生成
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileCodeNakedFunctionReturn
							( const ECSTypeInfo & typeReturn )
{
	//
	// ローカル変数破棄
	//
	ESLError	err = CompileCodeNakedFunctionDestruction() ;
	if ( err )
	{
		return	err ;
	}
	//
	// 返り値
	//
	bool	fThis = (GetCurrentThisClass() != NULL)
					&& (IsLocalVariableName(L"this") == 0) ;
	if ( !typeReturn.IsVoid() )
	{
		if ( !typeReturn.IsNakedPrimitiveDataType() )
		{
			ECSTypeInfo	typeRetObjPtr ;
			int	iRetObjPtr =
				IsLocalVariableName( L"<return>", &typeRetObjPtr ) ;
			ESLAssert( iRetObjPtr >= 0 ) ;
			if ( (iRetObjPtr >= 0) && typeRetObjPtr.IsAddressingInfo() )
			{
				int	regAcc = ECSSakura2Processor::regAcc ;
				WriteSakuraLoadMemory( regAcc, typeRetObjPtr, true ) ;
				if ( regAcc != ECSSakura2Processor::regAcc )
				{
					m_pcsxi->WriteSakuraMoveRegReg
						( ECSSakura2Processor::regAcc, regAcc ) ;
				}
			}
			else
			{
				return	ESLErrorMsg
					( "内部エラー：返り値オブジェクト"
							"ポインタを取得出来ませんでした" ) ;
			}
		}
		else
		{
			m_pcsxi->WriteSakuraMoveRegReg
				( ECSSakura2Processor::regAcc, GetExpressionRegister() ) ;
		}
		FreeExpressionRegister() ;
	}
	//
	// ローカルフレーム解放
	//
	m_pcsxi->ResetAllRegisterAssigns() ;
	//
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
	//
	// リターン命令
	//
	m_pcsxi->WriteSakuraReturn( ) ;
	//
	return	eslErrSuccess ;
}

// naked ローカル変数の初期値設定コード生成
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileCodeNakedLocalInitialDefault
							( const ECSTypeInfo & typeVar )
{
	if ( !m_modeNakedCode )
	{
		return	ESLErrorMsg
			( "内部エラー：object モードから"
				" naked メモリ変数を初期化しようとしています" ) ;
	}
	ESLAssert( typeVar.IsAddressingInfo() ) ;
	const ECSClassInfo *
				pObjectClassInf = typeVar.GetObjectClassInfo( *this ) ;
	const int	nArraySize = typeVar.CalcNakedMemoryArraySize() ;
	if ( nArraySize <= 0 )
	{
		return	eslErrSuccess ;
	}
	if ( pObjectClassInf != NULL )
	{
		//
		// object 変数生成
		//
		if ( nArraySize <= 1 )
		{
			m_pcsxi->WriteSakuraLoadInt64_ClassID
				( AllocateExpressionRegister(),
						pObjectClassInf->GetGlobalName() ) ;
			CompileNakedSystemCall( L"object_new", 1 ) ;
			//
			int	regSrc = ECSSakura2Processor::regAcc ;
			WriteSakuraStoreMemory( regSrc, typeVar ) ;
		}
		else
		{
			const int	regThis = AllocateExpressionRegister() ;
			const int	regCounter = AllocateExpressionRegister() ;
			ESLAssert( typeVar.m_regIndex < 0 ) ;
			m_pcsxi->WriteSakuraAddRegRegImm32
				( regThis, typeVar.m_regBase, typeVar.m_addrOffset ) ;
			m_pcsxi->WriteSakuraLoadInt64( regCounter, nArraySize ) ;
			m_pcsxi->FenceInstruction() ;
			//
			DWORD	addrLoopBegin = CompileCodeGetCurrent() ;
			//
			m_pcsxi->WriteSakuraLoadInt64_ClassID
				( AllocateExpressionRegister(),
						pObjectClassInf->GetGlobalName() ) ;
			CompileNakedSystemCall( L"object_new", 1 ) ;
			//
			int	regSrc = ECSSakura2Processor::regAcc ;
			WriteSakuraStoreMemory
				( ECSSakura2Processor::addrBase,
					ECSSakura2Processor::dataInt64, regSrc, regThis, 0 ) ;
			//
			const int	regCmp = AllocateExpressionRegister() ;
			m_pcsxi->WriteSakuraXorRegReg( regCmp, regCmp ) ;
			m_pcsxi->WriteSakuraAddRegRegImm32( regCounter, regCounter, -1 ) ;
			m_pcsxi->WriteSakuraAddRegRegImm32( regThis, regThis, 8 ) ;
			m_pcsxi->WriteSakuraCmpNeRegReg( regCmp, regCounter ) ;
			m_pcsxi->FenceInstruction() ;
			//
			CompileCodeConditionalJump( addrLoopBegin, true, false ) ;
			//
			FreeExpressionRegister() ;
			FreeExpressionRegister() ;
		}
	}
	else
	{
		//
		// naked クラス・変数
		//
		const ECSClassInfo *
				pNakedClassInf = typeVar.GetNakedMemoryClassInfo() ;
		if ( pNakedClassInf != NULL )
		{
			const int	regCounter = AllocateExpressionRegister() ;
			const int	regThis = AllocateExpressionRegister() ;
			ESLAssert( typeVar.m_regIndex < 0 ) ;
			m_pcsxi->WriteSakuraAddRegRegImm32
				( regThis, typeVar.m_regBase, typeVar.m_addrOffset ) ;
			//
			if ( nArraySize > 1 )
			{
				m_pcsxi->WriteSakuraLoadInt64( regCounter, nArraySize ) ;
				m_pcsxi->FenceInstruction() ;
			}
			DWORD	addrLoopBegin = CompileCodeGetCurrent() ;
			//
			CompileCodeNakedClassInitialize( pNakedClassInf ) ;
			//
			if ( IsDefaultConstructor( *pNakedClassInf ) )
			{
				m_pcsxi->WriteSakuraMoveRegReg
					( AllocateExpressionRegister(), regThis ) ;
				//
				CompileCallDefaultConstructor( *pNakedClassInf, false ) ;
			}
			//
			if ( nArraySize > 1 )
			{
				const int	regCmp = AllocateExpressionRegister() ;
				m_pcsxi->WriteSakuraXorRegReg( regCmp, regCmp ) ;
				m_pcsxi->WriteSakuraAddRegRegImm32( regCounter, regCounter, -1 ) ;
				m_pcsxi->WriteSakuraAddRegRegImm32
					( regThis, regThis, pNakedClassInf->GetNakedMemorySize() ) ;
				m_pcsxi->WriteSakuraCmpNeRegReg( regCmp, regCounter ) ;
				m_pcsxi->FenceInstruction() ;
				//
				CompileCodeConditionalJump( addrLoopBegin, true, false ) ;
			}
			FreeExpressionRegister() ;
			FreeExpressionRegister() ;
		}
		else
		{
			//
			// ゼロ初期化
			//
			int	nNakedSize = 0, nObjCount = 0 ;
			typeVar.GetNakedMemorySize( nNakedSize, nObjCount ) ;
			if ( nNakedSize > 0 )
			{
				if ( nArraySize == 1 )
				{
					int	regZero = ECSSakura2Processor::regIntZero ;
					WriteSakuraStoreMemory
						( regZero, typeVar, false, 0 ) ;
				}
				else if ( !IsCStyleCompatibleMode() )
				{
					if ( nNakedSize <= 8 * 8 )
					{
						for ( int i = 0; i < nNakedSize; i += 8 )
						{
							int	regZero = ECSSakura2Processor::regIntZero ;
							WriteSakuraStoreMemory
								( ECSSakura2Processor::addrBaseOffset32,
									ECSSakura2Processor::dataInt64,
									regZero,
									typeVar.m_regBase,
									typeVar.m_addrOffset + i,
									0, 0, true ) ;
						}
					}
					else
					{
						int	regExpr ;
						regExpr = AllocateExpressionRegister() ;
						m_pcsxi->WriteSakuraAddRegRegImm32
							( regExpr, typeVar.m_regBase, typeVar.m_addrOffset ) ;
						//
						regExpr = AllocateExpressionRegister() ;
						m_pcsxi->WriteSakuraMoveRegReg
								( regExpr, ECSSakura2Processor::regIntZero ) ;
						//
						regExpr = AllocateExpressionRegister() ;
						m_pcsxi->WriteSakuraLoadInt64( regExpr, nNakedSize ) ;
						//
						CompileNakedSystemCall( L"memset", 3 ) ;
					}
				}
			}
		}
	}
	return	eslErrSuccess ;
}

ESLError ECSCompiler::CompileCodeNakedLocalInitialConstValue
	( const ECSTypeInfo & typeVar, ECSObject * pObjInit )
{
	if ( pObjInit == NULL )
	{
		return	CompileCodeNakedLocalInitialDefault( typeVar ) ;
	}
	if ( !m_modeNakedCode )
	{
		return	ESLErrorMsg
			( "内部エラー：object モードから"
				" naked メモリ変数を初期化しようとしています" ) ;
	}
	ESLAssert( typeVar.IsAddressingInfo() ) ;
	const ECSClassInfo *
				pObjectClassInf = typeVar.GetObjectClassInfo( *this ) ;
	const int	nArraySize = typeVar.CalcNakedMemoryArraySize() ;
	if ( nArraySize <= 0 )
	{
		return	eslErrSuccess ;
	}
	ESLError	err ;
	if ( pObjectClassInf != NULL )
	{
		//
		// object 変数生成
		//
		err = CompileCodeNakedLocalInitialDefault( typeVar ) ;
		if ( err )
		{
			return	err ;
		}
		return	ESLErrorMsg
			( "naked モードで object に初期値を指定しています" ) ;
	}
	//
	// naked クラス
	//
	const ECSClassInfo *
			pNakedClassInf = typeVar.GetNakedMemoryClassInfo() ;
	if ( !typeVar.IsTypeArray() && (pNakedClassInf != NULL) )
	{
		if ( pNakedClassInf->IsNeedsNakedClassConstruction( false )
			|| (pNakedClassInf->GetVirtualFunctionCount() > 0) )
		{
			err = CompileCodeNakedLocalInitialDefault( typeVar ) ;
			if ( err )
			{
				return	err ;
			}
			return	ESLErrorMsg
				( "naked モードで naked クラスに初期値を指定しています" ) ;
		}
		if ( pObjInit->m_vtType != csvtArray )
		{
			return	ESLErrorMsg
				( "naked モードでクラスの初期値が不正です" ) ;
		}
		ECSArray *	pArrayInit = (ECSArray*) pObjInit ;
		const int	nMemberCount = pNakedClassInf->GetVariableCount() ;
		for ( int i = 0; i < nMemberCount; i ++ )
		{
			ECSTypeInfo *	pVarType = pNakedClassInf->GetVariableAt( i ) ;
			if ( pVarType == NULL )
			{
				return	ESLErrorMsg
					( "内部エラー：naked クラスの初期化で"
								"メンバの型情報が見つかりません" ) ;
			}
			ECSObject *	pInitElement = pArrayInit->m_varArray.GetAt( i ) ;
			if ( pInitElement == NULL )
			{
				switch ( pVarType->m_pValue->m_vtType )
				{
				case	csvtInteger:
				case	csvtReal:
				case	csvtArray:
					pInitElement = pVarType->m_pValue ;
				}
			}
			ECSTypeInfo	typeMember = *pVarType ;
			typeMember.MoveRegisterAndAddressingFrom( typeVar ) ;
			ESLAssert( typeMember.IsAddressingInfo() ) ;
			typeMember.m_addrOffset +=
					pNakedClassInf->GetVariableNakedOffsetAt( i ) ;
			//
			err = CompileCodeNakedLocalInitialConstValue
									( typeMember, pInitElement ) ;
			if ( err )
			{
				return	err ;
			}
		}
		return	eslErrSuccess ;
	}
	//
	// 整数・実数・ポインタ
	//
	ECSObject *	pTypePitch = typeVar.m_pValue ;
	if ( pTypePitch->m_vtType == csvtArray )
	{
		pTypePitch = ((ECSArray*)pTypePitch)->GetEndDefaultElement() ;
	}
	if ( pTypePitch == NULL )
	{
		return	ESLErrorMsg( "void 変数を初期化しようとしています" ) ;
	}
	int		nElementPitch = 0 ;
	int		nObjCount = 0 ;
	ECSTypeInfo::GetNakedMemorySize
			( nElementPitch, nObjCount, pTypePitch ) ;
	//
	ECSTypeInfo	typePitch
		( ECSTypeInfo::DuplicateType( pTypePitch ), typeVar.m_dwFlags ) ;
	bool	fPrimitiveType = typePitch.IsNakedPrimitiveDataType() ;
	typePitch.MoveRegisterAndAddressingFrom( typeVar ) ;
	//
	for ( int i = 0; i < nArraySize; i ++ )
	{
		ECSObject *	pValue = pObjInit ;
		if ( pValue->m_vtType == csvtArray )
		{
			pValue = ((ECSArray*)pValue)->m_varArray.GetAt( i ) ;
		}
		if ( fPrimitiveType )
		{
			if ( pValue != NULL )
			{
				ECSTypeInfo	typeTemp ;
				ESLError	err =
					CompileImmediateObject( typeTemp, pValue ) ;
				if ( err )
				{
					return	err ;
				}
				MakeCommitValueToNakedRegister( typeTemp ) ;
				//
				int	regSrc = GetExpressionRegister() ;
				WriteSakuraStoreMemory
						( regSrc, typePitch, true, i * nElementPitch ) ;
				//
				FreeExpressionRegister( typeTemp ) ;
			}
			else
			{
				WriteSakuraStoreMemory
					( ECSSakura2Processor::regIntZero,
							typePitch, true, i * nElementPitch ) ;
			}
		}
		else
		{
			ECSTypeInfo	typeElement = typePitch ;
			typeElement.m_addrOffset += i * nElementPitch ;
			//
			err = CompileCodeNakedLocalInitialConstValue
										( typeElement, pValue ) ;
			if ( err )
			{
				return	err ;
			}
		}
	}
	return	eslErrSuccess ;
}

// ユーザー定義シンボルの有効性確認
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::VerifyUserSymbol( const wchar_t * pwszSymbol ) const
{
	if ( pwszSymbol == NULL )
	{
		return	ESLErrorMsg( "内部エラー：無名シンボルです。" ) ;
	}
	if ( (pwszSymbol[0] >= L'0') && (pwszSymbol[0] <= L'9') )
	{
		return	ESLErrorMsg
			( "数値シンボルがユーザー定義名として使用されています。" ) ;
	}
	int	i ;
	for ( i = 0; pwszSymbol[i]; i ++ )
	{
		wchar_t	wch = pwszSymbol[i] ;
		if ( !(((wch >= L'A') && (wch <= L'Z'))
			|| ((wch >= L'a') && (wch <= L'z'))
			|| ((wch >= L'0') && (wch <= L'9'))
			|| (wch == L'@') || (wch == L'_')
			|| (wch == L':') || ((WORD) wch >= 0x80)) )
		{
			return	ESLErrorMsg
				( "ユーザー定義名に使用できない文字が使用されています。" ) ;
		}
	}
	ECSWideString	wstrSymbol = pwszSymbol ;
	ReservedWord	rwIndex = IsReservedWord( wstrSymbol ) ;
	if ( rwIndex != rwInvalid )
	{
		return	ESLErrorMsg
			( "予約語がユーザー定義名に指定されています。" ) ;
	}
	MacroWord	mwIndex = IsMacroWord( wstrSymbol ) ;
	if ( mwIndex != mwInvalid )
	{
		return	ESLErrorMsg
			( "予約マクロ名がユーザー定義名に指定されています。" ) ;
	}
	CSObjectMode	csomType = IsMemoryClass( wstrSymbol ) ;
	if ( csomType != csomImmediate )
	{
		return	ESLErrorMsg
			( "予約語がユーザー定義名に指定されています。" ) ;
	}
/*	int	iType = IsTypeName( wstrSymbol ) ;
	if ( iType >= 0 )
	{
		return	ESLErrorMsg
			( "型名が重複してユーザー定義名として指定されました。" ) ;
	}
*/	static const wchar_t *	pwszOperator[] =
	{
		L"mod", L"and", L"land", L"or", L"lor", L"xor",
		L"not", L"lnot",
		L"shift_right", L"shift_left",
		L"boolean", L"sizeof", L"typeof",
		L"parent", L"static_cast", L"dynamic_cast",
		L"operator",
		L"virtual", L"native", L"naked",
		L"double", L"float",
		NULL
	} ;
	for ( i = 0; pwszOperator[i]; i ++ )
	{
		if ( wstrSymbol.Compare( pwszOperator[i] ) == 0 )
		{
			return	ESLErrorMsg
				( "演算子がユーザー定義名に使用されています。" ) ;
		}
	}
	static const wchar_t *	pwszExReserved[] =
	{
		L"Int64", L"Int32", L"Uint32", L"Int16", L"Uint16",
		L"Int8", L"Uint8", L"Boolean", L"void",
		L"Naked", L"Native",
		NULL
	} ;
	for ( i = 0; pwszExReserved[i]; i ++ )
	{
		if ( CompareReservedWord( pwszExReserved[i], wstrSymbol ) == 0 )
		{
			return	ESLErrorMsg
				( "予約語がユーザー定義名に指定されています。" ) ;
		}
	}
	return	eslErrSuccess ;
}

// 制御ネストに変数を追加する
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::AddVariableToControlNest
	( EControlNest * pNest,
		const wchar_t * pwszVarName, ECSTypeInfo * pVarType )
{
	pNest->m_lstLocalObj.Add( pVarType ) ;
	pNest->m_lstLocalName.Add( pwszVarName ) ;
	//
	if ( m_modeNakedCode )
	{
		//
		// naked メモリアドレス確定
		//
		UpdateNestLocalFrameBase() ;
		//
		int	nNakedSize = 0, nObjCount ;
		if ( pVarType->GetNakedMemorySize( nNakedSize, nObjCount ) )
		{
			nNakedSize = 8 ;
		}
		//
		pNest->m_nLocalSize += (nNakedSize + 0x07) & ~0x07 ;
		//
		pVarType->SetAddressingInfo
			( ECSSakura2Processor::regBP,
				pNest->m_baseLocalFrame - pNest->m_nLocalSize ) ;
		UpdateNestLocalFrameBase() ;
	}
}

// 制御ネストを１つ離脱する
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::LeaveControlNest( EControlNest * pTopNest )
{
	EObjArray<EWideString>	lstThrows ;
	ReservedWord	rwLastNest = rwInvalid ;
	if ( pTopNest == NULL )
	{
		pTopNest = m_nestCtrl.Pop( ) ;
	}
	if ( pTopNest != NULL )
	{
		rwLastNest = pTopNest->m_rwType ;
		lstThrows.Merge( 0, pTopNest->m_lstThrows ) ;
		m_dwImplementFlags = pTopNest->m_dwLastImplFlags ;
		delete	pTopNest ;
	}
	EControlNest *	pNest = m_nestCtrl.GetLastAt( 0 ) ;
	if ( pNest != NULL )
	{
		m_rwCtrlType = pNest->m_rwType ;
		//
//		if ( rwLastNest != rwTry )
		{
			for ( int i = 0; i < (int) lstThrows.GetSize(); i ++ )
			{
				if ( pNest->m_lstThrows.Find( lstThrows[i] ) < 0 )
				{
					pNest->m_lstThrows.Add( new EWideString(lstThrows[i]) ) ;
				}
			}
		}
	}
	else
	{
		m_rwCtrlType = rwInvalid ;
	}
	if ( !IsInFunctionNest() )
	{
		m_pcsxi = &m_csxiInitFunc ;
	}
	m_modeNakedCode = IsInNakedCodeFunction() ;
}

// naked ローカル変数割り当てアドレス更新
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::UpdateNestLocalFrameBase( void )
{
	EControlNest *	pFuncNest = NULL ;
	int		sizeLocalFrame = 0 ;
	for ( int i = 0; i < (int) m_nestCtrl.GetSize(); i ++ )
	{
		EControlNest *	pNest = m_nestCtrl.GetAt( i ) ;
		if ( pNest == NULL )
		{
			continue ;
		}
		if ( pNest->m_rwType == rwFunction )
		{
			pFuncNest = pNest ;
			sizeLocalFrame = pNest->m_nLocalSize ;
		}
		else
		{
			pNest->m_baseLocalFrame = - sizeLocalFrame ;
			sizeLocalFrame += pNest->m_nLocalSize ;
		}
		if ( (pFuncNest != NULL)
			&& (pFuncNest->m_maxLocalSize < (DWORD) sizeLocalFrame) )
		{
			pFuncNest->m_maxLocalSize = sizeLocalFrame ;
		}
	}
}

// メンバ関数の修飾を補正
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CompileEffectPrototype
	( ECSPrototypeInfo & prototype, EControlNest * pClassNest, bool fFuncBlock )
{
	ECSClassInfo *	pClassInf =
				GetClassInfoAs( pClassNest->m_wstrCurSpaceName ) ;
	if ( pClassInf == NULL )
	{
		return	ESLErrorMsg( "内部エラー：クラス情報が見つかりません。" ) ;
	}
	if ( pClassInf->IsNamespace() )
	{
		prototype.SetAttribute
			( prototype.GetAttribute() | ECSTypeInfo::flagStatic ) ;
	}
	if ( !(prototype.GetAttribute() & ECSTypeInfo::flagStatic) )
	{
		prototype.SetAttribute
			( prototype.GetAttribute() | ECSTypeInfo::flagThisCall ) ;
	}
	if ( pClassInf->GetName() == prototype.GetName() )
	{
		if ( !prototype.GetReturnType().IsVoid() )
		{
			return	ESLErrorMsg( "構築関数に返り値が定義されています。" ) ;
		}
		if ( pClassInf->IsNamespace() )
		{
			return	ESLErrorMsg( "名前空間の構築関数は定義できません" ) ;
		}
		prototype.SetAttribute
			( prototype.GetAttribute() & ~ECSTypeInfo::flagVirtual ) ;
	}
	else if ( L"~" + pClassInf->GetName() == prototype.GetName() )
	{
		if ( !prototype.GetReturnType().IsVoid() )
		{
			return	ESLErrorMsg( "消滅関数に返り値が定義されています。" ) ;
		}
		if ( prototype.GetArgumentCount() != 0 )
		{
			return	ESLErrorMsg( "消滅関数に引数が定義されています。" ) ;
		}
		if ( pClassInf->IsNamespace() )
		{
			return	ESLErrorMsg( "名前空間の消滅関数は定義できません" ) ;
		}
		if ( !pClassInf->IsNakedMemoryClass() )
		{
			prototype.SetAttribute
				( prototype.GetAttribute() & ~ECSTypeInfo::flagVirtual ) ;
		}
	}
	else
	{
		if ( prototype.GetName().CompareLeft( L"operator " ) )
		{
			ESLError	err = VerifyUserSymbol( prototype.GetName() ) ;
			if ( err )
			{
				return	err ;
			}
			if ( pClassInf->GetAttribute()
							& ECSTypeInfo::flagNativeObject )
			{
				prototype.SetAttribute
					( prototype.GetAttribute()
								| ECSTypeInfo::flagNativeObject ) ;
			}
		}
	}
	prototype.SetGlobalName
		( pClassInf->GetGlobalName() + L"::" + prototype.GetName() ) ;
	//
	if ( (prototype.GetAttribute() & ECSTypeInfo::flagVarArgument)
		&& !(prototype.GetAttribute() & ECSTypeInfo::flagNakedCall)
		&& !(pClassInf->GetAttribute() & ECSTypeInfo::flagNativeObject) )
	{
		return	ESLErrorMsg( "可変長引数が定義されています。" ) ;
	}
	if ( (prototype.GetAttribute() & ECSTypeInfo::flagNativeObject)
		&& !(pClassInf->GetAttribute() & ECSTypeInfo::flagNativeObject)
		&& !pClassInf->IsNakedMemoryClass()
		&& !pClassInf->IsNamespace() && !pClassInf->IsUnion() )
	{
		return	ESLErrorMsg( "メンバ関数の Native 指定は無効です。" ) ;
	}
	if ( !fFuncBlock && (pClassInf->GetThisFunctionAs( prototype ) != NULL) )
	{
		return	ESLErrorMsg
			( "既に同名・同一引数の関数が定義されています。" ) ;
	}
	prototype.SetAttribute
		( prototype.GetAttribute() | pClassNest->m_dwProtectedScope ) ;
	//
	return	CompileEffectPrototype( prototype, pClassInf ) ;
}

ESLError ECSCompiler::CompileEffectPrototype
	( ECSPrototypeInfo & prototype, ECSClassInfo * pClassInf )
{
	if ( pClassInf->IsNamespace() )
	{
		prototype.SetAttribute
			( (prototype.GetAttribute()
				| ECSTypeInfo::flagStatic)
					& ~ECSTypeInfo::flagThisCall ) ;
	}
	if ( pClassInf->IsNakedMemoryClass()
		&& (m_dwModeFlags & flagDefualtNakedFunc) )
	{
		prototype.SetAttribute
			( prototype.GetAttribute() | ECSTypeInfo::flagNakedCall ) ;
	}
	if ( prototype.GetAttribute() & ECSTypeInfo::flagObjectedCall )
	{
		prototype.SetAttribute
			( prototype.GetAttribute() & ~ECSTypeInfo::flagNakedCall ) ;
	}
	if ( prototype.IsNakedCall() )
	{
		if ( pClassInf->GetAttribute() & ECSTypeInfo::flagNativeObject )
		{
			prototype.SetAttribute
				( prototype.GetAttribute() & ~ECSTypeInfo::flagVirtual ) ;
		}
		prototype.MakeNakedAttribute() ;
	}
	return	eslErrSuccess ;
}

// 関数の引数定義リストを出力
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::EncodeArgumentList
	( EControlNest * pNest,
		const EObjArray<ECSTypeInfo> & lstArgType,
		const EObjArray<EWideString> & lstArgName )
{
	DWORD	dwArgCount = lstArgType.GetSize( ) ;
	DWORD	i ;
	for ( i = 0; i < dwArgCount; i ++ )
	{
		ECSTypeInfo *	pArgType = lstArgType.GetAt( i ) ;
		EWideString *	pwstrName = lstArgName.GetAt( i ) ;
		ESLAssert( pArgType != NULL ) ;
		ESLAssert( pwstrName != NULL ) ;
		//
		ECSObject *	pValue = pArgType->m_pValue ;
		ESLAssert( pValue != NULL ) ;
		if ( pValue->m_vtType == csvtObject )
		{
			int	nClassIndex = GetClassInfoIndex( pValue->GetTypeName() ) ;
			if ( nClassIndex >= 0 )
			{
				m_pcsxi->WriteVariableTypeCode( csvtClassObject ) ;
				m_pcsxi->WriteClassIndex( nClassIndex ) ;
			}
			else
			{
				m_pcsxi->WriteVariableTypeCode( csvtObject ) ;
				m_pcsxi->WriteConstantString
						( ECSWideString( pValue->GetTypeName() ) ) ;
			}
		}
		else if ( pValue->m_vtType == csvtInteger )
		{
			m_pcsxi->WriteVariableTypeCode
					( ((ECSInteger*)pValue)->GetIntegerType() ) ;
		}
		else if ( pArgType->IsRuntimeIntegerType() )
		{
			m_pcsxi->WriteVariableTypeCode( csvtInteger ) ;
		}
		else
		{
			m_pcsxi->WriteVariableTypeCode( pValue->m_vtType ) ;
		}
		m_pcsxi->WriteConstantString( *pwstrName ) ;
		//
		pNest->m_lstLocalObj.Add( new ECSTypeInfo( *pArgType ) ) ;
		pNest->m_lstLocalName.Add( *pwstrName ) ;
	}
	for ( i = 0; i < dwArgCount; i ++ )
	{
		ECSTypeInfo *	pArgType = lstArgType.GetAt( i ) ;
		ESLAssert( pArgType != NULL ) ;
		//
		ECSObject *	pValue = pArgType->m_pValue ;
		ESLAssert( pValue != NULL ) ;
		if ( pValue->m_vtType == csvtArray )
		{
			ECSArray *	pArray = (ECSArray*) pValue ;
			ECSObject *	pElement = pArray->GetEndDefaultElement() ;
			if ( pElement != NULL )
			{
				CompileCodeLoadRefVariable( csomStack, i ) ;
				//
				CompileArrayDimension( pArray ) ;
				//
				CompileCodeFreeStack() ;
			}
		}
		else if ( pValue->m_vtType == csvtArray )
		{
			ECSHash *	pHash = (ECSHash*) pValue ;
			ECSObject *	pElement = pHash->m_pDefObj ;
			if ( pElement != NULL )
			{
				CompileCodeLoadRefVariable( csomStack, i ) ;
				//
				CompileHashContainer( pHash ) ;
				//
				CompileCodeFreeStack() ;
			}
		}
	}
}

// 制御ブロックの脱出アドレスを確定
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::CommitBreakAddressOnNest
	( EControlNest * pNest, DWORD dwBreakAddr )
{
	for ( int i = 0; i < (int) pNest->m_lstBreak.GetSize(); i ++ )
	{
		DWORD	dwAddr = pNest->m_lstBreak.GetAt( i ) ;
		DWORD *	pdwJumpAddr =
			(DWORD*) m_pcsxi->m_bufImage.
				ModifyBuffer( dwAddr, sizeof(DWORD) ) ;
		ESLAssert( pdwJumpAddr != NULL ) ;
		if ( pdwJumpAddr != NULL )
		{
			*pdwJumpAddr = dwBreakAddr - (dwAddr + sizeof(DWORD)) ;
		}
	}
}

// 特定の制御ブロックを取得
//////////////////////////////////////////////////////////////////////////////
ECSCompiler::EControlNest * ECSCompiler::GetMostInnerNest
	( ECSCompiler::ReservedWord rwFirst,
		ECSCompiler::ReservedWord rwLast, int * pNestCount ) const
{
	EControlNest *	pNest = NULL ;
	for ( int i = 0; i < (int) m_nestCtrl.GetSize(); i ++ )
	{
		pNest = m_nestCtrl.GetLastAt( i ) ;
		ESLAssert( pNest != NULL ) ;
		if ( pNest == NULL )
		{
			continue ;
		}
		if ( pNest->m_rwType == rwTemplate )
		{
			break ;
		}
		if ( (pNest->m_rwType >= rwFirst) && (pNest->m_rwType <= rwLast) )
		{
			if ( pNestCount != NULL )
				*pNestCount = i ;
			break ;
		}
	}
	if ( (pNest == NULL) ||
		(pNest->m_rwType < rwFirst) || (pNest->m_rwType > rwLast) )
	{
		return	NULL ;
	}
	return	pNest ;
}

// throw されうる例外リストを現在のネストに追加する
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::AddThrowListToCurrentNest
	( const EObjArray<EWideString>& lstThrows )
{
	EControlNest *	pNest = m_nestCtrl.GetLastAt( 0 ) ;
	if ( pNest == NULL )
	{
		return ;
	}
	const int	nCount = lstThrows.GetSize() ;
	for ( int i = 0; i < nCount; i ++ )
	{
		if ( pNest->m_lstThrows.Find( lstThrows[i] ) < 0 )
		{
			pNest->m_lstThrows.Add( new EWideString(lstThrows[i]) ) ;
		}
	}
}

// naked モードで構造化例外処理に対応するブロックか判定する
//////////////////////////////////////////////////////////////////////////////
bool ECSCompiler::IsNakedThrowableNest( int iNest ) const
{
	if ( m_flagThrowable )
	{
		return	true ;
	}
	int	i, nCount ;
	nCount = m_nestCtrl.GetSize() ;
	for ( i = iNest; i < nCount; i ++ )
	{
		EControlNest *	pNest = m_nestCtrl.GetLastAt( i ) ;
		if ( pNest != NULL )
		{
			if ( pNest->m_rwType == rwTry )
			{
				return	true ;
			}
			else if ( (pNest->m_rwType == rwFunction)
					&& (pNest->m_pFuncPrototype != NULL) )
			{
				if ( pNest->m_pFuncPrototype->GetThrows().GetSize() > 0 )
				{
					return	true ;
				}
			}
		}
	}
	return	false ;
}

// 名前空間検索リストに含まれるか判定
//////////////////////////////////////////////////////////////////////////////
bool ECSCompiler::IsUsingNamespaceNest
		( const wchar_t * pwszNamespace, int iNest ) const
{
	for ( int i = 0; i < iNest; i ++ )
	{
		EControlNest *	pNest = m_nestCtrl.GetLastAt( i ) ;
		const EObjArray<EWideString> *	pList = &m_lstUsingNamespace ;
		if ( pNest != NULL )
		{
			pList = &(pNest->m_lstUsingNamespace) ;
		}
		unsigned int	nCount = pList->GetSize() ;
		for ( unsigned int j = 0; j < nCount; j ++ )
		{
			EWideString *	pwstrNamespace = pList->GetAt( j ) ;
			if ( pwstrNamespace != NULL )
			{
				if ( *pwstrNamespace == pwszNamespace )
				{
					return	true ;
				}
			}
		}
	}
	return	false ;
}

// 検索名前空間に必要なリストを追加する
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::AddNecessaryUsingNamespace( EControlNest * pNest )
{
	if ( pNest != NULL )
	{
		ECSClassInfo *	pClassInf = NULL ;
		if ( !pNest->m_wstrCurSpaceName.IsEmpty() )
		{
			pClassInf = m_pcsxiDst->GetClassInfoAs( pNest->m_wstrCurSpaceName ) ;
		}
		if ( !pNest->m_wstrThisSpaceName.IsEmpty() )
		{
			if ( pClassInf == NULL )
			{
				pClassInf = m_pcsxiDst->GetClassInfoAs
								( pNest->m_wstrThisSpaceName ) ;
			}
		}
		if ( pClassInf != NULL )
		{
			AddNecessaryUsingNamespacePath
					( pNest, pClassInf->GetGlobalName() ) ;
			AddNecessaryUsingParentClassList( pNest, pClassInf ) ;
		}
	}
}

void ECSCompiler::AddNecessaryUsingNamespacePath
	( EControlNest * pNest, const wchar_t * pwszName ) const
{
	SYMBOL_NAMESPACE	snsSymbol ;
	snsSymbol.ParseSymbol( pwszName ) ;
	while ( !snsSymbol.wstrNamespace.IsEmpty() )
	{
		bool	fAddNamespace = true ;
		int	j, m ;
		m = (int) pNest->m_lstUsingNamespace.GetSize() ;
		for ( j = 0; j < m; j ++ )
		{
			EWideString *	pwstrNamespace =
					pNest->m_lstUsingNamespace.GetAt( j ) ;
			if ( (pwstrNamespace != NULL)
				&& (*pwstrNamespace == snsSymbol.wstrNamespace) )
			{
				fAddNamespace = false ;
				break ;
			}
		}
		if ( fAddNamespace )
		{
			EWideString *	pwstrNamespace =
					new EWideString(snsSymbol.wstrNamespace) ;
			pNest->m_lstUsingNamespace.Add( pwstrNamespace ) ;
		}
		snsSymbol.ParseSymbol( EWideString(snsSymbol.wstrNamespace) ) ;
	}
}

void ECSCompiler::AddNecessaryUsingParentClassList
	( EControlNest * pNest, ECSClassInfo * pClassInf ) const
{
	if ( pClassInf != NULL )
	{
		unsigned int	i, nParentCount = pClassInf->GetParentClassCount() ;
		for ( i = 0; i < nParentCount; i ++ )
		{
			ECSClassInfo::ParentClass *
				pParent = pClassInf->GetParentClassAt( i ) ;
			if ( (pParent != NULL)
				&& (pParent->pClassInf != NULL)
				&& (pParent->dwFlags <= ECSTypeInfo::flagProtected)
				&& (pClassInf->GetGlobalName()
						!= pParent->pClassInf->GetGlobalName()) )
			{
				pNest->m_lstUsingNamespace.Add
					( new EWideString( pParent->pClassInf->GetGlobalName() ) ) ;
				//
				AddNecessaryUsingNamespacePath
						( pNest, pParent->pClassInf->GetGlobalName() ) ;
				AddNecessaryUsingParentClassList
						( pNest, pParent->pClassInf ) ;
			}
		}
	}
}

// 名前空間検索リストを取得
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::GetUsingNamespaceList
	( EPtrObjArray<const wchar_t> & lstNamespace ) const
{
	int	i, nCount ;
	nCount = m_nestCtrl.GetSize() ;
	lstNamespace.SetLimit( 0x10 ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		EControlNest *	pNest = m_nestCtrl.GetLastAt( i ) ;
		if ( pNest != NULL )
		{
			ECSClassInfo *	pClassInf = NULL ;
			if ( !pNest->m_wstrCurSpaceName.IsEmpty() )
			{
				lstNamespace.Add( pNest->m_wstrCurSpaceName ) ;
				pClassInf = m_pcsxiDst->GetClassInfoAs( pNest->m_wstrCurSpaceName ) ;
			}
			if ( !pNest->m_wstrThisSpaceName.IsEmpty() )
			{
				lstNamespace.Add( pNest->m_wstrThisSpaceName ) ;
				if ( pClassInf == NULL )
				{
					pClassInf = m_pcsxiDst->GetClassInfoAs
									( pNest->m_wstrThisSpaceName ) ;
				}
			}
			int	j, m ;
			m = (int) pNest->m_lstUsingNamespace.GetSize() ;
			for ( j = 0; j < m; j ++ )
			{
				EWideString *	pwstrNamespace =
						pNest->m_lstUsingNamespace.GetAt( j ) ;
				if ( pwstrNamespace != NULL )
				{
					lstNamespace.Add( *pwstrNamespace ) ;
				}
			}
			if ( pClassInf != NULL )
			{
				/*
				SYMBOL_NAMESPACE	snsSymbol ;
				snsSymbol.ParseSymbol( pClassInf->GetGlobalName() ) ;
				while ( !snsSymbol.wstrNamespace.IsEmpty() )
				{
					bool	fAddNamespace = true ;
					for ( j = 0; j < m; j ++ )
					{
						EWideString *	pwstrNamespace =
								pNest->m_lstUsingNamespace.GetAt( j ) ;
						if ( (pwstrNamespace != NULL)
							&& (*pwstrNamespace == snsSymbol.wstrNamespace) )
						{
							fAddNamespace = false ;
							break ;
						}
					}
					if ( fAddNamespace )
					{
						EWideString *	pwstrNamespace =
								new EWideString(snsSymbol.wstrNamespace) ;
						pNest->m_lstUsingNamespace.Add( pwstrNamespace ) ;
						lstNamespace.Add( *pwstrNamespace ) ;
					}
					snsSymbol.ParseSymbol( EWideString(snsSymbol.wstrNamespace) ) ;
				}
				AddUsingParentClassList( lstNamespace, pClassInf ) ;
				*/
			}
			if ( pNest->m_rwType == rwTemplate )
			{
				break ;
			}
		}
	}
	nCount = m_lstUsingNamespace.GetSize() ;
	for ( i = 0; i < nCount; i ++ )
	{
		EWideString *	pwstrNamespace =
				m_lstUsingNamespace.GetAt( i ) ;
		if ( pwstrNamespace != NULL )
		{
			lstNamespace.Add( *pwstrNamespace ) ;
		}
	}
}

void ECSCompiler::AddUsingParentClassList
	( EPtrObjArray<const wchar_t> & lstNamespace, ECSClassInfo * pClassInf ) const
{
	if ( pClassInf != NULL )
	{
		unsigned int	i, nParentCount = pClassInf->GetParentClassCount() ;
		for ( i = 0; i < nParentCount; i ++ )
		{
			ECSClassInfo::ParentClass *
				pParent = pClassInf->GetParentClassAt( i ) ;
			if ( (pParent != NULL)
				&& (pParent->pClassInf != NULL)
				&& (pParent->dwFlags <= ECSTypeInfo::flagProtected)
				&& (pClassInf->GetGlobalName()
						!= pParent->pClassInf->GetGlobalName()) )
			{
				lstNamespace.Add( pParent->pClassInf->GetGlobalName() ) ;
				//
				AddUsingParentClassList( lstNamespace, pParent->pClassInf ) ;
			}
		}
	}
}

// 現在の名前空間名を取得
//////////////////////////////////////////////////////////////////////////////
EWideString ECSCompiler::GetCurrentSpaceName( void ) const
{
	for ( int i = 0; i < (int) m_nestCtrl.GetSize(); i ++ )
	{
		EControlNest *	pNest = m_nestCtrl.GetLastAt( i ) ;
		if ( pNest != NULL )
		{
			if ( pNest->m_rwType == rwTemplate )
			{
				break ;
			}
			if ( !pNest->m_wstrCurSpaceName.IsEmpty() )
			{
				return	pNest->m_wstrCurSpaceName ;
			}
		}
	}
	return	EWideString() ;
}

// 現在の this 型名を取得
//////////////////////////////////////////////////////////////////////////////
EWideString ECSCompiler::GetCurrentThisClassName( void ) const
{
	for ( int i = 0; i < (int) m_nestCtrl.GetSize(); i ++ )
	{
		EControlNest *	pNest = m_nestCtrl.GetLastAt( i ) ;
		if ( pNest != NULL )
		{
			if ( pNest->m_rwType == rwTemplate )
			{
				break ;
			}
			if ( !pNest->m_wstrThisSpaceName.IsEmpty() )
			{
				return	pNest->m_wstrThisSpaceName ;
			}
		}
	}
	return	EWideString() ;
}

// 現在の this クラス取得
//////////////////////////////////////////////////////////////////////////////
ECSClassInfo * ECSCompiler::GetCurrentThisClass( void ) const
{
	EWideString	wstrThisClass = GetCurrentThisClassName() ;
	if ( wstrThisClass.IsEmpty() )
	{
		return	NULL ;
	}
	return	GetClassInfoAs( wstrThisClass ) ;
}

// 現在のスコープから指定クラスのアクセススコープ取得
//////////////////////////////////////////////////////////////////////////////
DWORD ECSCompiler::GetAccessClassTo( const ECSClassInfo * pClassInf, DWORD dwFlags ) const
{
	dwFlags = (dwFlags & ~ECSTypeInfo::flagProtectedMask)	
									| ECSTypeInfo::flagPublic ;
	if ( pClassInf != NULL )
	{
		ECSClassInfo *	pThisClass = GetCurrentThisClass() ;
		if ( pThisClass != NULL )
		{
			if ( pThisClass->GetGlobalName() == pClassInf->GetGlobalName() )
			{
				return	dwFlags | ECSTypeInfo::flagPrivate2 ;
			}
			if ( pClassInf->IsFriendClass( pThisClass->GetGlobalName() ) )
			{
				return	dwFlags | ECSTypeInfo::flagPrivate ;
			}
			if ( pThisClass->GetCastClassInfoAs
							( pClassInf->GetGlobalName() ) != NULL )
			{
				return	dwFlags | ECSTypeInfo::flagProtected ;
			}
		}
	}
	return	dwFlags ;
}

DWORD ECSCompiler::GetAccessClassTo( const wchar_t * pwszClassName, DWORD dwFlags ) const
{
	if ( pwszClassName != NULL )
	{
		return	GetAccessClassTo( GetClassInfoAs(pwszClassName), dwFlags ) ;
	}
	return	ECSTypeInfo::flagPublic ;
}

// 現在の this が naked クラスか？
//////////////////////////////////////////////////////////////////////////////
bool ECSCompiler::IsNakedCurrentThisClass( void ) const
{
	ECSClassInfo *	pThisClass = GetCurrentThisClass() ;
	if ( pThisClass == NULL )
	{
		return	false ;
	}
	return	pThisClass->IsNakedMemoryClass() ;
}

// C 互換性モードか？
//////////////////////////////////////////////////////////////////////////////
bool ECSCompiler::IsCStyleCompatibleMode( void ) const
{
	const DWORD	modeCCompatible =
		flagCStyleCast | flagCStyleBasicType
			| flagCompatibleInt32 | flagDefaultNakedAll ;
	return	((m_dwModeFlags & modeCCompatible) == modeCCompatible) ;
}

// naked 関数内か？
//////////////////////////////////////////////////////////////////////////////
bool ECSCompiler::IsInNakedCodeFunction( void ) const
{
	EControlNest *
		pFuncNest = GetMostInnerNest( rwFunction, rwEndFunc ) ;
	if ( pFuncNest != NULL )
	{
		return	pFuncNest->m_flagNakedMode ;
	}
	EControlNest *
		pTempNest = GetMostInnerNest( rwTemplate, rwTemplate ) ;
	if ( pTempNest != NULL )
	{
		return	pTempNest->m_flagNakedMode ;
	}
	return	((m_dwModeFlags & flagDefaultNakedAll) != 0) ;
}

// テンプレートの宣言文中か？
//////////////////////////////////////////////////////////////////////////////
bool ECSCompiler::IsInTemplateDeclaration( void ) const
{
	EControlNest *
		pTempNest = GetMostInnerNest( rwTemplate, rwTemplate ) ;
	if ( pTempNest != NULL )
	{
		return	!pTempNest->m_fCommitBlock ;
	}
	return	false ;
}

// 外部参照マクロ変数設定
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::AttachExternalMacroVariable( ECSObject * pVar )
{
	m_pRefMacroVariable = pVar ;
}

// 変数オブジェクトの条件判定
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::EvaluateVariable( ECSObject * pObj )
{
	if ( pObj == NULL )
	{
		return	eslErrInvalidParam ;
	}
	int	nBoolean ;
	if ( pObj->OperateBoolean( nBoolean ) )
	{
		return	eslErrInvalidParam ;
	}
	return	nBoolean ? eslErrSuccess : eslErrContinue ;
}

// 定数値（マクロ変数）取得
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSCompiler::GetMacroVariable( const wchar_t * pwszName )
{
	//
	// ローカルマクロ変数を検索
	//
	int			i, nCount ;
	ECSObject *	pObj = NULL ;
	nCount = m_nestMacro.GetSize( ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		EMacroNest *	pNest = m_nestMacro.GetLastAt( i ) ;
		if ( pNest == NULL )
		{
			continue ;
		}
		pObj = pNest->m_staLocal.GetAs( pwszName ) ;
		if ( pObj != NULL )
		{
			return	pObj ;
		}
	}
	nCount = m_nestCtrl.GetSize( ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		EControlNest *	pNest = m_nestCtrl.GetLastAt( i ) ;
		if ( pNest == NULL )
		{
			break ;
		}
		pObj = pNest->m_staConstant.GetAs( pwszName ) ;
		if ( pObj != NULL )
		{
			return	pObj ;
		}
		if ( pNest->m_rwType == rwTemplate )
		{
			break ;
		}
	}
	//
	// ユーザー定義定数から検索
	//
	EPtrObjArray<const wchar_t>	lstNamespace ;
	EWideString	wstrTempName ;
	GetUsingNamespaceList( lstNamespace ) ;
	nCount = lstNamespace.GetSize() ;
	for ( i = 0; i < nCount; i ++ )
	{
		const wchar_t *	pwszNamespace = lstNamespace.GetAt( i ) ;
		if ( pwszNamespace != NULL )
		{
			wstrTempName = pwszNamespace ;
			wstrTempName += L"::" ;
			wstrTempName += pwszName ;
			pObj = m_staConstant.GetAs( wstrTempName ) ;
			if ( pObj != NULL )
			{
				return	pObj ;
			}
		}
	}
	pObj = m_staConstant.GetAs( pwszName ) ;
	if ( pObj != NULL )
	{
		return	pObj ;
	}
	//
	// システム定義変数名判定
	//
	ECSWideString	wstrName = pwszName ;
	if ( !wstrName.CompareNoCase( L"@macromode" ) )
	{
		return	&m_nMacroMode ;
	}
	if ( !wstrName.CompareNoCase( L"@linenum" ) )
	{
		m_csintLineNum.SetValue( m_nLineNum ) ;
		return	&m_csintLineNum ;
	}
	if ( !wstrName.CompareNoCase( L"@filename" ) )
	{
		return	&m_csstrFileName ;
	}
	//
	// マクロ参照
	//
	if ( m_pRefMacroVariable != NULL )
	{
		int	nIndex ;
		ESLError	err =
			m_pRefMacroVariable->GetVariableIndex( nIndex, wstrName ) ;
		if ( !err )
		{
			return	m_pRefMacroVariable->GetVariableAt( nIndex ) ;
		}
	}
	return	NULL ;
}

// 大域定数値（マクロ変数）設定
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::SetMacroVariable
	( const wchar_t * pwszName, ECSObject * pObj )
{
	m_staConstant.SetAs( pwszName, pObj ) ;
}

// 定数式を実行する時に利用する実行イメージで初期化する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::InitConstExprContext( ECSExecutionImage * pcsxi )
{
	return	m_ctxExpr.InitializeContext( pcsxi, false ) ;
}

// 定数式を実行する時に利用する実行コンテキストのリソース解放
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::ReleaseConstExprContext( void )
{
	m_ctxExpr.ReleaseContext( false ) ;
	m_ctxExpr.m_pcsxi = nullptr ;
}

// 定数式評価
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::CalculateExpression
	( ECSObject *& pValue,
		ECSSourceStream & cssLine,
		int nPriority, const wchar_t * pwszExit,
		bool fImmidiateValue, bool fDefaultZeroSymbol )
{
	//
	// 第一項処理
	//////////////////////////////////////////////////////////////////////////
	pValue = NULL ;
	if ( cssLine.DisregardSpace() )
	{
		return	ESLErrorMsg( "定数式の解析中に行末に到達しました。" ) ;
	}
	ESLError	err ;
	OPERATOR_INFO	opinf ;
//	ECSContext	context ;		// ダミー
	do
	{
		//
		// 即値判定
		//////////////////////////////////////////////////////////////////////
		bool	fRealNumber ;
		int		nRadix = GetNumberLiteralRadix( cssLine, fRealNumber ) ;
		if ( nRadix >= 0 )
		{
			//
			// 整数・実数値
			//
			if ( !fRealNumber )
			{
				INT64	nValue ;
				err = GetIntegerLiteral( nValue, cssLine, nRadix ) ;
				if ( err )
				{
					return	err ;
				}
				pValue = new ECSInteger( nValue ) ;
			}
			else
			{
				REAL64	rValue ;
				err = GetRealLiteral( rValue, cssLine, nRadix ) ;
				if ( err )
				{
					return	err ;
				}
				pValue = new ECSReal( rValue ) ;
			}
			break ;
		}
		wchar_t	wch ;
		wch = cssLine.CurrentCharacter( ) ;
		if ( ((wch == L'L') || (wch == L'l'))
			&& (cssLine.GetIndex() + 1 < cssLine.GetLength()) )
		{
			wchar_t	wchNext = cssLine.GetAt( cssLine.GetIndex() + 1 ) ;
			if ( (wchNext == L'\"') || (wchNext == L'\'') )
			{
				cssLine.GetCharacter() ;
				wch = wchNext ;
			}
		}
		if ( (wch == L'\"') || (wch == L'\'') )
		{
			//
			// 文字列
			//
			EWideString	wstrStr ;
			err = GetStringLiteral( wstrStr, cssLine, wch ) ;
			if ( err )
			{
				return	err ;
			}
			if ( (wch == L'\'')
				&& (m_dwModeFlags & flagQuoteCharactorCode) )
			{
				INT64	nCode = 0 ;
				for ( unsigned int i = 0; i < 4; i ++ )
				{
					if ( i >= wstrStr.GetLength() )
					{
						break ;
					}
					nCode = (nCode << 16) | (wstrStr.GetAt(i) & 0xFFFF) ;
				}
				pValue = new ECSInteger( nCode, ECSInteger::m_maskUint16 ) ;
			}
			else
			{
				pValue = new ECSString( wstrStr ) ;
			}
			break ;
		}
		else if ( wch == L'(' )
		{
			//
			// 括弧記号
			//
			cssLine.GetCharacter( ) ;
			if ( m_dwModeFlags & flagCStyleCast )
			{
				int	nIndex = cssLine.GetIndex() ;
				ECSTypeInfo	typeCast ;
				err = ParseTypeDescription( typeCast, cssLine ) ;
				if ( !err && (cssLine.HasToComeChar( L")" ) == L')') )
				{
					//
					// C 言語互換キャスト記述
					//
					ECSTypeInfo	typeExpr ;
					err = CalculateExpression
						( pValue, cssLine, oppCast,
							pwszExit, true, fDefaultZeroSymbol ) ;
					if ( err )
					{
						return	err ;
					}
					ECSObject *	pCast = NULL ;
					err = CompileImmidiateCast( pCast, typeCast, pValue ) ;
					if ( err )
					{
						delete	pValue ;
						return	err ;
					}
					delete	pValue ;
					pValue = pCast ;
					break ;
				}
				cssLine.MoveIndex( nIndex ) ;
			}
			err = CalculateExpression
				( pValue, cssLine,
					0, L")", fImmidiateValue, fDefaultZeroSymbol ) ;
			if ( err )
			{
				return	err ;
			}
			if ( cssLine.HasToComeChar( L")" ) != L')' )
			{
				return	ESLErrorMsg
					( "\'(\' に対応する \')\' が見つかりません。" ) ;
			}
			break ;
		}
		else if ( wch == L'{' )
		{
			//
			// 配列定数
			//
			ECSArray *	pArray = new ECSArray ;
			cssLine.GetCharacter( ) ;
			//
			wch = cssLine.HasToComeChar( L"}" ) ;
			while ( wch != L'}' )
			{
				err = CalculateExpression
					( pValue, cssLine, 0, L",}", true, fDefaultZeroSymbol ) ;
				if ( err )
				{
					delete	pArray ;
					return	err ;
				}
				pArray->m_varArray.Add( pValue ) ;
				pValue = NULL ;
				//
				wch = cssLine.HasToComeChar( L",}" ) ;
				if ( wch == L'\0' )
				{
					return	ESLErrorMsg
						( "\'{\' に対応する \'}\' が見つかりません。" ) ;
				}
				if ( wch == L',' )
				{
					wch = cssLine.HasToComeChar( L"}" ) ;
				}
			}
			NormalizeArrayTypeInfo( *pArray ) ;
			//
			pValue = pArray ;
			break ;
		}
		//
		// トークン取得（名前空間記述）
		//////////////////////////////////////////////////////////////////////
		ECSWideString	wstrToken ;
		wstrToken = cssLine.GetAToken( ) ;
		//
		EWideString	wstrNameSpace ;
		EWideString	wstrGlobalName = wstrToken ;
		while ( cssLine.HasToComeToken( L"::" ) )
		{
			wstrNameSpace = wstrGlobalName ;
			wstrToken = cssLine.GetAToken() ;
			wstrGlobalName += L"::" ;
			wstrGlobalName += wstrToken ;
		}
		//
		// 定数値判定
		//////////////////////////////////////////////////////////////////////
		wstrToken = wstrGlobalName ;
		//
		ECSObject *	pObj = GetMacroVariable( wstrToken ) ;
		if ( pObj != NULL )
		{
			pValue = new ECSReference( pObj ) ;
			break ;
		}
		//
		// マクロ関数判定
		//////////////////////////////////////////////////////////////////////
		EMacroBlock *	pmbMacro = m_staMacro.GetAs( wstrToken ) ;
		if ( (pmbMacro != NULL) && pmbMacro->m_fMacroFunc )
		{
			EObjArray<EWideString>	lstParam ;
			err = CompileMacroArgument( lstParam, cssLine ) ;
			if ( err )
			{
				return	err ;
			}
			err = CompileUserMacro( pmbMacro, lstParam, &pObj ) ;
			if ( err )
			{
				return	err ;
			}
			if ( pObj != NULL )
			{
				pValue = pObj ;
				break ;
			}
			else
			{
				return	ESLErrorMsg( "マクロ関数に返り値がありません。" ) ;
			}
		}
		if ( (wstrToken == L"@")
			|| !CompareReservedWord( L"@Compile", wstrToken )
			|| !CompareReservedWord( L"@IsDefined", wstrToken ) )
		{
			wch = cssLine.HasToComeChar( L"(" ) ;
			if ( wch != L'(' )
			{
				return	ESLErrorMsg
					( "@compile マクロ関数に引数がありません。" ) ;
			}
			ECSObject *	pMacro = NULL ;
			err = CalculateExpression
					( pMacro, cssLine,
						0, L")", fImmidiateValue, fDefaultZeroSymbol ) ;
			if ( err )
			{
				return	err ;
			}
			if ( cssLine.HasToComeChar( L")" ) != L')' )
			{
				m_strErrMsg = wstrToken ;
				m_strErrMsg +=
					" マクロ関数の引数が \')\' 括弧で閉じられていません。" ;
				return	ESLErrorMsg( m_strErrMsg ) ;
			}
			if ( pMacro->m_vtType != csvtString )
			{
				m_strErrMsg = wstrToken ;
				m_strErrMsg += " マクロ関数の引数が文字列型でありません。" ;
				return	ESLErrorMsg( m_strErrMsg ) ;
			}
			if ( (wstrToken == L"@")
				|| !wstrToken.CompareNoCase( L"@compile" ) )
			{
				ECSSourceStream	cssExpr = ((ECSString*)pMacro)->m_varStr ;
				delete	pMacro ;
				if ( cssExpr.DisregardSpace() )
				{
					pValue = new ECSInteger( ) ;
				}
				else
				{
					err = CalculateExpression
						( pValue, cssExpr, 0, NULL, true, fDefaultZeroSymbol ) ;
					if ( err )
					{
						return	err ;
					}
				}
			}
			else
			{
				int	nValue = 0 ;
				wstrToken = ((ECSString*)pMacro)->m_varStr ;
				delete	pMacro ;
				//
				if ( m_staMacro.GetAs( wstrToken ) != NULL )
				{
					nValue = 7 ;
				}
				else if ( (pObj = GetMacroVariable( wstrToken )) != NULL )
				{
					if ( m_staConstant.GetAs( wstrToken ) == pObj )
					{
						nValue = 2 ;
					}
					else
					{
						nValue = 1 ;
					}
				}
				else if ( IsTypeName( wstrToken ) >= 0 )
				{
					nValue = 3 ;
				}
				else if ( IsLocalVariableName( wstrToken ) >= 0 )
				{
					nValue = 4 ;
				}
				else if ( m_pcsxiDst->m_csgGlobalType.
							m_staObjName.FindIndex( wstrToken ) >= 0 )
				{
					nValue = 5 ;
				}
				else if ( m_pcsxiDst->m_csgDataType.
							m_staObjName.FindIndex( wstrToken ) >= 0 )
				{
					nValue = 6 ;
				}
				else if ( m_staExternName.FindIndex( wstrToken ) >= 0 )
				{
					nValue = 5 ;
				}
				pValue = new ECSInteger( nValue ) ;
			}
			break ;
		}
		else if ( wstrToken == L"null" )
		{
			pValue = new ECSReference ;
			break ;
		}
		else if ( wstrToken == L"true" )
		{
			pValue = new ECSInteger( -1, ECSInteger::m_maskBoolean ) ;
			break ;
		}
		else if ( wstrToken == L"false" )
		{
			pValue = new ECSInteger( 0, ECSInteger::m_maskBoolean ) ;
			break ;
		}
		//
		// 単項演算子判定
		//
		if ( !GetOperatorInfo( opinf, wstrToken, true ) )
		{
			if ( (opinf.opiType == optExtraUniary)
				&& (opinf.xuoptExUnary == csxuotSizeOf) )
			{
				//
				// sizeof(type) 判定
				//
				bool	fSizeOf = false ;
				int		nNakedSize = 0 ;
				err = ParseNakedSizeOfTypeOperator
						( cssLine, fSizeOf, nNakedSize ) ;
				if ( fSizeOf )
				{
					if ( err )
					{
						return	err ;
					}
					delete	pValue ;
					pValue = new ECSInteger( nNakedSize ) ;
					break ;
				}
			}
			err = CalculateExpression
				( pValue, cssLine,
					GetUnaryOperatorPriority(opinf),
					pwszExit, true, fDefaultZeroSymbol ) ;
			if ( err )
			{
				return	err ;
			}
			if ( opinf.opiType == optUnary )
			{
				err = pValue->UnaryOperate( m_ctxExpr, opinf.uoptUnary ) ;
				if ( pValue->m_pResult != NULL )
				{
					ECSObject *	pObj ;
					pObj = pValue->m_pResult ;
					pValue->m_pResult = NULL ;
					delete	pValue ;
					pValue = pObj ;
				}
			}
			else if ( opinf.opiType == optExtraUniary )
			{
				switch ( opinf.xuoptExUnary )
				{
				case	csxuotBoolean:
					{
						int	nBoolean ;
						err = pValue->OperateBoolean( nBoolean ) ;
						delete	pValue ;
						pValue = new ECSInteger
								( nBoolean, ECSInteger::m_maskBoolean ) ;
					}
					break ;
				case	csxuotSizeOf:
					{
						INT64	nSize ;
						err = pValue->OperateSizeOf( nSize ) ;
						delete	pValue ;
						pValue = new ECSInteger( nSize ) ;
					}
					break ;
				case	csxuotTypeOf:
					{
						ECSString *	pTypeOf = new ECSString ;
						pTypeOf->m_varStr = pValue->OperateTypeOf( ) ;
						delete	pValue ;
						pValue = pTypeOf ;
					}
					break ;
				case	csxuotDeselect:
				default:
					err = ESLErrorMsg( "定数式で使用できない単項演算子です。" ) ;
					break ;
				}
			}
			else
			{
				err = ESLErrorMsg( "定数式で使用できない単項演算子です。" ) ;
			}
			if ( err )
			{
				delete	pValue ;
				pValue = NULL ;
				return	err ;
			}
			break ;
		}
		//
		// 静的キャスト判定
		//
		if ( wstrToken == L"static_cast" )
		{
			if ( cssLine.HasToComeChar( L"<" ) != L'<' )
			{
				m_strErrMsg =
					EString( wstrToken )
						+ " に \'<\' 記号が見つかりません。" ;
				return	ESLErrorMsg( m_strErrMsg ) ;
			}
			ECSTypeInfo	typeCast ;
			err = ParseTypeDescription( typeCast, cssLine ) ;
			if ( err )
			{
				return	err ;
			}
			if ( cssLine.HasToComeChar( L">" ) != L'>' )
			{
				m_strErrMsg =
					EString( wstrToken )
						+ " に閉じ \'>\' 記号が見つかりません。" ;
				return	ESLErrorMsg( m_strErrMsg ) ;
			}
			if ( cssLine.HasToComeChar( L"(" ) != L'(' )
			{
				m_strErrMsg =
					EString( wstrToken )
						+ " に \'(\' 記号が見つかりません。" ;
				return	ESLErrorMsg( m_strErrMsg ) ;
			}
			pValue = NULL ;
			err = CalculateExpression
				( pValue, cssLine, 0, L")", true, fDefaultZeroSymbol ) ;
			if ( err )
			{
				return	err ;
			}
			if ( pValue == NULL )
			{
				return	ESLErrorMsg
					( "void なデータをキャストしようとしています" ) ;
			}
			if ( cssLine.HasToComeChar( L")" ) != L')' )
			{
				delete	pValue ;
				pValue = NULL ;
				return	ESLErrorMsg
					( "\'(\' に対応する \')\' が見つかりません。" ) ;
			}
			ECSObject *	pNakedCastType = typeCast.GetNakedType() ;
			if ( pNakedCastType == NULL )
			{
				delete	pValue ;
				pValue = NULL ;
				return	ESLErrorMsg
					( "void な型へキャストしようとしています" ) ;
			}
			switch ( pNakedCastType->m_vtType )
			{
				case	csvtInteger:
				case	csvtReal:
				case	csvtString:
				case	csvtArray:
				case	csvtHash:
					break ;
				default:
					delete	pValue ;
					pValue = NULL ;
					return	ESLErrorMsg
						( "定数式で使用できない型へ"
							"型変換しようとしています。" ) ;
			}
			pNakedCastType = ECSTypeInfo::DuplicateType( pNakedCastType ) ;
			err = pNakedCastType->Move( m_ctxExpr, pValue ) ;
			if ( err )
			{
				delete	pValue ;
				pValue = NULL ;
				return	err ;
			}
			pValue = pNakedCastType ;
			break ;
		}
		//
		// オブジェクト構築か？
		//
		int	csvtType = IsTypeName( wstrToken ) ;
		if ( csvtType >= 0 )
		{
			if ( csvtType < csvtMax )
			{
				switch ( csvtType )
				{
				case	csvtInteger:
					pValue = new ECSInteger ;
					break ;
				case	csvtReal:
					pValue = new ECSReal ;
					break ;
				case	csvtString:
					pValue = new ECSString ;
					break ;
				case	csvtArray:
					pValue = new ECSArray ;
					break ;
				case	csvtHash:
					pValue = new ECSHash ;
					break ;
				default:
					return	ESLErrorMsg
						( "定数式で使用できない型の"
							"オブジェクトを構築しようとしています。" ) ;
				}
			}
			else
			{
				if ( wstrToken == L"File" )
				{
					pValue = new ECSFile ;
				}
				else
				{
					return	ESLErrorMsg
						( "定数式で使用できない型の"
							"オブジェクトを構築しようとしています。" ) ;
				}
			}
			if ( cssLine.HasToComeChar( L"(" ) == L'(' )
			{
				ECSObject *	pObj ;
				ESLError	err =
					CalculateExpression
						( pObj, cssLine, 0, L")", true, fDefaultZeroSymbol ) ;
				if ( err )
				{
					return	err ;
				}
				if ( cssLine.HasToComeChar( L")" ) != L')' )
				{
					return	ESLErrorMsg
						( "'(' に対応する ')' が見つかりませんでした。" ) ;
				}
				err = pValue->Move( m_ctxExpr, pObj ) ;
				if ( err )
				{
					delete	pObj ;
					delete	pValue ;
					pValue = NULL ;
					return	err ;
				}
			}
			break ;
		}
		if ( fDefaultZeroSymbol )
		{
			pValue = new ECSInteger( 0 ) ;
			break ;
		}
		m_strErrMsg =
			"定数式の中で使用できないシンボル \'"
				+ EString(wstrToken) + "\' があります。" ;
		return	ESLErrorMsg( m_strErrMsg ) ;
	}
	while ( false ) ;
	//
	// 第二項処理
	//////////////////////////////////////////////////////////////////////////
	while ( !cssLine.DisregardSpace() )
	{
		//
		// 終了判定
		//
		int		iOpIndex = cssLine.GetIndex( ) ;
		if ( pwszExit != NULL )
		{
			if ( cssLine.HasToComeChar( pwszExit ) != L'\0' )
			{
				cssLine.MoveIndex( iOpIndex ) ;
				break ;
			}
		}
		//
		// 演算子取得
		//
		ECSWideString	wstrToken = cssLine.GetAToken( ) ;
		if ( GetOperatorInfo( opinf, wstrToken, false ) )
		{
			delete	pValue ;
			return	ESLErrorMsg( "演算子が指定されていません。" ) ;
		}
		int		nCurrentPriority = GetOperatorPriority( opinf ) ;
		if ( nCurrentPriority <= nPriority )
		{
			cssLine.MoveIndex( iOpIndex ) ;
			break ;
		}
		//
		if ( opinf.opiType == optGeneral )
		{
			//
			// 二項演算子
			//
			ECSObject *	pObj ;
			err = CalculateExpression
				( pObj, cssLine, nCurrentPriority,
					pwszExit, fImmidiateValue, fDefaultZeroSymbol ) ;
			if ( err )
			{
				delete	pValue ;
				pValue = NULL ;
				return	err ;
			}
			if ( pValue->m_vtType == csvtReference )
			{
				ECSObject *	pTemp = pValue ;
				pValue = pTemp->Duplicate() ;
				delete	pTemp ;
			}
			err = pValue->Operate( m_ctxExpr, opinf.optOperator, pObj ) ;
			if ( err )
			{
				delete	pObj ;
				delete	pValue ;
				pValue = NULL ;
				return	err ;
			}
			if ( pValue->m_pResult != NULL )
			{
				pObj = pValue->m_pResult ;
				pValue->m_pResult = NULL ;
				delete	pValue ;
				pValue = pObj ;
			}
		}
		else if ( opinf.opiType == optCompare )
		{
			//
			// 比較演算
			//
			ECSObject *	pObj ;
			err = CalculateExpression
				( pObj, cssLine, nCurrentPriority,
					pwszExit, fImmidiateValue, fDefaultZeroSymbol ) ;
			if ( err )
			{
				delete	pValue ;
				return	err ;
			}
			int	nResult ;
			err = pValue->Compare
				( m_ctxExpr, nResult, opinf.cptCompare, *pObj ) ;
			delete	pObj ;
			delete	pValue ;
			if ( err )
			{
				pValue = NULL ;
				return	err ;
			}
			pValue = new ECSInteger( nResult, ECSInteger::m_maskBoolean ) ;
		}
		else if ( !fImmidiateValue && (opinf.opiType == optMove) )
		{
			//
			// 代入
			//
			ECSObject *	pObj ;
			err = CalculateExpression
				( pObj, cssLine, nCurrentPriority,
					pwszExit, fImmidiateValue, fDefaultZeroSymbol ) ;
			if ( err )
			{
				return	err ;
			}
			if ( opinf.optOperator == csotNop )
			{
				err = pValue->Move( m_ctxExpr, pObj ) ;
			}
			else
			{
				err = pValue->Operate( m_ctxExpr, opinf.optOperator, pObj ) ;
			}
			if ( err )
			{
				delete	pValue ;
				delete	pObj ;
				pValue = NULL ;
				return	err ;
			}
		}
		else if ( opinf.opiType == optUnary )
		{
			//
			// 後置単項演算子
			//
			switch ( opinf.uoptUnary )
			{
			case	csuotIncrement:
				opinf.uoptUnary = csuotIncrementAfter ;
				break ;
			case	csuotDecrement:
				opinf.uoptUnary = csuotDecrementAfter ;
				break ;
			default:
				delete	pValue ;
				pValue = NULL ;
				return	ESLErrorMsg( "定数式で不正な後置単項演算子です。" ) ;
			}
			err = pValue->UnaryOperate( m_ctxExpr, opinf.uoptUnary ) ;
			if ( err )
			{
				delete	pValue ;
				pValue = NULL ;
				return	err ;
			}
			if ( pValue->m_pResult != NULL )
			{
				ECSObject *	pObj ;
				pObj = pValue->m_pResult ;
				pValue->m_pResult = NULL ;
				delete	pValue ;
				pValue = pObj ;
			}
		}
		else if ( opinf.opiType == optMember )
		{
			//
			// メンバ関数呼び出し
			//
			int		nTokenType ;
			wstrToken = cssLine.GetAToken( &nTokenType ) ;
			if ( cssLine.HasToComeChar( L"(" ) != L'(' )
			{
				int	nIndex ;
				err = pValue->GetVariableIndex( nIndex, wstrToken ) ;
				if ( err )
				{
					delete	pValue ;
					pValue = NULL ;
					return	err ;
				}
				ECSObject *	pObjTemp ;
				ECSObject *	pObj = pValue->GetVariableAt( nIndex ) ;
				if ( pObj == NULL )
				{
					delete	pValue ;
					pValue = NULL ;
					return	ESLErrorMsg( "未定義メンバの指定です。" ) ;
				}
				else if ( pValue->m_vtType == csvtReference )
				{
					pObjTemp = pValue ;
					pValue = new ECSReference( pObj ) ;
					delete	pObjTemp ;
				}
				else
				{
					pObjTemp = pObj->Duplicate() ;
					delete	pValue ;
					pValue = pObjTemp ;
				}
				continue ;
			}
			//
			// 引数を取得
			//
			ECSObjArray<ECSObject>	lstArg ;
			wchar_t	wchNext ;
			lstArg.Add( pValue ) ;
			//
			if ( (wchNext = cssLine.HasToComeChar( L")" )) != L')' )
			while ( !cssLine.DisregardSpace() )
			{
				ECSObject *	pArgObj ;
				err = CalculateExpression
					( pArgObj, cssLine, 0, L",)",
						fImmidiateValue, fDefaultZeroSymbol ) ;
				if ( err )
				{
					return	err ;
				}
				lstArg.Add( pArgObj ) ;
				wchNext = cssLine.HasToComeChar( L",)" ) ;
				if ( wchNext != L',' )
				{
					break ;
				}
			}
			if ( wchNext != L')' )
			{
				delete	pValue ;
				pValue = NULL ;
				return	ESLErrorMsg
					( "メンバ関数の引数が ')' で閉じられていません。" ) ;
			}
			//
			// メンバ関数呼び出し
			//
			int	iFuncIndex ;
			err = pValue->GetFunction( m_ctxExpr, iFuncIndex, wstrToken ) ;
			if ( err )
			{
				delete	pValue ;
				pValue = NULL ;
				return	err ;
			}
			m_ctxExpr.m_ip = (DWORD) -1 ;
			//
			err = pValue->CallFunction( m_ctxExpr, iFuncIndex, lstArg ) ;
			if ( err )
			{
				pValue = NULL ;
				return	err ;
			}
			if ( (m_ctxExpr.m_pcsxi != nullptr)
				&& (m_ctxExpr.m_ip != (DWORD) -1) )
			{
				m_ctxExpr.ResumeExecution() ;
			}
			pValue = m_ctxExpr.PopObject( ) ;
			if ( pValue == NULL )
			{
				return	ESLErrorMsg( "メンバ関数が値を返しませんでした。" ) ;
			}
		}
		else if ( opinf.opiType == optReference )
		{
			ECSObject *	pObj ;
			ECSObject *	pObjTemp ;
			err = CalculateExpression
				( pObj, cssLine, 0, L"]", true, fDefaultZeroSymbol ) ;
			if ( err )
			{
				delete	pValue ;
				pValue = NULL ;
				return	err ;
			}
			cssLine.HasToComeChar( L"]" ) ;
			//
			int	nIndex ;
			if ( pObj->m_vtType == csvtInteger )
			{
				err = pValue->GetVariableIndex
					( nIndex, ((ECSInteger*)pObj)->GetInt() ) ;
			}
			else if ( pObj->m_vtType == csvtString )
			{
				err = pValue->GetVariableIndex
					( nIndex, ((ECSString*)pObj)->m_varStr ) ;
			}
			else
			{
				err = ESLErrorMsg
					( "整数でも文字列でもないオブジェクトが指標に指定されています。" ) ;
			}
			delete	pObj ;
			if ( err )
			{
				delete	pValue ;
				pValue = NULL ;
				return	err ;
			}
			pObj = pValue->GetVariableAt( nIndex ) ;
			if ( pObj == NULL )
			{
				ECSObject *	pEntity = ECSObject::GetEntity( pValue ) ;
				if ( (pEntity == NULL) || (pEntity == pValue) )
				{
					delete	pValue ;
					pValue = NULL ;
					return	ESLErrorMsg( "配列の指標が範囲を超えています。" ) ;
				}
				pObjTemp = pValue ;
				pValue = new ECSReference ;
				((ECSReference*)pValue)->m_pRefParent = pEntity ;
				((ECSReference*)pValue)->m_iParentRef = nIndex ;
				delete	pObjTemp ;
			}
			else if ( pValue->m_vtType == csvtReference )
			{
				pObjTemp = pValue ;
				pValue = new ECSReference( pObj ) ;
				delete	pObjTemp ;
			}
			else
			{
				pObjTemp = pObj->Duplicate() ;
				delete	pValue ;
				pValue = pObjTemp ;
			}
		}
		else
		{
			delete	pValue ;
			pValue = NULL ;
			return	ESLErrorMsg( "定数式で使用できない不正な演算子です。" ) ;
		}
	}
	if ( fImmidiateValue && (pValue->m_vtType == csvtReference) )
	{
		ECSObject *	pTemp = pValue ;
		pValue = pValue->Duplicate() ;
		delete	pTemp ;
	}
	return	eslErrSuccess ;
}

// 配列即値の型情報を正規化
//////////////////////////////////////////////////////////////////////////////
void ECSCompiler::NormalizeArrayTypeInfo( ECSArray & arrayType )
{
	bool		fElementType = false ;
	ECSObject *	pElementType = NULL ;
	//
	unsigned int	i, nCount ;
	nCount = arrayType.m_varArray.GetSize() ;
	for ( i = 0; i < nCount; i ++ )
	{
		ECSObject *	pElement = arrayType.m_varArray.GetAt( i ) ;
		if ( pElement == NULL )
		{
			continue ;
		}
		if ( pElement->m_vtType == csvtObject )
		{
			break ;
		}
		if ( fElementType )
		{
			if ( !ECSTypeInfo::IsTypeEqual( pElementType, pElement ) )
			{
				fElementType = false ;
				break ;
			}
		}
		else
		{
			fElementType = true ;
			pElementType = ECSTypeInfo::DuplicateType( pElement ) ;
		}
	}
	if ( fElementType )
	{
		arrayType.SetDefaultElement( pElementType ) ;
	}
	else
	{
		delete	pElementType ;
	}
}

// 演算子の情報を取得する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCompiler::GetOperatorInfo
	( ECSCompiler::OPERATOR_INFO & opinf,
		const wchar_t * pwszOperator, bool fUnary )
{
	static const wchar_t *	pwszOperators[] =
	{
		L"+", L"-", L"*", L"/", L"mod", L"%",
		L"and", L"&", L"or", L"|", L"xor", L"^",
		L"land", L"&&", L"lor", L"||",
		L"shift_right", L">>", L"shift_left", L"<<",
		NULL
	} ;
	static const CSOperatorType	csotOperators[] =
	{
		csotAdd, csotSub, csotMul, csotDiv, csotMod, csotMod,
		csotAnd, csotAnd, csotOr, csotOr, csotXor, csotXor,
		csotLogicalAnd, csotLogicalAnd,
		csoutLogicalOr, csoutLogicalOr,
		csotShiftRight, csotShiftRight,
		csotShiftLeft, csotShiftLeft,
	} ;
	static const wchar_t *	pwszMoves[] =
	{
		L"+=", L"-=", L"*=", L"/=", L"%=",
		L"&=", L"|=", L"^=", L">>=", L"<<=", NULL
	} ;
	static const CSOperatorType	csotMoveOperators[] =
	{
		csotAdd, csotSub, csotMul, csotDiv, csotMod,
		csotAnd, csotOr, csotXor, csotShiftRight, csotShiftLeft,
	} ;
	static const wchar_t *	pwszUnaryOperators[] =
	{
		L"+", L"-", L"not", L"~", L"lnot", L"!", L"++", L"--", NULL
	} ;
	static const CSUnaryOperatorType	csuotUnaryOperators[] =
	{
		csuotPlus, csuotNegate, csuotBitNot, csuotBitNot,
		csuotLogicalNot, csuotLogicalNot,
		csuotIncrement, csuotDecrement,
	} ;
	static const wchar_t *	pwszExUnaryOperators[] =
	{
		L"deselect", L"boolean", L"sizeof", L"typeof",
		L"&", L"*", NULL
	} ;
	static const CSExtraUniOperatorType	csxuotExUnaryOperators[] =
	{
		csxuotDeselect,	csxuotBoolean,	csxuotSizeOf,	csxuotTypeOf,
		csxuotLoadAddress, csxuotRefAddress,
	} ;
	static const wchar_t *	pwszComparators[csctMax] =
	{
		L"!=", L"==", L"<", L"<=", L">", L">=", L"!==", L"===",
	} ;
	//
	int				i ;
	ECSWideString	wstrOperator = pwszOperator ;
	wstrOperator.MakeLower( ) ;
	if ( !fUnary )
	{
		//
		// 二項演算子判定
		//
		for ( i = 0; pwszOperators[i]; i ++ )
		{
			if ( wstrOperator == pwszOperators[i] )
			{
				opinf.opiType = optGeneral ;
				opinf.optOperator = csotOperators[i] ;
				opinf.uoptUnary = csuotMax ;
				if ( opinf.optOperator <= csotSub )
				{
					opinf.uoptUnary = (CSUnaryOperatorType) i ;
				}
				return	eslErrSuccess ;
			}
		}
		//
		// 代入演算子判定
		//
		if ( (wstrOperator == L":=") || (wstrOperator == L"=") )
		{
			if ( (wstrOperator == L"=")
				&& !(m_dwModeFlags & flagNoWarningEquMove) )
			{
				ESLError	err = OutputWarning1
					( "代入演算子に = が使用されています。"
						":= を用いてください。（推奨）",
							m_strFilePath, m_nLineNum ) ;
				if ( err )
				{
					return	err ;
				}
			}
			opinf.opiType = optMove ;
			opinf.optOperator = csotNop ;
			return	eslErrSuccess ;
		}
		if ( wstrOperator == L"::=" )
		{
			opinf.opiType = optExtraOperator ;
			opinf.xoptExOperator = csxotMoveReference ;
			return	eslErrSuccess ;
		}
		for ( i = 0; pwszMoves[i] != NULL; i ++ )
		{
			if ( wstrOperator == pwszMoves[i] )
			{
				opinf.opiType = optMove ;
				opinf.optOperator = csotMoveOperators[i] ;
				return	eslErrSuccess ;
			}
		}
		//
		// 比較演算子判定
		//
		for ( i = 0; i < csctMax; i ++ )
		{
			if ( wstrOperator == pwszComparators[i] )
			{
				opinf.opiType = optCompare ;
				opinf.cptCompare = (CSCompareType) i ;
				return	eslErrSuccess ;
			}
		}
		//
		// 比較選択演算子
		//
		if ( wstrOperator == L"?" )
		{
			opinf.opiType = optCompareSelector ;
			opinf.cstSelector = cstEvaluation ;
			return	eslErrSuccess ;
		}
		else if ( wstrOperator == L":" )
		{
			opinf.opiType = optCompareSelector ;
			opinf.cstSelector = cstSeparator ;
			return	eslErrSuccess ;
		}
		//
		// 式列挙演算子
		//
		if ( wstrOperator == L"," )
		{
			opinf.opiType = optListExpression ;
			return	eslErrSuccess ;
		}
	}
	//
	// 単項演算子判定
	//
	for ( i = 0; pwszUnaryOperators[i]; i ++ )
	{
		if ( wstrOperator == pwszUnaryOperators[i] )
		{
			opinf.opiType = optUnary ;
			opinf.uoptUnary = csuotUnaryOperators[i] ;
			return	eslErrSuccess ;
		}
	}
	//
	// 特殊単項演算子判定
	//
	for ( i = 0; pwszExUnaryOperators[i]; i ++ )
	{
		if ( wstrOperator == pwszExUnaryOperators[i] )
		{
			opinf.opiType = optExtraUniary ;
			opinf.xuoptExUnary = csxuotExUnaryOperators[i] ;
			return	eslErrSuccess ;
		}
	}
	//
	// メンバ指定演算子判定
	//
	if ( wstrOperator == L"." )
	{
		opinf.opiType = optMember ;
		return	eslErrSuccess ;
	}
	if ( wstrOperator == L"->" )
	{
		opinf.opiType = optPtrMember ;
		return	eslErrSuccess ;
	}
	if ( wstrOperator == L".*" )
	{
		opinf.opiType = optMemberFunc ;
		return	eslErrSuccess ;
	}
	if ( wstrOperator == L"->*" )
	{
		opinf.opiType = optPtrMemberFunc ;
		return	eslErrSuccess ;
	}
	//
	// 要素参照演算子判定
	//
	if ( wstrOperator == L"[" )
	{
		opinf.opiType = optReference ;
		return	eslErrSuccess ;
	}
	//
	// 関数呼び出し判定
	//
	if ( wstrOperator == L"(" )
	{
		opinf.opiType = optArgument ;
		return	eslErrSuccess ;
	}
	//
	// new 判定
	//
	if ( wstrOperator == L"new" )
	{
		opinf.opiType = optNew ;
		return	eslErrSuccess ;
	}
	return	ESLErrorMsg( "定義されていない演算子が指定されました。" ) ;
}

// 演算子の operator オーバーロード名を取得する
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSCompiler::GetOperatorString( const OPERATOR_INFO & opinf )
{
	static const wchar_t *	pwszMoves[csotMax] =
	{
		L"+=", L"-=", L"*=", L"/=", L"%=",
		L"&=", L"|=", L"^=", NULL, NULL, L">>=", L"<<="
	} ;
	static const wchar_t *	pwszOperators[csotMax] =
	{
		L"+", L"-", L"*", L"/", L"mod",
		L"and", L"or", L"xor", L"land", L"lor",
		L"shift_right", L"shift_left",
	} ;
	static const wchar_t *	pwszComparators[csctMax] =
	{
		L"!=", L"==", L"<", L"<=", L">", L">=", L"!==", L"===",
	} ;
	static const wchar_t *	pwszUnaryOperators[csuotMax] =
	{
		L"+", L"-", L"~", L"!", L"++", L"--", L"++", L"--",
	} ;
	static const wchar_t *	pwszExUnaryOperators[csxuotMax] =
	{
		L"deselect", L"boolean", L"sizeof", L"typeof",
		NULL, NULL, NULL,
		L"&", L"*",
	} ;
	switch ( opinf.opiType )
	{
	case	optMove:
		if ( opinf.optOperator == csotNop )
		{
			return	L":=" ;
		}
		else if ( opinf.optOperator < csotMax )
		{
			return	pwszMoves[opinf.optOperator] ;
		}
		break ;

	case	optGeneral:
		if ( opinf.optOperator < csotMax )
		{
			return	pwszOperators[opinf.optOperator] ;
		}
		break ;

	case	optCompare:
		if ( opinf.cptCompare < csctMax )
		{
			return	pwszComparators[opinf.cptCompare] ;
		}
		break ;

	case	optMember:
		return	L"." ;

	case	optPtrMember:
		return	L"->" ;

	case	optMemberFunc:
		return	L".*" ;

	case	optPtrMemberFunc:
		return	L"->*" ;

	case	optReference:
		return	L"[]" ;

	case	optUnary:
		if ( opinf.uoptUnary < csctMax )
		{
			return	pwszUnaryOperators[opinf.uoptUnary] ;
		}
		break ;

	case	optExtraOperator:
		if ( opinf.xoptExOperator == csxotMoveReference )
		{
			return	L"::=" ;
		}
		break ;

	case	optExtraUniary:
		if ( opinf.xuoptExUnary < csxuotMax )
		{
			return	pwszExUnaryOperators[opinf.xuoptExUnary] ;
		}
		break ;

	case	optArgument:
	case	optNew:
	case	optCompareSelector:
	case	optListExpression:
		return	NULL ;
	}
	return	NULL ;
}

// 演算子の優先度を取得する
//////////////////////////////////////////////////////////////////////////////
int ECSCompiler::GetOperatorPriority
	( const ECSCompiler::OPERATOR_INFO & opinf )
{
	switch ( opinf.opiType )
	{
	case	optMove:
		return	oppMove ;
	case	optGeneral:
		switch ( opinf.optOperator )
		{
		case	csotAdd:
		case	csotSub:
			return	oppAdd ;
		case	csotMul:
		case	csotDiv:
		case	csotMod:
			return	oppMul ;
		case	csotAnd:
			return	oppAnd ;
		case	csotOr:
			return	oppOr ;
		case	csotXor:
			return	oppXor ;
		case	csotLogicalAnd:
			return	oppLAnd ;
		case	csoutLogicalOr:
			return	oppLOr ;
		case	csotShiftRight:
		case	csotShiftLeft:
			return	oppShift ;
		}
		break ;
	case	optCompare:
		return	oppCompare ;
	case	optMember:
	case	optPtrMember:
	case	optMemberFunc:
	case	optPtrMemberFunc:
	case	optReference:
		return	oppMember ;
	case	optArgument:
		return	oppArgument ;
	case	optExtraOperator:
		return	oppExOperator ;
	case	optUnary:
	case	optExtraUniary:
	case	optNew:
		return	GetUnaryOperatorPriority( opinf ) ;
	case	optCompareSelector:
		return	oppCmpSelector ;
	case	optListExpression:
		return	oppList ;
	}
	return	oppNothing ;
}

int ECSCompiler::GetUnaryOperatorPriority
	( const ECSCompiler::OPERATOR_INFO & opinf )
{
	switch ( opinf.opiType )
	{
	case	optUnary:
		switch ( opinf.uoptUnary )
		{
		case	csuotPlus:
		case	csuotNegate:
		case	csuotBitNot:
		case	csuotLogicalNot:
		case	csuotIncrement:
		case	csuotDecrement:
		case	csuotIncrementAfter:
		case	csuotDecrementAfter:
			return	oppUnary ;
		}
		break ;
	case	optExtraUniary:
		switch ( opinf.xuoptExUnary )
		{
		case	csxuotBoolean:
		case	csxuotSizeOf:
		case	csxuotTypeOf:
		case	csxuotDeselect:
		case	csxuotLoadAddress:
		case	csxuotRefAddress:
			return	oppExUnary ;
		}
		break ;
	case	optNew:
		return	oppNew ;
	}
	return	oppNothing ;
}
