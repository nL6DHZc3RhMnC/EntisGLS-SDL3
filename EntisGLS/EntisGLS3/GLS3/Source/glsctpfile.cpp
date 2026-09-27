
/*****************************************************************************
				Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
    Copyright (c) 2003-2013 Leshade Entis, Entis-soft. All rights reserved.
 ****************************************************************************/


#include <gls.h>


//////////////////////////////////////////////////////////////////////////////
// ファイル入出力用オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ECSFile, ECSObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ECSFile::ECSFile( void )
{
	m_pFile = NULL ;
	m_dwOpenFlags = 0 ;
	m_nCharaEncoding = EDescription::ceShiftJIS ;
	m_ppif = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ECSFile::~ECSFile( void )
{
	Close( ) ;
	//
	if ( m_ppif != NULL )
	{
		::eslHeapFree( NULL, m_ppif, 0 ) ;
	}
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Open
	( const wchar_t * pwszFileName,
		DWORD dwOpenFlags, ECSContext * pContext )
{
	Close( ) ;
	//
	if ( (pwszFileName != NULL) && (pwszFileName[0] != L'\0') )
	{
		if ( pContext != NULL )
		{
			m_pFile = pContext->OpenFileOnScript( pwszFileName, dwOpenFlags ) ;
		}
		if ( m_pFile == NULL )
		{
			ERawFile *	pfile = new ERawFile ;
			if ( pfile->Open( EString(pwszFileName), dwOpenFlags ) )
			{
				delete	pfile ;
				return	eslErrGeneral ;
			}
			m_pFile = pfile ;
		}
		m_strFileName.m_varStr = pwszFileName ;
	}
	else
	{
		ESLError	err ;
		ERawFile *	pfile = new ERawFile ;
		if ( dwOpenFlags & ESLFileObject::modeWrite )
		{
			err = pfile->Create
				( ::GetStdHandle( STD_OUTPUT_HANDLE ), dwOpenFlags ) ;
		}
		else
		{
			err = pfile->Create
				( ::GetStdHandle( STD_INPUT_HANDLE ), dwOpenFlags ) ;
		}
		if ( err )
		{
			delete	pfile ;
			return	eslErrGeneral ;
		}
		m_pFile = pfile ;
		m_strFileName.m_varStr = L"" ;
	}
	m_stackFile.Push( m_pFile ) ;
	m_dwOpenFlags = dwOpenFlags ;
	return	eslErrSuccess ;
}

// インターネット上のファイルを開く
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::OpenURL
	( const wchar_t * pwszURL,
		const wchar_t * pwszDownloadFile,
		unsigned int nFlags, ECSEnvironment * pEnv )
{
	Close( ) ;
	//
	ESLFileObject *	pDownloadFile = NULL ;
	if ( (pwszDownloadFile != NULL) && (pwszDownloadFile[0] != 0) )
	{
		pDownloadFile =
			pEnv->OpenFileObject
				( EString(pwszDownloadFile), ESLFileObject::modeCreate ) ;
		if ( pDownloadFile == NULL )
		{
			return	eslErrFailed ;
		}
	}
	EString	strURL = pwszURL ;
	if ( !strURL.CompareLeft( "http://" )
			|| !strURL.CompareLeft( "https://" ) )
	{
		pEnv->OpenInternetSession( ) ;
		//
		EString	strHeader = "User-Agent: CotophaScript" ;
		EInternetHttpFile *	phttpf = new EInternetHttpFile ;
		DWORD	dwFlags = INTERNET_FLAG_EXISTING_CONNECT
						| INTERNET_FLAG_TRANSFER_BINARY ;
		if ( nFlags & flagNoCacheURLAccess )
		{
			dwFlags |= INTERNET_FLAG_RELOAD ;
		}
		if ( phttpf->OpenURL
			( *(pEnv->m_pisSession), strURL, pDownloadFile,
				strHeader, strHeader.GetLength(), dwFlags ) )
		{
			delete	phttpf ;
			delete	pDownloadFile ;
			return	eslErrFailed ;
		}
		m_stackFile.Push( phttpf ) ;
		m_pFile = phttpf ;
		m_dwOpenFlags = ESLFileObject::modeRead ;
		return	eslErrSuccess ;
	}
	delete	pDownloadFile ;
	return	eslErrFailed ;
}

// メモリファイルを生成する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::CreateMemoryFile( DWORD dwInitBufSize )
{
	Close( ) ;
	//
	EMemoryFile *	pmemfile = new EMemoryFile ;
	pmemfile->Create( dwInitBufSize ) ;
	m_stackFile.Push( pmemfile ) ;
	m_pFile = pmemfile ;
	m_dwOpenFlags = m_pFile->GetAttribute( ) ;
	//
	return	eslErrSuccess ;
}

// ファイルを閉じる
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Close( void )
{
	while ( m_stackFile.GetSize() > 0 )
	{
		ESLFileObject *	pfile = m_stackFile.Pop( ) ;
		delete	pfile ;
	}
	m_pFile = NULL ;
	m_strFileName.m_varStr.FreeString( ) ;
	m_dwOpenFlags = 0 ;
	return	eslErrSuccess ;
}

// アーカイブを開く
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::OpenArchive( void )
{
	if ( m_pFile == NULL )
	{
		return	eslErrGeneral ;
	}
	ERISAArchive *	parcf = new ERISAArchive ;
	if ( parcf->Open( m_pFile ) )
	{
		delete	parcf ;
		return	eslErrGeneral ;
	}
	m_stackFile.Push( parcf ) ;
	m_pFile = parcf ;
	return	eslErrSuccess ;
}

// アーカイブを閉じる
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::CloseArchive( void )
{
	ERISAArchive *	parcf = ESLTypeCast<ERISAArchive>( m_pFile ) ;
	if ( parcf == NULL )
	{
		return	eslErrGeneral ;
	}
	delete	m_stackFile.Pop() ;
	m_pFile = m_stackFile.GetLastAt( ) ;
	return	eslErrSuccess ;
}

// アーカイブファイルを開く
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::OpenArchiveFile
	( const char * pszFilePath, const char * pszPassword, bool fStream )
{
	ERISAArchive *	parcf = ESLTypeCast<ERISAArchive>( m_pFile ) ;
	if ( parcf == NULL )
	{
		return	eslErrGeneral ;
	}
	ERISAArchive::OpenType	otType = ERISAArchive::otNormal ;
	if ( fStream )
	{
		otType = ERISAArchive::otStream ;
	}
	return	parcf->OpenFile
		( pszFilePath, pszPassword, otType ) ;
}

// アーカイブファイルを閉じる
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::CloseArchiveFile( void )
{
	ERISAArchive *	parcf = ESLTypeCast<ERISAArchive>( m_pFile ) ;
	if ( parcf == NULL )
	{
		return	eslErrGeneral ;
	}
	return	parcf->AscendFile( ) ;
}

// 文字エンコーディングを設定する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::SetCharacterEncoding( const char * pszType )
{
	m_nCharaEncoding = EDescription::GetCharaEncodingType( pszType ) ;
	return	eslErrSuccess ;
}

// 文字エンコーディングを取得する
//////////////////////////////////////////////////////////////////////////////
const char * ECSFile::GetCharacterEncoding( void ) const
{
	return	EDescription::GetCharaEncodingName
				( (EDescription::CharacterEncoding) m_nCharaEncoding ) ;
}

// ファイルのダウンロード状況を取得
//////////////////////////////////////////////////////////////////////////////
INT64 ECSFile::GetCurrentDownloaded( void ) const
{
	ESyncHttpFile *	phttpFile =
		ESLTypeCast<ESyncHttpFile>( m_stackFile.GetAt(0) ) ;
	if ( phttpFile != NULL )
	{
		return	phttpFile->GetCurrentStatus( NULL ) ;
	}
	ESyncStreamFile *	ponlineFile =
		ESLTypeCast<ESyncStreamFile>( m_stackFile.GetAt(0) ) ;
	if ( ponlineFile != NULL )
	{
		return	ponlineFile->GetBufferLength() ;
	}
	return	(INT64) -1 ;
}

// ダウンロードファイルのファイル長を取得
//////////////////////////////////////////////////////////////////////////////
INT64 ECSFile::GetDownloadingFileLength( void ) const
{
	ESyncHttpFile *	phttpFile =
		ESLTypeCast<ESyncHttpFile>( m_stackFile.GetAt(0) ) ;
	if ( phttpFile != NULL )
	{
		DWORD	dwTotal ;
		DWORD	dwSize = phttpFile->GetCurrentStatus( &dwTotal ) ;
		if ( dwTotal != 0 )
		{
			return	dwTotal ;
		}
		return	-1 ;
	}
	EInternetHttpFile *	ponlineFile =
		ESLTypeCast<EInternetHttpFile>( m_stackFile.GetAt(0) ) ;
	if ( ponlineFile != NULL )
	{
		DWORD	dwContentLength ;
		if ( !ponlineFile->QueryContentLength( dwContentLength ) )
		{
			return	dwContentLength ;
		}
		return	-1 ;
	}
	return	(INT64) -1 ;
}

// ファイルのダウンロード完了を取得
//////////////////////////////////////////////////////////////////////////////
bool ECSFile::IsFileDownloaded( void ) const
{
	ESyncHttpFile *	phttpFile =
		ESLTypeCast<ESyncHttpFile>( m_stackFile.GetAt(0) ) ;
	if ( phttpFile != NULL )
	{
		return	phttpFile->HasContentsReceived( ) ;
	}
	EInternetFile *	ponlineFile =
		ESLTypeCast<EInternetFile>( m_stackFile.GetAt(0) ) ;
	if ( ponlineFile != NULL )
	{
		return	(ponlineFile->WaitUntilDownload(0) == eslErrSuccess) ;
	}
	return	true ;
}

// ファイルのダウンロードが失敗しているか？
//////////////////////////////////////////////////////////////////////////////
bool ECSFile::IsFileDownloadFailed( void ) const
{
	ESyncHttpFile *	phttpFile =
		ESLTypeCast<ESyncHttpFile>( m_stackFile.GetAt(0) ) ;
	if ( phttpFile != NULL )
	{
		if ( phttpFile->HasContentsReceived() )
		{
			if ( phttpFile->GetStatusCode() != 200 )
			{
				return	true ;
			}
			DWORD	dwTotal ;
			DWORD	dwSize = phttpFile->GetCurrentStatus( &dwTotal ) ;
			if ( dwTotal != 0 )
			{
				return	(dwSize < dwTotal) ;
			}
		}
		return	false ;
	}
	EInternetHttpFile *	ponlineFile =
		ESLTypeCast<EInternetHttpFile>( m_stackFile.GetAt(0) ) ;
	if ( ponlineFile != NULL )
	{
		if ( ponlineFile->WaitUntilDownload(0) == eslErrSuccess )
		{
			DWORD	dwStatusCode ;
			if ( ponlineFile->QueryStatusCode( dwStatusCode )
				|| (dwStatusCode != 200) )
			{
				return	true ;
			}
			DWORD	dwContentLength ;
			if ( !ponlineFile->QueryContentLength( dwContentLength ) )
			{
				return	(ponlineFile->GetBufferLength() < dwContentLength) ;
			}
		}
	}
	return	false ;
}

// ファイルのダウンロードを中断
//////////////////////////////////////////////////////////////////////////////
void ECSFile::CancelFileDownloading( void ) const
{
	ESyncHttpFile *	phttpFile =
		ESLTypeCast<ESyncHttpFile>( m_stackFile.GetAt(0) ) ;
	if ( phttpFile != NULL )
	{
		phttpFile->CancelBlocking( ) ;
		return ;
	}
	EInternetFile *	ponlineFile =
		ESLTypeCast<EInternetFile>( m_stackFile.GetAt(0) ) ;
	if ( ponlineFile != NULL )
	{
		ponlineFile->CancelDownload() ;
		return ;
	}
}

// ファイルから読み込む
//////////////////////////////////////////////////////////////////////////////
unsigned long int ECSFile::Read
	( void * ptrBuffer, unsigned long int nBytes )
{
	if ( m_pFile != NULL )
	{
		return	m_pFile->Read( ptrBuffer, nBytes ) ;
	}
	return	0 ;
}

// ファイルへ書き出す
//////////////////////////////////////////////////////////////////////////////
unsigned long int ECSFile::Write
	( const void * ptrBuffer, unsigned long int nBytes )
{
	if ( m_pFile != NULL )
	{
		return	m_pFile->Write( ptrBuffer, nBytes ) ;
	}
	return	0 ;
}

// ファイル長取得
//////////////////////////////////////////////////////////////////////////////
UINT64 ECSFile::GetFileLength( void ) const
{
	if ( m_pFile != NULL )
	{
		return	m_pFile->GetLargeLength( ) ;
	}
	return	0 ;
}

// ファイルポインタ取得
//////////////////////////////////////////////////////////////////////////////
UINT64 ECSFile::GetFilePosition( void ) const
{
	if ( m_pFile != NULL )
	{
		return	m_pFile->GetLargePosition( ) ;
	}
	return	0 ;
}

// ファイルポインタ移動
//////////////////////////////////////////////////////////////////////////////
UINT64 ECSFile::Seek( INT64 nPos, int nSeekType )
{
	if ( m_pFile != NULL )
	{
		return	m_pFile->SeekLarge( nPos, (ESLFileObject::SeekOrigin) nSeekType ) ;
	}
	return	0 ;
}

// EOF 判定
//////////////////////////////////////////////////////////////////////////////
bool ECSFile::IsEndOfFile( void ) const
{
	if ( m_pFile != NULL )
	{
		return	(m_pFile->GetPosition() >= m_pFile->GetLength()) ;
	}
	return	true ;
}

// EOF 設定
//////////////////////////////////////////////////////////////////////////////
void ECSFile::SetEndOfFile( void )
{
	if ( (m_pFile != NULL) && (m_dwOpenFlags & ESLFileObject::modeWrite) )
	{
		m_pFile->SetEndOfFile( ) ;
	}
}

// 文字列の読み込み
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::ReadText( ECSObject & obj )
{
	if ( (m_pFile == NULL) || !(m_dwOpenFlags & ESLFileObject::modeRead) )
	{
		return	eslErrGeneral ;
	}
	//
	// テキストを１行読み込む
	//
	EStreamBuffer	bufText ;
	for ( ; ; )
	{
		char *	pcBuf = (char*) bufText.PutBuffer( 0x40 ) ;
		DWORD	dwReadBytes = m_pFile->Read( pcBuf, 0x40 ) ;
		DWORD	i ;
		for ( i = 0; i < dwReadBytes; i ++ )
		{
			if ( pcBuf[i] == '\n' )
				break ;
		}
		if ( i < dwReadBytes )
		{
			bufText.Flush( i ) ;
			m_pFile->Seek
				( i + 1 - (long int) dwReadBytes,
							ESLFileObject::FromCurrent ) ;
			break ;
		}
		bufText.Flush( dwReadBytes ) ;
		if ( dwReadBytes < 0x40 )
		{
			break ;
		}
	}
	//
	// 文字コードを変換する
	//
	EWideString	wstrText ;
	EPtrBuffer		pbufText = bufText.GetBuffer() ;
	if ( m_nCharaEncoding == EDescription::ceUnknown )
	{
		const wchar_t *	pwszBuf = (const wchar_t *) pbufText.GetBuffer( ) ;
		unsigned int	nLength = pbufText.GetLength() / sizeof(wchar_t) ;
		if ( (nLength >= 1) && (pwszBuf[nLength - 1] == L'\r') )
		{
			nLength -- ;
		}
		wstrText = EWideString( pwszBuf, nLength ) ;
	}
	else
	{
		const char *	pszBuf = (const char *) pbufText.GetBuffer( ) ;
		unsigned int	nLength = pbufText.GetLength() / sizeof(char) ;
		if ( (nLength >= 1) && (pszBuf[nLength - 1] == L'\r') )
		{
			nLength -- ;
		}
		EString	strText( pszBuf, nLength ) ;
		EDescription::DecodeText
			( wstrText, strText,
				(EDescription::CharacterEncoding) m_nCharaEncoding ) ;
	}
	//
	// オブジェクト型の変換
	//
	if ( obj.m_vtType == csvtString )
	{
		((ECSString&)obj).m_varStr = wstrText ;
	}
	else if ( obj.m_vtType == csvtInteger )
	{
		EStreamWideString	swsText = wstrText ;
		((ECSInteger&)obj).SetValue( swsText.GetInteger( ) ) ;
	}
	else if ( obj.m_vtType == csvtReal )
	{
		EStreamWideString	swsText = wstrText ;
		((ECSReal&)obj).m_varReal = swsText.GetRealNumber( ) ;
	}
	else
	{
		return	ESLErrorMsg( "不正な型変換です。" ) ;
	}
	return	eslErrSuccess ;
}

// 文字列の書き出し
//////////////////////////////////////////////////////////////////////////////
unsigned long int ECSFile::WriteText( ECSObject & obj )
{
	if ( (m_pFile == NULL) || !(m_dwOpenFlags & ESLFileObject::modeWrite) )
	{
		return	0 ;
	}
	//
	// オブジェクト型の変換
	//
	EWideString	wstrText ;
	if ( obj.m_vtType == csvtString )
	{
		wstrText = ((ECSString&)obj).m_varStr ;
	}
	else if ( obj.m_vtType == csvtInteger )
	{
		wstrText.FromInteger( ((ECSInteger&)obj).GetValue() ) ;
	}
	else if ( obj.m_vtType == csvtReal )
	{
		wstrText.FromReal( ((ECSReal&)obj).m_varReal ) ;
	}
	else if ( obj.m_vtType == csvtPointerReference )
	{
		CSVariableType	csvtRefType =
			((ECSPointerReference*)&obj)->GetMemoryObjectType() ;
		ESLError	err ;
		if ( csvtRefType == csvtReal )
		{
			REAL64		rValue ;
			err = obj.OperateReal( rValue ) ;
			if ( err )
			{
				return	0 ;
			}
			wstrText.FromReal( rValue ) ;
		}
		else
		{
			INT64		nValue ;
			err = obj.OperateInteger( nValue ) ;
			if ( err )
			{
				return	0 ;
			}
			wstrText.FromInteger( nValue ) ;
		}
	}
	else
	{
		return	0 ;
	}
	wstrText += L"\r\n" ;
	//
	// 文字コードを変換して書き出す
	//
	if ( m_nCharaEncoding == EDescription::ceUnknown )
	{
		return	m_pFile->Write
			( wstrText.CharPtr(), wstrText.GetLength() * sizeof(wchar_t) ) ;
	}
	EString	strText ;
	EDescription::EncodeText
		( strText, wstrText,
			(EDescription::CharacterEncoding) m_nCharaEncoding ) ;
	return	m_pFile->Write( strText.CharPtr(), strText.GetLength() ) ;
}

// バイナリの読み込み
//////////////////////////////////////////////////////////////////////////////
unsigned long int ECSFile::ReadBinary( ECSObject & obj, long int nBytes )
{
	if ( (m_pFile == NULL) || !(m_dwOpenFlags & ESLFileObject::modeRead) )
	{
		return	0 ;
	}
	DWORD	dwReadBytes = 0 ;
	if ( obj.m_vtType == csvtPointer )
	{
		//
		// ポインタ
		//
		void *	ptrBuf = obj.GetBuffer( 0, nBytes, true ) ;
		if ( ptrBuf != NULL )
		{
			dwReadBytes = m_pFile->Read( ptrBuf, nBytes ) ;
			obj.FlushBuffer( 0, nBytes, ptrBuf, true ) ;
		}
	}
	else if ( obj.m_vtType == csvtString )
	{
		//
		// 文字列
		//
		ECSString &	strObj = ((ECSString&)obj) ;
		if ( (m_nCharaEncoding == EDescription::ceUnknown)
			|| (m_nCharaEncoding == EDescription::ceUTF16) )
		{
			nBytes = (nBytes + 1) / sizeof(wchar_t) ;
			dwReadBytes = m_pFile->Read
				( strObj.m_varStr.GetBuffer( nBytes ),
								nBytes * sizeof(wchar_t) ) ;
			strObj.m_varStr.ReleaseBuffer( (dwReadBytes + 1) / sizeof(wchar_t) ) ;
		}
		else
		{
			EString		strText ;
			EWideString	wstrText ;
			dwReadBytes = m_pFile->Read( strText.GetBuffer( nBytes ), nBytes ) ;
			strText.ReleaseBuffer( dwReadBytes ) ;
			EDescription::DecodeText
				( wstrText, strText,
					(EDescription::CharacterEncoding) m_nCharaEncoding ) ;
			strObj.m_varStr = wstrText ;
		}
	}
	else if ( obj.m_vtType == csvtInteger )
	{
		//
		// 整数
		//
		if ( nBytes <= 0 )
		{
			nBytes = sizeof(long int) ;
		}
		else if ( nBytes > sizeof(INT64) )
		{
			nBytes = sizeof(INT64) ;
		}
		INT64	nVal = 0 ;
		dwReadBytes = m_pFile->Read( &nVal, nBytes ) ;
		((ECSInteger&)obj).SetValue( nVal ) ;
	}
	else if ( obj.m_vtType == csvtReal )
	{
		//
		// 実数
		//
		if ( nBytes <= 0 )
		{
			nBytes = sizeof(REAL64) ;
		}
		else if ( nBytes < sizeof(REAL32) )
		{
			nBytes = 0 ;
		}
		else if ( nBytes < sizeof(REAL64) )
		{
			nBytes = sizeof(REAL32) ;
		}
		else
		{
			nBytes = sizeof(REAL64) ;
		}
		if ( nBytes == sizeof(REAL32) )
		{
			REAL32	rVal = 0 ;
			dwReadBytes = m_pFile->Read( &rVal, nBytes ) ;
			((ECSReal&)obj).m_varReal = rVal ;
		}
		else if ( nBytes == sizeof(REAL64) )
		{
			dwReadBytes =
				m_pFile->Read( &(((ECSReal&)obj).m_varReal), nBytes ) ;
		}
	}
	else
	{
		ECSFile *	pfile = ESLTypeCast<ECSFile>( &obj ) ;
		if ( (pfile != NULL)
			&& (pfile->m_pFile != NULL)
			&& (pfile->m_dwOpenFlags & ESLFileObject::modeWrite) )
		{
			const DWORD		dwBufSize = 0x10000 ;
			EStreamBuffer	buf ;
			void *	ptrBuf = buf.PutBuffer( dwBufSize ) ;
			while ( dwReadBytes < (DWORD) nBytes )
			{
				DWORD	dwCurBytes = nBytes - dwReadBytes ;
				if ( dwCurBytes > dwBufSize )
				{
					dwCurBytes = dwBufSize ;
				}
				DWORD	dwCurRead = m_pFile->Read( ptrBuf, dwCurBytes ) ;
				DWORD	dwCurWritten =
					pfile->m_pFile->Write( ptrBuf, dwCurRead ) ;
				dwReadBytes += dwCurWritten ;
				if ( (dwCurRead < dwCurBytes) || (dwCurWritten < dwCurBytes) )
				{
					break ;
				}
			}
		}
	}
	return	dwReadBytes ;
}

// バイナリの書き出し
//////////////////////////////////////////////////////////////////////////////
unsigned long int ECSFile::WriteBinary( ECSObject & obj, long int nBytes )
{
	if ( (m_pFile == NULL) || !(m_dwOpenFlags & ESLFileObject::modeWrite) )
	{
		return	0 ;
	}
	if ( obj.m_vtType == csvtPointer )
	{
		//
		// ポインタ
		//
		DWORD	dwWrittenBytes = 0 ;
		void *	ptrBuf = obj.GetBuffer( 0, nBytes, false ) ;
		if ( ptrBuf != NULL )
		{
			dwWrittenBytes = m_pFile->Write( ptrBuf, nBytes ) ;
			obj.FlushBuffer( 0, nBytes, ptrBuf, false ) ;
		}
		return	dwWrittenBytes ;
	}
	else if ( obj.m_vtType == csvtString )
	{
		//
		// 文字列
		//
		ECSString &	strObj = ((ECSString&)obj) ;
		if ( (m_nCharaEncoding == EDescription::ceUnknown)
			|| (m_nCharaEncoding == EDescription::ceUTF16) )
		{
			if ( nBytes <= 0 )
			{
				nBytes = strObj.m_varStr.GetLength() * sizeof(wchar_t) ;
			}
			else if ( (unsigned long int) nBytes
						> strObj.m_varStr.GetLength() * sizeof(wchar_t) )
			{
				nBytes = strObj.m_varStr.GetLength() * sizeof(wchar_t) ;
			}
			return	m_pFile->Write( strObj.m_varStr.CharPtr(), nBytes ) ;
		}
		EString		strText ;
		EWideString	wstrText = strObj.m_varStr ;
		EDescription::EncodeText
			( strText, wstrText,
				(EDescription::CharacterEncoding) m_nCharaEncoding ) ;
		if ( nBytes <= 0 )
		{
			nBytes = strText.GetLength( ) ;
		}
		return	m_pFile->Write( strText.CharPtr(), nBytes ) ;
	}
	else if ( obj.m_vtType == csvtInteger )
	{
		//
		// 整数
		//
		if ( nBytes <= 0 )
		{
			nBytes = sizeof(long int) ;
		}
		else if ( nBytes > sizeof(INT64) )
		{
			nBytes = sizeof(INT64) ;
		}
		INT64	nVal = ((ECSInteger&)obj).GetValue( ) ;
		return	m_pFile->Write( &nVal, nBytes ) ;
	}
	else if ( obj.m_vtType == csvtReal )
	{
		if ( nBytes <= 0 )
		{
			nBytes = sizeof(REAL64) ;
		}
		else if ( nBytes < sizeof(REAL32) )
		{
			nBytes = 0 ;
		}
		else if ( nBytes < sizeof(REAL64) )
		{
			nBytes = sizeof(REAL32) ;
		}
		else
		{
			nBytes = sizeof(REAL64) ;
		}
		if ( nBytes == sizeof(REAL32) )
		{
			REAL32	rVal = (REAL32) ((ECSReal&)obj).m_varReal ;
			return	m_pFile->Write( &rVal, nBytes ) ;
		}
		else if ( nBytes == sizeof(REAL64) )
		{
			return	m_pFile->Write( &(((ECSReal&)obj).m_varReal), nBytes ) ;
		}
	}
	else
	{
		ECSFile *	pfile = ESLTypeCast<ECSFile>( &obj ) ;
		if ( (pfile != NULL)
			&& (pfile->m_pFile != NULL)
			&& (pfile->m_dwOpenFlags & ESLFileObject::modeRead) )
		{
			const DWORD		dwBufSize = 0x10000 ;
			EStreamBuffer	buf ;
			void *	ptrBuf = buf.PutBuffer( dwBufSize ) ;
			DWORD	dwTotalWritten = 0 ;
			while ( dwTotalWritten < (DWORD) nBytes )
			{
				DWORD	dwCurBytes = nBytes - dwTotalWritten ;
				if ( dwCurBytes > dwBufSize )
				{
					dwCurBytes = dwBufSize ;
				}
				DWORD	dwCurRead = pfile->m_pFile->Read( ptrBuf, dwCurBytes ) ;
				DWORD	dwCurWritten = m_pFile->Write( ptrBuf, dwCurRead ) ;
				dwTotalWritten += dwCurWritten ;
				if ( (dwCurRead < dwCurBytes) || (dwCurWritten < dwCurBytes) )
				{
					break ;
				}
			}
			return	dwTotalWritten ;
		}
	}
	return	0 ;
}

// セーブファイルの中のチャンクを開く
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::OpenSaveFile( EMCFile & emcfile, ESLFileObject & file )
{
	//
	// サムネイル画像を読み飛ばす
	//
	BITMAPFILEHEADER	bmfhdr ;
	long int			nFilePos = file.GetPosition( ) ;
	long int			nOrgPos = nFilePos ;
	if ( file.Read( &bmfhdr, sizeof(bmfhdr) ) == sizeof(bmfhdr) )
	{
		if ( bmfhdr.bfType == 'MB' )
		{
			nFilePos += bmfhdr.bfSize ;
		}
	}
	file.Seek( nFilePos, ESLFileObject::FromBegin ) ;
	//
	// EMC ファイルを開く
	//
	if ( emcfile.Open( &file ) )
	{
		file.Seek( nOrgPos, ESLFileObject::FromBegin ) ;
		return	eslErrGeneral ;
	}
	return	eslErrSuccess ;
}

// セーブファイル見出しの読み込み
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::LoadContextTitle( ECSObject *& pObj, ECSContext & context )
{
	if ( (m_pFile == NULL) || !(m_dwOpenFlags & ESLFileObject::modeRead) )
	{
		return	eslErrGeneral ;
	}
	EMCFile		emcfile ;
	long int	nFilePos = m_pFile->GetPosition( ) ;
	if ( OpenSaveFile( emcfile, *m_pFile ) )
	{
		return	eslErrGeneral ;
	}
	if ( emcfile.DescendRecord( (UINT64*) "title   " ) )
	{
		return	ESLErrorMsg
			( "セーブファイルに見出し"
				"レコードが見つかりませんでした。" ) ;
	}
	if ( context.LoadObject( emcfile, pObj ) )
	{
		return	ESLErrorMsg( "見出しの読み込みに失敗しました。" ) ;
	}
	if ( pObj != NULL )
	{
		pObj->CommitAllReference( context ) ;
	}
	emcfile.AscendRecord( ) ;
	emcfile.Close( ) ;
	m_pFile->Seek( nFilePos, ESLFileObject::FromBegin ) ;
	return	eslErrSuccess ;
}

// オブジェクトの読み込み
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::LoadObject( ECSObject *& pObj, ECSContext & context )
{
	if ( (m_pFile == NULL) || !(m_dwOpenFlags & ESLFileObject::modeRead) )
	{
		return	eslErrGeneral ;
	}
	EMCFile		emcfile ;
	long int	nFilePos = m_pFile->GetPosition( ) ;
	if ( OpenSaveFile( emcfile, *m_pFile ) )
	{
		return	eslErrGeneral ;
	}
	if ( emcfile.DescendRecord( (UINT64*) "object  " ) )
	{
		return	ESLErrorMsg
			( "セーブファイルにデータ"
				"レコードが見つかりませんでした。" ) ;
	}
	try
	{
		EMemoryFile			memfile ;
		ERISADecodeContext	decoder( 0x10000 ) ;
		DWORD				dwBytes ;
		emcfile.Read( &dwBytes, sizeof(DWORD) ) ;
		memfile.Create( dwBytes ) ;
		decoder.AttachInputFile( &emcfile ) ;
		decoder.PrepareToDecodeERISANCode( ) ;
		if ( decoder.DecodeERISANCodeBytes
			( (SBYTE*) memfile.GetBuffer(), dwBytes ) < dwBytes )
		{
			return	ESLErrorMsg( "ファイルの展開に失敗しました。" ) ;
		}
		memfile.Seek( dwBytes, memfile.FromBegin ) ;
		memfile.SetEndOfFile( ) ;
		memfile.Seek( 0, memfile.FromBegin ) ;
		//
		if ( context.LoadObject( memfile, pObj ) )
		{
			return	ESLErrorMsg( "オブジェクトの読み込みに失敗しました。" ) ;
		}
	}
	catch ( ... )
	{
		return	ESLErrorMsg( "オブジェクトの読み込みに失敗しました。" ) ;
	}
	if ( pObj != NULL )
	{
		pObj->CommitAllReference( context ) ;
	}
	emcfile.AscendRecord( ) ;
	emcfile.Close( ) ;
	m_pFile->Seek( nFilePos, ESLFileObject::FromBegin ) ;
	return	eslErrSuccess ;
}

// コンテキストの読み込み
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::LoadContext( ECSContext & context, bool fNoCompressed )
{
	if ( (m_pFile == NULL) || !(m_dwOpenFlags & ESLFileObject::modeRead) )
	{
		return	eslErrGeneral ;
	}
	ESLFileObject *	pfile = m_pFile->Duplicate( ) ;
	Close( ) ;
	ESLError		err = LoadContext( *pfile, fNoCompressed, context ) ;
	delete	pfile ;
	return	err ;
}

ESLError ECSFile::LoadContext
	( ESLFileObject & file, bool fNoCompressed, ECSContext & context )
{
	if ( ECotophaScript::GetCurrentThread() != NULL )
	{
		return	eslErrGeneral ;
	}
	EMCFile		emcfile ;
	long int	nFilePos = file.GetPosition( ) ;
	if ( OpenSaveFile( emcfile, file ) )
	{
		return	eslErrGeneral ;
	}
	if ( !fNoCompressed )
	{
		for ( ; ; )
		{
			if ( emcfile.DescendRecord() )
			{
				return	ESLErrorMsg
					( "セーブファイルにコンテキスト"
						"レコードが見つかりませんでした。" ) ;
			}
			UINT64	idRect = emcfile.GetRecordID() ;
			if ( idRect == *((UINT64*) "ccontext") )
			{
				fNoCompressed = true ;
				break ;
			}
			if ( idRect == *((UINT64*) "context ") )
			{
				fNoCompressed = false ;
				break ;
			}
			emcfile.AscendRecord() ;
		}
	}
	else
	{
		if ( emcfile.DescendRecord( (UINT64*) "ccontext" ) )
		{
			return	ESLErrorMsg
				( "セーブファイルにコンテキスト"
					"レコードが見つかりませんでした。" ) ;
		}
	}
	if ( fNoCompressed )
	{
		try
		{
			if ( context.Load( emcfile ) )
			{
				return	ESLErrorMsg( "コンテキストの読み込みに失敗しました。" ) ;
			}
		}
		catch( ... )
		{
			return	ESLErrorMsg( "コンテキストの読み込みに失敗しました。" ) ;
		}
		emcfile.AscendRecord( ) ;
		emcfile.Close( ) ;
		return	eslErrSuccess ;
	}
	try
	{
		EMemoryFile			memfile ;
		ERISADecodeContext	decoder( 0x10000 ) ;
		DWORD				dwBytes ;
		emcfile.Read( &dwBytes, sizeof(DWORD) ) ;
		memfile.Create( dwBytes ) ;
		decoder.AttachInputFile( &emcfile ) ;
		decoder.PrepareToDecodeERISANCode( ) ;
		if ( decoder.DecodeERISANCodeBytes
			( (SBYTE*) memfile.GetBuffer(), dwBytes ) < dwBytes )
		{
			return	ESLErrorMsg( "ファイルの展開に失敗しました。" ) ;
		}
		memfile.Seek( dwBytes, memfile.FromBegin ) ;
		memfile.SetEndOfFile( ) ;
		memfile.Seek( 0, memfile.FromBegin ) ;
		//
		if ( context.Load( memfile ) )
		{
			return	ESLErrorMsg( "コンテキストの読み込みに失敗しました。" ) ;
		}
	}
	catch( ... )
	{
		return	ESLErrorMsg( "コンテキストの読み込みに失敗しました。" ) ;
	}
	emcfile.AscendRecord( ) ;
	emcfile.Close( ) ;
	file.Seek( nFilePos, ESLFileObject::FromBegin ) ;
	return	eslErrSuccess ;
}

// セーブファイルサムネイル画像の書き出し
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::SaveThumbnailImage
	( ECSSprite * pPreview, int nWidth, int nHeight )
{
	if ( (pPreview == NULL) || (m_pFile == NULL)
			|| !(m_dwOpenFlags & ESLFileObject::modeWrite) )
	{
		return	eslErrGeneral ;
	}
	//
	EGL_DRAW_PARAM	dp ;
	EGL_IMAGE_AXES	axes ;
	EGL_SIZE		sizeDisplay = pPreview->GetSize() ;
	EGL_SIZE		sizePreview = { nWidth, nHeight } ;
	if ( (sizeDisplay.w == 0) || (sizeDisplay.h == 0) )
	{
		return	eslErrGeneral ;
	}
	axes.xAxis.y = axes.yAxis.x = 0 ;
	axes.xAxis.x = (REAL32) ((double) (sizePreview.w + 0.5) / sizeDisplay.w) ;
	axes.yAxis.y = (REAL32) ((double) (sizePreview.h + 0.5) / sizeDisplay.h) ;
	if ( (nWidth == 0) || (nHeight == 0) )
	{
		sizePreview = sizeDisplay ;
		axes.xAxis.x = 1 ;
		axes.yAxis.y = 1 ;
	}
	//
	EGLMediaLoader	eglPreview ;
	eglPreview.CreateImage
		( EIF_RGB_BITMAP, sizePreview.w, sizePreview.h, 32, 0 ) ;
//	eglPreview.ReverseVertically( ) ;
	//
	HEGL_DRAW_IMAGE	hDraw = ::eglCreateDrawImage( ) ;
	::eslFillMemory( &dp, 0, sizeof(dp) ) ;
	dp.dwFlags = EGL_SMOOTH_STRETCH ;
	dp.pSrcImage = *pPreview ;
	dp.pImageAxes = &axes ;
	hDraw->Initialize( eglPreview, NULL, NULL ) ;
	if ( dp.pSrcImage != NULL )
	{
		if ( !hDraw->PrepareDraw( &dp ) )
		{
			hDraw->DrawImage( ) ;
		}
	}
	hDraw->Release( ) ;
	//
//	eglPreview.ReverseVertically( ) ;
	return	eglPreview.WriteBitmapFile( *m_pFile ) ;
}

// オブジェクトの書き出し
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::SaveObject
	( ECSObject & obj, ECSObject * pTitle, ECSContext & context )
{
	if ( (m_pFile == NULL) || !(m_dwOpenFlags & ESLFileObject::modeWrite) )
	{
		return	eslErrGeneral ;
	}
	ESLError	err ;
	EMCFile		emcfile ;
	EMCFile::FILE_HEADER	fhdr ;
	EMCFile::SetFileHeader
		( fhdr, EMCFile::fidUndefinedEMC, "cotomi context file" ) ;
	if ( emcfile.Open( m_pFile, &fhdr ) )
	{
		return	ESLErrorMsg( "セーブファイルを開けませんでした。" ) ;
	}
	//
	// 見出し書き出し
	//
	emcfile.DescendRecord( (UINT64*) "title   " ) ;
	if ( pTitle != NULL )
	{
		pTitle->IndexAllMember( ) ;
	}
	err = context.SaveObject( emcfile, pTitle ) ;
	if ( err )
	{
		return	ESLErrorMsg( "見出しデータの書き出しに失敗しました。" ) ;
	}
	emcfile.AscendRecord( ) ;
	//
	// 保存
	//
	EMemoryFile	memfile ;
	memfile.Create( 0x10000 ) ;
	err = context.SaveObject( memfile, &obj ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// オブジェクト書き出し
	//
	DWORD	dwBytes = memfile.GetLength() ;
	ERISAEncodeContext	encoder( 0x10000 ) ;
	emcfile.DescendRecord( (UINT64*) "object  " ) ;
	emcfile.Write( &dwBytes, sizeof(DWORD) ) ;
	encoder.AttachOutputFile( &emcfile ) ;
	encoder.PrepareToEncodeERISANCode( ) ;
	encoder.EncodeERISANCodeBytes
		( (const SBYTE *) memfile.GetBuffer(), dwBytes ) ;
	encoder.FinishERISACode( ) ;
	emcfile.AscendRecord( ) ;
	//
	emcfile.Close( ) ;
	//
	return	eslErrSuccess ;
}

// コンテキストの読み込み
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::SaveContext
	( ECSObject * pTitle, bool fNoCompress, ECSContext & context )
{
	if ( (m_pFile == NULL) || !(m_dwOpenFlags & ESLFileObject::modeWrite) )
	{
		return	eslErrGeneral ;
	}
	if ( ECotophaScript::GetCurrentThread() != NULL )
	{
		return	eslErrGeneral ;
	}
	ESLError	err ;
	EMCFile		emcfile ;
	EMCFile::FILE_HEADER	fhdr ;
	EMCFile::SetFileHeader
		( fhdr, EMCFile::fidUndefinedEMC, "cotomi context file" ) ;
	if ( emcfile.Open( m_pFile, &fhdr ) )
	{
		return	ESLErrorMsg( "セーブファイルを開けませんでした。" ) ;
	}
	//
	// 見出し書き出し
	//
	m_strFileName.m_varStr.FreeString( ) ;
	//
	emcfile.DescendRecord( (UINT64*) "title   " ) ;
	if ( pTitle != NULL )
	{
		pTitle->IndexAllMember( ) ;
	}
	err = context.SaveObject( emcfile, pTitle ) ;
	if ( err )
	{
		return	ESLErrorMsg( "見出しデータの書き出しに失敗しました。" ) ;
	}
	emcfile.AscendRecord( ) ;
	//
	// 保存
	//
	if ( fNoCompress )
	{
		emcfile.DescendRecord( (UINT64*) "ccontext" ) ;
		err = context.Save( emcfile ) ;
		emcfile.AscendRecord( ) ;
		emcfile.Close( ) ;
		return	err ;
	}
	EMemoryFile	memfile ;
	memfile.Create( 0x10000 ) ;
	err = context.Save( memfile ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// オブジェクト書き出し
	//
	DWORD	dwBytes = memfile.GetLength() ;
	if ( dwBytes > 0x40000 )
	{
		emcfile.DescendRecord( (UINT64*) "ccontext" ) ;
		emcfile.Write( memfile.GetBuffer(), dwBytes ) ;
		emcfile.AscendRecord( ) ;
	}
	else
	{
		ERISAEncodeContext	encoder( 0x10000 ) ;
		emcfile.DescendRecord( (UINT64*) "context " ) ;
		emcfile.Write( &dwBytes, sizeof(DWORD) ) ;
		encoder.AttachOutputFile( &emcfile ) ;
		encoder.PrepareToEncodeERISANCode( ) ;
		encoder.EncodeERISANCodeBytes
			( (const SBYTE *) memfile.GetBuffer(), dwBytes ) ;
		encoder.FinishERISACode( ) ;
		emcfile.AscendRecord( ) ;
	}
	emcfile.Close( ) ;
	//
	return	eslErrSuccess ;
}

// オブジェクトをダンプする
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::DumpObject( ECSObject & obj, ECSContext & context )
{
	if ( (m_pFile == NULL) || (context.m_pcsxi == NULL)
			|| !(m_dwOpenFlags & ESLFileObject::modeWrite) )
	{
		return	eslErrGeneral ;
	}
	context.SuspendAllThread( ) ;
	obj.IndexAllMember( ) ;
	//
	EStreamBuffer	bufDump ;
	obj.DumpObject( bufDump, 0, context ) ;
	//
	EPtrBuffer	ptrbuf = bufDump.GetBuffer( ) ;
	m_pFile->Write( ptrbuf, ptrbuf.GetLength() ) ;
	//
	context.ResumeAllThread( ) ;
	//
	return	eslErrSuccess ;
}

// コンテキストをダンプする
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::DumpContext( ECSContext & context )
{
	if ( (m_pFile == NULL) || (context.m_pcsxi == NULL)
			|| !(m_dwOpenFlags & ESLFileObject::modeWrite) )
	{
		return	eslErrGeneral ;
	}
	context.SuspendAllThread( ) ;
	//
	EStreamBuffer	bufDump ;
	context.DumpContext( bufDump ) ;
	//
	EPtrBuffer	ptrbuf = bufDump.GetBuffer( ) ;
	m_pFile->Write( ptrbuf, ptrbuf.GetLength() ) ;
	//
	context.ResumeAllThread( ) ;
	//
	return	eslErrSuccess ;
}

// オブジェクトの型名を取得する
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSFile::GetTypeName( void ) const
{
	return	L"File" ;
}

// オブジェクトを複製
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSFile::Duplicate( void )
{
	ECSFile *	pFile = new ECSFile ;
	if ( m_pFile != NULL )
	{
		pFile->m_pFile = m_pFile->Duplicate() ;
		pFile->m_strFileName.m_varStr = m_strFileName.m_varStr ;
		pFile->m_dwOpenFlags = m_dwOpenFlags ;
		pFile->m_nCharaEncoding = m_nCharaEncoding ;
	}
	return	pFile ;
}

// オブジェクトを代入
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Move( ECSContext & context, ECSObject * obj )
{
	ECSObject *	pEntity = ECSObject::GetEntity( obj ) ;
	if ( pEntity == NULL )
	{
		return	ESLErrorMsg( "代入元のオブジェクトが存在しません。" ) ;
	}
	INT64		nValue ;
	ESLError	err = pEntity->OperateInteger( nValue ) ;
	if ( err )
	{
		return	err ;
	}
	if ( m_pFile != NULL )
	{
		Seek( nValue, ESLFileObject::FromBegin ) ;
	}
	context.delete_CSObject( obj ) ;
	return	eslErrSuccess ;
}

// 単項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::UnaryOperate
	( ECSContext & context, CSUnaryOperatorType csuopType )
{
	if ( csuopType == csuotLogicalNot )
	{
		m_pResult = new ECSInteger( - (long int) IsEndOfFile() ) ;
		return	eslErrSuccess ;
	}
	return	ESLErrorMsg( "File 型の定義されていない単項演算子です。" ) ;
}

// 二項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Operate
	( ECSContext & context, CSOperatorType csopType, ECSObject * obj )
{
	ECSObject *	pEntity = ECSObject::GetEntity( obj ) ;
	if ( pEntity == NULL )
	{
		return	ESLErrorMsg( "File 型への演算オブジェクトが存在しません。" ) ;
	}
	if ( csopType != csotAdd )
	{
		return	ESLErrorMsg( "File 型の定義されていない演算子です。" ) ;
	}
	WriteText( *pEntity ) ;
	context.delete_CSObject( obj ) ;
	return	eslErrSuccess ;
}

// 比較演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Compare
	( ECSContext & context, int & nResult,
		CSCompareType cscpType, ECSObject & obj )
{
	if ( obj.m_vtType != csvtInteger )
	{
		return	ESLErrorMsg( "File 型の定義されていない比較です。" ) ;
	}
	ECSInteger	intPos ;
	intPos.SetValue( GetFilePosition( ) ) ;
	return	intPos.Compare( context, nResult, cscpType, obj ) ;
}

// メンバ関数インデックス取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::GetFunction
	( ECSContext & context, int & nIndex, const wchar_t * pwszName )
{
	nIndex = m_staFuncName->FindIndex( pwszName ) ;
	if ( nIndex < 0 )
	{
		return	ESLErrorMsg( "定義されていない File メンバ関数です。" ) ;
	}
	return	eslErrSuccess ;
}

// メンバ関数呼び出し
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::CallFunction
	( ECSContext & context,
		int nIndex, ECSObjArray<ECSObject> & lstArg )
{
	if ( (unsigned int) nIndex >= m_staFuncName->GetSize() )
	{
		return	ESLErrorMsg
			( "定義されていない File メンバ関数の呼び出しです。" ) ;
	}
	return	(this->*m_pfnCallFunc[nIndex])( context, lstArg ) ;
}

// 特殊演算子 : boolean 判定
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::OperateBoolean( int & nBoolean )
{
	nBoolean = - (int) IsEndOfFile() ;
	return	eslErrSuccess ;
}

// 特殊演算子 : sizeof
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::OperateSizeOf( INT64 & nSize )
{
	nSize = GetFileLength() ;
	return	eslErrSuccess ;
}

// 整数値取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::OperateInteger( INT64 & nValue )
{
	nValue = GetFilePosition() ;
	return	eslErrSuccess ;
}

// 文字列取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::OperateString( EWideString & wstrValue )
{
	ECSString	strText ;
	ReadText( strText ) ;
	wstrValue = strText.m_varStr ;
	return	eslErrSuccess ;
}

// 内部バッファインターフェース
//////////////////////////////////////////////////////////////////////////////
void * ECSFile::GetBuffer( int iOffset, int nSize, bool fWritable )
{
	EMemoryFile *	pmemfile = ESLTypeCast<EMemoryFile>( m_pFile ) ;
	if ( pmemfile != NULL )
	{
		if ( (unsigned int) (iOffset + nSize) <= pmemfile->GetLength() )
		{
			return	((BYTE*)pmemfile->GetBuffer()) + iOffset ;
		}
	}
	return	NULL ;
}

// データを保存
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Save( ESLFileObject & file, ECSContext & context )
{
	ESLError	err = m_strFileName.Save( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	UINT64	nFilePos = GetFilePosition( ) ;
	file.Write( &m_dwOpenFlags, sizeof(m_dwOpenFlags) ) ;
	file.Write( &m_nCharaEncoding, sizeof(m_nCharaEncoding) ) ;
	file.Write( &nFilePos, sizeof(nFilePos) ) ;
	//
	DWORD			dwFlags = 0 ;
	EMemoryFile *	pmemfile = ESLTypeCast<EMemoryFile>( m_pFile ) ;
	if ( pmemfile != NULL )
	{
		dwFlags |= 0x01 ;
	}
	file.Write( &dwFlags, sizeof(DWORD) ) ;
	//
	if ( pmemfile != NULL )
	{
		DWORD	dwLength = pmemfile->GetLength() ;
		file.Write( &dwLength, sizeof(DWORD) ) ;
		file.Write( pmemfile->GetBuffer(), dwLength ) ;
	}
	return	eslErrSuccess ;
}

// データを復元
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Load( ESLFileObject & file, ECSContext & context )
{
	Close( ) ;
	//
	ESLError	err = m_strFileName.Load( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	UINT64		nFilePos ;
	DWORD		dwOpenFlags ;
	int			nCharaEncoding ;
	file.Read( &dwOpenFlags, sizeof(dwOpenFlags) ) ;
	file.Read( &nCharaEncoding, sizeof(nCharaEncoding) ) ;
	file.Read( &nFilePos, sizeof(nFilePos) ) ;
	//
	if ( !m_strFileName.m_varStr.IsEmpty() )
	{
		err = Open
			( m_strFileName.m_varStr,
				(dwOpenFlags & ~ESLFileObject::modeCreateFlag), &context ) ;
		if ( !err )
		{
			m_nCharaEncoding = nCharaEncoding ;
			Seek( nFilePos, ESLFileObject::FromBegin ) ;
		}
	}
	DWORD	dwFlags = 0 ;
	file.Read( &dwFlags, sizeof(DWORD) ) ;
	if ( dwFlags & 0x01 )
	{
		DWORD	dwLength = 0 ;
		file.Read( &dwLength, sizeof(DWORD) ) ;
		CreateMemoryFile( dwLength ) ;
		//
		EStreamBuffer	buf ;
		void *	ptrBuf = buf.PutBuffer( dwLength ) ;
		file.Read( ptrBuf, dwLength ) ;
		//
		Seek( 0, ESLFileObject::FromBegin ) ;
		Write( ptrBuf, dwLength ) ;
		//
		m_nCharaEncoding = nCharaEncoding ;
		Seek( nFilePos, ESLFileObject::FromBegin ) ;
	}
	return	eslErrSuccess ;
}

// データをダンプ
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::DumpObject
	( EStreamBuffer & buf, int nIndent, ECSContext & context )
{
	EString	strDump ;
	if ( !m_strFileName.m_varStr.IsEmpty() )
	{
		strDump = "filename = " + EString(m_strFileName.m_varStr) ;
		buf.Write( strDump.CharPtr(), strDump.GetLength() ) ;
	}
	return	eslErrSuccess ;
}


//////////////////////////////////////////////////////////////////////////////
// メンバ関数
//////////////////////////////////////////////////////////////////////////////

ECSStrTagArray *	ECSFile::m_staFuncName = NULL ;
const wchar_t *	ECSFile::m_pwszFuncName[40] =
{
	L"Open", L"OpenURL", L"CreateMemoryFile", L"Close",
	L"OpenArchive", L"CloseArchive",
	L"OpenArchiveFile", L"CloseArchiveFile",
	L"GetCharacterEncoding", L"SetCharacterEncoding",
	L"IsEndOfFile", L"SetEndOfFile",
	L"GetCurrentDownloaded", L"GetDownloadingFileLength",
	L"IsFileDownloaded", L"IsFileDownloadFailed", L"CancelFileDownloading",
	L"GetLength", L"GetPosition", L"Seek",
	L"ReadText", L"WriteText", L"Read", L"Write", L"GetFileTime",
	L"LoadContextTitle", L"LoadObject", L"LoadContext",
	L"SaveThumbnailImage", L"SaveObject", L"SaveContext",
	L"DumpObject", L"DumpContext",
	L"IsExisting", L"Rename",
	L"FindFile", L"FindDirectory", L"FilterFilePath",
	NULL
} ;
const ECSFile::PFUNC_CALL	ECSFile::m_pfnCallFunc[39] =
{
	&ECSFile::Call_Open,
	&ECSFile::Call_OpenURL,
	&ECSFile::Call_CreateMemoryFile,
	&ECSFile::Call_Close,
	&ECSFile::Call_OpenArchive,	&ECSFile::Call_CloseArchive,
	&ECSFile::Call_OpenArchiveFile,	&ECSFile::Call_CloseArchiveFile,
	&ECSFile::Call_GetCharacterEncoding,
	&ECSFile::Call_SetCharacterEncoding,
	&ECSFile::Call_IsEndOfFile,	&ECSFile::Call_SetEndOfFile,
	&ECSFile::Call_GetCurrentDownloaded,
	&ECSFile::Call_GetDownloadingFileLength,
	&ECSFile::Call_IsFileDownloaded,
	&ECSFile::Call_IsFileDownloadFailed,
	&ECSFile::Call_CancelFileDownloading,
	&ECSFile::Call_GetLength, &ECSFile::Call_GetPosition, &ECSFile::Call_Seek,
	&ECSFile::Call_ReadText, &ECSFile::Call_WriteText,
	&ECSFile::Call_Read, &ECSFile::Call_Write, &ECSFile::Call_GetFileTime,
	&ECSFile::Call_LoadContextTitle, &ECSFile::Call_LoadObject,
	&ECSFile::Call_LoadContext, &ECSFile::Call_SaveThumbnailImage,
	&ECSFile::Call_SaveObject, &ECSFile::Call_SaveContext,
	&ECSFile::Call_DumpObject, &ECSFile::Call_DumpContext,
	&ECSFile::Call_IsExisting, &ECSFile::Call_Rename,
	&ECSFile::Call_FindFile, &ECSFile::Call_FindDirectory,
	&ECSFile::Call_FilterFilePath,
} ;

// メンバ関数 : Integer Open( String sFileName, [Integer nFlags] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_Open
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2, 3 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrFileName ;
	int				nOpenFlags ;
	err = context.GetArgumentAsStr( wstrFileName, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt
		( nOpenFlags, lstArg, 2,
			(ESLFileObject::modeRead | ESLFileObject::shareRead) ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject
		( new ECSInteger( Open( wstrFileName, nOpenFlags, &context ) ) ) ;
}

// メンバ関数 : Integer OpenURL( String strURL, String strDownloadFile := "" )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_OpenURL
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2, 4 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrFileName ;
	ECSWideString	wstrDownloadFile ;
	int				nFlags ;
	err = context.GetArgumentAsStr( wstrFileName, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsStr( wstrDownloadFile, lstArg, 2, NULL ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( nFlags, lstArg, 3, 0 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject
		( new ECSInteger( OpenURL
			( wstrFileName, wstrDownloadFile,
				(unsigned int) nFlags, context.GetEnvironment() ) ) ) ;
}

// メンバ関数 : Integer CreateMemoryFile( [Integer nInitBufSize] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_CreateMemoryFile
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
		return	err ;
	//
	int		nInitBufSize ;
	err = context.GetArgumentAsInt( nInitBufSize, lstArg, 1, 0x4000 ) ;
	if ( err )
		return	err ;
	//
	err = CreateMemoryFile( nInitBufSize ) ;
	//
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Close()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_Close
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	Close( ) ;
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : Integer OpenArchive()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_OpenArchive
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	err = OpenArchive( ) ;
	//
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer CloseArchive()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_CloseArchive
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	err = CloseArchive( ) ;
	//
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer OpenArchiveFile
//		( String sFilepath, String sPassword, Integer fStream )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_OpenArchiveFile
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 4 ) ;
	if ( err )
		return	err ;
	//
	EWideString	wstrFilePath, wstrPassword ;
	int			fStream ;
	err = context.GetArgumentAsStr( wstrFilePath, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsStr( wstrPassword, lstArg, 2, NULL ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( fStream, lstArg, 3, 0 ) ;
	if ( err )
		return	err ;
	//
	err =
		OpenArchiveFile
			( EString( wstrFilePath ),
				EString( wstrPassword ), fStream != 0 ) ;
	//
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer CloseArchiveFile()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_CloseArchiveFile
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	err = CloseArchiveFile( ) ;
	//
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : String GetCharacterEncoding()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_GetCharacterEncoding
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	ECSString *	pstrEncoding = new ECSString ;
	pstrEncoding->m_varStr = GetCharacterEncoding( ) ;
	//
	return	context.PushObject( pstrEncoding ) ;
}

// メンバ関数 : SetCharacterEncoding( String sType )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_SetCharacterEncoding
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrType ;
	err = context.GetArgumentAsStr( wstrType, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	//
	SetCharacterEncoding( EString( wstrType ) ) ;
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : Integer IsEndOfFile()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_IsEndOfFile
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject( new ECSInteger( - (int) IsEndOfFile() ) ) ;
}

// メンバ関数 : Integer GetCurrentDownloaded()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_GetCurrentDownloaded
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject( new ECSInteger( GetCurrentDownloaded() ) ) ;
}

// メンバ関数 : Integer GetDownloadingFileLength()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_GetDownloadingFileLength
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject( new ECSInteger( GetDownloadingFileLength() ) ) ;
}

// メンバ関数 : Boolean IsFileDownloaded()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_IsFileDownloaded
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject
		( new ECSInteger( - (int) IsFileDownloaded() ) ) ;
}

// メンバ関数 : Boolean IsFileDownloadFailed()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_IsFileDownloadFailed
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject
		( new ECSInteger( - (int) IsFileDownloadFailed() ) ) ;
}

// メンバ関数 : CancelFileDownloading()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_CancelFileDownloading
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	CancelFileDownloading( ) ;
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : SetEndOfFile()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_SetEndOfFile
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	SetEndOfFile( ) ;
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : Integer GetLength()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_GetLength
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject( new ECSInteger( GetFileLength() ) ) ;
}

// メンバ関数 : Integer GetPosition()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_GetPosition
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject( new ECSInteger( GetFilePosition() ) ) ;
}

// メンバ関数 : Integer Seek( Integer nPos, [Integer nOrigin] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_Seek
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2, 3 ) ;
	if ( err )
		return	err ;
	//
	INT64	nPos ;
	int		nSeekOrigin ;
	err = context.GetArgumentAsInt64( nPos, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt
		( nSeekOrigin, lstArg, 2, ESLFileObject::FromBegin ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject
		( new ECSInteger( Seek( nPos, nSeekOrigin ) ) ) ;
}

// メンバ関数 : Integer ReadText( object )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_ReadText
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSObject *	pObj = ECSObject::GetEntity( lstArg.GetAt(1) ) ;
	if ( pObj == NULL )
	{
		return	ESLErrorMsg( "読込先オブジェクトが指定されていません。" ) ;
	}
	return	context.PushObject( new ECSInteger( ReadText( *pObj ) ) ) ;
}

// メンバ関数 : Integer WriteText( object )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_WriteText
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSObject *	pObj = ECSObject::GetEntity( lstArg.GetAt(1) ) ;
	if ( pObj == NULL )
	{
		return	ESLErrorMsg( "書き出しオブジェクトが指定されていません。" ) ;
	}
	return	context.PushObject( new ECSInteger( WriteText( *pObj ) ) ) ;
}

// メンバ関数 : Integer Read( object, Integer nBytes := 0 )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_Read
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2, 3 ) ;
	if ( err )
		return	err ;
	//
	ECSObject *	pObj = ECSObject::GetEntity( lstArg.GetAt(1) ) ;
	if ( pObj == NULL )
	{
		return	ESLErrorMsg( "読込先オブジェクトが指定されていません。" ) ;
	}
	int	nBytes ;
	err = context.GetArgumentAsInt( nBytes, lstArg, 2, 0 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject( new ECSInteger( ReadBinary( *pObj, nBytes ) ) ) ;
}

