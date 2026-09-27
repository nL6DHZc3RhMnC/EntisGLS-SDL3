
#if	!defined(__SAKURA2_QUEUE_BUFFER_H__)
#define	__SAKURA2_QUEUE_BUFFER_H__

namespace	SSystem
{
	//////////////////////////////////////////////////////////////////////////
	// キューバッファ
	//////////////////////////////////////////////////////////////////////////

	class	SQueueBuffer	: public SFileInterface
	{
	protected:
		class	Fragment	: public SArray<uint8_t>
		{
		public:
			size_t	m_nUsed ;
			ssize_t	m_nStuffed ;
		public:
			Fragment( void ) : m_nUsed(0), m_nStuffed(0) {}
		} ;
		SObjectArray<Fragment>	m_queBuffer ;
		size_t					m_nLength ;
		size_t					m_nGetting ;
		size_t					m_nPutting ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SSystem::SQueueBuffer, SFileInterface )
		// 構築関数
		SQueueBuffer( void ) ;
		// 消滅関数
		virtual ~SQueueBuffer( void ) ;
		// バッファの全削除
		void ClearAll( void ) ;
		// ファイルから読み込み
		size_t ReadFromStream
			( SInputStream& file, ssize_t nBytes = -1 ) ;
		size_t ReadFromTextStream
			( SInputStream& file, ssize_t nBytes = -1 ) ;

	public:
		// 読み取りバッファ確保
		const uint8_t * GetBuffer( size_t& nBytes ) ;
		// 読み取りバッファ解放
		void ReleaseBuffer( ssize_t nBytes = -1 ) ;

	public:
		// 書き込みバッファ確保
		uint8_t * PutBuffer( size_t nBytes ) ;
		// 書き込みバッファ解放
		void FlushBuffer( size_t nBytes ) ;

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


}

#endif
