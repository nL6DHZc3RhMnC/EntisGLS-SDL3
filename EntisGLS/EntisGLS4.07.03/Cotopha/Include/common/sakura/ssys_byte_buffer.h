
#if	!defined(__SAKURA2_BYTE_BUFFER_H__)
#define	__SAKURA2_BYTE_BUFFER_H__

namespace	SSystem
{
	//////////////////////////////////////////////////////////////////////////
	// ただの BYTE バッファ
	//////////////////////////////////////////////////////////////////////////

	class	SByteBuffer	: public SFileInterface, public SArray<uint8_t>
	{
	protected:
		uint32_t	m_posBuf ;

		#if	defined(__COTOPHA__)
		MemoryReferenceFile *	m_pMemFile ;
		#endif

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SSystem::SByteBuffer, SFileInterface )
		ESL_DECLARE_CLASS_OPERATOR_NEW( SFileInterface )
		// 構築関数
		SByteBuffer( void ) : m_posBuf( 0 ) {}
		SByteBuffer( const SByteBuffer& buf )
			: SArray<uint8_t>( buf ), m_posBuf( buf.m_posBuf ) {}
		// 消滅関数
		virtual ~SByteBuffer( void ) ;
		// File 変換
		#if	defined(__COTOPHA__)
		virtual File* GetFileObject( void ) ;
		#endif

	public:
		// 配列複製
		const SByteBuffer & operator = ( const SByteBuffer & buf )
		{
			SArray<uint8_t>::operator = ( buf ) ;
			m_posBuf = buf.m_posBuf ;
			return	*this ;
		}
		// バッファ読み取り
		size_t ReadBuffer
			( size_t nPos, void * ptrBuf, size_t nBytes ) const ;
		// バッファ書き込み
		size_t WriteBuffer
			( size_t nPos, const void * ptrBuf, size_t nBytes ) ;
		// 入力ストリームから読み込み
		size_t ReadFromStream( SInputStream& file, ssize_t nBytes = -1 ) ;
		size_t ReadFromTextStream( SInputStream& file, ssize_t nBytes = -1 ) ;
		// ファイルから読み込み
		size_t ReadFromFile( SFileInterface& file, ssize_t nBytes = -1 ) ;

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
