
#if	!defined(__SAKURA2_XML_DOCUMENT_H__)
#define	__SAKURA2_XML_DOCUMENT_H__

namespace	SSystem
{
	//////////////////////////////////////////////////////////////////////////
	// XML（風）ドキュメント
	//////////////////////////////////////////////////////////////////////////

	class	SXMLDocument	: public SObject, public SParserErrorTracer
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
			( SSystem::SXMLDocument, SObject, SParserErrorInterface )
		// 構築関数
		SXMLDocument( void ) ;
		SXMLDocument( const SXMLDocument& xmlSrc ) ;
		// 消滅関数
		virtual ~SXMLDocument( void ) ;

	public:
		// データ種類
		enum	DocumentType
		{
			typeRoot,
			typeTag,
			typeText,
			typeCDATA,
			typeComment,
		} ;

	public:
		enum	DataTypeFlag
		{
			dataString	= 0x0001,
			dataInteger	= 0x0002,
			dataHexInt	= 0x0004,
			dataFloat	= 0x0008,
		} ;
		class	AttrData
		{
		public:
			uint32_t	m_nFlags ;		// complex of enum DataTypeFlag
			int64_t		m_nInteger ;
			int64_t		m_nHexInt ;
			double		m_nFloat ;
			SString		m_strText ;
		public:
			ESL_DECLARE_CLASS_OPERATOR_NEW_NV( ESLObject )
			AttrData( void ) : m_nFlags( dataString ) { }
			AttrData( const AttrData& ad )
				: m_nFlags( ad.m_nFlags ),
					m_nInteger( ad.m_nInteger ),
					m_nHexInt( ad.m_nHexInt ),
					m_nFloat( ad.m_nFloat ),
					m_strText( ad.m_strText ) { }
		} ;

	protected:
		DocumentType					m_typeDoc ;
		SString							m_strTag ;
		SString							m_strText ;
		ESLObject *						m_pAppData ;
		SStrSortObjectArray<AttrData>	m_ssoaAttr ;
		SObjectArray<SXMLDocument>		m_xmlElements ;
		const ESLRuntimeClass *			m_prsElementClass ;

	public:
		// 代入
		const SXMLDocument & operator = ( const SXMLDocument& xmlSrc )
		{
			CopyAllContentsFrom( xmlSrc ) ;
			return	*this ;
		}
		// 複製
		void CopyAllContentsFrom( const SXMLDocument& xmlSrc ) ;
		// 全ての内容を消去
		void RemoveAllContents( void ) ;
		// 空のデータか？
		bool IsEmpty( void ) const ;

	public:
		// データ種類取得
		DocumentType GetType( void ) const
		{
			return	m_typeDoc ;
		}
		// データ種類設定
		void SetType( DocumentType type )
		{
			m_typeDoc = type ;
		}
		// タグ取得
		const SString& GetTag( void ) const
		{
			return	m_strTag ;
		}
		// タグ設定
		void SetTag( const wchar_t * pszTag ) ;
		// テキスト取得
		const SString& GetText( void ) const
		{
			return	m_strText ;
		}
		// テキスト設定
		void SetText
			( const wchar_t * pszText, DocumentType type = typeText ) ;
		// アプリケーションデータを取得する
		ESLObject * GetApplicationData( void ) const
		{
			return	m_pAppData ;
		}
		// アプリケーションデータを設定する
		void SetApplicationData( ESLObject * pObj ) ;

