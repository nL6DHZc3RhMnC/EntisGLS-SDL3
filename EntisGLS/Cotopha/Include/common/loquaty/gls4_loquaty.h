
#if	!defined(__GLS4_LOQUATY_H__)
#define	__GLS4_LOQUATY_H__

#include <loquaty.h>
#include <sakuraglx/sakuraglx.h>
#include <loquaty/gls4_loquaty_file.h>
#include <sakuragl/sgl2d/sgl_paint_buffer.h>
#include <sakuraglx/extra/sglx_image_composition.h>
#include <sakuraglx/sprite/sglx_sprite_edit.h>
#include <sakuraglx/sprite/sglx_sprite_message.h>
#include <sakuraglx/render/sglx_model_buffer.h>
#include <sakuraglx/render/sglx3d_scene.h>
#include <sakuraglx/render/sglx3d_scene_composer.h>
#include <sakuraglx/render/sglx3d_scene_item.h>
#include <loquaty/gls4_loquaty_scene.h>


using namespace Loquaty ;
using namespace SSystem ;
using namespace SakuraGL ;

using size_t = ::size_t ;
using ssize_t = ::ssize_t ;


namespace Loquaty
{
	//////////////////////////////////////////////////////////////////////////
	// 初期設定関数
	//////////////////////////////////////////////////////////////////////////

	// LURLSchemer::Open を EntisGLS4 のファイルシステムに上書きする
	void SetEntisGLS4FileSystem( void ) ;

	// EntisGLS4 ネイティブ関数を登録
	void DefineEntisGLS4NativeFunctions( LVirtualMachine * vm ) ;

	// 環境変数からインクルードパスを追加
	void AddEnvironmentIncludePath
		( LVirtualMachine * vm,
			const wchar_t * pwszEnvName = L"LOQUATY_INCLUDE_PATH" ) ;


	//////////////////////////////////////////////////////////////////////////
	// Loquaty::Object コンテナ
	//////////////////////////////////////////////////////////////////////////

	class	LReference	: public SSyncReference, public Loquaty::Object
	{
	public:
		LReference( void ) {}
		LReference( const LReference& ref )
			: SSyncReference( ref ) { }
		LReference( SObject * pObj )
			: SSyncReference( pObj ) { }

		static void * operator new ( size_t stObj )
		{
			return	SSyncReference::operator new ( stObj ) ;
		}
		static void * operator new ( size_t stObj, void * ptrObj )
		{
			return	SSyncReference::operator new ( stObj, ptrObj ) ;
		}
		static void * operator new ( size_t stObj, const char * pszFileName, int nLine )
		{
			return	SSyncReference::operator new ( stObj, pszFileName, nLine ) ;
		}
		static void operator delete ( void * ptrObj )
		{
			SSyncReference::operator delete ( ptrObj ) ;
		}

		template <class T> static T * GetNative( LObject * pObj )
		{
			std::shared_ptr<LReference>
					pRef = LNativeObj::GetNative<LReference>( pObj ) ;
			return	(pRef != nullptr) ? pRef->GetRef<T>() : nullptr ;
		}
	} ;

	class	LObjectPtr	: public SSystem::SObject, public LObjPtr
	{
	public:
		ESL_DECLARE_CLASS_INFO( LObjectPtr, SObject )

		LObjectPtr( LObject * p = nullptr ) : LObjPtr( p ) { }
		LObjectPtr( const LObjPtr& ptr ) : LObjPtr( ptr ) { }
		const LObjectPtr& operator = ( const LObjPtr& ptr )
		{
			LObjPtr::operator = ( ptr ) ;
			return	*this ;
		}
	} ;


	//////////////////////////////////////////////////////////////////////////
	// Loquaty NativeObject, struct 型名定義
	//////////////////////////////////////////////////////////////////////////

	typedef	LReference /*ECSSakura2::EnvironmentVM*/	LEntisGLS4_Environment ;

	typedef	LReference /*SGLImageObject*/				LEntisGLS4_Image ;
	typedef	LReference /*SGLPaintContextInterface*/		LEntisGLS4_PaintContext ;
	typedef	LReference /*SGLImageComposition*/			LEntisGLS4_ImageComposition ;
	typedef	LReference /*SGLImageComposition::Layer*/	LEntisGLS4_ImageComposition_Layer ;
	typedef	LObject										LEntisGLS4_PaintParam ;

