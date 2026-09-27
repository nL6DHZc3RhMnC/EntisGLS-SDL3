
; ----------------------------------------------------------------------------
;                          Entis Graphic Library
; ----------------------------------------------------------------------------
;    Copyright (c) 2008 Leshade Entis, Entis-soft. Al rights reserved.
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
xmmSignBit	DWORD	4 DUP( 80000000H )
xmmPacked128	REAL4	4 DUP( 128.0 )
xmmPacked256	REAL4	4 DUP( 256.0 )

mmxConstDW_FFH	LABEL	MMWORD
		DWORD	2 DUP( 0FFH )
mmxConstW_0_0_1_0	LABEL	MMWORD
		WORD	0, 0, 1, 0
mmxConstW_0_1_0_1	LABEL	MMWORD
		WORD	0, 1, 0, 1

ConstSeg	ENDS


; ----------------------------------------------------------------------------
;	コードセグメント
; ----------------------------------------------------------------------------

CodeSeg	SEGMENT	PARA READONLY FLAT 'CODE'

;
;	ポリゴンとの当たり判定
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@Polygons@IsHitSegmentSSE	PROC	NEAR32 C USES ebx esi edi,
	pMesh:PE3D_POLYGON_ENTRY,
	prmhp:PTR EGL_RENDER_MESH_HIT_PARAM,
	pvRay:PE3D_VECTOR4, pvPos:PE3D_VECTOR4

	LOCAL	vRay:E3D_VECTOR4	; 長さ 1.0
	LOCAL	vPos:E3D_VECTOR4
	LOCAL	dwSaveESP:DWORD
	LOCAL	nDistanceMask:DWORD
	LOCAL	nPolyCount:DWORD
	LOCAL	nBaseIndex:DWORD
	LOCAL	nQCount:DWORD		; = (nPolyCount + 3) / 4
	LOCAL	nOddCountMask:DWORD	; = (1 shl (nPolyCount mod 4)) - 1
	LOCAL	nLastMask:DWORD
	LOCAL	nLeftCount:DWORD
	LOCAL	rErrorGap:REAL4
	LOCAL	pNextMeshBlock:PTR EGL_RENDER_POLY_SPHERE_PCK4
	LOCAL	dwTemp[4]:DWORD

	mov	ebx, pMesh
	ASSUME	ebx:PE3D_POLYGON_ENTRY
	mov	eax, pvRay
	mov	edx, pvPos
	mov	esi, [ebx].surface.mesh.pMesh
	ASSUME	esi:PE3D_PRIMITIVE_MESH_LIST
	movups	xmm0, [eax]
	movups	xmm1, [edx]
	mov	ecx, [esi].dwPolyCount
	movups	vRay, xmm0
	movups	vPos, xmm1
	;
	mov	eax, [ebx].surface.mesh.pMeshReserved2
	mov	esi, [ebx].surface.mesh.pMeshReserved
	mov	pNextMeshBlock, eax
	ASSUME	esi:PTR EGL_RENDER_POLY_MATRIX_PCK4
	;
	mov	nPolyCount, ecx
	mov	eax, 1
	and	ecx, 03H
	shl	eax, cl
	mov	edx, -1
	dec	eax
	cmovz	eax, edx
	mov	nOddCountMask, eax
	mov	nLastMask, -1
	;
		mov	eax, pMesh
		xor	edi, edi
	mov	ebx, prmhp
	ASSUME	ebx:PTR EGL_RENDER_MESH_HIT_PARAM
		cmp	eax, [ebx].pExceptingMeshEntry
		jz	Label_Exit
	mov	ecx, nPolyCount
		cmp	eax, [ebx].pExceptingMesh
		cmovz	edi, [ebx].rRangeMin
	add	ecx, 3
	shr	ecx, 2
		mov	rErrorGap, edi
	mov	nQCount, ecx
	;
	sub	ecx, 128/4
	mov	nBaseIndex, 0
	mov	edi, pNextMeshBlock
	ASSUME	edi:PTR EGL_RENDER_POLY_SPHERE_PCK4
	.WHILE	!SIGN?
		;
		; (xmm0, xmm1, xmm2) = Q - vPos
		;
		movss	xmm4, vPos.x
		movss	xmm5, vPos.y
		movss	xmm6, vPos.z
		shufps	xmm4, xmm4, 0
		movaps	xmm0, [edi].xCenter
		shufps	xmm5, xmm5, 0
		movaps	xmm1, [edi].yCenter
		shufps	xmm6, xmm6, 0
		movaps	xmm2, [edi].zCenter
		subps	xmm0, xmm4
			movss	xmm4, vRay.x
		subps	xmm1, xmm5
			movss	xmm5, vRay.y
		subps	xmm2, xmm6
			movss	xmm6, vRay.z
		;
		; xmm0 = q^2 = |Q - vPos|^2
		; xmm4 = r = <Q - vPos, vRay>
		;
		shufps	xmm4, xmm4, 0
		shufps	xmm5, xmm5, 0
		shufps	xmm6, xmm6, 0
		mulps	xmm4, xmm0
			mulps	xmm0, xmm0
		mulps	xmm5, xmm1
			mulps	xmm1, xmm1
		mulps	xmm6, xmm2
			mulps	xmm2, xmm2
		addps	xmm4, xmm5
			movss	xmm3, [ebx].rRangeMax
			addps	xmm0, xmm1
		addps	xmm4, xmm6
			movss	xmm7, [ebx].rRangeMin
			addps	xmm0, xmm2
		;
		; xmm3 ?= (rRangeMax + rRadius > r)
		;	 && (rRangeMin - rRadius < r)
		;
		movaps	xmm2, [edi].rRadius
		shufps	xmm3, xmm3, 0
		shufps	xmm7, xmm7, 0
			movaps	xmm5, xmm4
			mulps	xmm4, xmm4	; xmm4 = r^2
		addps	xmm3, xmm2
		subps	xmm7, xmm2
			mulps	xmm2, xmm2	; xmm2 = rRadius^2
		cmpps	xmm3, xmm5, 6
		cmpps	xmm7, xmm5, 1
		andps	xmm3, xmm7
			subps	xmm0, xmm4
		movmskps	eax, xmm3
		test	eax, eax
		jz	Label_ContinueBlock
		;
		; xmm3 ?= q^2 - r^2 < rRadius^2
		;
		cmpps	xmm0, xmm2, 1
		;
		push	ecx
		push	esi
		push	edi
		push	nBaseIndex
		;
		movmskps	edx, xmm0
		and	edx, eax
		;
		.WHILE	!ZERO?
			.IF	edx & 1
				push	edx
				mov	ecx, 32/4
				call	SubFubc_HitSegment
				pop	edx
			.ELSE
				add	esi, (SIZEOF EGL_RENDER_POLY_MATRIX_PCK4) * (32/4)
			.ENDIF
			add	nBaseIndex, 32
			shr	edx, 1
		.ENDW
		;
		pop	nBaseIndex
		pop	edi
		pop	esi
		pop	ecx
		;
