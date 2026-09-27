
; ----------------------------------------------------------------------------
;                          Entis Graphic Library
; ----------------------------------------------------------------------------
;    Copyright (c) 2008-2009 Leshade Entis, Entis-soft. Al rights reserved.
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
xmmErrorScale	REAL4	4 DUP( 0.0009765625 )	; 1 / 1024
xmmPackedDiv256	REAL4	4 DUP( 0.00390625 )

ConstSeg	ENDS


; ----------------------------------------------------------------------------
;	コードセグメント
; ----------------------------------------------------------------------------

CodeSeg	SEGMENT	PARA READONLY FLAT 'CODE'

;
;	レイトレーシング準備（ダミー）
; ----------------------------------------------------------------------------
eglRenderPoly@PrepareRenderRayTracing_486	PROC	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON, pRenderRect:PCEGL_RECT

	mov	eax, eslErrGeneral
	ret

eglRenderPoly@PrepareRenderRayTracing_486	ENDP

;
;	レイトレーシング開始（ダミー）
; ----------------------------------------------------------------------------
eglRenderPoly@RenderRayTracing_486	PROC	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON, hSyncRender:HEGL_RENDER_POLYGON

	mov	eax, eslErrGeneral
	ret

eglRenderPoly@RenderRayTracing_486	ENDP

;
;	レイトレーシング中止（ダミー）
; ----------------------------------------------------------------------------
eglRenderPoly@AbortRenderRayTracing_486	PROC	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON

	mov	eax, eslErrGeneral
	ret

eglRenderPoly@AbortRenderRayTracing_486	ENDP

;
;	レイトレーシング進行状況取得（ダミー）
; ----------------------------------------------------------------------------
eglRenderPoly@GetProgressRayTracing_486	PROC	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON, pdwPixelCount:PTR DWORD

	mov	eax, eslErrGeneral
	ret

eglRenderPoly@GetProgressRayTracing_486	ENDP

;
;	レイトレーシング準備 SSE コード
; ----------------------------------------------------------------------------
eglRenderPoly@PrepareRenderRayTracing_SSE	PROC	NEAR32 C USES ebx esi edi,
	hRenderPoly:HEGL_RENDER_POLYGON, pRenderRect:PCEGL_RECT

	LOCAL	dwSaveESP:DWORD

	mov	ebx, hRenderPoly
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF

	;
	; レンダリング領域設定
	;
	mov	esi, pRenderRect
	ASSUME	esi:PCEGL_RECT
	.IF	esi != NULL
		mov	eax, [ebx].dib.rectClip.left
		mov	ecx, [ebx].dib.rectClip.top
		mov	edx, [esi].left
		mov	edi, [esi].top
		cmp	eax, edx
		cmovl	eax, edx
		cmp	ecx, edi
		cmovl	ecx, edi
		mov	[ebx].rctRenderRayTracing.left, eax
		mov	[ebx].rctRenderRayTracing.top, ecx
		;
		mov	eax, [ebx].dib.rectClip.right
		mov	ecx, [ebx].dib.rectClip.bottom
		mov	edx, [esi].right
		mov	edi, [esi].bottom
		cmp	eax, edx
		cmovg	eax, edx
		cmp	ecx, edi
		cmovg	ecx, edi
		mov	[ebx].rctRenderRayTracing.right, eax
		mov	[ebx].rctRenderRayTracing.bottom, ecx
	.ELSE
		mov	eax, [ebx].dib.rectClip.left
		mov	ecx, [ebx].dib.rectClip.top
		mov	[ebx].rctRenderRayTracing.left, eax
		mov	[ebx].rctRenderRayTracing.top, ecx
		;
		mov	eax, [ebx].dib.rectClip.right
		mov	ecx, [ebx].dib.rectClip.bottom
		mov	[ebx].rctRenderRayTracing.right, eax
		mov	[ebx].rctRenderRayTracing.bottom, ecx
	.ENDIF
	ASSUME	esi:NOTHING

	;
	; レンダリング同期用フラグ初期化
	;
	sub	ecx, [ebx].rctRenderRayTracing.top
	.IF	!SIGN?
		mov	edi, [ebx].pRenderSyncBuf
		xor	eax, eax
		inc	ecx
		.REPEAT
			mov	DWORD PTR [edi], eax
			add	edi, 4
			dec	ecx
		.UNTIL	ZERO?
	.ENDIF
	;
	mov	[ebx].yNextRenderSyncBuf, 0

