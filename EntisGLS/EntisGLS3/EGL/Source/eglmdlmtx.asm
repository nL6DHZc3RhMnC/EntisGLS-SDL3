
; ----------------------------------------------------------------------------
;                          Entis Graphic Library
; ----------------------------------------------------------------------------
;    Copyright (c) 2004-2008 Leshade Entis, Entis-soft. Al rights reserved.
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
xmmSignFlag	DWORD	4 DUP( 80000000H )
xmmMaskSignFlag	DWORD	4 DUP( 7FFFFFFFH )

rConstHalf		REAL4	0.5
rConst2			REAL4	2.0
rConst1p001		REAL4	1.001
rConstZeroLittle	REAL4	0.0000001

ConstSeg	ENDS


; ----------------------------------------------------------------------------
;	コードセグメント
; ----------------------------------------------------------------------------

CodeSeg	SEGMENT	PARA READONLY FLAT 'CODE'

;
;	モデル当たり判定オブジェクト生成
; ----------------------------------------------------------------------------
ALIGN	10H
eglCreateModelMatrix		PROC	NEAR32 C USES ebx esi edi

	;
	; メモリ確保
	;
	.IF	EGL_hImageHeap == NULL
		INVOKE	eslHeapCreate , 0, 0, ESL_HEAP_ZERO_INIT, NULL
		mov	EGL_hImageHeap, eax
	.ENDIF
	;
	INVOKE	eslHeapAllocate ,
			EGL_hImageHeap,
			(SIZEOF EGL_MODEL_MATRIX_BUF), ESL_HEAP_ZERO_INIT
	mov	ebx, eax
	ASSUME	ebx:PTR EGL_MODEL_MATRIX_BUF
	;
	; 関数ポインタ設定
	;
	mov	[ebx].mmat.pfnRelease, OFFSET eglModelMatrix@Release
	.IF	ERI_EnabledProcessorType & ERI_USE_XMM_P3
		mov	[ebx].mmat.pfnInitialize, \
				OFFSET eglModelMatrix@InitializeSSE
		mov	[ebx].mmat.pfnIsHitInclusiveSphere, \
				OFFSET eglModelMatrix@IsHitInclusiveSphereSSE
		mov	[ebx].mmat.pfnIsHitAgainstSphere, \
				OFFSET eglModelMatrix@IsHitAgainstSphereSSE
		mov	[ebx].mmat.pfnIsCrossingSegment, \
				OFFSET eglModelMatrix@IsCrossingSegmentSSE
	.ELSE
		mov	[ebx].mmat.pfnInitialize, \
				OFFSET eglModelMatrix@Initialize486
		mov	[ebx].mmat.pfnIsHitInclusiveSphere, \
				OFFSET eglModelMatrix@IsHitInclusiveSphere486
		mov	[ebx].mmat.pfnIsHitAgainstSphere, \
				OFFSET eglModelMatrix@IsHitAgainstSphere486
		mov	[ebx].mmat.pfnIsCrossingSegment, \
				OFFSET eglModelMatrix@IsCrossingSegment486
	.ENDIF
	;
	ASSUME	ebx:NOTHING
	mov	eax, ebx
	ret

eglCreateModelMatrix		ENDP

;
;	モデル当たり判定オブジェクト解放
; ----------------------------------------------------------------------------
ALIGN	10H
eglModelMatrix@Release		PROC	NEAR32 C USES ebx esi edi,
	hMatrix:HEGL_MODEL_MATRIX

	mov	ebx, hMatrix
	ASSUME	ebx:PTR EGL_MODEL_MATRIX_BUF
	;
	.IF	[ebx].pPolyMatrixBuf != NULL
		INVOKE	eslHeapFree , EGL_hImageHeap, [ebx].pPolyMatrixBuf, 0
	.ENDIF
	;
	INVOKE	eslHeapFree , EGL_hImageHeap , ebx, 0
	;
	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglModelMatrix@Release		ENDP

;
;	モデル当たり判定用パラメータ計算 486 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglModelMatrix@InitializeMatrixInfo486	PROC	NEAR32 C USES esi,
	pv1:PCE3D_VECTOR4, pv2:PCE3D_VECTOR4,
	pv3:PCE3D_VECTOR4, pvNormal:PCE3D_VECTOR4

	LOCAL	v1:E3D_VECTOR, v2:E3D_VECTOR, v3:E3D_VECTOR

	ASSUME	ebx:PTR EGL_MODEL_MATRIX_BUF
	ASSUME	edi:PEGL_POLY_MATRIX_INFO

	;
	; モデル最大座標・最小座標更新
	;
	.IF	[ebx].fModelSized == 0
		mov	ecx, pv1
		ASSUME	ecx:PTR E3D_VECTOR4
		mov	eax, [ecx].x
		mov	edx, [ecx].y
		mov	ecx, [ecx].z
		ASSUME	ecx:NOTHING
		mov	[ebx].vMinModelPos.x, eax
		mov	[ebx].vMinModelPos.y, edx
		mov	[ebx].vMinModelPos.z, ecx
		mov	[ebx].vMaxModelPos.x, eax
		mov	[ebx].vMaxModelPos.y, edx
		mov	[ebx].vMaxModelPos.z, ecx
		mov	[ebx].fModelSized, 1
	.ENDIF

	;
	; 頂点座標取得
	;
	@INDEX = 0
	FOR	@DUMMY, <pv1, pv2, pv3>
		mov	ecx, @DUMMY
		ASSUME	ecx:PTR E3D_VECTOR4
		fld	[ecx].z
		fld	[ecx].y
		fld	[ecx].x
		mov	eax, [ecx].x
		mov	edx, [ecx].y
		mov	ecx, [ecx].z
		ASSUME	ecx:NOTHING
		mov	[edi].vertex[@INDEX*10H].x, eax
		mov	[edi].vertex[@INDEX*10H].y, edx
		mov	[edi].vertex[@INDEX*10H].z, ecx
		@INDEX = @INDEX + 1
		;
		FOR	@MEMBER, <x, y, z>
			fcom	[ebx].vMinModelPos.@MEMBER
			fstsw	ax
			.IF	ax & 0100H	; st(0) < [ebx].vMinModelPos.@MEMBER
				fst	[ebx].vMinModelPos.@MEMBER
			.ENDIF
			fcom	[ebx].vMaxModelPos.@MEMBER
			fstsw	ax
			.IF	!(ax & 0100H)	; st(0) >= [ebx].vMaxModelPos.@MEMBER
				fst	[ebx].vMaxModelPos.@MEMBER
			.ENDIF
			fstp	st(0)
		ENDM
	ENDM
	;
	; 法線取得
	;
	FOR	@MEMBER, <x, y, z>
		fld	[edi].vertex[10H].@MEMBER
		fsub	[edi].vertex[0].@MEMBER
		fstp	v1.@MEMBER
		fld	[edi].vertex[20H].@MEMBER
		fsub	[edi].vertex[0].@MEMBER
		fstp	v2.@MEMBER
	ENDM
	;
	fld	v1.y
	fmul	v2.z
	fld	v1.z
	fmul	v2.y
	fsubp	st(1), st
	fstp	[edi].normal.x
	;
	fld	v1.z
	fmul	v2.x
	fld	v1.x
	fmul	v2.z
	fsubp	st(1), st
	fstp	[edi].normal.y
	;
	fld	v1.x
	fmul	v2.y
	fld	v1.y
	fmul	v2.x
	fsubp	st(1), st
	fstp	[edi].normal.z
	;
	mov	edx, pvNormal
	.IF	edx != NULL
		ASSUME	edx:PTR E3D_VECTOR4
		FOR	@MEMBER, <x, y, z>
			fld	[edx].@MEMBER
			fmul	[edi].normal.@MEMBER
		ENDM
		faddp	st(2), st
		faddp	st(1), st
		ftst
		fstsw	ax
		fstp	st(0)
		sahf
		.IF	CARRY?
			xor	[edi].normal.x, 80000000H
			xor	[edi].normal.y, 80000000H
			xor	[edi].normal.z, 80000000H
		.ENDIF
		ASSUME	edx:NOTHING
	.ENDIF
	ASSUME	esi:NOTHING
	;
	fld	[edi].normal.x
	fmul	st(0), st
	fld	[edi].normal.y
	fmul	st(0), st
	fld	[edi].normal.z
	fmul	st(0), st
	faddp	st(2), st
	faddp	st(1), st
	fsqrt
	fld1
	fdivrp	st(1), st
	;
	fld	[edi].normal.x
	fmul	st, st(1)
	fstp	[edi].normal.x
	fld	[edi].normal.y
	fmul	st, st(1)
	fstp	[edi].normal.y
	fmul	[edi].normal.z
	fstp	[edi].normal.z
	;
	; 各辺のベクトルと長さを計算
	;
	FOR	@MEMBER, <x, y, z>
		fld	[edi].vertex[0].@MEMBER
		fsub	[edi].vertex[10H].@MEMBER
		fstp	v1.@MEMBER
		fld	[edi].vertex[20H].@MEMBER
		fsub	[edi].vertex[10H].@MEMBER
		fstp	v2.@MEMBER
		fld	[edi].vertex[20H].@MEMBER
		fsub	[edi].vertex[0].@MEMBER
		fstp	v3.@MEMBER
	ENDM
	;
	@INDEX = 0
	FOR	@DUMMY, <v1, v2, v3>
		FOR	@MEMBER, <x, y, z>
			fld	@DUMMY.@MEMBER
			fmul	st(0), st
		ENDM
		faddp	st(2), st
		faddp	st(1), st
		fsqrt
		fstp	[edi].vertex[@INDEX].d
		@INDEX = @INDEX + 10H
	ENDM
	;
	; cos ∠ABC の計算
	;
	FOR	@MEMBER, <x, y, z>
		fld	v1.@MEMBER
		fmul	v2.@MEMBER
	ENDM
	faddp	st(2), st
	faddp	st(1), st
	;
	fld	[edi].vertex[0].d
	fmul	[edi].vertex[10H].d
	fdivp	st(1), st
	fstp	[edi].cos_abc
	;
	; cos ∠ACB の計算
	;
	FOR	@MEMBER, <x, y, z>
		fld	v2.@MEMBER
		fmul	v3.@MEMBER
	ENDM
	faddp	st(2), st
	faddp	st(1), st
	;
	fld	[edi].vertex[10H].d
	fmul	[edi].vertex[20H].d
	fdivp	st(1), st
	fstp	[edi].cos_acb

	ASSUME	ebx:NOTHING
	ASSUME	edi:NOTHING
	ret

