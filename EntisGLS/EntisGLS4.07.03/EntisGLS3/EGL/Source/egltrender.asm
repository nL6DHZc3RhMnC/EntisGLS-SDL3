
; ----------------------------------------------------------------------------
;                          Entis Graphic Library
; ----------------------------------------------------------------------------
;   Copyright (c) 2002-2008 Leshade Entis, Entis-soft. Al rights reserved.
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
xmmConst1	REAL4	1.0, 0.0, 0.0, 0.0
xmmConst2	REAL4	2.0, 2.0, 2.0, 2.0
xmmConstHalf	REAL4	0.5, 0.5, 0.5, 0.5

mmx0100HPW	LABEL	MMWORD
		WORD	4 DUP( 100H )

ConstSeg	ENDS


; ----------------------------------------------------------------------------
;	コードセグメント
; ----------------------------------------------------------------------------

CodeSeg	SEGMENT	PARA READONLY FLAT 'CODE'

;
;	シェーディング・透視変換・クリッピング 486 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@MakeUpPolygon486	PROC	NEAR32 C USES ebx esi edi,
	hRenderPoly:HEGL_RENDER_POLYGON,
	hStackHeap:HSTACKHEAP, pPolyEntry:PTR E3D_POLYGON_ENTRY

	LOCAL	rTemp[4]:REAL4, nTemp[4]:SDWORD
	LOCAL	nIndex:DWORD, nNextIndex:DWORD
	LOCAL	rZMinClip:REAL4, nZMinClip:DWORD
	LOCAL	nOrgVertexCount:DWORD
	LOCAL	pOrgVertexes:PE3D_VECTOR4
	LOCAL	pOrgNormals:PE3D_VECTOR4
	LOCAL	pOrgVertexColors:PE3D_COLOR
	LOCAL	vLastVertex:E3D_VECTOR4
	LOCAL	vLastNormal:E3D_VECTOR4
	LOCAL	clLastColor:E3D_COLOR

	mov	esi, pPolyEntry
	mov	ebx, hRenderPoly
	ASSUME	esi:PTR E3D_POLYGON_ENTRY
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF

	mov	eax, [esi].dwTypeFlag
	.IF	eax & E3D_MESH_POLYGON
	; --------------------------------------------------------------------
	;	ポリゴンメッシュ
	; --------------------------------------------------------------------
	;
	mov	eax, [esi].dwVertexCount
	mov	[esi].dwProjectedCount, eax
	;
	;	各頂点ごとのシェーディングを実施する
	; --------------------------------------------------------------------
	.IF	[esi].pNormals != NULL
		INVOKE	eglRenderPoly@ShadeVectors486 ,
				ebx, [esi].pAttr,
				[esi].pNormals, [esi].pVertexes,
				[esi].pVertexColors, [esi].dwVertexCount
	.ENDIF
	;
	mov	eax, esi
	ret


	.ELSEIF	eax == E3D_IMAGE_PRIMITIVE
	; --------------------------------------------------------------------
	;	画像
	; --------------------------------------------------------------------
	;
	; ｚ座標クリップ処理
	;
	mov	eax, [esi].vCenter.z
	mov	ecx, [ebx].rZMinClip
	mov	edx, [ebx].rZMaxClip
	xor	eax, 80000000H
	xor	ecx, 80000000H
	xor	edx, 80000000H
	.IF	(eax < ecx) || (eax > edx)
		xor	eax, eax
		ret
	.ENDIF
	;
	; 透視変換
	;
	INVOKE	eglRenderPoly@ProjectScreen486 ,
		ebx, ADDR [esi].surface.image.vCenter, ADDR [esi].vCenter, 1
	;
	mov	eax, esi
	ret

	.ELSEIF	eax == E3D_INFINITE_PLANE
	; --------------------------------------------------------------------
	;	無限平面
	; --------------------------------------------------------------------
	;
	; 各頂点色を設定
	;
	INVOKE	eslStackHeapAllocate , hStackHeap, (SIZEOF E3D_COLOR) * 4
	mov	edi, eax
	mov	[esi].pVertexColors, eax
	;
	mov	ecx, [esi].pAttr
	mov	eax, 00FFFFFFH
	xor	edx, edx
	.IF	ecx != NULL
		ASSUME	ecx:PTR E3D_SURFACE_ATTRIBUTE
		mov	eax, [ecx].rgbaColor.rgbMul.dwPixelCode
		mov	edx, [ecx].rgbaColor.rgbAdd.dwPixelCode
		ASSUME	ecx:NOTHING
	.ENDIF
	ASSUME	edi:PTR E3D_COLOR
	@INDEX = 0
	REPEAT	4
		mov	[edi + (SIZEOF E3D_COLOR) * @INDEX].rgbMul.dwPixelCode, eax
		mov	[edi + (SIZEOF E3D_COLOR) * @INDEX].rgbAdd.dwPixelCode, edx
		@INDEX = @INDEX + 1
	ENDM
	ASSUME	edi:NOTHING
	;
	mov	eax, [esi].plane.y
	mov	edx, [esi].plane.x
	and	eax, 7FFFFFFFH
	and	edx, 7FFFFFFFH
	.IF	eax < 37800000H		; abs([esi].plane.y) < 2^-16
	.IF	edx < 37800000H		; abs([esi].plane.x) < 2^-16
		;
		; 視線に対して垂直な平面
		;
		mov	eax, [esi].plane.d
		xor	eax, [esi].plane.z
		.IF	!SIGN?
			xor	eax, eax
			ret
		.ENDIF
	.ELSE
		;
		; 垂直な地平線
		;
		fld	[esi].plane.z
		fchs
		fdiv	[esi].plane.x
		fmul	[ebx].vScreenPos.z
		fadd	[ebx].vScreenPos.x
		fist	nTemp[0]
		fstp	rTemp[0]
		;
		mov	eax, [esi].plane.d
		mov	edx, nTemp[0]
		xor	eax, [esi].plane.x
		.IF	SIGN?
			.IF	edx > [ebx].dib.rectClip.right
				xor	eax, eax
				ret
			.ELSEIF	edx < [ebx].dib.rectClip.left
				mov	eax, [ebx].dib.rectClip.left
				mov	edx, [ebx].dib.rectClip.right
			.ELSE
				mov	eax, edx
				mov	edx, [ebx].dib.rectClip.right
			.ENDIF
		.ELSE
			.IF	edx < [ebx].dib.rectClip.left
				xor	eax, eax
				ret
			.ELSEIF	edx > [ebx].dib.rectClip.right
				mov	eax, [ebx].dib.rectClip.left
				mov	edx, [ebx].dib.rectClip.right
			.ELSE
				mov	eax, [ebx].dib.rectClip.left
			.ENDIF
		.ENDIF
		;
		mov	nTemp[0], eax
		mov	nTemp[4], edx
		;
		INVOKE	eslStackHeapAllocate ,
				hStackHeap, (SIZEOF E3D_VECTOR_2D) * 4
		mov	edi, eax
		mov	[esi].pProjVertexes, eax
		mov	[esi].dwProjectedCount, 4
		;
		ASSUME	edi:PTR E3D_VECTOR_2D
		fild	nTemp[0]
		fst	[edi + (SIZEOF E3D_VECTOR_2D) * 0].x
		fstp	[edi + (SIZEOF E3D_VECTOR_2D) * 3].x
		fild	[ebx].dib.rectClip.top
		fst	[edi + (SIZEOF E3D_VECTOR_2D) * 0].y
		fstp	[edi + (SIZEOF E3D_VECTOR_2D) * 1].y
		fild	nTemp[4]
		fst	[edi + (SIZEOF E3D_VECTOR_2D) * 1].x
		fstp	[edi + (SIZEOF E3D_VECTOR_2D) * 2].x
		fild	[ebx].dib.rectClip.bottom
		fst	[edi + (SIZEOF E3D_VECTOR_2D) * 2].y
		fstp	[edi + (SIZEOF E3D_VECTOR_2D) * 3].y
		ASSUME	edi:NOTHING
		;
		mov	eax, esi
		ret
	.ENDIF
	.ELSE
		;
		; 一般的な地平線
		;
		fld1
		fdiv	[esi].plane.y
		fchs
		;
		fild	[ebx].dib.rectClip.left
		fsub	[ebx].vScreenPos.x
		fmul	[esi].plane.x
		fld	[ebx].vScreenPos.z
		fmul	[esi].plane.z
		faddp	st(1), st
		fmul	st, st(1)
		fadd	[ebx].vScreenPos.y
		fist	nTemp[0]
		fstp	rTemp[0]
		;
		fild	[ebx].dib.rectClip.right
		fsub	[ebx].vScreenPos.x
		fmul	[esi].plane.x
		fld	[ebx].vScreenPos.z
		fmul	[esi].plane.z
		faddp	st(1), st
		fmulp	st(1), st
		fadd	[ebx].vScreenPos.y
		fist	nTemp[4]
		fstp	rTemp[4]
		;
		; 画面領域判定
		;
		mov	eax, [esi].plane.d
		mov	ecx, nTemp[0]
		xor	eax, [esi].plane.y
		mov	edx, nTemp[4]
		.IF	SIGN?
			.IF	(ecx > [ebx].dib.rectClip.bottom) \
					&& (edx > [ebx].dib.rectClip.bottom)
				xor	eax, eax
				ret
			.ELSEIF	(ecx > [ebx].dib.rectClip.top) \
					|| (edx > [ebx].dib.rectClip.top)
				INVOKE	eslStackHeapAllocate ,
					hStackHeap, (SIZEOF E3D_VECTOR_2D) * 4
				mov	edi, eax
				mov	[esi].pProjVertexes, eax
				mov	[esi].dwProjectedCount, 4
				;
				ASSUME	edi:PTR E3D_VECTOR_2D
				mov	eax, rTemp[0]
				mov	edx, rTemp[4]
				mov	[edi + (SIZEOF E3D_VECTOR_2D) * 0].y, eax
				mov	[edi + (SIZEOF E3D_VECTOR_2D) * 1].y, edx
				fild	[ebx].dib.rectClip.left
				fst	[edi + (SIZEOF E3D_VECTOR_2D) * 0].x
				fstp	[edi + (SIZEOF E3D_VECTOR_2D) * 3].x
				fild	[ebx].dib.rectClip.right
				fst	[edi + (SIZEOF E3D_VECTOR_2D) * 1].x
				fstp	[edi + (SIZEOF E3D_VECTOR_2D) * 2].x
				fild	[ebx].dib.rectClip.bottom
				fst	[edi + (SIZEOF E3D_VECTOR_2D) * 2].y
				fstp	[edi + (SIZEOF E3D_VECTOR_2D) * 3].y
				ASSUME	edi:NOTHING
				;
				mov	eax, esi
				ret
			.ENDIF
		.ELSE
			.IF	(ecx < [ebx].dib.rectClip.top) \
					&& (edx < [ebx].dib.rectClip.top)
				xor	eax, eax
				ret
			.ELSEIF	(ecx < [ebx].dib.rectClip.bottom) \
					|| (edx < [ebx].dib.rectClip.bottom)
				INVOKE	eslStackHeapAllocate ,
					hStackHeap, (SIZEOF E3D_VECTOR_2D) * 4
				mov	edi, eax
				mov	[esi].pProjVertexes, eax
				mov	[esi].dwProjectedCount, 4
				;
				ASSUME	edi:PTR E3D_VECTOR_2D
				mov	eax, rTemp[4]
				mov	edx, rTemp[0]
				mov	[edi + (SIZEOF E3D_VECTOR_2D) * 2].y, eax
				mov	[edi + (SIZEOF E3D_VECTOR_2D) * 3].y, edx
				fild	[ebx].dib.rectClip.left
				fst	[edi + (SIZEOF E3D_VECTOR_2D) * 0].x
				fstp	[edi + (SIZEOF E3D_VECTOR_2D) * 3].x
				fild	[ebx].dib.rectClip.top
				fst	[edi + (SIZEOF E3D_VECTOR_2D) * 0].y
				fstp	[edi + (SIZEOF E3D_VECTOR_2D) * 1].y
				fild	[ebx].dib.rectClip.right
				fst	[edi + (SIZEOF E3D_VECTOR_2D) * 1].x
				fstp	[edi + (SIZEOF E3D_VECTOR_2D) * 2].x
				ASSUME	edi:NOTHING
				;
				mov	eax, esi
				ret
			.ENDIF
		.ENDIF
	.ENDIF
	;
	; 全画面を覆う無限平面
	;
	INVOKE	eslStackHeapAllocate , hStackHeap, (SIZEOF E3D_VECTOR_2D) * 4
	mov	edi, eax
	mov	[esi].pProjVertexes, eax
	mov	[esi].dwProjectedCount, 4
	;
	ASSUME	edi:PTR E3D_VECTOR_2D
	fild	[ebx].dib.rectClip.left
	fst	[edi + (SIZEOF E3D_VECTOR_2D) * 0].x
	fstp	[edi + (SIZEOF E3D_VECTOR_2D) * 3].x
	fild	[ebx].dib.rectClip.top
	fst	[edi + (SIZEOF E3D_VECTOR_2D) * 0].y
	fstp	[edi + (SIZEOF E3D_VECTOR_2D) * 1].y
	fild	[ebx].dib.rectClip.right
	fst	[edi + (SIZEOF E3D_VECTOR_2D) * 1].x
	fstp	[edi + (SIZEOF E3D_VECTOR_2D) * 2].x
	fild	[ebx].dib.rectClip.bottom
	fst	[edi + (SIZEOF E3D_VECTOR_2D) * 2].y
	fstp	[edi + (SIZEOF E3D_VECTOR_2D) * 3].y
	ASSUME	edi:NOTHING
	;
	mov	eax, esi
	ret

	.ENDIF
	; --------------------------------------------------------------------
	;	通常ポリゴン
	; --------------------------------------------------------------------
	;
	;	ｚ座標クリップ処理
	; --------------------------------------------------------------------
	mov	eax, [esi].vCenter.z
	mov	edx, [ebx].rZMaxClip
	test	eax, eax
	.IF	SIGN?
		neg	eax
		or	eax, 80000000H
	.ENDIF
	test	edx, edx
	.IF	SIGN?
		neg	edx
		or	edx, 80000000H
	.ENDIF
	mov	ecx, [esi].dwVertexCount
	.IF	(eax > edx) || (ecx < 3)
		xor	eax, eax
		ret
	.ENDIF
	;
	; クリッピングが必要か判定
	;
	mov	edx, [ebx].rZMinClip
	test	edx, edx
	mov	rZMinClip, edx
	.IF	SIGN?
		neg	edx
		or	edx, 80000000H
	.ENDIF
	mov	nZMinClip, edx
	mov	esi, [esi].pVertexes
	ASSUME	esi:PE3D_VECTOR4
	mov	edx, ecx
	shl	edx, 4
	mov	eax, [esi + edx - 10H].z
	test	eax, eax
	.IF	SIGN?
		neg	eax
		or	eax, 80000000H
	.ENDIF
	cmp	eax, nZMinClip
	setl	al
	movzx	edi, al
	xor	edx, edx
	mov	nTemp[0], 0
	;
	.REPEAT
		mov	eax, [esi].z
		add	esi, (SIZEOF E3D_VECTOR4)
		test	eax, eax
		.IF	SIGN?
			neg	eax
			or	eax, 80000000H
		.ENDIF
		cmp	eax, nZMinClip
		setl	al
		movzx	eax, al
		.IF	eax != edi
			inc	edx
			mov	edi, eax
		.ENDIF
		test	eax, eax
		.IF	ZERO?
			inc	edx
		.ELSE
			mov	nTemp[0], 1
		.ENDIF
		dec	ecx
	.UNTIL	ZERO?
	;
	mov	esi, pPolyEntry
	mov	eax, nTemp[0]
	mov	nTemp[4], edx
	ASSUME	esi:PTR E3D_POLYGON_ENTRY
	;
	.IF	eax != 0
		;
		; ｚクリッピング必要
		;
		ASSUME	ebx:NOTHING
		mov	eax, [esi].dwVertexCount
		mov	ecx, [esi].pVertexes
		mov	edx, [esi].pNormals
		mov	edi, [esi].pVertexColors
		mov	nOrgVertexCount, eax
		mov	pOrgVertexes, ecx
		mov	pOrgNormals, edx
		mov	pOrgVertexColors, edi
		;
		mov	edi, nTemp[4]
		mov	[esi].dwVertexCount, edi
		.IF	edi < 3
			xor	eax, eax
			ret
		.ENDIF
		shl	edi, 4
		INVOKE	eslStackHeapAllocate , hStackHeap, edi
		mov	[esi].pVertexes, eax
		INVOKE	eslStackHeapAllocate , hStackHeap, edi
		mov	[esi].pNormals, eax
		shr	edi, 1
		INVOKE	eslStackHeapAllocate , hStackHeap, edi
		mov	[esi].pVertexColors, eax
		;
		; クリッピングの為に頂点情報をコピーする
		;
		mov	edx, nOrgVertexCount
		dec	edx
		mov	edi, pOrgVertexes
		shl	edx, 3
		ASSUME	edi:PE3D_VECTOR4
		mov	eax, [edi + edx * 2].x
		mov	ebx, [edi + edx * 2].y
		mov	ecx, [edi + edx * 2].z
		mov	vLastVertex.x, eax
		mov	vLastVertex.y, ebx
		mov	vLastVertex.z, ecx
		;
		mov	edi, pOrgNormals
		mov	eax, [edi + edx * 2].x
		mov	ebx, [edi + edx * 2].y
		mov	ecx, [edi + edx * 2].z
		mov	vLastNormal.x, eax
		mov	vLastNormal.y, ebx
		mov	vLastNormal.z, ecx
		;
		mov	edi, pOrgVertexColors
		ASSUME	edi:PE3D_COLOR
		mov	eax, [edi + edx].rgbMul.dwPixelCode
		mov	ecx, [edi + edx].rgbAdd.dwPixelCode
		mov	clLastColor.rgbMul.dwPixelCode, eax
		mov	clLastColor.rgbAdd.dwPixelCode, ecx
		ASSUME	edi:NOTHING
		;
		mov	eax, vLastVertex.z
		test	eax, eax
		.IF	SIGN?
			neg	eax
			or	eax, 80000000H
		.ENDIF
		cmp	eax, nZMinClip
		setl	al
		movzx	eax, al
		mov	nTemp[0], eax
		;
		; 頂点をコピーしながらクリッピング
		;
		xor	edx, edx
		xor	ecx, ecx
		.REPEAT
			mov	nIndex, edx
			mov	nNextIndex, ecx
			;
			mov	edi, pOrgVertexes
			shl	edx, 3
			ASSUME	edi:PE3D_VECTOR4
			mov	eax, [edi + edx * 2].z
			test	eax, eax
			.IF	SIGN?
				neg	eax
				or	eax, 80000000H
			.ENDIF
			cmp	eax, nZMinClip
			setl	al
			movzx	eax, al
			.IF	eax != nTemp[0]
				;
				; クリッピング処理
				;
				mov	nTemp[0], eax
				mov	nTemp[4], 100H
				fld	rZMinClip
				fsub	vLastVertex.z
				fld	[edi + edx * 2].z
				fsub	vLastVertex.z
				fdivp	st(1), st
				fst	rTemp[0]
				fimul	nTemp[4]
				mov	ecx, nNextIndex
				fistp	nTemp[4]
				;
				shl	ecx, 3
				mov	ebx, [esi].pVertexes
				ASSUME	ebx:PE3D_VECTOR4
				mov	eax, rZMinClip
				FOR	@MEMBER, <x, y>
					fld	vLastVertex.@MEMBER
					fld	[edi + edx * 2].@MEMBER
					fsub	st, st(1)
					fmul	rTemp[0]
					faddp	st(1), st
					fstp	[ebx + ecx * 2].@MEMBER
				ENDM
				mov	[ebx + ecx * 2].z, eax
				;
				mov	edi, pOrgNormals
				mov	ebx, [esi].pNormals
				FOR	@MEMBER, <x, y, z>
					fld	vLastNormal.@MEMBER
					fld	[edi + edx * 2].@MEMBER
					fsub	st, st(1)
					fmul	rTemp[0]
					faddp	st(1), st
				ENDM
				fld	st(2)
				fmul	st, st(0)
				fld	st(2)
				fmul	st, st(0)
				fld	st(2)
				fmul	st, st(0)
				faddp	st(1), st
				faddp	st(1), st
				fsqrt
				fld1
				fdivrp	st(1), st
				fmul	st(1), st
				fmul	st(2), st
				fmulp	st(3), st
				fstp	[ebx + ecx * 2].z
				fstp	[ebx + ecx * 2].y
				fstp	[ebx + ecx * 2].x
				;
				mov	edi, pOrgVertexColors
				mov	ebx, [esi].pVertexColors
				ASSUME	edi:PE3D_COLOR
				ASSUME	ebx:PE3D_COLOR
				add	edi, edx
				add	ebx, ecx
				FOR	@MEMBER, <rgb.Blue, rgb.Green, rgb.Red>
					movzx	eax, [edi].rgbMul.@MEMBER
					movzx	edx, clLastColor.rgbMul.@MEMBER
					movzx	ecx, [edi].rgbAdd.@MEMBER
					sub	eax, edx
					movzx	edx, clLastColor.rgbAdd.@MEMBER
					sub	ecx, edx
					mov	edx, nTemp[4]
					imul	eax, edx
					imul	ecx, edx
					add	ah, clLastColor.rgbMul.@MEMBER
					add	ch, clLastColor.rgbAdd.@MEMBER
					mov	[ebx].rgbMul.@MEMBER, ah
					mov	[ebx].rgbAdd.@MEMBER, ch
				ENDM
				ASSUME	edi:NOTHING
				ASSUME	ecx:NOTHING
				mov	edx, nIndex
				mov	edi, pOrgVertexes
				shl	edx, 3
				inc	nNextIndex
			.ENDIF
			;
			; 頂点複製
			;
			ASSUME	edi:PE3D_VECTOR4
			mov	eax, [edi + edx * 2].x
			mov	ebx, [edi + edx * 2].y
			mov	ecx, [edi + edx * 2].z
			mov	vLastVertex.x, eax
			mov	vLastVertex.y, ebx
			mov	vLastVertex.z, ecx
			;
			mov	edi, pOrgNormals
			mov	eax, [edi + edx * 2].x
			mov	ebx, [edi + edx * 2].y
			mov	ecx, [edi + edx * 2].z
			mov	vLastNormal.x, eax
			mov	vLastNormal.y, ebx
			mov	vLastNormal.z, ecx
			;
			mov	edi, pOrgVertexColors
			ASSUME	edi:PE3D_COLOR
			mov	eax, [edi + edx].rgbMul.dwPixelCode
			mov	ecx, [edi + edx].rgbAdd.dwPixelCode
			mov	clLastColor.rgbMul.dwPixelCode, eax
			mov	eax, nTemp[0]
			mov	clLastColor.rgbAdd.dwPixelCode, ecx
			ASSUME	edi:NOTHING
			;
			.IF	eax == 0
				mov	ecx, nNextIndex
				mov	edi, [esi].pVertexes
				shl	ecx, 3
				ASSUME	edi:PE3D_VECTOR4
				mov	eax, vLastVertex.x
				mov	ebx, vLastVertex.y
				mov	edx, vLastVertex.z
				mov	[edi + ecx * 2].x, eax
				mov	[edi + ecx * 2].y, ebx
				mov	[edi + ecx * 2].z, edx
				;
				mov	edi, [esi].pNormals
				mov	eax, vLastNormal.x
				mov	ebx, vLastNormal.y
				mov	edx, vLastNormal.z
				mov	[edi + ecx * 2].x, eax
				mov	[edi + ecx * 2].y, ebx
				mov	[edi + ecx * 2].z, edx
				;
				mov	edi, [esi].pVertexColors
				ASSUME	edi:PE3D_COLOR
				mov	eax, clLastColor.rgbMul.dwPixelCode
				mov	edx, clLastColor.rgbAdd.dwPixelCode
				mov	[edi + ecx].rgbMul.dwPixelCode, eax
				mov	[edi + ecx].rgbAdd.dwPixelCode, edx
				ASSUME	edi:NOTHING
				;
				inc	nNextIndex
			.ENDIF
			;
			mov	edx, nIndex
			mov	ecx, nNextIndex
			inc	edx
		.UNTIL	edx >= nOrgVertexCount
		;
		ASSERT	<ecx == [esi].dwVertexCount>, \
			"ecx == [esi].dwVertexCount"
		mov	ebx, hRenderPoly
		ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	.ENDIF
	;
	;	各頂点ごとのシェーディングを実施する
	; --------------------------------------------------------------------
	INVOKE	eglRenderPoly@ShadeVectors486 ,
			ebx, [esi].pAttr,
				[esi].pNormals, [esi].pVertexes,
				[esi].pVertexColors, [esi].dwVertexCount
	;
	;	透視変換
	; --------------------------------------------------------------------
	mov	eax, [esi].dwVertexCount
	mov	[esi].dwProjectedCount, eax
	shl	eax, 3
	INVOKE	eslStackHeapAllocate , hStackHeap, eax
	mov	[esi].pProjVertexes, eax
	mov	edi, eax
	INVOKE	eglRenderPoly@ProjectScreen486 ,
		ebx, edi, [esi].pVertexes, [esi].dwVertexCount

	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	mov	eax, esi
	ret