// メンバ関数 : Integer Write( object, Integer nBytes := 0 )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_Write
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2, 3 ) ;
	if ( err )
		return	err ;
	//
	ECSObject *	pObj = ECSObject::GetEntity( lstArg.GetAt(1) ) ;
	if ( pObj == NULL )
	{
		return	ESLErrorMsg( "書き出しオブジェクトが指定されていません。" ) ;
	}
	int	nBytes ;
	err = context.GetArgumentAsInt( nBytes, lstArg, 2, 0 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject( new ECSInteger( WriteBinary( *pObj, nBytes ) ) ) ;
}

// メンバ関数 : Integer GetFileTime
//		( Time & ftCreate, Time & ftLastAccess, Time & ftLastWrite )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_GetFileTime
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1, 4 ) ;
	if ( err )
		return	err ;
	//
	ECSStructureInterface *	pftCreate =
		ESLTypeCast<ECSStructureInterface>
			( context.GetArgumentObjectAs( lstArg, 1, L"Time" ) ) ;
	ECSStructureInterface *	pftLastAccess =
		ESLTypeCast<ECSStructureInterface>
			( context.GetArgumentObjectAs( lstArg, 2, L"Time" ) ) ;
	ECSStructureInterface *	pftLastWrite =
		ESLTypeCast<ECSStructureInterface>
			( context.GetArgumentObjectAs( lstArg, 3, L"Time" ) ) ;
	//
	ERawFile *	pfile = ESLTypeCast<ERawFile>( m_pFile ) ;
	if ( pfile == NULL )
	{
		return	context.PushObject
					( context.new_CSInteger( eslErrGeneral ) ) ;
	}
	SYSTEMTIME	stCreate, stLastAccess, stLastWrite ;
	err = pfile->GetFileTime( &stCreate, &stLastAccess, &stLastWrite ) ;
	if ( err )
	{
		return	context.PushObject( context.new_CSInteger( err ) ) ;
	}
	if ( pftCreate != NULL )
	{
		pftCreate->SetMemberAsInt( L"nYear", stCreate.wYear ) ;
		pftCreate->SetMemberAsInt( L"nMonth", stCreate.wMonth ) ;
		pftCreate->SetMemberAsInt( L"nDay", stCreate.wDay ) ;
		pftCreate->SetMemberAsInt( L"nWeek", stCreate.wDayOfWeek ) ;
		pftCreate->SetMemberAsInt( L"nHour", stCreate.wHour ) ;
		pftCreate->SetMemberAsInt( L"nMinute", stCreate.wMinute ) ;
		pftCreate->SetMemberAsInt( L"nSecond", stCreate.wSecond ) ;
	}
	if ( pftLastAccess != NULL )
	{
		pftLastAccess->SetMemberAsInt( L"nYear", stLastAccess.wYear ) ;
		pftLastAccess->SetMemberAsInt( L"nMonth", stLastAccess.wMonth ) ;
		pftLastAccess->SetMemberAsInt( L"nDay", stLastAccess.wDay ) ;
		pftLastAccess->SetMemberAsInt( L"nWeek", stLastAccess.wDayOfWeek ) ;
		pftLastAccess->SetMemberAsInt( L"nHour", stLastAccess.wHour ) ;
		pftLastAccess->SetMemberAsInt( L"nMinute", stLastAccess.wMinute ) ;
		pftLastAccess->SetMemberAsInt( L"nSecond", stLastAccess.wSecond ) ;
	}
	if ( pftLastWrite != NULL )
	{
		pftLastWrite->SetMemberAsInt( L"nYear", stLastWrite.wYear ) ;
		pftLastWrite->SetMemberAsInt( L"nMonth", stLastWrite.wMonth ) ;
		pftLastWrite->SetMemberAsInt( L"nDay", stLastWrite.wDay ) ;
		pftLastWrite->SetMemberAsInt( L"nWeek", stLastWrite.wDayOfWeek ) ;
		pftLastWrite->SetMemberAsInt( L"nHour", stLastWrite.wHour ) ;
		pftLastWrite->SetMemberAsInt( L"nMinute", stLastWrite.wMinute ) ;
		pftLastWrite->SetMemberAsInt( L"nSecond", stLastWrite.wSecond ) ;
	}
	return	context.PushObject( context.new_CSInteger( err ) ) ;
}

