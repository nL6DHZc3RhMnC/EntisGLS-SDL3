
#if	!defined(__SAKURA2_HEAP_MEMORY_H__)
#define	__SAKURA2_HEAP_MEMORY_H__

namespace	SSystem
{
	//////////////////////////////////////////////////////////////////////////
	// ヒープメモリ領域管理
	//////////////////////////////////////////////////////////////////////////

	class	SFileInterface ;
	class	SHeapManager
	{
	public:
		// 構築関数
		SHeapManager( uint8_t * pbytBuf, size_t nBufBytes ) ;
		SHeapManager( void ) ;

	public:
		// メモリブロック・フラグ
		enum	BlockFlags
		{
			blockSizeMask	= 0x0FFFFFFF,
			blockFlagMask	= 0xF0000000,
			blockFreeFlag	= 0x80000000,
			blockLastFlag	= 0x40000000,
			blockFirstFlag	= 0x20000000,
			blockSignature	= 0x10000000,	// must be set for verify
			blockSizeScale	= 3,
			blockSizeOddMask = (1 << blockSizeScale) - 1,
			blockMaxBytes	= (blockSizeMask + 1) << blockSizeScale,
		} ;
		// メモリブロック・ヘッダ
		struct	BLOCK_HEADER
		{
			uint32_t	dwSizeFlags ;	// 有効なサイズと enum BlockFlags
			uint32_t	dwPrevSize ;		// 直前 BLOCK_HEADER のサイズ

			inline uint32_t GetBlockSize( void ) const
			{
				return	(dwSizeFlags & blockSizeMask) << blockSizeScale ;
			}
			inline bool IsValidBlock( void ) const
			{
				return	(dwSizeFlags & blockSignature) != 0 ;
			}
			inline bool IsFirstBlock( void ) const
			{
				return	(dwSizeFlags & blockFirstFlag) != 0 ;
			}
			inline bool IsLastBlock( void ) const
			{
				return	(dwSizeFlags & blockLastFlag) != 0 ;
			}
			inline bool IsFreeBlock( void ) const
			{
				return	(dwSizeFlags & blockFreeFlag) != 0 ;
			}
			inline uint8_t * GetBlockBody( void ) const
			{
				ESLAssert( IsValidBlock() ) ;
				return	(uint8_t*) (this + 1) ;
			}
			inline BLOCK_HEADER * GetNextBlock( void ) const
			{
				ESLAssert( !IsLastBlock() ) ;
				return	(BLOCK_HEADER*) (GetBlockBody() + GetBlockSize()) ;
			}
			inline BLOCK_HEADER * GetPrevBlock( void ) const
			{
				ESLAssert( !IsFirstBlock() ) ;
				return	(BLOCK_HEADER*)
					(((uint8_t*)(this - 1)) - (dwPrevSize << blockSizeScale)) ;
			}
		} ;
		// 未使用ブロックはブロック・ボディに FREE_BLOCK を保持
		struct	FREE_BLOCK
		{
			int32_t	dwPrevOffset ;
			int32_t	dwNextOffset ;

			inline FREE_BLOCK * GetPrevFreeBlock( void ) const
			{
				return	(FREE_BLOCK*) (((uint8_t*)this) + dwPrevOffset) ;
			}
			inline FREE_BLOCK * GetNextFreeBlock( void ) const
			{
				return	(FREE_BLOCK*) (((uint8_t*)this) + dwNextOffset) ;
			}
		} ;
		enum	BlockSizeValue
		{
			sizeMinBlock = sizeof(FREE_BLOCK),
			sizeMinDouble = sizeof(BLOCK_HEADER) * 2 + sizeof(FREE_BLOCK) * 2,
		} ;

	protected:
		// ヒープを割り当てるメモリ領域
		uint8_t *		m_pbytBuf ;
		size_t			m_nBufBytes ;

		// 未使用チェインの先頭
		FREE_BLOCK *	m_pFreeFirst ;

	public:
		// ヒープ領域割り当て初期化
		void Initialize( uint8_t * pbytBuf, size_t nBufBytes ) ;
		// メモリブロック確保
		int32_t Allocate( uint32_t nBytes ) ;
		// メモリブロック再確保
		int32_t Reallocate( uint32_t nAddr, uint32_t nBytes ) ;
		// メモリブロック解放
		void Free( uint32_t nAddr ) ;
		// メモリブロックのサイズ取得
		uint32_t GetBlockLength( uint32_t nAddr ) const ;
		// ヒープメモリが空か判定
		bool IsEmpty( void ) const ;
		// メモリ領域アドレス取得
		uint8_t * GetMemoryAddress( uint32_t nAddr ) const ;
		int32_t GetMemoryOffset( uint8_t * pbytBlock ) const ;
		// メモリ領域全体の長さ
		size_t GetHeapLength( void ) const ;
		// 確保メモリブロックのダンプ
		size_t DumpMemoryBlocks( size_t nDumpLimit ) const ;

	protected:
		// 先頭のブロックを取得する
		BLOCK_HEADER * GetFirstBlock( void ) const
		{
			return	(BLOCK_HEADER*) m_pbytBuf ;
		}
		// 未使用ブロックに使用ブロックを割り当て
		BLOCK_HEADER * AllocateBlock( BLOCK_HEADER * pBlock, uint32_t nBytes ) ;
		// 2つの連続する未使用ブロックを結合
		bool MergeFreeBlock( BLOCK_HEADER * pBlock ) ;
		// 次のブロックとの結合を正規化する
		void NormalizeNextBlock( BLOCK_HEADER * pBlock ) ;
		// 未使用ブロックをチェインの先頭に追加
		void AttachFreeBlockChain( FREE_BLOCK * pFreeBlock ) ;
		// 未使用ブロックをチェインから分離
		void DetachFreeBlockChain( FREE_BLOCK * pFreeBlock ) ;