eglRenderPoly@MakeUpPolygon486	ENDP

;
;	シェーディング・透視変換・クリッピング SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@MakeUpPolygonSSE	PROC	NEAR32 C USES ebx esi edi,
	hRenderPoly:HEGL_RENDER_POLYGON,
	hStackHeap:HSTACKHEAP, pPolyEntry:PTR E3D_POLYGON_ENTRY

	LOCAL	rTemp[4]:REAL4, nTemp[4]:SDWORD
	LOCAL	nIndex:DWORD, nNextIndex:DWORD
	LOCAL	rZMinClip:REAL4, nZMinClip:DWORD
	LOCAL	nOrgVertexCount:DWORD
	LOCAL	pOrgVertexes:PE3D_VECTOR4
	LOCAL	pOrgNormals:PE3D_VECTOR4
	LOCAL	pOrgVertexColors:PE3D_COLOR
	LOCAL	vLastVertex:E3D_VECTOR4
	LOCAL	vLastNormal:E3D_VECTOR4
	LOCAL	clLastColor:E3D_COLOR

	mov	esi, pPolyEntry
	mov	ebx, hRenderPoly
	ASSUME	esi:PTR E3D_POLYGON_ENTRY
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF

	mov	eax, [esi].dwTypeFlag
	.IF	eax & E3D_MESH_POLYGON
	; --------------------------------------------------------------------
	;	ポリゴンメッシュ
	; --------------------------------------------------------------------
	;
	mov	eax, [esi].dwVertexCount
	mov	[esi].dwProjectedCount, eax
	;
	;	各頂点ごとのシェーディングを実施する
	; --------------------------------------------------------------------
	.IF	[esi].pNormals != NULL
		.IF	!([esi].dwShadingFlags & E3DSAF_PHONG_SHADE)
			INVOKE	eglRenderPoly@ShadeVectorsSSE ,
					ebx, [esi].pAttr,
					[esi].pNormals, [esi].pVertexes,
					[esi].pVertexColors,
					[esi].dwVertexCount, NULL
			emms
		.ENDIF
	.ENDIF
	;
	mov	eax, esi
	ret


	.ELSEIF	eax == E3D_IMAGE_PRIMITIVE
	; --------------------------------------------------------------------
	;	画像
	; --------------------------------------------------------------------
	;
	; ｚ座標クリップ処理
	;
	mov	eax, [esi].vCenter.z
	mov	ecx, [ebx].rZMinClip
	mov	edx, [ebx].rZMaxClip
	xor	eax, 80000000H
	xor	ecx, 80000000H
	xor	edx, 80000000H
	.IF	(eax < ecx) || (eax > edx)
		xor	eax, eax
		ret
	.ENDIF
	;
	; 透視変換
	;
	INVOKE	eglRenderPoly@ProjectScreenSSE ,
		ebx, ADDR [esi].surface.image.vCenter, ADDR [esi].vCenter, 1
	;
	mov	eax, esi
	ret

	.ELSEIF	eax == E3D_INFINITE_PLANE
	; --------------------------------------------------------------------
	;	無限平面
	; --------------------------------------------------------------------
	;
	; 各頂点色を設定
	;
	INVOKE	eslStackHeapAllocate , hStackHeap, (SIZEOF E3D_COLOR) * 4
	mov	edi, eax
	mov	[esi].pVertexColors, eax
	;
	mov	ecx, [esi].pAttr
	mov	eax, 00FFFFFFH
	xor	edx, edx
	.IF	ecx != NULL
		ASSUME	ecx:PE3D_SURFACE_ATTRIBUTE
		mov	eax, [ecx].rgbaColor.rgbMul.dwPixelCode
		mov	edx, [ecx].rgbaColor.rgbAdd.dwPixelCode
		ASSUME	ecx:NOTHING
	.ENDIF
	ASSUME	edi:PTR E3D_COLOR
	@INDEX = 0
	REPEAT	4
		mov	[edi + (SIZEOF E3D_COLOR) * @INDEX].rgbMul.dwPixelCode, eax
		mov	[edi + (SIZEOF E3D_COLOR) * @INDEX].rgbAdd.dwPixelCode, edx
		@INDEX = @INDEX + 1
	ENDM
	ASSUME	edi:NOTHING
	;
	mov	eax, [esi].plane.y
	mov	edx, [esi].plane.x
	and	eax, 7FFFFFFFH
	and	edx, 7FFFFFFFH
	.IF	eax < 37800000H		; abs([esi].plane.y) < 2^-16
	.IF	edx < 37800000H		; abs([esi].plane.x) < 2^-16
		;
		; 視線に対して垂直な平面
		;
		mov	eax, [esi].plane.d
		xor	eax, [esi].plane.z
		.IF	!SIGN?
			xor	eax, eax
			ret
		.ENDIF
	.ELSE
		;
		; 垂直な地平線
		;
		movss	xmm0, [esi].plane.z
		movss	xmm1, [esi].plane.x
		movss	xmm2, [ebx].vScreenPos.x
		movss	xmm3, [ebx].vScreenPos.z
		divss	xmm0, xmm1
		mulss	xmm0, xmm3
		subss	xmm2, xmm0
		;
		mov	eax, [esi].plane.d
		cvtss2si	edx, xmm2
		xor	eax, [esi].plane.x
		.IF	SIGN?
			.IF	edx > [ebx].dib.rectClip.right
				xor	eax, eax
				ret
			.ELSEIF	edx < [ebx].dib.rectClip.left
				mov	eax, [ebx].dib.rectClip.left
				mov	edx, [ebx].dib.rectClip.right
			.ELSE
				mov	eax, edx
				mov	edx, [ebx].dib.rectClip.right
			.ENDIF
		.ELSE
			.IF	edx < [ebx].dib.rectClip.left
				xor	eax, eax
				ret
			.ELSEIF	edx > [ebx].dib.rectClip.right
				mov	eax, [ebx].dib.rectClip.left
				mov	edx, [ebx].dib.rectClip.right
			.ELSE
				mov	eax, [ebx].dib.rectClip.left
			.ENDIF
		.ENDIF
		;
		mov	nTemp[0], eax
		mov	nTemp[4], edx
		;
		INVOKE	eslStackHeapAllocate ,
				hStackHeap, (SIZEOF E3D_VECTOR_2D) * 4
		mov	edi, eax
		mov	[esi].pProjVertexes, eax
		mov	[esi].dwProjectedCount, 4
		;
		ASSUME	edi:PTR E3D_VECTOR_2D
		cvtsi2ss	xmm0, nTemp[0]
		cvtsi2ss	xmm1, [ebx].dib.rectClip.top
		cvtsi2ss	xmm2, nTemp[4]
		cvtsi2ss	xmm3, [ebx].dib.rectClip.bottom
		movss	[edi + (SIZEOF E3D_VECTOR_2D) * 0].x, xmm0
		movss	[edi + (SIZEOF E3D_VECTOR_2D) * 3].x, xmm0
		movss	[edi + (SIZEOF E3D_VECTOR_2D) * 0].y, xmm1
		movss	[edi + (SIZEOF E3D_VECTOR_2D) * 1].y, xmm1
		movss	[edi + (SIZEOF E3D_VECTOR_2D) * 1].x, xmm2
		movss	[edi + (SIZEOF E3D_VECTOR_2D) * 2].x, xmm2
		movss	[edi + (SIZEOF E3D_VECTOR_2D) * 2].y, xmm3
		movss	[edi + (SIZEOF E3D_VECTOR_2D) * 3].y, xmm3
		ASSUME	edi:NOTHING
		;
		mov	eax, esi
		ret
	.ENDIF
	.ELSE
		;
		; 一般的な地平線
		;
		movss	xmm1, [esi].plane.y
		movss	xmm0, xmmConst1
		movss	xmm2, [esi].plane.z
		divss	xmm0, xmm1
		;
		cvtsi2ss	xmm4, [ebx].dib.rectClip.left
		cvtsi2ss	xmm5, [ebx].dib.rectClip.right
		movss	xmm1, [esi].plane.x
		movss	xmm6, [ebx].vScreenPos.x
		movss	xmm7, [ebx].vScreenPos.y
		movss	xmm3, [ebx].vScreenPos.z
		unpcklps	xmm4, xmm5
		shufps	xmm1, xmm1, 0
		mulss	xmm2, xmm3
		shufps	xmm6, xmm6, 0
		shufps	xmm7, xmm7, 0
		subps	xmm4, xmm6
		shufps	xmm2, xmm2, 0
		;
		mulps	xmm4, xmm1
		shufps	xmm0, xmm0, 0
		addps	xmm4, xmm2
		mulps	xmm4, xmm0
		subps	xmm7, xmm4
		cvtps2pi	mm0, xmm7
		movlps	QWORD PTR rTemp[0], xmm7
		;
		; 画面領域判定
		;
		mov	eax, [esi].plane.d
		movd	ecx, mm0
		psrlq	mm0, 32
		xor	eax, [esi].plane.y
		movd	edx, mm0
		emms
		.IF	SIGN?
			.IF	(ecx > [ebx].dib.rectClip.bottom) \
					&& (edx > [ebx].dib.rectClip.bottom)
				xor	eax, eax
				ret
			.ELSEIF	(ecx > [ebx].dib.rectClip.top) \
					|| (edx > [ebx].dib.rectClip.top)
				INVOKE	eslStackHeapAllocate ,
					hStackHeap, (SIZEOF E3D_VECTOR_2D) * 4
				mov	edi, eax
				mov	[esi].pProjVertexes, eax
				mov	[esi].dwProjectedCount, 4
				;
				ASSUME	edi:PTR E3D_VECTOR_2D
				mov	eax, rTemp[0]
				mov	edx, rTemp[4]
				cvtsi2ss	xmm1, [ebx].dib.rectClip.left
				cvtsi2ss	xmm2, [ebx].dib.rectClip.right
				cvtsi2ss	xmm3, [ebx].dib.rectClip.bottom
				mov	[edi + (SIZEOF E3D_VECTOR_2D) * 0].y, eax
				mov	[edi + (SIZEOF E3D_VECTOR_2D) * 1].y, edx
				movss	[edi + (SIZEOF E3D_VECTOR_2D) * 0].x, xmm1
				movss	[edi + (SIZEOF E3D_VECTOR_2D) * 3].x, xmm1
				movss	[edi + (SIZEOF E3D_VECTOR_2D) * 1].x, xmm2
				movss	[edi + (SIZEOF E3D_VECTOR_2D) * 2].x, xmm2
				movss	[edi + (SIZEOF E3D_VECTOR_2D) * 2].y, xmm3
				movss	[edi + (SIZEOF E3D_VECTOR_2D) * 3].y, xmm3
				ASSUME	edi:NOTHING
				;
				mov	eax, esi
				ret
			.ENDIF
		.ELSE
			.IF	(ecx < [ebx].dib.rectClip.top) \
					&& (edx < [ebx].dib.rectClip.top)
				xor	eax, eax
				ret
			.ELSEIF	(ecx < [ebx].dib.rectClip.bottom) \
					|| (edx < [ebx].dib.rectClip.bottom)
				INVOKE	eslStackHeapAllocate ,
					hStackHeap, (SIZEOF E3D_VECTOR_2D) * 4
				mov	edi, eax
				mov	[esi].pProjVertexes, eax
				mov	[esi].dwProjectedCount, 4
				;
				ASSUME	edi:PTR E3D_VECTOR_2D
				mov	eax, rTemp[4]
				mov	edx, rTemp[0]
				cvtsi2ss	xmm1, [ebx].dib.rectClip.left
				cvtsi2ss	xmm2, [ebx].dib.rectClip.top
				cvtsi2ss	xmm3, [ebx].dib.rectClip.right
				mov	[edi + (SIZEOF E3D_VECTOR_2D) * 2].y, eax
				mov	[edi + (SIZEOF E3D_VECTOR_2D) * 3].y, edx
				movss	[edi + (SIZEOF E3D_VECTOR_2D) * 0].x, xmm1
				movss	[edi + (SIZEOF E3D_VECTOR_2D) * 3].x, xmm1
				movss	[edi + (SIZEOF E3D_VECTOR_2D) * 0].y, xmm2
				movss	[edi + (SIZEOF E3D_VECTOR_2D) * 1].y, xmm2
				movss	[edi + (SIZEOF E3D_VECTOR_2D) * 1].x, xmm3
				movss	[edi + (SIZEOF E3D_VECTOR_2D) * 2].x, xmm3
				ASSUME	edi:NOTHING
				;
				mov	eax, esi
				ret
			.ENDIF
		.ENDIF
	.ENDIF
	;
	; 全画面を覆う無限平面
	;
	INVOKE	eslStackHeapAllocate , hStackHeap, (SIZEOF E3D_VECTOR_2D) * 4
	mov	edi, eax
	mov	[esi].pProjVertexes, eax
	mov	[esi].dwProjectedCount, 4
	;
	ASSUME	edi:PTR E3D_VECTOR_2D
	cvtsi2ss	xmm0, [ebx].dib.rectClip.left
	cvtsi2ss	xmm1, [ebx].dib.rectClip.top
	cvtsi2ss	xmm2, [ebx].dib.rectClip.right
	cvtsi2ss	xmm3, [ebx].dib.rectClip.bottom
	movss	[edi + (SIZEOF E3D_VECTOR_2D) * 0].x, xmm0
	movss	[edi + (SIZEOF E3D_VECTOR_2D) * 3].x, xmm0
	movss	[edi + (SIZEOF E3D_VECTOR_2D) * 0].y, xmm1
	movss	[edi + (SIZEOF E3D_VECTOR_2D) * 1].y, xmm1
	movss	[edi + (SIZEOF E3D_VECTOR_2D) * 1].x, xmm2
	movss	[edi + (SIZEOF E3D_VECTOR_2D) * 2].x, xmm2
	movss	[edi + (SIZEOF E3D_VECTOR_2D) * 2].y, xmm3
	movss	[edi + (SIZEOF E3D_VECTOR_2D) * 3].y, xmm3
	ASSUME	edi:NOTHING
	;
	mov	eax, esi
	ret

	.ENDIF
	; --------------------------------------------------------------------
	;	通常ポリゴン
	; --------------------------------------------------------------------
	;
	;	ｚ座標クリップ処理
	; --------------------------------------------------------------------
	mov	eax, [esi].vCenter.z
	mov	edx, [ebx].rZMaxClip
	xor	eax, 80000000H
	xor	edx, 80000000H
	mov	ecx, [esi].dwVertexCount
	.IF	(eax > edx) || (ecx < 3)
		xor	eax, eax
		ret
	.ENDIF
	;
	; クリッピングが必要か判定
	;
	movss	xmm7, [ebx].rZMinClip
	movss	rZMinClip, xmm7
	mov	esi, [esi].pVertexes
	ASSUME	esi:PE3D_VECTOR4
	mov	edx, ecx
	shl	edx, 4
	movss	xmm0, [esi + edx - 10H].z
	comiss	xmm0, xmm7
	sbb	edi, edi
	xor	edx, edx
	mov	nTemp[0], 0
	;
	.REPEAT
		movss	xmm0, [esi].z
		add	esi, (SIZEOF E3D_VECTOR4)
		comiss	xmm0, xmm7
		sbb	eax, eax
		.IF	eax != edi
			inc	edx
			mov	edi, eax
		.ENDIF
		test	eax, eax
		.IF	ZERO?
			inc	edx
		.ELSE
			mov	nTemp[0], 1
		.ENDIF
		dec	ecx
	.UNTIL	ZERO?
	;
	mov	esi, pPolyEntry
	mov	eax, nTemp[0]
	mov	nTemp[4], edx
	ASSUME	esi:PTR E3D_POLYGON_ENTRY
	;
	.IF	eax != 0
		;
		; ｚクリッピング必要
		;
		ASSUME	ebx:NOTHING
		mov	eax, [esi].dwVertexCount
		mov	ecx, [esi].pVertexes
		mov	edx, [esi].pNormals
		mov	edi, [esi].pVertexColors
		mov	nOrgVertexCount, eax
		mov	pOrgVertexes, ecx
		mov	pOrgNormals, edx
		mov	pOrgVertexColors, edi
		;
		mov	edi, nTemp[4]
		mov	[esi].dwVertexCount, edi
		.IF	edi < 3
			xor	eax, eax
			ret
		.ENDIF
		shl	edi, 4
		INVOKE	eslStackHeapAllocate , hStackHeap, edi
		mov	[esi].pVertexes, eax
		INVOKE	eslStackHeapAllocate , hStackHeap, edi
		mov	[esi].pNormals, eax
		shr	edi, 1
		INVOKE	eslStackHeapAllocate , hStackHeap, edi
		mov	[esi].pVertexColors, eax
		;
		; クリッピングの為に頂点情報をコピーする
		;
		mov	edx, nOrgVertexCount
		dec	edx
		mov	edi, pOrgVertexes
		shl	edx, 3
		ASSUME	edi:PE3D_VECTOR4
		movups	xmm0, [edi + edx * 2]
		movups	vLastVertex, xmm0
		;
		mov	edi, pOrgNormals
		movups	xmm0, [edi + edx * 2]
		movups	vLastNormal, xmm0
		;
		mov	edi, pOrgVertexColors
		ASSUME	edi:PE3D_COLOR
		movq	mm0, MMWORD PTR [edi + edx]
		movq	MMWORD PTR clLastColor, mm0
		ASSUME	edi:NOTHING
		;
		movss	xmm0, vLastVertex.z
		movss	xmm7, rZMinClip
		comiss	xmm0, xmm7
		sbb	eax, eax
		mov	nTemp[0], eax
		;
		; 頂点をコピーしながらクリッピング
		;
		xor	edx, edx
		xor	ecx, ecx
		.REPEAT
			mov	nIndex, edx
			mov	nNextIndex, ecx
			;
			mov	edi, pOrgVertexes
			shl	edx, 3
			ASSUME	edi:PE3D_VECTOR4
			movss	xmm7, rZMinClip
			movss	xmm0, [edi + edx * 2].z
			comiss	xmm0, xmm7
			sbb	eax, eax
			.IF	eax != nTemp[0]
				;
				; クリッピング処理
				;
				mov	nTemp[0], eax
				mov	eax, 100H
				movss	xmm2, vLastVertex.z
				movss	xmm1, xmm7
				subss	xmm0, xmm2
				subss	xmm1, xmm2
				mov	ecx, nNextIndex
				divss	xmm1, xmm0
				cvtsi2ss	xmm2, eax
				shl	ecx, 3
				mov	ebx, [esi].pVertexes
				mulss	xmm2, xmm1
				xorps	xmm4, xmm4
				cvtps2pi	mm7, xmm2
				shufps	xmm1, xmm1, 0
				xorps	xmm5, xmm5
				pshufw	mm7, mm7, 0
				movaps	xmm7, xmm1
				;
				ASSUME	ebx:PE3D_VECTOR4
				movlps	xmm4, QWORD PTR vLastVertex
				movlps	xmm5, QWORD PTR [edi + edx * 2]
				subps	xmm5, xmm4
				mulps	xmm5, xmm7
				movss	xmm6, rZMinClip
				addps	xmm4, xmm5
				movlhps	xmm4, xmm6
				movups	[ebx + ecx * 2], xmm4
				;
				mov	edi, pOrgNormals
				mov	ebx, [esi].pNormals
				movups	xmm4, vLastNormal
				movups	xmm5, [edi + edx * 2]
				subps	xmm5, xmm4
				mulps	xmm5, xmm7
				addps	xmm4, xmm5
				;
				movaps	xmm0, xmm4	; 法線の正規化
				mulps	xmm0, xmm0
				movhlps	xmm1, xmm0
				movaps	xmm2, xmm0
				shufps	xmm0, xmm0, 1
				addss	xmm1, xmm2
				addss	xmm0, xmm1
				rsqrtss	xmm0, xmm0
				shufps	xmm0, xmm0, 0
				mulps	xmm4, xmm0
				;
				movups	[ebx + ecx * 2], xmm4
				;
				mov	edi, pOrgVertexColors
				mov	ebx, [esi].pVertexColors
				ASSUME	edi:PE3D_COLOR
				ASSUME	ebx:PE3D_COLOR
				add	edi, edx
				add	ebx, ecx
				pxor	mm6, mm6
				movq	mm0, MMWORD PTR [edi]
				movq	mm2, MMWORD PTR clLastColor
				movq	mm1, mm0
				punpcklbw	mm0, mm6
				movq	mm3, mm2
				punpcklbw	mm2, mm6
				punpckhbw	mm1, mm6
				psubsw	mm0, mm2
				psllw	mm2, 8
				punpckhbw	mm3, mm6
				psubsw	mm1, mm3
				psllw	mm3, 8
				pmullw	mm0, mm7
				pmullw	mm1, mm7
				paddw	mm0, mm2
				paddw	mm1, mm3
				psrlw	mm0, 8
				psrlw	mm1, 8
				packuswb	mm0, mm1
				movq	MMWORD PTR [ebx], mm0
				ASSUME	edi:NOTHING
				ASSUME	ecx:NOTHING
				mov	edx, nIndex
				mov	edi, pOrgVertexes
				shl	edx, 3
				inc	nNextIndex
			.ENDIF
			;
			; 頂点複製
			;
			mov	eax, pOrgNormals
			mov	ecx, pOrgVertexColors
			ASSUME	edi:PE3D_VECTOR4
			ASSUME	eax:PE3D_VECTOR4
			ASSUME	ecx:PE3D_COLOR
			movups	xmm0, [edi + edx * 2]
			movups	xmm1, [eax + edx * 2]
			movq	mm0, MMWORD PTR [ecx + edx]
			movups	XMMWORD_PTR vLastVertex, xmm0
			movups	vLastNormal, xmm1
			mov	eax, nTemp[0]
			movq	MMWORD PTR clLastColor, mm0
			;
			.IF	eax == 0
				mov	edx, nNextIndex
				mov	edi, [esi].pVertexes
				shl	edx, 3
				mov	eax, [esi].pNormals
				mov	ecx, [esi].pVertexColors
				;
				movups	[edi + edx * 2], xmm0
				movups	[eax + edx * 2], xmm1
				movq	MMWORD PTR [ecx + edx], mm0
				;
				inc	nNextIndex
			.ENDIF
			ASSUME	edi:NOTHING
			ASSUME	eax:NOTHING
			ASSUME	ecx:NOTHING
			;
			mov	edx, nIndex
			mov	ecx, nNextIndex
			inc	edx
		.UNTIL	edx >= nOrgVertexCount
		;
		emms
		ASSERT	<ecx == [esi].dwVertexCount>, \
			"ecx == [esi].dwVertexCount"
		mov	ebx, hRenderPoly
		ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	.ENDIF
	;
	;	透視変換
	; --------------------------------------------------------------------
	mov	eax, [esi].dwVertexCount
	mov	[esi].dwProjectedCount, eax
	shl	eax, 3
	INVOKE	eslStackHeapAllocate , hStackHeap, eax
	mov	[esi].pProjVertexes, eax
	mov	edi, eax
	INVOKE	eglRenderPoly@ProjectScreenSSE ,
			ebx, edi, [esi].pVertexes, [esi].dwVertexCount
	;
	ASSUME	edi:PTR E3D_VECTOR_2D
	xorps	xmm0, xmm0
	xorps	xmm2, xmm2
	mov	ecx, 1
	movlps	xmm0, [edi]
	mov	edx, [esi].dwVertexCount
	movaps	xmm1, xmm0
	cvtpi2ps	xmm4, MMWORD PTR [ebx].dib.rectClip.left
	cvtpi2ps	xmm5, MMWORD PTR [ebx].dib.rectClip.right
	;
	.REPEAT
		movlps	xmm2, [edi + ecx * 8]
		inc	ecx
		minps	xmm0, xmm2
		maxps	xmm1, xmm2
	.UNTIL	ecx >= edx
	ASSUME	edi:NOTHING
	;
	cmpps	xmm4, xmm1, 5
	cmpps	xmm5, xmm0, 1
	orps	xmm4, xmm5
	movmskps	eax, xmm4
	.IF	eax & 03H
		xor	eax, eax
		ret
	.ENDIF
	;
	;	各頂点ごとのシェーディングを実施する
	; --------------------------------------------------------------------
	.IF	!([esi].dwShadingFlags & E3DSAF_PHONG_SHADE)
		INVOKE	eglRenderPoly@ShadeVectorsSSE ,
				ebx, [esi].pAttr,
				[esi].pNormals, [esi].pVertexes,
				[esi].pVertexColors,
				[esi].dwVertexCount, NULL
		emms
	.ENDIF

	mov	eax, esi
	ret

