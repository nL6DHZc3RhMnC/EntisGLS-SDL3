
/*****************************************************************************
                   Entis Standard Library implementations
 ----------------------------------------------------------------------------

	In this file, the file object classes source codes.

	Copyright (C) 1998-2011 Leshade Entis.  All rights reserved.

 ****************************************************************************/


// Include esl.h
//////////////////////////////////////////////////////////////////////////////

#include	<windows.h>
#include	<eritypes.h>
#include	<esl.h>


/*****************************************************************************
                           ファイル抽象クラス
 ****************************************************************************/

// クラス情報をインプリメント
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ESLFileObject, ESLObject )
	IMPLEMENT_CLASS_INFO( ERawFile, ESLFileObject )
	IMPLEMENT_CLASS_INFO( EMemoryFile, ESLFileObject )
	IMPLEMENT_CLASS_INFO( EFileMappedBuffer, ESLFileObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ESLFileObject::ESLFileObject( void )
	: m_nAttribute( 0 ), m_pOpener( NULL )
{
	::InitializeCriticalSection( &m_cs ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ESLFileObject::~ESLFileObject( void )
{
	::DeleteCriticalSection( &m_cs ) ;
}

// スレッド排他アクセス
//////////////////////////////////////////////////////////////////////////////
void ESLFileObject::Lock( void ) const
{
	::EnterCriticalSection( (CRITICAL_SECTION*) &m_cs ) ;
}

void ESLFileObject::Unlock( void ) const
{
	::LeaveCriticalSection( (CRITICAL_SECTION*) &m_cs ) ;
}

// ファイルの終端を現在の位置に設定する
//////////////////////////////////////////////////////////////////////////////
ESLError ESLFileObject::SetEndOfFile( void )
{
	return	eslErrGeneral ;
}

// ファイルの長さを取得
//////////////////////////////////////////////////////////////////////////////
UINT64 ESLFileObject::GetLargeLength( void ) const
{
	return	GetLength( ) ;
}

// ファイルポインタを移動
//////////////////////////////////////////////////////////////////////////////
UINT64 ESLFileObject::SeekLarge
	( INT64 nOffsetPos, ESLFileObject::SeekOrigin fSeekFrom )
{
	return	Seek( (long int) nOffsetPos, fSeekFrom ) ;
}

// ファイルポインタを取得
//////////////////////////////////////////////////////////////////////////////
UINT64 ESLFileObject::GetLargePosition( void ) const
{
	return	GetPosition( ) ;
}

// 新規にファイルを開く
//////////////////////////////////////////////////////////////////////////////
ESLFileObject * ESLFileObject::OpenFileObject
	( const wchar_t * pwszFileName, int nOpenFlags )
{
	if ( m_pOpener != NULL )
	{
		return	m_pOpener->OpenFileObject( pwszFileName, nOpenFlags ) ;
	}
	return	NULL ;
}



/*****************************************************************************
                           生ファイルクラス
 ****************************************************************************/

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ERawFile::ERawFile( void )
	: m_hFile( INVALID_HANDLE_VALUE ), m_pszFileTitle( NULL )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ERawFile::~ERawFile( void )
{
	Close( ) ;
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
ESLError ERawFile::Open( const char * pszFileName, int nOpenFlags )
{
	//
	// 現在のファイルを閉じる
	//
	Close( ) ;
	//
	// ファイルオープンフラグを変換
	//
	DWORD	dwAccess = 0, dwShareMode = 0, dwCreate = OPEN_EXISTING ;
	if ( nOpenFlags & modeCreateFlag )	dwCreate = CREATE_ALWAYS ;
	if ( nOpenFlags & modeRead )		dwAccess |= GENERIC_READ ;
	if ( nOpenFlags & modeWrite )		dwAccess |= GENERIC_WRITE ;
	if ( nOpenFlags & shareRead )		dwShareMode |= FILE_SHARE_READ ;
	if ( nOpenFlags & shareWrite )		dwShareMode |= FILE_SHARE_WRITE ;
	//
	// ファイルを開く
	//
	if ( (pszFileName == NULL) || (pszFileName[0] == '\0') )
	{
		return	eslErrGeneral ;
	}
	if ( pszFileName[1] == ':' )
	{
		char	szDrvRoot[] = "A:\\" ;
		szDrvRoot[0] = pszFileName[0] ;
		UINT	nDrvType = ::GetDriveType( szDrvRoot ) ;
		if ( (nDrvType == DRIVE_UNKNOWN)
				|| (nDrvType == DRIVE_NO_ROOT_DIR) )
		{
			return	eslErrGeneral ;
		}
	}
	//
	UINT	nErrorMode = ::SetErrorMode( SEM_FAILCRITICALERRORS ) ;
	//
	m_hFile = ::CreateFile
		( pszFileName, dwAccess, dwShareMode,
			NULL, dwCreate, FILE_ATTRIBUTE_NORMAL, NULL ) ;
	//
	::SetErrorMode( nErrorMode ) ;
	//
	if ( m_hFile != INVALID_HANDLE_VALUE )
	{
		//
		// ファイル情報を設定
		//
		m_nAttribute = nOpenFlags ;
		DWORD	dwNeededLength =
			::GetFullPathName( pszFileName, 0x100,
				m_strFilePath.GetBuffer( 0x100 ), (char**) &m_pszFileTitle ) ;
		//
		if( dwNeededLength >= 0x100 )
		{
			::GetFullPathName
				( pszFileName, dwNeededLength,
					m_strFilePath.GetBuffer( dwNeededLength + 1 ),
					(char**) &m_pszFileTitle ) ;
		}
		m_strFilePath.ReleaseBuffer( ) ;
		//
		return	eslErrSuccess ;
	}
	else
	{
		//
		// ファイルのオープンに失敗
		//
		m_nAttribute = 0 ;
		return	eslErrGeneral ;
	}
}

// ファイルハンドルを複製してファイルオブジェクトに関連付ける
//////////////////////////////////////////////////////////////////////////////
ESLError ERawFile::Create( HANDLE hFile, int nOpenFlags )
{
	//
	// 現在のファイルを閉じる
	//
	Close( ) ;
	//
	// オープンフラグを変換する
	//
	DWORD	dwAccess = 0 ;
	if ( nOpenFlags & modeRead )	dwAccess |= GENERIC_READ ;
	if ( nOpenFlags & modeWrite )	dwAccess |= GENERIC_WRITE ;
	//
	// ファイルハンドルを複製する
	//
	m_pszFileTitle = m_strFilePath = EString( ) ;
	//
	if ( ::DuplicateHandle
		( ::GetCurrentProcess(), hFile,
			::GetCurrentProcess(), &m_hFile, dwAccess, TRUE, 0 ) )
	{
		//
		// ファイル情報を設定する
		//
		m_nAttribute = nOpenFlags ;
		return	eslErrSuccess ;
	}
	else
	{
		//
		// ファイルの複製に失敗
		//
		m_hFile = INVALID_HANDLE_VALUE ;
		m_nAttribute = 0 ;
		return	eslErrGeneral ;
	}
}

// ファイルを閉じる
//////////////////////////////////////////////////////////////////////////////
void ERawFile::Close( void )
{
	Lock( ) ;
	if( m_hFile != INVALID_HANDLE_VALUE )
	{
		::CloseHandle( m_hFile ) ;
		m_hFile = INVALID_HANDLE_VALUE ;
	}
	Unlock( ) ;
}

// ファイルオブジェクトを複製する
//////////////////////////////////////////////////////////////////////////////
ESLFileObject * ERawFile::Duplicate( void ) const
{
	ESLAssert( m_hFile != INVALID_HANDLE_VALUE ) ;
	Lock( ) ;
	ERawFile *	pRawFile = new ERawFile ;
	pRawFile->m_strFilePath = m_strFilePath ;
	pRawFile->m_pszFileTitle =
		((const char*)pRawFile->m_strFilePath)
			+ (DWORD)((const char*)m_strFilePath - (DWORD)m_pszFileTitle) ;
	pRawFile->m_hFile = INVALID_HANDLE_VALUE ;
	if ( (m_nAttribute & modeRead) && !m_strFilePath.IsEmpty() )
	{
		DWORD	dwAccess = 0, dwShareMode = 0, dwCreate = OPEN_EXISTING ;
		if ( m_nAttribute & modeRead )		dwAccess |= GENERIC_READ ;
		if ( m_nAttribute & modeWrite )		dwAccess |= GENERIC_WRITE ;
		if ( m_nAttribute & shareRead )		dwShareMode |= FILE_SHARE_READ ;
		if ( m_nAttribute & shareWrite )	dwShareMode |= FILE_SHARE_WRITE ;
		//
		pRawFile->m_hFile = ::CreateFile
			( m_strFilePath, dwAccess, dwShareMode,
				NULL, dwCreate, FILE_ATTRIBUTE_NORMAL, NULL ) ;
		pRawFile->m_nAttribute = m_nAttribute ;
		//
		if ( pRawFile->m_hFile == INVALID_HANDLE_VALUE )
		{
			::DuplicateHandle
				( ::GetCurrentProcess(), m_hFile,
					::GetCurrentProcess(), &(pRawFile->m_hFile),
					0, TRUE, DUPLICATE_SAME_ACCESS );
		}
	}
	else
	{
		::DuplicateHandle
			( ::GetCurrentProcess(), m_hFile,
				::GetCurrentProcess(), &(pRawFile->m_hFile),
				0, TRUE, DUPLICATE_SAME_ACCESS );
		pRawFile->m_nAttribute = m_nAttribute ;
	}
	Unlock( ) ;
	return	pRawFile ;
}

// ファイルからデータを読み込む
//////////////////////////////////////////////////////////////////////////////
unsigned long int ERawFile::Read
	( void * ptrBuffer, unsigned long int nBytes )
{
	ESLAssert( m_hFile != INVALID_HANDLE_VALUE ) ;
	ESLAssert( m_nAttribute & modeRead ) ;
	Lock( ) ;
	unsigned long int	nReadBytes = 0 ;
	::ReadFile( m_hFile, ptrBuffer, nBytes, &nReadBytes, NULL ) ;
	Unlock( ) ;
	return	nReadBytes ;
}

// ファイルへデータを書き出す
//////////////////////////////////////////////////////////////////////////////
unsigned long int ERawFile::Write
	( const void * ptrBuffer, unsigned long int nBytes )
{
	ESLAssert( m_hFile != INVALID_HANDLE_VALUE ) ;
	ESLAssert( m_nAttribute & modeWrite ) ;
	Lock( ) ;
	unsigned long int	nWrittenBytes = 0 ;
	::WriteFile( m_hFile, ptrBuffer, nBytes, &nWrittenBytes, NULL ) ;
	Unlock( ) ;
	return	nWrittenBytes ;
}

// ファイルの長さを取得する
//////////////////////////////////////////////////////////////////////////////
unsigned long int ERawFile::GetLength( void ) const
{
	return	(unsigned long int) ERawFile::GetLargeLength( ) ;
}

// ファイルポインタを移動する
//////////////////////////////////////////////////////////////////////////////
unsigned long int ERawFile::Seek
	( long int nOffsetPos, SeekOrigin fSeekFrom )
{
	return	(unsigned long int)
		ERawFile::SeekLarge( nOffsetPos, fSeekFrom ) ;
}

// ファイルポインタを取得する
//////////////////////////////////////////////////////////////////////////////
unsigned long int ERawFile::GetPosition( void ) const
{
	return	(unsigned long int) ERawFile::GetLargePosition( ) ;
}

// ファイルの終端を現在の位置に設定する
//////////////////////////////////////////////////////////////////////////////
ESLError ERawFile::SetEndOfFile( void )
{
	ESLAssert( GetAttribute() & modeWrite ) ;
	ESLAssert( m_hFile != INVALID_HANDLE_VALUE ) ;
	if ( ::SetEndOfFile( m_hFile ) )
	{
		return	eslErrSuccess ;
	}
	return	eslErrGeneral ;
}

// ファイルの長さを取得
//////////////////////////////////////////////////////////////////////////////
UINT64 ERawFile::GetLargeLength( void ) const
{
	ESLAssert( m_hFile != INVALID_HANDLE_VALUE ) ;
	SetLastError( NO_ERROR ) ;
	Lock( ) ;
	DWORD	dwHigh = 0 ;
	DWORD	dwSize = ::GetFileSize( m_hFile, &dwHigh ) ;
	Unlock( ) ;
	if ( (dwSize == (DWORD) -1) && (GetLastError() != NO_ERROR) )
	{
		dwSize = 0 ;
	}
	return	(((UINT64) dwHigh) << 32) | dwSize ;
}

// ファイルポインタを移動
//////////////////////////////////////////////////////////////////////////////
UINT64 ERawFile::SeekLarge
	( INT64 nOffsetPos, SeekOrigin fSeekFrom )
{
	ESLAssert( m_hFile != INVALID_HANDLE_VALUE ) ;
	LONG	nHigh = (LONG) (nOffsetPos >> 32) ;
	DWORD	dwNewPos ;
	SetLastError( NO_ERROR ) ;
	Lock( ) ;
	dwNewPos = ::SetFilePointer
		( m_hFile, (DWORD) nOffsetPos, &nHigh, fSeekFrom ) ;
	Unlock( ) ;
	if ( (dwNewPos == INVALID_SET_FILE_POINTER)
					&& (GetLastError() != NO_ERROR) )
	{
		nHigh = 0 ;
		dwNewPos = 0 ;
	}
	return	(((UINT64) nHigh) << 32) | dwNewPos ;
}

// ファイルポインタを取得
//////////////////////////////////////////////////////////////////////////////
UINT64 ERawFile::GetLargePosition( void ) const
{
	ESLAssert( m_hFile != INVALID_HANDLE_VALUE ) ;
	LONG	nHigh = 0 ;
	DWORD	dwNewPos ;
	SetLastError( NO_ERROR ) ;
	Lock( ) ;
	dwNewPos = ::SetFilePointer( m_hFile, 0, &nHigh, FromCurrent ) ;
	Unlock( ) ;
	if ( (dwNewPos == INVALID_SET_FILE_POINTER)
					&& GetLastError() != NO_ERROR )
	{
		nHigh = 0 ;
		dwNewPos = 0 ;
	}
	return	(((UINT64) nHigh) << 32) | dwNewPos ;
}


// 新規にファイルを開く
//////////////////////////////////////////////////////////////////////////////
ESLFileObject * ERawFile::OpenFileObject
	( const wchar_t * pwszFileName, int nOpenFlags )
{
	EWideString	wstrFilePath =
		EWideString(m_strFilePath.GetFileDirectoryPart()).
								OffsetFilePath( pwszFileName ) ;
	ERawFile *	pNewFile = new ERawFile ;
	if ( pNewFile->Open( EString(wstrFilePath), nOpenFlags ) )
	{
		delete	pNewFile ;
		return	NULL ;
	}
	return	pNewFile ;
}


// ファイルのタイムスタンプを取得する
//////////////////////////////////////////////////////////////////////////////
ESLError ERawFile::GetFileTime
	( LPSYSTEMTIME lpCreationTime,
		LPSYSTEMTIME lpLastAccessTime, LPSYSTEMTIME lpLastWriteTime )
{
	if( m_hFile == INVALID_HANDLE_VALUE )
	{
		return	eslErrGeneral ;
	}
	//
	// ファイルタイムを取得
	//
	FILETIME	ftCreation, ftLastAccess, ftLastWrite ;
	if( !::GetFileTime
		( m_hFile, &ftCreation, &ftLastAccess, &ftLastWrite ) )
	{
		return	eslErrGeneral ;
	}
	//
	// ファイルタイムフォーマットを変換する
	//
	FILETIME	ftLocalTime ;
	if( lpCreationTime )
	{
		::FileTimeToLocalFileTime( &ftCreation, &ftLocalTime ) ;
		::FileTimeToSystemTime( &ftLocalTime, lpCreationTime ) ;
	}
	if( lpLastAccessTime )
	{
		::FileTimeToLocalFileTime( &ftLastAccess, &ftLocalTime ) ;
		::FileTimeToSystemTime( &ftLocalTime, lpLastAccessTime ) ;
	}
	if( lpLastWriteTime )
	{
		::FileTimeToLocalFileTime( &ftLastWrite, &ftLocalTime ) ;
		::FileTimeToSystemTime( &ftLocalTime, lpLastWriteTime ) ;
	}
	return	eslErrSuccess ;
}

// ファイルのタイムスタンプを設定する
//////////////////////////////////////////////////////////////////////////////
ESLError ERawFile::SetFileTime
( const SYSTEMTIME * lpCreationTime,
	const SYSTEMTIME * lpLastAccessTime, const SYSTEMTIME * lpLastWriteTime )
{
	if( m_hFile == INVALID_HANDLE_VALUE )
	{
		return	eslErrGeneral ;
	}
	//
	// ファイルタイムフォーマットを変換する
	//
	FILETIME	ftCreation, ftLastAccess, ftLastWrite ;
	FILETIME	ftLocalTime ;
	LPFILETIME	pftCreation = NULL, pftLastAccess = NULL, pftLastWrite = NULL ;
	if ( lpCreationTime != NULL )
	{
		::SystemTimeToFileTime( lpCreationTime, &ftLocalTime ) ;
		::LocalFileTimeToFileTime( &ftLocalTime, &ftCreation ) ;
		pftCreation = &ftCreation ;
	}
	if ( lpLastAccessTime != NULL )
	{
		::SystemTimeToFileTime( lpLastAccessTime, &ftLocalTime ) ;
		::LocalFileTimeToFileTime( &ftLocalTime, &ftLastAccess ) ;
		pftLastAccess = &ftLastAccess ;
	}
	if ( lpLastWriteTime != NULL )
	{
		::SystemTimeToFileTime( lpLastWriteTime, &ftLocalTime ) ;
		::LocalFileTimeToFileTime( &ftLocalTime, &ftLastWrite ) ;
		pftLastWrite = &ftLastWrite ;
	}
	//
	// ファイルタイムを設定
	//
	if ( !::SetFileTime
		( m_hFile, pftCreation, pftLastAccess, pftLastWrite ) )
	{
		return	eslErrGeneral ;
	}
	return	eslErrSuccess ;
}


/*****************************************************************************
                           メモリファイルクラス
 ****************************************************************************/

// 構築関数
//////////////////////////////////////////////////////////////////////////////
EMemoryFile::EMemoryFile( void )
	: m_ptrMemory( NULL )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
EMemoryFile::~EMemoryFile( void )
{
	Delete( ) ;
}

// 読み書き可能なメモリファイルを作成する
//////////////////////////////////////////////////////////////////////////////
ESLError EMemoryFile::Create( unsigned long int nLength )
{
	//
	// 現在のメモリファイルを解放する
	//
	Delete( ) ;
	//
	// メモリを確保する
	//
	Lock( ) ;
	m_nLength = 0 ;
	m_nPosition = 0 ;
	m_nBufferSize = nLength ;
	m_ptrMemory = ::eslHeapAllocate( NULL, m_nBufferSize, 0 ) ;
	m_nAttribute = (modeRead | modeCreate) ;
	Unlock( ) ;

	return	eslErrSuccess ;
}

// 読み込み専用のメモリファイルオブジェクトを作成する
//////////////////////////////////////////////////////////////////////////////
ESLError EMemoryFile::Open
	( const void * ptrMemory, unsigned long int nLength )
{
	//
	// 現在のメモリファイルを解放する
	//
	Delete( ) ;
	//
	// メモリを関連付ける
	//
	ESLAssert( ptrMemory != NULL ) ;
	Lock( ) ;
	m_nBufferSize = m_nLength = nLength ;
	m_nPosition = 0 ;
	m_ptrMemory = (void*) ptrMemory ;
	m_nAttribute = modeRead ;
	Unlock( ) ;

	return	eslErrSuccess ;
}

// メモリファイルを解放する
//////////////////////////////////////////////////////////////////////////////
void EMemoryFile::Delete( void )
{
	Lock( ) ;
	if( m_ptrMemory != NULL )
	{
		if( m_nAttribute & modeWrite )
		{
			::eslHeapFree( NULL, m_ptrMemory ) ;
		}
		m_ptrMemory = NULL ;
		m_nAttribute = 0 ;
	}
	Unlock( ) ;
}

// メモリファイルを複製する
//////////////////////////////////////////////////////////////////////////////
ESLFileObject * EMemoryFile::Duplicate( void ) const
{
	ESLAssert( m_ptrMemory != NULL ) ;
	Lock( ) ;
	EMemoryFile *	pMemFile = new EMemoryFile ;
	pMemFile->Create( m_nLength ) ;
	pMemFile->Write( m_ptrMemory, m_nLength ) ;
	pMemFile->Seek( 0, FromBegin ) ;
	Unlock( ) ;
	return	pMemFile ;
}

// メモリファイルからデータを転送する
//////////////////////////////////////////////////////////////////////////////
unsigned long int EMemoryFile::Read
	( void * ptrBuffer, unsigned long int nBytes )
{
	ESLAssert( m_ptrMemory != NULL ) ;
	ESLAssert( m_nAttribute & modeRead ) ;
	Lock( ) ;
	unsigned long int	nReadBytes = nBytes ;
	if ( (nReadBytes + m_nPosition) > m_nLength )
	{
		nReadBytes = m_nLength - m_nPosition ;
	}
	::eslMoveMemory
		( ptrBuffer, (((BYTE*)m_ptrMemory) + m_nPosition), nReadBytes ) ;
	m_nPosition += nReadBytes ;
	Unlock( ) ;
	return	nReadBytes ;
}

// メモリファイルにデータを転送する
//////////////////////////////////////////////////////////////////////////////
unsigned long int EMemoryFile::Write
	( const void * ptrBuffer, unsigned long int nBytes )
{
	ESLAssert( m_ptrMemory != NULL ) ;
	ESLAssert( m_nAttribute & modeWrite ) ;
	Lock( ) ;
	unsigned long int	nWrittenBytes = nBytes ;
	if ( nWrittenBytes + m_nPosition > m_nLength )
	{
		m_nLength = nWrittenBytes + m_nPosition ;
		if ( m_nLength > m_nBufferSize )
		{
			m_nBufferSize =
				(m_nLength + (m_nBufferSize >> 1)
						+ nWrittenBytes + 0xFFF) & (~0xFFF) ;
			m_ptrMemory = ::eslHeapReallocate
				( NULL, m_ptrMemory, m_nBufferSize, 0 ) ;
		}
	}
	::eslMoveMemory
		( (((BYTE*)m_ptrMemory) + m_nPosition), ptrBuffer, nWrittenBytes ) ;
	m_nPosition += nWrittenBytes ;
	Unlock( ) ;
	return	nWrittenBytes ;
}

// メモリファイルの長さを取得する
//////////////////////////////////////////////////////////////////////////////
unsigned long int EMemoryFile::GetLength( void ) const
{
	ESLAssert( m_ptrMemory != NULL ) ;
	return	m_nLength ;
}

// メモリファイルのポインタを移動する
//////////////////////////////////////////////////////////////////////////////
unsigned long int EMemoryFile::Seek
	( long int nOffsetPos, SeekOrigin fSeekFrom )
{
	ESLAssert( m_ptrMemory != NULL ) ;
	Lock( ) ;
	switch ( fSeekFrom )
	{
	case	FromBegin:
		m_nPosition = nOffsetPos ;
		break ;
	case	FromCurrent:
		m_nPosition += nOffsetPos ;
		break ;
	case	FromEnd:
		m_nPosition = m_nLength + nOffsetPos ;
		break ;
	default:
		ESLAssert( false ) ;
		break ;
	}
	if ( (signed long int) m_nPosition < 0 )
	{
		m_nPosition = 0 ;
	}
	else if ( m_nPosition > m_nLength )
	{
		if ( m_nAttribute & modeWrite )
		{
			m_nLength = m_nPosition ;
		}
	}
	Unlock( ) ;
	return	m_nPosition ;
}

// メモリファイルポインタを取得する
//////////////////////////////////////////////////////////////////////////////
unsigned long int EMemoryFile::GetPosition( void ) const
{
	ESLAssert( m_ptrMemory != NULL ) ;
	return	m_nPosition ;
}

// ファイルの終端を現在の位置に設定する
//////////////////////////////////////////////////////////////////////////////
ESLError EMemoryFile::SetEndOfFile( void )
{
	ESLAssert( m_ptrMemory != NULL ) ;
	ESLAssert( m_nAttribute & modeWrite ) ;
	m_nLength = m_nPosition ;
	return	eslErrSuccess ;
}


#if	!defined(COMPACT_NOA_DECODER)

/*****************************************************************************
                 動的メモリマップファイルバッファクラス
 ****************************************************************************/

// 構築関数
//////////////////////////////////////////////////////////////////////////////
EFileMappedBuffer::EFileMappedBuffer( void )
{
	m_nLoadedPages = 0 ;
	m_nCacheSize = 0 ;
	m_nAccessCounter = 0 ;
	m_pfile = NULL ;
	m_nPointer = 0 ;
	m_nLength = 0 ;
	m_mode = modeNothing ;
	m_nRefAddr = 0 ;
	m_nRefLength = 0 ;
	m_pbytTempBuf = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
EFileMappedBuffer::~EFileMappedBuffer( void )
{
}

// ファイルオブジェクトを複製する
//////////////////////////////////////////////////////////////////////////////
ESLFileObject * EFileMappedBuffer::Duplicate( void ) const
{
	EFileMappedBuffer *	pfmb = new EFileMappedBuffer ;
	pfmb->OpenBuffer( m_pfile ) ;
	return	pfmb ;
}

// ファイルから読み込む
//////////////////////////////////////////////////////////////////////////////
unsigned long int EFileMappedBuffer::Read
	( void * ptrBuffer, unsigned long int nBytes )
{
	unsigned long int	nReadBytes = 0 ;
	while ( (nBytes > 0) && (m_nLength > m_nPointer) )
	{
		UINT64	nLeftBytes = m_nLength - m_nPointer ;
		UINT	nPageIndex = (UINT) (m_nPointer >> BUFFER_SCALE) ;
		UINT	nPageOffset = (UINT) (m_nPointer & BUFFER_MASK) ;
		if ( nLeftBytes > (BUFFER_SIZE - nPageOffset) )
		{
			nLeftBytes = (BUFFER_SIZE - nPageOffset) ;
		}
		if ( nLeftBytes > nBytes )
		{
			nLeftBytes = nBytes ;
		}
		BUFFER *	pbuf = LoadPageBuffer( nPageIndex ) ;
		if ( (pbuf == NULL) || (pbuf->pbytBuf == NULL) )
		{
			break ;
		}
		//
		eslMoveMemory
			( ptrBuffer,
				pbuf->pbytBuf + (ULONG_PTR) nPageOffset,
									(ULONG_PTR) nLeftBytes ) ;
		//
		nBytes -= (ULONG_PTR) nLeftBytes ;
		nReadBytes += (ULONG_PTR) nLeftBytes ;
		m_nPointer += nLeftBytes ;
	}
	return	nReadBytes ;
}

// ファイルへ書き出す
//////////////////////////////////////////////////////////////////////////////
unsigned long int EFileMappedBuffer::Write
	( const void * ptrBuffer, unsigned long int nBytes )
{
	unsigned long int	nWrittenBytes = 0 ;
	while ( nBytes > 0 )
	{
		UINT64	nLeftBytes = nBytes ;
		UINT	nPageIndex = (UINT) (m_nPointer >> BUFFER_SCALE) ;
		UINT	nPageOffset = (UINT) (m_nPointer & BUFFER_MASK) ;
		if ( nLeftBytes > (BUFFER_SIZE - nPageOffset) )
		{
			nLeftBytes = (BUFFER_SIZE - nPageOffset) ;
		}
		BUFFER *	pbuf = LoadPageBuffer( nPageIndex ) ;
		if ( (pbuf == NULL) || (pbuf->pbytBuf == NULL) )
		{
			break ;
		}
		pbuf->fModified = true ;
		//
		eslMoveMemory
			( pbuf->pbytBuf + (ULONG_PTR) nPageOffset,
							ptrBuffer, (ULONG_PTR) nLeftBytes ) ;
		//
		nBytes -= (ULONG_PTR) nLeftBytes ;
		nWrittenBytes += (ULONG_PTR) nLeftBytes ;
		m_nPointer += nLeftBytes ;
		//
		if ( m_nPointer > m_nLength )
		{
			m_nLength = m_nPointer ;
		}
	}
	return	nWrittenBytes ;
}

// ファイルの長さを取得
//////////////////////////////////////////////////////////////////////////////
unsigned long int EFileMappedBuffer::GetLength( void ) const
{
	return	(unsigned long int) m_nLength ;
}

// ファイルポインタを移動
//////////////////////////////////////////////////////////////////////////////
unsigned long int EFileMappedBuffer::Seek
	( long int nOffsetPos, SeekOrigin fSeekFrom )
{
	return	(unsigned long int) SeekLarge( nOffsetPos, fSeekFrom ) ;
}

// ファイルポインタを取得
//////////////////////////////////////////////////////////////////////////////
unsigned long int EFileMappedBuffer::GetPosition( void ) const
{
	return	(unsigned long int) m_nPointer ;
}

// ファイルの終端を現在の位置に設定する
//////////////////////////////////////////////////////////////////////////////
ESLError EFileMappedBuffer::SetEndOfFile( void )
{
	m_nLength = m_nPointer ;
	//
	unsigned int	nPageIndex =
		(unsigned int) (m_nPointer >> BUFFER_SCALE) + 1 ;
	if ( m_lstBuffer.GetSize() > nPageIndex )
	{
		m_lstBuffer.RemoveBetween
			( nPageIndex, m_lstBuffer.GetSize() - nPageIndex ) ;
	}
	//
	if ( m_pfile != NULL )
	{
		m_pfile->SeekLarge( m_nPointer, FromBegin ) ;
		return	m_pfile->SetEndOfFile( ) ;
	}
	return	eslErrSuccess ;
}

// ファイルの長さを取得
//////////////////////////////////////////////////////////////////////////////
UINT64 EFileMappedBuffer::GetLargeLength( void ) const
{
	return	m_nLength ;
}

// ファイルポインタを移動
//////////////////////////////////////////////////////////////////////////////
UINT64 EFileMappedBuffer::SeekLarge
	( INT64 nOffsetPos, SeekOrigin fSeekFrom )
{
	switch ( fSeekFrom )
	{
	case	FromBegin:
		m_nPointer = nOffsetPos ;
		break ;
	case	FromCurrent:
		m_nPointer += nOffsetPos ;
		break ;
	case	FromEnd:
		m_nPointer = m_nLength + nOffsetPos ;
		break ;
	}
	if ( (INT64) m_nPointer < 0 )
	{
		m_nPointer = 0 ;
	}
	return	m_nPointer ;
}

// ファイルポインタを取得
//////////////////////////////////////////////////////////////////////////////
UINT64 EFileMappedBuffer::GetLargePosition( void ) const
{
	return	m_nPointer ;
}

// バッファ新規作成
//////////////////////////////////////////////////////////////////////////////
ESLError EFileMappedBuffer::OpenBuffer
	( ESLFileObject * pfile, ULONG nCacheSize )
{
	CloseBuffer( ) ;
	//
	m_pfile = pfile ;
	m_nCacheSize = (nCacheSize + BUFFER_MASK) >> BUFFER_SCALE ;
	//
	if ( pfile != NULL )
	{
		m_nLength = pfile->GetLargeLength() ;
	}
	//
	return	eslErrSuccess ;
}

// バッファを開放する
//////////////////////////////////////////////////////////////////////////////
void EFileMappedBuffer::CloseBuffer( void )
{
	m_lstBuffer.RemoveAll( ) ;
	//
	m_nLoadedPages = 0 ;
	m_nAccessCounter = 0 ;
	//
	m_pfile = NULL ;
	m_nPointer = 0 ;
	m_nLength = 0 ;
	//
	ReleaseBuffer() ;
	m_bufTemp.Delete( ) ;
}

// 読み出し用バッファを参照する
//////////////////////////////////////////////////////////////////////////////
const BYTE * EFileMappedBuffer::GetBuffer( UINT64 nAddr, ULONG & nBytes )
{
	unsigned int	nPageIndex = (unsigned int) (nAddr >> BUFFER_SCALE) ;
	unsigned int	nPageOffset = (unsigned int) (nAddr & BUFFER_MASK) ;
	if ( nAddr + nBytes > m_nLength )
	{
		if ( m_nLength > nAddr )
		{
			nBytes = (ULONG) (m_nLength - nAddr) ;
		}
		else
		{
			nBytes = 0 ;
		}
	}
	if ( BUFFER_SIZE - nPageOffset >= nBytes )
	{
		//
		// ページを跨がないバッファ参照
		//
		BUFFER *	pbufPage = LoadPageBuffer( nPageIndex ) ;
		if ( pbufPage == NULL )
		{
			nBytes = 0 ;
			return	NULL ;
		}
		m_mode = modeReadDirect ;
		m_nRefAddr = nAddr ;
		m_nRefLength = nBytes ;
		return	pbufPage->pbytBuf + nPageOffset ;
	}
	//
	// ページを跨ったバッファ参照
	//
	ReleaseBuffer( ) ;
	//
	m_mode = modeRead ;
	m_nRefAddr = nAddr ;
	//
	BYTE *	pbytBuf =
		m_pbytTempBuf = (BYTE*) m_bufTemp.PutBuffer( nBytes ) ;
	ULONG	i = 0 ;
	while ( i < nBytes )
	{
		UINT64	nCurrentAddr = nAddr + i ;
		ULONG	nCurrentBytes ;
		nPageIndex = (unsigned int) (nCurrentAddr >> BUFFER_SCALE) ;
		nPageOffset = (unsigned int) (nCurrentAddr & BUFFER_MASK) ;
		nCurrentBytes = BUFFER_SIZE - nPageOffset ;
		if ( nCurrentAddr + nCurrentBytes > m_nLength )
		{
			nCurrentBytes = (ULONG) (m_nLength - nCurrentAddr) ;
		}
		if ( i + nCurrentBytes > nBytes )
		{
			nCurrentBytes = nBytes - i ;
		}
		//
		BUFFER *	pbufPage = LoadPageBuffer( nPageIndex ) ;
		if ( pbufPage == NULL )
		{
			break ;
		}
		eslMoveMemory
			( pbytBuf + i,
				pbufPage->pbytBuf + nPageOffset, nCurrentBytes ) ;
		//
		i += nCurrentBytes ;
	}
	nBytes = m_nRefLength = i ;
	return	pbytBuf ;
}

// 読み出し用バッファを開放する
//////////////////////////////////////////////////////////////////////////////
void EFileMappedBuffer::ReleaseBuffer( void )
{
	if ( m_mode == modeRead )
	{
		m_bufTemp.Flush( 0 ) ;
	}
	else if ( m_mode == modeWrite )
	{
		m_bufTemp.Flush( 0 ) ;
	}
	m_mode = modeNothing ;
	m_nRefAddr = 0 ;
	m_nRefLength = 0 ;
	m_pbytTempBuf = NULL ;
}

// 書き出し用バッファを参照する
//////////////////////////////////////////////////////////////////////////////
BYTE * EFileMappedBuffer::PutBuffer( UINT64 nAddr, ULONG nBytes )
{
	unsigned int	nPageIndex = (unsigned int) (nAddr >> BUFFER_SCALE) ;
	unsigned int	nPageOffset = (unsigned int) (nAddr & BUFFER_MASK) ;
	if ( BUFFER_SIZE - nPageOffset >= nBytes )
	{
		//
		// ページを跨がないバッファ参照
		//
		BUFFER *	pbufPage = LoadPageBuffer( nPageIndex ) ;
		if ( pbufPage == NULL )
		{
			nBytes = 0 ;
			return	NULL ;
		}
		pbufPage->fModified = true ;
		m_mode = modeWriteDirect ;
		m_nRefAddr = nAddr ;
		m_nRefLength = nBytes ;
		return	pbufPage->pbytBuf + nPageOffset ;
	}
	//
	// ページを跨ったバッファ参照
	//
	ReleaseBuffer( ) ;
	//
	m_mode = modeWrite ;
	m_nRefAddr = nAddr ;
	m_nRefLength = nBytes ;
	//
	BYTE *	pbytBuf =
		m_pbytTempBuf = (BYTE*) m_bufTemp.PutBuffer( nBytes ) ;
	return	pbytBuf ;
}

// 書き出し用バッファを開放する
//////////////////////////////////////////////////////////////////////////////
void EFileMappedBuffer::FlushBuffer( ULONG nBytes )
{
	if ( m_mode == modeRead )
	{
		m_bufTemp.Flush( 0 ) ;
	}
	else if ( m_mode == modeWriteDirect )
	{
		if ( m_nRefAddr + nBytes > m_nLength )
		{
			m_nLength = m_nRefAddr + nBytes ;
		}
	}
	else if ( m_mode == modeWrite )
	{
		BYTE *	pbytBuf = m_pbytTempBuf ;
		ULONG	i = 0 ;
		if ( nBytes > m_nRefLength )
		{
			nBytes = m_nRefLength ;
		}
		while ( i < nBytes )
		{
			UINT64	nCurrentAddr = m_nRefAddr + i ;
			ULONG	nCurrentBytes ;
			UINT	nPageIndex = (UINT) (nCurrentAddr >> BUFFER_SCALE) ;
			UINT	nPageOffset = (UINT) (nCurrentAddr & BUFFER_MASK) ;
			nCurrentBytes = BUFFER_SIZE - nPageOffset ;
			if ( nCurrentAddr + nCurrentBytes > m_nLength )
			{
				nCurrentBytes = (ULONG) (m_nLength - nCurrentAddr) ;
			}
			if ( i + nCurrentBytes > nBytes )
			{
				nCurrentBytes = nBytes - i ;
			}
			//
			if ( nCurrentAddr + nCurrentBytes > m_nLength )
			{
				m_nLength = nCurrentAddr + nCurrentBytes ;
			}
			//
			BUFFER *	pbufPage = LoadPageBuffer( nPageIndex ) ;
			if ( pbufPage != NULL )
			{
				pbufPage->fModified = true ;
				//
				eslMoveMemory
					( pbufPage->pbytBuf + nPageOffset,
							pbytBuf + i, nCurrentBytes ) ;
			}
			i += nCurrentBytes ;
		}
		//
		m_bufTemp.Flush( 0 ) ;
	}
	m_mode = modeNothing ;
	m_nRefAddr = 0 ;
	m_nRefLength = 0 ;
	m_pbytTempBuf = NULL ;
}

// ブロックをロードする
//////////////////////////////////////////////////////////////////////////////
EFileMappedBuffer::BUFFER *
	EFileMappedBuffer::LoadPageBuffer( unsigned int nPageIndex )
{
	BUFFER *	pbufPage = m_lstBuffer.GetAt( nPageIndex ) ;
	if ( pbufPage == NULL )
	{
		pbufPage = new BUFFER ;
		m_lstBuffer.SetAt( nPageIndex, pbufPage ) ;
	}
	pbufPage->nLastAccess = m_nAccessCounter ++ ;
	//
	if ( !pbufPage->fLoaded )
	{
		if ( m_pfile != NULL )
		{
			if ( (m_nCacheSize != 0) && (m_nLoadedPages >= m_nCacheSize) )
			{
				UnloadOldPageCache( ) ;
			}
			while ( pbufPage->Allocate() == NULL )
			{
				UnloadOldPageCache( ) ;
				::Sleep( 1 ) ;
			}
			UINT64	nAddr = ((UINT64)nPageIndex) << BUFFER_SCALE ;
			if ( m_pfile->SeekLarge
				( nAddr, ESLFileObject::FromBegin ) == nAddr )
			{
				m_pfile->Read( pbufPage->pbytBuf, BUFFER_SIZE ) ;
			}
		}
		else
		{
			pbufPage->Allocate() ;
		}
		pbufPage->fLoaded = true ;
		m_nLoadedPages ++ ;
	}
	return	pbufPage ;
}

// ロードしているページキャッシュを解放する
//////////////////////////////////////////////////////////////////////////////
void EFileMappedBuffer::UnloadOldPageCache( void )
{
	if ( m_pfile == NULL )
	{
		return ;
	}
	unsigned int	nOldAccess = 0 ;
	int				iOldPage = -1 ;
	BUFFER *		pOldPage = NULL ;
	bool			fOldPageModified = true ;
	int	i, nCount = m_lstBuffer.GetSize() ;
	for ( i = 0; i < nCount; i ++ )
	{
		BUFFER *	pbufPage = m_lstBuffer.GetAt( i ) ;
		if ( pbufPage != NULL )
		{
			if ( pbufPage->fLoaded
				&& (!(pbufPage->fModified) || fOldPageModified) )
			{
				unsigned int	nAccess =
					m_nAccessCounter - pbufPage->nLastAccess ;
				if ( (nAccess >= nOldAccess)
						|| (!(pbufPage->fModified) && fOldPageModified) )
				{
					nOldAccess = nAccess ;
					iOldPage = i ;
					pOldPage = pbufPage ;
					fOldPageModified = pbufPage->fModified ;
				}
			}
		}
	}
	if ( pOldPage != NULL )
	{
		if ( pOldPage->fLoaded )
		{
			if ( pOldPage->fModified )
			{
				UINT64	nAddr = ((INT64)iOldPage) << BUFFER_SCALE ;
				if ( nAddr < m_nLength )
				{
					if ( m_pfile->SeekLarge
						( nAddr, ESLFileObject::FromBegin ) == nAddr )
					{
						UINT64	nBytes = m_nLength - nAddr ;
						if ( nBytes > BUFFER_SIZE )
						{
							nBytes = BUFFER_SIZE ;
						}
						m_pfile->Write
							( pOldPage->pbytBuf,
									(unsigned long int) nBytes ) ;
					}
				}
				pOldPage->fModified = false ;
			}
			ESLAssert( m_nLoadedPages > 0 ) ;
			pOldPage->Release( ) ;
			pOldPage->fLoaded = false ;
			m_nLoadedPages -- ;
		}
	}
}



/*****************************************************************************
                 ストリーミングバッファファイルクラス
 ****************************************************************************/

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO2( EStreamFileBuffer, ESLFileObject, EStreamBuffer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
EStreamFileBuffer::EStreamFileBuffer( void )
{
	SetAttribute( modeReadWrite ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
EStreamFileBuffer::~EStreamFileBuffer( void )
{
}

// ファイルオブジェクトを複製する
//////////////////////////////////////////////////////////////////////////////
ESLFileObject * EStreamFileBuffer::Duplicate( void ) const
{
	return	new EStreamFileBuffer ;
}

// ファイルから読み込む
//////////////////////////////////////////////////////////////////////////////
unsigned long int EStreamFileBuffer::Read
	( void * ptrBuffer, unsigned long int nBytes )
{
	return	EStreamBuffer::Read( ptrBuffer, nBytes ) ;
}

// ファイルへ書き出す
//////////////////////////////////////////////////////////////////////////////
unsigned long int EStreamFileBuffer::Write
	( const void * ptrBuffer, unsigned long int nBytes )
{
	return	EStreamBuffer::Write( ptrBuffer, nBytes ) ;
}

// ファイルの長さを取得
//////////////////////////////////////////////////////////////////////////////
unsigned long int EStreamFileBuffer::GetLength( void ) const
{
	return	EStreamBuffer::GetLength( ) ;
}

// ファイルポインタを移動
//////////////////////////////////////////////////////////////////////////////
unsigned long int EStreamFileBuffer::Seek
	( long int nOffsetPos, SeekOrigin fSeekFrom )
{
	if ( fSeekFrom == FromEnd )
	{
		nOffsetPos += EStreamBuffer::GetLength( ) ;
	}
	EStreamBuffer::GetBuffer( nOffsetPos ) ;
	EStreamBuffer::Release( nOffsetPos ) ;
	return	0 ;
}

// ファイルポインタを取得
//////////////////////////////////////////////////////////////////////////////
unsigned long int EStreamFileBuffer::GetPosition( void ) const
{
	return	0 ;
}


/*****************************************************************************
                 同期ストリーミングバッファファイルクラス
 ****************************************************************************/

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ESyncStreamFile, ESLFileObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ESyncStreamFile::ESyncStreamFile( void )
{
	m_pbLastRead = NULL ;
	m_pTempFile = NULL ;
	m_hFinished = ::CreateEvent( NULL, TRUE, FALSE, NULL ) ;
	m_hWritten = ::CreateEvent( NULL, TRUE, FALSE, NULL ) ;
	m_nLength = -1 ;
	m_nPosition = 0 ;
	m_nBufLength = 0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ESyncStreamFile::~ESyncStreamFile( void )
{
	::CloseHandle( m_hFinished ) ;
	::CloseHandle( m_hWritten ) ;
}

// 内容を初期化
//////////////////////////////////////////////////////////////////////////////
void ESyncStreamFile::Initialize
	( unsigned long int nLength, ESLFileObject * pTempFile )
{
	Lock( ) ;
	::ResetEvent( m_hFinished ) ;
	::ResetEvent( m_hWritten ) ;
	m_pbLastRead = NULL ;
	m_pTempFile = pTempFile ;
	m_tblBuffer.RemoveAll( ) ;
	m_nAttribute = (modeRead | modeCreate) ;
	m_nLength = nLength ;
	m_nPosition = 0 ;
	m_nBufLength = 0 ;
	Unlock( ) ;
}

// バッファへの書き込みを完了
//////////////////////////////////////////////////////////////////////////////
void ESyncStreamFile::FinishStream( void )
{
	Lock( ) ;
	::SetEvent( m_hFinished ) ;
	m_nLength = m_nBufLength ;
	m_nAttribute = modeRead ;
	Unlock( ) ;
}

// ファイルオブジェクトを複製する
//////////////////////////////////////////////////////////////////////////////
ESLFileObject * ESyncStreamFile::Duplicate( void ) const
{
	Lock( ) ;
	EMemoryFile *	pfile = new EMemoryFile ;
	pfile->Create( m_nBufLength ) ;
	//
	for ( unsigned int i = 0; i < m_tblBuffer.GetSize(); i ++ )
	{
		EBuffer *	pBuf = m_tblBuffer.GetAt( i ) ;
		ESLAssert( pBuf != NULL ) ;
		pBuf->Lock( ) ;
		pfile->Write( pBuf->pbytBuf, pBuf->dwBytes ) ;
		pBuf->Unlock( ) ;
	}
	//
	pfile->Seek( m_nPosition, FromBegin ) ;
	Unlock( ) ;
	return	pfile ;
}

// ファイルから読み込む
//////////////////////////////////////////////////////////////////////////////
unsigned long int ESyncStreamFile::Read
	( void * ptrBuffer, unsigned long int nBytes )
{
	return	ReadTimeout( ptrBuffer, nBytes ) ;
}

unsigned long int ESyncStreamFile::ReadTimeout
	( void * ptrBuffer, unsigned long int nBytes,
					bool fAsync, DWORD dwTimeout )
{
	DWORD	dwResult ;
	HANDLE	hEvents[2] =
	{
		m_hWritten, m_hFinished
	} ;
	unsigned long int	nReadBytes = 0 ;
	BYTE *	pbytBuffer = (BYTE*) ptrBuffer ;
	DWORD	dwBeginTime = ::GetCurrentTime( ) ;
	Lock( ) ;
	while ( nBytes != 0 )
	{
		//
		// バッファリングされている有効なバイト数を取得
		//
		unsigned long int	nLeftBytes ;
		for ( ; ; )
		{
			nLeftBytes = m_nBufLength - m_nPosition ;
			if ( nLeftBytes > 0 )
			{
				break ;
			}
			//
			// バッファリング同期
			//
			if ( fAsync )
			{
				break ;
			}
			DWORD	dwCurrentTimeout = INFINITE ;
			if ( dwTimeout != INFINITE )
			{
				DWORD	dwCurrentTime = ::GetCurrentTime( ) ;
				if ( dwCurrentTime - dwBeginTime >= dwTimeout )
				{
					break ;
				}
				dwCurrentTimeout = dwTimeout - (dwCurrentTime - dwBeginTime) ;
			}
			Unlock( ) ;
			dwResult =
				::WaitForMultipleObjects
					( 2, hEvents, FALSE, dwCurrentTimeout ) ;
			Lock( ) ;
			if ( dwResult == WAIT_TIMEOUT )
			{
				break ;
			}
			if ( dwResult == (WAIT_OBJECT_0 + 1) )
			{
				break ;
			}
			::ResetEvent( m_hWritten ) ;
		}
		//
		// バッファからデータを読み込む
		//
		if ( nLeftBytes == 0 )
		{
			break ;
		}
		else if ( nLeftBytes > nBytes )
		{
			nLeftBytes = nBytes ;
		}
		unsigned int	iBuf = m_nPosition >> BUFFER_SCALE ;
		unsigned int	iOffset = m_nPosition & BUFFER_MASK ;
		EBuffer *	pBuf = m_tblBuffer.GetAt( iBuf ) ;
		ESLAssert( pBuf != NULL ) ;
		if ( m_pbLastRead != pBuf )
		{
			if ( m_pbLastRead != NULL )
			{
				m_pbLastRead->Unlock( ) ;
			}
			pBuf->Lock( ) ;
			m_pbLastRead = pBuf ;
		}
		//
		if ( iOffset + nLeftBytes > pBuf->dwBytes )
		{
			nLeftBytes = pBuf->dwBytes - iOffset ;
		}
		BYTE *	pbytSrc = pBuf->pbytBuf + iOffset ;
		for ( unsigned long int i = 0; i < nLeftBytes; i ++ )
		{
			pbytBuffer[i] = pbytSrc[i] ;
		}
		//
		// ポインタを進める
		//
		pbytBuffer += nLeftBytes ;
		nReadBytes += nLeftBytes ;
		nBytes -= nLeftBytes ;
		m_nPosition += nLeftBytes ;
	}
	Unlock( ) ;
	return	nReadBytes ;
}

// ファイルへ書き出す
//////////////////////////////////////////////////////////////////////////////
unsigned long int ESyncStreamFile::Write
	( const void * ptrBuffer, unsigned long int nBytes )
{
	unsigned long int	nWrittenBytes = 0 ;
	const BYTE *	pbytBuffer = (const BYTE *) ptrBuffer ;
	Lock( ) ;
	while ( nBytes != 0 )
	{
		//
		// 書き込むバッファを取得する
		//
		unsigned int	iBuf = m_nBufLength >> BUFFER_SCALE ;
		unsigned int	iOffset = m_nBufLength & BUFFER_MASK ;
		EBuffer *	pBuf = m_tblBuffer.GetAt( iBuf ) ;
		if ( pBuf == NULL )
		{
			pBuf = new EBuffer( iBuf << BUFFER_SCALE, m_pTempFile ) ;
			m_tblBuffer.SetAt( iBuf, pBuf ) ;
		}
		ESLAssert( pBuf->dwLocked >= 1 ) ;
		//
		// 書き込み可能なバイト数を計算し、書き込む
		//
		unsigned int	nWriteBytes = nBytes ;
		if ( nWriteBytes > BUFFER_SIZE - pBuf->dwBytes )
		{
			nWriteBytes = BUFFER_SIZE - pBuf->dwBytes ;
		}
		BYTE *	pbytDst = pBuf->pbytBuf + iOffset ;
		//
		for ( unsigned int i = 0; i < nWriteBytes; i ++ )
		{
			pbytDst[i] = pbytBuffer[i] ;
		}
		//
		// ポインタを進める
		//
		pbytBuffer += nWriteBytes ;
		nWrittenBytes += nWriteBytes ;
		nBytes -= nWriteBytes ;
		pBuf->dwBytes += nWriteBytes ;
		m_nBufLength += nWriteBytes ;
		//
		if ( pBuf->dwBytes >= BUFFER_SIZE )
		{
			ESLAssert( (m_nBufLength >> BUFFER_SCALE) != iBuf ) ;
			pBuf->Unlock( ) ;
		}
	}
	::SetEvent( m_hWritten ) ;
	Unlock( ) ;
	return	nWrittenBytes ;
}

// ファイルの長さを取得
//////////////////////////////////////////////////////////////////////////////
unsigned long int ESyncStreamFile::GetLength( void ) const
{
	if ( m_nLength != -1 )
	{
		return	max( m_nLength, m_nBufLength ) ;
	}
	return	m_nBufLength ;
}

// ファイルポインタを移動
//////////////////////////////////////////////////////////////////////////////
unsigned long int ESyncStreamFile::Seek
	( long int nOffsetPos, SeekOrigin fSeekFrom )
{
	Lock( ) ;
	switch ( fSeekFrom )
	{
	case	FromBegin:
		break ;
	case	FromCurrent:
		nOffsetPos += m_nPosition ;
		break ;
	case	FromEnd:
		if ( m_nLength != -1 )
			nOffsetPos += m_nLength ;
		else
			nOffsetPos += m_nBufLength ;
		break ;
	}
	//
	DWORD	dwResult ;
	HANDLE	hEvents[2] =
	{
		m_hWritten, m_hFinished
	} ;
	for ( ; ; )
	{
		if ( (unsigned long int) nOffsetPos <= m_nBufLength )
		{
			m_nPosition = nOffsetPos ;
			break ;
		}
		Unlock( ) ;
		dwResult =
			::WaitForMultipleObjects
				( 2, hEvents, FALSE, INFINITE ) ;
		Lock( ) ;
		if ( dwResult == (WAIT_OBJECT_0 + 1) )
		{
			break ;
		}
		::ResetEvent( m_hWritten ) ;
	}
	Unlock( ) ;
	return	m_nPosition ;
}

// ファイルポインタを取得
//////////////////////////////////////////////////////////////////////////////
unsigned long int ESyncStreamFile::GetPosition( void ) const
{
	return	m_nPosition ;
}

// ファイルの終端を現在の位置に設定する
//////////////////////////////////////////////////////////////////////////////
ESLError ESyncStreamFile::SetEndOfFile( void )
{
	FinishStream( ) ;
	return	eslErrSuccess ;
}


#endif
