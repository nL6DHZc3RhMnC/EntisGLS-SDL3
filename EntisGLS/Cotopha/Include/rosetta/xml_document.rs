
//////////////////////////////////////////////////////////////////////////////
// エラー出力インターフェース
//////////////////////////////////////////////////////////////////////////////

class	ParserErrorInterface
{
	public abstract void outputError
			( StringParser ss, String strError ) ;
	public abstract void outputWarning
			( StringParser ss, String strError ) ;
}

class	ParserErrorTracer	extends ParserErrorInterface
{
	class	Log
	{
		public String	m_strError ;
		public String	m_strLine ;
		public int		m_nLineNum ;
	}
	public Log[]	m_logError = new Log[] ;
	public Log[]	m_logWarning = new Log[] ;

	public void outputError
			( StringParser ss, String strError )
	{
		ParserErrorTracer.Log	log = new ParserErrorTracer.Log() ;
		int[]	pLineIndex = new int[1] ;
		int		nIndex = ss.getIndex() ;
		log.m_strLine = strError ;
		log.m_nLineNum = ss.getLineNumberOf( nIndex, pLineIndex ) ;
		ss.seekIndex( pLineIndex[0] ) ;
		log.m_strLine = ss.getLine() ;
		ss.seekIndex( nIndex ) ;
		m_logError.add( log ) ;
	}

	public void outputWarning
			( StringParser ss, String strError )
	{
		ParserErrorTracer.Log	log = new ParserErrorTracer.Log() ;
		int[]	pLineIndex = new int[1] ;
		int		nIndex = ss.getIndex() ;
		log.m_strLine = strError ;
		log.m_nLineNum = ss.getLineNumberOf( nIndex, pLineIndex ) ;
		ss.seekIndex( pLineIndex[0] ) ;
		log.m_strLine = ss.getLine() ;
		ss.seekIndex( nIndex ) ;
		m_logWarning.add( log ) ;
	}

	public int printAllErrors()
	{
		Console	con = System.console() ;
		for ( int i = 0; i < m_logError.length(); i ++ )
		{
			Log	log = m_logError[i] ;
			con.printf( "error(%d):%s\n>%s\n",
						log.m_nLineNum, log.m_strError, log.m_strLine ) ;
		}
		return	m_logError.length() ;
	}

	public int printAllWarnings()
	{
		Console	con = System.console() ;
		for ( int i = 0; i < m_logWarning.length(); i ++ )
		{
			Log	log = m_logWarning[i] ;
			con.printf( "error(%d):%s\n>%s\n",
						log.m_nLineNum, log.m_strError, log.m_strLine ) ;
		}
		return	m_logWarning.length() ;
	}

}


//////////////////////////////////////////////////////////////////////////////
// XML ドキュメント
// <summary>
// 　XML ドキュメントを読み込み、保存、データの取得、変更、追加機能を提供します。<br/>
// （import xml_document.rs）
// </summary>
//////////////////////////////////////////////////////////////////////////////

class	XMLDocument
{
	class	DocumentType
	{
		public static const int	Root	= 0 ;
		public static const int	Tag		= 1 ;
		public static const int	Text	= 2 ;
		public static const int	CDATA	= 3 ;
		public static const int	Comment	= 4 ;
	}

	protected int				m_typeDoc = XMLDocument.DocumentType.Root ;
	protected String			m_strTag ;
	protected String			m_strText ;
	protected HashMap<String>	m_mapAttr = new HashMap<String>() ;
	protected XMLDocument[]		m_xmlElements = new XMLDocument[] ;

	// 構築関数
	public XMLDocument()
	{
	}
	public XMLDocument( XMLDocument xmlSrc )
	{
		copyAllContentsFrom( xmlSrc ) ;
	}

	// 複製
	public void copyAllContentsFrom( XMLDocument xmlSrc )
	{
		removeAllContents() ;
		//
		if ( xmlSrc )
		{
			m_typeDoc = xmlSrc.m_typeDoc ;
			m_strTag = xmlSrc.m_strTag ;
			m_strText = xmlSrc.m_strText ;
			//
			for ( attr in xmlSrc.m_mapAttr )
			{
				m_mapAttr[attr] = xmlSrc.m_mapAttr[attr] ;
			}
			for ( int i = 0; i < xmlSrc.m_xmlElements.length(); i ++ )
			{
				m_xmlElements[i] = new XMLDocument( xmlSrc.m_xmlElements[i] ) ;
			}
		}
	}

	// すべての内容を消去
	public void removeAllContents( void )
	{
		m_typeDoc = XMLDocument.DocumentType.Root ;
		m_strTag = "" ;
		m_strText = "" ;
		m_mapAttr.clear() ;
		m_xmlElements.clear() ;
	}
	public void removeAllAttributes( void )
	{
		m_mapAttr.clear() ;
	}
	public void removeAllElements( void )
	{
		m_xmlElements.clear() ;
	}

	// 空のデータか？
	public const boolean isEmpty( void )
	{
		return	(m_strTag == "")
					&& (m_strText == "")
					&& (m_mapAttr.size() == 0)
					&& (m_xmlElements.length() == 0) ;
	}

	// データ種類取得
	public const int getType( void )
	{
		return	m_typeDoc ;
	}

