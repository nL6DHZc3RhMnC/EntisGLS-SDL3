
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/


#include <sakuraglx/sakuraglx.h>
#include <sakura/ssys_module.h>
#include <sakuragl/sgl_window.h>
#include <glscs/glscs_sakura2_obj_render.h>
#include <sakuragl/sgl3d/sgl_hybrid_renderer.h>
#include <sakuraglx/sakuraglx.h>

using	namespace SSystem ;
using	namespace SakuraGL ;
using	namespace ECSSakura2 ;
using	namespace ECSSakura2Processor ;


//////////////////////////////////////////////////////////////////////////////
// Material オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2( ECSSakura2::MaterialObject, Object, S3DMaterial ) ;

// 実行時型名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * MaterialObject::GetTypeName( void ) const
{
	return	L"SakuraGL::Material" ;
}

#if	!defined(ENTISGLS4_DLL_IMPORT)

// new SakuraGL::Material
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_NEW_OBJECT(SakuraGL_Material,context,cls_id)
{
	return	new MaterialObject ;
}

// void Material::GetSurfaceAttribute( S3DSurfaceAttribute& attr ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_Material_GetSurfaceAttribute,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DMaterial, pMaterial,
					arg, Material::GetSurfaceAttribute ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, S3DSurfaceAttribute, pSufAttr,
					arg[1].i, Material::GetSurfaceAttribute ) ;
	//
	pMaterial->GetSurfaceAttribute( *pSufAttr ) ;
	//
	return	NULL ;
}

// void Material::GetBackSurfaceAttribute( S3DSurfaceAttribute& attr ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_Material_GetBackSurfaceAttribute,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DMaterial, pMaterial,
					arg, Material::GetBackSurfaceAttribute ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, S3DSurfaceAttribute, pSufAttr,
					arg[1].i, Material::GetBackSurfaceAttribute ) ;
	//
	pMaterial->GetBackSurfaceAttribute( *pSufAttr ) ;
	//
	return	NULL ;
}

// bool Material::IsEnabledBackSurfaceAttribute( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_Material_IsEnabledBackSurfaceAttribute,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DMaterial, pMaterial,
					arg, Material::IsEnabledBackSurfaceAttribute ) ;
	//
	context->m_regset[regAcc].i =
		pMaterial->IsEnabledBackSurfaceAttribute() ? -1 : 0 ;
	//
	return	NULL ;
}

// void Material::SetSurfaceAttribute( const S3DSurfaceAttribute& attr ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_Material_SetSurfaceAttribute,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DMaterial, pMaterial,
					arg, Material::SetSurfaceAttribute ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, const S3DSurfaceAttribute, pSufAttr,
					arg[1].i, Material::SetSurfaceAttribute ) ;
	//
	pMaterial->SetSurfaceAttribute( *pSufAttr ) ;
	//
	return	NULL ;
}

// void Material::SetBackSurfaceAttribute( const S3DSurfaceAttribute& attr ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_Material_SetBackSurfaceAttribute,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DMaterial, pMaterial,
					arg, Material::SetBackSurfaceAttribute ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, const S3DSurfaceAttribute, pSufAttr,
					arg[1].i, Material::SetBackSurfaceAttribute ) ;
	//
	pMaterial->SetBackSurfaceAttribute( *pSufAttr ) ;
	//
	return	NULL ;
}

// void Material::EnableBackSurfaceAttribute( bool flagBack ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_Material_EnableBackSurfaceAttribute,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DMaterial, pMaterial,
					arg, Material::EnableBackSurfaceAttribute ) ;
	//
	pMaterial->EnableBackSurfaceAttribute( arg[1].i != 0 ) ;
	//
	return	NULL ;
}

// Image * Material::GetTexture( int iTexture = 0 ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_Material_GetTexture,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DMaterial, pMaterial,
					arg, Material::GetTexture ) ;
	Object *	pImage =
		ESLTypeCast<Object>( pMaterial->GetTexture( (int) arg[1].i ) ) ;
	if ( pImage != NULL )
	{
		context->m_regset[regAcc].h32 = pImage->m_dwHighAddr ;
		context->m_regset[regAcc].l32 = 0 ;
	}
	else
	{
		context->m_regset[regAcc].i = 0 ;
	}
	return	NULL ;
}

// Image * Material::GetBackTexture( int iTexture = 0 ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_Material_GetBackTexture,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DMaterial, pMaterial,
					arg, Material::GetBackTexture ) ;
	Object *	pImage =
		ESLTypeCast<Object>( pMaterial->GetBackTexture( (int) arg[1].i ) ) ;
	if ( pImage != NULL )
	{
		context->m_regset[regAcc].h32 = pImage->m_dwHighAddr ;
		context->m_regset[regAcc].l32 = 0 ;
	}
	else
	{
		context->m_regset[regAcc].i = 0 ;
	}
	return	NULL ;
}

// void Material::SetTexture
//	( Image * pImage, int iTexture = 0,
//		uint32_t nFlags = 0, double nApply = 1.0, double nParam1 = 0.0 ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_Material_SetTexture,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DMaterial, pMaterial,
					arg, Material::SetTexture ) ;
	SGLImageObject *	pImage =
		ESLTypeCast<SGLImageObject>( vm->ObjectFromAddress( arg[1].h32 ) ) ;
	//
	pMaterial->SetTexture
		( pImage, (int) arg[2].i,
			(uint32_t) arg[3].i, (float32_t) arg[4].f, (float32_t) arg[5].f ) ;
	//
	return	NULL ;
}

// void Material::SetBackTexture
//	( Image * pImage, int iTexture = 0,
//		uint32_t nFlags = 0, double nApply = 1.0, double nParam1 = 0.0 ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_Material_SetBackTexture,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DMaterial, pMaterial,
					arg, Material::SetBackTexture ) ;
	SGLImageObject *	pImage =
		ESLTypeCast<SGLImageObject>( vm->ObjectFromAddress( arg[1].h32 ) ) ;
	//
	pMaterial->SetBackTexture
		( pImage, (int) arg[2].i,
			(uint32_t) arg[3].i, (float32_t) arg[4].f, (float32_t) arg[5].f ) ;
	//
	return	NULL ;
}

// void Material::SetSubTextureZ( double zTexture ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_Material_SetSubTextureZ,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DMaterial, pMaterial,
					arg, Material::SetBackTexture ) ;
	//
	pMaterial->SetSubTextureZ( arg[1].f ) ;
	//
	return	NULL ;
}

#endif


//////////////////////////////////////////////////////////////////////////////
// VertexBuffer オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2( ECSSakura2::VertexBufferObject, Object, S3DVertexBuffer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
VertexBufferObject::VertexBufferObject( void )
{
}

VertexBufferObject::VertexBufferObject( VertexBuffer * buffer )
	: S3DVertexBuffer( buffer, true )
{
}

// 実行時型名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * VertexBufferObject::GetTypeName( void ) const
{
	return	L"SakuraGL::VertexBuffer" ;
}

#if	!defined(ENTISGLS4_DLL_IMPORT)

// new SakuraGL::VertexBuffer
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_NEW_OBJECT(SakuraGL_VertexBuffer, context, cls_id)
{
	return	new VertexBufferObject ;
}

