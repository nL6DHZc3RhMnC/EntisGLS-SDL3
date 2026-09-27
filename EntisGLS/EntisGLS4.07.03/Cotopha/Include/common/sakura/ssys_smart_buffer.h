
#if	!defined(__SAKURA2_SMART_BUFFER_H__)
#define	__SAKURA2_SMART_BUFFER_H__

namespace	SSystem
{
	//////////////////////////////////////////////////////////////////////////
	// 断片化バッファ・ファイルインターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SSmartBuffer	: public SFileInterface
	{
	public:
		#if	!defined(__COTOPHA__)
		enum	ConstantValue
		{
			PAGE_BYTES			= 0x4000,
			PAGE_OFFSET_MASK	= PAGE_BYTES - 1,
			PAGE_SCALE			= 14,
		} ;
		#else
		constant	PAGE_BYTES			= 0x4000 ;
		constant	PAGE_OFFSET_MASK	= PAGE_BYTES - 1 ;
		constant	PAGE_SCALE			= 14 ;
		#endif

	protected:
		// SSyncReference -> SSmartObject -> SByteBuffer
		SObjectArray<SSyncReference>	m_buffers ;
		size_t							m_position ;
		size_t							m_length ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SSystem::SSmartBuffer, SFileInterface )
		// 構築関数
		SSmartBuffer( void ) ;
		SSmartBuffer( const SSmartBuffer& sbufSrc ) ;
		// 消滅関数
		virtual ~SSmartBuffer( void ) ;

	public:
		// バッファ長設定
		void SetLength( size_t nLength ) ;
		// バッファ解放
		void ReleaseBuffer( void ) ;
		// バッファ読み取り
		size_t ReadBuffer
			( size_t nPos, void * ptrBuf, size_t nBytes ) const ;
		// バッファ書き込み
		size_t WriteBuffer
			( size_t nPos, const void * ptrBuf, size_t nBytes ) ;
		// ファイルから読み込み
		size_t ReadFromStream
			( SInputStream& file, ssize_t nBytes = -1 ) ;
		// ファイルへ書き出し
		size_t WriteToStream
			( SOutputStream& file, ssize_t nBytes = -1 ) const ;
		// バッファの複製参照
		void CopyReferenceBuffer( const SSmartBuffer& sbufSrc ) ;

	protected:
		// ページ取得
		SByteBuffer * GetPageAt( size_t iPage ) const ;
		// ロード済みページ取得
		SByteBuffer * GetLoadedPageAt( size_t iPage ) ;

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
