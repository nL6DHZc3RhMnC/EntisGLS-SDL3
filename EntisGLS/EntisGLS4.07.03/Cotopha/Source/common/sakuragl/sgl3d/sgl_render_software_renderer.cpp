
#include <sakuragl/sakuragl.h>
#include <sakuragl/sgl_window.h>
#include <sakuragl/sgl3d_image.h>
#include <sakuragl/sgl3d/sgl_render_software_renderer.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// ソフトウェア・レンダラ・デバイス
//////////////////////////////////////////////////////////////////////////////

SGLSoftwareRenderDevice *	SGLSoftwareRenderDevice::m_pChainFirst = NULL ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSoftwareRenderDevice, S3DRenderDevice )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSoftwareRenderDevice::SGLSoftwareRenderDevice( void )
{
	m_pChainNext = NULL ;
	AddToChain() ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSoftwareRenderDevice::~SGLSoftwareRenderDevice( void )
{
	DetachFromChain() ;
}

// レンダリングスレッドか判定
//////////////////////////////////////////////////////////////////////////////
bool SGLSoftwareRenderDevice::IsOnRenderThread( void )
{
	return	true ;
}

// レンダリングスレッドで実行する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSoftwareRenderDevice::Procedure
		( SSystem::SProcedure* pProc, ProcedurePriority priority )
{
	pProc->Prepare() ;
	pProc->Run() ;
	pProc->Finalize() ;
	return	sglErrSuccess ;
}

// レンダリングスレッドでの遅延実行が全て完了するまで待機
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSoftwareRenderDevice::WaitUntilAsyncAllProcedures( int64_t msecTimeout )
{
	return	sglErrSuccess ;
}

// レンダラ生成
//////////////////////////////////////////////////////////////////////////////
S3DRenderContextInterface * SGLSoftwareRenderDevice::NewRenderer( void ) const
{
	return	new S3DSoftwareBufferedRenderer ;
}

// レンダリングデバイス用の画像インスタンスを生成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSoftwareRenderDevice::CommitDeviceImage
	( SGLImageObject * pImage, int64_t msecTimeout )
{
	return	sglErrSuccess ;
}

// レンダリングデバイス用の VBO インスタンスを生成／更新
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSoftwareRenderDevice::CommitDeviceVertexBuffer
	( S3DVertexBufferInterface * pVertexBuf, int64_t msecTimeout )
{
	return	sglErrSuccess ;
}

// レンダリングデバイス用の画像インスタンスを解放
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSoftwareRenderDevice::ReleaseDeviceImage
	( SGLImageObject * pImage, int64_t msecTimeout )
{
	return	sglErrSuccess ;
}

// レンダリングデバイス用の VBO インスタンスを解放
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSoftwareRenderDevice::ReleaseDeviceVertexBuffer
	( S3DVertexBufferInterface * pVertexBuf, int64_t msecTimeout )
{
	return	sglErrSuccess ;
}

// デバイス機能
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSoftwareRenderDevice::GetDeviceFeatures( Features& features )
{
	eslFillMemory( &features, 0, sizeof(S3DRenderDevice::Features) ) ;
	//
	features.flagsFeatures[0] =
			feature0_TextureNonPowerOf2 | feature0_DepthTexture ;
	features.maxTextureSize = 0x7FFF ;
	features.maxMultiTextureUnits = 1 ;
	//
	return	sglErrSuccess ;
}

// カスタムシェーダー・オブジェクト生成
//////////////////////////////////////////////////////////////////////////////
S3DCustomShader * SGLSoftwareRenderDevice::NewCustomShader( ShaderProgramType type )
{
	return	NULL ;
}

// 定義済み標準シェーダー生成／コンパイル／取得
//////////////////////////////////////////////////////////////////////////////
S3DCustomShader *
		SGLSoftwareRenderDevice::GetDefaultShaderProgramAs( const wchar_t * pwszID )
{
	return	NULL ;
}

// カスタムシェーダーコンパイル
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSoftwareRenderDevice::CompileCustomShader
		( S3DCustomShader * pShader,
			const ShaderSourceInfo& src,
			S3DCustomShader::CompileListener * pListener )
{
	return	sglErrFailed ;
}

// カスタムシェーダー読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSoftwareRenderDevice::LoadCustomShader
		( S3DCustomShader * pShader,
			const S3DShaderBinary& bin,
			S3DCustomShader::CompileListener * pListener )
{
	return	sglErrFailed ;
}

// カスタムシェーダー・バイナリの保存
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSoftwareRenderDevice::SaveCustomShaderBinary
	( S3DShaderBinary& bin, S3DCustomShader * pShader )
{
	return	sglErrFailed ;
}

// SGLSoftwareRenderDevice チェーン
//////////////////////////////////////////////////////////////////////////////
void SGLSoftwareRenderDevice::AddToChain( void )
{
	ESLAssert( m_pChainNext == NULL ) ;
	QuickLock() ;
	SGLSoftwareRenderDevice *	pLast = NULL ;
	SGLSoftwareRenderDevice *	pNext = m_pChainFirst ;
	while ( pNext != NULL )
	{
		if ( pNext == this )
		{
			QuickUnlock() ;
			return ;
		}
		pLast = pNext ;
		pNext = pNext->m_pChainNext ;
	}
	if ( pLast != NULL )
	{
		pLast->m_pChainNext = this ;
	}
	else
	{
		m_pChainFirst = this ;
	}
	QuickUnlock() ;
}

void SGLSoftwareRenderDevice::DetachFromChain( void )
{
	QuickLock() ;
	SGLSoftwareRenderDevice *	pLast = NULL ;
	SGLSoftwareRenderDevice *	pNext = m_pChainFirst ;
	while ( pNext != NULL )
	{
		if ( pNext == this )
		{
			pNext = m_pChainNext ;
			m_pChainNext = NULL ;
			//
			if ( pLast != NULL )
			{
				pLast->m_pChainNext = pNext ;
			}
			else
			{
				m_pChainFirst = pNext ;
			}
		}
		else
		{
			pLast = pNext ;
			pNext = pNext->m_pChainNext ;
		}
	}
	QuickUnlock() ;
	ESLAssert( m_pChainNext == NULL ) ;
}

// デフォルトの SGLSoftwareRenderDevice を取得する
//////////////////////////////////////////////////////////////////////////////
SGLSoftwareRenderDevice * SGLSoftwareRenderDevice::GetDefault( void )
{
	return	m_pChainFirst ;
}

// パフォーマンスログ・フレーム開始
//////////////////////////////////////////////////////////////////////////////
void SGLSoftwareRenderDevice::BeginFramePerformanceLog( void )
{
}

// パフォーマンスログ・フレーム終了
//////////////////////////////////////////////////////////////////////////////
void SGLSoftwareRenderDevice::EndFramePerformanceLog( void )
{
}

// パフォーマンスログ・デバッグ出力
//////////////////////////////////////////////////////////////////////////////
void SGLSoftwareRenderDevice::DebugTracePerformanceLog( PerformanceLogInfo * pli )
{
}



//////////////////////////////////////////////////////////////////////////////
// ソフトウェア・レンダラ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
( SakuraGL::S3DSoftwareRenderer, S3DRenderParameterContext, SGLPaintBuffer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DSoftwareRenderer::S3DSoftwareRenderer( void )
{
	m_flagBegin3D = false ;
	m_pFirstMesh = NULL ;
	m_pLastMesh = NULL ;
	m_nQueueMesh = 0 ;
	m_sigQueue.Initialize( false ) ;
	//
	m_pRenderingMesh = NULL ;
	m_iPolygon = 0 ;
	m_iNextPolygon = 0 ;
	m_pLastTexture = NULL ;
	//
	m_flagAbortThread = false ;
	m_nRunningThreads = 0 ;
	m_sigEndThread.Initialize( false ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DSoftwareRenderer::~S3DSoftwareRenderer( void )
{
	SyncRender() ;
	ExitRenderThread() ;
}

// 描画先取得
//////////////////////////////////////////////////////////////////////////////
SGLImageObject * S3DSoftwareRenderer::GetTargetImage( void )
{
	return	S3DRenderParameterContext::GetTargetImage() ;
}

SGLImageObject * S3DSoftwareRenderer::GetTargetZBuffer( void )
{
	return	S3DRenderParameterContext::GetTargetZBuffer() ;
}

// ビューポート取得
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSoftwareRenderer::GetViewPort( SGLImageRect & rctView ) const
{
	return	S3DRenderParameterContext::GetViewPort( rctView ) ;
}

// 描画先設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSoftwareRenderer::AttachTargetImage
	( SGLImageObject * pImage,
			SGLImageObject * pZBuffer, const SGLImageRect * pView )
{
	SyncRender() ;
	ExitRenderThread() ;
	//
	// 描画先設定
	//
	SGLError	err ;
	S3DRenderParameterContext::AttachTargetImage( pImage, pZBuffer, pView ) ;
	err = SGLPaintBuffer::AttachTargetImage( pImage, pZBuffer, pView ) ;
	//
	// リージョンバッファ確保
	//
	const size_t	nRegionBytes =
						sizeof(SGLRegion)
							+ SGLPaintBuffer::m_rctView.h
										* sizeof(SGLRegionLine) ;
	for ( size_t i = 0; i < 2; i ++ )
	{
		m_bufPolygon[i].m_pbufRegion =
			(SGLRegion*) m_bufPolygon[i].m_bufRegion.GetArray( nRegionBytes ) ;
	}
	//
	// 中間描画バッファ確保
	//
	size_t	nCount = (size_t) SSystem::g_cpuLogicalCount ;
	m_arrRasterizeBuf.SetLength( nCount ) ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		RasterizeBuffer *	prb = m_arrRasterizeBuf.GetAt( i ) ;
		if ( prb == NULL )
		{
			prb = new RasterizeBuffer ;
			m_arrRasterizeBuf.SetAt( i, prb ) ;
		}
		prb->m_pTempSrc =
			prb->m_bufTempSrc.GetArray
				( (size_t) SGLPaintBuffer::m_rctView.w ) ;
		prb->m_pTempColor =
			prb->m_bufTempColor.GetArray
				( (size_t) SGLPaintBuffer::m_rctView.w ) ;
		prb->m_pTempZ =
			prb->m_bufTempZ.GetArray
				( (size_t) SGLPaintBuffer::m_rctView.w ) ;
		//
		prb->m_pRenderer = this ;
		prb->m_iThread = i ;
	}
	return	err ;
}

// 描画先解除
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSoftwareRenderer::DetachTargetImage( void )
{
	SyncRender() ;
	S3DRenderParameterContext::DetachTargetImage() ;
	return	SGLPaintBuffer::DetachTargetImage() ;
}

// 描画座標空間設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSoftwareRenderer::AppendTransformation
	( const SGLAffine & af, unsigned int nTransparency )
{
	SGLPaintBuffer::AppendTransformation( af, nTransparency ) ;
	return	S3DRenderParameterContext::AppendTransformation( af, nTransparency ) ;
}

SGLError S3DSoftwareRenderer::SetTransformation
	( const SGLAffine & af, unsigned int nTransparency )
{
	SGLPaintBuffer::SetTransformation( af, nTransparency ) ;
	return	S3DRenderParameterContext::SetTransformation( af, nTransparency ) ;
}

SGLError S3DSoftwareRenderer::CurrentAffine( SGLAffine & af )
{
	return	SGLPaintBuffer::CurrentAffine( af ) ;
}

unsigned int S3DSoftwareRenderer::CurrentTransparency( void )
{
	return	S3DRenderParameterContext::CurrentTransparency() ;
}

SGLError S3DSoftwareRenderer::PushTransformation( void )
{
	SGLPaintBuffer::PushTransformation() ;
	return	S3DRenderParameterContext::PushTransformation() ;
}

SGLError S3DSoftwareRenderer::PopTransformation( void )
{
	SGLPaintBuffer::PopTransformation() ;
	return	S3DRenderParameterContext::PopTransformation() ;
}

SGLError S3DSoftwareRenderer::ResetTransformation( void )
{
	SGLPaintBuffer::ResetTransformation() ;
	return	S3DRenderParameterContext::ResetTransformation() ;
}

// 描画デフォルトフラグ
//////////////////////////////////////////////////////////////////////////////
void S3DSoftwareRenderer::SetPaintFlags( int64_t nFlags )
{
	SGLPaintBuffer::SetPaintFlags( nFlags ) ;
	S3DRenderParameterContext::SetPaintFlags( nFlags ) ;
}

int64_t S3DSoftwareRenderer::GetPaintFlags( void )
{
	return	SGLPaintBuffer::GetPaintFlags() ;
}

// 描画先クリア
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSoftwareRenderer::FillClearTarget( uint32_t argb, int64_t flags )
{
	SyncRender() ;
	return	SGLPaintBuffer::FillClearTarget( argb, flags ) ;
}

// 形状描画
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSoftwareRenderer::FillRectangle
	( int x, int y, int width, int height,
		uint32_t argb, double z, uint32_t flags )
{
	SyncRender() ;
	return	SGLPaintBuffer::FillRectangle
				( x, y, width, height, argb, z, flags ) ;
}

SGLError S3DSoftwareRenderer::FillPolygon
	( const S2DVector * vertices, size_t count,
		uint32_t argb, double z, uint32_t flags )
{
	SyncRender() ;
	return	SGLPaintBuffer::FillPolygon
				( vertices, count, argb, z, flags ) ;
}

// 画像描画
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSoftwareRenderer::DrawImage
	( const SGLPaintParam & ppPaint,
		SGLImageObject * pSrcImage,
		const SGLImageRect * pSrcClip )
{
	SyncRender() ;
	return	SGLPaintBuffer::DrawImage
				( ppPaint, pSrcImage, pSrcClip ) ;
}

// ２Ｄメッシュ描画
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSoftwareRenderer::DrawMesh
	( const S2DVector * pDstMesh,
		const S2DVector * pSrcMesh,
		size_t widthMesh, size_t heightMesh,
		const SGLPaintParam & ppPaint,
		SGLImageObject * pSrcImage,
		const SGLImageRect * pSrcClip )
{
	SyncRender() ;
	return	SGLPaintBuffer::DrawMesh
				( pDstMesh, pSrcMesh,
					widthMesh, heightMesh, ppPaint, pSrcImage, pSrcClip ) ;
}

// 描画の確定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSoftwareRenderer::Flush( void )
{
	SyncRender() ;
	return	SGLPaintBuffer::Flush() ;
}

SGLError S3DSoftwareRenderer::Finish( void )
{
	SGLError	err ;
	SyncRender() ;
	err = SGLPaintBuffer::Flush() ;
	//
	for ( ; ; )
	{
		m_csQueue.Lock() ;
		if ( m_nRunningThreads == 0 )
		{
			if ( (m_pFirstMesh == NULL) && (m_pRenderingMesh == NULL) )
			{
				m_bufRender.FreeAll() ;
				m_sigQueue.ResetSignal() ;
			}
			m_csQueue.Unlock() ;
			break ;
		}
//		m_sigQueue.SetSignal() ;
		m_sigEndThread.ResetSignal() ;
		m_csQueue.Unlock() ;
		m_sigEndThread.Wait( 1 ) ;
	}
	return	err ;
}

// バッファ複製
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSoftwareRenderer::CopyBufferFrom
	( S3DRenderContextInterface& renderSrc, uint32_t nFlags,
		int xDst, int yDst, const SGLImageRect * pSrcRect )
{
	renderSrc.Finish() ;
	SyncRender() ;
	//
	if ( nFlags == 0 )
	{
		nFlags = copyBufferColor | copyBufferDepth ;
	}
	SGLError	err = sglErrInvalidParam ;
	if ( nFlags & copyBufferColor )
	{
		SGLImageObject *	pDstImage = GetTargetImage() ;
		SGLImageObject *	pSrcImage = renderSrc.GetTargetImage() ;
		if ( pDstImage && pSrcImage )
		{
			err = pDstImage->CopyImage( pSrcImage, xDst, yDst, pSrcRect ) ;
		}
	}
	if ( nFlags & copyBufferDepth )
	{
		SGLImageObject *	pDstImage = GetTargetZBuffer() ;
		SGLImageObject *	pSrcImage = renderSrc.GetTargetZBuffer() ;
		if ( pDstImage && pSrcImage )
		{
			err = pDstImage->CopyImage( pSrcImage, xDst, yDst, pSrcRect ) ;
		}
	}
	return	err ;
}

// ポリゴンリストをレンダリングバッファに追加
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSoftwareRenderer::AddIndexedTriangleList
	( S3DMaterial * pMaterial, uint32_t nFlags,
		size_t countPolygon, size_t countVertex,
		const S3DVector4 * pvVertex,
		const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap,
		const S3DColor * pColor,
		const uint32_t * pIndexedList )
{
	//
	// メッシュエントリ開始
	//
	MeshEntry *	pMesh ;
	m_csQueue.Lock() ;
	pMesh = (MeshEntry*) m_bufRender.Allocate( sizeof(MeshEntry) ) ;
	pMesh->pChainNext = NULL ;
	pMesh->countPolygon = countPolygon ;
	pMesh->pMaterial = pMaterial ;
	pMesh->nAlpha = (uint32_t) esl_clampi
					( 0x100 - (int) CurrentTransparency(), 0, 0x100 ) ;
	pMesh->pvVertex = (S3DVector4*)
		m_bufRender.Allocate( countVertex * sizeof(S3DVector4) ) ;
	pMesh->pvNormal = (S3DVector4*)
		m_bufRender.Allocate( countVertex * sizeof(S3DVector4) ) ;
	pMesh->pvUVMap = NULL ;
	pMesh->pColor = NULL ;
	if ( pvUVMap != NULL )
	{
		pMesh->pvUVMap = (S2DVector*)
			m_bufRender.Allocate( countVertex * sizeof(S2DVector) ) ;
	}
	if ( (pColor != NULL)
		|| !pMaterial->m_attrSurface.colorBase.IsTransparent()
		|| ((pMaterial->m_attrSurface.flagsShading
					& shadingMethodMask) != shadingMethodNothing) )
	{
		pMesh->pColor = (S3DColor*)
			m_bufRender.Allocate( countVertex * sizeof(S3DColor) ) ;
	}
	pMesh->pIndexedList = (uint32_t*)
		m_bufRender.Allocate( countPolygon * (sizeof(uint32_t) * 3) ) ;
	m_nQueueMesh ++ ;
	m_csQueue.Unlock() ;
	//
	// 頂点座標変換
	//
	S3DDMatrix	matdView ;
	S3DDVector	vdView ;
	GetTransformMatrix( matdView, vdView ) ;
	//
	S3DMatrix	matView = matdView ;
	S3DVector	vView = vdView ;
	matView.RevolveVectors
		( pMesh->pvVertex, pvVertex, countVertex, vView ) ;
	//
	// 法線変換
	//
	if ( pvNormal != NULL )
	{
		S3DVector	vZero( 0, 0, 0 ) ;
		matView.RevolveVectors
			( pMesh->pvNormal, pvNormal, countVertex, vZero ) ;
	}
	else
	{
		m_tnbNormal.SetForIndexedTriangleList
			( countPolygon, countVertex,
				pMesh->pvVertex, pvUVMap, pIndexedList ) ;
		eslMoveMemory
			( pMesh->pvNormal,
				m_tnbNormal.GetNormalBuffer(),
				countVertex * sizeof(S3DVector4) ) ;
	}
	//
	// UV マップ
	//
	if ( pvUVMap != NULL )
	{
		eslMoveMemory
			( pMesh->pvUVMap, pvUVMap, countVertex * sizeof(S2DVector) ) ;
	}
	//
	// シェーディング
	//
	if ( pMesh->pColor != NULL )
	{
		m_shader.ShadeVertexColors
			( pMesh->pColor, pMaterial->m_attrSurface,
				pMesh->pvNormal, pMesh->pvVertex, pColor, countVertex ) ;
	}
	//
	// 三角頂点指標リスト
	//
	eslMoveMemory
		( pMesh->pIndexedList,
			pIndexedList, countPolygon * (sizeof(uint32_t) * 3) ) ;
	//
	// 待ち行列へ追加
	//
	m_csQueue.Lock() ;
	if ( m_pLastMesh != NULL )
	{
		ESLAssert( m_pLastMesh->pChainNext == NULL ) ;
		m_pLastMesh->pChainNext = pMesh ;
	}
	else
	{
		ESLAssert( m_pFirstMesh == NULL ) ;
		m_pFirstMesh = pMesh ;
	}
	m_pLastMesh = pMesh ;
	m_csQueue.Unlock() ;
	//
	// レンダリング実行
	//
	DoRender() ;
	//
	m_flagUpdateTarget = true ;
	m_flagUpdateZBuf = true ;
	return	sglErrFailed ;
}

// トライアングルストリップをレンダリングバッファに追加
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSoftwareRenderer::AddTriangleStrip
	( S3DMaterial * pMaterial, uint32_t nFlags,
		size_t countTriangleStrip,
		const S3DVector4 * pvVertex,
		const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap, const S3DColor * pColor )
{
	return	S3DSoftwareRenderer::AddIndexedTriangleList
		( pMaterial, nFlags,
			countTriangleStrip, countTriangleStrip + 2,
			pvVertex, pvNormal, pvUVMap, pColor,
			m_titsIndex.MakeIndexList(countTriangleStrip) ) ;
}

// プリミティブリストをレンダリングバッファに追加
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSoftwareRenderer::AddIndexedPrimitiveList
	( S3DMaterial * pMaterial, uint32_t nFlags,
		S3DPrimitiveType typePrimitive,
		size_t countIndex, size_t countVertex,
		const S3DVector4 * pvVertex,
		const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap,
		const S3DColor * pColor,
		const uint32_t * pIndexedList )
{
	if ( typePrimitive == primitiveTriangle )
	{
		return	AddIndexedTriangleList
			( pMaterial, nFlags, countIndex / 3, countVertex,
				pvVertex, pvNormal, pvUVMap, pColor, pIndexedList ) ;
	}
	else if ( typePrimitive == primitiveTriangle )
	{
		return	AddTriangleStrip
			( pMaterial, nFlags, countVertex - 2,
				pvVertex, pvNormal, pvUVMap, pColor ) ;
	}
	return	sglErrFailed ;
}

// 頂点バッファの内容を描画
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSoftwareRenderer::AddVertexBuffer
	( S3DMaterial * pMaterial, uint32_t nFlags,
		S3DVertexBufferInterface * pBuffer,
		size_t iFirst, ssize_t iEnd,
		size_t nInstancing,
		const S4DMatrix * pmatInstancing,
		const S3DColor * pColorInstancing )
{
	S3DRenderBuffer *	prb = ESLTypeCast<S3DRenderBuffer>( pBuffer ) ;
	if ( prb == nullptr )
	{
		S3DVertexBuffer *	pvb = ESLTypeCast<S3DVertexBuffer>( pBuffer ) ;
		if ( pvb != nullptr )
		{
			prb = ESLTypeCast<S3DRenderBuffer>( pvb->GetVertexBuffer() ) ;
		}
	}
	if ( prb != nullptr )
	{
		return	prb->RenderTemporaryBufferTo
					( this, 0, iFirst, iEnd,
						nInstancing, pmatInstancing, pColorInstancing ) ;
	}
	return	pBuffer->RenderBufferTo
				( this, 0, iFirst, iEnd,
					nInstancing, pmatInstancing, pColorInstancing ) ;
}

// カメラ設定
//////////////////////////////////////////////////////////////////////////////
void S3DSoftwareRenderer::SetCamera
	( const S3DDMatrix& matCamera, const S3DDVector& posCamera )
{
	S3DRenderParameterContext::SetCamera( matCamera, posCamera ) ;
	if ( m_flagBegin3D )
	{
		UpdateLightEntries() ;
	}
}

// 光源を設定
//////////////////////////////////////////////////////////////////////////////
void S3DSoftwareRenderer::SetLightEntries
	( const S3DLightEntry* pLights, size_t countLight )
{
	S3DRenderParameterContext::SetLightEntries( pLights, countLight ) ;
	if ( m_flagBegin3D )
	{
		UpdateLightEntries() ;
	}
}

// シャドウマップを設定
//////////////////////////////////////////////////////////////////////////////
void S3DSoftwareRenderer::SetShadowMap
	( uint32_t idLight,
		SGLImageObject* pShadowMapDepth,
		const S3DShadowMapInfo& infShadowMap,
		SGLImageObject* pShadowMapColor )
{
	S3DRenderParameterContext::SetShadowMap
		( idLight, pShadowMapDepth, infShadowMap, pShadowMapColor ) ;
	if ( m_flagBegin3D )
	{
//		UpdateLightEntries() ;
	}
}

// 疑似フォッグを設定
//////////////////////////////////////////////////////////////////////////////
void S3DSoftwareRenderer::SetFog
	( uint32_t rgbFog, double zFogNear, double zFogFar )
{
	S3DRenderParameterContext::SetFog( rgbFog, zFogNear, zFogFar ) ;
	m_shader.SetFog( rgbFog, zFogNear, zFogFar ) ;
}

void S3DSoftwareRenderer::EnableFog( bool fFog )
{
	S3DRenderParameterContext::EnableFog( fFog ) ;
	m_shader.EnableFog( fFog ) ;
}

// 対応機能取得
//////////////////////////////////////////////////////////////////////////////
void S3DSoftwareRenderer::GetRenderingCapacity( S3DRenderingCapacity& caps )
{
	eslFillMemory( &caps, 0, sizeof(S3DRenderingCapacity) ) ;
	caps.flagsRendering = 0 ;
	caps.flagsShading = S3DRenderingCapacity::shadingGouraud
						| S3DRenderingCapacity::shadingPhong
						| S3DRenderingCapacity::shadingShadowMapping ;
	caps.typeHeadware = S3DRenderingCapacity::softwareEntisGLS4 ;
	caps.maxTextureSize = 0x10000 ;
	caps.maxTextureUnit = 0x100 ;
	caps.maxLightCount = 0x100 ;
	caps.maxShadowmapCount = 0x100 ;
	caps.flagsExtensions1 = S3DRenderingCapacity::extFramebuffer
						| S3DRenderingCapacity::extTextureNonPowerOf2
						| S3DRenderingCapacity::extMultiTexture
						| S3DRenderingCapacity::extVertexBuffer ;
}

// 内部バッファサイズ設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSoftwareRenderer::SetRenderingBufferSize( uint32_t countVertex )
{
	m_bufRender.SetBlockSize
		( countVertex * (sizeof(S3DVector4) * 2 + sizeof(S3DColor)
						+ sizeof(S2DVector) + sizeof(uint32_t) * 2) ) ;
	return	sglErrSuccess ;
}

// 3D レンダリング用バッファ・インターフェース開始
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSoftwareRenderer::Begin3DRenderer( uint64_t nFlags )
{
	m_flagBegin3D = true ;
	UpdateLightEntries() ;
	//
	return	S3DRenderParameterContext::Begin3DRenderer( nFlags ) ;
}

// 3D レンダリング用バッファ・インターフェース終了
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSoftwareRenderer::End3DRenderer( uint64_t nFlags )
{
	m_flagBegin3D = false ;
	//
	return	S3DRenderParameterContext::Begin3DRenderer( nFlags ) ;
}

#if	!defined(__COTOPHA__)
S3DRenderDevice * S3DSoftwareRenderer::GetRenderDeviceObject( uint64_t nFlags )
{
	return	SGLSoftwareRenderDevice::GetDefault() ;
}
#endif

// 光源更新（カメラ変換反映）
//////////////////////////////////////////////////////////////////////////////
void S3DSoftwareRenderer::UpdateLightEntries( void )
{
	S3DMatrix	mat = m_matCamera ;
	S3DVector	pos = - m_vCameraPos ;
	//
	m_lstLightEntries.SetLength( 0 ) ;
	//
	size_t					nLights ;
	const S3DLightEntry *	pVecLights = m_arrayVectorLights.GetConstArray() ;
	nLights = m_arrayVectorLights.GetLength() ;
	for ( size_t i = 0; i < nLights; i ++ )
	{
		S3DLightEntry	light = pVecLights[i] ;
		mat.RevolveVector( light.vecDirection ) ;
		m_lstLightEntries.Add( light ) ;
	}
	//
	const S3DLightEntry *	pPointLights = m_arrayPointLights.GetConstArray() ;
	nLights = m_arrayPointLights.GetLength() ;
	for ( size_t i = 0; i < nLights; i ++ )
	{
		S3DLightEntry	light = pPointLights[i] ;
		mat.RevolveVector( light.vecPosition ) ;
		mat.RevolveVector( light.vecDirection ) ;
		light.vecPosition += pos ;
		m_lstLightEntries.Add( light ) ;
	}
	//
	const S3DLightEntry *	pFogLights = m_arrayFogLights.GetConstArray() ;
	nLights = m_arrayFogLights.GetLength() ;
	for ( size_t i = 0; i < nLights; i ++ )
	{
		S3DLightEntry	light = pFogLights[i] ;
		mat.RevolveVector( light.vecPosition ) ;
		mat.RevolveVector( light.vecDirection ) ;
		light.vecPosition += pos ;
		m_lstLightEntries.Add( light ) ;
	}
	//
	m_shader.SetLightEntries
		( m_lstLightEntries.GetConstArray(), m_lstLightEntries.GetLength() ) ;
}

// 非同期レンダリングの同期
//////////////////////////////////////////////////////////////////////////////
void S3DSoftwareRenderer::SyncRender( void )
{
	SyncPolygonBuffer( m_bufPolygon[0] ) ;
	SyncPolygonBuffer( m_bufPolygon[1] ) ;
	//
	if ( m_pLastTexture != NULL )
	{
		m_pLastTexture->UnlockBuffer( SGLImageObject::lockRead ) ;
		m_pLastTexture = NULL ;
	}
}

// 実行中のスレッドを終了
//////////////////////////////////////////////////////////////////////////////
void S3DSoftwareRenderer::ExitRenderThread( void )
{
	m_csQueue.Lock() ;
	m_flagAbortThread = true ;
	m_sigQueue.SetSignal() ;
	m_csQueue.Unlock() ;
	//
	for ( size_t i = 0; i < m_arrRasterizeBuf.GetLength(); i ++ )
	{
		RasterizeBuffer *	prb = m_arrRasterizeBuf.GetAt( i ) ;
		if ( (prb != NULL) && prb->m_flagThreading )
		{
			prb->m_sigEndThread.Wait() ;
			prb->m_sigEndThread.ResetSignal() ;
			prb->m_flagThreading = false ;
		}
	}
	//
	m_csQueue.Lock() ;
	m_flagAbortThread = false ;
	m_sigQueue.ResetSignal() ;
	m_csQueue.Unlock() ;
}

// レンダリング中のメッシュをすべてスケジューリングし終えるまでレンダリング処理
//////////////////////////////////////////////////////////////////////////////
void S3DSoftwareRenderer::DoRender( void )
{
	RasterizeBuffer *	prbMain = m_arrRasterizeBuf.GetAt( 0 ) ;
	ESLAssert( prbMain != NULL ) ;
	if ( prbMain == NULL )
	{
		return ;
	}
	for ( ; ; )
	{
		//
		// レンダリングキューにメッシュを割り当て
		//
		m_csQueue.Lock() ;
		while ( m_pRenderingMesh == NULL )
		{
			if ( m_pFirstMesh == NULL )
			{
				break ;
			}
			m_pRenderingMesh = m_pFirstMesh ;
			m_pFirstMesh = m_pFirstMesh->pChainNext ;
			if ( m_pFirstMesh == NULL )
			{
				ESLAssert( m_pLastMesh == m_pRenderingMesh ) ;
				m_pLastMesh = NULL ;
			}
			m_iPolygon = 0 ;
			m_iNextPolygon = 0 ;
			//
			// メッシュ描画パラメータ準備
			//
			if ( SetupRenderMesh() )
			{
				while ( m_iNextPolygon < m_pRenderingMesh->countPolygon )
				{
					if ( SetupPolygonBuffer
						( &m_bufPolygon[m_iPolygon],
							m_pRenderingMesh, m_iNextPolygon ++ ) )
					{
						break ;
					}
				}
				while ( m_iNextPolygon < m_pRenderingMesh->countPolygon )
				{
					if ( SetupPolygonBuffer
						( &m_bufPolygon[m_iPolygon ^ 1],
							m_pRenderingMesh, m_iNextPolygon ++ ) )
					{
						break ;
					}
				}
			}
			else
			{
				m_pRenderingMesh = NULL ;
				ESLVerify( -- m_nQueueMesh >= 0 ) ;
			}
		}
		if ( m_pRenderingMesh == NULL )
		{
			m_csQueue.Unlock() ;
			break ;
		}
		m_sigQueue.SetSignal() ;
		//
		SThread::IdType	tidMain = SThread::GetCurrentId() ;
		prbMain->m_tidMain = tidMain ;
		//
		if ( SThread::GetRunningStockThread()
					< (int) SSystem::g_cpuLogicalCount - 1 )
		{
			for ( size_t i = 1; i < m_arrRasterizeBuf.GetLength(); i ++ )
			{
				RasterizeBuffer *	prbSub = m_arrRasterizeBuf.GetAt( i ) ;
				if ( (prbSub == NULL) 
					|| prbSub->m_flagThreading )
				{
					continue ;
				}
				prbSub->m_flagThreading = true ;
				prbSub->m_sigEndThread.ResetSignal() ;
				prbSub->m_pRenderer = this ;
				prbSub->m_tidMain = tidMain ;
				prbSub->m_iThread = i ;
				//
				if ( SThread::BeginStockThread
					( &S3DSoftwareRenderer::RenderingThreadProc, prbSub ) != NULL )
				{
					AtomicAdd( &m_nRunningThreads, 1 ) ;
				}
				//
				if ( SThread::GetRunningStockThread()
							>= (int) SSystem::g_cpuLogicalCount - 1 )
				{
					break ;
				}
			}
		}
		//
		for ( ; ; )
		{
			//
			// ポリゴンラスタライザ割り当て
			//
			PolygonBuffer&	pb = m_bufPolygon[m_iPolygon] ;
			AtomicAdd( &(pb.m_countRef), 1 ) ;
			m_csQueue.Unlock() ;
			//
			while ( AssinRasterize( pb, *prbMain ) )
			{
				(pb.m_rfs.pfnSampling)( &pb, prbMain ) ;
				(pb.m_rfs.pfnWriteFill)( prbMain ) ;
			}
			//
			pb.m_flagReady = false ;
			AtomicSub( &(pb.m_countRef), 1 ) ;
			//
			m_csQueue.Lock() ;
			//
			// ポリゴン割り当て
			//
			if ( !AssinMeshPolygon() )
			{
				m_csQueue.Unlock() ;
				break ;
			}
		}
	}
}

// メッシュ描画パラメータ準備
//////////////////////////////////////////////////////////////////////////////
bool S3DSoftwareRenderer::SetupRenderMesh( void )
{
	ESLAssert( m_pRenderingMesh != NULL ) ;
	S3DMaterial *	pMaterial = m_pRenderingMesh->pMaterial ;
	if ( pMaterial == NULL )
	{
		return	false ;
	}
	m_nMeshAlpha = m_pRenderingMesh->nAlpha ;
	//
	const S3DSurfaceAttribute&	attr = pMaterial->m_attrSurface ;
	FUNC_RASTER_SAMPLING	pfnSampling = NULL ;
	m_flagsShading = attr.flagsShading ;
	if ( attr.flagsShading & shadingTextureMapping )
	{
		if ( m_pRenderingMesh->pvUVMap == NULL )
		{
			return	false ;
		}
		if ( attr.flagsShading & shadingTextureTiling )
		{
			if ( attr.flagsShading & shadingTextureSmoothing )
			{
				pfnSampling =
					&S3DSoftwareRenderer::rasterize_SampleTextureTileSmooth ;
			}
			else
			{
				pfnSampling =
					&S3DSoftwareRenderer::rasterize_SampleTextureTile ;
			}
		}
		else
		{
			pfnSampling =
				&S3DSoftwareRenderer::rasterize_SampleTexture ;
		}
		SGLImageObject *	pImage =
			pMaterial->GetTexture
				( pMaterial->FindTextureTypeOf( S3DMaterial::textureMain ) ) ;
		if ( pImage == NULL )
		{
			return	false ;
		}
		if ( m_pLastTexture != pImage )
		{
			SyncPolygonBuffer( m_bufPolygon[0] ) ;
			SyncPolygonBuffer( m_bufPolygon[1] ) ;
			//
			if ( m_pLastTexture != NULL )
			{
				m_pLastTexture->UnlockBuffer( SGLImageObject::lockRead ) ;
			}
			m_pLastTexture = pImage ;
			m_imgTexture.ptrBuffer =
				pImage->LockBuffer( m_imgTexture, SGLImageObject::lockRead ) ;
		}
		if ( m_imgTexture.format & formatImageFlagAlpha )
		{
			if ( (m_imgTexture.format
					& formatImageTypeMask) == formatImageBGR )
			{
				m_rfsFuncSet.pfnTexture = pfnSampling ;
				pfnSampling = &S3DSoftwareRenderer::rasterize_TextureABGR ;
			}
		}
		else
		{
			if ( (m_imgTexture.format
					& formatImageTypeMask) == formatImageBGR )
			{
				m_rfsFuncSet.pfnTexture = pfnSampling ;
				pfnSampling = &S3DSoftwareRenderer::rasterize_TextureBGR ;
			}
			else
			{
				m_rfsFuncSet.pfnTexture = pfnSampling ;
				pfnSampling = &S3DSoftwareRenderer::rasterize_TextureRGB ;
			}
		}
		if ( m_pRenderingMesh->pColor != NULL )
		{
			m_rfsFuncSet.pfnPreVertexColor = pfnSampling ;
			pfnSampling = &S3DSoftwareRenderer::
							rasterize_VertexColorAfterTexture ;
		}
	}
	else
	{
		m_imgTexture.ptrBuffer = NULL ;
		pfnSampling =
			&S3DSoftwareRenderer::rasterize_VertexColorWithoutTexture ;
	}
	if ( m_nMeshAlpha < 0xFF )
	{
		if ( m_nMeshAlpha <= 1 )
		{
			return	false ;
		}
		m_rfsFuncSet.pfnPreTransparency = pfnSampling ;
		pfnSampling = &S3DSoftwareRenderer::rasterize_PostTransparencyEffect ;
	}
	m_rfsFuncSet.pfnSampling = pfnSampling ;
	//
	if ( m_pbytZBuffer != NULL )
	{
		if ( attr.flagsShading & shadingNoZBuffer )
		{
			m_rfsFuncSet.pfnWriteFill =
					&S3DSoftwareRenderer::rasterize_WriteSimple ;
		}
		else if ( attr.flagsShading & shadingZBufferNoWrite )
		{
			m_rfsFuncSet.pfnWriteFill =
					&S3DSoftwareRenderer::rasterize_WriteCompareZ ;
		}
		else
		{
			m_rfsFuncSet.pfnWriteFill =
					&S3DSoftwareRenderer::rasterize_WriteWithZ ;
		}
	}
	else
	{
		m_rfsFuncSet.pfnWriteFill =
				&S3DSoftwareRenderer::rasterize_WriteSimple ;
	}
	return	true ;
}

// ポリゴン描画パラメータ準備
//////////////////////////////////////////////////////////////////////////////
bool S3DSoftwareRenderer::SetupPolygonBuffer
	( S3DSoftwareRenderer::PolygonBuffer * ppb,
		S3DSoftwareRenderer::MeshEntry * pMesh, size_t iPolygon )
{
	SyncPolygonBuffer( *ppb ) ;
	//
	ESLAssert( !(ppb->m_flagReady) ) ;
	ESLAssert( ppb->m_pbufRegion != NULL ) ;
	ppb->m_flagReady = false ;
	ppb->m_pbufRegion->yTop = 0 ;
	ppb->m_pbufRegion->yBottom = -1 ;
	ppb->m_yScanLine = 0 ;
	//
	if ( pMesh->countPolygon <= iPolygon )
	{
		return	false ;
	}
	ppb->m_rfs = m_rfsFuncSet ;
	ppb->m_nAlpha = m_nMeshAlpha ;
	ppb->m_imgTexture = m_imgTexture ;
	//
	// 頂点取得
	//
	S3DVector4		vVertexSrc[3] ;
	S2DVector		vUVMapSrc[3] ;
	S3DColor		clrVertexSrc[3] ;
	uint32_t *		pIndex = pMesh->pIndexedList + (iPolygon * 3) ;
	S3DVector4 *	pvVertex = pMesh->pvVertex ;
	S2DVector *		pvUVMap = pMesh->pvUVMap ;
	S3DColor *		pColor = pMesh->pColor ;
	size_t			i ;
	//
	for ( i = 0; i < 3; i ++ )
	{
		uint32_t	j = pIndex[i] ;
		vVertexSrc[i] = pvVertex[j] ;
		if ( pvUVMap != NULL )
		{
			vUVMapSrc[i] = pvUVMap[j] ;
		}
		if ( pColor != NULL )
		{
			clrVertexSrc[i] = pColor[j] ;
		}
	}
	//
	// 裏面判定
	//
	if ( pMesh->pMaterial->m_attrSurface.flagsShading
									& shadingSingleSidePlane )
	{
		S3DVector	v1 = vVertexSrc[1] - vVertexSrc[0] ;
		S3DVector	v2 = vVertexSrc[2] - vVertexSrc[0] ;
		if ( (v1 * v2).InnerProduct( vVertexSrc[0] ) > 0.0 )
		{
			return	false ;
		}
	}
	//
	// ｚクリッピング
	//
	const int	MaxVertexCount = 6 ;
	S3DVector4	vVertexClip[MaxVertexCount] ;
	S3DColor	clrVertexClip[MaxVertexCount] ;
	size_t		nVertexCount = 0 ;
	float32_t	zMinClip = (float32_t) m_zMinClip ;
	bool		fLastOutZ = (vVertexSrc[2].z < zMinClip) ;
	size_t		iLast = 2 ;
	for ( i = 0; i < 3; i ++ )
	{
		bool	fCurOutZ = (vVertexSrc[i].z < zMinClip) ;
		if ( fCurOutZ != fLastOutZ )
		{
			S3DVector4	vLast = vVertexSrc[iLast] ;
			S3DVector4	vDelta = vVertexSrc[i] ;
			vDelta -= vLast ;
			//
			double		t = (zMinClip - vLast.z) / vDelta.z ;
			S3DVector4	vClip = vDelta ;
			vClip *= (float32_t) t ;
			vClip += vLast ;
			//
			S3DColor	clrClip =
				clrVertexSrc[iLast] * (1.0 - t) + clrVertexSrc[i] * t ;
			//
			vVertexClip[nVertexCount] = vClip ;
			clrVertexClip[nVertexCount] = clrClip ;
			nVertexCount ++ ;
		}
		if ( !fCurOutZ )
		{
			vVertexClip[nVertexCount] = vVertexSrc[i] ;
			clrVertexClip[nVertexCount] = clrVertexSrc[i] ;
			nVertexCount ++ ;
		}
		fLastOutZ = fCurOutZ ;
		iLast = i ;
	}
	ESLAssert( nVertexCount <= (size_t) MaxVertexCount ) ;
	if ( nVertexCount < 3 )
	{
		return	false ;
	}
	//
	// 透視変換
	//
	S2DVector	vVertexProj[MaxVertexCount] ;
	S3DVector	vScreen = m_vProjectionScreen ;
	vScreen.z *= (float32_t) m_zProjectionScale ;
	ppb->m_vScreen = vScreen ;
	//
	for ( i = 0; i < nVertexCount; i ++ )
	{
		float32_t	rz = vScreen.z / vVertexClip[i].z ;
		vVertexProj[i].x = vVertexClip[i].x * rz + vScreen.x ;
		vVertexProj[i].y = vVertexClip[i].y * rz + vScreen.y ;
	}
	//
	// リージョン生成
	//
	SGLRect	rctView = SGLPaintBuffer::m_rctView ;
	if ( pColor != NULL )
	{
		pColor = &clrVertexClip[0] ;
	}
	if ( !sglCreatePolygonRegion
		( ppb->m_pbufRegion, rctView,
			&vVertexProj[0], nVertexCount, pColor ) )
	{
		ppb->m_pbufRegion->yTop = 0 ;
		ppb->m_pbufRegion->yBottom = -1 ;
		ppb->m_yScanLine = 0 ;
		return	false ;
	}
	ppb->m_yScanLine = ppb->m_pbufRegion->yTop ;
	ppb->m_pDstColor =
		m_pbytTarget + (m_infTarget.pitchLine * ppb->m_yScanLine) ;
	ppb->m_pDstZBuf = NULL ;
	if ( m_pbytZBuffer != NULL )
	{
		ppb->m_pDstZBuf =
			m_pbytZBuffer + (m_infZBuffer.pitchLine * ppb->m_yScanLine) ;
	}
	//
	// 平面・テクスチャパラメータ計算
	//
	if ( m_flagsShading & shadingTextureMapping )
	{
		S3DVector	vDelta1 = vVertexSrc[1] - vVertexSrc[0] ;
		S3DVector	vDelta2 = vVertexSrc[2] - vVertexSrc[0] ;
		S2DVector	vTexDelta1 = vUVMapSrc[1] - vUVMapSrc[0] ;
		S2DVector	vTexDelta2 = vUVMapSrc[2] - vUVMapSrc[0] ;
		//
		S3DTemporaryTextureAxisBuffer::TextureBaseAxis
			( ppb->m_vTextureX, ppb->m_vTextureY,
				vDelta1, vDelta2, vTexDelta1, vTexDelta2 ) ;
		//
		ppb->m_vTextureO = vVertexSrc[0] ;
		ppb->m_vTextureO -= ppb->m_vTextureX * vUVMapSrc[0].x ;
		ppb->m_vTextureO -= ppb->m_vTextureY * vUVMapSrc[0].y ;
		//
		ppb->m_vNormal = ppb->m_vTextureX * ppb->m_vTextureY ;
		//
		float32_t	d = ppb->m_vNormal.InnerProduct( ppb->m_vNormal ) ;
		if ( d > 1.0e-8 )
		{
			ppb->m_vNormal *= 1.0f / d ;
		}
		else
		{
			ppb->m_vNormal = vDelta1 * vDelta2 ;
			ppb->m_vNormal.Normalize() ;
		}
		//
		ppb->m_fpTexO_Normal =
			ppb->m_vTextureO.InnerProduct( ppb->m_vNormal ) ;
		ppb->m_fpSpt_Normal =
			(- vScreen.x * ppb->m_vNormal.x)
			+ (((float32_t) ppb->m_yScanLine - vScreen.y) * ppb->m_vNormal.y)
			+ (vScreen.z * ppb->m_vNormal.z) ;
		ppb->m_vDeltaTexX = ppb->m_vTextureY * ppb->m_vNormal ;
		ppb->m_vDeltaTexY = ppb->m_vNormal * ppb->m_vTextureX ;
		ppb->m_fpBaseTexX =
			- ppb->m_vTextureO.InnerProduct( ppb->m_vDeltaTexX ) ;
		ppb->m_fpBaseTexY =
			- ppb->m_vTextureO.InnerProduct( ppb->m_vDeltaTexY ) ;
	}
	else
	{
		S3DVector	vDelta1 = vVertexSrc[1] - vVertexSrc[0] ;
		S3DVector	vDelta2 = vVertexSrc[2] - vVertexSrc[0] ;
		ppb->m_vNormal = vDelta1 * vDelta2 ;
		ppb->m_vNormal.Normalize() ;
		//
		ppb->m_fpTexO_Normal =
			vVertexSrc[0].InnerProduct( ppb->m_vNormal ) ;
		ppb->m_fpSpt_Normal =
			(- vScreen.x * ppb->m_vNormal.x)
			+ (((float32_t) ppb->m_yScanLine - vScreen.y) * ppb->m_vNormal.y)
			+ (vScreen.z * ppb->m_vNormal.z) ;
	}
	ppb->m_flagReady = true ;
	return	true ;
}

// ポリゴン描画完了待ち
//////////////////////////////////////////////////////////////////////////////
void S3DSoftwareRenderer::SyncPolygonBuffer
	( S3DSoftwareRenderer::PolygonBuffer& pb )
{
	while ( pb.m_countRef != 0 )
	{
	}
}

// ポリゴン割り当て
//////////////////////////////////////////////////////////////////////////////
bool S3DSoftwareRenderer::AssinMeshPolygon( void )
{
	if ( m_pRenderingMesh == NULL )
	{
		return	false ;
	}
	if ( m_bufPolygon[m_iPolygon].m_flagReady )
	{
		return	true ;
	}
	m_iPolygon ^= 1 ;
	while ( m_iNextPolygon < m_pRenderingMesh->countPolygon )
	{
		if ( SetupPolygonBuffer
				( &m_bufPolygon[m_iPolygon ^ 1],
					m_pRenderingMesh, m_iNextPolygon ++ ) )
		{
			break ;
		}
	}
	if ( !m_bufPolygon[m_iPolygon].m_flagReady )
	{
		m_pRenderingMesh = NULL ;
		m_sigQueue.ResetSignal() ;
		return	false ;
	}
	return	true ;
}

// ラスタライズ割り当て
//////////////////////////////////////////////////////////////////////////////
bool S3DSoftwareRenderer::AssinRasterize
	( S3DSoftwareRenderer::PolygonBuffer& pb,
			S3DSoftwareRenderer::RasterizeBuffer& rb )
{
	bool		flagAssin = false ;
	SGLRegion *	pRegion ;
	SpinLock( pb ) ;
	pRegion = pb.m_pbufRegion ;
	while ( pb.m_yScanLine <= pRegion->yBottom )
	{
		ESLAssert( pb.m_yScanLine >= pRegion->yTop ) ;
		rb.m_rglLine =
			pRegion->rgLine[(pb.m_yScanLine ++) - pRegion->yTop] ;
		//
		int32_t	nLeft = (rb.m_rglLine.fxLeft + 0xFFFF) >> 16 ;
		int32_t	nRight = rb.m_rglLine.fxRight >> 16 ;
		//
		if ( nLeft > nRight )
		{
			pb.m_pDstColor += m_infTarget.pitchLine ;
			if ( pb.m_pDstZBuf != NULL )
			{
				pb.m_pDstZBuf += m_infZBuffer.pitchLine ;
			}
			continue ;
		}
		rb.m_nWidth = nRight - nLeft + 1 ;
		ESLAssert( rb.m_nWidth <= rb.m_bufTempSrc.GetLength() ) ;
		//
		rb.m_pDstColor = ((uint32_t*) pb.m_pDstColor) + nLeft ;
		pb.m_pDstColor += m_infTarget.pitchLine ;
		//
		rb.m_pDstZBuf = (uint32_t*) pb.m_pDstZBuf ;
		if ( rb.m_pDstZBuf != NULL )
		{
			rb.m_pDstZBuf += nLeft ;
			pb.m_pDstZBuf += m_infZBuffer.pitchLine ;
		}
		//
		float32_t	fpLeft = (float32_t) nLeft ;
		rb.m_vPosScreen.x = fpLeft - pb.m_vScreen.x ;
		rb.m_vPosScreen.y =
				(float32_t) (pb.m_yScanLine - 1) - pb.m_vScreen.y ;
		rb.m_vPosScreen.z = pb.m_vScreen.z ;
		rb.m_vPosScreen *= pb.m_fpTexO_Normal ;
		//
		rb.m_fpSpt_Normal =
				pb.m_fpSpt_Normal
					+ pb.m_vNormal.x * fpLeft
					+ pb.m_vNormal.y
						* (float32_t) (pb.m_yScanLine - 1 - pRegion->yTop) ;
		//
		flagAssin = true ;
		break ;
	}
	SpinUnlock( pb ) ;
	return	flagAssin ;
}

// スピンロック
//////////////////////////////////////////////////////////////////////////////
void S3DSoftwareRenderer::SpinLock( S3DSoftwareRenderer::PolygonBuffer& pb )
{
	while ( AtomicXchg( &(pb.m_flagSpinLock), 1 ) == 1 )
	{
	}
}

void S3DSoftwareRenderer::SpinUnlock( S3DSoftwareRenderer::PolygonBuffer& pb )
{
	AtomicXchg( &(pb.m_flagSpinLock), 0 ) ;
}

// 描画スレッド関数
//////////////////////////////////////////////////////////////////////////////
void S3DSoftwareRenderer::RenderingThreadProc( void * pInstance )
{
	RasterizeBuffer *	prb = (RasterizeBuffer*) pInstance ;
	prb->m_pRenderer->RenderingThread( prb ) ;
}

void S3DSoftwareRenderer::RenderingThread
		( S3DSoftwareRenderer::RasterizeBuffer * prb )
{
	while ( !m_flagAbortThread )
	{
		m_csQueue.Lock() ;
		PolygonBuffer&	pb = m_bufPolygon[m_iPolygon] ;
		if ( pb.m_flagReady )
		{
			AtomicAdd( &(pb.m_countRef), 1 ) ;
			m_csQueue.Unlock() ;
			//
			while ( AssinRasterize( pb, *prb ) )
			{
				(pb.m_rfs.pfnSampling)( &pb, prb ) ;
				(pb.m_rfs.pfnWriteFill)( prb ) ;
			}
			//
			pb.m_flagReady = false ;
			AtomicSub( &(pb.m_countRef), 1 ) ;
			//
			m_csQueue.Lock() ;
			//
			// ポリゴン割り当て
			//
			if ( AssinMeshPolygon() )
			{
				m_csQueue.Unlock() ;
			}
			else
			{
				m_csQueue.Unlock() ;
				//
				if ( m_sigQueue.Wait( 1 ) == errTimeout )
				{
					break ;
				}
			}
		}
		else
		{
			m_csQueue.Unlock() ;
			//
			if ( m_sigQueue.Wait( 1 ) == errTimeout )
			{
				break ;
			}
		}
	}
	//
	m_csQueue.Lock() ;
	ESLVerify( AtomicSub( &m_nRunningThreads, 1 ) >= 0 ) ;
	m_sigEndThread.SetSignal() ;
	prb->m_flagThreading = false ;
	prb->m_sigEndThread.SetSignal() ;
	m_csQueue.Unlock() ;
}

// サンプリング関数
//////////////////////////////////////////////////////////////////////////////
void S3DSoftwareRenderer::rasterize_TextureRGB
	( S3DSoftwareRenderer::PolygonBuffer * ppb,
		S3DSoftwareRenderer::RasterizeBuffer * prb )
{
	(ppb->m_rfs.pfnTexture)( ppb, prb ) ;
	//
	size_t		nCount = prb->m_nWidth ;
	uint32_t *	pTempTex = prb->m_pTempSrc ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		pTempTex[i] |= 0xFF000000 ;
	}
}

void S3DSoftwareRenderer::rasterize_TextureBGR
	( S3DSoftwareRenderer::PolygonBuffer * ppb,
		S3DSoftwareRenderer::RasterizeBuffer * prb )
{
	(ppb->m_rfs.pfnTexture)( ppb, prb ) ;
	//
	size_t		nCount = prb->m_nWidth ;
	uint32_t *	pTempTex = prb->m_pTempSrc ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		uint32_t	argbTex = pTempTex[i] ;
		pTempTex[i] = ((argbTex & 0xFF00FF00) | 0xFF000000)
						| (((argbTex & 0x00FF0000) >> 16)
						| ((argbTex & 0x000000FF) << 16)) ;
	}
}

void S3DSoftwareRenderer::rasterize_TextureABGR
	( S3DSoftwareRenderer::PolygonBuffer * ppb,
		S3DSoftwareRenderer::RasterizeBuffer * prb )
{
	(ppb->m_rfs.pfnTexture)( ppb, prb ) ;
	//
	size_t		nCount = prb->m_nWidth ;
	uint32_t *	pTempTex = prb->m_pTempSrc ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		uint32_t	argbTex = pTempTex[i] ;
		pTempTex[i] = (argbTex & 0xFF00FF00)
					| ((argbTex & 0x00FF0000) >> 16)
					| ((argbTex & 0x000000FF) << 16) ;
	}
}

