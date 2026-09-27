
#include <cotopha.h>
#include <objected/xml_document.h>

//////////////////////////////////////////////////////////////////////////////
// XML ドキュメント
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
XMLDocument::XMLDocument( const XMLDocument& xmlSrc )
{
	CopyAllContentsFrom( xmlSrc ) ;
}

// 代入
//////////////////////////////////////////////////////////////////////////////
const XMLDocument& XMLDocument::operator = ( const XMLDocument& xmlSrc )
{
	CopyAllContentsFrom( xmlSrc ) ;
	return	this ;
}

// 複製
//////////////////////////////////////////////////////////////////////////////
void XMLDocument::CopyAllContentsFrom( const XMLDocument& xmlSrc )
{
	RemoveAllContents() ;
	//
	if ( xmlSrc !== null )
	{
		m_typeDoc = xmlSrc.m_typeDoc ;
		m_strTag = xmlSrc.m_strTag ;
		m_strText = xmlSrc.m_strText ;
		//
		for ( attr : xmlSrc.m_ssoaAttr : i )
		{
			m_ssoaAttr[xmlSrc.m_ssoaAttr.GetTagName(i)] = attr ;
		}
		for ( element : xmlSrc.m_xmlElements : i )
		{
			m_xmlElements[i] ::= XMLDocument( element ) ;
		}
	}
}

// 全ての内容を消去
//////////////////////////////////////////////////////////////////////////////
void XMLDocument::RemoveAllContents( void )
{
	m_typeDoc = DocumentType::Root ;
	m_strTag = "" ;
	m_strText = "" ;
	m_ssoaAttr.RemoveAll() ;
	m_xmlElements.Remove() ;
}

// 空のデータか？
//////////////////////////////////////////////////////////////////////////////
bool XMLDocument::IsEmpty( void ) const
{
	return	(m_strTag == "")
				&& (m_strText == "")
				&& (m_ssoaAttr.GetLength() == 0)
				&& (m_xmlElements.GetLength() == 0) ;
}

// データ種類取得
//////////////////////////////////////////////////////////////////////////////
int XMLDocument::GetType( void ) const
{
	return	m_typeDoc ;
}

// タグ取得
//////////////////////////////////////////////////////////////////////////////
String XMLDocument::GetTag( void ) const
{
	return	m_strTag ;
}


// タグ設定
//////////////////////////////////////////////////////////////////////////////
void XMLDocument::SetTag( String strTag )
{
	m_typeDoc = DocumentType::Tag ;
	m_strTag = strTag ;
}

// テキスト取得
//////////////////////////////////////////////////////////////////////////////
String XMLDocument::GetText( void ) const
{
	return	m_strText ;
}

// テキスト設定
//////////////////////////////////////////////////////////////////////////////
void XMLDocument::SetText( String strText, int type )
{
	m_typeDoc = type ;
	m_strText = strText ;
}

// 属性値取得
//////////////////////////////////////////////////////////////////////////////
const String& XMLDocument::GetAttributeAs( String strAttrName ) const
{
	if ( !m_ssoaAttr.IsEmpty( strAttrName ) )
	{
		return	m_ssoaAttr[strAttrName] ;
	}
	return	null ;
}

String XMLDocument::GetAttrStringAs
	( String strAttrName, String strDefValue ) const
{
	if ( m_ssoaAttr.IsEmpty( strAttrName ) )
	{
		return	strDefValue ;
	}
	return	m_ssoaAttr[strAttrName] ;
}

int XMLDocument::GetAttrIntegerAs
	( String strAttrName, int nDefValue ) const
{
	if ( m_ssoaAttr.IsEmpty( strAttrName ) )
	{
		return	nDefValue ;
	}
	return	int( m_ssoaAttr[strAttrName] ) ;
}

int XMLDocument::GetAttrHexIntegerAs
	( String strAttrName, int nDefValue ) const
{
	if ( m_ssoaAttr.IsEmpty( strAttrName ) )
	{
		return	nDefValue ;
	}
	return	NumberFromHexString( m_ssoaAttr[strAttrName] ) ;
}

double XMLDocument::GetAttrRealAs
	( String strAttrName, double nDefValue ) const
{
	if ( m_ssoaAttr.IsEmpty( strAttrName ) )
	{
		return	nDefValue ;
	}
	return	double( m_ssoaAttr[strAttrName] ) ;
}

// 属性値設定
//////////////////////////////////////////////////////////////////////////////
void XMLDocument::SetAttributeAs( String strAttrName, String strValue )
{
	m_ssoaAttr[strAttrName] = strValue ;
}

void XMLDocument::SetAttrIntegerAs( String strAttrName, int nValue )
{
	m_ssoaAttr[strAttrName] = String(nValue) ;
}

void XMLDocument::SetAttrHexIntegerAs( String strAttrName, int nValue )
{
	m_ssoaAttr[strAttrName] = nValue.Format(16) ;
}

void XMLDocument::SetAttrRealAs( String strAttrName, double nValue )
{
	m_ssoaAttr[strAttrName] = String(nValue) ;
}

// 属性配列取得
//////////////////////////////////////////////////////////////////////////////
Hash<String> & XMLDocument::GetAttributes( void )
{
	return	m_ssoaAttr ;
}

// 属性総数取得
//////////////////////////////////////////////////////////////////////////////
int XMLDocument::GetAttributeCount( void ) const
{
	return	m_ssoaAttr.GetLength() ;
}

// 属性名取得
//////////////////////////////////////////////////////////////////////////////
String XMLDocument::GetAttributeNameAt( int index ) const
{
	return	m_ssoaAttr.GetTagName( index ) ;
}

