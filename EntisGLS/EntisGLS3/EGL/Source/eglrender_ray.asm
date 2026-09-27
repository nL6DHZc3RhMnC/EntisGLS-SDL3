
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
xmmSignBit		DWORD	4 DUP( 80000000H )
xmmHalf3_Zero		REAL4	0.5, 0.5, 0.5, 0.0
			REAL4	0.5005, 0.5005, 0.5005, 0.0
xmmConst1		REAL4	4 DUP( 1.0 )
xmmConst2		REAL4	4 DUP( 2.0 )
xmmConst4		REAL4	4 DUP( 4.0 )
;xmmErrorScale		REAL4	4 DUP( 0.0001220703125 ); 1 / 8192
xmmErrorScale		REAL4	4 DUP( 0.0002 )
;xmmErrorScale		REAL4	4 DUP( 0.00085 )
xmmErrorDivCosRange	REAL4	4 DUP( 8.0 )
xmmCosErrorScale	REAL4	4 DUP( 0.9990234375 )	; 1 - 1 / 1024
;xmmPolySizeErrorScale	REAL4	4 DUP( 1.0009765625 )	; 1 + 1 / 1024
xmmPolySizeErrorScale	REAL4	4 DUP( 1.125 )

ConstSeg	ENDS


; ----------------------------------------------------------------------------
;	コードセグメント
; ----------------------------------------------------------------------------

CodeSeg	SEGMENT	PARA READONLY FLAT 'CODE'

;
;	レイトレーシングパラメータ設定（ダミー）
; ----------------------------------------------------------------------------
eglRenderPoly@SetRayTracingParameter486	PROC	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON,
	prrtpParam:PTR EGL_RENDER_RAY_TRACE_PARAM

	mov	eax, eslErrGeneral
	ret

eglRenderPoly@SetRayTracingParameter486	ENDP

;
;	レイトレーシングターゲット設定（ダミー）
; ----------------------------------------------------------------------------
eglRenderPoly@AttachRayTracingTarget486	PROC	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON, hStackHeap:HSTACKHEAP,
	ppShadowingPoly:PTR PE3D_POLYGON_ENTRY, nShadowingCount:DWORD,
	ppRayTracingPoly:PTR PE3D_POLYGON_ENTRY, nRayTracingCount:DWORD,
	ppGlobalReflection:PTR PE3D_POLYGON_ENTRY, nGlobalReflections:DWORD

	mov	eax, eslErrGeneral
	ret

eglRenderPoly@AttachRayTracingTarget486	ENDP

;
;	レイトレーシングポリゴンエントリ（ダミー）
; ----------------------------------------------------------------------------
eglRenderPoly@CreatePolygonEntryRT_486	PROC	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON, hStackHeap:HSTACKHEAP,
	pPrimitive:PCE3D_PRIMITIVE_POLYGON, pdwResult:PTR DWORD

	INVOKE	eglRenderPoly@CretaePolygonEntry486,
			hRenderPoly, hStackHeap, pPrimitive
	mov	edx, pdwResult
	mov	DWORD PTR [edx], E3D_RTCPE_RESULT_NO_RAYTRACING
	ret

eglRenderPoly@CreatePolygonEntryRT_486	ENDP


;
;	レイトレーシングパラメータ設定 SSE コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@SetRayTracingParameterSSE	PROC	NEAR32 C USES ebx esi,
	hRenderPoly:HEGL_RENDER_POLYGON,
	prrtpParam:PTR EGL_RENDER_RAY_TRACE_PARAM

	mov	ebx, hRenderPoly
	mov	esi, prrtpParam
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	ASSUME	esi:PTR EGL_RENDER_RAY_TRACE_PARAM

	mov	eax, [esi].dwFlags
	mov	edx, [esi].rShadowingDistance
	or	eax, E3D_RAYTRACE_SHADOWING_APPEND
	mov	[ebx].rrtpRayParam.dwFlags, eax
	mov	[ebx].rrtpRayParam.rShadowingDistance, edx
	mov	eax, [esi].rRayTracingDistance
	mov	edx, [esi].dwRayReflectCount
	mov	[ebx].rrtpRayParam.rRayTracingDistance, eax
	mov	[ebx].rrtpRayParam.dwRayReflectCount, edx
	mov	eax, [esi].dwAppendShadowingCount
	xor	edx, edx
	test	[esi].dwFlags, E3D_RAYTRACE_SHADOWING_APPEND
	cmovz	eax, edx
	mov	[ebx].rrtpRayParam.dwAppendShadowingCount, eax

	mov	ecx, 1
	cvtsi2ss	xmm0, ecx
	divss	xmm0, [ebx].rrtpRayParam.rShadowingDistance
	movss	[ebx].rRcpShadowDistance, xmm0

	INVOKE	eglRenderPoly@SetFunctionFlags, ebx, [ebx].dwFunctionFlags

	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglRenderPoly@SetRayTracingParameterSSE	ENDP


