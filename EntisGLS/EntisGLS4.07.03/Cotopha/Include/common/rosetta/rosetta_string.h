
#if	!defined(__ROSETTA_STRING_H__)
#define	__ROSETTA_STRING_H__

namespace	Rosetta
{
	//////////////////////////////////////////////////////////////////////////
	// 文字列
	//////////////////////////////////////////////////////////////////////////

	class	RSString	: public RSObject
	{
	public:
		SSystem::SString	m_strValue ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSString, RSObject )
		// 構築関数
		RSString( RSClass * pClass, const wchar_t * pwszStr )
			: RSObject(pClass,typeString), m_strValue(pwszStr) {}

	public:	// 型情報
		// 型名
		virtual const wchar_t * GetTypeName( void ) const ;
		// 型テスト
		virtual RSObject * InstanceOf( const wchar_t * pwszType ) ;
		// 文字列型か？
		virtual bool IsStringType( void ) const ;
		// 即値型か？
		virtual bool IsObjectType( void ) const ;
		// 整数値取得
		virtual bool AsInteger( int64_t& number ) const ;
		// 実数値取得
		virtual bool AsRealNumber( double& number ) const ;
		// ブール判定
		virtual bool AsBoolean( void ) const ;
		// 文字列変換
		virtual bool AsString( SSystem::SString& strValue ) const ;
		// 同定判定
		virtual bool IsEqualObject( RSObject * pObj ) const ;
		// デバッグ用ダンプ文字列
		virtual void ToDebugDump
			( SSystem::SFileInterface& dump,
					size_t nPtrNest = 10,
					const wchar_t * pwszIndent = NULL ) ;
		// 値設定
		virtual SSystem::SError SetIntegerAs( int64_t nValue ) ;
		virtual SSystem::SError SetNumberAs( double nValue ) ;
		virtual SSystem::SError SetStringAs( const wchar_t * pwszValue ) ;

	public:	// オブジェクト
		// 内部リソース解放
		virtual void DisposeObject( RSContext& context ) ;
		// 複製（実体も可能な限り複製）
		virtual RSObject * CloneObject( RSContext& context ) const ;

	public:
		// 単項演算子
		virtual RSObject * OperatorPlus( RSContext& context ) const ;
		virtual RSObject * OperatorNegate( RSContext& context ) const ;
		// 二項演算子
		virtual RSObject * OperatorMul( RSContext& context, RSObject * pObj ) const ;
		virtual RSObject * OperatorAdd( RSContext& context, RSObject * pObj ) const ;
		virtual RSObject * OperatorCompareEQ( RSContext& context, RSObject * pObj ) const ;
		virtual RSObject * OperatorCompareNE( RSContext& context, RSObject * pObj ) const ;
		virtual RSObject * OperatorCompareGE( RSContext& context, RSObject * pObj ) const ;
		virtual RSObject * OperatorCompareGT( RSContext& context, RSObject * pObj ) const ;
		virtual RSObject * OperatorCompareLE( RSContext& context, RSObject * pObj ) const ;
		virtual RSObject * OperatorCompareLT( RSContext& context, RSObject * pObj ) const ;
		// 代入演算子
		virtual RSObject * OperatorMove( RSContext& context, RSObject * pObj ) ;

	public:
		// シリアライズ
		virtual RSObject * SerializeObject( RSContext& context ) ;
		virtual SSystem::SError SerializeBinary
				( RSContext& context, SSystem::SFileInterface& file ) ;
		virtual SSystem::SError MakeXMLDocument
				( RSContext& context, SSystem::SXMLDocument& xmlDoc ) ;
		// 復元
		virtual SSystem::SError RestoreObject
				( RSContext& context, RSObject * pObj ) ;
		virtual SSystem::SError RestoreBinary
				( RSContext& context, SSystem::SFileInterface& file ) ;
		virtual SSystem::SError RestoreXMLDocument
				( RSContext& context, const SSystem::SXMLDocument& xmlDoc ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 文字列型
	//////////////////////////////////////////////////////////////////////////

	class	RSStringClass	: public RSClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSStringClass, RSClass )
		// 構築関数
		RSStringClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"String" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;

	public:
		// インスタンス生成
		virtual RSObject * NewInstance( RSContext& context, RSObject * pArg ) ;
		// 変数インスタンス生成
		virtual RSObject * NewVariable( RSContext& context ) ;
		// キャスト処理
		virtual bool TestCastInstance
			( RSObject * pObj, CastMethod castMethod = castNatural ) ;
		virtual RSObject * CastInstance
			( RSContext& context, RSObject * pObj, CastMethod castMethod ) ;