// メンバ関数 : Integer LoadContextTitle( object )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_LoadContextTitle
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSObject *	pObj = NULL ;
	err = LoadContextTitle( pObj, context ) ;
	if ( !err && pObj )
	{
		ECSObject *	pDst = ECSObject::GetEntity( lstArg.GetAt(1) ) ;
		if ( pDst != NULL )
		{
			err = pDst->Move( context, pObj ) ;
			if ( err )
			{
				delete	pObj ;
			}
		}
	}
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer LoadObject( object )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_LoadObject
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSObject *	pObj = NULL ;
	err = LoadObject( pObj, context ) ;
	if ( !err && pObj )
	{
		ECSObject *	pDst = ECSObject::GetEntity( lstArg.GetAt(1) ) ;
		if ( pDst != NULL )
		{
			err = pDst->Move( context, pObj ) ;
			if ( err )
			{
				delete	pObj ;
			}
		}
	}
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer LoadContext()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_LoadContext
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
		return	err ;
	//
	int	fNoCompressed ;
	err = context.GetArgumentAsInt( fNoCompressed, lstArg, 1, 0 ) ;
	if ( err )
		return	err ;
	//
	err = LoadContext( context, (fNoCompressed != 0) ) ;
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer SaveThumbnailImage
//		( Reference sprThumbnail, [Integer nWidth, Integer nHeight] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_SaveThumbnailImage
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2, 4 ) ;
	if ( err )
		return	err ;
	//
	ECSSprite *	pSprite =
