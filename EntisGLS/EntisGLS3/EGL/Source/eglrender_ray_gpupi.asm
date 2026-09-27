
; ----------------------------------------------------------------------------
;                          Entis Graphic Library
; ----------------------------------------------------------------------------
;    Copyright (c) 2009 Leshade Entis, Entis-soft. Al rights reserved.
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
xmmPackedDiv256		REAL4	4 DUP( 0.00390625 )
xmmPackedDivM256	REAL4	4 DUP( -0.00390625 )
xmmPackedDiv1024	REAL4	4 DUP( 0.0009765625 )
xmmPacked1p001		REAL4	4 DUP( 1.001 )

ConstSeg	ENDS


; ----------------------------------------------------------------------------
;	コードセグメント
; ----------------------------------------------------------------------------

CodeSeg	SEGMENT	PARA READONLY FLAT 'CODE'


IF	0
;
;	GPU プラグイン光線追跡処理結果待ち
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@GetResultToTraceRays	PROC	NEAR32 C USES ebx esi edi,
	hRenderPoly:HEGL_RENDER_POLYGON

	mov	ebx, hRenderPoly
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	xor	eax, eax

	mov	esi, [ebx].pGPUPluginInterface
	ASSUME	esi:PE3D_GPU_PLUGIN_INTERFACE
	.IF	esi != NULL
		.IF	[ebx].hGPUBufRayTracing != NULL
			INVOKE	[esi].pfnGetResultToTraceRays ,
					[ebx].hGPUBufRayTracing
		.ELSE
			mov	eax, eslErrGeneral
		.ENDIF
	.ENDIF
	ASSUME	esi:NOTHING

	ASSUME	ebx:NOTHING
	ret

eglRenderPoly@GetResultToTraceRays	ENDP

ALIGN	10H
eglRenderPoly@GetResultToTraceRaysSSE	PROC	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON

	xor	eax, eax
	ret

eglRenderPoly@GetResultToTraceRaysSSE	ENDP

;
;	GPU プラグイン光線追跡処理開始
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@BeginTraceRays		PROC	NEAR32 C USES ebx esi edi,
	hRenderPoly:HEGL_RENDER_POLYGON,
	pRayHitResults:PTR E3D_GPU_RAY_HIT_ENTRY,
	pRays:PTR E3D_GPU_RAY_VECTOR_ENTRY, nRayCount:DWORD

	mov	ebx, hRenderPoly
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	mov	eax, eslErrGeneral

	mov	esi, [ebx].pGPUPluginInterface
	ASSUME	esi:PE3D_GPU_PLUGIN_INTERFACE
	.IF	(esi != NULL) && ([ebx].hGPUBufRayTracing != NULL)
		INVOKE	[esi].pfnBeginTraceRays ,
			[ebx].hGPUBufRayTracing,
			pRayHitResults, pRays, nRayCount,
			[ebx].rrtpRayParam.dwRayReflectCount,
			[ebx].nAppendRayTracingCount
	.ENDIF
	ASSUME	esi:NOTHING

	ASSUME	ebx:NOTHING
	ret

eglRenderPoly@BeginTraceRays		ENDP


