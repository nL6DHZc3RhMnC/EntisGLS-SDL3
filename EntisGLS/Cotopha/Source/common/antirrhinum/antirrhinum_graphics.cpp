
#include <antirrhinum/antirrhinum.h>
#include <sakura/ssys_smart_buffer.h>
#include <sakuraglx/sprite/sglx_sprite_filter.h>
#include <sakuraglx/sprite/sglx_sprite_movie.h>

using namespace	SSystem ;
using namespace	Rosetta ;
using namespace	SakuraGL ;
using namespace	AntirrhinumGL ;


//////////////////////////////////////////////////////////////////////////////
// グラフィック処理用レイヤー・アニメーション LayerMoveAction
//////////////////////////////////////////////////////////////////////////////

// クラス情報
SGL_IMPLEMENT_CLASS_INFO( AntirrhinumGL::AGLGraphicsLayer::LayerMoveAction, Action )

// 構築関数
AGLGraphicsLayer::LayerMoveAction::LayerMoveAction( void )
	: m_flagOffset( false )
{
}

AGLGraphicsLayer::LayerMoveAction::LayerMoveAction( const LayerMoveAction& act )
	: Action( act ), m_flagOffset( false )
{
}

// 設定
void AGLGraphicsLayer::LayerMoveAction::SetAnimation
	( const SSystem::SArray<MoveAnimeParam>& aAnime, bool flagOffset )
{
	m_aMoveAnime = aAnime ;
	m_flagOffset = flagOffset ;
	//
	if ( flagOffset )
	{
		m_maskModifyElement = flagParamPos | flagParamZoom
							| flagParamAngle | flagParamTransparency ;
	}
	else
	{
		m_maskSetElement = flagParamPos | flagParamZoom
							| flagParamAngle | flagParamTransparency ;
	}
	uint32_t	msecDuration = 0 ;
	for ( size_t i = 0; i < m_aMoveAnime.GetLength(); i ++ )
	{
		msecDuration += m_aMoveAnime.At(i).msecDuration ;
	}
	SetDuration( msecDuration ) ;
}

// パラメータ反映
void AGLGraphicsLayer::LayerMoveAction::EffectParameter
	( SGLSprite::Parameter& param, double t )
{
	uint32_t	msecTime = (uint32_t) (m_msecDuration * t) ;
	for ( size_t i = 1; i < m_aMoveAnime.GetLength(); i ++ )
	{
		const MoveAnimeParam&	map = m_aMoveAnime.At(i) ;
		if ( msecTime <= map.msecDuration )
		{
			const MoveAnimeParam&	map0 = m_aMoveAnime.At(i - 1) ;
			double	p = (map.msecDuration > 0)
							? (double) msecTime / map.msecDuration : 0.0 ;
			double	q = 1.0 - p ;
			double	r = map.fpSpeedStart * p * q * q ;
			r += (3.0 - map.fpSpeedEnd) * p * p * q ;
			r += p * p * p ;
			//
			double	s = 1.0 - r ;
			if ( m_flagOffset )
			{
				param.vDst.x += map0.vPosition.x * s + map.vPosition.x * r ;
				param.vDst.y += map0.vPosition.y * s + map.vPosition.y * r ;
				param.vZoom.x *= map0.vZoom.x * s + map.vZoom.x * r ;
				param.vZoom.y *= map0.vZoom.y * s + map.vZoom.y * r ;
				param.zAngle += map0.degRotate * s + map.degRotate * r ;
				//
				uint32_t	nTrans =
					(uint32_t) esl_lroundfi( map0.nTransparency * s + map.nTransparency * r ) ;
				param.nTransparency =
						(uint32_t) esl_clampi( (int) param.nTransparency, 0, 0x100 ) ;
				nTrans = (uint32_t) esl_clampi( (int) nTrans, 0, 0x100 ) ;
				//
				param.nTransparency =
					0x100 - (0x100 - param.nTransparency) * (0x100 - nTrans) ;
			}
			else
			{
				param.vDst.x = map0.vPosition.x * s + map.vPosition.x * r ;
				param.vDst.y = map0.vPosition.y * s + map.vPosition.y * r ;
				param.vZoom.x = map0.vZoom.x * s + map.vZoom.x * r ;
				param.vZoom.y = map0.vZoom.y * s + map.vZoom.y * r ;
				param.zAngle = map0.degRotate * s + map.degRotate * r ;
				param.nTransparency =
					(uint32_t) esl_lroundfi( map0.nTransparency * s + map.nTransparency * r ) ;
				param.paramFilter =
					(uint32_t) esl_lroundfi( map0.nFilterParam * s + map.nFilterParam * r ) ;
				param.paramFilter2 =
					(uint32_t) esl_lroundfi( map0.nFilterParam2 * s + map.nFilterParam2 * r ) ;
			}
			break ;
		}
		else
		{
			msecTime -= map.msecDuration ;
		}
	}
}

// 複製
SGLObject * AGLGraphicsLayer::LayerMoveAction::DuplicateObject( void )
{
	return	new LayerMoveAction( *this ) ;
}

// シリアライズ
SakuraGL::SGLError AGLGraphicsLayer::LayerMoveAction::OnSave( SSystem::SFileInterface& file )
{
	SGLError	err = Action::OnSave( file ) ;
	//
	uint32_t	nFlags = m_flagOffset ? 1 : 0 ;
	file.Write( &nFlags, sizeof(nFlags) ) ;
	//
	SaveArray<MoveAnimeParam>( file, m_aMoveAnime ) ;
	//
	return	err ;
}

// 復元
SakuraGL::SGLError AGLGraphicsLayer::LayerMoveAction::OnRestore( SSystem::SFileInterface& file )
{
	SGLError	err = Action::OnRestore( file ) ;
	//
	uint32_t	nFlags = 0 ;
	file.Read( &nFlags, sizeof(nFlags) ) ;
	m_flagOffset = ((nFlags & 0x01) != 0) ;
	//
	LoadArray<MoveAnimeParam>( file, m_aMoveAnime ) ;
	//
	return	err ;
}


//////////////////////////////////////////////////////////////////////////////
// グラフィック処理用レイヤー Sprite
//////////////////////////////////////////////////////////////////////////////

const wchar_t *	AGLGraphicsLayer::s_pwszEntitySpriteID = L"@sub" ;
const wchar_t *	AGLGraphicsLayer::s_pwszFadeoutSpriteID = L"$<fadeout>" ;
const int32_t	AGLGraphicsLayer::s_nEntityPriority = 1 ;
const int32_t	AGLGraphicsLayer::s_nFadeoutPriority = 0 ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( AntirrhinumGL::AGLGraphicsLayer, SakuraGL::SGLSprite )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
AGLGraphicsLayer::AGLGraphicsLayer( void )
	: m_flagNextParam( false )
{
}

AGLGraphicsLayer::AGLGraphicsLayer( const AGLGraphicsLayer& layer )
	: SGLSprite( layer ),
		m_style( layer.m_style ),
		m_flagNextParam( layer.m_flagNextParam ),
		m_gpNext( layer.m_gpNext ),
		m_aMoveAnime( layer.m_aMoveAnime )
{
}

// スタイル
//////////////////////////////////////////////////////////////////////////////
const AGLGraphicsLayer::LayerStyle& AGLGraphicsLayer::GetLayerStyle( void ) const
{
	return	m_style ;
}

void AGLGraphicsLayer::SetLayerStyle( const AGLGraphicsLayer::LayerStyle& style )
{
	SGLPoint	ptOffsetDelta = style.ptDstOffset - m_style.ptDstOffset ;
	m_style = style ;
	//
	S2DDVector	vCenter( style.ptSrcPivot.x, style.ptSrcPivot.y ) ;
	//
	LockTrace( __FILE__, __LINE__ ) ;
	S2DDVector	vOffsetCenter = vCenter - GetCenterPosition() ;
	S2DDVector	vPosition = GetPosition2D() ;
	vPosition.x += ptOffsetDelta.x ;
	vPosition.y += ptOffsetDelta.y ;
	//
	SetCenterPosition( vCenter.x, vCenter.y ) ;
	SetPosition( vPosition.x, vPosition.y ) ;
	m_gpNext.vPosition = vPosition ;
	Unlock() ;
}