eglModelMatrix@InitializeMatrixInfo486	ENDP

;
;	モデル当たり判定オブジェクト初期化 486 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglModelMatrix@Initialize486	PROC	NEAR32 C USES ebx esi edi,
	hMatrix:HEGL_MODEL_MATRIX,
	pModel:PTR PCE3D_PRIMITIVE_POLYGON, nPolyCount:DWORD

	LOCAL	pvVertexBuf:PCE3D_VECTOR4
	LOCAL	pvNormalBuf:PCE3D_VECTOR4
	LOCAL	pv1:PCE3D_VECTOR4, pv2:PCE3D_VECTOR4, pv3:PCE3D_VECTOR4
	LOCAL	pvNormal:PCE3D_VECTOR4
	LOCAL	nMeshPolyCount:DWORD

	mov	ebx, hMatrix
	ASSUME	ebx:PTR EGL_MODEL_MATRIX_BUF
	mov	[ebx].fModelSized, 0
	;
	; ポリゴン数カウント
	;
	xor	ecx, ecx
	xor	edi, edi
	.WHILE	ecx < nPolyCount
		mov	esi, pModel
		mov	esi, [esi + ecx * 4]
		ASSUME	esi:PCE3D_PRIMITIVE_POLYGON
		.IF	esi != NULL
			.IF	[esi].dwTypeFlag & E3D_MESH_POLYGON
				mov	eax, [esi].dwVertexCount
				add	edi, (E3D_PRIMITIVE_MESH_LIST PTR [esi].mesh.uv_map[eax*8]).dwPolyCount
			.ELSEIF	!([esi].dwTypeFlag & (NOT E3D_POLYGON_PRIMITIVE_MASK))
				inc	edi
			.ENDIF
		.ENDIF
		ASSUME	esi:NOTHING
		inc	ecx
	.ENDW
	mov	nMeshPolyCount, edi
	;
	; メモリ確保
	;
	.IF	[ebx].pPolyMatrixBuf != NULL
		mov	edx, nMeshPolyCount
		.IF	edx == [ebx].nPolygonCount
			mov	eax, [ebx].pPolyMatrix
			jmp	Label_BeginParameter
		.ENDIF
		INVOKE	eslHeapFree , EGL_hImageHeap, [ebx].pPolyMatrixBuf, 0
	.ENDIF
	;
	mov	edx, nMeshPolyCount
	mov	[ebx].nPolygonCount, edx
	imul	edx, @POLY_MATRIX_SIZE
	INVOKE	eslHeapAllocate , EGL_hImageHeap, edx, 0
	mov	[ebx].pPolyMatrix, eax
	mov	[ebx].pPolyMatrixBuf, eax
	;
	; 当たり判定用パラメータ計算
	;
Label_BeginParameter:
	mov	edi, eax
	ASSUME	edi:PEGL_POLY_MATRIX_INFO
	xor	ecx, ecx
	mov	[ebx].nPolygonCount, ecx
	.WHILE	ecx < nPolyCount
		mov	esi, pModel
		mov	esi, [esi + ecx * 4]
		ASSUME	esi:PCE3D_PRIMITIVE_POLYGON
		.IF	esi != NULL
		.IF	[esi].dwTypeFlag & E3D_MESH_POLYGON
			push	ecx
			;
			mov	eax, [esi].mesh.vertices
			mov	edx, [esi].mesh.normals
			mov	pvVertexBuf, eax
			mov	pvNormalBuf, edx
			mov	eax, [esi].dwVertexCount
			lea	esi, [esi].mesh.uv_map[eax*8]
			ASSUME	esi:PTR E3D_PRIMITIVE_MESH_LIST
			;
			mov	ecx, [esi].dwPolyCount
			lea	esi, [esi].mpEntries[0]
			ASSUME	esi:PTR E3D_PRIMITIVE_MESH_POLY
			;
			test	ecx, ecx
			.WHILE	!ZERO?
				push	ecx
				.IF	[esi].dwVertexCount >= 3
					mov	eax, [esi].dwIndex[0]
					mov	edx, [esi].dwIndex[4]
					shl	eax, 4
					shl	edx, 4
					add	eax, pvVertexBuf
					add	edx, pvVertexBuf
					mov	pv1, eax
					mov	pv2, edx
					;
					mov	eax, [esi].dwIndex[8]
					mov	edx, [esi].dwIndex[0]
					shl	eax, 4
					shl	edx, 4
					add	eax, pvVertexBuf
					;
					.IF	pvNormalBuf != NULL
						add	edx, pvNormalBuf
					.ELSE
						xor	edx, edx
					.ENDIF
					mov	pv3, eax
					;
					INVOKE	eglModelMatrix@InitializeMatrixInfo486 ,
									pv1, pv2, pv3, edx
					;
					mov	eax, [esi].dwVertexCount
					add	edi, @POLY_MATRIX_SIZE
					inc	[ebx].nPolygonCount
					lea	esi, [esi].dwIndex[eax*4]
				.ENDIF
				;
				pop	ecx
				dec	ecx
			.ENDW
			;
			ASSUME	esi:PCE3D_PRIMITIVE_POLYGON
			;
			pop	ecx

		.ELSEIF	!([esi].dwTypeFlag & (NOT E3D_POLYGON_PRIMITIVE_MASK)) \
						&& ([esi].dwVertexCount >= 3)
			push	ecx
			;
			@VERTEX_SIZE = (SIZEOF E3D_PRIMITIVE_VERTEX)
			mov	eax, [esi].polygon[0].vertex
			mov	pv1, eax
			mov	eax, [esi].polygon[@VERTEX_SIZE].vertex
			mov	pv2, eax
			mov	eax, [esi].polygon[@VERTEX_SIZE*2].vertex
			mov	pv3, eax
			;
			xor	eax, eax
			.IF	[esi].dwTypeFlag & E3D_SMOOTH_POLYGON
				mov	eax, [esi].polygon[0].normal
			.ENDIF
			;
			INVOKE	eglModelMatrix@InitializeMatrixInfo486 ,
							pv1, pv2, pv3, eax
			;
			pop	ecx
			add	edi, @POLY_MATRIX_SIZE
			inc	[ebx].nPolygonCount
		.ENDIF
		.ENDIF
		inc	ecx
	.ENDW
	;
	; モデル全体サイズ計算
	;
	fld	[ebx].vMinModelPos.x
	fadd	[ebx].vMaxModelPos.x
	fld	[ebx].vMinModelPos.y
	fadd	[ebx].vMaxModelPos.y
	fld	[ebx].vMinModelPos.z
	fadd	[ebx].vMaxModelPos.z
	fld	rConstHalf
	fmul	st(1), st
	fmul	st(2), st
	fmulp	st(3), st
	fstp	[ebx].vMatrixSphere.z
	fstp	[ebx].vMatrixSphere.y
	fstp	[ebx].vMatrixSphere.x
	;
	fld	[ebx].vMaxModelPos.x
	fsub	[ebx].vMinModelPos.x
	fmul	st(0), st
	fld	[ebx].vMaxModelPos.y
	fsub	[ebx].vMinModelPos.y
	fmul	st(0), st
	fld	[ebx].vMaxModelPos.z
	fsub	[ebx].vMinModelPos.z
	fmul	st(0), st
	faddp	st(1), st
	faddp	st(1), st
	fsqrt
	fmul	rConstHalf
	fmul	rConst1p001
	fstp	[ebx].vMatrixSphere.d
	;
	ASSUME	edi:NOTHING
	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglModelMatrix@Initialize486	ENDP