eglRenderPoly@MakeUpPolygonSSE	ENDP

;
;	属性適用 486 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@ApplyAttribute486	PROC	NEAR32 C USES ebx esi edi,
	hRenderPoly:HEGL_RENDER_POLYGON,
	pPolyEntry:PTR E3D_POLYGON_ENTRY,
	pColor:PCE3D_COLOR, nTransparency:DWORD

	LOCAL	nAddBlue:DWORD, nAddGreen:DWORD, nAddRed:DWORD
	LOCAL	nAlphaBlue:DWORD, nAlphaGreen:DWORD, nAlphaRed:DWORD

	mov	esi, pPolyEntry
	ASSUME	esi:PTR E3D_POLYGON_ENTRY

	;
	;	透明度適用
	; --------------------------------------------------------------------
	mov	eax, 100H
	mov	edx, 100H
	sub	eax, [esi].dwTransparency
	.IF	CARRY?
		xor	eax, eax
	.ENDIF
	sub	edx, nTransparency
	.IF	CARRY?
		xor	edx, edx
	.ENDIF
	imul	eax, edx
	add	eax, 80H
	mov	edx, 100H
	shr	eax, 8
	sub	edx, eax
	mov	[esi].dwTransparency, edx

	mov	eax, [esi].dwTypeFlag
	.IF	eax == E3D_IMAGE_PRIMITIVE
		;
		; 通常画像の場合には透明度のみ
		;
		xor	eax, eax
		ret
	.ENDIF

	;
	;	色適用
	; --------------------------------------------------------------------
	mov	edi, pColor
	mov	ecx, [esi].dwProjectedCount
	.IF	(edi != NULL) && (ecx != 0)
		ASSUME	edi:PCE3D_COLOR
		movzx	eax, [edi].rgbMul.rgb.Blue
		movzx	ebx, [edi].rgbMul.rgb.Green
		movzx	edx, [edi].rgbMul.rgb.Red
		inc	eax
		inc	ebx
		inc	edx
		mov	nAlphaBlue, eax
		mov	nAlphaGreen, ebx
		mov	nAlphaRed, edx
		movzx	eax, [edi].rgbAdd.rgb.Blue
		movzx	ebx, [edi].rgbAdd.rgb.Green
		movzx	edx, [edi].rgbAdd.rgb.Red
		mov	nAddBlue, eax
		mov	nAddGreen, ebx
		mov	nAddRed, edx
		;
		mov	edi, [esi].pVertexColors
		.REPEAT
			FOR	@MEMBER, <Blue, Green, Red>
				movzx	edx, [edi].rgbAdd.rgb.@MEMBER
				movzx	eax, [edi].rgbMul.rgb.@MEMBER
				shl	edx, 24
				mov	ebx, @CatStr(<nAlpha>,@MEMBER)
				or	eax, edx
				mul	ebx
				mov	ebx, @CatStr(<nAdd>,@MEMBER)
				add	dl, bl
				sbb	dh, dh
