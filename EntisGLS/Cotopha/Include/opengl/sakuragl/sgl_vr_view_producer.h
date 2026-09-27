
#if	!defined(__SAKURAGL_VR_VIEW_PRODUCER_H__)
#define	__SAKURAGL_VR_VIEW_PRODUCER_H__	1

#include <sakuragl/window/sgl_window_producer.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// VR 出力基底インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SGLVRViewProducer	: public SGLSecondaryViewProducer
	{
	public:
		// 位置
		enum	PostureFlag
		{
			trackedOrientation	= 0x0001,
			trackedPosition		= 0x0002,
		} ;
		struct	Posture
		{
			uint32_t	nFlags ;			// complex of enum PostureFlag
			S3DMatrix	matOrientation ;	// 向き
			S3DVector	vPosition ;			// 位置

			Posture( void )
				: nFlags(0),
					matOrientation(1,0,0, 0,1,0, 0,0,1),
					vPosition(0,0,0) { }
			Posture( const Posture& p )
				: nFlags(p.nFlags),
					matOrientation(p.matOrientation),
					vPosition(p.vPosition) { }
		} ;

		// コントローラー状態
		enum	ControllerFlag
		{
			controllerConnected	= 0x0001,
		} ;
		enum	ControllerCapacity
		{
			capAxisThumbStick		= 0x0001,
			capAxisIndexTrigger		= 0x0002,
			capAxisMiddleTrigger	= 0x0004,
			capAxisRingTrigger		= 0x0008,
			capAxisLittileTrigger	= 0x0010,
			capAllAxis				= 0x00FF,
		} ;
		enum	ButtonIndex
		{
			buttonUp,	buttonDown,	buttonLeft,	buttonRight,
			button1,	button2,	button3,	button4,
			button5,	button6,	button7,	button8,
			button9,	button10,	button11,	button12,
			button13,	button14,	button15,	button16,
			buttonSystemFirst	= 32,
			buttonSystem		= buttonSystemFirst,
			buttonAppMenu,				// メニュー／戻るボタン
			buttonGrip,
			buttonAxis0			= 48,	// axisThumbStick: 親指（ジョイスティック）
			buttonAxis1,				// axisIndexTrigger: 人差し指トリガー
			buttonAxis2,				// axisMiddleTrigger: 中指トリガー
			buttonAxis3,
			buttonAxis4,
			buttonAxis5,
		} ;
		enum	AxisIndex
		{
			axisThumbStick,				// ジョイスティック／トラックパッド
			axisIndexTrigger,
			axisMiddleTrigger,
			axisRingTrigger,
			axisLittileTrigger,
			axisJoyStick2,				// トラックパッド（ジョイスティックがある場合の）
			axisJoyStick3,
			axisJoyStick4,
			axisCount,
			axisJoyStick1	= axisThumbStick,
			axisInvalid		= -1,
		} ;
		struct	ControllerState
		{
			uint32_t	nState ;		// complex of enum ControllerFlag
			uint32_t	nCapacity ;		// complex of enum ControllerCapacity
			uint64_t	maskButtonPressed ;
			uint64_t	maskButtonTouched ;
			S2DVector	vAxis[axisCount] ;

			ControllerState( void )
				: nState(0), nCapacity(0),
					maskButtonPressed(0), maskButtonTouched(0)
			{
				eslFillMemory( vAxis, 0, sizeof(vAxis) ) ;
			}
			ControllerState( const ControllerState& cs )
				: nState(cs.nState),
					nCapacity(cs.nCapacity),
					maskButtonPressed(cs.maskButtonPressed),
					maskButtonTouched(cs.maskButtonTouched)
			{
				eslCopyMemory( vAxis, cs.vAxis, sizeof(vAxis) ) ;
			}
			bool IsButtonPressed( size_t i ) const
			{
				return	(maskButtonPressed & ((uint64_t)1 << i)) != 0 ;
			}
			bool IsButtonTouched( size_t i ) const
			{
				return	(maskButtonTouched & ((uint64_t)1 << i)) != 0 ;
			}
		} ;

		// 画面情報
		struct	EyeFieldOfView
		{
			SGLSize		sizeOfView ;		// 画面サイズ
			S3DVector	vScreenPos ;		// 透視変換パラメータ
			float32_t	fpPixelAspect ;		// w/h

			EyeFieldOfView( void )
				: sizeOfView(0,0), vScreenPos(0,0,0), fpPixelAspect(1.0) { }
			EyeFieldOfView( const EyeFieldOfView& fov )
				: sizeOfView(fov.sizeOfView),
					vScreenPos(fov.vScreenPos),
					fpPixelAspect(fov.fpPixelAspect) { }
		} ;
		enum	EyeIndex
		{
			eyeInvalid	= -1,
			eyeRight,
			eyeLeft,
			eyeCount,
		} ;
		enum	HandIndex
		{
			handInvalid	= -1,
			handRight,
			handLeft,
			handCount,
		} ;
		enum	ControllerIndex
		{
			controllerInvalid	= -1,
			controllerRight,
			controllerLeft,
			controllerCount,
		} ;
		enum	ViewSpaceType
		{
			spaceEyeLevel,
			spaceFloorLevel,
		} ;

	protected:
		// プライマリウィンドウへ描画するか？
		bool				m_flagDrawToPrimary ;

		// スケール：モデル空間 / HMD 空間
		double				m_fpScaleHMDToModel ;

		// デバイス状態（座標・姿勢・コントローラー）パラメータ排他同期用
		SSystem::SCriticalSection	m_csDevState ;

		// 現在のフレームの位置情報
		Posture				m_postureHead ;				// 頭の位置
		Posture				m_postureEyes[eyeCount] ;	// 目の位置 [EyeIndex]
		Posture				m_postureHands[handCount] ;	// 手の位置 [HandIndex]
		Posture				m_postureHandsBase ;		// アプリ側で HMD の位置を補正した場合の手の位置補正用

		// コントローラーの状態
		ControllerState		m_stateControllers[controllerCount] ;

		// 視点ごとの視野情報 [EyeIndex]
		EyeFieldOfView		m_fovEyes[eyeCount] ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO
			( SGLVRViewProducer, SGLSecondaryViewProducer )
		// 構築関数
		SGLVRViewProducer( void ) ;

	public:	// SGLSecondaryViewProducer 実装
		// ステレオ立体視モードか？
		virtual bool IsStereoDisplayMode( void ) ;
		// プライマリウィンドウへの描画も行うか？
		virtual bool DoesDrawToPrimaryWindow( void ) ;
		// デバイス状態更新タイミング同期
		virtual SGLError WaitForPollDeviceState( int64_t msecTimeout ) = 0 ;

	public:
		// 現在の HMD 姿勢を基準座標・姿勢にリセット
		virtual SGLError ResetCurrentHMDPosture( void ) = 0 ;
		// 現在の HMD 座標を基準座標にリセット
		virtual SGLError ResetCurrentHMDPosition( void ) = 0 ;

	public:
		// デバイスモデル情報
		enum	ModelStatus
		{
			statusModelError	= -1,
			statusModelLoading,
			statusModelCompleted,
		} ;
		enum	ModelStatusFlag
		{
			modelVisible	= 0x0001,
		} ;
		struct	ModelInfo
		{
			ModelStatus					status ;
			uint32_t					nFlags ;	// complext of enum ModelStatusFlag
			S3DVertexBufferInterface *	pVBO ;
			Posture						postureModel ;
			Posture						postureTipLocal ;	// 人挿し指方向は *(0,0,1)
		} ;
		// デバイスモデル取得
		virtual SGLError GetControllerModel
				( ModelInfo& mi, ControllerIndex iCtrl ) = 0 ;

	public:
		// デバイス状態同期処理（姿勢やコントローラー状態取得用）
		void LockForDeviceState( void ) const ;
		void UnlockForDeviceState( void ) const ;

	public:
		// プライマリウィンドウへの描画設定
		void SetDrawingToPrimaryWindow( bool fDrawToPrimary ) ;
		// HMD スケール
		virtual void SetScaleHMDToModel( double fpScale ) ;
		double GetScaleHMDToModel( void ) const
		{
			return	m_fpScaleHMDToModel ;
		}
		// HMD 空間タイプ取得
		virtual ViewSpaceType GetViewSpaceType( void ) const = 0 ;
		// 現在のフレームの姿勢取得
		const Posture& GetHeadPosture( void ) const
		{
			return	m_postureHead ;
		}
		// 視点毎の相対位置取得
		const Posture& GetEyePosture( EyeIndex index ) const
		{
			return	m_postureEyes[index] ;
		}
		// 視点毎の位置計算
		const Posture& CalcEyePosture
				( Posture& postureEye, EyeIndex index ) const ;
		// アプリ側で補正した基準 HMD 位置を設定（手の取得位置に反映）
		virtual void SetHandsBasePosture( const Posture& postureBase ) ;
		// 現在のフレームの手（コントローラー）の位置取得
		Posture GetHandPosture( HandIndex iHand ) const ;
		// コントローラーの状態取得
		const ControllerState& GetControllerState( ControllerIndex iCtrl ) const
		{
			return	m_stateControllers[iCtrl] ;
		}
		// 視野情報取得
		const EyeFieldOfView& GetEyeFieldOfView( EyeIndex index ) const
		{
			return	m_fovEyes[index] ;
		}

	public:
		// バイブレーション・パラメータ
		struct	VibrationParam
		{
			uint32_t	duration ;		// [ms]
			float32_t	frequency ;		// [Hz]
			float32_t	amplitude ;		// [0,1]

			VibrationParam( void )
				: duration( 0 ), frequency( 0.0f ), amplitude( 1.0f ) {}
		} ;
		static constexpr const uint32_t		vibMinDuration	= 0 ;
		static constexpr const float32_t	vibDefaultFrequency = 0.0f ;

		// バイブレーション開始
		virtual SGLError StartVibration
			( ControllerIndex iCtrl, const VibrationParam& vibParam ) ;
		// バイブレーション即時停止
		virtual SGLError StopVibration( ControllerIndex iCtrl ) ;

	} ;

}

#endif