void AGLGraphicsLayer::SetLayerStyle
	( SakuraGL::SGLSprite& sprite, const AGLGraphicsLayer::LayerStyle& style )
{
	SGLPoint	ptOffsetDelta = style.ptDstOffset ;
	S2DDVector	vCenter( style.ptSrcPivot.x, style.ptSrcPivot.y ) ;
	//
	sprite.LockTrace( __FILE__, __LINE__ ) ;
	S2DDVector	vOffsetCenter = vCenter - sprite.GetCenterPosition() ;
	S2DDVector	vPosition = sprite.GetPosition2D() + vOffsetCenter ;
	vPosition.x += ptOffsetDelta.x ;
	vPosition.y += ptOffsetDelta.y ;
	//
	sprite.SetCenterPosition( vCenter.x, vCenter.y ) ;
	sprite.SetPosition( vPosition.x, vPosition.y ) ;
	sprite.Unlock() ;
}

// フレームバッファ設定反映
//////////////////////////////////////////////////////////////////////////////
void AGLGraphicsLayer::ApplyFrameBufferStyle( SakuraGL::S3DRenderDevice * pDevice )
{
	ApplyFrameBufferStyle( *this, m_style, pDevice ) ;
}

void AGLGraphicsLayer::ApplyFrameBufferStyle
	( SakuraGL::SGLSprite& sprite,
		const AGLGraphicsLayer::LayerStyle& style,
		SakuraGL::S3DRenderDevice * pDevice )
{
	sprite.LockTrace( __FILE__, __LINE__ ) ;
	if ( (style.nType == typeBufferedLayer)
		|| (style.nType == typeFrameBufferVRAM)
		|| (style.nType == typeFrameBufferRAM)
		|| (style.nType == typeFrameBufferCPU) )
	{
		if ( !sprite.IsBuffered() || (sprite.GetImageSize() != style.sizeScreen) )
		{
			sprite.ReleaseBuffer() ;
			//
			int	nBufFlags = (style.nType == typeFrameBufferVRAM)
								? SGLImageObject::bufferOnDeviceOnly
								: SGLImageObject::bufferOnMemory ;
			sprite.CreateBuffer
				( style.sizeScreen.w, style.sizeScreen.h,
					formatImageDefaultRGBA, 32, nBufFlags ) ;
			//
			if ( (pDevice != NULL) && (style.nType != typeFrameBufferCPU) )
			{
				sprite.SetBufferRenderer( pDevice->NewRenderer() ) ;
			}
		}
	}
	else
	{
		sprite.ReleaseBuffer() ;
	}
	sprite.Unlock() ;
}

// 移動アニメの最後のパラメータか、現在のパラメータ取得
//////////////////////////////////////////////////////////////////////////////
AGLGraphicsLayer::GraphicsParam AGLGraphicsLayer::GetLastMovedParam( void ) const
{
	if ( m_flagNextParam )
	{
		return	m_gpNext ;
	}
	if ( m_aMoveAnime.GetLength() >= 1 )
	{
		MoveAnimeParam *	pmap = m_aMoveAnime.GetLastAt(0) ;
		if ( pmap != NULL )
		{
			return	*pmap ;
		}
	}
	return	GetCurrentParam() ;
}

// 現在の Sprite 表示状態を取得
//////////////////////////////////////////////////////////////////////////////
AGLGraphicsLayer::GraphicsParam AGLGraphicsLayer::GetCurrentParam( void ) const
{
	return	GetCurrentParam( *this ) ;
}

AGLGraphicsLayer::GraphicsParam
	AGLGraphicsLayer::GetCurrentParam( const SakuraGL::SGLSprite& sprite )
{
	GraphicsParam	gp ;
	gp.vPosition = sprite.GetPosition2D() ;
	gp.vZoom = sprite.GetZoom() ;
	gp.degRotate = (float32_t) sprite.GetRotation() ;
	gp.nTransparency = sprite.GetTransparency() ;
	gp.nFilterParam = sprite.GetFilterParameter() ;
	gp.nFilterParam2 = sprite.GetFilter2Parameter() ;
	return	gp ;
}

// 現在の Sprite 表示状態を設定
//////////////////////////////////////////////////////////////////////////////
void AGLGraphicsLayer::SetCurrentParam( const AGLGraphicsLayer::GraphicsParam& param )
{
	SetCurrentParam( *this, param ) ;
	//
	if ( m_flagNextParam )
	{
		m_gpNext = param ;
	}
}

void AGLGraphicsLayer::SetCurrentParam
	( SakuraGL::SGLSprite& sprite, const AGLGraphicsLayer::GraphicsParam& param )
{
	sprite.SetPosition( param.vPosition.x, param.vPosition.y ) ;
	sprite.SetZoom( param.vZoom.x, param.vZoom.y ) ;
	sprite.SetRotation( param.degRotate ) ;
	sprite.SetTransparency( param.nTransparency ) ;
	sprite.SetFilterParameter( param.nFilterParam ) ;
	sprite.SetFilter2Parameter( param.nFilterParam2 ) ;
}

// 移動アニメ設定開始
//////////////////////////////////////////////////////////////////////////////
void AGLGraphicsLayer::PrepareMoveAnimation( bool flagOffset )
{
	m_aMoveAnime.RemoveAll() ;
	//
	MoveAnimeParam	map ;
	map.msecDuration = 0 ;
	//
	if ( !flagOffset )
	{
		map = GetLastMovedParam() ;
	}
	m_aMoveAnime.Add( map ) ;
}

// 移動アニメ追加
//////////////////////////////////////////////////////////////////////////////
void AGLGraphicsLayer::AddMoveAnimeParam( const AGLGraphicsLayer::MoveAnimeParam& param )
{
	m_aMoveAnime.Add( param ) ;
}

// 移動アニメ開始
//////////////////////////////////////////////////////////////////////////////
void AGLGraphicsLayer::StartMoveAnimation
		( SGLSprite::ActionType type, bool flagOffset )
{
	LockTrace( __FILE__, __LINE__ ) ;
	if ( m_aMoveAnime.GetLength() >= 2 )
	{
		FlushAction( actionOnce ) ;
		//
		LayerMoveAction *	pAction = new LayerMoveAction ;
		pAction->SetActionType( type ) ;
		pAction->SetAnimation( m_aMoveAnime, flagOffset ) ;
		//
		if ( flagOffset )
		{
			AddAction( pAction ) ;
		}
		else
		{
			InsertActionAt( 0, pAction ) ;
			//
			m_flagNextParam = true ;
			m_gpNext = *(m_aMoveAnime.GetLastAt(0)) ;
		}
		m_aMoveAnime.RemoveAll() ;
	}
	else if ( m_aMoveAnime.GetLength() == 1 )
	{
		if ( !flagOffset )
		{
			FlushAction( actionOnce ) ;
			//
			m_flagNextParam = true ;
			m_gpNext = m_aMoveAnime.At(0) ;
			//
			SetCurrentParam( m_gpNext ) ;
		}
		m_aMoveAnime.RemoveAll() ;
	}
	Unlock() ;
}

// 画像読み込み
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLError AGLGraphicsLayer::LoadLayerImage
	( const wchar_t * pwszFilePath, uint32_t msecFadeTime )
{
	if ( (m_style.nType == typeLayer)
		|| (m_style.nType == typeSubLayer) )
	{
		return	SGLSprite::LoadImage( pwszFilePath ) ;
	}
	else if ( m_style.nType == typeBufferedLayer )
	{
		AGLGraphicsLayer *	pSubLayer = new AGLGraphicsLayer ;
		LayerStyle	style = GetLayerStyle() ;
		style.nType = typeSubLayer ;
		pSubLayer->SetLayerStyle( style ) ;
		pSubLayer->SetID( s_pwszEntitySpriteID ) ;
		//
		if ( pSubLayer->LoadImage( pwszFilePath ) )
		{
			delete	pSubLayer ;
			return	sglErrFailed ;
		}
		LockTrace( __FILE__, __LINE__ ) ;
		AGLGraphicsLayer *	pFadeout = m_refFadeout ;
		if ( pFadeout != NULL )
		{
			RemoveChild( pFadeout ) ;
		}
		AGLGraphicsLayer *	pLastEntity = m_refEntity ;
		if ( pLastEntity != NULL )
		{
			pLastEntity->SetID( s_pwszFadeoutSpriteID ) ;
			pLastEntity->ChangePriority( s_nFadeoutPriority ) ;
			pLastEntity->ModifyDrawFlags( paintFunctionAdd, paintMaskFunction ) ;
			pLastEntity->FlushAction() ;
			pSubLayer->SetCurrentParam( pLastEntity->GetLastMovedParam() ) ;
		}
		m_refFadeout = pLastEntity ;
		//
		m_refEntity = pSubLayer ;
		pSubLayer->ChangePriority( s_nEntityPriority ) ;
		pSubLayer->SetTransparency( 0x100 ) ;
		pSubLayer->SetVisible( true ) ;
		pSubLayer->SetActionLinearTo( msecFadeTime, 0, NULL, NULL, 1.0, 1.0 ) ;
		AddSmartChild( pSubLayer ) ;
		//
		if ( pLastEntity != NULL )
		{
			pLastEntity->SetActionLinearTo( msecFadeTime, 0x100, NULL, NULL, 1.0, 1.0 ) ;
		}
		Unlock() ;
		return	sglErrSuccess ;
	}
	else
	{
		return	sglErrFailed ;
	}
}

