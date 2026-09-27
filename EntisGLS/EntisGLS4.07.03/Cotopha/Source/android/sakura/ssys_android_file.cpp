
#include <sakura/sakura.h>
#include <sakura/ssys_android_file.h>
#include <sakura/ssys_smart_buffer.h>

using namespace SSystem ;


//////////////////////////////////////////////////////////////////////////////
// Android 環境特有の情報取得
//////////////////////////////////////////////////////////////////////////////

// Java パッケージ名取得
//////////////////////////////////////////////////////////////////////////////
void JNI::GetAndroidJavaPackageName( SSystem::SString& strPackage )
{
	JNI::JSmartClass	jsclsEntisGLS
		( JNI::FindJavaClass( ENTIS_GLS4_JAVA_PACKAGE "/EntisGLS" ) ) ;
	jmethodID	jmidGetPackageName =
		jsclsEntisGLS.GetStaticMethodID
			( "getPackageName", "()L" JAVA_LANG_STRING ";" ) ;
	JNI::JSmartObject	jsobjPackageName
		( jsclsEntisGLS.CallStaticObjectMethod( jmidGetPackageName ) ) ;
	JNI::JString	jstrPackageName( (jstring) jsobjPackageName.GetObject() ) ;
	jstrPackageName.ToString( strPackage ) ;
}

// ローカルファイルディレクトリ取得
//////////////////////////////////////////////////////////////////////////////
void JNI::GetAndroidLocalFilesDirectory( SSystem::SString& strDirPath )
{
	JNI::JSmartClass	jsclsEntisGLS
		( JNI::FindJavaClass( ENTIS_GLS4_JAVA_PACKAGE "/EntisGLS" ) ) ;
	jmethodID	jmidGetLocalFilesDirectoryPath =
		jsclsEntisGLS.GetStaticMethodID
			( "getLocalFilesDirectoryPath", "()L" JAVA_LANG_STRING ";" ) ;
	JNI::JSmartObject	jsobjLocalDir
		( jsclsEntisGLS.CallStaticObjectMethod( jmidGetLocalFilesDirectoryPath ) ) ;
	JNI::JString	jstrLocalDir( (jstring) jsobjLocalDir.GetObject() ) ;
	jstrLocalDir.ToString( strDirPath ) ;
}

// 外部ストレージディレクトリ取得
//////////////////////////////////////////////////////////////////////////////
void JNI::GetAndroidStorageDirectory( SSystem::SString& strDirPath )
{
	JNI::JSmartClass	jsclsEntisGLS
		( JNI::FindJavaClass( ENTIS_GLS4_JAVA_PACKAGE "/EntisGLS" ) ) ;
	jmethodID	jmidGetStorageDirectoryPath =
		jsclsEntisGLS.GetStaticMethodID
			( "getExternalStorageDirectoryPath", "()L" JAVA_LANG_STRING ";" ) ;
	JNI::JSmartObject	jsobjStorageDir
		( jsclsEntisGLS.CallStaticObjectMethod( jmidGetStorageDirectoryPath ) ) ;
	JNI::JString	jstrStorageDir( (jstring) jsobjStorageDir.GetObject() ) ;
	jstrStorageDir.ToString( strDirPath ) ;
}

// 外部ストレージプライベートディレクトリ取得
//////////////////////////////////////////////////////////////////////////////
void JNI::GetAndroidStoragePrivateDirectory( SSystem::SString& strDirPath )
{
	JNI::JSmartClass	jsclsEntisGLS
		( JNI::FindJavaClass( ENTIS_GLS4_JAVA_PACKAGE "/EntisGLS" ) ) ;
	jmethodID	jmidGetStoragePrivatePath =
		jsclsEntisGLS.GetStaticMethodID
			( "getExternalStoragePrivatePath", "()L" JAVA_LANG_STRING ";" ) ;
	JNI::JSmartObject	jsobjStorageDir
		( jsclsEntisGLS.CallStaticObjectMethod( jmidGetStoragePrivatePath ) ) ;
	JNI::JString	jstrStorageDir( (jstring) jsobjStorageDir.GetObject() ) ;
	jstrStorageDir.ToString( strDirPath ) ;
}



