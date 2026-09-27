
; ----------------------------------------------------------------------------
;                          Entis Graphic Library
; ----------------------------------------------------------------------------
;    Copyright (c) 2002-2012 Leshade Entis, Entis-soft. Al rights reserved.
; ----------------------------------------------------------------------------


	.486
	.387
	.MODEL	FLAT

	INCLUDE	experi.inc
	INCLUDE	egl.inc



; ----------------------------------------------------------------------------
;	データセグメント
; ----------------------------------------------------------------------------

ConstSeg	SEGMENT	PARA READONLY FLAT 'CONST'

ALIGN	10H
rConstR2PI	REAL4	4 DUP( 0.159154943 )	; = 0.5 / π
rConstHalfPI	REAL4	4 DUP( 1.570796327 )	; = π / 2
rConstPI	REAL4	4 DUP( 3.141592654 )	; = π
;rConstLittle	DWORD	4 DUP( 33800000H )	; = 2^-24
rConstLittle	REAL4	4 DUP( 0.0000000000001 )
;rConstSinLittle	DWORD	4 DUP( 3C400000H )	; = 1.5/128 ≒ sin(0.67[deg])
rConstSinLittle	REAL4	4 DUP( 0.0000000000001 )
xmmMaskSign	DWORD	4 DUP( 7FFFFFFFH )
xmmMaskSignBit	DWORD	4 DUP( 80000000H )
xmmMaskFull	DWORD	4 DUP( 0FFFFFFFFH )
xmmConst1	REAL4	1.0, 1.0, 1.0, 1.0
xmmRcp3		REAL4	4 DUP( 0.333333333 )	; = 1.0 / 3.0
xmmRcp5		REAL4	4 DUP( 0.2 )		; = 1.0 / 5.0
xmmRcp7		REAL4	4 DUP( 0.142857142 )	; = 1.0 / 7.0
xmmRcp1024	REAL4	4 DUP( 0.000976562 )	; = 1.0 / 1024.0
xmmRevHalfPI	REAL4	-0.707, 0.707, 0.707, -0.707
xmmMaskLP3	DWORD	0FFFFFFFFH, 0FFFFFFFFH, 0FFFFFFFFH, 0
xmmMaskLS1	DWORD	80000000H, 0, 0, 0
xmmHalf3_Zero	REAL4	0.5, 0.5, 0.5, 0.0

ConstSeg	ENDS


; ----------------------------------------------------------------------------
;	コードセグメント
; ----------------------------------------------------------------------------

CodeSeg	SEGMENT	PARA READONLY FLAT 'CODE'

;
;	テクスチャマッピングパラメータ計算 486 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@SetTextureParameter486	PROC	NEAR32 C USES ebx esi edi,
	ptxmap:PTR E3D_TEXTURE_MAPINFO,
	vertex:PTR E3D_VECTOR4, uvmap:PTR E3D_VECTOR_2D

	LOCAL	uv1:E3D_VECTOR_2D, v1:E3D_VECTOR4
	LOCAL	uv2:E3D_VECTOR_2D, v2:E3D_VECTOR4
	LOCAL	rTemp:REAL4

	mov	ebx, ptxmap
	mov	esi, vertex
	mov	edi, uvmap
	ASSUME	ebx:PTR E3D_TEXTURE_MAPINFO
	ASSUME	esi:PTR E3D_VECTOR4
	ASSUME	edi:PTR E3D_VECTOR_2D
	;
	; テクスチャマッピング軸を計算
	;
	fld	[edi][8].x		; v1 を取得
	fsub	[edi].x
	fld	[edi][8].y
	fsub	[edi].y
	fxch	st(1)
	fstp	uv1.x
	fstp	uv1.y
	;
	fld	[esi][10H].x
	fsub	[esi].x
	fld	[esi][10H].y
	fsub	[esi].y
	fld	[esi][10H].z
	fsub	[esi].z
	fxch	st(2)
	fstp	v1.x
	fstp	v1.y
	fstp	v1.z
	;
	fld	[edi][16].x		; v2 を取得
	fsub	[edi].x
	fld	[edi][16].y
	fsub	[edi].y
	fxch	st(1)
	fstp	uv2.x
	fstp	uv2.y
	;
	fld	[esi][20H].x
	fsub	[esi].x
	fld	[esi][20H].y
	fsub	[esi].y
	fld	[esi][20H].z
	fsub	[esi].z
	fxch	st(2)
	fstp	v2.x
	fstp	v2.y
	fstp	v2.z
	;
	mov	eax, uv1.x
	mov	edx, uv1.y
	and	eax, 7FFFFFFFH
	and	edx, 7FFFFFFFH
	.IF	(eax <= 20000000H) && (edx <= 20000000H)
		mov	eax, uv2.x
		mov	edx, uv2.y
		and	eax, 7FFFFFFFH
		and	edx, 7FFFFFFFH
		.IF	(eax <= 01000000H) && (edx <= 01000000H)
			mov	uv1.x, 3E000000H
			mov	uv1.y, 0
			mov	uv2.x, 0
			mov	uv2.y, 3E000000H
		.ELSE
			mov	eax, uv2.x
			mov	edx, uv2.y
			sub	eax, 01800000H
			sub	edx, 01800000H
			xor	edx, 80000000H
			mov	uv1.y, eax
			mov	uv1.x, edx
		.ENDIF
	.ELSE
		mov	eax, uv2.x
		mov	edx, uv2.y
		and	eax, 7FFFFFFFH
		and	edx, 7FFFFFFFH
		.IF	(eax <= 20000000H) && (edx <= 20000000H)
			mov	eax, uv1.x
			mov	edx, uv1.y
			sub	eax, 01800000H
			sub	edx, 01800000H
			xor	edx, 80000000H
			mov	uv2.y, eax
			mov	uv2.x, edx
		.ENDIF
	.ENDIF
	;
	; テクスチャマッピングｘ軸を計算
	;
	fld1			; st(0) = 1 / Da
	fld	uv1.x
	fmul	uv2.y
	fld	uv1.y
	fmul	uv2.x
	fsubp	st(1), st
	fst	rTemp
	mov	eax, rTemp
	and	eax, 7FFFFFFFH
	.IF	eax <= 01000000H
		fstp	st(0)
	.ELSE
		fdivp	st(1), st
	.ENDIF
	;
	FOR	@MEMBER, <x, y, z>
		fld	v1.@MEMBER
		fmul	uv2.y
		fld	uv1.y
		fmul	v2.@MEMBER
		fsubp	st(1), st
		fmul	st, st(1)
		fstp	[ebx].vAxisX.@MEMBER
	ENDM
	;
	; テクスチャマッピングｙ軸を計算
	;
	fchs
	FOR	@MEMBER, <x, y, z>
		fld	v1.@MEMBER
		fmul	uv2.x
		fld	uv1.x
		fmul	v2.@MEMBER
		fsubp	st(1), st
		fmul	st, st(1)
		fstp	[ebx].vAxisY.@MEMBER
	ENDM
	fstp	st(0)
	;
	mov	[ebx].vAxisX.d, 0
	mov	[ebx].vAxisY.d, 0
	;
	; テクスチャマッピング相対中心座標を計算
	;
	fld	[ebx].vAxisX.z
	fld	[ebx].vAxisX.y
	fld	[ebx].vAxisX.x
	fld	[edi].x
	fmul	st(1), st
	fmul	st(2), st
	fmulp	st(3), st
	;
	fld	[ebx].vAxisY.z
	fld	[ebx].vAxisY.y
	fld	[ebx].vAxisY.x
	fld	[edi].y
	fmul	st(1), st
	fmul	st(2), st
	fmulp	st(3), st
	;
	faddp	st(3), st
	faddp	st(3), st
	faddp	st(3), st
	;
	fld	[esi].z
	fld	[esi].y
	fld	[esi].x
	fsubrp	st(3), st
	fsubrp	st(3), st
	fsubrp	st(3), st
	;
	fstp	[ebx].vOriginPos.x
	fstp	[ebx].vOriginPos.y
	fstp	[ebx].vOriginPos.z
	mov	[ebx].vOriginPos.d, 0
	;
	ASSUME	ebx:NOTHING
	ASSUME	esi:NOTHING
	ASSUME	edi:NOTHING
	ret

eglRenderPoly@SetTextureParameter486	ENDP

