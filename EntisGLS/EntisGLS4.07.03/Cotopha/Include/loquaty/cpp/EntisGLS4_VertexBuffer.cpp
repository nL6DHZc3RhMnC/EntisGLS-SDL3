
#include <loquaty.h>
#include "EntisGLS4_VertexBuffer.h"

using namespace Loquaty ;


// VertexBuffer( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_VertexBuffer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ( LEntisGLS4_VertexBuffer, pThis, () ) ;

	// pThis->Initialize() ;

	LQT_RETURN_VOID() ;
}

// EntisGLS4.VertexBuffer.BufferControlFlag getBufferControlFlags( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_getBufferControlFlags)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;

	LUint32	valRet ;
	// valRet = pThis->getBufferControlFlags(...) ;

	LQT_RETURN_UINT( valRet ) ;
}

// void setBufferControlFlags( EntisGLS4.VertexBuffer.BufferControlFlag nFlags )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_setBufferControlFlags)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	LQT_FUNC_ARG_UINT( nFlags ) ;

	// pThis->setBufferControlFlags(...) ;

	LQT_RETURN_VOID() ;
}

// EntisGLS4.Material getDefaultMaterial( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_getDefaultMaterial)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.Material) ) ) ;
	// valRet = pThis->getDefaultMaterial(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_Material> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_Material>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// void attachDefaultMaterial( EntisGLS4.Material material )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_attachDefaultMaterial)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Material, material ) ;
	LQT_VERIFY_NULL_PTR( material ) ;

	// pThis->attachDefaultMaterial(...) ;

	LQT_RETURN_VOID() ;
}

