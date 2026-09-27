
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/

#if	!defined(__GLSCS_SAKURA2_MODULE_H__)
#define	__GLSCS_SAKURA2_MODULE_H__

namespace	ECSSakura2
{
	using	SSystem::SError ;
	using	SSystem::SFileInterface ;


	//////////////////////////////////////////////////////////////////////////
	// 詞葉モジュール
	//////////////////////////////////////////////////////////////////////////

	class	ExecutableModule	: public	ESLObject
	{
	public:
		// ヘッダ
		enum	HeaderFlags
		{
			// 拡張格納データフラグ
			flagContainerExtRefClass	= 0x0001,	// m_reallcRefClassId
			flagContainerImpRefFunc		= 0x0100,	// m_importRefCode
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
		// シンボルフラグ
		enum	SymbolFlags
		{
			flagNakedCall		= 0x00000020,	// naked (Sakura2) 関数
		} ;
		// 関数情報（コード領域シンボル）
		struct	FUNC_ENTRY_HEADER
		{
			DWORD	dwFlags ;
			DWORD	dwAddress ;
			DWORD	dwBytes ;
			DWORD	dwReserved ;

			FUNC_ENTRY_HEADER( void )
				: dwFlags(0), dwAddress(0), dwBytes(0), dwReserved(0) {}
			FUNC_ENTRY_HEADER( const FUNC_ENTRY_HEADER& feh )
				: dwFlags(feh.dwFlags), dwAddress(feh.dwAddress),
					dwBytes(feh.dwBytes), dwReserved(feh.dwReserved) {}
		} ;
		enum	FuncExtendedDataID
		{
			fexidSimplePrototype	= 1,	// 簡易プロトタイプ (UTF-8 文字列)
											// return-type '(' argument-list ')'
											// 'V':void, 'P':pointer, 'T':this pointer
											// 'I':int64, 'D':double, '.':vararg
											// 'O':object pointer etc. OSSystem::SObject;
		} ;
		struct	FUNC_ENTRY_EXTENDED
		{
			DWORD						dwID ;
			DWORD						dwBytes ;
			SSystem::SArray<uint8_t>	bufData ;

			FUNC_ENTRY_EXTENDED( void ) : dwID(0), dwBytes(0) {}
			FUNC_ENTRY_EXTENDED( const FUNC_ENTRY_EXTENDED& fex )
				: dwID(fex.dwID), dwBytes(fex.dwBytes),  bufData(fex.bufData) {}
			~FUNC_ENTRY_EXTENDED( void ) {}
		} ;
		struct	FUNC_ENTRY	: public FUNC_ENTRY_HEADER
		{
			SSystem::SObjectArray<FUNC_ENTRY_EXTENDED>	listExtended ;

			FUNC_ENTRY( void ) {}
			FUNC_ENTRY( const FUNC_ENTRY& fe )
				: FUNC_ENTRY_HEADER(fe), listExtended(fe.listExtended) {}
			~FUNC_ENTRY( void ) {}
		} ;
		// シンボル情報（データ領域）
		struct	SYMBOL_INFO
		{
			DWORD	dwFlags ;
			DWORD	dwReserved ;
			INT64	nAddress ;
		} ;
		// 文字列配列（検索インデックス付き）
		typedef	SSystem::SIndexedArray
					<SSystem::SString,const wchar_t*>	StringIndexedArray ;
		// 関数情報配列
		typedef	SSystem::SStrSortArray<FUNC_ENTRY>	CodeSymbolArray ;
		// シンボル情報配列
		typedef	SSystem::SStrSortArray<SYMBOL_INFO>	DataSymbolArray ;
		// アドレス参照アドレス配列（リアロケーション用）
		typedef	SSystem::SArray<DWORD>	ReallocationArray ;
		// シンボル・インポート・アドレス配列
		typedef	SSystem::SStrSortObjectArray<ReallocationArray>	TaggedImportArray ;

	public:
		// 割り当てられたモジュール番号
		int					m_iModule ;

		// ヘッダ情報
		HEADER				m_exmHeader ;

		// イメージ・バッファ
		DualBufferObject	m_bufCode ;		// コード
		BufferObject		m_bufGlobal ;	// 大域変数
		BufferObject		m_bufConst ;
		BufferObject		m_bufShared ;

		// 初期化・終了処理関数アドレス配列
		SSystem::SArray<DWORD>	m_vectorPrologue ;
		SSystem::SArray<DWORD>	m_vectorEpilogue ;

