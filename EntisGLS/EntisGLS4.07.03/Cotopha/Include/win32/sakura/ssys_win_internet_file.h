
#if	!defined(__SAKURA2_WIN_INTERNET_FILE_H__)
#define	__SAKURA2_WIN_INTERNET_FILE_H__

#include <wininet.h>

namespace	SSystem
{
	//////////////////////////////////////////////////////////////////////////
	// インターネットセッション
	//////////////////////////////////////////////////////////////////////////

	class	SInternetSession	: public SObject
	{
	protected:
		HINTERNET	m_hInternet ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SInternetSession, SObject )
		// 構築関数
		SInternetSession( void ) ;
		// 消滅関数
		virtual ~SInternetSession( void ) ;

	public:
		// セッションを開く
		SError Open
			( const wchar_t * pszAgent = NULL,
				DWORD dwAccessType = PRE_CONFIG_INTERNET_ACCESS,
				const wchar_t * pszProxyName = NULL,
				const wchar_t * pszProxyBypass = NULL, DWORD dwFlags = 0 ) ;
		// セッションを閉じる
		void Close( void ) ;
		// コールバック設定
		SError EnableCallback( bool fCallback = true ) ;
		// オプション取得
		SError QueryOption
			( DWORD dwOption, void * pBuffer, DWORD * pdwBufLen ) ;
		// オプション設定
		SError SetOption
			( DWORD dwOption, void * pBuffer,
				DWORD dwBufferLength, DWORD dwFlags = 0 ) ;
		// クッキー設定
		static SError SetCookie
			( const wchar_t * pszURL,
				const wchar_t * pszCookieName, const wchar_t * pszCookieData ) ;
		// クッキー取得
		static SError GetCookie
			( const wchar_t * pszURL,
				const wchar_t * pszCookieName, SString & strCookieData ) ;
		// セッションハンドル取得
		operator HINTERNET ( void ) const
			{
				return	m_hInternet ;
			}

	protected:
		static ESL_DLL_EXPORT SInternetSession *	m_pisDefault ;

	public:
		// デフォルトセッション取得
		static SInternetSession * GetInstance( void ) ;
		// デフォルトセッション設定
		static void AttachInstance( SInternetSession * pisDef ) ;

	protected:
		// コールバック用チェーン
		SInternetSession *							m_pisPrev ;
		SInternetSession *							m_pisNext ;
		static ESL_DLL_EXPORT SInternetSession *	m_pisFirst ;
		static void CALLBACK InternetStatusCallback
			( HINTERNET hInternet, DWORD_PTR dwContext,
				DWORD dwInternetStatus,
				LPVOID lpvStatusInformation, DWORD dwStatusInformationLength ) ;
		void AddChain( void ) ;
		void DetachChain( void ) ;

	protected:
		// コールバック関数
		virtual void OnStatusCallback
			( DWORD_PTR dwContext, DWORD dwInternetStatus,
				LPVOID lpvStatusInformation, DWORD dwStatusInformationLength ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// インターネットファイル (HTTP, HTTPS)
	//////////////////////////////////////////////////////////////////////////

	class	SInternetFile	: public SHttpFileInterface
	{
	protected:
		SInternetSession *	m_pSession ;
		HINTERNET			m_hConnect ;
		HINTERNET			m_hFile ;
		SString				m_strCmd ;
		SString				m_strScheme ;
		SString				m_strHost ;
		SString				m_strPort ;
		SString				m_strUser ;
		SString				m_strPassword ;
		SString				m_strPath ;
		uint32_t			m_nConnectFlags ;
		SArray<uint8_t>		m_bufSendData ;
		SStrSortObjectArray<SString>	m_sendHeader ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SInternetFile, SHttpFileInterface )
		// 構築関数
		SInternetFile( void ) ;
		// 消滅関数
		virtual ~SInternetFile( void ) ;
		// SInternetSession 関連付け
		void AttachSession( SInternetSession * pis ) ;
		// 終了処理
		void Close( void ) ;

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
		// 受信ヘッダ文字列取得
		virtual SError QueryInfoString
			( SString& strValue, DWORD dwInfoLevel ) const ;

	public:	// SFileInterface オーバーライド
		// ファイルインターフェースの複製
		virtual SFileInterface * Duplicate( void ) const ;
		// ファイルから読み込み
		virtual size_t Read( void * ptrBuf, size_t nBytes ) ;
		// ファイルへ書き込み
		virtual size_t Write( const void * ptrBuf, size_t nBytes ) ;
	} ;

}

#endif