	public:	// String method
		// void <init>( char[] value )
		// void <init>( char[] value, int offset, int count )
		static RSObject * method_init1
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void <init>( String value )
		static RSObject * method_init2
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void <init>( Uint16Pointer value )
		// void <init>( Uint16Pointer value, int count )
		static RSObject * method_init3
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setString( String str )
		static RSObject * method_setString
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int length()
		static RSObject * method_length
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int indexOf( String str, int first = 0 )
		static RSObject * method_indexOf
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String charAt( int index )
		static RSObject * method_charAt
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// char charCodeAt( int index )
		static RSObject * method_charCodeAt
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// char charLastCodeAt( int index )
		static RSObject * method_charLastCodeAt
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int compareTo( String str )
		static RSObject * method_compareTo
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int compareToIgnoreCase( String str )
		static RSObject * method_compareToIgnoreCase
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String replace( String oldStr, String newStr )
		static RSObject * method_replace
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String substring( int begin )
		// String substring( int begin, int end )
		static RSObject * method_substring
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// char[] toCharArray()
		static RSObject * method_toCharArray
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String toUpperCase()
		static RSObject * method_toUpperCase
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String toLowerCase()
		static RSObject * method_toLowerCase
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String chopRight( int count )
		static RSObject * method_chopRight
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String trim()
		static RSObject * method_trim
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String trimRight()
		static RSObject * method_trimRight
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String trimLeft()
		static RSObject * method_trimLeft
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// long asInteger()
		static RSObject * method_asInteger
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// double asNumber()
		static RSObject * method_asNumber
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// long parseInt( int radix = 10 )
		static RSObject * method_parseInt
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static String format( String fmt, ... )
		static RSObject * method_format
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;

	public:
		// String mappingFilter( HashMap map )
		static RSObject * method_mappingFilter
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String getFileNamePart( char sep = '\\' )
		static RSObject * method_getFileNamePart
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String getFileExtensionPart( char sep = '\\' )
		static RSObject * method_getFileExtensionPart
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String getFileDirectoryPart( char sep = '\\' )
		static RSObject * method_getFileDirectoryPart
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String getFileTitlePart( char sep = '\\' )
		static RSObject * method_getFileTitlePart
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String getFileDrivePart( char sep = '\\' )
		static RSObject * method_getFileDrivePart
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String offsetFilePath( String sOffsetPath, char sep = '\\' )
		static RSObject * method_offsetFilePath
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String relativeFilePath( String sFullPath, int nAscendLimit = 5, char sep = '\\' )
		static RSObject * method_relativeFilePath
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String normalizeFilePath( char sep = '\\' )
		static RSObject * method_normalizeFilePath
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;

	public:
		// static String getEncodingName( int encoding )
		static RSObject * method_getEncodingName
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static int getEncodingType( String encoding )
		static RSObject * method_getEncodingType
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static int analyzeEncoding
		//	( Uint8Pointer ptrSrc, int nLength = -1 )
		static RSObject * method_analyzeEncoding
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Uint8Pointer encodeTo( int encoding )
		static RSObject * method_encodeTo
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static String decode
		//	( int encoding, Uint8Pointer ptrSrc, int nLength = -1 )
		static RSObject * method_decode
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static String encodeBase64( Uint8Pointer ptrSrc, int nLength = -1 )
		static RSObject * method_encodeBase64
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Uint8Pointer decodeBase64()
		static RSObject * method_decodeBase64
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static String encodeHex( Uint8Pointer ptrSrc, int nLength = -1 )
		static RSObject * method_encodeHex
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Uint8Pointer decodeHex()
		static RSObject * method_decodeHex
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String decodeCLangString()
		static RSObject * method_decodeCLangString
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String encodeCLangString()
		static RSObject * method_encodeCLangString
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String decodeCSVString()
		static RSObject * method_decodeCSVString
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String encodeCSVString()
		static RSObject * method_encodeCSVString
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 文字列パーサー
	//////////////////////////////////////////////////////////////////////////

	class	RSStringParser	: public RSObject
	{
	public:
		SSystem::SStringParser	m_parser ;
		RSString *				m_pRefString ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSStringParser, RSObject )
		// 構築関数
		RSStringParser( RSClass * pClass )
					: RSObject(pClass,typeOther), m_pRefString(NULL) {}
		// 消滅関数
		virtual ~RSStringParser( void ) ;