//////////////////////////////////////////////////////////////////////////////
// 疑似コンソール（標準入力・標準出力）
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SConsoleFile, SFileInterface )

// ファイルインターフェースの複製
//////////////////////////////////////////////////////////////////////////////
SFileInterface * SConsoleFile::Duplicate( void ) const
{
	return	new SConsoleFile ;
}

// ファイルから読み込み
//////////////////////////////////////////////////////////////////////////////
size_t SConsoleFile::Read( void * ptrBuf, size_t nBytes )
{
	if ( m_qbufInput.GetLength() > 0 )
	{
		return	m_qbufInput.Read( ptrBuf, nBytes ) ;
	}
	//
	// EntisGLS.consoleInput() 呼び出し
	//
	atomic_int_t	countLocked = SSystem::UnlockAll() ;
	//
	JNI::JSmartClass	jsclsEntisGLS
		( JNI::FindJavaClass( ENTIS_GLS4_JAVA_PACKAGE "/EntisGLS" ) ) ;
	jmethodID	jmidConsoleInput =
			jsclsEntisGLS.GetStaticMethodID
					( "consoleInput", "()L" JAVA_LANG_STRING ";" ) ;
	JNI::JSmartObject	jsobjString =
		jsclsEntisGLS.CallStaticObjectMethod( jmidConsoleInput ) ;
	//
	SSystem::Relock( countLocked ) ;
	//
	// String を UTF-8 へエンコード
	//
	JNI::JString	jstrConsole( (jstring) jsobjString.GetObject() ) ;
	const jchar *	pjszJStr = jstrConsole.GetBuffer() ;
	jsize			lenJStr = jstrConsole.GetLength() ;
	//
	SArray<wchar_t>	strTemp ;
	wchar_t *		pwszTemp ;
	strTemp.SetLength( lenJStr ) ;
	pwszTemp = strTemp.GetArray() ;
	for ( jsize i = 0; i < lenJStr; i ++ )
	{
		pwszTemp[i] = (wchar_t) pjszJStr[i] ;
	}
	//
	SArray<uint8_t>	strDstUTF8 ;
	Charset::Encode( strDstUTF8, Charset::encodingUTF8, pwszTemp, lenJStr ) ;
	//
	// 一時バッファへ書き出しと読み出し
	//
	m_qbufInput.Write( strDstUTF8.GetArray(), strDstUTF8.GetLength() ) ;
	return	m_qbufInput.Read( ptrBuf, nBytes ) ;
}

// ファイルへ書き込み
//////////////////////////////////////////////////////////////////////////////
size_t SConsoleFile::Write( const void * ptrBuf, size_t nBytes )
{
	//
	// UTF-8 として複製
	//
	SArray<char>	strBuf ;
	strBuf.SetLength( nBytes + 1 ) ;
	char *	pszBuf = strBuf.GetArray() ;
	for ( size_t i = 0; i < nBytes; i ++ )
	{
		pszBuf[i] = ((uint8_t*)ptrBuf)[i] ;
	}
	pszBuf[nBytes] = 0 ;
	//
	// EntisGLS.consoleOutput() 呼び出し
	//
	atomic_int_t	countLocked = SSystem::UnlockAll() ;
	//
	JNI::JSmartClass	jsclsEntisGLS
		( JNI::FindJavaClass( ENTIS_GLS4_JAVA_PACKAGE "/EntisGLS" ) ) ;
	jmethodID	jmidConsoleOutput =
			jsclsEntisGLS.GetStaticMethodID
					( "consoleOutput", "(L" JAVA_LANG_STRING ";)V" ) ;
	JNI::JavaObject	jobjString ;
	jsclsEntisGLS.CallStaticVoidMethod
		( jmidConsoleOutput, jobjString.CreateUTFString(pszBuf) ) ;
	//
	SSystem::Relock( countLocked ) ;
	//
	return	nBytes ;
}

// シーク可能か否か？
//////////////////////////////////////////////////////////////////////////////
bool SConsoleFile::IsSeekable( void ) const
{
	return	false ;
}

