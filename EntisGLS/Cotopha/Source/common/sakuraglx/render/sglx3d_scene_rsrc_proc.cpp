
#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl2d/sgl_image_conversion.h>
#include <sakuraglx/render/sglx3d_scene_composer.h>
#include <sakuraglx/render/sglx3d_scene_rsrc_proc.h>
#include <sakuraglx/render/sglx3d_scene_item.h>
#include <sakuraglx/render/sglx3d_scene_edit_mesh.h>
/*
#include <loquaty_date_time.h>
#include <loquaty_file.h>
#include <loquaty_parser.h>
#include <loquaty_source_file.h>

#include <loquaty_array_buffer.h>
#include <loquaty_arrangement.h>
#include <loquaty_code_buffer.h>
#include <loquaty_stack_buffer.h>
#include <loquaty_function.h>

#include <loquaty_obj_array.h>
#include <loquaty_obj_map.h>
#include <loquaty_obj_generic.h>

#include <loquaty_namespace.h>
*/
using namespace SSystem ;
using namespace SakuraGL ;
using namespace SakuraCL ;


//////////////////////////////////////////////////////////////////////////////
// テクスチャ・アトラス化
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::S3DResourceAtlasTextureProc, ImageRsrcProcedure, Controller )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DResourceAtlasTextureProc, atlas_texture )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DResourceAtlasTextureProc::S3DResourceAtlasTextureProc( void )
	: Controller( m_ItemClassDescriptor.pwszClassID ),
		m_pComposer( NULL ),
		m_sizeInit( 128, 128 ),
		m_flagSizePOT( false ),
		m_flagClampBorder( false ),
		m_nRefCount( 4 )
{
	AddParameterEntry
		( L"init_width", S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrConstant,
			L"画像幅", L"アトラス化した水平画像サイズ。"
						L"不足した場合には自動的に伸張します。" ) ;
	AddParameterEntry
		( L"init_height", S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrConstant,
			L"画像高", L"アトラス化した垂直画像サイズ。"
						L"不足した場合には自動的に伸張します。" ) ;
	AddParameterEntry
		( L"size_pot", S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrConstant,
			L"サイズ２の累乗化",
			L"アトラス化した画像サイズが不足した場合に２の累乗サイズに伸張します。" ) ;
	AddParameterEntry
		( L"clamp_border", S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrConstant,
			L"境界ピクセルの拡張",
			L"アトラス化する際に周囲を1ピクセル拡張する（false の場合には透明ピクセルを挿入）" ) ;
	AddParameterEntry
		( L"ref_count", S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrDynamicValidation,
			L"参照画像数", NULL ) ;
	//
	UpdateParameterEntry() ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DResourceAtlasTextureProc::~S3DResourceAtlasTextureProc( void )
{
}

// 参照画像プロパティエントリ設定
//////////////////////////////////////////////////////////////////////////////
void S3DResourceAtlasTextureProc::UpdateParameterEntry( void )
{
	ChopParameterEntryLastAt( paramRefCount ) ;
	//
	for ( size_t i = 0; i < m_nRefCount; i ++ )
	{
		SString *	pstrID = m_aPropImageID.GetAt( i ) ;
		if ( pstrID == NULL )
		{
			pstrID = new SString ;
			m_aPropImageID.SetAt( i, pstrID ) ;
			pstrID->Format( L"ref_image%d", i ) ;
		}
		SString *	pstrName = m_aPropImageName.GetAt( i ) ;
		if ( pstrName == NULL )
		{
			pstrName = new SString ;
			m_aPropImageName.SetAt( i, pstrName ) ;
			pstrName->Format( L"参照画像[%d]", i + 1 ) ;
		}
		//
		AddParameterEntry
			( *pstrID, S3DSceneComposer::typeCommand,
				S3DSceneComposer::attrConstant
				| S3DSceneComposer::attrStringEnumeration,
				*pstrName, NULL ) ;
	}
}

// 参照画像収集
//////////////////////////////////////////////////////////////////////////////
SGLError S3DResourceAtlasTextureProc::CollectReferenceImages
	( S3DSceneComposer::ResourceAssets& assets,
		SSystem::SPointerArray<SGLImageBuffer>& aImageBufs,
		SSystem::SArray<SGLSize>& aImageSizes,
		SSystem::SPointerArray<SGLImageBuffer>& aSubImages )
{
	for ( size_t i = 0; i < m_nRefCount; i ++ )
	{
		SString *	pstrImageID = m_aRefImages.GetAt( i ) ;
		if ( (pstrImageID == NULL) || pstrImageID->IsEmpty() )
		{
			continue ;
		}
		S3DSceneComposer::ResourceContainer *
			prc = assets.GetResourceContainerAs( *pstrImageID ) ;
		if ( (prc != NULL) && prc->IsPendingProceduralRsrc() )
		{
			return	sglErrPending ;
		}
		ImageRsrcProcedure *	pImageProc =
			ESLTypeCast<ImageRsrcProcedure>( prc->GetProcedure() ) ;
		if ( pImageProc != NULL )
		{
			SPointerArray<SGLImageBuffer>	aTempSub ;
			pImageProc->CollectReferenceSubImages( assets, aTempSub ) ;
			//
			aSubImages.Merge( aSubImages.GetLength(), aTempSub ) ;
		}
		SGLImageObject *	pImage = assets.GetImageAs( *pstrImageID ) ;
		if ( pImage == NULL )
		{
			continue ;
		}
		SGLImageInfo	imginf ;
		pImage->GetImageInfo( imginf ) ;
		if ( (imginf.format != formatImageARGB)
			&& (imginf.format != formatImageABGR) )
		{
			pImage->NormalizeFormat( formatImageARGB, 32 ) ;
		}
		size_t	nFrames = pImage->GetFrameCount() ;
		size_t	iSelFrame = pImage->GetSelectedFrame() ;
		for ( size_t j = 0; j < nFrames; j ++ )
		{
			pImage->SelectFrame( j ) ;
			//
			SGLImageBuffer *	pImageBuf = pImage->GetImageBuffer() ;
			if ( pImageBuf != NULL )
			{
				SGLSize	size ;
				size.w = (int32_t) pImageBuf->width + 2 ;
				size.h = (int32_t) pImageBuf->height + 2 ;
				aImageSizes.Add( size ) ;
				aImageBufs.Add( pImageBuf ) ;
			}
		}
		pImage->SelectFrame( iSelFrame ) ;
	}
	return	sglErrSuccess ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
int32_t S3DResourceAtlasTextureProc::GetIntegerParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramWidth:
		return	m_sizeInit.w ;
	case	paramHeight:
		return	m_sizeInit.h ;
	case	paramRefCount:
		return	(int32_t) m_nRefCount ;
	}
	return	0 ;
}

bool S3DResourceAtlasTextureProc::GetBooleanParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramSizePOT:
		return	m_flagSizePOT ;
	case	paramClampBorder:
		return	m_flagClampBorder ;
	}
	return	false ;
}

const wchar_t * S3DResourceAtlasTextureProc::GetCommandParameter( size_t i ) const
{
	if ( i < paramRefTexture1 )
	{
		return	NULL ;
	}
	SString *	pstrImage = m_aRefImages.GetAt( i - paramRefTexture1 ) ;
	if ( pstrImage == NULL )
	{
		return	NULL ;
	}
	return	*pstrImage ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DResourceAtlasTextureProc::SetIntegerParameter( size_t i, int32_t n )
{
	switch ( i )
	{
	case	paramWidth:
		m_sizeInit.w = esl_clampi( n, 1, 16384 ) ;
		return ;
	case	paramHeight:
		m_sizeInit.h = esl_clampi( n, 1, 16384 ) ;
		return ;
	case	paramRefCount:
		m_nRefCount = (size_t) esl_clampi( n, 0, 128 ) ;
		UpdateParameterEntry() ;
		return ;
	}
}

void S3DResourceAtlasTextureProc::SetBooleanParameter( size_t i, bool b )
{
	switch ( i )
	{
	case	paramSizePOT:
		m_flagSizePOT = b ;
		return ;
	case	paramClampBorder:
		m_flagClampBorder = b ;
		return ;
	}
}

void S3DResourceAtlasTextureProc::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	if ( i < paramRefTexture1 )
	{
		return ;
	}
	SString *	pstrImage = m_aRefImages.GetAt( i - paramRefTexture1 ) ;
	if ( pstrImage == NULL )
	{
		pstrImage = new SString ;
		m_aRefImages.SetAt( i - paramRefTexture1, pstrImage ) ;
	}
	*pstrImage = pwszCmd ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DResourceAtlasTextureProc::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	if ( m_pComposer != NULL )
	{
		m_pComposer->Assets().EnumerateResourceIDsAs
			( aStrSet, ESL_RUNTIME_CLASS(SGLImageObject) ) ;
	}
	return	true ;
}

// S3DSceneComposer 関連付け
//////////////////////////////////////////////////////////////////////////////
void S3DResourceAtlasTextureProc::AttachSceneComposer( S3DSceneComposer * pComposer )
{
	m_pComposer = pComposer ;
}

// デシリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError S3DResourceAtlasTextureProc::ParseParameter( const SSystem::SXMLDocument& xmlProc )
{
	return	ParseParameterProperties( xmlProc ) ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError S3DResourceAtlasTextureProc::FormatParameter( SSystem::SXMLDocument& xmlProc )
{
	xmlProc.SetAttributeAs( L"proc_id", m_ItemClassDescriptor.pwszClassID ) ;
	xmlProc.RemoveAllElements() ;
	return	FormatParameterProperties( xmlProc ) ;
}

// リソース生成
//////////////////////////////////////////////////////////////////////////////
SGLError S3DResourceAtlasTextureProc::CreateResource
	( ESLObject*& pRsrc, S3DSceneComposer::ResourceAssets& assets )
{
	//
	// 画像収集
	//
	SPointerArray<SGLImageBuffer>	aImageBufs ;
	SPointerArray<SGLImageBuffer>	aSubImages ;
	SArray<SGLSize>					aImageSizes ;
	//
	SGLError	err =
		CollectReferenceImages
			( assets, aImageBufs, aImageSizes, aSubImages ) ;
	if ( err == sglErrPending )
	{
		return	err ;
	}
	//
	// 配置
	//
	const size_t	nImageCount = aImageBufs.GetLength() ;
	if ( nImageCount == 0 )
	{
		SGLImage *	pImage = new SGLImage ;
		pImage->CreateImage( 16, 16, formatImageARGB, 32 ) ;
		pRsrc = pImage ;
		return	sglErrSuccess ;
	}
	SPointerArray<SGLImageRect>	aTxtMapRects ;
	SGLImageRect **	ppTxtMapRects = aTxtMapRects.GetArray( nImageCount ) ;
	SGLSize *		pImageSizes = aImageSizes.GetArray() ;
	//
	SGLAreaAllocator	aalcTexture ;
	aalcTexture.SetInitialSize( m_sizeInit.w, m_sizeInit.h ) ;
	aalcTexture.SetFlags
		( m_flagSizePOT ? SGLAreaAllocator::flagSizePoweredBy2 : 0 ) ;
	aalcTexture.BatchAllocate( ppTxtMapRects, pImageSizes, nImageCount ) ;
	//
	aImageSizes.FinishArray() ;
	//
	// 結合先画像バッファ生成
	//
	const SGLSize	sizeAtlas = aalcTexture.GetTotalSize() ;
	SGLImage *	pAtlasImage = new SGLImage ;
	pAtlasImage->CreateImage( sizeAtlas.w, sizeAtlas.h, formatImageARGB, 32 ) ;
	pAtlasImage->NormalizeToTexture( 0 ) ;
	pRsrc = pAtlasImage ;
	//
	// 画像結合
	//
	SGLImageBuffer *	pAtlasBuf = pAtlasImage->GetImageBuffer() ;
	if ( pAtlasBuf == NULL )
	{
		return	sglErrSuccess ;
	}
	for ( size_t i = 0; i < nImageCount; i ++ )
	{
		ESLAssert( ppTxtMapRects[i] != NULL ) ;
		if ( ppTxtMapRects[i] == NULL )
		{
			continue ;
		}
		SGLImageBuffer *	pSrcImage = aImageBufs.GetAt( i ) ;
		if ( pSrcImage == NULL )
		{
			continue ;
		}
		SGLImageRect	rect = *(ppTxtMapRects[i]) ;
		//
		// 画像複製
		//
		sglConvertImageBuffer
			( *pAtlasBuf, *pSrcImage, rect.x + 1, rect.y + 1 ) ;
		//
		if ( m_flagClampBorder
			|| !(pSrcImage->format & formatImageFlagAlpha) )
		{
			SGLImageRect	rectSrc ;
			rectSrc.x = 0 ;
			rectSrc.y = 0 ;
			rectSrc.w = (int32_t) pSrcImage->width ;
			rectSrc.h = 1 ;
			sglConvertImageBuffer
				( *pAtlasBuf, *pSrcImage,
					rect.x + 1, rect.y, &rectSrc ) ;
			//
			rectSrc.y = (int32_t) pSrcImage->height - 1 ;
			sglConvertImageBuffer
				( *pAtlasBuf, *pSrcImage,
					rect.x + 1, rect.y + rect.h - 1, &rectSrc ) ;
			//
			rectSrc.x = 0 ;
			rectSrc.y = 0 ;
			rectSrc.w = 1 ;
			rectSrc.h = (int32_t) pSrcImage->height ;
			sglConvertImageBuffer
				( *pAtlasBuf, *pSrcImage, rect.x, rect.y + 1, &rectSrc ) ;
			//
			rectSrc.x = (int32_t) pSrcImage->width - 1 ;
			sglConvertImageBuffer
				( *pAtlasBuf, *pSrcImage,
					rect.x + rect.w - 1, rect.y + 1, &rectSrc ) ;
			//
			rectSrc.x = 0 ;
			rectSrc.y = 0 ;
			rectSrc.w = 1 ;
			rectSrc.h = 1 ;
			sglConvertImageBuffer
				( *pAtlasBuf, *pSrcImage,
					rect.x, rect.y, &rectSrc ) ;
			//
			rectSrc.x = (int32_t) pSrcImage->width - 1 ;
			sglConvertImageBuffer
				( *pAtlasBuf, *pSrcImage,
					rect.x + rect.w - 1, rect.y, &rectSrc ) ;
			//
			rectSrc.y = (int32_t) pSrcImage->height - 1 ;
			sglConvertImageBuffer
				( *pAtlasBuf, *pSrcImage,
					rect.x + rect.w - 1, rect.y + rect.h - 1, &rectSrc ) ;
			//
			rectSrc.x = 0 ;
			sglConvertImageBuffer
				( *pAtlasBuf, *pSrcImage,
					rect.x, rect.y + rect.h - 1, &rectSrc ) ;
		}
		//
		// バッファ参照
		//
		SGLImageRect	rectRef ;
		rectRef.x = rect.x + 1 ;
		rectRef.y = rect.y + 1 ;
		rectRef.w = rect.w - 2 ;
		rectRef.h = rect.h - 2 ;
		sglMakeReferenceImageBuffer( pSrcImage, pAtlasBuf, &rectRef ) ;
	}
	aTxtMapRects.FinishArray() ;
	//
	// 参照元の画像の参照を更新
	//
	S3DSceneComposer::ResourceAssets::
			UpdateImageReference( pAtlasBuf, aSubImages ) ;
	//
	return	sglErrSuccess ;
}

SGLError S3DResourceAtlasTextureProc::CollectReferenceSubImages
	( S3DSceneComposer::ResourceAssets& assets,
		SSystem::SPointerArray<SGLImageBuffer>& aSubImages )
{
	for ( size_t i = 0; i < m_nRefCount; i ++ )
	{
		SString *	pstrImageID = m_aRefImages.GetAt( i ) ;
		if ( (pstrImageID == NULL) || pstrImageID->IsEmpty() )
		{
			continue ;
		}
		SGLError	err =
			assets.CollectReferenceSubImage( aSubImages, *pstrImageID ) ;
		if ( err )
		{
			return	err ;
		}
	}
	return	sglErrSuccess ;
}



//////////////////////////////////////////////////////////////////////////////
// ３次元テクスチャ化
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::S3DRsrc3DTextureBuilderProc, ImageRsrcProcedure, Controller )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DRsrc3DTextureBuilderProc, texture3d_builder )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DRsrc3DTextureBuilderProc::S3DRsrc3DTextureBuilderProc( void )
	: Controller( m_ItemClassDescriptor.pwszClassID ),
		m_pComposer( nullptr ), m_flagTextureArray( false ),
		m_flagMipmap( true ), m_flagCompressed( true ), m_nRefCount( 1 )
{
	ESLVerify( paramTextureArray == AddParameterEntry
		( L"tex_array", S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrConstant,
			L"テクスチャアレイ", L"テクスチャアレイとして扱います。" ) ) ;
	ESLVerify( paramMakeMipmap == AddParameterEntry
		( L"mipmap", S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrConstant,
			L"ミップマップ", L"ミップマップテクスチャにします。" ) ) ;
	ESLVerify( paramCompressed == AddParameterEntry
		( L"compressed", S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrConstant,
			L"圧縮テクスチャ化", L"圧縮テクスチャにします。" ) ) ;
	ESLVerify( paramRefCount == AddParameterEntry
		( L"ref_count", S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrDynamicValidation,
			L"参照画像数", NULL ) ) ;
	//
	UpdateParameterEntry() ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DRsrc3DTextureBuilderProc::~S3DRsrc3DTextureBuilderProc( void )
{
}

// 参照画像プロパティエントリ設定
//////////////////////////////////////////////////////////////////////////////
void S3DRsrc3DTextureBuilderProc::UpdateParameterEntry( void )
{
	ChopParameterEntryLastAt( paramRefTexture1 - 1 ) ;
	//
	for ( size_t i = 0; i < m_nRefCount; i ++ )
	{
		SString *	pstrID = m_aPropImageID.GetAt( i ) ;
		if ( pstrID == NULL )
		{
			pstrID = new SString ;
			m_aPropImageID.SetAt( i, pstrID ) ;
			pstrID->Format( L"ref_image%d", i ) ;
		}
		SString *	pstrName = m_aPropImageName.GetAt( i ) ;
		if ( pstrName == NULL )
		{
			pstrName = new SString ;
			m_aPropImageName.SetAt( i, pstrName ) ;
			pstrName->Format( L"参照画像[%d]", i + 1 ) ;
		}
		//
		AddParameterEntry
			( *pstrID, S3DSceneComposer::typeCommand,
				S3DSceneComposer::attrConstant
				| S3DSceneComposer::attrStringEnumeration,
				*pstrName, NULL ) ;
	}
}

// 参照画像収集
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRsrc3DTextureBuilderProc::CollectReferenceImages
	( S3DSceneComposer::ResourceAssets& assets,
		SGLSize& sizeMaxImage,
		SSystem::SPointerArray<SGLImageBuffer>& aImageBufs,
		SSystem::SPointerArray<SGLImageBuffer>& aSubImages )
{
	sizeMaxImage.w = 0 ;
	sizeMaxImage.h = 0 ;
	for ( size_t i = 0; i < m_nRefCount; i ++ )
	{
		SString *	pstrImageID = m_aRefImages.GetAt( i ) ;
		if ( (pstrImageID == NULL) || pstrImageID->IsEmpty() )
		{
			continue ;
		}
		S3DSceneComposer::ResourceContainer *
			prc = assets.GetResourceContainerAs( *pstrImageID ) ;
		if ( (prc != NULL) && prc->IsPendingProceduralRsrc() )
		{
			return	sglErrPending ;
		}
		ImageRsrcProcedure *	pImageProc =
			ESLTypeCast<ImageRsrcProcedure>( prc->GetProcedure() ) ;
		if ( pImageProc != NULL )
		{
			SPointerArray<SGLImageBuffer>	aTempSub ;
			pImageProc->CollectReferenceSubImages( assets, aTempSub ) ;
			//
			aSubImages.Merge( aSubImages.GetLength(), aTempSub ) ;
		}
		SGLImageObject *	pImage = assets.GetImageAs( *pstrImageID ) ;
		if ( pImage == NULL )
		{
			continue ;
		}
		SGLImageInfo	imginf ;
		pImage->GetImageInfo( imginf ) ;
		if ( (imginf.format != formatImageARGB)
			&& (imginf.format != formatImageABGR) )
		{
			pImage->NormalizeFormat( formatImageARGB, 32 ) ;
		}
		size_t	nFrames = pImage->GetFrameCount() ;
		size_t	iSelFrame = pImage->GetSelectedFrame() ;
		for ( size_t j = 0; j < nFrames; j ++ )
		{
			pImage->SelectFrame( j ) ;
			//
			SGLImageBuffer *	pImageBuf = pImage->GetImageBuffer() ;
			if ( pImageBuf != NULL )
			{
				sizeMaxImage.w = (int32_t) esl_max( sizeMaxImage.w, pImageBuf->width ) ;
				sizeMaxImage.h = (int32_t) esl_max( sizeMaxImage.h, pImageBuf->height ) ;
				aImageBufs.Add( pImageBuf ) ;
			}
		}
		pImage->SelectFrame( iSelFrame ) ;
	}
	return	sglErrSuccess ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
int32_t S3DRsrc3DTextureBuilderProc::GetIntegerParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramRefCount:
		return	(int32_t) m_nRefCount ;
	}
	return	0 ;
}

