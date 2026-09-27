
/*****************************************************************************
             Entis Generalized Library System version 3
													last update 2002/08/22
 ----------------------------------------------------------------------------
		Copyright (c) 1998-2002 Leshade Entis. All rights reserved.
 ****************************************************************************/



#if	!defined(__SOCKBASE_H__)
#define	__SOCKBASE_H__


//////////////////////////////////////////////////////////////////////////////
// TCP/IP ソケットクラス
//////////////////////////////////////////////////////////////////////////////

class	ESocket	: public	ESLObject
{
public:
	// 構築関数
	ESocket( void ) ;
	// 消滅関数
	virtual ~ESocket( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( ESocket, ESLObject )

protected:
	SOCKET				m_hSocket ;		// ソケット

	HANDLE				m_hCancelBlocking ;

	EStreamBuffer		m_bufSend ;		// 送信バッファ
	EStreamBuffer		m_bufRecv ;		// 受信バッファ
	HANDLE				m_hConnected ;	// 接続した
	HANDLE				m_hSendEmpty ;	// 送信バッファは空か？
	HANDLE				m_hRecvData ;	// 受信バッファは空ではないか？
	CRITICAL_SECTION	m_cs ;			// 排他アクセス

	DWORD				m_dwRecvBufLimit ;	// 受信バッファ最大サイズ

public:
	// ソケット作成
	ESLError Create
		( UINT nPort = 0, int nType = SOCK_STREAM,
			long int nEvent = FD_READ | FD_WRITE | FD_OOB
					| FD_ACCEPT | FD_CONNECT | FD_CLOSE,
			const char * pszAddress = NULL ) ;
	// ソケットを関連付ける
	ESLError Attach
		( SOCKET hSocket, long int nEvent =
			FD_READ | FD_WRITE | FD_OOB
				| FD_ACCEPT | FD_CONNECT | FD_CLOSE ) ;
	// ソケットをローカルアドレスに結びつける
	ESLError Bind( UINT nPort = 0, const char * pszAddress = NULL ) ;
	// ソケットを閉じる
	virtual void Close( void ) ;
	// ソケット接続
	ESLError Connect( const char * pszHostAddress, UINT nHostPort ) ;
	// 接続要求を待つ
	ESLError Listen( int nConnectionBacklog = 5 ) ;
	// 接続受け入れ
	ESLError Accept
		( ESocket & socket,
			SOCKADDR * pSockAddr = NULL, int * pSockAddrLen = NULL ) ;
	// 受信
	virtual int Receive( void * ptrBuf, int nBufLen ) ;
	// 送信
	virtual int Send( const void * ptrBuf, int nBufLen ) ;
	// １行受信
	virtual int ReceiveLine( EString & strLine ) ;
	// ブロッキングをキャンセル
	void CancelBlocking( void ) ;
	// ブロッキングがキャンセルされているか？
	bool IsBlockingCanceled( void ) const ;
	// 接続完了を待つ
	virtual ESLError WaitUntilConnected( DWORD dwTimeout, bool fDispMsg = false ) ;
	// 送信完了を待つ
	virtual ESLError WaitUntilSent( DWORD dwTimeout, bool fDispMsg = false ) ;
	// 何らかのデータを受信するまで待つ
	virtual ESLError WaitUntilReceived( DWORD dwTimeout, bool fDispMsg = false ) ;
	// イベントを待機
	virtual ESLError WaitEvent
		( HANDLE hEvent, DWORD dwTimeout, bool fDispMsg = false ) ;
	// 指定バイト数受信
	virtual int ReceiveTimeout
		( void * ptrBuf, int nBufLen, DWORD dwTimeout, bool fDispMsg = false ) ;
	// １行受信
	virtual int ReceiveLineTimeout
		( EString & strLine, DWORD dwTimeout, bool fDispMsg = false ) ;
	// 受信バッファ最大サイズを設定する
	virtual void SetReceiveBufferLimit( DWORD dwBufLimit ) ;

protected:
	// 受信データがある
	virtual void OnReceive( int nErrorCode ) ;
	// 送信可能状態になった
	virtual void OnSend( int nErrorCode ) ;
	// 帯域外データがある
	virtual void OnOutOfBandData( int nErrorCode ) ;
	// 接続要求受け入れ可能
	virtual void OnAccept( int nErrorCode ) ;
	// ソケットが接続された
	virtual void OnConnect( int nErrorCode ) ;
	// ソケットが閉じられた
	virtual void OnClose( int nErrorCode ) ;
	// 排他アクセス
	void Lock( void ) ;
	void Unlock( void ) ;

protected:
	static HANDLE			m_hThread ;		// 送受信スレッド
	static DWORD			m_dwThreadID ;
	static HWND				m_hWnd ;		// 通知ウィンドウ
	static EIntTagArray<ESocket> *
							m_pitaSocket ;	// ソケットリスト

	enum	WindowMessage
	{
		wmSockNotify	= WM_USER,
		wmExit,
		wmAdd,
		wmSend,
		wmClose
	} ;

public:
	// ESocket クラス初期化
	static void InitSocket( void ) ;
	// ESocket クラス終了
	static void ExitSocket( void ) ;

protected:
	// スレッドプロシージャ
	static DWORD WINAPI ThreadProc( LPVOID param ) ;
	// ウィンドウプロシージャ
	static LRESULT CALLBACK
		WindowProc( HWND hWnd, UINT nMsg, WPARAM wParam, LPARAM lParam ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
// HTTP (Hypertext Transfer Protocol)
//////////////////////////////////////////////////////////////////////////////

class	EHttpConnection	: public	ESocket
{
public:
	// 構築関数
	EHttpConnection( void ) ;
	// 消滅関数
	virtual ~EHttpConnection( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( EHttpConnection, ESocket )

protected:
	EString					m_strCmd ;			// コマンド
	EString					m_strHost ;			// ホスト名
	EString					m_strPath ;			// ファイルパス
	EString					m_strSendData ;		// 送信データ
	EObjArray<EString>		m_listSendHeader ;	// 送信ヘッダ
	EStrTagArray<EString>	m_staRecvHeader ;	// 受信ヘッダ
	EString					m_strLastHeader ;
	HANDLE					m_hHeaderRecv ;		// ヘッダを受信した
	HANDLE					m_hHtmlRecv ;		// </html> タグを発見した
	HANDLE					m_hFinishRecv ;		// 全データを受信した
	DWORD					m_dwStatusCode ;	// ステータスコード
	DWORD					m_dwTotalBytes ;	// 総バイト数
	DWORD					m_dwCurrentBytes ;	// 受信されたデータ量
	EStreamBuffer			m_bufRecvHttp ;		// 受信データ
	HANDLE					m_hContentRecv ;

public:
	// URL を開いてデータをダウンロードする
	ESLError OpenURL
		( const char * pszURL,
			DWORD dwTimeout = 10000, bool fDispMsg = false,
			const char * pszAgent = NULL, const char * pszData = NULL,
			unsigned int nLength = -1, const char * pszContentType = NULL ) ;
	// URL 設定
	void SetRequest
		( const char * pszURL, const char * pszCmd = "GET" ) ;
	// 送信データ設定
	void SetSendData( const char * pszData, unsigned int nLength = -1 ) ;
	// URL フォームパラメータを送信データに設定
	void SetSendURLFormData
		( const char * pszData, const char * pszCharset = NULL ) ;
	// 送信ヘッダ設定
	void AddHeader( const char * pszHeader ) ;
	// Accept 送信ヘッダ設定
	void SetHeaderAccept( const char * pszAccept = NULL ) ;
	// Accept-Encoding 送信ヘッダ設定
	void SetHeaderAcceptEncoding( const char * pszAcceptEncoding ) ;
	// User-Agent 送信ヘッダ設定
	void SetHeaderUserAgent( const char * pszUserAgent ) ;
	// ホストに接続
	ESLError ConnectHost( void ) ;
	// リクエスト送信
	ESLError SendRequest( void ) ;
	// ヘッダを受信し終えているか？
	bool HasHeaderReceived( void ) const ;
	// ヘッダを受信するまで待機
	ESLError WaitRecvHeader( DWORD dwTimeout, bool fDispMsg = false ) ;
	// HTML データを受信し終えるまで待機
	ESLError WaitRecvHTMLData( DWORD dwTimeout, bool fDispMsg = false ) ;
	// 内容を受信し終えているか？
	bool HasContentsReceived( void ) const ;
	// 全て受信するまで待機
	ESLError WaitRecvContents( DWORD dwTimeout, bool fDispMsg = false ) ;
	// ステータスコードを取得
	DWORD GetStatusCode( void ) const ;
	// 現在の受信状況取得
	DWORD GetCurrentStatus( DWORD * pdwTotal ) const ;

public:
	// 受信
	virtual int Receive( void * ptrBuf, int nBufLen ) ;
	// １行受信
	virtual int ReceiveLine( EString & strLine ) ;
	// 何らかのデータを受信するまで待つ
	virtual ESLError WaitUntilReceived( DWORD dwTimeout, bool fDispMsg = false ) ;

public:
	// URL フォーマット
	static EString FormatURL( const char * pszURL ) ;
	// URL フォーマットを復元
	static EString UnformatURL( const char * pszURL ) ;

protected:
	// 受信データがある
	virtual void OnReceive( int nErrorCode ) ;
	// ソケットが閉じられた
	virtual void OnClose( int nErrorCode ) ;

protected:
	const char *	m_pszEndOfHTML ;
	unsigned int	m_iSeekEndOfHTML ;

protected:
	// HTML データの終端記号チェックの準備処理
	virtual void OnBeginHTMLData( void ) ;
	// HTML データの終端記号のチェック
	virtual bool IsEndOfHTMLData
		( const BYTE * ptrBuf, unsigned long int nBufLen ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
// HTTP (Hypertext Transfer Protocol) ファイル
//////////////////////////////////////////////////////////////////////////////

class	ESyncHttpFile	: public ESyncStreamFile, public EHttpConnection
{
public:
	// 構築関数
	ESyncHttpFile( void ) ;
	// 消滅関数
	virtual ~ESyncHttpFile( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO2( ESyncHttpFile, ESyncStreamFile, EHttpConnection )

public:
	// ファイルを開く
	ESLError OpenURL
		( const char * pszURL,
			DWORD dwTimeout = 10000, bool fDispMsg = false,
			const char * pszAgent = NULL, const char * pszData = NULL,
			unsigned int nLength = -1, const char * pszContentType = NULL ) ;
	// ファイルを閉じる
	virtual void Close( void ) ;

public:
	// 排他アクセス
	void Lock( void ) ;
	void Unlock( void ) ;

protected:
	// 受信データがある
	virtual void OnReceive( int nErrorCode ) ;
	// ソケットが閉じられた
	virtual void OnClose( int nErrorCode ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
// FTP (FILE TRANSFER PROTOCOL)
//////////////////////////////////////////////////////////////////////////////

class	EFtpConnection	: public	ESocket
{
public:
	// 構築関数
	EFtpConnection( void ) ;
	// 消滅関数
	virtual ~EFtpConnection( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( EFtpConnection, ESocket )

protected:
	DWORD		m_dwDataSize ;

public:
	// FTP サーバに接続する
	ESLError ConnectHost
		( const char * pszHostAddr,
			int nPort = 21, DWORD dwTimeout = 10000 ) ;
	// FTP サーバにログインする
	ESLError Login
		( const char * pszUser,
			const char * pszPassword, DWORD dwTimeout = 10000 ) ;
	// コマンドを実行する
	ESLError SendCommand
		( const char * pszCmdLine,
			EString & strResponse, DWORD dwTimeout = 10000 ) ;
	// PASV コマンドを実行する
	ESLError PassiveMode
		( ESocket & sockData, DWORD dwTimeout = 10000 ) ;

public:
	// FTP 上のファイルを開く（ダウンロード）
	ESLError OpenURL
		( ESocket & sockData,
			const char * pszURL, const char * pszUser,
			const char * pszPassword, DWORD dwTimeout = 10000 ) ;
	// FTP 上のファイルリストを取得する
	ESLError OpenList
		( ESocket & sockData,
			const char * pszURL, const char * pszUser,
			const char * pszPassword, DWORD dwTimeout = 10000 ) ;
	// FTP にファイルをアップロードする
	ESLError OpenUpload
		( ESocket & sockData,
			const char * pszURL, const char * pszUser,
			const char * pszPassword, DWORD dwTimeout = 10000 ) ;
	// FTP を開く（共通動作）
	ESLError OpenFTP
		( ESocket & sockData,
			const char * pszHost, const char * pszUser,
			const char * pszPassword, DWORD dwTimeout = 10000 ) ;
	// URL からホスト名とパスを分離
	static void ParseURL
		( EString & strHost, EString & strPath,
			EString & strUser, EString & strPassword, const char * pszURL ) ;

public:
	// ダウンロード中のファイルサイズを取得
	DWORD GetDownloadDataSize( void ) const
		{
			return	m_dwDataSize ;
		}

} ;


//////////////////////////////////////////////////////////////////////////////
// ソケットファイルインターフェース
//////////////////////////////////////////////////////////////////////////////

class	ESocketFile	: public ESLFileObject, public ESocket
{
public:
	// 構築関数
	ESocketFile( void ) ;
	// 消滅関数
	virtual ~ESocketFile( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO2( ESocketFile, ESLFileObject, ESocket )

public:
	// ファイルオブジェクトを複製する
	virtual ESLFileObject * Duplicate( void ) const ;

public:
	// ファイルから読み込む
	virtual unsigned long int Read
		( void * ptrBuffer, unsigned long int nBytes ) ;
	// ファイルへ書き出す
	virtual unsigned long int Write
		( const void * ptrBuffer, unsigned long int nBytes ) ;

public:
	// ファイルの長さを取得
	virtual unsigned long int GetLength( void ) const ;
	// ファイルポインタを移動
	virtual unsigned long int Seek
		( long int nOffsetPos, SeekOrigin fSeekFrom ) ;
	// ファイルポインタを取得
	virtual unsigned long int GetPosition( void ) const ;
	// ファイルの終端を現在の位置に設定する
	virtual ESLError SetEndOfFile( void ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
// FTP (FILE TRANSFER PROTOCOL) ファイルインターフェース
//////////////////////////////////////////////////////////////////////////////

class	ESyncFtpFile	: public ESyncStreamFile, public ESocket
{
public:
	// 構築関数
	ESyncFtpFile( void ) ;
	// 消滅関数
	virtual ~ESyncFtpFile( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO2( ESyncFtpFile, ESyncStreamFile, ESocket )

protected:
	EFtpConnection *	m_pFtpConnection ;

public:
	// FTP 上のファイルを開く
	ESLError OpenURL
		( const char * pszURL, const char * pszUser,
			const char * pszPassword, DWORD dwTimeout = 10000 ) ;
	// FTP 上のファイルリストを取得する
	ESLError OpenList
		( const char * pszURL, const char * pszUser,
			const char * pszPassword, DWORD dwTimeout = 10000 ) ;
	// FTP にファイルをアップロードする
	ESLError OpenUpload
		( const char * pszURL, const char * pszUser,
			const char * pszPassword, DWORD dwTimeout = 10000 ) ;
	// ファイルを閉じる
	virtual void Close( void ) ;

public:
	// 排他アクセス
	void Lock( void ) ;
	void Unlock( void ) ;

public:
	// ファイルから読み込む
	virtual unsigned long int Read
		( void * ptrBuffer, unsigned long int nBytes ) ;
	// ファイルへ書き出す
	virtual unsigned long int Write
		( const void * ptrBuffer, unsigned long int nBytes ) ;
	// ファイルの長さを取得
	virtual unsigned long int GetLength( void ) const ;
	// ファイルポインタを移動
	virtual unsigned long int Seek
		( long int nOffsetPos, SeekOrigin fSeekFrom ) ;
	// ファイルポインタを取得
	virtual unsigned long int GetPosition( void ) const ;
	// ファイルの終端を現在の位置に設定する
	virtual ESLError SetEndOfFile( void ) ;

protected:
	// 受信データがある
	virtual void OnReceive( int nErrorCode ) ;
	// ソケットが閉じられた
	virtual void OnClose( int nErrorCode ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
// GCTP (Game Command Transfer Protocol)
//////////////////////////////////////////////////////////////////////////////

class	EGctpConnection	: public	ESocket
{
public:
	// 構築関数
	EGctpConnection( void ) ;
	// 消滅関数
	virtual ~EGctpConnection( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( EGctpConnection, ESocket )

protected:
	EObjArray<EDescription>	m_queRecvCmd ;		// 受信コマンド待ち行列
	HANDLE					m_hRecvAnyCmd ;		// 受信コマンドがあるか？

	HWND					m_hWndNotify ;		// 通知ウィンドウ
	UINT					m_nMsgNotify ;

	ERISADecodeContext *	m_pDecodeERISA ;
	ERISADecodeContext *	m_pDecodeBSHF ;
	ERISAEncodeContext *	m_pEncodeERISA ;
	ERISAEncodeContext *	m_pEnocdeBSHF ;

public:
	// ソケットを閉じる
	virtual void Close( void ) ;
	// 何らかのコマンドを受信したことを通知するメッセージを設定
	void SetNotifyWindow( HWND hWndNotify, UINT nMsg ) ;
	// 何らかのコマンドを受信するまで待機
	ESLError WaitForCommand( DWORD dwTimeout, bool fDispMsg = false ) ;
	// コマンドを取得
	ESLError GetCommand( EDescription & cmd, const char * pszCmd = NULL ) ;
	// コマンドを送信
	ESLError SendCommand( const EDescription & cmd, bool fEncoding ) ;
	// 符号化方式設定
	ESLError SetEncodingType( int fEncodingType, const char * pszPassword ) ;

protected:
	// コマンドをエンコードする
	void EncodeCommand
		( EString & strCmd, const EDescription & cmd, bool fEncoding ) ;
	// コマンドをデコードする
	ESLError DecodeCommand( EDescription & cmd, EString & strCmd ) ;
	// パラメータをエンコードする
	void EncodeParameter( EString & strBuf ) ;
	// パラメータをデコードする
	void DecodeParameter( EString & strBuf ) ;

protected:
	// 受信データがある
	virtual void OnReceive( int nErrorCode ) ;

} ;


#endif
