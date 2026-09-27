
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/

#if	!defined(__GLSCS_SAKURA2_STANDARD_VM_H__)
#define	__GLSCS_SAKURA2_STANDARD_VM_H__

namespace	ECSSakura2
{
	//////////////////////////////////////////////////////////////////////////
	// 文字列索引ベクタ
	//////////////////////////////////////////////////////////////////////////

	template <class T> class	IndexedVector	: public SSystem::SArray<T>
	{
	public:
		typedef	SSystem::SIndexedArray
					<SSystem::SString,const wchar_t*>	EntryIndex ;
	protected:
		EntryIndex	m_Index ;

	public:
		// クラス名エントリ追加
		int AddEntry( const wchar_t * pwszName )
		{
			return	(int) m_Index.Add( new SSystem::SString( pwszName ) ) ;
		}
		// クラスエントリ指標取得
		int FindEntry( const wchar_t * pwszName ) const
		{
			return	(int) m_Index.FindIndex( pwszName ) ;
		}
		// 全て削除
		void RemoveAll( void )
		{
			// MS-C++ のバグか SSystem::SArray<T>::RemoveAll() は NG
			using	namespace SSystem ;
			SArray<T>::RemoveAll() ;
			m_Index.RemoveAll() ;
		}
		// インデックス配列取得
		EntryIndex & GetEntryIndex( void )
		{
			return	m_Index ;
		}
		const EntryIndex & GetEntryIndex( void ) const
		{
			return	m_Index ;
		}
	} ;


	//////////////////////////////////////////////////////////////////////////
	// オブジェクト生成ベクタ
	//////////////////////////////////////////////////////////////////////////

	class	NewObjectVector
				: public IndexedVector<VirtualMachine::PROC_NEW_OBJECT>
	{
	} ;


	//////////////////////////////////////////////////////////////////////////
	// システム関数ベクタ
	//////////////////////////////////////////////////////////////////////////

	class	SystemCallVector
				: public IndexedVector<ECSSakura2Processor::PROC_SYSCALL>
	{
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 例外ハンドラインターフェース
	//////////////////////////////////////////////////////////////////////////

	class	StandardVM ;
	class	ExceptionHandler
	{
	public:
		virtual bool HandleExceptionError
			( StandardVM * pVM, Context * context, const wchar_t * pwszErr ) = 0 ;
	} ;

}

#include <glscs/glscs_sakura2_module.h>

namespace	ECSSakura2
{
	using	ECSSakura2Processor::LinearAddressCache ;
	using	ECSSakura2Processor::Context ;
	using	ECSSakura2Processor::Register ;
	using	SSystem::SError ;
	using	SSystem::SFileInterface ;

	//////////////////////////////////////////////////////////////////////////
	// 標準的な仮想マシンの実装
	//////////////////////////////////////////////////////////////////////////

	class	StandardVM	: public VirtualMachine
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( StandardVM, VirtualMachine )
		// 構築関数
		StandardVM( void ) ;
		// 消滅関数
		virtual ~StandardVM( void ) ;

	protected:
		// リニアアドレス・ルートディレクトリ・テーブル
		SSystem::SPointerArray<Object> *	m_pAddressRootDirectory[roasMax] ;

		// モジュール・アロケーション
		SSystem::SPointerArray<ExecutableModule>	m_allocModules ;

		// 環境設定
		SSystem::SEnvironmentInterface *	m_pEnv ;

		// システム・コンテキスト（コールバック処理などに利用）
		ThreadObject *		m_pSysContext ;
		SSystem::SPointerArray<ThreadObject>
							m_lstTempContext ;

		// メインスレッド
		ThreadObject *		m_pMainThread ;

		// ヒープ索引
		class	HeapIndex
		{
		public:
			SSystem::SArray<DWORD>	m_indexHeapObj ;
			DWORD					m_dwCurrentHeap ;
		public:
			HeapIndex( void ) : m_dwCurrentHeap(0) {}
		} ;
		HeapIndex	m_indexHeap ;
		HeapIndex	m_indexSharedHeap ;

		// エラーメッセージ用テンポラリ
		SSystem::SString	m_strException ;

		// 例外エラーハンドラ
		ExceptionHandler *	m_pExceptionHandler ;

