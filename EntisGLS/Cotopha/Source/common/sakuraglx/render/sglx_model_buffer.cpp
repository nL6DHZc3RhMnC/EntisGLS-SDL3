
#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl2d/sgl_image_conversion.h>
#include <sakuragl/sgl2d/sgl_image_buf_object.h>
#include <sakuragl/sgl2d/sgl_paint_buffer.h>
#include <sakuraglx/sglx3d_render.h>
#include <sakuraglx/render/sglx_model_loader.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// テクスチャ画像ライブラリ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DTextureLibrary, SGLResourceProducer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DTextureLibrary::S3DTextureLibrary( void )
{
}

S3DTextureLibrary::S3DTextureLibrary( const S3DTextureLibrary& lib )
	: m_refParent( lib.m_refParent ), m_aRefLib( lib.m_aRefLib )
{
	AddLibraryFrom( lib ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DTextureLibrary::~S3DTextureLibrary( void )
{
}

// 代入（複製）
//////////////////////////////////////////////////////////////////////////////
const S3DTextureLibrary&
	S3DTextureLibrary::operator = ( const S3DTextureLibrary& lib )
{
	size_t	nCount = lib.m_ssaTextures.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		const SString *		pstrID = lib.m_ssaTextures.GetTagAt( i ) ;
		SSyncReference *	pTxRef = lib.m_ssaTextures.GetAt( i ) ;
		ESLAssert( pstrID != NULL ) ;
		ESLAssert( pTxRef != NULL ) ;
		m_ssaTextures.Add( *pstrID, *pTxRef ) ;
	}
	m_refParent = lib.m_refParent ;
	m_aRefLib = lib.m_aRefLib ;
	return	*this ;
}

// 画像取得
//////////////////////////////////////////////////////////////////////////////
SGLImageObject * S3DTextureLibrary::GetTextureAs
		( const wchar_t * pwszID, bool flagNoRefOther ) const
{
	SSyncReference *	pRef = m_ssaTextures.GetAs( pwszID ) ;
	if ( pRef != NULL )
	{
		SGLImageObject *	pImage =
			ESLTypeCast<SGLImageObject>( pRef->GetReference() ) ;
		if ( pImage != NULL )
		{
			return	pImage ;
		}
	}
	if ( !flagNoRefOther )
	{
		S3DTextureLibrary *	pParent = m_refParent ;
		if ( pParent != NULL )
		{
			SGLImageObject *	pImage = pParent->GetTextureAs( pwszID ) ;
			if ( pImage != NULL )
			{
				return	pImage ;
			}
		}
		SString	strLibName ;
		size_t	iTexID = 0 ;
		for ( size_t i = 0; pwszID[i] != 0; i ++ )
		{
			if ( pwszID[i] == L'#' )
			{
				strLibName = SString( pwszID, (ssize_t) i ) ;
				iTexID = i + 1 ;
				break ;
			}
		}
		for ( size_t i = 0; i < m_aRefLib.GetLength(); i ++ )
		{
			S3DTextureLibrary *	pRef = m_aRefLib.GetAt( i ) ;
			if ( (pRef != NULL)
				&& (strLibName.IsEmpty()
					|| (pRef->GetLibraryName() == strLibName)) )
			{
				SGLImageObject *
					pImage = pRef->GetTextureAs( pwszID + iTexID ) ;
				if ( pImage != NULL )
				{
					return	pImage ;
				}
			}
		}
		for ( size_t i = 0; i < m_aRefLib.GetLength(); i ++ )
		{
			S3DTextureLibrary *	pRef = m_aRefLib.GetAt( i ) ;
			if ( pRef != NULL )
			{
				SGLImageObject *
					pImage = pRef->GetTextureAs( pwszID ) ;
				if ( pImage != NULL )
				{
					return	pImage ;
				}
			}
		}
	}
	return	NULL ;
}

SSystem::SObject * S3DTextureLibrary::GetResourceAs( const wchar_t * pwszID )
{
	return	GetTextureAs( pwszID ) ;
}

// 画像登録
//////////////////////////////////////////////////////////////////////////////
SGLError S3DTextureLibrary::AddTextureAs
		( const wchar_t * pwszID, SGLImageObject * pTexture )
{
	SSyncReference	ref( pTexture ) ;
	m_ssaTextures.Add( pwszID, ref ) ;
	return	sglErrSuccess ;
}

SGLError S3DTextureLibrary::AddSmartTextureAs
		( const wchar_t * pwszID, SGLImageObject * pTexture )
{
	SSyncReference	ref( new SSmartObject( pTexture ) ) ;
	m_ssaTextures.Add( pwszID, ref ) ;
	return	sglErrSuccess ;
}

SGLError S3DTextureLibrary::AddLibraryFrom( const S3DTextureLibrary & lib )
{
	const size_t	nCount = lib.m_ssaTextures.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		const SString *		pstrID = lib.m_ssaTextures.GetTagAt( i ) ;
		SSyncReference *	pRef = lib.m_ssaTextures.GetAt( i ) ;
		if ( pstrID && pRef )
		{
			m_ssaTextures.Add( *pstrID, *pRef ) ;
		}
	}
	return	sglErrSuccess ;
}

// 登録削除
//////////////////////////////////////////////////////////////////////////////
SGLError S3DTextureLibrary::RemoveTextureAs( const wchar_t * pwszID )
{
	m_ssaTextures.RemoveAs( pwszID ) ;
	return	sglErrSuccess ;
}

// 全登録削除
//////////////////////////////////////////////////////////////////////////////
SGLError S3DTextureLibrary::RemoveAllTexture( void )
{
	m_ssaTextures.RemoveAll() ;
	return	sglErrSuccess ;
}

// 名前変更
//////////////////////////////////////////////////////////////////////////////
SGLError S3DTextureLibrary::RenameTextureAs
	( SGLImageObject * pTexture, const wchar_t * pwszID )
{
	ssize_t	i = FindTexturePtr( pTexture ) ;
	if ( i < 0 )
	{
		return	sglErrFailed ;
	}
	SSyncReference *	pRef = m_ssaTextures.GetAt( (size_t) i ) ;
	const SString *		pstrTag = m_ssaTextures.GetTagAt( (size_t) i ) ;
	if ( (pRef == NULL) || (pstrTag == NULL) )
	{
		return	sglErrFailed ;
	}
	SObject *	pObj = pRef->GetReference() ;
	SReference	refTemp( pObj ) ;
	pRef->ReleaseReference() ;
	//
	m_ssaTextures.RemoveAt( (size_t) i ) ;
	m_ssaTextures.Add( pwszID, pObj ) ;
	//
	return	sglErrSuccess ;
}

// 画像検索
//////////////////////////////////////////////////////////////////////////////
ssize_t S3DTextureLibrary::FindTexturePtr( SGLImageObject * pImage ) const
{
	size_t	nCount = m_ssaTextures.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		SSyncReference *	pRef = m_ssaTextures.GetAt( i ) ;
		if ( pRef != NULL )
		{
			if ( ESLTypeCast<SGLImageObject>( pRef->GetReference() ) == pImage )
			{
				return	(ssize_t) i ;
			}
		}
	}
	S3DTextureLibrary *	pParent = m_refParent ;
	if ( pParent != NULL )
	{
		ssize_t	iImage = pParent->FindTexturePtr( pImage ) ;
		if ( iImage >= 0 )
		{
			return	iImage + (ssize_t) nCount ;
		}
		nCount += pParent->GetTextureCount() ;
	}
	for ( size_t i = 0; i < m_aRefLib.GetLength(); i ++ )
	{
		S3DTextureLibrary *	pRef = m_aRefLib.GetAt( i ) ;
		if ( pRef != NULL )
		{
			ssize_t	iImage = pRef->FindTexturePtr( pImage ) ;
			if ( iImage >= 0 )
			{
				return	iImage + (ssize_t) nCount ;
			}
			nCount += pRef->GetTextureCount() ;
		}
	}
	return	-1 ;
}

// 登録数取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DTextureLibrary::GetTextureCount( void ) const
{
	size_t	nCount = m_ssaTextures.GetLength() ;
	//
	S3DTextureLibrary *	pParent = m_refParent ;
	if ( pParent != NULL )
	{
		nCount += pParent->GetTextureCount() ;
	}
	//
	for ( size_t i = 0; i < m_aRefLib.GetLength(); i ++ )
	{
		S3DTextureLibrary *	pRef = m_aRefLib.GetAt( i ) ;
		if ( pRef != NULL )
		{
			nCount += pRef->GetTextureCount() ;
		}
	}
	return	nCount ;
}

// 登録名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DTextureLibrary::GetTextureIdentityAt( size_t i ) const
{
	if ( i < m_ssaTextures.GetLength() )
	{
		const SString *	pstrID = m_ssaTextures.GetTagAt( i ) ;
		if ( pstrID != NULL )
		{
			return	*pstrID ;
		}
		return	NULL ;
	}
	i -= m_ssaTextures.GetLength() ;
	//
	S3DTextureLibrary *	pParent = m_refParent ;
	if ( pParent != NULL )
	{
		size_t	nCount = pParent->GetTextureCount() ;
		if ( i < nCount )
		{
			return	pParent->GetTextureIdentityAt( i ) ;
		}
		i -= nCount ;
	}
	size_t	iRefIndex = i ;
	for ( size_t i = 0; i < m_aRefLib.GetLength(); i ++ )
	{
		S3DTextureLibrary *	pRef = m_aRefLib.GetAt( i ) ;
		if ( pRef != NULL )
		{
			size_t	nCount = pRef->GetTextureCount() ;
			if ( iRefIndex < nCount )
			{
				return	pRef->GetTextureIdentityAt( iRefIndex ) ;
			}
			iRefIndex -= nCount ;
		}
	}
	return	NULL ;
}

const wchar_t * S3DTextureLibrary::GetTextureIdentityOf
								( SGLImageObject * pImage ) const
{
	ssize_t	i = FindTexturePtr( pImage ) ;
	if ( i >= 0 )
	{
		return	GetTextureIdentityAt( (size_t) i ) ;
	}
	return	NULL ;
}

// 表面属性取得
//////////////////////////////////////////////////////////////////////////////
SGLImageObject * S3DTextureLibrary::GetTextureAt( size_t i ) const
{
	if ( i < m_ssaTextures.GetLength() )
	{
		SSyncReference *	pRef = m_ssaTextures.GetAt( i ) ;
		if ( pRef != NULL )
		{
			return	ESLTypeCast<SGLImageObject>( pRef->GetReference() ) ;
		}
		return	NULL ;
	}
	i -= m_ssaTextures.GetLength() ;
	//
	S3DTextureLibrary *	pParent = m_refParent ;
	if ( pParent != NULL )
	{
		size_t	nCount = pParent->GetTextureCount() ;
		if ( i < nCount )
		{
			return	pParent->GetTextureAt( i ) ;
		}
		i -= nCount ;
	}
	for ( size_t i = 0; i < m_aRefLib.GetLength(); i ++ )
	{
		S3DTextureLibrary *	pRef = m_aRefLib.GetAt( i ) ;
		if ( pRef != NULL )
		{
			size_t	nCount = pRef->GetTextureCount() ;
			if ( i < nCount )
			{
				return	pRef->GetTextureAt( i ) ;
			}
			i -= nCount ;
		}
	}
	return	NULL ;
}

// ライブラリ名
//////////////////////////////////////////////////////////////////////////////
const SSystem::SString& S3DTextureLibrary::GetLibraryName( void ) const
{
	return	m_strName ;
}

void S3DTextureLibrary::SetLibraryName( const wchar_t * pwszName )
{
	m_strName = pwszName ;
}

// 参照ライブラリを設定する
//////////////////////////////////////////////////////////////////////////////
void S3DTextureLibrary::SetParentLibrary( S3DTextureLibrary * pLib )
{
	m_refParent = pLib ;
}

size_t S3DTextureLibrary::AddReferenceLibrary( S3DTextureLibrary * pLib )
{
	ssize_t	i = m_aRefLib.FindPtr( pLib ) ;
	if ( i >= 0 )
	{
		return	(size_t) i ;
	}
	return	m_aRefLib.Add( pLib ) ;
}

// 参照ライブラリを取得する
//////////////////////////////////////////////////////////////////////////////
S3DTextureLibrary * S3DTextureLibrary::GetParentLibrary( void ) const
{
	return	m_refParent ;
}

// 参照ライブラリを取得する
//////////////////////////////////////////////////////////////////////////////
size_t S3DTextureLibrary::GetReferenceLibraryCount( void ) const
{
	return	m_aRefLib.GetLength() ;
}

S3DTextureLibrary * S3DTextureLibrary::GetReferenceLibraryAt( size_t i ) const
{
	return	m_aRefLib.GetAt( i ) ;
}

void S3DTextureLibrary::DetachReferenceLibraryAt( size_t i )
{
	m_aRefLib.DetachAt( i ) ;
}

void S3DTextureLibrary::DetachReferenceLibraryOf( S3DTextureLibrary * pLib )
{
	m_aRefLib.DetachAt( (size_t) m_aRefLib.FindPtr( pLib ) ) ;
}

void S3DTextureLibrary::DetachAllReferenceLibrarys( void )
{
	m_aRefLib.DetachAll() ;
}

// 画像名正規化（未使用であることを保証）
//////////////////////////////////////////////////////////////////////////////
const SString& S3DTextureLibrary::NormalizeIdentity( SString& strID ) const
{
	if ( m_ssaTextures.GetAs( strID ) == NULL )
	{
		return	strID ;
	}
	SString	strBase = strID ;
	for ( int i = 1; i < 0x10000; i ++ )
	{
		strID = strBase + SString( i ) ;
		if ( m_ssaTextures.GetAs( strID ) == NULL )
		{
			break ;
		}
	}
	return	strID ;
}

// 解放
//////////////////////////////////////////////////////////////////////////////
void S3DTextureLibrary::Release( void )
{
	m_ssaTextures.RemoveAll() ;
	m_refParent.SetReference( NULL ) ;
	m_aRefLib.RemoveAll() ;
}

// 圧縮フラグ付きのテクスチャを事前に圧縮する
//////////////////////////////////////////////////////////////////////////////
SGLError S3DTextureLibrary::MakeCompressedTexture( uint32_t format, uint64_t nFlags )
{
	if ( !(format & formatImageFlagS3TC) )
	{
		return	sglErrInvalidParam ;
	}
	size_t	nCount = GetTextureCount() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		SGLImageObject *	pImage = GetTextureAt( i ) ;
		if ( pImage != NULL )
		{
			uint32_t	fmtOrg = pImage->GetImageFormat() ;
			if ( !(fmtOrg & formatImageFlagS3TC)
				&& (pImage->GetBufferFlags()
						& SGLImageObject::bufferCompressedTexture) )
			{
				pImage->NormalizeFormat( format | (fmtOrg & formatImageFlagAlpha) ) ;
			}
		}
	}
	return	sglErrSuccess ;
}

// デバイスメモリ上に準備する
//////////////////////////////////////////////////////////////////////////////
SGLError S3DTextureLibrary::CommitToDevice
	( S3DRenderDevice * pDev, int64_t msecTimeout )
{
	size_t	nCount = GetTextureCount() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		SGLImageObject *	pImage = GetTextureAt( i ) ;
		if ( pImage != NULL )
		{
			pDev->CommitDeviceImage( pImage, msecTimeout ) ;
		}
	}
	return	sglErrSuccess ;
}

// デバイス上リソースを開放する
//////////////////////////////////////////////////////////////////////////////
SGLError S3DTextureLibrary::ReleaseAllDeviceResources( void )
{
	size_t	nCount = GetTextureCount() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		SGLImageObject *	pImage = GetTextureAt( i ) ;
		if ( pImage != NULL )
		{
			pImage->DeleteAllImageObjects() ;
		}
	}
	return	sglErrSuccess ;
}

// バンプマッピング用画像から法線画像生成
//////////////////////////////////////////////////////////////////////////////
SGLImageObject *
	S3DTextureLibrary::MakeNormalMapFromBumpMap
		( SGLImageObject * pNormal,
			SGLImageObject * pTexture, float32_t fpHeight )
{
	//
	// グレイスケールへ変換
	//
	SGLImageBuffer	imgSrc ;
	imgSrc.ptrBuffer =
		pTexture->LockBuffer( imgSrc, SGLImageObject::lockRead ) ;
	//
	SGLImageBuffer *	pimgGray = NULL ;
	SGLSmartImage		imgTempGray ;
	imgTempGray.CreateImage
		( imgSrc.width, imgSrc.height, formatImageGray, 8 ) ;
	//
	SGLImageBuffer	imgGray ;
	imgGray.ptrBuffer = imgTempGray.LockBuffer( imgGray ) ;
	//
	sglConvertImageBuffer( imgGray, imgSrc ) ;
	//
	pTexture->UnlockBuffer( SGLImageObject::lockRead ) ;
	//
	// 法線マッピング画像へ変換
	//
	pNormal->CreateImage
		( imgSrc.width, imgSrc.height, formatImageARGB, 32 ) ;
	//
	SGLImageBuffer	imgNormal ;
	imgNormal.ptrBuffer = pNormal->LockBuffer( imgNormal ) ;
	//
	S3DVector	vNormal, vTempX( 256, 0, 0 ), vTempY( 0, 256, 0 ) ;
	uint8_t *	pbytLastLine =
		imgGray.ptrBuffer + (imgGray.pitchLine * (imgSrc.height - 1)) ;
	uint8_t *	pbytSrcLine = imgGray.ptrBuffer ;
	uint8_t *	pbytDstLine = imgNormal.ptrBuffer ;
	for ( size_t y = 0; y < imgSrc.height; y ++ )
	{
		uint8_t *	pbytSrcLeft = pbytSrcLine + (imgSrc.width - 1) ;
		uint8_t *	pbytDstNext = pbytDstLine ;
		for ( size_t x = 0; x < imgSrc.width; x ++ )
		{
			vTempX.z = ((int) pbytSrcLine[x] - *pbytSrcLeft) * fpHeight ;
			vTempY.z = ((int) pbytSrcLine[x] - pbytLastLine[x]) * fpHeight ;
			vNormal = vTempX * vTempY ;
			vNormal.Normalize() ;
			vNormal *= 127.0f ;
			//
			pbytDstNext[2] = 
				(uint8_t) esl_clampi( eslRoundR32ToInt( vNormal.x ) + 0x80, 0, 255 ) ;
			pbytDstNext[1] =
				(uint8_t) esl_clampi( eslRoundR32ToInt( vNormal.y ) + 0x80, 0, 255 ) ;
			pbytDstNext[0] =
				(uint8_t) esl_clampi( eslRoundR32ToInt( vNormal.z ) + 0x80, 0, 255 ) ;
			pbytDstNext[3] = 0xFF ;
			pbytDstNext += 4 ;
			pbytSrcLeft = pbytSrcLine + x ;
		}
		pbytLastLine = pbytSrcLine ;
		pbytSrcLine += imgGray.pitchLine ;
		pbytDstLine += imgNormal.pitchLine ;
	}
	//
	pNormal->UnlockBuffer() ;
	imgTempGray.UnlockBuffer() ;
	//
	return	pNormal ;
}

// 法線画像をグレイスケールへ戻す（※不可逆）
//////////////////////////////////////////////////////////////////////////////
SGLImageObject * S3DTextureLibrary::RestoreBumpTexture
	( SGLImageObject * pGrayscale, SGLImageObject * pNormal )
{
	SGLImageBuffer	imgNormal ;
	imgNormal.ptrBuffer =
		pNormal->LockBuffer( imgNormal, SGLImageObject::lockRead ) ;
	if ( imgNormal.depth < 24 )
	{
		return	NULL ;
	}
	//
	// 誤差計算
	//
	SArray<float32_t>	bufHError ;
	SArray<float32_t>	bufVError ;
	float32_t *	pHError = bufHError.GetArray( imgNormal.height ) ;
	float32_t *	pVError = bufVError.GetArray( imgNormal.width ) ;
	uint8_t *	pbytSrcLine = imgNormal.ptrBuffer ;
	S3DVector	vNormal ;
	float32_t	d ;
	size_t		x, y ;
	for ( y = 0; y < imgNormal.height; y ++ )
	{
		uint8_t *	pbytSrcNext = pbytSrcLine ;
		float32_t	e = 0.0f ;
		for ( x = 0; x < imgNormal.width; x ++ )
		{
			vNormal.x = (float32_t) ((int) pbytSrcNext[2] - 0x80) ;
			vNormal.y = (float32_t) - ((int) pbytSrcNext[1] - 0x80) ;
			vNormal.z = (float32_t) ((int) pbytSrcNext[0] - 0x80) ;
			vNormal.Normalize() ;
			//
			d = -1.0f / vNormal.z ;
			e += vNormal.x * d ;
			pVError[x] += vNormal.y * d ;
			//
			pbytSrcNext += imgNormal.pitchPixel ;
		}
		pHError[y] = e / (float32_t) imgNormal.width ;
		//
		pbytSrcLine += imgNormal.pitchLine ;
	}
	d = 1.0f / (float32_t) imgNormal.height ;
	for ( x = 0; x < imgNormal.width; x ++ )
	{
		pVError[x] *= d ;
	}
	//
	// 標高計算
	//
	SArray<float32_t>	bufHeight1 ;
	SArray<float32_t>	bufHeight2 ;
	size_t		nPixelCount = imgNormal.width * imgNormal.height ;
	float32_t *	pHeight1 = bufHeight1.GetArray( nPixelCount ) ;
	float32_t *	pHeight2 = bufHeight2.GetArray( nPixelCount ) ;
	//
	float32_t	dx, dy ;
	pbytSrcLine = imgNormal.ptrBuffer ;
	for ( y = 0; y < imgNormal.height; y ++ )
	{
		uint8_t *	pbytSrcNext = pbytSrcLine ;
		float32_t *	pBaseHeight2 = pHeight2 ;
		float32_t *	pNextHeight2 =
						bufHeight2.GetArray()
							+ ((y + 1) % imgNormal.height)
											* imgNormal.width ;
		float32_t	ey = pHError[y] ;
		float32_t	hx = pHeight2[0] ;
		if ( y == 0 )
		{
			pBaseHeight2 = pHeight1 ;
		}
		for ( x = 0; x < imgNormal.width; x ++ )
		{
			vNormal.x = (float32_t) ((int) pbytSrcNext[2] - 0x80) ;
			vNormal.y = (float32_t) - ((int) pbytSrcNext[1] - 0x80) ;
			vNormal.z = (float32_t) ((int) pbytSrcNext[0] - 0x80) ;
			vNormal.Normalize() ;
			//
			size_t	xNext = x + 1 ;
			if ( xNext >= imgNormal.width )
			{
				xNext = 0 ;
			}
			d = -1.0f / vNormal.z ;
			dx = vNormal.x * d - ey ;
			dy = vNormal.y * d - pVError[x] ;
			//
			hx += dx ;
			pHeight1[xNext] = hx ;
			pHeight2[x] = pBaseHeight2[x] + dy ;
			//
			pbytSrcNext += imgNormal.pitchPixel ;
		}
		//
		pHeight1 += imgNormal.width ;
		pHeight2 += imgNormal.width ;
		pbytSrcLine += imgNormal.pitchLine ;
	}
	bufHError.FinishArray() ;
	bufVError.FinishArray() ;
	bufHeight1.FinishArray() ;
	bufHeight2.FinishArray() ;
	//
	// 正規化
	//
	float32_t	hMin, hMax ;
	size_t		i ;
	pHeight1 = bufHeight1.GetArray() ;
	pHeight2 = bufHeight2.GetArray() ;
	for ( i = 0; i < nPixelCount; i ++ )
	{
		pHeight1[i] = (pHeight1[i] + pHeight2[i]) * 0.5f ;
	}
	hMin = pHeight1[0] ;
	hMax = pHeight1[0] ;
	for ( i = 0; i < nPixelCount; i ++ )
	{
		hMin = esl_fminf( hMin, pHeight1[i] ) ;
		hMax = esl_fmaxf( hMax, pHeight1[i] ) ;
	}
	d = 0.0f ;
	if ( hMax > hMin )
	{
		d = 255.0f / (hMax - hMin) ;
	}
	for ( i = 0; i < nPixelCount; i ++ )
	{
		pHeight1[i] = (pHeight1[i] - hMin) * d ;
	}
	//
	// 出力
	//
	SGLImageBuffer	imgGray ;
	pGrayscale->CreateImage
		( imgNormal.width, imgNormal.height, formatImageGray, 8 ) ;
	imgGray.ptrBuffer = pGrayscale->LockBuffer( imgGray ) ;
	//
	uint8_t *	pbytDstLine = imgGray.ptrBuffer ;
	for ( y = 0; y < imgNormal.height; y ++ )
	{
		for ( x = 0; x < imgNormal.width; x ++ )
		{
			pbytDstLine[x] =
				(uint8_t) esl_clampi
						( eslRoundR32ToInt( pHeight1[x] ), 0, 0xFF ) ;
		}
		pHeight1 += imgNormal.width ;
		pbytDstLine += imgGray.pitchLine ;
	}
	//
	bufHeight1.FinishArray() ;
	bufHeight2.FinishArray() ;
	pNormal->UnlockBuffer() ;
	pGrayscale->UnlockBuffer() ;
	//
	return	pGrayscale ;
}

// キューブマップ画像からパノラマ画像を生成
//////////////////////////////////////////////////////////////////////////////
SGLError S3DTextureLibrary::MakePanoramaImageFromCubeMap
	( SGLImageObject& imgPanorama,
		int nPanoramaWidth, int nPanoramaHeight, SGLImageObject& imgCubeMap )
{
	//
	// キューブマップ画像情報取得
	//
	SSmartPointer<SGLImageObject>	pCubeMap[6] ;
	SGLImageSmartBuffer				bufLocker[6] ;
	SGLImageBuffer					bufCubeMap[6] ;
	SGLImageSampler					smpCubeMap[6] ;
	for ( int i = 0; i < 6; i ++ )
	{
		pCubeMap[i] = imgCubeMap.NewReference( NULL, (ssize_t) i ) ;
		bufLocker[i].Lock( bufCubeMap[i], pCubeMap[i], SGLImageObject::lockRead ) ;
		if ( (bufCubeMap[i].ptrBuffer == NULL)
			|| (bufCubeMap[i].depth != 32) )
		{
			return	sglErrFailed ;
		}
		smpCubeMap[i].Prepare( bufCubeMap[i], bufCubeMap[i].ptrBuffer ) ;
	}
	//
	// バッファ生成
	//
	imgPanorama.CreateImage
		( (uint32_t) nPanoramaWidth,
			(uint32_t) nPanoramaHeight, formatImageRGB, 32 ) ;
	//
	SGLImageBuffer		bufPanorama ;
	SGLImageSmartBuffer	bufPanoramaLocker
			( bufPanorama, &imgPanorama, SGLImageObject::lockWrite ) ;
	//
	// サンプリング
	//
	const float32_t	fpHalfSqrt2 = (float32_t) (sqrt( 2.0 ) * 0.5) ;
	const float32_t	rcpSqrt2 = (float32_t) (1.0 / sqrt( 2.0 )) ;
	const int		wPanorama = nPanoramaWidth ;
	const int		hPanorama = nPanoramaHeight ;
	//
	for ( int y = 0; y < hPanorama; y ++ )
	{
		double		radY = ((double) y + 0.5) / hPanorama * PI ;
		float32_t	cosY = (float32_t) - cos( radY ) ;
		float32_t	sinY = (float32_t) sin( radY ) ;
		//
		SGLPalette *	ppxLine =
			(SGLPalette*) (bufPanorama.ptrBuffer + y * bufPanorama.pitchLine) ;
		//
		for ( int x = 0; x < wPanorama; x ++ )
		{
			double		radX = ((double) x + 0.5) / wPanorama * (2.0 * PI) ;
			float32_t	cosX = (float32_t) - cos( radX ) ;
			float32_t	sinX = (float32_t) sin( radX ) ;
			//
			S3DVector	v( cosX * sinY, cosY, sinX * sinY ) ;
			int			iCubeFace ;
			float32_t	xSample, ySample, zSample ;
			if ( cosX < - rcpSqrt2 )
			{
				iCubeFace = SGLImageObject::cubemapNegativeX ;
				xSample = v.z ;
				ySample = v.y ;
				zSample = - v.x ;
			}
			else if ( cosX > rcpSqrt2 )
			{
				iCubeFace = SGLImageObject::cubemapPositiveX ;
				xSample = - v.z ;
				ySample = v.y ;
				zSample = v.x ;
			}
			else if ( sinX < 0 )
			{
				iCubeFace = SGLImageObject::cubemapNegativeZ ;
				xSample = - v.x ;
				ySample = v.y ;
				zSample = - v.z ;
			}
			else
			{
				iCubeFace = SGLImageObject::cubemapPositiveZ ;
				xSample = v.x ;
				ySample = v.y ;
				zSample = v.z ;
			}
			ESLAssert( zSample >= 0.0f ) ;
			float32_t	r = 0.5f / zSample ;
			if ( ySample * r < -0.5f )
			{
				iCubeFace = SGLImageObject::cubemapNegativeY ;
				xSample = v.x ;
				ySample = v.z ;
				zSample = - v.y ;
				r = 0.5f / zSample ;
			}
			else if ( ySample * r > 0.5f )
			{
				iCubeFace = SGLImageObject::cubemapPositiveY ;
				xSample = v.x ;
				ySample = - v.z ;
				zSample = v.y ;
				r = 0.5f / zSample ;
			}
			xSample = (xSample * r + 0.5f)
							* (float32_t) bufCubeMap[iCubeFace].width ;
			ySample = (ySample * r + 0.5f)
							* (float32_t) bufCubeMap[iCubeFace].height ;
			//
			ppxLine[x] = smpCubeMap[iCubeFace].ClampedBilinear( xSample, ySample ) ;
		}
	}
	return	sglErrSuccess ;
}

// パノラマ画像から環境マッピング（球マップ）画像を生成
//////////////////////////////////////////////////////////////////////////////
SGLError S3DTextureLibrary::MakeSphereMapFromPanoramaImage
	( SGLImageObject& imgSphereMap,
		int nTextureSize, double degHRotAngle, SGLImageObject& imgPanorama )
{
	//
	// パノラマ画像情報取得
	//
	SGLImageInfo	infPanorama ;
	const uint8_t *	pbytPanorama =
		imgPanorama.LockBuffer( infPanorama, SGLImageObject::lockRead ) ;
	if ( pbytPanorama == NULL )
	{
		return	sglErrFailed ;
	}
	if ( infPanorama.depth != 32 )
	{
		imgPanorama.UnlockBuffer( SGLImageObject::lockRead ) ;
		return	sglErrFailed ;
	}
	//
	// バッファ生成
	//
	imgSphereMap.CreateImage
		( (uint32_t) nTextureSize,
			(uint32_t) (nTextureSize * 2), formatImageRGB, 32 ) ;
	//
	SGLImageInfo	infSphereMap ;
	uint8_t *		pbytSphereMap =
		imgSphereMap.LockBuffer( infSphereMap, SGLImageObject::lockWrite ) ;
	//
	// サンプリング用行列
	//
	SGLAffine	affineHRot( 1, 0, 0, 0, 1, 0 ) ;
	affineHRot.SetRotation( degHRotAngle * PI / 180.0 ) ;
	//
	// サンプリング
	//
	SGLImageSampler	smpPanorama( infPanorama, pbytPanorama ) ;
	//
	float32_t	wPanorama = ((float32_t) infPanorama.width - 1.0f) * 0.5f ;
	float32_t	hPanorama = ((float32_t) infPanorama.height - 1.0f) * 0.5f ;
	float32_t	fpHalfSize = (float32_t) (nTextureSize - 1) * 0.5f ;
	float32_t	fpRcpHalfSize = 1.0f / fpHalfSize ;
	//
	S3DVector	vSampleBase ;
	float32_t	xy, z ;
	const int	hTexture = nTextureSize * 2 ;
	for ( int y = 0; y < hTexture; y ++ )
	{
		if ( y < nTextureSize )
		{
			vSampleBase.y = ((float32_t) y - fpHalfSize) * fpRcpHalfSize ;
			vSampleBase.z = -1.0f ;
		}
		else
		{
			vSampleBase.y =
				((float32_t) (y - nTextureSize) - fpHalfSize) * fpRcpHalfSize ;
			vSampleBase.z = 1.0f ;
		}
		SGLPalette *	ppxDstLine =
			(SGLPalette*) (pbytSphereMap + y *infSphereMap.pitchLine) ;
		//
		for ( int x = 0; x < nTextureSize; x ++ )
		{
			vSampleBase.x = ((float32_t) x - fpHalfSize) * fpRcpHalfSize ;
			xy = (float32_t) sqrt( vSampleBase.x * vSampleBase.x
									+ vSampleBase.y * vSampleBase.y ) ;
			z = esl_fminf( xy, 1.0f ) ;
			z = (float32_t) sqrt( 1.0f - z * z ) * vSampleBase.z ;
			z = (float32_t) (asin( z ) * (2.0 / PI)) ;
			//
			float32_t	xSample = 0.0f ;
			float32_t	ySample = (z + 1.0f) * hPanorama ;
			if ( xy != 0.0f )
			{
				S2DVector	v = affineHRot * S2DVector( vSampleBase.x, vSampleBase.y ) ;
				double	rx = atan2( v.y, v.x ) / PI + 1.0 ;
				xSample = (float32_t) rx * wPanorama ;
			}
			ppxDstLine[x] = smpPanorama.ClampedBilinear( xSample, ySample ) ;
		}
	}
	//
	imgSphereMap.UnlockBuffer( SGLImageObject::lockWrite ) ;
	imgPanorama.UnlockBuffer( SGLImageObject::lockRead ) ;
	//
	return	sglErrSuccess ;
}



//////////////////////////////////////////////////////////////////////////////
// 表面属性ライブラリ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DMaterialLibrary, SObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DMaterialLibrary::S3DMaterialLibrary( void )
{
}

S3DMaterialLibrary::S3DMaterialLibrary( const S3DMaterialLibrary& lib )
	: m_refParent( lib.m_refParent )
{
	AddLibraryFrom( lib ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DMaterialLibrary::~S3DMaterialLibrary( void )
{
}

// 代入（複製）
//////////////////////////////////////////////////////////////////////////////
const S3DMaterialLibrary&
	S3DMaterialLibrary::operator = ( const S3DMaterialLibrary& lib )
{
	size_t	nCount = lib.m_ssaMaterials.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		const SString *		pstrID = lib.m_ssaMaterials.GetTagAt( i ) ;
		SSyncReference *	pTxRef = lib.m_ssaMaterials.GetAt( i ) ;
		ESLAssert( pstrID != NULL ) ;
		ESLAssert( pTxRef != NULL ) ;
		m_ssaMaterials.Add( *pstrID, *pTxRef ) ;
	}
	m_refParent = lib.m_refParent ;
	return	*this ;
}

// ファイル読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError S3DMaterialLibrary::LoadLibraryXML
	( const wchar_t * pwszFilePath,
		const S3DTextureLibraryReferencer& libTexture )
{
	SXMLDocument	xmlDoc ;
	if ( xmlDoc.LoadDocument( pwszFilePath, xmlDoc ) )
	{
		return	sglErrFailed ;
	}
	SXMLDocument *	pxmlMaterials = xmlDoc.GetElementTagAs( L"materials" ) ;
	if ( pxmlMaterials == NULL )
	{
		return	sglErrFailed ;
	}
	return	ParseXML( *pxmlMaterials, libTexture ) ;
}

// XML デシリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError S3DMaterialLibrary::ParseXML
	( const SSystem::SXMLDocument & xmlMaterials,
				const S3DTextureLibraryReferencer& libTexture )
{
	for ( size_t i = 0; i < xmlMaterials.GetElementsCount(); i ++ )
	{
		SXMLDocument *	pxmlTag = xmlMaterials.GetElementAt( i ) ;
		ESLAssert( pxmlTag != NULL ) ;
		if ( pxmlTag->GetTag() != L"material" )
		{
			continue ;
		}
		SString	strID = pxmlTag->GetAttrStringAs( L"id" ) ;
		if ( strID.IsEmpty() )
		{
			continue ;
		}
		S3DMaterial *	pMaterial = new S3DMaterial ;
		AddSmartMaterialAs( strID, pMaterial ) ;
		pMaterial->ParseXML( *pxmlTag, libTexture ) ;
	}
	return	sglErrSuccess ;
}

// ファイル書き出し
//////////////////////////////////////////////////////////////////////////////
SGLError S3DMaterialLibrary::SaveLibraryXML
	( const wchar_t * pwszFilePath,
		const S3DTextureLibraryReferencer& libTexture )
{
	SXMLDocument	xmlMaterials ;
	FormatXML( xmlMaterials, libTexture ) ;
	//
	xmlMaterials.SetTag( L"materials" ) ;
	return	(SGLError) xmlMaterials.SaveDocument( pwszFilePath ) ;
}

// XML シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError S3DMaterialLibrary::FormatXML
	( SSystem::SXMLDocument & xmlMaterials,
			const S3DTextureLibraryReferencer& libTexture )
{
	for ( size_t i = 0; i < GetMaterialCount(); i ++ )
	{
		const wchar_t *	pwszID = GetMaterialIdentityAt( i ) ;
		S3DMaterial *	pMaterial = GetMaterialAt( i ) ;
		ESLAssert( pwszID != NULL ) ;
		ESLAssert( pMaterial != NULL ) ;
		if ( (pwszID == NULL) || (pMaterial == NULL) )
		{
			continue ;
		}
		SXMLDocument *	pxmlTag = new SXMLDocument ;
		pxmlTag->SetTag( L"material" ) ;
		pxmlTag->SetAttributeAs( L"id", pwszID ) ;
		xmlMaterials.AddElement( pxmlTag ) ;
		//
		pMaterial->FormatXML( *pxmlTag, libTexture ) ;
	}
	return	sglErrSuccess ;
}

// ローカルテクスチャライブラリ設定
//////////////////////////////////////////////////////////////////////////////
void S3DMaterialLibrary::AttachLocalTextureLibrary( S3DTextureLibrary * pTxtLib )
{
	m_refLocalTexture.SetReference( pTxtLib ) ;
}

// 全テクスチャ参照更新
//////////////////////////////////////////////////////////////////////////////
void S3DMaterialLibrary::UpdateAllTextureReference
	( const S3DTextureLibraryReferencer& libTexture )
{
	if ( m_refLocalTexture.GetReference() == NULL )
	{
		for ( size_t i = 0; i < GetMaterialCount(); i ++ )
		{
			S3DMaterial *	pMaterial = GetMaterialAt( i ) ;
			ESLAssert( pMaterial != NULL ) ;
			if ( pMaterial != NULL )
			{
				pMaterial->UpdateTextureReference( libTexture ) ;
			}
		}
	}
	for ( size_t i = 0; i < m_aRefLib.GetLength(); i ++ )
	{
		S3DMaterialLibrary *	pRefLib = m_aRefLib.GetAt( i ) ;
		if ( pRefLib != NULL )
		{
			pRefLib->UpdateAllTextureReference( libTexture ) ;
		}
	}
}

// 解放
//////////////////////////////////////////////////////////////////////////////
void S3DMaterialLibrary::Release( void )
{
	m_ssaMaterials.RemoveAll() ;
	m_refParent.SetReference( NULL ) ;
	m_aRefLib.RemoveAll() ;
}

// 画像取得
//////////////////////////////////////////////////////////////////////////////
S3DMaterial * S3DMaterialLibrary::GetMaterialAs
		( const wchar_t * pwszID, bool flagNoRefOther ) const
{
	SSyncReference *	pRef = m_ssaMaterials.GetAs( pwszID ) ;
	if ( pRef != NULL )
	{
		S3DMaterial *	pMaterial =
			ESLTypeCast<S3DMaterial>( pRef->GetReference() ) ;
		if ( pMaterial != NULL )
		{
			return	pMaterial ;
		}
	}
	if ( !flagNoRefOther )
	{
		S3DMaterialLibrary *	pParent = m_refParent ;
		if ( pParent != NULL )
		{
			S3DMaterial *	pMaterial = pParent->GetMaterialAs( pwszID ) ;
			if ( pMaterial != NULL )
			{
				return	pMaterial ;
			}
		}
		SString	strLibName ;
		size_t	iMaterialID = 0 ;
		if ( pwszID != nullptr )
		{
			for ( size_t i = 0; pwszID[i] != 0; i ++ )
			{
				if ( pwszID[i] == L'#' )
				{
					strLibName = SString( pwszID, (ssize_t) i ) ;
					iMaterialID = i + 1 ;
					break ;
				}
			}
		}
		for ( size_t i = 0; i < m_aRefLib.GetLength(); i ++ )
		{
			S3DMaterialLibrary *	pRef = m_aRefLib.GetAt( i ) ;
			if ( (pRef != NULL)
				&& (strLibName.IsEmpty()
					|| (pRef->GetLibraryName() == strLibName)) )
			{
				S3DMaterial *	pMaterial = pRef->GetMaterialAs( pwszID + iMaterialID ) ;
				if ( pMaterial != NULL )
				{
					return	pMaterial ;
				}
			}
		}
		for ( size_t i = 0; i < m_aRefLib.GetLength(); i ++ )
		{
			S3DMaterialLibrary *	pRef = m_aRefLib.GetAt( i ) ;
			if ( pRef != NULL )
			{
				S3DMaterial *	pMaterial = pRef->GetMaterialAs( pwszID ) ;
				if ( pMaterial != NULL )
				{
					return	pMaterial ;
				}
			}
		}
	}
	return	NULL ;
}

// 画像登録
//////////////////////////////////////////////////////////////////////////////
SGLError S3DMaterialLibrary::AddMaterialAs
		( const wchar_t * pwszID, S3DMaterial * pMaterial )
{
	SSyncReference	ref( pMaterial ) ;
	m_ssaMaterials.Add( pwszID, ref ) ;
	return	sglErrSuccess ;
}

SGLError S3DMaterialLibrary::AddSmartMaterialAs
		( const wchar_t * pwszID, S3DMaterial * pMaterial )
{
	SSyncReference	ref( new SSmartObject( pMaterial ) ) ;
	m_ssaMaterials.Add( pwszID, ref ) ;
	return	sglErrSuccess ;
}

SGLError S3DMaterialLibrary::AddLibraryFrom( const S3DMaterialLibrary & lib )
{
	const size_t	nCount = lib.m_ssaMaterials.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		const SString *		pstrID = lib.m_ssaMaterials.GetTagAt( i ) ;
		SSyncReference *	pRef = lib.m_ssaMaterials.GetAt( i ) ;
		if ( pstrID && pRef )
		{
			m_ssaMaterials.Add( *pstrID, *pRef ) ;
		}
	}
	return	sglErrSuccess ;
}

// 登録削除
//////////////////////////////////////////////////////////////////////////////
SGLError S3DMaterialLibrary::RemoveMaterialAs( const wchar_t * pwszID )
{
	m_ssaMaterials.RemoveAs( pwszID ) ;
	return	sglErrSuccess ;
}

// 全登録削除
//////////////////////////////////////////////////////////////////////////////
SGLError S3DMaterialLibrary::RemoveAllMaterial( void )
{
	m_ssaMaterials.RemoveAll() ;
	return	sglErrSuccess ;
}

// 名前変更
//////////////////////////////////////////////////////////////////////////////
SGLError S3DMaterialLibrary::RenameMaterialAs
	( S3DMaterial * pMaterial, const wchar_t * pwszID )
{
	ssize_t	i = FindMaterialPtr( pMaterial ) ;
	if ( i < 0 )
	{
		return	sglErrFailed ;
	}
	SSyncReference *	pRef = m_ssaMaterials.GetAt( (size_t) i ) ;
	const SString *		pstrTag = m_ssaMaterials.GetTagAt( (size_t) i ) ;
	if ( (pRef == NULL) || (pstrTag == NULL) )
	{
		return	sglErrFailed ;
	}
	SObject *	pObj = pRef->GetReference() ;
	SReference	refTemp( pObj ) ;
	pRef->ReleaseReference() ;
	//
	m_ssaMaterials.RemoveAt( (size_t) i ) ;
	m_ssaMaterials.Add( pwszID, pObj ) ;
	//
	return	sglErrSuccess ;
}

// 表面属性検索
//////////////////////////////////////////////////////////////////////////////
ssize_t S3DMaterialLibrary::FindMaterialPtr( S3DMaterial * pMaterial ) const
{
	size_t	nCount = m_ssaMaterials.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		SSyncReference *	pRef = m_ssaMaterials.GetAt( i ) ;
		if ( pRef != NULL )
		{
			if ( ESLTypeCast<S3DMaterial>( pRef->GetReference() ) == pMaterial )
			{
				return	(ssize_t) i ;
			}
		}
	}
	return	-1 ;
}

// 登録数取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DMaterialLibrary::GetMaterialCount( void ) const
{
	return	m_ssaMaterials.GetLength() ;
}

// 登録名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DMaterialLibrary::GetMaterialIdentityAt( size_t i ) const
{
	const SString *	pstrID = m_ssaMaterials.GetTagAt( i ) ;
	if ( pstrID != NULL )
	{
		return	*pstrID ;
	}
	return	NULL ;
}

const wchar_t * S3DMaterialLibrary::GetMaterialIdentityOf
									( S3DMaterial * pMaterial ) const
{
	ssize_t	i = FindMaterialPtr( pMaterial ) ;
	if ( i >= 0 )
	{
		return	GetMaterialIdentityAt( (size_t) i ) ;
	}
	S3DMaterialLibrary *	pParent = m_refParent ;
	if ( pParent != NULL )
	{
		const wchar_t *	pwszID = pParent->GetMaterialIdentityOf( pMaterial ) ;
		if ( pwszID != NULL )
		{
			return	pwszID ;
		}
	}
	for ( size_t i = 0; i < m_aRefLib.GetLength(); i ++ )
	{
		S3DMaterialLibrary *	pRef = m_aRefLib.GetAt( i ) ;
		if ( pRef != NULL )
		{
			const wchar_t *	pwszID = pRef->GetMaterialIdentityOf( pMaterial ) ;
			if ( pwszID != NULL )
			{
				return	pwszID ;
			}
		}
	}
	return	NULL ;
}

// 表面属性取得
//////////////////////////////////////////////////////////////////////////////
S3DMaterial * S3DMaterialLibrary::GetMaterialAt( size_t i ) const
{
	SSyncReference *	pRef = m_ssaMaterials.GetAt( i ) ;
	if ( pRef != NULL )
	{
		return	ESLTypeCast<S3DMaterial>( pRef->GetReference() ) ;
	}
	return	NULL ;
}

// 参照ライブラリを設定する
//////////////////////////////////////////////////////////////////////////////
void S3DMaterialLibrary::SetParentLibrary( S3DMaterialLibrary * pLib )
{
	m_refParent = pLib ;
}

size_t S3DMaterialLibrary::AddReferenceLibrary( S3DMaterialLibrary * pLib )
{
	ssize_t	i = m_aRefLib.FindPtr( pLib ) ;
	if ( i >= 0 )
	{
		return	(size_t) i ;
	}
	return	m_aRefLib.Add( pLib ) ;
}

// ライブラリ名
//////////////////////////////////////////////////////////////////////////////
const SSystem::SString& S3DMaterialLibrary::GetLibraryName( void ) const
{
	return	m_strName ;
}

void S3DMaterialLibrary::SetLibraryName( const wchar_t * pwszName )
{
	m_strName = pwszName ;
}

// 参照ライブラリを取得する
//////////////////////////////////////////////////////////////////////////////
S3DMaterialLibrary * S3DMaterialLibrary::GetParentLibrary( void ) const
{
	return	m_refParent ;
}

size_t S3DMaterialLibrary::GetReferenceLibraryCount( void ) const
{
	return	m_aRefLib.GetLength() ;
}

S3DMaterialLibrary * S3DMaterialLibrary::GetReferenceLibraryAt( size_t i ) const
{
	return	m_aRefLib.GetAt( i ) ;
}

void S3DMaterialLibrary::DetachReferenceLibraryAt( size_t i )
{
	m_aRefLib.DetachAt( i ) ;
}

void S3DMaterialLibrary::DetachReferenceLibraryOf( S3DMaterialLibrary * pLib )
{
	m_aRefLib.DetachAt( (size_t) m_aRefLib.FindPtr( pLib ) ) ;
}

void S3DMaterialLibrary::DetachAllReferenceLibrarys( void )
{
	m_aRefLib.DetachAll() ;
}

// 属性名正規化（未使用であることを保証）
//////////////////////////////////////////////////////////////////////////////
const SString& S3DMaterialLibrary::NormalizeIdentity( SString& strID ) const
{
	if ( m_ssaMaterials.GetAs( strID ) == NULL )
	{
		return	strID ;
	}
	SString	strBase = strID ;
	for ( int i = 1; i < 0x10000; i ++ )
	{
		strID = strBase + SString( i ) ;
		if ( m_ssaMaterials.GetAs( strID ) == NULL )
		{
			break ;
		}
	}
	return	strID ;
}

// テクスチャの用途を調査する（シェーディングフラグで取得する）
//////////////////////////////////////////////////////////////////////////////
uint64_t S3DMaterialLibrary::GetTextureUsedFlags( SGLImageObject * pTexture ) const
{
	uint64_t	nFlags = 0 ;
	size_t		nCount = GetMaterialCount() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		S3DMaterial *	pMaterial = GetMaterialAt( i ) ;
		if ( pMaterial == NULL )
		{
			continue ;
		}
		for ( int i = 0; i < S3DMaterial::textureMaxCount; i ++ )
		{
			if ( pMaterial->GetTexture( i ) == pTexture )
			{
				switch ( pMaterial->GetTextureType( i ) )
				{
				case	S3DMaterial::textureDiffusion:
					nFlags |= shadingTextureMapping ;
					break ;
				case	S3DMaterial::textureNormal:
					nFlags |= shadingNormalTexture ;
					break ;
				case	S3DMaterial::textureLuminous:
					nFlags |= shadingLuminousTexture ;
					break ;
				case	S3DMaterial::textureEnvironment:
					nFlags |= shadingEnvironmentMapping ;
					break ;
				case	S3DMaterial::textureAlpha:
					nFlags |= shadingAlphaTexture ;
					break ;
				}
			}
			if ( pMaterial->GetBackTexture( i ) == pTexture )
			{
				switch ( pMaterial->GetBackTextureType( i ) )
				{
				case	S3DMaterial::textureDiffusion:
					nFlags |= shadingTextureMapping ;
					break ;
				case	S3DMaterial::textureNormal:
					nFlags |= shadingNormalTexture ;
					break ;
				case	S3DMaterial::textureLuminous:
					nFlags |= shadingLuminousTexture ;
					break ;
				case	S3DMaterial::textureEnvironment:
					nFlags |= shadingEnvironmentMapping ;
					break ;
				case	S3DMaterial::textureAlpha:
					nFlags |= shadingAlphaTexture ;
					break ;
				}
			}
		}
	}
	return	nFlags ;
}

// 指定画像をテクスチャとして参照しているか？
//////////////////////////////////////////////////////////////////////////////
bool S3DMaterialLibrary::IsTextureUsed( SGLImageObject * pTexture ) const
{
	for ( size_t i = 0; i < GetMaterialCount(); i ++ )
	{
		S3DMaterial *	pMaterial = GetMaterialAt( i ) ;
		if ( pMaterial == NULL )
		{
			continue ;
		}
		if ( (pMaterial->FindTextureOf( pTexture ) >= 0)
			|| (pMaterial->FindBackTextureOf( pTexture ) >= 0) )
		{
			return	true ;
		}
	}
	return	false ;
}


//////////////////////////////////////////////////////////////////////////////
// ウェイトマップ（疎な配列用）
//////////////////////////////////////////////////////////////////////////////

// ウェイトマップページ操作
//////////////////////////////////////////////////////////////////////////////
S3DWeightMapBuffer::Page::Page( void )
{
}

S3DWeightMapBuffer::Page::Page( const Page& src )
{
	for ( size_t i = 0; i < WPAGE_SIZE; i ++ )
	{
		buf[i] = src.buf[i] ;
	}
}

void S3DWeightMapBuffer::Page::Clear( void )
{
	for ( size_t i = 0; i < WPAGE_SIZE; i ++ )
	{
		buf[i] = 0.0f ;
	}
}

void S3DWeightMapBuffer::Page::CopyFrom
	( size_t iDst, const Page& pageSrc, size_t iSrc, size_t nCount )
{
	ESLAssert( iDst <= WPAGE_SIZE ) ;
	ESLAssert( iSrc + nCount <= WPAGE_SIZE ) ;
	ESLAssert( iDst + nCount <= WPAGE_SIZE ) ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		buf[iDst + i] = pageSrc.buf[iSrc + i] ;
	}
}

void S3DWeightMapBuffer::Page::Write
	( size_t iDst, const float32_t * pfpSrc, size_t nCount )
{
	ESLAssert( iDst <= WPAGE_SIZE ) ;
	ESLAssert( iDst + nCount <= WPAGE_SIZE ) ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		buf[iDst + i] = pfpSrc[i] ;
	}
}

bool S3DWeightMapBuffer::Page::IsEmpty( void ) const
{
	for ( size_t i = 0; i < WPAGE_SIZE; i ++ )
	{
		if ( buf[i] != 0.0f )
		{
			return	false ;
		}
	}
	return	true ;
}

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DWeightMapBuffer::S3DWeightMapBuffer( void )
{
	m_iFirst = 0 ;
	m_nLength = 0 ;
	//
	m_iLockFirst = 0 ;
	m_nLockLength = 0 ;
}

S3DWeightMapBuffer::S3DWeightMapBuffer( const S3DWeightMapBuffer& wmb )
	: m_pages( wmb.m_pages ),
		m_iFirst( wmb.m_iFirst ),
		m_nLength( wmb.m_nLength ),
		m_iLockFirst( 0 ),
		m_nLockLength( 0 )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DWeightMapBuffer::~S3DWeightMapBuffer( void )
{
}

// クリア
//////////////////////////////////////////////////////////////////////////////
void S3DWeightMapBuffer::ClearBuffer( void )
{
	m_pages.FreeArray() ;
	m_iFirst = 0 ;
	m_nLength = 0 ;
	//
	m_bufLocked.FreeArray() ;
	m_iLockFirst = 0 ;
	m_nLockLength = 0 ;
}

// 範囲拡張
//////////////////////////////////////////////////////////////////////////////
void S3DWeightMapBuffer::ExpandBounds( size_t iFirst, size_t nCount )
{
	ESLAssert( m_nLockLength == 0 ) ;
	if ( iFirst > m_iFirst )
	{
		nCount += iFirst - m_iFirst ;
		iFirst = m_iFirst ;
		if ( (ssize_t) nCount < 0 )
		{
			nCount = 0 ;
		}
	}
	if ( iFirst + nCount < m_iFirst + m_nLength )
	{
		nCount = m_iFirst + m_nLength - iFirst ;
	}
	//
	m_pages.SetLength( PageLength(nCount) ) ;
	//
	size_t	nPrePadding = m_iFirst - iFirst ;
	if ( nPrePadding > 0 )
	{
		SSmartPointer<Page>	spTempPage = new Page ;
		Page *	pTemp = spTempPage.Ptr() ;
		for ( ssize_t i = (ssize_t) PageLength(nCount) - 1;
				i >= (ssize_t) (nPrePadding >> WPAGE_SIZE_SHIFTER); i -- )
		{
			ssize_t	iDst = i << WPAGE_SIZE_SHIFTER ;
			ssize_t	iSrc = iDst - (ssize_t) nPrePadding ;
			size_t	nSrcOdd = iSrc & WPAGE_ODD_MASK ;
			pTemp->Clear() ;
			if ( iSrc >= 0 )
			{
				Page *	pSrc =
					m_pages.GetAt( (size_t) (iSrc >> WPAGE_SIZE_SHIFTER) ) ;
				if ( pSrc != NULL )
				{
					pTemp->CopyFrom
						( 0, *pSrc, nSrcOdd, (WPAGE_SIZE - nSrcOdd) ) ;
				}
			}
			ESLAssert( (size_t) (iSrc >> WPAGE_SIZE_SHIFTER) >= 0 ) ;
			Page *	pSrc =
				m_pages.GetAt( (size_t) (iSrc >> WPAGE_SIZE_SHIFTER) + 1 ) ;
			if ( pSrc != NULL )
			{
				pTemp->CopyFrom
					( (WPAGE_SIZE - nSrcOdd), *pSrc, 0, nSrcOdd ) ;
			}
			if ( pTemp->IsEmpty() )
			{
				m_pages.SetAt( i, NULL ) ;
			}
			else
			{
				Page *	pDst = m_pages.GetAt( i ) ;
				if ( pDst == NULL )
				{
					pDst = new Page ;
					m_pages.SetAt( i, pDst ) ;
				}
				pDst->CopyFrom( 0, *pTemp, 0, WPAGE_SIZE ) ;
			}
		}
		size_t	iPadBound = (nPrePadding >> WPAGE_SIZE_SHIFTER) ;
		for ( size_t i = 0; i < iPadBound; i ++ )
		{
			m_pages.SetAt( i, NULL ) ;
		}
		Page *	pPage = m_pages.GetAt( iPadBound ) ;
		if ( pPage != NULL )
		{
			size_t	nPadOdd = nPrePadding & WPAGE_ODD_MASK ;
			for ( size_t i = 0; i < nPadOdd; i ++ )
			{
				pPage->buf[i] = 0.0f ;
			}
		}
	}
	//
	m_iFirst = iFirst ;
	m_nLength = nCount ;
	//
	ZeroPadForEndOfPage() ;
}

// 一部書き換え
//////////////////////////////////////////////////////////////////////////////
size_t S3DWeightMapBuffer::WriteWeight
	( size_t iDstFirst, const float32_t * pfpWeight, size_t nCount )
{
	size_t	iFirst = iDstFirst ;
	if ( !NormalizeBounds( iFirst, nCount ) )
	{
		return	0 ;
	}
	ESLAssert( iFirst >= m_iFirst ) ;
	pfpWeight += iFirst - iDstFirst ;
	//
	size_t	nResult = nCount ;
	do
	{
		size_t	iPage = (iFirst - m_iFirst) >> WPAGE_SIZE_SHIFTER ;
		Page *	pPage = m_pages.GetAt( iPage ) ;
		if ( pPage == NULL )
		{
			pPage = new Page ;
			pPage->Clear() ;
			m_pages.SetAt( iPage, pPage ) ;
		}
		size_t	iOdd = ((iFirst - m_iFirst) & WPAGE_ODD_MASK) ;
		size_t	nPageLen = WPAGE_SIZE - iOdd ;
		if ( nPageLen > nCount )
		{
			nPageLen = nCount ;
		}
		pPage->Write( iOdd, pfpWeight, nPageLen ) ;
		if ( pPage->IsEmpty() )
		{
			m_pages.SetAt( iPage, NULL ) ;
		}
		//
		iFirst += nPageLen ;
		pfpWeight += nPageLen ;
		nCount -= nPageLen ;
	}
	while ( nCount > 0 ) ;
	//
	return	nResult ;
}

// ボーン影響範囲切り捨て
//////////////////////////////////////////////////////////////////////////////
void S3DWeightMapBuffer::TrimBounds( void )
{
	size_t		iFirst = 0 ;
	size_t		iEnd = m_nLength ;
	while ( iFirst < iEnd )
	{
		Page *	pPage = m_pages.GetAt( (iEnd - 1) >> WPAGE_SIZE_SHIFTER ) ;
		if ( pPage != NULL )
		{
			if ( pPage->buf[(iEnd - 1) & WPAGE_ODD_MASK] > 1.0e-7 )
			{
				break ;
			}
			iEnd -- ;
		}
		else
		{
			iEnd = (iEnd - 1) & ~WPAGE_ODD_MASK ;
		}
	}
	while ( iFirst < iEnd )
	{
		Page *	pPage = m_pages.GetAt( iFirst >> WPAGE_SIZE_SHIFTER ) ;
		if ( pPage != NULL )
		{
			if ( pPage->buf[iFirst & WPAGE_ODD_MASK] > 1.0e-7 )
			{
				break ;
			}
			iFirst ++ ;
		}
		else
		{
			iFirst = (iFirst & ~WPAGE_ODD_MASK) + WPAGE_SIZE ;
		}
	}
	size_t	nCount = iEnd - iFirst ;
	if ( nCount == 0 )
	{
		m_iFirst = iFirst ;
		m_nLength = 0 ;
		m_pages.FreeArray() ;
		return ;
	}
	ChopBounds( iFirst, m_nLength - iEnd ) ;
}

void S3DWeightMapBuffer::ChopBounds( size_t nLeftCount, size_t nRightCount )
{
	ESLAssert( m_nLockLength == 0 ) ;
	if ( (nRightCount > 0) && (m_nLength >= nRightCount) )
	{
		m_nLength -= nRightCount ;
		m_pages.SetLength( PageLength( m_nLength ) ) ;
		ZeroPadForEndOfPage() ;
	}
	if ( (nLeftCount > 0) && (m_nLength >= nLeftCount) )
	{
		m_iFirst += nLeftCount ;
		m_nLength -= nLeftCount ;
		m_pages.Remove( 0, (nLeftCount >> WPAGE_SIZE_SHIFTER) ) ;
		//
		size_t	nOdd = nLeftCount & WPAGE_ODD_MASK ;
		for ( size_t i = 0; i < m_pages.GetLength(); i ++ )
		{
			Page *	pPage0 = m_pages.GetAt( i ) ;
			Page *	pPage1 = m_pages.GetAt( i + 1 ) ;
			if ( pPage0 == NULL )
			{
				if ( pPage1 == NULL )
				{
					continue ;
				}
				pPage0 = new Page ;
				pPage0->Clear() ;
				m_pages.SetAt( i, pPage0 ) ;
			}
			size_t	nLeft = WPAGE_SIZE - nOdd ;
			size_t	j ;
			for ( j = 0; j < nLeft; j ++ )
			{
				pPage0->buf[j] = pPage0->buf[nOdd + j] ;
			}
			if ( pPage1 != NULL )
			{
				for ( j = 0; j < nOdd; j ++ )
				{
					pPage0->buf[nLeft + j] = pPage1->buf[j] ;
				}
			}
			else
			{
				for ( j = 0; j < nOdd; j ++ )
				{
					pPage0->buf[nLeft + j] = 0.0f ;
				}
			}
			if ( pPage0->IsEmpty() )
			{
				m_pages.SetAt( i, NULL ) ;
			}
		}
	}
}

// 末端ページの終端を０パディング
//////////////////////////////////////////////////////////////////////////////
void S3DWeightMapBuffer::ZeroPadForEndOfPage( void )
{
	Page *	pPage = m_pages.GetAt( m_nLength >> WPAGE_SIZE_SHIFTER ) ;
	if ( pPage != NULL )
	{
		for ( size_t i = (m_nLength & WPAGE_ODD_MASK); i < WPAGE_SIZE; i ++ )
		{
			pPage->buf[i] = 0.0f ;
		}
		if ( pPage->IsEmpty() )
		{
			m_pages.SetAt( (m_nLength >> WPAGE_SIZE_SHIFTER), NULL ) ;
		}
	}
}

// 有効領域取得
//////////////////////////////////////////////////////////////////////////////
bool S3DWeightMapBuffer::NormalizeBounds( size_t& iFirst, size_t& nCount ) const
{
	if ( iFirst < m_iFirst )
	{
		if ( iFirst + nCount <= m_iFirst )
		{
			iFirst = m_iFirst ;
			nCount = 0 ;
			return	false ;
		}
		nCount -= m_iFirst - iFirst ;
		iFirst = m_iFirst ;
	}
	if ( iFirst + nCount > m_iFirst + m_nLength )
	{
		if ( iFirst >= m_iFirst + m_nLength )
		{
			iFirst = m_iFirst + m_nLength ;
			nCount = 0 ;
			return	false ;
		}
		nCount = m_iFirst + m_nLength - iFirst ;
	}
	return	true ;
}

// バッファ参照
//////////////////////////////////////////////////////////////////////////////
float32_t S3DWeightMapBuffer::GetAt( size_t i ) const
{
	if ( (i >= m_iFirst) && (i < m_iFirst + m_nLength) )
	{
		size_t	iPage = (i - m_iFirst) >> WPAGE_SIZE_SHIFTER ;
		size_t	nOdd = (i - m_iFirst) & WPAGE_ODD_MASK ;
		Page *	pPage = m_pages.GetAt( iPage ) ;
		if ( pPage != NULL )
		{
			return	pPage->buf[nOdd] ;
		}
	}
	return	0.0f ;
}

void S3DWeightMapBuffer::SetAt( size_t i, float32_t w )
{
	ESLAssert( (i >= m_iFirst) && (i < m_iFirst + m_nLength) ) ;
	size_t	iPage = (i - m_iFirst) >> WPAGE_SIZE_SHIFTER ;
	size_t	nOdd = (i - m_iFirst) & WPAGE_ODD_MASK ;
	Page *	pPage = m_pages.GetAt( iPage ) ;
	if ( pPage != NULL )
	{
		pPage->buf[nOdd] = w ;
		if ( (w <= 1.0e-7) && pPage->IsEmpty() )
		{
			m_pages.SetAt( iPage, NULL ) ;
		}
	}
	else if ( w > 1.0e-7 )
	{
		pPage = new Page ;
		pPage->Clear() ;
		pPage->buf[nOdd] = w ;
		m_pages.SetAt( iPage, pPage ) ;
	}
}

void S3DWeightMapBuffer::ReadWeight
	( float32_t * pfpWeight, size_t iFirst, size_t nCount ) const 
{
	size_t	iReadFirst = iFirst ;
	eslFillMemory( pfpWeight, 0, nCount * sizeof(float32_t) ) ;
	if ( !NormalizeBounds( iReadFirst, nCount ) )
	{
		return ;
	}
	ESLAssert( iReadFirst >= iFirst ) ;
	ESLAssert( iReadFirst >= m_iFirst ) ;
	pfpWeight += iReadFirst - iFirst ;
	size_t	iDst = 0 ;
	while ( iDst < nCount )
	{
		size_t	iPage = (iReadFirst + iDst - m_iFirst) >> WPAGE_SIZE_SHIFTER ;
		size_t	nOdd = (iReadFirst + iDst - m_iFirst) & WPAGE_ODD_MASK ;
		size_t	nLeft = WPAGE_SIZE - nOdd ;
		if ( iDst + nLeft > nCount )
		{
			nLeft = nCount - iDst ;
		}
		Page *	pPage = m_pages.GetAt( iPage ) ;
		if ( pPage != NULL )
		{
			for ( size_t i = 0; i < nLeft; i ++ )
			{
				pfpWeight[iDst + i] = pPage->buf[nOdd + i] ;
			}
		}
		iDst += nLeft ;
	}
}

float32_t * S3DWeightMapBuffer::LockBuffer( size_t& iFirst, size_t& nCount )
{
	ESLAssert( m_nLockLength == 0 ) ;
	if ( !NormalizeBounds( iFirst, nCount ) )
	{
		m_iLockFirst = 0 ;
		m_nLockLength = 0 ;
		return	NULL ;
	}
	ESLAssert( iFirst >= m_iFirst ) ;
	m_bufLocked.SetLength( nCount ) ;
	//
	float32_t *	pLock = m_bufLocked.GetArray() ;
	size_t	iDst = 0 ;
	while ( iDst < nCount )
	{
		size_t	iPage = (iFirst + iDst - m_iFirst) >> WPAGE_SIZE_SHIFTER ;
		size_t	nOdd = (iFirst + iDst - m_iFirst) & WPAGE_ODD_MASK ;
		size_t	nLeft = WPAGE_SIZE - nOdd ;
		if ( iDst + nLeft > nCount )
		{
			nLeft = nCount - iDst ;
		}
		Page *	pPage = m_pages.GetAt( iPage ) ;
		if ( pPage != NULL )
		{
			for ( size_t i = 0; i < nLeft; i ++ )
			{
				pLock[iDst + i] = pPage->buf[nOdd + i] ;
			}
		}
		else
		{
			for ( size_t i = 0; i < nLeft; i ++ )
			{
				pLock[iDst + i] = 0.0f ;
			}
		}
		iDst += nLeft ;
	}
	m_iLockFirst = iFirst ;
	m_nLockLength = nCount ;
	return	pLock ;
}

void S3DWeightMapBuffer::UnlockBuffer( bool flagWrite )
{
	m_bufLocked.FinishArray() ;
	//
	if ( m_nLockLength == 0 )
	{
		return ;
	}
	if ( flagWrite )
	{
		ESLAssert( m_iLockFirst >= m_iFirst ) ;
		float32_t *	pSrc = m_bufLocked.GetArray() ;
		size_t		i = 0 ;
		while ( i < m_nLockLength )
		{
			size_t	iPage = (m_iLockFirst + i - m_iFirst) >> WPAGE_SIZE_SHIFTER ;
			size_t	nOdd = (m_iLockFirst + i - m_iFirst) & WPAGE_ODD_MASK ;
			size_t	nLeft = WPAGE_SIZE - nOdd ;
			if ( i + nLeft > m_nLockLength )
			{
				nLeft = m_nLockLength - i ;
			}
			Page *	pPage = m_pages.GetAt( iPage ) ;
			if ( pPage == NULL )
			{
				pPage = new Page ;
				pPage->Clear() ;
				m_pages.SetAt( iPage, pPage ) ;
			}
			for ( size_t j = 0; j < nLeft; j ++ )
			{
				pPage->buf[nOdd + j] = pSrc[i + j] ;
			}
			if ( pPage->IsEmpty() )
			{
				m_pages.SetAt( iPage, NULL ) ;
			}
			i += nLeft ;
		}
		m_bufLocked.FinishArray() ;
	}
	m_bufLocked.FreeArray() ;
	m_iLockFirst = 0 ;
	m_nLockLength = 0 ;
}


//////////////////////////////////////////////////////////////////////////////
// モデルデータ・メッシュオブジェクト
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DModelData::MeshObject::MeshObject( void )
	: m_pMaterial(NULL), m_typeMesh(primitiveTriangle),
		m_countPolygon(0), m_countVertex(0),
		m_nExAttrElements(0),
		m_iSubMeshSelector(-1), m_fpSubMeshDensity(4.0f),
		m_iVertex(-1), m_iNormal(-1),
		m_nTargetMeshCount(0)
{
	for ( int i = 0; i < VertexBuffer::countSubMesh; i ++ )
	{
		m_countSubPoly[i] = 0 ;
	}
}

S3DModelData::MeshObject::MeshObject( const MeshObject& mesh )
	: m_pMaterial(mesh.m_pMaterial), m_typeMesh(mesh.m_typeMesh),
		m_countPolygon(mesh.m_countPolygon),
		m_countVertex(mesh.m_countVertex),
		m_nExAttrElements(mesh.m_nExAttrElements),
		m_iSubMeshSelector(mesh.m_iSubMeshSelector),
		m_fpSubMeshDensity(mesh.m_fpSubMeshDensity),
		m_iVertex(mesh.m_iVertex), m_iNormal(mesh.m_iNormal),
		m_bufUVMap(mesh.m_bufUVMap),
		m_bufColor(mesh.m_bufColor), m_bufIndex(mesh.m_bufIndex),
		m_bufExAttrElements(mesh.m_bufExAttrElements),
		m_arrMorphTarget(mesh.m_arrMorphTarget),
		m_arrMorphTargetMesh(mesh.m_arrMorphTargetMesh),
		m_arrMorphApplication(mesh.m_arrMorphApplication),
		m_nTargetMeshCount(mesh.m_nTargetMeshCount)
{
	for ( int i = 0; i < VertexBuffer::countSubMesh; i ++ )
	{
		m_countSubPoly[i] = mesh.m_countSubPoly[i] ;
		m_bufSubIndex[i].Merge( 0, mesh.m_bufSubIndex[i] ) ;
	}
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DModelData::MeshObject::~MeshObject( void )
{
}

// 影響ボーン検索
//////////////////////////////////////////////////////////////////////////////
ssize_t S3DModelData::MeshObject::FindRelativeBone( S3DModelBoneSpace * pBone ) const
{
	for ( size_t i = 0; i < m_arrRelBone.GetLength(); i ++ )
	{
		BONE_LINK_INFO *	pbli = m_arrRelBone.GetAt( i ) ;
		ESLAssert( pbli != NULL ) ;
		if ( (pbli != NULL) && (pbli->pRelBone == pBone) )
		{
			return	(ssize_t) i ;
		}
	}
	return	-1 ;
}

// モーフィングターゲット指標検索
//////////////////////////////////////////////////////////////////////////////
ssize_t S3DModelData::MeshObject::FindMorphTarget( const wchar_t * pwszTargetID ) const
{
	ssize_t	iTarget = -1 ;
	for ( size_t i = 0; i < m_arrMorphTarget.GetLength(); i ++ )
	{
		SString *	pstrTarget = m_arrMorphTarget.GetAt( i ) ;
		ESLAssert( pstrTarget != NULL ) ;
		if ( *pstrTarget == pwszTargetID )
		{
			iTarget = (ssize_t) i ;
			break ;
		}
	}
	return	iTarget ;
}



//////////////////////////////////////////////////////////////////////////////
// 分割メッシュ情報
//////////////////////////////////////////////////////////////////////////////

// 分割先のモーフィングターゲットを取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DModelData::MeshDivision::MapMorphTargetAs
	( const S3DModelData::MeshDivision::SplittedEntry * pSplitted,
									const wchar_t * pwszMorphTarget ) const
{
	ESLAssert( pSplitted != nullptr ) ;
	if ( pSplitted != nullptr )
	{
		ssize_t	iEntry = m_aMorphEntries.FindIndex( pwszMorphTarget ) ;
		if ( iEntry >= 0 )
		{
			const SString *	pstrMappedMorph =
				pSplitted->m_aSplittedMorph.GetAt( (size_t) iEntry ) ;
			if ( pstrMappedMorph != nullptr )
			{
				return	*pstrMappedMorph ;
			}
		}
	}
	return	pwszMorphTarget ;
}



//////////////////////////////////////////////////////////////////////////////
// マーカー情報
//////////////////////////////////////////////////////////////////////////////

const wchar_t *	S3DModelData::MarkerInfo::m_pwszTypeTags[S3DModelData::MarkerInfo::typeCount] =
{
	L"position", L"bone_collider", L"collision", L"collider", L"effector",
} ;

const wchar_t *	S3DModelData::MarkerInfo::m_pwszShapeIDs[S3DModelData::MarkerInfo::shapeCount] =
{
	L"sphere", L"cube", L"tube",
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DModelData::MarkerInfo, SObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DModelData::MarkerInfo::MarkerInfo( void )
	: m_type( typePosition ), m_shape( shapeSphere ), m_iCollider( -1 ),
		m_qRotation( 1, 0, 0, 0 ), m_vPosition( 0, 0, 0 ),
		m_vDirection( 0, 0, 1 ), m_vSize( 1, 1, 1 ),
		m_fpRadius( 1 ), m_fpLength( 1 )
{
}

S3DModelData::MarkerInfo::MarkerInfo( const S3DModelData::MarkerInfo& mi )
	: m_type( mi.m_type ), m_shape( mi.m_shape ),
		m_qRotation( mi.m_qRotation ),
		m_vPosition( mi.m_vPosition ),
		m_vDirection( mi.m_vDirection ),
		m_vSize( mi.m_vSize ),
		m_fpRadius( mi.m_fpRadius ),
		m_fpLength( mi.m_fpLength ),
		m_strRefBone( mi.m_strRefBone ),
		m_xmlMarker( mi.m_xmlMarker )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DModelData::MarkerInfo::~MarkerInfo( void )
{
}

// 解釈
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelData::MarkerInfo::ParseMarker( const SSystem::SXMLDocument& xmlMarker )
{
	m_xmlMarker = xmlMarker ;
	//
	size_t	i ;
	for ( i = 0; i < typeCount; i ++ )
	{
		if ( m_xmlMarker.GetTag() == m_pwszTypeTags[i] )
		{
			m_type = (Type) i ;
			break ;
		}
	}
	const SString *	pstrShape = m_xmlMarker.GetAttributeAs( L"shape" ) ;
	if ( pstrShape != NULL )
	{
		for ( i = 0; i < typeCount; i ++ )
		{
			if ( *pstrShape == m_pwszShapeIDs[i] )
			{
				m_shape = (Shape) i ;
				break ;
			}
		}
	}
	m_iCollider = (int) m_xmlMarker.GetAttrIntegerAs( L"collider_index", m_iCollider ) ;
	m_qRotation.q[0] = (float32_t) m_xmlMarker.GetAttrRealAs( L"q0", m_qRotation.q[0] ) ;
	m_qRotation.q[1] = (float32_t) m_xmlMarker.GetAttrRealAs( L"q1", m_qRotation.q[1] ) ;
	m_qRotation.q[2] = (float32_t) m_xmlMarker.GetAttrRealAs( L"q2", m_qRotation.q[2] ) ;
	m_qRotation.q[3] = (float32_t) m_xmlMarker.GetAttrRealAs( L"q3", m_qRotation.q[3] ) ;
	m_vPosition.x = (float32_t) m_xmlMarker.GetAttrRealAs( L"x", m_vPosition.x ) ;
	m_vPosition.y = (float32_t) m_xmlMarker.GetAttrRealAs( L"y", m_vPosition.y ) ;
	m_vPosition.z = (float32_t) m_xmlMarker.GetAttrRealAs( L"z", m_vPosition.z ) ;
	m_vDirection.x = (float32_t) m_xmlMarker.GetAttrRealAs( L"dx", m_vDirection.x ) ;
	m_vDirection.y = (float32_t) m_xmlMarker.GetAttrRealAs( L"dy", m_vDirection.y ) ;
	m_vDirection.z = (float32_t) m_xmlMarker.GetAttrRealAs( L"dz", m_vDirection.z ) ;
	m_vSize.x = (float32_t) m_xmlMarker.GetAttrRealAs( L"sx", m_vSize.x ) ;
	m_vSize.y = (float32_t) m_xmlMarker.GetAttrRealAs( L"sy", m_vSize.y ) ;
	m_vSize.z = (float32_t) m_xmlMarker.GetAttrRealAs( L"sz", m_vSize.z ) ;
	m_fpRadius = (float32_t) m_xmlMarker.GetAttrRealAs( L"radius", m_fpRadius ) ;
	m_fpLength = (float32_t) m_xmlMarker.GetAttrRealAs( L"length", m_fpLength ) ;
	m_strRefBone = m_xmlMarker.GetAttrStringAs( L"ref_bone", m_strRefBone ) ;
	return	sglErrSuccess ;
}

// 書式確定
//////////////////////////////////////////////////////////////////////////////
void S3DModelData::MarkerInfo::CommitInfo( void )
{
	m_xmlMarker.SetTag( m_pwszTypeTags[m_type] ) ;
	m_xmlMarker.SetAttributeAs( L"shape", m_pwszShapeIDs[m_shape] ) ;
	m_xmlMarker.SetAttrIntegerAs( L"collider_index", m_iCollider ) ;
	m_xmlMarker.SetAttrRealAs( L"q0", m_qRotation.q[0] ) ;
	m_xmlMarker.SetAttrRealAs( L"q1", m_qRotation.q[1] ) ;
	m_xmlMarker.SetAttrRealAs( L"q2", m_qRotation.q[2] ) ;
	m_xmlMarker.SetAttrRealAs( L"q3", m_qRotation.q[3] ) ;
	m_xmlMarker.SetAttrRealAs( L"x", m_vPosition.x ) ;
	m_xmlMarker.SetAttrRealAs( L"y", m_vPosition.y ) ;
	m_xmlMarker.SetAttrRealAs( L"z", m_vPosition.z ) ;
	m_xmlMarker.SetAttrRealAs( L"dx", m_vDirection.x ) ;
	m_xmlMarker.SetAttrRealAs( L"dy", m_vDirection.y ) ;
	m_xmlMarker.SetAttrRealAs( L"dz", m_vDirection.z ) ;
	m_xmlMarker.SetAttrRealAs( L"sx", m_vSize.x ) ;
	m_xmlMarker.SetAttrRealAs( L"sy", m_vSize.y ) ;
	m_xmlMarker.SetAttrRealAs( L"sz", m_vSize.z ) ;
	m_xmlMarker.SetAttrRealAs( L"radius", m_fpRadius ) ;
	m_xmlMarker.SetAttrRealAs( L"length", m_fpLength ) ;
	m_xmlMarker.SetAttributeAs( L"ref_bone", m_strRefBone ) ;
}



//////////////////////////////////////////////////////////////////////////////
// モデルデータ・バッファ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DModelBuffer, S3DVertexBuffer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DModelBuffer::S3DModelBuffer( void )
{
	m_iUpdateVertex = 0 ;
	m_iUpdateNormal = 0 ;
	m_pRefModel = NULL ;
	m_flagUpdateBones = false ;
	m_flagPhysicsBones = false ;
	//
	m_materials.AttachLocalTextureLibrary( &m_textures ) ;
}

S3DModelBuffer::S3DModelBuffer( const S3DModelBuffer & model )
	: m_arrMeshObj( model.m_arrMeshObj ),
		m_bufVertex( model.m_bufVertex ),
		m_bufNormal( model.m_bufNormal ),
		m_iUpdateVertex( model.m_iUpdateVertex ),
		m_iUpdateNormal( model.m_iUpdateNormal ),
		m_textures( model.m_textures ),
		m_materials( model.m_materials ),
		m_poses( model.m_poses ),
		m_ssaMeshGroup( model.m_ssaMeshGroup ),
		m_ssoaMorphTarget( model.m_ssoaMorphTarget ),
		m_pRefModel( model.m_pRefModel ),
		m_ssaPhysMaterial( model.m_ssaPhysMaterial ),
		m_ssoaBones( model.m_ssoaBones )
{
	m_flagUpdateBones = false ;
	m_flagPhysicsBones = model.m_flagPhysicsBones ;
	//
	m_materials.AttachLocalTextureLibrary( &m_textures ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DModelBuffer::~S3DModelBuffer( void )
{
}

// 画像読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelBuffer::LoadModel
	( const wchar_t * pszFilePath, const wchar_t * pszMIME )
{
	SSmartPointer<SFileInterface>	pFile =
		SFileOpener::DefaultNewOpenFile( pszFilePath, SFileOpener::shareRead ) ;
	if ( pFile == NULL )
	{
		return	sglErrFailed ;
	}
	if ( (pszMIME == nullptr) || (pszMIME[0] == 0) )
	{
		SString	strFileExt = SString(pszFilePath).GetFileExtensionPart() ;
		SSmartPointer<S3DModelLoaderInterface>	pModelLoader =
			S3DModelLoaderInterface::NewModelLoaderFileExtensionAs( strFileExt ) ;
		if ( pModelLoader != nullptr )
		{
			SGLError	err = pModelLoader->ReadModel( *this, *pFile ) ;
			ReflectAllBonesIdentity() ;
			m_boneRoot.ResetPhysicsParameter() ;
			return	err ;
		}
	}
	return	ReadModel( pFile, pszMIME ) ;
}

SGLError S3DModelBuffer::ReadModel
	( SSystem::SFileInterface * file, const wchar_t * pszMIME )
{
	ClearBuffer() ;
	m_boneRoot.AttachModel( this ) ;
	SetBufferUnitSize( 0xF000 ) ;
	//
	if ( (pszMIME != nullptr) && (pszMIME[0] != 0) )
	{
		SSmartPointer<S3DModelLoaderInterface>	pModelLoader =
			S3DModelLoaderInterface::NewModelLoaderMIMETypeAs( pszMIME ) ;
		if ( pModelLoader != nullptr )
		{
			SGLError	err = pModelLoader->ReadModel( *this, *file ) ;
			ReflectAllBonesIdentity() ;
			m_boneRoot.ResetPhysicsParameter() ;
			return	err ;
		}
	}
	SGLError	err ;
	if ( SString::CompareNoCase( pszMIME, L"application/x-xmlmdf" ) == 0 )
	{
		S3DStdXMLModelLoader	loader ;
		err = loader.ReadModel( *this, *file ) ;
	}
	else
	{
		S3DStdModelLoader	loader ;
		err = loader.ReadModel( *this, *file ) ;
	}
	ReflectAllBonesIdentity() ;
	m_boneRoot.ResetPhysicsParameter() ;
	return	err ;
}

// モデル書き出し
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelBuffer::SaveModel
	( const wchar_t * pszFilePath,
		const wchar_t * pszMIME, const wchar_t * pszImageMIME )
{
	SSmartPointer<SFileInterface>	pFile =
		SFileOpener::DefaultNewOpenFile( pszFilePath, SFileOpener::modeCreate ) ;
	if ( pFile == NULL )
	{
		return	sglErrFailed ;
	}
	return	WriteModel( pFile, pszMIME, pszImageMIME ) ;
}

SGLError S3DModelBuffer::WriteModel
	( SSystem::SFileInterface * file,
		const wchar_t * pszMIME, const wchar_t * pszImageMIME )
{
	if ( SString::CompareNoCase( pszMIME, L"application/x-xmlmdf" ) == 0 )
	{
		S3DStdXMLModelSaver	saver ;
		const wchar_t *	pwszExt = L".eri" ;
		if ( SString::CompareNoCase( pszImageMIME, L"image/png" ) == 0 )
		{
			pwszExt = L".png" ;
		}
		else if ( (SString::CompareNoCase( pszImageMIME, L"image/jpg" ) == 0)
				|| (SString::CompareNoCase( pszImageMIME, L"image/jpeg" ) == 0) )
		{
			pwszExt = L".jpg" ;
		}
		if ( pszImageMIME != NULL )
		{
			saver.SetImageFormat( pszImageMIME, pwszExt ) ;
		}
		return	saver.WriteModel( *file, *this ) ;
	}
	else
	{
		S3DStdModelSaver	saver ;
		if ( pszImageMIME != NULL )
		{
			saver.SetImageFormat( pszImageMIME ) ;
		}
		return	saver.WriteModel( *file, *this ) ;
	}
}

// ボーン情報のみ読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelBuffer::ReadBoneFile( SSystem::SChunkFile * file )
{
	m_boneRoot.RemoveAllChildren() ;
	m_ssoaBones.RemoveAll() ;
	//
	S3DStdModelLoader	loader ;
	SGLError	err = loader.ReadBoneChunk( *this, *file ) ;
	//
	NotifyPhysicsBonesUpdate() ;
	m_boneRoot.ResetPhysicsParameter() ;
	return	err ;
}

SGLError S3DModelBuffer::ParseBoneXML( SSystem::SXMLDocument & xmlBone )
{
	m_boneRoot.RemoveAllChildren() ;
	m_ssoaBones.RemoveAll() ;
	//
	S3DStdXMLModelLoader	loader ;
	SGLError	err = loader.ParseBoneTag( *this, xmlBone ) ;
	//
	NotifyPhysicsBonesUpdate() ;
	m_boneRoot.ResetPhysicsParameter() ;
	return	err ;
}

SGLError S3DModelBuffer::ParseBonePhysicsXML( SSystem::SXMLDocument & xmlBone )
{
	S3DStdXMLModelLoader	loader ;
	SGLError	err = loader.ParseBoneTagPhysics( *this, xmlBone ) ;
	//
	NotifyPhysicsBonesUpdate() ;
	m_boneRoot.ResetPhysicsParameter() ;
	return	err ;
}

// ボーン情報のみ書き出し
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelBuffer::WriteBoneFile( SSystem::SChunkFile * file )
{
	S3DStdModelSaver	saver ;
	return	saver.WriteBoneChunk( *file, *this ) ;
}

SGLError S3DModelBuffer::FormatBoneXML( SSystem::SXMLDocument & xmlBone )
{
	S3DStdXMLModelSaver	saver ;
	xmlBone.SetTag( L"bone" ) ;
	return	saver.FormatBoneTag( xmlBone, *this ) ;
}

SGLError S3DModelBuffer::FormatBonePhysicsXML( SSystem::SXMLDocument & xmlBone )
{
	S3DStdXMLModelSaver	saver ;
	xmlBone.SetTag( L"bone" ) ;
	return	saver.FormatBoneTag
				( xmlBone, *this, S3DStdXMLModelSaver::boneOnlyPhysics ) ;
}

// マーカー情報読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelBuffer::ImportMarkerXML
	( SSystem::SXMLDocument & xmlMarker, bool fOverwrite )
{
	for ( size_t i = 0; i < xmlMarker.GetElementsCount(); i ++ )
	{
		SXMLDocument *	pxmlTag = xmlMarker.GetElementAt( i ) ;
		if ( pxmlTag == NULL )
		{
			continue ;
		}
		const SString *	pstrID = pxmlTag->GetAttributeAs( L"id" ) ;
		if ( pstrID == NULL )
		{
			continue ;
		}
		S3DModelData::MarkerInfo *	pmi = new S3DModelData::MarkerInfo ;
		pmi->ParseMarker( *pxmlTag ) ;
		if ( fOverwrite )
		{
			GetMarkerInfoList().SetAs( *pstrID, pmi ) ;
		}
		else if ( GetMarkerInfoList().GetAs( *pstrID ) == NULL )
		{
			GetMarkerInfoList().Add( *pstrID, pmi ) ;
		}
		else
		{
			delete	pmi ;
		}
	}
	return	sglErrSuccess ;
}

// マーカー情報書き出し
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelBuffer::FormatMarkerXML( SSystem::SXMLDocument & xmlMarker )
{
	xmlMarker.SetTag( L"markers" ) ;
	//
	size_t	nCount = GetMarkerInfoCount() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		const SString *	pstrID = GetMarkerInfoIdentityAt( i ) ;
		S3DModelData::MarkerInfo *	pmi = GetMarkerInfoAt( i ) ;
		if ( (pstrID != NULL) && (pmi != NULL) )
		{
			pmi->CommitInfo() ;
			pmi->m_xmlMarker.SetAttributeAs( L"id", *pstrID ) ;
			xmlMarker.AddElement( new SXMLDocument( pmi->m_xmlMarker ) ) ;
		}
	}
	return	sglErrSuccess ;
}

// ボーンアニメーション用のモデル参照設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelBuffer::AttachModelReference( S3DModelBuffer & model )
{
	ClearBuffer() ;
	//
	// 参照設定
	//
	m_pRefModel = model.GetReferenceModel() ;
	if ( m_pRefModel == nullptr )
	{
		m_pRefModel = &model ;
	}
	AttachVertexBuffer( model.NewReferenceVariantBuffer(), true ) ;
	//
	m_textures.SetParentLibrary( &(model.m_textures) ) ;
	m_materials.SetParentLibrary( &(model.m_materials) ) ;
	m_poses.SetParentLibrary( &(model.m_poses) ) ;
	//
	// ボーン複製
	//
	BonePtrSortObjectArray	mapBone ;
	DuplicateReferenceBones
		( mapBone, m_boneRoot, model, model.m_boneRoot ) ;
	ReflectAllBonesIdentity() ;
	//
	// メッシュ情報にボーン関連付け
	//
	const size_t	nMeshCount = model.m_arrMeshObj.GetLength() ;
	m_arrMeshObj.SetLength( nMeshCount ) ;
	for ( size_t i = 0; i < nMeshCount; i ++ )
	{
		MeshObject *	pmoRef = model.m_arrMeshObj.GetAt( i ) ;
		ESLAssert( pmoRef != NULL ) ;
		if ( pmoRef == NULL )
		{
			continue ;
		}
		MeshObject *	pmoDup = new MeshObject ;
		pmoDup->m_pMaterial = pmoRef->m_pMaterial ;
		pmoDup->m_typeMesh = pmoRef->m_typeMesh ;
		pmoDup->m_countPolygon = pmoRef->m_countPolygon ;
		//
		for ( int j = 0; j < VertexBuffer::countSubMesh; j ++ )
		{
			pmoDup->m_countSubPoly[j] = pmoRef->m_countSubPoly[j] ;
		}
		pmoDup->m_fpSubMeshDensity = pmoRef->m_fpSubMeshDensity ;
		pmoDup->m_countVertex = pmoRef->m_countVertex ;
		pmoDup->m_iVertex = pmoRef->m_iVertex ;
		pmoDup->m_iNormal = pmoRef->m_iNormal ;
		pmoDup->m_arrMorphTarget = pmoRef->m_arrMorphTarget ;
		pmoDup->m_arrMorphTargetMesh = pmoRef->m_arrMorphTargetMesh ;
		pmoDup->m_arrMorphApplication = pmoRef->m_arrMorphApplication ;
		pmoDup->m_nTargetMeshCount = pmoRef->m_nTargetMeshCount ;
		//
		for ( size_t j = 0; j < pmoRef->m_arrRelBone.GetLength(); j ++ )
		{
			S3DModelData::BONE_LINK_INFO *
								pbliRelBone = pmoRef->m_arrRelBone.GetAt( j ) ;
			BoneMapper *	pbm = mapBone.GetAs( pbliRelBone->pRelBone ) ;
			if ( pbm != NULL )
			{
				S3DModelData::BONE_LINK_INFO	bli ;
				bli.pRelBone = pbm->pDupBone ;
				bli.matIMesh = pbliRelBone->matIMesh ;
				bli.matRelMesh = pbliRelBone->matRelMesh ;
				pmoDup->m_arrRelBone.Add( bli ) ;
			}
		}
		m_arrMeshObj.SetAt( i, pmoDup ) ;
	}
	return	sglErrSuccess ;
}

void S3DModelBuffer::DuplicateReferenceBones
	( S3DModelBuffer::BonePtrSortObjectArray& mapBone,
		S3DModelBoneSpace& boneDst,
		S3DModelBuffer & modelRef, S3DModelBoneSpace& boneRef )
{
	for ( size_t i = 0; i < boneRef.GetChildrenCount(); i ++ )
	{
		S3DModelBoneSpace *	pRefChild =
			ESLTypeCast<S3DModelBoneSpace>( boneRef.GetChildAt( i ) ) ;
		if ( pRefChild == NULL )
		{
			continue ;
		}
		//
		// ボーン名取得
		//
		SString	strBoneID ;
		ssize_t	iRefBone = modelRef.m_ssoaBones.FindPtr( pRefChild ) ;
		if ( iRefBone >= 0 )
		{
			const SString *	pstrKey =
				modelRef.m_ssoaBones.GetTagAt( (size_t) iRefBone ) ;
			if ( pstrKey != NULL )
			{
				strBoneID = *pstrKey ;
			}
		}
		if ( strBoneID.IsEmpty() )
		{
			int	iNum = 1 ;
			do
			{
				strBoneID = L"bone" ;
				strBoneID += SString( iNum ++ ) ;
			}
			while ( m_ssoaBones.GetAs( strBoneID ) != NULL ) ;
		}
		//
		// 複製
		//
		S3DModelBoneSpace *	pDupChild = new S3DModelBoneSpace ;
		pDupChild->m_matTransformation = pRefChild->m_matTransformation ;
		pDupChild->m_vCenter = pRefChild->m_vCenter ;
		pDupChild->m_nTransparency = pRefChild->m_nTransparency ;
		pDupChild->m_colorEffect = pRefChild->m_colorEffect ;
		//
		pDupChild->SetBoneFlags( pRefChild->GetBoneFlags() ) ;
		pDupChild->SetBoneOffset( pRefChild->GetBoneOffset() ) ;
		pDupChild->SetBoneHandle( pRefChild->GetBoneHandle() ) ;
		pDupChild->SetIKParameter( pRefChild->GetIKParameter() ) ;
		pDupChild->SetBonePhysicalMaterial
						( pRefChild->GetBonePhysicalMaterial() ) ;
		pDupChild->SetBonePhysicalMaterialID
						( pRefChild->GetBonePhysicalMaterialID() ) ;
		//
		pDupChild->AttachModel( this ) ;
		//
		m_ssoaBones.Add( strBoneID, pDupChild ) ;
		boneDst.AddChild( pDupChild ) ;
		//
		if ( pDupChild->GetBoneFlags() & S3DModelBoneSpace::flagBonePhysics )
		{
			m_flagPhysicsBones = true ;
		}
		//
		// マップ
		//
		BoneMapper *	pbm = new BoneMapper ;
		pbm->pDupBone = pDupChild ;
		mapBone.SetAs( pRefChild, pbm ) ;
		//
		// 子ボーン
		//
		DuplicateReferenceBones
			( mapBone, *pDupChild, modelRef, *pRefChild ) ;
	}
}

void S3DModelBuffer::ReflectAllBonesIdentity( void )
{
	const size_t	nCount = m_ssoaBones.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		S3DModelBoneSpace *	pBone = m_ssoaBones.GetAt( i ) ;
		const SString *		pID = m_ssoaBones.GetTagAt( i ) ;
		if ( (pBone != nullptr) && (pID != nullptr) )
		{
			pBone->m_pwszID = *pID ;
		}
	}
}

// モーフターゲット追加
//////////////////////////////////////////////////////////////////////////////
S3DModelBuffer::MorphTargetMesh *
	S3DModelBuffer::AddMorhTargetAs
		( const wchar_t * pwszID,
			S3DModelBuffer::MorphTargetMesh * pMorphTarget )
{
	m_ssoaMorphTarget.Add( pwszID, pMorphTarget ) ;
	return	pMorphTarget ;
}

// メッシュグループ取得
//////////////////////////////////////////////////////////////////////////////
S3DModelBuffer::MeshGroup *
	S3DModelBuffer::GetMeshGroupAs( const wchar_t * pwszID ) const
{
	return	GetMeshGroupList().GetAs( pwszID ) ;
}

// メッシュ名取得
//////////////////////////////////////////////////////////////////////////////
const SSystem::SString *
		S3DModelBuffer::GetMeshIdentityAt( size_t iMesh ) const
{
	const SStrSortArray<MeshGroup>&	ssaMeshGroup = GetMeshGroupList() ;
	for ( size_t i = 0; i < ssaMeshGroup.GetLength(); i ++ )
	{
		const SString *	pstrTag = ssaMeshGroup.GetTagAt( i ) ;
		MeshGroup *		pmg = ssaMeshGroup.GetAt( i ) ;
		if ( (pstrTag == NULL) || (pmg == NULL) )
		{
			continue ;
		}
		if ( (pmg->m_iFirstMesh == iMesh)
			&& (pmg->m_nMeshCount == 1) )
		{
			return	pstrTag ;
		}
	}
	return	NULL ;
}

// モーフターゲット取得
//////////////////////////////////////////////////////////////////////////////
S3DModelBuffer::MorphTargetMesh *
	S3DModelBuffer::GetMorhTargetAs( const wchar_t * pwszID ) const
{
	if ( m_pRefModel == NULL )
	{
		return	m_ssoaMorphTarget.GetAs( pwszID ) ;
	}
	else
	{
		return	m_pRefModel->m_ssoaMorphTarget.GetAs( pwszID ) ;
	}
}

// 分割メッシュ情報取得
//////////////////////////////////////////////////////////////////////////////
const S3DModelBuffer::MeshDivision *
	S3DModelBuffer::GetMeshDivisionAs( const wchar_t * pwszID ) const
{
	if ( m_pRefModel == NULL )
	{
		return	m_ssoaMeshDivInfo.GetAs( pwszID ) ;
	}
	else
	{
		return	m_pRefModel->m_ssoaMeshDivInfo.GetAs( pwszID ) ;
	}
}

// マーカー情報数
//////////////////////////////////////////////////////////////////////////////
size_t S3DModelBuffer::GetMarkerInfoCount( void ) const
{
	return	(m_pRefModel == NULL) ?
				m_ssoaMarker.GetLength() :
				m_pRefModel->m_ssoaMarker.GetLength() ;
}

// マーカー情報ID取得
//////////////////////////////////////////////////////////////////////////////
const SSystem::SString * S3DModelBuffer::GetMarkerInfoIdentityAt( size_t i ) const
{
	return	(m_pRefModel == NULL) ?
				m_ssoaMarker.GetTagAt( i ) :
				m_pRefModel->m_ssoaMarker.GetTagAt( i ) ;
}

// マーカー情報取得
//////////////////////////////////////////////////////////////////////////////
S3DModelData::MarkerInfo * S3DModelBuffer::GetMarkerInfoAs( const wchar_t * pwszID ) const
{
	return	(m_pRefModel == NULL) ?
				m_ssoaMarker.GetAs( pwszID ) :
				m_pRefModel->m_ssoaMarker.GetAs( pwszID ) ;
}

S3DModelData::MarkerInfo * S3DModelBuffer::GetMarkerInfoAt( size_t i ) const
{
	return	(m_pRefModel == NULL) ?
				m_ssoaMarker.GetAt( i ) :
				m_pRefModel->m_ssoaMarker.GetAt( i ) ;
}

// 所有ボーン登録
//////////////////////////////////////////////////////////////////////////////
void S3DModelBuffer::AddBonePropertyAs
	( const wchar_t * pwszName, S3DModelBoneSpace * pBone )
{
	ESLAssert( pBone != nullptr ) ;
	m_ssoaBones.Add( pwszName, pBone ) ;
}

// 所有ボーン取得
//////////////////////////////////////////////////////////////////////////////
S3DModelBoneSpace *
	S3DModelBuffer::GetBonePropertyAs( const wchar_t * pwszName ) const
{
	return	m_ssoaBones.GetAs( pwszName ) ;
}

// 所有ボーンID取得
//////////////////////////////////////////////////////////////////////////////
const SSystem::SString *
	S3DModelBuffer::GetBoneIdentityOf( S3DModelBoneSpace * pBone ) const
{
	return	m_ssoaBones.GetTagAt( (size_t) m_ssoaBones.FindPtr( pBone ) ) ;
}

// ボーン物理演算パラメータパレット削除
//////////////////////////////////////////////////////////////////////////////
void S3DModelBuffer::RemovePhysMaterialPaletteAs( const wchar_t * pwszID )
{
	for ( size_t i = 0; i < m_ssoaBones.GetLength(); i ++ )
	{
		S3DModelBoneSpace *	pBone = m_ssoaBones.GetAt( i ) ;
		if ( (pBone != NULL)
			&& (pBone->GetBonePhysicalMaterialID() == pwszID) )
		{
			pBone->SetBonePhysicalMaterialID( L"" ) ;
		}
	}
	m_ssaPhysMaterial.RemoveAs( pwszID ) ;
}

// ボーン物理演算パラメータパレット更新反映
//////////////////////////////////////////////////////////////////////////////
void S3DModelBuffer::UpdatePhysMaterialPaletteAs
	( const wchar_t * pwszID,
		const S3DModelBoneSpace::PhysMaterial& physMaterial )
{
	for ( size_t i = 0; i < m_ssoaBones.GetLength(); i ++ )
	{
		S3DModelBoneSpace *	pBone = m_ssoaBones.GetAt( i ) ;
		if ( (pBone != NULL)
			&& (pBone->GetBonePhysicalMaterialID() == pwszID) )
		{
			pBone->SetBonePhysicalMaterial( physMaterial ) ;
		}
	}
	S3DModelBoneSpace::PhysMaterial *
			pMaterial = m_ssaPhysMaterial.GetAs( pwszID ) ;
	if ( pMaterial == NULL )
	{
		return ;
	}
	*pMaterial = physMaterial ;
}

// 関連アイテムを設定
//////////////////////////////////////////////////////////////////////////////
void S3DModelBuffer::AttachRelationItem( S3DScene::Item * pItem )
{
	m_refRelItem.SetReference( pItem ) ;
}

// 関連アイテムを取得
//////////////////////////////////////////////////////////////////////////////
S3DScene::Item * S3DModelBuffer::GetRelationItem( void ) const
{
	return	ESLTypeCast<S3DScene::Item>( m_refRelItem.GetReference() ) ;
}

// デバイスメモリ上に準備する
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelBuffer::CommitToDevice
		( S3DRenderDevice * pDev, int64_t msecTimeout )
{
	m_textures.CommitToDevice( pDev, msecTimeout ) ;
	return	pDev->CommitDeviceVertexBuffer
				( S3DVertexBuffer::GetVertexBuffer(), msecTimeout ) ;
}

// デバイス上のメモリを開放する
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelBuffer::ReleaseForDevice
	( S3DRenderDevice * pDev, int64_t msecTimeout )
{
	m_textures.ReleaseAllDeviceResources() ;
	return	pDev->ReleaseDeviceVertexBuffer
				( S3DVertexBuffer::GetVertexBuffer(), msecTimeout ) ;
}

// 頂点バッファのサイズを設定する
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelBuffer::SetVertexBufferLength( size_t nLength )
{
	m_bufVertex.SetLength( nLength ) ;
	return	sglErrSuccess ;
}

// 頂点バッファのサイズを取得する
//////////////////////////////////////////////////////////////////////////////
size_t S3DModelBuffer::GetVertexBufferLength( void ) const
{
	return	m_bufVertex.GetLength() ;
}
// 頂点バッファへの変更を確定する
//////////////////////////////////////////////////////////////////////////////
void S3DModelBuffer::CommitVertexBuffer( size_t iFirst, size_t nLength )
{
	size_t	nBufLength = m_bufVertex.GetLength() ;
	if ( iFirst < nBufLength )
	{
		if ( iFirst + nLength > nBufLength )
		{
			nLength = nBufLength - iFirst ;
		}
		if ( (m_iUpdateVertex >= iFirst)
				&& (m_iUpdateVertex < iFirst + nLength) )
		{
			m_iUpdateVertex = iFirst + nLength ;
		}
	}
}

// 頂点バッファへのポインタを取得
//////////////////////////////////////////////////////////////////////////////
S3DVector4 * S3DModelBuffer::GetVertexBufferAt( size_t index ) const
{
	return	m_bufVertex.GetAt( index ) ;
}

// 頂点ポインタをインデックスへ変換
//////////////////////////////////////////////////////////////////////////////
ssize_t S3DModelBuffer::VertexPointerToIndex( const S3DVector4 * pvVertex ) const
{
	if ( pvVertex == NULL )
	{
		return	-1 ;
	}
	ulong_ptr_t	index =
		((ulong_ptr_t) pvVertex
			- (ulong_ptr_t) m_bufVertex.GetConstArray()) / sizeof(S3DVector4) ;
	if ( index >= m_bufVertex.GetLength() )
	{
		return	-1 ;
	}
	return	(ssize_t) index ;
}

// 法線バッファのサイズを設定する
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelBuffer::SetNormalBufferLength( size_t nLength )
{
	m_bufNormal.SetLength( nLength ) ;
	return	sglErrSuccess ;
}

// 法線バッファのサイズを取得する
//////////////////////////////////////////////////////////////////////////////
size_t S3DModelBuffer::GetNormalBufferLength( void ) const
{
	return	m_bufNormal.GetLength() ;
}

// 法線バッファへの変更を確定する
//////////////////////////////////////////////////////////////////////////////
void S3DModelBuffer::CommitNormalBuffer( size_t iFirst, size_t nLength )
{
	size_t	nBufLength = m_bufNormal.GetLength() ;
	if ( iFirst < nBufLength )
	{
		if ( iFirst + nLength > nBufLength )
		{
			nLength = nBufLength - iFirst ;
		}
		if ( (m_iUpdateNormal >= iFirst)
				&& (m_iUpdateNormal < iFirst + nLength) )
		{
			m_iUpdateNormal = iFirst + nLength ;
		}
	}
}

// 法線バッファへのポインタを取得
//////////////////////////////////////////////////////////////////////////////
S3DVector4 * S3DModelBuffer::GetNormalBufferAt( size_t index ) const
{
	return	m_bufNormal.GetAt( index ) ;
}

// 法線ポインタをインデックスへ変換
//////////////////////////////////////////////////////////////////////////////
ssize_t S3DModelBuffer::NormalPointerToIndex( const S3DVector4 * pvNormal ) const
{
	if ( pvNormal == NULL )
	{
		return	-1 ;
	}
	ulong_ptr_t	index =
		((ulong_ptr_t) pvNormal
			- (ulong_ptr_t) m_bufNormal.GetConstArray()) / sizeof(S3DVector4) ;
	if ( index >= m_bufNormal.GetLength() )
	{
		return	-1 ;
	}
	return	(ssize_t) index ;
}

// メッシュエントリ取得
//////////////////////////////////////////////////////////////////////////////
S3DModelBuffer::MeshObject * S3DModelBuffer::GetMeshObjectAt( size_t iMesh ) const
{
	return	m_arrMeshObj.GetAt( iMesh ) ;
}

// 指定メッシュのメッシュ名取得
//////////////////////////////////////////////////////////////////////////////
const SSystem::SString *
		S3DModelBuffer::GetMeshGroupNameIndexOf( size_t iMesh ) const
{
	for ( size_t i = 0; i < m_ssaMeshGroup.GetLength(); i ++ )
	{
		MeshGroup *	pmg = m_ssaMeshGroup.GetAt( i ) ;
		if ( (pmg->m_iFirstMesh == iMesh) && (pmg->m_nMeshCount == 1) )
		{
			return	m_ssaMeshGroup.GetTagAt( i ) ;
		}
	}
	return	NULL ;
}

// メッシュの外接直方体取得
//////////////////////////////////////////////////////////////////////////////
bool S3DModelBuffer::GetCircumscribedBoxOfMesh
	( S3DVector& vMin, S3DVector& vMax,
			const S3DModelBuffer::MeshObject& meshObj ) const
{
	if ( (meshObj.m_countVertex == 0)
		|| ((size_t) meshObj.m_iVertex >= m_bufVertex.GetLength()) )
	{
		vMin.x = 0 ;
		vMin.y = 0 ;
		vMin.z = 0 ;
		vMax.x = 0 ;
		vMax.y = 0 ;
		vMax.z = 0 ;
		return	false ;
	}
	size_t	nCount = meshObj.m_countVertex ;
	if ( meshObj.m_iVertex + nCount > m_bufVertex.GetLength() )
	{
		nCount = m_bufVertex.GetLength() - meshObj.m_iVertex ;
	}
	S3DVector4	vMin4, vMax4 ;
	MinMaxVector4DArray
		( vMin4, vMax4,
			m_bufVertex.GetConstArray() + meshObj.m_iVertex, nCount ) ;
	vMin = vMin4 ;
	vMax = vMax4 ;
	return	true ;
}

bool S3DModelBuffer::GetCircumscribedBoxOfMeshAt
	( S3DVector& vMin, S3DVector& vMax, size_t iMesh ) const
{
	MeshObject *	pMeshObj = GetMeshObjectAt( iMesh ) ;
	if ( pMeshObj == nullptr )
	{
		vMin.x = 0 ;
		vMin.y = 0 ;
		vMin.z = 0 ;
		vMax.x = 0 ;
		vMax.y = 0 ;
		vMax.z = 0 ;
		return	false ;
	}
	return	GetCircumscribedBoxOfMesh( vMin, vMax, *pMeshObj ) ;
}

// 物理演算の内部パラメータを複製
//////////////////////////////////////////////////////////////////////////////
void S3DModelBuffer::CopyBonePhysicsParamaters( const S3DModelBuffer& model )
{
	CopyBonePhysicsParamaters( model.m_ssoaBones ) ;
}

void S3DModelBuffer::CopyBonePhysicsParamaters
	( const SSystem::SStrSortObjectArray<S3DModelBoneSpace>& ssoaBones )
{
	size_t	nCount = m_ssoaBones.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		S3DModelBoneSpace *	pBoneDst = m_ssoaBones.GetAt( i ) ;
		const SString *		pstrID = m_ssoaBones.GetTagAt( i ) ;
		if ( pBoneDst && pstrID )
		{
			S3DModelBoneSpace *	pBoneSrc = ssoaBones.GetAs( *pstrID ) ;
			if ( pBoneSrc != NULL )
			{
				pBoneDst->SetPhysicsParamater
					( pBoneSrc->GetPhysicsParamater() ) ;
			}
		}
	}
}

void S3DModelBuffer::GetBonePhysicsParamaters
	( SSystem::SStrSortArray
			<S3DModelBoneSpace::PhysVertex>& ssaPhys )
{
	size_t	nCount = m_ssoaBones.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		S3DModelBoneSpace *	pBone = m_ssoaBones.GetAt( i ) ;
		const SString *		pstrID = m_ssoaBones.GetTagAt( i ) ;
		if ( pBone && pstrID )
		{
			ssaPhys.SetAs( *pstrID, pBone->GetPhysicsParamater() ) ;
		}
	}
}

void S3DModelBuffer::SetBonePhysicsParamaters
	( const SSystem::SStrSortArray
			<S3DModelBoneSpace::PhysVertex>& ssaPhys )
{
	size_t	nCount = m_ssoaBones.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		S3DModelBoneSpace *	pBone = m_ssoaBones.GetAt( i ) ;
		const SString *		pstrID = m_ssoaBones.GetTagAt( i ) ;
		if ( pBone && pstrID )
		{
			S3DModelBoneSpace::PhysVertex *	pPhys = ssaPhys.GetAs( *pstrID ) ;
			if ( pPhys != NULL )
			{
				pBone->SetPhysicsParamater( *pPhys ) ;
			}
		}
	}
}

// 関連性のあるモーフターゲットをメッシュに設定する
//////////////////////////////////////////////////////////////////////////////
void S3DModelBuffer::BuildupMeshMorphingTarget( void )
{
	size_t	nMeshs = m_arrMeshObj.GetLength() ;
	for ( size_t i = 0; i < nMeshs; i ++ )
	{
		MeshObject *	pMesh = m_arrMeshObj.GetAt( i ) ;
		ESLAssert( pMesh != NULL ) ;
		//
		const size_t	nMorphTarget = pMesh->m_arrMorphTarget.GetLength() ;
		if ( nMorphTarget > 0 )
		{
			AllocateMorphing( i, nMorphTarget ) ;
			for ( size_t j = 0; j < nMorphTarget; j ++ )
			{
				SString *	pstrTargetID = pMesh->m_arrMorphTarget.GetAt( j ) ;
				ESLAssert( pstrTargetID != NULL ) ;
				//
				MorphTargetMesh *
					pmtm = m_ssoaMorphTarget.GetAs( *pstrTargetID ) ;
				if ( pmtm != NULL )
				{
					SetMorphingTargetMesh
						( i, j, pmtm->m_countVertex,
							pmtm->m_bufVertex.GetConstArray(),
							pmtm->m_bufNormal.GetConstArray(),
							pmtm->m_bufUVMap.GetConstArray(),
							pmtm->m_bufColor.GetConstArray() ) ;
					//
					if ( pmtm->m_bufWeight.GetLength() >= pmtm->m_countVertex )
					{
						SetMorphingTargetWeight
							( i, j, pmtm->m_countVertex,
								pmtm->m_bufWeight.GetConstArray() ) ;
					}
				}
			}
			SetMorphingApplication( i, NULL, NULL, 0 ) ;
		}
	}
}

// ボーン関連性を解決して構造を完成する
//////////////////////////////////////////////////////////////////////////////
void S3DModelBuffer::BuildupBoneRelation( void )
{
	m_flagPhysicsBones = false ;
	//
	// 関連ボーンを列挙
	//
	size_t	nMeshCount = m_arrMeshObj.GetLength() ;
	size_t	i ;
	for ( i = 0; i < nMeshCount; i ++ )
	{
		MeshObject *	pMesh = m_arrMeshObj.GetAt( i ) ;
		ESLAssert( pMesh != NULL ) ;
		pMesh->m_arrRelBone.RemoveAll() ;
	}
	BuildupSubBoneRelation( &m_boneRoot ) ;
	//
	// ボーンのウェイトマップを設定
	//
	SPointerArray<float32_t>	arrWeightMaps ;
	for ( i = 0; i < nMeshCount; i ++ )
	{
		MeshObject *	pMesh = m_arrMeshObj.GetAt( i ) ;
		ESLAssert( pMesh != NULL ) ;
		//
		size_t	j, nRelCount ;
		nRelCount = pMesh->m_arrRelBone.GetLength() ;
		arrWeightMaps.RemoveAll() ;
		for ( j = 0; j < nRelCount; j ++ )
		{
			S3DModelBoneSpace *	pBone = pMesh->m_arrRelBone.At(j).pRelBone ;
			ESLAssert( pBone != NULL ) ;
			size_t	iFirst = pMesh->m_iVertex ;
			size_t	nCount = pMesh->m_countVertex ;
			float32_t *	pWeightMap = pBone->LockBoneWeightMap( iFirst, nCount ) ;
			ESLAssert( iFirst == pMesh->m_iVertex  ) ;
			ESLAssert( nCount >= pMesh->m_countVertex  ) ;
			arrWeightMaps.Add( pWeightMap ) ;
		}
		if ( arrWeightMaps.GetLength() > 0 )
		{
			BuildupBoneWeightMap
				( i, (const float32_t**) arrWeightMaps.GetConstArray(),
						arrWeightMaps.GetLength(), pMesh->m_countVertex ) ;
		}
		for ( j = 0; j < nRelCount; j ++ )
		{
			S3DModelBoneSpace *	pBone = pMesh->m_arrRelBone.At(j).pRelBone ;
			ESLAssert( pBone != NULL ) ;
			pBone->UnlockBoneWeightMap( false ) ;
		}
	}
	//
	// ボーン行列を設定
	//
	UpdateBoneMatrix() ;
}

void S3DModelBuffer::BuildupSubBoneRelation( S3DModelBoneSpace * pBone )
{
	size_t	nRelCount = pBone->m_arrRefMesh.GetLength() ;
	size_t	i ;
	for ( i = 0; i < nRelCount; i ++ )
	{
		S3DModelBoneSpace::REF_MESH_INFO *
						pRefMeshInfo = pBone->m_arrRefMesh.GetAt( i ) ;
		ESLAssert( pRefMeshInfo != NULL ) ;
		//
		MeshObject *	pRefMesh = m_arrMeshObj.GetAt( pRefMeshInfo->iMesh ) ;
		ESLAssert( (pRefMesh != NULL)
			&& (pBone->VertexIndexOfBoneWeight() <= (size_t) pRefMesh->m_iVertex)
			&& (pBone->VertexIndexOfBoneWeight()
					+ pBone->VertexCountOfBoneWeight()
					>= (size_t) pRefMesh->m_iVertex + pRefMesh->m_countVertex) ) ;
		if ( (pRefMesh != NULL)
			&& (pBone->VertexIndexOfBoneWeight() <= (size_t) pRefMesh->m_iVertex)
			&& (pBone->VertexIndexOfBoneWeight()
					+ pBone->VertexCountOfBoneWeight()
					>= (size_t) pRefMesh->m_iVertex + pRefMesh->m_countVertex) )
		{
			S3DModelData::BONE_LINK_INFO	bli ;
			bli.pRelBone = pBone ;
			bli.matIMesh = pRefMeshInfo->matIMesh ;
			bli.matRelMesh = pRefMeshInfo->matRelMesh ;
			pRefMesh->m_arrRelBone.Add( bli ) ;
		}
	}
	if ( pBone->GetBoneFlags() & S3DModelBoneSpace::flagBonePhysics )
	{
		m_flagPhysicsBones = true ;
	}
	size_t					countChildren = pBone->m_children.GetLength() ;
	S3DScene::Space*const*	ppChildren = pBone->m_children.GetConstArray() ;
	for ( i = 0; i < countChildren; i ++ )
	{
		S3DModelBoneSpace *	pSubBone =
			ESLTypeCast<S3DModelBoneSpace>( ppChildren[i] ) ;
		if ( pSubBone != NULL )
		{
			BuildupSubBoneRelation( pSubBone ) ;
		}
	}
}

void S3DModelBuffer::BuildupBoneWeightMap
	( size_t iMesh, const float32_t ** ppWeightMaps,
						size_t nBoneCount, size_t nVertexCount )
{
	if ( (nBoneCount <= 16) && !S3DRenderDevice::m_availableMultiShapeVB )
	{
		SetBoneWeightMap( iMesh, nBoneCount, ppWeightMaps ) ;
		return ;
	}
	SArray<float32_t>	bufWeight ;
	SArray<uint32_t>	bufIndex ;
	float32_t *			pfpWeight = NULL ;
	uint32_t *			pIndex = NULL ;
	size_t				nJointCount = 0 ;
	for ( size_t i = 0; i < nVertexCount; i ++ )
	{
		uint32_t	k = 0 ;
		for ( size_t j = 0; j < nBoneCount; j ++ )
		{
			float32_t	w = ppWeightMaps[j][i] ;
			if ( fabs(w) > 0.000001 )
			{
				if ( k >= nJointCount )
				{
					nJointCount ++ ;
					bufWeight.SetLength( nJointCount * nVertexCount ) ;
					bufIndex.SetLength( nJointCount * nVertexCount ) ;
					pfpWeight = bufWeight.GetArray() ;
					pIndex = bufIndex.GetArray() ;
				}
				size_t	l = k * nVertexCount + i ;
				pfpWeight[l] = w ;
				pIndex[l] = (uint32_t) j ;
				k ++ ;
			}
		}
	}
	SPointerArray<float32_t>	bufWeightPtr ;
	SPointerArray<uint32_t>		bufIndexPtr ;
	bufWeightPtr.SetLength( nJointCount ) ;
	bufIndexPtr.SetLength( nJointCount ) ;
	for ( size_t i = 0; i < nJointCount; i ++ )
	{
		bufWeightPtr.SetAt( i, pfpWeight + i * nVertexCount ) ;
		bufIndexPtr.SetAt( i, pIndex + i * nVertexCount ) ;
	}
	SetBoneJointMap
		( iMesh, nBoneCount, nJointCount,
			(const uint32_t**) bufIndexPtr.GetConstArray() ) ;
	SetBoneWeightMap
		( iMesh, nJointCount,
			(const float32_t**) bufWeightPtr.GetConstArray() ) ;
	//
	bufWeight.FinishArray() ;
	bufIndex.FinishArray() ;
}

// ボーンの有無
//////////////////////////////////////////////////////////////////////////////
bool S3DModelBuffer::AreAnyBones( void ) const
{
	return	(m_boneRoot.m_children.GetLength() > 0) ;
}

// 物理演算ボーンの有無
//////////////////////////////////////////////////////////////////////////////
bool S3DModelBuffer::AreAnyPhysicsBones( void ) const
{
	return	m_flagPhysicsBones ;
}

// 物理演算ボーンの更新処理
//////////////////////////////////////////////////////////////////////////////
void S3DModelBuffer::NotifyPhysicsBonesUpdate( void )
{
	m_flagPhysicsBones = false ;
	NotifyPhysicsSubBonesUpdate( &m_boneRoot ) ;
}

void S3DModelBuffer::NotifyPhysicsSubBonesUpdate( S3DModelBoneSpace * pBone )
{
	if ( pBone->GetBoneFlags() & S3DModelBoneSpace::flagBonePhysics )
	{
		m_flagPhysicsBones = true ;
	}
	size_t					countChildren = pBone->m_children.GetLength() ;
	S3DScene::Space*const*	ppChildren = pBone->m_children.GetConstArray() ;
	for ( size_t i = 0; i < countChildren; i ++ )
	{
		S3DModelBoneSpace *	pSubBone =
			ESLTypeCast<S3DModelBoneSpace>( ppChildren[i] ) ;
		if ( pSubBone != NULL )
		{
			NotifyPhysicsSubBonesUpdate( pSubBone ) ;
		}
	}
}

// すべてのボーンの変更フラグをクリア
//////////////////////////////////////////////////////////////////////////////
void S3DModelBuffer::ClearAllBonesModifiedFlags( void )
{
	m_boneRoot.ClearAllModifiedFlags() ;
}

// ボーン更新フラグ取得
//////////////////////////////////////////////////////////////////////////////
bool S3DModelBuffer::IsUpdateBone( void ) const
{
	return	m_flagUpdateBones ;
}

// ボーン更新フラグ設定
//////////////////////////////////////////////////////////////////////////////
void S3DModelBuffer::PostUpdateBone( void )
{
	m_flagUpdateBones = true ;
}

// ボーン回転行列を VBO に反映する
//////////////////////////////////////////////////////////////////////////////
void S3DModelBuffer::UpdateBoneMatrix( void )
{
	MeshObject *const*	ppMeshObj = m_arrMeshObj.GetConstArray() ;
	size_t				nMeshCount = m_arrMeshObj.GetLength() ;
	SArray<S3DMatrix>	bufRotation ;
	SArray<S3DVector>	bufTranslate ;
	//
	for ( size_t i = 0; i < nMeshCount; i ++ )
	{
		MeshObject *	pMesh = ppMeshObj[i] ;
		ESLAssert( pMesh != NULL ) ;
		size_t			nBones = pMesh->m_arrRelBone.GetLength() ;
		if ( nBones == 0 )
		{
			continue ;
		}
		const S3DModelData::BONE_LINK_INFO *
						pbliBones = pMesh->m_arrRelBone.GetConstArray() ;
		S3DMatrix *		pRotation = bufRotation.GetArray( nBones ) ;
		S3DVector *		pTranslate = bufTranslate.GetArray( nBones ) ;
		for ( size_t j = 0; j < nBones; j ++ )
		{
			S3DModelBoneSpace *	pBone = pbliBones[j].pRelBone ;
			ESLAssert( pBone != NULL ) ;
			//
			S3DDMatrix	matdBone ;
			S3DDVector	vdBone ;
			pBone->CalcBoneTransformation( matdBone, vdBone ) ;
			//
			S4DMatrix	mat4Bone ;
			Matrix4x4From3x3
				<float32_t,S3DDMatrix,S3DDVector>
						( mat4Bone, matdBone, vdBone ) ;
			//
			mat4Bone = pbliBones[j].matRelMesh
							* mat4Bone * pbliBones[j].matIMesh ;
			//
			Matrix3x3From4x4
				<S3DMatrix,S3DVector,float32_t>
					( pRotation[j], pTranslate[j], mat4Bone ) ;
		}
		SetBoneMatrix( i, nBones, pRotation, pTranslate ) ;
		//
		bufRotation.FinishArray() ;
		bufTranslate.FinishArray() ;
	}
	m_flagUpdateBones = false ;
}

// バッファバリアントの場合、ボーンやマテリアルの設定を参照先のバッファへ即時に反映する
//////////////////////////////////////////////////////////////////////////////
void S3DModelBuffer::ReflectBufferVariantImmediately( void )
{
	if ( m_pRefModel != nullptr )
	{
		S3DRenderVariantBuffer *	prvb =
			ESLTypeCast<S3DRenderVariantBuffer>( GetVertexBuffer() ) ;
		if ( prvb != nullptr )
		{
			m_pRefModel->UpdateVertexVariant( prvb->GetVariantBuffer() ) ;
		}
	}
}

// ボーンの外接直方体を取得する
//////////////////////////////////////////////////////////////////////////////
void S3DModelBuffer::GetBoneCircumscribedParallelepiped
			( S3DDVector& vMin, S3DDVector& vMax ) const
{
	S3DDMatrix	matRoot( 1, 1, 1 ) ;
	S3DDVector	vZero( 0, 0, 0 ) ;
	vMin = vZero ;
	vMax = vZero ;
	//
	m_boneRoot.GetExternalRectangular( vMin, vMax, matRoot, vZero ) ;
}

// 一度構築した VBO を再構築する（MeshObject 等を編集した場合）
//////////////////////////////////////////////////////////////////////////////
void S3DModelBuffer::RebuildVertexBuffer( void )
{
	S3DVertexBuffer::ClearBuffer() ;
	//
	for ( size_t iMesh = 0; iMesh < m_arrMeshObj.GetLength(); iMesh ++ )
	{
		MeshObject *	pMesh = m_arrMeshObj.GetAt( iMesh ) ;
		ESLAssert( pMesh != NULL ) ;
		if ( pMesh == NULL )
		{
			continue ;
		}
		switch ( pMesh->m_typeMesh )
		{
		case	primitiveTriangle:
			S3DVertexBuffer::AddIndexedTriangleList
				( pMesh->m_pMaterial, 0,
					pMesh->m_countPolygon, pMesh->m_countVertex,
					m_bufVertex.GetAt(pMesh->m_iVertex),
					m_bufNormal.GetAt(pMesh->m_iNormal),
					pMesh->m_bufUVMap.GetConstArray(),
					pMesh->m_bufColor.GetConstArray(),
					pMesh->m_bufIndex.GetConstArray() ) ;
			break ;
		case	primitiveTriangleStrip:
			S3DVertexBuffer::AddTriangleStrip
				( pMesh->m_pMaterial, 0,
					pMesh->m_countPolygon,
					m_bufVertex.GetAt(pMesh->m_iVertex),
					m_bufNormal.GetAt(pMesh->m_iNormal),
					pMesh->m_bufUVMap.GetConstArray(),
					pMesh->m_bufColor.GetConstArray() ) ;
			break ;
		default:
			S3DVertexBuffer::AddIndexedPrimitiveList
				( pMesh->m_pMaterial, 0,
					pMesh->m_typeMesh,
					pMesh->m_countPolygon
						* GetPrimitiveVertexCount(pMesh->m_typeMesh),
					pMesh->m_countVertex,
					m_bufVertex.GetAt(pMesh->m_iVertex),
					m_bufNormal.GetAt(pMesh->m_iNormal),
					pMesh->m_bufUVMap.GetConstArray(),
					pMesh->m_bufColor.GetConstArray(),
					pMesh->m_bufIndex.GetConstArray() ) ;
			break ;
		}
		if ( pMesh->m_nExAttrElements >= 1 )
		{
			S3DVertexBuffer::SetExtendVertexAttribute
				( iMesh, pMesh->m_nExAttrElements,
					pMesh->m_countVertex,
					pMesh->m_bufExAttrElements.GetConstArray() ) ;
		}
		//
		S3DVertexBuffer::SetSubMeshDensity
				( iMesh, pMesh->m_fpSubMeshDensity, pMesh->m_iSubMeshSelector ) ;
		//
		for ( size_t iSub = 0; iSub < countSubMesh; iSub ++ )
		{
			if ( (pMesh->m_countSubPoly[iSub] == 0)
				|| (pMesh->m_bufSubIndex[iSub].GetLength() == 0) )
			{
				continue ;
			}
			S3DVertexBuffer::UpdateSubIndexedTriangleList
				( iMesh, iSub, 0,
					pMesh->m_countSubPoly[iSub],
					pMesh->m_bufSubIndex[iSub].GetConstArray() ) ;
		}
	}
	BuildupMeshMorphingTarget() ;
	BuildupBoneRelation() ;
}

// メッシュにボーン行列設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelBuffer::SetBoneMatrix
	( size_t iMesh, size_t nCount,
		const S3DMatrix * pMatrix, const S3DVector * pTrans )
{
	return	S3DVertexBuffer::SetBoneMatrix( iMesh, nCount, pMatrix, pTrans ) ;
}

// モーフィング設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelBuffer::SetMorphingApplication
	( size_t iMesh, const ssize_t * pTargetMesh,
			const float32_t * pApplication, size_t nTargetMeshCount )
{
	MeshObject *	pMesh = m_arrMeshObj.GetAt( iMesh ) ;
	if ( pMesh != NULL )
	{
		pMesh->m_arrMorphTargetMesh.SetLength( nTargetMeshCount ) ;
		pMesh->m_arrMorphApplication.SetLength( nTargetMeshCount ) ;
		//
		eslCopyMemory
			( pMesh->m_arrMorphTargetMesh.GetArray(),
				pTargetMesh, nTargetMeshCount * sizeof(ssize_t) ) ;
		pMesh->m_arrMorphTargetMesh.FinishArray() ;
		//
		eslCopyMemory
			( pMesh->m_arrMorphApplication.GetArray(),
				pApplication, nTargetMeshCount * sizeof(float32_t) ) ;
		pMesh->m_arrMorphApplication.FinishArray() ;
	}
	return	S3DVertexBuffer::SetMorphingApplication
				( iMesh, pTargetMesh, pApplication, nTargetMeshCount ) ;
}

// モーフィング設定取得
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelBuffer::GetMorphingApplication
	( size_t iMesh, ssize_t& iTargetMesh,
			float32_t& fpApplication, size_t iTargetMeshIndex )
{
	return	S3DVertexBuffer::GetMorphingApplication
				( iMesh, iTargetMesh, fpApplication, iTargetMeshIndex ) ;
}

// メッシュ表示設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelBuffer::EnableToRenderMesh
	( size_t iFirst, ssize_t iEnd, bool fEnable )
{
	return	S3DVertexBuffer::EnableToRenderMesh( iFirst, iEnd, fEnable ) ;
}

// メッシュ表示フラグ取得
//////////////////////////////////////////////////////////////////////////////
bool S3DModelBuffer::IsEnabledToRenderMesh( size_t iMesh ) const
{
	return	S3DVertexBuffer::IsEnabledToRenderMesh( iMesh ) ;
}

// ポリゴンリストをレンダリングバッファに追加
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelBuffer::AddIndexedTriangleList
	( S3DMaterial * pMaterial, uint32_t nFlags,
		size_t countPolygon, size_t countVertex,
		const S3DVector4 * pvVertex,
		const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap,
		const S3DColor * pColor,
		const uint32_t * pIndexedList )
{
	MeshObject *	pMesh = new MeshObject ;
	m_arrMeshObj.Add( pMesh ) ;
	//
	pMesh->m_pMaterial = pMaterial ;
	pMesh->m_typeMesh = primitiveTriangle ;
	pMesh->m_countPolygon = countPolygon ;
	pMesh->m_countVertex = countVertex ;
	pMesh->m_iVertex = VertexPointerToIndex( pvVertex ) ;
	pMesh->m_iNormal = NormalPointerToIndex( pvNormal ) ;
	if ( (pvVertex != NULL) && (pMesh->m_iVertex < 0) )
	{
		pMesh->m_iVertex = (ssize_t) m_bufVertex.GetLength() ;
		m_bufVertex.AddArray( pvVertex, countVertex ) ;
	}
	if ( (pvNormal != NULL) && (pMesh->m_iNormal < 0) )
	{
		pMesh->m_iNormal = (ssize_t) m_bufNormal.GetLength() ;
		m_bufNormal.AddArray( pvNormal, countVertex ) ;
	}
	if ( pvUVMap != NULL )
	{
		pMesh->m_bufUVMap.AddArray( pvUVMap, countVertex ) ;
	}
	if ( pColor != NULL )
	{
		pMesh->m_bufColor.AddArray( pColor, countVertex ) ;
	}
	if ( pIndexedList != NULL )
	{
		pMesh->m_bufIndex.AddArray( pIndexedList, countPolygon * 3 ) ;
	}
	if ( nFlags & renderNormalizeFace )
	{
		NormalizeTriangleSurface( pMesh ) ;
	}
	//
	return	S3DVertexBuffer::AddIndexedTriangleList
		( pMaterial, nFlags, countPolygon, countVertex,
			pvVertex, pvNormal,
			pvUVMap, pColor, pMesh->m_bufIndex.GetConstArray() ) ;
}

// トライアングルストリップをレンダリングバッファに追加
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelBuffer::AddTriangleStrip
	( S3DMaterial * pMaterial, uint32_t nFlags,
		size_t countTriangleStrip,
		const S3DVector4 * pvVertex,
		const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap, const S3DColor * pColor )
{
	MeshObject *	pMesh = new MeshObject ;
	m_arrMeshObj.Add( pMesh ) ;
	//
	const size_t	countVertex = countTriangleStrip + 2 ;
	pMesh->m_pMaterial = pMaterial ;
	pMesh->m_typeMesh = primitiveTriangleStrip ;
	pMesh->m_countPolygon = countTriangleStrip ;
	pMesh->m_countVertex = countVertex ;
	pMesh->m_iVertex = VertexPointerToIndex( pvVertex ) ;
	pMesh->m_iNormal = NormalPointerToIndex( pvNormal ) ;
	if ( (pvVertex != NULL) && (pMesh->m_iVertex < 0) )
	{
		pMesh->m_iVertex = (ssize_t) m_bufVertex.GetLength() ;
		m_bufVertex.AddArray( pvVertex, countVertex ) ;
	}
	if ( (pvNormal != NULL) && (pMesh->m_iNormal < 0) )
	{
		pMesh->m_iNormal = (ssize_t) m_bufNormal.GetLength() ;
		m_bufNormal.AddArray( pvNormal, countVertex ) ;
	}
	if ( pvUVMap != NULL )
	{
		pMesh->m_bufUVMap.AddArray( pvUVMap, countVertex ) ;
	}
	if ( pColor != NULL )
	{
		pMesh->m_bufColor.AddArray( pColor, countVertex ) ;
	}
	return	S3DVertexBuffer::AddTriangleStrip
		( pMaterial, nFlags, countTriangleStrip,
				pvVertex, pvNormal, pvUVMap, pColor ) ;
}

// プリミティブリストをレンダリングバッファに追加
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelBuffer::AddIndexedPrimitiveList
	( S3DMaterial * pMaterial, uint32_t nFlags,
		S3DPrimitiveType typePrimitive,
		size_t countIndex, size_t countVertex,
		const S3DVector4 * pvVertex,
		const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap,
		const S3DColor * pColor,
		const uint32_t * pIndexedList )
{
	MeshObject *	pMesh = new MeshObject ;
	m_arrMeshObj.Add( pMesh ) ;
	//
	pMesh->m_pMaterial = pMaterial ;
	pMesh->m_typeMesh = typePrimitive ;
	pMesh->m_countPolygon = countIndex / GetPrimitiveVertexCount(typePrimitive) ;
	pMesh->m_countVertex = countVertex ;
	pMesh->m_iVertex = VertexPointerToIndex( pvVertex ) ;
	pMesh->m_iNormal = NormalPointerToIndex( pvNormal ) ;
	if ( (pvVertex != NULL) && (pMesh->m_iVertex < 0) )
	{
		pMesh->m_iVertex = (ssize_t) m_bufVertex.GetLength() ;
		m_bufVertex.AddArray( pvVertex, countVertex ) ;
	}
	if ( (pvNormal != NULL) && (pMesh->m_iNormal < 0) )
	{
		pMesh->m_iNormal = (ssize_t) m_bufNormal.GetLength() ;
		m_bufNormal.AddArray( pvNormal, countVertex ) ;
	}
	if ( pvUVMap != NULL )
	{
		pMesh->m_bufUVMap.AddArray( pvUVMap, countVertex ) ;
	}
	if ( pColor != NULL )
	{
		pMesh->m_bufColor.AddArray( pColor, countVertex ) ;
	}
	if ( pIndexedList != NULL )
	{
		pMesh->m_bufIndex.AddArray( pIndexedList, countIndex ) ;
	}
	if ( nFlags & renderNormalizeFace )
	{
		NormalizeTriangleSurface( pMesh ) ;
	}
	//
	return	S3DVertexBuffer::AddIndexedPrimitiveList
		( pMaterial, nFlags, typePrimitive, countIndex, countVertex,
			pvVertex, pvNormal,
			pvUVMap, pColor, pMesh->m_bufIndex.GetConstArray() ) ;
}

// プリミティブを追加するためのバッファを確保する
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelBuffer::AllocatePrimitiveBuffer
	( PrimitiveBuffer& prmbuf,
		S3DPrimitiveType typePrimitive,
		size_t countIndex, size_t countVertex )
{
	prmbuf.pvVertex =
		(S3DVector4*) esl_malloc
			( countVertex * (sizeof(S3DVector4) * 2
							+ sizeof(S2DVector) + sizeof(S3DColor))
				+ countIndex * sizeof(uint32_t) ) ;
	prmbuf.pvNormal = prmbuf.pvVertex + countVertex ;
	prmbuf.pvUVMap = (S2DVector*) (prmbuf.pvNormal + countVertex) ;
	prmbuf.pColor = (S3DColor*) (prmbuf.pvUVMap + countVertex) ;
	prmbuf.pIndexedList = (uint32_t*) (prmbuf.pColor + countVertex) ;
	return	sglErrSuccess ;
}

// プリミティブを追加する（バッファの管理は S3DVertexBufferInterface に移る）
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelBuffer::AddPrimitiveBuffer
	( S3DMaterial * pMaterial, uint32_t nFlags,
		S3DPrimitiveType typePrimitive,
		const PrimitiveBuffer& prmbuf,
		size_t countIndex, size_t countVertex )
{
	SGLError	err = AddIndexedPrimitiveList
		( (pMaterial == NULL) ? GetDefaultMaterial() : pMaterial,
			nFlags, typePrimitive, countIndex, countVertex,
			prmbuf.pvVertex, prmbuf.pvNormal,
			prmbuf.pvUVMap, prmbuf.pColor, prmbuf.pIndexedList ) ;
	esl_free( prmbuf.pvVertex ) ;
	return	err ;
}

// プリミティブを追加せずにバッファを開放する
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelBuffer::FreePrimitiveBuffer
	( const PrimitiveBuffer& prmbuf )
{
	esl_free( prmbuf.pvVertex ) ;
	return	sglErrSuccess ;
}

// 描画の確定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelBuffer::Flush( void )
{
	return	S3DVertexBuffer::Flush() ;
}

// ポリゴンリストを更新
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelBuffer::UpdateIndexedTriangleList
	( size_t iMesh, uint32_t nFlags,
		size_t countPolygon, size_t countVertex,
		const S3DVector4 * pvVertex, const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap, const S3DColor * pColor,
		const uint32_t * pIndexedList )
{
	MeshObject *	pMesh = m_arrMeshObj.GetAt( iMesh ) ;
	if ( (pMesh != NULL)
		&& (pMesh->m_countPolygon == countPolygon)
		&& (pMesh->m_countVertex == countVertex) )
	{
		if ( (pvVertex != NULL) && (pMesh->m_iVertex >= 0) )
		{
			eslMoveMemory
				( GetVertexBufferAt( pMesh->m_iVertex ),
					pvVertex, countVertex * sizeof(S3DVector4) ) ;
			//
			if ( (size_t) pMesh->m_iVertex < m_iUpdateVertex )
			{
				m_iUpdateVertex = (size_t) pMesh->m_iVertex ;
			}
		}
		if ( (pvNormal != NULL) && (pMesh->m_iNormal >= 0) )
		{
			eslMoveMemory
				( GetNormalBufferAt( pMesh->m_iNormal ),
					pvNormal, countVertex * sizeof(S3DVector4) ) ;
			//
			if ( (size_t) pMesh->m_iNormal < m_iUpdateNormal )
			{
				m_iUpdateNormal = (size_t) pMesh->m_iNormal ;
			}
		}
		if ( pvUVMap != NULL )
		{
			eslMoveMemory
				( pMesh->m_bufUVMap.GetArray( countVertex ),
					pvUVMap, countVertex * sizeof(S2DVector) ) ;
			pMesh->m_bufUVMap.FinishArray() ;
		}
		if ( pColor != NULL )
		{
			eslMoveMemory
				( pMesh->m_bufColor.GetArray( countVertex ),
					pColor, countVertex * sizeof(S3DColor) ) ;
			pMesh->m_bufColor.FinishArray() ;
		}
		if ( pIndexedList != NULL )
		{
			eslMoveMemory
				( pMesh->m_bufIndex.GetArray( countVertex ),
					pIndexedList, countPolygon * (3 * sizeof(uint32_t)) ) ;
			pMesh->m_bufIndex.FinishArray() ;
		}
	}
	return	S3DVertexBuffer::UpdateIndexedTriangleList
				( iMesh, nFlags, countPolygon, countVertex,
					pvVertex, pvNormal, pvUVMap, pColor, pIndexedList ) ;
}

// トライアングルストリップを更新
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelBuffer::UpdateTriangleStrip
	( size_t iMesh, uint32_t nFlags,
		size_t countTriangleStrip,
		const S3DVector4 * pvVertex, const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap, const S3DColor * pColor )
{
	MeshObject *	pMesh = m_arrMeshObj.GetAt( iMesh ) ;
	const size_t	countVertex = countTriangleStrip + 2 ;
	if ( (pMesh != NULL)
		&& (pMesh->m_countPolygon == countTriangleStrip)
		&& (pMesh->m_countVertex == countVertex) )
	{
		if ( (pvVertex != NULL) && (pMesh->m_iVertex >= 0) )
		{
			eslMoveMemory
				( GetVertexBufferAt( pMesh->m_iVertex ),
					pvVertex, countVertex * sizeof(S3DVector4) ) ;
			//
			if ( (size_t) pMesh->m_iVertex < m_iUpdateVertex )
			{
				m_iUpdateVertex = (size_t) pMesh->m_iVertex ;
			}
		}
		if ( (pvNormal != NULL) && (pMesh->m_iNormal >= 0) )
		{
			eslMoveMemory
				( GetNormalBufferAt( pMesh->m_iNormal ),
					pvNormal, countVertex * sizeof(S3DVector4) ) ;
			//
			if ( (size_t) pMesh->m_iNormal < m_iUpdateNormal )
			{
				m_iUpdateNormal = (size_t) pMesh->m_iNormal ;
			}
		}
		if ( pvUVMap != NULL )
		{
			pMesh->m_bufUVMap.RemoveAll() ;
			pMesh->m_bufUVMap.AddArray( pvUVMap, countVertex ) ;
		}
		if ( pColor != NULL )
		{
			pMesh->m_bufColor.RemoveAll() ;
			pMesh->m_bufColor.AddArray( pColor, countVertex ) ;
		}
	}
	return	S3DVertexBuffer::UpdateTriangleStrip
				( iMesh, nFlags, countTriangleStrip,
						pvVertex, pvNormal, pvUVMap, pColor ) ;
}

// プリミティブリストを更新
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelBuffer::UpdateIndexedPrimitiveList
	( size_t iMesh, uint32_t nFlags,
		size_t countIndex, size_t countVertex,
		const S3DVector4 * pvVertex,
		const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap,
		const S3DColor * pColor,
		const uint32_t * pIndexedList )
{
	MeshObject *	pMesh = m_arrMeshObj.GetAt( iMesh ) ;
	if ( (pMesh != NULL)
		&& (pMesh->m_countPolygon
				* GetPrimitiveVertexCount(pMesh->m_typeMesh) == countIndex)
		&& (pMesh->m_countVertex == countVertex) )
	{
		if ( (pvVertex != NULL) && (pMesh->m_iVertex >= 0) )
		{
			eslMoveMemory
				( GetVertexBufferAt( pMesh->m_iVertex ),
					pvVertex, countVertex * sizeof(S3DVector4) ) ;
			//
			if ( (size_t) pMesh->m_iVertex < m_iUpdateVertex )
			{
				m_iUpdateVertex = (size_t) pMesh->m_iVertex ;
			}
		}
		if ( (pvNormal != NULL) && (pMesh->m_iNormal >= 0) )
		{
			eslMoveMemory
				( GetNormalBufferAt( pMesh->m_iNormal ),
					pvNormal, countVertex * sizeof(S3DVector4) ) ;
			//
			if ( (size_t) pMesh->m_iNormal < m_iUpdateNormal )
			{
				m_iUpdateNormal = (size_t) pMesh->m_iNormal ;
			}
		}
		if ( pvUVMap != NULL )
		{
			eslMoveMemory
				( pMesh->m_bufUVMap.GetArray( countVertex ),
					pvUVMap, countVertex * sizeof(S2DVector) ) ;
			pMesh->m_bufUVMap.FinishArray() ;
		}
		if ( pColor != NULL )
		{
			eslMoveMemory
				( pMesh->m_bufColor.GetArray( countVertex ),
					pColor, countVertex * sizeof(S3DColor) ) ;
			pMesh->m_bufColor.FinishArray() ;
		}
		if ( pIndexedList != NULL )
		{
			eslMoveMemory
				( pMesh->m_bufIndex.GetArray( countVertex ),
					pIndexedList, countIndex * sizeof(uint32_t) ) ;
			pMesh->m_bufIndex.FinishArray() ;
		}
	}
	return	S3DVertexBuffer::UpdateIndexedPrimitiveList
				( iMesh, nFlags, countIndex, countVertex,
					pvVertex, pvNormal, pvUVMap, pColor, pIndexedList ) ;
}

// サブメッシュ（ポリゴンリスト）を更新
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelBuffer::UpdateSubIndexedTriangleList
	( size_t iMesh, size_t iSubMesh, uint32_t nFlags,
		size_t countPolygon, const uint32_t * pIndexedList )
{
	MeshObject *	pMesh = m_arrMeshObj.GetAt( iMesh ) ;
	if ( (pMesh != NULL) && (iSubMesh < countSubMesh) )
	{
		pMesh->m_countSubPoly[iSubMesh] = countPolygon ;
		pMesh->m_bufSubIndex[iSubMesh].RemoveAll() ;
		pMesh->m_bufSubIndex[iSubMesh].AddArray
				( pIndexedList, countPolygon * 3 ) ;
	}
	return	S3DVertexBuffer::UpdateSubIndexedTriangleList
				( iMesh, iSubMesh, nFlags, countPolygon, pIndexedList ) ;
}

// サブメッシュ切り替えｚ座標比を設定する
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelBuffer::SetSubMeshDensity
	( size_t iMesh, float32_t fpDensity, ssize_t iSelector )
{
	MeshObject *	pMesh = m_arrMeshObj.GetAt( iMesh ) ;
	if ( pMesh != NULL )
	{
		pMesh->m_iSubMeshSelector = iSelector ;
		pMesh->m_fpSubMeshDensity = fpDensity ;
	}
	return	S3DVertexBuffer::SetSubMeshDensity( iMesh, fpDensity, iSelector ) ;
}

// 追加的な頂点属性を設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelBuffer::SetExtendVertexAttribute
	( size_t iMesh, size_t countElements,
		size_t countVertex, const float32_t * pfpAttrElements )
{
	MeshObject *	pMesh = m_arrMeshObj.GetAt( iMesh ) ;
	if ( pMesh != NULL )
	{
		size_t	nCopyVertexCount = pMesh->m_countVertex ;
		if ( countVertex < nCopyVertexCount )
		{
			nCopyVertexCount = countVertex ;
		}
		pMesh->m_nExAttrElements = countElements ;
		pMesh->m_bufExAttrElements.SetLength
					( countElements * pMesh->m_countVertex ) ;
		eslCopyMemory
			( pMesh->m_bufExAttrElements.GetArray(),
				pfpAttrElements,
				countElements * nCopyVertexCount * sizeof(float32_t) ) ;
	}
	return	S3DVertexBuffer::SetExtendVertexAttribute
				( iMesh, countElements, countVertex, pfpAttrElements ) ;
}

// バッファを S3DRenderBufferInterface へ出力
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelBuffer::RenderBufferTo
	( S3DRenderBufferInterface * render,
			uint64_t flagsExclusion, size_t iFrist, ssize_t iEnd,
			size_t nInstancing,
			const S4DMatrix * pmatInstancing,
			const S3DColor * pColorInstancing ) const
{
	if ( m_flagUpdateBones )
	{
		((S3DModelBuffer*)this)->UpdateBoneMatrix() ;
	}
	SGLError	err ;
	S3DDVector	vPos = m_boneSpace.m_vCenter + m_boneSpace.GetBoneOffset() ;
	render->PushTransformation() ;
	render->AppendMatrixTransformation
				( m_boneSpace.m_matTransformation, vPos ) ;
	err = S3DVertexBuffer::RenderBufferTo
				( render, flagsExclusion, iFrist, iEnd,
					nInstancing, pmatInstancing, pColorInstancing ) ;
	render->PopTransformation() ;
	return	err ;
}

// モデル描画が表示範囲にあるか見積もる
//////////////////////////////////////////////////////////////////////////////
bool S3DModelBuffer::IsModelIntoView
	( S3DRenderContextInterface * render,
			float32_t fpScaleMargin, float32_t fpModelMargin )
{
	S3DVector	vCenter ;
	double	r = GetCircumscribedSphere( vCenter ) ;
	//
	S3DDVector	vdCenter = vCenter ;
	vdCenter += m_boneSpace.m_vCenter + m_boneSpace.GetBoneOffset() ;
	return	render->IsSphereIntoView
				( vdCenter, r * (1.0 + fpScaleMargin) + fpModelMargin ) ;
}

// 三角ポリゴンリストの頂点順（表裏）正規化
//////////////////////////////////////////////////////////////////////////////
void S3DModelBuffer::NormalizeTriangleSurface( S3DModelBuffer::MeshObject * pMesh )
{
	if ( GetPrimitiveVertexCount( pMesh->m_typeMesh ) != 3 )
	{
		return ;
	}
	const size_t	countPolygon = pMesh->m_countPolygon ;
	S3DVector4 *	pvVertex = GetVertexBufferAt( pMesh->m_iVertex ) ;
	S3DVector4 *	pvNormal = GetNormalBufferAt( pMesh->m_iNormal ) ;
	uint32_t *		pIndexedList = pMesh->m_bufIndex.GetArray() ;
	ESLAssert( pMesh->m_bufIndex.GetLength() >= countPolygon * 3 ) ;
	//
	for ( size_t i = 0; i < countPolygon; i ++ )
	{
		S3DVector	vNormal = pvNormal[pIndexedList[0]] ;
		vNormal += pvNormal[pIndexedList[1]] ;
		vNormal += pvNormal[pIndexedList[2]] ;
		//
		S3DVector&	v0 = pvVertex[pIndexedList[0]] ;
		S3DVector&	v1 = pvVertex[pIndexedList[1]] ;
		S3DVector&	v2 = pvVertex[pIndexedList[2]] ;
		S3DVector	vPlane = ((v1 - v0) * (v2 - v0)) ;
		//
		if ( (vPlane | vNormal) < 0.0 )
		{
			// 反転
			uint32_t	t = pIndexedList[1] ;
			pIndexedList[1] = pIndexedList[2] ;
			pIndexedList[2] = t ;
		}
		//
		pIndexedList += 3 ;
	}
	pMesh->m_bufIndex.FinishArray() ;
}

// メタ情報
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelBuffer::ParseMetaInfo( S3DModelBuffer::MetaInfo& infMeta ) const
{
	infMeta.nFlags = 0 ;
	infMeta.vImportScale = S3DVector( 1, 1, 1 ) ;
	infMeta.pwszSrcFile = NULL ;
	infMeta.pxmlEditLog = NULL ;
	//
	const SXMLDocument *	pxmlMetaInfo = &m_xmlMetaInfo ;
	if ( m_xmlMetaInfo.GetType() == SXMLDocument::typeRoot )
	{
		pxmlMetaInfo = m_xmlMetaInfo.GetElementTagAs( L"meta_info" ) ;
		if ( pxmlMetaInfo == NULL )
		{
			return	sglErrFailed ;
		}
	}
	else if ( m_xmlMetaInfo.GetTag() != L"meta_info" )
	{
		return	sglErrFailed ;
	}
	//
	const SXMLDocument *
			pxmlImport = pxmlMetaInfo->GetElementTagAs( L"import" ) ;
	if ( pxmlImport != NULL )
	{
		infMeta.nFlags |= metaInfoFlagImport ;
		infMeta.vImportScale.x =
				(float32_t) pxmlImport->GetAttrRealAs( L"scale_x", 1.0f ) ;
		infMeta.vImportScale.y =
				(float32_t) pxmlImport->GetAttrRealAs( L"scale_y", 1.0f ) ;
		infMeta.vImportScale.z =
				(float32_t) pxmlImport->GetAttrRealAs( L"scale_z", 1.0f ) ;
		//
		const SString *	pstrSrcFile = pxmlImport->GetAttributeAs( L"src_file" ) ;
		if ( pstrSrcFile != NULL )
		{
			infMeta.pwszSrcFile = *pstrSrcFile ;
		}
	}
	infMeta.pxmlEditLog = pxmlMetaInfo->GetElementTagAs( L"edit_log" ) ;
	if ( infMeta.pxmlEditLog != NULL )
	{
		infMeta.nFlags |= metaInfoFlagEditLog ;
	}
	return	sglErrSuccess ;
}

void S3DModelBuffer::SetMetaInfo( const S3DModelBuffer::MetaInfo& infMeta )
{
	SXMLDocument *	pxmlMetaInfo = &m_xmlMetaInfo ;
	if ( m_xmlMetaInfo.GetType() == SXMLDocument::typeRoot )
	{
		pxmlMetaInfo = m_xmlMetaInfo.GetElementTagAs( L"meta_info" ) ;
		if ( pxmlMetaInfo == NULL )
		{
			pxmlMetaInfo = &m_xmlMetaInfo ;
			m_xmlMetaInfo.RemoveAllContents() ;
			m_xmlMetaInfo.SetTag( L"meta_info" ) ;
		}
	}
	else if ( m_xmlMetaInfo.GetTag() != L"meta_info" )
	{
		m_xmlMetaInfo.RemoveAllContents() ;
		m_xmlMetaInfo.SetTag( L"meta_info" ) ;
	}
	if ( infMeta.nFlags & metaInfoFlagImport )
	{
		SXMLDocument *	pxmlImport = pxmlMetaInfo->CreateElementTagAs( L"import" ) ;
		pxmlImport->SetAttrRealAs( L"scale_x", infMeta.vImportScale.x ) ;
		pxmlImport->SetAttrRealAs( L"scale_y", infMeta.vImportScale.y ) ;
		pxmlImport->SetAttrRealAs( L"scale_z", infMeta.vImportScale.z ) ;
		//
		if ( infMeta.pwszSrcFile != NULL )
		{
			pxmlImport->SetAttributeAs( L"src_file", infMeta.pwszSrcFile ) ;
		}
		else
		{
			pxmlImport->RemoveAttributeAs( L"src_file" ) ;
		}
	}
	if ( infMeta.nFlags & metaInfoFlagEditLog )
	{
		if ( infMeta.pxmlEditLog != NULL )
		{
			SXMLDocument *
				pxmlEditLog = pxmlMetaInfo->CreateElementTagAs( L"edit_log" ) ;
			*pxmlEditLog = *(infMeta.pxmlEditLog) ;
		}
		else
		{
			ssize_t	iEditLog = pxmlMetaInfo->FindElementTag( L"edit_log" ) ;
			if ( iEditLog >= 0 )
			{
				pxmlMetaInfo->RemoveElementAt( (size_t) iEditLog ) ;
			}
		}
	}
}

// バッファを消去
//////////////////////////////////////////////////////////////////////////////
void S3DModelBuffer::ClearBuffer( void )
{
	if ( m_pRefModel == NULL )
	{
		S3DVertexBuffer::ClearBuffer() ;
	}
	else
	{
		AttachVertexBuffer( NULL, false ) ;
		m_pRefModel = NULL ;
	}
	//
	m_arrMeshObj.RemoveAll() ;
	m_bufVertex.FreeArray() ;
	m_bufNormal.FreeArray() ;
	m_textures.RemoveAllTexture() ;
	m_materials.RemoveAllMaterial() ;
	m_poses.RemoveAllPoses() ;
	m_ssaMeshGroup.RemoveAll() ;
	m_ssoaMorphTarget.RemoveAll() ;
	m_ssaPhysMaterial.RemoveAll() ;
	m_boneRoot.RemoveAllChildren() ;
	m_boneRoot.RemoveAllItems() ;
	m_ssoaBones.RemoveAll() ;
	m_iUpdateVertex = 0 ;
	m_iUpdateNormal = 0 ;
}

// マーカーを当たり判定として追加
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DModelBuffer::AddMarkerForCollision
	( S3DCollision& render,
		const S3DModelData::MarkerInfo& mi, ssize_t iColOffset ) const
{
	uint32_t	maskSave = render.GetUserClassesMask() ;
	uint32_t	maskMarker = maskSave ;
	render.PushTransformation() ;
	//
	if ( !mi.m_strRefBone.IsEmpty() )
	{
		S3DModelBoneSpace *	pBone = GetBonePropertyAs( mi.m_strRefBone ) ;
		if ( pBone != NULL )
		{
			S3DDMatrix	matBone ;
			S3DDVector	vBone ;
			pBone->CalcBoneTransformation( matBone, vBone ) ;
			render.AppendMatrixTransformation( matBone, vBone ) ;
		}
	}
	//
	S3DDMatrix	matMarker( S3DDQuaternion( mi.m_qRotation ) ) ;
	S3DDVector	vMarker = mi.m_vPosition ;
	//
	if ( (mi.m_iCollider >= 0)
		&& (mi.m_iCollider < S3DModelBoneSpace::flagPhysExColliderMaxCount) )
	{
		if ( (iColOffset != markerInvalidOffset)
			&& (mi.m_iCollider + iColOffset >= 0) )
		{
			maskMarker = 1 << (mi.m_iCollider + iColOffset) ;
			render.SetUserClassesMask( maskMarker ) ;
		}
	}
	if ( mi.m_shape == S3DModelData::MarkerInfo::shapeSphere )
	{
		S3DVector	vPos( 0, 0, 0 ) ;
		matMarker *= S3DDMatrix( mi.m_vSize.x, mi.m_vSize.y, mi.m_vSize.z ) ;
		render.AppendMatrixTransformation( matMarker, vMarker ) ;
		render.AddSolidSphere( vPos, mi.m_fpRadius, 0 ) ;
	}
	else if ( mi.m_shape == S3DModelData::MarkerInfo::shapeCube )
	{
		S3DVector	vPos( 0, 0, 0 ) ;
		render.AppendMatrixTransformation( matMarker, vMarker ) ;
		render.AddSolidCube( vPos, mi.m_vSize, 0 ) ;
	}
	else if ( mi.m_shape == S3DModelData::MarkerInfo::shapeTube )
	{
		S3DVector	vPoints[2] ;
		vPoints[0].x = 0 ;
		vPoints[0].y = 0 ;
		vPoints[0].z = 0 ;
		vPoints[1] = mi.m_vDirection.Normalized() * mi.m_fpLength ;
		render.AppendMatrixTransformation( matMarker, vMarker ) ;
		render.AddSolidTubeList( vPoints, 2, mi.m_fpRadius ) ;
	}
	render.PopTransformation() ;
	render.SetUserClassesMask( maskSave ) ;
	return	maskMarker ;
}

size_t S3DModelBuffer::AddAllMarkerForCollision
	( S3DCollision& render,
		S3DModelData::MarkerInfo::Type type,
		const wchar_t * pwszLeadID,
		ssize_t iColOffset, uint32_t * pGetMarkerColMask )
{
	SStrSortObjectArray<S3DModelData::MarkerInfo>&
								ssoaMarker = GetMarkerInfoList() ;
	size_t		nAddCount = 0 ;
	uint32_t	maskMarkers = 0 ;
	for ( size_t i = 0; i < ssoaMarker.GetLength(); i ++ )
	{
		S3DModelData::MarkerInfo *	pmi = ssoaMarker.GetAt( i ) ;
		if ( pmi == NULL )
		{
			continue ;
		}
		if ( (pwszLeadID != NULL) && (pwszLeadID[0] != 0) )
		{
			const SString *	pstrID = ssoaMarker.GetTagAt( i ) ;
			if ( (pstrID == NULL)
				|| (pstrID->CompareLeft( pwszLeadID ) != 0) )
			{
				continue ;
			}
		}
		if ( (pmi->m_type == type)
			|| (type == S3DModelData::MarkerInfo::typeInvalid) )
		{
			maskMarkers |= AddMarkerForCollision( render, *pmi, iColOffset ) ;
			nAddCount ++ ;
		}
	}
	if ( pGetMarkerColMask != nullptr )
	{
		*pGetMarkerColMask = maskMarkers ;
	}
	return	nAddCount ;
}

// マーカーの当たり判定
//////////////////////////////////////////////////////////////////////////////
bool S3DModelBuffer::IsHitAgainstMarker
	( S3DCollider& collider,
		S3DCollisionResult& rsHit,
		const S3DDMatrix& matSpace,
		const S3DDVector& vSpace,
		const S3DModelData::MarkerInfo& mi ) const
{
	S3DDVector	vMarker ;
	float		fpRadius ;
	CalcMarkerPosition( vMarker, fpRadius, matSpace, vSpace, mi ) ;
	return	collider.IsHitAgainstSphere( vMarker, fpRadius, rsHit ) ;
}

// マーカー球座標計算
//////////////////////////////////////////////////////////////////////////////
void S3DModelBuffer::CalcMarkerPosition
	( S3DDVector& vPos, float& fpRadius,
		const S3DDMatrix& matSpace,
		const S3DDVector& vSpace,
		const S3DModelData::MarkerInfo& mi ) const
{
	S3DDMatrix	matTemp = matSpace ;
	S3DDVector	vTemp = vSpace ;
	//
	CalcMarkerTransformation( vPos, fpRadius, matTemp, vTemp, mi ) ;
}

void S3DModelBuffer::CalcMarkerTransformation
	( S3DDVector& vPos, float& fpRadius,
		S3DDMatrix& matSpace,
		S3DDVector& vSpace,
		const S3DModelData::MarkerInfo& mi ) const
{
	S3DDMatrix	matBone ;
	S3DDVector	vBone ;
	CalcMarkerReferenceBoneSpace( matBone, vBone, mi ) ;
	vSpace += matSpace * vBone ;
	matSpace *= matBone ;
	//
	vSpace += matSpace * S3DDVector( mi.m_vPosition ) ;
	matSpace *= S3DDMatrix( S3DDQuaternion( mi.m_qRotation ) ) ;
	matSpace *= S3DDMatrix( mi.m_vSize.x, mi.m_vSize.y, mi.m_vSize.z ) ;
	//
	double	d = pow( fabs( matSpace.Determinant() ), 1.0 / 3.0 ) ;
	//
	vPos = vSpace ;
	fpRadius = (mi.m_fpRadius * (float) d) ;
}

void S3DModelBuffer::CalcMarkerReferenceBoneSpace
	( S3DDMatrix& matBone,
		S3DDVector& vBone,
		const S3DModelData::MarkerInfo& mi ) const
{
	matBone = S3DDMatrix( 1, 1, 1 ) ;
	vBone = S3DDVector( 0, 0, 0 ) ;
	//
	if ( !mi.m_strRefBone.IsEmpty() )
	{
		S3DModelBoneSpace *	pBone = GetBonePropertyAs( mi.m_strRefBone ) ;
		if ( pBone != NULL )
		{
			pBone->CalcBoneTransformation( matBone, vBone ) ;
		}
	}
}

// スレッド排他処理
//////////////////////////////////////////////////////////////////////////////
void S3DModelBuffer::LockModelData( void )
{
	m_csSync.Lock() ;
}

void S3DModelBuffer::UnlockModelData( void )
{
	m_csSync.Unlock() ;
}

// 別モデルのポーズをこのモデルへ複製する
//////////////////////////////////////////////////////////////////////////////
void S3DModelBuffer::DuplicatePoseOf( const S3DModelBuffer& model )
{
	size_t	nCount = m_ssoaBones.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		const SString *		pstrID = m_ssoaBones.GetTagAt( i ) ;
		S3DModelBoneSpace *	pBone = m_ssoaBones.GetAt( i ) ;
		if ( pstrID && pBone )
		{
			S3DModelBoneSpace *	pSrcBone = model.GetBonePropertyAs( *pstrID ) ;
			if ( pSrcBone != nullptr )
			{
				pBone->SetBoneOffset( pSrcBone->GetBoneOffset() ) ;
				pBone->SetLocalTransformation( pSrcBone->m_matTransformation ) ;
			}
		}
	}
	PostUpdateBone() ;
}

// 現在のポーズを取得する
//////////////////////////////////////////////////////////////////////////////
void S3DModelBuffer::TranscribeCurrentPose( S3DModelPose * pPose ) const
{
	pPose->Lock() ;
	pPose->ResetPoseTarget() ;
	TranscribeBoneToPose( pPose ) ;
	TranscribeMorphToPose( pPose ) ;
	TranscribeMeshVisibleToPose( pPose ) ;
	pPose->Unlock() ;
}

// 現在のボーンの状態のポーズを取得する
//////////////////////////////////////////////////////////////////////////////
void S3DModelBuffer::TranscribeBoneToPose( S3DModelPose * pPose ) const
{
	size_t	nCount = m_ssoaBones.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		const SString *		pstrID = m_ssoaBones.GetTagAt( i ) ;
		S3DModelBoneSpace *	pBone = m_ssoaBones.GetAt( i ) ;
		if ( pstrID && pBone )
		{
			S3DModelPose::JointAnimation *
							pja = pPose->GetJointAs( *pstrID ) ;
			if ( pja == NULL )
			{
				pja = new S3DModelPose::JointAnimation ;
				pPose->AddJointAs( *pstrID, pja ) ;
			}
			S3DModelPose::MatrixElement	me ;
			me.FromMatrix( pBone->m_matTransformation ) ;
			pja->m_fDisabled = false ;
			pja->m_qRotation = me.qRotation ;
			pja->m_vZoom = me.vZoom ;
			pja->m_vOffset = pBone->GetBoneOffset() ;
			pja->m_wPhysBlend = pBone->GetPhysicsBlendWeight() ;
			//
			if ( pBone->GetBoneFlags() & S3DModelBoneSpace::flagBonePhysics )
			{
				pja->m_fDisabled = true ;
			}
		}
	}
}

// 現在のモーフィングの状態のポーズを取得する
//////////////////////////////////////////////////////////////////////////////
void S3DModelBuffer::TranscribeMorphToPose( S3DModelPose * pPose ) const
{
	const SSystem::SStrSortArray<MeshGroup>&
								ssaMesh = GetMeshGroupList() ;
	for ( size_t i = 0; i < ssaMesh.GetLength(); i ++ )
	{
		const SString *	pstrID = ssaMesh.GetTagAt( i ) ;
		MeshGroup *		pmg = ssaMesh.GetAt( i ) ;
		if ( pstrID && pmg )
		{
			MeshObject *	pmo = GetMeshObjectAt( pmg->m_iFirstMesh ) ;
			if ( (pmo == NULL)
				|| (pmo->m_arrMorphTarget.GetLength() == 0) )
			{
				continue ;
			}
			S3DModelPose::MorphInfo *	pmi = pPose->GetMorphingAs( *pstrID ) ;
			if ( pmi == NULL )
			{
				pmi = new S3DModelPose::MorphInfo ;
				pPose->AddMorphingAs( *pstrID, pmi ) ;
			}
			const ssize_t *		pMorphTarget = pmo->m_arrMorphTargetMesh.GetConstArray() ;
			const float32_t *	pMorphWeight = pmo->m_arrMorphApplication.GetConstArray() ;
			size_t		nTargets = pmo->m_arrMorphTargetMesh.GetLength() ;
			//
			pmi->m_fDisabled = false ;
			pmi->m_strMorphTarget = L"" ;
			pmi->m_aTargetIDs.RemoveAll() ;
			pmi->m_aAnimation.RemoveAll() ;
			//
			for ( size_t j = 0; j < nTargets; j ++ )
			{
				SString *	pstrMorphID =
					pmo->m_arrMorphTarget.GetAt( (size_t) pMorphTarget[j] ) ;
				if ( pstrMorphID != NULL )
				{
					pmi->m_aTargetIDs.Add( new SString( *pstrMorphID ) ) ;
				}
				else
				{
					pmi->m_aTargetIDs.Add( new SString() ) ;
				}
				pmi->m_aAnimation.Add( pMorphWeight[j] ) ;
			}
		}
	}
}

// 現在のメッシュ表示状態のポーズを取得する
//////////////////////////////////////////////////////////////////////////////
void S3DModelBuffer::TranscribeMeshVisibleToPose( S3DModelPose * pPose ) const
{
	const SSystem::SStrSortArray<MeshGroup>&
								ssaMesh = GetMeshGroupList() ;
	for ( size_t i = 0; i < ssaMesh.GetLength(); i ++ )
	{
		const SString *	pstrID = ssaMesh.GetTagAt( i ) ;
		MeshGroup *		pmg = ssaMesh.GetAt( i ) ;
		if ( pstrID && pmg )
		{
			MeshObject *	pmo = GetMeshObjectAt( pmg->m_iFirstMesh ) ;
			if ( pmo == NULL )
			{
				continue ;
			}
			S3DModelPose::MeshSelector *
					pms = pPose->GetMeshSelectorAs( *pstrID ) ;
			if ( pms == NULL )
			{
				pms = new S3DModelPose::MeshSelector ;
				pPose->AddMeshSelectorAs( *pstrID, pms ) ;
			}
			pms->m_flagVisible = IsEnabledToRenderMesh( pmg->m_iFirstMesh ) ;
		}
	}
}

// 現在のポーズをモーションフレームとしてサンプリングする
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelBuffer::SampleCurrentPose
	( S3DModelPose * pPose, size_t iFrame,
		const S3DDMatrix * pBaseMatrix, const S3DDVector * pBasePos ) const
{
	size_t	i ;
	for ( i = 0; i < pPose->GetJointCount(); i ++ )
	{
		S3DModelPose::JointAnimation *	pja = pPose->GetJointAt( i ) ;
		if ( pja == NULL )
		{
			continue ;
		}
		S3DModelBoneSpace *
				pBone = m_ssoaBones.GetAs( pPose->GetJointNameAt( i ) ) ;
		if ( pBone == NULL )
		{
			continue ;
		}
		S3DDMatrix	matBase( 1, 1, 1 ) ;
		S3DDVector	vBase( 0, 0, 0 ) ;
		if ( &m_boneRoot == pBone->GetParentSpace() )
		{
			if ( pBaseMatrix != NULL )
			{
				matBase = *pBaseMatrix ;
			}
			if ( pBasePos != NULL )
			{
				vBase = *pBasePos ;
			}
		}
		S3DDMatrix	matBone = matBase * pBone->m_matTransformation ;
		S3DDVector	vBone = matBase * pBone->GetBoneOffset() + vBase ;
		//
		if ( iFrame < pja->m_aMatrixs.GetLength() )
		{
			S4DMatrix	mat4 ;
			Matrix4x4From3x3
				<float32_t,S3DDMatrix,S3DDVector>( mat4, matBone, vBone ) ;
			pja->m_aMatrixs.SetAt( iFrame, mat4 ) ;
		}
		S3DModelPose::MatrixElement	me ;
		me.FromMatrix( matBone ) ;
		if ( iFrame < pja->m_aRotations.GetLength() )
		{
			pja->m_aRotations.SetAt( iFrame, me.qRotation ) ;
		}
		if ( iFrame < pja->m_aOffsets.GetLength() )
		{
			pja->m_aOffsets.SetAt( iFrame, vBone ) ;
		}
		if ( iFrame < pja->m_aZooms.GetLength() )
		{
			pja->m_aZooms.SetAt( iFrame, me.vZoom ) ;
		}
		if ( iFrame < pja->m_aPhysBlends.GetLength() )
		{
			pja->m_aPhysBlends.SetAt
				( iFrame, (float32_t) pBone->GetPhysicsBlendWeight() ) ;
		}
		if ( iFrame == 0 )
		{
			pja->m_qRotation = me.qRotation ;
			pja->m_vZoom = me.vZoom ;
			pja->m_vOffset = pBone->GetBoneOffset() ;
			pja->m_wPhysBlend = pBone->GetPhysicsBlendWeight() ;
		}
	}
	for ( i = 0; i < pPose->GetMorphingCount(); i ++ )
	{
		S3DModelPose::MorphInfo *	pmi = pPose->GetMorphingAt( i ) ;
		if ( (pmi == NULL)
			|| ((iFrame + 1) * pmi->m_aTargetIDs.GetLength()
								> pmi->m_aAnimation.GetLength()) )
		{
			continue ;
		}
		MeshGroup *	pmg =
			GetMeshGroupList().GetAs( pPose->GetMorphingMeshNameAt( i ) ) ;
		if ( pmg == NULL )
		{
			continue ;
		}
		MeshObject *	pmo = GetMeshObjectAt( pmg->m_iFirstMesh ) ;
		if ( (pmo == NULL)
			|| (pmo->m_arrMorphTarget.GetLength() == 0) )
		{
			continue ;
		}
		const ssize_t *		pMorphTarget = pmo->m_arrMorphTargetMesh.GetConstArray() ;
		const float32_t *	pMorphWeight = pmo->m_arrMorphApplication.GetConstArray() ;
		size_t		nTargets = pmo->m_arrMorphTargetMesh.GetLength() ;
		size_t		nDstTargets = pmi->m_aTargetIDs.GetLength() ;
		float32_t *	pfpWeights = pmi->m_aAnimation.GetAt( iFrame * nDstTargets ) ;
		//
		for ( size_t j = 0; j < nTargets; j ++ )
		{
			SString *		pstrMorphID =
				pmo->m_arrMorphTarget.GetAt( (size_t) pMorphTarget[j] ) ;
			ssize_t			iDstTarget = -1 ;
			const wchar_t *	pwszMorphID =
				(pstrMorphID != NULL) ? (const wchar_t*) *pstrMorphID : L"" ;
			for ( size_t k = 0; k < nDstTargets; k ++ )
			{
				SString *	pstrTargetID = pmi->m_aTargetIDs.GetAt( k ) ;
				if ( (pstrTargetID != NULL)
					&& (*pstrTargetID == pwszMorphID) )
				{
					iDstTarget = (ssize_t) k ;
					break ;
				}
			}
			if ( iDstTarget >= 0 )
			{
				pfpWeights[iDstTarget] = pMorphWeight[j] ;
			}
		}
	}
	for ( i = 0; i < pPose->GetMeshSelectorCount(); i ++ )
	{
		S3DModelPose::MeshSelector *	pms = pPose->GetMeshSelectorAt( i ) ;
		if ( (pms == NULL)
			|| (iFrame >= pms->m_aVisibles.GetLength()) )
		{
			continue ;
		}
		MeshGroup *	pmg =
			GetMeshGroupList().GetAs( pPose->GetSelectedMeshNameAt( i ) ) ;
		if ( pmg == NULL )
		{
			continue ;
		}
		pms->m_aVisibles.SetAt
			( iFrame, IsEnabledToRenderMesh( pmg->m_iFirstMesh ) ? 1 : 0 ) ;
	}
	for ( i = 0; i < pPose->GetMaterialSelectorCount(); i ++ )
	{
		S3DModelPose::MaterialSelector *	pms = pPose->GetMaterialSelectorAt( i ) ;
		if ( (pms == NULL)
			|| (iFrame >= pms->m_aAnimation.GetLength()) )
		{
			continue ;
		}
		MeshGroup *	pmg =
			GetMeshGroupList().GetAs( pPose->GetSelMaterialMeshNameAt( i ) ) ;
		if ( pmg == NULL )
		{
			continue ;
		}
		S3DMaterial *	pMaterial = GetMaterialToRenderMesh( pmg->m_iFirstMesh ) ;
		uint32_t		iMaterial = 0 ;
		if ( pMaterial != nullptr )
		{
			ssize_t	iFind = pms->m_aMaterials.FindPtr( pMaterial ) ;
			if ( iFind < 0 )
			{
				const wchar_t *	pwszMaterialID =
					GetMaterialLibrary().GetMaterialIdentityOf( pMaterial ) ;
				if ( pwszMaterialID != nullptr )
				{
					ESLAssert( pms->m_aMaterialIDs.GetLength() == pms->m_aMaterials.GetLength() ) ;
					iMaterial = (uint32_t) pms->m_aMaterialIDs.GetLength() ;
					pms->m_aMaterialIDs.Add( new SString( pwszMaterialID ) ) ;
					pms->m_aMaterials.Add( pMaterial ) ;
				}
			}
			else
			{
				iMaterial = (uint32_t) iFind ;
			}
		}
		pms->m_aAnimation.SetAt( iFrame, iMaterial ) ;
	}
	return	sglErrSuccess ;
}

// 物理演算ボーン要素（物理演算適用度1.0）ジョイントを削除るする
//////////////////////////////////////////////////////////////////////////////
void S3DModelBuffer::CleanupPhysJointOfMotion( S3DModelPose * pPose ) const
{
	for ( size_t i = 0; i < pPose->GetJointCount(); i ++ )
	{
		S3DModelBoneSpace *
				pBone = m_ssoaBones.GetAs( pPose->GetJointNameAt( i ) ) ;
		if ( pBone == nullptr )
		{
			pPose->RemoveJointAt( i -- ) ;
			continue ;
		}
		if ( !(pBone->GetBoneFlags() & S3DModelBoneSpace::flagBonePhysics) )
		{
			continue ;
		}
		S3DModelPose::JointAnimation *	pja = pPose->GetJointAt( i ) ;
		if ( pja == nullptr )
		{
			continue ;
		}
		if ( pja->m_aPhysBlends.GetLength() > 1 )
		{
			continue ;
		}
		if ( pja->m_wPhysBlend >= 0.9999 )
		{
			pPose->RemoveJointAt( i -- ) ;
			continue ;
		}
	}
}

// 指定画像をテクスチャとして参照しているか？
//////////////////////////////////////////////////////////////////////////////
bool S3DModelBuffer::IsTextureUsed( SGLImageObject * pImage ) const
{
	return	GetMaterialLibrary().IsTextureUsed( pImage ) ;
}

// 指定マテリアルは使用中か？
//////////////////////////////////////////////////////////////////////////////
bool S3DModelBuffer::IsMaterialUsed( S3DMaterial * pMaterial ) const
{
	for ( size_t i = 0; i < m_arrMeshObj.GetLength(); i ++ )
	{
		MeshObject *	pMesh = m_arrMeshObj.GetAt( i ) ;
		if ( (pMesh != NULL) && (pMesh->m_pMaterial == pMaterial) )
		{
			return	true ;
		}
	}
	return	false ;
}

// 指定モーフターゲットは使用中か？
//////////////////////////////////////////////////////////////////////////////
bool S3DModelBuffer::IsMorphTargetUsed( const wchar_t * pwszMorphID ) const
{
	for ( size_t i = 0; i < m_arrMeshObj.GetLength(); i ++ )
	{
		MeshObject *	pMesh = m_arrMeshObj.GetAt( i ) ;
		if ( pMesh != NULL )
		{
			for ( size_t j = 0; j < pMesh->m_arrMorphTarget.GetLength(); j ++ )
			{
				SString *	pstrID = pMesh->m_arrMorphTarget.GetAt( j ) ;
				if ( (pstrID != NULL) && (*pstrID == pwszMorphID) )
				{
					return	true ;
				}
			}
		}
	}
	return	false ;
}

// メッシュの裏表を法線に向きに一致させる
//////////////////////////////////////////////////////////////////////////////
size_t S3DModelBuffer::NormalizeAllMeshsFace( void )
{
	size_t	nUpdateMeshs = 0 ;
	for ( size_t iMesh = 0; iMesh < m_arrMeshObj.GetLength(); iMesh ++ )
	{
		MeshObject *	pMesh = m_arrMeshObj.GetAt( iMesh ) ;
		if ( pMesh == NULL )
		{
			continue ;
		}
		if ( pMesh->m_typeMesh == primitiveTriangle )
		{
			const size_t	countVertex = pMesh->m_countVertex ;
			const size_t	countPolygon = pMesh->m_countPolygon ;
			S3DVector4 *	pvVertex = GetVertexBufferAt( pMesh->m_iVertex ) ;
			S3DVector4 *	pvNormal = GetNormalBufferAt( pMesh->m_iNormal ) ;
			uint32_t *		pIndexed = pMesh->m_bufIndex.GetArray() ;
			ESLAssert( pMesh->m_bufIndex.GetLength() >= countPolygon * 3 ) ;
			bool	flagUpdateFace = false ;
			for ( size_t i = 0, j = 0; i < countPolygon; i ++, j += 3 )
			{
				uint32_t	i0 = pIndexed[j] ;
				uint32_t	i1 = pIndexed[j + 1] ;
				uint32_t	i2 = pIndexed[j + 2] ;
				S3DVector	v0 = pvVertex[i0] ;
				S3DVector	v1 = pvVertex[i1] ;
				S3DVector	v2 = pvVertex[i2] ;
				S3DVector	n0 = pvNormal[i0] ;
				S3DVector	n1 = pvNormal[i1] ;
				S3DVector	n2 = pvNormal[i2] ;
				S3DVector	vNormal = n0 + n1 + n2 ;
				if ( vNormal.InnerProduct( (v1 - v0) * (v2 - v0) ) < 0.0f )
				{
					pIndexed[j + 1] = i2 ;
					pIndexed[j + 2] = i1 ;
					flagUpdateFace = true ;
				}
			}
			if ( flagUpdateFace )
			{
				UpdateIndexedTriangleList
					( iMesh, 0, countPolygon, countVertex,
						NULL, NULL, NULL, NULL, pIndexed ) ;
				nUpdateMeshs ++ ;
			}
			pMesh->m_bufIndex.FinishArray() ;
		}
	}
	return	nUpdateMeshs ;
}

// メッシュ名変更
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelBuffer::ModifyMeshIdentity
	( const wchar_t * pwszOldID, const wchar_t * pwszNewID )
{
	if ( m_ssaMeshGroup.GetAs( pwszNewID ) != NULL )
	{
		return	sglErrFailed ;
	}
	MeshGroup *	pmg = m_ssaMeshGroup.GetAs( pwszOldID ) ;
	if ( pmg == NULL )
	{
		return	sglErrFailed ;
	}
	MeshGroup	mgTemp = *pmg ;
	m_ssaMeshGroup.RemoveAs( pwszOldID ) ;
	m_ssaMeshGroup.Add( pwszNewID, mgTemp ) ;
	//
	m_poses.ChangeTargetMeshID( pwszOldID, pwszNewID ) ;
	return	sglErrSuccess ;
}

// 表裏反転
//////////////////////////////////////////////////////////////////////////////
void S3DModelBuffer::InverseMeshFace( size_t iMesh, bool fInverseNormal )
{
	MeshObject *	pMesh = m_arrMeshObj.GetAt( iMesh ) ;
	if ( pMesh == NULL )
	{
		return ;
	}
	if ( pMesh->m_typeMesh == primitiveTriangle )
	{
		uint32_t *		pIndexed = pMesh->m_bufIndex.GetArray() ;
		const size_t	countPolygon = pMesh->m_countPolygon ;
		ESLAssert( pMesh->m_bufIndex.GetLength() >= countPolygon * 3 ) ;
		for ( size_t i = 0, j = 0; i < countPolygon; i ++, j += 3 )
		{
			uint32_t	k = pIndexed[j + 1] ;
			pIndexed[j + 1] = pIndexed[j + 2] ;
			pIndexed[j + 2] = k ;
		}
		S3DVector4 *	pvNormal = GetNormalBufferAt( pMesh->m_iNormal ) ;
		const size_t	countVertex = pMesh->m_countVertex ;
		if ( fInverseNormal )
		{
			ESLAssert( pMesh->m_iNormal + countVertex <= GetNormalBufferLength() ) ;
			for ( size_t i = 0; i < countVertex; i ++ )
			{
				pvNormal[i].x = - pvNormal[i].x ;
				pvNormal[i].y = - pvNormal[i].y ;
				pvNormal[i].z = - pvNormal[i].z ;
			}
//			CommitNormalBuffer( pMesh->m_iNormal, countVertex ) ;
		}
		UpdateIndexedTriangleList
			( iMesh, 0, countPolygon, countVertex,
				NULL, pvNormal, NULL, NULL, pIndexed ) ;
		//
		pMesh->m_bufIndex.FinishArray() ;
	}
}

// 重複頂点の法線を揃える
//////////////////////////////////////////////////////////////////////////////
size_t S3DModelBuffer::MergeNormalOfRedundantVertex
	( size_t iMesh, size_t nMeshCount,
		double cosThreshold, double fpError,
		bool flagRefLocalMesh, bool flagUVMatch )
{
	S3DVector4 *	pvVertexBuf = GetVertexBufferAt( 0 ) ;
	S3DVector4 *	pvNormalBuf = GetNormalBufferAt( 0 ) ;
	const size_t	countVertexBuf = GetVertexBufferLength() ;
	size_t			limitVertexBuf = countVertexBuf ;
	bool			flagUpdateBuf = false ;
	SBitArray		bufVertBitMask ;			// 処理済頂点ビット配列
	size_t			nProcessedCount = 0 ;
	//
	bufVertBitMask.SetLength( countVertexBuf ) ;
	bufVertBitMask.Clear() ;
	//
	if ( flagRefLocalMesh )
	{
		MeshObject *	pMesh = m_arrMeshObj.GetAt( iMesh + nMeshCount - 1 ) ;
		if ( pMesh != NULL )
		{
			limitVertexBuf = pMesh->m_iVertex + pMesh->m_countVertex ;
		}
	}
	//
	for ( size_t i = 0; i < nMeshCount; i ++ )
	{
		MeshObject *	pMesh = m_arrMeshObj.GetAt( iMesh + i ) ;
		if ( pMesh == NULL )
		{
			continue ;
		}
		S3DVector4 *		pvVertex = pvVertexBuf + pMesh->m_iVertex ;
		S3DVector4 *		pvNormal = pvNormalBuf + pMesh->m_iNormal ;
		const S2DVector *	pvUV = pMesh->m_bufUVMap.GetConstArray() ;
		const size_t		countVertex = pMesh->m_countVertex ;
		SArray<size_t>		bufDoubleIndex ;
		bool				flagUpdateMesh = false ;
		//
		for ( size_t j = 0; j < countVertex; j ++ )
		{
			if ( bufVertBitMask.GetAt( pMesh->m_iVertex + j ) )
			{
				// 二重処理はしない
				continue ;
			}
			S3DVector	vVertex = pvVertex[j] ;
			S3DVector	vNormal = pvNormal[j] ;
			S2DVector	vUV ;
			if ( flagUVMatch && pvUV )
			{
				vUV = pvUV[j] ;
			}
			vNormal.Normalize() ;
			//
			bufVertBitMask.SetAt( pMesh->m_iVertex + j, true ) ;
			bufDoubleIndex.RemoveAll() ;
			//
			for ( size_t k = pMesh->m_iVertex + j + 1;
									k < limitVertexBuf; k ++ )
			{
				if ( bufVertBitMask.GetAt( k ) )
				{
					continue ;
				}
				S3DVector	vDelta = pvVertexBuf[k] ;
				vDelta -= vVertex ;
				if ( vDelta.InnerProduct( vDelta ) > fpError * fpError )
				{
					continue ;
				}
				S3DVector	vNormalTemp = pvNormalBuf[k] ;
				vNormalTemp.Normalize() ;
				if ( vNormal.InnerProduct( vNormalTemp ) < cosThreshold )
				{
					continue ;
				}
				if ( flagUVMatch && pvUV )
				{
					if ( pvUV[k - pMesh->m_iVertex] != vUV )
					{
						continue ;
					}
				}
				bufVertBitMask.SetAt( k, true ) ;
				bufDoubleIndex.Add( k ) ;
				vNormal += pvNormalBuf[k] ;
			}
			if ( bufDoubleIndex.GetLength() >= 1 )
			{
				vNormal.Normalize() ;
				//
				const size_t *	pDoubleIndex = bufDoubleIndex.GetConstArray() ;
				for ( size_t k = 0; k < bufDoubleIndex.GetLength(); k ++ )
				{
					pvNormalBuf[pDoubleIndex[k]] = vNormal ;
				}
				pvNormal[j] = vNormal ;
				flagUpdateMesh = true ;
				nProcessedCount += bufDoubleIndex.GetLength() + 1 ;
			}
		}
		if ( flagUpdateMesh )
		{
			flagUpdateBuf = true ;
			//
			UpdateIndexedTriangleList
				( iMesh, 0, pMesh->m_countPolygon, countVertex,
					NULL, pvNormal, NULL, NULL, NULL ) ;
		}
	}
	if ( flagUpdateBuf )
	{
//		CommitNormalBuffer( 0, GetNormalBufferLength() ) ;
	}
	return	nProcessedCount ;
}

// 重複頂点のボーンウェイトを揃える
//////////////////////////////////////////////////////////////////////////////
size_t S3DModelBuffer::MergeBoneWeightOfRedundantVertex
	( size_t iMesh, size_t nMeshCount, double fpError )
{
	S3DVector4 *		pvVertexBuf = GetVertexBufferAt( 0 ) ;
	bool				flagUpdateBuf = false ;
	SBitArray			bufVertBitMask ;			// 処理済頂点ビット配列
	SArray<float32_t>	bufWeight ;
	size_t				nProcessedCount = 0 ;
	//
	for ( size_t i = 0; i < nMeshCount; i ++ )
	{
		MeshObject *	pMesh = m_arrMeshObj.GetAt( iMesh + i ) ;
		if ( pMesh == NULL )
		{
			continue ;
		}
		if ( pMesh->m_arrRelBone.GetLength() == 0 )
		{
			continue ;
		}
		S3DVector4 *	pvVertex = pvVertexBuf + pMesh->m_iVertex ;
		const size_t	countVertex = pMesh->m_countVertex ;
		SArray<size_t>	bufDoubleIndex ;
		//
		bufVertBitMask.SetLength( countVertex ) ;
		bufVertBitMask.Clear() ;
		//
		for ( size_t j = 0; j < countVertex; j ++ )
		{
			if ( bufVertBitMask.GetAt( j ) )
			{
				// 二重処理はしない
				continue ;
			}
			S3DVector	vVertex = pvVertex[j] ;
			//
			bufVertBitMask.SetAt( j, true ) ;
			bufDoubleIndex.RemoveAll() ;
			//
			for ( size_t k = 0; k < countVertex; k ++ )
			{
				if ( bufVertBitMask.GetAt( k ) )
				{
					continue ;
				}
				S3DVector	vDelta = pvVertexBuf[k] ;
				vDelta -= vVertex ;
				if ( vDelta.InnerProduct( vDelta ) > fpError * fpError )
				{
					continue ;
				}
				bufVertBitMask.SetAt( k, true ) ;
				bufDoubleIndex.Add( pMesh->m_iVertex + k ) ;
			}
			if ( bufDoubleIndex.GetLength() > 1 )
			{
				const size_t *	pDoubledIndex ;
				size_t			nDoubledCount ;
				bufDoubleIndex.Add( pMesh->m_iVertex + j ) ;
				pDoubledIndex = bufDoubleIndex.GetConstArray() ;
				nDoubledCount = bufDoubleIndex.GetLength() ;
				//
				// 重み平均を計算
				//
				size_t		nRelBoneCount = pMesh->m_arrRelBone.GetLength() ;
				float32_t *	pfpWeightAvg = bufWeight.GetArray( nRelBoneCount ) ;
				eslFillMemory
					( pfpWeightAvg, 0,
						nRelBoneCount * sizeof(float32_t) ) ;
				//
				for ( size_t k = 0; k < nRelBoneCount; k ++ )
				{
					S3DModelData::BONE_LINK_INFO *
							pbli = pMesh->m_arrRelBone.GetAt( k ) ;
					ESLAssert( pbli != NULL ) ;
					if ( (pbli == NULL) || (pbli->pRelBone == NULL) )
					{
						continue ;
					}
					S3DModelBoneSpace *	pBone = pbli->pRelBone ;
//					const float32_t *
//							pfpWeightMap = pBone->GetBoneWeightMap() ;
//					size_t	iWeight = pBone->VertexIndexOfBoneWeight() ;
//					size_t	nWeightCount = pBone->VertexCountOfBoneWeight() ;
					float32_t	w = 0.0 ;
					size_t		n = 0 ;
					for ( size_t l = 0; l < nDoubledCount; l ++ )
					{
						size_t	iVert = pDoubledIndex[l] ;
//						ESLAssert( (iWeight <= iVert) && (iVert < iWeight + nWeightCount) ) ;
//						if ( (iVert < iWeight)
//							|| (iWeight + nWeightCount <= iVert) )
//						{
//							continue ;
//						}
//						w += pfpWeightMap[iVert - iWeight] ;
						w += pBone->GetBoneWeightAt( iVert ) ;
						n ++ ;
					}
					if ( n > 1 )
					{
						w /= (float32_t) n ;
					}
					pfpWeightAvg[k] = w ;
				}
				//
				// 正規化
				//
				float32_t	w = 0.0 ;
				for ( size_t k = 0; k < nRelBoneCount; k ++ )
				{
					w += pfpWeightAvg[k] ;
				}
				for ( size_t k = 0; k < nRelBoneCount; k ++ )
				{
					pfpWeightAvg[k] /= w ;
				}
				//
				// 重みを全ての頂点に上書き
				//
				for ( size_t k = 0; k < nRelBoneCount; k ++ )
				{
					S3DModelData::BONE_LINK_INFO *
							pbli = pMesh->m_arrRelBone.GetAt( k ) ;
					ESLAssert( pbli != NULL ) ;
					if ( (pbli == NULL) || (pbli->pRelBone == NULL) )
					{
						continue ;
					}
					S3DModelBoneSpace *	pBone = pbli->pRelBone ;
//					const float32_t *
//							pfpWeightMap = pBone->GetBoneWeightMap() ;
//					size_t	iWeight = pBone->VertexIndexOfBoneWeight() ;
//					size_t	nWeightCount = pBone->VertexCountOfBoneWeight() ;
					for ( size_t l = 0; l < nDoubledCount; l ++ )
					{
						size_t	iVert = pDoubledIndex[l] ;
//						ESLAssert( (iWeight <= iVert) && (iVert < iWeight + nWeightCount) ) ;
//						if ( (iVert < iWeight)
//							|| (iWeight + nWeightCount <= iVert) )
//						{
//							continue ;
//						}
//						pBone->UpdateBoneWeight
//							( iVert, 1, &pfpWeightAvg[k] ) ;
						pBone->SetBoneWeightAt( iVert, pfpWeightAvg[k] ) ;
					}
				}
				bufWeight.FinishArray() ;
				flagUpdateBuf = true ;
				nProcessedCount += nDoubledCount ;
			}
		}
	}
	if ( flagUpdateBuf )
	{
		BuildupBoneRelation() ;
	}
	return	nProcessedCount ;
}

// 未使用頂点を削除する
//////////////////////////////////////////////////////////////////////////////
void S3DModelBuffer::TrimUnusedVertex( void )
{
	//
	// 圧縮マップ作成
	//
	SArray<ssize_t>	 arrIndexMap ;		// source -> destination
	size_t		nTotalVertexCount = m_bufVertex.GetLength() ;
	ssize_t *	pIndexMap = arrIndexMap.GetArray( nTotalVertexCount ) ;
	for ( size_t i = 0; i < nTotalVertexCount; i ++ )
	{
		pIndexMap[i] = -1 ;
	}
	for ( size_t i = 0; i < m_arrMeshObj.GetLength(); i ++ )
	{
		MeshObject *	pmo = m_arrMeshObj.GetAt( i ) ;
		ESLAssert( pmo != NULL ) ;
		if ( pmo == NULL )
		{
			continue ;
		}
		S3DTemporaryIndexTriangleStrip	tits ;
		const uint32_t *	pIndex = pmo->m_bufIndex.GetConstArray() ;
		if ( pmo->m_typeMesh == primitiveTriangleStrip )
		{
			pIndex = tits.MakeIndexList( pmo->m_countPolygon ) ;
		}
		size_t	nIndexCount = pmo->m_countPolygon * 3 ;
		for ( size_t j = 0; j < nIndexCount; j ++ )
		{
			size_t	k = (size_t) (pmo->m_iVertex + pIndex[j]) ;
			ESLAssert( k < nTotalVertexCount ) ;
			pIndexMap[k] = (ssize_t) k ;
		}
	}
	size_t	nTotalUsed = 0 ;
	for ( size_t i = 0; i < nTotalVertexCount; i ++ )
	{
		if ( pIndexMap[i] != -1 )
		{
			pIndexMap[i] = (ssize_t) (nTotalUsed ++) ;
		}
	}
	arrIndexMap.FinishArray() ;
	if ( nTotalUsed == nTotalVertexCount )
	{
		return ;
	}
	TrimRemapVertex
		( arrIndexMap.GetConstArray(), nTotalVertexCount, nTotalUsed ) ;
}

void S3DModelBuffer::TrimRemapVertex
		( const ssize_t * pIndexMap, size_t nIndexLength, size_t nTotalUsed )
{
	//
	// 頂点バッファを圧縮
	//
	SArray<S3DVector4>	bufVertexTemp ;
	SArray<S3DVector4>	bufNormalTemp ;
	S3DVector4 *	pvVertex = m_bufVertex.GetArray() ;
	S3DVector4 *	pvNormal = m_bufNormal.GetArray() ;
	size_t			nTotalVertexCount = m_bufVertex.GetLength() ;
	//
	bufVertexTemp.AddArray( pvVertex, nTotalVertexCount ) ;
	bufNormalTemp.AddArray( pvNormal, nTotalVertexCount ) ;
	//
	const S3DVector4 *	pvVertexTemp = bufVertexTemp.GetConstArray() ;
	const S3DVector4 *	pvNormalTemp = bufNormalTemp.GetConstArray() ;
	//
	for ( size_t i = 0; i < nTotalVertexCount; i ++ )
	{
		ssize_t	j = pIndexMap[i] ;
		if ( j != -1 )
		{
			pvVertex[j] = pvVertexTemp[i] ;
			pvNormal[j] = pvNormalTemp[i] ;
		}
	}
	m_bufVertex.FinishArray() ;
	m_bufNormal.FinishArray() ;
	m_bufVertex.SetLength( nTotalUsed ) ;
	m_bufNormal.SetLength( nTotalUsed ) ;
	//
	CommitVertexBuffer( 0, nTotalUsed ) ;
	CommitNormalBuffer( 0, nTotalUsed ) ;
	//
	// 各メッシュを圧縮
	//
	SStrSortArray<MorphTargetMesh*>	ssaRemapped ;
	for ( size_t i = 0; i < m_arrMeshObj.GetLength(); i ++ )
	{
		MeshObject *	pmo = m_arrMeshObj.GetAt( i ) ;
		ESLAssert( pmo != NULL ) ;
		if ( pmo == NULL )
		{
			continue ;
		}
		size_t	iOrgBaseVertex = pmo->m_iVertex ;
		size_t	countVertex = pmo->m_countVertex ;
		//
		// 頂点範囲
		//
		size_t	iNextVertex = (size_t) -1 ;
		size_t	iLastVertex = (size_t) -1 ;
		for ( size_t j = 0; j < countVertex; j ++ )
		{
			ssize_t	k = pIndexMap[j + iOrgBaseVertex] ;
			if ( k != -1 )
			{
				if ( (iNextVertex == (size_t) -1) || ((size_t) k < iNextVertex) )
				{
					iNextVertex = (size_t) k ;
				}
				if ( (iLastVertex == (size_t) -1) || ((size_t) k > iLastVertex) )
				{
					iLastVertex = (size_t) k ;
				}
			}
		}
		size_t	nUsed = iLastVertex - iNextVertex + 1 ;
		//
		if ( pmo->m_bufUVMap.GetLength() >= countVertex )
		{
			//
			// UV マップを圧縮
			//
			SArray<S2DVector>	bufUVMapTemp = pmo->m_bufUVMap ;
			S2DVector *			pvUVMap = pmo->m_bufUVMap.GetArray( nUsed ) ;
			const S2DVector *	pvUVMapTemp = bufUVMapTemp.GetConstArray() ;
			for ( size_t j = 0; j < countVertex; j ++ )
			{
				ssize_t	k = pIndexMap[j + iOrgBaseVertex] ;
				if ( k != -1 )
				{
					ESLAssert( k - iNextVertex >= 0 ) ;
					ESLAssert( (size_t) k - iNextVertex < nUsed ) ;
					pvUVMap[k - iNextVertex] = pvUVMapTemp[j] ;
				}
			}
			pmo->m_bufUVMap.FinishArray() ;
			pmo->m_bufUVMap.SetLength( nUsed ) ;
		}
		if ( pmo->m_bufColor.GetLength() >= countVertex )
		{
			//
			// 頂点色を圧縮
			//
			SArray<S3DColor>	bufColorTemp = pmo->m_bufColor ;
			S3DColor *			pColor = pmo->m_bufColor.GetArray( nUsed ) ;
			const S3DColor *	pColorTemp = bufColorTemp.GetConstArray() ;
			for ( size_t j = 0; j < countVertex; j ++ )
			{
				ssize_t	k = pIndexMap[j + iOrgBaseVertex] ;
				if ( k != -1 )
				{
					ESLAssert( k - iNextVertex >= 0 ) ;
					ESLAssert( (size_t) k - iNextVertex < nUsed ) ;
					pColor[k - iNextVertex] = pColorTemp[j] ;
				}
			}
			pmo->m_bufColor.FinishArray() ;
			pmo->m_bufColor.SetLength( nUsed ) ;
		}
		if ( pmo->m_typeMesh == primitiveTriangle )
		{
			//
			// インデックスリストを再マップ
			//
			uint32_t *	pIndex = pmo->m_bufIndex.GetArray() ;
			size_t		nIndexCount = pmo->m_bufIndex.GetLength() ;
			for ( size_t j = 0; j < nIndexCount; j ++ )
			{
				ssize_t	k = pIndexMap[pIndex[j] + iOrgBaseVertex] ;
				ESLAssert( k != -1 ) ;
				ESLAssert( k - iNextVertex >= 0 ) ;
				pIndex[j] = (uint32_t) (k - iNextVertex) ;
			}
			pmo->m_bufIndex.FinishArray() ;
		}
		//
		// モーフィングターゲット
		//
		for ( size_t j = 0; j < pmo->m_arrMorphTarget.GetLength(); j ++ )
		{
			SString *	pstrTarget = pmo->m_arrMorphTarget.GetAt( j ) ;
			ESLAssert( pstrTarget != NULL ) ;
			if ( pstrTarget == NULL )
			{
				continue ;
			}
			if ( ssaRemapped.GetAs( *pstrTarget ) != NULL )
			{
				continue ;
			}
			MorphTargetMesh *	pmtm = m_ssoaMorphTarget.GetAs( *pstrTarget ) ;
			ESLAssert( pmtm != NULL ) ;
			if ( pmtm == NULL )
			{
				continue ;
			}
			if ( pmtm->m_countVertex != pmo->m_countVertex )
			{
				continue ;
			}
			ssaRemapped.Add( *pstrTarget, pmtm ) ;
			//
			SArray<S3DVector4>	bufVertexTemp = pmtm->m_bufVertex ;
			SArray<S3DVector4>	bufNormalTemp = pmtm->m_bufNormal ;
			SArray<S2DVector>	bufUVMapTemp = pmtm->m_bufUVMap ;
			SArray<S3DColor>	bufColorTemp = pmtm->m_bufColor ;
			//
			S3DVector4 *		pvMorphVertex = pmtm->m_bufVertex.GetArray() ;
			const S3DVector4 *	pvMorphVertexTemp = bufVertexTemp.GetConstArray() ;
			S3DVector4 *		pvMorphNormal = pmtm->m_bufNormal.GetArray() ;
			const S3DVector4 *	pvMorphNormalTemp = bufNormalTemp.GetConstArray() ;
			S2DVector *			pvMorphUVMap = pmtm->m_bufUVMap.GetArray() ;
			const S2DVector *	pvMorphUVMapTemp = bufUVMapTemp.GetConstArray() ;
			S3DColor *			pvMorphColor = pmtm->m_bufColor.GetArray() ;
			const S3DColor *	pvMorphColorTemp = bufColorTemp.GetConstArray() ;
			//
			if ( pmtm->m_bufNormal.GetLength() < countVertex )
			{
				pvMorphNormal = NULL ;
			}
			if ( pmtm->m_bufUVMap.GetLength() < countVertex )
			{
				pvMorphUVMap = NULL ;
			}
			if ( pmtm->m_bufColor.GetLength() < countVertex )
			{
				pvMorphColor = NULL ;
			}
			for ( size_t j = 0; j < countVertex; j ++ )
			{
				ssize_t	k = pIndexMap[j + iOrgBaseVertex] ;
				if ( k != -1 )
				{
					ESLAssert( k - iNextVertex >= 0 ) ;
					ESLAssert( (size_t) k - iNextVertex < nUsed ) ;
					pvMorphVertex[k - iNextVertex] = pvMorphVertexTemp[j] ;
					if ( pvMorphNormal != NULL )
					{
						pvMorphNormal[k - iNextVertex] = pvMorphNormalTemp[j] ;
					}
					if ( pvMorphUVMap != NULL )
					{
						pvMorphUVMap[k - iNextVertex] = pvMorphUVMapTemp[j] ;
					}
					if ( pvMorphColor != NULL )
					{
						pvMorphColor[k - iNextVertex] = pvMorphColorTemp[j] ;
					}
				}
			}
			pmtm->m_bufVertex.FinishArray() ;
			pmtm->m_bufNormal.FinishArray() ;
			pmtm->m_bufUVMap.FinishArray() ;
			pmtm->m_bufColor.FinishArray() ;
			//
			pmtm->m_bufVertex.SetLength( nUsed ) ;
			if ( pvMorphNormal != NULL )
			{
				pmtm->m_bufNormal.SetLength( nUsed ) ;
			}
			if ( pvMorphUVMap != NULL )
			{
				pmtm->m_bufUVMap.SetLength( nUsed ) ;
			}
			if ( pvMorphColor != NULL )
			{
				pmtm->m_bufColor.SetLength( nUsed ) ;
			}
		}
		//
		// 次へ
		//
		pmo->m_iVertex = (ssize_t) iNextVertex ;
		pmo->m_iNormal = (ssize_t) iNextVertex ;
		pmo->m_countVertex = nUsed ;
	}
	//
	// ボーン
	//
	for ( size_t i = 0; i < m_ssoaBones.GetLength(); i ++ )
	{
		S3DModelBoneSpace *	pBone = m_ssoaBones.GetAt( i ) ;
		ESLAssert( pBone != NULL ) ;
		if ( pBone == NULL )
		{
			continue ;
		}
		//
		// 影響範囲取得
		//
//		ESLAssert( pBone->m_iTargetVertex == pBone->m_iTargetNormal ) ;
		size_t		iMappedVertex = (size_t) -1 ;
		size_t		iLastMappedVertex = (size_t) -1 ;
		for ( size_t j = 0; j < pBone->VertexCountOfBoneWeight(); j ++ )
		{
			ESLAssert( pBone->VertexIndexOfBoneWeight() + j < nIndexLength ) ;
			ssize_t	k = pIndexMap[pBone->VertexIndexOfBoneWeight() + j] ;
			if ( k != -1 )
			{
				if ( (iMappedVertex == (size_t) -1)
						|| (iMappedVertex > (size_t) k) )
				{
					iMappedVertex = (size_t) k ;
				}
				if ( (iLastMappedVertex == (size_t) -1)
						|| (iLastMappedVertex < (size_t) k) )
				{
					iLastMappedVertex = (size_t) k ;
				}
			}
		}
		if ( iMappedVertex == (size_t) -1 )
		{
			continue ;
		}
		//
		// 影響範囲正規化
		//
		for ( size_t j = 0; j < m_arrMeshObj.GetLength(); j ++ )
		{
			MeshObject *	pmo = m_arrMeshObj.GetAt( j ) ;
			ESLAssert( pmo != NULL ) ;
			if ( pmo == NULL )
			{
				continue ;
			}
			if ( (pmo->m_iVertex <= (ssize_t) iMappedVertex)
				&& (iMappedVertex < (size_t) pmo->m_iVertex + pmo->m_countVertex) )
			{
				iMappedVertex = (size_t) pmo->m_iVertex ;
			}
			if ( (pmo->m_iVertex <= (ssize_t) iLastMappedVertex)
				&& (iLastMappedVertex < (size_t) pmo->m_iVertex + pmo->m_countVertex) )
			{
				iLastMappedVertex = (size_t) pmo->m_iVertex + pmo->m_countVertex - 1 ;
			}
		}
		//
		// ウェイトマップ写像
		//
		SArray<float32_t>	bufWeightTemp ;
		size_t	iBoneFirst = pBone->VertexIndexOfBoneWeight() ;
		size_t	iBoneLength = pBone->VertexCountOfBoneWeight() ;
		pBone->ReadBoneWeightMap
			( bufWeightTemp.GetArray( iBoneLength ), iBoneFirst, iBoneLength ) ;
		bufWeightTemp.FinishArray() ;
		//
		size_t	nUsed = iLastMappedVertex - iMappedVertex + 1 ;
		pBone->ExpandBoneWeightBounds( iMappedVertex, nUsed ) ;
		//
		size_t	nChopLeft = 0 ;
		size_t	nChopRight = 0 ;
		if ( pBone->VertexIndexOfBoneWeight() < iMappedVertex )
		{
			nChopLeft = iMappedVertex - pBone->VertexIndexOfBoneWeight() ;
		}
		if ( pBone->VertexIndexOfBoneWeight()
			+ pBone->VertexCountOfBoneWeight() > iMappedVertex + nUsed )
		{
			nChopRight = pBone->VertexIndexOfBoneWeight()
						+ pBone->VertexCountOfBoneWeight()
						- (iMappedVertex + nUsed) ;
		}
		pBone->ChopBoneWeightBounds( nChopLeft, nChopRight ) ;
		//
		size_t		iLockFirst = iMappedVertex ;
		size_t		nLockCount = nUsed ;
		float32_t *	pWeightBuf =
			pBone->LockBoneWeightMap( iLockFirst, nLockCount ) ;
		ESLAssert( iLockFirst == iMappedVertex ) ;
		ESLAssert( nLockCount >= nUsed ) ;
		eslFillMemory( pWeightBuf, 0, nLockCount * sizeof(float32_t) ) ;
		//
		const float32_t *	pWeightTemp = bufWeightTemp.GetConstArray() ;
		for ( size_t j = 0; j < iBoneLength; j ++ )
		{
			ssize_t	k = pIndexMap[iBoneFirst + j] ;
			if ( k != -1 )
			{
				ESLAssert( k - iMappedVertex >= 0 ) ;
				ESLAssert( (size_t) k - iMappedVertex < nUsed ) ;
				pWeightBuf[k - iMappedVertex] = pWeightTemp[j] ;
			}
		}
		pBone->UnlockBoneWeightMap( true ) ;
	}
	RebuildVertexBuffer() ;
}

// ボーンのウェイトマップサイズをメッシュ境界に合うように正規化する
//////////////////////////////////////////////////////////////////////////////
void S3DModelBuffer::NormalizeBoneWeightMapRange( void )
{
	for ( size_t i = 0; i < m_ssoaBones.GetLength(); i ++ )
	{
		S3DModelBoneSpace *	pBone = m_ssoaBones.GetAt( i ) ;
		ESLAssert( pBone != NULL ) ;
		if ( pBone == NULL )
		{
			continue ;
		}
		//
		// 前後の０要素削除
		//
		pBone->TrimBoneWeightBounds() ;
		//
		// 参照メッシュ境界判定
		//
		size_t	iVertex = pBone->VertexIndexOfBoneWeight() ;
		size_t	iLast = pBone->VertexIndexOfBoneWeight()
						+ pBone->VertexCountOfBoneWeight() - 1 ;
//		size_t	iVertex = pBone->m_iTargetVertex ;
//		size_t	iLast = pBone->m_iTargetVertex + pBone->m_nTargetCount - 1 ;
		for ( size_t j = 0; j < pBone->m_arrRefMesh.GetLength(); j ++ )
		{
			S3DModelBoneSpace::REF_MESH_INFO *
						prmi = pBone->m_arrRefMesh.GetAt( j ) ;
			if ( prmi == NULL )
			{
				continue ;
			}
			MeshObject *	pmo = m_arrMeshObj.GetAt( prmi->iMesh ) ;
			if ( pmo == NULL )
			{
				pBone->m_arrRefMesh.RemoveAt( j -- ) ;
				continue ;
			}
			if ( pmo->m_iVertex <= (ssize_t) iVertex )
			{
				iVertex = (size_t) pmo->m_iVertex ;
			}
			if ( iLast < (size_t) pmo->m_iVertex + pmo->m_countVertex )
			{
				iLast = (size_t) pmo->m_iVertex + pmo->m_countVertex - 1 ;
			}
		}
		//
		// ウェイトマップの拡張（メッシュ境界合わせ）
		//
		if ( iLast - iVertex + 1 > pBone->VertexCountOfBoneWeight() )
		{
			pBone->ExpandBoneWeightBounds( iVertex, iLast - iVertex + 1 ) ;
		}
	}
}

// マテリアル統合
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelBuffer::MergeMaterials
	( S3DMaterial*const* ppMaterials, size_t nCount,
		const wchar_t * pwszTextureBaseID, const wchar_t * pwszBackTextureBaseID )
{
	if ( nCount == 0 )
	{
		return	sglErrFailed ;
	}
	//
	// テクスチャの統合
	//
	SArray<SGLImageRect>	aTextureRects ;
	uint64_t		nFaceTextureFlags = 0 ;
	uint64_t		nBackTextureFlags = 0 ;
	uint32_t		nFaceTextures = 0 ;
	uint32_t		nBackTextures = 0 ;
	SGLImageRect *	pTextureRects = aTextureRects.GetArray( nCount ) ;
	bool			flagBackSurface = false ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		S3DMaterial *	pMaterial = ppMaterials[i] ;
		ESLAssert( pMaterial != nullptr ) ;
		//
		pTextureRects[i].SetPosition( SGLPoint( 0, 0 ) ) ;
		pTextureRects[i].SetSize( SGLSize( 0, 0 ) ) ;
		//
		if ( !flagBackSurface
			&& pMaterial->IsEnabledBackSurfaceAttribute() )
		{
			if ( i >= 1 )
			{
				S3DSurfaceAttribute	attrBack ;
				pMaterial->GetBackSurfaceAttribute( attrBack ) ;
				ppMaterials[0]->EnableBackSurfaceAttribute( true ) ;
				ppMaterials[0]->SetBackSurfaceAttribute( attrBack ) ;
			}
			flagBackSurface = true ;
		}
		//
		for ( int j = 0; j < S3DMaterial::textureMaxCount; j ++ )
		{
			SGLImageObject *	pImage = pMaterial->GetTexture( j ) ;
			if ( pImage != nullptr )
			{
				SGLSize	sizeImage = pImage->GetImageSize() ;
				pTextureRects[i].w =
					(int32_t) esl_max( pTextureRects[i].w, sizeImage.w ) ;
				pTextureRects[i].h =
					(int32_t) esl_max( pTextureRects[i].h, sizeImage.h ) ;
				//
				uint32_t	nType = pMaterial->GetTextureType( j ) ;
				nFaceTextureFlags |=
					S3DMaterial::ShadingFlagOfTextureType( nType ) ;
				nFaceTextures |= 1 << nType ;
			}
			pImage = pMaterial->GetBackTexture( j ) ;
			if ( pImage != nullptr )
			{
				SGLSize	sizeImage = pImage->GetImageSize() ;
				pTextureRects[i].w =
					(int32_t) esl_max( pTextureRects[i].w, sizeImage.w ) ;
				pTextureRects[i].h =
					(int32_t) esl_max( pTextureRects[i].h, sizeImage.h ) ;
				//
				uint32_t	nType = pMaterial->GetBackTextureType( j ) ;
				nBackTextureFlags |=
					S3DMaterial::ShadingFlagOfTextureType( nType ) ;
				nBackTextures |= 1 << nType ;
			}
		}
	}
	//
	// テクスチャ参照領域確認
	//
	size_t	nMeshCount = m_arrMeshObj.GetLength() ;
	for ( size_t i = 0; i < nMeshCount; i ++ )
	{
		MeshObject *	pMesh = m_arrMeshObj.GetAt( i ) ;
		if ( pMesh == nullptr )
		{
			continue ;
		}
		ssize_t	iMaterial = -1 ;
		for ( size_t j = 0; j < nCount; j ++ )
		{
			if ( ppMaterials[j] == pMesh->m_pMaterial )
			{
				iMaterial = (ssize_t) j ;
				break ;
			}
		}
		if ( iMaterial < 0 )
		{
			continue ;
		}
		SGLImageRect	rect = pTextureRects[iMaterial] ;
		S2DVector		vMin( (float32_t) rect.x, (float32_t) rect.y ) ;
		S2DVector		vMax( (float32_t) (rect.x + rect.w),
								(float32_t) (rect.y + rect.h) ) ;
		const S2DVector *	pUVMap = pMesh->m_bufUVMap.GetConstArray() ;
		size_t		nVertexCount = pMesh->m_bufUVMap.GetLength() ;
		for ( size_t j = 0; j < nVertexCount; j ++ )
		{
			vMin.x = esl_fminf( vMin.x, pUVMap[j].x ) ;
			vMin.y = esl_fminf( vMin.y, pUVMap[j].y ) ;
			vMax.x = esl_fmaxf( vMax.x, pUVMap[j].x ) ;
			vMax.y = esl_fmaxf( vMax.y, pUVMap[j].y ) ;
		}
		rect.x = eslRoundR32ToInt( vMin.x - 0.49999f ) ;
		rect.y = eslRoundR32ToInt( vMin.y - 0.49999f ) ;
		rect.w = eslRoundR32ToInt( vMax.x + 0.49999f ) - rect.x ;
		rect.h = eslRoundR32ToInt( vMax.y + 0.49999f ) - rect.y ;
		pTextureRects[iMaterial] = rect ;
	}
	//
	// テクスチャ配置
	//
	SArray<SGLSize>	aTextureSizes ;
	SGLSize *	pTextureSizes = aTextureSizes.GetArray( nCount ) ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		pTextureRects[i].x -- ;
		pTextureRects[i].y -- ;
		pTextureRects[i].w += 2 ;
		pTextureRects[i].h += 2 ;
		pTextureSizes[i] = pTextureRects[i].GetSize() ;
	}
	SPointerArray<SGLImageRect>	aTxtMapRects ;
	SGLImageRect **	ppTxtMapRects = aTxtMapRects.GetArray( nCount ) ;
	//
	SGLAreaAllocator	aalcTexture ;
	aalcTexture.BatchAllocate( ppTxtMapRects, pTextureSizes, nCount ) ;
	//
	aTextureSizes.FinishArray() ;
	//
	SGLSize	sizeMergeTexture = aalcTexture.GetTotalSize() ;
	sizeMergeTexture.w = (sizeMergeTexture.w + 0x07) & ~0x07 ;
	sizeMergeTexture.h = (sizeMergeTexture.h + 0x07) & ~0x07 ;
	//
	// 表面テクスチャ統合
	//
	S3DSurfaceAttribute	attrFace ;
	ppMaterials[0]->GetSurfaceAttribute( attrFace ) ;
	if ( nFaceTextureFlags != 0 )
	{
		int	iTexture = 0 ;
		for ( int i = 0; ((uint32_t) (1 << i) <= nFaceTextures)
						&& (iTexture < S3DMaterial::textureMaxCount); i ++ )
		{
			if ( !(nFaceTextures & (1 << i)) )
			{
				continue ;
			}
			SString		strTextureID = S3DMaterial::GetTextureTypeString( i ) ;
			uint32_t	nTxtType = i ;
			float32_t	nTxtApply = 1.0f ;
			float32_t	nTxtParam = 0.0f ;
			SGLImageObject *	pMerged =
				MergeMaterialTextures
					( sizeMergeTexture, ppMaterials,
						ppTxtMapRects, pTextureRects, nTxtType, false, nCount ) ;
			//
			if ( pwszTextureBaseID != nullptr )
			{
				strTextureID = SString(pwszTextureBaseID) + strTextureID ;
			}
			m_textures.AddSmartTextureAs
				( m_textures.NormalizeIdentity( strTextureID ), pMerged ) ;
			//
			ppMaterials[0]->SetTexture
				( pMerged, iTexture, nTxtType, nTxtApply, nTxtParam ) ;
			ppMaterials[0]->m_idTexture[iTexture] = strTextureID ;
			//
			iTexture ++ ;
		}
		while ( iTexture < S3DMaterial::textureMaxCount )
		{
			ppMaterials[0]->SetTexture( NULL, iTexture ) ;
			ppMaterials[0]->m_idTexture[iTexture] = L"" ;
			iTexture ++ ;
		}
		attrFace.flagsShading |= nFaceTextureFlags ;
		ppMaterials[0]->SetSurfaceAttribute( attrFace ) ;
	}
	//
	// 裏面テクスチャ統合
	//
	if ( ppMaterials[0]->IsEnabledBackSurfaceAttribute() )
	{
		S3DSurfaceAttribute	attrBack ;
		ppMaterials[0]->GetBackSurfaceAttribute( attrBack ) ;
		if ( nBackTextureFlags != 0 )
		{
			int	iTexture = 0 ;
			for ( int i = 0; ((uint32_t) (1 << i) <= nBackTextures)
							&& (iTexture < S3DMaterial::textureMaxCount); i ++ )
			{
				if ( !(nBackTextures & (1 << i)) )
				{
					continue ;
				}
				SString		strTextureID = S3DMaterial::GetTextureTypeString( i ) ;
				uint32_t	nTxtType = i ;
				float32_t	nTxtApply = 1.0f ;
				float32_t	nTxtParam = 0.0f ;
				SGLImageObject *	pMerged =
					MergeMaterialTextures
						( sizeMergeTexture, ppMaterials,
							ppTxtMapRects, pTextureRects, nTxtType, true, nCount ) ;
				//
				if ( pwszBackTextureBaseID != nullptr )
				{
					strTextureID = SString(pwszBackTextureBaseID) + strTextureID ;
				}
				m_textures.AddSmartTextureAs
					( m_textures.NormalizeIdentity( strTextureID ), pMerged ) ;
				//
				ppMaterials[0]->SetBackTexture
					( pMerged, iTexture, nTxtType, nTxtApply, nTxtParam ) ;
				ppMaterials[0]->m_idTexture[iTexture] = strTextureID ;
				//
				iTexture ++ ;
			}
			while ( iTexture < S3DMaterial::textureMaxCount )
			{
				ppMaterials[0]->SetBackTexture( NULL, iTexture ) ;
				ppMaterials[0]->m_idBackTexture[iTexture] = L"" ;
				iTexture ++ ;
			}
			attrBack.flagsShading |= nFaceTextureFlags ;
			ppMaterials[0]->SetBackSurfaceAttribute( attrBack ) ;
		}
	}
	//
	// UV座標修正
	//
	for ( size_t i = 0; i < nMeshCount; i ++ )
	{
		MeshObject *	pMesh = m_arrMeshObj.GetAt( i ) ;
		if ( pMesh == NULL )
		{
			continue ;
		}
		ssize_t	iMaterial = -1 ;
		for ( size_t j = 0; j < nCount; j ++ )
		{
			if ( ppMaterials[j] == pMesh->m_pMaterial )
			{
				iMaterial = (ssize_t) j ;
				break ;
			}
		}
		if ( iMaterial < 0 )
		{
			continue ;
		}
		SGLImageRect	rect = *(ppTxtMapRects[iMaterial]) ;
		S2DVector *	pUVMap = pMesh->m_bufUVMap.GetArray() ;
		size_t		nVertexCount = pMesh->m_bufUVMap.GetLength() ;
		float32_t	xOffset = (float32_t) (rect.x - pTextureRects[iMaterial].x) ;
		float32_t	yOffset = (float32_t) (rect.y - pTextureRects[iMaterial].y) ;
		//
		pMesh->m_pMaterial = ppMaterials[0] ;
		for ( size_t j = 0; j < nVertexCount; j ++ )
		{
			pUVMap[j].x += xOffset ;
			pUVMap[j].y += yOffset ;
		}
		pMesh->m_bufUVMap.FinishArray() ;
	}
	aTextureRects.FinishArray() ;
	aTxtMapRects.FinishArray() ;
	//
	// マテリアル削除
	//
	for ( size_t i = 1; i < nCount; i ++ )
	{
		ssize_t	iMaterial = m_materials.FindMaterialPtr( ppMaterials[i] ) ;
		if ( iMaterial >= 0 )
		{
			m_materials.RemoveMaterialAs
				( m_materials.GetMaterialIdentityAt( (size_t) iMaterial ) ) ;
		}
	}
	//
	// VBO 更新
	//
	RebuildVertexBuffer() ;
	return	sglErrSuccess ;
}

// テクスチャ統合
//////////////////////////////////////////////////////////////////////////////
SGLImageObject * S3DModelBuffer::MergeMaterialTextures
	( const SGLSize& sizeMergeTexture,
		S3DMaterial*const* ppMaterials,
		SGLImageRect** ppDstRects, const SGLImageRect* pSrcRects,
		int nTextureType, bool fBackSurface, size_t nCount )
{
	SGLImage *	pMergedImage = new SGLImage ;
	pMergedImage->CreateImage
		( sizeMergeTexture.w, sizeMergeTexture.h, formatImageARGB, 32 ) ;
	//
	if ( nTextureType == S3DMaterial::textureNormal )
	{
		pMergedImage->FillImage( SGLPalette( 0xFF00FF00 ) ) ;
	}
	//
	SGLPaintBuffer	paint ;
	uint64_t		nBufFlags = 0 ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		S3DMaterial *	pMaterial = ppMaterials[i] ;
		ESLAssert( pMaterial != NULL ) ;
		SGLImageObject *	pImage = NULL ;
		if ( !fBackSurface )
		{
			pImage = pMaterial->GetTexture
				( pMaterial->FindTextureTypeOf( nTextureType ) ) ;
		}
		else
		{
			pImage = pMaterial->GetBackTexture
				( pMaterial->FindBackTextureTypeOf( nTextureType ) ) ;
		}
		if ( pImage == NULL )
		{
			if ( nTextureType == S3DMaterial::textureDiffusion )
			{
				S3DColor	clrBase =
					fBackSurface ? pMaterial->m_attrBack.colorBase
								: pMaterial->m_attrSurface.colorBase ;
				SGLPalette	argbFill = clrBase.rgbAdd ;
				argbFill.argb.Alpha =
					(uint8_t) (0xFF - (clrBase.rgbMul.argb.Blue
									+ clrBase.rgbMul.argb.Green
									+ clrBase.rgbMul.argb.Red) / 3) ;
				pMergedImage->FillImage( argbFill, ppDstRects[i] ) ;
			}
			continue ;
		}
		nBufFlags |= pImage->GetBufferFlags() ;
		//
		// 描画座標補正
		//
		SGLSize		sizeImage = pImage->GetImageSize() ;
		SGLPoint	ptUpperLeft ;
		ptUpperLeft.x = pSrcRects[i].x ;
		ptUpperLeft.y = pSrcRects[i].y ;
		if ( ptUpperLeft.x < 0 )
		{
			ptUpperLeft.x -= sizeImage.w - (- ptUpperLeft.x % sizeImage.w) ;
		}
		else
		{
			ptUpperLeft.x -= ptUpperLeft.x % sizeImage.w ;
		}
		if ( ptUpperLeft.y < 0 )
		{
			ptUpperLeft.y -= sizeImage.h - (- ptUpperLeft.y % sizeImage.h) ;
		}
		else
		{
			ptUpperLeft.y -= ptUpperLeft.y % sizeImage.h ;
		}
		SGLImageRect *	pDstRect = ppDstRects[i] ;
		ptUpperLeft.x += pDstRect->x - pSrcRects[i].x ;
		ptUpperLeft.y += pDstRect->y - pSrcRects[i].y ;
		//
		// 描画
		//
		paint.AttachTargetImage( pMergedImage, NULL, ppDstRects[i] ) ;
		for ( int y = ptUpperLeft.y;
				y < pDstRect->y + pDstRect->h; y += sizeImage.h )
		{
			for ( int x = ptUpperLeft.x;
					x < pDstRect->x + pDstRect->w; x += sizeImage.w )
			{
				SGLPaintParam	pp ;
				pp.ptPaint.x = x ;
				pp.ptPaint.y = y ;
				//
				paint.DrawImage( pp, pImage ) ;
			}
		}
		paint.DetachTargetImage() ;
	}
	if ( nBufFlags & (SGLImageObject::bufferForMipmapTexture 
						| SGLImageObject::bufferCompressedTexture
						| SGLImageObject::bufferCompressionFormatMask) )
	{
		pMergedImage->NormalizeToTexture
			( nBufFlags & (SGLImageObject::bufferForMipmapTexture 
						| SGLImageObject::bufferCompressedTexture
						| SGLImageObject::bufferCompressionFormatMask) ) ;
	}
	//
	return	pMergedImage ;
}

// メッシュ統合
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelBuffer::MergeMeshMaterialOf( S3DMaterial * pMaterial )
{
	//
	// 対象メッシュ列挙
	//
	SArray<size_t>	arrTargetMesh ;
	size_t	nMeshCount = m_arrMeshObj.GetLength() ;
	size_t	nMergedMeshVertex = 0 ;
	for ( size_t i = 0; i < nMeshCount; i ++ )
	{
		MeshObject *	pMesh = m_arrMeshObj.GetAt( i ) ;
		if ( (pMesh != NULL) && (pMesh->m_pMaterial == pMaterial) )
		{
			if ( pMesh->m_typeMesh == primitiveTriangleStrip )
			{
				S3DTemporaryIndexTriangleStrip	tits ;
				const uint32_t *
					pIndex = tits.MakeIndexList( pMesh->m_countPolygon ) ;
				//
				pMesh->m_typeMesh = primitiveTriangle ;
				pMesh->m_bufIndex.SetLength( 0 ) ;
				pMesh->m_bufIndex.AddArray( pIndex, pMesh->m_countPolygon * 3 ) ;
			}
			else
			{
				ESLAssert( pMesh->m_typeMesh == primitiveTriangle ) ;
			}
			arrTargetMesh.Add( i ) ;
			nMergedMeshVertex += pMesh->m_countVertex ;
		}
	}
	if ( arrTargetMesh.GetLength() <= 1 )
	{
		return	sglErrSuccess ;
	}
	return	MergeMeshs( arrTargetMesh.GetConstArray(), arrTargetMesh.GetLength() ) ;
}

SGLError S3DModelBuffer::MergeMeshs
	( const size_t * pTargetMeshs, size_t nTargetMeshCount )
{
	if ( nTargetMeshCount <= 1 )
	{
		return	sglErrSuccess ;
	}
	const size_t	nMeshCount = m_arrMeshObj.GetLength() ;
	SBitArray		bitTargetMeshMask ;
	bitTargetMeshMask.SetLength( nMeshCount ) ;
	//
	// 対象メッシュ頂点数計算
	//
	S3DMaterial *	pMaterial = NULL ;
	size_t			nMergedMeshVertex = 0 ;
	bool			flagVertexColor = false ;
	size_t			nMaxExAttrElements = 0 ;
	size_t			i ;
	for ( i = 0; i < nTargetMeshCount; i ++ )
	{
		MeshObject *	pMesh = m_arrMeshObj.GetAt( pTargetMeshs[i] ) ;
		if ( pMesh != NULL )
		{
			if ( pMaterial == NULL )
			{
				pMaterial = pMesh->m_pMaterial ;
			}
			else if ( pMaterial != pMesh->m_pMaterial )
			{
				return	sglErrFailed ;
			}
			if ( pMesh->m_typeMesh == primitiveTriangleStrip )
			{
				S3DTemporaryIndexTriangleStrip	tits ;
				const uint32_t *
					pIndex = tits.MakeIndexList( pMesh->m_countPolygon ) ;
				//
				pMesh->m_typeMesh = primitiveTriangle ;
				pMesh->m_bufIndex.SetLength( 0 ) ;
				pMesh->m_bufIndex.AddArray( pIndex, pMesh->m_countPolygon * 3 ) ;
			}
			else
			{
				ESLAssert( pMesh->m_typeMesh == primitiveTriangle ) ;
			}
			if ( pMesh->m_bufColor.GetLength() >= pMesh->m_countVertex )
			{
				flagVertexColor = true ;
			}
			if ( nMaxExAttrElements < pMesh->m_nExAttrElements )
			{
				nMaxExAttrElements = pMesh->m_nExAttrElements ;
			}
			nMergedMeshVertex += pMesh->m_countVertex ;
			bitTargetMeshMask.SetAt( pTargetMeshs[i], true ) ;
		}
	}
	//
	// 頂点整列
	//
	SArray<ssize_t>	arrVertexRemap ;
	size_t		nTotalVertexCount = m_bufVertex.GetLength() ;
	ssize_t *	pVertexRemap = arrVertexRemap.GetArray( nTotalVertexCount ) ;
	for ( i = 0; i < nTotalVertexCount; i ++ )
	{
		pVertexRemap[i] = -1 ;
	}
	ssize_t	iNextVertex = 0 ;
	for ( i = 0; i < nMeshCount; i ++ )
	{
		MeshObject *	pMesh = m_arrMeshObj.GetAt( i ) ;
		if ( (pMesh != NULL) && !bitTargetMeshMask.GetAt( i ) )
		{
			size_t	iVertex = (size_t) pMesh->m_iVertex ;
			size_t	nVertexCount = pMesh->m_countVertex ;
			for ( size_t j = 0; j < nVertexCount; j ++ )
			{
				pVertexRemap[iVertex + j] = iNextVertex ++ ;
			}
		}
	}
	for ( i = 0; i < nTargetMeshCount; i ++ )
	{
		MeshObject *	pMesh = m_arrMeshObj.GetAt( pTargetMeshs[i] ) ;
		if ( pMesh != NULL )
		{
			size_t	iVertex = (size_t) pMesh->m_iVertex ;
			size_t	nVertexCount = pMesh->m_countVertex ;
			for ( size_t j = 0; j < nVertexCount; j ++ )
			{
				pVertexRemap[iVertex + j] = iNextVertex ++ ;
			}
		}
	}
	TrimRemapVertex
		( pVertexRemap, nTotalVertexCount, (size_t) iNextVertex ) ;
	//
	arrVertexRemap.FinishArray() ;
	//
	// ボーン影響メッシュの統合
	//
	for ( i = 0; i < m_ssoaBones.GetLength(); i ++ )
	{
		S3DModelBoneSpace *	pBone = m_ssoaBones.GetAt( i ) ;
		ESLAssert( pBone != NULL ) ;
		if ( pBone == NULL )
		{
			continue ;
		}
		size_t	iDstMesh = pTargetMeshs[0] ;
		for( size_t j = 1; j < nTargetMeshCount; j ++ )
		{
			pBone->MergeEffectiveMeshIndex( iDstMesh, pTargetMeshs[j] ) ;
		}
	}
	//
	// モーフィングターゲットの統合
	//
	MeshObject *	pTargetMesh = m_arrMeshObj.GetAt( pTargetMeshs[0] ) ;
	ESLAssert( pTargetMesh != NULL ) ;
	//
	size_t	nMorphPreVertex = 0 ;
	for ( i = 0; i < nTargetMeshCount; i ++ )
	{
		MeshObject *	pMesh = m_arrMeshObj.GetAt( pTargetMeshs[i] ) ;
		for ( size_t j = 0; j < pMesh->m_arrMorphTarget.GetLength(); j ++ )
		{
			SString *	pstrMorphID = pMesh->m_arrMorphTarget.GetAt( j ) ;
			if ( pstrMorphID == NULL )
			{
				continue ;
			}
			MorphTargetMesh *
				pMorph = m_ssoaMorphTarget.GetAs( *pstrMorphID ) ;
			if ( pMorph != NULL )
			{
				BoundMorphTargetMesh
					( pMorph, (ssize_t) nMorphPreVertex, nMergedMeshVertex ) ;
			}
			if ( pTargetMesh != pMesh )
			{
				pTargetMesh->m_arrMorphTarget.Add( new SString(*pstrMorphID) ) ;
			}
		}
		nMorphPreVertex += pMesh->m_countVertex ;
		//
		if ( pTargetMesh != pMesh )
		{
			pMesh->m_arrMorphTarget.RemoveAll() ;
		}
	}
	//
	// メッシュ統合
	//
	if ( flagVertexColor
		&& (pTargetMesh->m_bufColor.GetLength() < pTargetMesh->m_countVertex) )
	{
		// 頂点色生成
		S3DColor	clrDummy( 0xFFFFFFFF, 0 ) ;
		pTargetMesh->m_bufColor.SetLimit( pTargetMesh->m_countVertex ) ;
		while ( pTargetMesh->m_bufColor.GetLength() < pTargetMesh->m_countVertex )
		{
			pTargetMesh->m_bufColor.Add( clrDummy ) ;
		}
	}
	if ( nMaxExAttrElements > pTargetMesh->m_nExAttrElements )
	{
		// 拡張属性整列
		SArray<float32_t>	bufTemp = pTargetMesh->m_bufExAttrElements ;
		const size_t	nOldExAttr = pTargetMesh->m_nExAttrElements ;
		pTargetMesh->m_bufExAttrElements.RemoveAll() ;
		pTargetMesh->m_bufExAttrElements.SetLength
					( nMaxExAttrElements * pTargetMesh->m_countVertex ) ;
		pTargetMesh->m_nExAttrElements = nMaxExAttrElements ;
		//
		const float32_t *	pfpOldExAttr = bufTemp.GetConstArray() ;
		float32_t *			pfpDstExAttr = pTargetMesh->m_bufExAttrElements.GetArray() ;
		for ( size_t i = 0; i < pTargetMesh->m_countVertex; i ++ )
		{
			size_t	iDst = i * nMaxExAttrElements ;
			size_t	iSrc = i * nOldExAttr ;
			for ( size_t j = 0; j < nOldExAttr; j ++ )
			{
				pfpDstExAttr[iDst + j] = pfpOldExAttr[iSrc + j] ;
			}
		}
		pTargetMesh->m_bufExAttrElements.FinishArray() ;
	}
	for ( i = 1; i < nTargetMeshCount; i ++ )
	{
		MeshObject *	pMesh = m_arrMeshObj.GetAt( pTargetMeshs[i] ) ;
		ESLAssert( pMesh != NULL ) ;
		ESLAssert( pTargetMesh->m_iVertex + pTargetMesh->m_countVertex == pMesh->m_iVertex ) ;
		size_t	nOrgPolyCount = pTargetMesh->m_countPolygon ;
		size_t	nOrgVertexCount = pTargetMesh->m_countVertex ;
		pTargetMesh->m_countPolygon += pMesh->m_countPolygon ;
		pTargetMesh->m_countVertex += pMesh->m_countVertex ;
		if ( pMesh->m_bufUVMap.GetLength() >= pMesh->m_countVertex )
		{
			// UV統合
			pTargetMesh->m_bufUVMap.AddArray
				( pMesh->m_bufUVMap.GetConstArray(), pMesh->m_countVertex ) ;
		}
		else
		{
			pTargetMesh->m_bufUVMap.SetLength
				( pTargetMesh->m_bufUVMap.GetLength() + pMesh->m_countVertex ) ;
		}
		if ( flagVertexColor || (pTargetMesh->m_bufColor.GetLength() > 0) )
		{
			// 頂点色統合
			if ( pMesh->m_bufColor.GetLength() >= pMesh->m_countVertex )
			{
				pTargetMesh->m_bufColor.AddArray
					( pMesh->m_bufColor.GetConstArray(), pMesh->m_countVertex ) ;
			}
			else
			{
				S3DColor	clrDummy( 0xFFFFFFFF, 0 ) ;
				pTargetMesh->m_bufColor.SetLimit( pTargetMesh->m_countVertex ) ;
				for ( size_t j = 0; j < pMesh->m_countVertex; j ++ )
				{
					pTargetMesh->m_bufColor.Add( clrDummy ) ;
				}
			}
		}
		if ( pTargetMesh->m_nExAttrElements > 0 )
		{
			// 拡張属性統合
			size_t	iDstBase = pTargetMesh->m_bufExAttrElements.GetLength() ;
			pTargetMesh->m_bufExAttrElements.SetLength
				( iDstBase + pMesh->m_countVertex * pTargetMesh->m_nExAttrElements ) ;
			//
			const size_t	nDstExAttr = pTargetMesh->m_nExAttrElements ;
			const size_t	nSrcExAttr = pMesh->m_nExAttrElements ;
			if ( nSrcExAttr > 0 )
			{
				float32_t *			pfpDstExAttr = pTargetMesh->m_bufExAttrElements.GetArray() ;
				const float32_t *	pfpSrcExAttr = pMesh->m_bufExAttrElements.GetConstArray() ;
				for ( size_t j = 0; j < pMesh->m_countVertex; j ++ )
				{
					size_t	iDst = iDstBase + j * nDstExAttr ;
					size_t	iSrc = j * nSrcExAttr ;
					for ( size_t k = 0; k < nSrcExAttr; k ++ )
					{
						pfpDstExAttr[iDst + k] = pfpSrcExAttr[iSrc + k] ;
					}
				}
				pTargetMesh->m_bufExAttrElements.FinishArray() ;
			}
		}
		//
		pTargetMesh->m_bufIndex.AddArray
			( pMesh->m_bufIndex.GetConstArray(), pMesh->m_bufIndex.GetLength() ) ;
		//
		size_t	k ;
		uint32_t *	pIndex ;
		pIndex = pTargetMesh->m_bufIndex.GetAt( nOrgPolyCount * 3 ) ;
		for ( k = 0; k < pMesh->m_bufIndex.GetLength(); k ++ )
		{
			pIndex[k] += (uint32_t) nOrgVertexCount ;
		}
		//
		for ( size_t j = 0; j < countSubMesh; j ++ )
		{
			nOrgPolyCount = pTargetMesh->m_countSubPoly[j] ;
			pTargetMesh->m_countSubPoly[j] += pMesh->m_countSubPoly[j] ;
			//
			pTargetMesh->m_bufSubIndex[j].AddArray
				( pMesh->m_bufSubIndex[j].GetConstArray(),
					pMesh->m_bufSubIndex[j].GetLength() ) ;
			//
			pIndex = pTargetMesh->m_bufSubIndex[j].GetAt( nOrgPolyCount * 3 ) ;
			for ( k = 0; k < pMesh->m_bufSubIndex[j].GetLength(); k ++ )
			{
				pIndex[k] += (uint32_t) nOrgVertexCount ;
			}
		}
	}
	//
	// ポーズのメッシュ参照統合
	//
	const SString *	pstrTargetMeshID = GetMeshIdentityAt( pTargetMeshs[0] ) ;
	SPointerArray<const wchar_t>	aMergeSourceMeshIDs ;
	for ( i = 1; i < nTargetMeshCount; i ++ )
	{
		const SString *	pstrMeshID = GetMeshIdentityAt( pTargetMeshs[i] ) ;
		if ( pstrMeshID != NULL )
		{
			aMergeSourceMeshIDs.Add( (const wchar_t*) *pstrMeshID ) ;
		}
	}
	if ( (pstrTargetMeshID != NULL)
			&& (aMergeSourceMeshIDs.GetLength() > 0) )
	{
		m_poses.MergeTargetMeshID
			( *pstrTargetMeshID,
				aMergeSourceMeshIDs.GetConstArray(),
				aMergeSourceMeshIDs.GetLength() ) ;
	}
	//
	// メッシュオブジェクトの削除とボーン処理
	//
	SArray<size_t>	arrRemoveMeshs ;
	arrRemoveMeshs.AddArray( pTargetMeshs + 1, nTargetMeshCount - 1 ) ;
	for ( i = 0; i + 1 < arrRemoveMeshs.GetLength(); i ++ )
	{
		size_t	iMaxMesh = arrRemoveMeshs.At( i ) ;
		size_t	iMaxIndex = i ;
		for ( size_t j = i + 1; j < arrRemoveMeshs.GetLength(); j ++ )
		{
			size_t	iMesh = arrRemoveMeshs.At( j ) ;
			if ( iMaxMesh < iMesh )
			{
				iMaxMesh = iMesh ;
				iMaxIndex = j ;
			}
		}
		arrRemoveMeshs.Swap( i, iMaxIndex ) ;
	}
	for ( i = 0; i < arrRemoveMeshs.GetLength(); i ++ )
	{
		RemoveMeshObjectAt( arrRemoveMeshs.At(i), false ) ;
	}
	NormalizeBoneWeightMapRange() ;
	RebuildVertexBuffer() ;
	return	sglErrSuccess ;
}

// メッシュオブジェクト削除（頂点は無修正）
//////////////////////////////////////////////////////////////////////////////
void S3DModelBuffer::RemoveMeshObjectAt( size_t iMesh, bool fRebuildVBO )
{
	m_arrMeshObj.RemoveAt( iMesh ) ;
	//
	RemoveBoneMeshRef( &m_boneRoot, iMesh ) ;
	//
	for ( size_t i = 0; i < m_ssaMeshGroup.GetLength(); i ++ )
	{
		MeshGroup *	pmg = m_ssaMeshGroup.GetAt( i ) ;
		if ( (pmg->m_iFirstMesh <= iMesh)
			&& (iMesh < pmg->m_iFirstMesh + pmg->m_nMeshCount) )
		{
			if ( pmg->m_nMeshCount == 1 )
			{
				m_ssaMeshGroup.RemoveAt( i -- ) ;
			}
			else
			{
				pmg->m_nMeshCount -- ;
			}
		}
		else if ( iMesh < pmg->m_iFirstMesh )
		{
			pmg->m_iFirstMesh -- ;
		}
	}
	//
	if ( fRebuildVBO )
	{
		RebuildVertexBuffer() ;
	}
}

void S3DModelBuffer::RemoveBoneMeshRef( S3DModelBoneSpace * pBone, size_t iMesh )
{
	for ( size_t i = 0; i < pBone->m_arrRefMesh.GetLength(); i ++ )
	{
		S3DModelBoneSpace::REF_MESH_INFO *
					prmi = pBone->m_arrRefMesh.GetAt( i ) ;
		ESLAssert( prmi != NULL ) ;
		if ( prmi->iMesh == iMesh )
		{
			pBone->m_arrRefMesh.RemoveAt( i -- ) ;
		}
		else if ( prmi->iMesh >= iMesh )
		{
			prmi->iMesh -- ;
		}
	}
	for ( size_t i = 0; i < pBone->m_children.GetLength(); i ++ )
	{
		S3DModelBoneSpace *	pSubBone =
			ESLTypeCast<S3DModelBoneSpace>( pBone->m_children.GetAt(i) ) ;
		if ( pSubBone != NULL )
		{
			RemoveBoneMeshRef( pSubBone, iMesh ) ;
		}
	}
}

// モーフィングターゲット範囲補正
//////////////////////////////////////////////////////////////////////////////
void S3DModelBuffer::BoundMorphTargetMesh
	( S3DModelBuffer::MorphTargetMesh * pMorph,
					ssize_t nLeftPad, size_t nVertexCount )
{
	if ( pMorph->m_bufVertex.GetLength() != 0 )
	{
		if ( nLeftPad > 0 )
		{
			pMorph->m_bufVertex.Insert( 0, (size_t) nLeftPad ) ;
		}
		else if ( nLeftPad < 0 )
		{
			pMorph->m_bufVertex.Remove( 0, (size_t) (- nLeftPad) ) ;
		}
		pMorph->m_bufVertex.SetLength( nVertexCount ) ;
	}
	if ( pMorph->m_bufNormal.GetLength() != 0 )
	{
		if ( nLeftPad > 0 )
		{
			pMorph->m_bufNormal.Insert( 0, (size_t) nLeftPad ) ;
		}
		else if ( nLeftPad < 0 )
		{
			pMorph->m_bufNormal.Remove( 0, (size_t) (- nLeftPad) ) ;
		}
		pMorph->m_bufNormal.SetLength( nVertexCount ) ;
	}
	if ( pMorph->m_bufUVMap.GetLength() != 0 )
	{
		if ( nLeftPad > 0 )
		{
			pMorph->m_bufUVMap.Insert( 0, (size_t) nLeftPad ) ;
		}
		else if ( nLeftPad < 0 )
		{
			pMorph->m_bufUVMap.Remove( 0, (size_t) (- nLeftPad) ) ;
		}
		pMorph->m_bufUVMap.SetLength( nVertexCount ) ;
	}
	if ( pMorph->m_bufColor.GetLength() != 0 )
	{
		if ( nLeftPad > 0 )
		{
			pMorph->m_bufColor.Insert( 0, (size_t) nLeftPad ) ;
		}
		else if ( nLeftPad < 0 )
		{
			pMorph->m_bufColor.Remove( 0, (size_t) (- nLeftPad) ) ;
		}
		pMorph->m_bufColor.SetLength( nVertexCount ) ;
	}
	if ( pMorph->m_bufWeight.GetLength() != 0 )
	{
		if ( nLeftPad > 0 )
		{
			pMorph->m_bufWeight.Insert( 0, (size_t) nLeftPad ) ;
		}
		else if ( nLeftPad < 0 )
		{
			pMorph->m_bufWeight.Remove( 0, (size_t) (- nLeftPad) ) ;
		}
		pMorph->m_bufWeight.SetLength( nVertexCount ) ;
	}
	pMorph->m_countVertex = nVertexCount ;
}

// メッシュ削除（頂点も削除）
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelBuffer::RemoveVertexMeshAt( size_t iMesh, bool fRebuildVBO )
{
	//
	// メッシュ情報取得
	//
	MeshObject *	pMesh = m_arrMeshObj.GetAt( iMesh ) ;
	if ( pMesh == NULL )
	{
		return	sglErrFailed ;
	}
	SObjectArray<SString>	arrMorphTarget = pMesh->m_arrMorphTarget ;
	const size_t	iMeshVertex = pMesh->m_iVertex ;
	const size_t	nMeshVertexCount = pMesh->m_countVertex ;
	ESLAssert( pMesh->m_iVertex == pMesh->m_iNormal ) ;
	//
	// メッシュ情報を削除
	//
	RemoveMeshObjectAt( iMesh, false ) ;
	//
	// 使用されなくなったモーフターゲットを削除
	//
	size_t	i ;
	for ( i = 0; i < arrMorphTarget.GetLength(); i ++ )
	{
		SString *	pstrMorphID = arrMorphTarget.GetAt( i ) ;
		if ( (pstrMorphID != NULL) && !IsMorphTargetUsed( *pstrMorphID ) )
		{
			m_ssoaMorphTarget.RemoveAs( *pstrMorphID ) ;
		}
	}
	//
	// 頂点を削除する
	//
	SArray<ssize_t>	bufIndexRemap ;
	size_t			nOldVertexCount = m_bufVertex.GetLength() ;
	ssize_t *		pIndexRemap = bufIndexRemap.GetArray( nOldVertexCount ) ;
	for ( i = 0; i < iMeshVertex; i ++ )
	{
		pIndexRemap[i] = (ssize_t) i ;
	}
	for ( i = 0; i < nMeshVertexCount; i ++ )
	{
		pIndexRemap[iMeshVertex + i] = -1 ;
	}
	for ( i = iMeshVertex + nMeshVertexCount; i < nOldVertexCount; i ++ )
	{
		pIndexRemap[i] = (ssize_t) (i - nMeshVertexCount) ;
	}
	TrimRemapVertex
		( pIndexRemap, nOldVertexCount, nOldVertexCount - nMeshVertexCount ) ;
	//
	bufIndexRemap.FinishArray() ;
	//
	// ボーン正規化と VBO 再生成
	//
	NormalizeBoneWeightMapRange() ;
	//
	if ( fRebuildVBO )
	{
		RebuildVertexBuffer() ;
	}
	return	sglErrSuccess ;
}

// メッシュをボーン影響ごとに分割
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelBuffer::DivideMeshByBoneBounds
	( SSystem::SObjectArray<SSystem::SString>& aDivMeshIDs,
		size_t iMesh, const S3DModelBuffer::DivieMeshByBoneParam& dmbbp )
{
	MeshObject *	pmoSrc = m_arrMeshObj.GetAt( iMesh ) ;
	if ( pmoSrc == NULL )
	{
		return	sglErrFailed ;
	}
	if ( pmoSrc->m_typeMesh != primitiveTriangle )
	{
		return	sglErrFailed ;
	}
	if ( dmbbp.nMaxBoneCount <= 1 )
	{
		return	sglErrInvalidParam ;
	}
	if ( (pmoSrc->m_countVertex <= dmbbp.nLowVertexCount)
		|| (pmoSrc->m_arrRelBone.GetLength() <= dmbbp.nMaxBoneCount) )
	{
		return	sglErrSuccess ;
	}
	//
	// モーフターゲットがある場合、その範囲を取得
	//
	SBitArray	maskMorphing ;
	maskMorphing.SetLength( pmoSrc->m_countVertex ) ;
	maskMorphing.Clear() ;
	//
	size_t	i ;
	for ( i = 0; i < pmoSrc->m_arrMorphTarget.GetLength(); i ++ )
	{
		SString *	pstrMorphID = pmoSrc->m_arrMorphTarget.GetAt( i ) ;
		if ( pstrMorphID == NULL )
		{
			continue ;
		}
		MorphTargetMesh *	pmtmMesh = m_ssoaMorphTarget.GetAs( *pstrMorphID ) ;
		if ( pmtmMesh == NULL )
		{
			continue ;
		}
		SBitArray	maskMorph ;
		GetMorphMeshEffectMask( maskMorph, *pmtmMesh ) ;
		//
		maskMorphing |= maskMorph ;
	}
	//
	ExpandMeshEffectMaskNextPolygon
		( maskMorphing,
			pmoSrc->m_bufIndex.GetConstArray(), pmoSrc->m_countPolygon ) ;
	//
	// 各ボーンの影響範囲で分割
	//
	SString			strMeshBaseID = L"mesh" ;
	const SString *	pstrSrcMeshID = GetMeshGroupNameIndexOf( iMesh ) ;
	if ( pstrSrcMeshID != NULL )
	{
		strMeshBaseID = *pstrSrcMeshID ;
	}
	//
	SBitArray	maskPending ;
	maskPending.SetLength( pmoSrc->m_countVertex ) ;
	maskPending.Clear() ;
	//
	SBitArray	maskException = maskMorphing ;
	DivideMeshByBoneBoundsInBone
		( aDivMeshIDs, pmoSrc, iMesh, strMeshBaseID,
			maskException, maskPending, dmbbp, &m_boneRoot ) ;
	//
	// 残りのメッシュを分割
	//
	if ( aDivMeshIDs.GetLength() > 0 )
	{
		maskException.NotAnd( 0, maskMorphing ) ;
		maskException.Not() ;
		ExpandMeshEffectMaskNextPolygon
			( maskException,
				pmoSrc->m_bufIndex.GetConstArray(), pmoSrc->m_countPolygon ) ;
		//
		size_t	iLastDiv = aDivMeshIDs.GetLength() ;
		DivideMeshByPendingBoneBounds
			( aDivMeshIDs, iMesh, strMeshBaseID, maskException, true ) ;
		//
		SString *	pstrMeshID = aDivMeshIDs.GetAt( iLastDiv ) ;
		if ( pstrMeshID != NULL )
		{
			RemoveVertexMeshAt( iMesh, false ) ;
			ModifyMeshIdentity( *pstrMeshID, strMeshBaseID ) ;
			*pstrMeshID = strMeshBaseID ;
		}
		//
		// ボーン正規化と VBO 再生成
		//
		NormalizeBoneWeightMapRange() ;
		RebuildVertexBuffer() ;
	}
	return	sglErrSuccess ;
}

SGLError S3DModelBuffer::DivideMeshByBoneBoundsInBone
	( SSystem::SObjectArray<SSystem::SString>& aDivMeshIDs,
		MeshObject * pmoSrc, size_t iSrcMesh,
		const SSystem::SString& strMeshBaseID,
		SSystem::SBitArray& maskException,
		SSystem::SBitArray& maskPending,
		const S3DModelBuffer::DivieMeshByBoneParam& dmbbp,
		S3DModelBoneSpace * pBone )
{
	//
	// 子ボーンを順次処理
	//
	size_t	nChildren = pBone->GetChildrenCount() ;
	for ( size_t i = 0; i < nChildren; i ++ )
	{
		S3DModelBoneSpace *	pChild =
			ESLTypeCast<S3DModelBoneSpace>( pBone->GetChildAt( i ) ) ;
		if ( pChild == NULL )
		{
			continue ;
		}
		DivideMeshByBoneBoundsInBone
			( aDivMeshIDs, pmoSrc, iSrcMesh, strMeshBaseID,
				maskException, maskPending, dmbbp, pChild ) ;
	}
	//
	// このボーンを処理する
	//
	if ( pmoSrc->FindRelativeBone( pBone ) < 0 )
	{
		return	sglErrSuccess ;
	}
	SBitArray	maskBone ;
	maskBone.SetLength( pmoSrc->m_countVertex ) ;
	maskBone.Clear() ;
	pBone->GetBoneEffectVertexMap( pmoSrc->m_iVertex, maskBone ) ;
	//
	SBitArray	maskMergeBone = maskBone ;
	maskMergeBone |= maskPending ;
	maskMergeBone.NotAnd( 0, maskException ) ;
	ExpandMeshEffectMaskNextPolygon
		( maskMergeBone,
			pmoSrc->m_bufIndex.GetConstArray(), pmoSrc->m_countPolygon ) ;
	//
	size_t	nEffectBones =
				m_boneRoot.GetBoneCountEffectedVertexMap
							( pmoSrc->m_iVertex, maskMergeBone ) ;
	if ( (nEffectBones <= dmbbp.nMaxBoneCount)
		|| (maskMergeBone.BitCount() < dmbbp.nLowVertexCount) )
	{
		maskPending = maskMergeBone ;
		return	sglErrSuccess ;
	}
	//
	// このボーンを結合すると分割条件になるので
	// ペンディング中の範囲を分割する
	//
	if ( maskPending.BitCount() > 0 )
	{
		const SString *	pstrBoneID = GetBoneIdentityOf( pBone ) ;
		SString	strNewMeshID = strMeshBaseID ;
		strNewMeshID += L"_" ;
		if ( pstrBoneID != NULL )
		{
			strNewMeshID += *pstrBoneID ;
		}
		ExpandMeshEffectMaskNextPolygon
			( maskPending,
				pmoSrc->m_bufIndex.GetConstArray(), pmoSrc->m_countPolygon ) ;
		DivideMeshByPendingBoneBounds
			( aDivMeshIDs, iSrcMesh, strNewMeshID, maskPending, false ) ;
		maskException |= maskPending ;
		maskPending.Clear() ;
	}
	else
	{
		maskPending |= maskBone ;
		maskPending.NotAnd( 0, maskException ) ;
	}
	return	sglErrSuccess ;
}

void S3DModelBuffer::DivideMeshByPendingBoneBounds
	( SSystem::SObjectArray<SSystem::SString>& aDivMeshIDs,
		size_t iSrcMesh, const SString& strMeshBaseID,
		SBitArray& maskPending, bool fMoveMorphMesh )
{
	SString	strNewMeshID = strMeshBaseID ;
	NormalizeMeshID( strNewMeshID ) ;
	//
	if ( !DuplicatePortionOfMesh
		( strNewMeshID, iSrcMesh, maskPending, fMoveMorphMesh, false ) )
	{
		aDivMeshIDs.Add( new SString( strNewMeshID ) ) ;
	}
}

// １メッシュが指定ポリゴン数以下になるように分割
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelBuffer::DivideMeshByPolygonCount
	( SObjectArray<SString>& aDivMeshIDs, size_t iMesh, size_t nLimitPolygon )
{
	MeshObject *	pmoSrc = m_arrMeshObj.GetAt( iMesh ) ;
	if ( pmoSrc == NULL )
	{
		return	sglErrFailed ;
	}
	if ( pmoSrc->m_typeMesh != primitiveTriangle )
	{
		return	sglErrFailed ;
	}
	//
	// 全ポリゴンの中心座標を計算する
	//
	SArray<S3DVector>	aPolygonCenter ;
	S3DVector4 *		pvSrcVertex = m_bufVertex.GetAt( pmoSrc->m_iVertex ) ;
	size_t				nSrcPolygons = pmoSrc->m_countPolygon ;
	const uint32_t *	pIndexes = pmoSrc->m_bufIndex.GetConstArray() ;
	S3DVector *		pvPolygonCenter = aPolygonCenter.GetArray( nSrcPolygons ) ;
	for ( size_t i = 0, j = 0; i < nSrcPolygons; i ++, j += 3 )
	{
		pvPolygonCenter[i] =
			(pvSrcVertex[pIndexes[j]]
				+ pvSrcVertex[pIndexes[j + 1]]
				+ pvSrcVertex[pIndexes[j + 2]]) / 3.0f ;
	}
	aPolygonCenter.FinishArray() ;
	//
	// 分割の基準点を決める
	//
	const size_t	nDivCount =
						(nSrcPolygons + nLimitPolygon * 3 / 2) / nLimitPolygon ;
	if ( nDivCount <= 1 )
	{
		return	sglErrSuccess ;
	}
	SArray<S3DVector>	aCriterionPos ;
	SArray<size_t>		aCriterionIndex ;
	SBitArray			maskCriterion ;
	maskCriterion.SetLength( nSrcPolygons ) ;
	maskCriterion.Clear() ;
	//
	aCriterionPos.Add( pvPolygonCenter[nSrcPolygons / 2] ) ;
	aCriterionIndex.Add( nSrcPolygons / 2 ) ;
	maskCriterion.SetAt( nSrcPolygons / 2, true ) ;
	//
	for ( size_t i = 1; i < nDivCount; i ++ )
	{
		const S3DVector *	pvCriterionPos = aCriterionPos.GetConstArray() ;
		size_t				nCriterionCount = aCriterionPos.GetLength() ;
		//
		ssize_t	iPolygon = -1 ;
		double	maxDistance = 0.0 ;
		for ( size_t j = 0; j < nSrcPolygons; j ++ )
		{
			if ( maskCriterion.GetAt( j ) )
			{
				continue ;
			}
			double	minDistance = 1.0e+99 ;
			for ( size_t k = 0; k < nCriterionCount; k ++ )
			{
				double	d = (pvPolygonCenter[j]
								- pvCriterionPos[k]).Absolute() ;
				if ( d < minDistance )
				{
					minDistance = d ;
				}
			}
			if ( maxDistance < minDistance )
			{
				iPolygon = (ssize_t) j ;
				maxDistance = minDistance ;
			}
		}
		if ( iPolygon >= 0 )
		{
			aCriterionPos.Add( pvPolygonCenter[iPolygon] ) ;
			aCriterionIndex.Add( iPolygon ) ;
			maskCriterion.SetAt( (size_t) iPolygon, true ) ;
		}
	}
	if ( aCriterionPos.GetLength() <= 1 )
	{
		return	sglErrSuccess ;
	}
	//
	// 基準点からの距離を計算する
	//
	const S3DVector *	pvCriterionPos = aCriterionPos.GetConstArray() ;
	size_t				nCriterionCount = aCriterionPos.GetLength() ;
	//
	SArray<DivPolyEntryInfo>	aEntryInfos ;
	aEntryInfos.SetLimit( nSrcPolygons * nCriterionCount ) ;
	//
	for ( size_t i = 0; i < nSrcPolygons; i ++ )
	{
		if ( maskCriterion.GetAt( i ) )
		{
			continue ;
		}
		for ( size_t j = 0; j < nCriterionCount; j ++ )
		{
			DivPolyEntryInfo	dpei ;
			dpei.fpDistance = (pvPolygonCenter[i]
								- pvCriterionPos[j]).Absolute() ;
			dpei.iPolygon = i ;
			dpei.iCriterion = j ;
			aEntryInfos.Add( dpei ) ;
		}
	}
	//
	// 距離でソートする
	//
	QuickSortDivPolyEntries
		( aEntryInfos.GetArray(), aEntryInfos.GetLength() ) ;
	aEntryInfos.FinishArray() ;
	//
	// 各基準点に振り分けるポリゴンを決定する
	//
	SObjectArray< SArray<size_t> >	aPolygonEntries ;
	for ( size_t i = 0; i < aCriterionIndex.GetLength(); i ++ )
	{
		SArray<size_t> *	pEntries = new SArray<size_t> ;
		pEntries->Add( aCriterionIndex.At(i) ) ;
		aPolygonEntries.Add( pEntries ) ;
	}
	for ( size_t i = 0; i < aEntryInfos.GetLength(); i ++ )
	{
		DivPolyEntryInfo&	dpei = aEntryInfos.At( i ) ;
		if ( maskCriterion.GetAt( dpei.iPolygon ) )
		{
			continue ;
		}
		SArray<size_t> *
			pEntries = aPolygonEntries.GetAt( dpei.iCriterion ) ;
		ESLAssert( pEntries != NULL ) ;
		if ( pEntries->GetLength() >= nLimitPolygon )
		{
			continue ;
		}
		pEntries->Add( dpei.iPolygon ) ;
		maskCriterion.SetAt( dpei.iPolygon, true ) ;
	}
	//
	// メッシュを分割
	//
	SString			strMeshBaseID = L"mesh" ;
	const SString *	pstrSrcMeshID = GetMeshGroupNameIndexOf( iMesh ) ;
	if ( pstrSrcMeshID != NULL )
	{
		strMeshBaseID = *pstrSrcMeshID ;
	}
	for ( size_t i = 0; i < aPolygonEntries.GetLength(); i ++ )
	{
		SArray<size_t> *	pEntries = aPolygonEntries.GetAt( i ) ;
		ESLAssert( pEntries != NULL ) ;
		//
		SBitArray	maskVertex ;
		maskVertex.SetLength( pmoSrc->m_countVertex ) ;
		maskVertex.Clear() ;
		//
		for ( size_t j = 0; j < pEntries->GetLength(); j ++ )
		{
			size_t	iPoly3 = pEntries->At( j ) * 3 ;
			maskVertex.SetAt( pIndexes[iPoly3], true ) ;
			maskVertex.SetAt( pIndexes[iPoly3 + 1] ,true ) ;
			maskVertex.SetAt( pIndexes[iPoly3 + 2], true ) ;
		}
		//
		SString	strNewMeshID = strMeshBaseID ;
		NormalizeMeshID( strNewMeshID ) ;
		aDivMeshIDs.Add( new SString( strNewMeshID ) ) ;
		//
		DuplicatePortionOfMesh
			( strNewMeshID, iMesh, maskVertex, false, false ) ;
	}
	//
	// 元のメッシュを削除
	//
	RemoveVertexMeshAt( iMesh, false ) ;
	//
	// VBO 再生成
	//
	RebuildVertexBuffer() ;
	//
	return	sglErrSuccess ;
}

// ポリゴンを基準点からの距離でソート
//////////////////////////////////////////////////////////////////////////////
void S3DModelBuffer::QuickSortDivPolyEntries
	( S3DModelBuffer::DivPolyEntryInfo * pdpis, size_t nCount )
{
	if ( nCount <= 4 )
	{
		//
		// ソート用ハッシュ値の昇順に選択ソート
		//
		size_t	i = nCount ;
		while ( -- i >= 1 )
		{
			DivPolyEntryInfo	dpeiPivot = pdpis[i] ;
			double				fpMin = dpeiPivot.fpDistance ;
			double				fpTemp ;
			size_t				iMin = i ;
			size_t				j = i - 1 ;
			do
			{
				fpTemp = pdpis[j].fpDistance ;
				if ( fpTemp < fpMin )
				{
					fpMin = fpTemp ;
					iMin = j ;
				}
			}
			while ( (j --) > 0 ) ;
			//
			pdpis[i] = pdpis[iMin] ;
			pdpis[iMin] = dpeiPivot ;
		}
		return ;
	}
	//
	// ソート用ハッシュ値の昇順にクイックソート
	//
	ssize_t	iFirst = 0 ;
	ssize_t	iEnd = (ssize_t) nCount - 1 ;
	ssize_t	iPivot = iEnd ;
	DivPolyEntryInfo	dpeiPivot = pdpis[iPivot] ;
	DivPolyEntryInfo	dpeiLeft ;
	DivPolyEntryInfo	dpeiRight ;
	double				fpPivot = dpeiPivot.fpDistance ;
	while ( iFirst < iEnd )
	{
		// 基準値より大きい要素を左側から順次検索
		dpeiLeft = pdpis[iFirst] ;
		if ( dpeiLeft.fpDistance > fpPivot )
		{
			pdpis[iEnd --] = dpeiLeft ;
			//
			while ( iFirst < iEnd )
			{
				// 基準値より小さい要素を右側から順次検索
				dpeiRight = pdpis[iEnd] ;
				if ( dpeiRight.fpDistance < fpPivot )
				{
					pdpis[iFirst ++] = dpeiRight ;
					break ;
				}
				-- iEnd ;
			}
			continue ;
		}
		++ iFirst ;
	}
	ESLAssert( iFirst == iEnd ) ;
	ESLAssert( iFirst >= 0 ) ;
	pdpis[iFirst] = dpeiPivot ;
	//
	if ( iFirst >= 2 )
	{
		QuickSortDivPolyEntries( pdpis, iFirst ) ;
	}
	++ iFirst ;
	if ( (ssize_t) nCount > iFirst + 1 )
	{
		QuickSortDivPolyEntries( pdpis + iFirst, nCount - iFirst ) ;
	}
}

// 部分メッシュ複製追加
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelBuffer::DuplicatePortionOfMesh
	( const wchar_t * pwszNewMeshID, size_t iSrcMesh,
		const SSystem::SBitArray& maskVertexPortion,
		bool fMoveMorphMesh, bool fRebuildVBO )
{
	MeshObject *	pmoSrc = m_arrMeshObj.GetAt( iSrcMesh ) ;
	if ( pmoSrc == NULL )
	{
		return	sglErrFailed ;
	}
	if ( pmoSrc->m_typeMesh != primitiveTriangle )
	{
		return	sglErrFailed ;
	}
	if ( pmoSrc->m_countVertex < maskVertexPortion.GetLength() )
	{
		return	sglErrFailed ;
	}
	//
	// 再配列用インデックス作成
	//
	SArray<size_t>	aRemapIndex ;
	SArray<ssize_t>	aMapIndex ;
	size_t		nVertexCount = maskVertexPortion.BitCount() ;
	size_t *	pRemapIndex = aRemapIndex.GetArray( nVertexCount ) ;
	ssize_t *	pMapIndex = aMapIndex.GetArray( pmoSrc->m_countVertex ) ;
	size_t		iDst, iSrc ;
	for ( iDst = 0, iSrc = 0; iSrc < maskVertexPortion.GetLength(); iSrc ++ )
	{
		pMapIndex[iSrc] = -1 ;
		if ( maskVertexPortion.GetAt( iSrc ) )
		{
			ESLAssert( iDst < nVertexCount ) ;
			pMapIndex[iSrc] = (ssize_t) iDst ;
			pRemapIndex[iDst ++] = iSrc ;
		}
	}
	aRemapIndex.FinishArray() ;
	aMapIndex.FinishArray() ;
	ESLAssert( iDst == nVertexCount ) ;
	//
	// 新規メッシュオブジェクト
	//
	MeshObject *	pNewMesh = new MeshObject ;
	const size_t	iNewMesh = m_arrMeshObj.GetLength() ;
	m_arrMeshObj.Add( pNewMesh ) ;
	//
	pNewMesh->m_pMaterial = pmoSrc->m_pMaterial ;
	pNewMesh->m_typeMesh = primitiveTriangle ;
	pNewMesh->m_countPolygon = 0 ;
	pNewMesh->m_countVertex = nVertexCount ;
	pNewMesh->m_iVertex = (ssize_t) m_bufVertex.GetLength() ;
	pNewMesh->m_iNormal = (ssize_t) m_bufNormal.GetLength() ;
	//
	MeshGroup	mgNewMesh ;
	mgNewMesh.m_iFirstMesh = (uint32_t) iNewMesh ;
	mgNewMesh.m_nMeshCount = 1 ;
	mgNewMesh.m_vCenter.x = 0 ;
	mgNewMesh.m_vCenter.y = 0 ;
	mgNewMesh.m_vCenter.z = 0 ;
	m_ssaMeshGroup.Add( pwszNewMeshID, mgNewMesh ) ;
	//
	// 頂点配列
	//
	S3DVector4 *		pvDstVertex ;
	const S3DVector4 *	pvSrcVertex ;
	m_bufVertex.SetLength( pNewMesh->m_iVertex + nVertexCount ) ;
	pvDstVertex = m_bufVertex.GetAt( pNewMesh->m_iVertex ) ;
	pvSrcVertex = m_bufVertex.GetAt( pmoSrc->m_iVertex ) ;
	//
	size_t	i, j ;
	for ( i = 0; i < nVertexCount; i ++ )
	{
		pvDstVertex[i] = pvSrcVertex[ pRemapIndex[i] ] ;
	}
	CommitVertexBuffer( pNewMesh->m_iVertex, nVertexCount ) ;
	//
	// 法線配列
	//
	S3DVector4 *		pvDstNormal ;
	const S3DVector4 *	pvSrcNormal ;
	m_bufNormal.SetLength( pNewMesh->m_iNormal + nVertexCount ) ;
	pvDstNormal = m_bufNormal.GetAt( pNewMesh->m_iNormal ) ;
	pvSrcNormal = m_bufNormal.GetAt( pmoSrc->m_iNormal ) ;
	//
	for ( i = 0; i < nVertexCount; i ++ )
	{
		pvDstNormal[i] = pvSrcNormal[ pRemapIndex[i] ] ;
	}
	CommitNormalBuffer( pNewMesh->m_iNormal, nVertexCount ) ;
	//
	// UV配列
	//
	if ( pmoSrc->m_bufUVMap.GetLength() >= pmoSrc->m_countVertex )
	{
		S2DVector *			pvDstUV ;
		const S2DVector *	pvSrcUV ;
		pNewMesh->m_bufUVMap.SetLength( nVertexCount ) ;
		pvDstUV = pNewMesh->m_bufUVMap.GetArray() ;
		pvSrcUV = pmoSrc->m_bufUVMap.GetConstArray() ;
		//
		for ( i = 0; i < nVertexCount; i ++ )
		{
			pvDstUV[i] = pvSrcUV[ pRemapIndex[i] ] ;
		}
		pNewMesh->m_bufUVMap.FinishArray() ;
	}
	//
	// 頂点色配列
	//
	if ( pmoSrc->m_bufColor.GetLength() >= pmoSrc->m_countVertex )
	{
		S3DColor *			pDstColor ;
		const S3DColor *	pSrcColor ;
		pNewMesh->m_bufColor.SetLength( nVertexCount ) ;
		pDstColor = pNewMesh->m_bufColor.GetArray() ;
		pSrcColor = pmoSrc->m_bufColor.GetConstArray() ;
		//
		for ( i = 0; i < nVertexCount; i ++ )
		{
			pDstColor[i] = pSrcColor[ pRemapIndex[i] ] ;
		}
		pNewMesh->m_bufColor.FinishArray() ;
	}
	//
	// 指標配列
	//
	const uint32_t *	pSrcIndex = pmoSrc->m_bufIndex.GetConstArray() ;
	for ( i = 0; i < pmoSrc->m_countPolygon; i ++ )
	{
		uint32_t	i0 = pSrcIndex[0] ;
		uint32_t	i1 = pSrcIndex[1] ;
		uint32_t	i2 = pSrcIndex[2] ;
		pSrcIndex += 3 ;
		//
		if ( (pMapIndex[i0] >= 0)
			&& (pMapIndex[i1] >= 0)
			&& (pMapIndex[i2] >= 0) )
		{
			pNewMesh->m_bufIndex.Add( (uint32_t) pMapIndex[i0] ) ;
			pNewMesh->m_bufIndex.Add( (uint32_t) pMapIndex[i1] ) ;
			pNewMesh->m_bufIndex.Add( (uint32_t) pMapIndex[i2] ) ;
			pNewMesh->m_countPolygon ++ ;
		}
	}
	for ( j = 0; j < VertexBuffer::countSubMesh; j ++ )
	{
		pNewMesh->m_countSubPoly[j] = 0 ;
		pSrcIndex = pmoSrc->m_bufSubIndex[j].GetConstArray() ;
		for ( i = 0; i < pmoSrc->m_countSubPoly[j]; i ++ )
		{
			uint32_t	i0 = pSrcIndex[0] ;
			uint32_t	i1 = pSrcIndex[1] ;
			uint32_t	i2 = pSrcIndex[2] ;
			pSrcIndex += 3 ;
			//
			if ( (pMapIndex[i0] >= 0)
				&& (pMapIndex[i1] >= 0)
				&& (pMapIndex[i2] >= 0) )
			{
				pNewMesh->m_bufSubIndex[j].Add( (uint32_t) pMapIndex[i0] ) ;
				pNewMesh->m_bufSubIndex[j].Add( (uint32_t) pMapIndex[i1] ) ;
				pNewMesh->m_bufSubIndex[j].Add( (uint32_t) pMapIndex[i2] ) ;
				pNewMesh->m_countSubPoly[j] ++ ;
			}
		}
	}
	//
	// 関連ボーン情報
	//
	for ( i = 0; i < pmoSrc->m_arrRelBone.GetLength(); i ++ )
	{
		S3DModelData::BONE_LINK_INFO *	pbli = pmoSrc->m_arrRelBone.GetAt( i ) ;
		ESLAssert( pbli != NULL ) ;
		if ( (pbli != NULL) && (pbli->pRelBone != NULL)
			&& pbli->pRelBone->IsBoneEffectVertexMap
						( pmoSrc->m_iVertex, maskVertexPortion ) )
		{
			//
			// ウェイトマップ複製
			//
			size_t	iDstVertex = pNewMesh->m_iVertex ;
			size_t	nDstVertCount = pNewMesh->m_countVertex ;
			pbli->pRelBone->ExpandBoneWeightBounds( iDstVertex, nDstVertCount ) ;
			//
			size_t	iSrcVertex = pmoSrc->m_iVertex ;
			size_t	nSrcVertCount = pmoSrc->m_countVertex ;
//			const float32_t *
//				pSrcWeight = pbli->pRelBone->GetBoneWeightMapBoundsAt
//											( iSrcVertex, nSrcVertCount ) ;
			const float32_t *
				pSrcWeight = pbli->pRelBone->LockBoneWeightMap
											( iSrcVertex, nSrcVertCount ) ;
			//
			SArray<float32_t>	bufWeight ;
			float32_t *	pTempWeight = bufWeight.GetArray( nDstVertCount ) ;
			for ( j = 0; j < nDstVertCount; j ++ )
			{
				pTempWeight[j] = pSrcWeight[ pRemapIndex[j] ] ;
			}
			pbli->pRelBone->UnlockBoneWeightMap( false ) ;
			pbli->pRelBone->ModifyBoneWeightMapBounds
						( iDstVertex, pTempWeight, nDstVertCount ) ;
			bufWeight.FinishArray() ;
			//
			// ボーンに影響メッシュを追加
			//
			pbli->pRelBone->MergeEffectiveMeshIndex( iNewMesh, iSrcMesh ) ;
		}
	}
	//
	// モーフィングターゲットを付け替える
	//
	if ( fMoveMorphMesh )
	{
		SBitArray	bitMorphEffect ;
		for ( i = 0; i < pmoSrc->m_arrMorphTarget.GetLength(); i ++ )
		{
			//
			// モーフィング影響反映確認
			//
			SString *	pstrMorphID = pmoSrc->m_arrMorphTarget.GetAt( i ) ;
			if ( pstrMorphID == NULL )
			{
				continue ;
			}
			MorphTargetMesh *
					pmtmMorph = m_ssoaMorphTarget.GetAs( *pstrMorphID ) ;
			if ( pmtmMorph == NULL )
			{
				continue ;
			}
			GetMorphMeshEffectMask( bitMorphEffect, *pmtmMorph ) ;
			if ( !maskVertexPortion.Test( 0, bitMorphEffect ) )
			{
				continue ;
			}
			//
			// モーフィングリマップ
			//
			MorphTargetMesh	mtmTemp = *pmtmMorph ;
			if ( pmtmMorph->m_bufVertex.GetLength() > 0 )
			{
				pmtmMorph->m_bufVertex.SetLength( nVertexCount ) ;
				pvDstVertex = pmtmMorph->m_bufVertex.GetArray() ;
				pvSrcVertex = mtmTemp.m_bufVertex.GetConstArray() ;
				for ( j = 0; j < nVertexCount; j ++ )
				{
					pvDstVertex[j] = pvSrcVertex[ pRemapIndex[j] ] ;
				}
				pmtmMorph->m_bufVertex.FinishArray() ;
			}
			if ( pmtmMorph->m_bufNormal.GetLength() > 0 )
			{
				pmtmMorph->m_bufNormal.SetLength( nVertexCount ) ;
				pvDstNormal = pmtmMorph->m_bufNormal.GetArray() ;
				pvSrcNormal = mtmTemp.m_bufNormal.GetConstArray() ;
				for ( j = 0; j < nVertexCount; j ++ )
				{
					pvDstNormal[j] = pvSrcNormal[ pRemapIndex[j] ] ;
				}
				pmtmMorph->m_bufNormal.FinishArray() ;
			}
			if ( pmtmMorph->m_bufUVMap.GetLength() > 0 )
			{
				S2DVector *			pvDstUV ;
				const S2DVector *	pvSrcUV ;
				pmtmMorph->m_bufUVMap.SetLength( nVertexCount ) ;
				pvDstUV = pmtmMorph->m_bufUVMap.GetArray() ;
				pvSrcUV = mtmTemp.m_bufUVMap.GetConstArray() ;
				for ( j = 0; j < nVertexCount; j ++ )
				{
					pvDstUV[j] = pvSrcUV[ pRemapIndex[j] ] ;
				}
				pmtmMorph->m_bufUVMap.FinishArray() ;
			}
			if ( pmtmMorph->m_bufColor.GetLength() > 0 )
			{
				S3DColor *			pDstColor ;
				const S3DColor *	pSrcColor ;
				pmtmMorph->m_bufColor.SetLength( nVertexCount ) ;
				pDstColor = pmtmMorph->m_bufColor.GetArray() ;
				pSrcColor = mtmTemp.m_bufColor.GetConstArray() ;
				for ( j = 0; j < nVertexCount; j ++ )
				{
					pDstColor[j] = pSrcColor[ pRemapIndex[j] ] ;
				}
				pmtmMorph->m_bufColor.FinishArray() ;
			}
			if ( pmtmMorph->m_bufWeight.GetLength() > 0 )
			{
				float32_t *			pDstWeight ;
				const float32_t *	pSrcWeight ;
				pmtmMorph->m_bufWeight.SetLength( nVertexCount ) ;
				pDstWeight = pmtmMorph->m_bufWeight.GetArray() ;
				pSrcWeight = mtmTemp.m_bufWeight.GetConstArray() ;
				for ( j = 0; j < nVertexCount; j ++ )
				{
					pDstWeight[j] = pSrcWeight[ pRemapIndex[j] ] ;
				}
				pmtmMorph->m_bufWeight.FinishArray() ;
			}
			pmtmMorph->m_countVertex = nVertexCount ;
			//
			// 付け替え
			//
			pNewMesh->m_arrMorphTarget.Add( new SString( *pstrMorphID ) ) ;
			pmoSrc->m_arrMorphTarget.RemoveAt( i -- ) ;
		}
	}
	//
	// ボーン正規化と VBO 再生成
	//
	if ( fRebuildVBO )
	{
		RebuildVertexBuffer() ;
	}
	return	sglErrSuccess ;
}

// メッシュ名を重複しない名前に正規化
//////////////////////////////////////////////////////////////////////////////
void S3DModelBuffer::NormalizeMeshID( SSystem::SString& strMeshID ) const
{
	if ( m_ssaMeshGroup.GetAs( strMeshID ) != NULL )
	{
		for ( int i = 1; i < 10000; i ++ )
		{
			SString	strTemp ;
			strTemp.Format( L"%s%d", (const wchar_t*) strMeshID, i ) ;
			if ( m_ssaMeshGroup.GetAs( strTemp ) == NULL )
			{
				strMeshID = strTemp ;
				break ;
			}
		}
	}
}

// メッシュ頂点ポリゴン隣接マスク生成
//////////////////////////////////////////////////////////////////////////////
void S3DModelBuffer::ExpandMeshEffectMaskNextPolygon
	( SSystem::SBitArray& maskEffect,
		const uint32_t * pPolyIndex, size_t nPolyCount )
{
	for ( size_t i = 0; i < nPolyCount; i ++ )
	{
		uint32_t	v0 = pPolyIndex[0] ;
		uint32_t	v1 = pPolyIndex[1] ;
		uint32_t	v2 = pPolyIndex[2] ;
		//
		if ( maskEffect.GetAt( v0 )
			|| maskEffect.GetAt( v1 )
			|| maskEffect.GetAt( v2 ) )
		{
			maskEffect.SetAt( v0, true ) ;
			maskEffect.SetAt( v1, true ) ;
			maskEffect.SetAt( v2, true ) ;
		}
		//
		pPolyIndex += 3 ;
	}
}

// モーフィングメッシュ影響範囲取得
//////////////////////////////////////////////////////////////////////////////
void S3DModelBuffer::GetMorphMeshEffectMask
	( SSystem::SBitArray& maskEffect,
		const S3DModelBuffer::MorphTargetMesh& mtmMesh )
{
	const float32_t *	pfpWeight = mtmMesh.m_bufWeight.GetConstArray() ;
	maskEffect.SetLength( mtmMesh.m_countVertex ) ;
	for ( size_t i = 0; i < mtmMesh.m_bufWeight.GetLength(); i ++ )
	{
		if ( pfpWeight[i] >= 1.0e-8 )
		{
			maskEffect.SetAt( i, true ) ;
		}
	}
}

// パノラマ画像から skybox モデルを生成
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelBuffer::BuildSkyboxFromPanoramaImage
	( double fpBoxScale, int nBoxSize,
		double degHRotAngle, SGLImageObject& imgPanorama )
{
	//
	// テクスチャ生成
	//
	SGLImage *		pSkyboxImage = new SGLImage ;
	SGLImageRect	rectFace[6] ;
	SGLError	err =
		MakeSkyboxFromPanoramaImage
			( *pSkyboxImage, rectFace, nBoxSize, degHRotAngle, imgPanorama ) ;
	if ( err )
	{
		delete	pSkyboxImage ;
		return	err ;
	}
	m_textures.AddSmartTextureAs( L"skybox", pSkyboxImage ) ;
	//
	// マテリアル登録
	//
	S3DMaterial *	pMaterial = new S3DMaterial ;
	S3DSurfaceAttribute	attr ;
	attr.flagsShading = shadingMethodNothing
					| shadingTextureSmoothing | shadingTextureMapping
					| shadingSingleSidePlane
					| shadingNoZBuffer | shadingHintPriority0
					| shadingNoShadowObject | shadingNoDropShadow
					| shadingNoReflectObject | shadingNoFogEffect ;
	attr.colorBase.rgbMul = 0x00FFFFFF ;
	attr.colorBase.rgbAdd = 0 ;
	attr.nAmbient = 0x100 ;
	pMaterial->SetSurfaceAttribute( attr ) ;
	pMaterial->SetTexture( pSkyboxImage, 0, S3DMaterial::textureDiffusion ) ;
	m_materials.AddSmartMaterialAs( L"skybox", pMaterial ) ;
	//
	// メッシュ追加
	//
	S3DVector	vCubeVertex[8] =
	{
		S3DVector( -1, -1, 1 ),
		S3DVector( 1, -1, 1 ),
		S3DVector( 1, -1, -1 ),
		S3DVector( -1, -1, -1 ),
		S3DVector( -1, 1, 1 ),
		S3DVector( 1, 1, 1 ),
		S3DVector( 1, 1, -1 ),
		S3DVector( -1, 1, -1 ),
	} ;
	uint32_t	iCubeFaces[6][4] =
	{
		{ 3, 0, 4, 7 },
		{ 3, 2, 1, 0 },
		{ 2, 3, 7, 6 },
		{ 1, 2, 6, 5 },
		{ 4, 5, 6, 7 },
		{ 0, 1, 5, 4 },
	} ;
	S3DVector4	vVertics[6][4] ;
	S3DVector4	vNormals[6][4] ;
	S2DVector	vUVMaps[6][4] ;
	uint32_t	nIndexes[6][6] ;
	//
	for ( int iFace = 0; iFace < 6; iFace ++ )
	{
		vVertics[iFace][0] = vCubeVertex[iCubeFaces[iFace][0]] * fpBoxScale ;
		vVertics[iFace][1] = vCubeVertex[iCubeFaces[iFace][1]] * fpBoxScale ;
		vVertics[iFace][2] = vCubeVertex[iCubeFaces[iFace][2]] * fpBoxScale ;
		vVertics[iFace][3] = vCubeVertex[iCubeFaces[iFace][3]] * fpBoxScale ;
		//
		S3DVector	vNormal = (vVertics[iFace][3] - vVertics[iFace][0])
								* (vVertics[iFace][1] - vVertics[iFace][0]) ;
		vNormal.Normalize() ;
		vNormals[iFace][0] = vNormal ;
		vNormals[iFace][1] = vNormal ;
		vNormals[iFace][2] = vNormal ;
		vNormals[iFace][3] = vNormal ;
		//
		SGLImageRect	rect = rectFace[iFace] ;
		vUVMaps[iFace][0] = S2DVector( rect.x, rect.y ) ;
		vUVMaps[iFace][1] = S2DVector( rect.x + rect.w, rect.y ) ;
		vUVMaps[iFace][2] = S2DVector( rect.x + rect.w, rect.y + rect.h ) ;
		vUVMaps[iFace][3] = S2DVector( rect.x, rect.y + rect.h ) ;
		//
		uint32_t	iBase = iFace * 4 ;
		nIndexes[iFace][0] = iBase ;
		nIndexes[iFace][1] = iBase + 3 ;
		nIndexes[iFace][2] = iBase + 2 ;
		nIndexes[iFace][3] = iBase ;
		nIndexes[iFace][4] = iBase + 2 ;
		nIndexes[iFace][5] = iBase + 1 ;
	}
	size_t	iMesh = GetMeshCount() ;
	AddIndexedTriangleList
		( pMaterial, 0, 6 * 2, 6 * 4,
			&vVertics[0][0], &vNormals[0][0],
			&vUVMaps[0][0], NULL, &nIndexes[0][0] ) ;
	//
	// メッシュ名
	//
	MeshGroup	mg ;
	mg.m_iFirstMesh = (uint32_t) iMesh ;
	mg.m_nMeshCount = 1 ;
	mg.m_vCenter.x = 0 ;
	mg.m_vCenter.y = 0 ;
	mg.m_vCenter.z = 0 ;
	//
	GetMeshGroupList().Add( L"skybox", mg ) ;
	//
	return	sglErrSuccess ;
}

SGLError S3DModelBuffer::MakeSkyboxFromPanoramaImage
	( SGLImageObject& imgSkybox,
		SGLImageRect* pCubeFaces,
		int nBoxSize, double degHRotAngle,
		SGLImageObject& imgPanorama )
{
	SGLImageInfo	imginf ;
	if ( imgPanorama.GetImageInfo( imginf ) )
	{
		return	sglErrFailed ;
	}
	imgSkybox.CreateImage
		( (uint32_t) nBoxSize * 4,
			(uint32_t) nBoxSize * 2, imginf.format, imginf.depth,
			SGLImageObject::bufferCompressedTexture ) ;
	//
	// 水平帯４面サンプリング
	//
	SGLImageObject::CubeMapFrameIndex	cmfiSideFace[4] =
	{
		SGLImageObject::cubemapNegativeX,
		SGLImageObject::cubemapPositiveZ,
		SGLImageObject::cubemapPositiveX,
		SGLImageObject::cubemapNegativeZ,
	} ;
	for ( int iFace = 0; iFace < 4; iFace ++ )
	{
		SGLImageObject::CubeMapFrameIndex	cmfi = cmfiSideFace[iFace] ;
		pCubeFaces[cmfi].x = (int32_t) (iFace * nBoxSize) ;
		pCubeFaces[cmfi].y = (int32_t) nBoxSize ;
		pCubeFaces[cmfi].w = (int32_t) nBoxSize ;
		pCubeFaces[cmfi].h = (int32_t) nBoxSize ;
		//
		SGLImageRect	rectFace = pCubeFaces[cmfi] ;
		//
		S3DVector	vScreen ;
		vScreen.x = (float32_t) rectFace.x + (float32_t) rectFace.w * 0.5f ;
		vScreen.y = (float32_t) rectFace.y + (float32_t) rectFace.h * 0.5f ;
		vScreen.z = (float32_t) rectFace.w * 0.5f ;
		//
		S3DMatrix	matFace( 1, 1, 1 ) ;
		double	rad = (degHRotAngle + (iFace - 1) * 90) * PI / 180.0 ;
		matFace.RevolveOnY( sin(rad), cos(rad) ) ;
		//
		rectFace.y -- ;
		rectFace.h ++ ;
		//
		SampleSkyboxFaceForPanoramaImage
			( imgSkybox, rectFace, matFace, vScreen, imgPanorama ) ;
	}
	//
	// 上面サンプリング
	//
	SGLImageObject::CubeMapFrameIndex
					cmfiUp = SGLImageObject::cubemapNegativeY ;
	SGLImageRect	rectFaceUp( nBoxSize + 1, 1,
								nBoxSize - 2, nBoxSize - 2 ) ;
	pCubeFaces[cmfiUp] = rectFaceUp ;
	//
	S3DVector	vScreenUp ;
	vScreenUp.x = (float32_t) rectFaceUp.x + (float32_t) rectFaceUp.w * 0.5f ;
	vScreenUp.y = (float32_t) rectFaceUp.y + (float32_t) rectFaceUp.h * 0.5f ;
	vScreenUp.z = (float32_t) rectFaceUp.w * 0.5f ;
	//
	S3DMatrix	matFaceUp( 1, 1, 1 ) ;
	double	rad = degHRotAngle * PI / 180.0 ;
	matFaceUp.RevolveOnY( sin(rad), cos(rad) ) ;
	matFaceUp.RevolveOnX( -1, 0 ) ;
	//
	rectFaceUp.x -- ;
	rectFaceUp.y -- ;
	rectFaceUp.w += 2 ;
	rectFaceUp.h ++ ;
	//
	SampleSkyboxFaceForPanoramaImage
		( imgSkybox, rectFaceUp, matFaceUp, vScreenUp, imgPanorama ) ;
	//
	// 底面サンプリング
	//
	SGLImageObject::CubeMapFrameIndex
					cmfiDown = SGLImageObject::cubemapPositiveY ;
	SGLImageRect	rectFaceDown( nBoxSize * 3 + 1, 0,
									nBoxSize - 2, nBoxSize - 2 ) ;
	pCubeFaces[cmfiDown] = rectFaceDown ;
	//
	S3DVector	vScreenDown ;
	vScreenDown.x = (float32_t) rectFaceDown.x + (float32_t) rectFaceDown.w * 0.5f ;
	vScreenDown.y = (float32_t) rectFaceDown.y + (float32_t) rectFaceDown.h * 0.5f ;
	vScreenDown.z = (float32_t) rectFaceDown.w * 0.5f ;
	//
	S3DMatrix	matFaceDown( 1, 1, 1 ) ;
	matFaceDown.RevolveOnY( sin(rad), cos(rad) ) ;
	matFaceDown.RevolveOnX( 1, 0 ) ;
	//
	rectFaceDown.x -- ;
	rectFaceDown.w += 2 ;
	rectFaceDown.h ++ ;
	//
	SampleSkyboxFaceForPanoramaImage
		( imgSkybox, rectFaceDown, matFaceDown, vScreenDown, imgPanorama ) ;
	//
	return	sglErrSuccess ;
}

void S3DModelBuffer::SampleSkyboxFaceForPanoramaImage
	( SGLImageObject& imgSkybox,
		SGLImageRect& rectFace,
		const S3DMatrix& matFace,
		const S3DVector& vScreen,
		SGLImageObject& imgPanorama )
{
	for ( int yDst = 0; yDst < rectFace.h; yDst ++ )
	{
		for ( int xDst = 0; xDst < rectFace.w; xDst ++ )
		{
			S3DVector	vRay ;
			vRay.x = (float32_t) (rectFace.x + xDst) - vScreen.x ;
			vRay.y = (float32_t) (rectFace.y + yDst) - vScreen.y ;
			vRay.z = vScreen.z ;
			//
			matFace.RevolveVector( vRay ) ;
			//
			vRay.Normalize() ;
			//
			imgSkybox.SetPixel
				( rectFace.x + xDst,
					rectFace.y + yDst,
					SampleByRayFromPanoramaImage( imgPanorama, vRay ) ) ;
		}
	}
}

SGLPalette S3DModelBuffer::SampleByRayFromPanoramaImage
	( SGLImageObject& imgPanorama, const S3DVector& vRay )
{
	SGLSize	sizePanorama = imgPanorama.GetImageSize() ;
	double	xRadScale = sizePanorama.w / (2.0 * PI) ;
	double	yRadScale = sizePanorama.h / PI ;
	//
	double	r = sqrt( vRay.x * vRay.x + vRay.z * vRay.z ) ;
	double	radX = 0.0 ;
	double	radY = acos( - vRay.y ) ;
	if ( r > 1.0e-8 )
	{
		radX = atan2( - vRay.z, vRay.x ) ;
	}
	//
	const double		x = radX * xRadScale ;
	const double		y = radY * yRadScale ;
	const int			fx = (int) esl_lroundfi( x * 0x100 ) ;
	const int			fy = (int) esl_lroundfi( y * 0x100 ) ;
	const int			ix = ((fx >> 8) + sizePanorama.w) % sizePanorama.w ;
	const int			iy = fy >> 8 ;
	const uint32_t	dx = fx & 0xFF ;
	const uint32_t	dy = fy & 0xFF ;
	const int			ix1 = (ix + 1) % sizePanorama.w ;
	const int			iy1 = esl_min( iy + 1, sizePanorama.h - 1 ) ;
	//
	SGLPalette	rgb00, rgb01, rgb10, rgb11 ;
	imgPanorama.GetPixel( rgb00, ix, iy ) ;
	imgPanorama.GetPixel( rgb01, ix1, iy ) ;
	imgPanorama.GetPixel( rgb10, ix, iy1 ) ;
	imgPanorama.GetPixel( rgb11, ix1, iy1 ) ;
	//
	SGLPalette	rgb0 = rgb00.imul(0x100U - dx) + rgb01.imul(dx) ;
	SGLPalette	rgb1 = rgb10.imul(0x100U - dx) + rgb11.imul(dx) ;
	SGLPalette	rgb = rgb0.imul(0x100U - dy) + rgb1.imul(dy) ;
	//
	return	rgb ;
}

// パノラマ画像から八面体モデルを生成
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelBuffer::BuildOctahedronFromPanoramaImage
	( double fpOctahedronSize, int nOctMapSize,
		double degHRotAngle, SGLImageObject& imgPanorama, bool flagDivFace )
{
	// テクスチャ生成
	//
	SGLImage *		pOctMapImage = new SGLImage ;
	SGLError	err =
		MakeOctahedronMapFromPanoramaImage
			( *pOctMapImage, nOctMapSize, degHRotAngle, imgPanorama ) ;
	if ( err )
	{
		delete	pOctMapImage ;
		return	err ;
	}
	m_textures.AddSmartTextureAs( L"octmap", pOctMapImage ) ;
	//
	// マテリアル登録
	//
	S3DMaterial *	pMaterial = new S3DMaterial ;
	S3DSurfaceAttribute	attr ;
	attr.flagsShading = shadingMethodNothing
					| shadingTextureSmoothing | shadingTextureMapping
					| shadingSingleSidePlane
					| shadingNormalizedUVScale
					| shadingNoZBuffer | shadingHintPriority0
					| shadingNoShadowObject | shadingNoDropShadow
					| shadingNoReflectObject | shadingNoFogEffect ;
	attr.colorBase.rgbMul = 0x00FFFFFF ;
	attr.colorBase.rgbAdd = 0 ;
	attr.nAmbient = 0x100 ;
	pMaterial->SetSurfaceAttribute( attr ) ;
	pMaterial->SetTexture( pOctMapImage, 0, S3DMaterial::textureDiffusion ) ;
	m_materials.AddSmartMaterialAs( L"octmap", pMaterial ) ;
	//
	// メッシュ追加
	//
	S3DVector	vOctVertex[6] =
	{
		S3DVector( 0, -1, 0 ),
		S3DVector( 1, 0, 0 ),
		S3DVector( 0, 0, 1 ),
		S3DVector( -1, 0, 0 ),
		S3DVector( 0, 0, -1 ),
		S3DVector( 0, 1, 0 ),
	} ;
	S2DVector	vOctUVMap[9] =
	{
		S2DVector( 0.5, 0.5 ),
		S2DVector( 1.0, 0.5 ),
		S2DVector( 0.5, 0.0 ),
		S2DVector( 0.0, 0.5 ),
		S2DVector( 0.5, 1.0 ),
		S2DVector( 1.0, 0.0 ),
		S2DVector( 0.0, 0.0 ),
		S2DVector( 0.0, 1.0 ),
		S2DVector( 1.0, 1.0 ),
	} ;
	uint32_t	iOctFaces[8][3] =
	{
		{ 0, 1, 2 },
		{ 0, 2, 3 },
		{ 0, 3, 4 },
		{ 0, 4, 1 },
		{ 2, 1, 5 },
		{ 3, 2, 5 },
		{ 4, 3, 5 },
		{ 1, 4, 5 },
	} ;
	uint32_t	iOctUVs[8][3] =
	{
		{ 0, 1, 2 },
		{ 0, 2, 3 },
		{ 0, 3, 4 },
		{ 0, 4, 1 },
		{ 2, 1, 5 },
		{ 3, 2, 6 },
		{ 4, 3, 7 },
		{ 1, 4, 8 },
	} ;
	S3DVector4	vVertics[8*2][3] ;
	S3DVector4	vNormals[8*2][3] ;
	S2DVector	vUVMaps[8*2][3] ;
	uint32_t	nIndexes[8*4][3] ;
	//
	for ( int iFace = 0; iFace < 8; iFace ++ )
	{
		const size_t	ivFace = flagDivFace ? iFace * 2 : iFace ;
		vVertics[ivFace][0] = vOctVertex[iOctFaces[iFace][0]] * fpOctahedronSize ;
		vVertics[ivFace][1] = vOctVertex[iOctFaces[iFace][1]] * fpOctahedronSize ;
		vVertics[ivFace][2] = vOctVertex[iOctFaces[iFace][2]] * fpOctahedronSize ;
		//
		S3DVector	vNormal = (vVertics[ivFace][2] - vVertics[ivFace][0])
								* (vVertics[ivFace][1] - vVertics[ivFace][0]) ;
		vNormal.Normalize() ;
		vNormals[ivFace][0] = vNormal ;
		vNormals[ivFace][1] = vNormal ;
		vNormals[ivFace][2] = vNormal ;
		//
		vUVMaps[ivFace][0] = vOctUVMap[iOctUVs[iFace][0]] ;
		vUVMaps[ivFace][1] = vOctUVMap[iOctUVs[iFace][1]] ;
		vUVMaps[ivFace][2] = vOctUVMap[iOctUVs[iFace][2]] ;
		//
		if ( flagDivFace )
		{
			const size_t	ivFace1 = ivFace + 1 ;
			vVertics[ivFace1][0] = (vVertics[ivFace][0] + vVertics[ivFace][1]) * 0.5f ;
			vVertics[ivFace1][1] = (vVertics[ivFace][0] + vVertics[ivFace][2]) * 0.5f ;
			vVertics[ivFace1][2] = (vVertics[ivFace][1] + vVertics[ivFace][2]) * 0.5f ;
			//
			vNormals[ivFace1][0] = vNormal ;
			vNormals[ivFace1][1] = vNormal ;
			vNormals[ivFace1][2] = vNormal ;
			//
			vUVMaps[ivFace1][0] = (vUVMaps[ivFace][0] + vUVMaps[ivFace][1]) * 0.5f ;
			vUVMaps[ivFace1][1] = (vUVMaps[ivFace][0] + vUVMaps[ivFace][2]) * 0.5f ;
			vUVMaps[ivFace1][2] = (vUVMaps[ivFace][1] + vUVMaps[ivFace][2]) * 0.5f ;
		}
		//
		const uint32_t	iBase = (uint32_t) ivFace * 3 ;
		size_t			idxFace = iFace ;
		if ( flagDivFace )
		{
			idxFace *= 4 ;
			nIndexes[idxFace][0] = iBase ;
			nIndexes[idxFace][1] = iBase + 4 ;
			nIndexes[idxFace][2] = iBase + 3 ;
			nIndexes[idxFace+1][0] = iBase + 3 ;
			nIndexes[idxFace+1][1] = iBase + 4 ;
			nIndexes[idxFace+1][2] = iBase + 5 ;
			nIndexes[idxFace+2][0] = iBase + 4 ;
			nIndexes[idxFace+2][1] = iBase + 2 ;
			nIndexes[idxFace+2][2] = iBase + 5 ;
			nIndexes[idxFace+3][0] = iBase + 3 ;
			nIndexes[idxFace+3][1] = iBase + 5 ;
			nIndexes[idxFace+3][2] = iBase + 1 ;
		}
		else
		{
			nIndexes[idxFace][0] = iBase ;
			nIndexes[idxFace][1] = iBase + 2 ;
			nIndexes[idxFace][2] = iBase + 1 ;
		}
	}
	size_t	iMesh = GetMeshCount() ;
	if ( flagDivFace )
	{
		AddIndexedTriangleList
			( pMaterial, 0, 8 * 4, 8 * 2 * 3,
				&vVertics[0][0], &vNormals[0][0],
				&vUVMaps[0][0], nullptr, &nIndexes[0][0] ) ;
	}
	else
	{
		AddIndexedTriangleList
			( pMaterial, 0, 8, 8 * 3,
				&vVertics[0][0], &vNormals[0][0],
				&vUVMaps[0][0], nullptr, &nIndexes[0][0] ) ;
	}
	//
	// メッシュ名
	//
	MeshGroup	mg ;
	mg.m_iFirstMesh = (uint32_t) iMesh ;
	mg.m_nMeshCount = 1 ;
	mg.m_vCenter.x = 0 ;
	mg.m_vCenter.y = 0 ;
	mg.m_vCenter.z = 0 ;
	//
	GetMeshGroupList().Add( L"octahedron", mg ) ;
	//
	return	sglErrSuccess ;
}

SGLError S3DModelBuffer::MakeOctahedronMapFromPanoramaImage
	( SGLImageObject& imgOctMap, int nOctMapSize,
		double degHRotAngle, SGLImageObject& imgPanorama )
{
	SGLImageInfo	imginf ;
	if ( imgPanorama.GetImageInfo( imginf ) )
	{
		return	sglErrFailed ;
	}
	imgOctMap.CreateImage
		( (uint32_t) nOctMapSize,
			(uint32_t) nOctMapSize, imginf.format, imginf.depth,
			SGLImageObject::bufferCompressedTexture ) ;
	//
	const int	c_iFace[4] =
	{
		1, 0, 2, 3
	} ;
	const S2DVector	c_vUVCorner[4] =
	{
		S2DVector( 1.0, 0.0 ),
		S2DVector( 0.0, 0.0 ),
		S2DVector( 0.0, 1.0 ),
		S2DVector( 1.0, 1.0 ),
	} ;
	const S2DVector	vUVCenter( 0.5f, 0.5f ) ;
	const double	fpCornerLen = sqrt( 0.5 ) ;
	const double	fpRcpCornerLen = 1.0 / fpCornerLen ;
	//
	for ( int yDst = 0; yDst < nOctMapSize; yDst ++ )
	{
		const float32_t	yfDst = (float32_t) yDst / (float32_t) (nOctMapSize - 1) ;
		for ( int xDst = 0; xDst < nOctMapSize; xDst ++ )
		{
			const float32_t	xfDst = (float32_t) xDst / (float32_t) (nOctMapSize - 1) ;
			const int		iFace = c_iFace[((yDst >= (nOctMapSize >> 1)) ? 2 : 0)
											| ((xDst >= (nOctMapSize >> 1)) ? 1 : 0)] ;
			//
			S2DVector	vCornerDir = (c_vUVCorner[iFace] - vUVCenter) * fpRcpCornerLen ;
			S2DVector	vOffset( xfDst - vUVCenter.x, yfDst - vUVCenter.y ) ;
			//
			S3DVector	vRay ;
			float32_t	r =
				(float32_t) (vOffset.InnerProduct( vCornerDir ) * fpRcpCornerLen) ;
			if( r <= 0.5 )
			{
				vRay.y = -1.0f + r * 2.0f ;
			}
			else
			{
				vRay.y = (r - 0.5f) * 2.0f ;
				vOffset -= vCornerDir * (vRay.y * fpCornerLen) ;
			}
			vRay.x = vOffset.x * 2.0f ;
			vRay.z = - vOffset.y * 2.0f ;
			vRay.Normalize() ;
			//
			imgOctMap.SetPixel
				( xDst, yDst,
					SampleByRayFromPanoramaImage( imgPanorama, vRay ) ) ;
		}
	}
	return	sglErrSuccess ;
}

// 形状をメッシュテクスチャに投影（法線も生成）
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelBuffer::ProjectMeshTextureForShape
	( const S3DModelBuffer::ProjectMeshTextureParam& pmtp,
		S3DCollider& collider,
		S3DModelBuffer::ProgressNotification * pNotification ) const
{
	//
	// 出力先画像
	//
	if ( pmtp.pDiffusionTexture == NULL )
	{
		return	sglErrFailed ;
	}
	SGLImageBuffer		bufDiffusion, bufLuminous, bufNormal ;
	SGLImageSmartBuffer	isbDiffusion, isbLuminous, isbNormal ;
	isbDiffusion.Lock
		( bufDiffusion, pmtp.pDiffusionTexture, SGLImageObject::lockWrite ) ;
	if ( bufDiffusion.depth != 32 )
	{
		return	sglErrFailed ;
	}
	//
	if ( pmtp.pLuminousTexture != NULL )
	{
		isbLuminous.Lock
			( bufLuminous, pmtp.pLuminousTexture, SGLImageObject::lockWrite ) ;
		if ( (bufLuminous.depth != 32)
			|| (bufDiffusion.width != bufLuminous.width)
			|| (bufDiffusion.height != bufLuminous.height) )
		{
			return	sglErrFailed ;
		}
	}
	if ( pmtp.pNormalTexture != NULL )
	{
		isbNormal.Lock
			( bufNormal, pmtp.pNormalTexture, SGLImageObject::lockWrite ) ;
		if ( (bufNormal.depth != 32)
			|| (bufDiffusion.width != bufNormal.width)
			|| (bufDiffusion.height != bufNormal.height) )
		{
			return	sglErrFailed ;
		}
	}
	//
	// 実行
	//
	ProjectMeshTextureProcInstance	instance[32] ;
	size_t	nThreadCount = esl_min( SSystem::g_cpuLogicalCount, 32 ) ;
	//
	void *	pInstance[32] ;
	for ( size_t i = 0; i < nThreadCount; i ++ )
	{
		pInstance[i] = &instance[i] ;
	}
	ProjectMeshTextureProc	proc
		( *this, pmtp, &bufDiffusion,
			(pmtp.pLuminousTexture != NULL) ? &bufLuminous : NULL,
			(pmtp.pNormalTexture != NULL) ? &bufNormal : NULL,
			collider, pNotification ) ;
	proc.Start( pInstance, nThreadCount ) ;
	//
	if ( proc.IsCanceled() )
	{
		return	sglErrFailed ;
	}
	if ( pNotification != NULL )
	{
		pNotification->OnProgress
			( bufDiffusion.width * bufDiffusion.height,
				bufDiffusion.width * bufDiffusion.height ) ;
	}
	return	sglErrSuccess ;
}

// ProjectMeshTextureProc 構築関数
S3DModelBuffer::ProjectMeshTextureProc::ProjectMeshTextureProc
	( const S3DModelBuffer& model,
		const ProjectMeshTextureParam& pmtp,
		SGLImageBuffer * pDiffusion,
		SGLImageBuffer * pLuminous,
		SGLImageBuffer * pNormal,
		S3DCollider & collider,
		ProgressNotification * pNotification )
	: m_model( model ), m_pmtp( pmtp ),
		m_pDiffusion( pDiffusion ),
		m_pLuminous( pLuminous ), m_pNormal( pNormal ),
		m_yLineCount( pDiffusion->height ),
		m_yNextLine( 0 ),
		m_collider( collider ),
		m_pNotification( pNotification ),
		m_flagCanceled( false )
{
}

// ProjectMeshTextureProc ループ処理／終了判定関数
bool S3DModelBuffer::ProjectMeshTextureProc::Continue( void * pInstance )
{
	if ( m_flagCanceled )
	{
		return	false ;
	}
	if ( m_yNextLine >= m_yLineCount )
	{
		return	false ;
	}
	ProjectMeshTextureProcInstance *
		ppmtpi = (ProjectMeshTextureProcInstance*) pInstance ;
	ppmtpi->yLine = m_yNextLine ++ ;
	return	true ;
}

// ProjectMeshTextureProc 並列処理関数
void S3DModelBuffer::ProjectMeshTextureProc::RunParallel( void * pInstance )
{
	ProjectMeshTextureProcInstance *
		ppmtpi = (ProjectMeshTextureProcInstance*) pInstance ;
	if ( m_pmtp.nFlags & flagProjTexWithLight )
	{
		m_pmtp.vVecLight.Normalize() ;
	}
	for ( size_t x = 0; (x < m_pDiffusion->width) && !m_flagCanceled; x ++ )
	{
		//
		// 投影座標
		//
		S2DVector	uv( (double) x, (double) ppmtpi->yLine ) ;
		ppmtpi->aMeshPoints.RemoveAll() ;
		if ( m_model.GetMeshPointInfoAtTexturePos
				( ppmtpi->aMeshPoints, 1, m_pmtp.iMesh, uv ) < 1 )
		{
			continue ;
		}
		MeshPointInfo&	mpi = ppmtpi->aMeshPoints.At(0) ;
		mpi.vNormal.Normalize() ;
		//
		// 透視当たり判定
		//
		S3DDVector	vPos0 = mpi.vPoint + mpi.vNormal * m_pmtp.fpFrontReach ;
		S3DDVector	vPos1 = mpi.vPoint - mpi.vNormal * m_pmtp.fpBackReach ;
		//
		S3DCollisionResult	rsHit ;
		rsHit.fpDistance =
			(float32_t) (m_pmtp.fpBackReach + m_pmtp.fpFrontReach) ;
		//
		if ( !m_collider.IsSegmentCrossing
			( vPos0, vPos1, m_pmtp.fpHitErrorGap, rsHit ) )
		{
			continue ;
		}
		//
		// 頂点属性取得
		//
		rsHit.ComputeHitLocalCoord() ;
		rsHit.ComplementeAndSampleAttributes() ;
		//
		// テクスチャサンプリング
		//
		SGLPalette	argbDiffusion =
						rsHit.vtxColor * rsHit.attrTexture.argbDiffusion ;
//		if ( (rsHit.pMaterial != NULL)
//			&& rsHit.attrTexture.nTextureFlags
//					& (1 << S3DMaterial::textureDiffusion) )
		{
			argbDiffusion.argb.Alpha = 0xFF ;
		}
		//
		// シェーディング
		//
		if ( m_pmtp.nFlags & flagProjTexWithLight )
		{
			float32_t	f, l, s ;
			S3DVector	vRefl = m_pmtp.vVecLight ;
			rsHit.vNormalLocal.Normalize() ;
			f = - rsHit.vNormalLocal.InnerProduct( mpi.vNormal ) ;
			l = rsHit.vNormalLocal.InnerProduct( m_pmtp.vVecLight ) ;
			vRefl -= rsHit.vNormalLocal * (l * 2.0) ;
			s = esl_fmaxf( vRefl.InnerProduct( mpi.vNormal ), 0.0f ) ;
			if ( f * l < 0.0f )
			{
				l = - l ;
			}
			if ( rsHit.pMaterial != NULL )
			{
				l = esl_fmaxf( - l, 0.0f ) *
					(float32_t) (rsHit.pMaterial->m_attrSurface.nDiffusion
											* m_pmtp.fpBrightness / 256.0)
					+ m_pmtp.fpAmbientLight ;
				s = (float32_t) pow( s, 256.0 / esl_max(rsHit.pMaterial->m_attrSurface.nSpecularSize,1) )
					* (float32_t) (rsHit.pMaterial->m_attrSurface.nSpecular / 256.0) ;
			}
			else
			{
				l = esl_fmaxf( - l, 0.0f ) * m_pmtp.fpBrightness
												+ m_pmtp.fpAmbientLight ;
				s *= s ;
			}
			argbDiffusion *= (double) esl_fminf( l, 1.0f ) ;
			argbDiffusion += SGLPalette( 0xFFFFFF ) * (double) esl_fminf( s, 1.0f ) ;
			argbDiffusion.argb.Alpha = 0xFF ;
		}
		//
		// テクスチャ出力
		//
		((uint32_t*)(m_pDiffusion->ptrBuffer
					+ m_pDiffusion->pitchLine * (int) ppmtpi->yLine))[x] = argbDiffusion ;
		//
		if ( m_pLuminous != NULL )
		{
			((uint32_t*)(m_pLuminous->ptrBuffer
						+ m_pLuminous->pitchLine * (int) ppmtpi->yLine))[x]
							= rsHit.attrTexture.argbLuminous.ui32 ;
		}
		//
		if ( m_pNormal != NULL )
		{
			S3DVector	vAxisZ ;
			rsHit.vNormalLocal.Normalize() ;
			if ( rsHit.vNormalLocal.InnerProduct( mpi.vNormal ) < 0 )
			{
				rsHit.vNormalLocal = - rsHit.vNormalLocal ;
			}
			mpi.vAxisX.Normalize() ;
			mpi.vAxisY.Normalize() ;
			vAxisZ = mpi.vAxisX * mpi.vAxisY ;
			if ( vAxisZ.InnerProduct( mpi.vNormal ) < 0 )
			{
				vAxisZ = - vAxisZ ;
			}
			//
			SGLPalette	argbTexture ;
			argbTexture.argb.Red =
				(uint8_t) esl_clampi( esl_roundfi
					( mpi.vAxisX.InnerProduct(rsHit.vNormalLocal) * 127.0f ) + 0x80, 0, 255 ) ;
			argbTexture.argb.Green =
				(uint8_t) esl_clampi( esl_roundfi
					( mpi.vAxisY.InnerProduct(rsHit.vNormalLocal) * -127.0f ) + 0x80, 0, 255 ) ;
			argbTexture.argb.Blue =
				(uint8_t) esl_clampi( esl_roundfi
					( vAxisZ.InnerProduct(rsHit.vNormalLocal) * 127.0f ) + 0x80, 0, 255 ) ;
			argbTexture.argb.Alpha = 0xFF ;
			//
			((uint32_t*)(m_pNormal->ptrBuffer
						+ m_pNormal->pitchLine * (int) ppmtpi->yLine))[x] = argbTexture.ui32 ;
		}
		//
		// 進捗通知
		//
		if ( (m_pNotification != NULL)
			&& (m_timer.GetRealTime() > 10.0) )
		{
			if ( !m_pNotification->OnProgress
				( (unsigned long) (ppmtpi->yLine * m_pDiffusion->width + x),
					(unsigned long) (m_pDiffusion->width * m_pDiffusion->height) ) )
			{
				m_flagCanceled = true ;
				return ;
			}
			m_timer.Reset() ;
		}
	}
}

// AO／GIを頂点色へ反映する
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelBuffer::GlobalIlluminationForVertexColor
	( const S3DModelBuffer::GlobalIlluminationParam& gip,
		S3DCollider& collider,
		S3DModelBuffer::ProgressNotification * pNotification )
{
	//
	// 頂点座標取得
	//
	if ( gip.pVBO == NULL )
	{
		return	sglErrFailed ;
	}
	S3DVertexBufferInterface::MeshInfo	mi ;
	eslFillMemory( &mi, 0, sizeof(S3DVertexBufferInterface::MeshInfo) ) ;
	if ( gip.pVBO->GetMeshInfoAt( mi, gip.iMesh, 0 ) )
	{
		return	sglErrFailed ;
	}
	SArray<S3DVector4>	vVertex, vNormal ;
	mi.pvVertex = vVertex.GetArray( mi.countVertex ) ;
	mi.pvNormal = vNormal.GetArray( mi.countVertex ) ;
	if ( gip.pVBO->GetMeshInfoAt( mi, gip.iMesh, mi.countVertex ) )
	{
		vVertex.FinishArray() ;
		vNormal.FinishArray() ;
		return	sglErrFailed ;
	}
	vVertex.FinishArray() ;
	vNormal.FinishArray() ;
	//
	// 頂点毎に光線を飛ばす
	//
	GlobalIlluminationProcInstance	instance[32] ;
	size_t	nThreadCount = esl_min( SSystem::g_cpuLogicalCount, 32 ) ;
	//
	SArray<S3DColor>	aColor ;
	S3DColor *	pColor = aColor.GetArray( mi.countVertex ) ;
	//
	void *	pInstance[32] ;
	for ( size_t i = 0; i < nThreadCount; i ++ )
	{
		pInstance[i] = &instance[i] ;
	}
	GlobalIlluminationProc	proc
		( gip, mi.pvVertex, mi.pvNormal, pColor,
			mi.countVertex, collider, pNotification ) ;
	proc.Start( pInstance, nThreadCount ) ;
	//
	if ( proc.IsCanceled() )
	{
		return	sglErrFailed ;
	}
	if ( pNotification != NULL )
	{
		pNotification->OnProgress( mi.countVertex, mi.countVertex ) ;
	}
	aColor.FinishArray() ;
	//
	// 頂点色更新
	//
	return	gip.pVBO->UpdateIndexedPrimitiveList
				( gip.iMesh, 0,
					mi.countPrimitive
						* GetPrimitiveVertexCount( mi.typeMesh ),
					mi.countVertex, NULL, NULL, NULL, pColor, NULL ) ;
}

// GlobalIlluminationProc 構築関数
S3DModelBuffer::GlobalIlluminationProc::GlobalIlluminationProc
	( const GlobalIlluminationParam& gip,
		const S3DVector4 * pvVertex,
		const S3DVector4 * pvNormal,
		S3DColor * pColor, size_t nVertexCount,
		S3DCollider & collider,
		ProgressNotification * pNotification )
	: m_gip( gip ),
		m_pvVertex( pvVertex ), m_pvNormal( pvNormal ),
		m_pColor( pColor ), m_nVertexCount( nVertexCount ),
		m_iNextVertex( 0 ), m_collider( collider ),
		m_pNotification( pNotification ), m_flagCanceled( false )
{
}

// GlobalIlluminationProc ループ処理／終了判定関数
bool S3DModelBuffer::GlobalIlluminationProc::Continue( void * pInstance )
{
	if ( m_flagCanceled )
	{
		return	false ;
	}
	if ( (m_pNotification != NULL)
		&& (m_timer.GetRealTime() > 10.0) )
	{
		if ( !m_pNotification->OnProgress
			( (unsigned long) m_iNextVertex,
				(unsigned long) m_nVertexCount ) )
		{
			m_flagCanceled = true ;
			return	false ;
		}
		m_timer.Reset() ;
	}
	if ( m_iNextVertex >= m_nVertexCount )
	{
		return	false ;
	}
	GlobalIlluminationProcInstance *
		pgipi = (GlobalIlluminationProcInstance*) pInstance ;
	pgipi->iVertex = m_iNextVertex ++ ;
	return	true ;
}

// GlobalIlluminationProc 並列処理関数
void S3DModelBuffer::GlobalIlluminationProc::RunParallel( void * pInstance )
{
	GlobalIlluminationProcInstance *
		pgipi = (GlobalIlluminationProcInstance*) pInstance ;
	//
	 size_t		iVertex = pgipi->iVertex ;
	S3DVector	vPos0 = m_gip.matItem * m_pvVertex[iVertex] + m_gip.vItem ;
	S3DVector	vNormal = m_gip.matItem * m_pvNormal[iVertex] ;
	vNormal.Normalize() ;
	//
	S3DMatrix	matDir( 1, 1, 1 ) ;
	matDir.RevolveForAngle( vNormal ) ;
	//
	vPos0 += vNormal * m_gip.fpErrorGap ;
	//
	S3DVector	vGI( 0, 0, 0 ) ;
	double		fpAO = 0.0 ;
	for ( size_t j = 0; j < m_gip.nSamplingCount; j ++ )
	{
		S3DVector	vDir ;
		double	rnd = pgipi->randomizer.QuickRandomFloat( 1.0f ) ;
		double	radX = (2.0 - 2.0 * sqrt( 1.0 - rnd )) * (PI * 0.25) ;
		double	radZ = pgipi->randomizer.QuickRandomDouble( PI ) ;
		vDir.x = (float32_t) (sin( radX ) * cos( radZ )) ;
		vDir.y = (float32_t) (sin( radX ) * sin( radZ )) ;
		vDir.z = (float32_t) cos( radX ) ;
		matDir.RevolveVector( vDir ) ;
		//
		S3DCollisionResult	rsHit ;
		rsHit.fpDistance = m_gip.fpReachLength ;
		//
		S3DVector	vPos1 = vPos0 + vDir * m_gip.fpReachLength ;
		if ( !m_collider.IsSegmentCrossing
				( vPos0, vPos1, m_gip.fpErrorGap, rsHit ) )
		{
			continue ;
		}
		//
		// 衝突処理
		//
		rsHit.ComputeHitLocalCoord() ;
		rsHit.ComplementeAndSampleAttributes() ;
		//
		fpAO += 1.0 ;
		//
		if ( rsHit.pMaterial != NULL )
		{
			//
			// GI 反映
			//
			vGI.x += rsHit.attrTexture.argbLuminous.argb.Red
							* rsHit.attrTexture.fpLuminousApply ;
			vGI.y += rsHit.attrTexture.argbLuminous.argb.Green
							* rsHit.attrTexture.fpLuminousApply ;
			vGI.z += rsHit.attrTexture.argbLuminous.argb.Blue
							* rsHit.attrTexture.fpLuminousApply ;
			//
			if ( rsHit.pMaterial->m_attrSurface.nEmission > 0 )
			{
				SGLPalette	rgbDiffusion ;
				if ( rsHit.attrTexture.nTextureFlags
						& (1 << S3DMaterial::textureDiffusion) )
				{
					rgbDiffusion = rsHit.pMaterial->m_attrSurface.colorBase
										* rsHit.attrTexture.argbDiffusion ;
				}
				else
				{
					rgbDiffusion = rsHit.pMaterial->m_attrSurface.colorBase.rgbAdd ;
				}
				rgbDiffusion *=
					(unsigned int) esl_min
						( rsHit.pMaterial->m_attrSurface.nEmission, 0x100 ) ;
				vGI.x += rgbDiffusion.argb.Red ;
				vGI.y += rgbDiffusion.argb.Green ;
				vGI.z += rgbDiffusion.argb.Blue ;
			}
		}
	}
	vGI *= m_gip.fpApplyGI / m_gip.nSamplingCount ;
	fpAO *= m_gip.fpApplyAO / m_gip.nSamplingCount ;
	//
	// 頂点色
	//
	m_pColor[iVertex].rgbMul = SGLPalette(0xFFFFFFFF) * (1.0 - fpAO)
										+ m_gip.rgbAmbient * fpAO ;
	m_pColor[iVertex].rgbMul.argb.Alpha = 0xFF ;
	//
	m_pColor[iVertex].rgbAdd.argb.Red =
		(uint8_t) esl_clampi( esl_roundfi( vGI.x ), 0, 0xFF ) ;
	m_pColor[iVertex].rgbAdd.argb.Green =
		(uint8_t) esl_clampi( esl_roundfi( vGI.y ), 0, 0xFF ) ;
	m_pColor[iVertex].rgbAdd.argb.Blue =
		(uint8_t) esl_clampi( esl_roundfi( vGI.z ), 0, 0xFF ) ;
}

// 指定メッシュの指定UVに関する情報を取得する
//////////////////////////////////////////////////////////////////////////////
size_t S3DModelBuffer::GetMeshPointInfoAtTexturePos
	( SSystem::SArray<S3DModelBuffer::MeshPointInfo>& aResult,
		size_t nResultLimit, size_t iMesh, const S2DVector& vUV ) const
{
	const MeshObject *	pmoMesh = m_arrMeshObj.GetAt( iMesh ) ;
	if ( pmoMesh == NULL )
	{
		return	0 ;
	}
	if ( pmoMesh->m_typeMesh != primitiveTriangle )
	{
		return	0 ;
	}
	const S3DVector4 *	pvVertexBuf = m_bufVertex.GetConstArray() ;
	const S3DVector4 *	pvNormalBuf = m_bufNormal.GetConstArray() ;
	const size_t		nPolyCount = pmoMesh->m_countPolygon ;
	const S3DVector4 *	pvVertex = pvVertexBuf + pmoMesh->m_iVertex ;
	const S3DVector4 *	pvNormal = pvNormalBuf + pmoMesh->m_iNormal ;
	const S2DVector *	pvUV = pmoMesh->m_bufUVMap.GetConstArray() ;
	const uint32_t *	pIndex = pmoMesh->m_bufIndex.GetConstArray() ;
	if ( (pvNormal == NULL) || (pvUV == NULL) || (pIndex == NULL) )
	{
		return	0 ;
	}
	size_t	nMatchPoints = 0 ;
	for ( size_t i = 0; i < nPolyCount; i ++ )
	{
		size_t		iv0 = pIndex[i * 3] ;
		size_t		iv1 = pIndex[i * 3 + 1] ;
		size_t		iv2 = pIndex[i * 3 + 2] ;
		S2DVector	uv0 = pvUV[iv0] ;
		S2DVector	uv1 = pvUV[iv1] ;
		S2DVector	uv2 = pvUV[iv2] ;
		S2DVector	uv10 = uv1 - uv0 ;
		S2DVector	uv21 = uv2 - uv1 ;
		S2DVector	uv02 = uv0 - uv2 ;
		S2DVector	uvd0a = vUV - uv0 ;
		S2DVector	uvd1a = vUV - uv1 ;
		S2DVector	uvd2a = vUV - uv2 ;
		S2DVector	uvd0 = uvd0a - uvd2a * 0.000001f ;
		S2DVector	uvd1 = uvd1a - uvd0a * 0.000001f ;
		S2DVector	uvd2 = uvd2a - uvd1a * 0.000001f ;
		float32_t	z0 = uv10.x * uvd0.y - uv10.y * uvd0.x ;
		float32_t	z1 = uv21.x * uvd1.y - uv21.y * uvd1.x ;
		float32_t	z2 = uv02.x * uvd2.y - uv02.y * uvd2.x ;
		bool		s0 = (z0 >= 0.0f) ;
		bool		s1 = (z1 >= 0.0f) ;
		bool		s2 = (z2 >= 0.0f) ;
		if ( (s0 != s1) || (s0 != s2) )
		{
			// 三角形範囲外
			continue ;
		}
		//
		// uvd = uv10 * x + uv20 * y となる x, y を求める
		//
		S2DVector	uvd = vUV - uv0 ;
		S2DVector	uv20 = uv2 - uv0 ;
		float32_t	d = uv10.x * uv20.y - uv10.y * uv20.x ;
		float32_t	x = (uvd.x * uv20.y - uvd.y * uv20.x) / d ;
		float32_t	y = (uvd.y * uv10.x - uvd.x * uv10.y) / d ;
		//
		// 頂点情報
		//
		MeshPointInfo	mpi ;
		S3DVector	vV1 = pvVertex[iv1] - pvVertex[iv0] ;
		S3DVector	vV2 = pvVertex[iv2] - pvVertex[iv0] ;
		S3DVector	vN1 = pvNormal[iv1] - pvNormal[iv0] ;
		S3DVector	vN2 = pvNormal[iv2] - pvNormal[iv0] ;
		mpi.vPoint = vV1 * x + vV2 * y + pvVertex[iv0] ;
		mpi.vNormal = vN1 * x + vN2 * y + pvNormal[iv0] ;
		mpi.vNormal.Normalize() ;
		mpi.vUV = vUV ;
		S3DTemporaryTextureAxisBuffer::TextureBaseAxis
			( mpi.vAxisX, mpi.vAxisY, vV1, vV2, uv10, uv20 ) ;
		//
		aResult.Add( mpi ) ;
		//
		if ( ++ nMatchPoints >= nResultLimit )
		{
			break ;
		}
	}
	return	nMatchPoints ;
}

SGLError S3DModelBuffer::GetMeshPointInfoAtPolygon
	( S3DModelBuffer::MeshPointInfo& mpiResult,
		size_t iMesh, size_t iPolygon, float32_t uDelta, float32_t vDelta ) const
{
	const MeshObject *	pmoMesh = m_arrMeshObj.GetAt( iMesh ) ;
	if ( pmoMesh == NULL )
	{
		return	sglErrFailed ;
	}
	ESLAssert( iPolygon < pmoMesh->m_countPolygon ) ;
	const S3DVector4 *	pvVertexBuf = m_bufVertex.GetConstArray() ;
	const S3DVector4 *	pvNormalBuf = m_bufNormal.GetConstArray() ;
	const S3DVector4 *	pvVertex = pvVertexBuf + pmoMesh->m_iVertex ;
	const S3DVector4 *	pvNormal = pvNormalBuf + pmoMesh->m_iNormal ;
	const S2DVector *	pvUV = pmoMesh->m_bufUVMap.GetConstArray() ;
	if ( pvUV == NULL )
	{
		return	sglErrFailed ;
	}
	const uint32_t *	pIndex = pmoMesh->m_bufIndex.GetConstArray() ;
	const size_t		iv0 = pIndex[iPolygon * 3] ;
	const size_t		iv1 = pIndex[iPolygon * 3 + 1] ;
	const size_t		iv2 = pIndex[iPolygon * 3 + 2] ;
	const S2DVector		uv0 = pvUV[iv0] ;
	const S2DVector		uv1 = pvUV[iv1] ;
	const S2DVector		uv2 = pvUV[iv2] ;
	const S2DVector		uv10 = uv1 - uv0 ;
	const S2DVector		uv20 = uv2 - uv0 ;
	const S3DVector		vV1 = pvVertex[iv1] - pvVertex[iv0] ;
	const S3DVector		vV2 = pvVertex[iv2] - pvVertex[iv0] ;
	const S3DVector		vN1 = pvNormal[iv1] - pvNormal[iv0] ;
	const S3DVector		vN2 = pvNormal[iv2] - pvNormal[iv0] ;
	mpiResult.vPoint = vV1 * uDelta + vV2 * vDelta + pvVertex[iv0] ;
	mpiResult.vNormal = vN1 * uDelta + vN2 * vDelta + pvNormal[iv0] ;
	mpiResult.vNormal.Normalize() ;
	mpiResult.vUV = uv0 + uv10 * uDelta + uv20 * vDelta ;
	S3DTemporaryTextureAxisBuffer::TextureBaseAxis
		( mpiResult.vAxisX, mpiResult.vAxisY, vV1, vV2, uv10, uv20 ) ;
	//
	return	sglErrSuccess ;
}



//////////////////////////////////////////////////////////////////////////////
// ボーン／疑似物理演算パラメータ（物性）
//////////////////////////////////////////////////////////////////////////////

const S3DModelBoneSpace::PhysMaterial
	S3DModelBoneSpace::PhysMaterial::m_preset
		[S3DModelBoneSpace::PhysMaterial::presetCount] =
{
	PhysMaterial( 0.9999, 0.9, 0.9, 0.99, 1.01, 0.9 ),
	PhysMaterial( 0.99, 0.9, 0.9, 0.9, 1.1, 0.7 ),
	PhysMaterial( 0.99, 0.9, 0.9, 0.9, 1.1, 0.5 ),
	PhysMaterial( 0.99, 0.9, 0.9, 0.7, 1.5, 0.3 ),
	PhysMaterial( 0.997, 0.7, 0.7, 0.95, 1.05, 0.13 ),
	PhysMaterial( 0.999, 0.7, 0.7, 0.95, 1.05, 0.1 ),
	PhysMaterial( 0.9999, 0.8, 0.8, 0.99, 1.01, 0.03 ),
} ;

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
void S3DModelBoneSpace::PhysMaterial::ParseXML( const SXMLDocument& xmlMaterial )
{
	fpAttenuation =
		xmlMaterial.GetAttrRealAs( L"attenuation", fpAttenuation ) ;
	fpShrinkable =
		xmlMaterial.GetAttrRealAs( L"shrinkable", fpShrinkable ) ;
	fpElasticity =
		xmlMaterial.GetAttrRealAs( L"elasticity", fpElasticity ) ;
	fpMinStretch =
		xmlMaterial.GetAttrRealAs( L"min_stretch", fpMinStretch ) ;
	fpMaxStretch =
		xmlMaterial.GetAttrRealAs( L"max_stretch", fpMaxStretch ) ;
	fpHardness =
		xmlMaterial.GetAttrRealAs( L"hardness", fpHardness ) ;
	fpEffect =
		xmlMaterial.GetAttrRealAs( L"effect", fpEffect ) ;
	fpLimitedAngle =
		xmlMaterial.GetAttrRealAs( L"limited_angle", fpLimitedAngle ) ;
	fpCollisionRadius =
		xmlMaterial.GetAttrRealAs( L"collision_radius", fpCollisionRadius ) ;
	fpFrictionalResistance =
		xmlMaterial.GetAttrRealAs( L"frictional_resistance", fpFrictionalResistance ) ;
	nPhysExFlags1 =
		(uint32_t) xmlMaterial.GetAttrHexIntegerAs( L"ex_flags1", nPhysExFlags1 ) ;
	nPhysExFlags2 =
		(uint32_t) xmlMaterial.GetAttrHexIntegerAs( L"ex_flags2", nPhysExFlags2 ) ;
}

void S3DModelBoneSpace::PhysMaterial::FormatXML( SXMLDocument& xmlMaterial ) const
{
	xmlMaterial.SetAttrRealAs( L"attenuation", fpAttenuation ) ;
	xmlMaterial.SetAttrRealAs( L"shrinkable", fpShrinkable ) ;
	xmlMaterial.SetAttrRealAs( L"elasticity", fpElasticity ) ;
	xmlMaterial.SetAttrRealAs( L"min_stretch", fpMinStretch ) ;
	xmlMaterial.SetAttrRealAs( L"max_stretch", fpMaxStretch ) ;
	xmlMaterial.SetAttrRealAs( L"hardness", fpHardness ) ;
	xmlMaterial.SetAttrRealAs( L"effect", fpEffect ) ;
	xmlMaterial.SetAttrRealAs( L"limited_angle", fpLimitedAngle ) ;
	xmlMaterial.SetAttrRealAs( L"collision_radius", fpCollisionRadius ) ;
	xmlMaterial.SetAttrRealAs( L"frictional_resistance", fpFrictionalResistance ) ;
	xmlMaterial.SetAttrHexIntegerAs( L"ex_flags1", nPhysExFlags1 ) ;
	xmlMaterial.SetAttrHexIntegerAs( L"ex_flags2", nPhysExFlags2 ) ;
}



//////////////////////////////////////////////////////////////////////////////
// ボーン
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DModelBoneSpace, Space )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DModelBoneSpace::S3DModelBoneSpace( void )
	: m_mat4Original( 1, 1, 1, 1 )
{
	m_flagsBone = 0 ;
	//
	m_paramIK.nIKFlags = 0 ;
	m_paramIK.fpWeight = 1.0f ;
	m_paramIK.degMinBent = 0.0f ;
	m_paramIK.degMaxBent = 180.0f ;
	//
	m_fpPhysBelnd = 1.0 ;
	m_fpLastPhysBelnd = 1.0 ;
	//
	m_pModel = NULL ;
//	m_iTargetVertex = 0 ;
//	m_iTargetNormal = 0 ;
//	m_nTargetCount = 0 ;
	//
	#if	defined(__DEBUG__)
	m_flagDebug = false ;
	#endif

	ResetPhysicsParameter() ;
}

S3DModelBoneSpace::S3DModelBoneSpace( const S3DModelBoneSpace& bone )
	: m_flagsBone( bone.m_flagsBone ),
		m_mat4Original( bone.m_mat4Original ),
		m_vBoneHandle( bone.m_vBoneHandle ),
		m_paramIK( bone.m_paramIK ),
		m_physMaterial( bone.m_physMaterial ),
		m_physCurrent( bone.m_physCurrent ),
		m_fpPhysBelnd( bone.m_fpPhysBelnd ),
		m_fpLastPhysBelnd( bone.m_fpLastPhysBelnd ),
		m_pModel( bone.m_pModel ),
		m_wmbWeight( bone.m_wmbWeight ),
//		m_bufWeight( bone.m_bufWeight ),
//		m_iTargetVertex( bone.m_iTargetVertex ),
//		m_iTargetNormal( bone.m_iTargetNormal ),
//		m_nTargetCount( bone.m_nTargetCount ),
		m_arrRefMesh( bone.m_arrRefMesh )
{
	#if	defined(__DEBUG__)
	m_flagDebug = false ;
	#endif
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DModelBoneSpace::~S3DModelBoneSpace( void )
{
}

// デバッグフラグ
//////////////////////////////////////////////////////////////////////////////
void S3DModelBoneSpace::SetDebugFlag( bool flagDebug )
{
	#if	defined(__DEBUG__)
	m_flagDebug = flagDebug ;
	#endif
}

// ボーンの変換行列計算
//////////////////////////////////////////////////////////////////////////////
void S3DModelBoneSpace::CalcBoneTransformation
	( S3DDMatrix& matrix, S3DDVector& translate ) const
{
	S3DDVector	vPos = m_vCenter + GetBoneOffset() ;
	S3DDMatrix	matBone = m_matTransformation ;
	S3DModelBoneSpace *
		pBone = ESLTypeCast<S3DModelBoneSpace>( m_refParent.GetReference() ) ;
	while ( pBone != NULL )
	{
		matBone = pBone->m_matTransformation * matBone ;
		vPos = pBone->m_matTransformation * vPos
					+ pBone->m_vCenter + pBone->GetBoneOffset() ;
		//
		pBone = ESLTypeCast<S3DModelBoneSpace>
						( pBone->m_refParent.GetReference() ) ;
	}
	matrix = matBone ;
	translate = vPos ;
}

const S3DDVector&
	S3DModelBoneSpace::CalcBoneBasePosition( S3DDVector& pos ) const
{
	S3DModelBoneSpace *	pBone =
			ESLTypeCast<S3DModelBoneSpace>( m_refParent.GetReference() ) ;
	S3DDVector	vPos = m_vCenter ;
	while ( pBone != NULL )
	{
		vPos += pBone->m_vCenter + pBone->GetBoneOffset() ;
		pBone = ESLTypeCast<S3DModelBoneSpace>
						( pBone->m_refParent.GetReference() ) ;
	}
	pos = vPos ;
	return	pos ;
}

void S3DModelBoneSpace::CalcSubBoneTransformation
	( S3DDMatrix& matSub, S3DDVector& vSub,
		const S3DDMatrix& matCur, S3DDVector& vCur,
		S3DModelBoneSpace * pSubBone ) const
{
	S3DDVector	vPos = pSubBone->m_vCenter + pSubBone->GetBoneOffset() ;
	S3DDMatrix	matBone = pSubBone->m_matTransformation ;
	//
	S3DModelBoneSpace *
		pBone = ESLTypeCast<S3DModelBoneSpace>
						( pSubBone->m_refParent.GetReference() ) ;
	while ( (pBone != this) && (pBone != NULL) )
	{
		matBone = pBone->m_matTransformation * matBone ;
		vPos = pBone->m_matTransformation * vPos
					+ pBone->m_vCenter + pBone->GetBoneOffset() ;
		//
		pBone = ESLTypeCast<S3DModelBoneSpace>
						( pBone->m_refParent.GetReference() ) ;
	}
	if ( pBone == this )
	{
		matSub = matCur * matBone ;
		vSub = matCur * vPos + vCur ;
	}
	else
	{
		matSub = matBone ;
		vSub = vCur ;
	}
}

// ボーンの基準座標（回転とオフセットを含まない初期状態のグローバル座標）
//////////////////////////////////////////////////////////////////////////////
S3DDVector S3DModelBoneSpace::CalcBoneNormalizedPosition( void ) const
{
	S3DModelBoneSpace *	pBone =
			ESLTypeCast<S3DModelBoneSpace>( m_refParent.GetReference() ) ;
	S3DDVector	vPos = m_vCenter ;
	while ( pBone != NULL )
	{
		vPos += pBone->m_vCenter ;
		pBone = ESLTypeCast<S3DModelBoneSpace>
						( pBone->m_refParent.GetReference() ) ;
	}
	return	vPos ;
}

// グローバル空間変換行列を計算
//////////////////////////////////////////////////////////////////////////////
void S3DModelBoneSpace::CalcGlobalTransformation
				( S3DDMatrix& matrix, S3DDVector& pos ) const
{
	CalcBoneTransformation( matrix, pos ) ;
	//
	if ( m_pModel != NULL )
	{
		S3DModelBoneSpace&	boneLocal = m_pModel->GetLocalSpaceBone() ;
		matrix = boneLocal.m_matTransformation * matrix ;
		pos = boneLocal.m_matTransformation * pos + boneLocal.m_vCenter ;
		//
		S3DScene::Item *	pRelItem = m_pModel->GetRelationItem() ;
		if ( pRelItem != NULL )
		{
			S3DDMatrix	matItem ;
			S3DDVector	vItem ;
			pRelItem->CalcGlobalTransformation( matItem, vItem ) ;
			//
			matrix = matItem * matrix ;
			pos = matItem * pos + vItem ;
		}
	}
}

const S3DDVector&
		S3DModelBoneSpace::CalcGlobalPosition( S3DDVector& pos ) const
{
	S3DDMatrix	matTemp ;
	CalcGlobalTransformation( matTemp, pos ) ;
	return	pos ;
}

// 親ボーン
//////////////////////////////////////////////////////////////////////////////
S3DModelBoneSpace * S3DModelBoneSpace::GetParentBone( void ) const
{
	return	ESLTypeCast<S3DModelBoneSpace>( m_refParent.GetReference() ) ;
}

// 親ボーン判定
//////////////////////////////////////////////////////////////////////////////
bool S3DModelBoneSpace::IsParentBoneOf( S3DModelBoneSpace * pChild ) const
{
	if ( (this == pChild) || (pChild == nullptr) )
	{
		return	false ;
	}
	S3DModelBoneSpace *	pParent = pChild->GetParentBone() ;
	while ( pParent != nullptr )
	{
		if ( this == pParent )
		{
			return	true ;
		}
		pParent = pParent->GetParentBone() ;
	}
	return	false ;
}

// このボーンと全ての子ボーンの変更フラグをクリアする
//////////////////////////////////////////////////////////////////////////////
void S3DModelBoneSpace::ClearAllModifiedFlags( void )
{
	m_flagsModified = 0 ;

	size_t	nChildCount = GetChildrenCount() ;
	for ( size_t i = 0; i < nChildCount; i ++ )
	{
		S3DModelBoneSpace *	pChild =
			ESLTypeCast<S3DModelBoneSpace>( GetChildAt( i ) ) ;
		if ( pChild != nullptr )
		{
			pChild->ClearAllModifiedFlags() ;
		}
	}
}

// ボーンフラグ
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DModelBoneSpace::GetBoneFlags( void ) const
{
	return	m_flagsBone ;
}

void S3DModelBoneSpace::SetBoneFlags( uint32_t nFlags )
{
	m_flagsBone = nFlags ;
}

// 元モデルの行列（座標空間変換なしのグローバル変換行列）
//////////////////////////////////////////////////////////////////////////////
const S4DMatrix& S3DModelBoneSpace::GetOriginalBoneMatrix( void ) const
{
	return	m_mat4Original ;
}

void S3DModelBoneSpace::SetOriginalBoneMatrix( const S4DMatrix& matOrg )
{
	m_mat4Original = matOrg ;
}

// 元モデルと同じ座標空間のグローバル変換行列を反映させる
// （親ボーンから順に設定する＆flagHaveOrgMatrix 必須）
//////////////////////////////////////////////////////////////////////////////
void S3DModelBoneSpace::ApplyModifiedOriginalBoneMatrix
		( const S4DMatrix& matGlobal, const S4DMatrix& matCvtSpace )
{
// 頂点バッファ上の座標はノード行列であらかじめ乗算されている。
// 元頂点:v, 変換後頂点:v', 元ノード行列:No, ノード移動行列:Nt が定数として与えられ
// ノード（ボーン）の動的な行列（回転と行列）が Br の時の、変形後頂点座標 vx は
//   v' = No * v
//   vx = (No^-1) * Br * (Nt^-1 * No) * v'
// で計算される。
// ボーンは正規化されているので無変型時ボーン行列 Br0 = Nt である。
// EntisGLS4 のボーンで上記式の (No^-1), (Nt^-1 * No) はボーン固有の定数値で
// Br は変数としてボーンのグローバル空間変換で与えられる。
//
// Rel = (No^-1), IM = (Nt^-1 * No) として、ボーンによる座標計算は
//   vx = Rel * Br * IM * v'
// で行う。
//
// スケルトンの場合、メッシュ=スケルトン結合用逆行列を IB，
// スケルトン行列を Sk とした場合、IM は
//   IM = Nt^-1 * Sk * IB
// で与えられる。
//
// 新しいスケルトン行列 Sx が与えられたとき、
// vx = No^-1 * Sx * IB * No * v となる vx = Rel * Br * IM * v' の Br を求めたい。
// Rel = (No^-1), IM = Nt^-1 * Sk * IB なので、
//   No^-1 * Sx * IB * No * v = Rel * Br * IM * No * v
// から
//   Sx * IB = Br * IM
// となり、
//   Br = Sx * IB * IM^-1
//      = Sx * IB * (Nt^-1 * Sk * IB)^-1
//      = Sx * IB * (IB^-1 * Sk * Nt)
//      = Sx * Sk * Nt
// ここで Sk は変換元スケルトンの初期行列であり、Nt は正規化時のボーン座標の移動行列である。
//
// glTF, OpenXR の座標空間と EntisGLS4 の座標空間を変換する行列 CS は
// (1, -1, -1) を対角にする行列である。
// 相互の回転行列を変換する場合には R' = CS * R * CS^-1 で得られる。
// 上記ボーン座標変換式を glTF -> EntisGLS4 座標空間変換を含んだ場合、
//   Rel = CS * No^-1 * CS^-1
//   IM = Nt^-1 * CS * Sk * IB * CS^-1
// となる（Nt は EntisGLS4 の座標空間に変換後の移動行列とする）
//
// vx = CS * No^-1 * Sx * IB * No * v となる vx = Rel * Br * IM * v' の Br を求めたい。
//    CS * No^-1 * Sx * IB * No * v = Rel * Br * IM * v'
//    CS * No^-1 * Sx * IB * No * v = Rel * Br * IM * (CS * No * v)
//    CS * No^-1 * Sx * IB          = Rel * Br * IM * CS
// Rel, IM を展開
//    CS * No^-1 * Sx * IB          = CS * No^-1 * CS^-1 * Br * Nt^-1 * CS * Sk * IB * CS^-1 * CS
//                 Sx               =              CS^-1 * Br * Nt^-1 * CS * Sk
// から
//    Br = CS * Sx * Sk^-1 * CS^-1 * Nt

	S3DVector	vNormalPos = CalcBoneNormalizedPosition() ;
	S4DMatrix	mat4NormalPos( 1, 1, 1, 1 ) ;
	mat4NormalPos.SetTranslation( vNormalPos ) ;

	S4DMatrix	matOrg( 1, 1, 1, 1 ) ;
	if ( GetBoneFlags() & flagHaveOrgMatrix )
	{
		matOrg = GetOriginalBoneMatrix() ;
	}
	else
	{
		matOrg.SetTranslation
			( matCvtSpace.GetMatrix3().Inverse() * vNormalPos ) ;
	}
	//
	//
	S4DMatrix	mat4Bone = matCvtSpace
							* matGlobal
							* matOrg.Inverse()
							* matCvtSpace.Inverse()
							* mat4NormalPos ;

	ApplyGlobalTransformation( mat4Bone ) ;
}

// グローバル変換行列をローカルに変換して設定する
//////////////////////////////////////////////////////////////////////////////
void S3DModelBoneSpace::ApplyGlobalTransformation( const S4DMatrix& matGlobal )
{
	S3DDMatrix	matdBone = matGlobal.GetMatrix3() ;
	S3DDVector	vdBone = matGlobal.GetTranslation() ;
	ApplyGlobalTransformation( matdBone, vdBone ) ;
}

void S3DModelBoneSpace::ApplyGlobalTransformation
	( const S3DDMatrix& matGlobal, const S3DDVector& vGlobalPos )
{
	//
	// 親空間を計算する
	//
	S3DDMatrix	matdParent( 1, 1, 1 ) ;
	S3DDVector	vdParent( 0, 0, 0 ) ;
	S3DModelBoneSpace *	pParent = GetParentBone() ;
	if ( pParent != nullptr )
	{
		pParent->CalcBoneTransformation( matdParent, vdParent ) ;
	}
	//
	// ローカル空間へ変換する
	//
	S3DDVector	vBonePos ;
	S3DDMatrix	matdIParent = matdParent.Inverse() ;
	S3DDMatrix	matdLocal = matdIParent * matGlobal ;
	S3DDVector	vdLocal = matdIParent * (vGlobalPos - vdParent) ;
	S3DDVector	vdOffset = vdLocal - GetLocalSpacePosition(vBonePos) ;
	//
	SetLocalTransformation( matdLocal ) ;
	SetBoneOffset( vdOffset ) ;
}

// ボーン内平行移動
//////////////////////////////////////////////////////////////////////////////
const S3DDVector& S3DModelBoneSpace::GetBoneOffset( void ) const
{
	return	m_vBoneOffset ;
}

void S3DModelBoneSpace::SetBoneOffset( const S3DDVector& vOffset )
{
	m_vBoneOffset = vOffset ;
}

// ボーンハンドル
//////////////////////////////////////////////////////////////////////////////
const S3DDVector& S3DModelBoneSpace::GetBoneHandle( void ) const
{
	return	m_vBoneHandle ;
}

void S3DModelBoneSpace::SetBoneHandle( const S3DDVector& vHandle )
{
	m_vBoneHandle = vHandle ;
}

// IK 用パラメータ
//////////////////////////////////////////////////////////////////////////////
const S3DModelBoneSpace::IKParameter& S3DModelBoneSpace::GetIKParameter( void ) const
{
	return	m_paramIK ;
}

void S3DModelBoneSpace::SetIKParameter( const S3DModelBoneSpace::IKParameter& param )
{
	m_paramIK = param ;
}

// 物性（物理演算）
//////////////////////////////////////////////////////////////////////////////
const S3DModelBoneSpace::PhysMaterial&
	S3DModelBoneSpace::GetBonePhysicalMaterial( void ) const
{
	return	m_physMaterial ;
}

void S3DModelBoneSpace::SetBonePhysicalMaterial
			( const S3DModelBoneSpace::PhysMaterial& phyMaterial )
{
	m_physMaterial = phyMaterial ;
}

// 物理属性パレット名
//////////////////////////////////////////////////////////////////////////////
const SSystem::SString& S3DModelBoneSpace::GetBonePhysicalMaterialID( void ) const
{
	return	m_idPhysMaterial ;
}

void S3DModelBoneSpace::SetBonePhysicalMaterialID( const wchar_t * pwszID )
{
	m_idPhysMaterial = pwszID ;
}

// このボーンの物性（物理演算）を子ボーンにも設定する
//////////////////////////////////////////////////////////////////////////////
void S3DModelBoneSpace::CopyPhysicalMaterialToAllChildren( bool fWithMaterialID )
{
	uint32_t	nPhysFlag = GetBoneFlags() & flagBonePhysics ;
	size_t	nChildCount = GetChildrenCount() ;
	for ( size_t i = 0; i < nChildCount; i ++ )
	{
		S3DModelBoneSpace *	pChild =
			ESLTypeCast<S3DModelBoneSpace>( GetChildAt( i ) ) ;
		if ( pChild != NULL )
		{
			pChild->SetBoneFlags
				( (pChild->GetBoneFlags() & ~flagBonePhysics) | nPhysFlag ) ;
			pChild->SetBonePhysicalMaterial( GetBonePhysicalMaterial() ) ;
			//
			if ( fWithMaterialID && !GetBonePhysicalMaterialID().IsEmpty() )
			{
				pChild->SetBonePhysicalMaterialID( GetBonePhysicalMaterialID() ) ;
			}
			//
			pChild->CopyPhysicalMaterialToAllChildren( fWithMaterialID ) ;
		}
	}
}

// 物理演算・被影響ボーン
//////////////////////////////////////////////////////////////////////////////
size_t S3DModelBoneSpace::GetEffectivePhysBoneCount( void ) const
{
	return	m_aEffectiveBones.GetLength() ;
}

S3DModelBoneSpace::EffectiveBoneEntry *
	S3DModelBoneSpace::GettEffectivePhysBoneAt( size_t i ) const
{
	return	m_aEffectiveBones.GetAt( i ) ;
}

size_t S3DModelBoneSpace::AddEffectivePhysBone
		( S3DModelBoneSpace::EffectiveBoneEntry * pebe )
{
	return	m_aEffectiveBones.Add( pebe ) ;
}

void S3DModelBoneSpace::InsertEffectivePhysBoneAt
		( size_t i, S3DModelBoneSpace::EffectiveBoneEntry * pebe )
{
	m_aEffectiveBones.InsertAt( i, pebe ) ;
}

ssize_t S3DModelBoneSpace::FindEffectivePhysBone( const wchar_t * pwszBoneID ) const
{
	for ( size_t i = 0; i < m_aEffectiveBones.GetLength(); i ++ )
	{
		EffectiveBoneEntry *	pebe = m_aEffectiveBones.GetAt( i ) ;
		if ( (pebe != NULL) && (pebe->m_strBoneID == pwszBoneID) )
		{
			return	(ssize_t) i ;
		}
	}
	return	-1 ;
}

ssize_t S3DModelBoneSpace::FindEffectivePhysBoneOf( EffectiveBoneEntry * pebe ) const
{
	return	m_aEffectiveBones.FindPtr( pebe ) ;
}

void S3DModelBoneSpace::RemoveEffectivePhysBoneAt( size_t i )
{
	m_aEffectiveBones.RemoveAt( i ) ;
}

void S3DModelBoneSpace::RemoveAllEffectivePhysBones( void )
{
	m_aEffectiveBones.RemoveAll() ;
}

// 物理演算の内部パラメータをリセット
//////////////////////////////////////////////////////////////////////////////
void S3DModelBoneSpace::ResetPhysicsParameter( void )
{
	//
	S3DModelBoneSpace *	pParent =
			ESLTypeCast<S3DModelBoneSpace>( GetParentSpace() ) ;
	if ( pParent == NULL )
	{
		pParent = this ;
	}
	S3DDMatrix	matBone ;
	S3DDVector	vBone ;
	pParent->CalcBoneTransformation( matBone, vBone ) ;
	//
	ResetPhysicsParameter( matBone, vBone ) ;
}

void S3DModelBoneSpace::ResetPhysicsParameter
		( const S3DDMatrix& matBone, const S3DDVector& vBone )
{
	#if	defined(__DEBUG__)
	if ( m_flagDebug )
	{
		ESLTrace( "ResetPhysicsParameter(%08X)\n", (ulong_ptr_t) this ) ;
	}
	#endif

	m_physCurrent.vPos = m_vBoneHandle ;
	m_physCurrent.vSpeed.x = 0 ;
	m_physCurrent.vSpeed.y = 0 ;
	m_physCurrent.vSpeed.z = 0 ;
	m_physCurrent.vLastExSpeed.x = 0 ;
	m_physCurrent.vLastExSpeed.y = 0 ;
	m_physCurrent.vLastExSpeed.z = 0 ;
	m_physCurrent.vExSpeedLPF.x = 0 ;
	m_physCurrent.vExSpeedLPF.y = 0 ;
	m_physCurrent.vExSpeedLPF.z = 0 ;
	m_physCurrent.matLastSpace = matBone ;
	m_physCurrent.vLastSpace = vBone ;
	m_physCurrent.nHitCollider = 0 ;
	m_physCurrent.vHitNormal.x = 0 ;
	m_physCurrent.vHitNormal.y = 0 ;
	m_physCurrent.vHitNormal.z = 0 ;
	//
	S3DDMatrix	matSubBone = matBone * m_matTransformation ;
	S3DDVector	vSubBonePos = matBone * (m_vCenter + GetBoneOffset()) + vBone ;
	Space *const*	ppChildren = m_children.GetConstArray() ;
	size_t			countChildren = m_children.GetLength() ;
	for ( size_t i = 0; i < countChildren; i ++ )
	{
		S3DModelBoneSpace *	pChild =
			ESLTypeCast<S3DModelBoneSpace>( ppChildren[i] ) ;
		if ( pChild != NULL )
		{
			pChild->ResetPhysicsParameter( matSubBone, vSubBonePos ) ;
		}
	}
}

// 物理演算の内部パラメータ
//////////////////////////////////////////////////////////////////////////////
const S3DModelBoneSpace::PhysVertex&
		S3DModelBoneSpace::GetPhysicsParamater( void ) const
{
	return	m_physCurrent ;
}

void S3DModelBoneSpace::SetPhysicsParamater
		( const S3DModelBoneSpace::PhysVertex& physVertex )
{
	m_physCurrent = physVertex ;
}

// 物理演算の適用度
//////////////////////////////////////////////////////////////////////////////
double S3DModelBoneSpace::GetPhysicsBlendWeight( void ) const
{
	return	m_fpPhysBelnd ;
}

double S3DModelBoneSpace::GetLastPhysicsBlendWeight( void ) const
{
	return	m_fpLastPhysBelnd ;
}

void S3DModelBoneSpace::SetPhysicsBlendWeight( double fpBlend )
{
	m_fpPhysBelnd = fpBlend ;
}

// 物理演算のハンドル座標
//////////////////////////////////////////////////////////////////////////////
const S3DDVector& S3DModelBoneSpace::GetPhysicsHandlePosition( void ) const
{
	return	m_physCurrent.vPos ;
}

void S3DModelBoneSpace::SetPhysicsHandlePosition( const S3DDVector& vPos )
{
	m_physCurrent.vPos = vPos ;
}

// 物理演算適用度をリセット（フレームポーズ適用前に一度実行）
//////////////////////////////////////////////////////////////////////////////
void S3DModelBoneSpace::ResetPhysicsBlendWeight( void )
{
	m_fpLastPhysBelnd = m_fpPhysBelnd ;
	m_fpPhysBelnd = 1.0 ;
	//
	Space *const*	ppChildren = m_children.GetConstArray() ;
	size_t			countChildren = m_children.GetLength() ;
	for ( size_t i = 0; i < countChildren; i ++ )
	{
		S3DModelBoneSpace *	pChild =
			ESLTypeCast<S3DModelBoneSpace>( ppChildren[i] ) ;
		if ( pChild != NULL )
		{
			pChild->ResetPhysicsBlendWeight() ;
		}
	}
}

// 物理演算
//////////////////////////////////////////////////////////////////////////////
void S3DModelBoneSpace::CalculatePhysics
	( const S3DModelBoneSpace::PhysExogenous& exog,
		double secPast, const S3DModelBoneSpace::ColliderParam * pColParam )
{
	S3DDMatrix	matBone( 1, 1, 1 ) ;
	S3DDVector	vBone( 0, 0, 0 ) ;
	CalculateSubPhysics
		( exog, m_physMaterial, matBone, vBone, secPast, pColParam ) ;
}

void S3DModelBoneSpace::CalculateSubPhysics
	( const S3DModelBoneSpace::PhysExogenous& exog,
		const S3DModelBoneSpace::PhysMaterial& mtrl,
		const S3DDMatrix& matBone,
		const S3DDVector& vBone,
		double secPast,
		const S3DModelBoneSpace::ColliderParam * pColParam )
{
	if ( secPast <= 1.0e-7 )
	{
		ReflectPhysics( matBone ) ;
		return ;
	}
	//
	// 被影響ボーン処理
	//
	S3DDVector	vBoneHandle = m_vBoneHandle ;
	if ( (m_flagsBone & flagBonePhysics)
		&& (m_aEffectiveBones.GetLength() > 0) && (m_pModel != NULL) )
	{
		S3DDVector		vTempBoneHandle = matBone * m_vBoneHandle ;
		S3DDQuaternion	qEffRotation( 1, 0, 0, 0 ) ;
		bool			flagRotation = false ;
		for ( size_t i = 0; i < m_aEffectiveBones.GetLength(); i ++ )
		{
			EffectiveBoneEntry *	pebe = m_aEffectiveBones.GetAt( i ) ;
			if ( pebe == nullptr )
			{
				continue ;
			}
			S3DModelBoneSpace *	pEffBone =
				m_pModel->GetBonePropertyAs( pebe->m_strBoneID ) ;
			if ( pEffBone == nullptr )
			{
				continue ;
			}
			S3DModelBoneSpace *	pEffParent = pEffBone->GetParentBone() ;
			if ( pEffParent == nullptr )
			{
				continue ;
			}
			S3DDMatrix	matdEffBone ;
			S3DDVector	vdEffBone ;
			pEffParent->CalcBoneTransformation( matdEffBone, vdEffBone ) ;
			//
			S3DDVector	vEffBone0 = matdEffBone * pEffBone->GetBoneHandle() ;
			S3DDVector	vEffBone1 =
				matdEffBone * (pEffBone->m_matTransformation * pEffBone->GetBoneHandle()) ;
			//
			vEffBone1 = vEffBone0 + (vEffBone1 - vEffBone0) * pebe->m_fpWeight ;
			//
			S3DDMatrix	matdEffRot( 1, 1, 1 ) ;
			matdEffRot.VectorRotationOf( vEffBone0, vEffBone1 ) ;
			//
			qEffRotation *= S3DDQuaternion(matdEffRot) ;
			flagRotation = true ;
		}
		if ( flagRotation )
		{
			S3DDMatrix	matEffRotation ;
			qEffRotation.Normalize() ;
			qEffRotation.ToMatrix( matEffRotation ) ;
			//
			S3DDMatrix	matLocalEffRot= matBone.Inverse() * matEffRotation * matBone ;
			vBoneHandle = matLocalEffRot * m_vBoneHandle ;
		}
	}
	//
	// ボーン物理演算
	//
	if ( (m_flagsBone & (flagBonePhysics | flagFreezePhysics)) == flagBonePhysics )
	{
		PhysVertex *		pParentPhys = nullptr ;
		S3DModelBoneSpace *	pParent =
			ESLTypeCast<S3DModelBoneSpace>( m_refParent.GetReference() ) ;
		if ( pParent != NULL )
		{
			pParentPhys = &(pParent->m_physCurrent) ;
		}
		const S3DDMatrix&	matLocal = m_matTransformation ;
		const S3DDVector	vLocalMove = m_vCenter + GetBoneOffset() ;
		if ( CalculatePhysicsVertex
			( m_physCurrent, pParentPhys,
				exog, mtrl, matBone, vBone,
				matLocal, vLocalMove,
				m_flagsBone, vBoneHandle, secPast, pColParam, this ) )
		{
			ReflectBonePhysics( matBone ) ;
		}
	}
	m_physCurrent.matLastSpace = matBone ;
	m_physCurrent.vLastSpace = vBone ;
	//
	// サブボーン処理
	//
	Space *const*	ppChildren = m_children.GetConstArray() ;
	size_t			countChildren = m_children.GetLength() ;
	S3DDMatrix		matSubBone = matBone * m_matTransformation ;
	S3DDVector		vSubBonePos = matBone * (m_vCenter + GetBoneOffset()) + vBone ;
	for ( size_t i = 0; i < countChildren; i ++ )
	{
		S3DModelBoneSpace *	pChild =
			ESLTypeCast<S3DModelBoneSpace>( ppChildren[i] ) ;
		if ( pChild != NULL )
		{
			pChild->CalculateSubPhysics
				( exog, pChild->m_physMaterial,
						matSubBone, vSubBonePos, secPast, pColParam ) ;
		}
	}
}

void S3DModelBoneSpace::ResetLastSpaceOfChildrenPhysics
	( const S3DDMatrix& matBone, const S3DDVector& vBone )
{
	S3DDMatrix	matSubBone = matBone * m_matTransformation ;
	S3DDVector	vSubBonePos = matBone * m_vCenter + vBone
									+ matSubBone * GetBoneOffset() ;
	Space *const*	ppChildren = m_children.GetConstArray() ;
	size_t			countChildren = m_children.GetLength() ;
	for ( size_t i = 0; i < countChildren; i ++ )
	{
		S3DModelBoneSpace *	pChild =
			ESLTypeCast<S3DModelBoneSpace>( ppChildren[i] ) ;
		if ( pChild != NULL )
		{
			pChild->m_physCurrent.matLastSpace = matSubBone ;
			pChild->m_physCurrent.vLastSpace = vSubBonePos ;
			//
			pChild->ResetLastSpaceOfChildrenPhysics( matSubBone, vSubBonePos ) ;
		}
	}
}

// 物理演算（PhysVertex インスタンス）
//////////////////////////////////////////////////////////////////////////////
bool S3DModelBoneSpace::CalculatePhysicsVertex
	( S3DModelBoneSpace::PhysVertex& physVertex,
		S3DModelBoneSpace::PhysVertex * pParentPhys,
		const PhysExogenous& exog,
		const PhysMaterial& mtrl,
		const S3DDMatrix& matBone,
		const S3DDVector& vBone,
		const S3DDMatrix& matLocal,
		const S3DDVector& vLocalMove,
		uint32_t nBoneFlags,
		const S3DDVector& vBoneHandle,
		double secPast, const ColliderParam * pColParam,
		const S3DModelBoneSpace * pOtherParams )
{
	if ( (nBoneFlags & (flagBonePhysics | flagFreezePhysics)) != flagBonePhysics )
	{
		return	false ;
	}
	double	framesPast = secPast * 60.0 ;
	//
	// 変換行列計算
	//
	S3DDMatrix	matSubBone = matBone * matLocal ;
	S3DDVector	vSubBonePos = matBone * vLocalMove + vBone ;
	S3DDMatrix	matCurBone = exog.matSpace * matBone ;
	S3DDVector	vCurBonePos = exog.matSpace * vBone + exog.vSpace ;
	S3DDMatrix	matLastBone = exog.matLastSpace * physVertex.matLastSpace ;
	S3DDVector	vLastBonePos = exog.matLastSpace * physVertex.vLastSpace + exog.vLastSpace ;
	physVertex.matLastSpace = matBone ;
	physVertex.vLastSpace = vBone ;
	//
	S3DDMatrix	matIBone, matISubBone ;
	matIBone.InverseOf( matBone ) ;
	matISubBone.InverseOf( matSubBone ) ;
	//
	// 外因（回転変換）変位
	//
	S3DDVector	vDeltaBone0 = vLastBonePos ;
	S3DDVector	vDeltaBone1 = vCurBonePos ;
	vDeltaBone0 -= vDeltaBone1 ;
	//
	S3DDVector	vExSpeed = vDeltaBone0 * (1.0 / secPast) ;
	S3DDVector	vExSpeedLPF = vExSpeed * 0.125 + physVertex.vExSpeedLPF * 0.875 ;
	physVertex.vExSpeedLPF = vExSpeedLPF ;
	//
	S3DDVector	vAccel = vExSpeedLPF - physVertex.vLastExSpeed ;
	physVertex.vLastExSpeed = vExSpeedLPF ;
	//
	vAccel += exog.vAcceleration * secPast ;
	//
	physVertex.vSpeed += vAccel * mtrl.fpEffect ;
	//
	#if	defined(__DEBUG__)
	if ( pOtherParams && pOtherParams->m_flagDebug )
	{
		ESLTrace( "bone phys(%08X): pos=(%f,%f,%f), speed=%f ex_speed=%f (/ %f[sec])\n",
				(ulong_ptr_t) pOtherParams,
				physVertex.vPos.x, physVertex.vPos.y, physVertex.vPos.z,
				physVertex.vSpeed.Absolute(), vExSpeedLPF.Absolute(), secPast ) ;
	}
	#endif
	//
	// 内因（伸縮）変位
	//
	S3DDVector	vDeltaBone( 0, 0, 0 ) ;
	double		fpBoneLength = vBoneHandle.Absolute() ;
	double		fpCurLength = physVertex.vPos.Absolute() ;
	S3DDVector	vAbsCurBone ;
	double		fpOrthSpeed ;
	if ( fpCurLength > 0.0 )
	{
		vAbsCurBone = physVertex.vPos * (1.0 / fpCurLength) ;
	}
	else
	{
		vAbsCurBone = vBoneHandle ;
		vAbsCurBone.Normalize() ;
	}
	S3DDVector	vLocalSpeed = matISubBone * physVertex.vSpeed ;
	fpOrthSpeed = (vAbsCurBone | vLocalSpeed) ;
	//
	if ( fpCurLength < fpBoneLength )
	{
		double	fpShrinkable = mtrl.fpShrinkable * framesPast ;
		double	fpMinLength = fpBoneLength * mtrl.fpMinStretch ;
		if ( fpCurLength > fpMinLength )
		{
			vDeltaBone =
				vAbsCurBone
					* ((fpBoneLength - fpCurLength) * fpShrinkable) ;
		}
		else
		{
			vDeltaBone =
				vAbsCurBone
					* ((fpBoneLength - fpMinLength) * fpShrinkable) ;
			physVertex.vPos = vAbsCurBone * fpMinLength ;
			ESLAssert( !physVertex.vPos.IsNaN() ) ;
		}
	}
	else
	{
		double	fpElasticity = mtrl.fpElasticity * framesPast ;
		double	fpMaxLength = fpBoneLength * mtrl.fpMaxStretch ;
		if ( fpCurLength < fpMaxLength )
		{
			vDeltaBone =
				vAbsCurBone
					* ((fpBoneLength - fpCurLength) * fpElasticity) ;
		}
		else
		{
			vDeltaBone =
				vAbsCurBone
					* ((fpBoneLength - fpMaxLength) * fpElasticity) ;
			physVertex.vPos = vAbsCurBone * fpMaxLength ;
			ESLAssert( !physVertex.vPos.IsNaN() ) ;
		}
	}
	vLocalSpeed += vDeltaBone ;
	//
/*	if ( pParentPhys != nullptr )
	{
		S3DDVector	vDeltaSpeed = matBone * vDeltaBone * mtrl.fpEffect * 0.5 ;
		if ( pParentPhys->nHitCollider > 0 )
		{
			double	fpResistance =
						pow( mtrl.fpFrictionalResistance, framesPast ) ;
			vDeltaSpeed *= fpResistance ;
		}
		pParentPhys->vSpeed -= vDeltaSpeed ;
	}
*/	//
	// 内因（曲がり）変位
	//
	S3DDVector	vCurDelta = (physVertex.vPos - vBoneHandle) * (1.0 / secPast) ;
	double	fpHardness = esl_fclamp( mtrl.fpHardness, 0.0, 0.99 ) * framesPast ;
	fpHardness *= pow( 1.0 - mtrl.fpAttenuation, secPast ) ;
	vLocalSpeed -= vCurDelta * fpHardness ;
	if ( physVertex.nHitCollider > 0 )
	{
		double	fpResistance =
					pow( mtrl.fpFrictionalResistance, framesPast ) ;
		physVertex.vSpeed *= fpResistance ;
	}
	physVertex.vSpeed = matSubBone * vLocalSpeed ;
	//
	// 速度をボーンハンドルへ反映／行列計算
	//
	S3DDVector	vStream = exog.vStream ;
	double		fpAttenuationDelta =
					pow( 1.0 - mtrl.fpAttenuation, secPast ) ;
	physVertex.vSpeed -= vStream ;
	physVertex.vSpeed *= fpAttenuationDelta ;
	physVertex.vSpeed += vStream ;
	physVertex.vPos += matISubBone * (physVertex.vSpeed * secPast) ;
	ESLAssert( !physVertex.vPos.IsNaN() ) ;
	//
	fpBoneLength = vBoneHandle.Absolute() ;
	fpCurLength = physVertex.vPos.Absolute() ;
	if ( fpCurLength < fpBoneLength * mtrl.fpMinStretch )
	{
		physVertex.vPos.Normalize() ;
		physVertex.vPos *= fpBoneLength * mtrl.fpMinStretch ;
		ESLAssert( !physVertex.vPos.IsNaN() ) ;
	}
	if ( fpCurLength > fpBoneLength * mtrl.fpMaxStretch )
	{
		physVertex.vPos *=
				fpBoneLength * mtrl.fpMaxStretch / fpCurLength ;
		ESLAssert( !physVertex.vPos.IsNaN() ) ;
	}
	//
	if ( physVertex.nHitCollider > 0 )
	{
		physVertex.nHitCollider -- ;
	}
	physVertex.vHitDeltaSpeed.x = 0 ;
	physVertex.vHitDeltaSpeed.y = 0 ;
	physVertex.vHitDeltaSpeed.z = 0 ;
	//
	if ( (pColParam != nullptr) && !(nBoneFlags & flagNoCollision) )
	{
		ESLAssert( pColParam->pCollider != nullptr ) ;
		//
		// ボーン当たり判定
		//
		S3DDVector	vHandlePos = matSubBone * physVertex.vPos + vSubBonePos ;
		vHandlePos = pColParam->matCollider * vHandlePos + pColParam->vCollider ;
		//
		S3DCollisionResult	rsHit ;
		rsHit.SetInclusionUserFlags( pColParam->maskColUser ) ;
		if ( mtrl.nPhysExFlags1 & flagPhysExColliderUseMask )
		{
			rsHit.SetExceptionUserFlags
				( (mtrl.nPhysExFlags1 & flagPhysExColliderAll)
										<< S3DCollision::colliderBone0Shift ) ;
		}
		HitColiderParam	hcp ;
		hcp.nHitCount = 0 ;
		hcp.vSumHitPos = S3DVector( 0, 0, 0 ) ;
		hcp.vSumHitNormal = S3DVector( 0, 0, 0 ) ;
		hcp.fpColRadius = (float) mtrl.fpCollisionRadius ;
		//
		rsHit.pfnOnHitCollider = &S3DModelBoneSpace::Callback_OnHitCollider ;
		rsHit.ptrOnHitInstance = &hcp ;
		//
		pColParam->pCollider->IsHitAgainstSphere
			( vHandlePos, (float) mtrl.fpCollisionRadius, rsHit ) ;
		if ( hcp.nHitCount > 0 )
		{
			// 当たり座標（表面）座標をボーンローカル空間に変換
			S3DDVector	vdHitLocal = hcp.vSumHitPos / (float) hcp.nHitCount ;
			S3DDVector	vGlobalNormal = hcp.vSumHitNormal ;
			vdHitLocal -= pColParam->vCollider ;
			pColParam->matICollider.RevolveVector( vdHitLocal ) ;
			pColParam->matICollider.RevolveVector( vGlobalNormal ) ;
			vdHitLocal -= vSubBonePos ;
			matISubBone.RevolveVector( vdHitLocal ) ;
			vdHitLocal.Normalize() ;
			//
			S3DDVector	vModHanlde = vdHitLocal * physVertex.vPos.Absolute() ;
			S3DDVector	vModDelta = vModHanlde - physVertex.vPos ;
			physVertex.vPos = vModHanlde ;
			ESLAssert( !physVertex.vPos.IsNaN() ) ;
			//
			// 運動量から当たり表面の法線方向を削除
			//
			vGlobalNormal.Normalize() ;
			S3DDVector	vHitSpeed = physVertex.vSpeed ;
			double		fpHitSpeed = vGlobalNormal.InnerProduct( vHitSpeed ) ;
			double		fpResistance = pow( mtrl.fpFrictionalResistance, framesPast ) ;
			vHitSpeed += vGlobalNormal * esl_fmin( - fpHitSpeed, 0.0 ) ;
			vHitSpeed *= fpResistance ;
			physVertex.vSpeed = vHitSpeed ;
			//
			// 当たり判定オブジェクトに押された運動量加算
			S3DDVector	vModSpeed =
				matBone * (vModDelta * (mtrl.fpEffect / secPast * fpResistance)) ;
			physVertex.vSpeed += vModSpeed ;
			physVertex.vHitDeltaSpeed = vModSpeed ;
			//
			physVertex.nHitCollider = 4 ;
			physVertex.vHitNormal = vGlobalNormal ;
		}
	}
	if ( mtrl.fpLimitedAngle > 0.0 )
	{
		//
		// 曲がり角制限処理
		//
		S3DDVector	vPos = physVertex.vPos ;
		double		r = vPos.Absolute() ;
		if ( r > 0 )
		{
			S3DDVector	vHandle = vBoneHandle ;
			vHandle.Normalize() ;
			//
			vPos *= 1.0 / r ;
			//
			double	pb2 = (pOtherParams != nullptr)
							? pOtherParams->m_fpLastPhysBelnd
								* pOtherParams->m_fpLastPhysBelnd : 1.0 ;
			double	radLim = (180.0 - mtrl.fpLimitedAngle * pb2) * PI / 180.0 ;
			double	cosLim = cos( radLim ) ;
			double	cosCross = vHandle.InnerProduct( vPos ) ;
			if ( cosCross < cosLim )
			{
				S3DDVector	vOrth = vPos - vHandle * cosCross ;
				vOrth.Normalize() ;
				//
				physVertex.vPos =
						vHandle * (cosLim * r)
							+ vOrth * (sin(radLim) * r) ;
				ESLAssert( !physVertex.vPos.IsNaN() ) ;
			}
		}
	}
	return	true ;
}

S3DCollision::HitColliderCallback
	S3DModelBoneSpace::Callback_OnHitCollider
		( const S3DCollisionResult& rsHit,
			const S3DVector& vHitPos, const S3DVector& vHitNormal,
			const S3DCollision::MeshCollision * pMesh, size_t iPolygon )
{
	HitColiderParam *	phcp = (HitColiderParam*) rsHit.ptrOnHitInstance ;

	S3DCollision::HitColliderGlobalInfo	hcgi ;
	rsHit.GetHitColliderGlobalInfo( hcgi, pMesh ) ;

	S3DVector	vGlobalNormal = (hcgi.matToGlobal * vHitNormal).Normalized() ;
	phcp->vSumHitPos += (hcgi.matToGlobal * vHitPos + hcgi.vToGlobal)
									+ vGlobalNormal * phcp->fpColRadius ;
	phcp->vSumHitNormal += vGlobalNormal ;
	phcp->nHitCount ++ ;

	return	S3DCollision::hitColliderNext ;
}

// 物理演算の状態をボーンに反映（経過時間0の場合の処理）
//////////////////////////////////////////////////////////////////////////////
void S3DModelBoneSpace::ReflectPhysics( const S3DDMatrix& matParentBone )
{
	if ( m_flagsBone & flagBonePhysics )
	{
		ReflectBonePhysics( matParentBone ) ;
	}
	S3DDMatrix		matBone = matParentBone * m_matTransformation ;
	Space *const*	ppChildren = m_children.GetConstArray() ;
	size_t			countChildren = m_children.GetLength() ;
	for ( size_t i = 0; i < countChildren; i ++ )
	{
		S3DModelBoneSpace *	pChild =
			ESLTypeCast<S3DModelBoneSpace>( ppChildren[i] ) ;
		if ( pChild != NULL )
		{
			pChild->ReflectPhysics( matBone ) ;
		}
	}
}

void S3DModelBoneSpace::ReflectBonePhysics( const S3DDMatrix& matParentBone )
{
//	S3DDMatrix	matOld = m_matTransformation ;

	BoneTransformationFromPhysics
		( m_matTransformation, m_vBoneHandle, m_physCurrent, m_fpPhysBelnd ) ;
/*
	S3DDMatrix		matSubDelta = m_matTransformation.Inverse() * matOld ;
	S3DDMatrix		matSubDelta2 = (matParentBone * m_matTransformation).Inverse() * m_physCurrent.matLastSpace ;
	Space *const*	ppChildren = m_children.GetConstArray() ;
	size_t			countChildren = m_children.GetLength() ;
	for ( size_t i = 0; i < countChildren; i ++ )
	{
		S3DModelBoneSpace *	pChild =
			ESLTypeCast<S3DModelBoneSpace>( ppChildren[i] ) ;
		if ( pChild != NULL )
		{
			pChild->m_matTransformation =
				matSubDelta * pChild->m_matTransformation ;
			pChild->m_physCurrent.matLastSpace =
				matSubDelta2 * pChild->m_physCurrent.matLastSpace ;
			pChild->m_physCurrent.vSpeed += m_physCurrent.vHitDeltaSpeed * 0.5 ;
		}
	}
*/
}

void S3DModelBoneSpace::BoneTransformationFromPhysics
	( S3DDMatrix& matBoneLocal, const S3DDVector& vBoneHandle,
			PhysVertex& physVertex, double fpPhysBlend )
{
	S3DDMatrix	matRotate( 1, 1, 1 ) ;
	matRotate.VectorRotationOf( vBoneHandle, physVertex.vPos ) ;
	//
	S3DDMatrix	matMag( 1, 1, 1 ) ;
	double		fpMag = physVertex.vPos.Absolute() / vBoneHandle.Absolute() ;
	matMag.MagnifyOnVectorOf( physVertex.vPos, fpMag ) ;
	//
	if ( fpPhysBlend < 0.99999 )
	{
		S3DModelPose::MatrixElement	meOrg ;
		meOrg.FromMatrix( matBoneLocal ) ;
		//
		S3DDQuaternion	qRotate ;
		qRotate.FromMatrix( matRotate ) ;
		qRotate = qRotate * fpPhysBlend
				+ meOrg.qRotation * (1.0 - fpPhysBlend) ;
		qRotate.Normalize() ;
		qRotate.ToMatrix( matRotate ) ;
		//
		S3DDVector	vBlendHandle =
			matBoneLocal * vBoneHandle * fpPhysBlend
							+ vBoneHandle * (1.0 - fpPhysBlend) ;
		fpMag = fpMag * fpPhysBlend + (1.0 - fpPhysBlend) ;
		matMag = S3DDMatrix( 1, 1, 1 ) ;
		matMag.MagnifyOnVectorOf( vBlendHandle, fpMag ) ;
		//
		physVertex.vPos = matRotate * matMag * vBoneHandle ;
	}
	matBoneLocal = matRotate * matMag ;
}

// 特定のボーン配列のインデックスを自分自身と子ボーンを親から順番列挙する
//////////////////////////////////////////////////////////////////////////////
void S3DModelBoneSpace::AddBoneOrderIndex
	( SSystem::SArray<size_t>& aBoneIndex,
		const S3DModelBoneSpace *const * ppBoneSet, size_t nBoneSetCount ) const
{
	for ( size_t iBone = 0; iBone < nBoneSetCount; iBone ++ )
	{
		if ( ppBoneSet[iBone] == this )
		{
			aBoneIndex.Add( iBone ) ;
			break ;
		}
	}
	Space *const*	ppChildren = m_children.GetConstArray() ;
	size_t			countChildren = m_children.GetLength() ;
	for ( size_t i = 0; i < countChildren; i ++ )
	{
		S3DModelBoneSpace *	pChild =
			ESLTypeCast<S3DModelBoneSpace>( ppChildren[i] ) ;
		if ( pChild != nullptr )
		{
			pChild->AddBoneOrderIndex( aBoneIndex, ppBoneSet, nBoneSetCount ) ;
		}
	}
}

// ボーン操作（IK）
//////////////////////////////////////////////////////////////////////////////
void S3DModelBoneSpace::OperateInverseKinematics
	( const S3DDVector& vPos,
		const S3DDMatrix& matRotate,
		const S3DDVector& vLocalTip,
		double fpBendingWeight, double zGimbalWeight,
		double fpTipBoneWeight, size_t nEffectParents )
{
	//
	// flagIKParentAxis ボーンの処理は先に行う
	//
	{
		S3DModelBoneSpace *	pJoint = this ;
		for ( size_t j = 0; j < nEffectParents; j ++ )
		{
			S3DModelBoneSpace *	pParent = pJoint->GetParentBone() ;
			if ( pParent == nullptr )
			{
				break ;
			}
			double	cosParentBendingLimit = -1 ;
			if ( pParent->GetIKParameter().nIKFlags & flagIKMaxBent )
			{
				cosParentBendingLimit =
						cos( pParent->GetIKParameter().degMaxBent * PI / 180.0 ) ;
			}
			const IKParameter&	ikParam = pJoint->GetIKParameter() ;
			if ( (ikParam.nIKFlags & flagIKParentAxis)
				&& (ikParam.vBendDirection.InnerProduct( ikParam.vBendDirection ) > 1.0e-7) )
			{
				S3DDMatrix	matParent ;
				S3DDVector	vParent ;
				S3DDMatrix	matJoint ;
				S3DDVector	vJoint ;
				pParent->CalcBoneTransformation( matParent, vParent ) ;
				pParent->CalcSubBoneTransformation
							( matJoint, vJoint, matParent, vParent, pJoint ) ;
				//
				S3DDMatrix	matBone ;
				S3DDVector	vBonePos ;
				CalcBoneTransformation( matBone, vBonePos ) ;
				S3DDVector	vJointTip = matBone * vLocalTip + vBonePos ;
				//
				S3DDVector	vRotAxis = ikParam.vBendDirection.Normalized() ;
				S3DDVector	vTipDelta = vJointTip - vParent ;
				S3DDVector	vPosDelta = vPos - vParent ;
				vTipDelta -= vRotAxis * vTipDelta.InnerProduct( vRotAxis ) ;
				vPosDelta -= vRotAxis * vPosDelta.InnerProduct( vRotAxis ) ;
				//
				S3DDMatrix	matRotOnAxis( 1, 1, 1 ) ;
				matRotOnAxis.VectorRotationOf( vTipDelta, vPosDelta ) ;
				//
				S3DDMatrix	matRotated = matRotOnAxis * matParent ;
				pParent->SetGlobalMatrixLimitedAngle( matRotated, cosParentBendingLimit ) ;
			}
			if ( pParent->GetIKParameter().nIKFlags & flagIKTerminate )
			{
				break ;
			}
			pJoint = pParent ;
		}
	}
	//
	// 通常の処理
	//
	for ( int i = 0; i < 2; i ++ )
	{
		S3DModelBoneSpace *	pJoint = this ;
		double				zJointGimbal = zGimbalWeight ;
		double				fpJointWeight = fpBendingWeight ;
		bool				flagReached = false ;
		for ( size_t j = 0; j < nEffectParents; j ++ )
		{
			S3DModelBoneSpace *	pParent = pJoint->GetParentBone() ;
			if ( pParent == nullptr )
			{
				break ;
			}
			if ( pParent->GetBoneFlags() & flagTrackingPos )
			{
				break ;
			}
			S3DDMatrix	matBone ;
			S3DDVector	vBonePos ;
			CalcBoneTransformation( matBone, vBonePos ) ;
			S3DDVector	vJointTip = matBone * vLocalTip + vBonePos ;
			//
			if ( OperateInverseKinematicsAtSubJoint
				( vPos, matRotate, vJointTip, pParent, fpJointWeight, zJointGimbal ) )
			{
				flagReached = true ;
				break ;
			}
			if ( pParent->GetIKParameter().nIKFlags & flagIKTerminate )
			{
				break ;
			}
			fpJointWeight *= fpBendingWeight ;
			zJointGimbal *= zGimbalWeight ;
			pJoint = pParent ;
		}
		if ( flagReached )
		{
			break ;
		}
	}
	SlerpGlobalMatrix( matRotate, fpTipBoneWeight ) ;
}

bool S3DModelBoneSpace::OperateInverseKinematicsAtSubJoint
	( const S3DDVector& vPos,
		const S3DDMatrix& matRotate,
		const S3DDVector& vJointTip,
		S3DModelBoneSpace * pJoint,
		double fpBendingWeight, double zGimbalWeight )
{
	if ( this == pJoint )
	{
		return	true ;
	}
	const IKParameter&	ikParam = pJoint->GetIKParameter() ;
	//
	// 曲げ制限角
	//
	double	cosLimit = -1.0 ;		// cos(180[deg])
	cosLimit = 1.0 - (1.0 - cosLimit)
					* (fpBendingWeight * ikParam.fpWeight) ;
	//
	double	cosBendingLimit = -1 ;
	if ( pJoint->GetIKParameter().nIKFlags & flagIKMaxBent )
	{
		cosBendingLimit = cos( pJoint->GetIKParameter().degMaxBent * PI / 180.0 ) ;
	}
	//
	S3DModelBoneSpace *	pParent = pJoint->GetParentBone() ;
	if ( pParent == NULL )
	{
		//
		// ルートボーンの処理
		//
		pJoint->OperateBoneRotation
			( pJoint->m_vBoneHandle, vPos,
				matRotate, cosBendingLimit, zGimbalWeight ) ;
		return	true ;
	}
	S3DDMatrix	matParent ;
	S3DDVector	vParent ;
	S3DDMatrix	matJoint ;
	S3DDVector	vJoint ;
	pParent->CalcBoneTransformation( matParent, vParent ) ;
	pParent->CalcSubBoneTransformation
				( matJoint, vJoint, matParent, vParent, pJoint ) ;
	//
	// 曲げ制限角
	//
	double	cosParentBendingLimit = -1 ;
	if ( pParent->GetIKParameter().nIKFlags & flagIKMaxBent )
	{
		cosParentBendingLimit =
				cos( pParent->GetIKParameter().degMaxBent * PI / 180.0 ) ;
	}
	//
	// 回転軸適用（ロボットアームなど用）
	//
	S3DDVector	vSubTip = vJointTip ;
	/*
	if ( (ikParam.nIKFlags & flagIKParentAxis)
		&& (ikParam.vBendDirection.InnerProduct( ikParam.vBendDirection ) > 1.0e-7) )
	{
		S3DDVector	vRotAxis = ikParam.vBendDirection.Normalized() ;
		S3DDVector	vTipDelta = vJointTip - vParent ;
		S3DDVector	vPosDelta = vPos - vParent ;
		vTipDelta -= vRotAxis * vTipDelta.InnerProduct( vRotAxis ) ;
		vPosDelta -= vRotAxis * vPosDelta.InnerProduct( vRotAxis ) ;
		//
		S3DDMatrix	matRotOnAxis( 1, 1, 1 ) ;
		matRotOnAxis.VectorRotationOf( vTipDelta, vPosDelta ) ;
		//
		S3DDMatrix	matRotated = matRotOnAxis * matParent ;
		pParent->SetGlobalMatrixLimitedAngle( matRotated, cosParentBendingLimit ) ;
		//
		S3DDMatrix	matLastParent = matParent ;
		S3DDVector	vLastParent = vParent ;
		pParent->CalcBoneTransformation( matParent, vParent ) ;
		pParent->CalcSubBoneTransformation
					( matJoint, vJoint, matParent, vParent, pJoint ) ;
		//
		vSubTip = matParent * (matLastParent.Inverse()
									* (vJointTip - vParent)) + vParent ;
	}
	*/
	//
	// 曲げ位置計算
	// ※腕の長さ = a0, a1, 曲げたときの先端までの距離 = d のとき、
	//   先端方向の曲げ位置 x, 先端方向に対して垂直方向のずれ y は
	//   x = (a1^2 - a0^2 - d^2) / (-2d)
	//   y^2 = a0^2 - x^2
	//
	//
	S3DDVector	vJoint0 = vJoint - vParent ;
	S3DDVector	vJoint1 = vSubTip - vJoint ;
	S3DDVector	vJoint2 = vPos - vParent ;
	double		a02 = vJoint0.InnerProduct( vJoint0 ) ;
	double		a12 = vJoint1.InnerProduct( vJoint1 ) ;
	double		d2 = vJoint2.InnerProduct( vJoint2 ) ;
	double		a0 = sqrt( a02 ) ;
	double		a1 = sqrt( a12 ) ;
	double		d = sqrt( d2 ) ;
	//
	if ( a0 + a1 <= d )
	{
		//
		// 腕をいっぱいに伸ばした状態
		//
		pParent->OperateBoneRotation
			( pJoint, vPos, matRotate, cosParentBendingLimit, 0.0 ) ;
		pJoint->OperateBoneRotation
			( this, vPos, matRotate, cosBendingLimit, zGimbalWeight ) ;
		return	false ;
	}
	double		x = (a12 - a02 - d2) / (-2.0 * d) ;
	double		y = sqrt( esl_fmax( a02 - x * x, 0.0 ) ) ;
	//
	// 曲げすぎ補正
	//
	bool	flagLimited = false ;
	double	aMin = esl_fmin( a0, a1 ) ;
	if ( (d < a0 + a1 - aMin) && (x < a0 * cosLimit) )
	{
		x = aMin * cosLimit ;
		y = sqrt( esl_fmax( a02 - x * x, 0.0 ) ) ;
		d = sqrt( fabs( a12 - y * y ) ) + x ;
		d2 = d * d ;
		flagLimited = true ;
	}
	//
	// 関節の曲げ方向
	//
	S3DDVector	vJoint2Dir = vJoint2 ;
	vJoint2Dir.Normalize() ;
	//
	S3DDVector	vBendDir ;
	if ( (ikParam.nIKFlags & flagIKBendDirection)
		&& (ikParam.vBendDirection.InnerProduct( ikParam.vBendDirection ) > 1.0e-7) )
	{
		vBendDir = ikParam.vBendDirection ;
	}
	else
	{
		vBendDir = vJoint0 ;
		vBendDir -= vJoint2Dir * vJoint2Dir.InnerProduct(vJoint0) ;
		//
		if ( vBendDir.InnerProduct( vBendDir ) < 1.0e-8 )
		{
			vBendDir = matParent * S3DDVector( 0, 0, -1 ) ;
		}
		else
		{
			vBendDir = - vBendDir ;
		}
	}
	vBendDir = CalcGimbalRotationElement
				( matRotate, matParent,
					vJoint2, zGimbalWeight ) * vBendDir ;
	vBendDir -= vJoint2Dir * vJoint2Dir.InnerProduct(vBendDir) ;
	vBendDir.Normalize() ;
	//
	// 根元ボーン回転
	//
	S3DDVector	vHandlePos = vParent + vJoint2Dir * x - vBendDir * y ;
	bool	flagParent =
		pParent->OperateBoneRotation
			( vJoint /*pJoint*/, vHandlePos, matRotate, cosParentBendingLimit, 0.0 ) ;
	//
	// 間接ボーン回転
	//
	S3DDMatrix	matLastParent = matParent ;
	S3DDVector	vLastParent = vParent ;
	pParent->CalcBoneTransformation( matParent, vParent ) ;
	vSubTip = matParent * (matLastParent.Inverse()
								* (vSubTip - vParent)) + vParent ;
	bool	flagJoint =
		pJoint->OperateBoneRotation
			( vSubTip, vPos, matRotate, cosBendingLimit, zGimbalWeight ) ;
	//
	return	!flagLimited && flagParent && flagJoint ;
}

bool S3DModelBoneSpace::OperateBoneRotation
	( S3DModelBoneSpace * pHandleBone,
		const S3DDVector& vGlobalPos,
		const S3DDMatrix& matRotate,
		double cosLimitBending, double zGimbalWeight )
{
	S3DDMatrix	matHandle ;
	S3DDVector	vHandle ;
	pHandleBone->CalcBoneTransformation( matHandle, vHandle ) ;
	//
	return	OperateBoneRotation
		( vHandle, vGlobalPos, matRotate, cosLimitBending, zGimbalWeight ) ;
}

bool S3DModelBoneSpace::OperateBoneRotation
	( const S3DDVector& vGlobalHandlePos,
		const S3DDVector& vGlobalPos,
		const S3DDMatrix& matRotate,
		double cosLimitBending, double zGimbalWeight )
{
	S3DDMatrix	matBone ;
	S3DDVector	vBone ;
	CalcBoneTransformation( matBone, vBone ) ;
	//
	S3DDVector	vDelta0 = vGlobalHandlePos - vBone ;
	S3DDVector	vDelta1 = vGlobalPos - vBone ;
	S3DDMatrix	matHandleRot( 1, 1, 1 ) ;
	matHandleRot.VectorRotationOf( vDelta0, vDelta1 ) ;
	//
	S3DDMatrix	matGimbal =
		CalcGimbalRotationElement
			( matRotate, matHandleRot * matBone, vDelta1, zGimbalWeight ) ;
	return	SetGlobalMatrixLimitedAngle( matGimbal, cosLimitBending ) ;
}

S3DDMatrix S3DModelBoneSpace::CalcGimbalRotationElement
	( const S3DDMatrix& matRotate,
		const S3DDMatrix& matBone,
		const S3DDVector& vAxis, double zGimbalWeight )
{
	if ( zGimbalWeight <= 0.01 )
	{
		return	matBone ;
	}
	S3DDMatrix	matAxis( 1, 1, 1 ) ;
	matAxis.RevolveByAngleOn( vAxis ) ;
	//
	S3DDMatrix	matTemp = matAxis * matRotate ;
	S3DDMatrix	matOrg = matAxis * matBone ;
	double	c1 = matTemp.m[0][0] + matTemp.m[1][1] ;
	double	s1 = matTemp.m[1][0] - matTemp.m[0][1] ;
	double	c0 = matOrg.m[0][0] + matOrg.m[1][1] ;
	double	s0 = matOrg.m[1][0] - matOrg.m[0][1] ;
	double	r1 = sqrt( c1 * c1 + s1 * s1 ) ;
	double	r0 = sqrt( c0 * c0 + s0 * s0 ) ;
	if ( (r0 < 0.25) || (r1 < 0.25) )
	{
		return	matBone ;
	}
	s1 /= r1 ;
	c1 /= r1 ;
	s0 /= r0 ;
	c0 /= r0 ;
	//
	// (c1 + i*s1) / (c0 + i*s0)
	double	c = c0 * c1 + s0 * s1 ;
	double	s = c0 * s1 - c1 * s0 ;
	//
	if ( zGimbalWeight < 1.0 )
	{
		c = 1.0 - (1.0 - c) * zGimbalWeight ;
		if ( s >= 0.0 )
		{
			s = sqrt( esl_fmax( 1.0 - c * c, 0.0 ) ) ;
		}
		else
		{
			s = - sqrt( esl_fmax( 1.0 - c * c, 0.0 ) ) ;
		}
	}
	//
	S3DDMatrix	matGimbal( 1, 1, 1 ) ;
	matGimbal.RevolveOnZ( s, c ) ;
	return	matAxis.Inverse() * matGimbal * matAxis * matBone ;
}

void S3DModelBoneSpace::SetGlobalMatrix( const S3DDMatrix& matRotate )
{
	S3DModelBoneSpace *	pParent = GetParentBone() ;
	if ( pParent != NULL )
	{
		S3DDMatrix	matParent ;
		S3DDVector	vParent ;
		pParent->CalcBoneTransformation( matParent, vParent ) ;
		//
		m_matTransformation = matParent.Inverse() * matRotate ;
	}
	else
	{
		m_matTransformation = matRotate ;
	}
}

void S3DModelBoneSpace::SlerpGlobalMatrix( const S3DDMatrix& matRotate, double w )
{
	if ( w <= 0.0 )
	{
		return ;
	}
	S3DModelBoneSpace *	pParent = GetParentBone() ;
	S3DDMatrix	matNewLocal( 1, 1, 1 ) ;
	if ( pParent != NULL )
	{
		S3DDMatrix	matParent ;
		S3DDVector	vParent ;
		pParent->CalcBoneTransformation( matParent, vParent ) ;
		//
		matNewLocal = matParent.Inverse() * matRotate ;
	}
	else
	{
		matNewLocal = matRotate ;
	}
	S3DDQuaternion	qOld = m_matTransformation ;
	S3DDQuaternion	qNew = matNewLocal ;
	S3DDQuaternion	qBlend ;
	qBlend.Slerp( qOld, qNew, w ) ;
	qBlend.ToMatrix( m_matTransformation ) ;
}

bool S3DModelBoneSpace::SetGlobalMatrixLimitedAngle
		( const S3DDMatrix& matRotate, double cosLimit )
{
	SetGlobalMatrix( matRotate ) ;
	//
	S3DDVector	vBoneHandle = m_vBoneHandle.Normalized() ;
	S3DDVector	vRotHandle = (m_matTransformation * vBoneHandle).Normalized() ;
	double		cosRotate = vBoneHandle.InnerProduct( vRotHandle ) ;
	if ( (cosRotate < cosLimit) && (cosRotate < 0.99999) )
	{
		const double	t = acos(cosLimit) / acos(cosRotate) ;
		S3DDQuaternion	qBone = m_matTransformation ;
		S3DDQuaternion	q1( 1, 0, 0, 0 ) ;
		S3DDQuaternion	qLimited ;
		qLimited.Slerp( q1, qBone, t ) ;
		qLimited.ToMatrix( m_matTransformation ) ;
		return	false ;
	}
	return	true ;
}

// ボーンを回転し、子ボーンの姿勢は維持する
//////////////////////////////////////////////////////////////////////////////
void S3DModelBoneSpace::RotateBoneAndKeepChildrenPosture
	( const S3DDMatrix& matRevDelta, S3DDVector * pvLocalPos )
{
	S3DDMatrix	matOrgBone = m_matTransformation ;
	m_matTransformation = matRevDelta * matOrgBone ;
	//
	S3DDMatrix	matIBone ;
	matIBone.InverseOf( m_matTransformation ) ;
	//
	S3DDMatrix	matRevChild = matIBone * matOrgBone ;
	for ( size_t i = 0; i < GetChildrenCount(); i ++ )
	{
		S3DScene::Space *	pChild = GetChildAt( i ) ;
		if ( pChild == NULL )
		{
			continue ;
		}
		pChild->m_matTransformation =
			matRevChild * pChild->m_matTransformation ;
	}
	if ( pvLocalPos != NULL )
	{
		*pvLocalPos = matRevChild * *pvLocalPos ;
	}
}

// 座標とハンドルの範囲取得
//////////////////////////////////////////////////////////////////////////////
void S3DModelBoneSpace::GetExternalRectangular
	( S3DDVector& vMin, S3DDVector& vMax,
		const S3DDMatrix& matParent, const S3DDVector& vParent ) const
{
	S3DDVector	vPos = matParent * (m_vCenter + GetBoneOffset()) ;
	S3DDMatrix	matBone = matParent * m_matTransformation ;
	//
	vMin.x = esl_fmin( vMin.x, vPos.x ) ;
	vMin.y = esl_fmin( vMin.y, vPos.y ) ;
	vMin.z = esl_fmin( vMin.z, vPos.z ) ;
	vMax.x = esl_fmax( vMax.x, vPos.x ) ;
	vMax.y = esl_fmax( vMax.y, vPos.y ) ;
	vMax.z = esl_fmax( vMax.z, vPos.z ) ;
	//
	S3DDVector	vHandle = matBone * GetBoneHandle() + vPos ;
	//
	vMin.x = esl_fmin( vMin.x, vHandle.x ) ;
	vMin.y = esl_fmin( vMin.y, vHandle.y ) ;
	vMin.z = esl_fmin( vMin.z, vHandle.z ) ;
	vMax.x = esl_fmax( vMax.x, vHandle.x ) ;
	vMax.y = esl_fmax( vMax.y, vHandle.y ) ;
	vMax.z = esl_fmax( vMax.z, vHandle.z ) ;
	//
	Space *const*	ppChildren = m_children.GetConstArray() ;
	size_t			countChildren = m_children.GetLength() ;
	for ( size_t i = 0; i < countChildren; i ++ )
	{
		S3DModelBoneSpace *	pChild =
			ESLTypeCast<S3DModelBoneSpace>( ppChildren[i] ) ;
		if ( pChild != NULL )
		{
			pChild->GetExternalRectangular( vMin, vMax, matBone, vPos ) ;
		}
	}
}

// モデル関連付け
//////////////////////////////////////////////////////////////////////////////
void S3DModelBoneSpace::AttachModel( S3DModelBuffer * pModel )
{
	m_pModel = pModel ;
//	m_iTargetVertex = 0 ;
//	m_iTargetNormal = 0 ;
//	m_nTargetCount = 0 ;
	m_arrRefMesh.FreeArray() ;
	m_wmbWeight.ClearBuffer() ;
//	m_bufWeight.FreeArray() ;
}

// ボーン設定（メッシュ境界補正あり）
//////////////////////////////////////////////////////////////////////////////
void S3DModelBoneSpace::SetBoneWeight
	( S3DModelBuffer * pModel,
		size_t iFirstVertex, size_t iFirstNormal,
		size_t nCount, const float32_t * pfpWeight )
{
	m_pModel = pModel ;
	m_wmbWeight.ClearBuffer() ;
//	m_bufWeight.FreeArray() ;
//	m_iTargetVertex = iFirstVertex ;
//	m_iTargetNormal = iFirstNormal ;
//	m_nTargetCount = nCount ;
	m_arrRefMesh.FreeArray() ;
	//
	if ( pModel != NULL )
	{
		const size_t	iWeightFirst = iFirstVertex ;
		size_t	nPrePadding = 0 ;
		size_t	nPostPadding = 0 ;
		size_t	iEndVertex = iFirstVertex + nCount ;
		size_t	iEndNormal = iFirstNormal + nCount ;
		ESLAssert( iEndVertex <= pModel->GetVertexBufferLength() ) ;
		ESLAssert( iEndNormal <= pModel->GetNormalBufferLength() ) ;
		size_t	nMeshCount = pModel->GetMeshCount() ;
		for ( size_t i = 0; i < nMeshCount; i ++ )
		{
			S3DModelBuffer::MeshObject *
				pMeshObj = pModel->GetMeshObjectAt( i ) ;
			ESLAssert( pMeshObj != NULL ) ;
			if ( pMeshObj == NULL )
			{
				continue ;
			}
			size_t	iFirstMeshVertex = pMeshObj->m_iVertex ;
			size_t	iEndMeshVertex = iFirstMeshVertex + pMeshObj->m_countVertex - 1 ;
			size_t	iFirstMeshNormal = pMeshObj->m_iNormal ;
			size_t	iEndMeshNormal = iFirstMeshNormal + pMeshObj->m_countVertex - 1 ;
			if ( ((iFirstVertex <= iFirstMeshVertex)
						&& (iFirstMeshVertex < iEndVertex))
				|| ((iFirstVertex <= iEndMeshVertex)
						&& (iEndMeshVertex < iEndVertex))
				|| ((iFirstMeshVertex <= iFirstVertex)
						&& (iFirstVertex < iEndMeshVertex))
				|| ((iFirstMeshVertex <= iEndVertex - 1)
						&& (iEndVertex - 1 < iEndMeshVertex))
				|| ((iFirstNormal <= iFirstMeshNormal)
						&& (iFirstMeshNormal < iEndNormal))
				|| ((iFirstNormal <= iEndMeshNormal)
						&& (iEndMeshNormal < iEndNormal))
				|| ((iFirstMeshNormal <= iFirstNormal)
						&& (iFirstNormal < iEndMeshNormal))
				|| ((iFirstMeshNormal <= iEndNormal - 1)
						&& (iEndNormal - 1 < iEndMeshNormal)) )
			{
				iEndMeshVertex ++ ;
				iEndMeshNormal ++ ;
				//
				if ( iFirstVertex > iFirstMeshVertex )
				{
					size_t	n = iFirstVertex - iFirstMeshVertex ;
					nPrePadding += n ;
					iFirstVertex = iFirstMeshVertex ;
					iFirstNormal -= n ;
					ESLAssert( (ssize_t) iFirstNormal >= 0 ) ;
				}
				if ( iFirstNormal > iFirstMeshNormal )
				{
					size_t	n = iFirstNormal - iFirstMeshNormal ;
					nPrePadding += n ;
					iFirstVertex -= n ;
					iFirstNormal = iFirstMeshNormal ;
					ESLAssert( (ssize_t) iFirstVertex >= 0 ) ;
				}
				if ( iEndVertex < iEndMeshVertex )
				{
					size_t	n = iEndMeshVertex - iEndVertex ;
					nPostPadding += n ;
					iEndVertex = iEndMeshVertex ;
					iEndNormal += n ;
				}
				if ( iEndNormal < iEndMeshNormal )
				{
					size_t	n = iEndMeshNormal - iEndNormal ;
					nPostPadding += n ;
					iEndVertex += n ;
					iEndNormal = iEndMeshVertex ;
				}
				//
				size_t	iInnerFirst = iFirstMeshVertex - iWeightFirst ;
				size_t	iInnerEnd = iEndMeshVertex - iWeightFirst ;
				if ( iFirstMeshVertex < iWeightFirst )
				{
					iInnerFirst = 0 ;
				}
				if ( iInnerEnd > nCount )
				{
					iInnerEnd = nCount ;
				}
				for ( size_t j = iInnerFirst; j < iInnerEnd; j ++ )
				{
					if ( pfpWeight[j] > 1.0e-5 )
					{
						REF_MESH_INFO	rmi ;
						rmi.iMesh = i ;
						rmi.matIMesh.InitializeMatrix( 1, 1, 1, 1 ) ;
						rmi.matRelMesh.InitializeMatrix( 1, 1, 1, 1 ) ;
						m_arrRefMesh.Add( rmi ) ;
						break ;
					}
				}
			}
		}
		m_wmbWeight.ExpandBounds
			( iFirstVertex, nPrePadding + nCount + nPostPadding ) ;
		m_wmbWeight.WriteWeight
			( iFirstVertex + nPrePadding, pfpWeight, nCount ) ;
		ESLAssert( m_wmbWeight.GetFirst() + m_wmbWeight.GetLength() <= pModel->GetVertexBufferLength() ) ;
	}
}

// ボーン影響範囲変更
//////////////////////////////////////////////////////////////////////////////
void S3DModelBoneSpace::ExpandBoneWeightBounds
	( size_t iFirstVertex, size_t nCount )
{
	m_wmbWeight.ExpandBounds( iFirstVertex, nCount ) ;
#ifdef	__DEBUG__
	if ( m_pModel != NULL )
	{
		ESLAssert( m_wmbWeight.GetFirst() + m_wmbWeight.GetLength() <= m_pModel->GetVertexBufferLength() ) ;
	}
#endif
}

// ボーンウェイトマップ部分書き換え
//////////////////////////////////////////////////////////////////////////////
size_t S3DModelBoneSpace::ModifyBoneWeightMapBounds
	( size_t iFirstVertex, const float32_t * pfpWeight, size_t nCount )
{
	return	m_wmbWeight.WriteWeight( iFirstVertex, pfpWeight, nCount ) ;
}

// ボーン影響範囲減少（前後０要素削除）
//////////////////////////////////////////////////////////////////////////////
void S3DModelBoneSpace::TrimBoneWeightBounds( void )
{
	m_wmbWeight.TrimBounds() ;
#ifdef	__DEBUG__
	if ( m_pModel != NULL )
	{
		ESLAssert( m_wmbWeight.GetFirst() + m_wmbWeight.GetLength() <= m_pModel->GetVertexBufferLength() ) ;
	}
#endif
}

// ボーン影響範囲減少
//////////////////////////////////////////////////////////////////////////////
void S3DModelBoneSpace::ChopBoneWeightBounds( size_t nLeftCount, size_t nRightCount )
{
	m_wmbWeight.ChopBounds( nLeftCount, nRightCount ) ;
#ifdef	__DEBUG__
	if ( m_pModel != NULL )
	{
		ESLAssert( m_wmbWeight.GetFirst() + m_wmbWeight.GetLength() <= m_pModel->GetVertexBufferLength() ) ;
	}
#endif
}

// ボーン影響範囲を有意な範囲になるように正規化
//////////////////////////////////////////////////////////////////////////////
void S3DModelBoneSpace::NormalizeBoneWeightBounds( void )
{
	m_wmbWeight.TrimBounds() ;
	size_t	nCount = m_wmbWeight.GetLength() ;
	if ( nCount == 0 )
	{
		AttachModel( m_pModel ) ;
	}
	else
	{
		SArray<float32_t>	bufTemp ;
		size_t		iFirst = m_wmbWeight.GetFirst() ;
		float32_t *	pLock = m_wmbWeight.LockBuffer( iFirst, nCount ) ;
		eslCopyMemory
			( bufTemp.GetArray( nCount ),
					pLock, nCount * sizeof(float32_t) ) ;
		bufTemp.FinishArray() ;
		m_wmbWeight.UnlockBuffer( false ) ;
		//
		SetBoneWeight
			( m_pModel, iFirst, iFirst, nCount, bufTemp.GetConstArray() ) ;
	}
}

// ボーン重みマップを更新（指標はグローバル）
//////////////////////////////////////////////////////////////////////////////
void S3DModelBoneSpace::UpdateBoneWeight
	( size_t iFirstVertex, size_t nCount, const float32_t * pfpWeight )
{
	m_wmbWeight.WriteWeight( iFirstVertex, pfpWeight, nCount ) ;
}

// ボーン影響頂点取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DModelBoneSpace::VertexIndexOfBoneWeight( void ) const
{
	return	m_wmbWeight.GetFirst() ;
}

size_t S3DModelBoneSpace::NormalIndexOfBoneWeight( void ) const
{
	return	m_wmbWeight.GetFirst() ;
}

size_t S3DModelBoneSpace::VertexCountOfBoneWeight( void ) const
{
	return	m_wmbWeight.GetLength() ;
}

// ボーンウェイトマップ取得
//////////////////////////////////////////////////////////////////////////////
float32_t S3DModelBoneSpace::GetBoneWeightAt( size_t iVertex ) const
{
	return	m_wmbWeight.GetAt( iVertex ) ;
}

void S3DModelBoneSpace::SetBoneWeightAt( size_t iVertex, float32_t fpWeight )
{
	m_wmbWeight.SetAt( iVertex, fpWeight ) ;
}

void S3DModelBoneSpace::ReadBoneWeightMap
	( float32_t * pfpWeight, size_t iVertex, size_t nCount ) const
{
	m_wmbWeight.ReadWeight( pfpWeight, iVertex, nCount ) ;
}

float32_t * S3DModelBoneSpace::LockBoneWeightMap( size_t& iVertex, size_t& nCount )
{
	return	m_wmbWeight.LockBuffer( iVertex, nCount ) ;
}

void S3DModelBoneSpace::UnlockBoneWeightMap( bool flagWrite )
{
	m_wmbWeight.UnlockBuffer( flagWrite ) ;
}

// ボーンが影響するメッシュ番号を追加
//////////////////////////////////////////////////////////////////////////////
void S3DModelBoneSpace::AddEffectiveMeshIndex
	( size_t iMesh, const S4DMatrix& matIMesh, const S4DMatrix& matRelMesh )
{
	REF_MESH_INFO *	pRelMeshs = m_arrRefMesh.GetArray() ;
	size_t			nRelMeshs = m_arrRefMesh.GetLength() ;
	for ( size_t i = 0; i < nRelMeshs; i ++ )
	{
		if ( pRelMeshs[i].iMesh == iMesh )
		{
			pRelMeshs[i].matIMesh = matIMesh ;
			pRelMeshs[i].matRelMesh = matRelMesh ;
			m_arrRefMesh.FinishArray() ;
			return ;
		}
	}
	m_arrRefMesh.FinishArray() ;
	//
	REF_MESH_INFO	rmi ;
	rmi.iMesh = iMesh ;
	rmi.matIMesh = matIMesh ;
	rmi.matRelMesh = matRelMesh ;
	m_arrRefMesh.Add( rmi ) ;
}

// ボーン影響範囲テスト
//////////////////////////////////////////////////////////////////////////////
bool S3DModelBoneSpace::IsBoneEffectVertexMap
	( size_t iVertex, const SSystem::SBitArray& maskVertex ) const
{
	size_t	iDstVertex = iVertex ;
	size_t	nCount = maskVertex.GetLength() ;
//	const float32_t *
//			pWegiht = GetBoneWeightMapBoundsAt( iDstVertex, nCount ) ;
	size_t	nOffset = iVertex - iDstVertex ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
//		if ( maskVertex.GetAt( i + nOffset ) && (pWegiht[i] >= 1.0e-8) )
		if ( maskVertex.GetAt( i + nOffset )
			&& (GetBoneWeightAt( iDstVertex + i ) >= 1.0e-8) )
		{
			return	true ;
		}
	}
	return	false ;
}

// 影響ボーン数カウント
//////////////////////////////////////////////////////////////////////////////
size_t S3DModelBoneSpace::GetBoneCountEffectedVertexMap
	( size_t iVertex, const SSystem::SBitArray& maskVertex ) const
{
	size_t	nBoneCount = 0 ;
	if ( IsBoneEffectVertexMap( iVertex, maskVertex ) )
	{
		nBoneCount ++ ;
	}
	size_t	nChildren = GetChildrenCount() ;
	for ( size_t i = 0; i < nChildren; i ++ )
	{
		S3DModelBoneSpace *	pChild =
			ESLTypeCast<S3DModelBoneSpace>( GetChildAt( i ) ) ;
		if ( pChild == NULL )
		{
			continue ;
		}
		nBoneCount += pChild->GetBoneCountEffectedVertexMap( iVertex, maskVertex ) ;
	}
	return	nBoneCount ;
}

// ボーン影響範囲取得
//////////////////////////////////////////////////////////////////////////////
bool S3DModelBoneSpace::GetBoneEffectVertexMap
	( size_t iVertex, SSystem::SBitArray& maskVertex ) const
{
	size_t	iDstVertex = iVertex ;
	size_t	nCount = maskVertex.GetLength() ;
//	const float32_t *
//			pWegiht = GetBoneWeightMapBoundsAt( iDstVertex, nCount ) ;
	size_t	nOffset = iVertex - iDstVertex ;
	bool	fAnyEffect = false ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
//		if ( pWegiht[i] >= 1.0e-8 )
		if ( GetBoneWeightAt( iDstVertex + i ) >= 1.0e-8 )
		{
			maskVertex.SetAt( i + nOffset, true ) ;
			fAnyEffect = true ;
		}
	}
	return	fAnyEffect ;
}

// メッシュ統合に付随して必要なら影響メッシュの追加
//////////////////////////////////////////////////////////////////////////////
void S3DModelBoneSpace::MergeEffectiveMeshIndex( size_t iMeshDst, size_t iMeshSrc )
{
	const REF_MESH_INFO *	pRelMeshs = m_arrRefMesh.GetConstArray() ;
	size_t					nRelMeshs = m_arrRefMesh.GetLength() ;
	bool					fMergeSrc = false ;
	REF_MESH_INFO			rmiSrc ;
	for ( size_t i = 0; i < nRelMeshs; i ++ )
	{
		if ( pRelMeshs[i].iMesh == iMeshDst )
		{
			return ;
		}
		if ( pRelMeshs[i].iMesh == iMeshSrc )
		{
			fMergeSrc = true ;
			rmiSrc = pRelMeshs[i] ;
		}
	}
	if ( fMergeSrc )
	{
		rmiSrc.iMesh = iMeshDst ;
		m_arrRefMesh.Add( rmiSrc ) ;
	}
}

// ボーンが影響するメッシュ番号配列取得
//////////////////////////////////////////////////////////////////////////////
const S3DModelBoneSpace::REF_MESH_INFO *
	S3DModelBoneSpace::GetEffectiveMeshIndexArray( size_t& nCount ) const
{
	nCount = m_arrRefMesh.GetLength() ;
	return	m_arrRefMesh.GetConstArray() ;
}



//////////////////////////////////////////////////////////////////////////////
// 文字列配列
//////////////////////////////////////////////////////////////////////////////

// 文字列ID取得
//////////////////////////////////////////////////////////////////////////////
int32_t S3DModelPose::StringArray::GetStringIndex( const wchar_t * pwszStr ) const
{
	size_t	nCount = GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		SString *	pstr = GetAt( i ) ;
		if ( (pstr != NULL) && (*pstr == pwszStr) )
		{
			return	(int32_t) i ;
		}
	}
	return	-1 ;
}

// 文字列ID割り当て
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DModelPose::StringArray::AllocateString( const wchar_t * pwszStr )
{
	size_t	nCount = GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		SString *	pstr = GetAt( i ) ;
		if ( (pstr != NULL) && (*pstr == pwszStr) )
		{
			return	(uint32_t) i ;
		}
	}
	return	(uint32_t) Add( new SString( pwszStr ) ) ;
}


//////////////////////////////////////////////////////////////////////////////
// ポーズ関節
//////////////////////////////////////////////////////////////////////////////

// m_vHandle, m_zRotation から m_qRotation 計算
//////////////////////////////////////////////////////////////////////////////
void S3DModelPose::JointInfo::CalculateRotation( const S3DDVector& vOrgBoneHandle )
{
	S3DDMatrix	matRevHandle ;
	S3DDMatrix	matRevZ ;
	double		rad = m_zRotation * PI / 180.0 ;
	matRevHandle.VectorRotationOf( vOrgBoneHandle, m_vHandle ) ;
	matRevZ.RotationOnVectorOf( m_vHandle, sin(rad), cos(rad) ) ;
	m_qRotation.FromMatrix( matRevZ * matRevHandle ) ;
}


// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DModelPose::MatrixElement::MatrixElement( void )
	: matTransform( 1, 1, 1 ), qRotation( 1, 0, 0, 0 ), vZoom( 1, 1, 1 )
{
	fOrthogonal = true ;
}

// 補完処理のために要素分解
//////////////////////////////////////////////////////////////////////////////
void S3DModelPose::MatrixElement::FromMatrix( const S3DDMatrix& mat )
{
	matTransform = mat ;
	//
	vZoom.x = sqrt( mat.m[0][0] * mat.m[0][0]
					+ mat.m[1][0] * mat.m[1][0]
					+ mat.m[2][0] * mat.m[2][0] ) ;
	vZoom.y = sqrt( mat.m[0][1] * mat.m[0][1]
					+ mat.m[1][1] * mat.m[1][1]
					+ mat.m[2][1] * mat.m[2][1] ) ;
	vZoom.z = sqrt( mat.m[0][2] * mat.m[0][2]
					+ mat.m[1][2] * mat.m[1][2]
					+ mat.m[2][2] * mat.m[2][2] ) ;
	//
	double	dx = (fabs(vZoom.x) < 1.0e-8) ? 1.0 : (1.0 / vZoom.x) ;
	double	dy = (fabs(vZoom.y) < 1.0e-8) ? 1.0 : (1.0 / vZoom.y) ;
	double	dz = (fabs(vZoom.z) < 1.0e-8) ? 1.0 : (1.0 / vZoom.z) ;
	//
	S3DDMatrix	matTemp
		( mat.m[0][0] * dx, mat.m[0][1] * dy, mat.m[0][2] * dz,
			mat.m[1][0] * dx, mat.m[1][1] * dy, mat.m[1][2] * dz,
			mat.m[2][0] * dx, mat.m[2][1] * dy, mat.m[2][2] * dz ) ;
	qRotation.FromMatrix( matTemp ) ;
	//
	S3DDMatrix	matRot ;
	qRotation.ToMatrix( matRot ) ;
	//
	double	d = 0.0 ;
	for ( int i = 0; i < 3; i ++ )
	{
		d += fabs( matRot.m[i][0] - matTemp.m[i][0] ) ;
		d += fabs( matRot.m[i][1] - matTemp.m[i][1] ) ;
		d += fabs( matRot.m[i][2] - matTemp.m[i][2] ) ;
	}
	fOrthogonal = (d < 0.01) ;
}

// 行列へ変換
//////////////////////////////////////////////////////////////////////////////
void S3DModelPose::MatrixElement::ToMatrix( S3DDMatrix& mat ) const
{
	if ( fOrthogonal )
	{
		qRotation.ToMatrix( mat ) ;
		mat.MagnifyByVector( vZoom ) ;
	}
	else
	{
		mat = matTransform ;
	}
}

// 補完
//////////////////////////////////////////////////////////////////////////////
void S3DModelPose::MatrixElement::Slerp
	( const MatrixElement& meSrc, const MatrixElement& meDst, double t )
{
	if ( meSrc.fOrthogonal && meDst.fOrthogonal )
	{
		qRotation.Slerp( meSrc.qRotation, meDst.qRotation, t ) ;
		vZoom = meSrc.vZoom * (1.0 - t) + meDst.vZoom * t ;
		//
		qRotation.ToMatrix( matTransform ) ;
		matTransform.MagnifyByVector( vZoom ) ;
		fOrthogonal = true ;
	}
	else
	{
		S3DDMatrix	matSrc, matDst ;
		meSrc.GetRotation( matSrc ) ;
		meDst.GetRotation( matDst ) ;
		//
		S3DDMatrix	matTemp ;
		meSrc.qRotation.ToMatrix( matTemp ) ;
		matSrc -= matTemp ;
		//
		meDst.qRotation.ToMatrix( matTemp ) ;
		matDst -= matTemp ;
		//
		qRotation.Slerp( meSrc.qRotation, meDst.qRotation, t ) ;
		vZoom = meSrc.vZoom * (1.0 - t) + meDst.vZoom * t ;
		//
		qRotation.ToMatrix( matTransform ) ;
		matTransform += matSrc * (1.0 - t) + matDst * t ;
		matTransform.MagnifyByVector( vZoom ) ;
		//
		if ( t < 0.01 )
		{
			fOrthogonal = meSrc.fOrthogonal ;
		}
		else if ( t > 0.99 )
		{
			fOrthogonal = meDst.fOrthogonal ;
		}
		else
		{
			fOrthogonal = false ;
		}
	}
}

// 回転行列取得（拡大成分以外）
//////////////////////////////////////////////////////////////////////////////
void S3DModelPose::MatrixElement::GetRotation( S3DDMatrix& matRot ) const
{
	double	dx = (fabs(vZoom.x) < 1.0e-8) ? 1.0 : (1.0 / vZoom.x) ;
	double	dy = (fabs(vZoom.y) < 1.0e-8) ? 1.0 : (1.0 / vZoom.y) ;
	double	dz = (fabs(vZoom.z) < 1.0e-8) ? 1.0 : (1.0 / vZoom.z) ;
	//
	for ( int i = 0; i < 3; i ++ )
	{
		matRot.m[i][0] = matTransform.m[i][0] * dx ;
		matRot.m[i][1] = matTransform.m[i][1] * dy ;
		matRot.m[i][2] = matTransform.m[i][2] * dz ;
	}
}


// モーションデータが静止データの場合削除する
//////////////////////////////////////////////////////////////////////////////
void S3DModelPose::JointAnimation::SmartMotion( void )
{
	size_t	i, nCount ;
	//
	// 行列
	//
	nCount = m_aMatrixs.GetLength() ;
	if ( nCount > 0 )
	{
		const S4DMatrix *	pMatrixs = m_aMatrixs.GetConstArray() ;
		S4DMatrix			mat0 = pMatrixs[0] ;
		bool				fMotion = false ;
		for ( i = 1; i < nCount; i ++ )
		{
			if ( pMatrixs[i] != mat0 )
			{
				fMotion = true ;
				break ;
			}
		}
		if ( !fMotion )
		{
			S4DDMatrix	mat4Temp = mat0 ;
			S3DDMatrix	matTrans ;
			S3DDVector	vTrans ;
			Matrix3x3From4x4
				<S3DDMatrix,S3DDVector,double>( matTrans, vTrans, mat4Temp ) ;
			//
			MatrixElement	me ;
			me.FromMatrix( matTrans ) ;
			m_qRotation = me.qRotation ;
			m_vZoom = me.vZoom ;
			m_vOffset = vTrans ;
			//
			m_aMatrixs.FreeArray() ;
		}
	}
	//
	// 回転
	//
	nCount = m_aRotations.GetLength() ;
	if ( nCount > 0 )
	{
		const S3DDQuaternion *	pRotations = m_aRotations.GetConstArray() ;
		S3DDQuaternion			q0 = pRotations[0] ;
		bool					fMotion = false ;
		for ( i = 1; i < nCount; i ++ )
		{
			if ( (pRotations[i] - q0).Norm() > 1.0e-8 )
			{
				fMotion = true ;
				break ;
			}
		}
		if ( !fMotion )
		{
			m_qRotation = q0 ;
			m_aRotations.FreeArray() ;
		}
	}
	//
	// 移動
	//
	nCount = m_aOffsets.GetLength() ;
	if ( nCount > 0 )
	{
		const S3DDVector *	pOffsets = m_aOffsets.GetConstArray() ;
		S3DDVector			v0 = pOffsets[0] ;
		bool				fMotion = false ;
		for ( i = 1; i < nCount; i ++ )
		{
			if ( (pOffsets[i] - v0).Absolute() > 1.0e-8 )
			{
				fMotion = true ;
				break ;
			}
		}
		if ( !fMotion )
		{
			m_vOffset = v0 ;
			m_aOffsets.FreeArray() ;
		}
	}
	//
	// 拡大
	//
	nCount = m_aZooms.GetLength() ;
	if ( nCount > 0 )
	{
		const S3DDVector *	pZooms = m_aZooms.GetConstArray() ;
		S3DDVector			v0 = pZooms[0] ;
		bool				fMotion = false ;
		for ( i = 1; i < nCount; i ++ )
		{
			if ( (pZooms[i] - v0).Absolute() > 1.0e-8 )
			{
				fMotion = true ;
				break ;
			}
		}
		if ( !fMotion )
		{
			m_vZoom = v0 ;
			m_aZooms.FreeArray() ;
		}
	}
	//
	// 物理演算合成
	//
	nCount = m_aPhysBlends.GetLength() ;
	if ( nCount > 0 )
	{
		const float32_t *	pPhysBlends = m_aPhysBlends.GetConstArray() ;
		float32_t			w0 = pPhysBlends[0] ;
		bool				fMotion = false ;
		for ( i = 1; i < nCount; i ++ )
		{
			if ( fabs( pPhysBlends[i] - w0 ) > 1.0e-8 )
			{
				fMotion = true ;
				break ;
			}
		}
		if ( !fMotion )
		{
			m_wPhysBlend = w0 ;
			m_aPhysBlends.FreeArray() ;
		}
	}
}



//////////////////////////////////////////////////////////////////////////////
// ポーズ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DModelPose, SObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DModelPose::S3DModelPose( void )
	: m_countRef( 0 )
{
	eslFillMemory( &m_metaInfo, 0, sizeof(MetaInfo) ) ;
	m_pLastPoseTarget = NULL ;
}

S3DModelPose::S3DModelPose( const S3DModelPose& pose )
	: m_ssoaJoint( pose.m_ssoaJoint ),
		m_ssoaMorph( pose.m_ssoaMorph ),
		m_ssoaMeshSel( pose.m_ssoaMeshSel ),
		m_ssoaMaterialSel( pose.m_ssoaMaterialSel ),
		m_metaInfo( pose.m_metaInfo ), m_countRef( 0 )
{
	m_pLastPoseTarget = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DModelPose::~S3DModelPose( void )
{
	ESLAssert( m_countRef == 0 ) ;
}

// 代入
//////////////////////////////////////////////////////////////////////////////
const S3DModelPose& S3DModelPose::operator = ( const S3DModelPose& pose )
{
	m_metaInfo = pose.m_metaInfo ;
	m_ssoaJoint = pose.m_ssoaJoint ;
	m_ssoaMorph = pose.m_ssoaMorph ;
	m_ssoaMeshSel = pose.m_ssoaMeshSel ;
	m_ssoaMaterialSel = pose.m_ssoaMaterialSel ;
	m_pLastPoseTarget = NULL ;
	return	*this ;
}

// リソース削除
//////////////////////////////////////////////////////////////////////////////
void S3DModelPose::ClearAll( void )
{
	SSmartLock<SCriticalSection>	lock( &m_csLock ) ;
	eslFillMemory( &m_metaInfo, 0, sizeof(MetaInfo) ) ;
	m_ssoaJoint.RemoveAll() ;
	m_ssoaMorph.RemoveAll() ;
	m_ssoaMeshSel.RemoveAll() ;
	m_ssoaMaterialSel.RemoveAll() ;
	m_pLastPoseTarget = NULL ;
}

// 禁止状態の要素を削除する
//////////////////////////////////////////////////////////////////////////////
void S3DModelPose::ClearDisabledElement( void )
{
	SSmartLock<SCriticalSection>	lock( &m_csLock ) ;
	ClearDisabledJoint() ;
	ClearDisabledMorphing() ;
	ClearDisabledMeshSelector() ;
	ClearDisabledMaterialSelector() ;
	m_pLastPoseTarget = NULL ;
}

// 参照カウンタ加算
//////////////////////////////////////////////////////////////////////////////
atomic_int_t S3DModelPose::AddRef( void )
{
	ESLAssert( this != NULL ) ;
	return	AtomicAdd( &m_countRef, 1 ) ;
}

// 参照カウンタ減少／解放
//////////////////////////////////////////////////////////////////////////////
atomic_int_t S3DModelPose::ReleaseRef( void )
{
	ESLAssert( this != NULL ) ;
	atomic_int_t	count = AtomicSub( &m_countRef, 1 ) ;
	if ( count <= 0 )
	{
		ESLAssert( count == 0 ) ;
		delete	this ;
		return	0 ;
	}
	return	count ;
}

// 参照カウンタ加算
//////////////////////////////////////////////////////////////////////////////
atomic_int_t S3DModelPose::LockRef( void )
{
	ESLAssert( this != NULL ) ;
	return	AtomicAdd( &m_countRef, 1 ) ;
}

// 参照カウンタ減少
//////////////////////////////////////////////////////////////////////////////
atomic_int_t S3DModelPose::UnlockRef( void )
{
	ESLAssert( this != NULL ) ;
	atomic_int_t	count = AtomicSub( &m_countRef, 1 ) ;
	ESLAssert( count >= 0 ) ;
	return	count ;
}

// スレッド排他アクセス用
//////////////////////////////////////////////////////////////////////////////
void S3DModelPose::Lock( void )
{
	m_csLock.Lock() ;
}

void S3DModelPose::Unlock( void )
{
	m_csLock.Unlock() ;
}

// ポーズファイル読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelPose::LoadPose( const wchar_t * pwszFilePath )
{
	SSmartPointer<SFileInterface>	pFile =
			SFileOpener::DefaultNewOpenFile
				( pwszFilePath, SFileOpener::shareRead ) ;
	if ( pFile == NULL )
	{
		return	sglErrFailed ;
	}
	return	ReadPoseFile( *pFile ) ;
}

SGLError S3DModelPose::ReadPoseFile( SSystem::SFileInterface & file )
{
	SChunkFile	cf ;
	if ( cf.OpenChunkFile( &file ) )
	{
		return	sglErrFailed ;
	}
	if ( cf.GetFileHeader().dwFileID != SChunkFile::fidEGL3DModelPose )
	{
		return	sglErrFailed ;
	}
	SGLError	err ;
	err = ReadPose( cf ) ;
	cf.Close() ;
	return	err ;
}

SGLError S3DModelPose::ReadPose( SSystem::SChunkFile & cf )
{
	ClearAll() ;
	//
	SGLError	err = sglErrSuccess ;
	while ( !cf.DescendChunk() )
	{
		if ( cf.IsEqualCurrentChunkID( "metainfo" ) )
		{
			err = ReadMetaInfoChunk( cf ) ;
		}
		else if ( cf.IsEqualCurrentChunkID( "pose    " ) )
		{
			err = ReadPoseChunk( cf ) ;
		}
		else if ( cf.IsEqualCurrentChunkID( "morphing" ) )
		{
			err = ReadMorphingChunk( cf ) ;
		}
		else if ( cf.IsEqualCurrentChunkID( "mesh_sel" ) )
		{
			err = ReadMeshSelectorChunk( cf ) ;
		}
		else if ( cf.IsEqualCurrentChunkID( "material" ) )
		{
			err = ReadMaterialSelectorChunk( cf ) ;
		}
		else
		{
			err = ReadExtentionChunk( cf ) ;
		}
		cf.AscendChunk() ;
		if ( err )
		{
			break ;
		}
	}
	return	err ;
}

SGLError S3DModelPose::ReadMetaInfoChunk( SSystem::SChunkFile & cf )
{
	if ( cf.Read( &m_metaInfo, sizeof(MetaInfo) ) < sizeof(MetaInfo) )
	{
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

SGLError S3DModelPose::ReadPoseChunk( SSystem::SChunkFile & cf )
{
	//
	// 定義文字列配列
	//
	SObjectArray<SString>	aStrings ;
	if ( cf.DescendChunk( "strings " ) )
	{
		return	sglErrFailed ;
	}
	S3DStdModelLoader::ReadStrings( aStrings, cf ) ;
	cf.AscendChunk() ;
	//
	// 関節配列
	//
	m_ssoaJoint.RemoveAll() ;
	for ( ; ; )
	{
		if ( cf.DescendChunk() )
		{
			break ;
		}
		if ( cf.IsEqualCurrentChunkID( "joint   " ) )
		{
			JointDataHeader	jdHeader ;
			JointInfo		infJoint ;
			cf.Read( &jdHeader, sizeof(JointDataHeader) ) ;
			if ( jdHeader.nFlags & flagDataSize )
			{
				uint32_t	nDataSize = 0 ;
				cf.Read( &nDataSize, sizeof(uint32_t) ) ;
				//
				size_t	nReadBytes = esl_min( nDataSize, sizeof(JointInfo) ) ;
				cf.Read( &infJoint, nReadBytes ) ;
				if ( nDataSize > nReadBytes )
				{
					cf.Seek( nDataSize - nReadBytes,
								SFileInterface::FromCurrent ) ;
				}
			}
			else
			{
				cf.Read( &infJoint, sizeof(JointData1) ) ;
			}
			//
			SString *	pstrID = aStrings.GetAt( jdHeader.iStringID ) ;
			if ( pstrID != NULL )
			{
				JointAnimation *	pja = new JointAnimation ;
				//
				*((JointInfo*)pja) = infJoint ;
				pja->m_fDisabled = ((jdHeader.nFlags & flagDisabled) != 0) ;
				//
				m_ssoaJoint.Add( *pstrID, pja ) ;
				//
				while ( !cf.DescendChunk() )
				{
					AnimationHeader	anihdr ;
					if ( cf.Read
						( &anihdr,
							sizeof(AnimationHeader) ) < sizeof(AnimationHeader) )
					{
						cf.AscendChunk() ;
						continue ;
					}
					if ( cf.IsEqualCurrentChunkID( "animat44" ) )
					{
						pja->m_aMatrixs.SetLength( anihdr.nFrameCount ) ;
						cf.Read
							( pja->m_aMatrixs.GetArray(),
								anihdr.nFrameCount * sizeof(S4DMatrix) ) ;
						pja->m_aMatrixs.FinishArray() ;
					}
					else if ( cf.IsEqualCurrentChunkID( "anirotqt" ) )
					{
						pja->m_aRotations.SetLength( anihdr.nFrameCount ) ;
						cf.Read
							( pja->m_aRotations.GetArray(),
								anihdr.nFrameCount * sizeof(S3DDQuaternion) ) ;
						pja->m_aRotations.FinishArray() ;
					}
					else if ( cf.IsEqualCurrentChunkID( "anitvec3" ) )
					{
						pja->m_aOffsets.SetLength( anihdr.nFrameCount ) ;
						cf.Read
							( pja->m_aOffsets.GetArray(),
								anihdr.nFrameCount * sizeof(S3DDVector) ) ;
						pja->m_aOffsets.FinishArray() ;
					}
					else if ( cf.IsEqualCurrentChunkID( "anizoom3" ) )
					{
						pja->m_aZooms.SetLength( anihdr.nFrameCount ) ;
						cf.Read
							( pja->m_aZooms.GetArray(),
								anihdr.nFrameCount * sizeof(S3DDVector) ) ;
						pja->m_aZooms.FinishArray() ;
					}
					else if ( cf.IsEqualCurrentChunkID( "aniphybl" ) )
					{
						pja->m_aPhysBlends.SetLength( anihdr.nFrameCount ) ;
						cf.Read
							( pja->m_aPhysBlends.GetArray(),
								anihdr.nFrameCount * sizeof(float32_t) ) ;
						pja->m_aPhysBlends.FinishArray() ;
					}
					cf.AscendChunk() ;
				}
			}
		}
		cf.AscendChunk() ;
	}
	return	sglErrSuccess ;
}

SGLError S3DModelPose::ReadMorphingChunk( SSystem::SChunkFile & cf )
{
	//
	// 定義文字列配列
	//
	StringArray	aStrings ;
	if ( cf.DescendChunk( "strings " ) )
	{
		return	sglErrFailed ;
	}
	S3DStdModelLoader::ReadStrings( aStrings, cf ) ;
	cf.AscendChunk() ;
	//
	// モーフィング配列
	//
	m_ssoaMorph.RemoveAll() ;
	for ( ; ; )
	{
		if ( cf.DescendChunk() )
		{
			break ;
		}
		if ( cf.IsEqualCurrentChunkID( "target  " ) )
		{
			MorphData	mdata ;
			cf.Read( &mdata, sizeof(MorphData) ) ;
			//
			SString *	pstrID = aStrings.GetAt( mdata.iStringID ) ;
			SString *	pstrTarget = aStrings.GetAt( mdata.iTargetID ) ;
			if ( (pstrID != NULL) && (pstrTarget != NULL) )
			{
				MorphInfo *	pmi = new MorphInfo ;
				pmi->m_strMorphTarget = *pstrTarget ;
				pmi->m_fDisabled = ((mdata.nFlags & flagDisabled) != 0) ;
				m_ssoaMorph.Add( *pstrID, pmi ) ;
				//
				while ( !cf.DescendChunk() )
				{
					if ( cf.IsEqualCurrentChunkID( "trgtmesh" ) )
					{
						uint32_t	nTargetCount = 0 ;
						if ( cf.Read
							( &nTargetCount, sizeof(uint32_t) ) < sizeof(uint32_t) )
						{
							cf.AscendChunk() ;
							continue ;
						}
						for ( size_t j = 0; j < nTargetCount; j ++ )
						{
							uint32_t	iTargetID ;
							if ( cf.Read( &iTargetID, sizeof(uint32_t) ) == sizeof(uint32_t) )
							{
								SString *	pstrID =
										aStrings.GetAt( (size_t) iTargetID ) ;
								if ( pstrID != NULL )
								{
									pmi->m_aTargetIDs.Add( new SString( *pstrID ) ) ;
								}
								else
								{
									pmi->m_aTargetIDs.Add( new SString() ) ;
								}
							}
						}
					}
					else if ( cf.IsEqualCurrentChunkID( "animesh " ) )
					{
						AnimationHeader	anihdr ;
						if ( cf.Read
							( &anihdr, sizeof(AnimationHeader) )
											< sizeof(AnimationHeader) )
						{
							cf.AscendChunk() ;
							continue ;
						}
						size_t	nDataCount =
									anihdr.nFrameCount
										* pmi->m_aTargetIDs.GetLength() ;
						pmi->m_aAnimation.SetLength( nDataCount ) ;
						cf.Read
							( pmi->m_aAnimation.GetArray(),
									nDataCount * sizeof(float32_t) ) ;
						pmi->m_aAnimation.FinishArray() ;
					}
					cf.AscendChunk() ;
				}
			}
		}
		cf.AscendChunk() ;
	}
	return	sglErrSuccess ;
}

SGLError S3DModelPose::ReadMeshSelectorChunk( SSystem::SChunkFile & cf )
{
	//
	// 定義文字列配列
	//
	SObjectArray<SString>	aStrings ;
	if ( cf.DescendChunk( "strings " ) )
	{
		return	sglErrFailed ;
	}
	S3DStdModelLoader::ReadStrings( aStrings, cf ) ;
	cf.AscendChunk() ;
	//
	// メッシュ配列
	//
	m_ssoaMeshSel.RemoveAll() ;
	for ( ; ; )
	{
		if ( cf.DescendChunk() )
		{
			break ;
		}
		if ( cf.IsEqualCurrentChunkID( "mesh_swt" ) )
		{
			MeshSelectorData	msdata ;
			cf.Read( &msdata, sizeof(MeshSelectorData) ) ;
			//
			SString *	pstrID = aStrings.GetAt( msdata.iStringID ) ;
			if ( pstrID != NULL )
			{
				MeshSelector *	pms = new MeshSelector ;
				pms->m_fDisabled = ((msdata.nVisible & 0x80) != 0) ;
				pms->m_flagVisible = ((msdata.nVisible & 0x01) != 0) ;
				//
				if ( msdata.nSeqFrames > 0 )
				{
					cf.Read
						( pms->m_aVisibles.GetArray(msdata.nSeqFrames),
								msdata.nSeqFrames * sizeof(uint8_t) ) ;
					pms->m_aVisibles.FinishArray() ;
				}
				m_ssoaMeshSel.Add( *pstrID, pms ) ;
			}
		}
		cf.AscendChunk() ;
	}
	return	sglErrSuccess ;
}

SGLError S3DModelPose::ReadMaterialSelectorChunk( SSystem::SChunkFile & cf )
{
	//
	// 定義文字列配列
	//
	SObjectArray<SString>	aStrings ;
	if ( cf.DescendChunk( "strings " ) )
	{
		return	sglErrFailed ;
	}
	S3DStdModelLoader::ReadStrings( aStrings, cf ) ;
	cf.AscendChunk() ;
	//
	// マテリアル配列
	//
	m_ssoaMaterialSel.RemoveAll() ;
	for ( ; ; )
	{
		if ( cf.DescendChunk() )
		{
			break ;
		}
		if ( cf.IsEqualCurrentChunkID( "mtrl_swt" ) )
		{
			MaterialSelectorData	msdata ;
			cf.Read( &msdata, sizeof(MaterialSelectorData) ) ;
			//
			SString *	pstrID = aStrings.GetAt( msdata.iStringID ) ;
			if ( pstrID != NULL )
			{
				MaterialSelector *	pms = new MaterialSelector ;
				pms->m_fDisabled = ((msdata.nFlags & 0x80) != 0) ;
				//
				SArray<uint32_t>	aMaterialIDs ;
				uint32_t *			pMaterialIDs =
						aMaterialIDs.GetArray( msdata.nMaterialCount ) ;
				cf.Read( pMaterialIDs, msdata.nMaterialCount * sizeof(uint32_t) ) ;
				aMaterialIDs.FinishArray() ;
				//
				for ( size_t i = 0; i < msdata.nMaterialCount; i ++ )
				{
					SString *	pstrMaterialID = aStrings.GetAt( pMaterialIDs[i] ) ;
					if ( pstrMaterialID != NULL )
					{
						pms->m_aMaterialIDs.Add( new SString( *pstrMaterialID ) ) ;
					}
					else
					{
						pms->m_aMaterialIDs.Add( new SString() ) ;
					}
				}
				//
				cf.Read( pms->m_aAnimation.GetArray(msdata.nSeqFrames),
								msdata.nSeqFrames * sizeof(uint32_t) ) ;
				pms->m_aAnimation.FinishArray() ;
				//
				m_ssoaMaterialSel.Add( *pstrID, pms ) ;
			}
		}
		cf.AscendChunk() ;
	}
	return	sglErrSuccess ;
}

SGLError S3DModelPose::ReadExtentionChunk( SSystem::SChunkFile & cf )
{
	return	sglErrSuccess ;
}

// ポーズファイル読み込み (XML)
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelPose::LoadPoseXML( const wchar_t * pwszFilePath )
{
	SXMLDocument	xmlDoc ;
	if ( xmlDoc.LoadDocument( pwszFilePath, xmlDoc ) )
	{
		return	sglErrFailed ;
	}
	SXMLDocument *	pxmlPose = xmlDoc.GetElementTagAs( L"pose" ) ;
	if ( pxmlPose == NULL )
	{
		return	sglErrFailed ;
	}
	return	ParsePoseXML( *pxmlPose ) ;
}

SGLError S3DModelPose::ParsePoseXML( const SSystem::SXMLDocument & xmlPoseTag )
{
	ClearAll() ;
	//
	for ( size_t i = 0; i < xmlPoseTag.GetElementsCount(); i ++ )
	{
		SXMLDocument *	pxmlTag = xmlPoseTag.GetElementAt( i ) ;
		if ( pxmlTag == NULL )
		{
			continue ;
		}
		if ( pxmlTag->GetTag() == L"meta" )
		{
			ParseMetaTag( *pxmlTag ) ;
		}
		else if ( pxmlTag->GetTag() == L"joint" )
		{
			ParseJointTag( *pxmlTag ) ;
		}
		else if ( pxmlTag->GetTag() == L"morphing" )
		{
			ParseMorphingTag( *pxmlTag ) ;
		}
		else if ( pxmlTag->GetTag() == L"mesh_selector" )
		{
			ParseMeshSelectorTag( *pxmlTag ) ;
		}
		else if ( pxmlTag->GetTag() == L"material_selector" )
		{
			ParseMaterialSelectorTag( *pxmlTag ) ;
		}
	}
	return	sglErrSuccess ;
}

SGLError S3DModelPose::ParseMetaTag( const SSystem::SXMLDocument & xmlTag )
{
	m_metaInfo.msecDuration =
		(uint32_t) xmlTag.GetAttrIntegerAs( L"duration" ) ;
	m_metaInfo.fxFrameRatio =
		(uint32_t) xmlTag.GetAttrIntegerAs( L"frame_ratio" ) ;
	return	sglErrSuccess ;
}

SGLError S3DModelPose::ParseJointTag( const SSystem::SXMLDocument & xmlTag )
{
	SString *	pstrID = xmlTag.GetAttributeAs( L"id" ) ;
	if ( pstrID == NULL )
	{
		return	sglErrFailed ;
	}
	JointInfo	jinf ;
	SString *	pstrQuaternion =
					xmlTag.GetTextElementAs( L"quaternion" ) ;
	if ( pstrQuaternion != NULL )
	{
		//
		// 回転クォータニオン表現
		//
		SStringParser	sparsQ ;
		sparsQ.AttachString( *pstrQuaternion ) ;
		//
		double	q[4] ;
		if ( sparsQ.ParseNumberArray( &q[0], 4 ) < 4 )
		{
			ESLTrace( "failed to parse quaternion.\n" ) ;
			return	sglErrFailed ;
		}
		jinf.m_qRotation.q[0] = q[0] ;
		jinf.m_qRotation.q[1] = q[1] ;
		jinf.m_qRotation.q[2] = q[2] ;
		jinf.m_qRotation.q[3] = q[3] ;
	}
	else
	{
		SString *	pstrMatrix =
						xmlTag.GetTextElementAs( L"matrix3" ) ;
		if ( pstrMatrix != NULL )
		{
			//
			// 回転行列表現
			//
			SStringParser	sparsM ;
			sparsM.AttachString( *pstrMatrix ) ;
			//
			double	m[3][3] ;
			if ( sparsM.ParseNumberArray( &m[0][0], 9 ) < 9 )
			{
				ESLTrace( "failed to parse matrix3.\n" ) ;
				return	sglErrFailed ;
			}
			S3DDMatrix	mat ;
			for ( int j = 0; j < 3; j ++ )
			{
				mat.m[j][0] = m[j][0] ;
				mat.m[j][1] = m[j][1] ;
				mat.m[j][2] = m[j][2] ;
			}
			jinf.m_qRotation.FromMatrix( mat ) ;
		}
		else
		{
			//
			// 回転オイラー角表現
			//
			S3DDMatrix	mat( 1, 0, 0,  0, 1, 0,  0, 0, 1 ) ;
			size_t	iLastTag = 0 ;
			for ( ; ; )
			{
				ssize_t	iAngleTag =
					xmlTag.FindElementTag( L"euler_angles", iLastTag ) ;
				if ( iAngleTag < 0 )
				{
					break ;
				}
				iLastTag = (size_t) iAngleTag + 1 ;
				//
				SXMLDocument *	pxmlAngle =
						xmlTag.GetElementAt( (size_t) iAngleTag ) ;
				ESLAssert( pxmlAngle != NULL ) ;
				ESLAssert( pxmlAngle->GetTag() == L"euler_angles" ) ;
				double	rad ;
				rad = pxmlAngle->GetAttrRealAs( L"z", 0.0 ) * PI / 180.0 ;
				mat.RevolveOnZ( sin(rad), cos(rad) ) ;
				//
				rad = pxmlAngle->GetAttrRealAs( L"y", 0.0 ) * PI / 180.0 ;
				mat.RevolveOnY( sin(rad), cos(rad) ) ;
				//
				rad = pxmlAngle->GetAttrRealAs( L"x", 0.0 ) * PI / 180.0 ;
				mat.RevolveOnX( sin(rad), cos(rad) ) ;
			}
			jinf.m_qRotation.FromMatrix( mat ) ;
		}
	}
	SString *	pstrOffset =
					xmlTag.GetTextElementAs( L"offset" ) ;
	if ( pstrOffset != NULL )
	{
		SStringParser	sparsOffset ;
		sparsOffset.AttachString( *pstrOffset ) ;
		//
		double	v[3] ;
		if ( sparsOffset.ParseNumberArray( &v[0], 3 ) == 3 )
		{
			jinf.m_vOffset.x = v[0] ;
			jinf.m_vOffset.y = v[1] ;
			jinf.m_vOffset.z = v[2] ;
		}
	}
	SString *	pstrHandle =
					xmlTag.GetTextElementAs( L"bone_handle" ) ;
	if ( pstrHandle != NULL )
	{
		SStringParser	sparsHandle ;
		sparsHandle.AttachString( *pstrHandle ) ;
		//
		double	v[3] ;
		if ( sparsHandle.ParseNumberArray( &v[0], 3 ) == 3 )
		{
			jinf.m_vHandle.x = v[0] ;
			jinf.m_vHandle.y = v[1] ;
			jinf.m_vHandle.z = v[2] ;
		}
	}
	SXMLDocument *	pxmlZRot =
					xmlTag.GetElementTagAs( L"z_rotation" ) ;
	if ( pxmlZRot != NULL )
	{
		jinf.m_zRotation = pxmlZRot->GetAttrRealAs( L"angle" ) ;
	}
	SXMLDocument *	pxmlPhysBlend =
					xmlTag.GetElementTagAs( L"phys_blend" ) ;
	if ( pxmlPhysBlend != NULL )
	{
		jinf.m_wPhysBlend = pxmlPhysBlend->GetAttrRealAs( L"weight" ) ;
	}
	SXMLDocument *	pxmlOrgMatrix =
					xmlTag.GetElementTagAs( L"original_matrix" ) ;
	if ( pxmlOrgMatrix != NULL )
	{
		SString *	pstrMatrix = pxmlOrgMatrix->GetTextElement() ;
		if ( pstrMatrix != NULL )
		{
			SStringParser	sparsMatrix ;
			double	mat4[4][4] ;
			sparsMatrix.AttachString( *pstrMatrix ) ;
			if ( sparsMatrix.ParseNumberArray( &mat4[0][0], 4 * 4 ) == 4 * 4 )
			{
				for ( int j = 0; j < 4; j ++ )
				{
					jinf.m_mat4OrgLocal.m[j][0] = (float32_t) mat4[j][0] ;
					jinf.m_mat4OrgLocal.m[j][1] = (float32_t) mat4[j][1] ;
					jinf.m_mat4OrgLocal.m[j][2] = (float32_t) mat4[j][2] ;
					jinf.m_mat4OrgLocal.m[j][3] = (float32_t) mat4[j][3] ;
				}
				jinf.m_nExFlags |= flagHaveOrgMatrix ;
			}
		}
	}
	JointAnimation *	pja = new JointAnimation ;
	*((JointInfo*)pja) = jinf ;
	pja->m_fDisabled = (xmlTag.GetAttrIntegerAs( L"disabled" ) != 0) ;
	m_ssoaJoint.Add( *pstrID, pja ) ;
	//
	SXMLDocument *	pxmlMatrixAni =
		xmlTag.GetElementTagAs( L"matrix44_animation" ) ;
	if ( pxmlMatrixAni != NULL )
	{
		SString *	pstrSeq = pxmlMatrixAni->GetTextElement() ;
		if ( pstrSeq != NULL )
		{
			size_t	nFrames =
				(size_t) pxmlMatrixAni->GetAttrIntegerAs( L"frame_count" ) ;
			SArray<double>	aBuf ;
			double *	pBuf = aBuf.GetArray( nFrames * 4 * 4 ) ;
			//
			SStringParser	sparsQ ;
			sparsQ.AttachString( *pstrSeq ) ;
			sparsQ.ParseNumberArray( pBuf, nFrames * 4 * 4 ) ;
			aBuf.FinishArray() ;
			//
			S4DMatrix *	pmatAni = pja->m_aMatrixs.GetArray( nFrames ) ;
			for ( size_t i = 0; i < nFrames; i ++ )
			{
				S4DMatrix&	m4 = pmatAni[i] ;
				double *	pMat = pBuf + (i * 4 * 4) ;
				for ( int j = 0; j < 4; j ++ )
				{
					m4.m[j][0] = (float32_t) pMat[0] ;
					m4.m[j][1] = (float32_t) pMat[1] ;
					m4.m[j][2] = (float32_t) pMat[2] ;
					m4.m[j][3] = (float32_t) pMat[3] ;
					pMat += 4 ;
				}
			}
			pja->m_aMatrixs.FinishArray() ;
		}
	}
	SXMLDocument *	pxmlQuatAni =
		xmlTag.GetElementTagAs( L"quaternion_animation" ) ;
	if ( pxmlQuatAni != NULL )
	{
		SString *	pstrSeq = pxmlQuatAni->GetTextElement() ;
		if ( pstrSeq != NULL )
		{
			size_t	nFrames =
				(size_t) pxmlQuatAni->GetAttrIntegerAs( L"frame_count" ) ;
			SArray<double>	aBuf ;
			double *	pBuf = aBuf.GetArray( nFrames * 4 ) ;
			//
			SStringParser	sparsQ ;
			sparsQ.AttachString( *pstrSeq ) ;
			sparsQ.ParseNumberArray( pBuf, nFrames * 4 ) ;
			aBuf.FinishArray() ;
			//
			S3DDQuaternion *
				paAni = pja->m_aRotations.GetArray( nFrames ) ;
			for ( size_t i = 0; i < nFrames; i ++ )
			{
				paAni[i].q[0] = pBuf[i * 4] ;
				paAni[i].q[1] = pBuf[i * 4 + 1] ;
				paAni[i].q[2] = pBuf[i * 4 + 2] ;
				paAni[i].q[3] = pBuf[i * 4 + 3] ;
			}
			pja->m_aRotations.FinishArray() ;
		}
	}
	SXMLDocument *	pxmlVec3Ani =
		xmlTag.GetElementTagAs( L"vector3_animation" ) ;
	if ( pxmlVec3Ani != NULL )
	{
		SString *	pstrSeq = pxmlVec3Ani->GetTextElement() ;
		if ( pstrSeq != NULL )
		{
			size_t	nFrames =
				(size_t) pxmlVec3Ani->GetAttrIntegerAs( L"frame_count" ) ;
			SArray<double>	aBuf ;
			double *	pBuf = aBuf.GetArray( nFrames * 3 ) ;
			//
			SStringParser	sparsQ ;
			sparsQ.AttachString( *pstrSeq ) ;
			sparsQ.ParseNumberArray( pBuf, nFrames * 3 ) ;
			aBuf.FinishArray() ;
			//
			S3DDVector *
				pvAni = pja->m_aOffsets.GetArray( nFrames ) ;
			for ( size_t i = 0; i < nFrames; i ++ )
			{
				pvAni[i].x = pBuf[i * 3] ;
				pvAni[i].y = pBuf[i * 3 + 1] ;
				pvAni[i].z = pBuf[i * 3 + 2] ;
			}
			pja->m_aOffsets.FinishArray() ;
		}
	}
	SXMLDocument *	pxmlZoomAni =
		xmlTag.GetElementTagAs( L"zoom_animation" ) ;
	if ( pxmlZoomAni != NULL )
	{
		SString *	pstrSeq = pxmlZoomAni->GetTextElement() ;
		if ( pstrSeq != NULL )
		{
			size_t	nFrames =
				(size_t) pxmlZoomAni->GetAttrIntegerAs( L"frame_count" ) ;
			SArray<double>	aBuf ;
			double *	pBuf = aBuf.GetArray( nFrames * 3 ) ;
			//
			SStringParser	sparsQ ;
			sparsQ.AttachString( *pstrSeq ) ;
			sparsQ.ParseNumberArray( pBuf, nFrames * 3 ) ;
			aBuf.FinishArray() ;
			//
			S3DDVector *
				pvAni = pja->m_aZooms.GetArray( nFrames ) ;
			for ( size_t i = 0; i < nFrames; i ++ )
			{
				pvAni[i].x = pBuf[i * 3] ;
				pvAni[i].y = pBuf[i * 3 + 1] ;
				pvAni[i].z = pBuf[i * 3 + 2] ;
			}
			pja->m_aZooms.FinishArray() ;
		}
	}
	SXMLDocument *	pxmlPhyBlendAni =
		xmlTag.GetElementTagAs( L"phy_blend_animation" ) ;
	if ( pxmlPhyBlendAni != NULL )
	{
		SString *	pstrSeq = pxmlPhyBlendAni->GetTextElement() ;
		if ( pstrSeq != NULL )
		{
			size_t	nFrames =
				(size_t) pxmlPhyBlendAni->GetAttrIntegerAs( L"frame_count" ) ;
			SArray<double>	aBuf ;
			double *	pBuf = aBuf.GetArray( nFrames ) ;
			//
			SStringParser	sparsQ ;
			sparsQ.AttachString( *pstrSeq ) ;
			sparsQ.ParseNumberArray( pBuf, nFrames ) ;
			aBuf.FinishArray() ;
			//
			float32_t *
				pwAni = pja->m_aPhysBlends.GetArray( nFrames ) ;
			for ( size_t i = 0; i < nFrames; i ++ )
			{
				pwAni[i] = (float32_t) pBuf[i] ;
			}
			pja->m_aPhysBlends.FinishArray() ;
		}
	}
	return	sglErrSuccess ;
}

SGLError S3DModelPose::ParseMorphingTag( const SSystem::SXMLDocument & xmlTag )
{
	SString *	pstrID = xmlTag.GetAttributeAs( L"id" ) ;
	if ( pstrID == NULL )
	{
		return	sglErrFailed ;
	}
	MorphInfo *	pmi = new MorphInfo ;
	pmi->m_strMorphTarget = xmlTag.GetAttrStringAs( L"target" ) ;
	pmi->m_fDisabled = (xmlTag.GetAttrIntegerAs( L"disabled" ) != 0) ;
	m_ssoaMorph.Add( *pstrID, pmi ) ;
	//
	SXMLDocument *	pxmlTarget = xmlTag.GetElementTagAs( L"target" ) ;
	if ( pxmlTarget != NULL )
	{
		for ( size_t i = 0; i < pxmlTarget->GetElementsCount(); i ++ )
		{
			SXMLDocument *	pxmlEntry = pxmlTarget->GetElementAt( i ) ;
			if ( (pxmlEntry == NULL)
				|| (pxmlEntry->GetTag() != L"entry") )
			{
				continue ;
			}
			pmi->m_aTargetIDs.Add
				( new SString( pxmlEntry->GetAttrStringAs( L"id" ) ) ) ;
		}
	}
	SXMLDocument *	pxmlAnim = xmlTag.GetElementTagAs( L"animation" ) ;
	if ( pxmlAnim != NULL )
	{
		SString *	pstrAniText = pxmlAnim->GetTextElement() ;
		if ( pstrAniText != NULL )
		{
			size_t	nFrameCount =
				(size_t) pxmlAnim->GetAttrIntegerAs( L"frame_count" ) ;
			size_t	nWeightCount = nFrameCount
										* pmi->m_aTargetIDs.GetLength() ;
			//
			SArray<double>	bufWeights ;
			double *		pWeights = bufWeights.GetArray ( nWeightCount ) ;
			SStringParser	sparsAni ;
			sparsAni.AttachString( *pstrAniText ) ;
			sparsAni.ParseNumberArray( pWeights, nWeightCount ) ;
			bufWeights.FinishArray() ;
			//
			float32_t *	pAniWeights =
							pmi->m_aAnimation.GetArray( nWeightCount ) ;
			for ( size_t i = 0; i < nWeightCount; i ++ )
			{
				pAniWeights[i] = (float32_t) pWeights[i] ;
			}
			pmi->m_aAnimation.FinishArray() ;
		}
	}
	return	sglErrSuccess ;
}

SGLError S3DModelPose::ParseMeshSelectorTag( const SSystem::SXMLDocument & xmlTag )
{
	SString *	pstrID = xmlTag.GetAttributeAs( L"id" ) ;
	if ( pstrID == NULL )
	{
		return	sglErrFailed ;
	}
	MeshSelector *	pms = new MeshSelector ;
	pms->m_fDisabled = (xmlTag.GetAttrIntegerAs( L"disabled" ) != 0) ;
	const SString *	pstrVisible = xmlTag.GetAttributeAs( L"visible" ) ;
	if ( pstrVisible != NULL )
	{
		pms->m_flagVisible = (*pstrVisible == L"true") ;
	}
	//
	const SString *	pstrSeq = xmlTag.GetAttributeAs( L"sequence" ) ;
	if ( pstrSeq != NULL )
	{
		SStringParser	sparsSeq ;
		sparsSeq.AttachString( *pstrSeq ) ;
		while ( sparsSeq.PassSpace() )
		{
			pms->m_aVisibles.Add
				( (uint8_t) (sparsSeq.NextInteger() != 0) ) ;
		}
	}
	m_ssoaMeshSel.Add( *pstrID, pms ) ;
	return	sglErrSuccess ;
}

SGLError S3DModelPose::ParseMaterialSelectorTag( const SSystem::SXMLDocument & xmlTag )
{
	SString *	pstrID = xmlTag.GetAttributeAs( L"id" ) ;
	if ( pstrID == NULL )
	{
		return	sglErrFailed ;
	}
	MaterialSelector *	pms = new MaterialSelector ;
	pms->m_fDisabled = (xmlTag.GetAttrIntegerAs( L"disabled" ) != 0) ;
	//
	const SXMLDocument *
			pxmlMaterialIDs = xmlTag.GetElementTagAs( L"material_ids" ) ;
	if ( pxmlMaterialIDs != NULL )
	{
		for ( size_t i = 0; i < pxmlMaterialIDs->GetElementsCount(); i ++ )
		{
			const SXMLDocument *	pxmlTag = pxmlMaterialIDs->GetElementAt( i ) ;
			if ( (pxmlTag == NULL)
				|| (pxmlTag->GetTag() != L"entry") )
			{
				continue ;
			}
			pms->m_aMaterialIDs.Add
				( new SString( pxmlTag->GetAttrStringAs( L"id" ) ) ) ;
		}
	}
	//
	const SString *	pstrSeq = xmlTag.GetAttributeAs( L"sequence" ) ;
	if ( pstrSeq != NULL )
	{
		SStringParser	sparsSeq ;
		sparsSeq.AttachString( *pstrSeq ) ;
		while ( sparsSeq.PassSpace() )
		{
			pms->m_aAnimation.Add
				( (uint32_t) (sparsSeq.NextInteger() != 0) ) ;
		}
	}
	m_ssoaMaterialSel.Add( *pstrID, pms ) ;
	return	sglErrSuccess ;
}

// ポーズファイル書き出し (バイナリ)
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelPose::SavePose( const wchar_t * pwszFilePath )
{
	SSmartPointer<SFileInterface>	pFile =
			SFileOpener::DefaultNewOpenFile
				( pwszFilePath, SFileOpener::modeCreate ) ;
	if ( pFile == NULL )
	{
		return	sglErrFailed ;
	}
	return	WritePoseFile( *pFile ) ;
}

SGLError S3DModelPose::WritePoseFile( SSystem::SFileInterface & file )
{
	SChunkFile::FILE_HEADER	fhdr ;
	fhdr.SetHeaderInfo
		( SChunkFile::fidEGL3DModelPose, "EntisGLS4 pose binary file" ) ;
	//
	SChunkFile	cf ;
	if ( cf.OpenChunkFile
		( &file, false, SFileOpener::modeCreate, &fhdr ) )
	{
		return	sglErrFailed ;
	}
	SGLError	err ;
	err = WritePose( cf ) ;
	cf.Close() ;
	return	err ;
}

SGLError S3DModelPose::WritePose( SSystem::SChunkFile & cf )
{
	if ( cf.DescendChunk( "metainfo" ) )
	{
		return	sglErrFailed ;
	}
	SGLError	err = WriteMetaInfoChunk( cf ) ;
	cf.AscendChunk() ;
	if ( err )
	{
		return	err ;
	}
	//
	if ( cf.DescendChunk( "pose    " ) )
	{
		return	sglErrFailed ;
	}
	err = WritePoseChunk( cf ) ;
	cf.AscendChunk() ;
	if ( err )
	{
		return	err ;
	}
	//
	if ( cf.DescendChunk( "morphing" ) )
	{
		return	sglErrFailed ;
	}
	err = WriteMorphingChunk( cf ) ;
	cf.AscendChunk() ;
	//
	if ( cf.DescendChunk( "mesh_sel" ) )
	{
		return	sglErrFailed ;
	}
	err = WriteMeshSelectorChunk( cf ) ;
	cf.AscendChunk() ;
	//
	if ( cf.DescendChunk( "material" ) )
	{
		return	sglErrFailed ;
	}
	err = WriteMaterialSelectorChunk( cf ) ;
	cf.AscendChunk() ;
	//
	return	err ;
}

SGLError S3DModelPose::WriteMetaInfoChunk( SSystem::SChunkFile & cf )
{
	cf.Write( &m_metaInfo, sizeof(MetaInfo) ) ;
	return	sglErrSuccess ;
}

SGLError S3DModelPose::WritePoseChunk( SSystem::SChunkFile & cf )
{
	//
	// 定義文字列配列
	//
	SObjectArray<SString>	aStrings ;
	size_t	i ;
	size_t	nJoints = m_ssoaJoint.GetLength() ;
	for ( i = 0; i < nJoints; i ++ )
	{
		const SString *	pstrTag = m_ssoaJoint.GetTagAt( i ) ;
		ESLAssert( pstrTag != NULL ) ;
		if ( pstrTag != NULL )
		{
			aStrings.Add( new SString( *pstrTag ) ) ;
		}
	}
	cf.DescendChunk( "strings " ) ;
	S3DStdModelSaver::WriteStrings( cf, aStrings ) ;
	cf.AscendChunk() ;
	//
	// 関節配列
	//
	for ( i = 0; i < nJoints; i ++ )
	{
		JointAnimation *	pja = m_ssoaJoint.GetAt( i ) ;
		ESLAssert( pja != NULL ) ;
		if ( pja != NULL )
		{
			JointDataHeader	jdHeader ;
			eslFillMemory( &jdHeader, 0, sizeof(JointDataHeader) ) ;
			jdHeader.iStringID = (uint32_t) i ;
			jdHeader.nFlags |= pja->m_fDisabled ? flagDisabled : 0 ;
			jdHeader.nFlags |= flagDataSize ;
			pja->m_qRotation.ToMatrix( jdHeader.matRotation ) ;
			//
			cf.DescendChunk( "joint   " ) ;
			cf.Write( &jdHeader, sizeof(JointDataHeader) ) ;
			//
			uint32_t	nJointInfoSize = sizeof(JointInfo) ;
			cf.Write( &nJointInfoSize, sizeof(uint32_t) ) ;
			cf.Write( (JointInfo*) pja, sizeof(JointInfo) ) ;
			//
			if ( pja->m_aMatrixs.GetLength() > 0 )
			{
				cf.DescendChunk( "animat44" ) ;
				AnimationHeader	anihdr ;
				eslFillMemory( &anihdr, 0, sizeof(AnimationHeader) ) ;
				anihdr.nFrameCount = (uint32_t) pja->m_aMatrixs.GetLength() ;
				cf.Write( &anihdr, sizeof(AnimationHeader) ) ;
				cf.Write
					( pja->m_aMatrixs.GetConstArray(),
						pja->m_aMatrixs.GetLength() * sizeof(S4DMatrix) ) ;
				cf.AscendChunk() ;
			}
			//
			if ( pja->m_aRotations.GetLength() > 0 )
			{
				cf.DescendChunk( "anirotqt" ) ;
				AnimationHeader	anihdr ;
				eslFillMemory( &anihdr, 0, sizeof(AnimationHeader) ) ;
				anihdr.nFrameCount = (uint32_t) pja->m_aRotations.GetLength() ;
				cf.Write( &anihdr, sizeof(AnimationHeader) ) ;
				cf.Write
					( pja->m_aRotations.GetConstArray(),
						pja->m_aRotations.GetLength() * sizeof(S3DDQuaternion) ) ;
				cf.AscendChunk() ;
			}
			//
			if ( pja->m_aOffsets.GetLength() > 0 )
			{
				cf.DescendChunk( "anitvec3" ) ;
				AnimationHeader	anihdr ;
				eslFillMemory( &anihdr, 0, sizeof(AnimationHeader) ) ;
				anihdr.nFrameCount = (uint32_t) pja->m_aOffsets.GetLength() ;
				cf.Write( &anihdr, sizeof(AnimationHeader) ) ;
				cf.Write
					( pja->m_aOffsets.GetConstArray(),
						pja->m_aOffsets.GetLength() * sizeof(S3DDVector) ) ;
				cf.AscendChunk() ;
			}
			//
			if ( pja->m_aZooms.GetLength() > 0 )
			{
				cf.DescendChunk( "anizoom3" ) ;
				AnimationHeader	anihdr ;
				eslFillMemory( &anihdr, 0, sizeof(AnimationHeader) ) ;
				anihdr.nFrameCount = (uint32_t) pja->m_aZooms.GetLength() ;
				cf.Write( &anihdr, sizeof(AnimationHeader) ) ;
				cf.Write
					( pja->m_aZooms.GetConstArray(),
						pja->m_aZooms.GetLength() * sizeof(S3DDVector) ) ;
				cf.AscendChunk() ;
			}
			//
			if ( pja->m_aPhysBlends.GetLength() > 0 )
			{
				cf.DescendChunk( "aniphybl" ) ;
				AnimationHeader	anihdr ;
				eslFillMemory( &anihdr, 0, sizeof(AnimationHeader) ) ;
				anihdr.nFrameCount = (uint32_t) pja->m_aPhysBlends.GetLength() ;
				cf.Write( &anihdr, sizeof(AnimationHeader) ) ;
				cf.Write
					( pja->m_aPhysBlends.GetConstArray(),
						pja->m_aPhysBlends.GetLength() * sizeof(float32_t) ) ;
				cf.AscendChunk() ;
			}
			//
			cf.AscendChunk() ;
		}
	}
	return	sglErrSuccess ;
}

SGLError S3DModelPose::WriteMorphingChunk( SSystem::SChunkFile & cf )
{
	//
	// 定義文字列配列
	//
	StringArray	aStrings ;
	size_t	i ;
	size_t	nMorphs = m_ssoaMorph.GetLength() ;
	for ( i = 0; i < nMorphs; i ++ )
	{
		const SString *	pstrTag = m_ssoaMorph.GetTagAt( i ) ;
		ESLAssert( pstrTag != NULL ) ;
		if ( pstrTag != NULL )
		{
			aStrings.AllocateString( *pstrTag ) ;
		}
		MorphInfo *	pmi = m_ssoaMorph.GetAt( i ) ;
		ESLAssert( pmi != NULL ) ;
		if ( pmi != NULL )
		{
			aStrings.AllocateString( pmi->m_strMorphTarget ) ;
			//
			for ( size_t j = 0; j < pmi->m_aTargetIDs.GetLength(); j ++ )
			{
				SString *	pstrID = pmi->m_aTargetIDs.GetAt( j ) ;
				ESLAssert( pstrID != NULL ) ;
				if ( pstrID != NULL )
				{
					aStrings.AllocateString( *pstrID ) ;
				}
			}
		}
	}
	cf.DescendChunk( "strings " ) ;
	S3DStdModelSaver::WriteStrings( cf, aStrings ) ;
	cf.AscendChunk() ;
	//
	// モーフィング配列
	//
	for ( i = 0; i < nMorphs; i ++ )
	{
		const SString *	pstrTag = m_ssoaMorph.GetTagAt( i ) ;
		ESLAssert( pstrTag != NULL ) ;
		MorphInfo *	pmi = m_ssoaMorph.GetAt( i ) ;
		ESLAssert( pmi != NULL ) ;
		if ( (pstrTag != NULL) && (pmi != NULL) )
		{
			MorphData	mdata ;
			eslFillMemory( &mdata, 0, sizeof(MorphData) ) ;
			mdata.iStringID = aStrings.AllocateString( *pstrTag ) ;
			mdata.iTargetID =
				aStrings.AllocateString( pmi->m_strMorphTarget ) ;
			mdata.nFlags |= pmi->m_fDisabled ? flagDisabled : 0 ;
			//
			cf.DescendChunk( "target  " ) ;
			cf.Write( &mdata, sizeof(MorphData) ) ;
			//
			uint32_t	nTargetCount =
							(uint32_t) pmi->m_aTargetIDs.GetLength() ;
			if ( pmi->m_aTargetIDs.GetLength() > 0 )
			{
				cf.DescendChunk( "trgtmesh" ) ;
				cf.Write( &nTargetCount, sizeof(uint32_t) ) ;
				for ( size_t j = 0; j < nTargetCount; j ++ )
				{
					SString *	pstrID = pmi->m_aTargetIDs.GetAt( (size_t) j ) ;
					uint32_t	iMesh = 0 ;
					if ( pstrID != NULL )
					{
						iMesh = aStrings.AllocateString( *pstrID ) ;
					}
					cf.Write( &iMesh, sizeof(uint32_t) ) ;
				}
				cf.AscendChunk() ;
			}
			if ( pmi->m_aAnimation.GetLength() > 0 )
			{
				cf.DescendChunk( "animesh " ) ;
				AnimationHeader	anihdr ;
				eslFillMemory( &anihdr, 0, sizeof(AnimationHeader) ) ;
				anihdr.nFrameCount =
							(uint32_t) pmi->m_aAnimation.GetLength() ;
				if ( nTargetCount > 0 )
				{
					anihdr.nFrameCount /= nTargetCount ;
				}
				cf.Write( &anihdr, sizeof(AnimationHeader) ) ;
				cf.Write
					( pmi->m_aAnimation.GetConstArray(),
						pmi->m_aAnimation.GetLength() * sizeof(float32_t) ) ;
				cf.AscendChunk() ;
			}
			//
			cf.AscendChunk() ;
		}
	}
	return	sglErrSuccess ;
}

SGLError S3DModelPose::WriteMeshSelectorChunk( SSystem::SChunkFile & cf )
{
	//
	// 定義文字列配列
	//
	SObjectArray<SString>	aStrings ;
	size_t	i ;
	size_t	nMeshSels = m_ssoaMeshSel.GetLength() ;
	for ( i = 0; i < nMeshSels; i ++ )
	{
		const SString *	pstrTag = m_ssoaMeshSel.GetTagAt( i ) ;
		ESLAssert( pstrTag != NULL ) ;
		if ( pstrTag != NULL )
		{
			aStrings.Add( new SString( *pstrTag ) ) ;
		}
	}
	cf.DescendChunk( "strings " ) ;
	S3DStdModelSaver::WriteStrings( cf, aStrings ) ;
	cf.AscendChunk() ;
	//
	// メッシュ表示状態配列
	//
	for ( i = 0; i < nMeshSels; i ++ )
	{
		MeshSelector *	pms = m_ssoaMeshSel.GetAt( i ) ;
		ESLAssert( pms != NULL ) ;
		if ( pms != NULL )
		{
			MeshSelectorData	msdata ;
			eslFillMemory( &msdata, 0, sizeof(MeshSelectorData) ) ;
			msdata.iStringID = (uint32_t) i ;
			msdata.nVisible = pms->m_flagVisible ? 0x01 : 0 ;
			msdata.nVisible |= pms->m_fDisabled ? 0x80 : 0 ;
			msdata.nSeqFrames = (uint32_t) pms->m_aVisibles.GetLength() ;
			//
			cf.DescendChunk( "mesh_swt" ) ;
			cf.Write( &msdata, sizeof(MeshSelectorData) ) ;
			if ( msdata.nSeqFrames > 0 )
			{
				cf.Write( pms->m_aVisibles.GetConstArray(),
							msdata.nSeqFrames * sizeof(uint8_t) ) ;
			}
			cf.AscendChunk() ;
		}
	}
	return	sglErrSuccess ;
}

SGLError S3DModelPose::WriteMaterialSelectorChunk( SSystem::SChunkFile & cf )
{
	//
	// 定義文字列配列
	//
	StringArray	aStrings ;
	size_t	i ;
	size_t	nMaterialSels = m_ssoaMaterialSel.GetLength() ;
	for ( i = 0; i < nMaterialSels; i ++ )
	{
		const SString *	pstrTag = m_ssoaMaterialSel.GetTagAt( i ) ;
		ESLAssert( pstrTag != NULL ) ;
		if ( pstrTag != NULL )
		{
			aStrings.AllocateString( *pstrTag ) ;
		}
		MaterialSelector *	pms = m_ssoaMaterialSel.GetAt( i ) ;
		if ( pms != NULL )
		{
			for ( size_t j = 0; j < pms->m_aMaterialIDs.GetLength(); j ++ )
			{
				SString *	pstrID = pms->m_aMaterialIDs.GetAt( j ) ;
				if ( pstrID != NULL )
				{
					aStrings.AllocateString( *pstrID ) ;
				}
			}
		}
	}
	cf.DescendChunk( "strings " ) ;
	S3DStdModelSaver::WriteStrings( cf, aStrings ) ;
	cf.AscendChunk() ;
	//
	// マテリアル状態配列
	//
	for ( i = 0; i < nMaterialSels; i ++ )
	{
		const SString *	pstrTag = m_ssoaMaterialSel.GetTagAt( i ) ;
		MaterialSelector *	pms = m_ssoaMaterialSel.GetAt( i ) ;
		if ( (pstrTag != NULL) && (pms != NULL) )
		{
			MaterialSelectorData	msdata ;
			eslFillMemory( &msdata, 0, sizeof(MeshSelectorData) ) ;
			msdata.iStringID = (uint32_t) aStrings.GetStringIndex( *pstrTag ) ;
			msdata.nFlags = pms->m_fDisabled ? 0x80 : 0 ;
			msdata.nMaterialCount = (uint32_t) pms->m_aMaterialIDs.GetLength() ;
			msdata.nSeqFrames = (uint32_t) pms->m_aAnimation.GetLength() ;
			//
			cf.DescendChunk( "mtrl_swt" ) ;
			cf.Write( &msdata, sizeof(MaterialSelectorData) ) ;
			//
			SArray<uint32_t>	aMaterialIDs ;
			uint32_t *			pMaterialIDs =
					aMaterialIDs.GetArray( msdata.nMaterialCount ) ;
			for ( size_t j = 0; j < msdata.nMaterialCount; j ++ )
			{
				SString *	pstrID = pms->m_aMaterialIDs.GetAt( j ) ;
				if ( pstrID != NULL )
				{
					pMaterialIDs[j] =
						(uint32_t) aStrings.GetStringIndex( *pstrID ) ;
				}
			}
			cf.Write( pMaterialIDs, msdata.nMaterialCount * sizeof(uint32_t) ) ;
			aMaterialIDs.FinishArray() ;
			//
			if ( msdata.nSeqFrames > 0 )
			{
				cf.Write( pms->m_aAnimation.GetConstArray(),
							msdata.nSeqFrames * sizeof(uint32_t) ) ;
			}
			cf.AscendChunk() ;
		}
	}
	return	sglErrSuccess ;
}

// ポーズファイル書き出し (XML)
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelPose::SavePoseXML( const wchar_t * pwszFilePath )
{
	SXMLDocument	xmlPose ;
	xmlPose.SetTag( L"pose" ) ;
	FormatPoseXML( xmlPose ) ;
	if ( xmlPose.SaveDocument( pwszFilePath ) )
	{
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

SGLError S3DModelPose::FormatPoseXML( SSystem::SXMLDocument & xmlPoseTag )
{
	//
	// メタ情報
	//
	SXMLDocument *	pxmlMeta = new SXMLDocument ;
	pxmlMeta->SetTag( L"meta" ) ;
	pxmlMeta->SetAttrIntegerAs( L"duration", m_metaInfo.msecDuration ) ;
	pxmlMeta->SetAttrIntegerAs( L"frame_ratio", m_metaInfo.fxFrameRatio ) ;
	xmlPoseTag.AddElement( pxmlMeta ) ;
	//
	// ボーン
	//
	size_t	nJoints = m_ssoaJoint.GetLength() ;
	for ( size_t i = 0; i < nJoints; i ++ )
	{
		const SString *		pstrTag = m_ssoaJoint.GetTagAt( i ) ;
		JointAnimation *	pja = m_ssoaJoint.GetAt( i ) ;
		ESLAssert( pstrTag != NULL ) ;
		ESLAssert( pja != NULL ) ;
		if ( (pstrTag == NULL) || (pja == NULL) )
		{
			continue ;
		}
		SXMLDocument *	pxmlJoint = new SXMLDocument ;
		pxmlJoint->SetTag( L"joint" ) ;
		pxmlJoint->SetAttributeAs( L"id", *pstrTag ) ;
		pxmlJoint->SetAttrIntegerAs( L"disabled", (int) pja->m_fDisabled ) ;
		xmlPoseTag.AddElement( pxmlJoint ) ;
		//
		// クォータニオン
		//
		SXMLDocument *	pxmlQuaternion = new SXMLDocument ;
		pxmlQuaternion->SetTag( L"quaternion" ) ;
		pxmlJoint->AddElement( pxmlQuaternion ) ;
		//
		SString	strQuaternion ;
		strQuaternion.Format
			( L"%f %f %f %f", pja->m_qRotation.q[0],
									pja->m_qRotation.q[1],
									pja->m_qRotation.q[2],
									pja->m_qRotation.q[3] ) ;
		pxmlQuaternion->AddTextElement( strQuaternion ) ;
		//
		// 3x3 回転行列
		//
		SXMLDocument *	pxmlMatrix3 = new SXMLDocument ;
		pxmlMatrix3->SetTag( L"matrix3" ) ;
		pxmlJoint->AddElement( pxmlMatrix3 ) ;
		//
		SString		strMatrix3 ;
		S3DDMatrix	mat3 ;
		pja->m_qRotation.ToMatrix( mat3 ) ;
		//
		strMatrix3.Format
			( L"%f %f %f  %f %f %f  %f %f %f",
				mat3.m[0][0], mat3.m[0][1], mat3.m[0][2],
				mat3.m[1][0], mat3.m[1][1], mat3.m[1][2],
				mat3.m[2][0], mat3.m[2][1], mat3.m[2][2] ) ;
		pxmlMatrix3->AddTextElement( strMatrix3 ) ;
		//
		// 平行移動ベクトル
		//
		SXMLDocument *	pxmlOffset = new SXMLDocument ;
		pxmlOffset->SetTag( L"offset" ) ;
		pxmlJoint->AddElement( pxmlOffset ) ;
		//
		SString	strOffset ;
		strOffset.Format
			( L"%f %f %f", pja->m_vOffset.x,
							pja->m_vOffset.y, pja->m_vOffset.z ) ;
		pxmlOffset->AddTextElement( strOffset ) ;
		//
		// ボーンハンドル
		//
		SXMLDocument *	pxmlHandle = new SXMLDocument ;
		pxmlHandle->SetTag( L"bone_handle" ) ;
		pxmlJoint->AddElement( pxmlHandle ) ;
		//
		SString	strHandle ;
		strHandle.Format
			( L"%f %f %f", pja->m_vHandle.x,
							pja->m_vHandle.y, pja->m_vHandle.z ) ;
		pxmlHandle->AddTextElement( strHandle ) ;
		//
		// ハンドル軸回り回転
		//
		SXMLDocument *	pxmlZRot = new SXMLDocument ;
		pxmlZRot->SetTag( L"z_rotation" ) ;
		pxmlJoint->AddElement( pxmlZRot ) ;
		//
		pxmlZRot->SetAttrRealAs( L"angle", pja->m_zRotation ) ;
		//
		// 物理演算合成
		//
		SXMLDocument *	pxmlPhysBlend = new SXMLDocument ;
		pxmlPhysBlend->SetTag( L"phys_blend" ) ;
		pxmlJoint->AddElement( pxmlPhysBlend ) ;
		//
		pxmlPhysBlend->SetAttrRealAs( L"weight", pja->m_wPhysBlend ) ;
		//
		// 元ボーン行列
		//
		if ( pja->m_nExFlags & flagHaveOrgMatrix )
		{
			SXMLDocument *	pxmlOrgMatrix = new SXMLDocument ;
			pxmlOrgMatrix->SetTag( L"original_matrix" ) ;
			pxmlJoint->AddElement( pxmlOrgMatrix ) ;
			//
			SString				strMatrix ;
			const S4DMatrix&	mat4 = pja->m_mat4OrgLocal ;
			strMatrix.Format
				( L"%f %f %f %f  %f %f %f %f  %f %f %f %f  %f %f %f %f\n",
					mat4.m[0][0], mat4.m[0][1], mat4.m[0][2], mat4.m[0][3],
					mat4.m[1][0], mat4.m[1][1], mat4.m[1][2], mat4.m[1][3],
					mat4.m[2][0], mat4.m[2][1], mat4.m[2][2], mat4.m[2][3],
					mat4.m[3][0], mat4.m[3][1], mat4.m[3][2], mat4.m[3][3] ) ;
			//
			pxmlOrgMatrix->AddTextElement( strMatrix ) ;
		}
		//
		// 行列アニメーション
		//
		if ( pja->m_aMatrixs.GetLength() > 0 )
		{
			SString				strMatrixAnim ;
			const S4DMatrix *	pmatRot = pja->m_aMatrixs.GetConstArray() ;
			for ( size_t j = 0; j < pja->m_aMatrixs.GetLength(); j ++ )
			{
				SString	strEntry ;
				strEntry.Format
					( L"%f %f %f %f  %f %f %f %f  %f %f %f %f  %f %f %f %f\n",
						pmatRot[j].m[0][0], pmatRot[j].m[0][1],
							pmatRot[j].m[0][2], pmatRot[j].m[0][3],
						pmatRot[j].m[1][0], pmatRot[j].m[1][1],
							pmatRot[j].m[1][2], pmatRot[j].m[1][3],
						pmatRot[j].m[2][0], pmatRot[j].m[2][1],
							pmatRot[j].m[2][2], pmatRot[j].m[2][3],
						pmatRot[j].m[3][0], pmatRot[j].m[3][1],
							pmatRot[j].m[3][2], pmatRot[j].m[3][3] ) ;
				strMatrixAnim += strEntry ;
			}
			SXMLDocument *	pxmlMatrixAni = new SXMLDocument ;
			pxmlMatrixAni->SetTag( L"matrix44_animation" ) ;
			pxmlMatrixAni->SetAttrIntegerAs
				( L"frame_count", pja->m_aRotations.GetLength() ) ;
			pxmlMatrixAni->AddTextElement( strMatrixAnim ) ;
			pxmlJoint->AddElement( pxmlMatrixAni ) ;
		}
		//
		// 回転アニメーション
		//
		if ( pja->m_aRotations.GetLength() > 0 )
		{
			SString					strQtAnim ;
			const S3DDQuaternion *	pqRot = pja->m_aRotations.GetConstArray() ;
			for ( size_t j = 0; j < pja->m_aRotations.GetLength(); j ++ )
			{
				SString	strEntry ;
				strEntry.Format
					( L"%f %f %f %f\n",
						pqRot[j].q[0], pqRot[j].q[1],
						pqRot[j].q[2], pqRot[j].q[3] ) ;
				strQtAnim += strEntry ;
			}
			SXMLDocument *	pxmlQuatAni = new SXMLDocument ;
			pxmlQuatAni->SetTag( L"quaternion_animation" ) ;
			pxmlQuatAni->SetAttrIntegerAs
				( L"frame_count", pja->m_aRotations.GetLength() ) ;
			pxmlQuatAni->AddTextElement( strQtAnim ) ;
			pxmlJoint->AddElement( pxmlQuatAni ) ;
		}
		//
		// 移動アニメーション
		//
		if ( pja->m_aOffsets.GetLength() > 0 )
		{
			SString				strVAnim ;
			const S3DDVector *	pvMove = pja->m_aOffsets.GetConstArray() ;
			for ( size_t j = 0; j < pja->m_aOffsets.GetLength(); j ++ )
			{
				SString	strEntry ;
				strEntry.Format
					( L"%f %f %f\n",
						pvMove[j].x, pvMove[j].y, pvMove[j].z ) ;
				strVAnim += strEntry ;
			}
			SXMLDocument *	pxmlVec3Ani = new SXMLDocument ;
			pxmlVec3Ani->SetTag( L"vector3_animation" ) ;
			pxmlVec3Ani->SetAttrIntegerAs
				( L"frame_count", pja->m_aOffsets.GetLength() ) ;
			pxmlVec3Ani->AddTextElement( strVAnim ) ;
			pxmlJoint->AddElement( pxmlVec3Ani ) ;
		}
		//
		// 拡大アニメーション
		//
		if ( pja->m_aZooms.GetLength() > 0 )
		{
			SString				strZAnim ;
			const S3DDVector *	pvZoom = pja->m_aZooms.GetConstArray() ;
			for ( size_t j = 0; j < pja->m_aZooms.GetLength(); j ++ )
			{
				SString	strEntry ;
				strEntry.Format
					( L"%f %f %f\n",
						pvZoom[j].x, pvZoom[j].y, pvZoom[j].z ) ;
				strZAnim += strEntry ;
			}
			SXMLDocument *	pxmlZoomAni = new SXMLDocument ;
			pxmlZoomAni->SetTag( L"zoom_animation" ) ;
			pxmlZoomAni->SetAttrIntegerAs
				( L"frame_count", pja->m_aZooms.GetLength() ) ;
			pxmlZoomAni->AddTextElement( strZAnim ) ;
			pxmlJoint->AddElement( pxmlZoomAni ) ;
		}
		//
		// 物理演算合成アニメーション
		//
		if ( pja->m_aPhysBlends.GetLength() > 0 )
		{
			SString				strBlendAnim ;
			const float32_t *	pwBlend = pja->m_aPhysBlends.GetConstArray() ;
			for ( size_t j = 0; j < pja->m_aPhysBlends.GetLength(); j ++ )
			{
				SString	strEntry ;
				strEntry.Format( L"%f\n", pwBlend[j] ) ;
				strBlendAnim += strEntry ;
			}
			SXMLDocument *	pxmlBlendAni = new SXMLDocument ;
			pxmlBlendAni->SetTag( L"phy_blend_animation" ) ;
			pxmlBlendAni->SetAttrIntegerAs
				( L"frame_count", pja->m_aPhysBlends.GetLength() ) ;
			pxmlBlendAni->AddTextElement( strBlendAnim ) ;
			pxmlJoint->AddElement( pxmlBlendAni ) ;
		}
	}
	//
	// モーフィング
	//
	size_t	nMorphs = m_ssoaMorph.GetLength() ;
	for ( size_t i = 0; i < nMorphs; i ++ )
	{
		const SString *	pstrTag = m_ssoaMorph.GetTagAt( i ) ;
		MorphInfo *	pminf = m_ssoaMorph.GetAt( i ) ;
		ESLAssert( pstrTag != NULL ) ;
		ESLAssert( pminf != NULL ) ;
		if ( (pstrTag == NULL) || (pminf == NULL) )
		{
			continue ;
		}
		SXMLDocument *	pxmlMorph = new SXMLDocument ;
		pxmlMorph->SetTag( L"morphing" ) ;
		pxmlMorph->SetAttributeAs( L"id", *pstrTag ) ;
		pxmlMorph->SetAttrIntegerAs( L"disabled", (int) pminf->m_fDisabled ) ;
		pxmlMorph->SetAttributeAs( L"target", pminf->m_strMorphTarget ) ;
		xmlPoseTag.AddElement( pxmlMorph ) ;
		//
		if ( pminf->m_aTargetIDs.GetLength() > 0 )
		{
			SXMLDocument *	pxmlTarget = new SXMLDocument ;
			pxmlTarget->SetTag( L"target" ) ;
			pxmlMorph->AddElement( pxmlTarget ) ;
			//
			for ( size_t j = 0; j < pminf->m_aTargetIDs.GetLength(); j ++ )
			{
				SXMLDocument *	pxmlEntry = new SXMLDocument ;
				pxmlEntry->SetTag( L"entry" ) ;
				pxmlTarget->AddElement( pxmlEntry ) ;
				//
				SString *	pstrTarget = pminf->m_aTargetIDs.GetAt( j ) ;
				if ( pstrTarget != NULL )
				{
					pxmlEntry->SetAttributeAs( L"id", *pstrTarget ) ;
				}
			}
		}
		if ( pminf->m_aAnimation.GetLength() > 0 )
		{
			SXMLDocument *	pxmlAnim = new SXMLDocument ;
			pxmlAnim->SetTag( L"animation" ) ;
			pxmlMorph->AddElement( pxmlAnim ) ;
			//
			size_t	nFrameCount = pminf->m_aAnimation.GetLength() ;
			size_t	nTargetCount = pminf->m_aTargetIDs.GetLength() ;
			if ( nTargetCount > 0 )
			{
				nFrameCount /= nTargetCount ;
			}
			pxmlAnim->SetAttrIntegerAs( L"frame_count", nFrameCount ) ;
			//
			SString	strAniText ;
			for ( size_t j = 0; j < nFrameCount; j ++ )
			{
				SString	strFrame ;
				for ( size_t k = 0; k < nTargetCount; k ++ )
				{
					double	w = pminf->m_aAnimation.At( j * nTargetCount + k ) ;
					if ( k > 0 )
					{
						strFrame += L' ' ;
					}
					strFrame += SString( w, 8 ) ;
				}
				strFrame += L'\n' ;
				strAniText += strFrame ;
			}
			pxmlAnim->AddTextElement( strAniText ) ;
		}
	}
	//
	// 表示制御
	//
	size_t	nMeshSels = m_ssoaMeshSel.GetLength() ;
	for ( size_t i = 0; i < nMeshSels; i ++ )
	{
		const SString *	pstrTag = m_ssoaMeshSel.GetTagAt( i ) ;
		MeshSelector *	pmsel = m_ssoaMeshSel.GetAt( i ) ;
		ESLAssert( pstrTag != NULL ) ;
		ESLAssert( pmsel != NULL ) ;
		if ( (pstrTag != NULL) && (pmsel != NULL) )
		{
			SXMLDocument *	pxmlMeshSwitch = new SXMLDocument ;
			pxmlMeshSwitch->SetTag( L"mesh_selector" ) ;
			pxmlMeshSwitch->SetAttributeAs( L"id", *pstrTag ) ;
			pxmlMeshSwitch->SetAttrIntegerAs
				( L"disabled", (int) pmsel->m_fDisabled ) ;
			pxmlMeshSwitch->SetAttributeAs
				( L"visible", (pmsel->m_flagVisible ? L"true" : L"false") ) ;
			if ( pmsel->m_aVisibles.GetLength() > 0 )
			{
				SString			strSeq ;
				const uint8_t *	pVisibles = pmsel->m_aVisibles.GetConstArray() ;
				for ( size_t j = 0; j < pmsel->m_aVisibles.GetLength(); j ++ )
				{
					strSeq += (pVisibles[j] ? L"1 " : L"0 ") ;
				}
				strSeq.TrimRight() ;
				pxmlMeshSwitch->SetAttributeAs( L"sequence", strSeq ) ;
			}
			xmlPoseTag.AddElement( pxmlMeshSwitch ) ;
		}
	}
	//
	// マテリアル制御
	//
	size_t	nMaterialSels = m_ssoaMaterialSel.GetLength() ;
	for ( size_t i = 0; i < nMaterialSels; i ++ )
	{
		const SString *		pstrTag = m_ssoaMaterialSel.GetTagAt( i ) ;
		MaterialSelector *	pms = m_ssoaMaterialSel.GetAt( i ) ;
		ESLAssert( pstrTag != NULL ) ;
		ESLAssert( pms != NULL ) ;
		if ( (pstrTag != NULL) && (pms != NULL) )
		{
			SXMLDocument *	pxmlMaterialSwitch = new SXMLDocument ;
			pxmlMaterialSwitch->SetTag( L"material_selector" ) ;
			pxmlMaterialSwitch->SetAttributeAs( L"id", *pstrTag ) ;
			pxmlMaterialSwitch->SetAttrIntegerAs
				( L"disabled", (int) pms->m_fDisabled ) ;
			//
			SXMLDocument *	pxmlMaterialIDs =
						pxmlMaterialSwitch->CreateElementTagAs( L"material_ids" ) ;
			if ( pxmlMaterialIDs != NULL )
			{
				for ( size_t j = 0; j < pms->m_aMaterialIDs.GetLength(); j ++ )
				{
					SXMLDocument *	pxmlTag = new SXMLDocument ;
					pxmlTag->SetTag( L"entry" ) ;
					pxmlMaterialIDs->AddElement( pxmlTag ) ;
					//
					SString *	pstrID = pms->m_aMaterialIDs.GetAt( j ) ;
					if ( pstrID != NULL )
					{
						pxmlTag->SetAttributeAs( L"id", *pstrID ) ;
					}
				}
			}
			if ( pms->m_aAnimation.GetLength() > 0 )
			{
				SString				strSeq ;
				const uint32_t *	pAnimation = pms->m_aAnimation.GetConstArray() ;
				for ( size_t j = 0; j < pms->m_aAnimation.GetLength(); j ++ )
				{
					strSeq += SString( pAnimation[j] ) ;
					strSeq += L" " ;
				}
				strSeq.TrimRight() ;
				pxmlMaterialSwitch->SetAttributeAs( L"sequence", strSeq ) ;
			}
			xmlPoseTag.AddElement( pxmlMaterialSwitch ) ;
		}
	}
	return	sglErrSuccess ;
}

// ポーズに含まれる関節数取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DModelPose::GetJointCount( void ) const
{
	return	m_ssoaJoint.GetLength() ;
}

// 関節のボーン識別子名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DModelPose::GetJointNameAt( size_t i ) const
{
	SSmartLock<const SCriticalSection>	lock( &m_csLock ) ;
	//
	const SString *	pstrTag = m_ssoaJoint.GetTagAt( i ) ;
	if ( pstrTag != NULL )
	{
		return	*pstrTag ;
	}
	return	NULL ;
}

// 回転情報取得
//////////////////////////////////////////////////////////////////////////////
S3DModelPose::JointAnimation * S3DModelPose::GetJointAt( size_t i ) const
{
	SSmartLock<const SCriticalSection>	lock( &m_csLock ) ;
	return	m_ssoaJoint.GetAt( i ) ;
}

S3DModelPose::JointAnimation * S3DModelPose::GetJointAs( const wchar_t * pwszBoneID ) const
{
	SSmartLock<const SCriticalSection>	lock( &m_csLock ) ;
	return	m_ssoaJoint.GetAs( pwszBoneID ) ;
}

// 関節追加
//////////////////////////////////////////////////////////////////////////////
S3DModelPose::JointAnimation * S3DModelPose::AddJointAs
	( const wchar_t * pwszBoneID, S3DModelPose::JointAnimation * pjaJoint )
{
	SSmartLock<SCriticalSection>	lock( &m_csLock ) ;
	//
	m_pLastPoseTarget = NULL ;
	//
	size_t	i = m_ssoaJoint.Add( pwszBoneID, pjaJoint ) ;
	return	m_ssoaJoint.GetAt( i ) ;
}

// 関節削除
//////////////////////////////////////////////////////////////////////////////
void S3DModelPose::RemoveJointAt( size_t i )
{
	SSmartLock<SCriticalSection>	lock( &m_csLock ) ;
	//
	m_ssoaJoint.RemoveAt( i ) ;
	m_pLastPoseTarget = NULL ;
}

void S3DModelPose::RemoveJointAs( const wchar_t * pwszBoneID )
{
	SSmartLock<const SCriticalSection>	lock( &m_csLock ) ;
	//
	m_ssoaJoint.RemoveAs( pwszBoneID ) ;
	m_pLastPoseTarget = NULL ;
}

// 全関節削除
//////////////////////////////////////////////////////////////////////////////
void S3DModelPose::RemoveAllJoints( void )
{
	m_ssoaJoint.RemoveAll() ;
	m_pLastPoseTarget = NULL ;
}

// モーションデータが静止データの場合削除する
//////////////////////////////////////////////////////////////////////////////
void S3DModelPose::SmartAllJointMotion( void )
{
	SSmartLock<SCriticalSection>	lock( &m_csLock ) ;
	//
	for ( size_t i = 0; i < m_ssoaJoint.GetLength(); i ++ )
	{
		JointAnimation *	pja = m_ssoaJoint.GetAt( i ) ;
		if ( pja == NULL )
		{
			continue ;
		}
		pja->SmartMotion() ;
	}
}

// 禁止状態のジョイントを削除する
//////////////////////////////////////////////////////////////////////////////
void S3DModelPose::ClearDisabledJoint( void )
{
	SSmartLock<SCriticalSection>	lock( &m_csLock ) ;
	//
	m_pLastPoseTarget = NULL ;
	//
	for ( size_t i = 0; i < m_ssoaJoint.GetLength(); i ++ )
	{
		JointAnimation *	pja = m_ssoaJoint.GetAt( i ) ;
		if ( (pja == NULL) || pja->m_fDisabled )
		{
			m_ssoaJoint.RemoveAt( i -- ) ;
			continue ;
		}
	}
}

// ポーズに含まれるモーフィングメッシュ数取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DModelPose::GetMorphingCount( void ) const
{
	return	m_ssoaMorph.GetLength() ;
}

// 対象となるメッシュ（グループ）名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DModelPose::GetMorphingMeshNameAt( size_t i ) const
{
	SSmartLock<const SCriticalSection>	lock( &m_csLock ) ;
	//
	const SString *	pstrTag = m_ssoaMorph.GetTagAt( i ) ;
	if ( pstrTag != NULL )
	{
		return	*pstrTag ;
	}
	return	NULL ;
}

// モーフィングターゲット取得
//////////////////////////////////////////////////////////////////////////////
S3DModelPose::MorphInfo * S3DModelPose::GetMorphingAt( size_t i ) const
{
	SSmartLock<const SCriticalSection>	lock( &m_csLock ) ;
	return	m_ssoaMorph.GetAt( i ) ;
}

S3DModelPose::MorphInfo *
	S3DModelPose::GetMorphingAs( const wchar_t * pwszMeshID ) const
{
	SSmartLock<const SCriticalSection>	lock( &m_csLock ) ;
	return	m_ssoaMorph.GetAs( pwszMeshID ) ;
}

// モーフィングターゲット追加
//////////////////////////////////////////////////////////////////////////////
S3DModelPose::MorphInfo * S3DModelPose::AddMorphingAs
	( const wchar_t * pwszMeshID, MorphInfo * pmi )
{
	SSmartLock<SCriticalSection>	lock( &m_csLock ) ;
	//
	m_pLastPoseTarget = NULL ;
	//
	size_t	i = m_ssoaMorph.Add( pwszMeshID, pmi ) ;
	return	m_ssoaMorph.GetAt( i ) ;
}

// モーフィング削除
//////////////////////////////////////////////////////////////////////////////
void S3DModelPose::RemoveMorphingAt( size_t i )
{
	SSmartLock<SCriticalSection>	lock( &m_csLock ) ;
	m_ssoaMorph.RemoveAt( i ) ;
	m_pLastPoseTarget = NULL ;
}

void S3DModelPose::RemoveMorphingAs( const wchar_t * pwszMeshID )
{
	SSmartLock<SCriticalSection>	lock( &m_csLock ) ;
	m_ssoaMorph.RemoveAs( pwszMeshID ) ;
	m_pLastPoseTarget = NULL ;
}

// 全モーフィング削除
//////////////////////////////////////////////////////////////////////////////
void S3DModelPose::RemoveAllMorphings( void )
{
	SSmartLock<SCriticalSection>	lock( &m_csLock ) ;
	m_ssoaMorph.RemoveAll() ;
	m_pLastPoseTarget = NULL ;
}

// 禁止状態のモーフィングを削除する
//////////////////////////////////////////////////////////////////////////////
void S3DModelPose::ClearDisabledMorphing( void )
{
	SSmartLock<SCriticalSection>	lock( &m_csLock ) ;
	m_pLastPoseTarget = NULL ;
	//
	for ( size_t i = 0; i < m_ssoaMorph.GetLength(); i ++ )
	{
		MorphInfo *	pmi = m_ssoaMorph.GetAt( i ) ;
		if ( (pmi == NULL) || pmi->m_fDisabled )
		{
			m_ssoaMorph.RemoveAt( i -- ) ;
			continue ;
		}
	}
}

// ポーズに含まれるメッシュセレクタ数取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DModelPose::GetMeshSelectorCount( void ) const
{
	return	m_ssoaMeshSel.GetLength() ;
}

// 対象となるメッシュ（グループ）名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DModelPose::GetSelectedMeshNameAt( size_t i ) const
{
	SSmartLock<const SCriticalSection>	lock( &m_csLock ) ;
	//
	const SString *	pstrTag = m_ssoaMeshSel.GetTagAt( i ) ;
	if ( pstrTag != NULL )
	{
		return	*pstrTag ;
	}
	return	NULL ;
}

// メッシュセレクタ取得
//////////////////////////////////////////////////////////////////////////////
S3DModelPose::MeshSelector * S3DModelPose::GetMeshSelectorAt( size_t i ) const
{
	SSmartLock<const SCriticalSection>	lock( &m_csLock ) ;
	return	m_ssoaMeshSel.GetAt( i ) ;
}

S3DModelPose::MeshSelector *
	S3DModelPose::GetMeshSelectorAs( const wchar_t * pwszMeshID ) const
{
	SSmartLock<const SCriticalSection>	lock( &m_csLock ) ;
	return	m_ssoaMeshSel.GetAs( pwszMeshID ) ;
}

// メッシュセレクタ追加
//////////////////////////////////////////////////////////////////////////////
S3DModelPose::MeshSelector * S3DModelPose::AddMeshSelectorAs
	( const wchar_t * pwszMeshID, MeshSelector * pmsel )
{
	SSmartLock<SCriticalSection>	lock( &m_csLock ) ;
	//
	m_pLastPoseTarget = NULL ;
	//
	size_t	i = m_ssoaMeshSel.Add( pwszMeshID, pmsel ) ;
	return	m_ssoaMeshSel.GetAt( i ) ;
}

// メッシュセレクタ削除
//////////////////////////////////////////////////////////////////////////////
void S3DModelPose::RemoveMeshSelectorAt( size_t i )
{
	SSmartLock<SCriticalSection>	lock( &m_csLock ) ;
	m_ssoaMeshSel.RemoveAt( i ) ;
	m_pLastPoseTarget = NULL ;
}

void S3DModelPose::RemoveMeshSelectorAs( const wchar_t * pwszMeshID )
{
	SSmartLock<const SCriticalSection>	lock( &m_csLock ) ;
	m_ssoaMeshSel.RemoveAs( pwszMeshID ) ;
	m_pLastPoseTarget = NULL ;
}

// 全メッシュセレクタ削除
//////////////////////////////////////////////////////////////////////////////
void S3DModelPose::RemoveAllMeshSelectors( void )
{
	SSmartLock<const SCriticalSection>	lock( &m_csLock ) ;
	m_ssoaMeshSel.RemoveAll() ;
	m_pLastPoseTarget = NULL ;
}

// 禁止状態のメッシュセレクタを削除する
//////////////////////////////////////////////////////////////////////////////
void S3DModelPose::ClearDisabledMeshSelector( void )
{
	SSmartLock<const SCriticalSection>	lock( &m_csLock ) ;
	m_pLastPoseTarget = NULL ;
	//
	for ( size_t i = 0; i < m_ssoaMeshSel.GetLength(); i ++ )
	{
		MeshSelector *	pms = m_ssoaMeshSel.GetAt( i ) ;
		if ( (pms == NULL) || pms->m_fDisabled )
		{
			m_ssoaMeshSel.RemoveAt( i -- ) ;
			continue ;
		}
	}
}

// ポーズに含まれるマテリアルセレクタ数取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DModelPose::GetMaterialSelectorCount( void ) const
{
	return	m_ssoaMaterialSel.GetLength() ;
}

// 対象となるメッシュ（グループ）名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DModelPose::GetSelMaterialMeshNameAt( size_t i ) const
{
	SSmartLock<const SCriticalSection>	lock( &m_csLock ) ;
	//
	const SString *	pstrTag = m_ssoaMaterialSel.GetTagAt( i ) ;
	if ( pstrTag != NULL )
	{
		return	*pstrTag ;
	}
	return	NULL ;
}

// マテリアルセレクタ取得
//////////////////////////////////////////////////////////////////////////////
S3DModelPose::MaterialSelector *
	S3DModelPose::GetMaterialSelectorAt( size_t i ) const
{
	SSmartLock<const SCriticalSection>	lock( &m_csLock ) ;
	return	m_ssoaMaterialSel.GetAt( i ) ;
}

S3DModelPose::MaterialSelector *
	S3DModelPose::GetMaterialSelectorAs( const wchar_t * pwszMeshID ) const
{
	SSmartLock<const SCriticalSection>	lock( &m_csLock ) ;
	return	m_ssoaMaterialSel.GetAs( pwszMeshID ) ;
}

// メッシュセレクタ追加
//////////////////////////////////////////////////////////////////////////////
S3DModelPose::MaterialSelector *
	S3DModelPose::AddMaterialSelectorAs
		( const wchar_t * pwszMeshID, S3DModelPose::MaterialSelector * pmsel )
{
	SSmartLock<SCriticalSection>	lock( &m_csLock ) ;
	//
	m_pLastPoseTarget = NULL ;
	//
	size_t	i = m_ssoaMaterialSel.Add( pwszMeshID, pmsel ) ;
	return	m_ssoaMaterialSel.GetAt( i ) ;
}

// メッシュセレクタ削除
//////////////////////////////////////////////////////////////////////////////
void S3DModelPose::RemoveMaterialSelectorAt( size_t i )
{
	SSmartLock<SCriticalSection>	lock( &m_csLock ) ;
	m_ssoaMaterialSel.RemoveAt( i ) ;
	m_pLastPoseTarget = NULL ;
}

void S3DModelPose::RemoveMaterialSelectorAs( const wchar_t * pwszMeshID )
{
	SSmartLock<SCriticalSection>	lock( &m_csLock ) ;
	m_ssoaMaterialSel.RemoveAs( pwszMeshID ) ;
	m_pLastPoseTarget = NULL ;
}

// 全メッシュセレクタ削除
//////////////////////////////////////////////////////////////////////////////
void S3DModelPose::RemoveAllMaterialSelectors( void )
{
	SSmartLock<SCriticalSection>	lock( &m_csLock ) ;
	m_ssoaMaterialSel.RemoveAll() ;
	m_pLastPoseTarget = NULL ;
}

// 禁止状態のマテリアルセレクタを削除する
//////////////////////////////////////////////////////////////////////////////
void S3DModelPose::ClearDisabledMaterialSelector( void )
{
	SSmartLock<SCriticalSection>	lock( &m_csLock ) ;
	m_pLastPoseTarget = NULL ;
	//
	for ( size_t i = 0; i < m_ssoaMaterialSel.GetLength(); i ++ )
	{
		MaterialSelector *	pms = m_ssoaMaterialSel.GetAt( i ) ;
		if ( (pms == NULL) || pms->m_fDisabled )
		{
			m_ssoaMaterialSel.RemoveAt( i -- ) ;
			continue ;
		}
	}
}

// ポーズターゲット取得
//////////////////////////////////////////////////////////////////////////////
template <class T> static void MakeMeshDivisionMapList
	( SSystem::SArray<S3DModelPose::MeshDivMap>& aMapList,
		const S3DModelBuffer& model,
		SSystem::SStrSortObjectArray<T>& ssoaElement )
{
	aMapList.RemoveAll() ;
	aMapList.SetLimit( ssoaElement.GetLength() ) ;
	//
	for ( size_t i = 0; i < ssoaElement.GetLength(); i ++ )
	{
		const SString *	pstrTag = ssoaElement.GetTagAt( i ) ;
		ESLAssert( pstrTag != nullptr ) ;
		if ( pstrTag == nullptr )
		{
			continue ;
		}
		const S3DModelBuffer::MeshDivision *
				pMeshDiv = model.GetMeshDivisionAs( *pstrTag ) ;
		if ( pMeshDiv != nullptr )
		{
			for ( size_t j = 0; j < pMeshDiv->m_aSplittedEntries.GetLength(); j ++ )
			{
				S3DModelPose::MeshDivMap	mdm ;
				mdm.iElement = i ;
				mdm.iMeshDiv = j ;
				mdm.pMeshDiv = pMeshDiv ;
				mdm.pSplitted = pMeshDiv->m_aSplittedEntries.GetAt( j ) ;
				aMapList.Add( mdm ) ;
			}
		}
		else
		{
			S3DModelPose::MeshDivMap	mdm ;
			mdm.iElement = i ;
			mdm.iMeshDiv = 0 ;
			mdm.pMeshDiv = nullptr ;
			mdm.pSplitted = nullptr ;
			aMapList.Add( mdm ) ;
		}
	}
}

void S3DModelPose::UpdatePoseTarget( S3DModelBuffer * pModel )
{
	if ( m_pLastPoseTarget == pModel )
	{
		return ;
	}
	SSmartLock<SCriticalSection>	lock( &m_csLock ) ;
	//
	MakeMeshDivisionMapList<MorphInfo>
			( m_aMorphMeshDiv, *pModel, m_ssoaMorph ) ;
	MakeMeshDivisionMapList<MeshSelector>
			( m_aMeshSelMeshDiv, *pModel, m_ssoaMeshSel ) ;
	MakeMeshDivisionMapList<MaterialSelector>
			( m_aMaterialSelMeshDiv, *pModel, m_ssoaMaterialSel ) ;
	//
	const size_t	nJoints = m_ssoaJoint.GetLength() ;
	const size_t	nMorphs = m_aMorphMeshDiv.GetLength() ;
	const size_t	nMeshSels = m_aMeshSelMeshDiv.GetLength() ;
	const size_t	nMaterialSels = m_aMaterialSelMeshDiv.GetLength() ;
	m_aBoneBuf.SetLength( nJoints ) ;
	m_aMorphBuf.SetLength( nMorphs ) ;
	m_aMeshSelBuf.SetLength( nMeshSels ) ;
	m_aMaterialSelBuf.SetLength( nMaterialSels ) ;
	//
	S3DModelBoneSpace **			ppJoints = m_aBoneBuf.GetArray() ;
	MorphTarget **					pMorphs = m_aMorphBuf.GetArray() ;
	S3DModelBuffer::MeshGroup **	ppMeshSels = m_aMeshSelBuf.GetArray() ;
	S3DModelBuffer::MeshGroup **	ppMaterialSels = m_aMaterialSelBuf.GetArray() ;
	//
	for ( size_t i = 0; i < nJoints; i ++ )
	{
		const SString *	pstrTag = m_ssoaJoint.GetTagAt( i ) ;
		ESLAssert( pstrTag != nullptr ) ;
		S3DModelBoneSpace *	pBone = nullptr ;
		if ( pstrTag != nullptr )
		{
			pBone = pModel->GetBonePropertyAs( *pstrTag ) ;
			if ( (pBone == nullptr) && (*pstrTag == L"@local") )
			{
				pBone = &(pModel->GetLocalSpaceBone()) ;
			}
		}
		ppJoints[i] = pBone ;
	}
	m_aBoneBuf.FinishArray() ;
	//
	for ( size_t i = 0; i < nMorphs; i ++ )
	{
		MorphTarget *	pmt = pMorphs[i] ;
		if ( pmt == nullptr )
		{
			pmt = new MorphTarget ;
			pMorphs[i] = pmt ;
		}
		MorphTarget&	mt = *pmt ;
		mt.pmg = nullptr ;
		mt.pMesh = nullptr ;
		//
		MeshDivMap *	pDivMap = m_aMorphMeshDiv.GetAt( i ) ;
		ESLAssert( pDivMap != nullptr ) ;
		if ( pDivMap == nullptr )
		{
			continue ;
		}
		const size_t	iMorph = pDivMap->iElement ;
		const SString *	pstrTag = m_ssoaMorph.GetTagAt( iMorph ) ;
		MorphInfo *		pminf = m_ssoaMorph.GetAt( iMorph ) ;
		if ( (pstrTag == nullptr) || (pminf == nullptr) )
		{
			continue ;
		}
		if ( pDivMap->pSplitted != nullptr )
		{
			pstrTag = &(pDivMap->pSplitted->m_strSplittedMesh) ;
		}
		S3DModelBuffer::MeshGroup *
			pmg = pModel->GetMeshGroupList().GetAs( *pstrTag ) ;
		if ( pmg == NULL )
		{
			continue ;
		}
		S3DModelBuffer::MeshObject *
				pMesh = pModel->GetMeshObjectAt( pmg->m_iFirstMesh ) ;
		if ( pMesh == NULL )
		{
			continue ;
		}
		mt.pmg = pmg ;
		mt.pMesh = pMesh ;
		PrepareMorphTargetIndex
			( mt, *pminf, pMesh, pDivMap->pMeshDiv, pDivMap->pSplitted ) ;
	}
	m_aMorphBuf.FinishArray() ;
	//
	for ( size_t i = 0; i < nMeshSels; i ++ )
	{
		ppMeshSels[i] = NULL ;
		//
		MeshDivMap *	pDivMap = m_aMeshSelMeshDiv.GetAt( i ) ;
		ESLAssert( pDivMap != nullptr ) ;
		if ( pDivMap == nullptr )
		{
			continue ;
		}
		const SString *	pstrTag = m_ssoaMeshSel.GetTagAt( pDivMap->iElement ) ;
		ESLAssert( pstrTag != NULL ) ;
		if ( pstrTag == NULL )
		{
			continue ;
		}
		if ( pDivMap->pSplitted != nullptr )
		{
			pstrTag = &(pDivMap->pSplitted->m_strSplittedMesh) ;
		}
		S3DModelBuffer::MeshGroup *
			pmg = pModel->GetMeshGroupList().GetAs( *pstrTag ) ;
		if ( pmg == NULL )
		{
			continue ;
		}
		ppMeshSels[i] = pmg ;
	}
	m_aMeshSelBuf.FinishArray() ;
	//
	for ( size_t i = 0; i < nMaterialSels; i ++ )
	{
		ppMaterialSels[i] = NULL ;
		//
		MeshDivMap *	pDivMap = m_aMaterialSelMeshDiv.GetAt( i ) ;
		ESLAssert( pDivMap != nullptr ) ;
		if ( pDivMap == nullptr )
		{
			continue ;
		}
		const SString *	pstrTag = m_ssoaMaterialSel.GetTagAt( pDivMap->iElement ) ;
		ESLAssert( pstrTag != NULL ) ;
		if ( pstrTag == NULL )
		{
			continue ;
		}
		if ( pDivMap->pSplitted != nullptr )
		{
			pstrTag = &(pDivMap->pSplitted->m_strSplittedMesh) ;
		}
		S3DModelBuffer::MeshGroup *
			pmg = pModel->GetMeshGroupList().GetAs( *pstrTag ) ;
		if ( pmg == NULL )
		{
			continue ;
		}
		ppMaterialSels[i] = pmg ;
		//
		MaterialSelector *	pms = m_ssoaMaterialSel.GetAt( pDivMap->iElement ) ;
		if ( pms != NULL )
		{
			const size_t	nMaterials = pms->m_aMaterialIDs.GetLength() ;
			S3DMaterial **	ppMaterials = pms->m_aMaterials.GetArray( nMaterials ) ;
			for ( size_t j = 0; j < nMaterials; j ++ )
			{
				SString *	pstrMaterialID = pms->m_aMaterialIDs.GetAt( j ) ;
				if ( pstrMaterialID != NULL )
				{
					ppMaterials[j] = pModel->GetMaterialLibrary().
										GetMaterialAs( *pstrMaterialID ) ;
				}
			}
			pms->m_aMaterials.FinishArray() ;
		}
	}
	m_aMaterialSelBuf.FinishArray() ;
	//
	m_pLastPoseTarget = pModel ;
}

// ポーズターゲットリセット
//////////////////////////////////////////////////////////////////////////////
void S3DModelPose::ResetPoseTarget( void )
{
	SSmartLock<SCriticalSection>	lock( &m_csLock ) ;
	//
	m_pLastPoseTarget = NULL ;
	m_aBoneBuf.RemoveAll() ;
	m_aMorphMeshDiv.RemoveAll() ;
	m_aMorphBuf.RemoveAll() ;
	m_aMeshSelMeshDiv.RemoveAll() ;
	m_aMeshSelBuf.RemoveAll() ;
	m_aMaterialSelMeshDiv.RemoveAll() ;
	//
	for ( size_t i = 0; i < m_ssoaMaterialSel.GetLength(); i ++ )
	{
		MaterialSelector *	pms = m_ssoaMaterialSel.GetAt( i ) ;
		if ( pms != NULL )
		{
			pms->m_aMaterials.RemoveAll() ;
		}
	}
}

// アニメーションフレーム補完情報
//////////////////////////////////////////////////////////////////////////////
void S3DModelPose::CalculateAnimationFrame
	( S3DModelPose::AnimationFrame& af, double t, size_t nFrames ) const
{
	if ( nFrames == 0 )
	{
		af.iFrame0 = 0 ;
		af.iFrame1 = 0 ;
		af.fpDelta = 0.0 ;
		return ;
	}
	double	fpFrame ;
	if ( (nFrames > 0) && (m_metaInfo.msecDuration > 0) )
	{
		fpFrame = t * 1000 / m_metaInfo.msecDuration * (nFrames - 1) ;
	}
	else
	{
		fpFrame = 0.0 ;
	}
	af.iFrame0 = (int) floor( fpFrame ) ;
	af.iFrame1 = af.iFrame0 + 1 ;
	af.fpDelta = fpFrame - af.iFrame0 ;
	if ( af.iFrame0 >= (int) nFrames - 1 )
	{
		af.iFrame0 = (int) nFrames - 1 ;
		af.iFrame1 = af.iFrame0 ;
		af.fpDelta = 0.0 ;
	}
	else if ( af.iFrame0 < 0 )
	{
		af.iFrame0 = 0 ;
		af.iFrame1 = 0 ;
		af.fpDelta = 0.0 ;
	}
	ESLAssert( af.iFrame0 >= 0 ) ;
	ESLAssert( af.iFrame0 < (int) nFrames ) ;
	ESLAssert( af.iFrame1 >= 0 ) ;
	ESLAssert( af.iFrame1 < (int) nFrames ) ;
}

// モデルへポーズ設定
//////////////////////////////////////////////////////////////////////////////
void S3DModelPose::ApplyPoseTo( S3DModelBuffer& model, double w, double t )
{
	if ( w < 1.0e-8 )
	{
		return ;
	}
	SSmartLock<SCriticalSection>	lock( &m_csLock ) ;
	//
	UpdatePoseTarget( &model ) ;
	//
	// ボーン
	//
	BlendBonePoseTo( model, w, t ) ;
	//
	// モーフィング
	//
	BlendMorphPoseTo( model, w, t, false ) ;
	//
	// メッシュ表示選択
	//
	if ( w >= 0.99999 )
	{
		BlendMeshSelectorPoseTo( model, t ) ;
	}
	//
	// マテリアル選択
	//
	if ( w >= 0.99999 )
	{
		BlendMaterialSelectorPoseTo( model, t ) ;
	}
}

void S3DModelPose::ProductPoseTo( S3DModelBuffer& model, double w, double t )
{
	if ( w < 1.0e-8 )
	{
		return ;
	}
	SSmartLock<SCriticalSection>	lock( &m_csLock ) ;
	//
	UpdatePoseTarget( &model ) ;
	//
	// ボーン
	//
	ProductBonePoseTo( model, w, t ) ;
	//
	// モーフィング
	//
	BlendMorphPoseTo( model, w, t, true ) ;
	//
	// メッシュ表示選択
	//
	if ( w >= 0.99999 )
	{
		BlendMeshSelectorPoseTo( model, t ) ;
	}
	//
	// マテリアル選択
	//
	if ( w >= 0.99999 )
	{
		BlendMaterialSelectorPoseTo( model, t ) ;
	}
}

// ボーン合成
//////////////////////////////////////////////////////////////////////////////
void S3DModelPose::BlendBonePoseTo
	( S3DModelBuffer& model, double w, double t )
{
	SSmartLock<SCriticalSection>	lock( &m_csLock ) ;
	ESLAssert( m_pLastPoseTarget == &model ) ;
	//
	bool						flagPosed = false ;
	const size_t				nJoints = m_aBoneBuf.GetLength() ;
	S3DModelBoneSpace *const*	ppJoints = m_aBoneBuf.GetConstArray() ;
	for ( size_t i = 0; i < nJoints; i ++ )
	{
		S3DModelBoneSpace *	pBone = ppJoints[i] ;
		JointAnimation *	pja = m_ssoaJoint.GetAt( i ) ;
		ESLAssert( pja != NULL ) ;
		if ( (pBone == NULL) || (pja == NULL) || pja->m_fDisabled )
		{
			continue ;
		}
		//
		// フレーム情報取得
		//
		MatrixElement	meBone ;
		S3DDVector		vBoneMove ;
		double			wPhysBlend ;
		CalcJointFrameMatrix( meBone, vBoneMove, wPhysBlend, *pja, t ) ;
		//
		// 合成
		//
		if ( w < 1.0 - 1.0e-8 )
		{
			MatrixElement	meDst ;
			meDst.FromMatrix( pBone->m_matTransformation ) ;
			//
			double	wb = w ;
			if ( pBone->GetBoneFlags() & S3DModelBoneSpace::flagBonePhysics )
			{
				double	nw = 1.0 - pBone->GetPhysicsBlendWeight() ;
				wb = 1.0 - (1.0 - wb) * nw ;
				wb *= 1.0 - wPhysBlend ;
			}
			//
			meBone.Slerp( meDst, meBone, wb ) ;
			vBoneMove = pBone->GetBoneOffset() * (1.0 - wb) + vBoneMove * wb ;
		}
		S3DDMatrix	matPose ;
		meBone.ToMatrix( matPose ) ;
		//
		// モデル行列補正
		//
		/*
		if ( (pja->m_nExFlags & flagHaveOrgMatrix)
			&& (pBone->GetBoneFlags()
						& S3DModelBoneSpace::flagHaveOrgMatrix) )
		{
			S3DDMatrix	matOrgModel, matOrgPose ;
			S3DDVector	vOrgModel, vOrgPose ;
			Matrix3x3From4x4<S3DDMatrix,S3DDVector,float32_t>
				( matOrgModel, vOrgModel, pBone->GetOriginalBoneMatrix() ) ;
			Matrix3x3From4x4<S3DDMatrix,S3DDVector,float32_t>
				( matOrgPose, vOrgPose, pja->m_mat4OrgLocal ) ;
			//
			double	d = pow( matOrgModel.Determinant(), 1.0 / 3.0 ) ;
			matOrgModel /= d ;
			d = pow( matOrgPose.Determinant(), 1.0 / 3.0 ) ;
			matOrgPose /= d ;
			//
			matPose *= matOrgPose * matOrgModel.Inverse() ;
		}
		*/
		//
		// ボーンへ反映
		//
		pBone->SetLocalTransformation( matPose ) ;
		pBone->SetBoneOffset( vBoneMove ) ;
		//
		pBone->SetPhysicsBlendWeight
			( wPhysBlend * w
				+ pBone->GetPhysicsBlendWeight() * (1.0 - w) ) ;
		//
		flagPosed = true ;
	}
	if ( flagPosed )
	{
		model.PostUpdateBone() ;
	}
}

void S3DModelPose::ProductBonePoseTo
	( S3DModelBuffer& model, double w, double t )
{
	SSmartLock<SCriticalSection>	lock( &m_csLock ) ;
	ESLAssert( m_pLastPoseTarget == &model ) ;
	//
	bool						flagPosed = false ;
	const size_t				nJoints = m_aBoneBuf.GetLength() ;
	S3DModelBoneSpace *const*	ppJoints = m_aBoneBuf.GetConstArray() ;
	for ( size_t i = 0; i < nJoints; i ++ )
	{
		S3DModelBoneSpace *	pBone = ppJoints[i] ;
		JointAnimation *	pja = m_ssoaJoint.GetAt( i ) ;
		ESLAssert( pja != NULL ) ;
		if ( (pBone == NULL) || (pja == NULL) || pja->m_fDisabled )
		{
			continue ;
		}
		//
		// フレーム情報取得
		//
		MatrixElement	meBone ;
		S3DDVector		vBoneMove ;
		double			wPhysBlend ;
		CalcJointFrameMatrix( meBone, vBoneMove, wPhysBlend, *pja, t ) ;
		//
		// 合成
		//
		if ( w < 1.0 - 1.0e-8 )
		{
			MatrixElement	meI ;
			S3DDMatrix		matI( 1, 1, 1 ) ;
			meI.FromMatrix( matI ) ;
			//
			meBone.Slerp( meI, meBone, w ) ;
			vBoneMove = vBoneMove * w ;
		}
		S3DDMatrix	matPose ;
		meBone.ToMatrix( matPose ) ;
		//
		// ボーンへ反映
		//
		if ( pBone->m_flagsModified & S3DScene::spaceElementTransformation )
		{
			pBone->SetLocalTransformation( matPose * pBone->m_matTransformation ) ;
		}
		else
		{
			pBone->SetLocalTransformation( matPose ) ;
		}
		pBone->SetBoneOffset( vBoneMove + pBone->GetBoneOffset() ) ;
		pBone->SetPhysicsBlendWeight
			( wPhysBlend * w
				+ pBone->GetPhysicsBlendWeight() * (1.0 - w) ) ;
		//
		flagPosed = true ;
	}
	if ( flagPosed )
	{
		model.PostUpdateBone() ;
	}
}

// モーフィング合成
//////////////////////////////////////////////////////////////////////////////
void S3DModelPose::BlendMorphPoseTo
	( S3DModelBuffer& model, double w, double t, bool flagBlendAdd )
{
	SSmartLock<SCriticalSection>	lock( &m_csLock ) ;
	ESLAssert( m_pLastPoseTarget == &model ) ;
	//
	MorphContext		mcMorphTemp ;
	MorphContext		mcMorphTempSrc ;
	MorphTarget *const*	ppMorphTargets = m_aMorphBuf.GetConstArray() ;
	const size_t		nMorphs = m_aMorphBuf.GetLength() ;
	for ( size_t i = 0; i < nMorphs; i ++ )
	{
		MorphTarget *	pmt = ppMorphTargets[i] ;
		if ( pmt == NULL )
		{
			continue ;
		}
		const MeshDivMap *	pMeshDiv = m_aMorphMeshDiv.GetAt( i ) ;
		ESLAssert( pMeshDiv != nullptr ) ;
		if ( pMeshDiv == nullptr )
		{
			continue ;
		}
		S3DModelBuffer::MeshGroup *		pmg = pmt->pmg ;
		S3DModelBuffer::MeshObject *	pMesh = pmt->pMesh ;
		MorphInfo *						pminf = m_ssoaMorph.GetAt( pMeshDiv->iElement ) ;
		ESLAssert( pminf != NULL ) ;
		if ( (pmg == NULL) || (pMesh == NULL)
			|| (pminf == NULL) || pminf->m_fDisabled )
		{
			continue ;
		}
		//
		// フレーム情報取得
		//
		CalcMorphFrameContext( *pmt, *pminf, t ) ;
		//
		size_t				nTargetCount = pmt->nTargetCount ;
		const ssize_t *		pTargets = pmt->aTarget.GetArray( nTargetCount ) ;
		const float32_t *	pWeights = pmt->aWeight.GetArray( nTargetCount ) ;
		//
		if ( w < 1.0 - 1.0e-8 )
		{
			//
			// 現在のモーフィング状態取得
			//
			const size_t	nMorphTargetCount =
									pMesh->m_arrMorphTarget.GetLength() + 1 ;
			ssize_t *	pMorphMesh = mcMorphTemp.aTarget.GetArray( nMorphTargetCount ) ;
			float32_t *	pMorphApply = mcMorphTemp.aWeight.GetArray( nMorphTargetCount ) ;
			size_t		j ;
			for ( j = 0; j < nMorphTargetCount; j ++ )
			{
				pMorphMesh[j] = (ssize_t) j - 1 ;
				pMorphApply[j] = 0.0f ;
			}
			for ( j = 0; j < nMorphTargetCount; j ++ )
			{
				ssize_t		iMesh ;
				float32_t	fpApply ;
				if ( model.GetMorphingApplication
					( pmg->m_iFirstMesh, iMesh, fpApply, j ) )
				{
					break ;
				}
				iMesh += 1 ;
				if ( (size_t) iMesh < nMorphTargetCount )
				{
					pMorphApply[iMesh] = fpApply ;
				}
			}
			mcMorphTemp.aTarget.FinishArray() ;
			mcMorphTemp.aWeight.FinishArray() ;
			mcMorphTemp.aTarget.SetLength( nMorphTargetCount ) ;
			mcMorphTemp.aWeight.SetLength( nMorphTargetCount ) ;
			mcMorphTemp.nTargetCount = nMorphTargetCount ;
			//
			// 合成
			//
			NormalizeMorphContext( mcMorphTempSrc, *pmt, nMorphTargetCount ) ;
			if ( flagBlendAdd )
			{
				BlendAddMorphContext( mcMorphTemp, mcMorphTempSrc, w ) ;
			}
			else
			{
				BlendMorphContext( mcMorphTemp, mcMorphTempSrc, w ) ;
			}
			TrimMorphContext( mcMorphTemp ) ;
			//
			nTargetCount = mcMorphTemp.nTargetCount ;
			pTargets = mcMorphTemp.aTarget.GetConstArray() ;
			pWeights = mcMorphTemp.aWeight.GetConstArray() ;
		}
		model.SetMorphingApplication
			( pmg->m_iFirstMesh, pTargets, pWeights, nTargetCount ) ;
	}
}

// メッシュ表示状態合成
//////////////////////////////////////////////////////////////////////////////
void S3DModelPose::BlendMeshSelectorPoseTo
	( S3DModelBuffer& model, double t )
{
	SSmartLock<SCriticalSection>	lock( &m_csLock ) ;
	ESLAssert( m_pLastPoseTarget == &model ) ;
	//
	S3DModelBuffer::MeshGroup *const*
					ppMeshs = m_aMeshSelBuf.GetConstArray() ;
	const size_t	nMeshSels = m_aMeshSelBuf.GetLength() ;
	for ( size_t i = 0; i < nMeshSels; i ++ )
	{
		S3DModelBuffer::MeshGroup *	pmg = ppMeshs[i] ;
		const MeshDivMap *			pMeshDiv = m_aMeshSelMeshDiv.GetAt( i ) ;
		ESLAssert( pMeshDiv != nullptr ) ;
		if ( pMeshDiv == nullptr )
		{
			continue ;
		}
		MeshSelector *	pmsel = m_ssoaMeshSel.GetAt( pMeshDiv->iElement ) ;
		ESLAssert( pmsel != NULL ) ;
		if ( (pmg == NULL) || (pmsel == NULL) || pmsel->m_fDisabled )
		{
			continue ;
		}
		if ( pmsel->m_aVisibles.GetLength() > 0 )
		{
			AnimationFrame	af ;
			CalculateAnimationFrame
				( af, t, pmsel->m_aVisibles.GetLength() ) ;
			//
			ESLAssert( (size_t) af.iFrame0 < pmsel->m_aVisibles.GetLength() ) ;
			if ( (size_t) af.iFrame0 < pmsel->m_aVisibles.GetLength() )
			{
				model.EnableToRenderMesh
					( pmg->m_iFirstMesh,
						pmg->m_iFirstMesh + pmg->m_nMeshCount,
						(pmsel->m_aVisibles.At((size_t)af.iFrame0) != 0) ) ;
			}
		}
		else
		{
			model.EnableToRenderMesh
				( pmg->m_iFirstMesh,
					pmg->m_iFirstMesh + pmg->m_nMeshCount, pmsel->m_flagVisible ) ;
		}
	}
}

// マテリアル選択合成
//////////////////////////////////////////////////////////////////////////////
void S3DModelPose::BlendMaterialSelectorPoseTo
	( S3DModelBuffer& model, double t )
{
	SSmartLock<SCriticalSection>	lock( &m_csLock ) ;
	ESLAssert( m_pLastPoseTarget == &model ) ;
	//
	S3DModelBuffer::MeshGroup *const*
					ppMtrlMeshs = m_aMaterialSelBuf.GetConstArray() ;
	const size_t	nMaterialSels = m_aMaterialSelBuf.GetLength() ;
	for ( size_t i = 0; i < nMaterialSels; i ++ )
	{
		S3DModelBuffer::MeshGroup *	pmg = ppMtrlMeshs[i] ;
		const MeshDivMap *			pMeshDiv = m_aMaterialSelMeshDiv.GetAt( i ) ;
		ESLAssert( pMeshDiv != nullptr ) ;
		if ( pMeshDiv == nullptr )
		{
			continue ;
		}
		MaterialSelector *	pms = m_ssoaMaterialSel.GetAt( pMeshDiv->iElement ) ;
		ESLAssert( pms != NULL ) ;
		if ( (pmg == NULL) || (pms == NULL) || pms->m_fDisabled )
		{
			continue ;
		}
		uint32_t	iMaterial = 0 ;
		if ( pms->m_aAnimation.GetLength() > 0 )
		{
			AnimationFrame	af ;
			CalculateAnimationFrame
				( af, t, pms->m_aAnimation.GetLength() ) ;
			//
			ESLAssert( (size_t) af.iFrame0 < pms->m_aAnimation.GetLength() ) ;
			if ( (size_t) af.iFrame0 < pms->m_aAnimation.GetLength() )
			{
				iMaterial = pms->m_aAnimation.At( (size_t) af.iFrame0 ) ;
			}
		}
		S3DMaterial *	pMaterial = pms->m_aMaterials.GetAt( iMaterial ) ;
		if ( pMaterial != NULL )
		{
			for ( size_t j = 0; j < pmg->m_nMeshCount; j ++ )
			{
				model.SetMaterialToRenderMesh
						( pmg->m_iFirstMesh + j, pMaterial ) ;
			}
		}
	}
}

// Joint フレーム取得
//////////////////////////////////////////////////////////////////////////////
void S3DModelPose::CalcJointFrameMatrix
	( S3DModelPose::MatrixElement& meBone,
		S3DDVector& vBoneMove, double& wPhysBlend,
		const S3DModelPose::JointAnimation& ja, double t /* sec */ ) const
{
	//
	// 行列
	//
	if ( ja.m_aMatrixs.GetLength() >= 2 )
	{
		const S4DMatrix * pMatrixs = ja.m_aMatrixs.GetConstArray() ;
		const size_t	nFrames = ja.m_aMatrixs.GetLength() ;
		AnimationFrame	af ;
		CalculateAnimationFrame( af, t, nFrames ) ;
		//
		S4DDMatrix	mat4Temp = pMatrixs[af.iFrame0] ;
		S3DDMatrix	matTrans ;
		S3DDVector	vTrans ;
		Matrix3x3From4x4
			<S3DDMatrix,S3DDVector,double>( matTrans, vTrans, mat4Temp ) ;
		meBone.FromMatrix( matTrans ) ;
		vBoneMove = vTrans ;
	}
	else
	{
		//
		// 回転成分
		//
		if ( ja.m_aRotations.GetLength() >= 2 )
		{
			const S3DDQuaternion *
							pRotations = ja.m_aRotations.GetConstArray() ;
			const size_t	nFrames = ja.m_aRotations.GetLength() ;
			AnimationFrame	af ;
			CalculateAnimationFrame( af, t, nFrames ) ;
			//
			S3DDQuaternion	q = pRotations[af.iFrame0] ;
			q += (pRotations[af.iFrame1]
					- pRotations[af.iFrame0]) * af.fpDelta ;
			q.Normalize() ;
			//
			meBone.qRotation = q ;
		}
		else
		{
			meBone.qRotation = ja.m_qRotation ;
		}
		//
		// 拡大率成分
		//
		if ( ja.m_aZooms.GetLength() >= 2 )
		{
			const S3DDVector *	pvZooms = ja.m_aZooms.GetConstArray() ;
			const size_t	nFrames = ja.m_aZooms.GetLength() ;
			AnimationFrame	af ;
			CalculateAnimationFrame( af, t, nFrames ) ;
			//
			S3DDVector	vZoom = pvZooms[af.iFrame0] ;
			vZoom += (pvZooms[af.iFrame1]
						- pvZooms[af.iFrame0]) * af.fpDelta ;
			//
			meBone.vZoom = vZoom ;
		}
		else
		{
			meBone.vZoom = ja.m_vZoom ;
		}
		//
		// 平行移動
		//
		if ( ja.m_aOffsets.GetLength() >= 2 )
		{
			const S3DDVector *	pvOffsets = ja.m_aOffsets.GetConstArray() ;
			const size_t	nFrames = ja.m_aOffsets.GetLength() ;
			AnimationFrame	af ;
			CalculateAnimationFrame( af, t, nFrames ) ;
			//
			vBoneMove = pvOffsets[af.iFrame0] ;
			vBoneMove += (pvOffsets[af.iFrame1]
							- pvOffsets[af.iFrame0]) * af.fpDelta ;
		}
		else
		{
			vBoneMove = ja.m_vOffset ;
		}
	}
	//
	// 物理演算ブレンド
	//
	if ( ja.m_aPhysBlends.GetLength() >= 2 )
	{
		const float32_t *	pwPhysBlends = ja.m_aPhysBlends.GetConstArray() ;
		const size_t	nFrames = ja.m_aPhysBlends.GetLength() ;
		AnimationFrame	af ;
		CalculateAnimationFrame( af, t, nFrames ) ;
		//
		wPhysBlend = pwPhysBlends[af.iFrame0] ;
		wPhysBlend +=
			(pwPhysBlends[af.iFrame1]
				- pwPhysBlends[af.iFrame0]) * af.fpDelta ;
	}
	else
	{
		wPhysBlend = ja.m_wPhysBlend ;
	}
}

// モーフィングターゲット指標取得
//////////////////////////////////////////////////////////////////////////////
void S3DModelPose::PrepareMorphTargetIndex
	( S3DModelPose::MorphContext& mc,
		const S3DModelPose::MorphInfo& mi,
		const S3DModelData::MeshObject * pMesh,
		const S3DModelData::MeshDivision * pMeshDiv,
		const S3DModelData::MeshDivision::SplittedEntry * pSplitted )
{
	if ( pMeshDiv == nullptr )
	{
		mc.iTarget = pMesh->FindMorphTarget( mi.m_strMorphTarget ) ;
	}
	else
	{
		mc.iTarget =
			pMesh->FindMorphTarget
				( pMeshDiv->MapMorphTargetAs( pSplitted, mi.m_strMorphTarget ) ) ;
	}
	//
	const size_t	nTargets = mi.m_aTargetIDs.GetLength() ;
	ssize_t *		pTargetMap = mc.aTarget.GetArray( nTargets ) ;
	mc.nTargetCount = nTargets ;
	mc.aTarget.SetLength( nTargets ) ;
	for ( size_t j = 0; j < nTargets; j ++ )
	{
		SString *	pstrTargetID = mi.m_aTargetIDs.GetAt( j ) ;
		if ( pstrTargetID == NULL )
		{
			continue ;
		}
		if ( pMeshDiv == nullptr )
		{
			pTargetMap[j] = pMesh->FindMorphTarget( *pstrTargetID ) ;
		}
		else
		{
			pTargetMap[j] =
				pMesh->FindMorphTarget
					( pMeshDiv->MapMorphTargetAs( pSplitted, *pstrTargetID ) ) ;
		}
	}
	mc.aTarget.FinishArray() ;
}

// モーフィングフレーム取得
//////////////////////////////////////////////////////////////////////////////
void S3DModelPose::CalcMorphFrameContext
	( S3DModelPose::MorphContext& mc,
		const S3DModelPose::MorphInfo& mi, double t /* sec */ ) const
{
	if ( mi.m_aAnimation.GetLength() >= 1 )
	{
		// フレーム補完
		ESLAssert( mc.nTargetCount == mi.m_aTargetIDs.GetLength() ) ;
		size_t	nFrames = mi.m_aAnimation.GetLength() ;
//		mc.nTargetCount = mi.m_aTargetIDs.GetLength() ;
		if ( mc.nTargetCount > 0 )
		{
			nFrames /= mc.nTargetCount ;
		}
		AnimationFrame	af ;
		CalculateAnimationFrame( af, t, nFrames ) ;
		//
		ESLAssert( mc.nTargetCount == mc.aTarget.GetLength() ) ;
		float32_t *	pWeights = mc.aWeight.GetArray( mc.nTargetCount ) ;
		//
		const float32_t *	pw0 = mi.m_aAnimation.GetAt
										( af.iFrame0 * mc.nTargetCount ) ;
		const float32_t *	pw1 = mi.m_aAnimation.GetAt
										( af.iFrame1 * mc.nTargetCount ) ;
		for ( size_t j = 0; j < mc.nTargetCount; j ++ )
		{
			pWeights[j] =
				(float32_t) (pw0[j] + (pw1[j] - pw0[j]) * af.fpDelta) ;
		}
		mc.aWeight.FinishArray() ;
	}
	else
	{
		ESLAssert( mc.nTargetCount == mi.m_aTargetIDs.GetLength() ) ;
		const ssize_t *	pTargets = mc.aTarget.GetConstArray() ;
		float32_t *		pWeights = mc.aWeight.GetArray( mc.nTargetCount ) ;
		for ( size_t j = 0; j < mc.nTargetCount; j ++ )
		{
			if ( mc.iTarget == pTargets[j] )
			{
				pWeights[j] = 1.0f ;
			}
			else
			{
				pWeights[j] = 0.0f ;
			}
		}
		mc.aWeight.FinishArray() ;
	}
}

// モーフィング合成のために全ターゲット順配列に正規化
//////////////////////////////////////////////////////////////////////////////
void S3DModelPose::NormalizeMorphContext
	( S3DModelPose::MorphContext& mcDst,
		const S3DModelPose::MorphContext& mcSrc, size_t nTotalTargetCount )
{
	size_t				nOldCount = mcSrc.aTarget.GetLength() ;
	const ssize_t *		pSrcTargets = mcSrc.aTarget.GetConstArray() ;
	const float32_t *	pSrcWeight = mcSrc.aWeight.GetConstArray() ;
	ssize_t *			pDstTargets = mcDst.aTarget.GetArray( nTotalTargetCount ) ;
	float32_t *			pDstWeight = mcDst.aWeight.GetArray( nTotalTargetCount ) ;
	mcDst.nTargetCount = nTotalTargetCount ;
	//
	size_t	i ;
	for ( i = 0; i < nTotalTargetCount; i ++ )
	{
		pDstTargets[i] = (ssize_t) i - 1 ;
		pDstWeight[i] = 0.0f ;
	}
	for ( i = 0; i < nOldCount; i ++ )
	{
		ssize_t	iTarget = pSrcTargets[i] + 1 ;
		if ( (size_t) iTarget < nTotalTargetCount )
		{
			pDstWeight[iTarget] = pSrcWeight[i] ;
		}
	}
	mcDst.aTarget.FinishArray() ;
	mcDst.aWeight.FinishArray() ;
	mcDst.aTarget.SetLength( nTotalTargetCount ) ;
	mcDst.aWeight.SetLength( nTotalTargetCount ) ;
}

// モーフィングターゲットから使用されていないものを削除
//////////////////////////////////////////////////////////////////////////////
void S3DModelPose::TrimMorphContext( S3DModelPose::MorphContext& mc )
{
	ESLAssert( mc.aTarget.GetLength() >= mc.nTargetCount ) ;
	ESLAssert( mc.aWeight.GetLength() >= mc.nTargetCount ) ;
	ssize_t *	pTargets = mc.aTarget.GetArray() ;
	float32_t *	pWeight = mc.aWeight.GetArray() ;
	size_t		nCount = mc.nTargetCount ;
	size_t		iDst = 0 ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		if ( pWeight[i] > 1.0e-8 )
		{
			pTargets[iDst] = pTargets[i] ;
			pWeight[iDst] = pWeight[i] ;
			iDst ++ ;
		}
	}
	mc.aTarget.FinishArray() ;
	mc.aWeight.FinishArray() ;
	//
	mc.nTargetCount = iDst ;
}

// モーフィングコンテキスト合成
//////////////////////////////////////////////////////////////////////////////
void S3DModelPose::BlendMorphContext
	( S3DModelPose::MorphContext& mcDst,
		const S3DModelPose::MorphContext& mcSrc, double w )
{
	size_t	nCount = mcDst.nTargetCount ;
	if ( nCount > mcSrc.nTargetCount )
	{
		nCount = mcSrc.nTargetCount ;
	}
	ESLAssert( mcDst.aTarget.GetLength() >= nCount ) ;
	ESLAssert( mcSrc.aTarget.GetLength() >= nCount ) ;
	const ssize_t *		pDstTargets = mcDst.aTarget.GetConstArray() ;
	float32_t *			pDstWeight = mcDst.aWeight.GetArray() ;
	const ssize_t *		pSrcTargets = mcSrc.aTarget.GetConstArray() ;
	const float32_t *	pSrcWeight = mcSrc.aWeight.GetConstArray() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		ESLAssert( pDstTargets[i] == pSrcTargets[i] ) ;
		if ( pDstTargets[i] == pSrcTargets[i] )
		{
			pDstWeight[i] += (pSrcWeight[i] - pDstWeight[i]) * (float32_t) w ;
		}
	}
	mcDst.aWeight.FinishArray() ;
}

void S3DModelPose::BlendAddMorphContext
	( MorphContext& mcDst,
		const MorphContext& mcSrc, double w )
{
	size_t	nCount = mcDst.nTargetCount ;
	if ( nCount > mcSrc.nTargetCount )
	{
		nCount = mcSrc.nTargetCount ;
	}
	ESLAssert( mcDst.aTarget.GetLength() >= nCount ) ;
	ESLAssert( mcSrc.aTarget.GetLength() >= nCount ) ;
	const ssize_t *		pDstTargets = mcDst.aTarget.GetConstArray() ;
	float32_t *			pDstWeight = mcDst.aWeight.GetArray() ;
	const ssize_t *		pSrcTargets = mcSrc.aTarget.GetConstArray() ;
	const float32_t *	pSrcWeight = mcSrc.aWeight.GetConstArray() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		ESLAssert( pDstTargets[i] == pSrcTargets[i] ) ;
		if ( pDstTargets[i] == pSrcTargets[i] )
		{
			pDstWeight[i] = esl_fclampf
				( pDstWeight[i] + pSrcWeight[i] * (float32_t) w, 0.0f, 1.0f ) ;
		}
	}
	mcDst.aWeight.FinishArray() ;
}

// メッシュ表示状態
//////////////////////////////////////////////////////////////////////////////
bool S3DModelPose::GetMeshSelectorFrameVisible
	( const S3DModelPose::MeshSelector& ms, double t /* sec */ ) const
{
	if ( ms.m_aVisibles.GetLength() >= 1 )
	{
		AnimationFrame	af ;
		CalculateAnimationFrame( af, t, ms.m_aVisibles.GetLength() ) ;
		ESLAssert( (size_t) af.iFrame0 < ms.m_aVisibles.GetLength() ) ;
		return	(ms.m_aVisibles.At( (size_t) af.iFrame0 ) != 0) ;
	}
	else
	{
		return	ms.m_flagVisible ;
	}
}

// ポーズを合成のために存在しないボーンやメッシュターゲットを追加
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelPose::PrepareBlendPoseTarget( const S3DModelPose& pose )
{
	size_t	i ;
	for ( i = 0; i < pose.GetJointCount(); i ++ )
	{
		S3DModelPose::JointAnimation *	pjaSrc = pose.GetJointAt( i ) ;
		if ( (pjaSrc != NULL)
			&& !pjaSrc->m_fDisabled
			&& (GetJointAs( pose.GetJointNameAt( i ) ) == NULL) )
		{
			S3DModelPose::JointAnimation *
					pja = new S3DModelPose::JointAnimation ;
			pja->m_qRotation = pjaSrc->m_qRotation ;
			pja->m_vZoom = pjaSrc->m_vZoom ;
			pja->m_vOffset = pjaSrc->m_vOffset ;
			pja->m_vHandle = pjaSrc->m_vHandle ;
			pja->m_zRotation = pjaSrc->m_zRotation ;
			AddJointAs( pose.GetJointNameAt( i ), pja ) ;
		}
	}
	for ( i = 0; i < pose.GetMorphingCount(); i ++ )
	{
		S3DModelPose::MorphInfo *	pmiSrc = pose.GetMorphingAt( i ) ;
		ESLAssert( pmiSrc != NULL ) ;
		if ( (pmiSrc == NULL) || pmiSrc->m_fDisabled )
		{
			continue ;
		}
		S3DModelPose::MorphInfo *	pmiDst =
					GetMorphingAs( pose.GetMorphingMeshNameAt( i ) ) ;
		if ( pmiDst == NULL )
		{
			pmiDst = new S3DModelPose::MorphInfo ;
			pmiDst->m_aTargetIDs.AllocateString( L"" ) ;
			AddMorphingAs( pose.GetMorphingMeshNameAt( i ), pmiDst ) ;
		}
		pmiDst->m_aTargetIDs.AllocateString( pmiSrc->m_strMorphTarget ) ;
		for ( size_t k = 0; k < pmiSrc->m_aTargetIDs.GetLength(); k ++ )
		{
			ESLAssert( pmiSrc->m_aTargetIDs.GetAt( k ) != NULL ) ;
			pmiDst->m_aTargetIDs.AllocateString
							( pmiSrc->m_aTargetIDs.At( k ) ) ;
		}
	}
	for ( i = 0; i < pose.GetMeshSelectorCount(); i ++ )
	{
		S3DModelPose::MeshSelector *	pmsSrc = pose.GetMeshSelectorAt( i ) ;
		if ( (pmsSrc == NULL) || pmsSrc->m_fDisabled )
		{
			continue ;
		}
		const wchar_t *	pwszMeshID = pose.GetSelectedMeshNameAt( i ) ;
		ESLAssert( pwszMeshID != NULL ) ;
		if ( GetMeshSelectorAs( pwszMeshID ) == NULL )
		{
			S3DModelPose::MeshSelector *
					pms = new S3DModelPose::MeshSelector ;
			pms->m_flagVisible = true ;
			AddMeshSelectorAs( pwszMeshID, pms ) ;
		}
	}
	for ( i = 0; i < pose.GetMaterialSelectorCount(); i ++ )
	{
		S3DModelPose::MaterialSelector *
						pmsSrc = pose.GetMaterialSelectorAt( i ) ;
		if ( (pmsSrc == NULL) || pmsSrc->m_fDisabled )
		{
			continue ;
		}
		const wchar_t *	pwszMeshID = pose.GetSelMaterialMeshNameAt( i ) ;
		ESLAssert( pwszMeshID != NULL ) ;
		if ( GetMaterialSelectorAs( pwszMeshID ) == NULL )
		{
			S3DModelPose::MaterialSelector *
					pms = new S3DModelPose::MaterialSelector ;
			AddMaterialSelectorAs( pwszMeshID, pms ) ;
		}
	}
	return	sglErrSuccess ;
}

// ポーズを合成のために存在しないボーンやメッシュターゲットを追加
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelPose::PrepareBlendPoseAllTarget
		( const S3DModelBuffer& model, uint32_t nFlags )
{
	if ( nFlags & poseTargetBone )
	{
		const SStrSortObjectArray<S3DModelBoneSpace>&
					ssoaBones = model.ConstBonePropertyList() ;
		for ( size_t i = 0; i < ssoaBones.GetLength(); i ++ )
		{
			S3DModelBoneSpace *	pBone = ssoaBones.GetAt( i ) ;
			const SString *		pstrID = ssoaBones.GetTagAt( i ) ;
			if ( (pBone == NULL)
				|| (pstrID == NULL)
				|| (pBone->GetBoneFlags()
						& S3DModelBoneSpace::flagBonePhysics) )
			{
				continue ;
			}
			if ( GetJointAs( *pstrID ) == NULL )
			{
				S3DModelPose::JointAnimation *
						pja = new S3DModelPose::JointAnimation ;
				MatrixElement	me ;
				me.FromMatrix( pBone->m_matTransformation ) ;
				pja->m_qRotation = me.qRotation ;
				pja->m_vZoom = me.vZoom ;
				pja->m_vOffset = pBone->GetBoneOffset() ;
				pja->m_vHandle = pBone->GetBoneHandle() ;
				AddJointAs( *pstrID, pja ) ;
			}
		}
	}
	if ( nFlags & poseTargetMorph )
	{
		const SStrSortArray<S3DModelBuffer::MeshGroup>&
							ssaMesh = model.GetMeshGroupList() ;
		for ( size_t i = 0; i < ssaMesh.GetLength(); i ++ )
		{
			S3DModelBuffer::MeshGroup *	pmg = ssaMesh.GetAt( i ) ;
			const SString *				pstrID = ssaMesh.GetTagAt( i ) ;
			if ( (pmg == NULL)
				|| (pstrID == NULL) )
			{
				continue ;
			}
			S3DModelBuffer::MeshObject *
				pMeshObj = model.GetMeshObjectAt( pmg->m_iFirstMesh ) ;
			if ( (pMeshObj == NULL)
				|| (pMeshObj->m_arrMorphTarget.GetLength() > 0) )
			{
				continue ;
			}
			S3DModelPose::MorphInfo *	pmiDst = GetMorphingAs( *pstrID ) ;
			if ( pmiDst == NULL )
			{
				pmiDst = new S3DModelPose::MorphInfo ;
				pmiDst->m_aTargetIDs.AllocateString( L"" ) ;
				AddMorphingAs( *pstrID, pmiDst ) ;
			}
			for ( size_t k = 0; k < pMeshObj->m_arrMorphTarget.GetLength(); k ++ )
			{
				ESLAssert( pMeshObj->m_arrMorphTarget.GetAt( k ) != NULL ) ;
				pmiDst->m_aTargetIDs.AllocateString
								( pMeshObj->m_arrMorphTarget.At( k ) ) ;
			}
		}
	}
	if ( nFlags & poseTargetMeshSel )
	{
		const SStrSortArray<S3DModelBuffer::MeshGroup>&
							ssaMesh = model.GetMeshGroupList() ;
		for ( size_t i = 0; i < ssaMesh.GetLength(); i ++ )
		{
			S3DModelBuffer::MeshGroup *	pmg = ssaMesh.GetAt( i ) ;
			const SString *				pstrID = ssaMesh.GetTagAt( i ) ;
			if ( (pmg == NULL)
				|| (pstrID == NULL) )
			{
				continue ;
			}
			S3DModelBuffer::MeshObject *
				pMeshObj = model.GetMeshObjectAt( pmg->m_iFirstMesh ) ;
			if ( (pMeshObj == NULL)
				|| (pMeshObj->m_arrMorphTarget.GetLength() > 0) )
			{
				continue ;
			}
			if ( GetMeshSelectorAs( *pstrID ) == NULL )
			{
				S3DModelPose::MeshSelector *
						pms = new S3DModelPose::MeshSelector ;
				pms->m_flagVisible = true ;
				AddMeshSelectorAs( *pstrID, pms ) ;
			}
		}
	}
	return	sglErrSuccess ;
}

// アニメーションの全長フレーム数を設定
//////////////////////////////////////////////////////////////////////////////
void S3DModelPose::AllocateAnimationFrameCount( size_t nFrameCount )
{
	size_t	i ;
	for ( i = 0; i < GetJointCount(); i ++ )
	{
		S3DModelPose::JointAnimation *	pja = GetJointAt( i ) ;
		if ( pja != NULL )
		{
			pja->m_aMatrixs.SetLength( nFrameCount ) ;
			pja->m_aPhysBlends.SetLength( nFrameCount ) ;
		}
	}
	for ( i = 0; i < GetMorphingCount(); i ++ )
	{
		S3DModelPose::MorphInfo *	pmi = GetMorphingAt( i ) ;
		if ( pmi != NULL )
		{
			pmi->m_aAnimation.SetLength
				( nFrameCount * pmi->m_aTargetIDs.GetLength() ) ;
		}
	}
	for ( i = 0; i < GetMeshSelectorCount(); i ++ )
	{
		MeshSelector *	pms = GetMeshSelectorAt( i ) ;
		if ( pms != NULL )
		{
			pms->m_aVisibles.SetLength( nFrameCount ) ;
		}
	}
	for ( i = 0; i < GetMaterialSelectorCount(); i ++ )
	{
		MaterialSelector *	pms = GetMaterialSelectorAt( i ) ;
		if ( pms != NULL )
		{
			pms->m_aAnimation.SetLength( nFrameCount ) ;
		}
	}
}

// 静止ポーズ用にバッファを確保
//////////////////////////////////////////////////////////////////////////////
void S3DModelPose::AllocateStillPoseBuffer( void )
{
	size_t	i ;
	for ( i = 0; i < GetMorphingCount(); i ++ )
	{
		S3DModelPose::MorphInfo *	pmi = GetMorphingAt( i ) ;
		if ( pmi != NULL )
		{
			pmi->m_aAnimation.SetLength
				( pmi->m_aTargetIDs.GetLength() ) ;
		}
	}
	for ( i = 0; i < GetMaterialSelectorCount(); i ++ )
	{
		MaterialSelector *	pms = GetMaterialSelectorAt( i ) ;
		if ( pms != NULL )
		{
			pms->m_aAnimation.SetLength( 1 ) ;
		}
	}
}

// アニメーションフレームを合成
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelPose::BlendAnimationFrame
	( double secDst,
		const S3DModelPose& pose, double secFrame, double wBlend )
{
	const size_t	nFrameCount =
		(size_t) (((uint64_t) m_metaInfo.msecDuration
					* m_metaInfo.fxFrameRatio + 0xFFFF) / (1000 * 0x10000)) ;
	size_t	i ;
	for ( i = 0; i < pose.GetJointCount(); i ++ )
	{
		S3DModelPose::JointAnimation *	pjaSrc = pose.GetJointAt( i ) ;
		S3DModelPose::JointAnimation *
					pjaDst = GetJointAs( pose.GetJointNameAt( i ) ) ;
		if ( pjaSrc && pjaDst )
		{
			MatrixElement	meBone ;
			S3DDVector		vBoneMove ;
			double			wPhysBlend ;
			CalcJointFrameMatrix
				( meBone, vBoneMove, wPhysBlend, *pjaSrc, secFrame ) ;
			//
			if ( wBlend < 1.0 - 1.0e-8 )
			{
				MatrixElement	meDst ;
				S3DDVector		vDstMove ;
				double			wDstBlend ;
				CalcJointFrameMatrix
					( meDst, vDstMove, wDstBlend, *pjaDst, secDst ) ;
				//
				meBone.Slerp( meDst, meBone, wBlend ) ;
				vBoneMove = vDstMove * (1.0 - wBlend)
								+ vBoneMove * wBlend ;
				wPhysBlend = wDstBlend * (1.0 - wBlend)
								+ wPhysBlend * wBlend ;
			}
			AnimationFrame	af ;
			CalculateAnimationFrame( af, secDst, nFrameCount ) ;
			//
			S4DMatrix *
				pMatrixDst = pjaDst->m_aMatrixs.GetAt( af.iFrame0 ) ;
			if ( pMatrixDst != NULL )
			{
				Matrix4x4From3x3
					<float32_t,S3DDMatrix,S3DDVector>
						( *pMatrixDst, meBone.matTransform, vBoneMove ) ;
			}
			S3DDQuaternion *
				pRotDst = pjaDst->m_aRotations.GetAt( af.iFrame0 ) ;
			if ( pRotDst != NULL )
			{
				*pRotDst = meBone.qRotation ;
			}
			S3DDVector *
				pOffsetDst = pjaDst->m_aOffsets.GetAt( af.iFrame0 ) ;
			if ( pOffsetDst != NULL )
			{
				*pOffsetDst = vBoneMove ;
			}
			S3DDVector *
				pZoomDst = pjaDst->m_aZooms.GetAt( af.iFrame0 ) ;
			if ( pZoomDst != NULL )
			{
				*pZoomDst = meBone.vZoom ;
			}
			float32_t *
				pPhysBlend = pjaDst->m_aPhysBlends.GetAt( af.iFrame0 ) ;
			if ( pPhysBlend != NULL )
			{
				*pPhysBlend = (float32_t) wPhysBlend ;
			}
			if ( secDst == 0.0 )
			{
				pjaDst->m_qRotation = meBone.qRotation ;
				pjaDst->m_vZoom = meBone.vZoom ;
				pjaDst->m_vOffset = vBoneMove ;
				pjaDst->m_wPhysBlend = wPhysBlend ;
			}
		}
	}
	for ( i = 0; i < pose.GetMorphingCount(); i ++ )
	{
		S3DModelPose::MorphInfo *	pmiSrc = pose.GetMorphingAt( i ) ;
		ESLAssert( pmiSrc != NULL ) ;
		S3DModelPose::MorphInfo *	pmiDst =
					GetMorphingAs( pose.GetMorphingMeshNameAt( i ) ) ;
		if ( pmiDst != NULL )
		{
			MorphContext	mcSrc ;
			mcSrc.iTarget = pmiDst->m_aTargetIDs.
								GetStringIndex( pmiSrc->m_strMorphTarget ) ;
			size_t		j ;
			ssize_t *	pSrcTargets =
				mcSrc.aTarget.GetArray( pmiSrc->m_aTargetIDs.GetLength() ) ;
			for ( j = 0; j < pmiSrc->m_aTargetIDs.GetLength(); j ++ )
			{
				pSrcTargets[j] =
					pmiDst->m_aTargetIDs.
						GetStringIndex( pmiSrc->m_aTargetIDs.At(j) ) ;
			}
			mcSrc.aTarget.FinishArray() ;
			CalcMorphFrameContext( mcSrc, *pmiSrc, secFrame ) ;
			//
			MorphContext	mcDst ;
			const size_t	nDstTargetCount = pmiDst->m_aTargetIDs.GetLength() ;
			mcDst.iTarget =
				pmiDst->m_aTargetIDs.
						GetStringIndex( pmiDst->m_strMorphTarget ) ;
			ssize_t *	pDstTargets =
				mcDst.aTarget.GetArray( nDstTargetCount ) ;
			for ( j = 0; j < nDstTargetCount; j ++ )
			{
				pDstTargets[j] = (ssize_t) j ;
			}
			mcDst.aTarget.FinishArray() ;
			CalcMorphFrameContext( mcDst, *pmiDst, secDst ) ;
			//
			if ( mcDst.nTargetCount == nDstTargetCount )
			{
				float32_t *			pwDstWeight = mcDst.aWeight.GetArray() ;
				const float32_t *	pwSrcWeight = mcSrc.aWeight.GetConstArray() ;
				const ssize_t *		pSrcTargets = mcSrc.aTarget.GetConstArray() ;
				//
				if ( wBlend < 1.0 - 1.0e-8 )
				{
					for ( j = 0; j < mcSrc.nTargetCount; j ++ )
					{
						ssize_t	k = pSrcTargets[j] ;
						if ( (k >= 0) && ((size_t) k < nDstTargetCount) )
						{
							pwDstWeight[k] =
								pwDstWeight[k] * (float32_t) (1.0 - wBlend)
								+ pwSrcWeight[j] * (float32_t) wBlend ;
						}
					}
				}
				else
				{
					for ( j = 0; j < mcSrc.nTargetCount; j ++ )
					{
						ssize_t	k = pSrcTargets[j] ;
						if ( (k >= 0) && ((size_t) k < nDstTargetCount) )
						{
							pwDstWeight[k] = pwSrcWeight[j] ;
						}
					}
				}
				//
				AnimationFrame	af ;
				CalculateAnimationFrame( af, secDst, nFrameCount ) ;
				//
				if ( (af.iFrame0 + 1) * nDstTargetCount
							<= pmiDst->m_aAnimation.GetLength() )
				{
					eslCopyMemory
						( pmiDst->m_aAnimation.GetAt
								( af.iFrame0 * nDstTargetCount ),
							pwDstWeight,
							nDstTargetCount * sizeof(float32_t) ) ;
				}
				mcDst.aWeight.FinishArray() ;
			}
		}
	}
	for ( i = 0; i < pose.GetMeshSelectorCount(); i ++ )
	{
		S3DModelPose::MeshSelector *
				pmsSrc = pose.GetMeshSelectorAt( i ) ;
		ESLAssert( pmsSrc != NULL ) ;
		S3DModelPose::MeshSelector *
				pmsDst = GetMeshSelectorAs
							( pose.GetSelectedMeshNameAt( i ) ) ;
		if ( pmsDst != NULL )
		{
			if ( wBlend >= 0.99999 )
			{
				if ( pmsDst->m_aVisibles.GetLength() >= 1 )
				{
					AnimationFrame	af ;
					CalculateAnimationFrame
						( af, secDst, pmsDst->m_aVisibles.GetLength() ) ;
					ESLAssert( (size_t) af.iFrame0 < pmsDst->m_aVisibles.GetLength() ) ;
					pmsDst->m_aVisibles.SetAt
						( af.iFrame0,
							GetMeshSelectorFrameVisible( *pmsSrc, secFrame ) ) ;
				}
				if ( secDst == 0.0 )
				{
					pmsDst->m_flagVisible =
						GetMeshSelectorFrameVisible( *pmsSrc, secFrame ) ;
				}
			}
		}
	}
	for ( i = 0; i < pose.GetMaterialSelectorCount(); i ++ )
	{
		S3DModelPose::MaterialSelector *
				pmsSrc = pose.GetMaterialSelectorAt( i ) ;
		ESLAssert( pmsSrc != NULL ) ;
		S3DModelPose::MaterialSelector *
				pmsDst = GetMaterialSelectorAs
							( pose.GetSelMaterialMeshNameAt( i ) ) ;
		if ( pmsDst != NULL )
		{
			if ( wBlend >= 0.99999 )
			{
				if ( pmsDst->m_aAnimation.GetLength() >= 1 )
				{
					AnimationFrame	af ;
					CalculateAnimationFrame
						( af, secDst, pmsDst->m_aAnimation.GetLength() ) ;
					ESLAssert( (size_t) af.iFrame0 < pmsDst->m_aAnimation.GetLength() ) ;
					if ( (size_t) af.iFrame0 < pmsSrc->m_aAnimation.GetLength() )
					{
						pmsDst->m_aAnimation.SetAt
							( af.iFrame0, pmsSrc->m_aAnimation.At( af.iFrame0 ) ) ;
					}
				}
			}
		}
	}
	return	sglErrSuccess ;
}

// モーション区間を切り出し複製
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelPose::DuplicatePoseDuration
	( const S3DModelPose& pose, int iFirstFrame, int iEndFrame )
{
	//
	// アニメーション情報
	//
	S3DModelPose::MetaInfo	metaInfo = pose.GetMetaInfo() ;
	if ( iEndFrame < 0 )
	{
		iEndFrame = (int) ((uint64_t) metaInfo.msecDuration
										* metaInfo.fxFrameRatio
										/ (1000 * 0x10000)) ;
	}
	size_t	nFrameCount = (size_t) (iEndFrame - iFirstFrame + 1) ;
	metaInfo.msecDuration =
		(uint32_t) ((int64_t) nFrameCount
						* (1000 * 0x10000) / metaInfo.fxFrameRatio) ;
	SetMetaInfo( metaInfo ) ;
	//
	// 各ボーンアニメーション切り出し
	//
	size_t	i ;
	for ( i = 0; i < pose.GetJointCount(); i ++ )
	{
		S3DModelPose::JointAnimation *	pjaSrc = pose.GetJointAt( i ) ;
		if ( pjaSrc == NULL )
		{
			continue ;
		}
		S3DModelPose::JointAnimation *
				pja = new S3DModelPose::JointAnimation ;
		pja->m_qRotation = pjaSrc->m_qRotation ;
		pja->m_vZoom = pjaSrc->m_vZoom ;
		pja->m_vOffset = pjaSrc->m_vOffset ;
		pja->m_vHandle = pjaSrc->m_vHandle ;
		pja->m_zRotation = pjaSrc->m_zRotation ;
		pja->m_wPhysBlend = pjaSrc->m_wPhysBlend ;
		pja->m_nExFlags = pjaSrc->m_nExFlags ;
		pja->m_mat4OrgLocal = pjaSrc->m_mat4OrgLocal ;
		pja->m_fDisabled = pjaSrc->m_fDisabled ;
		//
		if ( pjaSrc->m_aMatrixs.GetLength() > (size_t) iFirstFrame )
		{
			pja->m_aMatrixs.Merge
				( 0, pjaSrc->m_aMatrixs,
						iFirstFrame, (ssize_t) nFrameCount ) ;
			//
			S3DDMatrix	matSrc0 ;
			S3DDVector	vSrc0 ;
			Matrix3x3From4x4<S3DDMatrix,S3DDVector,float32_t>
						( matSrc0, vSrc0, pja->m_aMatrixs.At(0) ) ;
			//
			MatrixElement	me ;
			me.FromMatrix( matSrc0 ) ;
			//
			pja->m_qRotation = me.qRotation ;
			pja->m_vZoom = me.vZoom ;
			pja->m_vOffset = vSrc0 ;
		}
		if ( pjaSrc->m_aRotations.GetLength() > (size_t) iFirstFrame )
		{
			pja->m_aRotations.Merge
				( 0, pjaSrc->m_aRotations,
						iFirstFrame, (ssize_t) nFrameCount ) ;
			pja->m_qRotation = pja->m_aRotations.At(0) ;
		}
		if ( pjaSrc->m_aOffsets.GetLength() > (size_t) iFirstFrame )
		{
			pja->m_aOffsets.Merge
				( 0, pjaSrc->m_aOffsets,
						iFirstFrame, (ssize_t) nFrameCount ) ;
			pja->m_vOffset = pja->m_aOffsets.At(0) ;
		}
		if ( pjaSrc->m_aZooms.GetLength() > (size_t) iFirstFrame )
		{
			pja->m_aZooms.Merge
				( 0, pjaSrc->m_aZooms,
						iFirstFrame, (ssize_t) nFrameCount ) ;
			pja->m_vZoom = pja->m_aZooms.At(0) ;
		}
		if ( pjaSrc->m_aPhysBlends.GetLength() > (size_t) iFirstFrame )
		{
			pja->m_aPhysBlends.Merge
				( 0, pjaSrc->m_aPhysBlends,
						iFirstFrame, (ssize_t) nFrameCount ) ;
			pja->m_wPhysBlend = pja->m_aPhysBlends.At(0) ;
		}
		AddJointAs( pose.GetJointNameAt( i ), pja ) ;
	}
	//
	// 各モーフィングアニメーション切り出し
	//
	for ( i = 0; i < pose.GetMorphingCount(); i ++ )
	{
		S3DModelPose::MorphInfo *	pmiSrc = pose.GetMorphingAt( i ) ;
		if ( pmiSrc == NULL )
		{
			continue ;
		}
		S3DModelPose::MorphInfo *
				pmi = new S3DModelPose::MorphInfo ;
		pmi->m_fDisabled = pmiSrc->m_fDisabled ;
		pmi->m_strMorphTarget = pmiSrc->m_strMorphTarget ;
		pmi->m_aTargetIDs = pmiSrc->m_aTargetIDs ;
		//
		size_t	nStride = pmiSrc->m_aTargetIDs.GetLength() ;
		if ( pmiSrc->m_aAnimation.GetLength() > iFirstFrame * nStride )
		{
			pmi->m_aAnimation.Merge
				( 0, pmiSrc->m_aAnimation,
					iFirstFrame * nStride,
					(ssize_t) (nFrameCount * nStride) ) ;
		}
		AddMorphingAs( pose.GetMorphingMeshNameAt( i ), pmi ) ;
	}
	//
	// メッシュ表示状態
	//
	for ( i = 0; i < pose.GetMeshSelectorCount(); i ++ )
	{
		S3DModelPose::MeshSelector *
					pmsSrc = pose.GetMeshSelectorAt( i ) ;
		if ( pmsSrc == NULL )
		{
			continue ;
		}
		S3DModelPose::MeshSelector *
				pms = new S3DModelPose::MeshSelector ;
		pms->m_fDisabled = pmsSrc->m_fDisabled ;
		pms->m_flagVisible = pmsSrc->m_flagVisible ;
		//
		if ( pmsSrc->m_aVisibles.GetLength() > (size_t) iFirstFrame )
		{
			pms->m_aVisibles.Merge
				( 0, pmsSrc->m_aVisibles,
						iFirstFrame, (ssize_t) nFrameCount ) ;
			pms->m_flagVisible = (pms->m_aVisibles.At(0) != 0) ;
		}
		AddMeshSelectorAs( pose.GetSelectedMeshNameAt( i ), pms ) ;
	}
	//
	// マテリアル表示状態
	//
	for ( i = 0; i < pose.GetMaterialSelectorCount(); i ++ )
	{
		S3DModelPose::MaterialSelector *
					pmsSrc = pose.GetMaterialSelectorAt( i ) ;
		if ( pmsSrc == NULL )
		{
			continue ;
		}
		S3DModelPose::MaterialSelector *
				pms = new S3DModelPose::MaterialSelector ;
		pms->m_fDisabled = pmsSrc->m_fDisabled ;
		//
		if ( pmsSrc->m_aAnimation.GetLength() > (size_t) iFirstFrame )
		{
			pms->m_aAnimation.Merge
				( 0, pmsSrc->m_aAnimation,
						iFirstFrame, (ssize_t) nFrameCount ) ;
		}
		AddMaterialSelectorAs( pose.GetSelMaterialMeshNameAt( i ), pms ) ;
	}
	return	sglErrSuccess ;
}

// ポーズに含まれる参照先メッシュIDの変更
//////////////////////////////////////////////////////////////////////////////
void S3DModelPose::ChangeTargetMeshID
	( const wchar_t * pwszOldID, const wchar_t * pwszNewID )
{
	MorphInfo *	pmi = m_ssoaMorph.DetachAs( pwszOldID ) ;
	if ( pmi != NULL )
	{
		m_ssoaMorph.Add( pwszNewID, pmi ) ;
	}
	MeshSelector *	pms = m_ssoaMeshSel.DetachAs( pwszOldID ) ;
	if ( pms != NULL )
	{
		m_ssoaMeshSel.Add( pwszNewID, pms ) ;
	}
	MaterialSelector *	pmts = m_ssoaMaterialSel.DetachAs( pwszOldID ) ;
	if ( pms != NULL )
	{
		m_ssoaMaterialSel.Add( pwszNewID, pmts ) ;
	}
	//
	m_pLastPoseTarget = NULL ;
}

// 参照先メッシュの統合
//////////////////////////////////////////////////////////////////////////////
void S3DModelPose::MergeTargetMeshID
	( const wchar_t * pwszMergeTarget,
		const wchar_t *const* ppwszMergeSources, size_t nSourceCount )
{
	//
	// モーフィングの統合
	//
	size_t	i, j ;
	MorphInfo *	pmiTarget = GetMorphingAs( pwszMergeTarget ) ;
	for ( i = 0; i < nSourceCount; i ++ )
	{
		MorphInfo *	pmiSrc = GetMorphingAs( ppwszMergeSources[i] ) ;
		if ( pmiSrc == NULL )
		{
			continue ;
		}
		if ( pmiTarget == NULL )
		{
			pmiTarget = new MorphInfo( *pmiSrc ) ;
			AddMorphingAs( pwszMergeTarget, pmiTarget ) ;
		}
		else
		{
			pmiTarget->m_fDisabled =
					pmiTarget->m_fDisabled && pmiSrc->m_fDisabled ;
			if ( pmiTarget->m_strMorphTarget.IsEmpty()
					&& !pmiSrc->m_strMorphTarget.IsEmpty() )
			{
				pmiTarget->m_strMorphTarget = pmiSrc->m_strMorphTarget ;
			}
			if ( (pmiTarget->m_aAnimation.GetLength() == 0)
				&& (pmiSrc->m_aAnimation.GetLength() > 0) )
			{
				pmiTarget->m_aTargetIDs = pmiSrc->m_aTargetIDs ;
				pmiTarget->m_aAnimation = pmiSrc->m_aAnimation ;
			}
			else if ( (pmiTarget->m_aTargetIDs.GetLength() > 0)
						&& (pmiTarget->m_aAnimation.GetLength() > 0)
						&& (pmiSrc->m_aTargetIDs.GetLength() > 0)
						&& (pmiSrc->m_aAnimation.GetLength() > 0) )
			{
				SArray<float32_t>	aAnimeSrc1 = pmiTarget->m_aAnimation ;
				const float32_t *	pAnimSrc1 = aAnimeSrc1.GetConstArray() ;
				const float32_t *	pAnimSrc2 = pmiSrc->m_aAnimation.GetConstArray() ;
				size_t	pitchSrc1 = pmiTarget->m_aTargetIDs.GetLength() ;
				size_t	pitchSrc2 = pmiSrc->m_aTargetIDs.GetLength() ;
				size_t	nSrc1Length = aAnimeSrc1.GetLength() / pitchSrc1 ;
				size_t	nSrc2Length = pmiSrc->m_aAnimation.GetLength() / pitchSrc2 ;
				size_t	nDstLength =
							(size_t) esl_min( (int) nSrc1Length, (int) nSrc2Length ) ;
				for ( j = 0; j < pitchSrc2; j ++ )
				{
					SString *	pstrMorphID = pmiSrc->m_aTargetIDs.GetAt( j ) ;
					ESLAssert( pstrMorphID != NULL ) ;
					if ( pstrMorphID != NULL )
					{
						pmiTarget->m_aTargetIDs.Add( new SString(*pstrMorphID) ) ;
					}
					else
					{
						pmiTarget->m_aTargetIDs.Add( new SString() ) ;
					}
				}
				size_t		pitchDst = pmiTarget->m_aTargetIDs.GetLength() ;
				pmiTarget->m_aAnimation.SetLength( nDstLength * pitchDst ) ;
				//
				float32_t *	pAnimDst = pmiTarget->m_aAnimation.GetArray() ;
				for ( j = 0; j < nDstLength; j ++ )
				{
					eslCopyMemory
						( pAnimDst + j * pitchDst,
							pAnimSrc1 + j * pitchSrc1,
							pitchSrc1 * sizeof(float32_t) ) ;
					eslCopyMemory
						( pAnimDst + j * pitchDst + pitchSrc1,
							pAnimSrc2 + j * pitchSrc2,
							pitchSrc2 * sizeof(float32_t) ) ;
				}
				pmiTarget->m_aAnimation.FinishArray() ;
			}
		}
		RemoveMorphingAs( ppwszMergeSources[i] ) ;
	}
	//
	// 表示メッシュの統合
	//
	MeshSelector *	pmsTarget = GetMeshSelectorAs( pwszMergeTarget ) ;
	for ( i = 0; i < nSourceCount; i ++ )
	{
		MeshSelector *	pmsSrc = GetMeshSelectorAs( ppwszMergeSources[i] ) ;
		if ( pmsSrc == NULL )
		{
			continue ;
		}
		if ( pmsTarget == NULL )
		{
			pmsTarget = new MeshSelector( *pmsSrc ) ;
			AddMeshSelectorAs( pwszMergeTarget, pmsTarget ) ;
		}
		else
		{
			pmsTarget->m_fDisabled =
					pmsTarget->m_fDisabled && pmsSrc->m_fDisabled ;
			pmsTarget->m_flagVisible =
					pmsTarget->m_flagVisible || pmsSrc->m_flagVisible ;
			//
			size_t	nLength = pmsTarget->m_aVisibles.GetLength() ;
			if ( nLength < pmsSrc->m_aVisibles.GetLength() )
			{
				nLength = pmsSrc->m_aVisibles.GetLength() ;
				pmsTarget->m_aVisibles.SetLength( nLength ) ;
			}
			uint8_t *		pDstVis = pmsTarget->m_aVisibles.GetArray() ;
			const uint8_t *	pSrcVis = pmsSrc->m_aVisibles.GetConstArray() ;
			nLength = pmsSrc->m_aVisibles.GetLength() ;
			for ( j = 0; j < nLength; j ++ )
			{
				pDstVis[j] |= pSrcVis[j] ;
			}
			pmsTarget->m_aVisibles.FinishArray() ;
		}
		RemoveMeshSelectorAs( ppwszMergeSources[i] ) ;
	}
	//
	// 表示マテリアルの統合
	//
	MaterialSelector *	pmtsTarget = GetMaterialSelectorAs( pwszMergeTarget ) ;
	for ( i = 0; i < nSourceCount; i ++ )
	{
		MaterialSelector *	pmtsSrc = GetMaterialSelectorAs( ppwszMergeSources[i] ) ;
		if ( pmtsSrc == NULL )
		{
			continue ;
		}
		if ( pmtsTarget == NULL )
		{
			pmtsTarget = new MaterialSelector( *pmtsSrc ) ;
			AddMaterialSelectorAs( pwszMergeTarget, pmtsTarget ) ;
		}
		else
		{
			pmtsTarget->m_fDisabled =
					pmtsTarget->m_fDisabled && pmtsSrc->m_fDisabled ;
		}
		RemoveMeshSelectorAs( ppwszMergeSources[i] ) ;
	}
}




//////////////////////////////////////////////////////////////////////////////
// ポーズライブラリ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DModelPoseLibrary, SObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DModelPoseLibrary::S3DModelPoseLibrary( void )
{
}

S3DModelPoseLibrary::S3DModelPoseLibrary( const S3DModelPoseLibrary& lib )
	: m_ssoaPoses( lib.m_ssoaPoses ), m_aRefLib( lib.m_aRefLib )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DModelPoseLibrary::~S3DModelPoseLibrary( void )
{
}

// 代入
//////////////////////////////////////////////////////////////////////////////
const S3DModelPoseLibrary&
		S3DModelPoseLibrary::operator = ( const S3DModelPoseLibrary& lib )
{
	m_ssoaPoses = lib.m_ssoaPoses ;
	m_refParent = lib.m_refParent ;
	m_aRefLib = lib.m_aRefLib ;
	return	*this ;
}

// ポーズファイル読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelPoseLibrary::LoadLibrary( const wchar_t * pwszFilePath )
{
	SSmartPointer<SFileInterface>	pFile =
			SFileOpener::DefaultNewOpenFile
				( pwszFilePath, SFileOpener::shareRead ) ;
	if ( pFile == NULL )
	{
		return	sglErrFailed ;
	}
	return	ReadLibrary( *pFile ) ;
}

SGLError S3DModelPoseLibrary::ReadLibrary( SSystem::SFileInterface & file )
{
	SChunkFile	cf ;
	if ( cf.OpenChunkFile( &file ) )
	{
		return	sglErrFailed ;
	}
	if ( cf.GetFileHeader().dwFileID != SChunkFile::fidEGL3DPoseLibrary )
	{
		return	sglErrFailed ;
	}
	return	ReadLibraryChunk( cf ) ;
}

SGLError S3DModelPoseLibrary::ReadLibraryChunk( SSystem::SChunkFile & cf )
{
	//
	// 定義名配列
	//
	SObjectArray<SString>	aStrings ;
	if ( cf.DescendChunk( "strings " ) )
	{
		return	sglErrFailed ;
	}
	S3DStdModelLoader::ReadStrings( aStrings, cf ) ;
	cf.AscendChunk() ;
	//
	// ポーズ配列
	//
	size_t	iPose = 0 ;
	m_ssoaPoses.RemoveAll() ;
	for ( ; ; )
	{
		if ( cf.DescendChunk() )
		{
			break ;
		}
		if ( cf.IsEqualCurrentChunkID( "pose    " ) )
		{
			SString *	pstrID = aStrings.GetAt( iPose ++ ) ;
			if ( pstrID != NULL )
			{
				S3DModelPose *	pPose = new S3DModelPose ;
				if ( pPose->ReadPose( cf ) )
				{
					delete	pPose ;
				}
				else
				{
					m_ssoaPoses.Add( *pstrID, pPose ) ;
				}
			}
		}
		cf.AscendChunk() ;
	}
	return	sglErrSuccess ;
}

SGLError S3DModelPoseLibrary::LoadLibraryXML( const wchar_t * pwszFilePath )
{
	SXMLDocument	xmlDoc ;
	if ( xmlDoc.LoadDocument( pwszFilePath, xmlDoc ) )
	{
		return	sglErrFailed ;
	}
	SXMLDocument *	pxmlLib = xmlDoc.GetElementTagAs( L"library" ) ;
	if ( pxmlLib == NULL )
	{
		return	sglErrFailed ;
	}
	return	ParseLibraryXML( *pxmlLib ) ;
}

SGLError S3DModelPoseLibrary::ParseLibraryXML( const SSystem::SXMLDocument & xmlLibTag )
{
	m_ssoaPoses.RemoveAll() ;
	for ( size_t i = 0; i < xmlLibTag.GetElementsCount(); i ++ )
	{
		SXMLDocument *	pxmlTag = xmlLibTag.GetElementAt( i ) ;
		if ( pxmlTag == NULL )
		{
			continue ;
		}
		if ( pxmlTag->GetTag() == L"pose" )
		{
			SString *	pstrID = pxmlTag->GetAttributeAs( L"id" ) ;
			if ( pstrID != NULL )
			{
				S3DModelPose *	pPose = new S3DModelPose ;
				if ( pPose->ParsePoseXML( *pxmlTag ) )
				{
					delete	pPose ;
				}
				else
				{
					m_ssoaPoses.Add( *pstrID, pPose ) ;
				}
			}
		}
	}
	return	sglErrSuccess ;
}

// ポーズファイル書き出し
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelPoseLibrary::SaveLibrary( const wchar_t * pwszFilePath )
{
	SSmartPointer<SFileInterface>	pFile =
			SFileOpener::DefaultNewOpenFile
				( pwszFilePath, SFileOpener::modeCreate ) ;
	if ( pFile == NULL )
	{
		return	sglErrFailed ;
	}
	return	WriteLibrary( *pFile ) ;
}

SGLError S3DModelPoseLibrary::WriteLibrary( SSystem::SFileInterface & file )
{
	SChunkFile::FILE_HEADER	fhdr ;
	fhdr.SetHeaderInfo
		( SChunkFile::fidEGL3DPoseLibrary, "EntisGLS4 pose library file" ) ;
	//
	SChunkFile	cf ;
	if ( cf.OpenChunkFile
		( &file, false, SFileOpener::modeCreate, &fhdr ) )
	{
		return	sglErrFailed ;
	}
	SGLError	err = WriteLibraryChunk( cf ) ;
	cf.Close() ;
	return	err ;
}

SGLError S3DModelPoseLibrary::WriteLibraryChunk( SSystem::SChunkFile & cf )
{
	//
	// 定義名配列
	//
	SObjectArray<SString>	aStrings ;
	size_t	i ;
	size_t	nPoses = m_ssoaPoses.GetLength() ;
	for ( i = 0; i < nPoses; i ++ )
	{
		const SString *	pstrID = m_ssoaPoses.GetTagAt( i ) ;
		ESLAssert( pstrID != NULL ) ;
		if ( pstrID != NULL )
		{
			aStrings.Add( new SString( *pstrID ) ) ;
		}
	}
	if ( cf.DescendChunk( "strings " ) )
	{
		return	sglErrFailed ;
	}
	S3DStdModelSaver::WriteStrings( cf, aStrings ) ;
	cf.AscendChunk() ;
	//
	// ポーズ配列
	//
	for ( i = 0; i < nPoses; i ++ )
	{
		S3DModelPose *	pPose = m_ssoaPoses.GetAt( i ) ;
		ESLAssert( pPose != NULL ) ;
		if ( pPose != NULL )
		{
			if ( cf.DescendChunk( "pose    " ) )
			{
				return	sglErrFailed ;
			}
			pPose->WritePose( cf ) ;
			cf.AscendChunk() ;
		}
	}
	return	sglErrSuccess ;
}

SGLError S3DModelPoseLibrary::SaveLibraryXML( const wchar_t * pwszFilePath )
{
	SXMLDocument	xmlLib ;
	xmlLib.SetTag( L"library" ) ;
	FormatLibraryXML( xmlLib ) ;
	if ( xmlLib.SaveDocument( pwszFilePath ) )
	{
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

SGLError S3DModelPoseLibrary::FormatLibraryXML( SSystem::SXMLDocument & xmlLibTag )
{
	size_t	nPoses = m_ssoaPoses.GetLength() ;
	for ( size_t i = 0; i < nPoses; i ++ )
	{
		S3DModelPose *	pPose = m_ssoaPoses.GetAt( i ) ;
		const SString *	pstrID = m_ssoaPoses.GetTagAt( i ) ;
		ESLAssert( pPose != NULL ) ;
		ESLAssert( pstrID != NULL ) ;
		if ( (pPose != NULL) && (pstrID != NULL) )
		{
			SXMLDocument *	pxmlPose = new SXMLDocument ;
			pxmlPose->SetTag( L"pose" ) ;
			pxmlPose->SetAttributeAs( L"id", *pstrID ) ;
			xmlLibTag.AddElement( pxmlPose ) ;
			//
			pPose->FormatPoseXML( *pxmlPose ) ;
		}
	}
	return	sglErrSuccess ;
}

// 解放
//////////////////////////////////////////////////////////////////////////////
void S3DModelPoseLibrary::Release( void )
{
	m_ssoaPoses.RemoveAll() ;
	m_refParent.SetReference( NULL ) ;
	m_aRefLib.RemoveAll() ;
}

void S3DModelPoseLibrary::ReleaseAllPoses( void )
{
	m_ssoaPoses.RemoveAll() ;
}

// ポーズ取得
//////////////////////////////////////////////////////////////////////////////
S3DModelPose * S3DModelPoseLibrary::GetPoseAs
				( const wchar_t * pwszID, bool fOnlyLocal ) const
{
	S3DModelPose *	pPose = m_ssoaPoses.GetAs( pwszID ) ;
	if ( (pPose == NULL) && !fOnlyLocal )
	{
		S3DModelPoseLibrary *	pParent = m_refParent ;
		if ( pParent != NULL )
		{
			pPose = pParent->GetPoseAs( pwszID, false ) ;
		}
		if ( (pPose == NULL) && (pwszID != nullptr) )
		{
			SString	strLibName ;
			size_t	iPoseID = 0 ;
			for ( size_t i = 0; pwszID[i] != 0; i ++ )
			{
				if ( pwszID[i] == L'#' )
				{
					strLibName = SString( pwszID, (ssize_t) i ) ;
					iPoseID = i + 1 ;
					break ;
				}
			}
			for ( size_t i = 0; i < m_aRefLib.GetLength(); i ++ )
			{
				S3DModelPoseLibrary *	pRef = m_aRefLib.GetAt( i ) ;
				if ( (pRef != NULL)
					&& (strLibName.IsEmpty()
						|| (pRef->GetLibraryName() == strLibName)) )
				{
					pPose = pRef->GetPoseAs( pwszID + iPoseID, false ) ;
					if ( pPose != NULL )
					{
						break ;
					}
				}
			}
		}
	}
	return	pPose ;
}

S3DModelPose * S3DModelPoseLibrary::GetPoseAt( size_t i ) const
{
	return	m_ssoaPoses.GetAt( i ) ;
}

// ポーズ検索
//////////////////////////////////////////////////////////////////////////////
ssize_t S3DModelPoseLibrary::FindPosePtr( S3DModelPose * pPose ) const
{
	return	m_ssoaPoses.FindPtr( pPose ) ;
}

// ポーズ数取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DModelPoseLibrary::GetPoseCount( void ) const
{
	return	m_ssoaPoses.GetLength() ;
}

// 登録名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DModelPoseLibrary::GetPoseIdentityAt( size_t i ) const
{
	const SString *	pstrID = m_ssoaPoses.GetTagAt( i ) ;
	if ( pstrID != NULL )
	{
		return	*pstrID ;
	}
	return	NULL ;
}

const wchar_t * S3DModelPoseLibrary::GetPoseIdentityOf
					( S3DModelPose * pPose, bool fOnlyLocal ) const
{
	ssize_t	i = m_ssoaPoses.FindPtr( pPose ) ;
	if ( i >= 0 )
	{
		return	GetPoseIdentityAt( (size_t) i ) ;
	}
	if ( !fOnlyLocal )
	{
		S3DModelPoseLibrary *	pParent = m_refParent ;
		if ( pParent != NULL )
		{
			const wchar_t *	pwszID =
					pParent->GetPoseIdentityOf( pPose, false ) ;
			if ( pwszID != NULL )
			{
				return	pwszID ;
			}
		}
		for ( size_t i = 0; i < m_aRefLib.GetLength(); i ++ )
		{
			S3DModelPoseLibrary *	pRef = m_aRefLib.GetAt( i ) ;
			if ( pRef != NULL )
			{
				const wchar_t *	pwszID =
						pRef->GetPoseIdentityOf( pPose, false ) ;
				if ( pwszID != NULL )
				{
					return	pwszID ;
				}
			}
		}
	}
	return	NULL ;
}

// ポーズ追加
//////////////////////////////////////////////////////////////////////////////
void S3DModelPoseLibrary::AddPoseAs( const wchar_t * pwszID, S3DModelPose * pPose )
{
	m_ssoaPoses.Add( pwszID, pPose ) ;
}

// ポーズ名変更
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelPoseLibrary::RenamePoseAs
	( S3DModelPose * pPose, const wchar_t * pwszID )
{
	ssize_t	i = m_ssoaPoses.FindPtr( pPose ) ;
	if ( i < 0 )
	{
		return	sglErrFailed ;
	}
	m_ssoaPoses.DetachAt( (size_t) i ) ;
	m_ssoaPoses.Add( pwszID, pPose ) ;
	return	sglErrSuccess ;
}

// ポーズ削除
//////////////////////////////////////////////////////////////////////////////
void S3DModelPoseLibrary::RemovePoseAs( const wchar_t * pwszID )
{
	m_ssoaPoses.RemoveAs( pwszID ) ;
}

void S3DModelPoseLibrary::RemovePoseAt( size_t i )
{
	m_ssoaPoses.RemoveAt( i ) ;
}

// 参照されていないポーズを削除
//////////////////////////////////////////////////////////////////////////////
void S3DModelPoseLibrary::CleanupPose( void )
{
	size_t	nPoses = m_ssoaPoses.GetLength() ;
	for ( size_t i = 0; i < nPoses; i ++ )
	{
		S3DModelPose *	pPose = m_ssoaPoses.GetAt( i ) ;
		if ( (pPose != NULL) && (pPose->GetReferenceCount() == 0) )
		{
			m_ssoaPoses.SetAt( i, NULL ) ;
		}
	}
	m_ssoaPoses.TrimEmpty() ;
}

// 全ポーズ削除
//////////////////////////////////////////////////////////////////////////////
void S3DModelPoseLibrary::RemoveAllPoses( void )
{
	m_ssoaPoses.RemoveAll() ;
}

// ライブラリ名
//////////////////////////////////////////////////////////////////////////////
const SSystem::SString& S3DModelPoseLibrary::GetLibraryName( void ) const
{
	return	m_strName ;
}

void S3DModelPoseLibrary::SetLibraryName( const wchar_t * pwszName )
{
	m_strName = pwszName ;
}

// 参照ライブラリを設定する
//////////////////////////////////////////////////////////////////////////////
void S3DModelPoseLibrary::SetParentLibrary( S3DModelPoseLibrary * pLib )
{
	m_refParent = pLib ;
}

size_t S3DModelPoseLibrary::AddReferenceLibrary( S3DModelPoseLibrary * pLib )
{
	ssize_t	i = m_aRefLib.FindPtr( pLib ) ;
	if ( i >= 0 )
	{
		return	(size_t) i ;
	}
	return	m_aRefLib.Add( pLib ) ;
}

// 参照ライブラリを取得する
//////////////////////////////////////////////////////////////////////////////
S3DModelPoseLibrary * S3DModelPoseLibrary::GetParentLibrary( void ) const
{
	return	m_refParent ;
}

size_t S3DModelPoseLibrary::GetReferenceLibraryCount( void ) const
{
	return	m_aRefLib.GetLength() ;
}

S3DModelPoseLibrary * S3DModelPoseLibrary::GetReferenceLibraryAt( size_t i ) const
{
	return	m_aRefLib.GetAt( i ) ;
}

void S3DModelPoseLibrary::DetachReferenceLibraryAt( size_t i )
{
	m_aRefLib.DetachAt( i ) ;
}

void S3DModelPoseLibrary::DetachReferenceLibraryOf( S3DModelPoseLibrary * pLib )
{
	m_aRefLib.DetachAt( (size_t) m_aRefLib.FindPtr( pLib ) ) ;
}

void S3DModelPoseLibrary::DetachAllReferenceLibrarys( void )
{
	m_aRefLib.DetachAll() ;
}

// ポーズターゲットリセット
//////////////////////////////////////////////////////////////////////////////
void S3DModelPoseLibrary::ResetAllPoseTarget( bool flagResetRef )
{
	size_t	nPoses = m_ssoaPoses.GetLength() ;
	for ( size_t i = 0; i < nPoses; i ++ )
	{
		S3DModelPose *	pPose = m_ssoaPoses.GetAt( i ) ;
		if ( pPose != NULL )
		{
			pPose->ResetPoseTarget() ;
		}
	}
	if ( flagResetRef )
	{
		S3DModelPoseLibrary *	pParent = m_refParent ;
		if ( pParent != NULL )
		{
			pParent->ResetAllPoseTarget() ;
		}
		for ( size_t i = 0; i < m_aRefLib.GetLength(); i ++ )
		{
			S3DModelPoseLibrary *	pRef = m_aRefLib.GetAt( i ) ;
			if ( pRef != NULL )
			{
				pRef->ResetAllPoseTarget() ;
			}
		}
	}
}

// 全てのポーズに含まれる参照先メッシュIDの変更
//////////////////////////////////////////////////////////////////////////////
void S3DModelPoseLibrary::ChangeTargetMeshID
	( const wchar_t * pwszOldID, const wchar_t * pwszNewID )
{
	size_t	nPoses = m_ssoaPoses.GetLength() ;
	for ( size_t i = 0; i < nPoses; i ++ )
	{
		S3DModelPose *	pPose = m_ssoaPoses.GetAt( i ) ;
		if ( pPose != NULL )
		{
			pPose->ChangeTargetMeshID( pwszOldID, pwszNewID ) ;
		}
	}
}

// 参照先メッシュの統合
//////////////////////////////////////////////////////////////////////////////
void S3DModelPoseLibrary::MergeTargetMeshID
	( const wchar_t * pwszMergeTarget,
		const wchar_t *const* ppwszMergeSources, size_t nSourceCount )
{
	size_t	nPoses = m_ssoaPoses.GetLength() ;
	for ( size_t i = 0; i < nPoses; i ++ )
	{
		S3DModelPose *	pPose = m_ssoaPoses.GetAt( i ) ;
		if ( pPose != NULL )
		{
			pPose->MergeTargetMeshID
				( pwszMergeTarget, ppwszMergeSources, nSourceCount ) ;
		}
	}
}



//////////////////////////////////////////////////////////////////////////////
// 動的モデル・抽象ポーズアニメーション
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DDynamicModelItem::PoseAnimator, SObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DDynamicModelItem::PoseAnimator::PoseAnimator( void )
{
	m_priority = 0 ;
	m_flags = 0 ;
	m_blend = 1.0 ;
}

S3DDynamicModelItem::PoseAnimator::PoseAnimator
		( const S3DDynamicModelItem::PoseAnimator& pose )
{
	m_priority = pose.m_priority ;
	m_flags = pose.m_flags ;
	m_blend = pose.m_blend ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DDynamicModelItem::PoseAnimator::~PoseAnimator( void )
{
}

// 時間経過 (終了時に true 返却)
//////////////////////////////////////////////////////////////////////////////
bool S3DDynamicModelItem::PoseAnimator::OnTimer
	( S3DDynamicModelItem& item, uint32_t msecPast )
{
	return	true ;
}

// ポーズ設定
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicModelItem::PoseAnimator::ApplyPose( S3DModelBuffer& model )
{
}

// 即時完了
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicModelItem::PoseAnimator::FlushAnimation( S3DDynamicModelItem& item )
{
}

// キャンセル（アニメーションの停止）
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicModelItem::PoseAnimator::CancelAnimation( S3DDynamicModelItem& item )
{
}

// 揮発ポーズか？
//////////////////////////////////////////////////////////////////////////////
bool S3DDynamicModelItem::PoseAnimator::IsVolatilePose( void ) const
{
	return	(m_flags & (flagVolatile | flagTransition)) != 0 ;
}

// 複製
//////////////////////////////////////////////////////////////////////////////
S3DDynamicModelItem::PoseAnimator *
		S3DDynamicModelItem::PoseAnimator::Duplicate( void )
{
	return	new PoseAnimator( *this ) ;
}

// 優先度
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicModelItem::PoseAnimator::SetPriority( int32_t priority )
{
	m_priority = priority ;
}

// フラグ
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicModelItem::PoseAnimator::SetFlags( uint32_t flags )
{
	m_flags = flags ;
}

// ブレンド率
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicModelItem::PoseAnimator::SetBlendWeight( double blend )
{
	m_blend = blend ;
}



//////////////////////////////////////////////////////////////////////////////
// S3DScene 動的モデルデータアイテム
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DDynamicModelItem, ModelItem )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DDynamicModelItem::S3DDynamicModelItem( void )
{
	m_classItem = S3DScene::classDynamicItem1 ;
	m_flagsBehavior |= S3DScene::itemOwnerBehavior ;
	m_physExogenous.matSpace.InitializeMatrix( S3DDVector( 1, 1, 1 ) ) ;
	m_physExogenous.matLastSpace.InitializeMatrix( S3DDVector( 1, 1, 1 ) ) ;
	m_msecPastPhys = 0 ;
	m_flagTimerAnime = true ;
	m_flagTimerPhysics = true ;
	m_flagDelayResetPhys = false ;
	m_nDelayDrivePhysFrame = 0 ;
	m_secDelayDriveFrame = 0.033 ;
	m_physEffect.flagsEffects = physSwayStream ;
	m_physEffect.fpMoveAccel = 1.0 ;
	m_physEffect.fpMoveStream = 0.25 ;
	m_physEffect.fpSwayAmplitude[0] = 1.0 ;
	m_physEffect.fpSwayAmplitude[1] = 1.5 ;
	m_physEffect.fpSwayWaveLength[0] = 5.0 ;
	m_physEffect.fpSwayWaveLength[1] = 3.0 ;
	m_fpMoveLength = 0.0 ;
	m_pAnimListener = NULL ;
	m_maskMarkerCollider = 0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DDynamicModelItem::~S3DDynamicModelItem( void )
{
	DetachAll() ;
}

// モデルデータ関連付け
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicModelItem::AttachModel( S3DVertexBufferInterface * pModel )
{
	ModelItem::AttachModel( pModel ) ;
	//
	S3DModelBuffer *	pModelBuf = ESLTypeCast<S3DModelBuffer>( pModel ) ;
	if ( pModelBuf != NULL )
	{
		m_flagsBehavior |= S3DScene::itemTimer | S3DScene::itemOwnerBehavior ;
		pModelBuf->AttachRelationItem( this ) ;
	}
}

void S3DDynamicModelItem::AttachCollisionModel( S3DVertexBufferInterface * pColModel )
{
	ModelItem::AttachCollisionModel( pColModel ) ;
}

// 当たり判定モデル構築
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicModelItem::BuildCollisionMesh( void )
{
	ModelItem::BuildCollisionMesh() ;
	//
	if ( m_maskMarkerCollider != 0 )
	{
		S3DModelBuffer *	pModel = ESLTypeCast<S3DModelBuffer>( m_pModel ) ;
		if ( pModel != NULL )
		{
			m_collisionMesh.AttachMeshUserData( (Item*) this ) ;
			m_collisionMesh.SetSceneClassesMask
								( (1 << m_classItem) | m_maskClasses ) ;
			m_collisionMesh.SetUserClassesMask( m_maskColliderFlags ) ;
			//
			uint32_t	mask = m_maskMarkerCollider ;
			int			iType = 0 ;
			while ( (mask != 0)
				&& (iType < S3DModelData::MarkerInfo::typeCount) )
			{
				if ( mask & 0x01 )
				{
					pModel->AddAllMarkerForCollision
						( m_collisionMesh, (S3DModelData::MarkerInfo::Type) iType ) ;
					m_flagCollision = true ;
				}
				mask >>= 1 ;
				iType ++ ;
			}
		}
	}
}

// モデルデータのマーカーをコライダに設定する
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicModelItem::SetModelMarkerForCollider( uint32_t maskTypes )
{
	if ( m_maskMarkerCollider != maskTypes )
	{
		m_maskMarkerCollider = maskTypes ;
		BuildCollisionMesh() ;
	}
}

uint32_t S3DDynamicModelItem::GetModelMarkerForCollider( void ) const
{
	return	m_maskMarkerCollider ;
}

// モデルとポーズの関連付けを解除する
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicModelItem::DetachAll( void )
{
	if ( m_arrPoseTracks.GetLength() > 0 )
	{
		RemoveAllPoseAnimators() ;
	}
	ModelItem::AttachModel( NULL ) ;
	ModelItem::AttachCollisionModel( NULL ) ;
}

// ポーズアニメーション追加
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicModelItem::AddPoseAnimator
	( S3DDynamicModelItem::PoseAnimator * pPose )
{
	Lock() ;
	ESLAssert( m_arrPoseTracks.FindPtr( pPose ) < 0 ) ;
	size_t	iPose = OrderPoseAnimatorPriorityOf( pPose->GetPriority() ) ;
	m_arrPoseTracks.InsertAt( iPose, pPose ) ;
	NotifyOnAddAnimation( pPose ) ;
	Unlock() ;
}

// ポーズトラック削除
//////////////////////////////////////////////////////////////////////////////
bool S3DDynamicModelItem::RemovePoseAnimator
			( S3DDynamicModelItem::PoseAnimator * pPose )
{
	ssize_t	iPose ;
	Lock() ;
	iPose = m_arrPoseTracks.FindPtr( pPose ) ;
	if ( iPose >= 0 )
	{
		NotifyOnFinishAnimation( pPose, reasonRemove ) ;
		m_arrPoseTracks.RemoveAt( (size_t) iPose ) ;
	}
	Unlock() ;
	return	(iPose >= 0) ;
}

void S3DDynamicModelItem::RemoveAllPoseAnimators( void )
{
	Lock() ;
	if ( m_pAnimListener != NULL )
	{
		for ( size_t i = 0; i < m_arrPoseTracks.GetLength(); i ++ )
		{
			PoseAnimator *	ppa = m_arrPoseTracks.GetAt( i ) ;
			ESLAssert( ppa != NULL ) ;
			if ( ppa != NULL )
			{
				NotifyOnFinishAnimation( ppa, reasonRemove ) ;
			}
		}
	}
	m_arrPoseTracks.RemoveAll() ;
	Unlock() ;
}

// ポーズトラック分離
//////////////////////////////////////////////////////////////////////////////
S3DDynamicModelItem::PoseAnimator *
	S3DDynamicModelItem::DetachPoseAnimator
		( S3DDynamicModelItem::PoseAnimator * pPose )
{
	ssize_t	iPose ;
	Lock() ;
	iPose = m_arrPoseTracks.FindPtr( pPose ) ;
	if ( iPose >= 0 )
	{
		NotifyOnFinishAnimation( pPose, reasonDetach ) ;
		iPose = m_arrPoseTracks.FindPtr( pPose ) ;
		if ( iPose >= 0 )
		{
			pPose = m_arrPoseTracks.DetachAt( (size_t) iPose ) ;
		}
	}
	else
	{
		pPose = NULL ;
	}
	Unlock() ;
	return	pPose ;
}

// ポーズトラック取得
//////////////////////////////////////////////////////////////////////////////
S3DDynamicModelItem::PoseAnimator *
	S3DDynamicModelItem::GetPoseAnimatorPriorityOf( int32_t nPriority ) const
{
	PoseAnimator *	pPose = NULL ;
	Lock() ;
	const size_t	countPoseTrack = m_arrPoseTracks.GetLength() ;
	for ( size_t i = 0; i < countPoseTrack; i ++ )
	{
		PoseAnimator *	ppa = m_arrPoseTracks.GetAt( i ) ;
		if ( (ppa != NULL) && (ppa->GetPriority() == nPriority) )
		{
			pPose = ppa ;
			break ;
		}
	}
	Unlock() ;
	return	pPose ;
}

// ポーズ優先度挿入指標
//////////////////////////////////////////////////////////////////////////////
size_t S3DDynamicModelItem::OrderPoseAnimatorPriorityOf( int32_t nPriority ) const
{
	Lock() ;
	const size_t	countPoseTrack = m_arrPoseTracks.GetLength() ;
	for ( size_t i = 0; i < countPoseTrack; i ++ )
	{
		PoseAnimator *	ppa = m_arrPoseTracks.GetAt( i ) ;
		if ( (ppa != NULL) && (nPriority <= ppa->GetPriority()) )
		{
			Unlock() ;
			return	i ;
		}
	}
	Unlock() ;
	return	countPoseTrack ;
}

// ポーズアニメーションの即時強制完了と物理演算初期化
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicModelItem::FlushPoseAnimation( void )
{
	Lock() ;
	const size_t	countPoseTrack = m_arrPoseTracks.GetLength() ;
	for ( size_t i = 0; i < countPoseTrack; i ++ )
	{
		PoseAnimator *	pPoseAni = m_arrPoseTracks.GetAt( i ) ;
		if ( pPoseAni != NULL )
		{
			pPoseAni->FlushAnimation( *this ) ;
		}
	}
	//
	S3DModelBuffer *	pModel = ESLTypeCast<S3DModelBuffer>( m_pModel ) ;
	if ( pModel != NULL )
	{
		UpdateModelPose( *pModel ) ;
		ResetPhysicsParameter() ;
	}
	Unlock() ;
}

// ポーズアニメーションのキャンセル処理（アニメーションの停止）
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicModelItem::CancelPoseAnimation( void )
{
	Lock() ;
	const size_t	countPoseTrack = m_arrPoseTracks.GetLength() ;
	for ( size_t i = 0; i < countPoseTrack; i ++ )
	{
		PoseAnimator *	pPoseAni = m_arrPoseTracks.GetAt( i ) ;
		if ( pPoseAni != NULL )
		{
			pPoseAni->CancelAnimation( *this ) ;
		}
	}
	Unlock() ;
}

// すべての揮発性アニメーションを削除
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicModelItem::CleanupVolatilePoses( void )
{
	Lock() ;
	const size_t	countPoseTrack = m_arrPoseTracks.GetLength() ;
	for ( size_t i = 0; i < countPoseTrack; i ++ )
	{
		PoseAnimator *	pPoseAni = m_arrPoseTracks.GetAt( i ) ;
		if ( (pPoseAni != NULL) && pPoseAni->IsVolatilePose() )
		{
			NotifyOnFinishAnimation( pPoseAni, reasonCleanup ) ;
			m_arrPoseTracks.SetAt( i, NULL ) ;
		}
	}
	m_arrPoseTracks.TrimEmpty() ;
	Unlock() ;
}

// 不揮発アニメーションの有無
//////////////////////////////////////////////////////////////////////////////
bool S3DDynamicModelItem::AreAnyVolatilePoses( void ) const
{
	bool	fVolatile = false ;
	Lock() ;
	const size_t	countPoseTrack = m_arrPoseTracks.GetLength() ;
	for ( size_t i = 0; i < countPoseTrack; i ++ )
	{
		PoseAnimator *	pPoseAni = m_arrPoseTracks.GetAt( i ) ;
		if ( (pPoseAni != NULL) && pPoseAni->IsVolatilePose() )
		{
			fVolatile = true ;
			break ;
		}
	}
	Unlock() ;
	return	fVolatile ;
}

// 物理演算内部運動量リセット
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicModelItem::ResetPhysicsParameter( void )
{
	S3DModelBuffer *	pModel = ESLTypeCast<S3DModelBuffer>( m_pModel ) ;
	if ( pModel != NULL )
	{
		Lock() ;
		//
		CalcGlobalTransformation
			( m_physExogenous.matSpace, m_physExogenous.vSpace ) ;
//		if ( m_physEffect.flagsEffects & physNoEffectRotation )
//		{
//			m_physExogenous.matSpace.InitializeMatrix( S3DDVector( 1, 1, 1 ) ) ;
//		}
//		m_physExogenous.vSpace *= m_physEffect.fpMoveAccel ;
		m_physExogenous.matLastSpace = m_physExogenous.matSpace ;
		m_physExogenous.vLastSpace = m_physExogenous.vSpace ;
		//
		pModel->GetBoneRoot().ResetPhysicsParameter() ;
		//
		Unlock() ;
	}
}

// 物理演算を指定フレーム数行う
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicModelItem::DrivePhysicsFrames( size_t nFrames, double secFrame )
{
	S3DModelBuffer *	pModel = ESLTypeCast<S3DModelBuffer>( m_pModel ) ;
	if ( pModel != NULL )
	{
		Lock() ;
		for ( size_t i = 0; i < nFrames; i ++ )
		{
			m_msecPastPhys =
				(uint32_t) eslRoundR64ToLInt
								( esl_fmax( secFrame, 0.001 ) * 1000.0 ) ;
			OnPhysicsBoneAnimation( *pModel ) ;
		}
		Unlock() ;
	}
}

// 遅延物理演算リセット処理
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicModelItem::DelayResetPhysicsAndDriveFrames( size_t nFrames, double secFrame )
{
	Lock() ;
	m_msecPastPhys = 0 ;
	m_flagDelayResetPhys = true ;
	m_nDelayDrivePhysFrame = nFrames ;
	m_secDelayDriveFrame = secFrame ;
	Unlock() ;
}

// OnTimer でポーズアニメーション処理（デフォルト＝有効）
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicModelItem::EnablePoseAnimationOnTimer( bool fTimerPoseAni )
{
	m_flagTimerAnime = fTimerPoseAni ;
}

bool S3DDynamicModelItem::IsEnabledPoseAnimationOnTimer( void ) const
{
	return	m_flagTimerAnime ;
}

// ポーズアニメーション進行
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicModelItem::AdvanceAnimationTime
		( S3DScene& scene, uint32_t msecPoseTime, uint32_t msecPhysTime )
{
	Lock() ;
	const size_t	countPoseTrack = m_arrPoseTracks.GetLength() ;
	if ( countPoseTrack > 0 )
	{
		for ( size_t i = 0; i < countPoseTrack; i ++ )
		{
			PoseAnimator *	pPoseAni = m_arrPoseTracks.GetAt( i ) ;
			if ( pPoseAni != NULL )
			{
				if ( pPoseAni->OnTimer( *this, msecPoseTime ) )
				{
					NotifyOnFinishAnimation( pPoseAni, reasonCompleted ) ;
					ssize_t	j = m_arrPoseTracks.FindPtr( pPoseAni ) ;
					if ( j >= 0 )
					{
						m_arrPoseTracks.SetAt( (size_t) j, NULL ) ;
						if ( i > (size_t) j )
						{
							i -- ;
						}
					}
				}
			}
		}
		m_arrPoseTracks.TrimEmpty() ;
		//
		NotifyOnAnimation() ;
		scene.PostSceneUpdate() ;
	}
	else
	{
		S3DModelBuffer *	pModel = ESLTypeCast<S3DModelBuffer>( m_pModel ) ;
		if ( (pModel != NULL) && pModel->AreAnyPhysicsBones() )
		{
			NotifyOnAnimation() ;
			scene.PostSceneUpdate() ;
		}
	}
	Unlock() ;
}

// OnTimer で物理演算処理（デフォルト＝有効）
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicModelItem::EnablePhysicsTimeOnTimer( bool fTimerPhysics )
{
	m_flagTimerPhysics = fTimerPhysics ;
}

bool S3DDynamicModelItem::IsEnabledPhysicsTimeOnTimer( void ) const
{
	return	m_flagTimerPhysics ;
}

// 物理演算進行
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicModelItem::AddPhysicsTime( uint32_t msecPhysTime )
{
	m_msecPastPhys += msecPhysTime ;
}

// アニメーションリスナ設定
//////////////////////////////////////////////////////////////////////////////
bool S3DDynamicModelItem::AttachAnimationListener
		( S3DDynamicModelItem::AnimationListener * pListener )
{
	Lock() ;
	m_pAnimListener = pListener ;
	Unlock() ;
	return	true ;
}

// アニメーションリスナ解除
//////////////////////////////////////////////////////////////////////////////
bool S3DDynamicModelItem::DetachAnimationListener
			( S3DDynamicModelItem::AnimationListener * pListener )
{
	bool	fSuccess = false ;
	Lock() ;
	if ( m_pAnimListener == pListener )
	{
		m_pAnimListener = NULL ;
		fSuccess = true ;
	}
	Unlock() ;
	return	fSuccess ;
}

// アニメーション通知
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicModelItem::NotifyOnAnimation( void )
{
	ESLAssert( TestLocked() ) ;
	if ( m_pAnimListener != NULL )
	{
		m_pAnimListener->OnAnimation( this ) ;
	}
}

// アニメーション追加通知
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicModelItem::NotifyOnAddAnimation
		( S3DDynamicModelItem::PoseAnimator * pAnim )
{
	ESLAssert( TestLocked() ) ;
	if ( m_pAnimListener != NULL )
	{
		m_pAnimListener->OnAddAnimation( this, pAnim ) ;
	}
}

// アニメーション完了通知
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicModelItem::NotifyOnFinishAnimation
	( S3DDynamicModelItem::PoseAnimator * pAnim,
		S3DDynamicModelItem::FinishAnimationReason reason )
{
	ESLAssert( TestLocked() ) ;
	if ( m_pAnimListener != NULL )
	{
		m_pAnimListener->OnFinishAnimation( this, pAnim, reason ) ;
	}
}

// 物理演算影響フラグ
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DDynamicModelItem::GetPhysEffectFlags( void ) const
{
	return	m_physEffect.flagsEffects ;
}

void S3DDynamicModelItem::SetPhysEffectFlags( uint32_t nFlags )
{
	m_physEffect.flagsEffects = nFlags ;
}

// 平行移動加速度影響度
//////////////////////////////////////////////////////////////////////////////
double S3DDynamicModelItem::GetMoveAccelEffect( void ) const
{
	return	m_physEffect.fpMoveAccel ;
}

void S3DDynamicModelItem::SetMoveAccelEffect( double fpEffect )
{
	m_physEffect.fpMoveAccel = fpEffect ;
}

// 物理演算影響パラメータ
//////////////////////////////////////////////////////////////////////////////
const S3DDynamicModelItem::PhysEffectParam&
	S3DDynamicModelItem::GetPhysEffectParam( void ) const
{
	return	m_physEffect ;
}

void S3DDynamicModelItem::SetPhysEffectParam
	( const S3DDynamicModelItem::PhysEffectParam& param )
{
	m_physEffect = param ;
}

// 平行移動流速影響度
//////////////////////////////////////////////////////////////////////////////
double S3DDynamicModelItem::GetMoveStreamEffect( void ) const
{
	return	m_physEffect.fpMoveStream ;
}

void S3DDynamicModelItem::SetMoveStreamEffect( double fpEffect )
{
	m_physEffect.fpMoveStream = fpEffect ;
}

// 重力加速度
//////////////////////////////////////////////////////////////////////////////
const S3DDVector& S3DDynamicModelItem::GetAcceleration( void ) const
{
	return	m_physExogenous.vAcceleration ;
}

void S3DDynamicModelItem::SetAcceleration( const S3DDVector& vAccel )
{
	m_physExogenous.vAcceleration = vAccel ;
}

// 空間流速
//////////////////////////////////////////////////////////////////////////////
const S3DDVector& S3DDynamicModelItem::GetStream( void ) const
{
	return	m_physExogenous.vStream ;
}

void S3DDynamicModelItem::SetStream( const S3DDVector& vStream )
{
	m_physExogenous.vStream = vStream ;
}

// タイマ処理
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicModelItem::OnTimer( S3DScene& scene, uint32_t msecPast )
{
	if ( m_flagTimerAnime )
	{
		AdvanceAnimationTime( scene, msecPast, msecPast ) ;
	}
	if ( m_flagTimerPhysics )
	{
		if ( msecPast > 0 )
		{
			AddPhysicsTime( msecPast ) ;
			scene.PostSceneUpdate() ;
		}
	}
}

// アイテム作用の追加処理
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicModelItem::OnUpdateBehavior( S3DScene& scene )
{
	S3DModelBuffer *	pModel = ESLTypeCast<S3DModelBuffer>( m_pModel ) ;
	if ( pModel != NULL )
	{
		UpdateModelPose( *pModel ) ;
	}
}

// 当たり判定追加
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicModelItem::RenderLocalCollision
	( const S3DScene& scene, S3DCollision& render )
{
	ModelItem::RenderLocalCollision( scene, render ) ;
	//
	if ( m_physEffect.flagsEffects & physGlobalCollision )
	{
		S3DModelBuffer *	pModel = ESLTypeCast<S3DModelBuffer>( m_pModel ) ;
		if ( pModel != nullptr )
		{
			m_collisionBone.ClearBuffer() ;
			m_collisionBone.ResetTransformation() ;
			m_collisionBone.SetUserClassesMask( S3DCollision::colliderBone ) ;
			//
			uint32_t	maskUserColBone = 0 ;
			if ( pModel->AddAllMarkerForCollision
				( m_collisionBone,
					S3DModelData::MarkerInfo::typeBoneColider,
					nullptr, S3DCollision::colliderBone0Shift, &maskUserColBone ) > 0 )
			{
				render.SetUserClassesMask( maskUserColBone ) ;
				render.AddColliderObject( &m_collisionBone, NULL, 0 ) ;
			}
		}
	}
}

// 表示モデル追加
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicModelItem::RenderLocalModel
	( const S3DScene& scene,
		S3DRenderContextInterface& render, uint64_t flagsExclusion )
{
	S3DModelBuffer *	pModel = ESLTypeCast<S3DModelBuffer>( m_pModel ) ;
	if ( pModel != NULL )
	{
		pModel->LockModelData() ;
		if ( pModel->IsUpdateBone() )
		{
			pModel->UpdateBoneMatrix() ;
		}
		pModel->UnlockModelData() ;
	}
	ModelItem::RenderLocalModel( scene, render, flagsExclusion ) ;
}

// ポーズ設定
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicModelItem::UpdateModelPose( S3DModelBuffer& model )
{
	model.LockModelData() ;
	model.ClearAllBonesModifiedFlags() ;
	//
	S3DModelBoneSpace&	boneSpace = model.GetLocalSpaceBone() ;
	boneSpace.m_matTransformation.InitializeMatrix( S3DDVector( 1, 1, 1 ) ) ;
	boneSpace.m_vCenter = S3DDVector( 0, 0, 0 ) ;
	boneSpace.SetBoneOffset( S3DDVector( 0, 0, 0 ) ) ;
	//
	model.GetBoneRoot().ResetPhysicsBlendWeight() ;
	//
	OnPoseAnimationTrack( model ) ;
	//
	if ( DoesNeedPhysicsBoneAnimation( model ) )
	{
		if ( m_flagDelayResetPhys )
		{
			m_flagDelayResetPhys = false ;
			//
			OnPhysicsBoneAnimation( model ) ;
			ResetPhysicsParameter() ;
			//
			if ( m_nDelayDrivePhysFrame > 0 )
			{
				DrivePhysicsFrames
					( m_nDelayDrivePhysFrame, m_secDelayDriveFrame ) ;
			}
		}
		else
		{
			OnPhysicsBoneAnimation( model ) ;
		}
	}
	m_msecPastPhys = 0 ;
	//
	model.UnlockModelData() ;
}

// ボーンアニメーションするか？
//////////////////////////////////////////////////////////////////////////////
bool S3DDynamicModelItem::DoesNeedPhysicsBoneAnimation( S3DModelBuffer& model )
{
	return	model.AreAnyPhysicsBones() ;
}

// アニメーション・トラック処理
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicModelItem::OnPoseAnimationTrack( S3DModelBuffer& model )
{
	const size_t	countPoseAni = m_arrPoseTracks.GetLength() ;
	for ( size_t i = 0; i < countPoseAni; i ++ )
	{
		PoseAnimator *	pPoseAni = m_arrPoseTracks.GetAt( i ) ;
		ESLAssert( pPoseAni != NULL ) ;
		if ( pPoseAni == NULL )
		{
			continue ;
		}
		int32_t	nMinPriority = pPoseAni->GetPriority() ;
		size_t	iMinPose = i ;
		//
		for ( size_t j = i + 1; j < countPoseAni; j ++ )
		{
			PoseAnimator *	ppa = m_arrPoseTracks.GetAt( j ) ;
			ESLAssert( ppa != NULL ) ;
			if ( (ppa != NULL)
				&& (ppa->GetPriority() < nMinPriority) )
			{
				nMinPriority = ppa->GetPriority() ;
				iMinPose = j ;
				pPoseAni = ppa ;
			}
		}
		m_arrPoseTracks.Swap( i, iMinPose ) ;
		//
		pPoseAni->ApplyPose( model ) ;
	}
}

// ボーンアニメーション実行
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicModelItem::OnPhysicsBoneAnimation( S3DModelBuffer& model )
{
	//
	// コライダ設定
	//
	S3DModelBoneSpace::ColliderParam *	pColParam = nullptr ;
	S3DModelBoneSpace::ColliderParam	cpTemp ;
	S3DCollider *						pCollider = nullptr ;
	if ( !(m_physEffect.flagsEffects & physGlobalCollision) )
	{
		m_collisionBone.ClearBuffer() ;
		m_collisionBone.ResetTransformation() ;
		m_collisionBone.SetUserClassesMask( S3DCollision::colliderBoneAllMask ) ;
		if ( model.AddAllMarkerForCollision
			( m_collisionBone,
				S3DModelData::MarkerInfo::typeBoneColider,
				nullptr, S3DCollision::colliderBone0Shift ) > 0 )
		{
			pCollider = &m_collisionBone ;
			//
			cpTemp.pCollider = pCollider ;
			cpTemp.maskColUser = (uint32_t) -1 ;
			cpTemp.matCollider = S3DDMatrix( 1, 1, 1 ) ;
			cpTemp.matICollider = S3DDMatrix( 1, 1, 1 ) ;
			cpTemp.vCollider = S3DDVector( 0, 0, 0 ) ;
			pColParam = &cpTemp ;
		}
	}
	//
	// 空間情報
	//
	m_physExogenous.matLastSpace = m_physExogenous.matSpace ;
	m_physExogenous.vLastSpace = m_physExogenous.vSpace ;
	//
	S3DModelBoneSpace&	boneSpace = model.GetLocalSpaceBone() ;
	CalcGlobalTransformation
		( m_physExogenous.matSpace, m_physExogenous.vSpace ) ;
	m_physExogenous.vSpace +=
		m_physExogenous.matSpace
			* (boneSpace.m_vCenter + boneSpace.GetBoneOffset()) ;
	m_physExogenous.matSpace *= boneSpace.m_matTransformation ;
	//
	if ( m_physEffect.flagsEffects & physGlobalCollision )
	{
		cpTemp.pCollider = GetScene() ;
		if ( cpTemp.pCollider != nullptr )
		{
			cpTemp.maskColUser = S3DCollision::colliderBoneAllMask ;
			cpTemp.matCollider = m_physExogenous.matSpace ;
			cpTemp.vCollider = m_physExogenous.vSpace ;
			cpTemp.matICollider.InverseOf( cpTemp.matCollider ) ;
			pColParam = &cpTemp ;
		}
	}
	S3DDMatrix	matBoneSpace = m_physExogenous.matSpace ;
	if ( m_physEffect.flagsEffects & physNoEffectRotation )
	{
		m_physExogenous.matSpace.InitializeMatrix( S3DDVector( 1, 1, 1 ) ) ;
	}
	m_physExogenous.vSpace *= m_physEffect.fpMoveAccel ;
	//
	// 空間流速
	//
	double	secPast = esl_fmax( (double) m_msecPastPhys * 0.001, 0.1 ) ;
	S3DModelBoneSpace::PhysExogenous	physExogenous = m_physExogenous ;
	S3DDVector	vDelta = m_physExogenous.vSpace - m_physExogenous.vLastSpace ;
	double		fpMoveDelta = vDelta.Absolute() ;
	m_fpMoveLength += fpMoveDelta ;
	//
	double	fpStream = m_physEffect.fpMoveStream ;
	if ( m_physEffect.flagsEffects & physSwayStream )
	{
		S3DDMatrix	matDir( 1, 1, 1 ) ;
		matDir.RevolveForAngle( vDelta ) ;
		//
		S3DDVector	vDir( 0, 0, fpMoveDelta ) ;
		//
		double	rad = m_fpMoveLength * 2 * PI ;
		double	amp[2] = { 0.0, 0.0 } ;
		for ( int i = 0; i < 2; i ++ )
		{
			double	a = sin( rad / m_physEffect.fpSwayWaveLength[i] ) ;
			amp[i] = m_physEffect.fpSwayAmplitude[i] * a ;
		}
		vDir.x = fpMoveDelta * amp[0] ;
		vDir.y = fpMoveDelta * amp[1] ;
		vDelta = matDir * vDir ;
	}
	if ( m_msecPastPhys > 0 )
	{
		physExogenous.vStream =
			matBoneSpace.Inverse() * (vDelta * (-fpStream / secPast)) ;
	}
	//
	// ボーン物理演算
	//
	model.GetBoneRoot().CalculatePhysics( physExogenous, secPast, pColParam ) ;
	//
	model.PostUpdateBone() ;
}

// アイテム複製
//////////////////////////////////////////////////////////////////////////////
void S3DDynamicModelItem::DuplicateOf( const S3DDynamicModelItem& item )
{
	m_space = item.m_space ;
	m_classItem = item.m_classItem ;
	m_flagsBehavior = item.m_flagsBehavior ;
	//
	m_pModel = item.m_pModel ;
	m_pCollision = item.m_pCollision ;
	//
	m_arrPoseTracks.RemoveAll() ;
	for ( size_t i = 0; i < item.m_arrPoseTracks.GetLength(); i ++ )
	{
		PoseAnimator *	pPoseAni = item.m_arrPoseTracks.GetAt( i ) ;
		if ( pPoseAni != NULL )
		{
			m_arrPoseTracks.Add( pPoseAni->Duplicate() ) ;
		}
	}
	//
	m_physExogenous = item.m_physExogenous ;
	m_msecPastPhys = item.m_msecPastPhys ;
}


//////////////////////////////////////////////////////////////////////////////
// ポーズアニメーション
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DModelPoseAnimator, PoseAnimator )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DModelPoseAnimator::S3DModelPoseAnimator( void )
{
	m_pPose = NULL ;
	m_flagPlaying = false ;
	m_flagLoop = false ;
	m_fpAnimationSpeed = 1.0 ;
	m_secAnimation = 0.0 ;
	m_secEndOfAnimation = 0.0 ;
	m_secLoopStart = 0.0 ;
	m_secLoopEnd = 0.0 ;
	//
	m_pTransitionPrev = NULL ;
	m_secTransTimer = 0.0 ;
	m_secTransDuration = 0.0 ;
	//
	m_pPostAnimator = NULL ;
}

S3DModelPoseAnimator::S3DModelPoseAnimator( const S3DModelPoseAnimator& pose )
	: PoseAnimator( pose ), m_strPoseID( pose.m_strPoseID )
{
	m_pPose = pose.m_pPose ;
	m_flagPlaying = pose.m_flagPlaying ;
	m_flagLoop = pose.m_flagLoop ;
	m_fpAnimationSpeed = pose.m_fpAnimationSpeed ;
	m_secAnimation = pose.m_secAnimation ;
	m_secEndOfAnimation = pose.m_secEndOfAnimation ;
	m_secLoopStart = pose.m_secLoopStart ;
	m_secLoopEnd = pose.m_secLoopEnd ;
	//
	m_pTransitionPrev = NULL ;
	if ( pose.m_pTransitionPrev != NULL )
	{
		m_pTransitionPrev = pose.m_pTransitionPrev->Duplicate() ;
	}
	m_secTransTimer = pose.m_secTransTimer ;
	m_secTransDuration = pose.m_secTransDuration ;
	//
	m_pPostAnimator = NULL ;
	if ( pose.m_pPostAnimator != NULL )
	{
		m_pPostAnimator = pose.m_pPostAnimator->Duplicate() ;
	}
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DModelPoseAnimator::~S3DModelPoseAnimator( void )
{
	delete	m_pTransitionPrev ;
	delete	m_pPostAnimator ;
}

// ポーズ設定
//////////////////////////////////////////////////////////////////////////////
void S3DModelPoseAnimator::SetPose
	( S3DModelPose * pPose, bool fVolatile, const wchar_t * pwszID )
{
	m_pPose = pPose ;
	m_strPoseID = pwszID ;
	m_flags = (m_flags & ~flagVolatile) | (fVolatile ? flagVolatile : 0) ;
	//
	m_flagPlaying = false ;
	m_secAnimation = 0.0 ;
	m_secEndOfAnimation = 0.0 ;
	//
	if ( pPose != NULL )
	{
		if ( pPose->GetMetaInfo().msecDuration > 0 )
		{
			m_secEndOfAnimation =
				pPose->GetMetaInfo().msecDuration * 0.001 ;
			m_flagPlaying = true ;
		}
	}
}

// ポーズ区間設定
//////////////////////////////////////////////////////////////////////////////
void S3DModelPoseAnimator::SetDuration( double secStart, double secEnd )
{
	m_flagPlaying = (secStart < secEnd) ;
	m_secAnimation = secStart ;
	m_secEndOfAnimation = secEnd ;
}

// ループ設定
//////////////////////////////////////////////////////////////////////////////
void S3DModelPoseAnimator::SetLoop
	( bool fLoop, double secStart, double secEnd )
{
	m_flagLoop = fLoop ;
	m_secLoopStart = secStart ;
	m_secLoopEnd = (secEnd > secStart) ? secEnd : m_secEndOfAnimation ;
}

// アニメーション速度設定
//////////////////////////////////////////////////////////////////////////////
void S3DModelPoseAnimator::SetAnimationSpeed( double fpSpeed )
{
	m_fpAnimationSpeed = fpSpeed ;
}

// アニメーション中か？
//////////////////////////////////////////////////////////////////////////////
bool S3DModelPoseAnimator::IsAnimationPlaying( void ) const
{
	return	m_flagPlaying || (m_pTransitionPrev != NULL) ;
}

// トランジッションを設定する
//////////////////////////////////////////////////////////////////////////////
void S3DModelPoseAnimator::SetTransition
	( S3DModelPose * pBasePose, double secDuration, double v0, double v1 )
{
	S3DModelPoseAnimator *	pAnime = new S3DModelPoseAnimator ;
	pAnime->SetPose( pBasePose ) ;
	pAnime->SetDuration( 0.0, secDuration ) ;
	SetTransition( pAnime, secDuration, v0, v1 ) ;
}

void S3DModelPoseAnimator::SetTransition
	( S3DModelPoseAnimator::PoseAnimator * pBasePose,
					double secDuration, double v0, double v1 )
{
	delete	m_pTransitionPrev ;
	m_secTransTimer = 0.0 ;
	m_secTransDuration = secDuration ;
	m_bzTransTime.SetLine( 0, 1, v0, v1 ) ;
	m_pTransitionPrev = pBasePose ;
}

// 次のアニメーションを設定する
//////////////////////////////////////////////////////////////////////////////
void S3DModelPoseAnimator::SetPostAnimator
		( S3DDynamicModelItem::PoseAnimator * pAnimator )
{
	delete	m_pPostAnimator ;
	m_pPostAnimator = pAnimator ;
}

// ポーズ取得
//////////////////////////////////////////////////////////////////////////////
S3DModelPose * S3DModelPoseAnimator::GetPose( void ) const
{
	return	m_pPose ;
}

// ポーズ識別子取得
//////////////////////////////////////////////////////////////////////////////
const SSystem::SString& S3DModelPoseAnimator::GetPoseID( void ) const
{
	return	m_strPoseID ;
}

// 次のアニメーションを取得する
//////////////////////////////////////////////////////////////////////////////
S3DDynamicModelItem::PoseAnimator *
		S3DModelPoseAnimator::GetPostAnimation( void ) const
{
	return	m_pPostAnimator ;
}

// 時間経過 (終了時に true 返却)
//////////////////////////////////////////////////////////////////////////////
bool S3DModelPoseAnimator::OnTimer
	( S3DDynamicModelItem& item, uint32_t msecPast )
{
	if ( m_pTransitionPrev != NULL )
	{
		m_secTransTimer += msecPast * 0.001 ;
		if ( m_secTransTimer >= m_secTransDuration )
		{
			delete	m_pTransitionPrev ;
			m_pTransitionPrev = NULL ;
			m_secTransTimer = m_secTransDuration ;
		}
	}
	if ( m_flagPlaying )
	{
		m_secAnimation += m_fpAnimationSpeed * msecPast * 0.001 ;
		if ( m_flagLoop )
		{
			if ( m_secLoopEnd <= m_secAnimation )
			{
				m_secAnimation = m_secLoopStart ;
			}
		}
		else if ( m_secEndOfAnimation < m_secAnimation )
		{
			m_secAnimation = m_secEndOfAnimation ;
			m_flagPlaying = false ;
			//
			if ( m_pPostAnimator != NULL )
			{
				item.AddPoseAnimator( m_pPostAnimator ) ;
				m_pPostAnimator = NULL ;
			}
			return	true ;
		}
	}
	return	!m_flagPlaying && (IsVolatilePose() && (m_pTransitionPrev == NULL)) ;
}

// ポーズ設定
//////////////////////////////////////////////////////////////////////////////
void S3DModelPoseAnimator::ApplyPose( S3DModelBuffer& model )
{
	if ( m_pPose != NULL )
	{
		double	w = m_blend ;
		if ( m_pTransitionPrev != NULL )
		{
			m_pTransitionPrev->ApplyPose( model ) ;
			//
			if ( m_secTransDuration > 0 )
			{
				double	t = m_secTransTimer / m_secTransDuration ;
				w *= m_bzTransTime.PointAt( t ) ;
			}
		}
		m_pPose->ApplyPoseTo( model, w, m_secAnimation ) ;
	}
}

// 即時完了
//////////////////////////////////////////////////////////////////////////////
void S3DModelPoseAnimator::FlushAnimation( S3DDynamicModelItem& item )
{
	if ( m_flagPlaying )
	{
		m_secAnimation = m_secEndOfAnimation ;
		m_flagPlaying = false ;
	}
}

// キャンセル（アニメーションの停止）
//////////////////////////////////////////////////////////////////////////////
void S3DModelPoseAnimator::CancelAnimation( S3DDynamicModelItem& item )
{
	m_flagPlaying = false ;
}

// 複製
//////////////////////////////////////////////////////////////////////////////
S3DDynamicModelItem::PoseAnimator * S3DModelPoseAnimator::Duplicate( void )
{
	return	new S3DModelPoseAnimator( *this ) ;
}

// 保存
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelPoseAnimator::SavePoseAnimator( SFileInterface& file )
{
	file.Write( &m_priority, sizeof(m_priority) ) ;
	file.Write( &m_flags, sizeof(m_flags) ) ;
	file.Write( &m_blend, sizeof(m_blend) ) ;
	//
	file.WriteString( m_strPoseID ) ;
	//
	uint32_t	nFlags = 0 ;
	nFlags |= m_flagPlaying ? 1 : 0 ;
	nFlags |= m_flagLoop ? 2 : 0 ;
	file.Write( &nFlags, sizeof(uint32_t) ) ;
	//
	file.Write( &m_fpAnimationSpeed, sizeof(m_fpAnimationSpeed) ) ;
	file.Write( &m_secAnimation, sizeof(m_secAnimation) ) ;
	file.Write( &m_secEndOfAnimation, sizeof(m_secEndOfAnimation) ) ;
	file.Write( &m_secLoopStart, sizeof(m_secLoopStart) ) ;
	file.Write( &m_secLoopEnd, sizeof(m_secLoopEnd) ) ;
	//
	S3DModelPoseAnimator *	pPostAni =
			ESLTypeCast<S3DModelPoseAnimator>( m_pPostAnimator ) ;
	if ( pPostAni != NULL )
	{
		nFlags = 1 ;
		file.Write( &nFlags, sizeof(uint32_t) ) ;
		pPostAni->SavePoseAnimator( file ) ;
	}
	else
	{
		nFlags = 0 ;
		file.Write( &nFlags, sizeof(uint32_t) ) ;
	}
	return	sglErrSuccess ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelPoseAnimator::LoadPoseAnimator
			( const S3DModelPoseLibrary& libPose, SFileInterface& file )
{
	file.Read( &m_priority, sizeof(m_priority) ) ;
	file.Read( &m_flags, sizeof(m_flags) ) ;
	file.Read( &m_blend, sizeof(m_blend) ) ;
	//
	file.ReadString( m_strPoseID ) ;
	m_pPose = libPose.GetPoseAs( m_strPoseID ) ;
	//
	uint32_t	nFlags = 0 ;
	file.Read( &nFlags, sizeof(uint32_t) ) ;
	m_flagPlaying = ((nFlags & 1) != 0) ;
	m_flagLoop = ((nFlags & 2) != 0) ;
	//
	file.Read( &m_fpAnimationSpeed, sizeof(m_fpAnimationSpeed) ) ;
	file.Read( &m_secAnimation, sizeof(m_secAnimation) ) ;
	file.Read( &m_secEndOfAnimation, sizeof(m_secEndOfAnimation) ) ;
	file.Read( &m_secLoopStart, sizeof(m_secLoopStart) ) ;
	file.Read( &m_secLoopEnd, sizeof(m_secLoopEnd) ) ;
	//
	nFlags = 0 ;
	file.Read( &nFlags, sizeof(uint32_t) ) ;
	if ( nFlags == 1 )
	{
		ESLAssert( m_pPostAnimator == NULL ) ;
		S3DModelPoseAnimator *	pPoseAni = new S3DModelPoseAnimator ;
		pPoseAni->LoadPoseAnimator( libPose, file ) ;
		m_pPostAnimator = pPoseAni ;
	}
	return	sglErrSuccess ;
}



//////////////////////////////////////////////////////////////////////////////
// ポリゴン・リダクション
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DSimpleMeshReduction, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DSimpleMeshReduction::S3DSimpleMeshReduction( void )
	: m_pMesh( nullptr )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DSimpleMeshReduction::~S3DSimpleMeshReduction( void )
{
	Release() ;
}

// 変換元メッシュ・セットアップ
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSimpleMeshReduction::SetupTargetMesh( S3DRenderBuffer::MeshBuffer * pMesh )
{
	ESLAssert( pMesh != nullptr ) ;
	if ( pMesh->m_type != primitiveTriangle )
	{
		return	sglErrFailed ;
	}
	m_pMesh = pMesh ;
	//
	// 頂点情報初期化
	//
	const size_t	nVertexCount = pMesh->m_nVertexCount ;
	m_vertices.SetLength( nVertexCount ) ;
	//
	VertexInfo *	pvi = m_vertices.GetArray() ;
	for ( size_t i = 0; i < nVertexCount; i ++ )
	{
		VertexInfo&	vi = pvi[i] ;
		vi.fixed = false ;
		vi.collapsed = false ;
		vi.removed = false ;
		vi.bend = 0.0f ;
		vi.nEdges = 0 ;
		vi.nEdgesBuf = 2 ;
		vi.ppEdges = (EdgeInfo**) m_stackBuf.Allocate
									( vi.nEdgesBuf * sizeof(EdgeInfo*) ) ;
	}
	m_vertices.FinishArray() ;
	//
	// 面情報構築
	//
	const uint32_t *	pIndices = pMesh->m_bufIndex.GetConstArray() ;
	const size_t		nTriangles = pMesh->m_nIndexCount / 3 ;
	m_triangles.SetLength( nTriangles ) ;
	//
	for ( size_t i = 0; i < nTriangles; i ++ )
	{
		const uint32_t *	pTriangle = pIndices + i * 3 ;
		const uint32_t		vi0 = pTriangle[0] ;
		const uint32_t		vi1 = pTriangle[1] ;
		const uint32_t		vi2 = pTriangle[2] ;
		ESLAssert( pTriangle[0] < nVertexCount ) ;
		ESLAssert( pTriangle[1] < nVertexCount ) ;
		ESLAssert( pTriangle[2] < nVertexCount ) ;
		//
		TriangleInfo&	ti = m_triangles.At(i) ;
		ti.collapsed = (vi0 == vi1) || (vi1 == vi2) || (vi0 == vi2) ;
		ti.vNormal = CalcNormalOfTriangle
						( (size_t) vi0, (size_t) vi1, (size_t) vi2 ) ;
		ti.iVertex[0] = vi0 ;
		ti.iVertex[1] = vi1 ;
		ti.iVertex[2] = vi2 ;
		ti.pEdges[0] = nullptr ;
		ti.pEdges[1] = nullptr ;
		ti.pEdges[2] = nullptr ;
	}
	//
	// 辺情報構築
	//
	for ( size_t i = 0; i < nTriangles; i ++ )
	{
		const uint32_t *	pTriangle = pIndices + i * 3 ;
		TriangleInfo&		ti = m_triangles.At(i) ;
		if ( !ti.collapsed )
		{
			ti.pEdges[0] = AddEdgeOfTriangle( pTriangle[0], pTriangle[1], i ) ;
			ti.pEdges[1] = AddEdgeOfTriangle( pTriangle[1], pTriangle[2], i ) ;
			ti.pEdges[2] = AddEdgeOfTriangle( pTriangle[0], pTriangle[2], i ) ;
		}
	}
	//
	// 頂点に曲がり具合集計
	//
	for ( size_t i = 0; i < nVertexCount; i ++ )
	{
		VertexInfo&	vi = m_vertices.At(i) ;
		EdgeInfo **	ppEdges = vi.ppEdges ;
		for ( size_t j = 0; j < vi.nEdges; j ++ )
		{
			if ( ppEdges[j] != nullptr )
			{
				vi.bend += ppEdges[j]->bend ;
				if ( ppEdges[j]->nShared <= 1 )
				{
					vi.fixed = true ;
				}
			}
		}
	}
	//
	// ソートの準備と分布計算
	//
	size_t *	pSorted = m_sorted.GetArray( nVertexCount ) ;
	size_t		nSorted = 0 ;
	double		sumBend = 0.0 ;
	double		sum2Bend = 0.0 ;
	for ( size_t i = 0; i < nVertexCount; i ++ )
	{
		VertexInfo&	vi = m_vertices.At(i) ;
		if ( !vi.fixed )
		{
			pSorted[nSorted ++] = i ;
			//
			sumBend += vi.bend ;
			sum2Bend += vi.bend * vi.bend ;
		}
	}
	m_meanBend = 0.0 ;
	m_varBend = 0.0 ;
	if ( nSorted > 0 )
	{
		m_meanBend = sumBend / (double) nSorted ;
		m_varBend = sum2Bend / (double) nSorted - m_meanBend * m_meanBend ;
	}
	m_sorted.FinishArray() ;
	m_sorted.SetLength( nSorted ) ;

	return	sglErrSuccess ;
}

// バッファ開放
//////////////////////////////////////////////////////////////////////////////
void S3DSimpleMeshReduction::Release( void )
{
	m_pMesh = nullptr ;
	m_sorted.FreeArray() ;
	m_vertices.FreeArray() ;
	m_triangles.FreeArray() ;
	m_reduced.FreeArray() ;
	m_stackBuf.FreeAll() ;
}

// 削減処理
//////////////////////////////////////////////////////////////////////////////
size_t S3DSimpleMeshReduction::ReduceTriangles( double threshold )
{
	// threshold の 0.0～1.0 を -σ～+σ として変換する
	const float32_t	bendThreshold =
		(float32_t) ((threshold - 0.5) * 2.0 * sqrt(m_varBend) + m_meanBend) ;

	// ソート
	SortVertices( bendThreshold ) ;

	size_t	nReduced = 0 ;
	for ( size_t i = 0; i < m_sorted.GetLength(); i ++ )
	{
		const size_t	iVertex = m_sorted.At(i) ;
		VertexInfo&		vi = m_vertices.At( iVertex ) ;
		if ( vi.fixed || vi.collapsed || vi.removed )
		{
			continue ;
		}
		if ( vi.bend > bendThreshold )
		{
			break ;
		}
		//
		// 移動先の頂点を探す
		//
		size_t		iMoveTo = iVertex ;
		float32_t	minDelta = 1.0e+38f ;
		EdgeInfo *	pColEdge = nullptr ;
		EdgeInfo **	ppEdges = vi.ppEdges ;
		for ( size_t j = 0; j < vi.nEdges; j ++ )
		{
			EdgeInfo *	pEdge = ppEdges[j] ;
			if ( pEdge == nullptr )
			{
				continue ;
			}
			ESLAssert( (pEdge->iVertex0 == iVertex) || (pEdge->iVertex1 == iVertex) ) ;
			const size_t	iNext = (pEdge->iVertex0 == iVertex)
									? pEdge->iVertex1 : pEdge->iVertex0 ;
			if ( iNext == iVertex )
			{
				continue ;
			}
			VertexInfo&		viNext = m_vertices.At( iNext ) ;
			float32_t	cosDelta = MaximumAngleOfEdgeDelta( iVertex, iMoveTo ) ;
			if ( cosDelta < minDelta )
			{
				minDelta = cosDelta ;
				iMoveTo = iNext ;
				pColEdge = pEdge ;
			}
		}
		if ( pColEdge == nullptr )
		{
			continue ;
		}
		//
		// 移動する稜線を含む三角を潰す
		//
		for ( size_t j = 0; j < pColEdge->nShared; j ++ )
		{
			TriangleInfo&	ti = m_triangles.At( pColEdge->pTriangles[j] ) ;
			CollapseAndMergeEdge
				( ti, pColEdge->pTriangles[j], pColEdge, iVertex, iMoveTo ) ;
		}
		//
		// それ以外の三角の頂点を移動する
		//
		MoveVertex( iVertex, iMoveTo ) ;
		//
		nReduced ++ ;
	}

	// 三角リスト構築
	BuildTriangleList() ;

	return	nReduced ;
}

// ReduceTriangles 後に再度 ReduceTriangles するための処理
//////////////////////////////////////////////////////////////////////////////
void S3DSimpleMeshReduction::RecycleMesh( void )
{
	//
	// 面の法線更新
	//
	for ( size_t i = 0; i < m_triangles.GetLength(); i ++ )
	{
		TriangleInfo&	ti = m_triangles.At(i) ;
		if ( !ti.collapsed )
		{
			ti.vNormal =
				CalcNormalOfTriangle
					( (size_t) ti.iVertex[0],
						(size_t) ti.iVertex[1], (size_t) ti.iVertex[2] ) ;
		}
	}
	//
	// 頂点に曲がり具合集計
	//
	for ( size_t i = 0; i < m_vertices.GetLength(); i ++ )
	{
		VertexInfo&	vi = m_vertices.At(i) ;
		EdgeInfo **	ppEdges = vi.ppEdges ;
		vi.collapsed = false ;
		vi.bend = 0 ;
		for ( size_t j = 0; j < vi.nEdges; j ++ )
		{
			EdgeInfo *	pEdge = ppEdges[j] ;
			if ( pEdge != nullptr )
			{
				pEdge->bend = 0.0f ;
				if ( pEdge->nShared > 1 )
				{
					TriangleInfo&	ti0 = m_triangles.At( pEdge->pTriangles[0] ) ;
					for ( size_t k = 1; k < pEdge->nShared; k ++ )
					{
						TriangleInfo&	tik = m_triangles.At( pEdge->pTriangles[k] ) ;
						float32_t	cosBend = ti0.vNormal.InnerProduct( tik.vNormal ) ;
						pEdge->bend += 1.0f - cosBend ;
					}
					vi.bend += pEdge->bend ;
				}
				else
				{
					vi.fixed = true ;
				}
			}
		}
	}
	//
	// ソート準備
	//
	size_t *	pSorted = m_sorted.GetArray( m_vertices.GetLength() ) ;
	size_t		nSorted = 0 ;
	double		sumBend = 0.0 ;
	double		sum2Bend = 0.0 ;
	for ( size_t i = 0; i < m_vertices.GetLength(); i ++ )
	{
		VertexInfo&	vi = m_vertices.At(i) ;
		if ( !vi.fixed )
		{
			pSorted[nSorted ++] = i ;
		}
	}
	m_sorted.FinishArray() ;
	m_sorted.SetLength( nSorted ) ;
}

// 削減後インデックス数取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DSimpleMeshReduction::GetReducedIndexCount( void ) const
{
	return	m_reduced.GetLength() ;
}

// 削減後インデックス配列取得
//////////////////////////////////////////////////////////////////////////////
const uint32_t * S3DSimpleMeshReduction::GetReducedIndexList( void ) const
{
	return	m_reduced.GetConstArray() ;
}

// ポリゴンの辺を検索
//////////////////////////////////////////////////////////////////////////////
S3DSimpleMeshReduction::EdgeInfo *
	S3DSimpleMeshReduction::FindEdge( size_t iVertex0, size_t iVertex1 ) const
{
	if ( iVertex0 > iVertex1 )
	{
		size_t	t = iVertex0 ;
		iVertex0 = iVertex1 ;
		iVertex1 = t ;
	}
	VertexInfo&	vi = m_vertices.At(iVertex0) ;
	for ( size_t i = 0; i < vi.nEdges; i ++ )
	{
		ESLAssert( vi.ppEdges[i] != nullptr ) ;
		EdgeInfo *	pEdge = vi.ppEdges[i] ;
		if ( (pEdge->iVertex0 == iVertex0)
			&& (pEdge->iVertex1 == iVertex1) )
		{
			return	pEdge ;
		}
	}
	return	nullptr ;
}

// ポリゴンの辺を追加
//////////////////////////////////////////////////////////////////////////////
S3DSimpleMeshReduction::EdgeInfo *
	S3DSimpleMeshReduction::AddEdgeOfTriangle
		( size_t iVertex0, size_t iVertex1, size_t iTriangle )
{
	if ( iVertex0 > iVertex1 )
	{
		size_t	t = iVertex0 ;
		iVertex0 = iVertex1 ;
		iVertex1 = t ;
	}
	EdgeInfo *	pEdge = FindEdge( iVertex0, iVertex1 ) ;
	if ( pEdge != nullptr )
	{
		TriangleInfo&	ti0 = m_triangles.At( pEdge->iTriangle0 ) ;
		TriangleInfo&	ti1 = m_triangles.At( iTriangle ) ;
		//
		float32_t	cosBend = ti0.vNormal.InnerProduct( ti1.vNormal ) ;
		pEdge->bend += 1.0f - cosBend ;
		//
		AddTriangleOfEdge( *pEdge, iTriangle ) ;
	}
	else
	{
		pEdge = (EdgeInfo*) m_stackBuf.Allocate( sizeof(EdgeInfo) ) ;
		pEdge->iVertex0 = iVertex0 ;
		pEdge->iVertex1 = iVertex1 ;
		pEdge->iTriangle0 = iTriangle ;
		pEdge->nShared = 1 ;
		pEdge->nTrianglesBuf = 2 ;
		pEdge->pTriangles =
				(size_t*) m_stackBuf.Allocate
							( pEdge->nTrianglesBuf * sizeof(size_t) ) ;
		pEdge->pTriangles[0] = iTriangle ;
		pEdge->bend = 0.0f ;
		//
		AddEdgeOfVertex( m_vertices.At( iVertex0 ), pEdge ) ;
		AddEdgeOfVertex( m_vertices.At( iVertex1 ), pEdge ) ;
	}
	return	pEdge ;
}

// 頂点に繋がる辺情報追加
//////////////////////////////////////////////////////////////////////////////
void S3DSimpleMeshReduction::AddEdgeOfVertex( VertexInfo& vi, EdgeInfo * pEdge )
{
	if ( vi.nEdges >= vi.nEdgesBuf )
	{
		EdgeInfo **	ppEdges = vi.ppEdges ;
		vi.nEdgesBuf *= 2 ;
		vi.ppEdges = (EdgeInfo**) m_stackBuf.Allocate
									( vi.nEdgesBuf * sizeof(EdgeInfo*) ) ;
		eslCopyMemory
			( vi.ppEdges, ppEdges, vi.nEdges * sizeof(EdgeInfo*) ) ;
	}
	ESLAssert( vi.nEdges < vi.nEdgesBuf ) ;
	vi.ppEdges[vi.nEdges ++] = pEdge ;
}

// 頂点に繋がる辺情報削除
//////////////////////////////////////////////////////////////////////////////
void S3DSimpleMeshReduction::RemoveEdgeOfVertex( VertexInfo& vi, EdgeInfo * pEdge )
{
	EdgeInfo **	ppEdges = vi.ppEdges ;
	for ( size_t i = 0; i < vi.nEdges; i ++ )
	{
		if ( ppEdges[i] == pEdge )
		{
			for ( size_t j = i + 1; j < vi.nEdges; j ++ )
			{
				ppEdges[j - 1] = ppEdges[j] ;
			}
			vi.nEdges -- ;
			return ;
		}
	}
}

// 辺を含む三角をリストに追加
//////////////////////////////////////////////////////////////////////////////
void S3DSimpleMeshReduction::AddTriangleOfEdge( EdgeInfo& edge, size_t iTriangle )
{
	for ( size_t i = 0; i < edge.nShared; i ++ )
	{
		if ( edge.pTriangles[i] == iTriangle )
		{
			return ;
		}
	}
	if ( edge.nShared >= edge.nTrianglesBuf )
	{
		size_t *	pBuf = edge.pTriangles ;
		edge.nTrianglesBuf *= 2 ;
		edge.pTriangles =
			(size_t*) m_stackBuf.Allocate
						( edge.nTrianglesBuf * sizeof(size_t) ) ;
		eslCopyMemory
			( edge.pTriangles,
				pBuf, edge.nShared * sizeof(size_t) ) ;
	}
	ESLAssert( edge.nShared < edge.nTrianglesBuf ) ;
	edge.pTriangles[edge.nShared ++] = iTriangle ;
}

// 三角の特定の辺を縮退して残りの辺を結合処理
//////////////////////////////////////////////////////////////////////////////
void S3DSimpleMeshReduction::CollapseAndMergeEdge
	( TriangleInfo& ti, size_t iTriangle,
		EdgeInfo * pColEdge, size_t iVertex0, size_t iMoveTo )
{
	//
	// 移動元の辺と、移動先の辺を検索
	//
	EdgeInfo *	pEdge0 = nullptr ;
	EdgeInfo *	pEdge1 = nullptr ;
	for ( size_t i = 0; i < 3; i ++ )
	{
		EdgeInfo *	pEdge = ti.pEdges[i] ;
		if ( pColEdge != pEdge )
		{
			if ( (pEdge->iVertex0 == iVertex0)
				|| (pEdge->iVertex1 == iVertex0) )
			{
				pEdge0 = pEdge ;
			}
			if ( (pEdge->iVertex0 == iMoveTo)
				|| (pEdge->iVertex1 == iMoveTo) )
			{
				pEdge1 = pEdge ;
			}
		}
	}
	if ( (pEdge0 != nullptr) && (pEdge1 != nullptr) )
	{
		//
		// 移動先の辺に移動元の辺に接する三角を連結
		//
		for ( size_t i = 0; i < pEdge0->nShared; i ++ )
		{
			if ( pEdge0->pTriangles[i] != iTriangle )
			{
				AddTriangleOfEdge( *pEdge1, pEdge0->pTriangles[i] ) ;
			}
		}
	}
	ti.collapsed = true ;
}

// 頂点を移動したときの辺周りの面の角度変化が最大のものを調べる
//////////////////////////////////////////////////////////////////////////////
float32_t S3DSimpleMeshReduction::MaximumAngleOfEdgeDelta
							( size_t iVertex0, size_t iMoveTo )
{
	VertexInfo&	vi0 = m_vertices.At( iVertex0 ) ;
	S3DVector	vVertex0 = m_pMesh->m_bufVertex.At(iVertex0) ;
	S3DVector	vMoveTo = m_pMesh->m_bufVertex.At(iMoveTo) ;
	float32_t	maxDelta = 0.0f ;
	for ( size_t i = 0; i < vi0.nEdges; i ++ )
	{
		EdgeInfo *	pEdge = vi0.ppEdges[i] ;
		if ( pEdge == nullptr )
		{
			continue ;
		}
		if ( pEdge->nShared < 2 )
		{
			continue ;
		}
		TriangleInfo*	pti[2] = 
		{
			m_triangles.GetAt( pEdge->pTriangles[0] ),
			m_triangles.GetAt( pEdge->pTriangles[1] )
		} ;
		if ( pti[0]->collapsed || pti[1]->collapsed )
		{
			continue ;
		}
		size_t	iMoved[2][3] ;
		for ( size_t j = 0; j < 2; j ++ )
		{
			for ( size_t k = 0; k < 3; k ++ )
			{
				iMoved[j][k] = pti[j]->iVertex[k] ;
				if ( iMoved[j][k] == iVertex0 )
				{
					iMoved[j][k] = iMoveTo ;
				}
			}
		}
		S3DVector	vNormal[2], vMovedNormal[2] ;
		for ( size_t j = 0; j < 2; j ++ )
		{
			vNormal[j] =
				CalcNormalOfTriangle
					( (size_t) pti[j]->iVertex[0],
						(size_t) pti[j]->iVertex[1],
						(size_t) pti[j]->iVertex[2] ) ;
			vMovedNormal[j] =
				CalcNormalOfTriangle
					( iMoved[j][0], iMoved[j][1], iMoved[j][2] ) ;
		}
		float32_t	cosBeforeMove = vNormal[0].InnerProduct( vNormal[1] ) ;
		float32_t	cosAfterMove = vMovedNormal[0].InnerProduct( vMovedNormal[1] ) ;
		float32_t	cosDelta = (float32_t) fabs( cosBeforeMove - cosAfterMove ) ;
		if ( maxDelta < cosDelta )
		{
			maxDelta = cosDelta ;
		}
	}
	return	maxDelta ;
}

// 面の法線を計算する
//////////////////////////////////////////////////////////////////////////////
S3DVector S3DSimpleMeshReduction::CalcNormalOfTriangle
				( size_t vi0, size_t vi1, size_t vi2 ) const
{
	S3DVector	v0 = m_pMesh->m_bufVertex.At(vi0) ;
	S3DVector	v1 = m_pMesh->m_bufVertex.At(vi1) ;
	S3DVector	v2 = m_pMesh->m_bufVertex.At(vi2) ;
	S3DVector	vNormal = (v1 - v0) * (v2 - v0) ;
	vNormal.Normalize() ;
	return	vNormal ;
}

// 頂点を移動して頂点を含むすべての三角を再構築する
//////////////////////////////////////////////////////////////////////////////
void S3DSimpleMeshReduction::MoveVertex( size_t iVertex0, size_t iMoveTo )
{
	VertexInfo&	vi0 = m_vertices.At( iVertex0 ) ;
	for ( size_t i = 0; i < vi0.nEdges; i ++ )
	{
		EdgeInfo *	pEdge = vi0.ppEdges[i] ;
		if ( pEdge == nullptr )
		{
			continue ;
		}
		for ( size_t j = 0; j < pEdge->nShared; j ++ )
		{
			TriangleInfo&	tri = m_triangles.At( pEdge->pTriangles[j] ) ;
			if ( tri.collapsed )
			{
				continue ;
			}
			static const size_t	iShift1[5] = { 2, 0, 1, 2, 0 } ;
			for ( size_t k = 0; k < 3; k ++ )
			{
				if ( tri.iVertex[k] == iVertex0 )
				{
					tri.iVertex[k] = (uint32_t) iMoveTo ;
					tri.pEdges[k] =
						AddEdgeOfTriangle
							( tri.iVertex[k],
								tri.iVertex[iShift1[k+2]],
								pEdge->pTriangles[j] ) ;
					tri.pEdges[iShift1[k]] =
						AddEdgeOfTriangle
							( tri.iVertex[k],
								tri.iVertex[iShift1[k]],
								pEdge->pTriangles[j] ) ;
				}
			}
			tri.collapsed = (tri.iVertex[0] == tri.iVertex[1])
							|| (tri.iVertex[1] == tri.iVertex[2])
							|| (tri.iVertex[0] == tri.iVertex[2]) ;
		}
		//
		// 移動元の辺は削除
		//
		ESLAssert( (pEdge->iVertex0 == iVertex0) || (pEdge->iVertex1 == iVertex0) ) ;
		size_t	iAnother = (pEdge->iVertex0 == iVertex0)
							? pEdge->iVertex1 : pEdge->iVertex0 ;
		if ( iAnother != iVertex0 )
		{
			RemoveEdgeOfVertex( m_vertices.At(iAnother), pEdge ) ;
		}
	}
	vi0.nEdges = 0 ;
	vi0.removed = true ;
	//
	m_vertices.At( iMoveTo ).collapsed = true ;
}

// 頂点をソートする
//////////////////////////////////////////////////////////////////////////////
void S3DSimpleMeshReduction::SortVertices( float32_t bendThreshold )
{
	size_t *	pSorted = m_sorted.GetArray() ;
	size_t		nCount = m_sorted.GetLength() ;
	for ( size_t i = 0; i + 1 < nCount; i ++ )
	{
		VertexInfo&	vi0 = m_vertices.At( pSorted[i] ) ;
		float32_t	minBend = vi0.bend ;
		size_t		minIndex = i ;
		for ( size_t j = i + 1; j < nCount; j ++ )
		{
			VertexInfo&	vij = m_vertices.At( pSorted[j] ) ;
			if ( minBend > vij.bend )
			{
				minBend = vij.bend ;
				minIndex = j ;
			}
		}
		if ( minIndex != i )
		{
			size_t	t = pSorted[i] ;
			pSorted[i] = pSorted[minIndex] ;
			pSorted[minIndex] = t ;
		}
		if ( minBend > bendThreshold )
		{
			break ;
		}
	}
	m_sorted.FinishArray() ;
}

// 三角リストを構築
//////////////////////////////////////////////////////////////////////////////
void S3DSimpleMeshReduction::BuildTriangleList( void )
{
	size_t	nTriangles = 0 ;
	for ( size_t i = 0; i < m_triangles.GetLength(); i ++ )
	{
		TriangleInfo&	ti = m_triangles.At(i) ;
		if ( !ti.collapsed )
		{
			nTriangles ++ ;
		}
	}
	m_reduced.SetLength( nTriangles * 3 ) ;
	//
	uint32_t *	pIndex = m_reduced.GetArray() ;
	size_t		iDst = 0 ;
	for ( size_t i = 0; i < m_triangles.GetLength(); i ++ )
	{
		TriangleInfo&	ti = m_triangles.At(i) ;
		if ( !ti.collapsed )
		{
			pIndex[iDst]     = ti.iVertex[0] ;
			pIndex[iDst + 1] = ti.iVertex[1] ;
			pIndex[iDst + 2] = ti.iVertex[2] ;
			iDst += 3 ;
		}
	}
	ESLAssert( iDst == m_reduced.GetLength() ) ;
}