	public:
		// 状態保存処理
		SError SaveContext( SFileInterface& file ) ;
		// 状態復元処理
		SError LoadContext( SFileInterface& file ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// ヒープメモリ統合管理
	//////////////////////////////////////////////////////////////////////////

	class	SHeapMemory	: public ESLObject
	{
	protected:
		// ヒープエントリ
		class	SubHeapEntry	: public SHeapManager
		{
		public:
			SubHeapEntry *	m_pPrev ;
			SubHeapEntry *	m_pNext ;
			bool			m_flagLargeHeap ;
			size_t			m_sizeMemPage ;

		public:
			// リストから分離
			void Detach( void )
			{
				SubHeapEntry *	pPrev = m_pPrev ;
				SubHeapEntry *	pNext = m_pNext ;
				if ( pPrev != NULL )
				{
					pPrev->m_pNext = pNext ;
				}
				if ( pNext != NULL )
				{
					pNext->m_pPrev = pPrev ;
				}
				m_pPrev = NULL ;
				m_pNext = NULL ;
			}
		} ;
		// 定数
		enum	HeapSizeValue
		{
			scaleMinPage	= 16,
			sizeMinPage		= (1 << scaleMinPage),
			sizePageHeader	= ((sizeof(SubHeapEntry) + 0x0F) & ~0x0F),
		} ;
		// 逆引きテーブル
		enum	PageIndexedNumbers
		{
			numberIndexBits		= 16,
			numberIndexCount	= 1 << numberIndexBits,
			numberIndexMask		= numberIndexCount - 1,
		} ;
		struct	PageIndexedTable
		{
			SubHeapEntry *	pHeap[numberIndexCount] ;
		} ;

		SCriticalSection	m_csSync ;
		size_t				m_sizePageUnit ;
		size_t				m_sizeHeapPage ;
		size_t				m_nCommitCharge ;
		size_t				m_maxCommitCharge ;
		SubHeapEntry *		m_pFirstHeap ;
		SubHeapEntry *		m_pLargeHeap ;
		SubHeapEntry *		m_pCacheHeap ;

		PageIndexedTable *		m_pIndexedPage ;
		size_t					m_nPageTableBytes ;
		ulong_ptr_t*			m_pHeapEntries ;
		size_t					m_nHeapCount ;
		size_t					m_nHeapCountLimit ;
		size_t					m_nHeapBufBytes ;

	public:
		// 確保時動作フラグ
		enum	AllocationFlag
		{
			flagNoRetryAlloc	= 0x0001,	// メモリ不足時にリトライせずに nullptr を返す
		} ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SHeapMemory, ESLObject )
		// 構築関数
		SHeapMemory( void ) ;
		// 消滅関数
		virtual ~SHeapMemory( void ) ;

	public:
		// メモリブロック確保
		uint8_t * Allocate( uint32_t nBytes, uint32_t nFlags = 0 ) ;
		// メモリブロック再確保
		uint8_t * Reallocate
			( uint8_t * pMemBlock, uint32_t nBytes, uint32_t nFlags ) ;
		// メモリブロック解放
		void Free( uint8_t * pMemBlock ) ;
		// メモリブロックのサイズ取得
		uint32_t GetBlockLength( uint8_t * pMemBlock ) ;
		// ページ割り当てメモリ総量取得
		size_t GetCommitCharge( void ) const ;
		// ページ割り当て履歴最大メモリ総量取得
		size_t GetMaxCommitCharge( void ) const ;
		// 確保メモリブロックのダンプ
		size_t DumpMemoryBlocks( size_t nDumpLimit ) const ;

	protected:
		// メモリブロックを含むヒープエントリ
		SubHeapEntry * GetHeapEntryOf( uint8_t * pMemBlock ) ;
		// ヒープエントリを逆引きテーブルに登録
		void RegisterHeapIndex( SubHeapEntry * pHeap ) ;
		// ヒープエントリを逆引きテーブルから削除
		void UnregisterHeapIndex( SubHeapEntry * pHeap ) ;
		// ヒープエントリ配列挿入指標検索
		size_t OrderIndexOfHeapAddress( ulong_ptr_t addrHeap ) const ;

	public:
		// ページサイズ取得
		static size_t GetMemoryPageSize( void ) ;
		// ページメモリ確保
		uint8_t * AllocatePage( size_t& nSize ) ;
		// ページメモリ解放
		static void FreePage( uint8_t * pbytPage, size_t nSize ) ;

	public:
		// アロケーション
		void * operator new ( size_t nBytes ) ;
		void operator delete( void * ptrMem ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 標準ヒープ
	//////////////////////////////////////////////////////////////////////////

	extern	ESL_DLL_EXPORT SHeapMemory *	g_pStdHeap ;

	// ヒープの準備
	void eslHeapInitialize( void ) ;
	// ヒープの終了
	void eslHeapUninitialize( void ) ;
	// メモリ確保
	void * eslHeapAllocate( size_t nBytes ) ;
	void * eslHeapAllocate( size_t nBytes, uint32_t nFlags ) ;
	// メモリ再確保
	void * eslHeapReallocate( void * pMemBlock, size_t nBytes ) ;
	// メモリ解放
	void eslHeapFree( void * pMemBlock ) ;
	// メモリサイズ取得
	size_t eslHeapSizeOf( void * pMemBlock ) ;
	// コミットチャージ取得
	size_t eslHeapCommitCharge( void ) ;
	// 最大コミットチャージ取得
	size_t eslHeapMaxCommitCharge( void ) ;

}


#endif