	public:
		// 値ペア
		struct	AttrInteger
		{
			const wchar_t *	pszSymbol ;	// null for end of list
			int64_t			nValue ;
		} ;
		static int64_t GetIntegerAsSymbolOf
			( const AttrInteger * pPairs, const wchar_t * pwszSymbol,
				int64_t nDefault = 0, ssize_t * pFindIndex = NULL ) ;
		static int64_t GetIntegerAsNoCaseSymbolOf
			( const AttrInteger * pPairs, const wchar_t * pwszSymbol,
				int64_t nDefault = 0, ssize_t * pFindIndex = NULL ) ;
		static const wchar_t * GetSymbolAsIntegerOf
			( const AttrInteger * pPairs,
				int64_t nValue, ssize_t * pFindIndex = NULL ) ;
		// 属性値取得
		SString * GetAttributeAs( const wchar_t * pszAttrName ) const ;
		SString GetAttrStringAs
			( const wchar_t * pszAttrName,
					const wchar_t * pszDefValue = NULL ) const ;
		int64_t GetAttrIntegerAs
			( const wchar_t * pszAttrName, int64_t nDefValue = 0 ) const ;
		int64_t GetAttrHexIntegerAs
			( const wchar_t * pszAttrName, int64_t nDefValue = 0 ) const ;
		int64_t GetAttrRichIntegerAs
			( const wchar_t * pszAttrName, int64_t nDefValue = 0 ) const ;
		int64_t GetAttrSymbolizedIntegerAs
			( const wchar_t * pszAttrName,
				const AttrInteger * pPairs, int64_t nDefValue = 0 ) const ;
		int64_t GetAttrComplexIntegerAs
			( const wchar_t * pszAttrName,
				const AttrInteger * pPairs,
				int64_t nDefValue = 0,
				const wchar_t * pszSeparators = L",|" ) const ;
		double GetAttrRealAs
			( const wchar_t * pszAttrName, double nDefValue = 0.0 ) const ;
		double GetAttrRichRealAs
			( const wchar_t * pszAttrName, double nDefValue = 0.0 ) const ;
		// 属性値設定
		void SetAttributeAs
			( const wchar_t * pszAttrName, const wchar_t * pszValue ) ;
		void SetAttrIntegerAs
			( const wchar_t * pszAttrName, int64_t nValue ) ;
		void SetAttrHexIntegerAs
			( const wchar_t * pszAttrName, int64_t nValue ) ;
		SError SetAttrSymbolizedIntegerAs
			( const wchar_t * pszAttrName,
				const AttrInteger * pPairs, int64_t nValue ) ;
		SError SetAttrComplexIntegerAs
			( const wchar_t * pszAttrName,
				const AttrInteger * pPairs,
				int64_t nValue, const wchar_t * pszSeparator = L" " ) ;
		void SetAttrRealAs
			( const wchar_t * pszAttrName, double nValue ) ;
		// 属性配列取得
		SStrSortObjectArray<AttrData> & GetAttributes( void )
		{
			return	m_ssoaAttr ;
		}
		// 属性総数取得
		size_t GetAttributeCount( void ) const
		{
			return	m_ssoaAttr.GetLength() ;
		}
		// 属性名取得
		const SString * GetAttributeNameAt( size_t index ) const
		{
			return	m_ssoaAttr.GetTagAt( index ) ;
		}
		// 属性値取得
		SString * GetAttributeValueAt( size_t index ) const
		{
			AttrData *	pad = m_ssoaAttr.GetAt( index ) ;
			return	(pad != NULL) ? &(pad->m_strText) : NULL ;
		}
		// 属性削除
		void RemoveAttributeAs( const wchar_t * pwszAttrName ) ;
		// 属性全削除
		void RemoveAllAttributes( void ) ;

	public:
		// 要素検索
		ssize_t FindElement
			( DocumentType typeDoc,
				const wchar_t * pszTag = NULL, size_t iFirst = 0 ) const ;
		ssize_t FindElementTag
			( const wchar_t * pszTag, size_t iFirst = 0 ) const
		{
			return	FindElement( typeTag, pszTag, iFirst ) ;
		}
		// サブタグ取得
		SXMLDocument * GetElementAs
			( DocumentType typeDoc,
				const wchar_t * pszTag = NULL, size_t iFirst = 0 ) const ;
		SXMLDocument * GetElementTagAs
			( const wchar_t * pszTag, size_t iFirst = 0 ) const
		{
			return	GetElementAs( typeTag, pszTag, iFirst ) ;
		}
		SString * GetTextElement( size_t iFirst = 0 ) const ;
		SString * GetTextElementAs( const wchar_t * pszTag, size_t iFirst = 0 ) const
		{
			SXMLDocument *	pxmlTag = GetElementTagAs( pszTag, iFirst ) ;
			if ( pxmlTag != NULL )
			{
				return	pxmlTag->GetTextElement() ;
			}
			return	NULL ;
		}
		// サブタグ生成
		SXMLDocument * CreateElementAs
			( DocumentType typeDoc,
				const wchar_t * pszTag = NULL, size_t iFirst = 0 ) ;
		SXMLDocument * CreateElementTagAs
			( const wchar_t * pszTag, size_t iFirst = 0 )
		{
			return	CreateElementAs( typeTag, pszTag, iFirst ) ;
		}
		// サブコンテンツ数取得
		size_t GetElementsCount( void ) const
		{
			return	m_xmlElements.GetLength() ;
		}
		// サブコンテンツ取得
		SXMLDocument * GetElementAt( size_t index ) const
		{
			return	m_xmlElements.GetAt( index ) ;
		}
		// サブコンテンツ追加
		void AddElement( SXMLDocument * pDoc )
		{
			ESLAssert( pDoc != NULL ) ;
			ESLAssert( pDoc->IsKindOf( *m_prsElementClass ) ) ;
			m_xmlElements.Add( pDoc ) ;
		}
		SXMLDocument * AddTextElement
			( const wchar_t * pszText, DocumentType type = typeText ) ;
		SXMLDocument * SetTextElement
			( const wchar_t * pszText, DocumentType type = typeText ) ;
		// サブコンテンツ挿入
		void InsertElementAt( size_t index, SXMLDocument * pDoc )
		{
			m_xmlElements.InsertAt( index, pDoc ) ;
		}
		// サブコンテンツ削除
		void RemoveElementAt( size_t index )
		{
			m_xmlElements.RemoveAt( index ) ;
		}
		void RemoveAllElements( void )
		{
			m_xmlElements.RemoveAll() ;
		}
		// サブコンテンツ配列取得
		SObjectArray<SXMLDocument> & GetElementsArray( void )
		{
			return	m_xmlElements ;
		}