// ファイル長の取得
//////////////////////////////////////////////////////////////////////////////
int64_t SConsoleFile::GetLength( void ) const
{
	return	-1 ;
}

// ファイルポインタを移動
//////////////////////////////////////////////////////////////////////////////
int64_t SConsoleFile::Seek
	( int64_t posFile, SFileInterface::SeekOrigin seekFrom )
{
	return	-1 ;
}

// ファイルポインタを取得
//////////////////////////////////////////////////////////////////////////////
int64_t SConsoleFile::GetPosition( void ) const
{
	return	-1 ;
}

// ファイルの終端を現在の位置に設定する
//////////////////////////////////////////////////////////////////////////////
SError SConsoleFile::SetEndOfFile( void )
{
	return	errFailed ;
}



//////////////////////////////////////////////////////////////////////////////
// assets オープナー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SAssetFileOpener, SFileOpener )

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SFileInterface * SAssetFileOpener::NewOpenFile
	( const wchar_t * pszFilePath, long int nOpenFlags )
{
	if ( !IsExisting( pszFilePath ) )
	{
		return	NULL ;
	}
	SString	strFilePath = pszFilePath ;
	NormalizePath( strFilePath ) ;
	//
	// EntisGLS.openAssetFile を呼び出す
	//
	JNI::JSmartClass	jsclsEntisGLS
		( JNI::FindJavaClass( ENTIS_GLS4_JAVA_PACKAGE "/EntisGLS" ) ) ;
	jmethodID	jmidOpenAssetFile =
		jsclsEntisGLS.GetStaticMethodID
			( "openAssetFile",
				"(L" JAVA_LANG_STRING ";)Ljava/io/InputStream;" ) ;
	//
	JNI::JavaObject	jobjFilePath ;
	JNI::JavaObject	jobjInputStream =
		jsclsEntisGLS.CallStaticObjectMethod
			( jmidOpenAssetFile,
				jobjFilePath.CreateWideString( strFilePath ) ) ;
	//
	if ( jobjInputStream.GetObject() == NULL )
	{
		return	NULL ;
	}
	//
	// InputStream からメモリ上に読み込む
	//
	jmethodID	jmidRead =
			jobjInputStream.GetMethodID( "read", "([B)I" ) ;
	//
	JNI::JavaObject	jobjBuf ;
	jbyteArray	jbarrBuf = jobjBuf.CreateByteArray( 0x10000 ) ;
	//
	SSmartBuffer *	psbuf = new SSmartBuffer ;
	size_t	nPos = 0 ;
	for ( ; ; )
	{
		jint	nReadBytes =
			jobjInputStream.CallIntMethod( jmidRead, jbarrBuf ) ;
		if ( nReadBytes <= 0 )
		{
			break ;
		}
		JNI::JByteArray	jbarrBufObj ;
		nPos += psbuf->WriteBuffer
					( nPos, jbarrBufObj.GetBuffer(jbarrBuf), nReadBytes ) ;
	}
	psbuf->Seek( 0 ) ;
	//
	// InputStream を close
	//
	jmethodID	jmidClose = jobjInputStream.GetMethodID( "close", "()V" ) ;
	jobjInputStream.CallVoidMethod( jmidClose ) ;
	//
	JNI::GetJNIEnv()->ExceptionClear() ;
	//
	return	psbuf ;
}