IF	0
	;
	; GPU レンダリング用パラメータ初期化
	;
	mov	[ebx].flagDoneRayTracing, 0
	mov	[ebx].flagReadyGPURenderBuf, 0
	mov	[ebx].nLineCountGPURender, 0
	mov	[ebx].nLineCountGPURenderNext, 0
	;
	.IF	[ebx].hGPUBufRayTracing[0] != NULL
		;
		; GPU 処理用バッファに光源を設定
		;
		mov	dwSaveESP, esp
		mov	ecx, [ebx].nVectorLightCount
		add	ecx, [ebx].nPointLightCount
		shl	ecx, 4
		sub	esp, ecx
		mov	edi, esp
		xor	eax, eax
		;
		mov	esi, [ebx].pVectorLights
		ASSUME	esi:PE3D_VECTOR_LIGHT_ENTRY
		mov	ecx, [ebx].nVectorLightCount
		movaps	xmm0, xmmPackedDiv256
		test	ecx, ecx
		.WHILE	!ZERO?
			movups	xmm1, [esi].vcLight
			add	esi, (SIZEOF E3D_VECTOR_LIGHT_ENTRY)
			mulps	xmm1, xmm0
			movups	[edi + eax], xmm1
			add	eax, 4
			dec	ecx
		.ENDW
		;
		mov	esi, [ebx].pPointLights
		ASSUME	esi:PE3D_VECTOR_LIGHT_ENTRY
		mov	ecx, [ebx].nPointLightCount
		test	ecx, ecx
		.WHILE	!ZERO?
			movups	xmm1, [esi].vLight
			add	esi, (SIZEOF E3D_POINT_LIGHT_ENTRY)
			movups	[edi + eax], xmm1
			add	eax, 4
			dec	ecx
		.ENDW
		;
		mov	esi, [ebx].pGPUPluginInterface
		ASSUME	esi:PE3D_GPU_PLUGIN_INTERFACE
		INVOKE	[esi].pfnSetLightEntries ,
			[ebx].hGPUBufRayTracing,
			edi, [ebx].nVectorLightCount, [ebx].nPointLightCount
		;
		ASSUME	esi:NOTHING
		;
		mov	esp, dwSaveESP
		;
		; 同期用イベント初期化
		;
		.IF	[ebx].hGPURayTracingSyncHandle == NULL
			INVOKE	CreateEventA , NULL, 1, 0, NULL
			mov	[ebx].hGPURayTracingSyncHandle, eax
		.ENDIF
		INVOKE	ResetEvent , [ebx].hGPURayTracingSyncHandle
		;
		; 光線追跡結果エントリのバイト数計算
		;
		mov	edx, [ebx].nVectorLightCount
		mov	ecx, [ebx].rrtpRayParam.dwRayReflectCount
		add	edx, [ebx].nPointLightCount
		;
		E3D_GPU_RAY_HIT_ENTRY@CountOfBlock	edx, ecx
		;
		add	eax, [ebx].nAppendRayTracingCount
		mov	esi, eax
		imul	eax, (SIZEOF E3D_GPU_RAY_HIT_ENTRY)
		mov	[ebx].nGPURayTracingEntryBytes, eax
		;
		; 処理単位ライン数計算
		;
			mov	ecx, [ebx].rctRenderRayTracing.right
		mov	eax, 5000H * 4 ;5000H * 32
			sub	ecx, [ebx].rctRenderRayTracing.left
		xor	edx, edx
			inc	ecx
			imul	ecx, esi
		div	ecx
			mov	ecx, [ebx].rctRenderRayTracing.bottom
			sub	ecx, [ebx].rctRenderRayTracing.top
			shr	ecx, 2
		add	edx, -1
			inc	ecx
		adc	eax, 0
			cmp	eax, ecx
			cmova	eax, ecx
		mov	[ebx].nGPURayTracingLineCount, eax
		;
		; GPU 処理準備
		;
		mov	ecx, [ebx].rctRenderRayTracing.right
		mov	esi, [ebx].pGPUPluginInterface
		sub	ecx, [ebx].rctRenderRayTracing.left
		inc	eax
		imul	eax, ecx
		push	eax
		;
		ASSUME	esi:PE3D_GPU_PLUGIN_INTERFACE
		;
		INVOKE	[esi].pfnPrepareToTraceRays ,
			[ebx].hGPUBufRayTracing,
			[ebx].rrtpRayParam.rShadowingDistance,
			[ebx].rrtpRayParam.rRayTracingDistance,
			eax, [ebx].rrtpRayParam.dwRayReflectCount
		;
		ASSUME	esi:NOTHING
		;
		; 処理用バッファ確保
		;
		pop	eax
		imul	eax, [ebx].nGPURayTracingEntryBytes
		.IF	eax > [ebx].nGPURayTracingBufLimit
			mov	[ebx].nGPURayTracingBufLimit, eax
			;
			INVOKE	eslHeapReallocate ,
				[ebx].dib.hHeap,
				[ebx].pGPURayTracingBuf, eax, 0
			;
			mov	[ebx].pGPURayTracingBuf, eax
			;
			INVOKE	eslHeapReallocate ,
				[ebx].dib.hHeap,
				[ebx].pGPURayTracingBuf[4],
				[ebx].nGPURayTracingBufLimit, 0
			;
			mov	[ebx].pGPURayTracingBuf[4], eax
		.ENDIF
		;
		mov	eax, [ebx].nGPURayTracingLineCount
		.IF	eax > [ebx].nGPURayTracingSyncBufLines
			mov	[ebx].nGPURayTracingSyncBufLines, eax
			shl	eax, 2
			;
			INVOKE	eslHeapReallocate ,
				[ebx].dib.hHeap,
				[ebx].pGPURayTracingSyncBuf, eax, 0
			;
			mov	[ebx].pGPURayTracingSyncBuf, eax
		.ENDIF
		;
		mov	ecx, [ebx].nGPURayTracingLineCount
		mov	eax, [ebx].pGPURayTracingSyncBuf
		xor	edx, edx
		test	ecx, ecx
		.WHILE	!ZERO?
			mov	DWORD PTR [eax], edx
			add	eax, (SIZEOF DWORD)
			dec	ecx
		.ENDW
	.ELSE
		mov	[ebx].flagDoneRayTracing, 1
	.ENDIF
