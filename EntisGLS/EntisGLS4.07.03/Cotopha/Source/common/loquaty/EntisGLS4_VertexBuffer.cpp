
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_VertexBuffer.h>


// VertexBuffer( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_VertexBuffer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ
		( LEntisGLS4_VertexBuffer, pThis,
			( new SSmartObject
				( (S3DRenderBufferInterface*) new S3DVertexBuffer ) ) ) ;

	LQT_RETURN_VOID() ;
}

// uint getBufferControlFlags( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_getBufferControlFlags)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	S3DVertexBufferInterface *	pVB = pThis->GetRef<S3DVertexBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pVB ) ;

	LQT_RETURN_UINT( pVB->GetBufferControlFlags() ) ;
}

// void setBufferControlFlags( uint nFlags )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_setBufferControlFlags)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	S3DVertexBufferInterface *	pVB = pThis->GetRef<S3DVertexBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pVB ) ;
	LQT_FUNC_ARG_UINT( nFlags ) ;

	pVB->SetBufferControlFlags( nFlags ) ;

	LQT_RETURN_VOID() ;
}

// EntisGLS4.Material getDefaultMaterial( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_getDefaultMaterial)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	S3DVertexBufferInterface *	pVB = pThis->GetRef<S3DVertexBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pVB ) ;

	S3DMaterial *	pMaterial = pVB->GetDefaultMaterial() ;
	if ( pMaterial == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.Material) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_Material>(pMaterial) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// void attachDefaultMaterial( EntisGLS4.Material material )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_attachDefaultMaterial)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	S3DVertexBufferInterface *	pVB = pThis->GetRef<S3DVertexBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pVB ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Material, material ) ;
	S3DMaterial *	pMaterial = nullptr ;
	if ( material != nullptr )
	{
		pMaterial = material->GetRef<S3DMaterial>() ;
	}
	pVB->AttachDefaultMaterial( pMaterial ) ;

	LQT_RETURN_VOID() ;
}