;
;	内包空間と球体との交差判定　486 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglModelMatrix@IsHitInclusiveSphere486	PROC	NEAR32 C USES ebx esi edi,
	hMatrix:HEGL_MODEL_MATRIX,
	pSphere:PTR E3D_VECTOR, rRadius:REAL4, pHitResult:PTR SDWORD

	LOCAL	vSphere:E3D_VECTOR

	mov	ebx, hMatrix
	ASSUME	ebx:PTR EGL_MODEL_MATRIX_BUF
	;
	; 球の中心座標取得
	;
	mov	esi, pSphere
	ASSUME	esi:PTR E3D_VECTOR
	FOR	@MEMBER, <x, y, z>
		fld	[esi].@MEMBER
		fst	vSphere.@MEMBER
		fsub	[ebx].vMatrixSphere.@MEMBER
		fmul	st(0), st
	ENDM
	faddp	st(1), st
	faddp	st(1), st	; st(0) = distance^2
	;
	fld	rRadius
	fadd	[ebx].vMatrixSphere.d
	fmul	st(0), st
	fcompp			; radius^2 < distance^2 ?
	fstsw	ax
	.IF	ax & 0100H
		mov	edx, pHitResult
		xor	eax, eax
		mov	SDWORD PTR [edx], 0
		ret
	.ENDIF
	;
	ASSUME	esi:NOTHING
	;
	; 順次面判定
	;
	mov	ecx, [ebx].nPolygonCount
	mov	esi, [ebx].pPolyMatrix
	ASSUME	esi:PTR EGL_POLY_MATRIX_INFO
	test	ecx, ecx
	.WHILE	!ZERO?
		FOR	@MEMBER, <x, y, z>
			fld	vSphere.@MEMBER
			fsub	[esi].vertex[0].@MEMBER
			fmul	[esi].normal.@MEMBER
		ENDM
		faddp	st(2), st
		faddp	st(1), st
		fcomp	rRadius
		fstsw	ax
		sahf
		.IF	!CARRY?
			mov	edx, pHitResult
			xor	eax, eax
			mov	SDWORD PTR [edx], 0
			ret
		.ENDIF
		add	esi, (SIZEOF EGL_POLY_MATRIX_INFO)
		dec	ecx
	.ENDW
	;
	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	mov	edx, pHitResult
	xor	eax, eax
	mov	SDWORD PTR [edx], -1
	ret

eglModelMatrix@IsHitInclusiveSphere486	ENDP

;
;	指定のポリゴンの領域内の座標かどうかを判定
; ----------------------------------------------------------------------------
;　パラメータ；
;	[esi] : PTR EGL_POLY_MATRIX_INFO = ポリゴン情報
;	[edi] : PTR E3D_VECTOR = 同平面上の座標
;  返り値；
;	CARRY? : = 1 ; 領域外
;	CARRY? : = 0 ; 領域内
; ----------------------------------------------------------------------------
ALIGN	10H
eglModelMatrix@IsInclusiveTriangle486	PROC	NEAR32 C PRIVATE, rRadius:REAL4

	LOCAL	rDistance:REAL4
	LOCAL	vp:E3D_VECTOR
	LOCAL	cos_abc:REAL4, cos_acb:REAL4

	ASSUME	esi:PTR EGL_POLY_MATRIX_INFO
	ASSUME	edi:PTR E3D_VECTOR
	;
	;	cos ∠ABC と cos ∠PBA, cos ∠PBC の比較
	; --------------------------------------------------------------------
	;
	; P-B の計算
	;
	FOR	@MEMBER, <x, y, z>
		fld	[edi].@MEMBER
		fsub	[esi].vertex[10H].@MEMBER
		fst	vp.@MEMBER
		fmul	st(0), st
	ENDM
	faddp	st(2), st
	faddp	st(1), st
	fsqrt
	fst	rDistance
	;
	; 判定角補正
	;
	.IF	(DWORD PTR rDistance) >= 00800000H
		fld	rRadius
		fdivrp	st(1), st
	.ELSE
		fstp	st(0)
		fld1
		fadd	st(0), st
	.ENDIF
	fsubr	[esi].cos_abc
	fstp	cos_abc
	;
	; cos ∠ABC と cos ∠PBA を比較
	;
	FOR	@MEMBER, <x, y, z>
		fld	[esi].vertex[0].@MEMBER
		fsub	[esi].vertex[10H].@MEMBER
		fmul	vp.@MEMBER
	ENDM
	faddp	st(2), st
	faddp	st(1), st
	;
	fld	[esi].vertex[0].d
	fmul	rDistance
	fdivp	st(1), st
	;
	fcomp	cos_abc
	fstsw	ax
	sahf
	jc	Label_Return
	;
	; cos ∠ABC と cos ∠PBC を比較
	;
	FOR	@MEMBER, <x, y, z>
		fld	[esi].vertex[20H].@MEMBER
		fsub	[esi].vertex[10H].@MEMBER
		fmul	vp.@MEMBER
	ENDM
	faddp	st(2), st
	faddp	st(1), st
	;
	fld	[esi].vertex[10H].d
	fmul	rDistance
	fdivp	st(1), st
	;
	fcomp	cos_abc
	fstsw	ax
	sahf
	jc	Label_Return
	;
	;	cos ∠ACB と cos ∠PCA, cos ∠PCB の比較
	; --------------------------------------------------------------------
	;
	; P-C の計算
	;
	FOR	@MEMBER, <x, y, z>
		fld	[edi].@MEMBER
		fsub	[esi].vertex[20H].@MEMBER
		fst	vp.@MEMBER
		fmul	st(0), st
	ENDM
	faddp	st(2), st
	faddp	st(1), st
	fsqrt
	fst	rDistance
	;
	; 判定角補正
	;
	.IF	(DWORD PTR rDistance) >= 00800000H
		fld	rRadius
		fdivrp	st(1), st
	.ELSE
		fstp	st(0)
		fld1
		fadd	st(0), st
	.ENDIF
	fsubr	[esi].cos_acb
	fstp	cos_acb
	;
	; cos ∠ACB と cos ∠PCA を比較
	;
	FOR	@MEMBER, <x, y, z>
		fld	[esi].vertex[0].@MEMBER
		fsub	[esi].vertex[20H].@MEMBER
		fmul	vp.@MEMBER
	ENDM
	faddp	st(2), st
	faddp	st(1), st
	;
	fld	[esi].vertex[20H].d
	fmul	rDistance
	fdivp	st(1), st
	;
	fcomp	cos_acb
	fstsw	ax
	sahf
	jc	Label_Return
	;
	; cos ∠ACB と cos ∠PCB を比較
	;
	FOR	@MEMBER, <x, y, z>
		fld	[esi].vertex[10H].@MEMBER
		fsub	[esi].vertex[20H].@MEMBER
		fmul	vp.@MEMBER
	ENDM
	faddp	st(2), st
	faddp	st(1), st
	;
	fld	[esi].vertex[10H].d
	fmul	rDistance
	fdivp	st(1), st
	;
	fcomp	cos_acb
	fstsw	ax
	sahf
	;
	ASSUME	esi:NOTHING
	ASSUME	edi:NOTHING
Label_Return:
	sbb	eax, eax
	ret

eglModelMatrix@IsInclusiveTriangle486	ENDP

