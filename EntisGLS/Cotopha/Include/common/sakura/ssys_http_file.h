
#if	!defined(__SAKURA2_HTTP_FILE_H__)
#define	__SAKURA2_HTTP_FILE_H__

#include <sakura/ssys_socket.h>

namespace	SSystem
{
	//////////////////////////////////////////////////////////////////////////
	// HTTP ファイル抽象クラス
	//////////////////////////////////////////////////////////////////////////

	class	SHttpFileInterface	: public SFileInterface
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SHttpFileInterface, SFileInterface )
			// URL 設定
			virtual SError SetRequest
			( const wchar_t * pwszURL, const wchar_t * pwszCmd = L"GET" ) = 0 ;
		// 送信データ設定
		virtual SError SetSendData
			( const uint8_t * pbytData, ssize_t nBytes = -1 ) = 0 ;
		// 送信ヘッダ設定
		virtual SError AddHeader( const wchar_t * pwszHeader ) = 0 ;
		// サーバへ接続
		enum	ConnectFlag
		{
			connectNoCache	= 0x0001,
		} ;
		virtual SError Connect( uint32_t nFlags = 0 ) = 0 ;
		// リクエスト送信
		virtual SError SendRequest( void ) = 0 ;
		// HTTP ステータスコード取得
		virtual SError QueryStatusCode( uint32_t& codeStatus ) const ;
		// HTTP データ長取得
		virtual SError QueryContentLength( uint64_t& numLength ) const ;
		// HTTP データタイプ取得
		virtual SError QueryContentType( SString& strType ) const ;
		// 文字エンコーディング取得 (Content-Type の charset)
		virtual SError QueryContentTypeCharset( SString& strCharset ) const ;
		// HTTP データエンコーディング取得
		virtual SError QueryContentTransferEncoding( SString& strEncoding ) const ;
		// HTTP Date 取得
		virtual SError QueryContentDate( DATE_TIME& dt ) const ;
		// HTTP Last-Modified 取得
		virtual SError QueryContentLastModified( DATE_TIME& dt ) const ;

	public:
		// URL を開く
		virtual SError OpenURL
			( const wchar_t * pwszURL,
				const wchar_t * pszAgent = NULL,
				const uint8_t * pbytData = NULL,
				ssize_t nLength = -1,
				const wchar_t * pwszContentType = NULL,
				uint32_t nFlags = connectNoCache ) ;
		// URL フォームパラメータを送信データに設定
		virtual SError SetSendURLFormData
			( const wchar_t * pwszURLFormed,
				Charset::EncodingType typeEncoding = Charset::encodingUTF8 ) ;
		// URL 解釈
		static SError ParseURL
			( const wchar_t * pwszURL,
			SString& strScheme, SString& strHost, SString& strPort,
			SString& strUser, SString& strPassword, SString& strPath ) ;
		// URL 文字列のエンコード
		static void FormatURL( SString& strURL, const wchar_t * pwszURL ) ;
		// URL 文字列のデコード
		static void UnformatURL( SString& strURL, const wchar_t * pwszURL ) ;
		// HTTP 日付の解釈
		static SError ParseDate( DATE_TIME& dt, const wchar_t * pwszDate ) ;
	protected:
		static int ParseDateWeek( const wchar_t * pwszWeek ) ;
		static int ParseDateWeek3( const wchar_t * pwszWeek ) ;
		static int ParseDateMonth( const wchar_t * pwszMonth ) ;

	public:	// SFileInterface オーバーライド
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

#if	defined(__COTOPHA__)
	class	native HttpFile	: public File
	{
	public:
		// URL 設定
		native SError SetRequest
			( const wchar_t * pwszURL, const wchar_t * pwszCmd = L"GET" ) ;
		// 送信データ設定
		native SError SetSendData
			( const uint8_t * pbytData, ssize_t nBytes = -1 ) ;
		// 送信ヘッダ設定
		native SError AddHeader( const wchar_t * pwszHeader ) ;
		// サーバへ接続
		native SError Connect( uint32_t nFlags = 0 ) ;
		// リクエスト送信
		native SError SendRequest( void ) ;
		// HTTP ステータスコード取得
		native SError QueryStatusCode( uint32_t& codeStatus ) const ;
		// HTTP データ長取得
		native SError QueryContentLength( uint64_t& numLength ) const ;
		// HTTP データタイプ取得
		native SError QueryContentType( SString& strType ) const ;
		// HTTP データエンコーディング取得
		native SError QueryContentTransferEncoding( SString& strEncoding ) const ;
		// HTTP Date 取得
		native SError QueryContentDate( DATE_TIME& dt ) const ;
		// HTTP Last-Modified 取得
		native SError QueryContentLastModified( DATE_TIME& dt ) const ;
	} ;
#else
	typedef	SHttpFileInterface	HttpFile ;
