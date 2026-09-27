
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/

#if	!defined(__GLSCS_SAKURA2_OBJECT_RENDER_H__)
#define	__GLSCS_SAKURA2_OBJECT_RENDER_H__

namespace	ECSSakura2
{
	//////////////////////////////////////////////////////////////////////////
	// Material オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	MaterialObject	: public ECSSakura2::Object, public SakuraGL::S3DMaterial
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2( MaterialObject, ECSSakura2::Object, S3DMaterial )
		// 実行時型名
		virtual const wchar_t * GetTypeName( void ) const ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// VertexBuffer オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	VertexBufferObject	: public ECSSakura2::Object, public SakuraGL::S3DVertexBuffer
	{
	public:
		struct	MESH_INFO
		{
			int64_t				pMaterial ;
			int64_t				typeMesh ;
			uint32_t			countPrimitive ;
			uint32_t			countVertex ;
			uint32_t			nReserved ;
			SakuraGL::S3DVector	vCenter ;
			float32_t			fpRadius ;
			int64_t				pvVertex ;
			int64_t				pvNormal ;
			int64_t				pvUVMap ;
			int64_t				pColor ;
			int64_t				pIndexedList ;
			int64_t				pSubIndexedList[countSubMesh] ;
			uint32_t			nSubPolyCount[countSubMesh] ;
			float32_t			fpSubMeshDensity ;
			uint32_t			nExAttrElements ;
			int64_t				pfpExAttrElements ;
		} ;
		// クラス情報
		ESL_DECLARE_CLASS_INFO2( VertexBufferObject, ECSSakura2::Object, S3DVertexBuffer )
		// 構築関数
		VertexBufferObject( void ) ;
		VertexBufferObject( SakuraGL::VertexBuffer * buffer ) ;
		// 実行時型名
		virtual const wchar_t * GetTypeName( void ) const ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 描画オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	RenderContextObject	: public ECSVolatileObject
	{
	protected:
		const wchar_t *							m_pwszType ;
		SakuraGL::S3DRenderContextInterface *	m_render ;
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RenderContextObject, ECSVolatileObject )
		// 構築関数
		RenderContextObject
			( const wchar_t * pwszType,
				SakuraGL::S3DRenderContextInterface * render ) ;
		// S3DRenderContextInterface 関連付け
		void AttachRenderInterface
				( SakuraGL::S3DRenderContextInterface * render ) ;
	public:
		// 実行時型名
		virtual const wchar_t * GetTypeName( void ) const ;
	} ;

	class	RenderContextOwnerObject	: public RenderContextObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RenderContextOwnerObject, RenderContextObject )
		// 構築関数
		RenderContextOwnerObject
			( const wchar_t * pwszType,
				SakuraGL::S3DRenderContextInterface * render )
			: RenderContextObject( pwszType, render ) {}
		// 消滅関数
		virtual ~RenderContextOwnerObject( void ) ;
	} ;


}


//////////////////////////////////////////////////////////////////////////////
// Material スタブ
//////////////////////////////////////////////////////////////////////////////

// new SakuraGL::Material
ECS_LIB_DECLARE_EXPORT_NEW_OBJECT(SakuraGL_Material) ;

// void Material::GetSurfaceAttribute( S3DSurfaceAttribute& attr ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Material_GetSurfaceAttribute) ;

// void Material::GetBackSurfaceAttribute( S3DSurfaceAttribute& attr ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Material_GetBackSurfaceAttribute) ;

// bool Material::IsEnabledBackSurfaceAttribute( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Material_IsEnabledBackSurfaceAttribute) ;

// void Material::SetSurfaceAttribute( const S3DSurfaceAttribute& attr ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Material_SetSurfaceAttribute) ;

// void Material::SetBackSurfaceAttribute( const S3DSurfaceAttribute& attr ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Material_SetBackSurfaceAttribute) ;

// void Material::EnableBackSurfaceAttribute( bool flagBack ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Material_EnableBackSurfaceAttribute) ;

