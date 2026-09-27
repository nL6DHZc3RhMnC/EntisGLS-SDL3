
/*****************************************************************************
             Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
	Copyright (c) 2002-2014 Leshade Entis, Entis-soft. All rights reserved.
 ****************************************************************************/


#include <gls.h>


//////////////////////////////////////////////////////////////////////////////
// 詞葉 C style 文字列ストリーミング
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ECSCStyleCompiler::ECSCStyleStream, ECSSourceStream )
IMPLEMENT_CLASS_INFO( ECSCStyleCompiler::ECSCStyleSourceStream, ECSCStyleStream )
IMPLEMENT_CLASS_INFO( ECSCStyleCompiler::ECSCStyleFrontStream, ECSCStyleStream )

// 空白文字を読み飛ばす
//////////////////////////////////////////////////////////////////////////////
int ECSCStyleCompiler::ECSCStyleStream::DisregardSpace( void )
{
	return	ECSCStyleCompiler::SeekNextCStyleSource( *this ) ;
}

int ECSCStyleCompiler::ECSCStyleSourceStream::DisregardSpace( void )
{
	return	m_compiler->SeekNextSourceStream( this ) ;
}

int ECSCStyleCompiler::ECSCStyleFrontStream::DisregardSpace( void )
{
	ECSCStyleSourceStream *	pcsssLast = m_compiler->GetCurrentSourceStream() ;
	while ( ECSCStyleStream::DisregardSpace()
					&& m_compiler->IsInCStyleMode() )
	{
		ECSCStyleSourceStream *	pcsss = m_compiler->GetCurrentSourceStream() ;
		if ( pcsss != NULL )
		{
			if ( m_pcsssLast != pcsss )
			{
				m_strFilePath = pcsss->m_strFilePath ;
				m_pcsssLast = pcsss ;
			}
			m_nLine = pcsss->m_nLine ;
		}
		if ( m_compiler->IsEndOfAllSourceStream() )
		{
			return	1 ;
		}
		int	nIndex = GetIndex() ;
		if ( !m_compiler->AddNextFilteredSourceLine( *this ) )
		{
			MoveIndex( nIndex ) ;
			break ;
		}
		MoveIndex( nIndex ) ;
	}
	return	m_compiler->IsInCStyleMode() ? 0 : 1 ;
}

// 現在の文字列（区切り記号は無視）を通過する
//////////////////////////////////////////////////////////////////////////////
void ECSCStyleCompiler::ECSCStyleStream::PassEnclosedString
	( wchar_t wchClose, int flagCtrlCode )
{
	if ( (wchClose != L'\"') && (wchClose != L'\'') )
	{
		while ( !DisregardSpace() )
		{
			if ( CurrentCharacter() == wchClose )
			{
				break ;
			}
			PassAExpressionTerm( flagCtrlCode ) ;
		}
	}
	else
	{
		ECSSourceStream::PassEnclosedString( wchClose, flagCtrlCode ) ;
	}
}

// 1ステートメント又は行末まで読み飛ばす
//////////////////////////////////////////////////////////////////////////////
bool ECSCStyleCompiler::ECSCStyleFrontStream::PassAStatementLine( void )
{
	while ( !ECSSourceStream::DisregardSpace() )
	{
		wchar_t	wch = CurrentCharacter() ;
		if ( (wch == L';') || (wch == L'{') || (wch == L'}') )
		{
			GetCharacter() ;
			return	true ;
		}
		PassAExpressionTerm( ECSSourceStream::flagNormal ) ;
	}
	return	false ;
}

// 行バッファ更新
//////////////////////////////////////////////////////////////////////////////
void ECSCStyleCompiler::ECSCStyleFrontStream::FlushLine( void )
{
	int			iBase = GetIndex() ;
	int			nLength = GetLength() ;
	int			nCount = nLength - iBase ;
	wchar_t *	pwszBuf = GetBuffer( nLength ) ;
	//
	for ( int i = 0; i < nCount; i ++ )
	{
		pwszBuf[i] = pwszBuf[iBase + i] ;
	}
	ReleaseBuffer( nCount ) ;
	//
	MoveIndex( 0 ) ;
}


