
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_ModelBuffer.h>


// ModelBuffer( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_ModelBuffer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ
		( LEntisGLS4_ModelBuffer, pThis,
			( new SSmartObject
				( (S3DRenderBufferInterface*) new S3DModelBuffer ) ) ) ;

	LQT_RETURN_VOID() ;
}

// boolean loadModel( String file, String mime )
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBuffer_loadModel)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBuffer, pThis ) ;
	S3DModelBuffer *	pModel = pThis->GetRef<S3DModelBuffer>() ;
	LQT_VERIFY_NULL_PTR( pModel ) ;
	LQT_FUNC_ARG_STRING( file ) ;
	LQT_FUNC_ARG_STRING( mime ) ;

	LQT_RETURN_BOOL
		( pModel->LoadModel( file.c_str(), mime.c_str() ) == sglErrSuccess ) ;
}

// boolean readModel( File file, String mime )
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBuffer_readModel)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBuffer, pThis ) ;
	S3DModelBuffer *	pModel = pThis->GetRef<S3DModelBuffer>() ;
	LQT_VERIFY_NULL_PTR( pModel ) ;
	LQT_FUNC_ARG_NOBJ( LPureFile, file ) ;
	LQT_VERIFY_NULL_PTR( file ) ;
	LQT_FUNC_ARG_STRING( mime ) ;

	SLoquatyFile	lfile( file ) ;
	LQT_RETURN_BOOL( pModel->ReadModel( &lfile, mime.c_str() ) == sglErrSuccess ) ;
}

// boolean saveModel( String file, String mime, String imageMIME )
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBuffer_saveModel)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBuffer, pThis ) ;
	S3DModelBuffer *	pModel = pThis->GetRef<S3DModelBuffer>() ;
	LQT_VERIFY_NULL_PTR( pModel ) ;
	LQT_FUNC_ARG_STRING( file ) ;
	LQT_FUNC_ARG_STRING( mime ) ;
	LQT_FUNC_ARG_STRING( imageMIME ) ;

	LQT_RETURN_BOOL
		( pModel->SaveModel
			( file.c_str(), mime.c_str(), imageMIME.c_str() ) == sglErrSuccess ) ;
}

// boolean writeModel( File file, String mime, String imageMIME )
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBuffer_writeModel)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBuffer, pThis ) ;
	S3DModelBuffer *	pModel = pThis->GetRef<S3DModelBuffer>() ;
	LQT_VERIFY_NULL_PTR( pModel ) ;
	LQT_FUNC_ARG_NOBJ( LPureFile, file ) ;
	LQT_VERIFY_NULL_PTR( file ) ;
	LQT_FUNC_ARG_STRING( mime ) ;
	LQT_FUNC_ARG_STRING( imageMIME ) ;

	SLoquatyFile	lfile( file ) ;
	LQT_RETURN_BOOL
		( pModel->WriteModel
			( &lfile, mime.c_str(), imageMIME.c_str() ) == sglErrSuccess ) ;
}

// EntisGLS4.TextureLibrary getTextureLibrary( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBuffer_getTextureLibrary)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBuffer, pThis ) ;
	S3DModelBuffer *	pModel = pThis->GetRef<S3DModelBuffer>() ;
	LQT_VERIFY_NULL_PTR( pModel ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.TextureLibrary) ) ) ;
	valRet->SetNative
		( std::make_shared<LEntisGLS4_TextureLibrary>
						( &(pModel->GetTextureLibrary()) ) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.MaterialLibrary getMaterialLibrary( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBuffer_getMaterialLibrary)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBuffer, pThis ) ;
	S3DModelBuffer *	pModel = pThis->GetRef<S3DModelBuffer>() ;
	LQT_VERIFY_NULL_PTR( pModel ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.MaterialLibrary) ) ) ;
	valRet->SetNative
		( std::make_shared<LEntisGLS4_MaterialLibrary>
						( &(pModel->GetMaterialLibrary()) ) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.ModelPoseLibrary getPoseLibrary( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBuffer_getPoseLibrary)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBuffer, pThis ) ;
	S3DModelBuffer *	pModel = pThis->GetRef<S3DModelBuffer>() ;
	LQT_VERIFY_NULL_PTR( pModel ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.ModelPoseLibrary) ) ) ;
	valRet->SetNative
		( std::make_shared<LEntisGLS4_ModelPoseLibrary>( &(pModel->GetPoseLibrary()) ) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.ModelBone getBoneAs( String boneId ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBuffer_getBoneAs)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBuffer, pThis ) ;
	S3DModelBuffer *	pModel = pThis->GetRef<S3DModelBuffer>() ;
	LQT_VERIFY_NULL_PTR( pModel ) ;
	LQT_FUNC_ARG_STRING( boneId ) ;

	S3DModelBoneSpace *	pBone = pModel->GetBonePropertyAs( boneId.c_str() ) ;
	if ( pBone == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.ModelBone) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_ModelBone>(pBone) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// String getBoneIdentityOf( EntisGLS4.ModelBone bone ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBuffer_getBoneIdentityOf)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBuffer, pThis ) ;
	S3DModelBuffer *	pModel = pThis->GetRef<S3DModelBuffer>() ;
	LQT_VERIFY_NULL_PTR( pModel ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_ModelBone, bone ) ;
	LQT_VERIFY_NULL_PTR( bone ) ;
	S3DModelBoneSpace *	pBone = bone->GetRef<S3DModelBoneSpace>() ;
	LQT_VERIFY_NULL_PTR( pBone ) ;

	const SSystem::SString *	pstrId = pModel->GetBoneIdentityOf( pBone ) ;
	if ( pstrId == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}

	LQT_RETURN_STRING( *pstrId ) ;
}

