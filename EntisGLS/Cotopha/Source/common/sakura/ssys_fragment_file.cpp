
#include <sakuragl/sakuragl.h>
#include <sakuragl/sgl_erisa_lib.h>
#include <sakura/ssys_fragment_file.h>

using namespace SSystem ;


//////////////////////////////////////////////////////////////////////////////
// 分割ファイル・キャッシュ
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SFragmentFile::CacheObject::CacheObject( void )
{
	m_countRef = 1 ;
	m_limitCahce = 4 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SFragmentFile::CacheObject::~CacheObject( void )
{
	ESLAssert( m_countRef == 0 ) ;
}

// 参照追加
//////////////////////////////////////////////////////////////////////////////
void SFragmentFile::CacheObject::AddRef( void )
{
	AtomicAdd( &m_countRef, 1 ) ;
}

// 参照解放
//////////////////////////////////////////////////////////////////////////////
void SFragmentFile::CacheObject::ReleaseRef( void )
{
	if ( AtomicSub( &m_countRef, 1 ) == 0 )
	{
		delete	this ;
	}
}

// エントリ検索
//////////////////////////////////////////////////////////////////////////////
ssize_t SFragmentFile::CacheObject::FindCacheEntry( size_t iFragment ) const
{
	CacheEntry*const*	ppEntries = GetConstArray() ;
	size_t				nCount = GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		CacheEntry *	pCache = ppEntries[i] ;
		ESLAssert( pCache != NULL ) ;
		if ( pCache->m_iFragment == iFragment )
		{
			return	(ssize_t) i ;
		}
	}
	return	-1 ;
}

// エントリ追加
//////////////////////////////////////////////////////////////////////////////
void SFragmentFile::CacheObject::AddCacheEntry( SFragmentFile::CacheEntry * pCache )
{
	ssize_t	iCache = FindCacheEntry( pCache->m_iFragment ) ;
	if ( iCache >= 0 )
	{
		RemoveAt( iCache ) ;
	}
	InsertAt( 0, pCache ) ;
	if ( GetLength() > m_limitCahce )
	{
		Remove( m_limitCahce, GetLength() - m_limitCahce ) ;
	}
}

// 排他処理
//////////////////////////////////////////////////////////////////////////////
void SFragmentFile::CacheObject::Lock( void )
{
	m_csSync.Lock() ;
}

void SFragmentFile::CacheObject::Unlock( void )
{
	m_csSync.Unlock() ;
}


//////////////////////////////////////////////////////////////////////////////
// 分割ファイル
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SFragmentFile, SFileInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SFragmentFile::SFragmentFile( void )
{
	m_pCache = NULL ;
	m_fposLoaded = 0 ;
	//
	m_length = 0 ;
	m_opener = NULL ;
	m_flagOwnOpener = false ;
}