	// タグ取得
	public const String getTag( void )
	{
		return	m_strTag ;
	}

	// タグ設定
	public void setTag( String strTag )
	{
		m_typeDoc = XMLDocument.DocumentType.Tag ;
		m_strTag = strTag ;
	}

	// テキスト取得
	public const String getText( void )
	{
		return	m_strText ;
	}

	// テキスト設定
	public void setText
		( String strText, int type = XMLDocument.DocumentType.Text )
	{
		m_typeDoc = type ;
		m_strText = strText ;
	}

	// 属性値取得
	public const String getAttributeAs( String strAttrName )
	{
		if ( m_mapAttr[strAttrName] !== null )
		{
			return	m_mapAttr[strAttrName] ;
		}
		return	null ;
	}

	public const String getAttrStringAs( String strAttrName, String strDefValue = "" )
	{
		if ( m_mapAttr[strAttrName] === null )
		{
			return	strDefValue ;
		}
		return	m_mapAttr[strAttrName] ;
	}

	public const long getAttrIntegerAs( String strAttrName, long nDefValue = 0 )
	{
		if ( m_mapAttr[strAttrName] === null )
		{
			return	nDefValue ;
		}
		StringParser	spars = new StringParser() ;
		spars.attachString( m_mapAttr[strAttrName] ) ;
		return	spars.nextInteger() ;
	}

	public const long getAttrHexIntegerAs( String strAttrName, long nDefValue = 0 )
	{
		if ( m_mapAttr[strAttrName] === null )
		{
			return	nDefValue ;
		}
		StringParser	spars = new StringParser() ;
		spars.attachString( m_mapAttr[strAttrName] ) ;
		return	spars.nextInteger( StringParser.numberRadix16 ) ;
	}

	public const long getAttrSymbolizedIntegerAs
		( String strAttrName, HashMap mapPairs, long nDefValue = 0 )
	{
		String	strValue = m_mapAttr[strAttrName] ;
		if ( strValue === null )
		{
			return	nDefValue ;
		}
		if ( mapPairs[strValue] === null )
		{
			return	nDefValue ;
		}
		return	mapPairs[strValue] ;
	}

	public const long getAttrComplexIntegerAs
		( String strAttrName, HashMap mapPairs,
				long nDefValue = 0, String strSeparators = ",|" )
	{
		String	strValue = m_mapAttr[strAttrName] ;
		if ( strValue === null )
		{
			return	nDefValue ;
		}
		StringParser	sparsAttr = new StringParser() ;
		sparsAttr.attachString( strValue ) ;
		//
		long	nValue = 0 ;
		while ( sparsAttr.passSpace() )
		{
			String	strToken = sparsAttr.getToken() ;
			if ( mapPairs[strToken] !== null )
			{
				nValue |= (int) mapPairs[strToken] ;
			}
			if ( strSeparators != null )
			{
				sparsAttr.hasToComeChar( strSeparators ) ;
			}
		}
		return	nValue ;
	}

	public const double getAttrRealAs( String strAttrName, double nDefValue = 0.0 )
	{
		if ( m_mapAttr[strAttrName] === null )
		{
			return	nDefValue ;
		}
		StringParser	spars = new StringParser() ;
		spars.attachString( m_mapAttr[strAttrName] ) ;
		return	spars.nextRealNumber() ;
	}

	// 属性値設定
	public void setAttributeAs( String strAttrName, String strValue )
	{
		m_mapAttr[strAttrName] = strValue ;
	}

	public void setAttrIntegerAs( String strAttrName, long nValue )
	{
		m_mapAttr[strAttrName] = (String) nValue ;
	}

	public void setAttrHexIntegerAs( String strAttrName, long nValue )
	{
		m_mapAttr[strAttrName] = String.format( "%16X", nValue ) ;
	}

	public void setAttrSymbolizedIntegerAs
			( String strAttrName, HashMap mapPairs, long nValue )
	{
		int	nCount = mapPairs.size() ;
		for ( int i = 0; i < nCount; i ++ )
		{
			if ( mapPairs[i] == nValue )
			{
				m_mapAttr[strAttrName] = mapPairs.keyAt( i ) ;
				return ;
			}
		}
		m_mapAttr.remove( strAttrName ) ;
	}

	public boolean setAttrComplexIntegerAs
			( String strAttrName, HashMap mapPairs,
					long nValue, String strSeparators = " " )
	{
		String	strValues = "" ;
		int	nCount = mapPairs.size() ;
		for ( int i = 0; i < nCount; i ++ )
		{
			if ( mapPairs[i] == 0 )
			{
				if ( nValue == 0 )
				{
					strValues = mapPairs.keyAt( i ) ;
					break ;
				}
			}
			else if ( (nValue & mapPairs[i]) == mapPairs[i] )
			{
				if ( strValues != "" )
				{
					strValues += strSeparators ;
				}
				strValues += mapPairs.keyAt( i ) ;
				//
				nValue &= ~ (int) mapPairs[i] ;
				if ( nValue == 0 )
				{
					break ;
				}
			}
		}
		m_mapAttr[strAttrName] = strValues ;
		return	(nValue == 0) ;
	}