IF	0
;
;	環境マッピングパラメータ計算 486 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@SetEnvironmentMapping486	PROC	NEAR32 C PRIVATE USES ebx esi edi,
	ptxmap:PTR E3D_TEXTURE_MAPINFO,
	vertex:PTR E3D_VECTOR, normal:PTR E3D_VECTOR4,
	envmat:PTR E3D_REV_MATRIX, envmap:PTR E3D_ENVIRONMENT_MAPPING

	LOCAL	vNormals[3]:E3D_VECTOR4
	LOCAL	vUVMap[3]:E3D_VECTOR_2D

	;
	;	法線を回転
	; --------------------------------------------------------------------
	mov	esi, normal
	lea	edi, vNormals[0]
	mov	ebx, envmat
	ASSUME	esi:PTR E3D_VECTOR4
	ASSUME	edi:PTR E3D_VECTOR4
	ASSUME	ebx:PTR E3D_REV_MATRIX
	mov	ecx, 3
	.REPEAT
		fld	[esi].z
		fld	[esi].y
		fld	[esi].x
		add	esi, (SIZEOF E3D_VECTOR4)
		;
		fld	[ebx].matrix[0][0]		; x 座標計算
		fld	[ebx].matrix[0][4]
		fld	[ebx].matrix[0][8]
		fxch	st(2)
		fmul	st, st(3)
		fxch	st(1)
		fmul	st, st(4)
		fxch	st(2)
		fmul	st, st(5)
		fxch	st(2)
		faddp	st(1), st
		faddp	st(1), st
		fstp	[edi].x
		;
		fld	[ebx].matrix[10H][0]		; y 座標計算
		fld	[ebx].matrix[10H][4]
		fld	[ebx].matrix[10H][8]
		fxch	st(2)
		fmul	st, st(3)
		fxch	st(1)
		fmul	st, st(4)
		fxch	st(2)
		fmul	st, st(5)
		fxch	st(2)
		faddp	st(1), st
		faddp	st(1), st
		fstp	[edi].y
		;
		fld	[ebx].matrix[20H][8]		; z 座標計算
		fld	[ebx].matrix[20H][4]
		fld	[ebx].matrix[20H][0]
		fmulp	st(3), st
		fmulp	st(3), st
		fmulp	st(3), st
		faddp	st(1), st
		faddp	st(1), st
		fstp	[edi].z
		add	edi, (SIZEOF E3D_VECTOR4)
		;
		dec	ecx
	.UNTIL	ZERO?
	ASSUME	ebx:NOTHING
	;
	;	法線への視線の反射を計算
	; --------------------------------------------------------------------
	mov	ecx, 3
	lea	edi, vNormals[0]
	mov	esi, vertex
	.REPEAT
		;
		; 視線ベクトルと法線の内積を計算
		;
		fld	[esi].z
		fld	[esi].y
		fld	[esi].x
		fld	[edi].x
		fld	[edi].y
		fld	[edi].z
		fxch	st(2)
		fmulp	st(3), st
		fmulp	st(3), st
		fmulp	st(3), st
		fxch	st(2)
		faddp	st(1), st
		faddp	st(1), st
		;
		; 反射ベクトルを計算
		;
		fadd	st, st(0)
		fld	[edi].y
		fld	[edi].x
		fld	[edi].z
		fxch	st(3)
		fmul	st(1), st
		fmul	st(2), st
		fmulp	st(3), st
		;
		fld	[esi].x
		fld	[esi].y
		fld	[esi].z
		fxch	st(2)
		fsubrp	st(3), st
		fsubrp	st(3), st
		fsubrp	st(3), st
		;
		; 反射ベクトルを正規化
		;
		fld	st(2)
		fmul	st, st(0)
		fld	st(2)
		fmul	st, st(0)
		fld	st(2)
		fmul	st, st(0)
		fxch	st(2)
		faddp	st(1), st
		faddp	st(1), st
		fsqrt
		fld1
		fdivrp	st(1), st
		;
		fmul	st(1), st
		fmul	st(2), st
		fmulp	st(3), st
		;
		; 反射ベクトルをストア
		;
		fstp	[edi].x
		fstp	[edi].y
		fstp	[edi].z
		add	esi, (SIZEOF E3D_VECTOR4)
		add	edi, (SIZEOF E3D_VECTOR4)
		;
		dec	ecx
	.UNTIL	ZERO?
	ASSUME	esi:NOTHING
	ASSUME	edi:NOTHING
	;
	;	マップ座標を計算
	; --------------------------------------------------------------------
	;
	; ｘ－ｚ平面上のベクトルの長さを計算
	;
	mov	ecx, 3
	lea	edi, vNormals[0]
	ASSUME	edi:PTR E3D_VECTOR4
	.REPEAT
		fld	[edi].x
		fld	[edi].z
		fxch	st(1)
		fmul	st, st(0)
		fxch	st(1)
		fmul	st, st(0)
		faddp	st(1), st
		fsqrt
		fstp	[edi].d
		add	edi, (SIZEOF E3D_VECTOR4)
		dec	ecx
	.UNTIL	ZERO?
	ASSUME	edi:NOTHING
	;
	; マッピングモードを特定
	;
	mov	eax, vNormals[0].y
	mov	ecx, vNormals[10H].y
	mov	edx, vNormals[20H].y
	xor	ecx, eax
	xor	edx, eax
	mov	ebx, ptxmap
	mov	esi, envmap
	or	ecx, edx	; ecx = 法線ｙ座標の符号が異なっている場合
	.IF	!SIGN?		; ecx <- 法線の傾きが45度以下である場合
		xor	edi, edi
		FOR	@INDEX, <0, 10H, 20H>
			mov	edx, vNormals[@INDEX].y
			and	edx, 7FFFFFFFH
			sub	edx, vNormals[@INDEX].d
			and	edi, edx
		ENDM
		or	ecx, edi
	.ENDIF
	ASSUME	ebx:PTR E3D_TEXTURE_MAPINFO
	ASSUME	esi:PTR E3D_ENVIRONMENT_MAPPING
	.IF	SIGN?
		;
		; ラウンドマッピングを使用
		;
		fld	rConstR2PI	; st(0) = 0.5 / π
		FOR	@INDEX, <0, 1, 2>
			fld	vNormals[@INDEX * 10H].z
			fld	vNormals[@INDEX * 10H].x
			fpatan
			fmul	st, st(1)
			fstp	vUVMap[@INDEX * 8].x
			;
			fld	vNormals[@INDEX * 10H].y
			fld	vNormals[@INDEX * 10H].d
			fpatan
			fldpi
			faddp	st(1), st
			fmul	st, st(1)
			fstp	vUVMap[@INDEX * 8].y
		ENDM
		fstp	st(0)
		;
		mov	eax, [esi].pRoundImage
		mov	edx, [esi].pRoundLuminous
		mov	[ebx].pTextureImage, eax
		mov	[ebx].pLuminousImage, edx

	.ELSEIF	!(eax & 80000000H)
		;
		; 上半球を使用
		;
		FOR	@INDEX, <0, 1, 2>
			mov	eax, vNormals[@INDEX * 10H].x
			mov	edx, vNormals[@INDEX * 10H].z
			mov	vUVMap[@INDEX * 8].x, eax
			mov	vUVMap[@INDEX * 8].y, edx
		ENDM
		mov	eax, [esi].pUpperImage
		mov	edx, [esi].pUpperLuminous
		mov	[ebx].pTextureImage, eax
		mov	[ebx].pLuminousImage, edx

	.ELSE
		;
		; 下半球を使用
		;
		FOR	@INDEX, <0, 1, 2>
			mov	eax, vNormals[@INDEX * 10H].x
			mov	edx, vNormals[@INDEX * 10H].z
			mov	vUVMap[@INDEX * 8].x, eax
			mov	vUVMap[@INDEX * 8].y, edx
		ENDM
		mov	eax, [esi].pUnderImage
		mov	edx, [esi].pUnderLuminous
		mov	[ebx].pTextureImage, eax
		mov	[ebx].pLuminousImage, edx
	.ENDIF
	ASSUME	esi:NOTHING
	;
	;	テクスチャ画像情報を適用
	; --------------------------------------------------------------------
	mov	eax, [ebx].pTextureImage
	ASSUME	eax:PTR EGL_IMAGE_INFO
	fild	[eax].dwImageHeight
	fild	[eax].dwImageWidth
	FOR	@INDEX, <0, 8, 16>
		fld	vUVMap[@INDEX].y
		fmul	st, st(2)
		fld	vUVMap[@INDEX].x
		fmul	st, st(2)
		fxch	st(1)
		fstp	vUVMap[@INDEX].y
		fstp	vUVMap[@INDEX].x
	ENDM
	fstp	st(0)
	fstp	st(0)
	ASSUME	ebx:NOTHING
	ASSUME	eax:NOTHING
	;
	INVOKE	eglRenderPoly@SetTextureParameter486,
			ptxmap, vertex, ADDR vUVMap[0]
	;
	ret

eglRenderPoly@SetEnvironmentMapping486	ENDP
ENDIF