// boolean allocatePrimitiveBuffer( EntisGLS4.VertexBuffer.PrimitiveBuffer prmbuf, EntisGLS4.PrimitiveType typePrimitive, ulong countIndex, ulong countVertex )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_allocatePrimitiveBuffer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	LQT_FUNC_ARG_OBJECT( LEntisGLS4_VertexBuffer_PrimitiveBuffer, prmbuf ) ;
	LQT_VERIFY_NULL_PTR( prmbuf ) ;
	LQT_FUNC_ARG_INT( typePrimitive ) ;
	LQT_FUNC_ARG_ULONG( countIndex ) ;
	LQT_FUNC_ARG_ULONG( countVertex ) ;

	LBoolean	valRet ;
	// valRet = pThis->allocatePrimitiveBuffer(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean addPrimitiveBuffer( EntisGLS4.Material material, uint flags, EntisGLS4.PrimitiveType typePrimitive, const EntisGLS4.VertexBuffer.PrimitiveBuffer prmbuf, ulong countIndex, ulong countVertex )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_addPrimitiveBuffer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Material, material ) ;
	LQT_VERIFY_NULL_PTR( material ) ;
	LQT_FUNC_ARG_UINT( flags ) ;
	LQT_FUNC_ARG_INT( typePrimitive ) ;
	LQT_FUNC_ARG_OBJECT( LEntisGLS4_VertexBuffer_PrimitiveBuffer, prmbuf ) ;
	LQT_VERIFY_NULL_PTR( prmbuf ) ;
	LQT_FUNC_ARG_ULONG( countIndex ) ;
	LQT_FUNC_ARG_ULONG( countVertex ) ;

	LBoolean	valRet ;
	// valRet = pThis->addPrimitiveBuffer(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean freePrimitiveBuffer( const EntisGLS4.VertexBuffer.PrimitiveBuffer prmbuf )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_freePrimitiveBuffer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	LQT_FUNC_ARG_OBJECT( LEntisGLS4_VertexBuffer_PrimitiveBuffer, prmbuf ) ;
	LQT_VERIFY_NULL_PTR( prmbuf ) ;

	LBoolean	valRet ;
	// valRet = pThis->freePrimitiveBuffer(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// ulong getMeshCount( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_getMeshCount)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;

	LUint64	valRet ;
	// valRet = pThis->getMeshCount(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// boolean getMeshInfoAt( EntisGLS4.VertexBuffer.MeshInfo info, ulong iMesh, ulong nCopyVertices, ulong iFirstVertex, EntisGLS4.VertexBuffer.MeshInfoFlag flags ) const
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_getMeshInfoAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	LQT_FUNC_ARG_OBJECT( LEntisGLS4_VertexBuffer_MeshInfo, info ) ;
	LQT_VERIFY_NULL_PTR( info ) ;
	LQT_FUNC_ARG_ULONG( iMesh ) ;
	LQT_FUNC_ARG_ULONG( nCopyVertices ) ;
	LQT_FUNC_ARG_ULONG( iFirstVertex ) ;
	LQT_FUNC_ARG_UINT( flags ) ;

	LBoolean	valRet ;
	// valRet = pThis->getMeshInfoAt(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean updateIndexedPrimitive( ulong iMesh, uint flags, ulong countIndex, ulong countVertex, const Vector4* pvVertex, const Vector4* pvNormal, const Vector2* pvUVMap, const EntisGLS4.ColorMulAdd* pColor, const uint* pIndexedList )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_updateIndexedPrimitive)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	LQT_FUNC_ARG_ULONG( iMesh ) ;
	LQT_FUNC_ARG_UINT( flags ) ;
	LQT_FUNC_ARG_ULONG( countIndex ) ;
	LQT_FUNC_ARG_ULONG( countVertex ) ;
	LQT_FUNC_ARG_STRUCT( LVector4, pvVertex ) ;
	LQT_VERIFY_NULL_PTR( pvVertex ) ;
	LQT_FUNC_ARG_STRUCT( LVector4, pvNormal ) ;
	LQT_VERIFY_NULL_PTR( pvNormal ) ;
	LQT_FUNC_ARG_STRUCT( LVector2, pvUVMap ) ;
	LQT_VERIFY_NULL_PTR( pvUVMap ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_ColorMulAdd, pColor ) ;
	LQT_VERIFY_NULL_PTR( pColor ) ;
	LQT_FUNC_ARG_POINTER( LUint32, pIndexedList ) ;
	LQT_VERIFY_NULL_PTR( pIndexedList ) ;

	LBoolean	valRet ;
	// valRet = pThis->updateIndexedPrimitive(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean setExtendVertexAttribute( ulong iMesh, ulong countElements, ulong countVertex, const float* pfpAttrElements )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_setExtendVertexAttribute)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	LQT_FUNC_ARG_ULONG( iMesh ) ;
	LQT_FUNC_ARG_ULONG( countElements ) ;
	LQT_FUNC_ARG_ULONG( countVertex ) ;
	LQT_FUNC_ARG_POINTER( LFloat, pfpAttrElements ) ;
	LQT_VERIFY_NULL_PTR( pfpAttrElements ) ;

	LBoolean	valRet ;
	// valRet = pThis->setExtendVertexAttribute(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean setBoneWeightMap( ulong iMesh, ulong nCount, const float*[] ppWeightMaps )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_setBoneWeightMap)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	LQT_FUNC_ARG_ULONG( iMesh ) ;
	LQT_FUNC_ARG_ULONG( nCount ) ;
	LQT_FUNC_ARG_OBJECT( LArrayObj, ppWeightMaps ) ;
	LQT_VERIFY_NULL_PTR( ppWeightMaps ) ;

	LBoolean	valRet ;
	// valRet = pThis->setBoneWeightMap(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean setBoneJointMap( ulong iMesh, ulong nBoneCount, ulong nJointCount, const uint*[] ppJointMaps )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_setBoneJointMap)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	LQT_FUNC_ARG_ULONG( iMesh ) ;
	LQT_FUNC_ARG_ULONG( nBoneCount ) ;
	LQT_FUNC_ARG_ULONG( nJointCount ) ;
	LQT_FUNC_ARG_OBJECT( LArrayObj, ppJointMaps ) ;
	LQT_VERIFY_NULL_PTR( ppJointMaps ) ;

	LBoolean	valRet ;
	// valRet = pThis->setBoneJointMap(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean allocateMorphing( ulong iMesh, ulong nCount )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_allocateMorphing)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	LQT_FUNC_ARG_ULONG( iMesh ) ;
	LQT_FUNC_ARG_ULONG( nCount ) ;

	LBoolean	valRet ;
	// valRet = pThis->allocateMorphing(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean setMorphingTargetMesh( ulong iMesh, ulong iMorph, ulong countVertex, const Vector4* pvVertex, const Vector4* pvNormal, const Vector2* pvUVMap, const EntisGLS4.ColorMulAdd* pColor )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_setMorphingTargetMesh)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	LQT_FUNC_ARG_ULONG( iMesh ) ;
	LQT_FUNC_ARG_ULONG( iMorph ) ;
	LQT_FUNC_ARG_ULONG( countVertex ) ;
	LQT_FUNC_ARG_STRUCT( LVector4, pvVertex ) ;
	LQT_VERIFY_NULL_PTR( pvVertex ) ;
	LQT_FUNC_ARG_STRUCT( LVector4, pvNormal ) ;
	LQT_VERIFY_NULL_PTR( pvNormal ) ;
	LQT_FUNC_ARG_STRUCT( LVector2, pvUVMap ) ;
	LQT_VERIFY_NULL_PTR( pvUVMap ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_ColorMulAdd, pColor ) ;
	LQT_VERIFY_NULL_PTR( pColor ) ;

	LBoolean	valRet ;
	// valRet = pThis->setMorphingTargetMesh(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean setMorphingTargetWeight( ulong iMesh, ulong iMorph, ulong countVertex, const float* pfpWeight )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_setMorphingTargetWeight)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	LQT_FUNC_ARG_ULONG( iMesh ) ;
	LQT_FUNC_ARG_ULONG( iMorph ) ;
	LQT_FUNC_ARG_ULONG( countVertex ) ;
	LQT_FUNC_ARG_POINTER( LFloat, pfpWeight ) ;
	LQT_VERIFY_NULL_PTR( pfpWeight ) ;

	LBoolean	valRet ;
	// valRet = pThis->setMorphingTargetWeight(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// EntisGLS4.VertexVariantBuffer createVariantBuffer( )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_createVariantBuffer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.VertexVariantBuffer) ) ) ;
	// valRet = pThis->createVariantBuffer(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_VertexVariantBuffer> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_VertexVariantBuffer>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// boolean updateVertexVariant( EntisGLS4.VertexVariantBuffer vvb, ulong iFirst, long iEnd )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_updateVertexVariant)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_VertexVariantBuffer, vvb ) ;
	LQT_VERIFY_NULL_PTR( vvb ) ;
	LQT_FUNC_ARG_ULONG( iFirst ) ;
	LQT_FUNC_ARG_LONG( iEnd ) ;

	LBoolean	valRet ;
	// valRet = pThis->updateVertexVariant(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// EntisGLS4.VertexBuffer newReferenceVariantBuffer( )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_newReferenceVariantBuffer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.VertexBuffer) ) ) ;
	// valRet = pThis->newReferenceVariantBuffer(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_VertexBuffer> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_VertexBuffer>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// boolean renderBufferTo( EntisGLS4.RenderBuffer render, ulong flagsExclusion, ulong iFirst, long iEnd, ulong nInstancing, const Matrix4* pmatInstancing, const EntisGLS4.ColorMulAdd* pColorInstancing ) const
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_renderBufferTo)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_RenderBuffer, render ) ;
	LQT_VERIFY_NULL_PTR( render ) ;
	LQT_FUNC_ARG_ULONG( flagsExclusion ) ;
	LQT_FUNC_ARG_ULONG( iFirst ) ;
	LQT_FUNC_ARG_LONG( iEnd ) ;
	LQT_FUNC_ARG_ULONG( nInstancing ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix4, pmatInstancing ) ;
	LQT_VERIFY_NULL_PTR( pmatInstancing ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_ColorMulAdd, pColorInstancing ) ;
	LQT_VERIFY_NULL_PTR( pColorInstancing ) ;

	LBoolean	valRet ;
	// valRet = pThis->renderBufferTo(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean isModelIntoView( EntisGLS4.RenderContext render, double scaleMargin, double modelMargin )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_isModelIntoView)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_RenderContext, render ) ;
	LQT_VERIFY_NULL_PTR( render ) ;
	LQT_FUNC_ARG_DOUBLE( scaleMargin ) ;
	LQT_FUNC_ARG_DOUBLE( modelMargin ) ;

	LBoolean	valRet ;
	// valRet = pThis->isModelIntoView(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void clearBuffer( )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_clearBuffer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;

	// pThis->clearBuffer(...) ;

	LQT_RETURN_VOID() ;
}