// Image * Material::GetTexture( int iTexture = 0 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Material_GetTexture) ;

// Image * Material::GetBackTexture( int iTexture = 0 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Material_GetBackTexture) ;

// void Material::SetTexture
// ( Image * pImage, int iTexture = 0,
//		uint32_t nFlags = 0, double nApply = 1.0, double nParam1 = 0.0 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Material_SetTexture) ;

// void Material::SetBackTexture
// ( Image * pImage, int iTexture = 0,
//		uint32_t nFlags = 0, double nApply = 1.0, double nParam1 = 0.0 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Material_SetBackTexture) ;

// void Material::SetSubTextureZ( double zTexture ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Material_SetSubTextureZ) ;


//////////////////////////////////////////////////////////////////////////////
// VertexBuffer スタブ
//////////////////////////////////////////////////////////////////////////////

// new SakuraGL::VertexBuffer
ECS_LIB_DECLARE_EXPORT_NEW_OBJECT(SakuraGL_VertexBuffer) ;

// static VertexBuffer *
//	VertexBuffer::NewBuffer( SGLPaintContextType type = typePaintDefault ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_VertexBuffer_NewBuffer) ;

// SGLError VertexBuffer::AppendMatrixTransformation
//	( const S3DDMatrix& mat, const S3DDVector& pos,
//		const S3DColor * color = NULL, unsigned int nTransparency = 0 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_VertexBuffer_AddMatrixTransformation) ;

// SGLError VertexBuffer::SetMatrixTransformation
//	( const S3DDMatrix& mat, const S3DDVector& pos,
//		const S3DColor * color = NULL, unsigned int nTransparency = 0 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_VertexBuffer_SetMatrixTransformation) ;

// SGLError VertexBuffer::GetMatrixTransformation
//	( S3DDMatrix& mat, S3DDVector& pos,
//		S3DColor * color = NULL, unsigned int * pTransparency = 0 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_VertexBuffer_GetMatrixTransformation) ;

// SGLError VertexBuffer::PushTransformation( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_VertexBuffer_PushTransformation) ;

// SGLError VertexBuffer::PopTransformation( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_VertexBuffer_PopTransformation) ;

// SGLError VertexBuffer::ResetTransformation( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_VertexBuffer_ResetTransformation) ;

// SGLError VertexBuffer::AddIndexedTriangleList
//	( Material * pMaterial, uint32_t nFlags,
//		size_t countPolygon, size_t countVertex,
//		const S3DVector4 * pvVertex, const S3DVector4 * pvNormal,
//		const S2DVector * pvUVMap, const S3DColor * pColor,
//		const uint32_t * pIndexedList ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_VertexBuffer_AddIndexedTriangleList) ;

// SGLError VertexBuffer::AddTriangleStrip
//	( Material * pMaterial, uint32_t nFlags, size_t countTriangleStrip,
//		const S3DVector4 * pvVertex, const S3DVector4 * pvNormal,
//		const S2DVector * pvUVMap, const S3DColor * pColor ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_VertexBuffer_AddTriangleStrip) ;

// SGLError VertexBuffer::AddIndexedPrimitiveList
//	( S3DMaterial * pMaterial, uint32_t nFlags,
//		S3DPrimitiveType typePrimitive,
//		size_t countIndex, size_t countVertex,
//		const S3DVector4 * pvVertex, const S3DVector4 * pvNormal,
//		const S2DVector * pvUVMap, const S3DColor * pColor,
//		const uint32_t * pIndexedList ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_VertexBuffer_AddIndexedPrimitiveList) ;

// SGLError VertexBuffer::AddVertexBuffer
//	( Material * pMaterial, uint32_t nFlags,
//		VertexBuffer * pBuffer, size_t iFirst = 0, ssize_t iEnd = -1 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_VertexBuffer_AddVertexBuffer) ;

// SGLError VertexBuffer::Flush( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_VertexBuffer_Flush) ;

// size_t VertexBuffer::GetMeshCount( void ) const ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_VertexBuffer_GetMeshCount) ;

