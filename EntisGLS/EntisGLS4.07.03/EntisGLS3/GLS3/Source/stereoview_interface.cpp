
/*****************************************************************************
             Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
		Copyright (c) 2010 Leshade Entis. All rights reserved.
 ****************************************************************************/


#include <gls.h>
#include <ddraw.h>
#include <d3d9.h>

using namespace E3DSDisplayPlugin ;


// EGL_IMAGE_INFO 構造体から変換
//////////////////////////////////////////////////////////////////////////////
void E3DSDisplayPlugin::ImageBuffer::ConvertFrom
					( const EGL_IMAGE_INFO & imginf )
{
	memset( this, 0, sizeof(ImageBuffer) ) ;
	nFlags = E3DSDisplayPlugin::flagMemoryImage
					| E3DSDisplayPlugin::flagSourceRectangle ;
	//
	nWidth = imginf.dwImageWidth ;
	nHeight = imginf.dwImageHeight ;
	nBitsPerPixel = imginf.dwBitsPerPixel ;
	nBytesPerLine = imginf.dwBytesPerLine ;
	pBuffer = imginf.ptrImageArray ;
	//
	rctSource.x = 0 ;
	rctSource.y = 0 ;
	rctSource.w = imginf.dwImageWidth ;
	rctSource.h = imginf.dwImageHeight ;
	//
	hDC = ::eglGetDC( &imginf ) ;
	if ( hDC != NULL )
	{
		nFlags |= E3DSDisplayPlugin::flagDeviceContext ;
		//
		hBitmap = (HBITMAP) ::GetCurrentObject( hDC, OBJ_BITMAP ) ;
		if ( hBitmap != NULL )
		{
			nFlags |= E3DSDisplayPlugin::flagBitmapHandle ;
		}
	}
}