;
;	レンダリング用ポリゴン情報をセットアップ 486 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@CretaePolygonEntry486	PROC	NEAR32 C USES ebx esi edi,
	hRenderPoly:HEGL_RENDER_POLYGON,
	hStackHeap:HSTACKHEAP, pPrimitive:PCE3D_PRIMITIVE_POLYGON

	LOCAL	pPolyEntry:PE3D_POLYGON_ENTRY
	LOCAL	r:REAL4
	LOCAL	dwShadingFlags:DWORD
	LOCAL	vi[3]:DWORD
	LOCAL	v1:E3D_VECTOR, v2:E3D_VECTOR
	LOCAL	vMeshMax:E3D_VECTOR, vMeshMin:E3D_VECTOR
	LOCAL	vVertexes[3]:E3D_VECTOR4
	LOCAL	vUVMaps[3]:E3D_VECTOR_2D

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
	mov	ecx, eax
	and	eax, [ebx].dwMaskShadingFlags
	or	eax, [ebx].dwAddShadingFlags
	test	ecx, E3DSAF_SHADING_MASK
	.IF	ZERO?
		and	eax, NOT E3DSAF_SHADING_MASK
	.ENDIF
	ASSUME	eax:NOTHING
	mov	[edi].dwTransparency, 0
	mov	[edi].dwShadingFlags, eax
	mov	[edi].surface.mesh.pMeshReserved, 0
	mov	dwShadingFlags, eax
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
	ASSUME	edx:PE3D_VECTOR4
	mov	esi, [esi].mesh.vertices
	ASSUME	esi:PE3D_VECTOR4
	mov	ecx, [edi].dwVertexCount
	ASSUME	ebx:NOTHING
	;
	FOR	@MEMBER, <x, y, z>
		mov	eax, [esi].@MEMBER
		mov	ebx, eax
		sar	eax, 31
		shr	eax, 1
		xor	eax, ebx
		mov	vMeshMin.@MEMBER, eax
		mov	vMeshMax.@MEMBER, eax
	ENDM
	fldz
	fldz
	fldz
	test	ecx, ecx
	.WHILE	!ZERO?
		fld	[esi].x
		fst	[edx].x
		faddp	st(1), st
		fld	[esi].y
		fst	[edx].y
		faddp	st(2), st
		fld	[esi].z
		fst	[edx].z
		faddp	st(3), st
		;
		FOR	@MEMBER, <x, y, z>
			mov	eax, [esi].@MEMBER
			mov	ebx, eax
			sar	eax, 31
			shr	eax, 1
			xor	eax, ebx
			.IF	(SDWORD PTR vMeshMin.@MEMBER) > eax
				mov	vMeshMin.@MEMBER, eax
			.ENDIF
			.IF	(SDWORD PTR vMeshMax.@MEMBER) < eax
				mov	vMeshMax.@MEMBER, eax
			.ENDIF
		ENDM
		add	esi, 10H
		add	edx, 10H
		dec	ecx
	.ENDW
	ASSUME	esi:NOTHING
	;
	;	重心点を計算する
	; --------------------------------------------------------------------
	fld1
	fidiv	[edi].dwVertexCount
	fmul	st(1), st
	fmul	st(2), st
	fmulp	st(3), st
	fstp	[edi].vCenter.x
	fstp	[edi].vCenter.y
	fstp	[edi].vCenter.z
	;
	FOR	@MEMBER, <x, y, z>
		mov	eax, vMeshMin.@MEMBER
		mov	ecx, vMeshMax.@MEMBER
		mov	ebx, eax
		mov	edx, ecx
		sar	eax, 31
		sar	ecx, 31
		shr	eax, 1
		shr	ecx, 1
		xor	eax, ebx
		xor	ecx, edx
		mov	vMeshMin.@MEMBER, eax
		mov	vMeshMax.@MEMBER, ecx
		mov	[edi].surface.mesh.vMinMesh.@MEMBER, eax
		mov	[edi].surface.mesh.vMaxMesh.@MEMBER, ecx
	ENDM
	;
	fld	[edi].surface.mesh.vMaxMesh.x
	fsub	[edi].surface.mesh.vMinMesh.x
	fmul	st(0), st
	fld	[edi].surface.mesh.vMaxMesh.y
	fsub	[edi].surface.mesh.vMinMesh.y
	fmul	st(0), st
	fld	[edi].surface.mesh.vMaxMesh.z
	fsub	[edi].surface.mesh.vMinMesh.z
	fmul	st(0), st
	faddp	st(1), st
	faddp	st(1), st
	fsqrt
	fstp	[edi].surface.mesh.rMeshRadius
	;
	;	画面外判定
	; --------------------------------------------------------------------
	;
	; ｚ判定
	;
	mov	ebx, hRenderPoly
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	mov	eax, vMeshMin.z
	mov	edx, vMeshMax.z
	cmp	eax, [ebx].rZMaxClip
	jge	Label_ErrorExit
	cmp	edx, [ebx].rZMinClip
	jle	Label_ErrorExit
	;
	; x, y 判定
	;
	fld	[ebx].vScreenPos.z
	fdiv	vMeshMax.z
	fld	vMeshMax.x
	fmul	st, st(1)
	fadd	[ebx].vScreenPos.x
	ficomp	[ebx].dib.rectClip.left
	fstsw	ax
	test	ax, 0100H		; max x < left ?
	jnz	Label_ErrorExit_fpop1
	fld	vMeshMax.y
	fmul	st, st(1)
	fadd	[ebx].vScreenPos.y
	ficomp	[ebx].dib.rectClip.top
	fstsw	ax
	test	ax, 0100H		; max y < top ?
	jnz	Label_ErrorExit_fpop1
	;
	fld	vMeshMin.x
	fmul	st, st(1)
	fadd	[ebx].vScreenPos.x
	ficomp	[ebx].dib.rectClip.right
	fstsw	ax
	test	ax, 0100H		; min x > right ?
	jz	Label_ErrorExit_fpop1
	fmul	vMeshMin.y
	fadd	[ebx].vScreenPos.y
	ficomp	[ebx].dib.rectClip.bottom
	fstsw	ax
	test	ax, 0100H		; min y > bottom ?
	jz	Label_ErrorExit
	;
	;	法線複製
	; --------------------------------------------------------------------
	mov	esi, pPrimitive
	ASSUME	esi:PCE3D_PRIMITIVE_POLYGON
	.IF	[edi].dwTypeFlag & E3D_SMOOTH_POLYGON
		mov	eax, [edi].dwVertexCount
		shl	eax, 4				; * (SIZEOF E3D_VECTOR4)
		INVOKE	eslStackHeapAllocate , hStackHeap, eax
		pushfd
		cld
		mov	[edi].pNormals, eax
		mov	esi, [esi].mesh.normals
		ASSUME	esi:PE3D_VECTOR4
		mov	ecx, [edi].dwVertexCount
		mov	edi, eax
		ASSUME	edi:PE3D_VECTOR4
		shl	ecx, 2
		rep	movsd
		popfd
		mov	esi, pPrimitive
		mov	edi, pPolyEntry
		ASSUME	esi:PCE3D_PRIMITIVE_POLYGON
		ASSUME	edi:PTR E3D_POLYGON_ENTRY
	.ELSE
		mov	[edi].pNormals, 0
	.ENDIF
	;
	;	UV座標複製
	; --------------------------------------------------------------------
	lea	esi, [esi].mesh.uv_map[0]
	ASSUME	esi:PTR E3D_VECTOR_2D
	.IF	([edi].dwTypeFlag & E3D_TEXTURE_POLYGON) \
			&& (dwShadingFlags & E3DSAF_TEXTURE_MAPPING)
		mov	eax, [edi].dwVertexCount
		shl	eax, 3				; * (SIZEOF E3D_VECTOR_2D)
		INVOKE	eslStackHeapAllocate , hStackHeap, eax
		pushfd
		cld
		mov	[edi].surface.mesh.pUVMap, eax
		mov	ecx, [edi].dwVertexCount
		mov	edi, eax
		shl	ecx, 1
		rep	movsd
		popfd
		mov	edi, pPolyEntry
		ASSUME	edi:PTR E3D_POLYGON_ENTRY
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
	INVOKE	eslStackHeapAllocate , hStackHeap, [esi].dwMeshBytes
	pushfd
	cld
	mov	[edi].surface.mesh.pMesh, eax
	mov	ecx, [esi].dwMeshBytes
	mov	edi, eax
	shr	ecx, 2
	rep	movsd
	popfd
	mov	edi, pPolyEntry
	ASSUME	edi:PTR E3D_POLYGON_ENTRY
	;
	;	頂点色設定
	; --------------------------------------------------------------------
	mov	ecx, [edi].dwVertexCount
	shl	ecx, 3			; *= (SIZEOF E3D_COLOR)
	INVOKE	eslStackHeapAllocate , hStackHeap, ecx
	mov	[edi].pVertexColors, eax
	mov	ecx, [edi].dwVertexCount
	;
	.IF	[edi].dwTypeFlag & E3D_VERTEX_COLOR_POLYGON
		mov	edi, eax
		ASSUME	edi:PTR E3D_COLOR
		mov	esi, pPrimitive
		ASSUME	esi:PCE3D_PRIMITIVE_POLYGON
		lea	esi, [esi].mesh.color[0]
		ASSUME	esi:PTR E3D_COLOR
		pushfd
		cld
		shl	ecx, 1
		rep	movsd
		popfd
		ASSUME	esi:NOTHING
	.ELSE
		ASSUME	edi:PTR E3D_POLYGON_ENTRY
		mov	esi, [edi].pAttr
		mov	edi, eax
		ASSUME	esi:PE3D_SURFACE_ATTRIBUTE
		ASSUME	edi:PTR E3D_COLOR
		mov	eax, DWORD PTR [esi].rgbaColor[0]
		mov	edx, DWORD PTR [esi].rgbaColor[4]
		test	ecx, ecx
		.WHILE	!ZERO?
			mov	DWORD PTR [edi], eax
			mov	DWORD PTR [edi + 4], edx
			add	edi, 8		; += (SIZEOF E3D_COLOR)
			dec	ecx
		.ENDW
	.ENDIF
	ASSUME	edi:NOTHING

	mov	eax, pPolyEntry
	ret


	.ELSEIF	eax == E3D_IMAGE_PRIMITIVE
	; --------------------------------------------------------------------
	;	画像プリミティブ
	; --------------------------------------------------------------------
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	ASSUME	esi:PTR E3D_PRIMITIVE_POLYGON
	ASSUME	edi:PTR E3D_POLYGON_ENTRY
	;
	INVOKE	eslStackHeapAllocate , hStackHeap, (SIZEOF E3D_POLYGON_ENTRY)
	mov	pPolyEntry, eax
	mov	edi, eax
	ASSUME	edi:PTR E3D_POLYGON_ENTRY
	;
	; フラグ設定
	;
	mov	[edi].dwTypeFlag, E3D_IMAGE_PRIMITIVE
	;
	; 中心座標設定
	;
	mov	ecx, [esi].image.vCenter
	ASSUME	ecx:PE3D_VECTOR4
	mov	eax, [ecx].x
	mov	edx, [ecx].y
	mov	ecx, [ecx].z
	ASSUME	ecx:NOTHING
	mov	[edi].vCenter.x, eax
	mov	[edi].vCenter.y, edx
	mov	[edi].vCenter.z, ecx
	;
	; 拡大率設定
	;
	mov	ecx, [esi].image.vEnlarge
	ASSUME	ecx:PE3D_VECTOR4
	fld	[ecx].y
	fld	[ecx].x
	fld	[ebx].vScreenPos.z
	fdiv	[edi].vCenter.z
	fmul	st(1), st
	fmulp	st(2), st
	mov	eax, [ecx].z
	fstp	[edi].surface.image.vEnlarge.x
	fstp	[edi].surface.image.vEnlarge.y
	ASSUME	ecx:NOTHING
	;
	mov	ecx, [ebx].rZMinClip
	mov	edx, [ebx].rZMaxClip
	xor	eax, 80000000H
	xor	ecx, 80000000H
	xor	edx, 80000000H
	.IF	((DWORD PTR eax) < (DWORD PTR ecx)) \
			|| ((DWORD PTR eax) > (DWORD PTR edx))
		xor	eax, eax
		ret
	.ENDIF
	;
	; 画像の中心座標を設定
	;
	mov	eax, [esi].image.vImageBase.x
	mov	edx, [esi].image.vImageBase.y
	mov	[edi].surface.image.vImageBase.x, eax
	mov	[edi].surface.image.vImageBase.y, edx
	;
	; 透明度、回転角度および画像を設定
	;
	mov	ecx, [esi].image.pImageInf
	mov	eax, [esi].image.dwTransparency
	mov	edx, [esi].image.rRevolveAngle
	.IF	ecx == NULL
		mov	ecx, [esi].pSurfaceAttr
		ASSUME	ecx:PE3D_SURFACE_ATTRIBUTE
		mov	ecx, [ecx].txmap.pTextureImage
		ASSUME	ecx:NOTHING
	.ENDIF
	mov	[edi].dwTransparency, eax
	mov	[edi].surface.image.rRevolveAngle, edx
	mov	[edi].surface.image.pInfo, ecx
	mov	eax, [esi].pSurfaceAttr
	mov	[edi].pAttr, eax
	ASSUME	edi:NOTHING
	;
	mov	eax, edi
	ret

	.ELSEIF	eax == E3D_INFINITE_PLANE
	; --------------------------------------------------------------------
	;	無限平面
	; --------------------------------------------------------------------
	INVOKE	eslStackHeapAllocate , hStackHeap, (SIZEOF E3D_POLYGON_ENTRY)
	mov	pPolyEntry, eax
	mov	edi, eax
	ASSUME	edi:PTR E3D_POLYGON_ENTRY
	;
	; フラグ設定
	;
	mov	[edi].dwTypeFlag, E3D_INFINITE_PLANE
	mov	[edi].dwTransparency, 0
	mov	[edi].dwShadingFlags, 0
	;
	; 中心座標設定
	;
	mov	ecx, [esi].infinite_plane.vertex
	ASSUME	ecx:PE3D_VECTOR4
	mov	eax, [ecx].x
	mov	edx, [ecx].y
	mov	ecx, [ecx].z
	mov	[edi].vCenter.x, eax
	mov	[edi].vCenter.y, edx
	mov	[edi].vCenter.z, ecx
	mov	[edi].surface.poly.txmap.vOriginPos.x, eax
	mov	[edi].surface.poly.txmap.vOriginPos.y, edx
	mov	[edi].surface.poly.txmap.vOriginPos.z, ecx
	mov	[edi].surface.poly.txmap.vOriginPos.d, 0
	ASSUME	ecx:NOTHING
	;
	; テクスチャｘ軸ベクトル設定
	;
	mov	ecx, [esi].infinite_plane.xAxis
	ASSUME	ecx:PE3D_VECTOR4
	mov	eax, [ecx].x
	mov	edx, [ecx].y
	mov	ecx, [ecx].z
	mov	[edi].surface.poly.txmap.vAxisX.x, eax
	mov	[edi].surface.poly.txmap.vAxisX.y, edx
	mov	[edi].surface.poly.txmap.vAxisX.z, ecx
	mov	[edi].surface.poly.txmap.vAxisX.d, 0
	ASSUME	ecx:NOTHING
	;
	; テクスチャｙ軸ベクトル設定
	;
	mov	ecx, [esi].infinite_plane.yAxis
	ASSUME	ecx:PE3D_VECTOR4
	mov	eax, [ecx].x
	mov	edx, [ecx].y
	mov	ecx, [ecx].z
	mov	[edi].surface.poly.txmap.vAxisY.x, eax
	mov	[edi].surface.poly.txmap.vAxisY.y, edx
	mov	[edi].surface.poly.txmap.vAxisY.z, ecx
	mov	[edi].surface.poly.txmap.vAxisY.d, 0
	ASSUME	ecx:NOTHING
	;
	; 擬似フォッグを設定
	;
	mov	eax, [esi].infinite_plane.rgbFogColor.dwPixelCode
	mov	edx, [esi].infinite_plane.rFogDeepness
	mov	[edi].surface.poly.txmap.rgbFogColor.dwPixelCode, eax
	mov	[edi].surface.poly.txmap.rFogDeepness, edx
	;
	; 属性と画像を設定する
	;
	mov	eax, [esi].pSurfaceAttr
	ASSUME	eax:PE3D_SURFACE_ATTRIBUTE
	mov	edx, [eax].txmap.pTextureImage
	mov	[edi].pAttr, eax
	mov	[edi].surface.poly.txmap.pTextureImage, edx
	mov	[edi].surface.poly.txmap.pLuminousImage, 0
	mov	[edi].surface.poly.txmap.nTextureApply, 0
	mov	[edi].surface.poly.txmap.nLuminousApply, 0
	ASSUME	eax:NOTHING
	;
	; 平面パラメータを計算する
	;
	fld	[edi].surface.poly.txmap.vAxisX.y
	fmul	[edi].surface.poly.txmap.vAxisY.z
	fld	[edi].surface.poly.txmap.vAxisY.y
	fmul	[edi].surface.poly.txmap.vAxisX.z
	fsubp	st(1), st
	fst	[edi].plane.x
	;
	fld	[edi].surface.poly.txmap.vAxisX.z
	fmul	[edi].surface.poly.txmap.vAxisY.x
	fld	[edi].surface.poly.txmap.vAxisY.z
	fmul	[edi].surface.poly.txmap.vAxisX.x
	fsubp	st(1), st
	fst	[edi].plane.y
	;
	fld	[edi].surface.poly.txmap.vAxisX.x
	fmul	[edi].surface.poly.txmap.vAxisY.y
	fld	[edi].surface.poly.txmap.vAxisY.x
	fmul	[edi].surface.poly.txmap.vAxisX.y
	fsubp	st(1), st
	fst	[edi].plane.z
	;
	fld	[edi].vCenter.x
	fld	[edi].vCenter.y
	fld	[edi].vCenter.z
	fmulp	st(3), st
	fmulp	st(3), st
	fmulp	st(3), st
	faddp	st(1), st
	faddp	st(1), st
	fchs
	;
	; 法線を正規化
	;
	fld	[edi].plane.x
	fld	[edi].plane.y
	fld	[edi].plane.z
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
	;
	fmul	st(1), st
	fmul	st(2), st
	fmul	st(3), st
	fmulp	st(4), st
	fstp	[edi].plane.z
	fstp	[edi].plane.y
	fstp	[edi].plane.x
	fstp	[edi].plane.d
	ASSUME	edi:NOTHING
	;
	mov	eax, edi
	ret

	.ELSE
	; --------------------------------------------------------------------
	;	通常ポリゴン
	; --------------------------------------------------------------------
	INVOKE	eslStackHeapAllocate , hStackHeap, (SIZEOF E3D_POLYGON_ENTRY)
	mov	pPolyEntry, eax
	mov	edi, eax
	ASSUME	edi:PTR E3D_POLYGON_ENTRY
	;
	; フラグ設定
	;
	mov	eax, [esi].dwTypeFlag
	mov	[edi].dwTypeFlag, eax
	;
	; 頂点数と表面属性を設定
	;
	mov	eax, [esi].pSurfaceAttr
	mov	edx, [esi].dwVertexCount
	mov	[edi].pAttr, eax
	mov	[edi].dwVertexCount, edx
	mov	[edi].dwProjectedCount, 0
	.IF	(SDWORD PTR edx) < 3
		xor	eax, eax
		ret
	.ENDIF
	ASSUME	eax:PTR E3D_SURFACE_ATTRIBUTE
	mov	eax, [eax].dwShadingFlags
	ASSUME	eax:NOTHING
	mov	ecx, eax
	and	eax, [ebx].dwMaskShadingFlags
	or	eax, [ebx].dwAddShadingFlags
	test	ecx, E3DSAF_SHADING_MASK
	.IF	ZERO?
		and	eax, NOT E3DSAF_SHADING_MASK
	.ENDIF
	mov	[edi].dwTransparency, 0
	mov	[edi].dwShadingFlags, eax
	mov	dwShadingFlags, eax
	;
	; 頂点複製
	;
	mov	eax, [edi].dwVertexCount
	shl	eax, 4 + 1
	INVOKE	eslStackHeapAllocate , hStackHeap, eax
	mov	ecx, [edi].dwVertexCount
	mov	[edi].pVertexes, eax
	shl	ecx, 4
	add	eax, ecx
	mov	[edi].pNormals, eax
	;
	mov	eax, [edi].dwVertexCount
	shl	eax, 3
	INVOKE	eslStackHeapAllocate , hStackHeap, eax
	mov	[edi].pVertexColors, eax
	;
	mov	edx, [edi].pVertexes
	lea	esi, [esi].polygon
	mov	ecx, [edi].dwVertexCount
	ASSUME	esi:PTR E3D_PRIMITIVE_VERTEX
	ASSUME	edx:PE3D_VECTOR4
	ASSUME	ebx:PE3D_VECTOR4
	.REPEAT
		mov	ebx, [esi].vertex
		add	esi, (SIZEOF E3D_PRIMITIVE_VERTEX)
		FOR	@MEMBER, <x, y, z>
			mov	eax, [ebx].@MEMBER
			mov	[edx].@MEMBER, eax
		ENDM
		mov	[edx].d, 0
		add	edx, (SIZEOF E3D_VECTOR4)
		dec	ecx
	.UNTIL	ZERO?
	mov	esi, pPrimitive
	mov	ebx, hRenderPoly
	ASSUME	esi:PCE3D_PRIMITIVE_POLYGON
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	;
	; 重心点を計算する
	;
	fldz
	fldz
	fldz
	mov	edx, [edi].pVertexes
	mov	ecx, [edi].dwVertexCount
	ASSUME	edx:PE3D_VECTOR4
	.REPEAT
		fld	[edx].z
		fld	[edx].y
		fld	[edx].x
		add	edx, (SIZEOF E3D_VECTOR4)
		faddp	st(3), st
		faddp	st(3), st
		faddp	st(3), st
		dec	ecx
	.UNTIL	ZERO?
	ASSUME	edx:NOTHING
	fld1
	fidiv	[edi].dwVertexCount
	fmul	st(1), st
	fmul	st(2), st
	fmulp	st(3), st
	fstp	[edi].vCenter.x
	fstp	[edi].vCenter.y
	fstp	[edi].vCenter.z
	;
	; 平面パラメータを計算する
	;
	mov	ecx, [edi].pVertexes
	ASSUME	ecx:PE3D_VECTOR4
	mov	edx, [edi].dwVertexCount	; v1 を取得
	lea	edx, [edx + edx * 4 + 8]
	shr	edx, 4
	shl	edx, 4
	mov	vi[0], 0
	mov	vi[4], edx
	FOR	@MEMBER, <x, y, z>
		fld	[ecx + edx].@MEMBER
		fsub	[ecx].@MEMBER
		fstp	v1.@MEMBER
	ENDM
	mov	edx, [edi].dwVertexCount	; v2 を取得
	lea	edx, [edx + edx * 4 + 4]
	shr	edx, 3
	shl	edx, 4
	mov	vi[8], edx
	FOR	@MEMBER, <x, y, z>
		fld	[ecx + edx].@MEMBER
		fsub	[ecx].@MEMBER
		fstp	v2.@MEMBER
	ENDM
	;
	fld	v1.y
	fmul	v2.z
	fld	v2.y
	fmul	v1.z
	fsubp	st(1), st
	fst	[edi].plane.x
	;
	fld	v1.z
	fmul	v2.x
	fld	v2.z
	fmul	v1.x
	fsubp	st(1), st
	fst	[edi].plane.y
	;
	fld	v1.x
	fmul	v2.y
	fld	v2.x
	fmul	v1.y
	fsubp	st(1), st
	fst	[edi].plane.z
	;
	fld	[ecx].x
	fld	[ecx].y
	fld	[ecx].z
	fmulp	st(3), st
	fmulp	st(3), st
	fmulp	st(3), st
	faddp	st(1), st
	faddp	st(1), st
	fchs
	ASSUME	ecx:PE3D_VECTOR4
	;
	; 法線を正規化
	;
	fld	[edi].plane.x
	fld	[edi].plane.y
	fld	[edi].plane.z
	fld	st(2)
	fmul	st(0), st
	fld	st(2)
	fmul	st(0), st
	fld	st(2)
	fmul	st(0), st
	faddp	st(1), st
	faddp	st(1), st
	fsqrt
	fst	r
	.IF	(DWORD PTR r) < 33800000H	; r < 2^-24
		fstp	st(0)
		fstp	st(0)
		fstp	st(0)
		fstp	st(0)
		fstp	st(0)
		xor	eax, eax
		ret
	.ENDIF
	fld1
	fdivrp	st(1), st
	;
	fmul	st(1), st
	fmul	st(2), st
	fmul	st(3), st
	fmulp	st(4), st
	fstp	[edi].plane.z
	fstp	[edi].plane.y
	fstp	[edi].plane.x
	fstp	[edi].plane.d
	;
	fld	[edi].vCenter.x
	fmul	st, st(0)
	fld	[edi].vCenter.y
	fmul	st, st(0)
	fld	[edi].vCenter.z
	fmul	st, st(0)
	faddp	st(2), st
	faddp	st(1), st
	fsqrt
	fld	[edi].plane.d
	fdivrp	st(1), st
	fabs
	fstp	r
	.IF	(DWORD PTR r) < 3CC00000H	; r < 1.5/64 ≒ sin(1.34[deg])
		xor	eax, eax
		ret
	.ENDIF
	;
	; 頂点法線を設定
	;
	mov	edx, [edi].pNormals
	lea	esi, [esi].polygon
	mov	ecx, [edi].dwVertexCount
	ASSUME	esi:PTR E3D_PRIMITIVE_VERTEX
	ASSUME	edx:PE3D_VECTOR4
	ASSUME	ebx:PE3D_VECTOR4
	.IF	[edi].dwTypeFlag & E3D_SMOOTH_POLYGON

		.IF	dwShadingFlags & E3DSAF_SINGLE_SIDE_PLANE
			mov	ebx, [esi].normal
			fld	[edi].plane.x
			fmul	[ebx].x
			fld	[edi].plane.y
			fmul	[ebx].y
			fld	[edi].plane.z
			fmul	[ebx].z
			fxch	st(2)
			faddp	st(1), st
			faddp	st(1), st
			fstp	r
			mov	eax, r
			xor	eax, [edi].plane.d
			.IF	SIGN?
				xor	eax, eax
				ret
			.ENDIF
		.ENDIF

		mov	eax, [edi].pVertexes
		.REPEAT
			mov	ebx, [esi].normal
			add	esi, (SIZEOF E3D_PRIMITIVE_VERTEX)
			;
			fld	[ebx].z
			fld	[ebx].y
			fld	[ebx].x
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
			fstp	[edx].x
			fstp	[edx].y
			fstp	[edx].z
			mov	[edx].d, 0
			;
			add	edx, (SIZEOF E3D_VECTOR4)
			dec	ecx
		.UNTIL	ZERO?
	.ELSE
		mov	eax, [edi].plane.x
		mov	ebx, [edi].plane.y
		mov	esi, [edi].plane.z
		.REPEAT
			mov	[edx].x, eax
			mov	[edx].y, ebx
			mov	[edx].z, esi
			mov	[edx].d, 0
			add	edx, (SIZEOF E3D_VECTOR4)
			dec	ecx
		.UNTIL	ZERO?
	.ENDIF
	mov	esi, pPrimitive
	mov	ebx, hRenderPoly
	ASSUME	edx:NOTHING
	ASSUME	esi:PCE3D_PRIMITIVE_POLYGON
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	;
	; 頂点色を設定
	;
	mov	edx, [edi].pVertexColors
	lea	esi, [esi].polygon
	mov	ecx, [edi].dwVertexCount
	ASSUME	esi:PTR E3D_PRIMITIVE_VERTEX
	ASSUME	edx:PE3D_COLOR
	.IF	[edi].dwTypeFlag & E3D_VERTEX_COLOR_POLYGON
		.REPEAT
			mov	eax, [esi].color.rgbMul.dwPixelCode
			mov	ebx, [esi].color.rgbAdd.dwPixelCode
			add	esi, (SIZEOF E3D_PRIMITIVE_VERTEX)
			mov	[edx].rgbMul.dwPixelCode, eax
			mov	[edx].rgbAdd.dwPixelCode, ebx
			add	edx, (SIZEOF E3D_COLOR)
			dec	ecx
		.UNTIL	ZERO?
	.ELSE
		mov	esi, [edi].pAttr
		ASSUME	esi:PE3D_SURFACE_ATTRIBUTE
		mov	eax, [esi].rgbaColor.rgbMul.dwPixelCode
		mov	ebx, [esi].rgbaColor.rgbAdd.dwPixelCode
		ASSUME	ebx:NOTHING
		.REPEAT
			mov	[edx].rgbMul.dwPixelCode, eax
			mov	[edx].rgbAdd.dwPixelCode, ebx
			add	edx, (SIZEOF E3D_COLOR)
			dec	ecx
		.UNTIL	ZERO?
	.ENDIF
	mov	ebx, hRenderPoly
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	;
	; テクスチャーマッピングのパラメータを設定
	;
