
#if	!defined(__ROSETTA_CLASS_H__)
#define	__ROSETTA_CLASS_H__

namespace	Rosetta
{
	class	RSParenthesis ;
	class	RSFunctionPrototype ;

	//////////////////////////////////////////////////////////////////////////
	// 名前空間
	//////////////////////////////////////////////////////////////////////////

	class	RSNamespace	: public RSGenericObject
	{
	protected:
		RSObject *	m_pRefBackLink ;
		RSObject *	m_pRefNamespace ;
		uint32_t	m_accModifier ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSNamespace, RSGenericObject )
		// 構築関数
		RSNamespace
			( RSObject * pRefSpace = NULL,
				uint32_t accMod = RSObject::modifierPublic,
				RSObject * pBackLink = NULL, RSClass * pClass = NULL ) ;
		// 消滅関数
		virtual ~RSNamespace( void ) ;
		// 参照名前空間設定
		void AttachReference
			( RSObject * pRefSpace = NULL,
				uint32_t accMod = RSObject::modifierPublic,
				RSObject * pBackLink = NULL ) ;
		// 参照チェーン取得
		RSObject * GetBackLink( void ) const
		{
			return	m_pRefBackLink ;
		}

	public:	// オブジェクト
		// 内部リソース解放
		virtual void DisposeObject( RSContext& context ) ;