	typedef	LReference /*S3DMaterial*/					LEntisGLS4_Material ;
	typedef	LReference /*S3DRenderDevice::ShaderDescriptor*/	LEntisGLS4_RenderDevice_ShaderDesc ;
	typedef	LReference /*S3DRenderDevice*/				LEntisGLS4_RenderDevice ;
	typedef	LReference /*SGLSoftwareRenderDevice*/		LEntisGLS4_SoftwareRenderDevice ;
	typedef	LReference /*S3DCustomShader*/				LEntisGLS4_CustomShader ;
	typedef	LReference /*S3DRenderContextInterface*/	LEntisGLS4_RenderContext ;
	typedef	LReference /*S3DRenderBufferInterface*/		LEntisGLS4_RenderBuffer ;
	typedef	LReference /*S3DVertexBufferInterface*/		LEntisGLS4_VertexBuffer ;
	typedef	LObject										LEntisGLS4_VertexBuffer_PrimitiveBuffer ;
	typedef	LObject										LEntisGLS4_VertexBuffer_MeshInfo ;
	typedef	LReference /*S3DVertexVariantBuffer*/		LEntisGLS4_VertexVariantBuffer ;

	typedef	LReference /*SGLAbstractWindow*/			LEntisGLS4_Window ;

	typedef	LReference /*LSpriteTimer*/					LEntisGLS4_SpriteTimer ;
	typedef	LReference /*LSpriteMouseListener*/			LEntisGLS4_SpriteMouseListener ;
	typedef	LReference /*LSpriteKeyListener*/			LEntisGLS4_SpriteKeyListener ;
	typedef	LReference /*SGLSprite*/					LEntisGLS4_Sprite ;
	typedef	LReference /*SGLSpriteFilter*/				LEntisGLS4_SpriteFilter ;
	typedef	LReference /*SGLSpriteFilterTransparencyDrawer*/	LEntisGLS4_SpriteFilterTransparencyDrawer ;
	typedef	LReference /*SGLSpriteFilterBlendAlpha*/	LEntisGLS4_SpriteFilterBlendAlpha ;
	typedef	LReference /*SGLSpriteFilterTone*/			LEntisGLS4_SpriteFilterTone ;
	typedef	LReference /*SGLSpriteFilterShadingOff*/	LEntisGLS4_SpriteFilterShadingOff ;
	typedef	LReference /*SGLSpriteWindow*/				LEntisGLS4_WindowSprite ;
	typedef	LReference /*SGLSpriteRectangle*/			LEntisGLS4_RectangleSprite ;
	typedef	LReference /*SGLSpriteText*/				LEntisGLS4_TextSprite ;
	typedef	LObject										LEntisGLS4_FontStyle ;
	typedef	LObject										LEntisGLS4_TextSprite_TextStyle ;
	typedef	LReference /*SGLSpriteEdit*/				LEntisGLS4_EditSprite ;
	typedef	LObject										LEntisGLS4_EditSprite_ExitStyle ;
	typedef	LReference /*SGLSpriteMessage*/				LEntisGLS4_MessageSprite ;
	typedef	LObject										LEntisGLS4_MessageSprite_RichTextStyle ;
	typedef	LObject										LEntisGLS4_MessageSprite_MessageStyle ;
	typedef	LReference /*SGLSpriteMovie*/				LEntisGLS4_MovieSprite ;
	typedef	LReference /*LRenderableSprite*/			LEntisGLS4_RenderableSprite ;

	typedef	LReference /*SGLVirtualInput*/				LEntisGLS4_VirtualInput ;
	typedef	LObject										LEntisGLS4_VirtualInput_Event ;
	typedef	LObject										LEntisGLS4_VirtualInput_Command ;

	typedef	LReference /*S3DCollider*/					LEntisGLS4_Collider ;
	typedef	LObject										LEntisGLS4_Collider_Result ;
	typedef	LReference /*S3DCollision*/					LEntisGLS4_Collision ;

