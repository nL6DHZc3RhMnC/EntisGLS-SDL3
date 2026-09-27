
#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl2d/sgl_image_filter.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// スプライト描画インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSpriteDrawer, SGLObject )

// スプライトにアタッチされた
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteDrawer::OnAttachedSprite( SGLSprite * pSprite )
{
}

// 描画
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteDrawer::Draw
	( S3DRenderContextInterface& render,
		const SGLPaintParam& pp, SGLImageObject* image )
{
	if ( image != NULL )
	{
		render.DrawImage( pp, image ) ;
	}
}

// 描画域取得
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteDrawer::GetRectangle
	( SGLImageRect& rectDraw, SGLImageObject* image ) const
{
	if ( image == NULL )
	{
		return	false ;
	}
	SGLImageInfo	imginf ;
	if ( image->GetImageInfo( imginf ) != sglErrSuccess )
	{
		return	false ;
	}
	rectDraw.x = 0 ;
	rectDraw.y = 0 ;
	rectDraw.w = imginf.width ;
	rectDraw.h = imginf.height ;
	return	true ;
}

// 当たり判定
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteDrawer::IsHitPointAt
		( SGLImageObject* pImage, double x, double y ) const
{
	int32_t	xPos = (int32_t) x ;
	int32_t	yPos = (int32_t) y ;
	if ( (xPos < 0) || (yPos < 0) )
	{
		return	false ;
	}
	if ( pImage == NULL )
	{
		return	false ;
	}
	SGLImageInfo	imginf ;
	if ( pImage->GetImageInfo( imginf ) != sglErrSuccess )
	{
		return	false ;
	}
	if ( (xPos >= (int32_t) imginf.width)
			|| (yPos >= (int32_t) imginf.height) )
	{
		return	false ;
	}
	bool	fAlpha = ((imginf.format & formatImageFlagAlpha) != 0)
												&& (imginf.depth == 32) ;
	bool	fClip = ((imginf.format & formatImageFlagClipping) != 0)
												&& (imginf.depth == 8) ;
	if ( !(fAlpha || fClip) )
	{
		return	true ;
	}
	SGLImageRect	rect( xPos, yPos, 1, 1 ) ;
	uint8_t *	pbytPixel =
		pImage->LockBuffer( imginf, SGLImageObject::lockRead, &rect ) ;
	if ( pbytPixel == NULL )
	{
		return	true ;
	}
	bool	fHit ;
	if ( fAlpha )
	{
		fHit = (pbytPixel[3] >= 0x80) ;
	}
	else
	{
		fHit = (pbytPixel[0] != imginf.colorClip) ;
	}
	pImage->UnlockBuffer( SGLImageObject::lockRead ) ;
	return	fHit ;
}

// 複製（可能なら）
//////////////////////////////////////////////////////////////////////////////
SGLObject * SGLSpriteDrawer::DuplicateObject( void )
{
	return	new SGLSpriteDrawer ;
}


//////////////////////////////////////////////////////////////////////////////
// タイマー処理
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSpriteTimer, SGLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteTimer::SGLSpriteTimer( void )
{
}

// 構築関数（ダミー）
//////////////////////////////////////////////////////////////////////////////
SGLSpriteTimer::SGLSpriteTimer( const SGLSpriteTimer& timer )
{
}

// タイマー処理
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteTimer::OnTimer( SGLSprite& sprite, uint32_t msecPast )
{
	return	true ;		// タイマ終了
}


//////////////////////////////////////////////////////////////////////////////
// マウス入力インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSpriteMouseListener, SGLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteMouseListener::SGLSpriteMouseListener( void )
{
}

// マウス移動
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteMouseListener::OnMouseMove
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	return	false ;
}

void SGLSpriteMouseListener::OnMouseLeave( SGLSprite& sprite, int64_t nFlags )
{
}

// ホイール回転
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteMouseListener::OnMouseWheel
	( SGLSprite& sprite, int32_t zDelta,
		double xPos, double yPos, int64_t nFlags )
{
	return	false ;
}

// マウスボタン（前処理）
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteMouseListener::OnButtonDown
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	uint32_t	id = GetButtonID( nFlags ) ;
	if ( id == LeftButtonID )
	{
		return	OnLButtonDown( sprite, xPos, yPos, nFlags ) ;
	}
	else if ( id == RightButtonID )
	{
		return	OnRButtonDown( sprite, xPos, yPos, nFlags ) ;
	}
	else if ( id == MiddleButtonID )
	{
		return	OnMButtonDown( sprite, xPos, yPos, nFlags ) ;
	}
	return	false ;
}

bool SGLSpriteMouseListener::OnButtonUp
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	uint32_t	id = GetButtonID( nFlags ) ;
	if ( id == LeftButtonID )
	{
		return	OnLButtonUp( sprite, xPos, yPos, nFlags ) ;
	}
	else if ( id == RightButtonID )
	{
		return	OnRButtonUp( sprite, xPos, yPos, nFlags ) ;
	}
	else if ( id == MiddleButtonID )
	{
		return	OnMButtonUp( sprite, xPos, yPos, nFlags ) ;
	}
	return	false ;
}

bool SGLSpriteMouseListener::OnButtonDblClk
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	uint32_t	id = GetButtonID( nFlags ) ;
	if ( id == LeftButtonID )
	{
		return	OnLButtonDblClk( sprite, xPos, yPos, nFlags ) ;
	}
	else if ( id == RightButtonID )
	{
		return	OnRButtonDblClk( sprite, xPos, yPos, nFlags ) ;
	}
	else if ( id == MiddleButtonID )
	{
		return	OnMButtonDblClk( sprite, xPos, yPos, nFlags ) ;
	}
	return	false ;
}

// 左ボタン
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteMouseListener::OnLButtonDown
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	return	false ;
}

bool SGLSpriteMouseListener::OnLButtonUp
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	return	false ;
}

bool SGLSpriteMouseListener::OnLButtonDblClk
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	return	false ;
}

// 右ボタン
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteMouseListener::OnRButtonDown
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	return	false ;
}

bool SGLSpriteMouseListener::OnRButtonUp
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	return	false ;
}

bool SGLSpriteMouseListener::OnRButtonDblClk
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	return	false ;
}

// 中央ボタン
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteMouseListener::OnMButtonDown
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	return	false ;
}

bool SGLSpriteMouseListener::OnMButtonUp
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	return	false ;
}

bool SGLSpriteMouseListener::OnMButtonDblClk
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	return	false ;
}

// マウスボタン（後処理）
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteMouseListener::AfterButtonDown
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	uint32_t	id = GetButtonID( nFlags ) ;
	if ( id == LeftButtonID )
	{
		return	AfterLButtonDown( sprite, xPos, yPos, nFlags ) ;
	}
	else if ( id == RightButtonID )
	{
		return	AfterRButtonDown( sprite, xPos, yPos, nFlags ) ;
	}
	else if ( id == MiddleButtonID )
	{
		return	AfterMButtonDown( sprite, xPos, yPos, nFlags ) ;
	}
	return	false ;
}

bool SGLSpriteMouseListener::AfterButtonUp
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	uint32_t	id = GetButtonID( nFlags ) ;
	if ( id == LeftButtonID )
	{
		return	AfterLButtonUp( sprite, xPos, yPos, nFlags ) ;
	}
	else if ( id == RightButtonID )
	{
		return	AfterRButtonUp( sprite, xPos, yPos, nFlags ) ;
	}
	else if ( id == MiddleButtonID )
	{
		return	AfterMButtonUp( sprite, xPos, yPos, nFlags ) ;
	}
	return	false ;
}

bool SGLSpriteMouseListener::AfterButtonDblClk
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	uint32_t	id = GetButtonID( nFlags ) ;
	if ( id == LeftButtonID )
	{
		return	AfterLButtonDblClk( sprite, xPos, yPos, nFlags ) ;
	}
	else if ( id == RightButtonID )
	{
		return	AfterRButtonDblClk( sprite, xPos, yPos, nFlags ) ;
	}
	else if ( id == MiddleButtonID )
	{
		return	AfterMButtonDblClk( sprite, xPos, yPos, nFlags ) ;
	}
	return	false ;
}

// 左ボタン
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteMouseListener::AfterLButtonDown
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	return	false ;
}

bool SGLSpriteMouseListener::AfterLButtonUp
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	return	false ;
}

bool SGLSpriteMouseListener::AfterLButtonDblClk
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	return	false ;
}

// 右ボタン
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteMouseListener::AfterRButtonDown
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	return	false ;
}

bool SGLSpriteMouseListener::AfterRButtonUp
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	return	false ;
}

bool SGLSpriteMouseListener::AfterRButtonDblClk
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	return	false ;
}

// 中央ボタン
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteMouseListener::AfterMButtonDown
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	return	false ;
}

bool SGLSpriteMouseListener::AfterMButtonUp
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	return	false ;
}

bool SGLSpriteMouseListener::AfterMButtonDblClk
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	return	false ;
}



//////////////////////////////////////////////////////////////////////////////
// マウス入力インターフェース（マルチタッチ座標記録）
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSpriteMouseStateListener, SGLSpriteMouseListener )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteMouseStateListener::SGLSpriteMouseStateListener( void )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteMouseStateListener::~SGLSpriteMouseStateListener( void )
{
}

// 有効ポインタ数取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLSpriteMouseStateListener::GetPointerCount( void ) const
{
	return	m_vecMousePointers.GetLength() ;
}

// 指標検索
//////////////////////////////////////////////////////////////////////////////
ssize_t SGLSpriteMouseStateListener::FindMouseIndexById( uint32_t idMouse ) const
{
	ssize_t				iFound = -1 ;
	const size_t		nCount = m_vecMousePointers.GetLength() ;
	MouseState*const*	ppms = m_vecMousePointers.GetConstArray() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		MouseState*	pms = ppms[i] ;
		ESLAssert( pms != NULL ) ;
		if ( pms->idMouse == idMouse )
		{
			iFound = (ssize_t) i ;
			break ;
		}
	}
	return	iFound ;
}

// ポインタ座標取得
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteMouseStateListener::GetMousePointAt( size_t i, S2DDVector& vPos ) const
{
	MouseState *	pms = m_vecMousePointers.GetAt( i ) ;
	if ( pms != NULL )
	{
		vPos = pms->vPos ;
		return	true ;
	}
	return	false ;
}

// マウスボタンの押下状態取得
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteMouseStateListener::IsLButtonDownAt( size_t i ) const
{
	MouseState *	pms = m_vecMousePointers.GetAt( i ) ;
	if ( pms != NULL )
	{
		return	pms->fLeftDown ;
	}
	return	false ;
}

bool SGLSpriteMouseStateListener::IsRButtonDownAt( size_t i ) const
{
	MouseState *	pms = m_vecMousePointers.GetAt( i ) ;
	if ( pms != NULL )
	{
		return	pms->fRightDown ;
	}
	return	false ;
}

// 左ドラッグ中／タップ中ポインタ数取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLSpriteMouseStateListener::GetLDownPointsCount( void ) const
{
	const size_t		nCount = m_vecMousePointers.GetLength() ;
	MouseState*const*	ppms = m_vecMousePointers.GetConstArray() ;
	size_t	nDownCount = 0 ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		MouseState*	pms = ppms[i] ;
		ESLAssert( pms != NULL ) ;
		if ( pms->fLeftDown )
		{
			nDownCount ++ ;
		}
	}
	return	nDownCount ;
}

// 左ドラッグ中／タップ中ポインタ座標取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLSpriteMouseStateListener::EnumerateLDownPoints
			( SSystem::SArray<S2DDVector>& arrPoints ) const
{
	const size_t		nCount = m_vecMousePointers.GetLength() ;
	MouseState*const*	ppms = m_vecMousePointers.GetConstArray() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		MouseState*	pms = ppms[i] ;
		ESLAssert( pms != NULL ) ;
		if ( pms->fLeftDown )
		{
			arrPoints.Add( pms->vPos ) ;
		}
	}
	return	arrPoints.GetLength() ;
}

// マウスステータス取得／存在しない場合には生成
//////////////////////////////////////////////////////////////////////////////
SGLSpriteMouseStateListener::MouseState *
	SGLSpriteMouseStateListener::CreateMouseStateAs
			( double xPos, double yPos, uint32_t idMouse )
{
	MouseState *	pms ;
	ssize_t	iMouse = FindMouseIndexById( idMouse ) ;
	if ( iMouse < 0 )
	{
		pms = new MouseState ;
		pms->idMouse = idMouse ;
		pms->fLeftDown = false ;
		pms->fRightDown = false ;
		pms->fMiddleDown = false ;
		iMouse = (ssize_t) m_vecMousePointers.Add( pms ) ;
	}
	else
	{
		pms = m_vecMousePointers.GetAt( iMouse ) ;
		ESLAssert( pms != NULL ) ;
	}
	pms->vPos.x = xPos ;
	pms->vPos.y = yPos ;
	return	pms ;
}

// マウス移動
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteMouseStateListener::OnMouseMove
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	CreateMouseStateAs( xPos, yPos, GetMouseID( nFlags ) ) ;
	return	SGLSpriteMouseListener::OnMouseMove( sprite, xPos, yPos, nFlags ) ;
}

void SGLSpriteMouseStateListener::OnMouseLeave
	( SGLSprite& sprite, int64_t nFlags )
{
	ssize_t	iMouse = FindMouseIndexById( GetMouseID( nFlags ) ) ;
	if ( iMouse >= 0 )
	{
		m_vecMousePointers.RemoveAt( iMouse ) ;
	}
	SGLSpriteMouseListener::OnMouseLeave( sprite, nFlags ) ;
}

// マウスボタン（前処理）
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteMouseStateListener::OnButtonDown
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	MouseState *	pms =
		CreateMouseStateAs( xPos, yPos, GetMouseID( nFlags ) ) ;
	uint32_t	idButton = GetButtonID( nFlags ) ;
	if ( idButton == LeftButtonID )
	{
		pms->fLeftDown = true ;
	}
	else if ( idButton == RightButtonID )
	{
		pms->fRightDown = true ;
	}
	else if ( idButton == MiddleButtonID )
	{
		pms->fMiddleDown = true ;
	}
	return	SGLSpriteMouseListener::OnButtonDown( sprite, xPos, yPos, nFlags ) ;
}

bool SGLSpriteMouseStateListener::OnButtonUp
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	MouseState *	pms =
		CreateMouseStateAs( xPos, yPos, GetMouseID( nFlags ) ) ;
	uint32_t	idButton = GetButtonID( nFlags ) ;
	if ( idButton == LeftButtonID )
	{
		pms->fLeftDown = false ;
	}
	else if ( idButton == RightButtonID )
	{
		pms->fRightDown = false ;
	}
	else if ( idButton == MiddleButtonID )
	{
		pms->fMiddleDown = false ;
	}
	return	SGLSpriteMouseListener::OnButtonUp( sprite, xPos, yPos, nFlags ) ;
}


//////////////////////////////////////////////////////////////////////////////
// キー入力インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSpriteKeyListener, SGLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteKeyListener::SGLSpriteKeyListener( void )
{
}

// キー入力
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteKeyListener::OnKeyDown
	( SGLSprite& sprite, int64_t nVirtKey, int64_t nFlags )
{
	return	false ;
}

bool SGLSpriteKeyListener::OnKeyUp
	( SGLSprite& sprite, int64_t nVirtKey, int64_t nFlags )
{
	return	false ;
}

// 文字入力
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteKeyListener::OnChar( SGLSprite& sprite, uint16_t codeChar )
{
	return	false ;
}

// コンポジション開始
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteKeyListener::OnStartComposition
	( SGLSprite& sprite, SGLInputStartComposition& iscForm )
{
	return	false ;
}

// コンポジション終了
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteKeyListener::OnEndComposition( SGLSprite& sprite )
{
	return	false ;
}

// コンポジション文字列
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteKeyListener::OnCompositionString
	( SGLSprite& sprite, const SGLInputCompositionString& icsComp )
{
	return	false ;
}

// コマンド
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteKeyListener::OnCommand
	( SGLSprite& sprite, const wchar_t * pszCmd,
		int64_t nParam, int64_t nCode, int nPriority, bool fOverwritable )
{
	return	false ;
}


//////////////////////////////////////////////////////////////////////////////
// スプライト・パラメータ・アニメーション
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSprite::Action, SGLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSprite::Action::Action( void )
	: m_typeAction(SGLSprite::actionOnce),
		m_flagPaused(false),
		m_msecPast(0), m_msecStart(0), m_msecDuration(0),
		m_maskSetElement(0), m_maskModifyElement(0)
{
}

SGLSprite::Action::Action( const SGLSprite::Action& act )
	: m_typeAction(act.m_typeAction),
		m_flagPaused(act.m_flagPaused),
		m_msecPast(act.m_msecPast), m_msecStart(act.m_msecStart),
		m_msecDuration(act.m_msecDuration),
		m_maskSetElement(act.m_maskSetElement),
		m_maskModifyElement(act.m_maskModifyElement)
{
}

// アニメーション処理
//////////////////////////////////////////////////////////////////////////////
bool SGLSprite::Action::OnAction
	( SGLSprite::Parameter& param, uint32_t msecPast )
{
	if ( m_flagPaused )
	{
		if ( m_msecStart > 0 )
		{
			if ( m_msecStart >= msecPast )
			{
				m_msecStart -= msecPast ;
				return	false ;
			}
			msecPast -= m_msecStart ;
			m_msecStart = 0 ;
		}
		if ( m_msecDuration == 0 )
		{
			OnFinish( param ) ;
			return	true ;
		}
	}
	double	t = 0.0 ;
	m_msecPast += msecPast ;
	if ( m_typeAction == SGLSprite::actionLoop )
	{
		m_msecPast %= m_msecDuration ;
		t = (double) m_msecPast / m_msecDuration ;
	}
	else if ( m_typeAction == SGLSprite::actionTurn )
	{
		m_msecPast %= m_msecDuration * 2 ;
		if ( m_msecPast <= m_msecDuration )
		{
			t = (double) m_msecPast / m_msecDuration ;
		}
		else
		{
			t = (double) (m_msecDuration * 2 - m_msecPast) / m_msecDuration ;
		}
	}
	else
	{
		if ( m_msecPast >= m_msecDuration )
		{
			m_msecPast = m_msecDuration ;
			OnFinish( param ) ;
			return	true ;
		}
		t = (double) m_msecPast / m_msecDuration ;
	}
	EffectParameter( param, t ) ;
	return	false ;
}

// アニメーション完了処理
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::Action::OnFinish( SGLSprite::Parameter& param )
{
	EffectParameter( param, 1.0 ) ;
}

// パラメータ反映
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::Action::EffectParameter
			( SGLSprite::Parameter& param, double t )
{
}

// アニメーションタイプ設定
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::Action::SetActionType( uint32_t typeAct )
{
	m_typeAction = typeAct ;
}

// 時間設定
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::Action::SetDuration
		( uint32_t msecDuration, uint32_t msecStart )
{
	m_msecDuration = msecDuration ;
	m_msecStart = msecStart ;
}

// 一時停止
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::Action::Pause( void )
{
	m_flagPaused = true ;
}

// 再開
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::Action::Restart( void )
{
	m_flagPaused = false ;
}

