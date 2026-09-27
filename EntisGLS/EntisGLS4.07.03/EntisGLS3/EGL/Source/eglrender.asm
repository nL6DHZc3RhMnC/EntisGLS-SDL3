
; ----------------------------------------------------------------------------
;                          Entis Graphic Library
; ----------------------------------------------------------------------------
;   Copyright (c) 2002-2011 Leshade Entis, Entis-soft. Al rights reserved.
; ----------------------------------------------------------------------------


	.686
	.XMM
	.MODEL	FLAT

	INCLUDE	experi.inc
	INCLUDE	egl.inc


; ----------------------------------------------------------------------------
;	データセグメント
; ----------------------------------------------------------------------------

ConstSeg	SEGMENT	PARA READONLY FLAT 'CONST'

ALIGN	10H
xmmMaskLP3	DWORD	0FFFFFFFFH, 0FFFFFFFFH, 0FFFFFFFFH, 0
xmmMaskLS1	DWORD	80000000H, 0, 0, 0
realConst256	REAL4	256.0
realConstHalf	REAL4	0.5

ConstSeg	ENDS


; ----------------------------------------------------------------------------
;	コードセグメント
; ----------------------------------------------------------------------------

CodeSeg	SEGMENT	PARA READONLY FLAT 'CODE'

;
;	レンダリングオブジェクトを作成
; ----------------------------------------------------------------------------
ALIGN	10H
eglCreateRenderPolygon		PROC	NEAR32 C USES ebx esi edi

	LOCAL	hHeap:HESLHEAP

	;
	; ヒープを作成し、メモリを確保
	;
	.IF	EGL_hImageHeap == NULL
		INVOKE	eslHeapCreate , 0, 0, ESL_HEAP_ZERO_INIT, NULL
		mov	EGL_hImageHeap, eax
	.ENDIF
	INVOKE	eslHeapCreate , 2000H, 0, ESL_HEAP_NO_SERIALIZE, EGL_hImageHeap
	mov	hHeap, eax
	;
	INVOKE	eslHeapAllocate ,
		hHeap, (SIZEOF EGL_RENDER_POLYGON_BUF) + 10H, ESL_HEAP_ZERO_INIT
	add	eax, 0FH
	and	eax, NOT 0FH
	mov	ebx, eax
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	;
	; 関数ポインタを初期化
	;
	mov	[ebx].dib.pfnRelease, OFFSET eglDrawImage@Release_Dummy
	mov	[ebx].dib.pfnInitialize, OFFSET eglDrawImage@Initialize_Dummy
	mov	[ebx].dib.pfnGetDestination, OFFSET eglDrawImage@GetDestination
	mov	[ebx].dib.pfnGetFunctionFlags, OFFSET eglDrawImage@GetFunctionFlags
	mov	[ebx].dib.pfnSetFunctionFlags, OFFSET eglDrawImage@SetFunctionFlags
	mov	[ebx].dib.pfnGetDrawingOffset, OFFSET eglDrawImage@GetDrawingOffset
	mov	[ebx].dib.pfnSetDrawingOffset, OFFSET eglRenderPoly@SetDrawingOffset
	mov	[ebx].dib.pfnPrepareDraw, OFFSET eglDrawImage@PrepareDraw
	mov	[ebx].dib.pfnDrawImage, OFFSET eglDrawImage@DrawImage_NotSupported
	mov	[ebx].dib.pfnPrepareLine, OFFSET eglDrawImage@PrepareLine
	mov	[ebx].dib.pfnPrepareFillRect, OFFSET eglDrawImage@PrepareFillRect
	mov	[ebx].dib.pfnPrepareFillEllipse, OFFSET eglDrawImage@PrepareFillEllipse
	mov	[ebx].dib.pfnPrepareFillPolygon, OFFSET eglDrawImage@PrepareFillPolygon
	mov	[ebx].dib.pfnFillRegion, OFFSET eglDrawImage@FillRegion_Error
	mov	[ebx].dib.pfnDrawRegion, OFFSET eglDrawImage@FillRegion_Error
	;
	mov	[ebx].rp.pfnRelease, OFFSET eglRenderPoly@Release
	mov	[ebx].rp.pfnInitialize, OFFSET eglRenderPoly@Initialize
	mov	[ebx].rp.pfnInitializeToReference, OFFSET eglRenderPoly@InitializeToReference
	mov	[ebx].rp.pfnGetScreenPos, OFFSET eglRenderPoly@GetScreenPos
	mov	[ebx].rp.pfnGetDrawImage, OFFSET eglRenderPoly@GetDrawImage
	mov	[ebx].rp.pfnGetFunctionFlags, OFFSET eglRenderPoly@GetFunctionFlags
	mov	[ebx].rp.pfnSetFunctionFlags, OFFSET eglRenderPoly@SetFunctionFlags
	mov	[ebx].rp.pfnSetZClipRange, OFFSET eglRenderPoly@SetZClipRange
	mov	[ebx].rp.pfnSetEnvironmentMapping, OFFSET eglRenderPoly@SetEnvironmentMapping
	mov	[ebx].rp.pfnPrepareLight, OFFSET eglRenderPoly@PrepareLight
	mov	[ebx].rp.pfnSortPolygonEntry, OFFSET eglRenderPoly@SortPolygonEntry486
	mov	[ebx].rp.pfnPrepareRender, OFFSET eglRenderPoly@PrepareRender
	mov	[ebx].rp.pfnPrepareRenderParam, OFFSET eglRenderPoly@PrepareRenderParam
	mov	[ebx].rp.pfnRenderPolygon, OFFSET eglRenderPoly@RenderPolygon_Err
	mov	[ebx].rp.pfnAllocateRayTraceRect, OFFSET eglRenderPoly@AllocateRayTraceRect
	;
IF	0
	mov	[ebx].rp.pfnAttachGPUInterface, OFFSET eglRenderPoly@AttachGPUInterface
	mov	[ebx].rp.pfnGetResultToTraceRays, OFFSET eglRenderPoly@GetResultToTraceRays
	mov	[ebx].rp.pfnBeginTraceRays, OFFSET eglRenderPoly@BeginTraceRays
	mov	[ebx].rp.pfnBeginTraceRaysRect, OFFSET eglRenderPoly@BeginTraceRaysRect
	mov	[ebx].rp.pfnShadeByRayTrace, OFFSET eglRenderPoly@ShadeByRayTrace486
	mov	[ebx].rp.pfnCountOfRayHitEntry, OFFSET eglRenderPoly@CountOfRayHitEntry
ENDIF
	;
	.IF	ERI_EnabledProcessorType & ERI_USE_XMM_P3
		mov	[ebx].rp.pfnPrepareMatrix, OFFSET eglRenderPoly@PrepareMatrixSSE
		mov	[ebx].rp.pfnRevolveMatrix, OFFSET eglRenderPoly@RevolveMatrixSSE
		mov	[ebx].rp.pfnCreatePolygonEntry, OFFSET eglRenderPoly@CretaePolygonEntrySSE
		mov	[ebx].rp.pfnMakeUpPolygon, OFFSET eglRenderPoly@MakeUpPolygonSSE
		mov	[ebx].rp.pfnApplyAttribute, OFFSET eglRenderPoly@ApplyAttributeSSE
		mov	[ebx].rp.pfnGetExternalRect, OFFSET eglRenderPoly@GetExternalRectSSE
		mov	[ebx].rp.pfnShadeReflectLights, OFFSET eglRenderPoly@ShadeVectorsProcSSE
		mov	[ebx].rp.pfnProjectScreen, OFFSET eglRenderPoly@ProjectScreenSSE
		mov	[ebx].rp.pfnPrepareRender, OFFSET eglRenderPoly@PrepareRenderSSE
		;
		mov	[ebx].rp.pfnSetRayTracingParameter, OFFSET eglRenderPoly@SetRayTracingParameterSSE
		mov	[ebx].rp.pfnAttachRayTracingTarget, OFFSET eglRenderPoly@AttachRayTracingTargetSSE
		mov	[ebx].rp.pfnCreatePolygonEntryRT, OFFSET eglRenderPoly@CreatePolygonEntryRT_SSE
		mov	[ebx].rp.pfnPrepareRenderRayTracing, OFFSET eglRenderPoly@PrepareRenderRayTracing_SSE
		mov	[ebx].rp.pfnRenderRayTracing, OFFSET eglRenderPoly@RenderRayTracing_SSE
		mov	[ebx].rp.pfnAbortRenderRayTracing, OFFSET eglRenderPoly@AbortRenderRayTracing_SSE
		mov	[ebx].rp.pfnGetProgressRayTracing, OFFSET eglRenderPoly@GetProgressRayTracing_SSE
		;
