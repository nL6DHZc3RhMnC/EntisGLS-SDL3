
#include <rosetta/rosetta.h>
#include <sakuraglx/sakuraglx.h>
#include <rosetta/rosetta_scene.h>


using namespace	SSystem ;
using namespace	SakuraGL ;
using namespace	Rosetta ;


//////////////////////////////////////////////////////////////////////////////
// Scene.Space クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSSceneClass::RSSpaceClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSSceneClass::RSSpaceClass::RSSpaceClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSSceneClass::RSSpaceClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( nullptr, this ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"getChildAs", L"Scene.Space", L"String id",
				nullptr, &RSSpaceClass::method_getChildAs,
				nullptr, RSFunctionPrototype::flagConstant ) ;
}

// this オブジェクトを取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::S3DScene::Space *
	RSSceneClass::RSSpaceClass::GetThisSceneManager
				( RSContext& context, RSObject* pThis )
{
	S3DScene::Space *	pSpace =
		RSNativeObject::GetNative<S3DScene::Space>( pThis ) ;
	if ( pSpace == nullptr )
	{
		context.ThrowExceptionError( L"this が Scene.Space ではありません" ) ;
	}
	return	pSpace ;
}

// const Scene.Space getChildAs( String id )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneClass::RSSpaceClass::method_getChildAs
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DScene::Space *	pThisSpace = GetThisSceneManager( context, pThis ) ;
	if ( pThisSpace == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DScene::Space *	pChild = pThisSpace->GetChildAs( arg.StringAt(0) ) ;
	if ( pChild == nullptr )
	{
		return	nullptr ;
	}
	return	new RSNativeObject( pChild, context.GetClassAs( L"Scene.Space" ) ) ;
}



//////////////////////////////////////////////////////////////////////////////
// Scene クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSSceneClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSSceneClass::RSSceneClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSSceneClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( nullptr, this ) ;
	//
	RSSpaceClass *	pSpaceClass =
			new RSSpaceClass( context.GetClassClass(), L"Space" ) ;
	context.ReleaseObjectRef
		( CreateMemberAs( context, L"Space", pSpaceClass ) ) ;
	pSpaceClass->Initialize( context ) ;
	pSpaceClass->FinishClass( context ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", nullptr, L"",
				nullptr, &RSSceneClass::method_init, nullptr ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getRootSpace", L"Scene.Space", L"",
				nullptr, &RSSceneClass::method_getRootSpace,
				nullptr, RSFunctionPrototype::flagConstant ) ;
}

// ネイティブ型テスト
//////////////////////////////////////////////////////////////////////////////
bool RSSceneClass::IsNativeObjectOf( ESLObject * pObj ) const
{
	return	(ESLTypeCast<S3DScene>(pObj) != nullptr) ;
}

// this オブジェクトを取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::S3DScene *
	RSSceneClass::GetThisScene( RSContext& context, RSObject* pThis )
{
	S3DScene *	pScene = RSNativeObject::GetNative<S3DScene>( pThis ) ;
	if ( pScene == nullptr )
	{
		context.ThrowExceptionError( L"this が Scene.Space ではありません" ) ;
	}
	return	pScene ;
}

// void <init>()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj == nullptr )
	{
		context.ThrowExceptionError
			( L"Scene.<init> の this が Scene ではありません" ) ;
		return	nullptr ;
	}
	pNativeObj->SetObject( new S3DScene ) ;
	return	nullptr ;
}

// const Scene.Space getRootSpace()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneClass::method_getRootSpace
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DScene *	pThisScene = GetThisScene( context, pThis ) ;
	if ( pThisScene == nullptr )
	{
		return	nullptr ;
	}
	return	new RSNativeObject
		( &(pThisScene->GetRootSpace()), context.GetClassAs( L"Scene.Space" ) ) ;
}



//////////////////////////////////////////////////////////////////////////////
// SceneManager クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSSceneManagerClass, RGenericNativeObjectClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSSceneManagerClass::RSSceneManagerClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RGenericNativeObjectClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSSceneManagerClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( nullptr, this ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", nullptr, L"",
				nullptr, &RSSceneManagerClass::method_init, nullptr ) ;
}

// ネイティブ型テスト
//////////////////////////////////////////////////////////////////////////////
bool RSSceneManagerClass::IsNativeObjectOf( ESLObject * pObj ) const
{
	return	(ESLTypeCast<S3DCompositionManager>(pObj) != nullptr) ;
}

// this オブジェクトを取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::S3DCompositionManager *
	RSSceneManagerClass::GetThisSceneManager( RSContext& context, RSObject* pThis )
{
	S3DCompositionManager *	pManager =
		RSNativeObject::GetNative<S3DCompositionManager>( pThis ) ;
	if ( pManager == nullptr )
	{
		context.ThrowExceptionError( L"this が SceneManager ではありません" ) ;
	}
	return	pManager ;
}

// void <init>()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneManagerClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj == nullptr )
	{
		context.ThrowExceptionError
			( L"SceneManager.<init> の this が SceneManager ではありません" ) ;
		return	nullptr ;
	}
	pNativeObj->SetObject( new S3DCompositionManager ) ;
	return	nullptr ;
}




//////////////////////////////////////////////////////////////////////////////
// SceneComposer クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSSceneComposerClass, RGenericNativeObjectClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSSceneComposerClass::RSSceneComposerClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RGenericNativeObjectClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSSceneComposerClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( nullptr, this ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", nullptr, L"SceneManager manager",
			nullptr, &RSSceneComposerClass::method_init, nullptr ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getAssetImageAs", L"Image", L"String id",
			nullptr, &RSSceneComposerClass::method_getAssetImageAs,
			nullptr, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getAssetAudioAs", L"AudioPlayer", L"String id",
			nullptr, &RSSceneComposerClass::method_getAssetAudioAs,
			nullptr, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getAssetModelAs", L"ModelBuffer", L"String id",
			nullptr, &RSSceneComposerClass::method_getAssetModelAs,
			nullptr, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getAssetPoseAs", L"ModelPose", L"String id",
			nullptr, &RSSceneComposerClass::method_getAssetPoseAs,
			nullptr, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getAssetMaterialAs", L"Material", L"String id",
			nullptr, &RSSceneComposerClass::method_getAssetMaterialAs,
			nullptr, RSFunctionPrototype::flagConstant ) ;
}

// ネイティブ型テスト
//////////////////////////////////////////////////////////////////////////////
bool RSSceneComposerClass::IsNativeObjectOf( ESLObject * pObj ) const
{
	return	(ESLTypeCast<S3DSceneComposer>(pObj) != nullptr) ;
}

// this オブジェクトを取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::S3DSceneComposer *
	RSSceneComposerClass::GetThisSceneComposer( RSContext& context, RSObject* pThis )
{
	S3DSceneComposer *	pComposer =
		RSNativeObject::GetNative<S3DSceneComposer>( pThis ) ;
	if ( pComposer == nullptr )
	{
		context.ThrowExceptionError( L"this が SceneComposer ではありません" ) ;
	}
	return	pComposer ;
}

// void <init>( SceneManager manager )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneComposerClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj == nullptr )
	{
		context.ThrowExceptionError
			( L"SceneComposer.<init> の this が SceneComposer ではありません" ) ;
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DCompositionManager *	pManager =
		ESLTypeCast<S3DCompositionManager>( arg.NativeObjectAt(0) ) ;
	if ( pManager == nullptr )
	{
		context.ThrowExceptionError
			( L"SceneComposer.<init> の引数が SceneManager ではありません" ) ;
		return	nullptr ;
	}
	pNativeObj->SetObject( new S3DSceneComposer( pManager ) ) ;
	return	nullptr ;
}

// const Image getAssetImageAs( String id )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneComposerClass::method_getAssetImageAs
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer *	pThisComposer = GetThisSceneComposer( context, pThis ) ;
	if ( pThisComposer == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLImageObject *	pImage =
		pThisComposer->GetAssets().GetImageAs( arg.StringAt(0) ) ;
	if ( pImage == nullptr )
	{
		return	nullptr ;
	}
	return	new RSNativeObject( pImage, context.GetClassAs( L"Image" ) ) ;
}

// const AudioPlayer getAssetAudioAs( String id )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneComposerClass::method_getAssetAudioAs
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer *	pThisComposer = GetThisSceneComposer( context, pThis ) ;
	if ( pThisComposer == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLAudioPlayer *	pAudio =
		pThisComposer->GetAssets().GetAudioAs( arg.StringAt(0) ) ;
	if ( pAudio == nullptr )
	{
		return	nullptr ;
	}
	return	new RSNativeObject( pAudio, context.GetClassAs( L"AudioPlayer" ) ) ;
}

// const ModelBuffer getAssetModelAs( String id )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneComposerClass::method_getAssetModelAs
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer *	pThisComposer = GetThisSceneComposer( context, pThis ) ;
	if ( pThisComposer == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DModelBuffer *	pModel =
		pThisComposer->GetAssets().GetModelAs( arg.StringAt(0) ) ;
	if ( pModel == nullptr )
	{
		return	nullptr ;
	}
	return	new RSNativeObject
	( (S3DRenderBufferInterface*) pModel, context.GetClassAs( L"ModelBuffer" ) ) ;
}