Label_ContinueBlock:
		add	nBaseIndex, 128
		add	edi, (SIZEOF EGL_RENDER_POLY_SPHERE_PCK4)
		add	esi, (SIZEOF EGL_RENDER_POLY_MATRIX_PCK4) * (128/4)
		sub	ecx, 128/4
	.ENDW
	ASSUME	edi:NOTHING
	;
	mov	eax, nOddCountMask
	add	ecx, 128/4
	mov	nLastMask, eax
	;
	call	SubFubc_HitSegment
	;
Label_Exit:
	ret


ALIGN	10H
SubFubc_HitSegment:
	mov	dwSaveESP, esp
	sub	esp, 80H
	mov	nQCount, ecx
	and	esp, NOT 0FH
	test	ecx, ecx
	.WHILE	!ZERO?
		;
		; (xmm3, xmm4, xmm5) = P - vPos
		;
		movss	xmm0, vPos.x
		movss	xmm1, vPos.y
		movss	xmm2, vPos.z
		shufps	xmm0, xmm0, 0
		movaps	xmm3, [esi].xVertexA
		shufps	xmm1, xmm1, 0
		movaps	xmm4, [esi].yVertexA
		shufps	xmm2, xmm2, 0
		movaps	xmm5, [esi].zVertexA
		subps	xmm3, xmm0
			movss	xmm0, vRay.x
		subps	xmm4, xmm1
			movss	xmm1, vRay.y
		subps	xmm5, xmm2
			movss	xmm2, vRay.z
		;
		; xmm0 = <P - vPos | vRay>
		; xmm3 = |P - vPos|^2
		;
		shufps	xmm0, xmm0, 0
		shufps	xmm1, xmm1, 0
		shufps	xmm2, xmm2, 0
		mulps	xmm0, xmm3
		movaps	[esp], xmm3
			mulps	xmm3, xmm3
		mulps	xmm1, xmm4
		movaps	[esp + 10H], xmm4
			mulps	xmm4, xmm4
		mulps	xmm2, xmm5
		movaps	[esp + 20H], xmm5
			mulps	xmm5, xmm5
		addps	xmm0, xmm1
				movss	xmm1, vRay.x
			addps	xmm3, xmm4
				movss	xmm4, vRay.z
		addps	xmm0, xmm2
				movss	xmm2, vRay.y
			addps	xmm3, xmm5
				movaps	xmm5, [esi].xNormal
		;
		; xmm3 = |P - vPos|^2 - <P - vPos | vRay>^2 <= max_ab_ac_sqr
		;
		mulps	xmm0, xmm0
				shufps	xmm1, xmm1, 0
				movaps	xmm6, [esi].yNormal
				shufps	xmm2, xmm2, 0
				movaps	xmm7, [esi].zNormal
			mov	eax, -1
			cmp	ecx, 1
		subps	xmm3, xmm0
				shufps	xmm4, xmm4, 0
				mulps	xmm1, xmm5
					mulps	xmm5, [esp]
			cmovz	eax, nLastMask
		cmpps	xmm3, [esi].max_ab_ac_sqr, 2
				mulps	xmm2, xmm6
					mulps	xmm6, [esp + 10H]
		movmskps	edi, xmm3
				mulps	xmm4, xmm7
					mulps	xmm7, [esp + 20H]
		and	edi, eax
		jz	Label_Continue
		;mov	nDistanceMask, edi
		;
		; xmm4 = <vRay | A>
		; xmm5 = <P - vPos | A>
		;
		addps	xmm1, xmm2
			addps	xmm5, xmm6
		addps	xmm4, xmm1
			addps	xmm5, xmm7
		;
		; xmm5 = distance
		;
		rcpps	xmm6, xmm4		; xmm4 = 1 / xmm4
			movss	xmm0, rErrorGap ; [ebx].rRangeMin
			movss	xmm7, [ebx].rRangeMax
		mulps	xmm4, xmm6
			shufps	xmm0, xmm0, 0
				movss	xmm1, vRay.x
		mulps	xmm5, xmm6
			shufps	xmm7, xmm7, 0
				movss	xmm2, vRay.y
		rcpps	xmm4, xmm4
				movss	xmm3, vRay.z
				shufps	xmm1, xmm1, 0
		;mulps	xmm4, xmm6
		mulps	xmm5, xmm4
				shufps	xmm2, xmm2, 0
				shufps	xmm3, xmm3, 0
		;mulps	xmm5, xmm4
				movss	xmm4, vPos.x
		;
		cmpps	xmm0, xmm5, 2	; ?= (min <= distance)
		cmpps	xmm7, xmm5, 6	; ?= (max > distance)
		andps	xmm7, xmm0
			movaps	xmm0, xmm5
		movmskps	eax, xmm7
		and	edi, eax
		jz	Label_Continue
		;mov	nDistanceMask, edi
		;
		; (xmm1, xmm2, xmm3) = P = 平面と光線の交点
		;
		mulps	xmm1, xmm0
			movss	xmm5, vPos.y
			movss	xmm6, vPos.z
		mulps	xmm2, xmm0
			shufps	xmm4, xmm4, 0
		mulps	xmm3, xmm0
			shufps	xmm5, xmm5, 0
			shufps	xmm6, xmm6, 0
		;movaps	[esp + 30H], xmm0
		addps	xmm1, xmm4
		addps	xmm2, xmm5
		addps	xmm3, xmm6
		;
		; (xmm4, xmm5, xmm6) = (P - B) / |P - B|
		;
			movaps	[esp], xmm1
		subps	xmm1, [esi].xVertexB
			movaps	[esp + 10H], xmm2
		subps	xmm2, [esi].yVertexB
			movaps	[esp + 20H], xmm3
		subps	xmm3, [esi].zVertexB
			movaps	xmm4, xmm1
		mulps	xmm1, xmm1
			movaps	xmm5, xmm2
		mulps	xmm2, xmm2
			movaps	xmm6, xmm3
		mulps	xmm3, xmm3
		addps	xmm2, xmm1
			movaps	xmm1, xmm4
			mulps	xmm4, [esi].xAB
		addps	xmm3, xmm2
			movaps	xmm2, xmm5
			mulps	xmm5, [esi].yAB
		sqrtps	xmm7, xmm3
		;
		; cos(∠PBA) = <P - B | A - B> / (|P - B| * |A - B|)
		; cos(∠PBC) = <P - B | C - B> / (|P - B| * |C - B|)
		;
		movaps	xmm3, xmm6
		mulps	xmm6, [esi].zAB
			mulps	xmm1, [esi].xBC
			mulps	xmm2, [esi].yBC
			mulps	xmm3, [esi].zBC
		addps	xmm4, xmm5
		mulps	xmm7, [esi].cos_abc
		addps	xmm4, xmm6
			addps	xmm1, xmm2
			addps	xmm1, xmm3
		subps	xmm7, [esi].error_gap
			xorps	xmm1, xmmSignBit
		cmpps	xmm4, xmm7, 5	; ?= cos(∠PBA) ≧ cos(∠ABC)
		cmpps	xmm1, xmm7, 5	; ?= cos(∠PBC) ≧ cos(∠ABC)
		andps	xmm1, xmm4
		movmskps	eax, xmm1
		;mov	edi, nDistanceMask
		and	edi, eax
		jz	Label_Continue
		;
		; (xmm4, xmm5, xmm6) = (P - C) / |P - C|
		;
		movaps	xmm1, [esp]
		movaps	xmm2, [esp + 10H]
		movaps	xmm3, [esp + 20H]
		subps	xmm1, [esi].xVertexC
		subps	xmm2, [esi].yVertexC
		subps	xmm3, [esi].zVertexC
			movaps	xmm4, xmm1
		mulps	xmm1, xmm1
			movaps	xmm5, xmm2
		mulps	xmm2, xmm2
			movaps	xmm6, xmm3
		mulps	xmm3, xmm3
		addps	xmm2, xmm1
			movaps	xmm1, xmm4
			mulps	xmm4, [esi].xAC
		addps	xmm3, xmm2
			movaps	xmm2, xmm5
			mulps	xmm5, [esi].yAC
		sqrtps	xmm7, xmm3
		;
		; cos(∠PCA) = <P - C | A - C> / (|P - C| * |A - C|)
		; cos(∠PCB) = <P - C | B - C> / (|P - C| * |B - C|)
		;
		movaps	xmm3, xmm6
		mulps	xmm6, [esi].zAC
			mulps	xmm1, [esi].xBC
			mulps	xmm2, [esi].yBC
			mulps	xmm3, [esi].zBC
		addps	xmm4, xmm5
		mulps	xmm7, [esi].cos_acb
		addps	xmm4, xmm6
			addps	xmm1, xmm2
		subps	xmm7, [esi].error_gap
			addps	xmm1, xmm3
		cmpps	xmm4, xmm7, 5	; ?= cos(∠PCA) ≧ cos(∠ACB)
		cmpps	xmm1, xmm7, 5	; ?= cos(∠PCB) ≧ cos(∠ACB)
		andps	xmm1, xmm4
		movmskps	eax, xmm1
		;mov	edi, nDistanceMask
		and	edi, eax
		jz	Label_Continue
		;mov	nDistanceMask, edi
		;
		; 最近ポリゴンを更新
		;
		mov	eax, nQCount
		mov	edx, pMesh
		mov	nLeftCount, ecx
		sub	eax, ecx
		xor	ecx, ecx
		shl	eax, 2
		add	eax, nBaseIndex
		.REPEAT
			.IF	(edi & 1) && \
				(([ebx].pExceptingMesh != edx) \
					|| ([ebx].nExceptingMeshIndex != eax))
			comiss	xmm0, [ebx].rRangeMax
			.IF	CARRY?
				mov	dwTemp[0], esi
				mov	dwTemp[4], edi
				mov	esi, [ebx].nResultCount
				imul	edi, esi, (SIZEOF EGL_RENDER_MESH_HIT_ENTRY)
				test	esi, esi
				.IF	!ZERO?
					comiss	xmm0, [ebx].rmheResults[edi - (SIZEOF EGL_RENDER_MESH_HIT_ENTRY)].rDistance
					.IF	CARRY?
						.IF	esi < [ebx].nResultLimit
							movups	xmm1, XMMWORD_PTR [ebx].rmheResults[edi - (SIZEOF EGL_RENDER_MESH_HIT_ENTRY)]
							movlps	xmm2, QWORD PTR [ebx].rmheResults[edi - (SIZEOF EGL_RENDER_MESH_HIT_ENTRY) + 16]
							movups	XMMWORD_PTR [ebx].rmheResults[edi], xmm1
							movlps	QWORD PTR [ebx].rmheResults[edi + 16], xmm2
							inc	[ebx].nResultCount
						.ENDIF
						sub	edi, (SIZEOF EGL_RENDER_MESH_HIT_ENTRY)
						dec	esi
						.WHILE	!ZERO?
							comiss	xmm0, [ebx].rmheResults[edi - (SIZEOF EGL_RENDER_MESH_HIT_ENTRY)].rDistance
							.BREAK	.IF	!CARRY?
							movups	xmm1, XMMWORD_PTR [ebx].rmheResults[edi - (SIZEOF EGL_RENDER_MESH_HIT_ENTRY)]
							movlps	xmm2, QWORD PTR [ebx].rmheResults[edi - (SIZEOF EGL_RENDER_MESH_HIT_ENTRY) + 16]
							movups	XMMWORD_PTR [ebx].rmheResults[edi], xmm1
							movlps	QWORD PTR [ebx].rmheResults[edi + 16], xmm2
							sub	edi, (SIZEOF EGL_RENDER_MESH_HIT_ENTRY)
							dec	esi
						.ENDW
						jmp	Label_InsertResult
					.ENDIF
				.ENDIF
				.IF	esi < [ebx].nResultLimit