// 属性値取得
//////////////////////////////////////////////////////////////////////////////
String XMLDocument::GetAttributeValueAt( int index ) const
{
	if ( (index >= 0) && (index < m_ssoaAttr.GetLength()) )
	{
		return	m_ssoaAttr[index] ;
	}
	return	"" ;
}

// 要素検索
//////////////////////////////////////////////////////////////////////////////
int XMLDocument::FindElement
	( int typeDoc, String strTag, int iFirst ) const
{
	const int	nCount = m_xmlElements.GetLength() ;
	for ( int i = iFirst; i < nCount; i ++ )
	{
		const XMLDocument&	pDoc = m_xmlElements[i] ;
		if ( (pDoc.m_typeDoc == typeDoc)
			&& (pDoc.m_strTag == strTag) )
		{
			return	i ;
		}
	}
	return	-1 ;
}

int XMLDocument::FindElementTag( String strTag, int iFirst ) const
{
	return	FindElement( DocumentType::Tag, strTag, iFirst ) ;
}

// サブタグ取得
//////////////////////////////////////////////////////////////////////////////
const XMLDocument& XMLDocument::GetElementAs
	( int typeDoc, String strTag, int iFirst ) const
{
	return	GetElementAt( FindElement( typeDoc, strTag, iFirst ) ) ;
}

const XMLDocument&
	XMLDocument::GetElementTagAs( String strTag, int iFirst ) const
{
	return	GetElementAs( DocumentType::Tag, strTag, iFirst ) ;
}

XMLDocument& XMLDocument::GetElementAs
	( int typeDoc, String strTag, int iFirst )
{
	return	GetElementAt( FindElement( typeDoc, strTag, iFirst ) ) ;
}

XMLDocument&
	XMLDocument::GetElementTagAs( String strTag, int iFirst )
{
	return	GetElementAs( DocumentType::Tag, strTag, iFirst ) ;
}

// サブタグ生成
//////////////////////////////////////////////////////////////////////////////
XMLDocument& XMLDocument::CreateElementAs
	( int typeDoc, String strTag, int iFirst )
{
	int	iElement = FindElement( typeDoc, strTag, iFirst ) ;
	if ( iElement >= 0 )
	{
		return		m_xmlElements[iElement] ;
	}
	XMLDocument&	pDoc = XMLDocument ;
	pDoc.m_typeDoc = typeDoc ;
	pDoc.m_strTag = strTag ;
	AddElement( pDoc ) ;
	return	pDoc ;
}

XMLDocument& XMLDocument::CreateElementTagAs( String strTag, int iFirst )
{
	return	CreateElementAs( DocumentType::Tag, strTag, iFirst ) ;
}

// サブコンテンツ数取得
//////////////////////////////////////////////////////////////////////////////
int XMLDocument::GetElementsCount( void ) const
{
	return	m_xmlElements.GetLength() ;
}

// サブコンテンツ取得
//////////////////////////////////////////////////////////////////////////////
const XMLDocument& XMLDocument::GetElementAt( int index ) const
{
	if ( (index >= 0) && (index < m_xmlElements.GetLength()) )
	{
		return	m_xmlElements[index] ;
	}
	return	null ;
}

XMLDocument& XMLDocument::GetElementAt( int index )
{
	if ( (index >= 0) && (index < m_xmlElements.GetLength()) )
	{
		return	m_xmlElements[index] ;
	}
	return	null ;
}

// サブコンテンツ追加
//////////////////////////////////////////////////////////////////////////////
void XMLDocument::AddElement( XMLDocument& pDoc )
{
	m_xmlElements[sizeof(m_xmlElements)] ::= pDoc ;
}

void XMLDocument::InsertElementAt( int iElementBefore, XMLDocument& pDoc )
{
	XMLDocument&	refNull ;
	if ( iElementBefore < 0 )
	{
		iElementBefore = 0 ;
	}
	else if ( iElementBefore > sizeof(m_xmlElements) )
	{
		iElementBefore = sizeof(m_xmlElements) ;
	}
	m_xmlElements.Insert( iElementBefore, refNull ) ;
	m_xmlElements[iElementBefore] ::= pDoc ;
}

void XMLDocument::AddTextElement( String sText, int type )
{
	XMLDocument&	xmlText = XMLDocument ;
	xmlText.SetText( sText, type ) ;
	m_xmlElements[sizeof(m_xmlElements)] ::= xmlText ;
}

// サブコンテンツ配列取得
//////////////////////////////////////////////////////////////////////////////
XMLDocument&[]& XMLDocument::GetElementsArray( void )
{
	return	m_xmlElements ;
}

// コンテンツ値取得
//////////////////////////////////////////////////////////////////////////////
const String& XMLDocument::GetContentsValue( String strPath ) const
{
	String[]	aNests ;
	strPath.Separate( aNests, "\\" ) ;
	if ( sizeof(strPath) == 0 )
	{
		return	null ;
	}
	const XMLDocument&	pDoc = this ;
	for ( int i = 0; i < sizeof(aNests) - 1; i ++ )
	{
		pDoc ::= pDoc.GetElementTagAs( aNests[i] ) ;
		if ( pDoc === null )
		{
			return	null ;
		}
	}
	String	strName = aNests[sizeof(aNests) - 1] ;
	if ( !pDoc.m_ssoaAttr.IsEmpty( strName ) )
	{
		return	pDoc.m_ssoaAttr[strName] ;
	}
	pDoc ::= pDoc.GetElementTagAs( strName ) ;
	if ( pDoc !== null )
	{
		if ( pDoc.GetElementsCount() == 1 )
		{
			const XMLDocument&
				pElement = pDoc.GetElementAt( 0 ) ;
			if ( pElement !== null )
			{
				if ( (pElement.m_typeDoc == DocumentType::Text)
					|| (pElement.m_typeDoc == DocumentType::CDATA) )
				{
					return	pElement.m_strText ;
				}
			}
		}
	}
	return	null ;
}