// SGLError VertexBuffer::GetMeshInfoAt
//	( MeshInfo& info, size_t iMesh, size_t nCopyVertices,
//			size_t iFirstVertex = 0, uint32_t nFlags = 0 ) const ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_VertexBuffer_GetMeshInfoAt) ;

// SGLError VertexBuffer::UpdateIndexedTriangleList
//	( size_t iMesh, uint32_t nFlags,
//		size_t countPolygon, size_t countVertex,
//		const S3DVector4 * pvVertex, const S3DVector4 * pvNormal,
//		const S2DVector * pvUVMap, const S3DColor * pColor,
//		const uint32_t * pIndexedList ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_VertexBuffer_UpdateIndexedTriangleList) ;

// SGLError VertexBuffer::UpdateTriangleStrip
//	( size_t iMesh, uint32_t nFlags, size_t countTriangleStrip,
//		const S3DVector4 * pvVertex, const S3DVector4 * pvNormal,
//		const S2DVector * pvUVMap, const S3DColor * pColor ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_VertexBuffer_UpdateTriangleStrip) ;

// SGLError VertexBuffer::UpdateIndexedPrimitiveList
//	( size_t iMesh, uint32_t nFlags,
//		size_t countIndex, size_t countVertex,
//		const S3DVector4 * pvVertex, const S3DVector4 * pvNormal,
//		const S2DVector * pvUVMap, const S3DColor * pColor,
//		const uint32_t * pIndexedList ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_VertexBuffer_UpdateIndexedPrimitiveList) ;

// SGLError VertexBuffer::UpdateSubIndexedTriangleList
//	( size_t iMesh, size_t iSubMesh, uint32_t nFlags,
//		size_t countPolygon, const uint32_t * pIndexedList ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_VertexBuffer_UpdateSubIndexedTriangleList) ;

// SGLError VertexBuffer::SetSubMeshDensity
//	( size_t iMesh, float32_t fpDensity, ssize_t iSelector = -1 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_VertexBuffer_SetSubMeshDensity) ;

// SGLError VertexBuffer::SetExtendVertexAttribute
//	( size_t iMesh, size_t countElements,
//			size_t countVertex, const float32_t * pfpAttrElements ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_VertexBuffer_SetExtendVertexAttribute) ;

// SGLError VertexBuffer::SetBoneWeightMap
//	( size_t iMesh, size_t nCount, const float32_t ** ppWeightMaps ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_VertexBuffer_SetBoneWeightMap) ;

// SGLError VertexBuffer::SetBoneMatrix
//	( size_t iMesh, size_t nCount,
//		const S3DMatrix * pMatrix, const S3DVector * pTrans ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_VertexBuffer_SetBoneMatrix) ;

// SGLError VertexBuffer::AllocateMorphing( size_t iMesh, size_t nCount ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_VertexBuffer_AllocateMorphing) ;

// SGLError VertexBuffer::SetMorphingTargetMesh
//	( size_t iMesh, size_t iMorph, size_t countVertex,
//		const S3DVector4 * pvVertex, const S3DVector4 * pvNormal,
//		const S2DVector * pvUVMap, const S3DColor * pColor ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_VertexBuffer_SetMorphingTargetMesh) ;

// SGLError VertexBuffer::SetMorphingTargetWeight
//	( size_t iMesh, size_t iMorph,
//		size_t countVertex, const float32_t * pfpWeight )
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_VertexBuffer_SetMorphingTargetWeight) ;

// SGLError VertexBuffer::SetMorphingApplication
//	( size_t iMesh, const ssize_t * pTargetMesh,
//		const float32_t * pApplication, size_t nTargetMeshCount ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_VertexBuffer_SetMorphingApplication) ;

// SGLError VertexBuffer::GetMorphingApplication
//	( size_t iMesh, ssize_t& iTargetMesh,
//		float32_t& fpApplication, size_t iTargetMeshIndex ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_VertexBuffer_GetMorphingApplication) ;