bool S3DRsrc3DTextureBuilderProc::GetBooleanParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramTextureArray:
		return	m_flagTextureArray ;
	case	paramMakeMipmap:
		return	m_flagMipmap ;
	case	paramCompressed:
		return	m_flagCompressed ;
	}
	return	false ;
}

const wchar_t * S3DRsrc3DTextureBuilderProc::GetCommandParameter( size_t iParam ) const
{
	if ( iParam < paramRefTexture1 )
	{
		return	nullptr ;
	}
	SString *	pstrImage = m_aRefImages.GetAt( iParam - paramRefTexture1 ) ;
	if ( pstrImage == nullptr )
	{
		return	nullptr ;
	}
	return	*pstrImage ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DRsrc3DTextureBuilderProc::SetIntegerParameter( size_t iParam, int32_t n )
{
	switch ( iParam )
	{
	case	paramRefCount:
		m_nRefCount = (size_t) esl_clampi( n, 0, 128 ) ;
		UpdateParameterEntry() ;
		return ;
	}
}

void S3DRsrc3DTextureBuilderProc::SetBooleanParameter( size_t iParam, bool b )
{
	switch ( iParam )
	{
	case	paramTextureArray:
		m_flagTextureArray = b ;
		return ;
	case	paramMakeMipmap:
		m_flagMipmap = b ;
		return ;
	case	paramCompressed:
		m_flagCompressed = b ;
		return ;
	}
}

void S3DRsrc3DTextureBuilderProc::SetCommandParameter( size_t iParam, const wchar_t * pwszCmd )
{
	if ( iParam < paramRefTexture1 )
	{
		return ;
	}
	SString *	pstrImage = m_aRefImages.GetAt( iParam - paramRefTexture1 ) ;
	if ( pstrImage == nullptr )
	{
		pstrImage = new SString ;
		m_aRefImages.SetAt( iParam - paramRefTexture1, pstrImage ) ;
	}
	*pstrImage = pwszCmd ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DRsrc3DTextureBuilderProc::EnumerateStringSet
	( size_t iParam, SSystem::SStringArray& aStrSet )
{
	if ( m_pComposer != nullptr )
	{
		m_pComposer->Assets().EnumerateResourceIDsAs
			( aStrSet, ESL_RUNTIME_CLASS(SGLImageObject) ) ;
	}
	return	true ;
}

// S3DSceneComposer 関連付け
//////////////////////////////////////////////////////////////////////////////
void S3DRsrc3DTextureBuilderProc::AttachSceneComposer( S3DSceneComposer * pComposer )
{
	m_pComposer = pComposer ;
}

// デシリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRsrc3DTextureBuilderProc::ParseParameter( const SSystem::SXMLDocument& xmlProc )
{
	return	ParseParameterProperties( xmlProc ) ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRsrc3DTextureBuilderProc::FormatParameter( SSystem::SXMLDocument& xmlProc )
{
	xmlProc.SetAttributeAs( L"proc_id", m_ItemClassDescriptor.pwszClassID ) ;
	xmlProc.RemoveAllElements() ;
	return	FormatParameterProperties( xmlProc ) ;
}

// リソース生成
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRsrc3DTextureBuilderProc::CreateResource
	( ESLObject*& pRsrc, S3DSceneComposer::ResourceAssets& assets )
{
	//
	// 画像収集
	//
	SPointerArray<SGLImageBuffer>	aImageBufs ;
	SPointerArray<SGLImageBuffer>	aSubImages ;
	SGLSize							sizeMaxImage ;
	//
	SGLError	err =
		CollectReferenceImages
			( assets, sizeMaxImage, aImageBufs, aSubImages ) ;
	if ( err == sglErrPending )
	{
		return	err ;
	}
	//
	// 画像バッファ生成
	//
	const size_t	nImageCount = aImageBufs.GetLength() ;
	if ( nImageCount == 0 )
	{
		SGLImage *	pImage = new SGLImage ;
		pImage->CreateImage( 16, 16, formatImageARGB, 32 ) ;
		pRsrc = pImage ;
		return	sglErrSuccess ;
	}
	uint64_t nFlags = SGLImageObject::bufferOnMemory
						| SGLImageObject::bufferSampleTiling ;
	if ( m_flagTextureArray )
	{
		nFlags |= SGLImageObject::bufferTextureArray ;
	}
	else
	{
		nFlags |= SGLImageObject::bufferTexture3D ;
	}
	if ( m_flagMipmap )
	{
		nFlags |= SGLImageObject::bufferForMipmapTexture ;
	}
	if ( m_flagCompressed )
	{
		nFlags |= SGLImageObject::bufferCompressedTexture ;
	}
	SGLImage *	pBuild3DImage = new SGLImage ;
	pBuild3DImage->CreateImage
		( sizeMaxImage.w, sizeMaxImage.h,
			formatImageARGB, 32, nFlags, nImageCount, 1000 ) ;
	pRsrc = pBuild3DImage ;
	//
	// 画像結合
	//
	const SGLImageRect	irFrame( 0, 0, sizeMaxImage.w, sizeMaxImage.h ) ;
	for ( size_t i = 0; i < nImageCount; i ++ )
	{
		SGLImageBuffer *	pSrcImage = aImageBufs.GetAt( i ) ;
		if ( pSrcImage == NULL )
		{
			continue ;
		}
		//
		// 画像複製
		//
		pBuild3DImage->SelectFrame( i ) ;
		//
		SGLImageBuffer *	pBuildBuf = pBuild3DImage->GetImageBuffer() ;
		ESLAssert( pBuildBuf != nullptr ) ;
		if ( pBuildBuf != nullptr )
		{
			sglConvertImageBuffer( *pBuildBuf, *pSrcImage, 0, 0 ) ;
			//
			// バッファ参照
			//
//			sglMakeReferenceImageBuffer( pSrcImage, pBuildBuf, &irFrame, (int) i ) ;
		}
	}
	//
	// 参照元の画像の参照を更新
	//
	pBuild3DImage->SelectFrame( 0 ) ;
	//
/*	SGLImageBuffer *	pBuildBuf = pBuild3DImage->GetImageBuffer() ;
	if ( pBuildBuf != nullptr )
	{
		while ( pBuildBuf->ptrRefOriginal != nullptr )
		{
			pBuildBuf = pBuildBuf->ptrRefOriginal ;
		}
		S3DSceneComposer::ResourceAssets::UpdateImageReference( pBuildBuf, aSubImages ) ;
	}
*/	//
	return	sglErrSuccess ;
}

// 参照元画像一覧
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRsrc3DTextureBuilderProc::CollectReferenceSubImages
	( S3DSceneComposer::ResourceAssets& assets,
		SSystem::SPointerArray<SGLImageBuffer>& aSubImages )
{
/*
	for ( size_t i = 0; i < m_nRefCount; i ++ )
	{
		SString *	pstrImageID = m_aRefImages.GetAt( i ) ;
		if ( (pstrImageID == NULL) || pstrImageID->IsEmpty() )
		{
			continue ;
		}
		SGLError	err =
			assets.CollectReferenceSubImage( aSubImages, *pstrImageID ) ;
		if ( err )
		{
			return	err ;
		}
	}
*/
	return	sglErrSuccess ;
}



//////////////////////////////////////////////////////////////////////////////
// 反復画像
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::S3DRsrcImageRepeaterProc, ImageRsrcProcedure, Controller )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DRsrcImageRepeaterProc, image_rep )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DRsrcImageRepeaterProc::S3DRsrcImageRepeaterProc( void )
	: Controller( m_ItemClassDescriptor.pwszClassID ),
		m_pComposer( NULL ),
		m_sizeRepeat( 2, 2 )
{
	AddParameterEntry
		( L"ref_image", S3DSceneComposer::typeCommand,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrStringEnumeration,
			L"元画像", NULL ) ;
	AddParameterEntry
		( L"x_repeat", S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrConstant,
			L"ｘ反復数", NULL ) ;
	AddParameterEntry
		( L"y_repeat", S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrConstant,
			L"ｙ反復数", NULL ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DRsrcImageRepeaterProc::~S3DRsrcImageRepeaterProc( void )
{
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
int32_t S3DRsrcImageRepeaterProc::GetIntegerParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramXRepeat:
		return	m_sizeRepeat.w ;
	case	paramYRepeat:
		return	m_sizeRepeat.h ;
	}
	return	0 ;
}

const wchar_t * S3DRsrcImageRepeaterProc::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramImageID:
		return	m_strImageID ;
	}
	return	NULL ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DRsrcImageRepeaterProc::SetIntegerParameter( size_t i, int32_t n )
{
	switch ( i )
	{
	case	paramXRepeat:
		m_sizeRepeat.w = esl_clampi( n, 1, 32 ) ;
		return ;
	case	paramYRepeat:
		m_sizeRepeat.h = esl_clampi( n, 1, 32 ) ;
		return ;
	}
}

void S3DRsrcImageRepeaterProc::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	switch ( i )
	{
	case	paramImageID:
		m_strImageID = pwszCmd ;
		return ;
	}
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DRsrcImageRepeaterProc::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	switch ( i )
	{
	case	paramImageID:
		m_pComposer->Assets().EnumerateResourceIDsAs
			( aStrSet, ESL_RUNTIME_CLASS(SGLImageObject) ) ;
		return	true ;
	}
	return	false ;
}

