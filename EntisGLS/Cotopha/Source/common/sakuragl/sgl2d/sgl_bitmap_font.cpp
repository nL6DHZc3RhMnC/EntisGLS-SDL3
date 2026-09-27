
#include <sakuragl/sakuragl.h>
#include <sakuragl/sgl2d_image.h>
#include <sakuragl/sgl2d/sgl_bitmap_font.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// 画像フォントオブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLBitmapFontLoader, SGLFontObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLBitmapFontLoader::SGLBitmapFontLoader( void )
{
	m_countLoaded = 0 ;
	m_limitLoaded = (4*1024*1024) >> bufferPageBits ;
	m_pSelFontSet = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLBitmapFontLoader::~SGLBitmapFontLoader( void )
{
	SGLBitmapFontLoader::Close() ;
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SGLError SGLBitmapFontLoader::OpenFontFile( const wchar_t * pwszFilePath )
{
	SFileInterface *	pFile =
			SFileOpener::DefaultNewOpenFile
					( pwszFilePath, SFileOpener::shareRead ) ;
	if ( pFile == NULL )
	{
		return	sglErrFailed ;
	}
	return	OpenFontFile( pFile, true ) ;
}

SGLError SGLBitmapFontLoader::OpenFontFile( SFileInterface * pFile, bool flagOwner )
{
	//
	// チャンクファイルを開く
	//
	Close() ;
	//
	if ( m_cfFont.OpenChunkFile( pFile, flagOwner, SFileOpener::modeRead ) )
	{
		return	sglErrFailed ;
	}
	//
	// サイズ／スタイル・ヘッダを読み込む
	//
	for ( ; ; )
	{
		if ( m_cfFont.DescendChunk() )
		{
			m_cfFont.Close() ;
			return	sglErrFailed ;
		}
		if ( m_cfFont.IsEqualCurrentChunkID( "fontentr" ) )
		{
			//
			// ヘッダ読み込み
			//
			FontSet *	pFontSet = new FontSet ;
			m_arrayFontSets.Add( pFontSet ) ;
			if ( m_cfFont.Read
				( &(pFontSet->m_style),
					sizeof(FontEntry) ) < sizeof(FontEntry) )
			{
				return	sglErrFailed ;
			}
			//
			// 文字エントリ配列読み込み
			//
			const size_t	countEntries = pFontSet->m_style.countCharacters ;
			const size_t	bytesEntries = countEntries * sizeof(CharacterEntry) ;
			pFontSet->m_entries.SetLength( countEntries ) ;
			CharacterEntry *	pEntries = pFontSet->m_entries.GetArray() ;
			if ( m_cfFont.Read( pEntries, bytesEntries ) < bytesEntries )
			{
				pFontSet->m_entries.FinishArray() ;
				return	sglErrFailed ;
			}
			pFontSet->m_entries.FinishArray() ;
			//
			// 参照テーブル初期化
			//
			size_t	maxUnicode = 0 ;
			size_t	i ;
			for ( i = 0; i < countEntries; i ++ )
			{
				if ( maxUnicode < pEntries[i].codeChar )
				{
					maxUnicode = pEntries[i].codeChar ;
				}
			}
			ESLAssert( maxUnicode < 0x100000 ) ;
			if ( maxUnicode > 0x100000 )
			{
				maxUnicode = 0x100000 ;
			}
			pFontSet->m_aptrEntries.SetLength( maxUnicode ) ;
			//
			for ( i = 0; i < countEntries; i ++ )
			{
				pFontSet->m_aptrEntries.SetAt
					( (pEntries[i].codeChar & 0xFFFFF), pEntries + i ) ;
			}
		}
		else if ( m_cfFont.IsEqualCurrentChunkID( "fontgrph" ) )
		{
			break ;
		}
		m_cfFont.AscendChunk() ;
	}
	return	sglErrSuccess ;
}

// ファイルを閉じる
//////////////////////////////////////////////////////////////////////////////
void SGLBitmapFontLoader::Close( void )
{
	m_arrayFontSets.RemoveAll() ;
	m_pSelFontSet = NULL ;
	//
	m_listLoaded.DeleteAllEntries() ;
	m_countLoaded = 0 ;
	m_cfFont.Close() ;
}

// メモリキャッシュ最大サイズ [bytes] を設定
//////////////////////////////////////////////////////////////////////////////
void SGLBitmapFontLoader::SetCacheLimit( size_t limitCache )
{
	m_limitLoaded = (limitCache >> bufferPageBits) ;
	if ( m_limitLoaded <= 1 )
	{
		m_limitLoaded = 2 ;
	}
}

// メモリキャッシュ最大サイズ [bytes] を取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLBitmapFontLoader::GetCacheLimit( void ) const
{
	return	m_limitLoaded << bufferPageBits ;
}

// メモリをロード
//////////////////////////////////////////////////////////////////////////////
SGLBitmapFontLoader::GrphBufferCache *
	SGLBitmapFontLoader::LoadGrphBuffer( uint32_t addrEntry )
{
	const uint32_t	addrPage = addrEntry >> bufferPageBits ;
	SLinkedListEntry<GrphBufferCache> *
					pCache = m_listLoaded.GetEntryAt( 0 ) ;
	while ( pCache != NULL )
	{
		if ( pCache->addrPage == addrPage )
		{
			m_listLoaded.DetachEntry( pCache ) ;
			m_listLoaded.InsertFirstEntry( pCache ) ;
			return	pCache ;
		}
		pCache = pCache->GetNext() ;
	}
	if ( m_countLoaded >= m_limitLoaded )
	{
		pCache = m_listLoaded.GetLastEntryAt( 0 ) ;
		while ( (pCache != NULL) && (pCache->countRef != 0) )
		{
			pCache = pCache->GetPrev() ;
		}
		if ( (pCache != NULL) && (pCache->countRef == 0) )
		{
			m_listLoaded.DetachEntry( pCache ) ;
			pCache->addrPage = addrPage ;
			m_cfFont.Seek( addrPage << bufferPageBits ) ;
			m_cfFont.Read( &(pCache->memory[0]), bufferPageSize ) ;
			m_listLoaded.InsertFirstEntry( pCache ) ;
			return	pCache ;
		}
	}
	pCache = new SLinkedListEntry<GrphBufferCache> ;
	pCache->addrPage = addrPage ;
	pCache->countRef = 0 ;
	m_cfFont.Seek( addrPage << bufferPageBits ) ;
	m_cfFont.Read( &(pCache->memory[0]), bufferPageSize ) ;
	m_listLoaded.InsertFirstEntry( pCache ) ;
	m_countLoaded ++ ;
	return	pCache ;
}

// メモリをロード＆ロック（解放されないように参照カウンタ＋１）
//////////////////////////////////////////////////////////////////////////////
SGLBitmapFontLoader::GrphBufferCache *
	SGLBitmapFontLoader::LockGrphBuffer( uint32_t addrEntry )
{
	GrphBufferCache *	pCache ;
	m_csSync.Lock() ;
	pCache = LoadGrphBuffer( addrEntry ) ;
	pCache->countRef ++ ;
	m_csSync.Unlock() ;
	return	pCache ;
}

// メモリをアンロック（解放できるように参照カウンタ－１）
//////////////////////////////////////////////////////////////////////////////
void SGLBitmapFontLoader::UnlockGrphBuffer( GrphBufferCache * pCache )
{
	if ( pCache != NULL )
	{
		m_csSync.Lock() ;
		if ( pCache->countRef > 0 )
		{
			pCache->countRef -- ;
		}
		m_csSync.Unlock() ;
	}
}

// FontSet 総数
//////////////////////////////////////////////////////////////////////////////
size_t SGLBitmapFontLoader::GetFontSetCount( void ) const
{
	return	m_arrayFontSets.GetLength() ;
}

// FontSet 取得
//////////////////////////////////////////////////////////////////////////////
SGLBitmapFontLoader::FontSet * SGLBitmapFontLoader::GetFontSetAt( size_t i ) const
{
	return	m_arrayFontSets.GetAt( i ) ;
}

// 最も近いサイズの FontSet を取得
//////////////////////////////////////////////////////////////////////////////
SGLBitmapFontLoader::FontSet *
	SGLBitmapFontLoader::GetNearestFontSet( const SGLFontStyle& style ) const
{
	FontSet *	pNearestFont = NULL ;
	double		minDistance = 1.0e+7 ;
	for ( size_t i = 0; i < m_arrayFontSets.GetLength(); i ++ )
	{
		FontSet *	pFontSet = m_arrayFontSets.GetAt( i ) ;
		ESLAssert( pFontSet != NULL ) ;
		if ( pFontSet != NULL )
		{
			double	fpDistance =
				fabs( (double) style.nSize
						/ (double) pFontSet->m_style.sizeFont - 1.0 ) ;
			if ( fpDistance < minDistance )
			{
				pNearestFont = pFontSet ;
				minDistance = fpDistance ;
			}
		}
	}
	return	pNearestFont ;
}

// フォント画像を取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLBitmapFontLoader::GetFontMetrics
	( SGLBitmapFontLoader::FontSet* pFontSet,
		uint8_t* pbytRasterized, size_t nBufBytes,
				SGLFontMetrics& metrics, wchar_t wch )
{
	if ( pFontSet == NULL )
	{
		return	sglErrFailed ;
	}
	CharacterEntry *	pceChar = pFontSet->m_aptrEntries.GetAt( wch ) ;
	if ( pceChar == NULL )
	{
		return	sglErrFailed ;
	}
	//
	// ヘッダ情報取得
	//
	GrphBufferCache *	pCache = LockGrphBuffer( pceChar->offsetAddr ) ;
	const uint32_t		offsetAddr =
							pceChar->offsetAddr & bufferPageOffsetMask ;
	if ( (pCache == NULL)
		|| (offsetAddr + sizeof(CharacterHeader) > bufferPageSize) )
	{
		UnlockGrphBuffer( pCache ) ;
		return	sglErrFailed ;
	}
	CharacterHeader *	pcgHeader =
			(CharacterHeader*) &(pCache->memory[offsetAddr]) ;
	metrics = pFontSet->m_style.metricsFont ;
	#if	!defined(__COTOPHA__)
	if ( offsetAddr & 0x03 )
	{
		ESLAssert( sizeof(metrics.nWidth) == sizeof(int32_t) ) ;
		ESLAssert( sizeof(pcgHeader->pitchChar) == sizeof(int32_t) ) ;
		memmove( &(metrics.nWidth),
					&(pcgHeader->pitchChar), sizeof(int32_t) ) ;
		ESLAssert( sizeof(metrics.rctExterior) == sizeof(SGLImageRect) ) ;
		ESLAssert( sizeof(pcgHeader->rctExterior) == sizeof(SGLImageRect) ) ;
		memmove( &metrics.rctExterior,
					&(pcgHeader->rctExterior), sizeof(SGLImageRect) ) ;
	}
	else
	#endif
	{
		metrics.nWidth = pcgHeader->pitchChar ;
		metrics.rctExterior = pcgHeader->rctExterior ;
	}
	//
	// 画像情報取得
	//
	SGLError	err = sglErrSuccess ;
	if ( pbytRasterized != NULL )
	{
		size_t	nGrphBytes =
					metrics.rctExterior.w * metrics.rctExterior.h
								* pFontSet->m_style.depthBitmap / 8 ;
		if ( nBufBytes > nGrphBytes )
		{
			nBufBytes = nGrphBytes ;
		}
		uint32_t	addrGrph =
						pceChar->offsetAddr + sizeof(CharacterHeader) ;
		while ( nBufBytes > 0 )
		{
			uint32_t	offsetGrph = addrGrph & bufferPageOffsetMask ;
			size_t		nLeftBytes = bufferPageSize - offsetGrph ;
			if ( nLeftBytes > nBufBytes )
			{
				nLeftBytes = nBufBytes ;
			}
			GrphBufferCache *	pGrphBuf = LockGrphBuffer( addrGrph ) ;
			if ( pGrphBuf == NULL )
			{
				err = sglErrFailed ;
				break ;
			}
			memmove
				( pbytRasterized,
					&(pGrphBuf->memory[offsetGrph]), nLeftBytes ) ;
			//
			UnlockGrphBuffer( pGrphBuf ) ;
			//
			pbytRasterized += nLeftBytes ;
			addrGrph += (uint32_t) nLeftBytes ;
			nBufBytes -= nLeftBytes ;
		}
	}
	UnlockGrphBuffer( pCache ) ;
	return	err ;
}

// リサンプリング
//////////////////////////////////////////////////////////////////////////////
void SGLBitmapFontLoader::ResampleGrayscaleFont
	( uint8_t* pbytDst, const SGLSize& szDst,
		const uint8_t* pbytSrc, const SGLSize& szSrc,
		float x0, float y0, float xz, float yz )
{
	int32_t	fxX0 = eslRoundR32ToInt( x0 * 0x10000 ) ;
	int32_t	fxY0 = eslRoundR32ToInt( y0 * 0x10000 ) ;
	int32_t	fxXZ = eslRoundR32ToInt( xz * 0x10000 ) ;
	int32_t	fxYZ = eslRoundR32ToInt( yz * 0x10000 ) ;
	SGLSize	sizeDst = szDst ;
	SGLSize	sizeSrc = szSrc ;
	int32_t	ys = fxY0 ;
	for ( int32_t y = 0; y < sizeDst.h; y ++ )
	{
		int32_t	xs = fxX0 ;
		int32_t	yi = ys >> 16 ;
		size_t	yd = (ys >> 8) & 0xFF ;
		//
		const uint8_t *	pbytSrc0 = pbytSrc + yi * sizeSrc.w ;
		const uint8_t *	pbytSrc1 = pbytSrc0 + sizeSrc.w ;
		uint32_t		maskY0 = 0xFF, maskY1 = 0xFF ;
		if ( yi < 0 )
		{
			maskY0 = 0 ;
			maskY1 = (yi < -1) ? 0 : 0xFF ;
			yi = 0 ;
			pbytSrc0 = pbytSrc ;
			pbytSrc1 = pbytSrc ;
		}
		else if ( yi >= sizeSrc.h - 1 )
		{
			maskY0 = (yi >= sizeSrc.h) ? 0 : 0xFF ;
			maskY1 = 0 ;
			yi = sizeSrc.h - 1 ;
			pbytSrc0 = pbytSrc + yi * szSrc.w ;
			pbytSrc1 = pbytSrc0 ;
		}
		//
		for ( int32_t x = 0; x < sizeDst.w; x ++ )
		{
			int32_t	xi = xs >> 16 ;
			size_t	xd = (xs >> 8) & 0xFF ;
			//
			uint32_t	px0 = 0, px1 = 0 ;
			if ( (uint32_t) xi < (uint32_t) sizeSrc.w )
			{
				px0 = (pbytSrc0[xi] & maskY0)
						| ((uint32_t)(pbytSrc1[xi] & maskY1) << 16) ;
			}
			xi ++ ;
			if ( (uint32_t) xi < (uint32_t) sizeSrc.w )
			{
				px1 = (pbytSrc0[xi] & maskY0)
						| ((uint32_t)(pbytSrc1[xi] & maskY1) << 16) ;
			}
			//
			px0 = ((px0 * (0x100 - xd)) + (px1 * xd)) & 0xFF00FF00 ;
			//
			*(pbytDst ++) =
				(uint8_t) (((((px0 >> 8) & 0xFF) * (0x100 - yd))
							+ (((px0 >> 24) & 0xFF) * yd)) >> 8) ;
			//
			xs += fxXZ ;
		}
		ys += fxYZ ;
	}
}

// フォントオブジェクト生成
//////////////////////////////////////////////////////////////////////////////
SGLFontObject * SGLBitmapFontLoader::NewFont( const SGLFontStyle& style )
{
	SGLReferenceFont *	pFont = new SGLReferenceFont( this ) ;
	pFont->SetStyle( style ) ;
	return	pFont ;
}

// スタイル設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLBitmapFontLoader::SetStyle( const SGLFontStyle& style )
{
	m_fsSelStyle = style ;
	m_pSelFontSet = GetNearestFontSet( style ) ;
	return	(m_pSelFontSet != NULL) ? sglErrSuccess : sglErrFailed ;
}

// フォント情報取得・ラスタライズ
//////////////////////////////////////////////////////////////////////////////
SGLError SGLBitmapFontLoader::GetMetrics
	( uint8_t* pbytRasterized, size_t nBufBytes,
				SGLFontMetrics& metrics, uint32_t wch )
{
	if ( m_pSelFontSet == NULL )
	{
		return	sglErrFailed ;
	}
	return	GetFontMetrics
		( m_pSelFontSet, pbytRasterized, nBufBytes, metrics, wch ) ;
}


//////////////////////////////////////////////////////////////////////////////
// フォント参照インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLBitmapFontLoader::SGLReferenceFont, SGLFontObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLBitmapFontLoader::SGLReferenceFont::SGLReferenceFont( SGLBitmapFontLoader * pbmFont )
	: m_refFont( pbmFont )
{
	m_pFontSet = NULL ;
}

// フォントオブジェクト生成
//////////////////////////////////////////////////////////////////////////////
SGLFontObject * SGLBitmapFontLoader::SGLReferenceFont::NewFont( const SGLFontStyle& style )
{
	SGLBitmapFontLoader *	pbmFont = m_refFont ;
	if ( pbmFont != NULL )
	{
		SGLReferenceFont *	pFont = new SGLReferenceFont( pbmFont ) ;
		pFont->SetStyle( style ) ;
		return	pFont ;
	}
	else
	{
		SGLFont *	pFont = new SGLFont ;
		pFont->SetStyle( style ) ;
		return	pFont ;
	}
}

// スタイル設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLBitmapFontLoader::SGLReferenceFont::SetStyle( const SGLFontStyle& style )
{
	SGLBitmapFontLoader *	pbmFont = m_refFont ;
	if ( pbmFont == NULL )
	{
		return	sglErrFailed ;
	}
	m_pFontSet = pbmFont->GetNearestFontSet( style ) ;
	m_style = style ;
	if ( m_pFontSet == NULL )
	{
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

// フォント情報取得・ラスタライズ
//////////////////////////////////////////////////////////////////////////////
SGLError SGLBitmapFontLoader::SGLReferenceFont::GetMetrics
	( uint8_t* pbytRasterized, size_t nBufBytes,
				SGLFontMetrics& metrics, uint32_t wch )
{
	SGLBitmapFontLoader *	pbmFont = m_refFont ;
	if ( (pbmFont == NULL) || (m_pFontSet == NULL) )
	{
		return	sglErrFailed ;
	}
	if ( m_pFontSet->m_style.sizeFont == m_style.nSize )
	{
		return	pbmFont->GetFontMetrics
					( m_pFontSet, pbytRasterized, nBufBytes, metrics, wch ) ;
	}
	SGLFontMetrics	mtrOrg ;
	SGLError		err ;
	err = pbmFont->GetFontMetrics( m_pFontSet, NULL, 0, mtrOrg, wch ) ;
	if ( err )
	{
		return	err ;
	}
	int32_t	fxZoom = m_style.nSize * 0x100 / m_pFontSet->m_style.sizeFont ;
	int32_t	fxRZoom = m_pFontSet->m_style.sizeFont * 0x100 / m_style.nSize ;
	int32_t	fxLeft = fxZoom * mtrOrg.rctExterior.x ;
	int32_t	fxTop = fxZoom * mtrOrg.rctExterior.y ;
	int32_t	fxRight = fxZoom * (mtrOrg.rctExterior.x + mtrOrg.rctExterior.w) ;
	int32_t	fxBottom = fxZoom * (mtrOrg.rctExterior.y + mtrOrg.rctExterior.h) ;
	//
	metrics.nFlags = mtrOrg.nFlags ;
	metrics.nAscent = (mtrOrg.nAscent * fxZoom + 0x80) >> 8 ;
	metrics.nDescent = (mtrOrg.nDescent * fxZoom + 0x80) >> 8 ;
	metrics.nLeading = (mtrOrg.nLeading * fxZoom + 0x80) >> 8 ;
	metrics.nWidth = (mtrOrg.nWidth * fxZoom + 0x80) >> 8 ;
	metrics.nHeight = (mtrOrg.nHeight * fxZoom + 0x80) >> 8 ;
	metrics.rctExterior.x = fxLeft >> 8 ;
	metrics.rctExterior.y = fxTop >> 8 ;
	metrics.rctExterior.w = ((fxRight + 0xFF) >> 8) - metrics.rctExterior.x ;
	metrics.rctExterior.h = ((fxBottom + 0xFF) >> 8) - metrics.rctExterior.y ;
	//
	if ( pbytRasterized != NULL )
	{
		if ( nBufBytes < (size_t) (metrics.rctExterior.w * metrics.rctExterior.h) )
		{
			return	sglErrFailed ;
		}
		size_t	areaFont = mtrOrg.rctExterior.w * mtrOrg.rctExterior.h ;
		if ( areaFont > m_bufGrph.GetLength() )
		{
			m_bufGrph.SetLength( areaFont ) ;
		}
		uint8_t *	pbytBuf = m_bufGrph.GetArray() ;
		err = pbmFont->GetFontMetrics
					( m_pFontSet, pbytBuf, areaFont, mtrOrg, wch ) ;
		if ( !err )
		{
			SGLSize	sizeSrc, sizeDst ;
			sizeSrc.w = mtrOrg.rctExterior.w ;
			sizeSrc.h = mtrOrg.rctExterior.h ;
			sizeDst.w = metrics.rctExterior.w ;
			sizeDst.h = metrics.rctExterior.h ;
			//
			float	fpScale = 1.0f / 256.0f ;
			float	fpRZoom = (float) fxRZoom * fpScale ;
			//
			ESLTrace( "resample bitmap font for %dpx.\n", m_style.nSize ) ;
			ResampleGrayscaleFont
				( pbytRasterized, sizeDst,
					pbytBuf, sizeSrc,
					- (float) (fxLeft & 0xFF) * fpScale * fpRZoom,
					- (float) (fxTop & 0xFF) * fpScale * fpRZoom,
					fpRZoom, fpRZoom ) ;
		}
		else
		{
			memset( pbytRasterized, 0, nBufBytes ) ;
		}
		m_bufGrph.FinishArray() ;
	}
	return	sglErrSuccess ;
}