;
;	GPU プラグイン光線追跡処理 SSE エミュレーション関数
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@PerformTraceRaySSE	PROC	NEAR32 C USES ebx esi edi,
	hRenderPoly:HEGL_RENDER_POLYGON,
	pRayHitResults:PTR E3D_GPU_RAY_HIT_ENTRY,
	pRays:PTR E3D_GPU_RAY_VECTOR_ENTRY,
	pExceptionMesh:PE3D_POLYGON_ENTRY,
	nExceptingPolyIndex:DWORD,
	nReflectionCount:DWORD

	LOCAL	rmhpTemp:EGL_RENDER_MESH_HIT_PARAM
	LOCAL	vTemp[3]:E3D_VECTOR4
	LOCAL	dwTemp[2]:DWORD
	LOCAL	pHitPoly:PE3D_POLYGON_ENTRY
	LOCAL	pHitPolyAttr:PE3D_SURFACE_ATTRIBUTE
	LOCAL	iHitPoly:DWORD
	LOCAL	vHitPos:E3D_VECTOR4
	LOCAL	vHitUV:E3D_VECTOR_2D
	LOCAL	vHitNormal:E3D_VECTOR4
	LOCAL	pNormals:PE3D_VECTOR4
	LOCAL	dwShadingFlags:DWORD
	LOCAL	nRefraction:REAL4
	LOCAL	rvRayPos:E3D_GPU_RAY_VECTOR_ENTRY
	LOCAL	nLightCount:DWORD
	LOCAL	nRayCount:DWORD
	LOCAL	vShadowOrgPos:E3D_VECTOR4
	LOCAL	rShadowAccumulation:REAL4
	LOCAL	rShadowDistance:REAL4
	LOCAL	pNextVectorLight:PE3D_VECTOR_LIGHT_ENTRY
	LOCAL	pNextPointLight:PE3D_POINT_LIGHT_ENTRY

	mov	ebx, hRenderPoly
	mov	edi, pRayHitResults
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	ASSUME	edi:PTR E3D_GPU_RAY_HIT_ENTRY

	;
	;	光線追跡１次処理
	; --------------------------------------------------------------------
	mov	eax, pExceptionMesh
	mov	edx, nExceptingPolyIndex
	mov	rmhpTemp.rRangeMin, 0
	mov	rmhpTemp.rRangeMax, 7F000000H
	mov	rmhpTemp.pLastHitMesh, NULL
	mov	rmhpTemp.pExceptingMesh, eax
	mov	rmhpTemp.nExceptingMeshIndex, edx
	mov	rmhpTemp.pExceptingMeshEntry, 0
	;
	mov	esi, pRays
	ASSUME	esi:PTR E3D_GPU_RAY_VECTOR_ENTRY
	;
	mov	eax, [esi].vPos.z
	sub	eax, (10 SHL 23)
	mov	rmhpTemp.rRangeMin, eax
	;
	INVOKE	eglRenderPoly@MeshList@IsHitSegmentSSE,
			[ebx].ppRayReflectionList,
			[ebx].nRayReflectionListCount,
			ADDR rmhpTemp,
			ADDR [esi].vRay, ADDR [esi].vPos
	;
	.IF	rmhpTemp.pLastHitMesh == NULL
		mov	rmhpTemp.rRangeMax, 7F000000H
		INVOKE	eglRenderPoly@MeshArray@IsHitSegmentSSE,
				[ebx].pGlobalRayReflection,
				[ebx].nGlobalRayReflections,
				ADDR rmhpTemp,
				ADDR [esi].vRay, ADDR [esi].vPos
	.ENDIF
	;
	mov	eax, rmhpTemp.pLastHitMesh
	mov	edx, rmhpTemp.nLastHitMeshIndex
	mov	[edi].ppeMesh, eax
	mov	[edi].iPolyEntry, edx
	;
	test	eax, eax
	jz	Label_Exit
	;
	;	ヒットポリゴンの属性を取得する
	; --------------------------------------------------------------------
	ASSUME	eax:PE3D_POLYGON_ENTRY
	movups	xmm0, rmhpTemp.vHitPosition
	mov	pHitPoly, eax
	mov	iHitPoly, edx
	movups	vHitPos, xmm0
	mov	edx, [eax].dwShadingFlags
	mov	dwShadingFlags, edx
	ASSUME	eax:NOTHING
	;
	call	SubFunc_SetRayHitResult
	ASSUME	esi:NOTHING
	;
	movlps	xmm0, QWORD PTR [edi].vUV
	movlps	QWORD PTR vHitUV, xmm0
	;
	add	edi, (SIZEOF E3D_GPU_RAY_HIT_ENTRY)
	cmp	nReflectionCount, 0
	jz	Label_Exit
	;
	;	陰用光線追跡
	; --------------------------------------------------------------------
	;
	; 陰付け用共通パラメータ設定
	;
	mov	eax, pHitPoly
	xor	edx, edx
	test	dwShadingFlags, E3DSAF_NO_SALF_SHADOW
	cmovz	eax, edx
	mov	rmhpTemp.pExceptingMeshEntry, eax
	;
	; 平行光追跡
	;
	mov	ecx, [ebx].nVectorLightCount
	mov	esi, [ebx].pVectorLights
	test	ecx, ecx
	mov	pNextVectorLight, esi
	.WHILE	!ZERO?
		;
		; 光線情報設定
		;
		mov	esi, pNextVectorLight
		mov	nLightCount, ecx
		ASSUME	esi:PE3D_VECTOR_LIGHT_ENTRY
		movups	xmm0, [esi].vcLight
		mulps	xmm0, xmmPackedDivM256
		movups	rvRayPos.vRay, xmm0
		ASSUME	esi:NOTHING
		;
		mov	eax, vHitPos.z
			add	esi, (SIZEOF E3D_VECTOR_LIGHT_ENTRY)
		sub	eax, (10 SHL 23)
			mov	pNextVectorLight, esi
		mov	edx, [ebx].rrtpRayParam.rShadowingDistance
		mov	ecx, nReflectionCount
		mov	rmhpTemp.rRangeMin, eax
		mov	rmhpTemp.rRangeMax, edx
		mov	rShadowDistance, edx
		;
		call	SubFunc_Shadowing
		;
		mov	ecx, nLightCount
		dec	ecx
	.ENDW
	;
	; 点光源追跡
	;
	mov	eax, rmhpTemp.pLastHitMesh
	mov	edx, rmhpTemp.nLastHitMeshIndex
	mov	rmhpTemp.pExceptingMesh, eax
	mov	rmhpTemp.nExceptingMeshIndex, edx
	;
	mov	ecx, [ebx].nPointLightCount
	mov	esi, [ebx].pPointLights
	test	ecx, ecx
	mov	pNextPointLight, esi
	.WHILE	!ZERO?
		;
		; 光線情報設定
		;
		mov	esi, pNextPointLight
		mov	nLightCount, ecx
		ASSUME	esi:PE3D_POINT_LIGHT_ENTRY
		;
		movups	xmm0, [esi].vLight
		movups	xmm1, vHitPos
		subps	xmm0, xmm1
		;
		movaps	xmm1, xmm0
		mulps	xmm0, xmm0
		movss	xmm2, xmm0
		movhlps	xmm3, xmm0
		addss	xmm0, xmm2
		addss	xmm0, xmm3
		sqrtss	xmm0, xmm0
		movss	rShadowDistance, xmm0
		shufps	xmm0, xmm0, 0
		divps	xmm1, xmm0
		;
		movups	rvRayPos.vRay, xmm1
		ASSUME	esi:NOTHING
		;
		mov	eax, vHitPos.z
			add	esi, (SIZEOF E3D_POINT_LIGHT_ENTRY)
		sub	eax, (10 SHL 23)
			mov	pNextPointLight, esi
		mov	edx, rShadowDistance
		mov	ecx, nReflectionCount
		mov	rmhpTemp.rRangeMin, eax
		mov	rmhpTemp.rRangeMax, edx
		;
		call	SubFunc_Shadowing
		;
		mov	ecx, nLightCount
		dec	ecx
	.ENDW
	;
	;	反射・屈折判定
	; --------------------------------------------------------------------
	mov	ecx, pHitPoly
	ASSUME	ecx:PE3D_POLYGON_ENTRY
	mov	esi, [ecx].pAttr
	ASSUME	esi:PE3D_SURFACE_ATTRIBUTE
	mov	pHitPolyAttr, esi
	;
	mov	eax, [esi].nRefraction
	mov	nRefraction, eax
	;
	mov	eax, dwShadingFlags
	mov	edx, eax
	and	eax, NOT E3DSAF_RAY_REFLECTING
	cmp	[esi].nReflection, 0
	cmovnz	eax, edx
	;
	.IF	([ecx].dwTransparency == 0) \
			&& !(dwShadingFlags & E3DSAF_SINGLE_SIDE_PLANE) \
			&& ([esi].nTransparency == 0) \
			&& ([esi].nDeepness == 0)
		.IF	dwShadingFlags & E3DSAF_TEXTURE_MAPPING
			mov	ecx, [esi].txmap.pTextureImage
			ASSUME	ecx:PEGL_IMAGE_INFO
			.IF	!([ecx].fdwFormatType & EIF_WITH_ALPHA)
				and	eax, NOT E3DSAF_RAY_REFRACTING
			.ENDIF
			ASSUME	ecx:NOTHING
		.ELSEIF	!([esi].rgbaColor.rgbMul.dwPixelCode & 0FFFFFFH)
			and	eax, NOT E3DSAF_RAY_REFRACTING
		.ENDIF
	.ENDIF
	;
	mov	dwShadingFlags, eax
	.IF	!(eax & (E3DSAF_RAY_REFLECTING OR E3DSAF_RAY_REFRACTING))
		mov	edx, [ebx].nVectorLightCount
		mov	ecx, nReflectionCount
		add	edx, [ebx].nPointLightCount
		dec	ecx
		;
		E3D_GPU_RAY_HIT_ENTRY@CountOfBlock	edx, ecx
		;
		lea	ecx, [eax * 2]
		call	SubFunc_FillDummy
		;
		jmp	Label_Exit
	.ENDIF
	;
	ASSUME	ecx:NOTHING
	ASSUME	esi:NOTHING
	;
	;	法線計算
	; --------------------------------------------------------------------
	mov	esi, pHitPoly
	ASSUME	esi:PE3D_POLYGON_ENTRY
	mov	ecx, iHitPoly
	mov	edx, [esi].surface.mesh.pMesh
	mov	eax, [esi].pNormals
	shl	ecx, 4
	mov	pNormals, eax
	;
	ASSUME	edx:PTR E3D_PRIMITIVE_MESH_LIST
	lea	edx, [edx].mpEntries[ecx]
	ASSUME	edx:PTR E3D_PRIMITIVE_MESH_POLY
	;
	mov	eax, [edx].dwIndex[0]
	mov	ecx, [edx].dwIndex[4]
	mov	edx, [edx].dwIndex[8]
	ASSUME	edx:NOTHING
	;
	sub	ecx, eax
	sub	edx, eax
	shl	eax, 4
	shl	ecx, 4
	shl	edx, 4
	add	eax, pNormals
	;
	movups	xmm0, [eax]		; xmm0 = normal0
	movups	xmm1, [eax + ecx]	; xmm1 = normal1 - normal0
	movups	xmm2, [eax + edx]	; xmm2 = normal2 - normal0
	subps	xmm1, xmm0
	subps	xmm2, xmm0
	;
	movss	xmm3, vHitUV.x
	movss	xmm4, vHitUV.y
	shufps	xmm3, xmm3, 0
	shufps	xmm4, xmm4, 0
	;
	mulps	xmm1, xmm3
	mulps	xmm2, xmm4
	addps	xmm0, xmm1
	addps	xmm0, xmm2
	;
	movaps	xmm1, xmm0		; ベクトル正規化
	mulps	xmm0, xmm0
	movss	xmm2, xmm0
	movhlps	xmm3, xmm0
	shufps	xmm0, xmm0, 1
	addss	xmm2, xmm3
	addss	xmm0, xmm2
	sqrtss	xmm0, xmm0
	shufps	xmm0, xmm0, 0
	divps	xmm1, xmm0
	;
	movups	vHitNormal, xmm1
	;
	ASSUME	esi:NOTHING
	;
	;	屈折光線追跡
	; --------------------------------------------------------------------
	.IF	dwShadingFlags & E3DSAF_RAY_REFRACTING
		;
		; 屈折光線計算
		;
		movss	xmm0, nRefraction
			mov	eax, pRays
		shufps	xmm0, xmm0, 0
			movups	xmm2, (E3D_GPU_RAY_VECTOR_ENTRY PTR [eax]).vRay
		mulps	xmm0, xmm1
		subps	xmm2, xmm0
		;
		movaps	xmm3, xmm2
		mulps	xmm2, xmm2
		movss	xmm4, xmm2
		movhlps	xmm5, xmm2
		shufps	xmm2, xmm2, 1
		addss	xmm4, xmm5
		addss	xmm2, xmm4
		sqrtss	xmm2, xmm2
		shufps	xmm2, xmm2, 0
		divps	xmm3, xmm2
		;
		movups	xmm0, vHitPos
		movups	xmm1, vHitNormal
		movups	xmm2, (E3D_GPU_RAY_VECTOR_ENTRY PTR [eax]).vRay
		movups	rvRayPos.vRay, xmm3
		movups	rvRayPos.vPos, xmm0
		;
		; 臨界角判定
		;
		mulps	xmm2, xmm1
			mulps	xmm3, xmm1
		movhlps	xmm0, xmm2
			movhlps	xmm4, xmm3
		movss	xmm1, xmm2
			movss	xmm5, xmm3
		shufps	xmm2, xmm2, 1
			shufps	xmm3, xmm3, 1
		addss	xmm0, xmm1
			addss	xmm4, xmm5
		addss	xmm0, xmm2
			addss	xmm4, xmm3
		movss	dwTemp[0], xmm0
			movss	dwTemp[4], xmm4
		mov	eax, dwTemp[0]
		xor	eax, dwTemp[4]
		js	Label_WithoutRefraction
		;
		; 屈折光線追跡
		;
		mov	ecx, nReflectionCount
		dec	ecx
		;
		INVOKE	eglRenderPoly@PerformTraceRaySSE ,
				ebx, edi, ADDR rvRayPos,
				pHitPoly, iHitPoly, ecx
		;
		; 次のエントリへ
		;
		mov	edx, [ebx].nVectorLightCount
		mov	ecx, nReflectionCount
		add	edx, [ebx].nPointLightCount
		dec	ecx
		;
		E3D_GPU_RAY_HIT_ENTRY@CountOfBlock	edx, ecx
		;
		imul	eax, (SIZEOF E3D_GPU_RAY_HIT_ENTRY)
		add	edi, eax

	.ELSE
