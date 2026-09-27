
#if	!defined(__SAKURAGLX3D_SCENE_VR_ITEM_H__)
#define	__SAKURAGLX3D_SCENE_VR_ITEM_H__	1

#include <sakuraglx/render/sglx3d_scene_item.h>
#include <sakuragl/sgl_vr_view_producer.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// VR HMD ポジショントラッキング
	//////////////////////////////////////////////////////////////////////////

	class	S3DCameraHMDPosition : public S3DDynamicCamera::CameraController
	{
	public:
		enum	ControlFlag
		{
			flagLevelMatching		= 0x0001,	// 水平方向はHMDの姿勢を優先
			flagExtraHMDLevel		= 0x0002,	// HMD 基準 y 座標を特別に指定する
			flagIgnoreHMDPosition	= 0x0004,	// HMD 位置を反映しない
			flagIgnoreHMDRotation	= 0x0008,	// HMD 回転を反映しない
		} ;

	protected:
		SGLVRViewProducer *	m_pVR ;
		uint32_t			m_nCtrlFlags ;	// complex of enum ControllFlag
		SGLVRViewProducer::Posture
							m_postureLast ;
		S3DMatrix			m_matIBase ;	// 基準姿勢
		S3DVector			m_vOrigin ;
		double				m_yExtraHMD ;	// HMD 専用 y 座標

		bool				m_flagModifiedCamera ;
		S3DDVector			m_vModifiedCameraPos ;
		S3DDVector			m_vModifiedCameraTarget ;
		S3DDVector			m_vModifiedCameraTop ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DCameraHMDPosition, CameraController )
		S3D_DECLARE_COMPOSER_ITEM( S3DCameraHMDPosition, camera_hmd_position )
		// 構築関数
		S3DCameraHMDPosition
			( SGLVRViewProducer * pVR = NULL,
					uint32_t nFlags = flagLevelMatching ) ;
		// 消滅関数
		virtual ~S3DCameraHMDPosition( void ) ;

	public:
		// VR HMD 関連付け
		void AttachVRHMD( SGLVRViewProducer * pVR ) ;
		SGLVRViewProducer * GetVRHMD( void ) const
		{
			return	m_pVR ;
		}
		// 制御フラグ
		uint32_t GetControllFlags( void ) const
		{
			return	m_nCtrlFlags ;
		}
		void SetControllFlags( uint32_t nFlags ) ;
		// HMD 基準 y 座標（spaceEyeLevel の時に HMD の高さとして使用する）
		double GetHMDExtraY( void ) const ;
		void SetHMDExtraY( double y ) ;
		// 現在の姿勢を基準姿勢にリセット
		void ResetCurrentPosture( void ) ;
		void ResetCurrentPosition( void ) ;
		// 最後に取得された VR HMD で修正される前のカメラ座標取得
		bool GetLastModifiedCameraExceptVR
			( S3DDVector& vPos, S3DDVector& vTarget, S3DDVector& vTop ) const ;

	public:
		// フレーム描画前処理（全視点・ビュー共通処理）
		virtual void OnBeforeRender
			( S3DScene& scene, S3DDynamicCamera * pItem ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// VR コントローラーモデル表示アイテム
	//////////////////////////////////////////////////////////////////////////

	class	S3DVRControllerModelSerializer
						: public S3DSceneComposer::ModelSerializer
	{
	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO
			( S3DVRControllerModelSerializer, ModelSerializer )
		S3D_DECLARE_COMPOSER_ITEM
			( S3DVRControllerModelSerializer, vr_controller_model )
		// 構築関数
		S3DVRControllerModelSerializer( void ) ;

	public:
		// ポインタ用ビーム形状
		struct	BeamParam
		{
			float32_t	fpThickness ;		// ビーム幅
			float32_t	fpDefaultLength ;	// デフォルトの長さ
			float32_t	fpMinLength ;		// 最小の長さ
			float32_t	fpOffset ;			// ビーム開始オフセット
			SGLPalette	argbColor ;			// 表示色
			S3DVector	vDirection ;		// 発射方向
			S3DVector	vThickDir ;			// ビーム幅方向

			BeamParam( void )
				: fpThickness( 0.03f ),
					fpDefaultLength( 5.0f ),
					fpMinLength( 0.05f ),
					fpOffset( 0.05f ),
					argbColor( 0x8000A0A0 ),
					vDirection( 0, 0, 1 ),
					vThickDir( 1, 0, 0 ) { }
		} ;

	protected:
		SGLVRViewProducer *					m_pVR ;
		SGLVRViewProducer::HandIndex		m_iHand ;
		SGLVRViewProducer::ControllerIndex	m_iController ;

		bool						m_flagVisibleController ;
		S3DVertexBufferInterface *	m_pCtrlModelVBO ;

		bool		m_flagVisibleBeam ;
		float32_t	m_fpBeamLength ;
		BeamParam	m_bpBeam ;
		S3DMatrix	m_matTipBeam ;
		S3DVector	m_vTipBeam ;

	public:
		// VR 設定
		void SetVRController
			( SGLVRViewProducer * pVR,
				SGLVRViewProducer::HandIndex iHand,
				SGLVRViewProducer::ControllerIndex iController ) ;
		// ポインタ用ビーム設定
		const BeamParam& GetBeamParam( void ) const ;
		void SetBeamParam( const BeamParam& bp ) ;
		bool IsBeamVisible( void ) const ;
		void SetBeamVisible( bool flagVisible ) ;
		float32_t GetBeamLength( void ) const ;
		void SetBeamLength( float32_t fpLength ) ;

	public:	// S3DScene::Item
		// タイマ処理
		virtual void OnTimer( S3DScene& scene, uint32_t msecPast ) ;

	public:	// S3DScene::ModelItem
		// 表示モデル追加
		virtual void RenderLocalModel
			( const S3DScene& scene,
				S3DRenderContextInterface& render,
				uint64_t flagsExclusion = 0 ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// VR コントローラーアイテム
	//////////////////////////////////////////////////////////////////////////

	class	S3DVRHandController	: public S3DSceneComposer::Controller
	{
	public:
		enum	MousePointerParamConstantValue
		{
			mouseButtonMaxEntry	= 4,
		} ;
		struct	MousePointerParam
		{
			int			idVirtualMouse ;
			size_t		nLeftButtonEntries ;
			int			iLeftMouseButton[mouseButtonMaxEntry] ;
			size_t		nRightButtonEntries ;
			int			iRightMouseButton[mouseButtonMaxEntry] ;
			S3DVector	vPointerArrow ;
		} ;

		// カメラに対して固定されている2D表示レイヤ情報
		struct	Layer2DEntry
		{
			SGLSprite *	pSprite ;
			S3DDVector	vOrigin ;
			S3DDVector	vAxisX ;
			S3DDVector	vAxisY ;
		} ;

	protected:
		SGLVirtualInput *					m_pInput ;
		SGLVRViewProducer *					m_pVR ;
		SGLVRViewProducer::HandIndex		m_iHand ;
		SGLVRViewProducer::ControllerIndex	m_iController ;
		uint64_t							m_maskPressed ;

		SSystem::SObjectArray<SGLVirtualInput::InputEvent>
											m_aButtonEvents ;
		SSystem::SCriticalSection			m_csEventMap ;

		bool				m_flagVirtualMouse ;
		bool				m_flagVisibleMouse ;
		MousePointerParam	m_mppMousePointer ;
		SGLSprite			m_sprVirtualCursor ;
		SGLImage			m_imgDefaultCursor ;
		SGLSprite *			m_pFocusSprite ;

		S3DScene::Camera *				m_pCamera ;
		SSystem::SArray<Layer2DEntry>	m_aLayer2Ds ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DVRHandController, Controller )
		S3D_DECLARE_COMPOSER_ITEM( S3DVRHandController, vr_hand_controller )
		// 構築関数
		S3DVRHandController( void ) ;

	public:
		// 出力先入力キュー設定
		void AttachVirtualInput( SGLVirtualInput * pInput ) ;
		// VR 設定
		void SetVRHand
			( SGLVRViewProducer * pVR,
				SGLVRViewProducer::HandIndex iHand,
				SGLVRViewProducer::ControllerIndex iController ) ;
		// 仮想マウス設定
		void SetVirtualMouse( const MousePointerParam& mpp ) ;
		void GetVirtualMouse( MousePointerParam& mpp ) ;
		void EnableVirtualMouse( bool fVirtualMouse ) ;
		bool IsEnabledVirtualMouse( void ) const ;
		void SetVisibleMouseCursor( bool flagVisible ) ;
		// 仮想マウスカーソル画像設定
		void AttachCursorImage( SGLImageObject * pImage ) ;
		void AttachDefaultCursor( void ) ;
		// ボタンイベントマッピング
		SGLError SetButtonEvent
			( int iButton, const SGLVirtualInput::InputEvent& evOut ) ;
		SGLError GetButtonEvent
			( int iButton, SGLVirtualInput::InputEvent& evOut ) const ;
		SGLError RemoveButtonEvent( int iButton ) ;

	public:
		// 2Dレイヤー基準カメラ設定
		void SetCameraForLayer2D( S3DScene::Camera * pCamera ) ;
		// 2D表示レイヤ情報設定
		void SetLayer2DEntry( const Layer2DEntry& layer2D ) ;
		// 2D表示レイヤ情報計算
		static void CalcLayer2DEntry
			( Layer2DEntry& layer2D,
				SGLVRViewProducer * pVR,
				SGLSprite * pLayer2D,
				const S3DVector& vScreenDstPos,
				const S2DVector& vScreenCenter,
				const S2DVector& vScreenZoom ) ;
		// 2D表示レイヤ情報削除
		void RemoveLayer2DEntry( SGLSprite * pLayer2D ) ;
		// 全2D表示レイヤ情報削除
		void RemoveAllLayer2DEntries( void ) ;
	protected:
		// 2D表示レイヤ情報検索
		ssize_t FindLayer2DEntry( SGLSprite * pLayer2D ) const ;
		// 2D表示レイヤ当たり判定
		SGLSprite * GetHitRayForLayer2D
			( const S3DScene& scene,
				S2DDVector& vPos, double& zDistance,
				const S3DDVector& vRay, const S3DDVector& vOrigin ) const ;

	public:	// Controller
		// タイマー処理
		virtual void OnTimer
			( S3DScene& scene,
				S3DSceneComposer::ItemSerializer * pItem, uint32_t msecPast ) ;
	protected:
		void MouseButtonEvent
			( const S2DDVector& vPos,
				uint64_t maskLastPressed, int iVRButton,
					int idMouseButton, int vkeyMouseButton ) const ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// VR コントローラー表示アイテム空間セット
	//////////////////////////////////////////////////////////////////////////

	class	S3DVRItemSetSpace	: public S3DCameraRelativeSpace
	{
	public:
		enum	ParameterIndex
		{
			paramVisibleRightHand	= S3DCameraRelativeSpace::paramCameraRelSpaceTotalCount,
			paramVisibleLeftHand,
			paramVRItemTotalCount,
			paramVRItemCount
						= paramVRItemTotalCount - paramVisibleRightHand,
		} ;
	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DVRItemSetSpace, S3DCameraRelativeSpace ) 
		S3D_DECLARE_COMPOSER_ITEM( S3DVRItemSetSpace, vr_item_set_space )
		// 構築関数
		S3DVRItemSetSpace( void ) ;

	protected:
		SGLVRViewProducer *				m_pVR ;
		SSystem::SSmartReference<S3DCameraRelationController>
										m_pctrlRelCamera ;
		S3DVRControllerModelSerializer	m_modelController[SGLVRViewProducer::handCount] ;
		S3DVRHandController *			m_pctrlHand[SGLVRViewProducer::handCount] ;

	public:
		// 
		// VR 設定
		void SetVRController( SGLVRViewProducer * pVR ) ;
		void DetachVRController( void ) ;
		// 参照カメラ設定
		virtual void SetRelativeCamera
			( S3DScene::Camera * pCamera,
				const wchar_t * pwszCameraID = NULL,
				bool flagModifiedCameraSpace = false ) ;
		void DetachRelativeCamera( void ) ;
		// 座標更新
		virtual void GetReferenceCameraTransformation
			( S3DDMatrix& matCamera, S3DDVector& vCamera,
							const S3DScene::Camera * pCamera ) ;
		// コントローラー表示クラス
		void SetControllerRenderClass( int iHand, S3DScene::ItemClass cls ) ;
		S3DScene::ItemClass GetControllerRenderClass( int iHand ) const ;
		// コントローラー表示
		void SetVisibleController( int iHand, bool fVisible ) ;
		bool IsVisibleController( int iHand ) const ;
		// 仮想マウス設定
		SGLError SetVirtualMouse
			( int iHand, const S3DVRHandController::MousePointerParam& mpp ) ;
		SGLError GetVirtualMouse
			( int iHand, S3DVRHandController::MousePointerParam& mpp ) ;
		SGLError EnableVirtualMouse( int iHand, bool fVirtualMouse ) ;
		bool IsEnabledVirtualMouse( int iHand ) const ;
		void SetVisibleMouseCursor( int iHand, bool flagVisible ) ;
		// ポインタ用ビーム設定
		typedef	S3DVRControllerModelSerializer::BeamParam	BeamParam ;
		const BeamParam& GetPointerBeamParam( int iHand ) const ;
		void SetPointerBeamParam( int iHand, const BeamParam& bp ) ;
		bool IsPointerBeamVisible( int iHand ) const ;
		void SetPointerBeamVisible( int iHand, bool flagVisible ) ;
		// 出力先入力キュー設定
		void AttachVirtualInput( SGLVirtualInput * pInput ) ;
		// ボタンイベントマッピング
		SGLError SetButtonEvent
			( int iHand, int iButton, const SGLVirtualInput::InputEvent& evOut ) ;
		SGLError GetButtonEvent
			( int iHand, int iButton, SGLVirtualInput::InputEvent& evOut ) const ;
		SGLError RemoveButtonEvent( int iHand, int iButton ) ;

	public:
		// 2Dレイヤー基準カメラ設定
		void SetCameraForLayer2D( S3DScene::Camera * pCamera ) ;
		// 2D表示レイヤ情報設定
		void SetLayer2DEntry( const S3DVRHandController::Layer2DEntry& layer2D ) ;
		// 2D表示レイヤ情報削除
		void RemoveLayer2DEntry( SGLSprite * pLayer2D ) ;
		// 全2D表示レイヤ情報削除
		void RemoveAllLayer2DEntries( void ) ;
	} ;

}

#endif