IF	0
		mov	[ebx].rp.pfnGetResultToTraceRays, OFFSET eglRenderPoly@GetResultToTraceRaysSSE
		mov	[ebx].rp.pfnBeginTraceRays, OFFSET eglRenderPoly@BeginTraceRaysSSE
		mov	[ebx].rp.pfnBeginTraceRaysRect, OFFSET eglRenderPoly@BeginTraceRaysRectSSE
		mov	[ebx].rp.pfnShadeByRayTrace, OFFSET eglRenderPoly@ShadeByRayTraceSSE
ENDIF
	.ELSE
		mov	[ebx].rp.pfnPrepareMatrix, OFFSET eglRenderPoly@PrepareMatrix486
		mov	[ebx].rp.pfnRevolveMatrix, OFFSET eglRenderPoly@RevolveMatrix486
		mov	[ebx].rp.pfnCreatePolygonEntry, OFFSET eglRenderPoly@CretaePolygonEntry486
		mov	[ebx].rp.pfnMakeUpPolygon, OFFSET eglRenderPoly@MakeUpPolygon486
		mov	[ebx].rp.pfnApplyAttribute, OFFSET eglRenderPoly@ApplyAttribute486
		mov	[ebx].rp.pfnGetExternalRect, OFFSET eglRenderPoly@GetExternalRect486
		mov	[ebx].rp.pfnShadeReflectLights, OFFSET eglRenderPoly@ShadeVectors486
		mov	[ebx].rp.pfnProjectScreen, OFFSET eglRenderPoly@ProjectScreen486
		;
		mov	[ebx].rp.pfnSetRayTracingParameter, OFFSET eglRenderPoly@SetRayTracingParameter486
		mov	[ebx].rp.pfnAttachRayTracingTarget, OFFSET eglRenderPoly@AttachRayTracingTarget486
		mov	[ebx].rp.pfnCreatePolygonEntryRT, OFFSET eglRenderPoly@CreatePolygonEntryRT_486
		mov	[ebx].rp.pfnPrepareRenderRayTracing, OFFSET eglRenderPoly@PrepareRenderRayTracing_486
		mov	[ebx].rp.pfnRenderRayTracing, OFFSET eglRenderPoly@RenderRayTracing_486
		mov	[ebx].rp.pfnAbortRenderRayTracing, OFFSET eglRenderPoly@AbortRenderRayTracing_486
		mov	[ebx].rp.pfnGetProgressRayTracing, OFFSET eglRenderPoly@GetProgressRayTracing_486
	.ENDIF
	;
	; 変数を初期化
	;
	mov	eax, hHeap
	mov	[ebx].dib.hHeap, eax
	;
	mov	[ebx].dwMaskShadingFlags, \
			NOT (E3DSAF_RAY_SHADOWING \
				OR E3DSAF_RAY_REFLECTING \
				OR E3DSAF_RAY_REFRACTING)
	mov	[ebx].rZMinClip, 3F800000H	; = 1.0
	mov	[ebx].rZMaxClip, 4F800000H	; = 2.0 ^ 32
	;
	mov	[ebx].hStackHeap, 0

	ASSUME	ebx:NOTHING
	mov	eax, ebx
	ret

eglCreateRenderPolygon		ENDP

;
;	レンダリングオブジェクトダミー関数
; ----------------------------------------------------------------------------
eglDrawImage@Release_Dummy	PROC	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE

	xor	eax, eax
	ret

eglDrawImage@Release_Dummy	ENDP

eglDrawImage@Initialize_Dummy	PROC	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE, pDstImage:PEGL_IMAGE_INFO,
	pClipRect:PCEGL_RECT, pZBuffer:PEGL_IMAGE_INFO

	xor	eax, eax
	ret

eglDrawImage@Initialize_Dummy	ENDP

;
;	描画オフセット座標設定
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@SetDrawingOffset		PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE, pDrawOffset:PTR EGL_POINT

	xor	eax, eax
	mov	ebx, hDrawImage
	lea	eax, (EGL_RENDER_POLYGON_BUF PTR [eax]).dib
	mov	esi, pDrawOffset
	sub	ebx, eax
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	ASSUME	esi:PTR EGL_POINT

	mov	eax, [esi].x
	mov	edx, [esi].y
	mov	[ebx].dib.ptDrawOffset.x, eax
	mov	[ebx].dib.ptDrawOffset.y, edx
	;
	fld	[ebx].vPureScreenPos.x
	fiadd	[ebx].dib.ptDrawOffset.x
	fstp	[ebx].vScreenPos.x
	fld	[ebx].vPureScreenPos.y
	fiadd	[ebx].dib.ptDrawOffset.y
	fstp	[ebx].vScreenPos.y

	ASSUME	ebx:NOTHING
	ASSUME	esi:NOTHING

	ret

eglRenderPoly@SetDrawingOffset		ENDP

;
;	レンダリングオブジェクト開放
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@Release		PROC	NEAR32 C USES ebx esi edi,
	hRenderPoly:HEGL_RENDER_POLYGON

	mov	ebx, hRenderPoly
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF

	.IF	[ebx].hStackHeap != NULL
		INVOKE	eslStackHeapDestroy , [ebx].hStackHeap
	.ENDIF
	INVOKE	eslHeapDestroy , [ebx].dib.hHeap

	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglRenderPoly@Release		ENDP