// レイヤーアイテム取得
//////////////////////////////////////////////////////////////////////////////
SGLSprite * AGLGraphicsLayer::GetSpriteAs( const wchar_t * pwszLayerID )
{
	if ( (pwszLayerID == NULL) || (pwszLayerID[0] == 0) )
	{
		return	this ;
	}
	ssize_t	iSep = -1 ;
	for ( size_t i = 0; pwszLayerID[i] != 0; i ++ )
	{
		if ( pwszLayerID[i] == L'.' )
		{
			iSep = (ssize_t) i ;
			break ;
		}
	}
	if ( iSep < 0 )
	{
		return	GetItemAs( pwszLayerID ) ;
	}
	SGLSprite *	pSubSprite = GetItemAs( SString( pwszLayerID, iSep ) ) ;
	if ( pSubSprite == NULL )
	{
		return	NULL ;
	}
	AGLGraphicsLayer *	pLayer = ESLTypeCast<AGLGraphicsLayer>( pSubSprite ) ;
	if ( pLayer != NULL )
	{
		return	pLayer->GetSpriteAs( pwszLayerID + (iSep + 1) ) ;
	}
	else
	{
		return	pSubSprite->GetItemAs( pwszLayerID + (iSep + 1) ) ;
	}
}

AGLGraphicsLayer * AGLGraphicsLayer::GetLayerAs( const wchar_t * pwszLayerID )
{
	return	ESLTypeCast<AGLGraphicsLayer>( GetSpriteAs( pwszLayerID ) ) ;
}

// レイヤー移動／フェード・アニメーション中か？
//////////////////////////////////////////////////////////////////////////////
bool AGLGraphicsLayer::IsPendingLayerAnimation
				( bool flagAllLayers, bool flagAllAction ) const
{
	if ( flagAllAction )
	{
		if ( IsAction() )
		{
			return	true ;
		}
	}
	else
	{
		if ( IsAction( actionOnce ) )
		{
			return	true ;
		}
	}
	if ( !flagAllLayers )
	{
		return	false ;
	}
	LockTrace( __FILE__, __LINE__ ) ;
	bool			flagAnimation = false ;
	const size_t	nCount = GetChildCount() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		SGLSprite *	pChild = GetChildAt( i ) ;
		AGLGraphicsLayer *
			pLayer = ESLTypeCast<AGLGraphicsLayer>( pChild ) ;
		if ( pLayer != NULL )
		{
			if ( pLayer->IsPendingLayerAnimation
						( flagAllLayers, flagAllAction ) )
			{
				flagAnimation = true ;
				break ;
			}
		}
		else if ( pChild != NULL )
		{
			if ( flagAllAction )
			{
				if ( pChild->IsAction() )
				{
					flagAnimation = true ;
					break ;
				}
			}
			else
			{
				if ( pChild->IsAction( actionOnce ) )
				{
					flagAnimation = true ;
					break ;
				}
			}
		}
	}
	Unlock() ;
	return	flagAnimation ;
}

// レイヤー移動／フェード・アニメーション即時完了
//////////////////////////////////////////////////////////////////////////////
void AGLGraphicsLayer::FlushLayerAnimation
			( bool flagAllLayers, bool flagAllAction )
{
	LockTrace( __FILE__, __LINE__ ) ;
	if ( flagAllAction )
	{
		FlushAction() ;
	}
	else
	{
		FlushAction( actionOnce ) ;
	}
	//
	if ( flagAllLayers )
	{
		const size_t	nCount = GetChildCount() ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			SGLSprite *	pChild = GetChildAt( i ) ;
			AGLGraphicsLayer *
				pLayer = ESLTypeCast<AGLGraphicsLayer>( pChild ) ;
			if ( pLayer != NULL )
			{
				pLayer->FlushLayerAnimation( flagAllLayers, flagAllAction ) ;
			}
			else if ( pChild != NULL )
			{
				if ( flagAllAction )
				{
					pChild->FlushAction() ;
				}
				else
				{
					pChild->FlushAction( actionOnce ) ;
				}
			}
		}
	}
	Unlock() ;
}

// 時間経過処理
//////////////////////////////////////////////////////////////////////////////
void AGLGraphicsLayer::AdvanceTime( uint32_t msecPast )
{
	SGLSprite::AdvanceTime( msecPast ) ;
	//
	AGLGraphicsLayer *	pFadeout = m_refFadeout ;
	if ( pFadeout != NULL )
	{
		if ( !pFadeout->IsPendingLayerAnimation() )
		{
			RemoveChild( pFadeout ) ;
			m_refFadeout = NULL ;
		}
	}
}