// static VertexBuffer *
//	VertexBuffer::NewBuffer( SGLPaintContextType type = typePaintDefault ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_VertexBuffer_NewBuffer, context, arg)
{
	const SGLPaintContextType
			type = (SGLPaintContextType) arg[0].i ;
	VertexBufferObject *
			pObj = new VertexBufferObject
						( S3DVertexBufferInterface::NewBuffer(type) ) ;
	ECS_DECLARE_SYSCALL_VM( context, vm ) ;
	AssertLock() ;
	context->m_regset[regAcc].i = vm->AllocateHeapObjectAddress( pObj ) ;
	AssertUnlock() ;
	return	NULL ;
}
// SGLError VertexBuffer::AppendMatrixTransformation
//	( const S3DDMatrix& mat, const S3DDVector& pos,
//		const S3DColor * color = NULL, unsigned int nTransparency = 0 ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_VertexBuffer_AddMatrixTransformation,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DVertexBufferInterface,
					pBuffer, arg, VertexBuffer::AppendMatrixTransformation ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, S3DDMatrix, pMatrix,
					arg[1].i, VertexBuffer::AppendMatrixTransformation ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, S3DDVector, pPos,
					arg[2].i, VertexBuffer::AppendMatrixTransformation ) ;
	const S3DColor *	pColor =
		(const S3DColor*)
			context->AtomicTranslateAddress( arg[3].i, sizeof(S3DColor) ) ;
	const unsigned int	nTransparency = (unsigned int) arg[4].i ;
	//
	context->m_regset[regAcc].i =
		pBuffer->AppendMatrixTransformation
			( *pMatrix, *pPos, pColor, nTransparency ) ;
	//
	return	NULL ;
}

// SGLError VertexBuffer::SetMatrixTransformation
//	( const S3DDMatrix & mat, const S3DDVector& pos,
//		const S3DColor * color = NULL, unsigned int nTransparency = 0 ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_VertexBuffer_SetMatrixTransformation,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DVertexBufferInterface,
					pBuffer, arg, VertexBuffer::SetMatrixTransformation ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, S3DDMatrix, pMatrix,
					arg[1].i, VertexBuffer::SetMatrixTransformation ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, S3DDVector, pPos,
					arg[2].i, VertexBuffer::SetMatrixTransformation ) ;
	const S3DColor *	pColor =
		(const S3DColor*)
			context->AtomicTranslateAddress( arg[3].i, sizeof(S3DColor) ) ;
	const unsigned int	nTransparency = (unsigned int) arg[4].i ;
	//
	context->m_regset[regAcc].i =
		pBuffer->SetMatrixTransformation
			( *pMatrix, *pPos, pColor, nTransparency ) ;
	//
	return	NULL ;
}

// SGLError VertexBuffer::GetMatrixTransformation
//	( S3DDMatrix& mat, S3DDVector& pos,
//		S3DColor * color = NULL, unsigned int * pTransparency = 0 ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_VertexBuffer_GetMatrixTransformation,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DVertexBufferInterface,
					pBuffer, arg, VertexBuffer::GetMatrixTransformation ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, S3DDMatrix, pMatrix,
					arg[1].i, VertexBuffer::GetMatrixTransformation ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, S3DDVector, pPos,
					arg[2].i, VertexBuffer::GetMatrixTransformation ) ;
	S3DColor *	pColor =
		(S3DColor*)
			context->AtomicTranslateAddress( arg[3].i, sizeof(S3DColor) ) ;
	unsigned int *	pTransparency =
		(unsigned int*)
			context->AtomicTranslateAddress( arg[4].i, sizeof(unsigned int) ) ;
	//
	context->m_regset[regAcc].i =
		pBuffer->GetMatrixTransformation
			( *pMatrix, *pPos, pColor, pTransparency ) ;
	//
	return	NULL ;
}

// SGLError VertexBuffer::PushTransformation( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_VertexBuffer_PushTransformation,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DVertexBufferInterface,
					pBuffer, arg, PaintContext::PushTransformation ) ;
	//
	context->m_regset[regAcc].i = pBuffer->PushTransformation() ;
	//
	return	NULL ;
}

// SGLError VertexBuffer::PopTransformation( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_VertexBuffer_PopTransformation,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DVertexBufferInterface,
					pBuffer, arg, PaintContext::PopTransformation ) ;
	//
	context->m_regset[regAcc].i = pBuffer->PopTransformation() ;
	//
	return	NULL ;
}

// SGLError VertexBuffer::ResetTransformation( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_VertexBuffer_ResetTransformation,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DVertexBufferInterface,
					pBuffer, arg, PaintContext::ResetTransformation ) ;
	//
	context->m_regset[regAcc].i = pBuffer->ResetTransformation() ;
	//
	return	NULL ;
}

// SGLError VertexBuffer::AddIndexedTriangleList
//	( Material * pMaterial, uint32_t nFlags,
//		size_t countPolygon, size_t countVertex,
//		const S3DVector4 * pvVertex, const S3DVector4 * pvNormal,
//		const S2DVector * pvUVMap, const S3DColor * pColor,
//		const uint32_t * pIndexedList ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_VertexBuffer_AddIndexedTriangleList, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DVertexBufferInterface,
					pBuffer, arg, VertexBuffer::AddIndexedTriangleList ) ;
	ECS_DECLARE_SYSCALL_OBJECT
		( vm, S3DMaterial, pMaterial,
					arg[1].i, VertexBuffer::AddIndexedTriangleList ) ;
	const S3DVector4 *	pvVertex =
		(const S3DVector4*) context->AtomicTranslateAddress( arg[5].i ) ;
	const S3DVector4 *	pvNormal =
		(const S3DVector4*) context->AtomicTranslateAddress( arg[6].i ) ;
	const S2DVector *	pvUVMap =
		(const S2DVector*) context->AtomicTranslateAddress( arg[7].i ) ;
	const S3DColor *	pColor =
		(const S3DColor*) context->AtomicTranslateAddress( arg[8].i ) ;
	const uint32_t *	pIndexedList =
		(const uint32_t*) context->AtomicTranslateAddress( arg[9].i ) ;
	//
	context->m_regset[regAcc].i =
		pBuffer->AddIndexedTriangleList
			( pMaterial, (uint32_t) arg[2].i,
				(size_t) arg[3].i, (size_t) arg[4].i,
				pvVertex, pvNormal, pvUVMap, pColor, pIndexedList ) ;
	//
	return	NULL ;
}

// SGLError VertexBuffer::AddTriangleStrip
//	( Material * pMaterial, uint32_t nFlags, size_t countTriangleStrip,
//		const S3DVector4 * pvVertex, const S3DVector4 * pvNormal,
//		const S2DVector * pvUVMap, const S3DColor * pColor ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_VertexBuffer_AddTriangleStrip, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DVertexBufferInterface,
					pBuffer, arg, VertexBuffer::AddTriangleStrip ) ;
	ECS_DECLARE_SYSCALL_OBJECT
		( vm, S3DMaterial, pMaterial,
					arg[1].i, VertexBuffer::AddTriangleStrip ) ;
	const S3DVector4 *	pvVertex =
		(const S3DVector4*) context->AtomicTranslateAddress( arg[4].i ) ;
	const S3DVector4 *	pvNormal =
		(const S3DVector4*) context->AtomicTranslateAddress( arg[5].i ) ;
	const S2DVector *	pvUVMap =
		(const S2DVector*) context->AtomicTranslateAddress( arg[6].i ) ;
	const S3DColor *	pColor =
		(const S3DColor*) context->AtomicTranslateAddress( arg[7].i ) ;
	//
	context->m_regset[regAcc].i =
		pBuffer->AddTriangleStrip
			( pMaterial, (uint32_t) arg[2].i, (size_t) arg[3].i,
				pvVertex, pvNormal, pvUVMap, pColor ) ;
	//
	return	NULL ;
}

// SGLError AddIndexedPrimitiveList
//	( S3DMaterial * pMaterial, uint32_t nFlags,
//		S3DPrimitiveType typePrimitive,
//		size_t countIndex, size_t countVertex,
//		const S3DVector4 * pvVertex, const S3DVector4 * pvNormal,
//		const S2DVector * pvUVMap, const S3DColor * pColor,
//		const uint32_t * pIndexedList ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_VertexBuffer_AddIndexedPrimitiveList, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DVertexBufferInterface,
					pBuffer, arg, VertexBuffer::AddIndexedPrimitiveList ) ;
	ECS_DECLARE_SYSCALL_OBJECT
		( vm, S3DMaterial, pMaterial,
					arg[1].i, VertexBuffer::AddIndexedPrimitiveList ) ;
	const S3DVector4 *	pvVertex =
		(const S3DVector4*) context->AtomicTranslateAddress( arg[6].i ) ;
	const S3DVector4 *	pvNormal =
		(const S3DVector4*) context->AtomicTranslateAddress( arg[7].i ) ;
	const S2DVector *	pvUVMap =
		(const S2DVector*) context->AtomicTranslateAddress( arg[8].i ) ;
	const S3DColor *	pColor =
		(const S3DColor*) context->AtomicTranslateAddress( arg[9].i ) ;
	const uint32_t *	pIndexedList =
		(const uint32_t*) context->AtomicTranslateAddress( arg[10].i ) ;
	//
	context->m_regset[regAcc].i =
		pBuffer->AddIndexedPrimitiveList
			( pMaterial, (uint32_t) arg[2].i,
				(S3DPrimitiveType) arg[3].i,
				(size_t) arg[4].i, (size_t) arg[5].i,
				pvVertex, pvNormal, pvUVMap, pColor, pIndexedList ) ;
	//
	return	NULL ;
}