// 複製
//////////////////////////////////////////////////////////////////////////////
SGLObject * SGLSprite::Action::DuplicateObject( void )
{
	return	new Action( *this ) ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSprite::Action::OnSave( SSystem::SFileInterface& file )
{
	size_t		nWritten = 0 ;
	uint32_t	flagsOpt = 0 ;
	if ( m_flagPaused )
	{
		flagsOpt |= 0x01 ;
	}
	nWritten += file.Write( &m_typeAction, sizeof(uint32_t) ) ;
	nWritten += file.Write( &flagsOpt, sizeof(uint32_t) ) ;
	nWritten += file.Write( &m_msecPast, sizeof(uint32_t) ) ;
	nWritten += file.Write( &m_msecStart, sizeof(uint32_t) ) ;
	nWritten += file.Write( &m_msecDuration, sizeof(uint32_t) ) ;
	nWritten += file.Write( &m_maskSetElement, sizeof(uint32_t) ) ;
	nWritten += file.Write( &m_maskModifyElement, sizeof(uint32_t) ) ;
	if ( nWritten < sizeof(uint32_t) * 7 )
	{
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSprite::Action::OnRestore( SSystem::SFileInterface& file )
{
	size_t		nRead = 0 ;
	uint32_t	flagsOpt = 0 ;
	nRead += file.Read( &m_typeAction, sizeof(uint32_t) ) ;
	nRead += file.Read( &flagsOpt, sizeof(uint32_t) ) ;
	nRead += file.Read( &m_msecPast, sizeof(uint32_t) ) ;
	nRead += file.Read( &m_msecStart, sizeof(uint32_t) ) ;
	nRead += file.Read( &m_msecDuration, sizeof(uint32_t) ) ;
	nRead += file.Read( &m_maskSetElement, sizeof(uint32_t) ) ;
	nRead += file.Read( &m_maskModifyElement, sizeof(uint32_t) ) ;
	if ( nRead < sizeof(uint32_t) * 7 )
	{
		return	sglErrFailed ;
	}
	m_flagPaused = ((flagsOpt & 0x01) != 0) ;
	return	sglErrSuccess ;
}


//////////////////////////////////////////////////////////////////////////////
// パラメータ・アニメーション（ベジェ曲線）
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSpriteAction, Action )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteAction::SGLSpriteAction( void )
{
}

SGLSpriteAction::SGLSpriteAction( const SGLSpriteAction& act )
	: SGLSprite::Action( act ),
		m_bzPos( act.m_bzPos ), m_bzCenter( act.m_bzCenter ),
		m_bzZoom( act.m_bzZoom ), m_bzAngle( act.m_bzAngle ),
		m_bzTransparency( act.m_bzTransparency ),
		m_bzFilterParam( act.m_bzFilterParam ),
		m_bzFilterParam2( act.m_bzFilterParam2 )
{
}

// パラメータ反映
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteAction::EffectParameter
			( SGLSprite::Parameter& param, double t )
{
	if ( m_maskModifyElement & SGLSprite::flagParamPos )
	{
		S3DVector	vPos = m_bzPos.PointAt( t ) ;
		if ( m_maskModifyElement & SGLSprite::flagParamPosX )
			param.vDst.x += vPos.x ;
		if ( m_maskModifyElement & SGLSprite::flagParamPosY )
			param.vDst.y += vPos.y ;
		if ( m_maskModifyElement & SGLSprite::flagParamPosZ )
			param.vDst.z += vPos.z ;
	}
	else if ( m_maskSetElement & SGLSprite::flagParamPos )
	{
		S3DVector	vPos = m_bzPos.PointAt( t ) ;
		if ( m_maskSetElement & SGLSprite::flagParamPosX )
			param.vDst.x = vPos.x ;
		if ( m_maskSetElement & SGLSprite::flagParamPosY )
			param.vDst.y = vPos.y ;
		if ( m_maskSetElement & SGLSprite::flagParamPosZ )
			param.vDst.z = vPos.z ;
	}
	if ( m_maskModifyElement & SGLSprite::flagParamCenter )
	{
		S2DVector	vCenter = m_bzCenter.PointAt( t ) ;
		if ( m_maskModifyElement & SGLSprite::flagParamCenterX )
			param.vCenter.x += vCenter.x ;
		if ( m_maskModifyElement & SGLSprite::flagParamCenterY )
			param.vCenter.y += vCenter.y ;
	}
	else if ( m_maskSetElement & SGLSprite::flagParamCenter )
	{
		S2DVector	vCenter = m_bzCenter.PointAt( t ) ;
		if ( m_maskSetElement & SGLSprite::flagParamCenterX )
			param.vCenter.x = vCenter.x ;
		if ( m_maskSetElement & SGLSprite::flagParamCenterY )
			param.vCenter.y = vCenter.y ;
	}
	if ( m_maskModifyElement & SGLSprite::flagParamZoom )
	{
		S2DVector	vZoom = m_bzZoom.PointAt( t ) ;
		if ( m_maskModifyElement & SGLSprite::flagParamZoomX )
			param.vZoom.x += vZoom.x ;
		if ( m_maskModifyElement & SGLSprite::flagParamZoomY )
			param.vZoom.y += vZoom.y ;
	}
	else if ( m_maskSetElement & SGLSprite::flagParamZoom )
	{
		S2DVector	vZoom = m_bzZoom.PointAt( t ) ;
		if ( m_maskSetElement & SGLSprite::flagParamZoomX )
			param.vZoom.x = vZoom.x ;
		if ( m_maskSetElement & SGLSprite::flagParamZoomY )
			param.vZoom.y = vZoom.y ;
	}
	if ( m_maskModifyElement & SGLSprite::flagParamAngle )
	{
		param.zAngle += m_bzAngle.PointAt( t ) ;
	}
	else if ( m_maskSetElement & SGLSprite::flagParamAngle )
	{
		param.zAngle = m_bzAngle.PointAt( t ) ;
	}
	if ( m_maskModifyElement & SGLSprite::flagParamTransparency )
	{
		param.nTransparency +=
			(int32_t) eslRoundR64ToLInt( m_bzTransparency.PointAt( t ) ) ;
	}
	else if ( m_maskSetElement & SGLSprite::flagParamTransparency )
	{
		param.nTransparency =
			(int32_t) eslRoundR64ToLInt( m_bzTransparency.PointAt( t ) ) ;
	}
	if ( m_maskModifyElement & SGLSprite::flagParamFilter )
	{
		param.paramFilter +=
			(int32_t) eslRoundR64ToLInt( m_bzFilterParam.PointAt( t ) ) ;
	}
	else if ( m_maskSetElement & SGLSprite::flagParamFilter )
	{
		param.paramFilter =
			(int32_t) eslRoundR64ToLInt( m_bzFilterParam.PointAt( t ) ) ;
	}
	if ( m_maskModifyElement & SGLSprite::flagParamFilter2 )
	{
		param.paramFilter2 +=
			(int32_t) eslRoundR64ToLInt( m_bzFilterParam2.PointAt( t ) ) ;
	}
	else if ( m_maskSetElement & SGLSprite::flagParamFilter2 )
	{
		param.paramFilter2 =
			(int32_t) eslRoundR64ToLInt( m_bzFilterParam2.PointAt( t ) ) ;
	}
}

// 移動アニメーション設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteAction::SetMoveTo
	( const SGLSprite& sprite,
		double x, double y, double a0, double a1 )
{
	const S3DDVector&	vPos = sprite.GetPosition() ;
	S3DDVector			vDst( x, y, vPos.z ) ;
	m_bzPos.SetLength( 4 ) ;
	m_bzPos.SetLine( vPos, vDst, a0, a1 ) ;
	m_maskSetElement |= SGLSprite::flagParamPos ;
}

// 拡大アニメーション設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteAction::SetZoomTo
	( const SGLSprite& sprite,
		double x, double y, double a0, double a1 )
{
	const S2DDVector&	vZoom = sprite.GetZoom() ;
	S2DDVector			vDst( x, y ) ;
	m_bzZoom.SetLength( 4 ) ;
	m_bzZoom.SetLine( vZoom, vDst, a0, a1 ) ;
	m_maskSetElement |= SGLSprite::flagParamZoom ;
}

// 回転アニメーション設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteAction::SetRotationTo
	( const SGLSprite& sprite, double z, double a0, double a1 )
{
	m_bzAngle.SetLength( 4 ) ;
	m_bzAngle.SetLine( sprite.GetParameter().zAngle, z, a0, a1 ) ;
	m_maskSetElement |= SGLSprite::flagParamAngle ;
}

// 透明度アニメーション設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteAction::SetTransparencyTo
	( const SGLSprite& sprite, uint32_t nTransparency )
{
	m_bzTransparency.SetLength( 4 ) ;
	m_bzTransparency.SetLine
		( (double) sprite.GetTransparency(),
					(double) nTransparency, 1.0, 1.0 ) ;
	m_maskSetElement |= SGLSprite::flagParamTransparency ;
}

// フィルタアニメーション設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteAction::SetFilterTo
	( const SGLSprite& sprite, uint32_t paramFilter )
{
	m_bzFilterParam.SetLength( 4 ) ;
	m_bzFilterParam.SetLine
		( (double) sprite.GetParameter().paramFilter,
						(double) paramFilter, 1.0, 1.0 ) ;
	m_maskSetElement |= SGLSprite::flagParamFilter ;
}

void SGLSpriteAction::SetFilter2To
	( const SGLSprite& sprite, uint32_t paramFilter )
{
	m_bzFilterParam2.SetLength( 4 ) ;
	m_bzFilterParam2.SetLine
		( (double) sprite.GetParameter().paramFilter2,
						(double) paramFilter, 1.0, 1.0 ) ;
	m_maskSetElement |= SGLSprite::flagParamFilter2 ;
}

// 座標
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteAction::SetBezierCurve
	( const SSystem::SArray<S3DDVector>& bzCurve, bool fOffset )
{
	m_bzPos = bzCurve ;
	if ( fOffset )
	{
		m_maskModifyElement |= SGLSprite::flagParamPos ;
	}
	else
	{
		m_maskSetElement |= SGLSprite::flagParamPos ;
	}
}

// 中心座標
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteAction::SetCenterCurve
	( const SSystem::SArray<S2DDVector>& bzCurve, bool fOffset )
{
	m_bzCenter = bzCurve ;
	if ( fOffset )
	{
		m_maskModifyElement |= SGLSprite::flagParamCenter ;
	}
	else
	{
		m_maskSetElement |= SGLSprite::flagParamCenter ;
	}
}

// 拡大率
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteAction::SetZoomCurve
	( const SSystem::SArray<S2DDVector>& bzCurve, bool fOffset )
{
	m_bzZoom = bzCurve ;
	if ( fOffset )
	{
		m_maskModifyElement |= SGLSprite::flagParamZoom ;
	}
	else
	{
		m_maskSetElement |= SGLSprite::flagParamZoom ;
	}
}

// 回転角
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteAction::SetAngleCurve
	( const SSystem::SArray<double>& bzCurve, bool fOffset )
{
	m_bzAngle = bzCurve ;
	if ( fOffset )
	{
		m_maskModifyElement |= SGLSprite::flagParamAngle ;
	}
	else
	{
		m_maskSetElement |= SGLSprite::flagParamAngle ;
	}
}

// 透明度
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteAction::SetTransparencyCurve
	( const SSystem::SArray<double>& bzCurve, bool fOffset )
{
	m_bzTransparency = bzCurve ;
	if ( fOffset )
	{
		m_maskModifyElement |= SGLSprite::flagParamTransparency ;
	}
	else
	{
		m_maskSetElement |= SGLSprite::flagParamTransparency ;
	}
}

// フィルタパラメータ
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteAction::SetFilterParamCurve
	( const SSystem::SArray<double>& bzCurve, bool fOffset )
{
	m_bzFilterParam = bzCurve ;
	if ( fOffset )
	{
		m_maskModifyElement |= SGLSprite::flagParamFilter ;
	}
	else
	{
		m_maskSetElement |= SGLSprite::flagParamFilter ;
	}
}

void SGLSpriteAction::SetFilter2ParamCurve
	( const SSystem::SArray<double>& bzCurve, bool fOffset )
{
	m_bzFilterParam2 = bzCurve ;
	if ( fOffset )
	{
		m_maskModifyElement |= SGLSprite::flagParamFilter2 ;
	}
	else
	{
		m_maskSetElement |= SGLSprite::flagParamFilter2 ;
	}
}

// 複製（可能なら）
//////////////////////////////////////////////////////////////////////////////
SGLObject * SGLSpriteAction::DuplicateObject( void )
{
	return	new SGLSpriteAction( *this ) ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteAction::OnSave( SSystem::SFileInterface& file )
{
	SGLError	err = Action::OnSave( file ) ;
	if ( err )
	{
		return	err ;
	}
	if ( SaveArray<S3DDVector>( file, m_bzPos )
		|| SaveArray<S2DDVector>( file, m_bzCenter )
		|| SaveArray<S2DDVector>( file, m_bzZoom )
		|| SaveArray<double>( file, m_bzAngle )
		|| SaveArray<double>( file, m_bzTransparency )
		|| SaveArray<double>( file, m_bzFilterParam ) )
	{
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteAction::OnRestore( SSystem::SFileInterface& file )
{
	SGLError	err = Action::OnRestore( file ) ;
	if ( err )
	{
		return	err ;
	}
	if ( LoadArray<S3DDVector>( file, m_bzPos )
		|| LoadArray<S2DDVector>( file, m_bzCenter )
		|| LoadArray<S2DDVector>( file, m_bzZoom )
		|| LoadArray<double>( file, m_bzAngle )
		|| LoadArray<double>( file, m_bzTransparency )
		|| LoadArray<double>( file, m_bzFilterParam ) )
	{
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}


//////////////////////////////////////////////////////////////////////////////
// 疑似 3D カメラ移動アニメーション
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSpriteCameraAction, SGLSpriteTimer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteCameraAction::SGLSpriteCameraAction( void )
	: m_msecDuration( 0 ), m_msecElapsed( 0 )
{
}

SGLSpriteCameraAction::SGLSpriteCameraAction( const SGLSpriteCameraAction& act )
	: SGLSpriteTimer( act ),
		m_bzPos( act.m_bzPos ),
		m_bzTarget( act.m_bzTarget ),
		m_msecDuration( 0 ), m_msecElapsed( 0 )
{
}

// カメラ座標
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteCameraAction::SetMoveTo
	( const SGLSprite& sprite,
		const S3DDVector& vPos, double a0, double a1 )
{
	SGLSprite::Virtual3DParam *	pV3D = sprite.GetVirtual3DParam() ;
	if ( pV3D != nullptr )
	{
		m_bzPos.SetLine( pV3D->m_vCameraView, vPos, a0, a1 ) ;
	}
}

void SGLSpriteCameraAction::SetPositionBezier
	( const SSystem::SArray<S3DDVector>& bzCurve )
{
	m_bzPos = bzCurve ;
}

// ターゲット座標
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteCameraAction::SetTargetTo
	( const SGLSprite& sprite,
		const S3DDVector& vTarget, double a0, double a1 )
{
	SGLSprite::Virtual3DParam *	pV3D = sprite.GetVirtual3DParam() ;
	if ( pV3D != nullptr )
	{
		m_bzTarget.SetLine( pV3D->m_vCameraTarget, vTarget, a0, a1 ) ;
	}
}

void SGLSpriteCameraAction::SetTargetBezier
	( const SSystem::SArray<S3DDVector>& bzCurve )
{
	m_bzTarget = bzCurve ;
}

// 開始
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteCameraAction::StartAction( uint32_t msecDuration )
{
	m_msecElapsed = 0 ;
	m_msecDuration = msecDuration ;
}

// アニメーション中か？
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteCameraAction::IsAction( void ) const
{
	return	(m_msecDuration != 0) && (m_msecElapsed < m_msecDuration) ;
}

// 複製
//////////////////////////////////////////////////////////////////////////////
SGLObject * SGLSpriteCameraAction::DuplicateObject( void )
{
	return	new SGLSpriteCameraAction( *this ) ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteCameraAction::OnSave( SSystem::SFileInterface& file )
{
	SGLError	err = SGLSpriteTimer::OnSave( file ) ;
	if ( err )
	{
		return	err ;
	}
	uint32_t	nFlags = 0 ;
	file.Write( &nFlags, sizeof(nFlags) ) ;
	//
	if ( SaveArray<S3DDVector>( file, m_bzPos )
		|| SaveArray<S3DDVector>( file, m_bzTarget ) )
	{
		return	sglErrFailed ;
	}
	file.Write( &m_msecDuration, sizeof(m_msecDuration) ) ;
	file.Write( &m_msecElapsed, sizeof(m_msecElapsed) ) ;
	//
	return	sglErrSuccess ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteCameraAction::OnRestore( SSystem::SFileInterface& file )
{
	SGLError	err = SGLSpriteTimer::OnRestore( file ) ;
	if ( err )
	{
		return	err ;
	}
	uint32_t	nFlags = 0 ;
	file.Read( &nFlags, sizeof(nFlags) ) ;
	//
	if ( LoadArray<S3DDVector>( file, m_bzPos )
		|| LoadArray<S3DDVector>( file, m_bzTarget ) )
	{
		return	sglErrFailed ;
	}
	file.Read( &m_msecDuration, sizeof(m_msecDuration) ) ;
	file.Read( &m_msecElapsed, sizeof(m_msecElapsed) ) ;
	return	sglErrSuccess ;
}

// タイマー処理（true で終了）
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteCameraAction::OnTimer( SGLSprite& sprite, uint32_t msecPast )
{
	SGLSprite::Virtual3DParam *	pV3D = sprite.GetVirtual3DParam() ;
	if ( (m_msecDuration > 0) && (pV3D != nullptr) )
	{
		m_msecElapsed += msecPast ;
		if ( m_msecElapsed > m_msecDuration )
		{
			m_msecElapsed = m_msecDuration ;
		}
		const double	t = (double) m_msecElapsed / (double) m_msecDuration ;
		//
		S3DDVector	vCamera = pV3D->m_vCameraView ;
		S3DDVector	vTarget = pV3D->m_vCameraTarget ;
		//
		if ( m_bzPos.GetLength() >= 4 )
		{
			vCamera = m_bzPos.PointAt( t ) ;
		}
		if ( m_bzTarget.GetLength() >= 4 )
		{
			vTarget = m_bzTarget.PointAt( t ) ;
		}
		sprite.SetVirtualCamera( vCamera, vTarget ) ;
		//
		if ( m_msecElapsed >= m_msecDuration )
		{
			m_msecDuration = 0 ;
			m_msecElapsed = 0 ;
			m_bzPos.RemoveAll() ;
			m_bzTarget.RemoveAll() ;
		}
	}
	return	false ;
}



//////////////////////////////////////////////////////////////////////////////
// 表示画像制御
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSprite::Imager, SGLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSprite::Imager::Imager( void )
{
}

// 関連付け時処理
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::Imager::OnAttached( SGLSprite& sprite )
{
}

// アニメーション処理
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::Imager::OnAnimation( SGLSprite& sprite, uint32_t msecPast )
{
}


//////////////////////////////////////////////////////////////////////////////
// 画像アニメーション
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSpriteAnimator, Imager )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteAnimator::SGLSpriteAnimator( void )
{
	m_countLoop = -1 ;
	m_iLoopStart = 0 ;
	m_iLoopEnd = -1 ;
	m_msecDuration = 0 ;
	m_iFrame = 0 ;
	m_msecFrameDelta = 0 ;
}

SGLSpriteAnimator::SGLSpriteAnimator( const SGLSpriteAnimator& anime )
	: m_strFilePath( anime.m_strFilePath ), m_tableSeq( anime.m_tableSeq )
{
	if ( anime.m_pAnimation != NULL )
	{
		m_pAnimation =
			anime.m_pAnimation->
					NewReference( NULL, -1, stereoImageRight ) ;
	}
	if ( anime.m_pLeftAnimation != NULL )
	{
		m_pLeftAnimation =
			anime.m_pLeftAnimation->
					NewReference( NULL, -1, stereoImageLeft ) ;
	}
	m_countLoop = anime.m_countLoop ;
	m_iLoopStart = anime.m_iLoopStart ;
	m_iLoopEnd = anime.m_iLoopEnd ;
	m_msecDuration = anime.m_msecDuration ;
	m_msecFrameDelta = anime.m_msecFrameDelta ;
	SelectFrame( anime.m_iFrame ) ;
}

// 関連付け時処理
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteAnimator::OnAttached( SGLSprite& sprite )
{
	AttachImageToSprite( sprite ) ;
}

// アニメーション処理
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteAnimator::OnAnimation( SGLSprite& sprite, uint32_t msecPast )
{
	SGLImageObject *	pImage = m_pAnimation ;
	if ( (pImage != NULL) && (m_countLoop != 0) )
	{
		//
		// 時間更新
		//
		size_t	msecDuration = m_msecDuration ;
		size_t	nFrameCount = m_tableSeq.GetLength() ;
		if ( msecDuration == 0 )
		{
			msecDuration = (ssize_t) pImage->GetTotalTime() ;
			if ( msecDuration == 0 )
			{
				return ;
			}
		}
		if ( nFrameCount <= 1 )
		{
			nFrameCount = pImage->GetFrameCount() ;
			if ( nFrameCount <= 1 )
			{
				return ;
			}
		}
		m_msecFrameDelta += msecPast ;
		//
		size_t	iDeltaFrame =
					m_msecFrameDelta * nFrameCount / msecDuration ;
		if ( iDeltaFrame != 0 )
		{
			m_iFrame += iDeltaFrame ;
			m_msecFrameDelta -= iDeltaFrame * msecDuration / nFrameCount ;
			//
			// ループ処理
			//
			size_t	iLoopEnd = m_iLoopEnd ;
			if ( iLoopEnd == 0 )
			{
				iLoopEnd = nFrameCount ;
			}
			if ( m_iFrame >= iLoopEnd )
			{
				m_iFrame -= iLoopEnd ;
				if ( m_countLoop > 0 )
				{
					m_countLoop -- ;
				}
				if ( m_iLoopStart < iLoopEnd )
				{
					ssize_t	nLoop = (ssize_t) (m_iFrame / (iLoopEnd - m_iLoopStart)) ;
					m_iFrame %= (iLoopEnd - m_iLoopStart) ;
					if ( m_countLoop > 0 )
					{
						m_countLoop -= nLoop ;
						if ( m_countLoop <= 0 )
						{
							m_countLoop = 0 ;
						}
					}
				}
				if ( m_countLoop == 0 )
				{
					m_iFrame = iLoopEnd - 1 ;
				}
				else
				{
					m_iFrame += m_iLoopStart ;
				}
			}
			//
			// フレーム更新
			//
			SelectFrame( m_iFrame ) ;
			sprite.NotifyUpdate() ;
		}
	}
}

// 画像ファイル読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteAnimator::LoadImage( const wchar_t * pwszFilePath )
{
	SGLImage *	pImage = new SGLImage ;
	if ( pImage->LoadImage( pwszFilePath ) )
	{
		delete	pImage ;
		return	sglErrFailed ;
	}
	m_strFilePath = pwszFilePath ;
	m_pAnimation = pImage ;
	m_pLeftAnimation = pImage->NewReference( NULL, -1, stereoImageLeft ) ;
	//
	size_t	nLength = pImage->GetSequenceLength() ;
	m_tableSeq.SetLength( nLength ) ;
	nLength = pImage->GetSequenceTable( m_tableSeq.GetArray(), nLength ) ;
	m_tableSeq.FinishArray() ;
	m_tableSeq.SetLength( nLength ) ;
	//
	BeginAnimation() ;
	//
	return	sglErrSuccess ;
}

// 画像を関連付ける
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteAnimator::AttachImage
	( SGLImageObject* pImage, const SGLImageRect * pClip )
{
	m_strFilePath.FreeArray() ;
	if ( pImage != NULL )
	{
		m_pAnimation = pImage->NewReference( pClip, -1, stereoImageRight ) ;
		m_pLeftAnimation = pImage->NewReference( pClip, -1, stereoImageLeft ) ;
		//
		size_t	nLength = pImage->GetSequenceLength() ;
		m_tableSeq.SetLength( nLength ) ;
		nLength = pImage->GetSequenceTable( m_tableSeq.GetArray(), nLength ) ;
		m_tableSeq.FinishArray() ;
		m_tableSeq.SetLength( nLength ) ;
		//
		BeginAnimation() ;
	}
	else
	{
		m_tableSeq.FreeArray() ;
		m_pAnimation = NULL ;
		m_pLeftAnimation = NULL ;
		m_countLoop = 0 ;
	}
}

// アニメーション開始
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteAnimator::BeginAnimation
	( ssize_t countLoop,
		size_t iLoopStart, size_t iLoopEnd,
		size_t iStartFrame, size_t msecDuration )
{
	m_countLoop = countLoop ;
	m_iLoopStart = iLoopStart ;
	m_iLoopEnd = iLoopEnd ;
	m_iFrame = iStartFrame ;
	m_msecDuration = msecDuration ;
	m_msecFrameDelta = 0 ;
}

// ループ設定変更
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteAnimator::SetLoop
	( ssize_t countLoop, size_t iLoopStart, size_t iLoopEnd )
{
	m_countLoop = countLoop ;
	m_iLoopStart = iLoopStart ;
	m_iLoopEnd = iLoopEnd ;
}

// フレーム選択
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteAnimator::SelectFrame( size_t iFrame )
{
	SGLImageObject *	pImage = m_pAnimation ;
	if ( iFrame < m_tableSeq.GetLength() )
	{
		iFrame = m_tableSeq.At( iFrame ) ;
	}
	if ( pImage != NULL )
	{
		pImage->SelectFrame( iFrame ) ;
	}
	SGLImageObject *	pLeftImage = m_pLeftAnimation ;
	if ( pLeftImage != NULL )
	{
		pLeftImage->SelectFrame( iFrame ) ;
	}
	m_iFrame = iFrame ;
}

// 画像を関連付ける
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteAnimator::AttachImageToSprite( SGLSprite& sprite )
{
	sprite.AttachImage( m_pAnimation, m_pLeftAnimation ) ;
}

// 複製（可能なら）
//////////////////////////////////////////////////////////////////////////////
SGLObject * SGLSpriteAnimator::DuplicateObject( void )
{
	return	new SGLSpriteAnimator( *this ) ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteAnimator::OnSave( SSystem::SFileInterface& file )
{
	SGLError	err = Imager::OnSave( file ) ;
	if ( err )
	{
		return	err ;
	}
	err = (SGLError) file.WriteString( m_strFilePath ) ;
	if ( err )
	{
		return	err ;
	}
	//
	size_t	nWritten = 0 ;
	nWritten += file.Write( &m_countLoop, sizeof(ssize_t) ) ;
	nWritten += file.Write( &m_iLoopStart, sizeof(size_t) ) ;
	nWritten += file.Write( &m_iLoopEnd, sizeof(size_t) ) ;
	nWritten += file.Write( &m_msecDuration, sizeof(size_t) ) ;
	nWritten += file.Write( &m_msecFrameDelta, sizeof(size_t) ) ;
	nWritten += file.Write( &m_iFrame, sizeof(size_t) ) ;
	//
	if ( nWritten < sizeof(size_t) * 6 )
	{
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteAnimator::OnRestore( SSystem::SFileInterface& file )
{
	SGLError	err = Imager::OnRestore( file ) ;
	if ( err )
	{
		return	err ;
	}
	SString	strFilePath ;
	err = (SGLError) file.ReadString( strFilePath ) ;
	if ( err )
	{
		return	err ;
	}
	if ( !strFilePath.IsEmpty() )
	{
		if ( LoadImage( strFilePath ) )
		{
			SArray<char>	bufFilePath ;
			SSystem::Trace
				( "failed to load image \'%s\'\n",
						strFilePath.EncodeDefaultTo(bufFilePath) ) ;
		}
	}
	//
	size_t	nRead = 0 ;
	nRead += file.Read( &m_countLoop, sizeof(ssize_t) ) ;
	nRead += file.Read( &m_iLoopStart, sizeof(size_t) ) ;
	nRead += file.Read( &m_iLoopEnd, sizeof(size_t) ) ;
	nRead += file.Read( &m_msecDuration, sizeof(size_t) ) ;
	nRead += file.Read( &m_msecFrameDelta, sizeof(size_t) ) ;
	nRead += file.Read( &m_iFrame, sizeof(size_t) ) ;
	//
	if ( nRead < sizeof(size_t) * 6 )
	{
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}


//////////////////////////////////////////////////////////////////////////////
// 立体視用インデックス変換
//////////////////////////////////////////////////////////////////////////////

RenderContext::StereoViewIndex SGLSprite::ToStereoViewIndex( Stereo3DView s3dView )
{
	switch( s3dView )
	{
	case	s3dRightView:
		return	RenderContext::stereoViewRight ;

	case	s3dLeftView:
		return	RenderContext::stereoViewLeft ;

	default:
		break ;
	}
	return	RenderContext::stereoViewAuto ;
}

SGLSprite::Stereo3DView SGLSprite::FromStereoViewIndex( RenderContext::StereoViewIndex sviView )
{
	switch( sviView )
	{
	case	RenderContext::stereoViewRight:
		return	s3dRightView ;

	case	RenderContext::stereoViewLeft:
		return	s3dLeftView ;

	default:
		break ;
	}
	return	s3dMonoview ;
}



//////////////////////////////////////////////////////////////////////////////
// 描画バッファ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSprite::Buffer, SGLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSprite::Buffer::Buffer( SGLPaintContextType type )
	: m_render( RenderContext::NewContext(type), true ), m_typePaint( type )
{
	m_flagStereo3D = false ;
	m_flagZBuffer = false ;
	m_flagMultiSampling = false ;
	m_flagFillBack = true ;
	m_rgbaFillBack = 0 ;
	m_nBufFlags = SGLImageObject::bufferOnMemory ;
}

SGLSprite::Buffer::Buffer( const SGLSprite::Buffer& buf )
	: m_render( RenderContext::NewContext(buf.m_typePaint), true ),
		m_typePaint( buf.m_typePaint )
{
	m_flagStereo3D = false ;
	m_flagZBuffer = false ;
	m_flagMultiSampling = false ;
	m_flagFillBack = buf.m_flagFillBack ;
	m_rgbaFillBack = buf.m_rgbaFillBack ;
	m_nBufFlags = buf.m_nBufFlags ;
	//
	SGLImageInfo	imginf ;
	if ( buf.m_imgBuffer.GetImageInfo( imginf ) == sglErrSuccess )
	{
		CreateBuffer
			( imginf.width, imginf.height,
				imginf.format, imginf.depth,
				buf.m_nBufFlags,
				buf.m_flagZBuffer, buf.m_flagStereo3D ) ;
	}
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSprite::Buffer::~Buffer( void )
{
}

// バッファ生成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSprite::Buffer::CreateBuffer
	( uint32_t width, uint32_t height,
		uint32_t format, uint32_t depth,
		uint64_t nBufFlags, bool flagZBuffer, bool flagStereo3D )
{
	m_imgBuffer.ReleaseBuffer() ;
	m_imgLeftBuffer.ReleaseBuffer() ;
	m_imgZBuffer.ReleaseBuffer() ;
	m_flagStereo3D = false ;
	m_flagZBuffer = false ;
	m_flagMultiSampling = ((nBufFlags & SGLImageObject::bufferRenderNonTextureFlags) != 0) ;
	m_nBufFlags = SGLImageObject::bufferNonPowerOf2 | nBufFlags ;
	nBufFlags = SGLImageObject::bufferNonPowerOf2
					| (nBufFlags & ~uint64_t(SGLImageObject::bufferRenderNonTextureFlags)) ;
	//
	SGLImageInfo	imginf ;
	imginf.format = format ;
	imginf.width = width ;
	imginf.height = height ;
	imginf.depth = depth ;
	if ( m_imgBuffer.CreateBuffer( imginf, nBufFlags ) )
	{
		return	sglErrFailed ;
	}
	if ( flagStereo3D )
	{
		if ( !m_imgLeftBuffer.CreateBuffer( imginf, nBufFlags ) )
		{
			m_flagStereo3D = true ;
		}
	}
	if ( flagZBuffer )
	{
		SGLImageInfo	imginfDepth = imginf ;
		imginfDepth.format = formatImageDepth ;
		imginfDepth.depth = 32 ;
		if ( !m_imgZBuffer.CreateBuffer( imginfDepth, nBufFlags ) )
		{
			m_flagZBuffer = true ;
		}
	}
	if ( m_flagMultiSampling )
	{
		PrepareMultiSampling() ;
	}
	return	sglErrSuccess ;
}

void SGLSprite::Buffer::PrepareMultiSampling( void )
{
	SGLImageInfo	imginf ;
	if ( m_flagMultiSampling
		&& !m_imgBuffer.GetImageInfo(imginf) )
	{
		SGLImageInfo	imginfDepth = imginf ;
		imginfDepth.format = formatImageDepth ;
		imginfDepth.depth = 32 ;
		//
		m_imgTempBuffer.CreateBuffer( imginf, m_nBufFlags ) ;
		m_imgTempLeftBuffer.CreateBuffer( imginf, m_nBufFlags ) ;
		m_imgTempZBuffer.CreateBuffer( imginfDepth, m_nBufFlags ) ;
		//
		if ( m_render.GetRenderDeviceObject()
				!= m_renderTemp.GetRenderDeviceObject() )
		{
			S3DRenderDevice *	pDevice = m_render.GetRenderDeviceObject() ;
			if ( pDevice != nullptr )
			{
				m_renderTemp.AttachRenderContext( pDevice->NewRenderer(), true ) ;
			}
		}
	}
}

// 描画ターゲット設定
//////////////////////////////////////////////////////////////////////////////
SGLImageObject * SGLSprite::Buffer::AttachRenderTarget
	( SGLSprite::Stereo3DView s3dView, const SGLImageRect * pRectView )
{
	SGLImageObject *	pImage = m_flagMultiSampling ? &m_imgTempBuffer : &m_imgBuffer ;
	if ( m_flagStereo3D && (s3dView == s3dLeftView) )
	{
		pImage = m_flagMultiSampling ? &m_imgTempLeftBuffer : &m_imgLeftBuffer ;
	}
	SGLImageObject *	pZBuffer = NULL ;
	if ( m_flagZBuffer )
	{
		pZBuffer = m_flagMultiSampling ? &m_imgTempZBuffer : &m_imgZBuffer ;
	}
	if ( m_nBufFlags & SGLImageObject::bufferOnDeviceOnly )
	{
		m_render.AttachTargetImage( pImage, pZBuffer, nullptr ) ;
	}
	else
	{
		m_render.AttachTargetImage( pImage, pZBuffer, pRectView ) ;
	}
	m_render.ResetTransformation() ;
	return	pImage ;
}

// 描画ターゲット解除
//////////////////////////////////////////////////////////////////////////////
SGLImageObject * SGLSprite::Buffer::DetachRenderTarget
	( SGLSprite::Stereo3DView s3dView, const SGLImageRect * pRectView )
{
	SGLImageObject *	pImage =
			(s3dView == s3dLeftView) ? &m_imgLeftBuffer : &m_imgBuffer ;
	m_render.Finish() ;
	if ( m_flagMultiSampling )
	{
		m_renderTemp.AttachTargetImage( pImage, &m_imgZBuffer, nullptr ) ;
		m_renderTemp.CopyBufferFrom
			( m_render, RenderContext::copyBufferColor
						| RenderContext::copyBufferDepth ) ;
		m_renderTemp.DetachTargetImage() ;
	}
	m_render.DetachTargetImage() ;
	return	pImage ;
}

// レンダラ設定
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::Buffer::SetFrameRenderer
		( RenderContext * render, bool flagOwner )
{
	m_render.AttachRenderContext( render, flagOwner ) ;
	//
	if ( m_flagMultiSampling
		&& (m_render.GetRenderDeviceObject()
				!= m_renderTemp.GetRenderDeviceObject()) )
	{
		S3DRenderDevice *	pDevice = m_render.GetRenderDeviceObject() ;
		if ( pDevice != nullptr )
		{
			m_renderTemp.AttachRenderContext( pDevice->NewRenderer(), true ) ;
		}
	}
}

// 背景色
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::Buffer::SetFillBack( SGLPalette rgbaFillBack )
{
	m_flagFillBack = true ;
	m_rgbaFillBack = rgbaFillBack ;
}

void SGLSprite::Buffer::DisableFillBack( void )
{
	m_flagFillBack = false ;
}

bool SGLSprite::Buffer::IsFillBack( void ) const
{
	return	m_flagFillBack ;
}

const SGLPalette& SGLSprite::Buffer::GetFillBackColor( void ) const
{
	return	m_rgbaFillBack ;
}

// レンダラ取得
//////////////////////////////////////////////////////////////////////////////
S3DRenderContext& SGLSprite::Buffer::Renderer( void )
{
	return	m_render ;
}

// カラーバッファ
//////////////////////////////////////////////////////////////////////////////
SGLImage * SGLSprite::Buffer::GetImage( void )
{
	return	&m_imgBuffer ;
}

SGLImage * SGLSprite::Buffer::GetLeftImage( void )
{
	return	&m_imgLeftBuffer ;
}

// ｚバッファ取得
//////////////////////////////////////////////////////////////////////////////
SGLImage* SGLSprite::Buffer::GetZBuffer( void )
{
	return	&m_imgZBuffer ;
}

// バッファフラグ
//////////////////////////////////////////////////////////////////////////////
uint64_t SGLSprite::Buffer::GetBufFlags( void ) const
{
	return	m_nBufFlags ;
}

// ステレオ立体視か？
//////////////////////////////////////////////////////////////////////////////
bool SGLSprite::Buffer::IsStereo3D( void ) const
{
	return	m_flagStereo3D ;
}

// ｚバッファ
//////////////////////////////////////////////////////////////////////////////
bool SGLSprite::Buffer::HasZBuffer( void ) const
{
	return	m_flagZBuffer ;
}

// マルチサンプリング
//////////////////////////////////////////////////////////////////////////////
bool SGLSprite::Buffer::IsMultisampling( void ) const
{
	return	m_flagMultiSampling ;
}

// 一時バッファ（レンダリング対象）
//////////////////////////////////////////////////////////////////////////////
SGLImage * SGLSprite::Buffer::GetTempImage( void )
{
	return	&m_imgTempBuffer ;
}

SGLImage * SGLSprite::Buffer::GetTempLeftImage( void )
{
	return	&m_imgTempLeftBuffer ;
}

SGLImage * SGLSprite::Buffer::GetTempZBuffer( void )
{
	return	&m_imgTempZBuffer ;
}

// 複製
//////////////////////////////////////////////////////////////////////////////
SGLObject * SGLSprite::Buffer::DuplicateObject( void )
{
	return	new Buffer( *this ) ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSprite::Buffer::OnSave( SSystem::SFileInterface& file )
{
	uint32_t	nFlags = 0x10 ;
	if ( m_flagStereo3D )
	{
		nFlags |= 0x01 ;
	}
	if ( m_flagZBuffer )
	{
		nFlags |= 0x02 ;
	}
	if ( m_flagFillBack )
	{
		nFlags |= 0x04 ;
	}
	file.Write( &nFlags, sizeof(uint32_t) ) ;
	//
	uint32_t	nPaintType = m_typePaint ;
	file.Write( &nPaintType, sizeof(uint32_t) ) ;
	//
	uint32_t	argbFillBack = (uint32_t) m_rgbaFillBack ;
	file.Write( &argbFillBack, sizeof(uint32_t) ) ;
	//
	file.Write( &m_nBufFlags, sizeof(uint64_t) ) ;
	//
	SGLImageInfo	imginf ;
	m_imgBuffer.GetImageInfo( imginf ) ;
	file.Write( &imginf.format, sizeof(uint32_t) ) ;
	file.Write( &imginf.depth, sizeof(uint32_t) ) ;
	file.Write( &imginf.width, sizeof(uint32_t) ) ;
	file.Write( &imginf.height, sizeof(uint32_t) ) ;
	//
	return	sglErrSuccess ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSprite::Buffer::OnRestore( SSystem::SFileInterface& file )
{
	uint32_t	nFlags = 0 ;
	uint32_t	nPaintType ;
	uint32_t	argbFillBack ;
	uint64_t	nBufFlags = 0 ;
	uint32_t	format, depth, width, height ;
	file.Read( &nFlags, sizeof(uint32_t) ) ;
	file.Read( &nPaintType, sizeof(uint32_t) ) ;
	file.Read( &argbFillBack, sizeof(uint32_t) ) ;
	if ( nFlags & 0x10 )
	{
		file.Read( &nBufFlags, sizeof(uint64_t) ) ;
	}
	else
	{
		file.Read( &nBufFlags, sizeof(int32_t) ) ;
	}
	file.Read( &format, sizeof(uint32_t) ) ;
	file.Read( &depth, sizeof(uint32_t) ) ;
	file.Read( &width, sizeof(uint32_t) ) ;
	file.Read( &height, sizeof(uint32_t) ) ;
	//
	if ( m_typePaint != (SGLPaintContextType) nPaintType )
	{
		m_typePaint = (SGLPaintContextType) nPaintType ;
		m_render.AttachRenderContext
			( RenderContext::NewContext(m_typePaint), true ) ;
	}
	m_flagFillBack = ((nFlags & 0x04) != 0) ;
	m_rgbaFillBack = argbFillBack ;
	//
	CreateBuffer
		( width, height, format, depth, nBufFlags,
			((nFlags & 0x02) != 0), ((nFlags & 0x01) != 0) ) ;
	//
	return	sglErrSuccess ;
}


//////////////////////////////////////////////////////////////////////////////
// 疑似 3D 表示用パラメータ
//////////////////////////////////////////////////////////////////////////////

SGLSprite::Virtual3DParam::Virtual3DParam( void )
: m_vProjectScreen(320,240,512),
	m_zProjectScale(1.0), m_fpPixelAspect(1.0),
	m_xParallax(10.0), m_zParallaxFocus(1.0),
	m_vCameraView(0,0,0), m_vCameraTarget(0,0,512)
{
}

SGLSprite::Virtual3DParam::Virtual3DParam( const SGLSprite::Virtual3DParam& src )
: m_vProjectScreen(src.m_vProjectScreen),
	m_zProjectScale(src.m_zProjectScale),
	m_fpPixelAspect(src.m_fpPixelAspect),
	m_xParallax(src.m_xParallax), m_zParallaxFocus(src.m_zParallaxFocus),
	m_vCameraView(src.m_vCameraView), m_vCameraTarget(src.m_vCameraTarget)
{
}



//////////////////////////////////////////////////////////////////////////////
// 表示パラメータ
//////////////////////////////////////////////////////////////////////////////

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSprite::Parameter::SaveWithoutPointer( SSystem::SFileInterface& file ) const
{
	file.Write( &nFlags, sizeof(uint32_t) ) ;
	file.Write( &nSpriteFlags, sizeof(uint32_t) ) ;
	file.Write( &vDst, sizeof(S3DDVector) ) ;
	file.Write( &vCenter, sizeof(S2DDVector) ) ;
	file.Write( &zAngle, sizeof(double) ) ;
	file.Write( &xyCross, sizeof(double) ) ;
	file.Write( &nTransparency, sizeof(uint32_t) ) ;
	file.Write( &paramFilter, sizeof(int32_t) ) ;
	file.Write( &paramFilter2, sizeof(int32_t) ) ;
	file.Write( &rgbColorParam, sizeof(SGLPalette) ) ;
	//
	uint32_t	nVertex = (uint32_t) countVertex ;
	file.Write( &nVertex, sizeof(uint32_t) ) ;
	uint32_t	nVertexNull = 0 ;
	file.Write( &nVertexNull, sizeof(uint32_t) ) ;
	//
	return	sglErrSuccess ;
}

// デシリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSprite::Parameter::LoadWithoutPointer( SSystem::SFileInterface& file )
{
	file.Read( &nFlags, sizeof(uint32_t) ) ;
	file.Read( &nSpriteFlags, sizeof(uint32_t) ) ;
	file.Read( &vDst, sizeof(S3DDVector) ) ;
	file.Read( &vCenter, sizeof(S2DDVector) ) ;
	file.Read( &zAngle, sizeof(double) ) ;
	file.Read( &xyCross, sizeof(double) ) ;
	file.Read( &nTransparency, sizeof(uint32_t) ) ;
	file.Read( &paramFilter, sizeof(int32_t) ) ;
	file.Read( &paramFilter2, sizeof(int32_t) ) ;
	file.Read( &rgbColorParam, sizeof(SGLPalette) ) ;
	//
	uint32_t	nVertex ;
	file.Read( &nVertex, sizeof(uint32_t) ) ;
	countVertex = (size_t) nVertex ;
	//
	uint32_t	nVertexNull = 0 ;
	file.Read( &nVertexNull, sizeof(uint32_t) ) ;
	//
	return	sglErrSuccess ;
}


//////////////////////////////////////////////////////////////////////////////
// スプライト基底
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSprite, SGLObject )

// デフォルト表示フラグ
uint32_t	SGLSprite::m_nDefParamFlags = paintSmoothStretch ;

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSprite::SGLSprite( void )
{
	m_paramView.nFlags = SGLSprite::m_nDefParamFlags ;
	m_flagsUI = 0 ;
	m_visible = true ;
	#if	defined(__DEBUG__)
	m_debug = false ;
	#endif
	m_priority = 0 ;
	m_statusUpdate = updateEmpty ;
	m_countFrozen = 0 ;
	m_pMutexUI = SSystem::g_mutexGlobal ;
	ESLAssert( m_pMutexUI != NULL ) ;
}

SGLSprite::SGLSprite( const SGLSprite& src )
{
	m_pMutexUI = SSystem::g_mutexGlobal ;
	//
	DuplicateReferenceArray<SGLSpriteFilter>( m_filters, src.m_filters ) ;
	DuplicateReferenceArray<SGLSpriteTimer>( m_timers, src.m_timers ) ;
	DuplicateObjectArray<Action>( m_actions, src.m_actions ) ;
	if ( src.m_pDrawer != NULL )
	{
		ssize_t	iDrawer =
			src.m_filters.FindPtr
				( ESLTypeCast<SGLSpriteFilter>( src.m_pDrawer.Ptr() ) ) ;
		if ( iDrawer >= 0 )
		{
			SGLSpriteFilter *
				pFilter = m_filters.DetachAt( (size_t) iDrawer ) ;
			m_pDrawer = pFilter ;
			m_filters.InsertAt( (size_t) iDrawer, pFilter ) ;
		}
		else
		{
			m_pDrawer = SGLSmartCast<SGLSpriteDrawer>
							( src.m_pDrawer->DuplicateObject() ) ;
		}
		if ( m_pDrawer.Ptr() != nullptr )
		{
			m_pDrawer->OnAttachedSprite( this ) ;
		}
	}
	m_paramAction = src.m_paramAction ;
	//
	if ( src.m_pImager != NULL )
	{
		SetSpriteImager
			( SGLSmartCast<Imager>( src.m_pImager->DuplicateObject() ) ) ;
	}
	if ( src.m_pBuffer != NULL )
	{
		m_pBuffer = SGLSmartCast<Buffer>
						( src.m_pBuffer->DuplicateObject() ) ;
		if ( m_pBuffer != NULL )
		{
			m_refImage = m_pBuffer->GetImage() ;
			m_refLeftImage = NULL ;
			if ( m_pBuffer->IsStereo3D() )
			{
				m_refLeftImage = m_pBuffer->GetLeftImage() ;
			}
		}
	}
	m_paramView = src.m_paramView ;
	m_paramVertex = src.m_paramVertex ;
	if ( m_paramView.pVertices != NULL )
	{
		m_paramView.pVertices = m_paramVertex.GetConstArray() ;
	}
	m_flagsUI = src.m_flagsUI ;
	m_visible = src.m_visible ;
	#if	defined(__DEBUG__)
	m_debug = false ;
	#endif
	m_priority = src.m_priority ;
	m_strID = src.m_strID ;
	m_statusUpdate = updateFull ;
	m_countFrozen = 0 ;
	//
	if ( src.m_pVirtual3D != NULL )
	{
		m_pVirtual3D = new Virtual3DParam( *src.m_pVirtual3D ) ;
	}
	//
	const size_t	nChildren = src.m_children.GetLength() ;
	m_children.RemoveAll() ;
	for ( size_t i = 0; i < nChildren; i ++ )
	{
		SGLSprite *	pChild = src.m_children.GetAt( i ) ;
		if ( pChild != NULL )
		{
			AddSmartChild
				( SGLSmartCast<SGLSprite>( pChild->DuplicateObject() ) ) ;
		}
	}
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSprite::~SGLSprite( void )
{
	SGLSprite *	pParent = m_refParent.GetReference() ;
	if ( pParent != NULL )
	{
		if ( LockTrace( __FILE__, __LINE__, 100 ) == errSuccess )
		{
			pParent->DetachChild( this ) ;
			Unlock() ;
		}
	}
	SGLSprite::AsyncRemoveAllChildren() ;
}

// 時間経過処理
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::AdvanceTime( uint32_t msecPast )
{
	Imager *	pImager = m_pImager ;
	if ( pImager != NULL )
	{
		pImager->OnAnimation( *this, msecPast ) ;
	}
	if ( m_filters.GetLength() > 0 )
	{
		const size_t	nCount = m_filters.GetLength() ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			SGLSpriteFilter *	pFilter = m_filters.GetAt( i ) ;
			if ( pFilter != NULL )
			{
				pFilter->OnTimer( *this, msecPast ) ;
			}
		}
	}
	if ( m_timers.GetLength() > 0 )
	{
		const size_t	nCount = m_timers.GetLength() ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			SGLSpriteTimer *	pTimer = m_timers.GetAt( i ) ;
			if ( pTimer != NULL )
			{
				if ( pTimer->OnTimer( *this, msecPast ) )
				{
					m_timers.SetAt( i, NULL ) ;
				}
			}
		}
		m_timers.TrimEmpty() ;
	}
	if ( m_actions.GetLength() > 0 )
	{
		UpdateAllActions( msecPast ) ;
	}
	SReferenceArray<SGLSprite>::Iterator	iter( m_children, 0 ) ;
	while ( iter.HasNext() )
	{
		SGLSprite *	pChild = iter.Next() ;
		if ( pChild != nullptr )
		{
			pChild->AdvanceTime( msecPast ) ;
		}
	}
}

// フレーム描画（視点に関係しない）共通処理
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::PrepareDrawFrame( void )
{
	const size_t	countChildren = m_children.GetLength() ;
	if ( countChildren == 0 )
	{
		return ;
	}
	SReferenceArray<SGLSprite>::Iterator	iter = m_children.End() ;
	while ( iter.HasPrev() )
	{
		SGLSprite *	pChild = iter.Prev() ;
		if ( pChild == nullptr )
		{
			m_children.RemoveAt( iter.Index() ) ;
			continue ;
		}
		else if ( !(pChild->m_visible) )
		{
			continue ;
		}
		pChild->PrepareDrawFrame() ;
	}
}

// フレーム描画完了後処理
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::FinishDrawFrame( void )
{
	const size_t	countChildren = m_children.GetLength() ;
	if ( countChildren == 0 )
	{
		return ;
	}
	SReferenceArray<SGLSprite>::Iterator	iter = m_children.End() ;
	while ( iter.HasPrev() )
	{
		SGLSprite *	pChild = iter.Prev() ;
		if ( pChild == nullptr )
		{
			m_children.RemoveAt( iter.Index() ) ;
			continue ;
		}
		else if ( !(pChild->m_visible) )
		{
			continue ;
		}
		pChild->FinishDrawFrame() ;
	}
}

// 描画前処理
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::BeforeDraw( SGLSprite::Stereo3DView s3dView )
{
	#if	defined(__DEBUG__)
	if ( m_debug )
	{
		Trace( "SGLSprite::BeforeDraw this=%X\n", this ) ;
	}
	#endif
	if ( !m_visible )
	{
		return ;
	}
	//
	// ローカルバッファへの描画
	//
	Buffer *	pBuffer = GetFrameBuffer() ;
	if ( pBuffer != NULL )
	{
		Refresh( s3dView ) ;
	}
	else
	{
		// 事前にすべての子スプライトに対し BeforeDraw を呼び出す
		BeforeDrawChildren( s3dView ) ;
	}
	//
	// 画像の更新確定
	// ※OpenGL 等のテクスチャを更新し
	// 　描画の合間にテクスチャの更新が
	// 　発生するのをできるだけ抑制する
	//
	SGLImageObject *	pImage = NULL ;
	if ( s3dView != s3dLeftView )
	{
		pImage = m_refImage.GetReference() ;
		if ( s3dView != s3dRightView )
		{
			m_statusUpdate = updateEmpty ;
		}
	}
	else
	{
		pImage = m_refLeftImage.GetReference() ;
		m_statusUpdate = updateEmpty ;
	}
	if ( pImage != NULL )
	{
		pImage->FlushImageObject() ;
	}
}

// 描画
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::Draw
	( S3DRenderContextInterface& render,
		const SGLSprite::Virtual3DParam* pV3D,
		SGLSprite::Stereo3DView s3dView ) const
{
	#if	defined(__DEBUG__)
	if ( m_debug )
	{
		Trace( "SGLSprite::Draw this=%X\n", this ) ;
	}
	#endif
	if ( !m_visible )
	{
		return ;
	}
	SGLPaintParam	pp ;
	SGLAffine		affine ;
	if ( !GetPaintParam( pp, affine, pV3D, s3dView ) )
	{
		return ;
	}
	//
	SGLImageObject *	pImage = NULL ;
	if ( s3dView != s3dLeftView )
	{
		pImage = m_refImage.GetReference() ;
	}
	else
	{
		pImage = m_refLeftImage.GetReference() ;
		if ( pImage == NULL )
		{
			pImage = m_refImage.GetReference() ;
		}
	}
	if ( m_pDrawer != NULL )
	{
		m_pDrawer->Draw( render, pp, pImage ) ;
	}
	else
	{
		SGLSpriteFilter *	pFilter = m_filters.GetLastAt( 0 ) ;
		if ( (pFilter != NULL) && pFilter->IsDynamicDrawer() )
		{
			pFilter->Draw( render, pp, pImage ) ;
		}
		else
		{
			DrawSprite( render, pp, pImage ) ;
		}
	}
	//
	if ( GetFrameBuffer() == NULL )
	{
		render.PushTransformation() ;
		render.AppendTransformation( affine, pp.nTransparency ) ;
		DrawChildren( render, s3dView ) ;
		render.PopTransformation() ;
	}
}

// 描画後処理
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::AfterDraw( SGLSprite::Stereo3DView s3dView )
{
	#if	defined(__DEBUG__)
	m_debug = false ;
	#endif
}

// SGLDrawImageParamList への描画
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::DrawImageList
	( SGLDrawImageParamList& dipl,
		const SGLSprite::Virtual3DParam* pV3D,
		SGLSprite::Stereo3DView s3dView )
{
	#if	defined(__DEBUG__)
	if ( m_debug )
	{
		Trace( "SGLSprite::Draw this=%X\n", this ) ;
	}
	#endif
	if ( !m_visible )
	{
		return ;
	}
	SGLPaintParam	pp ;
	SGLAffine		affine ;
	if ( !GetPaintParam( pp, affine, pV3D, s3dView ) )
	{
		return ;
	}
	//
	SGLImageObject *	pImage = NULL ;
	if ( s3dView != s3dLeftView )
	{
		pImage = m_refImage.GetReference() ;
	}
	else
	{
		pImage = m_refLeftImage.GetReference() ;
		if ( pImage == NULL )
		{
			pImage = m_refImage.GetReference() ;
		}
	}
	dipl.AddDrawParam( pp, pImage, NULL );
	//
	if ( GetFrameBuffer() == NULL )
	{
		SGLAffine	affSave = dipl.GetAffine() ;
		uint32_t	nTransSave = dipl.GetTransparency() ;
		//
		dipl.AppendAffine( affine ) ;
		dipl.AppendTransparency( pp.nTransparency ) ;
		//
		DrawChildrenImageList( dipl, s3dView ) ;
		//
		dipl.SetAffine( affSave ) ;
		dipl.SetTransparency( nTransSave ) ;
	}
}

// 中間バッファへ描画
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::Refresh( SGLSprite::Stereo3DView s3dView )
{
	Buffer *	pBuffer = GetFrameBuffer() ;
	if ( (m_statusUpdate == updateEmpty) || (pBuffer == NULL) )
	{
		return ;
	}
	if ( AtomicAdd( &m_countFrozen, 1 ) > 1 )
	{
		AtomicSub( &m_countFrozen, 1 ) ;
		return ;
	}
	//
	// 事前にすべての子スプライトに対し BeforeDraw を呼び出す
	//
	BeforeDrawChildren( s3dView ) ;
	//
	// 描画中間バッファ設定
	//
	SGLImageRect *	pViewRect = nullptr ;
	SGLImageRect	rectUpdate( m_rectUpdate ) ;
	if ( m_statusUpdate == updateRect )
	{
		pViewRect = &rectUpdate ;
	}
	SGLImageObject *	pImage = pBuffer->AttachRenderTarget( s3dView, pViewRect ) ;
	//
	Virtual3DParam *	pV3D = m_pVirtual3D ;
	if ( pV3D != NULL )
	{
		pBuffer->Renderer().SetProjectionScreen
			( pV3D->m_vProjectScreen,
				pV3D->m_zProjectScale, pV3D->m_fpPixelAspect ) ;
		pBuffer->Renderer().SetParallax
			( pV3D->m_xParallax,
				pV3D->m_zParallaxFocus, pV3D->m_xParallaxScreen ) ;
	}
	else
	{
		SGLSize		sizeBuf = pImage->GetImageSize() ;
		S3DVector	vScreen( sizeBuf.w * 0.5, sizeBuf.h * 0.5, sizeBuf.w ) ;
		pBuffer->Renderer().SetProjectionScreen( vScreen, 1.0 ) ;
		pBuffer->Renderer().SetParallax( 0.0, 1.0, 0.0 ) ;
	}
	if ( pBuffer->IsFillBack() )
	{
		pBuffer->Renderer().FillClearTarget( pBuffer->GetFillBackColor() ) ;
	}
	//
	// 子スプライト描画
	//
	DrawChildren( pBuffer->Renderer(), s3dView ) ;
	//
	pImage = pBuffer->DetachRenderTarget( s3dView, pViewRect ) ;
	//
	if ( (s3dView == s3dLeftView) || !pBuffer->IsStereo3D() )
	{
		m_statusUpdate = updateEmpty ;
	}
	//
	// フィルタ処理
	//
	S3DRenderContextInterface &	render = pBuffer->Renderer() ;
	SGLImageObject *			pLastFilter = pImage ;
	SGLImageInfo				infFilter ;
	pImage->GetImageInfo( infFilter ) ;
	//
	const size_t	countFilter = m_filters.GetLength() ;
	for ( size_t i = 0; i < countFilter; i ++ )
	{
		SGLSpriteFilter *	pFilter = m_filters.GetAt( i ) ;
		if ( pFilter == NULL )
		{
			continue ;
		}
		pFilter->SetFilterParameter
			( m_paramView.paramFilter, m_paramView.paramFilter2 ) ;
		pFilter->Filter( render, pLastFilter ) ;
		//
		if ( pFilter->IsDynamicDrawer()
			&& ((m_pDrawer != NULL) || (i + 1 < countFilter)) )
		{
			SGLImageObject *	pDstFilter = pImage ;
			if ( pLastFilter == pImage )
			{
				pDstFilter = pFilter->GetInternalBuffer( infFilter ) ;
			}
			SGLPaintParam	pp ;
			render.AttachTargetImage( pDstFilter, NULL, NULL ) ;
			render.FillClearTarget( 0 ) ;
			pFilter->Draw( render, pp, pLastFilter ) ;
			render.DetachTargetImage() ;
			pLastFilter = pDstFilter ;
		}
	}
	//
	pLastFilter = CustomFilter( render, pImage, pLastFilter ) ;
	//
	if ( pLastFilter != pImage )
	{
		if ( !(pBuffer->GetBufFlags() & SGLImageObject::bufferOnDeviceOnly) )
		{
			pImage->CopyImage( pLastFilter ) ;
		}
		else
		{
			SGLPaintParam	pp ;
			render.AttachTargetImage( pImage, NULL, NULL ) ;
			render.FillClearTarget( 0 ) ;
			render.DrawImage( pp, pLastFilter ) ;
			render.DetachTargetImage() ;
		}
	}
	AtomicSub( &m_countFrozen, 1 ) ;
}

// カスタムフィルタ
//////////////////////////////////////////////////////////////////////////////
SGLImageObject *
	SGLSprite::CustomFilter
		( S3DRenderContextInterface& render,
			SGLImageObject * pFrameBuf, SGLImageObject * pSrcImage )
{
	return	pSrcImage ;
}

// 更新領域通知
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::PostUpdate( SGLRect* pUpdate )
{
	if ( (m_statusUpdate != updateFull) || (GetFrameBuffer() == NULL) )
	{
		LockTrace( __FILE__, __LINE__ ) ;
		SGLSprite *	pParent = m_refParent.GetReference() ;
		SGLRect		rectExt ;
		Buffer *	pBuffer = GetFrameBuffer() ;
		if ( (pUpdate == NULL)
			|| (m_filters.GetLength() != 0) /*|| (pBuffer == NULL)*/ )
		{
			m_statusUpdate = updateFull ;
			if ( pParent != NULL )
			{
				if ( GetRectangle( rectExt ) )
				{
					pParent->PostUpdate( &rectExt ) ;
				}
				else
				{
					pParent->PostUpdate() ;
				}
			}
		}
		else
		{
			if ( m_statusUpdate == updateRect )
			{
				m_rectUpdate |= *pUpdate ;
			}
			else
			{
				m_statusUpdate = updateRect ;
				m_rectUpdate = *pUpdate ;
			}
			if ( pBuffer != NULL )
			{
				SGLImageInfo	imginf ;
				if ( pBuffer->GetImage()->
						GetImageInfo( imginf ) == sglErrSuccess )
				{
					if ( (m_rectUpdate.left <= 0)
						&& (m_rectUpdate.top <= 0)
						&& (m_rectUpdate.right + 1 >= (int32_t) imginf.width)
						&& (m_rectUpdate.bottom + 1 >= (int32_t) imginf.height) )
					{
						m_statusUpdate = updateFull ;
					}
				}
			}
			if ( pParent != NULL )
			{
				rectExt = m_rectUpdate ;
				LocalToGlobalRect( rectExt ) ;
				pParent->PostUpdate( &rectExt ) ;
			}
		}
		Unlock() ;
	}
}

void SGLSprite::NotifyUpdate( void )
{
	if ( m_visible )
	{
		LockTrace( __FILE__, __LINE__ ) ;
		SGLSprite *	pParent = m_refParent.GetReference() ;
		if ( pParent != NULL )
		{
			SGLRect		rectExt ;
			if ( GetRectangle( rectExt ) )
			{
				pParent->PostUpdate( &rectExt ) ;
			}
			else
			{
				pParent->PostUpdate() ;
			}
		}
		Unlock() ;
	}
}

// 更新領域状態取得
//////////////////////////////////////////////////////////////////////////////
bool SGLSprite::HasUpdate( void ) const
{
	return	(m_statusUpdate != updateEmpty) ;
}

// フレームバッファ更新一時停止
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::FreezeFrameUpdate( void )
{
	AtomicAdd( &m_countFrozen, 1 ) ;
}

bool SGLSprite::DefrostFrameUpdate( void )
{
	atomic_int_t	nFrozen = AtomicSub( &m_countFrozen, 1 ) ;
	ESLAssert( nFrozen >= 0 ) ;
	return	(nFrozen <= 0) ;
}

// スプライト画像の描画処理 (外部 SGLSpriteDrawer がない場合)
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::DrawSprite
	( S3DRenderContextInterface& render,
		const SGLPaintParam& pp, SGLImageObject* image ) const
{
	if ( image != NULL )
	{
		render.DrawImage( pp, image ) ;
	}
}

// 表示状態の子スプライトに対し BeforeDraw を呼び出し
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::BeforeDrawChildren( SGLSprite::Stereo3DView s3dView )
{
	const size_t	countChildren = m_children.GetLength() ;
	if ( countChildren == 0 )
	{
		return ;
	}
	SReferenceArray<SGLSprite>::Iterator	iter = m_children.End() ;
	while ( iter.HasPrev() )
	{
		SGLSprite *	pChild = iter.Prev() ;
		if ( pChild == nullptr )
		{
			m_children.RemoveAt( iter.Index() ) ;
			continue ;
		}
		else if ( !(pChild->m_visible) )
		{
			continue ;
		}
		pChild->BeforeDraw( s3dView ) ;
	}
}

// 子スプライトを描画
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::DrawChildren
	( S3DRenderContextInterface& render, SGLSprite::Stereo3DView s3dView ) const
{
	const size_t	countChildren = m_children.GetLength() ;
	if ( countChildren == 0 )
	{
		return ;
	}
	Virtual3DParam *	pV3D = m_pVirtual3D ;
	SReferenceArray<SGLSprite>::Iterator	iter = m_children.End() ;
	while ( iter.HasPrev() )
	{
		SGLSprite *	pChild = iter.Prev() ;
		if ( (pChild == nullptr) || !(pChild->m_visible) )
		{
			continue ;
		}
		if ( pChild->m_paramView.nSpriteFlags & flagZScale )
		{
			SReferenceArray<SGLSprite>	sort3dz ;
			const int32_t	nPriority = pChild->m_priority ;
			sort3dz.SetLength( 0 ) ;
			sort3dz.Add( pChild ) ;
			//
			while ( iter.HasPrev() )
			{
				pChild = iter.Prev() ;
				if ( pChild == nullptr )
				{
					continue ;
				}
				if ( (pChild->m_priority != nPriority)
					|| !(pChild->m_paramView.nSpriteFlags & flagZScale) )
				{
					iter.Next() ;
					break ;
				}
				sort3dz.Add( pChild ) ;
			}
			//
			const size_t	countSort = sort3dz.GetLength() ;
			for ( size_t j = 0; j < countSort; j ++ )
			{
				pChild = sort3dz.GetAt( j ) ;
				if ( pChild == nullptr )
				{
					continue ;
				}
				double	zMax = pChild->m_paramView.vDst.z ;
				size_t	iMax = j ;
				for ( size_t k = j + 1; k < countSort; k ++ )
				{
					pChild = sort3dz.GetAt( k ) ;
					if ( (pChild != nullptr)
						&& (pChild->m_paramView.vDst.z > zMax) )
					{
						zMax = pChild->m_paramView.vDst.z ;
						iMax = k ;
					}
				}
				sort3dz.Swap( j, iMax ) ;
				pChild = sort3dz.GetAt( j ) ;
				//
				SReference	refChild = pChild ;
//				pChild->BeforeDraw( s3dView ) ;
				pChild->Draw( render, pV3D, s3dView ) ;
				if ( refChild != nullptr )
				{
					pChild->AfterDraw( s3dView ) ;
				}
			}
			sort3dz.FinishArray() ;
		}
		else
		{
//			pChild->BeforeDraw( s3dView ) ;
			pChild->Draw( render, pV3D, s3dView ) ;
			if ( !iter.IsElementExpired() )
			{
				pChild->AfterDraw( s3dView ) ;
			}
		}
	}
}

void SGLSprite::DrawChildrenImageList
	( SGLDrawImageParamList& dipl, SGLSprite::Stereo3DView s3dView ) const
{
	const size_t	countChildren = m_children.GetLength() ;
	if ( countChildren == 0 )
	{
		return ;
	}
	Virtual3DParam *	pV3D = m_pVirtual3D ;
	SReferenceArray<SGLSprite>::Iterator	iter = m_children.End() ;
	while ( iter.HasPrev() )
	{
		SGLSprite *	pChild = iter.Prev() ;
		if ( (pChild == nullptr) || !(pChild->m_visible) )
		{
			continue ;
		}
		if ( pChild->m_paramView.nSpriteFlags & flagZScale )
		{
			SReferenceArray<SGLSprite>	sort3dz ;
			const int32_t	nPriority = pChild->m_priority ;
			sort3dz.SetLength( 0 ) ;
			sort3dz.Add( pChild ) ;
			//
			while ( iter.HasPrev() )
			{
				pChild = iter.Prev() ;
				if ( pChild == nullptr )
				{
					continue ;
				}
				if ( (pChild->m_priority != nPriority)
					|| !(pChild->m_paramView.nSpriteFlags & flagZScale) )
				{
					iter.Next() ;
					break ;
				}
				sort3dz.Add( pChild ) ;
			}
			//
			const size_t	countSort = sort3dz.GetLength() ;
			for ( size_t j = 0; j < countSort; j ++ )
			{
				pChild = sort3dz.GetAt( j ) ;
				if ( pChild == nullptr )
				{
					continue ;
				}
				double	zMax = pChild->m_paramView.vDst.z ;
				size_t	iMax = j ;
				for ( size_t k = j + 1; k < countSort; k ++ )
				{
					pChild = sort3dz.GetAt( k ) ;
					if ( (pChild != nullptr)
						&& (pChild->m_paramView.vDst.z > zMax) )
					{
						zMax = pChild->m_paramView.vDst.z ;
						iMax = k ;
					}
				}
				sort3dz.Swap( j, iMax ) ;
				pChild = sort3dz.GetAt( j ) ;
				//
				SReference	refChild = pChild ;
//				pChild->BeforeDraw( s3dView ) ;
				pChild->DrawImageList( dipl, pV3D, s3dView ) ;
				if ( refChild != nullptr )
				{
					pChild->AfterDraw( s3dView ) ;
				}
			}
			sort3dz.FinishArray() ;
		}
		else
		{
//			pChild->BeforeDraw( s3dView ) ;
			pChild->DrawImageList( dipl, pV3D, s3dView ) ;
			if ( !iter.IsElementExpired() )
			{
				pChild->AfterDraw( s3dView ) ;
			}
		}
	}
}

// 描画パラメータ取得
//////////////////////////////////////////////////////////////////////////////
bool SGLSprite::GetPaintParam
	( SGLPaintParam& pp, SGLAffine& affine,
		const SGLSprite::Virtual3DParam* pV3D,
		SGLSprite::Stereo3DView s3dView ) const
{
	double		xParallax = 0 ;
	S3DDVector	vDst = m_paramView.vDst ;
	double		zScale = 1.0 ;
	if ( pV3D != NULL )
	{
		if ( (m_paramView.nSpriteFlags & flagZScale)
					&& (pV3D->m_vProjectScreen.z != 0.0) )
		{
			S3DDMatrix	matCamera( 1, 1, 1 ) ;
			matCamera.RevolveByAngleOn
				( pV3D->m_vCameraTarget - pV3D->m_vCameraView ) ;
			//
			vDst -= pV3D->m_vCameraView ;
			matCamera.RevolveVector( vDst ) ;
			//
			if ( s3dView == s3dRightView )
			{
				xParallax = pV3D->m_xParallax ;
			}
			else if ( s3dView == s3dLeftView )
			{
				xParallax = - pV3D->m_xParallax ;
			}
			if ( vDst.z < 1.0 )
			{
				return	false ;
			}
			xParallax = (vDst.z - pV3D->m_vProjectScreen.z)
							* xParallax * pV3D->m_zProjectScale / vDst.z ;
			zScale = pV3D->m_vProjectScreen.z
						* pV3D->m_zProjectScale / vDst.z ;
			vDst.x = (vDst.x - pV3D->m_vProjectScreen.x)
							* zScale + pV3D->m_vProjectScreen.x + xParallax ;
			vDst.y = (vDst.y - pV3D->m_vProjectScreen.y)
							* zScale + pV3D->m_vProjectScreen.y ;
		}
		else
		{
			if ( vDst.z >= 1.0 )
			{
				xParallax = (vDst.z - pV3D->m_vProjectScreen.z)
								* pV3D->m_xParallax
								* pV3D->m_zProjectScale / vDst.z ;
				if ( s3dView == s3dRightView )
				{
					vDst.x += xParallax ;
				}
				else if ( s3dView == s3dLeftView )
				{
					vDst.x -= xParallax ;
				}
			}
		}
	}
	pp.SetAffine
		( affine, vDst.x, vDst.y,
			m_paramView.vCenter.x + m_vImageCenter.x,
			m_paramView.vCenter.y + m_vImageCenter.y,
			m_paramView.vZoom.x * zScale,
			m_paramView.vZoom.y * zScale,
			m_paramView.zAngle, m_paramView.xyCross ) ;
	pp.nFlags = m_paramView.nFlags | paintDelayable ;
	pp.nTransparency = m_paramView.nTransparency ;
	pp.zOrder = (float32_t) vDst.z ;
	pp.rgbColorParam = m_paramView.rgbColorParam ;
	pp.countVertex = (uint32_t) m_paramView.countVertex ;
	pp.pVertices = m_paramView.pVertices ;
	return	true ;
}

// 外接矩形取得
//////////////////////////////////////////////////////////////////////////////
bool SGLSprite::GetRectangle( SGLRect& rectExt ) const
{
	SGLImageObject *	pImage = m_refImage.GetReference() ;
	SGLSpriteDrawer *	pDrawer = m_pDrawer ;
	SGLImageRect	rectImage ;
	if ( pDrawer != NULL )
	{
		if ( !pDrawer->GetRectangle( rectImage, pImage ) )
		{
			return	false ;
		}
	}
	else
	{
		if ( pImage == NULL )
		{
			if ( GetAllChildrenRectangle( rectExt ) )
			{
				return	LocalToGlobalRect( rectExt ) ;
			}
			return	false ;
		}
		SGLImageInfo	imginf ;
		if ( pImage->GetImageInfo( imginf ) != sglErrSuccess )
		{
			return	false ;
		}
		rectImage.x = 0 ;
		rectImage.y = 0 ;
		rectImage.w = imginf.width ;
		rectImage.h = imginf.height ;
	}
	rectExt = rectImage ;
	return	LocalToGlobalRect( rectExt ) ;
}

// 小スプライトの外接矩形の集合を取得
//////////////////////////////////////////////////////////////////////////////
bool SGLSprite::GetAllChildrenRectangle( SGLRect& rectExt ) const
{
	SReferenceArray<SGLSprite>::Iterator	iter = m_children.End() ;
	bool	flagRect = false ;
	while ( iter.HasPrev() )
	{
		SGLSprite *	pChild = iter.Prev() ;
		if ( (pChild == nullptr) || !(pChild->m_visible) )
		{
			continue ;
		}
		SGLRect	rectChild ;
		if ( pChild->GetRectangle( rectChild ) )
		{
			if ( flagRect )
			{
				rectExt |= rectChild ;
			}
			else
			{
				rectExt = rectChild ;
				flagRect = true ;
			}
		}
	}
	return	flagRect ;
}

// ローカル座標からグローバル座標へ変換
//////////////////////////////////////////////////////////////////////////////
bool SGLSprite::LocalToGlobal( S2DDVector& vPos ) const
{
	SGLPaintParam	pp ;
	SGLAffine		affine ;
	Virtual3DParam*	pV3D = NULL ;
	if ( m_paramView.nSpriteFlags & flagZScale )
	{
		SGLSprite *	pParent = m_refParent.GetReference() ;
		if ( pParent != NULL )
		{
			pV3D = pParent->m_pVirtual3D ;
		}
	}
	if ( !GetPaintParam( pp, affine, pV3D ) )
	{
		return	false ;
	}
	double	x = vPos.x ;
	double	y = vPos.y ;
	vPos.x = affine.a11 * x + affine.a12 * y + affine.a13 ;
	vPos.y = affine.a21 * x + affine.a22 * y + affine.a23 ;
	return	true ;
}

bool SGLSprite::LocalToGlobalRect( SGLRect& rect ) const
{
	SGLPaintParam	pp ;
	SGLAffine		affine ;
	Virtual3DParam*	pV3D = NULL ;
	if ( m_paramView.nSpriteFlags & flagZScale )
	{
		SGLSprite *	pParent = m_refParent.GetReference() ;
		if ( pParent != NULL )
		{
			pV3D = pParent->m_pVirtual3D ;
		}
	}
	if ( !GetPaintParam( pp, affine, pV3D ) )
	{
		return	false ;
	}
	if ( !affine.IsRotation() )
	{
		rect.left += (int32_t) floor( affine.a13 ) ;
		rect.top += (int32_t) floor( affine.a23 ) ;
		rect.right += (int32_t) affine.a13 ;
		rect.bottom += (int32_t) affine.a23 ;
	}
	else
	{
		S2DVector	vRect[4] ;
		vRect[0].x = (float32_t) rect.left ;
		vRect[0].y = (float32_t) rect.top ;
		vRect[1].x = (float32_t) (rect.right + 1) ;
		vRect[1].y = vRect[0].y ;
		vRect[2].x = vRect[1].x ;
		vRect[2].y = (float32_t) (rect.bottom + 1) ;
		vRect[3].x = vRect[0].x ;
		vRect[3].y = vRect[2].y ;
		affine.TransformVectors( &vRect[0], &vRect[0], 4 ) ;
		//
		float32_t	xMin = vRect[0].x ;
		float32_t	xMax = vRect[0].x ;
		float32_t	yMin = vRect[0].y ;
		float32_t	yMax = vRect[0].y ;
		for ( size_t i = 1; i < 4; i ++ )
		{
			float32_t	x = vRect[i].x ;
			float32_t	y = vRect[i].y ;
			if ( x < xMin )
			{
				xMin = x ;
			}
			if ( x > xMax )
			{
				xMax = x ;
			}
			if ( y < yMin )
			{
				yMin = y ;
			}
			if ( y > yMax )
			{
				yMax = y ;
			}
		}
		rect.left = (int32_t) floor( xMin ) ;
		rect.top = (int32_t) floor( yMin ) ;
		rect.right = (int32_t) (xMax + 1.0) ;
		rect.bottom = (int32_t) (yMax + 1.0) ;
	}
	return	true ;
}

// グローバル座標からローカル座標へ変換
//////////////////////////////////////////////////////////////////////////////
bool SGLSprite::GlobalToLocal( S2DDVector& vPos ) const
{
	SGLPaintParam	pp ;
	SGLAffine		affine ;
	Virtual3DParam*	pV3D = NULL ;
	if ( m_paramView.nSpriteFlags & flagZScale )
	{
		SGLSprite *	pParent = m_refParent.GetReference() ;
		if ( pParent != NULL )
		{
			pV3D = pParent->m_pVirtual3D ;
		}
	}
	if ( !GetPaintParam( pp, affine, pV3D ) )
	{
		return	false ;
	}
	SGLAffine	iaff ;
	iaff.InverseOf( affine ) ;
	double	x = vPos.x ;
	double	y = vPos.y ;
	vPos.x = iaff.a11 * x + iaff.a12 * y + iaff.a13 ;
	vPos.y = iaff.a21 * x + iaff.a22 * y + iaff.a23 ;
	return	true ;
}

// デフォルト表示フラグ設定
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::SetDefaultParamFlags( uint32_t nDefFlags )
{
	m_nDefParamFlags = nDefFlags ;
}

// 表示パラメータ設定
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::SetParameter( const SGLSprite::Parameter& param )
{
	LockTrace( __FILE__, __LINE__ ) ;
	NotifyUpdate() ;
	m_paramView = param ;
	if ( (param.countVertex > 0) && (param.pVertices != NULL) )
	{
		m_paramView.pVertices = m_paramVertex.GetArray( param.countVertex ) ;
		eslMoveMemory
			( (void*) m_paramView.pVertices,
				param.pVertices, param.countVertex * sizeof(S2DVector) ) ;
		m_paramVertex.FinishArray() ;
	}
	NotifyUpdate() ;
	Unlock() ;
}

// 描画フラグ変更
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::ModifyDrawFlags( uint32_t nAddFlags, uint32_t nRemoveFlags )
{
	uint32_t	nNewFlags = (m_paramView.nFlags | nAddFlags) & ~nRemoveFlags ;
	if ( m_paramView.nFlags != nNewFlags )
	{
		LockTrace( __FILE__, __LINE__ ) ;
		NotifyUpdate() ;
		m_paramView.nFlags = nNewFlags ;
		NotifyUpdate() ;
		Unlock() ;
	}
}

// スプライト処理フラグ変更
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::ModifySpriteFlags( uint32_t nAddFlags, uint32_t nRemoveFlags )
{
	uint32_t	nNewFlags = (m_paramView.nSpriteFlags | nAddFlags) & ~nRemoveFlags ;
	if ( m_paramView.nSpriteFlags != nNewFlags )
	{
		LockTrace( __FILE__, __LINE__ ) ;
		NotifyUpdate() ;
		m_paramView.nSpriteFlags = nNewFlags ;
		NotifyUpdate() ;
		Unlock() ;
	}
}

// 座標設定
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::SetPosition( double x, double y )
{
	if ( (m_paramView.vDst.x != x)
		|| (m_paramView.vDst.y != y) )
	{
		LockTrace( __FILE__, __LINE__ ) ;
		NotifyUpdate() ;
		m_paramView.vDst.x = x ;
		m_paramView.vDst.y = y ;
		NotifyUpdate() ;
		Unlock() ;
	}
}

void SGLSprite::SetPosition3D( double x, double y, double z )
{
	if ( (m_paramView.vDst.x != x)
		|| (m_paramView.vDst.y != y)
		|| (m_paramView.vDst.z != z) )
	{
		LockTrace( __FILE__, __LINE__ ) ;
		NotifyUpdate() ;
		m_paramView.vDst.x = x ;
		m_paramView.vDst.y = y ;
		m_paramView.vDst.z = z ;
		NotifyUpdate() ;
		Unlock() ;
	}
}

void SGLSprite::SetPosition3D( const S3DDVector& vPos )
{
	if ( m_paramView.vDst != vPos )
	{
		LockTrace( __FILE__, __LINE__ ) ;
		NotifyUpdate() ;
		m_paramView.vDst = vPos ;
		NotifyUpdate() ;
		Unlock() ;
	}
}

// 中心座標設定
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::SetCenterPosition( double x, double y )
{
	if ( (m_paramView.vCenter.x != x)
		|| (m_paramView.vCenter.y != y) )
	{
		LockTrace( __FILE__, __LINE__ ) ;
		NotifyUpdate() ;
		m_paramView.vCenter.x = x ;
		m_paramView.vCenter.y = y ;
		NotifyUpdate() ;
		Unlock() ;
	}
}

// 拡大率設定
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::SetZoom( double x, double y )
{
	if ( (m_paramView.vZoom.x != x)
		|| (m_paramView.vZoom.y != y) )
	{
		LockTrace( __FILE__, __LINE__ ) ;
		NotifyUpdate() ;
		m_paramView.vZoom.x = x ;
		m_paramView.vZoom.y = y ;
		NotifyUpdate() ;
		Unlock() ;
	}
}

// 回転角設定
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::SetRotation( double zAngle )
{
	if ( m_paramView.zAngle != zAngle )
	{
		LockTrace( __FILE__, __LINE__ ) ;
		NotifyUpdate() ;
		m_paramView.zAngle = zAngle ;
		NotifyUpdate() ;
		Unlock() ;
	}
}

// 透明度設定
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::SetTransparency( uint32_t nTransparency )
{
	if ( m_paramView.nTransparency != nTransparency )
	{
		LockTrace( __FILE__, __LINE__ ) ;
		m_paramView.nTransparency = nTransparency ;
		NotifyUpdate() ;
		Unlock() ;
	}
}

// フィルタパラメータ設定
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::SetFilterParameter( int32_t paramFilter )
{
	if ( m_paramView.paramFilter != paramFilter )
	{
		LockTrace( __FILE__, __LINE__ ) ;
		m_paramView.paramFilter = paramFilter ;
		PostUpdate() ;
		Unlock() ;
	}
}

void SGLSprite::SetFilter2Parameter( int32_t paramFilter )
{
	if ( m_paramView.paramFilter2 != paramFilter )
	{
		LockTrace( __FILE__, __LINE__ ) ;
		m_paramView.paramFilter2 = paramFilter ;
		PostUpdate() ;
		Unlock() ;
	}
}

// 表示中心座標を調整する
//////////////////////////////////////////////////////////////////////////////
bool SGLSprite::RegulateCenter
	( uint32_t nFlags, double xOffset, double yOffset )
{
	SGLPaintParam	pp ;
	SGLAffine		affine ;
	Virtual3DParam*	pV3D = NULL ;
	if ( m_paramView.nSpriteFlags & flagZScale )
	{
		SGLSprite *	pParent = m_refParent.GetReference() ;
		if ( pParent != NULL )
		{
			pV3D = pParent->m_pVirtual3D ;
		}
	}
	if ( !GetPaintParam( pp, affine, pV3D ) )
	{
		return	false ;
	}
	SGLSize	sizeImage = GetImageSize() ;
	if ( nFlags & regRight )
	{
		xOffset += sizeImage.w - 1 ;
	}
	else if ( !(nFlags & regLeft) )
	{
		xOffset += sizeImage.w * 0.5 ;
	}
	if ( nFlags & regBottom )
	{
		yOffset += sizeImage.h - 1 ;
	}
	else if ( !(nFlags & regTop) )
	{
		yOffset += sizeImage.h * 0.5 ;
	}
	xOffset -= m_vImageCenter.x ;
	yOffset -= m_vImageCenter.y ;
	//
	LockTrace( __FILE__, __LINE__ ) ;
	double	xDelta = xOffset - m_paramView.vCenter.x ;
	double	yDelta = yOffset - m_paramView.vCenter.y ;
	m_paramView.vDst.x += affine.a11 * xDelta + affine.a12 * yDelta ;
	m_paramView.vDst.y += affine.a21 * xDelta + affine.a22 * yDelta ;
	m_paramView.vCenter.x = xOffset ;
	m_paramView.vCenter.y = yOffset ;
	Unlock() ;
	return	true ;
}

// UI フラグ変更
//////////////////////////////////////////////////////////////////////////////
uint64_t SGLSprite::ModifyUIFlag
	( uint64_t nAddFlags, uint64_t nRemoveFlags )
{
	m_flagsUI = (m_flagsUI | nAddFlags) & ~nRemoveFlags ;
	return	m_flagsUI ;
}

// ヒット領域設定
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::SetClickableRect( SGLImageRect& rect )
{
	Lock() ;
	m_rectClickable = rect ;
	Unlock() ;
}

// 表示フラグ設定
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::SetVisible( bool visible )
{
	if ( m_visible != visible )
	{
		LockTrace( __FILE__, __LINE__ ) ;
		NotifyUpdate() ;
		m_visible = visible ;
		NotifyUpdate() ;
		Unlock() ;
	}
}

// 表示優先度変更
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::ChangePriority( int32_t nPriority )
{
	SGLSprite *	pParent = m_refParent.GetReference() ;
	if ( pParent != NULL )
	{
		if ( m_priority != nPriority )
		{
			LockTrace( __FILE__, __LINE__ ) ;
			bool	fSmart = pParent->IsSmartChild( this ) ;
			pParent->DetachChild( this ) ;
			m_priority = nPriority ;
			if ( fSmart )
			{
				pParent->AddSmartChild( this ) ;
			}
			else
			{
				pParent->AddChild( this ) ;
			}
			Unlock() ;
		}
	}
	else
	{
		m_priority = nPriority ;
	}
}

// 識別子設定
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::SetID( const wchar_t * pwszID )
{
	m_strID = pwszID ;
}

// 優先度位置検索
//////////////////////////////////////////////////////////////////////////////
size_t SGLSprite::OrderIndexAs( int32_t nPriority ) const
{
	size_t	iFirst = 0 ;
	size_t	iEnd = m_children.GetLength() ;
	while ( iFirst <= iEnd )
	{
		size_t		iMiddle = (iFirst + iEnd) >> 1 ;
		SGLSprite *	pSprite = m_children.GetAt( iMiddle ) ;
		if ( pSprite != NULL )
		{
			if ( pSprite->m_priority > nPriority )
			{
				iEnd = iMiddle - 1 ;
			}
			else if ( pSprite->m_priority < nPriority )
			{
				iFirst = iMiddle + 1 ;
			}
			else
			{
				return	iMiddle ;
			}
		}
		else
		{
			for ( iMiddle = iFirst; iMiddle <= iEnd; iMiddle ++ )
			{
				pSprite = m_children.GetAt( iMiddle ) ;
				if ( (pSprite != NULL)
					&& (pSprite->m_priority >= nPriority) )
				{
					return	iMiddle ;
				}
			}
			return	iEnd + 1 ;
		}
	}
	return	iFirst ;
}

// 子スプライト追加
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::AddChild( SGLSprite* pSprite )
{
	if ( pSprite == NULL )
	{
		return ;
	}
	SGLSprite *	pParent = pSprite->m_refParent.GetReference() ;
	if ( pParent == this )
	{
		return ;
	}
	LockTrace( __FILE__, __LINE__ ) ;
	if ( pParent != NULL )
	{
		pParent->DetachChild( pSprite ) ;
	}
	m_children.InsertAt( OrderIndexAs( pSprite->m_priority ), pSprite ) ;
	pSprite->m_refParent.SetReference( this ) ;
	PostUpdate() ;
	Unlock() ;
}

void SGLSprite::AddSmartChild( SGLSprite* pSprite )
{
	if ( pSprite == NULL )
	{
		return ;
	}
	SGLSprite *	pParent = pSprite->m_refParent.GetReference() ;
	if ( pParent == this )
	{
		return ;
	}
	if ( pParent != NULL )
	{
		pSprite = SGLSmartCast<SGLSprite>( pSprite->DuplicateObject() ) ;
		if ( pSprite == NULL )
		{
			return ;
		}
	}
	LockTrace( __FILE__, __LINE__ ) ;
	m_children.SmartInsertAt( OrderIndexAs( pSprite->m_priority ), pSprite ) ;
	pSprite->m_refParent.SetReference( this ) ;
	PostUpdate() ;
	Unlock() ;
}

// 子スプライトは自動的に削除されるか？
//////////////////////////////////////////////////////////////////////////////
bool SGLSprite::IsSmartChild( SGLSprite* pSprite ) const
{
	bool	fSmartChild ;
	LockTrace( __FILE__, __LINE__ ) ;
	fSmartChild = m_children.IsSmartElementOf( pSprite ) ;
	Unlock() ;
	return	fSmartChild ;
}

// 子スプライト削除
//////////////////////////////////////////////////////////////////////////////
SGLSprite * SGLSprite::DetachChild( SGLSprite* pSprite )
{
	LockTrace( __FILE__, __LINE__ ) ;
	ssize_t	iChild = m_children.FindPtr( pSprite ) ;
	if ( iChild >= 0 )
	{
		AsyncReleaseChildReference( pSprite ) ;
		//
		SGLRect	rectExt ;
		if ( pSprite->GetRectangle( rectExt ) )
		{
			PostUpdate( &rectExt ) ;
		}
		m_children.DetachAt( iChild ) ;
	}
	else
	{
		pSprite = NULL ;
	}
	Unlock() ;
	return	pSprite ;
}

bool SGLSprite::RemoveChild( SGLSprite* pSprite )
{
	LockTrace( __FILE__, __LINE__ ) ;
	ssize_t	iChild = m_children.FindPtr( pSprite ) ;
	if ( iChild >= 0 )
	{
		AsyncReleaseChildReference( pSprite ) ;
		//
		SGLRect	rectExt ;
		if ( pSprite->GetRectangle( rectExt ) )
		{
			PostUpdate( &rectExt ) ;
		}
		m_children.RemoveAt( iChild ) ;
	}
	Unlock() ;
	return	(iChild >= 0) ;
}

// 親スプライトから安全に分離
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSprite::DetachSyncTimeout( int64_t msecTimeout )
{
	SGLSprite *	pParent = m_refParent.GetReference() ;
	if ( pParent == nullptr )
	{
		return	sglErrSuccess ;
	}
	if ( LockTrace( __FILE__, __LINE__, msecTimeout ) == errSuccess )
	{
		pParent = m_refParent.GetReference() ;
		if ( pParent != nullptr )
		{
			pParent->DetachChild( this ) ;
		}
		Unlock() ;
		return	sglErrSuccess ;
	}
	else
	{
		ESLTrace( "timeout lock at SGLSprite::DetachSyncTimeout (%s)\n", GetESLClassName() );
		return	sglErrTimeout ;
	}
}

// 全子スプライト削除
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::DetachAllChildren( void )
{
	LockTrace( __FILE__, __LINE__ ) ;
	AsyncReleaseAllChildrenReference() ;
	m_children.DetachAll() ;
	PostUpdate() ;
	Unlock() ;
}

void SGLSprite::RemoveAllChildren( void )
{
	LockTrace( __FILE__, __LINE__ ) ;
	AsyncRemoveAllChildren() ;
	PostUpdate() ;
	Unlock() ;
}

void SGLSprite::AsyncReleaseChildReference( SGLSprite* pSprite )
{
	pSprite->m_refParent.ReleaseReference() ;
	//
	ssize_t	iFocus = m_rfarMouseFocus.FindPtr( pSprite ) ;
	if ( iFocus >= 0 )
	{
		m_rfarMouseFocus.SetAt( iFocus, NULL ) ;
		pSprite->OnMouseLeave( 0 ) ;
	}
	if ( m_refKeyFocus.GetReference() == pSprite )
	{
		m_refKeyFocus.ReleaseReference() ;
		pSprite->OnKillKeyFocus() ;
	}
	if ( m_refCaptured.GetReference() == pSprite )
	{
		m_refCaptured.ReleaseReference() ;
		pSprite->OnReleaseMouseCapture() ;
	}
}

void SGLSprite::AsyncReleaseAllChildrenReference( void )
{
	const size_t	nCount = m_children.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		SGLSprite *	pChild = m_children.GetAt( i ) ;
		if ( pChild != NULL )
		{
			pChild->m_refParent.ReleaseReference() ;
		}
	}
	m_rfarMouseFocus.RemoveAll() ;
	m_refKeyFocus.ReleaseReference() ;
	m_refCaptured.ReleaseReference() ;
}

void SGLSprite::AsyncRemoveAllChildren( void )
{
	AsyncReleaseAllChildrenReference() ;
	m_children.RemoveAll() ;
}

// 親スプライト取得
//////////////////////////////////////////////////////////////////////////////
SGLSprite* SGLSprite::GetParent( void ) const
{
	return	m_refParent.GetReference() ;
}

// 指定アイテム取得 (孫アイテム以下は \ で区切って指定)
//////////////////////////////////////////////////////////////////////////////
SGLSprite* SGLSprite::GetItemAs( const wchar_t * pwszID ) const
{
	if ( (pwszID == NULL) || (*pwszID == 0) )
	{
		return	(SGLSprite*) this ;
	}
	const wchar_t *	pwszCurID = pwszID ;
	SString	strCurID ;
	ssize_t	iSep = -1 ;
	size_t	i ;
	for ( i = 0; pwszID[i]; i ++ )
	{
		if ( pwszID[i] == '\\' )
		{
			strCurID.SetString( pwszID, (ssize_t) i ) ;
			pwszCurID = strCurID ;
			iSep = (ssize_t) i ;
			break ;
		}
	}
	const size_t	nCount = m_children.GetLength() ;
	for ( i = 0; i < nCount; i ++ )
	{
		SGLSprite *	pChild = m_children.GetAt( i ) ;
		if ( (pChild != NULL) && (pChild->m_strID == pwszCurID) )
		{
			if ( iSep >= 0 )
			{
				return	pChild->GetItemAs( pwszID + (iSep + 1) ) ;
			}
			return	pChild ;
		}
	}
	return	NULL ;
}

// ヒットアイテム検索
//////////////////////////////////////////////////////////////////////////////
SGLSprite* SGLSprite::GetHitSpriteAt( S2DDVector& vPos ) const
{
	const size_t	nCount = m_children.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		SGLSprite *	pChild = m_children.GetAt( i ) ;
		if ( (pChild != NULL)
			&& pChild->IsVisible()
			&& (pChild->GetTransparency() < 0x100)
			&& !(pChild->m_flagsUI & uiUnclickable) )
		{
			S2DDVector	vLocal = vPos ;
			pChild->GlobalToLocal( vLocal ) ;
			if ( pChild->IsHitSprite( vLocal.x, vLocal.y ) )
			{
				vPos = vLocal ;
				return	pChild ;
			}
			if ( pChild->m_flagsUI & uiModalEnd )
			{
				break ;
			}
		}
	}
	return	NULL ;
}

// ヒット判定
//////////////////////////////////////////////////////////////////////////////
bool SGLSprite::IsHitSprite( double x, double y ) const
{
	if ( m_flagsUI & uiUnclickable )
	{
		return	false ;
	}
	if ( !m_visible || (m_paramView.nTransparency >= 0x100) )
	{
		return	false ;
	}
	if ( (m_rectClickable.x <= x)
		&& (x - m_rectClickable.x < m_rectClickable.w)
		&& (m_rectClickable.y <= y)
		&& (y - m_rectClickable.y < m_rectClickable.h) )
	{
		return	true ;
	}
	SGLImageObject *	pImage = m_refImage.GetReference() ;
	if ( m_pDrawer != NULL )
	{
		if ( m_pDrawer->IsHitPointAt( pImage, x, y ) )
		{
			return	true ;
		}
	}
	if ( pImage != NULL )
	{
		if ( IsHitSpriteImage( pImage, x, y, false ) )
		{
			return	true ;
		}
		SGLSize	sizeImage = pImage->GetImageSize() ;
		if ( (x < 0) || (y < 0)
			|| (x > sizeImage.w) || (y > sizeImage.h) )
		{
			return	false ;
		}
	}
	S2DDVector	vPos( x, y ) ;
	return	(GetHitSpriteAt( vPos ) != NULL) ;
}

// 画像ヒット判定
//////////////////////////////////////////////////////////////////////////////
bool SGLSprite::IsHitSpriteImage
	( SGLImageObject * pImage, double x, double y, bool fAlphaImage )
{
	int32_t	xPos = (int32_t) x ;
	int32_t	yPos = (int32_t) y ;
	if ( (xPos < 0) || (yPos < 0) )
	{
		return	false ;
	}
	if ( pImage == NULL )
	{
		return	false ;
	}
	SGLImageInfo	imginf ;
	if ( pImage->GetImageInfo( imginf ) != sglErrSuccess )
	{
		return	false ;
	}
	if ( (xPos >= (int32_t) imginf.width)
			|| (yPos >= (int32_t) imginf.height) )
	{
		return	false ;
	}
	bool	fAlpha = ((imginf.format & formatImageFlagAlpha) != 0)
												&& (imginf.depth == 32) ;
	bool	fClip = ((imginf.format & formatImageFlagClipping) != 0)
												&& (imginf.depth == 8) ;
	bool	fGray = fAlphaImage && (imginf.format == formatImageGray)
												&& (imginf.depth == 8) ;
	if ( !(fAlpha || fClip || fGray) )
	{
		return	true ;
	}
	SGLImageRect	rect( xPos, yPos, 1, 1 ) ;
	uint8_t *	pbytPixel =
		pImage->LockBuffer( imginf, SGLImageObject::lockRead, &rect ) ;
	if ( pbytPixel == NULL )
	{
		return	true ;
	}
	bool	fHit = false ;
	if ( fAlpha )
	{
		fHit = (pbytPixel[3] >= 0x08) ;
	}
	else if ( fClip )
	{
		fHit = (pbytPixel[0] != imginf.colorClip) ;
	}
	else
	{
		fHit = (pbytPixel[0] >= 0x80) ;
	}
	pImage->UnlockBuffer( SGLImageObject::lockRead ) ;
	return	fHit ;
}

// ドラッグ／スワイプ処理判定（子スプライトが処理するか？）
//////////////////////////////////////////////////////////////////////////////
bool SGLSprite::CanBeginDragOver( double x, double y ) const
{
	S2DDVector	vPos( x, y ) ;
	SGLSprite *	pHit = GetHitSpriteAt( vPos ) ;
	if ( (pHit != NULL) && (pHit != this) )
	{
		return	pHit->CanBeginDragOver( vPos.x, vPos.y ) ;
	}
	return	false ;
}

// 子スプライト数取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLSprite::GetChildCount( void ) const
{
	return	m_children.GetLength() ;
}

// 子スプライト取得
//////////////////////////////////////////////////////////////////////////////
SGLSprite * SGLSprite::GetChildAt( size_t i ) const
{
	return	m_children.GetAt( i ) ;
}

// 子スプライトを検索
//////////////////////////////////////////////////////////////////////////////
ssize_t SGLSprite::FindChildSprite( SGLSprite * pChild ) const
{
	return	m_children.FindPtr( pChild ) ;
}

// 指定された方向へ最も近い
// フォーカスを受け取り可能なスプライトを検索する
//////////////////////////////////////////////////////////////////////////////
SGLSprite * SGLSprite::SearchNearestFocusableSprite
	( SGLSprite * pOrigin,
		const S2DDVector& vOrigin, const S2DDVector& vDir ) const
{
	//
	// 有効な検索範囲を限定する
	//
	size_t	i, iFirst = 0, iEnd = m_children.GetLength() ;
	if ( pOrigin != NULL )
	{
		ssize_t	iOrigin = m_children.FindPtr( pOrigin ) ;
		if ( iOrigin >= 0 )
		{
			for ( i = 0; i <= (size_t) iOrigin; i ++ )
			{
				SGLSprite *	pChild = m_children.GetAt( i ) ;
				if ( (pChild != NULL)
					&& (pChild->m_flagsUI & uiModalFirst) )
				{
					iFirst = i ;
				}
			}
			for ( i = 0; i < iEnd; i ++ )
			{
				SGLSprite *	pChild = m_children.GetAt( i ) ;
				if ( (pChild != NULL)
					&& (pChild->m_flagsUI & uiModalEnd) )
				{
					iEnd = i + 1 ;
					break ;
				}
			}
		}
	}
	//
	// 最も近いスプライトを検索する
	//
	SGLSprite *	pNearest = NULL ;
	double		fpDistance = 1.0e+5 ;
	S2DDVector	vCross( - vDir.y, vDir.x ) ;
	for ( i = iFirst; i < iEnd; i ++ )
	{
		SGLSprite *	pChild = m_children.GetAt( i ) ;
		if ( (pChild != NULL)
			&& (pChild != pOrigin)
			&& (pChild->m_flagsUI & uiFocusable) )
		{
			S2DDVector	vChild ;
			if ( pChild->GetFocusPoint( vChild ) )
			{
				vChild -= vOrigin ;
				double	d = vChild.InnerProduct( vDir ) ;
				if ( d > 1.0 )
				{
					d += fabs( vChild.InnerProduct( vCross ) ) ;
					if ( d < fpDistance )
					{
						pNearest = pChild ;
						fpDistance = d ;
					}
				}
			}
		}
	}
	return	pNearest ;
}

// フォーカス座標（キー操作フォーカス移動用）
//////////////////////////////////////////////////////////////////////////////
bool SGLSprite::GetFocusPoint( S2DDVector& vPos )
{
	if ( m_flagsUI & uiFocusable )
	{
		SGLRect	rectExt ;
		if ( GetRectangle( rectExt ) )
		{
			vPos.x = (rectExt.left + rectExt.right) * 0.5 ;
			vPos.y = (rectExt.top + rectExt.bottom) * 0.5 ;
			return	true ;
		}
	}
	return	false ;
}

// 画像ファイル読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSprite::LoadImage( const wchar_t * pwszFilePath )
{
	SGLSpriteAnimator *	pAnimator = new SGLSpriteAnimator ;
	SGLError	err = pAnimator->LoadImage( pwszFilePath ) ;
	if ( err )
	{
		delete	pAnimator ;
		return	err ;
	}
	LockTrace( __FILE__, __LINE__ ) ;
	pAnimator->AttachImageToSprite( *this ) ;
	m_pImager = pAnimator ;
	Unlock() ;
	return	sglErrSuccess ;
}

// アニメーション関連付け
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::AttachAnimation
	( SGLImageObject * pImage, const SGLImageRect * pClip )
{
	SGLSpriteAnimator *	pAnimator = new SGLSpriteAnimator ;
	LockTrace( __FILE__, __LINE__ ) ;
	pAnimator->AttachImage( pImage, pClip ) ;
	pAnimator->AttachImageToSprite( *this ) ;
	m_pImager = pAnimator ;
	Unlock() ;
}

// アニメーション開始
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::BeginAnimation
	( ssize_t countLoop,
		size_t iLoopStart, size_t iLoopEnd,
		size_t iStartFrame, size_t msecDuration )
{
	SGLSpriteAnimator *	pAnimator =
		ESLTypeCast<SGLSpriteAnimator>( m_pImager.Ptr() ) ;
	if ( pAnimator != NULL )
	{
		pAnimator->BeginAnimation
			( countLoop, iLoopStart, iLoopEnd, iStartFrame, msecDuration ) ;
	}
}

// ループ設定変更
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::SetLoopAnimation
	( ssize_t countLoop, size_t iLoopStart, size_t iLoopEnd )
{
	SGLSpriteAnimator *	pAnimator =
		ESLTypeCast<SGLSpriteAnimator>( m_pImager.Ptr() ) ;
	if ( pAnimator != NULL )
	{
		pAnimator->SetLoop( countLoop, iLoopStart, iLoopEnd ) ;
	}
}

// 画像関連付け
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::AttachImage
	( SGLImageObject * pImage, SGLImageObject * pLeftImage )
{
	LockTrace( __FILE__, __LINE__ ) ;
	if ( m_refImage != NULL )
	{
		NotifyUpdate() ;
	}
	m_refImage = pImage ;
	m_refLeftImage = pLeftImage ;
	m_pBuffer = NULL ;
	//
	m_vImageCenter.x = 0 ;
	m_vImageCenter.y = 0 ;
	//
	if ( pImage != NULL )
	{
		SGLImageInfo	imginf ;
		if ( pImage->GetImageInfo( imginf ) == sglErrSuccess )
		{
			m_vImageCenter.x = imginf.ptOrigin.x ;
			m_vImageCenter.y = imginf.ptOrigin.y ;
		}
		NotifyUpdate() ;
	}
	Unlock() ;
}

// 関連付けられた画像取得
//////////////////////////////////////////////////////////////////////////////
SGLImageObject * SGLSprite::GetAttachedImage( void ) const
{
	return	m_refImage.GetReference() ;
}

SGLImageObject * SGLSprite::GetAttachedLeftImage( void ) const
{
	return	m_refLeftImage.GetReference() ;
}

// 関連付けられた画像サイズ取得
//////////////////////////////////////////////////////////////////////////////
SGLSize SGLSprite::GetImageSize( void ) const
{
	SGLImageObject *	pImage = m_refImage.GetReference() ;
	SGLImageInfo	imginf ;
	if ( (pImage == NULL)
		|| pImage->GetImageInfo( imginf ) )
	{
		return	SGLSize( 0, 0 ) ;
	}
	return	SGLSize( imginf.width, imginf.height ) ;
}

// 関連付けられた画像情報取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSprite::GetImageInfo( SGLImageInfo& imginf ) const
{
	SGLImageObject *	pImage = m_refImage.GetReference() ;
	if ( pImage == NULL )
	{
		return	sglErrFailed ;
	}
	return	pImage->GetImageInfo( imginf ) ;
}

// 描画オブジェクト設定
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::SetSpriteDrawer( SGLSpriteDrawer * pDrawer )
{
	LockTrace( __FILE__, __LINE__ ) ;
	m_pDrawer = pDrawer ;
	if ( pDrawer != nullptr )
	{
		pDrawer->OnAttachedSprite( this ) ;
	}
	Unlock() ;
}

// 画像制御オブジェクト設定
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::SetSpriteImager( SGLSprite::Imager * pImager )
{
	LockTrace( __FILE__, __LINE__ ) ;
	m_pImager = pImager ;
	if ( pImager != NULL )
	{
		pImager->OnAttached( *this ) ;
	}
	Unlock() ;
}

// バッファ生成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSprite::CreateBuffer
	( uint32_t width, uint32_t height,
		uint32_t format, uint32_t depth,
		uint64_t nBufFlags, bool flagZBuffer,
		bool flagStereo3D, SGLPaintContextType type )
{
	LockTrace( __FILE__, __LINE__ ) ;
	Buffer *	pBuffer = m_pBuffer ;
	if ( pBuffer == NULL )
	{
		pBuffer = new Buffer( type ) ;
		m_pBuffer = pBuffer ;
	}
	if ( pBuffer->CreateBuffer
		( width, height, format, depth, nBufFlags, flagZBuffer, flagStereo3D ) )
	{
		Unlock() ;
		return	sglErrFailed ;
	}
	m_refImage = pBuffer->GetImage() ;
	m_refLeftImage = NULL ;
	if ( pBuffer->IsStereo3D() )
	{
		m_refLeftImage = pBuffer->GetLeftImage() ;
	}
	Unlock() ;
	return	sglErrSuccess ;
}

// バッファ解放
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::ReleaseBuffer( void )
{
	LockTrace( __FILE__, __LINE__ ) ;
	m_pBuffer = NULL ;
	Unlock() ;
}

// バッファを保持しているか？
//////////////////////////////////////////////////////////////////////////////
bool SGLSprite::IsBuffered( void ) const
{
	return	(GetFrameBuffer() != NULL) ;
}

// レンダリングデバイスの設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSprite::SetRenderDevice( S3DRenderDevice * pDevice )
{
	S3DRenderContext *	pRender = GetBufferRenderer() ;
	if ( pRender == NULL )
	{
		return	sglErrFailed ;
	}
	return	pRender->SetRenderDeviceObject( pDevice ) ;
}

// 描画オブジェクト
//////////////////////////////////////////////////////////////////////////////
S3DRenderContext * SGLSprite::GetBufferRenderer( void ) const
{
	Buffer *	pBuffer = GetFrameBuffer() ;
	if ( pBuffer != NULL )
	{
		return	&(pBuffer->Renderer()) ;
	}
	return	NULL ;
}

S3DRenderDevice * SGLSprite::GetBufferRenderDevice( void ) const
{
	S3DRenderContext *	pRender = GetBufferRenderer() ;
	if ( pRender == nullptr )
	{
		return	nullptr ;
	}
	return	pRender->GetRenderDeviceObject() ;
}

// レンダラ設定
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::SetBufferRenderer( RenderContext * render, bool flagOwner )
{
	Buffer *	pBuffer = GetFrameBuffer() ;
	if ( pBuffer != NULL )
	{
		pBuffer->SetFrameRenderer( render, flagOwner ) ;
	}
	else if ( flagOwner )
	{
		delete	render ;
	}
}

// バッファを取得する
//////////////////////////////////////////////////////////////////////////////
SGLSprite::Buffer * SGLSprite::GetFrameBuffer( void ) const
{
	return	m_pBuffer ;
}

// 背景色取得
//////////////////////////////////////////////////////////////////////////////
bool SGLSprite::GetFillBackColor( uint32_t& argbFill ) const
{
	bool	fBackColor = false ;
	Lock() ;
	Buffer *	pBuffer = GetFrameBuffer() ;
	if ( pBuffer != NULL )
	{
		fBackColor = pBuffer->IsFillBack() ;
		argbFill = pBuffer->GetFillBackColor().ui32 ;
	}
	Unlock() ;
	return	fBackColor ;
}

// 背景色設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSprite::SetFillBackColor
	( uint32_t argbFill, bool flagFillBack )
{
	LockTrace( __FILE__, __LINE__ ) ;
	Buffer *	pBuffer = GetFrameBuffer() ;
	if ( pBuffer == NULL )
	{
		Unlock() ;
		return	sglErrFailed ;
	}
	if ( flagFillBack )
	{
		pBuffer->SetFillBack( SGLPalette( argbFill ) ) ;
	}
	else
	{
		pBuffer->DisableFillBack() ;
	}
	NotifyUpdate() ;
	Unlock() ;
	return	sglErrSuccess ;
}

// フィルタ追加
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::AddReferenceFilter( SGLSpriteFilter * pFilter )
{
	LockTrace( __FILE__, __LINE__ ) ;
	m_filters.Add( pFilter ) ;
	PostUpdate() ;
	Unlock() ;
}

void SGLSprite::AddSmartFilter( SGLSpriteFilter * pFilter )
{
	LockTrace( __FILE__, __LINE__ ) ;
	m_filters.SmartAdd( pFilter ) ;
	PostUpdate() ;
	Unlock() ;
}

// フィルタ削除
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::RemoveFilter( SGLSpriteFilter * pFilter )
{
	LockTrace( __FILE__, __LINE__ ) ;
	ssize_t	iFilter = m_filters.FindPtr( pFilter ) ;
	if ( iFilter >= 0 )
	{
		if ( m_pDrawer.Ptr() == pFilter )
		{
			m_pDrawer = NULL ;
		}
		m_filters.RemoveAt( (size_t) iFilter ) ;
		PostUpdate() ;
	}
	Unlock() ;
}

void SGLSprite::RemoveAllFilter( void )
{
	LockTrace( __FILE__, __LINE__ ) ;
	SGLSpriteFilter *	pFilter =
		ESLTypeCast<SGLSpriteFilter>( m_pDrawer.Ptr() ) ;
	if ( pFilter != NULL )
	{
		ssize_t	iFilter = m_filters.FindPtr( pFilter ) ;
		if ( iFilter >= 0 )
		{
			m_pDrawer = NULL ;
		}
	}
	m_filters.RemoveAll() ;
	PostUpdate() ;
	Unlock() ;
}

// 特定クラスのフィルタ検索
//////////////////////////////////////////////////////////////////////////////
SGLSpriteFilter * SGLSprite::GetFilterTypeOf( const ESLRuntimeClass& rtClass ) const
{
	size_t	iNext = 0 ;
	return	GetFilterTypeOf( rtClass, iNext ) ;
}

SGLSpriteFilter *
	SGLSprite::GetFilterTypeOf
		( const ESLRuntimeClass& rtClass, size_t& iNext ) const
{
	Lock() ;
	for ( size_t i = iNext; i < m_filters.GetLength(); i ++ )
	{
		SGLSpriteFilter *	pFilter = m_filters.GetAt( i ) ;
		if ( pFilter != NULL )
		{
			if ( pFilter->IsKindOf( rtClass ) )
			{
				iNext = i + 1 ;
				Unlock() ;
				return	pFilter ;
			}
		}
	}
	iNext = m_filters.GetLength() ;
	Unlock() ;
	return	NULL ;
}

// フィルタ取得
//////////////////////////////////////////////////////////////////////////////
const SReferenceArray<SGLSpriteFilter>& SGLSprite::GetFilterList( void ) const
{
	return	m_filters ;
}

// 文字列属性
//////////////////////////////////////////////////////////////////////////////
SString SGLSprite::GetText( void ) const
{
	return	SString() ;
}

void SGLSprite::SetText( const wchar_t * pwszText )
{
}

// 文字フォント属性
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::SetTextFont( const wchar_t * pwszFont, int nSize )
{
}

// 入力禁止状態
//////////////////////////////////////////////////////////////////////////////
bool SGLSprite::IsEnabled( void ) const
{
	return	(m_flagsUI & uiDisabled) == 0 ;
}

void SGLSprite::SetEnable( bool fEnable )
{
	uint64_t	flagLast = m_flagsUI ;
	if ( fEnable )
	{
		m_flagsUI &= ~uiDisabled ;
	}
	else
	{
		m_flagsUI |= uiDisabled ;
	}
	if ( flagLast != m_flagsUI )
	{
		LockTrace( __FILE__, __LINE__ ) ;
		NotifyUpdate() ;
		Unlock() ;
	}
}

// スクロール・トラック位置属性
//////////////////////////////////////////////////////////////////////////////
int SGLSprite::GetScrollPos( SGLSprite::ScrollDirection scrlDir )
{
	return	0 ;
}

void SGLSprite::SetScrollPos( int nPos, SGLSprite::ScrollDirection scrlDir )
{
}

// スクロール・トラック位置範囲属性
//////////////////////////////////////////////////////////////////////////////
int SGLSprite::GetScrollRange( SGLSprite::ScrollDirection scrlDir )
{
	return	0 ;
}

void SGLSprite::SetScrollRange
	( int nRange, SGLSprite::ScrollDirection scrlDir )
{
}

// スクロール・ページサイズ属性
//////////////////////////////////////////////////////////////////////////////
int SGLSprite::GetScrollPageSize( SGLSprite::ScrollDirection scrlDir )
{
	return	0 ;
}

void SGLSprite::SetScrollPageSize
	( int nPageSize, SGLSprite::ScrollDirection scrlDir )
{
}

// ボタン属性
//////////////////////////////////////////////////////////////////////////////
bool SGLSprite::IsButtonChecked( void )
{
	return	false ;
}

void SGLSprite::CheckButton( bool fCheck )
{
}

// コマンド処理
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSprite::InvokeCommand
	( const SSystem::SXMLDocument& xmlCmd,
		SSystem::SXMLDocument * pxmlResult )
{
	if ( xmlCmd.GetTag() == L"basic_flag" )
	{
		SString *	pHitTransparency =
			xmlCmd.GetAttributeAs( L"hit_transparency" ) ;
		if ( pHitTransparency != NULL )
		{
			m_flagsUI &= ~uiUnclickable ;
			if ( *pHitTransparency == L"true" )
			{
				m_flagsUI |= uiUnclickable ;
			}
		}
		SString *	pModalFirst =
			xmlCmd.GetAttributeAs( L"modal_first" ) ;
		if ( pModalFirst != NULL )
		{
			m_flagsUI &= ~uiModalFirst ;
			if ( *pModalFirst == L"true" )
			{
				m_flagsUI |= uiModalFirst ;
			}
		}
		SString *	pModalEnd =
			xmlCmd.GetAttributeAs( L"modal_end" ) ;
		if ( pModalEnd != NULL )
		{
			m_flagsUI &= ~uiModalEnd ;
			if ( *pModalEnd == L"true" )
			{
				m_flagsUI |= uiModalEnd ;
			}
		}
		return	sglErrSuccess ;
	}
	else if ( xmlCmd.GetTag() == L"input" )
	{
		SString *	pKey = xmlCmd.GetAttributeAs( L"key" ) ;
		if ( pKey != NULL )
		{
			m_flagsUI &= ~uiDisabledKeyInput ;
			if ( *pKey != L"true" )
			{
				m_flagsUI |= uiDisabledKeyInput ;
			}
		}
		SString *	pWheel = xmlCmd.GetAttributeAs( L"wheel" ) ;
		if ( pWheel != NULL )
		{
			m_flagsUI &= ~uiDisabledMouseWheel ;
			if ( *pWheel != L"true" )
			{
				m_flagsUI |= uiDisabledMouseWheel ;
			}
		}
		return	sglErrSuccess ;
	}
	return	sglErrFailed ;
}

SGLError SGLSprite::InvokeCommands
	( const wchar_t * pwszXMLCommands,
		SSystem::SXMLDocument * pxmlResults )
{
	SXMLDocument	xmlCmds ;
	SStringParser	sparsDoc = pwszXMLCommands ;
	SStrSortObjectArray<SString>	ssoaDTD ;
	if ( xmlCmds.ParseXMLElements( sparsDoc, ssoaDTD, xmlCmds ) )
	{
		return	sglErrFailed ;
	}
	SGLError	err = sglErrSuccess ;
	size_t		nCount = xmlCmds.GetElementsCount() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		SXMLDocument *	pxmlCmd = xmlCmds.GetElementAt( i ) ;
		if ( pxmlCmd != NULL )
		{
			if ( InvokeCommand( *pxmlCmd, pxmlResults ) )
			{
				err = sglErrFailed ;
			}
		}
	}
	return	err ;
}

// スクロールバー関連付け
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::AttachScrollBar
	( SGLSprite * pScrollBar, SGLSprite::ScrollDirection scrlDir )
{
}

// スクロールバー関連付け
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::DetachScrollBar
	( SGLSprite * pScrollBar, SGLSprite::ScrollDirection scrlDir )
{
}

// 可視状態
//////////////////////////////////////////////////////////////////////////////
bool SGLSprite::IsSpriteVisible( const wchar_t * pwszID ) const
{
	bool	fVisible = false ;
	LockTrace( __FILE__, __LINE__ ) ;
	SGLSprite *	pItem = GetItemAs( pwszID ) ;
	if ( pItem != NULL )
	{
		fVisible = pItem->IsVisible() ;
	}
	Unlock() ;
	return	fVisible ;
}

void SGLSprite::SetSpriteVisible( const wchar_t * pwszID, bool fVisible )
{
	LockTrace( __FILE__, __LINE__ ) ;
	SGLSprite *	pItem = GetItemAs( pwszID ) ;
	if ( pItem != NULL )
	{
		pItem->SetVisible( fVisible ) ;
	}
	Unlock() ;
}

// 表示優先度
//////////////////////////////////////////////////////////////////////////////
int SGLSprite::GetSpritePriority( const wchar_t * pwszID ) const
{
	int	nPriority = 0 ;
	LockTrace( __FILE__, __LINE__ ) ;
	SGLSprite *	pItem = GetItemAs( pwszID ) ;
	if ( pItem != NULL )
	{
		nPriority = pItem->GetPriority() ;
	}
	Unlock() ;
	return	nPriority ;
}

void SGLSprite::ChangeSpritePriority
	( const wchar_t * pwszID, int32_t nPriority )
{
	LockTrace( __FILE__, __LINE__ ) ;
	SGLSprite *	pItem = GetItemAs( pwszID ) ;
	if ( pItem != NULL )
	{
		pItem->ChangePriority( nPriority ) ;
	}
	Unlock() ;
}

// 透明度
//////////////////////////////////////////////////////////////////////////////
uint32_t SGLSprite::GetSpriteTransparency( const wchar_t * pwszID ) const
{
	uint32_t	nTrans = 0 ;
	LockTrace( __FILE__, __LINE__ ) ;
	SGLSprite *	pItem = GetItemAs( pwszID ) ;
	if ( pItem != NULL )
	{
		nTrans = pItem->GetTransparency() ;
	}
	Unlock() ;
	return	nTrans ;
}

void SGLSprite::SetSpriteTransparency
	( const wchar_t * pwszID, uint32_t nTransparency )
{
	LockTrace( __FILE__, __LINE__ ) ;
	SGLSprite *	pItem = GetItemAs( pwszID ) ;
	if ( pItem != NULL )
	{
		pItem->SetTransparency( nTransparency ) ;
	}
	Unlock() ;
}

// 表示領域
//////////////////////////////////////////////////////////////////////////////
bool SGLSprite::GetSpriteRectangle( const wchar_t * pwszID, SGLRect& rectExt ) const
{
	bool	fResult = false ;
	LockTrace( __FILE__, __LINE__ ) ;
	SGLSprite *	pItem = GetItemAs( pwszID ) ;
	if ( pItem != NULL )
	{
		fResult = pItem->GetRectangle( rectExt ) ;
	}
	Unlock() ;
	return	fResult ;
}

bool SGLSprite::GetSpriteTextRectangle
		( const wchar_t * pwszID, SGLRect& rectExt ) const
{
	bool	fResult = false ;
	LockTrace( __FILE__, __LINE__ ) ;
	SGLSprite *	pItem = GetItemAs( pwszID ) ;
	if ( pItem != NULL )
	{
		SGLSpriteText *	pTextSprite = ESLTypeCast<SGLSpriteText>( pItem ) ;
		if ( pTextSprite != NULL )
		{
			fResult = pTextSprite->GetTextRectangle( rectExt ) ;
		}
		else
		{
			fResult = pItem->GetRectangle( rectExt ) ;
		}
	}
	Unlock() ;
	return	fResult ;
}

// 文字列属性
//////////////////////////////////////////////////////////////////////////////
SString SGLSprite::GetSpriteText( const wchar_t * pwszID ) const
{
	SString	strText ;
	LockTrace( __FILE__, __LINE__ ) ;
	SGLSprite *	pItem = GetItemAs( pwszID ) ;
	if ( pItem != NULL )
	{
		strText = pItem->GetText() ;
	}
	Unlock() ;
	return	strText ;
}

void SGLSprite::SetSpriteText
		( const wchar_t * pwszID, const wchar_t * pwszText )
{
	LockTrace( __FILE__, __LINE__ ) ;
	SGLSprite *	pItem = GetItemAs( pwszID ) ;
	if ( pItem != NULL )
	{
		pItem->SetText( pwszText ) ;
	}
	Unlock() ;
}

// 文字フォント属性
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::SetSpriteTextFont
	( const wchar_t * pwszID, const wchar_t * pwszFont, int nSize )
{
	LockTrace( __FILE__, __LINE__ ) ;
	SGLSprite *	pItem = GetItemAs( pwszID ) ;
	if ( pItem != NULL )
	{
		pItem->SetTextFont( pwszFont, nSize ) ;
	}
	Unlock() ;
}

// 画像属性
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::SetSpriteImage
	( const wchar_t * pwszID, const wchar_t * pwszImageID )
{
}

// 入力禁止状態
//////////////////////////////////////////////////////////////////////////////
bool SGLSprite::IsSpriteEnabled( const wchar_t * pwszID ) const
{
	bool	fEnabled = false ;
	LockTrace( __FILE__, __LINE__ ) ;
	SGLSprite *	pItem = GetItemAs( pwszID ) ;
	if ( pItem != NULL )
	{
		fEnabled = pItem->IsEnabled() ;
	}
	Unlock() ;
	return	fEnabled ;
}

void SGLSprite::SetSpriteEnable( const wchar_t * pwszID, bool fEnable )
{
	LockTrace( __FILE__, __LINE__ ) ;
	SGLSprite *	pItem = GetItemAs( pwszID ) ;
	if ( pItem != NULL )
	{
		pItem->SetEnable( fEnable ) ;
	}
	Unlock() ;
}

// スクロール・トラック位置属性
//////////////////////////////////////////////////////////////////////////////
int SGLSprite::GetSpriteScrollPos
	( const wchar_t * pwszID, SGLSprite::ScrollDirection scrlDir ) const
{
	int	nPos = 0 ;
	LockTrace( __FILE__, __LINE__ ) ;
	SGLSprite *	pItem = GetItemAs( pwszID ) ;
	if ( pItem != NULL )
	{
		nPos = pItem->GetScrollPos( scrlDir ) ;
	}
	Unlock() ;
	return	nPos ;
}

void SGLSprite::SetSpriteScrollPos
	( const wchar_t * pwszID,
		int nPos, SGLSprite::ScrollDirection scrlDir )
{
	LockTrace( __FILE__, __LINE__ ) ;
	SGLSprite *	pItem = GetItemAs( pwszID ) ;
	if ( pItem != NULL )
	{
		pItem->SetScrollPos( nPos, scrlDir ) ;
	}
	Unlock() ;
}

// スクロール・トラック位置範囲属性
//////////////////////////////////////////////////////////////////////////////
int SGLSprite::GetSpriteScrollRange
	( const wchar_t * pwszID, SGLSprite::ScrollDirection scrlDir ) const
{
	int	nRange = 0 ;
	LockTrace( __FILE__, __LINE__ ) ;
	SGLSprite *	pItem = GetItemAs( pwszID ) ;
	if ( pItem != NULL )
	{
		nRange = pItem->GetScrollRange( scrlDir ) ;
	}
	Unlock() ;
	return	nRange ;
}

void SGLSprite::SetSpriteScrollRange
	( const wchar_t * pwszID,
		int nRange, SGLSprite::ScrollDirection scrlDir )
{
	LockTrace( __FILE__, __LINE__ ) ;
	SGLSprite *	pItem = GetItemAs( pwszID ) ;
	if ( pItem != NULL )
	{
		pItem->SetScrollRange( nRange, scrlDir ) ;
	}
	Unlock() ;
}

// スクロール・ページサイズ属性
//////////////////////////////////////////////////////////////////////////////
int SGLSprite::GetSpriteScrollPageSize
	( const wchar_t * pwszID, SGLSprite::ScrollDirection scrlDir ) const
{
	int	nPageSize = 0 ;
	LockTrace( __FILE__, __LINE__ ) ;
	SGLSprite *	pItem = GetItemAs( pwszID ) ;
	if ( pItem != NULL )
	{
		nPageSize = pItem->GetScrollPageSize( scrlDir ) ;
	}
	Unlock() ;
	return	nPageSize ;
}

void SGLSprite::SetSpriteScrollPageSize
	( const wchar_t * pwszID,
		int nPageSize, SGLSprite::ScrollDirection scrlDir )
{
	LockTrace( __FILE__, __LINE__ ) ;
	SGLSprite *	pItem = GetItemAs( pwszID ) ;
	if ( pItem != NULL )
	{
		pItem->SetScrollPageSize( nPageSize, scrlDir ) ;
	}
	Unlock() ;
}

// ボタン属性
//////////////////////////////////////////////////////////////////////////////
bool SGLSprite::IsSpriteButtonChecked( const wchar_t * pwszID ) const
{
	bool	fChecked = 0 ;
	LockTrace( __FILE__, __LINE__ ) ;
	SGLSprite *	pItem = GetItemAs( pwszID ) ;
	if ( pItem != NULL )
	{
		fChecked = pItem->IsButtonChecked() ;
	}
	Unlock() ;
	return	fChecked ;
}

void SGLSprite::CheckSpriteButton( const wchar_t * pwszID, bool fCheck )
{
	LockTrace( __FILE__, __LINE__ ) ;
	SGLSprite *	pItem = GetItemAs( pwszID ) ;
	if ( pItem != NULL )
	{
		pItem->CheckButton( fCheck ) ;
	}
	Unlock() ;
}

// 擬似 3D 投影スクリーン座標設定
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::SetProjectionScreen
	( const S3DVector& vScreen, double zScale, double fpPixelAspect )
{
	LockTrace( __FILE__, __LINE__ ) ;
	Virtual3DParam *	pV3D = m_pVirtual3D ;
	if ( pV3D == NULL )
	{
		pV3D = new Virtual3DParam ;
		m_pVirtual3D = pV3D ;
	}
	pV3D->m_vProjectScreen = vScreen ;
	pV3D->m_zProjectScale = (float32_t) zScale ;
	pV3D->m_fpPixelAspect = (float32_t) fpPixelAspect ;
	Unlock() ;
}

bool SGLSprite::GetProjectionScreen
	( S3DVector& vScreen, double& zScale, double& fpPixelAspect ) const
{
	bool	fProjection = false ;
	LockTrace( __FILE__, __LINE__ ) ;
	Virtual3DParam *	pV3D = m_pVirtual3D ;
	if ( pV3D != NULL )
	{
		vScreen = pV3D->m_vProjectScreen ;
		zScale = pV3D->m_zProjectScale ;
		fpPixelAspect = pV3D->m_fpPixelAspect ;
		fProjection = true ;
	}
	Unlock() ;
	return	fProjection ;
}

// 視差設定
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::SetParallax
	( double xParallax, double zFocusRate, double xScreenDelta )
{
	LockTrace( __FILE__, __LINE__ ) ;
	Virtual3DParam *	pV3D = m_pVirtual3D ;
	if ( pV3D == NULL )
	{
		pV3D = new Virtual3DParam ;
		m_pVirtual3D = pV3D ;
	}
	pV3D->m_xParallax = (float32_t) xParallax ;
	pV3D->m_zParallaxFocus = (float32_t) zFocusRate ;
	pV3D->m_xParallaxScreen = (float32_t) xScreenDelta ;
	Unlock() ;
}

double SGLSprite::GetParallax( void ) const
{
	double	xParallax = 0 ;
	Lock() ;
	Virtual3DParam *	pV3D = m_pVirtual3D ;
	if ( pV3D != NULL )
	{
		xParallax = pV3D->m_xParallax ;
	}
	Unlock() ;
	return	xParallax ;
}

double SGLSprite::GetParallaxFocusRatio( void ) const
{
	double	zParallaxFocus = 1.0 ;
	Lock() ;
	Virtual3DParam *	pV3D = m_pVirtual3D ;
	if ( pV3D != NULL )
	{
		zParallaxFocus = pV3D->m_zParallaxFocus ;
	}
	Unlock() ;
	return	zParallaxFocus ;
}

double SGLSprite::GetParallaxScreenX( void ) const
{
	double	xParallaxScreen = 0.0 ;
	Lock() ;
	Virtual3DParam *	pV3D = m_pVirtual3D ;
	if ( pV3D != NULL )
	{
		xParallaxScreen = pV3D->m_xParallaxScreen ;
	}
	Unlock() ;
	return	xParallaxScreen ;
}

// カメラ設定
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::SetVirtualCamera
	( const S3DDVector& vCamera, const S3DDVector& vTarget )
{
	LockTrace( __FILE__, __LINE__ ) ;
	Virtual3DParam *	pV3D = m_pVirtual3D ;
	if ( pV3D == NULL )
	{
		pV3D = new Virtual3DParam ;
		m_pVirtual3D = pV3D ;
	}
	pV3D->m_vCameraView = vCamera ;
	pV3D->m_vCameraTarget = vTarget ;
	//
	PostUpdate() ;
	Unlock() ;
}

// 仮想３Ｄ設定取得
//////////////////////////////////////////////////////////////////////////////
SGLSprite::Virtual3DParam * SGLSprite::GetVirtual3DParam( void ) const
{
	return	m_pVirtual3D ;
}

// タイマ処理追加
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::AddReferenceTimer( SGLSpriteTimer * pTimer )
{
	Lock() ;
	if ( m_timers.FindPtr( pTimer ) < 0 )
	{
		m_timers.Add( pTimer ) ;
	}
	Unlock() ;
}

void SGLSprite::AddSmartTimer( SGLSpriteTimer * pTimer )
{
	Lock() ;
	if ( m_timers.FindPtr( pTimer ) < 0 )
	{
		m_timers.SmartAdd( pTimer ) ;
	}
	Unlock() ;
}

// タイマ処理削除
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::RemoveTimer( SGLSpriteTimer * pTimer )
{
	LockTrace( __FILE__, __LINE__ ) ;
	ssize_t	iTimer = m_timers.FindPtr( pTimer ) ;
	if ( iTimer >= 0 )
	{
		m_timers.RemoveAt( iTimer ) ;
	}
	Unlock() ;
}

void SGLSprite::RemoveAllTimer( void )
{
	LockTrace( __FILE__, __LINE__ ) ;
	m_timers.RemoveAll() ;
	Unlock() ;
}

// 特定クラスのフィルタ検索
//////////////////////////////////////////////////////////////////////////////
SGLSpriteTimer * SGLSprite::GetTimerTypeOf( const ESLRuntimeClass& rtClass ) const
{
	Lock() ;
	for ( size_t i = 0; i < m_timers.GetLength(); i ++ )
	{
		SGLSpriteTimer *	pTimer = m_timers.GetAt( i ) ;
		if ( pTimer != NULL )
		{
			if ( pTimer->IsKindOf( rtClass ) )
			{
				Unlock() ;
				return	pTimer ;
			}
		}
	}
	Unlock() ;
	return	NULL ;
}

// 簡易アニメーション設定
//////////////////////////////////////////////////////////////////////////////
SGLSpriteAction * SGLSprite::SetActionLinearTo
	( uint32_t msecDuration,
		uint32_t nTransparency,
		const S2DDVector * pPos,
		const S2DDVector * pZoom,
		double a0, double a1 )
{
	SGLSpriteAction *	pAct = new SGLSpriteAction ;
	pAct->SetTransparencyTo( *this, nTransparency ) ;
	if ( pPos != NULL )
	{
		pAct->SetMoveTo( *this, pPos->x, pPos->y, a0, a1 ) ;
	}
	if ( pZoom != NULL )
	{
		pAct->SetZoomTo( *this, pZoom->x, pZoom->y, a0, a1 ) ;
	}
	pAct->SetDuration( msecDuration ) ;
	AddAction( pAct ) ;
	return	pAct ;
}

// アニメーション追加
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::AddAction( SGLSprite::Action * pAct )
{
	LockTrace( __FILE__, __LINE__ ) ;
	if ( m_actions.GetLength() == 0 )
	{
		m_paramAction = m_paramView ;
	}
	m_actions.Add( pAct ) ;
	UpdateAllActions( 0 ) ;
	Unlock() ;
}

void SGLSprite::InsertActionAt( size_t i, Action * pAct )
{
	LockTrace( __FILE__, __LINE__ ) ;
	if ( m_actions.GetLength() == 0 )
	{
		m_paramAction = m_paramView ;
	}
	m_actions.InsertAt( i, pAct ) ;
	UpdateAllActions( 0 ) ;
	Unlock() ;
}

// アニメーション即時完了
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::FlushAction( void )
{
	LockTrace( __FILE__, __LINE__ ) ;
	size_t		nCount = m_actions.GetLength() ;
	bool		fFlush = false ;
	Parameter	paramAction = m_paramAction ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		Action *	pAct = m_actions.GetAt( i ) ;
		if ( pAct != NULL )
		{
			pAct->OnFinish( paramAction ) ;
			fFlush = true ;
		}
	}
	m_actions.RemoveAll() ;
	//
	if ( fFlush )
	{
		NotifyUpdate() ;
		if ( m_paramView.paramFilter != paramAction.paramFilter )
		{
			m_paramView = paramAction ;
			PostUpdate() ;
		}
		else
		{
			m_paramView = paramAction ;
			NotifyUpdate() ;
		}
	}
	Unlock() ;
}

void SGLSprite::FlushAction( ActionType type )
{
	LockTrace( __FILE__, __LINE__ ) ;
	size_t		nCount = m_actions.GetLength() ;
	bool		fFlush = false ;
	Parameter	paramAction = m_paramAction ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		Action *	pAct = m_actions.GetAt( i ) ;
		if ( (pAct != NULL) && (pAct->m_typeAction == type) )
		{
			pAct->OnFinish( paramAction ) ;
			m_actions.SetAt( i, NULL ) ;
			fFlush = true ;
		}
	}
	if ( fFlush )
	{
		m_actions.TrimEmpty() ;
		//
		NotifyUpdate() ;
		if ( m_paramView.paramFilter != paramAction.paramFilter )
		{
			m_paramView = paramAction ;
			PostUpdate() ;
		}
		else
		{
			m_paramView = paramAction ;
			NotifyUpdate() ;
		}
	}
	Unlock() ;
}

// アニメーションキャンセル
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::CancelAction( void )
{
	LockTrace( __FILE__, __LINE__ ) ;
	m_actions.RemoveAll() ;
	Unlock() ;
}

void SGLSprite::CancelAction( ActionType type )
{
	LockTrace( __FILE__, __LINE__ ) ;
	size_t		nCount = m_actions.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		Action *	pAct = m_actions.GetAt( i ) ;
		if ( (pAct != NULL) && (pAct->m_typeAction == type) )
		{
			m_actions.SetAt( i, NULL ) ;
		}
	}
	m_actions.TrimEmpty() ;
	Unlock() ;
}

// アニメーション中か？
//////////////////////////////////////////////////////////////////////////////
bool SGLSprite::IsAction( void ) const
{
	return	(m_actions.GetLength() != 0) ;
}

bool SGLSprite::IsAction( ActionType type ) const
{
	bool	flagInAction = false ;
	LockTrace( __FILE__, __LINE__ ) ;
	size_t		nCount = m_actions.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		Action *	pAct = m_actions.GetAt( i ) ;
		if ( (pAct != NULL) && (pAct->m_typeAction == type) )
		{
			flagInAction = true ;
			break ;
		}
	}
	Unlock() ;
	return	flagInAction ;
}

// 全アニメーション一時停止
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::PauseAllAction( void )
{
	LockTrace( __FILE__, __LINE__ ) ;
	size_t	nCount = m_actions.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		Action *	pAct = m_actions.GetAt( i ) ;
		if ( pAct != NULL )
		{
			pAct->Pause() ;
		}
	}
	Unlock() ;
}

// 全アニメーション再開
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::RestartAllAction( void )
{
	LockTrace( __FILE__, __LINE__ ) ;
	size_t	nCount = m_actions.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		Action *	pAct = m_actions.GetAt( i ) ;
		if ( pAct != NULL )
		{
			pAct->Restart() ;
		}
	}
	Unlock() ;
}

// アニメーション処理
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::UpdateAllActions( uint32_t msecPast )
{
	Parameter		param = m_paramAction ;
	const size_t	nCount = m_actions.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		Action *	pAction = m_actions.GetAt( i ) ;
		if ( pAction != NULL )
		{
			if ( pAction->OnAction( param, msecPast ) )
			{
				m_actions.SetAt( i, NULL ) ;
			}
		}
	}
	m_actions.TrimEmpty() ;
	//
	SGLRect	rectExt1 ;
	bool	fRect1 = GetRectangle( rectExt1 ) ;
	//
	bool	fFilter = (m_paramView.paramFilter != param.paramFilter)
					|| (m_paramView.paramFilter2 != param.paramFilter2) ;
	m_paramView = param ;
	//
	if ( fFilter )
	{
		PostUpdate() ;
	}
	else
	{
		SGLSprite *	pParent = m_refParent.GetReference() ;
		if ( pParent != NULL )
		{
			SGLRect	rectExt2 ;
			if ( fRect1 && GetRectangle( rectExt2 ) )
			{
				rectExt1 |= rectExt2 ;
				pParent->PostUpdate( &rectExt1 ) ;
			}
			else
			{
				pParent->PostUpdate() ;
			}
		}
	}
}

// マウス入力リスナ設定
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::AttachMouseListener( SGLSpriteMouseListener * pListener )
{
	Lock() ;
	m_refMouseListener.SetReference( pListener ) ;
	Unlock() ;
}

// マウス入力リスナ解除
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::DetachMouseListener( SGLSpriteMouseListener * pListener )
{
	Lock() ;
	if ( m_refMouseListener.GetReference() == pListener )
	{
		m_refMouseListener.ReleaseReference() ;
	}
	Unlock() ;
}

// マウス入力リスナ設定
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::AttachKeyListener( SGLSpriteKeyListener * pListener )
{
	Lock() ;
	m_refKeyListener.SetReference( pListener ) ;
	Unlock() ;
}

// マウス入力リスナ解除
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::DetachKeyListener( SGLSpriteKeyListener * pListener )
{
	Lock() ;
	if ( m_refKeyListener.GetReference() == pListener )
	{
		m_refKeyListener.ReleaseReference() ;
	}
	Unlock() ;
}

// マウス入力キャプチャー要求
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSprite::SetMouseCapture( void )
{
	LockTrace( __FILE__, __LINE__ ) ;
	SGLSprite *	pParent = m_refParent.GetReference() ;
	if ( pParent == NULL )
	{
		Unlock() ;
		return	sglErrFailed ;
	}
	SGLSprite *	pCaptured = pParent->m_refCaptured.GetReference() ;
	if ( pCaptured != this )
	{
		if ( pCaptured != NULL )
		{
			pCaptured->OnReleaseMouseCapture() ;
		}
		pParent->m_refCaptured.SetReference( this ) ;
	}
	SGLError	err = pParent->SetMouseCapture() ;
	Unlock() ;
	return	err ;
}

// マウス入力キャプチャー解放
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSprite::ReleaseMouseCapture( void )
{
	LockTrace( __FILE__, __LINE__ ) ;
	SGLSprite *	pParent = m_refParent.GetReference() ;
	if ( pParent == NULL )
	{
		Unlock() ;
		return	sglErrFailed ;
	}
	SGLSprite *	pCaptured = pParent->m_refCaptured.GetReference() ;
	if ( pCaptured == this )
	{
		if ( pCaptured != NULL )
		{
			pCaptured->OnReleaseMouseCapture() ;
		}
		pParent->m_refCaptured.ReleaseReference() ;
	}
	SGLError	err = pParent->ReleaseMouseCapture() ;
	Unlock() ;
	return	err ;
}

// マウスキャプチャーが解放された
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::OnReleaseMouseCapture( void )
{
}

// キャプチャー中の子スプライトを取得
//////////////////////////////////////////////////////////////////////////////
SGLSprite * SGLSprite::GetMouseCapture( void ) const
{
	SGLSprite *	pCapture = NULL ;
	Lock() ;
	pCapture = m_refCaptured.GetReference() ;
	Unlock() ;
	return	pCapture ;
}

// キーフォーカスを要求
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSprite::SetKeyFocus( void )
{
	LockTrace( __FILE__, __LINE__ ) ;
	SGLSprite *	pParent = m_refParent.GetReference() ;
	if ( pParent == NULL )
	{
		Unlock() ;
		return	sglErrFailed ;
	}
	if ( m_flagsUI & uiFocusable )
	{
		SGLSprite *	pFocus = pParent->m_refKeyFocus.GetReference() ;
		if ( pFocus != this )
		{
			if ( pFocus != NULL )
			{
				pFocus->OnKillKeyFocus() ;
			}
			pParent->m_refKeyFocus.SetReference( this ) ;
			OnSetKeyFocus() ;
		}
		m_flagsUI |= uiHaveFocus ;
	}
	SGLError	err = pParent->SetKeyFocus() ;
	Unlock() ;
	return	err ;
}

// キーフォーカスを解除
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSprite::KillKeyFocus( void )
{
	LockTrace( __FILE__, __LINE__ ) ;
	SGLSprite *	pParent = m_refParent.GetReference() ;
	if ( pParent == NULL )
	{
		Unlock() ;
		return	sglErrFailed ;
	}
	SGLSprite *	pFocus = pParent->m_refKeyFocus.GetReference() ;
	if ( pFocus == this )
	{
		if ( pFocus != NULL )
		{
			pFocus->OnKillKeyFocus() ;
		}
		pParent->m_refKeyFocus.ReleaseReference() ;
		OnKillKeyFocus() ;
	}
	SGLError	err = pParent->KillKeyFocus() ;
	Unlock() ;
	return	err ;
}

// フォーカスが設定された
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::OnSetKeyFocus( void )
{
}

// キーフォーカスが解除された
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::OnKillKeyFocus( void )
{
	LockTrace( __FILE__, __LINE__ ) ;
	SGLSprite *	pFocus = m_refKeyFocus ;
	if ( pFocus != NULL )
	{
		pFocus->OnKillKeyFocus() ;
	}
	m_flagsUI &= ~uiHaveFocus ;
	m_refKeyFocus.ReleaseReference() ;
	Unlock() ;
}

// 次のフォーカスを受け取り可能なスプライトへフォーカスを移動する
//////////////////////////////////////////////////////////////////////////////
bool SGLSprite::MoveNextKeyFocus( void )
{
	LockTrace( __FILE__, __LINE__ ) ;
	SGLSprite *	pKeyFocus = m_refKeyFocus ;
	ssize_t		iKeyFocus = -1 ;
	if ( pKeyFocus != NULL )
	{
		iKeyFocus = m_children.FindPtr( pKeyFocus ) ;
	}
	size_t	iNextFirst = 0 ;
	if ( iKeyFocus >= 0 )
	{
		iNextFirst = iKeyFocus + 1 ;
	}
	size_t	nCount = m_children.GetLength() ;
	while ( iNextFirst < nCount )
	{
		SGLSprite *	pChild = m_children.GetAt( iNextFirst ++ ) ;
		if ( pChild != NULL )
		{
			if ( pChild->m_flagsUI & uiFocusable )
			{
				pChild->SetKeyFocus() ;
				Unlock() ;
				return	true ;
			}
			if ( pChild->m_flagsUI & uiModalEnd )
			{
				Unlock() ;
				return	true ;
			}
			if ( pChild->MoveNextKeyFocus() )
			{
				Unlock() ;
				return	true ;
			}
		}
	}
	if ( pKeyFocus != NULL )
	{
		pKeyFocus->KillKeyFocus() ;
	}
	Unlock() ;
	return	false ;
}

// 前のフォーカスを受け取り可能なスプライトへフォーカスを移動する
//////////////////////////////////////////////////////////////////////////////
bool SGLSprite::MovePrevKeyFocus( void )
{
	LockTrace( __FILE__, __LINE__ ) ;
	SGLSprite *	pKeyFocus = m_refKeyFocus ;
	ssize_t		iKeyFocus = -1 ;
	if ( pKeyFocus != NULL )
	{
		iKeyFocus = m_children.FindPtr( pKeyFocus ) ;
	}
	ssize_t	iNextFirst = (ssize_t) m_children.GetLength() - 1 ;
	if ( iKeyFocus >= 0 )
	{
		iNextFirst = iKeyFocus - 1 ;
	}
	while ( iNextFirst >= 0 )
	{
		SGLSprite *	pChild = m_children.GetAt( iNextFirst -- ) ;
		if ( pChild != NULL )
		{
			if ( pChild->m_flagsUI & uiFocusable )
			{
				pChild->SetKeyFocus() ;
				Unlock() ;
				return	true ;
			}
			if ( pChild->m_flagsUI & uiModalFirst )
			{
				Unlock() ;
				return	true ;
			}
			if ( pChild->MovePrevKeyFocus() )
			{
				Unlock() ;
				return	true ;
			}
		}
	}
	if ( pKeyFocus != NULL )
	{
		pKeyFocus->KillKeyFocus() ;
	}
	Unlock() ;
	return	false ;
}

// 指定の方向へフォーカスを移動する
//////////////////////////////////////////////////////////////////////////////
bool SGLSprite::MoveKeyFocusDirectionOf( const S2DDVector& vDir )
{
	LockTrace( __FILE__, __LINE__ ) ;
	SGLSprite *	pParent = m_refParent ;
	if ( (pParent != NULL) && !(pParent->m_flagsUI & uiNoMoveFocusByKey) )
	{
		S2DDVector	vPos ;
		if ( GetFocusPoint( vPos ) )
		{
			SGLSprite *	pNext =
				pParent->SearchNearestFocusableSprite( this, vPos, vDir ) ;
			if ( pNext != NULL )
			{
				pNext->SetKeyFocus() ;
				Unlock() ;
				return	true ;
			}
		}
	}
	else
	{
		SGLSprite *	pKeyFocus = m_refKeyFocus ;
		if ( pKeyFocus == NULL )
		{
			if ( MoveNextKeyFocus() )
			{
				Unlock() ;
				return	true ;
			}
		}
	}
	Unlock() ;
	return	false ;
}

// マウスフォーカス取得
//////////////////////////////////////////////////////////////////////////////
SGLSprite * SGLSprite::GetMouseFocusAt
	( S2DDVector& vPos, double xPos, double yPos, int64_t nFlags )
{
	// ※Mouse ID は SGLWindowSprite レイヤで必ず 0 から始まる指標に
	// 　正規化されているため基本的に大きな値はとらない
	uint32_t	idMouse = GetMouseID(nFlags) ;
	ESLAssert( idMouse < 0x100 ) ;
	idMouse &= 0x0FF ;
	//
	SGLSprite *	pChild = m_refCaptured.GetReference() ;
	SGLSprite *	pLast = m_rfarMouseFocus.GetAt( idMouse ) ;
	vPos.x = xPos ;
	vPos.y = yPos ;
	if ( pChild != NULL )
	{
		pChild->GlobalToLocal( vPos ) ;
	}
	else
	{
		pChild = GetHitSpriteAt( vPos ) ;
		if ( (pChild != pLast) && (pLast != NULL) && (pLast != this) )
		{
			pLast->OnMouseLeave( nFlags ) ;
		}
		m_rfarMouseFocus.SetAt( idMouse, pChild ) ;
	}
	if ( (pChild != NULL) && (pChild->m_flagsUI & uiDisabled) )
	{
		return	NULL ;
	}
	return	pChild ;
}

// マウス移動
//////////////////////////////////////////////////////////////////////////////
bool SGLSprite::OnMouseMove
	( double xPos, double yPos, int64_t nFlags )
{
	bool	flagProcessed = false ;
	if ( !(nFlags & SGLSpriteMouseListener::NoMouseListener)
									/*&& (m_refCaptured == NULL)*/ )
	{
		SGLSpriteMouseListener *
				pListener = m_refMouseListener.GetReference() ;
		if ( pListener != NULL )
		{
			if ( pListener->OnMouseMove( *this, xPos, yPos, nFlags ) )
			{
				flagProcessed = true ;
			}
		}
	}
	S2DDVector	vPos ;
	SGLSprite *	pChild = GetMouseFocusAt( vPos, xPos, yPos, nFlags ) ;
	if ( (pChild == NULL) || (pChild == this) )
	{
		return	flagProcessed ;
	}
	return	pChild->OnMouseMove( vPos.x, vPos.y, nFlags ) || flagProcessed ;
}

void SGLSprite::OnMouseLeave( int64_t nFlags )
{
	if ( !(nFlags & SGLSpriteMouseListener::NoMouseListener) )
	{
		SGLSpriteMouseListener *
				pListener = m_refMouseListener.GetReference() ;
		if ( pListener != NULL )
		{
			pListener->OnMouseLeave( *this, nFlags ) ;
		}
	}
	// ※Mouse ID は SGLWindowSprite レイヤで必ず 0 から始まる指標に
	// 　正規化されているため基本的に大きな値はとらない
	uint32_t	idMouse = GetMouseID(nFlags) ;
	ESLAssert( idMouse < 0x100 ) ;
	idMouse &= 0x0FF ;
	//
	SGLSprite *	pLast = m_rfarMouseFocus.GetAt( idMouse ) ;
	if ( (pLast != NULL) && (pLast != this) )
	{
		pLast->OnMouseLeave( nFlags ) ;
	}
}

// マウスカーソル取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * SGLSprite::HitTestMouseCursor
	( double xPos, double yPos, int64_t nFlags )
{
	S2DDVector	vPos ;
	SGLSprite *	pChild = GetMouseFocusAt( vPos, xPos, yPos, nFlags ) ;
	if ( (pChild == NULL) || (pChild == this) )
	{
		return	NULL ;
	}
	return	pChild->HitTestMouseCursor( vPos.x, vPos.y, nFlags ) ;
}

// ホイール回転
//////////////////////////////////////////////////////////////////////////////
bool SGLSprite::OnMouseWheel
	( int32_t zDelta, double xPos, double yPos, int64_t nFlags )
{
	if ( !(nFlags & SGLSpriteMouseListener::NoMouseListener)
									/*&& (m_refCaptured == NULL)*/ )
	{
		SGLSpriteMouseListener *
				pListener = m_refMouseListener.GetReference() ;
		if ( pListener != NULL )
		{
			if ( pListener->OnMouseWheel( *this, zDelta, xPos, yPos, nFlags ) )
			{
				return	true ;
			}
		}
	}
	S2DDVector	vPos ;
	SGLSprite *	pChild = GetMouseFocusAt( vPos, xPos, yPos, nFlags ) ;
	if ( (pChild == NULL)
		|| (pChild == this)
		|| (pChild->m_flagsUI & uiDisabledMouseWheel) )
	{
		return	false ;
	}
	return	pChild->OnMouseWheel( zDelta, vPos.x, vPos.y, nFlags ) ;
}

// マウスボタン
//////////////////////////////////////////////////////////////////////////////
bool SGLSprite::OnButtonDown
	( double xPos, double yPos, int64_t nFlags )
{
	if ( !(nFlags & SGLSpriteMouseListener::NoMouseListener)
									/*&& (m_refCaptured == NULL)*/ )
	{
		SGLSpriteMouseListener *
				pListener = m_refMouseListener.GetReference() ;
		if ( pListener != NULL )
		{
			if ( pListener->OnButtonDown( *this, xPos, yPos, nFlags ) )
			{
				return	true ;
			}
		}
	}
	uint32_t	id = SGLMouseInterface::GetButtonID( nFlags ) ;
	if ( id == SGLSpriteMouseListener::LeftButtonID )
	{
		if ( OnLButtonDown( xPos, yPos, nFlags ) )
		{
			return	true ;
		}
	}
	else if ( id == SGLSpriteMouseListener::RightButtonID )
	{
		if ( OnRButtonDown( xPos, yPos, nFlags ) )
		{
			return	true ;
		}
	}
	else if ( id == SGLSpriteMouseListener::MiddleButtonID )
	{
		if ( OnMButtonDown( xPos, yPos, nFlags ) )
		{
			return	true ;
		}
	}
	S2DDVector	vPos ;
	SGLSprite *	pChild = GetMouseFocusAt( vPos, xPos, yPos, nFlags ) ;
	if ( (pChild != NULL) && (pChild != this) )
	{
		if ( pChild->OnButtonDown( vPos.x, vPos.y, nFlags ) )
		{
			return	true ;
		}
	}
	if ( !(nFlags & SGLSpriteMouseListener::NoMouseListener) )
	{
		SGLSpriteMouseListener *
				pListener = m_refMouseListener.GetReference() ;
		if ( pListener != NULL )
		{
			if ( pListener->AfterButtonDown( *this, xPos, yPos, nFlags ) )
			{
				return	true ;
			}
		}
	}
	return	false ;
}

bool SGLSprite::OnButtonUp
	( double xPos, double yPos, int64_t nFlags )
{
	if ( !(nFlags & SGLSpriteMouseListener::NoMouseListener)
									/*&& (m_refCaptured == NULL)*/ )
	{
		SGLSpriteMouseListener *
				pListener = m_refMouseListener.GetReference() ;
		if ( pListener != NULL )
		{
			if ( pListener->OnButtonUp( *this, xPos, yPos, nFlags ) )
			{
				return	true ;
			}
		}
	}
	uint32_t	id = SGLSpriteMouseListener::GetButtonID( nFlags ) ;
	if ( id == SGLSpriteMouseListener::LeftButtonID )
	{
		if ( OnLButtonUp( xPos, yPos, nFlags ) )
		{
			return	true ;
		}
	}
	else if ( id == SGLSpriteMouseListener::RightButtonID )
	{
		if ( OnRButtonUp( xPos, yPos, nFlags ) )
		{
			return	true ;
		}
	}
	else if ( id == SGLSpriteMouseListener::MiddleButtonID )
	{
		if ( OnMButtonUp( xPos, yPos, nFlags ) )
		{
			return	true ;
		}
	}
	S2DDVector	vPos ;
	SGLSprite *	pChild = GetMouseFocusAt( vPos, xPos, yPos, nFlags ) ;
	if ( (pChild != NULL) && (pChild != this) )
	{
		if ( pChild->OnButtonUp( vPos.x, vPos.y, nFlags ) )
		{
			return	true ;
		}
	}
	if ( !(nFlags & SGLSpriteMouseListener::NoMouseListener) )
	{
		SGLSpriteMouseListener *
				pListener = m_refMouseListener.GetReference() ;
		if ( pListener != NULL )
		{
			if ( pListener->AfterButtonUp( *this, xPos, yPos, nFlags ) )
			{
				return	true ;
			}
		}
	}
	return	false ;
}

bool SGLSprite::OnButtonDblClk
	( double xPos, double yPos, int64_t nFlags )
{
	if ( !(nFlags & SGLSpriteMouseListener::NoMouseListener)
									/*&& (m_refCaptured == NULL)*/ )
	{
		SGLSpriteMouseListener *
				pListener = m_refMouseListener.GetReference() ;
		if ( pListener != NULL )
		{
			if ( pListener->OnButtonDblClk( *this, xPos, yPos, nFlags ) )
			{
				return	true ;
			}
		}
	}
	uint32_t	id = SGLSpriteMouseListener::GetButtonID( nFlags ) ;
	if ( id == SGLSpriteMouseListener::LeftButtonID )
	{
		if ( OnLButtonDblClk( xPos, yPos, nFlags ) )
		{
			return	true ;
		}
	}
	else if ( id == SGLSpriteMouseListener::RightButtonID )
	{
		if ( OnRButtonDblClk( xPos, yPos, nFlags ) )
		{
			return	true ;
		}
	}
	else if ( id == SGLSpriteMouseListener::MiddleButtonID )
	{
		if ( OnMButtonDblClk( xPos, yPos, nFlags ) )
		{
			return	true ;
		}
	}
	S2DDVector	vPos ;
	SGLSprite *	pChild = GetMouseFocusAt( vPos, xPos, yPos, nFlags ) ;
	if ( (pChild != NULL) && (pChild != this) )
	{
		if ( pChild->OnButtonDblClk( vPos.x, vPos.y, nFlags ) )
		{
			return	true ;
		}
	}
	if ( !(nFlags & SGLSpriteMouseListener::NoMouseListener) )
	{
		SGLSpriteMouseListener *
				pListener = m_refMouseListener.GetReference() ;
		if ( pListener != NULL )
		{
			if ( pListener->AfterRButtonDblClk( *this, xPos, yPos, nFlags ) )
			{
				return	true ;
			}
		}
	}
	return	false ;
}

// 左ボタン
//////////////////////////////////////////////////////////////////////////////
bool SGLSprite::OnLButtonDown
	( double xPos, double yPos, int64_t nFlags )
{
	if ( m_flagsUI & uiFocusable )
	{
		SetKeyFocus() ;
	}
	return	false ;
}

bool SGLSprite::OnLButtonUp
	( double xPos, double yPos, int64_t nFlags )
{
	return	false ;
}

bool SGLSprite::OnLButtonDblClk
	( double xPos, double yPos, int64_t nFlags )
{
	return	false ;
}

// 右ボタン
//////////////////////////////////////////////////////////////////////////////
bool SGLSprite::OnRButtonDown
	( double xPos, double yPos, int64_t nFlags )
{
	return	false ;
}

bool SGLSprite::OnRButtonUp
	( double xPos, double yPos, int64_t nFlags )
{
	return	false ;
}

bool SGLSprite::OnRButtonDblClk
	( double xPos, double yPos, int64_t nFlags )
{
	return	false ;
}

// 中央ボタン
//////////////////////////////////////////////////////////////////////////////
bool SGLSprite::OnMButtonDown
	( double xPos, double yPos, int64_t nFlags )
{
	return	false ;
}

bool SGLSprite::OnMButtonUp
	( double xPos, double yPos, int64_t nFlags )
{
	return	false ;
}

bool SGLSprite::OnMButtonDblClk
	( double xPos, double yPos, int64_t nFlags )
{
	return	false ;
}

// キー入力
//////////////////////////////////////////////////////////////////////////////
bool SGLSprite::OnKeyDown
	( int64_t nVirtKey, int64_t nFlags )
{
	SGLSpriteKeyListener *	pListener = m_refKeyListener.GetReference() ;
	if ( pListener != NULL )
	{
		if ( pListener->OnKeyDown( *this, nVirtKey, nFlags ) )
		{
			return	true ;
		}
	}
	SGLSprite *	pFocus = m_refKeyFocus.GetReference() ;
	if ( pFocus != NULL )
	{
		if ( pFocus->OnKeyDown( nVirtKey, nFlags ) )
		{
			return	true ;
		}
	}
	if ( !(m_flagsUI & uiNoMoveFocusByKey) )
	{
		if ( nVirtKey == vkeyTab )
		{
			if ( nFlags & vkeyContextShift )
			{
				MovePrevKeyFocus() ;
			}
			else
			{
				MoveNextKeyFocus() ;
			}
		}
		else if ( nVirtKey == vkeyLeft )
		{
			MoveKeyFocusDirectionOf( S2DDVector( -1, 0 ) ) ;
		}
		else if ( nVirtKey == vkeyUp )
		{
			MoveKeyFocusDirectionOf( S2DDVector( 0, -1 ) ) ;
		}
		else if ( nVirtKey == vkeyRight )
		{
			MoveKeyFocusDirectionOf( S2DDVector( 1, 0 ) ) ;
		}
		else if ( nVirtKey == vkeyDown )
		{
			MoveKeyFocusDirectionOf( S2DDVector( 0, 1 ) ) ;
		}
	}
	return	false ;
}

bool SGLSprite::OnKeyUp
	( int64_t nVirtKey, int64_t nFlags )
{
	SGLSpriteKeyListener *	pListener = m_refKeyListener.GetReference() ;
	if ( pListener != NULL )
	{
		if ( pListener->OnKeyUp( *this, nVirtKey, nFlags ) )
		{
			return	true ;
		}
	}
	SGLSprite *	pFocus = m_refKeyFocus.GetReference() ;
	if ( pFocus != NULL )
	{
		return	pFocus->OnKeyUp( nVirtKey, nFlags ) ;
	}
	return	false ;
}

// 文字入力
//////////////////////////////////////////////////////////////////////////////
bool SGLSprite::OnChar( uint16_t codeChar )
{
	SGLSpriteKeyListener *	pListener = m_refKeyListener.GetReference() ;
	if ( pListener != NULL )
	{
		if ( pListener->OnChar( *this, codeChar ) )
		{
			return	true ;
		}
	}
	SGLSprite *	pFocus = m_refKeyFocus.GetReference() ;
	if ( pFocus != NULL )
	{
		return	pFocus->OnChar( codeChar ) ;
	}
	return	false ;
}

// コンポジション開始
//////////////////////////////////////////////////////////////////////////////
bool SGLSprite::OnStartComposition
	(  SGLInputStartComposition& iscForm )
{
	SGLSpriteKeyListener *	pListener = m_refKeyListener.GetReference() ;
	if ( pListener != NULL )
	{
		if ( pListener->OnStartComposition( *this, iscForm ) )
		{
			return	true ;
		}
	}
	SGLSprite *	pFocus = m_refKeyFocus.GetReference() ;
	if ( pFocus != NULL )
	{
		if ( pFocus->OnStartComposition( iscForm ) )
		{
			if ( iscForm.nFlags & SGLInputStartComposition::flagPosition )
			{
				S2DDVector	vPos( iscForm.ptStart.x, iscForm.ptStart.y ) ;
				if ( LocalToGlobal( vPos ) )
				{
					iscForm.ptStart.x = (int32_t) vPos.x ;
					iscForm.ptStart.y = (int32_t) vPos.y ;
				}
			}
			if ( iscForm.nFlags & SGLInputStartComposition::flagRectangle )
			{
				SGLRect	rect( iscForm.rctArea ) ;
				if ( LocalToGlobalRect( rect ) )
				{
					iscForm.rctArea = rect ;
				}
			}
			return	true ;
		}
	}
	return	false ;
}

// コンポジション終了
//////////////////////////////////////////////////////////////////////////////
bool SGLSprite::OnEndComposition( void )
{
	SGLSpriteKeyListener *	pListener = m_refKeyListener.GetReference() ;
	if ( pListener != NULL )
	{
		if ( pListener->OnEndComposition( *this ) )
		{
			return	true ;
		}
	}
	SGLSprite *	pFocus = m_refKeyFocus.GetReference() ;
	if ( pFocus != NULL )
	{
		return	pFocus->OnEndComposition() ;
	}
	return	false ;
}

// コンポジション文字列
//////////////////////////////////////////////////////////////////////////////
bool SGLSprite::OnCompositionString
	( const SGLInputCompositionString& icsComp )
{
	SGLSpriteKeyListener *	pListener = m_refKeyListener.GetReference() ;
	if ( pListener != NULL )
	{
		if ( pListener->OnCompositionString( *this, icsComp ) )
		{
			return	true ;
		}
	}
	SGLSprite *	pFocus = m_refKeyFocus.GetReference() ;
	if ( pFocus != NULL )
	{
		return	pFocus->OnCompositionString( icsComp ) ;
	}
	return	false ;
}

// コマンド通知
//////////////////////////////////////////////////////////////////////////////
bool SGLSprite::NotifyCommand
	( const wchar_t * pszCmd,
		int64_t nParam, int64_t nCode, int nPriority, bool fOverwritable )
{
	SGLSprite *	pParent = m_refParent.GetReference() ;
	if ( pParent != NULL )
	{
		if ( m_strID.IsEmpty() )
		{
			return	pParent->OnCommand
				( pszCmd, nParam, nCode, nPriority, fOverwritable ) ;
		}
		else
		{
			SString	strCmd = m_strID ;
			strCmd += L'\\' ;
			strCmd += pszCmd ;
			return	pParent->OnCommand
				( strCmd, nParam, nCode, nPriority, fOverwritable ) ;
		}
	}
	else
	{
		return	DispatchCommand( pszCmd, nParam, nCode ) ;
	}
	return	false ;
}

// コマンド
//////////////////////////////////////////////////////////////////////////////
bool SGLSprite::OnCommand
	( const wchar_t * pszCmd,
		int64_t nParam, int64_t nCode, int nPriority, bool fOverwritable )
{
	SGLSpriteKeyListener *	pListener = m_refKeyListener.GetReference() ;
	if ( pListener != NULL )
	{
		if ( pListener->OnCommand
			( *this, pszCmd, nParam, nCode, nPriority, fOverwritable ) )
		{
			return	true ;
		}
	}
	return	NotifyCommand( pszCmd, nParam, nCode, nPriority, fOverwritable ) ;
}

// コマンド処理
//////////////////////////////////////////////////////////////////////////////
bool SGLSprite::DispatchCommand
	( const wchar_t * pszCmd, int64_t nParam, int64_t nCode )
{
	SGLSprite *	pFocus = m_refKeyFocus.GetReference() ;
	if ( pFocus != NULL )
	{
		return	pFocus->DispatchCommand( pszCmd, nParam, nCode ) ;
	}
	return	false ;
}

// スレッド排他処理用
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLSprite::Lock( int64_t msecTimeout ) const
{
	return	m_pMutexUI->Lock( msecTimeout ) ;
}

SSystem::SError SGLSprite::LockTrace
	( const char * pszSource, size_t nLineNum, int64_t msecTimeout ) const
{
#if	defined(__DEBUG__)
	return	m_pMutexUI->LockTrace( pszSource, nLineNum, msecTimeout ) ;
#else
	return	m_pMutexUI->Lock( msecTimeout ) ;
#endif
}

SSystem::SError SGLSprite::Unlock( void ) const
{
	m_pMutexUI->Unlock() ;
	return	errSuccess ;
}

atomic_int_t SGLSprite::UnlockAll( void ) const
{
	return	m_pMutexUI->UnlockAll() ;
}

SSystem::SError SGLSprite::Relock( atomic_int_t nLock ) const
{
	return	m_pMutexUI->Relock( nLock ) ;
}

atomic_int_t SGLSprite::TestLocked( void ) const
{
	return	m_pMutexUI->TestLocked() ;
}

// スレッド排他オブジェクト設定
//////////////////////////////////////////////////////////////////////////////
void SGLSprite::SetUIThreadMutex( SSystem::SSharableMutex * pMutex )
{
	ESLAssert( pMutex != NULL ) ;
	m_pMutexUI = pMutex ;
}

// Rosetta 用クラス
//////////////////////////////////////////////////////////////////////////////
const wchar_t * SGLSprite::GetRSClassName( void ) const
{
	return	L"Sprite" ;
}

// Loquaty 用クラス
//////////////////////////////////////////////////////////////////////////////
const wchar_t * SGLSprite::GetLQClassName( void ) const
{
	return	L"EntisGLS4.Sprite" ;
}

// 複製
//////////////////////////////////////////////////////////////////////////////
SGLObject * SGLSprite::DuplicateObject( void )
{
	return	new SGLSprite( *this ) ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSprite::OnSave( SSystem::SFileInterface& file )
{
	uint32_t	flagVisible = 0 ;
	if ( m_visible )
	{
		flagVisible |= 0x01 ;
	}
	m_paramView.SaveWithoutPointer( file ) ;
	if ( m_paramView.countVertex > 0 )
	{
		file.Write( m_paramView.pVertices,
					m_paramView.countVertex * sizeof(S2DVector) ) ;
	}
	//
	//
	file.Write( &m_flagsUI, sizeof(uint64_t) ) ;
	file.Write( &m_rectClickable, sizeof(SGLImageRect) ) ;
	file.Write( &flagVisible, sizeof(uint32_t) ) ;
	file.Write( &m_priority, sizeof(int32_t) ) ;
	file.WriteString( m_strID ) ;
	//
	SaveReferenceArray<SGLSpriteFilter>( file, m_filters ) ;
	SaveReferenceArray<SGLSpriteTimer>( file, m_timers ) ;
	SaveObjectArray<Action>( file, m_actions ) ;
	//
	int32_t	iDrawer =
		(int32_t) m_filters.FindPtr
			( ESLTypeCast<SGLSpriteFilter>( m_pDrawer.Ptr() ) ) ;
	file.Write( &iDrawer, sizeof(int32_t) ) ;
	if ( iDrawer < 0 )
	{
		SGLObject::SaveObject( m_pDrawer, file ) ;
	}
	//
	m_paramAction.SaveWithoutPointer( file ) ;
	//
	SGLObject::SaveObject( m_pImager, file ) ;
	//
	uint32_t	nFlags ;
	if ( m_pBuffer != NULL )
	{
		nFlags = 1 ;
		file.Write( &nFlags, sizeof(uint32_t) ) ;
		SGLObject::SaveObject( m_pBuffer, file ) ;
	}
	else
	{
		nFlags = 0 ;
		file.Write( &nFlags, sizeof(uint32_t) ) ;
	}
	if ( m_pVirtual3D != NULL )
	{
		nFlags = 1 ;
		file.Write( &nFlags, sizeof(uint32_t) ) ;
		file.Write( m_pVirtual3D.Ptr(), sizeof(Virtual3DParam) ) ;
	}
	else
	{
		nFlags = 0 ;
		file.Write( &nFlags, sizeof(uint32_t) ) ;
	}
	SaveReferenceArray<SGLSprite>( file, m_children ) ;
	return	sglErrSuccess ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSprite::OnRestore( SSystem::SFileInterface& file )
{
	Parameter		paramView, paramAction ;
	uint32_t		flagVisible = 0 ;
	uint64_t		flagsUI ;
	SGLImageRect	rectClickable ;
	int32_t			priority ;
	//
	paramView.LoadWithoutPointer( file ) ;
	if ( paramView.countVertex > 0 )
	{
		file.Read
			( m_paramVertex.GetArray(paramView.countVertex),
					paramView.countVertex * sizeof(S2DVector) ) ;
		m_paramVertex.FinishArray() ;
		paramView.pVertices = m_paramVertex.GetConstArray() ;
	}
	//
	file.Read( &flagsUI, sizeof(uint64_t) ) ;
	file.Read( &rectClickable, sizeof(SGLImageRect) ) ;
	file.Read( &flagVisible, sizeof(uint32_t) ) ;
	file.Read( &priority, sizeof(int32_t) ) ;
	file.ReadString( m_strID ) ;
	//
	LoadReferenceArray<SGLSpriteFilter>( file, m_filters ) ;
	LoadReferenceArray<SGLSpriteTimer>( file, m_timers ) ;
	LoadObjectArray<Action>( file, m_actions ) ;
	//
	int32_t	iDrawer = -1 ;
	file.Read( &iDrawer, sizeof(int32_t) ) ;
	if ( iDrawer < 0 )
	{
		m_pDrawer = SGLSmartCast<SGLSpriteDrawer>
						( SGLObject::LoadObject( file ) ) ;
	}
	else
	{
		ESLAssert( (size_t) iDrawer < m_filters.GetLength() ) ;
		SGLSpriteFilter *
			pFilter = m_filters.DetachAt( (size_t) iDrawer ) ;
		m_pDrawer = pFilter ;
		m_filters.InsertAt( (size_t) iDrawer, pFilter ) ;
	}
	//
	paramAction.LoadWithoutPointer( file ) ;
	if ( paramAction.countVertex > 0 )
	{
		paramAction.pVertices = m_paramVertex.GetConstArray() ;
	}
	//
	m_pImager = SGLSmartCast<Imager>
					( SGLObject::LoadObject( file ) ) ;
	if ( m_pImager != NULL )
	{
		m_pImager->OnAttached( *this ) ;
	}
	//
	uint32_t	nFlags ;
	if ( file.Read( &nFlags, sizeof(uint32_t) ) < sizeof(uint32_t) )
	{
		return	sglErrFailed ;
	}
	m_pBuffer = NULL ;
	if ( nFlags != 0 )
	{
		m_pBuffer = SGLSmartCast<Buffer>
						( SGLObject::LoadObject( file ) ) ;
		if ( m_pBuffer != NULL )
		{
			m_refImage = m_pBuffer->GetImage() ;
			m_refLeftImage = NULL ;
			if ( m_pBuffer->IsStereo3D() )
			{
				m_refLeftImage = m_pBuffer->GetLeftImage() ;
			}
		}
	}
	m_statusUpdate = updateFull ;
	//
	m_paramView = paramView ;
	m_paramAction = paramAction ;
	m_visible = ((flagVisible & 0x01) != 0) ;
	m_flagsUI = flagsUI ;
	m_rectClickable = rectClickable ;
	m_priority = priority ;
	//
	if ( file.Read( &nFlags, sizeof(uint32_t) ) < sizeof(uint32_t) )
	{
		return	sglErrFailed ;
	}
	m_pVirtual3D = NULL ;
	if ( nFlags != 0 )
	{
		m_pVirtual3D = new Virtual3DParam ;
		file.Read( m_pVirtual3D.Ptr(), sizeof(Virtual3DParam) ) ;
	}
	//
	LoadReferenceArray<SGLSprite>( file, m_children ) ;
	//
	size_t	countChildren = m_children.GetLength() ;
	for ( size_t i = 0; i < countChildren; i ++ )
	{
		SGLSprite *	pChild = m_children.GetAt( i ) ;
		if ( pChild != NULL )
		{
			pChild->m_refParent.SetReference( this ) ;
		}
	}
	return	sglErrSuccess ;
}

// 復元後処理
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSprite::OnAfterRestore( void )
{
	CommitReferenceArrayAfterRestore<SGLSpriteFilter>( m_filters ) ;
	CommitReferenceArrayAfterRestore<SGLSpriteTimer>( m_timers ) ;
	CommitObjectArrayAfterRestore<Action>( m_actions ) ;
	//
	int32_t	iDrawer =
		(int32_t) m_filters.FindPtr
			( ESLTypeCast<SGLSpriteFilter>( m_pDrawer.Ptr() ) ) ;
	if ( (iDrawer < 0) && (m_pDrawer != NULL) )
	{
		m_pDrawer->OnAfterRestore() ;
		m_pDrawer->OnAttachedSprite( this ) ;
	}
	if ( m_pImager != NULL )
	{
		m_pImager->OnAfterRestore() ;
	}
	CommitReferenceArrayAfterRestore<SGLSprite>( m_children ) ;
	//
	return	sglErrSuccess ;
}
