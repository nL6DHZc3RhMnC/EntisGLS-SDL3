
#if	!defined(__SAKURA2_SOCKET_H__)
#define	__SAKURA2_SOCKET_H__

#include <sakura/ssys_queue_buffer.h>

namespace	SSystem
{
	//////////////////////////////////////////////////////////////////////////
	// ソケット
	//////////////////////////////////////////////////////////////////////////

	#if	defined(__COTOPHA__)
	class	native Socket : public File
	{
	public:
		// ソケット作成フラグ
		enum	SocketFlag
		{
			// Types
			typeStream		= 0x0000,		// default
			typeDatagram	= 0x0001,		// datagram
			typeMask		= 0x00FF,
		} ;
		// ソケット作成
		native SError Create
			( uint32_t nPort = 0, int64_t nFlags = 0,
					const wchar_t * pwszAddress = NULL ) ;
		// ソケットを閉じる
		native void Close( void ) ;
		// ソケット接続
		native SError Connect
			( const wchar_t * pwszHostAddress, uint32_t nHostPort ) ;
		// 接続要求を待つ
		native SError Listen( int nConnectionBacklog = 5 ) ;
		// 接続受け入れ
		native SError Accept( Socket * socket ) ;
		// 状態フラグ
		enum	PollState
		{
			pollIn		= 0x0001,
			pollOut		= 0x0002,
			pollHangup	= 0x0004,
			pollError	= 0x0010,
		} ;
		// 状態ポーリング
		native int64_t Poll( int64_t nFlags = 0, int64_t msecTimeout = 0 ) ;
		// 受信 (Datagram)
		native size_t ReceiveFrom
			( void * ptrBuf, size_t nBytes,
				void * ptrAddrFrom, uint32_t& nAddrBytes ) ;
		// 送信 (Datagram)
		native size_t SendTo
			( const void * ptrBuf, size_t nBytes,
				void * ptrAddrTo, size_t nAddrBytes ) ;
		// Accept で接続したクライアントアドレスを取得する
		native SError GetAcceptedCleintIP( SArray<uint16_t>& strAddress ) const ;
		// このマシンのIPアドレス(localhostでない)を取得する
		static native SError GetLocalMachineIP( SArray<uint16_t> & strAddrIP ) ;
	} ;
	#endif

