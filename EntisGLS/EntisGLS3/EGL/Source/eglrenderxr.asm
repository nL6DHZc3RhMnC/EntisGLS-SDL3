
; ----------------------------------------------------------------------------
;                          Entis Graphic Library
; ----------------------------------------------------------------------------
;    Copyright (c) 2002-2008 Leshade Entis, Entis-soft. Al rights reserved.
; ----------------------------------------------------------------------------


	.686
	.XMM
	.MODEL	FLAT

	INCLUDE	experi.inc
	INCLUDE	egl.inc

LEFT_ERROR_RANGE	EQU	100H	; ライン左端・少数誤差幅（/10000H）


; ----------------------------------------------------------------------------
;	データセグメント
; ----------------------------------------------------------------------------

ConstSeg	SEGMENT	PARA READONLY FLAT 'CONST'

ALIGN	10H
mmxAlphaMask	LABEL	MMWORD
		WORD	00H, 00H, 00H, 0FFH
mmxColorMask	LABEL	MMWORD
		BYTE	0FFH, 0FFH, 0FFH, 0, 0FFH, 0FFH, 0FFH, 0
mmxTrimMask	LABEL	MMWORD
		BYTE	80H, 80H, 80H, 0, 80H, 80H, 80H, 0
mmxBiasZ	LABEL	MMWORD
		DWORD	80H, 80H
mmxMaskLowDWord	LABEL	MMWORD
		DWORD	0FFFFFFFFH, 0
mmxMaskHightDWord	LABEL	MMWORD
		DWORD	0, 0FFFFFFFFH
ALIGN	10H
xmmScale16bit	REAL4	4 DUP( 65536.0 )
xmmConst1	REAL4	4 DUP( 1.0 )
xmmConstHalf	REAL4	4 DUP( 0.5 )
xmmBiasScaleZ	REAL4	4 DUP( 3.5 )
xmmBiasZ	REAL4	4 DUP( 1.00125 )
xmmBiasDeltaZ	REAL4	4 DUP( 0.000244140625 )
xmmMaskSignFlag	DWORD	4 DUP( 7FFFFFFFH )
xmmSignFlag	DWORD	4 DUP( 80000000H )

ConstSeg	ENDS


; ----------------------------------------------------------------------------
;	コードセグメント
; ----------------------------------------------------------------------------

CodeSeg	SEGMENT	PARA READONLY FLAT 'CODE'

;
;	レンダリングフィル（RGB）関数 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_StoreSSEC	PROC	NEAR32 C

	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	mov	[ebx].dib.dwSaveRegEBP, ebp
	mov	esi, [ebx].pLineBuf[0]
	ASSUME	esi:PTR E3D_TRANS_LINE_BUF
	mov	ebp, [ebx].dib.ptrZBufLine
	mov	edi, [ebx].dib.ptrDstLine
	;
	mov	ecx, [ebx].nLineLeft[0]
	cmp	[ebx].nLeftDecimal, LEFT_ERROR_RANGE
	lea	eax, [ecx + 1]
	cmova	ecx, eax
	mov	eax, esi
	lea	edx, [esi + (SIZEOF E3D_TRANS_LINE_BUF)]
	cmova	eax, edx
	test	ecx, 1
	cmovz	esi, eax
	cmp	ecx, [ebx].nLineRight[0]
	ja	Label_Exit
	;
	lea	ebp, [ebp + ecx * 4]
	lea	edi, [edi + ecx * 4]
	;
	; 左側の端数ピクセルを出力
	;
	.IF	(ecx & 01H) && ((SDWORD PTR ecx) <= [ebx].nLineRight[0])
		movq	mm2, MMWORD PTR [ebp - 4]
		movq	mm1, MMWORD PTR [esi].rZValue[0]
		sub	ebp, 4
		movq	mm3, mm2
		pcmpgtd	mm2, mm1
		sub	edi, 4
		psrlq	mm2, 32
		movq	mm0, MMWORD PTR [esi].rgbAdd[0]
		psllq	mm2, 32
		;
		pand	mm1, mm2
		maskmovq	mm0, mm2
		pandn	mm2, mm3
		add	edi, 8
		por	mm1, mm2
		inc	ecx
		movq	MMWORD PTR [ebp], mm1
		add	ebp, 8
		add	esi, (SIZEOF E3D_TRANS_LINE_BUF)
	.ENDIF
	;
	; ピクセル連続出力
	;
	mov	edx, [ebx].nLineRight[0]
	inc	edx
	sub	edx, ecx
	sub	edx, 4
	.WHILE	!SIGN?
		movq	mm2, MMWORD PTR [ebp]
		movq	mm6, MMWORD PTR [ebp + 8]
		movq	mm0, MMWORD PTR [esi].rgbAdd[0]
		movq	mm4, MMWORD PTR [esi + @LBSTEP].rgbAdd[0]
		movq	mm1, MMWORD PTR [esi].rZValue[0]
		movq	mm5, MMWORD PTR [esi + @LBSTEP].rZValue[0]
		movq	mm3, mm2
		movq	mm7, mm6
		pcmpgtd	mm2, mm1
		pcmpgtd	mm6, mm5
		;
		pand	mm1, mm2
		pand	mm5, mm6
		maskmovq	mm0, mm2
		add	edi, 8
		pandn	mm2, mm3
		maskmovq	mm4, mm6
		pandn	mm6, mm7
		add	edi, 8
		add	esi, @LBSTEP * 2
		;
		por	mm1, mm2
		por	mm5, mm6
		movq	MMWORD PTR [ebp], mm1
		movq	MMWORD PTR [ebp + 8], mm5
		add	ebp, 16
		sub	edx, 4
	.ENDW
	;
	; 右側の端数ピクセルを出力
	;
	add	edx, 4
	.IF	(SDWORD PTR edx) >= 2
		movq	mm2, QWORD PTR [ebp]
		movq	mm1, MMWORD PTR [esi].rZValue[0]
		movq	mm0, MMWORD PTR [esi].rgbAdd[0]
		movq	mm3, mm2
		pcmpgtd	mm2, mm1
		;
		pand	mm1, mm2
		maskmovq	mm0, mm2
		pandn	mm2, mm3
		add	edi, 8
		por	mm1, mm2
		movq	MMWORD PTR [ebp], mm1
		add	ebp, 8
		add	esi, (SIZEOF E3D_TRANS_LINE_BUF)
		sub	edx, 2
	.ENDIF
	.IF	(SDWORD PTR edx) >= 1
		movd	mm2, DWORD PTR [ebp]
		movd	mm1, DWORD PTR [esi].rZValue[0]
		movd	mm0, DWORD PTR [esi].rgbAdd[0]
		movq	mm3, mm2
		pcmpgtd	mm2, mm1
		;
		pand	mm1, mm2
		maskmovq	mm0, mm2
		pandn	mm2, mm3
		por	mm1, mm2
		movd	DWORD PTR [ebp], mm1
	.ENDIF
	;
Label_Exit:
	mov	ebp, [ebx].dib.dwSaveRegEBP
	ret

eglRenderPoly@RenderPolygon_StoreSSEC	ENDP

;
;	レンダリングフィル（RGB,z修正比較）関数 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_StoreSSE	PROC	NEAR32 C

