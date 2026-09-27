
/*****************************************************************************
                          Entis Graphic Library
 -----------------------------------------------------------------------------
     Copyright (c) 2002-2010 Leshade Entis, Entis-soft. Al rights reserved.
 *****************************************************************************/


#if	!defined(__SPRITE_H__)
#define	__SPRITE_H__

#if	!defined(COMPACT_NOA_DECODER)

//////////////////////////////////////////////////////////////////////////////
// スプライト抽象クラス
//////////////////////////////////////////////////////////////////////////////

class	ESprite	: public	ESLObject
{
protected:
	int				m_priority ;		// 表示優先度
	ESprite *		m_parent ;			// 親スプライト
	bool			m_visible ;			// 表示フラグ
	bool			m_f3d ;				// 3D スプライトフラグ

public:
	enum	StereoViewIndex
	{
		sviStereoViewRight,
		sviStereoViewLeft,
	} ;
	struct	VIEW3D_INFO
	{
		double	zFocus ;
		double	xParallax ;
		double	zOffset ;
		int		nViewIndex ;
	} ;

public:
	// 構築関数
	ESprite( void )
		: m_priority( 0 ), m_parent( NULL ),
			m_visible( false ), m_f3d( false ) { }
	// 消滅関数
	virtual ~ESprite( void ) { }
	// クラス情報
	DECLARE_CLASS_INFO( ESprite, ESLObject )

	// 表示状態取得
	virtual bool IsVisible( void ) ;
	// 表示状態を取得（副作用なし）
	virtual bool IsVisibleMT( void ) const ;
	// 表示状態設定
	virtual void SetVisible( bool fVisible ) ;
	// 外接(最小)矩形取得
	virtual EGL_RECT GetRectangle( void ) = 0 ;
	virtual EGL_RECT GetRectangle3DView( const VIEW3D_INFO & v3dInfo ) ;
	// 陰になる内接（最大）矩形取得
	virtual bool GetHiddenRectangle( EGL_RECT & rect ) ;
	// スプライト描画
	virtual void Draw( HEGL_RENDER_POLYGON hRenderPoly ) ;
	// スプライト描画（立体視表示用）
	virtual void DrawTo3DView
		( HEGL_RENDER_POLYGON hRenderPoly, const VIEW3D_INFO & v3dInfo ) ;
	// マルチスレッド描画前準備処理
	virtual void BeforeMTDraw( void ) ;
	// スプライト描画（マルチスレッド）
	virtual void MTDraw( HEGL_RENDER_POLYGON hRenderPoly ) = 0 ;
	// スプライト描画（立体視表示用・マルチスレッド）
	virtual void MTDrawTo3DView
		( HEGL_RENDER_POLYGON hRenderPoly, const VIEW3D_INFO & v3dInfo ) ;
	// スプライト上の指定領域の更新通知
	virtual bool UpdateRect( EGL_RECT * pUpdateRect = NULL ) ;
	// 子スプライト領域を更新領域に設定
	virtual bool UpdateChild
		( ESprite * pChild, EGL_RECT * pUpdateRect = NULL ) ;
	// ローカル→大域座標変換
	virtual void LocalRectToGlocal3DView
		( EGL_RECT & rect, const VIEW3D_INFO * pv3dInfo ) ;
	// 3D投影スクリーン座標取得
	virtual const E3D_VECTOR * GetScreen3DPosition( void ) const ;

