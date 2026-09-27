
#if	!defined(__ROSETTA_VIRTUAL_MACHINE_H__)
#define	__ROSETTA_VIRTUAL_MACHINE_H__

#include <sakura/ssys_module.h>

namespace	Rosetta
{
	class	RSThread ;
	class	RSCompiler ;

	//////////////////////////////////////////////////////////////////////////
	// 仮想マシン
	//////////////////////////////////////////////////////////////////////////

	class	RSVirtualMachine	: public RSNamespace,
									public ECSSakura2::ExceptionHandler
	{
	protected:
		// Sakura2 仮想マシン
		ECSSakura2::StandardVM *			m_pSakura2VM ;
		SSystem::SObjectArray
			<ECSSakura2::ExecutableModule>	m_aModules ;

		class	CompiledInfo
		{
		public:
			RSFunctionPrototype *	m_pProto ;
			SSystem::SString		m_strFullName ;
		} ;
		SSystem::SULongPtrSortObjectArray<CompiledInfo>	m_ulsoaCompiledInfo ;

		// システムコンテキスト
		RSContext *	m_pContext ;
		RSContext *	m_pMacroCtx ;
		SSystem::SCriticalSection	m_csMacroCtx ;

		// システムスレッド
		RSThread *	m_pSysThread ;

		// 実行中スレッド
		RSThread *	m_pRunningThreads ;
		size_t		m_countRunningThreads ;

		// 基本型のクラス
		RSClass *	m_pMetaClass ;
		RSClass *	m_pVarClass ;
		RSClass *	m_pFuncClass ;
		RSClass *	m_pBooleanClass ;
		RSClass *	m_pIntegerClass ;
		RSClass *	m_pNumberClass ;
		RSClass *	m_pStringClass ;
		RSClass *	m_pArrayClass ;
		RSClass *	m_pArrayBufferClass ;
		RSClass *	m_pExceptionClass ;
		RSClass *	m_pJObjectClass ;
		RSClass *	m_pNObjectClass ;
		RSClass *	m_pJSObjectClass ;
		RSClass *	m_pStructureClass ;
		RSClass *	m_pBasicTypeClass[RSCodeControl::wiBasicTypeCount] ;
		RSClass *	m_pPtrTypeClass[RSReferenceNumber::typeCountOfNumber] ;

		// ジェネリック型
		RSGenericClassMembers	m_gcmGenClasses ;
		SSystem::SPointerArray<RSClass>
								m_aRegClasses ;

		// 実行中同期
		atomic_int_t			m_countRunning ;
		SSystem::SMutex			m_mutexAssertLock ;
		SSystem::SSignalEvent	m_sigAllLeaved ;

		// スクリプトパス
		SSystem::SObjectArray<SSystem::SString>			m_arrScriptPath ;
		SSystem::SPointerArray<SSystem::SFileOpener>	m_arrFileOpener ;

		// スクリプトソース
		SSystem::SStrSortObjectArray<RSScript>	m_ssoaScripts ;

		// 標準入出力
		SSystem::SBufferedFile *	m_pStdInput ;
		SSystem::SBufferedFile *	m_pStdOutput ;

		// ネイティブ関数
		SSystem::SStrSortArray<RSObject::METHOD_ENTRY>
								m_ssaNativeFuncs ;

		// サブ仮想マシン
		bool					m_flagRefVM ;

		// スクリプトデバッグフラグ
		bool					m_flagDebug ;

	public:
		// エラーログ
		class	ErrorLog
		{
		public:
			SSystem::SString	m_strErrMsg ;
			SSystem::SString	m_strFile ;
			SSystem::SString	m_strLineText ;
			size_t				m_nLineNum ;
			size_t				m_nColNum ;
		} ;

	protected:
		SSystem::SObjectArray<ErrorLog>	m_arrErrorLog ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSVirtualMachine, RSNamespace )
		// 構築関数
		RSVirtualMachine( void ) ;
		// 消滅関数
		virtual ~RSVirtualMachine( void ) ;