void S3DSoftwareRenderer::rasterize_VertexColorAfterTexture
	( S3DSoftwareRenderer::PolygonBuffer * ppb,
		S3DSoftwareRenderer::RasterizeBuffer * prb )
{
	(ppb->m_rfs.pfnPreVertexColor)( ppb, prb ) ;
	//
	rasterize_ComplementVertexColor( prb ) ;
	rasterize_EffectVertexColor
		( prb->m_pTempSrc, prb->m_pTempSrc,
				prb->m_pTempColor, prb->m_nWidth ) ;
}

void S3DSoftwareRenderer::rasterize_VertexColorWithoutTexture
	( S3DSoftwareRenderer::PolygonBuffer * ppb,
		S3DSoftwareRenderer::RasterizeBuffer * prb )
{
	S3DVector	vNormal = ppb->m_vNormal ;
	S3DVector	vPosScreen = prb->m_vPosScreen ;
	float32_t	fpSpt_Normal = prb->m_fpSpt_Normal ;
	size_t		nCount = prb->m_nWidth ;
	float32_t *	pTempZ = prb->m_pTempZ ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		*(pTempZ ++) = vPosScreen.z / fpSpt_Normal ;
		fpSpt_Normal += vNormal.x ;
	}
	//
	rasterize_ComplementVertexColor( prb ) ;
	rasterize_ConvertVertexColor
		( prb->m_pTempSrc, prb->m_pTempColor, prb->m_nWidth ) ;
}