#endif


	//////////////////////////////////////////////////////////////////////////
	// HTTP ファイル
	//////////////////////////////////////////////////////////////////////////

	class	SHttpFile	: public SHttpFileInterface
	{
	protected:
		HttpFile *	m_pHttpFile ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SHttpFile, SHttpFileInterface )
			// 構築関数
			SHttpFile( void ) ;
		SHttpFile( HttpFile * pHttpFile ) ;
		// 消滅関数
		virtual ~SHttpFile( void ) ;

	public:
		// URL 設定
		virtual SError SetRequest
			( const wchar_t * pwszURL, const wchar_t * pwszCmd = L"GET" ) ;
		// 送信データ設定
		virtual SError SetSendData
			( const uint8_t * pbytData, ssize_t nBytes = -1 ) ;
		// 送信ヘッダ設定
		virtual SError AddHeader( const wchar_t * pwszHeader ) ;
		// サーバへ接続
		virtual SError Connect( uint32_t nFlags = 0 ) ;
		// リクエスト送信
		virtual SError SendRequest( void ) ;
		// HTTP ステータスコード取得
		virtual SError QueryStatusCode( uint32_t& codeStatus ) const ;
		// HTTP データ長取得
		virtual SError QueryContentLength( uint64_t& numLength ) const ;
		// HTTP データタイプ取得
		virtual SError QueryContentType( SString& strType ) const ;
		// HTTP データエンコーディング取得
		virtual SError QueryContentTransferEncoding( SString& strEncoding ) const ;
		// HTTP Date 取得
		virtual SError QueryContentDate( DATE_TIME& dt ) const ;
		// HTTP Last-Modified 取得
		virtual SError QueryContentLastModified( DATE_TIME& dt ) const ;

	public:
		// File 変換
#if	defined(__COTOPHA__)
		virtual File* GetFileObject( void ) ;
#else
		virtual SFileInterface * GetFileObject( void ) ;