// boolean allocatePrimitiveBuffer( EntisGLS4.VertexBuffer.PrimitiveBuffer prmbuf, int typePrimitive, ulong countIndex, ulong countVertex )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_allocatePrimitiveBuffer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	S3DVertexBufferInterface *	pVB = pThis->GetRef<S3DVertexBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pVB ) ;
	LQT_FUNC_ARG_OBJECT( LEntisGLS4_VertexBuffer_PrimitiveBuffer, prmbuf ) ;
	LQT_VERIFY_NULL_PTR( prmbuf ) ;
	LQT_FUNC_ARG_INT( typePrimitive ) ;
	LQT_FUNC_ARG_ULONG( countIndex ) ;
	LQT_FUNC_ARG_ULONG( countVertex ) ;

	S3DVertexBufferInterface::PrimitiveBuffer	bufTemp ;
	LBoolean	valRet =
		(pVB->AllocatePrimitiveBuffer
			( bufTemp, (S3DPrimitiveType) typePrimitive,
				(size_t) countIndex, (size_t) countVertex ) == sglErrSuccess) ;
	if ( valRet )
	{
		prmbuf->SetElementPointerAs
			( L"pvVertex",
				std::make_shared<LArrayBufAlias>
					( (uint8_t*) bufTemp.pvVertex,
						(size_t) countVertex * sizeof(S3DVector4) ) ) ;
		prmbuf->SetElementPointerAs
			( L"pvNormal",
				std::make_shared<LArrayBufAlias>
					( (uint8_t*) bufTemp.pvNormal,
						(size_t) countVertex * sizeof(S3DVector4) ) ) ;
		prmbuf->SetElementPointerAs
			( L"pvUVMap",
				std::make_shared<LArrayBufAlias>
					( (uint8_t*) bufTemp.pvNormal,
						(size_t) countVertex * sizeof(S2DVector) ) ) ;
		prmbuf->SetElementPointerAs
			( L"pColor",
				std::make_shared<LArrayBufAlias>
					( (uint8_t*) bufTemp.pColor,
						(size_t) countVertex * sizeof(S3DColor) ) ) ;
		if ( bufTemp.pIndexedList != nullptr )
		{
			prmbuf->SetElementPointerAs
				( L"pIndexedList",
					std::make_shared<LArrayBufAlias>
						( (uint8_t*) bufTemp.pIndexedList,
							(size_t) countIndex * sizeof(uint32_t)
								* GetPrimitiveVertexCount
									( (S3DPrimitiveType) typePrimitive ) ) ) ;
		}
		else
		{
			prmbuf->SetElementPointerAs( L"pIndexedList", nullptr ) ;
		}
	}

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean addPrimitiveBuffer( EntisGLS4.Material material, uint flags, int typePrimitive, const EntisGLS4.VertexBuffer.PrimitiveBuffer prmbuf, ulong countIndex, ulong countVertex )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_addPrimitiveBuffer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	S3DVertexBufferInterface *	pVB = pThis->GetRef<S3DVertexBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pVB ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Material, material ) ;
	S3DMaterial *	pMaterial = nullptr ;
	if ( material != nullptr )
	{
		pMaterial = material->GetRef<S3DMaterial>() ;
	}
	LQT_FUNC_ARG_UINT( flags ) ;
	LQT_FUNC_ARG_INT( typePrimitive ) ;
	LQT_FUNC_ARG_OBJECT( LEntisGLS4_VertexBuffer_PrimitiveBuffer, prmbuf ) ;
	LQT_VERIFY_NULL_PTR( prmbuf ) ;
	LQT_FUNC_ARG_ULONG( countIndex ) ;
	LQT_FUNC_ARG_ULONG( countVertex ) ;

	S3DVertexBufferInterface::PrimitiveBuffer	bufTemp ;
	bufTemp.pvVertex =
		(S3DVector4*) prmbuf->GetElementPointerAs
			( L"pvVertex", (size_t) countVertex * sizeof(S3DVector4) ) ;
	bufTemp.pvNormal =
		(S3DVector4*) prmbuf->GetElementPointerAs
			( L"pvNormal", (size_t) countVertex * sizeof(S3DVector4) ) ;
	bufTemp.pvUVMap =
		(S2DVector*) prmbuf->GetElementPointerAs
			( L"pvUVMap", (size_t) countVertex * sizeof(S2DVector) ) ;
	bufTemp.pColor =
		(S3DColor*) prmbuf->GetElementPointerAs
			( L"pColor", (size_t) countVertex * sizeof(S3DColor) ) ;
	bufTemp.pIndexedList =
		(uint32_t*) prmbuf->GetElementPointerAs
			( L"pIndexedList", (size_t) countIndex * sizeof(uint32_t)
								* GetPrimitiveVertexCount
									( (S3DPrimitiveType) typePrimitive ) ) ;

	LQT_RETURN_BOOL
		( pVB->AddPrimitiveBuffer
			( pMaterial, flags, (S3DPrimitiveType) typePrimitive,
				bufTemp, (size_t) countIndex, (size_t) countVertex ) == sglErrSuccess ) ;
}

// boolean freePrimitiveBuffer( const EntisGLS4.VertexBuffer.PrimitiveBuffer prmbuf )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_freePrimitiveBuffer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	S3DVertexBufferInterface *	pVB = pThis->GetRef<S3DVertexBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pVB ) ;
	LQT_FUNC_ARG_OBJECT( LEntisGLS4_VertexBuffer_PrimitiveBuffer, prmbuf ) ;
	LQT_VERIFY_NULL_PTR( prmbuf ) ;

	S3DVertexBufferInterface::PrimitiveBuffer	bufTemp ;
	bufTemp.pvVertex =
		(S3DVector4*) prmbuf->GetElementPointerAs( L"pvVertex", 0 ) ;
	bufTemp.pvNormal =
		(S3DVector4*) prmbuf->GetElementPointerAs( L"pvNormal", 0 ) ;
	bufTemp.pvUVMap =
		(S2DVector*) prmbuf->GetElementPointerAs( L"pvUVMap", 0 ) ;
	bufTemp.pColor =
		(S3DColor*) prmbuf->GetElementPointerAs( L"pColor", 0 ) ;
	bufTemp.pIndexedList =
		(uint32_t*) prmbuf->GetElementPointerAs( L"pIndexedList", 0 ) ;

	LQT_RETURN_BOOL( pVB->FreePrimitiveBuffer( bufTemp ) == sglErrSuccess ) ;
}

