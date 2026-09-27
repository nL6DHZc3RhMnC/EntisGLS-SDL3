
#if	!defined(__SAKURAGL_VR_GOGGLE_PRODUCER_H__)
#define	__SAKURAGL_VR_GOGGLE_PRODUCER_H__	1

#include <sakuragl/sgl_vr_view_producer.h>
#include <sakuraglx/sglx_platform_ui.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// VR ゴーグル用 SideBySide 出力 HMD インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SGLSideBySideVRGoggleProducer
				: public SGLVRViewProducer,
					public SSystem::SProcedure,
					public SGLWindowViewSynchronizer
	{
	public:
		struct	LensDistortion
		{
			float32_t	fpDistortion[4] ;	// k = d0 + d1*x^2 + d2*x^4 + d3*x^6
			float32_t	fpLensScale ;
			float32_t	fpOffsetX ;
		} ;

	protected:
		SGLAbstractWindow *		m_pWindow ;
		UI::SGLPostureSensor	m_posture ;
		S3DDMatrix				m_matBasePose ;
		S3DDVector				m_vBasePosition ;

		bool					m_flagChangeMode ;
		bool					m_flagPollingThread ;
		volatile bool			m_flagQuitPolling ;
		SSystem::SThread		m_threadPolling ;
		SSystem::SSignalEvent	m_signalInitThread ;
		SSystem::SSignalEvent	m_signalDevPolled ;
		SSystem::SSignalEvent	m_signalFrameUpdate ;

		bool					m_flagNoPosition ;

		LensDistortion			m_lensDistortion ;

		float32_t				m_degHorzFOV ;
		float32_t				m_xParallax ;
		float32_t				m_zParallaxFocus ;

		SGLSize					m_sizeOrgDisplay ;
		SGLSize					m_sizePhysDisplay ;
		SGLSize					m_sizeLogicalView ;

		double					m_msecFrameInterval ;
		bool					m_flagPostUpdateFrame ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO3
			( SGLSideBySideVRGoggleProducer,
				SGLVRViewProducer, SProcedure, SGLWindowViewSynchronizer )
		// 構築関数
		SGLSideBySideVRGoggleProducer( void ) ;

	public:
		// 加速度センサでHMD座標を計算するか
		void EnableHMDPositionByAccelerometer( bool fEnable ) ;
		bool IsEnabledHMDPositionByAccelerometer( void ) const ;
		// フレーム更新インターバル
		void SetFrameInterval( double msecInterval, bool fPostUpdate ) ;
		double GetFrameInterval( void ) const ;

	public:
		// レンズ設定
		void SetLensDistortion( const LensDistortion& ld ) ;
		void GetLensDistortion( LensDistortion& ld ) const ;
	protected:
		void UpdateLensDistortion( void ) ;

	public:
		// 視野角（水平）[deg]
		void SetFieldOfView( float32_t degHFOV ) ;
		float32_t GetFieldOfView( void ) const ;
		// 視差設定（目間距離の半分）
		void SetParallax( float32_t xParallax ) ;
		float32_t GetParallax( void ) const ;
		// 視差焦点設定（0.0fの時平行）
		void SetParallaxFocus( float32_t zFocus ) ;
		float32_t GetParallaxFocus( void ) const ;
	protected:
		void UpdateEyePostures( void ) ;
		void UpdateEyeFieldOfView( void ) ;

	public:
		// 開始
		SGLError BeginHMDMode( SGLAbstractWindow * pWnd ) ;
		// 終了
		SGLError EndHMDMode( void ) ;

	public:	// SGLSecondaryViewProducer 実装
		// 描画ハンドラ開始
		virtual RenderContext * BeginDrawView
			( SGLAbstractWindow * pPrimaryWnd ) ;
		// 描画ハンドラ終了
		virtual void EndDrawView
			( SGLAbstractWindow * pPrimaryWnd, RenderContext * render ) ;
		// 表示バッファのフリップ処理
		virtual void FlipView
			( SGLAbstractWindow * pPrimaryWnd, bool fVSync ) ;
		// ステレオ立体視モードか？
		virtual bool IsStereoDisplayMode( void ) ;
		// プライマリウィンドウへの描画も行うか？
		virtual bool DoesDrawToPrimaryWindow( void ) ;
		// 表示状態か？
		virtual bool IsVisibleView( void ) const ;

	public:	// SGLVRViewProducer
		// デバイス状態更新タイミング同期
		virtual SGLError WaitForPollDeviceState( int64_t msecTimeout ) ;
		// 現在の HMD 姿勢を基準座標・姿勢にリセット
		virtual SGLError ResetCurrentHMDPosture( void ) ;
		// 現在の HMD 座標を基準座標にリセット
		virtual SGLError ResetCurrentHMDPosition( void ) ;
		// デバイスモデル取得
		virtual SGLError GetControllerModel
				( ModelInfo& mi, ControllerIndex iCtrl ) ;
		// HMD スケール
		virtual void SetScaleHMDToModel( double fpScale ) ;
		// HMD 空間タイプ取得
		virtual ViewSpaceType GetViewSpaceType( void ) const ;

	public:	// SProcedure
		// スレッド関数
		virtual void Run( void ) ;

	public:	// SGLWindowViewSynchronizer
		// 更新タイミング待ち
		virtual SGLError WaitForView( int64_t msecTimeout ) ;
	} ;

}

#endif

