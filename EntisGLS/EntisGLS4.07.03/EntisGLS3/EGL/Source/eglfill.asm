
; ----------------------------------------------------------------------------
;                          Entis Graphic Library
; ----------------------------------------------------------------------------
;   Copyright (c) 2002-2004 Leshade Entis, Entis-soft. Al rights reserved.
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
realConstHalf	REAL8	0.5

mmxPWMask0111	LABEL	MMWORD
	WORD	0FFFFH, 0FFFFH, 0FFFFH, 0000H
mmxPWMask1000	LABEL	MMWORD
	WORD	0, 0, 0, 0FFH
mmxPWMask0FFH	LABEL	MMWORD
	WORD	0FFH, 0FFH, 0FFH, 0FFH
mmxPW1		LABEL	MMWORD
	WORD	1, 1, 1, 1
mmxMaskDWHB	LABEL	MMWORD
	DWORD	0FF000000H, 0FF000000H

ConstSeg	ENDS


; ----------------------------------------------------------------------------
;	コードセグメント
; ----------------------------------------------------------------------------

CodeSeg	SEGMENT	PARA READONLY FLAT 'CODE'

eglDrawImage@DrawRegion_Lines		PROTO	NEAR32 C
eglDrawImage@FillLine_486		PROTO	NEAR32 C
eglDrawImage@FillLine_Blend_486		PROTO	NEAR32 C
eglDrawImage@FillLine_Inversion_486	PROTO	NEAR32


;
;	フィル関数振り分け
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@SetFillFunc	PROC	NEAR32 C PRIVATE
	LOCAL	irectClip:EGL_IMAGE_RECT

	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF

	mov	edi, [ebx].pDstImage
	mov	irectClip.x, 0
	mov	irectClip.y, 0
	ASSUME	edi:PEGL_IMAGE_INFO
	mov	ecx, [edi].dwImageWidth
	mov	edx, [edi].dwImageHeight
	mov	irectClip.w, ecx
	mov	irectClip.h, edx
	ASSUME	edi:NOTHING
	;
	INVOKE	eglGetClippedImageInfo ,
			ADDR [ebx].dstimg, edi, ADDR irectClip
	.IF	eax != eslErrSuccess
		mov	[ebx].pfnFillRegion, \
				OFFSET eglDrawImage@FillRegion_NoDraw
		ret
	.ENDIF
	;
	mov	eax, [ebx].dstimg.dwBitsPerPixel
	.IF	[ebx].dwFlags & EGL_DRAW_BLEND_ALPHA
		.IF	(eax != 24) && (eax != 32)
			and	[ebx].dwFlags, NOT EGL_DRAW_BLEND_ALPHA
		.ENDIF
	.ENDIF
	.IF	[ebx].dwFlags & EGL_DRAW_BLEND_ALPHA
		;
		; 透明度適用
		;
		mov	eax, 100H
		sub	eax, [ebx].nTrans
		.IF	SIGN?
			xor	eax, eax
		.ELSEIF	eax > 100H
			mov	eax, 100H
		.ENDIF
		;
		movzx	ecx, [ebx].colorDraw.rgba.Blue
		movzx	edx, [ebx].colorDraw.rgba.Green
		imul	ecx, eax
		imul	edx, eax
		shr	ecx, 8
		shr	edx, 8
		mov	[ebx].dwTemp[0], ecx
		mov	[ebx].dwTemp[4], edx
		;
		movzx	ecx, [ebx].colorDraw.rgba.Red
		movzx	edx, [ebx].colorDraw.rgba.Alpha
		imul	ecx, eax
		imul	edx, eax
		shr	ecx, 8
		shr	edx, 8
		mov	[ebx].dwTemp[8], ecx
		mov	[ebx].dwTemp[12], edx
		;
		xor	edx, 0FFH
		xor	eax, eax
		inc	edx
		xor	ecx, ecx
		.REPEAT
			mov	BYTE PTR [ebx].nBlueTone[ecx], ah
			inc	ecx
			add	eax, edx
		.UNTIL	ecx >= 100H
		;
		.IF	([ebx].dstimg.dwBitsPerPixel == 32) && \
			(ERI_EnabledProcessorType & ERI_USE_MMX_PENTIUM)
			mov	eax, OFFSET eglDrawImage@FillRegion_Blend_MMX
		.ELSE
			mov	eax, OFFSET eglDrawImage@FillRegion_Blend_486
		.ENDIF
		mov	edx, OFFSET eglDrawImage@DrawRegion_Blend_486
		mov	ecx, OFFSET eglDrawImage@FillLine_Blend_486

	.ELSEIF	[ebx].dwFlags & EGL_FILL_INVERSION
		FOR	@COLOR, <Blue, Green, Red>
			movzx	eax, [ebx].colorDraw.rgba.@COLOR
			mov	ecx, eax
			shr	eax, 7
			add	ecx, eax
			INVOKE	eglCalculateToneTable ,
				ADDR @CatStr( <[ebx].n>, @COLOR, <Tone> ),
					ecx, EGL_TONE_INVERSION
		ENDM
		mov	eax, OFFSET eglDrawImage@FillRegion_Inversion_486
		mov	edx, OFFSET eglDrawImage@DrawRegion_Inversion_486
		mov	ecx, OFFSET eglDrawImage@FillLine_Inversion_486

	.ELSE
		mov	eax, OFFSET eglDrawImage@FillRegion_486
		mov	edx, OFFSET eglDrawImage@DrawRegion_486
		mov	ecx, OFFSET eglDrawImage@FillLine_486
	.ENDIF
	mov	[ebx].pfnFillRegion, eax
	mov	[ebx].pfnDrawRegion, edx
	mov	[ebx].pfnDrawRegionLine, ecx

	ASSUME	ebx:NOTHING
	ret

eglDrawImage@SetFillFunc	ENDP