String XMLDocument::GetContentsAsString
		( String strPath, String strDefValue ) const
{
	const String&	strValue = GetContentsValue( strPath ) ;
	if ( strValue === null )
	{
		return	strDefValue ;
	}
	return	strValue ;
}

int XMLDocument::GetContentsAsInteger
	( String strPath, int nDefValue ) const
{
	const String&	strValue = GetContentsValue( strPath ) ;
	if ( strValue === null )
	{
		return	nDefValue ;
	}
	return	int(strValue) ;
}

int XMLDocument::GetContentsAsHexInteger
	( String strPath, int nDefValue ) const
{
	const String&	strValue = GetContentsValue( strPath ) ;
	if ( strValue === null )
	{
		return	nDefValue ;
	}
	return	NumberFromHexString( strValue ) ;
}

double XMLDocument::GetContentsAsReal
	( String strPath, double nDefValue ) const
{
	const String&	strValue = GetContentsValue( strPath ) ;
	if ( strValue === null )
	{
		return	nDefValue ;
	}
	return	double(strValue) ;
}

// XMLデータ読み込み
//////////////////////////////////////////////////////////////////////////////
Error XMLDocument::LoadDocument( String path, ParserErrorInterface& perr )
{
	File	file ;
	if ( file.Open
		( path, File::Mode::Read | File::Mode::ShareRead ) != eslErrSuccess )
	{
		return	eslErrFailed ;
	}
	return	ReadDocument( file, perr ) ;
}

Error XMLDocument::ReadDocument( File& file, ParserErrorInterface& perr )
{
	Hash<String>	ssoaDTD ;
	//
	// ファイルを読み込む
	// 冒頭の <?xml ～ ?> を処理（エンコーディング判定）
	//
	String	strXML ;
	String	strLine = file ;
	int		countError = 0 ;
	if ( strLine.NextString() == "<?xml" )
	{
		XMLDocument	xmlDoc ;
		countError +=
			xmlDoc.ParseTagAttributes( strLine, ssoaDTD, perr ) ;
		if ( strLine.Middle(strLine.GetIndex(),2) == "?>" )
		{
			strLine.SeekIndex( strLine.GetIndex() + 2 ) ;
			//
			const String&	strEncoding = xmlDoc.GetAttributeAs( "encoding" ) ;
			if ( strEncoding !== null )
			{
				file.SetCharacterEncoding( strEncoding ) ;
			}
			else
			{
				file.SetCharacterEncoding( "utf-8" ) ;
			}
		}
		else
		{
			strLine.SeekIndex( 0 ) ;
		}
	}
	else
	{
		strLine.SeekIndex( 0 ) ;
	}
	strXML += strLine.Middle( strLine.GetIndex() ) ;
	//
	String	strText ;
	file.Read( strText, file.GetLength() - file.GetPosition() ) ;
	strXML += strText ;
	//
	// ドキュメント全体を処理
	//
	RemoveAllContents() ;
	//
	strXML.SeekIndex( 0 ) ;
	countError += ParseXMLElements( strXML, ssoaDTD, perr ) ;
	if ( countError > 0 )
	{
		return	eslErrFailed ;
	}
	return	eslErrSuccess ;
}

// XMLデータ解釈（要素配列の解釈）
//////////////////////////////////////////////////////////////////////////////
int XMLDocument::ParseXMLElements
	( String& sparsDoc,
		Hash<String>& ssoaDTD, ParserErrorInterface& perr )
{
	int	countError = 0 ;
	while ( !sparsDoc.SeekNext() )
	{
		if ( sparsDoc.Middle( sparsDoc.GetIndex(), 2 ) == "</" )
		{
			// </tag-name> タグ終端
			sparsDoc.SeekIndex( sparsDoc.GetIndex() + 2 ) ;
			//
			String	strTagName = sparsDoc.NextToken() ;
			if ( strTagName != m_strTag )
			{
				perr.OutputError
					( sparsDoc, "<" + m_strTag
						+ "> に対応しない </" + strTagName + "> です" ) ;
				countError ++ ;
			}
			if ( sparsDoc.NextNeedChar( ">" ) != ">" )
			{
				perr.OutputError
					( sparsDoc, "</" + strTagName
									+ " が > で閉じられていません" ) ;
				countError ++ ;
			}
			break ;
		}
		XMLDocument&	xmlElement = XMLDocument ;
		countError +=
			xmlElement.ParseDocument( sparsDoc, ssoaDTD, perr ) ;
		if ( !xmlElement.IsEmpty() )
		{
			AddElement( xmlElement ) ;
		}
	}
	return	countError ;
}