jmp	eglRenderPoly@RenderPolygon_StoreSSEC

	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	mov	[ebx].dib.dwSaveRegEBP, ebp
	mov	esi, [ebx].pLineBuf[0]
	ASSUME	esi:PTR E3D_TRANS_LINE_BUF
	mov	ebp, [ebx].dib.ptrZBufLine
	mov	edi, [ebx].dib.ptrDstLine
	;
	mov	ecx, [ebx].nLineLeft[0]
	cmp	[ebx].nLeftDecimal, LEFT_ERROR_RANGE
	lea	eax, [ecx + 1]
	cmova	ecx, eax
	mov	eax, esi
	lea	edx, [esi + (SIZEOF E3D_TRANS_LINE_BUF)]
	cmova	eax, edx
	test	ecx, 1
	cmovz	esi, eax
	cmp	ecx, [ebx].nLineRight[0]
	ja	Label_Exit
	;
	; 左側の端数ピクセルを出力
	;
	@LBSTEP = (SIZEOF E3D_TRANS_LINE_BUF)
	lea	ebp, [ebp + ecx * 4]
	lea	edi, [edi + ecx * 4]
	movss	xmm7, xmmMaskSignFlag
	shufps	xmm7, xmm7, 0
	movlps	xmm0, QWORD PTR [esi].rZValue[0]
	movhps	xmm0, QWORD PTR [esi + @LBSTEP].rZValue[0]
	movaps	xmm1, xmm0
	shufps	xmm0, xmm0, 10110001B
	subps	xmm0, xmm1
	andps	xmm0, xmm7
	addps	xmm0, xmm1
	movlps	QWORD PTR [esi].rZValue[0], xmm0
	movhps	QWORD PTR [esi + @LBSTEP].rZValue[0], xmm0
	;
	.IF	ecx & 01H
			movlps	xmm0, QWORD PTR [esi + @LBSTEP*2].rZValue[0]
		movq	mm2, MMWORD PTR [ebp - 4]
		movq	mm1, MMWORD PTR [esi].rZValue[0]
		sub	ebp, 4
			movaps	xmm1, xmm0
			shufps	xmm0, xmm0, 10110001B
		movq	mm3, mm2
		pcmpgtd	mm2, mm1
			subps	xmm0, xmm1
		sub	edi, 4
		psrlq	mm2, 32
		movq	mm0, MMWORD PTR [esi].rgbAdd[0]
		psllq	mm2, 32
			andps	xmm0, xmm7
		;
		pand	mm1, mm2
		maskmovq	mm0, mm2
			addps	xmm0, xmm1
		pandn	mm2, mm3
		add	edi, 8
		por	mm1, mm2
		inc	ecx
		movq	MMWORD PTR [ebp], mm1
		add	ebp, 8
			movlps	QWORD PTR [esi + @LBSTEP*2].rZValue[0], xmm0
		add	esi, (SIZEOF E3D_TRANS_LINE_BUF)
	.ENDIF
	;
	; ピクセル連続出力
	;
	mov	edx, [ebx].nLineRight[0]
	inc	edx
	sub	edx, ecx
	sub	edx, 4
	.WHILE	!SIGN?
			movlps	xmm0, QWORD PTR [esi + @LBSTEP*2].rZValue[0]
		movq	mm2, MMWORD PTR [ebp]
		movq	mm6, MMWORD PTR [ebp + 8]
			movhps	xmm0, QWORD PTR [esi + @LBSTEP*3].rZValue[0]
		movq	mm0, MMWORD PTR [esi].rgbAdd[0]
		movq	mm4, MMWORD PTR [esi + @LBSTEP].rgbAdd[0]
			movaps	xmm1, xmm0
		movq	mm1, MMWORD PTR [esi].rZValue[0]
		movq	mm5, MMWORD PTR [esi + @LBSTEP].rZValue[0]
			shufps	xmm0, xmm0, 10110001B
		movq	mm3, mm2
		movq	mm7, mm6
		pcmpgtd	mm2, mm1
		pcmpgtd	mm6, mm5
			subps	xmm0, xmm1
		;
		pand	mm1, mm2
		pand	mm5, mm6
		maskmovq	mm0, mm2
		add	edi, 8
		pandn	mm2, mm3
		maskmovq	mm4, mm6
			andps	xmm0, xmm7
		pandn	mm6, mm7
		add	edi, 8
		add	esi, @LBSTEP * 2
		;
			addps	xmm0, xmm1
		por	mm1, mm2
		por	mm5, mm6
		movq	MMWORD PTR [ebp], mm1
		movq	MMWORD PTR [ebp + 8], mm5
		add	ebp, 16
			movlps	QWORD PTR [esi].rZValue[0], xmm0
		sub	edx, 4
			movhps	QWORD PTR [esi + @LBSTEP].rZValue[0], xmm0
	.ENDW
	;
	; 右側の端数ピクセルを出力
	;
	add	edx, 4
	.IF	(SDWORD PTR edx) >= 2
		movq	mm2, QWORD PTR [ebp]
		movq	mm1, MMWORD PTR [esi].rZValue[0]
		movq	mm0, MMWORD PTR [esi].rgbAdd[0]
		movq	mm3, mm2		; mm3 = z-buf
		pcmpgtd	mm2, mm1		; mm2 = z-buf > z
		;
		pand	mm1, mm2
		maskmovq	mm0, mm2
		pandn	mm2, mm3
		add	edi, 8
		por	mm1, mm2		; mm1 = min( z-buf, z )
		movq	MMWORD PTR [ebp], mm1
		add	ebp, 8
		add	esi, (SIZEOF E3D_TRANS_LINE_BUF)
		sub	edx, 2
	.ENDIF
	.IF	(SDWORD PTR edx) >= 1
		movd	mm2, DWORD PTR [ebp]
		movd	mm1, DWORD PTR [esi].rZValue[0]
		movd	mm0, DWORD PTR [esi].rgbAdd[0]
		movq	mm3, mm2
		pcmpgtd	mm2, mm1
		;
		pand	mm1, mm2
		maskmovq	mm0, mm2
		pandn	mm2, mm3
		por	mm1, mm2
		movd	DWORD PTR [ebp], mm1
	.ENDIF
	;
Label_Exit:
	mov	ebp, [ebx].dib.dwSaveRegEBP
	ret

eglRenderPoly@RenderPolygon_StoreSSE	ENDP

;
;	レンダリングフィル（RGB,z-buf無比較）関数 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_StoreNZSSE	PROC	NEAR32 C

	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	mov	esi, [ebx].pLineBuf[0]
	ASSUME	esi:PTR E3D_TRANS_LINE_BUF
	mov	ecx, [ebx].nLineLeft[0]
	cmp	[ebx].nLeftDecimal, LEFT_ERROR_RANGE
	lea	eax, [ecx + 1]
	cmova	ecx, eax
	mov	eax, esi
	lea	edx, [esi + (SIZEOF E3D_TRANS_LINE_BUF)]
	cmova	eax, edx
	test	ecx, 1
	cmovz	esi, eax
	cmp	ecx, [ebx].nLineRight[0]
	ja	Label_Exit
	;
	mov	edi, [ebx].dib.ptrDstLine
	mov	edx, [ebx].nLineRight[0]
	lea	edi, [edi + ecx * 4]
	sub	edx, ecx
	js	Label_Exit
	inc	edx
	cmp	edx, 40H
	jge	Label_LongRender

Label_ShortRender:
	;
	; 左側の端数ピクセルを出力
	;
	@LBSTEP = (SIZEOF E3D_TRANS_LINE_BUF)
	.IF	ecx & 01H
		mov	eax, DWORD PTR [esi].rgbAdd[4]
		mov	DWORD PTR [edi], eax
		add	esi, @LBSTEP
		add	edi, 4
		dec	edx
	.ENDIF
	;
	; ピクセル連続出力
	;
	sub	edx, 4
	.WHILE	!SIGN?
		movq	mm0, MMWORD PTR [esi].rgbAdd[0]
		movq	mm1, MMWORD PTR [esi + @LBSTEP].rgbAdd[0]
		add	esi, @LBSTEP * 2
		movq	MMWORD PTR [edi],     mm0
		movq	MMWORD PTR [edi + 8], mm1
		add	edi, 16
		sub	edx, 4
	.ENDW
	;
	; 右側の端数ピクセルを出力
	;
	add	edx, 4
	.IF	(SDWORD PTR edx) >= 2
		movq	mm0, MMWORD PTR [esi].rgbAdd[0]
		movq	MMWORD PTR [edi], mm0
		add	edi, 8
		add	esi, (SIZEOF E3D_TRANS_LINE_BUF)
		sub	edx, 2
	.ENDIF
	.IF	(SDWORD PTR edx) >= 1
		mov	eax, DWORD PTR [esi].rgbAdd[0]
		mov	DWORD PTR [edi], eax
	.ENDIF
	ret
;	jmp	Label_Exit

Label_LongRender:
	;
	; 左側の端数ピクセルを出力
	;
	@LBSTEP = (SIZEOF E3D_TRANS_LINE_BUF)
	.IF	ecx & 01H
		sub	edi, 4
		pcmpeqd	mm1, mm1
		movq	mm0, MMWORD PTR [esi].rgbAdd[0]
		psllq	mm1, 32
		maskmovq	mm0, mm1
		add	edi, 8
		add	esi, (SIZEOF E3D_TRANS_LINE_BUF)
		dec	edx
	.ENDIF
	;
	; ピクセル連続出力
	;
	sub	edx, 4
	.WHILE	!SIGN?
		movq	mm0, MMWORD PTR [esi].rgbAdd[0]
		movq	mm1, MMWORD PTR [esi + @LBSTEP].rgbAdd[0]
		add	esi, @LBSTEP * 2
		movntq	MMWORD PTR [edi],     mm0
		movntq	MMWORD PTR [edi + 8], mm1
		add	edi, 16
		sub	edx, 4
	.ENDW
	;
	; 右側の端数ピクセルを出力
	;
	add	edx, 4
	.IF	(SDWORD PTR edx) >= 2
		movq	mm0, MMWORD PTR [esi].rgbAdd[0]
		movntq	MMWORD PTR [edi], mm0
		add	edi, 8
		add	esi, (SIZEOF E3D_TRANS_LINE_BUF)
		sub	edx, 2
	.ENDIF
	.IF	(SDWORD PTR edx) >= 1
		pcmpeqd	mm2, mm2
		movd	mm0, DWORD PTR [esi].rgbAdd[0]
		psrlq	mm2, 32
		maskmovq	mm0, mm2
	.ENDIF
	;
Label_Exit:
	ret

eglRenderPoly@RenderPolygon_StoreNZSSE	ENDP

;
;	レンダリングフィル（RGBA）関数 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_StoreNTXSSE	PROC	NEAR32 C

	call	eglRenderPoly@RenderPolygon_StoreASSE
	ret

eglRenderPoly@RenderPolygon_StoreNTXSSE	ENDP

;
;	レンダリング透明度付フィル（RGB）関数 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_StoreTSSE	PROC	NEAR32 C

	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	mov	[ebx].dib.dwSaveRegEBP, ebp
	mov	esi, [ebx].pLineBuf[0]
	ASSUME	esi:PTR E3D_TRANS_LINE_BUF
	mov	ebp, [ebx].dib.ptrZBufLine
	mov	edi, [ebx].dib.ptrDstLine
	;
	mov	ecx, [ebx].nLineLeft[0]
	cmp	[ebx].nLeftDecimal, 0 ; LEFT_ERROR_RANGE
	seta	dl
	cmp	ecx, [ebx].dib.rectClip.left
	seta	dh
	lea	eax, [ecx + 1]
	or	dl, dh
	cmovnz	ecx, eax
	mov	eax, esi
	lea	edx, [esi + (SIZEOF E3D_TRANS_LINE_BUF)]
	cmovnz	eax, edx
	test	ecx, 1
	cmovz	esi, eax
	cmp	ecx, [ebx].nLineRight[0]
	ja	Label_Exit
	;