;
;	直線の描画準備
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@PrepareLine		PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE,
	x1:SDWORD, y1:SDWORD, x2:SDWORD, y2:SDWORD,
	colorDraw:EGL_PALETTE, nTransparency:DWORD, dwFlags:DWORD

	LOCAL	xDelta:REAL8
	LOCAL	xCurrentPos:SDWORD

	;
	; パラメータ取得
	;
	mov	ebx, hDrawImage
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF

	mov	eax, colorDraw.dwPixelCode
	mov	ecx, nTransparency
	mov	edx, dwFlags
	mov	[ebx].pfnFillRegion, OFFSET eglDrawImage@FillRegion_NoDraw
	mov	[ebx].pfnDrawRegion, OFFSET eglDrawImage@FillRegion_NoDraw
	mov	[ebx].colorDraw.dwPixelCode, eax
	mov	[ebx].nTrans, ecx
	mov	[ebx].dwFlags, edx
	;
	; リージョン作成
	;
	mov	eax, y1
	mov	edx, y2
	mov	ecx, [ebx].ptDrawOffset.y
	add	eax, ecx
	add	edx, ecx
	mov	ecx, [ebx].ptDrawOffset.x
	.IF	(SDWORD PTR eax) > (SDWORD PTR edx)
		mov	y1, edx
		mov	y2, eax
		mov	eax, x1
		mov	edx, x2
		add	eax, ecx
		add	edx, ecx
		mov	x1, edx
		mov	x2, eax
	.ELSE
		mov	y1, eax
		mov	y2, edx
		mov	eax, x1
		mov	edx, x2
		add	eax, ecx
		add	edx, ecx
		mov	x1, eax
		mov	x2, edx
	.ENDIF
	;
	mov	eax, y1
	mov	edx, y2
	.IF	((SDWORD PTR eax) > [ebx].rectClip.bottom) || \
			((SDWORD PTR edx) < [ebx].rectClip.top)
		xor	eax, eax
		ret
	.ENDIF
	;
	fild	x2
	fisub	x1
	fild	y2
	fisub	y1
	fld1
	faddp	st(1), st
	fdivp	st(1), st
	fstp	xDelta
	;
	.IF	(SDWORD PTR eax) < [ebx].rectClip.top
		fild	[ebx].rectClip.top
		fisub	y1
		mov	eax, [ebx].rectClip.top
		fmul	xDelta
		mov	y1, eax
		fiadd	x1
		fistp	x1
	.ENDIF
	.IF	(SDWORD PTR edx) > [ebx].rectClip.bottom
		fild	y2
		fisub	y1
		fild	[ebx].rectClip.bottom
		fisub	y1
		mov	eax, [ebx].rectClip.bottom
		fld1
		faddp	st(1), st
		mov	y2, eax
		fmul	xDelta
		fiadd	x1
		fistp	x2
	.ENDIF
	;
	mov	esi, [ebx].pRegion
	ASSUME	esi:PTR E3D_POLYGON_REGION
	mov	[esi].nTopLine, 80000000H
	lea	edi, [esi].plrLineRgn[0]
	ASSUME	edi:PTR E3D_POLY_LINE_REGION
	;
	fld	xDelta
	fild	x1
	;
	mov	ecx, y1
	mov	eax, x1
	.REPEAT
		fadd	st, st(1)
		fist	xCurrentPos
		mov	edx, xCurrentPos
		;
		.IF	ecx != y1
			.IF	(SDWORD PTR eax) > (SDWORD PTR edx)
				dec	eax
				xchg	eax, edx
			.ELSEIF	(SDWORD PTR eax) < (SDWORD PTR edx)
				inc	eax
			.ENDIF
		.ELSEIF	(SDWORD PTR eax) > (SDWORD PTR edx)
			xchg	eax, edx
		.ENDIF
		;
		.IF	(((SDWORD PTR eax) <= [ebx].rectClip.right) && \
				((SDWORD PTR edx) >= [ebx].rectClip.left)) || \
			(((SDWORD PTR edx) >= [ebx].rectClip.left) && \
				((SDWORD PTR eax) <= [ebx].rectClip.right))
			.IF	[esi].nTopLine == 80000000H
				mov	[esi].nTopLine, ecx
				mov	[esi].nBottomLine, ecx
			.ELSE
				mov	[esi].nBottomLine, ecx
			.ENDIF
			.IF	(SDWORD PTR eax) < [ebx].rectClip.left
				mov	eax, [ebx].rectClip.left
			.ENDIF
			.IF	(SDWORD PTR edx) > [ebx].rectClip.right
				mov	edx, [ebx].rectClip.right
			.ENDIF
			mov	[edi].nLeft, eax
			mov	[edi].nRight, edx
			add	edi, (SIZEOF E3D_POLY_LINE_REGION)
		.ELSE
			.BREAK	.IF	[esi].nTopLine != 80000000H
		.ENDIF
		;
		inc	ecx
		mov	eax, xCurrentPos
	.UNTIL	ecx > y2
	;
	fstp	st(0)
	fstp	st(0)
	;
	.IF	[esi].nTopLine == 80000000H
		xor	eax, eax
		ret
	.ENDIF
	;
	ASSUME	esi:NOTHING
	ASSUME	edi:NOTHING

	INVOKE	eglDrawImage@SetFillFunc

	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglDrawImage@PrepareLine		ENDP

