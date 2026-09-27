
#include <sakura/sakura.h>
#include <sakura/ssys_queue_buffer.h>
#include <sakura/ssys_smart_buffer.h>

using namespace SSystem ;


//////////////////////////////////////////////////////////////////////////////
// XML（風）ドキュメント
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2( SSystem::SXMLDocument, SObject, SParserErrorInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SXMLDocument::SXMLDocument( void )
{
	m_typeDoc = typeRoot ;
	m_pAppData = NULL ;
	m_prsElementClass = &ESL_RUNTIME_CLASS(SXMLDocument) ;
}

SXMLDocument::SXMLDocument( const SXMLDocument& xmlSrc )
{
	m_pAppData = NULL ;
	CopyAllContentsFrom( xmlSrc ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SXMLDocument::~SXMLDocument( void )
{
	delete	m_pAppData ;
	m_pAppData = NULL ;
}

// 複製
//////////////////////////////////////////////////////////////////////////////
void SXMLDocument::CopyAllContentsFrom( const SXMLDocument& xmlSrc )
{
	m_typeDoc = xmlSrc.m_typeDoc ;
	m_strTag = xmlSrc.m_strTag ;
	m_strText = xmlSrc.m_strText ;
	m_ssoaAttr = xmlSrc.m_ssoaAttr ;
	m_prsElementClass = xmlSrc.m_prsElementClass ;
	//
	const size_t	nElementsCount = xmlSrc.m_xmlElements.GetLength() ;
	m_xmlElements.SetLength( nElementsCount ) ;
	for ( size_t i = 0; i < nElementsCount; i ++ )
	{
		SXMLDocument *	pxmlSrcElement = xmlSrc.GetElementAt( i ) ;
		if ( pxmlSrcElement != NULL )
		{
			SXMLDocument *	pxmlNewElement = new_XMLDocument() ;
			pxmlNewElement->CopyAllContentsFrom( *pxmlSrcElement ) ;
			m_xmlElements.SetAt( i, pxmlNewElement ) ;
		}
		else
		{
			m_xmlElements.SetAt( i, NULL ) ;
		}
	}
}

// 全ての内容を消去
//////////////////////////////////////////////////////////////////////////////
void SXMLDocument::RemoveAllContents( void )
{
	m_typeDoc = typeRoot ;
	m_strTag.FreeArray() ;
	m_strText.FreeArray() ;
	m_ssoaAttr.FreeArray() ;
	m_xmlElements.FreeArray() ;
	//
	delete	m_pAppData ;
	m_pAppData = NULL ;
}

// 空のデータか？
//////////////////////////////////////////////////////////////////////////////
bool SXMLDocument::IsEmpty( void ) const
{
	return	m_strTag.IsEmpty()
				& m_strText.IsEmpty()
				& (m_ssoaAttr.GetLength() == 0)
				& (m_xmlElements.GetLength() == 0) ;
}

// タグ設定
//////////////////////////////////////////////////////////////////////////////
void SXMLDocument::SetTag( const wchar_t * pszTag )
{
	m_typeDoc = typeTag ;
	m_strTag = pszTag ;
}

// テキスト設定
//////////////////////////////////////////////////////////////////////////////
void SXMLDocument::SetText
	( const wchar_t * pszText, SXMLDocument::DocumentType type )
{
	m_typeDoc = type ;
	m_strText = pszText ;
}

// アプリケーションデータを設定する
//////////////////////////////////////////////////////////////////////////////
void SXMLDocument::SetApplicationData( ESLObject * pObj )
{
	delete	m_pAppData ;
	m_pAppData = pObj ;
}

// 値ペア
//////////////////////////////////////////////////////////////////////////////
int64_t SXMLDocument::GetIntegerAsSymbolOf
	( const SXMLDocument::AttrInteger * pPairs,
		const wchar_t * pwszSymbol,
		int64_t nDefault, ssize_t * pFindIndex )
{
	for ( size_t i = 0; pPairs[i].pszSymbol != NULL; i ++ )
	{
		if ( SString::Compare( pPairs[i].pszSymbol, pwszSymbol ) == 0 )
		{
			if ( pFindIndex != NULL )
			{
				*pFindIndex = (ssize_t) i ;
			}
			return	pPairs[i].nValue ;
		}
	}
	if ( pFindIndex != NULL )
	{
		*pFindIndex = -1 ;
	}
	return	nDefault ;
}

int64_t SXMLDocument::GetIntegerAsNoCaseSymbolOf
	( const SXMLDocument::AttrInteger * pPairs,
		const wchar_t * pwszSymbol,
		int64_t nDefault, ssize_t * pFindIndex )
{
	for ( size_t i = 0; pPairs[i].pszSymbol != NULL; i ++ )
	{
		if ( SString::CompareNoCase( pPairs[i].pszSymbol, pwszSymbol ) == 0 )
		{
			if ( pFindIndex != NULL )
			{
				*pFindIndex = (ssize_t) i ;
			}
			return	pPairs[i].nValue ;
		}
	}
	if ( pFindIndex != NULL )
	{
		*pFindIndex = -1 ;
	}
	return	nDefault ;
}

const wchar_t * SXMLDocument::GetSymbolAsIntegerOf
	( const SXMLDocument::AttrInteger * pPairs,
				int64_t nValue, ssize_t * pFindIndex )
{
	for ( size_t i = 0; pPairs[i].pszSymbol != NULL; i ++ )
	{
		if ( pPairs[i].nValue == nValue )
		{
			if ( pFindIndex != NULL )
			{
				*pFindIndex = (ssize_t) i ;
			}
			return	pPairs[i].pszSymbol ;
		}
	}
	if ( pFindIndex != NULL )
	{
		*pFindIndex = -1 ;
	}
	return	NULL ;
}

// 属性値取得
//////////////////////////////////////////////////////////////////////////////
SString * SXMLDocument::GetAttributeAs( const wchar_t * pszAttrName ) const
{
	AttrData *	pad = m_ssoaAttr.GetAs( pszAttrName ) ;
	return	(pad != NULL) ? &(pad->m_strText) : NULL ;
}

SString SXMLDocument::GetAttrStringAs
	( const wchar_t * pszAttrName, const wchar_t * pszDefValue ) const
{
	AttrData *	pad = m_ssoaAttr.GetAs( pszAttrName ) ;
	if ( pad == NULL )
	{
		return	pszDefValue ;
	}
	ESLAssert( pad->m_nFlags & dataString ) ;
	return	pad->m_strText ;
}

int64_t SXMLDocument::GetAttrIntegerAs
	( const wchar_t * pszAttrName, int64_t nDefValue ) const
{
	AttrData *	pad = m_ssoaAttr.GetAs( pszAttrName ) ;
	if ( pad == NULL )
	{
		return	nDefValue ;
	}
	if ( pad->m_nFlags & dataInteger )
	{
		return	pad->m_nInteger ;
	}
	bool	fError ;
	ESLAssert( pad->m_nFlags & dataString ) ;
	int64_t	nValue = pad->m_strText.AsInteger( 10, true, &fError ) ;
	if ( fError )
	{
		return	nDefValue ;
	}
	pad->m_nFlags |= dataInteger ;
	pad->m_nInteger = nValue ;
	return	nValue ;
}

int64_t SXMLDocument::GetAttrHexIntegerAs
	( const wchar_t * pszAttrName, int64_t nDefValue ) const
{
	AttrData *	pad = m_ssoaAttr.GetAs( pszAttrName ) ;
	if ( pad == NULL )
	{
		return	nDefValue ;
	}
	if ( pad->m_nFlags & dataHexInt )
	{
		return	pad->m_nHexInt ;
	}
	bool	fError ;
	ESLAssert( pad->m_nFlags & dataString ) ;
	int64_t	nValue = pad->m_strText.AsInteger( 16, false, &fError ) ;
	if ( fError )
	{
		return	nDefValue ;
	}
	pad->m_nFlags |= dataHexInt ;
	pad->m_nHexInt = nValue ;
	return	nValue ;
}

int64_t SXMLDocument::GetAttrRichIntegerAs
	( const wchar_t * pszAttrName, int64_t nDefValue ) const
{
	AttrData *	pad = m_ssoaAttr.GetAs( pszAttrName ) ;
	if ( pad == NULL )
	{
		return	nDefValue ;
	}
	SStringParser	sparsNumber ;
	ESLAssert( pad->m_nFlags & dataString ) ;
	sparsNumber.AttachString( pad->m_strText ) ;
	//
	int	typeNum = sparsNumber.IsNextNumber
						( SStringParser::ctrlCStyleNumber ) ;
	if ( typeNum == SStringParser::numberInvalid )
	{
		return	nDefValue ;
	}
	return	sparsNumber.NextInteger( typeNum ) ;
}

int64_t SXMLDocument::GetAttrSymbolizedIntegerAs
	( const wchar_t * pszAttrName,
		const AttrInteger * pPairs, int64_t nDefValue ) const
{
	AttrData *	pad = m_ssoaAttr.GetAs( pszAttrName ) ;
	if ( pad == NULL )
	{
		return	nDefValue ;
	}
	ESLAssert( pad->m_nFlags & dataString ) ;
	for ( size_t i = 0; pPairs[i].pszSymbol != NULL; i ++ )
	{
		if ( pad->m_strText == pPairs[i].pszSymbol )
		{
			return	pPairs[i].nValue ;
		}
	}
	return	nDefValue ;
}

int64_t SXMLDocument::GetAttrComplexIntegerAs
	( const wchar_t * pszAttrName,
		const AttrInteger * pPairs,
		int64_t nDefValue, const wchar_t * pszSeparators ) const
{
	AttrData *	pad = m_ssoaAttr.GetAs( pszAttrName ) ;
	if ( pad == NULL )
	{
		return	nDefValue ;
	}
	SStringParser	sparsAttr ;
	ESLAssert( pad->m_nFlags & dataString ) ;
	sparsAttr.AttachString( pad->m_strText ) ;
	//
	SString	strToken ;
	int64_t	nValue = 0 ;
	while ( sparsAttr.PassSpace() )
	{
		sparsAttr.NextToken( strToken ) ;
		//
		for ( size_t i = 0; pPairs[i].pszSymbol != NULL; i ++ )
		{
			if ( strToken == pPairs[i].pszSymbol )
			{
				nValue |= pPairs[i].nValue ;
				break ;
			}
		}
		if ( pszSeparators != NULL )
		{
			sparsAttr.HasToComeChar( pszSeparators ) ;
		}
	}
	return	nValue ;
}

double SXMLDocument::GetAttrRealAs
	( const wchar_t * pszAttrName, double nDefValue ) const
{
	AttrData *	pad = m_ssoaAttr.GetAs( pszAttrName ) ;
	if ( pad == NULL )
	{
		return	nDefValue ;
	}
	if ( pad->m_nFlags & dataFloat )
	{
		return	pad->m_nFloat ;
	}
	bool	fError ;
	ESLAssert( pad->m_nFlags & dataString ) ;
	double	nValue = pad->m_strText.AsReal( 10, &fError ) ;
	if ( fError )
	{
		return	nDefValue ;
	}
	pad->m_nFlags |= dataFloat ;
	pad->m_nFloat = nValue ;
	return	nValue ;
}

double SXMLDocument::GetAttrRichRealAs
	( const wchar_t * pszAttrName, double nDefValue ) const
{
	AttrData *	pad = m_ssoaAttr.GetAs( pszAttrName ) ;
	if ( pad == NULL )
	{
		return	nDefValue ;
	}
	SStringParser	sparsNumber ;
	ESLAssert( pad->m_nFlags & dataString ) ;
	sparsNumber.AttachString( pad->m_strText ) ;
	//
	int	typeNum = sparsNumber.IsNextNumber() ;
	if ( typeNum == SStringParser::numberInvalid )
	{
		return	nDefValue ;
	}
	return	sparsNumber.NextRealNumber( typeNum ) ;
}

// 属性値設定
//////////////////////////////////////////////////////////////////////////////
void SXMLDocument::SetAttributeAs
	( const wchar_t * pszAttrName, const wchar_t * pszValue )
{
	AttrData *	pad = m_ssoaAttr.GetAs( pszAttrName ) ;
	if ( pad == NULL )
	{
		pad = new AttrData ;
		m_ssoaAttr.SetAs( pszAttrName, pad ) ;
	}
	pad->m_nFlags = dataString ;
	pad->m_strText = pszValue ;
}

void SXMLDocument::SetAttrIntegerAs
	( const wchar_t * pszAttrName, int64_t nValue )
{
	AttrData *	pad = m_ssoaAttr.GetAs( pszAttrName ) ;
	if ( pad == NULL )
	{
		pad = new AttrData ;
		m_ssoaAttr.SetAs( pszAttrName, pad ) ;
	}
	pad->m_nFlags = dataString | dataInteger ;
	pad->m_nInteger = nValue ;
	pad->m_strText.FromInteger( nValue ) ;
}

void SXMLDocument::SetAttrHexIntegerAs
	( const wchar_t * pszAttrName, int64_t nValue )
{
	AttrData *	pad = m_ssoaAttr.GetAs( pszAttrName ) ;
	if ( pad == NULL )
	{
		pad = new AttrData ;
		m_ssoaAttr.SetAs( pszAttrName, pad ) ;
	}
	pad->m_nFlags = dataString | dataHexInt ;
	pad->m_nHexInt = nValue ;
	pad->m_strText.HexFromInteger( nValue ) ;
}

SError SXMLDocument::SetAttrSymbolizedIntegerAs
	( const wchar_t * pszAttrName,
		const AttrInteger * pPairs, int64_t nValue )
{
	for ( size_t i = 0; pPairs[i].pszSymbol != NULL; i ++ )
	{
		if ( pPairs[i].nValue == nValue )
		{
			SetAttributeAs( pszAttrName, pPairs[i].pszSymbol ) ;
			return	errSuccess ;
		}
	}
	m_ssoaAttr.RemoveAs( pszAttrName ) ;
	return	errFailed ;
}

SError SXMLDocument::SetAttrComplexIntegerAs
	( const wchar_t * pszAttrName,
		const AttrInteger * pPairs, int64_t nValue,
		const wchar_t * pszSeparator )
{
	SString	strValues ;
	for ( size_t i = 0; pPairs[i].pszSymbol != NULL; i ++ )
	{
		if ( pPairs[i].nValue == 0 )
		{
			if ( nValue == 0 )
			{
				strValues = pPairs[i].pszSymbol ;
				break ;
			}
		}
		else if ( (nValue & pPairs[i].nValue) == pPairs[i].nValue )
		{
			if ( !strValues.IsEmpty() )
			{
				strValues += pszSeparator ;
			}
			strValues += pPairs[i].pszSymbol ;
			//
			nValue &= ~pPairs[i].nValue ;
			if ( nValue == 0 )
			{
				break ;
			}
		}
	}
	SetAttributeAs( pszAttrName, strValues ) ;
	if ( nValue != 0 )
	{
		return	errFailed ;
	}
	return	errSuccess ;
}

void SXMLDocument::SetAttrRealAs
	( const wchar_t * pszAttrName, double nValue )
{
	AttrData *	pad = m_ssoaAttr.GetAs( pszAttrName ) ;
	if ( pad == NULL )
	{
		pad = new AttrData ;
		m_ssoaAttr.SetAs( pszAttrName, pad ) ;
	}
	pad->m_nFlags = dataString | dataFloat ;
	pad->m_nFloat = nValue ;
	pad->m_strText.FromReal( nValue ) ;
}

// 属性削除
//////////////////////////////////////////////////////////////////////////////
void SXMLDocument::RemoveAttributeAs( const wchar_t * pwszAttrName )
{
	m_ssoaAttr.RemoveAs( pwszAttrName ) ;
}

// 属性全削除
//////////////////////////////////////////////////////////////////////////////
void SXMLDocument::RemoveAllAttributes( void )
{
	m_ssoaAttr.RemoveAll() ;
}

// サブタグ検索
//////////////////////////////////////////////////////////////////////////////
ssize_t SXMLDocument::FindElement
	( SXMLDocument::DocumentType typeDoc,
			const wchar_t * pszTag, size_t iFirst ) const
{
	const size_t	nCount = m_xmlElements.GetLength() ;
	for ( size_t i = iFirst; i < nCount; i ++ )
	{
		SXMLDocument *	pDoc = m_xmlElements.GetAt( i ) ;
		if ( (pDoc != NULL)
			&& (pDoc->m_typeDoc == typeDoc)
			&& (pDoc->m_strTag == pszTag) )
		{
			return	(ssize_t) i ;
		}
	}
	return	-1 ;
}

// サブタグ取得
//////////////////////////////////////////////////////////////////////////////
SXMLDocument * SXMLDocument::GetElementAs
	( SXMLDocument::DocumentType typeDoc,
			const wchar_t * pszTag, size_t iFirst ) const
{
	return	m_xmlElements.GetAt( FindElement( typeDoc, pszTag, iFirst ) ) ;
}

SString * SXMLDocument::GetTextElement( size_t iFirst ) const
{
	const size_t	nCount = m_xmlElements.GetLength() ;
	for ( size_t i = iFirst; i < nCount; i ++ )
	{
		SXMLDocument *	pDoc = m_xmlElements.GetAt( i ) ;
		if ( (pDoc != NULL)
			&& ((pDoc->m_typeDoc == typeText)
				|| (pDoc->m_typeDoc == typeCDATA)) )
		{
			return	&(pDoc->m_strText) ;
		}
	}
	return	NULL ;
}

// サブタグ生成
//////////////////////////////////////////////////////////////////////////////
SXMLDocument * SXMLDocument::CreateElementAs
	( SXMLDocument::DocumentType typeDoc,
		const wchar_t * pszTag, size_t iFirst )
{
	SXMLDocument *	pDoc = GetElementAs( typeDoc, pszTag, iFirst ) ;
	if ( pDoc == NULL )
	{
		pDoc = new_XMLDocument() ;
		pDoc->m_typeDoc = typeDoc ;
		pDoc->m_strTag = pszTag ;
		//
		AddElement( pDoc ) ;
	}
	return	pDoc ;
}

// サブコンテンツ追加
//////////////////////////////////////////////////////////////////////////////
SXMLDocument * SXMLDocument::AddTextElement
	( const wchar_t * pszText, SXMLDocument::DocumentType type )
{
	SXMLDocument *	pDoc = new_XMLDocument() ;
	pDoc->SetText( pszText, type ) ;
	AddElement( pDoc ) ;
	return	pDoc ;
}

SXMLDocument * SXMLDocument::SetTextElement
	( const wchar_t * pszText, SXMLDocument::DocumentType type )
{
	SXMLDocument *	pxmlText = GetElementAs( type ) ;
	if ( pxmlText != NULL )
	{
		pxmlText->SetText( pszText, type ) ;
		return	pxmlText ;
	}
	else
	{
		return	AddTextElement( pszText, type ) ;
	}
}

// コンテンツ値取得
//////////////////////////////////////////////////////////////////////////////
SXMLDocument * SXMLDocument::GetContentsElement( const wchar_t * pszPath ) const
{
	SStringParser	spars ;
	if ( sizeof(wchar_t) == sizeof(uint16_t) )
	{
		spars.AttachString( (const uint16_t*) pszPath ) ;
	}
	else
	{
		spars.SetString( pszPath ) ;
	}
	const SXMLDocument *	pDoc = this ;
	SString	strName ;
	for ( ; ; )
	{
		if ( spars.NextEnclosedString( strName, L'\\' ) != L'\\' )
		{
			break ;
		}
		pDoc = pDoc->GetElementTagAs( strName ) ;
		if ( pDoc == NULL )
		{
			return	NULL ;
		}
	}
	return	pDoc->GetElementTagAs( strName ) ;
}

SXMLDocument * SXMLDocument::CreateContentsElement( const wchar_t * pszPath )
{
	SStringParser	spars ;
	if ( sizeof(wchar_t) == sizeof(uint16_t) )
	{
		spars.AttachString( (const uint16_t*) pszPath ) ;
	}
	else
	{
		spars.SetString( pszPath ) ;
	}
	SXMLDocument *	pDoc = this ;
	SString	strName ;
	for ( ; ; )
	{
		if ( spars.NextEnclosedString( strName, L'\\' ) != L'\\' )
		{
			break ;
		}
		pDoc = pDoc->CreateElementTagAs( strName ) ;
	}
	return	pDoc->CreateElementTagAs( strName ) ;
}

SString * SXMLDocument::GetContentsValue( const wchar_t * pszPath ) const
{
	SStringParser	spars ;
	if ( sizeof(wchar_t) == sizeof(uint16_t) )
	{
		spars.AttachString( (const uint16_t*) pszPath ) ;
	}
	else
	{
		spars.SetString( pszPath ) ;
	}
	const SXMLDocument *	pDoc = this ;
	SString	strName ;
	for ( ; ; )
	{
		if ( spars.NextEnclosedString( strName, L'\\' ) != L'\\' )
		{
			break ;
		}
		pDoc = pDoc->GetElementTagAs( strName ) ;
		if ( pDoc == NULL )
		{
			return	NULL ;
		}
	}
	SString *	pstrValue = pDoc->GetAttributeAs( strName ) ;
	if ( pstrValue != NULL )
	{
		return	pstrValue ;
	}
	pDoc = pDoc->GetElementTagAs( strName ) ;
	if ( pDoc != NULL )
	{
		if ( pDoc->GetElementsCount() == 1 )
		{
			SXMLDocument *	pElement = pDoc->GetElementAt( 0 ) ;
			if ( pElement != NULL )
			{
				if ( (pElement->m_typeDoc == typeText)
					|| (pElement->m_typeDoc == typeCDATA) )
				{
					return	&(pElement->m_strText) ;
				}
			}
		}
	}
	return	NULL ;
}

SString SXMLDocument::GetContentsAsString
	( const wchar_t * pszPath, const wchar_t * pszDefValue ) const
{
	SString *	pstrValue = GetContentsValue( pszPath ) ;
	if ( pstrValue == NULL )
	{
		return	pszDefValue ;
	}
	return	*pstrValue ;
}

int64_t SXMLDocument::GetContentsAsInteger
	( const wchar_t * pszPath, int64_t nDefValue ) const
{
	SString *	pstrValue = GetContentsValue( pszPath ) ;
	if ( pstrValue == NULL )
	{
		return	nDefValue ;
	}
	return	pstrValue->AsInteger() ;
}

int64_t SXMLDocument::GetContentsAsHexInteger
	( const wchar_t * pszPath, int64_t nDefValue ) const
{
	SString *	pstrValue = GetContentsValue( pszPath ) ;
	if ( pstrValue == NULL )
	{
		return	nDefValue ;
	}
	return	pstrValue->AsInteger( 16 ) ;
}

double SXMLDocument::GetContentsAsReal
	( const wchar_t * pszPath, double nDefValue ) const
{
	SString *	pstrValue = GetContentsValue( pszPath ) ;
	if ( pstrValue == NULL )
	{
		return	nDefValue ;
	}
	return	pstrValue->AsReal() ;
}

// XMLデータ読み込み
//////////////////////////////////////////////////////////////////////////////
SError SXMLDocument::LoadDocument
	( const wchar_t * pwszFilePath,
		SParserErrorInterface& perr, Charset::EncodingType encoding )
{
	SSmartPointer<SFileInterface>	pFile =
		SFileOpener::DefaultNewOpenFile( pwszFilePath, SFileOpener::shareRead ) ;
	if ( pFile == NULL )
	{
		return	errFailed ;
	}
	return	ReadDocument( *pFile, perr, encoding ) ;
}

SError SXMLDocument::ReadDocument
	( SFileInterface& file,
		SParserErrorInterface& perr, Charset::EncodingType encoding )
{
	SStrSortObjectArray<SString>	ssoaDTD ;
	//
	// ファイルを読み込む
	//
	SArray<uint8_t>	bufXML ;
	size_t			lenBufXML ;
	{
		SQueueBuffer	qbufXML ;
		qbufXML.ReadFromStream( file ) ;
		//
		lenBufXML = (size_t) qbufXML.GetLength() ;
		bufXML.SetLength( lenBufXML ) ;
		qbufXML.Read( bufXML.GetArray(), lenBufXML ) ;
		bufXML.FinishArray() ;
	}
	//
	// 冒頭の <?xml ～ ?> を処理
	//
	SStringParser	sparsDoc ;
	const uint8_t *	pbytXML = bufXML ;
	size_t	nFirstLine = 0 ;
	while ( nFirstLine < lenBufXML )
	{
		uint8_t	b = pbytXML[nFirstLine] ;
		if ( (b == '\n') | (b == '\r') )
		{
			break ;
		}
		++ nFirstLine ;
	}
	Charset::Decode
		( sparsDoc, Charset::encodingUTF8,
				bufXML.GetConstArray(), (ssize_t) nFirstLine ) ;
	sparsDoc.AttachString() ;
	//
	bool	flagPassFirstTag = false ;
	if ( sparsDoc.HasToComeString( L"<?xml" ) )
	{
		SStringParser	sparsXML ;
		sparsDoc.MarkIndex() ;
		if ( sparsDoc.SeekString( L"?>" ) )
		{
			sparsXML = sparsDoc.SubStringFromMark() ;
			sparsDoc.SeekIndex( sparsDoc.GetIndex() + 2 ) ;
			flagPassFirstTag = true ;
			//
			SXMLDocument	xmlDoc ;
			xmlDoc.ParseTagAttributes( sparsXML, ssoaDTD, perr ) ;
			//
			SString *	pstrEncoding = xmlDoc.GetAttributeAs( L"encoding" ) ;
			if ( pstrEncoding != NULL )
			{
				encoding = Charset::GetEncodingType( *pstrEncoding ) ;
			}
		}
		else
		{
			sparsDoc.SeekIndex( 0 ) ;
		}
	}
	//
	// 文字エンコーディングを判別しデコード
	//
	if ( encoding == Charset::encodingUnknown )
	{
		encoding = Charset::AnalyzeEncoding
						( bufXML, (ssize_t) bufXML.GetLength() ) ;
		if ( encoding == Charset::encodingUnknown )
		{
			encoding = Charset::encodingUTF8 ;
		}
	}
	Charset::Decode
		( sparsDoc, encoding,
				bufXML.GetConstArray(), (ssize_t) bufXML.GetLength() ) ;
	sparsDoc.AttachString() ;
	//
	if ( flagPassFirstTag )
	{
		if ( sparsDoc.HasToComeString( L"<?xml" ) )
		{
			if ( sparsDoc.SeekString( L"?>" ) )
			{
				sparsDoc.SeekIndex( sparsDoc.GetIndex() + 2 ) ;
			}
		}
	}
	//
	// ドキュメント全体を処理
	//
	RemoveAllContents() ;
	//
	size_t	countError =
		ParseXMLElements( sparsDoc, ssoaDTD, perr ) ;
	if ( countError > 0 )
	{
		return	errFailed ;
	}
	return	errSuccess ;
}

SError SXMLDocument::ParseDocumentFromString
	( const wchar_t * pwszXML, SParserErrorInterface * perr )
{
	SStringParser					sparsDoc = pwszXML ;
	SStrSortObjectArray<SString>	ssoaDTD ;
	//
	RemoveAllContents() ;
	//
	if ( perr == NULL )
	{
		perr = this ;
	}
	size_t	countError =
		ParseXMLElements( sparsDoc, ssoaDTD, *perr ) ;
	if ( countError > 0 )
	{
		return	errFailed ;
	}
	return	errSuccess ;
}

// XMLデータ解釈（要素配列の解釈）
//////////////////////////////////////////////////////////////////////////////
size_t SXMLDocument::ParseXMLElements
	( SStringParser& sparsDoc,
		SStrSortObjectArray<SString>& ssoaDTD, SParserErrorInterface& perr )
{
	size_t	countError = 0 ;
	while ( sparsDoc.PassSpace() )
	{
		if ( sparsDoc.HasToComeString( L"</" ) )
		{
			// </tag-name> タグ終端
			SString	strTagName ;
			countError += ParseXMLNameToken( strTagName, sparsDoc, perr ) ;
			if ( strTagName != m_strTag )
			{
				perr.OutputError
					( sparsDoc, SString(L"<") + strTagName
						+ L"> に対応しない </" + strTagName + L"> です" ) ;
				countError ++ ;
			}
			if ( sparsDoc.SeekString( L">" ) )
			{
				sparsDoc.HasToComeChar( L">" ) ;
			}
			else
			{
				perr.OutputError
					( sparsDoc, SString(L"</") + strTagName
									+ L" が > で閉じられていません" ) ;
				countError ++ ;
			}
			break ;
		}
		SXMLDocument *	pxmlElement = new_XMLDocument() ;
		countError +=
			pxmlElement->ParseDocument( sparsDoc, ssoaDTD, perr ) ;
		if ( pxmlElement->IsEmpty() )
		{
			delete	pxmlElement ;
		}
		else
		{
			AddElement( pxmlElement ) ;
		}
	}
	return	countError ;
}

// XMLデータ解釈
//////////////////////////////////////////////////////////////////////////////
size_t SXMLDocument::ParseDocument
	( SStringParser& sparsDoc,
		SStrSortObjectArray<SString>& ssoaDTD, SParserErrorInterface& perr )
{
	RemoveAllContents() ;
	if ( !sparsDoc.PassSpace() )
	{
		return	0 ;
	}
	if ( (sparsDoc.OffsetAt(0) == L'<')
		&& (sparsDoc.OffsetAt(1) == L'/') )
	{
		return	0 ;
	}
	size_t	countError = 0 ;
	if ( sparsDoc.HasToComeChar( L"<" ) == L'<' )
	{
		ESLAssert( sparsDoc.OffsetAt(0) != L'/' ) ;
		wchar_t	wch = sparsDoc.CurrentCharacter() ;
		if ( wch == '!' )
		{
			if ( sparsDoc.HasToComeString( L"!--" ) )
			{
				// コメント
				sparsDoc.MarkIndex() ;
				if ( sparsDoc.SeekString( L"-->" ) )
				{
					SetText( sparsDoc.SubStringFromMark(), typeComment ) ;
					sparsDoc.HasToComeString( L"-->" ) ;
				}
				else
				{
					sparsDoc.ReleaseMark() ;
					perr.OutputError
						( sparsDoc, L"<!-- に対応する --> が見つかりません" ) ;
					countError ++ ;
				}
			}
			else if ( sparsDoc.HasToComeString( L"![CDATA[" ) )
			{
				// CDATA
				sparsDoc.MarkIndex() ;
				if ( sparsDoc.SeekString( L"]]>" ) )
				{
					SetText( sparsDoc.SubStringFromMark(), typeCDATA ) ;
					sparsDoc.HasToComeString( L"]]>" ) ;
				}
				else
				{
					sparsDoc.ReleaseMark() ;
					perr.OutputError
						( sparsDoc, L"<![CDATA[ に対応する ]]> が見つかりません" ) ;
					countError ++ ;
				}
			}
			else
			{
				// <!DOCTYPE ～ >
				countError +=
					ParseDocTypeSection( sparsDoc, ssoaDTD, perr ) ;
			}
		}
		else if ( wch == L'?' )
		{
			// 処理命令 <?tag ～ ?>
			if ( sparsDoc.SeekString( L"?>" ) )
			{
				sparsDoc.HasToComeString( L"?>" ) ;
			}
			else
			{
				perr.OutputError
					( sparsDoc, L"<? に対応する ?> が見つかりません" ) ;
				countError ++ ;
			}
		}
		else
		{
			// <tag-name attribute-list ... >
			m_typeDoc = typeTag ;
			countError += ParseXMLNameToken( m_strTag, sparsDoc, perr ) ;
			//
			countError +=
				ParseTagAttributes( sparsDoc, ssoaDTD, perr ) ;
			//
			if ( sparsDoc.HasToComeString( L">" ) )
			{
				// 要素を解釈
				countError += ParseXMLElements( sparsDoc, ssoaDTD, perr ) ;
			}
			else if ( !sparsDoc.HasToComeString( L"/>" ) )
			{
				perr.OutputError
					( sparsDoc, L"タグを閉じる > が見つかりません" ) ;
				countError ++ ;
			}
		}
	}
	else
	{
		// 文字列要素
		sparsDoc.MarkIndex() ;
		if ( !sparsDoc.SeekString( L"<" ) )
		{
			sparsDoc.SeekIndex( sparsDoc.GetLength() ) ;
		}
		SStringParser	sparsText = sparsDoc.SubStringFromMark() ;
		SString			strLine ;
		m_strText = L"" ;
		while ( sparsText.PassSpace() )
		{
			sparsText.NextLine( strLine ) ;
			strLine.TrimRight() ;
			DecodeXMLText( strLine, ssoaDTD ) ;
			m_strText += strLine ;
		}
		m_typeDoc = typeText ;
	}
	return	countError ;
}

// 名前を取得 (namespace1:namespace2:...name)
//////////////////////////////////////////////////////////////////////////////
size_t SXMLDocument::ParseXMLNameToken
	( SString& strName,
		SStringParser& sparsDoc, SParserErrorInterface& perr )
{
	if ( sparsDoc.NextToken( strName ) != SStringParser::tokenNormal )
	{
		perr.OutputError
			( sparsDoc, SString(L"\'") + strName + L"\' は不正な名前です" ) ;
		return	1 ;
	}
	SString	strSub ;
	wchar_t	wch ;
	while ( (wch = sparsDoc.HasToComeChar( L":-" )) != L'\0' )
	{
		if ( sparsDoc.NextToken( strSub ) != SStringParser::tokenNormal )
		{
			perr.OutputError
				( sparsDoc, SString(L"\'") + strSub + L"\' は不正な名前です" ) ;
			return	1 ;
		}
		strName += wch ;
		strName += strSub ;
	}
	return	0 ;
}

// <! ... > を解釈（主に読み飛ばす）
//////////////////////////////////////////////////////////////////////////////
size_t SXMLDocument::ParseDocTypeSection
	( SStringParser& sparsDoc,
		SStrSortObjectArray<SString>& ssoaDTD,
		SParserErrorInterface& perr )
{
	size_t	countError = 0 ;
	int		countNest = 1 ;
	while ( !sparsDoc.IsIndexOverflow() )
	{
		wchar_t	wch = sparsDoc.GetCharacter() ;
		if ( wch == L'<' )
		{
			if ( sparsDoc.CurrentCharacter() == L'!' )
			{
				if ( sparsDoc.HasToComeString( L"!ENTITY" ) )
				{
					SString	strName = sparsDoc.GetString() ;
					if ( strName != L">" )
					{
						SString	strValue ;
						countError +=
							ParseTagAttributeValue
								( strValue, sparsDoc, ssoaDTD, perr ) ;
						ssoaDTD.SetAs( strName, new SString( strValue ) ) ;
					}
					else
					{
						perr.OutputError
							( sparsDoc, L"<!ENTITY> で定義名が見つかりません" ) ;
						countError ++ ;
					}
				}
				countNest ++ ;
			}
		}
		else if ( wch == L'>' )
		{
			if ( (-- countNest) <= 0 )
			{
				break ;
			}
		}
	}
	return	countError ;
}

// タグ属性を解釈（> 又は /> まで sparsDoc の指標を移動）
//////////////////////////////////////////////////////////////////////////////
size_t SXMLDocument::ParseTagAttributes
	( SStringParser& sparsDoc,
		SStrSortObjectArray<SString>& ssoaDTD,
		SParserErrorInterface& perr )
{
	size_t	countError = 0 ;
	while ( sparsDoc.PassSpace() )
	{
		wchar_t	wch = sparsDoc.CurrentCharacter() ;
		if ( (wch == L'>') | (wch == L'?') | (wch == L'/') )
		{
			break ;
		}
		SString	strName ;
		for ( ; ; )
		{
			SString	strNamePart ;
			SStringParser::TokenType
				typeToken = sparsDoc.NextToken( strNamePart );
			if ( typeToken != SStringParser::tokenNormal )
			{
				if ( strNamePart.IsEmpty() )
				{
					perr.OutputError
						( sparsDoc,
							L"タグ属性解釈中に終端に到達しました" ) ;
				}
				else
				{
					perr.OutputError
						( sparsDoc,
							SString(L"タグ属性解釈中に \"")
								+ strName + L"\" を発見しました" ) ;
				}
				countError ++ ;
				return	countError ;
			}
			strName += strNamePart ;
			//
			wchar_t	wchNext = sparsDoc.HasToComeChar( L"-:=" ) ;
			if ( wchNext == L'=' )
			{
				break ;
			}
			else if ( (wchNext == L':') || (wchNext == L'-') )
			{
				strName += wchNext ;
			}
			else
			{
				perr.OutputError
					( sparsDoc, L"タグ属性に = が見つかりません" ) ;
				countError ++ ;
				return	countError ;
			}
		}
		AttrData *	pad = new AttrData ;
		pad->m_nFlags = dataString ;
		countError +=
			ParseTagAttributeValue( pad->m_strText, sparsDoc, ssoaDTD, perr ) ;
		m_ssoaAttr.SetAs( strName, pad ) ;
	}
	return	countError ;
}

// 属性値を解釈
//////////////////////////////////////////////////////////////////////////////
size_t SXMLDocument::ParseTagAttributeValue
	( SString& strValue,
		SStringParser& sparsDoc,
		SStrSortObjectArray<SString>& ssoaDTD,
		SParserErrorInterface& perr )
{
	size_t	countError = 0 ;
	wchar_t	wchQuote = sparsDoc.CurrentCharacter() ;
	if ( (wchQuote == L'\"') | (wchQuote == L'\'') )
	{
		sparsDoc.GetCharacter() ;
		//
		wchar_t	wchClosed =
			sparsDoc.NextEnclosedString
				( strValue, wchQuote,
					(SStringParser::ctrlNoEscInQuote
						| SStringParser::ctrlNoEscInDQuote) ) ;
		if ( wchClosed != wchQuote )
		{
			perr.OutputError
				( sparsDoc, L"タグ属性がクォーテーションで閉じられていません" ) ;
			countError ++ ;
		}
	}
	else
	{
		perr.OutputWarning
			( sparsDoc, L"タグ属性がクォーテーションで囲まれていません" ) ;
		if ( (wchQuote == L'+') || (wchQuote == L'-') )
		{
			size_t	iStart = sparsDoc.GetIndex() ;
			sparsDoc.GetCharacter() ;
			wchQuote = sparsDoc.CurrentCharacter() ;
			if ( ((wchQuote >= L'0') && (wchQuote <= L'9'))
				|| ((wchQuote >= L'A') && (wchQuote <= L'Z'))
				|| ((wchQuote >= L'a') && (wchQuote <= L'f')) )
			{
				sparsDoc.PassToken() ;
			}
			strValue = sparsDoc.SubString
					( iStart, (ssize_t) (sparsDoc.GetIndex() - iStart) ) ;
		}
		else
		{
			sparsDoc.NextToken( strValue ) ;
		}
	}
	DecodeXMLText( strValue, ssoaDTD ) ;
	return	countError ;
}

// XMLデータ書き出し
//////////////////////////////////////////////////////////////////////////////
SError SXMLDocument::SaveDocument
	( const wchar_t * pwszFilePath,
		int nIndent, Charset::EncodingType encoding ) const
{
	SSmartPointer<SFileInterface>	pFile =
			SFileOpener::DefaultNewOpenFile
						( pwszFilePath, SFileOpener::modeCreate ) ;
	if ( pFile == NULL )
	{
		return	errFailed ;
	}
	return	WriteDocument( *pFile, nIndent, encoding ) ;
}

SError SXMLDocument::WriteDocument
	( SFileInterface& file,
		int nIndent, Charset::EncodingType encoding ) const
{
	//
	// エンコーディングの指定
	//
	SString	strFirstLine = L"<?xml version=\"1.0\" encoding=\"" ;
	strFirstLine += Charset::GetEncodingName( encoding ) ;
	strFirstLine += L"\"?>\r\n" ;
	file.WriteEncodedString( strFirstLine, encoding ) ;
	//
	// データ書き出し
	//
	if ( m_typeDoc == typeRoot )
	{
		return	FormatXMLElements( file, nIndent, encoding ) ;
	}
	else
	{
		return	FormatDocument( file, nIndent, encoding ) ;
	}
}

SError SXMLDocument::FormatDocumentToString( SString& strDoc, int nIndent ) const
{
	SSmartBuffer	sbuf ;
	SError	err ;
	if ( m_typeDoc == typeRoot )
	{
		err = FormatXMLElements( sbuf, nIndent, Charset::encodingUTF8 ) ;
	}
	else
	{
		err = FormatDocument( sbuf, nIndent, Charset::encodingUTF8 ) ;
	}
	if ( err )
	{
		return	err ;
	}
	SStringParser	sparsXML ;
	sbuf.Seek( 0 ) ;
	sparsXML.ReadTextFile( sbuf, Charset::encodingUTF8 ) ;
	//
	strDoc = sparsXML ;
	return	errSuccess ;
}

// XMLデータ書き出し（要素配列）
//////////////////////////////////////////////////////////////////////////////
SError SXMLDocument::FormatXMLElements
	( SFileInterface& file,
		int nIndent, Charset::EncodingType encoding ) const
{
	const size_t	nCount = m_xmlElements.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		SXMLDocument *	pElement = m_xmlElements.GetAt( i ) ;
		if ( pElement != NULL )
		{
			SError	err =
				pElement->FormatDocument( file, nIndent, encoding ) ;
			if ( err != errSuccess )
			{
				return	err ;
			}
		}
	}
	return	errSuccess ;
}

// XMLデータ書き出し（1つのタグ／文字列）
//////////////////////////////////////////////////////////////////////////////
SError SXMLDocument::FormatDocument
	( SFileInterface& file,
		int nIndent, Charset::EncodingType encoding ) const
{
	SString	strIndent ;
	if ( nIndent > 0 )
	{
		strIndent = L"\t" ;
		strIndent.Multiple( nIndent ) ;
	}
	if ( m_typeDoc == typeTag )
	{
		//
		// タグ出力 <tag ...
		//
		SString	strText = strIndent ;
		bool	flagAttr = false ;
		strText += L"<" ;
		strText += m_strTag ;
		//
		const size_t	countAttr = m_ssoaAttr.GetLength() ;
		for ( size_t iAttr = 0; iAttr < countAttr; iAttr ++ )
		{
			const SString *	pstrName = m_ssoaAttr.GetTagAt( iAttr ) ;
			AttrData *		pad = m_ssoaAttr.GetAt( iAttr ) ;
			if ( (pstrName != NULL) & (pad != NULL) )
			{
				SString	strValue ;
				ESLAssert( pad->m_nFlags & dataString ) ;
				EncodeXMLText( strValue, pad->m_strText ) ;
				//
				if ( flagAttr & (nIndent >= 0)
					& (strText.GetLength()
						+ pstrName->GetLength()
						+ strValue.GetLength() + 5 >= 80 ) )
				{
					strText += L"\r\n" ;
					file.WriteEncodedString( strText, encoding ) ;
					//
					strText = strIndent ;
					strText += L"  " ;
				}
				strText += L" " ;
				strText += *pstrName ;
				strText += L"=\"" ;
				strText += strValue ;
				strText += L"\"" ;
				flagAttr = true ;
			}
		}
		if ( m_xmlElements.GetLength() == 0 )
		{
			strText += L"/>\r\n" ;
			file.WriteEncodedString( strText, encoding ) ;
		}
		else
		{
			bool	fSimpleTextElement = false ;
			if ( m_xmlElements.GetLength() == 1 )
			{
				SXMLDocument *	pElement = m_xmlElements.GetAt( 0 ) ;
				if ( (pElement != NULL)
					&& (pElement->m_typeDoc == typeText) )
				{
					fSimpleTextElement =
						(pElement->m_strText.Find( L'\n' ) < 0) ;
				}
			}
			if ( fSimpleTextElement )
			{
				//
				// <tag>text</tag> 形式
				//
				SString	strDocText ;
				ESLAssert( m_xmlElements.GetAt(0) != NULL ) ;
				EncodeXMLText
					( strDocText, m_xmlElements.GetAt(0)->m_strText ) ;
				//
				strText += L">" ;
				strText += strDocText ;
				strText += L"</" ;
				strText += m_strTag ;
				strText += L">\r\n" ;
				//
				file.WriteEncodedString( strText, encoding ) ;
			}
			else
			{
				//
				// 複数の要素を持つ場合
				//
				strText += L">\r\n" ;
				file.WriteEncodedString( strText, encoding ) ;
				//
				SError	err =
					FormatXMLElements( file, nIndent + 1, encoding ) ;
				if ( err )
				{
					return	err ;
				}
				strText = strIndent ;
				strText += L"</" ;
				strText += m_strTag ;
				strText += L">\r\n" ;
				file.WriteEncodedString( strText, encoding ) ;
			}
		}
	}
	else if ( m_typeDoc == typeText )
	{
		//
		// 文字列要素出力
		//
		SString	strDocText ;
		EncodeXMLText( strDocText, m_strText, nIndent ) ;
		if ( nIndent > 0 )
		{
			strDocText = strIndent + strDocText ;
		}
		strDocText += L"\r\n" ;
		file.WriteEncodedString( strDocText, encoding ) ;
	}
	else if ( m_typeDoc == typeCDATA )
	{
		//
		// CDATA 出力
		//
		SString	strCDATA ;
		strCDATA = L"<![CDATA[" ;
		if ( m_strText.Find( L"]]>" ) >= 0 )
		{
			SString::FILTER_ENTRY	fe[1] =
			{
				{ L"]]>", L"]]&gt;" },
			} ;
			SString::PrepareFilter( fe, 1 ) ;
			strCDATA += m_strText.MappingFilter( fe, 1 ) ;
		}
		else
		{
			strCDATA += m_strText ;
		}
		strCDATA += L"]]>\r\n" ;
		file.WriteEncodedString( strCDATA, encoding ) ;
	}
	else if ( m_typeDoc == typeComment )
	{
		//
		// コメント出力
		//
		SString	strComment ;
		strComment = L"<!--" ;
		if ( m_strText.Find( L"-->" ) >= 0 )
		{
			SString::FILTER_ENTRY	fe[1] =
			{
				{ L"-->", L"--&gt;" },
			} ;
			SString::PrepareFilter( fe, 1 ) ;
			strComment += m_strText.MappingFilter( fe, 1 ) ;
		}
		else
		{
			strComment += m_strText ;
		}
		strComment += L"-->\r\n" ;
		file.WriteEncodedString( strComment, encoding ) ;
	}
	else
	{
		return	FormatXMLElements( file, nIndent, encoding ) ;
	}
	return	errSuccess ;
}

// タグ要素を除去したプレーンテキスト形式へ変換
//////////////////////////////////////////////////////////////////////////////
SString SXMLDocument::ToPlainText( void ) const
{
	if ( (m_typeDoc == typeText)
		|| (m_typeDoc == typeCDATA) )
	{
		return	m_strText ;
	}
	else if ( (m_typeDoc == typeRoot)
			|| (m_typeDoc == typeTag) )
	{
		SString	strText ;
		for ( size_t i = 0; i < m_xmlElements.GetLength(); i ++ )
		{
			strText += m_xmlElements.At(i).ToPlainText() ;
		}
		return	strText ;
	}
	return	SString() ;
}

// 文字列コンテンツのデコード
//////////////////////////////////////////////////////////////////////////////
SError SXMLDocument::DecodeXMLText
	( SString & strText, SStrSortObjectArray<SString>& ssoaDTD )
{
	static const wchar_t *	pwszSpecChar[] =
	{
		L"lt", L"gt", L"quot", L"amp", L"nbsp", NULL
	} ;
	static const wchar_t	wchSpecChar[] =
	{
		L'<', L'>', L'\"', L'&', 0xa0, L'\0'
	} ;
	ssize_t	iFind ;
	iFind = strText.Find( L'&' ) ;
	if ( iFind < 0 )
	{
		return	errSuccess ;
	}
	wchar_t	wchCode ;
	size_t	i, iLast = 0 ;
	SString	strBuf ;
	SString	strToken ;
	do
	{
		strBuf += strText.Middle( iLast, (ssize_t) (iFind - iLast) ) ;
		ESLAssert( strText.GetAt(iFind) == '&' ) ;
		//
		iLast = ++ iFind ;
		iFind = strText.Find( L';', iFind ) ;
		if ( iFind < 0 )
		{
			iFind = strText.Find( L'&', iLast -- ) ;
			continue ;
		}
		//
		strToken = strText.Middle( iLast, (ssize_t) (iFind - iLast) ) ;
		strToken.TrimRight( ) ;
		strToken.TrimLeft( ) ;
		//
		if ( strToken.GetAt(0) == L'#' )
		{
			SStringParser	sparsCode ;
			sparsCode.AttachString( strToken, (ssize_t) strToken.GetLength() ) ;
			sparsCode.SeekIndex( 1 ) ;
			//
			if ( sparsCode.HasToComeChar( L"Xx" ) != 0 )
			{
				// &#x...; 形式
				wchCode = (wchar_t) sparsCode.NextInteger( 16 ) ;
			}
			else
			{
				// &#...; 形式
				wchCode = (wchar_t) sparsCode.NextInteger( 10 ) ;
			}
			strBuf += wchCode ;
		}
		else
		{
			// &...; 形式
			SString *	pstrEntity = ssoaDTD.GetAs( strToken ) ;
			if ( pstrEntity != NULL )
			{
				strBuf += *pstrEntity ;
			}
			else
			{
				for ( i = 0; pwszSpecChar[i] != NULL; i ++ )
				{
					if ( strToken == pwszSpecChar[i] )
						break ;
				}
				if ( pwszSpecChar[i] )
				{
					strBuf += wchSpecChar[i] ;
				}
				else
				{
					iFind = strText.Find( L'&', iLast -- ) ;
					continue ;
				}
			}
		}
		iLast = ++ iFind ;
		iFind = strText.Find( L'&', iFind ) ;
	}
	while ( iFind >= 0 ) ;
	//
	strText = strBuf + strText.Middle( iLast ) ;
	return	errSuccess ;
}

// 文字列コンテンツのエンコーディング
//////////////////////////////////////////////////////////////////////////////
SError SXMLDocument::EncodeXMLText
	( SString & strText, const SString & strSrc, int nIndent )
{
	static const wchar_t *	pwszSpecChar[] =
	{
		L"&lt;", L"&gt;", L"&quot;", L"&amp;", L"&nbsp;", NULL
	} ;
	static const wchar_t	wchSpecChar[] =
	{
		L'<', L'>', L'\"', L'&', 0xa0, L'\0'
	} ;
	SString	strCode ;
	//
	strText = L"" ;
	strText.SetLimit( strSrc.GetLength() + 0x10 ) ;
	//
	const uint16_t *	pszSrc = strSrc ;
	const size_t		lenSrc = strSrc.GetLength() ;
	for ( size_t i = 0; i < lenSrc; i ++ )
	{
		wchar_t	wch = pszSrc[i] ;
		if ( wch <= L' ' )
		{
			if ( (wch == L' ')
				&& (strText.GetLastAt(0) > L' ')
				&& ((i + 1) < lenSrc) && (pszSrc[i + 1] > L' ') )
			{
				strText += wch ;
			}
			else
			{
				strCode.FromInteger( wch ) ;
				strText += L"&#" ;
				strText += strCode ;
				strText += L";" ;
				//
				if ( (wch == L'\n') && (i + 1 < lenSrc) )
				{
					if ( nIndent >= 0 )
					{
						strText += L"\r\n" ;
						for ( int j = 0; j < nIndent; j ++ )
						{
							strText += L'\t' ;
						}
					}
				}
			}
		}
		else if ( wch <= 0xA0 )
		{
			int	j ;
			for ( j = 0; wchSpecChar[j] != 0; j ++ )
			{
				if ( wch == wchSpecChar[j] )
					break ;
			}
			if ( wchSpecChar[j] != 0 )
			{
				strText += pwszSpecChar[j] ;
			}
			else
			{
				strText += wch ;
			}
		}
		else
		{
			strText += wch ;
		}
	}
	return	errSuccess ;
}

// SXMLDocument 要素作成
//////////////////////////////////////////////////////////////////////////////
SXMLDocument * SXMLDocument::new_XMLDocument( void )
{
	return	new SXMLDocument ;
}

// SParserErrorTracer
//////////////////////////////////////////////////////////////////////////////
const char * SXMLDocument::GetParserNameForTrace( void )
{
	return	"xml parser" ;
}