;
;	ポリゴンと球体との交差判定 486 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglModelMatrix@IsHitAgainstSphere486	PROC	NEAR32 C USES ebx esi edi,
	hMatrix:HEGL_MODEL_MATRIX,
	pSphere:PTR E3D_VECTOR, rRadius:REAL4,
	pHitResult:PTR SDWORD, pHitPos:PTR E3D_VECTOR,
	pHitNormal:PTR E3D_VECTOR, pReflection:PTR E3D_VECTOR

	LOCAL	rDistance:REAL4
	LOCAL	vPos:E3D_VECTOR
	LOCAL	vSphere:E3D_VECTOR
	LOCAL	vReflection:E3D_VECTOR

	mov	ebx, hMatrix
	ASSUME	ebx:PTR EGL_MODEL_MATRIX_BUF
	;
	; 球の中心座標取得
	;
	mov	esi, pSphere
	ASSUME	esi:PTR E3D_VECTOR
	FOR	@MEMBER, <x, y, z>
		fld	[esi].@MEMBER
		fst	vSphere.@MEMBER
		fsub	[ebx].vMatrixSphere.@MEMBER
		fmul	st(0), st
	ENDM
	faddp	st(1), st
	faddp	st(1), st	; st(0) = distance^2
	;
	fld	rRadius
	fadd	[ebx].vMatrixSphere.d
	fmul	st(0), st
	fcompp			; radius^2 < distance^2 ?
	fstsw	ax
	.IF	ax & 0100H
		mov	edx, pHitResult
		xor	eax, eax
		mov	SDWORD PTR [edx], 0
		ret
	.ENDIF
	;
	ASSUME	esi:NOTHING
	;
	; 反射ベクトル取得
	;
	mov	esi, pReflection
	.IF	esi != NULL
		ASSUME	esi:PTR E3D_VECTOR
		mov	eax, [esi].x
		mov	ecx, [esi].y
		mov	edx, [esi].z
		mov	vReflection.x, eax
		mov	vReflection.y, ecx
		mov	vReflection.z, edx
		ASSUME	esi:NOTHING
	.ENDIF
	;
	; 順次面判定
	;
	mov	edx, pHitResult
	mov	SDWORD PTR [edx], 0
	mov	ecx, [ebx].nPolygonCount
	mov	esi, [ebx].pPolyMatrix
	ASSUME	esi:PTR EGL_POLY_MATRIX_INFO
	test	ecx, ecx
	.WHILE	!ZERO?
		;
		; 面との距離判定
		;
		FOR	@MEMBER, <x, y, z>
			fld	vSphere.@MEMBER
			fsub	[esi].vertex[0].@MEMBER
			fmul	[esi].normal.@MEMBER
		ENDM
		faddp	st(2), st
		faddp	st(1), st
		fst	rDistance
		fabs
		fcomp	rRadius
		fstsw	ax
		sahf
		.IF	CARRY?
			;
			; 三角形の領域内か判定
			;
			push	ecx
			FOR	@MEMBER, <x, y, z>
				fld	vSphere.@MEMBER
				fld	[esi].normal.@MEMBER
				fmul	rDistance
				fsubp	st(1), st
				fstp	vPos.@MEMBER
			ENDM
			lea	edi, vPos
			INVOKE	eglModelMatrix@IsInclusiveTriangle486 , rRadius
			test	eax, eax
			.IF	ZERO?
				;
				; パラメータ設定
				;
				mov	ecx, pHitResult
				mov	eax, rDistance
				mov	edx, pHitPos
				mov	edi, pHitNormal
				and	eax, 7FFFFFFFH
				mov	SDWORD PTR [ecx], -1
				mov	rRadius, eax
				.IF	edx != NULL
					ASSUME	edx:PTR E3D_VECTOR
					mov	eax, vPos.x
					mov	ecx, vPos.y
					mov	[edx].x, eax
					mov	eax, vPos.z
					mov	[edx].y, ecx
					mov	[edx].z, eax
					ASSUME	edx:NOTHING
				.ENDIF
				.IF	edi != NULL
					ASSUME	edi:PTR E3D_VECTOR
					mov	eax, [esi].normal.x
					mov	ecx, [esi].normal.y
					mov	[edi].x, eax
					mov	eax, [esi].normal.z
					mov	[edi].y, ecx
					mov	[edi].z, eax
					ASSUME	edi:NOTHING
				.ENDIF
				mov	edi, pReflection
				.IF	edi != NULL
					FOR	@MEMBER, <x, y, z>
						fld	vReflection.@MEMBER
						fmul	[esi].normal.@MEMBER
					ENDM
					faddp	st(2), st
					faddp	st(1), st
					fadd	st(0), st
					;
					ASSUME	edi:PTR E3D_VECTOR
					fld	[esi].normal.x
					fmul	st, st(1)
					fsubr	vReflection.x
					fstp	[edi].x
					fld	[esi].normal.y
					fmul	st, st(1)
					fsubr	vReflection.y
					fstp	[edi].y
					fmul	[esi].normal.z
					fsubr	vReflection.z
					fstp	[edi].z
					ASSUME	edi:NOTHING
				.ENDIF
			.ENDIF
			pop	ecx
		.ENDIF
		add	esi, (SIZEOF EGL_POLY_MATRIX_INFO)
		dec	ecx
	.ENDW
	;
	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglModelMatrix@IsHitAgainstSphere486	ENDP

;
;	ポリゴンと線分との交差判定
; ----------------------------------------------------------------------------
ALIGN	10H
eglModelMatrix@IsCrossingSegment486	PROC	NEAR32 C USES ebx esi edi,
	hMatrix:HEGL_MODEL_MATRIX,
	pPos0:PTR E3D_VECTOR, pPos1:PTR E3D_VECTOR, rErrorGap:REAL4,
	pHitResult:PTR SDWORD, pHitPos:PTR E3D_VECTOR,
	pHitNormal:PTR E3D_VECTOR, pReflection:PTR E3D_VECTOR

	LOCAL	vPos0:E3D_VECTOR, vPos1:E3D_VECTOR
	LOCAL	vCrossPos:E3D_VECTOR
	LOCAL	vDeltaP1P0:E3D_VECTOR
	LOCAL	rSqrAbsDeltaP1P0:REAL4	; |P1 - P0|^2
	LOCAL	rAbsDeltaP1P0:REAL4	; |P1 - P0|
	LOCAL	rSqrAbsDeltaXP0:REAL4	; |X - P0|^2
	LOCAL	rDistanceModel:REAL4	; (X - P0 | P1 - P0) / |P1 - P0|
	LOCAL	rMinDeltaT:REAL4
	LOCAL	vInnerPA:REAL4
	LOCAL	rTemp:REAL4

	mov	ebx, hMatrix
	ASSUME	ebx:PTR EGL_MODEL_MATRIX_BUF
	;
	mov	edx, pHitResult
	mov	SDWORD PTR [edx], 0
	;
	; 線分の座標を取得
	;
	mov	esi, pPos0
	mov	edi, pPos1
	FOR	@MEMBER, <x, y, z>
		fld	(E3D_VECTOR PTR [esi]).@MEMBER
		fst	vPos0.@MEMBER
		fld	(E3D_VECTOR PTR [edi]).@MEMBER
		fst	vPos1.@MEMBER
		fsubrp	st(1), st
		fst	vDeltaP1P0.@MEMBER
		fmul	st(0), st
	ENDM
	faddp	st(1), st
	faddp	st(1), st
	fst	rSqrAbsDeltaP1P0
	;
	fsqrt
	fst	rAbsDeltaP1P0
	;
	fcomp	rConstZeroLittle
	fstsw	ax
	sahf
	.IF	CARRY?
		xor	eax, eax
		ret
	.ENDIF
	;
	; モデル中心点までの距離を計算
	;
	FOR	@MEMBER, <x, y, z>
		fld	[ebx].vMatrixSphere.@MEMBER
		fsub	vPos0.@MEMBER
		fmul	st(0), st
	ENDM
	faddp	st(1), st
	faddp	st(1), st
	fstp	rSqrAbsDeltaXP0
	;
	; モデル中心点までの線分方向の距離を計算
	;
	FOR	@MEMBER, <x, y, z>
		fld	[ebx].vMatrixSphere.@MEMBER
		fsub	vPos0.@MEMBER
		fmul	vDeltaP1P0.@MEMBER
	ENDM
	faddp	st(1), st
	faddp	st(1), st
	fdiv	rAbsDeltaP1P0
	fst	rDistanceModel
	;
	; 距離判定
	;
	fmul	st(0), st
	fld	rSqrAbsDeltaXP0
	fsubrp	st(1), st
	fld	[ebx].vMatrixSphere.d
	fmul	st(0), st
	fcompp
	fstsw	ax
	sahf
	.IF	CARRY?		; radius^2 < rSqrAbsDeltaXP0 - rDistanceModel^2 ?
		xor	eax, eax
		ret
	.ENDIF
	;
	.IF	rDistanceModel & 80000000H
		fld	[ebx].vMatrixSphere.d
		fmul	st(0), st
		fcomp	rSqrAbsDeltaXP0
		fstsw	ax
		sahf
		.IF	CARRY?		; radius^2 < rSqrAbsDeltaXP0
			xor	eax, eax
			ret
		.ENDIF
	.ELSE
		fld	[ebx].vMatrixSphere.d
		fadd	rAbsDeltaP1P0
		fcomp	rDistanceModel
		fstsw	ax
		sahf
		.IF	CARRY?		; |P1 - P0| + radius < rDistanceModel
			xor	eax, eax
			ret
		.ENDIF
	.ENDIF
	;
	; 順次線分交差判定
	;
	mov	rMinDeltaT, 3F8003FFH		; 1.000000
	mov	ecx, [ebx].nPolygonCount
	mov	esi, [ebx].pPolyMatrix
	ASSUME	esi:PTR EGL_POLY_MATRIX_INFO
	test	ecx, ecx
	.WHILE	!ZERO?
		;
		; 交差点までの t [0,1] を求める
		;
		FOR	@MEMBER, <x, y, z>
			fld	vDeltaP1P0.@MEMBER
			fmul	[esi].normal.@MEMBER
		ENDM
		faddp	st(2), st
		faddp	st(1), st
		fstp	vInnerPA
		mov	eax, vInnerPA
		and	eax, 7FFFFFFFH
		cmp	eax, 01800000H
		jb	Loop_Continue
		;
		FOR	@MEMBER, <x, y, z>
			fld	[esi].vertex[0].@MEMBER
			fsub	vPos0.@MEMBER
			fmul	[esi].normal.@MEMBER
		ENDM
		faddp	st(2), st
		faddp	st(1), st
		fdiv	vInnerPA
		fstp	rTemp
		mov	eax, rTemp
		test	eax, eax
		js	Loop_Continue
		.IF	(DWORD PTR eax) < (DWORD PTR rMinDeltaT)
			push	ecx
			;
			; 交差点の座標を求める
			;
			FOR	@MEMBER, <x, y, z>
				fld	vDeltaP1P0.@MEMBER
				fmul	rTemp
				fadd	vPos0.@MEMBER
				fstp	vCrossPos.@MEMBER
			ENDM
			;
			; 三角形の領域内か判定する
			;
			lea	edi, vCrossPos
			INVOKE	eglModelMatrix@IsInclusiveTriangle486 , rErrorGap
			test	eax, eax
			.IF	ZERO?
			;
			; パラメータ設定
			;
			mov	edx, pHitResult
			mov	eax, rTemp
			mov	rMinDeltaT, eax
			mov	SDWORD PTR [edx], -1
			mov	edx, pHitPos
			mov	edi, pHitNormal
			.IF	edx != NULL
				ASSUME	edx:PTR E3D_VECTOR
				mov	eax, vCrossPos.x
				mov	ecx, vCrossPos.y
				mov	[edx].x, eax
				mov	eax, vCrossPos.z
				mov	[edx].y, ecx
				mov	[edx].z, eax
				ASSUME	edx:NOTHING
			.ENDIF
			.IF	edi != NULL
				ASSUME	edi:PTR E3D_VECTOR
				mov	eax, [esi].normal.x
				mov	ecx, [esi].normal.y
				mov	[edi].x, eax
				mov	eax, [esi].normal.z
				mov	[edi].y, ecx
				mov	[edi].z, eax
				ASSUME	edi:NOTHING
			.ENDIF
			mov	edi, pReflection
			.IF	edi != NULL
				ASSUME	edi:PTR E3D_VECTOR
				FOR	@MEMBER, <x, y, z>
					fld	[esi].normal.@MEMBER
					fmul	vInnerPA
					fadd	st(0), st
					fld	vDeltaP1P0.@MEMBER
					fsubrp	st(1), st
					fstp	[edi].@MEMBER
				ENDM
				ASSUME	edi:NOTHING
			.ENDIF
			;
			.ENDIF
			pop	ecx
		.ENDIF