	typedef	LReference /*S3DScene*/						LEntisGLS4_Scene ;
	typedef	LReference /*S3DSceneSprite*/				LEntisGLS4_SceneSprite ;
	typedef	LReference /*S3DTextureLibrary*/			LEntisGLS4_TextureLibrary ;
	typedef	LReference /*S3DMaterialLibrary*/			LEntisGLS4_MaterialLibrary ;
	typedef	LReference /*S3DModelPose*/					LEntisGLS4_ModelPose ;
	typedef	LReference /*S3DModelPoseLibrary*/			LEntisGLS4_ModelPoseLibrary ;
	typedef	LReference /*S3DModelBoneSpace*/			LEntisGLS4_ModelBone ;
	typedef	LReference /*S3DModelData::MarkerInfo*/		LEntisGLS4_ModelMarker ;
	typedef	LReference /*S3DModelBuffer*/				LEntisGLS4_ModelBuffer ;
	typedef	LReference /*SGLAudioInputStream*/			LEntisGLS4_AudioInputStream ;
	typedef	LReference /*SGLAudioPlayer*/				LEntisGLS4_AudioPlayer ;
	typedef	LReference /*S3DSceneComposer::Parameter*/	LEntisGLS4_SceneParameter ;
	typedef	LReference /*S3DSceneComposer::Sequencer*/	LEntisGLS4_SceneSequencer ;
	typedef	LReference /*S3DSceneComposer::ParameterProperty*/	LEntisGLS4_SceneProperty ;
	typedef	LReference /*S3DSceneComposer::Controller*/	LEntisGLS4_SceneController ;
	typedef	LReference /*S3DSceneComposer::ItemSerializer*/	LEntisGLS4_SceneItem ;
	typedef	LReference /*S3DSceneComposer::SpaceSerializer*/	LEntisGLS4_SceneSpace ;
	typedef	LReference /*S3DSceneComposer::Composition*/	LEntisGLS4_SceneComposition ;
	typedef	LReference /*S3DSceneComposer::ItemCommonSerializer*/	LEntisGLS4_SceneCommon ;
	typedef	LReference /*S3DSceneComposer::CameraSerializer*/	LEntisGLS4_SceneCamera ;
	typedef	LReference /*S3DSceneComposer::LightSerializer*/	LEntisGLS4_SceneLight ;
	typedef	LReference /*S3DSoundItemSerializer*/		LEntisGLS4_SceneSoundItem ;
	typedef	LReference /*S3DSoundItemSerializer::Instance*/	LEntisGLS4_SceneSoundItem_Instance ;
	typedef	LReference /*S3DDynamicModelSerializer*/	LEntisGLS4_SceneDynamicModel ;
	typedef	LReference /*S3DInstancingItemInterface*/	LEntisGLS4_SceneInstancingItem ;
	typedef	LReference /*S3DItemInstanceRef*/			LEntisGLS4_SceneItemInstanceRef ;
	typedef	LReference /*S3DMultiModelSerializer*/		LEntisGLS4_SceneMultiModel ;
	typedef	LReference /*S3DMeshBufferItemSerializer*/	LEntisGLS4_SceneMeshBuffer ;
	typedef	LReference /*S3DSubCompositionSerializer*/	LEntisGLS4_SceneSubComposition ;
	typedef	LReference /*S3DSceneCustomItem*/			LEntisGLS4_SceneCustomItem ;
	typedef	LReference /*SceneCustomController*/		LEntisGLS4_SceneCustomController ;
	typedef	LReference /*ScenePoseController*/			LEntisGLS4_ScenePoseController ;
	typedef	LReference /*SceneMeshController*/			LEntisGLS4_SceneMeshController ;
	typedef	LReference /*S3DSceneComposer*/				LEntisGLS4_SceneComposer ;
	typedef	LReference /*S3DCompositionManager*/		LEntisGLS4_SceneManager ;
	typedef	LObject										LEntisGLS4_SceneManager_Context ;

	typedef	LReference /*SceneBulleteItemBulletInstance*/	LEntisGLS4_SceneBulleteItem_Bullet ;
	typedef	LReference /*S3DBulletItemSerializer*/		LEntisGLS4_SceneBulleteItem ;