// const ModelPose getAssetPoseAs( String id )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneComposerClass::method_getAssetPoseAs
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer *	pThisComposer = GetThisSceneComposer( context, pThis ) ;
	if ( pThisComposer == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DModelPose *	pPose =
		pThisComposer->GetAssets().GetPoseLibrary().GetPoseAs( arg.StringAt(0) ) ;
	if ( pPose == nullptr )
	{
		return	nullptr ;
	}
	return	new RSNativeObject( pPose, context.GetClassAs( L"ModelPose" ) ) ;
}

// const Material getAssetMaterialAs( String id )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneComposerClass::method_getAssetMaterialAs
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer *	pThisComposer = GetThisSceneComposer( context, pThis ) ;
	if ( pThisComposer == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DMaterial *	pMaterial =
		pThisComposer->GetAssets().GetMaterialLibrary().GetMaterialAs( arg.StringAt(0) ) ;
	if ( pMaterial == nullptr )
	{
		return	nullptr ;
	}
	return	new RSNativeObject( pMaterial, context.GetClassAs( L"Material" ) ) ;
}



//////////////////////////////////////////////////////////////////////////////
// SceneParameter クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSSceneParameterClass, RGenericNativeObjectClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSSceneParameterClass::RSSceneParameterClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RGenericNativeObjectClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSSceneParameterClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( nullptr, this ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"getMatrix4DParameter", L"Matrix4D", L"int i",
			nullptr, &RSSceneParameterClass::method_getMatrix4DParameter,
			nullptr, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getMatrixParameter", L"Matrix3D", L"int i",
			nullptr, &RSSceneParameterClass::method_getMatrixParameter,
			nullptr, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getVectorParameter", L"Vector3D", L"int i",
			nullptr, &RSSceneParameterClass::method_getVectorParameter,
			nullptr, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getVector4DParameter", L"Vector3D4", L"int i",
			nullptr, &RSSceneParameterClass::method_getVector4DParameter,
			nullptr, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getVector2DParameter", L"Vector2D", L"int i",
			nullptr, &RSSceneParameterClass::method_getVector2DParameter,
			nullptr, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getScalarParameter", L"double", L"int i",
			nullptr, &RSSceneParameterClass::method_getScalarParameter,
			nullptr, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getIntegerParameter", L"int", L"int i",
			nullptr, &RSSceneParameterClass::method_getIntegerParameter,
			nullptr, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getBooleanParameter", L"boolean", L"int i",
			nullptr, &RSSceneParameterClass::method_getBooleanParameter,
			nullptr, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getCommandParameter", L"String", L"int i",
			nullptr, &RSSceneParameterClass::method_getCommandParameter,
			nullptr, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getBinaryParameter",
			L"int", L"Uint8Pointer pDst, int nBufBytes, int i",
			nullptr, &RSSceneParameterClass::method_getBinaryParameter,
			nullptr, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setMatrix4DParameter", nullptr, L"int i, Matrix4D mat",
			nullptr, &RSSceneParameterClass::method_setMatrix4DParameter, nullptr ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setMatrixParameter", nullptr, L"int i, Matrix3D mat",
			nullptr, &RSSceneParameterClass::method_setMatrixParameter, nullptr ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setVectorParameter", nullptr, L"int i, Vector3D vec",
			nullptr, &RSSceneParameterClass::method_setVectorParameter, nullptr ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setVector4DParameter", nullptr, L"int i, Vector3D4 vec",
			nullptr, &RSSceneParameterClass::method_setVector4DParameter, nullptr ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setVector2DParameter", nullptr, L"int i, Vector2D vec",
			nullptr, &RSSceneParameterClass::method_setVector2DParameter, nullptr ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setScalarParameter", nullptr, L"int i, double s",
			nullptr, &RSSceneParameterClass::method_setScalarParameter, nullptr ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setIntegerParameter", nullptr, L"int i, int n",
			nullptr, &RSSceneParameterClass::method_setIntegerParameter, nullptr ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setBooleanParameter", nullptr, L"int i, boolean b",
			nullptr, &RSSceneParameterClass::method_setBooleanParameter, nullptr ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setCommandParameter", nullptr, L"int i, String str",
			nullptr, &RSSceneParameterClass::method_setCommandParameter, nullptr ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setBinaryParameter",
			nullptr, L"int i, Uint8Pointer pSrc, int nBufBytes",
			nullptr, &RSSceneParameterClass::method_setBinaryParameter, nullptr ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"colorFromVector", L"int", L"Vector3D vec",
			nullptr, &RSSceneParameterClass::method_colorFromVector, nullptr ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"vectorFromColor", L"Vector3D", L"int rgb",
			nullptr, &RSSceneParameterClass::method_vectorFromColor, nullptr ) ;
}

// ネイティブ型テスト
//////////////////////////////////////////////////////////////////////////////
bool RSSceneParameterClass::IsNativeObjectOf( ESLObject * pObj ) const
{
	return	(ESLTypeCast<S3DSceneComposer::Parameter>(pObj) != nullptr) ;
}

// this オブジェクトを取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::S3DSceneComposer::Parameter *
	RSSceneParameterClass::GetThisSceneParameter( RSContext& context, RSObject* pThis )
{
	S3DSceneComposer::Parameter *	pParameter =
		RSNativeObject::GetNative<S3DSceneComposer::Parameter>( pThis ) ;
	if ( pParameter == nullptr )
	{
		context.ThrowExceptionError( L"this が SceneParameter ではありません" ) ;
	}
	return	pParameter ;
}

// const Matrix4D getMatrix4DParameter( int i )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneParameterClass::method_getMatrix4DParameter
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::Parameter *
			pThisParam = GetThisSceneParameter( context, pThis ) ;
	if ( pThisParam == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S4DDMatrix			mat4( 1, 1, 1, 1 ) ;
	pThisParam->GetBinaryParameter( &mat4, sizeof(mat4), arg.IntAt(0) ) ;
	//
	RSStructuredPointer *	pObj =
		ESLSmartCast<RSStructuredPointer>
			( context.new_StructuredPointer( L"Matrix4D" ) ) ;
	ESLAssert( pObj != nullptr ) ;
	if ( pObj != nullptr )
	{
		*(pObj->GetPtr<S4DMatrix>()) = mat4 ;
	}
	return	pObj ;
}

// const Matrix3D getMatrixParameter( int i )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneParameterClass::method_getMatrixParameter
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::Parameter *
			pThisParam = GetThisSceneParameter( context, pThis ) ;
	if ( pThisParam == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DDMatrix	mat3 = pThisParam->GetMatrixParameter( arg.IntAt(0) ) ;
	//
	RSStructuredPointer *	pObj =
		ESLSmartCast<RSStructuredPointer>
			( context.new_StructuredPointer( L"Matrix3D" ) ) ;
	ESLAssert( pObj != nullptr ) ;
	if ( pObj != nullptr )
	{
		SGL3DMatrix<float32_t,3> *	pMat3 =
				pObj->GetPtr< SGL3DMatrix<float32_t,3> >() ;
		for ( int i = 0; i < 3; i ++ )
		{
			pMat3->m[i][0] = (float32_t) mat3.m[i][0] ;
			pMat3->m[i][1] = (float32_t) mat3.m[i][1] ;
			pMat3->m[i][2] = (float32_t) mat3.m[i][2] ;
		}
	}
	return	pObj ;
}

// const Vector3D getVectorParameter( int i )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneParameterClass::method_getVectorParameter
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::Parameter *
			pThisParam = GetThisSceneParameter( context, pThis ) ;
	if ( pThisParam == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DDVector	vec3 = pThisParam->GetVectorParameter( arg.IntAt(0) ) ;
	//
	RSStructuredPointer *	pObj =
		ESLSmartCast<RSStructuredPointer>
			( context.new_StructuredPointer( L"Vector3D" ) ) ;
	ESLAssert( pObj != nullptr ) ;
	if ( pObj != nullptr )
	{
		*(pObj->GetPtr<S3DVector>()) = vec3 ;
	}
	return	pObj ;
}

// const Vector3D4 getVector4DParameter( int i )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneParameterClass::method_getVector4DParameter
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::Parameter *
			pThisParam = GetThisSceneParameter( context, pThis ) ;
	if ( pThisParam == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S4DDVector			vec4( 0, 0, 0, 0 ) ;
	pThisParam->GetBinaryParameter( &vec4, sizeof(vec4), arg.IntAt(0) ) ;
	//
	RSStructuredPointer *	pObj =
		ESLSmartCast<RSStructuredPointer>
			( context.new_StructuredPointer( L"Vector3D4" ) ) ;
	ESLAssert( pObj != nullptr ) ;
	if ( pObj != nullptr )
	{
		*(pObj->GetPtr<S4DVector>()) = vec4 ;
	}
	return	pObj ;
}