void S3DSoftwareRenderer::rasterize_PostTransparencyEffect
	( S3DSoftwareRenderer::PolygonBuffer * ppb,
		S3DSoftwareRenderer::RasterizeBuffer * prb )
{
	(ppb->m_rfs.pfnPreTransparency)( ppb, prb ) ;
	//
	size_t		nCount = prb->m_nWidth ;
	uint32_t *	pTempSrc = prb->m_pTempSrc ;
	uint32_t	nAlpha = ppb->m_nAlpha ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		pTempSrc[i] = sglPackedColorMul( pTempSrc[i], nAlpha ) ;
	}
}

// テクスチャサンプリング関数
//////////////////////////////////////////////////////////////////////////////
void S3DSoftwareRenderer::rasterize_SampleTexture
	( S3DSoftwareRenderer::PolygonBuffer * ppb,
		S3DSoftwareRenderer::RasterizeBuffer * prb )
{
	S3DVector	vNormal = ppb->m_vNormal ;
	S3DVector	vDeltaTexX = ppb->m_vDeltaTexX ;
	S3DVector	vDeltaTexY = ppb->m_vDeltaTexY ;
	float32_t	fpBaseTexX = ppb->m_fpBaseTexX ;
	float32_t	fpBaseTexY = ppb->m_fpBaseTexY ;
	float32_t	fpTexO_Normal = ppb->m_fpTexO_Normal ;
	S3DVector	vPosScreen = prb->m_vPosScreen ;
	float32_t	fpPos_TexX = vPosScreen.InnerProduct( vDeltaTexX ) ;
	float32_t	fpPos_TexY = vPosScreen.InnerProduct( vDeltaTexY ) ;
	float32_t	fpDelta_TexX = vDeltaTexX.x * fpTexO_Normal ;
	float32_t	fpDelta_TexY = vDeltaTexY.x * fpTexO_Normal ;
	float32_t	fpSpt_Normal = prb->m_fpSpt_Normal ;
	//
	size_t		nCount = prb->m_nWidth ;
	uint32_t *	pTempTex = prb->m_pTempSrc ;
	float32_t *	pTempZ = prb->m_pTempZ ;
	uint32_t	widthTex = ppb->m_imgTexture.width ;
	uint32_t	heightTex = ppb->m_imgTexture.height ;
	int32_t		pitchLine = ppb->m_imgTexture.pitchLine ;
	uint8_t *	pTexture = ppb->m_imgTexture.ptrBuffer ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		float32_t	rcpSptNormal = (1.0f / fpSpt_Normal) ;
		float32_t	z = vPosScreen.z * rcpSptNormal ;
		float32_t	u = fpPos_TexX * rcpSptNormal + fpBaseTexX ;
		float32_t	v = fpPos_TexY * rcpSptNormal + fpBaseTexY ;
		int32_t		ui = eslRoundR32ToInt( u ) ;
		int32_t		vi = eslRoundR32ToInt( v ) ;
		//
		if ( (uint32_t) ui >= widthTex )
		{
			ui = ~(ui >> 31) & (widthTex - 1) ;
		}
		if ( (uint32_t) vi >= heightTex )
		{
			vi = ~(vi >> 31) & (heightTex - 1) ;
		}
		*(pTempTex ++) =
			*((uint32_t*)(pTexture + (vi * pitchLine + ui * 4))) ;
		*(pTempZ ++) = z ;
		//
		fpSpt_Normal += vNormal.x ;
		fpPos_TexX += fpDelta_TexX ;
		fpPos_TexY += fpDelta_TexY ;
	}
}