// XMLデータ解釈
//////////////////////////////////////////////////////////////////////////////
int XMLDocument::ParseDocument
	( String& sparsDoc,
		Hash<String>& ssoaDTD, ParserErrorInterface& perr )
{
	RemoveAllContents() ;
	if ( sparsDoc.SeekNext() )
	{
		return	0 ;
	}
	if ( sparsDoc.Middle( sparsDoc.GetIndex(), 2 ) == "</" )
	{
		return	0 ;
	}
	int	countError = 0 ;
	if ( sparsDoc.NextNeedChar( "<" ) == "<" )
	{
		int	wch = sparsDoc.Char( sparsDoc.GetIndex() ) ;
		if ( wch == '!' )
		{
			if ( sparsDoc.Middle( sparsDoc.GetIndex(), 3 ) == "!--" )
			{
				// コメント
				sparsDoc.SeekIndex( sparsDoc.GetIndex() + 3 ) ;
				int	iEndOfComment = sparsDoc.Find( "-->", sparsDoc.GetIndex() ) ;
				if ( iEndOfComment >= sparsDoc.GetIndex() )
				{
					SetText( sparsDoc.Middle
						( sparsDoc.GetIndex(),
								iEndOfComment - sparsDoc.GetIndex() ),
							DocumentType::Comment ) ;
					sparsDoc.SeekIndex( iEndOfComment + 3 ) ;
				}
				else
				{
					perr.OutputError
						( sparsDoc, "<!-- に対応する --> が見つかりません" ) ;
					countError ++ ;
				}
			}
			else if ( sparsDoc.Middle( sparsDoc.GetIndex(), 8 ) == "![CDATA[" )
			{
				// CDATA
				sparsDoc.SeekIndex( sparsDoc.GetIndex() + 8 ) ;
				int	iEndOfCDATA = sparsDoc.Find( "]]>", sparsDoc.GetIndex() ) ;
				if ( iEndOfCDATA >= sparsDoc.GetIndex() )
				{
					SetText( sparsDoc.Middle
						( sparsDoc.GetIndex(),
								iEndOfCDATA - sparsDoc.GetIndex() ),
							DocumentType::CDATA ) ;
					sparsDoc.SeekIndex( iEndOfCDATA + 3 ) ;
				}
				else
				{
					perr.OutputError
						( sparsDoc, "<![CDATA[ に対応する ]]> が見つかりません" ) ;
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
		else if ( wch == '?' )
		{
			// 処理命令 <?tag ～ ?>
			sparsDoc.SeekIndex( sparsDoc.GetIndex() + 1 ) ;
			int	iEndOfTag = sparsDoc.Find( "?>", sparsDoc.GetIndex() ) ;
			if ( iEndOfTag >= sparsDoc.GetIndex() )
			{
				sparsDoc.SeekIndex( iEndOfTag + 2 ) ;
			}
			else
			{
				perr.OutputError
					( sparsDoc, "<? に対応する ?> が見つかりません" ) ;
				countError ++ ;
			}
		}
		else
		{
			// <tag-name attribute-list ... >
			m_typeDoc = DocumentType::Tag ;
			m_strTag = sparsDoc.NextToken() ;
			//
			countError +=
				ParseTagAttributes( sparsDoc, ssoaDTD, perr ) ;
			//
			if ( sparsDoc.NextNeedChar( ">" ) == ">" )
			{
				// 要素を解釈
				countError += ParseXMLElements( sparsDoc, ssoaDTD, perr ) ;
			}
			else if ( sparsDoc.Middle( sparsDoc.GetIndex(), 2 ) == "/>" )
			{
				sparsDoc.SeekIndex( sparsDoc.GetIndex() + 2 ) ;
			}
			else
			{
				perr.OutputError
					( sparsDoc, m_strTag + " タグを閉じる > が見つかりません" ) ;
				countError ++ ;
			}
		}
	}
	else
	{
		// 文字列要素
		int	iNextTag = sparsDoc.Find( "<", sparsDoc.GetIndex() ) ;
		if ( iNextTag < sparsDoc.GetIndex() )
		{
			iNextTag = sparsDoc.GetLength() ;
		}
		String	sparsText =
			sparsDoc.Middle
				( sparsDoc.GetIndex(), iNextTag - sparsDoc.GetIndex() ) ;
		sparsDoc.SeekIndex( iNextTag ) ;
		//
		String	strLine ;
		m_strText = "" ;
		while ( !sparsText.SeekNext() )
		{
			int	iCR = sparsText.Find( "\r", sparsText.GetIndex() ) ;
			int	iLF = sparsText.Find( "\n", sparsText.GetIndex() ) ;
			if ( (iLF >= 0) && (iLF < iCR) )
			{
				iCR = iLF ;
			}
			if ( iCR < sparsText.GetIndex() )
			{
				iCR = sparsText.GetLength() ;
			}
			strLine = sparsText.Middle
				( sparsText.GetIndex(), iCR - sparsText.GetIndex() ) ;
			strLine.TrimRight() ;
			sparsText.SeekIndex( iCR ) ;
			DecodeXMLText( strLine, ssoaDTD ) ;
			m_strText += strLine ;
		}
		m_typeDoc = DocumentType::Text ;
	}
	return	countError ;
}

// <! ... > を解釈（主に読み飛ばす）
//////////////////////////////////////////////////////////////////////////////
int XMLDocument::ParseDocTypeSection
	( String& sparsDoc,
		Hash<String>& ssoaDTD, ParserErrorInterface& perr )
{
	int	countError = 0 ;
	int	countNest = 1 ;
	while ( sparsDoc.GetIndex() < sparsDoc.GetLength() )
	{
		int	wch = sparsDoc.NextChar().Char() ;
		if ( wch == '<' )
		{
			if ( sparsDoc.Char( sparsDoc.GetIndex() ) == '!' )
			{
				if ( sparsDoc.Middle( sparsDoc.GetIndex(), 7 ) == "!ENTITY" )
				{
					sparsDoc.SeekIndex( sparsDoc.GetIndex() + 7 ) ;
					//
					String	strName = sparsDoc.NextString() ;
					if ( strName != ">" )
					{
						String	strValue ;
						sparsDoc.SeekNext() ;
						countError +=
							ParseTagAttributeValue
								( strValue, sparsDoc, ssoaDTD, perr ) ;
						ssoaDTD[strName] = strValue ;
					}
					else
					{
						perr.OutputError
							( sparsDoc, "<!ENTITY> で定義名が見つかりません" ) ;
						countError ++ ;
					}
				}
				countNest ++ ;
			}
		}
		else if ( wch == '>' )
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
int XMLDocument::ParseTagAttributes
	( String& sparsDoc,
		Hash<String>& ssoaDTD, ParserErrorInterface& perr )
{
/*
	return	FastParseTagAttributes
			( sparsDoc, (uint32) sparsDoc.GetIndex(),
					(uint32) sparsDoc.GetLength(), ssoaDTD, perr ) ;
*/
	int	countError = 0 ;
	while ( !sparsDoc.SeekNext() )
	{
		int	wch = sparsDoc.Char( sparsDoc.GetIndex() ) ;
		if ( (wch == '>') || (wch == '?') || (wch == '/') )
		{
			break ;
		}
		String	strName ;
		for ( ; ; )
		{
			String	strNamePart ;
			int		typeToken ;
			strNamePart = sparsDoc.NextToken( typeToken );
			if ( typeToken != 0 )
			{
				if ( strNamePart == "" )
				{
					perr.OutputError
						( sparsDoc,
							"タグ属性解釈中に終端に到達しました" ) ;
				}
				else
				{
					perr.OutputError
						( sparsDoc,
							"タグ属性解釈中に \""
								+ strName + "\" を発見しました" ) ;
				}
				countError ++ ;
				return	countError ;
			}
			strName += strNamePart ;
			//
			int	wchNext = sparsDoc.NextNeedChar( ":=" ).Char() ;
			if ( wchNext == '=' )
			{
				break ;
			}
			else if ( wchNext == ':' )
			{
				strName += ":" ;
			}
			else
			{
				perr.OutputError
					( sparsDoc, "タグ属性に = が見つかりません" ) ;
				countError ++ ;
				return	countError ;
			}
		}
		String	strValue ;
		countError +=
			ParseTagAttributeValue( strValue, sparsDoc, ssoaDTD, perr ) ;
		m_ssoaAttr[strName] = strValue ;
	}
	return	countError ;
}

// 属性値を解釈
//////////////////////////////////////////////////////////////////////////////
int XMLDocument::ParseTagAttributeValue
	( String& strValue, String& sparsDoc,
		Hash<String>& ssoaDTD, ParserErrorInterface& perr )
{
	int	countError = 0 ;
	int	wchQuote = sparsDoc.Char( sparsDoc.GetIndex() ) ;
	if ( (wchQuote == '\"') || (wchQuote == '\'') )
	{
		int	iStart = sparsDoc.GetIndex() + 1 ;
		int	iClose = sparsDoc.Find( wchQuote.Char(), iStart ) ;
		if ( iClose < 0 )
		{
			perr.OutputError
				( sparsDoc, "タグ属性がクォーテーションで閉じられていません" ) ;
			countError ++ ;
			//
			sparsDoc.SeekIndex( sizeof(sparsDoc) ) ;
		}
		else
		{
			strValue = sparsDoc.Middle( iStart, iClose - iStart ) ;
			sparsDoc.SeekIndex( iClose + 1 ) ;
		}
	}
	else
	{
		perr.OutputWarning
			( sparsDoc, "タグ属性がクォーテーションで囲まれていません" ) ;
		strValue = sparsDoc.NextToken() ;
	}
	DecodeXMLText( strValue, ssoaDTD ) ;
	return	countError ;
}

// タグ属性を解釈（> 又は /> まで sparsDoc の指標を移動）（高速版）
//////////////////////////////////////////////////////////////////////////////
int XMLDocument::FastParseTagAttributes
	( String& sparsDoc, uint32 iIndex, uint32 nTagLimit,
		Hash<String>& ssoaDTD, ParserErrorInterface& perr ) naked
{
	const uint16 *	pwDoc = sparsDoc.GetBuffer() ;
	int	countError = 0 ;
	for ( uint32 i = iIndex; i < nTagLimit; i ++ )
	{
		int	wch = pwDoc[i] ;
		if ( (wch == '>') || (wch == '?') || (wch == '/') )
		{
			nTagLimit = i ;
			break ;
		}
	}
	while ( iIndex < nTagLimit )
	{
		//
		// 属性名判定
		//
		uint32	wch ;
		while ( iIndex < nTagLimit )
		{
			wch = pwDoc[iIndex] ;
			if ( wch > ' ' )
			{
				break ;
			}
			iIndex ++ ;
		}
		if ( iIndex >= nTagLimit )
		{
			break ;
		}
		uint32	iFirstName = iIndex ;
		while ( iIndex < nTagLimit )
		{
			wch = pwDoc[iIndex] ;
			if ( (wch < ' ') || (wch == '=') )
			{
				break ;
			}
			iIndex ++ ;
		}
		uint32	iEndName = iIndex ;
		//
		// ＝記号判定
		//
		while ( iIndex < nTagLimit )
		{
			if ( pwDoc[iIndex] == '=' )
			{
				break ;
			}
			iIndex ++ ;
		}
		if ( pwDoc[iIndex ++] != '=' )
		{
			nakedOutputError
				( perr, sparsDoc, "タグ属性に = が見つかりません" ) ;
			countError ++ ;
			break ;
		}
		//
		// 属性値判定
		//
		while ( iIndex < nTagLimit )
		{
			wch = pwDoc[iIndex] ;
			if ( wch > ' ' )
			{
				break ;
			}
			iIndex ++ ;
		}
		if ( (iIndex >= nTagLimit) | ((wch != '\'') & (wch != '\"')) )
		{
			nakedOutputError
				( perr, sparsDoc, "タグ属性がクォーテーションで囲まれていません" ) ;
			countError ++ ;
			break ;
		}
		uint32	wchClose = wch ;
		uint32	iFirstValue = (++ iIndex) ;
		while ( iIndex < nTagLimit )
		{
			wch = pwDoc[iIndex] ;
			if ( wch == wchClose )
			{
				break ;
			}
			iIndex ++ ;
		}
		uint32	iEndValue = (iIndex ++) ;
		//
		// 追加
		//
		String	strName ;
		String	strValue ;
		strName.SetString( pwDoc + iFirstName, iEndName - iFirstName ) ;
		nakedDecodeXMLText
			( strValue, pwDoc + iFirstValue,
						iEndValue - iFirstValue, ssoaDTD ) ;
		//
		nakedAddTagAttributeValue( &strName, &strValue ) ;
	}
	nakedSeekIndex( sparsDoc, nTagLimit ) ;
	return	countError ;
}

// 属性値追加
//////////////////////////////////////////////////////////////////////////////
void XMLDocument::nakedAddTagAttributeValue
	( String * pstrName, String * pstrValue ) naked
{
	AddTagAttributeValue( pstrName, pstrValue ) ;
}

void XMLDocument::AddTagAttributeValue
	( String * pstrName, String * pstrValue )
{
	m_ssoaAttr[*pstrName] = *pstrValue ;
}

// エラー出力
//////////////////////////////////////////////////////////////////////////////
void XMLDocument::nakedOutputError
	( ParserErrorInterface& perr,
			const String& ss, const uint16 * pwErrMsg ) naked
{
	String	strMsg ;
	strMsg.SetString( pwErrMsg ) ;
	perr.OutputError( ss, strMsg ) ;
}

// 指標移動
//////////////////////////////////////////////////////////////////////////////
void XMLDocument::nakedSeekIndex( String& ss, int nIndex ) naked
{
	ss.SeekIndex( nIndex ) ;
}

// XMLデータ書き出し
//////////////////////////////////////////////////////////////////////////////
Error XMLDocument::SaveDocument
	( String path, int nIndent, String encoding ) const
{
	File	file ;
	if ( file.Open( path, File::Mode::Create ) != eslErrSuccess )
	{
		return	eslErrFailed ;
	}
	return	WriteDocument( file, nIndent, encoding ) ;
}

Error XMLDocument::WriteDocument
	( File& file, int nIndent, String encoding ) const
{
	//
	// エンコーディングの指定
	//
	String	strFirstLine = "<?xml version=\"1.0\" encoding=\"" ;
	strFirstLine += encoding ;
	strFirstLine += "\"?>" ;
	file.SetCharacterEncoding( encoding ) ;
	file += strFirstLine ;
	//
	// データ書き出し
	//
	if ( m_typeDoc == DocumentType::Root )
	{
		return	FormatXMLElements( file, nIndent ) ;
	}
	else
	{
		return	FormatDocument( file, nIndent ) ;
	}
}

String XMLDocument::FormatDocumentToString( void ) const
{
	File	fileTemp ;
	fileTemp.CreateMemoryFile() ;
	//
	if ( m_typeDoc == DocumentType::Root )
	{
		FormatXMLElements( fileTemp, 0 ) ;
	}
	else
	{
		FormatDocument( fileTemp, 0 ) ;
	}
	//
	int	nBytes = fileTemp.GetLength() ;
	fileTemp.Seek( 0 ) ;
	//
	String	strText ;
	fileTemp.Read( strText, nBytes ) ;
	return	strText ;
}

// XMLデータ書き出し（要素配列）
//////////////////////////////////////////////////////////////////////////////
Error XMLDocument::FormatXMLElements( File& file, int nIndent ) const
{
	const int	nCount = m_xmlElements.GetLength() ;
	for ( int i = 0; i < nCount; i ++ )
	{
		const XMLDocument&	pElement = m_xmlElements[i] ;
		Error	err = pElement.FormatDocument( file, nIndent ) ;
		if ( err != eslErrSuccess )
		{
			return	err ;
		}
	}
	return	eslErrSuccess ;
}

// XMLデータ書き出し（1つのタグ／文字列）
//////////////////////////////////////////////////////////////////////////////
Error XMLDocument::FormatDocument( File& file, int nIndent ) const
{
	String	strIndent ;
	if ( nIndent > 0 )
	{
		strIndent = "  " * nIndent ;
	}
	if ( m_typeDoc == DocumentType::Tag )
	{
		//
		// タグ出力 <tag ...
		//
		String	strText = strIndent ;
		bool	flagAttr = false ;
		strText += "<" ;
		strText += m_strTag ;
		//
		const int	countAttr = m_ssoaAttr.GetLength() ;
		for ( int iAttr = 0; iAttr < countAttr; iAttr ++ )
		{
			String	strName = m_ssoaAttr.GetTagName( iAttr ) ;
			String	strValue = m_ssoaAttr[iAttr].GetXMLEncoded() ;
			//
			if ( flagAttr && (nIndent >= 0)
				&& (strText.GetLength()
					+ strName.GetLength()
					+ strValue.GetLength() + 5 >= 80) )
			{
				file += strText ;
				strText = strIndent ;
				strText += "  " ;
			}
			strText += " " ;
			strText += strName ;
			strText += "=\"" ;
			strText += strValue ;
			strText += "\"" ;
			flagAttr = true ;
		}
		if ( m_xmlElements.GetLength() == 0 )
		{
			strText += L"/>" ;
			file += strText ;
		}
		else
		{
			bool	fSimpleTextElement = false ;
			if ( m_xmlElements.GetLength() == 1 )
			{
				const XMLDocument&	pElement = m_xmlElements[0] ;
				if ( pElement.m_typeDoc == DocumentType::Text )
				{
					fSimpleTextElement =
						(pElement.m_strText.Find( "\n" ) < 0) ;
				}
			}
			if ( fSimpleTextElement )
			{
				//
				// <tag>text</tag> 形式
				//
				String	strDocText =
					m_xmlElements[0].m_strText.GetXMLEncoded() ;
				//
				strText += ">" ;
				strText += strDocText ;
				strText += "</" ;
				strText += m_strTag ;
				strText += ">" ;
				//
				file += strText ;
			}
			else
			{
				//
				// 複数の要素を持つ場合
				//
				file += strText + ">" ;
				//
				Error	err = FormatXMLElements( file, nIndent + 1 ) ;
				if ( err != eslErrSuccess )
				{
					return	err ;
				}
				strText = strIndent ;
				strText += "</" ;
				strText += m_strTag ;
				strText += ">" ;
				file += strText ;
			}
		}
	}
	else if ( m_typeDoc == DocumentType::Text )
	{
		//
		// 文字列要素出力
		//
		String	strDocText ;
		EncodeXMLText( strDocText, m_strText, nIndent ) ;
		if ( nIndent > 0 )
		{
			strDocText = strIndent + strDocText ;
		}
		file += strDocText ;
	}
	else if ( m_typeDoc == DocumentType::CDATA )
	{
		//
		// CDATA 出力
		//
		String	strCDATA ;
		strCDATA = strIndent ;
		strCDATA += "<![CDATA[" ;
		strCDATA += m_strText ;
		strCDATA += "]]>" ;
		file += strCDATA ;
	}
	else if ( m_typeDoc == DocumentType::Comment )
	{
		//
		// コメント出力
		//
		String	strComment ;
		strComment = strIndent ;
		strComment += "<!--" ;
		strComment += m_strText ;
		strComment += "-->" ;
		file += strComment ;
	}
	else
	{
		return	FormatXMLElements( file, nIndent ) ;
	}
	return	eslErrSuccess ;
}

// 16進数文字列を数値に変換
//////////////////////////////////////////////////////////////////////////////
int XMLDocument::NumberFromHexString( String strHex )
{
	int	nHex = 0 ;
	for ( int i = 0; i < sizeof(strHex); i ++ )
	{
		int	c = strHex.Char(i) ;
		if ( (c >= '0') && (c <= '9') )
		{
			nHex = nHex * 16 + (c - '0') ;
		}
		else if ( (c >= 'A') && (c <= 'F') )
		{
			nHex = nHex * 16 + (c - 'A' + 10) ;
		}
		else if ( (c >= 'a') && (c <= 'f') )
		{
			nHex = nHex * 16 + (c - 'a' + 10) ;
		}
		else
		{
			break ;
		}
	}
	return	nHex ;
}

// 文字列コンテンツのデコード
//////////////////////////////////////////////////////////////////////////////
data XMLDocument::XMLTextEscapeChar<String>
{
	lt = "<",
	gt = ">",
	quot = "\"",
	amp = "&",
	nbsp = " ",
} ;

naked const uint16 *	XMLDocument::m_pszXMLTextEscapeChar[6] =
{
	"lt", "gt", "quot", "amp", "nbsp", NULL
} ;
naked const uint16 *	XMLDocument::m_pszXMLTextEscapeCode = "<>\"& " ;

Error XMLDocument::DecodeXMLText
	( String & strText, Hash<String>& ssoaDTD )
{
	int	iFind = strText.Find( "&" ) ;
	if ( iFind < 0 )
	{
		return	eslErrSuccess ;
	}
	int		wchCode ;
	int		i, iLast = 0 ;
	String	strBuf ;
	String	strToken ;
	do
	{
		strBuf += strText.Middle( iLast, iFind - iLast ) ;
		//
		iLast = ++ iFind ;
		iFind = strText.Find( ";", iFind ) ;
		if ( iFind < 0 )
		{
			iFind = strText.Find( "&", iLast -- ) ;
			if ( iFind < 0 )
			{
				break ;
			}
			continue ;
		}
		//
		strToken = strText.Middle( iLast, iFind - iLast ) ;
		strToken.TrimRight( ) ;
		strToken.TrimLeft( ) ;
		//
		if ( strToken.Char(0) == '#' )
		{
			int	cx = strToken.Char(1) ;
			if ( (cx == 'X') || (cx == 'x') )
			{
				// &#x...; 形式
				strBuf += NumberFromHexString(strToken.Middle(2)).Char() ;
			}
			else
			{
				// &#...; 形式
				strBuf += int(strToken.Middle(1)).Char() ;
			}
		}
		else
		{
			// &...; 形式
			if ( !ssoaDTD.IsEmpty( strToken ) )
			{
				strBuf += ssoaDTD[strToken] ;
			}
			else if ( !XMLDocument::XMLTextEscapeChar.IsEmpty( strToken ) )
			{
				strBuf += XMLDocument::XMLTextEscapeChar[strToken] ;
			}
			else
			{
				iFind = strText.Find( "&", iLast -- ) ;
				continue ;
			}
		}
		iLast = ++ iFind ;
		iFind = strText.Find( "&", iFind ) ;
	}
	while ( iFind >= 0 ) ;
	//
	strText = strBuf + strText.Middle( iLast ) ;
	return	eslErrSuccess ;
}

Error XMLDocument::nakedDecodeXMLText
	( String& strText, const uint16 * pwSrc,
			uint32 nLength, Hash<String>& ssoaDTD ) naked
{
	uint32		nDstLimit = ((nLength + 0xFF) & ~0xFF) + 0xFF ;
	uint16 *	pwDst = strText.LockBuffer( nDstLimit ) ;
	uint32		iSrc = 0 ;
	uint32		iDst = 0 ;
	//
	while ( iSrc < nLength )
	{
		uint32	wch = pwSrc[iSrc ++] ;
		if ( wch == '&' )
		{
			if ( pwSrc[iSrc] == '#' )
			{
				uint32	wchCode = 0 ;
				wch = pwSrc[++ iSrc] ;
				if ( (wch == 'X') | (wch == 'x') )
				{
					// &#x...; 形式
					iSrc ++ ;
					while ( iSrc < nLength )
					{
						wch = pwSrc[iSrc ++] ;
						if ( (wch >= '0') & (wch <= '9') )
						{
							wchCode = (wchCode << 4) | (wch - '0') ;
						}
						else if ( (wch >= 'A') & (wch <= 'F') )
						{
							wchCode = (wchCode << 4) | (wch - ('A' - 10)) ;
						}
						else if ( (wch >= 'a') & (wch <= 'f') )
						{
							wchCode = (wchCode << 4) | (wch - ('a' - 10)) ;
						}
						else
						{
							break ;
						}
					}
				}
				else
				{
					// &#...; 形式
					while ( iSrc < nLength )
					{
						wch = pwSrc[iSrc ++] ;
						if ( (wch >= '0') & (wch <= '9') )
						{
							wchCode = (wchCode * 10) + (wch - '0') ;
						}
						else
						{
							break ;
						}
					}
				}
				if ( iDst >= nDstLimit )
				{
					nDstLimit += 0x100 ;
					strText.UnlockBuffer( iDst ) ;
					pwDst = strText.LockBuffer( nDstLimit ) ;
				}
				pwDst[iDst ++] = (uint16) wchCode ;
				//
				while ( (wch != ';') & (iSrc < nLength) )
				{
					wch = pwSrc[iSrc ++] ;
				}
			}
			else
			{
				// &...; 形式
				uint32	iFirstName = iSrc ;
				uint32	iEndName = iSrc ;
				while ( iSrc < nLength )
				{
					if ( pwSrc[iSrc ++] == ';' )
					{
						iEndName = iSrc - 1 ;
						break ;
					}
				}
				const uint16 *	pwSrcChar = pwSrc + iFirstName ;
				uint32			nCharLen = iEndName - iFirstName ;
				bool			flagProcessed = false ;
				for ( uint32 i = 0; m_pszXMLTextEscapeChar[i] != NULL; i ++ )
				{
					const uint16 *	pwEscChar = m_pszXMLTextEscapeChar[i] ;
					uint32	j ;
					for ( j = 0; j < nCharLen; j ++ )
					{
						if ( pwSrcChar[j] != pwEscChar[j] )
						{
							break ;
						}
					}
					if ( j >= nCharLen )
					{
						if ( iDst >= nDstLimit )
						{
							nDstLimit += 0x100 ;
							strText.UnlockBuffer( iDst ) ;
							pwDst = strText.LockBuffer( nDstLimit ) ;
						}
						pwDst[iDst ++] = m_pszXMLTextEscapeCode[i] ;
						flagProcessed = true ;
						break ;
					}
				}
				if ( !flagProcessed )
				{
					uint16 *	pwMapped =
						nakedGetMappedDTD
							( pwSrcChar, nCharLen, ssoaDTD ) ;
					if ( pwMapped != NULL )
					{
						for ( uint32 i = 0; pwMapped[i] != 0; i ++ )
						{
							if ( iDst >= nDstLimit )
							{
								nDstLimit += 0x100 ;
								strText.UnlockBuffer( iDst ) ;
								pwDst = strText.LockBuffer( nDstLimit ) ;
							}
							pwDst[iDst ++] = pwMapped[i] ;
						}
						delete []	pwMapped ;
					}
				}
			}
		}
		else
		{
			if ( iDst >= nDstLimit )
			{
				nDstLimit += 0x100 ;
				strText.UnlockBuffer( iDst ) ;
				pwDst = strText.LockBuffer( nDstLimit ) ;
			}
			pwDst[iDst ++] = (uint16) wch ;
		}
	}
	//
	strText.UnlockBuffer( iDst ) ;
	return	eslErrSuccess ;
}

uint16 * XMLDocument::nakedGetMappedDTD
	( const uint16 * pwChar, uint32 nLength, Hash<String>& ssoaDTD ) naked
{
	String	strChar ;
	strChar.SetString( pwChar, nLength ) ;
	//
	String *		pstrMapped = GetMappedDTD( &ssoaDTD, &strChar ) ;
	uint32			nSrcLen = (uint32) pstrMapped->GetLength() ;
	uint16 *		pwDst = new uint16[nSrcLen + 1] ;
	const uint16 *	pwSrc = pstrMapped->GetBuffer() ;
	memmove( pwDst, pwSrc, (nSrcLen + 1) * 2 ) ;
	delete	pstrMapped ;
	return	pwDst ;
}

String XMLDocument::GetMappedDTD( Hash<String> * ssoaDTD, String * strChar )
{
	if ( ssoaDTD->IsEmpty( *strChar ) )
	{
		return	"" ;
	}
	return	(*ssoaDTD)[*strChar] ;
}

// 文字列コンテンツのエンコーディング
//////////////////////////////////////////////////////////////////////////////
Error XMLDocument::EncodeXMLText
	( String & strText, String strSrc, int nIndent )
{
	int	iLast = 0 ;
	strText = "" ;
	for ( ; ; )
	{
		int	iLF = strSrc.Find( "\n", iLast ) ;
		if ( iLF < 0 )
		{
			break ;
		}
		iLF ++ ;
		strText += strSrc.Middle(iLast,iLF-iLast).GetXMLEncoded() ;
		iLast = iLF ;
	}
	strText += strSrc.Middle(iLast).GetXMLEncoded() ;
	return	eslErrSuccess ;
}


// エラー出力
//////////////////////////////////////////////////////////////////////////////
void XMLDocument::OutputError
	( const String& ss, String strError )
{
}

void XMLDocument::OutputWarning
	( const String& ss, String strWarning )
{
}