// const Vector2D getVector2DParameter( int i )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneParameterClass::method_getVector2DParameter
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::Parameter *
			pThisParam = GetThisSceneParameter( context, pThis ) ;
	if ( pThisParam == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S2DDVector			vec2( 0, 0 ) ;
	pThisParam->GetBinaryParameter( &vec2, sizeof(vec2), arg.IntAt(0) ) ;
	//
	RSStructuredPointer *	pObj =
		ESLSmartCast<RSStructuredPointer>
			( context.new_StructuredPointer( L"Vector2D" ) ) ;
	ESLAssert( pObj != nullptr ) ;
	if ( pObj != nullptr )
	{
		*(pObj->GetPtr<S2DVector>()) = vec2 ;
	}
	return	pObj ;
}

// const double getScalarParameter( int i )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneParameterClass::method_getScalarParameter
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::Parameter *
			pThisParam = GetThisSceneParameter( context, pThis ) ;
	if ( pThisParam == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	double	s = pThisParam->GetScalarParameter( arg.IntAt(0) ) ;
	//
	return	context.new_Number( s ) ;
}

// const int getIntegerParameter( int i )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneParameterClass::method_getIntegerParameter
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::Parameter *
			pThisParam = GetThisSceneParameter( context, pThis ) ;
	if ( pThisParam == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	int32_t	n = pThisParam->GetIntegerParameter( arg.IntAt(0) ) ;
	//
	return	context.new_Integer( n ) ;
}

// const boolean getBooleanParameter( int i )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneParameterClass::method_getBooleanParameter
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::Parameter *
			pThisParam = GetThisSceneParameter( context, pThis ) ;
	if ( pThisParam == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	bool	b = pThisParam->GetBooleanParameter( arg.IntAt(0) ) ;
	//
	return	context.new_Boolean( b ) ;
}

// const String getCommandParameter( int i )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneParameterClass::method_getCommandParameter
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::Parameter *
			pThisParam = GetThisSceneParameter( context, pThis ) ;
	if ( pThisParam == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	const wchar_t *	pwszCmd = pThisParam->GetCommandParameter( arg.IntAt(0) ) ;
	//
	return	context.new_String( pwszCmd ) ;
}

// const int getBinaryParameter( Uint8Pointer pDst, int nBufBytes, int i )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneParameterClass::method_getBinaryParameter
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::Parameter *
			pThisParam = GetThisSceneParameter( context, pThis ) ;
	if ( pThisParam == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	size_t	nBufBytes = (size_t) arg.IntAt(1) ;
	void *	pDst = arg.PointerAt( 0, nBufBytes ) ;
	size_t	nResultBytes =
		pThisParam->GetBinaryParameter( pDst, nBufBytes, arg.IntAt(2) ) ;
	//
	return	context.new_Integer( nResultBytes ) ;
}

// void setMatrix4DParameter( int i, Matrix4D mat )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneParameterClass::method_setMatrix4DParameter
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::Parameter *
			pThisParam = GetThisSceneParameter( context, pThis ) ;
	if ( pThisParam == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S4DMatrix *	pMat4 = (S4DMatrix*) arg.PointerAt( 1, sizeof(S4DMatrix) ) ;
	if ( pMat4 != nullptr )
	{
		S4DDMatrix	mat4 = *pMat4 ;
		pThisParam->SetBinaryParameter( arg.IntAt(0), &mat4, sizeof(mat4) ) ;
	}
	return	nullptr ;
}

// void setMatrixParameter( int i, Matrix3D mat )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneParameterClass::method_setMatrixParameter
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::Parameter *
			pThisParam = GetThisSceneParameter( context, pThis ) ;
	if ( pThisParam == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGL3DMatrix<float32_t,3> *	pMat3 =
		(SGL3DMatrix<float32_t,3>*)
			arg.PointerAt( 1, sizeof(SGL3DMatrix<float32_t,3>) ) ;
	if ( pMat3 != nullptr )
	{
		S3DDMatrix	mat3( pMat3->m[0][0], pMat3->m[0][1], pMat3->m[0][2],
							pMat3->m[1][0], pMat3->m[1][1], pMat3->m[1][2],
							pMat3->m[2][0], pMat3->m[2][1], pMat3->m[2][2] ) ;
		pThisParam->SetMatrixParameter( arg.IntAt(0), mat3 ) ;
	}
	return	nullptr ;
}

// void setVectorParameter( int i, Vector3D vec )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneParameterClass::method_setVectorParameter
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::Parameter *
			pThisParam = GetThisSceneParameter( context, pThis ) ;
	if ( pThisParam == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DVector *	pVec3 = (S3DVector*) arg.PointerAt( 1, sizeof(S3DVector) ) ;
	if ( pVec3 != nullptr )
	{
		S3DDVector	vec3 = *pVec3 ;
		pThisParam->SetVectorParameter( arg.IntAt(0), vec3 ) ;
	}
	return	nullptr ;
}

// void setVector4DParameter( int i, Vector3D4 vec )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneParameterClass::method_setVector4DParameter
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::Parameter *
			pThisParam = GetThisSceneParameter( context, pThis ) ;
	if ( pThisParam == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S4DVector *	pVec4 = (S4DVector*) arg.PointerAt( 1, sizeof(S4DVector) ) ;
	if ( pVec4 != nullptr )
	{
		S4DDVector	vec4 = *pVec4 ;
		pThisParam->SetBinaryParameter( arg.IntAt(0), &vec4, sizeof(vec4) ) ;
	}
	return	nullptr ;
}

// void setVector2DParameter( int i, Vector2D vec )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneParameterClass::method_setVector2DParameter
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::Parameter *
			pThisParam = GetThisSceneParameter( context, pThis ) ;
	if ( pThisParam == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S2DVector *	pVec2 = (S2DVector*) arg.PointerAt( 1, sizeof(S2DVector) ) ;
	if ( pVec2 != nullptr )
	{
		S2DDVector	vec2 = *pVec2 ;
		pThisParam->SetBinaryParameter( arg.IntAt(0), &vec2, sizeof(vec2) ) ;
	}
	return	nullptr ;
}

// void setScalarParameter( int i, double s )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneParameterClass::method_setScalarParameter
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::Parameter *
			pThisParam = GetThisSceneParameter( context, pThis ) ;
	if ( pThisParam == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pThisParam->SetScalarParameter( arg.IntAt(0), arg.DoubleAt(1) ) ;
	return	nullptr ;
}

// void setIntegerParameter( int i, int n )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneParameterClass::method_setIntegerParameter
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::Parameter *
			pThisParam = GetThisSceneParameter( context, pThis ) ;
	if ( pThisParam == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pThisParam->SetIntegerParameter( arg.IntAt(0), (int32_t) arg.IntAt(1) ) ;
	return	nullptr ;
}

// void setBooleanParameter( int i, boolean b )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneParameterClass::method_setBooleanParameter
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::Parameter *
			pThisParam = GetThisSceneParameter( context, pThis ) ;
	if ( pThisParam == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pThisParam->SetBooleanParameter( arg.IntAt(0), arg.BooleanAt(1) ) ;
	return	nullptr ;
}

// void setCommandParameter( int i, String str )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneParameterClass::method_setCommandParameter
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::Parameter *
			pThisParam = GetThisSceneParameter( context, pThis ) ;
	if ( pThisParam == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pThisParam->SetCommandParameter( arg.IntAt(0), arg.StringAt(1) ) ;
	return	nullptr ;
}

// int setBinaryParameter( int i, Uint8Pointer pSrc, int nBufBytes )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneParameterClass::method_setBinaryParameter
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::Parameter *
			pThisParam = GetThisSceneParameter( context, pThis ) ;
	if ( pThisParam == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	size_t	nBufBytes = (size_t) arg.IntAt(2) ;
	void *	pSrc = arg.PointerAt( 1, nBufBytes ) ;
	size_t	nResultBytes =
		pThisParam->SetBinaryParameter( arg.IntAt(0), pSrc, nBufBytes ) ;
	//
	return	context.new_Integer( nResultBytes ) ;
}

// static int colorFromVector( Vector3D vec )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneParameterClass::method_colorFromVector
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DVector *	pVec3 = (S3DVector*) arg.PointerAt( 0, sizeof(S3DVector) ) ;
	if ( pVec3 != nullptr )
	{
		S3DDVector	vec3 = *pVec3 ;
		SGLPalette	rgb = S3DSceneComposer::Parameter::ColorFromVector( vec3 ) ;
		return	context.new_Integer( rgb.ui32 ) ;
	}
	else
	{
		return	context.new_Integer(0) ;
	}
}

// static Vector3D vectorFromColor( int rgb )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneParameterClass::method_vectorFromColor
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLPalette	rgb( (uint32_t) arg.IntAt(0) ) ;
	S3DDVector	vec3 = S3DSceneComposer::Parameter::VectorFromColor( rgb ) ;
	//
	RSStructuredPointer *	pObj =
		ESLSmartCast<RSStructuredPointer>
			( context.new_StructuredPointer( L"Vector3D" ) ) ;
	ESLAssert( pObj != nullptr ) ;
	if ( pObj != nullptr )
	{
		*(pObj->GetPtr<S3DVector>()) = vec3 ;
	}
	return	pObj ;
}