		// ネイティブ関数アドレス・オーバーライド
		SSystem::SStrSortArray<void*>	m_ssaExFuncAddr ;

	public:
		// ヌル・ディレクトリ
		SSystem::SPointerArray<Object>	m_ptblNull ;

		// 規定ディレクトリ
		SSystem::SPointerArray<Object>	m_ptblCodeImages ;
		SSystem::SPointerArray<Object>	m_ptblNakedGlobals ;
		SSystem::SPointerArray<Object>	m_ptblNakedConsts ;
		SSystem::SPointerArray<Object>	m_ptblNakedShareds ;

		// オブジェクト・ヒープ
		SSystem::MemoryAllocationMode	m_modeDefHeap ;
		ObjectHeap			m_heapGlobal ;
		ObjectHeap			m_heapShared ;
		ObjectHeap			m_heapThread ;

		// new object ベクタ（クラス ID インデックス）
		NewObjectVector		m_vectorNewObject ;

		// syscall ベクタ（syscall ID インデックス）
		SystemCallVector	m_vectorSysCall ;

	public:
		// 初期化
		virtual void InitializeVM( void ) ;
		// 仮想マシンの解放
		virtual void ReleaseVM( void ) ;
		// 環境設定を関連付け
		virtual void AttachEnvironment( SSystem::SEnvironmentInterface * pEnv ) ;
		// 環境設定を取得
		virtual SSystem::SEnvironmentInterface * GetEnvironment( void ) const ;
		// モジュールを仮想マシンにロード
		virtual const wchar_t * LoadModule
			( ThreadObject * pThread,
				ExecutableModule * pModule, int iModule = -1 ) ;
		virtual const wchar_t *
			LoadModuleByPrologueOnSysThread
				( ExecutableModule * pModule, int iModule = -1 ) ;
		// モジュールを仮想マシンから解放
		virtual const wchar_t * UnloadModule
			( ThreadObject * pThread, ExecutableModule * pModule ) ;
		virtual const wchar_t *
			UnloadModuleByEpilogueOnSysThread( ExecutableModule * pModule ) ;
		// メインスレッドを生成
		virtual ThreadObject * CreateMainThread( void ) ;
		// メインスレッドを取得
		ThreadObject * GetMainThread( void ) const ;
		// システムスレッドを取得／生成
		virtual ThreadObject * LockSystemThread( void ) ;
		// システムスレッドを解放
		virtual void UnlockSystemThread( ThreadObject * pSysThread ) ;
		// システムスレッド（非同期・一時）を取得／生成
		virtual ThreadObject * CreateSystemAsyncThread( void ) ;
		// システムスレッド（非同期・一時）を解放
		virtual void ReleaseSystemAsyncThread( ThreadObject * pSysThread ) ;
		// システムスレッド上で関数を呼び出し
		virtual INT64 CallFunctionOnSysThread
			( INT64 addrFunc, const Register *pArg, int nArgCount ) ;
		virtual INT64 CallAsyncFunctionOnSysThread
			( INT64 addrFunc, const Register *pArg, int nArgCount ) ;
		// システムスレッド上で仮想関数を呼び出し
		virtual INT64 CallVirtualOnSysThread
			( INT64 addrObj, int iVirtual, const Register *pArg, int nArgCount ) ;
		virtual INT64 CallAsyncVirtualOnSysThread
			( INT64 addrObj, int iVirtual, const Register *pArg, int nArgCount ) ;

	public:	// VirtualMachine オーバーライド関数
		// アドレスからオブジェクトへ変換
		virtual Object * ObjectFromAddress( DWORD dwHighAddr ) const ;
		// アドレスからセグメント情報へ変換
		virtual LinearAddressCache *
			SegmentFromAddress( LinearAddressCache * plac, DWORD dwHighAddr ) const ;
		// クラス名からクラス ID を取得
		virtual int GetClassIdentity( const wchar_t * pwszClassName ) const ;
		// クラス ID を追加
		virtual int AddClassIdentity( const wchar_t * pwszClassName ) ;
		// クラス ID からオブジェクトを生成
		virtual Object * NewObjectByIdentity( Context * context, int cls_id ) ;
		// システム関数 ID から syscall 関数呼び出し
		virtual const wchar_t * SystemCallByIdentity
			( Context * context, int syscall_id, const Register * pArg ) ;