	typedef	SGLPoint							LPoint ;
	typedef	SGLSize								LSize ;
	typedef	SGLImageRect						LImageRect ;
	typedef	SGLPalette							LARGB8 ;
	typedef	S3DColor							LEntisGLS4_ColorMulAdd ;
	typedef	S2DVector							LVector2 ;
	typedef	S2DDVector							LVector2d ;
	typedef	S3DVector							LVector3 ;
	typedef	S3DDVector							LVector3d ;
	typedef	S3DVector4							LVector4 ;
	typedef	S4DDVector							LVector4d ;
	typedef	S3DQuaternion						LQuaternion ;
	typedef	S3DDQuaternion						LQuaterniond ;
	typedef	S3DDMatrix							LMatrix3d ;
	typedef	S4DMatrix							LMatrix4 ;
	typedef	S4DDMatrix							LMatrix4d ;
	typedef	SGLImageInfo						LEntisGLS4_ImageInfo ;
	typedef	SGLImageObject::BuildAtlasParam		LEntisGLS4_Image_BuildAtlasParam ;
	typedef	SGLImageEncoderInterface::Options	LEntisGLS4_Image_EncoderOptions ;
	typedef	S3DRenderDevice::Features			LEntisGLS4_RenderDevice_Features ;
	typedef	S3DLightEntry						LEntisGLS4_LightEntry ;
	typedef	S3DShadowMapInfo					LEntisGLS4_ShadowMapInfo ;
	typedef	SGLSprite::Virtual3DParam			LEntisGLS4_Sprite_Virtual3DParam ;
	typedef	UI::SGLJoyStickState				LEntisGLS4_VirtualInput_JoyStickState ;
	typedef	S3DScene::ProjectionParam			LEntisGLS4_Scene_ProjectionParam ;
	typedef	S3DScene::FogParam					LEntisGLS4_Scene_FogParam ;
	typedef	S3DScene::ParallaxParam				LEntisGLS4_Scene_ParallaxParam ;
	typedef	S3DModelPose::MetaInfo				LEntisGLS4_ModelPose_MetaInfo ;
	typedef	SGLSoundFormat						LEntisGLS4_SoundFormat ;
	typedef	S3DSceneComposer::KeyFrameParam		LEntisGLS4_SceneSequencer_KeyFrameParam ;
	typedef	S3DScene::ShadowMapParam			LEntisGLS4_SceneLight_ShadowMapParam ;


	//////////////////////////////////////////////////////////////////////////
	// 変換関数
	//////////////////////////////////////////////////////////////////////////

	// EntisGLS4.PaintParam -> SGLPaintParam 変換
	void GetLPaintParam
		( SGLPaintParam& param,
			SGLAffine& affine, LPtr<LEntisGLS4_PaintParam> pParam ) ;

	// EntisGLS4.FontStyle -> SGLFontStyle 変換
	void GetLFontStyle
		( SGLFontStyle& fontStyle,
			LString& strFontFace, LPtr<LEntisGLS4_FontStyle> pFontStyle ) ;

	// SGLFontStyle -> EntisGLS4.FontStyle 変換
	void SetLFontStyle
		( LPtr<LEntisGLS4_FontStyle> pFontStyle, const SGLFontStyle& fontStyle ) ;

	// SXMLDocument -> LXMLDocPtr 変換
	LXMLDocPtr GetLXMLDocPtr( const SXMLDocument& xmlDoc ) ;

	// EntisGLS4.TextSprite.TextStyle -> SGLSpriteText::TextStyle 変換
	void GetLTextStyle
		( SGLSpriteText::TextStyle& style,
			LString& strFontFace, LString& strProhibition,
			LPtr<LEntisGLS4_TextSprite_TextStyle> pStyle ) ;

	// SGLSpriteText::TextStyle -> EntisGLS4.TextSprite.TextStyle 変換
	void SetLTextStyle
		( LPtr<LEntisGLS4_TextSprite_TextStyle> pStyle,
					const SGLSpriteText::TextStyle& style ) ;

	// EntisGLS4.EditSprite.ExitStyle -> SGLSpriteEdit::EditStyle 変換
	void GetLEditStyle
		( SGLSpriteEdit::EditStyle& style,
			LString& strFontFace, LString& strProhibition,
			LString& strIMEFontFace,
			LPtr<LEntisGLS4_EditSprite_ExitStyle> pStyle ) ;

	// SGLSpriteEdit::EditStyle -> EntisGLS4.EditSprite.ExitStyle 変換
	void SetLEditStyle
		( LPtr<LEntisGLS4_EditSprite_ExitStyle> pStyle,
					const SGLSpriteEdit::EditStyle& style ) ;

	// EntisGLS4.MessageSprite.RichTextStyle -> SGLSpriteMessage::RichTextStyle 変換
	void GetLRichTextStyle
		( SGLSpriteMessage::RichTextStyle& style,
			LString& strFontFace, LString& strProhibition,
			LString& strRubyFontFace,
			LPtr<LEntisGLS4_MessageSprite_RichTextStyle> pStyle ) ;

	// SGLSpriteMessage::RichTextStyle -> EntisGLS4.MessageSprite.RichTextStyle 変換
	void SetLRichTextStyle
		( LPtr<LEntisGLS4_MessageSprite_RichTextStyle> pStyle,
					const SGLSpriteMessage::RichTextStyle& style ) ;

