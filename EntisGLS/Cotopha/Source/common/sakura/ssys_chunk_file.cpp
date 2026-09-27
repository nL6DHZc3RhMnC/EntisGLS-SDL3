
#include <sakura/sakura.h>

using namespace SSystem ;


//////////////////////////////////////////////////////////////////////////////
// 部分領域ファイルインターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SFileDomainInterface, SFileInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SFileDomainInterface::SFileDomainInterface( void )
{
	m_pFile = NULL ;
	m_flagFileOwner = false ;
	m_nFlags = 0 ;
	m_baseDomain = 0 ;
	m_lengthDomain = 0 ;
}

SFileDomainInterface::SFileDomainInterface
	( SFileInterface * pFile,
		bool flagOwner, long int nFlags,
		uint64_t baseDomain, uint64_t lengthDomain )
{
	m_pFile = pFile ;
	m_flagFileOwner = flagOwner ;
	m_nFlags = nFlags ;
	m_baseDomain = baseDomain ;
	m_lengthDomain = lengthDomain ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SFileDomainInterface::~SFileDomainInterface( void )
{
	if ( m_flagFileOwner )
	{
		delete	m_pFile ;
		m_pFile = NULL ;
		m_flagFileOwner = false ;
	}
}

// ファイル関連付け
//////////////////////////////////////////////////////////////////////////////
void SFileDomainInterface::AttachFile
	( SFileInterface * pFile,
		bool flagOwner, long int nFlags,
		uint64_t baseDomain, uint64_t lengthDomain )
{
	if ( m_flagFileOwner )
	{
		delete	m_pFile ;
	}
	m_pFile = pFile ;
	m_flagFileOwner = flagOwner ;
	m_nFlags = nFlags ;
	m_baseDomain = baseDomain ;
	m_lengthDomain = lengthDomain ;
}

// ファイルインターフェースの複製
//////////////////////////////////////////////////////////////////////////////
SFileInterface * SFileDomainInterface::Duplicate( void ) const
{
	if ( m_pFile == NULL )
	{
		return	new SFileDomainInterface ;
	}
	SFileInterface *	pFile = m_pFile->Duplicate() ;
	if ( pFile != NULL )
	{
		pFile->Seek( m_baseDomain ) ;
	}
	return	new SFileDomainInterface
				( pFile, true, m_nFlags, m_baseDomain, m_lengthDomain ) ;
}

// ファイルから読み込み
//////////////////////////////////////////////////////////////////////////////
size_t SFileDomainInterface::Read( void * ptrBuf, size_t nBytes )
{
	ESLAssert( m_pFile != NULL ) ;
	if ( m_pFile == NULL )
	{
		return	0 ;
	}
	int64_t	fpCurrent = m_pFile->GetPosition() ;
	int64_t	nLeftBytes = m_lengthDomain - (fpCurrent - m_baseDomain) ;
	if ( nLeftBytes <= 0 )
	{
		return	0 ;
	}
	if ( nLeftBytes < (int64_t) nBytes )
	{
		nBytes = (size_t) nLeftBytes ;
	}
	return	m_pFile->Read( ptrBuf, nBytes ) ;
}

// ファイルへ書き込み
//////////////////////////////////////////////////////////////////////////////
size_t SFileDomainInterface::Write( const void * ptrBuf, size_t nBytes )
{
	ESLAssert( m_pFile != NULL ) ;
	if ( m_pFile == NULL )
	{
		return	0 ;
	}
	if ( !IsFileCreatingMode() )
	{
		int64_t	fpCurrent = m_pFile->GetPosition() ;
		int64_t	nLeftBytes = m_lengthDomain - (fpCurrent - m_baseDomain) ;
		if ( nLeftBytes <= 0 )
		{
			return	0 ;
		}
		if ( nLeftBytes < (int64_t) nBytes )
		{
			nBytes = (size_t) nLeftBytes ;
		}
		return	m_pFile->Write( ptrBuf, nBytes ) ;
	}
	else
	{
		size_t	nWritten = m_pFile->Write( ptrBuf, nBytes ) ;
		//
		int64_t	nLength = m_pFile->GetPosition() - m_baseDomain ;
		if ( (nLength > 0) & ((uint64_t) nLength > m_lengthDomain) )
		{
			m_lengthDomain = nLength ;
		}
		return	nWritten ;
	}
}

// シーク可能か否か？
//////////////////////////////////////////////////////////////////////////////
bool SFileDomainInterface::IsSeekable( void ) const
{
	ESLAssert( m_pFile != NULL ) ;
	if ( m_pFile == NULL )
	{
		return	false ;
	}
	return	m_pFile->IsSeekable() ;
}

// ファイル長の取得
//////////////////////////////////////////////////////////////////////////////
int64_t SFileDomainInterface::GetLength( void ) const
{
	return	m_lengthDomain ;
}

// ファイルポインタを移動
//////////////////////////////////////////////////////////////////////////////
int64_t SFileDomainInterface::Seek
	( int64_t posFile, SFileInterface::SeekOrigin seekFrom )
{
	ESLAssert( m_pFile != NULL ) ;
	if ( m_pFile == NULL )
	{
		return	0 ;
	}
	switch ( seekFrom )
	{
	case	FromBegin:
		posFile += m_baseDomain ;
		break ;
	case	FromCurrent:
		posFile += m_pFile->GetPosition() ;
		break ;
	case	FromEnd:
		posFile += m_baseDomain + m_lengthDomain ;
		break ;
	}
	if ( (uint64_t) posFile < m_baseDomain )
	{
		return	m_pFile->Seek( m_baseDomain ) - m_baseDomain ;
	}
	if ( !IsFileCreatingMode() )
	{
		if ( m_lengthDomain < (posFile - m_baseDomain) )
		{
			posFile = m_baseDomain + m_lengthDomain ;
		}
		return	m_pFile->Seek( posFile ) - m_baseDomain ;
	}
	uint64_t	fpCurrent = m_pFile->Seek( posFile ) - m_baseDomain ;
	if ( fpCurrent > m_lengthDomain )
	{
		m_lengthDomain = fpCurrent ;
	}
	return	fpCurrent ;
}

// ファイルポインタを取得
//////////////////////////////////////////////////////////////////////////////
int64_t SFileDomainInterface::GetPosition( void ) const
{
	ESLAssert( m_pFile != NULL ) ;
	if ( m_pFile == NULL )
	{
		return	0 ;
	}
	return	m_pFile->GetPosition() - m_baseDomain ;
}

// ファイルの終端を現在の位置に設定する
//////////////////////////////////////////////////////////////////////////////
SError SFileDomainInterface::SetEndOfFile( void )
{
	ESLAssert( m_pFile != NULL ) ;
	if ( m_pFile == NULL )
	{
		return	errFailed ;
	}
	if ( !IsFileCreatingMode() )
	{
		return	errFailed ;
	}
	m_pFile->SetEndOfFile() ;
	//
	int64_t	fpCurrent = m_pFile->GetPosition() - m_baseDomain ;
	if ( fpCurrent > 0 )
	{
		m_lengthDomain = fpCurrent ;
	}
	return	errSuccess ;
}


//////////////////////////////////////////////////////////////////////////////
// チャンクファイル（Entis メディア複合ファイル形式）
//////////////////////////////////////////////////////////////////////////////

const BYTE	SSystem::SChunkFile::m_bytDefaultSignature[8] =
{
	(BYTE) 'E', (BYTE) 'n', (BYTE) 't', (BYTE) 'i', (BYTE) 's', 0x1A, 0, 0
} ;

// ファイルヘッダ情報設定
//////////////////////////////////////////////////////////////////////////////
void SChunkFile::FILE_HEADER::SetHeaderInfo( DWORD idFile, const char * pszDesc )
{
	eslMoveMemory
		( &bytSignature[0], &SChunkFile::m_bytDefaultSignature[0], 8 ) ;
	dwFileID = idFile ;
	dwReserved = 0 ;
	//
	size_t	i = 0 ;
	while ( (i < 0x30) && pszDesc[i] )
	{
		bytFormatDesc[i] = (BYTE) pszDesc[i] ;
		i ++ ;
	}
	while ( i < 0x30 )
	{
		bytFormatDesc[i] = 0 ;
		i ++ ;
	}
}

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SChunkFile, SFileDomainInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SChunkFile::SChunkFile( void )
{
	m_pChunk = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SChunkFile::~SChunkFile( void )
{
	SChunkFile::Close() ;
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SError SChunkFile::OpenChunkFile
	( SFileInterface * pFile, bool flagOwner,
		long int nFlags, const FILE_HEADER * pfhHeader )
{
	Close() ;
	//
	// ファイルヘッダ
	//
	if ( nFlags & SFileOpener::modeCreateFlag )
	{
		if ( pfhHeader == NULL )
		{
			return	errFailed ;
		}
		if ( pFile->Write
			( pfhHeader, sizeof(FILE_HEADER) ) < sizeof(FILE_HEADER) )
		{
			ESLTrace( "failed to write EMC file header.\n" ) ;
			return	errFailed ;
		}
		m_fhHeader = *pfhHeader ;
	}
	else
	{
		if ( pFile->Read
			( &m_fhHeader, sizeof(FILE_HEADER) ) < sizeof(FILE_HEADER) )
		{
			ESLTrace( "failed to read EMC file header.\n" ) ;
			return	errFailed ;
		}
		for ( int i = 0; i < 8; i ++ )
		{
			if ( m_bytDefaultSignature[i] != m_fhHeader.bytSignature[i] )
			{
				return	errFailed ;
			}
		}
	}
	//
	// ルートチャンク
	//
	int64_t	fpCurrent = pFile->GetPosition() ;
	int64_t	nFileLen = pFile->GetLength() - fpCurrent ;
	//
	m_nestChunk.SetLength( 1 ) ;
	m_pChunk = m_nestChunk.GetAt( 0 ) ;
	m_pChunk->idChunk = 0 ;
	m_pChunk->nLength = nFileLen ;
	m_pChunk->nPos = fpCurrent ;
	//
	AttachFile( pFile, flagOwner, nFlags, fpCurrent, nFileLen ) ;
	//
	return	errSuccess ;
}

// リソースを解放する
//////////////////////////////////////////////////////////////////////////////
void SChunkFile::Close( void )
{
	if ( IsFileCreatingMode() )
	{
		while ( m_nestChunk.GetLength() > 1 )
		{
			if ( AscendChunk() != errSuccess )
			{
				break ;
			}
		}
	}
	m_nestChunk.RemoveAll() ;
	m_pChunk = NULL ;
	AttachFile( NULL, false, 0, 0, 0 ) ;
}

// チャンクを開く
//////////////////////////////////////////////////////////////////////////////
SError SChunkFile::DescendChunk( const char * pszChunkID )
{
	ESLAssert( m_pFile != NULL ) ;
	if ( m_pFile == NULL )
	{
		return	errFailed ;
	}
	CHUNK_INFO	chunk ;
	if ( !IsFileCreatingMode() )
	{
		for ( ; ; )
		{
			if ( Read
				( (CHUNK_HEADER*) &chunk,
					sizeof(CHUNK_HEADER) ) < sizeof(CHUNK_HEADER) )
			{
				return	errFailed ;
			}
			chunk.nPos = m_pFile->GetPosition() ;
			if ( pszChunkID == NULL )
			{
				break ;
			}
			if ( IsEqualChunkID( chunk.idChunk, pszChunkID ) )
			{
				break ;
			}
			Seek( chunk.nLength, FromCurrent ) ;
		}
	}
	else
	{
		if ( pszChunkID == nullptr )
		{
			return	errFailed ;
		}
		BYTE *	pbytID = (BYTE*) &(chunk.idChunk) ;
		for ( int i = 0; i < 8; i ++ )
		{
			if ( pszChunkID[i] != 0 )
			{
				pbytID[i] = (BYTE) pszChunkID[i] ;
			}
			else
			{
				while ( i < 8 )
				{
					pbytID[i ++] = (BYTE) ' ' ;
				}
				break ;
			}
		}
		chunk.nLength = 0 ;
		if ( m_pFile->Write
			( (CHUNK_HEADER*) &chunk,
				sizeof(CHUNK_HEADER) ) < sizeof(CHUNK_HEADER) )
		{
			ESLTrace( "failed to write chunk header \'%s\'\n", pszChunkID ) ;
			return	errFailed ;
		}
		chunk.nPos = m_pFile->GetPosition() ;
	}
	const size_t	iNest = m_nestChunk.Add( chunk ) ;
	m_pChunk = m_nestChunk.GetAt( iNest ) ;
	m_baseDomain = chunk.nPos ;
	m_lengthDomain = chunk.nLength ;
	return	errSuccess ;
}

// チャンクを閉じる
//////////////////////////////////////////////////////////////////////////////
SError SChunkFile::AscendChunk( void )
{
	ESLAssert( m_pFile != NULL ) ;
	if ( m_pFile == NULL )
	{
		return	errFailed ;
	}
	if ( m_nestChunk.GetLength() > 1 )
	{
		if ( IsFileCreatingMode() )
		{
			m_pFile->Seek( m_baseDomain - sizeof(CHUNK_HEADER) ) ;
			m_pChunk->nLength = m_lengthDomain ;
			if ( m_pFile->Write
				( (CHUNK_HEADER*) m_pChunk,
						sizeof(CHUNK_HEADER) ) < sizeof(CHUNK_HEADER) )
			{
				return	errFailed ;
			}
			m_pFile->Seek( m_baseDomain + m_lengthDomain ) ;
			m_nestChunk.SetLength( m_nestChunk.GetLength() - 1 ) ;
			m_pChunk = m_nestChunk.GetLastAt( 0 ) ;
			m_lengthDomain =
				(m_baseDomain + m_lengthDomain) - m_pChunk->nPos ;
			m_baseDomain = m_pChunk->nPos ;
			if ( m_lengthDomain < m_pChunk->nLength )
			{
				m_lengthDomain = m_pChunk->nLength ;
			}
		}
		else
		{
			m_pFile->Seek( m_baseDomain + m_lengthDomain ) ;
			m_nestChunk.SetLength( m_nestChunk.GetLength() - 1 ) ;
			m_pChunk = m_nestChunk.GetLastAt( 0 ) ;
			m_baseDomain = m_pChunk->nPos ;
			m_lengthDomain = m_pChunk->nLength ;
		}
	}
	return	errSuccess ;
}

// ファイルポインタの更新通知（書き込みモード時のチャンクサイズ反映）
//////////////////////////////////////////////////////////////////////////////
void SChunkFile::UpdateFilePointer( void )
{
	if ( IsFileCreatingMode() )
	{
		ESLAssert( m_pFile != nullptr ) ;
		int64_t	nPos = m_pFile->GetPosition() - m_baseDomain ;
		if ( nPos > (int64_t) m_lengthDomain )
		{
			m_lengthDomain = nPos ;
		}
		ESLAssert( m_pChunk != nullptr ) ;
		if ( m_pChunk->nLength < m_lengthDomain )
		{
			m_pChunk->nLength = m_lengthDomain ;
		}
	}
}

// チャンク識別子一致判定
//////////////////////////////////////////////////////////////////////////////
bool SChunkFile::IsEqualChunkID( UINT64 idChunk, const char * pszChunk )
{
	const BYTE *	pbytID = (const BYTE*) &idChunk ;
	for ( int i = 0; i < 8; i ++ )
	{
		if ( pszChunk[i] != 0 )
		{
			if ( pbytID[i] != (BYTE) pszChunk[i] )
			{
				return	false ;
			}
		}
		else
		{
			while ( i < 8 )
			{
				if ( pbytID[i ++] != (BYTE) ' ' )
				{
					return	false ;
				}
			}
			break ;
		}
	}
	return	true ;
}

// ファイルへ書き込み
//////////////////////////////////////////////////////////////////////////////
size_t SChunkFile::Write( const void * ptrBuf, size_t nBytes )
{
	size_t	nWrittenBytes = SFileDomainInterface::Write( ptrBuf, nBytes ) ;
	//
	if ( IsFileCreatingMode() )
	{
		ESLAssert( m_pChunk != NULL ) ;
		m_pChunk->nLength = m_lengthDomain ;
	}
	return	nWrittenBytes ;
}

// ファイルポインタを移動
//////////////////////////////////////////////////////////////////////////////
int64_t SChunkFile::Seek( int64_t posFile, SeekOrigin seekFrom )
{
	int64_t	pos = SFileDomainInterface::Seek( posFile, seekFrom ) ;
	//
	if ( IsFileCreatingMode() )
	{
		ESLAssert( m_pChunk != NULL ) ;
		m_pChunk->nLength = m_lengthDomain ;
	}
	return	pos ;
}

// ファイルの終端を現在の位置に設定する
//////////////////////////////////////////////////////////////////////////////
SError SChunkFile::SetEndOfFile( void )
{
	SError	err = SFileDomainInterface::SetEndOfFile() ;
	//
	if ( IsFileCreatingMode() )
	{
		ESLAssert( m_pChunk != NULL ) ;
		m_pChunk->nLength = m_lengthDomain ;
	}
	return	err ;
}



//////////////////////////////////////////////////////////////////////////////
// チャンクファイル（Entis メディア複合ファイル形式）エディタ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SChunkFileEditor, SChunkFile )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SChunkFileEditor::SChunkFileEditor( void )
{
	m_flagWritten = false ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SChunkFileEditor::~SChunkFileEditor( void )
{
	SChunkFileEditor::Close() ;
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SError SChunkFileEditor::OpenChunkFile
	( SFileInterface * pDstFile, bool flagDstOwner,
		SFileInterface * pSrcFile, bool flagSrcOwner )
{
	SError	err = m_cfSrc.OpenChunkFile( pSrcFile, flagSrcOwner ) ;
	if ( err )
	{
		return	err ;
	}
	err = SChunkFile::OpenChunkFile
			( pDstFile, flagDstOwner,
				SFileOpener::modeCreate, &(m_cfSrc.GetFileHeader()) ) ;
	if ( err )
	{
		return	err ;
	}
	m_flagWritten = false ;
	return	errSuccess ;
}

// リソースを解放する
//////////////////////////////////////////////////////////////////////////////
void SChunkFileEditor::Close( void )
{
	SChunkFile::Close() ;
}

// ファイルから読み込み
//////////////////////////////////////////////////////////////////////////////
size_t SChunkFileEditor::Read( void * ptrBuf, size_t nBytes )
{
	return	m_cfSrc.Read( ptrBuf, nBytes ) ;
}

// ファイルへ書き込み
//////////////////////////////////////////////////////////////////////////////
size_t SChunkFileEditor::Write( const void * ptrBuf, size_t nBytes )
{
	m_flagWritten = true ;
	return	SChunkFile::Write( ptrBuf, nBytes ) ;
}

// ファイル長の取得
//////////////////////////////////////////////////////////////////////////////
int64_t SChunkFileEditor::GetLength( void ) const
{
	return	m_cfSrc.GetLength() ;
}

// ファイルポインタを移動
//////////////////////////////////////////////////////////////////////////////
int64_t SChunkFileEditor::Seek
	( int64_t posFile, SFileInterface::SeekOrigin seekFrom )
{
	return	m_cfSrc.Seek( posFile, seekFrom ) ;
}

// ファイルポインタを取得
//////////////////////////////////////////////////////////////////////////////
int64_t SChunkFileEditor::GetPosition( void ) const
{
	return	m_cfSrc.GetPosition() ;
}

// チャンクを開く
//////////////////////////////////////////////////////////////////////////////
SError SChunkFileEditor::DescendChunk( const char * pszChunkID )
{
	ESLAssert( m_pFile != NULL ) ;
	if ( (m_pFile == NULL) || (pszChunkID == NULL) )
	{
		return	errFailed ;
	}
	SArray<uint8_t>	bufTemp ;
	for ( ; ; )
	{
		SError	err = m_cfSrc.DescendChunk() ;
		if ( err )
		{
			return	err ;
		}
		UINT64	idChunk = m_cfSrc.GetCurrentChunkID() ;
		if ( IsEqualChunkID( idChunk, pszChunkID ) )
		{
			m_flagWritten = false ;
			return	SChunkFile::DescendChunk( pszChunkID ) ;
		}
		CHUNK_HEADER	chunk ;
		chunk.idChunk = idChunk ;
		chunk.nLength = (UINT64) m_cfSrc.GetLength() ;
		if ( SChunkFile::Write
			( &chunk, sizeof(CHUNK_HEADER) ) < sizeof(CHUNK_HEADER) )
		{
			return	errFailed ;
		}
		uint8_t *	pbytBuf = bufTemp.GetArray( 0x4000 ) ;
		uint64_t	nCopied = 0 ;
		while ( nCopied < chunk.nLength )
		{
			size_t	nCopyBytes = 0x4000 ;
			if ( nCopied + nCopyBytes > chunk.nLength )
			{
				nCopyBytes = (size_t) (chunk.nLength - nCopied) ;
			}
			nCopyBytes = m_cfSrc.Read( pbytBuf, nCopyBytes ) ;
			if ( SChunkFile::Write( pbytBuf, nCopyBytes ) < nCopyBytes )
			{
				return	errFailed ;
			}
			nCopied += nCopyBytes ;
		}
		bufTemp.FinishArray() ;
		m_cfSrc.AscendChunk() ;
	}
	return	errFailed ;
}

// チャンクを閉じる
//////////////////////////////////////////////////////////////////////////////
SError SChunkFileEditor::AscendChunk( void )
{
	if ( m_nestChunk.GetLength() > 1 )
	{
		if ( !m_flagWritten )
		{
			CopyAllSubChunks() ;
		}
	}
	m_flagWritten = false ;
	m_cfSrc.AscendChunk() ;
	return	SChunkFile::AscendChunk() ;
}

// 現在のチャンクの残りチャンクを複製
//////////////////////////////////////////////////////////////////////////////
SError SChunkFileEditor::CopyAllSubChunks( void )
{
	SArray<uint8_t>	bufTemp ;
	for ( ; ; )
	{
		SError	err = m_cfSrc.DescendChunk() ;
		if ( err )
		{
			break ;
		}
		CHUNK_HEADER	chunk ;
		chunk.idChunk = m_cfSrc.GetCurrentChunkID() ;
		chunk.nLength = (UINT64) m_cfSrc.GetLength() ;
		if ( SChunkFile::Write
			( &chunk, sizeof(CHUNK_HEADER) ) < sizeof(CHUNK_HEADER) )
		{
			return	errFailed ;
		}
		uint8_t *	pbytBuf = bufTemp.GetArray( 0x4000 ) ;
		uint64_t	nCopied = 0 ;
		while ( nCopied < chunk.nLength )
		{
			size_t	nCopyBytes = 0x4000 ;
			if ( nCopied + nCopyBytes > chunk.nLength )
			{
				nCopyBytes = (size_t) (chunk.nLength - nCopied) ;
			}
			nCopyBytes = m_cfSrc.Read( pbytBuf, nCopyBytes ) ;
			if ( SChunkFile::Write( pbytBuf, nCopyBytes ) < nCopyBytes )
			{
				return	errFailed ;
			}
			nCopied += nCopyBytes ;
		}
		bufTemp.FinishArray() ;
		m_cfSrc.AscendChunk() ;
	}
	return	errSuccess ;
}