// SGLError VertexBuffer::EnableToRenderMesh
//	( size_t iFirst = 0, ssize_t iEnd = -1, bool fEnable = true )
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_VertexBuffer_EnableToRenderMesh) ;

// bool VertexBuffer::IsEnabledToRenderMesh( size_t iMesh )
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_VertexBuffer_IsEnabledToRenderMesh) ;

// SGLError VertexBuffer::RenderBufferTo
//	( RenderContext * render,
//		uint64_t flagsExclusion = 0, size_t iFrist = 0, ssize_t iEnd = -1,
//		size_t nInstancing = 0,
//		const S4DMatrix * pmatInstancing = NULL,
//		const S3DColor * pColorInstancing = NULL ) const ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_VertexBuffer_RenderBufferTo) ;

// void VertexBuffer::ClearBuffer( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_VertexBuffer_ClearBuffer) ;

// void VertexBuffer::SetBufferUnitSize( size_t nBytes ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_VertexBuffer_SetBufferUnitSize) ;

// double VertexBuffer::GetCircumscribedSphere( S3DVector& vCenter ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_VertexBuffer_GetCircumscribedSphere) ;


//////////////////////////////////////////////////////////////////////////////
// RenderContext スタブ
//////////////////////////////////////////////////////////////////////////////

// new SakuraGL::RenderContext
ECS_LIB_DECLARE_EXPORT_NEW_OBJECT(SakuraGL_RenderContext) ;

// new SakuraGL::HybridRenderContext
ECS_LIB_DECLARE_EXPORT_NEW_OBJECT(SakuraGL_HybridRenderContext) ;

// static RenderContext * RenderContext::NewContext
//	( SGLPaintContextType type = typePaintDefault ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_RenderContext_NewContext) ;

// SGLError RenderContext::AppendMatrixTransformation
//	( const S3DDMatrix& mat, const S3DDVector& pos,
//		const S3DColor * color = NULL, unsigned int nTransparency = 0 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_RenderContext_AppendMatrixTransformation) ;

// SGLError RenderContext::SetMatrixTransformation
//	( const S3DDMatrix& mat, const S3DDVector& pos,
//		const S3DColor * color = NULL, unsigned int nTransparency = 0 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_RenderContext_SetMatrixTransformation) ;

// SGLError RenderContext::GetMatrixTransformation
//	( S3DDMatrix& mat, S3DDVector& pos,
//		S3DColor * color = NULL, unsigned int * pTransparency = 0 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_RenderContext_GetMatrixTransformation) ;

// SGLError RenderContext::SetProjectionScreen( const S3DVector& vScreen, double zScale = 1.0, double fpPixelAspect = 1.0 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_RenderContext_SetProjectionScreen) ;

// SGLError RenderContext::GetProjectionScreen( S3DVector& vScreen, double& zScale, double& fpPixelAspect ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_RenderContext_GetProjectionScreen) ;

// bool RenderContext::GetPerspectiveMatrix( S4DMatrix& matPars ) const ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_RenderContext_GetPerspectiveMatrix) ;

// void RenderContext::SetPerspectiveMatrix
//		( StereoViewIndex sviView,
//			const S4DMatrix& matPers, bool fPersMatrix = true ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_RenderContext_SetPerspectiveMatrix) ;

// void RenderContext::EnablePerspectiveMatrix( bool fPersMatrix ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_RenderContext_EnablePerspectiveMatrix) ;

// void RenderContext::SetCamera( const S3DDMatrix& matCamera, const S3DDVector& posCamera ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_RenderContext_SetCamera) ;

// void RenderContext::GetCamera( S3DDMatrix& matCamera, S3DDVector& posCamera ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_RenderContext_GetCamera) ;

// void RenderContext::SetParallax( double xParallax, double zFocusRate ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_RenderContext_SetParallax) ;

// double RenderContext::GetParallax( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_RenderContext_GetParallax) ;

// void RenderContext::SetZClipRange( double zMin, double zMax ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_RenderContext_SetZClipRange) ;

// void RenderContext::SetLightEntries( const S3DLightEntry* pLights, size_t countLight ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_RenderContext_SetLightEntries) ;