;
;	矩形の描画準備
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@PrepareFillRect		PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE,
	pDrawRect:PCEGL_RECT, colorDraw:EGL_PALETTE,
	nTransparency:DWORD, dwFlags:DWORD

	LOCAL	vxRect[4]:E3D_VECTOR_2D
	LOCAL	rectFill:EGL_RECT

	;
	; パラメータ取得
	;
	mov	ebx, hDrawImage
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF

	mov	eax, colorDraw.dwPixelCode
	mov	ecx, nTransparency
	mov	edx, dwFlags
	mov	[ebx].pfnFillRegion, OFFSET eglDrawImage@FillRegion_NoDraw
	mov	[ebx].pfnDrawRegion, OFFSET eglDrawImage@FillRegion_NoDraw
	mov	[ebx].colorDraw.dwPixelCode, eax
	mov	[ebx].nTrans, ecx
	mov	[ebx].dwFlags, edx
	;
	; リージョン作成
	;
	mov	esi, pDrawRect
	.IF	esi == 0
		xor	eax, eax
		ret
	.ENDIF
	mov	eax, [ebx].rectClip.left
	.IF	(SDWORD PTR eax) > [ebx].rectClip.right
		xor	eax, eax
		ret
	.ENDIF
	mov	eax, [ebx].rectClip.top
	.IF	(SDWORD PTR eax) > [ebx].rectClip.bottom
		xor	eax, eax
		ret
	.ENDIF
	mov	edi, [ebx].pRegion
	;
	ASSUME	esi:PCEGL_RECT
	ASSUME	edi:PTR E3D_POLYGON_REGION
	mov	eax, [esi].left
	mov	edx, [esi].right
	mov	ecx, [ebx].ptDrawOffset.x
	add	eax, ecx
	add	edx, ecx
	.IF	(SDWORD PTR eax) > (SDWORD PTR edx)
		xchg	eax, edx
	.ENDIF
	.IF	((SDWORD PTR eax) > [ebx].rectClip.right) || \
			((SDWORD PTR edx) < [ebx].rectClip.left)
		xor	eax, eax
		ret
	.ENDIF
	.IF	(SDWORD PTR eax) < [ebx].rectClip.left
		mov	eax, [ebx].rectClip.left
	.ENDIF
	.IF	(SDWORD PTR edx) > [ebx].rectClip.right
		mov	edx, [ebx].rectClip.right
	.ENDIF
	mov	rectFill.left, eax
	mov	rectFill.right, edx
	;
	mov	eax, [esi].top
	mov	edx, [esi].bottom
	mov	ecx, [ebx].ptDrawOffset.y
	add	eax, ecx
	add	edx, ecx
	.IF	(SDWORD PTR eax) > (SDWORD PTR edx)
		xchg	eax, edx
	.ENDIF
	.IF	((SDWORD PTR eax) > [ebx].rectClip.bottom) || \
			((SDWORD PTR edx) < [ebx].rectClip.top)
		xor	eax, eax
		ret
	.ENDIF
	.IF	(SDWORD PTR eax) < [ebx].rectClip.top
		mov	eax, [ebx].rectClip.top
	.ENDIF
	.IF	(SDWORD PTR edx) > [ebx].rectClip.bottom
		mov	edx, [ebx].rectClip.bottom
	.ENDIF
	mov	[edi].nTopLine, eax
	mov	[edi].nBottomLine, edx
	;
	lea	edi, [edi].plrLineRgn[0]
	ASSUME	edi:PTR E3D_POLY_LINE_REGION
	;
	sub	edx, eax
	inc	edx
	mov	eax, rectFill.left
	mov	ecx, rectFill.right
	.REPEAT
		mov	[edi].nLeft, eax
		mov	[edi].nRight, ecx
		add	edi, (SIZEOF E3D_POLY_LINE_REGION)
		dec	edx
	.UNTIL	ZERO?
	;
	ASSUME	edi:NOTHING

	INVOKE	eglDrawImage@SetFillFunc

	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglDrawImage@PrepareFillRect		ENDP

;
;	楕円の描画準備
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@PrepareFillEllipse		PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE,
	pCenter:PCEGL_POINT, pRadius:PCEGL_SIZE,
	colorDraw:EGL_PALETTE, nTransparency:DWORD, dwFlags:DWORD

	LOCAL	ptCenter:EGL_POINT
	LOCAL	rRadiusSqr:REAL8	; = ry * ry
	LOCAL	rXYRate:REAL8		; = abs( rx / ry )
	LOCAL	nXPos:SDWORD
	LOCAL	nTemp:SDWORD
	LOCAL	nYPos:SDWORD
	LOCAL	nBottomLine:SDWORD

	;
	; パラメータ取得
	;
	mov	ebx, hDrawImage
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF

	mov	eax, colorDraw.dwPixelCode
	mov	ecx, nTransparency
	mov	edx, dwFlags
	mov	[ebx].pfnFillRegion, OFFSET eglDrawImage@FillRegion_NoDraw
	mov	[ebx].pfnDrawRegion, OFFSET eglDrawImage@FillRegion_NoDraw
	mov	[ebx].colorDraw.dwPixelCode, eax
	mov	[ebx].nTrans, ecx
	mov	[ebx].dwFlags, edx
	;
	mov	esi, pCenter
	mov	edi, pRadius
	.IF	(esi == NULL) || (edi == NULL)
		xor	eax, eax
		ret
	.ENDIF
	;
	ASSUME	esi:PCEGL_POINT
	ASSUME	edi:PCEGL_SIZE
	mov	ecx, [esi].x
	mov	edx, [esi].y
	add	ecx, [ebx].ptDrawOffset.x
	add	edx, [ebx].ptDrawOffset.y
	mov	ptCenter.x, ecx
	mov	ptCenter.y, edx
	fild	[edi].h
	fmul	st, st(0)
	fstp	rRadiusSqr
	fild	[edi].w
	fidiv	[edi].h
	fabs
	fstp	rXYRate
	;
	; リージョン作成
	;
	mov	esi, [ebx].pRegion
	ASSUME	esi:PTR E3D_POLYGON_REGION
	mov	ecx, ptCenter.y
	mov	eax, [edi].h
	mov	edx, ecx
	test	eax, eax
	.IF	SIGN?
		neg	eax
	.ENDIF
	sub	ecx, eax
	add	edx, eax
	;
	.IF	((SDWORD PTR ecx) > [ebx].rectClip.bottom) || \
			((SDWORD PTR edx) < [ebx].rectClip.top)
		xor	eax, eax
		ret
	.ENDIF
	.IF	(SDWORD PTR ecx) < [ebx].rectClip.top
		mov	ecx, [ebx].rectClip.top
	.ENDIF
	.IF	(SDWORD PTR edx) > [ebx].rectClip.bottom
		mov	edx, [ebx].rectClip.bottom
	.ENDIF
	;
	mov	nYPos, ecx
	mov	[esi].nTopLine, 80000000H
	mov	nBottomLine, edx
	mov	[esi].nBottomLine, 80000000H
	;
	lea	edi, [esi].plrLineRgn[0]
	ASSUME	edi:PTR E3D_POLY_LINE_REGION
	;
	.REPEAT
		mov	nYPos, ecx
		;
		fld	rRadiusSqr
		;
		sub	ecx, ptCenter.y
		mov	[ebx].dwTemp[0], ecx
		fild	[ebx].dwTemp[0]
		.IF	!ZERO?
		.IF	SIGN?
			fadd	realConstHalf
		.ELSE
			fsub	realConstHalf
		.ENDIF
		.ENDIF
		;
		fmul	st, st(0)
		fsubp	st(1), st
		fsqrt
		fmul	rXYRate
		fistp	nXPos
		;
		mov	ecx, ptCenter.x
		mov	eax, nXPos
		mov	edx, ecx
		sub	ecx, eax
		add	edx, eax
		.IF	(SDWORD PTR ecx) > (SDWORD PTR edx)
			xchg	ecx, edx
		.ENDIF
		;
		.IF	(((SDWORD PTR ecx) <= [ebx].rectClip.right) && \
				((SDWORD PTR edx) >= [ebx].rectClip.left)) || \
			(((SDWORD PTR edx) >= [ebx].rectClip.left) && \
				((SDWORD PTR ecx) <= [ebx].rectClip.right))
			mov	eax, nYPos
			.IF	[esi].nTopLine == 80000000H
				mov	[esi].nTopLine, eax
				mov	[esi].nBottomLine, eax
			.ELSE
				mov	[esi].nBottomLine, eax
			.ENDIF
			.IF	(SDWORD PTR ecx) < [ebx].rectClip.left
				mov	ecx, [ebx].rectClip.left
			.ENDIF
			.IF	(SDWORD PTR edx) > [ebx].rectClip.right
				mov	edx, [ebx].rectClip.right
			.ENDIF
			mov	[edi].nLeft, ecx
			mov	[edi].nRight, edx
			add	edi, (SIZEOF E3D_POLY_LINE_REGION)
		.ELSE
			.BREAK	.IF	[esi].nTopLine != 80000000H
		.ENDIF
		;
		mov	ecx, nYPos
		inc	ecx
	.UNTIL	ecx > nBottomLine
	;
	.IF	[esi].nTopLine == 80000000H
		xor	eax, eax
		ret
	.ENDIF
	;
	ASSUME	esi:NOTHING
	ASSUME	edi:NOTHING

	INVOKE	eglDrawImage@SetFillFunc

	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglDrawImage@PrepareFillEllipse		ENDP

