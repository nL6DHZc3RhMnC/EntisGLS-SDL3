
#if	!defined(__SAKURA2_BUFFERED_FILE_H__)
#define	__SAKURA2_BUFFERED_FILE_H__


namespace	SSystem
{

	//////////////////////////////////////////////////////////////////////////
	// 一時バッファ付きファイル
	//////////////////////////////////////////////////////////////////////////

	class	SBufferedFile	: public SSmartFile
	{
	protected:
		enum	BufferMode
		{
			cacheNothing,
			cacheRead,
			cacheWriteBack,
		} ;
		BufferMode	m_modeCache ;
		size_t		m_nBuffered ;
		size_t		m_nOffset ;
		uint8_t		m_bufCache[0x100] ;

		Charset::EncodingType	m_encodingChar ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SBufferedFile, SSmartFile )
		// 構築関数
		SBufferedFile( void ) ;
		SBufferedFile
			( SFileOpener * pOpener,
				SFileInterface * pFile, bool flagOwner = true ) ;
		// 消滅関数
		virtual ~SBufferedFile( void ) ;
		// ファイルを開く
		SError Open
			( const wchar_t * pszFilePath,
				long int nOpenFlags, bool flagAppend = false ) ;
		// ファイルの参照を解除する
		virtual void Close( void ) ;
		// キャッシュをフラッシュする
		virtual void FlushBuffer( void ) ;

	public:
		// ファイルから読み込み
		virtual size_t Read( void * ptrBuf, size_t nBytes ) ;
		// ファイルへ書き込み
		virtual size_t Write( const void * ptrBuf, size_t nBytes ) ;
		// ファイル長の取得
		virtual int64_t GetLength( void ) const ;
		// ファイルポインタを移動
		virtual int64_t Seek
			( int64_t posFile, SeekOrigin seekFrom = FromBegin ) ;
		// ファイルポインタを取得
		virtual int64_t GetPosition( void ) const ;
		// ファイルの終端を現在の位置に設定する
		virtual SError SetEndOfFile( void ) ;

	public:
		// 文字エンコーディング取得
		Charset::EncodingType GetCharsetEncoding( void ) const
		{
			return	m_encodingChar ;
		}
		// 文字エンコーディング設定
		void SetCharsetEncoding( Charset::EncodingType encoding )
		{
			m_encodingChar = encoding ;
		}
		// 文字列書き出し
		size_t WriteString( const SString & strBuf ) ;
		size_t WriteString( const wchar_t * pszStr, ssize_t nLength = -1 ) ;
		size_t WriteFormat( const wchar_t * pszFormat, ... ) ;
		size_t WriteFormatV( const wchar_t * pszFormat, va_list argptr ) ;
		// 文字列１行読み込み
		size_t ReadStringLine( SString & strBuf ) ;

	public:
		// 文字列書き出し
		SBufferedFile& operator += ( const SString & strBuf ) ;
		SBufferedFile& operator += ( const wchar_t * pszStr ) ;
		SBufferedFile& operator << ( const SString & strBuf ) ;
		SBufferedFile& operator << ( const wchar_t * pszStr ) ;
		SBufferedFile& operator << ( int64_t nValue ) ;
		SBufferedFile& operator << ( double nValue ) ;
		// 文字列１行読み込み
		SBufferedFile& operator >> ( SString & strBuf ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 同期バッファ付きファイル
	//////////////////////////////////////////////////////////////////////////

	class	SSyncBufferedFile	: public SBufferedFile
	{
	protected:
		SSystem::SCriticalSection	m_csSync ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SSyncBufferedFile, SBufferedFile )
		// 構築関数
		SSyncBufferedFile( void ) ;
		SSyncBufferedFile
			( SFileOpener * pOpener,
				SFileInterface * pFile, bool flagOwner = true ) ;
		// キャッシュをフラッシュする
		virtual void FlushBuffer( void ) ;

	public:
		// ファイルから読み込み
		virtual size_t Read( void * ptrBuf, size_t nBytes ) ;
		// ファイルへ書き込み
		virtual size_t Write( const void * ptrBuf, size_t nBytes ) ;
	} ;

}

#endif