//		ESLTypeCast<ECSSprite>( lstArg.GetAt(1)->GetObjectEntity() ) ;
		ESLTypeCast<ECSSprite>
			( context.GetArgumentObjectAs( lstArg, 1, L"Sprite" ) ) ;
	if ( pSprite == NULL )
	{
		return	ESLErrorMsg
			( "引数に Sprite オブジェクトが指定されていません。" ) ;
	}
	int	nWidth, nHeight ;
	err = context.GetArgumentAsInt( nWidth, lstArg, 2, 0 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( nHeight, lstArg, 3, 0 ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject
		( new ECSInteger( SaveThumbnailImage( pSprite, nWidth, nHeight ) ) ) ;
}

// メンバ関数 : Integer SaveObject( object, [title] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_SaveObject
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2, 3 ) ;
	if ( err )
		return	err ;
	//
	ECSObject *	pObj = ECSObject::GetEntity( lstArg.GetAt(1) ) ;
	if ( pObj == NULL )
	{
		return	ESLErrorMsg( "引数にオブジェクトが指定されていません。" ) ;
	}
	err = SaveObject( *pObj, ECSObject::GetEntity( lstArg.GetAt(2) ), context ) ;
	//
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer SaveContext( [title] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_SaveContext
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1, 3 ) ;
	if ( err )
		return	err ;
	//
	int	fNoCompress ;
	err = context.GetArgumentAsInt( fNoCompress, lstArg, 2, 0 ) ;
	if ( err )
		return	err ;
	//
	err = SaveContext
		( ECSObject::GetEntity( lstArg.GetAt(1) ), (fNoCompress != 0), context ) ;
	//
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer DumpObject( object )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_DumpObject
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSObject *	pObj = ECSObject::GetEntity( lstArg.GetAt(1) ) ;
	if ( pObj == NULL )
	{
		return	ESLErrorMsg( "引数にオブジェクトが指定されていません。" ) ;
	}
	err = DumpObject( *pObj, context ) ;
	//
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer DumpContext()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_DumpContext
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	err = DumpContext( context ) ;
	//
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer IsExisting( String sFileName )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_IsExisting
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrFileName ;
	err = context.GetArgumentAsStr( wstrFileName, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	//
	ESLFileObject *	pfile = context.OpenFileOnScript( wstrFileName, 0 ) ;
	ECSInteger *	pResult = new ECSInteger( pfile ? -1 : 0 ) ;
	delete	pfile ;
	return	context.PushObject( pResult ) ;
}