;		movss	xmm7, xmmMaskSignFlag
;		movss	xmm6, xmmBiasZ
;		movss	xmm5, xmmBiasScaleZ
	movd	mm6, DWORD PTR [ebx].nTextureApply
	lea	ebp, [ebp + ecx * 4]
	lea	edi, [edi + ecx * 4]
;		shufps	xmm7, xmm7, 0
	pshufw	mm6, mm6, 0
	pxor	mm7, mm7
;		shufps	xmm6, xmm6, 0
	psrlw	mm6, 1
	;
	; 左側の端数ピクセルを出力
	;
	.IF	(ecx & 01H) && ((SDWORD PTR ecx) <= [ebx].nLineRight[0])
;				movss	xmm0, [esi].rZValue[0]
				movss	xmm1, [esi].rZValue[4]
		sub	edi, 4
		sub	ebp, 4
		movq	mm2, MMWORD PTR [esi].rZValue[0]
;				subss	xmm0, xmm1
		movq	mm0, MMWORD PTR [esi].rgbAdd[0]
		movq	mm4, MMWORD PTR [edi]
;				andps	xmm0, xmm7
		movq	mm1, mm0
;				mulss	xmm1, xmm6
		movq	mm5, mm4
		punpcklbw	mm0, mm7
;				mulss	xmm0, xmm5
		punpcklbw	mm4, mm7
		punpckhbw	mm1, mm7
				movss	xmm2, REAL4 PTR [ebp]
		punpckhbw	mm5, mm7
		psubsw	mm4, mm0
;				addss	xmm0, xmm1
		psubsw	mm5, mm1
		pmullw	mm4, mm6
				cmpss	xmm1, xmm2, 1
		pmullw	mm5, mm6
			inc	ecx
			add	esi, (SIZEOF E3D_TRANS_LINE_BUF)
		psraw	mm4, 7
				movss	[ebx].dib.dwTemp[0], xmm1
		psraw	mm5, 7
		paddsw	mm4, mm0
			movd	mm3, [ebx].dib.dwTemp[0]
		paddsw	mm5, mm1
			psllq	mm3, 32
		packuswb	mm4, mm5
		maskmovq	mm4, mm3
		add	edi, 8
		xchg	edi, ebp
		maskmovq	mm2, mm3
		add	edi, 8
		xchg	edi, ebp
	.ENDIF
	;
	; ピクセル連続出力
	;
	mov	edx, [ebx].nLineRight[0]
	inc	edx
	sub	edx, ecx
	sub	edx, 2
	.WHILE	!SIGN?
				movlps	xmm0, QWORD PTR [esi].rZValue[0]
		movq	mm0, MMWORD PTR [esi].rgbAdd[0]
		movq	mm4, MMWORD PTR [edi]
;				movaps	xmm1, xmm0
;				shufps	xmm0, xmm0, 1
		movq	mm1, mm0
;				subss	xmm0, xmm1
;				mulps	xmm1, xmm6
		movq	mm5, mm4
;				andps	xmm0, xmm7
		punpcklbw	mm0, mm7
		punpcklbw	mm4, mm7
;				mulss	xmm0, xmm5
			movq	mm2, MMWORD PTR [esi].rZValue[0]
				movlps	xmm2, QWORD PTR [ebp]
		punpckhbw	mm1, mm7
;				shufps	xmm0, xmm0, 0
		punpckhbw	mm5, mm7
;				addps	xmm0, xmm1
		psubsw	mm4, mm0
		psubsw	mm5, mm1
		pmullw	mm4, mm6
				cmpps	xmm0, xmm2, 1
		pmullw	mm5, mm6
			add	esi, (SIZEOF E3D_TRANS_LINE_BUF)
				movlps	QWORD PTR [ebx].dib.dwTemp[0], xmm0
		psraw	mm4, 7
		psraw	mm5, 7
		paddsw	mm4, mm0
		paddsw	mm5, mm1
				movq	mm3, MMWORD PTR [ebx].dib.dwTemp[0]
		packuswb	mm4, mm5
		maskmovq	mm4, mm3
		add	edi, 8
		xchg	edi, ebp
		maskmovq	mm2, mm3
		add	edi, 8
		xchg	edi, ebp
		sub	edx, 2
	.ENDW
	;
	; 右側の端数ピクセルを出力
	;
	add	edx, 2
	.IF	(SDWORD PTR edx) >= 1
				movss	xmm0, [esi].rZValue[0]
;				movss	xmm1, [esi].rZValue[4]
		movq	mm0, MMWORD PTR [esi].rgbAdd[0]
;				subss	xmm1, xmm0
		movq	mm4, MMWORD PTR [edi]
;				mulss	xmm0, xmm6
		movq	mm1, mm0
;				andps	xmm1, xmm7
		movq	mm5, mm4
		punpcklbw	mm0, mm7
;				mulss	xmm1, xmm5
		punpcklbw	mm4, mm7
		punpckhbw	mm1, mm7
				movss	xmm2, REAL4 PTR [ebp]
		punpckhbw	mm5, mm7
;				addss	xmm0, xmm1
		psubsw	mm4, mm0
		psubsw	mm5, mm1
		pmullw	mm4, mm6
				cmpss	xmm0, xmm2, 1
		pmullw	mm5, mm6
		psraw	mm4, 7
				movss	[ebx].dib.dwTemp[0], xmm0
		psraw	mm5, 7
		paddsw	mm4, mm0
			movq	mm2, MMWORD PTR [esi].rZValue[0]
		paddsw	mm5, mm1
				movd	mm3, [ebx].dib.dwTemp[0]
		packuswb	mm4, mm5
		maskmovq	mm4, mm3
		xchg	edi, ebp
		maskmovq	mm2, mm3
		xchg	edi, ebp
	.ENDIF
	;
Label_Exit:
	mov	ebp, [ebx].dib.dwSaveRegEBP
	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	ret

eglRenderPoly@RenderPolygon_StoreTSSE	ENDP

;
;	レンダリング透明度付フィル（RGB,z-buf無比較）関数 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_StoreTNZSSE	PROC	NEAR32 C

	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	mov	esi, [ebx].pLineBuf[0]
	ASSUME	esi:PTR E3D_TRANS_LINE_BUF
	mov	edi, [ebx].dib.ptrDstLine
	;
	mov	ecx, [ebx].nLineLeft[0]
	cmp	[ebx].nLeftDecimal, 0 ; LEFT_ERROR_RANGE
	seta	dl
	cmp	ecx, [ebx].dib.rectClip.left
	seta	dh
	lea	eax, [ecx + 1]
	or	dl, dh
	cmovnz	ecx, eax
	mov	eax, esi
	lea	edx, [esi + (SIZEOF E3D_TRANS_LINE_BUF)]
	cmovnz	eax, edx
	test	ecx, 1
	cmovz	esi, eax
	cmp	ecx, [ebx].nLineRight[0]
	ja	Label_Exit
	;
	mov	edx, [ebx].nLineRight[0]
	movd	mm6, DWORD PTR [ebx].nTextureApply
	lea	edi, [edi + ecx * 4]
	sub	edx, ecx
	js	Label_Exit
	pshufw	mm6, mm6, 0
	inc	edx
	pxor	mm7, mm7
	psrlw	mm6, 1
	;
	; 左側の端数ピクセルを出力
	;
	.IF	ecx & 01H
		movd	mm4, DWORD PTR [edi]
		movd	mm0, DWORD PTR [esi].rgbAdd[4]
		punpcklbw	mm4, mm7
		punpcklbw	mm0, mm7
		psubsw	mm4, mm0
		pmullw	mm4, mm6
		add	esi, (SIZEOF E3D_TRANS_LINE_BUF)
		psraw	mm4, 7
		paddsw	mm4, mm0
		packuswb	mm4, mm7
		movd	DWORD PTR [edi], mm4
		add	edi, 4
		dec	edx
	.ENDIF
	;
	; ピクセル連続出力
	;
	sub	edx, 2
	.WHILE	!SIGN?
		movq	mm0, MMWORD PTR [esi].rgbAdd[0]
		movq	mm4, MMWORD PTR [edi]
		movq	mm1, mm0
		movq	mm5, mm4
		punpcklbw	mm0, mm7
		punpcklbw	mm4, mm7
		punpckhbw	mm1, mm7
		punpckhbw	mm5, mm7
		psubsw	mm4, mm0
		psubsw	mm5, mm1
		pmullw	mm4, mm6
		pmullw	mm5, mm6
			add	esi, (SIZEOF E3D_TRANS_LINE_BUF)
		psraw	mm4, 7
		psraw	mm5, 7
		paddsw	mm4, mm0
		paddsw	mm5, mm1
		packuswb	mm4, mm5
		movq	MMWORD PTR [edi], mm4
		add	edi, 8
		sub	edx, 2
	.ENDW
	;
	; 右側の端数ピクセルを出力
	;
	add	edx, 2
	.IF	(SDWORD PTR edx) >= 1
		movd	mm4, DWORD PTR [edi]
		movd	mm0, DWORD PTR [esi].rgbAdd[0]
		punpcklbw	mm4, mm7
		punpcklbw	mm0, mm7
		psubsw	mm4, mm0
		pmullw	mm4, mm6
		psraw	mm4, 7
		paddsw	mm4, mm0
		packuswb	mm4, mm7
		movd	DWORD PTR [edi], mm4
	.ENDIF
	;
Label_Exit:
	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	ret

eglRenderPoly@RenderPolygon_StoreTNZSSE	ENDP