Loop_Continue:	;
		add	esi, (SIZEOF EGL_POLY_MATRIX_INFO)
		dec	ecx
	.ENDW
	;
	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglModelMatrix@IsCrossingSegment486	ENDP

;
;	モデル当たり判定オブジェクト初期化 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglModelMatrix@InitializeMatrixInfoSSE	PROC	NEAR32 C USES esi,
	pv1:PCE3D_VECTOR4, pv2:PCE3D_VECTOR4,
	pv3:PCE3D_VECTOR4, pvNormal:PCE3D_VECTOR4

	ASSUME	ebx:PTR EGL_MODEL_MATRIX_BUF
	ASSUME	edi:PEGL_POLY_MATRIX_INFO

	;
	; モデル最大座標・最小座標更新
	;
	.IF	[ebx].fModelSized == 0
		mov	ecx, pv1
		movups	xmm0, [ecx]
		movups	[ebx].vMinModelPos, xmm0
		movups	[ebx].vMaxModelPos, xmm0
		mov	[ebx].fModelSized, 1
	.ENDIF
	;
	movups	xmm4, [ebx].vMinModelPos
	movups	xmm5, [ebx].vMaxModelPos

	;
	; 頂点座標取得
	;
	mov	eax, pv1
	mov	ecx, pv2
	mov	edx, pv3
	movups	xmm0, [eax]
	movups	xmm1, [ecx]
	movups	xmm2, [edx]
	;
	minps	xmm4, xmm0
	maxps	xmm5, xmm0
	minps	xmm4, xmm1
	maxps	xmm5, xmm1
	minps	xmm4, xmm2
	maxps	xmm5, xmm2
	movups	[ebx].vMinModelPos, xmm4
	movups	[ebx].vMaxModelPos, xmm5
	;
	movaps	[edi].vertex[0], xmm0
	movaps	[edi].vertex[10H], xmm1
	subps	xmm1, xmm0
	movaps	[edi].vertex[20H], xmm2
	subps	xmm2, xmm0
	;
	; 法線取得
	;
	movaps	xmm3, xmm1
	movaps	xmm4, xmm2
	shufps	xmm1, xmm1, 11001001B
	shufps	xmm2, xmm2, 11010010B
	shufps	xmm3, xmm3, 11010010B
	shufps	xmm4, xmm4, 11001001B
	mulps	xmm1, xmm2
	mulps	xmm3, xmm4
	subps	xmm1, xmm3
	;
	mov	edx, pvNormal
	.IF	edx != NULL
		movups	xmm5, [edx]
		mulps	xmm5, xmm1
		movss	xmm3, xmmSignFlag
		movaps	xmm6, xmm5
		movhlps	xmm7, xmm5
		shufps	xmm5, xmm5, 1
		addss	xmm6, xmm7
		addss	xmm5, xmm6
		andps	xmm3, xmm5
		shufps	xmm3, xmm3, 0
		xorps	xmm1, xmm3
	.ENDIF
	ASSUME	esi:NOTHING
	;
	movaps	xmm0, xmm1
	mulps	xmm1, xmm1
	movaps	xmm2, xmm1
	movhlps	xmm3, xmm1
	shufps	xmm1, xmm1, 1
	addss	xmm2, xmm3
	addss	xmm1, xmm2
	sqrtss	xmm1, xmm1
	shufps	xmm1, xmm1, 0
	divps	xmm0, xmm1
	movaps	[edi].normal, xmm0
	;
	; 各辺のベクトルと長さを計算
	;
	movaps	xmm0, [edi].vertex[0]
	movaps	xmm3, [edi].vertex[10H]
	movaps	xmm2, [edi].vertex[20H]
	movaps	xmm1, xmm2
	subps	xmm2, xmm0
	subps	xmm0, xmm3
	subps	xmm1, xmm3
	;
	@INDEX = 0
	FOR	@DUMMY, <xmm0, xmm1, xmm2>
		movaps	xmm3, @DUMMY
		mulps	xmm3, xmm3
		movaps	xmm4, xmm3
		movhlps	xmm5, xmm3
		shufps	xmm3, xmm3, 1
		addss	xmm4, xmm5
		addss	xmm3, xmm4
		sqrtss	xmm3, xmm3
		movss	[edi].vertex[@INDEX].d, xmm3
		@INDEX = @INDEX + 10H
	ENDM
	;
	; cos ∠ABC の計算
	;
	mulps	xmm0, xmm1
	movss	xmm6, [edi].vertex[0].d
	movss	xmm7, [edi].vertex[10H].d
	mulss	xmm6, xmm7
	movaps	xmm4, xmm0
	movhlps	xmm5, xmm0
	shufps	xmm0, xmm0, 1
	addss	xmm4, xmm5
	addss	xmm0, xmm4
	divss	xmm0, xmm6
	movss	[edi].cos_abc, xmm0
	;
	; cos ∠ACB の計算
	;
	mulps	xmm1, xmm2
	movss	xmm6, [edi].vertex[10H].d
	movss	xmm7, [edi].vertex[20H].d
	mulss	xmm6, xmm7
	movaps	xmm4, xmm1
	movhlps	xmm5, xmm1
	shufps	xmm1, xmm1, 1
	addss	xmm4, xmm5
	addss	xmm1, xmm4
	divss	xmm1, xmm6
	movss	[edi].cos_acb, xmm1

	ASSUME	ebx:NOTHING
	ASSUME	edi:NOTHING
	ret

eglModelMatrix@InitializeMatrixInfoSSE	ENDP

;
;	モデル当たり判定オブジェクト初期化 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglModelMatrix@InitializeSSE	PROC	NEAR32 C USES ebx esi edi,
	hMatrix:HEGL_MODEL_MATRIX,
	pModel:PTR PCE3D_PRIMITIVE_POLYGON, nPolyCount:DWORD

	LOCAL	pvVertexBuf:PCE3D_VECTOR4
	LOCAL	pvNormalBuf:PCE3D_VECTOR4
	LOCAL	pv1:PCE3D_VECTOR4, pv2:PCE3D_VECTOR4, pv3:PCE3D_VECTOR4
	LOCAL	nMeshPolyCount:DWORD

	mov	ebx, hMatrix
	ASSUME	ebx:PTR EGL_MODEL_MATRIX_BUF
	mov	[ebx].fModelSized, 0
	;
	; ポリゴン数カウント
	;
	xor	ecx, ecx
	xor	edi, edi
	.WHILE	ecx < nPolyCount
		mov	esi, pModel
		mov	esi, [esi + ecx * 4]
		ASSUME	esi:PCE3D_PRIMITIVE_POLYGON
		.IF	esi != NULL
			.IF	[esi].dwTypeFlag & E3D_MESH_POLYGON
				mov	eax, [esi].dwVertexCount
				add	edi, (E3D_PRIMITIVE_MESH_LIST PTR [esi].mesh.uv_map[eax*8]).dwPolyCount
			.ELSEIF	!([esi].dwTypeFlag & (NOT E3D_POLYGON_PRIMITIVE_MASK))
				inc	edi
			.ENDIF
		.ENDIF
		ASSUME	esi:NOTHING
		inc	ecx
	.ENDW
	mov	nMeshPolyCount, edi
	;
	; メモリ確保
	;
	.IF	[ebx].pPolyMatrixBuf != NULL
		mov	edx, nMeshPolyCount
		.IF	edx == [ebx].nPolygonCount
			mov	eax, [ebx].pPolyMatrix
			jmp	Label_BeginParameter
		.ENDIF
		INVOKE	eslHeapFree , EGL_hImageHeap, [ebx].pPolyMatrixBuf, 0
	.ENDIF
	;
	mov	edx, nMeshPolyCount
	mov	[ebx].nPolygonCount, edx
	imul	edx, @POLY_MATRIX_SIZE
	add	edx, 10H
	INVOKE	eslHeapAllocate , EGL_hImageHeap, edx, 0
	mov	[ebx].pPolyMatrixBuf, eax
	add	eax, 0FH
	and	eax, NOT 0FH
	mov	[ebx].pPolyMatrix, eax
	;
	; 当たり判定用パラメータ計算
	;