ENDIF

	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglRenderPoly@PrepareRenderRayTracing_SSE	ENDP

;
;	レイトレーシング開始 SSE コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderRayTracing_SSE	PROC	NEAR32 C USES ebx esi edi,
	hRenderPoly:HEGL_RENDER_POLYGON, hSyncRender:HEGL_RENDER_POLYGON

	LOCAL	rmhpTemp:EGL_RENDER_MESH_HIT_PARAM
	LOCAL	rmvpTemp:EGL_RENDER_MESH_VIEW_POINT
	LOCAL	rmhsSurface:EGL_RENDER_MESH_HIT_SURFACE
	LOCAL	vRay:E3D_VECTOR4
	LOCAL	vRayOrigin:E3D_VECTOR4
	LOCAL	vLastHit:E3D_VECTOR4
	LOCAL	ptNextPos:EGL_POINT
	LOCAL	xCount:DWORD
	LOCAL	ptrImageArray:PTR DWORD
	LOCAL	dwLineBytes:DWORD
	LOCAL	ptrNextPixel:PTR DWORD
	LOCAL	pLastSurface:PE3D_SURFACE_ATTRIBUTE
	LOCAL	pNextSyncBuf:PTR DWORD

;	LOCAL	rctGPURays:EGL_IMAGE_RECT
;	LOCAL	rgbGPUShadingColor:E3D_COLOR
;	LOCAL	rhRoot:E3D_GPU_RAY_HIT_ENTRY
;	LOCAL	yGPURenderIndex:DWORD

	LOCAL	dwSaveESP:DWORD

	mov	ebx, hRenderPoly
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	mov	dwSaveESP, esp
	;
	;	パラメータ初期化
	; --------------------------------------------------------------------
	mov	[ebx].nAbortRenderingFlag, 0
	mov	[ebx].nRenderedPixelCount, 0
	;
	mov	ecx, [ebx].nRayShadowings
	mov	eax, [ebx].ppRayShadowingList
	add	ecx, 3
	mov	edx, [ebx].pRayShadowing
	shr	ecx, 2
	mov	[ebx].nRayShadowingListCount, ecx
	.WHILE	!ZERO?
		mov	DWORD PTR [eax], edx
		add	eax, 4
		add	edx, (SIZEOF EGL_RENDER_MESH_MATRIX_PCK4)
		dec	ecx
	.ENDW
	;
	mov	ecx, [ebx].nRayReflections
	mov	eax, [ebx].ppRayReflectionList
	add	ecx, 3
	mov	edx, [ebx].pRayReflection
	shr	ecx, 2
	mov	[ebx].nRayReflectionListCount, ecx
	.WHILE	!ZERO?
		mov	DWORD PTR [eax], edx
		add	eax, 4
		add	edx, (SIZEOF EGL_RENDER_MESH_MATRIX_PCK4)
		dec	ecx
	.ENDW
	;
	; 変数初期化
	;
	xorps	xmm0, xmm0
	movups	vRayOrigin, xmm0
	movups	rmvpTemp.vViewPosition, xmm0
	;
	mov	ecx, hSyncRender
	.IF	ecx != NULL
		ASSUME	ecx:PTR EGL_RENDER_POLYGON_BUF
		movq	mm0, MMWORD PTR [ecx].rctRenderRayTracing.left
		movq	mm1, MMWORD PTR [ecx].rctRenderRayTracing.right
		mov	eax, [ecx].pRenderSyncBuf
		movq	MMWORD PTR [ebx].rctRenderRayTracing.left, mm0
		movq	MMWORD PTR [ebx].rctRenderRayTracing.right, mm1
	.ELSE
		mov	hSyncRender, ebx
		mov	eax, [ebx].pRenderSyncBuf
	.ENDIF
	mov	pNextSyncBuf, eax
	test	eax, eax
	jz	Label_ErrorExit
	;
	;	レンダリングループ
	; --------------------------------------------------------------------
	mov	esi, [ebx].dib.pDstImage
	ASSUME	esi:PEGL_IMAGE_INFO
	mov	eax, [esi].ptrImageArray
	mov	edx, [esi].dwBytesPerLine
	mov	ptrImageArray, eax
	mov	dwLineBytes, edx
	ASSUME	esi:NOTHING
	;
	;	CPU レンダリング
	; --------------------------------------------------------------------
	mov	ecx, [ebx].rctRenderRayTracing.top
	mov	eax, pNextSyncBuf
	.WHILE	ecx <= [ebx].rctRenderRayTracing.bottom
		;
		; ライン初期化
		;
		mov	edx, 1
		mov	ptNextPos.y, ecx
		xchg	edx, DWORD PTR [eax]
		mov	pNextSyncBuf, eax
		test	edx, edx
		jnz	Label_NextLine
		;
			pxor	mm0, mm0
		imul	ecx, dwLineBytes
			movq	MMWORD PTR rmhpTemp.rRangeMin, mm0
			movq	MMWORD PTR rmhpTemp.pExceptingMesh, mm0
			movq	MMWORD PTR rmhpTemp.pExceptingMeshEntry, mm0
		mov	eax, ptrImageArray
			mov	edx, [ebx].rctRenderRayTracing.left
		add	eax, ecx
			movq	MMWORD PTR rmvpTemp.pExceptingMesh, mm0
		lea	eax, [eax + edx * 4]
			mov	xCount, 0
		mov	ptrNextPixel, eax
		;
		.WHILE	edx <= [ebx].rctRenderRayTracing.right
			mov	ptNextPos.x, edx
			;
			; 光線当たり判定
			;
			cvtpi2ps	xmm0, QWORD PTR ptNextPos
			movlps		xmm2, QWORD PTR [ebx].vScreenPos.x
			movss		xmm1, [ebx].vScreenPos.z
			subps		xmm0, xmm2
			movlhps		xmm0, xmm1
			movaps		xmm1, xmm0
			mulps		xmm0, xmm0
			movhlps		xmm2, xmm0
			addss		xmm2, xmm0
			shufps		xmm0, xmm0, 1
			addss		xmm0, xmm2
			sqrtss		xmm0, xmm0
			shufps		xmm0, xmm0, 0
			divps		xmm1, xmm0
			mov		rmhpTemp.rRangeMin, 0
			mov		rmhpTemp.rRangeMax, 7F000000H
			mov		rmhpTemp.pExceptingMesh, 0
			mov		rmhpTemp.pExceptingMeshEntry, 0
			mov		rmhpTemp.nResultLimit, 1
			xorps		xmm4, xmm4
			movups		vRay, xmm1
			movups		vRayOrigin, xmm4
