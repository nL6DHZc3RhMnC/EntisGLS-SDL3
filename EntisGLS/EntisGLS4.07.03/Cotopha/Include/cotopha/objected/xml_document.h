
#if	!defined(__XML_DOCUMENT_H__)
#define	__XML_DOCUMENT_H__

//////////////////////////////////////////////////////////////////////////////
// XML ドキュメント
//////////////////////////////////////////////////////////////////////////////

class	ParserErrorInterface
{
public:
	virtual void OutputError
		( const String& ss, String strError ) = 0 ;
	virtual void OutputWarning
		( const String& ss, String strWarning ) = 0 ;
} ;

class	XMLDocument	: public ParserErrorInterface
{
public:
	// 構築関数
	XMLDocument( const XMLDocument& xmlSrc ) ;

public:
	enum	DocumentType<int>
	{
		Root,
		Tag,
		Text,
		CDATA,
		Comment,
	} ;

protected:
	int				m_typeDoc = DocumentType::Root ;
	String			m_strTag ;
	String			m_strText ;
	Hash<String>	m_ssoaAttr ;
	XMLDocument&[]	m_xmlElements ;

public:
	// 代入
	const XMLDocument& operator = ( const XMLDocument& xmlSrc ) ;
	// 複製
	void CopyAllContentsFrom( const XMLDocument& xmlSrc ) ;
	// 全ての内容を消去
	void RemoveAllContents( void ) ;
	// 空のデータか？
	bool IsEmpty( void ) const ;

public:
	// データ種類取得
	int GetType( void ) const ;
	// タグ取得
	String GetTag( void ) const ;
	// タグ設定
	void SetTag( String strTag ) ;
	// テキスト取得
	String GetText( void ) const ;
	// テキスト設定
	void SetText
		( String strText, int type = DocumentType::Text ) ;

public:
	// 属性値取得
	const String& GetAttributeAs( String strAttrName ) const ;
	String GetAttrStringAs( String strAttrName, String strDefValue = "" ) const ;
	int GetAttrIntegerAs( String strAttrName, int nDefValue = 0 ) const ;
	int GetAttrHexIntegerAs( String strAttrName, int nDefValue = 0 ) const ;
	double GetAttrRealAs( String strAttrName, double nDefValue = 0.0 ) const ;
	// 属性値設定
	void SetAttributeAs( String strAttrName, String strValue ) ;
	void SetAttrIntegerAs( String strAttrName, int nValue ) ;
	void SetAttrHexIntegerAs( String strAttrName, int nValue ) ;
	void SetAttrRealAs( String strAttrName, double nValue ) ;
	// 属性配列取得
	Hash<String> & GetAttributes( void ) ;
	// 属性総数取得
	int GetAttributeCount( void ) const ;
	// 属性名取得
	String GetAttributeNameAt( int index ) const ;
	// 属性値取得
	String GetAttributeValueAt( int index ) const ;

public:
	// 要素検索
	int FindElement
		( int typeDoc,
			String strTag = "", int iFirst = 0 ) const ;
	int FindElementTag( String strTag, int iFirst = 0 ) const ;
	// サブタグ取得
	const XMLDocument& GetElementAs
		( int typeDoc,
			String strTag = "", int iFirst = 0 ) const ;
	const XMLDocument& GetElementTagAs( String strTag, int iFirst = 0 ) const ;
	XMLDocument& GetElementAs
		( int typeDoc,
			String strTag = "", int iFirst = 0 ) ;
	XMLDocument& GetElementTagAs( String strTag, int iFirst = 0 ) ;
	// サブタグ生成
	XMLDocument& CreateElementAs
		( int typeDoc,
			String strTag = "", int iFirst = 0 ) ;
	XMLDocument& CreateElementTagAs( String strTag, int iFirst = 0 ) ;
	// サブコンテンツ数取得
	int GetElementsCount( void ) const ;
	// サブコンテンツ取得
	const XMLDocument& GetElementAt( int index ) const ;
	XMLDocument& GetElementAt( int index ) ;
	// サブコンテンツ追加
	void AddElement( XMLDocument& pDoc ) ;
	void InsertElementAt( int iElementBefore, XMLDocument& pDoc ) ;
	void AddTextElement( String sText, int type = DocumentType::Text ) ;
	// サブコンテンツ配列取得
	XMLDocument&[]& GetElementsArray( void ) ;

public:
	// コンテンツ値（属性 or 要素）取得
	const String& GetContentsValue( String strPath ) const ;
	String GetContentsAsString
		( String strPath, String strDefValue = "" ) const ;
	int GetContentsAsInteger( String strPath, int nDefValue = 0 ) const ;
	int GetContentsAsHexInteger( String strPath, int nDefValue = 0 ) const ;
	double GetContentsAsReal( String strPath, double nDefValue = 0.0 ) const ;

public:
	// XMLデータ読み込み
	Error LoadDocument( String file, ParserErrorInterface& perr ) ;
	Error ReadDocument( File& file, ParserErrorInterface& perr ) ;
	// XMLデータ解釈（要素配列の解釈）
	int ParseXMLElements
		( String& sparsDoc,
			Hash<String>& ssoaDTD, ParserErrorInterface& perr ) ;
	// XMLデータ解釈（1つのタグ／文字列の解釈）
	int ParseDocument
		( String& sparsDoc,
			Hash<String>& ssoaDTD, ParserErrorInterface& perr ) ;
protected:
	// <! ... > を解釈（主に読み飛ばす）
	int ParseDocTypeSection
		( String& sparsDoc,
			Hash<String>& ssoaDTD, ParserErrorInterface& perr ) ;
	// タグ属性を解釈（> 又は /> まで sparsDoc の指標を移動）
	int ParseTagAttributes
		( String& sparsDoc,
			Hash<String>& ssoaDTD, ParserErrorInterface& perr ) ;
	// 属性値を解釈
	static int ParseTagAttributeValue
		( String& strValue, String& sparsDoc,
				Hash<String>& ssoaDTD, ParserErrorInterface& perr ) ;

protected:
	// タグ属性を解釈（> 又は /> まで sparsDoc の指標を移動）（高速版）
	int FastParseTagAttributes
		( String& sparsDoc, uint32 iIndex, uint32 nTagLimit,
			Hash<String>& ssoaDTD, ParserErrorInterface& perr ) naked ;
	// 属性値追加
	void nakedAddTagAttributeValue
			( String * pstrName, String * pstrValue ) naked ;
	void AddTagAttributeValue
			( String * pstrName, String * pstrValue ) ;
	// エラー出力
	static void nakedOutputError
		( ParserErrorInterface& perr,
				const String& ss, const uint16 * pwErrMsg ) naked ;
	// 指標移動
	static void nakedSeekIndex( String& ss, int nIndex ) naked ;

public:
	// XMLデータ書き出し
	Error SaveDocument
		( String file, int nIndent = 0, String encoding = "utf-8" ) const ;
	Error WriteDocument
		( File& file, int nIndent = 0, String encoding = "utf-8" ) const ;
	String FormatDocumentToString( void ) const ;
	// XMLデータ書き出し（要素配列）
	Error FormatXMLElements( File& file, int nIndent ) const ;
	// XMLデータ書き出し（1つのタグ／文字列）
	Error FormatDocument( File& file, int nIndent = -1 ) const ;

public:
	// 16進数文字列を数値に変換
	static int NumberFromHexString( String strHex ) ;

	data XMLTextEscapeChar<String> ;
	static naked const uint16 *	m_pszXMLTextEscapeChar[6] ;
	static naked const uint16 *	m_pszXMLTextEscapeCode ;

	// 文字列コンテンツのデコード
	static Error DecodeXMLText
		( String& strText, Hash<String>& ssoaDTD ) ;
	static Error nakedDecodeXMLText
		( String& strText, const uint16 * pwSrc,
				uint32 nLength, Hash<String>& ssoaDTD ) naked ;
	static uint16 * nakedGetMappedDTD
		( const uint16 * pwChar, uint32 nLength, Hash<String>& ssoaDTD ) naked ;
	static String GetMappedDTD( Hash<String> * ssoaDTD, String * pstrChar ) ;
	// 文字列コンテンツのエンコーディング
	static Error EncodeXMLText
		( String& strText, String strSrc, int nIndent = -1 ) ;

public:
	// エラー出力
	virtual void OutputError
		( const String& ss, String strError ) ;
	virtual void OutputWarning
		( const String& ss, String strWarning ) ;
} ;


#endif