;
;	多角形の描画準備
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@PrepareFillPolygon		PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE,
	pVertexes:PCEGL_POINT, nCount:DWORD,
	colorDraw:EGL_PALETTE, nTransparency:DWORD, dwFlags:DWORD

	LOCAL	pvxPoly:PE3D_VECTOR_2D

	;
	; パラメータ取得
	;
	mov	ebx, hDrawImage
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF

	mov	eax, colorDraw.dwPixelCode
	mov	ecx, nTransparency
	mov	edx, dwFlags
	mov	[ebx].pfnFillRegion, OFFSET eglDrawImage@FillRegion_NoDraw
	mov	[ebx].pfnDrawRegion, OFFSET eglDrawImage@FillRegion_NoDraw
	mov	[ebx].colorDraw.dwPixelCode, eax
	mov	[ebx].nTrans, ecx
	mov	[ebx].dwFlags, edx
	;
	; リージョン作成
	;
	mov	edx, nCount
	.IF	edx == 0
		xor	eax, eax
		ret
	.ENDIF
	imul	edx, (SIZEOF E3D_VECTOR_2D)
	INVOKE	eslHeapAllocate , [ebx].hHeap, edx, 0
	mov	pvxPoly, eax
	;
	mov	edi, eax
	mov	esi, pVertexes
	ASSUME	edi:PE3D_VECTOR_2D
	ASSUME	esi:PCEGL_POINT
	mov	ecx, nCount
	.REPEAT
		fild	[esi].x
		fiadd	[ebx].ptDrawOffset.x
		fstp	[edi].x
		fild	[esi].y
		fiadd	[ebx].ptDrawOffset.y
		fstp	[edi].y
		add	esi, (SIZEOF EGL_POINT)
		add	edi, (SIZEOF E3D_VECTOR_2D)
		dec	ecx
	.UNTIL	ZERO?
	ASSUME	edi:NOTHING
	ASSUME	esi:NOTHING
	;
	INVOKE	eglNormalizePolygonRegion ,
		[ebx].pRegion, ADDR [ebx].rectClip,
			nCount, pvxPoly, NULL, NULL
	mov	esi, eax
	;
	INVOKE	eslHeapFree , [ebx].hHeap, pvxPoly, 0
	;
	.IF	esi == 0
		xor	eax, eax
		ret
	.ENDIF

	ASSUME	esi:PTR E3D_POLYGON_REGION
	mov	ecx, [esi].nTopLine
	mov	edx, [esi].nBottomLine
	sub	edx, ecx
	inc	edx
	lea	esi, [esi].plrLineRgn[0]
	ASSUME	esi:PTR E3D_POLY_LINE_REGION
	;
	.IF	(SDWORD PTR edx) > 0
	.REPEAT
		mov	eax, 7FFFH
		mov	ecx, [esi].nLeft
		mov	edi, [esi].nRight
		cmp	eax, [esi].dwReserved1
		adc	ecx, 0
		cmp	eax, [esi].dwReserved2
		adc	edi, 0
		mov	[esi].nLeft, ecx
		mov	[esi].nRight, edi
		mov	[esi].dwReserved1, 0
		mov	[esi].dwReserved2, 0
		add	esi, (SIZEOF E3D_POLY_LINE_REGION)
		dec	edx
	.UNTIL	ZERO?
	.ENDIF

	INVOKE	eglDrawImage@SetFillFunc

	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglDrawImage@PrepareFillPolygon		ENDP

;
;	エラー
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@FillRegion_Error		PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE

	mov	eax, eslErrGeneral
	ret

eglDrawImage@FillRegion_Error		ENDP

;
;	描画領域が無い
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@FillRegion_NoDraw		PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE

	xor	eax, eax
	ret

eglDrawImage@FillRegion_NoDraw		ENDP