;
;	当たり判定用バッファ生成 SSE コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@PrepareRayTracingMeshParamSSE	PROC	NEAR32 C USES ebx esi edi,
	hStackHeap:HSTACKHEAP, pPolyEntry:PE3D_POLYGON_ENTRY

	LOCAL	nPolyCount:DWORD
	LOCAL	pMeshList:PTR E3D_PRIMITIVE_MESH_LIST
	LOCAL	pVertices:PE3D_VECTOR4
	LOCAL	pMeshMatrix:PTR EGL_RENDER_POLY_MATRIX_PCK4
	LOCAL	pMeshSphare:PTR EGL_RENDER_POLY_SPHERE_PCK4

	;
	; パラメータ取得
	;
	mov	edi, pPolyEntry
	ASSUME	edi:PE3D_POLYGON_ENTRY
	;
	mov	esi, [edi].surface.mesh.pMesh
	ASSUME	esi:PE3D_PRIMITIVE_MESH_LIST
	mov	eax, [esi].dwPolyCount
	mov	pMeshList, esi
	mov	nPolyCount, eax
	;
	;	頂点順補正
	; --------------------------------------------------------------------
	mov	ecx, eax
	lea	esi, [esi].mpEntries[0]
	ASSUME	esi:PTR E3D_PRIMITIVE_MESH_POLY
	mov	ebx, [edi].pVertexes
	sub	ecx, 2
	.WHILE	!SIGN?
		mov	eax, [esi].dwIndex[0]
		mov	edx, [esi].dwIndex[4]
		mov	edi, [esi].dwIndex[8]
		shl	eax, 4
		shl	edx, 4
		shl	edi, 4
		movaps	xmm0, XMMWORD_PTR [ebx + eax]
			mov	eax, [esi].dwIndex[10H]
		movaps	xmm1, XMMWORD_PTR [ebx + edx]
			mov	edx, [esi].dwIndex[14H]
		movaps	xmm2, XMMWORD_PTR [ebx + edi]
			mov	edi, [esi].dwIndex[18H]
		movaps	xmm3, xmm0
		subps	xmm0, xmm1
			shl	eax, 4
		subps	xmm1, xmm2
			shl	edx, 4
		subps	xmm2, xmm3
			shl	edi, 4
		mulps	xmm0, xmm0		; xmm0 = |A - B|^2
		mulps	xmm1, xmm1		; xmm1 = |B - C|^2
			movaps	xmm4, XMMWORD_PTR [ebx + eax]
		mulps	xmm2, xmm2		; xmm2 = |A - C|^2
			movaps	xmm5, XMMWORD_PTR [ebx + edx]
		movhlps	xmm3, xmm0
			movaps	xmm6, XMMWORD_PTR [ebx + edi]
		movhlps	xmm7, xmm1
		addss	xmm3, xmm0
		shufps	xmm0, xmm0, 1
		addss	xmm7, xmm1
		shufps	xmm1, xmm1, 1
		addss	xmm0, xmm3
			movaps	xmm3, xmm4
		addss	xmm1, xmm7
		movhlps	xmm7, xmm2
			subps	xmm4, xmm5	; xmm4 = |A - B|^2
			subps	xmm5, xmm6	; xmm5 = |B - C|^2
		addss	xmm7, xmm2
		shufps	xmm2, xmm2, 1
			subps	xmm6, xmm3	; xmm6 = |A - C|^2
			mulps	xmm4, xmm4
		addss	xmm2, xmm7
			mulps	xmm5, xmm5
			mulps	xmm6, xmm6
		;
			movhlps	xmm3, xmm4
		minss	xmm0, xmm2		; xmm0 = min( AB, AC )
			movhlps	xmm7, xmm5
			addss	xmm3, xmm4
			shufps	xmm4, xmm4, 1
			addss	xmm7, xmm5
			shufps	xmm5, xmm5, 1
			addss	xmm4, xmm3
			movhlps	xmm3, xmm6
			addss	xmm5, xmm7
			addss	xmm3, xmm6
			shufps	xmm6, xmm6, 1
			addss	xmm6, xmm3
		comiss	xmm0, xmm1
			minss	xmm4, xmm6	; min( AB, AC )
		;
		.IF	!CARRY?		; min( AB, AC ) >= BC
			mov	eax, [esi].dwIndex[0]
			mov	edx, [esi].dwIndex[4]
			mov	edi, [esi].dwIndex[8]
			mov	[esi].dwIndex[0], edx
			mov	[esi].dwIndex[4], edi
			mov	[esi].dwIndex[8], eax
		.ENDIF
		;
		comiss	xmm4, xmm5
		.IF	!CARRY?		; min( AB, AC ) >= BC
			mov	eax, [esi].dwIndex[10H]
			mov	edx, [esi].dwIndex[14H]
			mov	edi, [esi].dwIndex[18H]
			mov	[esi].dwIndex[10H], edx
			mov	[esi].dwIndex[14H], edi
			mov	[esi].dwIndex[18H], eax
		.ENDIF
		;
		add	esi, (4 * 4) * 2
		sub	ecx, 2
	.ENDW
	add	ecx, 2
	.IF	!ZERO?
		mov	eax, [esi].dwIndex[0]
		mov	edx, [esi].dwIndex[4]
		mov	edi, [esi].dwIndex[8]
		shl	eax, 4
		shl	edx, 4
		shl	edi, 4
		movaps	xmm0, XMMWORD_PTR [ebx + eax]
		movaps	xmm1, XMMWORD_PTR [ebx + edx]
		movaps	xmm2, XMMWORD_PTR [ebx + edi]
		movaps	xmm3, xmm0
		subps	xmm0, xmm1
		subps	xmm1, xmm2
		subps	xmm2, xmm3
		mulps	xmm0, xmm0		; xmm0 = |A - B|^2
		mulps	xmm1, xmm1		; xmm1 = |B - C|^2
		mulps	xmm2, xmm2		; xmm2 = |A - C|^2
		movhlps	xmm3, xmm0
		movhlps	xmm7, xmm1
		addss	xmm3, xmm0
		shufps	xmm0, xmm0, 1
		addss	xmm7, xmm1
		shufps	xmm1, xmm1, 1
		addss	xmm0, xmm3
		addss	xmm1, xmm7
		movhlps	xmm7, xmm2
		addss	xmm7, xmm2
		shufps	xmm2, xmm2, 1
		addss	xmm2, xmm7
		;
		minss	xmm0, xmm2		; xmm0 = min( AB, AC )
		comiss	xmm0, xmm1
		;
		.IF	!CARRY?		; min( AB, AC ) >= BC
			mov	eax, [esi].dwIndex[0]
			mov	edx, [esi].dwIndex[4]
			mov	edi, [esi].dwIndex[8]
			mov	[esi].dwIndex[0], edx
			mov	[esi].dwIndex[4], edi
			mov	[esi].dwIndex[8], eax
		.ENDIF
	.ENDIF
	mov	edi, pPolyEntry
	;
	;	パラメータ計算
	; --------------------------------------------------------------------
	;
	; メモリ確保
	;
	mov	eax, nPolyCount
	add	eax, 03H
	shr	eax, 2
	imul	eax, (SIZEOF EGL_RENDER_POLY_MATRIX_PCK4)
	;
	INVOKE	eslStackHeapAllocate , hStackHeap, eax
	;
	mov	esi, [edi].surface.mesh.pMesh
	ASSUME	esi:PE3D_PRIMITIVE_MESH_LIST
	mov	pMeshMatrix, eax
	mov	[edi].surface.mesh.pMeshReserved, eax
	mov	ebx, [edi].pVertexes
	mov	ecx, nPolyCount
	lea	esi, [esi].mpEntries[0]
	mov	edi, eax
	mov	pVertices, ebx
	ASSUME	ebx:PE3D_VECTOR4
	ASSUME	esi:PTR E3D_PRIMITIVE_MESH_POLY
	ASSUME	edi:PTR EGL_RENDER_POLY_MATRIX_PCK4
	;
	sub	ecx, 4
	.WHILE	!SIGN?
		;
		; 基準頂点 A
		;
		mov	eax, [esi].dwIndex[0]
		mov	edx, [esi].dwIndex[20H]
		shl	eax, 4
		shl	edx, 4
		movlps	xmm0, QWORD PTR [ebx + eax].x	; xmm0 = y1 x1 y0 x0
		movlps	xmm1, QWORD PTR [ebx + edx].x	; xmm1 = y3 x3 y2 x2
		movss	xmm4, [ebx + eax].z		; xmm4 = -- z1 -- z0
		movss	xmm5, [ebx + edx].z		; xmm5 = -- z3 -- z2
		mov	eax, [esi].dwIndex[10H]
		mov	edx, [esi].dwIndex[30H]
		shl	eax, 4
		shl	edx, 4
		movhps	xmm0, QWORD PTR [ebx + eax].x
		movhps	xmm1, QWORD PTR [ebx + edx].x
		movhps	xmm4, QWORD PTR [ebx + eax].z
		movhps	xmm5, QWORD PTR [ebx + edx].z
		;
		movaps	xmm2, xmm0
		shufps	xmm0, xmm1, 10001000B		; xmm0 = x3 x2 x1 x0
		shufps	xmm2, xmm1, 11011101B		; xmm2 = y3 y2 y1 y0
		shufps	xmm4, xmm5, 10001000B		; xmm4 = z3 z2 z1 z0
		;
		; 頂点 B
		;
		mov	eax, [esi].dwIndex[4]
		mov	edx, [esi].dwIndex[24H]
			movaps	[edi].xVertexA, xmm0
		shl	eax, 4
			movaps	[edi].yVertexA, xmm2
		shl	edx, 4
			movaps	[edi].zVertexA, xmm4
		movlps	xmm0, QWORD PTR [ebx + eax].x	; xmm0 = y1 x1 y0 x0
		movlps	xmm1, QWORD PTR [ebx + edx].x	; xmm1 = y3 x3 y2 x2
		movss	xmm4, [ebx + eax].z		; xmm4 = -- z1 -- z0
		movss	xmm5, [ebx + edx].z		; xmm5 = -- z3 -- z2
		mov	eax, [esi].dwIndex[14H]
		mov	edx, [esi].dwIndex[34H]
		shl	eax, 4
		shl	edx, 4
		movhps	xmm0, QWORD PTR [ebx + eax].x
		movhps	xmm1, QWORD PTR [ebx + edx].x
		movhps	xmm4, QWORD PTR [ebx + eax].z
		movhps	xmm5, QWORD PTR [ebx + edx].z
		;
		movaps	xmm2, xmm0
		shufps	xmm0, xmm1, 10001000B		; xmm0 = x3 x2 x1 x0
		shufps	xmm2, xmm1, 11011101B		; xmm2 = y3 y2 y1 y0
		shufps	xmm4, xmm5, 10001000B		; xmm4 = z3 z2 z1 z0
		;
		; 頂点 C
		;
		mov	eax, [esi].dwIndex[8]
		mov	edx, [esi].dwIndex[28H]
			movaps	[edi].xVertexB, xmm0
		shl	eax, 4
			movaps	[edi].yVertexB, xmm2
		shl	edx, 4
			movaps	[edi].zVertexB, xmm4
		movlps	xmm0, QWORD PTR [ebx + eax].x	; xmm0 = y1 x1 y0 x0
		movlps	xmm1, QWORD PTR [ebx + edx].x	; xmm1 = y3 x3 y2 x2
		movss	xmm4, [ebx + eax].z		; xmm4 = -- z1 -- z0
		movss	xmm5, [ebx + edx].z		; xmm5 = -- z3 -- z2
		mov	eax, [esi].dwIndex[18H]
		mov	edx, [esi].dwIndex[38H]
		shl	eax, 4
		shl	edx, 4
		movhps	xmm0, QWORD PTR [ebx + eax].x
		movhps	xmm1, QWORD PTR [ebx + edx].x
		movhps	xmm4, QWORD PTR [ebx + eax].z
		movhps	xmm5, QWORD PTR [ebx + edx].z
		;
		movaps	xmm2, xmm0
		shufps	xmm0, xmm1, 10001000B		; xmm0 = x3 x2 x1 x0
		shufps	xmm2, xmm1, 11011101B		; xmm2 = y3 y2 y1 y0
		shufps	xmm4, xmm5, 10001000B		; xmm4 = z3 z2 z1 z0
		;
		;
		; (A - C) / |A - C|
		;
		movaps	xmm1, [edi].xVertexA
			movaps	[edi].xVertexC, xmm0
		movaps	xmm3, [edi].yVertexA
			movaps	[edi].yVertexC, xmm2
		subps	xmm1, xmm0
		movaps	xmm5, [edi].zVertexA
			movaps	[edi].zVertexC, xmm4
		subps	xmm3, xmm2
		subps	xmm5, xmm4
		;
		movaps	xmm0, xmm1		; (xmm0, xmm2, xmm4) = A - C
			mulps	xmm1, xmm1
		movaps	xmm2, xmm3
			mulps	xmm3, xmm3
		movaps	xmm4, xmm5
			mulps	xmm5, xmm5
		;
		addps	xmm1, xmm3		; xmm7 = |A - C|
		addps	xmm1, xmm5
			movaps	[edi].max_ab_ac_sqr, xmm1
		sqrtps	xmm7, xmm1
		;
		; (A - B) / |A - B|
		;
		movaps	xmm1, [edi].xVertexA
		movaps	xmm3, [edi].yVertexA
		movaps	xmm5, [edi].zVertexA
		subps	xmm1, [edi].xVertexB
			divps	xmm0, xmm7	; (xmm0, xmm2, xmm4) / |A - C|
		subps	xmm3, [edi].yVertexB
			divps	xmm2, xmm7
		subps	xmm5, [edi].zVertexB
			divps	xmm4, xmm7
			movaps	[edi].xAC, xmm0
		;
		movaps	xmm0, xmm1		; (xmm0, xmm2, xmm4) = A - B
			mulps	xmm1, xmm1
			movaps	[edi].yAC, xmm2
		movaps	xmm2, xmm3
			mulps	xmm3, xmm3
			movaps	[edi].zAC, xmm4
		movaps	xmm4, xmm5
			mulps	xmm5, xmm5
		;
		addps	xmm1, xmm3		; xmm7 = |A - B|
		addps	xmm1, xmm5
			movaps	xmm3, [edi].max_ab_ac_sqr
		sqrtps	xmm7, xmm1
			maxps	xmm3, xmm1
		;
			mulps	xmm3, xmmPolySizeErrorScale
			movaps	[edi].max_ab_ac_sqr, xmm3
		;
		divps	xmm0, xmm7		; (xmm0, xmm2, xmm4) / |A - B|
		divps	xmm2, xmm7
		divps	xmm4, xmm7
		movaps	[edi].xAB, xmm0
		movaps	[edi].yAB, xmm2
		movaps	[edi].zAB, xmm4
		;
		; (A - B) * (A - C) / (|A - B| * |A - C|)
		;
		movaps	xmm3, xmm2
		movaps	xmm5, xmm4
		mulps	xmm2, [edi].zAC
		mulps	xmm4, [edi].yAC
			movaps	xmm1, xmm0
			mulps	xmm5, [edi].xAC
			mulps	xmm0, [edi].zAC
		subps	xmm2, xmm4
			subps	xmm5, xmm0
				mulps	xmm1, [edi].yAC
				mulps	xmm3, [edi].xAC
		movaps	[edi].xNormal, xmm2
			movaps	[edi].yNormal, xmm5
				subps	xmm1, xmm3
				movaps	[edi].zNormal, xmm1
		;
		; (B - C) / |B - C|
		;
		movaps	xmm1, [edi].xVertexB
		movaps	xmm3, [edi].yVertexB
		movaps	xmm5, [edi].zVertexB
		subps	xmm1, [edi].xVertexC
		subps	xmm3, [edi].yVertexC
		subps	xmm5, [edi].zVertexC
		;
		movaps	xmm0, xmm1		; (xmm0, xmm2, xmm4) = B - C
			mulps	xmm1, xmm1
		movaps	xmm2, xmm3
			mulps	xmm3, xmm3
		movaps	xmm4, xmm5
			mulps	xmm5, xmm5
		;
		addps	xmm1, xmm3		; xmm1 = |B - C|
			movaps	xmm3, [edi].xVertexA
		addps	xmm1, xmm5
			movaps	xmm5, [edi].yVertexA
		sqrtps	xmm1, xmm1
			movaps	xmm7, [edi].zVertexA
			mulps	xmm3, xmm3
			mulps	xmm5, xmm5
		;
			mulps	xmm7, xmm7
			addps	xmm3, xmm5
				movaps	xmm5, xmmConst2
		divps	xmm0, xmm1		; (xmm0, xmm2, xmm4) / |B - C|
			addps	xmm7, xmm3
				mulps	xmm5, xmm1	; xmm5 = |B - C| * 2
		divps	xmm2, xmm1
			sqrtps	xmm7, xmm7		; xmm7 = |A|
		;
		divps	xmm4, xmm1
			maxps	xmm7, xmm5
		movaps	[edi].xBC, xmm0
			mulps	xmm7, xmmErrorScale
		movaps	[edi].yBC, xmm2
		movaps	[edi].zBC, xmm4
			movaps	[edi].error_gap, xmm7	; = max(|A|,|B - C|*2) * xmmErrorScale
		;
		; cos (∠ABC) = <A - B, C - B> / (|A - B| * |C - B|)
		; cos (∠ACB) = <A - C, B - C> / (|A - C| * |B - C|)
		;
		movaps	xmm0, [edi].xAB
		movaps	xmm1, [edi].yAB
		movaps	xmm2, [edi].zAB
			movaps	xmm4, [edi].xBC
			movaps	xmm5, [edi].yBC
			movaps	xmm6, [edi].zBC
		mulps	xmm0, xmm4
			mulps	xmm4, [edi].xAC
		mulps	xmm1, xmm5
			mulps	xmm5, [edi].yAC
		mulps	xmm2, xmm6
			mulps	xmm6, [edi].zAC
		addps	xmm0, xmm1
			addps	xmm4, xmm5
		addps	xmm0, xmm2
			addps	xmm4, xmm6