void S3DSoftwareRenderer::rasterize_SampleTextureTile
	( S3DSoftwareRenderer::PolygonBuffer * ppb,
		S3DSoftwareRenderer::RasterizeBuffer * prb )
{
	S3DVector	vNormal = ppb->m_vNormal ;
	S3DVector	vDeltaTexX = ppb->m_vDeltaTexX ;
	S3DVector	vDeltaTexY = ppb->m_vDeltaTexY ;
	float32_t	fpBaseTexX = ppb->m_fpBaseTexX ;
	float32_t	fpBaseTexY = ppb->m_fpBaseTexY ;
	float32_t	fpTexO_Normal = ppb->m_fpTexO_Normal ;
	S3DVector	vPosScreen = prb->m_vPosScreen ;
	float32_t	fpPos_TexX = vPosScreen.InnerProduct( vDeltaTexX ) ;
	float32_t	fpPos_TexY = vPosScreen.InnerProduct( vDeltaTexY ) ;
	float32_t	fpDelta_TexX = vDeltaTexX.x * fpTexO_Normal ;
	float32_t	fpDelta_TexY = vDeltaTexY.x * fpTexO_Normal ;
	float32_t	fpSpt_Normal = prb->m_fpSpt_Normal ;
	//
	size_t		nCount = prb->m_nWidth ;
	uint32_t *	pTempTex = prb->m_pTempSrc ;
	float32_t *	pTempZ = prb->m_pTempZ ;
	uint32_t	maskTexX = ppb->m_imgTexture.width - 1 ;
	uint32_t	maskTexY = ppb->m_imgTexture.height - 1 ;
	int32_t		pitchLine = ppb->m_imgTexture.pitchLine ;
	uint8_t *	pTexture = ppb->m_imgTexture.ptrBuffer ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		float32_t	rcpSptNormal = (1.0f / fpSpt_Normal) ;
		float32_t	z = vPosScreen.z * rcpSptNormal ;
		float32_t	u = fpPos_TexX * rcpSptNormal + fpBaseTexX ;
		float32_t	v = fpPos_TexY * rcpSptNormal + fpBaseTexY ;
		int32_t		ui = eslRoundR32ToInt( u ) & maskTexX ;
		int32_t		vi = eslRoundR32ToInt( v ) & maskTexY ;
		//
		*(pTempTex ++) =
			*((uint32_t*)(pTexture + (vi * pitchLine + ui * 4))) ;
		*(pTempZ ++) = z ;
		//
		fpSpt_Normal += vNormal.x ;
		fpPos_TexX += fpDelta_TexX ;
		fpPos_TexY += fpDelta_TexY ;
	}
}