// SGLError VertexBuffer::AddVertexBuffer
//	( Material * pMaterial, uint32_t nFlags,
//		VertexBuffer * pBuffer, size_t iFirst = 0, ssize_t iEnd = -1 ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_VertexBuffer_AddVertexBuffer,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DVertexBufferInterface,
					pThisBuffer, arg, VertexBuffer::AddVertexBuffer ) ;
	ECS_DECLARE_SYSCALL_OBJECT
		( vm, S3DMaterial, pMaterial,
					arg[1].i, VertexBuffer::AddVertexBuffer ) ;
	ECS_DECLARE_SYSCALL_OBJECT
		( vm, S3DVertexBufferInterface, pSrcBuffer,
					arg[3].i, VertexBuffer::AddVertexBuffer ) ;
	//
	context->m_regset[regAcc].i =
		pThisBuffer->AddVertexBuffer
			( pMaterial, (uint32_t) arg[2].i,
				pSrcBuffer, (size_t) arg[4].i, (ssize_t) arg[5].i ) ;
	//
	return	NULL ;
}
// SGLError VertexBuffer::Flush( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_VertexBuffer_Flush, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DVertexBufferInterface,
					pBuffer, arg, VertexBuffer::Flush ) ;
	//
	context->m_regset[regAcc].i = pBuffer->Flush() ;
	//
	return	NULL ;
}

// size_t VertexBuffer::GetMeshCount( void ) const ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_VertexBuffer_GetMeshCount, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DVertexBufferInterface,
					pBuffer, arg, VertexBuffer::GetMeshCount ) ;
	//
	context->m_regset[regAcc].i = pBuffer->GetMeshCount() ;
	//
	return	NULL ;
}

// SGLError VertexBuffer::GetMeshInfoAt
//	( MeshInfo& info, size_t iMesh, size_t nCopyVertices,
//		size_t iFirstVertex = 0, uint32_t nFlags = 0 ) const ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_VertexBuffer_GetMeshInfoAt, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DVertexBufferInterface,
					pBuffer, arg, VertexBuffer::GetMeshInfoAt ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, VertexBufferObject::MESH_INFO, pInfo,
					arg[1].i, VertexBuffer::GetMeshInfoAt ) ;
	//
	VertexBuffer::MeshInfo	mi ;
	mi.pvVertex =
		(S3DVector4*) context->AtomicTranslateAddress( pInfo->pvVertex ) ;
	mi.pvNormal =
		(S3DVector4*) context->AtomicTranslateAddress( pInfo->pvNormal ) ;
	mi.pvUVMap =
		(S2DVector*) context->AtomicTranslateAddress( pInfo->pvUVMap ) ;
	mi.pColor =
		(S3DColor*) context->AtomicTranslateAddress( pInfo->pColor ) ;
	mi.pIndexedList =
		(uint32_t*) context->AtomicTranslateAddress( pInfo->pIndexedList ) ;
	mi.pfpExAttrElements =
		(float32_t*) context->AtomicTranslateAddress( pInfo->pfpExAttrElements ) ;
	//
	for ( int i = 0; i < S3DVertexBufferInterface::countSubMesh; i ++ )
	{
		mi.pSubIndexedList[i] =
			(uint32_t*) context->AtomicTranslateAddress( pInfo->pSubIndexedList[i] ) ;
	}
	//
	SGLError	err =
		pBuffer->GetMeshInfoAt
			( mi, (size_t) arg[2].i,
				(size_t) arg[3].i, (size_t) arg[4].i, (uint32_t) arg[5].i ) ;
	if ( !err )
	{
		pInfo->pMaterial = 0 ;
		pInfo->typeMesh = mi.typeMesh ;
		pInfo->countPrimitive = mi.countPrimitive ;
		pInfo->countVertex = mi.countVertex ;
		pInfo->vCenter = mi.vCenter ;
		pInfo->fpRadius = mi.fpRadius ;
		pInfo->fpSubMeshDensity = mi.fpSubMeshDensity ;
		pInfo->nExAttrElements = (uint32_t) mi.nExAttrElements ;
		//
		if ( mi.pvVertex == NULL )
		{
			pInfo->pvVertex = 0 ;
		}
		if ( mi.pvNormal == NULL )
		{
			pInfo->pvNormal = 0 ;
		}
		if ( mi.pvUVMap == NULL )
		{
			pInfo->pvUVMap = 0 ;
		}
		if ( mi.pColor == NULL )
		{
			pInfo->pColor = 0 ;
		}
		if ( mi.pIndexedList == NULL )
		{
			pInfo->pIndexedList = 0 ;
		}
		if ( mi.pfpExAttrElements == NULL )
		{
			pInfo->pfpExAttrElements = 0 ;
		}
		for ( int i = 0; i < S3DVertexBufferInterface::countSubMesh; i ++ )
		{
			if ( mi.pSubIndexedList[i] == NULL )
			{
				pInfo->pSubIndexedList[i] = 0 ;
			}
			pInfo->nSubPolyCount[i] = mi.nSubPolyCount[i] ;
		}
	}
	context->m_regset[regAcc].i = err ;
	//
	return	NULL ;
}

// SGLError VertexBuffer::UpdateIndexedTriangleList
//	( size_t iMesh, uint32_t nFlags,
//		size_t countPolygon, size_t countVertex,
//		const S3DVector4 * pvVertex, const S3DVector4 * pvNormal,
//		const S2DVector * pvUVMap, const S3DColor * pColor,
//		const uint32_t * pIndexedList ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_VertexBuffer_UpdateIndexedTriangleList, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DVertexBufferInterface,
					pBuffer, arg, VertexBuffer::UpdateIndexedTriangleList ) ;
	const S3DVector4 *	pvVertex =
		(const S3DVector4*) context->AtomicTranslateAddress( arg[5].i ) ;
	const S3DVector4 *	pvNormal =
		(const S3DVector4*) context->AtomicTranslateAddress( arg[6].i ) ;
	const S2DVector *	pvUVMap =
		(const S2DVector*) context->AtomicTranslateAddress( arg[7].i ) ;
	const S3DColor *	pColor =
		(const S3DColor*) context->AtomicTranslateAddress( arg[8].i ) ;
	const uint32_t *	pIndexedList =
		(const uint32_t*) context->AtomicTranslateAddress( arg[9].i ) ;
	//
	context->m_regset[regAcc].i =
		pBuffer->UpdateIndexedTriangleList
			( (size_t) arg[1].i, (uint32_t) arg[2].i,
				(size_t) arg[3].i, (size_t) arg[4].i,
				pvVertex, pvNormal, pvUVMap, pColor, pIndexedList ) ;
	//
	return	NULL ;
}

