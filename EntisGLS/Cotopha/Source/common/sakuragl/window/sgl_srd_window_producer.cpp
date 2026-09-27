
#include <sakuragl/sakuragl.h>
#include <sakuragl/sgl_window.h>
#include <sakuragl/window/sgl_srd_window_producer.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// ソフトウェア描画 ウィンドウ・表示インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::SGLSoftwareRenderWindowProducer,
			 SGLSoftwareRenderDevice, SGLWindowViewProducer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSoftwareRenderWindowProducer::SGLSoftwareRenderWindowProducer( void )
	: m_sizeVirtual(1,1), m_sizePhysical(1,1), m_rectVirtual(0,0,1,1)
{
	m_flagEnableZ = false ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSoftwareRenderWindowProducer::~SGLSoftwareRenderWindowProducer( void )
{
}

// 表示領域更新
//////////////////////////////////////////////////////////////////////////////
void SGLSoftwareRenderWindowProducer::UpdateVirtualViewRect( void )
{
	if ( m_sizePhysical.IsEmpty() )
	{
		return ;
	}
	if ( m_sizePhysical.w * m_sizeVirtual.h
					<= m_sizePhysical.h * m_sizeVirtual.w )
	{
		m_rectVirtual.w = m_sizePhysical.w ;
		if ( m_sizeVirtual.w != 0 )
		{
			m_rectVirtual.h =
				m_sizeVirtual.h * m_sizePhysical.w / m_sizeVirtual.w ;
		}
		else
		{
			m_rectVirtual.h = m_sizePhysical.h ;
		}
		m_rectVirtual.x = 0 ;
		m_rectVirtual.y = (m_sizePhysical.h - m_rectVirtual.h) / 2 ;
	}
	else
	{
		m_rectVirtual.h = m_sizePhysical.h ;
		if ( m_sizeVirtual.h != 0 )
		{
			m_rectVirtual.w =
				m_sizeVirtual.w * m_sizePhysical.h / m_sizeVirtual.h ;
		}
		else
		{
			m_rectVirtual.w = m_sizePhysical.w ;
		}
		m_rectVirtual.y = 0 ;
		m_rectVirtual.x = (m_sizePhysical.w - m_rectVirtual.w) / 2 ;
	}
}

// 対応機能フラグ
//////////////////////////////////////////////////////////////////////////////
uint64_t SGLSoftwareRenderWindowProducer::GetCapacityFlags( void ) const
{
	return	renderableAnyThread ;
}

// 論理ビューサイズ通知
//////////////////////////////////////////////////////////////////////////////
void SGLSoftwareRenderWindowProducer::OnChangeVirtualViewSize
	( SGLAbstractWindow * pWnd, uint32_t nWidth, uint32_t nHeight )
{
	SGLSize	sizeView( nWidth, nHeight ) ;
	pWnd->Lock() ;
	if ( m_imgVirtual.GetImageSize() != sizeView )
	{
		m_imgVirtual.CreateImage
			( nWidth, nHeight, formatImageARGB, 32 ) ;
		m_imgVirtualZ.CreateImage
			( nWidth, nHeight, formatImageZ, 32 ) ;
	}
	m_sizeVirtual = sizeView ;
	UpdateVirtualViewRect() ;
	pWnd->Unlock() ;
}

// 物理ビューサイズ通知
//////////////////////////////////////////////////////////////////////////////
void SGLSoftwareRenderWindowProducer::OnChangePhysicalViewSize
	( SGLAbstractWindow * pWnd, uint32_t nWidth, uint32_t nHeight )
{
	SGLSize	sizeView( nWidth, nHeight ) ;
	pWnd->Lock() ;
	if ( m_imgPhysical.GetImageSize() != sizeView )
	{
		m_imgPhysical.CreateImage
			( nWidth, nHeight, formatImageARGB, 32 ) ;
		m_imgPhysicalZ.CreateImage
			( nWidth, nHeight, formatImageZ, 32 ) ;
	}
	m_sizePhysical = sizeView ;
	UpdateVirtualViewRect() ;
	pWnd->Unlock() ;
}

// ウィンドウに関連付けられた（作成された）
//////////////////////////////////////////////////////////////////////////////
void SGLSoftwareRenderWindowProducer::OnAttachedWindow( SGLAbstractWindow * pWnd )
{
	sglSetDefaultImageFormat( formatImageARGB, 32 ) ;
	S3DRenderContextInterface::SetDefaultRenderType( typePaintEntisGLS4 ) ;
}

// ウィンドウから分離された（ウィンドウが破棄される）
//////////////////////////////////////////////////////////////////////////////
void SGLSoftwareRenderWindowProducer::OnDetachedWindow( SGLAbstractWindow * pWnd )
{
}

// ウィンドウの位置が変化した
//////////////////////////////////////////////////////////////////////////////
void SGLSoftwareRenderWindowProducer::OnMovedWindow( SGLAbstractWindow * pWnd )
{
}

// フルスクリーンモードへ変更する
//////////////////////////////////////////////////////////////////////////////
bool SGLSoftwareRenderWindowProducer::OnChangeFullscreen
	( SGLAbstractWindow * pWnd,
		uint32_t nBitsPerPixel, uint32_t nFrequency,
		bool flagChangePhysicalMode, const wchar_t * pszDisplayName )
{
	return	false ;
}

// フルスクリーンモードから復帰する
//////////////////////////////////////////////////////////////////////////////
void SGLSoftwareRenderWindowProducer::OnRestoreFullscreen( SGLAbstractWindow * pWnd )
{
}

// 論理ビュー表示座標取得
//////////////////////////////////////////////////////////////////////////////
void SGLSoftwareRenderWindowProducer::GetInternalViewPosition( SGLImageRect& rctVirtualView )
{
	rctVirtualView = m_rectVirtual ;
}

// 物理ビュー表示領域取得
//////////////////////////////////////////////////////////////////////////////
bool SGLSoftwareRenderWindowProducer::GetExternalViewPosition( SGLImageRect& rctPhysicalView )
{
	return	false ;
}

// 論理座標→物理ビュー座標変換行列取得
//////////////////////////////////////////////////////////////////////////////
void SGLSoftwareRenderWindowProducer::GetAffineVirtualToPhysical( SGLAffine& affine )
{
	affine.a11 = (float32_t) m_rectVirtual.w / (float32_t) m_sizeVirtual.w ;
	affine.a12 = 0.0f ;
	affine.a13 = (float32_t) m_rectVirtual.x ;
	affine.a21 = 0.0f ;
	affine.a22 = (float32_t) m_rectVirtual.h / (float32_t) m_sizeVirtual.h ;
	affine.a23 = (float32_t) m_rectVirtual.y ;
}

// 描画スレッドの関連付け
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSoftwareRenderWindowProducer::AttachViewThread( SGLAbstractWindow * pWnd )
{
	return	sglErrSuccess ;
}

// 描画スレッドの関連付け解除
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSoftwareRenderWindowProducer::DetachViewThread( SGLAbstractWindow * pWnd )
{
	return	sglErrSuccess ;
}

// 描画ハンドラ開始
//////////////////////////////////////////////////////////////////////////////
RenderContext * SGLSoftwareRenderWindowProducer::BeginDrawView
		( SGLAbstractWindow * pWnd,
			bool fOnWinThread,
			const SGLImageRect * pWindow,
			SGLImageObject * pImage,
			SGLImageObject * pZBuffer,
			SGLImageObject * pImageLeft,
			SGLImageObject * pZBufferLeft )
{
	if ( pImage == NULL )
	{
		if ( m_sizeVirtual == m_sizePhysical )
		{
			pImage = &m_imgPhysical ;
			pZBuffer = &m_imgPhysicalZ ;
		}
		else
		{
			pImage = &m_imgVirtual ;
			pZBuffer = &m_imgVirtualZ ;
		}
	}
	if ( !m_flagEnableZ )
	{
		pZBuffer = NULL ;
		pZBufferLeft = NULL ;
	}
	m_render.AttachTargetImage( pImage, pZBuffer, NULL ) ;
	m_render.ResetTransformation() ;
	return	&m_render ;
}

// 描画ハンドラ終了
//////////////////////////////////////////////////////////////////////////////
void SGLSoftwareRenderWindowProducer::EndDrawView
	( SGLAbstractWindow * pWnd,
			RenderContext * render, bool fOnWinThread )
{
	m_render.DetachTargetImage() ;
	//
	if ( m_sizeVirtual != m_sizePhysical )
	{
		SGLPaintParam	pp ;
		SGLAffine		affine ;
		GetAffineVirtualToPhysical( affine ) ;
		pp.pAffine = &affine ;
		//
		m_paint.AttachTargetImage( &m_imgPhysical, NULL ) ;
		m_paint.FillClearTarget( 0 ) ;
		m_paint.DrawImage( pp, &m_imgVirtual ) ;
		m_paint.DetachTargetImage() ;
	}
}

// 直接描画ハンドラ開始
//////////////////////////////////////////////////////////////////////////////
RenderContext * SGLSoftwareRenderWindowProducer::BeginDirectView
		( SGLAbstractWindow * pWnd,
			bool fOnWinThread,
			const SGLImageRect * pWindow,
			SGLImageObject * pImage,
			SGLImageObject * pZBuffer,
			SGLImageObject * pImageLeft,
			SGLImageObject * pZBufferLeft )
{
	if ( pImage == NULL )
	{
		pImage = &m_imgPhysical ;
		pZBuffer = &m_imgPhysicalZ ;
	}
	if ( !m_flagEnableZ )
	{
		pZBuffer = NULL ;
		pZBufferLeft = NULL ;
	}
	m_render.AttachTargetImage( pImage, pZBuffer, NULL ) ;
	if ( m_flagEnableZ )
	{
		m_render.FillClearTarget
			( 0, SGLPaintContextInterface::clearTargetZBuffer ) ;
	}
	m_render.ResetTransformation() ;
	return	&m_render ;
}

// 直接描画ハンドラ終了
//////////////////////////////////////////////////////////////////////////////
void SGLSoftwareRenderWindowProducer::EndDirectView
	( SGLAbstractWindow * pWnd,
			RenderContext * render, bool fOnWinThread )
{
	m_render.DetachTargetImage() ;
}

// 表示バッファのフリップ処理
//////////////////////////////////////////////////////////////////////////////
void SGLSoftwareRenderWindowProducer::FlipView
	( SGLAbstractWindow * pWnd, bool fVSync, bool fOnWinThread )
{
}

// ｚバッファ設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSoftwareRenderWindowProducer::EnableZBuffer
	( SGLAbstractWindow * pWnd, bool flagZBuffer )
{
	m_flagEnableZ = flagZBuffer ;
	return	sglErrSuccess ;
}

// レイヤードウィンドウ設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSoftwareRenderWindowProducer::EnableLayeredWindow
	( SGLAbstractWindow * pWnd, bool flagLayeredWindow )
{
	return	sglErrSuccess ;
}

// ステレオ立体視モード設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSoftwareRenderWindowProducer::SetStereoDisplayMode
	( SGLAbstractWindow * pWnd,
		const wchar_t * pszMethodID, uint64_t nParam )
{
	if ( SString::Compare
		( pszMethodID, SGLAbstractWindow::Stereo3D::MonoView ) == 0 )
	{
		return	sglErrSuccess ;
	}
	return	sglErrNotSupported ;
}

// ステレオ立体視モードか？
//////////////////////////////////////////////////////////////////////////////
bool SGLSoftwareRenderWindowProducer::IsStereoDisplayMode( void )
{
	return	false ;
}

// ステレオ立体視モードテスト
//////////////////////////////////////////////////////////////////////////////
bool SGLSoftwareRenderWindowProducer::IsSupportedStereoDisplayMode
	( SGLAbstractWindow * pWnd, const wchar_t * pszMethodID )
{
	if ( SString::Compare
		( pszMethodID, SGLAbstractWindow::Stereo3D::MonoView ) == 0 )
	{
		return	true ;
	}
	return	false ;
}

// ビューサイズ取得（side by side 表示時のウィンドウサイズ調整用）
//////////////////////////////////////////////////////////////////////////////
SGLSize SGLSoftwareRenderWindowProducer::GetStandardDisplaySize( void ) const
{
	return	m_sizeVirtual ;
}

// レンダリングデバイス取得
//////////////////////////////////////////////////////////////////////////////
S3DRenderDevice * SGLSoftwareRenderWindowProducer::GetRenderDevice( void )
{
	return	this ;
}