;				mov	[edi].rgbMul.rgb.@MEMBER, ah
				or	dl, dh
				mov	[edi].rgbAdd.rgb.@MEMBER, dl
			ENDM
			;
			add	edi, (SIZEOF E3D_COLOR)
			dec	ecx
		.UNTIL	ZERO?
		ASSUME	edi:NOTHING
	.ENDIF

	ASSUME	esi:NOTHING
	xor	eax, eax
	ret

eglRenderPoly@ApplyAttribute486	ENDP

;
;	属性適用 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@ApplyAttributeSSE	PROC	NEAR32 C USES ebx esi edi,
	hRenderPoly:HEGL_RENDER_POLYGON,
	pPolyEntry:PTR E3D_POLYGON_ENTRY,
	pColor:PCE3D_COLOR, nTransparency:DWORD

	mov	esi, pPolyEntry
	ASSUME	esi:PTR E3D_POLYGON_ENTRY

	;
	;	透明度適用
	; --------------------------------------------------------------------
	mov	eax, 100H
	mov	edx, 100H
	xor	ecx, ecx
	sub	eax, [esi].dwTransparency
	cmovc	eax, ecx
	sub	edx, nTransparency
	cmovc	edx, ecx
	imul	eax, edx
	add	eax, 80H
	mov	edx, 100H
	shr	eax, 8
	sub	edx, eax
	mov	[esi].dwTransparency, edx

	mov	eax, [esi].dwTypeFlag
	.IF	eax == E3D_IMAGE_PRIMITIVE
		;
		; 通常画像の場合には透明度のみ
		;
		xor	eax, eax
		ret
	.ENDIF

	;
	;	色適用
	; --------------------------------------------------------------------
	mov	edi, pColor
	mov	ecx, [esi].dwProjectedCount
	.IF	(edi != NULL) && (ecx != 0)
		ASSUME	edi:PCE3D_COLOR
		pxor	mm0, mm0
		pcmpeqb	mm1, mm1
		movd	mm6, [edi].rgbAdd.dwPixelCode
		movd	mm7, [edi].rgbMul.dwPixelCode
		punpcklbw	mm6, mm0
		psrlw	mm1, 15
		punpcklbw	mm7, mm0
		paddusw	mm7, mm1
		movq	mm5, mmx0100HPW
		.IF	eax & E3D_TEXTURE_POLYGON
			movq	mm5, mm7
		.ENDIF
		;
		mov	edi, [esi].pVertexColors
		.REPEAT
			movd	mm3, [edi].rgbAdd.dwPixelCode
			movd	mm2, [edi].rgbMul.dwPixelCode
			punpcklbw	mm3, mm0
			punpcklbw	mm2, mm0
			pmullw	mm3, mm7
			pmullw	mm2, mm5
			add	edi, (SIZEOF E3D_COLOR)
			psrlw	mm3, 8
			psrlw	mm2, 8
			paddusw	mm3, mm6
			dec	ecx
			packuswb	mm2, mm3
			movq	MMWORD PTR [edi - 8], mm2
			;
		.UNTIL	ZERO?
		ASSUME	edi:NOTHING
		;
		emms
	.ENDIF

	ASSUME	esi:NOTHING
	xor	eax, eax
	ret