	public:	// 型情報
		// 型名
		virtual const wchar_t * GetTypeName( void ) const ;
		// 型テスト
		virtual RSObject * InstanceOf( const wchar_t * pwszType ) ;
		// 文字列型か？
		virtual bool IsStringType( void ) const ;
		// 即値型か？
		virtual bool IsObjectType( void ) const ;
		// ブール判定
		virtual bool AsBoolean( void ) const ;
		// 文字列変換
		virtual bool AsString( SSystem::SString& strValue ) const ;
		// 同定判定
		virtual bool IsEqualObject( RSObject * pObj ) const ;
		// デバッグ用ダンプ文字列
		virtual void ToDebugDump
			( SSystem::SFileInterface& dump,
					size_t nPtrNest = 10,
					const wchar_t * pwszIndent = NULL ) ;

	public:	// オブジェクト
		// 内部リソース解放
		virtual void DisposeObject( RSContext& context ) ;
		// 複製（実体も可能な限り複製）
		virtual RSObject * CloneObject( RSContext& context ) const ;

	public:
		// 代入演算子
		virtual RSObject * OperatorMove( RSContext& context, RSObject * pObj ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 文字列パーサー型
	//////////////////////////////////////////////////////////////////////////

	class	RSStringParserClass	: public RSClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSStringParserClass, RSClass )
		// 構築関数
		RSStringParserClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"StringParser" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;

	public:	// StringParser method
		// void <init>( String value )
		static RSObject * method_init1
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void <init>( StringParser value )
		static RSObject * method_init2
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void attachString( String str )
		static RSObject * method_attachString
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void releaseString()
		static RSObject * method_releaseString
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean loadTextFile
		//	( String sFilePath, int encoding = String.encodingUnknown )
		static RSObject * method_loadTextFile
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean readTextFile
		//	( InputStream is, int encoding = String.encodingUnknown )
		// boolean readTextFile
		//	( RandomAccessFile raf, int encoding = String.encodingUnknown )
		static RSObject * method_readTextFile
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int getLength()
		static RSObject * method_getLength
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int getIndex()
		static RSObject * method_getIndex
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int seekIndex( int index )
		static RSObject * method_seekIndex
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean isIndexOverflow()
		static RSObject * method_isIndexOverflow
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean seekString( String str )
		static RSObject * method_seekString
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean seekAnyCharacters( String str )
		static RSObject * method_seekAnyCharacters
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void markIndex()
		static RSObject * method_markIndex
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void seekToMark()
		static RSObject * method_seekToMark
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void releaseMark()
		static RSObject * method_releaseMark
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String subString( int iStart, int nCount = -1 )
		static RSObject * method_subString
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String subStringFromMark()
		static RSObject * method_subStringFromMark
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String subStringFrom( int iStart )
		static RSObject * method_subStringFrom
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int getLineNumberOf( int index, int[] pGetLineIndex = null )
		static RSObject * method_getLineNumberOf
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// char currentCharacter()
		static RSObject * method_currentCharacter
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// char offsetAt( int iOffset )
		static RSObject * method_offsetAt
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static boolean isCharacterSpace( char c )
		static RSObject * method_isCharacterSpace
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static boolean isPunctuation( char c )
		static RSObject * method_isPunctuation
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static boolean isSpecialMark( char c )
		static RSObject * method_isSpecialMark
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean passSpace()
		static RSObject * method_passSpace
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int seekToNextLine()
		static RSObject * method_seekToNextLine
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void passString()
		static RSObject * method_passString
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void passEnclosedString( char chCloser, int nCtrlFlags = 0 )
		static RSObject * method_passEnclosedString
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int passToken()
		static RSObject * method_passToken
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void passExpressionTerm( int nCtrlFlags = 0 )
		static RSObject * method_passExpressionTerm
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void passExpression( String strCloses, int nCtrlFlags = 0 )
		static RSObject * method_passExpression
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// char getCharacter()
		static RSObject * method_getCharacter
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String getLine( int[] pRetCode = null )
		static RSObject * method_getLine
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String getString()
		static RSObject * method_getString
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String getEnclosedString
		//		( char chCloser, int nCtrlFlags = 0, char[] pGetClosed = null )
		static RSObject * method_getEnclosedString
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String getToken( int[] pGetType = null )
		static RSObject * method_getToken
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String getStringTerm( int nCtrlFlags = 0 )
		static RSObject * method_getStringTerm
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String getExpressionTerm( int nCtrlFlags = 0 )
		static RSObject * method_getExpressionTerm
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String getExpression
		//	( String strCloses, int nCtrlFlags = 0, char[] pGetClosed = null )
		static RSObject * method_getExpression
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// char hasToComeChar( String strNext )
		static RSObject * method_hasToComeChar
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean hasToComeString( String strNext )
		static RSObject * method_hasToComeString
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean hasToComeNoCaseString( String strNext )
		static RSObject * method_hasToComeNoCaseString
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean hasToComeToken( String strNext )
		static RSObject * method_hasToComeToken
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int isNextNumber( int nCtrlFlags = 0 )
		static RSObject * method_isNextNumber
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// long nextInteger( int type = StringParser.numberInteger )
		static RSObject * method_nextInteger
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// double nextRealNumber( int type = StringParser.numberDefault )
		static RSObject * method_nextRealNumber
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int parseIntegerArray
		//		( long[] pNumbers, int nCount,
		//			int nCtrlFlags = 0, String strSeparator = null )
		static RSObject * method_parseIntegerArray
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int parseHexIntegerArray
		//		( long[] pNumbers, int nCount, String strSeparator = null )
		static RSObject * method_parseHexIntegerArray
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int parseNumberArray
		//		( double[] pNumbers, int nCount,
		//			int nCtrlFlags = 0, String strSeparator = null )
		static RSObject * method_parseNumberArray
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int parseCommaSeparatedValues( String[] aValues )
		static RSObject * method_parseCommaSeparatedValues
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 構文マッチャー
	//////////////////////////////////////////////////////////////////////////

