
/*****************************************************************************
           ステレオ 3D ディスプレイ用インターフェース実装
 *****************************************************************************/

#if	!defined(__STEREO_VIEW_INTERFACE__)
#define	__STEREO_VIEW_INTERFACE__	1


//////////////////////////////////////////////////////////////////////////////
// ステレオ表示インターフェース
//////////////////////////////////////////////////////////////////////////////

class	E3DStereoDisplayInterface
			: public E3DSDisplayPlugin::I3DImageView
{
public:
	// 構築関数
	E3DStereoDisplayInterface( void ) ;
	// 消滅関数
	virtual ~E3DStereoDisplayInterface( void ) ;

protected:
	HWND	m_hWnd ;					// AttachWindow
	bool	m_fUseViewPosition ;		// SetViewPosition
	int		m_xDst, m_yDst ;
	int		m_nWidth, m_nHeight ;

public:
	// オーバーライド関数
	virtual long int Behavior( void ) = 0 ;
	virtual long int ImageBufferCapacity( void ) = 0 ;
	virtual long int BestImageBufferCapacity( void ) = 0 ;
	virtual E3DSDisplayPlugin::Error AttachWindow( HWND hWnd ) ;
	virtual E3DSDisplayPlugin::Error DetachWindow( void ) ;
	virtual E3DSDisplayPlugin::Error AttachThread( void ) ;
	virtual E3DSDisplayPlugin::Error DetachThread( void ) ;
	virtual E3DSDisplayPlugin::WindowProcMethod WindowProc
		( HWND hWnd, UINT uMsg,
			WPARAM wParam, LPARAM lParam, LRESULT * pResult ) ;
	virtual E3DSDisplayPlugin::Error SetBufferSize
		( long int nFlags, long int nFormat,
					int nWidth, int nHeight, int nViewCount ) = 0 ;
	virtual E3DSDisplayPlugin::Error SetViewPosition
		( int xDst, int yDst, int nWidth, int nHeight ) ;
	virtual E3DSDisplayPlugin::DisplayModeMethod OnChangeDisplayMode
		( HMONITOR hMonitor,
			int nWidth, int nHeight, int nBitsPerPixel, int nFrequency ) ;
	virtual E3DSDisplayPlugin::DisplayModeMethod OnRestoreDisplayMode( void ) ;
	virtual E3DSDisplayPlugin::Error DrawBuffer
		( long int nFlags, int x, int y,
			const E3DSDisplayPlugin::ImageBuffer * bufImage[], int nViewCount ) = 0 ;
	virtual E3DSDisplayPlugin::Error PrepareView( void ) ;
	virtual E3DSDisplayPlugin::Error ViewImage( long int nFlags ) ;

protected:
	// スタブ
	static E3DStereoDisplayInterface *
			FromI3DImageView( E3DSDisplayPlugin::I3DImageView * piv ) ;
	static void stubRelease( E3DSDisplayPlugin::I3DImageView * piv ) ;
	static long int stubBehavior
				( E3DSDisplayPlugin::I3DImageView * piv ) ;
	static long int stubImageBufferCapacity
				( E3DSDisplayPlugin::I3DImageView * piv ) ;
	static long int stubBestImageBufferCapacity
				( E3DSDisplayPlugin::I3DImageView * piv ) ;
	static E3DSDisplayPlugin::Error stubAttachWindow
				( E3DSDisplayPlugin::I3DImageView * piv, HWND hWnd ) ;
	static E3DSDisplayPlugin::Error stubDetachWindow
				( E3DSDisplayPlugin::I3DImageView * piv ) ;
	static E3DSDisplayPlugin::Error stubAttachThread
				( E3DSDisplayPlugin::I3DImageView * piv ) ;
	static E3DSDisplayPlugin::Error stubDetachThread
				( E3DSDisplayPlugin::I3DImageView * piv ) ;
	static E3DSDisplayPlugin::WindowProcMethod stubWindowProc
		( E3DSDisplayPlugin::I3DImageView * piv, HWND hWnd, UINT uMsg,
			WPARAM wParam, LPARAM lParam, LRESULT * pResult ) ;
	static E3DSDisplayPlugin::Error stubSetBufferSize
		( E3DSDisplayPlugin::I3DImageView * piv,
				long int nFlags, long int nFormat,
					int nWidth, int nHeight, int nViewCount ) ;
	static E3DSDisplayPlugin::Error stubSetViewPosition
		( E3DSDisplayPlugin::I3DImageView * piv,
				int xDst, int yDst, int nWidth, int nHeight ) ;
	static E3DSDisplayPlugin::DisplayModeMethod stubOnChangeDisplayMode
		( E3DSDisplayPlugin::I3DImageView * piv, HMONITOR hMonitor,
			int nWidth, int nHeight, int nBitsPerPixel, int nFrequency ) ;
	static E3DSDisplayPlugin::DisplayModeMethod
			stubOnRestoreDisplayMode( E3DSDisplayPlugin::I3DImageView * piv ) ;
	static E3DSDisplayPlugin::Error stubDrawBuffer
		( E3DSDisplayPlugin::I3DImageView * piv,
			long int nFlags, int x, int y,
			const E3DSDisplayPlugin::ImageBuffer * bufImage[], int nViewCount ) ;
	static E3DSDisplayPlugin::Error
			stubPrepareView( E3DSDisplayPlugin::I3DImageView * piv ) ;
	static E3DSDisplayPlugin::Error
			stubViewImage
				( E3DSDisplayPlugin::I3DImageView * piv, long int nFlags ) ;

public:
	// 画像フォーマット変換
	bool ConvertFromE3DSDisplayImageBuffer
		( EGL_IMAGE_INFO & imginf,
			const E3DSDisplayPlugin::ImageBuffer & imgbuf ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
// アナグリフ表示インターフェース
//////////////////////////////////////////////////////////////////////////////

class	E3DStereoDisplayAnaglyphView
			: public E3DStereoDisplayInterface, public EGLDrawImage::INotify
{
public:
	enum	Mode
	{
		modeHighHue,
		modeLowHue,
		modeGray,
		modeMax,
	} ;

protected:
	EGLDrawImage *		m_pDrawImage ;
	struct IDirectDrawSurface *
						m_iddsufVRAM ;
	HEGL_DRAW_IMAGE		m_hDraw ;
	EGLImage			m_imgView ;
	EGLImage			m_imgBuf[E3DSDisplayPlugin::stereoBufferCount] ;
	bool				m_fUpdateBuffer ;
	EGLRect				m_rectUpdate ;
	Mode				m_mode ;

public:
	// 構築関数
	E3DStereoDisplayAnaglyphView
		( EGLDrawImage * pDrawImage = NULL, Mode mode = modeHighHue ) ;
	// 消滅関数
	virtual ~E3DStereoDisplayAnaglyphView( void ) ;
	// ビューの振る舞い
	virtual long int Behavior( void ) ;
	// 受け取り可能な画像バッファの種類
	virtual long int ImageBufferCapacity( void ) ;
	// 最適な画像バッファの種類
	virtual long int BestImageBufferCapacity( void ) ;
	// 画像バッファのサイズを設定する
	virtual E3DSDisplayPlugin::Error SetBufferSize
		( long int nFlags, long int nFormat,
					int nWidth, int nHeight, int nViewCount ) ;
	// 画像バッファに描画する
	virtual E3DSDisplayPlugin::Error DrawBuffer
		( long int nFlags, int x, int y,
			const E3DSDisplayPlugin::ImageBuffer * bufImage[], int nViewCount ) ;
	// 表示のための準備処理
	virtual E3DSDisplayPlugin::Error PrepareView( void ) ;
	// 表示処理
	virtual E3DSDisplayPlugin::Error ViewImage( long int nFlags ) ;

public:	// DirectDraw 通知
	// DirectDraw オブジェクトが削除される前に呼び出される
	virtual void OnReleaseDirectDraw( EGLDrawImage * pdi ) ;
	// DirectDraw オブジェクトが作成された後に呼び出される
	virtual void OnCreateDirectDraw( EGLDrawImage * pdi ) ;

public:
	// アナグリフモード設定
	void SetAnaglyphMode( Mode mode ) ;
	// 画像合成処理
	E3DSDisplayPlugin::Error MixStereoGraphic
		( PEGL_IMAGE_INFO pDstInf,
			PEGL_IMAGE_INFO pRightInf, PEGL_IMAGE_INFO pLeftInf ) ;
protected:
	static void MixStereoGraphicHighHue
		( void * pDst, void * pRight, void * pLeft,
						DWORD dwWidth, DWORD dwBytesPerPixel ) ;
	static void MixStereoGraphicLowHue
		( void * pDst, void * pRight, void * pLeft,
						DWORD dwWidth, DWORD dwBytesPerPixel ) ;
	static void MixStereoGraphicGray
		( void * pDst, void * pRight, void * pLeft,
						DWORD dwWidth, DWORD dwBytesPerPixel ) ;
	typedef	void (*MixStereoGraphicLineFunc)
		( void * pDst, void * pRight, void * pLeft,
						DWORD dwWidth, DWORD dwBytesPerPixel ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
// インターリーブ表示インターフェース
//////////////////////////////////////////////////////////////////////////////

class	E3DStereoDisplayInterleaved : public E3DStereoDisplayInterface
{
protected:
	EGLDrawImage *	m_pDrawImage ;
	HEGL_DRAW_IMAGE	m_hDraw ;
	EGLImage		m_imgView ;
	EGLImage		m_imgBuf[E3DSDisplayPlugin::stereoBufferCount] ;
	bool			m_fUpdateBuffer ;			// ソース画像更新
	EGLRect			m_rctUpdated ;				// ソース更新領域
	SDWORD			m_fxHorzScale ;				// 水平表示倍率（/0x10000）
	SDWORD			m_fxHorzPitch ;
	SDWORD			m_fxVertScale ;				// 垂直表示倍率（/0x10000）
	SDWORD			m_fxVertPitch ;
	bool			m_fCurrentEvenRight ;		// 現在の偶数サイド視点
	bool			m_fInterleavedVertically ;	// 行インターリーブ？
	bool			m_fInterleavedEventLeft ;	// 偶数サイド視点

public:
	// 構築関数
	E3DStereoDisplayInterleaved
		( EGLDrawImage * pDrawImage,
			bool fInterleavedVertically, bool fInterleavedEvenLeft ) ;
	// 消滅関数
	virtual ~E3DStereoDisplayInterleaved( void ) ;

public:
	// ビューの振る舞い
	virtual long int Behavior( void ) ;
	// 受け取り可能な画像バッファの種類
	virtual long int ImageBufferCapacity( void ) ;
	// 最適な画像バッファの種類
	virtual long int BestImageBufferCapacity( void ) ;
	// ウィンドウプロシージャ
	virtual E3DSDisplayPlugin::WindowProcMethod WindowProc
		( HWND hWnd, UINT uMsg,
			WPARAM wParam, LPARAM lParam, LRESULT * pResult ) ;
	// 画像バッファのサイズを設定する
	virtual E3DSDisplayPlugin::Error SetBufferSize
		( long int nFlags, long int nFormat,
					int nWidth, int nHeight, int nViewCount ) ;
	// 表示サイズ設定
	virtual E3DSDisplayPlugin::Error SetViewPosition
		( int xDst, int yDst, int nWidth, int nHeight ) ;
	// 画像バッファに描画する
	virtual E3DSDisplayPlugin::Error DrawBuffer
		( long int nFlags, int x, int y,
			const E3DSDisplayPlugin::ImageBuffer * bufImage[], int nViewCount ) ;
	// 表示のための準備処理
	virtual E3DSDisplayPlugin::Error PrepareView( void ) ;
	// 表示処理
	virtual E3DSDisplayPlugin::Error ViewImage( long int nFlags ) ;

private:
	// スケール更新
	void UpdateViewScale( void ) ;
	// 入力矩形を出力先矩形に変換
	void ViewRectFromSourceRect( EGL_RECT & rect ) const ;
	// 画像描画
	void DrawBothSideImage
		( const EGL_RECT rctDst[],
			PEGL_IMAGE_INFO pSrcImages[],
			const EGL_RECT * pSrcRect ) ;
	// 画像描画
	void DrawSideImage
		( const EGL_RECT & rectDst,
			PEGL_IMAGE_INFO pSrcImage,
			SDWORD fxSrcX, SDWORD fxSrcY, int nSide ) ;
	void DrawImagePixels
		( BYTE * pbytDst, DWORD dwDstPixelBytes,
			DWORD dwDstPixelPitch, DWORD dwDstLineBytes,
			PEGL_IMAGE_INFO pSrcImage,
			SDWORD fxSrcX, SDWORD fxSrcY,
			SDWORD fxSrcPitchX, SDWORD fxSrcPitchY,
			DWORD dwWidth, DWORD dwHeight ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
// DirectDraw stereoscopic 表示インターフェース
//////////////////////////////////////////////////////////////////////////////

class	E3DStereoDisplayDDStereoscopic
			: public E3DStereoDisplayInterface, public EGLDrawImage::INotify
{
protected:
	EDisplayMode					m_dmMode ;
	EGLDrawImage *					m_pDrawImage ;
	struct IDirectDrawSurface7 *	m_iddsBuf[E3DSDisplayPlugin::stereoBufferCount] ;
	bool							m_fDynamicDraw ;
	bool							m_fBuffer ;
	EGL_SIZE						m_sizeBuffer ;

public:
	// 構築関数
	E3DStereoDisplayDDStereoscopic( EGLDrawImage * pDrawImage ) ;
	// 消滅関数
	virtual ~E3DStereoDisplayDDStereoscopic( void ) ;

protected:
	// 所有しているサーフェスを解放する
	void ReleaseSurface( void ) ;
	// サーフェスを生成する
	E3DSDisplayPlugin::Error CreateSurface( int nWidth, int nHeight ) ;

public:
	// ビューの振る舞い
	virtual long int Behavior( void ) ;
	// 受け取り可能な画像バッファの種類
	virtual long int ImageBufferCapacity( void ) ;
	// 最適な画像バッファの種類
	virtual long int BestImageBufferCapacity( void ) ;
	// 画像バッファのサイズを設定する
	virtual E3DSDisplayPlugin::Error SetBufferSize
		( long int nFlags, long int nFormat,
					int nWidth, int nHeight, int nViewCount ) ;
	// 画面モード変更時の処理
	virtual E3DSDisplayPlugin::DisplayModeMethod OnChangeDisplayMode
		( HMONITOR hMonitor,
			int nWidth, int nHeight, int nBitsPerPixel, int nFrequency ) ;
	// 画面モード復帰時の処理
	virtual E3DSDisplayPlugin::DisplayModeMethod OnRestoreDisplayMode( void ) ;
	// 画像バッファに描画する
	virtual E3DSDisplayPlugin::Error DrawBuffer
		( long int nFlags, int x, int y,
			const E3DSDisplayPlugin::ImageBuffer * bufImage[], int nViewCount ) ;
	// 表示のための準備処理
	virtual E3DSDisplayPlugin::Error PrepareView( void ) ;
	// 表示処理
	virtual E3DSDisplayPlugin::Error ViewImage( long int nFlags ) ;

public:	// DirectDraw 通知
	// DirectDraw オブジェクトが削除される前に呼び出される
	virtual void OnReleaseDirectDraw( EGLDrawImage * pdi ) ;
	// DirectDraw オブジェクトが作成された後に呼び出される
	virtual void OnCreateDirectDraw( EGLDrawImage * pdi ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
// NVIDIA StereoBLT 表示インターフェース
//////////////////////////////////////////////////////////////////////////////

class	E3DStereoDisplayNVStereoBLT
			: public E3DStereoDisplayInterface, public EGLDrawImage::INotify
{
protected:
	EDisplayMode					m_dmMode ;
	EGLDrawImage *					m_pDrawImage ;
	struct IDirectDrawSurface7 *	m_iddsBuf ;
	struct IDirect3DDevice9 *		m_id3d9Dev ;
	struct IDirect3DSurface9 *		m_idds9Buf ;
	HEGL_DRAW_IMAGE					m_hDraw ;
	bool							m_fBuffer ;
	EGL_SIZE						m_sizeBuffer ;

	struct Nv_Stereo_Image_Header
	{
		DWORD	dwSignature ;
		DWORD	dwWidth ;
		DWORD	dwHeight ;
		DWORD	dwBPP ;
		DWORD	dwFlags ;
	} ;
	enum	Nv_Stereo_Signature
	{
		NVSTEREO_IMAGE_SIGNATURE = 0x4433564e,	// "NV3D"
	} ;
	enum	Nv_Stereo_Flags
	{
		SIH_SWAP_EYES		= 0x00000001,
		SIH_SCALE_TO_FIT	= 0x00000002,
	} ;

public:
	// 構築関数
	E3DStereoDisplayNVStereoBLT( EGLDrawImage * pDrawImage ) ;
	// 消滅関数
	virtual ~E3DStereoDisplayNVStereoBLT( void ) ;

protected:
	// 所有しているサーフェスを解放する
	void ReleaseSurface( void ) ;
	// サーフェスを生成する
	E3DSDisplayPlugin::Error CreateSurface( int nWidth, int nHeight ) ;

public:
	// ビューの振る舞い
	virtual long int Behavior( void ) ;
	// 受け取り可能な画像バッファの種類
	virtual long int ImageBufferCapacity( void ) ;
	// 最適な画像バッファの種類
	virtual long int BestImageBufferCapacity( void ) ;
	// 画像バッファのサイズを設定する
	virtual E3DSDisplayPlugin::Error SetBufferSize
		( long int nFlags, long int nFormat,
					int nWidth, int nHeight, int nViewCount ) ;
	// 画面モード変更時の処理
	virtual E3DSDisplayPlugin::DisplayModeMethod OnChangeDisplayMode
		( HMONITOR hMonitor,
			int nWidth, int nHeight, int nBitsPerPixel, int nFrequency ) ;
	// 画面モード復帰時の処理
	virtual E3DSDisplayPlugin::DisplayModeMethod OnRestoreDisplayMode( void ) ;
	// 画像バッファに描画する
	virtual E3DSDisplayPlugin::Error DrawBuffer
		( long int nFlags, int x, int y,
			const E3DSDisplayPlugin::ImageBuffer * bufImage[], int nViewCount ) ;
	// 表示のための準備処理
	virtual E3DSDisplayPlugin::Error PrepareView( void ) ;
	// 表示処理
	virtual E3DSDisplayPlugin::Error ViewImage( long int nFlags ) ;

public:	// DirectDraw 通知
	// DirectDraw オブジェクトが削除される前に呼び出される
	virtual void OnReleaseDirectDraw( EGLDrawImage * pdi ) ;
	// DirectDraw オブジェクトが作成された後に呼び出される
	virtual void OnCreateDirectDraw( EGLDrawImage * pdi ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
// OpenGL ステレオ表示インターフェース
//////////////////////////////////////////////////////////////////////////////

class	E3DStereoDisplayOpenGL : public E3DStereoDisplayInterface
{
protected:
	HEGL_DRAW_IMAGE	m_hDraw ;
	HDC				m_hDC ;
	HGLRC			m_hGLRC ;
	EGL_IMAGE_INFO	m_infBuf[E3DSDisplayPlugin::stereoBufferCount] ;
	unsigned int	m_uiTexture[E3DSDisplayPlugin::stereoBufferCount] ;
	bool			m_fDynamicDraw ;
	bool			m_fStretchDraw ;

public:
	// 構築関数
	E3DStereoDisplayOpenGL( void ) ;
	// 消滅関数
	virtual ~E3DStereoDisplayOpenGL( void ) ;
	// 画像バッファ生成
	void CreateImageBuffer( int nWidth, int nHeight ) ;
	// 画像バッファ削除
	void DeleteImageBuffer( void ) ;
	// OpenGL 初期化
	E3DSDisplayPlugin::Error CreateGLContext( void ) ;
	// OpenGL 解放
	void DeleteGLContext( void ) ;
	// OpenGL カレント設定
	E3DSDisplayPlugin::Error AttachGLCurrent( void ) ;
	// OpenGL カレント解除
	E3DSDisplayPlugin::Error DetachGLCurrent( void ) ;
	// カメラ設定
	void UpdateCamera( void ) ;
	// テクスチャ設定（伸縮時）
	ESLError UpdateTexture
		( int iSide, int nWidth, int nHeight, void * ptrImageArray ) ;
	// 四角形テクスチャ描画
	void DrawTexturedQuad( int iSide ) ;

public:
	// ビューの振る舞い
	virtual long int Behavior( void ) ;
	// 受け取り可能な画像バッファの種類
	virtual long int ImageBufferCapacity( void ) ;
	// 最適な画像バッファの種類
	virtual long int BestImageBufferCapacity( void ) ;
	// ウィンドウに関連付け
	virtual E3DSDisplayPlugin::Error AttachWindow( HWND hWnd ) ;
	// ウィンドウから分離
	virtual E3DSDisplayPlugin::Error DetachWindow( void ) ;
	// 画像バッファのサイズを設定する
	virtual E3DSDisplayPlugin::Error SetBufferSize
		( long int nFlags, long int nFormat,
					int nWidth, int nHeight, int nViewCount ) ;
	// 画面モード変更時の処理
	virtual E3DSDisplayPlugin::DisplayModeMethod OnChangeDisplayMode
		( HMONITOR hMonitor,
			int nWidth, int nHeight, int nBitsPerPixel, int nFrequency ) ;
	// 画面モード復帰時の処理
	virtual E3DSDisplayPlugin::DisplayModeMethod OnRestoreDisplayMode( void ) ;
	// 画像バッファに描画する
	virtual E3DSDisplayPlugin::Error DrawBuffer
		( long int nFlags, int x, int y,
			const E3DSDisplayPlugin::ImageBuffer * bufImage[], int nViewCount ) ;
	// 表示のための準備処理
	virtual E3DSDisplayPlugin::Error PrepareView( void ) ;
	// 表示処理
	virtual E3DSDisplayPlugin::Error ViewImage( long int nFlags ) ;

public:
	// OpenGL Quad Buffer サポートテスト
	static bool IsSupportedStereo( void ) ;

} ;


#endif