;
;	レンダリングフィル（RGBA トリム）関数 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_StoreMSSE	PROC	NEAR32 C

	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	mov	[ebx].dib.dwSaveRegEBP, ebp
	mov	esi, [ebx].pLineBuf[0]
	ASSUME	esi:PTR E3D_TRANS_LINE_BUF
	mov	ebp, [ebx].dib.ptrZBufLine
	mov	edi, [ebx].dib.ptrDstLine
	;
	mov	ecx, [ebx].nLineLeft[0]
	cmp	[ebx].nLeftDecimal, LEFT_ERROR_RANGE
	lea	eax, [ecx + 1]
	cmova	ecx, eax
	mov	eax, esi
	lea	edx, [esi + (SIZEOF E3D_TRANS_LINE_BUF)]
	cmova	eax, edx
	test	ecx, 1
	cmovz	esi, eax
	cmp	ecx, [ebx].nLineRight[0]
	ja	Label_Exit
	;
	lea	ebp, [ebp + ecx * 4]
	lea	edi, [edi + ecx * 4]
	;
	; 左側の端数ピクセルを出力
	;
	movq	mm6, MMWORD PTR mmxTrimMask
	pxor	mm7, mm7
	.IF	(ecx & 01H) && ((SDWORD PTR ecx) <= [ebx].nLineRight[0])
		sub	edi, 4
		sub	ebp, 4
		pcmpeqb	mm4, mm4
		movq	mm1, MMWORD PTR [esi].rgbMul[0]
		movq	mm2, MMWORD PTR [esi].rZValue[0]
		movq	mm3, MMWORD PTR [ebp]
		pand	mm1, mm6
		psllq	mm4, 32
		movq	mm5, mm3
		pcmpgtd	mm3, mm2
		pcmpgtd	mm1, mm7
		movq	mm0, MMWORD PTR [esi].rgbAdd[0]
		pandn	mm1, mm3
		pand	mm1, mm4
		;
		pand	mm2, mm1
		maskmovq	mm0, mm1
		pandn	mm1, mm5
		add	edi, 8
		por	mm2, mm1
		inc	ecx
		movq	MMWORD PTR [ebp], mm2
		add	ebp, 8
		add	esi, (SIZEOF E3D_TRANS_LINE_BUF)
	.ENDIF
	;
	; ピクセル連続出力
	;
	@LBSTEP = (SIZEOF E3D_TRANS_LINE_BUF)
	mov	edx, [ebx].nLineRight[0]
	inc	edx
	sub	edx, ecx
	sub	edx, 4
	.WHILE	!SIGN?
		movq	mm1, MMWORD PTR [esi].rgbMul[0]
		movq	mm2, MMWORD PTR [esi].rZValue[0]
		movq	mm3, MMWORD PTR [ebp]
		movq	mm0, MMWORD PTR [esi].rgbAdd[0]
		pand	mm1, mm6
		;
			pcmpgtd	mm3, mm2
		movq	mm4, MMWORD PTR [esi + @LBSTEP].rgbMul[0]
			pcmpgtd	mm1, mm7
		movq	mm5, MMWORD PTR [esi + @LBSTEP].rZValue[0]
		pand	mm4, mm6
			pandn	mm1, mm3
		movq	mm3, MMWORD PTR [ebp + 8]
		pcmpgtd	mm4, mm7
		pcmpgtd	mm3, mm5
		;
		maskmovq	mm0, mm1
		add	edi, 8
		movq	mm0, MMWORD PTR [esi + @LBSTEP].rgbAdd[0]
		;
		pandn	mm4, mm3
		pand	mm2, mm1
		pandn	mm1, MMWORD PTR [ebp]
		add	esi, @LBSTEP * 2
		maskmovq	mm0, mm4
		add	edi, 8
		;
		pand	mm5, mm4
		pandn	mm4, MMWORD PTR [ebp + 8]
		por	mm2, mm1
		por	mm5, mm4
		movq	MMWORD PTR [ebp], mm2
		movq	MMWORD PTR [ebp + 8], mm5
		add	ebp, 16
		sub	edx, 4
	.ENDW
	;
	; 右側の端数ピクセルを出力
	;
	add	edx, 4
	.IF	(SDWORD PTR edx) >= 2
		movq	mm1, MMWORD PTR [esi].rgbMul[0]
		movq	mm2, MMWORD PTR [esi].rZValue[0]
		movq	mm3, MMWORD PTR [ebp]
		pand	mm1, mm6
		movq	mm4, mm3
		pcmpgtd	mm3, mm2
		pcmpgtd	mm1, mm7
		movq	mm0, MMWORD PTR [esi].rgbAdd[0]
		pandn	mm1, mm3
		pand	mm2, mm1
		maskmovq	mm0, mm1
		add	edi, 8
		pandn	mm1, mm4
		por	mm2, mm1
		movq	MMWORD PTR [ebp], mm2
		add	ebp, 8
		add	esi, (SIZEOF E3D_TRANS_LINE_BUF)
		sub	edx, 2
	.ENDIF
	.IF	(SDWORD PTR edx) >= 1
		pcmpeqb	mm4, mm4
		movq	mm1, MMWORD PTR [esi].rgbMul[0]
		movq	mm2, MMWORD PTR [esi].rZValue[0]
		movq	mm3, MMWORD PTR [ebp]
		pand	mm1, mm6
		psrlq	mm4, 32
		movq	mm5, mm3
		pcmpgtd	mm3, mm2
		pcmpgtd	mm1, mm7
		movq	mm0, MMWORD PTR [esi].rgbAdd[0]
		pandn	mm1, mm3
		pand	mm1, mm4
		pand	mm2, mm1
		maskmovq	mm0, mm1
		pandn	mm1, mm5
		por	mm2, mm1
		movd	DWORD PTR [ebp], mm2
	.ENDIF
	;
Label_Exit:
	mov	ebp, [ebx].dib.dwSaveRegEBP
	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	ret

eglRenderPoly@RenderPolygon_StoreMSSE	ENDP

;
;	レンダリング透明度付フィル（RGBA トリム）関数 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_StoreMTSSE	PROC	NEAR32 C

	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	mov	[ebx].dib.dwSaveRegEBP, ebp
	mov	esi, [ebx].pLineBuf[0]
	ASSUME	esi:PTR E3D_TRANS_LINE_BUF
	mov	ebp, [ebx].dib.ptrZBufLine
	mov	edi, [ebx].dib.ptrDstLine
	;
	mov	ecx, [ebx].nLineLeft[0]
	cmp	[ebx].nLeftDecimal, 0 ; LEFT_ERROR_RANGE
	seta	dl
	cmp	ecx, [ebx].dib.rectClip.left
	seta	dh
	lea	eax, [ecx + 1]
	or	dl, dh
	cmovnz	ecx, eax
	mov	eax, esi
	lea	edx, [esi + (SIZEOF E3D_TRANS_LINE_BUF)]
	cmovnz	eax, edx
	test	ecx, 1
	cmovz	esi, eax
	cmp	ecx, [ebx].nLineRight[0]
	ja	Label_Exit
	;
	lea	ebp, [ebp + ecx * 4]
	lea	edi, [edi + ecx * 4]
	;
	; 左側の端数ピクセルを出力
	;
;		movss	xmm7, xmmMaskSignFlag
;		movss	xmm6, xmmBiasZ
	movd	mm6, DWORD PTR [ebx].nTextureApply
;		movss	xmm5, xmmBiasScaleZ
	pshufw	mm6, mm6, 0
	pxor	mm7, mm7
;		shufps	xmm7, xmm7, 0
	psrlw	mm6, 1
;		shufps	xmm6, xmm6, 0
	.IF	(ecx & 01H) && ((SDWORD PTR ecx) <= [ebx].nLineRight[0])
		sub	edi, 4
		sub	ebp, 4
;				movss	xmm0, [esi].rZValue[0]
				movss	xmm1, [esi].rZValue[4]
		movd	mm2, [esi].rgbMul[4].dwPixelCode
;				subss	xmm0, xmm1
		movd	mm0, [esi].rgbAdd[4].dwPixelCode
		movd	mm1, DWORD PTR [edi]
;				andps	xmm0, xmm7
;				mulss	xmm1, xmm6
		punpcklbw	mm0, mm7
		punpcklbw	mm1, mm7
;				mulss	xmm0, xmm5
		psubsw	mm1, mm0
				movss	xmm2, REAL4 PTR [ebp + 4]
		pmullw	mm1, mm6
			pand	mm2, mmxTrimMask
;				addss	xmm0, xmm1
		psraw	mm1, 7
;				cmpss	xmm0, xmm2, 1
				cmpss	xmm1, xmm2, 1
		paddsw	mm1, mm0
			pcmpgtd	mm2, mm7
				movss	[ebx].dib.dwTemp[0], xmm1
		packuswb	mm1, mm7
				movd	mm0, [ebx].dib.dwTemp[0]
		psllq	mm1, 32
			pandn	mm2, mm0
		movq	mm3, MMWORD PTR [esi].rZValue[0]
			psllq	mm2, 32
		;
		maskmovq	mm1, mm2
		add	edi, 8
		xchg	edi, ebp
		maskmovq	mm3, mm2
		add	edi, 8
		xchg	edi, ebp
		inc	ecx
		add	esi, (SIZEOF E3D_TRANS_LINE_BUF)
	.ENDIF
	;
	; ピクセル連続出力
	;
	mov	edx, [ebx].nLineRight[0]
	inc	edx
	sub	edx, ecx
	sub	edx, 2
	.WHILE	!SIGN?
				movlps	xmm0, QWORD PTR [esi].rZValue[0]
		movq	mm2, MMWORD PTR [edi]
		movq	mm0, MMWORD PTR [esi].rgbAdd[0]