;		movaps	xmm1, xmmCosErrorScale
		xorps	xmm0, xmmSignBit
;		mulps	xmm0, xmm1
;			mulps	xmm4, xmm1
			movaps	[edi].cos_acb, xmm4
		movaps	[edi].cos_abc, xmm0
			maxps	xmm0, xmm4
			movaps	xmm1, xmmConst1
			mulps	xmm0, xmm0
			subps	xmm1, xmm0
			sqrtps	xmm1, xmm1
			mulps	xmm1, xmmErrorDivCosRange
			minps	xmm1, xmmConst1
			mulps	xmm1, xmm7
			movaps	[edi].error_gap, xmm1
		;
		add	esi, (4 * 4) * 4
		add	edi, (SIZEOF EGL_RENDER_POLY_MATRIX_PCK4)
		sub	ecx, 4
	.ENDW
	add	ecx, 4
	.IF	!ZERO?
		xorps	xmm0, xmm0
		mov	eax, edi
		mov	edx, (SIZEOF EGL_RENDER_POLY_MATRIX_PCK4) / 10H
		.REPEAT
			movaps	[eax], xmm0
			add	eax, 10H
			dec	edx
		.UNTIL	ZERO?
		;
		mov	[edi].error_gap[0], 0CF800000H	; -2^32
		mov	[edi].error_gap[4], 0CF800000H	; -2^32
		mov	[edi].error_gap[8], 0CF800000H	; -2^32
		mov	[edi].error_gap[12], 0CF800000H	; -2^32
		;
		test	ecx, ecx
		.WHILE	!ZERO?
			;
			; 頂点 A, B, C
			;
			mov	eax, [esi].dwIndex[0]
			mov	edx, [esi].dwIndex[4]
			shl	eax, 4
			shl	edx, 4
			movaps	xmm0, [ebx + eax]
			mov	eax, [esi].dwIndex[8]
			shl	eax, 4
			movaps	xmm1, [ebx + edx]
			movaps	xmm2, [ebx + eax]
			;
			; xmm4 = (A - B), xmm5 = (A - C), xmm6 = (B - C)
			;
			movaps	xmm4, xmm0
			movaps	xmm5, xmm0
				movss	[edi].xVertexA, xmm0
				shufps	xmm0, xmm0, 00111001B
			movaps	xmm6, xmm1
				movss	[edi].xVertexB, xmm1
			subps	xmm4, xmm1
				shufps	xmm1, xmm1, 00111001B
				movss	[edi].xVertexC, xmm2
			subps	xmm5, xmm2
			subps	xmm6, xmm2
				shufps	xmm2, xmm2, 00111001B
				movss	[edi].yVertexA, xmm0
				shufps	xmm0, xmm0, 00111001B
				movss	[edi].yVertexB, xmm1
				shufps	xmm1, xmm1, 00111001B
				movss	[edi].yVertexC, xmm2
				shufps	xmm2, xmm2, 00111001B
				movss	[edi].zVertexA, xmm0
				movss	[edi].zVertexB, xmm1
				movss	[edi].zVertexC, xmm2
			;
			movaps	xmm0, xmm4
				mulps	xmm4, xmm4
			movaps	xmm1, xmm5
				mulps	xmm5, xmm5
			movaps	xmm2, xmm6
				mulps	xmm6, xmm6
			;
			movhlps	xmm3, xmm4
				movhlps	xmm7, xmm5
			addss	xmm3, xmm4
				addss	xmm7, xmm5
			shufps	xmm4, xmm4, 1
				shufps	xmm5, xmm5, 1
			addss	xmm4, xmm3
				addss	xmm5, xmm7
			;
					movhlps	xmm3, xmm6
						movaps	xmm7, xmm4
			sqrtss	xmm4, xmm4
					addss	xmm3, xmm6
						maxss	xmm7, xmm5
				sqrtss	xmm5, xmm5
					shufps	xmm6, xmm6, 1
						mulss	xmm7, xmmPolySizeErrorScale
			shufps	xmm4, xmm4, 0
					addss	xmm6, xmm3
				shufps	xmm5, xmm5, 0
					sqrtss	xmm6, xmm6	; xmm6 = |B - C|
						movss	[edi].max_ab_ac_sqr, xmm7
						movss	xmm7, [edi].xVertexA
			divps	xmm0, xmm4
						movss	xmm4, [edi].yVertexA
					shufps	xmm6, xmm6, 0
						mulss	xmm7, xmm7
						mulss	xmm4, xmm4
				divps	xmm1, xmm5
						movss	xmm5, xmmConst2
						addss	xmm7, xmm4
						movss	xmm4, [edi].zVertexA
					divps	xmm2, xmm6
						mulss	xmm4, xmm4
						mulss	xmm5, xmm6	; xmm5 = |B - C| * 2
						addss	xmm7, xmm4
						sqrtss	xmm7, xmm7	; xmm7 = |A|
						maxss	xmm7, xmm5
						mulss	xmm7, xmmErrorScale
					movss	[edi].error_gap, xmm7	; = max(|A|,|B - C|*2) * xmmErrorScale
			;
			movaps	xmm4, xmm0
			movss	[edi].xAB, xmm0
			shufps	xmm0, xmm0, 00111001B
				movaps	xmm5, xmm1
				movss	[edi].xAC, xmm1
				shufps	xmm1, xmm1, 00111001B
					movss	[edi].xBC, xmm2
					shufps	xmm2, xmm2, 00111001B
			movss	[edi].yAB, xmm0
			shufps	xmm0, xmm0, 00111001B
				movss	[edi].yAC, xmm1
				shufps	xmm1, xmm1, 00111001B
					movss	[edi].yBC, xmm2
					shufps	xmm2, xmm2, 00111001B
			movss	[edi].zAB, xmm0
				movss	[edi].zAC, xmm1
					movss	[edi].zBC, xmm2
			;
			; (A - B) * (A - C) / (|A - B| * |A - C|)
			;
			movaps	xmm0, xmm4
			movaps	xmm1, xmm5
			shufps	xmm4, xmm4, 11001001B
			shufps	xmm5, xmm5, 11010010B
			shufps	xmm0, xmm0, 11010010B
			shufps	xmm1, xmm1, 11001001B
			mulps	xmm4, xmm5
			mulps	xmm0, xmm1
			subps	xmm4, xmm0
			;
			movhlps	xmm0, xmm4
			movss	[edi].xNormal, xmm4
			shufps	xmm4, xmm4, 1
			movss	[edi].zNormal, xmm0
			movss	[edi].yNormal, xmm4
			;
			; cos (∠ABC) = <A - B, C - B> / (|A - B| * |C - B|)
			; cos (∠ACB) = <A - C, B - C> / (|A - C| * |B - C|)
			;
			movss	xmm0, [edi].xAB
				movss	xmm4, [edi].xBC
			movss	xmm1, [edi].yAB
				movss	xmm5, [edi].yBC
			movss	xmm2, [edi].zAB
				movss	xmm6, [edi].zBC
			mulss	xmm0, xmm4
				mulss	xmm4, [edi].xAC
			mulss	xmm1, xmm5
				mulss	xmm5, [edi].yAC
			mulss	xmm2, xmm6
				mulss	xmm6, [edi].zAC
			movss	xmm3, xmmSignBit
			addss	xmm0, xmm1
				addss	xmm4, xmm5
					movss	xmm5, [edi].xVertexA
			addss	xmm0, xmm2
				addss	xmm4, xmm6
					movss	xmm6, [edi].yVertexA
					movss	xmm7, [edi].zVertexA
					mulss	xmm5, xmm5