Label_BeginParameter:
	mov	edi, eax
	ASSUME	edi:PEGL_POLY_MATRIX_INFO
	xor	ecx, ecx
	mov	[ebx].nPolygonCount, ecx
	.WHILE	ecx < nPolyCount
		mov	esi, pModel
		mov	esi, [esi + ecx * 4]
		ASSUME	esi:PCE3D_PRIMITIVE_POLYGON
		.IF	esi != NULL
		.IF	[esi].dwTypeFlag & E3D_MESH_POLYGON
			push	ecx
			;
			mov	eax, [esi].mesh.vertices
			mov	edx, [esi].mesh.normals
			mov	pvVertexBuf, eax
			mov	pvNormalBuf, edx
			mov	eax, [esi].dwVertexCount
			lea	esi, [esi].mesh.uv_map[eax*8]
			ASSUME	esi:PTR E3D_PRIMITIVE_MESH_LIST
			;
			mov	ecx, [esi].dwPolyCount
			lea	esi, [esi].mpEntries[0]
			ASSUME	esi:PTR E3D_PRIMITIVE_MESH_POLY
			;
			test	ecx, ecx
			.WHILE	!ZERO?
				push	ecx
				.IF	[esi].dwVertexCount >= 3
					mov	eax, [esi].dwIndex[0]
					mov	edx, [esi].dwIndex[4]
					shl	eax, 4
					shl	edx, 4
					add	eax, pvVertexBuf
					add	edx, pvVertexBuf
					mov	pv1, eax
					mov	pv2, edx
					;
					mov	eax, [esi].dwIndex[8]
					mov	edx, [esi].dwIndex[0]
					shl	eax, 4
					shl	edx, 4
					add	eax, pvVertexBuf
					;
					.IF	pvNormalBuf != NULL
						add	edx, pvNormalBuf
					.ELSE
						xor	edx, edx
					.ENDIF
					mov	pv3, eax
					;
					INVOKE	eglModelMatrix@InitializeMatrixInfoSSE ,
									pv1, pv2, pv3, edx
					;
					mov	eax, [esi].dwVertexCount
					add	edi, @POLY_MATRIX_SIZE
					inc	[ebx].nPolygonCount
					lea	esi, [esi].dwIndex[eax*4]
				.ENDIF
				;
				pop	ecx
				dec	ecx
			.ENDW
			;
			ASSUME	esi:PCE3D_PRIMITIVE_POLYGON
			;
			pop	ecx

		.ELSEIF	!([esi].dwTypeFlag & (NOT E3D_POLYGON_PRIMITIVE_MASK)) \
						&& ([esi].dwVertexCount >= 3)
			push	ecx
			@VERTEX_SIZE = (SIZEOF E3D_PRIMITIVE_VERTEX)
			mov	eax, [esi].polygon[0].vertex
			mov	pv1, eax
			mov	eax, [esi].polygon[@VERTEX_SIZE].vertex
			mov	pv2, eax
			mov	eax, [esi].polygon[@VERTEX_SIZE*2].vertex
			mov	pv3, eax
			;
			xor	eax, eax
			.IF	[esi].dwTypeFlag & E3D_SMOOTH_POLYGON
				mov	eax, [esi].polygon[0].normal
			.ENDIF
			;
			INVOKE	eglModelMatrix@InitializeMatrixInfoSSE ,
							pv1, pv2, pv3, eax
			;
			pop	ecx
			add	edi, @POLY_MATRIX_SIZE
			inc	[ebx].nPolygonCount
		.ENDIF
		.ENDIF
		inc	ecx
	.ENDW
	;
	; モデル全体サイズ計算
	;
	movups	xmm0, [ebx].vMinModelPos
	movups	xmm1, [ebx].vMaxModelPos
	movaps	xmm4, xmm0
	addps	xmm4, xmm1
		subps	xmm1, xmm0
	movss	xmm2, rConstHalf
	movss	xmm3, rConst1p001
		mulps	xmm1, xmm1
	shufps	xmm2, xmm2, 0
	shufps	xmm3, xmm3, 0
	mulps	xmm4, xmm2
		movhlps	xmm0, xmm1
		addss	xmm0, xmm1
		shufps	xmm1, xmm1, 1
		addss	xmm0, xmm1
		sqrtss	xmm0, xmm0
		mulss	xmm0, xmm2
		mulss	xmm0, xmm3
	shufps	xmm0, xmm4, 11100100B
	shufps	xmm4, xmm0, 00100100B
	movups	[ebx].vMatrixSphere, xmm4
	;
	ASSUME	edi:NOTHING
	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglModelMatrix@InitializeSSE	ENDP

;
;	内包空間と球体との交差判定
; ----------------------------------------------------------------------------
ALIGN	10H
eglModelMatrix@IsHitInclusiveSphereSSE	PROC	NEAR32 C USES ebx esi edi,
	hMatrix:HEGL_MODEL_MATRIX,
	pSphere:PTR E3D_VECTOR, rRadius:REAL4, pHitResult:PTR SDWORD

	mov	ebx, hMatrix
	ASSUME	ebx:PTR EGL_MODEL_MATRIX_BUF
	;
	; 球の中心座標取得
	;
	mov	esi, pSphere
	movlps	xmm7, QWORD PTR [esi]
	movss	xmm6, REAL4 PTR [esi + 8]
	movlhps	xmm7, xmm6
	movss	xmm6, rRadius
	;
	; オブジェクトの外接球との当たり判定
	;
	movups	xmm0, [ebx].vMatrixSphere
		movss	xmm3, [ebx].vMatrixSphere.d
	subps	xmm0, xmm7
	mulps	xmm0, xmm0
		mulss	xmm3, xmm3
	movhlps	xmm1, xmm0
	addss	xmm1, xmm0
	shufps	xmm0, xmm0, 1
	addss	xmm0, xmm1
	;
	comiss	xmm3, xmm0
	.IF	CARRY?
		mov	edx, pHitResult
		xor	eax, eax
		mov	SDWORD PTR [edx], 0
		ret
	.ENDIF
	;
	; 順次面判定
	;
	mov	ecx, [ebx].nPolygonCount
	mov	esi, [ebx].pPolyMatrix
	ASSUME	esi:PTR EGL_POLY_MATRIX_INFO
	test	ecx, ecx
	.WHILE	!ZERO?
		movaps	xmm0, xmm7
		movaps	xmm1, [esi].vertex[0]
		movaps	xmm2, [esi].normal
		subps	xmm0, xmm1
		mulps	xmm0, xmm2
		movaps	xmm1, xmm0
		movhlps	xmm2, xmm0
		shufps	xmm0, xmm0, 1
		addss	xmm1, xmm2
		addss	xmm0, xmm1
		comiss	xmm0, xmm6
		.IF	!CARRY?
			mov	edx, pHitResult
			xor	eax, eax
			mov	SDWORD PTR [edx], 0
			ret
		.ENDIF
		add	esi, (SIZEOF EGL_POLY_MATRIX_INFO)
		dec	ecx
	.ENDW
	;
	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	mov	edx, pHitResult
	xor	eax, eax
	mov	SDWORD PTR [edx], -1
	ret

eglModelMatrix@IsHitInclusiveSphereSSE	ENDP