;				movaps	xmm1, xmm0
;				shufps	xmm0, xmm0, 1
		movq	mm1, mm0
;				subss	xmm0, xmm1
		movq	mm3, mm2
;				mulps	xmm1, xmm6
		punpcklbw	mm0, mm7
;				andps	xmm0, xmm7
		punpcklbw	mm2, mm7
;				mulss	xmm0, xmm5
		punpckhbw	mm1, mm7
		punpckhbw	mm3, mm7
		psubsw	mm2, mm0
;				shufps	xmm0, xmm0, 0
		psubsw	mm3, mm1
;				addps	xmm0, xmm1
				movlps	xmm2, QWORD PTR [ebp]
			movq	mm4, MMWORD PTR [esi].rgbMul[0]
		pmullw	mm2, mm6
			movq	mm5, MMWORD PTR [esi].rZValue[0]
				cmpps	xmm0, xmm2, 1
		pmullw	mm3, mm6
			pand	mm4, MMWORD PTR mmxTrimMask
				movlps	QWORD PTR [ebx].dib.dwTemp[0], xmm0
		psraw	mm2, 7
			pcmpgtd	mm4, mm7
		psraw	mm3, 7
		paddsw	mm0, mm2
		paddsw	mm1, mm3
			movq	mm2, MMWORD PTR [ebx].dib.dwTemp[0]
		packuswb	mm0, mm1
			pandn	mm4, mm2
		;
		add	esi, (SIZEOF E3D_TRANS_LINE_BUF)
		maskmovq	mm0, mm4
		add	edi, 8
		xchg	edi, ebp
		maskmovq	mm5, mm4
		add	edi, 8
		sub	edx, 2
		xchg	edi, ebp
	.ENDW
	;
	; 右側の端数ピクセルを出力
	;
	add	edx, 2
	.IF	(SDWORD PTR edx) >= 1
				movss	xmm0, [esi].rZValue[0]
;				movss	xmm1, [esi].rZValue[4]
		movd	mm2, [esi].rgbMul[0].dwPixelCode
;				subss	xmm1, xmm0
;				mulss	xmm0, xmm6
		movd	mm1, [esi].rgbAdd[0].dwPixelCode
		movd	mm0, DWORD PTR [edi]
;				andps	xmm1, xmm7
		punpcklbw	mm0, mm7
;				mulss	xmm1, xmm5
		punpcklbw	mm1, mm7
		psubsw	mm0, mm1
				movss	xmm2, REAL4 PTR [ebp]
		pmullw	mm0, mm6
;				addss	xmm0, xmm1
			pand	mm2, mmxTrimMask
		psraw	mm0, 7
				cmpss	xmm0, xmm2, 1
			pcmpgtd	mm2, mm7
		paddsw	mm0, mm1
				movss	[ebx].dib.dwTemp[0], xmm0
		packuswb	mm0, mm7
			movd	mm1, [ebx].dib.dwTemp[0]
		movq	mm3, MMWORD PTR [esi].rZValue[0]
			pandn	mm2, mm1
		;
		maskmovq	mm0, mm2
		add	edi, 8
		xchg	edi, ebp
		maskmovq	mm3, mm2
		add	edi, 8
		xchg	edi, ebp
	.ENDIF
	;
Label_Exit:
	mov	ebp, [ebx].dib.dwSaveRegEBP
	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	ret

eglRenderPoly@RenderPolygon_StoreMTSSE	ENDP

;
;	レンダリングフィル（RGBA）関数 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_StoreASSE	PROC	NEAR32 C

	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	mov	[ebx].dib.dwSaveRegEBP, ebp
	mov	esi, [ebx].pLineBuf[0]
	ASSUME	esi:PTR E3D_TRANS_LINE_BUF
	mov	ebp, [ebx].dib.ptrZBufLine
	mov	edi, [ebx].dib.ptrDstLine
	;
	mov	ecx, [ebx].nLineLeft[0]
	cmp	[ebx].nLeftDecimal, 0 ; LEFT_ERROR_RANGE
	seta	dl
	cmp	ecx, [ebx].dib.rectClip.left
	seta	dh
	lea	eax, [ecx + 1]
	or	dl, dh
	cmovnz	ecx, eax
	mov	eax, esi
	lea	edx, [esi + (SIZEOF E3D_TRANS_LINE_BUF)]
	cmovnz	eax, edx
	test	ecx, 1
	cmovz	esi, eax
	cmp	ecx, [ebx].nLineRight[0]
	ja	Label_Exit
	;
	lea	ebp, [ebp + ecx * 4]
	lea	edi, [edi + ecx * 4]
	;
	; 左側の端数ピクセルを出力
	;
;	movss	xmm7, xmmMaskSignFlag
;	movss	xmm6, [ebx].rRcpTxoxyr
;	movss	xmm2, xmmBiasDeltaZ
;	movss	xmm4, [esi].rZValue[0]
;	movss	xmm5, [esi].rZValue[4]
;	mulss	xmm2, xmm4
;	subss	xmm4, xmm5
;	shufps	xmm7, xmm7, 0
;	mulss	xmm5, xmm5
;	andps	xmm4, xmm7
;	mulss	xmm5, xmm6
;	movss	xmm6, [ebx].vTxxy.y
;		mulss	xmm4, xmmBiasScaleZ
;	mulss	xmm5, xmm6
;		addss	xmm4, xmm2
;	andps	xmm5, xmm7
;	mulss	xmm5, xmmBiasScaleZ
	pxor	mm7, mm7
;	addss	xmm4, xmm5
;	shufps	xmm4, xmm4, 0
	;
	.IF	(ecx & 01H) && ((SDWORD PTR ecx) <= [ebx].nLineRight[0])
		inc	ecx
;				movss	xmm0, [esi].rZValue[4]
		movd	mm0, DWORD PTR [edi]
		movd	mm1, DWORD PTR [esi].rgbMul[4]
;				addss	xmm0, xmm4
		movd	mm2, DWORD PTR [esi].rgbAdd[4]
		punpcklbw	mm0, mm7
		punpcklbw	mm1, mm7
		pmullw	mm0, mm1
			movq	mm3, MMWORD PTR [esi].rZValue[0]
;				movss	[ebx].dib.dwTemp[0], xmm0
		psrlw	mm0, 8
			movq	mm6, MMWORD PTR [esi].rgbMul[0]
				movd	mm4, DWORD PTR [ebp]
			pxor	mm6, mmxColorMask
;				pcmpgtd	mm4, MMWORD PTR [ebx].dib.dwTemp[0]
				pcmpgtd	mm4, MMWORD PTR [esi].rZValue[4]
			paddusb	mm6, mm2
			pand	mm6, mmxColorMask
		packuswb	mm0, mm7
			pcmpgtd	mm6, mm7
		paddusb	mm0, mm2
			pand	mm6, mm4
		;
		movd	mm1, DWORD PTR [edi]
		movd	mm2, DWORD PTR [ebp]
		pand	mm0, mm4
		pandn	mm4, mm1
		pand	mm3, mm6
		pandn	mm6, mm2
		por	mm0, mm4
		por	mm3, mm6
		movd	DWORD PTR [edi], mm0
		add	edi, 4
		movd	DWORD PTR [ebp], mm3
		add	ebp, 4
		add	esi, (SIZEOF E3D_TRANS_LINE_BUF)
	.ENDIF
	;
	; ピクセル連続出力
	;
	mov	edx, [ebx].nLineRight[0]
	inc	edx
	sub	edx, ecx
	sub	edx, 2
	.WHILE	!SIGN?
;				movlps	xmm0, QWORD PTR [esi].rZValue[0]
		movq	mm4, MMWORD PTR [edi]
		movq	mm1, MMWORD PTR [esi].rgbMul[0]
		movq	mm2, MMWORD PTR [esi].rZValue[0]
		;
		movq	mm5, mm4
;				addps	xmm0, xmm4
		punpcklbw	mm4, mm7
		movq	mm6, mm1
		punpcklbw	mm1, mm7
		punpckhbw	mm5, mm7
		pmullw	mm4, mm1
		punpckhbw	mm6, mm7
		pmullw	mm5, mm6
;			movq	mm6, MMWORD PTR [esi].rgbMul[0]
		psrlw	mm4, 8
;				movlps	QWORD PTR [ebx].dib.dwTemp[0], xmm0
		psrlw	mm5, 8
		movq	mm0, MMWORD PTR [esi].rgbAdd[0]
				movq	mm3, MMWORD PTR [ebp]
			pxor	mm6, mmxColorMask
		packuswb	mm4, mm5
			paddusb	mm6, mm0
;				pcmpgtd	mm3, MMWORD PTR [ebx].dib.dwTemp[0]
				pcmpgtd	mm3, QWORD PTR [esi].rZValue[0]
			pand	mm6, mmxColorMask
		paddusb	mm4, mm0
		;
			pcmpgtd	mm6, mm7
		pand	mm4, mm3
			pand	mm6, mm3
		pandn	mm3, MMWORD PTR [edi]
		pand	mm2, mm6
		pandn	mm6, MMWORD PTR [ebp]
		por	mm4, mm3
		por	mm2, mm6
		;
		movq	MMWORD PTR [edi], mm4
		add	edi, 8
		movq	MMWORD PTR [ebp], mm2
		add	ebp, 8
		add	esi, (SIZEOF E3D_TRANS_LINE_BUF)
		sub	edx, 2
	.ENDW
	;
	; 右側の端数ピクセルを出力
	;
	add	edx, 2
	.IF	(SDWORD PTR edx) >= 1