;			movss	xmm1, xmmCosErrorScale
					mulss	xmm6, xmm6
			xorps	xmm0, xmm3
;				mulss	xmm4, xmm1
;			mulss	xmm0, xmm1
			;
					mulss	xmm7, xmm7
					addss	xmm5, xmm6
					addss	xmm5, xmm7
			movss	[edi].cos_acb, xmm4
			movss	[edi].cos_abc, xmm0
					sqrtss	xmm5, xmm5
					movss	xmm6, [edi].error_gap
				maxss	xmm0, xmm4
				movss	xmm1, xmmConst1
				mulss	xmm0, xmm0
				subss	xmm1, xmm0
				sqrtss	xmm1, xmm1
				mulss	xmm1, xmmErrorDivCosRange
				minss	xmm1, xmmConst1
				mulss	xmm6, xmm1
					movss	[edi].error_gap, xmm6
			;
			lea	esi, [esi].dwIndex[12]
			add	edi, 4
			dec	ecx
		.ENDW
	.ENDIF
	;
	;	ポリゴンブロック生成
	; --------------------------------------------------------------------
	;
	; メモリ確保
	;
	mov	eax, nPolyCount
	add	eax, 7FH
	shr	eax, 7
	imul	eax, (SIZEOF EGL_RENDER_POLY_MATRIX_PCK4)
	;
	INVOKE	eslStackHeapAllocate , hStackHeap, eax
	;
	mov	edi, pPolyEntry
	ASSUME	edi:PE3D_POLYGON_ENTRY
	mov	esi, [edi].surface.mesh.pMesh
	ASSUME	esi:PE3D_PRIMITIVE_MESH_LIST
	mov	pMeshSphare, eax
	mov	[edi].surface.mesh.pMeshReserved2, eax
	;
	; パラメータ計算
	;
	mov	esi, pMeshMatrix
	mov	edi, eax
	ASSUME	esi:PTR EGL_RENDER_POLY_MATRIX_PCK4
	ASSUME	edi:PTR EGL_RENDER_POLY_SPHERE_PCK4
	xor	edx, edx
	mov	ecx, nPolyCount
	shr	ecx, 5
	.WHILE	!ZERO?
		movaps	xmm0, [esi].xVertexA	; xmm0 = min x
		movaps	xmm1, [esi].yVertexA	; xmm1 = min y
		movaps	xmm2, [esi].zVertexA	; xmm2 = min z
		movaps	xmm4, xmm0		; xmm4 = max x
		movaps	xmm5, xmm1		; xmm5 = max y
		movaps	xmm6, xmm2		; xmm6 = max z
		;
		mov	eax, 8
		.REPEAT
			movaps	xmm3, [esi].xVertexA
			movaps	xmm7, [esi].yVertexA
			minps	xmm0, xmm3
			maxps	xmm4, xmm3
				movaps	xmm3, [esi].zVertexA
			minps	xmm1, xmm7
			maxps	xmm5, xmm7
				minps	xmm2, xmm3
				maxps	xmm6, xmm3
			;
			movaps	xmm3, [esi].xVertexB
			movaps	xmm7, [esi].yVertexB
			minps	xmm0, xmm3
			maxps	xmm4, xmm3
				movaps	xmm3, [esi].zVertexB
			minps	xmm1, xmm7
			maxps	xmm5, xmm7
				minps	xmm2, xmm3
				maxps	xmm6, xmm3
			;
			movaps	xmm3, [esi].xVertexC
			movaps	xmm7, [esi].yVertexC
			minps	xmm0, xmm3
			maxps	xmm4, xmm3
				movaps	xmm3, [esi].zVertexC
			minps	xmm1, xmm7
			maxps	xmm5, xmm7
				minps	xmm2, xmm3
				maxps	xmm6, xmm3
			;
			add	esi, (SIZEOF EGL_RENDER_POLY_MATRIX_PCK4)
			dec	eax
		.UNTIL	ZERO?
		;
		movhlps	xmm3, xmm0	; xmm0 <- min x
		movhlps	xmm7, xmm4	; xmm4 <- max x
		minps	xmm0, xmm3
		maxps	xmm4, xmm7
		movss	xmm3, xmm0
		movss	xmm7, xmm4
		shufps	xmm0, xmm0, 1
		shufps	xmm4, xmm4, 1
		minss	xmm0, xmm3
		maxss	xmm4, xmm7
		;
		movhlps	xmm3, xmm1	; xmm1 <- min y
		movhlps	xmm7, xmm5	; xmm5 <- max y
		minps	xmm1, xmm3
		maxps	xmm5, xmm7
		movss	xmm3, xmm1
		movss	xmm7, xmm5
		shufps	xmm1, xmm1, 1
		shufps	xmm5, xmm5, 1
		minss	xmm1, xmm3
		maxss	xmm5, xmm7
		;
		movhlps	xmm3, xmm2	; xmm2 <- min z
		movhlps	xmm7, xmm6	; xmm6 <- max z
		minps	xmm2, xmm3
		maxps	xmm6, xmm7
		movss	xmm3, xmm2
		movss	xmm7, xmm6
		shufps	xmm2, xmm2, 1
		shufps	xmm6, xmm6, 1
		minss	xmm2, xmm3
		maxss	xmm6, xmm7
		;
		unpcklps	xmm0, xmm1
		unpcklps	xmm4, xmm5
		movlhps		xmm0, xmm2
		movlhps		xmm4, xmm6
		movaps		xmm5, xmm4
		addps		xmm4, xmm0
		subps		xmm5, xmm0
		mulps		xmm4, xmmHalf3_Zero
		mulps		xmm5, xmmHalf3_Zero[10H]
		;
		mulps	xmm5, xmm5
			movaps	xmm1, xmm4
			movhlps	xmm2, xmm4
		movhlps	xmm6, xmm5
			shufps	xmm1, xmm1, 1
		addss	xmm6, xmm5
		shufps	xmm5, xmm5, 1
		addss	xmm5, xmm6
		sqrtss	xmm5, xmm5
		;
		movss	[edi].xCenter[edx*4], xmm4
		movss	[edi].yCenter[edx*4], xmm1
		movss	[edi].zCenter[edx*4], xmm2
		movss	[edi].rRadius[edx*4], xmm5
		;
		inc	edx
		lea	eax, [edi + (SIZEOF EGL_RENDER_POLY_SPHERE_PCK4)]
		and	edx, 03H
		cmovz	edi, eax
		;
		dec	ecx
	.ENDW
	;
	mov	eax, nPolyCount
	and	eax, 1FH
	.IF	!ZERO?
		xor	ecx, ecx
		movss	xmm0, [esi].xVertexA[0]	; xmm0 = min x
		movss	xmm1, [esi].yVertexA[0]	; xmm1 = min y
		movss	xmm2, [esi].zVertexA[0]	; xmm2 = min z
		movss	xmm4, xmm0		; xmm4 = max x
		movss	xmm5, xmm1		; xmm5 = max y
		movss	xmm6, xmm2		; xmm6 = max z
		;
		.REPEAT
			movss	xmm3, [esi].xVertexA[ecx*4]
			movss	xmm7, [esi].yVertexA[ecx*4]
			minss	xmm0, xmm3
			maxss	xmm4, xmm3
				movss	xmm3, [esi].zVertexA[ecx*4]
			minss	xmm1, xmm7
			maxss	xmm5, xmm7
				minss	xmm2, xmm3
				maxss	xmm6, xmm3
			;
			movss	xmm3, [esi].xVertexB[ecx*4]
			movss	xmm7, [esi].yVertexB[ecx*4]
			minss	xmm0, xmm3
			maxss	xmm4, xmm3
				movss	xmm3, [esi].zVertexB[ecx*4]
			minss	xmm1, xmm7
			maxss	xmm5, xmm7
				minss	xmm2, xmm3
				maxss	xmm6, xmm3
			;
			movss	xmm3, [esi].xVertexC[ecx*4]
			movss	xmm7, [esi].yVertexC[ecx*4]
			minss	xmm0, xmm3
			maxss	xmm4, xmm3
				movss	xmm3, [esi].zVertexC[ecx*4]
			minss	xmm1, xmm7
			maxss	xmm5, xmm7
				minss	xmm2, xmm3
				maxss	xmm6, xmm3
			;
			inc	ecx
			and	ecx, 03H
			.IF	ZERO?
				add	esi, (SIZEOF EGL_RENDER_POLY_MATRIX_PCK4)
			.ENDIF
			dec	eax
		.UNTIL	ZERO?
		;
		unpcklps	xmm0, xmm1
		unpcklps	xmm4, xmm5
		movlhps		xmm0, xmm2
		movlhps		xmm4, xmm6
		movaps		xmm5, xmm4
		addps		xmm4, xmm0
		subps		xmm5, xmm0
		mulps		xmm4, xmmHalf3_Zero
		mulps		xmm5, xmmHalf3_Zero[10H]
		;
		mulps	xmm5, xmm5
			movaps	xmm1, xmm4
			movhlps	xmm2, xmm4
		movhlps	xmm6, xmm5
			shufps	xmm1, xmm1, 1
		addss	xmm6, xmm5
		shufps	xmm5, xmm5, 1
		addss	xmm5, xmm6
		sqrtss	xmm5, xmm5
		;
		movss	[edi].xCenter[edx*4], xmm4
		movss	[edi].yCenter[edx*4], xmm1
		movss	[edi].zCenter[edx*4], xmm2
		movss	[edi].rRadius[edx*4], xmm5
		;
		inc	edx
		and	edx, 03H
	.ENDIF
	;
	.IF	edx != 0
		xorps	xmm0, xmm0
		.REPEAT
			movss	[edi].xCenter[edx*4], xmm0
			movss	[edi].yCenter[edx*4], xmm0
			movss	[edi].zCenter[edx*4], xmm0
			movss	[edi].rRadius[edx*4], xmm0
			inc	edx
		.UNTIL	edx >= 4
	.ENDIF

	ASSUME	ebx:NOTHING
	ASSUME	edi:NOTHING
	ret