	public void setAttrRealAs( String strAttrName, double nValue )
	{
		m_mapAttr[strAttrName] = (String) nValue ;
	}

	// 属性配列取得
	public const HashMap getAttributes( void )
	{
		return	m_mapAttr ;
	}

	// 属性総数取得
	public const int getAttributeCount( void )
	{
		return	m_mapAttr.size() ;
	}

	// 属性名取得
	public const String getAttributeNameAt( int index )
	{
		return	m_mapAttr.keyAt( index ) ;
	}

	// 属性値取得
	public const String getAttributeValueAt( int index )
	{
		return	m_mapAttr[index] ;
	}

	// 要素検索
	public const int findElement
			( int typeDoc, String strTag = "", int iFirst = 0 )
	{
		const int	nCount = m_xmlElements.length() ;
		for ( int i = iFirst; i < nCount; i ++ )
		{
			XMLDocument	xmlDoc = m_xmlElements[i] ;
			if ( (xmlDoc.m_typeDoc == typeDoc)
				&& (xmlDoc.m_strTag == strTag) )
			{
				return	i ;
			}
		}
		return	-1 ;
	}

	public const int findElementTag( String strTag, int iFirst = 0 )
	{
		return	findElement( XMLDocument.DocumentType.Tag, strTag, iFirst ) ;
	}

	// サブタグ取得
	public const XMLDocument getElementAs
			( int typeDoc, String strTag = "", int iFirst = 0 )
	{
		return	getElementAt( findElement( typeDoc, strTag, iFirst ) ) ;
	}

	public const XMLDocument getElementTagAs( String strTag = "", int iFirst = 0 )
	{
		return	getElementAt( findElementTag( strTag, iFirst ) ) ;
	}

	public const String getTextElement()
	{
		XMLDocument xmlText = getElementAs( XMLDocument.DocumentType.Text ) ;
		if ( xmlText == null )
		{
			xmlText = getElementAs( XMLDocument.DocumentType.CDATA ) ;
		}
		if ( xmlText != null )
		{
			return	xmlText.getText() ;
		}
		return	null ;
	}

	// サブタグ生成
	public XMLDocument createElementAs
			( int typeDoc, String strTag = "", int iFirst = 0 )
	{
		int	i = findElement( typeDoc, strTag, iFirst ) ;
		if ( i >= 0 )
		{
			return	m_xmlElements[i] ;
		}
		XMLDocument	xmlDoc = new XMLDocument() ;
		xmlDoc.m_typeDoc = typeDoc ;
		xmlDoc.m_strTag = strTag ;
		addElement( xmlDoc ) ;
		return	xmlDoc ;
	}

	public XMLDocument createElementTagAs( String strTag, int iFirst = 0 )
	{
		return	createElementAs( XMLDocument.DocumentType.Tag, strTag, iFirst ) ;
	}

	// サブコンテンツ数取得
	public const int getElementsCount( void )
	{
		return	m_xmlElements.length() ;
	}

	// サブコンテンツ取得
	public const XMLDocument getElementAt( int index )
	{
		if ( (index >= 0) && (index < m_xmlElements.length()) )
		{
			return	m_xmlElements[index] ;
		}
		return	null ;
	}

	// サブコンテンツ追加
	public void addElement( XMLDocument xmlDoc )
	{
		m_xmlElements.add( xmlDoc ) ;
	}

	public void insertElementAt( int iElementBefore, XMLDocument xmlDoc )
	{
		if ( iElementBefore < 0 )
		{
			iElementBefore = 0 ;
		}
		else if ( iElementBefore > m_xmlElements.length() )
		{
			iElementBefore = m_xmlElements.length() ;
		}
		m_xmlElements.add( iElementBefore, xmlDoc ) ;
	}

	public void addTextElement
		( String sText, int type = XMLDocument.DocumentType.Text )
	{
		XMLDocument	xmlText = new XMLDocument() ;
		xmlText.setText( sText, type ) ;
		m_xmlElements.add( xmlText ) ;
	}

	// サブコンテンツ配列取得
	public XMLDocument[] getElementsArray( void )
	{
		return	m_xmlElements ;
	}

	// コンテンツ値（属性 or 要素）取得
	public const XMLDocument getContentsElement( String strPath )
	{
		StringParser	spars = new StringParser() ;
		spars.attachString( strPath ) ;
		//
		XMLDocument	xmlDoc = this ;
		String		strName ;
		char[]		pGetClosed = new char[1] ;
		for ( ; ; )
		{
			strName = spars.getEnclosedString( '\\', 0, pGetClosed ) ;
			if ( pGetClosed[0] == 0 )
			{
				break ;
			}
			xmlDoc = xmlDoc.getElementTagAs( strName ) ;
			if ( xmlDoc == null )
			{
				return	null ;
			}
		}
		return	xmlDoc.getElementTagAs( strName ) ;
	}

	public XMLDocument createContentsElement( String strPath )
	{
		StringParser	spars = new StringParser() ;
		spars.attachString( strPath ) ;
		//
		XMLDocument	xmlDoc = this ;
		String		strName ;
		char[]		pGetClosed = new char[1] ;
		for ( ; ; )
		{
			strName = spars.getEnclosedString( '\\', 0, pGetClosed ) ;
			if ( pGetClosed[0] == 0 )
			{
				break ;
			}
			xmlDoc = xmlDoc.createElementTagAs( strName ) ;
		}
		return	xmlDoc.createElementTagAs( strName ) ;
	}