// SGLError VertexBuffer::UpdateTriangleStrip
//	( size_t iMesh, uint32_t nFlags, size_t countTriangleStrip,
//		const S3DVector4 * pvVertex, const S3DVector4 * pvNormal,
//		const S2DVector * pvUVMap, const S3DColor * pColor ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_VertexBuffer_UpdateTriangleStrip, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DVertexBufferInterface,
					pBuffer, arg, VertexBuffer::UpdateTriangleStrip ) ;
	const S3DVector4 *	pvVertex =
		(const S3DVector4*) context->AtomicTranslateAddress( arg[4].i ) ;
	const S3DVector4 *	pvNormal =
		(const S3DVector4*) context->AtomicTranslateAddress( arg[5].i ) ;
	const S2DVector *	pvUVMap =
		(const S2DVector*) context->AtomicTranslateAddress( arg[6].i ) ;
	const S3DColor *	pColor =
		(const S3DColor*) context->AtomicTranslateAddress( arg[7].i ) ;
	//
	context->m_regset[regAcc].i =
		pBuffer->UpdateTriangleStrip
			( (size_t) arg[1].i, (uint32_t) arg[2].i, (size_t) arg[3].i,
				pvVertex, pvNormal, pvUVMap, pColor ) ;
	//
	return	NULL ;
}

// SGLError VertexBuffer::UpdateIndexedPrimitiveList
//	( size_t iMesh, uint32_t nFlags,
//		size_t countIndex, size_t countVertex,
//		const S3DVector4 * pvVertex, const S3DVector4 * pvNormal,
//		const S2DVector * pvUVMap, const S3DColor * pColor,
//		const uint32_t * pIndexedList ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_VertexBuffer_UpdateIndexedPrimitiveList, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DVertexBufferInterface,
					pBuffer, arg, VertexBuffer::UpdateIndexedTriangleList ) ;
	const S3DVector4 *	pvVertex =
		(const S3DVector4*) context->AtomicTranslateAddress( arg[5].i ) ;
	const S3DVector4 *	pvNormal =
		(const S3DVector4*) context->AtomicTranslateAddress( arg[6].i ) ;
	const S2DVector *	pvUVMap =
		(const S2DVector*) context->AtomicTranslateAddress( arg[7].i ) ;
	const S3DColor *	pColor =
		(const S3DColor*) context->AtomicTranslateAddress( arg[8].i ) ;
	const uint32_t *	pIndexedList =
		(const uint32_t*) context->AtomicTranslateAddress( arg[9].i ) ;
	//
	context->m_regset[regAcc].i =
		pBuffer->UpdateIndexedPrimitiveList
			( (size_t) arg[1].i, (uint32_t) arg[2].i,
				(size_t) arg[3].i, (size_t) arg[4].i,
				pvVertex, pvNormal, pvUVMap, pColor, pIndexedList ) ;
	//
	return	NULL ;
}

// SGLError VertexBuffer::UpdateSubIndexedTriangleList
//	( size_t iMesh, size_t iSubMesh, uint32_t nFlags,
//		size_t countPolygon, const uint32_t * pIndexedList ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_VertexBuffer_UpdateSubIndexedTriangleList, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DVertexBufferInterface,
					pBuffer, arg, VertexBuffer::UpdateSubIndexedTriangleList ) ;
	const uint32_t *	pIndexedList =
		(const uint32_t*) context->AtomicTranslateAddress( arg[9].i ) ;
	//
	context->m_regset[regAcc].i =
		pBuffer->UpdateSubIndexedTriangleList
			( (size_t) arg[1].i, (size_t) arg[2].i,
				(uint32_t) arg[3].i, (size_t) arg[4].i, pIndexedList ) ;
	//
	return	NULL ;
}

// SGLError VertexBuffer::SetSubMeshDensity
//	( size_t iMesh, float32_t fpDensity, ssize_t iSelector ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_VertexBuffer_SetSubMeshDensity, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DVertexBufferInterface,
					pBuffer, arg, VertexBuffer::SetSubMeshDensity ) ;
	//
	context->m_regset[regAcc].i =
		pBuffer->SetSubMeshDensity
			( (size_t) arg[1].i, (float32_t) arg[2].f, (ssize_t) arg[3].i ) ;
	//
	return	NULL ;
}

// SGLError VertexBuffer::SetExtendVertexAttribute
//	( size_t iMesh, size_t countElements,
//			size_t countVertex, const float32_t * pfpAttrElements ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_VertexBuffer_SetExtendVertexAttribute, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DVertexBufferInterface,
					pBuffer, arg, VertexBuffer::SetSubMeshDensity ) ;
	const float32_t *	pfpAttrElements =
		(const float32_t*) context->AtomicTranslateAddress( arg[4].i ) ;
	if ( pfpAttrElements == NULL )
	{
		context->m_regset[regAcc].i = sglErrFailed ;
		return	NULL ;
	}
	//
	context->m_regset[regAcc].i =
		pBuffer->SetExtendVertexAttribute
			( (size_t) arg[1].i, (size_t) arg[2].i,
					(size_t) arg[3].i, pfpAttrElements ) ;
	//
	return	NULL ;
}

// SGLError VertexBuffer::SetBoneWeightMap
//	( size_t iMesh, size_t nCount, const float32_t ** ppWeightMaps ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_VertexBuffer_SetBoneWeightMap, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DVertexBufferInterface,
					pBuffer, arg, VertexBuffer::SetBoneWeightMap ) ;
	const uint64_t *	ppWeightMaps =
		(const uint64_t*) context->AtomicTranslateAddress( arg[3].i ) ;
	if ( ppWeightMaps == NULL )
	{
		context->m_regset[regAcc].i = sglErrFailed ;
		return	NULL ;
	}
	SPointerArray<float32_t>	bufWeightMaps ;
	size_t						nCount = (size_t) arg[2].i ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		bufWeightMaps.Add
			( (float32_t*)
				context->AtomicTranslateAddress( ppWeightMaps[i] ) ) ;
	}
	context->m_regset[regAcc].i =
		pBuffer->SetBoneWeightMap
			( (size_t) arg[1].i, nCount,
				(const float32_t**) bufWeightMaps.GetConstArray() ) ;
	//
	return	NULL ;
}

// SGLError VertexBuffer::SetBoneMatrix
//	( size_t iMesh, size_t nCount,
//		const S3DMatrix * pMatrix, const S3DVector * pTrans ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_VertexBuffer_SetBoneMatrix, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DVertexBufferInterface,
					pBuffer, arg, VertexBuffer::SetBoneMatrix ) ;
	const S3DMatrix *	pMatrix =
		(const S3DMatrix*) context->AtomicTranslateAddress( arg[3].i ) ;
	const S3DVector *	pTrans =
		(const S3DVector*) context->AtomicTranslateAddress( arg[4].i ) ;
	//
	context->m_regset[regAcc].i =
		pBuffer->SetBoneMatrix
			( (size_t) arg[1].i, (size_t) arg[2].i, pMatrix, pTrans ) ;
	//
	return	NULL ;
}

// SGLError VertexBuffer::AllocateMorphing( size_t iMesh, size_t nCount ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_VertexBuffer_AllocateMorphing, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DVertexBufferInterface,
					pBuffer, arg, VertexBuffer::AllocateMorphing ) ;
	//
	context->m_regset[regAcc].i =
		pBuffer->AllocateMorphing
					( (size_t) arg[1].i, (size_t) arg[2].i ) ;
	//
	return	NULL ;
}

// SGLError VertexBuffer::SetMorphingTargetMesh
//	( size_t iMesh, size_t iMorph, size_t countVertex,
//		const S3DVector4 * pvVertex, const S3DVector4 * pvNormal,
//		const S2DVector * pvUVMap, const S3DColor * pColor ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_VertexBuffer_SetMorphingTargetMesh, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DVertexBufferInterface,
					pBuffer, arg, VertexBuffer::SetMorphingTargetMesh ) ;
	const S3DVector4 *	pvVertex =
		(const S3DVector4*) context->AtomicTranslateAddress( arg[4].i ) ;
	const S3DVector4 *	pvNormal =
		(const S3DVector4*) context->AtomicTranslateAddress( arg[5].i ) ;
	const S2DVector *	pvUVMap =
		(const S2DVector*) context->AtomicTranslateAddress( arg[6].i ) ;
	const S3DColor *	pColor =
		(const S3DColor*) context->AtomicTranslateAddress( arg[7].i ) ;
	//
	context->m_regset[regAcc].i =
		pBuffer->SetMorphingTargetMesh
			( (size_t) arg[1].i,
				(size_t) arg[2].i, (size_t) arg[3].i,
				pvVertex, pvNormal, pvUVMap, pColor ) ;
	//
	return	NULL ;
}