eglRenderPoly@PrepareRayTracingMeshParamSSE	ENDP

;
;	レイトレーシングターゲット設定 SSE コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@PrepareRayTracingTargetSSE	PROC	NEAR32 C USES ebx,
	hStackHeap:HSTACKHEAP,
	ppPolyList:PTR PE3D_POLYGON_ENTRY, nPolyCount:DWORD

	LOCAL	pTarget:PEGL_RENDER_MESH_MATRIX_PCK4
	LOCAL	dwCount:DWORD

	mov	eax, nPolyCount
	add	eax, 3
	shr	eax, 2
	imul	eax, (SIZEOF EGL_RENDER_MESH_MATRIX_PCK4)
	INVOKE	eslStackHeapAllocate , hStackHeap, eax
	mov	pTarget, eax
	mov	edi, eax
	ASSUME	edi:PEGL_RENDER_MESH_MATRIX_PCK4
	;
	mov	esi, ppPolyList
	mov	edx, nPolyCount
	xor	ecx, ecx
	test	edx, edx
	.WHILE	!ZERO?
		mov	ebx, DWORD PTR [esi]
		add	esi, 4
		test	ebx, ebx
		.IF	!ZERO?
		ASSUME	ebx:PE3D_POLYGON_ENTRY
		.IF	[ebx].dwTypeFlag & E3D_MESH_POLYGON
			.IF	[ebx].surface.mesh.pMeshReserved == NULL
				push	ecx
				push	edx
				INVOKE	eglRenderPoly@PrepareRayTracingMeshParamSSE,
						hStackHeap, ebx
				pop	edx
				pop	ecx
			.ENDIF
			lea	eax, [ecx + 1]
			and	ecx, 03H
			mov	dwCount, eax
			movss	xmm0, [ebx].vCenter.x
			movss	xmm1, [ebx].vCenter.y
			movss	xmm2, [ebx].vCenter.z
			movss	xmm3, [ebx].surface.mesh.rMeshRadius
			movss	[edi].xCenter[ecx*4], xmm0
			movss	[edi].yCenter[ecx*4], xmm1
			movss	[edi].zCenter[ecx*4], xmm2
			movss	[edi].rRadius[ecx*4], xmm3
			mov	[edi].pMesh[ecx*4], ebx
			lea	eax, [edi + (SIZEOF EGL_RENDER_MESH_MATRIX_PCK4)]
			cmp	ecx, 3
			cmovz	edi, eax
			mov	ecx, dwCount
		.ENDIF
		ASSUME	ebx:NOTHING
		.ENDIF
		dec	edx
	.ENDW
	mov	eax, ecx
	xor	edx, edx
	and	eax, 03H
	.WHILE	!ZERO?
		mov	[edi].xCenter[eax*4], edx
		mov	[edi].yCenter[eax*4], edx
		mov	[edi].zCenter[eax*4], edx
		mov	[edi].rRadius[eax*4], edx
		mov	[edi].pMesh[eax*4], edx
		inc	eax
		and	eax, 03H
	.ENDW
	ASSUME	edi:NOTHING
	mov	edi, pTarget
	ret

eglRenderPoly@PrepareRayTracingTargetSSE	ENDP

eglRenderPoly@AttachRayTracingTargetSSE	PROC	NEAR32 C USES ebx esi edi,
	hRenderPoly:HEGL_RENDER_POLYGON, hStackHeap:HSTACKHEAP,
	ppShadowingPoly:PTR PE3D_POLYGON_ENTRY, nShadowingCount:DWORD,
	ppRayTracingPoly:PTR PE3D_POLYGON_ENTRY, nRayTracingCount:DWORD,
	ppGlobalReflection:PTR PE3D_POLYGON_ENTRY, nGlobalReflections:DWORD

	mov	ebx, hRenderPoly
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF

	;
	; 影ターゲット設定
	;
	mov	ecx, nShadowingCount
	.IF	ecx != 0
		INVOKE	eglRenderPoly@PrepareRayTracingTargetSSE,
				hStackHeap, ppShadowingPoly, ecx
		mov	[ebx].pRayShadowing, edi
	.ENDIF
	mov	[ebx].nRayShadowings, ecx
	;
	add	ecx, 3
	shr	ecx, 2
	mov	[ebx].nRayShadowingListCount, ecx
	.IF	!ZERO?
		INVOKE	eslStackHeapAllocate , hStackHeap, ADDR [ecx * 4]
		mov	[ebx].ppRayShadowingList, eax
		mov	ecx, [ebx].nRayShadowingListCount
		mov	edx, [ebx].pRayShadowing
		test	ecx, ecx
		.WHILE	!ZERO?
			mov	DWORD PTR [eax], edx
			add	eax, 4
			add	edx, (SIZEOF EGL_RENDER_MESH_MATRIX_PCK4)
			dec	ecx
		.ENDW
	.ENDIF

	;
	; 反射・屈折反映ターゲット設定
	;
	mov	ecx, nRayTracingCount
	.IF	ecx != 0
		INVOKE	eglRenderPoly@PrepareRayTracingTargetSSE,
				hStackHeap, ppRayTracingPoly, ecx
		mov	[ebx].pRayReflection, edi
	.ENDIF
	mov	[ebx].nRayReflections, ecx
	;
	add	ecx, 3
	shr	ecx, 2
	mov	[ebx].nRayReflectionListCount, ecx
	.IF	!ZERO?
		INVOKE	eslStackHeapAllocate , hStackHeap, ADDR [ecx * 4]
		mov	[ebx].ppRayReflectionList, eax
		mov	ecx, [ebx].nRayReflectionListCount
		mov	edx, [ebx].pRayReflection
		test	ecx, ecx
		.WHILE	!ZERO?
			mov	DWORD PTR [eax], edx
			add	eax, 4
			add	edx, (SIZEOF EGL_RENDER_MESH_MATRIX_PCK4)
			dec	ecx
		.ENDW
	.ENDIF

	mov	ecx, nGlobalReflections
	.IF	ecx != 0
		INVOKE	eglRenderPoly@PrepareRayTracingTargetSSE,
				hStackHeap, ppGlobalReflection, ecx
		mov	[ebx].pGlobalRayReflection, edi
	.ENDIF
	mov	[ebx].nGlobalRayReflections, ecx

	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglRenderPoly@AttachRayTracingTargetSSE	ENDP

;
;	レイトレーシング用ポリゴンエントリ作成 SSE コード
; ----------------------------------------------------------------------------
eglRenderPoly@CreatePolygonEntryRT_SSE	PROC	NEAR32 C USES ebx esi edi,
	hRenderPoly:HEGL_RENDER_POLYGON, hStackHeap:HSTACKHEAP,
	pPrimitive:PCE3D_PRIMITIVE_POLYGON, pdwResult:PTR DWORD

	LOCAL	pPolyEntry:PE3D_POLYGON_ENTRY
	LOCAL	nResult:DWORD
	LOCAL	nMeshAllocBytes:DWORD
	LOCAL	nPolyCount:DWORD
	LOCAL	pMeshList:PTR E3D_PRIMITIVE_MESH_LIST
	LOCAL	pVertices:PE3D_VECTOR4

	mov	esi, pPrimitive
	mov	ebx, hRenderPoly
	ASSUME	esi:PCE3D_PRIMITIVE_POLYGON
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF

	mov	eax, [esi].dwTypeFlag
	.IF	eax & E3D_MESH_POLYGON
	; --------------------------------------------------------------------
	;	ポリゴンメッシュ
	; --------------------------------------------------------------------
	INVOKE	eslStackHeapAllocate , hStackHeap, (SIZEOF E3D_POLYGON_ENTRY)
	mov	pPolyEntry, eax
	mov	edi, eax
	ASSUME	edi:PTR E3D_POLYGON_ENTRY
	;
	;	フラグ・頂点数・表面属性設定
	; --------------------------------------------------------------------
	mov	eax, [esi].dwTypeFlag
	mov	[edi].dwTypeFlag, eax
	;
	mov	eax, [esi].pSurfaceAttr
	mov	edx, [esi].dwVertexCount
	mov	[edi].pAttr, eax
	mov	[edi].dwVertexCount, edx
	mov	[edi].dwProjectedCount, 0
	ASSUME	eax:PTR E3D_SURFACE_ATTRIBUTE
	mov	eax, [eax].dwShadingFlags
	ASSUME	eax:NOTHING
	mov	ecx, eax
	and	eax, [ebx].dwMaskShadingFlags
	or	eax, [ebx].dwAddShadingFlags
	mov	edx, eax
	and	eax, NOT E3DSAF_SHADING_MASK
	test	ecx, E3DSAF_SHADING_MASK
	cmovnz	eax, edx
	mov	[edi].dwTransparency, 0
	mov	[edi].dwShadingFlags, eax
	mov	[edi].surface.mesh.pMeshReserved, 0
	;
	;	頂点複製
	; --------------------------------------------------------------------
	mov	eax, [edi].dwVertexCount
	cmp	eax, 3
	jb	Label_ErrorExit
	shl	eax, 4				; * (SIZEOF E3D_VECTOR4)
	INVOKE	eslStackHeapAllocate , hStackHeap, eax
	mov	[edi].pVertexes, eax
	mov	edx, eax
	mov	esi, [esi].mesh.vertices
	ASSUME	esi:PE3D_VECTOR4
	mov	ecx, [edi].dwVertexCount
	;
	movups	xmm0, [esi]
	movups	xmm1, [esi + 10H]
	movaps	xmm5, xmm0			; xmm5 := min pos
	movaps	xmm6, xmm0			; xmm6 := max pos
	movaps	[edx], xmm0
	add	esi, 20H
	add	edx, 10H
	sub	ecx, 2
	.WHILE	!ZERO?
		movaps	xmm0, xmm1
		movups	xmm1, [esi]
		add	esi, 10H
		minps	xmm5, xmm0
		maxps	xmm6, xmm0
		movaps	[edx], xmm0
		add	edx, 10H
		dec	ecx
	.ENDW
	minps	xmm5, xmm1
	maxps	xmm6, xmm1
	movaps	[edx], xmm1
	ASSUME	esi:NOTHING
	;
	;	画面外判定
	; --------------------------------------------------------------------
	;
	; ｚ判定
	;
	xor	eax, eax
	movhlps	xmm0, xmm5
	movhlps	xmm1, xmm6
	comiss	xmm0, [ebx].rZMaxClip
	movups	[edi].surface.mesh.vMinMesh, xmm5
	jae	Label_MeshNoRender
	comiss	xmm1, [ebx].rZMinClip
	movups	[edi].surface.mesh.vMaxMesh, xmm6
	jbe	Label_MeshNoRender
	;
	; x, y 判定
	;
	movss	xmm4, [ebx].vScreenPos.z
	divss	xmm4, xmm1
	movlps	xmm3, QWORD PTR [ebx].vScreenPos.x
	movlhps	xmm6, xmm5
	shufps	xmm4, xmm4, 0
	shufps	xmm3, xmm3, 01000100B
	mulps	xmm4, xmm6
	cvtpi2ps	xmm2, QWORD PTR [ebx].dib.rectClip.left
	addps	xmm4, xmm3
	cvtpi2ps	xmm3, QWORD PTR [ebx].dib.rectClip.right
	shufps	xmm2, xmm4, 11100100B	; ?= {left, top, min x, min y}
	movlhps	xmm4, xmm3		;    > {max x, max y, right, bottom}
	cmpps	xmm2, xmm4, 6
	movmskps	eax, xmm2
	test	eax, 0FH
	mov	eax, 0
	jnz	Label_MeshNoRender
	mov	eax, E3D_RTCPE_RESULT_SHOULD_RENDER