Label_LoopRayTrace:
			INVOKE	eglRenderPoly@MeshList@IsHitSegmentSSE,
					[ebx].ppRayReflectionList,
					[ebx].nRayReflectionListCount,
					ADDR rmhpTemp,
					ADDR vRay, ADDR vRayOrigin
			;
			.IF	rmhpTemp.nResultCount == 0
				INVOKE	eglRenderPoly@MeshArray@IsHitSegmentSSE,
						[ebx].pGlobalRayReflection,
						[ebx].nGlobalRayReflections,
						ADDR rmhpTemp,
						ADDR vRay, ADDR vRayOrigin
				;
				.IF	rmhpTemp.nResultCount == 0
Label_NoSurface:
					call	SubFunc_LineShading
					;
					mov	edi, ptrNextPixel
					inc	[ebx].nRenderedPixelCount
					mov	DWORD PTR [edi], 0
					add	edi, 4
					mov	ptrNextPixel, edi
					jmp	Label_ContinueLoop
				.ENDIF
			.ENDIF
			;
			; 表面属性取得
			;
			INVOKE	eglRenderPoly@Polygon@GetHitPointSurface,
					ebx, ADDR rmhsSurface, ADDR rmhpTemp.rmheResults
			;
			.IF	rmhsSurface.dwShadingFlags & E3DSAF_SINGLE_SIDE_PLANE
				movups	xmm0, rmhsSurface.vNormal
				movups	xmm1, vRay
				mulps	xmm0, xmm1
				xorps	xmm2, xmm2
				movhlps	xmm1, xmm0
				addss	xmm1, xmm0
				shufps	xmm0, xmm0, 1
				addss	xmm0, xmm1
				comiss	xmm0, xmm2
				ja	Label_FindBackSide
			.ENDIF