//////////////////////////////////////////////////////////////////////////////
// SceneSequencer.KeyFrameParam クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( Rosetta::RSSceneSequencerClass::RSKeyFrameParam, RSStructuredPointerClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSSceneSequencerClass::RSKeyFrameParam::RSKeyFrameParam
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSStructuredPointerClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSSceneSequencerClass::RSKeyFrameParam::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSStructuredPointer( this, NULL ) ;
	//
	AddArrayMemberAs
		( context, L"iFrame",
			context.GetBasicTypeClass(RSCodeControl::wiInt) ) ;
	AddArrayMemberAs
		( context, L"nFlags",
			context.GetBasicTypeClass(RSCodeControl::wiInt) ) ;
	AddArrayMemberAs
		( context, L"speedIn",
			context.GetBasicTypeClass(RSCodeControl::wiFloat) ) ;
	AddArrayMemberAs
		( context, L"speedOut",
			context.GetBasicTypeClass(RSCodeControl::wiFloat) ) ;
}



//////////////////////////////////////////////////////////////////////////////
// SceneSequencer クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSSceneSequencerClass, RSClass )
// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSSceneSequencerClass::RSSceneSequencerClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// メンバ初期設定
//////////////////////////////////////////////////////////////////////////////
void RSSceneSequencerClass::Initialize( RSContext& context )
{
	AddSuperClass( context, context.GetClassAs( L"SceneParameter" ) ) ;
	OverrideVirtuals( context ) ;
	m_flagInitialized = true ;
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSSceneSequencerClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( nullptr, this ) ;
	//
	RSKeyFrameParam *	pKeyFrameParamClass =
			new RSKeyFrameParam( context.GetClassClass(), L"KeyFrameParam" ) ;
	context.ReleaseObjectRef
		( CreateMemberAs( context, L"KeyFrameParam", pKeyFrameParamClass ) ) ;
	pKeyFrameParamClass->Initialize( context ) ;
	pKeyFrameParamClass->FinishClass( context ) ;
	//
	CreateMemberIntegerAs
		( context, L"keyframeCorner",
			S3DSceneComposer::keyframeCorner, modifierConst ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"getKeyFrameParameter", L"boolean",
			L"int i, SceneSequencer.KeyFrameParam kfp",
			nullptr, &RSSceneSequencerClass::method_getKeyFrameParameter,
			nullptr, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setKeyFrameParameter", L"boolean",
			L"int i, SceneSequencer.KeyFrameParam kfp",
			nullptr, &RSSceneSequencerClass::method_setKeyFrameParameter,
			nullptr, 0 ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getKeyFrameCount", L"int", L"",
			nullptr, &RSSceneSequencerClass::method_getKeyFrameCount,
			nullptr, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"insertKeyFrame", nullptr,
			L"int i, SceneSequencer.KeyFrameParam kfp",
			nullptr, &RSSceneSequencerClass::method_insertKeyFrame,
			nullptr, 0 ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"removeKeyFrame", nullptr, L"int i",
			nullptr, &RSSceneSequencerClass::method_removeKeyFrame,
			nullptr, 0 ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"updateAllFrameValues", nullptr, L"",
			nullptr, &RSSceneSequencerClass::method_updateAllFrameValues,
			nullptr, 0 ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"orderKeyFrameIndex", L"int", L"int iFrame",
			nullptr, &RSSceneSequencerClass::method_orderKeyFrameIndex,
			nullptr, 0 ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"findKeyFrame", L"int", L"int iFrame",
			nullptr, &RSSceneSequencerClass::method_findKeyFrame,
			nullptr, 0 ) ;
}

// ネイティブ型テスト
//////////////////////////////////////////////////////////////////////////////
bool RSSceneSequencerClass::IsNativeObjectOf( ESLObject * pObj ) const
{
	return	(ESLTypeCast<S3DSceneComposer::Sequencer>(pObj) != nullptr) ;
}

// this オブジェクトを取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::S3DSceneComposer::Sequencer *
	RSSceneSequencerClass::GetThisSceneSequencer( RSContext& context, RSObject* pThis )
{
	S3DSceneComposer::Sequencer *	pSeq =
		RSNativeObject::GetNative<S3DSceneComposer::Sequencer>( pThis ) ;
	if ( pSeq == nullptr )
	{
		context.ThrowExceptionError( L"this が SceneSequencer ではありません" ) ;
	}
	return	pSeq ;
}

// const boolean getKeyFrameParameter( int i, KeyFrameParam kfp )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneSequencerClass::method_getKeyFrameParameter
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::Sequencer *
			pThisSeq = GetThisSceneSequencer( context, pThis ) ;
	if ( pThisSeq == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DSceneComposer::KeyFrameParam *	pkfp =
		(S3DSceneComposer::KeyFrameParam*)
			arg.PointerAt( 1, sizeof(S3DSceneComposer::KeyFrameParam) ) ;
	if ( pkfp == nullptr )
	{
		context.ThrowExceptionError
			( L"SceneSequencer.getKeyFrameParameter 関数の引数が不正です" ) ;
		return	nullptr ;
	}
	return	context.new_Boolean
		( pThisSeq->GetKeyFrameParameter( (size_t) arg.IntAt(0), *pkfp ) ) ;
}

// boolean setKeyFrameParameter( int i, KeyFrameParam kfp )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneSequencerClass::method_setKeyFrameParameter
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::Sequencer *
			pThisSeq = GetThisSceneSequencer( context, pThis ) ;
	if ( pThisSeq == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DSceneComposer::KeyFrameParam *	pkfp =
		(S3DSceneComposer::KeyFrameParam*)
			arg.PointerAt( 1, sizeof(S3DSceneComposer::KeyFrameParam) ) ;
	if ( pkfp == nullptr )
	{
		context.ThrowExceptionError
			( L"SceneSequencer.setKeyFrameParameter 関数の引数が不正です" ) ;
		return	nullptr ;
	}
	return	context.new_Boolean
		( pThisSeq->SetKeyFrameParameter( (size_t) arg.IntAt(0), *pkfp ) ) ;
}

// const int getKeyFrameCount()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneSequencerClass::method_getKeyFrameCount
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::Sequencer *
			pThisSeq = GetThisSceneSequencer( context, pThis ) ;
	if ( pThisSeq == nullptr )
	{
		return	nullptr ;
	}
	return	context.new_Integer( pThisSeq->GetKeyFrameCount() ) ;
}

// void insertKeyFrame( int i, KeyFrameParam kfp )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneSequencerClass::method_insertKeyFrame
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::Sequencer *
			pThisSeq = GetThisSceneSequencer( context, pThis ) ;
	if ( pThisSeq == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DSceneComposer::KeyFrameParam *	pkfp =
		(S3DSceneComposer::KeyFrameParam*)
			arg.PointerAt( 1, sizeof(S3DSceneComposer::KeyFrameParam) ) ;
	if ( pkfp == nullptr )
	{
		context.ThrowExceptionError
			( L"SceneSequencer.insertKeyFrame 関数の引数が不正です" ) ;
		return	nullptr ;
	}
	pThisSeq->InsertKeyFrame( (size_t) arg.IntAt(0), *pkfp ) ;
	return	nullptr ;
}

// void removeKeyFrame( int i )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneSequencerClass::method_removeKeyFrame
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::Sequencer *
			pThisSeq = GetThisSceneSequencer( context, pThis ) ;
	if ( pThisSeq == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pThisSeq->RemoveKeyFrame( (size_t) arg.IntAt(0) ) ;
	return	nullptr ;
}

// void updateAllFrameValues()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneSequencerClass::method_updateAllFrameValues
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::Sequencer *
			pThisSeq = GetThisSceneSequencer( context, pThis ) ;
	if ( pThisSeq == nullptr )
	{
		return	nullptr ;
	}
	pThisSeq->UpdateAllFrameValues() ;
	return	nullptr ;
}

// int orderKeyFrameIndex( int iFrame )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneSequencerClass::method_orderKeyFrameIndex
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::Sequencer *
			pThisSeq = GetThisSceneSequencer( context, pThis ) ;
	if ( pThisSeq == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Integer
		( pThisSeq->OrderKeyFrameIndex( (size_t) arg.IntAt(0) ) ) ;
}

// int findKeyFrame( int iFrame )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneSequencerClass::method_findKeyFrame
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::Sequencer *
			pThisSeq = GetThisSceneSequencer( context, pThis ) ;
	if ( pThisSeq == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Integer
		( pThisSeq->FindKeyFrame( (size_t) arg.IntAt(0) ) ) ;
}