;
;	レンダリングオブジェクト初期化
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@Initialize	PROC	NEAR32 C USES ebx esi edi,
	hRenderPoly:HEGL_RENDER_POLYGON,
	pDstImage:PEGL_IMAGE_INFO, pClipRect:PCEGL_RECT,
	pZBuffer:PEGL_IMAGE_INFO, pScreenPos:PCE3D_VECTOR

	LOCAL	nLineWidth:DWORD

	mov	ebx, hRenderPoly
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	;
	; DrawImage オブジェクトを初期化する
	;
	INVOKE	eglDrawImage@Initialize ,
			ADDR [ebx].dib, pDstImage, pClipRect, pZBuffer
	;
	; 関数ポインタ初期化
	;
	.IF	ERI_EnabledProcessorType & ERI_USE_XMM_P3
		mov	[ebx].rp.pfnPrepareMatrix, OFFSET eglRenderPoly@PrepareMatrixSSE
		mov	[ebx].rp.pfnRevolveMatrix, OFFSET eglRenderPoly@RevolveMatrixSSE
		mov	[ebx].rp.pfnCreatePolygonEntry, OFFSET eglRenderPoly@CretaePolygonEntrySSE
		mov	[ebx].rp.pfnMakeUpPolygon, OFFSET eglRenderPoly@MakeUpPolygonSSE
		mov	[ebx].rp.pfnApplyAttribute, OFFSET eglRenderPoly@ApplyAttributeSSE
		mov	[ebx].rp.pfnGetExternalRect, OFFSET eglRenderPoly@GetExternalRectSSE
		mov	[ebx].rp.pfnShadeReflectLights, OFFSET eglRenderPoly@ShadeVectorsProcSSE
		mov	[ebx].rp.pfnProjectScreen, OFFSET eglRenderPoly@ProjectScreenSSE
		mov	[ebx].rp.pfnPrepareRender, OFFSET eglRenderPoly@PrepareRenderSSE
		;
		mov	[ebx].rp.pfnSetRayTracingParameter, OFFSET eglRenderPoly@SetRayTracingParameterSSE
		mov	[ebx].rp.pfnAttachRayTracingTarget, OFFSET eglRenderPoly@AttachRayTracingTargetSSE
		mov	[ebx].rp.pfnCreatePolygonEntryRT, OFFSET eglRenderPoly@CreatePolygonEntryRT_SSE
		mov	[ebx].rp.pfnPrepareRenderRayTracing, OFFSET eglRenderPoly@PrepareRenderRayTracing_SSE
		mov	[ebx].rp.pfnRenderRayTracing, OFFSET eglRenderPoly@RenderRayTracing_SSE
		mov	[ebx].rp.pfnAbortRenderRayTracing, OFFSET eglRenderPoly@AbortRenderRayTracing_SSE
		mov	[ebx].rp.pfnGetProgressRayTracing, OFFSET eglRenderPoly@GetProgressRayTracing_SSE
	.ELSE
		mov	[ebx].rp.pfnPrepareMatrix, OFFSET eglRenderPoly@PrepareMatrix486
		mov	[ebx].rp.pfnRevolveMatrix, OFFSET eglRenderPoly@RevolveMatrix486
		mov	[ebx].rp.pfnCreatePolygonEntry, OFFSET eglRenderPoly@CretaePolygonEntry486
		mov	[ebx].rp.pfnMakeUpPolygon, OFFSET eglRenderPoly@MakeUpPolygon486
		mov	[ebx].rp.pfnApplyAttribute, OFFSET eglRenderPoly@ApplyAttribute486
		mov	[ebx].rp.pfnGetExternalRect, OFFSET eglRenderPoly@GetExternalRect486
		mov	[ebx].rp.pfnShadeReflectLights, OFFSET eglRenderPoly@ShadeVectors486
		mov	[ebx].rp.pfnProjectScreen, OFFSET eglRenderPoly@ProjectScreen486
		;
		mov	[ebx].rp.pfnSetRayTracingParameter, OFFSET eglRenderPoly@SetRayTracingParameter486
		mov	[ebx].rp.pfnAttachRayTracingTarget, OFFSET eglRenderPoly@AttachRayTracingTarget486
		mov	[ebx].rp.pfnCreatePolygonEntryRT, OFFSET eglRenderPoly@CreatePolygonEntryRT_486
		mov	[ebx].rp.pfnPrepareRenderRayTracing, OFFSET eglRenderPoly@PrepareRenderRayTracing_486
		mov	[ebx].rp.pfnRenderRayTracing, OFFSET eglRenderPoly@RenderRayTracing_486
		mov	[ebx].rp.pfnAbortRenderRayTracing, OFFSET eglRenderPoly@AbortRenderRayTracing_486
		mov	[ebx].rp.pfnGetProgressRayTracing, OFFSET eglRenderPoly@GetProgressRayTracing_486
	.ENDIF
	;
	; レンダリングパラメータの初期化
	;
	mov	esi, pScreenPos
	ASSUME	esi:PCE3D_VECTOR
	.IF	esi != NULL
		mov	eax, [esi].x
		mov	ecx, [esi].y
		mov	edx, [esi].z
		mov	[ebx].vPureScreenPos.x, eax
		mov	[ebx].vPureScreenPos.y, ecx
		mov	[ebx].vPureScreenPos.z, edx
		mov	[ebx].vScreenPos.x, eax
		mov	[ebx].vScreenPos.y, ecx
		mov	[ebx].vScreenPos.z, edx
	.ELSE
		fild	[ebx].dib.dstimg.dwImageWidth
		fst	[ebx].vPureScreenPos.z
		fst	[ebx].vScreenPos.z
		fmul	realConstHalf
		fst	[ebx].vPureScreenPos.x
		fstp	[ebx].vScreenPos.x
		fild	[ebx].dib.dstimg.dwImageHeight
		fmul	realConstHalf
		fst	[ebx].vPureScreenPos.y
		fstp	[ebx].vScreenPos.y
	.ENDIF
	fld	[ebx].vScreenPos.x
	fiadd	[ebx].dib.ptDrawOffset.x
	fstp	[ebx].vScreenPos.x
	fld	[ebx].vScreenPos.y
	fiadd	[ebx].dib.ptDrawOffset.y
	fstp	[ebx].vScreenPos.y
	;
	xor	eax, eax
	mov	edx, 3F800000H
	mov	[ebx].envmat.matrix[0][0], edx
	mov	[ebx].envmat.matrix[0][4], eax
	mov	[ebx].envmat.matrix[0][8], eax
	mov	[ebx].envmat.matrix[0][12], eax
	mov	[ebx].envmat.matrix[10H][0], eax
	mov	[ebx].envmat.matrix[10H][4], edx
	mov	[ebx].envmat.matrix[10H][8], eax
	mov	[ebx].envmat.matrix[10H][12], eax
	mov	[ebx].envmat.matrix[20H][0], eax
	mov	[ebx].envmat.matrix[20H][4], eax
	mov	[ebx].envmat.matrix[20H][8], edx
	mov	[ebx].envmat.matrix[20H][12], eax
	mov	[ebx].genvmap.pUpperImage, eax
	mov	[ebx].genvmap.pUnderImage, eax
	mov	[ebx].genvmap.dwFlags, eax
	mov	[ebx].genvmap.dwReserved[0], eax
	mov	[ebx].genvmap.dwReserved[4], eax
	mov	[ebx].genvmap.dwReserved[8], eax
	;
	mov	[ebx].nVectorLightCount, eax
	mov	[ebx].nPointLightCount, eax
	;
	; メモリ開放
	;
	mov	eax, [ebx].pVectorLights[4]
	.IF	eax != NULL
		INVOKE	eslHeapFree , [ebx].dib.hHeap, eax, 0
		mov	[ebx].pVectorLights[0], NULL
		mov	[ebx].pVectorLights[4], NULL
	.ENDIF
	mov	eax, [ebx].pPointLights[4]
	.IF	eax != NULL
		INVOKE	eslHeapFree , [ebx].dib.hHeap, eax, 0
		mov	[ebx].pPointLights[0], NULL
		mov	[ebx].pPointLights[4], NULL
	.ENDIF
	mov	eax, [ebx].pTempZBuffer[4]
	.IF	eax != 0
		INVOKE	eslHeapFree , [ebx].dib.hHeap, eax, 0
		mov	[ebx].pTempZBuffer[4], NULL
	.ENDIF
	mov	eax, [ebx].pLineBuf[4]
	.IF	eax != 0
		INVOKE	eslHeapFree , [ebx].dib.hHeap, eax, 0
		mov	[ebx].pLineBuf[4], NULL
	.ENDIF
	mov	eax, [ebx].pLineVectorBuf[4]
	.IF	eax != 0
		INVOKE	eslHeapFree , [ebx].dib.hHeap, eax, 0
		mov	[ebx].pLineVectorBuf[4], NULL
	.ENDIF
	mov	eax, [ebx].pRenderSyncBuf
	.IF	eax != 0
		INVOKE	eslHeapFree , [ebx].dib.hHeap, eax, 0
		mov	[ebx].pRenderSyncBuf, NULL
	.ENDIF
	mov	eax, [ebx].ppRayShadowingList[4]
	.IF	eax != 0
		INVOKE	eslHeapFree , [ebx].dib.hHeap, eax, 0
		mov	[ebx].ppRayShadowingList[4], NULL
	.ENDIF
	mov	eax, [ebx].ppRayReflectionList[4]
	.IF	eax != 0
		INVOKE	eslHeapFree , [ebx].dib.hHeap, eax, 0
		mov	[ebx].ppRayReflectionList[4], NULL
	.ENDIF
	;
	mov	[ebx].nRayShadowings, 0
	mov	[ebx].nRayShadowingListCount, 0
	mov	[ebx].nRayReflections, 0
	mov	[ebx].nRayReflectionListCount, 0
	mov	[ebx].nGlobalRayReflections, 0
	;
	; 一時ｚバッファ確保
	;
	mov	eax, pDstImage
	mov	eax, (EGL_IMAGE_INFO PTR [eax]).dwImageWidth
	shl	eax, 2
	add	eax, 40H
	INVOKE	eslHeapAllocate , [ebx].dib.hHeap, eax, 0
	mov	[ebx].pTempZBuffer[4], eax
	add	eax, 0FH
	and	eax, NOT 0FH
	mov	[ebx].pTempZBuffer[0], eax
	;
	; レンダリングラインバッファ確保
	;
	mov	eax, pDstImage
	mov	eax, (EGL_IMAGE_INFO PTR [eax]).dwImageWidth
	add	eax, 16
	mov	nLineWidth, eax
	imul	eax, (SIZEOF E3D_TRANS_LINE_BUF) / 2
	INVOKE	eslHeapAllocate , [ebx].dib.hHeap, eax, 0
	mov	[ebx].pLineBuf[4], eax
	add	eax, 0FH
	and	eax, NOT 0FH
	mov	[ebx].pLineBuf[0], eax
	;
	imul	eax, nLineWidth, \
			(SIZEOF E3D_VECTOR4) * 2 \
				+ (SIZEOF E3D_COLOR) + (SIZEOF EGL_PALETTE)
	INVOKE	eslHeapAllocate , [ebx].dib.hHeap, eax, 0
	mov	[ebx].pLineVectorBuf[4], eax
	add	eax, 0FH
	and	eax, NOT 0FH
	mov	[ebx].pLineVectorBuf[0], eax
	;
	imul	edx, nLineWidth, (SIZEOF E3D_VECTOR4)
	add	eax, edx
	mov	[ebx].pLineNormalBuf, eax
	;
	add	eax, edx
	mov	[ebx].pLineColorBuf, eax
	;
	imul	edx, nLineWidth, (SIZEOF E3D_COLOR)
	add	eax, edx
	mov	[ebx].pLineTextureBuf, eax
	;
	; レイトレーシング用同期バッファ確保
	;
	mov	eax, pDstImage
	mov	eax, (EGL_IMAGE_INFO PTR [eax]).dwImageHeight
	add	eax, 16
	shl	eax, 2
	INVOKE	eslHeapAllocate , [ebx].dib.hHeap, eax, 0
	mov	[ebx].pRenderSyncBuf, eax
	;
	mov	[ebx].yNextRenderSyncBuf, 0

	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglRenderPoly@Initialize	ENDP