Label_WithoutRefraction:
		mov	edx, [ebx].nVectorLightCount
		mov	ecx, nReflectionCount
		add	edx, [ebx].nPointLightCount
		dec	ecx
		;
		E3D_GPU_RAY_HIT_ENTRY@CountOfBlock	edx, ecx
		;
		mov	ecx, eax
		call	SubFunc_FillDummy
	.ENDIF
	;
	;	反射光線追跡
	; --------------------------------------------------------------------
	.IF	dwShadingFlags & E3DSAF_RAY_REFLECTING
		;
		; 反射光線計算
		;	L' = L - 2 * <L|A> / (|A|^2) * A
		;	   = L - <L|A> / 8000H * A
		;
		mov	eax, pRays
		movups	xmm0, vHitNormal
		movups	xmm7, (E3D_GPU_RAY_VECTOR_ENTRY PTR [eax]).vRay
		movaps	xmm6, xmm0
		mulps	xmm0, xmm7		; xmm0 = <L|A>
		addps	xmm6, xmm6		; xmm6 = 2 * A
		movhlps	xmm1, xmm0
		movss	xmm2, xmm0
		shufps	xmm0, xmm0, 1
		addss	xmm1, xmm2
		addss	xmm0, xmm1
		shufps	xmm0, xmm0, 0
		mulps	xmm0, xmm6		; xmm0 = 2 * <L|A> * A
		subps	xmm7, xmm0
		;
		movups	xmm0, vHitPos
		movups	rvRayPos.vRay, xmm7
		movups	rvRayPos.vPos, xmm0
		;
		; 屈折光線追跡
		;
		mov	ecx, nReflectionCount
		dec	ecx
		;
		INVOKE	eglRenderPoly@PerformTraceRaySSE ,
				ebx, edi, ADDR rvRayPos,
				pHitPoly, iHitPoly, ecx

	.ELSE
		mov	edx, [ebx].nVectorLightCount
		mov	ecx, nReflectionCount
		add	edx, [ebx].nPointLightCount
		dec	ecx
		;
		E3D_GPU_RAY_HIT_ENTRY@CountOfBlock	edx, ecx
		;
		mov	ecx, eax
		call	SubFunc_FillDummy
	.ENDIF