	public:
		// new object 関数取得
		virtual VirtualMachine::PROC_NEW_OBJECT
					GetNewObjectProc( const wchar_t * pwszClassName ) ;
		// syscall 関数取得
		virtual ECSSakura2Processor::PROC_SYSCALL
					GetSystemCallProc( const wchar_t * pwszSysCall ) ;
		// エクスポート関数取得
		virtual void * GetModuleExportFunction( const wchar_t * pszFuncName ) ;

	public:
		struct	EX_FUNC_ENTRY
		{
			const wchar_t * pszFuncName ;
			void *			pFuncAddr ;
		} ;
		// GetModuleExportFunction で取得できる関数アドレスの登録
		void RegisterExportFunction
			( const wchar_t * pszFuncName, void * pFuncAddr ) ;
		void RegisterExportFunctions
			( const EX_FUNC_ENTRY * pFuncEntries, ssize_t nCount = -1 ) ;
		// クラスや関数名をインポートする名前を結合する
		void AppendImportCotophaSymbol
			( SSystem::SString & strSymbol, const wchar_t * pwszSymbol ) ;

	public:
		// ディレクトリ・テーブルを初期化
		virtual void InitializeDirectoryTable( void ) ;
		// デフォルトスタックサイズ取得
		virtual size_t GetDefaultStackSize( void ) const ;
		// デフォルトヒープサイズ取得
		virtual size_t GetDefaultHeapSize( void ) const ;

	public:
		// メモリブロック確保
		virtual INT64 AllocateHeapMemory
			( DWORD dwBytes, 
				SSystem::MemoryAllocationMode mode = SSystem::mallocModeAuto ) ;
		// メモリブロック再確保
		virtual INT64 ReallocateHeapMemory( INT64 addrBlock, DWORD dwBytes ) ;
		// メモリブロック解放
		virtual void FreeHeapMemory( INT64 addrBlock, Context * context ) ;

	protected:
		// ヒープ索引を更新
		void UpdateHeapIndexTable( HeapIndex& indexHeap, DWORD sel ) ;
		// ヒープ上にメモリブロック確保
		INT64 AllocateHeapBlockMemory
			( DWORD dwBytes, SSystem::MemoryAllocationMode mode ) ;

	public:
		// オブジェクトヒープにアドレスを確保
		virtual INT64 AllocateHeapObjectAddress
			( Object * pObj,
				SSystem::MemoryAllocationMode mode = SSystem::mallocModeAuto ) ;
		virtual INT64 AllocateHeapObjectAddress( INT64 nAddr, Object * pObj ) ;
		// オブジェクトヒープを解放
		virtual void FreeHeapObjectAddress( INT64 nAddress, Context * context ) ;
		// スレッド配列を参照
		ObjectHeap& GetThreadHeap( void )
		{
			return	m_heapThread ;
		}

	public:
		// 同期オブジェクト待機処理
		virtual SError WaitSynchronism
			( Context * context,
				SSystem::SSynchronismInterface * pSync, int64_t msecTimeout ) ;

	public:
		// 処理されない例外エラー処理
		virtual void HandleExceptionError
			( Context * context, const wchar_t * pwszErr ) ;
		// 例外エラーハンドラ設定
		void AttachExceptionHandler( ExceptionHandler * pHandler ) ;

		// スタックフレームのトレース
		struct	STACK_FRAME_INFO
		{
			const wchar_t *					pwszFuncName ;
			ExecutableModule::FUNC_ENTRY *	pFuncEntry ;
			Register *						pFuncArg ;
			size_t							countArg ;
			Register						ipReturn ;
			Register						bpReturn ;
		} ;
		SError TraceStackFrameInfo
			( STACK_FRAME_INFO& sfi, Context * context,
					const Register& ip, const Register& bp ) ;
		// スタックフレーム情報の表示形式フォーマット
		SSystem::SString
			FormatStackFrameArguments( const STACK_FRAME_INFO& sfi ) ;
		SSystem::SString
			FormatStackFrameArgumentValue( const Register& value ) ;
		// メモリダンプのトレース
		bool FormatMemoryDump
			( SSystem::SString& strDump,
				const wchar_t * pwszPointer, INT64 nAddress ) ;
		// 16進メモリダンプ表示形式フォーマット
		SSystem::SString FormatMemoryDumpLine( INT64 nAddress, size_t nCount ) ;