Label_InsertResult:			;
					movss	xmm1, REAL4 PTR [esp + ecx*4]
					movss	xmm2, REAL4 PTR [esp + ecx*4 + 10H]
					movss	xmm3, REAL4 PTR [esp + ecx*4 + 20H]
					movss	[ebx].rmheResults[edi].rDistance, xmm0
					mov	[ebx].rmheResults[edi].pHitMesh, edx
					mov	[ebx].rmheResults[edi].nHitMeshIndex, eax
					movss	[ebx].rmheResults[edi].vHitPosition.x, xmm1
					movss	[ebx].rmheResults[edi].vHitPosition.y, xmm2
					movss	[ebx].rmheResults[edi].vHitPosition.z, xmm3
					;
					.IF	esi == [ebx].nResultCount
						inc	esi
						mov	[ebx].nResultCount, esi
					.ENDIF
				.ENDIF
				mov	esi, dwTemp[0]
				mov	edi, dwTemp[4]
			.ENDIF
			.ENDIF
			shufps	xmm0, xmm0, 00111001B
			inc	ecx
			inc	eax
			shr	edi, 1
		.UNTIL	ZERO?
		mov	ecx, nLeftCount
Label_Continue:
		add	esi, (SIZEOF EGL_RENDER_POLY_MATRIX_PCK4)
		dec	ecx
	.ENDW

	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	mov	esp, dwSaveESP
	BYTE	0C3H	; ret

