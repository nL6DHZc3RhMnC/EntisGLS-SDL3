
#include <rosetta/rosetta.h>
#include <glscs/glscs_sakura2_module_maker.h>
#include <rosetta/rosetta_compiler.h>

using namespace	SSystem ;
using namespace	Rosetta ;
using namespace ECSSakura2 ;
using namespace ECSSakura2Processor ;


//////////////////////////////////////////////////////////////////////////////
// コメント
//////////////////////////////////////////////////////////////////////////////

// XML 取得
//////////////////////////////////////////////////////////////////////////////
const SSystem::SXMLDocument& RSCodeComment::GetXMLDocument( void )
{
	if ( m_pXMLDoc == NULL )
	{
		SStrSortObjectArray<SString>	ssoaDTD ;
		SStringParser	sparsDoc ;
		sparsDoc.AttachString( m_strText ) ;
		m_pXMLDoc = new SXMLDocument ;
		m_pXMLDoc->ParseXMLElements( sparsDoc, ssoaDTD, *m_pXMLDoc ) ;
	}
	return	*m_pXMLDoc ;
}


//////////////////////////////////////////////////////////////////////////////
// 中間コード
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSCode, ESLObject )
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSCodeLiteral, RSCode )
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSCodeControl, RSCode )
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSCodeOperator, RSCode )
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSParenthesis, RSCode )
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSCodeSymbol, RSCode )

// 制御語
//////////////////////////////////////////////////////////////////////////////
const wchar_t *	RSCodeControl::m_pwszControlWord[RSCodeControl::wiCount] =
{
	L"import",
	L"class", L"struct", L"function", L"extends", L"implements",
	L"for", L"while", L"do",
	L"if", L"else", L"switch", L"case", L"default",
	L"break", L"continue", L"try", L"catch", L"finally",
	L"throw", L"return", L"with", L"synchronized",
	L"static", L"abstract", L"native", L"const",
	L"public", L"protected", L"private",
	L"var", L"void", L"boolean", L"byte", L"short", L"char",
	L"int", L"long", L"float", L"double",
	L"this", L"super", L"extern",
} ;

RSCodeControl::WordIndex
	RSCodeControl::IsControlWord( const wchar_t * pwszSymbol )
{
	for ( size_t i = 0; i < wiCount; i ++ )
	{
		if ( SString::Compare( m_pwszControlWord[i], pwszSymbol ) == 0 )
		{
			return	(WordIndex) i ;
		}
	}
	return	wiInvalid ;
}

const wchar_t * RSCodeControl::ControlWordAt( int index )
{
	if ( (index >= 0) && (index < wiCount) )
	{
		return	m_pwszControlWord[index] ;
	}
	return	NULL ;
}

// 演算子
//////////////////////////////////////////////////////////////////////////////
const RSCodeOperator::OperatorInfo
	RSCodeOperator::m_infoOperators[RSCodeOperator::opCount] =
{
	{	// +
		RSCodeOperator::ruleBinary | RSCodeOperator::ruleUnary,
		RSCodeOperator::priorityAdd, RSCodeOperator::priorityUnary, 0
	},
	{	// -
		RSCodeOperator::ruleBinary | RSCodeOperator::ruleUnary,
		RSCodeOperator::priorityAdd, RSCodeOperator::priorityUnary, 0
	},
	{	// *
		RSCodeOperator::ruleBinary,
		RSCodeOperator::priorityMul, 0, 0
	},
	{	// /
		RSCodeOperator::ruleBinary,
		RSCodeOperator::priorityMul, 0, 0
	},
	{	// %
		RSCodeOperator::ruleBinary,
		RSCodeOperator::priorityMul, 0, 0
	},
	{	// &
		RSCodeOperator::ruleBinary,
		RSCodeOperator::priorityAnd, 0, 0
	},
	{	// |
		RSCodeOperator::ruleBinary,
		RSCodeOperator::priorityOr, 0, 0
	},
	{	// ^
		RSCodeOperator::ruleBinary,
		RSCodeOperator::priorityXor, 0, 0
	},
	{	// ~
		RSCodeOperator::ruleUnary,
		0, RSCodeOperator::priorityUnary, 0
	},
	{	// >>>
		RSCodeOperator::ruleBinary,
		RSCodeOperator::priorityShift, 0, 0
	},
	{	// <<
		RSCodeOperator::ruleBinary,
		RSCodeOperator::priorityShift, 0, 0
	},
	{	// >>
		RSCodeOperator::ruleBinary,
		RSCodeOperator::priorityShift, 0, 0
	},
	{	// ++
		RSCodeOperator::ruleUnary | RSCodeOperator::ruleUnaryPost,
		0, RSCodeOperator::priorityUnary, RSCodeOperator::priorityUnaryPost
	},
	{	// --
		RSCodeOperator::ruleUnary | RSCodeOperator::ruleUnaryPost,
		0, RSCodeOperator::priorityUnary, RSCodeOperator::priorityUnaryPost
	},
	{	// ==
		RSCodeOperator::ruleBinary,
		RSCodeOperator::priorityCompare, 0, 0
	},
	{	// !=
		RSCodeOperator::ruleBinary,
		RSCodeOperator::priorityCompare, 0, 0
	},
	{	// <=
		RSCodeOperator::ruleBinary,
		RSCodeOperator::priorityCompare, 0, 0
	},
	{	// <
		RSCodeOperator::ruleBinary,
		RSCodeOperator::priorityCompare, 0, 0
	},
	{	// >=
		RSCodeOperator::ruleBinary,
		RSCodeOperator::priorityCompare, 0, 0
	},
	{	// >
		RSCodeOperator::ruleBinary,
		RSCodeOperator::priorityCompare, 0, 0
	},
	{	// ===
		RSCodeOperator::ruleBinary,
		RSCodeOperator::priorityCompare, 0, 0
	},
	{	// !==
		RSCodeOperator::ruleBinary,
		RSCodeOperator::priorityCompare, 0, 0
	},
	{	// &&
		RSCodeOperator::ruleBinary | RSCodeOperator::ruleSpecialRight,
		RSCodeOperator::priorityLAnd, 0, 0
	},
	{	// ||
		RSCodeOperator::ruleBinary | RSCodeOperator::ruleSpecialRight,
		RSCodeOperator::priorityLOr, 0, 0
	},
	{	// !
		RSCodeOperator::ruleUnary,
		0, RSCodeOperator::priorityUnary, 0
	},
	{	// =
		RSCodeOperator::ruleBinary | RSCodeOperator::ruleRightToLeft,
		RSCodeOperator::priorityMove, 0, 0
	},
	{	// +=
		RSCodeOperator::ruleBinary | RSCodeOperator::ruleRightToLeft,
		RSCodeOperator::priorityMove, 0, 0
	},
	{	// -=
		RSCodeOperator::ruleBinary | RSCodeOperator::ruleRightToLeft,
		RSCodeOperator::priorityMove, 0, 0
	},
	{	// *=
		RSCodeOperator::ruleBinary | RSCodeOperator::ruleRightToLeft,
		RSCodeOperator::priorityMove, 0, 0
	},
	{	// /=
		RSCodeOperator::ruleBinary | RSCodeOperator::ruleRightToLeft,
		RSCodeOperator::priorityMove, 0, 0
	},
	{	// %=
		RSCodeOperator::ruleBinary | RSCodeOperator::ruleRightToLeft,
		RSCodeOperator::priorityMove, 0, 0
	},
	{	// &=
		RSCodeOperator::ruleBinary | RSCodeOperator::ruleRightToLeft,
		RSCodeOperator::priorityMove, 0, 0
	},
	{	// |=
		RSCodeOperator::ruleBinary | RSCodeOperator::ruleRightToLeft,
		RSCodeOperator::priorityMove, 0, 0
	},
	{	// ^=
		RSCodeOperator::ruleBinary | RSCodeOperator::ruleRightToLeft,
		RSCodeOperator::priorityMove, 0, 0
	},
	{	// >>>=
		RSCodeOperator::ruleBinary | RSCodeOperator::ruleRightToLeft,
		RSCodeOperator::priorityMove, 0, 0
	},
	{	// <<=
		RSCodeOperator::ruleBinary | RSCodeOperator::ruleRightToLeft,
		RSCodeOperator::priorityMove, 0, 0
	},
	{	// >>=
		RSCodeOperator::ruleBinary | RSCodeOperator::ruleRightToLeft,
		RSCodeOperator::priorityMove, 0, 0
	},
	{	// ::
		RSCodeOperator::ruleBinary | RSCodeOperator::ruleSpecialRight,
		RSCodeOperator::priorityNamespace, 0, 0
	},
	{	// .
		RSCodeOperator::ruleBinary | RSCodeOperator::ruleSpecialRight,
		RSCodeOperator::priorityMember, 0, 0
	},
	{	// .*
		RSCodeOperator::ruleBinary,
		RSCodeOperator::priorityMemberCall, 0, 0
	},
	{	// instanceof
		RSCodeOperator::ruleBinary,
		RSCodeOperator::priorityCompare, 0, 0
	},
	{	// new
		RSCodeOperator::ruleUnary,
		0, RSCodeOperator::priorityUnary, 0
	},
	{	// ?
		RSCodeOperator::ruleBinary | RSCodeOperator::ruleSpecialRight,
		RSCodeOperator::prioritySelector, 0, 0
	},
	{	// :
		RSCodeOperator::ruleBinary,
		RSCodeOperator::prioritySeparator, 0, 0
	},
	{	// ,
		RSCodeOperator::ruleBinary,
		RSCodeOperator::priorityList, 0, 0
	},
	{	// ;
		RSCodeOperator::ruleUnaryPost,
		0, 0, RSCodeOperator::priorityNothing
	},
	{	// ->
		RSCodeOperator::ruleBinary,
		RSCodeOperator::priorityNothing, 0, 0 
	},
} ;

const wchar_t *	RSCodeOperator::m_pwszOperators[opCount] =
{
	L"+", L"-", L"*", L"/", L"%",
	L"&", L"|", L"^", L"~",
	L">>>", L"<<", L">>", L"++", L"--",
	L"==", L"!=", L"<=", L"<", L">=", L">", L"===", L"!==",
	L"&&", L"||", L"!",
	L"=", L"+=", L"-=", L"*=", L"/=", L"%=",
	L"&=", L"|=", L"^=",
	L">>>=", L"<<=", L">>=",
	L"::", L".", L".*", L"instanceof", L"new",
	L"?", L":", L",", L";", L"->"
} ;

RSCodeOperator::OperatorIndex
	RSCodeOperator::IsOperatorWord( const wchar_t * pwszSymbol )
{
	for ( size_t i = 0; i < opCount; i ++ )
	{
		if ( SString::Compare( m_pwszOperators[i], pwszSymbol ) == 0 )
		{
			return	(OperatorIndex) i ;
		}
	}
	return	opInvalid ;
}

// 比較演算子か？
//////////////////////////////////////////////////////////////////////////////
bool RSCodeOperator::IsComparator( RSCodeOperator::OperatorIndex opIndex )
{
	return	(opEqual <= opIndex) && (opIndex <= opPointerNotEqual) ;
}

// 代入演算子か？
//////////////////////////////////////////////////////////////////////////////
bool RSCodeOperator::IsMoveOperator( RSCodeOperator::OperatorIndex opIndex )
{
	return	(opMoveFirst <= opIndex) && (opIndex <= opMoveLast) ;
}

// 左辺式（代入演算子・インクリメント・デクリメント）を要求するか？
//////////////////////////////////////////////////////////////////////////////
bool RSCodeOperator::IsMoveLeftOperator( OperatorIndex opIndex )
{
	return	IsMoveOperator( opIndex )
				|| (opIndex == opIncrement)
				|| (opIndex == opDecrement) ;
}

// 指定文字指標範囲内の最も若いデバッグ位置を検索
//////////////////////////////////////////////////////////////////////////////
ssize_t RSParenthesis::FindDebugPoint( size_t iFirst, size_t iEnd ) const
{
	size_t	nCount = m_terms.GetLength() ;
	size_t	i = 0 ;
	while ( i < nCount )
	{
		RSCode *	pCode = m_terms.GetAt( i ++ ) ;
		ESLAssert( pCode != NULL ) ;
		if ( pCode == NULL )
		{
			continue ;
		}
		if ( (pCode->m_iSrc >= iFirst)
			&& (pCode->m_iSrc < iEnd) )
		{
			if ( pCode->m_type == typeControlCode )
			{
				RSCodeControl *	pCtrl = ESLTypeCast<RSCodeControl>( pCode ) ;
				if ( (pCtrl != NULL)
					&& (pCtrl->m_word == RSCodeControl::wiDebugPoint) )
				{
					return	(ssize_t) (pCode->m_iSrc) ;
				}
			}
			else if ( pCode->m_type == typeParenthesis )
			{
				RSParenthesis *	pSub = ESLTypeCast<RSParenthesis>( pCode ) ;
				if ( pSub != NULL )
				{
					ssize_t	iDebugPos = pSub->FindDebugPoint( iFirst, iEnd ) ;
					if ( iDebugPos >= 0 )
					{
						return	iDebugPos ;
					}
				}
			}
		}
		else if ( pCode->m_type == typeParenthesis )
		{
			if ( pCode->m_iSrc > iEnd )
			{
				continue ;
			}
			RSCode *	pCodeNext = m_terms.GetAt( i ) ;
			if ( (pCodeNext == NULL)
				|| (pCodeNext->m_iSrc >= iFirst) )
			{
				RSParenthesis *	pSub = ESLTypeCast<RSParenthesis>( pCode ) ;
				if ( pSub != NULL )
				{
					ssize_t	iDebugPos = pSub->FindDebugPoint( iFirst, iEnd ) ;
					if ( iDebugPos >= 0 )
					{
						return	iDebugPos ;
					}
				}
			}
		}
	}
	return	-1 ;
}


//////////////////////////////////////////////////////////////////////////////
// 標準的なソースコード・パーサー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSSourceParser, SStringParser )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSSourceParser::RSSourceParser( void )
{
}

RSSourceParser::RSSourceParser( const SStringParser& ss )
	: SStringParser( ss )
{
}

RSSourceParser::RSSourceParser( const SString& src )
	: SStringParser( src )
{
}

RSSourceParser::RSSourceParser( const wchar_t * pszSrc, ssize_t nLength )
	: SStringParser( pszSrc, nLength )
{
}

// 現在のトークンを通過する
//////////////////////////////////////////////////////////////////////////////
SStringParser::TokenType RSSourceParser::PassToken( void )
{
	const uint16_t *	pszText = m_pszText ;
	const size_t		lenText = m_lenText ;
	size_t				index = m_index ;
	TokenType			typeToken = tokenInvalid ;
	//
	if ( index >= lenText )
	{
		return	tokenInvalid ;
	}
	wchar_t	wch = pszText[index ++] ;
	if ( wch <= L' ' )
	{
		return	tokenInvalid ;
	}
	else if ( wch == L'<' )
	{
		static const wchar_t *	pwszOperators1[] =
		{
			L"<=", L"<", L"=", NULL
		} ;
		for ( size_t i = 0; pwszOperators1[i]; i ++ )
		{
			const wchar_t *	pwszOperator = pwszOperators1[i] ;
			size_t	j ;
			for ( j = 0; pwszOperator[j] && (index + j < lenText); j ++ )
			{
				if ( pwszOperator[j] != pszText[index + j] )
				{
					break ;
				}
			}
			if ( pwszOperator[j] == 0 )
			{
				m_index = index + j ;
				return	tokenPunctuation ;
			}
		}
	}
	else if ( wch == L'>' )
	{
		static const wchar_t *	pwszOperators2[] =
		{
			L">>=", L">=", L">", L"=", NULL
		} ;
		for ( size_t i = 0; pwszOperators2[i]; i ++ )
		{
			const wchar_t *	pwszOperator = pwszOperators2[i] ;
			size_t	j ;
			for ( j = 0; pwszOperator[j] && (index + j < lenText); j ++ )
			{
				if ( pwszOperator[j] != pszText[index + j] )
				{
					break ;
				}
			}
			if ( pwszOperator[j] == 0 )
			{
				m_index = index + j ;
				return	tokenPunctuation ;
			}
		}
	}
	else if ( wch == L'.' )
	{
		if ( pszText[index] == L'*' )
		{
			m_index = index + 1 ;
			return	tokenPunctuation ;
		}
	}
	else if ( wch == L'-' )
	{
		if ( pszText[index] == L'>' )
		{
			m_index = index + 1 ;
			return	tokenPunctuation ;
		}
	}
	return	SStringParser::PassToken() ;
}


//////////////////////////////////////////////////////////////////////////////
// スクリプト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSScript, RSParenthesis )

// スクリプト読み込み
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSScript::LoadScript
	( RSContext& ctxMacro,
		const wchar_t * pwszFilePath,
		SSystem::SParserErrorInterface& perr )
{
	RSSourceParser	sparsSource ;
	if ( sparsSource.LoadTextFile( pwszFilePath ) )
	{
		perr.OutputError
			( sparsSource, L"ソースファイルを読み込めませんでした" ) ;
		return	errFailed ;
	}
	SError	err = ParseSource( this, ctxMacro, 0, sparsSource, perr ) ;
	if ( !err )
	{
		m_strSource = sparsSource ;
		m_strSrcPath = pwszFilePath ;
	}
	return	err ;
}

// スクリプト解釈
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSScript::ParseScript
	( RSContext& ctxMacro,
		SStringParser& sparsSource, SParserErrorInterface& perr )
{
	SError	err = ParseSource( this, ctxMacro, 0, sparsSource, perr ) ;
	if ( !err )
	{
		m_strSource = sparsSource ;
	}
	return	err ;
}

SSystem::SError RSScript::ParseSource
	( RSContext& ctxMacro,
		SSystem::SString& strSource,
		SSystem::SParserErrorInterface& perr )
{
	RSSourceParser	sparsSource ;
	sparsSource.AttachString( strSource ) ;
	return	ParseScript( ctxMacro, sparsSource, perr ) ;
}

SSystem::SError RSScript::ParseSource
	( RSContext& ctxMacro,
		const wchar_t * pwszSource,
		SSystem::SParserErrorInterface& perr )
{
	RSSourceParser	sparsSource = pwszSource ;
	return	ParseScript( ctxMacro, sparsSource, perr ) ;
}

// スクリプト解釈（低水準）
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSScript::ParseSource
	( RSParenthesis * pprthTarget,
		RSContext& ctxMacro, uint32_t flagsParser,
		SSystem::SStringParser& sparsSource,
		SSystem::SParserErrorInterface& perr )
{
	RSParenthesis *	pprthRoot = pprthTarget ;
	bool	flagLineFirst = true ;
	bool	flagStatementHead = true ;
	SString	strLastComment ;
	while ( !sparsSource.IsIndexOverflow() )
	{
		//
		// 空白を読み飛ばす
		//
		wchar_t	wch = sparsSource.CurrentCharacter() ;
		if ( !(flagsParser & parserNoComment) )
		{
			if ( wch == L'/' )
			{
				// コメント判定
				wchar_t	wchNext = sparsSource.OffsetAt( 1 ) ;
				if ( wchNext == L'/' )
				{
					size_t	iComment = 2 ;
					while ( true )
					{
						wchar_t	wchTemp = sparsSource.OffsetAt( iComment ) ;
						if ( wchTemp == L'/' )
						{
							iComment ++ ;
						}
						else if ( wchTemp == L' ' )
						{
							iComment ++ ;
							break ;
						}
						else
						{
							break ;
						}
					}
					iComment += sparsSource.GetIndex() ;
					//
					sparsSource.SeekToNextLine() ;
					strLastComment += sparsSource.SubStringFrom( iComment ) ;
					if ( !flagLineFirst )
					{
						AddCodeComment
							( pprthTarget->m_terms.GetLastAt(0), strLastComment ) ;
						strLastComment = L"" ;
					}
					flagLineFirst = true ;
					continue ;
				}
				else if ( wchNext == L'*' )
				{
					size_t	iComment = 2 ;
					while ( sparsSource.OffsetAt( iComment ) == L'*' )
					{
						iComment ++ ;
					}
					iComment += sparsSource.GetIndex() ;
					sparsSource.SeekIndex( iComment ) ;
					if ( sparsSource.SeekString( L"*/" ) )
					{
						SString	strCurComment = sparsSource.SubStringFrom( iComment ) ;
						while ( strCurComment.GetLastAt(0) == L'*' )
						{
							strCurComment.ChopRight( 1 ) ;
						}
						strLastComment += strCurComment ;
						sparsSource.SeekIndex( sparsSource.GetIndex() + 2 ) ;
						//
						if ( !flagLineFirst )
						{
							AddCodeComment
								( pprthTarget->m_terms.GetLastAt(0), strLastComment ) ;
							strLastComment = L"" ;
						}
						continue ;
					}
					else
					{
						perr.OutputError( sparsSource, L"/* が */ で閉じられていません" ) ;
					}
					break ;
				}
			}
		}
		if ( wch <= L' ' )
		{
			if ( wch == L'\n' )
			{
				flagLineFirst = true ;
			}
			sparsSource.GetCharacter() ;
			continue ;
		}
		//
		// # ディレクティブ処理
		//
		if ( !(flagsParser & parserNoDirective)
				&& flagLineFirst && (wch == L'#') )
		{
			strLastComment = L"" ;
			//
			SString	strDir ;
			sparsSource.GetCharacter() ;
			sparsSource.NextToken( strDir ) ;
			//
			SString	strDirExpr ;
			for ( ; ; )
			{
				SString	strDirLine ;
				sparsSource.NextLine( strDirLine ) ;
				strDirLine.TrimRight() ;
				if ( strDirLine.GetLastAt(0) == L'\\' )
				{
					strDirExpr += strDirLine.Left( strDirLine.GetLength() ) ;
					strDirExpr += L" " ;
				}
				else
				{
					strDirExpr += strDirLine ;
					break ;
				}
			}
			RSSourceParser	sparsDir ;
			sparsDir.AttachString( strDirExpr ) ;
			//
			if ( strDir == L"if" )
			{
				// #if <expr>
				ParseDirectiveIf( ctxMacro, sparsDir, sparsSource, perr ) ;
			}
			else if ( (strDir == L"elif") || (strDir == L"elseif") )
			{
				// #elseif <expr> | #elif <expr>
				ParseDirectiveElseIf( ctxMacro, sparsDir, sparsSource, perr ) ;
			}
			else if ( strDir == L"else" )
			{
				// #else
				ParseDirectiveElse( sparsDir, sparsSource, perr ) ;
			}
			else if ( strDir == L"endif" )
			{
				// #endif
				ParseDirectiveEndIf( sparsDir, sparsSource, perr ) ;
			}
			else if ( strDir == L"define" )
			{
				// #define <name> <expr>
				ParseDirectiveDefine( ctxMacro, sparsDir, sparsSource, perr ) ;
			}
			else if ( strDir == L"import" )
			{
				// #import <path> [, <encoding>]
				ParseDirectiveImport
					( pprthTarget, ctxMacro, sparsDir, sparsSource, perr ) ;
			}
			else if ( strDir == L"importlib" )
			{
			}
			else
			{
				perr.OutputError
					( sparsSource, L"不正なディレクティブです" ) ;
			}
			continue ;
		}
		if ( !(flagsParser & parserNoDirective) )
		{
			if ( !IsEnabledDirectiveBlock() )
			{
				sparsSource.SeekToNextLine() ;
				flagLineFirst = true ;
				continue ;
			}
		}
		//
		// リテラル判定
		//
		RSCode *	pCode = NULL ;
		if ( wch == L'\"' )
		{
			// ダブルクォーテーションで囲まれた文字列リテラル
			SString	strLiteral ;
			sparsSource.GetCharacter() ;
			if ( sparsSource.NextEnclosedString
					( strLiteral, L'\"', 0 ) != L'\"' )
			{
				perr.OutputError( sparsSource, L"二重引用符が閉じられていません" ) ;
			}
			SString	strText ;
			SStringParser::DecodeCLangString( strText, strLiteral ) ;
			//
			pCode = new RSCodeLiteral
				( ctxMacro.new_String( strText ), sparsSource.GetIndex() ) ;
		}
		else if ( wch == L'\'' )
		{
			// シングルクォーテーションで囲まれた文字リテラル
			SString	strLiteral ;
			sparsSource.GetCharacter() ;
			if ( sparsSource.NextEnclosedString
				( strLiteral, L'\'', 0 ) != L'\'' )
			{
				perr.OutputError( sparsSource, L"引用符が閉じられていません" ) ;
			}
			SString	strText ;
			SStringParser::DecodeCLangString( strText, strLiteral ) ;
			//
			int64_t	numCode = 0 ;
			for ( size_t i = 0; i < strText.GetLength(); i ++ )
			{
				numCode = strText.GetAt(i) | (numCode << 16) ;
			}
			pCode = new RSCodeLiteral
				( ctxMacro.new_Integer( numCode ), sparsSource.GetIndex() ) ;
		}
		else if ( (wch >= L'0') && (wch <= L'9') )
		{
			int	typeNumber =
				sparsSource.IsNextNumber
					( SStringParser::ctrlCStyleNumber
						| SStringParser::ctrlNoRadixPostfix ) ;
			if ( typeNumber & SStringParser::numberFlagReal )
			{
				// 実数値リテラル
				double	num = sparsSource.NextRealNumber( typeNumber ) ;
				pCode = new RSCodeLiteral
					( ctxMacro.new_Number( num ), sparsSource.GetIndex() ) ;
			}
			else
			{
				// 整数値リテラル
				int64_t	num = sparsSource.NextInteger( typeNumber ) ;
				pCode = new RSCodeLiteral
					( ctxMacro.new_Integer( num ), sparsSource.GetIndex() ) ;
			}
		}
		else if ( wch == L'{' )
		{
			// 大括弧
			RSParenthesis *	prth =
				new RSParenthesis
					( pprthTarget,
						RSParenthesis::ptBrace, sparsSource.GetIndex() ) ;
			pprthTarget->m_terms.Add( prth ) ;
			pprthTarget = prth ;
			sparsSource.GetCharacter() ;
			flagStatementHead = true ;
		}
		else if ( wch == L'[' )
		{
			// 括弧
			RSParenthesis *	prth =
				new RSParenthesis
					( pprthTarget,
						RSParenthesis::ptBracket, sparsSource.GetIndex() ) ;
			pprthTarget->m_terms.Add( prth ) ;
			pprthTarget = prth ;
			sparsSource.GetCharacter() ;
		}
		else if ( wch == L'(' )
		{
			// 丸括弧
			RSParenthesis *	prth =
				new RSParenthesis
					( pprthTarget,
						RSParenthesis::ptParenthesis, sparsSource.GetIndex() ) ;
			pprthTarget->m_terms.Add( prth ) ;
			pprthTarget = prth ;
			sparsSource.GetCharacter() ;
		}
		else if ( (wch == L'}') || (wch == L']') || (wch == L')') )
		{
			// 閉じ括弧
			RSParenthesis::ParenthesisType	typeParenth ;
			if ( wch == L'}' )
			{
				typeParenth = RSParenthesis::ptBrace ;
			}
			else if ( wch == L']' )
			{
				typeParenth = RSParenthesis::ptBracket ;
			}
			else
			{
				typeParenth = RSParenthesis::ptParenthesis ;
			}
			sparsSource.GetCharacter() ;
			//
			if ( (pprthTarget->m_parent == NULL)
				|| (pprthTarget->m_parenthesis != typeParenth) )
			{
				if ( wch == L'}' )
				{
					perr.OutputError( sparsSource, L"\'}\' が \'{\' に対応していません" ) ;
				}
				else if ( wch == L']' )
				{
					perr.OutputError( sparsSource, L"\']\' が \'[\' に対応していません" ) ;
				}
				else
				{
					perr.OutputError( sparsSource, L"\')\' が \'(\' に対応していません" ) ;
				}
				RSParenthesis *	pprth = pprthTarget ;
				while ( pprth->m_parent != NULL )
				{
					pprth = pprth->m_parent ;
					if ( pprth->m_parenthesis == typeParenth )
					{
						pprthTarget = pprth ;
						break ;
					}
				}
			}
			if ( (pprthTarget->m_parent != NULL)
				&& (pprthTarget->m_parenthesis == typeParenth) )
			{
				pprthTarget = pprthTarget->m_parent ;
			}
		}
		else
		{
			//
			// 任意の語
			//
			if ( flagStatementHead && m_flagDebug )
			{
				pprthTarget->m_terms.Add
					( new RSCodeControl
						( RSCodeControl::wiDebugPoint, sparsSource.GetIndex() ) ) ;
			}
			SString	strToken ;
			size_t	iTokenIndex = sparsSource.GetIndex() ;
			sparsSource.NextToken( strToken ) ;
			//
			RSCodeControl::WordIndex
				wiIndex = RSCodeControl::IsControlWord( strToken ) ;
			if ( wiIndex != RSCodeControl::wiInvalid )
			{
				// 制御語
				pCode = new RSCodeControl( wiIndex, iTokenIndex ) ;
				flagStatementHead = false ;
			}
			else
			{
				flagStatementHead = false ;
				//
				RSCodeOperator::OperatorIndex
					opIndex = RSCodeOperator::IsOperatorWord( strToken ) ;
				if ( opIndex != RSCodeOperator::opInvalid )
				{
					// 演算子
					pCode = new RSCodeOperator( opIndex, iTokenIndex ) ;
					if ( opIndex == RSCodeOperator::opEndOfStatement )
					{
						flagStatementHead = true ;
					}
				}
				else
				{
					RSObject *	pObj = ctxMacro.GetVariableAs( strToken ) ;
					if ( pObj != NULL )
					{
						// マクロ定数
						pCode = new RSCodeLiteral
							( pObj->CloneObject( ctxMacro ), iTokenIndex ) ;
					}
					else
					{
						// 任意のシンボル
						pCode = new RSCodeSymbol( strToken, iTokenIndex ) ;
					}
				}
			}
		}
		if ( pCode != NULL )
		{
			if ( flagLineFirst
				&& (pCode->m_type != RSCode::typeOperator) )
			{
				RSCode *	pLastCode = pprthTarget->m_terms.GetLastAt() ;
				if ( (pLastCode != NULL)
					&& (pLastCode->m_type != RSCode::typeOperator) )
				{
					// 改行のタイミングで自動的に ; の挿入
					if ( ctxMacro.IsBehavior
						( RSContext::behaveAutoEndOfStatement ) )
					{
						pprthTarget->m_terms.Add
							( new RSCodeOperator
								( RSCodeOperator::opEndOfStatement,
												sparsSource.GetIndex() ) ) ;
						flagStatementHead = false ;
					}
				}
			}
			AddCodeComment( pCode, strLastComment ) ;
			strLastComment = L"" ;
			pprthTarget->m_terms.Add( pCode ) ;
			flagLineFirst = false ;
		}
	}
	if ( pprthRoot != pprthTarget )
	{
		perr.OutputError( sparsSource, L"括弧が閉じられていません" ) ;
	}
	return	errSuccess ;
}

// #if ディレクティブ
//////////////////////////////////////////////////////////////////////////////
void RSScript::ParseDirectiveIf
	( RSContext& ctxMacro,
		RSSourceParser& sparsDir,
		SSystem::SStringParser& sparsSource,
		SSystem::SParserErrorInterface& perr )
{
	bool	fCondition = false ;
	if ( IsEnabledDirectiveBlock() )
	{
		RSObject *	pObj =
				ctxMacro.EvaluateExpression( sparsDir, &perr ) ;
		if ( pObj != NULL )
		{
			fCondition = pObj->AsBoolean() ;
			ctxMacro.ReleaseObjectRef( pObj ) ;
		}
	}
	DirectiveNest *	pNest = new DirectiveNest ;
	m_nestDir.Push( pNest ) ;
	pNest->m_type = directiveIf ;
	pNest->m_condition =
			fCondition ? conditionTrue : conditionPending ;
}

// #elseif ディレクティブ
//////////////////////////////////////////////////////////////////////////////
void RSScript::ParseDirectiveElseIf
	( RSContext& ctxMacro,
		RSSourceParser& sparsDir,
		SSystem::SStringParser& sparsSource,
		SSystem::SParserErrorInterface& perr )
{
	DirectiveNest *	pNest = m_nestDir.Pop() ;
	if ( (pNest == NULL)
		|| ((pNest->m_type != directiveIf)
			&& (pNest->m_type != directiveElseIf)) )
	{
		perr.OutputError
			( sparsSource,
				L"#elseif に対応する #if がみつかりません" ) ;
	}
	else
	{
		if ( IsEnabledDirectiveBlock()
			&& (pNest->m_condition == conditionPending) )
		{
			bool	fCondition = false ;
			RSObject *	pObj =
				ctxMacro.EvaluateExpression( sparsDir, &perr ) ;
			if ( pObj != NULL )
			{
				if ( pObj->AsBoolean() )
				{
					pNest->m_condition = conditionTrue ;
				}
				ctxMacro.ReleaseObjectRef( pObj ) ;
			}
		}
		else if ( pNest->m_condition == conditionTrue )
		{
			pNest->m_condition = conditionElse ;
		}
		pNest->m_type = directiveElseIf ;
	}
	if ( pNest != NULL )
	{
		m_nestDir.Push( pNest ) ;
	}
}

// #else ディレクティブ
//////////////////////////////////////////////////////////////////////////////
void RSScript::ParseDirectiveElse
	( RSSourceParser& sparsDir,
		SSystem::SStringParser& sparsSource,
		SSystem::SParserErrorInterface& perr )
{
	DirectiveNest *	pNest = m_nestDir.Pop() ;
	if ( (pNest == NULL)
		|| ((pNest->m_type != directiveIf)
			&& (pNest->m_type != directiveElseIf)) )
	{
		perr.OutputError
			( sparsSource,
				L"#else に対応する #if がみつかりません" ) ;
	}
	else
	{
		if ( pNest->m_condition == conditionPending )
		{
			pNest->m_condition = conditionTrue ;
		}
		else
		{
			pNest->m_condition = conditionElse ;
		}
		if ( sparsDir.PassSpace() )
		{
			perr.OutputError( sparsSource, L"構文エラーです" ) ;
		}
	}
	if ( pNest != NULL )
	{
		m_nestDir.Push( pNest ) ;
	}
}

// #endif ディレクティブ
//////////////////////////////////////////////////////////////////////////////
void RSScript::ParseDirectiveEndIf
	( RSSourceParser& sparsDir,
		SSystem::SStringParser& sparsSource,
		SSystem::SParserErrorInterface& perr )
{
	DirectiveNest *	pNest = m_nestDir.Pop() ;
	if ( (pNest == NULL)
		|| ((pNest->m_type != directiveIf)
			&& (pNest->m_type != directiveElseIf)
			&& (pNest->m_type != directiveElse)
			&& (pNest->m_type != directiveEndIf)) )
	{
		perr.OutputError
			( sparsSource,
				L"#endif に対応する #if がみつかりません" ) ;
		//
		if ( pNest != NULL )
		{
			m_nestDir.Push( pNest ) ;
		}
	}
	else
	{
		if ( sparsDir.PassSpace() )
		{
			perr.OutputError( sparsSource, L"構文エラーです" ) ;
		}
		delete	pNest ;
	}
}