// S3DSceneComposer 関連付け
//////////////////////////////////////////////////////////////////////////////
void S3DRsrcImageRepeaterProc::AttachSceneComposer( S3DSceneComposer * pComposer )
{
	m_pComposer = pComposer ;
}

// デシリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRsrcImageRepeaterProc::ParseParameter( const SSystem::SXMLDocument& xmlProc )
{
	return	ParseParameterProperties( xmlProc ) ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRsrcImageRepeaterProc::FormatParameter( SSystem::SXMLDocument& xmlProc )
{
	xmlProc.SetAttributeAs( L"proc_id", m_ItemClassDescriptor.pwszClassID ) ;
	xmlProc.RemoveAllElements() ;
	return	FormatParameterProperties( xmlProc ) ;
}

// リソース生成
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRsrcImageRepeaterProc::CreateResource
	( ESLObject*& pRsrc, S3DSceneComposer::ResourceAssets& assets )
{
	S3DSceneComposer::ResourceContainer *
		prc = assets.GetResourceContainerAs( m_strImageID ) ;
	if ( (prc != NULL) && prc->IsPendingProceduralRsrc() )
	{
		return	sglErrPending ;
	}
	SGLImageObject *	pSrcImage = assets.GetImageAs( m_strImageID ) ;
	if ( pSrcImage == NULL )
	{
		return	sglErrFailed ;
	}
	//
	// 画像バッファ生成
	//
	SGLImageInfo	imginf ;
	pSrcImage->GetImageInfo( imginf ) ;
	//
	SGLImage *	pImage = new SGLImage ;
	pImage->CreateImage
		( imginf.width * m_sizeRepeat.w,
			imginf.height * m_sizeRepeat.h,
			imginf.format, imginf.depth ) ;
	//
	// 反復処理
	//
	for ( int y = 0; y < m_sizeRepeat.h; y ++ )
	{
		for ( int x = 0; x < m_sizeRepeat.w; x ++ )
		{
			pImage->CopyImage
				( pSrcImage, x * imginf.width, y * imginf.height ) ;
		}
	}
	//
	// 参照設定
	//
	SGLImageBuffer *	pDstBuf = pImage->GetImageBuffer() ;
	SGLImageBuffer *	pSrcBuf = pSrcImage->GetImageBuffer() ;
	if ( pDstBuf && pSrcBuf )
	{
		SGLImageRect	rectRef ;
		rectRef.x = 0 ;
		rectRef.y = 0 ;
		rectRef.w = (int32_t) pSrcBuf->width ;
		rectRef.h = (int32_t) pSrcBuf->height ;
		sglMakeReferenceImageBuffer( pSrcBuf, pDstBuf, &rectRef ) ;
		//
		SPointerArray<SGLImageBuffer>	aSubImages ;
		assets.CollectReferenceSubImage( aSubImages, m_strImageID ) ;
		//
		S3DSceneComposer::ResourceAssets::
				UpdateImageReference( pDstBuf, aSubImages ) ;
	}
	//
	pRsrc = pImage ;
	return	sglErrSuccess ;
}

// 参照元画像一覧
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRsrcImageRepeaterProc::CollectReferenceSubImages
	( S3DSceneComposer::ResourceAssets& assets,
		SSystem::SPointerArray<SGLImageBuffer>& aSubImages )
{
	return	assets.CollectReferenceSubImage( aSubImages, m_strImageID ) ;
}



//////////////////////////////////////////////////////////////////////////////
// パーリンノイズ画像
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::S3DRsrcImagePerlinProc, ImageRsrcProcedure, Controller )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DRsrcImagePerlinProc, perlin_image )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DRsrcImagePerlinProc::S3DRsrcImagePerlinProc( void )
	: Controller( m_ItemClassDescriptor.pwszClassID ),
		m_pComposer( NULL ), m_sizeImage( 256, 256 ), m_nDepth( 1 )
{
	m_genParam.nRandomSeed = 1 ;
	m_genParam.fpLowCutOff = 0.0 ;
	m_genParam.fpHighSaturation = 1.0 ;
	m_genParam.argbGradation[0] = 0 ;
	m_genParam.argbGradation[1] = 0x555555 ;
	m_genParam.argbGradation[2] = 0xAAAAAA ;
	m_genParam.argbGradation[3] = 0xFFFFFF ;
	m_genParam.fpGradationPos[0] = 0.3333333f ;
	m_genParam.fpGradationPos[1] = 0.6666666f ;
	m_fpAlphaFill[0] = 1.0 ;
	m_fpAlphaFill[1] = 1.0 ;
	//
	AddParameterEntry
		( L"width", S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrConstant,
			L"幅", NULL ) ;
	AddParameterEntry
		( L"height", S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrConstant,
			L"高さ", NULL ) ;
	AddParameterEntry
		( L"depth", S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrConstant,
			L"奥行き", NULL ) ;
	AddParameterEntry
		( L"random_seek", S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrConstant,
			L"乱数種", NULL ) ;
	AddParameterEntry
		( L"low_level", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrUIScalarSlider,
			L"カットオフ値", NULL, 0.0, 1.0 ) ;
	AddParameterEntry
		( L"high_level", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrUIScalarSlider,
			L"飽和値", NULL, 0.0, 1.0 ) ;
	AddParameterEntry
		( L"color_map0", S3DSceneComposer::typeColor,
			S3DSceneComposer::attrConstant,
			L"色[0]", NULL ) ;
	AddParameterEntry
		( L"alpha_fill0", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrUIScalarSlider,
			L"α値[0]", NULL, 0.0, 1.0 ) ;
	AddParameterEntry
		( L"color_map1", S3DSceneComposer::typeColor,
			S3DSceneComposer::attrConstant,
			L"色[1]", NULL ) ;
	AddParameterEntry
		( L"color_pos1", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrUIScalarSlider,
			L"色[1]補完レベル", NULL, 0.0, 1.0 ) ;
	AddParameterEntry
		( L"color_map2", S3DSceneComposer::typeColor,
			S3DSceneComposer::attrConstant,
			L"色[2]", NULL ) ;
	AddParameterEntry
		( L"color_pos2", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrUIScalarSlider,
			L"色[2]補完レベル", NULL, 0.0, 1.0 ) ;
	AddParameterEntry
		( L"color_map3", S3DSceneComposer::typeColor,
			S3DSceneComposer::attrConstant,
			L"色[3]", NULL ) ;
	AddParameterEntry
		( L"alpha_fill3", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrUIScalarSlider,
			L"α値[3]", NULL, 0.0, 1.0 ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DRsrcImagePerlinProc::~S3DRsrcImagePerlinProc( void )
{
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDVector S3DRsrcImagePerlinProc::GetVectorParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramColorMap0:
		return	VectorFromColor( m_genParam.argbGradation[0] ) ;
	case	paramColorMap1:
		return	VectorFromColor( m_genParam.argbGradation[1] ) ;
	case	paramColorMap2:
		return	VectorFromColor( m_genParam.argbGradation[2] ) ;
	case	paramColorMap3:
		return	VectorFromColor( m_genParam.argbGradation[3] ) ;
	}
	return	S3DDVector( 0, 0, 0 ) ;
}

double S3DRsrcImagePerlinProc::GetScalarParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramLowLevel:
		return	m_genParam.fpLowCutOff ;
	case	paramHighLevel:
		return	m_genParam.fpHighSaturation ;
	case	paramAlphaFill0:
		return	m_fpAlphaFill[0] ;
	case	paramColorPos1:
		return	m_genParam.fpGradationPos[0] ;
	case	paramColorPos2:
		return	m_genParam.fpGradationPos[1] ;
	case	paramAlphaFill3:
		return	m_fpAlphaFill[1] ;
	}
	return	0.0 ;
}