// ulong getMeshCount( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_getMeshCount)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	S3DVertexBufferInterface *	pVB = pThis->GetRef<S3DVertexBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pVB ) ;

	LQT_RETURN_ULONG( pVB->GetMeshCount() ) ;
}

// boolean getMeshInfoAt( EntisGLS4.VertexBuffer.MeshInfo info, ulong iMesh, ulong nCopyVertices, ulong iFirstVertex, uint flags ) const
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_getMeshInfoAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	S3DVertexBufferInterface *	pVB = pThis->GetRef<S3DVertexBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pVB ) ;
	LQT_FUNC_ARG_OBJECT( LEntisGLS4_VertexBuffer_MeshInfo, info ) ;
	LQT_VERIFY_NULL_PTR( info ) ;
	LQT_FUNC_ARG_ULONG( iMesh ) ;
	LQT_FUNC_ARG_ULONG( nCopyVertices ) ;
	LQT_FUNC_ARG_ULONG( iFirstVertex ) ;
	LQT_FUNC_ARG_UINT( flags ) ;

	S3DVertexBufferInterface::MeshInfo	meshInfo ;
	memset( &meshInfo, 0, sizeof(meshInfo) ) ;
	pVB->GetMeshInfoAt( meshInfo, (size_t) iMesh, 0, 0, flags ) ;

	meshInfo.pvVertex =
		(S3DVector4*) info->GetElementPointerAs
			( L"pvVertex", (size_t) nCopyVertices * sizeof(S3DVector4) ) ;
	meshInfo.pvNormal =
		(S3DVector4*) info->GetElementPointerAs
			( L"pvNormal", (size_t) nCopyVertices * sizeof(S3DVector4) ) ;
	meshInfo.pvUVMap =
		(S2DVector*) info->GetElementPointerAs
			( L"pvUVMap", (size_t) nCopyVertices * sizeof(S2DVector) ) ;
	meshInfo.pColor =
		(S3DColor*) info->GetElementPointerAs
			( L"pColor", (size_t) nCopyVertices * sizeof(S3DColor) ) ;
	meshInfo.pIndexedList =
		(uint32_t*) info->GetElementPointerAs
			( L"pIndexedList", meshInfo.countPrimitive
								* GetPrimitiveVertexCount( meshInfo.typeMesh ) ) ;
	meshInfo.pfpExAttrElements =
		(float32_t*) info->GetElementPointerAs
			( L"pfpExAttrElements",
				(size_t) nCopyVertices
						* meshInfo.nExAttrElements * sizeof(float32_t) ) ;

	LBoolean	valRet =
		(pVB->GetMeshInfoAt
			( meshInfo, (size_t) iMesh,
				(size_t) nCopyVertices,
				(size_t) iFirstVertex, flags ) == sglErrSuccess ) ;
	if ( valRet )
	{
		if ( meshInfo.pMaterial != nullptr )
		{
			LPtr<LNativeObj>	pMaterialObj( new LNativeObj( LQT_GET_CLASS(EntisGLS4.Material) ) ) ;
			pMaterialObj->SetNative( std::make_shared<LEntisGLS4_Material>(meshInfo.pMaterial) ) ;
			LObject::ReleaseRef( info->SetElementAs( L"pMaterial", pMaterialObj.Get() ) ) ;
		}
		else
		{
			LObject::ReleaseRef( info->SetElementAs( L"pMaterial", nullptr ) ) ;
		}
		info->SetElementLongAs( L"typeMesh", meshInfo.typeMesh ) ;
		info->SetElementLongAs( L"countPrimitive", meshInfo.countPrimitive ) ;
		info->SetElementLongAs( L"countVertex", meshInfo.countVertex ) ;

		S3DVector*	pCenter =
			(S3DVector*) info->GetElementPointerAs( L"vCenter", sizeof(LVector3) ) ;
		if ( pCenter != nullptr )
		{
			*pCenter = meshInfo.vCenter ;
		}
		info->SetElementDoubleAs( L"fpRadius", meshInfo.fpRadius ) ;
		info->SetElementLongAs( L"nExAttrElements", meshInfo.nExAttrElements ) ;
	}

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean updateIndexedPrimitive( ulong iMesh, uint flags, ulong countIndex, ulong countVertex, const Vector4* pvVertex, const Vector4* pvNormal, const Vector2* pvUVMap, const EntisGLS4.ColorMulAdd* pColor, const uint* pIndexedList )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_updateIndexedPrimitive)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	S3DVertexBufferInterface *	pVB = pThis->GetRef<S3DVertexBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pVB ) ;
	LQT_FUNC_ARG_ULONG( iMesh ) ;
	LQT_FUNC_ARG_UINT( flags ) ;
	LQT_FUNC_ARG_ULONG( countIndex ) ;
	LQT_FUNC_ARG_ULONG( countVertex ) ;
	LQT_FUNC_ARG_STRUCT_N( LVector4, pvVertex, countVertex ) ;
	LQT_FUNC_ARG_STRUCT_N( LVector4, pvNormal, countVertex ) ;
	LQT_FUNC_ARG_STRUCT_N( LVector2, pvUVMap, countVertex ) ;
	LQT_FUNC_ARG_STRUCT_N( LEntisGLS4_ColorMulAdd, pColor, countVertex ) ;
	LQT_FUNC_ARG_POINTER_N( LUint32, pIndexedList, countIndex ) ;

	LQT_RETURN_BOOL
		( pVB->UpdateIndexedPrimitiveList
			( (size_t) iMesh, flags, (size_t) countIndex, (size_t) countVertex,
				pvVertex, pvNormal, pvUVMap, pColor, pIndexedList ) == sglErrSuccess ) ;
}

