
#if	!defined(__SAKURAGL_GDI_WINDOW_PROCEDURE_H__)
#define	__SAKURAGL_GDI_WINDOW_PROCEDURE_H__	1

#include <sakuragl/window/sgl_srd_window_producer.h>
#include <sakuragl/sgl3d/sgl_render_software_renderer.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// GDI ウィンドウ・表示インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SGLWin32GDIWindowProducer	: public SGLSoftwareRenderWindowProducer
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO
			( SGLWin32GDIWindowProducer, SGLSoftwareRenderWindowProducer )

	public:	// SGLWindowViewProducer 実装
		// 表示バッファのフリップ処理
		virtual void FlipView
			( SGLAbstractWindow * pWnd, bool fVSync, bool fOnWinThread ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// Direct3D9 表示インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SGLWin32D3D9WindowProducer	: public SGLSoftwareRenderWindowProducer
	{
	protected:
		static HMODULE					s_hModuleD3D9 ;
		HWND							m_hWnd ;
		struct IDirect3D9 *				m_id3d9 ;
		struct IDirect3DDevice9 *		m_id3d9Dev ;
		struct _D3DPRESENT_PARAMETERS_ *m_pd3dpp ;
		struct IDirect3DSurface9 *		m_idds9DispBuf ;
		SGLSize							m_sizeD3D9DispBuf ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO
			( SGLWin32D3D9WindowProducer, SGLSoftwareRenderWindowProducer )
		// 構築関数
		SGLWin32D3D9WindowProducer( void ) ;
		// 消滅関数
		virtual ~SGLWin32D3D9WindowProducer( void ) ;

	public:	// SGLWindowViewProducer 実装
		// 物理ビューサイズ通知
		virtual void OnChangePhysicalViewSize
			( SGLAbstractWindow * pWnd, uint32_t nWidth, uint32_t nHeight ) ;
		// ウィンドウに関連付けられた（作成された）
		virtual void OnAttachedWindow( SGLAbstractWindow * pWnd ) ;
		// ウィンドウから分離された（ウィンドウが破棄される）
		virtual void OnDetachedWindow( SGLAbstractWindow * pWnd ) ;
		// 表示バッファのフリップ処理
		virtual void FlipView
			( SGLAbstractWindow * pWnd, bool fVSync, bool fOnWinThread ) ;

	protected:
		// Direct3D9 初期化
		SGLError CreateD3D9Device
			( HWND hWnd, int nWidth, int nHeight,
					int nAdapter = 0, BOOL fWindowed = TRUE ) ;
		// Direct3D9 開放
		void ReleaseD3D9Device( void ) ;
		// Direct3D9 リセット
		SGLError ResetD3D9Device( int nWidth, int nHeight ) ;
		// サーフェースを開放する
		SGLError ReleaseD3D9Surface( void ) ;
		// 描画
		SGLError DrawImageToD3D9( SGLImageObject * pImage, int xDst, int yDst ) ;
	} ;

}

#endif

