
#if	!defined(__SAKURA2_FRAGMENT_FILE_H__)
#define	__SAKURA2_FRAGMENT_FILE_H__

#include <sakura/ssys_smart_buffer.h>

namespace	SSystem
{
	//////////////////////////////////////////////////////////////////////////
	// 分割ファイル
	//////////////////////////////////////////////////////////////////////////

	class	SFragmentFile	: public SFileInterface
	{
	protected:
		// キャッシュ
		class	CacheEntry	: public SSmartBuffer
		{
		public:
			int64_t	m_fposCached ;
			size_t	m_iFragment ;
		public:
			// 構築関数
			CacheEntry( void ) : m_fposCached(0), m_iFragment(0) {}
			CacheEntry( const CacheEntry& cache )
				 : SSmartBuffer(cache),
					m_fposCached(cache.m_fposCached),
					m_iFragment(cache.m_iFragment) {}
			// ファイルインターフェースの複製
			virtual SFileInterface * Duplicate( void ) const
			{
				return	new CacheEntry( *this ) ;
			}
		} ;
		class	CacheObject	: public SObjectArray<CacheEntry>
		{
		public:
			SCriticalSection	m_csSync ;
			atomic_int_t		m_countRef ;
			size_t				m_limitCahce ;
		public:
			// 構築関数
			CacheObject( void ) ;
			// 消滅関数
			~CacheObject( void ) ;
			// 参照追加
			void AddRef( void ) ;
			// 参照解放
			void ReleaseRef( void ) ;
			// エントリ検索
			ssize_t FindCacheEntry( size_t iFragment ) const ;
			// エントリ追加
			void AddCacheEntry( CacheEntry * pCache ) ;
			// 排他処理
			void Lock( void ) ;
			void Unlock( void ) ;
		} ;
		CacheObject *	m_pCache ;

		// 現在ロードしているフラグメント
		SSmartBuffer	m_sbufLoaded ;
		int64_t			m_fposLoaded ;

		// フラグメント情報
		class	Fragment
		{
		public:
			enum	EncodingType
			{
				encodingNothing	= 0,
				encodingErina,
			} ;
			EncodingType	m_encoding ;	// エンコードタイプ
			int64_t			m_pos ;			// 開始位置
			int64_t			m_length ;		// サイズ
			SString			m_file ;		// フラグメントファイル名
		public:
			Fragment( void )
				: m_encoding( encodingNothing ), m_pos( 0 ), m_length( 0 ) {}
			Fragment( const Fragment& frg )
				: m_encoding( frg.m_encoding ), m_pos( frg.m_pos ),
					m_length( frg.m_length ), m_file( frg.m_file ) {}
		} ;
		SObjectArray<Fragment>	m_fragments ;
		int64_t					m_length ;
		SFileOpener *			m_opener ;
		bool					m_flagOwnOpener ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SFragmentFile, SFileInterface )
		// 構築関数
		SFragmentFile( void ) ;
		SFragmentFile( const SFragmentFile& ff ) ;
		// 消滅関数
		virtual ~SFragmentFile( void ) ;

	public:
		// ファイルを開く
		SError Open
			( SFileInterface& file,
				SFileOpener * opener, bool fOwnOpener = false ) ;
		// キャッシュの制限値（分割ファイル数）を設定する
		void SetCacheLimit( size_t nLimit ) ;

	protected:
		// フラグメントファイルをロードする
		SError LoadFragment( int64_t fpos ) ;

	public:	// SFileInterface オーバーライド
		// ファイルインターフェースの複製
		virtual SFileInterface * Duplicate( void ) const ;
		// ファイルから読み込み
		virtual size_t Read( void * ptrBuf, size_t nBytes ) ;
		// ファイルへ書き込み
		virtual size_t Write( const void * ptrBuf, size_t nBytes ) ;
		// シーク可能か否か？
		virtual bool IsSeekable( void ) const ;
		// ファイル長の取得
		virtual int64_t GetLength( void ) const ;
		// ファイルポインタを移動
		virtual int64_t Seek
			( int64_t posFile, SeekOrigin seekFrom = FromBegin ) ;
		// ファイルポインタを取得
		virtual int64_t GetPosition( void ) const ;
		// ファイルの終端を現在の位置に設定する
		virtual SError SetEndOfFile( void ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 分割ファイル・オープナー
	//////////////////////////////////////////////////////////////////////////

	class	SFragmentFileOpener	: public SOffsetFileOpener
	{
	protected:
		SOffsetFileOpener	m_ofo ;
		ssize_t				m_limitCache ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SFragmentFileOpener, SOffsetFileOpener )
		// 構築関数
		SFragmentFileOpener
			( const wchar_t * pszBasePath,
				wchar_t wchSeparator,
				SFileOpener * pOpener,
				bool flagOwner, ssize_t nCacheSize = -1 ) ;
		// 消滅関数
		virtual ~SFragmentFileOpener( void ) ;

	public:
		// ファイルを開く
		virtual SFileInterface * NewOpenFile
			( const wchar_t * pszFilePath, long int nOpenFlags ) ;
	} ;

}

#endif