	public:
		enum	InitClassSet
		{
			classStandard	= 0x00000001,	// Java 互換の基本的なクラス
			classStdAppend	= 0x00000002,	// GLS4 の追加的な基本機能
			classCrypt		= 0x00000004,	// 暗号化関係
			classMedia		= 0x00000008,	// マルチメディア関係
			classPaint2D	= 0x00000010,	// 2D描画関係
			classRender3D	= 0x00000020,	// 3D描画関係
			classSprite		= 0x00000040,	// Sprite 関係
			classScene3D	= 0x00000080,	// 3D Scene 関係
			classSetAll		= 0xFFFFFFFF,
		} ;
		// 初期化
		void Initialize( uint32_t nInitClasses = classSetAll ) ;
		// システム定義クラスをインポートして初期化する
		void InitializeRefVM( RSVirtualMachine& vmProto ) ;
		// クラス以外の状態を初期化する
		void InitializeVMContext( void ) ;
		// 解放
		void Release( void ) ;
		// Sakura2 仮想マシン関連付け
		void AttachSakura2VM( ECSSakura2::StandardVM * pVM ) ;
		// スクリプトパスの追加
		void AddScriptPath( const wchar_t * pwszPath ) ;
		// 環境変数からスクリプトパスを追加
		void AddScriptEnvironmentPath
			( const wchar_t * pwszEnvName = L"ROSETTA_INCLUDE_PATH" ) ;
		// スクリプトファイル・オープナーを追加
		void AddScriptFileOpener( SSystem::SFileOpener * pOpener ) ;
		// スクリプトファイル・オープナーを削除
		void DetachScriptFileOpener( SSystem::SFileOpener * pOpener ) ;
		// 標準入出力関連付け
		void AttachStandardInput( SSystem::SBufferedFile * pStdIn ) ;
		void AttachStandardOutput( SSystem::SBufferedFile * pStdOut ) ;
		// 標準入出力取得
		SSystem::SBufferedFile & GetStandardInput( void ) const ;
		SSystem::SBufferedFile & GetStandardOutput( void ) const ;
		// デバッグフラグ
		bool IsDebug( void ) const ;
		void SetDebug( bool flagDebug ) ;
		// スクリプトファイルを開く
		SSystem::SFileInterface *
			OpenScriptFile( const wchar_t * pwszFilePath ) const ;
		// スクリプト読み込み
		RSScript * LoadScript
			( const wchar_t * pwszFilePath,
				SSystem::SParserErrorInterface& perr ) ;
		RSScript * LoadScript
			( RSContext& context, const wchar_t * pwszFilePath ) ;
		RSScript * AddScriptSource
			( const wchar_t * pwszSource,
				const wchar_t * pwszFilePath,
				SSystem::SParserErrorInterface& perr ) ;
		RSScript * AddScriptSource
			( RSContext& context,
				const wchar_t * pwszSource,
				const wchar_t * pwszFilePath ) ;
		// 単一の数式を評価（主に無名 function の生成の為）
		RSExpressionScript * CreateExpressionScript
			( const wchar_t * pwszExpr,
				SSystem::SParserErrorInterface& perr ) ;
		// スクリプト取得
		RSScript * GetLoadedScriptAs( const wchar_t * pwszFilePath ) const ;
		// 全クラス定義ダンプ
		void DumpAllClassDeclaration( SSystem::SFileInterface& file ) ;

	protected:
		// スクリプトパスの正規化
		static SSystem::SString
				NormalizeScriptPath( const wchar_t * pwszFilePath ) ;

	public:
		// Sakura2 コードへコンパイル
		SSystem::SError CompileToSakura2
			( bool flagFlatPointer,
				bool flagCompileToNative,
				SSystem::SParserErrorLogger& perr ) ;

	protected:
		SSystem::SError CompileNamespace
			( RSCompiler & compiler, RSObject * pObj,
				SSystem::SParserErrorLogger& perr ) ;
		SSystem::SError CompileFunction
			( RSCompiler & compiler,
				RSFunctionObject & func,
				SSystem::SParserErrorLogger& perr ) ;