ALIGN	10H
eglRenderPoly@InitializeToReference	PROC	NEAR32 C USES ebx esi edi,
	hRenderPoly:HEGL_RENDER_POLYGON,
	hRefRenderPoly:HEGL_RENDER_POLYGON,
	pClipRect:PCEGL_RECT, dwFlags:DWORD

	LOCAL	rectClip:EGL_RECT
	LOCAL	errResult:DWORD

	mov	errResult, eslErrSuccess
	mov	ebx, hRefRenderPoly
	mov	esi, pClipRect
	mov	edi, hRenderPoly
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	ASSUME	edi:PTR EGL_RENDER_POLYGON_BUF
	ASSUME	esi:PCEGL_RECT
	;
	;	基本初期化
	; --------------------------------------------------------------------
	;
	; クリッピング領域
	;
	.IF	esi != NULL
		mov	eax, [esi].left
		.IF	(SDWORD PTR eax) < [ebx].dib.rectClip.left
			mov	eax, [ebx].dib.rectClip.left
		.ENDIF
		mov	rectClip.left, eax
		;
		mov	eax, [esi].top
		.IF	(SDWORD PTR eax) < [ebx].dib.rectClip.top
			mov	eax, [ebx].dib.rectClip.top
		.ENDIF
		mov	rectClip.top, eax
		;
		mov	eax, [esi].right
		.IF	(SDWORD PTR eax) > [ebx].dib.rectClip.right
			mov	eax, [ebx].dib.rectClip.right
		.ENDIF
		mov	rectClip.right, eax
		;
		mov	eax, [esi].bottom
		.IF	(SDWORD PTR eax) > [ebx].dib.rectClip.bottom
			mov	eax, [ebx].dib.rectClip.bottom
		.ENDIF
		mov	rectClip.bottom, eax
	.ELSE
		mov	eax, [ebx].dib.rectClip.left
		mov	edx, [ebx].dib.rectClip.top
		mov	rectClip.left, eax
		mov	rectClip.top, edx
		;
		mov	eax, [ebx].dib.rectClip.right
		mov	edx, [ebx].dib.rectClip.bottom
		mov	rectClip.right, eax
		mov	rectClip.bottom, edx
	.ENDIF
	ASSUME	esi:NOTHING
	;
	mov	eax, rectClip.left
	mov	edx, rectClip.top
	.IF	(SDWORD PTR eax) > rectClip.right
		mov	rectClip.right, eax
		mov	errResult, eslErrGeneral
	.ENDIF
	.IF	(SDWORD PTR edx) > rectClip.bottom
		mov	rectClip.right, edx
		mov	errResult, eslErrGeneral
	.ENDIF
	;
	; DrawImage 表示設定反映
	;
	mov	eax, [ebx].dib.ptDrawOffset.x
	mov	edx, [ebx].dib.ptDrawOffset.y
	mov	[edi].dib.ptDrawOffset.x, eax
	mov	[edi].dib.ptDrawOffset.y, edx
	;
	mov	eax, [ebx].dib.dwDrawFlags
	mov	[edi].dib.dwDrawFlags, eax
	;
	; 基本初期化
	;
	INVOKE	eglRenderPoly@Initialize ,
			edi, [ebx].dib.pDstImage,
			ADDR rectClip, [ebx].dib.pZBuffer,
			ADDR [ebx].vPureScreenPos
	.IF	(eax != eslErrSuccess) && (errResult == eslErrSuccess)
		mov	errResult, eax
	.ENDIF

	;
	;	レンダリング各種パラメータ設定
	; --------------------------------------------------------------------
	;
	; 基本パラメータ
	;
	mov	eax, [ebx].dwFunctionFlags
	mov	ecx, [ebx].dwMaskShadingFlags
	mov	edx, [ebx].dwAddShadingFlags
	mov	[edi].dwFunctionFlags, eax
	mov	[edi].dwMaskShadingFlags, ecx
	mov	[edi].dwAddShadingFlags, edx
	;
	mov	eax, [ebx].rZMinClip
	mov	edx, [ebx].rZMaxClip
	mov	[edi].rZMinClip, eax
	mov	[edi].rZMaxClip, edx
	;
	; 光源設定参照
	;
	mov	eax, DWORD PTR [ebx].rgbAmbientLight[0]
	mov	edx, DWORD PTR [ebx].rgbAmbientLight[4]
	mov	DWORD PTR [edi].rgbAmbientLight[0], eax
	mov	DWORD PTR [edi].rgbAmbientLight[4], edx
	;
	mov	eax, [ebx].nVectorLightCount
	mov	edx, [ebx].pVectorLights
	mov	[edi].nVectorLightCount, eax
	mov	[edi].pVectorLights, edx
	;
	mov	eax, [ebx].nPointLightCount
	mov	edx, [ebx].pPointLights
	mov	[edi].nPointLightCount, eax
	mov	[edi].pPointLights, edx
	;
	mov	eax, DWORD PTR [ebx].rgbFogColor[0]
	mov	edx, DWORD PTR [ebx].rgbFogColor[4]
	mov	DWORD PTR [edi].rgbFogColor[0], eax
	mov	DWORD PTR [edi].rgbFogColor[4], edx
	;
	mov	eax, [ebx].rFogDeepness
	mov	edx, [ebx].rFogBiasZ
	mov	[edi].rFogDeepness, eax
	mov	[edi].rFogBiasZ, edx
	;
	; 環境マッピング設定
	;
	xor	esi, esi
	FOR	@INDEX, <0, 10H, 20H>
		mov	eax, [ebx].envmat.matrix[@INDEX][0]
		mov	edx, [ebx].envmat.matrix[@INDEX][4]
		mov	ecx, [ebx].envmat.matrix[@INDEX][8]
		mov	[edi].envmat.matrix[@INDEX][0], eax
		mov	[edi].envmat.matrix[@INDEX][4], edx
		mov	[edi].envmat.matrix[@INDEX][8], ecx
		mov	[edi].envmat.matrix[@INDEX][12], esi
	ENDM
	;
	FOR	@MEMBER, <pUpperImage, pUnderImage, dwFlags>
		mov	eax, [ebx].genvmap.@MEMBER
		mov	[edi].genvmap.@MEMBER, eax
	ENDM
	;
	; レイトレーシング設定参照
	;
	.IF	ERI_EnabledProcessorType & ERI_USE_XMM_P3
		INVOKE	eglRenderPoly@SetRayTracingParameterSSE,
					edi, ADDR [ebx].rrtpRayParam
		;
		mov	eax, [ebx].pRayShadowing
		mov	edx, [ebx].nRayShadowings
		mov	[edi].pRayShadowing, eax
		mov	[edi].nRayShadowings, edx
		;
		add	edx, 03H
		and	edx, NOT 03H
		INVOKE	eslHeapAllocate , [edi].dib.hHeap, edx, 0
		mov	esi, [ebx].ppRayShadowingList
		mov	ecx, [ebx].nRayShadowingListCount
		mov	[edi].ppRayShadowingList[0], eax
		mov	[edi].ppRayShadowingList[4], eax
		mov	edx, eax
		mov	[ebx].nRayShadowingListCount, ecx
		test	ecx, ecx
		.WHILE	!ZERO?
			mov	eax, [esi]
			add	esi, 4
			mov	[edx], eax
			add	edx, 4
			dec	ecx
		.ENDW
		;
		mov	eax, [ebx].pRayReflection
		mov	edx, [ebx].nRayReflections
		mov	[edi].pRayReflection, eax
		mov	[edi].nRayReflections, edx
		;
		add	edx, 03H
		and	edx, NOT 03H
		INVOKE	eslHeapAllocate , [edi].dib.hHeap, edx, 0
		mov	esi, [ebx].ppRayReflectionList
		mov	ecx, [ebx].nRayReflectionListCount
		mov	[edi].ppRayReflectionList[0], eax
		mov	[edi].ppRayReflectionList[4], eax
		mov	edx, eax
		mov	[ebx].nRayReflectionListCount, ecx
		test	ecx, ecx
		.WHILE	!ZERO?
			mov	eax, [esi]
			add	esi, 4
			mov	[edx], eax
			add	edx, 4
			dec	ecx
		.ENDW
		;
		mov	eax, [ebx].pGlobalRayReflection
		mov	edx, [ebx].nGlobalRayReflections
		mov	[edi].pGlobalRayReflection, eax
		mov	[edi].nGlobalRayReflections, edx
	.ENDIF

	ASSUME	ebx:NOTHING
	ASSUME	edi:NOTHING
	mov	eax, errResult
	ret