;				movss	xmm0, [esi].rZValue[0]
		movd	mm0, DWORD PTR [edi]
		movd	mm1, DWORD PTR [esi].rgbMul[0]
		movq	mm2, MMWORD PTR [esi].rgbAdd[0]
;				addss	xmm0, xmm4
		punpcklbw	mm0, mm7
		punpcklbw	mm1, mm7
		pmullw	mm0, mm1
			movq	mm1, MMWORD PTR [esi].rgbMul[0]
			movq	mm3, MMWORD PTR [esi].rZValue[0]
;				movss	[ebx].dib.dwTemp[0], xmm0
		psrlw	mm0, 8
			pxor	mm1, mmxColorMask
				movd	mm4, DWORD PTR [ebp]
			paddusb	mm1, mm2
		packuswb	mm0, mm7
;				pcmpgtd	mm4, MMWORD PTR [ebx].dib.dwTemp[0]
				pcmpgtd	mm4, MMWORD PTR [esi].rZValue[0]
			pand	mm1, mmxColorMask
				psllq	mm4, 32
			pcmpgtd	mm1, mm7
				psrlq	mm4, 32
		paddusb	mm0, mm2
		;
			pand	mm1, mm4
		pand	mm0, mm4
		pandn	mm4, MMWORD PTR [edi]
		pand	mm3, mm1
		pandn	mm1, MMWORD PTR [ebp]
		por	mm0, mm4
		por	mm3, mm1
		movd	DWORD PTR [edi], mm0
		movd	DWORD PTR [ebp], mm3
	.ENDIF
	;
Label_Exit:
	mov	ebp, [ebx].dib.dwSaveRegEBP
	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	ret

eglRenderPoly@RenderPolygon_StoreASSE	ENDP

;
;	レンダリングフィル（RGBA, z比較のみ）関数 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_StoreARZSSE	PROC	NEAR32 C

	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	mov	[ebx].dib.dwSaveRegEBP, ebp
	mov	esi, [ebx].pLineBuf[0]
	ASSUME	esi:PTR E3D_TRANS_LINE_BUF
	mov	ebp, [ebx].dib.ptrZBufLine
	mov	edi, [ebx].dib.ptrDstLine
	;
	mov	ecx, [ebx].nLineLeft[0]
	cmp	[ebx].nLeftDecimal, 0 ; LEFT_ERROR_RANGE
	seta	dl
	cmp	ecx, [ebx].dib.rectClip.left
	seta	dh
	lea	eax, [ecx + 1]
	or	dl, dh
	cmovnz	ecx, eax
	mov	eax, esi
	lea	edx, [esi + (SIZEOF E3D_TRANS_LINE_BUF)]
	cmovnz	eax, edx
	test	ecx, 1
	cmovz	esi, eax
	cmp	ecx, [ebx].nLineRight[0]
	ja	Label_Exit
	;
	lea	ebp, [ebp + ecx * 4]
	lea	edi, [edi + ecx * 4]
	;
	; 左側の端数ピクセルを出力
	;
	pxor	mm7, mm7
	;
	.IF	(ecx & 01H) && ((SDWORD PTR ecx) <= [ebx].nLineRight[0])
		inc	ecx
		movd	mm0, DWORD PTR [edi]
		movd	mm1, DWORD PTR [esi].rgbMul[4]
		movd	mm2, DWORD PTR [esi].rgbAdd[4]
		punpcklbw	mm0, mm7
		punpcklbw	mm1, mm7
		pmullw	mm0, mm1
			movq	mm3, MMWORD PTR [esi].rZValue[0]
		psrlw	mm0, 8
			movq	mm6, MMWORD PTR [esi].rgbMul[0]
				movd	mm4, DWORD PTR [ebp]
			pxor	mm6, mmxColorMask
				paddd	mm4, mmxBiasZ
			paddusb	mm6, mm2
				pcmpgtd	mm4, MMWORD PTR [esi].rZValue[4]
			pand	mm6, mmxColorMask
		packuswb	mm0, mm7
			pcmpgtd	mm6, mm7
		paddusb	mm0, mm2
			pand	mm6, mm4
		;
		movd	mm1, DWORD PTR [edi]
;		movd	mm2, DWORD PTR [ebp]
		pand	mm0, mm4
		pandn	mm4, mm1
;		pand	mm3, mm6
;		pandn	mm6, mm2
		por	mm0, mm4
;		por	mm3, mm6
		movd	DWORD PTR [edi], mm0
		add	edi, 4
;		movd	DWORD PTR [ebp], mm3
		add	ebp, 4
		add	esi, (SIZEOF E3D_TRANS_LINE_BUF)
	.ENDIF
	;
	; ピクセル連続出力
	;
	mov	edx, [ebx].nLineRight[0]
	inc	edx
	sub	edx, ecx
	sub	edx, 2
	.WHILE	!SIGN?
		movq	mm4, MMWORD PTR [edi]
		movq	mm1, MMWORD PTR [esi].rgbMul[0]
		movq	mm2, MMWORD PTR [esi].rZValue[0]
		;
		movq	mm5, mm4
		punpcklbw	mm4, mm7
		movq	mm6, mm1
		punpcklbw	mm1, mm7
		punpckhbw	mm5, mm7
		pmullw	mm4, mm1
		punpckhbw	mm6, mm7
		pmullw	mm5, mm6
		psrlw	mm4, 8
		psrlw	mm5, 8
		movq	mm0, MMWORD PTR [esi].rgbAdd[0]
				movq	mm3, MMWORD PTR [ebp]
			pxor	mm6, mmxColorMask
		packuswb	mm4, mm5
				paddd	mm3, mmxBiasZ
			paddusb	mm6, mm0
				pcmpgtd	mm3, QWORD PTR [esi].rZValue[0]
			pand	mm6, mmxColorMask
		paddusb	mm4, mm0
		;
			pcmpgtd	mm6, mm7
		pand	mm4, mm3
			pand	mm6, mm3
		pandn	mm3, MMWORD PTR [edi]
;		pand	mm2, mm6
;		pandn	mm6, MMWORD PTR [ebp]
		por	mm4, mm3
;		por	mm2, mm6
		;
		movq	MMWORD PTR [edi], mm4
		add	edi, 8
;		movq	MMWORD PTR [ebp], mm2
		add	ebp, 8
		add	esi, (SIZEOF E3D_TRANS_LINE_BUF)
		sub	edx, 2
	.ENDW
	;
	; 右側の端数ピクセルを出力
	;
	add	edx, 2
	.IF	(SDWORD PTR edx) >= 1
		movd	mm0, DWORD PTR [edi]
		movd	mm1, DWORD PTR [esi].rgbMul[0]
		movq	mm2, MMWORD PTR [esi].rgbAdd[0]
		punpcklbw	mm0, mm7
		punpcklbw	mm1, mm7
		pmullw	mm0, mm1
			movq	mm1, MMWORD PTR [esi].rgbMul[0]
			movq	mm3, MMWORD PTR [esi].rZValue[0]
		psrlw	mm0, 8
			pxor	mm1, mmxColorMask
				movd	mm4, DWORD PTR [ebp]
			paddusb	mm1, mm2
				paddd	mm4, mmxBiasZ
		packuswb	mm0, mm7
				pcmpgtd	mm4, MMWORD PTR [esi].rZValue[0]
			pand	mm1, mmxColorMask
				psllq	mm4, 32
			pcmpgtd	mm1, mm7
				psrlq	mm4, 32
		paddusb	mm0, mm2
		;
;			pand	mm1, mm4
		pand	mm0, mm4
		pandn	mm4, MMWORD PTR [edi]
;		pand	mm3, mm1
;		pandn	mm1, MMWORD PTR [ebp]
		por	mm0, mm4
		por	mm3, mm1
		movd	DWORD PTR [edi], mm0
;		movd	DWORD PTR [ebp], mm3
	.ENDIF
	;
Label_Exit:
	mov	ebp, [ebx].dib.dwSaveRegEBP
	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	ret

eglRenderPoly@RenderPolygon_StoreARZSSE	ENDP