;
;	領域塗りつぶし 468/SSE 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@FillRegion_486		PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE

	;
	; 関数準備
	;
	mov	ebx, hDrawImage
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF

	mov	esi, [ebx].pRegion
	mov	edi, [ebx].dstimg.ptrImageArray

	ASSUME	esi:PTR E3D_POLYGON_REGION
	mov	ecx, [esi].nTopLine
	mov	edx, [esi].nBottomLine
	sub	edx, ecx
	imul	ecx, [ebx].dstimg.dwBytesPerLine
	inc	edx
	add	edi, ecx
	mov	ecx, edx
	;
	lea	esi, [esi].plrLineRgn[0]
	ASSUME	esi:PTR E3D_POLY_LINE_REGION
	;
	; 描画
	;
	.IF	(SDWORD PTR ecx) > 0
	pushfd
	cld
	.REPEAT
		mov	[ebx].nLeftHeight, ecx
		mov	[ebx].ptrDstLine, edi
		;
		mov	ecx, [esi].nLeft
		mov	edx, [esi].nRight
		INVOKE	eglDrawImage@FillLine_486
		.BREAK	.IF	CARRY?
		;
		mov	edi, [ebx].ptrDstLine
		mov	ecx, [ebx].nLeftHeight
		add	edi, [ebx].dstimg.dwBytesPerLine
		add	esi, (SIZEOF E3D_POLY_LINE_REGION)
		dec	ecx
	.UNTIL	ZERO?
	popfd
	.ENDIF

	.IF	ERI_EnabledProcessorType & ERI_USE_XMM_P3
		sfence
		emms
	.ELSEIF	ERI_EnabledProcessorType & ERI_USE_MMX_PENTIUM
		emms
	.ENDIF

	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglDrawImage@FillRegion_486		ENDP

ALIGN	10H
eglDrawImage@DrawRegion_486	PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE

	mov	ebx, hDrawImage
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF

	INVOKE	eglDrawImage@DrawRegion_Lines
	ASSUME	ebx:NOTHING
	ret

eglDrawImage@DrawRegion_486	ENDP

ALIGN	10H
eglDrawImage@FillLine_486		PROC	NEAR32 C

	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF
	mov	eax, [ebx].dstimg.dwBitsPerPixel
	.IF	eax == 32
		mov	eax, [ebx].colorDraw.dwPixelCode
		lea	edi, [edi + ecx * 4]
		sub	edx, ecx
		.IF	!SIGN?
		inc	edx
		.IF	ERI_EnabledProcessorType & ERI_USE_XMM_P3
			.IF	edi & 07H
				mov	DWORD PTR [edi], eax
				add	edi, 4
				dec	edx
			.ENDIF
			movd	mm0, eax
			sub	edx, 2
			punpckldq	mm0, mm0
			.WHILE	!SIGN?
				movntq	MMWORD PTR [edi], mm0
				add	edi, 8
				sub	edx, 2
			.ENDW
			add	edx, 2
		.ENDIF
		mov	ecx, edx
		rep	stosd
		.ENDIF
		clc

	.ELSEIF	eax == 24
		lea	eax, [ecx * 2 + ecx]
		mov	eax, [ebx].colorDraw.dwPixelCode
		add	edi, eax
		.REPEAT
			mov	WORD PTR [edi], ax
			ror	eax, 16
			mov	BYTE PTR [edi + 2], al
			add	edi, 3
			inc	ecx
			ror	eax, 16
		.UNTIL	ecx > edx
		clc

	.ELSEIF	eax == 8
		sub	edx, ecx
		lea	edi, [edi + ecx]
		movzx	eax, BYTE PTR [ebx].colorDraw.dwPixelCode
		lea	ecx, [edx + 1]
		mov	edx, eax
		shl	eax, 8
		or	eax, edx
		mov	edx, eax
		shl	eax, 16
		or	eax, edx
		mov	edx, ecx
		shr	ecx, 2
		and	edx, 03H
		rep	stosd
		mov	ecx, edx
		rep	stosb
		clc

	.ELSEIF	eax == 16
		mov	eax, [ebx].colorDraw.dwPixelCode
		.REPEAT
			mov	WORD PTR [edi + ecx * 2], ax
			inc	ecx
		.UNTIL	ecx > edx
		clc

	.ELSEIF	eax == 1
		push	ebp
		push	esi
		mov	ebp, ecx
		mov	esi, edx
		and	ecx, 07H
		and	edx, 07H
		shr	ebp, 3
		shr	esi, 3
		mov	al, 80H
		mov	ah, 80H
		sar	al, cl
		mov	ecx, edx
		mov	edx, [ebx].colorDraw.dwPixelCode
		shl	al, 1
		and	edx, 01H
		sar	ah, cl
		not	al
		neg	edx
		add	edi, ebp
		sub	esi, ebp
		.IF	!ZERO?
			mov	cl, al
			not	al
			and	cl, dl
			and	al, BYTE PTR [edi]
			or	al, cl
			mov	BYTE PTR [edi], al
			inc	edi
			;
			dec	esi
			.WHILE	!ZERO?
				mov	BYTE PTR [edi], dl
				inc	edi
				dec	esi
			.ENDW
			;
			mov	ch, ah
			not	ah
			and	ch, dl
			and	ah, BYTE PTR [edi]
			or	ah, cl
			mov	BYTE PTR [edi], ah
		.ELSE
			and	al, ah
			mov	ah, al
			not	al
			and	ah, dl
			and	al, BYTE PTR [edi]
			or	al, ah
			mov	BYTE PTR [edi], al
		.ENDIF
		pop	esi
		pop	ebp
		clc

	.ELSE
		TRACE	<"未対応の画像フォーマットです。", 0AH>
		stc
	.ENDIF
	ASSUME	ebx:NOTHING
	ret

eglDrawImage@FillLine_486		ENDP