eglRenderPoly@ApplyAttributeSSE	ENDP

;
;	外接最小矩形取得 486 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@GetExternalRect486	PROC	NEAR32 C USES ebx esi edi,
	hRenderPoly:HEGL_RENDER_POLYGON,
	pExtRect:PTR EGL_RECT,
	pPolyEntry:PTR PCE3D_POLYGON_ENTRY, nCount:DWORD

	LOCAL	vMeshMinMax[4]:E3D_VECTOR_2D
	LOCAL	rTemp[4]:REAL4
	LOCAL	rMinX:REAL4, rMaxX:REAL4
	LOCAL	rMinY:REAL4, rMaxY:REAL4

	mov	ebx, hRenderPoly
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	fild	[ebx].dib.rectClip.right
	fstp	rMinX
	fild	[ebx].dib.rectClip.left
	fstp	rMaxX
	fild	[ebx].dib.rectClip.bottom
	fstp	rMinY
	fild	[ebx].dib.rectClip.top
	fstp	rMaxY
	FOR	@DUMMY, <rMinX, rMaxX, rMinY, rMaxY>
		mov	edx, @DUMMY
		mov	eax, edx
		sar	edx, 31
		shr	edx, 1
		xor	eax, edx
		mov	@DUMMY, eax
	ENDM
	ASSUME	ebx:NOTHING

	mov	ecx, nCount
	mov	esi, pPolyEntry
	test	ecx, ecx
	.WHILE	!ZERO?
		mov	nCount, ecx
		mov	pPolyEntry, esi
		;
		mov	esi, PCE3D_POLYGON_ENTRY PTR [esi]
		ASSUME	esi:PCE3D_POLYGON_ENTRY
		.IF	[esi].dwTypeFlag & E3D_MESH_POLYGON
		; ------------------------------------------------------------
		;	メッシュ
		; ------------------------------------------------------------
		mov	ebx, hRenderPoly
		ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
		fld	[ebx].vScreenPos.z
		fdiv	[esi].surface.mesh.vMinMesh.z
		fld	[esi].surface.mesh.vMinMesh.x
		fmul	st, st(1)
		fadd	[ebx].vScreenPos.x
		fstp	vMeshMinMax[0].x
		fld	[esi].surface.mesh.vMinMesh.y
		fmul	st, st(1)
		fadd	[ebx].vScreenPos.y
		fstp	vMeshMinMax[0].y
		fld	[esi].surface.mesh.vMaxMesh.x
		fmul	st, st(1)
		fadd	[ebx].vScreenPos.x
		fstp	vMeshMinMax[8].x
		fld	[esi].surface.mesh.vMaxMesh.y
		fmulp	st(1), st
		fadd	[ebx].vScreenPos.y
		fstp	vMeshMinMax[8].y
		;
		fld	[ebx].vScreenPos.z
		fdiv	[esi].surface.mesh.vMaxMesh.z
		fld	[esi].surface.mesh.vMinMesh.x
		fmul	st, st(1)
		fadd	[ebx].vScreenPos.x
		fstp	vMeshMinMax[10H].x
		fld	[esi].surface.mesh.vMinMesh.y
		fmul	st, st(1)
		fadd	[ebx].vScreenPos.y
		fstp	vMeshMinMax[10H].y
		fld	[esi].surface.mesh.vMaxMesh.x
		fmul	st, st(1)
		fadd	[ebx].vScreenPos.x
		fstp	vMeshMinMax[18H].x
		fld	[esi].surface.mesh.vMaxMesh.y
		fmulp	st(1), st
		fadd	[ebx].vScreenPos.y
		fstp	vMeshMinMax[18H].y
		;
		xor	ecx, ecx
		.REPEAT
			mov	eax, vMeshMinMax[ecx*8].x
			mov	edi, eax
			sar	eax, 31
				mov	edx, vMeshMinMax[ecx*8].y
			shr	eax, 1
			xor	eax, edi
				mov	edi, edx
				sar	edx, 31
				shr	edx, 1
				xor	edx, edi
			;
			.IF	(SDWORD PTR eax) < (SDWORD PTR rMinX)
				mov	rMinX, eax
			.ENDIF
			.IF	(SDWORD PTR eax) > (SDWORD PTR rMaxX)
				mov	rMaxX, eax
			.ENDIF
			.IF	(SDWORD PTR edx) < (SDWORD PTR rMinY)
				mov	rMinY, edx
			.ENDIF
			.IF	(SDWORD PTR edx) > (SDWORD PTR rMaxY)
				mov	rMaxY, edx
			.ENDIF
			;
			inc	ecx
		.UNTIL	ecx >= 4
		;
		ASSUME	ebx:NOTHING


		.ELSEIF	[esi].dwTypeFlag == E3D_IMAGE_PRIMITIVE
		; ------------------------------------------------------------
		;	画像オブジェクト
		; ------------------------------------------------------------
		;
		; 画像サイズ取得
		;
		fld	[esi].surface.image.vImageBase.y
		fmul	[esi].surface.image.vEnlarge.y
		fld	[esi].surface.image.vImageBase.x
		fmul	[esi].surface.image.vEnlarge.x
		;
		; 座標計算
		;
		fld	[esi].surface.image.vCenter.x
		fsub	st, st(1)
		fstp	rTemp[0]
		fld	[esi].surface.image.vCenter.x
		fadd	st, st(1)
		fstp	rTemp[4]
		;
		fld	[esi].surface.image.vCenter.y
		fsub	st, st(2)
		fstp	rTemp[8]
		fld	[esi].surface.image.vCenter.y
		fadd	st, st(2)
		fstp	rTemp[12]
		;
		fstp	st(0)
		fstp	st(0)
		;
		; 座標比較
		;
		mov	eax, rTemp[0]
		mov	edx, rTemp[4]
		test	eax, eax
		.IF	SIGN?
			xor	eax, 7FFFFFFFH
		.ENDIF
		test	edx, edx
		.IF	SIGN?
			xor	eax, 7FFFFFFFH
		.ENDIF
		.IF	(SDWORD PTR eax) < (SDWORD PTR rMinX)
			mov	rMinX, eax
		.ENDIF
		.IF	(SDWORD PTR edx) > (SDWORD PTR rMaxX)
			mov	rMaxX, edx
		.ENDIF
		mov	eax, rTemp[8]
		mov	edx, rTemp[12]
		test	eax, eax
		.IF	SIGN?
			xor	eax, 7FFFFFFFH
		.ENDIF
		test	edx, edx
		.IF	SIGN?
			xor	eax, 7FFFFFFFH
		.ENDIF
		.IF	(SDWORD PTR eax) < (SDWORD PTR rMinY)
			mov	rMinY, eax
		.ENDIF
		.IF	(SDWORD PTR edx) > (SDWORD PTR rMaxY)
			mov	rMaxY, edx
		.ENDIF

		.ELSE
		; ------------------------------------------------------------
		;	ポリゴンオブジェクト
		; ------------------------------------------------------------
		;
		; ｘ座標チェック
		;
		mov	ecx, [esi].dwProjectedCount
		mov	edi, [esi].pProjVertexes
		mov	ebx, rMinX
		mov	edx, rMaxX
		ASSERT	<ecx !>= 3>, "[esi].dwProjectedCount >= 3"
		ASSUME	edi:PE3D_VECTOR_2D
		.REPEAT
			mov	eax, [edi].x
			add	edi, (SIZEOF E3D_VECTOR_2D)
			test	eax, eax
			.IF	SIGN?
				xor	eax, 7FFFFFFFH
			.ENDIF
			.IF	(SDWORD PTR eax) < (SDWORD PTR ebx)
				mov	ebx, eax
			.ENDIF
			.IF	(SDWORD PTR eax) > (SDWORD PTR edx)
				mov	edx, eax
			.ENDIF
			dec	ecx
		.UNTIL	ZERO?
		mov	rMinX, ebx
		mov	rMaxX, edx
		;
		; ｙ座標チェック
		;
		mov	ecx, [esi].dwProjectedCount
		mov	edi, [esi].pProjVertexes
		mov	ebx, rMinY
		mov	edx, rMaxY
		ASSUME	edi:PE3D_VECTOR_2D
		.REPEAT
			mov	eax, [edi].y
			add	edi, (SIZEOF E3D_VECTOR_2D)
			test	eax, eax
			.IF	SIGN?
				xor	eax, 7FFFFFFFH
			.ENDIF
			.IF	(SDWORD PTR eax) < (SDWORD PTR ebx)
				mov	ebx, eax
			.ENDIF
			.IF	(SDWORD PTR eax) > (SDWORD PTR edx)
				mov	edx, eax
			.ENDIF
			dec	ecx
		.UNTIL	ZERO?
		mov	rMinY, ebx
		mov	rMaxY, edx
		ASSUME	edi:NOTHING

		.ENDIF
		ASSUME	esi:NOTHING
		;
		mov	esi, pPolyEntry
		mov	ecx, nCount
		add	esi, (SIZEOF PCE3D_POLYGON_ENTRY)
		dec	ecx
	.ENDW

	;
	; 矩形に変換
	;
	FOR	@DUMMY, <rMinX, rMaxX, rMinY, rMaxY>
		mov	edx, @DUMMY
		mov	eax, edx
		sar	edx, 31
		shr	edx,1
		xor	eax, edx
		mov	@DUMMY, eax
	ENDM
	mov	edi, pExtRect
	ASSUME	edi:PTR EGL_RECT
	fld1
	fadd	st, st(0)
	fld	rMinX
	fsub	st, st(1)
	fistp	[edi].left
	fld	rMaxX
	fadd	st, st(1)
	fistp	[edi].right
	fld	rMinY
	fsub	st, st(1)
	fistp	[edi].top
	fld	rMaxY
	fadd	st, st(1)
	fistp	[edi].bottom
	fstp	st(0)
	ASSUME	edi:NOTHING
	;
	xor	eax, eax
	ret