SFragmentFile::SFragmentFile( const SFragmentFile& ff )
	: m_sbufLoaded( ff.m_sbufLoaded ), m_fragments( ff.m_fragments )
{
	m_pCache = ff.m_pCache ;
	if ( m_pCache != NULL )
	{
		m_pCache->AddRef() ;
	}
	m_fposLoaded = ff.m_fposLoaded ;
	//
	m_length = ff.m_length ;
	m_opener = ff.m_opener ;
	m_flagOwnOpener = false ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SFragmentFile::~SFragmentFile( void )
{
	if ( m_pCache != NULL )
	{
		m_pCache->ReleaseRef() ;
		m_pCache = NULL ;
	}
	if ( m_flagOwnOpener )
	{
		delete	m_opener ;
	}
	m_flagOwnOpener = false ;
	m_opener = NULL ;
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SError SFragmentFile::Open
	( SFileInterface& file, SFileOpener * opener, bool fOwnOpener )
{
	if ( m_pCache != NULL )
	{
		m_pCache->ReleaseRef() ;
		m_pCache = NULL ;
	}
	if ( m_flagOwnOpener )
	{
		delete	m_opener ;
	}
	m_pCache = new CacheObject ;
	//
	m_fragments.RemoveAll() ;
	m_length = 0 ;
	m_opener = opener ;
	m_flagOwnOpener = fOwnOpener ;
	//
	// XML を読み込む
	//
	SXMLDocument			xmlDoc ;
	SParserErrorInterface	perr ;
	if ( xmlDoc.ReadDocument( file, perr ) )
	{
		return	errFailed ;
	}
	//
	// XML 解釈
	//
	SXMLDocument *	pxmlFragments = xmlDoc.GetElementTagAs( L"fragments" ) ;
	if ( pxmlFragments == NULL )
	{
		return	errFailed ;
	}
	const size_t	nCount = pxmlFragments->GetElementsCount() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		SXMLDocument *	pxmlTag = pxmlFragments->GetElementAt( i ) ;
		if ( (pxmlTag == NULL)
			|| (pxmlTag->GetTag() != L"fragment") )
		{
			continue ;
		}
		Fragment *	frg = new Fragment ;
		frg->m_pos = m_length ;
		m_fragments.Add( frg ) ;
		//
		SString *	pstrSize = pxmlTag->GetAttributeAs( L"size" ) ;
		if ( pstrSize != NULL )
		{
			SStringParser	sparsSize ;
			sparsSize.AttachString( *pstrSize ) ;
			frg->m_length =
				sparsSize.NextInteger( sparsSize.IsNextNumber() ) ;
			m_length += frg->m_length ;
		}
		frg->m_file = pxmlTag->GetAttrStringAs( L"file" ) ;
		//
		if ( pxmlTag->GetAttrStringAs( L"encode" ) == L"erina" )
		{
			frg->m_encoding = Fragment::encodingErina ;
		}
	}
	//
	// 先頭フラグメントロード
	//
	return	LoadFragment( 0 ) ;
}

// キャッシュの制限値（分割ファイル数）を設定する
//////////////////////////////////////////////////////////////////////////////
void SFragmentFile::SetCacheLimit( size_t nLimit )
{
	if ( (m_pCache != NULL) && (nLimit >= 1) )
	{
		m_pCache->m_limitCahce = nLimit ;
	}
}

// フラグメントファイルをロードする
//////////////////////////////////////////////////////////////////////////////
SError SFragmentFile::LoadFragment( int64_t fpos )
{
	m_sbufLoaded.ReleaseBuffer() ;
	m_fposLoaded = 0 ;
	//
	const size_t	nCount = m_fragments.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		Fragment *	frg = m_fragments.GetAt( i ) ;
		if ( frg == NULL )
		{
			continue ;
		}
		if ( fpos >= m_fposLoaded + frg->m_length )
		{
			m_fposLoaded += frg->m_length ;
			continue ;
		}
		bool	flagCached = false ;
		ESLAssert( m_pCache != NULL ) ;
		m_pCache->Lock() ;
		ssize_t	iCache = m_pCache->FindCacheEntry( i ) ;
		if ( iCache >= 0 )
		{
			CacheEntry *	pCache = m_pCache->GetAt( iCache ) ;
			ESLAssert( pCache != NULL ) ;
			ESLAssert( pCache->m_fposCached == m_fposLoaded ) ;
			//
			m_sbufLoaded.CopyReferenceBuffer( *pCache ) ;
			flagCached = true ;
			//
			ESLVerify( m_pCache->DetachAt( iCache ) == pCache ) ;
			m_pCache->InsertAt( 0, pCache ) ;
		}
		m_pCache->Unlock() ;
		//
		if ( !flagCached )
		{
			SSmartPointer<SFileInterface>	file =
				m_opener->NewOpenFile
					( frg->m_file, SFileOpener::shareRead ) ;
			if ( file == NULL )
			{
				return	errFailed ;
			}
			SSmartPointer<ERISA::SGLDecodeBitStream>		pBitStream ;
			SSmartPointer<ERISA::SGLHuffmanDecodeContext>	pContext ;
			SInputStream *	pStream = file.Ptr() ;
			if ( frg->m_encoding == Fragment::encodingErina )
			{
				pBitStream = new ERISA::SGLDecodeBitStream( 0x10000 ) ;
				pBitStream->AttachInputStream( pStream ) ;
				//
				pContext = new ERISA::SGLHuffmanDecodeContext( pBitStream ) ;
				pContext->PrepareToDecodeERINACode() ;
			}
			//
			CacheEntry *	pCache = new CacheEntry ;
			pCache->ReadFromStream( *pStream, (ssize_t) frg->m_length ) ;
			//
			pCache->m_fposCached = m_fposLoaded ;
			pCache->m_iFragment = i ;
			//
			m_sbufLoaded.CopyReferenceBuffer( *pCache ) ;
			//
			m_pCache->Lock() ;
			m_pCache->AddCacheEntry( pCache ) ;
			m_pCache->Unlock() ;
		}
		m_sbufLoaded.Seek( fpos - m_fposLoaded ) ;
		return	errSuccess ;
	}
	return	errFailed ;
}