IF	0
	.IF	dwShadingFlags & (E3DSAF_GENVIRONMENT_MAP OR E3DSAF_ENVIRONMENT_MAP)
		;
		;	環境マッピング
		; ------------------------------------------------------------
		;
		; 頂点座標を取得
		;
		FOR	@INDEX, <0, 1, 2>
			mov	eax, vi[@INDEX * 4]
			mov	ecx, [edi].pVertexes
			mov	esi, pPrimitive
			ASSUME	ecx:PE3D_VECTOR4
			ASSUME	esi:PCE3D_PRIMITIVE_POLYGON
			lea	ecx, [ecx + eax]
			lea	esi, [esi].polygon[eax]
			ASSUME	esi:PTR E3D_PRIMITIVE_VERTEX
			mov	eax, [ecx].x
			mov	edx, [esi].uv_map.x
			mov	vVertexes[@INDEX*10H].x, eax
			mov	vUVMaps[@INDEX*8].x, edx
			mov	eax, [ecx].y
			mov	edx, [esi].uv_map.y
			mov	vVertexes[@INDEX*10H].y, eax
			mov	eax, [ecx].z
			mov	vUVMaps[@INDEX*8].y, edx
			mov	vVertexes[@INDEX*10H].z, eax
			ASSUME	esi:NOTHING
			ASSUME	ecx:NOTHING
		ENDM
		;
		; マッピングパラメータ取得
		;
		.IF	dwShadingFlags & E3DSAF_GENVIRONMENT_MAP
			lea	eax, [ebx].genvmap
		.ELSE
			mov	eax, [edi].pAttr
			ASSUME	eax:PE3D_SURFACE_ATTRIBUTE
			lea	eax, [eax].envmap
			ASSUME	eax:NOTHING
		.ENDIF
		INVOKE	eglRenderPoly@SetEnvironmentMapping486 ,
				ADDR [edi].surface.poly.txmap,
				ADDR vVertexes[0], ADDR vUVMaps[0],
				ADDR [ebx].envmat, eax
		;
		; 適用度設定
		;
		mov	ecx, [edi].pAttr
		ASSUME	ecx:PE3D_SURFACE_ATTRIBUTE
		mov	eax, [ecx].nTextureApply
		mov	edx, [ecx].nLuminousApply
		mov	[edi].surface.poly.txmap.nTextureApply, eax
		mov	[edi].surface.poly.txmap.nLuminousApply, edx
		mov	[edi].surface.poly.txmap.rFogDeepness, 0
		mov	edx, [ecx].nDeepness
		mov	eax, [ecx].nTransparency
		test	edx, edx
		.IF	ZERO?
			mov	[edi].dwTransparency, eax
		.ENDIF
		ASSUME	ecx:NOTHING

	.ELSEIF	dwShadingFlags & E3DSAF_TEXTURE_MAPPING
