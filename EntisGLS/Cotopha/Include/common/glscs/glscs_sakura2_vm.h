
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/

#if	!defined(__GLSCS_SAKURA2_VIRTUAL_MACHINE_H__)
#define	__GLSCS_SAKURA2_VIRTUAL_MACHINE_H__

namespace	ECSSakura2
{
	using	ECSSakura2Processor::LinearAddressCache ;
	using	ECSSakura2Processor::Context ;
	using	ECSSakura2Processor::Register ;
	using	SSystem::SError ;
	using	SSystem::SFileInterface ;

	class	VirtualMachine ;
	class	ThreadObject ;
	class	ExecutableModule ;


	//////////////////////////////////////////////////////////////////////////
	// ネイティブ・オブジェクト基底
	//////////////////////////////////////////////////////////////////////////

	class	Object	: public SSystem::SObject
	{
	public:
		DWORD	m_dwHighAddr ;		// 割り当てられたアドレス上位32ビット

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( Object, SSystem::SObject )
		// 構築関数
		Object( void ) : m_dwHighAddr(0) {}
		// メモリマッピング
		virtual LinearAddressCache *
				GetSegmentBuffer( LinearAddressCache & seg ) ;
		virtual BYTE * GetSegmentShadowBuffer( int iShadow = 0 ) ;

	public:
		// 実行時型名
		virtual const wchar_t * GetTypeName( void ) const = 0 ;
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