int32_t S3DRsrcImagePerlinProc::GetIntegerParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramWidth:
		return	m_sizeImage.w ;
	case	paramHeight:
		return	m_sizeImage.h ;
	case	paramDepth:
		return	m_nDepth ;
	case	paramRandomSeed:
		return	(int32_t) m_genParam.nRandomSeed ;
	}
	return	0 ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DRsrcImagePerlinProc::SetVectorParameter( size_t i, const S3DDVector& vec )
{
	switch ( i )
	{
	case	paramColorMap0:
		m_genParam.argbGradation[0] = ColorFromVector( vec ) ;
		return ;
	case	paramColorMap1:
		m_genParam.argbGradation[1] = ColorFromVector( vec ) ;
		return ;
	case	paramColorMap2:
		m_genParam.argbGradation[2] = ColorFromVector( vec ) ;
		return ;
	case	paramColorMap3:
		m_genParam.argbGradation[3] = ColorFromVector( vec ) ;
		return ;
	}
}

void S3DRsrcImagePerlinProc::SetScalarParameter( size_t i, double s )
{
	switch ( i )
	{
	case	paramLowLevel:
		m_genParam.fpLowCutOff = s ;
		return ;
	case	paramHighLevel:
		m_genParam.fpHighSaturation = s ;
		return ;
	case	paramAlphaFill0:
		m_fpAlphaFill[0] = s ;
		return ;
	case	paramColorPos1:
		m_genParam.fpGradationPos[0] = (float32_t) s ;
		return ;
	case	paramColorPos2:
		m_genParam.fpGradationPos[1] = (float32_t) s ;
		return ;
	case	paramAlphaFill3:
		m_fpAlphaFill[1] = s ;
		return ;
	}
}

void S3DRsrcImagePerlinProc::SetIntegerParameter( size_t i, int32_t n )
{
	switch ( i )
	{
	case	paramWidth:
		m_sizeImage.w = esl_clampi( n, 1, 0x4000 ) ;
		return ;
	case	paramHeight:
		m_sizeImage.h = esl_clampi( n, 1, 0x4000 ) ;
		return ;
	case	paramDepth:
		m_nDepth = esl_clampi( n, 1, 0x1000 ) ;
		return ;
	case	paramRandomSeed:
		m_genParam.nRandomSeed = (uint32_t) n ;
		return ;
	}
}

// S3DSceneComposer 関連付け
//////////////////////////////////////////////////////////////////////////////
void S3DRsrcImagePerlinProc::AttachSceneComposer( S3DSceneComposer * pComposer )
{
	m_pComposer = pComposer ;
}

// デシリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRsrcImagePerlinProc::ParseParameter( const SSystem::SXMLDocument& xmlProc )
{
	return	ParseParameterProperties( xmlProc ) ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRsrcImagePerlinProc::FormatParameter( SSystem::SXMLDocument& xmlProc )
{
	xmlProc.SetAttributeAs( L"proc_id", m_ItemClassDescriptor.pwszClassID ) ;
	xmlProc.RemoveAllElements() ;
	return	FormatParameterProperties( xmlProc ) ;
}

// リソース生成
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRsrcImagePerlinProc::CreateResource
	( ESLObject*& pRsrc, S3DSceneComposer::ResourceAssets& assets )
{
	SGLImage *	pImage = new SGLImage ;
	uint32_t	nFlags = SGLImageObject::bufferOnMemory
							| SGLImageObject::bufferSampleTiling ;
	if ( m_nDepth > 1 )
	{
		nFlags |= SGLImageObject::bufferTexture3D ;
	}
	pImage->CreateImage
		( (uint32_t) m_sizeImage.w, (uint32_t) m_sizeImage.h,
			formatImageARGB, 32, nFlags, (size_t) m_nDepth, m_nDepth * 33 ) ;
	//
	double	a0 = m_fpAlphaFill[0] * 255.0 ;
	double	a1 = m_fpAlphaFill[1] * 255.0 ;
	double	ad = a1 -a0 ;
	m_genParam.argbGradation[0].argb.Alpha =
		(uint8_t) esl_clampi( (int) esl_lroundfi(a0), 0, 0xFF ) ;
	m_genParam.argbGradation[1].argb.Alpha =
		(uint8_t) esl_clampi( (int) esl_lroundfi(a0+ad/3.0), 0, 0xFF ) ;
	m_genParam.argbGradation[2].argb.Alpha =
		(uint8_t) esl_clampi( (int) esl_lroundfi(a1-ad/3.0), 0, 0xFF ) ;
	m_genParam.argbGradation[3].argb.Alpha =
		(uint8_t) esl_clampi( (int) esl_lroundfi(a1), 0, 0xFF ) ;
	//
	GeneratePerlin( *pImage, m_genParam ) ;
	//
//	pImage->NormalizeToTexture( SGLImageObject::bufferSampleTiling ) ;
	pRsrc = pImage ;
	return	sglErrSuccess ;
}

// 参照元画像一覧
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRsrcImagePerlinProc::CollectReferenceSubImages
	( S3DSceneComposer::ResourceAssets& assets,
		SSystem::SPointerArray<SGLImageBuffer>& aSubImages )
{
	return	sglErrSuccess ;
}

// 画像生成
//////////////////////////////////////////////////////////////////////////////
void S3DRsrcImagePerlinProc::GeneratePerlin
	( SGLImageObject& image, const GenerateParameter& gp )
{
	//
	// 乱数テーブル生成
	//
	SCLRandomizer		random( gp.nRandomSeed ) ;
	SArray<uint32_t>	aRandomTable ;
	uint32_t *			pRandomTable = aRandomTable.GetArray( 0x200 ) ;
	size_t				i ;
	//
	for ( i = 0; i < 0x100; i ++ )
	{
		pRandomTable[i] = random.QuickRandomize( 0x100 ) ;
		pRandomTable[i + 0x100] = pRandomTable[i] ;
	}
	//
	// 生成
	//
	PerlinParam	pp ;
	pp.pRandomTable = pRandomTable ;
	//
	size_t	nFrames = image.GetFrameCount() ;
	pp.zRepeat = (int) nFrames ;
	for ( i = 0; i < nFrames; i ++ )
	{
		image.SelectFrame( i ) ;
		//
		SGLImageInfo	imginf ;
		uint8_t *	pbytImage =
				image.LockBuffer( imginf, SGLImageObject::lockWrite ) ;
		pp.xRepeat = (int) imginf.width ;
		pp.yRepeat = (int) imginf.height ;
		//
		const float32_t	fpLowCutOff = (float32_t) gp.fpLowCutOff ;
		const float32_t	fpLevelRange =
				(float32_t) (1.0 / (gp.fpHighSaturation - gp.fpLowCutOff)) ;
		for ( int y = 0; (uint32_t) y < imginf.height; y ++ )
		{
			uint8_t *	pbytLine = pbytImage + (imginf.pitchLine * y) ;
			SGLPalette	rgba ;
			for ( int x = 0; (uint32_t) x < imginf.width; x ++ )
			{
				float32_t	v = OctavePerlin
								( pp, (float32_t) x,
										(float32_t) y,
										(float32_t) i ) ;
				//
				v = (Fade( v ) - fpLowCutOff) * fpLevelRange ;
				//
				if ( v < gp.fpGradationPos[0] )
				{
					double	t = esl_fclamp( v / gp.fpGradationPos[0], 0.0, 1.0 ) ;
					rgba = gp.argbGradation[0] * (1.0 - t)
							+ gp.argbGradation[1] * t ;
				}
				else if ( v < gp.fpGradationPos[1] )
				{
					v = (v - gp.fpGradationPos[0])
							/ (gp.fpGradationPos[1] - gp.fpGradationPos[0]) ;
					double	t = esl_fclamp( v, 0.0, 1.0 ) ;
					rgba = gp.argbGradation[1] * (1.0 - t)
							+ gp.argbGradation[2] * t ;
				}
				else
				{
					v = (v - gp.fpGradationPos[1])
							/ (1.0f - gp.fpGradationPos[1]) ;
					double	t = esl_fclamp( v, 0.0, 1.0 ) ;
					rgba = gp.argbGradation[2] * (1.0 - t)
							+ gp.argbGradation[3] * t ;
				}
				//
				*((SGLPalette*)pbytLine) = rgba ;
				//
				pbytLine += imginf.pitchPixel ;
			}
		}
		//
		image.UnlockBuffer( SGLImageObject::lockWrite ) ;
	}
	aRandomTable.FinishArray() ;
}

// パーリン関数
//////////////////////////////////////////////////////////////////////////////
float32_t S3DRsrcImagePerlinProc::OctavePerlin
	( const S3DRsrcImagePerlinProc::PerlinParam& pp,
						float32_t x, float32_t y, float32_t z )
{
	PerlinParam	ppTemp ;
	float32_t	v = 0.0f ;
	int			wRepeat = 64 ;
	float32_t	scale = 1.0f / 64.0f ;
	float32_t	amp = 0.5f ;
	float32_t	normal = 0.0f ;
	for ( int i = 0; i < 6; i ++ )
	{
		ppTemp.xRepeat = esl_max( pp.xRepeat / wRepeat, 1 ) ;
		ppTemp.yRepeat = esl_max( pp.yRepeat / wRepeat, 1 ) ;
		ppTemp.zRepeat = esl_max( pp.zRepeat / wRepeat, 1 ) ;
		ppTemp.pRandomTable = pp.pRandomTable ;
		//
		v += Perlin
			( ppTemp, (x + 0.5f) * scale,
						(y + 0.5f) * scale,
						(z + 0.5f) * scale ) * amp ;
		normal += amp ;
		//
		wRepeat >>= 1 ;
		scale *= 2.0f ;
		amp *= 0.5f ;
	}
	return	v / normal ;
}

float32_t S3DRsrcImagePerlinProc::Perlin
	( const S3DRsrcImagePerlinProc::PerlinParam& pp,
						float32_t x, float32_t y, float32_t z )
{
	uint32_t *	p = pp.pRandomTable ;
	const int	ix = (int) x ;
	const int	iy = (int) y ;
	const int	iz = (int) z ;
	const int	mx = ix & 0xFF ;
	const int	my = iy & 0xFF ;
	const int	mz = iz & 0xFF ;
	const int	nx = ((mx+1) % pp.xRepeat) & 0xFF ;
	const int	ny = ((my+1) % pp.yRepeat) & 0xFF ;
	const int	nz = ((mz+1) % pp.zRepeat) & 0xFF ;
	const float	dx = x - ix ;
	const float	dy = y - iy ;
	const float	dz = z - iz ;
	//
	const float	u = Fade( dx ) ;
	const float	v = Fade( dy ) ;
	const float	w = Fade( dz ) ;
	//
	const uint32_t	aaa = p[p[p[mx]+my]+mz] ;
	const uint32_t	aba = p[p[p[mx]+ny]+mz] ;
	const uint32_t	aab = p[p[p[mx]+my]+nz] ;
	const uint32_t	abb = p[p[p[mx]+ny]+nz] ;
	const uint32_t	baa = p[p[p[nx]+my]+mz] ;
	const uint32_t	bba = p[p[p[nx]+ny]+mz] ;
	const uint32_t	bab = p[p[p[nx]+my]+nz] ;
	const uint32_t	bbb = p[p[p[nx]+ny]+nz] ;
	//
	const float	x11 = Lerp( Grad(aaa, dx, dy, dz),
							Grad(baa, dx-1, dy, dz), u ) ;
	const float	x12 = Lerp( Grad(aba, dx, dy-1, dz),
							Grad(bba, dx-1, dy-1, dz), u ) ;
	const float	y1 = Lerp( x11, x12, v ) ;
	const float	x21 = Lerp( Grad(aab, dx, dy, dz-1),
							Grad(bab, dx-1, dy, dz-1), u ) ;
	const float	x22 = Lerp( Grad(abb, dx, dy-1, dz-1),
							Grad(bbb, dx-1, dy-1, dz-1), u ) ;
	const float	y2 = Lerp( x21, x22, v ) ;
	return	Lerp( y1, y2, w ) * 0.5f + 0.5f ;
}

float32_t S3DRsrcImagePerlinProc::Grad
	( uint32_t hash, float32_t x, float32_t y, float32_t z )
{
	switch ( hash & 0x0F )
	{
	case 0x0:
		return	x + y ;
	case 0x1:
		return	-x + y ;
	case 0x2:
		return	x - y ;
	case 0x3:
		return	-x - y ;
	case 0x4:
		return	x + z ;
	case 0x5:
		return	-x + z ;
	case 0x6:
		return	x - z ;
	case 0x7:
		return	-x - z ;
	case 0x8:
		return	y + z ;
	case 0x9:
		return	-y + z ;
	case 0xA:
		return	y - z ;
	case 0xB:
		return	-y - z ;
	case 0xC:
		return	y + x ;
	case 0xD:
		return	-y + z ;
	case 0xE:
		return	y - x ;
	case 0xF:
		return	-y - z ;
	default:
		break ;
	}
	return	0 ;
}

float32_t S3DRsrcImagePerlinProc::Fade( float32_t x )
{
//	return	x ;
	return	x * x * x * (x * (x * 6.0f - 15.0f) + 10.0f) ;
}

float32_t S3DRsrcImagePerlinProc::Lerp
	( float32_t x, float32_t y, float32_t t )
{
	return	x + (y -x) * t ;
}



//////////////////////////////////////////////////////////////////////////////
// 法線画像の符号反転（チャネル毎の輝度反転）
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::S3DRsrcImageNormalInverseProc, ImageRsrcProcedure, Controller )
S3D_IMPLEMENT_COMPOSER_ITEM( S3DRsrcImageNormalInverseProc, image_inverser )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DRsrcImageNormalInverseProc::S3DRsrcImageNormalInverseProc( void )
	: Controller( m_ItemClassDescriptor.pwszClassID ),
		m_pComposer( nullptr ),
			m_flagMipmap( false ), m_flagCompressed( false )
{
	for ( int i = 0; i < countChannel; i ++ )
	{
		m_flagInverse[i] = false ;
	}
	m_flagInverse[1] = true ;

	ESLVerify( AddParameterEntry
		( L"ref_image", S3DSceneComposer::typeCommand,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrStringEnumeration,
			L"元画像", NULL ) == paramImageID ) ;
	ESLVerify( paramMakeMipmap == AddParameterEntry
		( L"mipmap", S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrConstant,
			L"ミップマップ", L"ミップマップテクスチャにします。" ) ) ;
	ESLVerify( paramCompressed == AddParameterEntry
		( L"compressed", S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrConstant,
			L"圧縮テクスチャ化", L"圧縮テクスチャにします。" ) ) ;
	ESLVerify( paramChannel0 == AddParameterEntry
		( L"channel0", S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrConstant,
			L"x 要素反転", L"画像のチャネル0を反転する" ) ) ;
	ESLVerify( paramChannel1 == AddParameterEntry
		( L"channel1", S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrConstant,
			L"y 要素反転", L"画像のチャネル1を反転する" ) ) ;
	ESLVerify( paramChannel2 == AddParameterEntry
		( L"channel2", S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrConstant,
			L"z 要素反転", L"画像のチャネル2を反転する" ) ) ;
	ESLVerify( paramChannel3 == AddParameterEntry
		( L"channel3", S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrConstant,
			L"w 要素反転", L"画像のチャネル3を反転する" ) ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DRsrcImageNormalInverseProc::~S3DRsrcImageNormalInverseProc( void )
{
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
bool S3DRsrcImageNormalInverseProc::GetBooleanParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramMakeMipmap:
		return	m_flagMipmap ;
	case	paramCompressed:
		return	m_flagCompressed ;
	case	paramChannel0:
		return	m_flagInverse[0] ;
	case	paramChannel1:
		return	m_flagInverse[1] ;
	case	paramChannel2:
		return	m_flagInverse[2] ;
	case	paramChannel3:
		return	m_flagInverse[3] ;
	}
	return	false ;
}

const wchar_t * S3DRsrcImageNormalInverseProc::GetCommandParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramImageID:
		return	m_strImageID ;
	}
	return	nullptr ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DRsrcImageNormalInverseProc::SetBooleanParameter( size_t iParam, bool b )
{
	switch ( iParam )
	{
	case	paramMakeMipmap:
		m_flagMipmap = b ;
		return ;
	case	paramCompressed:
		m_flagCompressed = b ;
		return ;
	case	paramChannel0:
		m_flagInverse[0] = b ;
		return ;
	case	paramChannel1:
		m_flagInverse[1] = b ;
		return ;
	case	paramChannel2:
		m_flagInverse[2] = b ;
		return ;
	case	paramChannel3:
		m_flagInverse[3] = b ;
		return ;
	}
}