;
;	領域塗りつぶし 468 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@FillRegion_Blend_486	PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE

	mov	ebx, hDrawImage
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF
	;
	; 描画準備
	;
	mov	esi, [ebx].pRegion
	mov	edi, [ebx].dstimg.ptrImageArray

	ASSUME	esi:PTR E3D_POLYGON_REGION
	mov	ecx, [esi].nTopLine
	mov	edx, [esi].nBottomLine
	sub	edx, ecx
	imul	ecx, [ebx].dstimg.dwBytesPerLine
	inc	edx
	add	edi, ecx
	mov	ecx, edx
	;
	lea	esi, [esi].plrLineRgn[0]
	ASSUME	esi:PTR E3D_POLY_LINE_REGION
	;
	; 描画
	;
	.IF	(SDWORD PTR ecx) > 0
	.REPEAT
		mov	[ebx].nLeftHeight, ecx
		mov	[ebx].ptrDstLine, edi
		;
		mov	ecx, [esi].nLeft
		mov	edx, [esi].nRight
		INVOKE	eglDrawImage@FillLine_Blend_486
		.BREAK	.IF	CARRY?
		;
		mov	edi, [ebx].ptrDstLine
		mov	ecx, [ebx].nLeftHeight
		add	edi, [ebx].dstimg.dwBytesPerLine
		add	esi, (SIZEOF E3D_POLY_LINE_REGION)
		dec	ecx
	.UNTIL	ZERO?
	.ENDIF

	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglDrawImage@FillRegion_Blend_486	ENDP

ALIGN	10H
eglDrawImage@DrawRegion_Blend_486	PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE

	mov	ebx, hDrawImage
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF

	INVOKE	eglDrawImage@DrawRegion_Lines
	ASSUME	ebx:NOTHING
	ret

eglDrawImage@DrawRegion_Blend_486	ENDP

ALIGN	10H
eglDrawImage@FillLine_Blend_486	PROC	NEAR32 C

	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF
	mov	eax, [ebx].dstimg.dwBitsPerPixel
	;
	.IF	[ebx].dstimg.fdwFormatType & EIF_WITH_ALPHA
		lea	edi, [edi + ecx * 4]
		sub	edx, ecx
		inc	edx
		.REPEAT
			movzx	eax, BYTE PTR [edi]
			movzx	ecx, BYTE PTR [edi + 1]
			mov	al, [ebx].nBlueTone[eax]
			mov	cl, [ebx].nBlueTone[ecx]
			add	al, BYTE PTR [ebx].dwTemp[0]
			sbb	ah, ah
			add	cl, BYTE PTR [ebx].dwTemp[4]
			sbb	ch, ch
			or	al, ah
			or	cl, ch
			mov	BYTE PTR [edi], al
			movzx	eax, BYTE PTR [edi + 2]
			mov	BYTE PTR [edi + 1], cl
			movzx	ecx, BYTE PTR [edi + 3]
			mov	al, [ebx].nBlueTone[eax]
			xor	ecx, 0FFH
			add	al, BYTE PTR [ebx].dwTemp[8]
			mov	cl, [ebx].nBlueTone[ecx]
			sbb	ah, ah
			not	cl
			or	al, ah
			mov	BYTE PTR [edi + 3], cl
			mov	BYTE PTR [edi + 2], al
			add	edi, 4
			dec	edx
		.UNTIL	ZERO?
		clc
	.ELSEIF	eax == 32
		lea	edi, [edi + ecx * 4]
		sub	edx, ecx
		inc	edx
		.REPEAT
			mov	[ebx].nLeftWidth, edx
			movzx	edx, BYTE PTR [edi]
			movzx	ecx, BYTE PTR [edi + 1]
			movzx	eax, BYTE PTR [edi + 2]
			mov	dl, [ebx].nBlueTone[edx]
			mov	cl, [ebx].nBlueTone[ecx]
			mov	al, [ebx].nBlueTone[eax]
			add	dl, BYTE PTR [ebx].dwTemp[0]
			sbb	dh, dh
			add	cl, BYTE PTR [ebx].dwTemp[4]
			sbb	ch, ch
			add	al, BYTE PTR [ebx].dwTemp[8]
			sbb	ah, ah
			or	dl, dh
			or	cl, ch
			or	al, ah
			mov	BYTE PTR [edi], dl
			mov	edx, [ebx].nLeftWidth
			mov	BYTE PTR [edi + 1], cl
			mov	BYTE PTR [edi + 2], al
			add	edi, 4
			dec	edx
		.UNTIL	ZERO?
		clc
	.ELSEIF	eax == 24
		sub	edx, ecx
		lea	ecx, [ecx + ecx * 2]
		inc	edx
		add	edi, ecx
		.REPEAT
			mov	[ebx].nLeftWidth, edx
			movzx	edx, BYTE PTR [edi]
			movzx	ecx, BYTE PTR [edi + 1]
			movzx	eax, BYTE PTR [edi + 2]
			mov	dl, [ebx].nBlueTone[edx]
			mov	cl, [ebx].nBlueTone[ecx]
			mov	al, [ebx].nBlueTone[eax]
			add	dl, BYTE PTR [ebx].dwTemp[0]
			sbb	dh, dh
			add	cl, BYTE PTR [ebx].dwTemp[4]
			sbb	ch, ch
			add	al, BYTE PTR [ebx].dwTemp[8]
			sbb	ah, ah
			or	dl, dh
			or	cl, ch
			or	al, ah
			mov	BYTE PTR [edi], dl
			mov	edx, [ebx].nLeftWidth
			mov	BYTE PTR [edi + 1], cl
			mov	BYTE PTR [edi + 2], al
			add	edi, 3
			dec	edx
		.UNTIL	ZERO?
		clc
	.ELSE
		TRACE	<"未対応の画像フォーマットです。", 0AH>
		stc
	.ENDIF
	ASSUME	ebx:NOTHING
	ret

eglDrawImage@FillLine_Blend_486	ENDP