// ファイルの存在
//////////////////////////////////////////////////////////////////////////////
bool SAssetFileOpener::IsExisting( const wchar_t * pszFilePath )
{
	//
	// ファイル名正規化
	//
	SString	strFilePath = pszFilePath ;
	NormalizePath( strFilePath ) ;
	//
	// 過去にダンプしたディレクトリを検索
	//
	SString	strFileDir = strFilePath.GetFileDirectoryPart() ;
	if ( strFileDir.GetLastAt(0) == L'/' )
	{
		ESLAssert( strFileDir.GetLength() > 0 ) ;
		strFileDir.SetLength( strFileDir.GetLength() - 1 ) ;
	}
	QuickLock() ;
	FileSet *	pFileSet = m_directories.GetAs( strFileDir ) ;
	QuickUnlock() ;
	//
	if ( pFileSet == NULL )
	{
		//
		// ファイル一覧
		//
		SObjectArray<SString>	listFiles ;
		ListSubFiles( listFiles, strFileDir ) ;
		//
		if ( listFiles.GetLength() > 0 )
		{
			pFileSet = new FileSet ;
			//
			size_t	nCount = listFiles.GetLength() ;
			for ( size_t i = 0; i < nCount; i ++ )
			{
				ESLAssert( listFiles.GetAt(i) != NULL ) ;
				pFileSet->SetAs( listFiles.At(i), true ) ;
			}
			//
			QuickLock() ;
			m_directories.SetAs( strFileDir, pFileSet ) ;
			QuickUnlock() ;
		}
		else
		{
			return	false ;
		}
	}
	ESLAssert( pFileSet != NULL ) ;
	return	(pFileSet->GetAs( SString(strFilePath.GetFileNamePart()) ) != NULL) ;
}

// ファイル状態
//////////////////////////////////////////////////////////////////////////////
SError SAssetFileOpener::QueryState
	( const wchar_t * pszFilePath, SFileOpener::State& state )
{
	return	errFailed ;
}

// ファイルの一覧取得
//////////////////////////////////////////////////////////////////////////////
void SAssetFileOpener::ListSubFiles
	( SObjectArray<SString>& listFiles, const wchar_t * pszDirPath )
{
	SString	strDirPath = pszDirPath ;
	NormalizePath( strDirPath ) ;
	//
	// EntisGLS.listAssetFiles を呼び出す
	//
	JNI::JSmartClass	jsclsEntisGLS
		( JNI::FindJavaClass( ENTIS_GLS4_JAVA_PACKAGE "/EntisGLS" ) ) ;
	jmethodID	jmidListAssetFiles =
			jsclsEntisGLS.GetStaticMethodID
					( "listAssetFiles",
						"(L" JAVA_LANG_STRING ";)[L" JAVA_LANG_STRING ";" ) ;
	//
	JNI::JavaObject		jobjFilePath ;
	JNI::JSmartObject	jsobjFileList =
		jsclsEntisGLS.CallStaticObjectMethod
			( jmidListAssetFiles,
				jobjFilePath.CreateWideString( strDirPath ) ) ;
	//
	listFiles.RemoveAll() ;
	if ( jsobjFileList.GetObject() != NULL )
	{
		//
		// 配列から文字列を取得する
		//
		JNI::JObjectArray	joarrFileList
				( (jobjectArray) jsobjFileList.GetObject() ) ;
		const jsize	nCount = joarrFileList.GetLength() ;
		for ( jsize i = 0; i < nCount; i ++ )
		{
			JNI::JSmartObject	jsobjStr( joarrFileList.GetAt( i ) ) ;
			if ( jsobjStr.GetObject() != NULL )
			{
				JNI::JString	jstrFile( (jstring) jsobjStr.GetObject() ) ;
				SString *		pstrFile = new SString ;
				jstrFile.ToString( *pstrFile ) ;
				listFiles.Add( pstrFile ) ;
			}
		}
	}
}

// ディレクトリの一覧取得
//////////////////////////////////////////////////////////////////////////////
void SAssetFileOpener::ListSubDirectories
	( SObjectArray<SString>& listDirs, const wchar_t * pszDirPath )
{
}

// ファイルパスを正規化
//////////////////////////////////////////////////////////////////////////////
void SAssetFileOpener::NormalizePath( SString& strPath )
{
	strPath.Replace( L'\\', L'/' ) ;
	//
	size_t	i ;
	for ( i = 0; i < strPath.GetLength(); i ++ )
	{
		if ( strPath.GetAt(i) != L'/' )
		{
			break ;
		}
	}
	if ( i > 0 )
	{
		strPath = strPath.Middle( i ) ;
	}
}