// ulong getMarkerCount( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBuffer_getMarkerCount)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBuffer, pThis ) ;
	S3DModelBuffer *	pModel = pThis->GetRef<S3DModelBuffer>() ;
	LQT_VERIFY_NULL_PTR( pModel ) ;

	LQT_RETURN_ULONG( pModel->GetMarkerInfoCount() ) ;
}

// String getMarkerIdentityAt( ulong index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBuffer_getMarkerIdentityAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBuffer, pThis ) ;
	S3DModelBuffer *	pModel = pThis->GetRef<S3DModelBuffer>() ;
	LQT_VERIFY_NULL_PTR( pModel ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	const SString *	pstrId = pModel->GetMarkerInfoIdentityAt( (size_t) index ) ;
	if ( pstrId == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}

	LQT_RETURN_STRING( *pstrId ) ;
}

// EntisGLS4.ModelMarker getMarkerAt( ulong index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBuffer_getMarkerAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBuffer, pThis ) ;
	S3DModelBuffer *	pModel = pThis->GetRef<S3DModelBuffer>() ;
	LQT_VERIFY_NULL_PTR( pModel ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	S3DModelData::MarkerInfo *	pMarker = pModel->GetMarkerInfoAt( (size_t) index ) ;
	if ( pMarker == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.ModelMarker) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_ModelMarker>(pMarker) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.ModelMarker getMarkerAs( String markerId ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBuffer_getMarkerAs)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBuffer, pThis ) ;
	S3DModelBuffer *	pModel = pThis->GetRef<S3DModelBuffer>() ;
	LQT_VERIFY_NULL_PTR( pModel ) ;
	LQT_FUNC_ARG_STRING( markerId ) ;

	S3DModelData::MarkerInfo *	pMarker = pModel->GetMarkerInfoAs( markerId.c_str() ) ;
	if ( pMarker == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.ModelMarker) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_ModelMarker>(pMarker) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// const Vector3d* calcMarkerMatrix( float* pRadius, Matrix3d* pMarkerMatrix, EntisGLS4.ModelMarker marker, const Matrix3d* matSpace, const Vector3d* vSpace ) const
IMPL_LOQUATY_FUNC(EntisGLS4_ModelBuffer_calcMarkerMatrix)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_ModelBuffer, pThis ) ;
	S3DModelBuffer *	pModel = pThis->GetRef<S3DModelBuffer>() ;
	LQT_VERIFY_NULL_PTR( pModel ) ;
	LQT_FUNC_ARG_POINTER( LFloat, pRadius ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix3d, pMarkerMatrix ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_ModelMarker, marker ) ;
	LQT_VERIFY_NULL_PTR( marker ) ;
	S3DModelData::MarkerInfo *	pMarker = marker->GetRef<S3DModelData::MarkerInfo>() ;
	LQT_VERIFY_NULL_PTR( pMarker ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix3d, pSpaceMatrix ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, pSpacePos ) ;

	S3DDMatrix	matSpace( 1, 1, 1 ) ;
	S3DDVector	vSpace( 0, 0, 0 ) ;
	if ( pSpaceMatrix != nullptr )
	{
		matSpace = *pSpaceMatrix ;
	}
	if ( pSpacePos != nullptr )
	{
		vSpace = *pSpacePos ;
	}

	LVector3d	valRet ;
	float	fpRadius ;
	pModel->CalcMarkerTransformation
		( valRet, fpRadius, matSpace, vSpace, *pMarker ) ;
	if ( pRadius != nullptr )
	{
		*pRadius = fpRadius ;
	}
	if ( pMarkerMatrix != nullptr )
	{
		*pMarkerMatrix = matSpace ;
	}

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}