eglRenderPoly@InitializeToReference	ENDP

;
;	投影スクリーン取得
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@GetScreenPos	PROC	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON

	mov	eax, hRenderPoly
	lea	eax, (EGL_RENDER_POLYGON_BUF PTR [eax]).vPureScreenPos
	ret

eglRenderPoly@GetScreenPos	ENDP

;
;	画像描画バッファ取得
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@GetDrawImage	PROC	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON

	mov	eax, hRenderPoly
	lea	eax, (EGL_RENDER_POLYGON_BUF PTR [eax]).dib
	ret

eglRenderPoly@GetDrawImage	ENDP

;
;	ファンクションフラグを取得
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@GetFunctionFlags	PROC	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON

	mov	eax, hRenderPoly
	mov	eax, (EGL_RENDER_POLYGON_BUF PTR [eax]).dwFunctionFlags
	ret

eglRenderPoly@GetFunctionFlags	ENDP

;
;	ファンクションフラグを設定
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@SetFunctionFlags	PROC	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON, dwFlags:DWORD

	mov	edx, hRenderPoly
	ASSUME	edx:PTR EGL_RENDER_POLYGON_BUF
	mov	eax, dwFlags
	.IF	!(ERI_EnabledProcessorType & ERI_USE_SSE2)
		and	eax, NOT E3D_FLAG_ENABLE_SSE2
	.ENDIF
	mov	[edx].dwFunctionFlags, eax
	;
	and	eax, E3D_FLAG_RAY_TRACING
	mov	ecx, -1
	.IF	!([edx].dwFunctionFlags & E3D_FLAG_TEXTURE_SMOOTHING)
		and	ecx, NOT E3DSAF_TEXTURE_SMOOTH
	.ENDIF
	.IF	!([edx].rrtpRayParam.dwFlags & E3D_RAYTRACE_SHADOWING)
		and	ecx, NOT E3DSAF_RAY_SHADOWING
		and	eax, NOT E3DSAF_RAY_SHADOWING
	.ENDIF
	.IF	!([edx].rrtpRayParam.dwFlags & E3D_RAYTRACE_REFLECTION)
		and	ecx, NOT E3DSAF_RAY_REFLECTING
		and	eax, NOT E3DSAF_RAY_REFLECTING
	.ENDIF
	.IF	!([edx].rrtpRayParam.dwFlags & E3D_RAYTRACE_REFRACTION)
		and	ecx, NOT E3DSAF_RAY_REFRACTING
		and	eax, NOT E3DSAF_RAY_REFRACTING
	.ENDIF
	mov	[edx].dwMaskShadingFlags, ecx
	mov	[edx].dwAddShadingFlags, eax
	ASSUME	edx:NOTHING
	ret

eglRenderPoly@SetFunctionFlags	ENDP

;
;	行列変換準備 486 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@PrepareMatrix486	PROC	NEAR32 C USES ebx esi edi,
	hRenderPoly:HEGL_RENDER_POLYGON,
	matrix:PTR E3D_REV_MATRIX,
	pOrigin:PCE3D_VECTOR, pEnlarge:PCE3D_VECTOR

	mov	ebx, hRenderPoly
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF

	mov	esi, pOrigin
	ASSUME	esi:PCE3D_VECTOR
	.IF	esi != NULL
		mov	eax, [esi].x
		mov	ecx, [esi].y
		mov	edx, [esi].z
		mov	[ebx].vRevOrg.x, eax
		mov	[ebx].vRevOrg.y, ecx
		mov	[ebx].vRevOrg.z, edx
	.ELSE
		xor	eax, eax
		mov	[ebx].vRevOrg.x, eax
		mov	[ebx].vRevOrg.y, eax
		mov	[ebx].vRevOrg.z, eax
	.ENDIF

	mov	esi, matrix
	ASSUME	esi:PTR E3D_REV_MATRIX
	@INDEX = 0
	REPEAT	3
		mov	eax, [esi].matrix[@INDEX][0]
		mov	ecx, [esi].matrix[@INDEX][4]
		mov	edx, [esi].matrix[@INDEX][8]
		mov	[ebx].revmat.matrix[@INDEX][0], eax
		mov	[ebx].revmat.matrix[@INDEX][4], ecx
		mov	[ebx].revmat.matrix[@INDEX][8], edx
		@INDEX = @INDEX + 10H
	ENDM

	mov	esi, pEnlarge
	ASSUME	esi:PCE3D_VECTOR
	.IF	esi != NULL
		@INDEX = 0
		FOR	@MEMBER, <x, y, z>
			fld	[ebx].revmat.matrix[0][@INDEX]
			fld	[ebx].revmat.matrix[10H][@INDEX]
			fld	[ebx].revmat.matrix[20H][@INDEX]
			fld	[esi].@MEMBER
			fmul	st(1), st
			fmul	st(2), st
			fmulp	st(3), st
			fstp	[ebx].revmat.matrix[20H][@INDEX]
			fstp	[ebx].revmat.matrix[10H][@INDEX]
			fstp	[ebx].revmat.matrix[0][@INDEX]
			@INDEX = @INDEX + 4
		ENDM
	.ENDIF

	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglRenderPoly@PrepareMatrix486	ENDP