//////////////////////////////////////////////////////////////////////////////
// HTTP ファイル
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
	( SSystem::SAndroidHttpFile, SHttpFileInterface, JavaObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SAndroidHttpFile::SAndroidHttpFile( void )
{
	CreateJavaObject( ENTIS_GLS4_JAVA_PACKAGE "/HttpFile" ) ;
	MakeGlobalRef() ;
	//
	m_jmidSetRequest = GetMethodID
		( "setRequest",
			"(L" JAVA_LANG_STRING ";L" JAVA_LANG_STRING ";)Z" ) ;
	m_jmidAddHeader = GetMethodID
		( "addHeader",
			"(L" JAVA_LANG_STRING ";L" JAVA_LANG_STRING ";)Z" ) ;
	m_jmidSendData = GetMethodID( "setSendData", "([B)V" ) ;
	m_jmidConnect = GetMethodID( "connect", "()Z" ) ;
	m_jmidClose = GetMethodID( "close", "()V" ) ;
	m_jmidGetStatusCode = GetMethodID( "getStatusCode", "()I" ) ;
	m_jmidGetReceiveHeader = GetMethodID
		( "getReceiveHeader",
			"(L" JAVA_LANG_STRING ";)L" JAVA_LANG_STRING ";" ) ;
	m_jmidRead = GetMethodID( "read", "([B)I" ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SAndroidHttpFile::~SAndroidHttpFile( void )
{
	if ( GetObject() != NULL )
	{
		CallVoidMethod( m_jmidClose ) ;
	}
}

// URL 設定
//////////////////////////////////////////////////////////////////////////////
SError SAndroidHttpFile::SetRequest
	( const wchar_t * pwszURL, const wchar_t * pwszCmd )
{
	if ( GetObject() == NULL )
	{
		return	errFailed ;
	}
	JNI::JavaObject	jobjURL, jobjCmd ;
	if ( !CallBooleanMethod
		( m_jmidSetRequest,
			jobjURL.CreateWideString(pwszURL),
			jobjCmd.CreateWideString(pwszCmd) ) )
	{
		return	errFailed ;
	}
	return	errSuccess ;
}

// 送信データ設定
//////////////////////////////////////////////////////////////////////////////
SError SAndroidHttpFile::SetSendData
	( const uint8_t * pbytData, ssize_t nBytes )
{
	if ( GetObject() == NULL )
	{
		return	errFailed ;
	}
	JNI::JavaObject	jobjData ;
	JNI::JByteArray	jbarrData( jobjData.CreateByteArray( nBytes ) ) ;
	eslMoveMemory( jbarrData.GetBuffer(), pbytData, nBytes ) ;
	CallVoidMethod( m_jmidSendData, jobjData.GetObject() ) ;
	return	errSuccess ;
}

// 送信ヘッダ設定
//////////////////////////////////////////////////////////////////////////////
SError SAndroidHttpFile::AddHeader( const wchar_t * pwszHeader )
{
	if ( GetObject() == NULL )
	{
		return	errFailed ;
	}
	SStringParser	sparsHeader = pwszHeader ;
	SString			strField, strValue ;
	if ( sparsHeader.NextEnclosedString( strField, L':' ) != L':' )
	{
		return	errFailed ;
	}
	sparsHeader.PassSpace() ;
	strValue = sparsHeader.SubString( sparsHeader.GetIndex() ) ;
	//
	JNI::JavaObject	jobjField, jobjValue ;
	if ( !CallBooleanMethod
		( m_jmidAddHeader,
			jobjField.CreateWideString(strField),
			jobjValue.CreateWideString(strValue) ) )
	{
		return	errFailed ;
	}
	return	errSuccess ;
}

// サーバへ接続
//////////////////////////////////////////////////////////////////////////////
SError SAndroidHttpFile::Connect( uint32_t nFlags )
{
	if ( GetObject() == NULL )
	{
		return	errFailed ;
	}
	if ( !CallBooleanMethod( m_jmidConnect ) )
	{
		return	errFailed ;
	}
	return	errSuccess ;
}

// リクエスト送信
//////////////////////////////////////////////////////////////////////////////
SError SAndroidHttpFile::SendRequest( void )
{
	return	errSuccess ;
}

// HTTP ステータスコード取得
//////////////////////////////////////////////////////////////////////////////
SError SAndroidHttpFile::QueryStatusCode( uint32_t& codeStatus ) const
{
	if ( GetObject() == NULL )
	{
		return	errFailed ;
	}
	codeStatus = (uint32_t) CallIntMethod( m_jmidGetStatusCode ) ;
	return	errSuccess ;
}

// HTTP データ長取得
//////////////////////////////////////////////////////////////////////////////
SError SAndroidHttpFile::QueryContentLength( uint64_t& numLength ) const
{
	SString	strContentLength ;
	if ( QueryHeader( strContentLength, L"content-length" ) )
	{
		return	errFailed ;
	}
	bool	flagError ;
	numLength = strContentLength.AsInteger( 10, false, &flagError ) ;
	if ( flagError )
	{
		return	errFailed ;
	}
	return	errSuccess ;
}

// HTTP データタイプ取得
//////////////////////////////////////////////////////////////////////////////
SError SAndroidHttpFile::QueryContentType( SString& strType ) const
{
	return	QueryHeader( strType, L"content-type" ) ;
}

// HTTP データエンコーディング取得
//////////////////////////////////////////////////////////////////////////////
SError SAndroidHttpFile::QueryContentTransferEncoding( SString& strEncoding ) const
{
	return	QueryHeader( strEncoding, L"content-transfer-encoding" ) ;
}

// HTTP Date 取得
//////////////////////////////////////////////////////////////////////////////
SError SAndroidHttpFile::QueryContentDate( DATE_TIME& dt ) const
{
	SString	strDate ;
	if ( QueryHeader( strDate, L"date" ) )
	{
		return	errFailed ;
	}
	return	ParseDate( dt, strDate ) ;
}

// HTTP Last-Modified 取得
//////////////////////////////////////////////////////////////////////////////
SError SAndroidHttpFile::QueryContentLastModified( DATE_TIME& dt ) const
{
	SString	strDate ;
	if ( QueryHeader( strDate, L"last-modified" ) )
	{
		return	errFailed ;
	}
	return	ParseDate( dt, strDate ) ;
}

// 受信ヘッダ取得
//////////////////////////////////////////////////////////////////////////////
SError SAndroidHttpFile::QueryHeader
	( SString& strValue, const wchar_t * pwszName ) const
{
	if ( GetObject() == NULL )
	{
		return	errFailed ;
	}
	JNI::JavaObject		jobjFieldName ;
	JNI::JSmartObject	jsobjStr =
		CallObjectMethod
			( m_jmidGetReceiveHeader,
				jobjFieldName.CreateWideString(pwszName) ) ;
	if ( jsobjStr.GetObject() == NULL )
	{
		return	errFailed ;
	}
	JNI::JString	jstrValue( (jstring) jsobjStr.GetObject() ) ;
	jstrValue.ToString( strValue ) ;
	return	errSuccess ;
}

// ファイルインターフェースの複製
//////////////////////////////////////////////////////////////////////////////
SFileInterface * SAndroidHttpFile::Duplicate( void ) const
{
	return	new SAndroidHttpFile ;
}

// ファイルから読み込み
//////////////////////////////////////////////////////////////////////////////
size_t SAndroidHttpFile::Read( void * ptrBuf, size_t nBytes )
{
	if ( GetObject() == NULL )
	{
		return	errFailed ;
	}
	if ( nBytes == 0 )
	{
		return	0 ;
	}
	JNI::JavaObject	jobjBuf ;
	jint	nReadBytes =
		CallIntMethod
			( m_jmidRead, jobjBuf.CreateByteArray( nBytes ) ) ;
	if ( nReadBytes <= 0 )
	{
		return	0 ;
	}
	if ( nReadBytes < nBytes )
	{
		nBytes = nReadBytes ;
	}
	JNI::JByteArray	jbarrData( (jbyteArray) jobjBuf.GetObject() ) ;
	eslMoveMemory( ptrBuf, jbarrData.GetBuffer(), nBytes ) ;
	jbarrData.ReleaseBuffer() ;
	return	nBytes ;
}


