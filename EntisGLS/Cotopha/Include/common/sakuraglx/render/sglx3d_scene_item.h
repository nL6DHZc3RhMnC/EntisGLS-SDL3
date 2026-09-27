
#if	!defined(__SAKURAGLX3D_SCENE_ITEM_H__)
#define	__SAKURAGLX3D_SCENE_ITEM_H__	1

#include <sakuraglx/render/sglx3d_scene_composer.h>
#include <sakuraglx/render/sglx3d_scene_particle.h>
#include <sakuraglx/render/sglx3d_scene_instancing.h>
#include <sakuraglx/render/sglx3d_scene_shader.h>


namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// コンポジション・プレイヤー・コントローラー
	//////////////////////////////////////////////////////////////////////////

	class	S3DCompositionPlayerController	: public S3DSceneComposer::Controller
	{
	public:
		// パラメータ
		enum	ParameterIndex
		{
			paramTimeLabel,
			paramExpressionType,
			paramPlayerCommand,
			paramPlayerTotalCount,
		} ;
		// 条件式の言語
		enum	ExpressionVM
		{
			vmRosetta,
			vmLoquaty,
		} ;

	protected:
		SSystem::SString		m_strLabel ;
		SSystem::SString		m_strCommand ;
		SSystem::SStringParser	m_sparsCommands ;
		bool					m_flagPaused ;
		bool					m_flagPauseCondition ;
		ExpressionVM			m_vmExprType ;
		S3DCompositionManager::ScriptExpression
								m_exprCondition ;
		S3DCompositionEditorInterface *
								m_pEditor ;

		static const SSystem::SXMLDocument::AttrInteger	m_aiExpressionType[3] ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DCompositionPlayerController, Controller )
		S3D_DECLARE_COMPOSER_ITEM( S3DCompositionPlayerController, timeline_controller )
		// 構築関数
		S3DCompositionPlayerController( void ) ;
		S3DCompositionPlayerController( const wchar_t * pwszClassID ) ;
		// パラメータ追加
		void AddParameterEntries( void ) ;
		// 消滅関数
		virtual ~S3DCompositionPlayerController( void ) ;

	public:
		// ラベルフレーム取得
		int32_t GetLabelFrameAs
			( const wchar_t * pwszLabel, bool * pFoundFrame = NULL ) const ;

	public:
		// パラメータ値取得
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		// パラメータ値設定
		virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;

	public:
		// フレーム（パラメータ）更新後処理
		virtual void OnUpdateFrame
			( S3DSceneComposer::ItemSerializer * pItem,
				double fpFrame, S3DSceneComposer::SeekMethod seek ) ;
		// タイマー処理
		virtual void OnTimer
			( S3DScene& scene,
				S3DSceneComposer::ItemSerializer * pItem, uint32_t msecPast ) ;

	protected:
		// 次のコマンドをフェッチ
		bool FetchNextCommand( S3DSceneComposer::Composition * pComp ) ;
		// if 文解釈
		bool ParseIfExpression
			( S3DSceneComposer::Composition * pComp,
				SSystem::SStringParser& sparsCmdLine ) ;
		// 条件式評価
		bool EvalConditionExpression( void ) ;
		// 条件式設定
		void SetConditionExpression
			( S3DSceneComposer::Composition * pComp, const wchar_t * pwszExpr ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// フラグ表示制御コントローラー
	//////////////////////////////////////////////////////////////////////////

	class	S3DQuickVisibleFlagController	: public S3DSceneComposer::Controller
	{
	public:
		// パラメータ
		enum	ParameterIndex
		{
			paramNegativeLogic,
			paramCombinationLogic,
			paramExpressionType,
			paramDefaultFlag,
			paramFlagExpr0,
			paramFlagExpr1,
			paramFlagExpr2,
			paramFlagExpr3,
			paramTotalCount,
			paramFlagExprCount	= 4,
		} ;
		enum	CombinationLogicType
		{
			combineAnd,
			combineOr,
		} ;
		enum	DefaultVisibleFlag
		{
			defaultInvisible,
			defaultVisible,
			defaultTimeline,
		} ;
		enum	ExpressionVM
		{
			vmRosetta,
			vmLoquaty,
		} ;

	protected:
		bool					m_flagEditMode ;
		bool					m_flagUnderControl ;
		bool					m_flagResetVisible ;
		bool					m_flagNegativeLogic ;
		ExpressionVM			m_vmExprType ;
		CombinationLogicType	m_logicCombination ;
		DefaultVisibleFlag		m_defaultVisibleFlag ;
		SSystem::SString		m_strFlagExpr[paramFlagExprCount] ;
		S3DSceneComposer::ScriptObject
								m_pFlagExpr[paramFlagExprCount] ;

		static const SSystem::SXMLDocument::AttrInteger	m_aiCombinationLogic[3] ;
		static const SSystem::SXMLDocument::AttrInteger	m_aiExpressionType[3] ;
		static const SSystem::SXMLDocument::AttrInteger	m_aiDefaultVisibleFlag[4] ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DQuickVisibleFlagController, Controller )
		S3D_DECLARE_COMPOSER_ITEM( S3DQuickVisibleFlagController, quick_vis_flag )
		// 構築関数
		S3DQuickVisibleFlagController( void ) ;
		// 消滅関数
		virtual ~S3DQuickVisibleFlagController( void ) ;

	public:
		// フラグ変数参照
		void UpdateFlagReflections( void ) ;
		void UpdateFlagReflectionAt( size_t iFlag ) ;
		// フラグ評価
		bool EvaluateFlags( bool flagCurVisible, bool& flagController ) ;

	public:
		// パラメータ値取得
		virtual bool GetBooleanParameter( size_t iParam ) const ;
		virtual const wchar_t * GetCommandParameter( size_t iParam ) const ;
		// パラメータ値設定
		virtual void SetBooleanParameter( size_t iParam, bool b ) ;
		virtual void SetCommandParameter( size_t iParam, const wchar_t * pwszCmd ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;

	public:
		// アイテムプロパティのリソース等の参照を更新する
		virtual uint32_t UpdatePropertyReference
			( S3DSceneComposer::Composition& comp,
				S3DSceneComposer::ItemSerializer * pItem, uint32_t nFlags ) ;
		// フレーム（パラメータ）更新後処理
		virtual void OnUpdateFrame
			( S3DSceneComposer::ItemSerializer * pItem,
				double fpFrame, S3DSceneComposer::SeekMethod seek ) ;
		// タイマー処理
		virtual void OnTimer
			( S3DScene& scene,
				S3DSceneComposer::ItemSerializer * pItem, uint32_t msecPast ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// カメラ連動空間
	//////////////////////////////////////////////////////////////////////////

	class	S3DCameraRelativeSpace	: public S3DSceneComposer::SpaceSerializer
	{
	public:
		enum	ParameterIndex
		{
			paramRelativeCamera	= SpaceSerializer::paramSpaceTotalCount,
			paramModifiedCameraTrans,
			paramCameraRelSpaceTotalCount,
			paramCameraRelSpaceCount
						= paramCameraRelSpaceTotalCount - paramRelativeCamera,
		} ;
	protected:
		static const S3DSceneComposer::ParamEntry		m_paramEntries[paramCameraRelSpaceCount] ;
		static const S3DSceneComposer::ParamSetClass	m_pscClass ;

	protected:
		SSystem::SSmartReference<S3DScene::Camera>
								m_refCamera ;
		SSystem::SString		m_strRelCameraID ;
		S3DDMatrix				m_matLocalSpace ;
		S3DDVector				m_vLocalSpace ;
		bool					m_flagModifiedCamera ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DCameraRelativeSpace, SpaceSerializer )
		S3D_DECLARE_COMPOSER_ITEM( S3DCameraRelativeSpace, rel_camera_space )
		// 構築関数
		S3DCameraRelativeSpace( void ) ;

	public:
		// 参照カメラ設定
		virtual void SetRelativeCamera
			( S3DScene::Camera * pCamera,
				const wchar_t * pwszCameraID = NULL,
				bool flagModifiedCameraSpace = false ) ;
		// 座標更新
		virtual void UpdateLocalTransformation( void ) ;
		virtual void GetReferenceCameraTransformation
			( S3DDMatrix& matCamera, S3DDVector& vCamera,
							const S3DScene::Camera * pCamera ) ;

	public:	// Parameter
		// パラメータ値取得
		virtual bool GetBooleanParameter( size_t i ) const ;
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		// パラメータ値設定
		virtual void SetBooleanParameter( size_t i, bool b ) ;
		virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;

	public:	// ItemSerializer
		// パラメーター有効性
		virtual bool IsParameterValidation( size_t i ) const ;
		// アイテムプロパティのリソース等の参照を更新する
		virtual uint32_t UpdatePropertyReference
			( S3DSceneComposer::Composition& comp, uint32_t nFlags ) ;

	public:	// S3DScene::Space
		// ローカル変換行列を取得
		virtual const S3DDMatrix&
				GetLocalTransformation( S3DDMatrix& matLocal ) const ;
		// ローカル変換行列を設定
		virtual void SetLocalTransformation( const S3DDMatrix& matLocal ) ;
		// ローカル座標を取得
		virtual const S3DDVector&
				GetLocalSpacePosition( S3DDVector& vLocal ) const ;
		// ローカル変換行列を設定
		virtual void SetLocalSpacePosition( const S3DDVector& vLocal ) ;
		// タイマ処理
		virtual void OnTimer( S3DScene& scene, uint32_t msecPast ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// カメラ連動コントローラー
	//////////////////////////////////////////////////////////////////////////

	class	S3DCameraRelativeController	: public S3DSceneComposer::Controller
	{
	public:
		enum	ParameterIndex
		{
			paramBasePosition,
			paramMoveUnit,
			paramRelativeX,
			paramRelativeY,
			paramRelativeZ,
		} ;

	protected:
		S3DDVector	m_vBasePos ;
		S3DDVector	m_vMoveUnit ;
		bool		m_flagRelativeX ;
		bool		m_flagRelativeY ;
		bool		m_flagRelativeZ ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DCameraRelativeController, Controller )
		S3D_DECLARE_COMPOSER_ITEM( S3DCameraRelativeController, rel_camera_pos )
		// 構築関数
		S3DCameraRelativeController( void ) ;
		S3DCameraRelativeController( const wchar_t * pwszClassID ) ;

	public:
		// パラメータ値取得
		virtual S3DDVector GetVectorParameter( size_t iParam ) const ;
		virtual bool GetBooleanParameter( size_t iParam ) const ;
		// パラメータ値設定
		virtual void SetVectorParameter( size_t iParam, const S3DDVector& vec ) ;
		virtual void SetBooleanParameter( size_t iParam, bool b ) ;
		// タイマー処理
		virtual void OnTimer
			( S3DScene& scene,
				S3DSceneComposer::ItemSerializer * pItem, uint32_t msecPast ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 動的カメラアイテム
	//////////////////////////////////////////////////////////////////////////

	class	S3DDynamicCamera : public S3DSceneComposer::CameraSerializer
	{
	public:
		class	CameraController	: public S3DSceneComposer::Controller
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( CameraController, Controller )
			// 構築関数
			CameraController( const wchar_t * pwszClassID ) ;
			// フレーム描画前処理（全視点・ビュー共通処理）
			virtual void OnBeforeRender
				( S3DScene& scene, S3DDynamicCamera * pItem ) = 0 ;
		} ;

	protected:
		S3DDVector	m_vCameraPos ;
		S3DDVector	m_vCameraTarget ;
		S3DDVector	m_vCameraTop ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DDynamicCamera, CameraSerializer )
		S3D_DECLARE_COMPOSER_ITEM( S3DDynamicCamera, dynamic_camera )
		// 構築関数
		S3DDynamicCamera( void ) ;
		// 消滅関数
		virtual ~S3DDynamicCamera( void ) ;

	public:
		// 修正前カメラ座標
		virtual const S3DDVector& GetCameraPosition( void ) const ;
		virtual const S3DDVector& GetCameraTarget( void ) const ;
		virtual const S3DDVector& GetCameraTop( void ) const ;
		virtual void SetCameraPosition( const S3DDVector& vPos ) ;
		virtual void SetCameraTarget( const S3DDVector& vTarget ) ;
		virtual void SetCameraTop( const S3DDVector& vTop ) ;

	public:	// S3DScene::Item
		// アイテム作用の追加処理
		virtual void OnUpdateBehavior( S3DScene& scene ) ;

	public:
		// 視点修正
		void ModifyCameraPosture
			( const S3DDMatrix& matCameraRotation,
				const S3DDVector& vCameraOffset,
				bool fAngleLevelMatch, bool fOffsetLevelMatch ) ;
		// 修正カメラ計算
		void CalcModifiedCamera
			( S3DDVector& vModifiedPos,
				S3DDVector& vModifiedTarget,
				S3DDVector& vModifiedTop,
				const S3DDMatrix& matCameraRotation,
				const S3DDVector& vCameraOffset,
				bool fAngleLevelMatch, bool fOffsetLevelMatch ) const ;
		// カメラ座標修正空間行列計算
		static S3DDMatrix CalcModifiedCameraRotation
			( S3DDMatrix& matCameraRotation,
				const S3DDVector& vCameraPos,
				const S3DDVector& vCameraTarget,
				const S3DDVector& vCameraTop,
				bool fAngleLevelMatch, bool fOffsetLevelMatch ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 画面揺れ効果
	//////////////////////////////////////////////////////////////////////////

	class	S3DCameraWiggleController	: public S3DDynamicCamera::CameraController
	{
	public:
		// 振動パラメータ
		enum	WiggleFlag
		{
			flagFreqByRenderFrame	= 0x0001,	// フレーム描画毎に +freq*π
			flagVerticalRandom		= 0x0002,	// 垂直方向にランダム
			flagHorizontalRandom	= 0x0004,	// 水平方向にランダム
		} ;
		struct	WiggleParam
		{
			uint32_t	m_nFlags ;			// フラグ
			S2DVector	m_degAmplitude ;	// 振幅 [deg]
			S2DVector	m_vFrequency ;		// 周波数 [Hz]
			double		m_secFadeout ;		// フェードアウト時間 [sec]

			WiggleParam( void )
				: m_nFlags( 0 ), m_degAmplitude( 0, 0 ),
					m_vFrequency( 0, 0 ), m_secFadeout( 0.0 ) { }
			WiggleParam( const WiggleParam& wp )
				: m_nFlags( wp.m_nFlags ),
					m_degAmplitude( wp.m_degAmplitude ),
					m_vFrequency( wp.m_vFrequency ),
					m_secFadeout( wp.m_secFadeout ) { }
		} ;

		// パラメータ
		enum	ParameterIndex
		{
			paramAmplitudeX,
			paramFrequencyX,
			paramAmplitudeY,
			paramFrequencyY,
		} ;

	protected:
		struct	Wiggle	: public WiggleParam
		{
			S2DVector	m_radPhase ;		// 振動位相 [rad]
			double		m_secTime ;			// 経過時間 [sec]

			Wiggle( void )
				: m_radPhase( 0, 0 ), m_secTime( 0.0 ) { }
			Wiggle( const WiggleParam& wp )
				: WiggleParam( wp ),
					m_radPhase( 0, 0 ), m_secTime( 0.0 ) { }
		} ;
		SSystem::SObjectArray<Wiggle>	m_wiggles ;
		SakuraCL::SCLRandomizer			m_random ;

		S2DDVector	m_vAmplitude ;		// 振幅 [deg]
		S2DDVector	m_vFrequency ;		// 周波数 [Hz]
		S2DDVector	m_radPhase ;		// 振動位相 [rad]

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DCameraWiggleController, CameraController )
		S3D_DECLARE_COMPOSER_ITEM( S3DCameraWiggleController, camera_wiggle )
		// 構築関数
		S3DCameraWiggleController( void ) ;
		// 消滅関数
		virtual ~S3DCameraWiggleController( void ) ;

	public:
		// 振動追加
		void AddWiggle( const WiggleParam& wp ) ;
		// 全ての振動を削除
		void RemoveAllWiggles( void ) ;

	public:	// Parameter
		// パラメータ値取得
		virtual double GetScalarParameter( size_t i ) const ;
		// パラメータ値設定
		virtual void SetScalarParameter( size_t i, double s ) ;

	public:	// CameraController
		// フレーム描画前処理（全視点・ビュー共通処理）
		virtual void OnBeforeRender
			( S3DScene& scene, S3DDynamicCamera * pItem ) ;
		// タイマー処理
		virtual void OnTimer
			( S3DScene& scene,
				S3DSceneComposer::ItemSerializer * pItem, uint32_t msecPast ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// カメラ・オフセット制御
	//////////////////////////////////////////////////////////////////////////

	class	S3DCameraOffsetController	: public S3DDynamicCamera::CameraController
	{
	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DCameraOffsetController, CameraController )
		S3D_DECLARE_COMPOSER_ITEM( S3DCameraOffsetController, camera_offset )
		// 構築関数
		S3DCameraOffsetController( void ) ;
		// 消滅関数
		virtual ~S3DCameraOffsetController( void ) ;

	public:
		enum	ControlFlag
		{
			flagOffsetLevelMatching	= 0x0001,	// オフセット座標はカメラ空間の平面回転のみ
		} ;

	protected:
		uint32_t	m_nCtrlFlags ;	// complex of enum ControllFlag
		S3DDMatrix	m_matRotate ;
		S3DDVector	m_vOffset ;			// 視点座標系

	public:
		// 制御フラグ
		uint32_t GetControlFlags( void ) const
		{
			return	m_nCtrlFlags ;
		}
		void SetControlFlags( uint32_t nFlags ) ;
		// オフセット座標
		const S3DDVector& GetOffset( void ) const
		{
			return	m_vOffset ;
		}
		void SetOffset( const S3DDVector& vOffset ) ;
		// 回転
		const S3DDMatrix& GetRotation( void ) const
		{
			return	m_matRotate ;
		}
		void SetRotation( const S3DDMatrix& matRotate ) ;

	public:
		// オフセット後カメラ座標計算
		S3DDVector CalcOffsetCameraPos( const S3DDynamicCamera& camera ) const ;
		S3DDVector CalcOffsetCameraPos
			( const S3DDynamicCamera& camera, const S3DDVector& vOffset ) const ;
		// オフセット座標計算
		S3DVector CalcOffsetForCameraPos
			( const S3DDynamicCamera& camera, const S3DDVector& vCameraPos ) const ;

	public:	// CameraController
		// フレーム描画前処理（全視点・ビュー共通処理）
		virtual void OnBeforeRender
			( S3DScene& scene, S3DDynamicCamera * pItem ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// カメラ連動通知コントローラー
	//////////////////////////////////////////////////////////////////////////

	class	S3DCameraRelationController	: public S3DDynamicCamera::CameraController
	{
	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DCameraRelationController, CameraController )
		S3D_DECLARE_COMPOSER_ITEM( S3DCameraHMDPosition, camera_passive_relation )
		// 構築関数
		S3DCameraRelationController( S3DCameraRelativeSpace * pRelSpace = NULL ) ;

	protected:
		SSystem::SSyncReference	m_refRelSpace ;
		SSystem::SString		m_strRelSpaceID ;

		size_t	m_iParamRelCameraSpace ;

	public:
		// S3DCameraRelativeSpace 設定
		void AttachCameraRelativeSpace
				( S3DCameraRelativeSpace * pcrs,
					const wchar_t * pwszSpaceID = NULL ) ;

	public:	// Parameter
		// パラメータ値取得
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		// パラメータ値設定
		virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;

	public:	// CameraController
		// フレーム描画前処理（全視点・ビュー共通処理）
		virtual void OnBeforeRender
			( S3DScene& scene, S3DDynamicCamera * pItem ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 参照空間コントローラー
	//////////////////////////////////////////////////////////////////////////

	class	S3DSpaceReferenceController	: public S3DSceneComposer::Controller
	{
	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DSpaceReferenceController, Controller )
		S3D_DECLARE_COMPOSER_ITEM( S3DSpaceReferenceController, space_referencer )
		// 構築関数
		S3DSpaceReferenceController( void ) ;

	protected:
		SSystem::SString		m_strRefSpaceID ;
		SSystem::SString		m_strTargetBoneID ;
		SSystem::SSyncReference	m_refTargetItem ;

		size_t	m_iParamRefSpace ;
		size_t	m_iParamTargetBone ;

	public:
		// 参照空間設定
		void AttachReferenceSpace
				( S3DScene::Space * pSpace,
					const wchar_t * pwszSpaceID = NULL ) ;
		// ターゲット空間設定
		void AttachTargetSpace
				( S3DScene::Space * pSpace,
					const wchar_t * pwszTargetBoneID = NULL ) ;

	public:
		// 参照空間切り替え
		void SwitchReferenceSpace( S3DSceneComposer::Composition& comp ) ;

	public:
		// パラメータ値取得
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		// パラメータ値設定
		virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;

	public:
		// アイテムプロパティのリソース等の参照を更新する
		virtual uint32_t UpdatePropertyReference
			( S3DSceneComposer::Composition& comp,
				S3DSceneComposer::ItemSerializer * pItem, uint32_t nFlags ) ;
		// フレーム（パラメータ）更新後処理
		virtual void OnUpdateFrame
			( S3DSceneComposer::ItemSerializer * pItem,
				double fpFrame, S3DSceneComposer::SeekMethod seek ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 動的モデルアイテム
	//////////////////////////////////////////////////////////////////////////

	class	S3DDynamicModelSerializer
				: public S3DDynamicModelItem,
						public S3DSceneComposer::ItemCommonSerializer
	{
	public:
		// ポーズコントローラー（抽象）
		class	PoseInterface	: public ESLObject
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( PoseInterface, ESLObject )
			// ポーズ適用
			virtual void OnPoseTrack
				( S3DDynamicModelSerializer& item, S3DModelBuffer& model ) = 0 ;
		protected:
			// モデル変更時の処理
			virtual void OnChangedModel
				( S3DDynamicModelSerializer& item, S3DModelBuffer * pModel ) = 0 ;

			friend class S3DDynamicModelSerializer ;
		} ;
		class	PoseConntoller	: public S3DSceneComposer::Controller,
									public PoseInterface
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO2( PoseConntoller, Controller, PoseInterface )
			// 構築関数
			PoseConntoller( const wchar_t * pwszClassID ) ;
		} ;
		// ポーズトラック
		class	PoseTrack	: public PoseConntoller
		{
		public:
			enum	ParameterIndex
			{
				paramPose,
				paramBlend,
			} ;
		protected:
			S3DSceneComposer::PoseInstance	m_poseCurrent ;
			SSystem::SString				m_strPoseTemp ;
			double							m_fpPoseBlend ;
		public:
			// クラス情報
			SGL_DECLARE_CLASS_INFO( PoseTrack, PoseConntoller )
			S3D_DECLARE_COMPOSER_ITEM( PoseTrack, pose_track )
			// 構築関数
			PoseTrack( void ) ;
			// ポーズ適用
			virtual void OnPoseTrack
				( S3DDynamicModelSerializer& item, S3DModelBuffer& model ) ;
		public:
			// ポーズ
			const S3DSceneComposer::PoseInstance& GetCurrentPose( void ) const ;
			void SetCurrentPose( const S3DSceneComposer::PoseInstance& pose ) ;
			// 適用度
			double GetCurrentBlend( void ) const ;
			void SetCurrentBlend( double fpBlend ) ;
		public:	// Parameter
			// パラメータ値取得
			virtual double GetScalarParameter( size_t i ) const ;
			virtual const wchar_t * GetCommandParameter( size_t i ) const ;
			virtual S3DSceneComposer::PoseInstance GetPoseParameter( size_t i ) const ;
			// パラメータ値設定
			virtual void SetScalarParameter( size_t i, double s ) ;
			virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
			virtual void SetPoseParameter
				( size_t i, const S3DSceneComposer::PoseInstance& pose ) ;
			// パラメータ値域列挙
			virtual bool EnumerateStringSet
				( size_t iParam, SSystem::SStringArray& aStrSet ) ;
		protected:
			// モデル変更時の処理
			virtual void OnChangedModel
				( S3DDynamicModelSerializer& item, S3DModelBuffer * pModel ) ;
		public:
			// アイテムプロパティのリソース等の参照を更新する
			virtual uint32_t UpdatePropertyReference
				( S3DSceneComposer::Composition& comp,
						ItemSerializer * pItem, uint32_t nFlags ) ;
		} ;
		// IK ボーントラック
		class	IKBoneTrack	: public PoseConntoller
		{
		public:
			enum	ParameterIndex
			{
				paramBone,
				paramPosition,
				paramRotation,
				paramHandleTip,
				paramTipOffset,
				paramGimbalWeight,
				paramIKWeight,
				paramTipBoneWeight,
				paramEffectJoints,
				paramStability,
				paramRefPose,
			} ;
		protected:
			SSystem::SString	m_strBone ;
			S3DDVector			m_vIKPosition ;
			S3DDMatrix			m_matIKRotate ;
			bool				m_flagHandleTip ;
			S3DDVector			m_vTipOffset ;
			double				m_fpGimbalWeight ;
			double				m_fpIKWeight ;
			double				m_fpTipBoneWeight ;
			int32_t				m_nEffectJoints ;
			double				m_fpBoneStability ;
			S3DSceneComposer::PoseInstance	m_poseRef ;
			SSystem::SString				m_strPoseTemp ;
		public:
			// クラス情報
			SGL_DECLARE_CLASS_INFO( IKBoneTrack, PoseConntoller )
			S3D_DECLARE_COMPOSER_ITEM( IKBoneTrack, bone_ik )
			// 構築関数
			IKBoneTrack( void ) ;
			// ポーズ
			const S3DSceneComposer::PoseInstance& GetRefPose( void ) const ;
			void SetRefPose( const S3DSceneComposer::PoseInstance& pose ) ;
			// ポーズ適用
			virtual void OnPoseTrack
				( S3DDynamicModelSerializer& item, S3DModelBuffer& model ) ;
		protected:
			S3DDQuaternion GetPoseOfBone
				( S3DModelBuffer& model, S3DModelBoneSpace * pBone,
									const S3DModelPose & pose, double t ) ;
			// モデル変更時の処理
			virtual void OnChangedModel
				( S3DDynamicModelSerializer& item, S3DModelBuffer * pModel ) ;
		public:	// Parameter
			// パラメータ値取得
			virtual S3DDMatrix GetMatrixParameter( size_t iParam ) const ;
			virtual S3DDVector GetVectorParameter( size_t iParam ) const ;
			virtual double GetScalarParameter( size_t iParam ) const ;
			virtual int32_t GetIntegerParameter( size_t iParam ) const ;
			virtual bool GetBooleanParameter( size_t iParam ) const ;
			virtual const wchar_t * GetCommandParameter( size_t iParam ) const ;
			virtual S3DSceneComposer::PoseInstance GetPoseParameter( size_t iParam ) const ;
			// パラメータ値設定
			virtual void SetMatrixParameter( size_t iParam, const S3DDMatrix& mat ) ;
			virtual void SetVectorParameter( size_t iParam, const S3DDVector& vec ) ;
			virtual void SetScalarParameter( size_t iParam, double s ) ;
			virtual void SetIntegerParameter( size_t iParam, int32_t n ) ;
			virtual void SetBooleanParameter( size_t iParam, bool b ) ;
			virtual void SetCommandParameter( size_t iParam, const wchar_t * pwszCmd ) ;
			virtual void SetPoseParameter
				( size_t i, const S3DSceneComposer::PoseInstance& pose ) ;
			// パラメータ値域列挙
			virtual bool EnumerateStringSet
				( size_t iParam, SSystem::SStringArray& aStrSet ) ;
		public:
			// アイテムプロパティのリソース等の参照を更新する
			virtual uint32_t UpdatePropertyReference
				( S3DSceneComposer::Composition& comp,
						ItemSerializer * pItem, uint32_t nFlags ) ;
		} ;

	public:
		enum	ParameterIndex
		{
			paramModel			= ItemCommonSerializer::paramItemTotalCount,
			paramCollision,
			paramColliderFlags,
			paramMarkerCollision,
			paramDynamicCollision,
			paramPose,
			paramVDrawTarget,
			paramBorderParam,
			paramBorderColor,
			paramBorderThickness1,
			paramBorderThickness2,
			paramModelTotalCount,
			paramModelCount		= paramModelTotalCount - paramModel,
		} ;
	protected:
		static const S3DSceneComposer::ParamEntry		m_paramEntries[paramModelCount] ;
		static const S3DSceneComposer::ParamSetClass	m_pscClass ;

	protected:
		bool							m_flagVisible ;
		S3DModelBuffer					m_modelRef ;
		SSystem::SString				m_strModelID ;
		SSystem::SString				m_strCollisionID ;
		S3DModelData::MarkerInfo::Type	m_typeMarkerCollision ;
		bool							m_flagDynamicCollision ;
		S3DSceneComposer::PoseInstance	m_poseCurrent ;
		SSystem::SString				m_strPoseTemp ;
		SSystem::SString				m_strVDrawtTarget ;
		SSystem::SSyncReference			m_refVDrawTarget ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO2
			( S3DDynamicModelSerializer,
					S3DDynamicModelItem, ItemCommonSerializer )
		S3D_DECLARE_COMPOSER_ITEM( S3DDynamicModelSerializer, dynamic_model )
		// 構築関数
		S3DDynamicModelSerializer( void ) ;
		S3DDynamicModelSerializer
			( const wchar_t * pwszClassID,
				const S3DSceneComposer::ParamSetClass * pClass ) ;

	public:
		// モデル
		void AttachModelReference
			( S3DModelBuffer * pModel, S3DSceneComposer * pComposer = NULL ) ;
		void SetModel( const wchar_t * pwszModelID ) ;
		void UpdateModel( void ) ;
		const wchar_t * GetModelID( void ) const ;
		// 表示用モデルエイリアス取得
		S3DModelBuffer * GetModelAlias( void ) const ;
		// 衝突判定モデル
		void SetCollision( const wchar_t * pwszModelID ) ;
		void UpdateCollision( void ) ;
		const wchar_t * GetCollisionID( void ) const ;
		// 動的な衝突判定モデルか？
		bool IsDynamicCollision( void ) const ;
		void SetDynamicCollisionFlag( bool flagDynamic ) ;
		// バリアント描画ターゲット
		void SetVariantDrawTaregt( const wchar_t * pwszVDrawTarget ) ;
		bool UpdateVariantDrawTaregt( void ) ;
		const wchar_t * GetVariantDrawTarget( void ) const ;

	protected:
		// モデル変更時の処理
		virtual void OnChangedModel( S3DModelBuffer * pModel ) ;

	public:	// S3DScene::ModelItem
		// モデルデータ関連付け
		virtual void AttachModel( S3DVertexBufferInterface * pModel ) ;
		virtual void AttachCollisionModel
			( S3DVertexBufferInterface * pColModel, bool fBuildCollision = true ) ;

	protected:	// S3DDynamicModelItem
		// ポーズ設定
		virtual void UpdateModelPose( S3DModelBuffer& model ) ;
		// ポーズアニメーション
		virtual void OnPoseAnimationTrack( S3DModelBuffer& model ) ;

	public:	// S3DScene::ModelItem
		// 表示モデル追加
		virtual void RenderLocalModel
			( const S3DScene& scene,
				S3DRenderContextInterface& render,
				uint64_t flagsExclusion = 0 ) ;
		// レンダリング前後処理（全視点共通）
		virtual void OnRenderEvent
			( S3DScene& scene, S3DScene::ItemClass clsItem ) ;

	public:	// Parameter
		// パラメータ値取得
		virtual S3DDVector GetVectorParameter( size_t i ) const ;
		virtual double GetScalarParameter( size_t i ) const ;
		virtual int32_t GetIntegerParameter( size_t i ) const ;
		virtual bool GetBooleanParameter( size_t i ) const ;
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		virtual S3DSceneComposer::PoseInstance
								GetPoseParameter( size_t i ) const ;
		// パラメータ値設定
		virtual void SetVectorParameter( size_t i, const S3DDVector& vec ) ;
		virtual void SetScalarParameter( size_t i, double s ) ;
		virtual void SetIntegerParameter( size_t i, int32_t n ) ;
		virtual void SetBooleanParameter( size_t i, bool b ) ;
		virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		virtual void SetPoseParameter
				( size_t i, const S3DSceneComposer::PoseInstance& pose ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;
		void EnumeratePoseStringSet
			( SSystem::SStringArray& aStrSet ) const ;
		static void EnumeratePoseStringSetOf
			( SSystem::SStringArray& aStrSet,
							const S3DModelPoseLibrary *	pPoseLib ) ;
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;

	public:
		// タイマ処理 Item::OnTimer / ItemSerializer::OnTimer 実装
		virtual void OnTimer( S3DScene& scene, uint32_t msecPast ) ;

	public:	// ItemSerializer
		// パラメーター有効性
		virtual bool IsParameterValidation( size_t i ) const ;
		// ポーズ取得
		virtual S3DModelPose *
			GetPoseIdentityAs( const wchar_t * pwszPoseID ) const ;
		// ポーズID取得
		virtual bool GetPoseIdentityOf
			( SSystem::SString& strPoseID, S3DModelPose * pPose ) const ;
		// アイテムのプライマリモデル取得
		virtual S3DVertexBufferInterface * GetItemPrimaryModel( void ) ;
		// アイテムのコリジョンバッファ取得
		virtual S3DCollider * GetItemPrimaryCollider( void ) ;
		// ポーズライブラリ取得
		virtual S3DModelPoseLibrary * GetPoseLibraryChain( void ) ;
		// フレーム更新後処理
		virtual void OnUpdateFrame
			( double fpFrame, S3DSceneComposer::SeekMethod seek ) ;
		// アイテムプロパティのリソース等の参照を更新する
		virtual uint32_t UpdatePropertyReference
			( S3DSceneComposer::Composition& comp, uint32_t nFlags ) ;
		// シーン取得
		virtual S3DScene * GetScene( void ) const ;

	public:
		// Loquaty クラス名
		virtual const wchar_t * GetLQClassName( void ) const ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 物理演算慣性ポーズ制御
	//////////////////////////////////////////////////////////////////////////

	class	S3DBasicInertialPoser
	{
	protected:
		struct	InertialVertex	: public S3DModelBoneSpace::PhysVertex
		{
			S3DDMatrix	matInertialBone ;
		} ;
		double					m_degFreeAngle ;
		double					m_fpAttenuation ;
		double					m_fpHardness ;
		double					m_fpEffect ;
		SSystem::SObjectArray
			<SSystem::SString>	m_aTargetBone ;
		S3DModelBuffer *		m_pModel ;
		SSystem::SPointerArray
			<S3DModelBoneSpace>	m_aBonePtr ;
		SSystem::SArray
			<InertialVertex>	m_aPhysVertex ;
		S3DModelBoneSpace::PhysExogenous
								m_physExogenous ;
		bool					m_flagResetPose ;
		double					m_secPhysical ;

	public:
		// 構築関数
		S3DBasicInertialPoser( void ) ;

	public:
		// モデル関連付け
		void AttachTargetModel
			( S3DModelBuffer * pModel, bool flagForceUpdateRef = false ) ;
		// ボーン参照更新
		void UpdateModelBoneRef( void ) ;
		// 関連付けモデル取得
		S3DModelBuffer * GetTargetModel( void ) const ;
		// 対象ボーン数設定
		void SetTargetBoneCount( size_t nCount ) ;
		// 対象ボーン数取得
		size_t GetTargetBoneCount( void ) const ;
		// 対象ボーン設定
		void SetTargetBoneNameAt( size_t i, const wchar_t * pwszBoneID ) ;
		// 対象ボーン追加
		size_t AddTargetBoneName( const wchar_t * pwszBoneID ) ;
		// 対象ボーン名取得
		const wchar_t * GetTargetBoneNameAt( size_t i ) const ;

	public:
		// 角自由度[deg]
		double GetFreeAngle( void ) const ;
		void SetFreeAngle( double degAngle ) ;
		// 速度減衰率 [0,1] [/sec]
		double GetAttenuation( void ) const ;
		void SetAttenuation( double fpAttenuation ) ;
		// 弾性 [/frame]
		double GetHardness( void ) const ;
		void SetHardness( double fpHardness ) ;
		// 慣性影響度
		double GetEffect( void ) const ;
		void SetEffect( double fpEffect ) ;

	public:
		// 初期化フラグ設定
		void SetResetFlag( void ) ;
		// 経過時間
		void AddElapsedTime( double secElapsed ) ;

	public:
		// 物理演算パラメータ初期化
		void ResetPhysicalParameter( S3DSceneComposer::ItemSerializer& item ) ;
		// 物理演算実行
		void CalculatePhysics
			( S3DSceneComposer::ItemSerializer& item, S3DModelBuffer& model ) ;
		static void CalculateBonePhysics
			( S3DModelBoneSpace& bone,
				InertialVertex& iv,
				const S3DModelBoneSpace::PhysMaterial& mtrl,
				const S3DModelBoneSpace::PhysExogenous& exog, double secElapsed ) ;
		static void SetBoneRotationWithPhysics
			( S3DModelBoneSpace& bone, const InertialVertex& iv ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 物理演算慣性ポーズ・コントローラー
	//////////////////////////////////////////////////////////////////////////

	class	S3DInertialPoseController
				: public S3DDynamicModelSerializer::PoseConntoller,
					public S3DBasicInertialPoser
	{
	public:
		enum	ParameterIndex
		{
			paramFreeAngle,
			paramAttenuation,
			paramHardness,
			paramEffect,
			paramBoneCount,
			paramBoneTarget0,
		} ;

	protected:
		SSystem::SObjectArray
			<SSystem::SString>	m_aBoneParamIDs ;
		SSystem::SObjectArray
			<SSystem::SString>	m_aBoneParamDisps ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DInertialPoseController, PoseConntoller )
		S3D_DECLARE_COMPOSER_ITEM( S3DInertialPoseController, inertial_poser )
		// 構築関数
		S3DInertialPoseController( void ) ;
		// ターゲットボーン最大数変更（プロパティ設定）
		void UpdateTargetBoneProperties( size_t nCount ) ;

	public:
		// パラメータ値取得
		virtual double GetScalarParameter( size_t iParam ) const ;
		virtual int32_t GetIntegerParameter( size_t iParam ) const ;
		virtual const wchar_t * GetCommandParameter( size_t iParam ) const ;
		// パラメータ値設定
		virtual void SetScalarParameter( size_t iParam, double s ) ;
		virtual void SetIntegerParameter( size_t iParam, int32_t n ) ;
		virtual void SetCommandParameter( size_t iParam, const wchar_t * pwszCmd ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;

	public:
		// タイマー処理
		virtual void OnTimer
			( S3DScene& scene,
				S3DSceneComposer::ItemSerializer * pItem, uint32_t msecPast ) ;
		// フレーム（パラメータ）更新後処理
		virtual void OnUpdateFrame
			( S3DSceneComposer::ItemSerializer * pItem,
				double fpFrame, S3DSceneComposer::SeekMethod seek ) ;

	public:
		// ポーズ適用
		virtual void OnPoseTrack
			( S3DDynamicModelSerializer& item, S3DModelBuffer& model ) ;
	protected:
		// モデル変更時の処理
		virtual void OnChangedModel
			( S3DDynamicModelSerializer& item, S3DModelBuffer * pModel ) ;

	} ;



	//////////////////////////////////////////////////////////////////////////
	// 動的モデルアイテム（複数描画対応）
	//////////////////////////////////////////////////////////////////////////

	class	S3DMultiModelSerializer
					: public S3DDynamicModelSerializer,
							public S3DParticleSerializer::RenderTarget,
							public S3DInstancingItemInterface
	{
	public:
		enum	ParameterIndex
		{
			paramInstancing		= S3DDynamicModelSerializer::paramModelTotalCount,
			paramInstanceRotate,
			paramInstanceZoom,
			paramCollisionAlpha,
			paramForceFaceMethod,
			paramSortingMethod,
			paramCullingMethod,
			paramCullingNearZ,
			paramCullingFarZ,
			paramCullingOffset,
			paramCullingAngleGap,
			paramMultiModelTotalCount,
			paramMultiModelCount	= paramMultiModelTotalCount - paramInstancing,
		} ;
		enum	ForceFaceMethod
		{
			faceNoOperation,
			faceRotateOnY,
			faceBillboard,
			faceMethodCount,
		} ;

	protected:
		static const S3DSceneComposer::ParamEntry		m_paramEntries[paramMultiModelCount] ;
		static const S3DSceneComposer::ParamSetClass	m_pscClass ;
		static const SSystem::SXMLDocument::AttrInteger	m_aiFaceMethod[faceMethodCount+1] ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO3
			( S3DMultiModelSerializer,
				S3DDynamicModelSerializer,
				RenderTarget, S3DInstancingItemInterface )
		S3D_DECLARE_COMPOSER_ITEM( S3DMultiModelSerializer, multi_model )
		// 構築関数
		S3DMultiModelSerializer( void ) ;

	protected:
		S3DItemInstancingSerializer	m_instancing ;
		int32_t						m_nCollisionAlpha ;
		ForceFaceMethod				m_forceFace ;
		double						m_fpCullingOffset ;
		double						m_fpCullingAngle ;
		SSystem::SArray<S4DMatrix>	m_aTempMatrixs ;
		SSystem::SArray<S3DColor>	m_aTempColors ;

	public:
		// インスタンシング
		S3DItemInstancingSerializer& Instancing( void ) 
		{
			return	m_instancing ;
		}
		// カリングオフセット
		double GetCullingOffset( void ) const
		{
			return	m_fpCullingOffset ;
		}
		void SetCullingOffset( double fpOffset )
		{
			m_fpCullingOffset = fpOffset ;
		}
		double GetCullingAngleGap( void ) const
		{
			return	m_fpCullingAngle ;
		}
		void SetCullingAngleGap( double degAngle )
		{
			m_fpCullingAngle = degAngle ;
		}
		// 動的インスタンス追加（classPreRender で呼び出す）
		virtual void AddDynamicInstancingEntries
			( const S4DMatrix * pMatrixs,
					const S3DColor * pColors,
					size_t nCount, ESLObject * pSourceItem ) ;
		// S3DItemInstancingSerializer 取得
		virtual S3DItemInstancingSerializer * GetInstancing( void ) ;

	public:	// S3DSceneComposer::Parameter
		// パラメータ値取得
		virtual S3DDMatrix GetMatrixParameter( size_t i ) const ;
		virtual S3DDVector GetVectorParameter( size_t i ) const ;
		virtual double GetScalarParameter( size_t i ) const ;
		virtual int32_t GetIntegerParameter( size_t i ) const ;
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		virtual size_t GetBinaryParameter
			( void * pDst, size_t nBufBytes, size_t i ) const ;
		// パラメータ値設定
		virtual void SetMatrixParameter( size_t i, const S3DDMatrix& mat ) ;
		virtual void SetVectorParameter( size_t i, const S3DDVector& vec ) ;
		virtual void SetScalarParameter( size_t i, double s ) ;
		virtual void SetIntegerParameter( size_t i, int32_t n ) ;
		virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		virtual size_t SetBinaryParameter
			( size_t i, const void * pSrc, size_t nBufBytes ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;

	public:	// S3DSceneComposer::ParameterProperty
		// パラメーター有効性
		virtual bool IsParameterValidation( size_t i ) const ;
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;

	public:	// S3DScene::ModelItem
		// 当たり判定追加
		virtual void RenderLocalCollision
			( const S3DScene& scene, S3DCollision& render ) ;
		// 表示モデル追加
		virtual void RenderLocalModel
			( const S3DScene& scene,
				S3DRenderContextInterface& render,
				uint64_t flagsExclusion = 0 ) ;
	protected:
		void RenderMultiInstance
				( const S3DScene& scene,
					S3DRenderContextInterface& render,
					S3DItemInstancingSerializer::RenderingType type,
					uint64_t flagsExclusion = 0,
					size_t iFirst = 0, ssize_t iEnd = -1,
					size_t nInstancing = 0,
					const S4DMatrix * pmatInstancing = nullptr,
					const S3DColor * pColorInstancing = nullptr,
					const S3DItemInstancingSerializer::SortIndex * pSortIndexes = nullptr,
					size_t nOpaqueCount = 0 ) ;

	public:	// S3DSceneComposer::ItemSerializer
		// フレーム更新後処理
		void OnUpdateFrame( double fpFrame, S3DSceneComposer::SeekMethod seek ) ;

	public:	// S3DScene::Item
		// アイテム作用の追加処理
		virtual void OnUpdateBehavior( S3DScene& scene ) ;
		// レンダリング前後処理（全視点共通）
		virtual void OnRenderEvent
			( S3DScene& scene, S3DScene::ItemClass clsItem ) ;

	public:
		// Loquaty クラス名
		virtual const wchar_t * GetLQClassName( void ) const ;

	public:	// S3DParticleSerializer::RenderTarget
		// アニメーション長取得
		virtual bool GetTargetAnimationLength( double& secLength ) const ;
		// 全フレーム数取得
		virtual size_t GetTargetAnimationFrames( void ) const ;
		// ターゲット空間（逆変換用）
		virtual void GetTargetSpaceTransformation
				( S3DDMatrix& matITarget, S3DDVector& vITarget ) ;
		// パーティクル追加（classPreRender で呼び出す）
		virtual void AddParticles
			( size_t nCount,
				const S3DVector4 * pvPoints,
				const size_t * pFrames = NULL,
				const S3DColor * pColors = NULL,
				const float32_t * pZooms = NULL,
				const S4DVector * pFaceDirs = NULL,
				const float32_t * pxAspect  = NULL ) ;
		// AddIndexedParticles を使うか？
		virtual bool IsUsingIndexedParticles( void ) ;
		// パーティクル追加（高機能）（classPreRender で呼び出す）
		virtual void AddIndexedParticles
			( size_t nCount,
				const S3DVector4 * pvPoints,
				const S3DParticleSerializer::ParticleIndex * pIndexes,
				const size_t * pFrames = NULL,
				const S3DColor * pColors = NULL,
				const S3DMatrix * pFaceDirs = NULL ) ;
	} ;



	//////////////////////////////////////////////////////////////////////////
	// LOD モデルインスタンス描画コントローラー
	//////////////////////////////////////////////////////////////////////////

	class	S3DLODModelInstanceController
					: public S3DSceneComposer::Controller,
						public S3DItemInstancingSerializer::MultiRenderer
	{
	public:
		enum	ParameterIndex
		{
			paramLODModel,
			paramLODScale,
			paramDistance,
			paramBothBand,
			paramBillboard,
			paramBillboardRotAlpha,
			paramBillboardTop,
			paramBillboardFront,
			paramBillboardDip,
			paramCount,
		} ;

	protected:
		SSystem::SString	m_strLODModel ;
		S3DModelBuffer		m_modelRef ;
		S3DDVector			m_vModelScale ;
		double				m_fpLODDistance ;
		double				m_fpLODBothBand ;
		bool				m_flagBillboard ;
		bool				m_flagBillboardRotAlpha ;
		S3DDVector			m_vBillboardTop ;
		S3DDVector			m_vBillboardFront ;
		double				m_degBillboardDip ;

		SSystem::SArray<S4DMatrix>	m_aRenderMatrices ;
		SSystem::SArray<S3DColor>	m_aRenderColors ;

		SSystem::SArray<S4DMatrix>	m_aTempMatrices ;
		SSystem::SArray<S3DColor>	m_aTempColors ;
		SSystem::SArray
			<S3DItemInstancingSerializer::SortIndex>	m_aTempIndexes ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO2( S3DLODModelInstanceController, Controller, MultiRenderer )
		S3D_DECLARE_COMPOSER_ITEM( S3DLODModelInstanceController, lod_instance_renderer )
		// 構築関数
		S3DLODModelInstanceController( void ) ;
		S3DLODModelInstanceController( const wchar_t * pwszClassID ) ;
		// プロパティ設定
		void InitProperties( void ) ;
		// モデル参照更新
		void UpdateModelRef( void ) ;

	public:
		// パラメータ値取得
		virtual S3DDVector GetVectorParameter( size_t iParam ) const ;
		virtual double GetScalarParameter( size_t iParam ) const ;
		virtual bool GetBooleanParameter( size_t iParam ) const ;
		virtual const wchar_t * GetCommandParameter( size_t iParam ) const ;
		// パラメータ値設定
		virtual void SetVectorParameter( size_t iParam, const S3DDVector& vec ) ;
		virtual void SetScalarParameter( size_t iParam, double s ) ;
		virtual void SetBooleanParameter( size_t iParam, bool b ) ;
		virtual void SetCommandParameter( size_t iParam, const wchar_t * pwszCmd ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;
		// アイテムプロパティのリソース等の参照を更新する
		virtual uint32_t UpdatePropertyReference
			( S3DSceneComposer::Composition& comp,
				S3DSceneComposer::ItemSerializer * pItem, uint32_t nFlags ) ;

	public:	// MultiRenderer
		// 描画処理
		virtual S3DItemInstancingSerializer::RenderResult
			RenderMultiInstance
				( const S3DScene& scene,
					S3DRenderContextInterface& render,
					const S3DItemInstancingSerializer& instancing,
					S3DItemInstancingSerializer::RenderingType type,
					S3DItemInstancingSerializer::RenderInfo& info ) ;
		// LOD 描画
		virtual void OnRenderLODInstance
			( const S3DScene& scene,
				S3DRenderContextInterface& render,
				S3DVertexBufferInterface * pOrgModel,
				uint64_t flagsExclusion,
				size_t iMeshFirst, ssize_t iMeshEnd,
				size_t nInstanceCount,
				const S4DMatrix * pInstanceMatrices,
				const S3DColor * pInstanceColors ) ;
	} ;



	//////////////////////////////////////////////////////////////////////////
	// LOD モデル用ユーザーシェーダー
	//////////////////////////////////////////////////////////////////////////

	class	S3DLODModelShaderController
			: public S3DLODModelInstanceController, public S3DUserShaderSerializer
	{
	public:
		enum	ParameterIndex
		{
			paramShaderID	= S3DLODModelInstanceController::paramCount,
			paramFirstUniform,
		} ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DLODModelShaderController, S3DLODModelInstanceController )
		S3D_DECLARE_COMPOSER_ITEM( S3DLODModelShaderController, lod_user_shader )
		// 構築関数
		S3DLODModelShaderController( void ) ;
		// 消滅関数
		virtual ~S3DLODModelShaderController( void ) ;

	public:
		// シェーダ―更新
		void UpdateShaderReference( bool flagSwitchShader ) ;
		// パラメータ保存
		void SaveShaderParameters
			( SSystem::SStrSortObjectArray
					<S3DSceneComposer::ParameterEntryStorage>& ssoaParam ) ;
		// パラメータ復帰
		void ResotreShaderParameters
			( const SSystem::SStrSortObjectArray
					<S3DSceneComposer::ParameterEntryStorage>& ssoaParam ) ;

	protected:
		SSystem::SString	m_strPropShaderID ;

	public:
		// パラメータ値取得
		virtual S3DDMatrix GetMatrixParameter( size_t iParam ) const ;
		virtual S3DDVector GetVectorParameter( size_t iParam ) const ;
		virtual double GetScalarParameter( size_t iParam ) const ;
		virtual int32_t GetIntegerParameter( size_t iParam ) const ;
		virtual const wchar_t * GetCommandParameter( size_t iParam ) const ;
		virtual size_t GetBinaryParameter
			( void * pDst, size_t nBufBytes, size_t iParam ) const ;
		// パラメータ値設定
		virtual void SetMatrixParameter( size_t iParam, const S3DDMatrix& mat ) ;
		virtual void SetVectorParameter( size_t iParam, const S3DDVector& vec ) ;
		virtual void SetScalarParameter( size_t iParam, double s ) ;
		virtual void SetIntegerParameter( size_t iParam, int32_t n ) ;
		virtual void SetCommandParameter( size_t iParam, const wchar_t * pwszCmd ) ;
		virtual size_t SetBinaryParameter
			( size_t iParam, const void * pSrc, size_t nBufBytes ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;

	public:
		// アイテムプロパティのリソース等の参照を更新する
		virtual uint32_t UpdatePropertyReference
			( S3DSceneComposer::Composition& comp,
				S3DSceneComposer::ItemSerializer * pItem, uint32_t nFlags ) ;
		// タイマー処理
		virtual void OnTimer
			( S3DScene& scene,
				S3DSceneComposer::ItemSerializer * pItem, uint32_t msecPast ) ;

	public:	// S3DLODModelInstanceController
		// LOD 描画
		virtual void OnRenderLODInstance
			( const S3DScene& scene,
				S3DRenderContextInterface& render,
				S3DVertexBufferInterface * pOrgModel,
				uint64_t flagsExclusion,
				size_t iMeshFirst, ssize_t iMeshEnd,
				size_t nInstanceCount,
				const S4DMatrix * pInstanceMatrices,
				const S3DColor * pInstanceColors ) ;
	} ;



	//////////////////////////////////////////////////////////////////////////
	// モデル描画順序制御コントローラー
	//////////////////////////////////////////////////////////////////////////

	class	S3DModelPartialRenderController
							: public S3DSceneComposer::Controller
	{
	public:
		enum	ParameterIndex
		{
			paramRenderClass,
			paramRenderPriority,
			paramPartialFlag1,
			paramPartialFlag2,
			paramPartialFlag3,
			paramPartialFlag4,
		} ;

	protected:
		S3DScene::ItemClass	m_clsRender ;
		int32_t				m_nPriority ;
		uint64_t			m_flagPartial[4] ;
		uint64_t			m_flagsSaveExclusion ;
		uint64_t			m_flagsSaveRequest ;

		static const SSystem::SXMLDocument::AttrInteger	m_xaiFlagPairs[14] ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DModelPartialRenderController, Controller )
		S3D_DECLARE_COMPOSER_ITEM( S3DModelPartialRenderController, partial_renderer )
		// 構築関数
		S3DModelPartialRenderController( void ) ;

	public:
		// アイテムクラス
		S3DScene::ItemClass GetRenderItemClass( void ) const ;
		void SetRenderItemClass( S3DScene::ItemClass clsItem ) ;
		// 描画プライオリティ
		int32_t GetRenderPriority( void ) const ;
		void SetRenderPriority( int32_t nPriority ) ;
		// 選択フラグ
		uint64_t GetPartialFlagAt( size_t i ) const ;
		void SetPartialFlagAt( size_t i, uint64_t flag ) ;

	public:	// Parameter
		// パラメータ値取得
		virtual int32_t GetIntegerParameter( size_t i ) const ;
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		// パラメータ値設定
		virtual void SetIntegerParameter( size_t i, int32_t n ) ;
		virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;

	public:	// Controller
		// 描画前処理
		virtual void BeforeRenderModel
			( const S3DScene& scene,
				S3DScene::ItemClass clsItem,
				S3DSceneComposer::ItemSerializer * pItem,
				S3DRenderContextInterface& render,
				uint64_t flagsExclusion = 0 ) ;
		// 描画後処理
		virtual void AfterRenderModel
			( const S3DScene& scene,
				S3DScene::ItemClass clsItem,
				S3DSceneComposer::ItemSerializer * pItem,
				S3DRenderContextInterface& render,
				uint64_t flagsExclusion = 0 ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// ビルボード表示アイテム
	//////////////////////////////////////////////////////////////////////////

	class	S3DMeshBufferItemSerializer ;
	class	S3DBillboardSerializer
				: public S3DScene::BillboardItem,
						public S3DSceneComposer::ItemCommonSerializer,
						public S3DParticleSerializer::RenderTarget
	{
	public:
		enum	ParameterIndex
		{
			paramImage			= ItemCommonSerializer::paramItemTotalCount,
			paramMeshTarget,
			paramBillboardCenterX,
			paramBillboardCenterY,
			paramBillboardZoomX,
			paramBillboardZoomY,
			paramBillboardAngle,
			paramZBias,
			paramZOperation,
			paramAlphaDither,
			paramBillboardVisile,
			paramBillboardAutoNormal,
			paramBillboardNormal,
			paramBillboardTotalCount,
			paramBillboardCount		= paramBillboardTotalCount - paramImage,
		} ;

	protected:
		static const S3DSceneComposer::ParamEntry		m_paramEntries[paramBillboardCount] ;
		static const S3DSceneComposer::ParamSetClass	m_pscClass ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO3
			( S3DBillboardSerializer,
				BillboardItem, ItemCommonSerializer, RenderTarget )
		S3D_DECLARE_COMPOSER_ITEM( S3DBillboardSerializer, billboard )
		// 構築関数
		S3DBillboardSerializer( void ) ;

	protected:
		SSystem::SString		m_strMeshTarget ;
		SSystem::SSyncReference	m_refMeshTarget ;

		SSystem::SString	m_strImageID ;
		S2DDVector			m_vImageCenter ;
		S2DDVector			m_vImageZoom ;
		bool				m_visibleBillboard ;
		bool				m_flagAutoNormal ;
		S3DDVector			m_vBillboardNormal ;

		SSystem::SCriticalSection	m_csLock ;

	public:
		// 出力先設定
		void AttachMeshTarget
			( S3DMeshBufferItemSerializer * pMesh, const wchar_t * pwszMeshID ) ;
		bool UpdateMeshTarget( void ) ;
		// 画像設定
		void AttachBillboardImage
				( SGLImageObject * pImage, const wchar_t * pwszID ) ;
		// 画像基準座標
		void SetBillboardCenter( const S2DVector& vCenter ) ;
		// 画像拡大率
		void SetBillboardZoom( const S2DVector& vZoom ) ;
		// 画像回転角 [deg]
		void SetBillboardAngle( float32_t degAngle ) ;
		// ｚバイアス
		void SetBillboardBiasZ( float32_t zBias ) ;
		// ビルボード表示
		void SetBillboardVisible( bool fVisible ) ;
		bool IsBillboardVisible( void ) const ;
		// ビルボード向き
		void SetBillboardAutoNormal( bool flagAuto ) ;
		bool IsBillboardAutoNormal( void ) const ;
		void SetBillboardNormal( const S3DDVector& vNormal ) ;
		const S3DDVector& GetBillboardNormal( void ) const ;
		// 拡大率と画像中心座標を反映
		void UpdateBillboardCenterAndZoom( void ) ;

	public:	// Parameter
		// パラメータ値取得
		virtual S3DDVector GetVectorParameter( size_t i ) const ;
		virtual double GetScalarParameter( size_t i ) const ;
		virtual bool GetBooleanParameter( size_t i ) const ;
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		// パラメータ値設定
		virtual void SetVectorParameter( size_t i, const S3DDVector& vec ) ;
		virtual void SetScalarParameter( size_t i, double s ) ;
		virtual void SetBooleanParameter( size_t i, bool b ) ;
		virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;

	public:	// ItemSerializer
		// パラメーター有効性
		virtual bool IsParameterValidation( size_t i ) const ;
		// レンダリング前後処理（全視点共通）
		virtual void OnItemRenderEvent
			( S3DScene& scene, S3DScene::ItemClass clsItem ) ;
		// アイテムプロパティのリソース等の参照を更新する
		virtual uint32_t UpdatePropertyReference
			( S3DSceneComposer::Composition& comp, uint32_t nFlags ) ;

	protected:
		void AddBillboardParticleMesh
			( S3DScene& scene, S3DVertexBufferInterface& vb ) ;

	public:
		// シーン取得
		virtual S3DScene * GetScene( void ) const ;
		// タイマ処理 Item::OnTimer / ItemSerializer::OnTimer 実装
		virtual void OnTimer( S3DScene& scene, uint32_t msecPast ) ;
		// アイテム作用の追加処理
		virtual void OnUpdateBehavior( S3DScene& scene ) ;
		// 表示モデル追加
		virtual void RenderLocalModel
			( const S3DScene& scene,
				S3DRenderContextInterface& render,
				uint64_t flagsExclusion = 0 ) ;

	public:	// RenderTarget
		// アニメーション長取得
		virtual bool GetTargetAnimationLength( double& secLength ) const ;
		// 全フレーム数取得
		virtual size_t GetTargetAnimationFrames( void ) const ;
		// ターゲット空間（逆変換用）
		virtual void GetTargetSpaceTransformation
				( S3DDMatrix& matITarget, S3DDVector& vITarget ) ;
		// パーティクル追加
		virtual void AddParticles
			( size_t nCount,
				const S3DVector4 * pvPoints,
				const size_t * pFrames = NULL,
				const S3DColor * pColors = NULL,
				const float32_t * pZooms = NULL,
				const S4DVector * pFaceDirs = NULL,
				const float32_t * pxAspect = NULL ) ;
	protected:
		void FitFaceDirArray( size_t nLength ) ;
	public:
		// AddIndexedParticles を使うか？
		virtual bool IsUsingIndexedParticles( void ) ;
		// パーティクル追加（高機能）（classPreRender で呼び出す）
		virtual void AddIndexedParticles
			( size_t nCount,
				const S3DVector4 * pvPoints,
				const S3DParticleSerializer::ParticleIndex * pIndexes,
				const size_t * pFrames = NULL,
				const S3DColor * pColors = NULL,
				const S3DMatrix * pFaceDirs = NULL ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 文字列ビルボード
	//////////////////////////////////////////////////////////////////////////

	class	S3DStringBillboardRenderer
	{
	public:
		// 文字列ビルボード・表示インスタンス
		class	EntryList
				: public SSystem::SArray<S3DMeshShaper::BillboardEntry>
		{
		public:
			SGLSize	m_sizeExt ;		// 外接サイズ

		public:
			// 構築関数
			EntryList( void ) {}
			EntryList( const EntryList& src )
				: SArray<S3DMeshShaper::BillboardEntry>( src ),
					m_sizeExt( src.m_sizeExt ) {}
			// 外接サイズ
			const SGLSize& GetExternalSize( void ) const
			{
				return	m_sizeExt ;
			}
			// 座標設定
			void SetPosition( const S3DVector& vPosition ) ;
			// 中心座標加算
			void AddOffsetPosition( const S2DVector& vOffset ) ;
			// 拡大率設定
			void SetZoom( const S2DVector& vZoom ) ;
		} ;

		// 画像エントリ
		struct	ImageEntry
		{
			SGLImageObject *	pImage ;
			SGLImageRect		rect ;
			ssize_t				iFrame ;

			ImageEntry( void ) : pImage( nullptr ), iFrame( -1 ) {}
			ImageEntry( const ImageEntry& ie )
				: pImage( ie.pImage ), rect( ie.rect ), iFrame( ie.iFrame ) {}
		} ;

	protected:
		int				m_nKerning ;
		int				m_nLinePitch ;

		SSystem::SSortArray
			< SSystem::SSortElement<wchar_t,ImageEntry> >
						m_mapCharImage ;

	public:
		// 構築関数
		S3DStringBillboardRenderer( void ) ;

	public:
		// カーニング（文字間幅）[pixel]
		int GetKerning( void ) const ;
		void SetKerning( int nKerning ) ;
		// 行間 [pixel]
		int GetLinePitch( void ) const ;
		void SetLinePitch( int nPitch ) ;

	public:
		// 全画像設定クリア
		void ClearAllCharImages( void ) ;
		// 文字画像設定
		void SetCharImage
			( wchar_t wch,
				SGLImageObject * pImage,
				const SGLImageRect * pRect = nullptr,
				ssize_t iFrame = -1 ) ;
		// 文字画像検索
		const ImageEntry * GetCharImage( wchar_t wch ) const ;

	public:
		// 表示インスタンス生成
		void MakeBillboard
			( EntryList& billboard,
				const wchar_t * pwszString,
				const S3DVector& vPosition,
				const S2DVector& vZoom, const S3DColor& clrEffect ) ;
	} ;

	class	S3DStringBillboardSerializer
					: public S3DSceneComposer::ItemBasicSerializer,
						public S3DStringBillboardRenderer
	{
	public:
		enum	ParameterIndex
		{
			paramHorzAlign	= ItemBasicSerializer::paramItemTotalCount,
			paramVertAlign,
			paramKerning,
			paramLinePitch,
			paramString,
			paramPixelDensity,
			paramBiasZ,
			paramZBufOperation,
			paramCharSet,
			paramTileFontImage,
			paramTileWidth,
			paramTileHeight,
			paramCharImageCount,
			paramCharImage0,
			paramTotalCountWithoutCharImages	= paramCharImage0,
			paramStringBillboardCount			= paramTotalCountWithoutCharImages - paramHorzAlign,
		} ;
		enum	HorizontalAlignment
		{
			horzAlignCenter,
			horzAlignLeft,
			horzAlignRight,
		} ;
		enum	VerticalAlignment
		{
			vertAlignCenter,
			vertAlignTop,
			vertAlignBottom,
		} ;
		enum	ZBufferOperation
		{
			zbufferWrite,
			zbufferNoWrite,
			zbufferNoCompare,
		} ;

		static const SSystem::SXMLDocument::AttrInteger	s_aiHorzAlignPairs[4] ;
		static const SSystem::SXMLDocument::AttrInteger	s_aiVertAlignPairs[4] ;
		static const SSystem::SXMLDocument::AttrInteger	s_aiZBufOperationPairs[4] ;

		// パーティクル
		class	Particle	: public SSystem::SObject
		{
		public:
			SSystem::SString	m_strBillboard ;
			S3DVector			m_vPosition ;
			S2DVector			m_vZoom ;
			S3DColor			m_clrEffect ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( Particle, SObject )
			// タイマー処理
			virtual void OnTimer
				( S3DScene& scene,
					S3DStringBillboardSerializer& item, uint32_t msecPast ) = 0 ;
			// 寿命判定
			virtual bool IsLiving( void ) = 0 ;
		} ;

	protected:
		static const S3DSceneComposer::ParamEntry		m_paramEntries[paramStringBillboardCount] ;
		static const S3DSceneComposer::ParamSetClass	m_pscClass ;

		S3DMeshShaper					m_meshShaper ;
		SSystem::SArray<S3DMeshShaper::BillboardEntry>
										m_aBillboardEntryBuf ;
		EntryList						m_billboardTemp ;
		SSystem::SObjectArray<Particle>	m_aParticles ;
		SSystem::SCriticalSection		m_csParticle ;

		HorizontalAlignment	m_horzAlign ;
		VerticalAlignment	m_vertAlign ;

		SSystem::SString	m_strBillboard ;

		double				m_fpPixelDensity ;
		double				m_zBias ;
		ZBufferOperation	m_zbufOperation ;

		bool				m_flagUpdateCharImageRef ;
		bool				m_flagUpdateCharImage ;
		SSystem::SString	m_strCharSetExpr ;
		SSystem::SString	m_strCharSetArray ;

		SSystem::SString	m_strTileFontImage ;
		SGLImageObject *	m_pTileFontImage ;
		SGLSize				m_sizeTileFont ;
		size_t				m_nCharImageCount ;

		SSystem::SObjectArray<SSystem::SString>	m_aCharImageIDs ;
		SSystem::SPointerArray<SGLImageObject>	m_aCharImages ;

		SSystem::SArray<S3DSceneComposer::ParamEntry>	m_aCharImageProps ;
		SSystem::SObjectArray<SSystem::SString>			m_aCharImagePropIDs ;
		SSystem::SObjectArray<SSystem::SString>			m_aCharImagePropNames ;


	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DStringBillboardSerializer, ItemBasicSerializer )
		S3D_DECLARE_COMPOSER_ITEM( S3DStringBillboardSerializer, string_billboard )
		// 構築関数
		S3DStringBillboardSerializer( void ) ;
		// 消滅関数
		virtual ~S3DStringBillboardSerializer( void ) ;
		// 文字画像数に応じたプロパティ設定
		void UpdateCharImageCountProps( void ) ;

	public:
		// パーティクル追加
		void AddParticle( Particle * pParticle ) ;

	public:
		// アライメント
		HorizontalAlignment GetHorizontalAlignment( void ) const ;
		VerticalAlignment GetVerticalAlignment( void ) const ;
		void SetHorizontalAlignment( HorizontalAlignment align ) ;
		void SetVerticalAlignment( VerticalAlignment align ) ;
		// ピクセル密度（拡大率^-1）
		double GetPixelDensity( void ) const ;
		void SetPixelDensity( double density ) ;
		// ｚバイアス
		double GetZBias( void ) const ;
		void SetZBias( double zBias ) ;
		// ｚバッファ操作
		ZBufferOperation GetZBufferOperation( void ) const ;
		void SetZBufferOperation( ZBufferOperation zbufOp ) ;

	public:
		// 文字画像参照再設定
		void UpdateCharImagesRef( void ) ;
		// 文字画像再設定
		void UpdateCharImages( void ) ;

	public:	// Parameter
		// パラメータ値取得
		virtual double GetScalarParameter( size_t iParam ) const ;
		virtual int32_t GetIntegerParameter( size_t iParam ) const ;
		virtual const wchar_t * GetCommandParameter( size_t iParam ) const ;
		// パラメータ値設定
		virtual void SetScalarParameter( size_t iParam, double s ) ;
		virtual void SetIntegerParameter( size_t iParam, int32_t n ) ;
		virtual void SetCommandParameter( size_t iParam, const wchar_t * pwszCmd ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;

	public:	// ItemSerializer
		// パラメータエントリ取得
		virtual const S3DSceneComposer::ParamEntry *
							GetParameterEntryAt( size_t iParam ) const ;
		// パラメータ総数
		virtual size_t GetParameterCount( void ) const ;
		// パラメータ指標検索
		virtual ssize_t FindParameterID( const wchar_t * pwszID ) const ;
		// パラメーター有効性
		virtual bool IsParameterValidation( size_t iParam ) const ;
		// アイテムプロパティのリソース等の参照を更新する
		virtual uint32_t UpdatePropertyReference
			( S3DSceneComposer::Composition& comp, uint32_t nFlags ) ;
		// タイマー処理
		virtual void OnTimer( S3DScene& scene, uint32_t msecPast ) ;
		// 表示モデル追加
		virtual void OnItemRenderModel
			( const S3DScene& scene,
				S3DRenderContextInterface& render,
				uint64_t flagsExclusion = 0 ) ;

	public:
		// 表示インスタンス生成
		void MakeBillboard
			( EntryList& billboard,
				const wchar_t * pwszString,
				const S3DVector& vPosition,
				const S2DVector& vZoom, const S3DColor& clrEffect ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 単純な文字列ビルボード・パーティクル
	//////////////////////////////////////////////////////////////////////////

	class	S3DSimpleStringBillboardParticle
				: public S3DStringBillboardSerializer::Particle
	{
	protected:
		SGLBezierCurves<S3DVector>	m_bzCurve ;
		SGLBezierCurves<S2DVector>	m_bzZoom ;
		uint32_t					m_msecLife ;
		uint32_t					m_msecFadeout ;
		uint32_t					m_msecElapsed ;
		SGLPalette					m_argbMulColor ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DSimpleStringBillboardParticle, Particle )
		// 構築関数
		S3DSimpleStringBillboardParticle( void ) ;
		S3DSimpleStringBillboardParticle
			( const wchar_t * pwszBillboard,
				const S3DVector& vPosition, const S3DVector& vSpeed,
				const S2DVector& vZoom, uint32_t msecLife, uint32_t msecFadeout ) ;

	public:
		// 文字列設定
		void SetString( const wchar_t * pwszBillboard ) ;
		// 色効果（乗算 RGB 及び α）設定
		void SetBaseColor( uint32_t argbMul ) ;
		// 移動パラメータ設定
		void SetMovingParameter
			( const S3DVector& vPosition,
				const S3DVector& vSpeed, uint32_t msecLife ) ;
		// 寿命設定
		void SetLifeTime( uint32_t msecLife, uint32_t msecFadeout ) ;
		// 移動曲線設定
		void SetMoveCurve( const SGLBezierCurves<S3DVector>& bzCurve ) ;
		// 拡大率曲線設定
		void SetZoomCurve( const SGLBezierCurves<S2DVector>& bzZoom ) ;

	public:
		// タイマー処理
		virtual void OnTimer
			( S3DScene& scene,
				S3DStringBillboardSerializer& item, uint32_t msecPast ) ;
		// 寿命判定
		virtual bool IsLiving( void ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// サブコンポジション
	//////////////////////////////////////////////////////////////////////////

	class	S3DSubCompositionSerializer
					: public S3DSceneComposer::ItemBasicSerializer,
						public S3DParticleSerializer::RenderTarget,
						public S3DInstancingItemInterface
	{
	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO3
			( S3DSubCompositionSerializer,
				ItemBasicSerializer, RenderTarget, S3DInstancingItemInterface )
		S3D_DECLARE_COMPOSER_ITEM( S3DSubCompositionSerializer, sub_composition )
		// 構築関数
		S3DSubCompositionSerializer( void ) ;
		// 消滅関数
		virtual ~S3DSubCompositionSerializer( void ) ;

	public:
		enum	ParameterIndex
		{
			paramComposition	= ItemBasicSerializer::paramItemTotalCount,
			paramVisibleComp,
			paramPoolInstance,
			paramTimemapMethod,
			paramAutoStop,
			paramAutoRelease,
			paramTimemapLoop,
			paramTimemapOffset,
			paramTimemapFrame,
			paramSubCompTotalCount,
			paramSubCompositionCount	= paramSubCompTotalCount - paramComposition,
		} ;

	protected:
		static const S3DSceneComposer::ParamEntry		m_paramEntries[paramSubCompositionCount] ;
		static const S3DSceneComposer::ParamSetClass	m_pscClass ;

		bool	m_flagEditMode ;

		SSystem::SString							m_strCompID ;
		const S3DSceneComposer::CompositionInfo *	m_pciCompInfo ;

		enum	TimemapMethod
		{
			timemapOwnerFrame,
			timemapOwnerTimer,
			timemapSequence,
			timemapMethodCount,
		} ;
		static const wchar_t *	m_pwszTimemapMethod[timemapMethodCount] ;

		double							m_fpTimemapFrame ;
		double							m_fpStartOwnerFrame ;
		double							m_fpLastOwnerFrame ;
		TimemapMethod					m_timemapMethod ;
		bool							m_flagCompVisible ;
		bool							m_flagPoolInstance ;
		bool							m_flagAutoStop ;
		bool							m_flagAutoRelease ;
		bool							m_flagLoopTimemap ;
		bool							m_flagInitializeItem ;
		bool							m_flagStartItem ;
		S3DSceneComposer::Composition *	m_pComposition ;

		// 外部処理用コンポジションインスタンス
		SSystem::SPointerArray<S3DSceneComposer::Composition>	m_aInstance ;
		SSystem::SPointerArray<S3DSceneComposer::Composition>	m_aDelayRelease ;
		SSystem::SObjectArray<S3DSceneComposer::Composition>	m_aDelayRemove ;

		// パーティクル表示用コンポジション
		SSystem::SPointerArray<S3DSceneComposer::Composition>	m_aCompositions ;

		// リサイクル用ストック
		SSystem::SObjectArray<S3DSceneComposer::Composition>	m_aCompStock ;

		// 次のフレームで表示するパーティクル情報
		//（サブコンポジションのパーティクル表示は１フレーム遅れる）
		SSystem::SArray<S4DMatrix>	m_aParticleMatrics ;
		SSystem::SArray<S3DColor>	m_aParticleColors ;
		SSystem::SArray<size_t>		m_aParticleFrames ;
		S3DDVector					m_vParticleBase ;
		SSystem::SCriticalSection	m_csLock ;

		// シーンセットアップ情報
		struct	SceneSettingsInfo
		{
			SGLSize								sizeFrame ;
			SSystem::SSmartReference
				<SGLSecondaryViewProducer>		refSVP ;
			SSystem::SSmartReference<S3DScene>	refScene ;
		} ;
		SSystem::SObjectArray<SceneSettingsInfo>	m_aSettings ;

	public:
		// サブコンポジション取得
		S3DSceneComposer::Composition * GetSubComposition( void ) const
		{
			return	m_pComposition ;
		}
		// コンポジション設定
		void SetCompositionInfo
			( const S3DSceneComposer::CompositionInfo * pci,
									const wchar_t * pwszCompID ) ;
		// コンポジション表示設定
		bool IsCompositionVisible( void ) const
		{
			return	m_flagCompVisible ;
		}
		void SetCompositionVisible( bool flagVisible ) ;
		// インスタンス生成
		S3DSceneComposer::Composition * CreateInstance( void ) ;
		S3DSceneComposer::Composition *
			CreateInstance( const Rosetta::RSSmartPtr& ptrUserInstance ) ;
		S3DSceneComposer::Composition *
			CreateInstance( const S3DSceneComposer::ScriptObject& instance ) ;
		// インスタンス削除（リサイクル）
		void ReleaseInstance( S3DSceneComposer::Composition * pComp ) ;
		// インスタンスが有効か？
		bool IsValidInstance( S3DSceneComposer::Composition * pComp ) const ;
		// 有効インスタンス数取得
		size_t GetInstanceCount( void ) const ;
		// 有効インスタンス取得
		S3DSceneComposer::Composition * GetInstanceAt( size_t iInstance ) const ;
		// 遅延削除（リサイクル）設定
		void DelayReleaseInstance( S3DSceneComposer::Composition * pComp ) ;
		// 遅延削除インスタンスを削除（リサイクル）
		void FlushDelayReleaseInstance( void ) ;
		// 遅延削除インスタンスを削除（リサイクル）（インスタンスを削除する場合には非同期）
		void AsyncFlushDelayReleaseInstance( S3DScene& scene ) ;

	protected:
		// インスタンス削除の非同期呼び出し
		class	DelayRemoveInstanceProc
					: public ESLObject, public SSystem::SProcedure
		{
		public:
			SSystem::SObjectArray<S3DSceneComposer::Composition>	m_aDelayRemove ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO2( DelayRemoveInstanceProc, ESLObject, SProcedure )
			// 構築関数
			DelayRemoveInstanceProc
				( SSystem::SObjectArray<S3DSceneComposer::Composition>& aDelayRemove ) ;
			// スレッド関数
			virtual void Run( void ) ;
		} ;
		// SceneSettingsInfo 検索
		ssize_t FindSceneSettingsInfo( SGLSecondaryViewProducer * psvp ) const ;

	public:	// Parameter
		// パラメータ値取得
		virtual double GetScalarParameter( size_t i ) const ;
		virtual bool GetBooleanParameter( size_t i ) const ;
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		// パラメータ値設定
		virtual void SetScalarParameter( size_t i, double s ) ;
		virtual void SetBooleanParameter( size_t i, bool b ) ;
		virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;

	public:	// ItemSerializer
		// パラメーター有効性
		virtual bool IsParameterValidation( size_t i ) const ;
		// アイテムプロパティのリソース等の参照を更新する
		virtual uint32_t UpdatePropertyReference
			( S3DSceneComposer::Composition& comp, uint32_t nFlags ) ;

	public:
		// Loquaty クラス名
		virtual const wchar_t * GetLQClassName( void ) const ;

	public:
		// レンダリング設定
		virtual void OnSetupSceneSettings
			( S3DScene& scene,
				const SGLSize& sizeFrame,
				SGLSecondaryViewProducer * psvp = NULL ) ;
		// レンダリング後始末
		virtual void OnShoutdownSceneSettings
			( S3DScene& scene,
				SGLSecondaryViewProducer * psvp = NULL ) ;
		// 拡張的な処理の通知
		virtual void OnExtendNotify
			( const wchar_t * pwszCmd, const wchar_t * pwszParam,
				const void * pExParam, size_t nExParamBytes ) ;
		// タイマ処理
		virtual void OnTimer( S3DScene& scene, uint32_t msecPast ) ;
		// フレームを適用
		virtual void SetFrameParameters
			( double fpFrame, S3DSceneComposer::SeekMethod seek ) ;
		// アイテム作用の追加処理
		virtual void OnUpdateBehavior( S3DScene& scene ) ;
		// レンダリング前後処理（全視点共通）
		virtual void OnRenderEvent
			( S3DScene& scene, S3DScene::ItemClass clsItem ) ;

	protected:
		// コンポジションのフレーム更新
		bool UpdateCompositionFrame
			( S3DSceneComposer::Composition& comp,
				double& fpFrame, S3DSceneComposer::SeekMethod seek ) const ;
		// コンポジション・インスタンス生成
		S3DSceneComposer::Composition *
			LoadCompositionInstance
				( const S3DSceneComposer::ScriptObject& instance ) ;
		// コンポジション・シャットダウン処理
		void ShoutdownComposition( S3DSceneComposer::Composition& comp ) ;

	public:
		// 規定のレンダリングデバイス設定
		virtual void OnSetRenderDevice( S3DRenderDevice * pDevice ) ;
		// レンダリングの為のデバイスリソース準備
		virtual void OnPrepareToRender
			( S3DRenderDevice * pDevice, uint32_t nFlags = 0 ) ;
		// レンダリング前後処理（全視点共通）
		virtual void OnItemRenderEvent
			( S3DScene& scene, S3DScene::ItemClass clsItem ) ;
		// 当たり判定追加
		//（必要であれば scene.PhysicsScene() へ Actor の追加も行う）
		virtual void OnItemRenderCollision
			( const S3DScene& scene, S3DCollision& render ) ;
		// 表示モデル追加
		virtual void OnItemRenderModel
			( const S3DScene& scene,
				S3DRenderContextInterface& render,
				uint64_t flagsExclusion = 0 ) ;

	public:	// RenderTarget
		// アニメーション長取得
		virtual bool GetTargetAnimationLength( double& secLength ) const ;
		// 全フレーム数取得
		virtual size_t GetTargetAnimationFrames( void ) const ;
		// ターゲット空間（逆変換用）
		virtual void GetTargetSpaceTransformation
				( S3DDMatrix& matITarget, S3DDVector& vITarget ) ;
		// パーティクル追加
		virtual void AddParticles
			( size_t nCount,
				const S3DVector4 * pvPoints,
				const size_t * pFrames = NULL,
				const S3DColor * pColors = NULL,
				const float32_t * pZooms = NULL,
				const S4DVector * pFaceDirs = NULL,
				const float32_t * pxAspect = NULL ) ;
		// AddIndexedParticles を使うか？
		virtual bool IsUsingIndexedParticles( void ) ;
		// パーティクル追加（高機能）（classPreRender で呼び出す）
		virtual void AddIndexedParticles
			( size_t nCount,
				const S3DVector4 * pvPoints,
				const S3DParticleSerializer::ParticleIndex * pIndexes,
				const size_t * pFrames = NULL,
				const S3DColor * pColors = NULL,
				const S3DMatrix * pFaceDirs = NULL ) ;

	public:	// S3DInstancingItemInterface
		// 動的インスタンス追加（classPreRender で呼び出す）
		virtual void AddDynamicInstancingEntries
			( const S4DMatrix * pMatrixs,
				const S3DColor * pColors, size_t nCount, ESLObject * pSrcItem ) ;
		// S3DItemInstancingSerializer 取得
		virtual S3DItemInstancingSerializer * GetInstancing( void ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// サブコンポジション・インスタンス
	//////////////////////////////////////////////////////////////////////////

	class	S3DSubCompInstanceSerializer
					: public S3DSceneComposer::ItemBasicSerializer
	{
	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO
			( S3DSubCompInstanceSerializer, ItemBasicSerializer )
		S3D_DECLARE_COMPOSER_ITEM( S3DSubCompInstanceSerializer, subcomp_instance )
		// 構築関数
		S3DSubCompInstanceSerializer( void ) ;
		// 消滅関数
		virtual ~S3DSubCompInstanceSerializer( void ) ;

	public:
		enum	ParameterIndex
		{
			paramComposition	= ItemBasicSerializer::paramItemTotalCount,
			paramCreateComp,
			paramForceReleaseComp,
			paramTrackPosition,
			paramAutoRecreateCount,
			paramCompInstanceTotalCount,
			paramCompInstanceCount	= paramCompInstanceTotalCount - paramComposition,
		} ;

	protected:
		static const S3DSceneComposer::ParamEntry		m_paramEntries[paramCompInstanceCount] ;
		static const S3DSceneComposer::ParamSetClass	m_pscClass ;

		SSystem::SString				m_strSubComp ;
		SSystem::SSyncReference			m_refSubComp ;
		S3DSceneComposer::Composition *	m_pInstance ;
		bool							m_flagCreateComp ;
		bool							m_flagForceRelease ;
		bool							m_flagTrackPos ;
		size_t							m_nAutoRecreateLimit ;
		size_t							m_nCreatedCount ;

	public:
		// サブコンポジション参照
		void UpdateSubCompRef( void ) ;
		// インスタンス解放
		void ReleaseInstance( void ) ;
		// 有効なインスタンス取得
		S3DSceneComposer::Composition * GetValidInstance( void ) const ;
		// インスタンス生成
		S3DSceneComposer::Composition * CreateInstance( void ) ;
		// インスタンスに行列反映
		void ReflectInstanceParameter( S3DSceneComposer::Composition& comp ) ;

	public:	// Parameter
		// パラメータ値取得
		virtual int32_t GetIntegerParameter( size_t i ) const ;
		virtual bool GetBooleanParameter( size_t i ) const ;
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		// パラメータ値設定
		virtual void SetIntegerParameter( size_t i, int32_t n ) ;
		virtual void SetBooleanParameter( size_t i, bool b ) ;
		virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;

	public:	// ItemSerializer
		// パラメーター有効性
		virtual bool IsParameterValidation( size_t i ) const ;
		// アイテムプロパティのリソース等の参照を更新する
		virtual uint32_t UpdatePropertyReference
			( S3DSceneComposer::Composition& comp, uint32_t nFlags ) ;
		// 拡張的な処理の通知
		virtual void OnExtendNotify
			( const wchar_t * pwszCmd, const wchar_t * pwszParam,
				const void * pExParam, size_t nExParamBytes ) ;
		// タイマー処理
		virtual void OnTimer( S3DScene& scene, uint32_t msecPast ) ;
		// フレーム更新後処理
		virtual void OnUpdateFrame
			( double fpFrame, S3DSceneComposer::SeekMethod seek ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// サウンドアイテム
	//////////////////////////////////////////////////////////////////////////

	class	S3DSoundItemSerializer
				: public S3DScene::SoundItem,
						public S3DSceneComposer::ItemCommonSerializer
	{
	public:
		enum	ParameterIndex
		{
			paramSound			= ItemCommonSerializer::paramItemTotalCount,
			paramVolume,
			paramVolumeLine,
			paramLoop,
			paramPlay,
			paramSeek,
			paramMaxInstance,
			paramFadeVolume,
			paramFadeAutoPlay,
			paramFadeReach,
			paramFadeLatitude,
			paramEnvVolume,
			paramAttenuationPower,
			paramAttenuationDistance,
			paramDirectional,
			paramDirection,
			paramConeAngle,
			paramAngleGradation,
			paramBaseVolume,
			paramSoundTotalCount,
			paramSoundCount	= paramSoundTotalCount - paramSound,
		} ;

	protected:
		static const S3DSceneComposer::ParamEntry		m_paramEntries[paramSoundCount] ;
		static const S3DSceneComposer::ParamSetClass	m_pscClass ;
		static const SSystem::SXMLDocument::AttrInteger	m_aiVolumeLine[32] ;

	protected:
		SSystem::SString		m_strSoundID ;
		SGLAudioPlayer *		m_pAudioRsrc ;
		int32_t					m_iVolumeLine ;
		bool					m_flagScenePlaying ;
		bool					m_flagPlay ;
		bool					m_flagPlaying ;
		bool					m_flagLoop ;
		bool					m_flagAutoPlay ;
		double					m_secSeek ;
		size_t					m_nMaxInstance ;
		double					m_degConeAngle ;
		double					m_degAngleGradation ;

	public:
		// 音源インスタンス
		enum	InstanceFlag
		{
			instanceAutoDeleteOnEnd		= 0x0001,	// 再生完了時にインスタンス破棄
			instanceAutoDeleteWithItem	= 0x0002,	// 参照アイテム削除時にインスタンス破棄
		} ;
		class	Instance	: public SSystem::SObject
		{
		public:
			S3DDMatrix								m_matRotate ;
			S3DDVector								m_vPos ;
			double									m_fpVolume ;
			uint32_t								m_nFlags ;
			SSystem::SSmartPointer
				<SGLAudioPlayerInterface>			m_pPlayer ;
			SSystem::SSmartReference
				<S3DSceneComposer::ItemSerializer>	m_refItem ;
		public:
			ESL_DECLARE_CLASS_INFO( Instance, SObject )
			Instance( void )
				: m_matRotate( 1, 1, 1 ),
					m_vPos( 0, 0, 0 ),
					m_fpVolume( 1.0 ), m_nFlags( 0 ) { }
		} ;

	protected:
		SSystem::SCriticalSection		m_csSync ;
		SSystem::SObjectArray
			<SGLAudioPlayerInterface>	m_aPlayerStock ;
		SSystem::SObjectArray<Instance>	m_aInstance ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO2
			( S3DSoundItemSerializer, SoundItem, ItemCommonSerializer )
		S3D_DECLARE_COMPOSER_ITEM( S3DSoundItemSerializer, sound_item )
		// 構築関数
		S3DSoundItemSerializer( void ) ;
		S3DSoundItemSerializer
			( const wchar_t * pwszClassID,
				const S3DSceneComposer::ParamSetClass * pClass ) ;

	public:
		// サウンド
		void SetSound( const wchar_t * pwszSoundID ) ;
		void UpdateSoundRef( void ) ;
		const wchar_t * GetSoundID( void ) const ;
		SGLAudioPlayer * GetAudioResource( void ) const ;

		// SGLAudioPlayerInterface 参照プレーヤー生成
		SGLAudioPlayerInterface * CreateAudioPlayer( void ) ;
		// SGLAudioPlayerInterface 参照プレーヤー破棄
		void ReleaseAudioPlayer( SGLAudioPlayerInterface * pPlayer ) ;

		// 自動削除インスタンス再生
		Instance * PlayTemporary
			( double fpSubVolume = 1.0,
				S3DSceneComposer::ItemSerializer * pRefItem = NULL,
				const S3DDMatrix * pMatrix = NULL, const S3DDVector * pPos = NULL ) ;
		// インスタンス生成
		Instance * CreateInstance
			( uint32_t nFlags = 0, double fpSubVolume = 1.0,
				S3DSceneComposer::ItemSerializer * pRefItem = NULL,
				const S3DDMatrix * pMatrix = NULL, const S3DDVector * pPos = NULL ) ;
		// インスタンス削除
		void RemoveInstance( Instance * pInstance ) ;
		// 再生開始
		void PlayInstance( Instance * pInstance ) ;
		// インスタンスが存在するか？
		bool IsValidInstance( Instance * pInstance ) const ;
		// インスタンス座標変更
		void SetInstancePosition( Instance * pInstance, const S3DDVector& vPos ) const ;
		// インスタンス再生時間と再生中か照会
		bool GetPlayingTimeOfInstance
			( double& secPlaying, Instance * pInstance ) const ;
		// インスタンス操作同期用
		void LockInstance( void ) const ;
		void UnlockInstance( void ) const ;

	protected:
		// 再生開始
		void OnPlayStart( void ) ;
		// 単体音量反映
		void ApplySoundInstanceVolume
			( const S3DDMatrix& matInstance,
				const S3DDVector& vInstance,
				SGLAudioPlayerInterface * pPlayer, double fpSubVolume ) const ;

	public:	// Parameter
		// パラメータ値取得
		virtual S3DDVector GetVectorParameter( size_t i ) const ;
		virtual double GetScalarParameter( size_t i ) const ;
		virtual int32_t GetIntegerParameter( size_t i ) const ;
		virtual bool GetBooleanParameter( size_t i ) const ;
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		// パラメータ値設定
		virtual void SetVectorParameter( size_t i, const S3DDVector& vec ) ;
		virtual void SetScalarParameter( size_t i, double s ) ;
		virtual void SetIntegerParameter( size_t i, int32_t n ) ;
		virtual void SetBooleanParameter( size_t i, bool b ) ;
		virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;
		// パラメーター有効性
		virtual bool IsParameterValidation( size_t i ) const ;
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;

	public:
		// タイマ処理 Item::OnTimer / ItemSerializer::OnTimer 実装
		virtual void OnTimer( S3DScene& scene, uint32_t msecPast ) ;

	public:	// ItemSerializer
		// フレーム更新後処理
		virtual void OnUpdateFrame
			( double fpFrame, S3DSceneComposer::SeekMethod seek ) ;
		// アイテムプロパティのリソース等の参照を更新する
		virtual uint32_t UpdatePropertyReference
			( S3DSceneComposer::Composition& comp, uint32_t nFlags ) ;
		// 拡張的な処理の通知
		virtual void OnExtendNotify
			( const wchar_t * pwszCmd, const wchar_t * pwszParam,
				const void * pExParam, size_t nExParamBytes ) ;

	protected:
		// シーン再生モード
		void OnScenePlaying( bool flagScenePlaying ) ;
		// サウンド再生状態反映
		void ReflectSoundPlayState( void ) ;

	public:
		// Loquaty クラス名
		virtual const wchar_t * GetLQClassName( void ) const ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// レンズフレア
	//////////////////////////////////////////////////////////////////////////

	class	S3DLensFlareSerializer
				: public S3DSceneComposer::ItemBasicSerializer
	{
	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DLensFlareSerializer, ItemBasicSerializer )
		S3D_DECLARE_COMPOSER_ITEM( S3DLensFlareSerializer, lens_flare )
		// 構築関数
		S3DLensFlareSerializer( void ) ;

	public:
		struct	ImageEntry
		{
			SGLImageObject *	pImage ;
			float32_t			fpPosition ;
			float32_t			fpZoom ;
			S2DVector			vCenter ;
		} ;

	public:
		enum	ParameterIndex
		{
			paramLensFrareCount	= ItemBasicSerializer::paramItemTotalCount,
			paramFadeRadius,
			paramLensFlareTotlaCount,
			paramLensFlareCount			= paramLensFlareTotlaCount - paramLensFrareCount,
		} ;
		enum	ExParameterSubIndex
		{
			paramFlareImage,
			paramFlarePosition,
			paramFlareZoom,
			paramFlareCenterOffset,
			paramExEntryCount,
		} ;

	protected:
		SSystem::SArray<S3DSceneComposer::ParamEntry>	m_aParamEntries ;
		SSystem::SObjectArray<SSystem::SString>			m_aParamStrings ;
		S3DSceneComposer::ParamSetClass					m_pscExClass ;

		static const S3DSceneComposer::ParamEntry		m_paramEntries[paramLensFlareCount] ;
		static const S3DSceneComposer::ParamSetClass	m_pscClass ;

	protected:
		double						m_fpFadeRadius ;
		SSystem::SArray<ImageEntry>	m_aImages ;
		SSystem::SArray<ImageEntry>	m_aExImages ;
		SSystem::SObjectArray<SSystem::SString>	m_aExImageIDs ;

	public:
		// プロパティ追加数
		size_t GetPropExtensionCount( void ) const ;
		void SetPropExtensionCount( size_t nCount ) ;
		// プロパティ項目更新
		void UpdateExtensionProperties( void ) ;
		// 画像参照更新
		void UpdateImageReference
			( S3DSceneComposer::Composition& comp, bool flagForceUpdate ) ;
	protected:
		void AddExPropertyEntry
			( const wchar_t * id,
				S3DSceneComposer::ParameterType type, uint32_t attr,
				const wchar_t * name, const wchar_t * desc = NULL,
				double minRange = 0.0, double maxRange = 1.0 ) ;

	public:
		// 表示画像設定
		void SetImageEntries( const ImageEntry * pImages, size_t nCount ) ;

	public:
		// パラメータ値取得
		virtual double GetScalarParameter( size_t i ) const ;
		virtual int32_t GetIntegerParameter( size_t i ) const ;
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		virtual size_t GetBinaryParameter
			( void * pDst, size_t nBufBytes, size_t i ) const ;
		// パラメータ値設定
		virtual void SetScalarParameter( size_t i, double s ) ;
		virtual void SetIntegerParameter( size_t i, int32_t n ) ;
		virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		virtual size_t SetBinaryParameter
			( size_t i, const void * pSrc, size_t nBufBytes ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;
		// アイテムプロパティのリソース等の参照を更新する
		virtual uint32_t UpdatePropertyReference
			( S3DSceneComposer::Composition& comp, uint32_t nFlags ) ;

	public:
		// 表示モデル追加
		virtual void RenderModel
			( const S3DScene& scene,
				S3DRenderContextInterface& render,
				uint64_t flagsExclusion = 0 ) ;
	} ;


}

#include <sakuraglx/render/sglx3d_scene_item_mesh.h>
#include <sakuraglx/render/sglx3d_scene_item_bullet.h>
#include <sakuraglx/render/sglx3d_scene_canvas.h>
#include <loquaty/gls4_loquaty_scene.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// Loquaty/Rosetta スクリプト・インスタンス・コンテナ
	//////////////////////////////////////////////////////////////////////////

	class	S3DSceneScriptInstance
					: public S3DSceneComposer::Controller,
						public Loquaty::S3DSceneCustomProperty
	{
	protected:
		SSystem::SSmartPointer<Rosetta::RSContext>
								m_pContext ;
		Rosetta::RSClass *		m_pRSClass ;
		Rosetta::RSObject *		m_pInstance ;

		Loquaty::LPtr<Loquaty::LClass>
								m_pLClass ;
		Loquaty::LObjPtr		m_pLObject ;

		SSystem::SString		m_strRSClass ;

		size_t					m_iParamRSClass ;
		size_t					m_iParamBase ;

		class	ParamOption
		{
		public:
			SSystem::SArray
				<SSystem::SXMLDocument::AttrInteger>	m_aStrIntPairs ;
			SSystem::SObjectArray<SSystem::SString>		m_aStrBuffers ;
		} ;
		SSystem::SObjectArray<ParamOption>	m_aOptions ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO2
			( S3DSceneScriptInstance, Controller, S3DSceneCustomProperty )
		S3D_DECLARE_COMPOSER_ITEM
			( S3DSceneScriptInstance, script_obj_instance )
		// 構築関数
		S3DSceneScriptInstance( void ) ;
		// 消滅関数
		virtual ~S3DSceneScriptInstance( void ) ;

	public:
		// クラス名取得
		const SSystem::SString& GetScirptClassName( void ) const
		{
			return	m_strRSClass ;
		}
		// インスタンス取得
		Rosetta::RSObject * GetRSInstance( void ) const ;
		const Loquaty::LObjPtr& GetLoquatyInstance( void ) const ;
		// クラス参照を更新する
		void UpdateClassRef( void ) ;
		// クラスメンバのパラメータに更新
		void UpdateClassMember( void ) ;
		void UpdateRosettaClassMember( void ) ;
		void UpdateLoquatyClassMember( void ) ;
		// パラメータ保存
		void SaveInstanceParameters
			( SSystem::SStrSortObjectArray
					<S3DSceneComposer::ParameterEntryStorage>& ssoaParam ) ;
		// パラメータ復帰
		void ResotreInstanceParameters
			( const SSystem::SStrSortObjectArray
					<S3DSceneComposer::ParameterEntryStorage>& ssoaParam ) ;

	public:	// Controller
		// パラメータ値取得
		virtual S3DDMatrix GetMatrixParameter( size_t i ) const ;
		virtual S3DDVector GetVectorParameter( size_t i ) const ;
		virtual double GetScalarParameter( size_t i ) const ;
		virtual int32_t GetIntegerParameter( size_t i ) const ;
		virtual bool GetBooleanParameter( size_t i ) const ;
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		virtual size_t GetBinaryParameter
			( void * pDst, size_t nBufBytes, size_t i ) const ;
		// パラメータ値設定
		virtual void SetMatrixParameter( size_t i, const S3DDMatrix& mat ) ;
		virtual void SetVectorParameter( size_t i, const S3DDVector& vec ) ;
		virtual void SetScalarParameter( size_t i, double s ) ;
		virtual void SetIntegerParameter( size_t i, int32_t n ) ;
		virtual void SetBooleanParameter( size_t i, bool b ) ;
		virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		virtual size_t SetBinaryParameter
					( size_t i, const void * pSrc, size_t nBufBytes ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;
		// アイテムプロパティのリソース等の参照を更新する
		virtual uint32_t UpdatePropertyReference
			( S3DSceneComposer::Composition& comp,
				S3DSceneComposer::ItemSerializer * pItem, uint32_t nFlags ) ;

	public:	// S3DSceneCustomProperty
		// コンポジション取得
		virtual SakuraGL::S3DSceneComposer::Composition * GetComposition( void ) const ;
	} ;

}

#endif