// void releaseAllDeviceResources( )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_releaseAllDeviceResources)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;

	// pThis->releaseAllDeviceResources(...) ;

	LQT_RETURN_VOID() ;
}

// void setBufferUnitSize( ulong bytes )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_setBufferUnitSize)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	LQT_FUNC_ARG_ULONG( bytes ) ;

	// pThis->setBufferUnitSize(...) ;

	LQT_RETURN_VOID() ;
}

// double getCircumscribedSphere( Vector3* vCenter )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_getCircumscribedSphere)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LVector3, vCenter ) ;
	LQT_VERIFY_NULL_PTR( vCenter ) ;

	LDouble	valRet ;
	// valRet = pThis->getCircumscribedSphere(...) ;

	LQT_RETURN_DOUBLE( valRet ) ;
}

// boolean getCircumscribedParallelepiped( Vector3* vMin, Vector3* vMax )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_getCircumscribedParallelepiped)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LVector3, vMin ) ;
	LQT_VERIFY_NULL_PTR( vMin ) ;
	LQT_FUNC_ARG_STRUCT( LVector3, vMax ) ;
	LQT_VERIFY_NULL_PTR( vMax ) ;

	LBoolean	valRet ;
	// valRet = pThis->getCircumscribedParallelepiped(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// ulong countOfTotalPolygons( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_countOfTotalPolygons)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;

	LUint64	valRet ;
	// valRet = pThis->countOfTotalPolygons(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// ulong countOfTotalVertices( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_countOfTotalVertices)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;

	LUint64	valRet ;
	// valRet = pThis->countOfTotalVertices(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// boolean enableMultiInstancingMode( boolean enable )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_enableMultiInstancingMode)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	LQT_FUNC_ARG_BOOL( enable ) ;

	LBoolean	valRet ;
	// valRet = pThis->enableMultiInstancingMode(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean isMultiInstancingMode( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_isMultiInstancingMode)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->isMultiInstancingMode(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// ulong getInstancingCount( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_getInstancingCount)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;

	LUint64	valRet ;
	// valRet = pThis->getInstancingCount(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// ulong getInstancingEntries( EntisGLS4.VertexVariantBuffer[] vvbs, Matrix4* pmatInstance, EntisGLS4.ColorMulAdd* pcolorInstance, ulong iFirst, ulong nCount ) const
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_getInstancingEntries)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	LQT_FUNC_ARG_OBJECT( LArrayObj, vvbs ) ;
	LQT_VERIFY_NULL_PTR( vvbs ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix4, pmatInstance ) ;
	LQT_VERIFY_NULL_PTR( pmatInstance ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_ColorMulAdd, pcolorInstance ) ;
	LQT_VERIFY_NULL_PTR( pcolorInstance ) ;
	LQT_FUNC_ARG_ULONG( iFirst ) ;
	LQT_FUNC_ARG_ULONG( nCount ) ;

	LUint64	valRet ;
	// valRet = pThis->getInstancingEntries(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// boolean clearAllInstance( )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_clearAllInstance)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->clearAllInstance(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean addInstanceVariant( EntisGLS4.VertexVariantBuffer vvb, const Matrix4* matInstance, const EntisGLS4.ColorMulAdd* colorInstance )
IMPL_LOQUATY_FUNC(EntisGLS4_VertexBuffer_addInstanceVariant)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_VertexBuffer, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_VertexVariantBuffer, vvb ) ;
	LQT_VERIFY_NULL_PTR( vvb ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix4, matInstance ) ;
	LQT_VERIFY_NULL_PTR( matInstance ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_ColorMulAdd, colorInstance ) ;
	LQT_VERIFY_NULL_PTR( colorInstance ) ;

	LBoolean	valRet ;
	// valRet = pThis->addInstanceVariant(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}



