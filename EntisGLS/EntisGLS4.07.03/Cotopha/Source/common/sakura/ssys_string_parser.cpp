
#include <sakura/sakura.h>

using namespace SSystem ;


//////////////////////////////////////////////////////////////////////////////
// 文字列パーサー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2( SSystem::SStringParser, ESLObject, SString )

uint32_t	SStringParser::m_maskPunctuation[4] =
{
	0xFFFFFFFF,		// All control code is punctuation.
	0x7C00FFFF,		// " !"#$%&'()*+,-./" and ":;<=>" are punctuation.
	0x78000000,		// "[\]^" are punctuation.
	0xF8000001,		// '`' and "{|}~ " are punctuation.
} ;

uint32_t	SStringParser::m_maskSpecialMark[4] =
{
	0x00000000,		//
	0x58001384,		// ""'(),;<>" are special punctuation.
	0x28000000,		// "[]" are special punctuation.
	0x28000000,		// "{}" are special punctuation.
} ;

// U+3041 - U+3096
const wchar_t *const	SStringParser::s_pwszHiragana =
	L"あいうえお"
	L"かきくけこ"
	L"がぎぐげご"
	L"か゚き゚く゚け゚こ゚"
	L"さしすせそ"
	L"ざじずぜぞ"
	L"たちつてと"
	L"だぢづでど"
	L"なにぬねの"
	L"はひふへほ"
	L"ばびぶべぼ"
	L"ぱぴぷぺぽ"
	L"まみむめも"
	L"やゆよ"
	L"らりるれろ"
	L"わゐをゑん"
	L"ぁぃぅぇぉ"
	L"ゃゅょっゎ"
	L"ゔゕゖ" ;

// U+30A1 - U+30F6 (- U+30FA)
const wchar_t *const	SStringParser::s_pwszKatakana =
	L"アイウエオ"
	L"カキクケコ"
	L"ガギグゲゴ"
	L"カ゚キ゚ク゚ケ゚コ゚"
	L"サシスセソ"
	L"ザジズゼゾ"
	L"タチツテト"
	L"ダヂヅデド"
	L"ナニヌネノ"
	L"ハヒフヘホ"
	L"バビブベボ"
	L"パピプペポ"
	L"マミムメモ"
	L"ヤユヨ"
	L"ラリルレロ"
	L"ワヰヱヲン"
	L"ァィゥェォ"
	L"ャュョッヮ"
	L"ヴヵヶ"
	L"ヷヸヺヺ" ;

const wchar_t *const	SStringParser::s_pwszKomojiKana =
	L"ぁぃぅぇぉゃゅょっゎゕゖ"
	L"ァィゥェォャュョッヮヵヶ" ;

const wchar_t *const	SStringParser::s_pwszVoicelessKana =
	L"ハヒフヘホ"
	L"カキクケコ"
	L"サシスセソ"
	L"タチツテト" ;

const wchar_t *const	SStringParser::s_pwszVoicedKana =
	L"バビブベボ"
	L"ガギグゲゴ"
	L"ザジズゼゾ"
	L"ダヂヅデド" ;

const wchar_t *const	SStringParser::s_pwszSemivoicedKana =
	L"パピプペポ"
	L"カ゚キ゚ク゚ケ゚コ゚" ;

const wchar_t *const	SStringParser::s_pwszAuxiliaryKana =
	L"ーヽヾ゛゜ゝゞ仝々" ;

const wchar_t *const	SStringParser::s_pwszPunctuation =
	L",.:;、。，．・：；？！" ;

const wchar_t *const	SStringParser::s_pwszParenthesis =
	L"\"\'()<>[]{}‘’“”（）〔〕［］｛｝〈〉《》「」『』【】" ;

// U+FF21 - U+FF3A, U+FF41 - U+FF5A
// (U+FF01 - U+FF5E 全角ASCII <=> U+0021 - U+007E 半角ASCII)
const wchar_t *const	SStringParser::s_pwszJISAlphabet =
	L"ＡＢＣＤＥＦＧＨＩＪＫＬＭＯＮＰＱＲＳＴＵＷＷＸＹＺ"
	L"ａｂｃｄｅｆｇｉｉｊｋｌｍｎｏｐｑｒｓｔｕｖｗｘｙｚ" ;

const wchar_t *const	SStringParser::s_pwszJISNumber =
	L"０１２３４５６７８９" ;

// U+FF61 - U+FF9F 半角カナ <=> SJIS 0xA1 - 0xDF
// 0xFF61 - 0xFF9F 対応全角文字
const wchar_t	SStringParser::s_wchWideKana[0x40] =
		L"。「」、・ヲァィゥェォャュョッ"
		L"ーアイウエオカキクケコサシスセソ"
		L"タチツテトナニヌネノハヒフヘホマ"
		L"ミムメモヤユヨラリルレロワン゛゜" ;


// 構築関数
//////////////////////////////////////////////////////////////////////////////
SStringParser::SStringParser( void )
{
	m_pszText = NULL ;
	m_lenText = 0 ;
	m_index = 0 ;
	m_mark = 0 ;
	m_pwszFilePath = NULL ;
}

SStringParser::SStringParser( const SStringParser& ss )
	: SString( (const SString&) ss )
{
	if ( SString::m_ptrArray != NULL )
	{
		m_pszText = SString::m_ptrArray ;
		m_lenText = SString::m_nLength ;
	}
	else
	{
		m_pszText = ss.m_pszText ;
		m_lenText = ss.m_lenText ;
	}
	m_index = ss.m_index ;
	m_mark = ss.m_mark ;
	m_pwszFilePath = NULL ;
	if ( ss.m_pwszFilePath != NULL )
	{
		SetFilePath( ss.m_pwszFilePath ) ;
	}
}

SStringParser::SStringParser( const SString& src )
	: SString( src )
{
	m_pszText = SString::m_ptrArray ;
	m_lenText = SString::m_nLength ;
	m_index = 0 ;
	m_mark = 0 ;
	m_pwszFilePath = NULL ;
}

#if	!defined(__COTOPHA__)
SStringParser::SStringParser( const wchar_t * pszSrc, ssize_t nLength )
	: SString( pszSrc, nLength )
{
	m_pszText = SString::m_ptrArray ;
	m_lenText = SString::m_nLength ;
	m_index = 0 ;
	m_mark = 0 ;
	m_pwszFilePath = NULL ;
}
#endif

#if	!defined(__WCHAR_EQU_UINT16__)
SStringParser::SStringParser( const uint16_t * pszSrc, ssize_t nLength )
	: SString( pszSrc, nLength )
{
	m_pszText = SString::m_ptrArray ;
	m_lenText = SString::m_nLength ;
	m_index = 0 ;
	m_mark = 0 ;
	m_pwszFilePath = NULL ;
}
#endif