	public:
		// ファイルを開く
		virtual SSystem::SFileInterface * NewOpenFile
				( const wchar_t * pwszFilePath, long int nOpenFlags ) const ;
		// ファイルは存在するか？
		virtual bool IsExistingFile( const wchar_t * pwszFilePath ) const ;

	public:
		// モジュールをアロケーション
		int AllocateModule( ExecutableModule * pModule ) ;
		int AllocateModuleAt( int iModule, ExecutableModule * pModule ) ;
		// モジュールのアロケーションを解放
		void FreeModuleAllocation( ExecutableModule * pModule ) ;
		// モジュール番号をアロケーション
		int AttachModule( ExecutableModule * pModule ) ;
		// 特定のモジュール番号にアロケーション
		int AttachModuleAt( int iModule, ExecutableModule * pModule ) ;
		// モジュール番号を検索
		int FindModule( ExecutableModule * pModule ) ;
		// モジュール番号を解放
		void DetachModule( ExecutableModule * pModule ) ;
		void DetachModuleAt( int iModule ) ;
		// モジュール総数取得
		size_t GetModuleCount( void ) const ;
		// モジュール取得
		ExecutableModule * GetModuleAt( int iModule ) ;
		// 全モジュールの関数を検索
		virtual uint64_t GetFunctionAddress
			( const wchar_t * pwszFuncName, const wchar_t * pszReserved ) ;
		// コードアドレスから関数情報を検索
		class	SearchFunctionInfo
		{
		public:
			ExecutableModule *				pModule ;
			ExecutableModule::FUNC_ENTRY *	pFuncEntry ;
			const wchar_t *					pwszFuncName ;
		} ;
		virtual SError SearchFunctionAtAddress
					( SearchFunctionInfo& sfi, INT64 nAddress ) ;

	public:
		// メインスレッドと new object & syscall ベクタの保存
		SSystem::SError SaveMainThreadAndSysVector( SFileInterface * pfile ) ;
		// new object ベクタ保存
		SSystem::SError SaveNewObjectVector( SFileInterface * pfile ) ;
		// syscall ベクタ保存
		SSystem::SError SaveSystemCallVector( SFileInterface * pfile ) ;
		// メインスレッドと new object & syscall ベクタの復元
		SSystem::SError LoadMainThreadAndSysVector( SFileInterface * pfile ) ;
		// new object ベクタ復元
		SSystem::SError LoadNewObjectVector( SFileInterface * pfile ) ;
		// syscall ベクタ復元
		SSystem::SError LoadSystemCallVector( SFileInterface * pfile ) ;

	protected:
		// ベクタ保存
		SSystem::SError SaveStringIndexedArray
			( const SSystem::SIndexedArray
					<SSystem::SString,const wchar_t*> & indexArray,
						SFileInterface * pfile ) ;
		// ベクタ復元
		SSystem::SError LoadStringIndexedArray
			( SSystem::SIndexedArray
					<SSystem::SString,const wchar_t*> & indexArray,
						SFileInterface * pfile ) ;

	public:	// Object オーバーライド
		// 実行時型名
		virtual const wchar_t * GetTypeName( void ) const ;
		// 破棄処理
		virtual void OnDestruction
			( VirtualMachine * vm, Context * context ) ;
		// 保存準備処理
		virtual SError PrepareSave
			( VirtualMachine * vm, Context * context ) ;
		// 保存処理
		virtual SError SaveStatic
			( SFileInterface * file,
				VirtualMachine * vm, Context * context ) ;
		virtual SError SaveDynamic
			( SFileInterface * file,
				VirtualMachine * vm, Context * context ) ;
		// 復元処理
		virtual SError LoadStatic
			( SFileInterface * file,
				VirtualMachine * vm, Context * context ) ;
		virtual SError LoadDynamic
			( SFileInterface * file,
				VirtualMachine * vm, Context * context ) ;
		// 復元後処理
		virtual SError CommitAfterLoad
			( VirtualMachine * vm, Context * context ) ;
		// 復元後の後のスクリプト処理
		virtual SError OnLoadedDynamic
			( VirtualMachine * vm, Context * context ) ;

	} ;


}


#endif