Label_Exit:
	ASSUME	edi:NOTHING
	ASSUME	ebx:NOTHING
	ret


ALIGN	10H
SubFunc_Shadowing:
	;
	;	陰付け用光線追跡
	; --------------------------------------------------------------------
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	ASSUME	edi:PTR E3D_GPU_RAY_HIT_ENTRY
	mov	nRayCount, ecx
	;
	.IF	dwShadingFlags & E3DSAF_RAY_SHADOWING
	;
	movups	xmm0, vHitPos
	mov	eax, pHitPoly
	mov	edx, iHitPoly
	mov	rmhpTemp.pExceptingMesh, eax
	mov	rmhpTemp.nExceptingMeshIndex, edx
	movups	vShadowOrgPos, xmm0
	mov	rShadowAccumulation, 0
	;
	.REPEAT
		;
		; 光線追跡
		;
		mov	nRayCount, ecx
		mov	rmhpTemp.pLastHitMesh, NULL
		;
		INVOKE	eglRenderPoly@MeshArray@IsHitSegmentSSE,
				[ebx].pRayShadowing,
				[ebx].nRayShadowings,
				ADDR rmhpTemp,
				ADDR rvRayPos.vRay,
				ADDR vShadowOrgPos
		;
		.BREAK	.IF	rmhpTemp.pLastHitMesh == NULL
		;
		; 当たり判定情報を設定
		;
		mov	eax, rmhpTemp.pLastHitMesh
		mov	edx, rmhpTemp.nLastHitMeshIndex
		mov	[edi].ppeMesh, eax
		mov	[edi].iPolyEntry, edx
		mov	rmhpTemp.pExceptingMesh, eax
		mov	rmhpTemp.nExceptingMeshIndex, edx
		;
		lea	esi, rvRayPos
		ASSUME	esi:PTR E3D_GPU_RAY_VECTOR_ENTRY
		call	SubFunc_SetRayHitResult
		ASSUME	esi:NOTHING
		;
		; 次の当たり判定へ
		;
		movss	xmm4, [edi].rDistance
		movss	xmm5, rShadowAccumulation
		movups	xmm2, rmhpTemp.vHitPosition
		movss	xmm1, rShadowDistance
		addss	xmm4, xmm5
			movss	xmm0, rmhpTemp.rRangeMax
			mov	eax, rmhpTemp.vHitPosition.z
			addss	xmm5, xmm0
		subss	xmm1, xmm0
		movss	[edi].rDistance, xmm4
			movss	rShadowAccumulation, xmm5
		movups	vShadowOrgPos, xmm2
			sub	eax, (10 SHL 23)
		movss	rmhpTemp.rRangeMax, xmm1
		mov	ecx, nRayCount
		add	edi, (SIZEOF E3D_GPU_RAY_HIT_ENTRY)
		dec	ecx
			mov	rmhpTemp.rRangeMin, eax
		mov	nRayCount, ecx
	.UNTIL	ZERO? || !([ebx].rrtpRayParam.dwFlags & E3D_RAYTRACE_SHADOW_ALPHA)
	.ENDIF
	;
	mov	ecx, nRayCount