// boolean setExtendVertexAttribute( ulong iMesh, ulong countElements, ulong countVertex, const float* pfpAttrElements )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_setExtendVertexAttribute)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	S3DVertexBufferInterface *	pVB = pThis->GetRef<S3DVertexBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pVB ) ;
	LQT_FUNC_ARG_ULONG( iMesh ) ;
	LQT_FUNC_ARG_ULONG( countElements ) ;
	LQT_FUNC_ARG_ULONG( countVertex ) ;
	LQT_FUNC_ARG_POINTER_N( LFloat, pfpAttrElements, countElements * countVertex ) ;
	LQT_VERIFY_NULL_PTR( pfpAttrElements ) ;

	LQT_RETURN_BOOL
		( pVB->SetExtendVertexAttribute
			( (size_t) iMesh, (size_t) countElements,
				(size_t) countVertex, pfpAttrElements ) == sglErrSuccess ) ;
}

// boolean setBoneWeightMap( ulong iMesh, ulong nCount, const float*[] ppWeightMaps )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_setBoneWeightMap)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	S3DVertexBufferInterface *	pVB = pThis->GetRef<S3DVertexBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pVB ) ;
	LQT_FUNC_ARG_ULONG( iMesh ) ;
	LQT_FUNC_ARG_ULONG( nCount ) ;
	LQT_FUNC_ARG_OBJECT( LArrayObj, ppWeightMaps ) ;
	LQT_VERIFY_NULL_PTR( ppWeightMaps ) ;

	SPointerArray<const float32_t>	aWeightMapsBuf ;
	const float32_t**	ppWeightMapsBuf = aWeightMapsBuf.GetArray( (size_t) nCount ) ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		ppWeightMapsBuf[i] = (const float32_t*) ppWeightMaps->GetElementPointerAt( i, 0 ) ;
	}

	LQT_RETURN_BOOL
		( pVB->SetBoneWeightMap
			( (size_t) iMesh, (size_t) nCount, ppWeightMapsBuf ) == sglErrSuccess ) ;
}