;
;	指定のポリゴンの領域内の座標かどうかを判定
; ----------------------------------------------------------------------------
;　パラメータ；
;	[esi] : PTR EGL_POLY_MATRIX_INFO = ポリゴン情報
;	[edi] : PTR E3D_VECTOR = 同平面上の座標
;  返り値；
;	CARRY? : = 1 ; 領域外
;	CARRY? : = 0 ; 領域内
; ----------------------------------------------------------------------------
ALIGN	10H
eglModelMatrix@IsInclusiveTriangleSSE	PROC	NEAR32 C PRIVATE, rRadius:REAL4

	ASSUME	esi:PTR EGL_POLY_MATRIX_INFO
	ASSUME	edi:PTR E3D_VECTOR
	;
	;	cos ∠ABC と cos ∠PBA, cos ∠PBC の比較
	; --------------------------------------------------------------------
	;
	; P-B の計算
	;
	movlps	xmm4, QWORD PTR [edi]
	movss	xmm5, [edi].z
	movaps	xmm6, [esi].vertex[10H]
	movlhps	xmm4, xmm5
	subps	xmm4, xmm6
	movaps	xmm7, xmm4		; xmm7 = p - p2
	mulps	xmm4, xmm4
	movss	xmm5, xmm4
	movhlps	xmm6, xmm4
	shufps	xmm4, xmm4, 1
	addss	xmm5, xmm6
	addss	xmm4, xmm5
	sqrtss	xmm6, xmm4		; xmm6 = |p - p2|
	;
	; 判定角補正
	;
	xorps	xmm0, xmm0
	movss	xmm3, rRadius
	movss	xmm5, [esi].cos_abc
	comiss	xmm6, xmm0
	.IF	!ZERO?
		divss	xmm3, xmm6
	.ELSE
		movss	xmm3, rConst2
	.ENDIF
	subss	xmm5, xmm3		; xmm5 = cos'∠ABC
	;
	; cos ∠ABC と cos ∠PBA を比較
	;
	movaps	xmm0, [esi].vertex[0]
	subps	xmm0, [esi].vertex[10H]
	mulps	xmm0, xmm7
		movss	xmm3, [esi].vertex[0].d
	movss	xmm1, xmm0
	movhlps	xmm2, xmm0
	shufps	xmm0, xmm0, 1
	addss	xmm1, xmm2
		mulss	xmm3, xmm6
	addss	xmm0, xmm1
	;
	divss	xmm0, xmm3
	comiss	xmm0, xmm5
	jc	Label_Return
	;
	; cos ∠ABC と cos ∠PBC を比較
	;
	movaps	xmm0, [esi].vertex[20H]
	subps	xmm0, [esi].vertex[10H]
	mulps	xmm0, xmm7
		movss	xmm3, [esi].vertex[10H].d
	movss	xmm1, xmm0
	movhlps	xmm2, xmm0
	shufps	xmm0, xmm0, 1
	addss	xmm1, xmm2
		mulss	xmm3, xmm6
	addss	xmm0, xmm1
	;
	divss	xmm0, xmm3
	comiss	xmm0, xmm5
	jc	Label_Return
	;
	;	cos ∠ACB と cos ∠PCA, cos ∠PCB の比較
	; --------------------------------------------------------------------
	;
	; P-C の計算
	;
	movlps	xmm4, QWORD PTR [edi]
	movss	xmm5, [edi].z
	movaps	xmm6, [esi].vertex[20H]
	movlhps	xmm4, xmm5
	subps	xmm4, xmm6
	movaps	xmm7, xmm4		; xmm7 = p - p3
	mulps	xmm4, xmm4
	movss	xmm5, xmm4
	movhlps	xmm6, xmm4
	shufps	xmm4, xmm4, 1
	addss	xmm5, xmm6
	addss	xmm4, xmm5
	sqrtss	xmm6, xmm4		; xmm6 = |p - p3|
	;
	; 判定角補正
	;
	xorps	xmm0, xmm0
	movss	xmm3, rRadius
	movss	xmm5, [esi].cos_acb
	comiss	xmm3, xmm0
	.IF	!ZERO?
		divss	xmm3, xmm6
	.ELSE
		movss	xmm3, rConst2
	.ENDIF
	subss	xmm5, xmm3		; xmm5 = cos'∠ACB
	;
	; cos ∠ACB と cos ∠PCA を比較
	;
	movaps	xmm0, [esi].vertex[0]
	subps	xmm0, [esi].vertex[20H]
	mulps	xmm0, xmm7
		movss	xmm3, [esi].vertex[20H].d
	movss	xmm1, xmm0
	movhlps	xmm2, xmm0
	shufps	xmm0, xmm0, 1
	addss	xmm1, xmm2
		mulss	xmm3, xmm6
	addss	xmm0, xmm1
	;
	divss	xmm0, xmm3
	comiss	xmm0, xmm5
	jc	Label_Return
	;
	; cos ∠ACB と cos ∠PCB を比較
	;
	movaps	xmm0, [esi].vertex[10H]
	subps	xmm0, [esi].vertex[20H]
	mulps	xmm0, xmm7
		movss	xmm3, [esi].vertex[10H].d
	movss	xmm1, xmm0
	movhlps	xmm2, xmm0
	shufps	xmm0, xmm0, 1
	addss	xmm1, xmm2
		mulss	xmm3, xmm6
	addss	xmm0, xmm1
	;
	divss	xmm0, xmm3
	comiss	xmm0, xmm5
	;
	ASSUME	esi:NOTHING
	ASSUME	edi:NOTHING
Label_Return:
	sbb	eax, eax
	ret

eglModelMatrix@IsInclusiveTriangleSSE	ENDP

;
;	ポリゴンと球体との交差判定 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglModelMatrix@IsHitAgainstSphereSSE	PROC	NEAR32 C USES ebx esi edi,
	hMatrix:HEGL_MODEL_MATRIX,
	pSphere:PTR E3D_VECTOR, rRadius:REAL4,
	pHitResult:PTR SDWORD, pHitPos:PTR E3D_VECTOR,
	pHitNormal:PTR E3D_VECTOR, pReflection:PTR E3D_VECTOR

	LOCAL	rDistance:REAL4
	LOCAL	vPos:E3D_VECTOR4
	LOCAL	vSphere:E3D_VECTOR4
	LOCAL	vReflection:E3D_VECTOR4

	mov	ebx, hMatrix
	ASSUME	ebx:PTR EGL_MODEL_MATRIX_BUF
	;
	; 球の中心座標取得
	;
	mov	esi, pSphere
	movlps	xmm0, QWORD PTR [esi]
	movss	xmm1, REAL4 PTR [esi + 8]
	movlhps	xmm0, xmm1
	movups	vSphere, xmm0
	;
	; オブジェクトの外接球との当たり判定
	;
	movaps	xmm7, xmm0
	movups	xmm0, [ebx].vMatrixSphere
		movss	xmm3, [ebx].vMatrixSphere.d
	subps	xmm0, xmm7
	mulps	xmm0, xmm0
		mulss	xmm3, xmm3
	movhlps	xmm1, xmm0
	addss	xmm1, xmm0
	shufps	xmm0, xmm0, 1
	addss	xmm0, xmm1
	;
	comiss	xmm3, xmm0
	.IF	CARRY?
		mov	edx, pHitResult
		xor	eax, eax
		mov	SDWORD PTR [edx], 0
		ret
	.ENDIF
	;
	; 反射ベクトル取得
	;
	mov	esi, pReflection
	.IF	esi != NULL
		movlps	xmm0, QWORD PTR [esi]
		movss	xmm1, REAL4 PTR [esi + 8]
		movlhps	xmm0, xmm1
		movups	vReflection, xmm0
	.ENDIF
	;
	; 順次面判定
	;
	mov	edx, pHitResult
	mov	SDWORD PTR [edx], 0
	mov	ecx, [ebx].nPolygonCount
	mov	esi, [ebx].pPolyMatrix
	ASSUME	esi:PTR EGL_POLY_MATRIX_INFO
	test	ecx, ecx
	.WHILE	!ZERO?
		;
		; 面との距離判定
		;
		movups	xmm0, vSphere
		subps	xmm0, [esi].vertex[0]
		mulps	xmm0, [esi].normal
		movss	xmm1, xmm0
		movhlps	xmm2, xmm0
		shufps	xmm0, xmm0, 1
		addss	xmm1, xmm2
		movss	xmm4, xmmMaskSignFlag
		addss	xmm0, xmm1
		andps	xmm4, xmm0
		shufps	xmm0, xmm0, 0
		movss	rDistance, xmm4
		comiss	xmm4, rRadius
		.IF	CARRY?
			;
			; 三角形の領域内か判定
			;
			push	ecx
			movups	xmm1, vSphere
			mulps	xmm0, [esi].normal
			subps	xmm1, xmm0
			movups	vPos, xmm1
			lea	edi, vPos
			INVOKE	eglModelMatrix@IsInclusiveTriangleSSE , rRadius
			test	eax, eax
			.IF	ZERO?
				;
				; パラメータ設定
				;
				mov	ecx, pHitResult
				mov	eax, rDistance
				mov	edx, pHitPos
				mov	edi, pHitNormal
				mov	SDWORD PTR [ecx], -1
				mov	rRadius, eax
				.IF	edx != NULL
					movups	xmm0, vPos
					movlps	QWORD PTR [edx], xmm0
					movhlps	xmm0, xmm0
					movss	REAL4 PTR [edx + 8], xmm0
				.ENDIF
				.IF	edi != NULL
					movaps	xmm0, [esi].normal
					movlps	QWORD PTR [edi], xmm0
					movhlps	xmm0, xmm0
					movss	REAL4 PTR [edi + 8], xmm0
				.ENDIF
				mov	edi, pReflection
				.IF	edi != NULL
					movups	xmm0, vReflection
					movaps	xmm3, xmm0
					mulps	xmm0, [esi].normal
					movss	xmm1, xmm0
					movhlps	xmm2, xmm0
					shufps	xmm0, xmm0, 1
					addss	xmm1, xmm2
					addss	xmm0, xmm1
					;
					movaps	xmm1, [esi].normal
					shufps	xmm0, xmm0, 0
					mulps	xmm0, xmm1
					addps	xmm0, xmm0
					subps	xmm3, xmm0
					movlps	QWORD PTR [edi], xmm3
					movhlps	xmm3, xmm3
					movss	REAL4 PTR [edi + 8], xmm3
				.ENDIF
			.ENDIF
			pop	ecx
		.ENDIF
		add	esi, (SIZEOF EGL_POLY_MATRIX_INFO)
		dec	ecx
	.ENDW
	;
	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglModelMatrix@IsHitAgainstSphereSSE	ENDP