// 複製（可能なら）
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLObject * AGLGraphicsLayer::DuplicateObject( void )
{
	return	new AGLGraphicsLayer( *this ) ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLError AGLGraphicsLayer::OnSave( SSystem::SFileInterface& file )
{
	SGLError	err = SGLSprite::OnSave( file ) ;
	//
	SaveStructure<LayerStyle>( file, m_style ) ;
	//
	uint32_t	nFlags = m_flagNextParam ? 1 : 0 ;
	file.Write( &nFlags, sizeof(nFlags) ) ;
	//
	SaveStructure<GraphicsParam>( file, m_gpNext ) ;
	//
	SaveArray<MoveAnimeParam>( file, m_aMoveAnime ) ;
	//
	return	err ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLError AGLGraphicsLayer::OnRestore( SSystem::SFileInterface& file )
{
	SGLError	err = SGLSprite::OnRestore( file ) ;
	//
	LoadStructure<LayerStyle>( file, m_style ) ;
	//
	uint32_t	nFlags = 0 ;
	file.Read( &nFlags, sizeof(nFlags) ) ;
	m_flagNextParam = ((nFlags & 0x01) != 0) ;
	//
	LoadStructure<GraphicsParam>( file, m_gpNext ) ;
	//
	LoadArray<MoveAnimeParam>( file, m_aMoveAnime ) ;
	//
	m_refEntity = GetLayerAs( s_pwszEntitySpriteID ) ;
	m_refFadeout = GetLayerAs( s_pwszFadeoutSpriteID ) ;
	//
	return	err ;
}



//////////////////////////////////////////////////////////////////////////////
// グラフィック処理
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( AntirrhinumGL::AGLGraphicsProcessor, AGLEpicFuncProcessor )
AGL_IMPLEMENT_EPIC_PROCESSOR( AntirrhinumGL::AGLGraphicsProcessor )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
AGLGraphicsProcessor::AGLGraphicsProcessor( void )
	: AGLEpicFuncProcessor( m_pFirstFuncDesc, L"graphics" ),
		m_pDevice( NULL ), m_pRootScreen( NULL ), m_nBasePriority( 100 )
{
	m_styleDef.nType = AGLGraphicsLayer::typeFrameBufferRAM ;
	m_flagCfgInitScreen = false ;
	m_iCfgInitScreen = 0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
AGLGraphicsProcessor::~AGLGraphicsProcessor( void )
{
}

// 表示設定
//////////////////////////////////////////////////////////////////////////////
void AGLGraphicsProcessor::SetDisplaySettings
	( SakuraGL::S3DRenderDevice * pDevice,
		SakuraGL::SGLSprite * pRootScreen, int32_t nBasePriority )
{
	m_pDevice = pDevice ;
	m_pRootScreen = pRootScreen ;
	m_nBasePriority = nBasePriority ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SSystem::SError AGLGraphicsProcessor::Serialize
		( SSystem::SXMLDocument& xmlTag, AGLKernel * pKernel )
{
	SSmartLock<SGLSprite>	lock( m_pRootScreen ) ;
	if ( m_pScreen != NULL )
	{
		ssize_t	iScreen = m_aScreens.FindPtr( m_pScreen ) ;
		xmlTag.GetAttrIntegerAs( L"current_screen", iScreen ) ;
	}
	SSmartBuffer	sbufFile ;
	SByteBuffer		bbufTemp ;
	SString			strBase64 ;
	for ( size_t i = 0; i < m_aScreens.GetLength(); i ++ )
	{
		AGLGraphicsLayer *	pScreen = m_aScreens.GetAt( i ) ;
		if ( pScreen == NULL )
		{
			continue ;
		}
		sbufFile.Seek( 0 ) ;
		sbufFile.SetLength( 0 ) ;
		pScreen->OnSave( sbufFile ) ;
		//
		sbufFile.Seek( 0 ) ;
		bbufTemp.Seek( 0 ) ;
		bbufTemp.SetEndOfFile() ;
		bbufTemp.ReadFromFile( sbufFile ) ;
		//
		SStringParser::EncodeBase64String
			( strBase64, bbufTemp.GetConstArray(),
						(size_t) bbufTemp.GetLength() ) ;
		//
		SXMLDocument *	pxmlScreen = new SXMLDocument ;
		pxmlScreen->SetTag( L"screen" ) ;
		pxmlScreen->SetAttrIntegerAs( L"index", i ) ;
		pxmlScreen->SetAttributeAs( L"base64", strBase64 ) ;
	}
	return	errSuccess ;
}

// デシリアライズ
//////////////////////////////////////////////////////////////////////////////
SSystem::SError AGLGraphicsProcessor::Deserialize
		( const SSystem::SXMLDocument& xmlTag, AGLKernel * pKernel )
{
	ReleaseGame() ;
	//
	SByteBuffer		bbufTemp ;
	for ( size_t i = 0; i < xmlTag.GetElementsCount(); i ++ )
	{
		SXMLDocument *	pxmlScreen = xmlTag.GetElementAt(i) ;
		if ( (pxmlScreen == NULL)
			|| (pxmlScreen->GetTag() != L"screen") )
		{
			continue ;
		}
		const SString *	pstrBase64 = pxmlScreen->GetAttributeAs( L"base64" ) ;
		if ( pstrBase64 == NULL )
		{
			continue ;
		}
		SStringParser::DecodeBase64String
			( bbufTemp, *pstrBase64, (ssize_t) pstrBase64->GetLength() ) ;
		//
		AGLGraphicsLayer *	pScreen = new AGLGraphicsLayer ;
		bbufTemp.Seek( 0) ;
		pScreen->OnRestore( bbufTemp ) ;
		//
		m_aScreens.SetAt
			( (size_t) pxmlScreen->GetAttrIntegerAs( L"index", 0 ), pScreen ) ;
	}
	m_pScreen =
		m_aScreens.GetAt
			( (size_t) xmlTag.GetAttrIntegerAs( L"current_screen", -1 ) ) ;
	return	errSuccess ;
}

// デシリアライズ後の参照解決処理
//////////////////////////////////////////////////////////////////////////////
SSystem::SError AGLGraphicsProcessor::AfterDeserialize( AGLKernel * pKernel )
{
	SSmartLock<SGLSprite>	lock( m_pRootScreen ) ;
	for ( size_t i = 0; i < m_aScreens.GetLength(); i ++ )
	{
		AGLGraphicsLayer *	pScreen = m_aScreens.GetAt( i ) ;
		if ( pScreen == NULL )
		{
			continue ;
		}
		pScreen->OnAfterRestore() ;
		//
		if ( m_pRootScreen != NULL )
		{
			m_pRootScreen->AddChild( pScreen ) ;
		}
	}
	return	errSuccess ;
}

// 設定
//////////////////////////////////////////////////////////////////////////////
void AGLGraphicsProcessor::LoadConfiguration
	( const SSystem::SXMLDocument& xmlConfig )
{
	const SXMLDocument *	pxmlGraphics =
				xmlConfig.GetElementTagAs( L"graphics" ) ;
	if ( pxmlGraphics != NULL )
	{
		const SXMLDocument *
			pxmlStyles = pxmlGraphics->GetElementTagAs( L"styles" ) ;
		if ( pxmlStyles != NULL )
		{
			m_ssoaStyles.RemoveAll() ;
			//
			ParseLayerStyle( m_styleDef, *pxmlStyles ) ;
			m_ssoaStyles.SetAs( L"", m_styleDef ) ;
			//
			for ( size_t i = 0; i < pxmlStyles->GetElementsCount(); i ++ )
			{
				SXMLDocument *	pxmlTag = pxmlStyles->GetElementAt( i ) ;
				if ( (pxmlTag == NULL)
					|| (pxmlTag->GetTag() != L"style") )
				{
					continue ;
				}
				const SString *	pstrID = pxmlTag->GetAttributeAs( L"id" )  ;
				if ( pstrID == NULL )
				{
					continue ;
				}
				AGLGraphicsLayer::LayerStyle	style = m_styleDef ;
				const SString *	pstrRef = pxmlTag->GetAttributeAs( L"ref" )  ;
				if ( pstrRef != NULL )
				{
					AGLGraphicsLayer::LayerStyle *
							pRefStyle = m_ssoaStyles.GetAs( *pstrRef ) ;
					if ( pRefStyle != NULL )
					{
						style = *pRefStyle ;
					}
				}
				ParseLayerStyle( style, *pxmlTag ) ;
				m_ssoaStyles.SetAs( *pstrID, style ) ;
			}
		}
		const SXMLDocument *
			pxmlInitScreen = pxmlGraphics->GetElementTagAs( L"init_screen" ) ;
		if ( pxmlInitScreen != NULL )
		{
			m_flagCfgInitScreen = true ;
			m_iCfgInitScreen = (size_t) pxmlInitScreen->GetAttrIntegerAs( L"screen", 0 ) ;
			const AGLGraphicsLayer::LayerStyle *
				pInitStyle = GetLayerStyleAs( pxmlInitScreen-> GetAttrStringAs( L"style" ) ) ;
			if ( pInitStyle != NULL )
			{
				m_styleInitScreen = *pInitStyle ;
			}
			else
			{
				m_styleInitScreen = m_styleDef ;
			}
		}
		else
		{
			m_flagCfgInitScreen = false ;
		}
	}
}

void AGLGraphicsProcessor::ParseLayerStyle
	( AGLGraphicsLayer::LayerStyle& style,
				const SSystem::SXMLDocument& xmlStyle )
{
	style.nFlags = 0 ;
	style.nType =
		(uint32_t) ParseLayerType
					( xmlStyle, (AGLGraphicsLayer::LayerType) style.nType ) ;
	style.nBasePriority =
		(int32_t) xmlStyle.GetAttrIntegerAs
					( L"priority", style.nBasePriority ) ;
	style.sizeScreen.w =
		(int32_t) xmlStyle.GetAttrIntegerAs
					( L"layer_width", style.sizeScreen.w ) ;
	style.sizeScreen.h =
		(int32_t) xmlStyle.GetAttrIntegerAs
					( L"layer_height", style.sizeScreen.h ) ;
	style.ptDstOffset.x =
		(int32_t) xmlStyle.GetAttrIntegerAs
					( L"offset_x", style.ptDstOffset.x ) ;
	style.ptDstOffset.y =
		(int32_t) xmlStyle.GetAttrIntegerAs
					( L"offset_y", style.ptDstOffset.y ) ;
	style.ptSrcPivot.x =
		(int32_t) xmlStyle.GetAttrIntegerAs
					( L"pivot_x", style.ptSrcPivot.x ) ;
	style.ptSrcPivot.y =
		(int32_t) xmlStyle.GetAttrIntegerAs
					( L"pivot_y", style.ptSrcPivot.y ) ;
}

// タイマ処理 (実行フレーム前処理)
//////////////////////////////////////////////////////////////////////////////
void AGLGraphicsProcessor::OnKernelTimer( void )
{
}

// ゲーム開始時処理
//////////////////////////////////////////////////////////////////////////////
void AGLGraphicsProcessor::InitializeGame( void )
{
	if ( m_flagCfgInitScreen )
	{
		AGLGraphicsLayer *	pLayer = new AGLGraphicsLayer ;
		pLayer->SetLayerStyle( m_styleInitScreen ) ;
		pLayer->ApplyFrameBufferStyle( m_pDevice ) ;
		SetScreenAt( m_iCfgInitScreen, pLayer ) ;
	}
}

// ゲーム終了前フェードアウト処理
//////////////////////////////////////////////////////////////////////////////
void AGLGraphicsProcessor::FadeoutGame( uint32_t msecFadeout )
{
}

// ゲーム終了時処理
//////////////////////////////////////////////////////////////////////////////
void AGLGraphicsProcessor::ReleaseGame( void )
{
	if ( m_pRootScreen != NULL )
	{
		SSmartLock<SGLSprite>	lock( m_pRootScreen ) ;
		for ( size_t i = 0; i < m_aScreens.GetLength(); i ++ )
		{
			AGLGraphicsLayer *	pScreen = m_aScreens.GetAt( i ) ;
			if ( pScreen == NULL )
			{
				continue ;
			}
			m_pRootScreen->DetachChild( pScreen ) ;
		}
		m_aScreens.RemoveAll() ;
		m_pScreen = NULL ;
	}
}

// レイヤースタイル取得
//////////////////////////////////////////////////////////////////////////////
const AGLGraphicsLayer::LayerStyle *
	AGLGraphicsProcessor::GetLayerStyleAs( const wchar_t * pwszID ) const
{
	return	m_ssoaStyles.GetAs( pwszID ) ;
}

AGLGraphicsLayer::LayerStyle
	AGLGraphicsProcessor::MakeLayerStyleAs
		( const wchar_t * pwszID, AGLGraphicsLayer::LayerType type ) const
{
	AGLGraphicsLayer::LayerStyle	style = m_styleDef ;
	const AGLGraphicsLayer::LayerStyle *	pStyle = GetLayerStyleAs( pwszID ) ;
	if ( pStyle != NULL )
	{
		style = *pStyle ;
	}
	if ( type != AGLGraphicsLayer::typeDefault )
	{
		style.nType = type ;
	}
	return	style ;
}

// スクリーン作成
//////////////////////////////////////////////////////////////////////////////
void AGLGraphicsProcessor::InitScreen
	( size_t iScreen, int32_t nPriority, bool flagVisible,
		const wchar_t * pwszStyleID, AGLGraphicsLayer::LayerType type )
{
	AGLGraphicsLayer *	pScreen = CreateLayer( pwszStyleID, type ) ;
	pScreen->SetVisible( flagVisible ) ;
	pScreen->ChangePriority( m_nBasePriority + nPriority ) ;
	SetScreenAt( iScreen, pScreen ) ;
}

void AGLGraphicsProcessor::CloneScreen( size_t iScreen, size_t iSrcScreen, int32_t nPriority )
{
	AGLGraphicsLayer *	pScreen = m_aScreens.GetAt( iSrcScreen ) ;
	if ( pScreen != NULL )
	{
		AGLGraphicsLayer *	pCloned = new AGLGraphicsLayer( *pScreen ) ;
		pCloned->ChangePriority( m_nBasePriority + nPriority ) ;
		SetScreenAt( iScreen, pCloned ) ;
	}
}

void AGLGraphicsProcessor::ScreenPriority( size_t iScreen, int32_t nPriority )
{
	AGLGraphicsLayer *	pScreen = m_aScreens.GetAt( iScreen ) ;
	if ( pScreen != NULL )
	{
		pScreen->ChangePriority( nPriority ) ;
	}
}

void AGLGraphicsProcessor::ScreenVisible( size_t iScreen, bool flagVisible )
{
	AGLGraphicsLayer *	pScreen = m_aScreens.GetAt( iScreen ) ;
	if ( pScreen != NULL )
	{
		pScreen->SetVisible( flagVisible ) ;
	}
}

void AGLGraphicsProcessor::SetScreenAt( size_t iScreen, AGLGraphicsLayer * pScreen )
{
	SSmartLock<SGLSprite>	lock( m_pRootScreen ) ;
	//
	ReleaseScreen( iScreen ) ;
	m_aScreens.SetAt( iScreen, pScreen ) ;
	//
	if ( (m_pRootScreen != NULL) && (pScreen != NULL) )
	{
		m_pRootScreen->AddChild( pScreen ) ;
	}
}

void AGLGraphicsProcessor::SelectScreen( size_t iScreen )
{
	m_pScreen = m_aScreens.GetAt( iScreen ) ;
}

// スクリーン解放
//////////////////////////////////////////////////////////////////////////////
void AGLGraphicsProcessor::ReleaseScreen( size_t iScreen )
{
	SSmartLock<SGLSprite>	lock( m_pRootScreen ) ;
	AGLGraphicsLayer *	pScreen = m_aScreens.GetAt( iScreen ) ;
	if ( pScreen != NULL )
	{
		if ( m_pRootScreen != NULL )
		{
			m_pRootScreen->DetachChild( pScreen ) ;
		}
		if ( m_pScreen == pScreen )
		{
			m_pScreen = NULL ;
		}
		m_aScreens.SetAt( iScreen, NULL ) ;
	}
}

// スクリーン・スワップ
//////////////////////////////////////////////////////////////////////////////
void AGLGraphicsProcessor::SwapScreen( size_t iScreen0, size_t iScreen1 )
{
	if ( iScreen0 >= m_aScreens.GetLength() )
	{
		m_aScreens.SetLength( iScreen0 + 1 ) ;
	}
	if ( iScreen1 >= m_aScreens.GetLength() )
	{
		m_aScreens.SetLength( iScreen1 + 1 ) ;
	}
	m_aScreens.Swap( iScreen0, iScreen1 ) ;
}

// レイヤー取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLSprite * AGLGraphicsProcessor::GetSpriteAs( const wchar_t * pwszLayerID )
{
	if ( m_pScreen != NULL )
	{
		return	m_pScreen->GetSpriteAs( pwszLayerID ) ;
	}
	return	NULL ;
}

AGLGraphicsLayer * AGLGraphicsProcessor::GetLayerAs( const wchar_t * pwszLayerID )
{
	return	ESLTypeCast<AGLGraphicsLayer>( GetSpriteAs( pwszLayerID ) ) ;
}

SakuraGL::SGLSprite *
	AGLGraphicsProcessor::GetTargetSpriteAs
		( AGLThread& thread, const SSystem::SXMLDocument & xmlCode )
{
	if ( m_pScreen == NULL )
	{
		return	NULL ;
	}
	const SString *	pstrLayerID = xmlCode.GetAttributeAs( L"layer" ) ;
	if ( pstrLayerID == NULL )
	{
		return	m_pScreen ;
	}
	return	GetSpriteAs( EvaluateExprInText( thread, *pstrLayerID ) ) ;
}

// レイヤー作成
//////////////////////////////////////////////////////////////////////////////
AGLGraphicsLayer * AGLGraphicsProcessor::CreateLayer
	( const wchar_t * pwszStyleID, AGLGraphicsLayer::LayerType type )
{
	AGLGraphicsLayer::LayerStyle
				style = MakeLayerStyleAs( pwszStyleID, type ) ;
	AGLGraphicsLayer *	pLayer = new AGLGraphicsLayer ;
	pLayer->SetLayerStyle( style ) ;
	pLayer->ApplyFrameBufferStyle( m_pDevice ) ;
	return	pLayer ;
}

// レイヤーアイテム作成
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLSprite *
	AGLGraphicsProcessor::NewLayerItem( const SSystem::SString& strItemType )
{
	if ( strItemType == L"movie" )
	{
		return	new SGLSpriteMovie ;
	}
	return	new AGLGraphicsLayer ;
}

void AGLGraphicsProcessor::OnInitLayerItem
	( SakuraGL::SGLSprite * pNewSprite,
		const SSystem::SString& strItemType,
		AGLThread& thread, const SSystem::SXMLDocument & xmlParams )
{
	AGLGraphicsLayer *	pLayer = ESLTypeCast<AGLGraphicsLayer>( pNewSprite ) ;
	if ( strItemType == L"image" )
	{
		const SString *	pstrFile = xmlParams.GetAttributeAs( L"file" ) ;
		if ( pstrFile != NULL )
		{
			SString		strImageFile = EvaluateExprInText( thread, *pstrFile ) ;
			if ( SString(strImageFile.GetFileExtensionPart()).IsEmpty() )
			{
				strImageFile += L".eri" ;
			}
			if ( pLayer->LoadLayerImage( strImageFile, 0 ) )
			{
				SString	strErrMsg ;
				GetKernel()->OutputTrace
					( strErrMsg.Format
						( L"AGLGraphicsProcessor:error:"
							L"image レイヤーへ \"%s\" の読み込みに失敗しました。\n",
							(const wchar_t *) strImageFile ) ) ;
			}
		}
	}
	else if ( strItemType == L"movie" )
	{
		SGLSpriteMovie *	pMovie = ESLTypeCast<SGLSpriteMovie>( pNewSprite ) ;
		const SString *		pstrFile = xmlParams.GetAttributeAs( L"file" ) ;
		if ( pstrFile != NULL )
		{
			SString		strMovieFile = EvaluateExprInText( thread, *pstrFile ) ;
			if ( SString(strMovieFile.GetFileExtensionPart()).IsEmpty() )
			{
				strMovieFile += L".mei" ;
			}
			if ( pMovie->OpenMovieFile( strMovieFile ) )
			{
				SString	strErrMsg ;
				GetKernel()->OutputTrace
					( strErrMsg.Format
						( L"AGLGraphicsProcessor:error:"
							L"movie レイヤーで \"%s\" を開けませんでした。\n",
							(const wchar_t *) strMovieFile ) ) ;
			}
		}
	}
}

SakuraGL::SGLSprite * AGLGraphicsProcessor::CreateLayerItem
	( const SSystem::SString& strItemType,
		const AGLGraphicsLayer::LayerStyle& style,
		AGLThread& thread, const SSystem::SXMLDocument & xmlParams )
{
	//
	// アイテム生成
	//
	SGLSprite *	pSprite = NewLayerItem( strItemType ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	AGLGraphicsLayer *	pLayer = ESLTypeCast<AGLGraphicsLayer>( pSprite ) ;
	if ( pLayer != NULL )
	{
		pLayer->SetLayerStyle( style ) ;
		pLayer->ApplyFrameBufferStyle( m_pDevice ) ;
	}
	else
	{
		AGLGraphicsLayer::SetLayerStyle( *pSprite, style ) ;
		AGLGraphicsLayer::ApplyFrameBufferStyle( *pSprite, style, m_pDevice ) ;
	}
	//
	// 生成パラメータ処理
	//
	OnInitLayerItem( pSprite, strItemType, thread, xmlParams ) ;
	//
	// 共通パラメータ適用
	//
	ParseSpriteParameters( *pSprite, style, thread, xmlParams ) ;
	//
	pSprite->SetID
		( EvaluateExprInText( thread, xmlParams.GetAttrStringAs( L"id" ) ) ) ;
	return	pSprite ;
}


// レイヤー共通パラメータ設定
//////////////////////////////////////////////////////////////////////////////
void AGLGraphicsProcessor::ParseSpriteParameters
	( SakuraGL::SGLSprite& sprite,
		const AGLGraphicsLayer::LayerStyle& style,
		AGLThread& thread, const SSystem::SXMLDocument & xmlParams )
{
	AGLGraphicsLayer *	pLayer = ESLTypeCast<AGLGraphicsLayer>( &sprite ) ;
	if ( pLayer != NULL )
	{
		AGLGraphicsLayer::GraphicsParam	param = pLayer->GetLastMovedParam() ;
		ParseLayerParameters( param, style, thread, xmlParams ) ;
		pLayer->SetCurrentParam( param ) ;
	}
	else
	{
		AGLGraphicsLayer::GraphicsParam
				param = AGLGraphicsLayer::GetCurrentParam( sprite ) ;
		ParseLayerParameters( param, style, thread, xmlParams ) ;
		AGLGraphicsLayer::SetCurrentParam( sprite, param ) ;
	}
	sprite.SetVisible
		( ParseBoolParameter
			( L"visible", sprite.IsVisible(), thread, xmlParams ) ) ;
	//
	const SString *	pstrPriority = xmlParams.GetAttributeAs( L"priority" ) ;
	if ( pstrPriority != NULL )
	{
		int32_t	nBasePriority = m_nBasePriority + style.nBasePriority ;
		sprite.ChangePriority
			( nBasePriority +
				(int32_t) EvaluateIntExpression
					( thread, *pstrPriority,
						sprite.GetPriority() - nBasePriority ) ) ;
	}
}

void AGLGraphicsProcessor::ParseLayerParameters
	( AGLGraphicsLayer::GraphicsParam& param,
		const AGLGraphicsLayer::LayerStyle& style,
		AGLThread& thread, const SSystem::SXMLDocument & xmlParams )
{
	param.vPosition.x = (float32_t) style.ptDstOffset.x
		+ ParseNumParameter<float32_t>
			( L"x", param.vPosition.x - style.ptDstOffset.x, thread, xmlParams ) ;
	param.vPosition.y = (float32_t) style.ptDstOffset.y
		+ ParseNumParameter<float32_t>
			( L"y", param.vPosition.y - style.ptDstOffset.y, thread, xmlParams ) ;
	param.vPosition.x +=
		ParseNumParameter<float32_t>( L"dx", 0.0f, thread, xmlParams ) ;
	param.vPosition.y +=
		ParseNumParameter<float32_t>( L"dy", 0.0f, thread, xmlParams ) ;
	param.vZoom.x =
		ParseNumParameter<float32_t>
			( L"sx", param.vZoom.x, thread, xmlParams ) ;
	param.vZoom.y =
		ParseNumParameter<float32_t>
			( L"sy", param.vZoom.y, thread, xmlParams ) ;
	param.degRotate =
		ParseNumParameter<float32_t>
			( L"rz", param.degRotate, thread, xmlParams ) ;
	param.nTransparency =
		ParseUIntParameter<uint32_t>
			( L"tr", param.nTransparency, thread, xmlParams ) ;
	param.nFilterParam =
		ParseUIntParameter<uint32_t>
			( L"fp", param.nFilterParam, thread, xmlParams ) ;
	param.nFilterParam2 =
		ParseUIntParameter<uint32_t>
			( L"fp2", param.nFilterParam2, thread, xmlParams ) ;
}

AGLGraphicsLayer::LayerType
	AGLGraphicsProcessor::ParseLayerType
		( const SSystem::SXMLDocument & xmlParams,
				AGLGraphicsLayer::LayerType typeDefault )
{
	static const SXMLDocument::AttrInteger	s_aiLayerTypes[] =
	{
		{ L"layer", AGLGraphicsLayer::typeLayer },
		{ L"buf_layer", AGLGraphicsLayer::typeBufferedLayer },
		{ L"layer_set", AGLGraphicsLayer::typeLayerSet },
		{ L"fb_vram", AGLGraphicsLayer::typeFrameBufferVRAM },
		{ L"fb_ram", AGLGraphicsLayer::typeFrameBufferRAM },
		{ L"fb_cpu", AGLGraphicsLayer::typeFrameBufferCPU },
		{ NULL, 0 },
	} ;
	return	(AGLGraphicsLayer::LayerType)
				xmlParams.GetAttrSymbolizedIntegerAs
					( L"layer_type", s_aiLayerTypes, typeDefault ) ;
}

// フィルター作成
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLSpriteFilter * AGLGraphicsProcessor::CreateSpriteFilter
	( const SSystem::SString& strFilterType,
		AGLThread& thread, const SSystem::SXMLDocument & xmlParams )
{
	if ( strFilterType == L"transparency" )
	{
		return	new SGLSpriteFilterTransparencyDrawer ;
	}
	else if ( strFilterType == L"alpha_mask" )
	{
		SGLSpriteFilterBlendAlpha *	pAlpha = new SGLSpriteFilterBlendAlpha ;
		const SString *	pstrAlphaFile = xmlParams.GetAttributeAs( L"alpha_file" ) ;
		if ( pstrAlphaFile != NULL )
		{
			SString	strAlphaFile = EvaluateExprInText( thread, *pstrAlphaFile ) ;
			if ( SString(strAlphaFile.GetFileExtensionPart()).IsEmpty() )
			{
				strAlphaFile += L".eri" ;
			}
			pAlpha->LoadAlphaImage( strAlphaFile ) ;
		}
		const SString *	pstrCoefficient = xmlParams.GetAttributeAs( L"alpha_coef" ) ;
		if ( pstrCoefficient != NULL )
		{
			pAlpha->SetAlphaParameter
				( (int32_t) EvaluateIntExpression( thread, *pstrCoefficient, 0x100 ) ) ;
		}
		return	pAlpha ;
	}
	else if ( strFilterType == L"tone_curve" )
	{
		SGLSpriteFilterTone *	pTone = new SGLSpriteFilterTone ;
		const SString *	pstrToneFile = xmlParams.GetAttributeAs( L"tone_file" ) ;
		if ( pstrToneFile != NULL )
		{
			SString	strToneFile = EvaluateExprInText( thread, *pstrToneFile ) ;
			if ( SString(strToneFile.GetFileExtensionPart()).IsEmpty() )
			{
				strToneFile += L".tcf" ;
			}
			pTone->LoadFilterFile( strToneFile ) ;
		}
		return	pTone ;
	}
	else if ( strFilterType == L"shading_off" )
	{
		SGLSpriteFilterShadingOff *	pShading = new SGLSpriteFilterShadingOff ;
		bool			flagTrans = false ;
		const SString *	pstrTrans = xmlParams.GetAttributeAs( L"transition" ) ;
		if ( pstrTrans != NULL )
		{
			flagTrans = EvaluateBoolExpression( thread, *pstrTrans, false ) ;
		}
		pShading->SetTransitionOption
			( flagTrans, (uint32_t) xmlParams.GetAttrHexIntegerAs( L"over_color", 0 ) ) ;
		return	pShading ;
	}
	return	NULL ;
}

// レイヤー固有操作
//////////////////////////////////////////////////////////////////////////////
CodeProcessResult AGLGraphicsProcessor::OperateLayer
	( SakuraGL::SGLSprite& sprite,
		const SSystem::SString& strOperate,
		AGLThread& thread, const SSystem::SXMLDocument & xmlParams )
{
	if ( strOperate == L"movie_play" )
	{
		SGLSpriteMovie *	pMovie = ESLTypeCast<SGLSpriteMovie>( &sprite ) ;
		if ( pMovie == NULL )
		{
			return	codeProcessed ;
		}
		bool	flagLoop = ParseBoolParameter( L"loop", false, thread, xmlParams ) ;
		pMovie->SetMovieLoop( flagLoop ) ;
		pMovie->PlayMovie() ;
	}
	else if ( strOperate == L"movie_stop" )
	{
		SGLSpriteMovie *	pMovie = ESLTypeCast<SGLSpriteMovie>( &sprite ) ;
		if ( pMovie == NULL )
		{
			return	codeProcessed ;
		}
		pMovie->StopMovie() ;
	}
	else if ( strOperate == L"movie_wait" )
	{
		SGLSpriteMovie *	pMovie = ESLTypeCast<SGLSpriteMovie>( &sprite ) ;
		if ( pMovie == NULL )
		{
			return	codeProcessed ;
		}
		if ( thread.IsPermittedSkip( syncTypeEvent )
			&& m_pKernel->ShouldAbortSync( syncTypeEvent ) )
		{
			pMovie->StopMovie() ;
			m_pKernel->NotifyAbortedSync( syncTypeEvent ) ;
			return	codeProcessed ;
		}
		if ( pMovie->IsMoviePlaying() )
		{
			return	codePending ;
		}
	}
	return	codeProcessed ;
}

// コマンド実装
//////////////////////////////////////////////////////////////////////////////
IMPL_ANTIRRHINUM_PROC(AGLGraphicsProcessor,screen_init)
{
	size_t	iScreen =ParseUIntParameter<size_t>( L"screen", 0, thread, code ) ;
	if ( m_aScreens.GetAt(iScreen) != NULL )
	{
		return	codeProcessed ;
	}
	int32_t	nPriority = ParseIntParameter<int32_t>( L"priority", 0, thread, code ) ;
	bool	flagVisible = ParseBoolParameter( L"visible", true, thread, code ) ;
	SString	strStyle = EvaluateExprInText( thread, code.GetAttrStringAs( L"style" ) ) ;
	AGLGraphicsLayer::LayerType
			type = ParseLayerType( code, AGLGraphicsLayer::typeFrameBufferRAM ) ;
	//
	InitScreen( iScreen, nPriority, flagVisible, strStyle, type ) ;
	//
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLGraphicsProcessor,screen_clone)
{
	size_t	iDstScreen =ParseUIntParameter<size_t>( L"screen", 0, thread, code ) ;
	size_t	iSrcScreen =ParseUIntParameter<size_t>( L"src_screen", 1, thread, code ) ;
	int32_t	nPriority = ParseIntParameter<int32_t>( L"priority", 0, thread, code ) ;
	//
	CloneScreen( iDstScreen, iSrcScreen, nPriority ) ;
	//
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLGraphicsProcessor,screen_delete)
{
	size_t	iScreen =ParseUIntParameter<size_t>( L"screen", 0, thread, code ) ;
	//
	ReleaseScreen( iScreen ) ;
	//
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLGraphicsProcessor,screen_swap)
{
	size_t	iScreen0 =ParseUIntParameter<size_t>( L"screen", 0, thread, code ) ;
	size_t	iScreen1 =ParseUIntParameter<size_t>( L"src_screen", 1, thread, code ) ;
	//
	SwapScreen( iScreen0, iScreen1 ) ;
	//
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLGraphicsProcessor,screen_select)
{
	size_t	iScreen =ParseUIntParameter<size_t>( L"screen", 0, thread, code ) ;
	m_pScreen = m_aScreens.GetAt(iScreen) ;
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLGraphicsProcessor,layer_create)
{
	const SString *	pstrClass = code.GetAttributeAs( L"class" ) ;
	if ( pstrClass == NULL )
	{
		return	codeProcessed ;
	}
	SGLSprite *	pSprite = GetTargetSpriteAs( thread, code ) ;
	if ( pSprite == NULL )
	{
		return	codeProcessed ;
	}
	AGLGraphicsLayer::LayerType
		type = ParseLayerType( code, AGLGraphicsLayer::typeDefault ) ;
	AGLGraphicsLayer::LayerStyle
		style = MakeLayerStyleAs
					( EvaluateExprInText
						( thread, code.GetAttrStringAs( L"style" ) ), type ) ;
	//
	SGLSprite *	pNewLayer = CreateLayerItem( *pstrClass, style, thread, code ) ;
	if ( pNewLayer != NULL )
	{
		SGLSprite *	pLastLayer = pSprite->GetItemAs( pNewLayer->GetID() ) ;
		//
		pSprite->AddSmartChild( pNewLayer ) ;
		//
		if ( (pLastLayer != nullptr)
			&& (pLastLayer != pSprite)
			&& (pLastLayer->GetParent() == pSprite) )
		{
			pSprite->RemoveChild( pLastLayer ) ;
		}
	}
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLGraphicsProcessor,layer_image)
{
	AGLGraphicsLayer *	pLayer =
		ESLTypeCast<AGLGraphicsLayer>( GetTargetSpriteAs( thread, code ) ) ;
	if ( pLayer != NULL )
	{
		SString		strImageFile = EvaluateExprInText( thread, code.GetAttrStringAs( L"file" ) ) ;
		uint32_t	nFadeTime = ParseUIntParameter<uint32_t>( L"fade", 0, thread, code ) ;
		//
		if ( SString(strImageFile.GetFileExtensionPart()).IsEmpty() )
		{
			strImageFile += L".eri" ;
		}
		pLayer->LoadLayerImage( strImageFile, nFadeTime ) ;
	}
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLGraphicsProcessor,layer_delete)
{
	SGLSprite *	pSprite = GetTargetSpriteAs( thread, code ) ;
	if ( (pSprite == NULL)
		|| (m_aScreens.FindPtr( ESLTypeCast<AGLGraphicsLayer>( pSprite ) ) >= 0) )
	{
		return	codeProcessed ;
	}
	SGLSprite *	pParent = pSprite->GetParent() ;
	if ( pParent == NULL )
	{
		return	codeProcessed ;
	}
	pParent->RemoveChild( pSprite ) ;
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLGraphicsProcessor,layer_exist)
{
	const SString *	pstrExpr = code.GetAttributeAs( L"result" ) ;
	if ( pstrExpr == NULL )
	{
		return	codeProcessed ;
	}
	AGLScriptObject	objResult = EvaluateExpression( thread, *pstrExpr, true ) ;
	if ( !objResult.IsNull() )
	{
		SGLSprite *	pSprite = GetTargetSpriteAs( thread, code ) ;
		objResult.PutInteger( (pSprite != nullptr) ? -1 : 0 ) ;
	}
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLGraphicsProcessor,layer_parameter)
{
	SGLSprite *	pSprite = GetTargetSpriteAs( thread, code ) ;
	if ( pSprite == NULL )
	{
		return	codeProcessed ;
	}
	AGLGraphicsLayer *	pLayer = ESLTypeCast<AGLGraphicsLayer>( pSprite ) ;
	if ( pLayer != NULL )
	{
		ParseSpriteParameters( *pSprite, pLayer->GetLayerStyle(), thread, code ) ;
	}
	else
	{
		ParseSpriteParameters( *pSprite, m_styleDef, thread, code ) ;
	}
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLGraphicsProcessor,layer_pre_move)
{
	AGLGraphicsLayer *	pLayer =
		ESLTypeCast<AGLGraphicsLayer>( GetTargetSpriteAs( thread, code ) ) ;
	if ( pLayer == NULL )
	{
		return	codeProcessed ;
	}
	bool	flagOffset = ParseBoolParameter( L"offset", false, thread, code ) ;
	//
	pLayer->PrepareMoveAnimation( flagOffset ) ;
	//
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLGraphicsProcessor,layer_add_move)
{
	AGLGraphicsLayer *	pLayer =
		ESLTypeCast<AGLGraphicsLayer>( GetTargetSpriteAs( thread, code ) ) ;
	if ( pLayer == NULL )
	{
		return	codeProcessed ;
	}
	AGLGraphicsLayer::MoveAnimeParam	param = pLayer->GetLastMovedParam() ;
	ParseLayerParameters( param, pLayer->GetLayerStyle(), thread, code ) ;
	//
	param.msecDuration = ParseUIntParameter<uint32_t>( L"time", 1000, thread, code ) ;
	param.fpSpeedStart = ParseNumParameter<float32_t>( L"v0", 1.0f, thread, code ) ;
	param.fpSpeedEnd = ParseNumParameter<float32_t>( L"v1", 1.0f, thread, code ) ;
	//
	pLayer->AddMoveAnimeParam( param ) ;
	//
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLGraphicsProcessor,layer_start_move)
{
	AGLGraphicsLayer *	pLayer =
		ESLTypeCast<AGLGraphicsLayer>( GetTargetSpriteAs( thread, code ) ) ;
	if ( pLayer == NULL )
	{
		return	codeProcessed ;
	}
	static const SXMLDocument::AttrInteger	s_aiLoopType[] =
	{
		{ L"once", SGLSprite::actionOnce },
		{ L"loop", SGLSprite::actionLoop },
		{ L"turn", SGLSprite::actionTurn },
	} ;
	bool	flagOffset = ParseBoolParameter( L"offset", false, thread, code ) ;
	SGLSprite::ActionType	typeAct =
		(SGLSprite::ActionType)
			code.GetAttrSymbolizedIntegerAs
				( L"loop", s_aiLoopType, SGLSprite::actionOnce ) ;
	//
	pLayer->StartMoveAnimation( typeAct, flagOffset ) ;
	//
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLGraphicsProcessor,layer_wait_move)
{
	SGLSprite *	pSprite = GetTargetSpriteAs( thread, code ) ;
	if ( pSprite == NULL )
	{
		return	codeProcessed ;
	}
	AGLGraphicsLayer *	pLayer = ESLTypeCast<AGLGraphicsLayer>( pSprite ) ;
	if ( pLayer != NULL )
	{
		bool	flagAllLayer = ParseBoolParameter( L"all_layer", false, thread, code ) ;
		if ( thread.IsPermittedSkip( syncTypeEffect )
			&& m_pKernel->ShouldAbortSync( syncTypeEffect ) )
		{
			pLayer->FlushLayerAnimation( flagAllLayer, false ) ;
			m_pKernel->NotifyAbortedSync( syncTypeEffect ) ;
			return	codeProcessed ;
		}
		if ( pLayer->IsPendingLayerAnimation( flagAllLayer, false ) )
		{
			return	codePending ;
		}
	}
	else
	{
		if ( thread.IsPermittedSkip( syncTypeEffect )
			&& m_pKernel->ShouldAbortSync( syncTypeEffect ) )
		{
			pSprite->FlushAction( SGLSprite::actionOnce ) ;
			m_pKernel->NotifyAbortedSync( syncTypeEffect ) ;
			return	codeProcessed ;
		}
		if ( pSprite->IsAction( SGLSprite::actionOnce ) )
		{
			return	codePending ;
		}
	}
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLGraphicsProcessor,layer_flush_move)
{
	SGLSprite *	pSprite = GetTargetSpriteAs( thread, code ) ;
	if ( pSprite == NULL )
	{
		return	codeProcessed ;
	}
	bool	flagAllMove = ParseBoolParameter( L"all_move", false, thread, code ) ;
	//
	AGLGraphicsLayer *	pLayer = ESLTypeCast<AGLGraphicsLayer>( pSprite ) ;
	if ( pLayer != NULL )
	{
		bool	flagAllLayer = ParseBoolParameter( L"all_layer", false, thread, code ) ;
		pLayer->FlushLayerAnimation( flagAllLayer, flagAllMove ) ;
	}
	else
	{
		if ( flagAllMove )
		{
			pSprite->FlushAction() ;
		}
		else
		{
			pSprite->FlushAction( SGLSprite::actionOnce ) ;
		}
	}
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLGraphicsProcessor,layer_add_filter)
{
	SGLSprite *	pSprite = GetTargetSpriteAs( thread, code ) ;
	if ( pSprite == NULL )
	{
		return	codeProcessed ;
	}
	const SString *	pstrFilter = code.GetAttributeAs( L"filter" ) ;
	if ( pstrFilter == NULL )
	{
		return	codeProcessed ;
	}
	SString	strFilter = EvaluateExprInText( thread, *pstrFilter ) ;
	//
	SGLSpriteFilter *	pFilter = CreateSpriteFilter( strFilter, thread, code ) ;
	if ( pFilter != NULL )
	{
		pSprite->AddSmartFilter( pFilter ) ;
	}
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLGraphicsProcessor,layer_clear_filter)
{
	SGLSprite *	pSprite = GetTargetSpriteAs( thread, code ) ;
	if ( pSprite != NULL )
	{
		pSprite->RemoveAllFilter() ;
	}
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLGraphicsProcessor,layer_operate)
{
	SGLSprite *	pSprite = GetTargetSpriteAs( thread, code ) ;
	if ( pSprite == NULL )
	{
		return	codeProcessed ;
	}
	const SString *	pstrOpCmd = code.GetAttributeAs( L"operate" ) ;
	if ( pstrOpCmd == NULL )
	{
		return	codeProcessed ;
	}
	SString	strOpCmd = EvaluateExprInText( thread, *pstrOpCmd ) ;
	//
	return	OperateLayer( *pSprite, strOpCmd, thread, code ) ;
}

IMPL_ANTIRRHINUM_PROC(AGLGraphicsProcessor,layer_freeze)
{
	SGLSprite *	pSprite = GetTargetSpriteAs( thread, code ) ;
	if ( pSprite != NULL )
	{
		pSprite->FreezeFrameUpdate() ;
	}
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLGraphicsProcessor,layer_defrost)
{
	SGLSprite *	pSprite = GetTargetSpriteAs( thread, code ) ;
	if ( pSprite != NULL )
	{
		pSprite->DefrostFrameUpdate() ;
	}
	return	codeProcessed ;
}