	public:
		// デバッグ用ダンプ文字列
		virtual void ToDebugDump
			( SSystem::SFileInterface& dump,
					size_t nPtrNest = 10,
					const wchar_t * pwszIndent = NULL ) ;
		// メンバ取得
		virtual RSObject * GetMemberAs
			( RSContext& context, const wchar_t * pwszName ) const ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// クラスオブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	RSClass	: public RSNamespace
	{
	public:
		bool									m_flagInitialized ;
		bool									m_flagNativeClass ;
		bool									m_flagCompiled ;
		SSystem::SString						m_strClassName ;
		RSClass *								m_pNamespace ;
		RSClass *								m_pSuperClass ;
		SSystem::SPointerArray<RSClass>			m_lstImplements ;
		RSObject *								m_pPrototype ;
		RSGenericClassMembers					m_gcmVirtuals ;
		RSFunctionObject *						m_pConstructor ;
		RSFunctionObject *						m_pDestructor ;
		RSClass *								m_pArrayClass ;
		RSClass *								m_pHashMapClass ;
		SSystem::SObjectArray<RSCodeComment>	m_aImmComments ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSClass, RSNamespace )
		// 構築関数
		RSClass
			( RSClass * pClass, const wchar_t * pwszClassName,
				RSObject * pRefSpace = NULL,
				uint32_t accMod = RSObject::modifierPublic,
				RSObject * pBackLink = NULL ) ;
		// 消滅関数
		virtual ~RSClass( void ) ;
		// メンバ初期設定
		virtual void Initialize( RSContext& context ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// クラス定義の完了
		virtual void FinishClass( RSContext& context ) ;
		// ネイティブ関数情報
		struct	NativeMethodEntry
		{
			const wchar_t *			pwszName ;
			const wchar_t *			pwszReturnType ;
			const wchar_t *			pwszArgumentList ;
			RSObject::METHOD_PROC	pfnMethod ;
		} ;
		// ネイティブ関数を仮想関数としてクラスにオーバーライドする
		void AddVirtualNativeMethods
				( RSContext& context, const NativeMethodEntry * pnme ) ;
		// コンパイル済み（コンパイル試行済み）か？
		bool IsCompiledImplement( void ) const
		{
			return	m_flagCompiled ;
		}
		void SetCompiledFlag( void )
		{
			m_flagCompiled = true ;
		}
		// コメント生成
		RSCodeComment * ImmediateComment( const wchar_t * pwszText ) ;

	public:
		// クラス名取得
		const wchar_t * GetRSClassName( void ) const
		{
			return	m_strClassName ;
		}
		SSystem::SString GetFullClassName( void ) const ;
		// クラス型テスト
		bool IsInstanceOf( const wchar_t * pwszClass ) const ;
		bool IsInstanceOf( RSClass * pClass ) const ;
		// ネイティブ型テスト
		virtual bool IsNativeObjectOf( ESLObject * pObj ) const ;
		// 仮想関数取得
		RSFunctionObject * GetVirtualMemberAs
			( RSContext& context, const wchar_t * pwszName ) const ;
		// 仮想関数設定
		void SetVirtualMemberAs
			( RSContext& context,
				const wchar_t * pwszName, RSFunctionObject * pFunc ) ;
		size_t SetVirtualMemberAs
			( RSContext& context,
				const wchar_t * pwszName,
				RSFunctionPrototype * pProto, bool fOverride = true ) ;
		RSFunctionPrototype * AddVirtualDescriptiveAs
			( RSContext& context,
				SSystem::SParserErrorInterface& perr,
				const wchar_t * pwszName,
				const wchar_t * pwszType,
				const wchar_t * pwszArgList,
				RSParenthesis * pParenthesis,
				RSObject::METHOD_PROC pfnMethod = NULL,
				void * pMethodInstance = NULL, uint32_t nFlags = 0,
				const wchar_t * pwszComment = NULL ) ;
		// 仮想関数削除
		SSystem::SError RemoveVirtualMemberAs
			( RSContext& context, const wchar_t * pwszName ) ;
		// 派生元クラス追加
		void AddSuperClass( RSContext& context, RSClass * pSuperClass ) ;
		void AddImplementClass
			( RSContext& context,
				RSClass * pSuperClass, bool fMatchVirtualIndex = false ) ;
		// 親クラス
		RSClass * GetSuperClass( void ) const ;
		size_t GetImplementClassCount( void ) const ;
		RSClass * GetImplementClassAt( size_t i ) const ;
		// メンバ新規作成
		virtual RSObject * CreateMemberAs
			( RSContext& context, const wchar_t * pwszName, RSObject * pObj ) ;
		// インスタンス生成
		virtual RSObject * NewInstance( RSContext& context, RSObject * pArg ) ;
		// 変数インスタンス生成
		virtual RSObject * NewVariable( RSContext& context ) ;
		// キャスト処理
		enum	CastMethod
		{
			castNatural,
			castForce,
		} ;
		virtual bool TestCastInstance
			( RSObject * pObj, CastMethod castMethod = castNatural ) ;
		virtual RSObject * CastInstance
			( RSContext& context, RSObject * pObj,
							CastMethod castMethod = castNatural ) ;
		// 構築関数取得
		RSFunctionObject * GetConstructor( void ) const ;
		// 消滅関数取得
		RSFunctionObject * GetDestructor( void ) const ;
		// プロトタイプ取得
		RSObject * GetPrototypeObject( void ) const ;
		// 仮想関数配列取得
		const RSGenericClassMembers& GetVirtualFunctions( void ) const ;

	public:
		// クラス定義ダンプ
		virtual void DumpClassDeclaration
			( RSContext& context, SSystem::SString& strDecl ) ;
		// クラス静メンバダンプ
		virtual void DumpClassStaticMembers
			( RSContext& context, SSystem::SString& strDecl ) ;
		// クラスメンバダンプ
		virtual void DumpClassPrototypeMembers
			( RSContext& context, SSystem::SString& strDecl ) ;
		// クラス仮想関数ダンプ
		virtual void DumpClassVirtualFunctions
			( RSContext& context, SSystem::SString& strDecl ) ;
		// 変数定義ダンプ
		static void DumpVarDeclaration
			( RSContext& context, SSystem::SString& strVar,
				const wchar_t * pwszName, RSObject * pObj, uint32_t accMod ) ;
		// 関数プロトタイプ
		static void DumpFuncDeclaration
			( RSContext& context, SSystem::SString& strFunc,
				const wchar_t * pwszName,
				RSFunctionPrototype * pProto, uint32_t accMod ) ;
		// 初期値ダンプ表示形式
		static SSystem::SString FormatInitValue( RSObject * pObj ) ;
		// 修飾文字列生成
		static SSystem::SString FormatObjectModifiers( uint32_t accMod ) ;
		// 文字列インデント追加
		static void AddFormatIndentedString
			( SSystem::SString& strDst,
				SSystem::SString& strSrc, const wchar_t * pwszIndent ) ;

	public:
		// 型名
		virtual const wchar_t * GetTypeName( void ) const ;
		// 型テスト
		virtual RSObject * InstanceOf( const wchar_t * pwszType ) ;
		virtual RSObject * InstanceOf( RSClass * pClass ) ;
		// 内部リソース解放
		virtual void DisposeObject( RSContext& context ) ;
		// メンバ取得 (static 変数)
		virtual RSObject * GetMemberAs
			( RSContext& context, const wchar_t * pwszName ) const ;
		// static メンバ関数呼び出し
		RSObject * CallStaticFunction
			( RSContext& context,
				const wchar_t * pwszFuncName,
				RSObject**const ppArgs, size_t countArg,
				bool fStructCast = true, bool* pArgMatchResult = NULL ) ;

	public:
		// 参照チェーンを解放
		virtual void ReleaseReferenceChain( void ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// Object クラスオブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	RSGenricObjectClass	: public RSClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSGenricObjectClass, RSClass )
		// 構築関数
		RSGenricObjectClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"Object" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;

	public:	// Object 共通 method
		// void finalize()
		static RSObject * method_finalize
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean equals( Object obj )
		static RSObject * method_equals
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String toString()
		static RSObject * method_toString
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Class getClass()
		static RSObject * method_getClass
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void notify()
		static RSObject * method_notify
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void notifyAll()
		static RSObject * method_notifyAll
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void wait()
		// void wait( long timeout )
		static RSObject * method_wait
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// JavaScript 風 Object クラスオブジェクト (HashMap)
	//////////////////////////////////////////////////////////////////////////

	class	RSDynamicObjectClass	: public RSClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSDynamicObjectClass, RSClass )
		// 構築関数
		RSDynamicObjectClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"HashMap" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;

	protected:	// Object 共通 method
		// void <init>( Class clsElement )
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String keyAt( int index )
		static RSObject * method_keyAt
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String[] keys()
		static RSObject * method_keys
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int size()
		static RSObject * method_size
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void clear()
		static RSObject * method_clear
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Object remove( String key )
		static RSObject * method_remove
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void freeze()
		static RSObject * method_freeze
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean isEmpty( String key )
		static RSObject * method_isEmpty
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Object get( String key )
		static RSObject * method_get
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Object put( String key, Object obj )
		static RSObject * method_put
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// HashMap ジェネリック型
	//////////////////////////////////////////////////////////////////////////

	class	RSGenericHashMapClass	: public RSDynamicObjectClass
	{
	public:
		RSClass *	m_pElementClass ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO
			( RSGenericHashMapClass, RSDynamicObjectClass )
		// 構築関数
		RSGenericHashMapClass
			( RSClass * pClass,
				const wchar_t * pwszClassName, RSClass * pElementClass ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;

	protected:	// Object 共通 method
		// void <init>()
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// Class メタクラス
	//////////////////////////////////////////////////////////////////////////

	class	RSClassClass	: public RSClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSClassClass, RSClass )
		// 構築関数
		RSClassClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"Class" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;

	protected:	// Object 共通 method
		// String getName()
		static RSObject * method_getName
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 抽象ポインタ型
	//////////////////////////////////////////////////////////////////////////

	class	RSAbstractPointerClass	: public RSClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSAbstractPointerClass, RSClass )
		// 構築関数
		RSAbstractPointerClass
			( RSClass * pClass,
				const wchar_t * pwszClassName,
				RSObject * pRefNamespace = NULL ) ;
		// 消滅関数
		virtual ~RSAbstractPointerClass( void ) ;

	public:
		// クラス型テスト
		bool IsInstanceOf( const wchar_t * pwszClass ) const ;
		bool IsInstanceOf( RSClass * pClass ) const ;
		// インスタンス生成
		virtual RSObject * NewInstance( RSContext& context, RSObject * pArg ) ;
		// 変数インスタンス生成
		virtual RSObject * NewVariable( RSContext& context ) ;
		// キャスト処理
		virtual bool TestCastInstance
			( RSObject * pObj, CastMethod castMethod = castNatural ) ;
		virtual RSObject * CastInstance
			( RSContext& context, RSObject * pObj, CastMethod castMethod ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// Function 型
	//////////////////////////////////////////////////////////////////////////

	class	RSFunctionClass	: public RSClass
	{
	public:
		RSFunctionPrototype *	m_pProto ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSFunctionClass, RSClass )
		// 構築関数
		RSFunctionClass
			( RSClass * pClass,
				const wchar_t * pwszClassName = L"Function",
				RSFunctionPrototype * pProto = NULL ) ;
		// メンバ初期設定
		virtual void Initialize( RSContext& context ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// Function ジェネリック型
	//////////////////////////////////////////////////////////////////////////

	class	RSGenericFunctionClass	: public RSFunctionClass
	{
	public:
		SSystem::SSmartPointer<RSFunctionPrototype>	m_pProtoGen ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSGenericFunctionClass, RSFunctionClass )
		// 構築関数
		RSGenericFunctionClass
			( RSClass * pClass,
				const wchar_t * pwszClassName,
				const RSFunctionPrototype & proto ) ;
		// メンバ初期設定
		virtual void Initialize( RSContext& context ) ;
		// 参照チェーンを解放
		virtual void ReleaseReferenceChain( void ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 例外型
	//////////////////////////////////////////////////////////////////////////

	class	RSExceptionClass	: public RSClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSExceptionClass, RSClass )
		// 構築関数
		RSExceptionClass( RSClass * pClass, const wchar_t * pwszClassName ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;

	public:
		// インスタンス生成
		virtual RSObject * NewInstance( RSContext& context, RSObject * pArg ) ;

	protected:
		// void <init>( String err )
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// NativeObject クラス
	//////////////////////////////////////////////////////////////////////////

	class	RNativeObjectClass	: public RSClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSClassClass, RSClass )
		// 構築関数
		RNativeObjectClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"NativeObject" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;

	protected:	// NativeObject method
		// void dispose()
		static RSObject * method_dispose
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// long getNativePointer()
		static RSObject * method_getNativePointer
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// NativeObject queryObject( Class cls )
		static RSObject * method_queryObject
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;

	class	RGenericNativeObjectClass	: public RNativeObjectClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RGenericNativeObjectClass, RNativeObjectClass )
		// 構築関数
		RGenericNativeObjectClass
			( RSClass * pClass, const wchar_t * pwszClassName ) ;
		// メンバ初期設定
		virtual void Initialize( RSContext& context ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// System 型クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSSystemClass	: public RSClass
	{
	public:
		// System.ShellResult クラス
		class	ShellResultClass	: public RSClass
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( ShellResultClass, RSClass )
			// 構築関数
			ShellResultClass
				( RSClass * pClass, const wchar_t * pwszClassName = L"ShellResult" ) ;
			// クラス固有仮想関数オーバーライドと
			// クラス static 変数のオーバーロード
			virtual void OverrideVirtuals( RSContext& context ) ;
			// Object <- ShellOpenResult 変換
			static void ResultToObject
				( RSContext& context, RSObject * obj,
					const SSystem::ShellOpenResult& sorResult ) ;
		} ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSSystemClass, RSClass )
		// 構築関数
		RSSystemClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"System" ) ;
		// 消滅関数
		virtual ~RSSystemClass( void ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;

	protected:	// System method
		// static Console console()
		static RSObject * method_console
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static long currentTimeMillis()
		static RSObject * method_currentTimeMillis
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static void trace( String fmt, ... )
		static RSObject * method_trace
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static Object eval( String expr )
		static RSObject * method_eval
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static boolean addReadableFilePath( String path, String id = null )
		static RSObject * method_addReadableFilePath
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static boolean addReadableStorageFile( String path, String id = null )
		static RSObject * method_addReadableStorageFile
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static boolean removeReadableFilePath( String id )
		static RSObject * method_removeReadableFilePath
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static boolean setWritableFilePath( String path )
		static RSObject * method_setWritableFilePath
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static void enableToWriteAllFilePath( boolean flagEnable )
		static RSObject * method_enableToWriteAllFilePath
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static String currentLanguageType()
		static RSObject * method_currentLanguageType
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static HashMap<String> getEnvironmentVariables()
		static RSObject * method_getEnvironmentVariables
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static boolean shellOpenFile
		//	( String strURL,
		//		String strAppPath = null, String strAppPlacement = null,
		//		int nFlags = 0, System.ShellResult pResult = null )
		static RSObject * method_shellOpenFile
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static String serializeToXML( Object obj ) ;
		static RSObject * method_serializeToXML
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static Uint8Pointer serializeObject( Object obj ) ;
		static RSObject * method_serializeObject
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static Object restoreFromXML( String xml ) ;
		static RSObject * method_restoreFromXML
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static Object restoreObject( Uint8Pointer bin ) ;
		static RSObject * method_restoreObject
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// Console 型クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSConsoleClass	: public RSClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSConsoleClass, RSClass )
		// 構築関数
		RSConsoleClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"Console" ) ;
		// 消滅関数
		virtual ~RSConsoleClass( void ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;

	protected:	// System method
		// void flush()
		static RSObject * method_flush
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Console printf( String fmt, ... )
		static RSObject * method_printf
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String readLine()
		static RSObject * method_readLine
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// String getCharEncoding()
		static RSObject * method_getCharEncoding
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean setCharEncoding( String sCharsetType )
		static RSObject * method_setCharEncoding
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// Math 型クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSMathClass	: public RSClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSMathClass, RSClass )
		// 構築関数
		RSMathClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"Math" ) ;
		// 消滅関数
		virtual ~RSMathClass( void ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;

	public:	// Math method
		// static double abs( double a )
		static RSObject * method_abs
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static double acos( double a )
		static RSObject * method_acos
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static double asin( double a )
		static RSObject * method_asin
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static double atan( double a )
		static RSObject * method_atan
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static double atan2( double y, double x )
		static RSObject * method_atan2
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static double cos( double a )
		static RSObject * method_cos
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static double sin( double a )
		static RSObject * method_sin
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static double tan( double a )
		static RSObject * method_tan
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static double log( double a )
		static RSObject * method_log
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static double log10( double a )
		static RSObject * method_log10
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static double max( double a, double b )
		static RSObject * method_max
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static double min( double a, double b )
		static RSObject * method_min
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static double pow( double a, double b )
		static RSObject * method_pow
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static double sqrt( double a )
		static RSObject * method_sqrt
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static double floor( double a )
		static RSObject * method_floor
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static double rint( double a )
		static RSObject * method_rint
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static long round( double a )
		static RSObject * method_round
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;


}

#endif