//////////////////////////////////////////////////////////////////////////////
// SceneProperty クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSScenePropertyClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSScenePropertyClass::RSScenePropertyClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// メンバ初期設定
//////////////////////////////////////////////////////////////////////////////
void RSScenePropertyClass::Initialize( RSContext& context )
{
	AddSuperClass( context, context.GetClassAs( L"SceneParameter" ) ) ;
	OverrideVirtuals( context ) ;
	m_flagInitialized = true ;
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSScenePropertyClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( nullptr, this ) ;
	//
	struct	ConstIntValue
	{
		int64_t			nValue ;
		const wchar_t *	pwszName ;
	} ;
	#define	SceneConstInt(x)	{ S3DSceneComposer::x, L###x }
	static const ConstIntValue	s_civDefs[] =
	{
		// ParameterType
		SceneConstInt(typeInvalid),
		SceneConstInt(typeMatrix),
		SceneConstInt(typePosition),
		SceneConstInt(typeDirection),
		SceneConstInt(typeZoom),
		SceneConstInt(typeColor),
		SceneConstInt(typeScalar),
		SceneConstInt(typeInteger),
		SceneConstInt(typeBoolean),
		SceneConstInt(typeSelector),
		SceneConstInt(typeCommand),
		SceneConstInt(typePose),
		SceneConstInt(typeBinary),
		SceneConstInt(typeMatrix4),
		SceneConstInt(typeVector4),
		SceneConstInt(typeVector2),
		// ParameterAttribute
		SceneConstInt(attrConstant),
		SceneConstInt(attrNoLocalTransform),
		SceneConstInt(attrStringEnumeration),
		SceneConstInt(attrUIOnlyEnumeration),
		SceneConstInt(attrDynamicValidation),
		SceneConstInt(attrFlagSetInteger),
		SceneConstInt(attrUIScalarSlider),
		SceneConstInt(attrEditUpdateFrame),
		SceneConstInt(attrEditorCommand),
		SceneConstInt(attrGlobalTransform),
		SceneConstInt(attrReadOnlyParam),
		SceneConstInt(attrNoSerializeFlags),
		SceneConstInt(attrCategory1),
		SceneConstInt(attrCategory2),
		SceneConstInt(attrCategory3),
		SceneConstInt(attrCategory4),
		SceneConstInt(attrCategory5),
		SceneConstInt(attrCategory6),
		SceneConstInt(attrCategory7),
		SceneConstInt(attrCategoryMask),
		SceneConstInt(attrCategoryShift),
		SceneConstInt(attrConstant1),
		SceneConstInt(attrConstant2),
		SceneConstInt(attrConstant3),
		SceneConstInt(attrConstant4),
		SceneConstInt(attrConstant5),
		SceneConstInt(attrConstant6),
		SceneConstInt(attrConstant7),
		SceneConstInt(attrAppExtension1),
		SceneConstInt(attrAppExtension2),
		SceneConstInt(attrAppExtension3),
		SceneConstInt(attrAppExtension4),
		SceneConstInt(attrAppExtension5),
		SceneConstInt(attrAppExtension6),
		SceneConstInt(attrAppExtension7),
		SceneConstInt(attrAppExtension8),
	} ;
	#undef	SceneConstInt
	const size_t	nDefsCount = sizeof(s_civDefs)/sizeof(ConstIntValue) ;
	for ( size_t i = 0; i < nDefsCount; i ++ )
	{
		CreateMemberIntegerAs
			( context, s_civDefs[i].pwszName,
				s_civDefs[i].nValue, modifierConst ) ;
	}
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"getItemIdentity", L"String", L"",
			nullptr, &RSScenePropertyClass::method_getItemIdentity,
			nullptr, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setItemIdentity", nullptr, L"String id",
			nullptr, &RSScenePropertyClass::method_setItemIdentity, nullptr ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getParameterCount", L"int", L"",
			nullptr, &RSScenePropertyClass::method_getParameterCount,
			nullptr, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getParameterID", L"String", L"int i",
			nullptr, &RSScenePropertyClass::method_getParameterID,
			nullptr, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getParameterFriendlyName", L"String", L"int i",
			nullptr, &RSScenePropertyClass::method_getParameterFriendlyName,
			nullptr, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getParameterDescription", L"String", L"int i",
			nullptr, &RSScenePropertyClass::method_getParameterDescription,
			nullptr, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"findParameterID", L"int", L"String id",
			nullptr, &RSScenePropertyClass::method_findParameterID,
			nullptr, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getParameterType", L"int", L"int i",
			nullptr, &RSScenePropertyClass::method_getParameterType,
			nullptr, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getParameterAttributes", L"int", L"int i",
			nullptr, &RSScenePropertyClass::method_getParameterAttributes,
			nullptr, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"isParameterValidation", L"boolean", L"int i",
			nullptr, &RSScenePropertyClass::method_isParameterValidation,
			nullptr, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getParameterSequencer", L"SceneSequencer", L"int i",
			nullptr, &RSScenePropertyClass::method_getParameterSequencer,
			nullptr, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"createParameterSequencer", L"SceneSequencer", L"int i",
			nullptr, &RSScenePropertyClass::method_createParameterSequencer,
			nullptr, 0 ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"removeParameterSequencer", nullptr, L"int i",
			nullptr, &RSScenePropertyClass::method_removeParameterSequencer,
			nullptr, 0 ) ;
}

// ネイティブ型テスト
//////////////////////////////////////////////////////////////////////////////
bool RSScenePropertyClass::IsNativeObjectOf( ESLObject * pObj ) const
{
	return	(ESLTypeCast<S3DSceneComposer::ParameterProperty>(pObj) != nullptr) ;
}

// this オブジェクトを取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::S3DSceneComposer::ParameterProperty *
	RSScenePropertyClass::GetThisSceneProperty( RSContext& context, RSObject* pThis )
{
	S3DSceneComposer::ParameterProperty *	pProp =
		RSNativeObject::GetNative<S3DSceneComposer::ParameterProperty>( pThis ) ;
	if ( pProp == nullptr )
	{
		context.ThrowExceptionError( L"this が SceneProperty ではありません" ) ;
	}
	return	pProp ;
}

// const String getItemIdentity()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSScenePropertyClass::method_getItemIdentity
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::ParameterProperty *
			pThisProp = GetThisSceneProperty( context, pThis ) ;
	if ( pThisProp == nullptr )
	{
		return	nullptr ;
	}
	return	context.new_String( pThisProp->GetItemIdentity() ) ;
}

// void setItemIdentity( String id )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSScenePropertyClass::method_setItemIdentity
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::ParameterProperty *
			pThisProp = GetThisSceneProperty( context, pThis ) ;
	if ( pThisProp == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pThisProp->SetItemIdentity( arg.StringAt(0) ) ;
	return	nullptr ;
}

// const int getParameterCount()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSScenePropertyClass::method_getParameterCount
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::ParameterProperty *
			pThisProp = GetThisSceneProperty( context, pThis ) ;
	if ( pThisProp == nullptr )
	{
		return	nullptr ;
	}
	return	context.new_Integer( pThisProp->GetParameterCount() ) ;
}

// const String getParameterID( int i )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSScenePropertyClass::method_getParameterID
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::ParameterProperty *
			pThisProp = GetThisSceneProperty( context, pThis ) ;
	if ( pThisProp == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_String
		( pThisProp->GetParameterID( (size_t) arg.IntAt(0) ) ) ;
}

// const String getParameterFriendlyName( int i )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSScenePropertyClass::method_getParameterFriendlyName
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::ParameterProperty *
			pThisProp = GetThisSceneProperty( context, pThis ) ;
	if ( pThisProp == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_String
		( pThisProp->GetParameterFriendlyName( (size_t) arg.IntAt(0) ) ) ;
}

// const String getParameterDescription( int i )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSScenePropertyClass::method_getParameterDescription
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::ParameterProperty *
			pThisProp = GetThisSceneProperty( context, pThis ) ;
	if ( pThisProp == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_String
		( pThisProp->GetParameterDescription( (size_t) arg.IntAt(0) ) ) ;
}

// const int findParameterID( String id )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSScenePropertyClass::method_findParameterID
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::ParameterProperty *
			pThisProp = GetThisSceneProperty( context, pThis ) ;
	if ( pThisProp == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Integer( pThisProp->FindParameterID( arg.StringAt(0) ) ) ;
}

// const int getParameterType( int i )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSScenePropertyClass::method_getParameterType
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::ParameterProperty *
			pThisProp = GetThisSceneProperty( context, pThis ) ;
	if ( pThisProp == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Integer
		( pThisProp->GetParameterType( (size_t) arg.IntAt(0) ) ) ;
}

// const int getParameterAttributes( int i )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSScenePropertyClass::method_getParameterAttributes
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::ParameterProperty *
			pThisProp = GetThisSceneProperty( context, pThis ) ;
	if ( pThisProp == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Integer
		( pThisProp->GetParameterAttributes( (size_t) arg.IntAt(0) ) ) ;
}

// const boolean isParameterValidation( int i )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSScenePropertyClass::method_isParameterValidation
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::ParameterProperty *
			pThisProp = GetThisSceneProperty( context, pThis ) ;
	if ( pThisProp == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Boolean
		( pThisProp->IsParameterValidation( (size_t) arg.IntAt(0) ) ) ;
}

// const SceneSequencer getParameterSequencer( int i )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSScenePropertyClass::method_getParameterSequencer
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::ParameterProperty *
			pThisProp = GetThisSceneProperty( context, pThis ) ;
	if ( pThisProp == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DSceneComposer::Sequencer *	pSeq =
		pThisProp->GetParameterSequencer( (size_t) arg.IntAt(0) ) ;
	if ( pSeq == nullptr )
	{
		return	nullptr ;
	}
	return	new RSNativeObject( pSeq, context.GetClassAs( L"SceneSequencer" ) ) ;
}