//////////////////////////////////////////////////////////////////////////////
// Entis Cotopha Script C style コンパイラ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ECSCStyleCompiler, ECSCompiler )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ECSCStyleCompiler::ECSCStyleCompiler( void )
{
	m_flagCStyle = false ;
	m_pTemplateNest = NULL ;
	m_csintCotopha.SetValue( 3 ) ;
	m_dwCompileTime = ::timeGetTime() ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ECSCStyleCompiler::~ECSCStyleCompiler( void )
{
}

// スクリプトをコンパイルする
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileScript
	( ECSSourceStream & cssScript, const char * pszFilePath )
{
	ECSCStyleSourceStream *	pcsss = new ECSCStyleSourceStream( this ) ;
	*pcsss = cssScript ;
	//
	pcsss->m_strFilePath = pszFilePath ;
	m_lstSource.Push( pcsss ) ;
	//
	m_strFilePath = pszFilePath ;
	m_nLineNum = 1 ;
	//
	return	CompileAllScript( pcsss ) ;
}

// コンパイルを完了する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::FinishCompile( DWORD dwFlags )
{
	SetCStyleMode( false, false ) ;
	//
	return	ECSCompiler::FinishCompile( dwFlags ) ;
}

// スクリプトをインクルードする
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::IncludeSourceScript( const char * pszFileName )
{
	EString	strFilePath = pszFileName ;
	ESLFileObject *	pfile = OpenScriptFile( strFilePath ) ;
	if ( pfile == NULL )
	{
		m_strErrMsg =
			"\'" + strFilePath + "\' ファイルを開けませんでした。" ;
		return	ESLErrorMsg( m_strErrMsg ) ;
	}
	/*
	ERawFile *	pRawFile = ESLTypeCast<ERawFile>( pfile ) ;
	if ( pRawFile != NULL )
	{
		strFilePath = pRawFile->GetFilePath() ;
	}
	*/
	return	IncludeSourceScript( pfile, pszFileName ) ;
}

ESLError ECSCStyleCompiler::IncludeSourceScript
			( ESLFileObject * pfile, const char * pszFileName )
{
	ECSCStyleSourceStream *	pcsss = new ECSCStyleSourceStream( this ) ;
	pcsss->ReadTextFile( *pfile ) ;
	delete	pfile ;
	//
	pcsss->m_strFilePath = pszFileName ;
	pcsss->m_fReturnMode = true ;
	pcsss->m_fReturnCStyle = IsInCStyleMode() ;
	pcsss->m_fReturnCCompatible = IsInCCompatibleMode() ;
	m_lstSource.Push( pcsss ) ;
	//
	m_strFilePath = pszFileName ;
	m_nLineNum = 1 ;
	//
	return	eslErrSuccess ;
}

// スクリプトを最後までコンパイルする
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileAllScript( ECSCStyleSourceStream * pcsss )
{
	if ( pcsss != NULL )
	{
		pcsss->AddRef() ;
	}
	ESLError	err = eslErrSuccess ;
	while ( !IsEndOfAllSourceStream()
		&& ((pcsss == NULL) || !pcsss->IsIndexOverflow()) )
	{
		err = eslErrSuccess ;
		if ( IsInCStyleMode() )
		{
			err = CompileCStyleScript() ;
		}
		else
		{
			ECSCStyleSourceStream *	pcsss = GetCurrentSourceStream() ;
			if ( pcsss != NULL )
			{
				int	nFirstLine = pcsss->m_nLine ;
				m_strFilePath = pcsss->m_strFilePath ;
				m_nLineNum = pcsss->m_nLine ;
				//
				ECSSourceStream	cssLine ;
				GetNextScriptLine( cssLine, *pcsss, pcsss->m_nLine ) ;
				//
				err = CompileScriptLine
						( cssLine, nFirstLine, pcsss->m_strFilePath ) ;
				//
				if ( pcsss->IsIndexOverflow() )
				{
					ESLVerify( pcsss = m_lstSource.Pop() ) ;
					if ( pcsss->m_fReturnMode )
					{
						SetCStyleMode
							( pcsss->m_fReturnCStyle,
								pcsss->m_fReturnCCompatible ) ;
					}
					pcsss->ReleaseRef() ;
				}
			}
		}
		if ( err )
		{
			err = OutputError
				( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
			if ( err )
			{
				break;
			}
		}
	}
	if ( pcsss != NULL )
	{
		pcsss->ReleaseRef() ;
	}
	return	err ;
}

// Cスタイルスクリプトをコンパイルする
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileCStyleScript( void )
{
	ECSCStyleFrontStream	cssSource( this ) ;
	while ( IsInCStyleMode() )
	{
		ECSCStyleSourceStream *	pcsss = GetCurrentSourceStream() ;
		if ( pcsss != NULL )
		{
			cssSource.m_strFilePath = pcsss->m_strFilePath ;
			cssSource.m_nLine = pcsss->m_nLine ;
		}
		ESLError	err = CompileCStyleStatement( cssSource ) ;
		if ( err )
		{
			OutputError
				( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
			//
			cssSource.PassAStatementLine() ;
		}
		cssSource.FlushLine() ;
		//
		cssSource.TrimRight() ;
		cssSource.DisregardSpace() ;
		if ( cssSource.IsIndexOverflow() && IsEndOfAllSourceStream() )
		{
			break ;
		}
	}
	cssSource.TrimRight() ;
	if ( !cssSource.IsIndexOverflow() )
	{
		return	OutputError
			( "Cスタイルが構文の途中で終了させられました",
								m_strFilePath, m_nLineNum ) ;
	}
	return	eslErrSuccess ;
}

// Cスタイル１文コンパイルする
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileCStyleStatement
	( ECSCStyleFrontStream& cssSrc, bool fNoBlockForMulti )
{
	ESLError	err ;
	//
	// 先頭の行番号設定
	//
	cssSrc.DisregardSpace() ;
	//
	if ( !IsInCStyleMode() )
	{
		return	eslErrSuccess ;
	}
	//
	m_strFilePath = cssSrc.m_strFilePath ;
	m_nLineNum = cssSrc.m_nLine ;
	//
	// 複文判定
	//
	wchar_t	wch = cssSrc.HasToComeChar( L"{;" ) ;
	if ( wch == L'{' )
	{
		ECSCStyleStream	cssLine ;
		if ( !fNoBlockForMulti )
		{
			err = ECSCompiler::CompileBegin( cssLine ) ;
			if ( err )
			{
				OutputError
					( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
			}
		}
		err = CompileCStyleMultiStatement( cssSrc ) ;
		//
		if ( !fNoBlockForMulti )
		{
			if ( err )
			{
				OutputError
					( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
			}
			return	ECSCompiler::CompileEnd( cssLine ) ;
		}
		else
		{
			return	err ;
		}
	}
	else if ( wch == L';' )
	{
		return	eslErrSuccess ;
	}
	//
	// 予約語文判定
	//
	int			iLineFirst = cssSrc.GetIndex() ;
	EWideString	wstrToken = cssSrc.GetAToken() ;
	//
	CStyleReservedWord	csrwWord = IsCStyleReservedWord( wstrToken ) ;
	if ( csrwWord != csrwInvalid )
	{
		return	CompileCStyleReservedWord( csrwWord, cssSrc ) ;
	}
	//
	// ラベル文判定
	//
	if ( cssSrc.HasToComeToken( L":" ) )
	{
		err = VerifyUserSymbol( wstrToken ) ;
		if ( err )
		{
			return	err ;
		}
		ECSCStyleStream	cssLine ;
		cssLine = L"Private " + wstrToken ;
		return	ECSCompiler::CompileLabel( cssLine ) ;
	}
	cssSrc.MoveIndex( iLineFirst ) ;
	//
	// 定義文・式文
	//
	return	CompileCStyleDeclOrExprStatement( cssSrc ) ;
}

// Cスタイル宣言・定義・式文いずれかを１文コンパイルする
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileCStyleDeclOrExprStatement( ECSCStyleFrontStream& cssSrc )
{
	if ( cssSrc.HasToComeChar( L";" ) == L';' )
	{
		return	eslErrSuccess ;
	}
	const int	iStartLineIndex = cssSrc.GetIndex() ;
	DWORD	dwMemoryClass = ParseMemoryClass( cssSrc ) ;
	//
	// operator 判定
	//
	if ( cssSrc.HasToComeToken( L"operator" ) )
	{
		EWideString	wstrFuncName ;
		wchar_t	wchNext =
			ParseEnclosedExpression( cssSrc, L"(;", &wstrFuncName ) ;
		if ( wchNext != L'(' )
		{
			return	ESLErrorMsg( "operator に引数が指定されていません" ) ;
		}
		EWideString	wstrArgList = cssSrc.GetEnclosedString( L')', m_dwModeFlags ) ;
		if ( cssSrc.GetAt( cssSrc.GetIndex() - 1 ) != L')' )
		{
			return	ESLErrorMsg
				( "operator 関数の引数が \')\' で閉じられていません" ) ;
		}
		wstrFuncName = L"operator " + wstrFuncName ;
		return	CompileFunctionDeclaration
			( dwMemoryClass, L"", wstrFuncName, wstrArgList, cssSrc, false ) ;
	}
	//
	// 構築関数・消滅関数判定
	//
	bool	fDestructor = false ;
	if ( cssSrc.HasToComeChar( L"~" ) == L'~' )
	{
		fDestructor = true ;
	}
	EControlNest *
		pClassNest = GetMostInnerNest( rwStructure, rwUnion ) ;
	if ( pClassNest != NULL )
	{
		ECSClassInfo *	pClassInf =
					GetClassInfoAs( pClassNest->m_wstrCurSpaceName ) ;
		const int	iSaveIndex = cssSrc.GetIndex() ;
		if ( (pClassInf != NULL)
			&& (cssSrc.HasToComeToken( pClassInf->GetName() ))
			&& (cssSrc.HasToComeChar( L"(" ) == L'(') )
		{
			//
			// ブロック内構築・消滅関数
			//
			EWideString	wstrFuncName ;
			if ( fDestructor )
			{
				wstrFuncName += L'~' ;
			}
			wstrFuncName += pClassInf->GetName() ;
			//
			EWideString	wstrArgList = cssSrc.GetEnclosedString( L')', m_dwModeFlags ) ;
			if ( cssSrc.GetAt( cssSrc.GetIndex() - 1 ) != L')' )
			{
				if ( !fDestructor )
				{
					return	ESLErrorMsg
						( "構築関数の引数が \')\' で閉じられていません" ) ;
				}
				else
				{
					return	ESLErrorMsg
						( "消滅関数の引数が \')\' で閉じられていません" ) ;
				}
			}
			return	CompileFunctionDeclaration
				( dwMemoryClass, L"void",
					wstrFuncName, wstrArgList, cssSrc ) ;
		}
		cssSrc.MoveIndex( iSaveIndex ) ;
	}
//	else
	{
		ECSClassInfo *	pClassInf ;
		EWideString	wstrNameSpace ;
		EWideString	wstrName = cssSrc.GetAToken() ;
		if ( fDestructor )
		{
			wstrName = L"~" + wstrName ;
			fDestructor = false ;
		}
		wstrNameSpace = wstrName ;
		for ( ; ; )
		{
			pClassInf = GetClassInfoAs( wstrNameSpace ) ;
			if ( (pClassInf == NULL)
				|| !cssSrc.HasToComeToken( L"::" ) )
			{
				break ;
			}
			EWideString	wstrToken = cssSrc.GetAToken() ;
			if ( wstrToken == L"~" )
			{
				fDestructor = true ;
				wstrToken = cssSrc.GetAToken() ;
			}
			if ( wstrToken != wstrName )
			{
				wstrNameSpace += L"::" ;
				if ( fDestructor )
				{
					wstrNameSpace += L'~' ;
					fDestructor = false ;
				}
				wstrNameSpace += wstrToken ;
				wstrName = wstrToken ;
				continue ;
			}
			if ( cssSrc.HasToComeChar( L"(" ) != L'(' )
			{
				break ;
			}
			//
			// ブロック外構築・消滅関数
			//
			EWideString	wstrFuncName ;
			wstrFuncName = wstrNameSpace + L"::" ;
			if ( fDestructor )
			{
				wstrFuncName += L'~' ;
			}
			wstrFuncName += wstrToken ;
			//
			EWideString	wstrArgList = cssSrc.GetEnclosedString( L')', m_dwModeFlags ) ;
			if ( cssSrc.GetAt( cssSrc.GetIndex() - 1 ) != L')' )
			{
				if ( !fDestructor )
				{
					return	ESLErrorMsg
						( "構築関数の引数が \')\' で閉じられていません" ) ;
				}
				else
				{
					return	ESLErrorMsg
						( "消滅関数の引数が \')\' で閉じられていません" ) ;
				}
			}
			return	CompileFunctionDeclaration
				( dwMemoryClass, L"void",
					wstrFuncName, wstrArgList, cssSrc ) ;
		}
		if ( (wstrName == L"operator")
			&& (m_pStatementCache == NULL)
			&& (GetMostInnerNest( rwFunction, rwFunction ) == NULL) )
		{
			// クラスメンバの operator 実装
			EWideString	wstrFuncName ;
			wchar_t	wchNext =
				ParseEnclosedExpression( cssSrc, L"(;", &wstrFuncName ) ;
			if ( wchNext != L'(' )
			{
				return	ESLErrorMsg( "operator に引数が指定されていません" ) ;
			}
			EWideString	wstrArgList = cssSrc.GetEnclosedString( L')', m_dwModeFlags ) ;
			if ( cssSrc.GetAt( cssSrc.GetIndex() - 1 ) != L')' )
			{
				return	ESLErrorMsg
					( "operator 関数の引数が \')\' で閉じられていません" ) ;
			}
			wstrFuncName = wstrNameSpace + L" " + wstrFuncName ;
			return	CompileFunctionDeclaration
				( dwMemoryClass, L"", wstrFuncName, wstrArgList, cssSrc, false ) ;
		}
	}
	//
	// 定義文判定
	//
	int	iLineIndex = iStartLineIndex ;
	cssSrc.MoveIndex( iStartLineIndex ) ;
	//
	dwMemoryClass = ParseMemoryClass( cssSrc ) ;
	//
	EWideString	wstrTypeExpr ;
	if ( ParseCStyleTypeDescription( wstrTypeExpr, cssSrc, false ) )
	{
		OPERATOR_INFO	opinf ;
		iLineIndex = cssSrc.GetIndex() ;
		if ( GetOperatorInfo( opinf, cssSrc.GetAToken(), false ) )
		{
			cssSrc.MoveIndex( iLineIndex ) ;
			return	CompileVarFuncDeclaration
						( dwMemoryClass, wstrTypeExpr, cssSrc ) ;
		}
		else
		{
			cssSrc = wstrTypeExpr + cssSrc.Middle( iLineIndex ) ;
			cssSrc.MoveIndex( 0 ) ;
			iLineIndex = 0 ;
		}
	}
	//
	// 式文の処理
	//
	cssSrc.MoveIndex( iLineIndex ) ;
	return	CompileCStyleExpression( cssSrc ) ;
}

// Cスタイル複文をコンパイルする
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileCStyleMultiStatement
	( ECSCStyleFrontStream& cssSrc )
{
	for ( ; ; )
	{
		if ( cssSrc.DisregardSpace() )
		{
			return	ESLErrorMsg
				( "\'{\' に対応する \'}\' が見つかりませんでした" ) ;
		}
		if ( !IsInCStyleMode() )
		{
			return	ESLErrorMsg
				( "Cスタイルブロック内で標準モードに復帰しました" ) ;
		}
		if ( cssSrc.HasToComeChar( L"}" ) == L'}' )
		{
			return	eslErrSuccess ;
		}
		ESLError	err = CompileCStyleStatement( cssSrc ) ;
		//
		FreeExpressionTemporary() ;
		//
		if ( err )
		{
			err = OutputError
				( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
			//
			cssSrc.PassAStatementLine() ;
			if ( err )
			{
				return	err ;
			}
		}
		cssSrc.FlushLine() ;
	}
}

// 詞葉標準構文を１行コンパイルする
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileScriptLine
	( ECSSourceStream & cssLine, int nLineNum,
		const char * pszFilePath, bool fEnableUserMacro )
{
	int	iLineFirst = cssLine.GetIndex() ;
	//
	if ( cssLine.HasToComeChar( L"#" ) == L'#' )
	{
		CStyleDirective	csd = IsDirectiveWord( cssLine.GetAToken() ) ;
		if ( csd == csdInclude )
		{
			return	CompileDirectiveInclude( cssLine ) ;
		}
		else if ( csd == csdMode )
		{
			return	CompileDirectiveMode( cssLine ) ;
		}
	}
	//
	cssLine.MoveIndex( iLineFirst ) ;
	return	ECSCompiler::CompileScriptLine
				( cssLine, nLineNum, pszFilePath, fEnableUserMacro ) ;
}

// 記憶クラス修飾子をコンパイルする
// [extern] [{ shared | static }] [inline] [virtual] [native] [abstract] 
//////////////////////////////////////////////////////////////////////////////
DWORD ECSCStyleCompiler::ParseMemoryClass( ECSCStyleFrontStream & cssSrc )
{
	static const wchar_t *	pwszWords[] =
	{
		L"extern", L"shared", L"static", L"naked",
		L"inline", L"virtual", L"native", L"abstract",
		L"__jit_native__",
		NULL
	} ;
	static const DWORD		dwWordFlags[] =
	{
		flagExtern, flagShared, flagStatic, flagNaked,
		flagInline, flagVirtual, (DWORD) flagNative, flagAbstract,
		flagJITNative,
	} ;
	DWORD	dwFlags = 0 ;
	for ( ; ; )
	{
		const int	nIndex = cssSrc.GetIndex() ;
		EWideString	wstrToken = cssSrc.GetAToken() ;
		int			iWord = -1 ;
		for ( int i = 0; pwszWords[i] != NULL; i ++ )
		{
			if ( wstrToken == pwszWords[i] )
			{
				iWord = i ;
				break ;
			}
		}
		if ( iWord < 0 )
		{
			cssSrc.MoveIndex( nIndex ) ;
			break ;
		}
		dwFlags |= dwWordFlags[iWord] ;
	}
	return	dwFlags ;
}

// 型構文をコンパイルする
//////////////////////////////////////////////////////////////////////////////
bool ECSCStyleCompiler::ParseCStyleTypeDescription
	( EWideString & wstrTypeExpr,
		ECSCStyleFrontStream & cssSrc, bool fNoTemplateInstance )
{
	ECSCStyleStream	cssTypeExpr ;
	if ( cssSrc.HasToComeToken( L"const" ) )
	{
		cssTypeExpr += L"const " ;
	}
	EWideString	wstrToken = cssSrc.GetAToken() ;
	EWideString	wstrTypeName ;
	ESLError	err ;
	bool		fDeclaration = false ;
	if ( wstrToken == L"class" )
	{
		fDeclaration =
			ParseClassDeclaration( wstrTypeName, cssSrc, false ) ;
		if ( !fDeclaration )
		{
			return	false ;
		}
	}
	else if ( wstrToken == L"struct" )
	{
		fDeclaration =
			ParseClassDeclaration( wstrTypeName, cssSrc, true ) ;
		if ( !fDeclaration )
		{
			return	false ;
		}
	}
	else if ( wstrToken == L"enum" )
	{
		fDeclaration =
			ParseEnumDeclaration( wstrTypeName, cssSrc ) ;
		if ( !fDeclaration )
		{
			return	false ;
		}
	}
	/*
	else if ( wstrToken == L"data" )
	{
		fDeclaration =
			ParseDataDeclaration( wstrTypeName, cssSrc ) ;
		if ( !fDeclaration )
		{
			return	false ;
		}
	}
	*/
	else if ( wstrToken == L"void" )
	{
		wstrTypeName = wstrToken ;
	}
	else
	{
		if ( wstrToken == L"typename" )
		{
			cssTypeExpr += L"typename " ;
		}
		SYMBOL_NAMESPACE	snsSymbol( wstrToken ) ;
		err = ParseFullNameSymbol( snsSymbol, cssSrc, fNoTemplateInstance ) ;
		if ( err )
		{
			return	false ;
		}
		ECSTypeInfo::Flags	flagScope = ECSTypeInfo::flagProtected ;
		err = SearchTypeName( snsSymbol, flagScope ) ;
		if ( err )
		{
			EPtrObjArray<const wchar_t>	lstNamespace ;
			EWideString	wstrThisClass = GetCurrentThisClassName() ;
			GetUsingNamespaceList( lstNamespace ) ;
			err = ESLErrorMsg( "不正な型指定です" ) ;
			//
			for ( int i = 0; i < (int) lstNamespace.GetSize(); i ++ )
			{
				const wchar_t *	pwszNamespace = lstNamespace.GetAt( i ) ;
				if ( pwszNamespace != NULL )
				{
					EWideString	wstrThisSpace = pwszNamespace ;
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
						snsSymbol = snsTemp ;
						break ;
					}
					err = ESLErrorMsg( "不正な型指定です" ) ;
				}
			}
		}
		if ( err )
		{
			return	false ;
		}
		wstrTypeName = snsSymbol.wstrFullName ;
	}
	cssTypeExpr += wstrTypeName ;
	//
	if ( cssSrc.HasToComeChar( L"<" ) == L'<' )
	{
		EWideString	wstrSubType ;
		if ( !ParseCStyleTypeDescription( wstrSubType, cssSrc, false ) )
		{
			return	fDeclaration ;
		}
		cssTypeExpr += L'<' ;
		cssTypeExpr += wstrSubType ;
		cssTypeExpr += L'>' ;
		//
		if ( cssSrc.HasToComeChar( L">" ) != L'>' )
		{
			OutputError
				( "\'<\' に対応する \'>\' が見つかりません",
								m_strFilePath, m_nLineNum ) ;
		}
	}
	for ( ; ; )
	{
		wchar_t	wch = cssSrc.HasToComeChar( L"[&*" ) ;
		if ( wch == L'[' )
		{
			cssTypeExpr += L'[' ;
			cssTypeExpr += cssSrc.GetEnclosedString( L']', m_dwModeFlags ) ;
			cssTypeExpr += L']' ;
			//
			if ( cssSrc.GetAt( cssSrc.GetIndex() - 1 ) != L']' )
			{
				OutputError
					( "\'[\' に対応する \']\' が見つかりません",
									m_strFilePath, m_nLineNum ) ;
			}
		}
		else if ( wch == L'&' )
		{
			cssTypeExpr += L'&' ;
		}
		else if ( wch == L'*' )
		{
			cssTypeExpr += L'*' ;
			//
			bool	fPointerNext = true ;
			for ( ; ; )
			{
				if ( cssSrc.HasToComeToken( L"const" ) )
				{
					if ( !fPointerNext )
					{
						cssTypeExpr += L" " ;
					}
					cssTypeExpr += L"const" ;
					fPointerNext = false ;
				}
				else if ( cssSrc.HasToComeToken( L"naked" ) )
				{
					if ( !fPointerNext )
					{
						cssTypeExpr += L" " ;
					}
					cssTypeExpr += L"naked" ;
					fPointerNext = false ;
				}
				else
				{
					break ;
				}
			}
		}
		else
		{
			break ;
		}
	}
	wstrTypeExpr = cssTypeExpr ;
	return	true ;
}

// 配列型構文を解釈して追加する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::ParseCStyleTypeArrayDecoration
	( EWideString & wstrTypeExpr, ECSCStyleFrontStream & cssSrc )
{
	ESLError	err = eslErrSuccess ;
	while ( cssSrc.HasToComeChar( L"[" ) == L'[' )
	{
		wstrTypeExpr += L'[' ;
		if ( cssSrc.HasToComeChar( L"]" ) != L']')
		{
			EWideString	wstrSize =
					cssSrc.GetEnclosedString( L']', m_dwModeFlags ) ;
			wstrTypeExpr += wstrSize ;
			//
			if ( cssSrc.GetAt( cssSrc.GetIndex() - 1 ) != L']' )
			{
				err = ESLErrorMsg
					( "配列要素指定に閉じ括弧 \']\' が見つかりません。" ) ;
				break ;
			}
		}
		wstrTypeExpr += L']' ;
	}
	return	err ;
}

// 式文をコンパイルする
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileCStyleExpression
	( ECSCStyleFrontStream & cssSrc )
{
	ECSSourceStream	cssExpr ;
	ESLError	err ;
	int			iLastIndex = cssSrc.GetIndex() ;
	wchar_t		wchClose =
		ParseEnclosedExpression( cssSrc, L";}", &cssExpr ) ;
	if ( wchClose == L'}' )
	{
		ESLAssert( cssSrc.GetAt( cssSrc.GetIndex() - 1 ) == wchClose ) ;
		cssSrc.MoveIndex( cssSrc.GetIndex() - 1 ) ;
	}
	cssExpr.MoveIndex( 0 ) ;
	err = CompileExpressionStatement( cssExpr ) ;
	if ( err )
	{
		//
		// 行頭か判定
		//
		int		i ;
		bool	fLineFirst = false ;
		bool	fLastSemiColon = true ;
		for ( i = cssExpr.GetIndex() - 1; i >= 0; i -- )
		{
			wchar_t	wch = cssExpr.GetAt( i ) ;
			if ( wch == L'\n' )
			{
				fLineFirst = true ;
				fLastSemiColon = false ;
			}
			else if ( fLineFirst && (wch == L';') )
			{
				fLastSemiColon = true ;
				break ;
			}
			else if ( wch > L' ' )
			{
				break ;
			}
		}
		if ( !fLastSemiColon )
		{
			EString	strErrMsg = GetESLErrorMsg(err) ;
			m_strErrMsg = strErrMsg
				+ "（直前行末に \';\' が記述されていない為かもしれません）" ;
			err = ESLErrorMsg(m_strErrMsg) ;
		}
		return	err ;
	}
	if ( cssSrc.GetAt( cssSrc.GetIndex() - 1 ) != L';' )
	{
		return	ESLErrorMsg( "文末に \';\' が見つかりません" ) ;
	}
	return	eslErrSuccess ;
}

// 変数・関数定義文をコンパイルする
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileVarFuncDeclaration
	( DWORD dwMemoryClass,
		const EWideString & wstrVarTypeName, ECSCStyleFrontStream & cssSrc )
{
	EWideString	wstrBaseType = wstrVarTypeName ;
	//
	if ( cssSrc.HasToComeChar( L"<" ) == L'<' )
	{
		EWideString	wstrSubType ;
		if ( !ParseCStyleTypeDescription( wstrSubType, cssSrc, false ) )
		{
			return	ESLErrorMsg( " <> 括弧内に型が記述されていません" ) ;
		}
		else
		{
			wstrBaseType += L'<' ;
			wstrBaseType += wstrSubType ;
			wstrBaseType += L'>' ;
			//
			if ( cssSrc.HasToComeChar( L">" ) == L'>' )
			{
				return	ESLErrorMsg
					( "\'<\' に対応する \'>\' が見つかりません" ) ;
			}
		}
	}
	for ( ; ; )
	{
		wchar_t	wch = cssSrc.HasToComeChar( L"[&" ) ;
		if ( wch == L'[' )
		{
			wstrBaseType += L'[' ;
			wstrBaseType += cssSrc.GetEnclosedString( L']', m_dwModeFlags ) ;
			wstrBaseType += L']' ;
			//
			if ( cssSrc.GetAt( cssSrc.GetIndex() - 1 ) != L']' )
			{
				return	ESLErrorMsg
					( "\'[\' に対応する \']\' が見つかりません" ) ;
			}
		}
		else if ( wch == L'&' )
		{
			wstrBaseType += L'&' ;
		}
		else
		{
			break ;
		}
	}
	for ( ; ; )
	{
		if ( cssSrc.HasToComeChar( L";" ) == L';' )
		{
			break ;
		}
		//
		// 変数・関数名取得
		//
		EWideString	wstrToken = cssSrc.GetAToken() ;
		EWideString	wstrName = wstrToken ;
		ESLError	err ;
		err = VerifyUserSymbol( wstrName ) ;
		if ( err && (wstrToken != L"operator") )
		{
			return	err ;
		}
		while ( cssSrc.HasToComeToken( L"::" ) )
		{
			wstrToken = cssSrc.GetAToken() ;
			err = VerifyUserSymbol( wstrToken ) ;
			if ( err && (wstrToken != L"operator") )
			{
				return	err ;
			}
			wstrName += L"::" + wstrToken ;
		}
		bool	fOperator = false ;
		if ( wstrToken == L"operator" )
		{
			wstrToken = cssSrc.GetAToken() ;
			if ( wstrToken == L"(" )
			{
				return	ESLErrorMsg
					( "operator に演算子が指定されていません" ) ;
			}
			else if ( wstrToken == L"[" )
			{
				if ( cssSrc.HasToComeChar( L"]" ) != L']' )
				{
					return	ESLErrorMsg
						( "\'[\' に対応する \']\' が見つかりません" ) ;
				}
				wstrToken = L"[]" ;
			}
			wstrName += L' ' ;
			wstrName += wstrToken ;
			fOperator = true ;
		}
		//
		// 後置配列修飾判定
		//
		EWideString	wstrTypeExpr = wstrBaseType ;
		err = ParseCStyleTypeArrayDecoration( wstrTypeExpr, cssSrc ) ;
		if ( err )
		{
			return	err ;
		}
		//
		// 関数・構築関数判定
		//
		bool		fConstruction = false ;
		EWideString	wstrArgList ;
		if ( cssSrc.HasToComeChar( L"(" ) == L'(' )
		{
			ECSSourceStream	cssArgList = cssSrc.GetEnclosedString( L')', m_dwModeFlags ) ;
			if ( cssSrc.GetAt( cssSrc.GetIndex() - 1 ) != L')' )
			{
				return	ESLErrorMsg( "関数の引数が \')\' が閉じられていません" ) ;
			}
			bool	fFuncArgType = true ;
			cssArgList.MoveIndex( 0 ) ;
			if ( !cssArgList.DisregardSpace() )
			{
				ECSTypeInfo	typeFirstArg ;
				if ( ParseTypeDescription( typeFirstArg, cssArgList ) )
				{
					fFuncArgType = false ;
				}
			}
			if ( fFuncArgType )
			{
				return	CompileFunctionDeclaration
					( dwMemoryClass, wstrTypeExpr, wstrName, cssArgList, cssSrc ) ;
			}
			fConstruction = true ;
			wstrArgList = L"( " + cssArgList + L" )" ;
		}
		else if ( fOperator )
		{
			return	ESLErrorMsg
				( "operator 関数の引数が \'(\' で始まっていません" ) ;
		}
		//
		// 記憶クラスチェック
		//
		if ( dwMemoryClass & flagAbstract )
		{
			OutputWarning0
				( "変数宣言に abstract が指定されています",
									m_strFilePath, m_nLineNum ) ;
		}
		if ( dwMemoryClass & flagVirtual )
		{
			OutputWarning0
				( "変数宣言に virtual が指定されています",
									m_strFilePath, m_nLineNum ) ;
		}
		if ( dwMemoryClass & flagInline )
		{
			OutputWarning0
				( "変数宣言に inline が指定されています",
									m_strFilePath, m_nLineNum ) ;
		}
		if ( dwMemoryClass & flagNative )
		{
			OutputWarning0
				( "変数宣言に native が指定されています",
									m_strFilePath, m_nLineNum ) ;
		}
		//
		// 変数宣言
		//
		if ( dwMemoryClass & flagExtern )
		{
			ECSCStyleStream	cssDecl ;
			if ( dwMemoryClass & flagShared )
			{
				cssDecl += L"shared " ;
			}
			if ( dwMemoryClass & flagNaked )
			{
				cssDecl += L"naked " ;
			}
			cssDecl += wstrName ;
			cssDecl += L" : " ;
			cssDecl += wstrTypeExpr ;
			//
			cssDecl.MoveIndex( 0 ) ;
			//
			err = CompileDeclareDef( cssDecl ) ;
			if ( err )
			{
				return	err ;
			}
			if ( fConstruction )
			{
				OutputWarning0
					( "extern 宣言で構築関数引数が指定されています",
										m_strFilePath, m_nLineNum ) ;
			}
		}
		//
		// 変数定義
		//
		EWideString	wstrInitExpr ;
		wchar_t		wchInitEqu = L'\0' ;
		wchar_t		wchNext ;
		if ( !fConstruction )
		{
			wchInitEqu = cssSrc.HasToComeChar( L"=" ) ;
			if ( wchInitEqu != L'=' )
			{
				if ( cssSrc.HasToComeToken( L":=" ) )
				{
					wchInitEqu = L'=' ;
				}
			}
		}
		if ( wchInitEqu == L'=' )
		{
			if ( dwMemoryClass & flagExtern )
			{
				return	ESLErrorMsg
					( "extern 指定されているのに初期値を指定しようとしています" ) ;
			}
			wchNext = ParseEnclosedExpression
							( cssSrc, L",;", &wstrInitExpr ) ;
		}
		else
		{
			wchNext = cssSrc.HasToComeChar( L",;" ) ;
		}
		if ( !(dwMemoryClass & flagExtern) )
		{
			ECSCStyleStream	cssLine ;
			if ( !(dwMemoryClass & flagShared) || (dwMemoryClass & flagNaked) )
			{
				if ( dwMemoryClass & flagStatic )
				{
					cssLine += L"static " ;
				}
				if ( dwMemoryClass & flagShared )
				{
					cssLine += L"shared " ;
				}
				if ( dwMemoryClass & flagNaked )
				{
					cssLine += L"naked " ;
				}
				cssLine += wstrTypeExpr ;
				cssLine += L' ' ;
				cssLine += wstrName ;
				//
				if ( wchInitEqu == L'=' )
				{
					cssLine += L'=' ;
					cssLine += wstrInitExpr ;
				}
				else if ( fConstruction )
				{
					cssLine += wstrArgList ;
				}
				//
				cssLine.MoveIndex( 0 ) ;
				//
				err = CompileVariable( cssLine ) ;
				if ( err )
				{
					return	err ;
				}
			}
			else
			{
				cssLine += wstrName ;
				cssLine += L" : " ;
				cssLine += wstrTypeExpr ;
				//
				cssLine.MoveIndex( 0 ) ;
				//
				err = CompileData( cssLine ) ;
				if ( err )
				{
					return	err ;
				}
				if ( (wchInitEqu == L'=') || fConstruction )
				{
					OutputWarning0
						( "shared 記憶クラスで初期値が指定されています",
											m_strFilePath, m_nLineNum ) ;
				}
			}
		}
		if ( wchNext == L';' )
		{
			break ;
		}
		else if ( wchNext != L',' )
		{
			return	ESLErrorMsg
				( "変数の宣言が \',\' で区切られていません" ) ;
		}
	}
	return	eslErrSuccess ;
}

// 関数定義文をコンパイルする
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileFunctionDeclaration
	( DWORD dwMemoryClass,
		const wchar_t * pwszRetType,
		const EWideString & wstrFuncName,
		const EWideString & wstrArgList,
		ECSCStyleFrontStream & cssSrc, bool fRetType )
{
	//
	// 関数修飾子
	//
	EWideString	wstrThrows ;
	while ( !cssSrc.DisregardSpace() )
	{
		if ( cssSrc.HasToComeToken( L"const" ) )
		{
			dwMemoryClass |= flagConstant ;
		}
		else if ( cssSrc.HasToComeToken( L"naked" ) )
		{
			dwMemoryClass |= flagNakedCall ;
		}
		else if ( cssSrc.HasToComeToken( L"objected" ) )
		{
			dwMemoryClass |= flagObjected ;
		}
		else if ( cssSrc.HasToComeToken( L"throw" ) )
		{
			if ( cssSrc.HasToComeChar( L"(" ) != L'(' )
			{
				return	ESLErrorMsg( "throw に \'(\' が見つかりません" ) ;
			}
			wchar_t	wchClose =
				ParseEnclosedExpression( cssSrc, L");{}", &wstrThrows ) ;
			if ( wchClose != L')' )
			{
				return	ESLErrorMsg
					( "\'(\' に対応する \')\' が見つかりません" ) ;
			}
			wstrThrows = L"throw( " + wstrThrows + L" )" ;
		}
		else if ( cssSrc.HasToComeChar( L"=" ) == L'=' )
		{
			if ( cssSrc.HasToComeToken( L"0" ) )
			{
				dwMemoryClass |= flagAbstract ;
			}
			else
			{
				return	ESLErrorMsg
					( "関数定義構文で不正な \'=\' が指定されています" ) ;
			}
		}
		else
		{
			break ;
		}
	}
	//
	// プロトタイプ構文生成
	//
	ECSCStyleStream	cssLine ;
	if ( dwMemoryClass & flagVirtual )
	{
		cssLine += L"virtual " ;
	}
	if ( dwMemoryClass & flagJITNative )
	{
		cssLine += L" __jit_native__" ;
	}
	if ( fRetType )
	{
		cssLine += pwszRetType ;
		cssLine += L' ' ;
	}
	cssLine += wstrFuncName ;
	cssLine += L'(' ;
	cssLine += wstrArgList ;
	cssLine += L')' ;
	//
	if ( dwMemoryClass & flagConstant )
	{
		cssLine += L" const" ;
	}
	if ( dwMemoryClass & flagStatic )
	{
		cssLine += L" static" ;
	}
	if ( dwMemoryClass & flagAbstract )
	{
		cssLine += L" abstract" ;
	}
	if ( dwMemoryClass & flagInline )
	{
		cssLine += L" inline" ;
	}
	if ( dwMemoryClass & flagNative )
	{
		cssLine += L" native" ;
	}
	if ( dwMemoryClass & flagNakedCall )
	{
		cssLine += L" naked" ;
	}
	if ( dwMemoryClass & flagObjected )
	{
		cssLine += L" objected" ;
	}
	if ( !wstrThrows.IsEmpty() )
	{
		cssLine += L" " ;
		cssLine += wstrThrows ;
	}
	if ( dwMemoryClass & flagExtern )
	{
		OutputWarning0
			( "関数宣言に extern が指定されています",
								m_strFilePath, m_nLineNum ) ;
	}
	if ( dwMemoryClass & flagShared )
	{
		OutputWarning0
			( "関数宣言に shared が指定されています",
								m_strFilePath, m_nLineNum ) ;
	}
	cssLine.MoveIndex( 0 ) ;
	//
	// 宣言文か？
	//
	wchar_t	wch = cssSrc.HasToComeChar( L":;{" ) ;
	if ( wch == L':' )
	{
		EWideString	wstrExpr ;
		cssLine += L':' ;
		wch = ParseEnclosedExpression( cssSrc, L";{", &wstrExpr ) ;
		cssLine += wstrExpr ;
	}
	if ( wch == L';' )
	{
		return	CompilePrototype( cssLine ) ;
	}
	else if ( wch != L'{' )
	{
		ESLError	err = CompilePrototype( cssLine ) ;
		if ( err )
		{
			return	err ;
		}
		return	ESLErrorMsg( "関数宣言文に \';\' が見つかりません" ) ;
	}
	//
	// 関数定義
	//
	ESLError	err = CompileFunction( cssLine ) ;
	if ( err )
	{
		return	err ;
	}
	err = CompileCStyleMultiStatement( cssSrc ) ;
	if ( err )
	{
		OutputError( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
	}
	cssLine = L"" ;
	cssLine.MoveIndex( 0 ) ;
	return	CompileEndFunc( cssLine ) ;
}

// class/struct 構文を詞葉構文に変換する
//////////////////////////////////////////////////////////////////////////////
ECSCompiler::ReservedWord
	ECSCStyleCompiler::ParseClassDeclarationToBasicStatement
		( EWideString & wstrStatement,
			EWideString & wstrTagName, wchar_t & wchNext,
			ECSCStyleFrontStream & cssSrc, bool fStruct )
{
	//
	// native 指定と名前を取得
	//
	ESLError	err ;
	EWideString	wstrNative ;
	if ( cssSrc.HasToComeToken( L"native" ) )
	{
		wstrNative = L"native " ;
	}
	else if ( cssSrc.HasToComeToken( L"naked" ) )
	{
		wstrNative = L"naked " ;
	}
	wstrTagName = cssSrc.GetAToken() ;
	err = VerifyUserSymbol( wstrTagName ) ;
	if ( err )
	{
		return	rwInvalid ;
	}
	//
	// 宣言・定義判定
	//
	wchNext = cssSrc.HasToComeChar( L":{" ) ;
	if ( wchNext == L'\0' )
	{
		wstrStatement = wstrTagName ;
		wstrStatement += L" as " ;
		if ( fStruct )
		{
			wstrStatement += L"structure" ;
		}
		else
		{
			wstrStatement += L"class" ;
		}
		return	rwDeclareType ;
	}
	//
	// 定義
	//
	wstrStatement += wstrNative ;
	wstrStatement += wstrTagName ;
	//
	if ( wchNext == L':' )
	{
		wstrStatement += L" : " ;
		//
		if ( !fStruct )
		{
			EWideString	wstrParent ;
			wchNext = ParseEnclosedExpression
							( cssSrc, L"{", &wstrParent ) ;
			wstrStatement += wstrParent ;
		}
		else
		{
			for ( ; ; )
			{
				if ( !cssSrc.HasToComeToken( L"public" ) )
				{
					OutputError
						( "構造体の派生元に public 指定されていません",
											m_strFilePath, m_nLineNum ) ;
				}
				unsigned int	nLastIndex = cssSrc.GetIndex() ;
				unsigned int	nTempArgNest = 0 ;
				while ( !cssSrc.IsIndexOverflow() )
				{
					cssSrc.DisregardSpace() ;
					if ( nLastIndex < cssSrc.GetIndex() )
					{
						wstrStatement += L" " ;
					}
					wchNext = cssSrc.HasToComeChar( L"<>" ) ;
					if ( wchNext == L'<' )
					{
						nTempArgNest ++ ;
						wstrStatement += L"<" ;
					}
					else if ( wchNext == L'>' )
					{
						if ( nTempArgNest > 0 )
						{
							nTempArgNest -- ;
						}
						wstrStatement += L">" ;
					}
					else
					{
						EWideString	wstrToken = cssSrc.GetAToken() ;
						nLastIndex = cssSrc.GetIndex() ;
						if ( wstrToken.IsEmpty() )
						{
							break ;
						}
						wstrStatement += wstrToken ;
					}
					wchNext = cssSrc.HasToComeChar( L",{" ) ;
					if ( wchNext == L'{' )
					{
						break ;
					}
					if ( wchNext == L',' )
					{
						if ( nTempArgNest == 0 )
						{
							break ;
						}
						wstrStatement += wchNext ;
					}
				}
				if ( wchNext != L',' )
				{
					break ;
				}
				wstrStatement += L", " ;
			}
		}
	}
	if ( !fStruct )
	{
		return	rwClass ;
	}
	else
	{
		return	rwStructure ;
	}
}

// class/struct 構文をコンパイルする
//////////////////////////////////////////////////////////////////////////////
bool ECSCStyleCompiler::ParseClassDeclaration
	( EWideString & wstrTagName,
		ECSCStyleFrontStream & cssSrc, bool fStruct )
{
	//
	// 構文を変換する
	//
	ESLError		err ;
	ECSSourceStream	cssLine ;
	wchar_t			wchNext ;
	ReservedWord	rwType =
		ParseClassDeclarationToBasicStatement
				( cssLine, wstrTagName, wchNext, cssSrc, fStruct ) ;
	cssLine.MoveIndex( 0 ) ;
	if ( rwType == rwDeclareType )
	{
		err = CompileDeclareType( cssLine ) ;
		if ( err )
		{
			OutputError
				( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
		}
		return	true ;
	}
	else if ( rwType == rwClass )
	{
		err = CompileClass( cssLine ) ;
	}
	else if ( rwType == rwStructure )
	{
		err = CompileStructure( cssLine ) ;
	}
	else
	{
		return	false ;
	}
	if ( err )
	{
		OutputError
			( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
	}
	if ( wchNext != L'{' )
	{
		if ( !fStruct )
		{
			OutputError
				( "class に \'{\' が見つかりません",
						m_strFilePath, m_nLineNum ) ;
		}
		else
		{
			OutputError
				( "struct に \'{\' が見つかりません",
						m_strFilePath, m_nLineNum ) ;
		}
		return	true ;
	}
	//
	// クラス実体宣言
	//
	err = CompileCStyleMultiStatement( cssSrc ) ;
	if ( err )
	{
		OutputError
			( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
	}
	//
	// class/struct ブロック終端処理
	//
	cssLine = L"" ;
	cssLine.MoveIndex( 0 ) ;
	if ( !fStruct )
	{
		err = CompileEndClass( cssLine ) ;
	}
	else
	{
		err = CompileEndStruct( cssLine ) ;
	}
	if ( err )
	{
		OutputError
			( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
	}
	//
	// 一時型名定義
	//
	if ( m_pTemplateNest != NULL )
	{
		ECSTypeInfo *	pTempType = new ECSTypeInfo ;
		unsigned int	nIndex ;
		m_pTemplateNest->m_wstaTypeDef.SetAs( wstrTagName, pTempType ) ;
		if ( m_pTemplateNest->m_wstaTypeDef.GetAs( wstrTagName, &nIndex ) != NULL )
		{
			EWideString *	pwstrName =
				m_pTemplateNest->m_wstaTypeDef.GetTagAt( nIndex ) ;
			if ( pwstrName != NULL )
			{
				ECSStructure *	pStruct = new ECSStructure ;
				pStruct->m_pwszTag = *pwstrName ;
				pTempType->SetTypeValue( pStruct, 0 ) ;
			}
		}
	}
	return	true ;
}

// union 構文をコンパイルする
//////////////////////////////////////////////////////////////////////////////
bool ECSCStyleCompiler::ParseUnionDeclaration
	( EWideString & wstrTagName, ECSCStyleFrontStream & cssSrc )
{
	//
	// 名前取得
	//
	wstrTagName = cssSrc.GetAToken() ;
	ESLError	err = VerifyUserSymbol( wstrTagName ) ;
	if ( err )
	{
		return	false ;
	}
	//
	// 宣言・定義判定
	//
	wchar_t	wchNext = cssSrc.HasToComeChar( L"{" ) ;
	if ( wchNext != L'{' )
	{
		ECSCStyleStream	cssLine ;
		cssLine = wstrTagName ;
		cssLine += L" as union" ;
		cssLine.MoveIndex( 0 ) ;
		//
		err = CompileDeclareType( cssLine ) ;
		if ( err )
		{
			OutputError
				( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
		}
		return	true ;
	}
	//
	// union 定義
	//
	ECSCStyleStream	cssLine ;
	cssLine += wstrTagName ;
	cssLine.MoveIndex( 0 ) ;
	//
	err = CompileUnion( cssLine ) ;
	if ( err )
	{
		OutputError
			( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
	}
	//
	// クラス実体宣言
	//
	err = CompileCStyleMultiStatement( cssSrc ) ;
	if ( err )
	{
		OutputError
			( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
	}
	//
	// class/struct ブロック終端処理
	//
	cssLine = L"" ;
	cssLine.MoveIndex( 0 ) ;
	err = CompileEndUnion( cssLine ) ;
	if ( err )
	{
		OutputError
			( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
	}
	//
	// 一時型名定義
	//
	if ( m_pTemplateNest != NULL )
	{
		ECSTypeInfo *	pTempType = new ECSTypeInfo ;
		unsigned int	nIndex ;
		m_pTemplateNest->m_wstaTypeDef.SetAs( wstrTagName, pTempType ) ;
		if ( m_pTemplateNest->m_wstaTypeDef.GetAs( wstrTagName, &nIndex ) != NULL )
		{
			EWideString *	pwstrName =
				m_pTemplateNest->m_wstaTypeDef.GetTagAt( nIndex ) ;
			if ( pwstrName != NULL )
			{
				ECSStructure *	pStruct = new ECSStructure ;
				pStruct->m_pwszTag = *pwstrName ;
				pTempType->SetTypeValue( pStruct, 0 ) ;
			}
		}
	}
	return	true ;
}

// enum 構文をコンパイルする
//////////////////////////////////////////////////////////////////////////////
bool ECSCStyleCompiler::ParseEnumDeclaration
	( EWideString & wstrTagName, ECSCStyleFrontStream & cssSrc )
{
	//
	// abstract 指定と名前を取得
	//
	ESLError	err ;
	bool		fAbstract ;
	fAbstract = cssSrc.HasToComeToken( L"abstract" ) ;
	wstrTagName = cssSrc.GetAToken() ;
	err = VerifyUserSymbol( wstrTagName ) ;
	if ( err )
	{
		return	false ;
	}
	//
	// enum 型取得
	//
	EWideString	wstrEnumType ;
	EWideString	wstrAsCNamespace ;
	if ( cssSrc.HasToComeChar( L"<" ) == L'<' )
	{
		wstrEnumType = cssSrc.GetEnclosedString( L'>', m_dwModeFlags ) ;
		if ( cssSrc.GetAt( cssSrc.GetIndex() - 1 ) != L'>' )
		{
			OutputError
				( "\'<' に対応する \'>' が見つかりません",
									m_strFilePath, m_nLineNum ) ;
		}
		if ( fAbstract )
		{
			OutputWarning0
				( "abstract 指定されている enum に型指定されています",
											m_strFilePath, m_nLineNum ) ;
		}
	}
	else
	{
		if ( !fAbstract )
		{
			wstrEnumType = L"int64" ;
			wstrAsCNamespace = L" as c namespace" ;
		}
	}
	//
	// 宣言・定義判定
	//
	wchar_t	wchNext = cssSrc.HasToComeChar( L"{" ) ;
	if ( wchNext != L'{' )
	{
		ECSCStyleStream	cssLine ;
		cssLine = wstrTagName ;
		cssLine += L" as enumerator" ;
		cssLine.MoveIndex( 0 ) ;
		//
		err = CompileDeclareType( cssLine ) ;
		if ( err )
		{
			OutputError
				( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
		}
		return	true ;
	}
	//
	// enum 定義
	//
	ECSCStyleStream	cssLine ;
	cssLine += wstrTagName ;
	cssLine += L' ' ;
	//
	if ( !wstrEnumType.IsEmpty() )
	{
		cssLine += L'<' ;
		cssLine += wstrEnumType ;
		cssLine += L'>' ;
	}
	cssLine += wstrAsCNamespace ;
	cssLine.MoveIndex( 0 ) ;
	//
	err = CompileEnumerator( cssLine ) ;
	if ( err )
	{
		OutputError
			( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
	}
	//
	// 列挙子定義
	//
	for ( ; ; )
	{
		if ( cssSrc.DisregardSpace() )
		{
			OutputError
				( "\'{\' に対応する \'}\' が見つかりませんでした",
									m_strFilePath, m_nLineNum ) ;
			break ;
		}
		m_strFilePath = cssSrc.m_strFilePath ;
		m_nLineNum = cssSrc.m_nLine ;
		//
		if ( cssSrc.HasToComeChar( L"}" ) == L'}' )
		{
			break ;
		}
		wchar_t	wchNext =
			ParseEnclosedExpression( cssSrc, L",}", &cssLine ) ;
		//
		cssLine.MoveIndex( 0 ) ;
		//
		err = CompileEnumerate( cssLine ) ;
		if ( err )
		{
			OutputError
				( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
		}
		if ( wchNext == L'}' )
		{
			break ;
		}
		else if ( wchNext != L',' )
		{
			OutputError
				( "列挙子が \',\' で区切られていません",
								m_strFilePath, m_nLineNum ) ;
		}
		cssSrc.FlushLine() ;
	}
	//
	// enum ブロック終端処理
	//
	cssLine = L"" ;
	cssLine.MoveIndex( 0 ) ;
	err = CompileEndEnum( cssLine ) ;
	if ( err )
	{
		OutputError
			( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
	}
	return	true ;
}

// data 構文をコンパイルする
//////////////////////////////////////////////////////////////////////////////
bool ECSCStyleCompiler::ParseDataDeclaration
	( EWideString & wstrTagName, ECSCStyleFrontStream & cssSrc )
{
	//
	// 名前を取得
	//
	ESLError	err ;
	wstrTagName = cssSrc.GetAToken() ;
	err = VerifyUserSymbol( wstrTagName ) ;
	if ( err )
	{
		return	false ;
	}
	SYMBOL_NAMESPACE	snsSymbol = wstrTagName ;
	err = ParseFullNameSymbol( snsSymbol, cssSrc ) ;
	if ( err )
	{
		return	false ;
	}
	wstrTagName = snsSymbol.wstrFullName ;
	//
	// data 型取得
	//
	EWideString	wstrDataType ;
	if ( cssSrc.HasToComeChar( L"<" ) == L'<' )
	{
		wstrDataType = cssSrc.GetEnclosedString( L'>', m_dwModeFlags ) ;
		if ( cssSrc.GetAt( cssSrc.GetIndex() - 1 ) != L'>' )
		{
			OutputError
				( "\'<' に対応する \'>' が見つかりません",
									m_strFilePath, m_nLineNum ) ;
		}
	}
	//
	// 宣言・定義判定
	//
	wchar_t	wchNext = cssSrc.HasToComeChar( L"{" ) ;
	if ( wchNext != L'{' )
	{
		ECSCStyleStream	cssLine ;
		cssLine += L"data " ;
		cssLine += wstrTagName ;
		//
		if ( !wstrDataType.IsEmpty() )
		{
			cssLine += L'<' ;
			cssLine += wstrDataType ;
			cssLine += L'>' ;
		}
		cssLine.MoveIndex( 0 ) ;
		//
		err = CompileDeclareDef( cssLine ) ;
		if ( err )
		{
			OutputError
				( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
		}
		return	true ;
	}
	//
	// data 定義
	//
	ECSCStyleStream	cssLine ;
	cssLine += wstrTagName ;
	//
	if ( !wstrDataType.IsEmpty() )
	{
		cssLine += L'<' ;
		cssLine += wstrDataType ;
		cssLine += L'>' ;
	}
	cssLine.MoveIndex( 0 ) ;
	//
	err = CompileData( cssLine ) ;
	if ( err )
	{
		OutputError
			( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
	}

	//
	// data 要素定義
	//
	for ( ; ; )
	{
		if ( cssSrc.DisregardSpace() )
		{
			OutputError
				( "\'{\' に対応する \'}\' が見つかりませんでした",
									m_strFilePath, m_nLineNum ) ;
		}
		m_strFilePath = cssSrc.m_strFilePath ;
		m_nLineNum = cssSrc.m_nLine ;
		//
		if ( cssSrc.HasToComeChar( L"}" ) == L'}' )
		{
			break ;
		}
		wchar_t	wchNext =
			ParseEnclosedExpression( cssSrc, L",}", &cssLine ) ;
		//
		cssLine.MoveIndex( 0 ) ;
		//
		err = CompileConstant( cssLine ) ;
		if ( err )
		{
			OutputError
				( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
		}
		if ( wchNext == L'}' )
		{
			break ;
		}
		else if ( wchNext != L',' )
		{
			OutputError
				( "列挙子が \',\' で区切られていません",
								m_strFilePath, m_nLineNum ) ;
		}
		cssSrc.FlushLine() ;
	}
	//
	// data ブロック終端処理
	//
	cssLine = L"" ;
	cssLine.MoveIndex( 0 ) ;
	err = CompileEndData( cssLine ) ;
	if ( err )
	{
		OutputError
			( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
	}
	return	true ;
}

// 指定のいずれかの文字を発見するまで構文を読み進める
//////////////////////////////////////////////////////////////////////////////
wchar_t ECSCStyleCompiler::ParseEnclosedExpression
	( ECSCStyleFrontStream & cssSrc,
		const wchar_t * pwszCloses, EWideString * pwstrExpr )
{
	wchar_t	wchClose = L'\0' ;
	int		iFirst = cssSrc.GetIndex() ;
	int		iEnd = iFirst ;
	while ( !cssSrc.DisregardSpace() )
	{
		iEnd = cssSrc.GetIndex() ;
		wchClose = cssSrc.HasToComeChar( pwszCloses ) ;
		if ( wchClose != L'\0' )
		{
			break ;
		}
		cssSrc.PassAExpressionTerm( m_dwModeFlags ) ;
	}
	if ( pwstrExpr != NULL )
	{
		*pwstrExpr = cssSrc.Middle( iFirst, iEnd - iFirst ) ;
	}
	return	wchClose ;
}

bool ECSCStyleCompiler::ParseEnclosedExpressionByToken
	( ECSCStyleFrontStream & cssSrc,
		const wchar_t * pwszClose, EWideString * pwstrExpr )
{
	wchar_t	wchClose = L'\0' ;
	int		iFirst = cssSrc.GetIndex() ;
	int		iEnd = iFirst ;
	bool	fResult = false ;
	while ( !cssSrc.DisregardSpace() )
	{
		if ( cssSrc.HasToComeToken( pwszClose ) )
		{
			fResult = true ;
			break ;
		}
		cssSrc.PassAExpressionTerm( m_dwModeFlags ) ;
		iEnd = cssSrc.GetIndex() ;
	}
	if ( pwstrExpr != NULL )
	{
		*pwstrExpr = cssSrc.Middle( iFirst, iEnd - iFirst ) ;
	}
	return	fResult ;
}

// C スタイルモード設定
//////////////////////////////////////////////////////////////////////////////
void ECSCStyleCompiler::SetCStyleMode( bool fCStyleMode, bool fCCompatible )
{
	if ( fCStyleMode )
	{
		if ( !m_flagCStyle )
		{
			const DWORD	dwCStyleSillFlags =
						flagCompatibleInt32
							| flagDefualtNakedFunc | flagDefaultNakedAll ;
			//
			m_flagCStyle = true ;
			m_dwBasicModeFlags = m_dwModeFlags ;
			m_dwModeFlags =
				flagCStyleNumberLiteral
					| flagQuoteCharactorCode
					| flagStrictStyle
					| flagCStyleCast
					| flagCStyleBasicType
					| flagNoWarningEquMove
					| flagSmallReservedWord
					| flagNoDefaultVirtual
					| flagDefaultNakedNew
					| (m_dwModeFlags & dwCStyleSillFlags) ;
			if ( fCCompatible )
			{
				m_dwModeFlags |=
					flagCompatibleInt32
						| flagDefualtNakedFunc | flagDefaultNakedAll ;
			}
		}
		else
		{
			if ( fCCompatible )
			{
				m_dwModeFlags |=
					flagCompatibleInt32
						| flagDefualtNakedFunc | flagDefaultNakedAll ;
				if ( m_nestCtrl.GetSize() == 0 )
				{
					m_modeNakedCode = true ;
				}
			}
			else
			{
				m_dwModeFlags &= ~flagDefaultNakedAll ;
			}
		}
	}
	else
	{
		if ( m_flagCStyle )
		{
			m_flagCStyle = false ;
			m_dwModeFlags = m_dwBasicModeFlags ;
		}
	}
}

// 現在有効な #if ブロック内か？
//////////////////////////////////////////////////////////////////////////////
bool ECSCStyleCompiler::IsInEnabledDirectiveBlock( void ) const
{
	unsigned int	nCount = m_nestSource.GetSize() ;
	for ( unsigned int i = 0; i < nCount; i ++ )
	{
		ESourceStreamNest *	pNest = m_nestSource.GetAt( i ) ;
		if ( (pNest != NULL) && !pNest->m_fEnabled )
		{
			return	false ;
		}
	}
	return	true ;
}

// 空白文字を読み飛ばす
//////////////////////////////////////////////////////////////////////////////
int ECSCStyleCompiler::SeekNextSourceStream( ECSCStyleSourceStream * pcsss )
{
	if ( !m_flagCStyle )
	{
		return	pcsss->ECSSourceStream::DisregardSpace() ;
	}
	ECSCStyleSourceStream *	pcsssCur = GetCurrentSourceStream() ;
	if ( pcsssCur != pcsss )
	{
		return	SeekNextCStyleSource( *pcsss ) ;
	}
	while ( pcsss != NULL )
	{
		//
		// 行頭判定
		//
		const wchar_t *	pwszText = *pcsss ;
		int				nIndex = pcsss->GetIndex() ;
		bool			fLineFirst = (nIndex == 0) ;
		wchar_t			wch ;
		if ( !fLineFirst && (pwszText != NULL) )
		{
			for ( int i = nIndex - 1; i >= 0; i -- )
			{
				if ( pwszText[i] == '\n' )
				{
					fLineFirst = true ;
					break ;
				}
				else if ( pwszText[i] > L' ' )
				{
					break ;
				}
			}
		}
		//
		// #if ディレクティブ状態判定
		//
		while ( !IsInEnabledDirectiveBlock() )
		{
			for ( ; ; )
			{
				if ( pcsss->IsIndexOverflow() )
				{
					OutputError
						( "#if が閉じられないまま EOF を検出しました",
								pcsss->m_strFilePath, pcsss->m_nLine ) ;
					break ;
				}
				wch = pcsss->GetCharacter() ;
				if ( wch == L'#' )
				{
					ECSCStyleStream	cssLine ;
					int	nFirstLineNum = pcsss->m_nLine ;
					GetNextCStyleDirectiveLine( cssLine, *pcsss ) ;
					//
					EWideString	wstrToken = cssLine.GetAToken() ;
					CStyleDirective	csd = IsDirectiveWord( wstrToken ) ;
					ESLError	err = eslErrSuccess ;
					switch ( csd )
					{
					case	csdIf:
						err = CompileDirectiveIf( cssLine ) ;
						break ;
					case	csdIfdef:
						err = CompileDirectiveIfDef( cssLine ) ;
						break ;
					case	csdIfndef:
						err = CompileDirectiveIfNDef( cssLine ) ;
						break ;
					case	csdElseif:
						err = CompileDirectiveElseIf( cssLine ) ;
						break ;
					case	csdElse:
						err = CompileDirectiveElse( cssLine ) ;
						break ;
					case	csdEndif:
						err = CompileDirectiveEndIf( cssLine ) ;
						break ;
					}
					if ( err )
					{
						OutputError
							( GetESLErrorMsg(err),
								pcsss->m_strFilePath, nFirstLineNum ) ;
					}
					fLineFirst = true ;
					break ;
				}
				else if ( wch == L'\n' )
				{
					pcsss->m_nLine ++ ;
					fLineFirst = true ;
				}
				else if ( wch <= L' ' )
				{
				}
				else
				{
					pcsss->MoveToNextLine() ;
					pcsss->m_nLine ++ ;
					fLineFirst = true ;
				}
			}
		}
		//
		// 空白・コメントアウト・ディレクティブ判定
		//
		int	nResult = 1 ;
		while ( !pcsss->IsIndexOverflow() )
		{
			nIndex = pcsss->GetIndex() ;
			wch = pcsss->CurrentCharacter() ;
			if ( fLineFirst && (wch == L'#') )
			{
				ECSCStyleStream	cssLine ;
				int	nFirstLineNum = pcsss->m_nLine ;
				pcsss->GetCharacter() ;
				GetNextCStyleDirectiveLine( cssLine, *pcsss ) ;
				//
				ESLError	err = CompileDirectiveLine( cssLine ) ;
				if ( err )
				{
					OutputError
						( GetESLErrorMsg(err),
							pcsss->m_strFilePath, nFirstLineNum ) ;
				}
				fLineFirst = true ;
				nResult = 0 ;
				break ;
			}
			else if ( wch == L'/' )
			{
				//
				// コメントアウト判定
				//
				pcsss->GetCharacter() ;
				wch = pcsss->GetCharacter() ;
				if ( wch == L'/' )
				{
					pcsss->MoveToNextLine() ;
					pcsss->m_nLine ++ ;
					fLineFirst = true ;
				}
				else if ( wch == L'*' )
				{
					for ( ; ; )
					{
						if ( pcsss->IsIndexOverflow() )
						{
							OutputError
								( "/* に対応する */ が見つかりません",
									pcsss->m_strFilePath, pcsss->m_nLine ) ;
							break ;
						}
						wch = pcsss->GetCharacter() ;
						if ( wch == L'*' )
						{
							if ( pcsss->CurrentCharacter() == L'/' )
							{
								pcsss->GetCharacter() ;
								break ;
							}
						}
						else if ( wch == L'\n' )
						{
							pcsss->m_nLine ++ ;
							fLineFirst = true ;
						}
					}
				}
				else
				{
					pcsss->MoveIndex( nIndex ) ;
					return	0 ;
				}
			}
			else if ( wch == L'\n' )
			{
				pcsss->m_nLine ++ ;
				fLineFirst = true ;
				pcsss->GetCharacter() ;
			}
			else if ( wch > L' ' )
			{
				return	0 ;
			}
			else
			{
				pcsss->GetCharacter() ;
			}
		}
		//
		// インクルード元
		//
		if ( nResult )
		{
			if ( m_lstSource.GetSize() <= 1 )
			{
				return	1 ;
			}
			ECSCStyleSourceStream *	pcsssTemp = m_lstSource.Pop() ;
			if ( pcsssTemp != NULL )
			{
				if ( pcsssTemp->m_fReturnMode )
				{
					SetCStyleMode
						( pcsssTemp->m_fReturnCStyle,
							pcsssTemp->m_fReturnCCompatible ) ;
				}
				pcsssTemp->ReleaseRef() ;
			}
		}
		pcsss = GetCurrentSourceStream() ;
	}
	return	1 ;
}

// 空白文字とコメントを読み飛ばす
//////////////////////////////////////////////////////////////////////////////
int ECSCStyleCompiler::SeekNextCStyleSource( ECSSourceStream & cssSource )
{
	while ( !cssSource.IsIndexOverflow() )
	{
		int		nIndex = cssSource.GetIndex() ;
		wchar_t	wch = cssSource.CurrentCharacter() ;
		if ( wch > L' ' )
		{
			return	0 ;
		}
		else if ( wch == L'/' )
		{
			//
			// コメントアウト判定
			//
			cssSource.GetCharacter() ;
			wch = cssSource.GetCharacter() ;
			if ( wch == L'/' )
			{
				cssSource.MoveToNextLine() ;
			}
			else if ( wch == L'*' )
			{
				for ( ; ; )
				{
					if ( cssSource.IsIndexOverflow() )
					{
						return	1 ;
					}
					wch = cssSource.GetCharacter() ;
					if ( wch == L'*' )
					{
						if ( cssSource.CurrentCharacter() == L'/' )
						{
							cssSource.GetCharacter() ;
							break ;
						}
					}
				}
			}
			else
			{
				cssSource.MoveIndex( nIndex ) ;
				return	0 ;
			}
		}
		cssSource.GetCharacter() ;
	}
	return	1 ;
}

// １行取得
//////////////////////////////////////////////////////////////////////////////
void ECSCStyleCompiler::GetNextCStyleDirectiveLine
	( ECSSourceStream & cssLine, ECSCStyleSourceStream & cssScript )
{
	while ( !cssScript.IsIndexOverflow() )
	{
		int	iLineFirst = cssScript.GetIndex( ) ;
		cssScript.MoveToNextLine( ) ;
		cssScript.m_nLine ++ ;
		//
		cssLine += cssScript.Middle
			( iLineFirst, cssScript.GetIndex() - iLineFirst ) ;
		//
		cssLine.TrimRight( ) ;
		int		nLength = cssLine.GetLength( ) ;
		if ( nLength < 1 )
		{
			break ;
		}
		wchar_t	wchLast = cssLine.GetAt(nLength - 1) ;
		if ( wchLast == L'\\' )
		{
			cssLine = cssLine.Left( nLength - 1 ) ;
		}
		else
		{
			break ;
		}
	}
	cssLine.MoveIndex( 0 ) ;
}

// ソースストリームから１節をプリプロセスして追加
//////////////////////////////////////////////////////////////////////////////
void ECSCStyleCompiler::AddNextFilteredSourceTerm
	( ECSSourceStream & cssDst, ECSSourceStream & cssSrc )
{
	int	nIndex = cssSrc.GetIndex() ;
	if ( cssSrc.DisregardSpace() )
	{
		return ;
	}
	if ( nIndex != (int) cssSrc.GetIndex() )
	{
		cssSrc += L' ' ;
	}
	wchar_t	wch = cssSrc.CurrentCharacter() ;
	if ( (wch == L'\"') || (wch == L'\'') )
	{
		//
		// 文字列リテラル
		//
		cssSrc.GetCharacter() ;
		cssDst += wch ;
		cssDst += cssSrc.GetEnclosedString( wch, m_dwModeFlags ) ;
		if ( cssSrc.GetAt( cssSrc.GetIndex() - 1 ) == wch )
		{
			cssDst += wch ;
		}
	}
	else
	{
		EWideString	wstrToken = cssSrc.GetAToken() ;
		if ( wstrToken == L"delete" )
		{
			//
			// delete -> deselect
			//
			cssDst += L"deselect" ;
		}
		else if ( wstrToken == L"defined" )
		{
			//
			// defined(identity) -> @isdefined("identity")
			//
			if ( cssSrc.HasToComeChar( L"(" ) == L'(' )
			{
				EWideString	wstrIdentity = cssSrc.GetAToken() ;
				EDescription::EncodeTextCEscSequence( wstrIdentity ) ;
				cssDst += L"@isdefined(\"" + wstrIdentity + L"\"" ;
			}
			else
			{
				cssDst += wstrToken ;
			}
		}
		else
		{
			EMacroBlock *	pmbMacro = m_staMacro.GetAs( wstrToken ) ;
			if ( (pmbMacro != NULL) && pmbMacro->m_fMacroFunc )
			{
				do
				{
					//
					// マクロ関数
					//
					EObjArray<EWideString>	lstParam ;
					EObjArray<ECSObject>	lstArg ;
					ESLError				err ;
					cssSrc.DisregardSpace() ;
					if ( (pmbMacro->m_lstArgName.GetSize() != 0)
								&& (cssSrc.CurrentCharacter() == L'(') )
					{
						err = CompileMacroArgument( lstParam, cssSrc ) ;
						if ( err )
						{
							OutputError
								( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
							break ;
						}
						int	i, nCount ;
						nCount = lstParam.GetSize() ;
						for ( i = 0; i < nCount; i ++ )
						{
							lstArg.Add( new ECSString( lstParam[i] ) ) ;
						}
					}
					ECSObject *	pObj = NULL ;
					err = CompileUserMacro( pmbMacro, lstArg, &pObj ) ;
					if ( err )
					{
						OutputError
							( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
						break ;
					}
					if ( pObj != NULL )
					{
						EWideString	wstrTemp ;
						switch ( pObj->m_vtType )
						{
						case	csvtString:
							{
								ECSCStyleStream	cssSrc ;
								cssSrc = ((ECSString*)pObj)->m_varStr ;
								cssSrc.MoveIndex( 0 ) ;
								FilterTextPreprocessor( cssSrc ) ;
								cssDst += cssSrc ;
							}
							break ;
						case	csvtInteger:
							wstrTemp.FromInteger( ((ECSInteger*)pObj)->GetValue() ) ;
							cssDst += wstrTemp ;
							break ;
						case	csvtReal:
							cssDst += EWideString( ((ECSReal*)pObj)->m_varReal ) ;
							break ;
						}
						delete	pObj ;
					}
				}
				while ( false ) ;
			}
			else
			{
				cssDst += wstrToken ;
			}
		}
	}
}

// ソースストリームからプリプロセス済みの次の1行を取得して追加
//////////////////////////////////////////////////////////////////////////////
bool ECSCStyleCompiler::AddNextFilteredSourceLine( ECSSourceStream & cssDst )
{
	ECSCStyleSourceStream *	pcsss = GetCurrentSourceStream() ;
	if ( pcsss == NULL )
	{
		return	false ;
	}
	if ( pcsss->IsIndexOverflow() )
	{
		return	(SeekNextSourceStream( pcsss ) == 0) ;
	}
	int	nFirstLine = pcsss->m_nLine ;
	//
	pcsss->AddRef() ;
	//
	while ( !pcsss->IsIndexOverflow() && IsInCStyleMode() )
	{
		int		nIndex = pcsss->GetIndex() ;
		SeekNextCStyleSource( *pcsss ) ;
		int		nNewIndex = pcsss->GetIndex() ;
		bool	fNextLine = false ;
		for ( int i = nIndex; i < nNewIndex; i ++ )
		{
			if ( pcsss->GetAt( i ) == L'\n' )
			{
				pcsss->m_nLine ++ ;
				fNextLine = true ;
			}
		}
		if ( !fNextLine )
		{
			if ( pcsss->DisregardSpace() )
			{
				break ;
			}
			if ( !IsInCStyleMode() )
			{
				break ;
			}
		}
		if ( nFirstLine != pcsss->m_nLine )
		{
			cssDst += L'\n' ;
			break ;
		}
		if ( nIndex != (int) pcsss->GetIndex() )
		{
			cssDst += L' ' ;
		}
		AddNextFilteredSourceTerm( cssDst, *pcsss ) ;
	}
	//
	pcsss->ReleaseRef() ;
	//
	return	true ;
}

// #define テキストマクロプリプロセッサ
//////////////////////////////////////////////////////////////////////////////
void ECSCStyleCompiler::FilterTextPreprocessor( ECSSourceStream & cssSource )
{
	ECSCStyleStream cssDst ;
	int				iFirstIndex = cssSource.GetIndex() ;
	cssDst = cssSource.Left( iFirstIndex ) ;
	//
	while ( !cssSource.IsIndexOverflow() )
	{
		int	nIndex = cssSource.GetIndex() ;
		if ( cssSource.DisregardSpace() )
		{
			break ;
		}
		if ( nIndex != (int) cssSource.GetIndex() )
		{
			cssDst += L' ' ;
		}
		AddNextFilteredSourceTerm( cssDst, cssSource ) ;
	}
	//
	cssSource = cssDst ;
	cssSource.MoveIndex( iFirstIndex ) ;
}

// 定数値（マクロ変数）取得
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSCStyleCompiler::GetMacroVariable( const wchar_t * pwszName )
{
	if ( m_flagCStyle )
	{
		static const wchar_t *	pwszReplace[] =
		{
			L"__LINE__", L"__FILE__", NULL
		} ;
		static const wchar_t *	pwszReplaced[] =
		{
			L"@linenum", L"@filename", NULL
		} ;
		for ( int i = 0; pwszReplace[i] != NULL; i ++ )
		{
			if ( !EWideString::Compare( pwszName, pwszReplace[i] ) )
			{
				pwszName = pwszReplaced[i] ;
				break ;
			}
		}
		if ( !EWideString::Compare( pwszName, L"__COTOPHA__" ) )
		{
			return	&m_csintCotopha ;
		}
	}
	return	ECSCompiler::GetMacroVariable( pwszName ) ;
}

// 型情報を取得
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSCStyleCompiler::ParseBsaicType( const wchar_t * pwszName ) const
{
	if ( m_flagCStyle )
	{
		static const wchar_t *	pwszReplace[] =
		{
			/*L"int",*/ L"bool", NULL
		} ;
		static const wchar_t *	pwszReplaced[] =
		{
			/*L"Integer",*/ L"boolean", NULL
		} ;
		for ( int i = 0; pwszReplace[i] != NULL; i ++ )
		{
			if ( !EWideString::Compare( pwszName, pwszReplace[i] ) )
			{
				pwszName = pwszReplaced[i] ;
				break ;
			}
		}
	}
	return	ECSCompiler::ParseBsaicType( pwszName ) ;
}

// 型情報検索
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::SearchTypeName
	( SYMBOL_NAMESPACE& snsSymbol, ECSTypeInfo::Flags flagScope )
{
	ESLError	err = ECSCompiler::SearchTypeName( snsSymbol, flagScope ) ;
	if ( err && snsSymbol.wstrNamespace.IsEmpty() )
	{
		static const wchar_t *	pwszReplace[] =
		{
			/*L"int",*/ L"bool", NULL
		} ;
		static const wchar_t *	pwszReplaced[] =
		{
			/*L"Integer",*/ L"boolean", NULL
		} ;
		for ( int i = 0; pwszReplace[i] != NULL; i ++ )
		{
			if ( snsSymbol.wstrName == pwszReplace[i] )
			{
				snsSymbol.wstrFullName = pwszReplaced[i] ;
				snsSymbol.wstrName = pwszReplaced[i] ;
				return	eslErrSuccess ;
			}
		}
	}
	return	err ;
}

// Cスタイル予約語判定
//////////////////////////////////////////////////////////////////////////////
ECSCStyleCompiler::CStyleReservedWord
	ECSCStyleCompiler::IsCStyleReservedWord( const wchar_t * pwszToken ) const
{
	static const wchar_t *	pwszReservedWords[] =
	{
		L"typedef", L"constant",
		L"data", L"enum", L"struct", L"class",
		L"union", L"namespace", L"asm",
		L"public", L"protected", L"private",
		L"if", L"else",
		L"break", L"continue", L"return", L"goto",
		L"try", L"catch", L"throw",
		L"for", L"while", L"do",
		L"switch", L"case", L"default",
		L"_m_fence", L"template", L"using", L"friend",
		NULL
	} ;
	static const CStyleReservedWord	csrwReservedWords[] =
	{
		csrwTypeDef, csrwConstant,
		csrwData, csrwEnum, csrwStruct, csrwClass,
		csrwUnion, csrwNamespace, csrwAsm,
		csrwPublic, csrwProtected, csrwPrivate,
		csrwIf, csrwElse,
		csrwBreak, csrwContinue, csrwReturn, csrwGoto,
		csrwTry, csrwCatch, csrwThrow,
		csrwFor, csrwWhile,	csrwDo,
		csrwSwitch, csrwCase, csrwDefault,
		csrwMemoryFence, csrwTemplate, csrwUsing, csrwFriend,
	} ;
	for ( int i = 0; pwszReservedWords[i] != NULL; i ++ )
	{
		if ( !EWideString::Compare( pwszReservedWords[i], pwszToken ) )
		{
			return	csrwReservedWords[i] ;
		}
	}
	return	csrwInvalid ;
}

// 予約語処理
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileCStyleReservedWord
	( ECSCStyleCompiler::CStyleReservedWord csrwWord, ECSCStyleFrontStream & cssLine )
{
	if ( csrwWord != csrwInvalid )
	{
		return	(this->*m_pfnCompileCStyle[csrwWord])( cssLine ) ;
	}
	return	ESLErrorMsg( "不正な予約語です" ) ;
}

// 予約語
//////////////////////////////////////////////////////////////////////////////
const ECSCStyleCompiler::PFN_COMPILE_RESERVED_WORD
	ECSCStyleCompiler::m_pfnCompileCStyle[ECSCStyleCompiler::csrwMax] =
{
	&ECSCStyleCompiler::CompileCStyleTypedef,
	&ECSCStyleCompiler::CompileCStyleConstant,
	&ECSCStyleCompiler::CompileCStyleData,
	&ECSCStyleCompiler::CompileCStyleEnum,
	&ECSCStyleCompiler::CompileCStyleStruct,
	&ECSCStyleCompiler::CompileCStyleUnion,
	&ECSCStyleCompiler::CompileCStyleClass,
	&ECSCStyleCompiler::CompileCStyleNamespace,
	&ECSCStyleCompiler::CompileCStyleAsm,
	&ECSCStyleCompiler::CompileCStylePublic,
	&ECSCStyleCompiler::CompileCStyleProtected,
	&ECSCStyleCompiler::CompileCStylePrivate,
	&ECSCStyleCompiler::CompileCStyleIf,
	&ECSCStyleCompiler::CompileCStyleElse,
	&ECSCStyleCompiler::CompileCStyleBreak,
	&ECSCStyleCompiler::CompileCStyleContinue,
	&ECSCStyleCompiler::CompileCStyleReturn,
	&ECSCStyleCompiler::CompileCStyleGoto,
	&ECSCStyleCompiler::CompileCStyleTry,
	&ECSCStyleCompiler::CompileCStyleCatch,
	&ECSCStyleCompiler::CompileCStyleThrow,
	&ECSCStyleCompiler::CompileCStyleFor,
	&ECSCStyleCompiler::CompileCStyleWhile,
	&ECSCStyleCompiler::CompileCStyleDo,
	&ECSCStyleCompiler::CompileCStyleSwitch,
	&ECSCStyleCompiler::CompileCStyleCase,
	&ECSCStyleCompiler::CompileCStyleDefault,
	&ECSCStyleCompiler::CompileCStyleMemoryFence,
	&ECSCStyleCompiler::CompileCStyleTemplate,
	&ECSCStyleCompiler::CompileCStyleUsing,
	&ECSCStyleCompiler::CompileCStyleFriend,
} ;

// typedef 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileCStyleTypedef( ECSCStyleFrontStream & cssSrc )
{
	EWideString	wstrTypeExpr ;
	if ( !ParseCStyleTypeDescription( wstrTypeExpr, cssSrc, false ) )
	{
		return	ESLErrorMsg( "typedef 文に型が指定されていません" ) ;
	}
	EWideString	wstrTypeName = cssSrc.GetAToken() ;
	if ( wstrTypeName == L"(" )
	{
		//
		// 関数ポインタ型の解釈
		//
		wstrTypeExpr += L" (" ;
		wstrTypeName = L"" ;
		for ( ; ; )
		{
			EWideString	wstrToken = cssSrc.GetAToken() ;
			if ( (wstrToken == L")")
				|| (wstrToken == L";") || wstrToken.IsEmpty() )
			{
				break ;
			}
			wstrTypeExpr += wstrTypeName ;
			wstrTypeExpr += L' ' ;
			wstrTypeName = wstrToken ;
		}
		wstrTypeExpr += L")" ;
		//
		if ( cssSrc.HasToComeChar( L"(" ) != L'(' )
		{
			return	ESLErrorMsg( "関数ポインタの引数指定がありません" ) ;
		}
		wstrTypeExpr += L"(" ;
		//
		EWideString	wstrArgList ;
		wchar_t	wchClose =
			ParseEnclosedExpression( cssSrc, L")", &wstrArgList ) ;
		if ( wchClose != L')' )
		{
			return	ESLErrorMsg
				( "関数ポインタの引数が \')\' で閉じられていません" ) ;
		}
		wstrTypeExpr += wstrArgList ;
		wstrTypeExpr += L")" ;
		//
		for ( ; ; )
		{
			if ( cssSrc.HasToComeToken( L"naked" ) )
			{
				wstrTypeExpr += L" naked" ;
			}
			else if ( cssSrc.HasToComeToken( L"native" ) )
			{
				wstrTypeExpr += L" native" ;
			}
			else if ( cssSrc.HasToComeToken( L"const" ) )
			{
				wstrTypeExpr += L" const" ;
			}
			else
			{
				break ;
			}
		}
	}
	//
	// 詞葉標準構文へ変換
	//
	if ( wstrTypeName == L";" )
	{
		return	ESLErrorMsg( "typedef 文に定義名が指定されていません" ) ;
	}
	//
	ECSCStyleStream	cssLine ;
	cssLine += wstrTypeName ;
	cssLine += L" := " ;
	cssLine += wstrTypeExpr ;
	//
	ESLError	err = ECSCompiler::CompileTypeDef( cssLine ) ;
	if ( !err )
	{
		if ( cssSrc.HasToComeChar( L";" ) != L';' )
		{
			err = ESLErrorMsg( "typedef に \';\' が見つかりません" ) ;
		}
	}
	return	err ;
}

// constant 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileCStyleConstant( ECSCStyleFrontStream & cssSrc )
{
	EWideString	wstrList ;
	wchar_t	wchClose = ParseEnclosedExpression( cssSrc, L";", &wstrList ) ;
	//
	ECSCStyleStream	cssLine ;
	cssLine = wstrList ;
	cssLine.MoveIndex( 0 ) ;
	//
	return	ECSCompiler::CompileConstant( cssLine ) ;
}

// data 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileCStyleData( ECSCStyleFrontStream & cssSrc )
{
	EWideString	wstrTagName ;
	if ( !ParseDataDeclaration( wstrTagName, cssSrc ) )
	{
		return	ESLErrorMsg( "data 文にタグ名が指定されていません" ) ;
	}
	if ( cssSrc.HasToComeChar( L";" ) != L';' )
	{
		return	ESLErrorMsg( "data 文に \';\' が見つかりません" ) ;
	}
	return	eslErrSuccess ;
}

// enum 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileCStyleEnum( ECSCStyleFrontStream & cssSrc )
{
	EWideString	wstrTagName ;
	if ( !ParseEnumDeclaration( wstrTagName, cssSrc ) )
	{
		return	ESLErrorMsg( "enum 文にタグ名が指定されていません" ) ;
	}
	return	CompileVarFuncDeclaration( 0, wstrTagName, cssSrc ) ;
}

// struct 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileCStyleStruct( ECSCStyleFrontStream & cssSrc )
{
	EWideString	wstrTagName ;
	if ( !ParseClassDeclaration( wstrTagName, cssSrc, true ) )
	{
		return	ESLErrorMsg( "struct 文にタグ名が指定されていません" ) ;
	}
	return	CompileVarFuncDeclaration( 0, wstrTagName, cssSrc ) ;
}

// union 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileCStyleUnion( ECSCStyleFrontStream & cssSrc )
{
	EWideString	wstrTagName ;
	if ( !ParseUnionDeclaration( wstrTagName, cssSrc ) )
	{
		return	ESLErrorMsg( "struct 文にタグ名が指定されていません" ) ;
	}
	return	CompileVarFuncDeclaration( 0, wstrTagName, cssSrc ) ;
}

// class 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileCStyleClass( ECSCStyleFrontStream & cssSrc )
{
	EWideString	wstrTagName ;
	if ( !ParseClassDeclaration( wstrTagName, cssSrc, false ) )
	{
		return	ESLErrorMsg( "class 文にタグ名が指定されていません" ) ;
	}
	return	CompileVarFuncDeclaration( 0, wstrTagName, cssSrc ) ;
}

// namespace 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileCStyleNamespace( ECSCStyleFrontStream & cssSrc )
{
	//
	// namespace 名取得
	//
	EWideString	wstrTagName = cssSrc.GetAToken() ;
	ESLError	err = VerifyUserSymbol( wstrTagName ) ;
	if ( err )
	{
		return	err ;
	}
	wchar_t	wchNext = cssSrc.HasToComeChar( L"{" ) ;
	if ( wchNext != L'{' )
	{
		return	ESLErrorMsg( "namespace に \'{\' が見つかりません" ) ;
	}
	ECSSourceStream	cssLine ;
	cssLine = wstrTagName ;
	cssLine.MoveIndex( 0 ) ;
	err = CompileNamespace( cssLine ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// クラス実体宣言
	//
	err = CompileCStyleMultiStatement( cssSrc ) ;
	if ( err )
	{
		OutputError
			( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
	}
	//
	// namespace ブロック終端処理
	//
	cssLine = L"" ;
	cssLine.MoveIndex( 0 ) ;
	err = CompileEndNamespace( cssLine ) ;
	if ( err )
	{
		return	err ;
	}
	/*
	if ( cssSrc.HasToComeChar( L";" ) != L';' )
	{
		return	ESLErrorMsg( "namespace の終端に \';\' が見つかりません" ) ;
	}
	*/
	return	eslErrSuccess ;
}

// asm
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileCStyleAsm( ECSCStyleFrontStream & cssSrc )
{
	ESLError	err ;
	ECSCStyleStream	cssLine ;
	err = ECSCompiler::CompileAssembler( cssLine ) ;
	if ( err )
	{
		return	err ;
	}
	if ( cssSrc.HasToComeChar( L"{" ) == L'{' )
	{
		for ( ; ; )
		{
			if ( cssSrc.DisregardSpace() )
			{
				err = ESLErrorMsg( "asm ブロックが閉じられていません" ) ;
				break ;
			}
			m_strFilePath = cssSrc.m_strFilePath ;
			m_nLineNum = cssSrc.m_nLine ;
			//
			const int	iLineFirst = cssSrc.GetIndex() ;
			cssSrc.MoveToNextLine() ;
			const int	iNextLine = cssSrc.GetIndex() ;
			cssSrc.MoveIndex( iLineFirst ) ;
			//
			int		iLast = iLineFirst ;
			while ( (int) cssSrc.GetIndex() < iNextLine )
			{
				wchar_t	wch = cssSrc.CurrentCharacter() ;
				if ( (wch == L';') | (wch == L'}')
					| (wch == L'\n') | (wch == L'\r') )
				{
					break ;
				}
				cssSrc.PassAToken() ;
				iLast = cssSrc.GetIndex() ;
				cssSrc.DisregardSpace() ;
			}
			cssLine = cssSrc.Middle( iLineFirst, iLast - iLineFirst ) ;
			cssLine.MoveIndex( 0 ) ;
			err = ECSCompiler::CompileAssembleLine1( cssLine ) ;
			if ( err )
			{
				OutputError
					( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
			}
			wchar_t	wch = cssSrc.HasToComeChar( L";}" ) ;
			if ( wch == L'}' )
			{
				break ;
			}
		}
	}
	else
	{
		EWideString		wstrAsm ;
		if ( !ParseEnclosedExpressionByToken( cssSrc, L";", &wstrAsm ) )
		{
			OutputError
				( "asm 文末に \';\' が見つかりません",
								m_strFilePath, m_nLineNum ) ;
		}
		cssLine = wstrAsm ;
		cssLine.MoveIndex( 0 ) ;
		err = ECSCompiler::CompileAssembleLine1( cssLine ) ;
	}
	if ( err )
	{
		OutputError
			( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
	}
	cssLine = L"" ;
	cssLine.MoveIndex( 0 ) ;
	err = ECSCompiler::CompileEndAssembler( cssLine ) ;
	if ( err )
	{
		return	err ;
	}
	return	eslErrSuccess ;
}

// public
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileCStylePublic( ECSCStyleFrontStream & cssSrc )
{
	ECSCStyleStream	cssLine ;
	ESLError	err = ECSCompiler::CompilePublic( cssLine ) ;
	if ( !err )
	{
		if ( cssSrc.HasToComeChar( L":" ) != L':' )
		{
			err = ESLErrorMsg( "public に \':\' が見つかりません" ) ;
		}
	}
	return	err ;
}

// protected
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileCStyleProtected( ECSCStyleFrontStream & cssSrc )
{
	ECSCStyleStream	cssLine ;
	ESLError	err = ECSCompiler::CompileProtected( cssLine ) ;
	if ( !err )
	{
		if ( cssSrc.HasToComeChar( L":" ) != L':' )
		{
			err = ESLErrorMsg( "protected に \':\' が見つかりません" ) ;
		}
	}
	return	err ;
}

// private
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileCStylePrivate( ECSCStyleFrontStream & cssSrc )
{
	ECSCStyleStream	cssLine ;
	ESLError	err = ECSCompiler::CompilePrivate( cssLine ) ;
	if ( !err )
	{
		if ( cssSrc.HasToComeChar( L":" ) != L':' )
		{
			err = ESLErrorMsg( "private に \':\' が見つかりません" ) ;
		}
	}
	return	err ;
}

// if 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileCStyleIf( ECSCStyleFrontStream & cssSrc )
{
	//
	// if ブロック
	//
	if ( cssSrc.HasToComeChar( L"(" ) != L'(' )
	{
		return	ESLErrorMsg( "if 文に \'(\' が見つかりません" ) ;
	}
	ECSCStyleStream	cssLine ;
	ESLError		err ;
	cssLine = cssSrc.GetEnclosedString( L')', m_dwModeFlags ) ;
	cssLine.MoveIndex( 0 ) ;
	err = ECSCompiler::CompileIf( cssLine ) ;
	if ( err )
	{
		return	err ;
	}
	if ( cssSrc.GetAt( cssSrc.GetIndex() - 1 ) != L')' )
	{
		OutputError
			( "if 文で \')\' が見つかりません",
						m_strFilePath, m_nLineNum ) ;
	}
	err = CompileCStyleStatement( cssSrc, true ) ;
	if ( err )
	{
		OutputError
			( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
	}
	//
	// else ブロック判定
	//
	bool	fElse = false ;
	while ( cssSrc.HasToComeToken( L"else" ) )
	{
		if ( !fElse && cssSrc.HasToComeToken( L"if" ) )
		{
			if ( cssSrc.HasToComeChar( L"(" ) != L'(' )
			{
				return	ESLErrorMsg( "if 文に \'(\' が見つかりません" ) ;
			}
			ECSCStyleStream	cssLine ;
			ESLError		err ;
			cssLine = cssSrc.GetEnclosedString( L')', m_dwModeFlags ) ;
			cssLine.MoveIndex( 0 ) ;
			err = ECSCompiler::CompileElseIf( cssLine ) ;
			if ( err )
			{
				OutputError
					( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
			}
			else if ( cssSrc.GetAt( cssSrc.GetIndex() - 1 ) != L')' )
			{
				OutputError
					( "if 文で \')\' が見つかりません",
								m_strFilePath, m_nLineNum ) ;
			}
		}
		else
		{
			cssLine = L"" ;
			cssLine.MoveIndex( 0 ) ;
			err = ECSCompiler::CompileElse( cssLine ) ;
			if ( err )
			{
				OutputError
					( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
			}
			fElse = true ;
		}
		err = CompileCStyleStatement( cssSrc, true ) ;
		if ( err )
		{
			OutputError
				( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
		}
		if ( fElse )
		{
			break ;
		}
	}
	//
	// if ブロック完了
	//
	cssLine = L"" ;
	cssLine.MoveIndex( 0 ) ;
	return	ECSCompiler::CompileEndIf( cssLine ) ;
}

// else 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileCStyleElse( ECSCStyleFrontStream & cssSrc )
{
	return	ESLErrorMsg( "else に対応する if 文が見つかりません" ) ;
}

// break 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileCStyleBreak( ECSCStyleFrontStream & cssSrc )
{
	ECSCStyleStream	cssLine ;
	wchar_t	wchClose =
				ParseEnclosedExpression( cssSrc, L";}", &cssLine ) ;
	cssLine.MoveIndex( 0 ) ;
	//
	ESLError	err = ECSCompiler::CompileBreak( cssLine ) ;
	if ( !err )
	{
		if ( wchClose != L';' )
		{
			err = ESLErrorMsg( "break 文に \';\' が見つかりません" ) ;
		}
	}
	return	err ;
}

// continue 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileCStyleContinue( ECSCStyleFrontStream & cssSrc )
{
	ECSCStyleStream	cssLine ;
	wchar_t	wchClose =
				ParseEnclosedExpression( cssSrc, L";}", &cssLine ) ;
	cssLine.MoveIndex( 0 ) ;
	//
	ESLError	err = ECSCompiler::CompileContinue( cssLine ) ;
	if ( !err )
	{
		if ( wchClose != L';' )
		{
			err = ESLErrorMsg( "continue 文に \';\' が見つかりません" ) ;
		}
	}
	return	err ;
}

// return 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileCStyleReturn( ECSCStyleFrontStream & cssSrc )
{
	ECSCStyleStream	cssLine ;
	wchar_t	wchClose =
				ParseEnclosedExpression( cssSrc, L";}", &cssLine ) ;
	cssLine.MoveIndex( 0 ) ;
	//
	ESLError	err = ECSCompiler::CompileReturn( cssLine ) ;
	if ( !err )
	{
		if ( wchClose != L';' )
		{
			err = ESLErrorMsg( "return 文に \';\' が見つかりません" ) ;
		}
	}
	return	err ;
}

// goto 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileCStyleGoto( ECSCStyleFrontStream & cssSrc )
{
	ECSCStyleStream	cssLine ;
	wchar_t	wchClose =
				ParseEnclosedExpression( cssSrc, L";}", &cssLine ) ;
	cssLine.MoveIndex( 0 ) ;
	//
	ESLError	err = ECSCompiler::CompileGoto( cssLine ) ;
	if ( !err )
	{
		if ( wchClose != L';' )
		{
			err = ESLErrorMsg( "goto 文に \';\' が見つかりません" ) ;
		}
	}
	return	err ;
}

// try 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileCStyleTry( ECSCStyleFrontStream & cssSrc )
{
	//
	// try ブロック
	//
	ECSCStyleStream	cssLine ;
	ESLError	err = ECSCompiler::CompileTry( cssLine ) ;
	if ( err )
	{
		return	err ;
	}
	err = CompileCStyleStatement( cssSrc, true ) ;
	if ( err )
	{
		OutputError
			( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
	}
	//
	// catch ブロック
	//
	if ( m_modeNakedCode )
	{
		while ( cssSrc.HasToComeToken( L"catch" ) )
		{
			ECSCStyleSourceStream *	pcsss = GetCurrentSourceStream() ;
			if ( pcsss != NULL )
			{
				m_strFilePath = pcsss->m_strFilePath ;
				m_nLineNum = pcsss->m_nLine ;
			}
			if ( cssSrc.HasToComeChar( L"(" ) != L'(' )
			{
				OutputError
					( "catch 文に \'(\' が見つかりません",
								m_strFilePath, m_nLineNum ) ;
				break ;
			}
			EWideString	wstrCatch ;
			wchar_t	wchClose =
				ParseEnclosedExpression( cssSrc, L");{}", &wstrCatch ) ;
			if ( wchClose != L')' )
			{
				OutputError( "\'(\' に対応する \')\' が見つかりません" ) ;
				break ;
			}
			cssLine = L"( " + wstrCatch + L" )" ;
			cssLine.MoveIndex( 0 ) ;
			err = ECSCompiler::CompileCatch( cssLine ) ;
			if ( err )
			{
				OutputError
					( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
				break ;
			}
			err = CompileCStyleStatement( cssSrc, true ) ;
			if ( err )
			{
				OutputError
					( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
				break ;
			}
		}
	}
	else
	{
		cssLine = L"( Integer @err, String @msg, Reference @obj )" ;
		cssLine.MoveIndex( 0 ) ;
		err = ECSCompiler::CompileCatch( cssLine ) ;
		if ( err )
		{
			OutputError
				( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
		}
		else
		{
			cssLine = L"boolean @handled := false" ;
			cssLine.MoveIndex( 0 ) ;
			err = ECSCompiler::CompileVariable( cssLine ) ;
			if ( err )
			{
				OutputError
					( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
			}
			bool	fAllCatched = false ;
			while ( cssSrc.HasToComeToken( L"catch" ) )
			{
				//
				// catch 文引数解釈
				//
				ECSCStyleSourceStream *	pcsss = GetCurrentSourceStream() ;
				if ( pcsss != NULL )
				{
					m_strFilePath = pcsss->m_strFilePath ;
					m_nLineNum = pcsss->m_nLine ;
				}
				if ( cssSrc.HasToComeChar( L"(" ) != L'(' )
				{
					OutputError
						( "catch 文に \'(\' が見つかりません",
									m_strFilePath, m_nLineNum ) ;
					break ;
				}
				SYMBOL_NAMESPACE	snsTypeName = cssSrc.GetAToken() ;
				EWideString	wstrTypeName ;
				EWideString	wstrVarName ;
				if ( snsTypeName.wstrName != L"..." )
				{
					err = ParseFullNameSymbol( snsTypeName, cssSrc ) ;
					if ( err )
					{
						OutputError
							( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
						break ;
					}
					ECSTypeInfo	typeinf ;
					wstrTypeName = snsTypeName.wstrFullName ;
					if ( GetSimpleTypeInfoAs( typeinf, wstrTypeName ) )
					{
						OutputError
							( EString(wstrTypeName)
								+ " は有効な型名ではありません",
										m_strFilePath, m_nLineNum ) ;
						break ;
					}
					wstrVarName = cssSrc.GetAToken() ;
					err = VerifyUserSymbol( wstrVarName ) ;
					if ( err )
					{
						OutputError
							( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
						break ;
					}
				}
				if ( cssSrc.HasToComeChar( L")" ) != L')' )
				{
					OutputError
						( "catch 文に \')\' が見つかりません",
									m_strFilePath, m_nLineNum ) ;
				}
				//
				// catch ブロック生成
				//
				if ( !wstrVarName.IsEmpty() )
				{
					static const ReservedWord	rwCatchBlock[] =
					{
						rwIf, rwVariable, rwIf,
						rwInvalid, rwInvalid,
						rwEndIf, rwEndIf,
					} ;
					const int	nCatchBlockCount =
							sizeof(rwCatchBlock) / sizeof(rwCatchBlock[0]) ;
					//
					EWideString	wstrCatchBlockLine[nCatchBlockCount] ;
					wstrCatchBlockLine[0] = L"!@handled" ;
					wstrCatchBlockLine[1] =
						wstrTypeName + L"& " + wstrVarName
							+ L" := dynamic_cast<" + wstrTypeName + L">( @obj )" ;
					wstrCatchBlockLine[2] = wstrVarName + L" !== null" ;
					wstrCatchBlockLine[3] = L"@handled := true" ;
					//
					for ( int i = 0; i < sizeof(rwCatchBlock)/sizeof(rwCatchBlock[0]); i ++ )
					{
						if ( rwCatchBlock[i] != rwInvalid )
						{
							cssLine = wstrCatchBlockLine[i] ;
							cssLine.MoveIndex( 0 ) ;
							err = CompileReservedWord( rwCatchBlock[i], cssLine ) ;
						}
						else if ( !wstrCatchBlockLine[i].IsEmpty() )
						{
							cssLine = wstrCatchBlockLine[i] ;
							cssLine.MoveIndex( 0 ) ;
							err = CompileScriptLine
								( cssLine, m_nLineNum, m_strFilePath, false ) ;
						}
						else
						{
							err = CompileCStyleStatement( cssSrc, true ) ;
						}
						if ( err )
						{
							OutputError
								( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
							break ;
						}
					}
				}
				else
				{
					cssLine = L"!@handled" ;
					cssLine.MoveIndex( 0 ) ;
					err = CompileIf( cssLine ) ;
					if ( !err )
					{
						err = CompileCStyleStatement( cssSrc, true ) ;
						if ( err )
						{
							OutputError
								( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
						}
						cssLine = L"" ;
						cssLine.MoveIndex( 0 ) ;
						err = CompileEndIf( cssLine ) ;
					}
					if ( err )
					{
						OutputError
							( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
						break ;
					}
					fAllCatched = true ;
				}
			}
			//
			// ハンドルされない例外をスロー
			//
			if ( !fAllCatched )
			{
				cssLine = L"!@handled" ;
				cssLine.MoveIndex( 0 ) ;
				err = CompileIf( cssLine ) ;
				if ( !err )
				{
					cssLine = L"( @err, @msg, @obj )" ;
					cssLine.MoveIndex( 0 ) ;
					err = ECSCompiler::CompileThrow( cssLine ) ;
					//
					cssLine = L"" ;
					cssLine.MoveIndex( 0 ) ;
					CompileEndIf( cssLine ) ;
				}
				if ( err )
				{
					OutputError
						( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
				}
			}
		}
	}
	//
	// try ブロック終了
	//
	cssLine = L"" ;
	cssLine.MoveIndex( 0 ) ;
	return	ECSCompiler::CompileEndTry( cssLine ) ;
}

// catch 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileCStyleCatch( ECSCStyleFrontStream & cssSrc )
{
	return	ESLErrorMsg( "catch に対応する try 文が見つかりません" ) ;
}

// throw 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileCStyleThrow( ECSCStyleFrontStream & cssSrc )
{
	ECSCStyleStream	cssLine ;
	if ( m_modeNakedCode )
	{
		wchar_t	wchClose =
					ParseEnclosedExpression( cssSrc, L";}", &cssLine ) ;
		cssLine.MoveIndex( 0 ) ;
		//
		if ( wchClose != L';' )
		{
			OutputError
				( "throw 文に \';\' が見つかりません",
							m_strFilePath, m_nLineNum ) ;
		}
	}
	else
	{
		cssLine = L"( 0x80000000, \"C style exception\"" ;
		if ( cssSrc.HasToComeChar( L";" ) != L';' )
		{
			cssLine += L", " ;
			cssLine += cssSrc.GetEnclosedString( L';', m_dwModeFlags ) ;
		}
		cssLine += L')' ;
	}
	cssLine.MoveIndex( 0 ) ;
	//
	return	ECSCompiler::CompileThrow( cssLine ) ;
}

// for 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileCStyleFor( ECSCStyleFrontStream & cssSrc )
{
	if ( cssSrc.HasToComeChar( L"(" ) != L'(' )
	{
		return	ESLErrorMsg( "for 文に \'(\' が見つかりません" ) ;
	}
	ECSCStyleStream	cssLine ;
	int				iLineFirst = cssSrc.GetIndex() ;
	EWideString		wstrIter = cssSrc.GetAToken() ;
	ESLError		err ;
	wchar_t			wchNext ;
	bool			fBeginBlock = false ;
	if ( cssSrc.HasToComeChar( L":" ) == L':' )
	{
		//
		// for ( iter : expr : index )
		//
		cssLine += wstrIter ;
		cssLine += L" | " ;
		//
		EWideString	wstrArray ;
		wchNext = ParseEnclosedExpression( cssSrc, L":)", &wstrArray ) ;
		cssLine += wstrArray ;
		//
		if ( wchNext == L':' )
		{
			EWideString	wstrIndex ;
			wchNext = ParseEnclosedExpression( cssSrc, L")", &wstrIndex ) ;
			//
			cssLine += L'[' ;
			cssLine += wstrIndex ;
			cssLine += L']' ;
		}
		else
		{
			cssLine += L"[@iter]" ;
		}
		if ( wchNext != L')' )
		{
			OutputError
				( "for 文に \')\' が見つかりません",
							m_strFilePath, m_nLineNum ) ;
		}
	}
	else
	{
		//
		// for ( decl expr; expr )
		//
		cssSrc.MoveIndex( iLineFirst ) ;
		if ( cssSrc.HasToComeChar( L";" ) != L';' )
		{
			err = ECSCompiler::CompileBegin( cssLine ) ;
			if ( err )
			{
				return	err ;
			}
			fBeginBlock = true ;
			//
			err = CompileCStyleDeclOrExprStatement( cssSrc ) ;
			if ( err )
			{
				if ( fBeginBlock )
				{
					cssLine = L"" ;
					cssLine.MoveIndex( 0 ) ;
					return	ECSCompiler::CompileEnd( cssLine ) ;
				}
				return	err ;
			}
		}
		EWideString	wstrExpr ;
		EWideString	wstrBy ;
		if ( cssSrc.HasToComeChar( L";" ) != L';' )
		{
			wchNext = ParseEnclosedExpression( cssSrc, L";", &wstrExpr ) ;
			cssLine += L"while " ;
			cssLine += wstrExpr ;
			cssLine += L' ' ;
			//
			if ( wchNext != L';' )
			{
				OutputError
					( "for 文の継続条件式が \';\' で完了していません",
											m_strFilePath, m_nLineNum ) ;
			}
		}
		if ( cssSrc.HasToComeChar( L")" ) != L')' )
		{
			wchNext = ParseEnclosedExpression( cssSrc, L")", &wstrBy ) ;
			cssLine += L"by " ;
			cssLine += wstrBy ;
			//
			if ( wchNext != L')' )
			{
				OutputError
					( "for 文に \')\' が見つかりません",
								m_strFilePath, m_nLineNum ) ;
			}
		}
	}
	//
	// for ブロック
	//
	cssLine.MoveIndex( 0 ) ;
	err = ECSCompiler::CompileFor( cssLine ) ;
	if ( !err )
	{
		err = CompileCStyleStatement( cssSrc, true ) ;
		if ( err )
		{
			OutputError
				( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
		}
		cssLine = L"" ;
		cssLine.MoveIndex( 0 ) ;
		err = ECSCompiler::CompileNext( cssLine ) ;
	}
	if ( err )
	{
		OutputError
			( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
	}
	if ( fBeginBlock )
	{
		cssLine = L"" ;
		cssLine.MoveIndex( 0 ) ;
		return	ECSCompiler::CompileEnd( cssLine ) ;
	}
	return	eslErrSuccess ;
}

// while 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileCStyleWhile( ECSCStyleFrontStream & cssSrc )
{
	if ( cssSrc.HasToComeChar( L"(" ) != L'(' )
	{
		return	ESLErrorMsg( "while 文に \'(\' が見つかりません" ) ;
	}
	ECSCStyleStream	cssLine ;
	cssLine = cssSrc.GetEnclosedString( L')', m_dwModeFlags ) ;
	if ( cssSrc.GetAt( cssSrc.GetIndex() - 1 ) != L')' )
	{
		OutputError
			( "while 文に \')\' が見つかりません",
							m_strFilePath, m_nLineNum ) ;
	}
	cssLine.MoveIndex( 0 ) ;
	ESLError	err = ECSCompiler::CompileWhile( cssLine ) ;
	if ( err )
	{
		return	err ;
	}
	err = CompileCStyleStatement( cssSrc, true ) ;
	if ( err )
	{
		OutputError( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
	}
	cssLine = L"" ;
	cssLine.MoveIndex( 0 ) ;
	return	ECSCompiler::CompileEndWhile( cssLine ) ;
}

// do ... while 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileCStyleDo( ECSCStyleFrontStream & cssSrc )
{
	//
	// ブロック開始
	//
	ECSCStyleStream	cssLine ;
	ESLError	err = ECSCompiler::CompileRepeat( cssLine ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// do ブロック
	//
	err = CompileCStyleStatement( cssSrc, true ) ;
	if ( err )
	{
		OutputError( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
	}
	//
	// while 文
	//
	if ( !cssSrc.HasToComeToken( L"while" ) )
	{
		OutputError
			( "do に対応する while が見つかりません",
							m_strFilePath, m_nLineNum ) ;
		//
		cssLine = L"true" ;
		cssLine.MoveIndex( 0 ) ;
		return	ECSCompiler::CompileUntil( cssLine ) ;
	}
	if ( cssSrc.HasToComeChar( L"(" ) != L'(' )
	{
		OutputError
			( "while に \'(\' が見つかりません",
						m_strFilePath, m_nLineNum ) ;
		//
		cssLine = L"true" ;
		cssLine.MoveIndex( 0 ) ;
		return	ECSCompiler::CompileUntil( cssLine ) ;
	}
	cssLine = L"!(" ;
	cssLine += cssSrc.GetEnclosedString( L')', m_dwModeFlags ) ;
	cssLine += L')' ;
	//
	if ( cssSrc.GetAt( cssSrc.GetIndex() - 1 ) != L')' )
	{
		OutputError
			( "\'(\' に対応する \')\' が見つかりません",
							m_strFilePath, m_nLineNum ) ;
	}
	if ( cssSrc.HasToComeChar( L";" ) != L';' )
	{
		OutputError
			( "while 文に \';\' が見つかりません",
							m_strFilePath, m_nLineNum ) ;
	}
	cssLine.MoveIndex( 0 ) ;
	return	ECSCompiler::CompileUntil( cssLine ) ;
}

// switch 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileCStyleSwitch( ECSCStyleFrontStream & cssSrc )
{
	//
	// switch 文
	//
	if ( cssSrc.HasToComeChar( L"(" ) != L'(' )
	{
		return	ESLErrorMsg( "switch 文に \'(\' が見つかりません" ) ;
	}
	ECSCStyleStream	cssLine ;
	cssLine = cssSrc.GetEnclosedString( L')', m_dwModeFlags ) ;
	cssLine.MoveIndex( 0 ) ;
	//
	ESLError	err = ECSCompiler::CompileSwitch( cssLine ) ;
	if ( err )
	{
		OutputError( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
	}
	if ( cssSrc.GetAt( cssSrc.GetIndex() - 1 ) != L')' )
	{
		OutputError
			( "switch に \')\' が見つかりません",
							m_strFilePath, m_nLineNum ) ;
	}
	//
	// switch ブロック
	//
	err = CompileCStyleStatement( cssSrc, true ) ;
	if ( err )
	{
		OutputError( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
	}
	//
	// switch 終了
	//
	cssLine = L"" ;
	cssLine.MoveIndex( 0 ) ;
	return	ECSCompiler::CompileEndSwitch( cssLine ) ;
}

// case 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileCStyleCase( ECSCStyleFrontStream & cssSrc )
{
	ECSCStyleStream	cssLine ;
	EWideString	wstrExpr ;
	if ( !ParseEnclosedExpressionByToken( cssSrc, L":", &wstrExpr ) )
	{
		return	ESLErrorMsg( "case ラベルに \':\' が見つかりません" ) ;
	}
	cssLine = wstrExpr ;
	cssLine.MoveIndex( 0 ) ;
	//
	ESLError	err = ECSCompiler::CompileCase( cssLine ) ;
	if ( err )
	{
		return	err ;
	}
	return	eslErrSuccess ;
}

// default 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileCStyleDefault( ECSCStyleFrontStream & cssSrc )
{
	ECSCStyleStream	cssLine ;
	ESLError	err = ECSCompiler::CompileDefault( cssLine ) ;
	if ( err )
	{
		return	err ;
	}
	if ( cssSrc.HasToComeChar( L":" ) != L':' )
	{
		return	ESLErrorMsg( "default ラベルに \':\' が見つかりません" ) ;
	}
	return	eslErrSuccess ;
}

// _m_fence 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileCStyleMemoryFence( ECSCStyleFrontStream & cssSrc )
{
	ECSCStyleStream	cssLine ;
	ESLError	err = ECSCompiler::CompileMemoryFence( cssLine ) ;
	if ( err )
	{
		return	err ;
	}
	if ( cssSrc.HasToComeChar( L";" ) != L';' )
	{
		return	ESLErrorMsg( "_m_fence 文末に \';\' が見つかりません" ) ;
	}
	return	eslErrSuccess ;
}

// template 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileCStyleTemplate( ECSCStyleFrontStream & cssSrc )
{
	ECSCStyleStream	cssLine ;
	EWideString	wstrTemplateArg ;
	if ( cssSrc.HasToComeChar( L"<" ) != L'<' )
	{
		return	ESLErrorMsg( "テンプレート引数が見つかりません" ) ;
	}
	if ( !ParseEnclosedExpressionByToken( cssSrc, L">", &wstrTemplateArg ) )
	{
		return	ESLErrorMsg( "テンプレート引数が \'>\' で終わっていません" ) ;
	}
	EWideString	wstrTemplateType = cssSrc.GetAToken() ;
	if ( wstrTemplateType == L"{" )
	{
		return	ESLErrorMsg( "テンプレート名が見つかりません" ) ;
	}
	EWideString	wstrTemplateStatement ;
	bool		fFunction = false ;
	cssLine = L"< " + wstrTemplateArg + L" > " ;
	if ( wstrTemplateType == L"class" )
	{
		EWideString		wstrTagName ;
		wchar_t			wchNext ;
		ReservedWord	rwType =
			ParseClassDeclarationToBasicStatement
				( wstrTemplateStatement, wstrTagName, wchNext, cssSrc, false ) ;
		if ( rwType != rwClass )
		{
			return	ESLErrorMsg( "クラステンプレートの書式が不正です" ) ;
		}
		if ( wchNext != L'{' )
		{
			return	ESLErrorMsg( "テンプレートの開始 \'{\' が見つかりません" ) ;
		}
	}
	else if ( wstrTemplateType == L"struct" )
	{
		EWideString		wstrTagName ;
		wchar_t			wchNext ;
		ReservedWord	rwType =
			ParseClassDeclarationToBasicStatement
				( wstrTemplateStatement, wstrTagName, wchNext, cssSrc, true ) ;
		if ( rwType != rwStructure )
		{
			return	ESLErrorMsg( "構造体テンプレートの書式が不正です" ) ;
		}
		wstrTemplateType = L"structure" ;
		if ( wchNext != L'{' )
		{
			return	ESLErrorMsg( "テンプレートの開始 \'{\' が見つかりません" ) ;
		}
	}
	else
	{
		if ( !ParseEnclosedExpressionByToken
					( cssSrc, L"{", &wstrTemplateStatement ) )
		{
			return	ESLErrorMsg( "テンプレートの開始 \'{\' が見つかりません" ) ;
		}
		fFunction = true ;
		cssLine += L"function " ;
	}
	cssLine += wstrTemplateType + L" " + wstrTemplateStatement ;
	cssLine.MoveIndex( 0 ) ;
	//
	ESLError	err = ECSCompiler::CompileTemplate( cssLine ) ;
	if ( err )
	{
		return	err ;
	}
	m_pTemplateNest = GetMostInnerNest( rwTemplate, rwTemplate ) ;
	ESLAssert( m_pTemplateNest != NULL ) ;
	ESLAssert( m_pTemplateNest->m_rwType == rwTemplate ) ;
	//
	err = CompileCStyleMultiStatement( cssSrc ) ;
	if ( err )
	{
		OutputError( GetESLErrorMsg(err), m_strFilePath, m_nLineNum ) ;
	}
	cssLine = L"" ;
	cssLine.MoveIndex( 0 ) ;
	err = ECSCompiler::CompileEndTemplate( cssLine ) ;
	m_pTemplateNest = GetMostInnerNest( rwTemplate, rwTemplate ) ;
	if ( err )
	{
		return	err ;
	}
	if ( !fFunction )
	{
		if ( cssSrc.HasToComeChar( L";" ) != L';' )
		{
			return	ESLErrorMsg
				( "template ブロックの末尾に \';\' が見つかりません" ) ;
		}
	}
	return	eslErrSuccess ;
}

// using 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileCStyleUsing( ECSCStyleFrontStream & cssSrc )
{
	ECSCStyleStream	cssLine ;
	wchar_t	wchClose =
				ParseEnclosedExpression( cssSrc, L";}", &cssLine ) ;
	cssLine.MoveIndex( 0 ) ;
	//
	ESLError	err = ECSCompiler::CompileUsing( cssLine ) ;
	if ( err )
	{
		return	err ;
	}
	if ( wchClose != L';' )
	{
		return	ESLErrorMsg( "using 文末に \';\' が見つかりません" ) ;
	}
	return	eslErrSuccess ;
}

// friend 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileCStyleFriend( ECSCStyleFrontStream & cssSrc )
{
	ECSCStyleStream	cssLine ;
	wchar_t	wchClose =
				ParseEnclosedExpression( cssSrc, L";}", &cssLine ) ;
	cssLine.MoveIndex( 0 ) ;
	//
	ESLError	err = ECSCompiler::CompileFriend( cssLine ) ;
	if ( err )
	{
		return	err ;
	}
	if ( wchClose != L';' )
	{
		return	ESLErrorMsg( "friend 文末に \';\' が見つかりません" ) ;
	}
	return	eslErrSuccess ;
}

// ディレクティブ判定
//////////////////////////////////////////////////////////////////////////////
ECSCStyleCompiler::CStyleDirective
	ECSCStyleCompiler::IsDirectiveWord( const wchar_t * pwszToken ) const
{
	static const wchar_t *	pwszDirectives[] =
	{
		L"if", L"ifdef", L"ifndef", L"elif", L"elseif", L"else", L"endif",
		L"define", L"include", L"undef",
		L"error", L"warning", L"mode",
		NULL
	} ;
	static const CStyleDirective	csdDirectives[] =
	{
		csdIf, csdIfdef, csdIfndef, csdElseif, csdElseif, csdElse, csdEndif,
		csdDefine, csdInclude, csdUndef,
		csdError, csdWarning, csdMode,
	} ;
	for ( int i = 0; pwszDirectives[i] != NULL; i ++ )
	{
		if ( !EWideString::Compare( pwszDirectives[i], pwszToken ) )
		{
			return	csdDirectives[i] ;
		}
	}
	return	csdInvalid ;
}

// ディレクティブ処理
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileDirectiveLine( ECSSourceStream & cssLine )
{
	EWideString		wstrToken = cssLine.GetAToken() ;
	CStyleDirective	csdDirective = IsDirectiveWord( wstrToken ) ;
	if ( csdDirective != csdInvalid )
	{
		return	(this->*m_pfnCompileDirective[csdDirective])( cssLine ) ;
	}
	return	ESLErrorMsg( "不正なディレクティブです" ) ;
}

// マクロ予約語の処理
//////////////////////////////////////////////////////////////////////////////
const ECSCStyleCompiler::PFN_COMPILE_DIRECTIVE
	ECSCStyleCompiler::m_pfnCompileDirective[ECSCStyleCompiler::csdMax] =
{
	&ECSCStyleCompiler::CompileDirectiveIf,
	&ECSCStyleCompiler::CompileDirectiveIfDef,
	&ECSCStyleCompiler::CompileDirectiveIfNDef,
	&ECSCStyleCompiler::CompileDirectiveElseIf,
	&ECSCStyleCompiler::CompileDirectiveElse,
	&ECSCStyleCompiler::CompileDirectiveEndIf,
	&ECSCStyleCompiler::CompileDirectiveDefine,
	&ECSCStyleCompiler::CompileDirectiveInclude,
	&ECSCStyleCompiler::CompileDirectiveUnDef,
	&ECSCStyleCompiler::CompileDirectiveError,
	&ECSCStyleCompiler::CompileDirectiveWarning,
	&ECSCStyleCompiler::CompileDirectiveMode,
} ;

// #if 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileDirectiveIf( ECSSourceStream & cssLine )
{
	//
	// 条件式判定
	//
	ECSObject *	pExpr = NULL ;
	bool	fExpr = false ;
	if ( IsInEnabledDirectiveBlock() )
	{
		FilterTextPreprocessor( cssLine ) ;
		ESLError	err =
			CalculateExpression( pExpr, cssLine, 0, NULL, true, true ) ;
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
	//
	// ネスト生成
	//
	ESourceStreamNest *	pNest = new ESourceStreamNest ;
	pNest->m_fEnabled = fExpr ;
	pNest->m_fCompletion = fExpr ;
	//
	m_nestSource.Push( pNest ) ;
	//
	return	eslErrSuccess ;
}

// #ifdef 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileDirectiveIfDef( ECSSourceStream & cssLine )
{
	//
	// 条件判定
	//
	EWideString	wstrToken = cssLine.GetAToken() ;
	bool	fExpr = (GetMacroVariable( wstrToken ) != NULL) ;
	//
	// ネスト生成
	//
	ESourceStreamNest *	pNest = new ESourceStreamNest ;
	pNest->m_fEnabled = fExpr ;
	pNest->m_fCompletion = fExpr ;
	//
	m_nestSource.Push( pNest ) ;
	//
	// 余分な記述を判定
	//
	if ( !cssLine.DisregardSpace() )
	{
		return	ESLErrorMsg( "#ifdef に複数のシンボルが記述されています" ) ;
	}
	else if ( wstrToken.IsEmpty() )
	{
		return	ESLErrorMsg( "#ifdef にシンボルが指定されていません" ) ;
	}
	return	eslErrSuccess ;
}

// #ifndef 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileDirectiveIfNDef( ECSSourceStream & cssLine )
{
	//
	// 条件判定
	//
	EWideString	wstrToken = cssLine.GetAToken() ;
	bool	fExpr = (GetMacroVariable( wstrToken ) == NULL) ;
	//
	// ネスト生成
	//
	ESourceStreamNest *	pNest = new ESourceStreamNest ;
	pNest->m_fEnabled = fExpr ;
	pNest->m_fCompletion = fExpr ;
	//
	m_nestSource.Push( pNest ) ;
	//
	// 余分な記述を判定
	//
	if ( !cssLine.DisregardSpace() )
	{
		return	ESLErrorMsg( "#ifndef に複数のシンボルが記述されています" ) ;
	}
	else if ( wstrToken.IsEmpty() )
	{
		return	ESLErrorMsg( "#ifndef にシンボルが指定されていません" ) ;
	}
	return	eslErrSuccess ;
}

// #elseif 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileDirectiveElseIf( ECSSourceStream & cssLine )
{
	ESourceStreamNest *	pNest = m_nestSource.GetLastAt() ;
	if ( (pNest == NULL) || (pNest->m_csdType == csdElse) )
	{
		return	ESLErrorMsg( "#elseif 文に対応する #if 文が見つかりません" ) ;
	}
	if ( pNest->m_fCompletion )
	{
		pNest->m_csdType = csdElseif ;
		pNest->m_fEnabled = false ;
		return	eslErrSuccess ;
	}
	//
	// 条件式判定
	//
	bool	fExpr = false ;
	if ( !pNest->m_fCompletion && !pNest->m_fEnabled )
	{
		ECSObject *	pExpr = NULL ;
		FilterTextPreprocessor( cssLine ) ;
		ESLError	err =
			CalculateExpression( pExpr, cssLine, 0, NULL, true, true ) ;
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
	//
	// ネスト更新
	//
	pNest->m_csdType = csdElseif ;
	pNest->m_fEnabled = fExpr ;
	pNest->m_fCompletion = fExpr ;
	//
	return	eslErrSuccess ;
}

// #else 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileDirectiveElse( ECSSourceStream & cssLine )
{
	ESourceStreamNest *	pNest = m_nestSource.GetLastAt() ;
	if ( pNest == NULL )
	{
		return	ESLErrorMsg( "#else 文に対応する #if 文が見つかりません" ) ;
	}
	pNest->m_csdType = csdElse ;
	pNest->m_fEnabled = !pNest->m_fCompletion ;
	pNest->m_fCompletion = true ;
	return	eslErrSuccess ;
}

// #endif 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileDirectiveEndIf( ECSSourceStream & cssLine )
{
	ESourceStreamNest *	pNest = m_nestSource.GetLastAt() ;
	if ( pNest == NULL )
	{
		return	ESLErrorMsg( "#endif 文に対応する #if 文が見つかりません" ) ;
	}
	delete	m_nestSource.Pop() ;
	return	eslErrSuccess ;
}

// #define 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileDirectiveDefine( ECSSourceStream & cssLine )
{
	//
	// 定義名を取得
	//
	EWideString	wstrName = cssLine.GetAToken() ;
	ESLError	err = VerifyUserSymbol( wstrName ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// 引数取得
	//
	EObjArray<EWideString>	lstArg ;
	EWideString	wstrArgList ;
	if ( cssLine.CurrentCharacter() == L'(' )
	{
		cssLine.GetCharacter() ;
		for ( ; ; )
		{
			EWideString	wstrArg = cssLine.GetAToken() ;
			err = VerifyUserSymbol( wstrArg ) ;
			if ( err )
			{
				return	err ;
			}
			lstArg.Add( new EWideString( wstrArg ) ) ;
			//
			if ( !wstrArgList.IsEmpty() )
			{
				wstrArgList += L',' ;
			}
			wstrArgList += wstrArg ;
			//
			wchar_t	wch = cssLine.HasToComeChar( L",)" ) ;
			if ( wch == L')' )
			{
				break ;
			}
			else if ( wch != L',' )
			{
				return	ESLErrorMsg
					( "#define マクロ引数が \',\' で区切られていません" ) ;
			}
		}
	}
	//
	// 置き換えマクロ式生成
	//
	EWideString	wstrExpr ;
	EWideString	wstrLiteral ;
	FilterTextPreprocessor( cssLine ) ;
	int	iLastIndex = cssLine.GetIndex() ;
	while ( !cssLine.DisregardSpace() )
	{
		if ( iLastIndex != (int) cssLine.GetIndex() )
		{
			// スペースが含まれていた場合 ' ' に置き換え
			wstrLiteral += L' ' ;
		}
		bool	fTextEncoding = false ;
		bool	fTextMerge = false ;
		if ( cssLine.CurrentCharacter() == L'#' )
		{
			cssLine.GetCharacter() ;
			if ( cssLine.CurrentCharacter() == L'#' )
			{
				cssLine.GetCharacter() ;
				fTextMerge = true ;
			}
			else
			{
				fTextEncoding = true ;
			}
		}
		EWideString	wstrToken = cssLine.GetAToken() ;
		int	iArg = lstArg.Find( wstrToken ) ;
		if ( iArg >= 0 )
		{
			// 引数の前に直前リテラルを挿入
			if ( fTextEncoding )
			{
				wstrLiteral += L'\"' ;
			}
			if ( !wstrLiteral.IsEmpty() )
			{
				if ( !wstrExpr.IsEmpty() )
				{
					wstrExpr += L'+' ;
				}
				wstrExpr += L'\"' ;
				EDescription::EncodeTextCEscSequence( wstrLiteral ) ;
				wstrExpr += wstrLiteral ;
				wstrExpr += L'\"' ;
				wstrLiteral = L"" ;
			}
			// マクロ引数の展開
			if ( !wstrExpr.IsEmpty() )
			{
				wstrExpr += L'+' ;
			}
			if ( fTextMerge )
			{
				// X##Y 形式
				wstrExpr += wstrToken ;
			}
			else if ( fTextEncoding )
			{
				// #X 形式
				wstrExpr += wstrToken ;
				wstrExpr += L".GetCEncoded()" ;
				wstrLiteral += L'\"' ;
			}
			else
			{
				wstrExpr += wstrToken ;
			}
		}
		else
		{
			// リテラルを追加
			if ( fTextEncoding )
			{
				wstrLiteral += L'#' ;
			}
			wstrLiteral += wstrToken ;
		}
		iLastIndex = cssLine.GetIndex() ;
	}
	// 最終的にリテラルを追加
	if ( !wstrLiteral.IsEmpty() )
	{
		if ( !wstrExpr.IsEmpty() )
		{
			wstrExpr += L'+' ;
		}
		wstrExpr += L'\"' ;
		EDescription::EncodeTextCEscSequence( wstrLiteral ) ;
		wstrExpr += wstrLiteral ;
		wstrExpr += L'\"' ;
		wstrLiteral = L"" ;
	}
	if ( wstrExpr.IsEmpty() )
	{
		wstrExpr = L"\"\"" ;
	}
	//
	// マクロ定義
	//
	ECSCStyleStream	cssStatement ;
	cssStatement = L"@macro " + wstrName + L"(" + wstrArgList + L")" ;
	err = ECSCompiler::CompileScriptLine( cssStatement, m_nLineNum ) ;
	if ( !err )
	{
		cssStatement = L"@exitmacro " + wstrExpr ;
		err = ECSCompiler::CompileScriptLine( cssStatement, m_nLineNum ) ;
	}
	if ( err )
	{
		EString	strErrMsg = GetESLErrorMsg(err) ;
		m_strErrMsg = "Cスタイル変換内部エラー：" + strErrMsg ;
		OutputError( m_strErrMsg, m_strFilePath, m_nLineNum ) ;
	}
	cssStatement = L"@endmacro" ;
	err = ECSCompiler::CompileScriptLine( cssStatement, m_nLineNum ) ;
	if ( err )
	{
		EString	strErrMsg = GetESLErrorMsg(err) ;
		m_strErrMsg = "Cスタイル変換内部エラー：" + strErrMsg ;
		return	ESLErrorMsg( m_strErrMsg ) ;
	}
	return	eslErrSuccess ;
}

// #include 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileDirectiveInclude( ECSSourceStream & cssLine )
{
	wchar_t		wch = cssLine.HasToComeChar( L"<\"" ) ;
	EString	strFilePath ;
	if ( wch == L'<' )
	{
		strFilePath =
			cssLine.GetEnclosedString( L'>', cssLine.flagDisableExpression ) ;
	}
	else if ( wch == L'\"' )
	{
		strFilePath =
			cssLine.GetEnclosedString( L'\"', cssLine.flagDisableExpression ) ;
	}
	else
	{
		return	ESLErrorMsg
			( "インクルードファイル名が指定されていません。" ) ;
	}
	ESLError	err = IncludeSourceScript( strFilePath ) ;
	SetCStyleMode
		( true, ((m_dwModeFlags & flagDefaultNakedAll) != 0) ) ;
	return	err ;
}

// #undef 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileDirectiveUnDef( ECSSourceStream & cssLine )
{
	//
	// 条件判定
	//
	ESLError	err = ECSCompiler::CompileMacroUndefMacro( cssLine ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// 余分な記述を判定
	//
	if ( !cssLine.DisregardSpace() )
	{
		return	ESLErrorMsg( "#undef に複数のシンボルが記述されています" ) ;
	}
	return	eslErrSuccess ;
}

// #error 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileDirectiveError( ECSSourceStream & cssLine )
{
	cssLine.DisregardSpace() ;
	EString	strExpr = cssLine.Middle( cssLine.GetIndex() );
	return	OutputError( strExpr, m_strFilePath, m_nLineNum ) ;
}

// #warning 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileDirectiveWarning( ECSSourceStream & cssLine )
{
	cssLine.DisregardSpace() ;
	EString	strExpr = cssLine.Middle( cssLine.GetIndex() );
	return	OutputWarning0( strExpr, m_strFilePath, m_nLineNum ) ;
}

// #mode 文
//////////////////////////////////////////////////////////////////////////////
ESLError ECSCStyleCompiler::CompileDirectiveMode( ECSSourceStream & cssLine )
{
	enum	OptionUsageIndex
	{
		optionBasicStyle,
		optionCStyle,
		optionCCompatible,
	} ;
	static const wchar_t *	pwszOptionUsages[] =
	{
		L"basic style",
		L"c style",
		L"c compatible",
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
			if ( wstrToken.Compare( cssLine.GetAToken() ) )
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
	case	optionBasicStyle:
		SetCStyleMode( false, false ) ;
		break ;
	case	optionCStyle:
		SetCStyleMode( true, false ) ;
		break ;
	case	optionCCompatible:
		SetCStyleMode( true, true ) ;
		break ;
	default:
		return	ESLErrorMsg( "不正な #mode 引数です。" ) ;
	}
	return	eslErrSuccess ;
}