eglRenderPoly@GetExternalRect486	ENDP

;
;	外接最小矩形取得 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@GetExternalRectSSE	PROC	NEAR32 C USES ebx esi edi,
	hRenderPoly:HEGL_RENDER_POLYGON,
	pExtRect:PTR EGL_RECT,
	pPolyEntry:PTR PCE3D_POLYGON_ENTRY, nCount:DWORD

	mov	ebx, hRenderPoly
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	xorps	xmm0, xmm0
	xorps	xmm1, xmm1
	cvtpi2ps	xmm0, MMWORD PTR [ebx].dib.rectClip.right
	cvtpi2ps	xmm1, MMWORD PTR [ebx].dib.rectClip.left
	ASSUME	ebx:NOTHING

	mov	ecx, nCount
	mov	esi, pPolyEntry
	test	ecx, ecx
	.WHILE	!ZERO?
		mov	nCount, ecx
		mov	pPolyEntry, esi
		;
		mov	esi, PCE3D_POLYGON_ENTRY PTR [esi]
		ASSUME	esi:PCE3D_POLYGON_ENTRY
		.IF	[esi].dwTypeFlag & E3D_MESH_POLYGON
		; ------------------------------------------------------------
		;	メッシュ
		; ------------------------------------------------------------
		mov	ebx, hRenderPoly
		ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
		movlps	xmm4, QWORD PTR [esi].surface.mesh.vMinMesh
		movss	xmm5, [ebx].vScreenPos.z
		movss	xmm6, [esi].surface.mesh.vMinMesh.z
		movss	xmm7, [esi].surface.mesh.vMaxMesh.z
		movlhps	xmm4, xmm4
		shufps	xmm5, xmm5, 0
		shufps	xmm6, xmm7, 0
		mulps	xmm4, xmm5
		divps	xmm4, xmm6
		movlps	xmm7, QWORD PTR [ebx].vScreenPos
		movlhps	xmm7, xmm7
		addps	xmm4, xmm7
		minps	xmm0, xmm4
		maxps	xmm1, xmm4
		movhlps	xmm4, xmm4
		minps	xmm0, xmm4
		maxps	xmm1, xmm4
		;
		movlps	xmm4, QWORD PTR [esi].surface.mesh.vMaxMesh
		movss	xmm5, [ebx].vScreenPos.z
		movss	xmm6, [esi].surface.mesh.vMinMesh.z
		movss	xmm7, [esi].surface.mesh.vMaxMesh.z
		movlhps	xmm4, xmm4
		shufps	xmm5, xmm5, 0
		shufps	xmm6, xmm7, 0
		mulps	xmm4, xmm5
		divps	xmm4, xmm6
		movlps	xmm7, QWORD PTR [ebx].vScreenPos
		movlhps	xmm7, xmm7
		addps	xmm4, xmm7
		minps	xmm0, xmm4
		maxps	xmm1, xmm4
		movhlps	xmm4, xmm4
		minps	xmm0, xmm4
		maxps	xmm1, xmm4
		;
		ASSUME	ebx:NOTHING


		.ELSEIF	[esi].dwTypeFlag == E3D_IMAGE_PRIMITIVE
		; ------------------------------------------------------------
		;	画像オブジェクト
		; ------------------------------------------------------------
		;
		; 画像サイズ取得
		;
		movlps	xmm2, QWORD PTR [esi].surface.image.vImageBase
		movlps	xmm3, QWORD PTR [esi].surface.image.vEnlarge
		mulps	xmm2, xmm3
		;
		; 座標計算
		;
		movups	xmm4, XMMWORD_PTR [esi].surface.image.vCenter
		movaps	xmm5, xmm4
		subps	xmm4, xmm2
		addps	xmm5, xmm2
		;
		; 座標比較
		;
		minps	xmm0, xmm4
		maxps	xmm1, xmm5

		.ELSE
		; ------------------------------------------------------------
		;	ポリゴンオブジェクト
		; ------------------------------------------------------------
		;
		; 座標チェック
		;
		mov	ecx, [esi].dwProjectedCount
		mov	edi, [esi].pProjVertexes
		ASSERT	<ecx !>= 3>, "[esi].dwProjectedCount >= 3"
		ASSUME	edi:PE3D_VECTOR_2D
		xorps	xmm2, xmm2
		.REPEAT
			movlps	xmm2, [edi]
			add	edi, (SIZEOF E3D_VECTOR_2D)
			minps	xmm0, xmm2
			maxps	xmm1, xmm2
			dec	ecx
		.UNTIL	ZERO?
		ASSUME	edi:NOTHING

		.ENDIF
		ASSUME	esi:NOTHING
		;
		mov	esi, pPolyEntry
		mov	ecx, nCount
		add	esi, (SIZEOF PCE3D_POLYGON_ENTRY)
		dec	ecx
	.ENDW

	;
	; 矩形に変換
	;
	movups	xmm2, xmmConst2
	mov	edi, pExtRect
	ASSUME	edi:PTR EGL_RECT
	subps	xmm0, xmm2
	addps	xmm1, xmm2
	cvtss2si	eax, xmm0
	shufps	xmm0, xmm0, 1
	cvtss2si	edx, xmm1
	shufps	xmm1, xmm1, 1
	mov	[edi].left, eax
	mov	[edi].right, edx
	cvtss2si	eax, xmm0
	cvtss2si	edx, xmm1
	mov	[edi].top, eax
	mov	[edi].bottom, edx
	ASSUME	edi:NOTHING
	;
	xor	eax, eax
	ret

eglRenderPoly@GetExternalRectSSE	ENDP


CodeSeg	ENDS

	END