SubFunc_FillDummy:
	test	ecx, ecx
	.WHILE	!ZERO?
		mov	[edi].iPolyEntry, -1
		mov	[edi].ppeMesh, NULL
		add	edi, (SIZEOF E3D_GPU_RAY_HIT_ENTRY)
		dec	ecx
	.ENDW
	;
	ASSUME	edi:NOTHING
	ASSUME	ebx:NOTHING
	;
	BYTE	0C3H	; ret

ALIGN	10H
SubFunc_SetRayHitResult:
	;
	;	光線追跡の結果を構造体に設定
	; --------------------------------------------------------------------
	ASSUME	edi:PTR E3D_GPU_RAY_HIT_ENTRY
	ASSUME	esi:PTR E3D_GPU_RAY_VECTOR_ENTRY
	;
	movlps	xmm0, QWORD PTR [esi].vRay[0]
	movss	xmm1, REAL4 PTR [esi].vRay[8]
	movss	xmm4, rmhpTemp.rRangeMax
	;
	mov	esi, [edi].ppeMesh
	ASSUME	esi:PE3D_POLYGON_ENTRY
	;
	movlps	QWORD PTR [edi].vRay[0], xmm0
	movss	REAL4 PTR [edi].vRay[8], xmm1
	movss	[edi].rDistance, xmm4
	;
	mov	esi, [esi].surface.mesh.pMeshReserved
	mov	eax, [edi].iPolyEntry
	;
	movups	xmm2, rmhpTemp.vHitPosition
		mov	ecx, eax
		and	eax, 03H
		shr	ecx, 2
	movhlps	xmm3, xmm2
		imul	ecx, (SIZEOF EGL_RENDER_POLY_MATRIX_PCK4)
		lea	esi, [esi + eax * 4]
	movlps	QWORD PTR [edi].vHitPos[0], xmm2
	movss	REAL4 PTR [edi].vHitPos[8], xmm3
	;
	add	esi, ecx
	ASSUME	esi:PTR EGL_RENDER_POLY_MATRIX_PCK4
	;
	; A = V1 - V0, B = V2 - V0, C = A * B
	; X = vHitPos
	; P = X - V0
	;
	movaps	xmm7, xmm2		; xmm7 = vHitPos
	movss	xmm6, [esi].xVertexA	; xmm6 = V0
	movss	xmm0, [esi].yVertexA
	movss	xmm1, [esi].zVertexA
	movss	xmm4, [esi].xVertexB	; xmm4 = A = V1 - V0
	movss	xmm2, [esi].yVertexB
	movss	xmm3, [esi].zVertexB
		unpcklps	xmm6, xmm0
		unpcklps	xmm4, xmm2
		movlhps		xmm6, xmm1
	movss	xmm5, [esi].xVertexC	; xmm5 = B = V2 - V0
		movlhps		xmm4, xmm3
		movss	xmm0, [esi].yVertexC
		movss	xmm1, [esi].zVertexC
		unpcklps	xmm5, xmm0
		movlhps		xmm5, xmm1
	subps	xmm7, xmm6		; xmm7 = X - V0
	subps	xmm4, xmm6
	subps	xmm5, xmm6
	;
	movaps	xmm2, xmm4		; xmm0 = C = A * B
	movaps	xmm3, xmm5
	movaps	xmm0, xmm4
	movaps	xmm1, xmm5
	shufps	xmm2, xmm2, 11010010B
	shufps	xmm3, xmm3, 11001001B
	shufps	xmm0, xmm0, 11001001B
	shufps	xmm1, xmm1, 11010010B
	mulps	xmm2, xmm3
	mulps	xmm0, xmm1
	subps	xmm0, xmm2
	;
	movaps	xmm1, xmm0		; xmm1 = |C|^2
	mulps	xmm1, xmm1
	movhlps	xmm3, xmm1
	addss	xmm3, xmm1
	shufps	xmm1, xmm1, 1
	addss	xmm1, xmm3
		;
	rcpss	xmm3, xmm1		; xmm1 = |C|^-2
	mulss	xmm1, xmm3
	rcpss	xmm1, xmm1
	mulss	xmm1, xmm3
	;
	; u = |P * B| / |C|
	; v = |A * P| / |C|
	;
	movaps	xmm0, xmm4		; xmm0 = A
		movaps	xmm2, xmm5	; xmm2 = B
	;
	shufps	xmm4, xmm4, 11010010B
		shufps	xmm5, xmm5, 11001001B
	shufps	xmm0, xmm0, 11001001B
	movaps	xmm6, xmm7
		shufps	xmm2, xmm2, 11010010B
	shufps	xmm7, xmm7, 11001001B
	shufps	xmm6, xmm6, 11010010B
	;
	mulps	xmm4, xmm7
		mulps	xmm5, xmm6
	mulps	xmm0, xmm6
		mulps	xmm2, xmm7
	subps	xmm0, xmm4		; xmm0 = A * P
		subps	xmm2, xmm5	; xmm2 = P * B
	;
	mulps	xmm0, xmm0
		mulps	xmm2, xmm2
	movhlps	xmm4, xmm0
		movhlps	xmm5, xmm2
	addss	xmm4, xmm0
	shufps	xmm0, xmm0, 1
		addss	xmm5, xmm2
		shufps	xmm2, xmm2, 1
	addss	xmm0, xmm4		; xmm0 = |A * P|^2
		addss	xmm2, xmm5	; xmm2 = |P * B|^2
	mulss	xmm0, xmm1
		mulss	xmm2, xmm1
	sqrtss	xmm0, xmm0		; xmm0 = v
		sqrtss	xmm2, xmm2	; xmm2 = u
	;
	movss	[edi].vUV.y, xmm0
	movss	[edi].vUV.x, xmm2
	;
	ASSUME	edi:NOTHING
	ASSUME	esi:NOTHING
	;
	BYTE	0C3H	; ret