SStringParser::SStringParser( const char * pszSrc, ssize_t nLength )
	: SString( pszSrc, nLength )
{
	m_pszText = SString::m_ptrArray ;
	m_lenText = SString::m_nLength ;
	m_index = 0 ;
	m_mark = 0 ;
	m_pwszFilePath = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SStringParser::~SStringParser( void )
{
	if ( m_pwszFilePath != NULL )
	{
		esl_free( m_pwszFilePath ) ;
		m_pwszFilePath = NULL ;
	}
}

// 文字列関連付け
//////////////////////////////////////////////////////////////////////////////
void SStringParser::AttachString( void )
{
	m_pszText = SString::m_ptrArray ;
	m_lenText = SString::m_nLength ;
	m_index = 0 ;
	m_mark = 0 ;
}

void SStringParser::AttachString( const SStringParser& ss )
{
	SString::FreeArray() ;
	//
	m_pszText = ss.m_pszText ;
	m_lenText = ss.m_lenText ;
	m_index = ss.m_index ;
	m_mark = ss.m_mark ;
}

void SStringParser::AttachString( const SString& strSrc )
{
	SString::FreeArray() ;
	//
	m_pszText = strSrc.GetConstArray() ;
	m_lenText = strSrc.GetLength() ;
	m_index = 0 ;
	m_mark = 0 ;
}

void SStringParser::AttachSubString
	( const SStringParser& ssSrc, size_t iFirst, ssize_t iEnd )
{
	SString::FreeArray() ;
	//
	m_pszText = ssSrc.GetConstArray() ;
	m_lenText = ssSrc.GetLength() ;
	if ( iFirst < m_lenText )
	{
		m_pszText += iFirst ;
		if ( (iEnd < 0) || ((size_t) iEnd > m_lenText) )
		{
			m_lenText -= iFirst ;
		}
		else
		{
			m_lenText = iEnd - iFirst ;
		}
	}
	else
	{
		m_pszText += m_lenText ;
		m_lenText = 0 ;
	}
	m_index = 0 ;
	m_mark = 0 ;
	m_pwszFilePath = NULL ;
}

void SStringParser::AttachSubString
	( const SString& strSrc, size_t iFirst, ssize_t iEnd )
{
	SString::FreeArray() ;
	//
	m_pszText = strSrc.GetConstArray() ;
	m_lenText = strSrc.GetLength() ;
	if ( iFirst < m_lenText )
	{
		m_pszText += iFirst ;
		if ( (iEnd < 0) || ((size_t) iEnd > m_lenText) )
		{
			m_lenText -= iFirst ;
		}
		else
		{
			m_lenText = iEnd - iFirst ;
		}
	}
	else
	{
		m_pszText += m_lenText ;
		m_lenText = 0 ;
	}
	m_index = 0 ;
	m_mark = 0 ;
	m_pwszFilePath = NULL ;
}

void SStringParser::AttachString
	( const uint16_t * pszSrc, ssize_t nLength )
{
	SString::FreeArray() ;
	//
	if ( (pszSrc != NULL) & (nLength < 0) )
	{
		nLength = 0 ;
		while ( pszSrc[nLength] != 0 )
		{
			nLength ++ ;
		}
	}
	m_pszText = pszSrc ;
	m_lenText = nLength ;
	m_index = 0 ;
	m_mark = 0 ;
}

// 文字列解放
//////////////////////////////////////////////////////////////////////////////
void SStringParser::ReleaseString( void )
{
	AttachString( NULL, 0 ) ;
}

// 文字列設定
//////////////////////////////////////////////////////////////////////////////
void SStringParser::SetString( const SString& strSrc )
{
	SString::SetString( strSrc ) ;
	//
	m_pszText = SString::m_ptrArray ;
	m_lenText = SString::m_nLength ;
	m_index = 0 ;
	m_mark = 0 ;
}

#if	!defined(__COTOPHA__)
void SStringParser::SetString( const wchar_t * pszSrc, ssize_t nLength )
{
	SString::SetString( pszSrc, nLength ) ;
	//
	m_pszText = SString::m_ptrArray ;
	m_lenText = SString::m_nLength ;
	m_index = 0 ;
	m_mark = 0 ;
}
#endif

#if	!defined(__WCHAR_EQU_UINT16__)
void SStringParser::SetString( const uint16_t * pszSrc, ssize_t nLength )
{
	SString::SetString( pszSrc, nLength ) ;
	//
	m_pszText = SString::m_ptrArray ;
	m_lenText = SString::m_nLength ;
	m_index = 0 ;
	m_mark = 0 ;
}
#endif

void SStringParser::SetString( const char * pszSrc, ssize_t nLength )
{
	SString::SetString( pszSrc, nLength ) ;
	//
	m_pszText = SString::m_ptrArray ;
	m_lenText = SString::m_nLength ;
	m_index = 0 ;
	m_mark = 0 ;
}

// 代入操作
//////////////////////////////////////////////////////////////////////////////
const SStringParser& SStringParser::operator = ( const SStringParser& ss )
{
	if ( ss.GetConstArray() != NULL )
	{
		SetString( (const SString&) ss ) ;
		m_index = ss.m_index ;
	}
	else
	{
		AttachString( ss ) ;
	}
	return	*this ;
}

// ファイルから読み込み
//////////////////////////////////////////////////////////////////////////////
SError SStringParser::LoadTextFile
	( const wchar_t * pwszFilePath, Charset::EncodingType encoding )
{
	SSmartPointer<SFileInterface>	pFile =
		SFileOpener::DefaultNewOpenFile( pwszFilePath, SFileOpener::shareRead ) ;
	if ( pFile == NULL )
	{
		return	errFailed ;
	}
	SError	err = ReadTextFile( *pFile, encoding ) ;
	if ( !err )
	{
		SetFilePath( pwszFilePath ) ;
	}
	return	err ;
}

SError SStringParser::ReadTextFile
	( SFileInterface& file, Charset::EncodingType encoding )
{
	SByteBuffer	buf ;
	buf.ReadFromTextStream( file ) ;
	//
	if ( encoding == Charset::encodingUnknown )
	{
		encoding = Charset::AnalyzeEncoding
						( buf.GetConstArray(), (ssize_t) buf.GetLength() ) ;
		if ( encoding == Charset::encodingUnknown )
		{
			encoding = Charset::encodingUTF8 ;
		}
	}
	//
	Charset::Decode
		( *this, encoding, buf.GetConstArray(), (ssize_t) buf.GetLength() ) ;
	AttachString() ;
	//
	return	errSuccess ;
}

// ファイル名関連付け
//////////////////////////////////////////////////////////////////////////////
void SStringParser::SetFilePath( const wchar_t * pwszFilePath )
{
	if ( m_pwszFilePath != NULL )
	{
		esl_free( m_pwszFilePath ) ;
		m_pwszFilePath = NULL ;
	}
	if ( pwszFilePath != NULL )
	{
		size_t	nLength = SString::GetLength( pwszFilePath ) ;
		m_pwszFilePath =
			(wchar_t*) esl_malloc( (nLength + 1) * sizeof(wchar_t) ) ;
		eslMoveMemory
			( m_pwszFilePath,
				pwszFilePath, (nLength + 1) * sizeof(wchar_t) ) ;
	}
}

// 特定文字列を発見するまで指標を移動する（失敗時は指標は変化しない）
//////////////////////////////////////////////////////////////////////////////
bool SStringParser::SeekString( const wchar_t * pszString )
{
	if ( (pszString == NULL) || (pszString[0] == 0) )
	{
		return	false ;
	}
	const uint16_t *	pszText = m_pszText ;
	const size_t		lenText = m_lenText ;
	size_t				index = m_index ;
	while ( index < lenText )
	{
		int	i = 0 ;
		for ( ; ; )
		{
			wchar_t	wch = pszString[i] ;
			if ( wch == 0 )
			{
				break ;
			}
			if ( wch != pszText[index + i] )
			{
				break ;
			}
			++ i ;
		}
		if ( pszString[i] == 0 )
		{
			m_index = index ;
			return	true ;
		}
		++ index ;
	}
	return	false ;
}

// 特定文字を発見するまで指標を移動する（失敗時は指標は変化しない）
//////////////////////////////////////////////////////////////////////////////
bool SStringParser::SeekAnyCharacters( const wchar_t * pszChars )
{
	if ( (pszChars == NULL) || (pszChars[0] == 0) )
	{
		return	false ;
	}
	const uint16_t *	pszText = m_pszText ;
	const size_t		lenText = m_lenText ;
	size_t				index = m_index ;
	while ( index < lenText )
	{
		uint16_t	c = pszText[index] ;
		//
		int	i = 0 ;
		while ( pszChars[i] != 0 )
		{
			if ( c == pszChars[i] )
			{
				break ;
			}
			i ++ ;
		}
		if ( pszChars[i] != 0 )
		{
			m_index = index ;
			return	true ;
		}
		++ index ;
	}
	return	false ;
}

// 指標を記憶する
//////////////////////////////////////////////////////////////////////////////
void SStringParser::MarkIndex( void )
{
	m_mark = m_index ;
}

// 指標を記憶位置に戻す
//////////////////////////////////////////////////////////////////////////////
void SStringParser::SeekToMark( void )
{
	m_index = m_mark ;
}

// 指標の記憶を解除する
//////////////////////////////////////////////////////////////////////////////
void SStringParser::ReleaseMark( void )
{
	m_mark = m_index ;
}

// 文字列を切り出す
//////////////////////////////////////////////////////////////////////////////
SString SStringParser::SubString( size_t iStart, ssize_t nCount ) const
{
	if ( iStart >= m_lenText )
	{
		return	(const uint16_t *) NULL ;
	}
	if ( (nCount < 0)
		|| (iStart + nCount > m_lenText) )
	{
		nCount = (ssize_t) (m_lenText - iStart) ;
	}
	return	SString( m_pszText + iStart, nCount ) ;
}

// 記憶指標から現在の指標までの文字列を切り出す
//////////////////////////////////////////////////////////////////////////////
SString SStringParser::SubStringFromMark( void )
{
	size_t	iStart = m_mark ;
	ReleaseMark() ;
	return	SubStringFrom( iStart ) ;
}

// 指定指標から現在の指標までの文字列を切り出す
//////////////////////////////////////////////////////////////////////////////
SString SStringParser::SubStringFrom( size_t iStart ) const
{
	if ( (iStart >= m_lenText) | (iStart >= m_index) )
	{
		return	(const uint16_t *) NULL ;
	}
	return	SString( m_pszText + iStart, (ssize_t) (m_index - iStart) ) ;
}

// 行番号取得
//////////////////////////////////////////////////////////////////////////////
size_t SStringParser::GetLineNumberOf
	( size_t indexLimit, size_t * pGetLineIndex ) const
{
	const uint16_t *	pszText = m_pszText ;
	if ( m_lenText < indexLimit )
	{
		indexLimit = m_lenText ;
	}
	size_t	index = 0 ;
	size_t	indexLine = 0 ;
	size_t	numLine = 1 ;
	//
	while ( index < indexLimit )
	{
		uint16_t	c = pszText[index ++] ;
		if ( (c == L'\r') | (c == L'\n') )
		{
			if ( (c == L'\r')
				&& (index < indexLimit) && (pszText[index] == L'\n') )
			{
				index ++ ;
			}
			numLine ++ ;
			indexLine = index ;
		}
	}
	if ( pGetLineIndex != NULL )
	{
		*pGetLineIndex = indexLine ;
	}
	return	numLine ;
}

// 行情報取得
//  iCharIndex: in 文字指標 (0～), out 行カラム (1～)
//  strLine: out 行文字列（終端改行含む）
//  返り値: 行番号 (1～)
//////////////////////////////////////////////////////////////////////////////
size_t SStringParser::GetLineCharIndexOf
		( SString& strLine, size_t& iCharIndex ) const
{
	size_t	iLineFirst ;
	size_t	nLine = GetLineNumberOf( iCharIndex, &iLineFirst ) ;
	//
	SStringParser	spars ;
	spars.AttachString( *this ) ;
	spars.SeekIndex( iLineFirst ) ;
	spars.NextLine( strLine ) ;
	//
	iCharIndex = iCharIndex - iLineFirst + 1 ;
	return	nLine ;
}

// 平仮名→片仮名（該当しない場合には元の文字を返す）
//////////////////////////////////////////////////////////////////////////////
wchar_t SStringParser::HiraganaToKatakana( wchar_t wch )
{
	if ( IsHiragana( wch ) )
	{
		return	wch + (0x30A1 - 0x3041) ;
	}
	return	wch ;
}

// 片仮名→平仮名（該当しない場合には元の文字を返す）
//////////////////////////////////////////////////////////////////////////////
wchar_t SStringParser::KatakanaToHiragana( wchar_t wch )
{
	if ( (wch >= 0x30A1) && (wch <= 0x30F6) )
	{
		return	wch - (0x30A1 - 0x3041) ;
	}
	return	wch ;
}

// 濁音判定（濁音の場合清音仮名、それ以外の場合 0）
//////////////////////////////////////////////////////////////////////////////
wchar_t SStringParser::ClearVoicedKana( wchar_t wch )
{
	wchar_t	wchOffset = 0 ;
	if ( IsHiragana( wch ) )
	{
		wchOffset = (0x30A1 - 0x3041) ;
		wch += wchOffset ;
	}
	if ( IsKatakana( wch ) )
	{
		for ( size_t i = 0; s_pwszVoicedKana[i] != 0; i ++ )
		{
			if ( s_pwszVoicedKana[i] == wch )
			{
				return	s_pwszVoicelessKana[i] - wchOffset ;
			}
		}
	}
	return	0 ;
}

// 半濁音判定（半濁音の場合清音仮名、それ以外の場合 0）
//////////////////////////////////////////////////////////////////////////////
wchar_t SStringParser::ClearSemivoicedKana( wchar_t wch )
{
	wchar_t	wchOffset = 0 ;
	if ( IsHiragana( wch ) )
	{
		wchOffset = (0x30A1 - 0x3041) ;
		wch += wchOffset ;
	}
	if ( IsKatakana( wch ) )
	{
		for ( size_t i = 0; s_pwszSemivoicedKana[i] != 0; i ++ )
		{
			if ( s_pwszSemivoicedKana[i] == wch )
			{
				return	s_pwszVoicelessKana[i] - wchOffset ;
			}
		}
	}
	return	0 ;
}

// 濁音仮名生成（合成不可能の場合には 0）
//////////////////////////////////////////////////////////////////////////////
wchar_t SStringParser::MakeVoicedKana( wchar_t wch )
{
	wchar_t	wchOffset = 0 ;
	if ( IsHiragana( wch ) )
	{
		wchOffset = (0x30A1 - 0x3041) ;
		wch += wchOffset ;
	}
	for ( size_t i = 0; s_pwszVoicedKana[i] != 0; i ++ )
	{
		if ( s_pwszVoicelessKana[i] == wch )
		{
			return	s_pwszVoicedKana[i] - wchOffset ;
		}
	}
	return	0 ;
}

// 半濁音仮名生成（合成不可能の場合には 0）
//////////////////////////////////////////////////////////////////////////////
wchar_t SStringParser::MakeSemivoicedKana( wchar_t wch )
{
	wchar_t	wchOffset = 0 ;
	if ( IsHiragana( wch ) )
	{
		wchOffset = (0x30A1 - 0x3041) ;
		wch += wchOffset ;
	}
	for ( size_t i = 0; s_pwszSemivoicedKana[i] != 0; i ++ )
	{
		if ( s_pwszVoicelessKana[i] == wch )
		{
			return	s_pwszSemivoicedKana[i] - wchOffset ;
		}
	}
	return	0 ;
}

// 全角文字（仮名は清音のみ）→半角文字（対応文字がないときは 0）
//////////////////////////////////////////////////////////////////////////////
wchar_t SStringParser::ToHalfWidthChar( wchar_t wch )
{
	if ( (wch >= 0xFF01) && (wch <= 0xFF5E) )
	{
		return	wch - (0xFF01 - 0x21) ;
	}
	if ( wch == L'　' )
	{
		return	L' ' ;
	}
	if ( (wch >= 0x3001) && (wch <= 0x30FA) )
	{
		for ( size_t i = 0; s_wchWideKana[i] != 0; i ++ )
		{
			if ( s_wchWideKana[i] == wch )
			{
				return	(wchar_t) (i + 0xFF61) ;
			}
		}
	}
	return	0 ;
}

// 半角文字→全角文字（対応文字がないときは 0）
//////////////////////////////////////////////////////////////////////////////
wchar_t SStringParser::WideCharFromHalfWidth( wchar_t wch )
{
	if ( (wch >= 0x21) && (wch <= 0x7E) )
	{
		return	wch + (0xFF01 - 0x21) ;
	}
	if ( wch == L' ' )
	{
		return	L'　' ;
	}
	if ( IsHalfWidthKana( wch ) )
	{
		ESLAssert( wch >= 0xFF61 ) ;
		ESLAssert( wch - 0xFF61 < 0x40 ) ;
		return	s_wchWideKana[wch - 0xFF61] ;
	}
	return	0 ;
}

// 小文字仮名判定（ぁぃぅぇぉゃゅょ...）
//////////////////////////////////////////////////////////////////////////////
bool SStringParser::IsSmallKana( wchar_t wch )
{
	for ( size_t i = 0; s_pwszKomojiKana[i] != 0; i ++ )
	{
		if ( s_pwszKomojiKana[i] == wch )
		{
			return	true ;
		}
	}
	return	false ;
}

// 読み補助文字判定（長音・濁音・反復等）
//////////////////////////////////////////////////////////////////////////////
bool SStringParser::IsAuxiliaryKana( wchar_t wch )
{
	for ( size_t i = 0; s_pwszAuxiliaryKana[i] != 0; i ++ )
	{
		if ( s_pwszAuxiliaryKana[i] == wch )
		{
			return	true ;
		}
	}
	return	false ;
}

// 句読点判定（ASCII ,.:; 及び日本語句読点等）
//////////////////////////////////////////////////////////////////////////////
bool SStringParser::IsJISXPunctuation( wchar_t wch )
{
	for ( size_t i = 0; s_pwszPunctuation[i] != 0; i ++ )
	{
		if ( s_pwszPunctuation[i] == wch )
		{
			return	true ;
		}
	}
	return	false ;
}

// 括弧判定（ASCII "'()<>[]{} 及び日本語各種括弧）
//////////////////////////////////////////////////////////////////////////////
bool SStringParser::IsJISXParenthesis( wchar_t wch )
{
	for ( size_t i = 0; s_pwszParenthesis[i] != 0; i ++ )
	{
		if ( s_pwszParenthesis[i] == wch )
		{
			return	true ;
		}
	}
	return	false ;
}

// 空白をスキップ（空白以外を発見したら true）
//////////////////////////////////////////////////////////////////////////////
bool SStringParser::PassSpace( void )
{
	const uint16_t *	pszText = m_pszText ;
	const size_t		lenText = m_lenText ;
	size_t				index = m_index ;
	//
	while ( index < lenText )
	{
		if ( !IsCharacterSpace( pszText[index] ) )
		{
			m_index = index ;
			return	true ;
		}
		++ index ;
	}
	m_index = index ;
	return	false ;
}

// 次の行へ移動
//////////////////////////////////////////////////////////////////////////////
uint32_t SStringParser::SeekToNextLine( void )
{
	const uint16_t *	pszText = m_pszText ;
	const size_t		lenText = m_lenText ;
	size_t				index = m_index ;
	uint32_t			codeRet = 0 ;
	while ( index < lenText )
	{
		uint16_t	c = pszText[index ++] ;
		if ( (c == L'\r') | (c == L'\n') )
		{
			codeRet = c ;
			if ( (c == L'\r')
				&& (index < lenText) && (pszText[index] == L'\n') )
			{
				index ++ ;
				codeRet = (L'\r' << 16) | L'\n' ;
			}
			break ;
		}
	}
	m_index = index ;
	return	codeRet ;
}

// 現在の文字列（空白で区切られた）を通過する
//////////////////////////////////////////////////////////////////////////////
void SStringParser::PassString( void )
{
	const uint16_t *	pszText = m_pszText ;
	const size_t		lenText = m_lenText ;
	size_t				index = m_index ;
	while ( index < lenText )
	{
		if ( IsCharacterSpace( pszText[index] ) )
		{
			break ;
		}
		++ index ;
	}
	m_index = index ;
}

// 特定の文字まで通過する
//////////////////////////////////////////////////////////////////////////////
void SStringParser::PassEnclosedString( wchar_t wchCloser, int nCtrlFlags )
{
	const uint16_t *	pszText = m_pszText ;
	const size_t		lenText = m_lenText ;
	size_t				index = m_index ;
	//
	if ( (wchCloser == L'\"') & (nCtrlFlags & ctrlEscCSVInDQuote) )
	{
		while ( index < lenText )
		{
			uint16_t	c = pszText[index] ;
			if ( c == wchCloser )
			{
				if ( pszText[index + 1] == L'\"' )
				{
					++ index ;
				}
				else
				{
					break ;
				}
			}
			else if ( !(nCtrlFlags & ctrlNoEscInDQuote) )
			{
				if ( c == L'\\' )
				{
					++ index ;
				}
			}
			++ index ;
		}
	}
	else if ( ((wchCloser == L'\'') & !(nCtrlFlags & ctrlNoEscInQuote))
			| ((wchCloser == L'\"') & !(nCtrlFlags & ctrlNoEscInDQuote)) )
	{
		while ( index < lenText )
		{
			uint16_t	c = pszText[index] ;
			if ( c == wchCloser )
			{
				break ;
			}
			if ( c == L'\\' )
			{
				++ index ;
			}
			++ index ;
		}
	}
	else
	{
		while ( index < lenText )
		{
			if ( pszText[index] == wchCloser )
			{
				break ;
			}
			++ index ;
		}
	}
	m_index = index ;
}

// 現在のトークンを通過する
//////////////////////////////////////////////////////////////////////////////
SStringParser::TokenType SStringParser::PassToken( void )
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
	if ( IsCharacterSpace( wch ) )
	{
		typeToken = tokenInvalid ;
	}
	else if ( IsSpecialMark( wch ) )
	{
		typeToken = tokenSpecialMark ;
	}
	else if ( IsPunctuation( wch ) )
	{
		typeToken = tokenPunctuation ;
		while ( index < lenText )
		{
			wch = pszText[index] ;
			if( IsCharacterSpace( wch )
				|| IsSpecialMark( wch )
				|| !IsPunctuation( wch ) )
			{
				break ;
			}
			++ index ;
		}
	}
	else
	{
		typeToken = tokenNormal ;
		while ( index < lenText )
		{
			if( IsPunctuation( pszText[index] ) )
			{
				break ;
			}
			++ index ;
		}
	}
	m_index = index ;
	return	typeToken ;
}

// 式の１項を通過する
//////////////////////////////////////////////////////////////////////////////
void SStringParser::PassExpressionTerm( int nCtrlFlags )
{
	if ( !PassSpace() )
	{
		return ;
	}
	wchar_t	wch = CurrentCharacter() ;
	if ( wch == L'\'' )
	{
		++ m_index ;
		PassEnclosedString( L'\'', nCtrlFlags ) ;
		HasToComeChar( L"\'" ) ;
	}
	else if ( wch == L'\"' )
	{
		++ m_index ;
		PassEnclosedString( L'\"', nCtrlFlags ) ;
		HasToComeChar( L"\"" ) ;
	}
	else if ( wch == L'(' )
	{
		++ m_index ;
		PassExpression( L")", nCtrlFlags ) ;
		HasToComeChar( L")" ) ;
	}
	else if ( wch == L'[' )
	{
		++ m_index ;
		PassExpression( L"]", nCtrlFlags ) ;
		HasToComeChar( L"]" ) ;
	}
	else if ( wch == L'{' )
	{
		++ m_index ;
		PassExpression( L"}", nCtrlFlags ) ;
		HasToComeChar( L"}" ) ;
	}
	else
	{
		PassToken() ;
	}
}