	// 表示優先度変更
	void ChangePriority( int nPriority ) ;
	// 子スプライトのプライオリティ変更
	virtual void ChangedChildPriority( ESprite * pChild, int nPriority ) ;
	// 表示優先度取得
	int GetPriority( void ) const
		{
			return	m_priority ;
		}
	// 親スプライト取得
	ESprite * GetParent( void ) const
		{
			return	m_parent ;
		}
	// 親スプライト設定
	void SetParentAs( ESprite * pSprite )
		{
			ESLAssert( pSprite != NULL ) ;
			pSprite->m_parent = this ;
		}
	void SetSpriteParent( ESprite * pParent )
		{
			m_parent = pParent ;
		}
	// 表示優先度設定
	void SetSpritePriority( int nPriority )
		{
			m_priority = nPriority ;
		}
	// アニメーションを進める
	virtual ESLError OnAdvanceAnimation( unsigned int nTime ) ;

public:
	// 3D スプライト（ｚソート）
	bool Is3DSprite( void ) const
		{
			return	m_f3d ;
		}
	// ｚ座標取得
	virtual REAL32 GetZPosition( void ) const ;
	// カメラを取得する
	virtual bool Get3DViewCamera
		( E3D_VECTOR & vViewPos,
			E3D_VECTOR & vTargetPos, double & rRevAngleZ ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
// 画像スプライト
//////////////////////////////////////////////////////////////////////////////

class	EImageSprite	: public ESprite, public EGLImage
{
public:
	struct	PARAMETER
	{
		DWORD			dwFlags ;			// 描画フラグ
		EGL_POINT		ptDstPos ;			// 出力基準座標
		EGL_POINT		ptRevCenter ;		// 回転中心座標
		REAL32			rHorzUnit ;			// ｘ軸拡大率
		REAL32			rVertUnit ;			// ｙ軸拡大率
		REAL32			rRevAngle ;			// 回転角度 [deg]
		REAL32			rCrossingAngle ;	// ｘｙ軸交差角 [deg]
		EGL_PALETTE		rgbDimColor ;		// α描画色
		EGL_PALETTE		rgbLightColor ;
		unsigned int	nTransparency ;		// 透明度
		REAL32			rZOrder ;			// ｚ値
		EGL_PALETTE		rgbColorParam1 ;	// 色パラメータ
		REAL32			rZScale ;			// z スケール値
	} ;
	enum	FlagExtension
	{
		flagZScale				= 0x8000,
		flagMultiThreading		= 0x4000,
		maskEGLFlags			= 0xFFFF0FFF,
	} ;

protected:
	PARAMETER		m_ispParam ;	// スプライトパラメータ
	EGL_RECT		m_rectExt ;		// 外接矩形
	EGL_RECT		m_rectHidden ;	// 陰面矩形
	EGL_DRAW_PARAM	m_eglParam ;	// 描画パラメータ
	EGL_IMAGE_AXES	m_eglAxes ;

public:
	// 構築関数
	EImageSprite( void ) ;
	// 消滅関数
	virtual ~EImageSprite( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO2( EImageSprite, ESprite, EGLImage )

protected:
	// スプライトパラメータから描画用パラメータセットアップ
	void SetParameterToDraw( void ) ;

public:
	// 画像バッファ関連付け
	virtual void AttachImage( PEGL_IMAGE_INFO pImage ) ;
	// 画像バッファの参照を設定
	virtual void SetImageView
			( PEGL_IMAGE_INFO pImage, PCEGL_RECT pViewRect = NULL ) ;
	// 画像バッファ作成
	virtual PEGL_IMAGE_INFO CreateImage
			( DWORD fdwFormat, DWORD dwWidth, DWORD dwHeight,
					DWORD dwBitsPerPixel, DWORD dwFlags = 0 ) ;
	// 画像バッファ複製
	virtual PEGL_IMAGE_INFO DuplicateImage
			( PCEGL_IMAGE_INFO pImage, DWORD dwFlags = 0 ) ;
	// 画像ファイルを読み込む
	virtual ESLError ReadImageFile( ESLFileObject & file ) ;

public:
	// パラメータ取得
	void GetParameter( PARAMETER & param ) const ;
	// パラメータ設定
	void SetParameter( const PARAMETER & param ) ;
	// パラメータ複製
	virtual void CopyParameters( const EImageSprite * pSrc ) ;

	// 表示状態を取得
	virtual bool IsVisible( void ) ;
	// 表示状態を取得（副作用なし）
	virtual bool IsVisibleMT( void ) const ;
	// 表示基準座標取得
	virtual EGL_POINT GetPosition( void ) ;
	// 表示基準座標設定
	virtual void MovePosition( EGL_POINT ptDstPos ) ;
	// 透明度取得
	unsigned int GetTransparency( void ) const ;
	// 透明度設定
	void SetTransparency( unsigned int nTransparency ) ;
	// ｚ座標取得
	virtual REAL32 GetZPosition( void ) const ;
	// ｚ座標設定
	virtual void SetZPosition( REAL32 zPos ) ;
	// ｚスケール取得
	virtual REAL32 GetZScale( void ) const ;
	// ｚスケール設定
	virtual void SetZScale( REAL32 zScale ) ;

public:
	// ローカル座標からグローバル座標に変換
	static EGL_POINT LocalToGlobal
		( EGL_POINT ptLocal, const EGL_DRAW_PARAM & eglParam ) ;
	static EGL_RECT LocalToGlobal
		( const EGL_RECT & rect , const EGL_DRAW_PARAM & eglParam  ) ;
	EGL_POINT LocalToGlobal( EGL_POINT ptLocal ) const
		{
			return	LocalToGlobal( ptLocal, m_eglParam ) ;
		}
	EGL_RECT LocalToGlobal( const EGL_RECT & rect ) const
		{
			return	LocalToGlobal( rect, m_eglParam ) ;
		}
	// グローバル座標からローカル座標に変換
	EGL_POINT GlobalToLocal( EGL_POINT ptGlobal ) const ;
	// 画像描画パラメータ取得
	void GetDrawParameter( EGL_DRAW_PARAM & dp ) const
		{
			dp = m_eglParam ;
		}
	void GetDrawParameter3DView
		( EGL_DRAW_PARAM & dp, const VIEW3D_INFO & v3dInfo ) const ;

public:
	// 外接矩形を取得
	virtual EGL_RECT GetRectangle( void ) ;
	virtual EGL_RECT GetRectangle3DView( const VIEW3D_INFO & v3dInfo ) ;
	// 陰になる内接（最大）矩形取得
	virtual bool GetHiddenRectangle( EGL_RECT & rect ) ;
	// スプライト描画
	virtual void Draw( HEGL_RENDER_POLYGON hRenderPoly ) ;
	// スプライト描画（立体視表示用）
	virtual void DrawTo3DView
		( HEGL_RENDER_POLYGON hRenderPoly, const VIEW3D_INFO & v3dInfo ) ;
	// マルチスレッド描画前準備処理
	virtual void BeforeMTDraw( void ) ;
	// スプライト描画（マルチスレッド）
	virtual void MTDraw( HEGL_RENDER_POLYGON hRenderPoly ) ;
	// スプライト描画（立体視表示用・マルチスレッド）
	virtual void MTDrawTo3DView
		( HEGL_RENDER_POLYGON hRenderPoly, const VIEW3D_INFO & v3dInfo ) ;
	// スプライト上の指定領域の更新通知
//	virtual bool UpdateRect( EGL_RECT * pUpdateRect = NULL ) ;
	// ローカル→大域座標変換
	virtual void LocalRectToGlocal3DView
		( EGL_RECT & rect, const VIEW3D_INFO * pv3dInfo ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
// フィルタスプライト
//////////////////////////////////////////////////////////////////////////////

class	EFilterSprite	: public	ESprite
{
protected:
	EGL_RECT	m_rectEffect ;
	BYTE		m_bytBlue[0x100] ;
	BYTE		m_bytGreen[0x100] ;
	BYTE		m_bytRed[0x100] ;
	BYTE		m_bytAlpha[0x100] ;

public:
	// 構築関数
	EFilterSprite( void ) ;
	// 消滅関数
	virtual ~EFilterSprite( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( EFilterSprite, ESprite )

public:
	// 矩形取得
	virtual EGL_RECT GetRectangle( void ) ;
	// スプライト描画
	virtual void MTDraw( HEGL_RENDER_POLYGON hRenderPoly ) ;

public:
	// 矩形設定
	void SetRectangle( const EGL_RECT & rect ) ;
	// 輝度を設定
	void SetBrightness( int nBrightness ) ;
	// チャネルごとの輝度を設定
	void SetColorTone( int nRed, int nGreen, int nBlue, int nAlpha ) ;
	// 反転フィルタを設定
	void SetInversionTone( int nRed, int nGreen, int nBlue, int nAlpha ) ;
	// ライトフィルタを設定
	void SetLightTone( int nRed, int nGreen, int nBlue, int nAlpha ) ;
	// 複雑なフィルタを設定
	void SetGeneralTone
		( int nRedTone, int nRedFlag, int nGreenTone, int nGreenFlag,
			int nBlueTone, int nBlueFlag, int nAlphaTone, int nAlphaFlag ) ;
	// トーンテーブルを複製
	void SetToneTables
		( const void * pRed, const void * pGreen,
			const void * pBlue, const void * pAlpha ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
// 形状スプライト
//////////////////////////////////////////////////////////////////////////////

class	EShapeSprite	: public	ESprite
{
protected:
	EGLPalette		m_rgbaColor ;
	unsigned int	m_nTransparency ;
	DWORD			m_dwFlags ;

public:
	// 構築関数
	EShapeSprite( void )
		: m_rgbaColor((DWORD)0xFF000000), m_nTransparency(0),
			m_dwFlags(EGL_DRAW_BLEND_ALPHA) { }
	// 消滅関数
	virtual ~EShapeSprite( void ) { }
	// クラス情報
	DECLARE_CLASS_INFO( EShapeSprite, ESprite )

public:
	// 表示状態取得
	virtual bool IsVisible( void ) ;
	// 描画色取得
	EGL_PALETTE GetColor( void ) const
		{
			return	(EGL_PALETTE) m_rgbaColor ;
		}
	// 透明度取得
	unsigned int GetTransparency( void ) const
		{
			return	m_nTransparency ;
		}
	// 描画フラグ取得
	DWORD GetDrawFlag( void ) const
		{
			return	m_dwFlags ;
		}
	// 描画色設定
	void SetColor( EGL_PALETTE rgbaColor ) ;
	// 透明度設定
	void SetTransparency( unsigned int nTransparency ) ;
	// 描画フラグ設定
	void SetDrawFlag( DWORD dwFlags ) ;

} ;

class	ELineSprite	: public	EShapeSprite
{
protected:
	EGLRect					m_rectExt ;
	EObjArray<EGL_POINT>	m_lines ;

public:
	// 構築関数
	ELineSprite( void )
		: m_rectExt(0,0,-1,-1) { }
	// 消滅関数
	virtual ~ELineSprite( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( ELineSprite, EShapeSprite )

public:
	// 外接矩形取得
	virtual EGL_RECT GetRectangle( void ) ;
	// スプライト描画
	virtual void MTDraw( HEGL_RENDER_POLYGON hRenderPoly ) ;

public:
	// 直線を設定
	void SetLines( const EGL_POINT * pLines, unsigned int nCount ) ;

} ;

class	ERectangleSprite	: public	EShapeSprite
{
protected:
	EGLRect		m_rectFill ;

public:
	// 構築関数
	ERectangleSprite( void )
		: m_rectFill(0,0,0,0) { }
	// 消滅関数
	virtual ~ERectangleSprite( void ) { }
	// クラス情報
	DECLARE_CLASS_INFO( ERectangleSprite, EShapeSprite )

public:
	// 外接矩形取得
	virtual EGL_RECT GetRectangle( void ) ;
	// スプライト描画
	virtual void MTDraw( HEGL_RENDER_POLYGON hRenderPoly ) ;

public:
	// 矩形を設定
	void SetRectangle( const EGL_RECT & rectFill ) ;

} ;

class	EEllipseSprite	: public	EShapeSprite
{
protected:
	EGLPoint	m_ptCenter ;
	EGLSize		m_sizeRadius ;

public:
	// 構築関数
	EEllipseSprite( void )
		: m_ptCenter(0,0), m_sizeRadius(0,0) { }
	// 消滅関数
	virtual ~EEllipseSprite( void ) { }
	// クラス情報
	DECLARE_CLASS_INFO( EEllipseSprite, EShapeSprite )

public:
	// 外接矩形取得
	virtual EGL_RECT GetRectangle( void ) ;
	// スプライト描画
	virtual void MTDraw( HEGL_RENDER_POLYGON hRenderPoly ) ;

public:
	// 中心点を取得
	EGL_POINT GetPosition( void ) const
		{
			return	m_ptCenter ;
		}
	// 半径を取得
	EGL_SIZE GetRadiusSize( void ) const
		{
			return	m_sizeRadius ;
		}
	// 中心点を設定
	void MovePosition( EGL_POINT ptCenter ) ;
	// 半径を設定
	void SetRadiusSize( EGL_SIZE sizeRaidus ) ;

} ;

class	EPolygonSprite	: public	EShapeSprite
{
protected:
	EGLRect			m_rectExt ;
	unsigned int	m_nVertexes ;
	EGL_POINT *		m_pPolygon ;

public:
	// 構築関数
	EPolygonSprite( void )
		: m_rectExt(0,0,-1,-1), m_nVertexes(0), m_pPolygon(NULL) { }
	// 消滅関数
	virtual ~EPolygonSprite( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( EPolygonSprite, EShapeSprite )

public:
	// 外接矩形取得
	virtual EGL_RECT GetRectangle( void ) ;
	// スプライト描画
	virtual void MTDraw( HEGL_RENDER_POLYGON hRenderPoly ) ;

public:
	// 多角形を設定
	void SetPolygon( const EGL_POINT * pPolygon, unsigned int nCount ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
// 3D 表示スプライト
//////////////////////////////////////////////////////////////////////////////

class	E3DRenderSprite	: public ESprite, public E3DRenderPolygon
{
public:
	// 構築関数
	E3DRenderSprite( void ) ;
	// 消滅関数
	virtual ~E3DRenderSprite( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO2( E3DRenderSprite, ESprite, E3DRenderPolygon )

protected:
	EGL_RECT	m_rectExt ;

public:
	// 外接矩形取得
	virtual EGL_RECT GetRectangle( void ) ;
	// スプライト描画
	virtual void MTDraw( HEGL_RENDER_POLYGON hRenderPoly ) ;

public:
	// レンダリング準備
	virtual ESLError PrepareRendering( void ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
// スプライトサーバー
//////////////////////////////////////////////////////////////////////////////

#include <sakura/sakura.h>

class	ESpriteServer	: public EImageSprite
{
protected:
	EGLImage				m_imgZBuf ;			// ｚバッファ
	E3D_VECTOR				m_vScreen ;			// 投影スクリーン位置
	//
	bool					m_fStereoView ;		// 立体視ステレオモード
	EGLImage				m_imgLeftBuf ;		// 立体視用左視点バッファ
	VIEW3D_INFO				m_v3dInfo ;			// 立体視用パラメータ
	//
	HEGL_RENDER_POLYGON		m_hRenderPoly ;
	DWORD					m_dwRenderFlags ;
	DWORD					m_dwDrawFlags ;
	EIntTagArray<ESprite>	m_itaSprite ;
	EPtrObjArray<ESprite>	m_lstViewSprite ;
	//
	enum	UpdateStatus
	{
		usEmpty, usMulti, usFull
	} ;
	UpdateStatus			m_usStatus ;
	EObjArray<EGL_RECT>		m_listUpdateRect ;
	//
	enum
	{
		HideTestLimit = 16
	} ;
	EObjArray<EGL_RECT>		m_listHideRect ;
	//
	bool					m_fFillBack ;
	bool					m_fEnableDynamicMode ;
	EGL_PALETTE				m_rgbBackColor ;
	EGL_RECT				m_rectDynamic ;

	bool					m_fCamera ;
	E3D_VECTOR				m_vCameraPos ;
	E3D_VECTOR				m_vTargetPos ;
	double					m_zRevAngle ;

	enum	Values
	{
		countMaxParallelThreads	= 8,
	} ;
	HEGL_RENDER_POLYGON		m_hParallelRender[countMaxParallelThreads] ;

public:
	// 構築関数
	ESpriteServer( void ) ;
	// 消滅関数
	virtual ~ESpriteServer( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( ESpriteServer, EImageSprite )

public:
	// 画像バッファ作成
	virtual PEGL_IMAGE_INFO CreateImage
			( DWORD fdwFormat, DWORD dwWidth, DWORD dwHeight,
					DWORD dwBitsPerPixel, DWORD dwFlags = 0 ) ;
	// 画像バッファ消去
	virtual void DeleteImage( void ) ;

public:
	// 総スプライト数を取得
	unsigned int GetSpriteCount( void ) const ;
	// スプライトを取得
	ESprite * GetSpriteAt( unsigned int index ) ;
	// 指定スプライトの指標を取得する
	int GetSpriteIndex( ESprite * pSprite ) ;
	// スプライトを追加
	virtual void AddSprite( int nPriority, ESprite * pSprite ) ;

public:
	// スプライトを分離
	virtual ESLError DetachSprite( ESprite * pSprite ) ;
	// 全てのスプライトを分離
	virtual void DetachAllSprite( void ) ;
	// スプライトを削除
	virtual ESLError RemoveSprite( ESprite * pSprite ) ;
	// 全てのスプライトを削除
	virtual void RemoveAllSprite( void ) ;

public:
	// 外接矩形を取得
	virtual EGL_RECT GetRectangle( void ) ;
	// 陰になる内接（最大）矩形取得
	virtual bool GetHiddenRectangle( EGL_RECT & rect ) ;
	// マルチスレッド描画前準備処理
	virtual void BeforeMTDraw( void ) ;
	// スプライト描画
	virtual void MTDraw( HEGL_RENDER_POLYGON hRenderPoly ) ;
	// スプライト描画（立体視表示用）
	virtual void MTDrawTo3DView
		( HEGL_RENDER_POLYGON hRenderPoly, const VIEW3D_INFO & v3dInfo ) ;
protected:
	// スプライト描画（動的モード）
	void DrawDynamicMode
		( HEGL_RENDER_POLYGON hRenderPoly, const VIEW3D_INFO * pv3dInfo ) ;
public:
	// スプライト上の指定領域の更新通知
	virtual bool UpdateRect( EGL_RECT * pUpdateRect = NULL ) ;
	// 子スプライト領域を更新領域に設定
	virtual bool UpdateChild
		( ESprite * pChild, EGL_RECT * pUpdateRect = NULL ) ;
	// 更新領域を再描画
	virtual void Refresh( void ) ;
	// HEGL_RENDER_POLYGON 生成
	HEGL_RENDER_POLYGON GetRenderPolygon( void ) ;
protected:
	// 領域再描画
	virtual void RefreshRect( const EGL_RECT & rectRefresh ) ;
	virtual void RefreshRectWith3DView
		( PEGL_IMAGE_INFO pImage,
			const EGL_RECT & rectRefresh, const VIEW3D_INFO * pv3dInfo ) ;
	virtual void RefreshRectWith3DViewST
		( HEGL_RENDER_POLYGON hRender, PEGL_IMAGE_INFO pImage,
			const EGL_RECT & rectRefresh,
			const VIEW3D_INFO * pv3dInfo, bool flagOnMultiThread ) ;
	virtual void AfterRefreshRectWith3DView
		( HEGL_RENDER_POLYGON hRender, const VIEW3D_INFO * pv3dInfo ) ;
	// マルチスレッド描画準備処理
	class	BeforeMTDrawProc	: public SSystem::SParallelProcedure
	{
	protected:
		ESpriteServer *	m_pServer ;
		size_t			m_iNext ;
	public:
		struct	PARAM
		{
			ESprite *	pChild ;
		} ;
		// 構築関数
		BeforeMTDrawProc( ESpriteServer * pServer )
				: m_pServer( pServer ), m_iNext( 0 ) { }
		// ループ処理／終了判定関数
		virtual bool Continue( void * pInstance ) ;
		// 並列処理関数
		virtual void RunParallel( void * pInstance ) ;
	} ;
	// マルチスレッド描画準備処理
	class	MTDrawProc	: public SSystem::SParallelProcedure
	{
	protected:
		ESpriteServer *		m_pServer ;
		PEGL_IMAGE_INFO		m_pImage ;
		const VIEW3D_INFO *	m_pv3dInfo ;
		EGL_RECT			m_rectTarget ;
		int					m_yNext ;
		int					m_hBlock ;
	public:
		struct	PARAM
		{
			HEGL_RENDER_POLYGON	hRender ;
			EGL_RECT			rectBlock ;
		} ;
		// 構築関数
		MTDrawProc
			( ESpriteServer * pServer, PEGL_IMAGE_INFO pImage,
					const EGL_RECT& rectTarget,
					const VIEW3D_INFO *	pv3dInfo, int hBlock )
				: m_pServer(pServer), m_pImage(pImage),
					m_pv3dInfo(pv3dInfo),
					m_rectTarget(rectTarget),
					m_yNext(rectTarget.top), m_hBlock(hBlock) { }
		// ループ処理／終了判定関数
		virtual bool Continue( void * pInstance ) ;
		// 並列処理関数
		virtual void RunParallel( void * pInstance ) ;
	} ;
	
public:
	// 更新領域を削除
	void FlushUpdatedRect( void ) ;
	// 子スプライトのプライオリティ変更
	virtual void ChangedChildPriority( ESprite * pChild, int nPriority ) ;
	// アニメーションを進める
	virtual ESLError OnAdvanceAnimation( unsigned int nTime ) ;

public:
	// 動的スプライトモード設定
	bool EnableDynamicMode( bool fDynamicMode ) ;
	// 動的スプライトモード化取得
	virtual bool IsDynamicSpriteMode( void ) ;
	// 背景色取得
	EGL_PALETTE GetBackColor( void ) const ;
	// 背景色設定
	void SetBackColor( EGL_PALETTE rgbBack, bool fEnableBack ) ;
	// 背景色は有効か？
	bool IsEnabledFillBack( void ) const ;
	// 背景色を有効にする
	void EnableFillBack( bool fEnableBack ) ;

public:
	// 3D投影スクリーン座標取得
	virtual const E3D_VECTOR * GetScreen3DPosition( void ) const ;
	const E3D_VECTOR & GetScreenPosition( void ) const ;
	// スクリーン座標を設定
	void SetScreenPosition( const E3D_VECTOR & vScreen ) ;
	// 描画機能フラグを取得する
	DWORD GetDrawFunctionFlags( void ) const ;
	DWORD GetRenderFunctionFlags( void ) const ;
	// 描画機能フラグを設定する
	void SetDrawFunctionFlags( DWORD dwFlags ) ;
	void SetRenderFunctionFlags( DWORD dwFlags ) ;
	// Z バッファを作成
	void CreateZBuffer( void ) ;
	// Z バッファを削除
	void DeleteZBuffer( void ) ;
	// Z バッファを取得
	EGLImage & GetZBuffer( void ) ;
	// ステレオモードが有効か？
	bool IsEnabledStereoView( void ) const
		{
			return	m_fStereoView ;
		}
	// ステレオバッファ（左目視点）を作成
	ESLError CreateStereoBuffer( const VIEW3D_INFO & v3dInfo ) ;
	// ステレオ立体視パラメータを設定
	void SetStereoViewInfo( const VIEW3D_INFO & v3dInfo ) ;
	// ステレオ立体視パラメータを取得
	void GetStereoViewInfo( VIEW3D_INFO & v3dInfo ) ;
	// ステレオバッファを削除
	void DeleteStereoBuffer( void ) ;
	// ステレオバッファ（左目視点）を取得
	EGLImage & GetStereoLeftBuffer( void ) ;

public:
	// カメラ（3Dスプライト用）を設定する
	void Set3DViewCamera
		( const E3D_VECTOR vViewPos,
			const E3D_VECTOR & vTargetPos, double rRevAngleZ = 0.0 ) ;
	// カメラを有効・無効化する
	void Enable3DViewCamera( bool fCamera ) ;
	// カメラを取得する
	virtual bool Get3DViewCamera
		( E3D_VECTOR & vViewPos,
			E3D_VECTOR & vTargetPos, double & rRevAngleZ ) ;

} ;


#endif	//	!defined(COMPACT_NOA_DECODER)

#endif