	public:
		// スクリプト位置
		struct	ScriptPosition
		{
			const RSParenthesis *	pParenthesis ;
			size_t					iSource ;
		} ;
		// Sakura2 コード位置からスクリプト位置を検索
		bool SearchCodePosition( ScriptPosition& sp, int64_t ip ) const ;

	public:
		// 実行中スレッド登録
		void AddRunningThread( RSThread * pThread ) ;
		// 実行中スレッド解除
		void DetachRunningThread( RSThread * pThread ) ;
		// 実行中スレッド数取得
		size_t GetRunningThreadCount( void ) const ;
		// 全スレッドを強制終了
		void AbortAllThreads( void ) ;

	public:
		// マクロコンテキストの排他同期（スクリプト解釈同期用
		RSContext * LockMacroContext( void ) const ;
		void UnlockMacroContext( void ) const ;
		// スクリプト実行開始同期
		SSystem::SError EnterRunning( RSContext& context, int64_t msecTimeout ) ;
		SSystem::SError LeaveRunning( void ) ;
		// スクリプト実行排他同期
		SSystem::SError LockAssert( int64_t msecTimeout ) ;
		SSystem::SError UnlockAssert( void ) ;

	public:
		// Sakura2 仮想マシン
		ECSSakura2::StandardVM * GetSakura2VM( void ) const
		{
			return	m_pSakura2VM ;
		}
		// システムコンテキスト
		RSContext * GetSystemContext( void ) const
		{
			return	m_pContext ;
		}
		// クラス取得
		RSClass * GetClassAs( const wchar_t * pwszClassName ) const ;
		RSClass * GetClassClass( void ) const
		{
			return	m_pMetaClass ;
		}
		RSClass * GetVariableClass( void ) const
		{
			return	m_pVarClass ;
		}
		RSClass * GetFunctionClass( void ) const
		{
			return	m_pFuncClass ;
		}
		RSClass * GetBooleanClass( void ) const
		{
			return	m_pBooleanClass ;
		}
		RSClass * GetIntegerClass( void ) const
		{
			return	m_pIntegerClass ;
		}
		RSClass * GetNumberClass( void ) const
		{
			return	m_pNumberClass ;
		}
		RSClass * GetStringClass( void ) const
		{
			return	m_pStringClass ;
		}
		RSClass * GetArrayClass( void ) const
		{
			return	m_pArrayClass ;
		}
		RSClass * GetArrayBufferClass( void ) const
		{
			return	m_pArrayBufferClass ;
		}
		RSClass * GetExceptionClass( void ) const
		{
			return	m_pExceptionClass ;
		}
		RSClass * GetGenericObjectClass( void ) const
		{
			return	m_pJObjectClass ;
		}
		RSClass * GetNativeObjectClass( void ) const
		{
			return	m_pNObjectClass ;
		}
		RSClass * GetDynamicObjectClass( void ) const
		{
			return	m_pJSObjectClass ;
		}
		RSClass * GetStructureClass( void ) const
		{
			return	m_pStructureClass ;
		}
		RSClass * GetBasicTypeClass( RSCodeControl::WordIndex wiIndex ) const
		{
			ESLAssert( wiIndex >= RSCodeControl::wiFirstBasicType ) ;
			ESLAssert( wiIndex <= RSCodeControl::wiLastBasicType ) ;
			return	m_pBasicTypeClass
						[wiIndex - RSCodeControl::wiFirstBasicType] ;
		}
		RSClass * GetTypedPointerClass( RSReferenceNumber::NumberType type ) const
		{
			return	m_pPtrTypeClass[type] ;
		}
		RSClass * GetArrayClassAs
			( RSClass * pClass, int nArrayDimension = 1 ) ;
		RSClass * GetHashMapClassAs( RSClass * pClass ) ;
		RSClass * GetFunctionClassAs( const RSFunctionPrototype & proto ) ;
		RSClass * GetExceptioinClassAs( const wchar_t * pwszTypeClass ) ;

	public:
		// 拡張クラス定義
		void RegisterNewClass( RSClass * pClass, bool flagNoImpl = false ) ;
		void AddImplementClass( RSClass * pClass ) ;
		void ImplementNewClasses( void ) ;