Label_FrontSideSurface:
			mov	edx, rmvpTemp.nExceptingMeshIndex
			mov	eax, rmvpTemp.pExceptingMesh
			.IF	(edx != rmhpTemp.rmheResults.nHitMeshIndex) \
					|| (eax != rmhpTemp.rmheResults.pHitMesh)
				call	SubFunc_LineShading
				movq	mm0, MMWORD PTR rmhpTemp.rmheResults.pHitMesh
				movq	MMWORD PTR rmvpTemp.pExceptingMesh, mm0
			.ENDIF
			;
			; 表面データ追加
			;
			mov	ecx, xCount
			mov	eax, rmhsSurface.pAttr
			mov	esi, [ebx].pLineVectorBuf
			mov	edi, [ebx].pLineNormalBuf
			shl	ecx, 2
			mov	pLastSurface, eax
			movups	xmm0, rmhsSurface.vPlane
			mov	edx, [ebx].pLineColorBuf
			mov	eax, [ebx].pLineTextureBuf
			movups	xmm1, XMMWORD_PTR rmhpTemp.rmheResults.vHitPosition
			movups	xmm2, rmhsSurface.vNormal
			movq	mm0, MMWORD PTR rmhsSurface.clrSurface
			movd	mm1, rmhsSurface.rgbaTexture
			;
			movups	rmvpTemp.vTargetPlane, xmm0
			movaps	[esi + ecx * 4], xmm1
				mov	esi, xCount
			movaps	[edi + ecx * 4], xmm2
				inc	esi
			movq	MMWORD PTR [edx + ecx * 2], mm0
			movd	DWORD PTR [eax + ecx], mm1
				mov	xCount, esi