// ファイルインターフェースの複製
//////////////////////////////////////////////////////////////////////////////
SFileInterface * SFragmentFile::Duplicate( void ) const
{
	return	new SFragmentFile( *this ) ;
}

// ファイルから読み込み
//////////////////////////////////////////////////////////////////////////////
size_t SFragmentFile::Read( void * ptrBuf, size_t nBytes )
{
	size_t	nTotalBytes = 0 ;
	while ( nBytes != 0 )
	{
		size_t	nReadBytes = m_sbufLoaded.Read( ptrBuf, nBytes ) ;
		nTotalBytes += nReadBytes ;
		ptrBuf = ((uint8_t*)ptrBuf) + nReadBytes ;
		//
		if ( nReadBytes >= nBytes )
		{
			break ;
		}
		nBytes -= nReadBytes ;
		//
		if ( GetPosition() >= m_length )
		{
			break ;
		}
		if ( LoadFragment( GetPosition() ) )
		{
			break ;
		}
	}
	return	nTotalBytes ;
}

// ファイルへ書き込み
//////////////////////////////////////////////////////////////////////////////
size_t SFragmentFile::Write( const void * ptrBuf, size_t nBytes )
{
	return	0 ;
}

// シーク可能か否か？
//////////////////////////////////////////////////////////////////////////////
bool SFragmentFile::IsSeekable( void ) const
{
	return	true ;
}

// ファイル長の取得
//////////////////////////////////////////////////////////////////////////////
int64_t SFragmentFile::GetLength( void ) const
{
	return	m_length ;
}

// ファイルポインタを移動
//////////////////////////////////////////////////////////////////////////////
int64_t SFragmentFile::Seek
	( int64_t posFile, SFileInterface::SeekOrigin seekFrom )
{
	switch ( seekFrom )
	{
	case	FromBegin:
		break ;
	case	FromCurrent:
		posFile += GetPosition() ;
		break ;
	case	FromEnd:
		posFile += m_length ;
		break ;
	}
	if ( (posFile < m_fposLoaded)
		|| (posFile >= m_fposLoaded + m_sbufLoaded.GetLength()) )
	{
		LoadFragment( posFile ) ;
	}
	else
	{
		m_sbufLoaded.Seek( posFile - m_fposLoaded ) ;
	}
	return	GetPosition() ;
}

// ファイルポインタを取得
//////////////////////////////////////////////////////////////////////////////
int64_t SFragmentFile::GetPosition( void ) const
{
	return	m_fposLoaded + m_sbufLoaded.GetPosition() ;
}

// ファイルの終端を現在の位置に設定する
//////////////////////////////////////////////////////////////////////////////
SError SFragmentFile::SetEndOfFile( void )
{
	return	errFailed ;
}


//////////////////////////////////////////////////////////////////////////////
// 分割ファイル・オープナー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SFragmentFileOpener, SOffsetFileOpener )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SFragmentFileOpener::SFragmentFileOpener
	( const wchar_t * pszBasePath,
		wchar_t wchSeparator,
		SFileOpener * pOpener, bool flagOwner, ssize_t nCacheSize )
	: SOffsetFileOpener( pszBasePath, wchSeparator, pOpener, flagOwner ),
		m_ofo( pszBasePath, wchSeparator, pOpener, false ), m_limitCache( nCacheSize )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SFragmentFileOpener::~SFragmentFileOpener( void )
{
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SFileInterface * SFragmentFileOpener::NewOpenFile
	( const wchar_t * pszFilePath, long int nOpenFlags )
{
	SFileInterface *	pFile =
		SOffsetFileOpener::NewOpenFile( pszFilePath, nOpenFlags ) ;
	if ( pFile != NULL )
	{
		SFragmentFile *	pFragment = new SFragmentFile ;
		SError	err ;
		SString	strDirPath = SString(pszFilePath).GetFileDirectoryPart() ;
		if ( strDirPath.IsEmpty()
			|| (strDirPath == L"/") || (strDirPath == L"\\") )
		{
			err = pFragment->Open( *pFile, &m_ofo ) ;
		}
		else
		{
			err = pFragment->Open
				( *pFile, new SOffsetFileOpener
							( strDirPath, L'/', &m_ofo, false ) ) ;
		}
		delete	pFile ;
		if ( !err )
		{
			return	pFragment ;
		}
		if ( m_limitCache >= 1 )
		{
			pFragment->SetCacheLimit( (size_t) m_limitCache ) ;
		}
		delete	pFragment ;
	}
	return	NULL ;
}