void S3DSoftwareRenderer::rasterize_SampleTextureTileSmooth
	( S3DSoftwareRenderer::PolygonBuffer * ppb,
		S3DSoftwareRenderer::RasterizeBuffer * prb )
{
	S3DVector	vNormal = ppb->m_vNormal ;
	S3DVector	vDeltaTexX = ppb->m_vDeltaTexX ;
	S3DVector	vDeltaTexY = ppb->m_vDeltaTexY ;
	float32_t	fpBaseTexX = ppb->m_fpBaseTexX ;
	float32_t	fpBaseTexY = ppb->m_fpBaseTexY ;
	float32_t	fpTexO_Normal = ppb->m_fpTexO_Normal ;
	S3DVector	vPosScreen = prb->m_vPosScreen ;
	float32_t	fpPos_TexX = vPosScreen.InnerProduct( vDeltaTexX ) ;
	float32_t	fpPos_TexY = vPosScreen.InnerProduct( vDeltaTexY ) ;
	float32_t	fpDelta_TexX = vDeltaTexX.x * fpTexO_Normal ;
	float32_t	fpDelta_TexY = vDeltaTexY.x * fpTexO_Normal ;
	float32_t	fpSpt_Normal = prb->m_fpSpt_Normal ;
	//
	size_t		nCount = prb->m_nWidth ;
	uint32_t *	pTempTex = prb->m_pTempSrc ;
	float32_t *	pTempZ = prb->m_pTempZ ;
	uint32_t	maskTexX = ppb->m_imgTexture.width - 1 ;
	uint32_t	maskTexY = ppb->m_imgTexture.height - 1 ;
	int32_t		pitchLine = ppb->m_imgTexture.pitchLine ;
	uint8_t *	pTexture = ppb->m_imgTexture.ptrBuffer ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		float32_t	rcpSptNormal = (1.0f / fpSpt_Normal) ;
		float32_t	z = vPosScreen.z * rcpSptNormal ;
		float32_t	u = fpPos_TexX * rcpSptNormal + fpBaseTexX ;
		float32_t	v = fpPos_TexY * rcpSptNormal + fpBaseTexY ;
		int32_t		fxU = eslRoundR32ToInt( u * 256.0f ) ;
		int32_t		fxV = eslRoundR32ToInt( v * 256.0f ) ;
		int32_t		u0 = (fxU >> 8) & maskTexX ;
		int32_t		v0 = (fxV >> 8) & maskTexY ;
		int32_t		u1 = (u0 + 1) & maskTexX ;
		int32_t		v1 = (v0 + 1) & maskTexY ;
		uint32_t *	pTexL0 = (uint32_t*) (pTexture + (v0 * pitchLine)) ;
		uint32_t *	pTexL1 = (uint32_t*) (pTexture + (v1 * pitchLine)) ;
		uint32_t	px00 = pTexL0[u0] ;
		uint32_t	px01 = pTexL0[u1] ;
		uint32_t	px10 = pTexL1[u0] ;
		uint32_t	px11 = pTexL1[u1] ;
		uint32_t	dx = fxU & 0xFF ;
		uint32_t	dy = fxV & 0xFF ;
		//
		px00 = sglPackedColorMul( px00, 0x100 - dx )
						+ sglPackedColorMul( px01, dx ) ;
		px10 = sglPackedColorMul( px10, 0x100 - dx )
						+ sglPackedColorMul( px11, dx ) ;
		*(pTempTex ++) =
				sglPackedColorMul( px00, 0x100 - dy )
						+ sglPackedColorMul( px10, dy ) ;
		*(pTempZ ++) = z ;
		//
		fpSpt_Normal += vNormal.x ;
		fpPos_TexX += fpDelta_TexX ;
		fpPos_TexY += fpDelta_TexY ;
	}
}