// メンバ関数 : Integer Rename( String sOldFile, String sNewFile )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_Rename
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2, 3 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrOldFile, wstrNewFile ;
	err = context.GetArgumentAsStr( wstrOldFile, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsStr( wstrNewFile, lstArg, 2, NULL ) ;
	if ( err )
		return	err ;
	//
	ECSEnvironment *	pEnv = context.GetEnvironment( ) ;
	ECSInteger *		pResult = new ECSInteger( eslErrGeneral ) ;
	//
	// ファイル名フィルタリング
	//
	if ( (pEnv != NULL) && !(pEnv->m_fAcceptOtherSaveDir) )
	{
		if ( (wstrOldFile.GetAt(0) == L'\\')
				|| (wstrOldFile.Find( L':' ) >= 0) )
		{
			wstrOldFile = ECSWideString( wstrOldFile.GetFileNamePart( ) ) ;
		}
		if ( (wstrNewFile.GetAt(0) == L'\\')
				|| (wstrNewFile.Find( L':' ) >= 0) )
		{
			wstrNewFile = ECSWideString( wstrNewFile.GetFileNamePart( ) ) ;
		}
	}
	if ( pEnv != NULL )
	{
		ECSWideString	wstrSaveDir = pEnv->m_strSaveDir ;
		if ( !wstrOldFile.IsEmpty() )
		{
			wstrOldFile = wstrSaveDir.OffsetFilePath( wstrOldFile ) ;
		}
		if ( !wstrNewFile.IsEmpty() )
		{
			wstrNewFile = wstrSaveDir.OffsetFilePath( wstrNewFile ) ;
		}
	}
	if ( wstrNewFile.IsEmpty() )
	{
		//
		// ファイル削除
		//
		if ( ::DeleteFile( EString( wstrOldFile ) ) )
		{
			pResult->SetValue( eslErrSuccess ) ;
		}
	}
	else
	{
		//
		// ファイル移動・ファイル名変更
		//
		if ( ::MoveFile( EString( wstrOldFile ), EString( wstrNewFile ) ) )
		{
			pResult->SetValue( eslErrSuccess ) ;
		}
	}
	return	context.PushObject( pResult ) ;
}