		// シンボル情報
		StringIndexedArray	m_indexClass ;		// クラス名配列
		StringIndexedArray	m_indexSysCall ;	// システムコール関数名配列
		CodeSymbolArray		m_symbolCode ;		// 関数シンボル配列
		DataSymbolArray		m_symbolData ;		// データシンボル配列

		// ファイル・オープナー
		SSystem::SSmartPointer<SSystem::SFileOpener>	m_pOpener ;

	protected:
		// リアロケーション情報（参照先別）
		ReallocationArray	m_reallcRefCode ;
		ReallocationArray	m_reallcRefGlobal ;
		ReallocationArray	m_reallcRefConst ;
		ReallocationArray	m_reallcRefShared ;
		ReallocationArray	m_reallcRefClassId ;
		ReallocationArray	m_reallcRefSysCallId ;

		// 外部インポート情報（参照先別）
		TaggedImportArray	m_importRefCode ;
		TaggedImportArray	m_importRefGlobal ;
		TaggedImportArray	m_importRefConst ;
		TaggedImportArray	m_importRefShared ;

		// ネイティブコードバッファ
		SSystem::SSmartPointer<ECSSakura2JIT::CodeBuffer>	m_bufNativeCodes ;
		SSystem::SSmartPointer<ECSSakura2JIT::CodeBuffer>	m_bufNativeGates ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( ExecutableModule, ESLObject )
		// 構築関数
		ExecutableModule( void ) ;
		// 消滅関数
		virtual ~ExecutableModule( void ) ;

	public:
		// コード・リアロケーション
		virtual void ReallocateModule
			( int iModule,
				NewObjectVector& vecotorClass,
				SystemCallVector& vectorSysCall ) ;
		// シンボル・インポート解決
		virtual int ImportSymbols
			( const SSystem::SPointerArray<ExecutableModule> & modules ) ;

	protected:
		// 参照アドレス・リアロケーション
		void ReallocateRefAddress( const ReallocationArray& reallcRef ) ;
		// ID リアロケーション
		void ReallocateRefIdentity
			( SSystem::SIndexedArray
				<SSystem::SString,const wchar_t*>& indexDstId,
					const StringIndexedArray& indexSrcId,
					const ReallocationArray& reallcRef ) ;
		// 関数アドレス・インポート解決
		int ImportFunctionSymbols
			( const SSystem::SPointerArray<ExecutableModule> & modules,
							const TaggedImportArray& importRefCode ) ;
		// 変数アドレス・インポート解決
		int ImportDataSymbols
			( const SSystem::SPointerArray<ExecutableModule> & modules,
							const TaggedImportArray& importRefData ) ;

	public:
		// リソースを削除する
		virtual void DeleteModule( void ) ;
		// モジュールファイルを読み込む
		virtual SError ReadModule( SFileInterface * pFile ) ;
		// ネイティブコード化
		virtual void CompileToNativeCode
			( bool fNoBoundary, uint64_t maskCpuFeatures = -1 ) ;
		// JIT コンパイラ機能評価
		static uint32_t GetJITCompilerFeatures( void ) ;

	protected:
		// DWORD 配列を読み込む
		SError ReadDWordArray
			( SFileInterface & file, SSystem::SArray<DWORD> & arrayDWords ) ;
		// 文字列を読み込む
		SError ReadWideString
			( SFileInterface & file, SSystem::SString & strSymbol ) ;
		// 文字列配列を読み込む
		SError ReadWideStringArray
			( SFileInterface & file, StringIndexedArray & arrayStrings ) ;
		// インポート参照配列を読み込む
		SError ReadTaggedDWordArray
			( SFileInterface & file, TaggedImportArray & importRefs ) ;

	public:
		// 関数検索
		FUNC_ENTRY * GetFunctionEntry( const wchar_t * pwszFuncName ) const
			{
				return	m_symbolCode.GetAs( pwszFuncName ) ;
			}
		// 変数検索
		SYMBOL_INFO * GetVariableEntry( const wchar_t * pwszVarName ) const
			{
				return	m_symbolData.GetAs( pwszVarName ) ;
			}
		// アドレスを含む関数を検索
		const wchar_t * SearchFunctionAtAddress
			( DWORD dwAddress, FUNC_ENTRY** ppFuncEntry ) ;
		// コード逆アセンブル・デバッグ出力
		void DebugTraceDisassemble( DWORD dwAddress, DWORD dwBytes ) const ;

	} ;
}

#endif