Label_ContinueLoop:
			cmp	[ebx].nAbortRenderingFlag, 0
			mov	edx, ptNextPos.x
			jnz	Label_AbortExit
			inc	edx
		.ENDW
		;
		call	SubFunc_LineShading
Label_NextLine:
		;
		; 次のラインへ
		;
		mov	eax, pNextSyncBuf
		mov	ecx, ptNextPos.y
		add	eax, 4
		inc	ecx
	.ENDW
	;
	emms
	mov	esp, dwSaveESP
	mov	[ebx].nAbortRenderingFlag, 0
	xor	eax, eax
	ret

Label_ErrorExit:
	emms
	mov	esp, dwSaveESP
	mov	[ebx].nAbortRenderingFlag, 0
	mov	eax, eslErrGeneral
	ret

Label_AbortExit:
	emms
	mov	esp, dwSaveESP
	mov	[ebx].nAbortRenderingFlag, 0
	mov	eax, eslErrAbort
	ret


ALIGN	10H
Label_FindBackSide:
	;
	;	裏面ポリゴンの場合
	; --------------------------------------------------------------------
	;
	; 裏面ポリゴンのみを除外してもう一度光線追跡
	;
	mov	eax, rmhpTemp.rmheResults.pHitMesh
	mov	edx, rmhpTemp.rmheResults.nHitMeshIndex
	mov	rmhpTemp.rRangeMax, 7F000000H
	mov	rmhpTemp.pExceptingMesh, eax
	mov	rmhpTemp.nExceptingMeshIndex, edx
	mov	rmhpTemp.pExceptingMeshEntry, 0
	mov	rmhpTemp.nResultLimit, 1
	;
	INVOKE	eglRenderPoly@MeshList@IsHitSegmentSSE,
			[ebx].ppRayReflectionList,
			[ebx].nRayReflectionListCount,
			ADDR rmhpTemp,
			ADDR vRay, ADDR vRayOrigin
	;
	.IF	rmhpTemp.nResultCount == 0
		INVOKE	eglRenderPoly@MeshArray@IsHitSegmentSSE,
				[ebx].pGlobalRayReflection,
				[ebx].nGlobalRayReflections,
				ADDR rmhpTemp,
				ADDR vRay, ADDR vRayOrigin
		;
		cmp	rmhpTemp.nResultCount, 0
		jz	Label_NoSurface
	.ENDIF
	;
	; 裏面判定
	;
	INVOKE	eglRenderPoly@Polygon@GetHitPointSurface,
			ebx, ADDR rmhsSurface, ADDR rmhpTemp.rmheResults
	;
	test	rmhsSurface.dwShadingFlags, E3DSAF_SINGLE_SIDE_PLANE
	jz	Label_FrontSideSurface
	;
	movups	xmm0, rmhsSurface.vNormal
	movups	xmm1, vRay
	mulps	xmm0, xmm1
	xorps	xmm2, xmm2
	movhlps	xmm1, xmm0
	addss	xmm1, xmm0
	shufps	xmm0, xmm0, 1
	addss	xmm0, xmm1
	comiss	xmm0, xmm2
	jbe	Label_FrontSideSurface
	;
	; 同じメッシュにヒットしている場合は
	; メッシュを除外して再度光線追跡
	;
	movups	xmm0, XMMWORD_PTR rmhpTemp.rmheResults.vHitPosition
	mov	eax, rmhpTemp.rmheResults.pHitMesh
	movups	vLastHit, xmm0
	.IF	eax == rmhpTemp.pExceptingMesh
		mov	rmhpTemp.pExceptingMeshEntry, eax
		mov	rmhpTemp.rRangeMax, 7F000000H
		mov	rmhpTemp.nResultLimit, 1
		;
		INVOKE	eglRenderPoly@MeshList@IsHitSegmentSSE,
				[ebx].ppRayReflectionList,
				[ebx].nRayReflectionListCount,
				ADDR rmhpTemp,
				ADDR vRay, ADDR vRayOrigin
		;
		.IF	rmhpTemp.nResultCount == 0
			INVOKE	eglRenderPoly@MeshArray@IsHitSegmentSSE,
					[ebx].pGlobalRayReflection,
					[ebx].nGlobalRayReflections,
					ADDR rmhpTemp,
					ADDR vRay, ADDR vRayOrigin
			;
			cmp	rmhpTemp.nResultCount, 0
			jz	Label_NoSurface
		.ENDIF
		;
		; 裏面判定
		;
		INVOKE	eglRenderPoly@Polygon@GetHitPointSurface,
				ebx, ADDR rmhsSurface, ADDR rmhpTemp.rmheResults
		;
		test	rmhsSurface.dwShadingFlags, E3DSAF_SINGLE_SIDE_PLANE
		jz	Label_FrontSideSurface
		;
		movups	xmm0, rmhsSurface.vNormal
		movups	xmm1, vRay
		mulps	xmm0, xmm1
		xorps	xmm2, xmm2
		movhlps	xmm1, xmm0
		addss	xmm1, xmm0
		shufps	xmm0, xmm0, 1
		addss	xmm0, xmm1
		comiss	xmm0, xmm2
		jbe	Label_FrontSideSurface
	.ENDIF
	;
	movups	xmm0, vLastHit
	movaps	xmm2, xmm0
	mulps	xmm0, xmm0
	movhlps	xmm1, xmm0
	addss	xmm1, xmm0
	shufps	xmm0, xmm0, 1
	addss	xmm0, xmm1
	sqrtss	xmm0, xmm0
	movups	xmm1, vRay
	mulss	xmm0, xmmErrorScale
	movss	rmhpTemp.rRangeMin, xmm0
	shufps	xmm0, xmm0, 0
	mulps	xmm0, xmm1
	addps	xmm0, xmm2
	movups	vRayOrigin, xmm0
	jmp	Label_FindBackSide