// 式を通過する
//////////////////////////////////////////////////////////////////////////////
void SStringParser::PassExpression
	( const wchar_t * pszCloses, int nCtrlFlags )
{
	while ( PassSpace() )
	{
		wchar_t	wch = CurrentCharacter() ;
		int	i = 0 ;
		for ( ; ; )
		{
			wchar_t	wchClose = pszCloses[i ++] ;
			if ( wchClose == wch )
			{
				return ;
			}
			if ( wchClose == 0 )
			{
				break ;
			}
		}
		PassExpressionTerm( nCtrlFlags ) ;
	}
}

// 現在の行（残り）取得
//////////////////////////////////////////////////////////////////////////////
uint32_t SStringParser::NextLine( SString& strLine )
{
	const size_t	iCurrent = m_index ;
	uint32_t		codeRet = SeekToNextLine() ;
	//
	strLine.SetString( m_pszText + iCurrent, (ssize_t) (m_index - iCurrent) ) ;
	//
	return	codeRet ;
}

SString SStringParser::GetLine( uint32_t* pRetCode )
{
	const size_t	iCurrent = m_index ;
	uint32_t		codeRet = SeekToNextLine() ;
	if ( pRetCode != NULL )
	{
		*pRetCode = codeRet ;
	}
	return	SString( m_pszText + iCurrent, (ssize_t) (m_index - iCurrent) ) ;
}

// 次の文字列（空白で区切られた）を取得
//////////////////////////////////////////////////////////////////////////////
bool SStringParser::NextString( SString& strString )
{
	if ( PassSpace() )
	{
		const size_t	iCurrent = m_index ;
		PassString() ;
		strString.SetString( m_pszText + iCurrent, (ssize_t) (m_index - iCurrent) ) ;
		return	true ;
	}
	strString.FreeArray() ;
	return	false ;
}

SString SStringParser::GetString( void )
{
	if ( !PassSpace() )
	{
		return	SString() ;
	}
	const size_t	iCurrent = m_index ;
	PassString() ;
	return	SString( m_pszText + iCurrent, (ssize_t) (m_index - iCurrent) ) ;
}

// 次の文字列（特定の文字で区切られた）を取得
//////////////////////////////////////////////////////////////////////////////
wchar_t SStringParser::NextEnclosedString
	( SString& strString, wchar_t wchCloser, int nCtrlFlags )
{
	const size_t	iCurrent = m_index ;
	PassEnclosedString( wchCloser, nCtrlFlags ) ;
	strString.SetString( m_pszText + iCurrent, (ssize_t) (m_index - iCurrent) ) ;
	return	HasToComeChar( &wchCloser ) ;
}

SString SStringParser::GetEnclosedString
	( wchar_t wchCloser, int nCtrlFlags, wchar_t * pGetClosed )
{
	const size_t	iCurrent = m_index ;
	PassEnclosedString( wchCloser, nCtrlFlags ) ;
	const size_t	iEnd = m_index ;
	wchar_t			wchClosed = 0 ;
	if ( CurrentCharacter() == wchCloser )
	{
		wchClosed = GetCharacter() ;
	}
	if ( pGetClosed != NULL )
	{
		*pGetClosed = wchClosed ;
	}
	return	SString( m_pszText + iCurrent, (ssize_t) (iEnd - iCurrent) ) ;
}

// 次のトークンを取得
//////////////////////////////////////////////////////////////////////////////
SStringParser::TokenType SStringParser::NextToken( SString& strToken )
{
	if ( !PassSpace() )
	{
		strToken.FreeArray() ;
		return	tokenInvalid ;
	}
	const size_t	iCurrent = m_index ;
	TokenType		typeToken = PassToken() ;
	strToken.SetString( m_pszText + iCurrent, (ssize_t) (m_index - iCurrent) ) ;
	return	typeToken ;
}

SString SStringParser::GetToken( SStringParser::TokenType* pGetType )
{
	if ( !PassSpace() )
	{
		if ( pGetType != NULL )
		{
			*pGetType = tokenInvalid ;
		}
		return	SString() ;
	}
	const size_t	iCurrent = m_index ;
	TokenType		typeToken = PassToken() ;
	if ( pGetType != NULL )
	{
		*pGetType = typeToken ;
	}
	return	SString( m_pszText + iCurrent, (ssize_t) (m_index - iCurrent) ) ;
}

// 次の文字列項（空白で区切られた｜引用符で囲まれた）を取得
//////////////////////////////////////////////////////////////////////////////
bool SStringParser::NextStringTerm( SString& strTerm, int nCtrlFlags )
{
	wchar_t	wchQuote = HasToComeChar( L"\"\'" ) ;
	if ( wchQuote == 0 )
	{
		return	NextString( strTerm ) ;
	}
	else
	{
		NextEnclosedString( strTerm, wchQuote, nCtrlFlags ) ;
		return	true ;
	}
}

SString SStringParser::GetStringTerm( int nCtrlFlags )
{
	wchar_t	wchQuote = HasToComeChar( L"\"\'" ) ;
	if ( wchQuote == 0 )
	{
		return	GetString() ;
	}
	else
	{
		return	GetEnclosedString( wchQuote, nCtrlFlags ) ;
	}
}

// 式の１項を取得
//////////////////////////////////////////////////////////////////////////////
bool SStringParser::NextExpressionTerm( SString& strTerm, int nCtrlFlags )
{
	if ( !PassSpace() )
	{
		strTerm.FreeArray() ;
		return	false ;
	}
	const size_t	iCurrent = m_index ;
	PassExpressionTerm( nCtrlFlags ) ;
	strTerm.SetString( m_pszText + iCurrent, (ssize_t) (m_index - iCurrent) ) ;
	return	true ;
}

SString SStringParser::GetExpressionTerm( int nCtrlFlags )
{
	if ( !PassSpace() )
	{
		return	SString() ;
	}
	const size_t	iCurrent = m_index ;
	PassExpressionTerm( nCtrlFlags ) ;
	return	SString( m_pszText + iCurrent, (ssize_t) (m_index - iCurrent) ) ;
}

// 式を取得
//////////////////////////////////////////////////////////////////////////////
wchar_t SStringParser::NextExpression
	( SString& strExpr, const wchar_t * pszCloses, int nCtrlFlags )
{
	if ( !PassSpace() )
	{
		strExpr.FreeArray() ;
		return	0 ;
	}
	const size_t	iCurrent = m_index ;
	PassExpression( pszCloses, nCtrlFlags ) ;
	strExpr.SetString( m_pszText + iCurrent, (ssize_t) (m_index - iCurrent) ) ;
	return	HasToComeChar( pszCloses ) ;
}

SString SStringParser::GetExpression
	( const wchar_t * pszCloses, int nCtrlFlags, wchar_t* pGetClosed )
{
	if ( !PassSpace() )
	{
		if ( pGetClosed != NULL )
		{
			*pGetClosed = 0 ;
		}
		return	SString() ;
	}
	const size_t	iCurrent = m_index ;
	PassExpression( pszCloses, nCtrlFlags ) ;
	const size_t	iEnd = m_index ;
	wchar_t			wchClosed = HasToComeChar( pszCloses ) ;
	if ( pGetClosed != NULL )
	{
		*pGetClosed = wchClosed ;
	}
	return	SString( m_pszText + iCurrent, (ssize_t) (iEnd - iCurrent) ) ;
}

// 次に一致する文字を通過する
//////////////////////////////////////////////////////////////////////////////
wchar_t SStringParser::HasToComeChar( const wchar_t * pszNext )
{
	if ( !PassSpace() )
	{
		return	0 ;
	}
	wchar_t	wch = CurrentCharacter() ;
	int	i = 0 ;
	for ( ; ; )
	{
		wchar_t	wchNext = pszNext[i ++] ;
		if ( wchNext == wch )
		{
			++ m_index ;
			return	wch ;
		}
		if ( wchNext == 0 )
		{
			break ;
		}
	}
	return	0 ;
}

// 次に一致する文字列を通過する
//////////////////////////////////////////////////////////////////////////////
bool SStringParser::HasToComeString( const wchar_t * pszNext )
{
	if ( !PassSpace() )
	{
		return	false ;
	}
	const uint16_t *	pszText = m_pszText ;
	const size_t		lenText = m_lenText ;
	size_t				index = m_index ;
	while ( index < lenText )
	{
		wchar_t	wchStr = *(pszNext ++) ;
		if ( wchStr == 0 )
		{
			m_index = index ;
			return	true ;
		}
		if ( pszText[index ++] != wchStr )
		{
			break ;
		}
	}
	if ( (index >= lenText) && (*pszNext == 0) )
	{
		m_index = index ;
		return	true ;
	}
	return	false ;
}

bool SStringParser::HasToComeNoCaseString( const wchar_t * pszNext )
{
	if ( !PassSpace() )
	{
		return	false ;
	}
	const uint16_t *	pszText = m_pszText ;
	const size_t		lenText = m_lenText ;
	size_t				index = m_index ;
	while ( index < lenText )
	{
		wchar_t	wchStr = *(pszNext ++) ;
		if ( wchStr == 0 )
		{
			m_index = index ;
			return	true ;
		}
		wchar_t	wchText = pszText[index ++] ;
		if ( (wchStr >= L'a') & (wchStr <= L'z') )
		{
			wchStr -= L'a' - L'A' ;
		}
		if ( (wchText >= L'a') & (wchText <= L'z') )
		{
			wchText -= L'a' - L'A' ;
		}
		if ( wchText != wchStr )
		{
			return	false ;
		}
	}
	if ( *pszNext == 0 )
	{
		m_index = index ;
		return	true ;
	}
	return	false ;
}

// 次に一致するトークンを通過する
//////////////////////////////////////////////////////////////////////////////
bool SStringParser::HasToComeToken( const wchar_t * pszNext )
{
	const size_t	iCurrent = m_index ;
	if ( GetToken() == pszNext )
	{
		return	true ;
	}
	m_index = iCurrent ;
	return	false ;
}

// 数値文字列判定
//////////////////////////////////////////////////////////////////////////////
int SStringParser::IsNextNumber( int nCtrlFlags )
{
	const size_t	iCurrent = m_index ;
	int				typeNum = numberInvalid ;
	do
	{
		if ( !PassSpace() )
		{
			break ;
		}
		wchar_t	wch = GetCharacter() ;
		if ( (wch == L'+') | (wch == L'-') )
		{
			if ( !PassSpace() )
			{
				break ;
			}
			wch = GetCharacter() ;
		}
		if ( (wch < L'0') | (wch > L'9') )
		{
			break ;
		}
		int	numMax = wch - L'0' ;
		typeNum = numberInteger ;
		if ( (wch == L'0') && (nCtrlFlags & ctrlCStyleNumber) )
		{
			wch = CurrentCharacter() ;
			if ( (wch == L'X') | (wch == L'x') )
			{
				typeNum = numberStyleHex ;
				break ;
			}
			typeNum = numberStyleOct ;
		}
		wchar_t	wchLast = 0 ;
		bool	fDecimal = false ;
		while ( m_index < m_lenText )
		{
			int	num = 0 ;
			wch = GetCharacter() ;
			if ( ((wch == L'E') | (wch == L'e'))
				&& (nCtrlFlags & ctrlCStyleNumber)
				&& (typeNum != numberStyleHex) )
			{
				fDecimal = true ;
				break ;
			}
			else if ( (L'A' <= wch) & (wch <= L'F') )
			{
				num = wch - L'A' + 10 ;
			}
			else if ( (L'a' <= wch) & (wch <= L'f') )
			{
				num = wch - L'a' + 10 ;
			}
			else if ( (L'0' <= wch) & (wch <= L'9') )
			{
				num = wch - L'0' ;
			}
			else if ( wch == L'.' )
			{
				if ( fDecimal )
				{
					break ;
				}
				fDecimal = true ;
			}
			else
			{
				if ( (wch == L'B') | (wch == L'b')
					| (wch == L'O') | (wch == L'o')
					| (wch == L'T') | (wch == L't')
					| (wch == L'H') | (wch == L'h') )
				{
					wchLast = wch ;
				}
				break ;
			}
			wchLast = wch ;
			if ( numMax < num )
			{
				numMax = num ;
			}
		}
		if ( nCtrlFlags & ctrlCStyleNumber )
		{
			if ( fDecimal )
			{
				typeNum = numberFlagReal ;
				break ;
			}
			if ( (typeNum == numberStyleOct) && (numMax >= 8) )
			{
				typeNum = numberInteger ;
			}
		}
		if ( !(nCtrlFlags & ctrlNoRadixPostfix) )
		{
			if ( (wchLast == L'H') | (wchLast == L'h') )
			{
				typeNum = numberRadix16 ;
			}
			else if ( (wchLast == L'T') | (wchLast == L't') )
			{
				typeNum = numberRadix10 ;
			}
			else if ( (wchLast == L'O') | (wchLast == L'o') )
			{
				typeNum = numberRadix8 ;
			}
			else if ( (wchLast == L'B') | (wchLast == L'b') )
			{
				typeNum = numberRadix2 ;
			}
		}
		if ( fDecimal )
		{
			typeNum |= numberFlagReal ;
		}
	}
	while ( false ) ;
	m_index = iCurrent ;
	return	typeNum ;
}

// 整数解釈
//////////////////////////////////////////////////////////////////////////////
int64_t SStringParser::NextInteger( int type )
{
	uint32_t	numRadix = 10 ;
	if ( type & numberRadixMask )
	{
		numRadix = (uint32_t) (type & numberRadixMask) ;
	}
	int64_t	numInt = 0 ;
	int64_t	numSign = 0 ;
	do
	{
		if ( !PassSpace() )
		{
			break ;
		}
		wchar_t	wch = GetCharacter() ;
		if ( (wch == L'+') | (wch == L'-') )
		{
			if ( !PassSpace() )
			{
				break ;
			}
			numSign = (wch == L'-') ? -1 : 0 ;
			wch = GetCharacter() ;
		}
		if ( (wch < L'0') | (wch > L'9') )
		{
			if ( (numRadix != 16)
				|| !((wch >= L'A') & (wch <= L'F')
					|| (wch >= L'a') & (wch <= L'f')) )
			{
				break ;
			}
		}
		if ( (L'0' <= wch) & (wch <= L'9') )
		{
			numInt = wch - L'0' ;
		}
		else if ( (L'A' <= wch) & (wch <= L'F') )
		{
			numInt = wch - L'A' + 10 ;
		}
		else if ( (L'a' <= wch) & (wch <= L'f') )
		{
			numInt = wch - L'a' + 10 ;
		}
		if ( (wch == L'0') & (type == numberStyleHex) )
		{
			wch = CurrentCharacter() ;
			if ( (wch == L'X') | (wch == L'x') )
			{
				++ m_index ;
			}
		}
		while ( m_index < m_lenText )
		{
			wch = CurrentCharacter() ;
			if ( (L'0' <= wch) & (wch <= L'9') )
			{
				if ( (uint32_t) (wch - L'0') >= numRadix )
				{
					break ;
				}
				numInt = numInt * numRadix + (wch - L'0') ;
			}
			else if ( (numRadix == 16) & (L'A' <= wch) & (wch <= L'F') )
			{
				numInt = numInt * numRadix + (wch - L'A' + 10) ;
			}
			else if ( (numRadix == 16) & (L'a' <= wch) & (wch <= L'f') )
			{
				numInt = numInt * numRadix + (wch - L'a' + 10) ;
			}
			else
			{
				break ;
			}
			++ m_index ;
		}
		wch = CurrentCharacter() ;
		if ( (type == numberRadix2)
				& ((wch == L'B') | (wch == L'b')) )
		{
			++ m_index ;
		}
		else if ( (type == numberRadix8)
				& ((wch == L'O') | (wch == L'o')) )
		{
			++ m_index ;
		}
		else if ( (type == numberRadix10)
				& ((wch == L'T') | (wch == L't')) )
		{
			++ m_index ;
		}
		else if ( (type == numberRadix16)
				& ((wch == L'H') | (wch == L'h')) )
		{
			++ m_index ;
		}
	}
	while ( false ) ;
	return	(numInt + numSign) ^ numSign ;
}