	public:
		// コンテンツ値（属性 or テキスト要素）取得
		// (element1\element2\ ... elementX or attribute)
		SXMLDocument * GetContentsElement( const wchar_t * pszPath ) const ;
		SXMLDocument * CreateContentsElement( const wchar_t * pszPath ) ;
		SString * GetContentsValue( const wchar_t * pszPath ) const ;
		SString GetContentsAsString
			( const wchar_t * pszPath,
					const wchar_t * pszDefValue = NULL ) const ;
		int64_t GetContentsAsInteger
			( const wchar_t * pszPath, int64_t nDefValue = 0 ) const ;
		int64_t GetContentsAsHexInteger
			( const wchar_t * pszPath, int64_t nDefValue = 0 ) const ;
		double GetContentsAsReal
			( const wchar_t * pszPath, double nDefValue = 0.0 ) const ;

	public:
		// XMLデータ読み込み
		SError LoadDocument
			( const wchar_t * pwszFilePath,
				SParserErrorInterface& perr,
				Charset::EncodingType encoding = Charset::encodingUTF8 ) ;
		SError ReadDocument
			( SFileInterface& file, SParserErrorInterface& perr,
				Charset::EncodingType encoding = Charset::encodingUTF8 ) ;
		SError ParseDocumentFromString
			( const wchar_t * pwszXML, SParserErrorInterface * perr = NULL ) ;
		// XMLデータ解釈（要素配列の解釈）
		size_t ParseXMLElements
			( SStringParser& sparsDoc,
				SStrSortObjectArray<SString>& ssoaDTD,
				SParserErrorInterface& perr ) ;
		// XMLデータ解釈（1つのタグ／文字列の解釈）
		size_t ParseDocument
			( SStringParser& sparsDoc,
				SStrSortObjectArray<SString>& ssoaDTD,
				SParserErrorInterface& perr ) ;
	public:
		// 名前を取得 (namespace1:namespace2:...name)
		size_t ParseXMLNameToken
			( SString& strName,
				SStringParser& sparsDoc, SParserErrorInterface& perr ) ;
		// <! ... > を解釈（主に読み飛ばす）
		size_t ParseDocTypeSection
			( SStringParser& sparsDoc,
				SStrSortObjectArray<SString>& ssoaDTD,
				SParserErrorInterface& perr ) ;
		// タグ属性を解釈（> 又は /> まで sparsDoc の指標を移動）
		size_t ParseTagAttributes
			( SStringParser& sparsDoc,
				SStrSortObjectArray<SString>& ssoaDTD,
				SParserErrorInterface& perr ) ;
		// 属性値を解釈
		static size_t ParseTagAttributeValue
			( SString& strValue,
				SStringParser& sparsDoc,
				SStrSortObjectArray<SString>& ssoaDTD,
				SParserErrorInterface& perr ) ;

	public:
		// XMLデータ書き出し
		SError SaveDocument
			( const wchar_t * pwszFilePath, int nIndent = 0,
				Charset::EncodingType encoding = Charset::encodingUTF8 ) const ;
		SError WriteDocument
			( SFileInterface& file, int nIndent = 0,
				Charset::EncodingType encoding = Charset::encodingUTF8 ) const ;
		SError FormatDocumentToString( SString& strDoc, int nIndent = 0 ) const ;
		// XMLデータ書き出し（要素配列）
		SError FormatXMLElements
			( SFileInterface& file,
				int nIndent, Charset::EncodingType encoding ) const ;
		// XMLデータ書き出し（1つのタグ／文字列）
		SError FormatDocument
			( SFileInterface& file, int nIndent = -1,
				Charset::EncodingType encoding = Charset::encodingUTF8 ) const ;

	public:
		// タグ要素を除去したプレーンテキスト形式へ変換
		SString ToPlainText( void ) const ;

	public:
		// 文字列コンテンツのデコード
		static SError DecodeXMLText
			( SString & strText, SStrSortObjectArray<SString>& ssoaDTD ) ;
		// 文字列コンテンツのエンコーディング
		static SError EncodeXMLText
			( SString & strText,
				const SString & strSrc, int nIndent = -1 ) ;

	public:
		// SXMLDocument 要素作成
		virtual SXMLDocument * new_XMLDocument( void ) ;
		// SParserErrorTracer
		virtual const char * GetParserNameForTrace( void ) ;
	} ;
}

#endif