	// EntisGLS4.MessageSprite.MessageStyle -> SGLSpriteMessage::MessageStyle 変換
	void GetLMessageStyle
		( SGLSpriteMessage::MessageStyle& style,
			LString& strFontFace, LString& strProhibition,
			LString& strRubyFontFace,
			LPtr<LEntisGLS4_MessageSprite_MessageStyle> pStyle ) ;

	// SGLSpriteMessage::MessageStyle -> EntisGLS4.MessageSprite.MessageStyle 変換
	void SetLMessageStyle
		( LPtr<LEntisGLS4_MessageSprite_MessageStyle> pStyle,
					const SGLSpriteMessage::MessageStyle& style ) ;

	// EntisGLS4.VirtualInput.Event -> SGLVirtualInput::InputEvent 変換
	void GetLInputEvent
		( SGLVirtualInput::InputEvent& event,
			LPtr<LEntisGLS4_VirtualInput_Event> pEvent ) ;

	// S3DCollisionResult -> EntisGLS4.Collider.Result 変換
	bool SetLColliderResult
		( LPtr<LEntisGLS4_Collider_Result> pResult, const S3DCollisionResult& rs ) ;

	// EntisGLS4.SceneManager.Context -> S3DSceneComposerPluginSceneInfo 変換
	void GetLSceneManagerContext
		( S3DSceneComposerPluginSceneInfo& sceneInfo,
			LPtr<LEntisGLS4_SceneManager_Context> pContext ) ;

	// S3DSceneComposerPluginSceneInfo -> EntisGLS4.SceneManager.Context 変換
	void SetLSceneManagerContext
		( LPtr<LEntisGLS4_SceneManager_Context> pContext,
			const S3DSceneComposerPluginSceneInfo& sceneInfo ) ;


	//////////////////////////////////////////////////////////////////////////
	// ヘルパー関数
	//////////////////////////////////////////////////////////////////////////

	// Sprite の Loquaty クラスを取得する
	LClass * GetSpriteClass( LVirtualMachine& vm, const SGLSprite* pSprite ) ;

	// SceneParameter の Loquaty クラスを取得する
	LClass * GetSceneItemClass
		( LVirtualMachine& vm, const S3DSceneComposer::Parameter* pParam ) ;


	//////////////////////////////////////////////////////////////////////////
	// EntisGLS4.Affine - SGLAffine 変換
	//////////////////////////////////////////////////////////////////////////

	struct	LAffine
	{
		float32_t	a11, a12, a13 ;
		float32_t	a21, a22, a23 ;

		LAffine( const SGLAffine& aff )
			: a11( aff.a11 ), a12( aff.a12 ), a13( aff.a13 ),
				a21( aff.a21 ), a22( aff.a22 ), a23( aff.a23 ) { }
		const LAffine& operator = ( const SGLAffine& aff )
		{
			a11 = aff.a11 ;
			a12 = aff.a12 ;
			a13 = aff.a13 ;
			a21 = aff.a21 ;
			a22 = aff.a22 ;
			a23 = aff.a23 ;
			return	*this ;
		}
		SGLAffine ToSGLAffine( void ) const
		{
			return	SGLAffine( a11, a12, a13,  a21, a22, a23 ) ;
		}
	} ;


	//////////////////////////////////////////////////////////////////////////
	// EntisGLS4.Matrix3 - S3DMatrix 変換
	//////////////////////////////////////////////////////////////////////////

	struct	LMatrix3	: public SGL3DMatrix<float32_t,3>
	{
		LMatrix3( const S3DMatrix& mat )
			: SGL3DMatrix<float32_t,3>
				( mat.m[0][0], mat.m[0][1], mat.m[0][2],
					mat.m[1][0], mat.m[1][1], mat.m[1][2],
					mat.m[2][0], mat.m[2][1], mat.m[2][2] ) { }
		S3DMatrix ToS3DMatrix( void ) const
		{
			return	S3DMatrix
				( m[0][0], m[0][1], m[0][2],
					m[1][0], m[1][1], m[1][2],
					m[2][0], m[2][1], m[2][2] ) ;
		}
	} ;


	//////////////////////////////////////////////////////////////////////////
	// Function<void()> 実行 SProcedure
	//////////////////////////////////////////////////////////////////////////

	class	LProcedure	: public SProcedure
	{
	private:
		LPtr<LTaskObj>		m_pTask ;
		LPtr<LFunctionObj>	m_pFunc ;	// Function<void()>

	public:
		LProcedure( LPtr<LTaskObj> pTask, LPtr<LFunctionObj> pFunc )
			: m_pTask( pTask ), m_pFunc( pFunc ) { }
		// スレッド関数
		virtual void Run( void )
		{
			m_pTask->SyncCallFunction( m_pFunc.Ptr(), nullptr, 0 ) ;
		}
		// 完了後の処理
		virtual void Finalize( void )
		{
			delete	this ;
		}
	} ;


