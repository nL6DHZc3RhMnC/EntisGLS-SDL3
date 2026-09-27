
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/

#if	!defined(__GLSCS_SAKURA2_OBJECT_PAINT_H__)
#define	__GLSCS_SAKURA2_OBJECT_PAINT_H__

namespace	ECSSakura2
{
	//////////////////////////////////////////////////////////////////////////
	// 描画オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	PaintContextObject	: public ECSVolatileObject
	{
	protected:
		const wchar_t *							m_pwszType ;
		SakuraGL::SGLPaintContextInterface *	m_paint ;
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( PaintContextObject, Object )
		// 構築関数
		PaintContextObject
			( const wchar_t * pwszType,
				SakuraGL::SGLPaintContextInterface * paint ) ;
	public:
		// 実行時型名
		virtual const wchar_t * GetTypeName( void ) const ;
	} ;

	class	PaintContextOwnerObject	: public PaintContextObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( PaintContextOwnerObject, PaintContextObject )
		// 構築関数
		PaintContextOwnerObject
			( const wchar_t * pwszType,
				SakuraGL::SGLPaintContextInterface * paint )
			: PaintContextObject( pwszType, paint ) {}
		// 消滅関数
		virtual ~PaintContextOwnerObject( void ) ;
	} ;

	struct	ECS_SGL_PAINT_PARAM
	{
		uint32_t				nFlags ;
		uint32_t				nReserved1 ;
		SakuraGL::SGLPoint		ptPaint ;
		uint32_t				nTransparency ;
		float32_t				zOrder ;
		SakuraGL::SGLPalette	rgbColorParam ;
		uint32_t				nReserved2 ;
		int64_t					pAffine ;
		int64_t					pVertices ;
		uint32_t				countVertex ;
	} ;

}


//////////////////////////////////////////////////////////////////////////////
// PaintContext スタブ
//////////////////////////////////////////////////////////////////////////////

// new SakuraGL::PaintContext
ECS_LIB_DECLARE_EXPORT_NEW_OBJECT(SakuraGL_PaintContext) ;

// PaintContext* PaintContext::NewContext
//		( SGLPaintContextType type = typePaintDefault )
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_PaintContext_NewContext) ;
// Image * PaintContext::GetTargetImage( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_PaintContext_GetTargetImage) ;
// Image * PaintContext::GetTargetZBuffer( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_PaintContext_GetTargetZBuffer) ;
// SGLError PaintContext::GetViewPort( SGLImageRect & rctView ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_PaintContext_GetViewPort) ;
// SGLError PaintContext::AttachTargetImage
//	( Image * pImage, Image * pZBuffer, const SGLImageRect * pView = NULL ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_PaintContext_AttachTargetImage) ;
// SGLError PaintContext::DetachTargetImage( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_PaintContext_DetachTargetImage) ;
// SGLError PaintContext::AppendTransformation
//	( const SGLAffine & af, unsigned int nTransparency ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_PaintContext_AppendTransformation) ;
// SGLError PaintContext::SetTransformation
//	( const SGLAffine & af, unsigned int nTransparency ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_PaintContext_SetTransformation) ;
// SGLError PaintContext::CurrentAffine( SGLAffine & af ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_PaintContext_CurrentAffine) ;
// SGLError PaintContext::CurrentTransparency( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_PaintContext_CurrentTransparency) ;
// SGLError PaintContext::PushTransformation( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_PaintContext_PushTransformation) ;
// SGLError PaintContext::PopTransformation( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_PaintContext_PopTransformation) ;
// SGLError PaintContext::ResetTransformation( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_PaintContext_ResetTransformation) ;
// void PaintContext::SetPaintFlags( int64_t nFlags ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_PaintContext_SetPaintFlags) ;
// int64_t PaintContext::GetPaintFlags( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_PaintContext_GetPaintFlags) ;
// SGLError PaintContext::FillClearTarget
//		( uint32_t argb = 0xff000000, int64_t flags = 0 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_PaintContext_FillClearTarget) ;
// SGLError PaintContext::FillRectangle
//	( int x, int y, int width, int height,
//			uint32_t argb, double z, uint32_t flags ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_PaintContext_FillRectangle) ;
// SGLError PaintContext::FillPolygon
//	( const S2DVector * vertices, size_t count,
//			uint32_t argb, double z, uint32_t flags ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_PaintContext_FillPolygon) ;
// SGLError PaintContext::DrawImage
//	( const SGLPaintParam & ppPaint,
//		Image * pSrcImage, const SGLImageRect * pSrcClip = NULL ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_PaintContext_DrawImage) ;
// SGLError PaintContext::DrawMesh
//	( const S2DVector * pDstMesh,
//		const S2DVector * pSrcMesh,
//		size_t widthMesh, size_t heightMesh,
//		const SGLPaintParam & ppPaint,
//		Image * pSrcImage, const SGLImageRect * pSrcClip = NULL ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_PaintContext_DrawMesh) ;
// SGLError PaintContext::Flush( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_PaintContext_Flush) ;
// SGLError PaintContext::Finish( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_PaintContext_Finish) ;


#endif