;
;	行列変換 486 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RevolveMatrix486	PROC	NEAR32 C USES ebx esi edi,
	hRenderPoly:HEGL_RENDER_POLYGON,
	pDst:PE3D_VECTOR4, pSrc:PCE3D_VECTOR4, nVectorCount:DWORD

	mov	ecx, nVectorCount
	mov	ebx, hRenderPoly
	mov	esi, pSrc
	mov	edi, pDst
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	ASSUME	esi:PCE3D_VECTOR4
	ASSUME	edi:PE3D_VECTOR4

	test	ecx, ecx
	.WHILE	!ZERO?
		fld	[esi].z		; st(0) = x, st(1) = y, st(2) = z
		fld	[esi].y
		fld	[esi].x
		add	esi, (SIZEOF E3D_VECTOR4)
		;
		fld	[ebx].revmat.matrix[0][0]	; x 座標計算
		fld	[ebx].revmat.matrix[0][4]
		fld	[ebx].revmat.matrix[0][8]
		fxch	st(2)
		fmul	st, st(3)
		fxch	st(1)
		fmul	st, st(4)
		fxch	st(2)
		fmul	st, st(5)
		fxch	st(2)
		faddp	st(1), st
		faddp	st(1), st
		fadd	[ebx].vRevOrg.x
		fstp	[edi].x
		;
		fld	[ebx].revmat.matrix[10H][0]	; y 座標計算
		fld	[ebx].revmat.matrix[10H][4]
		fld	[ebx].revmat.matrix[10H][8]
		fxch	st(2)
		fmul	st, st(3)
		fxch	st(1)
		fmul	st, st(4)
		fxch	st(2)
		fmul	st, st(5)
		fxch	st(2)
		faddp	st(1), st
		faddp	st(1), st
		fadd	[ebx].vRevOrg.y
		fstp	[edi].y
		;
		fld	[ebx].revmat.matrix[20H][8]	; z 座標計算
		fld	[ebx].revmat.matrix[20H][4]
		fld	[ebx].revmat.matrix[20H][0]
		fmulp	st(3), st
		fmulp	st(3), st
		fmulp	st(3), st
		faddp	st(1), st
		faddp	st(1), st
		fadd	[ebx].vRevOrg.z
		fstp	[edi].z
		add	edi, (SIZEOF E3D_VECTOR4)
		;
		dec	ecx
	.ENDW

	ASSUME	ebx:NOTHING
	ASSUME	esi:NOTHING
	ASSUME	edi:NOTHING

	xor	eax, eax
	ret

eglRenderPoly@RevolveMatrix486	ENDP

;
;	行列変換準備 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@PrepareMatrixSSE	PROC	NEAR32 C USES ebx esi edi,
	hRenderPoly:HEGL_RENDER_POLYGON,
	matrix:PTR E3D_REV_MATRIX,
	pOrigin:PCE3D_VECTOR, pEnlarge:PCE3D_VECTOR

	mov	ebx, hRenderPoly
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF

	;
	;	変換パラメータを XMM レジスタに読み込む
	; --------------------------------------------------------------------
	;
	; 行列をスウィズリングして、xmm0, xmm1, xmm2 に読み込む
	;
	mov	esi, matrix
	ASSUME	esi:PTR QWORD
	xorps	xmm1, xmm1
	xorps	xmm3, xmm3
	movlps	xmm0, [esi][0][0]		; xmm0 = y2 x2 y1 x1
	movlps	xmm1, [esi][20H][0]		; xmm1 = 0. 0. y3 x3
	movlps	xmm2, [esi][0][8]		; xmm2 = -- z2 -- z1
	movlps	xmm3, [esi][20H][8]		; xmm3 = 0. 0. -- z3
	movhps	xmm0, [esi][10H][0]
	movhps	xmm2, [esi][10H][8]
	movaps	xmm4, xmm0
	shufps	xmm0, xmm1, 10001000B		; xmm0 = 0. x3 x2 x1
	shufps	xmm4, xmm1, 11011101B		; xmm4 = 0. y3 y2 y1
	shufps	xmm2, xmm3, 10001000B		; xmm2 = 0. z3 z2 z1
	movaps	xmm1, xmm4
	;
	; ベクトル拡大率を行列に適用
	;
	mov	esi, pEnlarge
	ASSUME	esi:PCE3D_VECTOR
	.IF	esi != NULL
		xorps	xmm4, xmm4
		movlps	xmm3, QWORD PTR [esi]
		movss	xmm4, [esi].z
		movlhps	xmm3, xmm4
		mulps	xmm0, xmm3
		mulps	xmm1, xmm3
		mulps	xmm2, xmm3
	.ENDIF
	;
	; ベクトル相対座標を xmm3 に読み込む
	;
	mov	esi, pOrigin
	.IF	esi != NULL
		xorps	xmm4, xmm4
		movlps	xmm3, QWORD PTR [esi]
		movss	xmm4, [esi].z
		movlhps	xmm3, xmm4
	.ELSE
		xorps	xmm3, xmm3
	.ENDIF
	ASSUME	esi:NOTHING
	;
	;	変換パラメータを保存する
	; --------------------------------------------------------------------
	movaps	[ebx].revmat.matrix[0], xmm0
	movaps	[ebx].revmat.matrix[10H], xmm1
	movaps	[ebx].revmat.matrix[20H], xmm2
	movaps	[ebx].vRevOrg, xmm3

	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglRenderPoly@PrepareMatrixSSE	ENDP

;
;	行列変換 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RevolveMatrixSSE	PROC	NEAR32 C USES ebx esi edi,
	hRenderPoly:HEGL_RENDER_POLYGON,
	pDst:PE3D_VECTOR4, pSrc:PCE3D_VECTOR4, nVectorCount:DWORD

	;
	;	変換パラメータを XMM レジスタに読み込む
	; --------------------------------------------------------------------
	mov	ebx, hRenderPoly
	mov	ecx, nVectorCount
	mov	esi, pSrc
	mov	edi, pDst
	ASSUME	esi:PCE3D_VECTOR4
	ASSUME	edi:PE3D_VECTOR4
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	movaps	xmm0, [ebx].revmat.matrix[0]
	movaps	xmm1, [ebx].revmat.matrix[10H]
	movaps	xmm2, [ebx].revmat.matrix[20H]
	movaps	xmm3, [ebx].vRevOrg

	;
	;	変換ループ
	; --------------------------------------------------------------------

	.IF	edi & 0FH
		sub	ecx, 2
		.WHILE	!SIGN?
			prefetchnta	[esi + 20H*8]
			movups	xmm4, [esi]
				movups	xmm7, [esi + 10H]
			add	esi, 20H
			movaps	xmm5, xmm4
			movaps	xmm6, xmm4
			shufps	xmm4, xmm4, 11000000B
			shufps	xmm5, xmm5, 11010101B
			shufps	xmm6, xmm6, 11101010B
			mulps	xmm4, xmm0
			mulps	xmm5, xmm1
			mulps	xmm6, xmm2
			prefetchnta	[edi + 20H*8]
			addps	xmm4, xmm5
				movaps	xmm5, xmm7
				shufps	xmm7, xmm7, 11101010B
			addps	xmm6, xmm3
				mulps	xmm7, xmm2
			addps	xmm4, xmm6
				movaps	xmm6, xmm5
				shufps	xmm5, xmm5, 11000000B
				shufps	xmm6, xmm6, 11010101B
				mulps	xmm5, xmm0
				mulps	xmm6, xmm1
				addps	xmm7, xmm3
				addps	xmm5, xmm6
			movups	[edi], xmm4
				addps	xmm7, xmm5
				movups	[edi + 10H], xmm7
			add	edi, 20H
			sub	ecx, 2
		.ENDW
		add	ecx, 2
	.ELSE
		sub	ecx, 2
		.WHILE	!SIGN?
			prefetchnta	[esi + 20H*8]
			movups	xmm4, [esi]
				movups	xmm7, [esi + 10H]
			add	esi, 20H
			movaps	xmm5, xmm4
			movaps	xmm6, xmm4
			shufps	xmm4, xmm4, 11000000B
			shufps	xmm5, xmm5, 11010101B
			shufps	xmm6, xmm6, 11101010B
			mulps	xmm4, xmm0
			mulps	xmm5, xmm1
			mulps	xmm6, xmm2
			addps	xmm4, xmm5
				movaps	xmm5, xmm7
				shufps	xmm7, xmm7, 11101010B
			addps	xmm6, xmm3
				mulps	xmm7, xmm2
			addps	xmm4, xmm6
				movaps	xmm6, xmm5
				shufps	xmm5, xmm5, 11000000B
				shufps	xmm6, xmm6, 11010101B
				mulps	xmm5, xmm0
				mulps	xmm6, xmm1
				addps	xmm7, xmm3
				addps	xmm5, xmm6
			movntps	[edi], xmm4
				addps	xmm7, xmm5
				movntps	[edi + 10H], xmm7
			add	edi, 20H
			sub	ecx, 2
		.ENDW
		sfence
		add	ecx, 2
	.ENDIF

	.IF	!ZERO?
		movups	xmm4, [esi]
		movaps	xmm5, xmm4
		movaps	xmm6, xmm4
		shufps	xmm4, xmm4, 11000000B
		shufps	xmm5, xmm5, 11010101B
		shufps	xmm6, xmm6, 11101010B
		;
		mulps	xmm4, xmm0
		mulps	xmm5, xmm1
		mulps	xmm6, xmm2
		;
		addps	xmm4, xmm5
		addps	xmm6, xmm3
		addps	xmm4, xmm6
		;
		movups	[edi], xmm4
	.ENDIF

	ASSUME	ebx:NOTHING
	ASSUME	esi:NOTHING
	ASSUME	edi:NOTHING
	xor	eax, eax
	ret