// 頂点色補完
//////////////////////////////////////////////////////////////////////////////
void S3DSoftwareRenderer::rasterize_ComplementVertexColor
	( S3DSoftwareRenderer::RasterizeBuffer * prb )
{
	uint32_t	fxRcp = 0x4000 / (uint32_t) prb->m_nWidth ;
	S3DWColor	clrNext ;
	S3DWColor	clrDelta ;
	clrDelta.rgbMul[0] = (int16_t) 
		((((int) prb->m_rglLine.rgbaRight.rgbMul.argb.Blue
				- prb->m_rglLine.rgbaLeft.rgbMul.argb.Blue) * fxRcp) >> 7) ;
	clrDelta.rgbMul[1] = (int16_t) 
		((((int) prb->m_rglLine.rgbaRight.rgbMul.argb.Green
				- prb->m_rglLine.rgbaLeft.rgbMul.argb.Green) * fxRcp) >> 7) ;
	clrDelta.rgbMul[2] = (int16_t) 
		((((int) prb->m_rglLine.rgbaRight.rgbMul.argb.Red
				- prb->m_rglLine.rgbaLeft.rgbMul.argb.Red) * fxRcp) >> 7) ;
	clrDelta.rgbAdd[0] = (int16_t) 
		((((int) prb->m_rglLine.rgbaRight.rgbAdd.argb.Blue
				- prb->m_rglLine.rgbaLeft.rgbAdd.argb.Blue) * fxRcp) >> 7) ;
	clrDelta.rgbAdd[1] = (int16_t) 
		((((int) prb->m_rglLine.rgbaRight.rgbAdd.argb.Green
				- prb->m_rglLine.rgbaLeft.rgbAdd.argb.Green) * fxRcp) >> 7) ;
	clrDelta.rgbAdd[2] = (int16_t) 
		((((int) prb->m_rglLine.rgbaRight.rgbAdd.argb.Red
				- prb->m_rglLine.rgbaLeft.rgbAdd.argb.Red) * fxRcp) >> 7) ;
	clrNext.rgbMul[0] =
		(int16_t) prb->m_rglLine.rgbaLeft.rgbMul.argb.Blue << 7 ;
	clrNext.rgbMul[1] =
		(int16_t) prb->m_rglLine.rgbaLeft.rgbMul.argb.Green << 7 ;
	clrNext.rgbMul[2] =
		(int16_t) prb->m_rglLine.rgbaLeft.rgbMul.argb.Red << 7 ;
	clrNext.rgbAdd[0] =
		(int16_t) prb->m_rglLine.rgbaLeft.rgbAdd.argb.Blue << 7 ;
	clrNext.rgbAdd[1] =
		(int16_t) prb->m_rglLine.rgbaLeft.rgbAdd.argb.Green << 7 ;
	clrNext.rgbAdd[2] =
		(int16_t) prb->m_rglLine.rgbaLeft.rgbAdd.argb.Red << 7 ;
	//
	size_t		nCount = prb->m_nWidth ;
	S3DColor *	pTempColor = prb->m_pTempColor ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		pTempColor->rgbMul.argb.Blue = (uint8_t) (clrNext.rgbMul[0] >> 7) ;
		pTempColor->rgbMul.argb.Green = (uint8_t) (clrNext.rgbMul[1] >> 7) ;
		pTempColor->rgbMul.argb.Red = (uint8_t) (clrNext.rgbMul[2] >> 7) ;
		pTempColor->rgbAdd.argb.Blue = (uint8_t) (clrNext.rgbAdd[0] >> 7) ;
		pTempColor->rgbAdd.argb.Green = (uint8_t) (clrNext.rgbAdd[1] >> 7) ;
		pTempColor->rgbAdd.argb.Red = (uint8_t) (clrNext.rgbAdd[2] >> 7) ;
		pTempColor ++ ;
		//
		clrNext.rgbMul[0] += clrDelta.rgbMul[0] ;
		clrNext.rgbMul[1] += clrDelta.rgbMul[1] ;
		clrNext.rgbMul[2] += clrDelta.rgbMul[2] ;
		clrNext.rgbAdd[0] += clrDelta.rgbAdd[0] ;
		clrNext.rgbAdd[1] += clrDelta.rgbAdd[1] ;
		clrNext.rgbAdd[2] += clrDelta.rgbAdd[2] ;
	}
}