// SGLError VertexBuffer::SetMorphingTargetWeight
//	( size_t iMesh, size_t iMorph,
//		size_t countVertex, const float32_t * pfpWeight )
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_VertexBuffer_SetMorphingTargetWeight, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DVertexBufferInterface,
					pBuffer, arg, VertexBuffer::SetMorphingTargetWeight ) ;
	const float32_t *	pfpWeight =
		(const float32_t*) context->AtomicTranslateAddress( arg[4].i ) ;
	//
	context->m_regset[regAcc].i =
		pBuffer->SetMorphingTargetWeight
			( (size_t) arg[1].i,
				(size_t) arg[2].i, (size_t) arg[3].i, pfpWeight ) ;
	//
	return	NULL ;
}

// SGLError VertexBuffer::SetMorphingApplication
//	( size_t iMesh, const ssize_t * pTargetMesh,
//		const float32_t * pApplication, size_t nTargetMeshCount ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_VertexBuffer_SetMorphingApplication, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DVertexBufferInterface,
					pBuffer, arg, VertexBuffer::SetMorphingApplication ) ;
	//
	const int32_t *	pTargetMesh =
		(const int32_t*) context->AtomicTranslateAddress( arg[2].i ) ;
	const float32_t *	pApplication =
		(const float32_t*) context->AtomicTranslateAddress( arg[3].i ) ;
	size_t	nTargetMeshCount = (size_t) arg[4].i ;
	//
	SArray<ssize_t>	bufMesh ;
	ssize_t *	pTargetBuf = bufMesh.GetArray( nTargetMeshCount ) ;
	for ( size_t i = 0; i < nTargetMeshCount; i ++ )
	{
		pTargetBuf[i] = pTargetMesh[i] ;
	}
	//
	context->m_regset[regAcc].i =
		pBuffer->SetMorphingApplication
			( (size_t) arg[1].i,
				pTargetBuf, pApplication, nTargetMeshCount ) ;
	bufMesh.FinishArray() ;
	//
	return	NULL ;
}

// SGLError VertexBuffer::GetMorphingApplication
//	( size_t iMesh, ssize_t& iTargetMesh,
//		float32_t& fpApplication, size_t iTargetMeshIndex ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_VertexBuffer_GetMorphingApplication, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DVertexBufferInterface,
					pBuffer, arg, VertexBuffer::GetMorphingApplication ) ;
	//
	ssize_t		iTargetMesh ;
	float32_t	fpApplication ;
	//
	context->m_regset[regAcc].i =
		pBuffer->GetMorphingApplication
			( (size_t) arg[1].i,
				iTargetMesh, fpApplication, (size_t) arg[4].i ) ;
	//
	uint32_t *	pTargetMesh =
		(uint32_t*) context->AtomicTranslateAddress( arg[2].i ) ;
	float32_t *	pApplication =
		(float32_t*) context->AtomicTranslateAddress( arg[3].i ) ;
	//
	if ( pTargetMesh != NULL )
	{
		*pTargetMesh = (uint32_t) iTargetMesh ;
	}
	if ( pApplication != NULL )
	{
		*pApplication = fpApplication ;
	}
	//
	return	NULL ;
}

// SGLError VertexBuffer::EnableToRenderMesh
//	( size_t iFirst = 0, ssize_t iEnd = -1, bool fEnable = true )
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_VertexBuffer_EnableToRenderMesh, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DVertexBufferInterface,
					pBuffer, arg, VertexBuffer::EnableToRenderMesh ) ;
	//
	context->m_regset[regAcc].i =
		pBuffer->EnableToRenderMesh
			( (size_t) arg[1].i, (ssize_t) arg[2].i, (arg[3].i != 0) ) ;
	//
	return	NULL ;
}

// bool VertexBuffer::IsEnabledToRenderMesh( size_t iMesh )
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_VertexBuffer_IsEnabledToRenderMesh, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DVertexBufferInterface,
					pBuffer, arg, VertexBuffer::EnableToRenderMesh ) ;
	//
	context->m_regset[regAcc].i =
		(pBuffer->IsEnabledToRenderMesh( (size_t) arg[1].i ) ? -1 : 0) ;
	//
	return	NULL ;
}

// SGLError VertexBuffer::RenderBufferTo
//	( RenderContext * render,
//		uint64_t flagsExclusion = 0, size_t iFrist = 0, ssize_t iEnd = -1,
//		size_t nInstancing = 0,
//		const S4DMatrix * pmatInstancing = NULL,
//		const S3DColor * pColorInstancing = NULL ) const
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_VertexBuffer_RenderBufferTo, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DVertexBufferInterface,
					pBuffer, arg, VertexBuffer::RenderBufferTo ) ;
	ECS_DECLARE_SYSCALL_OBJECT
		( vm, S3DRenderContextInterface, pRender,
					arg[1].i, VertexBuffer::RenderBufferTo ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, S4DMatrix, pmatInstancing,
					arg[6].i, RenderContext::AddVertexBuffer ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, S3DColor, pColorInstancing,
					arg[7].i, RenderContext::AddVertexBuffer ) ;
	//
	context->m_regset[regAcc].i =
		pBuffer->RenderBufferTo
			( pRender, arg[2].i, (size_t) arg[3].i, (ssize_t) arg[4].i,
				(size_t) arg[5].i, pmatInstancing, pColorInstancing ) ;
	//
	return	NULL ;
}

// void VertexBuffer::ClearBuffer( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_VertexBuffer_ClearBuffer, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DVertexBufferInterface,
					pBuffer, arg, VertexBuffer::ClearBuffer ) ;
	//
	pBuffer->ClearBuffer() ;
	//
	return	NULL ;
}

// void VertexBuffer::SetBufferUnitSize( size_t nBytes ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_VertexBuffer_SetBufferUnitSize, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DVertexBufferInterface,
					pBuffer, arg, VertexBuffer::SetBufferUnitSize ) ;
	//
	pBuffer->SetBufferUnitSize( (size_t) arg[1].i ) ;
	//
	return	NULL ;
}

// double VertexBuffer::GetCircumscribedSphere( S3DVector& vCenter ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_VertexBuffer_GetCircumscribedSphere, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DVertexBufferInterface,
					pBuffer, arg, VertexBuffer::GetCircumscribedSphere ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, S3DVector, pvCenter,
					arg[1].i, VertexBuffer::GetCircumscribedSphere ) ;
	//
	context->m_regset[regAcc].f =
		pBuffer->GetCircumscribedSphere( *pvCenter ) ;
	//
	return	NULL ;
}

#endif


//////////////////////////////////////////////////////////////////////////////
// RenderContextObject 描画オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO_CAST( ECSSakura2::RenderContextObject, ECSVolatileObject, m_render )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RenderContextObject::RenderContextObject
	( const wchar_t * pwszType,
		SakuraGL::S3DRenderContextInterface * render )
{
	m_pwszType = pwszType ;
	m_render = render ;
}

// S3DRenderContextInterface 関連付け
//////////////////////////////////////////////////////////////////////////////
void RenderContextObject::AttachRenderInterface
		( SakuraGL::S3DRenderContextInterface * render )
{
	m_render = render ;
}

// 実行時型名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * RenderContextObject::GetTypeName( void ) const
{
	return	m_pwszType ;
}


//////////////////////////////////////////////////////////////////////////////
// RenderContextOwnerObject 描画オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ECSSakura2::RenderContextOwnerObject, RenderContextObject )

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
RenderContextOwnerObject::~RenderContextOwnerObject( void )
{
	delete	m_render ;
	m_render = NULL ;
}



//////////////////////////////////////////////////////////////////////////////
// RenderContext スタブ
//////////////////////////////////////////////////////////////////////////////

#if	!defined(ENTISGLS4_DLL_IMPORT)