// boolean setBoneJointMap( ulong iMesh, ulong nBoneCount, ulong nJointCount, const uint*[] ppJointMaps )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_setBoneJointMap)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	S3DVertexBufferInterface *	pVB = pThis->GetRef<S3DVertexBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pVB ) ;
	LQT_FUNC_ARG_ULONG( iMesh ) ;
	LQT_FUNC_ARG_ULONG( nBoneCount ) ;
	LQT_FUNC_ARG_ULONG( nJointCount ) ;
	LQT_FUNC_ARG_OBJECT( LArrayObj, ppJointMaps ) ;
	LQT_VERIFY_NULL_PTR( ppJointMaps ) ;

	SPointerArray<const uint32_t>	aJointMapsBuf ;
	const uint32_t**	ppJointMapsBuf = aJointMapsBuf.GetArray( (size_t) nJointCount ) ;
	for ( size_t i = 0; i < nJointCount; i ++ )
	{
		ppJointMapsBuf[i] = (const uint32_t*) ppJointMaps->GetElementPointerAt( i, 0 ) ;
	}

	LQT_RETURN_BOOL
		( pVB->SetBoneJointMap
			( (size_t) iMesh, (size_t) nBoneCount,
				(size_t) nJointCount, ppJointMapsBuf ) == sglErrSuccess ) ;
}

// boolean allocateMorphing( ulong iMesh, ulong nCount )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_allocateMorphing)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	S3DVertexBufferInterface *	pVB = pThis->GetRef<S3DVertexBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pVB ) ;
	LQT_FUNC_ARG_ULONG( iMesh ) ;
	LQT_FUNC_ARG_ULONG( nCount ) ;

	LQT_RETURN_BOOL
		( pVB->AllocateMorphing( (size_t) iMesh, (size_t) nCount ) == sglErrSuccess ) ;
}

// boolean setMorphingTargetMesh( ulong iMesh, ulong iMorph, ulong countVertex, const Vector4* pvVertex, const Vector4* pvNormal, const Vector2* pvUVMap, const EntisGLS4.ColorMulAdd* pColor )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_setMorphingTargetMesh)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	S3DVertexBufferInterface *	pVB = pThis->GetRef<S3DVertexBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pVB ) ;
	LQT_FUNC_ARG_ULONG( iMesh ) ;
	LQT_FUNC_ARG_ULONG( iMorph ) ;
	LQT_FUNC_ARG_ULONG( countVertex ) ;
	LQT_FUNC_ARG_STRUCT_N( LVector4, pvVertex, countVertex ) ;
	LQT_VERIFY_NULL_PTR( pvVertex ) ;
	LQT_FUNC_ARG_STRUCT_N( LVector4, pvNormal, countVertex ) ;
	LQT_FUNC_ARG_STRUCT_N( LVector2, pvUVMap, countVertex ) ;
	LQT_FUNC_ARG_STRUCT_N( LEntisGLS4_ColorMulAdd, pColor, countVertex ) ;

	LQT_RETURN_BOOL
		( pVB->SetMorphingTargetMesh
			( (size_t) iMesh, (size_t) iMorph, (size_t) countVertex,
						pvVertex, pvNormal, pvUVMap, pColor ) == sglErrSuccess ) ;
}