//////////////////////////////////////////////////////////////////////////////
// ステレオ表示インターフェース
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
E3DStereoDisplayInterface::E3DStereoDisplayInterface( void )
{
	pfnRelease = stubRelease ;
	pfnImageBufferCapacity = stubImageBufferCapacity ;
	pfnBestImageBufferCapacity = stubBestImageBufferCapacity ;
	pfnAttachWindow = stubAttachWindow ;
	pfnDetachWindow = stubDetachWindow ;
	pfnAttachThread = stubAttachThread ;
	pfnDetachThread = stubDetachThread ;
	pfnWindowProc = stubWindowProc ;
	pfnSetBufferSize = stubSetBufferSize ;
	pfnSetViewPosition = stubSetViewPosition ;
	pfnOnChangeDisplayMode = stubOnChangeDisplayMode ;
	pfnOnRestoreDisplayMode = stubOnRestoreDisplayMode ;
	pfnDrawBuffer = stubDrawBuffer ;
	pfnPrepareView = stubPrepareView ;
	pfnViewImage = stubViewImage ;
	//
	m_hWnd = NULL ;
	m_fUseViewPosition = false ;
	m_xDst = 0 ;
	m_yDst = 0 ;
	m_nWidth = 0 ;
	m_nHeight = 0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
E3DStereoDisplayInterface::~E3DStereoDisplayInterface( void )
{
}

// オーバーライド関数
//////////////////////////////////////////////////////////////////////////////
Error E3DStereoDisplayInterface::AttachWindow( HWND hWnd )
{
	m_hWnd = hWnd ;
	return	errSuccess ;
}

Error E3DStereoDisplayInterface::DetachWindow( void )
{
	m_hWnd = NULL ;
	return	errSuccess ;
}

Error E3DStereoDisplayInterface::AttachThread( void )
{
	return	errSuccess ;
}

Error E3DStereoDisplayInterface::DetachThread( void )
{
	return	errSuccess ;
}

WindowProcMethod E3DStereoDisplayInterface::WindowProc
	( HWND hWnd, UINT uMsg,
		WPARAM wParam, LPARAM lParam, LRESULT * pResult )
{
	return	methodContinue ;
}

E3DSDisplayPlugin::Error E3DStereoDisplayInterface::SetViewPosition
	( int xDst, int yDst, int nWidth, int nHeight )
{
	m_fUseViewPosition = true ;
	m_xDst = xDst ;
	m_yDst = yDst ;
	m_nWidth = nWidth ;
	m_nHeight = nHeight ;
	return	errSuccess ;
}

DisplayModeMethod E3DStereoDisplayInterface::OnChangeDisplayMode
	( HMONITOR hMonitor,
		int nWidth, int nHeight, int nBitsPerPixel, int nFrequency )
{
	return	methodNoModeChanged ;
}

DisplayModeMethod E3DStereoDisplayInterface::OnRestoreDisplayMode( void )
{
	return	methodNoModeChanged ;
}

Error E3DStereoDisplayInterface::PrepareView( void )
{
	return	errSuccess ;
}

Error E3DStereoDisplayInterface::ViewImage( long int nFlags )
{
	return	errSuccess ;
}

// インターフェーススタブ
//////////////////////////////////////////////////////////////////////////////
E3DStereoDisplayInterface *
	E3DStereoDisplayInterface::FromI3DImageView( I3DImageView * piv )
{
	return	(E3DStereoDisplayInterface*) piv ;
}

void E3DStereoDisplayInterface::stubRelease( I3DImageView * piv )
{
	E3DStereoDisplayInterface *	psdi = FromI3DImageView(piv) ;
	delete	psdi ;
}

long int E3DStereoDisplayInterface::stubBehavior
			( E3DSDisplayPlugin::I3DImageView * piv )
{
	return	FromI3DImageView(piv)->Behavior() ;
}

long int E3DStereoDisplayInterface::stubImageBufferCapacity( I3DImageView * piv )
{
	return	FromI3DImageView(piv)->ImageBufferCapacity() ;
}

long int E3DStereoDisplayInterface::stubBestImageBufferCapacity( I3DImageView * piv )
{
	return	FromI3DImageView(piv)->BestImageBufferCapacity() ;
}

Error E3DStereoDisplayInterface::stubAttachWindow( I3DImageView * piv, HWND hWnd )
{
	return	FromI3DImageView(piv)->AttachWindow( hWnd ) ;
}

Error E3DStereoDisplayInterface::stubDetachWindow( I3DImageView * piv )
{
	return	FromI3DImageView(piv)->DetachWindow() ;
}

Error E3DStereoDisplayInterface::stubAttachThread( I3DImageView * piv )
{
	return	FromI3DImageView(piv)->AttachThread() ;
}

Error E3DStereoDisplayInterface::stubDetachThread( I3DImageView * piv )
{
	return	FromI3DImageView(piv)->DetachThread() ;
}

WindowProcMethod E3DStereoDisplayInterface::stubWindowProc
	( I3DImageView * piv, HWND hWnd, UINT uMsg,
		WPARAM wParam, LPARAM lParam, LRESULT * pResult )
{
	return	FromI3DImageView(piv)->
				WindowProc( hWnd, uMsg, wParam, lParam, pResult ) ;
}

Error E3DStereoDisplayInterface::stubSetBufferSize
	( I3DImageView * piv,
			long int nFlags, long int nFormat,
				int nWidth, int nHeight, int nViewCount )
{
	return	FromI3DImageView(piv)->
				SetBufferSize( nFlags, nFormat, nWidth, nHeight, nViewCount ) ;
}

Error E3DStereoDisplayInterface::stubSetViewPosition
	( I3DImageView * piv, int xDst, int yDst, int nWidth, int nHeight )
{
	return	FromI3DImageView(piv)->
				SetViewPosition( xDst, yDst, nWidth, nHeight ) ;
}

DisplayModeMethod E3DStereoDisplayInterface::stubOnChangeDisplayMode
	( I3DImageView * piv, HMONITOR hMonitor,
		int nWidth, int nHeight, int nBitsPerPixel, int nFrequency )
{
	return	FromI3DImageView(piv)->OnChangeDisplayMode
				( hMonitor, nWidth, nHeight, nBitsPerPixel, nFrequency ) ;
}

DisplayModeMethod
	E3DStereoDisplayInterface::stubOnRestoreDisplayMode( I3DImageView * piv )
{
	return	FromI3DImageView(piv)->OnRestoreDisplayMode() ;
}

Error E3DStereoDisplayInterface::stubDrawBuffer
	( I3DImageView * piv,
		long int nFlags, int x, int y,
		const ImageBuffer * bufImage[], int nViewCount )
{
	return	FromI3DImageView(piv)->DrawBuffer
					( nFlags, x, y, bufImage, nViewCount ) ;
}

Error E3DStereoDisplayInterface::stubPrepareView( I3DImageView * piv )
{
	return	FromI3DImageView(piv)->PrepareView() ;
}

Error E3DStereoDisplayInterface::stubViewImage
				( I3DImageView * piv, long int nFlags )
{
	return	FromI3DImageView(piv)->ViewImage( nFlags ) ;
}

// 画像フォーマット変換
//////////////////////////////////////////////////////////////////////////////
bool E3DStereoDisplayInterface::ConvertFromE3DSDisplayImageBuffer
	( EGL_IMAGE_INFO & imginf, const ImageBuffer & imgbuf )
{
	if ( !(imgbuf.nFlags & flagMemoryImage)
		|| (imgbuf.pBuffer == NULL) )
	{
		return	false ;
	}
	memset( &imginf, 0, sizeof(EGL_IMAGE_INFO) ) ;
	imginf.dwInfoSize = sizeof(EGL_IMAGE_INFO) ;
	imginf.fdwFormatType = EIF_RGB_BITMAP ;
	imginf.ptrImageArray = imgbuf.pBuffer ;
	imginf.dwImageWidth = imgbuf.nWidth ;
	imginf.dwImageHeight = imgbuf.nHeight ;
	imginf.dwBitsPerPixel = imgbuf.nBitsPerPixel ;
	imginf.dwBytesPerLine = imgbuf.nBytesPerLine ;
	imginf.dwSizeOfImage = abs(imgbuf.nBytesPerLine) * imgbuf.nHeight ;
	//
	if ( imgbuf.nFlags & flagSourceRectangle )
	{
		EGL_IMAGE_INFO	infTemp = imginf ;
		EGL_IMAGE_RECT	rect ;
		rect.x = imgbuf.rctSource.x ;
		rect.y = imgbuf.rctSource.y ;
		rect.w = imgbuf.rctSource.w ;
		rect.h = imgbuf.rctSource.h ;
		//
		eglGetClippedImageInfo( &imginf, &infTemp, &rect ) ;
	}
	return	true ;
}


//////////////////////////////////////////////////////////////////////////////
// アナグリフ表示インターフェース
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
E3DStereoDisplayAnaglyphView::E3DStereoDisplayAnaglyphView
	( EGLDrawImage * pDrawImage, E3DStereoDisplayAnaglyphView::Mode mode )
{
	m_pDrawImage = pDrawImage ;
	m_iddsufVRAM = NULL ;
	m_hDraw = NULL ;
	m_fUpdateBuffer = false ;
	m_mode = mode ;
	//
	if ( m_pDrawImage != NULL )
	{
		m_pDrawImage->AddNotify( this ) ;
	}
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
E3DStereoDisplayAnaglyphView::~E3DStereoDisplayAnaglyphView( void )
{
	if ( m_pDrawImage != NULL )
	{
		m_pDrawImage->DetachNotify( this ) ;
	}
	if ( m_iddsufVRAM != NULL )
	{
		m_iddsufVRAM->Release() ;
	}
	if ( m_hDraw != NULL )
	{
		m_hDraw->Release() ;
	}
}

// ビュー振る舞い
//////////////////////////////////////////////////////////////////////////////
long int E3DStereoDisplayAnaglyphView::Behavior( void )
{
	return	0 ;
}

// 受け取り可能な画像バッファの種類
//////////////////////////////////////////////////////////////////////////////
long int E3DStereoDisplayAnaglyphView::ImageBufferCapacity( void )
{
	return	flagMemoryImage ;
}

// 最適な画像バッファの種類
//////////////////////////////////////////////////////////////////////////////
long int E3DStereoDisplayAnaglyphView::BestImageBufferCapacity( void )
{
	return	flagMemoryImage ;
}

// 画像バッファのサイズを設定する
//////////////////////////////////////////////////////////////////////////////
Error E3DStereoDisplayAnaglyphView::SetBufferSize
	( long int nFlags, long int nFormat,
				int nWidth, int nHeight, int nViewCount )
{
	m_imgBuf[0].CreateImage( EIF_RGB_BITMAP, nWidth, nHeight, 32 ) ;
	m_imgBuf[1].CreateImage( EIF_RGB_BITMAP, nWidth, nHeight, 32 ) ;
	m_imgView.CreateImage
		( EIF_RGB_BITMAP, nWidth, nHeight, 32, EGL_IMAGE_HAS_DC ) ;
	m_imgView.ReverseVertically() ;
	//
	if ( m_iddsufVRAM != NULL )
	{
		m_iddsufVRAM->Release() ;
		m_iddsufVRAM = NULL ;
	}
	if ( m_pDrawImage != NULL )
	{
		m_iddsufVRAM =
			m_pDrawImage->CreateSurfaceOnVRAM( nWidth, nHeight ) ;
	}
	//
	return	errSuccess ;
}

// 画像バッファに描画する
//////////////////////////////////////////////////////////////////////////////
Error E3DStereoDisplayAnaglyphView::DrawBuffer
	( long int nFlags, int x, int y,
		const ImageBuffer * bufImage[], int nViewCount )
{
	if ( (nViewCount <= 0) || (nViewCount > 2) )
	{
		return	errInvalidParameter ;
	}
	if ( nViewCount == 1 )
	{
		//
		// モノビュー画像表示
		//
		EGL_IMAGE_INFO	imginf ;
		if ( !ConvertFromE3DSDisplayImageBuffer
						( imginf, *(bufImage[0]) ) )
		{
			return	errInvalidParameter ;
		}
		if ( m_imgView.GetInfo() == NULL )
		{
			return	errDeviceNotReady ;
		}
		if ( m_hDraw == NULL )
		{
			m_hDraw = ::eglCreateDrawImage() ;
		}
		m_hDraw->Initialize( m_imgView, NULL, NULL ) ;
		//
		EGL_DRAW_PARAM	dp ;
		memset( &dp, 0, sizeof(dp) ) ;
		dp.ptBasePos.x = x ;
		dp.ptBasePos.y = y ;
		dp.pSrcImage = &imginf ;
		//
		if ( !m_hDraw->PrepareDraw( &dp ) )
		{
			m_hDraw->DrawImage() ;
		}
		return	errSuccess ;
	}
	//
	//画像バッファ取得
	//
	EGLRect	rctDraw ;
	rctDraw.left = x ;
	rctDraw.top = y ;
	rctDraw.right = x ;
	rctDraw.bottom = y ;
	//
	EGL_IMAGE_INFO	imginf[stereoBufferCount] ;
	int	i ;
	for ( i = 0; i < stereoBufferCount; i ++ )
	{
		if ( bufImage[i] == NULL )
		{
			return	errInvalidParameter ;
		}
		if ( !ConvertFromE3DSDisplayImageBuffer
						( imginf[i], *(bufImage[i]) ) )
		{
			return	errInvalidParameter ;
		}
		if ( x + (int) imginf[i].dwImageWidth - 1 > rctDraw.right )
		{
			rctDraw.right = x + imginf[i].dwImageWidth - 1 ;
		}
		if ( y + (int) imginf[i].dwImageHeight - 1 > rctDraw.bottom )
		{
			rctDraw.bottom = y + imginf[i].dwImageHeight - 1 ;
		}
	}
	//
	rctDraw &= EGLRect
		( 0, 0, m_imgView.GetWidth() - 1, m_imgView.GetHeight() - 1 ) ;
	//
	// 動的バッファ処理
	//
	if ( nFlags & drawDynamic )
	{
		if ( (imginf[0].dwImageWidth == imginf[1].dwImageWidth)
			&& (imginf[0].dwImageHeight == imginf[1].dwImageHeight) )
		{
			EGLImageRect	irDraw = rctDraw ;
			EGL_IMAGE_INFO	infDstBuf ;
			infDstBuf.dwInfoSize = sizeof(EGL_IMAGE_INFO) ;
			if ( ::eglGetClippedImageInfo
					( &infDstBuf, m_imgView, &irDraw ) )
			{
				return	errFailed ;
			}
			return	MixStereoGraphic
				( &infDstBuf, &(imginf[stereoRightBuffer]),
								&(imginf[stereoLeftBuffer]) ) ;
		}
	}
	//
	// 中間バッファへ書き込む
	//
	if ( m_hDraw == NULL )
	{
		m_hDraw = ::eglCreateDrawImage() ;
	}
	//
	for ( i = 0; i < stereoBufferCount; i ++ )
	{
		m_hDraw->Initialize( m_imgBuf[i], NULL, NULL ) ;
		//
		EGL_DRAW_PARAM	dp ;
		memset( &dp, 0, sizeof(dp) ) ;
		dp.ptBasePos.x = x ;
		dp.ptBasePos.y = y ;
		dp.pSrcImage = &(imginf[i]) ;
		//
		if ( !m_hDraw->PrepareDraw( &dp ) )
		{
			m_hDraw->DrawImage() ;
		}
	}
	//
	if ( m_fUpdateBuffer )
	{
		m_rectUpdate |= rctDraw ;
	}
	else
	{
		m_rectUpdate = rctDraw ;
		m_fUpdateBuffer = true ;
	}
	return	errSuccess ;
}

// 表示のための準備処理
//////////////////////////////////////////////////////////////////////////////
Error E3DStereoDisplayAnaglyphView::PrepareView( void )
{
	if ( m_fUpdateBuffer )
	{
		m_fUpdateBuffer = false ;
		//
		EGLImageRect	irRect = m_rectUpdate ;
		EGL_IMAGE_INFO	infDstBuf ;
		EGL_IMAGE_INFO	infSrcBuf[stereoBufferCount] ;
		//
		infDstBuf.dwInfoSize = sizeof(EGL_IMAGE_INFO) ;
		if ( ::eglGetClippedImageInfo
				( &infDstBuf, m_imgView, &irRect ) )
		{
			return	errFailed ;
		}
		for ( int i = 0; i < stereoBufferCount; i ++ )
		{
			infSrcBuf[i].dwInfoSize = sizeof(EGL_IMAGE_INFO) ;
			if ( ::eglGetClippedImageInfo
					( &(infSrcBuf[i]), m_imgBuf[i], &irRect ) )
			{
				return	errFailed ;
			}
		}
		return	MixStereoGraphic
			( &infDstBuf, &(infSrcBuf[stereoRightBuffer]),
							&(infSrcBuf[stereoLeftBuffer]) ) ;
	}
	return	errSuccess ;
}

// 表示処理
//////////////////////////////////////////////////////////////////////////////
Error E3DStereoDisplayAnaglyphView::ViewImage( long int nFlags )
{
	if ( (m_hWnd == NULL) || (m_imgView.GetInfo() == NULL) )
	{
		return	errDeviceNotReady ;
	}
	bool		fStretch = false ;
	int			xDst = 0 ;
	int			yDst = 0 ;
	EGL_SIZE	sizeDst = m_imgView.GetSize() ;
	//
	if ( m_fUseViewPosition )
	{
		fStretch = ((sizeDst.w != m_nWidth)
					|| (sizeDst.h != m_nHeight)) ;
		xDst = m_xDst ;
		yDst = m_yDst ;
		sizeDst.w = m_nWidth ;
		sizeDst.h = m_nHeight ;
	}
	//
	if ( (m_pDrawImage != NULL) && (m_iddsufVRAM != NULL) && fStretch )
	{
		DWORD	fdwFlags = EGLDrawImage::dfDirectDraw ;
		if ( nFlags & E3DSDisplayPlugin::viewWaitVSync )
		{
			fdwFlags |= EGLDrawImage::dfWaitVerticalBlank ;
		}
		m_pDrawImage->DrawImageToDisplay
			( m_hWnd, m_imgView,
				xDst, yDst, &sizeDst, NULL, fdwFlags, m_iddsufVRAM ) ;
	}
	else
	{
		HDC	hdc = ::GetDC( m_hWnd ) ;
		m_imgView.DrawToDC( hdc, xDst, yDst, &sizeDst, NULL ) ;
		::ReleaseDC( m_hWnd, hdc ) ;
	}
	return	errSuccess ;
}

// DirectDraw オブジェクトが削除される前に呼び出される
//////////////////////////////////////////////////////////////////////////////
void E3DStereoDisplayAnaglyphView::OnReleaseDirectDraw( EGLDrawImage * pdi )
{
	if ( m_iddsufVRAM != NULL )
	{
		m_iddsufVRAM->Release() ;
		m_iddsufVRAM = NULL ;
	}
}

// DirectDraw オブジェクトが作成された後に呼び出される
//////////////////////////////////////////////////////////////////////////////
void E3DStereoDisplayAnaglyphView::OnCreateDirectDraw( EGLDrawImage * pdi )
{
}

// アナグリフモード設定
//////////////////////////////////////////////////////////////////////////////
void E3DStereoDisplayAnaglyphView::SetAnaglyphMode( Mode mode )
{
	m_mode = mode ;
}

// 画像合成処理
//////////////////////////////////////////////////////////////////////////////
Error E3DStereoDisplayAnaglyphView::MixStereoGraphic
	( PEGL_IMAGE_INFO pDstInf,
		PEGL_IMAGE_INFO pRightInf, PEGL_IMAGE_INFO pLeftInf )
{
	//
	// 画像バッファの妥当性検証
	//
	if ( (pDstInf == NULL) || (pRightInf == NULL) || (pLeftInf == NULL) )
	{
		return	errDeviceNotReady ;
	}
	//
	DWORD	dwWidth = pDstInf->dwImageWidth ;
	DWORD	dwHeight = pDstInf->dwImageHeight ;
	DWORD	dwBitsPerPixel = pDstInf->dwBitsPerPixel ;
	//
	if ( (pRightInf->dwImageWidth != dwWidth)
		|| (pRightInf->dwImageHeight != dwHeight)
		|| (pRightInf->dwBitsPerPixel != dwBitsPerPixel) )
	{
		return	errFailed ;
	}
	if ( (pLeftInf->dwImageWidth != dwWidth)
		|| (pLeftInf->dwImageHeight != dwHeight)
		|| (pLeftInf->dwBitsPerPixel != dwBitsPerPixel) )
	{
		return	errFailed ;
	}
	//
	// 画像バッファの情報整理
	//
	BYTE *	pbytDstLine = (BYTE*) pDstInf->ptrImageArray ;
	BYTE *	pbytRightLine = (BYTE*) pRightInf->ptrImageArray ;
	BYTE *	pbytLeftLine = (BYTE*) pLeftInf->ptrImageArray ;
	DWORD	dwBytesPerPixel = (dwBitsPerPixel >> 3) ;
	SDWORD	dwDstLineBytes = pDstInf->dwBytesPerLine ;
	SDWORD	dwRightLineBytes = pRightInf->dwBytesPerLine ;
	SDWORD	dwLeftLineBytes = pLeftInf->dwBytesPerLine ;
	//
	// アナグリフ画像合成
	//
	static const MixStereoGraphicLineFunc
					pfnMixStereoLineFuncs[modeMax] =
	{
		MixStereoGraphicHighHue,
		MixStereoGraphicLowHue,
		MixStereoGraphicGray,
	} ;
	if ( ((int) m_mode < 0) && ((int) m_mode >= modeMax) )
	{
		return	errFailed ;
	}
	MixStereoGraphicLineFunc
		pfnMixStereoLine = pfnMixStereoLineFuncs[m_mode] ;
	//
	for ( DWORD y = 0; y < dwHeight; y ++ )
	{
		pfnMixStereoLine
			( pbytDstLine, pbytRightLine, pbytLeftLine,
								dwWidth, dwBytesPerPixel ) ;
		//
		pbytDstLine += dwDstLineBytes ;
		pbytRightLine += dwRightLineBytes ;
		pbytLeftLine += dwLeftLineBytes ;
	}
	//
	return	errSuccess ;
}

void E3DStereoDisplayAnaglyphView::MixStereoGraphicHighHue
	( void * pDst, void * pRight, void * pLeft,
					DWORD dwWidth, DWORD dwBytesPerPixel )
{
	if ( dwWidth != 0 )
	{
		__asm
		{
			mov		ecx, dwWidth
			mov		esi, pLeft
			mov		ebx, pRight
			mov		edi, pDst
			mov		edx, dwBytesPerPixel
		LoopBegin:
					mov		ah, [esi]
				mov		al, [ebx]
					add		ah, [esi + 1]
				mov		[edi], al			; B' = B(r)
					rcr		ah, 1
				mov		al, [ebx + 1]
					add		ah, [esi + 2]
					rcr		ah, 1
				mov		[edi + 1], al		; G' = G(r)
				add		esi, edx
				add		ebx, edx
					mov		[edi + 2], ah	; R' = B(l)/4 + G(l)/4 + R(l)/2
				add		edi, edx
				dec		ecx
			jnz		LoopBegin
		}
	}
}

void E3DStereoDisplayAnaglyphView::MixStereoGraphicLowHue
	( void * pDst, void * pRight, void * pLeft,
					DWORD dwWidth, DWORD dwBytesPerPixel )
{
	if ( dwWidth != 0 )
	{
		__asm
		{
			mov		ecx, dwWidth
			mov		esi, pLeft
			mov		ebx, pRight
			mov		edi, pDst
			mov		edx, dwBytesPerPixel
		LoopBegin:
				mov		al, [ebx + 1]
					mov		ah, [esi]
				add		al, [ebx]
				rcr		al, 1
					add		ah, [esi + 1]
					rcr		ah, 1
				mov		[edi], al			; B' = (G(r) + B(r))/2
				mov		al, [ebx + 1]
					add		ah, [esi + 2]
					rcr		ah, 1
				add		ebx, edx
				add		esi, edx
				mov		[edi + 1], al		; G' = G(r)
					mov		[edi + 2], ah	; R' = B(l)/4 + G(l)/4 + R(l)/2
				add		edi, edx
				dec		ecx
			jnz		LoopBegin
		}
	}
}

void E3DStereoDisplayAnaglyphView::MixStereoGraphicGray
	( void * pDst, void * pRight, void * pLeft,
					DWORD dwWidth, DWORD dwBytesPerPixel )
{
	if ( dwWidth != 0 )
	{
		__asm
		{
			mov		ecx, dwWidth
			mov		esi, pLeft
			mov		ebx, pRight
			mov		edi, pDst
			mov		edx, dwBytesPerPixel
		LoopBegin:
				mov		al, [ebx]
					mov		ah, [esi]
				add		al, [ebx + 2]
				rcr		al, 1
					add		ah, [esi + 2]
					rcr		ah, 1
				add		al, [ebx + 1]
				rcr		al, 1
					add		ah, [esi + 1]
					rcr		ah, 1
				add		ebx, edx
				add		esi, edx
				mov		[edi], al			; B' = B(r)/4 + G(r)/2 + R(r)/4
				mov		[edi + 1], al		; G' = B(r)/4 + G(r)/2 + R(r)/4
					mov		[edi + 2], ah	; R' = B(l)/4 + G(l)/2 + R(l)/4
				add		edi, edx
				dec		ecx
			jnz		LoopBegin
		}
	}
}


//////////////////////////////////////////////////////////////////////////////
// インターリーブ表示インターフェース
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
E3DStereoDisplayInterleaved::E3DStereoDisplayInterleaved
	( EGLDrawImage * pDrawImage,
		bool fInterleavedVertically, bool fInterleavedEventLeft )
{
	m_pDrawImage = pDrawImage ;
	m_hDraw = NULL ;
	m_fUpdateBuffer = false ;
	m_rctUpdated.Clear() ;
	m_fxHorzScale = 0x10000 ;
	m_fxVertScale = 0x10000 ;
	m_fCurrentEvenRight = true ;
	m_fInterleavedVertically = fInterleavedVertically ;
	m_fInterleavedEventLeft = fInterleavedEventLeft ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
E3DStereoDisplayInterleaved::~E3DStereoDisplayInterleaved( void )
{
	if ( m_hDraw != NULL )
	{
		m_hDraw->Release() ;
	}
}

// ビューの振る舞い
//////////////////////////////////////////////////////////////////////////////
long int E3DStereoDisplayInterleaved::Behavior( void )
{
	return	0 ;
}

// 受け取り可能な画像バッファの種類
//////////////////////////////////////////////////////////////////////////////
long int E3DStereoDisplayInterleaved::ImageBufferCapacity( void )
{
	return	flagMemoryImage ;
}

// 最適な画像バッファの種類
//////////////////////////////////////////////////////////////////////////////
long int E3DStereoDisplayInterleaved::BestImageBufferCapacity( void )
{
	return	flagMemoryImage ;
}

// ウィンドウプロシージャ
//////////////////////////////////////////////////////////////////////////////
E3DSDisplayPlugin::WindowProcMethod E3DStereoDisplayInterleaved::WindowProc
	( HWND hWnd, UINT uMsg,
		WPARAM wParam, LPARAM lParam, LRESULT * pResult )
{
	if ( uMsg == WM_MOVE )
	{
		bool	fLastEvent = m_fCurrentEvenRight ;
		UpdateViewScale() ;
		if ( (fLastEvent && !m_fCurrentEvenRight)
			|| (!fLastEvent && m_fCurrentEvenRight) )
		{
			::InvalidateRect( hWnd, NULL, FALSE ) ;
		}
	}
	return	methodContinue ;
}

// 画像バッファのサイズを設定する
//////////////////////////////////////////////////////////////////////////////
E3DSDisplayPlugin::Error
		E3DStereoDisplayInterleaved::SetBufferSize
	( long int nFlags, long int nFormat,
				int nWidth, int nHeight, int nViewCount )
{
	for ( int i = 0; i < E3DSDisplayPlugin::stereoBufferCount; i ++ )
	{
		EGLSize	sizeBuf = m_imgBuf[i].GetSize() ;
		if ( (sizeBuf.w != nWidth) || (sizeBuf.h = nHeight) )
		{
			m_imgBuf[i].CreateImage( EIF_RGB_BITMAP, nWidth, nHeight, 32 ) ;
		}
	}
	//
	UpdateViewScale() ;
	//
	return	errSuccess ;
}

// 表示サイズ設定
//////////////////////////////////////////////////////////////////////////////
E3DSDisplayPlugin::Error E3DStereoDisplayInterleaved::SetViewPosition
	( int xDst, int yDst, int nWidth, int nHeight )
{
	E3DStereoDisplayInterface::SetViewPosition
						( xDst, yDst, nWidth, nHeight ) ;
	//
	EGLSize	sizeView = m_imgView.GetSize() ;
	if ( (sizeView.w != nWidth) || (sizeView.h != nHeight) )
	{
		m_imgView.CreateImage
			( EIF_RGB_BITMAP, nWidth, nHeight, 32, EGL_IMAGE_HAS_DC ) ;
		m_imgView.ReverseVertically() ;
	}
	//
	UpdateViewScale() ;
	//
	return	errSuccess ;
}

// 画像バッファに描画する
//////////////////////////////////////////////////////////////////////////////
E3DSDisplayPlugin::Error E3DStereoDisplayInterleaved::DrawBuffer
	( long int nFlags, int x, int y,
		const E3DSDisplayPlugin::ImageBuffer * bufImage[], int nViewCount )
{
	int	i ;
	if ( (nViewCount <= 0) || (nViewCount > 2) )
	{
		return	errInvalidParameter ;
	}
	if ( nViewCount == 1 )
	{
		//
		// モノビュー画像表示
		//
		EGL_IMAGE_INFO	imginf ;
		if ( !ConvertFromE3DSDisplayImageBuffer
						( imginf, *(bufImage[0]) ) )
		{
			return	errInvalidParameter ;
		}
		if ( m_imgView.GetInfo() == NULL )
		{
			return	errDeviceNotReady ;
		}
		if ( m_imgBuf[0].GetInfo() == NULL )
		{
			return	errDeviceNotReady ;
		}
		if ( m_hDraw == NULL )
		{
			m_hDraw = ::eglCreateDrawImage() ;
		}
		m_hDraw->Initialize( m_imgView, NULL, NULL ) ;
		//
		EGL_DRAW_PARAM	dp ;
		EGL_IMAGE_AXES	iax ;
		memset( &dp, 0, sizeof(dp) ) ;
		dp.ptBasePos.x = x ;
		dp.ptBasePos.y = y ;
		dp.pSrcImage = &imginf ;
		//
		if ( m_fUseViewPosition )
		{
			EGL_SIZE	sizeSrc = m_imgBuf[0].GetSize() ;
			dp.pImageAxes = &iax ;
			if ( sizeSrc.w > 0 )
			{
				iax.xAxis.x = (REAL32) ((double) m_nWidth / sizeSrc.w) ;
			}
			else
			{
				iax.xAxis.x = 1.0 ;
			}
			if ( sizeSrc.w > 0 )
			{
				iax.yAxis.y = (REAL32) ((double) m_nHeight / sizeSrc.h) ;
			}
			else
			{
				iax.yAxis.y = 1.0 ;
			}
			iax.xAxis.y = 0.0 ;
			iax.yAxis.x = 0.0 ;
		}
		//
		if ( !m_hDraw->PrepareDraw( &dp ) )
		{
			m_hDraw->DrawImage() ;
		}
		return	errSuccess ;
	}
	//
	// 動的描画判定
	//
	bool	fDynamicDraw = false ;
	if ( (nFlags & (drawDynamic | drawTemporary))
							== (drawDynamic | drawTemporary) )
	{
		fDynamicDraw = true ;
	}
	if ( fDynamicDraw )
	{
		//
		// 動的描画
		//
		EGL_IMAGE_INFO	imginf[E3DSDisplayPlugin::stereoBufferCount] ;
		EGL_RECT		rctDst[E3DSDisplayPlugin::stereoBufferCount] ;
		for ( i = 0; i < E3DSDisplayPlugin::stereoBufferCount; i ++ )
		{
			if ( !ConvertFromE3DSDisplayImageBuffer
							( imginf[i], *(bufImage[i]) ) )
			{
				return	errFailed ;
			}
			//
			rctDst[i].left = x ;
			rctDst[i].top = y ;
			rctDst[i].right = x + imginf[i].dwImageWidth - 1 ;
			rctDst[i].bottom = y + imginf[i].dwImageHeight - 1 ;
			//
			ViewRectFromSourceRect( rctDst[i] ) ;
		}
		PEGL_IMAGE_INFO	pSrcImages[E3DSDisplayPlugin::stereoBufferCount] =
		{
			&imginf[0], &imginf[1]
		} ;
		DrawBothSideImage( rctDst, pSrcImages, NULL ) ;
		return	errSuccess ;
	}
	//
	// バッファへ書き込み
	//
	if ( m_hDraw == NULL )
	{
		m_hDraw = ::eglCreateDrawImage() ;
	}
	for ( i = 0; i < E3DSDisplayPlugin::stereoBufferCount; i ++ )
	{
		EGL_IMAGE_INFO	imginf ;
		if ( !ConvertFromE3DSDisplayImageBuffer( imginf, *(bufImage[i]) ) )
		{
			continue ;
		}
		if ( m_hDraw->Initialize( m_imgBuf[i], NULL, NULL ) )
		{
			continue ;
		}
		EGL_DRAW_PARAM	dp ;
		::memset( &dp, 0, sizeof(dp) ) ;
		dp.pSrcImage = &imginf ;
		dp.ptBasePos.x = x ;
		dp.ptBasePos.y = y ;
		if ( !m_hDraw->PrepareDraw( &dp ) )
		{
			if ( !m_hDraw->DrawImage() )
			{
				EGL_RECT	rctUpdate ;
				rctUpdate.left = x ;
				rctUpdate.top = y ;
				rctUpdate.right = x + imginf.dwImageWidth - 1 ;
				rctUpdate.bottom = x + imginf.dwImageHeight - 1 ;
				//
				if ( m_rctUpdated.IsEmpty() )
				{
					m_rctUpdated = rctUpdate ;
				}
				else
				{
					m_rctUpdated |= rctUpdate ;
				}
				m_fUpdateBuffer = true ;
			}
		}
	}
	return	errSuccess ;
}

// 表示のための準備処理
//////////////////////////////////////////////////////////////////////////////
E3DSDisplayPlugin::Error E3DStereoDisplayInterleaved::PrepareView( void )
{
	if ( m_fUpdateBuffer )
	{
		m_fUpdateBuffer = false ;
		//
		EGL_RECT	rctDst[E3DSDisplayPlugin::stereoBufferCount] ;
		rctDst[0] = m_rctUpdated ;
		ViewRectFromSourceRect( rctDst[0] ) ;
		rctDst[1] = rctDst[0] ;
		//
		PEGL_IMAGE_INFO	pSrcImages[E3DSDisplayPlugin::stereoBufferCount] =
		{
			m_imgBuf[0], m_imgBuf[1]
		} ;
		DrawBothSideImage( rctDst, pSrcImages, &m_rctUpdated ) ;
	}
	return	errSuccess ;
}

// 表示処理
//////////////////////////////////////////////////////////////////////////////
E3DSDisplayPlugin::Error
	E3DStereoDisplayInterleaved::ViewImage( long int nFlags )
{
	if ( m_imgView.GetInfo() == NULL )
	{
		return	errDeviceNotReady ;
	}
	if ( (m_hWnd == NULL) || !::IsWindow( m_hWnd ) )
	{
		return	errDeviceNotReady ;
	}
	HDC	hdc = ::GetDC( m_hWnd ) ;
	if ( m_fUseViewPosition )
	{
		m_imgView.DrawToDC( hdc, m_xDst, m_yDst, NULL, NULL ) ;
	}
	else
	{
		m_imgView.DrawToDC( hdc, 0, 0, NULL, NULL ) ;
	}
	::ReleaseDC( m_hWnd, hdc ) ;
	return	errSuccess ;
}

// スケール更新
//////////////////////////////////////////////////////////////////////////////
void E3DStereoDisplayInterleaved::UpdateViewScale( void )
{
	EGLSize	sizeDst = m_imgView.GetSize() ;
	EGLSize	sizeSrc = m_imgBuf[0].GetSize() ;
	//
	m_fxHorzScale = 0x10000 ;
	m_fxHorzPitch = 0x10000 ;
	m_fxVertScale = 0x10000 ;
	m_fxVertPitch = 0x10000 ;
	//
	if ( (sizeSrc.w > 0) && (sizeDst.w > 0) )
	{
		m_fxHorzScale = sizeDst.w * 0x10000 / sizeSrc.w ;
		m_fxHorzPitch = sizeSrc.w * 0x10000 / sizeDst.w ;
	}
	if ( (sizeSrc.h > 0) && (sizeDst.h > 0) )
	{
		m_fxVertScale = sizeDst.h * 0x10000 / sizeSrc.h ;
		m_fxVertPitch = sizeSrc.h * 0x10000 / sizeDst.h ;
	}
	//
	POINT	ptClient = { 0, 0 } ;
	if ( m_fUseViewPosition )
	{
		ptClient.x = m_xDst ;
		ptClient.y = m_yDst ;
	}
	if ( (m_hWnd != NULL) && ::IsWindow( m_hWnd ) )
	{
		::ClientToScreen( m_hWnd, &ptClient ) ;
	}
	if ( !m_fInterleavedVertically )
	{
		if ( !m_fInterleavedEventLeft )
		{
			m_fCurrentEvenRight = ((ptClient.x & 0x01) == 0) ;
		}
		else
		{
			m_fCurrentEvenRight = ((ptClient.x & 0x01) != 0) ;
		}
	}
	else
	{
		if ( !m_fInterleavedEventLeft )
		{
			m_fCurrentEvenRight = ((ptClient.y & 0x01) == 0) ;
		}
		else
		{
			m_fCurrentEvenRight = ((ptClient.y & 0x01) != 0) ;
		}
	}
}

// 入力矩形を出力先矩形に変換
//////////////////////////////////////////////////////////////////////////////
void E3DStereoDisplayInterleaved::ViewRectFromSourceRect( EGL_RECT & rect ) const
{
	EGLSize	sizeBuf = m_imgBuf[0].GetSize() ;
	EGLSize	sizeView = m_imgView.GetSize() ;
	if ( (rect.left <= 0) && (rect.top <= 0)
		&& (rect.right + 1 >= sizeBuf.w)
		&& (rect.bottom + 1 >= sizeBuf.h) )
	{
		rect.left = 0 ;
		rect.top = 0 ;
		rect.right = sizeView.w - 1 ;
		rect.bottom = sizeView.h - 1 ;
	}
	else
	{
		rect.left = (int) ((INT64) rect.left * m_fxHorzScale / 0x10000) ;
		rect.top = (int) ((INT64) rect.top * m_fxVertScale / 0x10000) ;
		rect.right =
			(int) (((INT64) (rect.right + 1)
					* m_fxHorzScale + 0xFFFF) / 0x10000) - 1 ;
		rect.bottom =
			(int) (((INT64) (rect.bottom + 1)
					* m_fxVertScale + 0xFFFF) / 0x10000) - 1 ;
	}
}

// 画像描画
//////////////////////////////////////////////////////////////////////////////
void E3DStereoDisplayInterleaved::DrawBothSideImage
	( const EGL_RECT rctDst[],
		PEGL_IMAGE_INFO pSrcImages[], const EGL_RECT * pSrcRect )
{
	ESLAssert( pSrcImages[0] && pSrcImages[1] ) ;
	if ( !m_fInterleavedVertically
		&& (pSrcImages[0]->dwImageWidth == pSrcImages[1]->dwImageWidth)
		&& (pSrcImages[0]->dwImageHeight == pSrcImages[1]->dwImageHeight)
		&& (EGLRect(rctDst[0]) == EGLRect(rctDst[1])) )
	{
		EGLRect	rctDstTemp = rctDst[0] ;
		int	y, nBlockLines ;
		int	nDstWidth = rctDst[0].right - rctDst[0].left + 1 ;
		if ( nDstWidth <= 0 )
		{
			return ;
		}
		nBlockLines = 0x20000 / nDstWidth ;
		if ( nBlockLines <= 2 )
		{
			nBlockLines = 2 ;
		}
		SDWORD	fxSrcX = 0 ;
		SDWORD	fxSrcY = 0 ;
		if ( pSrcRect != NULL )
		{
			fxSrcX = pSrcRect->left * 0x10000 ;
			fxSrcY = pSrcRect->top * 0x10000 ;
		}
		for ( y = rctDst[0].top; y <= rctDst[0].bottom; y += nBlockLines )
		{
			rctDstTemp.top = y ;
			rctDstTemp.bottom = y + nBlockLines - 1 ;
			if ( rctDstTemp.bottom > rctDst[0].bottom )
			{
				rctDstTemp.bottom = rctDst[0].bottom ;
			}
			for ( int i = 0; i < E3DSDisplayPlugin::stereoBufferCount; i ++ )
			{
				DrawSideImage
					( rctDstTemp, pSrcImages[i], fxSrcX, fxSrcY, i ) ;
			}
			fxSrcY +=
				m_fxVertPitch * (rctDstTemp.bottom + 1 - rctDstTemp.top) ;
		}
	}
	else
	{
		SDWORD	fxSrcX = 0 ;
		SDWORD	fxSrcY = 0 ;
		if ( pSrcRect != NULL )
		{
			fxSrcX = pSrcRect->left * 0x10000 ;
			fxSrcY = pSrcRect->top * 0x10000 ;
		}
		for ( int i = 0; i < E3DSDisplayPlugin::stereoBufferCount; i ++ )
		{
			DrawSideImage
				( rctDst[i], pSrcImages[i], fxSrcX, fxSrcY, i ) ;
		}
	}
}

// 画像描画
//////////////////////////////////////////////////////////////////////////////
void E3DStereoDisplayInterleaved::DrawSideImage
	( const EGL_RECT & rectDst,
			PEGL_IMAGE_INFO pSrcImage,
			SDWORD fxSrcX, SDWORD fxSrcY, int nSide )
{
	//
	// 描画領域正規化
	//
/*	SDWORD	fxSrcX = 0, fxSrcY = 0 ;
	if ( pSrcRect != NULL )
	{
		fxSrcX = pSrcRect->left * 0x10000 ;
		fxSrcY = pSrcRect->top * 0x10000 ;
	}
*/	EGLImageRect	irDst = rectDst ;
	if ( irDst.x < 0 )
	{
		irDst.w += irDst.x ;
		fxSrcX += - irDst.x * m_fxHorzPitch ;
	}
	if ( irDst.y < 0 )
	{
		irDst.h += irDst.y ;
		fxSrcY += - irDst.y * m_fxVertPitch ;
	}
	PEGL_IMAGE_INFO	pDstInf = m_imgView ;
	if ( pDstInf == NULL )
	{
		return ;
	}
	EGLSize	sizeDst ;
	sizeDst.w = pDstInf->dwImageWidth ;
	sizeDst.h = pDstInf->dwImageHeight ;
	if ( sizeDst.w < irDst.w )
	{
		irDst.w = sizeDst.w ;
	}
	if ( sizeDst.h < irDst.h )
	{
		irDst.h = sizeDst.h ;
	}
	if ( (irDst.x >= sizeDst.w) || (irDst.y >= sizeDst.h)
						|| (irDst.w <= 0) || (irDst.h <= 0) )
	{
		return ;
	}
	//
	// 描画アドレス決定
	//
	DWORD	dwDstPixelBytes = (pDstInf->dwBitsPerPixel >> 3) ;
	DWORD	dwDstPixelPitch = dwDstPixelBytes ;
	DWORD	dwDstLineBytes = pDstInf->dwBytesPerLine ;
	SDWORD	fxSrcPitchX = m_fxHorzPitch ;
	SDWORD	fxSrcPitchY = m_fxVertPitch ;
	BYTE *	pbytDst = (BYTE*) pDstInf->ptrImageArray ;
	int		nSideOffset = nSide & 0x01 ;
	if ( !m_fCurrentEvenRight )
	{
		nSideOffset ^= 0x01 ;
	}
	pbytDst += (irDst.y * dwDstLineBytes) + irDst.x * dwDstPixelBytes ;
	//
	if ( !m_fInterleavedVertically )
	{
		if ( (irDst.x + nSideOffset) & 0x01 )
		{
			irDst.x ++ ;
			irDst.w -- ;
			fxSrcX += fxSrcPitchX ;
			pbytDst += dwDstPixelBytes ;
		}
		irDst.w >>= 1 ;
		dwDstPixelPitch *= 2 ;
		fxSrcPitchX *= 2 ;
	}
	else
	{
		if ( (irDst.y + nSideOffset) & 0x01 )
		{
			irDst.y ++ ;
			irDst.h -- ;
			fxSrcY += fxSrcPitchY ;
			pbytDst += dwDstLineBytes ;
		}
		irDst.h >>= 1 ;
		dwDstLineBytes *= 2 ;
		fxSrcPitchY *= 2 ;
	}
	DrawImagePixels
		( pbytDst, dwDstPixelBytes,
			dwDstPixelPitch, dwDstLineBytes,
			pSrcImage, fxSrcX, fxSrcY,
			fxSrcPitchX, fxSrcPitchY, irDst.w, irDst.h ) ;
}

void E3DStereoDisplayInterleaved::DrawImagePixels
	( BYTE * pbytDst, DWORD dwDstPixelBytes,
			DWORD dwDstPixelPitch, DWORD dwDstLineBytes,
			PEGL_IMAGE_INFO pSrcImage,
			SDWORD fxSrcX, SDWORD fxSrcY,
			SDWORD fxSrcPitchX, SDWORD fxSrcPitchY,
			DWORD dwWidth, DWORD dwHeight )
{
	if ( dwWidth == 0 )
	{
		return ;
	}
	DWORD	dwSrcWidth = pSrcImage->dwImageWidth ;
	DWORD	dwValidWidth = dwWidth ;
	DWORD	dwOddWidth = 0 ;
	if ( ((fxSrcX + fxSrcPitchX * dwValidWidth) >> 16) >= dwSrcWidth )
	{
		if ( fxSrcPitchX > 0 )
		{
			dwValidWidth =
				(DWORD) ((INT64) pSrcImage->dwImageWidth * 0x10000
											- 1 - fxSrcX) / fxSrcPitchX ;
			if ( dwValidWidth > dwWidth )
			{
				dwValidWidth = dwWidth ;
			}
			dwOddWidth = dwWidth - dwValidWidth ;
		}
	}
	//
	DWORD	dwSrcPixelPitch = (pSrcImage->dwBitsPerPixel >> 3) ;
	bool	fPixelDWord = (dwDstPixelBytes == 4) && (dwSrcPixelPitch == 4) ;
	BYTE *	pbytDstLine = pbytDst ;
	for ( DWORD yLine = 0; yLine < dwHeight; yLine ++ )
	{
		const BYTE *
			pbytSrcLine = (const BYTE *) pSrcImage->ptrImageArray ;
		long int	y = (fxSrcY >> 16) ;
		if ( y >= (long int) pSrcImage->dwImageHeight )
		{
			y = pSrcImage->dwImageHeight - 1 ;
		}
		pbytSrcLine += y * pSrcImage->dwBytesPerLine ;
		//
		if ( fPixelDWord )
		{
			__asm
			{
				mov		edi, pbytDstLine
				mov		esi, pbytSrcLine
				mov		ecx, dwValidWidth
				mov		edx, fxSrcX
				mov		eax, edx
				shr		eax, 16
LoopValidHorz:
					add		edx, fxSrcPitchX
					;
					mov		ebx, [esi + eax * 4]
					mov		eax, edx
					mov		[edi], ebx
					shr		eax, 16
					add		edi, dwDstPixelPitch
					;
					dec		ecx
				jnz		LoopValidHorz
				;
				mov		ecx, dwOddWidth
				mov		eax, dwSrcWidth
				test	ecx, ecx
				jz		LoopEndHorz
				mov		eax, [esi + eax * 4 - 4]
				mov		edx, dwDstPixelPitch
LoopOddHorz:
					mov		[edi], eax
					add		edi, edx
					dec		ecx
				jnz		LoopOddHorz
LoopEndHorz:
			}
		}
		else
		{
			DWORD	fxPosX = fxSrcX ;
			BYTE *	pbytDstPixel = pbytDstLine ;
			DWORD	x ;
			for ( x = 0; x < dwValidWidth; x ++ )
			{
				DWORD	dwSrcOffset = (fxPosX >> 16) * dwSrcPixelPitch ;
				fxPosX += fxSrcPitchX ;
				//
				pbytDstPixel[0] = pbytSrcLine[dwSrcOffset] ;
				pbytDstPixel[1] = pbytSrcLine[dwSrcOffset + 1] ;
				pbytDstPixel[2] = pbytSrcLine[dwSrcOffset + 2] ;
				pbytDstPixel += dwDstPixelPitch ;
			}
			for ( x = 0; x < dwOddWidth; x ++ )
			{
				DWORD	dwSrcOffset = (dwSrcWidth - 1) * dwSrcPixelPitch ;
				pbytDstPixel[0] = pbytSrcLine[dwSrcOffset] ;
				pbytDstPixel[1] = pbytSrcLine[dwSrcOffset + 1] ;
				pbytDstPixel[2] = pbytSrcLine[dwSrcOffset + 2] ;
				pbytDstPixel += dwDstPixelPitch ;
			}
		}
		//
		pbytDstLine += dwDstLineBytes ;
		fxSrcY += fxSrcPitchY ;
	}
}



//////////////////////////////////////////////////////////////////////////////
// DirectDraw stereoscopic 表示インターフェース
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
E3DStereoDisplayDDStereoscopic::E3DStereoDisplayDDStereoscopic
	( EGLDrawImage * pDrawImage )
{
	m_pDrawImage = pDrawImage ;
	//
	for ( int i = 0; i < stereoBufferCount; i ++ )
	{
		m_iddsBuf[i] = NULL ;
	}
	m_fDynamicDraw = false ;
	m_fBuffer = false ;
	//
	if ( m_pDrawImage != NULL )
	{
		m_pDrawImage->AddNotify( this ) ;
	}
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
E3DStereoDisplayDDStereoscopic::~E3DStereoDisplayDDStereoscopic( void )
{
	if ( m_pDrawImage != NULL )
	{
		m_pDrawImage->DetachNotify( this ) ;
	}
	ReleaseSurface() ;
}

// 所有しているサーフェスを解放する
//////////////////////////////////////////////////////////////////////////////
void E3DStereoDisplayDDStereoscopic::ReleaseSurface( void )
{
	for ( int i = 0; i < stereoBufferCount; i ++ )
	{
		if ( m_iddsBuf[i] != NULL )
		{
			m_iddsBuf[i]->Release() ;
			m_iddsBuf[i] = NULL ;
		}
	}
}

// サーフェスを生成する
//////////////////////////////////////////////////////////////////////////////
Error E3DStereoDisplayDDStereoscopic::CreateSurface( int nWidth, int nHeight )
{
	if ( m_pDrawImage == NULL )
	{
		return	errDeviceNotReady ;
	}
	for ( int i = 0; i < stereoBufferCount; i ++ )
	{
		if ( m_iddsBuf[i] != NULL )
		{
			m_iddsBuf[i]->Release() ;
			m_iddsBuf[i] = NULL ;
		}
		m_iddsBuf[i] =
			m_pDrawImage->CreateDD7Surface( nWidth, nHeight ) ;
		//
		if ( m_iddsBuf[i] == NULL )
		{
			return	errFailed ;
		}
	}
	return	errSuccess ;
}

// ビューの振る舞い
//////////////////////////////////////////////////////////////////////////////
long int E3DStereoDisplayDDStereoscopic::Behavior( void )
{
	return	flagNeedToChangeDisplayMode ;
}

// 受け取り可能な画像バッファの種類
//////////////////////////////////////////////////////////////////////////////
long int E3DStereoDisplayDDStereoscopic::ImageBufferCapacity( void )
{
	return	flagMemoryImage | flagDeviceContext | flagDirectDrawSurface7 ;
}

// 最適な画像バッファの種類
//////////////////////////////////////////////////////////////////////////////
long int E3DStereoDisplayDDStereoscopic::BestImageBufferCapacity( void )
{
	return	flagDirectDrawSurface7 ;
}

// 画像バッファのサイズを設定する
//////////////////////////////////////////////////////////////////////////////
Error E3DStereoDisplayDDStereoscopic::SetBufferSize
	( long int nFlags, long int nFormat,
				int nWidth, int nHeight, int nViewCount )
{
	if ( m_pDrawImage == NULL )
	{
		return	errDeviceNotReady ;
	}
	if ( m_pDrawImage->GetDirectDraw7() == NULL )
	{
		return	errDeviceNotReady ;
	}
	ReleaseSurface() ;
	//
	Error	err = CreateSurface( nWidth, nHeight ) ;
	if ( err )
	{
		return	err ;
	}
	m_fBuffer = true ;
	m_sizeBuffer.w = nWidth ;
	m_sizeBuffer.h = nHeight ;
	return	errSuccess ;
}

// 画面モード変更時の処理
//////////////////////////////////////////////////////////////////////////////
DisplayModeMethod E3DStereoDisplayDDStereoscopic::OnChangeDisplayMode
	( HMONITOR hMonitor,
		int nWidth, int nHeight, int nBitsPerPixel, int nFrequency )
{
	if ( m_pDrawImage == NULL )
	{
		return	methodNoModeChanged ;
	}
	GUID	guidDevice ;
	GUID *	pguidDevice =
		m_dmMode.GetDDMonitorGUID( &guidDevice, hMonitor ) ;
	if ( m_pDrawImage->CreateDirectDraw( pguidDevice ) )
	{
		ESLTrace( "Failed to CreateDirectDraw.\n" ) ;
		return	methodNoModeChanged ;
	}
	if ( m_pDrawImage->CreateSurfaceFullscreenMode
		( m_hWnd, nWidth, nHeight, nBitsPerPixel, nFrequency, true ) )
	{
		m_pDrawImage->CreateSurfaceWindowMode() ;
		return	methodNoModeChanged ;
	}
	::SetCursor( NULL ) ;
	return	methodModeChanged ;
}

// 画面モード復帰時の処理
//////////////////////////////////////////////////////////////////////////////
DisplayModeMethod E3DStereoDisplayDDStereoscopic::OnRestoreDisplayMode( void )
{
	if ( m_pDrawImage == NULL )
	{
		return	methodNoModeChanged ;
	}
	if ( m_pDrawImage->IsFullscreenMode() )
	{
		m_pDrawImage->Initialize() ;
		return	methodModeChanged ;
	}
	return	methodNoModeChanged ;
}

// 画像バッファに描画する
//////////////////////////////////////////////////////////////////////////////
Error E3DStereoDisplayDDStereoscopic::DrawBuffer
	( long int nFlags, int x, int y,
		const ImageBuffer * bufImage[], int nViewCount )
{
	if ( m_pDrawImage == NULL )
	{
		return	errDeviceNotReady ;
	}
	//
	// 動的描画処理判定
	//
	bool	fDynamicDraw = false ;
	if ( m_pDrawImage->IsFullscreenMode()
		&& ((nFlags & (drawDynamic | drawTemporary))
							== (drawDynamic | drawTemporary)) )
	{
		if ( m_fUseViewPosition )
		{
			if ( (m_nWidth == m_sizeBuffer.w)
				&& (m_nHeight == m_sizeBuffer.h) )
			{
				fDynamicDraw = true ;
			}
		}
		else
		{
			fDynamicDraw = true ;
		}
		if ( fDynamicDraw )
		{
			if ( m_fUseViewPosition )
			{
				x += m_xDst ;
				y += m_yDst ;
			}
			m_fDynamicDraw = true ;
		}
	}
	for ( int i = 0; i < stereoBufferCount; i ++ )
	{
		if ( m_iddsBuf[i] == NULL )
		{
			return	errDeviceNotReady ;
		}
		const ImageBuffer * pbufImage ;
		if ( i < nViewCount )
		{
			pbufImage = bufImage[i] ;
		}
		else
		{
			pbufImage = bufImage[nViewCount - 1] ;
		}
		if ( pbufImage == NULL )
		{
			return	errInvalidParameter ;
		}
		//
		// 描画領域取得
		//
		EGL_SIZE	sizeDst ;
		EGL_RECT	rectSrc ;
		EGL_RECT *	pSrcRect = NULL ;
		//
		sizeDst.w = pbufImage->nWidth ;
		sizeDst.h = pbufImage->nHeight ;
		rectSrc.left = 0 ;
		rectSrc.top = 0 ;
		rectSrc.right = sizeDst.w - 1 ;
		rectSrc.bottom = sizeDst.h - 1 ;
		//
		if ( pbufImage->nFlags & flagSourceRectangle )
		{
			sizeDst.w = pbufImage->rctSource.w ;
			sizeDst.h = pbufImage->rctSource.h ;
			rectSrc.left = pbufImage->rctSource.x ;
			rectSrc.top = pbufImage->rctSource.y ;
			rectSrc.right = rectSrc.left + sizeDst.w - 1 ;
			rectSrc.bottom = rectSrc.top + sizeDst.h - 1 ;
			pSrcRect = &rectSrc ;
		}
		//
		// 描画先サーフェス取得
		//
		IDirectDrawSurface7 *	iddsBuf = m_iddsBuf[i] ;
		if ( fDynamicDraw )
		{
			if ( i == stereoRightBuffer )
			{
				iddsBuf = m_pDrawImage->GetDD7SecondarySurface() ;
			}
			else
			{
				iddsBuf = m_pDrawImage->GetDD7SecondaryLeftSurface() ;
			}
			if ( iddsBuf == NULL )
			{
				continue ;
			}
		}
		//
		if ( (pbufImage->nFlags & flagDirectDrawSurface7)
					&& (pbufImage->iddsSurface != NULL) )
		{
			//
			// DirectDrawSurface7 からの描画
			//
			IDirectDrawSurface7 *
				iddsSrc = (IDirectDrawSurface7*) pbufImage->iddsSurface ;
			m_pDrawImage->BltSurface
				( iddsBuf, iddsSrc, x, y, &sizeDst, pSrcRect ) ;
		}
		else if ( (pbufImage->nFlags & flagDeviceContext)
							&& (pbufImage->hDC != NULL) )
		{
			//
			// DeviceContext からの描画
			//
			HDC	hdc ;
			if ( iddsBuf->GetDC( &hdc ) != DD_OK )
			{
				return	errFailed ;
			}
			::BitBlt( hdc, x, y, sizeDst.w, sizeDst.h,
					pbufImage->hDC, rectSrc.left, rectSrc.top, SRCCOPY ) ;
			iddsBuf->ReleaseDC( hdc ) ;
		}
		else
		{
			//
			// メモリからの描画
			//
			EGL_IMAGE_INFO	imginf ;
			if ( !ConvertFromE3DSDisplayImageBuffer( imginf, *pbufImage ) )
			{
				return	errInvalidParameter ;
			}
			HDC	hdc ;
			if ( iddsBuf->GetDC( &hdc ) != DD_OK )
			{
				return	errFailed ;
			}
			::eglDrawToDC( hdc, &imginf, x, y, &sizeDst, pSrcRect ) ;
			iddsBuf->ReleaseDC( hdc ) ;
		}
	}
	return	errSuccess ;
}

// 表示のための準備処理
//////////////////////////////////////////////////////////////////////////////
Error E3DStereoDisplayDDStereoscopic::PrepareView( void )
{
	return	errSuccess ;
}

// 表示処理
//////////////////////////////////////////////////////////////////////////////
Error E3DStereoDisplayDDStereoscopic::ViewImage( long int nFlags )
{
	if ( m_pDrawImage == NULL )
	{
		return	errDeviceNotReady ;
	}
	if ( m_fDynamicDraw )
	{
		//
		// バックバッファには既に書き込んであるのでフリップ処理のみ
		//
		m_fDynamicDraw = false ;
		if ( m_pDrawImage->Flip() )
		{
			return	errFailed ;
		}
		return	errSuccess ;
	}
	//
	// 中間バッファからバックバッファに転送してからフリップ
	//
	int			xDst = 0 ;
	int			yDst = 0 ;
	EGL_SIZE	sizeDst = m_sizeBuffer ;
	EGL_RECT	rectSrc ;
	//
	if ( m_fUseViewPosition )
	{
		xDst = m_xDst ;
		yDst = m_yDst ;
		sizeDst.w = m_nWidth ;
		sizeDst.h = m_nHeight ;
	}
	//
	rectSrc.left = 0 ;
	rectSrc.top = 0 ;
	rectSrc.right = m_sizeBuffer.w - 1 ;
	rectSrc.bottom = m_sizeBuffer.h - 1 ;
	//
	if ( m_pDrawImage->DrawStereoImageToDisplay
		( m_hWnd, m_iddsBuf[stereoLeftBuffer],
			m_iddsBuf[stereoRightBuffer], xDst, yDst, &sizeDst, &rectSrc ) )
	{
		return	errFailed ;
	}
	return	errSuccess ;
}

// DirectDraw オブジェクトが削除される前に呼び出される
//////////////////////////////////////////////////////////////////////////////
void E3DStereoDisplayDDStereoscopic::OnReleaseDirectDraw( EGLDrawImage * pdi )
{
	ReleaseSurface() ;
}

// DirectDraw オブジェクトが作成された後に呼び出される
//////////////////////////////////////////////////////////////////////////////
void E3DStereoDisplayDDStereoscopic::OnCreateDirectDraw( EGLDrawImage * pdi )
{
	if ( m_fBuffer )
	{
		CreateSurface( m_sizeBuffer.w, m_sizeBuffer.h ) ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// NVIDIA StereoBLT 表示インターフェース
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
E3DStereoDisplayNVStereoBLT::E3DStereoDisplayNVStereoBLT
	( EGLDrawImage * pDrawImage )
{
	m_pDrawImage = pDrawImage ;
	m_iddsBuf = NULL ;
	//
	m_id3d9Dev = NULL ;
	m_idds9Buf = NULL ;
	m_hDraw = NULL ;
	//
	m_fBuffer = false ;
	//
	if ( m_pDrawImage != NULL )
	{
		m_pDrawImage->AddNotify( this ) ;
	}
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
E3DStereoDisplayNVStereoBLT::~E3DStereoDisplayNVStereoBLT( void )
{
	if ( m_pDrawImage != NULL )
	{
		m_pDrawImage->DetachNotify( this ) ;
	}
	if ( m_hDraw != NULL )
	{
		m_hDraw->Release() ;
		m_hDraw = NULL ;
	}
	ReleaseSurface() ;
}

// 所有しているサーフェスを解放する
//////////////////////////////////////////////////////////////////////////////
void E3DStereoDisplayNVStereoBLT::ReleaseSurface( void )
{
	if ( m_iddsBuf != NULL )
	{
		m_iddsBuf->Release() ;
		m_iddsBuf = NULL ;
	}
	if ( m_idds9Buf != NULL )
	{
		m_idds9Buf->Release() ;
		m_idds9Buf = NULL ;
	}
}

// サーフェスを生成する
//////////////////////////////////////////////////////////////////////////////
E3DSDisplayPlugin::Error
	E3DStereoDisplayNVStereoBLT::CreateSurface( int nWidth, int nHeight )
{
	if ( m_pDrawImage == NULL )
	{
		return	errDeviceNotReady ;
	}
	if ( (m_iddsBuf != NULL) || (m_idds9Buf != NULL) )
	{
		ReleaseSurface() ;
	}
	if ( m_id3d9Dev != NULL )
	{
		m_id3d9Dev->CreateOffscreenPlainSurface
			( nWidth * 2, nHeight + 1, D3DFMT_A8R8G8B8,
						D3DPOOL_DEFAULT, &m_idds9Buf, NULL ) ;
		if ( m_idds9Buf == NULL )
		{
			return	errFailed ;
		}
	}
	else
	{
		m_iddsBuf = m_pDrawImage->CreateDD7Surface( nWidth * 2, nHeight + 1 ) ;
		if ( m_iddsBuf == NULL )
		{
			return	errFailed ;
		}
	}
	return	errSuccess ;
}

// ビューの振る舞い
//////////////////////////////////////////////////////////////////////////////
long int E3DStereoDisplayNVStereoBLT::Behavior( void )
{
	return	flagNeedToChangeDisplayMode ;
}

// 受け取り可能な画像バッファの種類
//////////////////////////////////////////////////////////////////////////////
long int E3DStereoDisplayNVStereoBLT::ImageBufferCapacity( void )
{
	return	flagMemoryImage ;
}

// 最適な画像バッファの種類
//////////////////////////////////////////////////////////////////////////////
long int E3DStereoDisplayNVStereoBLT::BestImageBufferCapacity( void )
{
	return	flagDirectDrawSurface7 ;
}

// 画像バッファのサイズを設定する
//////////////////////////////////////////////////////////////////////////////
E3DSDisplayPlugin::Error E3DStereoDisplayNVStereoBLT::SetBufferSize
	( long int nFlags, long int nFormat,
				int nWidth, int nHeight, int nViewCount )
{
	bool	fRecreateSurface = true ;
	if ( ((m_idds9Buf != NULL) || (m_iddsBuf != NULL))
		&& (m_sizeBuffer.w == nWidth) && (m_sizeBuffer.h == nHeight) )
	{
		fRecreateSurface = false ;
	}
	m_fBuffer = true ;
	m_sizeBuffer.w = nWidth ;
	m_sizeBuffer.h = nHeight ;
	//
	if ( m_pDrawImage == NULL )
	{
		return	errDeviceNotReady ;
	}
	if ( (m_id3d9Dev == NULL)
		&& (m_pDrawImage->GetDirectDraw7() == NULL) )
	{
		return	errDeviceNotReady ;
	}
	if ( fRecreateSurface )
	{
		ReleaseSurface() ;
		//
		Error	err = CreateSurface( nWidth, nHeight ) ;
		if ( err )
		{
			return	err ;
		}
	}
	return	errSuccess ;
}

// 画面モード変更時の処理
//////////////////////////////////////////////////////////////////////////////
E3DSDisplayPlugin::DisplayModeMethod
		E3DStereoDisplayNVStereoBLT::OnChangeDisplayMode
	( HMONITOR hMonitor,
		int nWidth, int nHeight, int nBitsPerPixel, int nFrequency )
{
	if ( m_pDrawImage == NULL )
	{
		return	methodNoModeChanged ;
	}
	GUID	guidDevice ;
	int		nAdapter = 0 ;
	GUID *	pguidDevice =
		m_dmMode.GetDDMonitorGUID( &guidDevice, hMonitor, &nAdapter ) ;
	//
	if ( !m_pDrawImage->CreateDirect3D9Device
				( m_hWnd, nWidth, nHeight, nAdapter ) )
	{
		m_id3d9Dev = m_pDrawImage->GetDirect3DDevice9() ;
		//
		if ( m_fBuffer )
		{
			ReleaseSurface() ;
			CreateSurface( m_sizeBuffer.w, m_sizeBuffer.h ) ;
			//
			const ImageBuffer *	pbufImages[2] = { NULL, NULL } ;
			DrawBuffer( drawDynamic | drawTemporary, 0, 0, pbufImages, 2 ) ;
			PrepareView( ) ;
			ViewImage( 0 ) ;
		}
	}
	else
	{
		if ( m_pDrawImage->CreateDirectDraw( pguidDevice ) )
		{
			ESLTrace( "Failed to CreateDirectDraw.\n" ) ;
			return	methodNoModeChanged ;
		}
		if ( m_pDrawImage->CreateSurfaceFullscreenMode
			( m_hWnd, nWidth, nHeight, nBitsPerPixel, nFrequency, false ) )
		{
			ESLTrace( "Failed to CreateSurfaceFullscreenMode.\n" ) ;
			m_pDrawImage->CreateSurfaceWindowMode() ;
			return	methodNoModeChanged ;
		}
	}
	::SetCursor( NULL ) ;
	return	methodModeChanged ;
}

// 画面モード復帰時の処理
//////////////////////////////////////////////////////////////////////////////
E3DSDisplayPlugin::DisplayModeMethod
		E3DStereoDisplayNVStereoBLT::OnRestoreDisplayMode( void )
{
	if ( m_pDrawImage == NULL )
	{
		return	methodNoModeChanged ;
	}
	m_id3d9Dev = m_pDrawImage->GetDirect3DDevice9() ;
	if ( m_id3d9Dev != NULL )
	{
		m_id3d9Dev = NULL ;
		ReleaseSurface() ;
		m_pDrawImage->Release() ;
		m_pDrawImage->Initialize() ;
		return	methodModeChanged ;
	}
	else if ( m_pDrawImage->IsFullscreenMode() )
	{
		m_pDrawImage->Initialize() ;
		return	methodModeChanged ;
	}
	return	methodNoModeChanged ;
}

// 画像バッファに描画する
//////////////////////////////////////////////////////////////////////////////
E3DSDisplayPlugin::Error E3DStereoDisplayNVStereoBLT::DrawBuffer
	( long int nFlags, int x, int y,
		const E3DSDisplayPlugin::ImageBuffer * bufImage[], int nViewCount )
{
	if ( m_pDrawImage == NULL )
	{
		return	errDeviceNotReady ;
	}
	m_id3d9Dev = m_pDrawImage->GetDirect3DDevice9() ;
	if ( (m_iddsBuf == NULL) && (m_idds9Buf == NULL) )
	{
		return	errDeviceNotReady ;
	}
	if ( (nViewCount <= 0) || (nViewCount > 2) )
	{
		return	errInvalidParameter ;
	}
#if	defined(_DEBUG)
	INT64	nBegin, nEnd, nFreq ;
	::QueryPerformanceCounter( (LARGE_INTEGER*) &nBegin ) ;
#endif
	//
	// サーフェスメモリロック
	//
	EGL_IMAGE_INFO	infSurface ;
	memset( &infSurface, 0, sizeof(infSurface) ) ;
	infSurface.dwInfoSize = sizeof(infSurface) ;
	//
	if ( m_idds9Buf != NULL )
	{
		D3DSURFACE_DESC	d3dsd ;
		if ( m_idds9Buf->GetDesc( &d3dsd ) != D3D_OK )
		{
			ESLTrace( "Failed to IDirect3DSurface9::GetDesc.\n" ) ;
			return	errFailed ;
		}
		D3DLOCKED_RECT	lr ;
		if ( m_idds9Buf->LockRect( &lr, NULL, 0 ) != D3D_OK )
		{
			ESLTrace( "Failed to IDirect3DSurface9::LockRect.\n" ) ;
			return	errFailed ;
		}
		infSurface.fdwFormatType = EIF_RGBA_BITMAP ;
		infSurface.ptrImageArray = lr.pBits ;
		infSurface.dwImageWidth = d3dsd.Width ;
		infSurface.dwImageHeight = d3dsd.Height ;
		infSurface.dwBitsPerPixel = 32 ;
		infSurface.dwBytesPerLine = lr.Pitch ;
		infSurface.dwSizeOfImage =
					infSurface.dwImageHeight * infSurface.dwBytesPerLine ;
	}
	else
	{
		DDSURFACEDESC2	ddsd ;
		memset( &ddsd, 0, sizeof(ddsd) ) ;
		ddsd.dwSize = sizeof(ddsd) ;
		ESLAssert( m_iddsBuf != NULL ) ;
		if ( m_iddsBuf->Lock
			( NULL, &ddsd,
				DDLOCK_SURFACEMEMORYPTR
					| DDLOCK_WAIT | DDLOCK_WRITEONLY, NULL ) != DD_OK )
		{
			ESLTrace( "Failed to IDirectDrawSurface7::Lock.\n" ) ;
			return	errFailed ;
		}
		infSurface.ptrImageArray = ddsd.lpSurface ;
		infSurface.dwImageWidth = ddsd.dwWidth ;
		infSurface.dwImageHeight = ddsd.dwHeight ;
		infSurface.dwBitsPerPixel = ddsd.ddpfPixelFormat.dwRGBBitCount ;
		infSurface.fdwFormatType =
					(infSurface.dwBitsPerPixel == 32) ?
								 EIF_RGBA_BITMAP : EIF_RGB_BITMAP ;
		infSurface.dwBytesPerLine = ddsd.lPitch ;
		infSurface.dwSizeOfImage =
					infSurface.dwImageHeight * infSurface.dwBytesPerLine ;
	}
	//
	if ( m_hDraw == NULL )
	{
		m_hDraw = ::eglCreateDrawImage() ;
	}
	m_hDraw->Initialize( &infSurface, NULL, NULL ) ;
	//
	for ( int i = 0; i < stereoBufferCount; i ++, x += m_sizeBuffer.w )
	{
		const E3DSDisplayPlugin::ImageBuffer *	pbufImage ;
		if ( i < nViewCount )
		{
			pbufImage = bufImage[i] ;
		}
		else
		{
			pbufImage = bufImage[nViewCount - 1] ;
		}
		if ( pbufImage == NULL )
		{
			continue ;
//			return	errInvalidParameter ;
		}
		//
		// メモリからの描画
		//
		EGL_IMAGE_INFO	imginf ;
		if ( !ConvertFromE3DSDisplayImageBuffer( imginf, *pbufImage ) )
		{
			return	errInvalidParameter ;
		}
		EGL_DRAW_PARAM	dp ;
		memset( &dp, 0, sizeof(dp) ) ;
		dp.pSrcImage = &imginf ;
		dp.ptBasePos.x = x ;
		dp.ptBasePos.y = y ;
		//
		if ( !m_hDraw->PrepareDraw( &dp ) )
		{
			m_hDraw->DrawImage() ;
		}
	}
	//
	// NVIDIA シグネチャ設定
	//
	Nv_Stereo_Image_Header *	pnvsih =
		(Nv_Stereo_Image_Header*)
			(((BYTE*)infSurface.ptrImageArray)
				+ (infSurface.dwBytesPerLine
						* (infSurface.dwImageHeight - 1))) ;
	//
	pnvsih->dwSignature = NVSTEREO_IMAGE_SIGNATURE ;
	pnvsih->dwWidth = infSurface.dwImageWidth ;
	pnvsih->dwHeight = infSurface.dwImageHeight - 1 ;
	pnvsih->dwBPP = infSurface.dwBitsPerPixel ;
	pnvsih->dwFlags = SIH_SWAP_EYES ;
	//
	if ( (m_sizeBuffer.w != m_nWidth) || (m_sizeBuffer.h != m_nHeight) )
	{
		pnvsih->dwFlags = SIH_SWAP_EYES | SIH_SCALE_TO_FIT ;
	}
	//
	if ( m_idds9Buf != NULL )
	{
		m_idds9Buf->UnlockRect() ;
	}
	else
	{
		m_iddsBuf->Unlock( NULL ) ;
	}
	//
#if	defined(_DEBUG)
	::QueryPerformanceCounter( (LARGE_INTEGER*) &nEnd ) ;
	::QueryPerformanceFrequency( (LARGE_INTEGER*) &nFreq ) ;
	ESLTrace( "E3DStereoDisplayNVStereoBLT::DrawBuffer : %f [ms]\n",
						((nEnd - nBegin) * 1000.0 / nFreq) ) ;
#endif
	//
	return	errSuccess ;
}

// 表示のための準備処理
//////////////////////////////////////////////////////////////////////////////
E3DSDisplayPlugin::Error E3DStereoDisplayNVStereoBLT::PrepareView( void )
{
	return	errSuccess ;
}

// 表示処理
//////////////////////////////////////////////////////////////////////////////
E3DSDisplayPlugin::Error
	E3DStereoDisplayNVStereoBLT::ViewImage( long int nFlags )
{
	if ( m_pDrawImage == NULL )
	{
		return	errDeviceNotReady ;
	}
	m_id3d9Dev = m_pDrawImage->GetDirect3DDevice9() ;
	//
	// 描画領域パラメータ設定
	//
	int			xDst = 0 ;
	int			yDst = 0 ;
	EGL_SIZE	sizeDst = m_sizeBuffer ;
	EGL_RECT	rectSrc ;
	//
	if ( m_fUseViewPosition )
	{
		xDst = m_xDst ;
		yDst = m_yDst ;
		sizeDst.w = m_nWidth ;
		sizeDst.h = m_nHeight ;
	}
	//
	rectSrc.left = 0 ;
	rectSrc.top = 0 ;
	rectSrc.right = m_sizeBuffer.w * 2 - 1 ;
	rectSrc.bottom = m_sizeBuffer.h ;
	//
	Error	err = errFailed ;
	if ( m_id3d9Dev != NULL )
	{
		//
		// IDirect3DDevice9::StretchRect で描画
		//
		IDirect3DSurface9 *	id3dsBack = NULL ;
		if ( m_id3d9Dev->GetBackBuffer
			( 0, 0, D3DBACKBUFFER_TYPE_MONO, &id3dsBack ) != D3D_OK )
		{
			ESLTrace( "Failed to IDirect3DDevice9::GetBackBuffer.\n" ) ;
			return	errFailed ;
		}
		//
		m_id3d9Dev->Clear
			( 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_ARGB(1,0,0,0), 1.0, 0 ) ;
		//
		if ( m_id3d9Dev->BeginScene() != D3D_OK )
		{
			ESLTrace( "Failed to IDirect3DDevice9::BeginScene.\n" ) ;
		}
		//
		RECT	rcSrc, rcDst ;
		rcSrc.left = rectSrc.left ;
		rcSrc.top = rectSrc.top ;
		rcSrc.right = rectSrc.right + 1 ;
		rcSrc.bottom = rectSrc.bottom + 1 ;
		rcDst.left = xDst ;
		rcDst.top = yDst ;
		rcDst.right = xDst + sizeDst.w ;
		rcDst.bottom = yDst + sizeDst.h ;
		//
		err = errSuccess ;
		//
		if ( m_id3d9Dev->StretchRect
			( m_idds9Buf, &rcSrc,
				id3dsBack, &rcDst, D3DTEXF_NONE ) != D3D_OK )
		{
			ESLTrace( "Failed to IDirect3DDevice9::StretchRect.\n" ) ;
			err = errFailed ;
		}
		if ( m_id3d9Dev->EndScene() != D3D_OK )
		{
			ESLTrace( "Failed to IDirect3DDevice9::EndScene.\n" ) ;
		}
		HRESULT	hr = m_id3d9Dev->Present( NULL, NULL, NULL, NULL ) ;
		//
		id3dsBack->Release() ;
		//
		if ( hr != D3D_OK )
		{
			ESLTrace( "Failed to IDirect3DDevice9::Present.(%08X)\n", hr ) ;
			err = errFailed ;
			//
			if ( (hr == D3DERR_DEVICEREMOVED) || (hr == D3DERR_DEVICELOST) )
			{
				//
				// ロストデバイスの復帰
				//
				ESLError	errReset ;
//				ReleaseSurface() ;
				errReset = m_pDrawImage->ResetDirect3D9Device() ;
//				CreateSurface( m_sizeBuffer.w, m_sizeBuffer.h ) ;
				//
				if ( !errReset &&
					(m_id3d9Dev->GetBackBuffer
						( 0, 0, D3DBACKBUFFER_TYPE_MONO, &id3dsBack ) == D3D_OK) )
				{
					//
					// 描画しなおす
					//
					m_id3d9Dev->Clear
						( 0, NULL, D3DCLEAR_TARGET,
							D3DCOLOR_ARGB(1,0,0,0), 1.0, 0 ) ;
					//
					m_id3d9Dev->BeginScene() ;
					m_id3d9Dev->StretchRect
						( m_idds9Buf, &rcSrc,
							id3dsBack, &rcDst, D3DTEXF_NONE ) ;
					m_id3d9Dev->EndScene() ;
					m_id3d9Dev->Present( NULL, NULL, NULL, NULL ) ;
					//
					id3dsBack->Release() ;
					//
					if ( ::IsWindow( m_hWnd ) )
					{
						::InvalidateRect( m_hWnd, NULL, TRUE ) ;
					}
				}
			}
		}
	}
	else
	{
		//
		// 中間バッファからバックバッファに転送してからフリップ
		//
		//
		IDirectDrawSurface7 *
			iddsBackSuf = m_pDrawImage->GetDD7SecondarySurface() ;
		if ( iddsBackSuf == NULL )
		{
			return	errDeviceNotReady ;
		}
		if ( !m_pDrawImage->BltSurface
			( iddsBackSuf, m_iddsBuf,
				xDst, yDst, &sizeDst, &rectSrc, false ) )
		{
			if ( !m_pDrawImage->Flip() )
			{
				err = errSuccess ;
			}
			else
			{
				ESLTrace( "Faled to Flip\n" ) ;
			}
		}
		else
		{
			ESLTrace( "Faled to BltSurface\n" ) ;
		}
	}
	return	err ;
}

// DirectDraw オブジェクトが削除される前に呼び出される
//////////////////////////////////////////////////////////////////////////////
void E3DStereoDisplayNVStereoBLT::OnReleaseDirectDraw( EGLDrawImage * pdi )
{
	ReleaseSurface() ;
}

// DirectDraw オブジェクトが作成された後に呼び出される
//////////////////////////////////////////////////////////////////////////////
void E3DStereoDisplayNVStereoBLT::OnCreateDirectDraw( EGLDrawImage * pdi )
{
	m_id3d9Dev = m_pDrawImage->GetDirect3DDevice9() ;
	//
	if ( m_fBuffer )
	{
		CreateSurface( m_sizeBuffer.w, m_sizeBuffer.h ) ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// OpenGL ステレオ表示インターフェース
//////////////////////////////////////////////////////////////////////////////

#include <GL/gl.h>
#include <GL/glu.h>

// 構築関数
//////////////////////////////////////////////////////////////////////////////
E3DStereoDisplayOpenGL::E3DStereoDisplayOpenGL( void )
{
	m_hDraw = NULL ;
	m_hDC = NULL ;
	m_hGLRC = NULL ;
	memset( m_infBuf, 0, sizeof(m_infBuf) ) ;
	m_uiTexture[0] = 0 ;
	m_uiTexture[1] = 0 ;
	m_fDynamicDraw = false ;
	m_fStretchDraw = false ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
E3DStereoDisplayOpenGL::~E3DStereoDisplayOpenGL( void )
{
	if ( m_hDraw != NULL )
	{
		m_hDraw->Release() ;
	}
	if ( m_uiTexture[0] != 0 )
	{
		glDeleteTextures( 1, &(m_uiTexture[0]) ) ;
	}
	if ( m_uiTexture[1] != 0 )
	{
		glDeleteTextures( 1, &(m_uiTexture[1]) ) ;
	}
	DeleteImageBuffer() ;
	DeleteGLContext() ;
}

// 画像バッファ生成
//////////////////////////////////////////////////////////////////////////////
void E3DStereoDisplayOpenGL::CreateImageBuffer( int nWidth, int nHeight )
{
	for ( int i = 0; i < E3DSDisplayPlugin::stereoBufferCount; i ++ )
	{
		if ( m_infBuf[i].ptrImageArray != NULL )
		{
			::eslHeapFree( NULL, m_infBuf[i].ptrImageArray ) ;
			m_infBuf[i].ptrImageArray = NULL ;
		}
		m_infBuf[i].dwInfoSize = sizeof(EGL_IMAGE_INFO) ;
		m_infBuf[i].fdwFormatType = EIF_RGBA_BITMAP ;
		m_infBuf[i].dwImageWidth = nWidth ;
		m_infBuf[i].dwImageHeight = nHeight ;
		m_infBuf[i].dwBitsPerPixel = 32 ;
		m_infBuf[i].dwBytesPerLine = nWidth * 4 ;
		m_infBuf[i].dwSizeOfImage = m_infBuf[i].dwBytesPerLine * nHeight ;
		m_infBuf[i].ptrImageArray =
			eslHeapAllocate
				( NULL, m_infBuf[i].dwSizeOfImage, ESL_HEAP_ZERO_INIT ) ;
	}
}

// 画像バッファ削除
//////////////////////////////////////////////////////////////////////////////
void E3DStereoDisplayOpenGL::DeleteImageBuffer( void )
{
	for ( int i = 0; i < E3DSDisplayPlugin::stereoBufferCount; i ++ )
	{
		if ( m_infBuf[i].ptrImageArray != NULL )
		{
			::eslHeapFree( NULL, m_infBuf[i].ptrImageArray ) ;
			m_infBuf[i].ptrImageArray = NULL ;
		}
	}
}

// OpenGL 初期化
//////////////////////////////////////////////////////////////////////////////
E3DSDisplayPlugin::Error E3DStereoDisplayOpenGL::CreateGLContext( void )
{
    static PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),	// Specifies the size of this data structure
		1,								// Specifies the version of this data structure
		PFD_DRAW_TO_WINDOW |			// ピクセルバッファのビットフラグの設定
		PFD_SUPPORT_OPENGL |
		//PFD_SUPPORT_GDI |
		//PFD_SUPPORT_DIRECTDRAW |
		PFD_DOUBLEBUFFER | PFD_STEREO,
		PFD_TYPE_RGBA,					// RGBA pixel values
		32,								// 32-bitカラーと指定
		0, 0, 0, 0, 0, 0,				// Specifies the number of red bitplanes in each RGBA color buffer
		0, 0,							// Specifies the number of alpha bitplanes in each RGBA color buffer
		0, 0, 0, 0, 0,					// Specifies the total number of bitplanes in the accumulation buffer
		32,								// Specifies the depth(bit) of the depth (z-axis) buffer
		32,								// Specifies the depth of the stencil buffer
		0,								// Specifies the number of auxiliary buffers
		PFD_MAIN_PLANE,					// Layer type　Ignored...
		0,								// Specifies the number of overlay and underlay planes
		0,								// Ignored
		0,								// Specifies the transparent color or index of an underlay plane
		0								// Ignored
    } ;
	ESLAssert( ::IsWindow( m_hWnd ) ) ;
	if ( !::IsWindow( m_hWnd ) )
	{
		return	errDeviceNotReady ;
	}
	m_hDC = ::GetDC( m_hWnd ) ;
    int	nPixelFormat = ChoosePixelFormat( m_hDC, &pfd ) ;
	if ( nPixelFormat == 0 )
	{
		ESLTrace( "Failed to OpenGL ChoosePixelFormat.\n" ) ;
		return	errFailed ;
	}
    if ( !SetPixelFormat( m_hDC, nPixelFormat, &pfd ) )
	{
		ESLTrace( "Failed to OpenGL SetPixelFormat.\n" ) ;
		return	errFailed ;
    }
    if ( DescribePixelFormat
		( m_hDC, nPixelFormat, sizeof(PIXELFORMATDESCRIPTOR), &pfd ) == 0 )
	{
		ESLTrace( "Failed to OpenGL DescribePixelFormat.\n" ) ;
		return	errFailed ;
    }
	m_hGLRC = wglCreateContext( m_hDC ) ;
	if ( m_hGLRC == NULL )
	{
		ESLTrace( "Failed to wglCreateContext.\n" ) ;
		return	errFailed ;
	}
	AttachGLCurrent() ;
	if ( IsSupportedStereo() )
	{
		ESLTrace( "Supported OpenGL Quad Buffer.\n" ) ;
	}
	else
	{
		ESLTrace( "Not supported OpenGL Quad Buffer.\n" ) ;
	}
	DetachGLCurrent() ;
	return	errSuccess ;
}

// OpenGL 解放
//////////////////////////////////////////////////////////////////////////////
void E3DStereoDisplayOpenGL::DeleteGLContext( void )
{
	if ( m_hGLRC != NULL )
	{
		wglMakeCurrent( m_hDC, NULL ) ;
		wglDeleteContext( m_hGLRC ) ;
		m_hGLRC = NULL ;
	}
	if ( m_hDC != NULL )
	{
		::ReleaseDC( m_hWnd, m_hDC ) ;
		m_hDC = NULL ;
	}
}

// OpenGL カレント設定
//////////////////////////////////////////////////////////////////////////////
E3DSDisplayPlugin::Error E3DStereoDisplayOpenGL::AttachGLCurrent( void )
{
	if ( (m_hDC == NULL) || (m_hGLRC == NULL) )
	{
		return	errDeviceNotReady ;
	}
	if ( !wglMakeCurrent( m_hDC, m_hGLRC ) )
	{
		ESLTrace( "Failed to wglMakeCurrent as attach to thread.\n" ) ;
		return	errFailed ;
	}
	return	errSuccess ;
}

// OpenGL カレント解除
//////////////////////////////////////////////////////////////////////////////
E3DSDisplayPlugin::Error E3DStereoDisplayOpenGL::DetachGLCurrent( void )
{
	if ( m_hDC == NULL )
	{
		return	errDeviceNotReady ;
	}
	if ( !wglMakeCurrent( m_hDC, NULL ) )
	{
		ESLTrace( "Failed to wglMakeCurrent as detach from thread.\n" ) ;
		return	errFailed ;
	}
	return	errSuccess ;
}

// カメラ設定
//////////////////////////////////////////////////////////////////////////////
void E3DStereoDisplayOpenGL::UpdateCamera( void )
{
	if ( !::IsWindow( m_hWnd ) )
	{
		return ;
	}
	RECT		rctClient ;
	EGL_SIZE	sizeClient ;
	::GetClientRect( m_hWnd, &rctClient ) ;
	sizeClient.w = rctClient.right - rctClient.left ;
	sizeClient.h = rctClient.bottom - rctClient.top ;
	double	xCenter = sizeClient.w * 0.5 ;
	double	yCenter = sizeClient.h * 0.5 ;
	//
	glMatrixMode( GL_PROJECTION ) ;
	glLoadIdentity() ;
	//
	GLfloat	fAspect = (GLfloat)sizeClient.w / (GLfloat) sizeClient.h ;
	gluPerspective( 90, fAspect, 1.0, 10000.0 ) ;
	glViewport( 0, 0, sizeClient.w, sizeClient.h ) ;
	glMatrixMode( GL_MODELVIEW ) ;
	glLoadIdentity() ;
	//
	GLdouble	zCamera = yCenter ;
	GLfloat		xLeft, yUnder ;
	xLeft = (GLfloat) - xCenter ;
	yUnder = (GLfloat) - yCenter ;
	//
	if ( m_fUseViewPosition )
	{
		double	r = 1.0 ;
		xLeft += (GLfloat) (m_xDst * r) ;
		yUnder += (GLfloat) ((sizeClient.h - (m_nHeight + m_yDst)) * r) ;
	}
	gluLookAt( 0, 0, zCamera,  0, 0, 0,  0, 1, 0 ) ;
	glRasterPos3f( xLeft, yUnder, 0 ) ;
}

// テクスチャ設定（伸縮時）
//////////////////////////////////////////////////////////////////////////////
ESLError E3DStereoDisplayOpenGL::UpdateTexture
		( int iSide, int nWidth, int nHeight, void * ptrImageArray )
{
	GLenum	err ;
	if ( m_uiTexture[iSide] == 0 )
	{
		glGenTextures( 1, &(m_uiTexture[iSide]) ) ;
		err = glGetError() ;
		if ( err )
		{
			ESLTrace( "Failed to glGenTextures (%08X)\n", err ) ;
			return	eslErrGeneral ;
		}
	}
	glBindTexture( GL_TEXTURE_2D, m_uiTexture[iSide] ) ;
	err = glGetError() ;
	if ( err )
	{
		ESLTrace( "Failed to glBindTexture (%08X)\n", err ) ;
		return	eslErrGeneral ;
	}
	// texture size must be 2^n
	glTexImage2D
		( GL_TEXTURE_2D, 0, GL_RGB, nWidth, nHeight, 0,
			GL_BGRA_EXT, GL_UNSIGNED_BYTE, ptrImageArray ) ;
	err = glGetError() ;
	if ( err != GL_NO_ERROR )
	{
		ESLTrace( "Failed to glTexImage2D (%08X)\n", err ) ;
		return	eslErrGeneral ;
	}
	return	eslErrSuccess ;
}

// 四角形テクスチャ描画
//////////////////////////////////////////////////////////////////////////////
void E3DStereoDisplayOpenGL::DrawTexturedQuad( int iSide )
{
	RECT		rctClient ;
	EGL_SIZE	sizeClient ;
	::GetClientRect( m_hWnd, &rctClient ) ;
	sizeClient.w = rctClient.right - rctClient.left ;
	sizeClient.h = rctClient.bottom - rctClient.top ;
	double	xCenter = sizeClient.w * 0.5 ;
	double	yCenter = sizeClient.h * 0.5 ;
	//
	GLfloat		xLeft, xRight, yTop, yUnder ;
	xLeft = (GLfloat) - xCenter ;
	xRight = (GLfloat) xCenter ;
	yTop = (GLfloat) yCenter ;
	yUnder = (GLfloat) - yCenter ;
	//
	if ( m_fUseViewPosition )
	{
		double	r = 1.0 ;
		xLeft += (GLfloat) (m_xDst * r) ;
		yUnder += (GLfloat) ((sizeClient.h - (m_nHeight + m_yDst)) * r) ;
		xRight = (GLfloat) (xLeft + m_nWidth) ;
		yTop = (GLfloat) (yUnder + m_nHeight) ;
	}
	//
	GLenum	err ;
	glEnable( GL_TEXTURE_2D ) ;
	//
	glBindTexture( GL_TEXTURE_2D, m_uiTexture[iSide] ) ;
	//
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST ) ;
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST ) ;
	//
	glBegin( GL_QUADS ) ;
	err = glGetError() ;
	if ( err != GL_NO_ERROR )
	{
		ESLTrace( "Failed to glBegin (%08X)\n", err ) ;
	}
	{
		glColor3f( 1.0, 1.0, 1.0 ) ;
		//
		glTexCoord2f( 0.0, 1.0 ) ;
		glVertex3f( xLeft, yTop, 0.0 ) ;
		//
		glTexCoord2f( 1.0, 1.0 ) ;
		glVertex3f( xRight, yTop, 0.0 ) ;
		//
		glTexCoord2f( 1.0, 0.0 ) ;
		glVertex3f( xRight, yUnder, 0.0 ) ;
		//
		glTexCoord2f( 0.0, 0.0 ) ;
		glVertex3f( xLeft, yUnder, 0.0 ) ;
	}
	glEnd( ) ;
	err = glGetError() ;
	if ( err != GL_NO_ERROR )
	{
		ESLTrace( "Failed to glEnd (%08X)\n", err ) ;
	}
}

// ビューの振る舞い
//////////////////////////////////////////////////////////////////////////////
long int E3DStereoDisplayOpenGL::Behavior( void )
{
	return	0 ;
}

// 受け取り可能な画像バッファの種類
//////////////////////////////////////////////////////////////////////////////
long int E3DStereoDisplayOpenGL::ImageBufferCapacity( void )
{
	return	flagMemoryImage ;
}

// 最適な画像バッファの種類
//////////////////////////////////////////////////////////////////////////////
long int E3DStereoDisplayOpenGL::BestImageBufferCapacity( void )
{
	return	flagMemoryImage ;
}

// ウィンドウに関連付け
//////////////////////////////////////////////////////////////////////////////
E3DSDisplayPlugin::Error E3DStereoDisplayOpenGL::AttachWindow( HWND hWnd )
{
	E3DSDisplayPlugin::Error
		err = E3DStereoDisplayInterface::AttachWindow( hWnd ) ;
	if ( err )
	{
		return	err ;
	}
	return	CreateGLContext() ;
}

// ウィンドウから分離
//////////////////////////////////////////////////////////////////////////////
E3DSDisplayPlugin::Error E3DStereoDisplayOpenGL::DetachWindow( void )
{
	DeleteGLContext() ;
	//
	return	E3DStereoDisplayInterface::DetachWindow() ;
}

// 画像バッファのサイズを設定する
//////////////////////////////////////////////////////////////////////////////
E3DSDisplayPlugin::Error E3DStereoDisplayOpenGL::SetBufferSize
	( long int nFlags, long int nFormat,
				int nWidth, int nHeight, int nViewCount )
{
	CreateImageBuffer( nWidth, nHeight ) ;
	return	errSuccess ;
}

// 画面モード変更時の処理
//////////////////////////////////////////////////////////////////////////////
E3DSDisplayPlugin::DisplayModeMethod
		E3DStereoDisplayOpenGL::OnChangeDisplayMode
	( HMONITOR hMonitor,
		int nWidth, int nHeight, int nBitsPerPixel, int nFrequency )
{
	return	methodNoModeChanged ;
}

// 画面モード復帰時の処理
//////////////////////////////////////////////////////////////////////////////
E3DSDisplayPlugin::DisplayModeMethod
	E3DStereoDisplayOpenGL::OnRestoreDisplayMode( void )
{
	return	methodNoModeChanged ;
}

// 画像バッファに描画する
//////////////////////////////////////////////////////////////////////////////
E3DSDisplayPlugin::Error E3DStereoDisplayOpenGL::DrawBuffer
	( long int nFlags, int x, int y,
		const E3DSDisplayPlugin::ImageBuffer * bufImage[], int nViewCount )
{
	if ( (nViewCount <= 0) || (nViewCount > 2) )
	{
		return	errInvalidParameter ;
	}
	int	i ;
	m_fDynamicDraw = true ;
	m_fStretchDraw =
		(m_infBuf[0].dwImageWidth != (DWORD) m_nWidth)
			|| (m_infBuf[0].dwImageHeight != (DWORD) m_nHeight) ;
	if ( (x == 0) && (y == 0)
		&& ((nFlags & (drawDynamic | drawTemporary))
							== (drawDynamic | drawTemporary)) )
	{
		for ( i = 0; i < nViewCount; i ++ )
		{
			if ( bufImage[i]->nFlags
					& E3DSDisplayPlugin::flagSourceRectangle )
			{
				if ( (bufImage[i]->rctSource.x != 0)
					|| (bufImage[i]->rctSource.y != 0)
					|| (bufImage[i]->rctSource.w != bufImage[i]->nWidth)
					|| (bufImage[i]->rctSource.h != bufImage[i]->nHeight) )
				{
					m_fDynamicDraw = false ;
					break ;
				}
			}
			if ( (bufImage[i]->nBitsPerPixel != 32)
				|| (bufImage[i]->nBytesPerLine != bufImage[i]->nWidth * -4)
				|| (bufImage[i]->nWidth
						!= (long int) m_infBuf[i].dwImageWidth)
				|| (bufImage[i]->nHeight
						!= (long int) m_infBuf[i].dwImageHeight) )
			{
				m_fDynamicDraw = false ;
				break ;
			}
		}
	}
	else
	{
		m_fDynamicDraw = false ;
	}
#if	defined(_DEBUG)
	INT64	nBegin, nEnd, nFreq ;
	::QueryPerformanceCounter( (LARGE_INTEGER*) &nBegin ) ;
#endif
	if ( m_fDynamicDraw )
	{
		//
		// 直接バックバッファへ書き込む
		//
		E3DSDisplayPlugin::Error	err = AttachGLCurrent() ;
		if ( err )
		{
			return	err ;
		}
		glDrawBuffer( GL_BACK ) ;
		glClearColor( 0, 0, 0, 0 ) ;
		glClear( GL_COLOR_BUFFER_BIT ) ;
		//
		UpdateCamera() ;
		//
		static const GLenum	glenDrawTarget[] =
		{
			GL_BACK_RIGHT, GL_BACK_LEFT
		} ;
		for ( int i = 0; i < E3DSDisplayPlugin::stereoBufferCount; i ++ )
		{
			const E3DSDisplayPlugin::ImageBuffer *	pbufImage ;
			if ( i < nViewCount )
			{
				pbufImage  = bufImage[i] ;
			}
			else
			{
				pbufImage  = bufImage[nViewCount - 1] ;
			}
			BYTE *	pbytBits = (BYTE*) pbufImage->pBuffer ;
			ESLAssert( pbufImage->nBytesPerLine < 0 ) ;
			ESLAssert( pbufImage->nBytesPerLine == pbufImage->nWidth * -4 ) ;
			pbytBits += pbufImage->nBytesPerLine
								* (pbufImage->nHeight - 1) ;
			glDrawBuffer( glenDrawTarget[i] ) ;
			//
			bool	fTexture = false ;
			if ( m_fStretchDraw )
			{
				if ( !UpdateTexture
						( i, pbufImage->nWidth,
								pbufImage->nHeight, pbytBits ) )
				{
					DrawTexturedQuad( i ) ;
					fTexture = true ;
				}
			}
			if ( !fTexture )
			{
				glDrawPixels
					( pbufImage->nWidth, pbufImage->nHeight,
						GL_BGRA_EXT, GL_UNSIGNED_BYTE, pbytBits ) ;
			}
//			glFlush() ;
		}
		DetachGLCurrent() ;
	}
	else
	{
		//
		// 中間バッファへ書き出す
		//
		if ( m_hDraw == NULL )
		{
			m_hDraw = ::eglCreateDrawImage() ;
		}
		EGL_DRAW_PARAM	dp ;
		memset( &dp, 0, sizeof(dp) ) ;
		dp.dwFlags = 0 ;
		dp.ptBasePos.x = x ;
		dp.ptBasePos.y = y ;

		for ( i = 0; i < E3DSDisplayPlugin::stereoBufferCount; i ++ )
		{
			const E3DSDisplayPlugin::ImageBuffer *	pbufImage ;
			if ( i < nViewCount )
			{
				pbufImage  = bufImage[i] ;
			}
			else
			{
				pbufImage  = bufImage[nViewCount - 1] ;
			}
			EGL_IMAGE_INFO	imginf ;
			ConvertFromE3DSDisplayImageBuffer( imginf, *pbufImage ) ;
			::eglReverseVertically( &imginf ) ;
			dp.pSrcImage = &imginf ;
			//
			m_hDraw->Initialize( &m_infBuf[i], NULL, NULL ) ;
			//
			if ( !m_hDraw->PrepareDraw( &dp ) )
			{
				m_hDraw->DrawImage() ;
			}
		}
	}
#if	defined(_DEBUG)
	::QueryPerformanceCounter( (LARGE_INTEGER*) &nEnd ) ;
	::QueryPerformanceFrequency( (LARGE_INTEGER*) &nFreq ) ;
	ESLTrace( "E3DStereoDisplayOpenGL::DrawBuffer as %s : %f [ms]\n",
				(m_fDynamicDraw ? "dynamic" : "static"),
						((nEnd - nBegin) * 1000.0 / nFreq) ) ;
#endif
	return	errSuccess ;
}

// 表示のための準備処理
//////////////////////////////////////////////////////////////////////////////
E3DSDisplayPlugin::Error E3DStereoDisplayOpenGL::PrepareView( void )
{
	return	errSuccess ;
}

// 表示処理
//////////////////////////////////////////////////////////////////////////////
E3DSDisplayPlugin::Error E3DStereoDisplayOpenGL::ViewImage( long int nFlags )
{
#if	defined(_DEBUG)
	INT64	nBegin, nEnd, nFreq ;
	::QueryPerformanceCounter( (LARGE_INTEGER*) &nBegin ) ;
#endif

	if ( !m_fDynamicDraw )
	{
		E3DSDisplayPlugin::Error	err = AttachGLCurrent() ;
		if ( err )
		{
			return	err ;
		}
		glDrawBuffer( GL_BACK ) ;
		glClearColor( 0, 0, 0, 0 ) ;
		glClear( GL_COLOR_BUFFER_BIT ) ;
		//
		UpdateCamera() ;
		//
		static const GLenum	glenDrawTarget[] =
		{
			GL_BACK_RIGHT, GL_BACK_LEFT
		} ;
		for ( int i = 0; i < E3DSDisplayPlugin::stereoBufferCount; i ++ )
		{
			glDrawBuffer( glenDrawTarget[i] ) ;
			//
			bool	fTexture = false ;
			if ( m_fStretchDraw )
			{
				if ( !UpdateTexture
						( i, m_infBuf[i].dwImageWidth,
								m_infBuf[i].dwImageHeight,
								m_infBuf[i].ptrImageArray ) )
				{
					DrawTexturedQuad( i ) ;
					fTexture = true ;
				}
			}
			if ( !fTexture )
			{
				glDrawPixels
					( m_infBuf[i].dwImageWidth, m_infBuf[i].dwImageHeight,
						GL_BGRA_EXT, GL_UNSIGNED_BYTE, m_infBuf[i].ptrImageArray ) ;
			}
//			glFlush() ;
		}

		DetachGLCurrent() ;
	}

//	SwapBuffers( m_hDC ) ;

	if ( !wglSwapLayerBuffers( m_hDC, WGL_SWAP_MAIN_PLANE ) )
	{
		ESLTrace( "Failed to wglSwapLayerBuffers.\n" ) ;
	}
	m_fDynamicDraw = false ;

#if	defined(_DEBUG)
	::QueryPerformanceCounter( (LARGE_INTEGER*) &nEnd ) ;
	::QueryPerformanceFrequency( (LARGE_INTEGER*) &nFreq ) ;
	ESLTrace( "OpenGL glDrawPixels : %f [ms]\n",
						((nEnd - nBegin) * 1000.0 / nFreq) ) ;
#endif

	return	errSuccess ;
}

// OpenGL Quad Buffer サポートテスト
//////////////////////////////////////////////////////////////////////////////
bool E3DStereoDisplayOpenGL::IsSupportedStereo( void )
{
	GLboolean	boolTest = FALSE ;
	glGetBooleanv( GL_STEREO, &boolTest ) ;
	GLenum	err = glGetError() ;
	if ( err )
	{
		ESLTrace( "Failed to glGetBooleanv(GL_STEREO) (%08X)\n", err ) ;
		return	false ;
	}
	return	(boolTest != 0) ;
}