// 色効果処理
//////////////////////////////////////////////////////////////////////////////
void S3DSoftwareRenderer::rasterize_EffectVertexColor
	( uint32_t * prgbaDst, const uint32_t * prgbaSrc,
				const S3DColor * pSrcColors, size_t nCount )
{
	for ( size_t i = 0; i < nCount; i ++ )
	{
		uint32_t	rgbaSrc = *(prgbaSrc ++) ;
		uint32_t	b = (((rgbaSrc & 0xFF) + 1)
							* pSrcColors->rgbMul.argb.Blue) >> 8 ;
		uint32_t	g = ((((rgbaSrc >> 8) & 0xFF) + 1)
							* pSrcColors->rgbMul.argb.Green) >> 8 ;
		uint32_t	r = ((((rgbaSrc >> 16) & 0xFF) + 1)
							* pSrcColors->rgbMul.argb.Red) >> 8 ;
		rgbaSrc = (b | (g << 8)) | ((r << 16) | (rgbaSrc & 0xFF000000)) ;
		*(prgbaDst ++) =
			sglPackedColorAdd
				( rgbaSrc, sglPackedColorMul
					( pSrcColors->rgbAdd.ui32, (rgbaSrc >> 24) + 1 ) ) ;
		pSrcColors ++ ;
	}
}

void S3DSoftwareRenderer::rasterize_ConvertVertexColor
	( uint32_t * prgbaDst,
		const S3DColor * pSrcColors, size_t nCount )
{
	for ( size_t i = 0; i < nCount; i ++ )
	{
		uint32_t	rgbaSrc = pSrcColors->rgbAdd.ui32 ;
		uint32_t	a = (uint32_t) pSrcColors->rgbMul.argb.Blue
									+ pSrcColors->rgbMul.argb.Green
									+ pSrcColors->rgbMul.argb.Green
									+ pSrcColors->rgbMul.argb.Red ;
		*(prgbaDst ++) = rgbaSrc | (~(a >> 2) << 24) ;
		pSrcColors ++ ;
	}
}

// 書き出し
//////////////////////////////////////////////////////////////////////////////
void S3DSoftwareRenderer::rasterize_WriteSimple
			( S3DSoftwareRenderer::RasterizeBuffer * prb )
{
	uint32_t *			prgbaDst = prb->m_pDstColor ;
	const uint32_t *	prgbaSrc = prb->m_pTempSrc ;
	size_t				nCount = prb->m_nWidth ;
	//
	for ( size_t i = 0; i < nCount; i ++ )
	{
		uint32_t	rgbaSrc = prgbaSrc[i] ;
		if ( rgbaSrc != 0 )
		{
			if ( (rgbaSrc >> 24) >= 0xFE )
			{
				prgbaDst[i] = rgbaSrc | 0xFF000000 ;
			}
			else
			{
				prgbaDst[i] = sglPackedColorBlend( prgbaDst[i], prgbaSrc[i] ) ;
			}
		}
	}
}

void S3DSoftwareRenderer::rasterize_WriteCompareZ
			( S3DSoftwareRenderer::RasterizeBuffer * prb )
{
	uint32_t *			prgbaDst = prb->m_pDstColor ;
	const int32_t *		pDstZ = (const int32_t *) prb->m_pDstZBuf ;
	const uint32_t *	prgbaSrc = prb->m_pTempSrc ;
	const int32_t *		pSrcZ = (const int32_t *) prb->m_pTempZ ;
	size_t				nCount = prb->m_nWidth ;
	//
	for ( size_t i = 0; i < nCount; i ++ )
	{
		int32_t	zDst = pDstZ[i] ;
		int32_t	zSrc = pSrcZ[i] ;
		if ( (zDst ^ ((zDst >> 31) & 0x7FFFFFFF))
				< (zSrc ^ ((zSrc >> 31) & 0x7FFFFFFF)) )
		{
			continue ;
		}
		uint32_t	rgbaSrc = prgbaSrc[i] ;
		if ( rgbaSrc != 0 )
		{
			if ( (rgbaSrc >> 24) >= 0xFE )
			{
				prgbaDst[i] = rgbaSrc | 0xFF000000 ;
			}
			else
			{
				prgbaDst[i] = sglPackedColorBlend( prgbaDst[i], prgbaSrc[i] ) ;
			}
		}
	}
}

void S3DSoftwareRenderer::rasterize_WriteWithZ
			( S3DSoftwareRenderer::RasterizeBuffer * prb )
{
	uint32_t *			prgbaDst = prb->m_pDstColor ;
	int32_t *			pDstZ = (int32_t*) prb->m_pDstZBuf ;
	const uint32_t *	prgbaSrc = prb->m_pTempSrc ;
	const int32_t *		pSrcZ = (const int32_t*) prb->m_pTempZ ;
	size_t				nCount = prb->m_nWidth ;
	//
	for ( size_t i = 0; i < nCount; i ++ )
	{
		int32_t	zDst = pDstZ[i] ;
		int32_t	zSrc = pSrcZ[i] ;
		if ( (zDst ^ ((zDst >> 31) & 0x7FFFFFFF))
				< (zSrc ^ ((zSrc >> 31) & 0x7FFFFFFF)) )
		{
			continue ;
		}
		uint32_t	rgbaSrc = prgbaSrc[i] ;
		if ( rgbaSrc != 0 )
		{
			if ( (rgbaSrc >> 24) >= 0xFE )
			{
				prgbaDst[i] = rgbaSrc | 0xFF000000 ;
			}
			else
			{
				prgbaDst[i] = sglPackedColorBlend( prgbaDst[i], prgbaSrc[i] ) ;
			}
			pDstZ[i] = zSrc ;
		}
	}
}