eglRenderPoly@RevolveMatrixSSE	ENDP

;
;	z 座標クリッピング領域設定
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@SetZClipRange	PROC	NEAR32 C USES ebx esi edi,
	hRenderPoly:HEGL_RENDER_POLYGON, rMin:REAL4, rMax:REAL4

	mov	ebx, hRenderPoly
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF

	mov	eax, rMin
	mov	edx, rMax
	mov	[ebx].rZMinClip, eax
	mov	[ebx].rZMaxClip, edx

	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglRenderPoly@SetZClipRange	ENDP

;
;	環境マッピング設定
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@SetEnvironmentMapping	PROC	NEAR32 C USES ebx esi edi,
	hRenderPoly:HEGL_RENDER_POLYGON,
	matrix:PTR E3D_REV_MATRIX,
	envmap:PTR E3D_ENVIRONMENT_MAPPING

	mov	ebx, hRenderPoly
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	;
	;	環境マッピング画像設定
	; --------------------------------------------------------------------
	mov	esi, envmap
	.IF	esi != NULL
		ASSUME	esi:PTR E3D_ENVIRONMENT_MAPPING
		mov	eax, [esi].pUpperImage
		mov	edx, [esi].pUnderImage
		mov	[ebx].genvmap.pUpperImage, eax
		mov	[ebx].genvmap.pUnderImage, edx
		mov	eax, [esi].dwFlags
		mov	edx, [esi].dwReserved
		mov	[ebx].genvmap.dwFlags, eax
		mov	[ebx].genvmap.dwReserved, edx
		mov	eax, [esi].dwReserved[4]
		mov	edx, [esi].dwReserved[8]
		mov	[ebx].genvmap.dwReserved[4], eax
		mov	[ebx].genvmap.dwReserved[8], edx
	.ENDIF
	;
	;	環境マッピング回転行列設定
	; --------------------------------------------------------------------
	mov	esi, matrix
	.IF	esi != NULL
		ASSUME	esi:PTR E3D_REV_MATRIX
		;         | a1 a2 a3 |
		; D = 1 / | b1 b2 b3 |
		;         | c1 c2 c3 |
		fld1
		FOR	@BASE, <0, 1, 2>
			@LINE = 0
			FOR	@INDEX, <0, 1, 2>
				@INDEX_SUB = (@BASE + @INDEX) MOD 3
				fld	[esi].matrix[@INDEX*10H][@INDEX_SUB*4]
				@LINE = @LINE + 1
			ENDM
			fxch	st(2)
			fmulp	st(1), st
			fmulp	st(1), st
			;
			@LINE = 0
			FOR	@INDEX, <0, 2, 1>
				@INDEX_SUB = (@BASE + @INDEX) MOD 3
				fld	[esi].matrix[@LINE*10H][@INDEX_SUB*4]
				@LINE = @LINE + 1
			ENDM
			fxch	st(2)
			fmulp	st(1), st
			fmulp	st(1), st
			fsubp	st(1), st
		ENDM
		faddp	st(1), st
		faddp	st(1), st
		fdivp	st(1), st
		;
		; A' = [B, C],  B' = [C, A],  C' = [A, B]
		;
		xor	eax, eax
		FOR	@BASE, <0, 1, 2>
			@LINE1 = ((@BASE + 1) MOD 3) * 10H
			@LINE2 = ((@BASE + 2) MOD 3) * 10H
			FOR	@INDEX, <0, 1, 2>
				@INDEX_SUB1 = ((@INDEX + 1) MOD 3) * 4
				@INDEX_SUB2 = ((@INDEX + 2) MOD 3) * 4
				fld	[esi].matrix[@LINE1][@INDEX_SUB1]
				fmul	[esi].matrix[@LINE2][@INDEX_SUB2]
				fld	[esi].matrix[@LINE1][@INDEX_SUB2]
				fmul	[esi].matrix[@LINE2][@INDEX_SUB1]
				fsubp	st(1), st
				fmul	st, st(1)
				fstp	[ebx].envmat.matrix[@INDEX*10H][@BASE*4]
			ENDM
			mov	[ebx].envmat.matrix[@BASE*10H][12], eax
		ENDM
		fstp	st(0)
	.ENDIF
	;
	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglRenderPoly@SetEnvironmentMapping	ENDP

