
#include <sakura/sakura.h>

using namespace SSystem ;


//////////////////////////////////////////////////////////////////////////////
// 一時バッファ付きファイル
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SBufferedFile, SSmartFile )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SBufferedFile::SBufferedFile( void )
{
	m_modeCache = cacheNothing ;
	m_nBuffered = 0 ;
	m_nOffset = 0 ;
	//
	m_encodingChar = Charset::encodingUTF8 ;
}

SBufferedFile::SBufferedFile
	( SFileOpener * pOpener,
		SFileInterface * pFile, bool flagOwner )
	: SSmartFile( pOpener, pFile, flagOwner )
{
	m_modeCache = cacheNothing ;
	m_nBuffered = 0 ;
	m_nOffset = 0 ;
	//
	m_encodingChar = Charset::encodingUTF8 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SBufferedFile::~SBufferedFile( void )
{
	SBufferedFile::FlushBuffer() ;
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SError SBufferedFile::Open
	( const wchar_t * pszFilePath, long int nOpenFlags, bool flagAppend )
{
	Close() ;
	//
	SFileInterface *	pFile =
		SFileOpener::DefaultNewOpenFile( pszFilePath, nOpenFlags ) ;
	if ( pFile == nullptr )
	{
		return	errFailed ;
	}
	if ( flagAppend )
	{
		pFile->Seek( 0, FromEnd ) ;
	}
	AttachFile( pFile, true ) ;
	return	errSuccess ;
}

// ファイルの参照を解除する
//////////////////////////////////////////////////////////////////////////////
void SBufferedFile::Close( void )
{
	FlushBuffer() ;
	SSmartFile::Close() ;
}

// キャッシュをフラッシュする
//////////////////////////////////////////////////////////////////////////////
void SBufferedFile::FlushBuffer( void )
{
	if ( m_modeCache == cacheWriteBack )
	{
		ESLAssert( m_pFile != NULL ) ;
		if ( m_pFile != NULL )
		{
			m_pFile->Write( &m_bufCache[0], m_nBuffered ) ;
		}
		m_nBuffered = 0 ;
		m_modeCache = cacheNothing ;
	}
	else if ( m_modeCache == cacheRead )
	{
		ESLAssert( m_pFile != NULL ) ;
		if ( (m_pFile != NULL) & (m_nOffset < m_nBuffered) )
		{
			m_pFile->Seek
				( (int64_t) m_nOffset - m_nBuffered, FromCurrent ) ;
		}
		m_modeCache = cacheNothing ;
		m_nBuffered = 0 ;
		m_nOffset = 0 ;
	}
}

// ファイルから読み込み
//////////////////////////////////////////////////////////////////////////////
size_t SBufferedFile::Read( void * ptrBuf, size_t nBytes )
{
	size_t		nReadBytes = 0 ;
	uint8_t *	pbytDstNext = (uint8_t*) ptrBuf ;
	while ( nBytes != 0 )
	{
		if ( m_modeCache == cacheRead )
		{
			size_t	nCopyBytes = m_nBuffered - m_nOffset ;
			if ( nCopyBytes > nBytes )
			{
				nCopyBytes = nBytes ;
			}
			bool		fEndOfFile = false ;
			uint8_t *	pbytBuf = &(m_bufCache[m_nOffset]) ;
			for ( size_t i = 0; i < nCopyBytes; i ++ )
			{
				if ( (pbytDstNext[i] = pbytBuf[i]) == 0x1A )
				{
					fEndOfFile = true ;
					nCopyBytes = i + 1 ;
					break ;
				}
			}
			nBytes -= nCopyBytes ;
			nReadBytes += nCopyBytes ;
			m_nOffset += nCopyBytes ;
			pbytDstNext += nCopyBytes ;
			//
			if ( m_nOffset >= m_nBuffered )
			{
				m_modeCache = cacheNothing ;
				m_nBuffered = 0 ;
				m_nOffset = 0 ;
			}
			if ( fEndOfFile )
			{
				break ;
			}
		}
		else if ( m_modeCache == cacheWriteBack )
		{
			FlushBuffer() ;
		}
		else
		{
			ESLAssert( m_pFile != NULL ) ;
			m_nBuffered = 0 ;
			m_nOffset = 0 ;
			if ( m_pFile != NULL )
			{
				m_nBuffered = m_pFile->Read( &m_bufCache[0], 0x100 ) ;
				if ( m_nBuffered > 0 )
				{
					m_modeCache = cacheRead ;
				}
				else
				{
					break ;
				}
			}
			else
			{
				break ;
			}
		}
	}
	return	nReadBytes ;
}

// ファイルへ書き込み
//////////////////////////////////////////////////////////////////////////////
size_t SBufferedFile::Write( const void * ptrBuf, size_t nBytes )
{
	size_t			nWrittenBytes = 0 ;
	const uint8_t *	pbytSrcNext = (const uint8_t*) ptrBuf ;
	while ( nBytes != 0 )
	{
		if ( m_modeCache == cacheWriteBack )
		{
			size_t	nCopyBytes = 0x100 - m_nBuffered ;
			if ( nCopyBytes > nBytes )
			{
				nCopyBytes = nBytes ;
			}
			eslMoveMemory
				( &m_bufCache[m_nBuffered], pbytSrcNext, nCopyBytes ) ;
			nBytes -= nCopyBytes ;
			nWrittenBytes += nCopyBytes ;
			m_nBuffered += nCopyBytes ;
			pbytSrcNext += nCopyBytes ;
			//
			if ( m_nBuffered >= 0x100 )
			{
				FlushBuffer() ;
			}
		}
		else if ( m_modeCache == cacheRead )
		{
			FlushBuffer() ;
		}
		else if ( nBytes < 0x100 )
		{
			m_modeCache = cacheWriteBack ;
			m_nBuffered = 0 ;
			m_nOffset = 0 ;
		}
		else
		{
			ESLAssert( m_pFile != NULL ) ;
			if ( m_pFile != NULL )
			{
				size_t	nWritten = m_pFile->Write( pbytSrcNext, nBytes ) ;
				nWrittenBytes += nWritten ;
			}
			break ;
		}
	}
	return	nWrittenBytes ;
}

// ファイル長の取得
//////////////////////////////////////////////////////////////////////////////
int64_t SBufferedFile::GetLength( void ) const
{
	if ( m_modeCache == cacheWriteBack )
	{
		ESLAssert( m_pFile != NULL ) ;
		if ( m_pFile != NULL )
		{
			int64_t	nWritingPos = SBufferedFile::GetPosition() ;
			int64_t	nFileLength = m_pFile->GetLength() ;
			if ( nFileLength < nWritingPos )
			{
				nFileLength = nWritingPos ;
			}
			return	nFileLength ;
		}
	}
	return	SSmartFile::GetLength() ;
}

// ファイルポインタを移動
//////////////////////////////////////////////////////////////////////////////
int64_t SBufferedFile::Seek
	( int64_t posFile, SeekOrigin seekFrom )
{
	FlushBuffer() ;
	return	SSmartFile::Seek( posFile, seekFrom ) ;
}

// ファイルポインタを取得
//////////////////////////////////////////////////////////////////////////////
int64_t SBufferedFile::GetPosition( void ) const
{
	if ( m_modeCache == cacheWriteBack )
	{
		ESLAssert( m_pFile != NULL ) ;
		if ( m_pFile != NULL )
		{
			return	m_pFile->GetPosition() + m_nBuffered ;
		}
	}
	else if ( m_modeCache == cacheRead )
	{
		ESLAssert( m_pFile != NULL ) ;
		if ( m_pFile != NULL )
		{
			return	m_pFile->GetPosition() - m_nBuffered + m_nOffset ;
		}
	}
	return	SSmartFile::GetPosition() ;
}

// ファイルの終端を現在の位置に設定する
//////////////////////////////////////////////////////////////////////////////
SError SBufferedFile::SetEndOfFile( void )
{
	FlushBuffer() ;
	return	SSmartFile::SetEndOfFile() ;
}

// 文字列書き出し
//////////////////////////////////////////////////////////////////////////////
size_t SBufferedFile::WriteString( const SString & strBuf )
{
	return	WriteString( strBuf, (ssize_t) strBuf.GetLength() ) ;
}

size_t SBufferedFile::WriteString( const wchar_t * pszStr, ssize_t nLength )
{
	SArray<uint8_t>	strDst ;
	Charset::Encode( strDst, m_encodingChar, pszStr, nLength ) ;
	//
	Write( strDst.GetConstArray(), strDst.GetLength() ) ;
	//
	if ( m_modeCache == cacheWriteBack )
	{
		for ( size_t i = 0; i < m_nBuffered; i ++ )
		{
			if ( (m_bufCache[i] == '\n') | (m_bufCache[i] == '\r') )
			{
				FlushBuffer() ;
				break ;
			}
		}
	}
	return	strDst.GetLength() ;
}

size_t SBufferedFile::WriteFormat( const wchar_t * pszFormat, ... )
{
	va_list	vl ;
	va_start( vl, pszFormat ) ;
	return	WriteFormatV( pszFormat, vl ) ;
}

size_t SBufferedFile::WriteFormatV( const wchar_t * pszFormat, va_list argptr )
{
	if ( pszFormat == nullptr )
	{
		return	0 ;
	}
	SString	strBuf ;
	strBuf.FormatV( pszFormat, argptr ) ;
	return	WriteString( strBuf ) ;
}

// 文字列１行読み込み
//////////////////////////////////////////////////////////////////////////////
size_t SBufferedFile::ReadStringLine( SString & strBuf )
{
	SArray<uint8_t>	strTemp ;
	size_t	nReadBytes = 0 ;
	strBuf = L"" ;
	//
	if ( m_modeCache == cacheWriteBack )
	{
		FlushBuffer() ;
	}
	for ( ; ; )
	{
		if ( m_modeCache != cacheRead )
		{
			ESLAssert( m_pFile != NULL ) ;
			m_nBuffered = 0 ;
			m_nOffset = 0 ;
			if ( m_pFile != NULL )
			{
				m_modeCache = cacheRead ;
				m_nBuffered = m_pFile->Read( &m_bufCache[0], 0x100 ) ;
			}
			if ( m_nBuffered == 0 )
			{
				break ;
			}
		}
		bool	fEndOfLine = false ;
		size_t	nLine = m_nOffset ;
		while ( nLine < m_nBuffered )
		{
			if ( m_bufCache[nLine ++] == '\n' )
			{
				fEndOfLine = true ;
				break ;
			}
		}
		strTemp.AddArray( &m_bufCache[m_nOffset], nLine - m_nOffset ) ;
		m_nOffset = nLine ;
		nReadBytes += nLine ;
		if ( m_nOffset >= m_nBuffered )
		{
			FlushBuffer() ;
		}
		if ( fEndOfLine )
		{
			break ;
		}
	}
	Charset::Decode
		( strBuf, m_encodingChar, strTemp, (ssize_t) strTemp.GetLength() ) ;
	//
	return	nReadBytes ;
}

// 文字列書き出し
//////////////////////////////////////////////////////////////////////////////
SBufferedFile& SBufferedFile::operator += ( const SString & strBuf )
{
	WriteString( strBuf ) ;
	return	*this ;
}

SBufferedFile& SBufferedFile::operator += ( const wchar_t * pszStr )
{
	WriteString( pszStr ) ;
	return	*this ;
}

SBufferedFile& SBufferedFile::operator << ( const SString & strBuf )
{
	WriteString( strBuf ) ;
	return	*this ;
}

SBufferedFile& SBufferedFile::operator << ( const wchar_t * pszStr )
{
	WriteString( pszStr ) ;
	return	*this ;
}

SBufferedFile& SBufferedFile::operator << ( int64_t nValue )
{
	SString	strNumber ;
	strNumber.FromInteger( nValue ) ;
	WriteString( strNumber ) ;
	return	*this ;
}

SBufferedFile& SBufferedFile::operator << ( double nValue )
{
	SString	strNumber ;
	strNumber.FromReal( nValue ) ;
	WriteString( strNumber ) ;
	return	*this ;
}

// 文字列１行読み込み
//////////////////////////////////////////////////////////////////////////////
SBufferedFile& SBufferedFile::operator >> ( SString & strBuf )
{
	ReadStringLine( strBuf ) ;
	return	*this ;
}



//////////////////////////////////////////////////////////////////////////////
// 同期バッファ付きファイル
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SSyncBufferedFile, SBufferedFile )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SSyncBufferedFile::SSyncBufferedFile( void )
{
}

SSyncBufferedFile::SSyncBufferedFile
		( SFileOpener * pOpener,
			SFileInterface * pFile, bool flagOwner )
	: SBufferedFile( pOpener, pFile, flagOwner )
{
}

// キャッシュをフラッシュする
//////////////////////////////////////////////////////////////////////////////
void SSyncBufferedFile::FlushBuffer( void )
{
	m_csSync.Lock() ;
	SBufferedFile::FlushBuffer() ;
	m_csSync.Unlock() ;
}

// ファイルから読み込み
//////////////////////////////////////////////////////////////////////////////
size_t SSyncBufferedFile::Read( void * ptrBuf, size_t nBytes )
{
	size_t	nRead = 0 ;
	m_csSync.Lock() ;
	nRead = SBufferedFile::Read( ptrBuf, nBytes ) ;
	m_csSync.Unlock() ;
	return	nRead ;
}

// ファイルへ書き込み
//////////////////////////////////////////////////////////////////////////////
size_t SSyncBufferedFile::Write( const void * ptrBuf, size_t nBytes )
{
	size_t	nRead = 0 ;
	m_csSync.Lock() ;
	nRead = SBufferedFile::Write( ptrBuf, nBytes ) ;
	m_csSync.Unlock() ;
	return	nRead ;
}