eglRenderPoly@PerformTraceRaySSE	ENDP

ALIGN	10H
eglRenderPoly@BeginTraceRaysSSE		PROC	NEAR32 C USES ebx esi edi,
	hRenderPoly:HEGL_RENDER_POLYGON,
	pRayHitResults:PTR E3D_GPU_RAY_HIT_ENTRY,
	pRays:PTR E3D_GPU_RAY_VECTOR_ENTRY, nRayCount:DWORD

	LOCAL	nRayHitEntryBytes:DWORD

	mov	ebx, hRenderPoly
	mov	edi, pRayHitResults
	mov	esi, pRays
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	;
	; 処理結果エントリバイト数計算
	;
	mov	edx, [ebx].nVectorLightCount
	mov	ecx, [ebx].rrtpRayParam.dwRayReflectCount
	add	edx, [ebx].nPointLightCount
	;
	E3D_GPU_RAY_HIT_ENTRY@CountOfBlock	edx, ecx
	;
	imul	eax, (SIZEOF E3D_GPU_RAY_HIT_ENTRY)
	mov	nRayHitEntryBytes, eax
	;
	; 処理ループ
	;
	mov	ecx, nRayCount
	test	ecx, ecx
	.WHILE	!ZERO?
		mov	nRayCount, ecx
		;
		INVOKE	eglRenderPoly@PerformTraceRaySSE ,
			hRenderPoly, edi, esi, NULL, -1,
			[ebx].rrtpRayParam.dwRayReflectCount
		;
		mov	(E3D_GPU_RAY_HIT_ENTRY PTR [edi]).pParent, NULL
		;
		mov	ecx, nRayCount
		add	edi, nRayHitEntryBytes
		add	esi, (SIZEOF E3D_GPU_RAY_VECTOR_ENTRY)
		dec	ecx
	.ENDW
	;
	ASSUME	ebx:NOTHING
	emms
	xor	eax, eax
	ret