// new SakuraGL::RenderContext
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_NEW_OBJECT(SakuraGL_RenderContext,context,cls_id)
{
	return	new RenderContextOwnerObject
					( L"SakuraGL::RenderContext",
						S3DRenderContextInterface::NewContext() ) ;
}

// new SakuraGL::HybridRenderContext
ECS_LIB_IMPLEMENT_EXPORT_NEW_OBJECT(SakuraGL_HybridRenderContext,context,cls_id)
{
	return	new RenderContextOwnerObject
					( L"SakuraGL::HybridRenderContext",
								new S3DHybridRenderContext ) ;
}

// static RenderContext * RenderContext::NewContext
//	( SGLPaintContextType type = typePaintDefault ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_RenderContext_NewContext,context,arg)
{
	RenderContextObject *	pObj = NULL ;
	const SGLPaintContextType	type = (SGLPaintContextType) arg[0].i ;
	switch ( type )
	{
	case	typePaintEntisGLS:
		#if	!defined(__PLATFORM_WINDOWS__) || !defined(__ENTIS_GLS__)
		pObj = new RenderContextOwnerObject
				( L"SakuraGL::HybridRenderContext",
						new S3DHybridRenderContext( type ) ) ;
		break ;
		#endif
	case	typePaintDefault:
	default:
		pObj = new RenderContextOwnerObject
					( L"SakuraGL::RenderContext",
						S3DRenderContextInterface::NewContext( type ) ) ;
		break ;
	}
	if ( pObj != NULL )
	{
		ECS_DECLARE_SYSCALL_VM( context, vm ) ;
		AssertLock() ;
		context->m_regset[regAcc].i = vm->AllocateHeapObjectAddress( pObj ) ;
		AssertUnlock() ;
	}
	else
	{
		context->m_regset[regAcc].i = 0 ;
	}
	return	NULL ;
}

// SGLError RenderContext::AppendMatrixTransformation
//	( const S3DDMatrix& mat, const S3DDVector& pos,
//		const S3DColor * color = NULL, unsigned int nTransparency = 0 ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_RenderContext_AppendMatrixTransformation,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DRenderContextInterface,
					pRender, arg, RenderContext::AppendMatrixTransformation ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, S3DDMatrix, pMatrix,
					arg[1].i, RenderContext::AppendMatrixTransformation ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, S3DDVector, pPos,
					arg[2].i, RenderContext::AppendMatrixTransformation ) ;
	const S3DColor *	pColor =
		(const S3DColor*)
			context->AtomicTranslateAddress( arg[3].i, sizeof(S3DColor) ) ;
	const unsigned int	nTransparency = (unsigned int) arg[4].i ;
	//
	context->m_regset[regAcc].i =
		pRender->AppendMatrixTransformation
			( *pMatrix, *pPos, pColor, nTransparency ) ;
	//
	return	NULL ;
}

// SGLError RenderContext::SetMatrixTransformation
//	( const S3DDMatrix & mat, const S3DDVector& pos,
//		const S3DColor * color = NULL, unsigned int nTransparency = 0 ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_RenderContext_SetMatrixTransformation,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DRenderContextInterface,
					pRender, arg, RenderContext::SetMatrixTransformation ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, S3DDMatrix, pMatrix,
					arg[1].i, RenderContext::SetMatrixTransformation ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, S3DDVector, pPos,
					arg[2].i, RenderContext::SetMatrixTransformation ) ;
	const S3DColor *	pColor =
		(const S3DColor*)
			context->AtomicTranslateAddress( arg[3].i, sizeof(S3DColor) ) ;
	const unsigned int	nTransparency = (unsigned int) arg[4].i ;
	//
	context->m_regset[regAcc].i =
		pRender->SetMatrixTransformation
			( *pMatrix, *pPos, pColor, nTransparency ) ;
	//
	return	NULL ;
}

// SGLError RenderContext::GetMatrixTransformation
//	( S3DDMatrix& mat, S3DDVector& pos,
//		S3DColor * color = NULL, unsigned int * pTransparency = 0 ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_RenderContext_GetMatrixTransformation,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DRenderContextInterface,
					pRender, arg, RenderContext::GetMatrixTransformation ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, S3DDMatrix, pMatrix,
					arg[1].i, RenderContext::GetMatrixTransformation ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, S3DDVector, pPos,
					arg[2].i, RenderContext::GetMatrixTransformation ) ;
	S3DColor *	pColor =
		(S3DColor*)
			context->AtomicTranslateAddress( arg[3].i, sizeof(S3DColor) ) ;
	unsigned int *	pTransparency =
		(unsigned int*)
			context->AtomicTranslateAddress( arg[4].i, sizeof(unsigned int) ) ;
	//
	context->m_regset[regAcc].i =
		pRender->GetMatrixTransformation
			( *pMatrix, *pPos, pColor, pTransparency ) ;
	//
	return	NULL ;
}

// SGLError RenderContext::SetProjectionScreen
//	( const S3DVector& vScreen, double zScale = 1.0, double fpPixelAspect = 1.0 ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_RenderContext_SetProjectionScreen,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DRenderContextInterface,
					pRender, arg, RenderContext::SetProjectionScreen ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, const S3DVector, pScreenPos,
					arg[1].i, RenderContext::SetProjectionScreen ) ;
	//
	context->m_regset[regAcc].i =
		pRender->SetProjectionScreen( *pScreenPos, arg[2].f, arg[3].f ) ;
	//
	return	NULL ;
}

// SGLError RenderContext::GetProjectionScreen
//	( S3DVector& vScreen, double& zScale, double& fpPixelAspect ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_RenderContext_GetProjectionScreen,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DRenderContextInterface,
					pRender, arg, RenderContext::GetProjectionScreen ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, S3DVector, pScreenPos,
					arg[1].i, RenderContext::GetProjectionScreen ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, double, pScaleZ,
					arg[2].i, RenderContext::GetProjectionScreen ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, double, pPixelAspect,
					arg[3].i, RenderContext::GetProjectionScreen ) ;
	//
	context->m_regset[regAcc].i =
		pRender->GetProjectionScreen( *pScreenPos, *pScaleZ, *pPixelAspect ) ;
	//
	return	NULL ;
}

// bool RenderContext::GetPerspectiveMatrix( S4DMatrix& matPers ) const ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_RenderContext_GetPerspectiveMatrix,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DRenderContextInterface,
					pRender, arg, RenderContext::GetPerspectiveMatrix ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, S4DMatrix, pmatPers,
					arg[1].i, RenderContext::GetPerspectiveMatrix ) ;
	//
	context->m_regset[regAcc].i =
		pRender->GetPerspectiveMatrix( *pmatPers ) ? -1 : 0 ;
	//
	return	NULL ;
}

// void RenderContext::SetPerspectiveMatrix
//		( StereoViewIndex sviView,
//			const S4DMatrix& matPers, bool fPersMatrix = true ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_RenderContext_SetPerspectiveMatrix,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DRenderContextInterface,
					pRender, arg, RenderContext::SetPerspectiveMatrix ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, S4DMatrix, pmatPers,
					arg[2].i, RenderContext::SetPerspectiveMatrix ) ;
	//
	pRender->SetPerspectiveMatrix
		( (RenderContext::StereoViewIndex) arg[1].i,
							*pmatPers, (arg[3].i != 0) ) ;
	//
	return	NULL ;
}

// void RenderContext::EnablePerspectiveMatrix( bool fPersMatrix ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_RenderContext_EnablePerspectiveMatrix,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DRenderContextInterface,
				pRender, arg, RenderContext::EnablePerspectiveMatrix ) ;
	//
	pRender->EnablePerspectiveMatrix( arg[1].i != 0 ) ;
	//
	return	NULL ;
}

// void RenderContext::SetCamera( const S3DDMatrix& matCamera, const S3DDVector& posCamera ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_RenderContext_SetCamera,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DRenderContextInterface,
					pRender, arg, RenderContext::SetCamera ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, const S3DDMatrix, pCameraMatrix,
					arg[1].i, RenderContext::SetCamera ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, const S3DDVector, pCameraPos,
					arg[2].i, RenderContext::SetCamera ) ;
	//
	pRender->SetCamera( *pCameraMatrix, *pCameraPos ) ;
	//
	return	NULL ;
}