// #define ディレクティブ
//////////////////////////////////////////////////////////////////////////////
void RSScript::ParseDirectiveDefine
	( RSContext& ctxMacro,
		RSSourceParser& sparsDir,
		SSystem::SStringParser& sparsSource,
		SSystem::SParserErrorInterface& perr )
{
	//
	// #define <name> <expr>
	//
	SString	strMacroName ;
	if ( sparsDir.NextToken( strMacroName )
					!= SStringParser::tokenNormal )
	{
		perr.OutputError
			( sparsSource, L"マクロ変数名が不正です" ) ;
	}
	else
	{
		RSObject *	pObj =
			ctxMacro.EvaluateExpression( sparsDir, &perr ) ;
		if ( pObj != NULL )
		{
			ctxMacro.CreateVariableAs( strMacroName, pObj ) ;
			ctxMacro.OutputExceptionError( perr ) ;
		}
	}
}

// #import ディレクティブ
//////////////////////////////////////////////////////////////////////////////
void RSScript::ParseDirectiveImport
	( RSParenthesis * pprthTarget,
		RSContext& ctxMacro,
		RSSourceParser& sparsDir,
		SSystem::SStringParser& sparsSource,
		SSystem::SParserErrorInterface& perr )
{
	//
	// #import <path> [, <encoding>]
	//
	wchar_t	wchClose = sparsDir.HasToComeChar( L"<\"\'" ) ;
	if ( wchClose == L'<' )
	{
		wchClose = L'>' ;
	}
	SString	strSrcPath ;
	if ( (wchClose != 0)
		&& (sparsDir.NextEnclosedString
						( strSrcPath, wchClose ) == wchClose) )
	{
		SSmartPointer<SFileInterface>	pFile =
			ctxMacro.GetVM()->OpenScriptFile( strSrcPath ) ;
		if ( pFile == NULL )
		{
			perr.OutputError
				( sparsSource, SString(L"\'")
								+ strSrcPath + L"\' を開けません" ) ;
			return ;
		}
		Charset::EncodingType	encoding = Charset::encodingUnknown ;
		if ( sparsDir.HasToComeChar( L"," ) == L',' )
		{
			SString	strEncoding = sparsDir.GetString() ;
			encoding = Charset::GetEncodingType( strEncoding ) ;
		}
		if ( sparsDir.PassSpace() )
		{
			perr.OutputError( sparsSource, L"構文エラーです" ) ;
		}
		RSSourceParser	sparsImport ;
		if ( sparsImport.ReadTextFile( *pFile, encoding ) )
		{
			perr.OutputError
				( sparsSource, L"ファイルを読み込めませんでした" ) ;
			return ;
		}
		//
		RSScript *	pScript = new RSScript ;
		pScript->m_parent = pprthTarget ;
		pprthTarget->m_terms.Add( pScript ) ;
		pScript->SetSourcePath( strSrcPath ) ;
		//
		ParseSource
			( pScript, ctxMacro, 0, sparsImport, perr ) ;
	}
	else
	{
		perr.OutputError( sparsSource, L"構文エラーです" ) ;
	}
}

// #importlib ディレクティブ
//////////////////////////////////////////////////////////////////////////////
void RSScript::ParseDirectiveImportLib
	( RSSourceParser& sparsDir,
		SSystem::SStringParser& sparsSource,
		SSystem::SParserErrorInterface& perr )
{
	//
	// #importlib <path> [, [<type>] [, <encoding>]]
	//
	wchar_t	wchClose = sparsDir.HasToComeChar( L"<\"\'" ) ;
	if ( wchClose == L'<' )
	{
		wchClose = L'>' ;
	}
	SString	strSrcPath ;
	if ( (wchClose != 0)
		&& (sparsDir.NextEnclosedString
						( strSrcPath, wchClose ) == wchClose) )
	{
		if ( m_aImportLibs.GetAs( strSrcPath ) == NULL )
		{
			ImportLibrary *	pil = new ImportLibrary ;
			pil->m_strPath = strSrcPath ;
			m_aImportLibs.SetAs( strSrcPath, pil ) ;
			//
			if ( sparsDir.HasToComeChar( L"," ) == L',' )
			{
				SString	strType = sparsDir.GetStringTerm() ;
				if ( strType != "," )
				{
					pil->m_strType = strType ;
				}
				else if ( sparsDir.HasToComeChar( L"," ) == L',' )
				{
					pil->m_strEncoding = sparsDir.GetStringTerm() ;
				}
			}
			if ( sparsDir.PassSpace() )
			{
				perr.OutputError( sparsSource, L"構文エラーです" ) ;
			}
		}
	}
	else
	{
		perr.OutputError( sparsSource, L"構文エラーです" ) ;
	}
}

// コメント追加
//////////////////////////////////////////////////////////////////////////////
RSCodeComment *
	RSScript::AddCodeComment( RSCode * pCode, SSystem::SString& strComment )
{
	if ( (pCode == NULL) || strComment.IsEmpty() )
	{
		return	NULL ;
	}
	RSCodeComment *	pComment = new RSCodeComment( strComment ) ;
	m_arrCommentStock.Add( pComment ) ;
	strComment = L"" ;
	pCode->m_pComment = pComment ;
	return	pComment ;
}

// ディレクティブ条件判定
//////////////////////////////////////////////////////////////////////////////
bool RSScript::IsEnabledDirectiveBlock( void ) const
{
	const size_t			nCount = m_nestDir.GetLength() ;
	DirectiveNest*const*	ppNest = m_nestDir.GetConstArray() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		DirectiveNest *	pNest = ppNest[i] ;
		ESLAssert( pNest != NULL ) ;
		if ( pNest->m_condition != conditionTrue )
		{
			return	false ;
		}
	}
	return	true ;
}

// コードのソース取得
//////////////////////////////////////////////////////////////////////////////
RSScript * RSScript::GetScriptOf( RSParenthesis * pPrth )
{
	RSScript *	pScript = ESLTypeCast<RSScript>( pPrth ) ;
	while ( (pScript == NULL) && (pPrth->m_parent != NULL) )
	{
		pPrth = pPrth->m_parent ;
		pScript = ESLTypeCast<RSScript>( pPrth ) ;
	}
	return	pScript ;
}



//////////////////////////////////////////////////////////////////////////////
// 関数プロトタイプ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSFunctionPrototype, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSFunctionPrototype::RSFunctionPrototype( void )
{
	m_nFlags = 0 ;
	m_flagCompiled = false ;
	m_pReturnType = NULL ;
	m_pRefNamespace = NULL ;
	m_pParenthesis = NULL ;
	m_pfnFuncAddr = -1 ;
	m_methodNative.pfnMethod = NULL ;
	m_methodNative.pInstance = NULL ;
	m_pFuncGroup = NULL ;
	m_pNamespaceClass = NULL ;
	m_pComment = NULL ;
}