Label_MeshNoRender:
	mov	nResult, eax
	;
	;	重心点を計算する
	; --------------------------------------------------------------------
	mov	esi, pPrimitive
	ASSUME	esi:PCE3D_PRIMITIVE_POLYGON
	movups	xmm5, [edi].surface.mesh.vMinMesh
	movups	xmm6, [edi].surface.mesh.vMaxMesh
	movaps	xmm4, xmmHalf3_Zero
	movaps	xmm7, xmm5
	addps	xmm5, xmm6
	subps	xmm6, xmm7
	mulps	xmm5, xmm4
	mulps	xmm6, xmm6
	movups	[edi].vCenter, xmm5
	movhlps	xmm7, xmm6
	addss	xmm7, xmm6
	shufps	xmm6, xmm6, 1
	addss	xmm7, xmm6
	sqrtss	xmm7, xmm7
	mulss	xmm7, xmm4
	mov	[edi].surface.mesh.pMeshReserved, NULL
	movss	[edi].surface.mesh.rMeshRadius, xmm7
	;
	;	法線複製
	; --------------------------------------------------------------------
	.IF	[edi].dwTypeFlag & E3D_SMOOTH_POLYGON
		mov	eax, [edi].dwVertexCount
		shl	eax, 4				; * (SIZEOF E3D_VECTOR4)
		INVOKE	eslStackHeapAllocate , hStackHeap, eax
		mov	[edi].pNormals, eax
		mov	edx, eax
		mov	esi, [esi].mesh.normals
		ASSUME	esi:PE3D_VECTOR4
		mov	ecx, [edi].dwVertexCount
		sub	ecx, 3
		.WHILE	!SIGN?
			movups	xmm0, [esi]
			movups	xmm1, [esi + 10H]
			movups	xmm6, [esi + 20H]
			add	esi, 30H
			movaps	xmm4, xmm0
			mulps	xmm0, xmm0
				movaps	xmm5, xmm1
				mulps	xmm1, xmm1
					movaps	xmm7, xmm6
					mulps	xmm6, xmm6
			movhlps	xmm2, xmm0
				movhlps	xmm3, xmm1
			addss	xmm2, xmm0
			shufps	xmm0, xmm0, 1
				addss	xmm3, xmm1
				shufps	xmm1, xmm1, 1
			addss	xmm0, xmm2
					movhlps	xmm2, xmm6
				addss	xmm1, xmm3
			rsqrtss	xmm0, xmm0
					addss	xmm2, xmm6
					shufps	xmm6, xmm6, 1
				rsqrtss	xmm1, xmm1
					addss	xmm6, xmm2
			shufps	xmm0, xmm0, 0
				shufps	xmm1, xmm1, 0
					rsqrtss	xmm6, xmm6
			mulps	xmm0, xmm4
					shufps	xmm6, xmm6, 0
				mulps	xmm1, xmm5
					mulps	xmm6, xmm7
			movaps	[edx], xmm0
			movaps	[edx + 10H], xmm1
			movaps	[edx + 20H], xmm6
			add	edx, 30H
			sub	ecx, 3
		.ENDW
		add	ecx, 3
		.WHILE	!ZERO?
			movups	xmm0, [esi]
			add	esi, 10H
			movaps	xmm4, xmm0
			mulps	xmm0, xmm0
			movhlps	xmm2, xmm0
			addss	xmm2, xmm0
			shufps	xmm0, xmm0, 1
			addss	xmm0, xmm2
			rsqrtss	xmm0, xmm0
			shufps	xmm0, xmm0, 0
			mulps	xmm0, xmm4
			movaps	[edx], xmm0
			add	edx, 10H
			dec	ecx
		.ENDW
		mov	esi, pPrimitive
		ASSUME	esi:PCE3D_PRIMITIVE_POLYGON
	.ELSE
		mov	[edi].pNormals, 0
	.ENDIF
	;
	;	UV座標複製
	; --------------------------------------------------------------------
	lea	esi, [esi].mesh.uv_map[0]
	ASSUME	esi:PTR E3D_VECTOR_2D
	.IF	([edi].dwTypeFlag & E3D_TEXTURE_POLYGON) \
			&& ([edi].dwShadingFlags & E3DSAF_TEXTURE_MAPPING)
		mov	eax, [edi].dwVertexCount
		shl	eax, 3				; * (SIZEOF E3D_VECTOR_2D)
		INVOKE	eslStackHeapAllocate , hStackHeap, eax
		mov	[edi].surface.mesh.pUVMap, eax
		mov	edx, eax
		mov	ecx, [edi].dwVertexCount
		sub	ecx, 2
		.WHILE	!SIGN?
			movups	xmm0, XMMWORD_PTR [esi]
			add	esi, 10H
			movaps	XMMWORD_PTR [edx], xmm0
			add	edx, 10H
			sub	ecx, 2
		.ENDW
		add	ecx, 2
		.IF	!ZERO?
			movlps	xmm0, QWORD PTR [esi]
			movlps	QWORD PTR [edx], xmm0
			add	esi, 8
		.ENDIF
	.ELSE
		and	[edi].dwTypeFlag, NOT E3D_TEXTURE_POLYGON
		and	[edi].dwShadingFlags, NOT E3DSAF_TEXTURE_MAPPING
		mov	[edi].surface.mesh.pUVMap, 0
		mov	eax, [edi].dwVertexCount
		lea	esi, [esi + eax * (SIZEOF E3D_VECTOR_2D)]
	.ENDIF
	;
	;	メッシュリスト複製
	; --------------------------------------------------------------------
	ASSUME	esi:PTR E3D_PRIMITIVE_MESH_LIST
	mov	eax, [esi].dwPolyCount
	shl	eax, 4
	add	eax, 8
	cmp	eax, [esi].dwMeshBytes
	ja	Label_ErrorExit
	.IF	ZERO?	; [esi].dwMeshBytes == ([esi].dwPolyCount * 16) + 8
		;
		; 三角ポリゴン限定の場合には複製のみ
		;
		INVOKE	eslStackHeapAllocate , hStackHeap, [esi].dwMeshBytes
		mov	[edi].surface.mesh.pMesh, eax
		mov	edx, eax
		mov	ecx, [esi].dwMeshBytes
		shr	ecx, 2
		sub	ecx, 8
		ASSUME	esi:NOTHING
		.WHILE	!SIGN?
			movups	xmm0, [esi]
			movups	xmm1, [esi + 10H]
			add	esi, 20H
			movaps	[edx], xmm0
			movaps	[edx + 10H], xmm1
			add	edx, 20H
			sub	ecx, 8
		.ENDW
		add	ecx, 8
		.WHILE	!ZERO?
			mov	eax, DWORD PTR [esi]
			add	esi, 4
			mov	DWORD PTR [edx], eax
			add	edx, 4
			dec	ecx
		.ENDW
	.ELSE
		;
		; 四角ポリゴン以上を含んでいる場合には三角ポリゴンに変換
		;
		ASSUME	esi:PTR E3D_PRIMITIVE_MESH_LIST
		mov	ecx, [esi].dwMeshBytes
		sub	ecx, eax
		shl	ecx, 2
		add	eax, ecx
		mov	nMeshAllocBytes, eax
		;
		INVOKE	eslStackHeapAllocate , hStackHeap, eax
		mov	pMeshList, eax
		mov	[edi].surface.mesh.pMesh, eax
		mov	edi, eax
		ASSUME	edi:PTR E3D_PRIMITIVE_MESH_LIST
		;
		mov	ecx, [esi].dwPolyCount
		lea	esi, [esi].mpEntries[0]
		lea	edi, [edi].mpEntries[0]
		ASSUME	esi:PTR E3D_PRIMITIVE_MESH_POLY
		ASSUME	edi:PTR E3D_PRIMITIVE_MESH_POLY
		xor	edx, edx
		test	ecx, ecx
		;mov	nPolyCount, edx
		.WHILE	!ZERO?
			push	ecx
			;
			mov	ecx, 2
			.WHILE	ecx < [esi].dwVertexCount
				mov	eax, [esi].dwIndex[0]
				movlps	xmm0, QWORD PTR [esi].dwIndex[ecx*4-4]
				inc	ecx
				mov	[edi].dwVertexCount, 3
				mov	[edi].dwIndex[0], eax
				movlps	QWORD PTR [edi].dwIndex[4], xmm0
				lea	edi, [edi].dwIndex[4*3]
				inc	edx
			.ENDW
			;
			pop	ecx
			mov	eax, [esi].dwVertexCount
			dec	ecx
			lea	esi, [esi].dwIndex[eax*4]
		.ENDW
		;
		;mov	nPolyCount, edx
		mov	eax, pMeshList
		sub	edi, pMeshList
		cmp	edi, nMeshAllocBytes
		ja	Label_ErrorExit
		;
		ASSUME	eax:PTR E3D_PRIMITIVE_MESH_LIST
		mov	[eax].dwMeshBytes, edi
		mov	[eax].dwPolyCount, edx
		ASSUME	eax:NOTHING
		;
		mov	edi, pPolyEntry
		ASSUME	edi:PE3D_POLYGON_ENTRY
	.ENDIF
	;
	;	頂点色設定
	; --------------------------------------------------------------------
	mov	ecx, [edi].dwVertexCount
	shl	ecx, 3			; *= (SIZEOF E3D_COLOR)
	INVOKE	eslStackHeapAllocate , hStackHeap, ecx
	mov	[edi].pVertexColors, eax
	mov	edx, eax
	;
	.IF	[edi].dwTypeFlag & E3D_VERTEX_COLOR_POLYGON
		mov	esi, pPrimitive
		ASSUME	esi:PCE3D_PRIMITIVE_POLYGON
		lea	esi, [esi].mesh.color[0]
		ASSUME	esi:PTR E3D_COLOR
		mov	ecx, [edi].dwVertexCount
		shufps	xmm0, xmm0, 01000100B
		sub	ecx, 4
		.WHILE	!SIGN?
			movups	xmm0, XMMWORD_PTR [esi]
			movups	xmm1, XMMWORD_PTR [esi + 10H]
			add	esi, 20H	; += (SIZEOF E3D_COLOR) * 4
			movaps	XMMWORD_PTR [edx], xmm0
			movaps	XMMWORD_PTR [edx + 10H], xmm1
			add	edx, 20H	; += (SIZEOF E3D_COLOR) * 4
			sub	ecx, 4
		.ENDW
		add	ecx, 4
		.WHILE	!ZERO?
			movlps	xmm0, QWORD PTR [esi]
			add	esi, 8		; += (SIZEOF E3D_COLOR)
			movlps	QWORD PTR [edx], xmm0
			add	edx, 8		; += (SIZEOF E3D_COLOR)
			dec	ecx
		.ENDW
		ASSUME	esi:NOTHING
	.ELSE
		mov	esi, [edi].pAttr
		ASSUME	esi:PE3D_SURFACE_ATTRIBUTE
		movlps	xmm0, QWORD PTR [esi].rgbaColor
		mov	ecx, [edi].dwVertexCount
		shufps	xmm0, xmm0, 01000100B
		sub	ecx, 4
		.WHILE	!SIGN?
			movaps	[edx], xmm0
			movaps	[edx + 10H], xmm0
			add	edx, 20H
			sub	ecx, 4
		.ENDW
		add	ecx, 4
		.WHILE	!ZERO?
			movlps	QWORD PTR [edx], xmm0
			add	edx, 8		; += (SIZEOF E3D_COLOR)
			dec	ecx
		.ENDW
	.ENDIF

	.ELSEIF	!(eax & (NOT E3D_POLYGON_PRIMITIVE_MASK))
	; --------------------------------------------------------------------
	;	通常ポリゴン
	; --------------------------------------------------------------------
	ASSUME	esi:PCE3D_PRIMITIVE_POLYGON
	INVOKE	eslStackHeapAllocate , hStackHeap, (SIZEOF E3D_POLYGON_ENTRY)
	mov	pPolyEntry, eax
	mov	edi, eax
	ASSUME	edi:PTR E3D_POLYGON_ENTRY
	;
	;	フラグ・頂点数・表面属性設定（ポリゴン→メッシュ）
	; --------------------------------------------------------------------
	mov	eax, [esi].dwTypeFlag
	or	eax, E3D_MESH_POLYGON
	mov	[edi].dwTypeFlag, eax
	;
	mov	eax, [esi].pSurfaceAttr
	mov	edx, [esi].dwVertexCount
	mov	[edi].pAttr, eax
	mov	[edi].dwVertexCount, edx
	mov	[edi].dwProjectedCount, 0
	ASSUME	eax:PTR E3D_SURFACE_ATTRIBUTE
	mov	eax, [eax].dwShadingFlags
	ASSUME	eax:NOTHING
	mov	ecx, eax
	and	eax, [ebx].dwMaskShadingFlags
	or	eax, [ebx].dwAddShadingFlags
	mov	edx, eax
	and	eax, NOT E3DSAF_SHADING_MASK
	test	ecx, E3DSAF_SHADING_MASK
	cmovnz	eax, edx
	mov	[edi].dwTransparency, 0
	mov	[edi].dwShadingFlags, eax
	mov	[edi].surface.mesh.pMeshReserved, 0
	;
	;	頂点複製（ポリゴン→メッシュ）
	; --------------------------------------------------------------------
	mov	eax, [edi].dwVertexCount
	cmp	eax, 3
	jb	Label_ErrorExit
	shl	eax, 4				; * (SIZEOF E3D_VECTOR4)
	INVOKE	eslStackHeapAllocate , hStackHeap, eax
	mov	[edi].pVertexes, eax
	mov	edx, eax
	lea	esi, [esi].polygon[0]
	ASSUME	esi:PTR E3D_PRIMITIVE_VERTEX
	mov	ecx, [edi].dwVertexCount
	;
	mov	eax, [esi].vertex
	add	esi, (SIZEOF E3D_PRIMITIVE_VERTEX)
	movups	xmm0, [eax]
	movaps	xmm5, xmm0			; xmm5 := min pos
	movaps	xmm6, xmm0			; xmm6 := max pos
	movaps	[edx], xmm0
	add	edx, 10H
	dec	ecx
	.WHILE	!ZERO?
		mov	eax, [esi].vertex
		add	esi, (SIZEOF E3D_PRIMITIVE_VERTEX)
		movups	xmm0, [eax]
		minps	xmm5, xmm0
		maxps	xmm6, xmm0
		movaps	[edx], xmm0
		add	edx, 10H
		dec	ecx
	.ENDW
	ASSUME	esi:NOTHING
	;
	;	画面外判定（ポリゴン→メッシュ）
	; --------------------------------------------------------------------
	;
	; ｚ判定
	;
	xor	eax, eax
	movhlps	xmm0, xmm5
	movhlps	xmm1, xmm6
	comiss	xmm0, [ebx].rZMaxClip
	movups	[edi].surface.mesh.vMinMesh, xmm5
	jae	Label_PolyNoRender
	comiss	xmm1, [ebx].rZMinClip
	movups	[edi].surface.mesh.vMaxMesh, xmm6
	jbe	Label_PolyNoRender
	;
	; x, y 判定
	;
	movss	xmm4, [ebx].vScreenPos.z
	divss	xmm4, xmm1
	movlps	xmm3, QWORD PTR [ebx].vScreenPos.x
	movlhps	xmm6, xmm5
	shufps	xmm4, xmm4, 0
	shufps	xmm3, xmm3, 01000100B
	mulps	xmm4, xmm6
	cvtpi2ps	xmm2, QWORD PTR [ebx].dib.rectClip.left
	addps	xmm4, xmm3
	cvtpi2ps	xmm3, QWORD PTR [ebx].dib.rectClip.right
	shufps	xmm2, xmm4, 11100100B	; ?= {left, top, min x, min y}
	movlhps	xmm4, xmm3		;    > {max x, max y, right, bottom}
	cmpps	xmm2, xmm4, 6
	movmskps	eax, xmm2
	test	eax, 0FH
	mov	eax, 0
	jnz	Label_PolyNoRender
	mov	eax, E3D_RTCPE_RESULT_SHOULD_RENDER