int64_t SStringParser::GetNextInteger( int& type, int nCtrlFlags )
{
	type = IsNextNumber( nCtrlFlags ) ;
	if ( type == numberInvalid )
	{
		return	0 ;
	}
	return	NextInteger( type ) ;
}

// 実数解釈
//////////////////////////////////////////////////////////////////////////////
double SStringParser::NextRealNumber( int type )
{
	double	numRadix = 10.0 ;
	if ( type & numberRadixMask )
	{
		numRadix = (type & numberRadixMask) ;
	}
	double	numReal = 0.0 ;
	double	numSign = 1.0 ;
	do
	{
		if ( !PassSpace() )
		{
			break ;
		}
		wchar_t	wch = GetCharacter() ;
		if ( (wch == L'+') | (wch == L'-') )
		{
			if ( !PassSpace() )
			{
				break ;
			}
			numSign = (wch == L'-') ? -1.0 : 1.0 ;
			wch = GetCharacter() ;
		}
		if ( (wch < L'0') | (wch > L'9') )
		{
			break ;
		}
		numReal = wch - L'0' ;
		if ( (wch == L'0') & (type == numberStyleHex) )
		{
			wch = CurrentCharacter() ;
			if ( (wch == L'X') | (wch == L'x') )
			{
				++ m_index ;
			}
		}
		bool	fDecimal = false ;
		double	numDecimal = 1.0 ;
		while ( m_index < m_lenText )
		{
			int	numCur = 0 ;
			wch = CurrentCharacter() ;
			if ( (L'0' <= wch) & (wch <= L'9') )
			{
				if ( wch - L'0' >= numRadix )
				{
					break ;
				}
				numCur = (wch - L'0') ;
			}
			else if ( (numRadix == 16) & (L'A' <= wch) & (wch <= L'F') )
			{
				numCur = (wch - L'A' + 10) ;
			}
			else if ( (numRadix == 16) & (L'a' <= wch) & (wch <= L'f') )
			{
				numCur = (wch - L'a' + 10) ;
			}
			else if ( !fDecimal & (wch == L'.') )
			{
				fDecimal = true ;
				++ m_index ;
				continue ;
			}
			else
			{
				break ;
			}
			if ( fDecimal )
			{
				numDecimal /= numRadix ;
				numReal += numDecimal * numCur ;
			}
			else
			{
				numReal = numReal * numRadix + numCur ;
			}
			++ m_index ;
		}
		wch = CurrentCharacter() ;
		if ( (wch == L'E') | (wch == L'e') )
		{
			int	numExp = 0 ;
			int	signExp = 0 ;
			++ m_index ;
			wch = CurrentCharacter() ;
			if ( wch == L'-' )
			{
				signExp = -1 ;
				++ m_index ;
			}
			else if ( wch == L'+' )
			{
				++ m_index ;
			}
			while ( m_index < m_lenText )
			{
				wch = CurrentCharacter() ;
				if ( (L'0' <= wch) & (wch <= L'9') )
				{
					numExp = numExp * 10 + (wch - L'0') ;
				}
				else
				{
					break ;
				}
				++ m_index ;
			}
			numExp = (numExp + signExp) ^ signExp ;
			numReal *= pow( 10.0, numExp ) ;
		}
		type &= ~numberFlagReal ;
		if ( (type == numberRadix2)
				& ((wch == L'B') | (wch == L'b')) )
		{
			++ m_index ;
		}
		else if ( (type == numberRadix8)
				& ((wch == L'O') | (wch == L'o')) )
		{
			++ m_index ;
		}
		else if ( (type == numberRadix10)
				& ((wch == L'T') | (wch == L't')) )
		{
			++ m_index ;
		}
		else if ( (type == numberRadix16)
				& ((wch == L'H') | (wch == L'h')) )
		{
			++ m_index ;
		}
	}
	while ( false ) ;
	return	numReal * numSign ;
}

double SStringParser::GetNextRealNumber( int& type, int nCtrlFlags )
{
	type = IsNextNumber( nCtrlFlags ) ;
	if ( type == numberInvalid )
	{
		return	0 ;
	}
	return	NextRealNumber( type ) ;
}

// 整数配列解釈
//////////////////////////////////////////////////////////////////////////////
size_t SStringParser::ParseIntegerArray
	( int64_t * pNumbers, size_t nCount,
		int nCtrlFlags, const wchar_t * pwszSeparator )
{
	for ( size_t i = 0; i < nCount; i ++ )
	{
		int	type = IsNextNumber( nCtrlFlags ) ;
		if ( type == numberInvalid )
		{
			return	i ;
		}
		pNumbers[i] = NextInteger( type ) ;
		//
		if ( (i + 1 < nCount) && (pwszSeparator != NULL) )
		{
			if ( HasToComeChar( pwszSeparator ) == 0 )
			{
				return	i ;
			}
		}
	}
	return	nCount ;
}

size_t SStringParser::ParseHexIntegerArray
	( int64_t * pNumbers, size_t nCount, const wchar_t * pwszSeparator )
{
	for ( size_t i = 0; i < nCount; i ++ )
	{
		PassSpace() ;
		//
		wchar_t	wch = CurrentCharacter() ;
		if ( ((wch >= L'0') && (wch <= L'9'))
			|| ((wch >= L'A') && (wch <= L'F'))
			|| ((wch >= L'a') && (wch <= L'f')) )
		{
			pNumbers[i] = NextInteger( numberRadix16 ) ;
		}
		else
		{
			return	i ;
		}
		if ( (i + 1 < nCount) && (pwszSeparator != NULL) )
		{
			if ( HasToComeChar( pwszSeparator ) == 0 )
			{
				return	i ;
			}
		}
	}
	return	nCount ;
}

// 実数配列解釈
//////////////////////////////////////////////////////////////////////////////
size_t SStringParser::ParseNumberArray
	( double * pNumbers, size_t nCount,
		int nCtrlFlags, const wchar_t * pwszSeparator )
{
	for ( size_t i = 0; i < nCount; i ++ )
	{
		int	type = IsNextNumber( nCtrlFlags ) ;
		if ( type == numberInvalid )
		{
			return	i ;
		}
		pNumbers[i] = NextRealNumber( type ) ;
		//
		if ( (i + 1 < nCount) && (pwszSeparator != NULL) )
		{
			if ( HasToComeChar( pwszSeparator ) == 0 )
			{
				return	i ;
			}
		}
	}
	return	nCount ;
}

// CSV ライン解釈（CSVダブルクオーテーションカラムのデコード処理）
//////////////////////////////////////////////////////////////////////////////
SError SStringParser::ParseCommaSeparatedValues
	( SObjectArray<SString>& aValues )
{
	SError	err = SplitValuesOneLine
						( aValues, ctrlEscCSVInDQuote, L',', L"\"" ) ;
	for ( size_t i = 0; i < aValues.GetLength(); i ++ )
	{
		SString *	pstrValue = aValues.GetAt( i ) ;
		if ( (pstrValue != NULL) && (pstrValue->GetAt(0) == L'\"') )
		{
			if ( pstrValue->GetLastAt(0) == L'\"' )
			{
				pstrValue->ChopRight( 1 ) ;
			}
			pstrValue->ChopLeft( 1 ) ;
			if ( DecodeCSVString( *pstrValue ) )
			{
				err = errFailed ;
			}
		}
	}
	return	err ;
}

SError SStringParser::ParseCommaSeparatedValues
	( SObjectArray<SString>& aValues, const SString& strLine )
{
	SStringParser	sparsLine ;
	sparsLine.AttachString( strLine ) ;
	return	sparsLine.ParseCommaSeparatedValues( aValues ) ;
}

SError SStringParser::ParseCommaSeparatedValues
	( SObjectArray<SString>& aValues, const wchar_t * pwszStrLine )
{
	SStringParser	sparsLine = pwszStrLine ;
	return	sparsLine.ParseCommaSeparatedValues( aValues ) ;
}

// 文字列配列解釈
//////////////////////////////////////////////////////////////////////////////
SError SStringParser::SplitValuesOneLine
	( SObjectArray<SString>& aValues, int nCtrlFlags,
		wchar_t wchSeparator, const wchar_t * pwszQuotes )
{
	SError	err = errSuccess ;
	//
	aValues.RemoveAll() ;
	//
	while ( m_index < m_lenText )
	{
		wchar_t	wch = CurrentCharacter() ;
		if ( (wch == L'\r') || (wch == L'\n') )
		{
			m_index ++ ;
			if ( (wch == L'\r')
				&& (m_index < m_lenText) && (m_pszText[m_index] == L'\n') )
			{
				m_index ++ ;
			}
			break ;
		}
		else if ( wch == wchSeparator )
		{
			m_index ++ ;
			aValues.Add( new SString() ) ;
		}
		else
		{
			bool	flagQuoted = false ;
			for ( size_t i = 0; pwszQuotes[i]; i ++ )
			{
				if ( pwszQuotes[i] == wch )
				{
					flagQuoted = true ;
					break ;
				}
			}
			size_t	iBeginColumnn = m_index ;
			if ( flagQuoted )
			{
				//
				// 引用符間文字列
				//
				m_index ++ ;
				PassEnclosedString( wch, nCtrlFlags ) ;
				if ( CurrentCharacter() == wch )
				{
					m_index ++ ;
				}
				else
				{
					err = errFailed ;
				}
				aValues.Add
					( new SString( SubStringFrom( iBeginColumnn ) ) ) ;
				//
				if ( CurrentCharacter() != wchSeparator )
				{
					err = errFailed ;
				}
			}
			else
			{
				//
				// 区切り記号間文字列
				//
				while ( m_index < m_lenText )
				{
					wchar_t	wch = CurrentCharacter() ;
					if ( (wch == wchSeparator)
						|| (wch == L'\r') || (wch == L'\n') )
					{
						break ;
					}
					m_index ++ ;
				}
				aValues.Add
					( new SString( SubStringFrom( iBeginColumnn ) ) ) ;
			}
			//
			// 区切り記号まで読み飛ばす
			//
			while ( m_index < m_lenText )
			{
				wchar_t	wch = CurrentCharacter() ;
				if ( wch == wchSeparator )
				{
					m_index ++ ;
					break ;
				}
				if ( (wch == L'\r') || (wch == L'\n') )
				{
					break ;
				}
				m_index ++ ;
			}
		}
	}
	return	err ;
}

// 16進数文字列のデコード
//////////////////////////////////////////////////////////////////////////////
SError SStringParser::DecodeHexString
	( SArray<uint8_t>& buf, const wchar_t * pwszHexText, ssize_t nHexLength )
{
	if ( nHexLength < 0 )
	{
		nHexLength = (ssize_t) SString::GetLength( pwszHexText ) ;
	}
	size_t	nBinLength = (size_t) nHexLength >> 1 ;
	buf.SetLength( nBinLength ) ;
	//
	uint8_t *	pbytBuf = buf.GetArray() ;
	for ( size_t i = 0, j = 0; i < nBinLength; i ++, j += 2 )
	{
		wchar_t	wch = pwszHexText[j] ;
		uint8_t	bin = 0 ;
		if ( (wch >= L'0') && (wch <= L'9') )
		{
			bin = (uint8_t) (wch - L'0') ;
		}
		else if ( (wch >= L'A') && (wch <= L'F') )
		{
			bin = (uint8_t) (wch - (L'A' - 10)) ;
		}
		else if ( (wch >= L'a') && (wch <= L'f') )
		{
			bin = (uint8_t) (wch - (L'a' - 10)) ;
		}
		bin <<= 4 ;
		//
		wch = pwszHexText[j + 1] ;
		if ( (wch >= L'0') && (wch <= L'9') )
		{
			bin |= (uint8_t) (wch - L'0') ;
		}
		else if ( (wch >= L'A') && (wch <= L'F') )
		{
			bin |= (uint8_t) (wch - (L'A' - 10)) ;
		}
		else if ( (wch >= L'a') && (wch <= L'f') )
		{
			bin |= (uint8_t) (wch - (L'a' - 10)) ;
		}
		pbytBuf[i] = bin ;
	}
	buf.FinishArray() ;
	return	errSuccess ;
}

// 16進数文字列のエンコード
//////////////////////////////////////////////////////////////////////////////
SError SStringParser::EncodeHexString
	( SString& strText, const uint8_t * pbytBin, size_t nBytes )
{
	uint16_t *	pwText = strText.LockBuffer( nBytes * 2 ) ;
	for ( size_t i = 0, j = 0; i < nBytes; i ++, j += 2 )
	{
		uint8_t	bin = pbytBin[i] ;
		uint8_t	h = (bin >> 4) & 0x0F ;
		if ( h < 10 )
		{
			pwText[j] = h + L'0' ;
		}
		else
		{
			pwText[j] = h + (L'A' - 10) ;
		}
		uint8_t	l = bin & 0x0F ;
		if ( l < 10 )
		{
			pwText[j + 1] = l + L'0' ;
		}
		else
		{
			pwText[j + 1] = l + (L'A' - 10) ;
		}
	}
	strText.UnlockBuffer( (ssize_t) nBytes * 2 ) ;
	return	errSuccess ;
}

// base64 文字列のデコード
//////////////////////////////////////////////////////////////////////////////
SError SStringParser::DecodeBase64String
	( SArray<uint8_t>& buf,
		const wchar_t * pwszBase64, ssize_t nLength )
{
	return	Charset::DecodeBase64( buf, pwszBase64, nLength ) ;
}

// base64 文字列のエンコード
//////////////////////////////////////////////////////////////////////////////
SError SStringParser::EncodeBase64String
	( SString& strText, const uint8_t * pbytBin, size_t nBytes )
{
	Charset::EncodeBase64( strText, pbytBin, nBytes ) ;
	return	errSuccess ;
}

// C 言語文字列エスケープシーケンス文字列のデコード
//////////////////////////////////////////////////////////////////////////////
SError SStringParser::DecodeCLangString( SString& strText, const wchar_t * pwszText )
{
	size_t		lenSrc = SString::GetLength( pwszText ) ;
	uint16_t *	pwDst = strText.LockBuffer( lenSrc ) ;
	size_t		iSrc = 0, iDst = 0 ;
	SError		errResult = errSuccess ;
	while ( iSrc < lenSrc )
	{
		wchar_t	wch = pwszText[iSrc ++] ;
		if ( wch == L'\\' )
		{
			wch = pwszText[iSrc ++] ;
			if ( (wch == L'x') || (wch == L'X') )
			{
				uint32_t	c = 0 ;
				while ( iSrc < lenSrc )
				{
					wch = pwszText[iSrc] ;
					if ( (wch >= L'0') && (wch <= L'9') )
					{
						c = (c << 4) | (wch - L'0') ;
					}
					else if ( (wch >= L'A') && (wch <= L'F') )
					{
						c = (c << 4) | (wch - L'A' + 10) ;
					}
					else if ( (wch >= L'a') && (wch <= L'f') )
					{
						c = (c << 4) | (wch - L'a' + 10) ;
					}
					else
					{
						break ;
					}
					iSrc ++ ;
				}
				pwDst[iDst ++] = (uint16_t) c ;
			}
			else if ( (wch >= L'0') && (wch < L'8') )
			{
				uint32_t	c = 0 ;
				for ( ; ; )
				{
					c = (c << 3) | (wch - L'0') ;
					wch = pwszText[iSrc] ;
					if ( (iSrc >= lenSrc)
						|| (wch < L'0') || (wch >= L'8') )
					{
						break ;
					}
					iSrc ++ ;
				}
				pwDst[iDst ++] = (uint16_t) c ;
			}
			else
			{
				const wchar_t *	pwchEscChar = L"abtnvfr?\\\"\'" ;
				const wchar_t *	pwchEscCode = L"\a\b\t\n\v\f\r\?\\\"\'" ;
				size_t	i ;
				for ( i = 0; pwchEscChar[i]; i ++ )
				{
					if ( pwchEscChar[i] == wch )
					{
						pwDst[iDst ++] = (uint16_t) pwchEscCode[i] ;
						break ;
					}
				}
				if ( pwchEscChar[i] == 0 )
				{
					pwDst[iDst ++] = (uint16_t) L'\\' ;
					pwDst[iDst ++] = (uint16_t) wch ;
					errResult = errInvalidParam ;
				}
			}
		}
		else
		{
			pwDst[iDst ++] = (uint16_t) wch ;
		}
	}
	ESLAssert( iDst <= lenSrc ) ;
	strText.UnlockBuffer( (ssize_t) iDst ) ;
	return	errResult ;
}