void S3DRsrcImageNormalInverseProc::SetCommandParameter( size_t iParam, const wchar_t * pwszCmd )
{
	switch ( iParam )
	{
	case	paramImageID:
		m_strImageID = pwszCmd ;
		return ;
	}
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DRsrcImageNormalInverseProc::EnumerateStringSet
	( size_t iParam, SSystem::SStringArray& aStrSet )
{
	switch ( iParam )
	{
	case	paramImageID:
		m_pComposer->Assets().EnumerateResourceIDsAs
			( aStrSet, ESL_RUNTIME_CLASS(SGLImageObject) ) ;
		return	true ;
	}
	return	false ;
}

// S3DSceneComposer 関連付け
//////////////////////////////////////////////////////////////////////////////
void S3DRsrcImageNormalInverseProc::AttachSceneComposer( S3DSceneComposer * pComposer )
{
	m_pComposer = pComposer ;
}

// デシリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRsrcImageNormalInverseProc::ParseParameter( const SSystem::SXMLDocument& xmlProc )
{
	return	ParseParameterProperties( xmlProc ) ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRsrcImageNormalInverseProc::FormatParameter( SSystem::SXMLDocument& xmlProc )
{
	xmlProc.SetAttributeAs( L"proc_id", m_ItemClassDescriptor.pwszClassID ) ;
	xmlProc.RemoveAllElements() ;
	return	FormatParameterProperties( xmlProc ) ;
}

// リソース生成
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRsrcImageNormalInverseProc::CreateResource
	( ESLObject*& pRsrc, S3DSceneComposer::ResourceAssets& assets )
{
	S3DSceneComposer::ResourceContainer *
		prc = assets.GetResourceContainerAs( m_strImageID ) ;
	if ( (prc != nullptr) && prc->IsPendingProceduralRsrc() )
	{
		return	sglErrPending ;
	}
	SGLImageObject *	pSrcImage = assets.GetImageAs( m_strImageID ) ;
	if ( pSrcImage == nullptr )
	{
		return	sglErrFailed ;
	}
	SGLSize		sizeSrc = pSrcImage->GetImageSize() ;
	SGLImage *	pInversed = new SGLImage ;
	pInversed->CreateImage( sizeSrc.w, sizeSrc.h, formatImageARGB, 32 ) ;
	pInversed->ConvertImage( pSrcImage ) ;
	//
	uint8_t	tone[4][0x100] ;
	for ( int i = 0; i < 4; i ++ )
	{
		SGLImageObject::MakeOffsetMultipleTone
				( tone[i], (m_flagInverse[i] ? -1.0f : 1.0f) ) ;
	}
	pInversed->ApplyToneFilter( tone[0], tone[1], tone[2], tone[3] ) ;
	//
	uint32_t nFlags = SGLImageObject::bufferSampleTiling ;
	if ( m_flagMipmap )
	{
		nFlags |= SGLImageObject::bufferForMipmapTexture ;
	}
	if ( m_flagCompressed )
	{
		nFlags |= SGLImageObject::bufferCompressedTexture ;
	}
	pInversed->NormalizeToTexture( nFlags ) ;
	//
	pRsrc = pInversed ;
	return	sglErrSuccess ;
}

// 参照元画像一覧
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRsrcImageNormalInverseProc::CollectReferenceSubImages
	( S3DSceneComposer::ResourceAssets& assets,
		SSystem::SPointerArray<SGLImageBuffer>& aSubImages )
{
	return	assets.CollectReferenceSubImage( aSubImages, m_strImageID ) ;
}



//////////////////////////////////////////////////////////////////////////////
// 高度画像（輝度）→ 法線画像
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::S3DRsrcImageBumpNormalProc, ImageRsrcProcedure, Controller )
S3D_IMPLEMENT_COMPOSER_ITEM( S3DRsrcImageBumpNormalProc, bump_normal )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DRsrcImageBumpNormalProc::S3DRsrcImageBumpNormalProc( void )
	: Controller( m_ItemClassDescriptor.pwszClassID ),
		m_pComposer( nullptr ), m_fpHeight( 1.0 ),
			m_flagMipmap( false ), m_flagCompressed( false )
{
	ESLVerify( AddParameterEntry
		( L"ref_image", S3DSceneComposer::typeCommand,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrStringEnumeration,
			L"元画像", NULL ) == paramImageID ) ;
	ESLVerify( paramMakeMipmap == AddParameterEntry
		( L"mipmap", S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrConstant,
			L"ミップマップ", L"ミップマップテクスチャにします。" ) ) ;
	ESLVerify( paramCompressed == AddParameterEntry
		( L"compressed", S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrConstant,
			L"圧縮テクスチャ化", L"圧縮テクスチャにします。" ) ) ;
	ESLVerify( AddParameterEntry
		( L"height", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrUIScalarSlider,
			L"高度係数", NULL, 0.0, 5.0 ) == paramHeight ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DRsrcImageBumpNormalProc::~S3DRsrcImageBumpNormalProc( void )
{
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
double S3DRsrcImageBumpNormalProc::GetScalarParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramHeight:
		return	m_fpHeight ;
	}
	return	0.0 ;
}

bool S3DRsrcImageBumpNormalProc::GetBooleanParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramMakeMipmap:
		return	m_flagMipmap ;
	case	paramCompressed:
		return	m_flagCompressed ;
	}
	return	false ;
}

const wchar_t * S3DRsrcImageBumpNormalProc::GetCommandParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramImageID:
		return	m_strImageID ;
	}
	return	nullptr ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DRsrcImageBumpNormalProc::SetScalarParameter( size_t iParam, double s )
{
	switch ( iParam )
	{
	case	paramHeight:
		m_fpHeight = s ;
		return ;
	}
}

void S3DRsrcImageBumpNormalProc::SetBooleanParameter( size_t iParam, bool b )
{
	switch ( iParam )
	{
	case	paramMakeMipmap:
		m_flagMipmap = b ;
		return ;
	case	paramCompressed:
		m_flagCompressed = b ;
		return ;
	}
}

void S3DRsrcImageBumpNormalProc::SetCommandParameter( size_t iParam, const wchar_t * pwszCmd )
{
	switch ( iParam )
	{
	case	paramImageID:
		m_strImageID = pwszCmd ;
		return ;
	}
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DRsrcImageBumpNormalProc::EnumerateStringSet
	( size_t iParam, SSystem::SStringArray& aStrSet )
{
	switch ( iParam )
	{
	case	paramImageID:
		m_pComposer->Assets().EnumerateResourceIDsAs
			( aStrSet, ESL_RUNTIME_CLASS(SGLImageObject) ) ;
		return	true ;
	}
	return	false ;
}

// S3DSceneComposer 関連付け
//////////////////////////////////////////////////////////////////////////////
void S3DRsrcImageBumpNormalProc::AttachSceneComposer( S3DSceneComposer * pComposer )
{
	m_pComposer = pComposer ;
}

// デシリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRsrcImageBumpNormalProc::ParseParameter( const SSystem::SXMLDocument& xmlProc )
{
	return	ParseParameterProperties( xmlProc ) ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRsrcImageBumpNormalProc::FormatParameter( SSystem::SXMLDocument& xmlProc )
{
	xmlProc.SetAttributeAs( L"proc_id", m_ItemClassDescriptor.pwszClassID ) ;
	xmlProc.RemoveAllElements() ;
	return	FormatParameterProperties( xmlProc ) ;
}

// リソース生成
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRsrcImageBumpNormalProc::CreateResource
	( ESLObject*& pRsrc, S3DSceneComposer::ResourceAssets& assets )
{
	S3DSceneComposer::ResourceContainer *
		prc = assets.GetResourceContainerAs( m_strImageID ) ;
	if ( (prc != NULL) && prc->IsPendingProceduralRsrc() )
	{
		return	sglErrPending ;
	}
	SGLImageObject *	pSrcImage = assets.GetImageAs( m_strImageID ) ;
	if ( pSrcImage == NULL )
	{
		return	sglErrFailed ;
	}
	SGLImage *	pNormalMap = new SGLImage ;
	S3DTextureLibrary::MakeNormalMapFromBumpMap
		( pNormalMap, pSrcImage, (float32_t) m_fpHeight ) ;
	//
	uint32_t nFlags = SGLImageObject::bufferSampleTiling ;
	if ( m_flagMipmap )
	{
		nFlags |= SGLImageObject::bufferForMipmapTexture ;
	}
	if ( m_flagCompressed )
	{
		nFlags |= SGLImageObject::bufferCompressedTexture ;
	}
	pNormalMap->NormalizeToTexture( nFlags ) ;
	//
	pRsrc = pNormalMap ;
	return	sglErrSuccess ;
}

// 参照元画像一覧
//////////////////////////////////////////////////////////////////////////////
SGLError S3DRsrcImageBumpNormalProc::CollectReferenceSubImages
	( S3DSceneComposer::ResourceAssets& assets,
		SSystem::SPointerArray<SGLImageBuffer>& aSubImages )
{
	return	assets.CollectReferenceSubImage( aSubImages, m_strImageID ) ;
}



//////////////////////////////////////////////////////////////////////////////
// コンポジション・モデル・インスタンス化
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::S3DResourceCompositionBakerProc, ResourceProcedure, Controller )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DResourceCompositionBakerProc, comp_baker )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DResourceCompositionBakerProc::S3DResourceCompositionBakerProc( void )
	: Controller( m_ItemClassDescriptor.pwszClassID ),
		m_pComposer( NULL ),
		m_matBaseRotation( 1, 1, 1 ),
		m_vBaseZoom( 1, 1, 1 ),
		m_vBaseCenter( 0, 0, 0 ),
		m_nCreateCount( 0 ),
		m_flagAllItems( false ),
		m_flagAllSpaces( false ),
		m_flagOptimizeBone( true ),
		m_flagWithoutBone( false ),
		m_flagMergeMaterialByName( true ),
		m_flagMergeMeshs( true ),
		m_flagMergeByMeshID( true ),
		m_flagMergeMaterials( false ),
		m_flagAnimaionTrack( false )
{
	AddParameterEntry
		( L"ref_composition", S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrStringEnumeration,
			L"参照コンポジション", NULL ) ;
	AddParameterEntry
		( L"rotation", S3DSceneComposer::typeRotation,
			S3DSceneComposer::attrConstant,
			L"回転", nullptr ) ;
	AddParameterEntry
		( L"zoom", S3DSceneComposer::typeZoom,
			S3DSceneComposer::attrConstant,
			L"拡大率", nullptr ) ;
	AddParameterEntry
		( L"center", S3DSceneComposer::typePosition,
			S3DSceneComposer::attrConstant,
			L"中心点", L"モデルの原点にしたい座標を指定します" ) ;
	AddParameterEntry
		( L"all_items", S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrConstant,
			L"不可視アイテムを含む", L"不可視アイテムもモデル化の対象とします" ) ;
	AddParameterEntry
		( L"all_spaces", S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrConstant,
			L"不可視空間を含む", L"不可視空間もモデル化の対象とします" ) ;
	AddParameterEntry
		( L"optimize_bone", S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrConstant,
			L"ボーン最適化", L"ボーンを GPU 用に最適化します" ) ;
	AddParameterEntry
		( L"without_bone", S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrConstant,
			L"ボーン無効",
			L"ボーンのないモデルデータとして出力します。\n"
			L"※ボーン無効を指定しない場合、"
			L"空間がボーンとして処理される為、ボーンのあるデータとなります。" ) ;
	AddParameterEntry
		( L"merge_by_material_id", S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrConstant,
			L"マテリアルIDで統合",
			L"マテリアルIDの @ 記号以降が同一のマテリアルを統合します" ) ;
	AddParameterEntry
		( L"merge_mesh", S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrConstant,
			L"メッシュ単位の統合",
			L"メッシュアイテム内の同一マテリアルの Patch を統合します" ) ;
	AddParameterEntry
		( L"merge_by_mesh_id", S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrConstant,
			L"メッシュIDで統合",
			L"メッシュIDの @ 記号以降が同一のメッシュを統合します" ) ;
	AddParameterEntry
		( L"merge_material", S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrConstant,
			L"マテリアル毎にメッシュ統合",
			L"同一マテリアルのメッシュを全て統合します" ) ;
	AddParameterEntry
		( L"animation_track", S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrConstant,
			L"アニメーショントラック", NULL ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DResourceCompositionBakerProc::~S3DResourceCompositionBakerProc( void )
{
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDMatrix S3DResourceCompositionBakerProc::GetMatrixParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramBaseRotation:
		return	m_matBaseRotation ;
	}
	return	S3DDMatrix( 1, 1, 1 ) ;
}

S3DDVector S3DResourceCompositionBakerProc::GetVectorParameter( size_t iParam ) const
{
	switch ( iParam )
	{
	case	paramBaseZoom:
		return	m_vBaseZoom ;
	case	paramBaseCenter:
		return	m_vBaseCenter ;
	}
	return	S3DDVector( 0, 0, 0 ) ;
}

bool S3DResourceCompositionBakerProc::GetBooleanParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramAllItems:
		return	m_flagAllItems ;
	case	paramAllSpaces:
		return	m_flagAllSpaces ;
	case	paramOptimizeBone:
		return	m_flagOptimizeBone ;
	case	paramWithoutBone:
		return	m_flagWithoutBone ;
	case	paramMergeByMaterialID:
		return	m_flagMergeMaterialByName ;
	case	paramMergeMeshs:
		return	m_flagMergeMeshs ;
	case	paramMergeByMeshID:
		return	m_flagMergeByMeshID ;
	case	paramMergeMaterials:
		return	m_flagMergeMaterials ;
	case	paramAnimationTrack:
		return	m_flagAnimaionTrack ;
	}
	return	false ;
}