RSFunctionPrototype::RSFunctionPrototype( const RSFunctionPrototype& proto )
	: m_nFlags( proto.m_nFlags ),
		m_flagCompiled( proto.m_flagCompiled ),
		m_aArgTypes( proto.m_aArgTypes ), 
		m_aArgNames( proto.m_aArgNames ),
		m_aArgDefault( proto.m_aArgDefault ),
		m_pReturnType( proto.m_pReturnType ),
		m_pRefNamespace( proto.m_pRefNamespace ),
		m_pParenthesis( proto.m_pParenthesis ),
		m_pfnFuncAddr( proto.m_pfnFuncAddr ),
		m_methodNative( proto.m_methodNative ),
		m_pFuncGroup( proto.m_pFuncGroup ),
		m_pNamespaceClass( proto.m_pNamespaceClass ),
		m_pComment( proto.m_pComment )
{
	size_t	i ;
	for ( i = 0; i < m_aArgDefault.GetLength(); i ++ )
	{
		RSObject::AddRef( m_aArgDefault.GetAt(i) ) ;
	}
	RSObject::AddRef( m_pRefNamespace ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
RSFunctionPrototype::~RSFunctionPrototype( void )
{
	RSObject::ReleaseRef( m_pRefNamespace ) ;
	m_pRefNamespace = NULL ;
	//
	for ( size_t i = 0; i < m_aArgDefault.GetLength(); i ++ )
	{
		RSObject::ReleaseRef( m_aArgDefault.GetAt(i) ) ;
	}
}

// 参照チェーンを解放
//////////////////////////////////////////////////////////////////////////////
void RSFunctionPrototype::ReleaseReferenceChain( void )
{
	RSObject::ReleaseRef( m_pRefNamespace ) ;
	m_pRefNamespace = NULL ;
}

// 名前空間設定
//////////////////////////////////////////////////////////////////////////////
void RSFunctionPrototype::SetRefNamespace( RSObject * pObj )
{
	RSObject::ReleaseRef( m_pRefNamespace ) ;
	m_pRefNamespace = pObj ;
}

// 返り値型設定
//////////////////////////////////////////////////////////////////////////////
void RSFunctionPrototype::SetReturnType( RSClass * pClass )
{
	m_pReturnType = pClass ;
}

// 引数追加
//////////////////////////////////////////////////////////////////////////////
void RSFunctionPrototype::AddArgument
	( RSClass * pClass,
		const wchar_t * pwszName, RSObject * pDefault )
{
	size_t	iArg = m_aArgTypes.GetLength() ;
	m_aArgTypes.SetAt( iArg, pClass ) ;
	m_aArgNames.SetAt( iArg, new SString( pwszName ) ) ;
	m_aArgDefault.SetAt( iArg, pDefault ) ;
}

// 書式解釈  ( [<type-expr>] <name> [= <init-expr>], ... )
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSFunctionPrototype::ParseArgument
	( RSContext& context,
		RSCodeStream& cs, SSystem::SParserErrorInterface& perr )
{
	m_aArgTypes.RemoveAll() ;
	m_aArgNames.RemoveAll() ;
	//
	if ( cs.NextControlWord( RSCodeControl::wiVoid ) != NULL )
	{
		if ( !cs.IsEndOfStream() )
		{
			SStringParser	ssTemp ;
			perr.OutputError( ssTemp, L"void 指定の書式が不正です" ) ;
			return	errFailed ;
		}
		return	errSuccess ;
	}
	size_t	iArg = 0 ;
	while ( !cs.IsEndOfStream() )
	{
		if ( cs.NextSymbol( L"..." ) != NULL )
		{
			if ( !cs.IsEndOfStream() )
			{
				SStringParser	ssTemp ;
				perr.OutputError( ssTemp, L"引数が ... で終わっていません" ) ;
				return	errFailed ;
			}
			m_nFlags |= flagVarArg ;
			break ;
		}
		RSClass *		pClass = context.ParseClassExpression( cs ) ;
		RSCodeSymbol *	pSymName = cs.NextSymbol() ;
		RSObject *		pObjInit = NULL ;
		if ( pSymName == NULL )
		{
			SStringParser	ssTemp ;
			perr.OutputError( ssTemp, L"引数名が指定されていません" ) ;
			return	errFailed ;
		}
		if ( cs.NextOperator( RSCodeOperator::opMove ) != NULL )
		{
			pObjInit = context.EvaluateExpression
							( cs, RSCodeOperator::priorityList ) ;
			context.OutputExceptionError( perr ) ;
			//
			context.ReleaseObjectRef
				( m_aArgDefault.ExchangeAt( iArg, pObjInit ) ) ;
		}
		if ( !cs.IsEndOfStream()
			&& (cs.NextOperator( RSCodeOperator::opSequencing ) == NULL) )
		{
			SStringParser	ssTemp ;
			perr.OutputError( ssTemp, L"引数が \',\' で区切られていません" ) ;
			return	errFailed ;
		}
		m_aArgTypes.SetAt( iArg, pClass ) ;
		m_aArgNames.SetAt( iArg, new SString(pSymName->m_symbol) ) ;
		iArg ++ ;
	}
	return	errSuccess ;
}

SSystem::SError RSFunctionPrototype::ParseArgument
	( RSContext& context,
		const wchar_t * pwszArgList,
		SSystem::SParserErrorInterface& perr )
{
	m_aArgTypes.RemoveAll() ;
	m_aArgNames.RemoveAll() ;
	//
	if ( pwszArgList == NULL )
	{
		return	errSuccess ;
	}
	RSScript		script ;
	RSSourceParser	sparsArg = pwszArgList ;
	SError	err = script.ParseScript
		( *(context.GetVM()->LockMacroContext()), sparsArg, perr ) ;
	context.GetVM()->UnlockMacroContext() ;
	if ( err )
	{
		return	err ;
	}
	RSCodeStream	cs( script ) ;
	return	ParseArgument( context, cs, perr ) ;
}

// 引数型一致判定
//////////////////////////////////////////////////////////////////////////////
bool RSFunctionPrototype::IsEqualArgumentTypes( const RSFunctionPrototype& proto ) const
{
	size_t	nArgs = m_aArgTypes.GetLength() ;
	if ( nArgs != proto.m_aArgTypes.GetLength() )
	{
		return	false ;
	}
	if ( (m_nFlags & flagArgTypeMask) != (proto.m_nFlags & flagArgTypeMask) )
	{
		return	false ;
	}
	for ( size_t i = 0; i < nArgs; i ++ )
	{
		if ( m_aArgTypes.GetAt(i) != proto.m_aArgTypes.GetAt(i) )
		{
			return	false ;
		}
	}
	return	true ;
}

// 関数呼び出し引数判定
//////////////////////////////////////////////////////////////////////////////
bool RSFunctionPrototype::IsMatchPrototype( RSObject*const* ppArgs, size_t countArg )
{
	size_t	nProtoArgs = m_aArgTypes.GetLength() ;
	if ( (nProtoArgs < countArg) && !(m_nFlags & flagVarArg) )
	{
		return	false ;
	}
	if ( nProtoArgs > countArg )
	{
		size_t	nDefArgs = m_aArgDefault.GetLength() ;
		if ( nDefArgs < nProtoArgs )
		{
			return	false ;
		}
		for ( size_t i = countArg; i < nProtoArgs; i ++ )
		{
			if ( m_aArgDefault.GetAt( i ) == NULL )
			{
				return	false ;
			}
		}
	}
	for ( size_t i = 0; i < countArg; i ++ )
	{
		RSClass *	pArgType = m_aArgTypes.GetAt( i ) ;
		if ( pArgType == NULL )
		{
			if ( (i < m_aArgTypes.GetLength()) || (m_nFlags & flagVarArg) )
			{
				continue ;
			}
			return	false ;
		}
		if ( !pArgType->TestCastInstance( ppArgs[i] ) )
		{
			return	false ;
		}
	}
	return	true ;
}

// 同一（実装）関数判定
//////////////////////////////////////////////////////////////////////////////
bool RSFunctionPrototype::IsEqualImplementFunc( const RSFunctionPrototype& proto ) const
{
	return	(m_pParenthesis == proto.m_pParenthesis)
			&& (m_pfnFuncAddr == proto.m_pfnFuncAddr)
			&& (m_methodNative.pfnMethod == proto.m_methodNative.pfnMethod)
			&& (m_methodNative.pInstance == proto.m_methodNative.pInstance) ;
}

// 引数文字列表記
//////////////////////////////////////////////////////////////////////////////
SSystem::SString RSFunctionPrototype::FormatArgument( void ) const
{
	SString	strArgList ;
	bool	flagInitVal = false ;
	for ( size_t i = 0; i < m_aArgTypes.GetLength(); i ++ )
	{
		if ( i > 0 )
		{
			strArgList += L", " ;
		}
		RSClass *	pArgType = m_aArgTypes.GetAt( i ) ;
		SString *	pArgName = m_aArgNames.GetAt( i ) ;
		RSObject *	pArgDef = m_aArgDefault.GetAt( i ) ;
		if ( pArgType != NULL )
		{
			strArgList += pArgType->GetFullClassName() ;
			if ( pArgName != NULL )
			{
				strArgList += L" " ;
				strArgList += *pArgName ;
			}
		}
		else
		{
			if ( pArgName != NULL )
			{
				strArgList += *pArgName ;
			}
		}
		if ( pArgDef != NULL )
		{
			SString	strValue = RSClass::FormatInitValue( pArgDef ) ;
			if ( !strValue.IsEmpty() )
			{
				strArgList += L" = " ;
				strArgList += strValue ;
				flagInitVal = true ;
			}
		}
	}
	if ( m_nFlags & flagVarArg )
	{
		if ( !strArgList.IsEmpty() )
		{
			strArgList += L", ..." ;
		}
		else
		{
			strArgList += L"..." ;
		}
	}
	return	strArgList ;
}


//////////////////////////////////////////////////////////////////////////////
// 関数オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSFunctionObject, RSDynamicObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSFunctionObject::RSFunctionObject( RSClass * pClass )
	: RSDynamicObject( pClass, typeFunction ),
		m_pFuncClass( NULL ), m_pPrototype( NULL )
{
}

RSFunctionObject::RSFunctionObject
		( RSFunctionPrototype * pProto, RSClass * pClass )
	: RSDynamicObject( pClass, typeFunction ),
		m_pFuncClass( NULL ), m_pPrototype( pProto )
{
	ESLAssert( pProto != NULL ) ;
	m_arrPrototypes.Add( pProto ) ;
}

RSFunctionObject::RSFunctionObject
		( RSContext& context, const RSFunctionObject& func )
	: RSDynamicObject( context, func ),
		m_pFuncClass( func.m_pFuncClass ),
		m_strFuncName( func.m_strFuncName ),
		m_arrPrototypes( func.m_arrPrototypes )
{
	m_pPrototype = m_arrPrototypes.GetAt(0) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
RSFunctionObject::~RSFunctionObject( void )
{
	ReleaseReferenceChain() ;
}

// プロトタイプ追加
//////////////////////////////////////////////////////////////////////////////
size_t RSFunctionObject::AddPrototype
	( RSFunctionPrototype * pProto, bool fOverride )
{
	pProto->m_pFuncGroup = this ;
	//
	// 関数オーバーライド
	//
	ssize_t	iOverride = FindEqualArgumentPrototype( *pProto ) ;
	if ( iOverride >= 0 )
	{
		if ( fOverride )
		{
			if ( m_pPrototype == m_arrPrototypes.GetAt( (size_t) iOverride ) )
			{
				m_pPrototype = NULL ;
			}
			m_arrPrototypes.SetAt( (size_t) iOverride, pProto ) ;
		}
		else
		{
			delete	pProto ;
		}
	}
	else
	{
		iOverride = 0 ;
		while ( (size_t) iOverride < m_arrPrototypes.GetLength() )
		{
			RSFunctionPrototype *
				pfp = m_arrPrototypes.GetAt( (size_t) iOverride ) ;
			if ( (pfp == nullptr)
				|| (pfp->m_pNamespaceClass != pProto->m_pNamespaceClass) )
			{
				break ;
			}
			iOverride ++ ;
		}
		m_arrPrototypes.InsertAt( (size_t) iOverride, pProto ) ;
	}
	//
	// デフォルト関数
	//
	if ( m_pPrototype == NULL )
	{
		m_pPrototype = pProto ;
	}
	return	(size_t) iOverride ;
}

// 一致プロトタイプ検索
//////////////////////////////////////////////////////////////////////////////
RSFunctionPrototype *
	RSFunctionObject::GetEqualArgumentPrototype
					( const RSFunctionPrototype & proto ) const
{
	return	m_arrPrototypes.GetAt
				( (size_t) FindEqualArgumentPrototype( proto ) ) ;
}

ssize_t RSFunctionObject::FindEqualArgumentPrototype
					( const RSFunctionPrototype & proto ) const
{
	size_t	nCount = m_arrPrototypes.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		RSFunctionPrototype *	pfp = m_arrPrototypes.GetAt( i ) ;
		ESLAssert( pfp != NULL ) ;
		if ( pfp->IsEqualArgumentTypes( proto ) )
		{
			return	(ssize_t) i ;
		}
	}
	return	-1 ;
}

// 適合プロトタイプ取得
//////////////////////////////////////////////////////////////////////////////
RSFunctionPrototype *
	RSFunctionObject::GetMatchPrototype( RSObject*const* ppArgs, size_t countArg )
{
	size_t	nPrototypes = m_arrPrototypes.GetLength() ;
	for ( size_t i = 0; i < nPrototypes; i ++ )
	{
		RSFunctionPrototype *	pProto = m_arrPrototypes.GetAt( i ) ;
		if ( pProto != NULL )
		{
			if ( pProto->IsMatchPrototype( ppArgs, countArg ) )
			{
				return	pProto ;
			}
		}
	}
	return	NULL ;
}

// 参照チェーンを解放
//////////////////////////////////////////////////////////////////////////////
void RSFunctionObject::ReleaseReferenceChain( void )
{
	for ( size_t i = 0; i < m_arrPrototypes.GetLength(); i ++ )
	{
		RSFunctionPrototype *	pProto = m_arrPrototypes.GetAt( i ) ;
		if ( pProto != NULL )
		{
			pProto->ReleaseReferenceChain() ;
		}
	}
}

// 実装一致（オーバーライドされていないか？）判定
//////////////////////////////////////////////////////////////////////////////
bool RSFunctionObject::IsEqualImplementFunc( const RSFunctionObject& func ) const
{
	if ( m_arrPrototypes.GetLength() != func.m_arrPrototypes.GetLength() )
	{
		return	false ;
	}
	for ( size_t i = 0; i < m_arrPrototypes.GetLength(); i ++ )
	{
		RSFunctionPrototype *	pProto1 = m_arrPrototypes.GetAt( i ) ;
		RSFunctionPrototype *	pProto2 = func.m_arrPrototypes.GetAt( i ) ;
		if ( (pProto1 != NULL) && (pProto2 != NULL) )
		{
			if ( !pProto1->IsEqualImplementFunc( *pProto2 ) )
			{
				return	false ;
			}
		}
	}
	return	true ;
}

bool RSFunctionObject::IsEqualImplementFunc( const RSFunctionPrototype& proto ) const
{
	RSFunctionPrototype *	pProto = GetEqualArgumentPrototype( proto ) ;
	if ( pProto == NULL )
	{
		return	false ;
	}
	return	proto.IsEqualImplementFunc( *pProto ) ;
}

// 型名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * RSFunctionObject::GetTypeName( void ) const
{
	return	L"Function" ;
}

// 内部リソース解放
//////////////////////////////////////////////////////////////////////////////
void RSFunctionObject::DisposeObject( RSContext& context )
{
	RSDynamicObject::DisposeObject( context ) ;
	//
	ReleaseReferenceChain() ;
	m_arrPrototypes.RemoveAll() ;
	m_pPrototype = NULL ;
}

// 複製（参照の複製を含む）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSFunctionObject::DuplicateObject( RSContext& context ) const
{
	RSObject *	pObj = new RSFunctionObject( context, *this ) ;
	return	pObj ;
}

// 複製（実体も可能な限り複製）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSFunctionObject::CloneObject( RSContext& context ) const
{
	RSObject *	pObj = new RSFunctionObject( context, *this ) ;
	return	pObj ;
}


//////////////////////////////////////////////////////////////////////////////
// テンポラルなコード（単一の数式）
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSExpressionScript, RSScript )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSExpressionScript::RSExpressionScript( void )
{
	m_pObj = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
RSExpressionScript::~RSExpressionScript( void )
{
	RSObject::ReleaseRef( m_pObj ) ;
	m_pObj = NULL ;
}

// コードの解釈と実行
//////////////////////////////////////////////////////////////////////////////
SError RSExpressionScript::Execute
	( RSVirtualMachine * pVM,
		const wchar_t * pwszExpr,
		SSystem::SParserErrorInterface& perr )
{
	RSContext *		pContext = pVM->LockMacroContext() ;
	RSSourceParser	sparsExpr = pwszExpr ;
	SError	err = ParseScript( *pContext, sparsExpr, perr ) ;
	pVM->UnlockMacroContext() ;
	if ( err )
	{
		return	err ;
	}
	RSContext		context( pVM ) ;
	RSCodeStream	cs( *this ) ;
	pVM->AddRef() ;
	context.PushNamespace( NULL, pVM, RSObject::modifierPublic, true ) ;
	m_pObj = context.EvaluateExpression( cs ) ;
	context.PopNamespace() ;
	if ( context.IsException() )
	{
		context.OutputExceptionError( perr ) ;
		return	errFailed ;
	}
	return	errSuccess ;
}


//////////////////////////////////////////////////////////////////////////////
// コードストリーム
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSCodeStream, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSCodeStream::RSCodeStream( void )
{
	m_prenthesis = NULL ;
	m_terms = NULL ;
	m_index = 0 ;
	m_start = 0 ;
	m_end = 0 ;
}

RSCodeStream::RSCodeStream( const RSCodeStream& cs )
	: m_prenthesis( cs.m_prenthesis ),
		m_terms( cs.m_terms ), m_index( cs.m_index ),
		m_start( cs.m_start ), m_end( cs.m_end )
{
}

RSCodeStream::RSCodeStream
	( const RSParenthesis& prth, size_t iFirst, ssize_t iEnd )
	: m_prenthesis( &prth ),
		m_terms( &(prth.m_terms) ), m_index( 0 ),
		m_start( iFirst ), m_end( (size_t) iEnd )
{
	if ( iEnd < 0 )
	{
		m_end = prth.m_terms.GetLength() ;
	}
}

RSCodeStream::RSCodeStream
	( const SObjectArray<RSCode> * terms,
							size_t iFirst, ssize_t iEnd )
	: m_prenthesis( NULL ),
		m_terms( terms ), m_index( 0 ),
		m_start( iFirst ), m_end( (size_t) iEnd )
{
	if ( (terms != NULL) && (iEnd < 0) )
	{
		m_end = terms->GetLength() ;
	}
	else if ( terms == NULL )
	{
		m_end = 0 ;
	}
}

// 関連付け
//////////////////////////////////////////////////////////////////////////////
void RSCodeStream::AttachCode
	( const RSCodeStream& cs, size_t iFirst, ssize_t iEnd )
{
	m_prenthesis = cs.m_prenthesis ;
	m_terms = cs.m_terms ;
	m_index = 0 ;
	m_start = cs.m_start + iFirst ;
	m_end = cs.m_end ;
	if ( (iEnd >= 0) && (cs.m_start + iEnd < cs.m_end) )
	{
		m_end = cs.m_start + iEnd ;
	}
}

void RSCodeStream::AttachCode
	( const RSParenthesis& prth, size_t iFirst, ssize_t iEnd )
{
	m_prenthesis = &prth ;
	m_terms = &(prth.m_terms) ;
	m_index = 0 ;
	m_start = iFirst ;
	m_end = (size_t) iEnd ;
	if ( iEnd < 0 )
	{
		m_end = prth.m_terms.GetLength() ;
	}
}

void RSCodeStream::AttachCode
	( const SObjectArray<RSCode> * terms, size_t iFirst, ssize_t iEnd )
{
	m_prenthesis = NULL ;
	m_terms = terms ;
	m_index = 0 ;
	m_start = iFirst ;
	m_end = (size_t) iEnd ;
	if ( (terms != NULL) && (iEnd < 0) )
	{
		m_end = terms->GetLength() ;
	}
	else if ( terms == NULL )
	{
		m_end = 0 ;
	}
}

// 関連付けられた括弧を取得
//////////////////////////////////////////////////////////////////////////////
const RSParenthesis * RSCodeStream::GetParenthesis( void ) const
{
	return	m_prenthesis ;
}

// 終端に到達しているか？
//////////////////////////////////////////////////////////////////////////////
bool RSCodeStream::IsEndOfStream( void ) const
{
	return	(m_terms == NULL) || (m_index + m_start >= m_end)
					|| (m_index + m_start >= m_terms->GetLength()) ;
}

// 指標
//////////////////////////////////////////////////////////////////////////////
size_t RSCodeStream::GetIndex( void ) const
{
	return	m_index ;
}

void RSCodeStream::SeekIndex( size_t iIndex )
{
	m_index = iIndex ;
}

// 項取得
//////////////////////////////////////////////////////////////////////////////
RSCode * RSCodeStream::GetTerm( size_t iOffset )
{
	for ( ; ; )
	{
		size_t	i = m_index + iOffset + m_start ;
		if ( (m_terms == NULL) || (i >= m_end) )
		{
			break ;
		}
		RSCode *	pCode = m_terms->GetAt( i ) ;
		if ( pCode == NULL )
		{
			break ;
		}
		if ( (iOffset == 0)
			&& (pCode->m_type == RSCode::typeControlCode) )
		{
			ESLAssert( pCode->IsKindOf( ESL_RUNTIME_CLASS(RSCodeControl) ) ) ;
			if ( ((RSCodeControl*)pCode)->m_word == RSCodeControl::wiDebugPoint )
			{
				m_index ++ ;
				continue ;
			}
		}
		return	pCode ;
	}
	return	NULL ;
}

// 項取得と指標の移動
//////////////////////////////////////////////////////////////////////////////
RSCode * RSCodeStream::NextTerm( size_t iOffset )
{
	if ( (m_terms != NULL) && (m_index + iOffset + m_start < m_end) )
	{
		m_index += iOffset ;
		return	m_terms->GetAt( m_start + (m_index ++) ) ;
	}
	return	NULL ;
}

// 次の項がリテラルの場合、それを返し指標を次に移動する
//////////////////////////////////////////////////////////////////////////////
RSCodeLiteral * RSCodeStream::NextLiteral( void )
{
	RSCode *	pCode = GetTerm() ;
	if ( pCode != NULL )
	{
		if ( pCode->m_type == RSCode::typeLiteral )
		{
			ESLAssert( pCode->IsKindOf( ESL_RUNTIME_CLASS(RSCodeLiteral) ) ) ;
			m_index ++ ;
			return	(RSCodeLiteral*) pCode ;
		}
	}
	return	NULL ;
}

// 次の項が括弧の場合、それを返し指標を次に移動する
//////////////////////////////////////////////////////////////////////////////
RSParenthesis * RSCodeStream::NextParenthesis
				( RSParenthesis::ParenthesisType type )
{
	RSCode *	pCode = GetTerm() ;
	if ( pCode != NULL )
	{
		if ( pCode->m_type == RSCode::typeParenthesis )
		{
			ESLAssert( pCode->IsKindOf( ESL_RUNTIME_CLASS(RSParenthesis) ) ) ;
			RSParenthesis *	pPrth = (RSParenthesis*) pCode ;
			if ( (type == RSParenthesis::ptInvalid)
				|| (pPrth->m_parenthesis == type) )
			{
				m_index ++ ;
				return	pPrth ;
			}
		}
	}
	return	NULL ;
}

// 次の項が演算子の場合、それを返し指標を次に移動する
//////////////////////////////////////////////////////////////////////////////
RSCodeOperator * RSCodeStream::NextOperator
		( RSCodeOperator::OperatorIndex opIndex )
{
	RSCode *	pCode = GetTerm() ;
	if ( pCode != NULL )
	{
		if ( pCode->m_type == RSCode::typeOperator )
		{
			ESLAssert( pCode->IsKindOf( ESL_RUNTIME_CLASS(RSCodeOperator) ) ) ;
			RSCodeOperator *	pOpCode = (RSCodeOperator*) pCode ;
			if ( (opIndex == RSCodeOperator::opInvalid)
				|| (pOpCode->m_operator == opIndex) )
			{
				m_index ++ ;
				return	pOpCode ;
			}
		}
	}
	return	NULL ;
}

// 次の項が制御語の場合、それを返し指標を次に移動する
//////////////////////////////////////////////////////////////////////////////
RSCodeControl * RSCodeStream::NextControlWord
						( RSCodeControl::WordIndex wiIndex )
{
	RSCode *	pCode = GetTerm() ;
	if ( pCode != NULL )
	{
		if ( pCode->m_type == RSCode::typeControlCode )
		{
			ESLAssert( pCode->IsKindOf( ESL_RUNTIME_CLASS(RSCodeControl) ) ) ;
			RSCodeControl *	pCtrlCode = (RSCodeControl*) pCode ;
			if ( (wiIndex == RSCodeControl::wiInvalid)
				|| (pCtrlCode->m_word == wiIndex) )
			{
				m_index ++ ;
				return	pCtrlCode ;
			}
		}
	}
	return	NULL ;
}

RSCodeControl * RSCodeStream::NextStatementControlWord( void )
{
	RSCode *	pCode = m_terms->GetAt( m_index + m_start ) ;
	if ( pCode != NULL )
	{
		if ( pCode->m_type == RSCode::typeControlCode )
		{
			ESLAssert( pCode->IsKindOf( ESL_RUNTIME_CLASS(RSCodeControl) ) ) ;
			RSCodeControl *	pCtrlCode = (RSCodeControl*) pCode ;
			m_index ++ ;
			return	pCtrlCode ;
		}
	}
	return	NULL ;
}

// 次の項がシンボルの場合、それを返し指標を次に移動する
//////////////////////////////////////////////////////////////////////////////
RSCodeSymbol * RSCodeStream::NextSymbol( const wchar_t * pwszSymbol )
{
	RSCode *	pCode = GetTerm() ;
	if ( pCode != NULL )
	{
		if ( pCode->m_type == RSCode::typeSymbol )
		{
			ESLAssert( pCode->IsKindOf( ESL_RUNTIME_CLASS(RSCodeSymbol) ) ) ;
			if ( pwszSymbol != NULL )
			{
				if ( ((RSCodeSymbol*)pCode)->m_symbol != pwszSymbol )
				{
					return	NULL ;
				}
			}
			m_index ++ ;
			return	(RSCodeSymbol*) pCode ;
		}
	}
	return	NULL ;
}

// 括弧を文末まで検索する
//////////////////////////////////////////////////////////////////////////////
RSParenthesis * RSCodeStream::FindParenthesis
			( RSParenthesis::ParenthesisType type )
{
	const size_t	iSave = m_index ;
	RSCode *		pCode = NextTerm() ;
	while ( pCode != NULL )
	{
		if ( pCode->m_type == RSCode::typeParenthesis )
		{
			ESLAssert( pCode->IsKindOf( ESL_RUNTIME_CLASS(RSParenthesis) ) ) ;
			if ( (type == RSParenthesis::ptInvalid)
				|| (((RSParenthesis*)pCode)->m_parenthesis == type) )
			{
				return	(RSParenthesis*) pCode ;
			}
		}
		else if ( (pCode->m_type == RSCode::typeOperator)
				&& (((RSCodeOperator*)pCode)->m_operator
							== RSCodeOperator::opEndOfStatement) )
		{
			break ;
		}
		pCode = NextTerm() ;
	}
	m_index = iSave ;
	return	NULL ;
}

// 演算子を文末まで検索する
//////////////////////////////////////////////////////////////////////////////
RSCodeOperator * RSCodeStream::FindOperator
			( RSCodeOperator::OperatorIndex opIndex )
{
	const size_t	iSave = m_index ;
	RSCode *		pCode = NextTerm() ;
	while ( pCode != NULL )
	{
		if ( pCode->m_type == RSCode::typeOperator )
		{
			ESLAssert( pCode->IsKindOf( ESL_RUNTIME_CLASS(RSCodeOperator) ) ) ;
			RSCodeOperator *	pCodeOp = (RSCodeOperator*) pCode ;
			if ( (opIndex == RSCodeOperator::opInvalid)
						|| (pCodeOp->m_operator == opIndex) )
			{
				return	pCodeOp ;
			}
			if ( pCodeOp->m_operator == RSCodeOperator::opEndOfStatement )
			{
				break ;
			}
		}
		pCode = NextTerm() ;
	}
	m_index = iSave ;
	return	NULL ;
}

// １文読み飛ばす
//////////////////////////////////////////////////////////////////////////////
void RSCodeStream::PassAStatement( void )
{
	RSCode *	pCode = NextTerm() ;
	if ( pCode == NULL )
	{
		return ;
	}
	if ( pCode->m_type == RSCode::typeControlCode )
	{
		switch ( ((RSCodeControl*)pCode)->m_word )
		{
		case	RSCodeControl::wiClass:
			if ( FindParenthesis( RSParenthesis::ptBrace ) )
			{
				NextOperator( RSCodeOperator::opEndOfStatement ) ;
			}
			return ;
		case	RSCodeControl::wiFunction:
			if ( FindParenthesis( RSParenthesis::ptParenthesis ) )
			{
				if ( NextParenthesis( RSParenthesis::ptBrace ) )
				{
					NextOperator( RSCodeOperator::opEndOfStatement ) ;
				}
			}
			return ;
		case	RSCodeControl::wiFor:
		case	RSCodeControl::wiWhile:
			if ( NextParenthesis( RSParenthesis::ptParenthesis ) )
			{
				PassAStatement() ;
			}
			return ;
		case	RSCodeControl::wiDo:
			PassAStatement() ;
			if ( NextControlWord( RSCodeControl::wiWhile ) )
			{
				if ( NextParenthesis( RSParenthesis::ptParenthesis ) )
				{
					NextOperator( RSCodeOperator::opEndOfStatement ) ;
				}
			}
			return ;
		case	RSCodeControl::wiIf:
			if ( NextParenthesis( RSParenthesis::ptParenthesis ) )
			{
				PassAStatement() ;
			}
			if ( NextControlWord( RSCodeControl::wiElse ) )
			{
				PassAStatement() ;
			}
			return ;
		case	RSCodeControl::wiSwitch:
			if ( NextParenthesis( RSParenthesis::ptParenthesis ) )
			{
				PassAStatement() ;
			}
			return ;
		case	RSCodeControl::wiCase:
			FindOperator( RSCodeOperator::opSeparator ) ;
			return ;
		case	RSCodeControl::wiDefault:
			NextOperator( RSCodeOperator::opSeparator ) ;
			return ;
		case	RSCodeControl::wiTry:
			PassAStatement() ;
			while ( NextControlWord( RSCodeControl::wiCatch ) )
			{
				PassAStatement() ;
			}
			return ;
		case	RSCodeControl::wiWith:
			if ( NextParenthesis( RSParenthesis::ptParenthesis ) )
			{
				PassAStatement() ;
			}
			return ;
		default:
			return ;
		}
	}
	else if ( pCode->m_type == RSCode::typeParenthesis )
	{
		if ( ((RSParenthesis*)pCode)->m_parenthesis == RSParenthesis::ptBrace )
		{
			return ;
		}
	}
	while ( pCode != NULL )
	{
		if ( pCode->m_type == RSCode::typeOperator )
		{
			ESLAssert( pCode->IsKindOf( ESL_RUNTIME_CLASS(RSCodeOperator) ) ) ;
			if ( ((RSCodeOperator*)pCode)->m_operator
							== RSCodeOperator::opEndOfStatement )
			{
				break ;
			}
		}
		else if ( (pCode->m_type == RSCode::typeParenthesis)
				&& (((RSParenthesis*)pCode)->m_parenthesis
										== RSParenthesis::ptParenthesis) )
		{
			if ( NextParenthesis( RSParenthesis::ptBrace ) != NULL )
			{
				NextOperator( RSCodeOperator::opEndOfStatement ) ;
				break ;
			}
		}
		pCode = NextTerm() ;
	}
}

// コメント取得
//////////////////////////////////////////////////////////////////////////////
RSCodeComment * RSCodeStream::FindCommentFrom( size_t iIndex ) const
{
	for ( size_t i = iIndex; i < m_index; i ++ )
	{
		RSCode *	pCode = m_terms->GetAt( i + m_start ) ;
		if ( (pCode != NULL)
			&& (pCode->m_pComment != NULL) )
		{
			return	pCode->m_pComment ;
		}
	}
	return	NULL ;
}


//////////////////////////////////////////////////////////////////////////////
// 基底コンテキスト
//////////////////////////////////////////////////////////////////////////////

const RSContext::PFUNC_EXECUTE_STATEMENT
			RSContext::m_pfnExecuteStatement[RSCodeControl::wiCount + 1] =
{
	&RSContext::ExecuteStatementImport,				// import
	&RSContext::ExecuteStatementClass,				// class
	&RSContext::ExecuteStatementStruct,				// struct
	&RSContext::ExecuteStatementFunction,			// function
	&RSContext::ExecuteStatementInvalid,			// extends
	&RSContext::ExecuteStatementInvalid,			// implements
	&RSContext::ExecuteStatementFor,				// for
	&RSContext::ExecuteStatementWhile,				// while
	&RSContext::ExecuteStatementDo,					// do
	&RSContext::ExecuteStatementIf,					// if
	&RSContext::ExecuteStatementInvalid,			// else
	&RSContext::ExecuteStatementSwitch,				// switch
	&RSContext::ExecuteStatementCase,				// case
	&RSContext::ExecuteStatementDefault,			// default
	&RSContext::ExecuteStatementBreak,				// break
	&RSContext::ExecuteStatementContinue,			// continue
	&RSContext::ExecuteStatementTry,				// try
	&RSContext::ExecuteStatementInvalid,			// catch
	&RSContext::ExecuteStatementInvalid,			// finally
	&RSContext::ExecuteStatementThrow,				// throw
	&RSContext::ExecuteStatementReturn,				// return
	&RSContext::ExecuteStatementWith,				// with
	&RSContext::ExecuteStatementSynchronized,		// synchronized
	&RSContext::ExecuteStatementAccessModifier,		// static
	&RSContext::ExecuteStatementAccessModifier,		// abstract
	&RSContext::ExecuteStatementAccessModifier,		// native
	&RSContext::ExecuteStatementAccessModifier,		// const
	&RSContext::ExecuteStatementAccessModifier,		// public
	&RSContext::ExecuteStatementAccessModifier,		// protected
	&RSContext::ExecuteStatementAccessModifier,		// private
	&RSContext::ExecuteStatementVar,				// var
	&RSContext::ExecuteStatementVar,				// void
	&RSContext::ExecuteStatementVar,				// boolean
	&RSContext::ExecuteStatementVar,				// byte
	&RSContext::ExecuteStatementVar,				// short
	&RSContext::ExecuteStatementVar,				// char
	&RSContext::ExecuteStatementVar,				// int
	&RSContext::ExecuteStatementVar,				// long
	&RSContext::ExecuteStatementVar,				// float
	&RSContext::ExecuteStatementVar,				// double
	&RSContext::ExecuteStatementExpression,			// this
	&RSContext::ExecuteStatementExpression,			// super
	&RSContext::ExecuteStatementAccessModifier,		// extern
	&RSContext::ExecuteStatementDebug,				// debug
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSContext, SObject )
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSContext::DebugListener, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSContext::RSContext( RSVirtualMachine * pVM, RSObject * pThread )
{
	m_pVM = pVM ;
	m_pThread = pThread ;
	m_pcsThread = NULL ;
	//
	m_pExceptionParenthesis = NULL ;
	m_iExceptionStatement = 0 ;
	m_pCurParenthesis = NULL ;
	m_iSrcStatement = 0 ;
	m_pThisObj = NULL ;
	m_pThisClass = NULL ;
	m_pDebugListener = NULL ;
	m_pLastDebugPrth = NULL ;
	m_pLastDebugScript = NULL ;
	m_pExprParentOf = NULL ;
	m_pRetValue = NULL ;
	m_pException = NULL ;
	m_pLastComment = NULL ;
	m_flagsBehavior = 0 ;
	m_escape = escapeNothing ;
	//
	pVM->AddRef() ;
	RSObject::AddRef( pThread ) ;
	//
	m_pClassClass = pVM->GetClassClass() ;
	m_pVarClass = pVM->GetVariableClass() ;
	m_pFuncClass = pVM->GetFunctionClass() ;
	m_pBooleanClass = pVM->GetBooleanClass() ;
	m_pIntegerClass = pVM->GetIntegerClass() ;
	m_pNumberClass = pVM->GetNumberClass() ;
	m_pStringClass = pVM->GetStringClass() ;
	m_pArrayClass = pVM->GetArrayClass() ;
	m_pArrayBufferClass = pVM->GetArrayBufferClass() ;
	m_pExceptionClass = pVM->GetExceptionClass() ;
	m_pJObjectClass = pVM->GetGenericObjectClass() ;
	m_pJSObjectClass = pVM->GetDynamicObjectClass() ;
	m_pStructureClass = pVM->GetStructureClass() ;
	//
	for ( int i = RSCodeControl::wiFirstBasicType;
					i <= RSCodeControl::wiLastBasicType; i ++ )
	{
		m_pBasicTypeClass[i - RSCodeControl::wiFirstBasicType] =
				pVM->GetBasicTypeClass( (RSCodeControl::WordIndex) i ) ;
	}
	for ( int i = 0; i < RSReferenceNumber::typeCountOfNumber; i ++ )
	{
		m_pPtrTypeClass[i] =
			pVM->GetTypedPointerClass( (RSReferenceNumber::NumberType) i ) ;
	}
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
RSContext::~RSContext( void )
{
	while ( m_arrWith.GetLength() >= 1 )
	{
		PopNamespace() ;
	}
	RSObject::ReleaseRef( m_pExprParentOf ) ;
	RSObject::ReleaseRef( m_pException ) ;
	RSObject::ReleaseRef( m_pRetValue ) ;
	m_pExprParentOf = NULL ;
	m_pRetValue = NULL ;
	m_pException = NULL ;
	//
	if ( m_pcsThread != NULL )
	{
		ECSSakura2::VirtualMachine *	pVM = m_pVM->GetSakura2VM() ;
		if ( pVM != NULL )
		{
			pVM->FreeHeapObjectAddress
				( ((INT64)m_pcsThread->m_dwHighAddr) << 32, m_pcsThread ) ;
		}
		else
		{
			delete	m_pcsThread ;
		}
		m_pcsThread = NULL ;
	}
	//
	m_pVM->ReleaseRef() ;
	m_pVM = NULL ;
	//
	RSObject::ReleaseRef( m_pThread ) ;
	m_pThread = NULL ;
}

// スレッドオブジェクト関連付け
//////////////////////////////////////////////////////////////////////////////
void RSContext::AttachThreadObject( RSObject * pThread )
{
	RSObject::AddRef( pThread ) ;
	ReleaseObjectRef( m_pThread ) ;
	m_pThread = pThread ;
}

// スレッドオブジェクト取得
//////////////////////////////////////////////////////////////////////////////
RSObject * RSContext::GetThreadObject( void )
{
	RSObject::AddRef( m_pThread ) ;
	return	m_pThread ;
}

// デバッグリスナ設定
//////////////////////////////////////////////////////////////////////////////
void RSContext::AttachDebugListener( RSContext::DebugListener * pListener )
{
	m_pDebugListener = pListener ;
}

// デバッグリスナ解除
//////////////////////////////////////////////////////////////////////////////
void RSContext::DetachDebugListener( RSContext::DebugListener * pListener )
{
	if ( m_pDebugListener == pListener )
	{
		m_pDebugListener = NULL ;
	}
}

// 文実行（名前空間を作成して文を実行）
//////////////////////////////////////////////////////////////////////////////
void RSContext::PerformStatement
	( RSCodeStream& cstrm,
		RSObject * pThisObj,
		SSystem::SParserErrorInterface* pperr )
{
	m_pVM->AddRef() ;
	RSObject::AddRef( pThisObj ) ;
	PushNamespace
		( pThisObj,
			new_Namespace( nullptr, RSObject::modifierPublic, m_pVM ),
			RSObject::modifierPublic, true ) ;
	//
	ExecuteAllStatements( cstrm ) ;
	//
	PopNamespace() ;
}

// 式実行（名前空間を作成して式を評価）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSContext::PerformExpression
	( const wchar_t * pwszExpr,
		RSObject * pThisObj,
		SSystem::SParserErrorInterface* pperr )
{
	m_pVM->AddRef() ;
	RSObject::AddRef( pThisObj ) ;
	PushNamespace
		( pThisObj, m_pVM, RSObject::modifierPublic, true ) ;
	//
	RSObject *	pObj = EvaluateExpression( pwszExpr, pperr ) ;
	//
	PopNamespace() ;
	//
	return	pObj ;
}

RSObject * RSContext::PerformExpression
	( RSCodeStream& cstrm, RSObject * pThisObj )
{
	m_pVM->AddRef() ;
	RSObject::AddRef( pThisObj ) ;
	PushNamespace
		( pThisObj, m_pVM, RSObject::modifierPublic, true ) ;
	//
	RSObject *	pObj = EvaluateExpression( cstrm ) ;
	//
	PopNamespace() ;
	//
	return	pObj ;
}

// 関数実行（名前空間を作成して式を評価）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSContext::PerformFunction
	( RSFunctionObject& func,
		RSObject * pThisObj,
		RSObject**const ppArgs, size_t countArg,
		SSystem::SParserErrorInterface* pperr )
{
	m_pVM->AddRef() ;
	RSObject::AddRef( pThisObj ) ;
	PushNamespace
		( pThisObj, m_pVM, RSObject::modifierPublic, true ) ;
	//
	RSObject *	pObj =
		CallFunction( func, pThisObj, ppArgs, countArg, true ) ;
	if ( pperr != NULL )
	{
		OutputExceptionError( *pperr ) ;
	}
	//
	PopNamespace() ;
	//
	return	pObj ;
}

// 例外発生ソース位置情報取得
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSContext::GetExceptionPositionInfo
	( SSystem::SString& strSrcPath,
		SSystem::SString& strSrcLine,
		size_t& iSrcIndex, size_t& iSrcLine )
{
	return	GetSourcePositionInfo
				( strSrcPath, strSrcLine, iSrcIndex, iSrcLine,
							m_pExceptionParenthesis, m_iExceptionStatement ) ;
}

// 実行中のソース位置情報取得
//////////////////////////////////////////////////////////////////////////////
SError RSContext::GetCurrentPositionInfo
	( SSystem::SString& strSrcPath,
		SSystem::SString& strSrcLine,
		size_t& iSrcIndex, size_t& iSrcLine )
{
	return	GetSourcePositionInfo
				( strSrcPath, strSrcLine, iSrcIndex, iSrcLine,
							m_pCurParenthesis, m_iSrcStatement ) ;
}

// ソース位置情報取得
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSContext::GetSourcePositionInfo
	( SSystem::SString& strSrcPath,
		SSystem::SString& strSrcLine,
		size_t& iSrcIndex, size_t& iSrcLine,
		const RSParenthesis * pParenthesis, size_t iSrcInChars )
{
	strSrcPath = L"" ;
	strSrcLine = L"" ;
	iSrcIndex = iSrcInChars ;
	iSrcLine = 0 ;
	//
	if ( pParenthesis == NULL )
	{
		return	errFailed ;
	}
	const RSParenthesis *	pSrcParenthesis = pParenthesis ;
	const RSScript *	pSrcScript = ESLTypeCast<RSScript>( pSrcParenthesis ) ;
	while ( (pSrcScript == NULL)
		&& (pSrcParenthesis->m_parent != NULL) )
	{
		pSrcParenthesis = pSrcParenthesis->m_parent ;
		pSrcScript = ESLTypeCast<RSScript>( pSrcParenthesis ) ;
	}
	if ( pSrcScript == NULL )
	{
		return	errFailed ;
	}
	strSrcPath = pSrcScript->GetSourcePath() ;
	//
	RSSourceParser	sparsSource ;
	sparsSource.AttachString( pSrcScript->GetSourceText() ) ;
	//
	size_t	iLineIndex ;
	iSrcLine = sparsSource.GetLineNumberOf( iSrcInChars, &iLineIndex ) ;
	sparsSource.SeekIndex( iLineIndex ) ;
	sparsSource.NextLine( strSrcLine ) ;
	strSrcLine.TrimRight() ;
	//
	return	errSuccess ;
}

// 関数実行
//////////////////////////////////////////////////////////////////////////////
RSObject * RSContext::CallFunction
	( RSFunctionObject& func,
		RSObject * pThisObj,
		RSObject*const* ppArgs, size_t countArg,
		bool fStructCast, bool* pArgMatchResult )
{
	RSFunctionPrototype *
		pProto = func.GetMatchPrototype( ppArgs, countArg ) ;
	if ( pProto == NULL )
	{
		if ( pArgMatchResult != NULL )
		{
			*pArgMatchResult = false ;
		}
		else
		{
			ThrowExceptionError
				( L"関数の呼び出しで引数がプロトタイプに一致しません" ) ;
		}
		return	NULL ;
	}
	if ( pArgMatchResult != NULL )
	{
		*pArgMatchResult = true ;
	}
	return	CallFunction( pProto, pThisObj, ppArgs, countArg, fStructCast ) ;
}

RSObject * RSContext::CallFunction
	( RSFunctionPrototype * pProto,
		RSObject * pThisObj,
		RSObject*const* ppArgs, size_t countArg,
		bool fStructCast )
{
	bool	fConstThis = false ;
	if ( pThisObj != NULL )
	{
		fConstThis = ((pThisObj->GetModifiers() & RSObject::modifierConst) != 0) ;
		pThisObj = pThisObj->GetEntityObject() ;
	}
	EscapeContextSaver	ecs( *this ) ;
	//
	m_escape = escapeNothing ;
	m_pRetValue = NULL ;
	//
	if ( fConstThis )
	{
		if ( !pProto->IsConstantModifier() )
		{
			ThrowExceptionError
				( L"const ポインタから非 const な関数を呼び出しています" ) ;
			return	NULL ;
		}
	}
	RSObject::Synchronized	sync ;
	if ( pThisObj && pProto->IsSynchronizedModifier() )
	{
		if ( pThisObj->LockSynchronized( &sync, this ) == errAbort )
		{
			return	NULL ;
		}
	}
	RSSmartPtr	sptrThis( NULL, this ) ;
	if ( fStructCast )
	{
		RSStructuredPointerClass *	pThisStructClass =
			ESLTypeCast<RSStructuredPointerClass>( pProto->m_pNamespaceClass ) ;
		if ( (pThisStructClass != NULL)
			&& !pThisStructClass->IsKindOf( ESL_RUNTIME_CLASS(RSStructureClass) ) )
		{
			pThisObj = pThisStructClass->CastInstance( *this, pThisObj ) ;
			sptrThis = pThisObj ;
			if ( pThisObj == NULL )
			{
				ThrowExceptionError
					( SString(pThisStructClass->GetRSClassName())
						+ L" へ変換できません" ) ;
				return	NULL ;
			}
		}
	}
	if ( pProto->m_methodNative.pfnMethod != NULL )
	{
		//
		// ネイティブ関数呼び出し
		//
		RSObject *	pObj =
			(pProto->m_methodNative.pfnMethod)
				( *this, pProto->m_methodNative.pInstance,
						pThisObj, (RSObject**) ppArgs, countArg ) ;
		//
		if ( pThisObj && pProto->IsSynchronizedModifier() )
		{
			pThisObj->UnlockSynchronized( this ) ;
		}
		return	pObj ;
	}
	if ( pProto->m_pfnFuncAddr != -1 )
	{
		//
		// Sakura2 関数呼び出し
		//
		ECSSakura2::VirtualMachine *	pVM = m_pVM->GetSakura2VM() ;
		if ( pVM != NULL )
		{
			if ( m_pcsThread == NULL )
			{
				m_pcsThread = ECSSakura2::ThreadObject::NewContext() ;
				ECSSakura2Processor::AssertLock() ;
				m_pcsThread->InitializeContext( pVM ) ;
				m_pcsThread->m_ptrReserved[0] = this ;
				ECSSakura2Processor::AssertUnlock() ;
			}
			Register *	prArgs = NULL ;
			size_t		nArgCount = 0 ;
			Register *	prOwnArgs = NULL ;
			Register	regArgs[32] ;
			size_t		nProtoArgs = pProto->m_aArgTypes.GetLength() ;
			//
			if ( countArg > nProtoArgs )
			{
				ThrowExceptionError( L"引数が多すぎます" ) ;
				return	NULL ;
			}
			if ( nProtoArgs <= 32 )
			{
				prArgs = &regArgs[0] ;
			}
			else
			{
				prOwnArgs = new Register[nProtoArgs] ;
				prArgs = prOwnArgs ;
			}
			for ( size_t i = 0; i < countArg; i ++ )
			{
				prArgs[i].i = (ulong_ptr_t) ppArgs[i] ;
			}
			for ( size_t i = countArg; i < nProtoArgs; i ++ )
			{
				RSObject *	pArgDef = pProto->m_aArgDefault.GetAt( i ) ;
				if ( pArgDef != NULL )
				{
					RSObject *	pEntity = pArgDef->GetEntityObject() ;
					if ( pEntity != NULL )
					{
						pEntity = pEntity->CloneObject( *this ) ;
					}
					prArgs[i].i = (ulong_ptr_t) pEntity ;
				}
				else
				{
					prArgs[i].i = (ulong_ptr_t) new_Pointer( NULL ) ;
				}
			}
			//
			DWORD		ipSave = m_pcsThread->m_ip ;
			DWORD		ipSegSave = m_pcsThread->m_ipSegment ;
			Register	regSave[16] ;
			for ( size_t i = 1; i < 15; i ++ )
			{
				regSave[i] = m_pcsThread->m_regset[i] ;
			}
			//
			m_pcsThread->m_regset[regTP].i = (ulong_ptr_t) pThisObj ;
			m_pcsThread->m_regset[regXP].i = 0 ;
			m_pcsThread->m_regset[regYP].i = 0 ;
			//
			m_pCurParenthesis = pProto->m_pParenthesis ;
			m_iSrcStatement = 0 ;
			//
			/*
			ECSSakura2::StandardVM *
				pStdVM = ESLTypeCast<ECSSakura2::StandardVM>( pVM ) ;
			if ( pStdVM != NULL )
			{
				ECSSakura2::StandardVM::SearchFunctionInfo	sfi ;
				if ( !pStdVM->SearchFunctionAtAddress
							( sfi, pProto->m_pfnFuncAddr ) )
				{
					sfi.pModule->DebugTraceDisassemble
						( sfi.pFuncEntry->dwAddress, sfi.pFuncEntry->dwBytes ) ;
				}
			}
//			*/
			const wchar_t *	pwszErr =
				m_pcsThread->CallFunction
					( pProto->m_pfnFuncAddr, prArgs, (int) nProtoArgs ) ;
			//
			RSObject *	pResult =
				(RSObject*) m_pcsThread->m_regset[regAcc].i ;
			//
			if ( pwszErr != NULL )
			{
				INT64	yp = m_pcsThread->m_regset[regYP].i ;
				while ( yp != 0 )
				{
					Register *	pYP =
						(Register*) pVM->TranslateAddress( yp, 24 ) ;
					if ( pYP == NULL )
					{
						break ;
					}
					if ( pYP[1].i == 0 )
					{
						ecs_nakedcall___nrs_release_ptr_ref
										( m_pcsThread, pYP + 2 ) ;
					}
					else
					{
						ecs_nakedcall___nrs_release_ref
										( m_pcsThread, pYP + 2 ) ;
					}
					yp = pYP[0].i ;
				}
				RSVirtualMachine::ScriptPosition	sp ;
				if ( m_pVM->SearchCodePosition
					( sp, (m_pcsThread->m_ip - 1)
							| (((INT64) m_pcsThread->m_ipSegment) << 32) ) )
				{
					m_pCurParenthesis = sp.pParenthesis ;
					m_iSrcStatement = sp.iSource ;
				}
				ThrowExceptionError( pwszErr ) ;
			}
			//
			for ( size_t i = countArg; i < nProtoArgs; i ++ )
			{
				ReleaseObjectRef( (RSObject*) prArgs[i].i ) ;
			}
			//
			m_pcsThread->m_ip = ipSave ;
			m_pcsThread->m_ipSegment = ipSegSave ;
			for ( size_t i = 1; i < 15; i ++ )
			{
				m_pcsThread->m_regset[i] = regSave[i] ;
			}
			m_pcsThread->m_status = ECSSakura2Processor::Context::xsExecution ;
			//
			if ( pThisObj && pProto->IsSynchronizedModifier() )
			{
				pThisObj->UnlockSynchronized( this ) ;
			}
			return	pResult ;
		}
	}
	const RSParenthesis *	pLastParenthesis = m_pCurParenthesis ;
	const size_t			iLastSrcStatement = m_iSrcStatement ;
	RSClass * const			pLastNamespaceClass = m_pThisClass ;
	m_pCurParenthesis = pProto->m_pParenthesis ;
	//
	// 名前空間作成
	//
	RSObject *	pRefNamespace = pProto->m_pRefNamespace ;
	RSObject::AddRef( pRefNamespace ) ;
	PushNamespace
		( NULL, pRefNamespace, RSObject::modifierPrivate, true, pProto ) ;
	//
	RSNamespace *	pNamespace = new_Namespace() ;
	RSObject::AddRef( pThisObj ) ;
	if ( pProto->IsConstantModifier() )
	{
		RSObject *	pConstThis = new_Reference( pThisObj ) ;
		pConstThis->SetModifiers( RSObject::modifierConst ) ;
		PushNamespace( pConstThis, pNamespace, RSObject::modifierPrivate ) ;
	}
	else
	{
		PushNamespace( pThisObj, pNamespace, RSObject::modifierPrivate ) ;
	}
	m_pThisClass = pProto->m_pNamespaceClass ;
	//
	// 引数設定
	//
	size_t	iArg ;
	for ( iArg = 0; iArg < countArg; iArg ++ )
	{
		SString *	pstrName = pProto->m_aArgNames.GetAt( iArg ) ;
		if ( pstrName == NULL )
		{
			break ;
		}
		RSClass *	pArgType = pProto->m_aArgTypes.GetAt( iArg ) ;
		RSObject *	pArg = ppArgs[iArg] ;
		RSObject *	pArgVar ;
		if ( pArgType != NULL )
		{
			if ( pArg != NULL )
			{
				pArgVar = pArgType->NewVariable( *this ) ;
				ESLAssert( pArgVar != NULL ) ;
				ReleaseObjectRef( pArgVar->OperatorMove( *this, pArg ) ) ;
			}
			else
			{
				pArgVar = new_Pointer( NULL, pArgType ) ;
			}
		}
		else
		{
			RSObject::AddRef( pArg ) ;
			pArgVar = new_Pointer( pArg ) ;
		}
		CreateVariableAs( *pstrName, pArgVar ) ;
	}
	//
	// デフォルト引数
	//
	for ( iArg = countArg; iArg < pProto->m_aArgNames.GetLength(); iArg ++ )
	{
		SString *	pstrName = pProto->m_aArgNames.GetAt( iArg ) ;
		ESLAssert( pstrName != NULL ) ;
		if ( pstrName == NULL )
		{
			break ;
		}
		RSClass *	pArgType = pProto->m_aArgTypes.GetAt( iArg ) ;
		RSObject *	pArgDef = NULL ;
		RSObject *	pArgVar ;
		pArgDef = pProto->m_aArgDefault.GetAt( iArg ) ;
		if ( pArgType != NULL )
		{
			if ( pArgDef != NULL )
			{
				if ( pArgDef != NULL )
				{
					RSObject *	pEntity = pArgDef->GetEntityObject() ;
					if ( pEntity != NULL )
					{
						pEntity = pEntity->CloneObject( *this ) ;
					}
					pArgDef = pEntity ;
				}
				pArgVar = pArgType->NewVariable( *this ) ;
				ESLAssert( pArgVar != NULL ) ;
				ReleaseObjectRef( pArgVar->OperatorMove( *this, pArgDef ) ) ;
				ReleaseObjectRef( pArgDef ) ;
			}
			else
			{
				ThrowExceptionError( L"関数の引数が不足しています" ) ;
				break ;
			}
		}
		else
		{
			pArgVar = new_Pointer( pArgDef ) ;
		}
		CreateVariableAs( *pstrName, pArgVar ) ;
	}
	//
	// 実行
	//
	if ( m_pDebugListener != NULL )
	{
		m_pDebugListener->OnCall( this, pProto ) ;
	}
	if ( !IsException() )
	{
		RSParenthesis *	pPrthCode = pProto->m_pParenthesis ;
		if ( pPrthCode != NULL )
		{
			RSCodeStream	cs( *pPrthCode ) ;
			ExecuteAllStatements( cs ) ;
		}
		else
		{
			ThrowExceptionError( L"純粋仮想関数を呼び出しています" ) ;
		}
	}
	if ( m_escape != escapeNothing )
	{
		if ( m_escape == escapeBreak )
		{
			ThrowExceptionError( L"break 文が反復文の内側にありません" ) ;
			m_escape = escapeNothing ;
		}
		else if ( m_escape == escapeContinue )
		{
			ThrowExceptionError( L"continue 文が反復文の内側にありません" ) ;
			m_escape = escapeNothing ;
		}
	}
	//
	// 終了
	//
	PopNamespace() ;
	PopNamespace() ;
	//
	m_pThisClass = pLastNamespaceClass ;
	m_pCurParenthesis = pLastParenthesis ;
	m_iSrcStatement = iLastSrcStatement ;
	//
	if ( pThisObj && pProto->IsSynchronizedModifier() )
	{
		pThisObj->UnlockSynchronized( this ) ;
	}
	RSObject *	pRetObj = PopReturnObject() ;
	if ( pRetObj != NULL )
	{
		RSClass *	pRetType = pProto->m_pReturnType ;
		if ( pRetType != NULL )
		{
			RSObject *	pCast =
				pRetType->CastInstance
						( *this, pRetObj, RSClass::castNatural ) ;
			if ( pCast != NULL )
			{
				ReleaseObjectRef( pRetObj ) ;
				pRetObj = pCast ;
			}
			else
			{
				ThrowExceptionError
					( SString(pRetObj->GetTypeName())
						+ L" から " + pRetType->GetRSClassName()
						+ L" へキャストできません" ) ;
			}
		}
	}
	if ( (m_pDebugListener != NULL) && !IsException() )
	{
		const RSScript *
			pScript = DebugScriptFromParenthesis(m_pCurParenthesis) ;
		m_pDebugListener->OnReturned( this, pScript, m_iSrcStatement ) ;
	}
	return	pRetObj ;
}

RSObject * RSContext::CallFunction
	( RSFunctionObject& func,
		RSObject * pThisObj, RSObject& arrayArg,
		bool fStructCast, bool* pArgMatchResult )
{
	SPointerArray<RSObject>	arrArgBuf ;
	RSObject*	pArgs[0x20] ;
	RSObject**	ppArgs ;
	size_t	nCount = arrayArg.GetElementCount() ;
	if ( nCount < 0x20 )
	{
		ppArgs = &pArgs[0] ;
	}
	else
	{
		ppArgs = arrArgBuf.GetArray( nCount ) ;
	}
	for ( size_t i = 0; i < nCount; i ++ )
	{
		ppArgs[i] = arrayArg.GetElementAt( *this, (int) i ) ;
	}
	RSObject *	pRetObj =
		CallFunction( func, pThisObj, ppArgs, nCount,
								fStructCast, pArgMatchResult ) ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		ReleaseObjectRef( ppArgs[i] ) ;
	}
	arrArgBuf.FinishArray() ;
	return	pRetObj ;
}

// メソッド実行
//////////////////////////////////////////////////////////////////////////////
RSObject * RSContext::CallMethod
	( RSObject * pThisObj,
		RSClass * pThisClass, const wchar_t * pwszFuncName,
		RSObject*const*const ppArgs, size_t countArg,
		bool fStructCast, bool* pArgMatchResult )
{
	RSFunctionObject *	pFuncObj =
				pThisClass->GetVirtualMemberAs( *this, pwszFuncName ) ;
	if ( pFuncObj == NULL )
	{
		if ( pArgMatchResult != NULL )
		{
			*pArgMatchResult = false ;
		}
		else
		{
			ThrowExceptionError
				( SString(pwszFuncName) + L" メソッドが見つかりません" ) ;
		}
		return	NULL ;
	}
	RSObject *	pObj =
		CallFunction
			( *pFuncObj, pThisObj, ppArgs, countArg,
							fStructCast, pArgMatchResult ) ;
	ReleaseObjectRef( pFuncObj ) ;
	return	pObj ;
}

RSObject * RSContext::CallMethod
	( RSObject * pThisObj, const wchar_t * pwszFuncName,
		RSObject*const* ppArgs, size_t countArg,
		bool fStructCast, bool* pArgMatchResult )
{
	ESLAssert( pThisObj != NULL ) ;
	pThisObj = pThisObj->GetEntityObject() ;
	if ( pThisObj == NULL )
	{
		ThrowExceptionError
			( L"null への不正なメソッド呼び出しです" ) ;
		return	NULL ;
	}
	return	CallMethod
		( pThisObj, pThisObj->GetRSClass(),
			pwszFuncName, ppArgs, countArg,
			fStructCast, pArgMatchResult ) ;
}

// 全文実行
//////////////////////////////////////////////////////////////////////////////
void RSContext::ExecuteAllStatements
		( RSCodeStream& cstrm, RSClass * pDefClass )
{
	const RSParenthesis *	pLastParenthesis = m_pCurParenthesis ;
	m_pCurParenthesis = cstrm.GetParenthesis() ;
	//
	while ( !cstrm.IsEndOfStream()
			&& (GetEscape() == escapeNothing) && !IsException() )
	{
		ExecuteAStatement( cstrm, pDefClass ) ;
	}
	m_pCurParenthesis = pLastParenthesis ;
}

// 文実行
//////////////////////////////////////////////////////////////////////////////
void RSContext::ExecuteStatements
		( RSCodeStream& cstrm, RSClass * pDefClass )
{
	RSParenthesis *	pPrth =
			cstrm.NextParenthesis( RSParenthesis::ptBrace ) ;
	if ( pPrth != NULL )
	{
		RSCodeStream	cs( *pPrth ) ;
		while ( !cs.IsEndOfStream()
				&& (GetEscape() == escapeNothing) && !IsException() )
		{
			ExecuteAStatement( cs, pDefClass ) ;
		}
	}
	else
	{
		ExecuteAStatement( cstrm, pDefClass ) ;
	}
}

// 一文実行
//////////////////////////////////////////////////////////////////////////////
void RSContext::ExecuteAStatement
		( RSCodeStream& cstrm, RSClass * pDefClass )
{
	RSCodeControl *	pCode = cstrm.NextStatementControlWord() ;
	RSCode *	pcd = cstrm.GetTerm() ;
	if ( pcd != NULL )
	{
		m_iSrcStatement = pcd->m_iSrc ;
		m_pLastComment = pcd->m_pComment ;
	}
	if ( pCode != NULL )
	{
		// <control-word> statement ;
		m_pCurParenthesis = cstrm.GetParenthesis() ;
		m_iSrcStatement = pCode->m_iSrc ;
		m_pLastComment = pCode->m_pComment ;
		(this->*m_pfnExecuteStatement[pCode->m_word])
						( cstrm, pCode->m_word, pDefClass ) ;
	}
	else
	{
		RSClass *	pClass = ParseClassExpression( cstrm ) ;
		if ( pClass != NULL )
		{
			// <type-expr> <var-name>
			//	{ [= <init-expr>] | ( <arg-list> ) <function-implements> }
			ExecuteDeclareVariable
				( cstrm, 0, pClass, declModeAny, pDefClass, m_pLastComment ) ;
		}
		else if ( cstrm.NextOperator
					( RSCodeOperator::opEndOfStatement ) == NULL )
		{
			// expression ;
			ReleaseObjectRef
				( EvaluateExpression
					( cstrm, RSCodeOperator::priorityNothing ) ) ;
			cstrm.NextOperator( RSCodeOperator::opEndOfStatement ) ;
		}
	}
}

// 定義文解釈／実行
//////////////////////////////////////////////////////////////////////////////
void RSContext::ExecuteDeclareVariable
	( RSCodeStream& cstrm, uint32_t accMod, RSClass * pClass,
		uint32_t modeDecl, RSClass * pDefClass, RSCodeComment * pDefComment )
{
	size_t			iDeclFirstCode = cstrm.GetIndex() ;
	RSCodeSymbol *	pSymName = cstrm.NextSymbol() ;
	if ( pSymName == NULL )
	{
		RSParenthesis *	pPrth =
				cstrm.NextParenthesis( RSParenthesis::ptParenthesis ) ;
		if ( (pPrth != NULL) && (pClass == pDefClass) && (modeDecl & declModeFunc) )
		{
			m_iSrcStatement = pPrth->m_iSrc ;
			//
			// 構築関数
			//
			RSFunctionPrototype *	pProto = new RSFunctionPrototype ;
			SParserErrorLogger		perrLog ;
			RSCodeStream			csArg( *pPrth ) ;
			if ( pProto->ParseArgument( *this, csArg, perrLog ) )
			{
				SParserErrorLogger::ErrorLog *
							pErr = perrLog.GetErrorLogAt( 0 ) ;
				if ( pErr != NULL )
				{
					ThrowExceptionError( pErr->m_strError ) ;
				}
				else
				{
					ThrowExceptionError( L"構築関数の引数の書式が不正です" ) ;
				}
				delete	pProto ;
				return ;
			}
			ESLAssert( pDefClass != NULL ) ;
			RSFunctionObject *	pFunc = pDefClass->GetConstructor() ;
			if ( pFunc == NULL )
			{
				pFunc = new RSFunctionObject( m_pFuncClass ) ;
				pDefClass->SetVirtualMemberAs( *this, L"<init>", pFunc ) ;
			}
			//
			if ( accMod & RSObject::modifierSynchronized )
			{
				pProto->m_nFlags |= RSFunctionPrototype::flagSynchronized ;
				accMod &= ~RSObject::modifierSynchronized ;
			}
			if ( accMod & RSObject::modifierConst )
			{
				pProto->m_nFlags |= RSFunctionPrototype::flagConstant ;
				accMod &= ~RSObject::modifierConst ;
			}
			pFunc->SetModifiers( accMod ) ;
			//
			pProto->SetRefNamespace( GetCurrentNamespace() ) ;
			pProto->m_pNamespaceClass = pDefClass ;
			//
			pPrth = cstrm.NextParenthesis( RSParenthesis::ptBrace ) ;
			if ( pPrth == NULL )
			{
				ThrowExceptionError( L"関数の実装がありません" ) ;
			}
			pProto->m_pParenthesis = pPrth ;
			pProto->m_pComment = pDefComment ;
			//
			pFunc->AddPrototype( pProto ) ;
			return ;
		}
		else
		{
			ThrowExceptionError( L"定義名が記述されていません" ) ;
			return ;
		}
	}
	size_t		iCode = cstrm.GetIndex() ;
	RSCode *	pCode = cstrm.NextTerm() ;
	if ( (pCode != NULL)
		&& (pCode->m_type == RSCode::typeParenthesis)
		&& (((RSParenthesis*)pCode)->m_parenthesis
							== RSParenthesis::ptParenthesis)
		&& (modeDecl & declModeFunc) )
	{
		m_iSrcStatement = pCode->m_iSrc ;
		//
		// 関数定義解釈
		//
		ESLAssert( pCode->IsKindOf( ESL_RUNTIME_CLASS(RSParenthesis) ) ) ;
		RSParenthesis *	pPrth = (RSParenthesis*) pCode ;
		if ( pPrth->m_parenthesis != RSParenthesis::ptParenthesis )
		{
			ThrowExceptionError( L"定義名が記述されていません" ) ;
			return ;
		}
		RSFunctionPrototype *	pProto = new RSFunctionPrototype ;
		SParserErrorLogger		perrLog ;
		RSCodeStream			csArg( *pPrth ) ;
		if ( pProto->ParseArgument( *this, csArg, perrLog ) )
		{
			SParserErrorLogger::ErrorLog *
						pErr = perrLog.GetErrorLogAt( 0 ) ;
			if ( pErr != NULL )
			{
				ThrowExceptionError( pErr->m_strError ) ;
			}
			else
			{
				ThrowExceptionError( L"関数の引数の書式が不正です" ) ;
			}
			delete	pProto ;
			return ;
		}
		if ( accMod & RSObject::modifierSynchronized )
		{
			pProto->m_nFlags |= RSFunctionPrototype::flagSynchronized ;
			accMod &= ~RSObject::modifierSynchronized ;
		}
		if ( accMod & RSObject::modifierConst )
		{
			pProto->m_nFlags |= RSFunctionPrototype::flagConstant ;
			accMod &= ~RSObject::modifierConst ;
		}
		pProto->SetReturnType( pClass ) ;
		pProto->SetRefNamespace( GetCurrentNamespace() ) ;
		pProto->m_pNamespaceClass = pDefClass ;
		pProto->m_pComment = pDefComment ;
		//
		if ( !(accMod & (RSObject::modifierAbstract
							| RSObject::modifierNative)) )
		{
			pPrth = cstrm.NextParenthesis( RSParenthesis::ptBrace ) ;
			if ( pPrth == NULL )
			{
				ThrowExceptionError( L"関数の実装がありません" ) ;
			}
			pProto->m_pParenthesis = pPrth ;
		}
		cstrm.NextOperator( RSCodeOperator::opEndOfStatement ) ;
		//
		// 関数追加
		//
		RSFunctionObject *	pFunc ;
		if ( pDefClass != NULL )
		{
			if ( !(accMod & RSObject::modifierStatic) )
			{
				pFunc = pDefClass->GetVirtualMemberAs( *this, pSymName->m_symbol ) ;
				if ( pFunc == NULL )
				{
					pFunc = new RSFunctionObject( m_pFuncClass ) ;
					pDefClass->SetVirtualMemberAs( *this, pSymName->m_symbol, pFunc ) ;
					pFunc->AddRef() ;
					pFunc->SetModifiers( accMod | RSObject::modifierConst ) ;
				}
				pFunc->AddPrototype( pProto ) ;
				pFunc->ReleaseRef() ;
			}
			else
			{
				RSObject *	pObjFunc =
					pDefClass->GetMemberAs( *this, pSymName->m_symbol ) ;
				if ( pObjFunc != NULL )
				{
					RSObject *	pObjEntity = pObjFunc->GetEntityObject() ;
					if ( (pObjEntity == NULL)
						|| (pObjEntity->GetBasicType() != RSObject::typeFunction) )
					{
						ReleaseObjectRef( pObjFunc ) ;
						ThrowExceptionError( L"関数名が重複しています" ) ;
						return ;
					}
					ESLAssert( pObjEntity->IsKindOf
								( ESL_RUNTIME_CLASS(RSFunctionObject) ) ) ;
					pFunc = (RSFunctionObject*) pObjEntity ;
				}
				else
				{
					pFunc = new RSFunctionObject( m_pFuncClass ) ;
					pFunc->m_strFuncName = pSymName->m_symbol ;
					pFunc->m_pFuncClass = pDefClass ;
					//
					ReleaseObjectRef
						( pDefClass->CreateMemberAs
							( *this, pSymName->m_symbol, pFunc ) ) ;
					pFunc->SetModifiers( accMod | RSObject::modifierConst ) ;
				}
				pFunc->AddPrototype( pProto ) ;
				ReleaseObjectRef( pObjFunc ) ;
			}
		}
		else
		{
			pFunc = new RSFunctionObject( m_pFuncClass ) ;
			pFunc->m_strFuncName = pSymName->m_symbol ;
			//
			pFunc->AddPrototype( pProto ) ;
			CreateVariableAs( pSymName->m_symbol, pFunc ) ;
			if ( IsException() )
			{
				return ;
			}
			pFunc->SetModifiers( accMod | RSObject::modifierConst ) ;
		}
		if ( accMod & RSObject::modifierNative )
		{
			ESLAssert( pProto->m_pParenthesis == NULL ) ;
			SString	strFuncName ;
			if ( pDefClass != NULL )
			{
				strFuncName = pDefClass->GetFullClassName() + L"." + pSymName->m_symbol ;
			}
			else
			{
				strFuncName = pSymName->m_symbol ;
			}
			m_pVM->GetNativeMethod( pProto->m_methodNative, *this, strFuncName ) ;
		}
	}
	else if ( (pClass != NULL) && (modeDecl & declModeVar) )
	{
		//
		// 変数定義解釈
		//
		cstrm.SeekIndex( iCode ) ;
		while ( !IsException() )
		{
			RSObject *	pVar = pClass->NewVariable( *this ) ;
			ESLAssert( pVar != NULL ) ;
			pVar->SetModifiers( accMod ) ;
			pVar->SetDefinitionComment( pDefComment ) ;
			//
			RSStructuredPointerClass *	pDefStruct = NULL ;
			if ( pDefClass != NULL )
			{
				if ( !(accMod & RSObject::modifierStatic) )
				{
					pDefStruct = ESLTypeCast<RSStructuredPointerClass>( pDefClass ) ;
					if ( pDefStruct != NULL )
					{
						// 構造体メンバ
						ReleaseObjectRef( pVar ) ;
						pVar = NULL ;
						//
						RSParenthesis *	pPrthArray =
							cstrm.NextParenthesis( RSParenthesis::ptBracket ) ;
						size_t		nArray = 0 ;
						RSObject *	pInitObj = NULL ;
						if ( pPrthArray != NULL )
						{
							RSCodeStream	csSize( *pPrthArray ) ;
							RSObject *	pObjSize = EvaluateExpression( csSize ) ;
							if ( IsException() )
							{
								return ;
							}
							int64_t	numSize ;
							if ( (pObjSize == NULL)
								|| !pObjSize->AsInteger( numSize ) )
							{
								ReleaseObjectRef( pObjSize ) ;
								ThrowExceptionError( L"構造体メンバの配列長が不正です" ) ;
								return ;
							}
							ReleaseObjectRef( pObjSize ) ;
							nArray = (size_t) numSize ;
						}
						else if ( cstrm.NextOperator( RSCodeOperator::opMove ) != NULL )
						{
							pInitObj = EvaluateExpression
										( cstrm, RSCodeOperator::priorityList ) ;
							if ( IsException() )
							{
								ReleaseObjectRef( pInitObj ) ;
								return ;
							}
						}
						pDefStruct->AddArrayMemberAs
							( *this, pSymName->m_symbol,
								pClass, accMod, nArray, pInitObj, pDefComment ) ;
						//
						if ( cstrm.NextParenthesis( RSParenthesis::ptBracket ) != NULL )
						{
							ThrowExceptionError
								( L"構造体メンバに多次元配列は使用できません" ) ;
							return ;
						}
					}
					else
					{
						// クラスメンバ
						if ( pDefClass->m_pPrototype == NULL )
						{
							pDefClass->m_pPrototype =
									new RSGenericObject( pDefClass ) ;
						}
						ReleaseObjectRef
							( pDefClass->m_pPrototype->
								CreateMemberAs
									( *this, pSymName->m_symbol, pVar ) ) ;
					}
				}
				else
				{
					// static メンバ
					ReleaseObjectRef
						( pDefClass->CreateMemberAs
							( *this, pSymName->m_symbol, pVar ) ) ;
				}
			}
			else
			{
				CreateVariableAs( pSymName->m_symbol, pVar ) ;
				//
				if ( accMod & RSObject::modifierStatic )
				{
					ThrowExceptionError( L"static は不正な修飾です" ) ;
				}
			}
			if ( IsException() )
			{
				return ;
			}
			if ( cstrm.NextOperator( RSCodeOperator::opMove ) != NULL )
			{
				if ( pVar == NULL )
				{
					ThrowExceptionError
						( L"構造体メンバの初期値が記述されています" ) ;
					return ;
				}
				RSObject *	pInitObj =
						EvaluateExpression
							( cstrm, RSCodeOperator::priorityList ) ;
				if ( IsException() )
				{
					ReleaseObjectRef( pInitObj ) ;
					return ;
				}
				ReleaseObjectRef( pVar->OperatorMove( *this, pInitObj ) ) ;
				ReleaseObjectRef( pInitObj ) ;
			}
			if ( cstrm.NextOperator( RSCodeOperator::opSequencing ) == NULL )
			{
				if ( (cstrm.NextOperator( RSCodeOperator::opEndOfStatement ) != NULL)
					|| cstrm.IsEndOfStream() )
				{
					RSCodeComment *	pLineComment = cstrm.FindCommentFrom( iDeclFirstCode ) ;
					if ( (pLineComment != NULL) && (pVar != NULL) )
					{
						pVar->SetDefinitionComment( pLineComment ) ;
						iDeclFirstCode = cstrm.GetIndex() ;
						pDefComment = NULL ;
					}
					break ;
				}
				else
				{
					ThrowExceptionError
						( L"変数の初期値と次の変数が"
							L" \',\' で区切られていません" ) ;
					return ;
				}
			}
			RSCodeComment *	pLineComment = cstrm.FindCommentFrom( iDeclFirstCode ) ;
			if ( (pLineComment != NULL) && (pVar != NULL) )
			{
				pVar->SetDefinitionComment( pLineComment ) ;
				iDeclFirstCode = cstrm.GetIndex() ;
				pDefComment = NULL ;
			}
			pSymName = cstrm.NextSymbol() ;
			if ( pSymName == NULL )
			{
				ThrowExceptionError
					( L"変数定義で、\',\' の後に変数名がありません" ) ;
				return ;
			}
		}
	}
	else
	{
		ThrowExceptionError( L"void 変数は定義できません" ) ;
	}
}

// 数式評価
//////////////////////////////////////////////////////////////////////////////
RSObject * RSContext::EvaluateExpression( RSCodeStream& cs, int priority )
{
	if ( m_pExprParentOf != NULL )
	{
		ReleaseObjectRef( m_pExprParentOf ) ;
		m_pExprParentOf = NULL ;
	}
	RSCode *	pCode = cs.NextTerm() ;
	if ( pCode == NULL )
	{
		ThrowExceptionError( L"数式の解釈中に文末に到達しました" ) ;
		return	NULL ;
	}
	m_iSrcStatement = pCode->m_iSrc ;
	//
	RSCode *	pNextCode ;
	RSClass *	pThisClass ;
	RSObject *	pObj = NULL ;
	switch ( pCode->m_type )
	{
	case	RSCode::typeLiteral:
		ESLAssert( pCode->IsKindOf( ESL_RUNTIME_CLASS(RSCodeLiteral) ) ) ;
		ESLAssert( ((RSCodeLiteral*)pCode)->m_literal != NULL ) ;
		pObj = ((RSCodeLiteral*)pCode)->m_literal->DuplicateObject( *this ) ;
		break ;

	case	RSCode::typeControlCode:
		ESLAssert( pCode->IsKindOf( ESL_RUNTIME_CLASS(RSCodeControl) ) ) ;
		switch ( ((RSCodeControl*)pCode)->m_word )
		{
		case	RSCodeControl::wiThis:
			// this オブジェクト
			pObj = m_pThisObj ;
			if ( pObj == NULL )
			{
				ThrowExceptionError( L"this は存在しません" ) ;
				return	NULL ;
			}
			pObj->AddRef() ;
			break ;

		case	RSCodeControl::wiFunction:
			// function オブジェクト
			pObj = EvaluateFunctionObject( cs ) ;
			break ;

		case	RSCodeControl::wiSuper:
			if ( m_pThisObj == NULL )
			{
				ThrowExceptionError( L"this は存在しません" ) ;
				return	NULL ;
			}
			pThisClass = m_pThisClass ;
			if ( (pThisClass == NULL)
				|| (pThisClass->m_pSuperClass == NULL) )
			{
				ThrowExceptionError( L"super は存在しません" ) ;
				return	NULL ;
			}
			pNextCode = cs.GetTerm() ;
			if ( (pNextCode != NULL)
				&& (pNextCode->m_type == RSCode::typeParenthesis)
				&& (((RSParenthesis*)pNextCode)->
						m_parenthesis == RSParenthesis::ptParenthesis) )
			{
				// super() コンストラクタ呼び出し
				RSFunctionObject *
					pFunc = pThisClass->m_pSuperClass->GetConstructor() ;
				if ( pFunc == NULL )
				{
					ThrowExceptionError
						( L"super クラスにコンストラクタは存在しません" ) ;
					return	NULL ;
				}
				RSObject *	pArg = new_Array() ;
				pNextCode = cs.NextTerm() ;
				EvaluateArgument( *pArg, *((RSParenthesis*)pNextCode) ) ;
				if ( IsException() )
				{
					ReleaseObjectRef( pArg ) ;
					return	NULL ;
				}
				pObj = CallFunction( *pFunc, m_pThisObj, *pArg, false ) ;
				ReleaseObjectRef( pArg ) ;
				break ;
			}
			else if ( (pNextCode != NULL)
				&& (pNextCode->m_type == RSCode::typeOperator)
				&& (((RSCodeOperator*)pNextCode)->
						m_operator == RSCodeOperator::opMemberOf) )
			{
				// super.member 親クラスのメンバ関数
				pNextCode = cs.NextTerm( 1 ) ;
				if ( (pNextCode == NULL)
					|| (pNextCode->m_type != RSCode::typeSymbol) )
				{
					ThrowExceptionError
						( L"super メンバが指定されていません" ) ;
					return	NULL ;
				}
				const SString&
					strMember = ((RSCodeSymbol*)pNextCode)->m_symbol ;
				RSClass *	pSuperClass = pThisClass->m_pSuperClass ;
				RSObject *	pMember =
						pSuperClass->GetMemberAs( *this, strMember ) ;
				ESLAssert( m_pExprParentOf == NULL ) ;
				if ( pMember != NULL )
				{
					RSObject::AddRef( pSuperClass ) ;
					m_pExprParentOf = pSuperClass ;
					pObj = pMember ;
				}
				else
				{
					pMember = pSuperClass->GetVirtualMemberAs( *this, strMember ) ;
					if ( pMember != NULL )
					{
						RSObject::AddRef( m_pThisObj ) ;
						m_pExprParentOf = m_pThisObj ;
						pObj = pMember ;
					}
					else
					{
						ThrowExceptionError
							( strMember + L" メンバが見つかりません" ) ;
						return	NULL ;
					}
				}
				break ;
			}

		default:
			if ( (((RSCodeControl*)pCode)->m_word >= RSCodeControl::wiFirstBasicType)
				&& (((RSCodeControl*)pCode)->m_word <= RSCodeControl::wiLastBasicType) )
			{
				// 基本型
				RSCodeControl::WordIndex	wi = ((RSCodeControl*)pCode)->m_word ;
				pObj = m_pBasicTypeClass[wi - RSCodeControl::wiFirstBasicType] ;
				if ( pObj == NULL )
				{
					ThrowExceptionError
						( SString(RSCodeControl::ControlWordAt(wi))
									+ L" は定義されていない基本型です" ) ;
					return	NULL ;
				}
				pObj->AddRef() ;
			}
			else
			{
				ThrowExceptionError( L"数式中に不正な予約語が含まれています" ) ;
				return	NULL ;
			}
			break ;
		}
		break ;

	case	RSCode::typeOperator:
		ESLAssert( pCode->IsKindOf( ESL_RUNTIME_CLASS(RSCodeOperator) ) ) ;
		{
			RSCodeOperator *	pOpCode = (RSCodeOperator*) pCode ;
			const RSCodeOperator::OperatorInfo &
								opinf = pOpCode->GetOperatorInfo() ;
			if ( opinf.flagsRule & RSCodeOperator::ruleUnary )
			{
				if ( pOpCode->m_operator == RSCodeOperator::opNew )
				{
					// new 演算子
					pObj = ExecuteNewOperator( cs )  ;
					if ( IsException() )
					{
						return	pObj ;
					}
				}
				else
				{
					// 単項演算子
					int	nOpPriority = opinf.priorityUnary ;
					pObj = EvaluateExpression( cs, nOpPriority ) ;
					if ( IsException() )
					{
						return	pObj ;
					}
					pObj = ExecuteUnaryOperator( pObj, pOpCode->m_operator ) ;
				}
			}
			else
			{
				ThrowExceptionError
					( SString(pOpCode->GetOperatorString())
									+ L" 演算子には左辺が必要です" ) ;
				return	NULL ;
			}
		}
		break ;

	case	RSCode::typeParenthesis:
		ESLAssert( pCode->IsKindOf( ESL_RUNTIME_CLASS(RSParenthesis) ) ) ;
		{
			RSParenthesis *	pPrth = (RSParenthesis*) pCode ;
			switch ( pPrth->m_parenthesis )
			{
			case	RSParenthesis::ptParenthesis:
				// ( expr )
				{
					RSCodeStream	csPrth( *pPrth ) ;
					pObj = EvaluateExpression( csPrth, 0 ) ;
					//
					if ( !IsException()
						&& (pObj->GetBasicType() == RSObject::typeClass) )
					{
						// (type) 型キャスト判定
						ESLAssert( pObj->IsKindOf( ESL_RUNTIME_CLASS(RSClass) ) ) ;
						RSClass *	pClass = (RSClass*) pObj ;
						pObj = EvaluateExpression( cs, RSCodeOperator::priorityUnary ) ;
						RSObject *	pCast =
							pClass->CastInstance( *this, pObj, RSClass::castForce ) ;
						if ( pCast == NULL )
						{
							ThrowExceptionError
								( SString(pObj->GetTypeName())
									+ L" から " + pClass->GetRSClassName()
									+ L" へキャストできません" ) ;
						}
						ReleaseObjectRef( pObj ) ;
						ReleaseObjectRef( pClass ) ;
						pObj = pCast ;
					}
				}
				break ;

			case	RSParenthesis::ptBracket:
				// [ expr, expr, ... ]
				pObj = EvaluateArrayExpression( *pPrth ) ;
				break ;

			case	RSParenthesis::ptBrace:
				// { id : expr, ... }
				pObj = EvaluateMapArrayExpression( *pPrth ) ;
				break ;

			default:
				break;
			}
		}
		break ;

	case	RSCode::typeSymbol:
		// 任意シンボル
		ESLAssert( pCode->IsKindOf( ESL_RUNTIME_CLASS(RSCodeSymbol) ) ) ;
		pObj = GetVariableAs( ((RSCodeSymbol*)pCode)->m_symbol ) ;
		if ( pObj == NULL )
		{
			RSClass *	pClass =
				m_pVM->GetClassAs( ((RSCodeSymbol*)pCode)->m_symbol ) ;
			if ( pClass != NULL )
			{
				pClass = ParseGenericClassDecoration( cs, pClass ) ;
				pClass->AddRef() ;
				pObj = pClass ;
			}
		}
		if ( pObj == NULL )
		{
			SString&	strSymbol = ((RSCodeSymbol*)pCode)->m_symbol ;
			if ( strSymbol == L"null" )
			{
				pObj = new_Pointer( NULL ) ;
			}
			else if ( strSymbol == L"true" )
			{
				pObj = new_Boolean( true ) ;
			}
			else if ( strSymbol == L"false" )
			{
				pObj = new_Boolean( false ) ;
			}
			else if ( strSymbol == L"undefined" )
			{
				pObj = new_Reference( NULL ) ;
			}
			else
			{
				ThrowExceptionError
					( SString(L"未定義シンボル \'")
						+ ((RSCodeSymbol*)pCode)->m_symbol + L"\'" ) ;
				return	NULL ;
			}
		}
		break ;

	default:
		ThrowExceptionError( L"内部エラー：未定義コードを解釈できません" ) ;
		return	NULL ;
	}
	while ( !IsException() )
	{
		//
		// 後置演算子／二項演算子判定
		//
		pCode = cs.GetTerm() ;
		if ( pCode == NULL )
		{
			break ;
		}
		if ( pCode->m_type == RSCode::typeOperator )
		{
			ESLAssert( pCode->IsKindOf( ESL_RUNTIME_CLASS(RSCodeOperator) ) ) ;
			RSCodeOperator *	pOpCode = (RSCodeOperator*) pCode ;
			const RSCodeOperator::OperatorInfo &
								opinf = pOpCode->GetOperatorInfo() ;
			if ( opinf.flagsRule & RSCodeOperator::ruleUnaryPost )
			{
				// 後置単項演算子
				if ( opinf.priorityUnaryPost <= priority )
				{
					break ;
				}
				if ( pObj == NULL )
				{
					pObj = new_Pointer( NULL ) ;
				}
				cs.NextTerm() ;
				//
				RSObject *	pDup = pObj->CloneObject( *this ) ;
				pObj = ExecuteUnaryOperator( pObj, pOpCode->m_operator ) ;
				ReleaseObjectRef( pObj ) ;
				pObj = pDup ;
			}
			else if ( opinf.flagsRule & RSCodeOperator::ruleBinary )
			{
				// 二項演算子
				if ( opinf.priorityBinary <= priority )
				{
					break ;
				}
				if ( pObj == nullptr )
				{
					pObj = new_Pointer( nullptr ) ;
				}
				cs.NextTerm() ;
				//
				RSCodeOperator::OperatorIndex	iOp = pOpCode->m_operator ;
				if ( opinf.flagsRule & RSCodeOperator::ruleSpecialRight )
				{
					if ( (iOp == RSCodeOperator::opStaticMemberOf)
								|| (iOp == RSCodeOperator::opMemberOf) )
					{
						// メンバ参照
						pNextCode = cs.NextTerm() ;
						if ( (pNextCode == nullptr)
							|| (pNextCode->m_type != RSCode::typeSymbol) )
						{
							ThrowExceptionError
								( L"数式の書式エラー：メンバが指定されていません" ) ;
							return	pObj ;
						}
						ESLAssert( pNextCode->IsKindOf( ESL_RUNTIME_CLASS(RSCodeSymbol) ) ) ;
						const SString&	strMember = ((RSCodeSymbol*)pNextCode)->m_symbol ;
						ReleaseObjectRef( m_pExprParentOf ) ;
						m_pExprParentOf = pObj ;
						RSObject *	pMember = nullptr ;
						RSClass *	pClass = nullptr ;
						if ( iOp == RSCodeOperator::opStaticMemberOf )
						{
							pClass = ESLTypeCast<RSClass>( pObj ) ;
							ReleaseObjectRef( m_pExprParentOf ) ;
							RSObject::AddRef( pClass ) ;
							m_pExprParentOf = pClass ;
						}
						else
						{
							pClass = pObj->GetEntityClass() ;
						}
						if ( pClass != nullptr )
						{
							pMember = pClass->GetVirtualMemberAs( *this, strMember ) ;
						}
						if ( pMember == nullptr )
						{
							pMember = pObj->GetMemberAs( *this, strMember ) ;
							if ( pMember == nullptr )
							{
								if ( pObj->GetEntityObject() == nullptr )
								{
									ThrowExceptionError
										( SString(L"null ポインタに対して ")
												+ strMember + L" メンバを参照しています",
											L"NullPointerException" ) ;
								}
								else
								{
									ThrowExceptionError
										( strMember + L" メンバが見つかりません" ) ;
								}
								return	nullptr ;
							}
						}
						if ( !pMember->IsEnableAccessModifier
										( RSObject::modifierPublic ) )
						{
							VerifyMemberAccessModifier( pObj, pMember, strMember ) ;
						}
						pObj = pMember ;
					}
					else if ( iOp == RSCodeOperator::opLogicalAnd )
					{
						// expr && expr
						if ( pObj->AsBoolean() )
						{
							ReleaseObjectRef( pObj ) ;
							pObj = EvaluateExpression
										( cs, opinf.priorityBinary ) ;
							if ( (pObj != nullptr) && !IsException() )
							{
								bool	fResult = pObj->AsBoolean() ;
								ReleaseObjectRef( pObj ) ;
								pObj = new_Boolean( fResult ) ;
							}
						}
						else
						{
							PassExpression( cs, opinf.priorityBinary ) ;
							ReleaseObjectRef( pObj ) ;
							pObj = new_Boolean( false ) ;
						}
					}
					else if ( iOp == RSCodeOperator::opLogicalOr )
					{
						// expr || expr
						if ( !pObj->AsBoolean() )
						{
							ReleaseObjectRef( pObj ) ;
							pObj = EvaluateExpression
										( cs, opinf.priorityBinary ) ;
							if ( (pObj != nullptr) && !IsException() )
							{
								bool	fResult = pObj->AsBoolean() ;
								ReleaseObjectRef( pObj ) ;
								pObj = new_Boolean( fResult ) ;
							}
						}
						else
						{
							PassExpression( cs, opinf.priorityBinary ) ;
							ReleaseObjectRef( pObj ) ;
							pObj = new_Boolean( true ) ;
						}
					}
					else if ( iOp == RSCodeOperator::opConditional )
					{
						// expr ? expr : expr
						bool	fCondition = pObj->AsBoolean() ;
						ReleaseObjectRef( pObj ) ;
						if ( fCondition )
						{
							pObj = EvaluateExpression
								( cs, RSCodeOperator::prioritySeparator ) ;
							//
							pNextCode =
								cs.NextOperator( RSCodeOperator::opSeparator ) ;
							if ( pNextCode == NULL )
							{
								ThrowExceptionError
									( L"数式の書式エラー：? に対応する : が見つかりません" ) ;
								return	pObj ;
							}
							PassExpression
								( cs, RSCodeOperator::prioritySelector ) ;
						}
						else
						{
							PassExpression( cs, RSCodeOperator::prioritySeparator ) ;
							//
							pNextCode =
								cs.NextOperator( RSCodeOperator::opSeparator ) ;
							if ( pNextCode == NULL )
							{
								ThrowExceptionError
									( L"数式の書式エラー：? に対応する : が見つかりません" ) ;
								return	pObj ;
							}
							pObj = EvaluateExpression
									( cs, RSCodeOperator::prioritySelector ) ;
						}
					}
					else
					{
						ThrowExceptionError
							( SString(L"内部エラー：")
								+ RSCodeOperator::m_pwszOperators[iOp]
								+ L" は未定義の特殊演算子です" ) ;
						return	pObj ;
					}
				}
				else
				{
					// 一般的な二項演算子
					ReleaseObjectRef( m_pExprParentOf ) ;
					m_pExprParentOf = NULL ;
					//
					int	nOpPriority = opinf.priorityBinary ;
					if ( opinf.flagsRule & RSCodeOperator::ruleRightToLeft )
					{
						nOpPriority -- ;
					}
					RSObject *	pObjRight =
							EvaluateExpression( cs, nOpPriority ) ;
					if ( IsException() )
					{
						break ;
					}
					pObj = ExecuteBinaryOperator
							( pObj, pObjRight, pOpCode->m_operator ) ;
				}
			}
			else
			{
				ThrowExceptionError
					( L"数式の書式エラー：二項演算子の用法が不正です" ) ;
				return	pObj ;
			}
		}
		else if ( pCode->m_type == RSCode::typeParenthesis )
		{
			ESLAssert( pCode->IsKindOf( ESL_RUNTIME_CLASS(RSParenthesis) ) ) ;
			RSParenthesis *	pPrth = (RSParenthesis*) pCode ;
			if ( pPrth->m_parenthesis == RSParenthesis::ptParenthesis )
			{
				//
				// expr(...)
				//
				RSObject *	pObjEntity = NULL ;
				if ( pObj != NULL )
				{
					pObjEntity = pObj->GetEntityObject() ;
				}
				if ( pObjEntity == NULL )
				{
					ThrowExceptionError
						( L"null を関数として呼び出そうとしています" ) ;
					return	pObj ;
				}
				cs.NextTerm() ;
				//
				pObjEntity->AddRef() ;
				ReleaseObjectRef( pObj ) ;
				pObj = pObjEntity ;
				//
				RSObject::BasicType	type = pObj->GetBasicType() ;
				if ( type == RSObject::typeFunction )
				{
					RSObject *	pThisObj = m_pExprParentOf ;
					m_pExprParentOf = NULL ;
					//
					ESLAssert( pObj->IsKindOf( ESL_RUNTIME_CLASS(RSFunctionObject) ) ) ;
					RSFunctionObject *	pFuncObj = (RSFunctionObject*) pObj ;
					RSObject *	pObjArg = new_Array() ;
					EvaluateArgument( *pObjArg, *pPrth ) ;
					if ( IsException() )
					{
						ReleaseObjectRef( pFuncObj ) ;
						ReleaseObjectRef( pObjArg ) ;
						ReleaseObjectRef( pThisObj ) ;
						pObj = NULL ;
						break ;
					}
					pObj = CallFunction( *pFuncObj, pThisObj, *pObjArg, true ) ;
					//
					ReleaseObjectRef( pFuncObj ) ;
					ReleaseObjectRef( pThisObj ) ;
					ReleaseObjectRef( pObjArg ) ;
				}
				else if ( type == RSObject::typeClass )
				{
					ESLAssert( pObj->IsKindOf( ESL_RUNTIME_CLASS(RSClass) ) ) ;
					RSClass *	pClassObj = (RSClass*) pObj ;
					RSObject *	pObjArg = new_Array() ;
					EvaluateArgument( *pObjArg, *pPrth ) ;
					if ( IsException() )
					{
						ReleaseObjectRef( pObj ) ;
						ReleaseObjectRef( pObjArg ) ;
						break ;
					}
					ReleaseObjectRef( m_pExprParentOf ) ;
					m_pExprParentOf = NULL ;
					//
					pObj = pClassObj->NewInstance( *this, pObjArg ) ;
				}
				else
				{
					ThrowExceptionError
						( L"関数ではないオブジェクトの関数の呼び出しです" ) ;
					return	pObj ;
				}
			}
			else if ( pPrth->m_parenthesis == RSParenthesis::ptBracket )
			{
				if ( pObj == NULL )
				{
					ThrowExceptionError
						( L"null の要素を参照しようとしています" ) ;
					return	pObj ;
				}
				cs.NextTerm() ;
				//
				if ( (pObj->GetBasicType() == RSObject::typeClass)
								&& (pPrth->m_terms.GetLength() == 0) )
				{
					//
					// type[]
					//
					ESLAssert( pObj->IsKindOf( ESL_RUNTIME_CLASS(RSClass) ) ) ;
					RSClass *	pClass = (RSClass*) pObj ;
					pClass = m_pVM->GetArrayClassAs( pClass, 1 ) ;
					ReleaseObjectRef( pObj ) ;
					pObj = pClass ;
					pObj->AddRef() ;
				}
				else
				{
					//
					// expr[expr]
					//
					RSCodeStream	csPrth( *pPrth ) ;
					RSObject *		pIndexObj = EvaluateExpression( csPrth, 0 ) ;
					//
					if ( IsException() )
					{
						ReleaseObjectRef( pIndexObj ) ;
						break ;
					}
					if ( pIndexObj == NULL )
					{
						ThrowExceptionError( L"要素の参照で指標が null です" ) ;
						return	pObj ;
					}
					RSObject *	pElementObj = NULL ;
					if ( pIndexObj->IsIntegerType() )
					{
						int64_t	index = 0 ;
						ESLVerify( pIndexObj->AsInteger( index ) ) ;
						if ( (index < 0) || (index > 0x7FFFFFFF)
							|| (index > (int64_t) pObj->GetElementLimit()) )
						{
							ThrowExceptionError( L"指標が範囲を超えています" ) ;
							return	pObj ;
						}
						pElementObj = pObj->GetElementAt( *this, (int) index ) ;
					}
					else
					{
						SString	strElement ;
						if ( pIndexObj->AsString( strElement ) )
						{
							pElementObj = pObj->GetMemberAs( *this, strElement ) ;
							if ( (pElementObj != NULL)
								&& !pElementObj->IsEnableAccessModifier
												( RSObject::modifierPublic ) )
							{
								VerifyMemberAccessModifier( pObj, pElementObj, strElement ) ;
							}
						}
						else
						{
							ReleaseObjectRef( pIndexObj ) ;
							ThrowExceptionError( L"要素の参照で指標型が不正です" ) ;
							return	pObj ;
						}
					}
					ReleaseObjectRef( pIndexObj ) ;
					ReleaseObjectRef( pObj ) ;
					pObj = pElementObj ;
				}
			}
			else
			{
				ThrowExceptionError( L"数式の書式エラー：{} 括弧が不正です" ) ;
				return	pObj ;
			}
		}
		else
		{
			ThrowExceptionError( L"数式の書式エラー：項間に演算子がありません" ) ;
			return	pObj ;
		}
		if ( pObj == NULL )
		{
			pObj = new_Pointer( NULL ) ;
		}
	}
	return	pObj ;
}

RSObject * RSContext::EvaluateExpression
	( SSystem::SStringParser& sparsExpr, SSystem::SParserErrorInterface* pperr )
{
	//
	// 文解釈
	//
	RSScript *	prsScript = new RSScript ;
	SError		err ;
	if ( pperr != NULL )
	{
		err = prsScript->ParseScript
			( *(m_pVM->LockMacroContext()), sparsExpr, *pperr ) ;
	}
	else
	{
		SParserErrorTracer	perrTracer ;
		err = prsScript->ParseScript
			( *(m_pVM->LockMacroContext()), sparsExpr, perrTracer ) ;
	}
	m_pVM->UnlockMacroContext() ;
	if ( err )
	{
		return	NULL ;
	}
	//
	// 数式評価
	//
	RSCodeStream	cs ;
	cs.AttachCode( *prsScript ) ;
	//
	RSObject *	pObj = EvaluateExpression( cs, 0 ) ;
	if ( pperr != NULL )
	{
		OutputExceptionError( *pperr ) ;
	}
	RSScriptOwner *	pScriptOwner = ESLTypeCast<RSScriptOwner>( pObj ) ;
	if ( pScriptOwner != nullptr )
	{
		pScriptOwner->SetOwnScript( prsScript ) ;
	}
	else
	{
		delete	prsScript ;
	}
	return	pObj ;
}

RSObject * RSContext::EvaluateExpression
	( const wchar_t * pwszExpr, SSystem::SParserErrorInterface* pperr )
{
	RSSourceParser	sparsExpr = pwszExpr ;
	return	EvaluateExpression( sparsExpr, pperr ) ;
}

// 数式を読み飛ばす
//////////////////////////////////////////////////////////////////////////////
void RSContext::PassExpression( RSCodeStream& cs, int priority )
{
	RSCode *	pCode = cs.NextTerm() ;
	if ( pCode == NULL )
	{
		return ;
	}
	RSCode *	pNextCode ;
	switch ( pCode->m_type )
	{
	case	RSCode::typeLiteral:
		// リテラル
		ESLAssert( pCode->IsKindOf( ESL_RUNTIME_CLASS(RSCodeLiteral) ) ) ;
		ESLAssert( ((RSCodeLiteral*)pCode)->m_literal != NULL ) ;
		break ;

	case	RSCode::typeControlCode:
		ESLAssert( pCode->IsKindOf( ESL_RUNTIME_CLASS(RSCodeControl) ) ) ;
		switch ( ((RSCodeControl*)pCode)->m_word )
		{
		case	RSCodeControl::wiThis:
			// this
			break ;

		case	RSCodeControl::wiFunction:
			// function(...){...}
			if ( cs.NextParenthesis( RSParenthesis::ptParenthesis ) != NULL )
			{
				while ( !cs.IsEndOfStream() )
				{
					pNextCode = cs.NextTerm() ;
					if ( (pNextCode != NULL)
						&& (pNextCode->m_type == RSCode::typeParenthesis)
						&& (((RSParenthesis*)pNextCode)->
								m_parenthesis == RSParenthesis::ptBrace) )
					{
						break ;
					}
					if ( (pNextCode != NULL)
						&& (pNextCode->m_type == RSCode::typeOperator)
						&& (((RSCodeOperator*)pNextCode)->
								m_operator == RSCodeOperator::opEndOfStatement) )
					{
						break ;
					}
				}
			}
			break ;

		case	RSCodeControl::wiSuper:
			// super
			break ;

		default:
			return ;
		}
		break ;

	case	RSCode::typeOperator:
		ESLAssert( pCode->IsKindOf( ESL_RUNTIME_CLASS(RSCodeOperator) ) ) ;
		{
			RSCodeOperator *	pOpCode = (RSCodeOperator*) pCode ;
			const RSCodeOperator::OperatorInfo &
								opinf = pOpCode->GetOperatorInfo() ;
			if ( opinf.flagsRule & RSCodeOperator::ruleUnary )
			{
				// 単項演算子
				PassExpression( cs, opinf.priorityUnary ) ;
			}
			else
			{
				return ;
			}
		}
		break ;

	case	RSCode::typeParenthesis:
		ESLAssert( pCode->IsKindOf( ESL_RUNTIME_CLASS(RSParenthesis) ) ) ;
		break ;

	case	RSCode::typeSymbol:
		// 任意シンボル
		ESLAssert( pCode->IsKindOf( ESL_RUNTIME_CLASS(RSCodeSymbol) ) ) ;
		break ;

	default:
		return ;
	}
	for ( ; ; )
	{
		//
		// 後置演算子／二項演算子判定
		//
		pCode = cs.GetTerm() ;
		if ( pCode == NULL )
		{
			break ;
		}
		if ( pCode->m_type == RSCode::typeOperator )
		{
			ESLAssert( pCode->IsKindOf( ESL_RUNTIME_CLASS(RSCodeOperator) ) ) ;
			RSCodeOperator *	pOpCode = (RSCodeOperator*) pCode ;
			const RSCodeOperator::OperatorInfo &
								opinf = pOpCode->GetOperatorInfo() ;
			if ( opinf.flagsRule & RSCodeOperator::ruleUnaryPost )
			{
				// 後置単項演算子
				if ( opinf.priorityUnaryPost <= priority )
				{
					break ;
				}
				cs.NextTerm() ;
			}
			else if ( opinf.flagsRule & RSCodeOperator::ruleBinary )
			{
				// 二項演算子
				if ( opinf.priorityBinary <= priority )
				{
					break ;
				}
				cs.NextTerm() ;
				//
				RSCodeOperator::OperatorIndex	iOp = pOpCode->m_operator ;
				if ( (iOp == RSCodeOperator::opStaticMemberOf)
							|| (iOp == RSCodeOperator::opMemberOf) )
				{
					// メンバ参照
					pNextCode = cs.NextTerm() ;
				}
				else if ( iOp == RSCodeOperator::opConditional )
				{
					// expr ? expr : expr
					PassExpression( cs, RSCodeOperator::prioritySeparator ) ;
					cs.NextOperator( RSCodeOperator::opSeparator ) ;
					PassExpression( cs, RSCodeOperator::prioritySelector ) ;
				}
				else
				{
					// 一般的な二項演算子
					int	nOpPriority = opinf.priorityBinary ;
					if ( opinf.flagsRule & RSCodeOperator::ruleRightToLeft )
					{
						nOpPriority -- ;
					}
					PassExpression( cs, nOpPriority ) ;
				}
			}
			else
			{
				return ;
			}
		}
		else if ( pCode->m_type == RSCode::typeParenthesis )
		{
			ESLAssert( pCode->IsKindOf( ESL_RUNTIME_CLASS(RSParenthesis) ) ) ;
			RSParenthesis *	pPrth = (RSParenthesis*) pCode ;
			if ( pPrth->m_parenthesis == RSParenthesis::ptParenthesis )
			{
				// expr(...)
				cs.NextTerm() ;
			}
			else if ( pPrth->m_parenthesis == RSParenthesis::ptBracket )
			{
				// expr[expr]
				cs.NextTerm() ;
			}
			else
			{
				return ;
			}
		}
		else
		{
			return ;
		}
	}
}

// 型式を評価
//////////////////////////////////////////////////////////////////////////////
RSClass * RSContext::ParseClassExpression( RSCodeStream& cs )
{
	size_t		iCode = cs.GetIndex() ;
	RSObject *	pTypeObj = ParseTypeExpression( cs ) ;
	if ( pTypeObj == NULL )
	{
		cs.SeekIndex( iCode ) ;
		return	NULL ;
	}
	if ( pTypeObj->GetBasicType() != RSObject::typeClass )
	{
		cs.SeekIndex( iCode ) ;
		pTypeObj->ReleaseRef() ;
		return	NULL ;
	}
	ESLAssert( pTypeObj->IsKindOf( ESL_RUNTIME_CLASS(RSClass) ) ) ;
	pTypeObj->ReleaseRef() ;		// RSClass は VM によって
									// AddRef されているので
									// ここでは解放されない
	return	ParseClassArrayDecoration( cs, (RSClass*) pTypeObj ) ;
}

RSObject * RSContext::ParseTypeExpression( RSCodeStream& cs )
{
	size_t	iStream = cs.GetIndex() ;
	//
	RSCodeSymbol *	pSymbol = cs.NextSymbol() ;
	if ( pSymbol == NULL )
	{
		RSCodeControl *	pCodeWord = cs.NextControlWord() ;
		if ( pCodeWord != NULL )
		{
			if ( (pCodeWord->m_word >= RSCodeControl::wiFirstBasicType)
				&& (pCodeWord->m_word <= RSCodeControl::wiLastBasicType) )
			{
				RSClass *	pClass =
					m_pBasicTypeClass
						[pCodeWord->m_word - RSCodeControl::wiFirstBasicType] ;
				pClass->AddRef() ;
				return	pClass ;
			}
		}
		return	NULL ;
	}
	m_iSrcStatement = pSymbol->m_iSrc ;
	//
	RSClass *	pClass = GetClassAs( pSymbol->m_symbol ) ;
	RSObject *	pTypeObj = pClass ;
	if ( pClass != NULL )
	{
		pTypeObj = ParseGenericClassDecoration( cs, pClass ) ;
		pTypeObj->AddRef() ;
	}
	else
	{
		pTypeObj = GetVariableAs( pSymbol->m_symbol ) ;
		if ( pTypeObj == NULL )
		{
			cs.SeekIndex( iStream ) ;
			return	NULL ;
		}
	}
	//
	// class.class 判定
	while ( cs.NextOperator( RSCodeOperator::opStaticMemberOf )
				|| cs.NextOperator( RSCodeOperator::opMemberOf ) )
	{
		pSymbol = cs.NextSymbol() ;
		if ( pSymbol == NULL )
		{
			cs.SeekIndex( iStream ) ;
			pTypeObj->ReleaseRef() ;
			return	NULL ;
		}
		RSObject *	pMember =
				pTypeObj->GetMemberAs( *this, pSymbol->m_symbol ) ;
		pTypeObj->ReleaseRef() ;
		if ( pMember == NULL )
		{
			cs.SeekIndex( iStream ) ;
			RSObject::ReleaseRef( pMember ) ;
			return	NULL ;
		}
		pTypeObj = pMember ;
	}
	return	pTypeObj ;
}

// 組み込みジェネリック型修飾
//////////////////////////////////////////////////////////////////////////////
RSClass * RSContext::ParseGenericClassDecoration
					( RSCodeStream& cs, RSClass * pClass )
{
	size_t	iCode = cs.GetIndex() ;
	if ( (pClass == GetDynamicObjectClass())
		&& cs.NextOperator( RSCodeOperator::opLessThan ) )
	{
		// Hash<type> 判定
		RSClass *	pElementType = ParseClassExpression( cs ) ;
		if ( (pElementType != NULL)
			&& cs.NextOperator( RSCodeOperator::opGraterThan ) )
		{
			pClass = m_pVM->GetHashMapClassAs( pElementType ) ;
		}
		else
		{
			cs.SeekIndex( iCode ) ;
		}
	}
	else if ( (pClass == GetFunctionClass())
		&& cs.NextOperator( RSCodeOperator::opLessThan ) )
	{
		// Function<type,...> 判定
		RSFunctionPrototype	proto ;
		//
		if ( cs.NextControlWord( RSCodeControl::wiVoid ) == NULL )
		{
			RSClass *	pRetType = ParseClassExpression( cs ) ;
			proto.SetReturnType( pRetType ) ;
		}
		if ( cs.NextOperator( RSCodeOperator::opSequencing ) )
		{
			proto.m_pNamespaceClass = ParseClassExpression( cs ) ;
		}
		int	nArg = 0 ;
		while ( cs.NextOperator( RSCodeOperator::opSequencing ) )
		{
			RSClass *	pArgType = ParseClassExpression( cs ) ;
			proto.AddArgument
				( pArgType, SString(L"a") + SString( nArg ) ) ;
		}
		if ( cs.NextOperator( RSCodeOperator::opGraterThan ) )
		{
			pClass = m_pVM->GetFunctionClassAs( proto ) ;
		}
		else
		{
			cs.SeekIndex( iCode ) ;
		}
	}
	return	pClass ;
}

// 配列型修飾
//////////////////////////////////////////////////////////////////////////////
RSClass * RSContext::ParseClassArrayDecoration
						( RSCodeStream& cs, RSClass * pClass )
{
	int	nArrayDimesion = 0 ;
	for ( ; ; )
	{
		RSParenthesis *	pPrth =
				cs.NextParenthesis( RSParenthesis::ptBracket ) ;
		if ( pPrth == NULL )
		{
			break ;
		}
		if ( pPrth->m_terms.GetLength() != 0 )
		{
			ThrowExceptionError( L"配列型の記述に配列長が含まれています" ) ;
		}
		nArrayDimesion ++ ;
	}
	if ( nArrayDimesion != 0 )
	{
		pClass = m_pVM->GetArrayClassAs( pClass, nArrayDimesion ) ;
	}
	return	pClass ;
}

// 配列リテラル評価  [ expr, expr, ... ]
//////////////////////////////////////////////////////////////////////////////
RSObject * RSContext::EvaluateArrayExpression( RSParenthesis& prth )
{
	RSCodeStream	cs ;
	cs.AttachCode( prth ) ;
	//
	RSObject *	pArray = new_Array() ;
	RSClass *	pElementClass = NULL ;
	int			iArray = 0 ;
	while ( !cs.IsEndOfStream() && !IsException() )
	{
		RSObject *	pObj =
			EvaluateExpression( cs, RSCodeOperator::priorityList ) ;
		if ( iArray == 0 )
		{
			if ( pObj != NULL )
			{
				pElementClass = pObj->GetRSClass() ;
			}
		}
		else
		{
			if ( pElementClass != pObj->GetRSClass() )
			{
				pElementClass = NULL ;
			}
		}
		ReleaseObjectRef
			( pArray->SetElementAt( *this, iArray ++, pObj ) ) ;
		//
		if ( cs.NextOperator( RSCodeOperator::opSequencing ) == NULL )
		{
			if ( cs.NextTerm() != NULL )
			{
				ThrowExceptionError
					( L"配列要素が \',\' が区切られていません" ) ;
			}
			break ;
		}
	}
	if ( pElementClass != NULL )
	{
		RSArray *	pArrayObj = ESLTypeCast<RSArray>( pArray ) ;
		if ( pArrayObj != NULL )
		{
			pArrayObj->SetArrayPrototype( 0x7FFFFFFF, pElementClass ) ;
			pArrayObj->SetRSClass
				( m_pVM->GetArrayClassAs( pElementClass ) ) ;
		}
	}
	return	pArray ;
}

// 連想配列リテラル評価  { id : expr, id : expr, ... }
//////////////////////////////////////////////////////////////////////////////
RSObject * RSContext::EvaluateMapArrayExpression( RSParenthesis& prth )
{
	RSCodeStream	cs ;
	cs.AttachCode( prth ) ;
	//
	RSDynamicObject *	pObject = new RSDynamicObject( m_pJSObjectClass ) ;
	SString				strId ;
	RSClass *			pElementClass = nullptr ;
	size_t				iElement = 0 ;
	while ( !cs.IsEndOfStream() && !IsException() )
	{
		RSCodeSymbol *	pSymbol = cs.NextSymbol() ;
		if ( pSymbol != NULL )
		{
			strId = pSymbol->m_symbol ;
		}
		else
		{
			RSObject *	pObjId =
				EvaluateExpression( cs, RSCodeOperator::prioritySeparator ) ;
			if ( IsException() )
			{
				break ;
			}
			if ( !pObjId->AsString( strId ) )
			{
				ThrowExceptionError( L"連想配列キーを評価できません" ) ;
				ReleaseObjectRef( pObjId ) ;
				break ;
			}
			ReleaseObjectRef( pObjId ) ;
		}
		if ( cs.NextOperator( RSCodeOperator::opSeparator ) == NULL )
		{
			ThrowExceptionError
				( L"連想配列キーが \':\' で値と区切られていません" ) ;
			break ;
		}
		RSObject *	pObj =
			EvaluateExpression( cs, RSCodeOperator::priorityList ) ;
		ReleaseObjectRef
			( pObject->SetMemberAs( *this, strId, pObj ) ) ;
		//
		if ( pObj != nullptr )
		{
			if ( iElement == 0 )
			{
				pElementClass = pObj->GetRSClass() ;
			}
			else
			{
				if ( pElementClass != pObj->GetRSClass() )
				{
					pElementClass = nullptr ;
				}
			}
		}
		iElement ++ ;
		//
		if ( cs.NextOperator( RSCodeOperator::opSequencing ) == NULL )
		{
			if ( cs.NextTerm() != NULL )
			{
				ThrowExceptionError
					( L"連想配列要素が \',\' が区切られていません" ) ;
			}
			break ;
		}
	}
	if ( pElementClass != nullptr )
	{
		pObject->SetRSClass( m_pVM->GetHashMapClassAs( pElementClass ) ) ;
//		pObject->SetMemberClass( pElementClass ) ;
	}
	return	pObject ;
}

// 関数リテラル評価  ( arg, arg, ... ) [: <ret-type>] [-> <class>] { statements }
//////////////////////////////////////////////////////////////////////////////
RSObject * RSContext::EvaluateFunctionObject( RSCodeStream& cs )
{
	RSParenthesis *	pPrth =
			cs.NextParenthesis( RSParenthesis::ptParenthesis ) ;
	if ( pPrth == NULL )
	{
		ThrowExceptionError( L"関数に引数がありません" ) ;
		return	NULL ;
	}
	RSFunctionPrototype *	pProto = new RSFunctionPrototype ;
	SParserErrorLogger		perrLog ;
	RSCodeStream			csArg( *pPrth ) ;
	if ( pProto->ParseArgument( *this, csArg, perrLog ) )
	{
		SParserErrorLogger::ErrorLog *	pLog = perrLog.GetErrorLogAt( 0 ) ;
		if ( pLog != NULL )
		{
			ThrowExceptionError( pLog->m_strError ) ;
		}
		else
		{
			ThrowExceptionError( L"関数の引数の書式が不正です" ) ;
		}
		delete	pProto ;
		return	NULL ;
	}
	if ( cs.NextOperator( RSCodeOperator::opSeparator ) != NULL )
	{
		RSClass *	pRetType = ParseClassExpression( cs ) ;
		if ( pRetType != NULL )
		{
			pProto->SetReturnType( pRetType ) ;
		}
	}
	if ( cs.NextOperator( RSCodeOperator::opPointerMemberOf ) != NULL )
	{
		RSClass *	pThisClass = ParseClassExpression( cs ) ;
		if ( pThisClass != NULL )
		{
			pProto->m_pNamespaceClass = pThisClass ;
		}
	}
	pProto->m_pParenthesis = cs.NextParenthesis( RSParenthesis::ptBrace ) ;
	if ( pProto->m_pParenthesis == NULL )
	{
		ThrowExceptionError( L"関数の実装がありません" ) ;
		delete	pProto ;
		return	NULL ;
	}
	pProto->SetRefNamespace( GetCurrentNamespace() ) ;
	//
	RSFunctionObject *	pFuncObj =
		new RSFunctionObject
			( pProto, m_pVM->GetFunctionClassAs( *pProto ) ) ;
	ReleaseObjectRef
		( pFuncObj->CreateMemberAs
			( *this, L"prototype", new RSDynamicObject( NULL ) ) ) ;
	return	pFuncObj ;
}

// new 演算子実行
//////////////////////////////////////////////////////////////////////////////
RSObject * RSContext::ExecuteNewOperator( RSCodeStream& cs )
{
	//
	// new Type[]...[](...)
	//
	RSObject *	pTypeObj = ParseTypeExpression( cs ) ;
	if ( pTypeObj == NULL )
	{
		ThrowExceptionError( L"new 演算子の型指定が不正です" ) ;
		return	NULL ;
	}
	size_t	iArrayDimStart = cs.GetIndex() ;
	int		nArrayDimension = 0 ;
	while ( cs.NextParenthesis
				( RSParenthesis::ptBracket ) != NULL )
	{
		nArrayDimension ++ ;
	}
	RSObject *	pObj = NULL ;
	RSObject *	pObjArg = new_Array() ;
	RSParenthesis *	pPrthArg =
		cs.NextParenthesis( RSParenthesis::ptParenthesis ) ;
	if ( pPrthArg != NULL )
	{
		EvaluateArgument( *pObjArg, *pPrthArg ) ;
		if ( IsException() )
		{
			ReleaseObjectRef( pObjArg ) ;
			ReleaseObjectRef( pTypeObj ) ;
			return	NULL ;
		}
	}
	if ( pTypeObj->GetBasicType() == RSObject::typeClass )
	{
		// Java 風インスタンス生成
		ESLAssert( pTypeObj->IsKindOf( ESL_RUNTIME_CLASS(RSClass) ) ) ;
		RSClass *	pClass = (RSClass*) pTypeObj ;
		if ( nArrayDimension > 0 )
		{
			pClass = m_pVM->GetArrayClassAs( pClass, nArrayDimension - 1 ) ;
			//
			size_t	iSaveIndex = cs.GetIndex() ;
			cs.SeekIndex( iArrayDimStart ) ;
			//
			RSParenthesis *	pBracket =
				cs.NextParenthesis( RSParenthesis::ptBracket ) ;
			ESLAssert( pBracket != NULL ) ;
			size_t	nLimit = 0x7FFFFFFF ;
			if ( pBracket->m_terms.GetLength() > 0 )
			{
				RSCodeStream	csLen( *pBracket ) ;
				RSObject *	pObjLen = EvaluateExpression( csLen ) ;
				if ( IsException() )
				{
					ReleaseObjectRef( pObjArg ) ;
					ReleaseObjectRef( pTypeObj ) ;
					ReleaseObjectRef( pObjLen ) ;
					return	NULL ;
				}
				int64_t	num ;
				if ( pObjLen->AsInteger( num ) )
				{
					nLimit = (size_t) num ;
				}
				ReleaseObjectRef( pObjLen ) ;
			}
			cs.SeekIndex( iSaveIndex ) ;
			//
			pObj = new_Array( nLimit, pClass ) ;
		}
		else
		{
			pObj = pClass->NewInstance( *this, pObjArg ) ;
		}
	}
	else
	{
		// JavaScript 風インスタンス生成
		if ( nArrayDimension > 0 )
		{
			ThrowExceptionError
				( L"new で配列型の指定が不正です" ) ;
			return	NULL ;
		}
		RSObject *	pPrototypeObj =
			pTypeObj->GetMemberAs( *this, L"prototype" ) ;
		if ( pPrototypeObj != NULL )
		{
			RSObject *	pPrototypeTemp = pPrototypeObj->GetEntityObject() ;
			if ( pPrototypeTemp != NULL )
			{
				pObj = pPrototypeTemp->CloneObject( *this ) ;
			}
			else
			{
				ReleaseObjectRef( pPrototypeObj ) ;
				pObj = new RSDynamicObject( NULL ) ;
			}
		}
		else
		{
			pObj = new RSDynamicObject( NULL ) ;
		}
		if ( pTypeObj->GetBasicType() == RSObject::typeFunction )
		{
			ESLAssert( pTypeObj->IsKindOf( ESL_RUNTIME_CLASS(RSFunctionObject) ) ) ;
			RSFunctionObject *	pFuncObj = (RSFunctionObject*) pTypeObj ;
			RSObject *	pNewObj = CallFunction( *pFuncObj, pObj, *pObjArg, false ) ;
			if ( pNewObj != NULL )
			{
				ReleaseObjectRef( pObj ) ;
				pObj = pNewObj ;
			}
		}
	}
	ReleaseObjectRef( pObjArg ) ;
	ReleaseObjectRef( pTypeObj ) ;
	return	pObj ;
}

// 単項演算子実行
//////////////////////////////////////////////////////////////////////////////
RSObject * RSContext::ExecuteUnaryOperator
	( RSObject * pObj, RSCodeOperator::OperatorIndex opIndex )
{
	RSObject *	pResult = NULL ;
	switch ( opIndex )
	{
	case	RSCodeOperator::opAdd:
		pResult = pObj->OperatorPlus( *this ) ;
		break ;
	case	RSCodeOperator::opSub:
		pResult = pObj->OperatorNegate( *this ) ;
		break ;
	case	RSCodeOperator::opBitNot:
		pResult = pObj->OperatorBitNot( *this ) ;
		break ;
	case	RSCodeOperator::opIncrement:
		if ( pObj->GetModifiers() & RSObject::modifierConst )
		{
			ThrowExceptionError
				( L"const オブジェクトへのインクリメントです" ) ;
		}
		else
		{
			pResult = pObj->OperatorIncrement( *this ) ;
		}
		break ;
	case	RSCodeOperator::opDecrement:
		if ( pObj->GetModifiers() & RSObject::modifierConst )
		{
			ThrowExceptionError
				( L"const オブジェクトへのデクリメントです" ) ;
		}
		else
		{
			pResult = pObj->OperatorDecrement( *this ) ;
		}
		break ;
	case	RSCodeOperator::opLogicalNot:
		pResult = new_Boolean( !pObj->AsBoolean() ) ;
		break ;
	case	RSCodeOperator::opNew:
		pResult = pObj ;
		RSObject::AddRef( pResult ) ;
		break ;
	default:
		ThrowExceptionError
			( SString(L"内部エラー：")
				+ RSCodeOperator::m_pwszOperators[opIndex]
				+ L" は定義されない単項演算子です" ) ;
		return	NULL ;
	}
	if ( (pResult == NULL) && !IsException() )
	{
		ThrowExceptionError
			( SString(RSCodeOperator::m_pwszOperators[opIndex])
						+ L" 単項演算子は評価されませんでした" ) ;
	}
	else
	{
		ReleaseObjectRef( pObj ) ;
	}
	return	pResult ;
}

// 二項演算子実行
//////////////////////////////////////////////////////////////////////////////
RSObject * RSContext::ExecuteBinaryOperator
	( RSObject * pObjLeft,
		RSObject * pObjRight, RSCodeOperator::OperatorIndex opIndex )
{
	RSObject *	pResult = NULL ;
	RSObject *	pEntityLeft = NULL ;
	RSObject *	pEntityRight = NULL ;
	if ( (pObjLeft == NULL) || (pObjRight == NULL) )
	{
		pEntityLeft = (pObjLeft != NULL) ? pObjLeft->GetEntityObject() : NULL ;
		pEntityRight = (pObjRight != NULL) ? pObjRight->GetEntityObject() : NULL ;
		if ( opIndex == RSCodeOperator::opPointerEqual )
		{
			pResult = new_Boolean( pEntityLeft == pEntityRight ) ;
		}
		else if ( opIndex == RSCodeOperator::opPointerNotEqual )
		{
			pResult = new_Boolean( pEntityLeft != pEntityRight ) ;
		}
		else
		{
			ThrowExceptionError
				( SString(L"null への演算子 ")
					+ RSCodeOperator::m_pwszOperators[opIndex]
					+ L" は定義されていません" ) ;
		}
		ReleaseObjectRef( pObjLeft ) ;
		ReleaseObjectRef( pObjRight ) ;
		return	pResult ;
	}
	switch ( opIndex )
	{
	case	RSCodeOperator::opAdd:
		pResult = pObjLeft->OperatorAdd( *this, pObjRight ) ;
		break ;
	case	RSCodeOperator::opSub:
		pResult = pObjLeft->OperatorSub( *this, pObjRight ) ;
		break ;
	case	RSCodeOperator::opMul:
		pResult = pObjLeft->OperatorMul( *this, pObjRight ) ;
		break ;
	case	RSCodeOperator::opDiv:
		pResult = pObjLeft->OperatorDiv( *this, pObjRight ) ;
		break ;
	case	RSCodeOperator::opMod:
		pResult = pObjLeft->OperatorMod( *this, pObjRight ) ;
		break ;
	case	RSCodeOperator::opBitAnd:
		pResult = pObjLeft->OperatorBitAnd( *this, pObjRight ) ;
		break ;
	case	RSCodeOperator::opBitOr:
		pResult = pObjLeft->OperatorBitOr( *this, pObjRight ) ;
		break ;
	case	RSCodeOperator::opBitXor:
		pResult = pObjLeft->OperatorBitXor( *this, pObjRight ) ;
		break ;
	case	RSCodeOperator::opShiftRight:
		pResult = pObjLeft->OperatorBitShiftRight( *this, pObjRight ) ;
		break ;
	case	RSCodeOperator::opShiftLeft:
		pResult = pObjLeft->OperatorShiftLeft( *this, pObjRight ) ;
		break ;
	case	RSCodeOperator::opShiftRightArithmetic:
		pResult = pObjLeft->OperatorShiftRight( *this, pObjRight ) ;
		break ;
	case	RSCodeOperator::opEqual:
		pResult = pObjLeft->OperatorCompareEQ( *this, pObjRight ) ;
		break ;
	case	RSCodeOperator::opNotEqual:
		pResult = pObjLeft->OperatorCompareNE( *this, pObjRight ) ;
		break ;
	case	RSCodeOperator::opLessEqual:
		pResult = pObjLeft->OperatorCompareLE( *this, pObjRight ) ;
		break ;
	case	RSCodeOperator::opLessThan:
		pResult = pObjLeft->OperatorCompareLT( *this, pObjRight ) ;
		break ;
	case	RSCodeOperator::opGraterEqual:
		pResult = pObjLeft->OperatorCompareGE( *this, pObjRight ) ;
		break ;
	case	RSCodeOperator::opGraterThan:
		pResult = pObjLeft->OperatorCompareGT( *this, pObjRight ) ;
		break ;
	case	RSCodeOperator::opPointerEqual:
		pResult = new_Boolean
			( pObjLeft->GetEntityObject() == pObjRight->GetEntityObject() ) ;
		break ;
	case	RSCodeOperator::opPointerNotEqual:
		pResult = new_Boolean
			( pObjLeft->GetEntityObject() != pObjRight->GetEntityObject() ) ;
		break ;
	case	RSCodeOperator::opLogicalAnd:
		pResult = new_Boolean( pObjLeft->AsBoolean() && pObjRight->AsBoolean() ) ;
		break ;
	case	RSCodeOperator::opLogicalOr:
		pResult = new_Boolean( pObjLeft->AsBoolean() || pObjRight->AsBoolean() ) ;
		break ;
	case	RSCodeOperator::opMove:
		if ( pObjLeft->GetModifiers() & RSObject::modifierConst )
		{
			ThrowExceptionError( L"const オブジェクトへの代入です" ) ;
		}
		else
		{
			pResult = pObjLeft->OperatorMove( *this, pObjRight ) ;
		}
		break ;
	case	RSCodeOperator::opMoveAdd:
		if ( pObjLeft->GetModifiers() & RSObject::modifierConst )
		{
			ThrowExceptionError( L"const オブジェクトへの代入です" ) ;
		}
		else
		{
			pResult = pObjLeft->OperatorMoveAdd( *this, pObjRight ) ;
		}
		break ;
	case	RSCodeOperator::opMoveSub:
		if ( pObjLeft->GetModifiers() & RSObject::modifierConst )
		{
			ThrowExceptionError( L"const オブジェクトへの代入です" ) ;
		}
		else
		{
			pResult = pObjLeft->OperatorMoveSub( *this, pObjRight ) ;
		}
		break ;
	case	RSCodeOperator::opMoveMul:
		if ( pObjLeft->GetModifiers() & RSObject::modifierConst )
		{
			ThrowExceptionError( L"const オブジェクトへの代入です" ) ;
		}
		else
		{
			pResult = pObjLeft->OperatorMoveMul( *this, pObjRight ) ;
		}
		break ;
	case	RSCodeOperator::opMoveDiv:
		if ( pObjLeft->GetModifiers() & RSObject::modifierConst )
		{
			ThrowExceptionError( L"const オブジェクトへの代入です" ) ;
		}
		else
		{
			pResult = pObjLeft->OperatorMoveDiv( *this, pObjRight ) ;
		}
		break ;
	case	RSCodeOperator::opMoveMod:
		if ( pObjLeft->GetModifiers() & RSObject::modifierConst )
		{
			ThrowExceptionError( L"const オブジェクトへの代入です" ) ;
		}
		else
		{
			pResult = pObjLeft->OperatorMoveMod( *this, pObjRight ) ;
		}
		break ;
	case	RSCodeOperator::opMoveBitAnd:
		if ( pObjLeft->GetModifiers() & RSObject::modifierConst )
		{
			ThrowExceptionError( L"const オブジェクトへの代入です" ) ;
		}
		else
		{
			pResult = pObjLeft->OperatorMoveBitAnd( *this, pObjRight ) ;
		}
		break ;
	case	RSCodeOperator::opMoveBitOr:
		if ( pObjLeft->GetModifiers() & RSObject::modifierConst )
		{
			ThrowExceptionError( L"const オブジェクトへの代入です" ) ;
		}
		else
		{
			pResult = pObjLeft->OperatorMoveBitOr( *this, pObjRight ) ;
		}
		break ;
	case	RSCodeOperator::opMoveBitXor:
		if ( pObjLeft->GetModifiers() & RSObject::modifierConst )
		{
			ThrowExceptionError( L"const オブジェクトへの代入です" ) ;
		}
		else
		{
			pResult = pObjLeft->OperatorMoveBitXor( *this, pObjRight ) ;
		}
		break ;
	case	RSCodeOperator::opMoveShiftRight:
		if ( pObjLeft->GetModifiers() & RSObject::modifierConst )
		{
			ThrowExceptionError( L"const オブジェクトへの代入です" ) ;
		}
		else
		{
			pResult = pObjLeft->OperatorMoveBitShiftRight( *this, pObjRight ) ;
		}
		break ;
	case	RSCodeOperator::opMoveShiftLeft:
		if ( pObjLeft->GetModifiers() & RSObject::modifierConst )
		{
			ThrowExceptionError( L"const オブジェクトへの代入です" ) ;
		}
		else
		{
			pResult = pObjLeft->OperatorMoveShiftLeft( *this, pObjRight ) ;
		}
		break ;
	case	RSCodeOperator::opMoveShiftRightArithmetic:
		if ( pObjLeft->GetModifiers() & RSObject::modifierConst )
		{
			ThrowExceptionError( L"const オブジェクトへの代入です" ) ;
		}
		else
		{
			pResult = pObjLeft->OperatorMoveShiftRight( *this, pObjRight ) ;
		}
		break ;
	case	RSCodeOperator::opStaticMemberOf:
	case	RSCodeOperator::opMemberOf:
		ThrowExceptionError
			( SString(L"内部エラー：")
				+ RSCodeOperator::m_pwszOperators[opIndex]
						+ L" は定義されない二項演算子です" ) ;
		break ;
	case	RSCodeOperator::opMemberCallOf:
		pObjLeft->AddRef() ;
		pObjRight->AddRef() ;
		ReleaseObjectRef( m_pExprParentOf ) ;
		m_pExprParentOf = pObjLeft ;
		pResult = pObjRight ;
		break ;
	case	RSCodeOperator::opInstanceOf:
		pEntityRight = pObjRight->GetEntityObject() ;
		if ( (pEntityRight != NULL)
			&& (pEntityRight->GetBasicType() == RSObject::typeClass) )
		{
			pResult = new_Boolean
				( pObjLeft->InstanceOf
					( (RSClass*) pEntityRight ) != NULL ) ;
		}
		else
		{
			ThrowExceptionError
				( L"instanceof の右辺が型指定ではありません" ) ;
		}
		break ;
	case	RSCodeOperator::opSequencing:
		pResult = pObjRight ;
		pResult->AddRef() ;
		break ;
	case	RSCodeOperator::opBitNot:
	case	RSCodeOperator::opIncrement:
	case	RSCodeOperator::opDecrement:
	case	RSCodeOperator::opLogicalNot:
	case	RSCodeOperator::opNew:
	case	RSCodeOperator::opConditional:
	case	RSCodeOperator::opSeparator:
	case	RSCodeOperator::opEndOfStatement:
	default:
		ThrowExceptionError
			( SString(RSCodeOperator::m_pwszOperators[opIndex])
								+ L" は定義されない二項演算子です" ) ;
		break ;
	}
	ReleaseObjectRef( pObjLeft ) ;
	ReleaseObjectRef( pObjRight ) ;
	if ( (pResult == NULL) && !IsException() )
	{
		ThrowExceptionError
			( SString(RSCodeOperator::m_pwszOperators[opIndex])
								+ L" 演算子は評価されませんでした" ) ;
	}
	return	pResult ;
}

// import 文
//////////////////////////////////////////////////////////////////////////////
void RSContext::ExecuteStatementImport
	( RSCodeStream& cstrm,
		RSCodeControl::WordIndex wiIndex, RSClass * pDefClass )
{
	//
	// import <script-file> ;
	//
	RSSmartPtr	pObj
		( EvaluateExpression
			( cstrm, RSCodeOperator::priorityNothing ), this ) ;
	cstrm.NextOperator( RSCodeOperator::opEndOfStatement ) ;
	if ( IsException() || (pObj == NULL) )
	{
		return ;
	}
	SString	strValue ;
	if ( !pObj->AsString( strValue ) )
	{
		return ;
	}
	ESLAssert( m_pVM != NULL ) ;
	if ( m_pVM->GetLoadedScriptAs( strValue ) != NULL )
	{
		return ;
	}
	if ( m_pVM->LoadScript( *this, strValue ) == NULL )
	{
		ThrowExceptionError( strValue + L" を読み込めませんでした" ) ;
	}
}

// class 文
//////////////////////////////////////////////////////////////////////////////
void RSContext::ExecuteStatementClass
	( RSCodeStream& cstrm,
		RSCodeControl::WordIndex wiIndex, RSClass * pDefClass )
{
	//
	// class [native] <name> [extends <super-class>] [implements <super-class>, ...] { ... }
	//
	RSCodeComment *	pComment = m_pLastComment ;
	RSCodeSymbol *	pSymName = cstrm.NextSymbol() ;
	bool			flagNativeClass = false ;
	if ( pSymName == NULL )
	{
		if ( cstrm.NextControlWord( RSCodeControl::wiNative ) == NULL )
		{
			ThrowExceptionError( L"クラス名が指定されていません" ) ;
			return ;
		}
		pSymName = cstrm.NextSymbol() ;
		if ( pSymName == NULL )
		{
			ThrowExceptionError( L"クラス名が指定されていません" ) ;
			return ;
		}
		flagNativeClass = true ;
	}
	//
	// クラス定義
	//
	RSSmartPtr	sptrClass( NULL, this ) ;
	RSClass *	pClass = NULL ;
	sptrClass = GetVariableAs( pSymName->m_symbol ) ;
	if ( sptrClass != NULL )
	{
		pClass = ESLTypeCast<RSClass>( sptrClass.Ptr() ) ;
		if ( pClass == NULL )
		{
			ThrowExceptionError
				( pSymName->m_symbol + L" はクラスではありません" ) ;
			return ;
		}
	}
	else
	{
		pClass = GetClassAs( pSymName->m_symbol ) ;
	}
	if ( pClass != NULL )
	{
		bool	fDeclMember = false ;
		for ( ; ; )
		{
			if ( pClass->m_flagInitialized )
			{
				if ( cstrm.NextOperator
						( RSCodeOperator::opEndOfStatement ) == NULL )
				{
					ThrowExceptionError
						( pSymName->m_symbol
							+ L" クラスが二重定義されています" ) ;
				}
				return ;
			}
			if ( (cstrm.NextOperator( RSCodeOperator::opMemberOf ) == NULL)
				&& (cstrm.NextOperator( RSCodeOperator::opStaticMemberOf ) == NULL) )
			{
				break ;
			}
			fDeclMember = true ;
			pSymName = cstrm.NextSymbol() ;
			if ( pSymName == NULL )
			{
				ThrowExceptionError( L"クラス名が指定されていません" ) ;
				return ;
			}
			RSClass *	pNamespace = pClass ;
			sptrClass = pNamespace->GetMemberAs( *this, pSymName->m_symbol ) ;
			if ( sptrClass != NULL )
			{
				pClass = ESLTypeCast<RSClass>( sptrClass.Ptr() ) ;
				if ( pClass == NULL )
				{
					ThrowExceptionError
						( pSymName->m_symbol + L" はクラス名ではありません" ) ;
					return ;
				}
			}
			else
			{
				pClass = new RSClass( m_pClassClass, pSymName->m_symbol ) ;
				RSObject::ReleaseRef
					( pNamespace->CreateMemberAs
						( *this, pSymName->m_symbol, pClass ) ) ;
			}
		}
		if ( fDeclMember )
		{
			if ( cstrm.NextOperator
					( RSCodeOperator::opEndOfStatement ) == NULL )
			{
				ThrowExceptionError
					( L"クラス宣言の文末に \';\' がありません" ) ;
			}
			return ;
		}
	}
	else
	{
		pClass = new RSClass( m_pClassClass, pSymName->m_symbol ) ;
		CreateVariableAs( pSymName->m_symbol, pClass ) ;
		if ( IsException() )
		{
			return ;
		}
		pClass->m_pNamespace = pDefClass ;
		pClass->m_flagNativeClass = flagNativeClass ;
	}
	if ( (pComment != NULL) && (pClass->GetDefinitionComment() == NULL) )
	{
		pClass->SetDefinitionComment( pComment ) ;
	}
	if ( cstrm.NextOperator( RSCodeOperator::opEndOfStatement ) != NULL )
	{
		return ;
	}
	if ( pComment != NULL )
	{
		pClass->SetDefinitionComment( pComment ) ;
	}
	//
	// クラス派生
	//
	if ( cstrm.NextControlWord( RSCodeControl::wiExtends ) != NULL )
	{
		RSClass *	pSuperClass = ParseClassExpression( cstrm ) ;
		if ( pSuperClass == NULL )
		{
			ThrowExceptionError( L"派生元クラスの指定が不正です" ) ;
			return ;
		}
		if ( pSuperClass->IsKindOf
				( ESL_RUNTIME_CLASS(RSStructuredPointerClass) ) )
		{
			ThrowExceptionError( L"クラスが構造体から派生しています" ) ;
			return ;
		}
		pClass->AddSuperClass( *this, pSuperClass ) ;
	}
	else
	{
		pClass->AddSuperClass( *this, m_pJObjectClass ) ;
	}
	if ( IsException() )
	{
		return ;
	}
	//
	// インターフェース
	//
	if ( cstrm.NextControlWord( RSCodeControl::wiImplements ) != NULL )
	{
		for ( ; ; )
		{
			RSClass *	pSuperClass = ParseClassExpression( cstrm ) ;
			if ( pSuperClass == NULL )
			{
				ThrowExceptionError( L"実装インターフェースの指定が不正です" ) ;
				return ;
			}
			if ( pSuperClass->IsKindOf
					( ESL_RUNTIME_CLASS(RSStructuredPointerClass) ) )
			{
				ThrowExceptionError( L"クラスが構造体から派生しています" ) ;
				return ;
			}
			pClass->AddImplementClass( *this, pSuperClass ) ;
			if ( IsException() )
			{
				return ;
			}
			if ( cstrm.NextOperator( RSCodeOperator::opSequencing ) == NULL )
			{
				break ;
			}
		}
	}
	//
	// クラス初期設定
	//
	pClass->Initialize( *this ) ;
	//
	if ( pClass->m_pPrototype == NULL )
	{
		pClass->m_pPrototype = new RSGenericObject( pClass ) ;
	}
	//
	// クラス実装
	//
	RSParenthesis *	pPrth = cstrm.NextParenthesis( RSParenthesis::ptBrace ) ;
	if ( pPrth == NULL )
	{
		ThrowExceptionError( L"クラス実装がありません" ) ;
		return ;
	}
	pClass->AddRef() ;
	PushNamespace( NULL, pClass, RSObject::modifierPrivate ) ;
	//
	RSCodeStream	csImplClass( *pPrth ) ;
	ExecuteAllStatements( csImplClass, pClass ) ;
	//
	PopNamespace() ;
	//
	pClass->FinishClass( *this ) ;
	//
	// 末尾セミコロンは無視
	//
	cstrm.NextOperator( RSCodeOperator::opEndOfStatement ) ;
}

// struct 文
//////////////////////////////////////////////////////////////////////////////
void RSContext::ExecuteStatementStruct
	( RSCodeStream& cstrm,
		RSCodeControl::WordIndex wiIndex, RSClass * pDefClass )
{
	//
	// struct <name> [extends <super-class>, ...] { ... }
	//
	RSCodeComment *	pComment = m_pLastComment ;
	RSCodeSymbol *	pSymName = cstrm.NextSymbol() ;
	if ( pSymName == NULL )
	{
		ThrowExceptionError( L"構造体名が指定されていません" ) ;
		return ;
	}
	//
	// クラス定義
	//
	RSSmartPtr					sptrStruct( NULL, this ) ;
	RSStructuredPointerClass *	pStructClass = NULL ;
	sptrStruct = GetVariableAs( pSymName->m_symbol ) ;
	if ( sptrStruct != NULL )
	{
		pStructClass =
			ESLTypeCast<RSStructuredPointerClass>( sptrStruct.Ptr() ) ;
		if ( pStructClass == NULL )
		{
			ThrowExceptionError
				( pSymName->m_symbol + L" は構造体ではありません" ) ;
			return ;
		}
	}
	else
	{
		RSClass *	pClass = GetClassAs( pSymName->m_symbol ) ;
		pStructClass =
			ESLTypeCast<RSStructuredPointerClass>( pClass ) ;
		if ( (pClass != NULL) && (pStructClass == NULL) )
		{
			ThrowExceptionError
				( pSymName->m_symbol + L" は構造体ではありません" ) ;
			return ;
		}
	}
	if ( pStructClass != NULL )
	{
		if ( pStructClass->m_flagInitialized )
		{
			if ( cstrm.NextOperator
					( RSCodeOperator::opEndOfStatement ) == NULL )
			{
				ThrowExceptionError
					( pSymName->m_symbol
						+ L" 構造体が二重定義されています" ) ;
			}
			return ;
		}
	}
	else
	{
		pStructClass =
			new RSStructuredPointerClass( m_pClassClass, pSymName->m_symbol ) ;
		CreateVariableAs( pSymName->m_symbol, pStructClass ) ;
		if ( IsException() )
		{
			return ;
		}
		pStructClass->m_pNamespace = pDefClass ;
	}
	if ( (pComment != NULL) && (pStructClass->GetDefinitionComment() == NULL) )
	{
		pStructClass->SetDefinitionComment( pComment ) ;
	}
	if ( cstrm.NextOperator( RSCodeOperator::opEndOfStatement ) != NULL )
	{
		return ;
	}
	if ( pComment != NULL )
	{
		pStructClass->SetDefinitionComment( pComment ) ;
	}
	//
	// 構造体派生
	//
	if ( cstrm.NextControlWord( RSCodeControl::wiExtends ) != NULL )
	{
		RSClass *	pSuperClass = ParseClassExpression( cstrm ) ;
		if ( (pSuperClass == NULL)
			|| !pSuperClass->IsKindOf
					( ESL_RUNTIME_CLASS(RSStructuredPointerClass) ) )
		{
			ThrowExceptionError( L"派生元構造体が不正です" ) ;
			return ;
		}
		pStructClass->AddSuperClass( *this, pSuperClass ) ;
		//
		while ( !IsException()
			&& (cstrm.NextOperator( RSCodeOperator::opSequencing ) != NULL) )
		{
			pSuperClass = ParseClassExpression( cstrm ) ;
			if ( (pSuperClass == NULL)
				|| !pSuperClass->IsKindOf
						( ESL_RUNTIME_CLASS(RSStructuredPointerClass) ) )
			{
				ThrowExceptionError( L"派生元構造体が不正です" ) ;
				return ;
			}
			pStructClass->AddImplementClass( *this, pSuperClass ) ;
		}
	}
	else
	{
		pStructClass->AddSuperClass( *this, GetStructureClass() ) ;
	}
	if ( IsException() )
	{
		return ;
	}
	//
	// クラス初期設定
	//
	pStructClass->Initialize( *this ) ;
	//
	// クラス実装
	//
	RSParenthesis *	pPrth = cstrm.NextParenthesis( RSParenthesis::ptBrace ) ;
	if ( pPrth == NULL )
	{
		ThrowExceptionError( L"構造体実装がありません" ) ;
		return ;
	}
	pStructClass->AddRef() ;
	PushNamespace( NULL, pStructClass, RSObject::modifierPrivate ) ;
	//
	RSCodeStream	csImplStruct( *pPrth ) ;
	ExecuteAllStatements( csImplStruct, pStructClass ) ;
	//
	PopNamespace() ;
	//
	pStructClass->FinishClass( *this ) ;
	//
	// 末尾セミコロンは無視
	//
	cstrm.NextOperator( RSCodeOperator::opEndOfStatement ) ;
}

// function 文
//////////////////////////////////////////////////////////////////////////////
void RSContext::ExecuteStatementFunction
	( RSCodeStream& cstrm,
		RSCodeControl::WordIndex wiIndex, RSClass * pDefClass )
{
	//
	// function [<ret-type] <name>( <arg-list> ) { ... }
	//
	RSClass *		pRetType = NULL ;
	RSCodeSymbol *	pSymName = NULL ;
	RSCode *		pCodeArg = cstrm.GetTerm( 1 ) ;
	if ( (pCodeArg != NULL)
		&& (pCodeArg->m_type == RSCode::typeParenthesis)
		&& (((RSParenthesis*)pCodeArg)->m_parenthesis == RSParenthesis::ptParenthesis) )
	{
		pRetType = m_pVarClass ;
	}
	else
	{
		if ( cstrm.NextControlWord( RSCodeControl::wiVoid ) == NULL )
		{
			pRetType = ParseClassExpression( cstrm ) ;
			if ( pRetType == NULL )
			{
				ThrowExceptionError( L"関数の評価型が不正です" ) ;
				return ;
			}
		}
	}
	//
	// 関数名
	//
	pSymName = cstrm.NextSymbol() ;
	if ( pSymName == NULL )
	{
		ThrowExceptionError( L"関数名がありません" ) ;
		return ;
	}
	//
	// 関数引数
	//
	RSParenthesis *	pPrthArg = cstrm.NextParenthesis( RSParenthesis::ptParenthesis ) ;
	if ( pPrthArg == NULL )
	{
		ThrowExceptionError( L"関数名がありません" ) ;
		return ;
	}
	RSFunctionPrototype *	pProto = new RSFunctionPrototype ;
	SParserErrorLogger		perrLog ;
	RSCodeStream			csArg( *pPrthArg ) ;
	if ( pProto->ParseArgument( *this, csArg, perrLog ) )
	{
		SParserErrorLogger::ErrorLog *
					pErr = perrLog.GetErrorLogAt( 0 ) ;
		if ( pErr != NULL )
		{
			ThrowExceptionError( pErr->m_strError ) ;
		}
		else
		{
			ThrowExceptionError( L"関数の引数の書式が不正です" ) ;
		}
		delete	pProto ;
		return ;
	}
	pProto->SetReturnType( pRetType ) ;
	pProto->SetRefNamespace( GetCurrentNamespace() ) ;
	//
	// 関数実装
	//
	RSParenthesis *	pPrthCode = cstrm.NextParenthesis( RSParenthesis::ptBrace ) ;
	if ( pPrthCode == NULL )
	{
		ThrowExceptionError( L"関数の実装がありません" ) ;
	}
	pProto->m_pParenthesis = pPrthCode ;
	//
	cstrm.NextOperator( RSCodeOperator::opEndOfStatement ) ;
	//
	// 関数追加
	//
	RSFunctionObject *	pFunc ;
	if ( pDefClass != NULL )
	{
		pFunc = pDefClass->GetVirtualMemberAs( *this, pSymName->m_symbol ) ;
		if ( pFunc == NULL )
		{
			pFunc = new RSFunctionObject( m_pFuncClass ) ;
			pDefClass->SetVirtualMemberAs( *this, pSymName->m_symbol, pFunc ) ;
			//
			ReleaseObjectRef
				( pFunc->CreateMemberAs
					( *this, L"prototype", new RSDynamicObject( NULL ) ) ) ;
		}
		pFunc->AddPrototype( pProto ) ;
	}
	else
	{
		pFunc = new RSFunctionObject( m_pFuncClass ) ;
		pFunc->AddPrototype( pProto ) ;
		CreateVariableAs( pSymName->m_symbol, pFunc ) ;
		if ( IsException() )
		{
			return ;
		}
		ReleaseObjectRef
			( pFunc->CreateMemberAs
				( *this, L"prototype", new RSDynamicObject( NULL ) ) ) ;
	}
}

// for 文
//////////////////////////////////////////////////////////////////////////////
void RSContext::ExecuteStatementFor
	( RSCodeStream& cstrm,
		RSCodeControl::WordIndex wiIndex, RSClass * pDefClass )
{
	RSParenthesis *	pPrthFor =
			cstrm.NextParenthesis( RSParenthesis::ptParenthesis ) ;
	RSParenthesis *	pPrthCode =
			cstrm.NextParenthesis( RSParenthesis::ptBrace ) ;
	if ( pPrthFor == NULL )
	{
		ThrowExceptionError( L"for 文の構文エラーです" ) ;
		return ;
	}
	RSCodeStream	csFor( *pPrthFor ) ;
	RSCodeStream	csCode ;
	if ( pPrthCode != NULL )
	{
		csCode.AttachCode( *pPrthCode ) ;
	}
	else
	{
		size_t	iStart = cstrm.GetIndex() ;
		cstrm.PassAStatement() ;
		csCode.AttachCode( cstrm, iStart, (ssize_t) cstrm.GetIndex() ) ;
	}
	RSCode *		pCodeInSym = csFor.GetTerm( 1 ) ;
	if ( (pCodeInSym != NULL)
		&& (pCodeInSym->m_type == RSCode::typeSymbol)
		&& (((RSCodeSymbol*)pCodeInSym)->m_symbol == L"in") )
	{
		//
		// for ( <var-name> in <obj-expr> ) { ... }
		//
		RSCodeSymbol *	pSymVarName = csFor.NextSymbol() ;
		if ( pSymVarName == NULL )
		{
			ThrowExceptionError( L"for in 文の反復変数名が不正です" ) ;
			return ;
		}
		ESLVerify( csFor.NextSymbol() != NULL ) ;
		//
		RSObject *	pObjTarget = EvaluateExpression( csFor ) ;
		if ( IsException() )
		{
			ReleaseObjectRef( pObjTarget ) ;
			return ;
		}
		RSNamespace *	pLocal = new_Namespace() ;
		PushNamespace( NULL, pLocal ) ;
		RSPointer *	pPtrIter = 
			(RSPointer*) pLocal->CreateMemberAs
				( *this, pSymVarName->m_symbol, new_Pointer( NULL ) ) ;
		ESLAssert( pPtrIter->IsKindOf( ESL_RUNTIME_CLASS(RSPointer) ) ) ;
		//
		for ( size_t i = 0; i < pObjTarget->GetElementCount(); i ++ )
		{
			//
			// 要素反復実行
			//
			RSObject *	pKeyStr =
				new_String( pObjTarget->GetElementNameAt( (int) i ) ) ;
			ReleaseObjectRef
				( pPtrIter->OperatorMove( *this, pKeyStr ) ) ;
			ReleaseObjectRef( pKeyStr ) ;
			//
			csCode.SeekIndex( 0 ) ;
			PushNamespace( NULL, new_Namespace() ) ;
			ExecuteAllStatements( csCode ) ;
			PopNamespace() ;
			//
			if ( IsException() )
			{
				break ;
			}
			else if ( m_escape != escapeNothing )
			{
				if ( m_escape == escapeContinue )
				{
					m_escape = escapeNothing ;
				}
				else
				{
					if ( m_escape == escapeBreak )
					{
						m_escape = escapeNothing ;
					}
					break ;
				}
			}
		}
		//
		PopNamespace() ;
		ReleaseObjectRef( pPtrIter ) ;
		ReleaseObjectRef( pObjTarget ) ;
	}
	else
	{
		//
		// for ( <init-expr> ; <cond-expr> ; <step-expr> ) { ... }
		//
		RSNamespace *	pLocal = new_Namespace() ;
		PushNamespace( NULL, pLocal ) ;
		//
		RSClass *	pIterVarType = ParseClassExpression( csFor ) ;
		if ( pIterVarType != NULL )
		{
			ExecuteDeclareVariable
				( csFor, 0, pIterVarType, declModeVar ) ;
		}
		else if ( csFor.NextOperator
					( RSCodeOperator::opEndOfStatement ) == NULL )
		{
			ReleaseObjectRef( EvaluateExpression( csFor ) ) ;
			csFor.NextOperator( RSCodeOperator::opEndOfStatement ) ;
		}
		size_t	iCondExpr = csFor.GetIndex() ;
		PassExpression( csFor, RSCodeOperator::priorityNothing ) ;
		csFor.NextOperator( RSCodeOperator::opEndOfStatement ) ;
		//
		size_t	iStepExpr = csFor.GetIndex() ;
		//
		while ( !IsException() )
		{
			//
			// 反復条件判定
			//
			csFor.SeekIndex( iCondExpr ) ;
			if ( csFor.NextOperator
					( RSCodeOperator::opEndOfStatement ) == NULL )
			{
				RSObject *	pObjCond = EvaluateExpression( csFor ) ;
				if ( IsException() || (pObjCond == NULL) )
				{
					ReleaseObjectRef( pObjCond ) ;
					break ;
				}
				if ( !pObjCond->AsBoolean() )
				{
					ReleaseObjectRef( pObjCond ) ;
					break ;
				}
				ReleaseObjectRef( pObjCond ) ;
			}
			//
			// 反復実行
			//
			csCode.SeekIndex( 0 ) ;
			PushNamespace( NULL, new_Namespace() ) ;
			ExecuteAllStatements( csCode ) ;
			PopNamespace() ;
			if ( IsException() )
			{
				break ;
			}
			else if ( m_escape != escapeNothing )
			{
				if ( m_escape == escapeContinue )
				{
					m_escape = escapeNothing ;
				}
				else
				{
					if ( m_escape == escapeBreak )
					{
						m_escape = escapeNothing ;
					}
					break ;
				}
			}
			//
			// 反復子更新
			//
			csFor.SeekIndex( iStepExpr ) ;
			if ( !csFor.IsEndOfStream() )
			{
				ReleaseObjectRef( EvaluateExpression( csFor ) ) ;
			}
		}
		PopNamespace() ;
	}
}

// while 文
//////////////////////////////////////////////////////////////////////////////
void RSContext::ExecuteStatementWhile
	( RSCodeStream& cstrm,
		RSCodeControl::WordIndex wiIndex, RSClass * pDefClass )
{
	//
	// while( <expr> ) { ... }
	//
	RSParenthesis *	pPrthWhile =
			cstrm.NextParenthesis( RSParenthesis::ptParenthesis ) ;
	RSParenthesis *	pPrthCode =
			cstrm.NextParenthesis( RSParenthesis::ptBrace ) ;
	if ( pPrthWhile == NULL )
	{
		ThrowExceptionError( L"while 文の構文エラーです" ) ;
		return ;
	}
	RSCodeStream	csWhile( *pPrthWhile ) ;
	RSCodeStream	csCode ;
	if ( pPrthCode != NULL )
	{
		csCode.AttachCode( *pPrthCode ) ;
	}
	else
	{
		size_t	iStart = cstrm.GetIndex() ;
		cstrm.PassAStatement() ;
		csCode.AttachCode( cstrm, iStart, (ssize_t) cstrm.GetIndex() ) ;
	}
	while ( !IsException() )
	{
		//
		// 反復条件判定
		//
		csWhile.SeekIndex( 0 ) ;
		RSObject *	pObjCond = EvaluateExpression( csWhile ) ;
		if ( IsException() || (pObjCond == NULL) )
		{
			ReleaseObjectRef( pObjCond ) ;
			break ;
		}
		if ( !pObjCond->AsBoolean() )
		{
			ReleaseObjectRef( pObjCond ) ;
			break ;
		}
		ReleaseObjectRef( pObjCond ) ;
		//
		// 反復実行
		//
		csCode.SeekIndex( 0 ) ;
		PushNamespace( NULL, new_Namespace() ) ;
		ExecuteAllStatements( csCode ) ;
		PopNamespace() ;
		if ( IsException() )
		{
			break ;
		}
		else if ( m_escape != escapeNothing )
		{
			if ( m_escape == escapeContinue )
			{
				m_escape = escapeNothing ;
			}
			else
			{
				if ( m_escape == escapeBreak )
				{
					m_escape = escapeNothing ;
				}
				break ;
			}
		}
	}
}

// do 文
//////////////////////////////////////////////////////////////////////////////
void RSContext::ExecuteStatementDo
	( RSCodeStream& cstrm,
		RSCodeControl::WordIndex wiIndex, RSClass * pDefClass )
{
	//
	// do { ... } while( <expr> ) ;
	//
	RSParenthesis *	pPrthCode =
			cstrm.NextParenthesis( RSParenthesis::ptBrace ) ;
	RSCodeStream	csCode ;
	if ( pPrthCode != NULL )
	{
		csCode.AttachCode( *pPrthCode ) ;
	}
	else
	{
		size_t	iStart = cstrm.GetIndex() ;
		cstrm.PassAStatement() ;
		csCode.AttachCode( cstrm, iStart, (ssize_t) cstrm.GetIndex() ) ;
	}
	if ( cstrm.NextControlWord( RSCodeControl::wiWhile ) == NULL )
	{
		ThrowExceptionError( L"do ～ while 文の構文エラーです" ) ;
		return ;
	}
	RSParenthesis *	pPrthWhile =
			cstrm.NextParenthesis( RSParenthesis::ptParenthesis ) ;
	if ( pPrthWhile == NULL )
	{
		ThrowExceptionError( L"do ～ while 文の反復条件がありません" ) ;
		return ;
	}
	if ( cstrm.NextOperator( RSCodeOperator::opEndOfStatement ) == NULL )
	{
		ThrowExceptionError( L"do ～ while 文末のセミコロンがありません" ) ;
		return ;
	}
	RSCodeStream	csWhile( *pPrthWhile ) ;
	while ( !IsException() )
	{
		//
		// 反復実行
		//
		csCode.SeekIndex( 0 ) ;
		PushNamespace( NULL, new_Namespace() ) ;
		ExecuteAllStatements( csCode ) ;
		PopNamespace() ;
		if ( IsException() )
		{
			break ;
		}
		else if ( m_escape != escapeNothing )
		{
			if ( m_escape == escapeContinue )
			{
				m_escape = escapeNothing ;
			}
			else
			{
				if ( m_escape == escapeBreak )
				{
					m_escape = escapeNothing ;
				}
				break ;
			}
		}
		//
		// 反復条件判定
		//
		csWhile.SeekIndex( 0 ) ;
		RSObject *	pObjCond = EvaluateExpression( csWhile ) ;
		if ( IsException() || (pObjCond == NULL) )
		{
			ReleaseObjectRef( pObjCond ) ;
			break ;
		}
		if ( !pObjCond->AsBoolean() )
		{
			ReleaseObjectRef( pObjCond ) ;
			break ;
		}
		ReleaseObjectRef( pObjCond ) ;
	}
}

// if 文
//////////////////////////////////////////////////////////////////////////////
void RSContext::ExecuteStatementIf
	( RSCodeStream& cstrm,
		RSCodeControl::WordIndex wiIndex, RSClass * pDefClass )
{
	//
	// if ( <expr> ) <statement> [else <statement>]
	//
	RSParenthesis *	pPrthIf =
			cstrm.NextParenthesis( RSParenthesis::ptParenthesis ) ;
	if ( pPrthIf == NULL )
	{
		ThrowExceptionError( L"if 文の条件式がありません" ) ;
		return ;
	}
	RSCodeStream	csIf( *pPrthIf ) ;
	RSObject *	pObjCond = EvaluateExpression( csIf ) ;
	if ( IsException() )
	{
		ReleaseObjectRef( pObjCond ) ;
		return ;
	}
	bool	fCondition = false ;
	if ( pObjCond != NULL )
	{
		fCondition = pObjCond->AsBoolean() ;
		ReleaseObjectRef( pObjCond ) ;
	}
	if ( fCondition )
	{
		PushNamespace( NULL, new_Namespace() ) ;
		ExecuteStatements( cstrm ) ;
		PopNamespace() ;
	}
	else
	{
		cstrm.PassAStatement() ;
	}
	if ( cstrm.NextControlWord( RSCodeControl::wiElse ) != NULL )
	{
		if ( !fCondition )
		{
			PushNamespace( NULL, new_Namespace() ) ;
			ExecuteStatements( cstrm ) ;
			PopNamespace() ;
		}
		else
		{
			cstrm.PassAStatement() ;
		}
	}
}

// switch 文
//////////////////////////////////////////////////////////////////////////////
void RSContext::ExecuteStatementSwitch
	( RSCodeStream& cstrm,
		RSCodeControl::WordIndex wiIndex, RSClass * pDefClass )
{
	RSParenthesis *	pPrthSwitch =
			cstrm.NextParenthesis( RSParenthesis::ptParenthesis ) ;
	if ( pPrthSwitch == NULL )
	{
		ThrowExceptionError( L"switch 文評価式がありません" ) ;
		return ;
	}
	RSCodeStream	csSwitch( *pPrthSwitch ) ;
	RSSmartPtr		pObjSel( EvaluateExpression( csSwitch ), this ) ;
	if ( IsException() )
	{
		return ;
	}
	RSParenthesis *	pPrthCase =
			cstrm.NextParenthesis( RSParenthesis::ptBrace ) ;
	if ( pPrthCase == NULL )
	{
		ThrowExceptionError( L"switch 文ブロックがありません" ) ;
		return ;
	}
	RSCodeStream	csCase( *pPrthCase ) ;
	bool			fMatchCase = false ;
	ssize_t			iDefault = -1 ;
	while ( !csCase.IsEndOfStream() )
	{
		if ( csCase.NextControlWord( RSCodeControl::wiCase ) != NULL )
		{
			size_t	iCaseExpr = csCase.GetIndex() ;
			csCase.FindOperator( RSCodeOperator::opSeparator ) ;
			size_t	iEndOfExpr = csCase.GetIndex() ;
			//
			RSCodeStream	csCaseExpr
				( *pPrthCase, iCaseExpr, (ssize_t) (iEndOfExpr - 1) ) ;
			RSSmartPtr		pCaseExpr
				( EvaluateExpression( csCaseExpr ), this ) ;
			if ( IsException() )
			{
				return ;
			}
			if ( pCaseExpr == NULL )
			{
				ThrowExceptionError( L"case 値を評価できません" ) ;
				return ;
			}
			RSSmartPtr		pObjCase
				( pObjSel->OperatorCompareEQ( *this, pCaseExpr ), this ) ;
			if ( (pObjCase != NULL) && pObjCase->AsBoolean() )
			{
//				csCase.NextOperator( RSCodeOperator::opSeparator ) ;
				fMatchCase = true ;
				break ;
			}
		}
		else if ( csCase.NextControlWord( RSCodeControl::wiDefault ) != NULL )
		{
			csCase.NextOperator( RSCodeOperator::opSeparator ) ;
			iDefault = (ssize_t) csCase.GetIndex() ;
		}
		else
		{
			csCase.PassAStatement() ;
		}
	}
	//
	if ( !fMatchCase && (iDefault >= 0) )
	{
		csCase.SeekIndex( (size_t) iDefault ) ;
		fMatchCase = true ;
	}
	if ( fMatchCase )
	{
		ExecuteAllStatements( csCase ) ;
		//
		if ( m_escape == escapeBreak )
		{
			m_escape = escapeNothing ;
		}
	}
}

// case 文
//////////////////////////////////////////////////////////////////////////////
void RSContext::ExecuteStatementCase
	( RSCodeStream& cstrm,
		RSCodeControl::WordIndex wiIndex, RSClass * pDefClass )
{
	cstrm.FindOperator( RSCodeOperator::opSeparator ) ;
}

// default 文
//////////////////////////////////////////////////////////////////////////////
void RSContext::ExecuteStatementDefault
	( RSCodeStream& cstrm,
		RSCodeControl::WordIndex wiIndex, RSClass * pDefClass )
{
	cstrm.NextOperator( RSCodeOperator::opSeparator ) ;
}

// break 文
//////////////////////////////////////////////////////////////////////////////
void RSContext::ExecuteStatementBreak
	( RSCodeStream& cstrm,
		RSCodeControl::WordIndex wiIndex, RSClass * pDefClass )
{
	m_escape = escapeBreak ;
}

// continue 文
//////////////////////////////////////////////////////////////////////////////
void RSContext::ExecuteStatementContinue
	( RSCodeStream& cstrm,
		RSCodeControl::WordIndex wiIndex, RSClass * pDefClass )
{
	m_escape = escapeContinue ;
}

// try 文
//////////////////////////////////////////////////////////////////////////////
void RSContext::ExecuteStatementTry
	( RSCodeStream& cstrm,
		RSCodeControl::WordIndex wiIndex, RSClass * pDefClass )
{
	ExecuteStatements( cstrm ) ;
	//
	RSObject *	pException = PopException() ;
	if ( pException != nullptr )
	{
		RSCodeControl *	pCatch =
				cstrm.NextControlWord( RSCodeControl::wiCatch ) ;
		while ( pCatch != nullptr )
		{
			RSParenthesis *	pPrthCatch =
					cstrm.NextParenthesis( RSParenthesis::ptParenthesis ) ;
			if ( pPrthCatch == nullptr )
			{
				cstrm.PassAStatement() ;
				ThrowExceptionError( L"catch 文が不正です" ) ;
				break ;
			}
			RSCodeStream	csCatch( *pPrthCatch ) ;
			RSClass *	pCatchType = ParseClassExpression( csCatch ) ;
			if ( pCatchType != nullptr )
			{
				//
				// catch( <type> <var-name> ) { ... }
				//
				RSObject *	pObjExcept =
						pCatchType->CastInstance( *this, pException ) ;
				if ( pObjExcept != nullptr )
				{
					ReleaseObjectRef( pException ) ;
					pException = nullptr ;
					//
					RSCodeSymbol *	pSymVarName = csCatch.NextSymbol() ;
					if ( pSymVarName != nullptr )
					{
						RSNamespace *	pLocal = new_Namespace() ;
						PushNamespace( nullptr, pLocal ) ;
						ReleaseObjectRef
							( pLocal->CreateMemberAs
								( *this, pSymVarName->m_symbol, pObjExcept ) ) ;
						//
						ExecuteStatements( cstrm ) ;
						//
						PopNamespace() ;
					}
					else
					{
						ReleaseObjectRef( pObjExcept ) ;
						ExecuteStatements( cstrm ) ;
					}
					break ;
				}
			}
			else
			{
				//
				// catch( <var-name> ) { ... }
				//
				RSCodeSymbol *	pSymVarName = csCatch.NextSymbol() ;
				if ( pSymVarName == nullptr )
				{
					cstrm.PassAStatement() ;
					ThrowExceptionError( L"catch 文が不正です" ) ;
					break ;
				}
				RSNamespace *	pLocal = new_Namespace() ;
				PushNamespace( nullptr, pLocal ) ;
				ReleaseObjectRef
					( pLocal->CreateMemberAs
						( *this, pSymVarName->m_symbol, pException ) ) ;
				pException = nullptr ;
				//
				ExecuteStatements( cstrm ) ;
				//
				PopNamespace() ;
				break ;
			}
			cstrm.PassAStatement() ;
			pCatch = cstrm.NextControlWord( RSCodeControl::wiCatch ) ;
		}
	}
	RSCodeControl *	pCatch =
			cstrm.NextControlWord( RSCodeControl::wiCatch ) ;
	while ( pCatch != nullptr )
	{
		cstrm.NextParenthesis( RSParenthesis::ptParenthesis ) ;
		cstrm.PassAStatement() ;
		pCatch = cstrm.NextControlWord( RSCodeControl::wiCatch ) ;
	}
	RSCodeControl *	pFinally =
			cstrm.NextControlWord( RSCodeControl::wiFinally ) ;
	if ( pFinally != nullptr )
	{
		//
		// finally { ... }
		//
		EscapeContextSaver		ecs( *this ) ;
		const RSParenthesis *	pSaveExceptPrth = m_pExceptionParenthesis ;
		size_t					iSaveExceptStat = m_iExceptionStatement ;
		//
		ESLAssert( m_pException == nullptr ) ;
		m_escape = escapeNothing ;
		//
		ExecuteStatements( cstrm ) ;
		//
		if ( m_pException == nullptr )
		{
			m_pException = pException ;
			m_pExceptionParenthesis = pSaveExceptPrth ;
			m_iExceptionStatement = iSaveExceptStat ;
		}
		else
		{
			ReleaseObjectRef( pException ) ;
			pException = nullptr ;
		}
	}
	else if ( pException != nullptr )
	{
		ReleaseObjectRef( m_pException ) ;
		m_pException = pException ;
	}
}

// throw 文
//////////////////////////////////////////////////////////////////////////////
void RSContext::ExecuteStatementThrow
	( RSCodeStream& cstrm,
		RSCodeControl::WordIndex wiIndex, RSClass * pDefClass )
{
	RSObject *	pObj = EvaluateExpression( cstrm ) ;
	cstrm.NextOperator( RSCodeOperator::opEndOfStatement ) ;
	//
	if ( !IsException() )
	{
		SetException( pObj ) ;
	}
}

// return 文
//////////////////////////////////////////////////////////////////////////////
void RSContext::ExecuteStatementReturn
	( RSCodeStream& cstrm,
		RSCodeControl::WordIndex wiIndex, RSClass * pDefClass )
{
	if ( cstrm.NextOperator( RSCodeOperator::opEndOfStatement ) != NULL )
	{
		SetReturnObject( NULL ) ;
	}
	else
	{
		RSObject *	pObj = EvaluateExpression( cstrm ) ;
		cstrm.NextOperator( RSCodeOperator::opEndOfStatement ) ;
		//
		if ( !IsException() )
		{
			SetReturnObject( pObj ) ;
		}
		else
		{
			ReleaseObjectRef( pObj ) ;
		}
	}
}

// with 文
//////////////////////////////////////////////////////////////////////////////
void RSContext::ExecuteStatementWith
	( RSCodeStream& cstrm,
		RSCodeControl::WordIndex wiIndex, RSClass * pDefClass )
{
	RSParenthesis *	pPrthWith =
			cstrm.NextParenthesis( RSParenthesis::ptParenthesis ) ;
	if ( pPrthWith == NULL )
	{
		ThrowExceptionError( L"with 文が不正です" ) ;
		return ;
	}
	RSCodeStream	csWith( *pPrthWith ) ;
	RSObject *	pObj = EvaluateExpression( csWith ) ;
	//
	PushNamespace( NULL, pObj ) ;
	PushNamespace( NULL, new_Namespace() ) ;
	ExecuteStatements( cstrm ) ;
	PopNamespace() ;
	PopNamespace() ;
}

// synchronized 文
//////////////////////////////////////////////////////////////////////////////
void RSContext::ExecuteStatementSynchronized
	( RSCodeStream& cstrm,
		RSCodeControl::WordIndex wiIndex, RSClass * pDefClass )
{
	RSParenthesis *	pPrthSync =
			cstrm.NextParenthesis( RSParenthesis::ptParenthesis ) ;
	if ( pPrthSync == NULL )
	{
		ExecuteStatementAccessModifier( cstrm, wiIndex, pDefClass ) ;
		return ;
	}
	RSCodeStream	csSync( *pPrthSync ) ;
	RSObject *	pObj = EvaluateExpression( csSync ) ;
	RSObject *	pObjSync =
					(pObj != NULL) ? pObj->GetEntityObject() : NULL ;
	if ( pObjSync == NULL )
	{
		ThrowExceptionError
			( L"synchronized 文に null が指定されています" ) ;
		return ;
	}
	//
	RSObject::Synchronized	sync ;
	if ( !pObjSync->LockSynchronized( &sync, this ) )
	{
		ExecuteStatements( cstrm ) ;
		//
		pObjSync->UnlockSynchronized( this ) ;
	}
	ReleaseObjectRef( pObj ) ;
}

// static|abstract|const|public|protected|private 文
//////////////////////////////////////////////////////////////////////////////
void RSContext::ExecuteStatementAccessModifier
	( RSCodeStream& cstrm,
		RSCodeControl::WordIndex wiIndex, RSClass * pDefClass )
{
	uint32_t	accMod = 0 ;
	do
	{
		switch ( wiIndex )
		{
		case	RSCodeControl::wiStatic:
			accMod |= RSObject::modifierStatic ;
			break ;
		case	RSCodeControl::wiAbstract:
			accMod |= RSObject::modifierAbstract ;
			break ;
		case	RSCodeControl::wiNative:
			accMod |= RSObject::modifierNative ;
			break ;
		case	RSCodeControl::wiConst:
			accMod |= RSObject::modifierConst ;
			break ;
		case	RSCodeControl::wiSynchronized:
			accMod |= RSObject::modifierSynchronized ;
			break ;
		case	RSCodeControl::wiPublic:
			accMod = (accMod & ~RSObject::accessMask) | RSObject::modifierPublic ;
			break ;
		case	RSCodeControl::wiProtected:
			accMod = (accMod & ~RSObject::accessMask) | RSObject::modifierProtected ;
			break ;
		case	RSCodeControl::wiPrivate:
			accMod = (accMod & ~RSObject::accessMask) | RSObject::modifierPrivate ;
			break ;
		case	RSCodeControl::wiVar:
			ExecuteDeclareVariable
				( cstrm, accMod, m_pVarClass, declModeAny, pDefClass, m_pLastComment ) ;
			return ;
		case	RSCodeControl::wiVoid:
			ExecuteDeclareVariable
				( cstrm, accMod, NULL, declModeAny, pDefClass, m_pLastComment ) ;
			return ;
		case	RSCodeControl::wiExtern:
			accMod |= RSObject::modifierExtern ;
			break ;
		default:
			if ( (wiIndex >= RSCodeControl::wiFirstBasicType)
				&& (wiIndex <= RSCodeControl::wiLastBasicType) )
			{
				RSClass *	pClass =
					m_pBasicTypeClass[wiIndex - RSCodeControl::wiFirstBasicType] ;
				pClass = ParseClassArrayDecoration( cstrm, pClass ) ;
				ExecuteDeclareVariable
					( cstrm, accMod, pClass, declModeAny, pDefClass, m_pLastComment ) ;
			}
			else
			{
				(this->*m_pfnExecuteStatement[wiIndex])
								( cstrm, wiIndex, pDefClass ) ;
			}
			return ;
		}
		RSCodeControl *	pCode = cstrm.NextControlWord() ;
		if ( pCode == NULL )
		{
			RSClass *	pClass = ParseClassExpression( cstrm ) ;
			if ( pClass != NULL )
			{
				ExecuteDeclareVariable
					( cstrm, accMod, pClass, declModeAny, pDefClass, m_pLastComment ) ;
			}
			else
			{
				ThrowExceptionError( L"アクセス修飾子が無効です" ) ;
			}
			return ;
		}
		wiIndex = pCode->m_word ;
	}
	while ( !cstrm.IsEndOfStream() ) ;
}

// var|void|boolean|byte|short|char|int|long|float|double 文
//////////////////////////////////////////////////////////////////////////////
void RSContext::ExecuteStatementVar
	( RSCodeStream& cstrm,
		RSCodeControl::WordIndex wiIndex, RSClass * pDefClass )
{
	RSClass *	pClass = NULL ;
	if ( (wiIndex >= RSCodeControl::wiFirstBasicType)
				&& (wiIndex <= RSCodeControl::wiLastBasicType) )
	{
		pClass =
			ParseClassArrayDecoration
				( cstrm, m_pBasicTypeClass
							[wiIndex - RSCodeControl::wiFirstBasicType] ) ;
	}
	else if ( wiIndex == RSCodeControl::wiVar )
	{
		pClass = m_pVarClass ;
	}
	ExecuteDeclareVariable
		( cstrm, 0, pClass, declModeAny, pDefClass, m_pLastComment ) ;
}

// this|super 文
//////////////////////////////////////////////////////////////////////////////
void RSContext::ExecuteStatementExpression
	( RSCodeStream& cstrm,
		RSCodeControl::WordIndex wiIndex, RSClass * pDefClass )
{
	ESLAssert( cstrm.GetIndex() > 0 ) ;
	cstrm.SeekIndex( cstrm.GetIndex() - 1 ) ;
	ReleaseObjectRef
		( EvaluateExpression
			( cstrm, RSCodeOperator::priorityNothing ) ) ;
	cstrm.NextOperator( RSCodeOperator::opEndOfStatement ) ;
}

// デバッグ用
//////////////////////////////////////////////////////////////////////////////
void RSContext::ExecuteStatementDebug
	( RSCodeStream& cstrm,
		RSCodeControl::WordIndex wiIndex, RSClass * pDefClass )
{
	if ( m_pDebugListener != NULL )
	{
		RSCode *	pCode = cstrm.GetTerm( 0 ) ;
		if ( pCode != NULL )
		{
			const RSParenthesis *	pPrth = cstrm.GetParenthesis() ;
			const RSScript *
					pScript = DebugScriptFromParenthesis( pPrth ) ;
			m_pDebugListener->OnDebug( this, pScript, pCode->m_iSrc ) ;
		}
	}
}

// 不正文
//////////////////////////////////////////////////////////////////////////////
void RSContext::ExecuteStatementInvalid
	( RSCodeStream& cstrm,
		RSCodeControl::WordIndex wiIndex, RSClass * pDefClass )
{
	ThrowExceptionError( L"予約語が無効です" ) ;
}

// RSParenthesis -> RSScript（デバッグ用）
//////////////////////////////////////////////////////////////////////////////
const RSScript *
	RSContext::DebugScriptFromParenthesis( const RSParenthesis * pPrth )
{
	const RSScript *	pScript = NULL ;
	if ( m_pLastDebugPrth == pPrth )
	{
		pScript = m_pLastDebugScript ;
	}
	else
	{
		m_pLastDebugPrth = pPrth ;
		while ( pPrth != NULL )
		{
			pScript = ESLTypeCast<RSScript>( pPrth ) ;
			if ( pScript != NULL )
			{
				break ;
			}
			pPrth = pPrth->m_parent ;
		}
		m_pLastDebugScript = pScript ;
	}
	return	pScript ;
}

// 引数評価
//////////////////////////////////////////////////////////////////////////////
void RSContext::EvaluateArgument( RSObject& arg, RSCodeStream& cs )
{
	const RSParenthesis *	pPrth = cs.GetParenthesis() ;
	size_t	iArg = 0 ;
	while ( !cs.IsEndOfStream() && !IsException() )
	{
		m_pCurParenthesis = pPrth ;
		RSObject *	pObj =
			EvaluateExpression( cs, RSCodeOperator::priorityList ) ;
		ReleaseObjectRef
			( arg.SetElementAt( *this, (int) (iArg ++), pObj ) ) ;
		if ( IsException() )
		{
			break ;
		}
		if ( cs.NextOperator( RSCodeOperator::opSequencing ) == NULL )
		{
			if ( cs.NextTerm() != NULL )
			{
				ThrowExceptionError( L"引数の書式が不正です" ) ;
			}
			break ;
		}
	}
}

void RSContext::EvaluateArgument( RSObject& arg, RSParenthesis& prth )
{
	RSCodeStream	cs ;
	cs.AttachCode( prth ) ;
	//
	EvaluateArgument( arg, cs ) ;
}

// 関数脱出判定
//////////////////////////////////////////////////////////////////////////////
bool RSContext::IsReturn( void ) const
{
	return	(m_escape == escapeReturn) ;
}

// 処理の強制中断設定
//////////////////////////////////////////////////////////////////////////////
void RSContext::SetAbort( void )
{
	m_escape = escapeAbort ;
}

// 返り値取得
//////////////////////////////////////////////////////////////////////////////
RSObject * RSContext::PopReturnObject( void )
{
	RSObject *	pRetObj = m_pRetValue ;
	m_pRetValue = NULL ;
	if ( m_escape == escapeReturn )
	{
		m_escape = escapeNothing ;
	}
	return	pRetObj ;
}

// 返り値設定
//////////////////////////////////////////////////////////////////////////////
void RSContext::SetReturnObject( RSObject * pObj )
{
	ReleaseObjectRef( m_pRetValue ) ;
	if ( pObj != nullptr )
	{
		RSObject *	pEntity = RSObject::AddRef( pObj->GetEntityObject() ) ;
		ReleaseObjectRef( pObj ) ;
		pObj = pEntity ;
	}
	m_pRetValue = pObj ;
	m_escape = escapeReturn ;
}

// 例外発生判定
//////////////////////////////////////////////////////////////////////////////
bool RSContext::IsException( void ) const
{
	return	(m_pException != NULL) ;
}

// 例外取得
//////////////////////////////////////////////////////////////////////////////
RSObject * RSContext::GetException( void ) const
{
	return	m_pException ;
}

RSObject * RSContext::PopException( void )
{
	RSObject *	pException = m_pException ;
	m_pException = NULL ;
	return	pException ;
}

// 例外エラー設定
//////////////////////////////////////////////////////////////////////////////
void RSContext::SetException( RSObject * pException )
{
	ReleaseObjectRef( m_pException ) ;
	m_pException = pException ;
	m_pExceptionParenthesis = m_pCurParenthesis ;
	m_iExceptionStatement = m_iSrcStatement ;
}

void RSContext::ThrowExceptionError
	( const wchar_t * pwszErrMsg, const wchar_t * pwszClassName )
{
	if ( m_pException == NULL )
	{
		SetException( new_Exception( pwszErrMsg, pwszClassName ) ) ;
	}
}

// 例外エラー削除
//////////////////////////////////////////////////////////////////////////////
void RSContext::ClearException( void )
{
	ReleaseObjectRef( m_pException ) ;
	m_pException = NULL ;
	m_pExceptionParenthesis = NULL ;
	m_iExceptionStatement = 0 ;
}

// 例外エラーがある場合、これをエラー出力し、例外エラーをクリア
//////////////////////////////////////////////////////////////////////////////
void RSContext::OutputExceptionError( SSystem::SParserErrorInterface& perr )
{
	if ( m_pException != NULL )
	{
		SStringParser	ss ;
		SString	strErr ;
		if ( !m_pException->AsString( strErr ) )
		{
			strErr = m_pException->GetTypeName() ;
		}
		const RSParenthesis *	pSrcParenthesis = m_pExceptionParenthesis ;
		if ( pSrcParenthesis != NULL )
		{
			while ( pSrcParenthesis->m_parent != NULL )
			{
				pSrcParenthesis = pSrcParenthesis->m_parent ;
			}
		}
		const RSScript *	pSrcScript = ESLTypeCast<RSScript>( pSrcParenthesis ) ;
		if ( pSrcScript != NULL )
		{
			ss.AttachString( pSrcScript->GetSourceText() ) ;
			ss.SetFilePath( pSrcScript->GetSourcePath() ) ;
			ss.SeekIndex( m_iExceptionStatement ) ;
		}
		perr.OutputError( ss, strErr ) ;
		//
		ClearException() ;
	}
}

void RSContext::OutputExceptionError
	( SSystem::SParserErrorInterface& perr, const wchar_t * pwszInline )
{
	if ( m_pException != NULL )
	{
		SStringParser	ss ;
		SString	strErr ;
		if ( !m_pException->AsString( strErr ) )
		{
			strErr = m_pException->GetTypeName() ;
		}
		const RSParenthesis *	pSrcParenthesis = m_pExceptionParenthesis ;
		if ( pSrcParenthesis != NULL )
		{
			while ( pSrcParenthesis->m_parent != NULL )
			{
				pSrcParenthesis = pSrcParenthesis->m_parent ;
			}
		}
		const RSScript *	pSrcScript = ESLTypeCast<RSScript>( pSrcParenthesis ) ;
		if ( pSrcScript != NULL )
		{
			ss.AttachString( pSrcScript->GetSourceText() ) ;
			ss.SetFilePath( pSrcScript->GetSourcePath() ) ;
			ss.SeekIndex( m_iExceptionStatement ) ;
		}
		else if ( pwszInline != nullptr )
		{
			ss = pwszInline ;
		}
		perr.OutputError( ss, strErr ) ;
		//
		ClearException() ;
	}
}

// 待機処理（脱出判定付）
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSContext::WaitSynchronism
	( SSystem::SSynchronism& sync, int64_t nTimeout )
{
	if ( (nTimeout <= 100) && (nTimeout != SSynchronism::Infinite) )
	{
		return	sync.Wait( nTimeout ) ;
	}
	STimeCounter	timer ;
	while ( m_escape == escapeNothing )
	{
		uint64_t	nCurrent = timer.GetTime() ;
		int64_t		nCurWait = 0 ;
		if ( nCurrent < (uint64_t) nTimeout )
		{
			nCurWait = (uint64_t) nTimeout - nCurrent ;
			if ( nCurWait > 10 )
			{
				nCurWait = 10 ;
			}
		}
		SError	err = sync.Wait( nCurWait ) ;
		if ( err != errTimeout )
		{
			return	err ;
		}
		if ( nTimeout != SSynchronism::Infinite )
		{
			if ( timer.GetTime() >= nTimeout )
			{
				return	errTimeout ;
			}
		}
	}
	if ( m_escape != escapeNothing )
	{
		return	errAbort ;
	}
	return	errTimeout ;
}

// 一定時間待機（脱出判定付）
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSContext::SleepMilliSec( int64_t nTimeout )
{
	STimeCounter	timer ;
	while ( m_escape == escapeNothing )
	{
		uint64_t	nCurrent = timer.GetTime() ;
		int64_t		nCurWait = 0 ;
		if ( nCurrent >= (uint64_t) nTimeout )
		{
			break ;
		}
		nCurWait = (uint64_t) nTimeout - nCurrent ;
		if ( nCurWait > 10 )
		{
			nCurWait = 10 ;
		}
		SSystem::SleepMilliSec( (int) nCurWait ) ;
	}
	if ( m_escape != escapeNothing )
	{
		return	errAbort ;
	}
	return	errSuccess ;
}

// 参照チェーン追加
//////////////////////////////////////////////////////////////////////////////
void RSContext::PushNamespace
	( RSObject * pThisObj,
		RSObject * pNamespace,
		RSObject::AccessModifier accMod,
		bool fRootFunc,
		RSFunctionPrototype * pRootFunc )
{
	ESLAssert( pNamespace != NULL ) ;
	//
	NAMESPACE_NEST	nn ;
	nn.fRootFunc = fRootFunc ;
	nn.pRootFunc = pRootFunc ;
	nn.pNamespace = pNamespace ;
	nn.pThis = pThisObj ;
	nn.accModifier = accMod ;
	//
	if ( pThisObj != NULL )
	{
		pThisObj->AddRef() ;
		ReleaseObjectRef( m_pThisObj ) ;
		m_pThisObj = pThisObj ;
	}
	//
	m_arrWith.Add( nn ) ;
}

// 参照チェーン削除
//////////////////////////////////////////////////////////////////////////////
void RSContext::PopNamespace( void )
{
	if ( m_arrWith.GetLength() >= 1 )
	{
		NAMESPACE_NEST	nn = m_arrWith.Pop() ;
		ESLAssert( nn.pNamespace != NULL ) ;
		ReleaseObjectRef( nn.pNamespace ) ;
		ReleaseObjectRef( nn.pThis ) ;
		//
		if ( (nn.pThis != NULL) && (m_pThisObj == nn.pThis) )
		{
			ReleaseObjectRef( m_pThisObj ) ;
			m_pThisObj = NULL ;
		}
		//
		const NAMESPACE_NEST *	pNest = m_arrWith.GetConstArray() ;
		const size_t			nLength = m_arrWith.GetLength() ;
		for ( size_t i = 0; i < nLength; i ++ )
		{
			const NAMESPACE_NEST&	nest = pNest[nLength - i - 1] ;
			if ( nest.pThis != NULL )
			{
				ReleaseObjectRef( m_pThisObj ) ;
				m_pThisObj = nest.pThis ;
				RSObject::AddRef( m_pThisObj ) ;
				break ;
			}
			if ( nest.fRootFunc )
			{
				break ;
			}
		}
	}
}

// 現在の名前空間参照チェーン取得
//////////////////////////////////////////////////////////////////////////////
RSObject * RSContext::GetCurrentNamespace( void )
{
	const size_t	nNest = m_arrWith.GetLength() ;
	if ( nNest == 0 )
	{
		return	NULL ;
	}
	const NAMESPACE_NEST *	pNamespaces = m_arrWith.GetConstArray() ;
	size_t	iRootFunc = nNest - 1 ;
	while ( (iRootFunc > 0) && !pNamespaces[iRootFunc].fRootFunc )
	{
		iRootFunc -- ;
	}
	if ( (iRootFunc == nNest - 1)
		&& (pNamespaces[iRootFunc].pThis == NULL) )
	{
		RSObject *	pNamespace = pNamespaces[iRootFunc].pNamespace ;
		RSObject::AddRef( pNamespace ) ;
		return	pNamespace ;
	}
	else
	{
		RSObject *	pBackLink = NULL ;
		size_t		i = iRootFunc ;
		while ( i < nNest )
		{
			const NAMESPACE_NEST&	nn = pNamespaces[i ++] ;
			if ( nn.pThis != NULL )
			{
				pBackLink = new_Namespace
					( nn.pThis, nn.accModifier, pBackLink ) ;
			}
			if ( nn.pNamespace != NULL )
			{
				pBackLink = new_Namespace
					( nn.pNamespace, nn.accModifier, pBackLink ) ;
			}
		}
		return	pBackLink ;
	}
}

// ローカル変数空間を取得する
//////////////////////////////////////////////////////////////////////////////
RSFunctionPrototype * RSContext::GetLocalNamespaces
		( SSystem::SPointerArray<RSObject>& lstNamespaces )
{
	const size_t	nNest = m_arrWith.GetLength() ;
	if ( nNest == 0 )
	{
		return	NULL ;
	}
	const NAMESPACE_NEST *	pNamespaces = m_arrWith.GetConstArray() ;
	size_t	iRootFunc = nNest - 1 ;
	while ( (iRootFunc > 0) && !pNamespaces[iRootFunc].fRootFunc )
	{
		iRootFunc -- ;
	}
	RSFunctionPrototype *	pProto = pNamespaces[iRootFunc].pRootFunc ;
	lstNamespaces.RemoveAll() ;
	for ( size_t i = iRootFunc; i < nNest; i ++ )
	{
		if ( pNamespaces[i].pNamespace
			&& (pNamespaces[i].pNamespace != m_pVM) )
		{
			lstNamespaces.Add( pNamespaces[i].pNamespace ) ;
		}
	}
	return	pProto ;
}

// スタックダンプを出力する
//////////////////////////////////////////////////////////////////////////////
void RSContext::DebugDumpStack
	( SFileInterface& dump, const wchar_t * pwszIndent )
{
	SString	strIndent = pwszIndent ;
	strIndent += L"\t" ;
	//
	for ( size_t i = 0; i < m_arrWith.GetLength(); i ++ )
	{
		NAMESPACE_NEST *	pnn = m_arrWith.GetAt( i ) ;
		ESLAssert( pnn != NULL ) ;
		if ( pnn == NULL )
		{
			break ;
		}
		if ( pnn->fRootFunc )
		{
			SString	strFuncDump = L"function " ;
			if ( pnn->pRootFunc != NULL )
			{
				RSFunctionObject *	pFunc = pnn->pRootFunc->m_pFuncGroup ;
				if ( pFunc != NULL )
				{
					if ( pFunc->m_pFuncClass != NULL )
					{
						strFuncDump += pFunc->m_pFuncClass->GetFullClassName() ;
						strFuncDump += L"." ;
					}
					strFuncDump += pFunc->m_strFuncName ;
				}
				strFuncDump += L"(" ;
				strFuncDump += pnn->pRootFunc->FormatArgument() ;
				strFuncDump += L")" ;
			}
			strFuncDump += L"\r\n" ;
			dump.WriteEncodedString( strFuncDump ) ;
		}
		if ( pnn->pThis != NULL )
		{
			SString	strThisDump = L"this #" ;
			if ( sizeof(RSObject*) > 4 )
			{
				strThisDump += SString( (int64_t) pnn->pThis, 16, 16 ) ;
			}
			else
			{
				strThisDump +=
					SString( (uint32_t) ((ulong_ptr_t) pnn->pThis), 8, 16 ) ;
			}
			strThisDump += L": " ;
			dump.WriteEncodedString( strThisDump ) ;
			//
			pnn->pThis->ToDebugDump( dump, 3, strIndent ) ;
			//
			dump.WriteEncodedString( L"\r\n" ) ;
		}
		if ( pnn->pNamespace != NULL )
		{
			dump.WriteEncodedString( L"with " ) ;
			//
			pnn->pNamespace->ToDebugDump( dump, 10, strIndent ) ;
			//
			dump.WriteEncodedString( L"\r\n" ) ;
		}
	}
}

// 動作フラグ取得
//////////////////////////////////////////////////////////////////////////////
uint32_t RSContext::GetBehaviorFlags( void ) const
{
	return	m_flagsBehavior ;
}

bool RSContext::IsBehavior( int nFlags ) const
{
	return	(m_flagsBehavior & nFlags) != 0 ;
}

// 動作フラグ変更
//////////////////////////////////////////////////////////////////////////////
uint32_t RSContext::ModifyBehaviorFlags
	( uint32_t flagsAdd, uint32_t flagsRemove )
{
	uint32_t	flagsLast = m_flagsBehavior ;
	m_flagsBehavior = (m_flagsBehavior | flagsAdd) & ~flagsRemove ;
	return	flagsLast ;
}

// 変数参照
//////////////////////////////////////////////////////////////////////////////
RSObject * RSContext::GetVariableAs( const wchar_t * pwszName )
{
	const NAMESPACE_NEST *	pNamespaces = m_arrWith.GetConstArray() ;
	size_t					nNest = m_arrWith.GetLength() ;
	while ( nNest >= 1 )
	{
		NAMESPACE_NEST	nn = pNamespaces[-- nNest] ;
		RSObject *	pVar = nn.pNamespace->GetMemberAs( *this, pwszName ) ;
		if ( pVar != NULL )
		{
			if ( pVar->IsEnableAccessModifier( nn.accModifier ) )
			{
				return	pVar ;
			}
			ReleaseObjectRef( pVar ) ;
			ThrowExceptionError
				( SString(pwszName) + L" はプライベートなメンバ参照です" ) ;
			return	NULL ;
		}
		RSClass *	pClass = nn.pNamespace->GetEntityClass() ;
		if ( pClass != NULL )
		{
			RSObject *	pVirtual = pClass->GetVirtualMemberAs( *this, pwszName ) ;
			if ( pVirtual != NULL )
			{
				if ( pVirtual->IsEnableAccessModifier( nn.accModifier ) )
				{
					ReleaseObjectRef( m_pExprParentOf ) ;
					m_pExprParentOf = nn.pNamespace ;
					RSObject::AddRef( m_pExprParentOf ) ;
					return	pVirtual ;
				}
				ReleaseObjectRef( pVirtual ) ;
				ThrowExceptionError
					( SString(pwszName) + L" はプライベートなメソッド参照です" ) ;
				return	NULL ;
			}
		}
		RSObject *	pThis = nn.pThis ;
		if ( pThis != NULL )
		{
			RSObject *	pVar = pThis->GetMemberAs( *this, pwszName ) ;
			if ( pVar != NULL )
			{
				return	pVar ;
			}
			pClass = pThis->GetEntityClass() ;
			if ( pClass != NULL )
			{
				RSObject *	pVirtual = pClass->GetVirtualMemberAs( *this, pwszName ) ;
				if ( pVirtual != NULL )
				{
					ReleaseObjectRef( m_pExprParentOf ) ;
					m_pExprParentOf = pThis ;
					RSObject::AddRef( m_pExprParentOf ) ;
					return	pVirtual ;
				}
			}
		}
		if ( nn.fRootFunc )
		{
			break ;
		}
	}
	return	NULL ;
}

// メンバへのアクセス保護判定と例外のスロー
//////////////////////////////////////////////////////////////////////////////
bool RSContext::VerifyMemberAccessModifier
	( RSObject * pObj, RSObject * pMember, const wchar_t * pwszName )
{
	if ( pMember == NULL )
	{
		return	true ;
	}
	if ( pMember->IsEnableAccessModifier( RSObject::modifierPublic ) )
	{
		return	true ;
	}
	if ( m_pThisObj == pObj )
	{
		return	true ;
	}
	if ( (m_pThisClass != NULL) && (pObj != NULL) )
	{
		if ( m_pThisClass == pObj )
		{
			return	true ;
		}
		RSClass *	pObjClass = pObj->GetEntityClass() ;
		if ( pObjClass && pObjClass->IsInstanceOf( m_pThisClass ) )
		{
			return	true ;
		}
	}
	ThrowExceptionError
		( SString(pwszName) + L" は保護されたメンバです" ) ;
	return	false ;
}

// 変数生成／設定
//////////////////////////////////////////////////////////////////////////////
void RSContext::CreateVariableAs( const wchar_t * pwszName, RSObject * pObj )
{
	if ( m_arrWith.GetLength() == 0 )
	{
		ThrowExceptionError( L"名前空間が存在しない為、変数を作成できません" ) ;
		ReleaseObjectRef( pObj ) ;
		return ;
	}
	ReleaseObjectRef
		( m_arrWith.GetLastAt(0)->pNamespace->
					CreateMemberAs( *this, pwszName, pObj ) ) ;
}

// クラス取得
//////////////////////////////////////////////////////////////////////////////
RSClass * RSContext::GetClassAs( const wchar_t * pwszClassName )
{
	RSClass *	pClass = m_pVM->GetClassAs( pwszClassName ) ;
	if ( pClass != NULL )
	{
		return	pClass ;
	}
	RSSourceParser	sparsClass( pwszClassName ) ;
	SString			strName ;
	if ( sparsClass.NextToken( strName ) != SStringParser::tokenNormal )
	{
		return	NULL ;
	}
	RSClass *	pObjClass = m_pVM->GetClassAs( strName ) ;
	if ( pObjClass == NULL )
	{
		return	NULL ;
	}
	for ( ; ; )
	{
		if ( !sparsClass.PassSpace() )
		{
			return	pObjClass ;
		}
		if ( sparsClass.HasToComeChar( L"." ) == L'.' )
		{
		}
		else if ( sparsClass.HasToComeToken( L"::" ) )
		{
		}
		else
		{
			return	NULL ;
		}
		if ( sparsClass.NextToken( strName ) != SStringParser::tokenNormal )
		{
			return	NULL ;
		}
		RSObject *	pObj = pObjClass->GetMemberAs( *this, strName ) ;
		if ( pObj == NULL )
		{
			break ;
		}
		if ( pObj->GetBasicType() != RSObject::typeClass )
		{
			pObj->ReleaseRef() ;
			break ;
		}
		ESLAssert( pObj->IsKindOf( ESL_RUNTIME_CLASS(RSClass) ) ) ;
		pObj->ReleaseRef() ;
		pObjClass = (RSClass*) pObj ;
	}
	return	NULL ;
}

RSClass * RSContext::GetIntClassSizeOf( size_t nBytes ) const
{
	RSCodeControl::WordIndex	type ;
	switch ( nBytes )
	{
	case	8:
		type = RSCodeControl::wiLong ;
		break ;
	case	4:
		type = RSCodeControl::wiInt ;
		break ;
	case	2:
		type = RSCodeControl::wiShort ;
		break ;
	case	1:
		type = RSCodeControl::wiByte ;
		break ;
	default:
		return	NULL ;
	}
	return	GetBasicTypeClass( type ) ;
}

RSClass * RSContext::GetFloatClassSizeOf( size_t nBytes ) const
{
	RSCodeControl::WordIndex	type ;
	switch ( nBytes )
	{
	case	8:
		type = RSCodeControl::wiDouble ;
		break ;
	case	4:
		type = RSCodeControl::wiFloat ;
		break ;
	default:
		return	NULL ;
	}
	return	GetBasicTypeClass( type ) ;
}

// Boolean オブジェクト生成
//////////////////////////////////////////////////////////////////////////////
RSBoolean * RSContext::new_Boolean( bool boolValue )
{
	RSObject *	pObj = m_objbufBoolean.New() ;
	if ( pObj == NULL )
	{
		pObj = new RSBoolean( m_pBooleanClass, boolValue ) ;
	}
	else
	{
		ESLAssert( pObj->IsKindOf( ESL_RUNTIME_CLASS(RSBoolean) ) ) ;
		((RSBoolean*)pObj)->SetInteger( boolValue ) ;
	}
	return	(RSBoolean*) pObj ;
}

// Integer オブジェクト生成
//////////////////////////////////////////////////////////////////////////////
RSObject * RSContext::new_Integer( int64_t numValue, RSInteger::IntegerType typeInt )
{
	if ( !(m_flagsBehavior & behaveNoInteger) )
	{
		RSObject *	pObj = m_objbufInteger.New() ;
		if ( pObj == NULL )
		{
			pObj = new RSInteger( m_pIntegerClass, numValue, typeInt ) ;
		}
		else
		{
			ESLAssert( pObj->IsKindOf( ESL_RUNTIME_CLASS(RSInteger) ) ) ;
			((RSInteger*)pObj)->m_intType = typeInt ;
			((RSInteger*)pObj)->SetInteger( numValue ) ;
			pObj->SetRSClass( m_pIntegerClass ) ;
		}
		return	pObj ;
	}
	else
	{
		if ( (numValue < -0x7FFFFFFF) || (numValue > 0x7FFFFFFF) )
		{
			return	new_Number( (double) numValue ) ;
		}
		else
		{
			return	new_NumberInt32( (int32_t) numValue ) ;
		}
	}
}

// Number オブジェクト生成
//////////////////////////////////////////////////////////////////////////////
RSNumber * RSContext::new_NumberInt32( int32_t numValue )
{
	RSObject *	pObj = m_objbufNumber.New() ;
	if ( pObj == NULL )
	{
		pObj = new RSNumber( m_pNumberClass, numValue ) ;
	}
	else
	{
		ESLAssert( pObj->IsKindOf( ESL_RUNTIME_CLASS(RSNumber) ) ) ;
		((RSNumber*)pObj)->SetInteger( numValue ) ;
	}
	return	(RSNumber*) pObj ;
}

RSNumber * RSContext::new_Number( double numValue )
{
	RSObject *	pObj = m_objbufNumber.New() ;
	if ( pObj == NULL )
	{
		pObj = new RSNumber( m_pNumberClass, numValue ) ;
	}
	else
	{
		ESLAssert( pObj->IsKindOf( ESL_RUNTIME_CLASS(RSNumber) ) ) ;
		((RSNumber*)pObj)->SetNumber( numValue ) ;
	}
	return	(RSNumber*) pObj ;
}

// String オブジェクト生成
//////////////////////////////////////////////////////////////////////////////
RSString * RSContext::new_String( const wchar_t * pwszValue )
{
	RSObject *	pObj = m_objbufString.New() ;
	if ( pObj == NULL )
	{
		pObj = new RSString( m_pStringClass, pwszValue ) ;
	}
	else
	{
		ESLAssert( pObj->IsKindOf( ESL_RUNTIME_CLASS(RSString) ) ) ;
		((RSString*)pObj)->m_strValue = pwszValue ;
	}
	return	(RSString*) pObj ;
}

// Reference オブジェクト生成
//////////////////////////////////////////////////////////////////////////////
RSReference * RSContext::new_Reference( RSObject * pRef )
{
	RSObject *	pObj = m_objbufReference.New() ;
	if ( pObj == NULL )
	{
		pObj = new RSReference( pRef ) ;
	}
	else
	{
		ESLAssert( pObj->IsKindOf( ESL_RUNTIME_CLASS(RSReference) ) ) ;
		((RSReference*)pObj)->SetReference( pRef ) ;
	}
	return	(RSReference*) pObj ;
}

// Pointer オブジェクト生成
//////////////////////////////////////////////////////////////////////////////
RSPointer * RSContext::new_Pointer( RSObject * pRef, RSClass * pClass )
{
	RSObject *	pObj = m_objbufPointer.New() ;
	if ( pObj == NULL )
	{
		pObj = new RSPointer( pRef, pClass ) ;
	}
	else
	{
		ESLAssert( pObj->IsKindOf( ESL_RUNTIME_CLASS(RSPointer) ) ) ;
		((RSPointer*)pObj)->SetReference( pRef ) ;
		((RSPointer*)pObj)->SetPointerType( pClass ) ;
	}
	return	(RSPointer*) pObj ;
}

// ReferenceNumber オブジェクト生成
//////////////////////////////////////////////////////////////////////////////
RSReferenceNumber * RSContext::new_ReferenceNumber
	( void * ptrBuf, RSReferenceNumber::NumberType type, RSObject * pRef )
{
	RSObject *	pObj = m_objbufReferenceNumber.New() ;
	if ( pObj == NULL )
	{
		pObj = new RSReferenceNumber( m_pNumberClass, ptrBuf, type, pRef ) ;
	}
	else
	{
		ESLAssert( pObj->IsKindOf( ESL_RUNTIME_CLASS(RSReferenceNumber) ) ) ;
		((RSReferenceNumber*)pObj)->SetReference( ptrBuf, type, pRef ) ;
	}
	return	(RSReferenceNumber*) pObj ;
}

// PointerNumber オブジェクト生成
//////////////////////////////////////////////////////////////////////////////
RSTypedArrayPointer * RSContext::new_PointerNumber
	( RSArrayBuffer * pBuf,
		RSReferenceNumber::NumberType type, size_t iOffset, ssize_t nLimit )
{
	RSObject *	pObj = m_objbufPointerNumber.New() ;
	if ( pObj == NULL )
	{
		RSTypedArrayPointer *	pPtr =
			new RSTypedArrayPointer
				( m_pPtrTypeClass[type], pBuf, type, iOffset, nLimit ) ;
		pObj = pPtr ;
		//
		if ( iOffset & (pPtr->m_nElementBytes - 1) )
		{
			ThrowExceptionError( L"アライメントエラー" ) ;
		}
	}
	else
	{
		ESLAssert( pObj->IsKindOf( ESL_RUNTIME_CLASS(RSTypedArrayPointer) ) ) ;
		RSTypedArrayPointer *	pPtr = (RSTypedArrayPointer*) pObj ;
		pPtr->SetPointer( pBuf, type, iOffset, nLimit ) ;
		pPtr->SetRSClass( m_pPtrTypeClass[type] ) ;
		//
		if ( iOffset & (pPtr->m_nElementBytes - 1) )
		{
			ThrowExceptionError( L"アライメントエラー" ) ;
		}
	}
	return	(RSTypedArrayPointer*) pObj ;
}

// StructuredPointer オブジェクト生成
//////////////////////////////////////////////////////////////////////////////
RSStructuredPointer * RSContext::new_StructuredPointer
	( RSStructuredPointerClass * pType,
		RSArrayBuffer * pBuf, size_t iOffset, ssize_t nLimit )
{
	RSObject *	pObj = m_objbufStructuredPointer.New() ;
	if ( pObj == NULL )
	{
		RSStructuredPointer *
			pPtr = new RSStructuredPointer( pType, pBuf, iOffset, nLimit ) ;
		pObj = pPtr ;
	}
	else
	{
		ESLAssert( pObj->IsKindOf( ESL_RUNTIME_CLASS(RSStructuredPointer) ) ) ;
		RSStructuredPointer *	pPtr = (RSStructuredPointer*) pObj ;
		pPtr->SetPointer( pBuf, pType, iOffset, nLimit ) ;
	}
	if ( iOffset & (pType->GetStructureAlign() - 1) )
	{
		ThrowExceptionError( L"アライメントエラー" ) ;
	}
	return	(RSStructuredPointer*) pObj ;
}

RSStructuredPointer * RSContext::new_StructuredPointer
	( const wchar_t * pwszClass, size_t nLength )
{
	RSClass *	pClass = GetClassAs( pwszClass ) ;
	if ( pClass == NULL )
	{
		RSSourceParser	sparsClass = pwszClass ;
		RSScript		rsScript ;
		SParserErrorTracer	perrTracer ;
		if ( !rsScript.ParseScript
			( *(m_pVM->LockMacroContext()), sparsClass, perrTracer ) )
		{
			RSCodeStream	cs ;
			cs.AttachCode( rsScript ) ;
			pClass = ParseClassExpression( cs ) ;
		}
		m_pVM->UnlockMacroContext() ;
		if ( pClass == NULL )
		{
			ThrowExceptionError( SString(pwszClass) + L" クラスは未定義です" ) ;
			return	NULL ;
		}
	}
	RSStructuredPointerClass *	pStructType =
				ESLTypeCast<RSStructuredPointerClass>( pClass ) ;
	if ( pStructType == NULL )
	{
		ThrowExceptionError( SString(pwszClass) + L" は構造体ではありません" ) ;
		return	NULL ;
	}
	RSArrayBuffer *	pBuf = pStructType->NewBuffer( *this, nLength ) ;
	return	new_StructuredPointer( pStructType, pBuf ) ;
}

// Array オブジェクト生成
//////////////////////////////////////////////////////////////////////////////
RSArray * RSContext::new_Array( size_t nLimit, RSClass * pProto )
{
	RSClass *	pArrayClass = m_pArrayClass ;
	if ( pProto != NULL )
	{
		pArrayClass = m_pVM->GetArrayClassAs( pProto ) ;
	}
	RSObject *	pObj = m_objbufArray.New() ;
	if ( pObj == NULL )
	{
		pObj = new RSArray( pArrayClass, nLimit, pProto ) ;
	}
	else
	{
		ESLAssert( pObj->IsKindOf( ESL_RUNTIME_CLASS(RSArray) ) ) ;
		pObj->SetRSClass( pArrayClass ) ;
		((RSArray*)pObj)->SetArrayPrototype( nLimit, pProto ) ;
	}
	return	(RSArray*) pObj ;
}

// Exception オブジェクト生成
//////////////////////////////////////////////////////////////////////////////
RSException * RSContext::new_Exception( const wchar_t * pwszErr )
{
	return	new RSException
		( m_pExceptionClass, pwszErr, m_pCurParenthesis, m_iSrcStatement ) ;
}

RSException * RSContext::new_Exception
			( const wchar_t * pwszErr, const wchar_t * pwszClass )
{
	RSClass *	pExceptionClass = m_pExceptionClass ;
	if ( pwszClass != NULL )
	{
		pExceptionClass = m_pVM->GetExceptioinClassAs( pwszClass ) ;
	}
	return	new RSException
		( pExceptionClass, pwszErr, m_pCurParenthesis, m_iSrcStatement ) ;
}

RSException * RSContext::new_Exception
			( const wchar_t * pwszErr, RSClass * pClass )
{
	return	new RSException
		( (pClass!= nullptr) ? pClass : m_pExceptionClass,
				pwszErr, m_pCurParenthesis, m_iSrcStatement ) ;
}

// オブジェクト生成
//////////////////////////////////////////////////////////////////////////////
RSObject * RSContext::new_Object( const wchar_t * pwszClass )
{
	RSClass *	pClass = GetClassAs( pwszClass ) ;
	if ( pClass == NULL )
	{
		RSSourceParser	sparsClass = pwszClass ;
		RSScript		rsScript ;
		SParserErrorTracer	perrTracer ;
		if ( !rsScript.ParseScript
			( *(m_pVM->LockMacroContext()), sparsClass, perrTracer ) )
		{
			RSCodeStream	cs ;
			cs.AttachCode( rsScript ) ;
			pClass = ParseClassExpression( cs ) ;
		}
		m_pVM->UnlockMacroContext() ;
		if ( pClass == NULL )
		{
			ThrowExceptionError( SString(pwszClass) + L" クラスは未定義です" ) ;
			return	NULL ;
		}
	}
	RSObject *	pArg = new_Array() ;
	RSObject *	pObj = pClass->NewInstance( *this, pArg ) ;
	ReleaseObjectRef( pArg ) ;
	return	pObj ;
}

RSObject * RSContext::new_Object( RSClass * pClass )
{
	ESLAssert( pClass != nullptr ) ;
	RSObject *	pArg = new_Array() ;
	RSObject *	pObj = pClass->NewInstance( *this, pArg ) ;
	ReleaseObjectRef( pArg ) ;
	return	pObj ;
}

RSPointer * RSContext::new_ObjectPointer( const wchar_t * pwszClass )
{
	RSClass *	pClass = GetClassAs( pwszClass ) ;
	if ( pClass == NULL )
	{
		ThrowExceptionError( SString(pwszClass) + L" クラスは未定義です" ) ;
		return	NULL ;
	}
	RSObject *	pArg = new_Array() ;
	RSObject *	pObj = pClass->NewInstance( *this, pArg ) ;
	ReleaseObjectRef( pArg ) ;
	//
	return	new_Pointer( pObj, pClass ) ;
}

// 名前空間オブジェクト生成
//////////////////////////////////////////////////////////////////////////////
RSNamespace * RSContext::new_Namespace( void )
{
	RSObject *	pObj = m_objbufNamespace.New() ;
	if ( pObj == NULL )
	{
		pObj = new RSNamespace ;
	}
	else
	{
		ESLAssert( pObj->IsKindOf( ESL_RUNTIME_CLASS(RSNamespace) ) ) ;
	}
	return	(RSNamespace*) pObj ;
}

RSNamespace * RSContext::new_Namespace
	( RSObject * pRefSpace, uint32_t accMod, RSObject * pBackLink )
{
	RSObject *	pObj = m_objbufNamespace.New() ;
	if ( pObj == NULL )
	{
		pObj = new RSNamespace( pRefSpace, accMod, pBackLink ) ;
	}
	else
	{
		ESLAssert( pObj->IsKindOf( ESL_RUNTIME_CLASS(RSNamespace) ) ) ;
		((RSNamespace*)pObj)->
			AttachReference( pRefSpace, accMod, pBackLink ) ;
	}
	return	(RSNamespace*) pObj ;
}

// オブジェクト参照開放
//////////////////////////////////////////////////////////////////////////////
void RSContext::ReleaseObjectRef( RSObject * pObj )
{
	if ( pObj != NULL )
	{
		if ( SSystem::AtomicSub( &(pObj->m_countRef), 1 ) <= 0 )
		{
			ESLAssert( pObj->m_countRef == 0 ) ;
			switch ( pObj->GetBasicType() )
			{
			case	RSObject::typeNumber:
				m_objbufNumber.Free( *this, pObj ) ;
				break ;
			case	RSObject::typeInteger:
				m_objbufInteger.Free( *this, pObj ) ;
				break ;
			case	RSObject::typeBoolean:
				m_objbufBoolean.Free( *this, pObj ) ;
				break ;
			case	RSObject::typeString:
				m_objbufString.Free( *this, pObj ) ;
				break ;
			case	RSObject::typeArray:
				m_objbufArray.Free( *this, pObj ) ;
				break ;
			case	RSObject::typeReference:
				m_objbufReference.Free( *this, pObj ) ;
				break ;
			case	RSObject::typePointer:
				m_objbufPointer.Free( *this, pObj ) ;
				break ;
			case	RSObject::typeReferenceNumber:
				m_objbufReferenceNumber.Free( *this, pObj ) ;
				break ;
			case	RSObject::typePointerNumber:
				m_objbufPointerNumber.Free( *this, pObj ) ;
				break ;
			case	RSObject::typeStrucuredPointer:
				m_objbufStructuredPointer.Free( *this, pObj ) ;
				break ;
			case	RSObject::typeNamespace:
				m_objbufNamespace.Free( *this, pObj ) ;
				break ;
			default:
				pObj->AddRef() ;
				pObj->Finalize( *this ) ;
				delete	pObj ;
				break ;
			}
		}
	}
}

// 配列要素取得
//////////////////////////////////////////////////////////////////////////////
int64_t RSContext::GetObjElementIntegerAt
	( RSObject * pObj, int nIndex, int64_t nDefault, bool * pError )
{
	if ( pObj == NULL )
	{
		if ( pError != NULL )
		{
			*pError = true ;
		}
		return	nDefault ;
	}
	return	pObj->GetElementIntegerAt( *this, nIndex, nDefault, pError ) ;
}

double RSContext::GetObjElementNumberAt
	( RSObject * pObj, int nIndex, double nDefault, bool * pError )
{
	if ( pObj == NULL )
	{
		if ( pError != NULL )
		{
			*pError = true ;
		}
		return	nDefault ;
	}
	return	pObj->GetElementNumberAt( *this, nIndex, nDefault, pError ) ;
}

SSystem::SString RSContext::GetObjElementStringAt
	( RSObject * pObj, int nIndex,
		const wchar_t * pwszDefault, bool * pError )
{
	if ( pObj == NULL )
	{
		if ( pError != NULL )
		{
			*pError = true ;
		}
		return	pwszDefault ;
	}
	return	pObj->GetElementStringAt( *this, nIndex, pwszDefault, pError ) ;
}

// 配列要素設定
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSContext::SetObjElementIntegerAt
	( RSObject * pObj, int nIndex, int64_t nValue )
{
	if ( pObj == NULL )
	{
		return	errFailed ;
	}
	return	pObj->SetElementIntegerAt( *this, nIndex, nValue ) ;
}

SSystem::SError RSContext::SetObjElementNumberAt
	( RSObject * pObj, int nIndex, double nValue )
{
	if ( pObj == NULL )
	{
		return	errFailed ;
	}
	return	pObj->SetElementNumberAt( *this, nIndex, nValue ) ;
}

SSystem::SError RSContext::SetObjElementStringAt
	( RSObject * pObj, int nIndex, const wchar_t * pwszValue )
{
	if ( pObj == NULL )
	{
		return	errFailed ;
	}
	return	pObj->SetElementStringAt( *this, nIndex, pwszValue ) ;
}

// メンバ要素取得
//////////////////////////////////////////////////////////////////////////////
int64_t RSContext::GetObjMemberIntegerAs
	( RSObject * pObj, const wchar_t * pwszMember,
						int64_t nDefault, bool * pError )
{
	if ( pObj == NULL )
	{
		if ( pError != NULL )
		{
			*pError = true ;
		}
		return	nDefault ;
	}
	return	pObj->GetMemberIntegerAs( *this, pwszMember, nDefault, pError ) ;
}

double RSContext::GetObjMemberNumberAs
	( RSObject * pObj, const wchar_t * pwszMember,
						double nDefault, bool * pError )
{
	if ( pObj == NULL )
	{
		if ( pError != NULL )
		{
			*pError = true ;
		}
		return	nDefault ;
	}
	return	pObj->GetMemberNumberAs( *this, pwszMember, nDefault, pError ) ;
}

SSystem::SString RSContext::GetObjMemberStringAs
	( RSObject * pObj, const wchar_t * pwszMember,
						const wchar_t * pwszDefault, bool * pError )
{
	if ( pObj == NULL )
	{
		if ( pError != NULL )
		{
			*pError = true ;
		}
		return	pwszDefault ;
	}
	return	pObj->GetMemberStringAs( *this, pwszMember, pwszDefault, pError ) ;
}

// メンバ要素設定
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSContext::SetObjMemberIntegerAs
	( RSObject * pObj, const wchar_t * pwszMember, int64_t nValue )
{
	if ( pObj == NULL )
	{
		return	errFailed ;
	}
	return	pObj->SetMemberIntegerAs( *this, pwszMember, nValue ) ;
}

SSystem::SError RSContext::SetObjMemberNumberAs
	( RSObject * pObj, const wchar_t * pwszMember, double nValue )
{
	if ( pObj == NULL )
	{
		return	errFailed ;
	}
	return	pObj->SetMemberNumberAs( *this, pwszMember, nValue ) ;
}

SSystem::SError RSContext::SetObjMemberStringAs
	( RSObject * pObj,
		const wchar_t * pwszMember, const wchar_t * pwszValue )
{
	if ( pObj == NULL )
	{
		return	errFailed ;
	}
	return	pObj->SetMemberStringAs( *this, pwszMember, pwszValue ) ;
}

// 文字列書式化
//////////////////////////////////////////////////////////////////////////////
void RSContext::FormatStringVlist
	( SSystem::SString& strDst,
		const wchar_t * pwszFormat,
		RSObject**const ppArgs, size_t countArg )
{
	SStringParser	sparsFormat = pwszFormat ;
	SArgList		arg( ppArgs, countArg ) ;
	sparsFormat.Format( strDst, arg ) ;
}


//////////////////////////////////////////////////////////////////////////////
// printf パラメータ
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSContext::SArgList::SArgList( RSObject**const ppArgs, size_t countArg )
	: m_ppArgs( ppArgs ), m_countArg( countArg ), m_iNextArg( 0 )
{
}

// 整数取得
//////////////////////////////////////////////////////////////////////////////
int RSContext::SArgList::IntAt( size_t iArg, int nDefault ) const
{
	return	(int) LongAt( iArg, nDefault ) ;
}

int64_t RSContext::SArgList::LongAt( size_t iArg, int64_t nDefault ) const
{
	if ( iArg < m_countArg )
	{
		RSObject *	pObj = m_ppArgs[iArg] ;
		if ( pObj != NULL )
		{
			int64_t	num ;
			if ( pObj->AsInteger( num ) )
			{
				return	num ;
			}
		}
	}
	return	nDefault ;
}

int RSContext::SArgList::NextInt( int nDefault )
{
	return	IntAt( m_iNextArg ++, nDefault ) ;
}

int64_t RSContext::SArgList::NextLong( int64_t nDefault )
{
	return	LongAt( m_iNextArg ++, nDefault ) ;
}

// ブール値取得
//////////////////////////////////////////////////////////////////////////////
bool RSContext::SArgList::BooleanAt( size_t iArg, bool fDefault ) const
{
	if ( iArg < m_countArg )
	{
		RSObject *	pObj = m_ppArgs[iArg] ;
		if ( pObj != NULL )
		{
			return	pObj->AsBoolean() ;
		}
	}
	return	fDefault ;
}

bool RSContext::SArgList::NextBoolean( bool fDefault )
{
	return	BooleanAt( m_iNextArg ++, fDefault ) ;
}

// 浮動小数点取得
//////////////////////////////////////////////////////////////////////////////
double RSContext::SArgList::DoubleAt( size_t iArg, double nDefault ) const
{
	if ( iArg < m_countArg )
	{
		RSObject *	pObj = m_ppArgs[iArg] ;
		if ( pObj != NULL )
		{
			double	num ;
			if ( pObj->AsRealNumber( num ) )
			{
				return	num ;
			}
		}
	}
	return	nDefault ;
}

double RSContext::SArgList::NextDouble( double nDefault )
{
	return	DoubleAt( m_iNextArg ++, nDefault ) ;
}

// 文字列取得
//////////////////////////////////////////////////////////////////////////////
SString RSContext::SArgList::StringAt( size_t iArg, const wchar_t * pwszDefault ) const
{
	if ( iArg < m_countArg )
	{
		RSObject *	pObj = m_ppArgs[iArg] ;
		if ( (pObj != NULL) && (pObj->GetEntityObject() != NULL) )
		{
			SString	str ;
			if ( pObj->AsString( str ) )
			{
				return	str ;
			}
		}
	}
	return	pwszDefault ;
}

SSystem::SString RSContext::SArgList::NextString( const wchar_t * pwszDefault )
{
	return	StringAt( m_iNextArg ++, pwszDefault ) ;
}

// オブジェクト取得
//////////////////////////////////////////////////////////////////////////////
RSObject * RSContext::SArgList::ObjectAt( size_t iArg ) const
{
	if ( iArg < m_countArg )
	{
		RSObject *	pObj = m_ppArgs[iArg] ;
		if ( pObj != NULL )
		{
			return	pObj->GetEntityObject() ;
		}
	}
	return	NULL ;
}

ESLObject * RSContext::SArgList::NativeObjectAt( size_t iArg ) const
{
	RSNativeObject *
		pObj = ESLTypeCast<RSNativeObject>( ObjectAt( iArg ) ) ;
	if ( pObj != NULL )
	{
		return	pObj->GetObject() ;
	}
	return	NULL ;
}

RSObject * RSContext::SArgList::NextObject( void )
{
	return	ObjectAt( m_iNextArg ++ ) ;
}

ESLObject * RSContext::SArgList::NextNativeObject( void )
{
	return	NativeObjectAt( m_iNextArg ++ ) ;
}

// ポインタ取得
//////////////////////////////////////////////////////////////////////////////
uint8_t * RSContext::SArgList::PointerAt( size_t iArg, size_t * pBoundSize ) const
{
	RSTypedArrayPointer *
		pPtr = ESLTypeCast<RSTypedArrayPointer>( ObjectAt( iArg ) ) ;
	if ( pPtr != NULL )
	{
		if ( pBoundSize != NULL )
		{
			*pBoundSize = pPtr->m_nLimit ;
		}
		return	pPtr->GetPointer() ;
	}
	if ( pBoundSize != NULL )
	{
		*pBoundSize = 0 ;
	}
	return	NULL ;
}

uint8_t * RSContext::SArgList::PointerAt( size_t iArg, size_t nReqSize ) const
{
	RSTypedArrayPointer *
		pPtr = ESLTypeCast<RSTypedArrayPointer>( ObjectAt( iArg ) ) ;
	if ( pPtr != NULL )
	{
		if ( pPtr->m_nLimit < nReqSize )
		{
			return	NULL ;
		}
		return	pPtr->GetPointer() ;
	}
	return	NULL ;
}

uint8_t * RSContext::SArgList::NextPointer( size_t nReqSize )
{
	return	PointerAt( m_iNextArg ++, nReqSize ) ;
}

// 次の整数取得
//////////////////////////////////////////////////////////////////////////////
int64_t RSContext::SArgList::NextInteger( void )
{
	if ( m_iNextArg < m_countArg )
	{
		RSObject *	pObj = m_ppArgs[m_iNextArg ++] ;
		if ( pObj != NULL )
		{
			int64_t	num ;
			if ( pObj->AsInteger( num ) )
			{
				return	num ;
			}
		}
	}
	return	0 ;
}

// 次の浮動小数点取得
//////////////////////////////////////////////////////////////////////////////
double RSContext::SArgList::NextDouble( void )
{
	if ( m_iNextArg < m_countArg )
	{
		RSObject *	pObj = m_ppArgs[m_iNextArg ++] ;
		if ( pObj != NULL )
		{
			double	num ;
			if ( pObj->AsRealNumber( num ) )
			{
				return	num ;
			}
		}
	}
	return	0.0 ;
}

// 次の文字取得
//////////////////////////////////////////////////////////////////////////////
wchar_t RSContext::SArgList::NextCharacter( void )
{
	return	(wchar_t) NextInteger() ;
}

// 次の文字列取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * RSContext::SArgList::NextString( SSystem::SString& strNext )
{
	if ( m_iNextArg < m_countArg )
	{
		RSObject *	pObj = m_ppArgs[m_iNextArg ++] ;
		if ( pObj != NULL )
		{
			if ( pObj->AsString( strNext ) )
			{
				return	strNext ;
			}
		}
	}
	strNext = L"(null)" ;
	return	strNext ;
}


//////////////////////////////////////////////////////////////////////////////
// 数式インスタンス
//////////////////////////////////////////////////////////////////////////////

// 数式計算（名前空間を作成して式を評価）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSExpression::PerformExpression
	( RSVirtualMachine * pVM,
		const wchar_t * pwszExpr,
		RSObject * pThisObj,
		SSystem::SParserErrorInterface* pperr )
{
	PrepareContext( pVM ) ;
	SetSourceCode( pVM, pwszExpr ) ;
	return	PerformExpression( pThisObj, pperr ) ;
}

// 実行コンテキスト準備（既にある場合には何もしない）
//////////////////////////////////////////////////////////////////////////////
void RSExpression::PrepareContext( RSVirtualMachine * pVM, RSObject * pThread )
{
	if ( m_context == NULL )
	{
		m_context = new RSContext( pVM, pThread ) ;
	}
}

// 数式設定
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSExpression::SetSourceCode
	( RSVirtualMachine * pVM, const wchar_t * pwszCode )
{
	SParserErrorTracer	perrTracer ;
	RSSourceParser	sparsExpr = pwszCode ;
	m_source = new RSScript ;
	//
	SError	err =
		m_source->ParseScript
			( *(pVM->LockMacroContext()), sparsExpr, perrTracer ) ;
	pVM->UnlockMacroContext() ;
	if ( err )
	{
		m_source = NULL ;
		return	err ;
	}
	m_code.AttachCode( *m_source ) ;
	return	errSuccess ;
}

// 数式計算（名前空間を作成して設定された数式を評価）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSExpression::PerformExpression
	( RSObject * pThisObj, SSystem::SParserErrorInterface* pperr )
{
	ESLAssert( m_context != NULL ) ;
	ESLAssert( m_source != NULL ) ;
	m_code.SeekIndex( 0 ) ;
	//
	RSObject *	pObj = m_context->PerformExpression( m_code, pThisObj ) ;
	if ( pperr != NULL )
	{
		m_context->OutputExceptionError( *pperr ) ;
	}
	return	pObj ;
}

// 例外発生判定
//////////////////////////////////////////////////////////////////////////////
bool RSExpression::IsException( void ) const
{
	return	(m_context != NULL) && m_context->IsException() ;
}

// 例外取得
//////////////////////////////////////////////////////////////////////////////
RSObject * RSExpression::GetException( void ) const
{
	return	(m_context != NULL) ? m_context->GetException() : NULL ;
}

// 例外クリア
//////////////////////////////////////////////////////////////////////////////
void RSExpression::ClearException( void )
{
	if ( m_context != NULL )
	{
		m_context->ClearException() ;
	}
}



//////////////////////////////////////////////////////////////////////////////
// 基底デバッガ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSDebugger, DebugListener )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSDebugger::RSDebugger( void )
{
	m_modeTrace = traceStepIn ;
	m_countTraceSteps = 1 ;
	m_nestCall = 0 ;
}

// ブレークポイント追加
//////////////////////////////////////////////////////////////////////////////
void RSDebugger::AddBreakPoint( const RSDebugger::BreakPoint& bp )
{
	m_arrBreakPoints.Add( bp ) ;
}

void RSDebugger::AddBreakPointLineAt
	( const RSScript * pScript, size_t nLineNum )
{
	BreakPoint	bp ;
	bp.pScript = pScript ;
	bp.iSrcChar = nLineNum ;
	m_arrBreakPoints.Add( bp ) ;
}

// ブレークポイント数
//////////////////////////////////////////////////////////////////////////////
size_t RSDebugger::GetBreakPointCount( void ) const
{
	return	m_arrBreakPoints.GetLength() ;
}

// ブレークポイント取得
//////////////////////////////////////////////////////////////////////////////
const RSDebugger::BreakPoint * RSDebugger::GetBreakPointAt( size_t i ) const
{
	return	m_arrBreakPoints.GetAt( i ) ;
}

// ブレークポイント一致判定
//////////////////////////////////////////////////////////////////////////////
ssize_t RSDebugger::FindBreakPoint
	( const RSScript * pScript, size_t iIndex ) const
{
	const BreakPoint *	pbp = m_arrBreakPoints.GetConstArray() ;
	size_t				nCount = m_arrBreakPoints.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		if ( (pbp[i].pScript == pScript)
			&& (pbp[i].iSrcChar == iIndex) )
		{
			return	(ssize_t) i ;
		}
	}
	return	-1 ;
}

ssize_t RSDebugger::FindBreakPoint
	( const RSScript * pScript, size_t iFirst, size_t iEnd ) const
{
	const BreakPoint *	pbp = m_arrBreakPoints.GetConstArray() ;
	size_t				nCount = m_arrBreakPoints.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		if ( (pbp[i].pScript == pScript)
			&& (pbp[i].iSrcChar >= iFirst)
			&& (pbp[i].iSrcChar < iEnd) )
		{
			return	(ssize_t) i ;
		}
	}
	return	-1 ;
}

