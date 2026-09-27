
#if	!defined(__SAKURA2_ANDROID_FILE_H__)
#define	__SAKURA2_ANDROID_FILE_H__

#include <sakura/ssys_queue_buffer.h>
#include <sakura/ssys_http_file.h>
#include <esl/esl_java_object.h>

namespace	JNI
{
	//////////////////////////////////////////////////////////////////////////
	// Android 環境特有の情報取得
	//////////////////////////////////////////////////////////////////////////

	// Java パッケージ名取得
	void GetAndroidJavaPackageName( SSystem::SString& strPackage ) ;

	// ローカルファイルディレクトリ取得
	void GetAndroidLocalFilesDirectory( SSystem::SString& strDirPath ) ;

	// 外部ストレージディレクトリ取得
	void GetAndroidStorageDirectory( SSystem::SString& strDirPath ) ;

	// 外部ストレージプライベートディレクトリ取得
	void GetAndroidStoragePrivateDirectory( SSystem::SString& strDirPath ) ;
}


namespace	SSystem
{

	//////////////////////////////////////////////////////////////////////////
	// 疑似コンソール（標準入力・標準出力）
	//////////////////////////////////////////////////////////////////////////

	class	SConsoleFile : public SFileInterface
	{
	protected:
		SQueueBuffer	m_qbufInput ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SConsoleFile, SFileInterface )

	public:
		// ファイルインターフェースの複製
		virtual SFileInterface * Duplicate( void ) const ;

	public:
		// ファイルから読み込み
		virtual size_t Read( void * ptrBuf, size_t nBytes ) ;
		// ファイルへ書き込み
		virtual size_t Write( const void * ptrBuf, size_t nBytes ) ;

	public:
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
	// assets オープナー
	//////////////////////////////////////////////////////////////////////////

	class	SAssetFileOpener : public SFileOpener
	{
	protected:
		// ディレクトリ・ファイル集合
		class	FileSet	: public SStrSortArray<bool>
		{
		public:
			FileSet( void ) {}
			FileSet( const FileSet& src )
			{
				SStrSortArray<bool>::DuplicateArray( src ) ;
			}
		} ;
		SStrSortObjectArray<FileSet>	m_directories ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SAssetFileOpener, SFileOpener )

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

	public:
		// ファイルパスを正規化
		static void NormalizePath( SString& strPath ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// HTTP ファイル
	//////////////////////////////////////////////////////////////////////////

	class	SAndroidHttpFile
				: public SHttpFileInterface, public JNI::JavaObject
	{
	protected:
		jmethodID	m_jmidSetRequest ;
		jmethodID	m_jmidAddHeader ;
		jmethodID	m_jmidSendData ;
		jmethodID	m_jmidConnect ;
		jmethodID	m_jmidClose ;
		jmethodID	m_jmidGetStatusCode ;
		jmethodID	m_jmidGetReceiveHeader ;
		jmethodID	m_jmidRead ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
			( SAndroidHttpFile, SHttpFileInterface, JavaObject )
		// 構築関数
		SAndroidHttpFile( void ) ;
		// 消滅関数
		virtual ~SAndroidHttpFile( void ) ;

	public:	// SHttpFileInterface 実装
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
			( SString& strValue, const wchar_t * pwszName ) const ;

	public:	// SFileInterface 実装
		// ファイルインターフェースの複製
		virtual SFileInterface * Duplicate( void ) const ;
		// ファイルから読み込み
		virtual size_t Read( void * ptrBuf, size_t nBytes ) ;

	} ;


}

#endif