// boolean setMorphingTargetWeight( ulong iMesh, ulong iMorph, ulong countVertex, const float* pfpWeight )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_setMorphingTargetWeight)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	S3DVertexBufferInterface *	pVB = pThis->GetRef<S3DVertexBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pVB ) ;
	LQT_FUNC_ARG_ULONG( iMesh ) ;
	LQT_FUNC_ARG_ULONG( iMorph ) ;
	LQT_FUNC_ARG_ULONG( countVertex ) ;
	LQT_FUNC_ARG_POINTER_N( LFloat, pfpWeight, countVertex ) ;
	LQT_VERIFY_NULL_PTR( pfpWeight ) ;

	LQT_RETURN_BOOL
		( pVB->SetMorphingTargetWeight
			( (size_t) iMesh, (size_t) iMorph,
				(size_t) countVertex, pfpWeight ) == sglErrSuccess ) ;
}

// EntisGLS4.VertexVariantBuffer createVariantBuffer( )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_createVariantBuffer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	S3DVertexBufferInterface *	pVB = pThis->GetRef<S3DVertexBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pVB ) ;

	S3DVertexVariantBuffer *	pVVB = pVB->CreateVariantBuffer() ;
	if ( pVVB == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.VertexVariantBuffer) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_VertexVariantBuffer>( new SSmartObject( pVVB ) ) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// boolean updateVertexVariant( EntisGLS4.VertexVariantBuffer vvb, ulong iFirst, long iEnd )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_updateVertexVariant)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	S3DVertexBufferInterface *	pVB = pThis->GetRef<S3DVertexBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pVB ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_VertexVariantBuffer, vvb ) ;
	LQT_VERIFY_NULL_PTR( vvb ) ;
	S3DVertexVariantBuffer *	pVVB = vvb->GetRef<S3DVertexVariantBuffer>() ;
	LQT_VERIFY_NULL_PTR( pVVB ) ;
	LQT_FUNC_ARG_ULONG( iFirst ) ;
	LQT_FUNC_ARG_LONG( iEnd ) ;

	LQT_RETURN_BOOL
		( pVB->UpdateVertexVariant
			( pVVB, (size_t) iFirst, (ssize_t) iEnd ) == sglErrSuccess ) ;
}

// EntisGLS4.VertexBuffer newReferenceVariantBuffer( )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_newReferenceVariantBuffer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	S3DVertexBufferInterface *	pVB = pThis->GetRef<S3DVertexBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pVB ) ;

	S3DVertexBufferInterface *	pVVB = pVB->NewReferenceVariantBuffer() ;
	if ( pVVB == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.VertexBuffer) ) ) ;
	valRet->SetNative
		( std::make_shared<LEntisGLS4_VertexBuffer>
			( new SSmartObject( (S3DRenderBufferInterface*) pVVB ) ) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// boolean renderBufferTo( EntisGLS4.RenderBuffer render, ulong flagsExclusion, ulong iFirst, long iEnd, ulong nInstancing, const Matrix4* pmatInstancing, const EntisGLS4.ColorMulAdd* pColorInstancing ) const
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_renderBufferTo)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	S3DVertexBufferInterface *	pVB = pThis->GetRef<S3DVertexBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pVB ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_RenderBuffer, render ) ;
	LQT_VERIFY_NULL_PTR( render ) ;
	S3DRenderContextInterface *	pRender = render->GetRef<S3DRenderContextInterface>() ;
	LQT_VERIFY_NULL_PTR( pRender ) ;
	LQT_FUNC_ARG_ULONG( flagsExclusion ) ;
	LQT_FUNC_ARG_ULONG( iFirst ) ;
	LQT_FUNC_ARG_LONG( iEnd ) ;
	LQT_FUNC_ARG_ULONG( nInstancing ) ;
	LQT_FUNC_ARG_STRUCT_N( LMatrix4, pmatInstancing, nInstancing ) ;
	LQT_FUNC_ARG_STRUCT_N( LEntisGLS4_ColorMulAdd, pColorInstancing, nInstancing ) ;

	LQT_RETURN_BOOL
		( pVB->RenderBufferTo
			( pRender, flagsExclusion,
				(size_t) iFirst, (ssize_t) iEnd,
				(size_t) nInstancing, pmatInstancing, pColorInstancing ) == sglErrSuccess ) ;
}