	public const String getContentsValue( String strPath )
	{
		StringParser	spars = new StringParser() ;
		spars.attachString( strPath ) ;
		//
		XMLDocument	xmlDoc = this ;
		String		strName ;
		char[]		pGetClosed = new char[1] ;
		for ( ; ; )
		{
			strName = spars.getEnclosedString( '\\', 0, pGetClosed ) ;
			if ( pGetClosed[0] == 0 )
			{
				break ;
			}
			xmlDoc = xmlDoc.getElementTagAs( strName ) ;
			if ( xmlDoc == null )
			{
				return	null ;
			}
		}
		if ( xmlDoc.m_mapAttr[strName] !== null )
		{
			return	xmlDoc.m_mapAttr[strName] ;
		}
		xmlDoc = xmlDoc.getElementTagAs( strName ) ;
		if ( xmlDoc !== null )
		{
			if ( xmlDoc.getElementsCount() == 1 )
			{
				XMLDocument	xmlElement = xmlDoc.getElementAt( 0 ) ;
				if ( (xmlElement !== null)
					&& ((xmlElement.m_typeDoc == XMLDocument.DocumentType.Text)
						|| (xmlElement.m_typeDoc == XMLDocument.DocumentType.CDATA)) )
				{
					return	xmlElement.m_strText ;
				}
			}
		}
		return	null ;
	}

	public const String getContentsAsString( String strPath, String strDefValue = "" )
	{
		String	strValue = getContentsValue( strPath ) ;
		if ( strValue === null )
		{
			return	strDefValue ;
		}
		return	strValue ;
	}

	public const long getContentsAsInteger( String strPath, long nDefValue = 0 )
	{
		String	strValue = getContentsValue( strPath ) ;
		if ( strValue === null )
		{
			return	nDefValue ;
		}
		StringParser	spars = new StringParser() ;
		spars.attachString( strValue ) ;
		return	spars.nextInteger() ;
	}

	public const long getContentsAsHexInteger( String strPath, long nDefValue = 0 )
	{
		String	strValue = getContentsValue( strPath ) ;
		if ( strValue === null )
		{
			return	nDefValue ;
		}
		StringParser	spars = new StringParser() ;
		spars.attachString( strValue ) ;
		return	spars.nextInteger( StringParser.numberRadix16 ) ;
	}

	public const double getContentsAsReal( String strPath, double nDefValue = 0.0 )
	{
		String	strValue = getContentsValue( strPath ) ;
		if ( strValue === null )
		{
			return	nDefValue ;
		}
		StringParser	spars = new StringParser() ;
		spars.attachString( strValue ) ;
		return	spars.nextRealNumber() ;
	}

	// XML データ読み込み
	public boolean loadDocument( String path, ParserErrorInterface perr )
	{
		InputStream	is = null ;
		try
		{
			is = new InputStream( path ) ;
		}
		catch ( Exception e )
		{
			return	false ;
		}
		return	readDocument( is, perr ) ;
	}

	public boolean readDocument( InputStream file, ParserErrorInterface perr )
	{
		HashMap	mapDTD = new HashMap( String ) ;
		//
		// ファイルを読み込む
		// 冒頭の <?xml ～ ?> を処理（エンコーディング判定）
		//
		file.mark( 0x10000 ) ;
		//
		StringParser	spars = new StringParser() ;
		spars.attachString( file.readLine() ) ;
		//
		int	countError = 0 ;
		int	encoding = String.encodingUnknown ;
		if ( spars.hasToComeString( "<?xml" ) )
		{
			XMLDocument	xmlDoc = new XMLDocument() ;
			countError +=
				xmlDoc.parseTagAttributes( spars, mapDTD, perr ) ;
			if ( spars.subString( spars.getIndex(), 2 ) == "?>" )
			{
				spars.seekIndex( spars.getIndex() + 2 ) ;
				//
				String	strEncoding = xmlDoc.getAttributeAs( "encoding" ) ;
				if ( strEncoding !== null )
				{
					encoding = String.getEncodingType( strEncoding ) ;
				}
			}
			else
			{
				file.reset() ;
			}
		}
		else
		{
			file.reset() ;
		}
		//
		spars.readTextFile( file, encoding ) ;
		//
		// ドキュメント全体を処理
		//
		removeAllContents() ;
		//
		countError += parseXMLElements( spars, mapDTD, perr ) ;
		//
		return	(countError == 0) ;
	}