//////////////////////////////////////////////////////////////////////////////
// ソフトウェア・レンダラ（バッファリング：ソート／ステレオ立体視対応）
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DSoftwareBufferedRenderer, S3DRenderBufferedContext )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DSoftwareBufferedRenderer::S3DSoftwareBufferedRenderer( void )
{
	m_flagDelayUpdate = false ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DSoftwareBufferedRenderer::~S3DSoftwareBufferedRenderer( void )
{
}

// 現在の2D描画変換をS3DSoftwareRendererに反映
//////////////////////////////////////////////////////////////////////////////
void S3DSoftwareBufferedRenderer::Reflect2DTransformation( void )
{
	SGLAffine	affine ;
	CurrentAffine( affine ) ;
	m_render.SetTransformation( affine, CurrentTransparency() ) ;
}

// 描画先設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSoftwareBufferedRenderer::AttachTargetImage
	( SGLImageObject * pImage,
			SGLImageObject * pZBuffer, const SGLImageRect * pView )
{
	m_render.AttachTargetImage( pImage, pZBuffer, pView ) ;
	return	S3DRenderBufferedContext::
				AttachTargetImage( pImage, pZBuffer, pView ) ;
}

// 描画先解除
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSoftwareBufferedRenderer::DetachTargetImage( void )
{
	S3DSoftwareBufferedRenderer::Finish() ;
	//
	S3DRenderBufferedContext::DetachTargetImage() ;
	return	m_render.DetachTargetImage() ;
}

// 描画デフォルトフラグ
//////////////////////////////////////////////////////////////////////////////
void S3DSoftwareBufferedRenderer::SetPaintFlags( int64_t nFlags )
{
	S3DRenderBufferedContext::SetPaintFlags( nFlags ) ;
	m_render.SetPaintFlags( nFlags ) ;
}

// 描画先クリア
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSoftwareBufferedRenderer::FillClearTarget( uint32_t argb, int64_t flags )
{
	m_flagDelayUpdate = true ;
	return	S3DRenderBufferedContext::FillClearTarget( argb, flags ) ;
}

// 形状描画
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSoftwareBufferedRenderer::FillRectangle
	( int x, int y, int width, int height,
		uint32_t argb, double z, uint32_t flags )
{
	if ( m_flagDelayUpdate )
	{
		S3DSoftwareBufferedRenderer::Flush() ;
	}
	Reflect2DTransformation() ;
	return	m_render.FillRectangle( x, y, width, height, argb, z, flags ) ;
}

SGLError S3DSoftwareBufferedRenderer::FillPolygon
	( const S2DVector * vertices, size_t count,
		uint32_t argb, double z, uint32_t flags )
{
	if ( m_flagDelayUpdate )
	{
		S3DSoftwareBufferedRenderer::Flush() ;
	}
	Reflect2DTransformation() ;
	return	m_render.FillPolygon( vertices, count, argb, z, flags ) ;
}

// 画像描画
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSoftwareBufferedRenderer::DrawImage
	( const SGLPaintParam & ppPaint,
		SGLImageObject * pSrcImage,
		const SGLImageRect * pSrcClip )
{
	if ( m_flagDelayUpdate )
	{
		S3DSoftwareBufferedRenderer::Flush() ;
	}
	Reflect2DTransformation() ;
	return	m_render.DrawImage( ppPaint, pSrcImage, pSrcClip ) ;
}

// ２Ｄメッシュ描画
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSoftwareBufferedRenderer::DrawMesh
	( const S2DVector * pDstMesh,
		const S2DVector * pSrcMesh,
		size_t widthMesh, size_t heightMesh,
		const SGLPaintParam & ppPaint,
		SGLImageObject * pSrcImage,
		const SGLImageRect * pSrcClip )
{
	if ( m_flagDelayUpdate )
	{
		S3DSoftwareBufferedRenderer::Flush() ;
	}
	Reflect2DTransformation() ;
	return	m_render.DrawMesh
		( pDstMesh, pSrcMesh,
			widthMesh, heightMesh, ppPaint, pSrcImage, pSrcClip ) ;
}

// バッファ複製
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSoftwareBufferedRenderer::CopyBufferFrom
	( S3DRenderContextInterface& renderSrc, uint32_t nFlags,
		int xDst, int yDst, const SGLImageRect * pSrcRect )
{
	if ( m_flagDelayUpdate )
	{
		S3DSoftwareBufferedRenderer::Flush() ;
	}
	return	m_render.CopyBufferFrom( renderSrc, nFlags, xDst, yDst, pSrcRect ) ;
}

// 投影スクリーン座標設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSoftwareBufferedRenderer::SetProjectionScreen
	( const S3DVector& vScreen, double zScale, double fpPixelAspect )
{
	S3DRenderBufferedContext::SetProjectionScreen( vScreen, zScale, fpPixelAspect ) ;
	return	m_render.SetProjectionScreen( vScreen, zScale, fpPixelAspect ) ;
}

// カメラ設定
//////////////////////////////////////////////////////////////////////////////
void S3DSoftwareBufferedRenderer::SetCamera
	( const S3DDMatrix& matCamera, const S3DDVector& posCamera )
{
	S3DRenderBufferedContext::SetCamera( matCamera, posCamera ) ;
	m_render.SetCamera( matCamera, posCamera ) ;
}

// 立体視視差設定
//////////////////////////////////////////////////////////////////////////////
void S3DSoftwareBufferedRenderer::SetParallax
		( double xParallax, double zFocusRate, double xScreenDelta )
{
	S3DRenderBufferedContext::SetParallax( xParallax, zFocusRate, xScreenDelta ) ;
	m_render.SetParallax( xParallax, zFocusRate, xScreenDelta ) ;
}

// ｚクリップ範囲を設定
//////////////////////////////////////////////////////////////////////////////
void S3DSoftwareBufferedRenderer::SetZClipRange( double zMin, double zMax )
{
	S3DRenderBufferedContext::SetZClipRange( zMin, zMax ) ;
	m_render.SetZClipRange( zMin, zMax ) ;
}

// 光源を設定
//////////////////////////////////////////////////////////////////////////////
void S3DSoftwareBufferedRenderer::SetLightEntries
	( const S3DLightEntry* pLights, size_t countLight )
{
//	S3DRenderBufferedContext::SetLightEntries( pLights, countLight ) ;
	m_render.SetLightEntries( pLights, countLight ) ;
}

// シャドウマップを設定
//////////////////////////////////////////////////////////////////////////////
void S3DSoftwareBufferedRenderer::SetShadowMap
	( uint32_t idLight,
		SGLImageObject* pShadowMapDepth,
		const S3DShadowMapInfo& infShadowMap,
		SGLImageObject* pShadowMapColor )
{
//	S3DRenderBufferedContext::SetLightEntries( pLights, countLight ) ;
	m_render.SetShadowMap
		( idLight, pShadowMapDepth, infShadowMap, pShadowMapColor ) ;
}

// 疑似フォッグを設定
//////////////////////////////////////////////////////////////////////////////
void S3DSoftwareBufferedRenderer::SetFog
	( uint32_t rgbFog, double zFogNear, double zFogFar )
{
	S3DRenderBufferedContext::SetFog( rgbFog, zFogNear, zFogFar ) ;
	m_render.SetFog( rgbFog, zFogNear, zFogFar ) ;
}

void S3DSoftwareBufferedRenderer::EnableFog( bool fFog )
{
	S3DRenderBufferedContext::EnableFog( fFog ) ;
	m_render.EnableFog( fFog ) ;
}

// シェーディング設定
//////////////////////////////////////////////////////////////////////////////
void S3DSoftwareBufferedRenderer::SetShadingFlag( uint64_t nShadingMethod )
{
	S3DRenderBufferedContext::SetShadingFlag( nShadingMethod ) ;
	m_render.SetShadingFlag( nShadingMethod ) ;
}

// レイトレーシング設定
//////////////////////////////////////////////////////////////////////////////
void S3DSoftwareBufferedRenderer::SetRayTracingParameter
			( const S3DRenderRayTracingParam& rrtp )
{
	S3DRenderBufferedContext::SetRayTracingParameter( rrtp ) ;
	m_render.SetRayTracingParameter( rrtp ) ;
}

// グローバル環境マッピング設定
//////////////////////////////////////////////////////////////////////////////
void S3DSoftwareBufferedRenderer::SetEnvironmentMappingImage
		( SGLImageObject * pImage, uint32_t nFlags )
{
	S3DRenderBufferedContext::SetEnvironmentMappingImage( pImage, nFlags ) ;
	m_render.SetEnvironmentMappingImage( pImage, nFlags ) ;
}

// グローバル環境マッピング変換行列設定
//////////////////////////////////////////////////////////////////////////////
void S3DSoftwareBufferedRenderer::SetEnvironmentMappingMatrix( const S3DMatrix& matMapping )
{
	S3DRenderBufferedContext::SetEnvironmentMappingMatrix( matMapping ) ;
	m_render.SetEnvironmentMappingMatrix( matMapping ) ;
}

// 輪郭描画色設定
//////////////////////////////////////////////////////////////////////////////
void S3DSoftwareBufferedRenderer::SetOffsetBorderColor( uint32_t rgbBorder )
{
	S3DRenderBufferedContext::SetOffsetBorderColor( rgbBorder ) ;
	m_render.SetOffsetBorderColor( rgbBorder ) ;
}

// 輪郭描画オフセット係数設定
//////////////////////////////////////////////////////////////////////////////
void S3DSoftwareBufferedRenderer::SetOffsetBorderCoefficient( float32_t a, float32_t b )
{
	S3DRenderBufferedContext::SetOffsetBorderCoefficient( a, b ) ;
	m_render.SetOffsetBorderCoefficient( a, b ) ;
}

// ポリゴンリストをレンダリングバッファに追加
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSoftwareBufferedRenderer::AddIndexedTriangleList
	( S3DMaterial * pMaterial, uint32_t nFlags,
		size_t countPolygon, size_t countVertex,
		const S3DVector4 * pvVertex,
		const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap,
		const S3DColor * pColor,
		const uint32_t * pIndexedList )
{
	m_flagDelayUpdate = true ;
	//
	return	S3DRenderBufferedContext::AddIndexedTriangleList
				( pMaterial, nFlags, countPolygon, countVertex,
					pvVertex, pvNormal, pvUVMap, pColor, pIndexedList ) ;
}

// トライアングルストリップをレンダリングバッファに追加
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSoftwareBufferedRenderer::AddTriangleStrip
	( S3DMaterial * pMaterial, uint32_t nFlags,
		size_t countTriangleStrip,
		const S3DVector4 * pvVertex,
		const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap, const S3DColor * pColor )
{
	m_flagDelayUpdate = true ;
	//
	return	S3DRenderBufferedContext::AddTriangleStrip
				( pMaterial, nFlags, countTriangleStrip,
						pvVertex, pvNormal, pvUVMap, pColor ) ;
}

// プリミティブリストをレンダリングバッファに追加
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSoftwareBufferedRenderer::AddIndexedPrimitiveList
	( S3DMaterial * pMaterial, uint32_t nFlags,
		S3DPrimitiveType typePrimitive,
		size_t countIndex, size_t countVertex,
		const S3DVector4 * pvVertex,
		const S3DVector4 * pvNormal,
		const S2DVector * pvUVMap,
		const S3DColor * pColor,
		const uint32_t * pIndexedList )
{
	m_flagDelayUpdate = true ;
	//
	return	S3DRenderBufferedContext::AddIndexedPrimitiveList
				( pMaterial, nFlags, typePrimitive,
					countIndex, countVertex,
					pvVertex, pvNormal, pvUVMap, pColor, pIndexedList ) ;
}

// 頂点バッファの内容を描画
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSoftwareBufferedRenderer::AddVertexBuffer
	( S3DMaterial * pMaterial, uint32_t nFlags,
		S3DVertexBufferInterface * pBuffer, size_t iFirst, ssize_t iEnd,
		size_t nInstancing,
		const S4DMatrix * pmatInstancing,
		const S3DColor * pColorInstancing )
{
	m_flagDelayUpdate = true ;
	//
	return	S3DRenderBufferedContext::AddVertexBuffer
				( pMaterial, nFlags, pBuffer, iFirst, iEnd,
					nInstancing, pmatInstancing, pColorInstancing ) ;
}

// 描画の確定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSoftwareBufferedRenderer::Flush( void )
{
	S3DRenderBufferedContext::Flush() ;
	m_flagDelayUpdate = false ;
	//
	if ( !IsEmptyRenderBuffer() )
	{
		FlushRenderAllViewBufferTo( &m_render ) ;
		ClearAllViewBuffer() ;
	}
	return	m_render.Flush() ;
}

SGLError S3DSoftwareBufferedRenderer::Finish( void )
{
	S3DRenderBufferedContext::Finish() ;
	m_flagDelayUpdate = false ;
	//
	if ( !IsEmptyRenderBuffer() )
	{
		FlushRenderAllViewBufferTo( &m_render ) ;
		ClearAllViewBuffer() ;
	}
	return	m_render.Finish() ;
}

// 対応機能取得
//////////////////////////////////////////////////////////////////////////////
void S3DSoftwareBufferedRenderer::GetRenderingCapacity( S3DRenderingCapacity& caps )
{
	m_render.GetRenderingCapacity( caps ) ;
	//
	caps.flagsRendering |= S3DRenderingCapacity::renderingAutoStereo
							| S3DRenderingCapacity::renderingSortBuffered ;
}

// 内部バッファサイズ設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSoftwareBufferedRenderer::SetRenderingBufferSize( uint32_t countVertex )
{
	S3DRenderBufferedContext::SetRenderingBufferSize( countVertex ) ;
	//
	return	m_render.SetRenderingBufferSize( countVertex ) ;
}

// ハードウェア描画オブジェクト取得
//////////////////////////////////////////////////////////////////////////////
S3DRenderDevice * S3DSoftwareBufferedRenderer::GetRenderDeviceObject( uint64_t nFlags )
{
	return	m_render.GetRenderDeviceObject( nFlags ) ;
}

// 3D レンダリング用バッファ・インターフェース開始
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSoftwareBufferedRenderer::Begin3DRenderer( uint64_t nFlags )
{
	return	m_render.Begin3DRenderer( nFlags ) ;
}

// 3D レンダリング用バッファ・インターフェース終了
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSoftwareBufferedRenderer::End3DRenderer( uint64_t nFlags )
{
	return	m_render.End3DRenderer( nFlags ) ;
}

// 点描画
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSoftwareBufferedRenderer::DrawPoints
	( const S2DVector * pPoints, size_t nPoints,
		uint32_t argb, double z, uint32_t flags )
{
	if ( m_flagDelayUpdate )
	{
		S3DSoftwareBufferedRenderer::Flush() ;
	}
	Reflect2DTransformation() ;
	return	m_render.DrawPoints( pPoints, nPoints, argb, z, flags ) ;
}

// 直線描画
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSoftwareBufferedRenderer::DrawThinLines
	( const S2DVector * pLines, size_t nLines,
			uint32_t argb, double z, uint32_t flags )
{
	if ( m_flagDelayUpdate )
	{
		S3DSoftwareBufferedRenderer::Flush() ;
	}
	Reflect2DTransformation() ;
	return	m_render.DrawThinLines( pLines, nLines, argb, z, flags ) ;
}

// グラデーション解除
//////////////////////////////////////////////////////////////////////////////
void S3DSoftwareBufferedRenderer::FreeGradation( void )
{
	m_render.FreeGradation() ;
}

// 線形グラデーション設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSoftwareBufferedRenderer::SetLinearGradation
	( float32_t x0, float32_t y0, float32_t x1, float32_t y1,
			const SGLPalette * pGradation, size_t nCount )
{
	return	m_render.SetLinearGradation
				( x0, y0, x1, y1, pGradation, nCount ) ;
}

// 環状グラデーション設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DSoftwareBufferedRenderer::SetRingedGradation
	( float32_t xCenter, float32_t yCenter, float32_t radAngle,
		const SGLPalette * pGradation, size_t nCount )
{
	return	m_render.SetRingedGradation
				( xCenter, yCenter, radAngle, pGradation, nCount ) ;
}