// void RenderContext::GetCamera( S3DDMatrix& matCamera, S3DDVector& posCamera ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_RenderContext_GetCamera,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DRenderContextInterface,
					pRender, arg, RenderContext::GetCamera ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, S3DDMatrix, pCameraMatrix,
					arg[1].i, RenderContext::GetCamera ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, S3DDVector, pCameraPos,
					arg[2].i, RenderContext::GetCamera ) ;
	//
	pRender->GetCamera( *pCameraMatrix, *pCameraPos ) ;
	//
	return	NULL ;
}

// void RenderContext::SetParallax( double xParallax, double zFocusRate, double xScreenDelta ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_RenderContext_SetParallax,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DRenderContextInterface,
					pRender, arg, RenderContext::SetParallax ) ;
	//
	pRender->SetParallax( arg[1].f, arg[2].f, arg[3].f ) ;
	//
	return	NULL ;
}

// double RenderContext::GetParallax( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_RenderContext_GetParallax,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DRenderContextInterface,
					pRender, arg, RenderContext::GetParallax ) ;
	//
	context->m_regset[regAcc].f = pRender->GetParallax() ;
	//
	return	NULL ;
}

// void RenderContext::SetZClipRange( double zMin, double zMax ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_RenderContext_SetZClipRange,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DRenderContextInterface,
					pRender, arg, RenderContext::SetZClipRange ) ;
	//
	pRender->SetZClipRange( arg[1].f, arg[2].f ) ;
	//
	return	NULL ;
}

// void RenderContext::SetLightEntries( const S3DLightEntry* pLights, size_t countLight ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_RenderContext_SetLightEntries,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DRenderContextInterface,
					pRender, arg, RenderContext::SetLightEntries ) ;
	ECS_DECLARE_SYSCALL_ARRAYVAR
		( context, const S3DLightEntry, pLights,
					arg[1].i, arg[2].i, RenderContext::SetLightEntries ) ;
	//
	pRender->SetLightEntries( pLights, (size_t) arg[2].i ) ;
	//
	return	NULL ;
}

// void RenderContext::SetShadowMap
//	( uint32_t idLight, Image* pShadowMap,
//    const S3DShadowMapInfo& infShadowMap, Image* pShadowMapColor ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_RenderContext_SetShadowMap,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DRenderContextInterface,
					pRender, arg, RenderContext::SetShadowMap ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, S3DShadowMapInfo, pShadowMapInf,
					arg[3].i, RenderContext::SetShadowMap ) ;
	SGLImageObject *	pShadowMap =
		ESLTypeCast<SGLImageObject>( vm->ObjectFromAddress( arg[2].h32 ) ) ;
	SGLImageObject *	pShadowMapColor =
		ESLTypeCast<SGLImageObject>( vm->ObjectFromAddress( arg[4].h32 ) ) ;
	//
	pRender->SetShadowMap
		( (uint32_t) arg[1].i,
			pShadowMap, *pShadowMapInf, pShadowMapColor ) ;
	//
	return	NULL ;
}

// void RenderContext::SetFog( uint32_t rgbFog, double zFogNear, double zFogFar ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_RenderContext_SetFog,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DRenderContextInterface,
					pRender, arg, RenderContext::SetFog ) ;
	//
	pRender->SetFog( (uint32_t) arg[1].i, arg[2].f, arg[3].f ) ;
	//
	return	NULL ;
}

// void RenderContext::EnableFog( bool fFog ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_RenderContext_EnableFog,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DRenderContextInterface,
					pRender, arg, RenderContext::EnableFog ) ;
	//
	pRender->EnableFog( arg[1].i != 0 ) ;
	//
	return	NULL ;
}

// void RenderContext::SetShadingFlag( uint64_t nShadingMethod ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_RenderContext_SetShadingFlag,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DRenderContextInterface,
					pRender, arg, RenderContext::SetShadingFlag ) ;
	//
	pRender->SetShadingFlag( arg[1].i ) ;
	//
	return	NULL ;
}

// uint64_t RenderContext::GetShadingFlag( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_RenderContext_GetShadingFlag,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DRenderContextInterface,
					pRender, arg, RenderContext::GetShadingFlag ) ;
	//
	context->m_regset[regAcc].i = pRender->GetShadingFlag() ;
	//
	return	NULL ;
}

// void RenderContext::SetRayTracingParameter( const S3DRenderRayTracingParam& rrtp ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_RenderContext_SetRayTracingParameter,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DRenderContextInterface,
					pRender, arg, RenderContext::SetRayTracingParameter ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, const S3DRenderRayTracingParam, pParam,
					arg[1].i, RenderContext::SetRayTracingParameter ) ;
	//
	pRender->SetRayTracingParameter( *pParam ) ;
	//
	return	NULL ;
}

// void SetEnvironmentMappingImage( Image * pImage, uint32_t nFlags ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_RenderContext_SetEnvironmentMappingImage,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DRenderContextInterface,
					pRender, arg, RenderContext::SetEnvironmentMappingImage ) ;
	SGLImageObject *	pImage =
			ESLTypeCast<SGLImageObject>
				( vm->AtomicObjectFromAddress( arg[1].h32 ) ) ;
	//
	pRender->SetEnvironmentMappingImage( pImage, (uint32_t) arg[2].i ) ;
	//
	return	NULL ;
}

// void SetEnvironmentMappingMatrix( S3DMatrix& matMapping ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_RenderContext_SetEnvironmentMappingMatrix,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DRenderContextInterface,
					pRender, arg, RenderContext::SetEnvironmentMappingMatrix ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, S3DMatrix, pmatMapping,
					arg[1].i, RenderContext::SetEnvironmentMappingMatrix ) ;
	//
	pRender->SetEnvironmentMappingMatrix( *pmatMapping ) ;
	//
	return	NULL ;
}

// void SetOffsetBorderColor( uint32_t rgbBorder ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_RenderContext_SetOffsetBorderColor,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DRenderContextInterface,
					pRender, arg, RenderContext::SetOffsetBorderColor ) ;
	//
	pRender->SetOffsetBorderColor( (uint32_t) arg[1].i ) ;
	//
	return	NULL ;
}

// void SetOffsetBorderCoefficient( float32_t a, float32_t b ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_RenderContext_SetOffsetBorderCoefficient,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DRenderContextInterface,
					pRender, arg, RenderContext::SetOffsetBorderCoefficient ) ;
	//
	pRender->SetOffsetBorderCoefficient( (float32_t) arg[1].f, (float32_t) arg[2].f ) ;
	//
	return	NULL ;
}

// SGLError SetOptionalFeature
//	( FeatureType feature, int32_t nParam1, const void * pParam2, size_t sizeOfParam2 ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_RenderContext_SetOptionalFeature,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DRenderContextInterface,
					pRender, arg, RenderContext::SetOptionalFeature ) ;
	void * pParam2 = NULL ;
	if ( arg[3].i )
	{
		pParam2 = context->AtomicTranslateAddress( arg[3].i ) ;
	}
	//
	context->m_regset[regAcc].i =
		pRender->SetOptionalFeature
			( (RenderContext::FeatureType) arg[1].i,
					(int32_t) arg[2].i, pParam2, (size_t) arg[4].i ) ;
	//
	return	NULL ;
}

// void GetRenderingCapacity( S3DRenderingCapacity& caps ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_RenderContext_GetRenderingCapacity,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DRenderContextInterface,
					pRender, arg, RenderContext::GetRenderingCapacity ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, S3DRenderingCapacity, caps,
					arg[1].i, RenderContext::GetRenderingCapacity ) ;
	//
	pRender->GetRenderingCapacity( *caps ) ;
	//
	return	NULL ;
}

// StereoViewIndex CurrentParallaxView( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_RenderContext_CurrentParallaxView,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DRenderContextInterface,
					pRender, arg, RenderContext::CurrentParallaxView ) ;
	//
	context->m_regset[regAcc].i = pRender->CurrentParallaxView() ;
	//
	return	NULL ;
}

