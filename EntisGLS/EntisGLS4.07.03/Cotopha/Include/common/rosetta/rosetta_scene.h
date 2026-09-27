
#if	!defined(__ROSETTA_SCENE_H__)
#define	__ROSETTA_SCENE_H__	1

#include <rosetta/rosetta_reference.h>
#include <sakuraglx/render/sglx3d_scene_composer.h>

namespace	Rosetta
{
	//////////////////////////////////////////////////////////////////////////
	// Scene クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSSceneClass	: public RSClass
	{
	public:
		// Scene.Space クラス
		class	RSSpaceClass	: public RSClass
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( RSSpaceClass, RSClass )
			// 構築関数
			RSSpaceClass
				( RSClass * pClass, const wchar_t * pwszClassName = L"Space" ) ;
			// クラス固有仮想関数オーバーライドと
			// クラス static 変数のオーバーロード
			virtual void OverrideVirtuals( RSContext& context ) ;
			// this オブジェクトを取得
			static SakuraGL::S3DScene::Space *
				GetThisSceneManager( RSContext& context, RSObject* pThis ) ;

		public:
			// const Scene.Space getChildAs( String id )
			static RSObject * method_getChildAs
				( RSContext& context, void * pInstace,
					RSObject* pThis, RSObject** ppArg, size_t count ) ;
		} ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSSceneClass, RSClass )
		// 構築関数
		RSSceneClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"Scene" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// ネイティブ型テスト
		virtual bool IsNativeObjectOf( ESLObject * pObj ) const ;
		// this オブジェクトを取得
		static SakuraGL::S3DScene *
			GetThisScene( RSContext& context, RSObject* pThis ) ;

	public:
		// void <init>()
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const Scene.Space getRootSpace()
		static RSObject * method_getRootSpace
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// SceneManager クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSSceneManagerClass	: public RGenericNativeObjectClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSSceneManagerClass, RGenericNativeObjectClass )
		// 構築関数
		RSSceneManagerClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"SceneManager" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// ネイティブ型テスト
		virtual bool IsNativeObjectOf( ESLObject * pObj ) const ;
		// this オブジェクトを取得
		static SakuraGL::S3DCompositionManager *
			GetThisSceneManager( RSContext& context, RSObject* pThis ) ;

	public:
		// void <init>()
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// SceneComposer クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSSceneComposerClass	: public RGenericNativeObjectClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSSceneComposerClass, RGenericNativeObjectClass )
		// 構築関数
		RSSceneComposerClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"SceneComposer" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// ネイティブ型テスト
		virtual bool IsNativeObjectOf( ESLObject * pObj ) const ;
		// this オブジェクトを取得
		static SakuraGL::S3DSceneComposer *
			GetThisSceneComposer( RSContext& context, RSObject* pThis ) ;

	public:
		// void <init>( SceneManager manager )
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const Image getAssetImageAs( String id )
		static RSObject * method_getAssetImageAs
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const AudioPlayer getAssetAudioAs( String id )
		static RSObject * method_getAssetAudioAs
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const ModelBuffer getAssetModelAs( String id )
		static RSObject * method_getAssetModelAs
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const ModelPose getAssetPoseAs( String id )
		static RSObject * method_getAssetPoseAs
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const Material getAssetMaterialAs( String id )
		static RSObject * method_getAssetMaterialAs
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// SceneParameter クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSSceneParameterClass	: public RGenericNativeObjectClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSSceneParameterClass, RGenericNativeObjectClass )
		// 構築関数
		RSSceneParameterClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"SceneParameter" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// ネイティブ型テスト
		virtual bool IsNativeObjectOf( ESLObject * pObj ) const ;
		// this オブジェクトを取得
		static SakuraGL::S3DSceneComposer::Parameter *
			GetThisSceneParameter( RSContext& context, RSObject* pThis ) ;

	public:
		// const Matrix4D getMatrix4DParameter( int i )
		static RSObject * method_getMatrix4DParameter
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const Matrix3D getMatrixParameter( int i )
		static RSObject * method_getMatrixParameter
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const Vector3D getVectorParameter( int i )
		static RSObject * method_getVectorParameter
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const Vector3D4 getVector4DParameter( int i )
		static RSObject * method_getVector4DParameter
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const Vector2D getVector2DParameter( int i )
		static RSObject * method_getVector2DParameter
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const double getScalarParameter( int i )
		static RSObject * method_getScalarParameter
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const int getIntegerParameter( int i )
		static RSObject * method_getIntegerParameter
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const boolean getBooleanParameter( int i )
		static RSObject * method_getBooleanParameter
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const String getCommandParameter( int i )
		static RSObject * method_getCommandParameter
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const int getBinaryParameter( Uint8Pointer pDst, int nBufBytes, int i )
		static RSObject * method_getBinaryParameter
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setMatrix4DParameter( int i, Matrix4D mat )
		static RSObject * method_setMatrix4DParameter
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setMatrixParameter( int i, Matrix3D mat )
		static RSObject * method_setMatrixParameter
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setVectorParameter( int i, Vector3D vec )
		static RSObject * method_setVectorParameter
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setVector4DParameter( int i, Vector3D4 vec )
		static RSObject * method_setVector4DParameter
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setVector2DParameter( int i, Vector2D vec )
		static RSObject * method_setVector2DParameter
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setScalarParameter( int i, double s )
		static RSObject * method_setScalarParameter
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setIntegerParameter( int i, int n )
		static RSObject * method_setIntegerParameter
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setBooleanParameter( int i, boolean b )
		static RSObject * method_setBooleanParameter
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setCommandParameter( int i, String str )
		static RSObject * method_setCommandParameter
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int setBinaryParameter( int i, Uint8Pointer pSrc, int nBufBytes )
		static RSObject * method_setBinaryParameter
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static int colorFromVector( Vector3D vec )
		static RSObject * method_colorFromVector
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static Vector3D vectorFromColor( int rgb )
		static RSObject * method_vectorFromColor
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// SceneSequencer クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSSceneSequencerClass	: public RSClass
	{
	public:
		class	RSKeyFrameParam	: public RSStructuredPointerClass
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( RSKeyFrameParam, RSStructuredPointerClass )
			// 構築関数
			RSKeyFrameParam
				( RSClass * pClass, const wchar_t * pwszClassName = L"KeyFrameParam" ) ;
			// クラス固有仮想関数オーバーライドと
			// クラス static 変数のオーバーロード
			virtual void OverrideVirtuals( RSContext& context ) ;
		} ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSSceneSequencerClass, RSClass )
		// 構築関数
		RSSceneSequencerClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"SceneSequencer" ) ;
		// メンバ初期設定
		virtual void Initialize( RSContext& context ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// ネイティブ型テスト
		virtual bool IsNativeObjectOf( ESLObject * pObj ) const ;
		// this オブジェクトを取得
		static SakuraGL::S3DSceneComposer::Sequencer *
			GetThisSceneSequencer( RSContext& context, RSObject* pThis ) ;

	public:
		// const boolean getKeyFrameParameter( int i, KeyFrameParam kfp )
		static RSObject * method_getKeyFrameParameter
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean setKeyFrameParameter( int i, KeyFrameParam kfp )
		static RSObject * method_setKeyFrameParameter
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const int getKeyFrameCount()
		static RSObject * method_getKeyFrameCount
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void insertKeyFrame( int i, KeyFrameParam kfp )
		static RSObject * method_insertKeyFrame
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void removeKeyFrame( int i )
		static RSObject * method_removeKeyFrame
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void updateAllFrameValues()
		static RSObject * method_updateAllFrameValues
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int orderKeyFrameIndex( int iFrame )
		static RSObject * method_orderKeyFrameIndex
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int findKeyFrame( int iFrame )
		static RSObject * method_findKeyFrame
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// SceneProperty クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSScenePropertyClass	: public RSClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSScenePropertyClass, RSClass )
		// 構築関数
		RSScenePropertyClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"SceneProperty" ) ;
		// メンバ初期設定
		virtual void Initialize( RSContext& context ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// ネイティブ型テスト
		virtual bool IsNativeObjectOf( ESLObject * pObj ) const ;
		// this オブジェクトを取得
		static SakuraGL::S3DSceneComposer::ParameterProperty *
			GetThisSceneProperty( RSContext& context, RSObject* pThis ) ;

	public:
		// const String getItemIdentity()
		static RSObject * method_getItemIdentity
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setItemIdentity( String id )
		static RSObject * method_setItemIdentity
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const int getParameterCount()
		static RSObject * method_getParameterCount
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const String getParameterID( int i )
		static RSObject * method_getParameterID
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const String getParameterFriendlyName( int i )
		static RSObject * method_getParameterFriendlyName
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const String getParameterDescription( int i )
		static RSObject * method_getParameterDescription
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const int findParameterID( String id )
		static RSObject * method_findParameterID
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const int getParameterType( int i )
		static RSObject * method_getParameterType
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const int getParameterAttributes( int i )
		static RSObject * method_getParameterAttributes
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const boolean isParameterValidation( int i )
		static RSObject * method_isParameterValidation
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const SceneSequencer getParameterSequencer( int i )
		static RSObject * method_getParameterSequencer
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// SceneSequencer createParameterSequencer( int i )
		static RSObject * method_createParameterSequencer
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void removeParameterSequencer( int i )
		static RSObject * method_removeParameterSequencer
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// SceneController クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSSceneControllerClass	: public RSClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSSceneControllerClass, RSClass )
		// 構築関数
		RSSceneControllerClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"SceneController" ) ;
		// メンバ初期設定
		virtual void Initialize( RSContext& context ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// ネイティブ型テスト
		virtual bool IsNativeObjectOf( ESLObject * pObj ) const ;
		// this オブジェクトを取得
		static SakuraGL::S3DSceneComposer::Controller *
			GetThisSceneController( RSContext& context, RSObject* pThis ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// SceneItem クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSSceneItemClass	: public RSClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSSceneItemClass, RSClass )
		// 構築関数
		RSSceneItemClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"SceneItem" ) ;
		// メンバ初期設定
		virtual void Initialize( RSContext& context ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// ネイティブ型テスト
		virtual bool IsNativeObjectOf( ESLObject * pObj ) const ;
		// this オブジェクトを取得
		static SakuraGL::S3DSceneComposer::ItemSerializer *
			GetThisSceneItem( RSContext& context, RSObject* pThis ) ;

	public:
		// const SceneItem getParentSpace()
		static RSObject * method_getParentSpace
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const SceneComposition getComposition()
		static RSObject * method_getComposition
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const SceneComposer getComposer()
		static RSObject * method_getComposer
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const SceneManager getManager()
		static RSObject * method_getManager
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const Scene getScene()
		static RSObject * method_getScene
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const SceneItem getSceneItemAs( String id )
		static RSObject * method_getSceneItemAs
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const int getChildItemCount()
		static RSObject * method_getChildItemCount
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const SceneItem getChildItemAt( int i )
		static RSObject * method_getChildItemAt
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const int getControllerCount()
		static RSObject * method_getControllerCount
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const SceneController getControllerAt( int i )
		static RSObject * method_getControllerAt
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// SceneController createController( int i, String typeId, String ctrlId )
		static RSObject * method_createController
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const SceneController getControllerAs( String id )
		static RSObject * method_getControllerAs
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const void getGlobalTransformation( Matrix3D matGlobal, Vector3D vGlobalPos )
		static RSObject * method_getGlobalTransformation
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const Matrix3D getItemRotation()
		static RSObject * method_getItemRotation
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setItemRotation( Matrix3D matRotation )
		static RSObject * method_setItemRotation
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const Vector3D getItemZoom()
		static RSObject * method_getItemZoom
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setItemZoom( Vector3D vZoom )
		static RSObject * method_setItemZoom
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const Vector3D getItemPosition()
		static RSObject * method_getItemPosition
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setItemPosition( Vector3D vPos )
		static RSObject * method_setItemPosition
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const boolean isItemVisible()
		static RSObject * method_isItemVisible
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setItemVisible( boolean flagVisible )
		static RSObject * method_setItemVisible
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// SceneComposition クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSSceneCompositionClass	: public RSClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSSceneCompositionClass, RSClass )
		// 構築関数
		RSSceneCompositionClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"SceneComposition" ) ;
		// メンバ初期設定
		virtual void Initialize( RSContext& context ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// ネイティブ型テスト
		virtual bool IsNativeObjectOf( ESLObject * pObj ) const ;
		// this オブジェクトを取得
		static SakuraGL::S3DSceneComposer::Composition *
			GetThisSceneComposition( RSContext& context, RSObject* pThis ) ;

	public:
		// void postTimelineFrame( double frame )
		static RSObject * method_postTimelineFrame
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// SceneItem createSpaceChild
		//	( SceneItem space, String typeId, String itemId ) ;
		static RSObject * method_createSpaceChild
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean removeSpaceChild( SceneItem space, SceneItem item ) ;
		static RSObject * method_removeSpaceChild
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void delayRemoveItem( SceneItem space, SceneItem item ) ;
		static RSObject * method_delayRemoveItem
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;

	} ;

}

#endif