ALIGN	10H
SubFunc_LineShading:
	;
	;	シェーディング処理
	; --------------------------------------------------------------------
	.IF	xCount != 0
		INVOKE	eglRenderPoly@ShadeVectorsAndRaySSE,
				ebx, pLastSurface,
				[ebx].pLineNormalBuf,
				[ebx].pLineVectorBuf,
				[ebx].pLineColorBuf,
				[ebx].pLineTextureBuf,
				xCount, ADDR rmvpTemp,
				[ebx].rrtpRayParam.dwRayReflectCount
		;
		mov	esi, [ebx].pLineColorBuf
			mov	edx, 0FF000000H
		mov	edi, ptrNextPixel
			movd	mm7, edx
		mov	ecx, xCount
			punpckldq	mm7, mm7
		;
		add	[ebx].nRenderedPixelCount, ecx
		;
		@SOURCE_ALIGN = (SIZEOF E3D_COLOR)
		ASSUME	esi:PTR E3D_COLOR
		sub	ecx, 4
		.WHILE	!SIGN?
			movd	mm0, [esi].rgbAdd.dwPixelCode
			movd	mm1, [esi + @SOURCE_ALIGN].rgbAdd.dwPixelCode
			movd	mm2, [esi + @SOURCE_ALIGN*2].rgbAdd.dwPixelCode
			movd	mm3, [esi + @SOURCE_ALIGN*3].rgbAdd.dwPixelCode
			add	esi, @SOURCE_ALIGN * 4
			punpckldq	mm0, mm1
			punpckldq	mm2, mm3
			por	mm0, mm7
			por	mm2, mm7
			movq	MMWORD PTR [edi], mm0
			movq	MMWORD PTR [edi + 8], mm2
			add	edi, 16
			sub	ecx, 4
		.ENDW
		add	ecx, 4
		.WHILE	!ZERO?
			mov	eax, (E3D_COLOR PTR [esi]).rgbAdd.dwPixelCode
			add	esi, (SIZEOF E3D_COLOR)
			or	eax, 0FF000000H
			mov	DWORD PTR [edi], eax
			add	edi, 4
			dec	ecx
		.ENDW
		;
		mov	ptrNextPixel, edi
		mov	xCount, ecx
	.ENDIF
	BYTE	0C3H	; ret

	ASSUME	ebx:NOTHING