	// XML データ解釈（配列要素の解釈）
	public int parseXMLElements
		( StringParser sparsDoc, HashMap mapDTD, ParserErrorInterface perr )
	{
		int	countError = 0 ;
		while ( sparsDoc.passSpace() )
		{
			if ( sparsDoc.subString( sparsDoc.getIndex(), 2 ) == "</" )
			{
				// </tag-name> タグ終端
				sparsDoc.seekIndex( sparsDoc.getIndex() + 2 ) ;
				//
				String	strTagName = sparsDoc.getToken() ;
				if ( strTagName != m_strTag )
				{
					perr.outputError
						( sparsDoc, "<" + m_strTag
							+ "> に対応していない </" + strTagName + "> です" ) ;
					countError ++ ;
				}
				if ( sparsDoc.hasToComeChar( ">" ) != '>' )
				{
					perr.outputError
						( sparsDoc, "</" + strTagName
										+ " が > で閉じられていません" ) ;
					countError ++ ;
				}
				break ;
			}
			XMLDocument	xmlElement = new XMLDocument() ;
			countError +=
				xmlElement.parseDocument( sparsDoc, mapDTD, perr ) ;
			if ( !xmlElement.isEmpty() )
			{
				addElement( xmlElement ) ;
			}
		}
		return	countError ;
	}

	// XML データ解釈
	public int parseDocument
		( StringParser sparsDoc, HashMap mapDTD, ParserErrorInterface perr )
	{
		removeAllContents() ;
		if ( !sparsDoc.passSpace() )
		{
			return	0 ;
		}
		if ( sparsDoc.subString( sparsDoc.getIndex(), 2 ) == "</" )
		{
			return	0 ;
		}
		int	countError = 0 ;
		if ( sparsDoc.hasToComeChar( "<" ) == '<' )
		{
			char	wch = sparsDoc.currentCharacter() ;
			if ( wch == '!' )
			{
				if ( sparsDoc.subString( sparsDoc.getIndex(), 3 ) == "!--" )
				{
					// コメント
					sparsDoc.seekIndex( sparsDoc.getIndex() + 3 ) ;
					sparsDoc.markIndex() ;
					if ( sparsDoc.seekString( "-->" ) )
					{
						setText
							( sparsDoc.subStringFromMark(),
								XMLDocument.DocumentType.Comment ) ;
						sparsDoc.seekIndex( sparsDoc.getIndex() + 3 ) ;
					}
					else
					{
						perr.outputError
							( sparsDoc, "<!-- に対応する --> が見つかりません" ) ;
						countError ++ ;
					}
				}
				else if ( sparsDoc.subString( sparsDoc.getIndex(), 8 ) == "![CDATA[" )
				{
					// CDATA
					sparsDoc.seekIndex( sparsDoc.getIndex() + 8 ) ;
					sparsDoc.markIndex() ;
					if ( sparsDoc.seekString( "]]>" ) )
					{
						setText
							( sparsDoc.subStringFromMark(),
								XMLDocument.DocumentType.CDATA ) ;
						sparsDoc.seekIndex( sparsDoc.getIndex() + 3 ) ;
					}
					else
					{
						perr.outputError
							( sparsDoc, "<![CDATA[ に対応する ]]> が見つかりません" ) ;
						countError ++ ;
					}
				}
				else
				{
					// <!DOCTYPE ～ >
					countError +=
						parseDocTypeSection( sparsDoc, mapDTD, perr ) ;
				}
			}
			else if ( wch == '?' )
			{
				// 処理命令 <?tag ～ ?>
				sparsDoc.seekIndex( sparsDoc.getIndex() + 1 ) ;
				if ( sparsDoc.seekString( "?>" ) )
				{
					sparsDoc.seekIndex( sparsDoc.getIndex() + 2 ) ;
				}
				else
				{
					perr.outputError
						( sparsDoc, "<? に対応する ?> が見つかりません" ) ;
					countError ++ ;
				}
			}
			else
			{
				// <tag-name attribute-list ... >
				m_typeDoc = XMLDocument.DocumentType.Tag ;
				m_strTag = sparsDoc.getToken() ;
				//
				countError +=
					parseTagAttributes( sparsDoc, mapDTD, perr ) ;
				//
				if ( sparsDoc.hasToComeChar( ">" ) == '>' )
				{
					// 要素を解釈
					countError += parseXMLElements( sparsDoc, mapDTD, perr ) ;
				}
				else if ( sparsDoc.subString( sparsDoc.getIndex(), 2 ) == "/>" )
				{
					sparsDoc.seekIndex( sparsDoc.getIndex() + 2 ) ;
				}
				else
				{
					perr.outputError
						( sparsDoc, "タグを閉じる > が見つかりません" ) ;
					countError ++ ;
				}
			}
		}
		else
		{
			// 文字列要素
			sparsDoc.markIndex() ;
			if ( !sparsDoc.seekString( "<" ) )
			{
				sparsDoc.seekIndex( sparsDoc.getLength() ) ;
			}
			String	strText = sparsDoc.subStringFromMark() ;
			//
			m_strText = "" ;
			int	iNext = 0 ;
			while ( iNext < strText.length() )
			{
				int	iCR = strText.indexOf( "\r", iNext ) ;
				int	iLF = strText.indexOf( "\n", iNext ) ;
				if ( (iLF >= 0) && (iLF < iCR) )
				{
					iCR = iLF ;
				}
				if ( iCR < iNext )
				{
					iCR = strText.length() ;
				}
				String	strLine = strText.substring( iNext, iCR ) ;
				strLine.trim() ;
				iNext = iCR + 1 ;
				m_strText += XMLDocument.decodeXMLText( strLine, mapDTD ) ;
			}
			m_typeDoc = XMLDocument.DocumentType.Text ;
		}
		return	countError ;
	}