;
;	光源設定
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@PrepareLight	PROC	NEAR32 C USES ebx esi edi,
	hRenderPoly:HEGL_RENDER_POLYGON, hStackHeap:HSTACKHEAP,
	nLightCount:DWORD, pLightEntries:PCE3D_LIGHT_ENTRY

	LOCAL	nBrightness:DWORD

	mov	ebx, hRenderPoly
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF

	;
	; パラメータ初期化
	;
	xor	eax, eax
	mov	[ebx].rgbAmbientLight.Blue, ax
	mov	[ebx].rgbAmbientLight.Green, ax
	mov	[ebx].rgbAmbientLight.Red, ax
	mov	[ebx].rgbAmbientLight.Alpha, ax
	mov	[ebx].nVectorLightCount, eax
	mov	[ebx].nPointLightCount, eax
	mov	[ebx].rFogDeepness, eax
	mov	[ebx].rFogBiasZ, eax
	mov	[ebx].rgbShadowLight.Blue, 100H
	mov	[ebx].rgbShadowLight.Green, 100H
	mov	[ebx].rgbShadowLight.Red, 100H
	mov	[ebx].rgbShadowLight.Alpha, ax
	;
	; 光源の数を取得
	;
	mov	edx, nLightCount
	mov	esi, pLightEntries
	ASSUME	esi:PCE3D_LIGHT_ENTRY
	test	edx, edx
	.WHILE	!ZERO?
		mov	eax, [esi].dwLightType
		and	eax, E3D_LIGHT_TYPE_MASK
		.IF	eax == E3D_POINT_LIGHT
			inc	[ebx].nPointLightCount
		.ELSEIF	eax == E3D_VECTOR_LIGHT
			inc	[ebx].nVectorLightCount
		.ELSEIF	eax == E3D_AMBIENT_LIGHT
			movzx	eax, [esi].rgbColor.rgb.Blue
			add	[ebx].rgbAmbientLight.Blue, ax
			movzx	eax, [esi].rgbColor.rgb.Green
			add	[ebx].rgbAmbientLight.Green, ax
			movzx	eax, [esi].rgbColor.rgb.Red
			add	[ebx].rgbAmbientLight.Red, ax
		.ELSEIF	eax == E3D_SHADOW_LIGHT
			FOR	@MEMBER, <Blue, Green, Red>
				movzx	eax, [esi].rgbColor.rgb.@MEMBER
				movzx	ecx, [ebx].rgbShadowLight.@MEMBER
				inc	eax
				imul	eax, ecx
				shr	eax, 8
				add	[ebx].rgbAmbientLight.@MEMBER, ax
			ENDM
		.ELSEIF	eax == E3D_FOG_LIGHT
			fld	realConst256
			fdiv	[esi].rFogDeepness
			mov	eax, [esi].rFogDistance
			mov	[ebx].rFogBiasZ, eax
			fstp	[ebx].rFogDeepness
			FOR	@DUMMY, <Blue, Green, Red>
				movzx	eax, [esi].rgbColor.rgb.@DUMMY
				mov	[ebx].rgbFogColor.@DUMMY, ax
			ENDM
		.ENDIF
		add	esi, (SIZEOF E3D_LIGHT_ENTRY)
		dec	edx
	.ENDW
	;
	; 無限遠光源をセットアップ
	;
	mov	eax, [ebx].pVectorLights[4]
	.IF	eax != NULL
		INVOKE	eslHeapFree , [ebx].dib.hHeap, eax, 0
	.ENDIF
	mov	eax, [ebx].nVectorLightCount
	imul	eax, (SIZEOF E3D_VECTOR_LIGHT_ENTRY)
	add	eax, 20H
	INVOKE	eslHeapAllocate , [ebx].dib.hHeap, eax, 0
	mov	[ebx].pVectorLights[0], eax
	mov	[ebx].pVectorLights[4], eax
	mov	edi, eax
	ASSUME	edi:PTR E3D_VECTOR_LIGHT_ENTRY
	mov	edx, nLightCount
	mov	esi, pLightEntries
	test	edx, edx
	.WHILE	!ZERO?
		mov	eax, [esi].dwLightType
		and	eax, E3D_LIGHT_TYPE_MASK
		.IF	eax == E3D_VECTOR_LIGHT
			fld	[esi].vecLight.z
			fld	[esi].vecLight.y
			fld	[esi].vecLight.x
			fld	st(2)
			fmul	st(0), st
			fld	st(2)
			fmul	st(0), st
			fld	st(2)
			fmul	st(0), st
			faddp	st(1), st
			faddp	st(1), st
			fsqrt
			fld	realConst256
			fdivrp	st(1), st
			fmul	st(1), st
			fmul	st(2), st
			fmulp	st(3), st
			fst	[edi].vcLight.x
			fistp	[edi].vLight.x
			fst	[edi].vcLight.y
			fistp	[edi].vLight.y
			fst	[edi].vcLight.z
			fistp	[edi].vLight.z
			mov	[edi].vcLight.d, 0
			mov	[edi].ShadowMap.pShadowMap, NULL
			;
			fld	realConst256
			fmul	[esi].rBrightness
			fistp	nBrightness
			mov	ecx, nBrightness
			;
			FOR	@DUMMY, <Blue, Green, Red>
				movzx	eax, [esi].rgbColor.rgb.@DUMMY
				imul	eax, ecx
				sar	eax, 8
				.IF	(DWORD PTR eax) >= 7FFFH
					sar	eax, 31
					not	eax
					and	eax, 7FFFH
				.ENDIF
				mov	[edi].rgbColor.@DUMMY, ax
			ENDM
			;
			.IF	[esi].dwLightType & E3D_LIGHT_SHADOW_MAP
				mov	ecx, [esi].ShadowMap.pMapInfo
				ASSUME	ecx:PTR E3D_SHADOW_MAP_INFO
				;
				mov	eax, [ecx].pShadowMap
				mov	[edi].ShadowMap.pShadowMap, eax
				mov	eax, [ecx].rFixErrorGap
				mov	[edi].ShadowMap.rFixErrorGap, eax
				mov	eax, [ecx].rVarErrorGap
				mov	[edi].ShadowMap.rVarErrorGap, eax
				;
				FOR	@DUMMY, <vLightPos, vLightRay, vOriginPos, vAxisX, vAxisY>
					FOR	@MEMBER, <x, y, z>
						mov	eax, [ecx].@DUMMY.@MEMBER
						mov	[edi].ShadowMap.@DUMMY.@MEMBER, eax
					ENDM
				ENDM
				;
				fld	[edi].ShadowMap.vLightRay.z
				fld	[edi].ShadowMap.vLightRay.y
				fld	[edi].ShadowMap.vLightRay.x
				fld	st(2)
				fmul	st(0), st
				fld	st(2)
				fmul	st(0), st
				fld	st(2)
				fmul	st(0), st
				faddp	st(1), st
				faddp	st(1), st
				fsqrt
				fld1
				fdivrp	st(1), st
				fmul	st(1), st
				fmul	st(2), st
				fmulp	st(3), st
				;
				fld	[edi].ShadowMap.vOriginPos.z
				fsub	[edi].ShadowMap.vLightPos.z
				fmul	st, st(3)
				fld	[edi].ShadowMap.vOriginPos.y
				fsub	[edi].ShadowMap.vLightPos.y
				fmul	st, st(3)
				fld	[edi].ShadowMap.vOriginPos.x
				fsub	[edi].ShadowMap.vLightPos.x
				fmul	st, st(3)
				faddp	st(2), st
				faddp	st(1), st
				fstp	[edi].ShadowMap.rMapDistance
				;
				fstp	[edi].ShadowMap.vLightRay.x
				fstp	[edi].ShadowMap.vLightRay.y
				fstp	[edi].ShadowMap.vLightRay.z
				;
				ASSUME	ecx:NOTHING
			.ENDIF
			;
			add	edi, (SIZEOF E3D_VECTOR_LIGHT_ENTRY)
		.ENDIF
		add	esi, (SIZEOF E3D_LIGHT_ENTRY)
		dec	edx
	.ENDW
	;
	; 点光源をセットアップ
	;
	mov	eax, [ebx].pPointLights[4]
	.IF	eax != NULL
		INVOKE	eslHeapFree , [ebx].dib.hHeap, eax, 0
	.ENDIF
	mov	eax, [ebx].nPointLightCount
	imul	eax, (SIZEOF E3D_POINT_LIGHT_ENTRY)
	add	eax, 20H
	INVOKE	eslHeapAllocate , [ebx].dib.hHeap, eax, 0
	mov	edi, eax
	mov	[ebx].pPointLights[0], eax
	mov	[ebx].pPointLights[4], eax
	;
	ASSUME	edi:PTR E3D_POINT_LIGHT_ENTRY
	mov	edx, nLightCount
	mov	esi, pLightEntries
	test	edx, edx
	.WHILE	!ZERO?
		.IF	[esi].dwLightType == E3D_POINT_LIGHT
			mov	eax, [esi].vecLight.x
			mov	[edi].vLight.x, eax
			mov	eax, [esi].vecLight.y
			mov	[edi].vLight.y, eax
			mov	eax, [esi].vecLight.z
			mov	[edi].vLight.z, eax
			mov	eax, [esi].rBrightness
			mov	[edi].rBrightness, eax
			;
			FOR	@DUMMY, <Blue, Green, Red>
				movzx	eax, [esi].rgbColor.rgb.@DUMMY
				mov	[edi].rgbColor.@DUMMY, ax
			ENDM
			;
			add	edi, (SIZEOF E3D_POINT_LIGHT_ENTRY)
		.ENDIF
		add	esi, (SIZEOF E3D_LIGHT_ENTRY)
		dec	edx
	.ENDW

	ASSUME	edi:NOTHING
	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglRenderPoly@PrepareLight	ENDP


CodeSeg	ENDS

	END