// void RenderContext::SetShadowMap
//	( uint32_t idLight, Image* pShadowMap,
//    const S3DShadowMapInfo& infShadowMap, Image* pShadowMapColor ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_RenderContext_SetShadowMap) ;

// void RenderContext::SetFog( uint32_t rgbFog, double zFogNear, double zFogFar ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_RenderContext_SetFog) ;

// void RenderContext::EnableFog( bool fFog ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_RenderContext_EnableFog) ;

// void RenderContext::SetShadingFlag( uint64_t nShadingMethod ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_RenderContext_SetShadingFlag) ;

// uint64_t RenderContext::GetShadingFlag( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_RenderContext_GetShadingFlag) ;

// void RenderContext::SetRayTracingParameter( const S3DRenderRayTracingParam& rrtp ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_RenderContext_SetRayTracingParameter) ;

// void SetEnvironmentMappingImage( Image * pImage, uint32_t nFlags ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_RenderContext_SetEnvironmentMappingImage) ;

// void SetEnvironmentMappingMatrix( const S3DMatrix& matMapping ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_RenderContext_SetEnvironmentMappingMatrix) ;

// void SetOffsetBorderColor( uint32_t rgbBorder ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_RenderContext_SetOffsetBorderColor) ;

// void SetOffsetBorderCoefficient( float32_t a, float32_t b ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_RenderContext_SetOffsetBorderCoefficient) ;

// SGLError SetOptionalFeature( FeatureType feature, int32_t nParam1, void * pParam2 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_RenderContext_SetOptionalFeature) ;

// void GetRenderingCapacity( S3DRenderingCapacity& caps ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_RenderContext_GetRenderingCapacity) ;

// StereoViewIndex CurrentParallaxView( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_RenderContext_CurrentParallaxView) ;

// SGLError RenderContext::SelectParallaxView( StereoViewIndex sviView ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_RenderContext_SelectParallaxView) ;

// SGLError RenderContext::SetRenderingBufferSize( uint32_t countVertex ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_RenderContext_SetRenderingBufferSize) ;

// SGLError RenderContext::Begin3DRenderer( uint64_t nFlags ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_RenderContext_Begin3DRenderer) ;

// SGLError RenderContext::End3DRenderer( uint64_t nFlags ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_RenderContext_End3DRenderer) ;

// SGLError RenderContext::AddIndexedTriangleList
//	( Material * pMaterial, uint32_t nFlags,
//		size_t countPolygon, size_t countVertex,
//		const S3DVector4 * pvVertex, const S3DVector4 * pvNormal,
//		const S2DVector * pvUVMap, const S3DColor * pColor,
//		const uint32_t * pIndexedList ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_RenderContext_AddIndexedTriangleList) ;

// SGLError RenderContext::AddTriangleStrip
//	( Material * pMaterial, uint32_t nFlags, size_t countTriangleStrip,
//		const S3DVector4 * pvVertex, const S3DVector4 * pvNormal,
//		const S2DVector * pvUVMap, const S3DColor * pColor ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_RenderContext_AddTriangleStrip) ;

// SGLError RenderContext::AddIndexedPrimitiveList
//	( S3DMaterial * pMaterial, uint32_t nFlags,
//		S3DPrimitiveType typePrimitive,
//		size_t countIndex, size_t countVertex,
//		const S3DVector4 * pvVertex, const S3DVector4 * pvNormal,
//		const S2DVector * pvUVMap, const S3DColor * pColor,
//		const uint32_t * pIndexedList ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_RenderContext_AddIndexedPrimitiveList) ;

// SGLError RenderContext::AddVertexBuffer
//	( Material * pMaterial, uint32_t nFlags,
//		VertexBuffer * pBuffer, size_t iFirst = 0, ssize_t iEnd = -1,
//		size_t nInstancing = 0,
//		const S4DMatrix * pmatInstancing = NULL,
//		const S3DColor * pColorInstancing = NULL )
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_RenderContext_AddVertexBuffer) ;


#endif