eglRenderPoly@Polygons@IsHitSegmentSSE	ENDP

;
;	光線とポリゴンの交差点の情報を取得する
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@Polygon@GetHitPointSurface	PROC	NEAR32 C USES ebx esi edi,
	hRenderPoly:HEGL_RENDER_POLYGON,
	prmhs:PTR EGL_RENDER_MESH_HIT_SURFACE,
	prmhe:PTR EGL_RENDER_MESH_HIT_ENTRY

	mov	esi, prmhe
	mov	edi, prmhs
	ASSUME	esi:PTR EGL_RENDER_MESH_HIT_ENTRY
	ASSUME	edi:PTR EGL_RENDER_MESH_HIT_SURFACE

	movups	xmm7, XMMWORD_PTR [esi].vHitPosition	; xmm7 = vHitPos
	mov	eax, [esi].nHitMeshIndex
	mov	esi, [esi].pHitMesh
	shl	eax, 4
	ASSUME	esi:PE3D_POLYGON_ENTRY
	mov	ebx, [esi].surface.mesh.pMesh
	lea	ebx, (E3D_PRIMITIVE_MESH_LIST PTR [ebx]).mpEntries[eax]
	ASSUME	ebx:PTR E3D_PRIMITIVE_MESH_POLY
	;
	; A = V1 - V0, B = V2 - V0, C = A * B
	; X = vHitPos
	; P = X - V0
	;
	mov	eax, [ebx].dwIndex[0]
	mov	ecx, [ebx].dwIndex[4]
	mov	edx, [ebx].dwIndex[8]
	shl	eax, 4
	shl	ecx, 4
	shl	edx, 4
	sub	ecx, eax
	sub	edx, eax
	add	eax, [esi].pVertexes
	;
	movaps	xmm6, [eax]		; xmm6 = V0
	movaps	xmm4, [eax + ecx]	; xmm4 = A = V1 - V0
	movaps	xmm5, [eax + edx]	; xmm5 = B = V2 - V0
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
	movups	[edi].vPlane, xmm0
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
	movaps	xmm3, [eax]
			mov	eax, [ebx].dwIndex[0]
			mov	ecx, [ebx].dwIndex[4]
			mov	edx, [ebx].dwIndex[8]
	subps	xmm7, xmm3		; xmm7 = P = X - V0
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
			shl	eax, 4
			shl	ecx, 4
			shl	edx, 4
	subps	xmm0, xmm4		; xmm0 = A * P
		subps	xmm2, xmm5	; xmm2 = P * B
			sub	ecx, eax
			sub	edx, eax
			add	eax, [esi].pNormals
	;
	mulps	xmm0, xmm0
		mulps	xmm2, xmm2
			.IF	[esi].pNormals != NULL
				movaps	xmm3, [eax]	; xmm3 = normal0
				movaps	xmm6, [eax + ecx]
				movaps	xmm7, [eax + edx]
			.ELSE
				sub	eax, [esi].pNormals
				add	eax, [esi].pVertexes
				movaps	xmm3, [eax]	; xmm3 = normal0
				movaps	xmm6, [eax + ecx]
				movaps	xmm7, [eax + edx]
				subps	xmm6, xmm3
				subps	xmm7, xmm3
				movaps	xmm3, xmm6
				movaps	xmm4, xmm7
				shufps	xmm6, xmm6, 11010010B
				shufps	xmm7, xmm7, 11001001B
				shufps	xmm3, xmm3, 11001001B
				shufps	xmm4, xmm4, 11010010B
				mulps	xmm6, xmm7
				mulps	xmm3, xmm4
				subps	xmm3, xmm6
				movaps	xmm6, xmm3
				movaps	xmm7, xmm3
			.ENDIF
	movhlps	xmm4, xmm0
		movhlps	xmm5, xmm2
			mov	eax, [ebx].dwIndex[0]
			mov	ecx, [ebx].dwIndex[4]
			mov	edx, [ebx].dwIndex[8]
	addss	xmm4, xmm0
	shufps	xmm0, xmm0, 1
			mov	ebx, [esi].pVertexColors
			ASSUME	ebx:NOTHING
			subps	xmm6, xmm3	; xmm6 = normal1 - normal0
			subps	xmm7, xmm3	; xmm7 = normal2 - normal0
		addss	xmm5, xmm2
		shufps	xmm2, xmm2, 1
	addss	xmm0, xmm4		; xmm0 = |A * P|^2
		addss	xmm2, xmm5	; xmm2 = |P * B|^2
			pxor	mm4, mm4
			movd	mm6, DWORD PTR [ebx + eax * 8]
			movd	mm7, DWORD PTR [ebx + eax * 8 + 4]
			movd	mm0, DWORD PTR [ebx + ecx * 8]
			movd	mm1, DWORD PTR [ebx + ecx * 8 + 4]
			movd	mm2, DWORD PTR [ebx + edx * 8]
			movd	mm3, DWORD PTR [ebx + edx * 8 + 4]
	mulss	xmm0, xmm1
		mulss	xmm2, xmm1
			mov	ebx, [esi].surface.mesh.pUVMap
			punpcklbw	mm6, mm4
			punpcklbw	mm7, mm4
			punpcklbw	mm0, mm4
			punpcklbw	mm1, mm4
			punpcklbw	mm2, mm4
			punpcklbw	mm3, mm4
			xorps	xmm4, xmm4
			xorps	xmm5, xmm5
	sqrtss	xmm0, xmm0		; xmm0 = v
		sqrtss	xmm2, xmm2	; xmm2 = u
			psubw	mm0, mm6
			psubw	mm1, mm7
			psubw	mm2, mm6
			psubw	mm3, mm7
			.IF	ebx != NULL
				movlps	xmm4, QWORD PTR [ebx + ecx * 8]
				movlps	xmm5, QWORD PTR [ebx + eax * 8]
				movhps	xmm4, QWORD PTR [ebx + edx * 8]
				movlhps	xmm5, xmm5
				subps	xmm4, xmm5
			.ENDIF
	;
	movss	[edi].uv.y, xmm0
	shufps	xmm0, xmm0, 0
	movss	[edi].uv.x, xmm2
	shufps	xmm2, xmm2, 0
	;
	; 法線・テクスチャUV・色補間
	;
	mulps	xmm7, xmm0
	mulps	xmm6, xmm2
		movaps		xmm1, xmmPacked128
		unpcklps	xmm2, xmm0
		mulps		xmm1, xmm2
		unpcklps	xmm2, xmm2
	addps	xmm6, xmm7
		cvtps2pi	mm4, xmm1
		mulps		xmm4, xmm2
	addps	xmm3, xmm6
			movq	mm5, mm4
			pshufw	mm4, mm4, 00000000B
			pshufw	mm5, mm5, 10101010B
			pmullw	mm0, mm4
				movups	xmm6, [edi].vPlane
			pmullw	mm1, mm4
	movaps	xmm0, xmm3
	mulps	xmm3, xmm3
		movhlps		xmm2, xmm4
				mulps	xmm6, xmm0
		addps		xmm4, xmm5
			pmullw	mm2, mm5
			pmullw	mm3, mm5
	movhlps	xmm1, xmm3
			psraw	mm0, 7
				movhlps	xmm7, xmm6
			psraw	mm1, 7
		addps		xmm4, xmm2
	addss	xmm1, xmm3
	shufps	xmm3, xmm3, 1
				addss	xmm7, xmm6
				shufps	xmm6, xmm6, 1
			psraw	mm2, 7
			psraw	mm3, 7
			paddsw	mm6, mm0
			paddsw	mm7, mm1
	addss	xmm3, xmm1
				addss	xmm6, xmm7
				movups	xmm7, [edi].vPlane
			paddsw	mm6, mm2
			paddsw	mm7, mm3
	rsqrtss	xmm3, xmm3
				shufps	xmm6, xmm6, 0
		movlps	QWORD PTR [edi].uvTexture, xmm4
			packuswb	mm6, mm7
				andps	xmm6, xmmSignBit
	mov	ebx, hRenderPoly
	shufps	xmm3, xmm3, 0
				xorps	xmm6, xmm7
	mov	eax, [esi].dwShadingFlags
	mov	[edi].rgbaTexture.dwPixelCode, 0
	mov	ecx, eax
	and	eax, (EGL_RENDER_POLYGON_BUF PTR [ebx]).dwMaskShadingFlags
	mulps	xmm3, xmm0
				movups	[edi].vPlane, xmm6
	or	eax, (EGL_RENDER_POLYGON_BUF PTR [ebx]).dwAddShadingFlags
			movq	MMWORD PTR [edi].clrSurface, mm6
	mov	edx, eax
	and	eax, NOT E3DSAF_SHADING_MASK
	test	ecx, E3DSAF_SHADING_MASK
	cmovnz	eax, edx
	movups	[edi].vNormal, xmm3
	mov	[edi].dwShadingFlags, eax
	;
	; テクスチャピクセルサンプリング
	;
	mov	edx, [esi].pAttr
	mov	[edi].rgbaTexture.dwPixelCode, 0
	mov	eax, (E3D_SURFACE_ATTRIBUTE PTR [edx]).nDeepness
	mov	ecx, (E3D_SURFACE_ATTRIBUTE PTR [edx]).nTransparency
	mov	[edi].pAttr, edx
	mov	[edi].nDeepness, eax
	mov	eax, [edi].dwShadingFlags
	mov	[edi].nTransparency, ecx
	;
	.IF	eax & E3DSAF_TEXTURE_MAPPING
	.IF	eax & E3DSAF_TEXTURE_SMOOTH
		mulps		xmm4, xmmPacked256
		mov	edx, (E3D_SURFACE_ATTRIBUTE PTR [edx]).txmap.pTextureImage
		ASSUME	edx:PTR EGL_IMAGE_BUFF
		.IF	(edx != NULL) && \
				([edx].dwInfoSize == (SIZEOF EGL_IMAGE_BUFF))
			movd	mm6, [edx].dwBitsPerPixel
			movd	mm7, [edx].dwBytesPerLine
			cvtps2pi	mm0, xmm4
			.IF	eax & E3DSAF_TEXTURE_TILING
					movq		mm5, MMWORD PTR [edx].dwWidthMask
					psrld		mm6, 3
				movq		mm2, mm0
				psrad		mm0, 8
					packssdw	mm5, mm5
				packssdw	mm0, mm0
					punpcklwd	mm6, mm7
				paddw		mm0, mmxConstW_0_0_1_0
				movq		mm1, mm0
					punpckldq	mm6, mm6
				pand		mm0, mm5
				paddw		mm1, mmxConstW_0_1_0_1
				pmaddwd		mm0, mm6
				pand		mm1, mm5
				movd		mm7, [edx].ptrImageArray
				pmaddwd		mm1, mm6
				punpckldq	mm7, mm7
			.ELSE
					pcmpeqd		mm4, mm4
					movq		mm5, MMWORD PTR [edx].dwImageWidth
				movq		mm2, mm0
				psrad		mm0, 8
					paddd		mm5, mm4
				packssdw	mm0, mm0
					pxor		mm5, mm4
				paddw		mm0, mmxConstW_0_0_1_0
					psrld		mm6, 3
					packssdw	mm5, mm5
				movq		mm3, mm0
				movq		mm1, mm0
					punpcklwd	mm6, mm7
				paddusw		mm3, mm5
				psraw		mm0, 15
					punpckldq	mm6, mm6
				psubusw		mm3, mm5
				paddw		mm1, mmxConstW_0_1_0_1
				pandn		mm0, mm3
				;
				movq		mm3, mm1
				pmaddwd		mm0, mm6
				paddusw		mm3, mm5
				psraw		mm1, 15
				movd		mm7, [edx].ptrImageArray
				psubusw		mm3, mm5
				pandn		mm1, mm3
				punpckldq	mm7, mm7
				pmaddwd		mm1, mm6
			.ENDIF
			;
			paddd		mm0, mm7	; addr px00 : px01
			movd		eax, mm0
			psrlq		mm0, 32
			paddd		mm1, mm7	; addr px10 : px11
			movd		ecx, mm0
			;
			pand		mm2, mmxConstDW_FFH
			pxor		mm7, mm7
			movd		mm4, DWORD PTR [eax]
			movd		eax, mm1
			psrlq		mm1, 32
			movd		mm5, DWORD PTR [ecx]
			movd		ecx, mm1
			punpcklbw	mm4, mm7	; px00
			psrld		mm2, 1
			movd		mm0, DWORD PTR [eax]
			movd		mm1, DWORD PTR [ecx]
			punpcklbw	mm5, mm7	; px01
			punpcklbw	mm0, mm7	; px10
			punpcklbw	mm1, mm7	; px11
			;
			pshufw		mm3, mm2, 00000000B
			psubw		mm5, mm4
			psubw		mm1, mm0
			pmullw		mm5, mm3
			pmullw		mm1, mm3
				mov	ecx, 0FF000000H
				xor	eax, eax
			pshufw		mm2, mm2, 10101010B
			psraw		mm5, 7
			psraw		mm1, 7
			paddw		mm4, mm5
			paddw		mm0, mm1
			;
			psubw		mm0, mm4
			pmullw		mm0, mm2
				test	[edx].fdwFormatType, EIF_WITH_ALPHA
				cmovz	eax, ecx
			psraw		mm0, 7
				movd	mm1, eax
			paddw		mm0, mm4
			packuswb	mm0, mm0
			;
			por		mm0, mm1
			movq		mm1, mm0
			movd		ecx, mm0
			psrad		mm0, 31
			movq		mm2, mm0
			pand		mm0, mm1
			pslld		mm2, 24
			por		mm0, mm2
			;
			test	[edi].dwShadingFlags, E3DSAF_TEXTURE_TRIM
			movd	eax, mm0
			cmovz	eax, ecx
			mov	[edi].rgbaTexture.dwPixelCode, eax
		.ENDIF
	.ELSE
		cvtps2pi	mm4, xmm4
		mov	edx, (E3D_SURFACE_ATTRIBUTE PTR [edx]).txmap.pTextureImage
		ASSUME	edx:PTR EGL_IMAGE_BUFF
		.IF	(edx != NULL) && \
				([edx].dwInfoSize == (SIZEOF EGL_IMAGE_BUFF))
			movd	mm6, [edx].dwBitsPerPixel
			movd	mm7, [edx].dwBytesPerLine
			.IF	eax & E3DSAF_TEXTURE_TILING
				pand	mm4, MMWORD PTR [edx].dwWidthMask
					psrld	mm6, 3
				packssdw	mm4, mm4
					punpcklwd	mm6, mm7
				pmaddwd	mm4, mm6
			.ELSE
				pcmpeqd	mm1, mm1
				movq	mm0, MMWORD PTR [edx].dwImageWidth
				paddd	mm0, mm1
					packssdw	mm4, mm4
				pxor	mm0, mm1
					movq	mm5, mm4
				packssdw	mm0, mm0
					paddusw	mm5, mm0
					psraw	mm4, 15
					psubusw	mm5, mm0
						psrld	mm6, 3
				pandn	mm4, mm5
						punpcklwd	mm6, mm7
				pmaddwd	mm4, mm6
			.ENDIF
			movd	eax, mm4
			add	eax, [edx].ptrImageArray
			mov	ecx, 0FF000000H
			mov	eax, DWORD PTR [eax]
			or	ecx, eax
			test	[edx].fdwFormatType, EIF_WITH_ALPHA
			cmovz	eax, ecx
			mov	edx, 0FF000000H
			mov	ecx, eax
			sar	eax, 31
			and	edx, eax
			and	eax, ecx
			or	eax, edx
			test	[edi].dwShadingFlags, E3DSAF_TEXTURE_TRIM
			cmovz	eax, ecx
			mov	[edi].rgbaTexture.dwPixelCode, eax
		.ENDIF
	.ENDIF
	.ENDIF

	ASSUME	esi:NOTHING
	ASSUME	edi:NOTHING
	ret

