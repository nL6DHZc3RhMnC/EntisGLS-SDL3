
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/


#include <sakuraglx/sakuraglx.h>
#include <sakura/ssys_module.h>
#include <sakuragl/sgl2d_image.h>
#include <sakuragl/sgl2d/sgl_paint_buffer.h>
#include <glscs/glscs_sakura2_obj_paint.h>

using	namespace SSystem ;
using	namespace SakuraGL ;
using	namespace ECSSakura2 ;
using	namespace ECSSakura2Processor ;


//////////////////////////////////////////////////////////////////////////////
// 描画オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO_CAST( ECSSakura2::PaintContextObject, ECSVolatileObject, m_paint )
ESL_IMPLEMENT_CLASS_INFO( ECSSakura2::PaintContextOwnerObject, PaintContextObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
PaintContextObject::PaintContextObject
	( const wchar_t * pwszType,
		SakuraGL::SGLPaintContextInterface * paint )
{
	m_pwszType = pwszType ;
	m_paint = paint ;
}

// 実行時型名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * PaintContextObject::GetTypeName( void ) const
{
	return	m_pwszType ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
PaintContextOwnerObject::~PaintContextOwnerObject( void )
{
	delete	m_paint ;
	m_paint = NULL ;
}


//////////////////////////////////////////////////////////////////////////////
// PaintContext スタブ
//////////////////////////////////////////////////////////////////////////////

#if	!defined(ENTISGLS4_DLL_IMPORT)

// new SakuraGL::PaintContext
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_NEW_OBJECT(SakuraGL_PaintContext,context,cls_id)
{
	return	new PaintContextOwnerObject
				( L"SakuraGL::PaintContext", new SGLPaintBuffer ) ;
}

// PaintContext* PaintContext::NewContext
//		( SGLPaintContextType type = typePaintDefault )
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_PaintContext_NewContext,context,arg)
{
	PaintContextObject *	pObj = NULL ;
	const int	type = (int) arg[0].i ;
	switch ( type )
	{
	case	typePaintDefault:
	default:
		pObj = new PaintContextOwnerObject
					( L"SakuraGL::PaintContext", new SGLPaintBuffer ) ;
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

// Image * PaintContext::GetTargetImage( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_PaintContext_GetTargetImage,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLPaintContextInterface,
					pPaint, arg, PaintContext::GetTargetImage ) ;
	Object *	pObjImage = ESLTypeCast<Object>( pPaint->GetTargetImage() ) ;
	if ( pObjImage != NULL )
	{
		context->m_regset[regAcc].h32 = pObjImage->m_dwHighAddr ;
		context->m_regset[regAcc].l32 = 0 ;
	}
	else
	{
		context->m_regset[regAcc].i = 0 ;
	}
	return	NULL ;
}

// Image * PaintContext::GetTargetZBuffer( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_PaintContext_GetTargetZBuffer,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLPaintContextInterface,
					pPaint, arg, PaintContext::GetTargetZBuffer ) ;
	Object *	pObjImage = ESLTypeCast<Object>( pPaint->GetTargetZBuffer() ) ;
	if ( pObjImage != NULL )
	{
		context->m_regset[regAcc].h32 = pObjImage->m_dwHighAddr ;
		context->m_regset[regAcc].l32 = 0 ;
	}
	else
	{
		context->m_regset[regAcc].i = 0 ;
	}
	return	NULL ;
}

// SGLError PaintContext::GetViewPort( SGLImageRect & rctView ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_PaintContext_GetViewPort,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLPaintContextInterface,
					pPaint, arg, PaintContext::GetViewPort ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, SGLImageRect, pViewRect,
					arg[1].i, PaintContext::GetViewPort ) ;
	//
	context->m_regset[regAcc].i = pPaint->GetViewPort( *pViewRect ) ;
	//
	return	NULL ;
}

// SGLError PaintContext::AttachTargetImage
//	( Image * pImage, Image * pZBuffer, const SGLImageRect * pView = NULL ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_PaintContext_AttachTargetImage,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLPaintContextInterface,
					pPaint, arg, PaintContext::AttachTargetImage ) ;
	SGLImageObject *	pImage =
		ESLTypeCast<SGLImageObject>( vm->ObjectFromAddress( arg[1].h32 ) ) ;
	SGLImageObject *	pZBuffer =
		ESLTypeCast<SGLImageObject>( vm->ObjectFromAddress( arg[2].h32 ) ) ;
	const SGLImageRect *	pView =
		(const SGLImageRect*)
			context->AtomicTranslateAddress( arg[3].i, sizeof(SGLImageRect) ) ;
	//
	context->m_regset[regAcc].i =
		pPaint->AttachTargetImage( pImage, pZBuffer, pView ) ;
	//
	return	NULL ;
}

// SGLError PaintContext::DetachTargetImage( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_PaintContext_DetachTargetImage,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLPaintContextInterface,
					pPaint, arg, PaintContext::DetachTargetImage ) ;
	//
	context->m_regset[regAcc].i = pPaint->DetachTargetImage() ;
	//
	return	NULL ;
}