#endif

	public:	// SFileInterface オーバーライド
		// ファイルインターフェースの複製
		virtual SFileInterface * Duplicate( void ) const ;
		// ファイルから読み込み
		virtual size_t Read( void * ptrBuf, size_t nBytes ) ;
		// ファイルへ書き込み
		virtual size_t Write( const void * ptrBuf, size_t nBytes ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// HTTP1.1 クライアント簡易実装
	//////////////////////////////////////////////////////////////////////////

	class	SHttpSimpleClient	: public SHttpFileInterface
	{
	protected:
		SSyncSocket						m_socket ;
		SString							m_strCmd ;
		SString							m_strScheme ;
		SString							m_strHost ;
		SString							m_strPort ;
		SString							m_strUser ;
		SString							m_strPassword ;
		SString							m_strPath ;
		SArray<uint8_t>					m_bufSendData ;
		uint32_t						m_codeStatus ;
		SStrSortObjectArray<SString>	m_sendHeader ;
		SStrSortObjectArray<SString>	m_recvHeader ;
		bool							m_flagChunked ;
		SQueueBuffer					m_qbufRecv ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SHttpSimpleClient, SHttpFileInterface )
			// 構築関数
			SHttpSimpleClient( void ) ;
		// 消滅関数
		virtual ~SHttpSimpleClient( void ) ;

	public:
		// URL 設定
		virtual SError SetRequest
			( const wchar_t * pwszURL, const wchar_t * pwszCmd = L"GET" ) ;
		// 送信データ設定
		virtual SError SetSendData
			( const uint8_t * pbytData, ssize_t nBytes = -1 ) ;
		// 送信ヘッダ設定
		virtual SError AddHeader( const wchar_t * pwszHeader ) ;
		// サーバへ接続
		virtual SError Connect( uint32_t nFlags = 0 ) ;
		// リクエスト送信
		virtual SError SendRequest( void ) ;
		// HTTP ステータスコード取得
		virtual SError QueryStatusCode( uint32_t& codeStatus ) const ;
		// HTTP データ長取得
		virtual SError QueryContentLength( uint64_t& numLength ) const ;
		// HTTP データタイプ取得
		virtual SError QueryContentType( SString& strType ) const ;
		// HTTP データエンコーディング取得
		virtual SError QueryContentTransferEncoding( SString& strEncoding ) const ;
		// HTTP Date 取得
		virtual SError QueryContentDate( DATE_TIME& dt ) const ;
		// HTTP Last-Modified 取得
		virtual SError QueryContentLastModified( DATE_TIME& dt ) const ;
		// 受信ヘッダ取得
		virtual SError QueryHeader
			( SString& strValue, const wchar_t * pwszName ) ;

	public:
		// File 変換
#if	defined(__COTOPHA__)
		virtual File* GetFileObject( void ) ;
#else
		virtual SFileInterface * GetFileObject( void ) ;
#endif

	public:	// SFileInterface オーバーライド
		// ファイルインターフェースの複製
		virtual SFileInterface * Duplicate( void ) const ;
		// ファイルから読み込み
		virtual size_t Read( void * ptrBuf, size_t nBytes ) ;
		// ファイルへ書き込み
		virtual size_t Write( const void * ptrBuf, size_t nBytes ) ;

	protected:
		// chunked 転送に対応するために次の１チャンクを読み込む
		void ReceiveNextChunk( void ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// HTTP・オープン・インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SHttpFileOpener	: public SFileOpener
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SHttpFileOpener, SFileOpener )

	public:
		// ファイルを開く
		virtual SFileInterface * NewOpenFile
			( const wchar_t * pszFilePath, long int nOpenFlags ) ;
		// ファイルの存在
		virtual bool IsExisting( const wchar_t * pszFilePath ) ;
		// ファイル状態
		virtual SError QueryState
			( const wchar_t * pszFilePath, State& state ) ;
		// ファイルの一覧取得
		virtual void ListSubFiles
			( SObjectArray<SString>& listFiles,
			const wchar_t * pszDirPath = NULL ) ;
		// ディレクトリの一覧取得
		virtual void ListSubDirectories
			( SObjectArray<SString>& listDirs,
			const wchar_t * pszDirPath = NULL ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// HTTP 簡易サーバー
	//////////////////////////////////////////////////////////////////////////

	class	SHttpSimpleServer	: public ESLObject, public SProcedure
	{
	protected:
		SSocket			m_socketListen ;
		SThread			m_threadListen ;
		SSignalEvent	m_sigExitListen ;
		bool			m_fStartup ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2( SHttpSimpleServer, ESLObject, SProcedure )
			// 構築関数
			SHttpSimpleServer( void ) ;
		// 消滅関数
		virtual ~SHttpSimpleServer( void ) ;

	public:
		// サービス開始
		SError Startup( uint32_t nPort ) ;
		// サービス終了
		SError Shutdown( void ) ;

	public:
		// スレッド関数
		virtual void Run( void ) ;

	public:
		// HTTP ヘッダ
		class	HeaderList	: public SObjectArray<SString>
		{
		public:
			// ヘッダ追加
			void AddHeader( const wchar_t * pwszHeader ) ;
			// ヘッダ取得
			const wchar_t * GetHeader( const wchar_t * pwszFieldName ) const ;
			// ヘッダ設定
			void SetHeader( const wchar_t * pwszFieldName, const wchar_t * pwszValue ) ;
		} ;
		// HTTP 応答
		class	Response
		{
		public:
			int				m_nStatus ;
			HeaderList		m_lstHeaders ;
			SArray<uint8_t>	m_bufContent ;
		public:
			// 構築関数
			Response( int nStatus = 200 ) : m_nStatus( nStatus ) { }
			// ヘッダ追加
			void AddHeader( const wchar_t * pwszHeader )
			{
				m_lstHeaders.AddHeader( pwszHeader ) ;
			}
			// ヘッダ取得
			const wchar_t * GetHeader( const wchar_t * pwszFieldName ) const
			{
				return	m_lstHeaders.GetHeader( pwszFieldName ) ;
			}
			// ヘッダ設定
			void SetHeader( const wchar_t * pwszFieldName, const wchar_t * pwszValue )
			{
				return	m_lstHeaders.SetHeader( pwszFieldName, pwszValue ) ;
			}
			void AddContentType( const wchar_t * pwszType )
			{
				return	m_lstHeaders.SetHeader( L"Content-Type", pwszType ) ;
			}
			void AddAccessControlAllowOrigin( const wchar_t * pwszPath = L"*" )
			{
				return	m_lstHeaders.SetHeader( L"Access-Control-Allow-Origin", pwszPath ) ;
			}
			// コンテント設定
			void SetContent( const void * ptrData, size_t nBytes )
			{
				m_bufContent.SetLength( nBytes ) ;
				eslMoveMemory( m_bufContent.GetArray(), ptrData, nBytes ) ;
				m_bufContent.FinishArray() ;
			}
			// コンテント長取得
			size_t GetContentLength( void ) const
			{
				return	m_bufContent.GetLength() ;
			}
			// コンテント取得
			const uint8_t * GetContentData( void ) const
			{
				return	m_bufContent.GetConstArray() ;
			}
		} ;

	protected:
		// 接続受付
		virtual void OnAccept( SSyncSocket * socket ) ;
		// 応答
		virtual void OnResponse
			( SSyncSocket * socket,
			const SString& strCmd,
			const SString& strPath,
			const HeaderList& lstHeaders ) ;
		virtual Response * CreateResponse
			( SSyncSocket * socket,
			const SString& strCmd,
			const SString& strPath,
			const HeaderList& lstHeaders ) ;
		// 1行受信
		static SString ReadLine( SSyncSocket& socket ) ;
		// HTTP応答送信
		static void SendHTTPResponse
			( SSyncSocket& socket, Response& res ) ;

	} ;

}

#if	defined(__PLATFORM_WINDOWS__)
#include <sakura/ssys_win_internet_file.h>
#endif

#endif