	// <! ... > を解釈（主に読み飛ばす）
	protected int parseDocTypeSection
		( StringParser sparsDoc,
			HashMap mapDTD, ParserErrorInterface perr )
	{
		int	countError = 0 ;
		int	countNest = 1 ;
		while ( sparsDoc.getIndex() < sparsDoc.getLength() )
		{
			char	wch = sparsDoc.getCharacter() ;
			if ( wch == '<' )
			{
				if ( sparsDoc.currentCharacter() == '!' )
				{
					if ( sparsDoc.subString( sparsDoc.getIndex(), 7 ) == "!ENTITY" )
					{
						sparsDoc.seekIndex( sparsDoc.getIndex() + 7 ) ;
						//
						String	strName = sparsDoc.getString() ;
						if ( strName != ">" )
						{
							String	strValue =
								parseTagAttributeValue( sparsDoc, mapDTD, perr ) ;
							if ( strValue === null )
							{
								countError ++ ;
							}
							mapDTD[strName] = strValue ;
						}
						else
						{
							perr.outputError
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
	public int parseTagAttributes
		( StringParser sparsDoc,
			HashMap mapDTD, ParserErrorInterface perr )
	{
		int	countError = 0 ;
		while ( sparsDoc.passSpace() )
		{
			char	wch = sparsDoc.currentCharacter() ;
			if ( (wch == '>') | (wch == '?') | (wch == '/') )
			{
				break ;
			}
			String	strName = "" ;
			for ( ; ; )
			{
				String	strNamePart ;
				int[]	typeToken = new int[1] ;
				strNamePart = sparsDoc.getToken( typeToken );
				if ( typeToken[0] != StringParser.tokenNormal )
				{
					if ( strNamePart == "" )
					{
						perr.outputError
							( sparsDoc,
								"タグ属性解釈中に終端に到達しました" ) ;
					}
					else
					{
						perr.outputError
							( sparsDoc,
								"タグ属性解釈中に \""
									+ strName + "\" を発見しました" ) ;
					}
					countError ++ ;
					return	countError ;
				}
				strName += strNamePart ;
				//
				char	wchNext = sparsDoc.hasToComeChar( ":=" ) ;
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
					perr.outputError
						( sparsDoc, "タグ属性に = が見つかりません" ) ;
					countError ++ ;
					return	countError ;
				}
			}
			String	strValue =
				parseTagAttributeValue( sparsDoc, mapDTD, perr ) ;
			if ( strValue === null )
			{
				countError ++ ;
			}
			m_mapAttr[strName] = strValue ;
		}
		return	countError ;
	}

	// 属性値を解釈
	public String parseTagAttributeValue
		( StringParser sparsDoc,
			HashMap mapDTD, ParserErrorInterface perr )
	{
		char	wchQuote = sparsDoc.currentCharacter() ;
		String	strValue ;
		if ( (wchQuote == '\"') | (wchQuote == '\'') )
		{
			sparsDoc.getCharacter() ;
			//
			char[]	wchClosed = new char[1] ;
			strValue = sparsDoc.getEnclosedString
					( wchQuote,
						(StringParser.ctrlNoEscInQuote
							| StringParser.ctrlNoEscInDQuote), wchClosed ) ;
			if ( wchClosed[0] != wchQuote )
			{
				perr.outputError
					( sparsDoc, "タグ属性がクォーテーションで閉じられていません" ) ;
				return	null ;
			}
		}
		else
		{
			perr.outputWarning
				( sparsDoc, "タグ属性がクォーテーションで囲まれていません" ) ;
			strValue = sparsDoc.getToken() ;
		}
		strValue = XMLDocument.decodeXMLText( strValue, mapDTD ) ;
		return	strValue ;
	}

	// XMLデータ書き出し
	public const boolean saveDocument
		( String strFilePath,
				int nIndent = 0, int encoding = String.encodingUTF8 )
	{
		OutputStream	file = null ;
		try
		{
			file = new OutputStream( strFilePath ) ;
		}
		catch ( Exception e )
		{
			return	false ;
		}
		return	writeDocument( file, nIndent, encoding ) ;
	}

	public const boolean writeDocument
		( OutputStream file,
				int nIndent = 0, int encoding = String.encodingUTF8 )
	{
		//
		// エンコーディングの指定
		//
		String	strFirstLine = "<?xml version=\"1.0\" encoding=\"" ;
		strFirstLine += String.getEncodingName( encoding ) ;
		strFirstLine += "\"?>\r\n" ;
		//
		Uint8Pointer	bin = strFirstLine.encodeTo( encoding ) ;
		file.write( bin, 0, bin.getBytes() ) ;
		//
		// データ書き出し
		//
		if ( m_typeDoc == XMLDocument.DocumentType.Root )
		{
			return	formatXMLElements( file, nIndent, encoding ) ;
		}
		else
		{
			return	formatDocument( file, nIndent, encoding ) ;
		}
	}

	public const String formatDocumentToString( int nIndent = 0 )
	{
		SmartBufferFile	sbuf = new SmartBufferFile() ;
		if ( m_typeDoc == XMLDocument.DocumentType.Root )
		{
			if ( !formatXMLElements
				( sbuf.getOutputStream(), nIndent, String.encodingUTF8 ) )
			{
				return	null ;
			}
		}
		else
		{
			if ( !formatDocument
				( sbuf.getOutputStream(), nIndent, String.encodingUTF8 ) )
			{
				return	null ;
			}
		}
		StringParser	sparsXML = new StringParser() ;
		sbuf.seek( 0 ) ;
		sparsXML.readTextFile( sbuf, String.encodingUTF8 ) ;
		return	sparsXML.toString() ;
	}

	// XMLデータ書き出し（要素配列）
	//////////////////////////////////////////////////////////////////////////////
	public const boolean formatXMLElements
		( OutputStream file, int nIndent, int encoding )
	{
		const int	nCount = m_xmlElements.length() ;
		for ( int i = 0; i < nCount; i ++ )
		{
			XMLDocument	pElement = m_xmlElements[i] ;
			if ( pElement !== null )
			{
				if ( !pElement.formatDocument( file, nIndent, encoding ) )
				{
					return	false ;
				}
			}
		}
		return	true ;
	}

	// XMLデータ書き出し（1つのタグ／文字列）
	//////////////////////////////////////////////////////////////////////////////
	public const boolean formatDocument
		( OutputStream file,
				int nIndent = -1, int encoding = String.encodingUTF8 )
	{
		String	strIndent = "" ;
		for ( int i = 0; i < nIndent; i ++ )
		{
			strIndent += "\t" ;
		}
		if ( m_typeDoc == XMLDocument.DocumentType.Tag )
		{
			//
			// タグ出力 <tag ...
			//
			String	strText = strIndent ;
			boolean	flagAttr = false ;
			strText += "<" ;
			strText += m_strTag ;
			//
			const int	countAttr = m_mapAttr.size() ;
			for ( int iAttr = 0; iAttr < countAttr; iAttr ++ )
			{
				String	pstrName = m_mapAttr.keyAt( iAttr ) ;
				String	pstrValue = m_mapAttr[pstrName] ;
				if ( (pstrName !== null) & (pstrValue !== null) )
				{
					String	strValue =
						XMLDocument.encodeXMLText( pstrValue ) ;
					//
					if ( flagAttr && (nIndent >= 0)
						&& (strText.length()
							+ pstrName.length()
							+ strValue.length() + 5 >= 80 ) )
					{
						strText += "\r\n" ;
						//
						Uint8Pointer	bin = strText.encodeTo( encoding ) ;
						file.write( bin, 0, bin.getBytes() ) ;
						//
						strText = strIndent + "  " ;
					}
					strText += " " + pstrName + "=\"" + strValue + "\"" ;
					flagAttr = true ;
				}
			}
			if ( m_xmlElements.length() == 0 )
			{
				strText += "/>\r\n" ;
				//
				Uint8Pointer	bin = strText.encodeTo( encoding ) ;
				file.write( bin, 0, bin.getBytes() ) ;
			}
			else
			{
				boolean	fSimpleTextElement = false ;
				if ( m_xmlElements.length() == 1 )
				{
					XMLDocument	pElement = m_xmlElements[0] ;
					if ( (pElement !== null)
						&& (pElement.m_typeDoc == XMLDocument.DocumentType.Text) )
					{
						fSimpleTextElement =
							(pElement.m_strText.indexOf( "\n" ) < 0) ;
					}
				}
				if ( fSimpleTextElement )
				{
					//
					// <tag>text</tag> 形式
					//
					String	strDocText =
						XMLDocument.encodeXMLText( m_xmlElements[0].m_strText ) ;
					//
					strText += ">" + strDocText + "</" + m_strTag + ">\r\n" ;
					//
					Uint8Pointer	bin = strText.encodeTo( encoding ) ;
					file.write( bin, 0, bin.getBytes() ) ;
				}
				else
				{
					//
					// 複数の要素を持つ場合
					//
					strText += ">\r\n" ;
					//
					Uint8Pointer	bin = strText.encodeTo( encoding ) ;
					file.write( bin, 0, bin.getBytes() ) ;
					//
					if ( !formatXMLElements( file, nIndent + 1, encoding ) )
					{
						return	false ;
					}
					strText = strIndent + "</" + m_strTag + ">\r\n" ;
					bin = strText.encodeTo( encoding ) ;
					file.write( bin, 0, bin.getBytes() ) ;
				}
			}
		}
		else if ( m_typeDoc == XMLDocument.DocumentType.Text )
		{
			//
			// 文字列要素出力
			//
			String	strDocText =
				XMLDocument.encodeXMLText( m_strText, nIndent ) ;
			if ( nIndent > 0 )
			{
				strDocText = strIndent + strDocText ;
			}
			strDocText += "\r\n" ;
			//
			Uint8Pointer	bin = strDocText.encodeTo( encoding ) ;
			file.write( bin, 0, bin.getBytes() ) ;
		}
		else if ( m_typeDoc == XMLDocument.DocumentType.CDATA )
		{
			//
			// CDATA 出力
			//
			String	strCDATA ;
			strCDATA = "<![CDATA[" ;
			if ( m_strText.indexOf( "]]>" ) >= 0 )
			{
				HashMap	map = new HashMap( String ) ;
				map["]]>"] = "]]&gt;" ;
				strCDATA += m_strText.mappingFilter( map ) ;
			}
			else
			{
				strCDATA += m_strText ;
			}
			strCDATA += "]]>\r\n" ;
			//
			Uint8Pointer	bin = strCDATA.encodeTo( encoding ) ;
			file.write( bin, 0, bin.getBytes() ) ;
		}
		else if ( m_typeDoc == XMLDocument.DocumentType.Comment )
		{
			//
			// コメント出力
			//
			String	strComment ;
			strComment = "<!--" ;
			if ( m_strText.indexOf( "-->" ) >= 0 )
			{
				HashMap	map = new HashMap( String ) ;
				map["-->"] = "--&gt;" ;
				strComment += m_strText.mappingFilter( map ) ;
			}
			else
			{
				strComment += m_strText ;
			}
			strComment += "-->\r\n" ;
			//
			Uint8Pointer	bin = strComment.encodeTo( encoding ) ;
			file.write( bin, 0, bin.getBytes() ) ;
		}
		else
		{
			return	formatXMLElements( file, nIndent, encoding ) ;
		}
		return	true ;
	}

	public static const String[]	m_strSpecCharEncText =
	[
		"lt", "gt", "quot", "amp", "nbsp"
	] ;
	public static const String	m_strSpecCharDecChar = "<>\"&\xa0" ;

	// 文字列コンテンツのデコード
	public static String decodeXMLText( String strText, HashMap mapDTD )
	{
		int	iFind = strText.indexOf( "&" ) ;
		if ( iFind < 0 )
		{
			return	strText ;
		}
		char[]	wchCode = new char[1] ;
		int		iLast = 0 ;
		String	strBuf = "" ;
		String	strToken ;
		do
		{
			strBuf += strText.substring( iLast, iFind ) ;
			//
			iLast = ++ iFind ;
			iFind = strText.indexOf( ";", iFind ) ;
			if ( iFind < 0 )
			{
				iFind = strText.indexOf( "&", iLast -- ) ;
				continue ;
			}
			//
			strToken = strText.substring( iLast, iFind ).trim() ;
			//
			if ( strToken.charCodeAt(0) == '#' )
			{
				StringParser	sparsCode = new StringParser() ;
				sparsCode.attachString( strToken ) ;
				sparsCode.seekIndex( 1 ) ;
				//
				if ( sparsCode.hasToComeChar( "Xx" ) != 0 )
				{
					// &#x...; 形式
					wchCode[0] = (char) sparsCode.nextInteger
										( StringParser.numberRadix16 ) ;
				}
				else
				{
					// &#...; 形式
					wchCode[0] = (char) sparsCode.nextInteger
										( StringParser.numberRadix10 ) ;
				}
				strBuf += new String( wchCode ) ;
			}
			else
			{
				// &...; 形式
				String	pstrEntity = mapDTD[strToken] ;
				if ( pstrEntity !== null )
				{
					strBuf += pstrEntity ;
				}
				else
				{
					int	nCount = XMLDocument.m_strSpecCharEncText.length() ;
					int	i ;
					for ( i = 0; i < nCount; i ++ )
					{
						if ( strToken == XMLDocument.m_strSpecCharEncText[i] )
						{
							break ;
						}
					}
					if ( i < nCount )
					{
						strBuf += XMLDocument.m_strSpecCharDecChar.charAt(i) ;
					}
					else
					{
						iFind = strText.indexOf( "&", iLast -- ) ;
						continue ;
					}
				}
			}
			iLast = ++ iFind ;
			iFind = strText.indexOf( "&", iFind ) ;
		}
		while ( iFind >= 0 ) ;
		//
		return	strBuf + strText.substring( iLast ) ;
	}

	// 文字列コンテンツのエンコーディング
	public static String encodeXMLText( String strSrc, int nIndent = -1 )
	{
		String	strText = "" ;
		//
		const int	lenSrc = strSrc.length() ;
		int			iLast = 0 ;
		char[]		aChar = new char[1] ;
		for ( int i = 0; i < lenSrc; i ++ )
		{
			char	wch = strSrc.charCodeAt(i) ;
			if ( wch <= ' ' )
			{
				strText += strSrc.substring( iLast, i ) ;
				strText += "&#" + wch.toString() + ";" ;
				//
				if ( (wch == '\n') && (i + 1 < lenSrc) )
				{
					if ( nIndent >= 0 )
					{
						strText += "\r\n" ;
						for ( int j = 0; j < nIndent; j ++ )
						{
							strText += "\t" ;
						}
					}
				}
				iLast = i + 1 ;
			}
			else if ( wch < 0x80 )
			{
				aChar[0] = wch ;
				//
				int	j = XMLDocument.m_strSpecCharDecChar.
										indexOf( new String( aChar ) ) ;
				if ( j >= 0 )
				{
					strText += strSrc.substring( iLast, i ) ;
					strText += "&" ;
					strText +=
						XMLDocument.m_strSpecCharEncText[j] ;
					strText += ";" ;
					iLast = i + 1 ;
				}
			}
		}
		return	strText + strSrc.substring( iLast ) ;
	}

}