;
;	レンダリングフィル（RGBA,z-buf 無比較）関数 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_StoreANZSSE	PROC	NEAR32 C

	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	mov	esi, [ebx].pLineBuf[0]
	ASSUME	esi:PTR E3D_TRANS_LINE_BUF
	mov	edi, [ebx].dib.ptrDstLine
	;
	mov	ecx, [ebx].nLineLeft[0]
	cmp	[ebx].nLeftDecimal, 0 ; LEFT_ERROR_RANGE
	seta	dl
	cmp	ecx, [ebx].dib.rectClip.left
	seta	dh
	lea	eax, [ecx + 1]
	or	dl, dh
	cmovnz	ecx, eax
	mov	eax, esi
	lea	edx, [esi + (SIZEOF E3D_TRANS_LINE_BUF)]
	cmovnz	eax, edx
	test	ecx, 1
	cmovz	esi, eax
	cmp	ecx, [ebx].nLineRight[0]
	ja	Label_Exit
	;
	mov	edx, [ebx].nLineRight[0]
	lea	edi, [edi + ecx * 4]
	sub	edx, ecx
	js	Label_Exit
	inc	edx
	;
	; 左側の端数ピクセルを出力
	;
	pcmpeqw	mm6, mm6
	pxor	mm7, mm7
	.IF	ecx & 01H
		movd	mm1, DWORD PTR [esi].rgbMul[4]
		movd	mm0, DWORD PTR [edi]
		punpcklbw	mm1, mm7
		punpcklbw	mm0, mm7
		psubw	mm1, mm6
		pmullw	mm0, mm1
		movd	mm2, DWORD PTR [esi].rgbAdd[4]
		psrlw	mm0, 8
		packuswb	mm0, mm7
		paddusb	mm0, mm2
		movd	DWORD PTR [edi], mm0
		add	edi, 4
		add	esi, (SIZEOF E3D_TRANS_LINE_BUF)
		dec	edx
	.ENDIF
	;
	; ピクセル連続出力
	;
	movq	mm2, mm6
	sub	edx, 2
	.WHILE	!SIGN?
		movq	mm4, MMWORD PTR [edi]
		movq	mm1, MMWORD PTR [esi].rgbMul[0]
		;
		movq	mm5, mm4
		punpcklbw	mm4, mm7
		movq	mm6, mm1
		punpcklbw	mm1, mm7
		punpckhbw	mm5, mm7
		psubw	mm1, mm2
		pmullw	mm4, mm1
		punpckhbw	mm6, mm7
		psubw	mm6, mm2
		pmullw	mm5, mm6
		movq	mm0, MMWORD PTR [esi].rgbAdd[0]
		psrlw	mm4, 8
		psrlw	mm5, 8
		packuswb	mm4, mm5
		paddusb	mm4, mm0
		movq	MMWORD PTR [edi], mm4
		add	edi, 8
		add	esi, (SIZEOF E3D_TRANS_LINE_BUF)
		sub	edx, 2
	.ENDW
	;
	; 右側の端数ピクセルを出力
	;
	add	edx, 2
	.IF	(SDWORD PTR edx) >= 1
		movd	mm0, DWORD PTR [edi]
		movd	mm1, DWORD PTR [esi].rgbMul[0]
		punpcklbw	mm0, mm7
		punpcklbw	mm1, mm7
		psubw	mm1, mm2
		pmullw	mm0, mm1
		movq	mm2, MMWORD PTR [esi].rgbAdd[0]
		psrlw	mm0, 8
		packuswb	mm0, mm7
		paddusb	mm0, mm2
		movd	DWORD PTR [edi], mm0
	.ENDIF
	;
Label_Exit:
	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	ret

eglRenderPoly@RenderPolygon_StoreANZSSE	ENDP

;
;	レンダリング透明度付フィル（RGBA）関数 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_StoreATSSE	PROC	NEAR32 C

	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	mov	[ebx].dib.dwSaveRegEBP, ebp
	mov	esi, [ebx].pLineBuf[0]
	ASSUME	esi:PTR E3D_TRANS_LINE_BUF
	mov	ebp, [ebx].dib.ptrZBufLine
	mov	edi, [ebx].dib.ptrDstLine
	;
	mov	ecx, [ebx].nLineLeft[0]
	cmp	[ebx].nLeftDecimal, 0 ; LEFT_ERROR_RANGE
	seta	dl
	cmp	ecx, [ebx].dib.rectClip.left
	seta	dh
	lea	eax, [ecx + 1]
	or	dl, dh
	cmovnz	ecx, eax
	mov	eax, esi
	lea	edx, [esi + (SIZEOF E3D_TRANS_LINE_BUF)]
	cmovnz	eax, edx
	test	ecx, 1
	cmovz	esi, eax
	cmp	ecx, [ebx].nLineRight[0]
	ja	Label_Exit
	;
	lea	ebp, [ebp + ecx * 4]
	lea	edi, [edi + ecx * 4]
	;
	; 左側の端数ピクセルを出力
	;
;		movss	xmm7, xmmMaskSignFlag
	mov	eax, 100H
;		movss	xmm6, xmmBiasZ
	sub	eax, DWORD PTR [ebx].nTextureApply
;		movss	xmm5, xmmBiasScaleZ
	movd	mm6, eax
;		shufps	xmm7, xmm7, 0
	pshufw	mm6, mm6, 0
;		shufps	xmm6, xmm6, 0
	pxor	mm7, mm7
	psrlw	mm6, 1
	.IF	(ecx & 01H) && ((SDWORD PTR ecx) <= [ebx].nLineRight[0])
		inc	ecx
;				movss	xmm0, [esi].rZValue[0]
				movss	xmm1, [esi].rZValue[4]
		sub	edi, 4
		sub	ebp, 4
;				subss	xmm0, xmm1
		movq	mm1, MMWORD PTR [esi].rgbMul[0]
		pcmpeqb	mm0, mm0
		movq	mm4, MMWORD PTR [edi]
;				andps	xmm0, xmm7
		;
		pxor	mm0, mm1
			movq	mm5, mm4
;				mulss	xmm0, xmm5
		movq	mm1, mm0
			punpcklbw	mm4, mm7
		punpcklbw	mm0, mm7
			punpckhbw	mm5, mm7
		punpckhbw	mm1, mm7
;				mulss	xmm1, xmm6
		;
		pmullw	mm0, mm4
			movq	mm2, MMWORD PTR [esi].rgbAdd[0]
;				addss	xmm0, xmm1
				movss	xmm2, REAL4 PTR [ebp + 4]
		pmullw	mm1, mm5
			movq	mm3, mm2
			punpcklbw	mm2, mm7
			punpckhbw	mm3, mm7
;				cmpss	xmm0, xmm2, 1
				cmpss	xmm1, xmm2, 1
		psrlw	mm0, 8
		psrlw	mm1, 8
				movss	[ebx].dib.dwTemp[0], xmm1
		;
		psubsw	mm2, mm0
			movq	mm0, MMWORD PTR [esi].rZValue[0]
		psubsw	mm3, mm1
		pmullw	mm2, mm6
		pmullw	mm3, mm6
		;
		psraw	mm2, 7
		psraw	mm3, 7
		paddsw	mm2, mm4
				movq	mm4, MMWORD PTR [esi].rgbMul[0]
			movd	mm1, [ebx].dib.dwTemp[0]
		paddsw	mm3, mm5
				pxor	mm4, mmxColorMask
			psllq	mm1, 32
				paddusb	mm4, MMWORD PTR [esi].rgbAdd[0]
		packuswb	mm2, mm3
				pand	mm4, mmxColorMask
				pcmpgtd	mm4, mm7
		;
		maskmovq	mm2, mm1
			pand	mm4, mm1
		add	edi, 8
		xchg	edi, ebp
		maskmovq	mm0, mm4
		add	edi, 8
		xchg	edi, ebp
		add	esi, (SIZEOF E3D_TRANS_LINE_BUF)
	.ENDIF
	;
	; ピクセル連続出力
	;
	mov	edx, [ebx].nLineRight[0]
	inc	edx
	sub	edx, ecx
	sub	edx, 2
	.WHILE	!SIGN?
				movlps	xmm0, QWORD PTR [esi].rZValue[0]
		movq	mm1, MMWORD PTR [esi].rgbMul[0]
		pcmpeqb	mm0, mm0
		movq	mm4, MMWORD PTR [edi]
;				movaps	xmm1, xmm0
;				shufps	xmm0, xmm0, 1
		;
		pxor	mm0, mm1
			movq	mm5, mm4
;				subss	xmm0, xmm1
		movq	mm1, mm0
			punpcklbw	mm4, mm7
		punpcklbw	mm0, mm7
;				andps	xmm0, xmm7
			punpckhbw	mm5, mm7
;				mulps	xmm1, xmm6
				movlps	xmm2, QWORD PTR [ebp]
		punpckhbw	mm1, mm7
		;
		pmullw	mm0, mm4
;				mulss	xmm0, xmm5
			movq	mm2, MMWORD PTR [esi].rgbAdd[0]
		pmullw	mm1, mm5
			movq	mm3, mm2
;				shufps	xmm0, xmm0, 0
			punpcklbw	mm2, mm7
;				addps	xmm0, xmm1
			punpckhbw	mm3, mm7
		psrlw	mm0, 8
		psrlw	mm1, 8
		;
		psubsw	mm2, mm0
				cmpps	xmm0, xmm2, 1
			movq	mm0, MMWORD PTR [esi].rZValue[0]
		psubsw	mm3, mm1
		pmullw	mm2, mm6
		pmullw	mm3, mm6
		;
				movlps	QWORD PTR [ebx].dib.dwTemp[0], xmm0
		psraw	mm2, 7
		psraw	mm3, 7
		paddsw	mm2, mm4
			movq	mm4, MMWORD PTR [esi].rgbMul[0]
				movq	mm1, QWORD PTR [ebx].dib.dwTemp[0]
		paddsw	mm3, mm5
			pxor	mm4, mmxColorMask
		packuswb	mm2, mm3
			paddusb	mm4, MMWORD PTR [esi].rgbAdd[0]
		;
		maskmovq	mm2, mm1
			pand	mm4, mmxColorMask
			pcmpgtd	mm4, mm7
		add	edi, 8
			pand	mm4, mm1
		xchg	edi, ebp
		maskmovq	mm0, mm4
		add	edi, 8
		xchg	edi, ebp
		add	esi, (SIZEOF E3D_TRANS_LINE_BUF)
		sub	edx, 2
	.ENDW
	;
	; 右側の端数ピクセルを出力
	;
	add	edx, 2
	.IF	(SDWORD PTR edx) >= 1
				movss	xmm0, [esi].rZValue[0]
;				movss	xmm1, [esi].rZValue[4]
		movq	mm1, MMWORD PTR [esi].rgbMul[0]
		pcmpeqb	mm0, mm0
		movq	mm4, MMWORD PTR [edi]