// SGLError RenderContext::SelectParallaxView( StereoViewIndex sviView ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_RenderContext_SelectParallaxView,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DRenderContextInterface,
					pRender, arg, RenderContext::SelectParallaxView ) ;
	//
	context->m_regset[regAcc].i =
		pRender->SelectParallaxView
			( (S3DRenderContextInterface::StereoViewIndex) arg[1].i ) ;
	//
	return	NULL ;
}

// SGLError RenderContext::SetRenderingBufferSize( uint32_t countVertex ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_RenderContext_SetRenderingBufferSize,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DRenderContextInterface,
					pRender, arg, RenderContext::SetRenderingBufferSize ) ;
	//
	context->m_regset[regAcc].i =
		pRender->SetRenderingBufferSize( (uint32_t) arg[1].i ) ;
	//
	return	NULL ;
}

// SGLError RenderContext::Begin3DRenderer( uint64_t nFlags ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_RenderContext_Begin3DRenderer,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DRenderContextInterface,
					pRender, arg, RenderContext::Begin3DRenderer ) ;
	//
	context->m_regset[regAcc].i = pRender->Begin3DRenderer( arg[1].i ) ;
	//
	return	NULL ;
}

// SGLError RenderContext::End3DRenderer( uint64_t nFlags ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_RenderContext_End3DRenderer,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DRenderContextInterface,
					pRender, arg, RenderContext::End3DRenderer ) ;
	//
	context->m_regset[regAcc].i = pRender->End3DRenderer( arg[1].i ) ;
	//
	return	NULL ;
}

// SGLError RenderContext::AddIndexedTriangleList
//	( Material * pMaterial, uint32_t nFlags,
//		size_t countPolygon, size_t countVertex,
//		const S3DVector4 * pvVertex, const S3DVector4 * pvNormal,
//		const S2DVector * pvUVMap, const S3DColor * pColor,
//		const uint32_t * pIndexedList ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_RenderContext_AddIndexedTriangleList,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DRenderContextInterface,
					pRender, arg, RenderContext::AddIndexedTriangleList ) ;
	ECS_DECLARE_SYSCALL_OBJECT
		( vm, S3DMaterial, pMaterial,
					arg[1].i, RenderContext::AddIndexedTriangleList ) ;
	const S3DVector4 *	pvVertex =
		(const S3DVector4*) context->AtomicTranslateAddress( arg[5].i ) ;
	const S3DVector4 *	pvNormal =
		(const S3DVector4*) context->AtomicTranslateAddress( arg[6].i ) ;
	const S2DVector *	pvUVMap =
		(const S2DVector*) context->AtomicTranslateAddress( arg[7].i ) ;
	const S3DColor *	pColor =
		(const S3DColor*) context->AtomicTranslateAddress( arg[8].i ) ;
	const uint32_t *	pIndexedList =
		(const uint32_t*) context->AtomicTranslateAddress( arg[9].i ) ;
	//
	context->m_regset[regAcc].i =
		pRender->AddIndexedTriangleList
			( pMaterial, (uint32_t) arg[2].i,
				(size_t) arg[3].i, (size_t) arg[4].i,
				pvVertex, pvNormal, pvUVMap, pColor, pIndexedList ) ;
	//
	return	NULL ;
}

// SGLError RenderContext::AddTriangleStrip
//	( Material * pMaterial, uint32_t nFlags, size_t countTriangleStrip,
//		const S3DVector4 * pvVertex, const S3DVector4 * pvNormal,
//		const S2DVector * pvUVMap, const S3DColor * pColor ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_RenderContext_AddTriangleStrip,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DRenderContextInterface,
					pRender, arg, RenderContext::AddTriangleStrip ) ;
	ECS_DECLARE_SYSCALL_OBJECT
		( vm, S3DMaterial, pMaterial,
					arg[1].i, RenderContext::AddTriangleStrip ) ;
	const S3DVector4 *	pvVertex =
		(const S3DVector4*) context->AtomicTranslateAddress( arg[4].i ) ;
	const S3DVector4 *	pvNormal =
		(const S3DVector4*) context->AtomicTranslateAddress( arg[5].i ) ;
	const S2DVector *	pvUVMap =
		(const S2DVector*) context->AtomicTranslateAddress( arg[6].i ) ;
	const S3DColor *	pColor =
		(const S3DColor*) context->AtomicTranslateAddress( arg[7].i ) ;
	//
	context->m_regset[regAcc].i =
		pRender->AddTriangleStrip
			( pMaterial, (uint32_t) arg[2].i, (size_t) arg[3].i,
				pvVertex, pvNormal, pvUVMap, pColor ) ;
	//
	return	NULL ;
}

// SGLError RenderContext::AddIndexedPrimitiveList
//	( S3DMaterial * pMaterial, uint32_t nFlags,
//		S3DPrimitiveType typePrimitive,
//		size_t countIndex, size_t countVertex,
//		const S3DVector4 * pvVertex, const S3DVector4 * pvNormal,
//		const S2DVector * pvUVMap, const S3DColor * pColor,
//		const uint32_t * pIndexedList ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_RenderContext_AddIndexedPrimitiveList,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DRenderContextInterface,
					pRender, arg, RenderContext::AddIndexedPrimitiveList ) ;
	ECS_DECLARE_SYSCALL_OBJECT
		( vm, S3DMaterial, pMaterial,
					arg[1].i, RenderContext::AddIndexedPrimitiveList ) ;
	const S3DVector4 *	pvVertex =
		(const S3DVector4*) context->AtomicTranslateAddress( arg[6].i ) ;
	const S3DVector4 *	pvNormal =
		(const S3DVector4*) context->AtomicTranslateAddress( arg[7].i ) ;
	const S2DVector *	pvUVMap =
		(const S2DVector*) context->AtomicTranslateAddress( arg[8].i ) ;
	const S3DColor *	pColor =
		(const S3DColor*) context->AtomicTranslateAddress( arg[9].i ) ;
	const uint32_t *	pIndexedList =
		(const uint32_t*) context->AtomicTranslateAddress( arg[10].i ) ;
	//
	context->m_regset[regAcc].i =
		pRender->AddIndexedPrimitiveList
			( pMaterial, (uint32_t) arg[2].i,
				(S3DPrimitiveType) arg[3].i,
				(size_t) arg[4].i, (size_t) arg[5].i,
				pvVertex, pvNormal, pvUVMap, pColor, pIndexedList ) ;
	//
	return	NULL ;
}

// SGLError RenderContext::AddVertexBuffer
//	( Material * pMaterial, uint32_t nFlags,
//		VertexBuffer * pBuffer, size_t iFirst = 0, ssize_t iEnd = -1,
//		size_t nInstancing = 0,
//		const S4DMatrix * pmatInstancing = NULL,
//		const S3DColor * pColorInstancing = NULL )
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_RenderContext_AddVertexBuffer,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, S3DRenderContextInterface,
					pRender, arg, RenderContext::AddVertexBuffer ) ;
	ECS_DECLARE_SYSCALL_OBJECT
		( vm, S3DMaterial, pMaterial,
					arg[1].i, RenderContext::AddVertexBuffer ) ;
	ECS_DECLARE_SYSCALL_OBJECT
		( vm, S3DVertexBufferInterface, pBuffer,
					arg[3].i, RenderContext::AddVertexBuffer ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, S4DMatrix, pmatInstancing,
					arg[7].i, RenderContext::AddVertexBuffer ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, S3DColor, pColorInstancing,
					arg[8].i, RenderContext::AddVertexBuffer ) ;
	//
	context->m_regset[regAcc].i =
		pRender->AddVertexBuffer
			( pMaterial, (uint32_t) arg[2].i,
				pBuffer, (size_t) arg[4].i, (ssize_t) arg[5].i,
				(size_t) arg[6].i, pmatInstancing, pColorInstancing ) ;
	//
	return	NULL ;
}

#endif