	public:
		// オブジェクトの上位アドレス取得
		static DWORD GetHighAddressOf( const ESLObject * pObj ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// ネイティブ・オブジェクト・参照（マッピング・ゲート）
	//////////////////////////////////////////////////////////////////////////

	class	ReferenceObject	: public ECSSakura2::Object, public SSystem::SSyncReference
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2( ReferenceObject, ECSSakura2::Object, SSyncReference )

	protected:
		// 全ての参照が解除された
		virtual void OnReleaseFromReference( void ) ;

	private:
		DWORD	m_dwLoadedRefHighAddr ;

	public:
		// メモリマッピング
		virtual LinearAddressCache *
				GetSegmentBuffer( LinearAddressCache & seg ) ;
		virtual BYTE * GetSegmentShadowBuffer( int iShadow = 0 ) ;
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

// 関数記述支援マクロ
#define	ECS_LIB_DECLARE_EXPORT_NEW_OBJECT(func)	\
	ECS_LIB_EXPORT ECSSakura2::Object * ecs_new_object_##func	\
		( ECSSakura2Processor::Context * context, int cls_id )
#define	ECS_LIB_IMPLEMENT_EXPORT_NEW_OBJECT(func,context,cls_id)	\
	ECS_LIB_EXPORT ECSSakura2::Object * ecs_new_object_##func	\
		( ECSSakura2Processor::Context * context, int cls_id )
#define	ECS_DECLARE_EXPORT_NEW_OBJECT(func)	\
	ECS_EXPORT ECSSakura2::Object * ecs_new_object_##func	\
		( ECSSakura2Processor::Context * context, int cls_id )
#define	ECS_IMPLEMENT_EXPORT_NEW_OBJECT(func,context,cls_id)	\
	ECS_EXPORT ECSSakura2::Object * ecs_new_object_##func	\
		( ECSSakura2Processor::Context * context, int cls_id )

// new SSystem::Reference
ECS_LIB_DECLARE_EXPORT_NEW_OBJECT(SSystem_Reference) ;


namespace	ECSSakura2
{
	//////////////////////////////////////////////////////////////////////////
	// 抽象仮想マシン
	//////////////////////////////////////////////////////////////////////////

	class	VirtualMachine	: public Object
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( VirtualMachine, ECSSakura2::Object )

	public:
		// リニアアドレス・ディレクトリ・セレクタ
		enum	RefObjectAddrSelector
		{
			roasNull,
			roasCode,
			roasObjectGlobal,
			roasObjectData,
			roasNakedGlobal,
			roasNakedConst,
			roasNakedShared,
			roasNakedHeap,
			roasNakedSharedHeap,
			roasNakedThread,
			roasFree	= 0x10,
			roasMax		= 0x100,
		} ;

		// クラス ID
		enum	NewObjectClassID
		{
			clsidInvalid	= -1,
		} ;

		// new object 関数
		typedef	ECSSakura2::Object * (*PROC_NEW_OBJECT)( Context * context, int cls_id ) ;

		// アドレスからオブジェクトへ変換
		virtual ECSSakura2::Object * ObjectFromAddress( DWORD dwHighAddr ) const = 0 ;
		// アドレスからセグメント情報へ変換
		virtual LinearAddressCache *
			SegmentFromAddress( LinearAddressCache * plac, DWORD dwHighAddr ) const = 0 ;

		// クラス名からクラス ID を取得
		virtual int GetClassIdentity( const wchar_t * pwszClassName ) const = 0 ;
		// クラス ID を追加
		virtual int AddClassIdentity( const wchar_t * pwszClassName ) = 0 ;
		// クラス ID からオブジェクトを生成
		virtual ECSSakura2::Object * NewObjectByIdentity( Context * context, int cls_id ) = 0 ;
		// システム関数 ID から syscall 関数取得
		virtual const wchar_t * SystemCallByIdentity
			( Context * context, int syscall_id, const Register * pArg ) = 0 ;

		// 環境設定を取得
		virtual SSystem::SEnvironmentInterface * GetEnvironment( void ) const ;
		// デフォルトスタックサイズ取得
		virtual size_t GetDefaultStackSize( void ) const ;
		// デフォルトヒープサイズ取得
		virtual size_t GetDefaultHeapSize( void ) const ;

		// メモリブロック確保
		virtual INT64 AllocateHeapMemory
			( DWORD dwBytes, 
				SSystem::MemoryAllocationMode mode = SSystem::mallocModeAuto ) = 0 ;
		// メモリブロック再確保
		virtual INT64 ReallocateHeapMemory( INT64 addrBlock, DWORD dwBytes ) = 0 ;
		// メモリブロック解放
		virtual void FreeHeapMemory( INT64 addrBlock, Context * context ) = 0 ;

		// オブジェクトヒープにアドレスを確保
		virtual INT64 AllocateHeapObjectAddress
			( ECSSakura2::Object * pObj,
				SSystem::MemoryAllocationMode mode = SSystem::mallocModeAuto ) = 0 ;
		// オブジェクトヒープを解放
		virtual void FreeHeapObjectAddress
				( INT64 nAddress, Context * context ) = 0 ;

		// 同期オブジェクト待機処理
		virtual SError WaitSynchronism
			( Context * context,
				SSystem::SSynchronismInterface * pSync, int64_t msecTimeout ) = 0 ;

		// 処理されない例外エラー処理
		virtual void HandleExceptionError
			( Context * context, const wchar_t * pwszErr ) = 0 ;
		// デバッグ用例外エラー処理
		virtual DWORD HandleExceptionEscape
				( Context * context, DWORD maskException ) ;
		// モジュールがアタッチされた（デバッグ・フック処理用）
		virtual void OnModuleAttached( int iModule, ExecutableModule * module ) ;
		// モジュールがデタッチされる（デバッグ・フック処理用）
		virtual void OnModuleDetached( int iModule, ExecutableModule * module ) ;
		// スレッドがアタッチされた（デバッグ・フック処理用）
		virtual void OnThreadAttached( ThreadObject * thread ) ;
		// スレッドがデタッチされた（デバッグ・フック処理用）
		virtual void OnThreadDetached( ThreadObject * thread ) ;

		// ロードされているモジュール内関数検索
		virtual uint64_t GetFunctionAddress
			( const wchar_t * pwszFuncName, const wchar_t * pszReserved ) = 0 ;

		// ファイルを開く
		virtual SSystem::SFileInterface * NewOpenFile
				( const wchar_t * pwszFilePath, long int nOpenFlags ) const ;
		// ファイルは存在するか？
		virtual bool IsExistingFile( const wchar_t * pwszFilePath ) const ;
		// ファイル状態
		virtual SError QueryFileState
			( const wchar_t * pszFilePath, SSystem::SFileOpener::State& state ) ;
		// ファイルパス
		virtual SSystem::SString OffsetFilePath( const wchar_t * pwszFilePath ) const ;

	protected:
		SSystem::SCriticalSection	m_csMutex ;

	public:
		// printf パラメータ
		class	SFormatVarArg : public SSystem::SStringFormatSupplier
		{
		protected:
			VirtualMachine *	m_vm ;
			const Register *	m_pVarArg ;
			size_t				m_iNextArg ;
		public:
			// 構築関数
			SFormatVarArg( VirtualMachine * vm, const Register * pVarArg ) ;
			// 次の整数取得
			virtual int64_t NextInteger( void ) ;
			// 次の浮動小数点取得
			virtual double NextDouble( void ) ;
			// 次の文字取得
			virtual wchar_t NextCharacter( void ) ;
			// 次の文字列取得
			virtual const wchar_t * NextString( SSystem::SString& strNext ) ;
		} ;
		// printf スタイル文字列
		void FormatStringVlist
			( SSystem::SString& strDst,
				const wchar_t * pwszFormat, const Register * pVarArg ) ;
		// 同期
		void Lock( void ) const
		{
			m_csMutex.Lock() ;
		}
		void Unlock( void ) const
		{
			m_csMutex.Unlock() ;
		}
		// アドレスからオブジェクトへ変換（同期）
		ECSSakura2::Object * AtomicObjectFromAddress( DWORD dwHighAddr )
			{
				ECSSakura2::Object *	pObj ;
				Lock() ;
				pObj = ObjectFromAddress( dwHighAddr ) ;
				Unlock() ;
				return	pObj ;
			}
		// アドレスからセグメント情報へ変換（同期）
		LinearAddressCache *
			AtomicSegmentFromAddress
				( LinearAddressCache * plac, DWORD dwHighAddr )
			{
				Lock() ;
				plac = SegmentFromAddress( plac, dwHighAddr ) ;
				Unlock() ;
				return	plac ;
			}
		// アドレス変換（同期）
		BYTE * TranslateAddress( INT64 nAddress, size_t nRange )
			{
				LinearAddressCache	seg ;
				if ( AtomicSegmentFromAddress
						( &seg, (DWORD)(nAddress >> 32) ) != NULL )
				{
					int nOffset = (DWORD) nAddress - seg.baseOffset ;
					if ( (nOffset >= 0) &
						 ((DWORD)(nOffset + nRange) <= seg.limitSegment) )
					{
						return  seg.pbytBuffer + nOffset ;
					}
				}
				return  NULL ;
			}

	} ;

} ;


#endif