// SGLError PaintContext::AppendTransformation
//	( const SGLAffine & af, unsigned int nTransparency ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_PaintContext_AppendTransformation,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLPaintContextInterface,
					pPaint, arg, PaintContext::AppendTransformation ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, SGLAffine, pAffine,
					arg[1].i, PaintContext::AppendTransformation ) ;
	//
	context->m_regset[regAcc].i =
		pPaint->AppendTransformation( *pAffine, (unsigned int) arg[2].i ) ;
	//
	return	NULL ;
}

// SGLError PaintContext::SetTransformation
//	( const SGLAffine & af, unsigned int nTransparency ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_PaintContext_SetTransformation,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLPaintContextInterface,
					pPaint, arg, PaintContext::SetTransformation ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, SGLAffine, pAffine,
					arg[1].i, PaintContext::SetTransformation ) ;
	//
	context->m_regset[regAcc].i =
		pPaint->SetTransformation( *pAffine, (unsigned int) arg[2].i ) ;
	//
	return	NULL ;
}

// SGLError PaintContext::CurrentAffine( SGLAffine & af ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_PaintContext_CurrentAffine,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLPaintContextInterface,
					pPaint, arg, PaintContext::CurrentAffine ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, SGLAffine, pAffine,
					arg[1].i, PaintContext::AppendTransformation ) ;
	//
	context->m_regset[regAcc].i = pPaint->CurrentAffine( *pAffine ) ;
	//
	return	NULL ;
}

// SGLError PaintContext::CurrentTransparency( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_PaintContext_CurrentTransparency,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLPaintContextInterface,
					pPaint, arg, PaintContext::CurrentAffine ) ;
	//
	context->m_regset[regAcc].i = pPaint->CurrentTransparency() ;
	//
	return	NULL ;
}

// SGLError PaintContext::PushTransformation( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_PaintContext_PushTransformation,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLPaintContextInterface,
					pPaint, arg, PaintContext::PushTransformation ) ;
	//
	context->m_regset[regAcc].i = pPaint->PushTransformation() ;
	//
	return	NULL ;
}

// SGLError PaintContext::PopTransformation( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_PaintContext_PopTransformation,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLPaintContextInterface,
					pPaint, arg, PaintContext::PopTransformation ) ;
	//
	context->m_regset[regAcc].i = pPaint->PopTransformation() ;
	//
	return	NULL ;
}

// SGLError PaintContext::ResetTransformation( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_PaintContext_ResetTransformation,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLPaintContextInterface,
					pPaint, arg, PaintContext::ResetTransformation ) ;
	//
	context->m_regset[regAcc].i = pPaint->ResetTransformation() ;
	//
	return	NULL ;
}

// void PaintContext::SetPaintFlags( int64_t nFlags ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_PaintContext_SetPaintFlags,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLPaintContextInterface,
					pPaint, arg, PaintContext::SetPaintFlags ) ;
	//
	pPaint->SetPaintFlags( arg[1].i ) ;
	//
	return	NULL ;
}

// int64_t PaintContext::GetPaintFlags( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_PaintContext_GetPaintFlags,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLPaintContextInterface,
					pPaint, arg, PaintContext::GetPaintFlags ) ;
	//
	context->m_regset[regAcc].i = pPaint->GetPaintFlags() ;
	//
	return	NULL ;
}

// SGLError PaintContext::FillClearTarget
//		( uint32_t argb = 0xff000000, int64_t flags = 0 ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_PaintContext_FillClearTarget,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLPaintContextInterface,
					pPaint, arg, PaintContext::FillClearTarget ) ;
	//
	context->m_regset[regAcc].i =
		pPaint->FillClearTarget( (uint32_t) arg[1].i, arg[2].i ) ;
	//
	return	NULL ;
}

// SGLError PaintContext::FillRectangle
//	( int x, int y, int width, int height,
//			uint32_t argb, double z, uint32_t flags ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_PaintContext_FillRectangle,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLPaintContextInterface,
					pPaint, arg, PaintContext::FillRectangle ) ;
	//
	context->m_regset[regAcc].i =
		pPaint->FillRectangle
			( (int) arg[1].i, (int) arg[2].i,
				(int) arg[3].i, (int) arg[4].i,
				(uint32_t) arg[5].i, arg[6].f, (uint32_t) arg[7].i ) ;
	//
	return	NULL ;
}

// SGLError PaintContext::FillPolygon
//	( const S2DVector * vertices, size_t count,
//			uint32_t argb, double z, uint32_t flags ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_PaintContext_FillPolygon,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLPaintContextInterface,
					pPaint, arg, PaintContext::FillPolygon ) ;
	ECS_DECLARE_SYSCALL_ARRAYVAR
		( context, const S2DVector, vertices,
				arg[1].i, arg[2].i, PaintContext::FillPolygon ) ;
	//
	context->m_regset[regAcc].i =
		pPaint->FillPolygon
			( vertices, (size_t) arg[2].i,
				(uint32_t) arg[3].i, arg[4].f, (uint32_t) arg[5].i ) ;
	//
	return	NULL ;
}