// SceneSequencer createParameterSequencer( int i )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSScenePropertyClass::method_createParameterSequencer
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::ParameterProperty *
			pThisProp = GetThisSceneProperty( context, pThis ) ;
	if ( pThisProp == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DSceneComposer::Sequencer *	pSeq =
		pThisProp->CreateParameterSequencer( (size_t) arg.IntAt(0) ) ;
	if ( pSeq == nullptr )
	{
		return	nullptr ;
	}
	return	new RSNativeObject( pSeq, context.GetClassAs( L"SceneSequencer" ) ) ;
}

// void removeParameterSequencer( int i )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSScenePropertyClass::method_removeParameterSequencer
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::ParameterProperty *
			pThisProp = GetThisSceneProperty( context, pThis ) ;
	if ( pThisProp == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pThisProp->RemoveParameterSequencer( (size_t) arg.IntAt(0) ) ;
	return	nullptr ;
}



//////////////////////////////////////////////////////////////////////////////
// SceneController クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSSceneControllerClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSSceneControllerClass::RSSceneControllerClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// メンバ初期設定
//////////////////////////////////////////////////////////////////////////////
void RSSceneControllerClass::Initialize( RSContext& context )
{
	AddSuperClass( context, context.GetClassAs( L"SceneProperty" ) ) ;
	OverrideVirtuals( context ) ;
	m_flagInitialized = true ;
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSSceneControllerClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( nullptr, this ) ;
}

// ネイティブ型テスト
//////////////////////////////////////////////////////////////////////////////
bool RSSceneControllerClass::IsNativeObjectOf( ESLObject * pObj ) const
{
	return	(ESLTypeCast<S3DSceneComposer::Controller>(pObj) != nullptr) ;
}

// this オブジェクトを取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::S3DSceneComposer::Controller *
	RSSceneControllerClass::GetThisSceneController( RSContext& context, RSObject* pThis )
{
	S3DSceneComposer::Controller *	pCtrl =
		RSNativeObject::GetNative<S3DSceneComposer::Controller>( pThis ) ;
	if ( pCtrl == nullptr )
	{
		context.ThrowExceptionError( L"this が SceneController ではありません" ) ;
	}
	return	pCtrl ;
}



//////////////////////////////////////////////////////////////////////////////
// SceneItem クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSSceneItemClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSSceneItemClass::RSSceneItemClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// メンバ初期設定
//////////////////////////////////////////////////////////////////////////////
void RSSceneItemClass::Initialize( RSContext& context )
{
	AddSuperClass( context, context.GetClassAs( L"SceneProperty" ) ) ;
	OverrideVirtuals( context ) ;
	m_flagInitialized = true ;
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSSceneItemClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( nullptr, this ) ;
	//
	struct	ConstStrValue
	{
		const wchar_t *	pwszValue ;
		const wchar_t *	pwszName ;
	} ;
	static const ConstStrValue	s_csvDefs[] =
	{
		// Common
		{ L"position", L"paramPosition" },
		{ L"rotation", L"paramRotation" },
		{ L"zoom", L"paramZoom" },
		{ L"transparency", L"paramTransparency" },
		{ L"color_mul", L"paramColorMul" },
		{ L"color_add", L"paramColorAdd" },
		{ L"visible", L"paramVisible" },
		{ L"force_toon", L"paramForceToon" },
		{ L"force_border", L"paramForceBorder" },
		{ L"free_toon", L"paramFreeToon" },
		{ L"free_border", L"paramFreeBorder" },
		{ L"use_collision", L"paramUseCollision" },
		{ L"global_space", L"paramGlobalSpace" },
		{ L"camera_shift", L"paramCameraShift" },
		{ L"camera_space", L"paramCameraSpace" },
		{ L"hide_near", L"paramHideNear" },
		{ L"hide_far", L"paramHideFar" },
		// Item
		{ L"item_class", L"paramItemClass" },
		{ L"render_priority", L"paramItemPriority" },
		// Space
		{ L"space_ignore", L"paramIgnore" },
		{ L"layered_space", L"paramSetLayerSace" },
		{ L"layered_priority", L"paramLayeredPriority" },
		{ L"layered_param", L"paramLayeredParam" },
		{ L"layered_refl_map", L"paramLayeredReflMap" },
		{ L"layered_refr_map", L"paramLayeredRefrMap" },
		{ L"layered_field", L"paramLayeredField" },
		{ L"layered_static_item1", L"paramLayeredStaticItem1" },
		{ L"layered_static_item2", L"paramLayeredStaticItem2" },
		{ L"layered_dynamic_item1", L"paramLayeredDynamicItem1" },
		{ L"layered_dynamic_item2", L"paramLayeredDynamicItem2" },
		{ L"layered_dynamic_item3", L"paramLayeredDynamicItem3" },
		{ L"layered_effect_item", L"paramLayeredEffectItem" },
	} ;
	const size_t	nDefsCount = sizeof(s_csvDefs)/sizeof(ConstStrValue) ;
	for ( size_t i = 0; i < nDefsCount; i ++ )
	{
		CreateMemberStringAs
			( context, s_csvDefs[i].pwszName,
				s_csvDefs[i].pwszValue, modifierConst ) ;
	}
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"getParentSpace", L"SceneItem", L"",
			nullptr, &RSSceneItemClass::method_getParentSpace,
			nullptr, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getComposition", L"SceneComposition", L"",
			nullptr, &RSSceneItemClass::method_getComposition,
			nullptr, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getComposer", L"SceneComposer", L"",
			nullptr, &RSSceneItemClass::method_getComposer,
			nullptr, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getManager", L"SceneManager", L"",
			nullptr, &RSSceneItemClass::method_getManager,
			nullptr, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getScene", L"Scene", L"",
			nullptr, &RSSceneItemClass::method_getScene,
			nullptr, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getSceneItemAs", L"SceneItem", L"String id",
			nullptr, &RSSceneItemClass::method_getSceneItemAs,
			nullptr, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getChildItemCount", L"int", L"",
			nullptr, &RSSceneItemClass::method_getChildItemCount,
			nullptr, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getChildItemAt", L"SceneItem", L"int i",
			nullptr, &RSSceneItemClass::method_getChildItemAt,
			nullptr, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getControllerCount", L"int", L"",
			nullptr, &RSSceneItemClass::method_getControllerCount,
			nullptr, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getControllerAt", L"SceneController", L"int i",
			nullptr, &RSSceneItemClass::method_getControllerAt,
			nullptr, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"createController", L"SceneController",
			L"int i, String typeId, String ctrlId",
			nullptr, &RSSceneItemClass::method_createController,
			nullptr, 0 ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getControllerAs", L"SceneController", L"String id",
			nullptr, &RSSceneItemClass::method_getControllerAs,
			nullptr, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getGlobalTransformation",
			nullptr, L"Matrix3D matGlobal, Vector3D vGlobalPos",
			nullptr, &RSSceneItemClass::method_getGlobalTransformation,
			nullptr, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getItemRotation", L"Matrix3D", L"",
			nullptr, &RSSceneItemClass::method_getItemRotation,
			nullptr, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setItemRotation", nullptr, L"Matrix3D matRotation",
			nullptr, &RSSceneItemClass::method_setItemRotation, nullptr ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getItemZoom", L"Vector3D", L"",
			nullptr, &RSSceneItemClass::method_getItemZoom,
			nullptr, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setItemZoom", nullptr, L"Vector3D vZoom",
			nullptr, &RSSceneItemClass::method_setItemZoom, nullptr ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getItemPosition", L"Vector3D", L"",
			nullptr, &RSSceneItemClass::method_getItemPosition,
			nullptr, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setItemPosition", nullptr, L"Vector3D vPos",
			nullptr, &RSSceneItemClass::method_setItemPosition, nullptr ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"isItemVisible", L"boolean", L"",
			nullptr, &RSSceneItemClass::method_isItemVisible,
			nullptr, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setItemVisible", nullptr, L"boolean flagVisible",
			nullptr, &RSSceneItemClass::method_setItemVisible, nullptr ) ;
}

// ネイティブ型テスト
//////////////////////////////////////////////////////////////////////////////
bool RSSceneItemClass::IsNativeObjectOf( ESLObject * pObj ) const
{
	return	(ESLTypeCast<S3DSceneComposer::ItemSerializer>(pObj) != nullptr) ;
}

// this オブジェクトを取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::S3DSceneComposer::ItemSerializer *
	RSSceneItemClass::GetThisSceneItem( RSContext& context, RSObject* pThis )
{
	S3DSceneComposer::ItemSerializer *	pItem =
		RSNativeObject::GetNative<S3DSceneComposer::ItemSerializer>( pThis ) ;
	if ( pItem == nullptr )
	{
		context.ThrowExceptionError( L"this が SceneItem ではありません" ) ;
	}
	return	pItem ;
}

// const SceneItem getParentSpace()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneItemClass::method_getParentSpace
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::ItemSerializer *
			pThisItem = GetThisSceneItem( context, pThis ) ;
	if ( pThisItem == nullptr )
	{
		return	nullptr ;
	}
	S3DSceneComposer::ItemSerializer *	pParent = pThisItem->GetParentSpaceItem() ;
	if ( pParent == nullptr )
	{
		return	nullptr ;
	}
	return	new RSNativeObject( pParent, context.GetClassAs( L"SceneItem" ) ) ;
}