eglRenderPoly@Polygon@GetHitPointSurface	ENDP

;
;	メッシュとの当たり判定
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@MeshArray@IsHitSegmentSSE	PROC	NEAR32 C USES ebx esi edi,
	prmmpMesh:PEGL_RENDER_MESH_MATRIX_PCK4, nCount:DWORD,
	prmhp:PTR EGL_RENDER_MESH_HIT_PARAM,
	pvRay:PE3D_VECTOR4, pvPos:PE3D_VECTOR4

	LOCAL	vRay:E3D_VECTOR4	; 長さ 1.0
	LOCAL	vPos:E3D_VECTOR4
	LOCAL	rDistance[4]:REAL4
	LOCAL	nQCount:DWORD
	LOCAL	nLastMask:DWORD
	LOCAL	nLeftCount:DWORD

	mov	eax, pvRay
	mov	edx, pvPos
	movups	xmm0, [eax]
	movups	xmm1, XMMWORD_PTR [edx]
	movups	vRay, xmm0
	movups	vPos, xmm1

	mov	ecx, nCount
	mov	eax, 1
	and	ecx, 03H
	shl	eax, cl
	dec	eax
	mov	nLastMask, eax

	mov	ebx, prmhp
	ASSUME	ebx:PTR EGL_RENDER_MESH_HIT_PARAM
	mov	esi, prmmpMesh
	ASSUME	esi:PEGL_RENDER_MESH_MATRIX_PCK4
	mov	ecx, nCount
	add	ecx, 3
	mov	[ebx].nResultCount, 0
	shr	ecx, 2
	mov	nQCount, ecx
	.WHILE	!ZERO?
		;
		; (xmm0, xmm1, xmm2) = Q - vPos
		;
		movss	xmm4, vPos.x
		movss	xmm5, vPos.y
		movss	xmm6, vPos.z
		shufps	xmm4, xmm4, 0
		movaps	xmm0, [esi].xCenter
		shufps	xmm5, xmm5, 0
		movaps	xmm1, [esi].yCenter
		shufps	xmm6, xmm6, 0
		movaps	xmm2, [esi].zCenter
		subps	xmm0, xmm4
			movss	xmm4, vRay.x
		subps	xmm1, xmm5
			movss	xmm5, vRay.y
		subps	xmm2, xmm6
			movss	xmm6, vRay.z
		;
		; xmm0 = q^2 = |Q - vPos|^2
		; xmm4 = r = <Q - vPos, vRay>
		;
		shufps	xmm4, xmm4, 0
		shufps	xmm5, xmm5, 0
		shufps	xmm6, xmm6, 0
		mulps	xmm4, xmm0
			mulps	xmm0, xmm0
		mulps	xmm5, xmm1
			mulps	xmm1, xmm1
		mulps	xmm6, xmm2
			mulps	xmm2, xmm2
		addps	xmm4, xmm5
			movss	xmm3, [ebx].rRangeMax
			addps	xmm0, xmm1
		addps	xmm4, xmm6
			movss	xmm7, [ebx].rRangeMin
			addps	xmm0, xmm2
		;
		; xmm3 ?= (rRangeMax + rRadius > r)
		;	 && (rRangeMin - rRadius < r)
		;
		movaps	xmm2, [esi].rRadius
		shufps	xmm3, xmm3, 0
		shufps	xmm7, xmm7, 0
			movaps	xmm5, xmm4
			mulps	xmm4, xmm4	; xmm4 = r^2
		addps	xmm3, xmm2
		subps	xmm7, xmm2
			mulps	xmm2, xmm2	; xmm2 = rRadius^2
		cmpps	xmm3, xmm5, 6
		cmpps	xmm7, xmm5, 1
		andps	xmm3, xmm7
			subps	xmm0, xmm4
		mov	eax, -1
		cmp	ecx, 1
		cmovz	eax, nLastMask
		movmskps	edi, xmm3
		and	edi, eax
		jz	Label_Continue
		;
		; xmm3 ?= q^2 - r^2 < rRadius^2
		;
		cmpps	xmm0, xmm2, 1
		mov	nLeftCount, ecx
		xor	ecx, ecx
		movmskps	eax, xmm0
		and	edi, eax
		.WHILE	!ZERO?
			.IF	(edi & 1) && ([esi].pMesh[ecx*4] != NULL)
				push	ecx
				INVOKE	eglRenderPoly@Polygons@IsHitSegmentSSE,
						[esi].pMesh[ecx*4],
						ebx, pvRay, pvPos
				pop	ecx
			.ENDIF
			inc	ecx
			shr	edi, 1
		.ENDW
		mov	ecx, nLeftCount