;
;	ポリゴンと線分との交差判定
; ----------------------------------------------------------------------------
ALIGN	10H
eglModelMatrix@IsCrossingSegmentSSE	PROC	NEAR32 C USES ebx esi edi,
	hMatrix:HEGL_MODEL_MATRIX,
	pPos0:PTR E3D_VECTOR, pPos1:PTR E3D_VECTOR, rErrorGap:REAL4,
	pHitResult:PTR SDWORD, pHitPos:PTR E3D_VECTOR,
	pHitNormal:PTR E3D_VECTOR, pReflection:PTR E3D_VECTOR

	LOCAL	vPos0:E3D_VECTOR4, vPos1:E3D_VECTOR4
	LOCAL	vCrossPos:E3D_VECTOR4
	LOCAL	vDeltaP1P0:E3D_VECTOR4
	LOCAL	rSqrAbsDeltaP1P0:REAL4	; |P1 - P0|^2
	LOCAL	rAbsDeltaP1P0:REAL4	; |P1 - P0|
	LOCAL	rSqrAbsDeltaXP0:REAL4	; |X - P0|^2
	LOCAL	rDistanceModel:REAL4	; (X - P0 | P1 - P0) / |P1 - P0|
	LOCAL	rMinDeltaT:REAL4
	LOCAL	vInnerPA:REAL4
	LOCAL	rTemp:REAL4

	mov	ebx, hMatrix
	ASSUME	ebx:PTR EGL_MODEL_MATRIX_BUF
	;
	mov	edx, pHitResult
	mov	SDWORD PTR [edx], 0
	;
	; 線分の座標
	;
	mov	esi, pPos0
	mov	edi, pPos1
	movlps	xmm0, QWORD PTR [esi]
	movlps	xmm2, QWORD PTR [edi]
	movss	xmm1, REAL4 PTR [esi + 8]
	movss	xmm3, REAL4 PTR [edi + 8]
	movlhps	xmm0, xmm1
	movlhps	xmm2, xmm3
	movups	vPos0, xmm0
	movups	vPos1, xmm2
	subps	xmm2, xmm0
	movups	vDeltaP1P0, xmm2
	;
	mulps	xmm2, xmm2
	movhlps	xmm0, xmm2
	addss	xmm0, xmm2
	shufps	xmm2, xmm2, 1
	addss	xmm0, xmm2
	movss	rSqrAbsDeltaP1P0, xmm0
	;
	sqrtss	xmm0, xmm0
	movss	rAbsDeltaP1P0, xmm0
	;
	comiss	xmm0, rConstZeroLittle
	.IF	CARRY?
		xor	eax, eax
		ret
	.ENDIF
	;
	; モデル中心点までの距離を計算
	;
	movups	xmm0, [ebx].vMatrixSphere
	movups	xmm1, vPos0
	subps	xmm0, xmm1
	mulps	xmm0, xmm0
	;
	movhlps	xmm1, xmm0
	addss	xmm1, xmm0
	shufps	xmm0, xmm0, 1
	addss	xmm0, xmm1
	movss	rSqrAbsDeltaXP0, xmm0
	;
	; モデル中心点までの線分方向の距離を計算
	;
	movups	xmm0, [ebx].vMatrixSphere
	movups	xmm1, vPos0
	movups	xmm2, vDeltaP1P0
	subps	xmm0, xmm1
	mulps	xmm0, xmm2
	;
	movhlps	xmm1, xmm0
	addss	xmm1, xmm0
	shufps	xmm0, xmm0, 1
	addss	xmm0, xmm1
	divss	xmm0, rAbsDeltaP1P0
	movss	rDistanceModel, xmm0
	;
	; 距離判定
	;
	mulss	xmm0, xmm0
		movss	xmm2, [ebx].vMatrixSphere.d
	movss	xmm1, rSqrAbsDeltaXP0
		mulss	xmm2, xmm2
	subss	xmm1, xmm0
	;
	comiss	xmm2, xmm1
	.IF	CARRY?		; radius^2 < rSqrAbsDeltaXP0 - rDistanceModel^2 ?
		xor	eax, eax
		ret
	.ENDIF
	;
	.IF	rDistanceModel & 80000000H
		comiss	xmm2, rSqrAbsDeltaXP0
		.IF	CARRY?		; radius^2 < rSqrAbsDeltaXP0
			xor	eax, eax
			ret
		.ENDIF
	.ELSE
		movss	xmm0, [ebx].vMatrixSphere.d
		addss	xmm0, rAbsDeltaP1P0
		comiss	xmm0, rDistanceModel
		.IF	CARRY?		; |P1 - P0| + radius < rDistanceModel
			xor	eax, eax
			ret
		.ENDIF
	.ENDIF
	;
	; 線分順次交差判定
	;
	mov	rMinDeltaT, 3F8003FFH		; 1.000000
	mov	ecx, [ebx].nPolygonCount
	mov	esi, [ebx].pPolyMatrix
	ASSUME	esi:PTR EGL_POLY_MATRIX_INFO
	test	ecx, ecx
	.WHILE	!ZERO?
		;
		; 交差点までの t [0,1] を求める
		;
		movups	xmm0, vDeltaP1P0
		movaps	xmm4, [esi].normal
		mulps	xmm0, xmm4
		movss	xmm1, xmm0
		movhlps	xmm2, xmm0
		shufps	xmm0, xmm0, 1
		addss	xmm1, xmm2
		addss	xmm0, xmm1
		movss	vInnerPA, xmm0
		mov	eax, vInnerPA
		and	eax, 7FFFFFFFH
		cmp	eax, 01800000H
		jb	Loop_Continue
		;
		movups	xmm2, vPos0
		movaps	xmm1, [esi].vertex[0]
		subps	xmm1, xmm2
		mulps	xmm1, xmm4
		movss	xmm2, xmm1
		movhlps	xmm3, xmm1
		shufps	xmm1, xmm1, 1
		addss	xmm2, xmm3
		addss	xmm1, xmm2
		divss	xmm1, xmm0
		xorps	xmm2, xmm2
		movss	xmm3, rMinDeltaT
		comiss	xmm1, xmm2
		jc	Loop_Continue
		comiss	xmm1, xmm3
		shufps	xmm1, xmm1, 0
		.IF	CARRY?
			push	ecx
			;
			; 交差点の座標を求める
			;
			movss	rTemp, xmm1
			movups	xmm2, vDeltaP1P0
			movups	xmm0, vPos0
			mulps	xmm2, xmm1
			addps	xmm0, xmm2
			movups	vCrossPos, xmm0
			;
			; 三角形の領域内か判定する
			;
			lea	edi, vCrossPos
			INVOKE	eglModelMatrix@IsInclusiveTriangleSSE , rErrorGap
			test	eax, eax
			.IF	ZERO?
			;
			; パラメータ設定
			;
			mov	edx, pHitResult
			mov	eax, rTemp
			mov	rMinDeltaT, eax
			mov	SDWORD PTR [edx], -1
			mov	edx, pHitPos
			mov	edi, pHitNormal
			.IF	edx != NULL
				movups	xmm0, vCrossPos
				movlps	QWORD PTR [edx], xmm0
				movhlps	xmm0, xmm0
				movss	REAL4 PTR [edx + 8], xmm0
			.ENDIF
			.IF	edi != NULL
				movaps	xmm0, [esi].normal
				movlps	QWORD PTR [edi], xmm0
				movhlps	xmm0, xmm0
				movss	REAL4 PTR [edi + 8], xmm0
			.ENDIF
			mov	edi, pReflection
			.IF	edi != NULL
				movss	xmm0, vInnerPA
				movaps	xmm1, [esi].normal
				shufps	xmm0, xmm0, 0
				mulps	xmm0, xmm1
				movups	xmm2, vDeltaP1P0
				addps	xmm0, xmm0
				subps	xmm2, xmm0
				movlps	QWORD PTR [edi], xmm2
				movhlps	xmm2, xmm2
				movss	REAL4 PTR [edi + 8], xmm2
			.ENDIF
			;
			.ENDIF
			pop	ecx
		.ENDIF
Loop_Continue:	;
		add	esi, (SIZEOF EGL_POLY_MATRIX_INFO)
		dec	ecx
	.ENDW
	;
	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglModelMatrix@IsCrossingSegmentSSE	ENDP


CodeSeg	ENDS

	END