	//////////////////////////////////////////////////////////////////////////
	// Sprite.Parameter
	//////////////////////////////////////////////////////////////////////////

	struct	LEntisGLS4_Sprite_Parameter
	{
		uint32_t			nFlags ;		// 描画フラグ
		uint32_t			nSpriteFlags ;	// スプライトフラグ
		S3DDVector			vDst ;			// 表示座標
		S2DDVector			vCenter ;		// ソース画像の中心座標
		S2DDVector			vZoom ;			// 拡大率
		double				zAngle ;		// 回転角 [deg]
		double				xyCross ;		// ｘｙ軸交差角度 [deg]
		uint32_t			nTransparency ;	// 透明度 [0,256]
		int32_t				paramFilter ;	// フィルター進行度
		int32_t				paramFilter2 ;
		SGLPalette			rgbColorParam ;	// 色パラメータ

		// Sprite.Parameter -> SGLSprite::Parameter 変換
		void ToSGLSpriteParameter( SGLSprite::Parameter& param ) const ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// SpriteTimer
	//////////////////////////////////////////////////////////////////////////

	class	LSpriteTimer	: public SGLSpriteTimer
	{
	private:
		LPtr<LTaskObj>	m_pTask ;
		LObject *		m_pObj ;
		LClass *		m_pSpriteClass ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( LSpriteTimer, SGLSpriteTimer )
		// 構築関数
		LSpriteTimer( LVirtualMachine& vm, LObjPtr pObj ) ;
		// タイマー処理（true で終了）
		virtual bool OnTimer( SGLSprite& sprite, uint32_t msecPast ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// SpriteMouseListener
	//////////////////////////////////////////////////////////////////////////

	class	LSpriteMouseListener	: public SGLSpriteMouseStateListener
	{
	private:
		LPtr<LTaskObj>	m_pTask ;
		LObject *		m_pObj ;
		LClass *		m_pSpriteClass ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( LSpriteMouseListener, SGLSpriteMouseStateListener )
		// 構築関数
		LSpriteMouseListener( LVirtualMachine& vm, LObjPtr pObj ) ;
		// マウス移動
		virtual bool OnMouseMove
			( SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		virtual void OnMouseLeave( SGLSprite& sprite, int64_t nFlags ) ;
		// ホイール回転
		virtual bool OnMouseWheel
			( SGLSprite& sprite, int32_t zDelta,
				double xPos, double yPos, int64_t nFlags ) ;
		// マウスボタン（前処理）
		virtual bool OnButtonDown
			( SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		virtual bool OnButtonUp
			( SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		virtual bool OnButtonDblClk
			( SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		// マウスボタン（後処理）
		virtual bool AfterButtonDown
			( SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		virtual bool AfterButtonUp
			( SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
		virtual bool AfterButtonDblClk
			( SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;

	private:
		// コールバック
		bool CallbackOnMouse
			( const wchar_t * pwszName,
				SGLSprite& sprite,
				double xPos, double yPos, int64_t nFlags ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// SpriteKeyListener
	//////////////////////////////////////////////////////////////////////////

	class	LSpriteKeyListener	: public SGLSpriteKeyListener
	{
	private:
		LPtr<LTaskObj>	m_pTask ;
		LObject *		m_pObj ;
		LClass *		m_pSpriteClass ;
		LClass *		m_pStringClass ;
		LClass *		m_pInputStartCompositionClass ;
		LClass *		m_pInputCompositionStringClass ;
		LString			m_strFontFace ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( LSpriteKeyListener, SGLSpriteKeyListener )
		// 構築関数
		LSpriteKeyListener( LVirtualMachine& vm, LObjPtr pObj ) ;
		// キー入力
		virtual bool OnKeyDown
			( SGLSprite& sprite, int64_t nVirtKey, int64_t nFlags ) ;
		virtual bool OnKeyUp
			( SGLSprite& sprite, int64_t nVirtKey, int64_t nFlags ) ;
		// 文字入力
		virtual bool OnChar( SGLSprite& sprite, uint16_t codeChar ) ;
		// コンポジション開始
		virtual bool OnStartComposition
			( SGLSprite& sprite, SGLInputStartComposition& iscForm ) ;
		// コンポジション終了
		virtual bool OnEndComposition( SGLSprite& sprite ) ;
		// コンポジション文字列
		virtual bool OnCompositionString
			( SGLSprite& sprite, const SGLInputCompositionString& icsComp ) ;
		// コマンド
		virtual bool OnCommand
			( SGLSprite& sprite, const wchar_t * pszCmd,
				int64_t nParam, int64_t nCode, int nPriority, bool fOverwritable ) ;

	private:
		// コールバック
		bool CallbackOnKey
			( const wchar_t * pwszName,
				SGLSprite& sprite, int64_t nVirtKey, int64_t nFlags ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// TextSprite.Lettering 構造体
	//////////////////////////////////////////////////////////////////////////

	struct	LEntisGLS4_TextSprite_Lettering
	{
		SGLPoint		ptStartWriting ;
		SGLImageRect	rectWritable ;
		uint16_t		typeAlignment ;
		uint16_t		flagVertical ;
		int32_t			pitchChar ;
		int32_t			offsetChar ;
		int32_t			scalePitch ;
		int32_t			offsetInHalf ;
		int32_t			offsetOutHalf ;
		int32_t			pitchTab ;
		int32_t			pitchLine ;
		int32_t			widthIndent ;
		uint32_t		minHyphening ;
		uint32_t		maxProhibition ;

		void ToLetteringContext( SGLLetteringContext& lc ) ;
		void FromLetteringContext( const SGLLetteringContext& lc ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// TextSprite.Decoration 構造体
	//////////////////////////////////////////////////////////////////////////

	struct	LEntisGLS4_TextSprite_Decoration
	{
		uint32_t	nFlags ;		// enum DecorationFlag complex
		SGLPalette	rgbaBody ;		// 文字の色
		uint32_t	widthBorder ;	// 縁取りの幅
		SGLPalette	rgbaBorder ;	// 縁取りの色
		SGLPalette	rgbaShadow ;	// 影の色
		SGLPoint	ptShadow ;		// 影のオフセット
		uint32_t	widthBorder2 ;	// 縁取り２の幅
		SGLPalette	rgbaBorder2 ;	// 縁取り２の色
		uint32_t	nGradationCount ;
		uint32_t	nGradationHeight ;
		SGLPalette	rgbGradation[16] ;

		void ToLettererDecoration( SGLLetterer::Decoration& deco ) ;
		void FromLettererDecoration( const SGLLetterer::Decoration& deco ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// RenderableSprite
	//////////////////////////////////////////////////////////////////////////

	class	LRenderableSprite	: public SGLSprite
	{
	private:
		LPtr<LTaskObj>	m_pTask ;
		LObject *		m_pObj ;
		LClass *		m_pRenderClass ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( LRenderableSprite, SGLSprite )
		// 構築関数
		LRenderableSprite( LVirtualMachine& vm, LObjPtr pObj ) ;

	public:
		// フレーム描画（視点に関係しない）共通処理
		virtual void PrepareDrawFrame( void ) ;
		// フレーム描画完了後処理
		virtual void FinishDrawFrame( void ) ;
		// 子スプライトを描画
		virtual void DrawChildren
			( S3DRenderContextInterface& render,
					Stereo3DView s3dView = s3dMonoview ) const ;

	private:
		// コールバック
		void CallbackOnDrawFrame( const wchar_t * pwszFunc ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 衝突情報
	//////////////////////////////////////////////////////////////////////////

	struct	LEntisGLS4_Collider_HitInfo
	{
		LVector3		vHitLocal ;		// 交点（ローカル座標系）
		LVector3		vHitGlobal ;	// 交点（グローバル座標系）
		LVector3		vNormalLocal ;	// 法線（ローカル座標系）
		LVector3		vNormal ;		// 法線（グローバル座標系）
		LMatrix3		matLocal ;		// ローカル座標からグローバル座標へ変換する行列（全ての積）
		LVector3		vLocalBase ;	// ローカル座標からグローバル座標へ変換する移動（全ての合成）
		float32_t		fpDistance ;	// 交点までの距離
		uint64_t		iInstance ;		// 交差インスタンス指標
		uint64_t		iMesh ;			// 交差メッシュ指標
		uint64_t		iPolygon ;		// 交差ポリゴン指標
		uint64_t		maskClasses ;	// 交差したアイテムのクラス・ビットマスク
		uint64_t		maskColliders ;	// 交差したアイテムのクラス・ビットマスク

		void FromCollisionResult( const S3DCollisionResult& rs ) ;
	} ;



	//////////////////////////////////////////////////////////////////////////
	// モデル・マーカー情報
	//////////////////////////////////////////////////////////////////////////

	struct	LEntisGLS4_ModelMarker_Info
	{
		LInt			m_type ;
		LInt			m_shape ;
		LInt			m_iCollider ;
		LQuaternion		m_qRotation ;
		LVector3		m_vPosition ;
		LVector3		m_vDirection ;
		LVector3		m_vSize ;
		float32_t		m_fpRadius ;
		float32_t		m_fpLength ;

		void FromMarkerInfo( const S3DModelData::MarkerInfo& mi ) ;
	} ;
	

	//////////////////////////////////////////////////////////////////////////
	// EntisGLS4.SceneComposer.getCurrent 用スタック
	//////////////////////////////////////////////////////////////////////////

	class	LSceneComposerCurrent
	{
	private:
		static thread_local S3DSceneComposer *
								t_pCurrent ;
		S3DSceneComposer *		m_pPrev ;

	public:
		LSceneComposerCurrent( S3DSceneComposer * pCurrent ) ;
		~LSceneComposerCurrent( void ) ;

		static S3DSceneComposer * GetCurrent( void )
		{
			return	t_pCurrent ;
		}
	} ;


	//////////////////////////////////////////////////////////////////////////
	// EntisGLS4.SceneManager.getCurrent 用スタック
	//////////////////////////////////////////////////////////////////////////

	class	LSceneManagerCurrent
	{
	private:
		static thread_local S3DCompositionManager *
								t_pCurrent ;
		S3DCompositionManager *	m_pPrev ;

	public:
		LSceneManagerCurrent( S3DCompositionManager * pCurrent ) ;
		~LSceneManagerCurrent( void ) ;

		static S3DCompositionManager * GetCurrent( void )
		{
			return	t_pCurrent ;
		}
	} ;


	//////////////////////////////////////////////////////////////////////////
	// Loquaty クラスの ItemCreator
	//////////////////////////////////////////////////////////////////////////

	class	LSceneItemCreator	: public S3DSceneComposer::ItemCreator
	{
	protected:
		LClass *			m_pClass ;
		LPtr<LFunctionObj>	m_pInitFunc ;
		LPtr<LTaskObj>		m_pTask ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( LSceneItemCreator, ItemCreator )
		// 構築
		LSceneItemCreator( LClass * pClass ) ;
		// アイテム生成
		virtual S3DSceneComposer::Parameter * NewItem( void ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// Loquaty の S3DSceneComposerDynamicPlugin::Descriptor 実装
	//////////////////////////////////////////////////////////////////////////

	class	LScenePluginDescriptor
				: public S3DSceneComposerDynamicPlugin::Descriptor
	{
	protected:
		LPtr<LTaskObj>		m_pTask ;
		LPtr<LFunctionObj>	m_pFuncCreateItem ;
		LPtr<LFunctionObj>	m_pFuncOnUpdateMenu ;

	public:
		// 構築関数
		LScenePluginDescriptor
			( S3DSceneComposerPluginDescriptorType type,
				const wchar_t * pwszMenuPath, const wchar_t * pwszID,
				LPtr<LFunctionObj> pFuncCreateItem,
				LPtr<LFunctionObj> pFuncOnUpdateMenu ) ;
		// メニューからアイテム作成
		virtual ESLObject * CreateItem
				( const S3DSceneComposerPluginDescriptor& desc,
					const S3DSceneComposerEditorEnvironment& env ) ;
		// メニューの表示状態更新
		virtual uint32_t OnUpdateMenu
				( const S3DSceneComposerPluginDescriptor& desc,
					const S3DSceneComposer::ItemSerializer* pParentItem ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// LSourceFilePtr コンテナ
	//////////////////////////////////////////////////////////////////////////

	class	LSourceFilePtrObj	: public SSystem::SObject
	{
	public:
		LSourceFilePtr	m_pSource ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( LSourceFilePtrObj, SObject )
		// 構築
		LSourceFilePtrObj( LSourceFilePtr pSource )
			: m_pSource( pSource ) {}
	} ;


	//////////////////////////////////////////////////////////////////////////
	// S3DSceneComposer 用コンパイラ
	//////////////////////////////////////////////////////////////////////////

	class	LSceneCompiler	: public LCompiler
	{
	protected:
		S3DSceneComposer&	m_composer ;

	public:
		LSceneCompiler
			( S3DSceneComposer& composer,
				LVirtualMachine& vm, LStringParser * src = nullptr ) ;

		// 文字列出力
		virtual void PrintString( const LString& str ) ;
	} ;

}


#endif