// C 言語文字列エスケープシーケンス文字列へエンコード
//////////////////////////////////////////////////////////////////////////////
SError SStringParser::EncodeCLangString( SString& strText, const wchar_t * pwszText )
{
	size_t		lenSrc = SString::GetLength( pwszText ) ;
	uint16_t *	pwDst = strText.LockBuffer( lenSrc * 4 ) ;
	size_t		iSrc = 0, iDst = 0 ;
	SError		errResult = errSuccess ;
	while ( iSrc < lenSrc )
	{
		wchar_t	wch = pwszText[iSrc ++] ;
		if ( (wch < 0x20) || (wch == L'\"') || (wch == L'\'') || (wch == L'\\') )
		{
			const wchar_t *	pwchEscCode = L"\a\b\t\n\v\f\r\?\\\"\'\0" ;
			const wchar_t *	pwchEscChar = L"abtnvfr?\\\"\'0" ;
			size_t	i ;
			for ( i = 0; pwchEscCode[i]; i ++ )
			{
				if ( pwchEscCode[i] == wch )
				{
					pwDst[iDst ++] = (uint16_t) L'\\' ;
					pwDst[iDst ++] = (uint16_t) pwchEscChar[i] ;
					break ;
				}
			}
			if ( pwchEscCode[i] == 0 )
			{
				if ( wch == 0 )
				{
					pwDst[iDst ++] = (uint16_t) L'\\' ;
					pwDst[iDst ++] = (uint16_t) L'0' ;
				}
				else
				{
					pwDst[iDst ++] = (uint16_t) L'\\' ;
					pwDst[iDst ++] = (uint16_t) L'x' ;
					//
					uint32_t	hex[2] ;
					hex[0] = (wch >> 4) & 0x0F ;
					hex[1] = wch & 0x0F ;
					for ( i = 0; i < 2; i ++ )
					{
						if ( hex[i] < 10 )
						{
							pwDst[iDst ++] = (uint16_t) (L'0' + hex[i]) ;
						}
						else
						{
							pwDst[iDst ++] = (uint16_t) (L'A' + hex[i] - 10) ;
						}
					}
				}
			}
		}
		else
		{
			pwDst[iDst ++] = (uint16_t) wch ;
		}
	}
	ESLAssert( iDst <= lenSrc * 4 ) ;
	strText.UnlockBuffer( (ssize_t) iDst ) ;
	return	errResult ;
}

// CSV ダブルクオーテーション文字列のデコード
//////////////////////////////////////////////////////////////////////////////
SError SStringParser::DecodeCSVString( SString& strText )
{
	size_t		lenSrc = strText.GetLength() ;
	uint16_t *	pwText = strText.LockBuffer( lenSrc ) ;
	size_t		iSrc = 0, iDst = 0 ;
	SError		errResult = errSuccess ;
	while ( iSrc < lenSrc )
	{
		wchar_t	wch = pwText[iSrc ++] ;
		pwText[iDst ++] = wch ;
		if ( wch == L'\"' )
		{
			if ( pwText[iSrc] == L'\"' )
			{
				iSrc ++ ;
			}
		}
	}
	strText.UnlockBuffer( (ssize_t) iDst ) ;
	return	errResult ;
}

// CSV ダブルクオーテーション文字列へエンコード
//////////////////////////////////////////////////////////////////////////////
SError SStringParser::EncodeCSVString
	( SString& strText, const wchar_t * pwszText, bool& flagNeedDQuote )
{
	size_t		lenSrc = SString::GetLength( pwszText ) ;
	uint16_t *	pwDst = strText.LockBuffer( lenSrc * 2 ) ;
	size_t		iSrc = 0, iDst = 0 ;
	SError		errResult = errSuccess ;
	flagNeedDQuote = false ;
	while ( iSrc < lenSrc )
	{
		wchar_t	wch = pwszText[iSrc ++] ;
		if ( (wch < 0x20) || (wch == L'\"') || (wch == L',') )
		{
			flagNeedDQuote = true ;
			pwDst[iDst ++] = (uint16_t) wch ;
			if ( wch == L'\"' )
			{
				pwDst[iDst ++] = (uint16_t) wch ;
			}
		}
		else
		{
			pwDst[iDst ++] = (uint16_t) wch ;
		}
	}
	ESLAssert( iDst <= lenSrc * 4 ) ;
	strText.UnlockBuffer( (ssize_t) iDst ) ;
	return	errResult ;
}

// printf 書式化
//////////////////////////////////////////////////////////////////////////////
const wchar_t * SStringParser::Format
	( SString& strResult, SStringFormatSupplier& suppl )
{
	enum	FormatFlag
	{
		flagPackLeft	= 0x0001,
		flagSignPlus	= 0x0002,
		flagSpacePlus	= 0x0004,
		flagPadZero		= 0x0008,
		flagNumberStyle	= 0x0010,
	} ;
	strResult = L"" ;
	MarkIndex() ;
	while ( !IsIndexOverflow() )
	{
		if ( CurrentCharacter() != L'%' )
		{
			GetCharacter() ;
			continue ;
		}
		strResult += SubStringFromMark() ;
		GetCharacter() ;
		//
		// フラグ判定
		//
		wchar_t	wchNext = GetCharacter() ;
		MarkIndex() ;
		if ( wchNext == L'%' )
		{
			strResult += L'%' ;
			continue ;
		}
		int	nFlags = 0 ;
		for ( ; ; )
		{
			if ( wchNext == L'-' )
			{
				nFlags |= flagPackLeft ;
			}
			else if ( wchNext == L'+' )
			{
				nFlags |= flagSignPlus ;
			}
			else if ( wchNext == L' ' )
			{
				nFlags |= flagSpacePlus ;
			}
			else if ( wchNext == L'0' )
			{
				nFlags |= flagPadZero ;
			}
			else if ( wchNext == L'#' )
			{
				nFlags |= flagNumberStyle ;
			}
			else
			{
				break ;
			}
			wchNext = GetCharacter() ;
			if ( IsIndexOverflow() )
			{
				break ;
			}
		}
		//
		// 幅判定
		//
		int	nWidth = 0 ;
		for ( ; ; )
		{
			if ( (wchNext >= L'0') & (wchNext <= L'9') )
			{
				nWidth = (nWidth * 10) + (wchNext - L'0') ;
			}
			else
			{
				break ;
			}
			wchNext = GetCharacter() ;
			if ( IsIndexOverflow() )
			{
				break ;
			}
		}
		//
		// 精度判定
		//
		int	nPrecision = 0 ;
		if ( wchNext == L'.' )
		{
			wchNext = GetCharacter() ;
			for ( ; ; )
			{
				if ( (wchNext >= L'0') & (wchNext <= L'9') )
				{
					nPrecision = (nPrecision * 10) + (wchNext - L'0') ;
				}
				else
				{
					break ;
				}
				wchNext = GetCharacter() ;
				if ( IsIndexOverflow() )
				{
					break ;
				}
			}
		}
		//
		// プレフィックスは無視
		//
		for ( ; ; )
		{
			if ( (wchNext != L'h') & (wchNext != L'l') & (wchNext != L'L') )
			{
				break ;
			}
			wchNext = GetCharacter() ;
			if ( IsIndexOverflow() )
			{
				break ;
			}
		}
		//
		// 型判定
		//
		if ( (wchNext == L'd')
			| (wchNext == L'i') | (wchNext == L'o')
			| (wchNext == L'x') | (wchNext == L'X') )
		{
			//
			// 整数
			//
			SString	strNumber ;
			int	nRadix = 10 ;
			if ( wchNext != L'd' )
			{
				if ( (wchNext == L'x') | (wchNext == L'X') )
				{
					nRadix = 16 ;
				}
				else
				{
					nRadix = 8 ;
				}
			}
			if ( nFlags & flagPadZero )
			{
				if ( nRadix != 16 )
				{
					strNumber.FromInteger( suppl.NextInteger(), nWidth, nRadix ) ;
				}
				else
				{
					strNumber.HexFromInteger( suppl.NextInteger(), nWidth ) ;
				}
			}
			else
			{
				if ( nRadix != 16 )
				{
					strNumber.FromInteger( suppl.NextInteger(), 0, nRadix ) ;
				}
				else
				{
					strNumber.HexFromInteger( suppl.NextInteger() ) ;
				}
				int	i = 0 ;
				while ( (int) strNumber.GetLength() + i < nWidth )
				{
					strResult += L' ' ;
					i ++ ;
				}
			}
			strResult += strNumber ;
		}
		else if ( (wchNext == L'e') | (wchNext == L'E')
				| (wchNext == L'f') | (wchNext == L'g') | (wchNext == L'G') )
		{
			//
			// 浮動小数点
			//
			SString	strNumber ;
			strNumber.FromReal( suppl.NextDouble(), nPrecision ) ;
			int	i = 0 ;
			while ( (int) strNumber.GetLength() + i < nWidth )
			{
				strResult += L' ' ;
				i ++ ;
			}
			strResult += strNumber ;
		}
		else if ( (wchNext == L'c') | (wchNext == L'C') )
		{
			//
			// 文字
			//
			strResult += suppl.NextCharacter() ;
		}
		else if ( (wchNext == L's') | (wchNext == L'S') )
		{
			//
			// 文字列
			//
			SString	strNext ;
			strResult += suppl.NextString( strNext ) ;
		}
		else
		{
			strResult += SubStringFromMark() ;
		}
		MarkIndex() ;
	}
	strResult += SubStringFromMark() ;
	return	strResult ;
}