Label_PolyNoRender:
	mov	nResult, eax
	;
	;	重心点を計算する（ポリゴン→メッシュ）
	; --------------------------------------------------------------------
	mov	esi, pPrimitive
	ASSUME	esi:PCE3D_PRIMITIVE_POLYGON
	movups	xmm5, [edi].surface.mesh.vMinMesh
	movups	xmm6, [edi].surface.mesh.vMaxMesh
	movaps	xmm4, xmmHalf3_Zero
	movaps	xmm7, xmm5
	addps	xmm5, xmm6
	subps	xmm6, xmm7
	mulps	xmm5, xmm4
	mulps	xmm6, xmm6
	movups	[edi].vCenter, xmm5
	movhlps	xmm7, xmm6
	addss	xmm7, xmm6
	shufps	xmm6, xmm6, 1
	addss	xmm7, xmm6
	sqrtss	xmm7, xmm7
	mulss	xmm7, xmm4
	mov	[edi].surface.mesh.pMeshReserved, NULL
	movss	[edi].surface.mesh.rMeshRadius, xmm7
	;
	;	法線複製（ポリゴン→メッシュ）
	; --------------------------------------------------------------------
	.IF	[edi].dwTypeFlag & E3D_SMOOTH_POLYGON
		mov	eax, [edi].dwVertexCount
		shl	eax, 4				; * (SIZEOF E3D_VECTOR4)
		INVOKE	eslStackHeapAllocate , hStackHeap, eax
		mov	[edi].pNormals, eax
		mov	edx, eax
		lea	esi, [esi].polygon[0]
		ASSUME	esi:PTR E3D_PRIMITIVE_VERTEX
		mov	ecx, [edi].dwVertexCount
		.REPEAT
			mov	eax, [esi].normal
			add	esi, (SIZEOF E3D_PRIMITIVE_VERTEX)
			movups	xmm0, [eax]
			movaps	xmm4, xmm0
			mulps	xmm0, xmm0
			movhlps	xmm2, xmm0
			addss	xmm2, xmm0
			shufps	xmm0, xmm0, 1
			addss	xmm0, xmm2
			rsqrtss	xmm0, xmm0
			shufps	xmm0, xmm0, 0
			mulps	xmm0, xmm4
			movaps	[edx], xmm0
			add	edx, 10H
			dec	ecx
		.UNTIL	ZERO?
		mov	esi, pPrimitive
		ASSUME	esi:PCE3D_PRIMITIVE_POLYGON
	.ELSE
		mov	[edi].pNormals, 0
	.ENDIF
	;
	;	UV座標複製（ポリゴン→メッシュ）
	; --------------------------------------------------------------------
	lea	esi, [esi].polygon[0]
	ASSUME	esi:PTR E3D_PRIMITIVE_VERTEX
	.IF	([edi].dwTypeFlag & E3D_TEXTURE_POLYGON) \
			&& ([edi].dwShadingFlags & E3DSAF_TEXTURE_MAPPING)
		mov	eax, [edi].dwVertexCount
		shl	eax, 3				; * (SIZEOF E3D_VECTOR_2D)
		INVOKE	eslStackHeapAllocate , hStackHeap, eax
		mov	[edi].surface.mesh.pUVMap, eax
		mov	edx, eax
		mov	ecx, [edi].dwVertexCount
		sub	ecx, 2
		.WHILE	!SIGN?
			movlps	xmm0, QWORD PTR [esi].uv_map
			movhps	xmm0, QWORD PTR [esi + (SIZEOF E3D_PRIMITIVE_VERTEX)].uv_map
			add	esi, (SIZEOF E3D_PRIMITIVE_VERTEX) * 2
			movaps	[edx], xmm0
			add	edx, 10H
			sub	ecx, 2
		.ENDW
		add	ecx, 2
		.IF	!ZERO?
			movlps	xmm0, QWORD PTR [esi].uv_map
			movlps	QWORD PTR [edx], xmm0
		.ENDIF
	.ELSE
		and	[edi].dwTypeFlag, NOT E3D_TEXTURE_POLYGON
		and	[edi].dwShadingFlags, NOT E3DSAF_TEXTURE_MAPPING
		mov	[edi].surface.mesh.pUVMap, 0
	.ENDIF
	ASSUME	esi:NOTHING
	;
	;	メッシュリスト生成（ポリゴン→メッシュ）
	; --------------------------------------------------------------------
	mov	eax, [edi].dwVertexCount
	sub	eax, 2
	mov	nPolyCount, eax
	shl	eax, 4
	add	eax, 8
	mov	nMeshAllocBytes, eax
	;
	INVOKE	eslStackHeapAllocate , hStackHeap, eax
	mov	[edi].surface.mesh.pMesh, eax
	mov	edx, eax
	ASSUME	edx:PTR E3D_PRIMITIVE_MESH_LIST
	mov	eax, nMeshAllocBytes
	mov	ecx, nPolyCount
	mov	[edx].dwMeshBytes, eax
	mov	[edx].dwPolyCount, ecx
	lea	edx, [edx].mpEntries[0]
	mov	eax, 1
	ASSUME	edx:PTR E3D_PRIMITIVE_MESH_POLY
	.REPEAT
		lea	esi, [eax + 1]
		mov	[edx].dwVertexCount, 3
		mov	[edx].dwIndex[0], 0
		mov	[edx].dwIndex[4], eax
		mov	[edx].dwIndex[8], esi
		lea	edx, [edx].dwIndex[12]
		mov	eax, esi
		dec	ecx
	.UNTIL	ZERO?
	ASSUME	edx:NOTHING
	;
	;	頂点色設定（ポリゴン→メッシュ）
	; --------------------------------------------------------------------
	mov	ecx, [edi].dwVertexCount
	shl	ecx, 3			; *= (SIZEOF E3D_COLOR)
	INVOKE	eslStackHeapAllocate , hStackHeap, ecx
	mov	[edi].pVertexColors, eax
	mov	edx, eax
	;
	.IF	[edi].dwTypeFlag & E3D_VERTEX_COLOR_POLYGON
		mov	esi, pPrimitive
		ASSUME	esi:PCE3D_PRIMITIVE_POLYGON
		lea	esi, [esi].polygon[0]
		ASSUME	esi:PTR E3D_PRIMITIVE_VERTEX
		mov	ecx, [edi].dwVertexCount
		shufps	xmm0, xmm0, 01000100B
		sub	ecx, 2
		.WHILE	!SIGN?
			movlps	xmm0, QWORD PTR [esi].color
			movhps	xmm0, QWORD PTR  [esi + (SIZEOF E3D_PRIMITIVE_VERTEX)].color
			add	esi, (SIZEOF E3D_PRIMITIVE_VERTEX) * 2
			movaps	XMMWORD_PTR [edx], xmm0
			add	edx, (SIZEOF E3D_COLOR) * 2
			sub	ecx, 2
		.ENDW
		add	ecx, 2
		.IF	!ZERO?
			movlps	xmm0, QWORD PTR [esi].color
			movlps	QWORD PTR [edx], xmm0
		.ENDIF
		ASSUME	esi:NOTHING
	.ELSE
		mov	esi, [edi].pAttr
		ASSUME	esi:PE3D_SURFACE_ATTRIBUTE
		movlps	xmm0, QWORD PTR [esi].rgbaColor
		mov	ecx, [edi].dwVertexCount
		shufps	xmm0, xmm0, 01000100B
		sub	ecx, 4
		.WHILE	!SIGN?
			movaps	[edx], xmm0
			movaps	[edx + 10H], xmm0
			add	edx, 20H
			sub	ecx, 4
		.ENDW
		add	ecx, 4
		.WHILE	!ZERO?
			movlps	QWORD PTR [edx], xmm0
			add	edx, (SIZEOF E3D_COLOR)
			dec	ecx
		.ENDW
	.ENDIF

	.ELSE
	; --------------------------------------------------------------------
	;	その他のプリミティブ
	; --------------------------------------------------------------------
	INVOKE	eglRenderPoly@CretaePolygonEntrySSE ,
			hRenderPoly, hStackHeap, pPrimitive
	;
	mov	edx, pdwResult
	mov	DWORD PTR [edx], E3D_RTCPE_RESULT_NO_RAYTRACING
	ret

	.ENDIF

	; --------------------------------------------------------------------
	;	最終処理
	; --------------------------------------------------------------------
	mov	edi, pPolyEntry
	mov	ebx, hRenderPoly
	ASSUME	edi:PE3D_POLYGON_ENTRY
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	;
	; レンダリングモード判定
	;
	xor	eax, eax
	xor	edx, edx
	mov	ecx, [edi].dwShadingFlags
	test	[ebx].rrtpRayParam.dwFlags, E3D_RAYTRACE_ONLY_GLOBAL_REF
	setnz	al
	test	ecx, E3DSAF_GLOBAL_REFLECT_OBJECT
	setz	dl
	neg	eax
	neg	edx
	and	eax, edx
	and	eax, E3DSAF_NO_REFLECT_OBJECT
	or	ecx, eax
	;
	mov	eax, nResult
	mov	edx, eax
	or	edx, E3D_RTCPE_RESULT_NO_SHADOWING
	test	ecx, E3DSAF_NO_SHADOW_OBJECT
	cmovnz	eax, edx
	test	[ebx].rrtpRayParam.dwFlags, E3D_RAYTRACE_SHADOWING
	cmovz	eax, edx
	mov	edx, eax
	or	edx, E3D_RTCPE_RESULT_NO_REFLECTING
	test	ecx, E3DSAF_NO_REFLECT_OBJECT
	cmovnz	eax, edx
	test	[ebx].rrtpRayParam.dwFlags, \
			(E3D_RAYTRACE_REFLECTION OR E3D_RAYTRACE_REFRACTION)
	cmovz	eax, edx
	mov	nResult, eax
	cmp	eax, (E3D_RTCPE_RESULT_SHOULD_RENDER \
			OR E3D_RTCPE_RESULT_NO_SHADOWING \
			OR E3D_RTCPE_RESULT_NO_REFLECTING)
	jz	Label_SuccessfullyExit
	cmp	eax, (E3D_RTCPE_RESULT_NO_RENDER \
			OR E3D_RTCPE_RESULT_NO_SHADOWING \
			OR E3D_RTCPE_RESULT_NO_REFLECTING)
	jz	Label_ErrorExit
	;
	ASSUME	ebx:NOTHING
	ASSUME	edi:NOTHING

	;
	;	終了
	; --------------------------------------------------------------------
Label_SuccessfullyExit:
	mov	edx, pdwResult
	mov	ecx, nResult
	mov	eax, pPolyEntry
	mov	DWORD PTR [edx], ecx
	ret

Label_ErrorExit:
	INVOKE	eslStackHeapLeave , hStackHeap, pPolyEntry
	mov	edx, pdwResult
	xor	eax, eax
	mov	DWORD PTR [edx], eax
	ret

eglRenderPoly@CreatePolygonEntryRT_SSE	ENDP

CodeSeg	ENDS

	END