ELSE
	.IF	dwShadingFlags & E3DSAF_TEXTURE_MAPPING
ENDIF
	.IF	[edi].dwTypeFlag & E3D_TEXTURE_POLYGON
		;
		;	UV マップ
		; ------------------------------------------------------------
		;
		; 頂点座標を取得
		;
		FOR	@INDEX, <0, 1, 2>
			mov	eax, vi[@INDEX * 4]
			mov	ecx, [edi].pVertexes
			mov	esi, pPrimitive
			ASSUME	ecx:PE3D_VECTOR4
			ASSUME	esi:PCE3D_PRIMITIVE_POLYGON
			lea	ecx, [ecx + eax]
			lea	esi, [esi].polygon[eax]
			ASSUME	esi:PTR E3D_PRIMITIVE_VERTEX
			mov	eax, [ecx].x
			mov	edx, [esi].uv_map.x
			mov	vVertexes[@INDEX*10H].x, eax
			mov	vUVMaps[@INDEX*8].x, edx
			mov	eax, [ecx].y
			mov	edx, [esi].uv_map.y
			mov	vVertexes[@INDEX*10H].y, eax
			mov	eax, [ecx].z
			mov	vUVMaps[@INDEX*8].y, edx
			mov	vVertexes[@INDEX*10H].z, eax
			ASSUME	esi:NOTHING
			ASSUME	ecx:NOTHING
		ENDM
		;
		; マッピングパラメータ取得
		;
		INVOKE	eglRenderPoly@SetTextureParameter486 ,
				ADDR [edi].surface.poly.txmap,
				ADDR vVertexes[0], ADDR vUVMaps[0]
		;
		; テクスチャ画像を設定
		;
		mov	ecx, [edi].pAttr
		ASSUME	ecx:PE3D_SURFACE_ATTRIBUTE
		mov	eax, [edi].vCenter.z
		mov	edx, [ecx].txmap.rThresholdZ
		xor	eax, 80000000H
		xor	edx, 80000000H
		.IF	eax > edx
			fld	[edi].surface.poly.txmap.vAxisY.z
			fld	[edi].surface.poly.txmap.vAxisY.y
			fld	[edi].surface.poly.txmap.vAxisY.x
			fld	[edi].surface.poly.txmap.vAxisX.z
			fld	[edi].surface.poly.txmap.vAxisX.y
			fld	[edi].surface.poly.txmap.vAxisX.x
			;
			fld	[ecx].txmap.nSmallScale
			fld1
			fscale
			fstp	st(1)
			;
			fmul	st(1), st
			fmul	st(2), st
			fmul	st(3), st
			fmul	st(4), st
			fmul	st(5), st
			fmulp	st(6), st
			;
			fstp	[edi].surface.poly.txmap.vAxisX.x
			fstp	[edi].surface.poly.txmap.vAxisX.y
			fstp	[edi].surface.poly.txmap.vAxisX.z
			fstp	[edi].surface.poly.txmap.vAxisY.x
			fstp	[edi].surface.poly.txmap.vAxisY.y
			fstp	[edi].surface.poly.txmap.vAxisY.z
			;
			mov	eax, [ecx].txmap.pSmallImage
			mov	edx, [ecx].txmap.pSmallLuminous
		.ELSE
			mov	eax, [ecx].txmap.pTextureImage
			mov	edx, [ecx].txmap.pLuminousImage
		.ENDIF
		mov	[edi].surface.poly.txmap.pTextureImage, eax
		mov	[edi].surface.poly.txmap.pLuminousImage, edx
		mov	eax, [ecx].nTextureApply
		mov	edx, [ecx].nLuminousApply
		mov	[edi].surface.poly.txmap.nTextureApply, eax
		mov	[edi].surface.poly.txmap.nLuminousApply, edx
		mov	[edi].surface.poly.txmap.rFogDeepness, 0
		mov	edx, [ecx].nDeepness
		mov	eax, [ecx].nTransparency
		test	edx, edx
		.IF	ZERO?
			mov	[edi].dwTransparency, eax
		.ENDIF
		ASSUME	ecx:NOTHING
	.ELSE
		;
		;	テクスチャ無し
		; ------------------------------------------------------------
		and	[edi].dwTypeFlag, NOT E3D_TEXTURE_POLYGON
		and	[edi].dwShadingFlags, NOT E3DSAF_TEXTURE_MAPPING
	.ENDIF

	.ELSE
		;
		;	テクスチャ無し
		; ------------------------------------------------------------
		and	[edi].dwTypeFlag, NOT E3D_TEXTURE_POLYGON
		and	[edi].dwShadingFlags, NOT E3DSAF_TEXTURE_MAPPING
	.ENDIF
	.ENDIF
	mov	eax, edi

	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	ret


Label_ErrorExit_fpop1:
	fstp	st(0)
Label_ErrorExit:
	INVOKE	eslStackHeapLeave , hStackHeap, pPolyEntry
	xor	eax, eax
	ret

eglRenderPoly@CretaePolygonEntry486	ENDP



	.686
	.XMM

;
;	テクスチャマッピングパラメータ計算 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@SetTextureParameterSSE	PROC	NEAR32 C USES ebx esi edi,
	ptxmap:PTR E3D_TEXTURE_MAPINFO,
	vertex:PTR E3D_VECTOR4, uvmap:PTR E3D_VECTOR_2D

	LOCAL	uv1:E3D_VECTOR_2D, v1:E3D_VECTOR4
	LOCAL	uv2:E3D_VECTOR_2D, v2:E3D_VECTOR4

	mov	ebx, ptxmap
	mov	esi, vertex
	mov	edi, uvmap
	ASSUME	ebx:PTR E3D_TEXTURE_MAPINFO
	ASSUME	esi:PTR E3D_VECTOR4
	ASSUME	edi:PTR E3D_VECTOR_2D
	;
	; テクスチャマッピング軸を計算
	;
	movlps	xmm6, [edi]			; v1, v2 を取得
	movlps	xmm0, [edi][8]
	movlps	xmm1, [edi][16]
	movups	xmm7, [esi]
	movups	xmm2, [esi][10H]
	movups	xmm3, [esi][20H]
	subps	xmm0, xmm6
	subps	xmm1, xmm6
	subps	xmm2, xmm7
		movaps	xmm6, xmmMaskSign
	subps	xmm3, xmm7
	;
		movaps	xmm7, rConstLittle
	movaps	xmm4, xmm0
	movaps	xmm5, xmm1
	andps	xmm4, xmm6
	andps	xmm5, xmm6
	cmpps	xmm4, xmm7, 1		; xmm4 <- (xmm4 < xmm7)
	cmpps	xmm5, xmm7, 1		; xmm5 <- (xmm5 < xmm7)
	movmskps	eax, xmm4
	movmskps	edx, xmm5
	movaps	xmm7, xmmRevHalfPI
	and	eax, 03H
	and	edx, 03H
	.IF	eax == 03H
		movaps	xmm0, xmm1
		shufps	xmm0, xmm0, 11100001B
		mulps	xmm0, xmm7
		;
		movaps	xmm4, xmm0
		mulps	xmm0, xmm0
		movss	xmm5, xmm0
		shufps	xmm0, xmm0, 1
		addss	xmm0, xmm5
		sqrtss	xmm0, xmm0
		shufps	xmm0, xmm0, 0
		divps	xmm4, xmm0
		movaps	xmm0, xmm4

	.ELSEIF	edx == 03H
		movaps	xmm1, xmm0
		shufps	xmm1, xmm1, 11100001B
		mulps	xmm1, xmm7
		;
		movaps	xmm4, xmm1
		mulps	xmm1, xmm1
		movss	xmm5, xmm1
		shufps	xmm1, xmm1, 1
		addss	xmm1, xmm5
		sqrtss	xmm1, xmm1
		shufps	xmm1, xmm1, 0
		divps	xmm4, xmm1
		movaps	xmm1, xmm4
	.ENDIF
	;
	; テクスチャマッピングｘ、ｙ軸を計算
	;
	movaps	xmm4, xmm1
	shufps	xmm4, xmm4, 0001B
	mulps	xmm4, xmm0
	movaps	xmm5, xmm4
	shufps	xmm5, xmm5, 1
	subss	xmm4, xmm5
	movss	xmm5, xmmConst1
	divss	xmm5, xmm4
	;
	movaps	xmm6, xmm1
	movaps	xmm7, xmm0
		shufps	xmm0, xmm0, 0
		shufps	xmm1, xmm1, 0
	shufps	xmm6, xmm6, 01010101B
	shufps	xmm7, xmm7, 01010101B
		mulps	xmm0, xmm3
		mulps	xmm1, xmm2
	mulps	xmm6, xmm2
	mulps	xmm7, xmm3
		shufps	xmm5, xmm5, 0
		subps	xmm0, xmm1
	subps	xmm6, xmm7
		mulps	xmm0, xmm5
	mulps	xmm6, xmm5
	;
		movups	[ebx].vAxisY, xmm0
	movups	[ebx].vAxisX, xmm6
	;
	; テクスチャマッピング相対中心座標を計算
	;
	movss	xmm2, [edi].x
	movss	xmm3, [edi].y
	shufps	xmm2, xmm2, 0
	shufps	xmm3, xmm3, 0
	mulps	xmm2, xmm6
	mulps	xmm3, xmm0
	movups	xmm4, [esi]
	addps	xmm2, xmm3
	subps	xmm4, xmm2
	movups	[ebx].vOriginPos, xmm4
	;
	ASSUME	ebx:NOTHING
	ASSUME	esi:NOTHING
	ASSUME	edi:NOTHING
	ret

eglRenderPoly@SetTextureParameterSSE	ENDP