;				subss	xmm1, xmm0
		;
		pxor	mm0, mm1
			movq	mm5, mm4
		movq	mm1, mm0
			punpcklbw	mm4, mm7
;				andps	xmm1, xmm7
		punpcklbw	mm0, mm7
			punpckhbw	mm5, mm7
		punpckhbw	mm1, mm7
;				mulss	xmm1, xmm5
				movss	xmm2, REAL4 PTR [ebp]
		;
		pmullw	mm0, mm4
			movq	mm2, MMWORD PTR [esi].rgbAdd[0]
;				mulss	xmm0, xmm6
		pmullw	mm1, mm5
			movq	mm3, mm2
			punpcklbw	mm2, mm7
			punpckhbw	mm3, mm7
;				addss	xmm0, xmm1
		psrlw	mm0, 8
		psrlw	mm1, 8
		;
		psubsw	mm2, mm0
			movq	mm0, MMWORD PTR [esi].rZValue[0]
		psubsw	mm3, mm1
				cmpss	xmm0, xmm2, 1
		pmullw	mm2, mm6
		pmullw	mm3, mm6
				movq	mm6, MMWORD PTR [esi].rgbMul[0]
		;
		psraw	mm2, 7
				movss	[ebx].dib.dwTemp[0], xmm0
		psraw	mm3, 7
		paddsw	mm2, mm4
				pxor	mm6, mmxColorMask
		paddsw	mm3, mm5
				movd	mm1, [ebx].dib.dwTemp[0]
				paddusb	mm6, MMWORD PTR [esi].rgbAdd[0]
		packuswb	mm2, mm3
				pand	mm6, mmxColorMask
				pcmpgtd	mm6, mm7
		;
		maskmovq	mm2, mm1
				pand	mm6, mm1
		add	edi, 8
		xchg	edi, ebp
		maskmovq	mm0, mm6
		add	edi, 8
		xchg	edi, ebp
		add	esi, (SIZEOF E3D_TRANS_LINE_BUF)
		sub	edx, 2
	.ENDIF
	;
Label_Exit:
	mov	ebp, [ebx].dib.dwSaveRegEBP
	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	ret

eglRenderPoly@RenderPolygon_StoreATSSE	ENDP

;
;	アンチエイリアス関数
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_AntialiasSSE	PROC	NEAR32 C

	sfence

	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	mov	[ebx].dib.dwSaveRegEBP, ebp
	mov	esi, [ebx].pLineBuf[0]
	ASSUME	esi:PTR E3D_TRANS_LINE_BUF
	mov	ebp, [ebx].dib.ptrZBufLine
	mov	edi, [ebx].dib.ptrDstLine
	;
	mov	ecx, [ebx].nLineLeft[0]
	inc	ecx
	.IF	(SDWORD PTR ecx) <= (SDWORD PTR [ebx].nLineRight[0])
		;
		; 左側アンチエイリアス
		;
		.IF	(SDWORD PTR ecx) > [ebx].dib.rectClip.left
			lea	ebp, [ebp + ecx * 4]
			lea	edi, [edi + ecx * 4 - 4]
			;
			mov	edx, ecx
			and	ecx, NOT 01H
			sub	ecx, [ebx].nLineLeft[4]
			mov	eax, [ebx].nLeftDecimal
			shr	ecx, 1
			IF	(SIZEOF E3D_TRANS_LINE_BUF) NE 24
				.ERR
			ENDIF
			xor	eax, 0FFFFH
			lea	ecx, [ecx + ecx * 2]
			and	edx, 01H
			lea	esi, [esi + ecx * 8]
			mov	ecx, DWORD PTR [esi].rZValue[edx * 4]
			shr	eax, 8+1
			.IF	DWORD PTR [ebp] == ecx
				movd	mm4, eax
				pxor	mm7, mm7
				movd	mm1, DWORD PTR [edi]
				movd	mm0, DWORD PTR [esi].rgbAdd[edx * 4]
				punpcklbw	mm1, mm7
				punpcklbw	mm0, mm7
				pshufw	mm4, mm4, 0
				psubsw	mm0, mm1
				pmullw	mm0, mm4
				psllw	mm1, 7
				paddsw	mm0, mm1
				psraw	mm0, 7
				packuswb	mm0, mm7
				movd	DWORD PTR [edi], mm0
			.ENDIF
		.ENDIF
		;
		; 右側アンチエイリアス
		;
		mov	ebp, [ebx].dib.ptrZBufLine
		mov	edi, [ebx].dib.ptrDstLine
		mov	esi, [ebx].pLineBuf[0]
		mov	ecx, [ebx].nLineRight[0]
		mov	eax, [ebx].nRightDecimal
		lea	ebp, [ebp + ecx * 4]
		lea	edi, [edi + ecx * 4 + 4]
		;
		.IF	ecx < [ebx].dib.rectClip.right
			mov	edx, ecx
			and	ecx, NOT 01H
			sub	ecx, [ebx].nLineLeft[4]
			shr	ecx, 1
			IF	(SIZEOF E3D_TRANS_LINE_BUF) NE 24
				.ERR
			ENDIF
			lea	ecx, [ecx + ecx * 2]
			and	edx, 01H
			lea	esi, [esi + ecx * 8]
			mov	ecx, DWORD PTR [esi].rZValue[edx * 4]
			shr	eax, 8+1
			.IF	DWORD PTR [ebp] == ecx
				movd	mm4, eax
				pxor	mm7, mm7
				movd	mm1, DWORD PTR [edi]
				movd	mm0, DWORD PTR [esi].rgbAdd[edx * 4]
				punpcklbw	mm1, mm7
				punpcklbw	mm0, mm7
				pshufw	mm4, mm4, 0
				psubsw	mm0, mm1
				pmullw	mm0, mm4
				psllw	mm1, 7
				paddsw	mm0, mm1
				psraw	mm0, 7
				packuswb	mm0, mm7
				movd	DWORD PTR [edi], mm0
			.ENDIF
		.ENDIF
	.ELSE
		;
		; １ピクセル未満のアンチエイリアス
		;
		movss	xmm0, [esi].rZValue[0]
		movss	xmm1, [esi].rZValue[4]
		maxss	xmm0, xmm1
		mov	ecx, [ebx].nLineLeft[0]
		shufps	xmm0, xmm0, 0
		movlps	QWORD PTR [esi].rZValue, xmm0
		;
		.IF	(SDWORD PTR ecx) > [ebx].dib.rectClip.left
			lea	ebp, [ebp + ecx * 4]
			lea	edi, [edi + ecx * 4]
			;
			mov	edx, ecx
			and	ecx, NOT 01H
			sub	ecx, [ebx].nLineLeft[4]
			mov	eax, [ebx].nLeftDecimal
			shr	ecx, 1
			IF	(SIZEOF E3D_TRANS_LINE_BUF) NE 24
				.ERR
			ENDIF
			xor	eax, 0FFFFH
			lea	ecx, [ecx + ecx * 2]
			and	edx, 01H
			lea	esi, [esi + ecx * 8]
			mov	ecx, DWORD PTR [esi].rZValue[edx * 4]
			shr	eax, 8+1
			.IF	DWORD PTR [ebp] > ecx
				movd	mm4, eax
				pxor	mm7, mm7
				movd	mm1, DWORD PTR [edi]
				movd	mm0, DWORD PTR [esi].rgbAdd[edx * 4]
				punpcklbw	mm1, mm7
				punpcklbw	mm0, mm7
				pshufw	mm4, mm4, 0
				psubsw	mm0, mm1
				pmullw	mm0, mm4
				psllw	mm1, 7
				paddsw	mm0, mm1
				psraw	mm0, 7
				packuswb	mm0, mm7
				movd	DWORD PTR [edi], mm0
			.ENDIF
		.ENDIF
		;
		mov	ebp, [ebx].dib.ptrZBufLine
		mov	edi, [ebx].dib.ptrDstLine
		mov	esi, [ebx].pLineBuf[0]
		mov	eax, [ebx].nRightDecimal
		mov	ecx, [ebx].nLineRight[0]
		lea	ebp, [ebp + ecx * 4 + 4]
		lea	edi, [edi + ecx * 4 + 4]
		;
		.IF	ecx < [ebx].dib.rectClip.right
			mov	edx, ecx
			and	ecx, NOT 01H
			sub	ecx, [ebx].nLineLeft[4]
			shr	ecx, 1
			IF	(SIZEOF E3D_TRANS_LINE_BUF) NE 24
				.ERR
			ENDIF
			lea	ecx, [ecx + ecx * 2]
			and	edx, 01H
			lea	esi, [esi + ecx * 8]
			mov	ecx, DWORD PTR [esi].rZValue[edx * 4]
			shr	eax, 8+1
			.IF	DWORD PTR [ebp] > ecx
				movd	mm4, eax
				pxor	mm7, mm7
				movd	mm1, DWORD PTR [edi]
				movd	mm0, DWORD PTR [esi].rgbAdd[edx * 4]
				punpcklbw	mm1, mm7
				punpcklbw	mm0, mm7
				pshufw	mm4, mm4, 0
				psubsw	mm0, mm1
				pmullw	mm0, mm4
				psllw	mm1, 7
				paddsw	mm0, mm1
				psraw	mm0, 7
				packuswb	mm0, mm7
				movd	DWORD PTR [edi], mm0
			.ENDIF
		.ENDIF
	.ENDIF

	mov	ebp, [ebx].dib.dwSaveRegEBP
	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	ret

eglRenderPoly@RenderPolygon_AntialiasSSE	ENDP


CodeSeg	ENDS

	END