	public:
		// ジェネリッククラス取得
		size_t GetGenericClassCount( void ) const ;
		RSClass * GetGenericClassAt( size_t nIndex ) const ;
		// 基本型クラス判定
		RSCodeControl::WordIndex IsBasicTypeClass( RSClass * pClass ) const ;
		// ポインタ型クラス判定
		RSReferenceNumber::NumberType IsTypedPointerClass( RSClass * pClass ) const ;
		// ジェネリック型クラス判定
		bool IsGenericTypeClass( RSClass * pClass ) const ;

	public:
		// メンバ取得
		virtual RSObject * GetMemberAs
			( RSContext& context, const wchar_t * pwszName ) const ;
		// メンバ設定
		virtual RSObject * SetMemberAs
			( RSContext& context, const wchar_t * pwszName, RSObject * pObj ) ;
		// メンバ新規作成
		virtual RSObject * CreateMemberAs
			( RSContext& context, const wchar_t * pwszName, RSObject * pObj ) ;

	public:
		// ネイティブ関数の設定（native 関数定義後）
		SSystem::SError SetNativeMethod
			( RSContext& context,
				const wchar_t * pwszMethodPath,
				RSObject::METHOD_PROC pfnMethod, void * pInstance = NULL ) ;
		// ネイティブ関数の設定（native 関数宣言前・事前登録）
		void RegisterNativeMethod
			( const wchar_t * pwszMethodPath, const RSObject::METHOD_ENTRY & method ) ;
		// ネイティブ関数取得（native 関数定義時）
		// （デフォルトは RegisterNativeMethod で登録された関数を取得）
		virtual SSystem::SError GetNativeMethod
			( RSObject::METHOD_ENTRY& method,
				RSContext& context, const wchar_t * pwszMethodPath ) ;

	public:
		// エラー出力
		virtual void OutputError
			( const wchar_t * pwszErrorMsg,
				const wchar_t * pwszFile,
				const wchar_t * pwszLineText,
				size_t nLineNum, size_t nColNum ) ;
		// エラーログ取得
		size_t GetErrorLogCount( void ) const ;
		ErrorLog * GetErrorLogAt( size_t i ) const ;
	protected:
		void OutputErrorLog
			( const SSystem::SParserErrorLogger& pelog, const wchar_t * pwszFile ) ;

	public:
		// ExceptionHandler 実装
		virtual bool HandleExceptionError
			( ECSSakura2::StandardVM * pVM,
				ECSSakura2Processor::Context * context, const wchar_t * pwszErr ) ;

	public:
		// ネイティブ簡易実装関数を登録
		void RegisterNativeMethodAllDescriptors( void ) ;
		// ネイティブ関数簡易実装用
		struct	NativeFuncDescriptor
		{
			const wchar_t *					pwszFuncName ;	// クラス名 _ メンバ名 ...
			RSObject::METHOD_PROC			pfnNativeProc ;
			const NativeFuncDescriptor *	pnfdNext ;
		} ;
		static const NativeFuncDescriptor *
				AddNativeFuncDescriptor( const NativeFuncDescriptor * pnfdDesc ) ;