	class	RSUsageMatcher	: public RSObject
	{
	public:
		SSystem::SUsageMatcher	m_matcher ;
		SSystem::SString		m_usage ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSUsageMatcher, RSObject )
		// 構築関数
		RSUsageMatcher( RSClass * pClass ) : RSObject(pClass,typeOther) {}
		// 消滅関数
		virtual ~RSUsageMatcher( void ) ;
		// 書式設定
		SSystem::SError SetUsage
			( const wchar_t * pwszUsage,
				SSystem::SParserErrorInterface & perr ) ;
		// 構文の解釈
		SSystem::SError IsMatchedWith
			( SSystem::SStringParser & sparsTarget,
				SSystem::SObjectArray<SSystem::SString> * parrParam,
				SSystem::SParserErrorInterface & perr ) const ;
		// 書式検索
		ssize_t FindMatchedWith
			( SSystem::SStringParser & sparsTarget,
				SSystem::SUsageMatcher::UsageType utWildCardType
							= SSystem::SUsageMatcher::utWildCardToken ) const ;

	public:	// 型情報
		// 型名
		virtual const wchar_t * GetTypeName( void ) const ;
		// 文字列型か？
		virtual bool IsStringType( void ) const ;
		// 文字列変換
		virtual bool AsString( SSystem::SString& strValue ) const ;
		// デバッグ用ダンプ文字列
		virtual void ToDebugDump
			( SSystem::SFileInterface& dump,
					size_t nPtrNest = 10,
					const wchar_t * pwszIndent = NULL ) ;

	public:	// オブジェクト
		// 内部リソース解放
		virtual void DisposeObject( RSContext& context ) ;
		// 複製（実体も可能な限り複製）
		virtual RSObject * CloneObject( RSContext& context ) const ;

	public:
		// 代入演算子
		virtual RSObject * OperatorMove( RSContext& context, RSObject * pObj ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 構文マッチャー型
	//////////////////////////////////////////////////////////////////////////

	class	RSUsageMatcherClass	: public RSClass
	{
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSUsageMatcherClass, RSClass )
		// 構築関数
		RSUsageMatcherClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"UsageMatcher" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;

	public:	// UsageMatcher method
		// void <init>( String usage )
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setUsage( String usage )
		static RSObject * method_setUsage
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String parse( String text, String[] aParams = null )
		static RSObject * method_parse
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String parseNext( StringParser parser, String[] aParams = null )
		static RSObject * method_parseNext
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int find( StringParser parser, int typeWildCard = UsageMatcher.utWildCardToken )
		static RSObject * method_find
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;

	} ;

}

#endif