// boolean isModelIntoView( EntisGLS4.RenderContext render, double scaleMargin, double modelMargin )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_isModelIntoView)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	S3DVertexBufferInterface *	pVB = pThis->GetRef<S3DVertexBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pVB ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_RenderContext, render ) ;
	LQT_VERIFY_NULL_PTR( render ) ;
	S3DRenderContextInterface *	pRender = render->GetRef<S3DRenderContextInterface>() ;
	LQT_VERIFY_NULL_PTR( pRender ) ;
	LQT_FUNC_ARG_DOUBLE( scaleMargin ) ;
	LQT_FUNC_ARG_DOUBLE( modelMargin ) ;

	LQT_RETURN_BOOL
		( pVB->IsModelIntoView
			( pRender, (float32_t) scaleMargin, (float32_t) modelMargin ) ) ;
}

// void clearBuffer( )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_clearBuffer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	S3DVertexBufferInterface *	pVB = pThis->GetRef<S3DVertexBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pVB ) ;

	pVB->ClearBuffer() ;

	LQT_RETURN_VOID() ;
}

// void releaseAllDeviceResources( )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_releaseAllDeviceResources)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	S3DVertexBufferInterface *	pVB = pThis->GetRef<S3DVertexBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pVB ) ;

	pVB->ReleaseAllDeviceResources() ;

	LQT_RETURN_VOID() ;
}

// void setBufferUnitSize( ulong bytes )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_setBufferUnitSize)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	S3DVertexBufferInterface *	pVB = pThis->GetRef<S3DVertexBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pVB ) ;
	LQT_FUNC_ARG_ULONG( bytes ) ;

	pVB->SetBufferUnitSize( (size_t) bytes ) ;

	LQT_RETURN_VOID() ;
}

// double getCircumscribedSphere( Vector3* vCenter )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_getCircumscribedSphere)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	S3DVertexBufferInterface *	pVB = pThis->GetRef<S3DVertexBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pVB ) ;
	LQT_FUNC_ARG_STRUCT( LVector3, vCenter ) ;
	LQT_VERIFY_NULL_PTR( vCenter ) ;

	LQT_RETURN_DOUBLE( pVB->GetCircumscribedSphere( *vCenter ) ) ;
}

// boolean getCircumscribedParallelepiped( Vector3* vMin, Vector3* vMax )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_getCircumscribedParallelepiped)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	S3DVertexBufferInterface *	pVB = pThis->GetRef<S3DVertexBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pVB ) ;
	LQT_FUNC_ARG_STRUCT( LVector3, vMin ) ;
	LQT_VERIFY_NULL_PTR( vMin ) ;
	LQT_FUNC_ARG_STRUCT( LVector3, vMax ) ;
	LQT_VERIFY_NULL_PTR( vMax ) ;

	LQT_RETURN_BOOL( pVB->GetCircumscribedParallelepiped( *vMin, *vMax ) ) ;
}

// ulong countOfTotalPolygons( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_countOfTotalPolygons)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	S3DVertexBufferInterface *	pVB = pThis->GetRef<S3DVertexBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pVB ) ;

	LQT_RETURN_ULONG( pVB->CountOfTotalPolygons() ) ;
}

// ulong countOfTotalVertices( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_countOfTotalVertices)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	S3DVertexBufferInterface *	pVB = pThis->GetRef<S3DVertexBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pVB ) ;

	LQT_RETURN_ULONG( pVB->CountOfTotalVertices() ) ;
}

// boolean enableMultiInstancingMode( boolean enable )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_enableMultiInstancingMode)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	S3DVertexBufferInterface *	pVB = pThis->GetRef<S3DVertexBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pVB ) ;
	LQT_FUNC_ARG_BOOL( enable ) ;

	LQT_RETURN_BOOL( pVB->EnableMultiInstancingMode( enable ) == sglErrSuccess ) ;
}