Label_Continue:
		add	esi, (SIZEOF EGL_RENDER_MESH_MATRIX_PCK4)
		dec	ecx
	.ENDW

	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	ret

eglRenderPoly@MeshArray@IsHitSegmentSSE	ENDP


ALIGN	10H
eglRenderPoly@MeshList@IsHitSegmentSSE	PROC	NEAR32 C USES ebx esi edi,
	prmmpMesh:PTR PEGL_RENDER_MESH_MATRIX_PCK4, nQCount:DWORD,
	prmhp:PTR EGL_RENDER_MESH_HIT_PARAM,
	pvRay:PE3D_VECTOR4, pvPos:PE3D_VECTOR4

	LOCAL	vRay:E3D_VECTOR4	; 長さ 1.0
	LOCAL	vPos:E3D_VECTOR4
	LOCAL	rDistance[4]:REAL4
	LOCAL	nLeftCount:DWORD

	mov	eax, pvRay
	mov	edx, pvPos
	movups	xmm0, [eax]
	movups	xmm1, XMMWORD_PTR [edx]
	movups	vRay, xmm0
	movups	vPos, xmm1

	mov	ebx, prmhp
	ASSUME	ebx:PTR EGL_RENDER_MESH_HIT_PARAM
	mov	esi, prmmpMesh
	ASSUME	esi:PTR PEGL_RENDER_MESH_MATRIX_PCK4
	mov	ecx, nQCount
	mov	[ebx].nResultCount, 0
	test	ecx, ecx
	.WHILE	!ZERO?
		push	esi
		mov	esi, [esi]
		ASSUME	esi:PEGL_RENDER_MESH_MATRIX_PCK4
		;
		; (xmm0, xmm1, xmm2) = Q - vPos
		;
		movss	xmm4, vPos.x
		movss	xmm5, vPos.y
		movss	xmm6, vPos.z
		shufps	xmm4, xmm4, 0
		movaps	xmm0, [esi].xCenter
		shufps	xmm5, xmm5, 0
		movaps	xmm1, [esi].yCenter
		shufps	xmm6, xmm6, 0
		movaps	xmm2, [esi].zCenter
		subps	xmm0, xmm4
			movss	xmm4, vRay.x
		subps	xmm1, xmm5
			movss	xmm5, vRay.y
		subps	xmm2, xmm6
			movss	xmm6, vRay.z
		;
		; xmm0 = q^2 = |Q - vPos|^2
		; xmm4 = r = <Q - vPos, vRay>
		;
		shufps	xmm4, xmm4, 0
		shufps	xmm5, xmm5, 0
		shufps	xmm6, xmm6, 0
		mulps	xmm4, xmm0
			mulps	xmm0, xmm0
		mulps	xmm5, xmm1
			mulps	xmm1, xmm1
		mulps	xmm6, xmm2
			mulps	xmm2, xmm2
		addps	xmm4, xmm5
			movss	xmm3, [ebx].rRangeMax
			addps	xmm0, xmm1
		addps	xmm4, xmm6
			movss	xmm7, [ebx].rRangeMin
			addps	xmm0, xmm2
		;
		; xmm3 ?= (rRangeMax + rRadius > r)
		;	 && (rRangeMin - rRadius < r)
		;
		movaps	xmm2, [esi].rRadius
		shufps	xmm3, xmm3, 0
		shufps	xmm7, xmm7, 0
			movaps	xmm5, xmm4
			mulps	xmm4, xmm4	; xmm4 = r^2
		addps	xmm3, xmm2
		subps	xmm7, xmm2
			mulps	xmm2, xmm2	; xmm2 = rRadius^2
		cmpps	xmm3, xmm5, 6
		cmpps	xmm7, xmm5, 1
		andps	xmm3, xmm7
			subps	xmm0, xmm4
		movmskps	edi, xmm3
		test	edi, edi
		jz	Label_Continue
		;
		; xmm3 ?= q^2 - r^2 < rRadius^2
		;
		cmpps	xmm0, xmm2, 1
		mov	nLeftCount, ecx
		xor	ecx, ecx
		movmskps	eax, xmm0
		and	edi, eax
		.WHILE	!ZERO?
			.IF	(edi & 1) && ([esi].pMesh[ecx*4] != NULL)
				push	ecx
				INVOKE	eglRenderPoly@Polygons@IsHitSegmentSSE,
						[esi].pMesh[ecx*4],
						ebx, pvRay, pvPos
				pop	ecx
			.ENDIF
			inc	ecx
			shr	edi, 1
		.ENDW
		mov	ecx, nLeftCount