;
;	領域塗りつぶし MMX 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@FillRegion_Blend_MMX	PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE

	mov	ebx, hDrawImage
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF
	;
	; 透明度適用
	;
	mov	eax, 100H
	sub	eax, [ebx].nTrans
	.IF	SIGN?
		xor	eax, eax
	.ELSEIF	eax > 100H
		mov	eax, 100H
	.ENDIF
	;
	movd	mm7, eax
	movd	mm0, [ebx].colorDraw.dwPixelCode
	punpcklwd	mm7, mm7
	pxor	mm6, mm6
	punpckldq	mm7, mm7
	punpcklbw	mm0, mm6
	pmullw	mm0, mm7
	psrlw	mm0, 8
	movq	mm1, mm0
	;
	pand	mm0, mmxPWMask0111
	psrlq	mm1, 48
	punpcklwd	mm1, mm1
	punpckldq	mm1, mm1
	pxor	mm1, mmxPWMask0FFH
	paddw	mm1, mmxPW1
	;
	movq	mm2, mmxPWMask1000
	;
	; 描画準備
	;
	mov	esi, [ebx].pRegion
	mov	edi, [ebx].dstimg.ptrImageArray

	ASSUME	esi:PTR E3D_POLYGON_REGION
	mov	ecx, [esi].nTopLine
	mov	edx, [esi].nBottomLine
	sub	edx, ecx
	imul	ecx, [ebx].dstimg.dwBytesPerLine
	inc	edx
	add	edi, ecx
	mov	ecx, edx
	;
	lea	esi, [esi].plrLineRgn[0]
	ASSUME	esi:PTR E3D_POLY_LINE_REGION
	;
	; 描画
	;
	.IF	[ebx].dstimg.dwBitsPerPixel != 32
		TRACE	<"未対応の画像フォーマットです。", 0AH>
		emms
		xor	eax, eax
		ret
	.ENDIF
	.IF	!([ebx].dstimg.fdwFormatType & EIF_WITH_ALPHA)
		pxor	mm2, mm2
	.ENDIF
	.IF	(SDWORD PTR ecx) > 0
	.REPEAT
		mov	[ebx].nLeftHeight, ecx
		mov	[ebx].ptrDstLine, edi
		;
		mov	eax, [ebx].dstimg.dwBitsPerPixel
		mov	ecx, [esi].nLeft
		mov	edx, [esi].nRight
		lea	edi, [edi + ecx * 4]
		;
		sub	edx, ecx
		inc	edx
		.IF	ERI_EnabledProcessorType & ERI_USE_XMM_P3
			mov	eax, [ebx].dstimg.dwBytesPerLine
			sub	edx, 4
			.WHILE	!SIGN?
				movq	mm3, MMWORD PTR [edi]
				prefetchnta	[edi + eax]
				pxor	mm3, mmxMaskDWHB
				movq	mm4, mm3
				punpcklbw	mm3, mm6
				punpckhbw	mm4, mm6
				pmullw	mm3, mm1
					movq	mm5, MMWORD PTR [edi + 8]
					pxor	mm5, mmxMaskDWHB
					movq	mm7, mm5
				pmullw	mm4, mm1
					punpcklbw	mm5, mm6
					punpckhbw	mm7, mm6
				psrlw	mm3, 8
					pmullw	mm5, mm1
				psrlw	mm4, 8
				paddusw	mm3, mm0
				paddusw	mm4, mm0
					pmullw	mm7, mm1
				pxor	mm3, mm2
					psrlw	mm5, 8
				pxor	mm4, mm2
					psrlw	mm7, 8
					paddusw	mm5, mm0
					paddusw	mm7, mm0
					pxor	mm5, mm2
					pxor	mm7, mm2
				packuswb	mm3, mm4
					packuswb	mm5, mm7
				movq	MMWORD PTR [edi], mm3
				movq	MMWORD PTR [edi + 8], mm5
				add	edi, 16
				sub	edx, 4
			.ENDW
			add	edx, 4
		.ENDIF
		test	edx, edx
		.WHILE	!ZERO?
			movd		mm4, DWORD PTR [edi]
			punpcklbw	mm4, mm6
			pxor		mm4, mm2
			pmullw		mm4, mm1
			psrlw		mm4, 8
			paddusw		mm4, mm0
			pxor		mm4, mm2
			packuswb	mm4, mm6
			movd		DWORD PTR [edi], mm4
			add		edi, 4
			dec		edx
		.ENDW
		;
		mov	edi, [ebx].ptrDstLine
		mov	ecx, [ebx].nLeftHeight
		add	edi, [ebx].dstimg.dwBytesPerLine
		add	esi, (SIZEOF E3D_POLY_LINE_REGION)
		dec	ecx
	.UNTIL	ZERO?
	.ENDIF

	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	emms
	xor	eax, eax
	ret

eglDrawImage@FillRegion_Blend_MMX	ENDP

;
;	領域反転描画 468 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@FillRegion_Inversion_486	PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE

	mov	ebx, hDrawImage
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF
	;
	; 描画準備
	;
	mov	[ebx].dwSaveRegEBP, ebp
	mov	esi, [ebx].pRegion
	mov	edi, [ebx].dstimg.ptrImageArray
	mov	eax, [ebx].dstimg.dwBitsPerPixel
	.IF	(eax != 32) && (eax != 24)
		TRACE	<"未対応の画像フォーマットです。", 0AH>
		mov	eax, eslErrGeneral
		ret
	.ENDIF
	shr	eax, 3
	mov	[ebx].dwTemp[0], eax

	ASSUME	esi:PTR E3D_POLYGON_REGION
	mov	ecx, [esi].nTopLine
	mov	edx, [esi].nBottomLine
	sub	edx, ecx
	imul	ecx, [ebx].dstimg.dwBytesPerLine
	inc	edx
	add	edi, ecx
	mov	ecx, edx
	;
	lea	esi, [esi].plrLineRgn[0]
	ASSUME	esi:PTR E3D_POLY_LINE_REGION
	;
	; 描画
	;
	.IF	(SDWORD PTR ecx) > 0
	.REPEAT
		mov	[ebx].nLeftHeight, ecx
		mov	[ebx].ptrDstLine, edi
		;
		mov	ecx, [esi].nLeft
		mov	edx, [esi].nRight
		;
		mov	[ebx].dwTemp[4], esi
		;
		INVOKE	eglDrawImage@FillLine_Inversion_486
		;
		mov	esi, [ebx].dwTemp[4]
		mov	edi, [ebx].ptrDstLine
		mov	ecx, [ebx].nLeftHeight
		add	edi, [ebx].dstimg.dwBytesPerLine
		add	esi, (SIZEOF E3D_POLY_LINE_REGION)
		dec	ecx
	.UNTIL	ZERO?
	.ENDIF

	mov	ebp, [ebx].dwSaveRegEBP
	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglDrawImage@FillRegion_Inversion_486	ENDP