// メンバ関数 : Error FindFile( String[]& aFiles, String sFileName )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_FindFile
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	return	Call_FindFileDirectory( context, lstArg, false ) ;
}

// メンバ関数 : Error FindDirectory( String[]& aDirs, sDirName )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_FindDirectory
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	return	Call_FindFileDirectory( context, lstArg, true ) ;
}

ESLError ECSFile::Call_FindFileDirectory
	( ECSContext & context,
		ECSObjArray<ECSObject> & lstArg, bool fDirectory )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2, 3 ) ;
	if ( err )
		return	err ;
	//
	ECSArray *	paFiles =
		ESLTypeCast<ECSArray>
			( context.GetArgumentObjectAs( lstArg, 1, L"Array" ) ) ;
	if ( paFiles == NULL )
	{
		return	ESLErrorMsg( "Array オブジェクトが指定されていません。" ) ;
	}
	ECSWideString	wstrFileName ;
	err = context.GetArgumentAsStr( wstrFileName, lstArg, 2, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	//
	ECSEnvironment *	pEnv = context.GetEnvironment( ) ;
	//
	// ファイル名フィルタリング
	//
	if ( (pEnv != NULL) && !(pEnv->m_fAcceptOtherSaveDir) )
	{
		if ( (wstrFileName.GetAt(0) == L'\\')
				|| (wstrFileName.Find( L':' ) >= 0) )
		{
			wstrFileName = ECSWideString( wstrFileName.GetFileNamePart( ) ) ;
		}
	}
	if ( pEnv != NULL )
	{
		ECSWideString	wstrSaveDir = pEnv->m_strSaveDir ;
		wstrFileName = wstrSaveDir.OffsetFilePath( wstrFileName ) ;
	}
	EString	strFileName = wstrFileName ;
	WIN32_FIND_DATA	wfd ;
	HANDLE	hFind = ::FindFirstFile( strFileName, &wfd ) ;
	if ( hFind != INVALID_HANDLE_VALUE )
	{
		do
		{
			if ( fDirectory )
			{
				if ( (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					&& EString::Compare( wfd.cFileName, "." )
					&& EString::Compare( wfd.cFileName, ".." ) )
				{
					paFiles->m_varArray.Add
						( new ECSString( ECSWideString( wfd.cFileName ) ) ) ;
				}
			}
			else
			{
				if ( !(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
				{
					paFiles->m_varArray.Add
						( new ECSString( ECSWideString( wfd.cFileName ) ) ) ;
				}
			}
		}
		while ( ::FindNextFile( hFind, &wfd ) ) ;
		::FindClose( hFind ) ;
	}
	return	context.PushObject( context.new_CSInteger( eslErrSuccess ) ) ;
}

// メンバ関数 : String FilterFilePath( String sFilePath )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSFile::Call_FilterFilePath
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrFilePath ;
	err = context.GetArgumentAsStr( wstrFilePath, lstArg, 1, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	//
	ECSString *	pStrResult = context.new_CSString() ;
	ECSEnvironment *	pEnv = context.GetEnvironment( ) ;
	if ( pEnv != NULL )
	{
		pStrResult->m_varStr = pEnv->FilterFilePath( wstrFilePath ) ;
	}
	else
	{
		pStrResult->m_varStr = wstrFilePath ;
	}
	return	context.PushObject( pStrResult ) ;
}

// プラグインインターフェースを取得する
//////////////////////////////////////////////////////////////////////////////
void * ECSFile::GetObjectInterface( const wchar_t * pwszType )
{
	if ( !EWideString::CompareNoCase
			( pwszType, L"ECS_FILE_INTERFACE" ) )
	{
		if ( m_ppif == NULL )
		{
			m_ppif = (PLUGIN_FILE*)
				::eslHeapAllocate( NULL, sizeof(PLUGIN_FILE), 0 ) ;
			m_ppif->pBackLink = this ;
			m_ppif->pfnOpen = PIC_Open ;
			m_ppif->pfnClose = PIC_Close ;
			m_ppif->pfnSetCharacterEncoding = PIC_SetCharacterEncoding ;
			m_ppif->pfnGetCharacterEncoding = PIC_GetCharacterEncoding ;
			m_ppif->pfnGetFileLength = PIC_GetFileLength ;
			m_ppif->pfnGetFilePosition = PIC_GetFilePosition ;
			m_ppif->pfnSeek = PIC_Seek ;
			m_ppif->pfnIsEndOfFile = PIC_IsEndOfFile ;
			m_ppif->pfnSetEndOfFile = PIC_SetEndOfFile ;
			m_ppif->pfnReadText = PIC_ReadText ;
			m_ppif->pfnWriteText = PIC_WriteText ;
			m_ppif->pfnReadBinary = PIC_ReadBinary ;
			m_ppif->pfnWriteBinary = PIC_WriteBinary ;
			m_ppif->pfnLoadContextTitle = PIC_LoadContextTitle ;
			m_ppif->pfnLoadObject = PIC_LoadObject ;
			m_ppif->pfnLoadContext = PIC_LoadContext ;
			m_ppif->pfnSaveThumbnailImage = PIC_SaveThumbnailImage ;
			m_ppif->pfnSaveObject = PIC_SaveObject ;
			m_ppif->pfnSaveContext = PIC_SaveContext ;
			m_ppif->pfnDumpObject = PIC_DumpObject ;
			m_ppif->pfnDumpContext = PIC_DumpContext ;
			m_ppif->pfnGetFile = PIC_GetFile ;
		}
		return	(ECS_FILE_INTERFACE*) m_ppif ;
	}
	return	ECSObject::GetObjectInterface( pwszType ) ;
}

ESLError __stdcall ECSFile::PIC_Open
	( ECS_FILE_INTERFACE * instance,
		const wchar_t * pwszFileName,
		DWORD dwOpenFlags, ECS_CONTEXT * pContext )
{
	ECSContext *	context = NULL ;
	if ( pContext != NULL )
	{
		context = ECSContext::ContextFromPlugin( pContext ) ;
	}
	return	FileFromPlugin(instance)->
				Open( pwszFileName, dwOpenFlags, context ) ;
}

ESLError __stdcall ECSFile::PIC_Close( ECS_FILE_INTERFACE * instance )
{
	return	FileFromPlugin(instance)->Close( ) ;
}

ESLError __stdcall ECSFile::PIC_SetCharacterEncoding
	( ECS_FILE_INTERFACE * instance, const char * pszType )
{
	return	FileFromPlugin(instance)->SetCharacterEncoding( pszType ) ;
}

const char * __stdcall ECSFile::PIC_GetCharacterEncoding
	( ECS_FILE_INTERFACE * instance )
{
	return	FileFromPlugin(instance)->GetCharacterEncoding( ) ;
}

unsigned long int __stdcall ECSFile::PIC_GetFileLength
	( ECS_FILE_INTERFACE * instance )
{
	return	(unsigned long int) FileFromPlugin(instance)->GetFileLength( ) ;
}

unsigned long int __stdcall ECSFile::PIC_GetFilePosition
	( ECS_FILE_INTERFACE * instance )
{
	return	(unsigned long int) FileFromPlugin(instance)->GetFilePosition( ) ;
}

unsigned long int __stdcall ECSFile::PIC_Seek
	( ECS_FILE_INTERFACE * instance, long int nPos, int nSeekType )
{
	return	(unsigned long int) FileFromPlugin(instance)->Seek( nPos, nSeekType ) ;
}

int __stdcall ECSFile::PIC_IsEndOfFile( ECS_FILE_INTERFACE * instance )
{
	return	FileFromPlugin(instance)->IsEndOfFile( ) ;
}

void __stdcall ECSFile::PIC_SetEndOfFile( ECS_FILE_INTERFACE * instance )
{
	FileFromPlugin(instance)->SetEndOfFile( ) ;
}

ESLError __stdcall ECSFile::PIC_ReadText
	( ECS_FILE_INTERFACE * instance, ECS_OBJECT * pObj )
{
	ECSObject *	obj = ObjectFromPlugin( pObj ) ;
	return	FileFromPlugin(instance)->ReadText( *obj ) ;
}

unsigned long int __stdcall ECSFile::PIC_WriteText
	( ECS_FILE_INTERFACE * instance, ECS_OBJECT * pObj )
{
	ECSObject *	obj = ObjectFromPlugin( pObj ) ;
	return	FileFromPlugin(instance)->WriteText( *obj ) ;
}

unsigned long int __stdcall ECSFile::PIC_ReadBinary
	( ECS_FILE_INTERFACE * instance, ECS_OBJECT * pObj, long int nBytes )
{
	ECSObject *	obj = ObjectFromPlugin( pObj ) ;
	return	FileFromPlugin(instance)->ReadBinary( *obj, nBytes ) ;
}

unsigned long int __stdcall ECSFile::PIC_WriteBinary
	( ECS_FILE_INTERFACE * instance, ECS_OBJECT * pObj, long int nBytes )
{
	ECSObject *	obj = ObjectFromPlugin( pObj ) ;
	return	FileFromPlugin(instance)->WriteBinary( *obj, nBytes ) ;
}

ESLError __stdcall ECSFile::PIC_LoadContextTitle
	( ECS_FILE_INTERFACE * instance,
		ECS_OBJECT **pObj, ECS_CONTEXT * pContext )
{
	ECSContext *	context = ECSContext::ContextFromPlugin( pContext ) ;
	ECSObject *		obj ;
	ESLError		err =
		FileFromPlugin(instance)->LoadContextTitle( obj, *context ) ;
	if ( !err )
	{
		*pObj = obj->CreateInterface( ) ;
	}
	return	err ;
}

ESLError __stdcall ECSFile::PIC_LoadObject
	( ECS_FILE_INTERFACE * instance,
		ECS_OBJECT **pObj, ECS_CONTEXT * pContext )
{
	ECSContext *	context = ECSContext::ContextFromPlugin( pContext ) ;
	ECSObject *		obj ;
	ESLError		err =
		FileFromPlugin(instance)->LoadObject( obj, *context ) ;
	if ( !err )
	{
		*pObj = obj->CreateInterface( ) ;
	}
	return	err ;
}

ESLError __stdcall ECSFile::PIC_LoadContext
	( ECS_FILE_INTERFACE * instance, ECS_CONTEXT * pContext )
{
	ECSContext *	context = ECSContext::ContextFromPlugin( pContext ) ;
	return	FileFromPlugin(instance)->LoadContext( *context, true ) ;
}

ESLError __stdcall ECSFile::PIC_SaveThumbnailImage
	( ECS_FILE_INTERFACE * instance,
		ECS_OBJECT * pPreview, int nWidth, int nHeight )
{
	ECSSprite *	pPreviewSprite =
		ESLTypeCast<ECSSprite>( ObjectFromPlugin( pPreview ) ) ;
	return	FileFromPlugin(instance)->
				SaveThumbnailImage( pPreviewSprite, nWidth, nHeight ) ;
}

ESLError __stdcall ECSFile::PIC_SaveObject
	( ECS_FILE_INTERFACE * instance,
		ECS_OBJECT * pObj, ECS_OBJECT * pTitle, ECS_CONTEXT * pContext )
{
	ECSContext *	context = ECSContext::ContextFromPlugin( pContext ) ;
	ECSObject *		obj = ObjectFromPlugin( pObj ) ;
	ECSObject *		pTitleObj = NULL ;
	if ( pTitle != NULL )
	{
		pTitleObj = ObjectFromPlugin( pTitle ) ;
	}
	return	FileFromPlugin(instance)->
				SaveObject( *obj, pTitleObj, *context ) ;
}

ESLError __stdcall ECSFile::PIC_SaveContext
	( ECS_FILE_INTERFACE * instance,
		ECS_OBJECT * pTitle, ECS_CONTEXT * pContext )
{
	ECSContext *	context = ECSContext::ContextFromPlugin( pContext ) ;
	ECSObject *		pTitleObj = NULL ;
	if ( pTitle != NULL )
	{
		pTitleObj = ObjectFromPlugin( pTitle ) ;
	}
	return	FileFromPlugin(instance)->SaveContext( pTitleObj, true, *context ) ;
}

ESLError __stdcall ECSFile::PIC_DumpObject
	( ECS_FILE_INTERFACE * instance,
		ECS_OBJECT * pObj, ECS_CONTEXT * pContext )
{
	ECSContext *	context = ECSContext::ContextFromPlugin( pContext ) ;
	ECSObject *	obj = ObjectFromPlugin( pObj ) ;
	return	FileFromPlugin(instance)->DumpObject( *obj, *context ) ;
}

ESLError __stdcall ECSFile::PIC_DumpContext
	( ECS_FILE_INTERFACE * instance, ECS_CONTEXT * pContext )
{
	ECSContext *	context = ECSContext::ContextFromPlugin( pContext ) ;
	return	FileFromPlugin(instance)->DumpContext( *context ) ;
}

ECS_FILE * __stdcall ECSFile::PIC_GetFile( ECS_FILE_INTERFACE * instance )
{
	ECSFile *	pcsf = FileFromPlugin(instance) ;
	if ( pcsf->m_pFile == NULL )
	{
		return	NULL ;
	}
	return	new ECSFilePIInterface( pcsf->m_pFile ) ;
}