Label_Continue:
		pop	esi
		add	esi, (SIZEOF PEGL_RENDER_MESH_MATRIX_PCK4)
		dec	ecx
	.ENDW

	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	ret

eglRenderPoly@MeshList@IsHitSegmentSSE	ENDP

;
;	指定範囲内のメッシュを収集する
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@MeshList@CollectMeshSSE	PROC	NEAR32 C USES ebx esi edi,
	prmmpDst:PTR PEGL_RENDER_MESH_MATRIX_PCK4,
	prmmpSrc:PEGL_RENDER_MESH_MATRIX_PCK4, nCount:DWORD,
	pvPos:PE3D_VECTOR4, rRadius:REAL4

	LOCAL	vPos:E3D_VECTOR4
	LOCAL	nQCount:DWORD
	LOCAL	nLastMask:DWORD
	LOCAL	nLeftCount:DWORD

	mov	eax, pvPos
	movups	xmm7, [eax]
	movups	vPos, xmm7

	mov	ecx, nCount
	mov	eax, 1
	and	ecx, 03H
	mov	edx, -1
	shl	eax, cl
	dec	eax
	cmovz	eax, edx
	mov	nLastMask, eax

	mov	esi, prmmpSrc
	mov	edi, prmmpDst
	ASSUME	esi:PEGL_RENDER_MESH_MATRIX_PCK4
	mov	ecx, nCount
	add	ecx, 3
	shr	ecx, 2
	mov	nQCount, ecx
	.WHILE	!ZERO?
		;
		; xmm0 = |Q - vPos|^2
		; xmm7 = (rRadius + [esi].rRadius)^2
		;
		movss	xmm4, vPos.x
		movss	xmm5, vPos.y
		movss	xmm6, vPos.z
		shufps	xmm4, xmm4, 0
		movaps	xmm0, [esi].xCenter
		shufps	xmm5, xmm5, 0
		movaps	xmm1, [esi].yCenter
		shufps	xmm6, xmm6, 0
		movaps	xmm2, [esi].zCenter
		subps	xmm0, xmm4
			movss	xmm7, rRadius
		subps	xmm1, xmm5
			shufps	xmm7, xmm7, 0
		subps	xmm2, xmm6
			addps	xmm7, [esi].rRadius
		mulps	xmm0, xmm0
		mulps	xmm1, xmm1
			mulps	xmm7, xmm7
		mulps	xmm2, xmm2
		addps	xmm0, xmm1
		addps	xmm0, xmm2
		;
		; xmm0 ?= |Q - vPos|^2 < (rRadius + [esi].rRadius)^2
		;
		mov	eax, -1
		cmp	ecx, 1
		cmpps	xmm0, xmm7, 1
		cmovz	eax, nLastMask
		movmskps	ebx, xmm0
		and	eax, ebx
		.IF	!ZERO?
			mov	DWORD PTR [edi], esi
			add	edi, 4
		.ENDIF
		add	esi, (SIZEOF EGL_RENDER_MESH_MATRIX_PCK4)
		dec	ecx
	.ENDW

	ASSUME	esi:NOTHING
	mov	eax, edi
	sub	eax, prmmpDst
	shr	eax, 2
	ret

eglRenderPoly@MeshList@CollectMeshSSE	ENDP



CodeSeg	ENDS

	END