// const SceneComposition getComposition()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneItemClass::method_getComposition
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::ItemSerializer *
			pThisItem = GetThisSceneItem( context, pThis ) ;
	if ( pThisItem == nullptr )
	{
		return	nullptr ;
	}
	S3DSceneComposer::Composition *	pComp = pThisItem->GetComposition() ;
	if ( pComp == nullptr )
	{
		return	nullptr ;
	}
	return	new RSNativeObject
				( S3DSceneComposer::Composition::GetESLPointer( pComp ),
								context.GetClassAs( L"SceneComposition" ) ) ;
}

// const SceneComposer getComposer()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneItemClass::method_getComposer
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::ItemSerializer *
			pThisItem = GetThisSceneItem( context, pThis ) ;
	if ( pThisItem == nullptr )
	{
		return	nullptr ;
	}
	S3DSceneComposer *	pComp = pThisItem->GetComposer() ;
	if ( pComp == nullptr )
	{
		return	nullptr ;
	}
	return	new RSNativeObject
				( pComp, context.GetClassAs( L"SceneComposer" ) ) ;
}

// const SceneManager getManager()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneItemClass::method_getManager
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::ItemSerializer *
			pThisItem = GetThisSceneItem( context, pThis ) ;
	if ( pThisItem == nullptr )
	{
		return	nullptr ;
	}
	S3DCompositionManager *	pManager = pThisItem->GetManager() ;
	if ( pManager == nullptr )
	{
		return	nullptr ;
	}
	return	new RSNativeObject( pManager, context.GetClassAs( L"SceneManager" ) ) ;
}

// const Scene getScene()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneItemClass::method_getScene
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::ItemSerializer *
			pThisItem = GetThisSceneItem( context, pThis ) ;
	if ( pThisItem == nullptr )
	{
		return	nullptr ;
	}
	S3DScene *	pScene = pThisItem->GetScene() ;
	if ( pScene == nullptr )
	{
		return	nullptr ;
	}
	return	new RSNativeObject( pScene, context.GetClassAs( L"Scene" ) ) ;
}

// const SceneItem getSceneItemAs( String id )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneItemClass::method_getSceneItemAs
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::ItemSerializer *
			pThisItem = GetThisSceneItem( context, pThis ) ;
	if ( pThisItem == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DSceneComposer::ItemSerializer *
			pItem = pThisItem->GetSceneItemAs( arg.StringAt(0) ) ;
	if ( pItem == nullptr )
	{
		return	nullptr ;
	}
	return	new RSNativeObject( pItem, context.GetClassAs( L"SceneItem" ) ) ;
}

// const int getChildItemCount()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneItemClass::method_getChildItemCount
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::ItemSerializer *
			pThisItem = GetThisSceneItem( context, pThis ) ;
	if ( pThisItem == nullptr )
	{
		return	nullptr ;
	}
	return	context.new_Integer( pThisItem->GetChildItemCount() ) ;
}

// const SceneItem getChildItemAt( int i )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneItemClass::method_getChildItemAt
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::ItemSerializer *
			pThisItem = GetThisSceneItem( context, pThis ) ;
	if ( pThisItem == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DSceneComposer::ItemSerializer *
			pItem = pThisItem->GetChildItemAt( (size_t) arg.IntAt(0) ) ;
	if ( pItem == nullptr )
	{
		return	nullptr ;
	}
	return	new RSNativeObject( pItem, context.GetClassAs( L"SceneItem" ) ) ;
}

// const int getControllerCount()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneItemClass::method_getControllerCount
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::ItemSerializer *
			pThisItem = GetThisSceneItem( context, pThis ) ;
	if ( pThisItem == nullptr )
	{
		return	nullptr ;
	}
	return	context.new_Integer( pThisItem->GetControllerCount() ) ;
}

// const SceneController getControllerAt( int i )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneItemClass::method_getControllerAt
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::ItemSerializer *
			pThisItem = GetThisSceneItem( context, pThis ) ;
	if ( pThisItem == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DSceneComposer::Controller *
			pCtrl = pThisItem->GetControllerAt( (size_t) arg.IntAt(0) ) ;
	if ( pCtrl == nullptr )
	{
		return	nullptr ;
	}
	return	new RSNativeObject( pCtrl, context.GetClassAs( L"SceneController" ) ) ;
}

// SceneController createController( int i, String typeId, String ctrlId )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneItemClass::method_createController
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::ItemSerializer *
			pThisItem = GetThisSceneItem( context, pThis ) ;
	if ( pThisItem == nullptr )
	{
		return	nullptr ;
	}
	S3DSceneComposer *	pComposer = pThisItem->GetComposer() ;
	if ( pComposer == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DSceneComposer::Controller *
			pCtrl = pComposer->CreateController( arg.StringAt(1) ) ;
	if ( pCtrl == nullptr )
	{
		return	nullptr ;
	}
	pCtrl->SetItemIdentity( arg.StringAt(2) ) ;
	pThisItem->InsertController( (size_t) arg.IntAt(0), pCtrl ) ;
	return	new RSNativeObject( pCtrl, context.GetClassAs( L"SceneController" ) ) ;
}

// const SceneController getControllerAs( String id )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneItemClass::method_getControllerAs
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::ItemSerializer *
			pThisItem = GetThisSceneItem( context, pThis ) ;
	if ( pThisItem == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DSceneComposer::Controller *
			pCtrl = pThisItem->GetControllerAs( arg.StringAt(0) ) ;
	if ( pCtrl == nullptr )
	{
		return	nullptr ;
	}
	return	new RSNativeObject( pCtrl, context.GetClassAs( L"SceneController" ) ) ;
}

// const void getGlobalTransformation( Matrix3D matGlobal, Vector3D vGlobalPos )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneItemClass::method_getGlobalTransformation
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::ItemSerializer *
			pThisItem = GetThisSceneItem( context, pThis ) ;
	if ( pThisItem == nullptr )
	{
		return	nullptr ;
	}
	S3DDMatrix	matGlobal ;
	S3DDVector	vGlobal ;
	pThisItem->GetGlobalTransformation( matGlobal, vGlobal ) ;
	//
	RSContext::SArgList	arg( ppArg, count ) ;
	SGL3DMatrix<float32_t,3> *	pMat3 =
		(SGL3DMatrix<float32_t,3>*)
			arg.PointerAt( 0, sizeof(SGL3DMatrix<float32_t,3>) ) ;
	S3DVector *	pVec = (S3DVector*) arg.PointerAt( 0, sizeof(S3DVector) ) ;

	if ( pMat3 != nullptr )
	{
		for ( int i = 0; i < 3; i ++ )
		{
			pMat3->m[i][0] = (float32_t) matGlobal.m[i][0] ;
			pMat3->m[i][1] = (float32_t) matGlobal.m[i][1] ;
			pMat3->m[i][2] = (float32_t) matGlobal.m[i][2] ;
		}
	}
	if ( pVec != nullptr )
	{
		*pVec = vGlobal ;
	}
	return	nullptr ;
}

// const Matrix3D getItemRotation()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneItemClass::method_getItemRotation
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::CommonSerializer *
			pThisItem = ESLTypeCast<S3DSceneComposer::CommonSerializer>
									( GetThisSceneItem( context, pThis ) ) ;
	if ( pThisItem == nullptr )
	{
		return	nullptr ;
	}
	S3DDMatrix	matRot = pThisItem->GetItemRotation() ;

	RSStructuredPointer *	pObj =
		ESLSmartCast<RSStructuredPointer>
			( context.new_StructuredPointer( L"Matrix3D" ) ) ;
	ESLAssert( pObj != nullptr ) ;
	if ( pObj != nullptr )
	{
		SGL3DMatrix<float32_t,3> *	pMat3 =
				pObj->GetPtr< SGL3DMatrix<float32_t,3> >() ;
		for ( int i = 0; i < 3; i ++ )
		{
			pMat3->m[i][0] = (float32_t) matRot.m[i][0] ;
			pMat3->m[i][1] = (float32_t) matRot.m[i][1] ;
			pMat3->m[i][2] = (float32_t) matRot.m[i][2] ;
		}
	}
	return	pObj ;
}

// void setItemRotation( Matrix3D matRotation )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneItemClass::method_setItemRotation
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::CommonSerializer *
			pThisItem = ESLTypeCast<S3DSceneComposer::CommonSerializer>
									( GetThisSceneItem( context, pThis ) ) ;
	if ( pThisItem == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGL3DMatrix<float32_t,3> *	pMat3 =
		(SGL3DMatrix<float32_t,3>*)
			arg.PointerAt( 0, sizeof(SGL3DMatrix<float32_t,3>) ) ;
	if ( pMat3 != nullptr )
	{
		S3DDMatrix	matRot ;
		for ( int i = 0; i < 3; i ++ )
		{
			matRot.m[i][0] = pMat3->m[i][0] ;
			matRot.m[i][1] = pMat3->m[i][1] ;
			matRot.m[i][2] = pMat3->m[i][2] ;
		}
		pThisItem->SetItemRotation( matRot ) ;
	}
	return	nullptr ;
}