eglRenderPoly@BeginTraceRaysSSE		ENDP


;
;	GPU プラグイン光線追跡処理開始
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@BeginTraceRaysRect	PROC	NEAR32 C USES ebx esi edi,
	hRenderPoly:HEGL_RENDER_POLYGON,
	pRayHitResults:PTR E3D_GPU_RAY_HIT_ENTRY, pRect:PCEGL_IMAGE_RECT

	mov	ebx, hRenderPoly
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	mov	eax, eslErrGeneral

	mov	esi, [ebx].pGPUPluginInterface
	ASSUME	esi:PE3D_GPU_PLUGIN_INTERFACE
	.IF	(esi != NULL) && ([ebx].hGPUBufRayTracing != NULL)
		INVOKE	[esi].pfnBeginTraceRaysRect ,
			[ebx].hGPUBufRayTracing,
			pRayHitResults, pRect,
			ADDR [ebx].vScreenPos,
			[ebx].rrtpRayParam.dwRayReflectCount,
			[ebx].nAppendRayTracingCount
	.ENDIF
	ASSUME	esi:NOTHING

	ASSUME	ebx:NOTHING
	ret

eglRenderPoly@BeginTraceRaysRect	ENDP

ALIGN	10H
eglRenderPoly@BeginTraceRaysRectSSE	PROC	NEAR32 C USES ebx esi edi,
	hRenderPoly:HEGL_RENDER_POLYGON,
	pRayHitResults:PTR E3D_GPU_RAY_HIT_ENTRY,
	pRect:PCEGL_IMAGE_RECT

	LOCAL	nRayHitEntryBytes:DWORD
	LOCAL	rctTraceRays:EGL_IMAGE_RECT
	LOCAL	yPos:DWORD
	LOCAL	xPos:DWORD
	LOCAL	rvRayPos:E3D_GPU_RAY_VECTOR_ENTRY

	mov	ebx, hRenderPoly
	mov	esi, pRect
	mov	edi, pRayHitResults
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	ASSUME	esi:PCEGL_IMAGE_RECT
	;
	; 矩形取得
	;
	mov	eax, [esi].x
	mov	edx, [esi].y
	mov	rctTraceRays.x, eax
	mov	rctTraceRays.y, edx
	mov	eax, [esi].w
	mov	edx, [esi].h
	mov	rctTraceRays.w, eax
	mov	rctTraceRays.h, edx
	ASSUME	esi:NOTHING
	;
	; 処理結果エントリバイト数計算
	;
	mov	edx, [ebx].nVectorLightCount
	mov	ecx, [ebx].rrtpRayParam.dwRayReflectCount
	add	edx, [ebx].nPointLightCount
	;
	E3D_GPU_RAY_HIT_ENTRY@CountOfBlock	edx, ecx
	;
	imul	eax, (SIZEOF E3D_GPU_RAY_HIT_ENTRY)
	mov	nRayHitEntryBytes, eax
	;
	; 処理ループ
	;
	xor	ecx, ecx
	.WHILE	(SDWORD PTR ecx) < rctTraceRays.h
		mov	yPos, ecx
		;
		xor	ecx, ecx
		.WHILE	(SDWORD PTR ecx) < rctTraceRays.w
			;
			; 光線を設定
			;
			mov	xPos, ecx
			xorps	xmm7, xmm7
			mov	eax, ecx
			mov	edx, yPos
			add	eax, rctTraceRays.x
			add	edx, rctTraceRays.y
			cvtsi2ss	xmm0, eax
			cvtsi2ss	xmm1, edx
			subss	xmm0, [ebx].vScreenPos.x
			subss	xmm1, [ebx].vScreenPos.y
			movss	xmm2, [ebx].vScreenPos.z
			unpcklps	xmm0, xmm1
			movlhps	xmm0, xmm2
			;
			movaps	xmm1, xmm0
			mulps	xmm0, xmm0
			;
			movhlps	xmm2, xmm0
			movss	xmm3, xmm0
			shufps	xmm0, xmm0, 1
			addss	xmm2, xmm3
			addss	xmm0, xmm2
			sqrtss	xmm0, xmm0
			shufps	xmm0, xmm0, 0
			divps	xmm1, xmm0
			;
			movups	rvRayPos.vPos, xmm7
			movups	rvRayPos.vRay, xmm1
			;
			; 光線を追跡
			;
			INVOKE	eglRenderPoly@PerformTraceRaySSE ,
				hRenderPoly, edi, ADDR rvRayPos, NULL, -1,
				[ebx].rrtpRayParam.dwRayReflectCount
			;
			mov	(E3D_GPU_RAY_HIT_ENTRY PTR [edi]).pParent, NULL
			;
			mov	ecx, xPos
			add	edi, nRayHitEntryBytes
			inc	ecx
		.ENDW
		mov	ecx, yPos
		inc	ecx
	.ENDW
	;
	ASSUME	ebx:NOTHING
	emms
	xor	eax, eax
	ret