eglRenderPoly@RenderRayTracing_SSE	ENDP

;
;	レイトレーシング中止 SSE コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@AbortRenderRayTracing_SSE	PROC	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON

	mov	ecx, hRenderPoly
	ASSUME	ecx:PTR EGL_RENDER_POLYGON_BUF

	mov	[ecx].nAbortRenderingFlag, 1

	ASSUME	ecx:NOTHING
	xor	eax, eax
	ret

eglRenderPoly@AbortRenderRayTracing_SSE	ENDP

;
;	レイトレーシング進行状況取得 SSE コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@GetProgressRayTracing_SSE	PROC	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON, pdwPixelCount:PTR DWORD

	mov	ecx, hRenderPoly
	mov	edx, pdwPixelCount
	ASSUME	ecx:PTR EGL_RENDER_POLYGON_BUF

	mov	eax, [ecx].nRenderedPixelCount
	mov	DWORD PTR [edx], eax

	ASSUME	ecx:NOTHING
	xor	eax, eax
	ret

eglRenderPoly@GetProgressRayTracing_SSE	ENDP

;
;	レイトレーシングレンダリング領域割り当て
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@AllocateRayTraceRect	PROC	NEAR32 C USES ebx esi edi,
	hRenderPoly:HEGL_RENDER_POLYGON,
	pRenderRect:PTR EGL_RECT,
	nLineCount:DWORD, nOffset:DWORD

	mov	ebx, hRenderPoly
	mov	edi, pRenderRect
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	ASSUME	edi:PTR EGL_RECT

	mov	eax, [ebx].rctRenderRayTracing.left
	mov	edx, [ebx].rctRenderRayTracing.right
	mov	[edi].left, eax
	mov	[edi].right, edx

	mov	esi, [ebx].pRenderSyncBuf
	mov	edx, [ebx].rctRenderRayTracing.top
	mov	ecx, [ebx].yNextRenderSyncBuf
	lea	esi, [esi + ecx * (SIZEOF DWORD)]
	add	edx, ecx
	;
	mov	eax, nOffset
	add	eax, edx
	.IF	eax < [ebx].rctRenderRayTracing.bottom
	.WHILE	(SDWORD PTR edx) <= [ebx].rctRenderRayTracing.bottom
		mov	eax, 1
		xchg	eax, DWORD PTR [esi]
		inc	edx
		add	esi, (SIZEOF DWORD)
		;
		.IF	eax == 0
			lea	eax, [edx - 1]
			mov	[edi].top, eax
			;
			.WHILE	(SDWORD PTR edx) \
					<= [ebx].rctRenderRayTracing.bottom
				dec	nLineCount
				.BREAK	.IF	ZERO?
				;
				mov	eax, 1
				xchg	eax, DWORD PTR [esi]
				.BREAK	.IF	eax != 0
				inc	edx
				add	esi, (SIZEOF DWORD)
			.ENDW
			;
			dec	edx
			mov	[edi].bottom, edx
			inc	edx
			mov	eax, edx
			sub	edx, [ebx].rctRenderRayTracing.top
			sub	eax, [edi].top
			mov	[ebx].yNextRenderSyncBuf, edx
			ret
		.ENDIF
	.ENDW
	.ENDIF
	;
	sub	edx, [ebx].rctRenderRayTracing.top
	mov	[ebx].yNextRenderSyncBuf, edx

	ASSUME	ebx:NOTHING
	ASSUME	edi:NOTHING
	xor	eax, eax
	ret

eglRenderPoly@AllocateRayTraceRect	ENDP


CodeSeg	ENDS

	END
