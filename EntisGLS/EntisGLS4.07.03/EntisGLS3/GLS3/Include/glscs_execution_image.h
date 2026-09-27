
//////////////////////////////////////////////////////////////////////////////
// Entis Cotopha Script 実行イメージ
//////////////////////////////////////////////////////////////////////////////

class	ECSExecutionImage	: public	ECSSakura2::StandardVM
{
public:
	// 構築関数
	ECSExecutionImage( void ) ;
	// 消滅関数
	virtual ~ECSExecutionImage( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( ECSExecutionImage, ECSSakura2::StandardVM )

public:
	ECSEnvironment *	m_pEnv ;		// 環境設定オブジェクト
	ECSSakura2::ExecutableModule
						m_module ;		// プライマリ・モジュール（ダミー）

	BYTE *				m_pImage ;			// 実行イメージ
	DWORD				m_dwImageSize ;

	enum	HeaderFlags
	{
		// 拡張格納データフラグ
		flagContainerExtRefClass	= 0x0001,	// m_extClassIndexRef
		flagContainerImpRefFunc		= 0x0100,	// m_impFuncRef
	} ;
	struct	HEADER
	{
		DWORD	nVersion ;					// バージョン（=1）
		DWORD	nIntBase ;					// Integer データサイズ
		DWORD	nContainerFlags ;			// 格納データフラグ
		DWORD	nReserved ;
		DWORD	nStackSize ;				// デフォルトスタックサイズ
		DWORD	nHeapSize ;					// デフォルトヒープサイズ
		DWORD	fnEntryPoint ;				// エントリポイント
		DWORD	fnStaticInitialize ;		// StaticInitialize
		DWORD	fnResumePrepare ;			// ResumePrepare
	} ;
	HEADER				m_exiHeader ;		// ヘッダー

	ECSGlobal			m_csgGlobal ;		// 大域変数
	ECSGlobal			m_csgData ;			// 大域定数

	class	ECSHeapDummy	: public ECSArray
	{
	protected:
		ECSSakura2::ObjectHeap *	m_pHeap ;
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( ECSHeapDummy, ECSArray )
		// 構築関数
		ECSHeapDummy( ECSSakura2::ObjectHeap * pHeap ) : m_pHeap(pHeap) {}
		// 全てのメンバ変数にインデックスを振る
		virtual void IndexAllMember( void ) ;
	} ;
	ECSHeapDummy		m_csaHeap ;			// 自由領域（オブジェクトモード保存用ダミー）
	ECSHeapDummy		m_csaHeapShared ;

	EObjArray<ECSString>
						m_lstConstStr ;		// 固定文字列リスト

	// 関数情報
	struct	FUNC_EXTENDED
	{
		DWORD	dwID ;
		DWORD	dwBytes ;
		void *	ptrData ;
		FUNC_EXTENDED *	ptrNext ;

		FUNC_EXTENDED( void )
			: dwID(0), dwBytes(0), ptrData(NULL), ptrNext(NULL) {}
		FUNC_EXTENDED( const FUNC_EXTENDED & ext )
			: dwID(ext.dwID), dwBytes(ext.dwBytes), ptrData(NULL), ptrNext(NULL)
		{
			ptrData = eslHeapAllocate( NULL, dwBytes, 0 ) ;
			eslMoveMemory( ptrData, ext.ptrData, dwBytes ) ;
			if ( ext.ptrNext != NULL )
			{
				ptrNext = new FUNC_EXTENDED( *ext.ptrNext ) ;
			}
		}
		~FUNC_EXTENDED( void )
		{
			eslHeapFree( NULL, ptrData ) ;
			delete ptrNext ;
		}
	} ;
	struct	FUNC_ENTRY_HEADER
	{
		DWORD	dwFlags ;
		DWORD	dwAddress ;
		DWORD	dwBytes ;
		DWORD	dwReserved ;

		FUNC_ENTRY_HEADER( void )
			: dwFlags(0), dwAddress(-1), dwBytes(-1), dwReserved(0) {}
		FUNC_ENTRY_HEADER( const FUNC_ENTRY_HEADER & fe )
			: dwFlags(fe.dwFlags), dwAddress(fe.dwAddress),
				dwBytes(fe.dwBytes), dwReserved(fe.dwReserved) {}
	} ;
	struct	FUNC_ENTRY	: public FUNC_ENTRY_HEADER
	{
		FUNC_EXTENDED *	ptrExtended ;

		FUNC_ENTRY( void ) : ptrExtended(NULL) {}
		FUNC_ENTRY( const FUNC_ENTRY & fe )
			: FUNC_ENTRY_HEADER(fe), ptrExtended(NULL)
		{
			if ( fe.ptrExtended != NULL )
			{
				ptrExtended = new FUNC_EXTENDED( *fe.ptrExtended ) ;
			}
		}
		~FUNC_ENTRY( void )
		{
			delete	ptrExtended ;
		}
	} ;
	typedef ETagSortArray<ECSWideString,FUNC_ENTRY>	EWStrFuncEntryArray ;

	// naked データシンボル情報
	struct	NAKED_SYMBOL_INFO
	{
		DWORD	dwFlags ;
		DWORD	dwReserved ;
		INT64	nAddress ;

		NAKED_SYMBOL_INFO( void )
			: dwFlags(0), dwReserved(0), nAddress(0) {}
		NAKED_SYMBOL_INFO( const NAKED_SYMBOL_INFO & syminf )
			: dwFlags(syminf.dwFlags),
				dwReserved(syminf.dwReserved),
				nAddress(syminf.nAddress) {}
	} ;
	typedef ETagSortArray<ECSWideString,NAKED_SYMBOL_INFO>	EWStrSymbolArray ;

	// クラス情報拡張データフラグ
	enum	ClassExtensionFlag
	{
		cxfNakedAddress	= 0x00000001,
	} ;

	// インポート情報配列
	typedef	ETagSortArray< ECSWideString, ENumArray<DWORD> >
											TaggedRefAddresList ;

protected:
	// 実行イメージ
	ECSCodeBuffer		m_bufImage ;		// 実行イメージバッファ

	// naked グローバル領域
	ECSBuffer			m_bufNakedGlobal ;
	ECSBuffer			m_bufNakedGlobalInit ;

	// naked 不変グローバル領域
	ECSReadOnlyBuffer	m_bufNakedConst ;

	// naked 共有グローバル領域
	ECSBuffer			m_bufNakedShared ;

	// 初期化情報
	ENumArray<DWORD>	m_pifPrologue ;			// 初期化関数アドレス
	ENumArray<DWORD>	m_pifEpilogue ;			// 終了関数アドレス
	ENumArray<DWORD>	m_pifNakedPrologue ;	// naked 初期化関数アドレス
	ENumArray<DWORD>	m_pifNakedEpilogue ;	// naked 終了関数アドレス
	ECSGlobal			m_csgGlobalType ;	// 大域変数、名前・型情報
	ECSGlobal			m_csgDataType ;		// 大域定数、名前・型情報
	EWStrFuncEntryArray	m_wstaFunc ;		// 関数名連想アドレス配列
	EWStrSymbolArray	m_wstaSymbols ;		// naked シンボル情報
	EObjArray<ECSClassInfo>
						m_lstClassInfo ;	// クラス情報配列

	// リンク用データ
	// ※コードイメージ上から各記憶クラスに対して参照している
	// 　コードイメージ上の位置を記録した配列
	// 　コードイメージ上には参照するアドレスが格納されており、
	// 　リンク時にはコードイメージ上の値が修正される
	ENumArray<DWORD>	m_extCodeRef ;		// コード参照リスト
	ENumArray<DWORD>	m_extGlobalRef ;	// 大域変数参照リスト
	ENumArray<DWORD>	m_extDataRef ;		// 大域定数参照リスト
	ENumArray<DWORD>	m_extClassIndexRef ;// クラス情報インデックス参照
	//
	ENumArray<DWORD>	m_extNakedFuncRef ;		// naked 関数参照リスト (64bit)
	ENumArray<DWORD>	m_extNakedGlobalRef ;	// naked グローバル参照リスト (64bit)
	ENumArray<DWORD>	m_extNakedConstRef ;	// naked 不変グローバル参照リスト (64bit)
	ENumArray<DWORD>	m_extNakedSharedRef ;	// naked 共有グローバル参照リスト (64bit)

	// ネイティブ関数呼び出し参照
	ECSStrBufTagArray	m_staNativeFuncName ;	// ネイティブ関数名
	ENumArray<DWORD>	m_impNativeFunc ;		// 参照インデックスアドレス
	ENumArray<DWORD>	m_impNakedNativeFunc ;

	// 外部参照（未解決変数）
	TaggedRefAddresList	m_impGlobalRef ;
	// 外部参照（未解決定数）
	TaggedRefAddresList	m_impDataRef ;
	// 外部参照（未解決変数参照）
	TaggedRefAddresList	m_impNakedGlobalRef ;	// 64bit アドレス
	TaggedRefAddresList	m_impNakedConstRef ;	// 64bit アドレス
	TaggedRefAddresList	m_impNakedSharedRef ;	// 64bit アドレス
	TaggedRefAddresList	m_impNakedFuncRef ;		// 64bit アドレス
	// 外部参照（未解決関数）
	TaggedRefAddresList	m_impFuncRef ;
	// 固定文字列参照
	TaggedRefAddresList	m_extConstStr ;

	// システム関数呼び出し用コンテキスト
	ECSContext *		m_pSystemContent ;

	EString				m_strErrMsg ;		// エラーメッセージ

protected:
	// ネイティブコードバッファ
	ECSSakura2JIT::CodeBuffer *	m_bufNativeCodes ;
	ECSSakura2JIT::CodeBuffer *	m_bufNativeGates ;

public:
	// クラス情報を保持する擬似オブジェクト
	class	ECSClassInfoObject	: public ECSStructure
	{
	public:
		EWideString	m_wstrClassName ;
	public:
		// クラス情報
		DECLARE_CLASS_INFO( ECSClassInfoObject, ECSStructure )
		// オブジェクトを複製
		virtual ECSObject * Duplicate( void ) ;
	} ;

protected:
	// クラス情報読み込み時に2パス処理するためのバッファ
	class	ECSDelayClassInfo
	{
	public:
		ECSClassInfo *				m_pClassInfo ;
		EObjArray<ECSWideString>	m_lstVarName ;
		EPtrObjArray<ECSTypeInfo>	m_lstVarType ;
		EPtrObjArray<ECSClassInfo::MemberFunction>	m_lstPrototype ;
		EStreamBuffer				m_bufExData ;
	public:
		// 構築関数
		ECSDelayClassInfo( ECSClassInfo * pClassInf )
							: m_pClassInfo( pClassInf ) {}
	} ;

	EObjArray<ECSDelayClassInfo>	m_lstDelayClassInfo ;

	// クラス情報を復元する（2パス）
	void RestoreDelayClassInfo( ECSDelayClassInfo * pDelayClass ) ;

public:	// ECSSakura2::VirtualMachine オーバーライド
	// メモリブロック確保
	virtual INT64 AllocateHeapMemory
		( DWORD dwBytes, 
			SSystem::MemoryAllocationMode mode = SSystem::mallocModeAuto ) ;
	// メモリブロック再確保
	virtual INT64 ReallocateHeapMemory( INT64 addrBlock, DWORD dwBytes ) ;
	// メモリブロック解放
	virtual void FreeHeapMemory
		( INT64 addrBlock, ECSSakura2Processor::Context * context ) ;

public:
	// 実行イメージを読み込む
	virtual ESLError ReadExecution( ESLFileObject & file ) ;
	// 実行イメージを書き出す
	virtual ESLError WriteExecution( ESLFileObject & file ) ;
	// 実行イメージを消去
	virtual void DeleteImage( void ) ;
	// 実行イメージ初期化
	virtual ESLError InitializeExecution( ECSContext & context ) ;
	// 実行リソースの解放
	virtual void ReleaseExecution( ECSContext & context ) ;
	// ネイティブクラス情報初期化
	virtual ESLError InitializeNativeClass( ECSContext & context ) ;
	// クラス情報初期化
	virtual ESLError InitializeClassInfo( void ) ;

public:	// ECSSakura2::VirtualMachine オーバーライド関数
	// クラス名からクラス ID を取得
	virtual int GetClassIdentity( const wchar_t * pwszClassName ) const ;
	// クラス ID を追加
	virtual int AddClassIdentity( const wchar_t * pwszClassName ) ;
	// クラス ID からオブジェクトを生成
	virtual ECSSakura2::Object * NewObjectByIdentity
			( ECSSakura2Processor::Context * context, int cls_id ) ;
	// エクスポート関数取得
	virtual void * GetModuleExportFunction( const char * pszFuncName ) ;

public:
	// オブジェクト仮想アドレスディレクトリ・アロケーション
	INT64 AllocateVirtualAddressDirectory
			( SSystem::SPointerArray<ECSObject> * plstDirectory ) ;
	INT64 AllocateVirtualAddressDirectory
			( INT64 nAddr, SSystem::SPointerArray<ECSObject> * plstDirectory ) ;
	// オブジェクト仮想アドレスディレクトリ解放
	void FreeVirtualAddressDirectory
		( INT64 nAddress, SSystem::SPointerArray<ECSObject> * plstDirectory ) ;
	// システムコンテキストを取得
	ECSContext * GetSystemContext( void ) ;

public:
	// クラス名ベクタを保存する
	ESLError SaveClassVector( ESLFileObject & file ) ;
	// 自由領域を保存する
	ESLError SaveHeapMemory( ECSContext& context, ESLFileObject & file ) ;
	// クラス名ベクタを復元する
	ESLError LoadClassVector( ESLFileObject & file ) ;
	// 自由領域を復元する
	ESLError LoadHeapMemory( ECSContext& context, ESLFileObject & file ) ;

protected:
	// 数値配列を読み込む
	ESLError ReadDWordArray
		( ESLFileObject & file, ENumArray<DWORD> & array ) ;
	// 文字列を読み込む
	ESLError ReadWideString
		( ESLFileObject & file, ECSWideString & wstr ) ;
	// 文字列配列を読み込む
	ESLError ReadWideStringArray
		( ESLFileObject & file, ECSStrBufTagArray & array ) ;
	ESLError ReadWideStringArray
		( ESLFileObject & file,
			SSystem::SIndexedArray
				<SSystem::SString,const wchar_t*> & array ) ;
	// タグ付き数値配列を読み込む
	ESLError ReadTagedDWordArray
		( ESLFileObject & file, TaggedRefAddresList & array ) ;
	// オブジェクトを読み込む
	ESLError ReadObject( ESLFileObject & file, ECSObject *& pObj ) ;
	// 型情報を読み込む
	ESLError ReadTypeObject( ESLFileObject & file, ECSObject *& pObj ) ;
	ESLError ReadTypeInfo
		( ESLFileObject & file, ECSTypeInfo & typeinf ) ;
	// プロトタイプ情報を読み込む
	ESLError ReadPrototypeInfo
		( ESLFileObject & file, ECSPrototypeInfo & protoinf ) ;
	// クラス情報を読み込む
	ESLError ReadClassInfo
		( ESLFileObject & file, ECSClassInfo & clsinf ) ;
	// 数値配列を書き出す
	ESLError WriteDWordArray
		( ESLFileObject & file, const ENumArray<DWORD> & array ) ;
	// 文字列を書き出す
	ESLError WriteWideString
		( ESLFileObject & file, const ECSWideString & wstr ) ;
	// 文字列配列を書き出す
	ESLError WriteWideStringArray
		( ESLFileObject & file, const ECSStrBufTagArray & array ) ;
	ESLError WriteWideStringArray
		( ESLFileObject & file,
			const SSystem::SIndexedArray
				<SSystem::SString,const wchar_t*> & array ) ;
	// タグ付き数値配列を書き出す
	ESLError WriteTagedDWordArray
		( ESLFileObject & file, const TaggedRefAddresList & array ) ;
	// オブジェクトを書き出す
	ESLError WriteObject( ESLFileObject & file, ECSObject * pObj ) ;
	// 型情報を書き出す
	ESLError WriteTypeObject( ESLFileObject & file, ECSObject * pObj ) ;
	ESLError WriteTypeInfo
		( ESLFileObject & file, const ECSTypeInfo & typeinf ) ;
	// プロトタイプ情報を書き出す
	ESLError WritePrototypeInfo
		( ESLFileObject & file, const ECSPrototypeInfo & protoinf ) ;
	// クラス情報を書き出す
	ESLError WriteClassInfo
		( ESLFileObject & file, const ECSClassInfo & clsinf ) ;

public:
	// ネイティブコード化
	void CompileToNativeCode( bool fNoBoundary ) ;

public:
	// 関数のアドレス取得
	DWORD * GetFunctionAddress( const wchar_t * pwszFuncName ) const ;
	// 関数情報エントリを取得する
	FUNC_ENTRY * GetFunctionEntry( const wchar_t * pwszFuncName ) const ;
	// 関数エントリを追加
	ESLError AddFunctionEntry
		( const wchar_t * pwszFuncName,
			DWORD dwPosition, DWORD dwFlags = 0 ) ;
	// 関数エントリの終了アドレス設定
	ESLError SetEndOfFunctionAddress
		( const wchar_t * pwszFuncName, DWORD dwPosition ) ;
	// 関数参照コードアドレス登録
	void AddCodeRefFunctionAddress
		( const wchar_t * pwszGlobalFuncName, DWORD dwCodeAddr ) ;
	void AddCodeRefFunctionAddress64
		( const wchar_t * pwszGlobalFuncName, DWORD dwCodeAddr ) ;
	// グローバルオブジェクトを取得
	ECSObject * GetGlobalObject( const wchar_t * pwszObjName ) ;
	// クラス数を取得
	int GetClassInfoCount( void ) const ;
	// クラスを検索
	virtual int GetClassInfoIndex( const wchar_t * pwszClassName ) const ;
	// クラス情報を取得
	ECSClassInfo * GetClassInfoAt( int nIndex ) const ;
	ECSClassInfo * GetClassInfoAs( const wchar_t * pwszClassName ) const ;
	// クラスを追加
	int AddClassInfo( ECSClassInfo * pClassInf ) ;
	// マージ元のクラス情報から正規のインポートクラス情報を取得
	ECSClassInfo * GetImportedClassInfo( const ECSClassInfo * pClassInf ) ;
	// クラス情報を複製して追加
	int ImportClassInfo( const ECSClassInfo & clsinfImport ) ;
	// クラス情報からメンバ情報を削除
	void CleanupClassMemberInfo( void ) ;
protected:
	// 関数プロトタイプのクラス情報を正規化
	void NormalizePrototypeClassInfo( ECSPrototypeInfo * pPrototype ) ;
	// 変数型のクラス情報を正規化
	void NormalizeVariableTypeClassInfo( ECSObject * pVarType ) ;

public:
	// シンボル情報
	const ECSGlobal & GetGlobalType( void ) const
		{
			return	m_csgGlobalType ;
		}
	const ECSGlobal & GetGlobalDataType( void ) const
		{
			return	m_csgDataType ;
		}
	const EWStrFuncEntryArray & GetFunctionEntries( void ) const
		{
			return	m_wstaFunc ;
		}
	const EObjArray<ECSClassInfo> & GetClassInfoList( void ) const
		{
			return	m_lstClassInfo ;
		}
	const ECSStrBufTagArray & GetNativeFuncNameList( void ) const
		{
			return	m_staNativeFuncName ;
		}
	// インポート情報
	const TaggedRefAddresList & GetImportGlobalRef( void ) const
		{
			return	m_impGlobalRef ;
		}
	const TaggedRefAddresList & GetImportDataRef( void ) const
		{
			return	m_impDataRef ;
		}
	const TaggedRefAddresList & GetImportNakedGlobalRef( void ) const
		{
			return	m_impNakedGlobalRef ;
		}
	const TaggedRefAddresList & GetImportNakedConstRef( void ) const
		{
			return	m_impNakedConstRef ;
		}
	const TaggedRefAddresList & GetImportNakedSharedRef( void ) const
		{
			return	m_impNakedSharedRef ;
		}
	const TaggedRefAddresList & GetImportNakedFuncRef( void ) const
		{
			return	m_impNakedFuncRef ;
		}
	const TaggedRefAddresList & GetImportFuncRef( void ) const
		{
			return	m_impFuncRef ;
		}
	// 実行環境取得
	ECSEnvironment * GetCSEnvironment( void ) const
		{
			return	m_pEnv ;
		}
	// 実行環境設定
	void AttachCSEnvironment( ECSEnvironment * pEnv )
		{
			m_pEnv = pEnv ;
			StandardVM::AttachEnvironment( pEnv ) ;
		}

	friend	ECSAssembler ;
	friend	ECSCompiler ;
	friend	ECSContext ;
	friend	ECSExecutionImageLinker ;
} ;