IF	0
;
;	環境マッピングパラメータ計算 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@SetEnvironmentMappingSSE	PROC	NEAR32 C PRIVATE USES ebx esi edi,
	ptxmap:PTR E3D_TEXTURE_MAPINFO,
	vertex:PTR E3D_VECTOR, normal:PTR E3D_VECTOR4,
	envmat:PTR E3D_REV_MATRIX, envmap:PTR E3D_ENVIRONMENT_MAPPING

	LOCAL	vNormals[3]:E3D_VECTOR4
	LOCAL	vUVMap[4]:E3D_VECTOR_2D

	;
	;	法線を回転
	; --------------------------------------------------------------------
	mov	esi, normal
	lea	edi, vNormals[0]
	mov	ebx, envmat
	ASSUME	esi:PTR E3D_VECTOR4
	ASSUME	edi:PTR E3D_VECTOR4
	ASSUME	ebx:PTR E3D_REV_MATRIX
	mov	ecx, 3
	movups	xmm0, [ebx].matrix[0]
	movups	xmm1, [ebx].matrix[10H]
	movups	xmm2, [ebx].matrix[20H]
	;
	.REPEAT
		movups	xmm4, [esi]
		add	esi, (SIZEOF E3D_VECTOR4)
		;
		movaps	xmm5, xmm4
		movaps	xmm6, xmm4
		mulps	xmm4, xmm0
		mulps	xmm5, xmm1
		mulps	xmm6, xmm2
		;
		movaps	xmm3, xmm4
		shufps	xmm3, xmm3, 1
		movhlps	xmm7, xmm4
		addss	xmm4, xmm3
			movaps	xmm3, xmm5
			shufps	xmm3, xmm3, 1
		addss	xmm4, xmm7
			movhlps	xmm7, xmm5
			addss	xmm5, xmm3
				movaps	xmm3, xmm6
				shufps	xmm3, xmm3, 1
		movss	[edi].x, xmm4
			addss	xmm5, xmm7
				movhlps	xmm7, xmm7
				addss	xmm6, xmm3
			movss	[edi].y, xmm5
				addss	xmm6, xmm7
				movss	[edi].z, xmm6
		mov	[edi].d, 0
		add	edi, (SIZEOF E3D_VECTOR4)
		;
		dec	ecx
	.UNTIL	ZERO?
	ASSUME	ebx:NOTHING
	;
	;	法線への視線の反射を計算
	; --------------------------------------------------------------------
	mov	ecx, 3
	lea	edi, vNormals[0]
	mov	esi, vertex
	.REPEAT
		;
		; 視線ベクトルと法線の内積を計算
		;
		movups	xmm0, [esi]
		movups	xmm5, [edi]
		movaps	xmm4, xmm0
		mulps	xmm0, xmm5
		movaps	xmm1, xmm0
		movhlps	xmm2, xmm0
		shufps	xmm1, xmm1, 1
		addss	xmm0, xmm2
		addss	xmm0, xmm1
		;
		; 反射ベクトルを計算
		;
		addss	xmm0, xmm0
		shufps	xmm0, xmm0, 11000000B
		mulps	xmm0, xmm5
		subps	xmm4, xmm0
		;
		; 反射ベクトルを正規化
		;
		movaps	xmm0, xmm4
		mulps	xmm4, xmm4
		movaps	xmm1, xmm4
		movhlps	xmm2, xmm4
		shufps	xmm1, xmm1, 1
		addss	xmm4, xmm2
		addss	xmm4, xmm1
		rsqrtss	xmm4, xmm4
		shufps	xmm4, xmm4, 11000000B
		mulps	xmm0, xmm4
		;
		; 反射ベクトルをストア
		;
		movups	[edi], xmm0
		add	esi, (SIZEOF E3D_VECTOR4)
		add	edi, (SIZEOF E3D_VECTOR4)
		;
		dec	ecx
	.UNTIL	ZERO?
	ASSUME	esi:NOTHING
	ASSUME	edi:NOTHING
	;
	;	マップ座標を計算
	; --------------------------------------------------------------------
	;
	; ｘ－ｚ平面上のベクトルの長さを計算
	;
	mov	ecx, 3
	lea	edi, vNormals[0]
	ASSUME	edi:PTR E3D_VECTOR4
	.REPEAT
		movss	xmm0, [edi].x
		movss	xmm1, [edi].z
		mulss	xmm0, xmm0
		mulss	xmm1, xmm1
		addss	xmm0, xmm1
		sqrtss	xmm0, xmm0
		movss	[edi].d, xmm0
		add	edi, (SIZEOF E3D_VECTOR4)
		dec	ecx
	.UNTIL	ZERO?
	ASSUME	edi:NOTHING
	;
	; マッピングモードを特定
	;
	mov	eax, vNormals[0].y
	mov	ecx, vNormals[10H].y
	mov	edx, vNormals[20H].y
	xor	ecx, eax
	xor	edx, eax
	mov	ebx, ptxmap
	mov	esi, envmap
	or	ecx, edx	; ecx = 法線ｙ座標の符号が異なっている場合
	.IF	!SIGN?		; ecx <- 法線の傾きが45度以下である場合
		xor	edi, edi
		FOR	@INDEX, <0, 10H, 20H>
			mov	edx, vNormals[@INDEX].y
			and	edx, 7FFFFFFFH
			sub	edx, vNormals[@INDEX].d
			and	edi, edx
		ENDM
		or	ecx, edi
	.ENDIF
	ASSUME	ebx:PTR E3D_TEXTURE_MAPINFO
	ASSUME	esi:PTR E3D_ENVIRONMENT_MAPPING
	.IF	SIGN?
		;
		; ラウンドマッピングを使用
		; ------------------------------------------------------------
		;
		; xmm0 = z, xmm4 = y
		;
		movss	xmm0, vNormals[0].z
			movss	xmm4, vNormals[0].y
		movss	xmm1, vNormals[10H].z
			movss	xmm5, vNormals[10H].y
		movss	xmm2, vNormals[20H].z
			movss	xmm6, vNormals[20H].y
		unpcklps	xmm0, xmm1
			unpcklps	xmm4, xmm5
		movlhps	xmm0, xmm2
			movlhps	xmm4, xmm6
		;
		; xmm1 = x, xmm5 = d = sqrt(x^2 + z^2)
		;
		movss	xmm1, vNormals[0].x
			movss	xmm5, vNormals[0].d
		movss	xmm2, vNormals[10H].x
			movss	xmm6, vNormals[10H].d
		movss	xmm3, vNormals[20H].x
			movss	xmm7, vNormals[20H].d
		unpcklps	xmm1, xmm2
			unpcklps	xmm5, xmm6
		movlhps	xmm1, xmm3
			movlhps	xmm5, xmm7
		;
		; xmm0 = z / x, xmm4 = y / sqrt(x^2 + z^2)
		;
		rcpps	xmm2, xmm1
		movaps	xmm7, xmm1	; xmm7 = x
		rcpps	xmm6, xmm5
		mulps	xmm1, xmm2
		mulps	xmm5, xmm6
		rcpps	xmm1, xmm1
		rcpps	xmm5, xmm5
		mulps	xmm1, xmm2
		mulps	xmm5, xmm6
		mulps	xmm0, xmm1
		mulps	xmm4, xmm5
		;
		; 絶対値が１より大きい場合には、逆数で計算
		;
		movaps	xmm1, xmmMaskSign
		movaps	xmm3, xmmConst1
		movaps	xmm5, xmm1
		andps	xmm1, xmm0
		rcpps	xmm2, xmm1
		andps	xmm5, xmm4
		rcpps	xmm6, xmm4
		cmpps	xmm1, xmm3, 1		; 絶対値と 1.0 を比較
		cmpps	xmm5, xmm3, 1
		movaps	xmm3, xmmMaskFull
		andps	xmm0, xmm1		; 1.0 未満ならそのまま
		xorps	xmm1, xmm3	; xmm1 = abs(z) < abs(x)
		andps	xmm4, xmm5
		xorps	xmm5, xmm3	; xmm5 = abs(y) < abs(d)
		andps	xmm2, xmm1		; 1.0 以上なら逆数を選択
		andps	xmm6, xmm5
		orps	xmm0, xmm2
		orps	xmm4, xmm6
		;
		; Gregory 級数展開による逆正接の計算：ｘ座標
		;	atan(x) = x - x^3/3 + x^5/5 + x^7/7 ...
		;
		movaps	xmm2, xmm0	; xmm0 = x
		movaps	xmm3, xmm0
		movaps	xmm6, xmmRcp3
		mulps	xmm2, xmm2	; xmm2 = x ^ 2
		mulps	xmm3, xmm2	; xmm3 = x ^ 3
		mulps	xmm6, xmm3	; xmm6 = x ^ 3 / 3
		mulps	xmm3, xmm2	; xmm3 = x ^ 5
		subps	xmm0, xmm6
		movaps	xmm6, xmmRcp5
		mulps	xmm6, xmm3	; xmm6 = x ^ 5 / 5
		mulps	xmm3, xmm2	; xmm3 = x ^ 7
		addps	xmm0, xmm6
		mulps	xmm3, xmmRcp7
		subps	xmm0, xmm3	; xmm0 = x - x^3/3 + x^5/5 + x^7/7
		;
		; Gregory 級数展開による逆正接の計算：ｙ座標
		;	atan(y) = y - y^3/3 + y^5/5 + y^7/7 ...
		;
		movaps	xmm2, xmm4	; xmm4 = y
		movaps	xmm3, xmm4
		movaps	xmm6, xmmRcp3
		mulps	xmm2, xmm2	; xmm2 = y ^ 2
		mulps	xmm3, xmm2	; xmm3 = y ^ 3
		mulps	xmm6, xmm3	; xmm6 = y ^ 3 / 3
		mulps	xmm3, xmm2	; xmm3 = y ^ 5
		subps	xmm4, xmm6
		movaps	xmm6, xmmRcp5
		mulps	xmm6, xmm3	; xmm6 = y ^ 5 / 5
		mulps	xmm3, xmm2	; xmm3 = y ^ 7
		addps	xmm4, xmm6
		mulps	xmm3, xmmRcp7
		subps	xmm4, xmm3	; xmm4 = y - y^3/3 + y^5/5 + y^7/7
		;
		; 逆数の逆正接に対する補正
		;
		movaps	xmm3, xmmMaskSignBit
		movaps	xmm2, xmm0
		movaps	xmm6, xmm4
		andps	xmm2, xmm3
		andps	xmm6, xmm3
		movaps	xmm3, rConstHalfPI
		orps	xmm2, xmm3
		orps	xmm6, xmm3
		subps	xmm2, xmm0
		subps	xmm6, xmm4
		andps	xmm2, xmm1
		andnps	xmm1, xmm0
		andps	xmm6, xmm5
		andnps	xmm5, xmm4
		orps	xmm1, xmm2	; xmm1 <- atan(x')
		orps	xmm5, xmm6	; xmm5 <- atan(y')
		;
		; ｘ座標の符号に対する逆正接の補正
		;
		xorps	xmm6, xmm6
		movaps	xmm3, xmmMaskSignBit
		cmpps	xmm7, xmm6, 5
		movaps	xmm2, rConstPI
		andps	xmm3, xmm1
		movaps	xmm0, xmm1
		orps	xmm3, xmm2
		subps	xmm1, xmm3
		andps	xmm0, xmm7
		andnps	xmm7, xmm1
		;
		; 座標に変換
		;
		movaps	xmm2, rConstPI
		movaps	xmm1, rConstR2PI
		orps	xmm0, xmm7
		;
		addps	xmm5, xmm2
		mulps	xmm0, xmm1
		mulps	xmm5, xmm1
		;
		movaps	xmm1, xmm0
		unpcklps	xmm0, xmm5
		unpckhps	xmm1, xmm5
		movups	vUVMap[0], xmm0
		movups	vUVMap[16], xmm1
		;
		mov	eax, [esi].pRoundImage
		mov	edx, [esi].pRoundLuminous
		mov	[ebx].pTextureImage, eax
		mov	[ebx].pLuminousImage, edx

	.ELSEIF	!(eax & 80000000H)
		;
		; 上半球を使用
		; ------------------------------------------------------------
		FOR	@INDEX, <0, 1, 2>
			mov	eax, vNormals[@INDEX * 10H].x
			mov	edx, vNormals[@INDEX * 10H].z
			mov	vUVMap[@INDEX * 8].x, eax
			mov	vUVMap[@INDEX * 8].y, edx
		ENDM
		mov	eax, [esi].pUpperImage
		mov	edx, [esi].pUpperLuminous
		mov	[ebx].pTextureImage, eax
		mov	[ebx].pLuminousImage, edx

	.ELSE
		;
		; 下半球を使用
		; ------------------------------------------------------------
		FOR	@INDEX, <0, 1, 2>
			mov	eax, vNormals[@INDEX * 10H].x
			mov	edx, vNormals[@INDEX * 10H].z
			mov	vUVMap[@INDEX * 8].x, eax
			mov	vUVMap[@INDEX * 8].y, edx
		ENDM
		mov	eax, [esi].pUnderImage
		mov	edx, [esi].pUnderLuminous
		mov	[ebx].pTextureImage, eax
		mov	[ebx].pLuminousImage, edx
	.ENDIF
	ASSUME	esi:NOTHING
	;
	;	テクスチャ画像情報を適用
	; --------------------------------------------------------------------
	mov	eax, [ebx].pTextureImage
	ASSUME	ebx:NOTHING
	ASSUME	eax:PTR EGL_IMAGE_INFO
	cvtpi2ps	xmm7, MMWORD PTR [eax].dwImageWidth
	ASSUME	eax:NOTHING
	xorps	xmm1, xmm1
	movups	xmm0, vUVMap[0]
	movlhps	xmm7, xmm7
	movlps	xmm1, vUVMap[16]
	mulps	xmm0, xmm7
	mulps	xmm1, xmm7
	movups	vUVMap[0], xmm0
	movlps	vUVMap[16], xmm1
	;
	INVOKE	eglRenderPoly@SetTextureParameterSSE,
			ptxmap, vertex, ADDR vUVMap[0]
	;
	ret

eglRenderPoly@SetEnvironmentMappingSSE	ENDP
ENDIF

;
;	平面パラメータ計算 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@CalcPlaneParameterSSE	PROC	NEAR32 C

	subps	xmm1, xmm0
	subps	xmm2, xmm0
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
	mulps	xmm0, xmm1
		movaps	xmm6, xmm1
		mulps	xmm1, xmm1
	movaps	xmm4, xmm0
		movaps	xmm2, xmm1
	shufps	xmm4, xmm4, 1
		shufps	xmm2, xmm2, 1
		movhlps	xmm3, xmm1
	movhlps	xmm5, xmm0
	addss	xmm0, xmm4
		addss	xmm2, xmm1
	addss	xmm0, xmm5
		addss	xmm2, xmm3
	xorps	xmm0, xmmMaskLS1		; xmmMaskLS1 = 80000000H
		comiss	xmm2, rConstLittle
		sqrtss	xmm2, xmm2
		jc	Label_Exit
	shufps	xmm0, xmm6, 11100100B
	shufps	xmm2, xmm2, 0
	shufps	xmm6, xmm0, 00100100B
	divps	xmm6, xmm2
	clc
Label_Exit:
	ret

eglRenderPoly@CalcPlaneParameterSSE	ENDP

;
;	レンダリング用ポリゴン情報をセットアップ SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@CretaePolygonEntrySSE	PROC	NEAR32 C USES ebx esi edi,
	hRenderPoly:HEGL_RENDER_POLYGON,
	hStackHeap:HSTACKHEAP, pPrimitive:PCE3D_PRIMITIVE_POLYGON

	LOCAL	dwShadingFlags:DWORD
	LOCAL	vi[3]:DWORD
	LOCAL	r:REAL4
	LOCAL	pPolyEntry:PE3D_POLYGON_ENTRY
	LOCAL	vVertexes[3]:E3D_VECTOR4
	LOCAL	vUVMaps[3]:E3D_VECTOR_2D

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
	mov	dwShadingFlags, eax
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
	movaps	XMMWORD_PTR [edx], xmm0
	add	esi, 20H
	add	edx, 10H
	sub	ecx, 2
	.WHILE	!ZERO?
		movaps	xmm0, xmm1
		movups	xmm1, XMMWORD_PTR [esi]
		add	esi, 10H
		minps	xmm5, xmm0
		maxps	xmm6, xmm0
		movaps	XMMWORD_PTR [edx], xmm0
		add	edx, 10H
		dec	ecx
	.ENDW
	minps	xmm5, xmm1
	maxps	xmm6, xmm1
	movaps	XMMWORD_PTR [edx], xmm1
	ASSUME	esi:NOTHING
	;
	;	画面外判定
	; --------------------------------------------------------------------
	;
	; ｚ判定
	;
	movhlps	xmm0, xmm5
	movhlps	xmm1, xmm6
	comiss	xmm0, [ebx].rZMaxClip
	movups	[edi].surface.mesh.vMinMesh, xmm5
	jae	Label_ErrorExit
	comiss	xmm1, [ebx].rZMinClip
	movups	[edi].surface.mesh.vMaxMesh, xmm6
	jbe	Label_ErrorExit
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
	jnz	Label_ErrorExit
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
		sub	ecx, 2
		.WHILE	!SIGN?
			movups	xmm0, [esi]
			movups	xmm1, [esi + 10H]
			add	esi, 20H
			movaps	XMMWORD_PTR [edx], xmm0
			movaps	XMMWORD_PTR [edx + 10H], xmm1
			add	edx, 20H
			sub	ecx, 2
		.ENDW
		add	ecx, 2
		.IF	!ZERO?
			movups	xmm0, XMMWORD_PTR [esi]
			movaps	XMMWORD_PTR [edx], xmm0
		.ENDIF
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
			&& (dwShadingFlags & E3DSAF_TEXTURE_MAPPING)
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
		movaps	XMMWORD_PTR [edx], xmm0
		movaps	XMMWORD_PTR [edx + 10H], xmm1
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
			movaps	XMMWORD_PTR [edx], xmm0
			movaps	XMMWORD_PTR [edx + 10H], xmm0
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

	mov	eax, edi
	ret


	.ELSEIF	eax == E3D_IMAGE_PRIMITIVE
	ASSUME	esi:PCE3D_PRIMITIVE_POLYGON
	; --------------------------------------------------------------------
	;	画像プリミティブ
	; --------------------------------------------------------------------
	INVOKE	eslStackHeapAllocate , hStackHeap, (SIZEOF E3D_POLYGON_ENTRY)
	mov	pPolyEntry, eax
	mov	edi, eax
	ASSUME	edi:PTR E3D_POLYGON_ENTRY
	;
	; フラグ設定
	;
	mov	[edi].dwTypeFlag, E3D_IMAGE_PRIMITIVE
	mov	[edi].dwShadingFlags, 0
	;
	; 中心座標設定
	;
	mov	ecx, [esi].image.vCenter
	mov	edx, [esi].image.vEnlarge
	movlps	xmm0, QWORD PTR [ecx]
	movlps	QWORD PTR [edi].vCenter, xmm0
	;
	; 拡大率設定
	;
	movlps	xmm1, QWORD PTR [edx]
	movlps	xmm2, QWORD PTR (E3D_VECTOR4 PTR [ecx]).z
	movss	xmm3, [ebx].vScreenPos.z
	divss	xmm3, xmm2
	movlps	QWORD PTR [edi].vCenter.z, xmm2
	shufps	xmm3, xmm3, 0
	mulps	xmm1, xmm3
	movss	xmm0, xmm2 ; (E3D_VECTOR4 PTR [ecx]).z
	movlps	[edi].surface.image.vEnlarge, xmm1
	;
	movss	xmm2, [ebx].rZMinClip
	movss	xmm3, [ebx].rZMaxClip
	comiss	xmm0, xmm2
	jc	Label_ErrorExit
	comiss	xmm0, xmm3
	jnc	Label_ErrorExit
	;
	; 画像基準座標を設定
	;
	movlps	xmm0, [esi].image.vImageBase
	movlps	[edi].surface.image.vImageBase, xmm0
	;
	; 透明度、回転角度および画像を設定
	;
	mov	ecx, [esi].image.pImageInf
	mov	eax, [esi].image.dwTransparency
	mov	edx, [esi].image.rRevolveAngle
	.IF	ecx == NULL
		mov	ecx, [esi].pSurfaceAttr
		ASSUME	ecx:PE3D_SURFACE_ATTRIBUTE
		mov	ecx, [ecx].txmap.pTextureImage
		ASSUME	ecx:NOTHING
	.ENDIF
	mov	[edi].dwTransparency, eax
	mov	[edi].surface.image.rRevolveAngle, edx
	mov	[edi].surface.image.pInfo, ecx
	mov	eax, [esi].pSurfaceAttr
	mov	[edi].pAttr, eax
	ASSUME	edi:NOTHING
	;
	mov	eax, edi
	ret

	.ELSEIF	eax == E3D_INFINITE_PLANE
	; --------------------------------------------------------------------
	;	無限平面
	; --------------------------------------------------------------------
	INVOKE	eslStackHeapAllocate , hStackHeap, (SIZEOF E3D_POLYGON_ENTRY)
	mov	pPolyEntry, eax
	mov	edi, eax
	ASSUME	edi:PTR E3D_POLYGON_ENTRY
	;
	; フラグ設定
	;
	mov	[edi].dwTypeFlag, E3D_INFINITE_PLANE
	mov	[edi].dwTransparency, 0
	mov	[edi].dwShadingFlags, 0
	mov	[edi].dwVertexCount, 0
	mov	[edi].pVertexes, 0
	mov	[edi].pNormals, 0
	;
	; 中心座標設定
	; テクスチャｘ、ｙ軸ベクトル設定
	;
	movaps	xmm7, xmmMaskLP3
	mov	eax, [esi].infinite_plane.vertex
	mov	ecx, [esi].infinite_plane.xAxis
	mov	edx, [esi].infinite_plane.yAxis
	movups	xmm0, XMMWORD_PTR [eax]
	movups	xmm1, XMMWORD_PTR [ecx]
	movups	xmm2, XMMWORD_PTR [edx]
	andps	xmm0, xmm7
	andps	xmm1, xmm7
	andps	xmm2, xmm7
	movups	[edi].vCenter, xmm0
	movups	[edi].surface.poly.txmap.vOriginPos, xmm0
	movups	[edi].surface.poly.txmap.vAxisX, xmm1
	movups	[edi].surface.poly.txmap.vAxisY, xmm2
	;
	; 擬似フォッグを設定
	;
	mov	eax, [esi].infinite_plane.rgbFogColor.dwPixelCode
	mov	edx, [esi].infinite_plane.rFogDeepness
	mov	[edi].surface.poly.txmap.rgbFogColor.dwPixelCode, eax
	mov	[edi].surface.poly.txmap.rFogDeepness, edx
	;
	; 属性と画像を設定する
	;
	mov	eax, [esi].pSurfaceAttr
	ASSUME	eax:PE3D_SURFACE_ATTRIBUTE
	mov	edx, [eax].txmap.pTextureImage
	mov	[edi].pAttr, eax
	mov	[edi].surface.poly.txmap.pTextureImage, edx
	mov	[edi].surface.poly.txmap.pLuminousImage, 0
	mov	[edi].surface.poly.txmap.nTextureApply, 0
	mov	[edi].surface.poly.txmap.nLuminousApply, 0
	ASSUME	eax:NOTHING
	;
	; 平面パラメータを計算する
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
	movups	xmm0, [edi].vCenter
		movaps	xmm6, xmm1
		mulps	xmm1, xmm1
	mulps	xmm0, xmm6
		movaps	xmm4, xmm1
	movaps	xmm2, xmm0
		shufps	xmm4, xmm4, 1
	shufps	xmm2, xmm2, 1
	movhlps	xmm3, xmm0
		movhlps	xmm5, xmm1
	addss	xmm0, xmm2
		addss	xmm4, xmm1
	addss	xmm0, xmm3
		addss	xmm4, xmm5
	xorps	xmm0, xmmMaskLS1
		rsqrtss	xmm4, xmm4
	shufps	xmm0, xmm6, 11100100B
		shufps	xmm4, xmm4, 0
	shufps	xmm6, xmm0, 00100100B
	mulps	xmm6, xmm4
	;
	movups	[edi].plane, xmm6
	mov	eax, edi
	ret

	.ELSE
	; --------------------------------------------------------------------
	;	通常ポリゴン
	; --------------------------------------------------------------------
	INVOKE	eslStackHeapAllocate , hStackHeap, (SIZEOF E3D_POLYGON_ENTRY)
	mov	pPolyEntry, eax
	mov	edi, eax
	ASSUME	edi:PTR E3D_POLYGON_ENTRY
	;
	; フラグ設定
	;
	mov	eax, [esi].dwTypeFlag
	mov	[edi].dwTypeFlag, eax
	;
	; 頂点数と表面属性を設定
	;
	mov	eax, [esi].pSurfaceAttr
	mov	edx, [esi].dwVertexCount
	mov	[edi].pAttr, eax
	mov	[edi].dwVertexCount, edx
	mov	[edi].dwProjectedCount, 0
	cmp	edx, 3
	jb	Label_ErrorExit
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
	mov	dwShadingFlags, eax
	;
	; 頂点複製
	;
	mov	eax, [edi].dwVertexCount
	shl	eax, 4 + 1
	INVOKE	eslStackHeapAllocate , hStackHeap, eax
	mov	ecx, [edi].dwVertexCount
	mov	[edi].pVertexes, eax
	shl	ecx, 4
	add	eax, ecx
	mov	[edi].pNormals, eax
	;
	mov	eax, [edi].dwVertexCount
	shl	eax, 3
	INVOKE	eslStackHeapAllocate , hStackHeap, eax
	mov	[edi].pVertexColors, eax
	;
	xorps	xmm0, xmm0
	mov	ecx, [edi].dwVertexCount
	mov	edx, [edi].pVertexes
	cvtsi2ss	xmm7, ecx
	lea	esi, [esi].polygon
	movaps	xmm6, xmmMaskLP3
	ASSUME	esi:PTR E3D_PRIMITIVE_VERTEX
	ASSUME	edx:PE3D_VECTOR4
	.REPEAT
		mov	eax, [esi].vertex
		add	esi, (SIZEOF E3D_PRIMITIVE_VERTEX)
		movups	xmm1, [eax]
		andps	xmm1, xmm6
		addps	xmm0, xmm1
		movups	[edx], xmm1
		add	edx, (SIZEOF E3D_VECTOR4)
		dec	ecx
	.UNTIL	ZERO?
	shufps	xmm7, xmm7, 0
	mov	esi, pPrimitive
	ASSUME	esi:PCE3D_PRIMITIVE_POLYGON
	divps	xmm0, xmm7
	movups	[edi].vCenter.x, xmm0
	;
	; 平面パラメータを計算する
	;
	mov	ecx, [edi].pVertexes
	ASSUME	ecx:PE3D_VECTOR4
	mov	edx, [edi].dwVertexCount	; v1, v2 を取得
	mov	eax, edx
	lea	edx, [edx + edx * 4 + 8]
	lea	eax, [eax + eax * 4 + 4]
	shr	edx, 4
	shr	eax, 3
	shl	edx, 4
	shl	eax, 4
	mov	vi[0], 0
	mov	vi[4], edx
	mov	vi[8], eax
	;
	movups	xmm0, [ecx]
	movups	xmm1, [ecx + edx]
	movups	xmm2, [ecx + eax]
	;
	INVOKE	eglRenderPoly@CalcPlaneParameterSSE
	;
	jc	Label_ErrorExit
	;
	movups	[edi].plane, xmm6
	;
	movups	xmm0, [ecx]			; 視線と面の成す角度を求める
	shufps	xmm6, xmm6, 3			; 角度が小さい場合には表示しない
	mulps	xmm0, xmm0
	movaps	xmm1, xmm0
	movhlps	xmm2, xmm0
	shufps	xmm1, xmm1, 1
	addss	xmm0, xmm2
	addss	xmm0, xmm1
	sqrtss	xmm0, xmm0
	movss	xmm1, xmmMaskSign
	divss	xmm6, xmm0
	andps	xmm6, xmm1
	comiss	xmm6, rConstSinLittle
	jc	Label_ErrorExit
	;
	; 頂点法線を設定
	;
	mov	edx, [edi].pNormals
	lea	esi, [esi].polygon
	mov	ecx, [edi].dwVertexCount
	ASSUME	esi:PTR E3D_PRIMITIVE_VERTEX
	.IF	[edi].dwTypeFlag & E3D_SMOOTH_POLYGON

;		.IF	dwShadingFlags & E3DSAF_SINGLE_SIDE_PLANE
			mov	eax, [esi].normal
			movups	xmm0, [edi].plane
			movups	xmm1, [eax]
			mulps	xmm0, xmm1
			movhlps	xmm2, xmm0
			movaps	xmm1, xmm0
			shufps	xmm0, xmm0, 1
			addss	xmm0, xmm1
			addss	xmm0, xmm2
			mov	eax, [edi].plane.d
			movss	r, xmm0
			test	dwShadingFlags, E3DSAF_SINGLE_SIDE_PLANE
			shufps	xmm0, xmm0, 0
			cmovz	eax, r
			movups	xmm1, [edi].plane
			xor	eax, r
			js	Label_ErrorExit
			andps	xmm0, xmmMaskSignBit
			xorps	xmm0, xmm1
			movups	[edi].plane, xmm0
;		.ENDIF

		mov	ebx, [edi].pVertexes
		ASSUME	ebx:PE3D_VECTOR4
		.REPEAT
			mov	eax, [esi].normal
			add	esi, (SIZEOF E3D_PRIMITIVE_VERTEX)
			;
			movups	xmm0, [eax]
			movaps	xmm7, xmmMaskLP3
			andps	xmm0, xmm7
			;
			movaps	xmm1, xmm0
			mulps	xmm0, xmm0
			movaps	xmm2, xmm0
			movhlps	xmm3, xmm0
			shufps	xmm2, xmm2, 1
			addss	xmm0, xmm3
			addss	xmm0, xmm2
			rsqrtss	xmm0, xmm0
			shufps	xmm0, xmm0, 0
			mulps	xmm0, xmm1
			;
			movups	[edx], xmm0
			add	edx, (SIZEOF E3D_VECTOR4)
			dec	ecx
		.UNTIL	ZERO?
		;
		mov	ebx, hRenderPoly
		ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	.ELSE
		movaps	xmm7, xmmMaskLP3
		andps	xmm6, xmm7
		.REPEAT
			movups	[edx], xmm6
			add	edx, (SIZEOF E3D_VECTOR4)
			dec	ecx
		.UNTIL	ZERO?
	.ENDIF
	mov	esi, pPrimitive
	ASSUME	esi:PCE3D_PRIMITIVE_POLYGON
	;
	; 頂点色を設定
	;
	mov	edx, [edi].pVertexColors
	lea	esi, [esi].polygon
	mov	ecx, [edi].dwVertexCount
	ASSUME	esi:PTR E3D_PRIMITIVE_VERTEX
	ASSUME	edx:PE3D_COLOR
	.IF	[edi].dwTypeFlag & E3D_VERTEX_COLOR_POLYGON
		.REPEAT
			mov	eax, [esi].color.rgbMul.dwPixelCode
			mov	ebx, [esi].color.rgbAdd.dwPixelCode
			add	esi, (SIZEOF E3D_PRIMITIVE_VERTEX)
			mov	[edx].rgbMul.dwPixelCode, eax
			mov	[edx].rgbAdd.dwPixelCode, ebx
			add	edx, (SIZEOF E3D_COLOR)
			dec	ecx
		.UNTIL	ZERO?
	.ELSE
		mov	esi, [edi].pAttr
		ASSUME	esi:PE3D_SURFACE_ATTRIBUTE
		mov	eax, [esi].rgbaColor.rgbMul.dwPixelCode
		mov	ebx, [esi].rgbaColor.rgbAdd.dwPixelCode
		ASSUME	ebx:NOTHING
		.REPEAT
			mov	[edx].rgbMul.dwPixelCode, eax
			mov	[edx].rgbAdd.dwPixelCode, ebx
			add	edx, (SIZEOF E3D_COLOR)
			dec	ecx
		.UNTIL	ZERO?
	.ENDIF
	mov	esi, pPrimitive
	mov	ebx, hRenderPoly
	ASSUME	esi:PCE3D_PRIMITIVE_POLYGON
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	;
	; テクスチャーマッピングのパラメータを設定
	;
IF	0
	.IF	dwShadingFlags & (E3DSAF_GENVIRONMENT_MAP OR E3DSAF_ENVIRONMENT_MAP)
		;
		;	環境マッピング
		; ------------------------------------------------------------
		;
		; 頂点座標を取得
		;
		mov	ecx, [edi].pVertexes
		mov	esi, pPrimitive
		ASSUME	ecx:PE3D_VECTOR4
		ASSUME	esi:PCE3D_PRIMITIVE_POLYGON
		FOR	@INDEX, <0, 1, 2>
			mov	eax, vi[@INDEX * 4]
			movups	xmm0, [ecx + eax]
			movlps	xmm1, [esi].polygon[eax].uv_map
			movups	vVertexes[@INDEX*10H], xmm0
			movlps	vUVMaps[@INDEX*8], xmm1
		ENDM
		ASSUME	esi:NOTHING
		ASSUME	ecx:NOTHING
		;
		; マッピングパラメータ取得
		;
		.IF	dwShadingFlags & E3DSAF_GENVIRONMENT_MAP
			lea	eax, [ebx].genvmap
		.ELSE
			mov	eax, [edi].pAttr
			ASSUME	eax:PE3D_SURFACE_ATTRIBUTE
			lea	eax, [eax].envmap
			ASSUME	eax:NOTHING
		.ENDIF
		INVOKE	eglRenderPoly@SetEnvironmentMappingSSE ,
				ADDR [edi].surface.poly.txmap,
				ADDR vVertexes[0], ADDR vUVMaps[0],
				ADDR [ebx].envmat, eax
		;
		; 適用度設定
		;
		mov	ecx, [edi].pAttr
		ASSUME	ecx:PE3D_SURFACE_ATTRIBUTE
		mov	eax, [ecx].nTextureApply
		mov	edx, [ecx].nLuminousApply
		mov	[edi].surface.poly.txmap.nTextureApply, eax
		mov	[edi].surface.poly.txmap.nLuminousApply, edx
		mov	[edi].surface.poly.txmap.rFogDeepness, 0
		mov	edx, [ecx].nDeepness
		mov	eax, [ecx].nTransparency
		test	edx, edx
		.IF	ZERO?
			mov	[edi].dwTransparency, eax
		.ENDIF
		ASSUME	ecx:NOTHING

	.ELSEIF	dwShadingFlags & E3DSAF_TEXTURE_MAPPING
ELSE
	.IF	dwShadingFlags & E3DSAF_TEXTURE_MAPPING
ENDIF
	.IF	[edi].dwTypeFlag & E3D_TEXTURE_POLYGON
		;
		;	UV マップ
		; ------------------------------------------------------------
		;
		; 頂点座標を取得
		;
		mov	ecx, [edi].pVertexes
		mov	esi, pPrimitive
		ASSUME	ecx:PE3D_VECTOR4
		ASSUME	esi:PCE3D_PRIMITIVE_POLYGON
		FOR	@INDEX, <0, 1, 2>
			mov	eax, vi[@INDEX * 4]
			movups	xmm0, [ecx + eax]
			movlps	xmm1, [esi].polygon[eax].uv_map
			movups	vVertexes[@INDEX*10H], xmm0
			movlps	vUVMaps[@INDEX*8], xmm1
		ENDM
		ASSUME	esi:NOTHING
		ASSUME	ecx:NOTHING
		;
		; マッピングパラメータ取得
		;
		INVOKE	eglRenderPoly@SetTextureParameterSSE ,
				ADDR [edi].surface.poly.txmap,
				ADDR vVertexes[0], ADDR vUVMaps[0]
		;
		; テクスチャ画像を設定
		;
		mov	ecx, [edi].pAttr
		ASSUME	ecx:PE3D_SURFACE_ATTRIBUTE
		movss	xmm0, [edi].vCenter.z
		movss	xmm1, [ecx].txmap.rThresholdZ
		comiss	xmm0, xmm1
		.IF	!CARRY?
			mov	ecx, [ecx].txmap.nSmallScale
			mov	eax, 1
			shl	eax, cl
			;
			cvtsi2ss	xmm7, eax
			movups	xmm0, [edi].surface.poly.txmap.vAxisX
			movups	xmm1, [edi].surface.poly.txmap.vAxisY
			shufps	xmm7, xmm7, 0
			mulps	xmm0, xmm7
			mulps	xmm1, xmm7
			movups	[edi].surface.poly.txmap.vAxisX, xmm0
			movups	[edi].surface.poly.txmap.vAxisY, xmm1
			;
			mov	ecx, [edi].pAttr
			mov	eax, [ecx].txmap.pSmallImage
			mov	edx, [ecx].txmap.pSmallLuminous
		.ELSE
			mov	eax, [ecx].txmap.pTextureImage
			mov	edx, [ecx].txmap.pLuminousImage
		.ENDIF
		mov	[edi].surface.poly.txmap.pTextureImage, eax
		mov	[edi].surface.poly.txmap.pLuminousImage, edx
		mov	eax, [ecx].nTextureApply
		mov	edx, [ecx].nLuminousApply
		mov	[edi].surface.poly.txmap.nTextureApply, eax
		mov	[edi].surface.poly.txmap.nLuminousApply, edx
		mov	[edi].surface.poly.txmap.rFogDeepness, 0
		mov	edx, [ecx].nDeepness
		mov	eax, [ecx].nTransparency
		test	edx, edx
		.IF	ZERO?
			mov	[edi].dwTransparency, eax
		.ENDIF
		ASSUME	ecx:NOTHING
	.ELSE
		;
		;	テクスチャ無し
		; ------------------------------------------------------------
		and	[edi].dwTypeFlag, NOT E3D_TEXTURE_POLYGON
		and	[edi].dwShadingFlags, NOT E3DSAF_TEXTURE_MAPPING
	.ENDIF

	.ELSE
		;
		;	テクスチャ無し
		; ------------------------------------------------------------
		and	[edi].dwTypeFlag, NOT E3D_TEXTURE_POLYGON
		and	[edi].dwShadingFlags, NOT E3DSAF_TEXTURE_MAPPING
	.ENDIF
	.ENDIF
	mov	eax, edi

	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	ret


Label_ErrorExit:
	INVOKE	eslStackHeapLeave , hStackHeap, pPolyEntry
	xor	eax, eax
	ret

eglRenderPoly@CretaePolygonEntrySSE	ENDP


CodeSeg	ENDS

	END