//////////////////////////////////////////////////////////////////////////////
// 標準的な書式化パラメータ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SStringFormatArgList, SStringFormatSupplier )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SStringFormatArgList::SStringFormatArgList( void )
{
	m_next = 0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SStringFormatArgList::~SStringFormatArgList( void )
{
}

// 次の整数取得
//////////////////////////////////////////////////////////////////////////////
int64_t SStringFormatArgList::NextInteger( void )
{
	Argument *	pArg = m_arg.GetAt( m_next ++ ) ;
	if ( pArg == NULL )
	{
		return	0 ;
	}
	switch ( pArg->type )
	{
	case	typeInteger:
		return	pArg->numInt ;
	case	typeRealNumber:
		return	eslRoundR64ToLInt( pArg->numReal ) ;
	case	typeString:
		return	pArg->varStr.AsInteger() ;
	}
	return	0 ;
}

// 次の浮動小数点取得
//////////////////////////////////////////////////////////////////////////////
double SStringFormatArgList::NextDouble( void )
{
	Argument *	pArg = m_arg.GetAt( m_next ++ ) ;
	if ( pArg == NULL )
	{
		return	0.0 ;
	}
	switch ( pArg->type )
	{
	case	typeInteger:
		return	(double) pArg->numInt ;
	case	typeRealNumber:
		return	pArg->numReal ;
	case	typeString:
		return	pArg->varStr.AsReal() ;
	}
	return	0.0 ;
}

// 次の文字取得
//////////////////////////////////////////////////////////////////////////////
wchar_t SStringFormatArgList::NextCharacter( void )
{
	Argument *	pArg = m_arg.GetAt( m_next ++ ) ;
	if ( pArg == NULL )
	{
		return	0 ;
	}
	switch ( pArg->type )
	{
	case	typeInteger:
		return	(wchar_t) pArg->numInt ;
	case	typeRealNumber:
		return	(wchar_t) pArg->numReal ;
	case	typeString:
		return	pArg->varStr.GetAt( 0 ) ;
	}
	return	0 ;
}

// 次の文字列取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * SStringFormatArgList::NextString( SString& strNext )
{
	Argument *	pArg = m_arg.GetAt( m_next ++ ) ;
	if ( pArg == NULL )
	{
		return	NULL ;
	}
	switch ( pArg->type )
	{
	case	typeInteger:
		strNext.FromInteger( pArg->numInt ) ;
		break ;
	case	typeRealNumber:
		strNext.FromReal( pArg->numReal ) ;
		break ;
	case	typeString:
		strNext = pArg->varStr ;
		break ;
	}
	return	strNext ;
}

// 整数追加
//////////////////////////////////////////////////////////////////////////////
void SStringFormatArgList::AddInteger( int64_t num )
{
	Argument *	pArg = new Argument ;
	pArg->type = typeInteger ;
	pArg->numInt = num ;
	m_arg.Add( pArg ) ;
}

// 浮動小数点追加
//////////////////////////////////////////////////////////////////////////////
void SStringFormatArgList::AddDouble( double num )
{
	Argument *	pArg = new Argument ;
	pArg->type = typeRealNumber ;
	pArg->numReal = num ;
	m_arg.Add( pArg ) ;
}

// 次の文字取得
//////////////////////////////////////////////////////////////////////////////
void SStringFormatArgList::AddCharacter( wchar_t ch )
{
	Argument *	pArg = new Argument ;
	pArg->type = typeInteger ;
	pArg->numInt = ch ;
	m_arg.Add( pArg ) ;
}

// 次の文字列取得
//////////////////////////////////////////////////////////////////////////////
void SStringFormatArgList::AddString( const wchar_t * str )
{
	Argument *	pArg = new Argument ;
	pArg->type = typeString ;
	pArg->varStr = str ;
	m_arg.Add( pArg ) ;
}

// 可変長引数を追加
//////////////////////////////////////////////////////////////////////////////
void SStringFormatArgList::AddArgumentList( const char * pszFormat, va_list argptr )
{
	SStringParser	sparsForm = pszFormat ;
	while ( !sparsForm.IsIndexOverflow() )
	{
		if ( sparsForm.CurrentCharacter() != L'%' )
		{
			sparsForm.GetCharacter() ;
			continue ;
		}
		sparsForm.GetCharacter() ;
		//
		// フラグ判定
		//
		wchar_t	wchNext = sparsForm.GetCharacter() ;
		if ( wchNext == L'%' )
		{
			continue ;
		}
		int	nFlags = 0 ;
		for ( ; ; )
		{
			if ( (wchNext != L'-')
				&& (wchNext != L'+')
				&& (wchNext != L' ')
				&& (wchNext != L'0')
				&& (wchNext != L'#') )
			{
				break ;
			}
			wchNext = sparsForm.GetCharacter() ;
			if ( sparsForm.IsIndexOverflow() )
			{
				break ;
			}
		}
		//
		// 幅判定
		//
		for ( ; ; )
		{
			if ( (wchNext < L'0') | (wchNext > L'9') )
			{
				break ;
			}
			wchNext = sparsForm.GetCharacter() ;
			if ( sparsForm.IsIndexOverflow() )
			{
				break ;
			}
		}
		//
		// 精度判定
		//
		if ( wchNext == L'.' )
		{
			wchNext = sparsForm.GetCharacter() ;
			for ( ; ; )
			{
				if ( (wchNext < L'0') | (wchNext > L'9') )
				{
					break ;
				}
				wchNext = sparsForm.GetCharacter() ;
				if ( sparsForm.IsIndexOverflow() )
				{
					break ;
				}
			}
		}
		//
		// プレフィックス
		//
		bool	fLong = false ;
		for ( ; ; )
		{
			if ( (wchNext == L'l') | (wchNext == L'L') )
			{
				fLong = true ;
			}
			else if ( wchNext != L'h' )
			{
				break ;
			}
			wchNext = sparsForm.GetCharacter() ;
			if ( sparsForm.IsIndexOverflow() )
			{
				break ;
			}
		}
		//
		// 型判定
		//
		if ( (wchNext == L'd')
			| (wchNext == L'i') | (wchNext == L'o')
			| (wchNext == L'x') | (wchNext == L'X') )
		{
			//
			// 整数
			//
			if ( fLong )
			{
				AddInteger( va_arg( argptr, int64_t ) ) ;
			}
			else
			{
				AddInteger( va_arg( argptr, int ) ) ;
			}
		}
		else if ( (wchNext == L'e') | (wchNext == L'E')
				| (wchNext == L'f') | (wchNext == L'g') | (wchNext == L'G') )
		{
			//
			// 浮動小数点
			//
			AddDouble( va_arg( argptr, double ) ) ;
		}
		else if ( (wchNext == L'c') | (wchNext == L'C') )
		{
			//
			// 文字
			//
			AddInteger( (char) va_arg( argptr, int ) ) ;
		}
		else if ( (wchNext == L's') | (wchNext == L'S') )
		{
			//
			// 文字列
			//
			AddString( SString( va_arg( argptr, const char* ) ) ) ;
		}
	}
}

#if	!defined(__COTOPHA__)

void SStringFormatArgList::AddArgumentList( const wchar_t * pwszFormat, va_list argptr )
{
	SStringParser	sparsForm = pwszFormat ;
	while ( !sparsForm.IsIndexOverflow() )
	{
		if ( sparsForm.CurrentCharacter() != L'%' )
		{
			sparsForm.GetCharacter() ;
			continue ;
		}
		sparsForm.GetCharacter() ;
		//
		// フラグ判定
		//
		wchar_t	wchNext = sparsForm.GetCharacter() ;
		if ( wchNext == L'%' )
		{
			continue ;
		}
		int	nFlags = 0 ;
		for ( ; ; )
		{
			if ( (wchNext != L'-')
				&& (wchNext != L'+')
				&& (wchNext != L' ')
				&& (wchNext != L'0')
				&& (wchNext != L'#') )
			{
				break ;
			}
			wchNext = sparsForm.GetCharacter() ;
			if ( sparsForm.IsIndexOverflow() )
			{
				break ;
			}
		}
		//
		// 幅判定
		//
		for ( ; ; )
		{
			if ( (wchNext < L'0') | (wchNext > L'9') )
			{
				break ;
			}
			wchNext = sparsForm.GetCharacter() ;
			if ( sparsForm.IsIndexOverflow() )
			{
				break ;
			}
		}
		//
		// 精度判定
		//
		if ( wchNext == L'.' )
		{
			wchNext = sparsForm.GetCharacter() ;
			for ( ; ; )
			{
				if ( (wchNext < L'0') | (wchNext > L'9') )
				{
					break ;
				}
				wchNext = sparsForm.GetCharacter() ;
				if ( sparsForm.IsIndexOverflow() )
				{
					break ;
				}
			}
		}
		//
		// プレフィックス
		//
		bool	fLong = false ;
		for ( ; ; )
		{
			if ( (wchNext == L'l') | (wchNext == L'L') )
			{
				fLong = true ;
			}
			else if ( wchNext != L'h' )
			{
				break ;
			}
			wchNext = sparsForm.GetCharacter() ;
			if ( sparsForm.IsIndexOverflow() )
			{
				break ;
			}
		}
		//
		// 型判定
		//
		if ( (wchNext == L'd')
			| (wchNext == L'i') | (wchNext == L'o')
			| (wchNext == L'x') | (wchNext == L'X') )
		{
			//
			// 整数
			//
			if ( fLong )
			{
				AddInteger( va_arg( argptr, int64_t ) ) ;
			}
			else
			{
				AddInteger( va_arg( argptr, int ) ) ;
			}
		}
		else if ( (wchNext == L'e') | (wchNext == L'E')
				| (wchNext == L'f') | (wchNext == L'g') | (wchNext == L'G') )
		{
			//
			// 浮動小数点
			//
			AddDouble( va_arg( argptr, double ) ) ;
		}
		else if ( (wchNext == L'c') | (wchNext == L'C') )
		{
			//
			// 文字
			//
			AddInteger( (wchar_t) va_arg( argptr, int ) ) ;
		}
		else if ( (wchNext == L's') | (wchNext == L'S') )
		{
			//
			// 文字列
			//
			AddString( va_arg( argptr, const wchar_t* ) ) ;
		}
	}
}

#endif


//////////////////////////////////////////////////////////////////////////////
// 書式化パラメータ供給インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_NV_CLASS_INFO( SSystem::SStringFormatSupplier )


//////////////////////////////////////////////////////////////////////////////
// パーサーエラー出力（基底）
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_NV_CLASS_INFO( SSystem::SParserErrorInterface )

// エラー出力
//////////////////////////////////////////////////////////////////////////////
void SParserErrorInterface::OutputError
	( const SStringParser& ss, const wchar_t * pszError )
{
}

// 警告出力
//////////////////////////////////////////////////////////////////////////////
void SParserErrorInterface::OutputWarning
	( const SStringParser& ss, const wchar_t * pszWarning )
{
}


//////////////////////////////////////////////////////////////////////////////
// パーサーエラー出力（デバッグ出力）
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SParserErrorTracer, SParserErrorInterface )

// エラー出力
//////////////////////////////////////////////////////////////////////////////
void SParserErrorTracer::OutputError
	( const SStringParser& ss, const wchar_t * pszError )
{
	SString	strErr = pszError ;
	Trace( "%s error: %s\n",
			GetParserNameForTrace(), strErr.ToCharArray().GetConstArray() ) ;
	//
#if	defined(__DEBUG__)
	SString	strLine ;
	size_t	nLineCol = ss.GetIndex() ;
	size_t	nLineNum = ss.GetLineCharIndexOf( strLine, nLineCol ) ;
	//
	ESLTrace
		( "(%d行 %d桁):%s\n",
			(int) nLineNum, (int) nLineCol,
			strLine.ToCharArray().GetConstArray() ) ;
#endif
}

// 警告出力
//////////////////////////////////////////////////////////////////////////////
void SParserErrorTracer::OutputWarning
	( const SStringParser& ss, const wchar_t * pszWarning )
{
#if	defined(__DEBUG__)
	SString	strErr = pszWarning ;
	ESLTrace( "%s warning: %s\n",
		GetParserNameForTrace(), strErr.ToCharArray().GetConstArray() ) ;
#endif
}

// トレース文字出力時見出し
//////////////////////////////////////////////////////////////////////////////
const char * SParserErrorTracer::GetParserNameForTrace( void )
{
	return	"parser" ;
}


