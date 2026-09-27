
#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl_erisa_lib.h>
#include <sakuragl/sgl2d/sgl_image_filter.h>
#include <sakuragl/sgl2d/sgl_image_conversion.h>
#include <sakuraglx/sprite/sglx_sprite_filter.h>

#if	!defined(__COTOPHA__)
#include <sakuragl/sgl_opengl_render_context.h>
#endif

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// スプライト画像フィルタインターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSpriteFilter, SGLSpriteDrawer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteFilter::SGLSpriteFilter( void )
{
	m_paramFilter = 0 ;
	m_paramFilter2 = 0 ;
}

SGLSpriteFilter::SGLSpriteFilter( const SGLSpriteFilter& filter )
	: m_afParam( filter.m_afParam )
{
	m_paramFilter = filter.m_paramFilter ;
	m_paramFilter2 = filter.m_paramFilter2 ;
}

// 動的描画フィルタか？
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteFilter::IsDynamicDrawer( void ) const
{
	return	false ;
}

// 中間バッファを取得
//////////////////////////////////////////////////////////////////////////////
SGLImageObject * SGLSpriteFilter::GetInternalBuffer( const SGLImageInfo& infImage )
{
	SGLImageObject *	pImage = m_pBuffer ;
	if ( pImage == NULL )
	{
		pImage = new SGLImage ;
		m_pBuffer = pImage ;
	}
	SGLSize	size = pImage->GetImageSize() ;
	if ( (size.w != infImage.width)
		|| (size.h != infImage.height) )
	{
		pImage->CreateImage
			( infImage.width, infImage.height,
				infImage.format, infImage.depth,
				SGLImageObject::bufferOnMemory ) ;
	}
	return	pImage ;
}

// フィルタ処理
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFilter::Filter
	( S3DRenderContextInterface& render, SGLImageObject* image )
{
}

// フィルター進行度設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFilter::SetFilterParameter( int nParam, int nParam2 )
{
	m_paramFilter =
		eslRoundR32ToInt
			( (float32_t) (m_afParam.a11 * nParam
							+ m_afParam.a12 * nParam2 + m_afParam.a13) ) ;
	m_paramFilter2 =
		eslRoundR32ToInt
			( (float32_t) (m_afParam.a21 * nParam
							+ m_afParam.a22 * nParam2 + m_afParam.a23) ) ;
}

// フィルター変換行列設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFilter::SetParameterAffine( const SGLAffine& af )
{
	m_afParam = af ;
}

// タイマー処理
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFilter::OnTimer( SGLSprite& sprite, uint32_t msecPast )
{
}