eglRenderPoly@BeginTraceRaysRectSSE	ENDP


;
;	GPU プラグインの光線追跡処理結果からシェーディングを行う
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@ShadeByRayTrace486	PROC	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON,
	pShadeColor:PTR E3D_COLOR, pRayHit:PTR E3D_GPU_RAY_HIT_ENTRY

	mov	eax, eslErrGeneral
	ret

eglRenderPoly@ShadeByRayTrace486	ENDP


;
;	GPU プラグインの光線追跡処理結果からシェーディングを行う
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@ShadeByRayTraceSSE	PROC	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON,
	pShadeColor:PTR E3D_COLOR, pRayHit:PTR E3D_GPU_RAY_HIT_ENTRY

	LOCAL	rhRoot:E3D_GPU_RAY_HIT_ENTRY

	mov	eax, pRayHit
	mov	ecx, hRenderPoly
	ASSUME	ecx:PTR EGL_RENDER_POLYGON_BUF
	ASSUME	eax:PTR E3D_GPU_RAY_HIT_ENTRY
;	.IF	[eax].pParent == NULL
		lea	edx, rhRoot
		mov	[eax].pParent, edx
		mov	rhRoot.vHitPos.x, 0
		mov	rhRoot.vHitPos.y, 0
		mov	rhRoot.vHitPos.z, 0
;	.ENDIF
	ASSUME	eax:NOTHING

	INVOKE	eglRenderPoly@ShadeByGPURayTraceSSE ,
			ecx, pShadeColor, pRayHit,
			[ecx].rrtpRayParam.dwRayReflectCount,
			[ecx].nAppendRayTracingCount
	emms

	ASSUME	ecx:NOTHING
	ret

eglRenderPoly@ShadeByRayTraceSSE	ENDP


;
;	光線追跡エントリの１ピクセルあたりのエントリ数を計算する
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@CountOfRayHitEntry	PROC	NEAR32 C USES ebx,
	hRenderPoly:HEGL_RENDER_POLYGON

	mov	ebx, hRenderPoly
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF

	mov	edx, [ebx].nVectorLightCount
	mov	ecx, [ebx].rrtpRayParam.dwRayReflectCount
	add	edx, [ebx].nPointLightCount
	;
	E3D_GPU_RAY_HIT_ENTRY@CountOfBlock	edx, ecx
	;
	add	eax, [ebx].nAppendRayTracingCount
	ASSUME	ebx:NOTHING
	ret

eglRenderPoly@CountOfRayHitEntry	ENDP

ENDIF


CodeSeg	ENDS

	END