ALIGN	10H
eglDrawImage@DrawRegion_Inversion_486	PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE

	mov	ebx, hDrawImage
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF
	;
	; 描画準備
	;
	mov	[ebx].dwSaveRegEBP, ebp
	mov	esi, [ebx].pRegion
	mov	edi, [ebx].dstimg.ptrImageArray
	mov	eax, [ebx].dstimg.dwBitsPerPixel
	.IF	(eax != 32) && (eax != 24)
		TRACE	<"未対応の画像フォーマットです。", 0AH>
		mov	eax, eslErrGeneral
		ret
	.ENDIF
	shr	eax, 3
	mov	[ebx].dwTemp[0], eax

	INVOKE	eglDrawImage@DrawRegion_Lines
	ASSUME	ebx:NOTHING
	ret

eglDrawImage@DrawRegion_Inversion_486	ENDP

ALIGN	10H
eglDrawImage@FillLine_Inversion_486	PROC	NEAR32

	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF
	mov	ebp, [ebx].dwTemp[0]
	lea	edi, [edi + ecx * 4]
	sub	edx, ecx
	lea	esi, [edx + 1]
	.REPEAT
		movzx	edx, BYTE PTR [edi]
		movzx	ecx, BYTE PTR [edi + 1]
		movzx	eax, BYTE PTR [edi + 2]
		mov	dl, [ebx].nBlueTone[edx]
		mov	cl, [ebx].nGreenTone[ecx]
		mov	al, [ebx].nRedTone[eax]
		mov	BYTE PTR [edi], dl
		mov	BYTE PTR [edi + 1], cl
		mov	BYTE PTR [edi + 2], al
		add	edi, ebp
		dec	esi
	.UNTIL	ZERO?
	ASSUME	ebx:NOTHING
	clc
	ret

eglDrawImage@FillLine_Inversion_486	ENDP

;
;	領域輪郭線描画
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@DrawRegion_Lines	PROC	NEAR32 C

	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF
	mov	[ebx].dwSaveRegEBP, ebp
	mov	esi, [ebx].pRegion
	mov	edi, [ebx].dstimg.ptrImageArray

	ASSUME	esi:PTR E3D_POLYGON_REGION
	mov	ecx, [esi].nTopLine
	mov	edx, [esi].nBottomLine
	sub	edx, ecx
	imul	ecx, [ebx].dstimg.dwBytesPerLine
	inc	edx
	add	edi, ecx
	mov	ecx, edx
	;
	lea	esi, [esi].plrLineRgn[0]
	ASSUME	esi:PTR E3D_POLY_LINE_REGION
	@PLINE_STEP = (SIZEOF E3D_POLY_LINE_REGION)
	;
	; 描画
	;
	.IF	(SDWORD PTR ecx) > 0
	mov	eax, 1
	.REPEAT
		mov	[ebx].nLeftHeight, ecx
		mov	[ebx].ptrDstLine, edi
		mov	[ebx].dwTemp[10H], esi
		;
		test	eax, eax
		.IF	!ZERO?
			;
			; 先頭ライン
			;
			mov	ecx, [esi].nLeft
			mov	edx, [esi].nRight
			call	[ebx].pfnDrawRegionLine

		.ELSEIF	ecx == 1
			;
			; 最終ライン
			;
			mov	ecx, [esi].nLeft
			mov	edx, [esi].nRight
			mov	eax, [esi - @PLINE_STEP].nLeft
			.IF	(SDWORD PTR eax) < (SDWORD PTR ecx)
				lea	ecx, [eax + 1]
			.ENDIF
			mov	eax, [esi - @PLINE_STEP].nRight
			.IF	(SDWORD PTR eax) > (SDWORD PTR edx)
				lea	edx, [eax - 1]
			.ENDIF
			call	[ebx].pfnDrawRegionLine

		.ELSE
			;
			; 中間ライン
			;
			mov	ecx, [esi].nLeft
			mov	edx, [esi - @PLINE_STEP].nLeft
			.IF	(SDWORD PTR ecx) < (SDWORD PTR edx)
				dec	edx
			.ELSEIF	(SDWORD PTR ecx) > (SDWORD PTR edx)
				inc	edx
				xchg	ecx, edx
			.ENDIF
			mov	[ebx].dwTemp[20H], ecx
			mov	[ebx].dwTemp[24H], edx
			;
			mov	ecx, [esi].nRight
			mov	edx, [esi - @PLINE_STEP].nRight
			.IF	(SDWORD PTR ecx) < (SDWORD PTR edx)
				dec	edx
			.ELSEIF	(SDWORD PTR ecx) > (SDWORD PTR edx)
				inc	edx
				xchg	ecx, edx
			.ENDIF
			mov	[ebx].dwTemp[28H], ecx
			mov	[ebx].dwTemp[2CH], edx
			;
			.IF	(SDWORD PTR ecx) < (SDWORD PTR [ebx].dwTemp[24H])
				mov	ecx, [ebx].dwTemp[20H]
				call	[ebx].pfnDrawRegionLine
			.ELSE
				call	[ebx].pfnDrawRegionLine
				;
				mov	edi, [ebx].ptrDstLine
				mov	ecx, [ebx].dwTemp[20H]
				mov	edx, [ebx].dwTemp[24H]
				call	[ebx].pfnDrawRegionLine
			.ENDIF
		.ENDIF
		;
		mov	esi, [ebx].dwTemp[10H]
		mov	edi, [ebx].ptrDstLine
		mov	ecx, [ebx].nLeftHeight
		add	edi, [ebx].dstimg.dwBytesPerLine
		add	esi, @PLINE_STEP
		xor	eax, eax
		dec	ecx
	.UNTIL	ZERO?
	.ENDIF

	.IF	ERI_EnabledProcessorType & ERI_USE_XMM_P3
		sfence
		emms
	.ELSEIF	ERI_EnabledProcessorType & ERI_USE_MMX_PENTIUM
		emms
	.ENDIF

	mov	ebp, [ebx].dwSaveRegEBP
	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglDrawImage@DrawRegion_Lines	ENDP


CodeSeg	ENDS

	END
