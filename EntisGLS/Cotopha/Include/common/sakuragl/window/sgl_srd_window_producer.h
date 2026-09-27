
#if	!defined(__SAKURAGL_SRD_WINDOW_PROCEDURE_H__)
#define	__SAKURAGL_SRD_WINDOW_PROCEDURE_H__	1

#include <sakuragl/window/sgl_window_producer.h>
#include <sakuragl/sgl3d/sgl_render_software_renderer.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// ソフトウェア描画 ウィンドウ・表示インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SGLSoftwareRenderWindowProducer
				: public SGLSoftwareRenderDevice, public SGLWindowViewProducer
	{
	protected:
		SGLImage					m_imgVirtual ;	// 仮想ディスプレイ
		SGLImage					m_imgVirtualZ ;
		SGLImage					m_imgPhysical ;	// 物理ディスプレイ
		SGLImage					m_imgPhysicalZ ;
		SGLSize						m_sizeVirtual ;
		SGLSize						m_sizePhysical ;
		SGLImageRect				m_rectVirtual ;	// 仮想ディスプレイ表示域
		bool						m_flagEnableZ ;	// ｚバッファ有効
		SGLPaintBuffer				m_paint ;		// 仮想ディスプレイ伸縮描画用
		S3DSoftwareBufferedRenderer	m_render ;		// 描画用オブジェクト

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
			( SGLSoftwareRenderWindowProducer, SGLSoftwareRenderDevice, SGLWindowViewProducer )
		// 構築関数
		SGLSoftwareRenderWindowProducer( void ) ;
		// 消滅関数
		virtual ~SGLSoftwareRenderWindowProducer( void ) ;
		// 表示領域更新
		void UpdateVirtualViewRect( void ) ;

	public:	// SGLWindowViewProducer 実装
		// 対応機能フラグ
		virtual uint64_t GetCapacityFlags( void ) const ;
		// 論理ビューサイズ通知
		virtual void OnChangeVirtualViewSize
			( SGLAbstractWindow * pWnd, uint32_t nWidth, uint32_t nHeight ) ;
		// 物理ビューサイズ通知
		virtual void OnChangePhysicalViewSize
			( SGLAbstractWindow * pWnd, uint32_t nWidth, uint32_t nHeight ) ;
		// ウィンドウに関連付けられた（作成された）
		virtual void OnAttachedWindow( SGLAbstractWindow * pWnd ) ;
		// ウィンドウから分離された（ウィンドウが破棄される）
		virtual void OnDetachedWindow( SGLAbstractWindow * pWnd ) ;
		// ウィンドウの位置が変化した
		virtual void OnMovedWindow( SGLAbstractWindow * pWnd ) ;
		// フルスクリーンモードへ変更する
		virtual bool OnChangeFullscreen
			( SGLAbstractWindow * pWnd,
				uint32_t nBitsPerPixel, uint32_t nFrequency,
				bool flagChangePhysicalMode, const wchar_t * pszDisplayName ) ;
		// フルスクリーンモードから復帰する
		virtual void OnRestoreFullscreen( SGLAbstractWindow * pWnd ) ;
		// 論理ビュー表示座標取得
		virtual void GetInternalViewPosition( SGLImageRect& rctVirtualView ) ;
		// 物理ビュー表示領域取得
		virtual bool GetExternalViewPosition( SGLImageRect& rctPhysicalView ) ;
		// 論理座標→物理ビュー座標変換行列取得
		virtual void GetAffineVirtualToPhysical( SGLAffine& affine ) ;
		// 描画スレッドの関連付け
		virtual SGLError AttachViewThread( SGLAbstractWindow * pWnd ) ;
		// 描画スレッドの関連付け解除
		virtual SGLError DetachViewThread( SGLAbstractWindow * pWnd ) ;
		// 描画ハンドラ開始
		virtual RenderContext * BeginDrawView
				( SGLAbstractWindow * pWnd,
					bool fOnWinThread,
					const SGLImageRect * pWindow = NULL,
					SGLImageObject * pImage = NULL,
					SGLImageObject * pZBuffer = NULL,
					SGLImageObject * pImageLeft = NULL,
					SGLImageObject * pZBufferLeft = NULL ) ;
		// 描画ハンドラ終了
		virtual void EndDrawView
			( SGLAbstractWindow * pWnd,
					RenderContext * render, bool fOnWinThread ) ;
		// 直接描画ハンドラ開始
		virtual RenderContext * BeginDirectView
				( SGLAbstractWindow * pWnd,
					bool fOnWinThread,
					const SGLImageRect * pWindow = NULL,
					SGLImageObject * pImage = NULL,
					SGLImageObject * pZBuffer = NULL,
					SGLImageObject * pImageLeft = NULL,
					SGLImageObject * pZBufferLeft = NULL ) ;
		// 直接描画ハンドラ終了
		virtual void EndDirectView
			( SGLAbstractWindow * pWnd,
					RenderContext * render, bool fOnWinThread ) ;
		// 表示バッファのフリップ処理
		virtual void FlipView
			( SGLAbstractWindow * pWnd, bool fVSync, bool fOnWinThread ) ;
		// ｚバッファ設定
		virtual SGLError EnableZBuffer
			( SGLAbstractWindow * pWnd, bool flagZBuffer ) ;
		// レイヤードウィンドウ設定
		virtual SGLError EnableLayeredWindow
			( SGLAbstractWindow * pWnd, bool flagLayeredWindow ) ;
		// ステレオ立体視モード設定
		virtual SGLError SetStereoDisplayMode
			( SGLAbstractWindow * pWnd,
				const wchar_t * pszMethodID, uint64_t nParam = 0 ) ;
		// ステレオ立体視モードか？
		virtual bool IsStereoDisplayMode( void ) ;
		// ステレオ立体視モードテスト
		virtual bool IsSupportedStereoDisplayMode
			( SGLAbstractWindow * pWnd, const wchar_t * pszMethodID ) ;
		// ビューサイズ取得（side by side 表示時のウィンドウサイズ調整用）
		virtual SGLSize GetStandardDisplaySize( void ) const ;
		// レンダリングデバイス取得
		virtual S3DRenderDevice * GetRenderDevice( void ) ;

	} ;

}

#endif