// ブレークポイント削除
//////////////////////////////////////////////////////////////////////////////
void RSDebugger::RemoveBreakPointAt( size_t i )
{
	m_arrBreakPoints.RemoveAt( i ) ;
}

void RSDebugger::RemoveBreakPoint( const BreakPoint& bp )
{
	ssize_t	i = FindBreakPoint( bp.pScript, bp.iSrcChar ) ;
	if ( i >= 0 )
	{
		m_arrBreakPoints.RemoveAt( (size_t) i ) ;
	}
}

void RSDebugger::RemoveAllBreakPoints( void )
{
	m_arrBreakPoints.RemoveAll() ;
}

// ソース位置から行番号へ変換
//////////////////////////////////////////////////////////////////////////////
size_t RSDebugger::GetLineNumberOf( const BreakPoint& bp )
{
	return	GetLineNumberOf( bp.pScript, bp.iSrcChar ) ;
}

size_t RSDebugger::GetLineNumberOf( const RSScript * pScript, size_t iIndex )
{
	SStringParser	sparsSrc ;
	sparsSrc.AttachString( pScript->GetSourceText() ) ;
	return	sparsSrc.GetLineNumberOf( iIndex ) ;
}

// デバッグ
//////////////////////////////////////////////////////////////////////////////
void RSDebugger::OnDebug
	( RSContext * context,
		const RSScript * pScript, size_t iSrcChar )
{
	if ( m_modeTrace != traceExecute )
	{
		if ( m_modeTrace == traceStepOver )
		{
			if ( m_nestCall == 0 )
			{
				OnStepExecution( context, stepBreak, pScript, iSrcChar ) ;
				return ;
			}
		}
		else if ( m_modeTrace == traceStepIn )
		{
			if ( m_countTraceSteps > 1 )
			{
				m_countTraceSteps -- ;
				OnStepExecution( context, stepContinue, pScript, iSrcChar ) ;
				return ;
			}
			else if ( m_countTraceSteps == 1 )
			{
				m_countTraceSteps = 0  ;
				OnStepExecution( context, stepBreak, pScript, iSrcChar ) ;
				return ;
			}
		}
	}
	const BreakPoint *	pbp = m_arrBreakPoints.GetConstArray() ;
	size_t				nCount = m_arrBreakPoints.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		if ( (pbp[i].pScript == pScript)
			&& (pbp[i].iSrcChar == iSrcChar) )
		{
			BreakPoint	bp = pbp[i] ;
			OnBreakPoint( context, bp ) ;
			return ;
		}
	}
	if ( IsBreakExecution() )
	{
		OnStepExecution( context, stepBreak, pScript, iSrcChar ) ;
		return ;
	}
}