	class	SSocket	: public SFileInterface
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SSocket, SFileInterface )
		// 構築関数
		SSocket( void ) ;
		// 消滅関数
		virtual ~SSocket( void ) ;

	protected:
		#if	defined(__COTOPHA__)
			Socket *	m_socket ;
		#elif	defined(__PLATFORM_WINDOWS__)
			SOCKET		m_socket ;
			typedef		uint32_t	in_addr_t ;
			typedef		int			socklen_t ;
		#else
			enum
			{
				INVALID_SOCKET	= -1,
				SOCKET_ERROR	= -1,
			} ;
			typedef	int	SOCKET ;
			SOCKET		m_socket ;
		#endif
			SString	m_strAcceptClient ;

	public:
		// ソケット作成フラグ
		enum	SocketFlag
		{
			// Types
			typeStream		= 0x0000,		// default  TCP/IP
			typeDatagram	= 0x0001,		// datagram UDP/IP
			typeMask		= 0x00FF,
		} ;
		// ソケット作成
		virtual SError Create
			( uint32_t nPort = 0, int64_t nFlags = 0,
					const wchar_t * pwszAddress = NULL ) ;
		// ソケットを閉じる
		virtual void Close( void ) ;
		// ソケット接続
		virtual SError Connect
			( const wchar_t * pwszHostAddress, uint32_t nHostPort ) ;
		// 接続要求を待つ
		virtual SError Listen( int nConnectionBacklog = 5 ) ;
		// 接続受け入れ
		virtual SError Accept( SSocket & socket ) ;
		// 状態フラグ
		enum	PollState
		{
			pollIn		= 0x0001,
			pollOut		= 0x0002,
			pollHangup	= 0x0004,
			pollError	= 0x0010,
		} ;
		// 状態ポーリング
		virtual int64_t Poll
			( int64_t nFlags = 0, int64_t msecTimeout = 0 ) ;
		// 受信
		virtual size_t Receive( void * ptrBuf, size_t nBytes ) ;
		virtual size_t ReceiveFrom
			( void * ptrBuf, size_t nBytes,
				void * ptrAddrFrom, size_t& nAddrBytes ) ;
		// 送信
		virtual size_t Send( const void * ptrBuf, size_t nBytes ) ;
		virtual size_t SendTo
			( const void * ptrBuf, size_t nBytes,
				void * ptrAddrTo, size_t nAddrBytes ) ;
		// Accept で接続したクライアントアドレスを取得する
		virtual SError GetAcceptedCleintIP( SString& strAddress ) const ;
		// このマシンのIPアドレス(localhostでない)を取得する
		static SError GetLocalMachineIP( SString& strAddress ) ;

	protected:
		// Accept で接続された socket に対して呼び出される
		virtual void OnAccepted( void ) ;

	public:
		// File 変換
		#if	defined(__COTOPHA__)
		virtual File* GetFileObject( void ) ;
		#endif

	public:	// SFileInterface 実装
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

	#if	!defined(__COTOPHA__)
	typedef	SSocket	Socket ;
	#endif


	//////////////////////////////////////////////////////////////////////////
	// 同期ソケット
	//////////////////////////////////////////////////////////////////////////

	class	SSyncSocket	: public SSocket
	{
	protected:
		SCriticalSection	m_csSync ;
		SQueueBuffer		m_qbufRecv ;
		bool				m_flagClosed ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SSyncSocket, SSocket )
		// 構築関数
		SSyncSocket( void ) ;
		// 消滅関数
		virtual ~SSyncSocket( void ) ;

	public:	// SSocket 実装
		// 状態ポーリング
		virtual int64_t Poll
			( int64_t nFlags = 0, int64_t msecTimeout = 0 ) ;
		// 受信
		virtual size_t Receive( void * ptrBuf, size_t nBytes ) ;

	public:	// SFileInterface 実装
		// ファイルインターフェースの複製
		virtual SFileInterface * Duplicate( void ) const ;
		// ファイルから読み込み
		virtual size_t Read( void * ptrBuf, size_t nBytes ) ;
		// 改行コード (\n) まで読み込み
		virtual size_t ReadLine( uint8_t * ptrBuf, size_t nBytes ) ;
		virtual size_t ReadLine( SArray<uint8_t>& buf ) ;
		// ファイルへ書き込み
		virtual size_t Write( const void * ptrBuf, size_t nBytes ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 非同期ソケット
	//////////////////////////////////////////////////////////////////////////

	class	SAsyncSocket ;
	class	SAsyncSocketListener
	{
	public:
		// クラス情報
		ESL_DECLARE_NV_CLASS_INFO( SAsyncSocketListener )
		// データを受信可能
		virtual void OnReceive( SAsyncSocket& socket ) ;
		// 全てのデータを送信完了
		virtual void OnSent( SAsyncSocket& socket ) ;
		// 切断された
		virtual void OnClose( SAsyncSocket& socket ) ;
	} ;

	class	SAsyncSocket	: public SSocket
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SAsyncSocket, SSocket )
		// 構築関数
		SAsyncSocket( void ) ;
		// 消滅関数
		virtual ~SAsyncSocket( void ) ;

	protected:
		SAsyncSocketListener *	m_pListener ;
		bool					m_flagThread ;
		bool					m_flagExitThread ;
		size_t					m_limitRecv ;
		SCriticalSection		m_csSync ;
		SQueueBuffer			m_qbufRecv ;
		SQueueBuffer			m_qbufSend ;
		SSignalEvent			m_doneThread ;
		SSignalEvent			m_signalWake ;
		SSignalEvent			m_signalAnyRecv ;
		SSignalEvent			m_signalAllSent ;

	public:
		// ソケットを閉じる
		virtual void Close( void ) ;
		// ソケット接続
		virtual SError Connect
			( const wchar_t * pwszHostAddress, uint32_t nHostPort ) ;
		// 受信
		virtual size_t Receive( void * ptrBuf, size_t nBytes ) ;
		// 送信
		virtual size_t Send( const void * ptrBuf, size_t nBytes ) ;
	protected:
		// Accept で接続された socket に対して呼び出される
		virtual void OnAccepted( void ) ;
		// 受信スレッドを起動する
		void BeginRecvThread( void ) ;

	protected:
		// スレッド関数
		static void AsyncSocketThreadProc( void * pInstance ) ;
		void AsyncSocketProc( void ) ;

	public:	// SFileInterface 実装
		// ファイルインターフェースの複製
		virtual SFileInterface * Duplicate( void ) const ;
		// ファイルから読み込み
		virtual size_t Read( void * ptrBuf, size_t nBytes ) ;
		// 改行コード (\n) まで読み込み
		virtual size_t ReadLine( uint8_t * ptrBuf, size_t nBytes ) ;
		// ファイルへ書き込み
		virtual size_t Write( const void * ptrBuf, size_t nBytes ) ;

	public:
		// 非同期リスナ設定
		void AttachListener( SAsyncSocketListener* pListener ) ;
		// 受信バッファの最大サイズを設定する
		void SetReceiveLimit( size_t nBytes ) ;
		// 未送信データバイト数を取得する
		size_t GetNotSentDataBytes( void ) const ;
		// 受信データを待機する
		SError WaitToReceive( int64_t msecTimeout ) ;
		// 全ての送信データを送信し終えるまで待機する
		SError WaitToSendAll( int64_t msecTimeout ) ;
		// ソケットが切断されるまで待機する
		SError WaitToClose( int64_t msecTimeout ) ;

	} ;

}

#endif