// boolean isMultiInstancingMode( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_isMultiInstancingMode)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	S3DVertexBufferInterface *	pVB = pThis->GetRef<S3DVertexBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pVB ) ;

	LQT_RETURN_BOOL( pVB->IsMultiInstancingMode() ) ;
}

// ulong getInstancingCount( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_getInstancingCount)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	S3DVertexBufferInterface *	pVB = pThis->GetRef<S3DVertexBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pVB ) ;

	LQT_RETURN_ULONG( pVB->GetInstancingCount() ) ;
}

// ulong getInstancingEntries( EntisGLS4.VertexVariantBuffer[] vvbs, Matrix4* pmatInstance, EntisGLS4.ColorMulAdd* pcolorInstance, ulong iFirst, ulong nCount ) const
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_getInstancingEntries)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	S3DVertexBufferInterface *	pVB = pThis->GetRef<S3DVertexBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pVB ) ;
	LQT_FUNC_ARG_OBJECT( LArrayObj, vvbs ) ;
	LQT_VERIFY_NULL_PTR( vvbs ) ;
	LQT_FUNC_ARG_STRUCT_N( LMatrix4, pmatInstance, LQT_ARG_LONG(5) ) ;
	LQT_VERIFY_NULL_PTR( pmatInstance ) ;
	LQT_FUNC_ARG_STRUCT_N( LEntisGLS4_ColorMulAdd, pcolorInstance, LQT_ARG_LONG(5) ) ;
	LQT_VERIFY_NULL_PTR( pcolorInstance ) ;
	LQT_FUNC_ARG_ULONG( iFirst ) ;
	LQT_FUNC_ARG_ULONG( nCount ) ;

	SPointerArray<S3DVertexVariantBuffer>	bufVVBs ;
	S3DVertexVariantBuffer**	ppVVBs = bufVVBs.GetArray( (size_t) nCount ) ;

	LUint64	valRet =
		pVB->GetInstancingEntries
			( ppVVBs, pmatInstance, pcolorInstance, (size_t) iFirst, (size_t) nCount ) ;
	for ( size_t i = 0; i < valRet; i ++ )
	{
		LPtr<LNativeObj>	pVVBObj( new LNativeObj( LQT_GET_CLASS(EntisGLS4.VertexVariantBuffer) ) ) ;
		pVVBObj->SetNative( std::make_shared<LEntisGLS4_VertexVariantBuffer>( ppVVBs[i] ) ) ;
		LObject::ReleaseRef( vvbs->SetElementAt( i, pVVBObj.Get() ) ) ;
	}

	LQT_RETURN_ULONG( valRet ) ;
}

// boolean clearAllInstance( )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_clearAllInstance)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	S3DVertexBufferInterface *	pVB = pThis->GetRef<S3DVertexBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pVB ) ;

	LQT_RETURN_BOOL( pVB->ClearAllInstance() == sglErrSuccess ) ;
}

// boolean addInstanceVariant( EntisGLS4.VertexVariantBuffer vvb, const Matrix4* matInstance, const EntisGLS4.ColorMulAdd* colorInstance )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_addInstanceVariant)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	S3DVertexBufferInterface *	pVB = pThis->GetRef<S3DVertexBufferInterface>() ;
	LQT_VERIFY_NULL_PTR( pVB ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_VertexVariantBuffer, vvb ) ;
	LQT_VERIFY_NULL_PTR( vvb ) ;
	S3DVertexVariantBuffer *	pVVB = vvb->GetRef<S3DVertexVariantBuffer>() ;
	LQT_VERIFY_NULL_PTR( pVVB ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix4, matInstance ) ;
	LQT_VERIFY_NULL_PTR( matInstance ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_ColorMulAdd, colorInstance ) ;
	LQT_VERIFY_NULL_PTR( colorInstance ) ;

	LQT_RETURN_BOOL
		( pVB->AddInstanceVariant
			( pVVB, *matInstance, *colorInstance ) == sglErrSuccess ) ;
}