// SGLError PaintContext::DrawImage
//	( const SGLPaintParam & ppPaint,
//		Image * pSrcImage, const SGLImageRect * pSrcClip = NULL ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_PaintContext_DrawImage,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLPaintContextInterface,
					pPaint, arg, PaintContext::DrawImage ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, ECS_SGL_PAINT_PARAM, pPaintParam,
						arg[1].i, PaintContext::DrawImage ) ;
	SGLImageObject *	pSrcImage =
		ESLTypeCast<SGLImageObject>( vm->ObjectFromAddress( arg[2].h32 ) ) ;
	const SGLImageRect *	pSrcClip =
		(const SGLImageRect*)
			context->AtomicTranslateAddress( arg[3].i, sizeof(SGLImageRect) ) ;
	//
	SGLPaintParam	ppParam ;
	eslFillMemory( &ppParam, 0, sizeof(SGLPaintParam) ) ;
	ppParam.nFlags			= pPaintParam->nFlags ;
	ppParam.ptPaint			= pPaintParam->ptPaint ;
	ppParam.nTransparency	= pPaintParam->nTransparency ;
	ppParam.zOrder			= pPaintParam->zOrder ;
	ppParam.rgbColorParam	= pPaintParam->rgbColorParam ;
	ppParam.countVertex		= pPaintParam->countVertex ;
	ppParam.pAffine =
		(const SGLAffine *)
			context->AtomicTranslateAddress
				( pPaintParam->pAffine, sizeof(SGLAffine) ) ;
	ppParam.pVertices =
		(const S2DVector *)
			context->AtomicTranslateAddress
				( pPaintParam->pVertices,
					ppParam.countVertex * sizeof(S2DVector) ) ;
	//
	context->m_regset[regAcc].i =
		pPaint->DrawImage( ppParam, pSrcImage, pSrcClip ) ;
	//
	return	NULL ;
}

// SGLError PaintContext::DrawMesh
//	( const S2DVector * pDstMesh,
//		const S2DVector * pSrcMesh,
//		size_t widthMesh, size_t heightMesh,
//		const SGLPaintParam & ppPaint,
//		Image * pSrcImage, const SGLImageRect * pSrcClip = NULL ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_PaintContext_DrawMesh,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLPaintContextInterface,
					pPaint, arg, PaintContext::DrawMesh ) ;
	const size_t	widthMesh = (size_t) arg[3].i ;
	const size_t	heightMesh = (size_t) arg[4].i ;
	const size_t	countMeshElement = (widthMesh + 1) * (heightMesh + 1) ;
	const S2DVector *	pDstMesh =
		(const S2DVector*)
			context->AtomicTranslateAddress
				( arg[1].i, countMeshElement * sizeof(S2DVector) ) ;
	const S2DVector *	pSrcMesh =
		(const S2DVector*)
			context->AtomicTranslateAddress
				( arg[2].i, countMeshElement * sizeof(S2DVector) ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, ECS_SGL_PAINT_PARAM, pPaintParam,
						arg[5].i, PaintContext::DrawMesh ) ;
	SGLImageObject *	pSrcImage =
		ESLTypeCast<SGLImageObject>( vm->ObjectFromAddress( arg[6].h32 ) ) ;
	const SGLImageRect *	pSrcClip =
		(const SGLImageRect*)
			context->AtomicTranslateAddress( arg[7].i, sizeof(SGLImageRect) ) ;
	//
	SGLPaintParam	ppParam ;
	eslFillMemory( &ppParam, 0, sizeof(SGLPaintParam) ) ;
	ppParam.nFlags			= pPaintParam->nFlags ;
	ppParam.ptPaint			= pPaintParam->ptPaint ;
	ppParam.nTransparency	= pPaintParam->nTransparency ;
	ppParam.zOrder			= pPaintParam->zOrder ;
	ppParam.rgbColorParam	= pPaintParam->rgbColorParam ;
	//
	context->m_regset[regAcc].i =
		pPaint->DrawMesh
			( pDstMesh, pSrcMesh, widthMesh, heightMesh,
							ppParam, pSrcImage, pSrcClip ) ;
	//
	return	NULL ;
}

// SGLError PaintContext::Flush( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_PaintContext_Flush,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLPaintContextInterface,
					pPaint, arg, PaintContext::Flush ) ;
	//
	context->m_regset[regAcc].i = pPaint->Flush() ;
	//
	return	NULL ;
}

// SGLError PaintContext::Finish( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_PaintContext_Finish,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLPaintContextInterface,
					pPaint, arg, PaintContext::Finish ) ;
	//
	context->m_regset[regAcc].i = pPaint->Finish() ;
	//
	return	NULL ;
}

#endif