// 関数呼び出し
//////////////////////////////////////////////////////////////////////////////
void RSDebugger::OnCall
	( RSContext * context, RSFunctionPrototype * pProto )
{
	m_nestCall ++ ;
}

// 関数復帰
//////////////////////////////////////////////////////////////////////////////
void RSDebugger::OnReturned
	( RSContext * context,
		const RSScript * pScript, size_t iSrcChar )
{
	if ( m_nestCall > 0 )
	{
		m_nestCall -- ;
	}
}

// ブレークポイント
//////////////////////////////////////////////////////////////////////////////
void RSDebugger::OnBreakPoint
	( RSContext * context, const BreakPoint& bp )
{
}

// ステップ実行
//////////////////////////////////////////////////////////////////////////////
void RSDebugger::OnStepExecution
	( RSContext * context,
		StepExecution sx,
		const RSScript * pScript, size_t iSrcChar )
{
}

// ステップ実行割り込み停止判定
//////////////////////////////////////////////////////////////////////////////
bool RSDebugger::IsBreakExecution( void )
{
	return	false ;
}

// ステップオーバー停止位置設置
//////////////////////////////////////////////////////////////////////////////
void RSDebugger::PutBreakStepOver
	( const RSScript * pScript, size_t iSrcChar )
{
	m_modeTrace = traceStepOver ;
	m_nestCall = 0 ;
}

// トレース実行設定
//////////////////////////////////////////////////////////////////////////////
void RSDebugger::PutTraceSteps( size_t nSteps )
{
	m_modeTrace = traceStepIn ;
	m_countTraceSteps = nSteps ;
}

// 実行設定
//////////////////////////////////////////////////////////////////////////////
void RSDebugger::PutTraceExecution( void )
{
	m_modeTrace = traceExecute ;
}