const wchar_t * S3DResourceCompositionBakerProc::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramRefComposition:
		return	m_strCompositionID ;
	}
	return	NULL ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DResourceCompositionBakerProc::SetMatrixParameter( size_t iParam, const S3DDMatrix& mat )
{
	switch ( iParam )
	{
	case	paramBaseRotation:
		m_matBaseRotation = mat ;
		return ;
	}
}

void S3DResourceCompositionBakerProc::SetVectorParameter( size_t iParam, const S3DDVector& vec )
{
	switch ( iParam )
	{
	case	paramBaseZoom:
		m_vBaseZoom = vec ;
		return ;
	case	paramBaseCenter:
		m_vBaseCenter = vec ;
		return ;
	}
}

void S3DResourceCompositionBakerProc::SetBooleanParameter( size_t i, bool b )
{
	switch ( i )
	{
	case	paramAllItems:
		m_flagAllItems = b ;
		return ;
	case	paramAllSpaces:
		m_flagAllSpaces = b ;
		return ;
	case	paramOptimizeBone:
		m_flagOptimizeBone = b ;
		return ;
	case	paramWithoutBone:
		m_flagWithoutBone = b ;
		return ;
	case	paramMergeByMaterialID:
		m_flagMergeMaterialByName = b ;
		return ;
	case	paramMergeMeshs:
		m_flagMergeMeshs = b ;
		return ;
	case	paramMergeByMeshID:
		m_flagMergeByMeshID = b ;
		return ;
	case	paramMergeMaterials:
		m_flagMergeMaterials = b ;
		return ;
	case	paramAnimationTrack:
		m_flagAnimaionTrack = b ;
		return ;
	}
}

void S3DResourceCompositionBakerProc::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	switch ( i )
	{
	case	paramRefComposition:
		m_strCompositionID = pwszCmd ;
		return ;
	}
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DResourceCompositionBakerProc::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	switch ( i )
	{
	case	paramRefComposition:
		if ( m_pComposer != NULL )
		{
			m_pComposer->EnumerateCompositionIDs( aStrSet ) ;
		}
		return	true ;
	}
	return	false ;
}

// S3DSceneComposer 関連付け
//////////////////////////////////////////////////////////////////////////////
void S3DResourceCompositionBakerProc::AttachSceneComposer( S3DSceneComposer * pComposer )
{
	m_pComposer = pComposer ;
}

// デシリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError S3DResourceCompositionBakerProc::ParseParameter( const SSystem::SXMLDocument& xmlProc )
{
	return	ParseParameterProperties( xmlProc ) ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError S3DResourceCompositionBakerProc::FormatParameter( SSystem::SXMLDocument& xmlProc )
{
	xmlProc.SetAttributeAs( L"proc_id", m_ItemClassDescriptor.pwszClassID ) ;
	xmlProc.RemoveAllElements() ;
	return	FormatParameterProperties( xmlProc ) ;
}

// リソース生成
//////////////////////////////////////////////////////////////////////////////
SGLError S3DResourceCompositionBakerProc::CreateResource
	( ESLObject*& pRsrc, S3DSceneComposer::ResourceAssets& assets )
{
	if ( m_pComposer == NULL )
	{
		return	sglErrFailed ;
	}
	S3DSceneComposer::CompositionInfo *
		pci = m_pComposer->GetCompositionAs( m_strCompositionID, true ) ;
	if ( pci == NULL )
	{
		return	sglErrFailed ;
	}
	SSmartPointer<S3DSceneComposer::Composition>
				pComp = m_pComposer->CreateComposition( *pci ) ;
	if ( pComp == NULL )
	{
		return	sglErrFailed ;
	}
	S3DMeshEditorSerializer::ConvertModelParam	param ;
	param.nFlags = 0 ; //S3DMeshEditorSerializer::cvtFlagRefTexture ;
	if ( m_nCreateCount == 0 )
	{
		param.nFlags |= S3DMeshEditorSerializer::cvtFlagValidResourceRef ;
	}
	if ( m_flagAllItems )
	{
		param.nFlags |= S3DMeshEditorSerializer::cvtFlagAllItems ;
	}
	if ( m_flagAllSpaces )
	{
		param.nFlags |= S3DMeshEditorSerializer::cvtFlagAllSpaces ;
	}
	if ( m_flagOptimizeBone )
	{
		param.nFlags |= S3DMeshEditorSerializer::cvtFlagOptimizeBoneWeights ;
	}
	if ( m_flagMergeMaterialByName )
	{
		param.nFlags |= S3DMeshEditorSerializer::cvtFlagMergeMaterialByName ;
	}
	if ( m_flagMergeMeshs )
	{
		param.nFlags |= S3DMeshEditorSerializer::cvtFlagMergeMeshs ;
	}
	if ( m_flagMergeByMeshID )
	{
		param.nFlags |= S3DMeshEditorSerializer::cvtFlagMergeMeshByName ;
	}
	if ( m_flagMergeMaterials )
	{
		param.nFlags |= S3DMeshEditorSerializer::cvtFlagMergeMaterials ;
	}
	if ( m_flagWithoutBone )
	{
		param.nFlags |= S3DMeshEditorSerializer::cvtFlagWithoutBone ;
	}
	if ( !m_flagAnimaionTrack )
	{
		param.nFlags |= S3DMeshEditorSerializer::cvtFlagWithoutAnimation ;
	}
	param.pwszCompID = m_strCompositionID ;
	param.matBase = m_matBaseRotation * S3DDMatrix( m_vBaseZoom ) ;
	param.vBase = param.matBase * -m_vBaseCenter ;
	//
	S3DModelBuffer *	pModel = new S3DModelBuffer ;
	if ( !S3DMeshEditorSerializer::
				ConvertModelFromComposition( *pModel, *pComp, *pComp, param ) )
	{
		m_nCreateCount = 0 ;
		pRsrc = (S3DRenderBufferInterface*) pModel ;
		return	sglErrSuccess ;
	}
	else
	{
		m_nCreateCount ++ ;
		delete	pModel ;
		return	sglErrPending ;
	}
}