//////////////////////////////////////////////////////////////////////////////
// パーサーエラー出力（ログ記録）
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SParserErrorLogger, SParserErrorTracer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SParserErrorLogger::SParserErrorLogger( void )
{
	m_pUserContext = NULL ;
	m_flagTraceError = false ;
	m_flagTraceWarning = false ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SParserErrorLogger::~SParserErrorLogger( void )
{
}

// デバッグ出力有効／無効化
//////////////////////////////////////////////////////////////////////////////
void SParserErrorLogger::EnableDebugTrace( bool fError, bool fWarning )
{
	m_flagTraceError = fError ;
	m_flagTraceWarning = fWarning ;
}

// ユーザーコンテキスト設定
//////////////////////////////////////////////////////////////////////////////
void SParserErrorLogger::AttachErrorLogContext( void * pUser )
{
	m_pUserContext = pUser ;
}

// ユーザーコンテキスト取得
//////////////////////////////////////////////////////////////////////////////
void * SParserErrorLogger::GetErrorLogContext( void ) const
{
	return	m_pUserContext ;
}

// エラー数取得
//////////////////////////////////////////////////////////////////////////////
size_t SParserErrorLogger::GetErrorCount( void ) const
{
	return	m_logErrors.GetLength() ;
}

// 警告数取得
//////////////////////////////////////////////////////////////////////////////
size_t SParserErrorLogger::GetWarningCount( void ) const
{
	return	m_logWarnings.GetLength() ;
}

// エラーログ取得
//////////////////////////////////////////////////////////////////////////////
SParserErrorLogger::ErrorLog *
	SParserErrorLogger::GetErrorLogAt( size_t iError ) const
{
	return	m_logErrors.GetAt( iError ) ;
}

// 警告ログ取得
//////////////////////////////////////////////////////////////////////////////
SParserErrorLogger::ErrorLog *
	SParserErrorLogger::GetWarningLogAt( size_t iWarning ) const
{
	return	m_logWarnings.GetAt( iWarning ) ;
}

// エラーログ追加
//////////////////////////////////////////////////////////////////////////////
void SParserErrorLogger::AddErrorLog( SParserErrorLogger::ErrorLog * pErrLog )
{
	m_logErrors.Add( pErrLog ) ;
}

// 警告ログ追加
//////////////////////////////////////////////////////////////////////////////
void SParserErrorLogger::AddWarningLog( SParserErrorLogger::ErrorLog * pErrLog )
{
	m_logWarnings.Add( pErrLog ) ;
}

// ログの全消去
//////////////////////////////////////////////////////////////////////////////
void SParserErrorLogger::ClearAll( void )
{
	m_logErrors.RemoveAll() ;
	m_logWarnings.RemoveAll() ;
}

// エラー出力
//////////////////////////////////////////////////////////////////////////////
void SParserErrorLogger::OutputError
	( const SStringParser& ss, const wchar_t * pszError )
{
	ErrorLog *	pLog = new ErrorLog ;
	pLog->m_strFilePath = ss.GetFilePath() ;
	pLog->m_nColNum = ss.GetIndex() ;
	pLog->m_nLineNum =
			ss.GetLineCharIndexOf( pLog->m_strLine, pLog->m_nColNum ) ;
	pLog->m_strError = pszError ;
	pLog->m_pUser = m_pUserContext ;
	m_logErrors.Add( pLog ) ;
	//
	if ( m_flagTraceError )
	{
		Trace( "%s error: %s\n",
				GetParserNameForTrace(),
				pLog->m_strError.ToCharArray().GetConstArray() ) ;
		Trace
			( "(%d行 %d桁):%s\n",
				pLog->m_nLineNum, pLog->m_nColNum,
				pLog->m_strLine.ToCharArray().GetConstArray() ) ;
	}
}

// 警告出力
//////////////////////////////////////////////////////////////////////////////
void SParserErrorLogger::OutputWarning
	( const SStringParser& ss, const wchar_t * pszWarning )
{
	ErrorLog *	pLog = new ErrorLog ;
	pLog->m_strFilePath = ss.GetFilePath() ;
	pLog->m_nColNum = ss.GetIndex() ;
	pLog->m_nLineNum =
			ss.GetLineCharIndexOf( pLog->m_strLine, pLog->m_nColNum ) ;
	pLog->m_strError = pszWarning ;
	pLog->m_pUser = m_pUserContext ;
	m_logWarnings.Add( pLog ) ;
	//
	if ( m_flagTraceWarning )
	{
		Trace( "%s warning: %s\n",
			GetParserNameForTrace(),
			pLog->m_strError.ToCharArray().GetConstArray() ) ;
	}
}



//////////////////////////////////////////////////////////////////////////////
// 構文表現
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SUsageMatcher, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SUsageMatcher::SUsageMatcher( void )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SUsageMatcher::~SUsageMatcher( void )
{
}

// 書式判定
//////////////////////////////////////////////////////////////////////////////
SError SUsageMatcher::Match
	( const wchar_t * pwszTarget,
		const wchar_t * pwszUsage,
		SObjectArray<SString> * parrParam,
		SParserErrorInterface * perr )
{
	SUsageMatcher	matcher ;
	SStringParser	sparsTarget = pwszTarget ;
	return	matcher.IsMatched( sparsTarget, pwszUsage, parrParam, perr ) ;
}

// 書式判定
//////////////////////////////////////////////////////////////////////////////
SError SUsageMatcher::IsMatched
	( SStringParser& sparsTarget,
		const wchar_t * pwszUsage,
		SObjectArray<SString> * parrParam,
		SParserErrorInterface * perr )
{
	SParserErrorInterface	perrTemp ;
	if ( perr == NULL )
	{
		perr = &perrTemp ;
	}
	SError	err = ParseUsage( pwszUsage, *perr ) ;
	if ( err )
	{
		return	err ;
	}
	if ( parrParam != nullptr )
	{
		parrParam->RemoveAll() ;
	}
	return	IsMatchedWith( sparsTarget, parrParam, *perr ) ;
}

// 書式検索
//////////////////////////////////////////////////////////////////////////////
ssize_t SUsageMatcher::FindMatched
	( SStringParser& sparsTarget,
		const wchar_t * pwszUsage,
		SUsageMatcher::UsageType utWildCardType,
		SParserErrorInterface * perr )
{
	SParserErrorInterface	perrTemp ;
	if ( perr == NULL )
	{
		perr = &perrTemp ;
	}
	SError	err = ParseUsage( pwszUsage, *perr ) ;
	if ( err )
	{
		return	err ;
	}
	return	FindMatchedWith( sparsTarget, utWildCardType ) ;
}

// 書式の解釈
//////////////////////////////////////////////////////////////////////////////
SError SUsageMatcher::ParseUsage
	( const wchar_t * pwszUsage, SParserErrorInterface & perr )
{
	SStringParser	sparsUsage = pwszUsage ;
	m_pUsage = new Usage ;
	return	ParseUsageFor( m_pUsage, sparsUsage, perr ) ;
}

// 書式の解放
//////////////////////////////////////////////////////////////////////////////
void SUsageMatcher::ReleaseUsage( void )
{
	m_pUsage = NULL ;
}

// 構文の解釈
//////////////////////////////////////////////////////////////////////////////
SError SUsageMatcher::IsMatchedWith
	( SStringParser& sparsTarget,
		SObjectArray<SString> * parrParam,
		SParserErrorInterface& perr ) const
{
	if ( m_pUsage == NULL )
	{
		return	errFailed ;
	}
	return	IsMatchedWithUsageList
		( sparsTarget, *m_pUsage, 0, parrParam, perr ) ;
}

// 書式検索
//////////////////////////////////////////////////////////////////////////////
ssize_t SUsageMatcher::FindMatchedWith
	( SStringParser & sparsTarget,
		SUsageMatcher::UsageType utWildCardType ) const
{
	if ( m_pUsage == NULL )
	{
		return	-1 ;
	}
	return	FindMatchedWithUsageList
				( sparsTarget, *m_pUsage, 0, utWildCardType ) ;
}

// 書式の解釈
//////////////////////////////////////////////////////////////////////////////
SError SUsageMatcher::ParseUsageFor
	( SUsageMatcher::Usage * pParent,
		SStringParser & sparsUsage, SParserErrorInterface& perr )
{
	SError	err = errSuccess ;
	Usage *	pUsage = NULL ;
	size_t	iOrgUsage = sparsUsage.GetIndex( ) ;
	int		iParamBegin = -1 ;
	//
	while ( !sparsUsage.IsIndexOverflow() )
	{
		//
		// 次の書式を判定
		//
		wchar_t	wchNext ;
		sparsUsage.MarkIndex() ;
		wchNext = sparsUsage.GetCharacter() ;
		if ( SStringParser::IsCharacterSpace( wchNext ) )
		{
			//
			// 空白：トークンの切れ目
			//
			pParent->m_elements.Add( new Usage( utSpace, pParent ) ) ;
			sparsUsage.PassSpace() ;
			continue ;
		}
		else if ( wchNext == L'%' )
		{
			//
			// 任意タイプのトークン
			//
			wchNext = sparsUsage.GetCharacter() ;
			if ( wchNext == L't' )
			{
				// 任意トークン
				pParent->m_elements.Add
						( new Usage( utWildCardToken, pParent ) ) ;
			}
			else if ( wchNext == L'n' )
			{
				// 10進数字文字列
				pParent->m_elements.Add
						( new Usage( utNumberUsage, pParent ) ) ;
			}
			else if ( wchNext == L'c' )
			{
				// 指定文字コードまでの区間文字列
				wchNext = sparsUsage.GetCharacter( ) ;
				pUsage = new Usage( utEnclosed, pParent ) ;
				pUsage->m_string = SString( &wchNext, 1 ) ;
				pParent->m_elements.Add( pUsage ) ;
			}
			else if ( wchNext == L's' )
			{
				// 任意の書式
				pParent->m_elements.Add
						( new Usage( utWildCardUsage, pParent ) ) ;
			}
			else if ( wchNext == L'x' )
			{
				// 任意の書式
				pParent->m_elements.Add
						( new Usage( utWildCardExpression, pParent ) ) ;
			}
			else if ( ((wchNext >= L'0') && (wchNext <= L'9'))
					|| ((wchNext >= L'A') && (wchNext <= L'F'))
					|| ((wchNext >= L'a') && (wchNext <= L'f')) )
			{
				//
				// 任意の文字コード
				//
				wchar_t	wchCode = 0 ;
				if ( (wchNext >= L'0') && (wchNext <= L'9') )
					wchCode = (wchar_t) (wchNext - L'0') ;
				else if ( (wchNext >= L'A') && (wchNext <= L'F') )
					wchCode = (wchar_t) (wchNext - L'A' + 10) ;
				else //if ( (wchNext >= L'A') && (wchNext <= L'F') )
					wchCode = (wchar_t) (wchNext - L'a' + 10) ;
				//
				wchNext = sparsUsage.GetCharacter() ;
				wchCode <<= 4 ;
				if ( (wchNext >= L'0') && (wchNext <= L'9') )
					wchCode += (wchar_t) (wchNext - L'0') ;
				else if ( (wchNext >= L'A') && (wchNext <= L'F') )
					wchCode += (wchar_t) (wchNext - L'A' + 10) ;
				else if ( (wchNext >= L'A') && (wchNext <= L'F') )
					wchCode += (wchar_t) (wchNext - L'a' + 10) ;
				else
				{
					perr.OutputError
						( sparsUsage,
							L"%## による文字コードの書式が不正です" ) ;
					err = errFailed ;
					break ;
				}
				pUsage = new Usage( utString, pParent ) ;
				pUsage->m_string = SString( &wchNext, 1 ) ;
				pParent->m_elements.Add( pUsage ) ;
			}
			else
			{
				SString	strErr = L"%" ;
				strErr += wchNext ;
				strErr += L" は定義されていないトークンタイプの指定です" ;
				perr.OutputError( sparsUsage, strErr ) ;
				err = errFailed ;
				break ;
			}
		}
		else if ( wchNext == L'\\' )
		{
			//
			// 任意の1文字
			//
			wchNext = sparsUsage.GetCharacter() ;
			if ( wchNext )
			{
				pUsage = new Usage( utString, pParent ) ;
				pUsage->m_string = SString( &wchNext, 1 ) ;
				pParent->m_elements.Add( pUsage ) ;
			}
			else
			{
				pParent->m_elements.Add
						( new Usage( utEndOfUsage, pParent ) ) ;
				break ;
			}
		}
		else if ( wchNext == L'<' )
		{
			//
			// 任意の文字：合致文字コード取得
			//
			pUsage = new Usage( utCharacters, pParent ) ;
			pParent->m_elements.Add( pUsage ) ;
			pUsage->m_mask[0] = 0 ;
			pUsage->m_mask[1] = 0 ;
			pUsage->m_mask[2] = 0 ;
			pUsage->m_mask[3] = 0 ;
			//
			wchNext = sparsUsage.GetCharacter() ;
			if ( wchNext == L'-' )
			{
				wchNext = sparsUsage.GetCharacter() ;
				pUsage->m_flags |= ufNegative ;
			}
			wchar_t	wchLastCode = L'\0' ;
			while ( !sparsUsage.IsIndexOverflow() && (wchNext != L'>') )
			{
				if ( wchNext == L'\\' )
				{
					wchNext = sparsUsage.GetCharacter( ) ;
				}
				else if ( wchNext == L'%' )
				{
					wchar_t	wch ;
					int		i ;
					wchNext = 0 ;
					for ( i = 0; i < 2; i ++ )
					{
						wch = sparsUsage.GetCharacter( ) ;
						wchNext <<= 4 ;
						if ( (wch >= L'0') && (wch <= L'9') )
							wchNext += (wchar_t) (wch - L'0') ;
						else if ( (wch >= L'A') && (wch <= L'F') )
							wchNext += (wchar_t) (wch - L'A' + 10) ;
						else if ( (wch >= L'A') && (wch <= L'F') )
							wchNext += (wchar_t) (wch - L'a' + 10) ;
						else
						{
							sparsUsage.SeekIndex
									( sparsUsage.GetIndex() - 1 ) ;
							break ;
						}
					}
					if ( i == 0 )
					{
						wchNext = L'%' ;
					}
				}
				else if ( wchNext == L'-' )
				{
					wchNext = sparsUsage.GetCharacter( ) ;
					if ( wchNext < 0x80 )
					{
						while ( wchLastCode < wchNext )
						{
							pUsage->m_mask[(wchLastCode >> 5)]
									|= (1 << (wchLastCode & 0x1F)) ;
							wchLastCode ++ ;
						}
					}
				}
				if ( wchNext < 0x80 )
				{
					pUsage->m_mask[(wchNext >> 5)]
							|= (1 << (wchNext & 0x1F)) ;
				}
				else
				{
					pUsage->m_string += wchNext ;
				}
				wchLastCode = wchNext ;
				wchNext = sparsUsage.GetCharacter( ) ;
			}
			if ( sparsUsage.CurrentCharacter() == L'*' )
			{
				sparsUsage.GetCharacter( ) ;
				pUsage->m_flags |= ufRepeat ;
			}
		}
		else if ( wchNext == L'&' )
		{
			//
			// 文字の完全一致
			//
			pUsage = new Usage( utString, pParent ) ;
			pParent->m_elements.Add( pUsage ) ;
			//
			wchNext = sparsUsage.GetCharacter( ) ;
			if ( wchNext == L'+' )
			{
				pUsage->m_flags |= ufNoCase ;
				wchNext = sparsUsage.GetCharacter( ) ;
			}
			else if ( wchNext == L'!' )
			{
				pUsage->m_type = utToken ;
				pUsage->m_flags |= ufNoCase ;
				wchNext = sparsUsage.GetCharacter( ) ;
			}
			while ( !sparsUsage.IsIndexOverflow() && (wchNext != L'&') )
			{
				if ( wchNext == L'\\' )
				{
					wchNext = sparsUsage.GetCharacter( ) ;
				}
				pUsage->m_string += wchNext ;
				wchNext = sparsUsage.GetCharacter( ) ;
			}
		}
		else if ( wchNext == L'(' )
		{
			//
			// パラメータの開始
			//
			if ( iParamBegin >= 0 )
			{
				perr.OutputError
					( sparsUsage, L"() 括弧が二重に定義されています" ) ;
				err = errFailed ;
				break ;
			}
			iParamBegin = 0 ;
			pParent->m_elements.Add
				( new Usage( utBeginParam, pParent ) ) ;
		}
		else if ( wchNext == L')' )
		{
			//
			// パラメータの終了
			//
			if ( iParamBegin < 0 )
			{
				perr.OutputError
					( sparsUsage, L"\')\' 括弧に対応する \'(\' 括弧がありません" ) ;
				err = errFailed ;
				break ;
			}
			iParamBegin = -1 ;
			pParent->m_elements.Add
				( new Usage( utEndParam, pParent ) ) ;
		}
		else if ( (wchNext == L'[') || (wchNext == L'{') )
		{
			//
			// 省略・選択書式の判定
			//
			Usage *	pSubUsage ;
			size_t	iNestUsage = sparsUsage.GetIndex() ;
			size_t	iEndUsage ;
			size_t	nNestCount = 1 ;
			size_t	nUsageParams = 0 ;
			wchar_t	wchBegin = wchNext, wchClose ;
			if ( wchNext == L'[' )
			{
				pUsage = new Usage( utOmittable, pParent ) ;
				wchClose = L']' ;
			}
			else
			{
				pUsage = new Usage( utSelectable, pParent ) ;
				wchClose = L'}' ;
			}
			pParent->m_elements.Add( pUsage ) ;
			do
			{
				iEndUsage = sparsUsage.GetIndex( ) ;
				wchNext = sparsUsage.GetCharacter( ) ;
				if ( wchNext == wchClose )
				{
					if ( (-- nNestCount) == 0 )
					{
						break ;
					}
				}
				else if ( wchNext == L'\\' )
				{
					sparsUsage.GetCharacter( ) ;
				}
				else if ( wchNext == L'%' )
				{
					if ( sparsUsage.GetCharacter() == L'c' )
					{
						sparsUsage.GetCharacter( ) ;
					}
				}
				else if ( wchNext == wchBegin )
				{
					nNestCount ++ ;
				}
				else if ( (wchBegin == L'{') && (wchNext == L'|') )
				{
					SStringParser	sparsSubUsage ;
					sparsSubUsage.AttachSubString
						( sparsUsage, iNestUsage, (ssize_t) iEndUsage ) ;
					//
					Usage *	pSub = new Usage( utList, pUsage ) ;
					pUsage->m_elements.Add( pSub ) ;
					//
					err = ParseUsageFor( pSub, sparsSubUsage, perr ) ;
					if ( err )
					{
						break ;
					}
					iNestUsage = sparsUsage.GetIndex( ) ;
				}
				else if ( wchNext == L'(' )
				{
					nUsageParams ++ ;
				}
			}
			while ( !sparsUsage.IsIndexOverflow() ) ;
			if ( err )
			{
				break ;
			}
			SStringParser	sparsSubUsage ;
			sparsSubUsage.AttachSubString
					( sparsUsage, iNestUsage, (ssize_t) iEndUsage ) ;
			if ( pUsage->m_type == utOmittable )
			{
				pSubUsage = pUsage ;
			}
			else
			{
				Usage *	pSub = new Usage( utList, pUsage ) ;
				pUsage->m_elements.Add( pSub ) ;
				pSubUsage = pSub ;
			}
			err = ParseUsageFor( pSubUsage, sparsSubUsage, perr ) ;
			if ( err )
			{
				break ;
			}
		}
		else if ( wchNext == L'*' )
		{
			//
			// 書式の反復
			//
			pParent->m_elements.Add
				( new Usage( utRepeat, pParent ) ) ;
		}
		else
		{
			//
			// 任意文字列
			//
			pUsage = new Usage( utToken, pParent ) ;
			sparsUsage.SeekToMark() ;
			sparsUsage.NextToken( pUsage->m_string ) ;
			pParent->m_elements.Add( pUsage ) ;
		}
	}
	if ( err )
	{
		sparsUsage.SeekIndex( iOrgUsage ) ;
	}
	return	err ;
}

// 書式の一致判定
//////////////////////////////////////////////////////////////////////////////
SError SUsageMatcher::IsMatchedWithAUsage
	( SStringParser& sparsTarget,
		SUsageMatcher::Usage * pUsage,
		SObjectArray<SString> * parrParam,
		SParserErrorInterface & perr ) const
{
	SError				err = errSuccess ;
	size_t				iOrgIndex = sparsTarget.GetIndex() ;
	const uint16_t *	pwszCmp ;
	const uint16_t *	pwszStr ;
	size_t				nCmpStrLen, iLastIndex ;
	size_t				i, nCount ;
	bool				fCmpStrErr ;
	//
	ESLAssert( pUsage != NULL ) ;
	switch ( pUsage->m_type )
	{
	case	utToken:
	case	utString:
		pwszCmp = sparsTarget.GetConstArray() + sparsTarget.GetIndex() ;
		pwszStr = pUsage->m_string.GetConstArray() ;
		if ( pUsage->m_type == utToken )
		{
			iLastIndex = sparsTarget.GetIndex() ;
			sparsTarget.PassToken() ;
			nCmpStrLen = sparsTarget.GetIndex() - iLastIndex ;
		}
		else
		{
			nCmpStrLen = pUsage->m_string.GetLength() ;
			sparsTarget.SeekIndex( sparsTarget.GetIndex() + nCmpStrLen ) ;
		}
		if ( pUsage->m_flags & ufNoCase )
		{
			fCmpStrErr = true ;
			if ( pwszCmp && pwszStr
				&& (pUsage->m_string.GetLength() == nCmpStrLen) )
			{
				for ( i = 0; i < nCmpStrLen; i ++ )
				{
					wchar_t	wch1 = pwszCmp[i] ;
					wchar_t	wch2 = pwszStr[i] ;
					if ( (wch1 >= L'a') && (wch1 <= L'z') )
					{
						wch1 -= L'a' - L'A' ;
					}
					if ( (wch2 >= L'a') && (wch2 <= L'z') )
					{
						wch2 -= L'a' - L'A' ;
					}
					if ( wch1 != wch2 )
					{
						break ;
					}
				}
				fCmpStrErr = (i < nCmpStrLen) ;
			}
		}
		else
		{
			fCmpStrErr = true ;
			if ( pwszCmp && pwszStr
				&& (pUsage->m_string.GetLength() == nCmpStrLen) )
			{
				for ( i = 0; i < nCmpStrLen; i ++ )
				{
					if ( pwszCmp[i] != pwszStr[i] )
					{
						break ;
					}
				}
				fCmpStrErr = (i < nCmpStrLen) ;
			}
		}
		if ( fCmpStrErr )
		{
			if ( pwszCmp != NULL )
			{
				for ( i = 0; i < nCmpStrLen; i ++ )
				{
					if ( pwszCmp[i] == L'\0' )
					{
						nCmpStrLen = i ;
						break ;
					}
				}
			}
			else
			{
				nCmpStrLen = 0 ;
			}
			SString	strErrMsg( pwszCmp, (ssize_t) nCmpStrLen ) ;
			strErrMsg += L" が " ;
			strErrMsg += pUsage->m_string ;
			strErrMsg += L" と一致しません" ;
			perr.OutputError( sparsTarget, strErrMsg ) ;
			err = errFailed ;
		}
		break ;

	case	utSpace:
		sparsTarget.PassSpace( ) ;
		break ;

	case	utEnclosed:
		sparsTarget.PassEnclosedString( pUsage->m_string.GetAt(0) ) ;
		if ( pUsage->m_string.GetAt(0) != sparsTarget.CurrentCharacter() )
		{
			perr.OutputError
				( sparsTarget, pUsage->m_string + L" 記号が見つかりません" ) ;
			err = errFailed ;
		}
		break ;

	case	utWildCardToken:
		sparsTarget.PassToken() ;
		break ;

	case	utWildCardUsage:
	case	utWildCardExpression:
		break ;

	case	utNumberUsage:
		err = errFailed ;
		sparsTarget.HasToComeChar( L"+-" ) ;
		sparsTarget.PassSpace() ;
		for ( ; ; )
		{
			wchar_t	wch = sparsTarget.CurrentCharacter() ;
			if ( (wch < L'0') || (wch > L'9') )
			{
				break ;
			}
			err = errSuccess ;
			sparsTarget.GetCharacter() ;
		}
		if ( err )
		{
			perr.OutputError
				( sparsTarget, L"10進数文字列ではありません" ) ;
		}
		break ;

	case	utEndOfUsage:
		if ( sparsTarget.PassSpace() )
		{
			perr.OutputError
				( sparsTarget, L"末尾に不正な文字を発見しました" ) ;
			err = errFailed ;
		}
		break ;

	case	utCharacters:
		{
			bool	fLogic ;
			bool	fNegative = ((pUsage->m_flags & ufNegative) != 0) ;
			size_t	nRepeatCount = 0 ;
			while ( !sparsTarget.IsIndexOverflow() )
			{
				wchar_t	wchNext = sparsTarget.CurrentCharacter( ) ;
				if ( wchNext < 0x80 )
				{
					fLogic =
						((pUsage->m_mask[(wchNext >> 5)]
							& (1 << (wchNext & 0x1F))) != 0) ;
				}
				else
				{
					fLogic = false ;
					for ( i = 0; i < pUsage->m_string.GetLength(); i ++ )
					{
						if ( pUsage->m_string.GetAt(i) == wchNext )
						{
							fLogic = true ;
							break ;
						}
					}
				}
				if ( !(fLogic ^ fNegative) )
				{
					break ;
				}
				sparsTarget.GetCharacter( ) ;
				nRepeatCount ++ ;
				if ( !(pUsage->m_flags & ufRepeat) )
				{
					break ;
				}
			}
			if ( nRepeatCount == 0 )
			{
				perr.OutputError
					( sparsTarget, L"文字コードが適合しません" ) ;
				err = errFailed ;
			}
		}
		break ;

	case	utRepeat:
	case	utBeginParam:
	case	utEndParam:
		err = errFailed ;
		break ;

	case	utOmittable:
		{
			SObjectArray<SString>	arrParam ;
			SParserErrorInterface	dperr ;
			err = IsMatchedWithUsageList
					( sparsTarget, *pUsage, 0, &arrParam, dperr ) ;
			if ( !err && parrParam )
			{
				parrParam->MergeDuplicated
					( (size_t) parrParam->GetLength(), arrParam ) ;
			}
		}
		break ;

	case	utSelectable:
		err = errFailed ;
		nCount = pUsage->m_elements.GetLength() ;
		for ( i = 0; i < nCount; i ++ )
		{
			SParserErrorInterface	dperr ;
			ESLAssert( pUsage->m_elements.GetAt(i) != NULL ) ;
			err = IsMatchedWithAUsage
				( sparsTarget,
					pUsage->m_elements.GetAt(i), parrParam, dperr ) ;
			if ( !err )
			{
				break ;
			}
		}
		break ;

	case	utList:
		break ;
	}
	if ( err )
	{
		sparsTarget.SeekIndex( iOrgIndex ) ;
	}
	return	err ;
}

SError SUsageMatcher::IsMatchedWithUsageList
	( SStringParser& sparsTarget,
		SUsageMatcher::Usage & usage, size_t iStart,
		SObjectArray<SString> * parrParam,
		SParserErrorInterface & perr ) const
{
	SError	err = errSuccess ;
	size_t	iOrgIndex = sparsTarget.GetIndex() ;
	ssize_t	iParamBegin = -1 ;
	//
	for ( size_t iUsage = iStart;
			iUsage < usage.m_elements.GetLength(); iUsage ++ )
	{
		Usage *	pUsage = usage.m_elements.GetAt( iUsage ) ;
		ESLAssert( pUsage != NULL ) ;
		switch ( pUsage->m_type )
		{
		case	utWildCardUsage:
		case	utWildCardExpression:
			{
				ssize_t	iMatched =
					FindMatchedWithUsageList
						( sparsTarget, usage, iUsage + 1, pUsage->m_type ) ;
				if ( iMatched < 0 )
				{
					perr.OutputError
						( sparsTarget, L"適合する書式が見つかりません" ) ;
					err = errFailed ;
					break ;
				}
				sparsTarget.SeekIndex( (size_t) iMatched ) ;
			}
			break ;

		case	utBeginParam:
			ESLAssert( iParamBegin < 0 ) ;
			iParamBegin = (ssize_t) sparsTarget.GetIndex() ;
			break ;

		case	utEndParam:
			if ( iParamBegin < 0 )
			{
				perr.OutputError
					( sparsTarget, L"パラメータの開始位置が見つかりません" ) ;
			}
			else if ( parrParam != NULL )
			{
				parrParam->Add
					( new SString
						( sparsTarget.SubString
							( iParamBegin,
								(ssize_t) sparsTarget.GetIndex() - iParamBegin ) ) ) ;
				iParamBegin = -1 ;
			}
			break ;

		case	utRepeat:
			pUsage = usage.m_elements.GetAt( iUsage - 1 ) ;
			if ( pUsage != NULL )
			{
				SParserErrorInterface	dperr ;
				for ( ; ; )
				{
					SObjectArray<SString>	arrParam ;
					if ( IsMatchedWithAUsage
						( sparsTarget, pUsage, &arrParam, dperr ) )
					{
						break ;
					}
					if ( parrParam != NULL )
					{
						parrParam->MergeDuplicated( parrParam->GetLength(), arrParam ) ;
					}
				}
			}
			break ;

		case	utOmittable:
			{
				SObjectArray<SString>	arrParam ;
				SParserErrorInterface	dperr ;
				err = IsMatchedWithUsageList
						( sparsTarget, *pUsage, 0, &arrParam, dperr ) ;
				if ( parrParam != NULL )
				{
					if ( err )
					{
						size_t	nCount = pUsage->GetUsageParamCount() ;
						for ( size_t i = 0; i < nCount; i ++ )
						{
							parrParam->Add( new SString ) ;
						}
					}
					else
					{
						parrParam->MergeDuplicated( parrParam->GetLength(), arrParam ) ;
					}
				}
			}
			err = errSuccess ;
			break ;

		case	utSelectable:
			{
				bool	fSelected = false ;
				size_t	nCount = pUsage->m_elements.GetLength() ;
				for ( size_t i = 0; i < nCount; i ++ )
				{
					Usage *	pSub = pUsage->m_elements.GetAt( i ) ;
					ESLAssert( pSub != NULL ) ;
					if ( !fSelected )
					{
						SObjectArray<SString>	arrParam ;
						SParserErrorInterface	dperr ;
						SParserErrorInterface *	pperr = &dperr ;
						if ( i == (nCount - 1) )
						{
							pperr = &perr ;
						}
						if ( !IsMatchedWithUsageList
							( sparsTarget, *pSub, 0, &arrParam, *pperr ) )
						{
							if ( parrParam != NULL )
							{
								parrParam->MergeDuplicated
									( parrParam->GetLength(), arrParam ) ;
							}
							fSelected = true ;
							continue ;
						}
					}
					if ( parrParam != NULL )
					{
						size_t	m = pSub->GetUsageParamCount() ;
						for ( size_t j = 0; j < m; j ++ )
						{
							parrParam->Add( new SString ) ;
						}
					}
				}
				if ( !fSelected )
				{
					err = errFailed ;
				}
			}
			break ;

		case	utList:
			{
				SObjectArray<SString>	arrParam ;
				err = IsMatchedWithUsageList
						( sparsTarget, *pUsage, 0, &arrParam, perr ) ;
				if ( !err && (parrParam != NULL) )
				{
					parrParam->MergeDuplicated
							( parrParam->GetLength(), arrParam ) ;
				}
			}
			break ;

		default:
			err = IsMatchedWithAUsage
					( sparsTarget, pUsage, parrParam, perr ) ;
			break ;
		}
		if ( err )
		{
			break ;
		}
	}
	if ( err )
	{
		sparsTarget.SeekIndex( iOrgIndex ) ;
	}
	return	err ;
}

// 一致書式を見つける
//////////////////////////////////////////////////////////////////////////////
ssize_t SUsageMatcher::FindMatchedWithUsageList
	( SStringParser & sparsTarget,
		SUsageMatcher::Usage & usage, size_t iStart,
		SUsageMatcher::UsageType utWildCardType ) const
{
	Usage *	pUsage = NULL ;
	for ( ; ; )
	{
		pUsage = usage.m_elements.GetAt( iStart ) ;
		if ( pUsage == NULL )
		{
			Usage *	pParent = usage.m_parent ;
			Usage *	pChild = &usage ;
			while ( pParent != NULL )
			{
				ssize_t	nIndex = pParent->GetChildIndex( pChild ) ;
				ESLAssert( nIndex >= 0 ) ;
				if ( (pParent->m_type == utList)
					|| (pParent->m_type == utOmittable) )
				{
					return	FindMatchedWithUsageList
						( sparsTarget, *pParent, nIndex + 1, utWildCardType ) ;
				}
				else if ( pParent->m_parent == NULL )
				{
					return	-1 ;
				}
				pChild = pParent ;
				pParent = pParent->m_parent ;
			}
			return	-1 ;
		}
		if ( pUsage->m_type == utRepeat )
		{
			Usage *	pPrevUsage = usage.m_elements.GetAt( iStart - 1 ) ;
			if ( pPrevUsage != NULL )
			{
				ssize_t	iFind[2] ;
				iFind[0] = FindMatchedWithAUsage
					( sparsTarget, NULL, -1, pPrevUsage, utWildCardType ) ;
				if ( iFind[0] >= 0 )
				{
					iFind[1] = FindMatchedWithUsageList
						( sparsTarget, usage, iStart + 1, utWildCardType ) ;
					if ( iFind[1] < 0 )
					{
						return	iFind[0] ;
					}
					return	(iFind[0] < iFind[1]) ? iFind[0] : iFind[1] ;
				}
			}
		}
		if ( (pUsage->m_type != utSpace)
			&& (pUsage->m_type != utRepeat)
			&& (pUsage->m_type != utBeginParam)
			&& (pUsage->m_type != utEndParam) )
		{
			break ;
		}
		iStart ++ ;
	}
	return	FindMatchedWithAUsage
				( sparsTarget, &usage, iStart, pUsage, utWildCardType ) ;
}

ssize_t SUsageMatcher::FindMatchedWithAUsage
	( SStringParser & sparsTarget,
		SUsageMatcher::Usage * pParent, size_t nIndex,
		SUsageMatcher::Usage * pUsage,
		SUsageMatcher::UsageType utWildCardType ) const
{
	//
	// 書式の一致判定を行う
	//
	if ( pUsage->m_type == utOmittable )
	{
		ssize_t	iFind =
			FindMatchedWithUsageList
				( sparsTarget, *pUsage, 0, utWildCardType ) ;
		if ( iFind >= 0 )
		{
			return	iFind ;
		}
		if ( pParent == NULL )
		{
			return	-1 ;
		}
		return	FindMatchedWithUsageList
			( sparsTarget, *pParent, nIndex + 1, utWildCardType ) ;
	}
	else if ( pUsage->m_type == utSelectable )
	{
		ssize_t	iFirstFind = (ssize_t) sparsTarget.GetLength() ;
		for ( size_t i = 0; i < pUsage->m_elements.GetLength(); i ++ )
		{
			Usage *	pSelectUsage = pUsage->m_elements.GetAt( i ) ;
			ESLAssert( pSelectUsage != NULL ) ;
			ssize_t	iFind =
				FindMatchedWithUsageList
					( sparsTarget, *pSelectUsage, 0, utWildCardType ) ;
			if ( (iFind < iFirstFind) && (iFind >= 0) )
			{
				iFirstFind = iFind ;
			}
		}
		if ( iFirstFind < (ssize_t) sparsTarget.GetLength() )
		{
			return	iFirstFind ;
		}
		return	-1 ;
	}
	//
	// 単純な書式の一致判定
	//
	SParserErrorInterface	dperr ;
	SError	err = errSuccess ;
	size_t	iOrgIndex = sparsTarget.GetIndex( ) ;
	ssize_t	iFindIndex = -1 ;
	for ( ; ; )
	{
		iFindIndex = (ssize_t) sparsTarget.GetIndex( ) ;
		err = IsMatchedWithAUsage( sparsTarget, pUsage, NULL, dperr ) ;
		if ( !err )
		{
			break ;
		}
		if ( sparsTarget.IsIndexOverflow() )
		{
			iFindIndex = -1 ;
			break ;
		}
		if ( utWildCardType == utWildCardExpression )
		{
			sparsTarget.PassExpressionTerm() ;
			sparsTarget.PassSpace() ;
		}
		else if ( utWildCardType == utWildCardToken )
		{
			sparsTarget.PassToken() ;
			sparsTarget.PassSpace() ;
		}
		else if ( (pUsage->m_type == utCharacters)
					|| (pUsage->m_type == utString) )
		{
			sparsTarget.GetCharacter() ;
		}
		else
		{
			sparsTarget.PassToken() ;
			sparsTarget.PassSpace() ;
		}
	}
	sparsTarget.SeekIndex( iOrgIndex ) ;
	return	iFindIndex ;
}


//////////////////////////////////////////////////////////////////////////////
// 構文表現書式オブジェクト
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SUsageMatcher::Usage::Usage
		( SUsageMatcher::UsageType type, SUsageMatcher::Usage * pParent )
	: m_type( type ), m_flags( 0 ), m_parent( pParent )
{
}

SUsageMatcher::Usage::Usage( const SUsageMatcher::Usage& src )
	: m_type( src.m_type ), m_flags( src.m_flags ), m_parent( src.m_parent ),
		m_string( src.m_string )
{
	m_mask[0] = src.m_mask[0] ;
	m_mask[1] = src.m_mask[1] ;
	m_mask[2] = src.m_mask[2] ;
	m_mask[3] = src.m_mask[3] ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SUsageMatcher::Usage::~Usage( void )
{
}

// パラメータ数カウント
//////////////////////////////////////////////////////////////////////////////
size_t SUsageMatcher::Usage::GetUsageParamCount( void ) const
{
	if ( m_type == utBeginParam )
	{
		return	1 ;
	}
	size_t	nCount = 0 ;
	for ( size_t i = 0; i < m_elements.GetLength(); i ++ )
	{
		ESLAssert( m_elements.GetAt(i) != NULL ) ;
		nCount += m_elements.GetAt(i)->GetUsageParamCount( ) ;
	}
	return	nCount ;
}

// 子指標取得
//////////////////////////////////////////////////////////////////////////////
ssize_t SUsageMatcher::Usage::GetChildIndex( SUsageMatcher::Usage * pUsage ) const
{
	return	m_elements.FindPtr( pUsage ) ;
}