// 複製
//////////////////////////////////////////////////////////////////////////////
SGLObject * SGLSpriteFilter::DuplicateObject( void )
{
	return	new SGLSpriteFilter( *this ) ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteFilter::OnSave( SSystem::SFileInterface& file )
{
	file.Write( &m_afParam, sizeof(SGLAffine) ) ;
	file.Write( &m_paramFilter, sizeof(int32_t) ) ;
	file.Write( &m_paramFilter2, sizeof(int32_t) ) ;
	return	sglErrSuccess ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteFilter::OnRestore( SSystem::SFileInterface& file )
{
	file.Read( &m_afParam, sizeof(SGLAffine) ) ;
	file.Read( &m_paramFilter, sizeof(int32_t) ) ;
	file.Read( &m_paramFilter2, sizeof(int32_t) ) ;
	return	sglErrSuccess ;
}



//////////////////////////////////////////////////////////////////////////////
// 透明度描画フィルタ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSpriteFilterTransparencyDrawer, SGLSpriteFilter )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteFilterTransparencyDrawer::SGLSpriteFilterTransparencyDrawer( void )
{
}

SGLSpriteFilterTransparencyDrawer::SGLSpriteFilterTransparencyDrawer
						( const SGLSpriteFilterTransparencyDrawer& ftd )
: SGLSpriteFilter( ftd )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteFilterTransparencyDrawer::~SGLSpriteFilterTransparencyDrawer( void )
{
}

// 描画
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFilterTransparencyDrawer::Draw
	( S3DRenderContextInterface& render,
		const SGLPaintParam& pp, SGLImageObject* image )
{
	if ( image != NULL )
	{
		SGLPaintParam	ppPaint = pp ;
		if ( m_paramFilter >= 0x100 )
		{
			return ;
		}
		else if ( m_paramFilter > 0 )
		{
			ppPaint.nTransparency =
				0x100 - (0x100 - ppPaint.nTransparency)
							* (0x100 - m_paramFilter) / 0x100 ;
		}
		render.DrawImage( ppPaint, image ) ;
	}
}

// 動的描画フィルタか？
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteFilterTransparencyDrawer::IsDynamicDrawer( void ) const
{
	return	true ;
}

// 複製
//////////////////////////////////////////////////////////////////////////////
SGLObject * SGLSpriteFilterTransparencyDrawer::DuplicateObject( void )
{
	return	new SGLSpriteFilterTransparencyDrawer( *this ) ;
}


//////////////////////////////////////////////////////////////////////////////
// αチャネル窓関数画像フィルタ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSpriteFilterBlendAlpha, SGLSpriteFilter )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteFilterBlendAlpha::SGLSpriteFilterBlendAlpha( void )
{
	m_fxAlphaCoefficient = 0x100 ;
}

SGLSpriteFilterBlendAlpha::SGLSpriteFilterBlendAlpha
				( const SGLSpriteFilterBlendAlpha& filter )
	: SGLSpriteFilter( filter ),
		m_strAlphaFile( filter.m_strAlphaFile ),
		m_refAlphaImage( filter.m_refAlphaImage ),
		m_fxAlphaCoefficient( filter.m_fxAlphaCoefficient )
{
	if ( filter.m_pAlphaImage != NULL )
	{
		m_pAlphaImage = filter.m_pAlphaImage->NewReference() ;
	}
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteFilterBlendAlpha::~SGLSpriteFilterBlendAlpha( void )
{
}

// 画像ファイル読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteFilterBlendAlpha::LoadAlphaImage( const wchar_t * pwszFilePath )
{
	m_pAlphaImage = new SGLImage ;
	if ( m_pAlphaImage->LoadImage( pwszFilePath ) )
	{
		return	sglErrFailed ;
	}
	m_strAlphaFile = pwszFilePath ;
	m_refAlphaImage = m_pAlphaImage.Ptr() ;
	return	sglErrSuccess ;
}

// 画像を関連付ける
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFilterBlendAlpha::AttachAlphaImage( SGLImageObject* pImage )
{
	m_strAlphaFile.FreeArray() ;
	m_pAlphaImage = NULL ;
	m_refAlphaImage = pImage ;
}

// パラメータ設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFilterBlendAlpha::SetAlphaParameter( int32_t fxAlphaCoefficient )
{
	m_fxAlphaCoefficient = fxAlphaCoefficient ;
}

// フィルタ処理
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFilterBlendAlpha::Filter
	( S3DRenderContextInterface& render, SGLImageObject* image )
{
	SGLImageObject *	alpha = m_refAlphaImage.GetReference() ;
	if ( (image == NULL) || (alpha == NULL) )
	{
		return ;
	}
	SGLImageBuffer	imgDst, imgAlpha ;
	int32_t	fxAlphaIntercept =
				0x100 - (m_fxAlphaCoefficient + 1) * m_paramFilter ;
	imgDst.ptrBuffer = image->LockBuffer( imgDst ) ;
	imgAlpha.ptrBuffer =
			alpha->LockBuffer( imgAlpha, SGLImageObject::lockRead ) ;
	//
	sglBlendWithAlphaChannel
		( imgDst, imgAlpha, m_fxAlphaCoefficient, fxAlphaIntercept ) ;
	//
	image->UnlockBuffer() ;
	alpha->UnlockBuffer( SGLImageObject::lockRead ) ;
}

// 複製
//////////////////////////////////////////////////////////////////////////////
SGLObject * SGLSpriteFilterBlendAlpha::DuplicateObject( void )
{
	return	new SGLSpriteFilterBlendAlpha( *this ) ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteFilterBlendAlpha::OnSave( SSystem::SFileInterface& file )
{
	SGLError	err = SGLSpriteFilter::OnSave( file ) ;
	if ( err )
	{
		return	err ;
	}
	file.Write( &m_fxAlphaCoefficient, sizeof(int32_t) ) ;
	return	(SGLError) file.WriteString( m_strAlphaFile ) ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteFilterBlendAlpha::OnRestore( SSystem::SFileInterface& file )
{
	SGLError	err = SGLSpriteFilter::OnRestore( file ) ;
	if ( err )
	{
		return	err ;
	}
	SString	strAlphaFile ;
	file.Read( &m_fxAlphaCoefficient, sizeof(int32_t) ) ;
	err = (SGLError) file.ReadString( strAlphaFile ) ;
	if ( err )
	{
		return	err ;
	}
	return	LoadAlphaImage( strAlphaFile ) ;
}


//////////////////////////////////////////////////////////////////////////////
// トーンフィルタ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSpriteFilterTone, SGLSpriteFilter )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteFilterTone::SGLSpriteFilterTone( void )
{
	m_flagMorphFilter = true ;
	LoadStraightFilter() ;
}

SGLSpriteFilterTone::SGLSpriteFilterTone( const SGLSpriteFilterTone& filter )
	: SGLSpriteFilter( filter )
{
	m_flagMorphFilter = filter.m_flagMorphFilter ;
	m_nFilterFlags = filter.m_nFilterFlags ;
	eslMoveMemory( &m_bufTone[0][0], &filter.m_bufTone[0][0], 0x400 ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteFilterTone::~SGLSpriteFilterTone( void )
{
}

// フィルタファイルを読み込む
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteFilterTone::LoadFilterFile( const wchar_t * pwszFilePath )
{
	SSmartPointer<SFileInterface>
		pfile = SFileOpener::DefaultNewOpenFile
						( pwszFilePath, SFileOpener::shareRead ) ;
	if ( pfile == NULL )
	{
		return	sglErrFailed ;
	}
	return	ReadFilterFile( *pfile ) ;
}

SGLError SGLSpriteFilterTone::ReadFilterFile( SFileInterface& file )
{
	SChunkFile	cf ;
	if ( cf.OpenChunkFile( &file ) )
	{
		return	sglErrFailed ;
	}
	LoadStraightFilter() ;
	//
	for ( ; ; )
	{
		if ( cf.DescendChunk() )
		{
			break ;
		}
		if ( cf.IsEqualCurrentChunkID( "blue    " ) )
		{
			file.Read( &m_bufTone[0][0], 0x100 ) ;
		}
		else if ( cf.IsEqualCurrentChunkID( "green   " ) )
		{
			file.Read( &m_bufTone[1][0], 0x100 ) ;
		}
		else if ( cf.IsEqualCurrentChunkID( "red     " ) )
		{
			file.Read( &m_bufTone[2][0], 0x100 ) ;
		}
		else if ( cf.IsEqualCurrentChunkID( "alpha   " ) )
		{
			file.Read( &m_bufTone[3][0], 0x100 ) ;
		}
		else if ( cf.IsEqualCurrentChunkID( "info    " ) )
		{
			file.Read( &m_nFilterFlags, sizeof(uint32_t) ) ;
		}
		cf.AscendChunk() ;
	}
	return	sglErrSuccess ;
}

// ストレートフィルタを設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFilterTone::LoadStraightFilter( void )
{
	m_nFilterFlags = 0 ;
	for ( size_t i = 0; i < 0x100; i ++ )
	{
		m_bufTone[0][i] = (uint8_t) i ;
		m_bufTone[1][i] = (uint8_t) i ;
		m_bufTone[2][i] = (uint8_t) i ;
		m_bufTone[3][i] = (uint8_t) i ;
	}
}

// トーンカーブを取得
//////////////////////////////////////////////////////////////////////////////
uint32_t SGLSpriteFilterTone::GetToneFilter
		( uint8_t * pbytRed, uint8_t * pbytGreen,
			uint8_t * pbytBlue, uint8_t * pbytAlpha )
{
	if ( pbytRed != NULL )
	{
		eslMoveMemory( pbytRed, &m_bufTone[2][0], 0x100 ) ;
	}
	if ( pbytGreen != NULL )
	{
		eslMoveMemory( pbytGreen, &m_bufTone[1][0], 0x100 ) ;
	}
	if ( pbytBlue != NULL )
	{
		eslMoveMemory( pbytBlue, &m_bufTone[0][0], 0x100 ) ;
	}
	if ( pbytAlpha != NULL )
	{
		eslMoveMemory( pbytAlpha, &m_bufTone[3][0], 0x100 ) ;
	}
	return	m_nFilterFlags ;
}

// トーンカーブを設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFilterTone::SetToneFilter
	( const uint8_t * pbytRed,
		const uint8_t * pbytGreen,
		const uint8_t * pbytBlue,
		const uint8_t * pbytAlpha, uint32_t nFlags )
{
	if ( pbytRed != NULL )
	{
		eslMoveMemory( &m_bufTone[2][0], pbytRed, 0x100 ) ;
	}
	if ( pbytGreen != NULL )
	{
		eslMoveMemory( &m_bufTone[1][0], pbytGreen, 0x100 ) ;
	}
	if ( pbytBlue != NULL )
	{
		eslMoveMemory( &m_bufTone[0][0], pbytBlue, 0x100 ) ;
	}
	if ( pbytAlpha != NULL )
	{
		eslMoveMemory( &m_bufTone[3][0], pbytAlpha, 0x100 ) ;
	}
	m_nFilterFlags = nFlags ;
}

// トーンカーブを生成
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFilterTone::GenerateToneFilter
	( int nRed, int nGreen, int nBlue, int nAlpha, int nType, uint32_t nFlags )
{
	sglMakeToneFilter( &m_bufTone[2][0], nRed, nType ) ;
	sglMakeToneFilter( &m_bufTone[1][0], nGreen, nType ) ;
	sglMakeToneFilter( &m_bufTone[0][0], nBlue, nType ) ;
	sglMakeToneFilter( &m_bufTone[3][0], nAlpha, nType ) ;
	m_nFilterFlags = nFlags ;
	//
	if ( nFlags & flagFixZero )
	{
		m_bufTone[0][0] = 0 ;
		m_bufTone[1][0] = 0 ;
		m_bufTone[2][0] = 0 ;
		m_bufTone[3][0] = 0 ;
	}
}

// モーフィング有効／無効化
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFilterTone::EnableMorphing( bool flagMorph )
{
	m_flagMorphFilter = flagMorph ;
}

// 代入
//////////////////////////////////////////////////////////////////////////////
const SGLSpriteFilterTone&
	SGLSpriteFilterTone::operator = ( const SGLSpriteFilterTone& filter )
{
	m_paramFilter = filter.m_paramFilter ;
	m_flagMorphFilter = filter.m_flagMorphFilter ;
	m_nFilterFlags = filter.m_nFilterFlags ;
	eslMoveMemory( &m_bufTone[0][0], &filter.m_bufTone[0][0], 0x400 ) ;
	return	*this ;
}

// 積
//////////////////////////////////////////////////////////////////////////////
SGLSpriteFilterTone SGLSpriteFilterTone::operator * ( double t )
{
	SGLSpriteFilterTone	tone( *this ) ;
	tone *= t ;
	return	tone ;
}

const SGLSpriteFilterTone& SGLSpriteFilterTone::operator *= ( double t )
{
	uint32_t	n =
		(uint32_t) eslRoundR32ToInt( (float32_t) (t * 0x100) ) ;
	if ( (int32_t) n < 0 )
	{
		n = 0 ;
	}
	else if ( n > 0x100 )
	{
		n = 0x100 ;
	}
	for ( size_t i = 0; i < 0x100; i ++ )
	{
		m_bufTone[0][i] =
			(uint8_t) (((uint32_t) m_bufTone[0][i] * n) >> 8) ;
		m_bufTone[1][i] =
			(uint8_t) (((uint32_t) m_bufTone[1][i] * n) >> 8) ;
		m_bufTone[2][i] =
			(uint8_t) (((uint32_t) m_bufTone[2][i] * n) >> 8) ;
		m_bufTone[3][i] =
			(uint8_t) (((uint32_t) m_bufTone[3][i] * n) >> 8) ;
	}
	return	*this ;
}

// 和
//////////////////////////////////////////////////////////////////////////////
SGLSpriteFilterTone
	SGLSpriteFilterTone::operator + ( const SGLSpriteFilterTone& filter )
{
	SGLSpriteFilterTone	tone( *this ) ;
	tone += filter ;
	return	tone ;
}

const SGLSpriteFilterTone&
	SGLSpriteFilterTone::operator += ( const SGLSpriteFilterTone& filter )
{
	uint8_t *		pbytDstTone = &m_bufTone[0][0] ;
	const uint8_t *	pbytSrcTone = &filter.m_bufTone[0][0] ;
	for ( size_t i = 0; i < 0x400; i ++ )
	{
		uint32_t	n = (uint32_t) pbytDstTone[i] + pbytSrcTone[i] ;
		pbytDstTone[i] = (uint8_t) (n | (- (int32_t) (n >> 8))) ;
	}
	return	*this ;
}

// フィルタ処理
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFilterTone::Filter
	( S3DRenderContextInterface& render, SGLImageObject* image )
{
	if ( image == NULL )
	{
		return ;
	}
	uint8_t		bufTone[4][0x100] ;
	uint8_t *	pbytRedTone = &m_bufTone[2][0] ;
	uint8_t *	pbytGreenTone = &m_bufTone[1][0] ;
	uint8_t *	pbytBlueTone = &m_bufTone[0][0] ;
	uint8_t *	pbytAlphaTone = &m_bufTone[3][0] ;
	//
	if ( m_flagMorphFilter && (m_paramFilter < 0x100) )
	{
		uint32_t	t = 0 ;
		if ( m_paramFilter > 0 )
		{
			t = (uint32_t) m_paramFilter ;
		}
		uint32_t	nt = 0x100 - t ;
		//
		for ( size_t i = 0; i < 0x100; i ++ )
		{
			bufTone[0][i] =
				(uint8_t) ((i * nt + pbytBlueTone[i] * t) >> 8) ;
			bufTone[1][i] =
				(uint8_t) ((i * nt + pbytGreenTone[i] * t) >> 8) ;
			bufTone[2][i] =
				(uint8_t) ((i * nt + pbytRedTone[i] * t) >> 8) ;
			bufTone[3][i] =
				(uint8_t) ((i * nt + pbytAlphaTone[i] * t) >> 8) ;
		}
		pbytRedTone = &bufTone[2][0] ;
		pbytGreenTone = &bufTone[1][0] ;
		pbytBlueTone = &bufTone[0][0] ;
		pbytAlphaTone = &bufTone[3][0] ;
	}
	//
	SGLImageBuffer	imgbuf ;
	imgbuf.ptrBuffer = image->LockBuffer( imgbuf ) ;
	//
	if ( m_nFilterFlags & flagGrayFilter )
	{
		sglMakeGrayImageFromRGB( imgbuf ) ;
		sglApplyToneImageFilter
			( imgbuf, NULL,
				pbytRedTone, pbytGreenTone,
				pbytBlueTone, pbytAlphaTone ) ;
	}
	else if ( m_nFilterFlags & flagYUVFilter )
	{
		SGLImageBuffer	imgYUV = imgbuf ;
		imgYUV.format = formatImageYUV
							| (imgYUV.format & ~formatImageTypeMask) ;
		sglConvertImageBuffer( imgYUV, imgbuf ) ;
		sglApplyToneImageFilter
			( imgbuf, NULL,
				pbytRedTone, pbytGreenTone,
				pbytBlueTone, pbytAlphaTone ) ;
		sglConvertImageBuffer( imgbuf, imgYUV ) ;
	}
	else
	{
		sglApplyToneImageFilter
			( imgbuf, NULL,
				pbytRedTone, pbytGreenTone,
				pbytBlueTone, pbytAlphaTone ) ;
	}
	if ( m_nFilterFlags & flagMaskWithAlpha )
	{
		sglMultiplyImageRGBAlpha( imgbuf ) ;
	}
	image->UnlockBuffer() ;
}

// 複製
//////////////////////////////////////////////////////////////////////////////
SGLObject * SGLSpriteFilterTone::DuplicateObject( void )
{
	return	new SGLSpriteFilterTone( *this ) ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteFilterTone::OnSave( SSystem::SFileInterface& file )
{
	SGLError	err = SGLSpriteFilter::OnSave( file ) ;
	if ( err )
	{
		return	err ;
	}
	uint32_t	nFlags = 0 ;
	if ( m_flagMorphFilter )
	{
		nFlags |= 0x01 ;
	}
	file.Write( &nFlags, sizeof(uint32_t) ) ;
	file.Write( &m_nFilterFlags, sizeof(uint32_t) ) ;
	file.Write( &m_bufTone[0][0], 0x400 ) ;
	return	sglErrSuccess ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteFilterTone::OnRestore( SSystem::SFileInterface& file )
{
	SGLError	err = SGLSpriteFilter::OnRestore( file ) ;
	if ( err )
	{
		return	err ;
	}
	uint32_t	nFlags = 0 ;
	file.Read( &nFlags, sizeof(uint32_t) ) ;
	m_flagMorphFilter = ((nFlags & 0x01) != 0) ;
	//
	file.Read( &m_nFilterFlags, sizeof(uint32_t) ) ;
	if ( file.Read( &m_bufTone[0][0], 0x400 ) < 0x400 )
	{
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}


//////////////////////////////////////////////////////////////////////////////
// 簡易ぼかしフィルタ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLSpriteFilterShadingOff, SGLSpriteFilter )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteFilterShadingOff::SGLSpriteFilterShadingOff( void )
{
	m_nShadingScale = 32 ;
	m_flagTransition = false ;
	m_argbOverColor = 0 ;
}

SGLSpriteFilterShadingOff::SGLSpriteFilterShadingOff( const SGLSpriteFilterShadingOff& filter )
	: SGLSpriteFilter( filter )
{
	m_nShadingScale = filter.m_nShadingScale ;
	m_flagTransition = filter.m_flagTransition ;
	m_argbOverColor = filter.m_argbOverColor ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteFilterShadingOff::~SGLSpriteFilterShadingOff( void )
{
}

// トランジッション設定（第二パラメータでの透明度と色効果）
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFilterShadingOff::SetTransitionOption
	( bool flagTransition, uint32_t argbOverColor )
{
	m_flagTransition = flagTransition ;
	m_argbOverColor = argbOverColor ;
}

// 動的描画フィルタか？
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteFilterShadingOff::IsDynamicDrawer( void ) const
{
	return	m_flagTransition ;
}

// 描画
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFilterShadingOff::Draw
	( S3DRenderContextInterface& render,
		const SGLPaintParam& pp, SGLImageObject* image )
{
	if ( m_flagTransition )
	{
		SGLPaintParam	ppParam = pp ;
		if ( m_paramFilter2 > 0 )
		{
			if ( (m_paramFilter2 < 0x100)
				&& (ppParam.nTransparency < 0x100) )
			{
				ppParam.nTransparency =
					0x100 - (0x100 - m_paramFilter2)
							* (0x100 - ppParam.nTransparency) / 0x100 ;
			}
			else
			{
				return ;
			}
		}
		SGLSpriteFilter::Draw( render, ppParam, image ) ;
	}
	else
	{
		SGLSpriteFilter::Draw( render, pp, image ) ;
	}
}

// フィルタ処理
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFilterShadingOff::Filter
	( S3DRenderContextInterface& render, SGLImageObject* image )
{
#if	defined(__COTOPHA__)
	FilterByCPU( &render, image ) ;
#else
	FilterByGPU( &render, image ) ;
#endif
}

#if	!defined(__COTOPHA__)
void SGLSpriteFilterShadingOff::FilterByGPU
	( S3DRenderContextInterface * render, SGLImageObject* image )
{
	if ( m_paramFilter == 0 )
	{
		return ;
	}
	S3DRenderDevice *	pDevice = render->GetRenderDeviceObject() ;
	if ( pDevice == NULL )
	{
		FilterByCPU( render, image ) ;
		return ;
	}
	S3DCustomShader *	pShader =
			pDevice->GetDefaultShaderProgramAs
				( S3DRenderDevice::DefaultShaderId::GaussianBlur ) ;
	S3DGaussianBlurShaderInterface *	pgbShader =
		ESLTypeCast<S3DGaussianBlurShaderInterface>( pShader );
	if ( pgbShader == NULL )
	{
		FilterByCPU( render, image ) ;
		return ;
	}
	double	fpWidth = 0 ;
	fpWidth = (m_paramFilter * m_nShadingScale) / 256.0 * 4.0 ;
	//
	// 初段ぼかし
	//
	SGLImageObject *	pInternalDst = image ;
	double	g = fpWidth ;
	bool	fQuadBlur = false ;
	//
	SGLImageInfo	imginf ;
	image->GetImageInfo( imginf ) ;
	//
	SGLSize	sizeImage( imginf.width, imginf.height ) ;
	if ( m_imgTempBuf.GetImageSize() != sizeImage )
	{
		m_imgTempBuf.CreateImage
			( sizeImage.w, sizeImage.h,
				imginf.format, imginf.depth,
				SGLImageObject::bufferOnDeviceOnly ) ;
	}
	if ( g > 8.0 )
	{
		g = 8.0 ;
		fQuadBlur = true ;
		//
		if ( m_imgTempBuf2.GetImageSize() != sizeImage )
		{
			m_imgTempBuf2.CreateImage
				( sizeImage.w, sizeImage.h,
					imginf.format, imginf.depth,
					SGLImageObject::bufferOnDeviceOnly ) ;
		}
		pInternalDst = &m_imgTempBuf2 ;
	}
	render->PushTransformation() ;
	pgbShader->LockParameter() ;
	pgbShader->SetGauss( g ) ;
	pgbShader->SetDirection( 1, 0 ) ;
	pgbShader->SetShaderUniformsTo( *render ) ;
	pgbShader->UnlockParameter() ;
	render->AttachCustomShader( pShader ) ;
	//
	SGLPaintParam	pp ;
	render->AttachTargetImage( &m_imgTempBuf, NULL, NULL ) ;
	render->FillClearTarget( 0 ) ;
	render->Begin3DRenderer() ;
	render->DrawImage( pp, image, NULL ) ;
	render->End3DRenderer() ;
	render->DetachTargetImage( ) ;
	//
	pgbShader->LockParameter() ;
	pgbShader->SetGauss( g ) ;
	pgbShader->SetDirection( 0, 1 ) ;
	pgbShader->SetShaderUniformsTo( *render ) ;
	pgbShader->UnlockParameter() ;
	render->AttachTargetImage( pInternalDst, NULL, NULL ) ;
	render->FillClearTarget( 0 ) ;
	render->Begin3DRenderer() ;
	render->DrawImage( pp, &m_imgTempBuf, NULL ) ;
	render->End3DRenderer() ;
	render->DetachTargetImage( ) ;
	//
	// ２段目ぼかし
	//
	if ( fQuadBlur )
	{
		pgbShader->LockParameter() ;
		pgbShader->SetGauss( fpWidth / 8.0 ) ;
		pgbShader->SetDirection( 3, 0 ) ;
		pgbShader->SetShaderUniformsTo( *render ) ;
		pgbShader->UnlockParameter() ;
		render->AttachTargetImage( &m_imgTempBuf, NULL, NULL ) ;
		render->FillClearTarget( 0 ) ;
		render->Begin3DRenderer() ;
		render->DrawImage( pp, pInternalDst, NULL ) ;
		render->End3DRenderer() ;
		render->DetachTargetImage( ) ;
		//
		pgbShader->LockParameter() ;
		pgbShader->SetGauss( fpWidth / 8.0 ) ;
		pgbShader->SetDirection( 0, 3 ) ;
		pgbShader->SetShaderUniformsTo( *render ) ;
		pgbShader->UnlockParameter() ;
		render->AttachTargetImage( image, NULL, NULL ) ;
		render->FillClearTarget( 0 ) ;
		render->Begin3DRenderer() ;
		render->DrawImage( pp, &m_imgTempBuf, NULL ) ;
		render->End3DRenderer() ;
		render->DetachTargetImage( ) ;
	}
	render->AttachCustomShader( NULL ) ;
	render->PopTransformation() ;
	//
	// 色効果
	//
	if ( m_argbOverColor.ui32 != 0 )
	{
		uint32_t	n = m_paramFilter2 ;
		if ( n >= 0x100 )
		{
			n = 0x100 ;
		}
		render->FillRectangle
			( 0, 0, imginf.width, imginf.height, m_argbOverColor.imul(n) ) ;
	}
}
#endif

void SGLSpriteFilterShadingOff::FilterByCPU
	( S3DRenderContextInterface * render, SGLImageObject* image )
{
	double		fpWidth = 0 ;
	uint32_t	nWidth = 0 ;
	if ( m_paramFilter > 0 )
	{
		fpWidth = (m_paramFilter * m_nShadingScale) / 256.0 ;
		nWidth = (m_paramFilter * m_nShadingScale) >> 8 ;
	}
	if ( nWidth == 0 )
	{
		return ;
	}
	//
	// 縮小サンプリング
	//
	const size_t	nScaleLimit = 2 ;
	SGLImageObject *	pLast = image ;
	size_t	i ;
	for ( i = 0; i < nScaleLimit; i ++ )
	{
		SGLImageBuffer	imgSrc ;
		imgSrc.ptrBuffer =
			pLast->LockBuffer( imgSrc, SGLImageObject::lockRead ) ;
		//
		SGLSize	sizeHalf
			( (imgSrc.width + 1) >> 1, (imgSrc.height + 1) >> 1 ) ;
		SGLImage *	pImageDst = m_arrScaledBuffer.GetAt( i ) ;
		if ( (pImageDst == NULL)
			|| (pImageDst->GetImageSize() != sizeHalf) )
		{
			pImageDst = new SGLImage ;
			pImageDst->CreateImage
				( sizeHalf.w, sizeHalf.h, imgSrc.format, imgSrc.depth ) ;
			m_arrScaledBuffer.SetAt( i, pImageDst ) ;
		}
		//
		SGLImageBuffer	imgDst ;
		imgDst.ptrBuffer =
			pImageDst->LockBuffer( imgDst, SGLImageObject::lockWrite ) ;
		//
		sglEnlargeHalfImageBuffer( imgDst, imgSrc ) ;
		//
		pImageDst->UnlockBuffer( SGLImageObject::lockWrite ) ;
		pLast->UnlockBuffer( SGLImageObject::lockRead ) ;
		pLast = pImageDst ;
	}
	//
	// ガウスぼかし
	//
	SGLSize	sizeSrcImage = pLast->GetImageSize() ;
	if ( m_imgTempBuf.GetImageSize() != sizeSrcImage )
	{
		m_imgTempBuf.CreateImage
			( sizeSrcImage.w, sizeSrcImage.h, formatImageARGB, 32 ) ;
	}
	SGLImageBuffer	imgSrc, imgTemp ;
	imgSrc.ptrBuffer = pLast->LockBuffer( imgSrc ) ;
	imgTemp.ptrBuffer = m_imgTempBuf.LockBuffer( imgTemp ) ;
	//
	float32_t	fpGaussianValue =
		(float32_t) (fpWidth * (1.0 / (1 << nScaleLimit))) ;
	if ( fpGaussianValue < 2.0 )
	{
		fpGaussianValue = 2.0 ;
	}
	sglGaussianBlur
		( imgSrc, imgSrc, imgTemp, fpGaussianValue, 8 ) ;
	//
	m_imgTempBuf.UnlockBuffer() ;
	pLast->UnlockBuffer() ;
	//
	// 拡大
	//
	render->AttachTargetImage( image, NULL, NULL ) ;
	//
	SGLPaintParam	pp ;
	SGLAffine		affine ;
	SGLSize			sizeDst = image->GetImageSize() ;
	SGLSize			sizeSrc = pLast->GetImageSize() ;
	//
	pp.nFlags = paintFunctionMove ;
	pp.pAffine = &affine ;
	//
	if ( (sizeSrc.w > 1) && (sizeSrc.h > 1) )
	{
		if ( fpGaussianValue <= 2.0 )
		{
			pp.nTransparency =
				0x100 - (int) eslRoundR64ToLInt
						( (fpWidth * (256.0 / (2 << nScaleLimit))) ) ;
		}
		affine.a11 = (float32_t) (sizeDst.w - 1)
						/ (float32_t) (sizeSrc.w - 1) ;
		affine.a22 = (float32_t) (sizeDst.h - 1)
						/ (float32_t) (sizeSrc.h - 1) ;
		render->DrawImage( pp, pLast, NULL ) ;
	}
	//
	// 色効果
	//
	if ( m_argbOverColor.ui32 != 0 )
	{
		uint32_t	n = m_paramFilter2 ;
		if ( n >= 0x100 )
		{
			n = 0x100 ;
		}
		render->FillRectangle
			( 0, 0, sizeDst.w, sizeDst.h, m_argbOverColor.imul(n) ) ;
	}
	//
	render->DetachTargetImage() ;
}

// 複製
//////////////////////////////////////////////////////////////////////////////
SGLObject * SGLSpriteFilterShadingOff::DuplicateObject( void )
{
	return	new SGLSpriteFilterShadingOff( *this ) ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteFilterShadingOff::OnSave( SSystem::SFileInterface& file )
{
	SGLError	err = SGLSpriteFilter::OnSave( file ) ;
	//
	if ( file.Write( &m_nShadingScale, sizeof(uint32_t) ) < sizeof(uint32_t) )
	{
		err = sglErrFailed ;
	}
	if ( file.Write( &m_argbOverColor, sizeof(SGLPalette) ) < sizeof(SGLPalette) )
	{
		err = sglErrFailed ;
	}
	uint32_t	nFlags = 0 ;
	if ( m_flagTransition )
	{
		nFlags |= 0x01 ;
	}
	if ( file.Write( &nFlags, sizeof(uint32_t) ) < sizeof(uint32_t) )
	{
		err = sglErrFailed ;
	}
	return	err ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteFilterShadingOff::OnRestore( SSystem::SFileInterface& file )
{
	SGLError	err = SGLSpriteFilter::OnRestore( file ) ;
	//
	if ( file.Read( &m_nShadingScale, sizeof(uint32_t) ) < sizeof(uint32_t) )
	{
		err = sglErrFailed ;
	}
	if ( file.Read( &m_argbOverColor, sizeof(SGLPalette) ) < sizeof(SGLPalette) )
	{
		err = sglErrFailed ;
	}
	uint32_t	nFlags = 0 ;
	if ( file.Read( &nFlags, sizeof(uint32_t) ) < sizeof(uint32_t) )
	{
		err = sglErrFailed ;
	}
	m_flagTransition = ((nFlags & 0x01) != 0) ;
	return	err ;
}


//////////////////////////////////////////////////////////////////////////////
// メッシュ・エフェクタ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSpriteFilterMeshWarp::Effector, SGLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteFilterMeshWarp::Effector::Effector( void )
{
}

SGLSpriteFilterMeshWarp::Effector::Effector( const SGLSpriteFilterMeshWarp::Effector& eff )
{
}

// 初期設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFilterMeshWarp::Effector::OnAttachedMesh
		( size_t wMesh, size_t hMesh, size_t wCanvas, size_t hCanvas )
{
}

// 時間経過
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteFilterMeshWarp::Effector::OnTimer
	( SGLSprite& sprite,
		SGLSpriteFilterMeshWarp& mesh, uint32_t msecPast )
{
	return	true ;
}

// メッシュ頂点変位加算
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFilterMeshWarp::Effector::WarpMesh
	( SGLSpriteFilterMeshWarp& mesh,
		S2DVector * pvDst,
		const S2DVector * pvSrc, size_t wMesh, size_t hMesh )
{
}


//////////////////////////////////////////////////////////////////////////////
// メッシュワープフィルタ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSpriteFilterMeshWarp, SGLSpriteFilter )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteFilterMeshWarp::SGLSpriteFilterMeshWarp( void )
{
	m_flagSpcSrcMesh = false ;
	m_flagDynamicDraw = true ;
	m_flagFixBorder = true ;
}

SGLSpriteFilterMeshWarp::SGLSpriteFilterMeshWarp( const SGLSpriteFilterMeshWarp& filter )
	: m_meshSrc( filter.m_meshSrc ),
		m_meshDst( filter.m_meshDst ),
		m_flagSpcSrcMesh( filter.m_flagSpcSrcMesh ),
		m_flagDynamicDraw( filter.m_flagDynamicDraw ),
		m_flagFixBorder( filter.m_flagFixBorder ),
		m_sizeInPixels( filter.m_sizeInPixels ),
		m_sizeInMesh( filter.m_sizeInMesh ),
		m_rectDstMesh( filter.m_rectDstMesh )
{
	DuplicateObjectArray<Effector>( m_effectors, filter.m_effectors ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteFilterMeshWarp::~SGLSpriteFilterMeshWarp( void )
{
}

// メッシュサイズ設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFilterMeshWarp::SetMeshSize( size_t width, size_t height )
{
	m_sizeInMesh.w = (int32_t) width ;
	m_sizeInMesh.h = (int32_t) height ;
	//
	size_t	countVertex = (width + 1) * (height + 1) ;
	m_meshSrc.SetLength( countVertex ) ;
	m_meshDst.SetLength( countVertex ) ;
	//
	SetFilteredMesh() ;
}

// 画像サイズ設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFilterMeshWarp::SetCanvasSize( size_t width, size_t height )
{
	m_sizeInPixels.w = (int32_t) width ;
	m_sizeInPixels.h = (int32_t) height ;
	//
	SetFilteredMesh() ;
}

// メッシュサイズ取得
//////////////////////////////////////////////////////////////////////////////
const SGLSize& SGLSpriteFilterMeshWarp::GetMeshSize( void ) const
{
	return	m_sizeInMesh ;
}

// 画像サイズ取得
//////////////////////////////////////////////////////////////////////////////
const SGLSize& SGLSpriteFilterMeshWarp::GetCanvasSize( void ) const
{
	return	m_sizeInPixels ;
}

// ソースメッシュを設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFilterMeshWarp::SetSourceMesh( const S2DVector * pvMesh )
{
	if ( pvMesh != NULL )
	{
		size_t	countVertex = (m_sizeInMesh.w + 1) * (m_sizeInMesh.h + 1) ;
		m_meshSrc.SetLength( countVertex ) ;
		eslMoveMemory
			( m_meshSrc.GetArray(),
				pvMesh, countVertex * sizeof(S2DVector) ) ;
		m_meshSrc.FinishArray() ;
	}
	else
	{
		m_flagSpcSrcMesh = false ;
	}
}

// 出力メッシュを設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFilterMeshWarp::SetDestinationMesh( const S2DVector * pvMesh )
{
	if ( pvMesh != NULL )
	{
		size_t	countVertex = (m_sizeInMesh.w + 1) * (m_sizeInMesh.h + 1) ;
		m_meshDst.SetLength( countVertex ) ;
		eslMoveMemory
			( m_meshDst.GetArray(),
				pvMesh, countVertex * sizeof(S2DVector) ) ;
		m_meshDst.FinishArray() ;
		//
		S2DVector	vMin = *(pvMesh ++) ;
		S2DVector	vMax = vMin ;
		for ( size_t i = 1; i < countVertex; i ++ )
		{
			float32_t	x = pvMesh->x ;
			float32_t	y = pvMesh->y ;
			if ( x < vMin.x )
			{
				vMin.x = x ;
			}
			else if ( x > vMax.x )
			{
				vMax.x  = x ;
			}
			if ( y < vMin.y )
			{
				vMin.y = y ;
			}
			else if ( y > vMax.y )
			{
				vMax.y  = y ;
			}
			pvMesh ++ ;
		}
		m_rectDstMesh.x = (int32_t) floor( vMin.x ) ;
		m_rectDstMesh.y = (int32_t) floor( vMin.y ) ;
		m_rectDstMesh.w =
			(int32_t) floor( vMax.x + 1.0 ) - m_rectDstMesh.x ;
		m_rectDstMesh.h =
			(int32_t) floor( vMax.y + 1.0 ) - m_rectDstMesh.y ;
	}
}

// 描画方式設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFilterMeshWarp::SetDynamicDrawMesh( bool fDynamic )
{
	m_flagDynamicDraw = fDynamic ;
}

// メッシュの縁を固定する
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFilterMeshWarp::FixMeshBorder( bool fFix )
{
	m_flagFixBorder = fFix ;
}

// エフェクタ追加
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFilterMeshWarp::AddEffector
		( SGLSpriteFilterMeshWarp::Effector * pEffector )
{
	if ( pEffector != NULL )
	{
		pEffector->OnAttachedMesh
			( (size_t) m_sizeInMesh.w, (size_t) m_sizeInMesh.h,
				(size_t) m_sizeInPixels.w, (size_t) m_sizeInPixels.h ) ;
		m_effectors.Add( pEffector ) ;
	}
}

// 描画
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFilterMeshWarp::Draw
	( S3DRenderContextInterface& render,
		const SGLPaintParam& pp, SGLImageObject* image )
{
	if ( m_flagDynamicDraw )
	{
		const S2DVector * pSrcMesh = NULL ;
		if ( m_flagSpcSrcMesh )
		{
			pSrcMesh = m_meshSrc.GetConstArray() ;
		}
		SGLPaintParam	ppParam = pp ;
		if ( m_paramFilter2 > 0 )
		{
			if ( (m_paramFilter2 < 0x100)
				&& (ppParam.nTransparency < 0x100) )
			{
				ppParam.nTransparency =
					0x100 - (0x100 - m_paramFilter2)
							* (0x100 - ppParam.nTransparency) / 0x100 ;
			}
			else
			{
				return ;
			}
		}
		render.DrawMesh
			( m_meshDst.GetArray(), pSrcMesh,
				(size_t) m_sizeInMesh.w,
				(size_t) m_sizeInMesh.h, ppParam, image ) ;
		m_meshDst.FinishArray() ;
	}
	else
	{
		SGLSpriteFilter::Draw( render, pp, image ) ;
	}
}

// 描画域取得
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteFilterMeshWarp::GetRectangle
	( SGLImageRect& rectDraw, SGLImageObject* image ) const
{
	rectDraw = m_rectDstMesh ;
	return	true ;
}

// 動的描画フィルタか？
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteFilterMeshWarp::IsDynamicDrawer( void ) const
{
	return	m_flagDynamicDraw ;
}

// フィルタ処理
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFilterMeshWarp::Filter
	( S3DRenderContextInterface& render, SGLImageObject* image )
{
	if ( m_flagDynamicDraw
		|| (image == NULL) || (m_sizeInMesh.w * m_sizeInMesh.h == 0) )
	{
		return ;
	}
	//
	// 入力画像複製
	//
	SGLImageInfo	infSrc, infTemp ;
	if ( image->GetImageInfo( infSrc ) )
	{
		return ;
	}
	if ( m_imgFilterBuffer.GetImageInfo( infTemp )
		|| (infSrc.format != infTemp.format)
		|| (infSrc.width != infTemp.width)
		|| (infSrc.height != infTemp.height)
		|| (infSrc.depth != infTemp.depth) )
	{
		m_imgFilterBuffer.CreateBuffer( infSrc ) ;
	}
	SGLImageBuffer	imgSrc, imgTemp ;
	imgTemp.ptrBuffer = m_imgFilterBuffer.LockBuffer( imgTemp ) ;
	imgSrc.ptrBuffer =
			image->LockBuffer( imgSrc, SGLImageObject::lockRead ) ;
	sglCopyImageBuffer( imgTemp, imgSrc ) ;
	m_imgFilterBuffer.UnlockBuffer() ;
	image->UnlockBuffer( SGLImageObject::lockRead ) ;
	//
	// メッシュワープ描画
	//
	render.AttachTargetImage( image, NULL, NULL ) ;
	render.FillClearTarget( 0 ) ;
	//
	const S2DVector * pSrcMesh = NULL ;
	if ( m_flagSpcSrcMesh )
	{
		pSrcMesh = m_meshSrc.GetConstArray() ;
	}
	SGLPaintParam	pp ;
	pp.nTransparency = m_paramFilter2 ;
	render.DrawMesh
		( m_meshDst.GetArray(), pSrcMesh,
			(size_t) m_sizeInMesh.w,
			(size_t) m_sizeInMesh.h, pp, &m_imgFilterBuffer ) ;
	m_meshDst.FinishArray() ;
	//
	render.DetachTargetImage() ;
}

// タイマー処理
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFilterMeshWarp::OnTimer( SGLSprite& sprite, uint32_t msecPast )
{
	//
	// エフェクタ時間経過処理
	//
	Effector**	ppEffectors = m_effectors.GetArray() ;
	size_t		nEffectors = m_effectors.GetLength() ;
	size_t		i ;
	for ( i = 0; i < nEffectors; i ++ )
	{
		Effector *	pEffector = ppEffectors[i] ;
		if ( pEffector != NULL )
		{
			if ( pEffector->OnTimer( sprite, *this, msecPast ) )
			{
				delete	pEffector ;
				ppEffectors[i] = NULL ;
			}
		}
	}
	m_effectors.FinishArray() ;
	m_effectors.TrimEmpty() ;
	//
	// メッシュ初期値
	//
	SetFilteredMesh() ;
	//
	// メッシュ変位
	//
	const S2DVector *	pvSrcMesh = m_meshSrc.GetConstArray() ;
	S2DVector *			pvDstMesh = m_meshDst.GetArray() ;
	SGLSize				sizeInMesh = m_sizeInMesh ;
	nEffectors = m_effectors.GetLength() ;
	ppEffectors = m_effectors.GetArray() ;
	for ( i = 0; i < nEffectors; i ++ )
	{
		Effector *	pEffector = ppEffectors[i] ;
		if ( pEffector != NULL )
		{
			pEffector->WarpMesh
				( *this, pvDstMesh, pvSrcMesh,
					(size_t) sizeInMesh.w, (size_t) sizeInMesh.h ) ;
		}
	}
	m_meshDst.FinishArray() ;
	m_effectors.FinishArray() ;
	//
	if ( m_flagFixBorder )
	{
		CopyMeshBorderFromSource() ;
	}
	if ( nEffectors > 0 )
	{
		if ( m_flagDynamicDraw )
		{
			sprite.NotifyUpdate() ;
		}
		else
		{
			sprite.PostUpdate() ;
		}
	}
}

// フィルタパラメータに応じたメッシュ頂点設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFilterMeshWarp::SetFilteredMesh( void )
{
	if ( m_sizeInMesh.w * m_sizeInMesh.h == 0 )
	{
		return ;
	}
	S2DVector *	pvSrcMesh = m_meshSrc.GetArray() ;
	S2DVector *	pvDstMesh = m_meshDst.GetArray() ;
	SGLSize		sizeInMesh = m_sizeInMesh ;
	if ( !m_flagSpcSrcMesh )
	{
		S2DVector *	pvSrcNext = pvSrcMesh ;
		double	yPos = 0 ;
		double	xMesh = (double) m_sizeInPixels.w / sizeInMesh.w ;
		double	yMesh = (double) m_sizeInPixels.h / sizeInMesh.h ;
		for ( int y = 0; y <= sizeInMesh.h; y ++, yPos += yMesh )
		{
			double	xPos = 0 ;
			for ( int x = 0; x <= sizeInMesh.w; x ++, xPos += xMesh )
			{
				pvSrcNext->x = (float32_t) xPos ;
				pvSrcNext->y = (float32_t) yPos ;
				++ pvSrcNext ;
			}
		}
	}
	//
	size_t	countVertex = (m_sizeInMesh.w + 1) * (m_sizeInMesh.h + 1) ;
	ESLAssert( m_meshSrc.GetLength() >= countVertex ) ;
	ESLAssert( m_meshDst.GetLength() >= countVertex ) ;
	eslMoveMemory
		( pvDstMesh, pvSrcMesh, countVertex * sizeof(S2DVector) ) ;
	//
	m_meshSrc.FinishArray() ;
	m_meshDst.FinishArray() ;
}

// メッシュの縁を描画元座標に一致させる
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteFilterMeshWarp::CopyMeshBorderFromSource( void )
{
	if ( m_sizeInMesh.w * m_sizeInMesh.h == 0 )
	{
		return ;
	}
	const S2DVector *	pvSrcMesh = m_meshSrc.GetConstArray() ;
	S2DVector *			pvDstMesh = m_meshDst.GetArray() ;
	SGLSize		sizeInMesh = m_sizeInMesh ;
	sizeInMesh.w ++ ;
	//
	eslMoveMemory
		( pvDstMesh, pvSrcMesh, sizeInMesh.w * sizeof(S2DVector) ) ;
	pvDstMesh += sizeInMesh.w ;
	pvSrcMesh += sizeInMesh.w ;
	//
	for ( int y = 1; y < sizeInMesh.h; y ++ )
	{
		pvDstMesh[0] = pvSrcMesh[0] ;
		pvDstMesh += sizeInMesh.w ;
		pvSrcMesh += sizeInMesh.w ;
		pvDstMesh[-1] = pvSrcMesh[-1] ;
	}
	eslMoveMemory
		( pvDstMesh, pvSrcMesh, sizeInMesh.w * sizeof(S2DVector) ) ;
	//
	m_meshDst.FinishArray() ;
}

// 複製
//////////////////////////////////////////////////////////////////////////////
SGLObject * SGLSpriteFilterMeshWarp::DuplicateObject( void )
{
	return	new SGLSpriteFilterMeshWarp( *this ) ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteFilterMeshWarp::OnSave( SSystem::SFileInterface& file )
{
	SGLError	err = SGLSpriteFilter::OnSave( file ) ;
	if ( err )
	{
		return	err ;
	}
	uint32_t	nFlags = 0 ;
	if ( m_flagSpcSrcMesh )
	{
		nFlags |= 0x01 ;
	}
	if ( m_flagDynamicDraw )
	{
		nFlags |= 0x02 ;
	}
	if ( m_flagFixBorder )
	{
		nFlags |= 0x04 ;
	}
	file.Write( &nFlags, sizeof(uint32_t) ) ;
	file.Write( &m_sizeInPixels, sizeof(SGLSize) ) ;
	file.Write( &m_sizeInMesh, sizeof(SGLSize) ) ;
	file.Write( &m_rectDstMesh, sizeof(SGLImageRect) ) ;
	if ( m_flagSpcSrcMesh )
	{
		uint32_t	nSrcLen = (uint32_t) m_meshSrc.GetLength() ;
		file.Write( &nSrcLen, sizeof(uint32_t) ) ;
		if ( nSrcLen > 0 )
		{
			file.Write( m_meshSrc.GetConstArray(), nSrcLen * sizeof(S2DVector) ) ;
		}
	}
	uint32_t	nDstLen = (uint32_t) m_meshDst.GetLength() ;
	file.Write( &nDstLen, sizeof(uint32_t) ) ;
	if ( nDstLen > 0 )
	{
		file.Write( m_meshDst.GetConstArray(), nDstLen * sizeof(S2DVector) ) ;
	}
	return	SaveObjectArray<Effector>( file, m_effectors ) ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteFilterMeshWarp::OnRestore( SSystem::SFileInterface& file )
{
	SGLError	err = SGLSpriteFilter::OnRestore( file ) ;
	if ( err )
	{
		return	err ;
	}
	uint32_t	nFlags = 0 ;
	file.Read( &nFlags, sizeof(uint32_t) ) ;
	m_flagSpcSrcMesh = ((nFlags & 0x01) != 0) ;
	m_flagDynamicDraw = ((nFlags & 0x02) != 0) ;
	m_flagFixBorder = ((nFlags & 0x04) != 0) ;
	//
	file.Read( &m_sizeInPixels, sizeof(SGLSize) ) ;
	file.Read( &m_sizeInMesh, sizeof(SGLSize) ) ;
	file.Read( &m_rectDstMesh, sizeof(SGLImageRect) ) ;
	//
	if ( m_flagSpcSrcMesh )
	{
		uint32_t	nSrcLen = 0 ;
		if ( file.Read( &nSrcLen, sizeof(uint32_t) ) < sizeof(uint32_t) )
		{
			return	sglErrFailed ;
		}
		m_meshSrc.SetLength( nSrcLen ) ;
		file.Read( m_meshSrc.GetArray(), nSrcLen * sizeof(S2DVector) ) ;
		m_meshSrc.FinishArray() ;
	}
	uint32_t	nDstLen = 0 ;
	if ( file.Read( &nDstLen, sizeof(uint32_t) ) < sizeof(uint32_t) )
	{
		return	sglErrFailed ;
	}
	m_meshDst.SetLength( nDstLen ) ;
	if ( !m_flagSpcSrcMesh )
	{
		m_meshSrc.SetLength( nDstLen ) ;
	}
	file.Read( m_meshDst.GetArray(), nDstLen * sizeof(S2DVector) ) ;
	m_meshDst.FinishArray() ;
	//
	return	LoadObjectArray<Effector>( file, m_effectors ) ;
}


//////////////////////////////////////////////////////////////////////////////
// メッシュハイライト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSpriteMeshHighlight, SGLSprite )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteMeshHighlight::SGLSpriteMeshHighlight( void )
	: m_sizeInPixels( 0, 0 ), m_sizeInMesh( 0, 0 ), m_vLight( 1, 1, 1 )
{
	S3DSurfaceAttribute	attr ;
	attr.flagsShading = shadingMethodPhong | shadingNoZBuffer ;
	attr.colorBase.rgbMul = 0x00FFFFFF ;
	attr.colorBase.rgbAdd = 0 ;
	attr.nAmbient = 0x100 ;
	attr.nDiffusion = 0 ;
	attr.nSpecular = 0x100 ;
	attr.nSpecularSize = 0x40 ;
	attr.nTransparency = 0 ;
	attr.nDeepness = 0 ;
	attr.nReflection = 0 ;
	attr.fpRefraction = 0.0f ;
	m_material.SetSurfaceAttribute( attr ) ;
}

SGLSpriteMeshHighlight::SGLSpriteMeshHighlight( const SGLSpriteMeshHighlight& smh )
	: m_bufMesh( smh.m_bufMesh ),
		m_sizeInPixels( smh.m_sizeInMesh ),
		m_sizeInMesh( smh.m_sizeInMesh ), m_vLight( smh.m_vLight )
{
	S3DSurfaceAttribute	attr ;
	attr.flagsShading = shadingMethodGouraud | shadingNoZBuffer ;
	attr.colorBase.rgbMul = 0x00FFFFFF ;
	attr.colorBase.rgbAdd = 0 ;
	attr.nAmbient = 0x100 ;
	attr.nDiffusion = 0 ;
	attr.nSpecular = 0x100 ;
	attr.nSpecularSize = 0x40 ;
	attr.nTransparency = 0 ;
	attr.nDeepness = 0 ;
	attr.nReflection = 0 ;
	attr.fpRefraction = 0.0f ;
	m_material.SetSurfaceAttribute( attr ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteMeshHighlight::~SGLSpriteMeshHighlight( void )
{
}

// メッシュサイズ設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMeshHighlight::SetMeshSize( size_t width, size_t height )
{
	m_sizeInMesh.w = (int32_t) width ;
	m_sizeInMesh.h = (int32_t) height ;
}

// 画像サイズ設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMeshHighlight::SetCanvasSize( size_t width, size_t height )
{
	m_sizeInPixels.w = (int32_t) width ;
	m_sizeInPixels.h = (int32_t) height ;
	//
	if ( !IsBuffered() || (GetImageSize() != m_sizeInPixels) )
	{
		CreateBuffer
			( (uint32_t) width, (uint32_t) height, formatImageABGR, 32 ) ;
		SetFillBackColor( 0, false ) ;
	}
}

// メッシュサイズ取得
//////////////////////////////////////////////////////////////////////////////
const SGLSize& SGLSpriteMeshHighlight::GetMeshSize( void ) const
{
	return	m_sizeInMesh ;
}

// 画像サイズ取得
//////////////////////////////////////////////////////////////////////////////
const SGLSize& SGLSpriteMeshHighlight::GetCanvasSize( void ) const
{
	return	m_sizeInPixels ;
}

// 出力メッシュを設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMeshHighlight::SetMesh( const S3DVector4 * pvMesh )
{
	m_bufMesh.RemoveAll() ;
	m_bufMesh.AddArray
		( pvMesh, (size_t) ((m_sizeInMesh.w + 1) * (m_sizeInMesh.h + 1)) ) ;
}

// 光源ベクトル設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMeshHighlight::SetLight( const S3DVector& vLight )
{
	m_vLight = vLight ;
}

// 描画前処理
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMeshHighlight::BeforeDraw( SGLSprite::Stereo3DView s3dView )
{
	SGLSprite::BeforeDraw( s3dView ) ;
	//
	m_vboBuffer.ClearBuffer() ;
	m_meshShaper.GridMesh
		( m_vboBuffer, &m_material, 0,
			(size_t) m_sizeInMesh.w, (size_t) m_sizeInMesh.h,
			m_bufMesh.GetConstArray(), NULL, NULL, NULL ) ;
}

// 子スプライトを描画
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMeshHighlight::DrawChildren
	( S3DRenderContextInterface& render, SGLSprite::Stereo3DView s3dView ) const
{
	S3DDMatrix	matI( 1, 0, 0,  0, 1, 0,  0, 0, 1 ) ;
	S3DDVector	posZ( 0, 0, 0 ) ;
	render.SetMatrixTransformation( matI, posZ ) ;
	//
	S3DVector	vScreen
		( m_sizeInPixels.w * 0.5,
			m_sizeInPixels.h * 0.5,
			m_sizeInPixels.w + m_sizeInPixels.h ) ;
	render.SetProjectionScreen( vScreen ) ;
	render.SetZClipRange( vScreen.z * 0.25, vScreen.z * 2.0 ) ;
	//
	S3DDVector	vCameraTarget( vScreen.x, vScreen.y, 0 ) ;
	S3DDVector	vCameraPos( vScreen.x, vScreen.y, - vScreen.z ) ;
	S3DDVector	vCameraTop( 0, -1, 0 ) ;
	render.SetCameraAngleVector( vCameraTarget, vCameraPos, vCameraTop ) ;
	//
	S3DLightEntry	lights[1] ;
	lights[0].typeLight = lightTypeVector ;
	lights[0].rgbColor = 0x00FFFFFF ;
	lights[0].fpBrightness = 1.0f ;
	lights[0].vecDirection.x = m_vLight.x ;
	lights[0].vecDirection.y = m_vLight.y ;
	lights[0].vecDirection.z = m_vLight.z ;
	render.SetLightEntries( &lights[0], 1 ) ;
	//
	uint64_t	nSaveShading = render.GetShadingFlag() ;
	render.SetShadingFlag( shadingMethodPhong ) ;
	render.Begin3DRenderer() ;
	render.FillClearTarget( 0 ) ;
	//
	m_vboBuffer.RenderBufferTo( &render ) ;
	//
	render.End3DRenderer() ;
	render.Finish() ;
	render.SetShadingFlag( nSaveShading ) ;
	//
	SGLSprite::DrawChildren( render, s3dView ) ;
}

// 複製
//////////////////////////////////////////////////////////////////////////////
SGLObject * SGLSpriteMeshHighlight::DuplicateObject( void )
{
	return	new SGLSpriteMeshHighlight( *this ) ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteMeshHighlight::OnSave( SSystem::SFileInterface& file )
{
	SGLError	err = SGLSprite::OnSave( file ) ;
	if ( err )
	{
		return	err ;
	}
	file.Write( &m_sizeInPixels, sizeof(SGLSize) ) ;
	file.Write( &m_sizeInMesh, sizeof(SGLSize) ) ;
	file.Write( &m_vLight, sizeof(S3DVector) ) ;
	//
	uint32_t	nLength = (uint32_t) m_bufMesh.GetLength() ;
	file.Write( &nLength, sizeof(uint32_t) ) ;
	file.Write
		( m_bufMesh.GetConstArray(),
			m_bufMesh.GetLength() * sizeof(S3DVector4) ) ;
	//
	return	sglErrSuccess ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteMeshHighlight::OnRestore( SSystem::SFileInterface& file )
{
	SGLError	err = SGLSprite::OnRestore( file ) ;
	if ( err )
	{
		return	err ;
	}
	file.Read( &m_sizeInPixels, sizeof(SGLSize) ) ;
	file.Read( &m_sizeInMesh, sizeof(SGLSize) ) ;
	file.Read( &m_vLight, sizeof(S3DVector) ) ;
	//
	uint32_t	nLength ;
	if ( file.Read( &nLength, sizeof(uint32_t) ) < sizeof(uint32_t) )
	{
		return	sglErrFailed ;
	}
	file.Read
		( m_bufMesh.GetArray(nLength),
			nLength * sizeof(S3DVector4) ) ;
	m_bufMesh.FinishArray() ;
	//
	return	sglErrSuccess ;
}


//////////////////////////////////////////////////////////////////////////////
// メッシュ・エフェクタ
//////////////////////////////////////////////////////////////////////////////

// SGLSpriteWaveHighlight::Effector クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLSpriteWaveHighlight::Effector, SGLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteWaveHighlight::Effector::Effector( void )
{
}

SGLSpriteWaveHighlight::Effector::Effector
		( const SGLSpriteWaveHighlight::Effector& eff )
{
}

// 初期設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteWaveHighlight::Effector::OnAttachedMesh
		( size_t wMesh, size_t hMesh,
				size_t wCanvas, size_t hCanvas )
{
}

// 時間経過
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteWaveHighlight::Effector::OnTimer
	( SGLSpriteWaveHighlight& swh, uint32_t msecPast )
{
	return	true ;
}

// メッシュ頂点変位加算
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteWaveHighlight::Effector::WarpMesh
	( SGLSpriteWaveHighlight& swh,
		S3DVector4 * pvMesh, size_t wMesh, size_t hMesh )
{
	return	false ;
}


// SGLSpriteWaveHighlight::WaveEffector クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLSpriteWaveHighlight::WaveEffector, Effector )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteWaveHighlight::WaveEffector::WaveEffector( void )
{
	m_fpPhase = 0.0 ;
	m_fpAmplitude = 50.0 ;
	m_fpCycle = 8.0 ;
	m_fpSpeed = 50.0 ;
	//
	m_nWrapCount = 0 ;
	m_fpReach = 2000.0 ;
}

SGLSpriteWaveHighlight::WaveEffector::WaveEffector( const WaveEffector& eff )
	: Effector( eff )
{
	m_fpPhase = eff.m_fpPhase ;
	m_fpAmplitude = eff.m_fpAmplitude ;
	m_fpCycle = eff.m_fpCycle ;
	m_fpSpeed = eff.m_fpSpeed ;
}

// パラメータ設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteWaveHighlight::WaveEffector::SetParameter
	( const S2DDVector& center,
		double amp, double cycle, double speed )
{
	m_vCenter = center ;
	m_fpAmplitude = amp ;
	m_fpCycle = cycle ;
	m_fpSpeed = speed ;
}

// ラップアラウンド設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteWaveHighlight::WaveEffector::SetWrapRect
	( const SGLImageRect& rect, double fpReach, size_t nWrapCount )
{
	m_nWrapCount = nWrapCount ;
	m_rectWrap = rect ;
	m_fpReach = fpReach ;
}

// 初期設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteWaveHighlight::WaveEffector::OnAttachedMesh
		( size_t wMesh, size_t hMesh,
				size_t wCanvas, size_t hCanvas )
{
}

// 時間経過
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteWaveHighlight::WaveEffector::OnTimer
	( SGLSpriteWaveHighlight& swh, uint32_t msecPast )
{
	m_fpPhase += (double) msecPast * 0.001 ;
	if ( m_fpPhase > m_fpCycle )
	{
		m_fpPhase -= floor( m_fpPhase / m_fpCycle) * m_fpCycle ;
	}
	return	false ;
}

// メッシュ頂点変位加算
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteWaveHighlight::WaveEffector::WarpMesh
	( SGLSpriteWaveHighlight& swh,
		S3DVector4 * pvMesh, size_t wMesh, size_t hMesh )
{
	size_t	nCount = (wMesh + 1) * (hMesh + 1) ;
	double	fpLength = m_fpCycle * m_fpSpeed ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		int	nWrapCount = (int) m_nWrapCount ;
		for ( int y = - nWrapCount; y <= nWrapCount; y ++ )
		{
			for ( int x = - nWrapCount; x <= nWrapCount; x ++ )
			{
				double	dx = pvMesh[i].x - (m_vCenter.x + x * m_rectWrap.w) ;
				double	dy = pvMesh[i].y - (m_vCenter.y + y * m_rectWrap.h) ;
				double	dr = sqrt( dx * dx + dy * dy ) ;
				double	rad = 2.0 * PI * (dr / fpLength - m_fpPhase / m_fpCycle) ;
				double	s = m_fpAmplitude * sin( rad ) ;
				if ( m_nWrapCount > 0 )
				{
					if ( dr < m_fpReach )
					{
						s *= 1.0 - dr / m_fpReach ;
					}
					else
					{
						s = 0.0 ;
					}
				}
				pvMesh[i].z -= (float32_t) s ;
			}
		}
	}
	return	true ;
}

// 複製
//////////////////////////////////////////////////////////////////////////////
SGLObject * SGLSpriteWaveHighlight::WaveEffector::DuplicateObject( void )
{
	return	new WaveEffector( *this ) ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteWaveHighlight::WaveEffector::OnSave( SSystem::SFileInterface& file )
{
	Effector::OnSave( file ) ;
	//
	file.Write( &m_vCenter, sizeof(S2DDVector) ) ;
	file.Write( &m_fpPhase, sizeof(double) ) ;
	file.Write( &m_fpAmplitude, sizeof(double) ) ;
	file.Write( &m_fpCycle, sizeof(double) ) ;
	file.Write( &m_fpSpeed, sizeof(double) ) ;
	//
	return	sglErrSuccess ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteWaveHighlight::WaveEffector::OnRestore( SSystem::SFileInterface& file )
{
	Effector::OnRestore( file ) ;
	//
	file.Read( &m_vCenter, sizeof(S2DDVector) ) ;
	file.Read( &m_fpPhase, sizeof(double) ) ;
	file.Read( &m_fpAmplitude, sizeof(double) ) ;
	file.Read( &m_fpCycle, sizeof(double) ) ;
	file.Read( &m_fpSpeed, sizeof(double) ) ;
	//
	return	sglErrSuccess ;
}


//////////////////////////////////////////////////////////////////////////////
// 波紋ハイライト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSpriteWaveHighlight, SGLSprite )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteWaveHighlight::SGLSpriteWaveHighlight( void )
{
}

SGLSpriteWaveHighlight::SGLSpriteWaveHighlight
		( const SGLSpriteWaveHighlight& swh )
	: SGLSpriteMeshHighlight( swh ), m_effectors( swh.m_effectors )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteWaveHighlight::~SGLSpriteWaveHighlight( void )
{
}

// エフェクタ追加
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteWaveHighlight::AddEffector
		( SGLSpriteWaveHighlight::Effector * pEffector )
{
	pEffector->OnAttachedMesh
		( (size_t) m_sizeInMesh.w, (size_t) m_sizeInMesh.h,
			(size_t) m_sizeInPixels.w, (size_t) m_sizeInPixels.h ) ;
	m_effectors.Add( pEffector ) ;
}

// 時間経過処理
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteWaveHighlight::AdvanceTime( uint32_t msecPast )
{
	SGLSpriteMeshHighlight::AdvanceTime( msecPast ) ;
	//
	for ( size_t i = 0; i < m_effectors.GetLength(); i ++ )
	{
		Effector *	pEff = m_effectors.GetAt( i ) ;
		if ( pEff != NULL )
		{
			if ( pEff->OnTimer( *this, msecPast ) )
			{
				m_effectors.SetAt( i, NULL ) ;
			}
		}
	}
	m_effectors.TrimEmpty() ;
	//
	S3DVector4 *	pvMesh =
		m_bufMesh.GetArray
			( (size_t) ((m_sizeInMesh.w + 1) * (m_sizeInMesh.h + 1)) ) ;
	float32_t	mx =
		(float32_t) m_sizeInPixels.w / (float32_t) m_sizeInMesh.w ;
	float32_t	my =
		(float32_t) m_sizeInPixels.h / (float32_t) m_sizeInMesh.h ;
	//
	for ( int y = 0; y <= m_sizeInMesh.h; y ++ )
	{
		int	i = y * (m_sizeInMesh.w + 1) ;
		for ( int x = 0; x <= m_sizeInMesh.w; x ++ )
		{
			pvMesh[i + x].x = (float32_t) x * mx ;
			pvMesh[i + x].y = (float32_t) y * my ;
			pvMesh[i + x].z = 0 ;
		}
	}
	bool	fUpdate = false ;
	for ( size_t i = 0; i < m_effectors.GetLength(); i ++ )
	{
		Effector *	pEff = m_effectors.GetAt( i ) ;
		if ( pEff != NULL )
		{
			if ( pEff->WarpMesh
				( *this, pvMesh,
					(size_t) m_sizeInMesh.w, (size_t) m_sizeInMesh.h ) )
			{
				fUpdate = true ;
			}
		}
	}
	m_bufMesh.FinishArray() ;
	//
	if ( fUpdate )
	{
		PostUpdate( NULL ) ;
	}
}

// 描画前処理
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteWaveHighlight::BeforeDraw( SGLSprite::Stereo3DView s3dView )
{
	SGLSpriteMeshHighlight::BeforeDraw( s3dView ) ;
}

// 複製
//////////////////////////////////////////////////////////////////////////////
SGLObject * SGLSpriteWaveHighlight::DuplicateObject( void )
{
	return	new SGLSpriteWaveHighlight( *this ) ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteWaveHighlight::OnSave( SSystem::SFileInterface& file )
{
	SGLError	err = SGLSpriteMeshHighlight::OnSave( file ) ;
	if ( err )
	{
		return	err ;
	}
	return	SaveObjectArray<Effector>( file, m_effectors ) ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteWaveHighlight::OnRestore( SSystem::SFileInterface& file )
{
	SGLError	err = SGLSpriteMeshHighlight::OnSave( file ) ;
	if ( err )
	{
		return	err ;
	}
	return	LoadObjectArray<Effector>( file, m_effectors ) ;
}