	protected:
		static const NativeFuncDescriptor *	s_pnfdFirstDesc ;

	} ;

	// ネイティブ関数簡易宣言
	#define	DECL_ROSETTA_FUNC(func_name)	\
		extern const RSVirtualMachine::NativeFuncDescriptor	s_rosetta_desc_##func_name ;	\
		Rosetta::RSObject *	rosetta_func_stub_##func_name	\
			( Rosetta::RSContext& context, void * pInstance,	\
				Rosetta::RSObject* prsObjThis, Rosetta::RSObject** ppArg, size_t count ) ;	\
		Rosetta::RSObject *	rosetta_func_##func_name	\
			( Rosetta::RSContext& context, void * pInstance,	\
				Rosetta::RSObject* prsObjThis, Rosetta::RSContext::SArgList& arglist ) ;

	#define	DECL_ROSETTA_CONSTRUCTOR(class_name)	DECL_ROSETTA_FUNC(class_name)

	// ネイティブ関数簡易実装
	// func_name_str は Rosetta 上の名前
	// '.' を１つでも含んでいると '_' は '.' には置き換えられない 
	// 先頭が '_' から始まる場合には、先頭の1文字を取り除いたのち、'__' を '.' に置き換える
	#define	IMPL_ROSETTA_FUNC_NAME(func_name,func_name_str)	\
		const RSVirtualMachine::NativeFuncDescriptor	s_rosetta_desc_##func_name =	\
		{	\
			func_name_str,	\
			&rosetta_func_stub_##func_name,	\
			Rosetta::RSVirtualMachine::AddNativeFuncDescriptor( &s_rosetta_desc_##func_name ),	\
		} ;	\
		Rosetta::RSObject *	rosetta_func_stub_##func_name	\
			( Rosetta::RSContext& context, void * pInstance,	\
				Rosetta::RSObject* prsObjThis, Rosetta::RSObject** ppArgs, size_t countArg )	\
		{	\
			Rosetta::RSContext::SArgList	arglist( ppArgs, countArg ) ;	\
			return	rosetta_func_##func_name( context, pInstance, prsObjThis, arglist ) ;	\
		}	\
		Rosetta::RSObject *	rosetta_func_##func_name	\
			( Rosetta::RSContext& _context, void * _pInstance,	\
				Rosetta::RSObject* _prsObjThis, Rosetta::RSContext::SArgList& _arglist )

	// ネイティブ関数簡易実装
	// func_name は Rosetta 上のフルパスの '.' を '_' に置き換えたもの
	// クラス名や関数名に '_' を含む場合には先頭を '_' にし、'.' は '__' にする
	#define	IMPL_ROSETTA_FUNC(func_name)	\
				IMPL_ROSETTA_FUNC_NAME(func_name,L###func_name)

	#define	IMPL_ROSETTA_CONSTRUCTOR(class_name)	\
				IMPL_ROSETTA_FUNC_NAME(class_name,L###class_name L"_<init>")

	#define	RS_FUNC_THIS_NOBJ(type,name)	\
			type*	name = RSNativeObject::GetNative<type>( _prsObjThis ) ;	\
			ESLAssert( name != nullptr )

	#define	RS_FUNC_THIS_INIT_NOBJ(type,name,init_new_expr)	\
			RSNativeObject::SetNative( _prsObjThis, init_new_expr ) ;	\
			RS_FUNC_THIS_NOBJ(type,name)

	#define	RS_FUNC_ARG_INT(name,def_val)	\
			int		name = _arglist.NextInt( def_val )

	#define	RS_FUNC_ARG_LONG(name,def_val)	\
			int64_t	name = _arglist.NextLong( def_val )

	#define	RS_FUNC_ARG_BOOL(name,def_val)	\
			bool	name = _arglist.NextBoolean( def_val )

	#define	RS_FUNC_ARG_DOUBLE(name,def_val)	\
			double	name = _arglist.NextDouble( def_val )

	#define	RS_FUNC_ARG_STRING(name,def_val)	\
			SSystem::SString	name = _arglist.NextString( def_val )

	#define	RS_FUNC_ARG_OBJECT(name)	\
			Rosetta::RSSmartPtr	name = Rosetta::RSObject::AddRef( _arglist.NextObject() )

	#define	RS_FUNC_ARG_NOBJ(type,name)	\
			type*	name = ESLTypeCast<type>( _arglist.NextNativeObject() )

	#define	RS_FUNC_ARG_STRUCT(type,name)	\
			type*	name = (type*) _arglist.NextPointer( sizeof(type) )

	#define	RS_RETURN_INT(expr)		return	_context.new_Integer( expr )

	#define	RS_RETURN_DOUBLE(expr)	return	_context.new_Number( expr )

	#define	RS_RETURN_BOOL(expr)	return	_context.new_Boolean( expr )

	#define	RS_RETURN_VOID()		return	nullptr

}

#endif