// const Vector3D getItemZoom()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneItemClass::method_getItemZoom
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::CommonSerializer *
			pThisItem = ESLTypeCast<S3DSceneComposer::CommonSerializer>
									( GetThisSceneItem( context, pThis ) ) ;
	if ( pThisItem == nullptr )
	{
		return	nullptr ;
	}
	S3DDVector	vZoom = pThisItem->GetItemZoom() ;

	RSStructuredPointer *	pObj =
		ESLSmartCast<RSStructuredPointer>
			( context.new_StructuredPointer( L"Vector3D" ) ) ;
	ESLAssert( pObj != nullptr ) ;
	if ( pObj != nullptr )
	{
		*(pObj->GetPtr<S3DVector>()) = vZoom ;
	}
	return	pObj ;
}

// void setItemZoom( Vector3D vZoom )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneItemClass::method_setItemZoom
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::CommonSerializer *
			pThisItem = ESLTypeCast<S3DSceneComposer::CommonSerializer>
									( GetThisSceneItem( context, pThis ) ) ;
	if ( pThisItem == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DVector *	pVec = (S3DVector*) arg.PointerAt( 0, sizeof(S3DVector) ) ;
	if ( pVec != nullptr )
	{
		S3DDVector	vZoom = *pVec ;
		pThisItem->SetItemZoom( vZoom ) ;
	}
	return	nullptr ;
}

// const Vector3D getItemPosition()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneItemClass::method_getItemPosition
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::CommonSerializer *
			pThisItem = ESLTypeCast<S3DSceneComposer::CommonSerializer>
									( GetThisSceneItem( context, pThis ) ) ;
	if ( pThisItem == nullptr )
	{
		return	nullptr ;
	}
	S3DDVector	vPos = pThisItem->GetItemPosition() ;

	RSStructuredPointer *	pObj =
		ESLSmartCast<RSStructuredPointer>
			( context.new_StructuredPointer( L"Vector3D" ) ) ;
	ESLAssert( pObj != nullptr ) ;
	if ( pObj != nullptr )
	{
		*(pObj->GetPtr<S3DVector>()) = vPos ;
	}
	return	pObj ;
}

// void setItemPosition( Vector3D vPos )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneItemClass::method_setItemPosition
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::CommonSerializer *
			pThisItem = ESLTypeCast<S3DSceneComposer::CommonSerializer>
									( GetThisSceneItem( context, pThis ) ) ;
	if ( pThisItem == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DVector *	pVec = (S3DVector*) arg.PointerAt( 0, sizeof(S3DVector) ) ;
	if ( pVec != nullptr )
	{
		S3DDVector	vPos = *pVec ;
		pThisItem->SetItemPositioin( vPos ) ;
	}
	return	nullptr ;
}

// const boolean isItemVisible()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneItemClass::method_isItemVisible
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::CommonSerializer *
			pThisItem = ESLTypeCast<S3DSceneComposer::CommonSerializer>
									( GetThisSceneItem( context, pThis ) ) ;
	if ( pThisItem == nullptr )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean( pThisItem->GetVisibleParameter() ) ;
}

// void setItemVisible( boolean flagVisible )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneItemClass::method_setItemVisible
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::CommonSerializer *
			pThisItem = ESLTypeCast<S3DSceneComposer::CommonSerializer>
									( GetThisSceneItem( context, pThis ) ) ;
	if ( pThisItem == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pThisItem->SetVisibleParameter( arg.BooleanAt(0) ) ;
	return	nullptr ;
}



//////////////////////////////////////////////////////////////////////////////
// SceneComposition クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSSceneCompositionClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSSceneCompositionClass::RSSceneCompositionClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// メンバ初期設定
//////////////////////////////////////////////////////////////////////////////
void RSSceneCompositionClass::Initialize( RSContext& context )
{
	AddSuperClass( context, context.GetClassAs( L"SceneItem" ) ) ;
	OverrideVirtuals( context ) ;
	m_flagInitialized = true ;
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSSceneCompositionClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( nullptr, this ) ;
	//
	CreateMemberIntegerAs
		( context, L"seekStream",
			S3DSceneComposer::seekStream, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"seekJump",
			S3DSceneComposer::seekJump, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"seekJumpReset",
			S3DSceneComposer::seekJumpReset, modifierConst ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"postTimelineFrame", nullptr, L"double frame",
			nullptr, &RSSceneCompositionClass::method_postTimelineFrame, nullptr ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"createSpaceChild", L"SceneItem",
			L"SceneItem space, String typeId, String itemId",
			nullptr, &RSSceneCompositionClass::method_createSpaceChild, nullptr ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"removeSpaceChild", L"boolean",
			L"SceneItem space, SceneItem item",
			nullptr, &RSSceneCompositionClass::method_removeSpaceChild, nullptr ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"delayRemoveItem", nullptr,
			L"SceneItem space, SceneItem item",
			nullptr, &RSSceneCompositionClass::method_delayRemoveItem, nullptr ) ;
}

// ネイティブ型テスト
//////////////////////////////////////////////////////////////////////////////
bool RSSceneCompositionClass::IsNativeObjectOf( ESLObject * pObj ) const
{
	return	(ESLTypeCast<S3DSceneComposer::Composition>(pObj) != nullptr) ;
}

// this オブジェクトを取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::S3DSceneComposer::Composition *
	RSSceneCompositionClass::GetThisSceneComposition( RSContext& context, RSObject* pThis )
{
	S3DSceneComposer::Composition *	pComp =
		RSNativeObject::GetNative<S3DSceneComposer::Composition>( pThis ) ;
	if ( pComp == nullptr )
	{
		context.ThrowExceptionError( L"this が SceneComposition ではありません" ) ;
	}
	return	pComp ;
}

// void seekCompositionFrame( double frame )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneCompositionClass::method_postTimelineFrame
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::Composition *
			pThisComp = GetThisSceneComposition( context, pThis ) ;
	if ( pThisComp == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pThisComp->PostTimelineFrame( arg.DoubleAt(0) ) ;
	return	nullptr ;
}

// SceneItem createSpaceChild
//	( SceneItem space, String typeId, String itemId ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneCompositionClass::method_createSpaceChild
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::Composition *
			pThisComp = GetThisSceneComposition( context, pThis ) ;
	if ( pThisComp == nullptr )
	{
		return	nullptr ;
	}
	S3DSceneComposer *	pComposer = pThisComp->GetComposer() ;
	if ( pComposer == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DSceneComposer::SpaceSerializer *	pSpaceItem =
		ESLTypeCast<S3DSceneComposer::SpaceSerializer>( arg.NativeObjectAt( 0 ) ) ;
	if ( pSpaceItem == nullptr )
	{
		return	nullptr ;
	}
	S3DSceneComposer::ItemSerializer *
			pItem = pComposer->CreateSceneItem( arg.StringAt(1) ) ;
	if ( pItem == nullptr )
	{
		return	nullptr ;
	}
	pThisComp->AddSpaceChild( *pSpaceItem, pItem, arg.StringAt(2) ) ;
	return	new RSNativeObject( pItem, context.GetClassAs( L"SceneItem" ) ) ;
}

// boolean removeSpaceChild( SceneItem space, SceneItem item ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneCompositionClass::method_removeSpaceChild
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::Composition *
			pThisComp = GetThisSceneComposition( context, pThis ) ;
	if ( pThisComp == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DSceneComposer::SpaceSerializer *	pSpaceItem =
		ESLTypeCast<S3DSceneComposer::SpaceSerializer>( arg.NativeObjectAt( 0 ) ) ;
	if ( pSpaceItem == nullptr )
	{
		return	context.new_Boolean( false ) ;
	}
	S3DSceneComposer::ItemSerializer *	pItem =
		ESLTypeCast<S3DSceneComposer::ItemSerializer>( arg.NativeObjectAt( 1 ) ) ;
	if ( pItem == nullptr )
	{
		return	context.new_Boolean( false ) ;
	}
	if ( pThisComp->RemoveSpaceChild( *pSpaceItem, pItem ) )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean( true ) ;
}

// void delayRemoveItem( SceneItem space, SceneItem item ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSceneCompositionClass::method_delayRemoveItem
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	S3DSceneComposer::Composition *
			pThisComp = GetThisSceneComposition( context, pThis ) ;
	if ( pThisComp == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S3DSceneComposer::SpaceSerializer *	pSpaceItem =
		ESLTypeCast<S3DSceneComposer::SpaceSerializer>( arg.NativeObjectAt( 0 ) ) ;
	if ( pSpaceItem == nullptr )
	{
		return	nullptr ;
	}
	S3DSceneComposer::ItemSerializer *	pItem =
		ESLTypeCast<S3DSceneComposer::ItemSerializer>( arg.NativeObjectAt( 1 ) ) ;
	if ( pItem == nullptr )
	{
		return	nullptr ;
	}
	pThisComp->PostDelayRemoveItem( *pSpaceItem, pItem ) ;
	return	nullptr ;
}



