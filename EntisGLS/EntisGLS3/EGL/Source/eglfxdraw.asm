
; ----------------------------------------------------------------------------
;                          Entis Graphic Library
; ----------------------------------------------------------------------------
;   Copyright (c) 2004-2007 Leshade Entis, Entis-soft. Al rights reserved.
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
TableScale5to8	LABEL	BYTE
	@INDEX = 0
	REPEAT	20H
		BYTE	@INDEX * 0FFH / 1FH
		@INDEX = @INDEX + 1
	ENDM

TableDivScale8	LABEL	DWORD
	@INDEX = 0FFH
	REPEAT	100H
		@SUBIDX = @INDEX
		IF	@SUBIDX EQ 0
			@SUBIDX = 1
		ENDIF
		DWORD	4000H / @SUBIDX
		@INDEX = @INDEX - 1
	ENDM

ALIGN	10H
mmxConstFF000000	LABEL	QWORD
		DWORD	4 DUP( 0FF000000H )
mmxConst00FFFFFF	LABEL	QWORD
		DWORD	4 DUP( 00FFFFFFH )
mmxMaskLow3Words	LABEL	QWORD
		WORD	-1, -1, -1, 0
		WORD	-1, -1, -1, 0
mmxHighWord_100		LABEL	QWORD
		WORD	0, 0, 0, 100H
		WORD	0, 0, 0, 100H
mmxQWord_1	LABEL	QWORD
		QWORD	1
		QWORD	1
mmxDWord_1	LABEL	QWORD
		DWORD	1, 1
		DWORD	1, 1
mmxDWord00010000	LABEL	QWORD
		DWORD	00010000H, 00010000H
		DWORD	00010000H, 00010000H

ConstSeg	ENDS


; ----------------------------------------------------------------------------
;	コードセグメント
; ----------------------------------------------------------------------------

CodeSeg	SEGMENT	PARA READONLY FLAT 'CODE'


; ----------------------------------------------------------------------------
;	サンプリング関数
; ----------------------------------------------------------------------------
; レジスタ；
;	ebx : HEGL_DRAW_IMAGE
;	esi : 入力画像アドレス（通常描画の場合のみ）
;	ecx : ピクセル数
;	ebx, esp 以外の全ての汎用レジスタは破壊してもよい
; ----------------------------------------------------------------------------

	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF

;
;	256 色画像をサンプリング
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@Sample_Index8:
	mov	ebp, [ebx].srcimg.dwClippedPixel
	.IF	!([ebx].srcimg.fdwFormatType & EIF_WITH_CLIPPING)
		mov	ebp, -1
	.ENDIF
	mov	edx, [ebx].srcimg.pPaletteEntries
	mov	edi, [ebx].pDrawLineBuf[0]
	test	ecx, ecx
	.WHILE	!ZERO?
		movzx	eax, BYTE PTR [esi]
		inc	esi
		.IF	eax == ebp
			xor	eax, eax
		.ELSE
			mov	eax, DWORD PTR [edx + eax * 4]
			or	eax, 0FF000000H
		.ENDIF
		mov	DWORD PTR [edi], eax
		add	edi, 4
		dec	ecx
	.ENDW
	ret

;
;	8 ビットグレイスケール画像をサンプリング
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@Sample_Gray8:
	mov	edi, [ebx].pDrawLineBuf[0]
	test	ecx, ecx
	.WHILE	!ZERO?
		movzx	eax, BYTE PTR [esi]
		inc	esi
		mov	eax, DWORD PTR [ebx].rgbaColor[eax * 4]
		mov	DWORD PTR [edi], eax
		add	edi, 4
		dec	ecx
	.ENDW
	ret

;
;	RGB-16 画像をサンプリング
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@Sample_RGB16:
	mov	edi, [ebx].pDrawLineBuf[0]
	test	ecx, ecx
	.WHILE	!ZERO?
		movzx	eax, WORD PTR [esi]
		add	esi, 2
		mov	edx, eax
		mov	ebp, eax
		and	eax, 1FH
		shr	edx, 5
		shr	ebp, 10
		mov	al, TableScale5to8[eax]
		and	edx, 1FH
		and	ebp, 1FH
		mov	BYTE PTR [edi], al
		mov	al, TableScale5to8[edx]
		mov	ah, TableScale5to8[ebp]
		mov	BYTE PTR [edi + 1], al
		mov	BYTE PTR [edi + 2], ah
		mov	BYTE PTR [edi + 3], 0FFH
		add	edi, 4
		dec	ecx
	.ENDW
	ret

;
;	RGB-24 画像をサンプリング
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@Sample_RGB24:
	mov	edi, [ebx].pDrawLineBuf[0]
	test	ecx, ecx
	.WHILE	!ZERO?
		mov	al, BYTE PTR [esi]
		mov	ah, BYTE PTR [esi + 1]
		mov	dl, BYTE PTR [esi + 2]
		add	esi, 3
		mov	BYTE PTR [edi], al
		mov	BYTE PTR [edi + 1], ah
		mov	BYTE PTR [edi + 2], dl
		mov	BYTE PTR [edi + 3], 0FFH
		add	edi, 4
		dec	ecx
	.ENDW
	ret

;
;	RGB-32 画像をサンプリング
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@Sample_RGB32:
	xor	ebp, ebp
	mov	edi, [ebx].pDrawLineBuf[0]
	.IF	!([ebx].srcimg.fdwFormatType & EIF_WITH_ALPHA)
		mov	ebp, 0FF000000H
	.ENDIF
	sub	ecx, 2
	.WHILE	!SIGN?
		mov	eax, DWORD PTR [esi]
		mov	edx, DWORD PTR [esi + 4]
		add	esi, 8
		or	eax, ebp
		or	edx, ebp
		mov	DWORD PTR [edi], eax
		mov	DWORD PTR [edi + 4], edx
		add	edi, 8
		sub	ecx, 2
	.ENDW
	add	ecx, 2
	.IF	!ZERO?
		mov	eax, DWORD PTR [esi]
		or	eax, ebp
		mov	DWORD PTR [edi], eax
	.ENDIF
	ret

;
;	256 色画像を変形サンプリング
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@Sample_Index8_X:
	mov	ebp, [ebx].srcimg.dwClippedPixel
	.IF	!([ebx].srcimg.fdwFormatType & EIF_WITH_CLIPPING)
		mov	ebp, -1
	.ENDIF
	mov	edi, [ebx].pDrawLineBuf[0]
	test	ecx, ecx
	mov	edx, [ebx].ptBasePos.y
	mov	eax, [ebx].ptBasePos.x
	.WHILE	!ZERO?
		mov	esi, [ebx].pSrcLineAddr
		sar	edx, 16
		sar	eax, 16
		;
		.IF	edx >= [ebx].srcimg.dwImageHeight
			sar	edx, 31
			not	edx
			and	edx, [ebx].srcimg.dwImageHeight
			sub	edx, 1
			adc	edx, 0
		.ENDIF
		.IF	eax >= [ebx].srcimg.dwImageWidth
			sar	eax, 31
			not	eax
			and	eax, [ebx].srcimg.dwImageWidth
			sub	eax, 1
			adc	eax, 0
		.ENDIF
		mov	esi, DWORD PTR [esi + edx * 4]
		movzx	eax, BYTE PTR [esi + eax]
		mov	edx, [ebx].srcimg.pPaletteEntries
		cmp	eax, ebp
		.IF	!ZERO?
			mov	eax, DWORD PTR [edx + eax * 4]
			or	eax, 0FF000000H
		.ELSE
			xor	eax, eax
		.ENDIF
		mov	DWORD PTR [edi], eax
		;
		add	edi, 4
		mov	edx, [ebx].ptBasePos.y
		mov	eax, [ebx].ptBasePos.x
		add	edx, [ebx].ptDltScanX.y
		add	eax, [ebx].ptDltScanX.x
		mov	[ebx].ptBasePos.y, edx
		mov	[ebx].ptBasePos.x, eax
		dec	ecx
	.ENDW
	ret

;
;	グレイスケール画像を変形サンプリング
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@Sample_Gray8_X:
	mov	edi, [ebx].pDrawLineBuf[0]
	test	ecx, ecx
	mov	edx, [ebx].ptBasePos.y
	mov	eax, [ebx].ptBasePos.x
	.WHILE	!ZERO?
		mov	esi, [ebx].pSrcLineAddr
		sar	edx, 16
		sar	eax, 16
		;
		.IF	edx >= [ebx].srcimg.dwImageHeight
			sar	edx, 31
			not	edx
			and	edx, [ebx].srcimg.dwImageHeight
			sub	edx, 1
			adc	edx, 0
		.ENDIF
		.IF	eax >= [ebx].srcimg.dwImageWidth
			sar	eax, 31
			not	eax
			and	eax, [ebx].srcimg.dwImageWidth
			sub	eax, 1
			adc	eax, 0
		.ENDIF
		mov	esi, DWORD PTR [esi + edx * 4]
		movzx	eax, BYTE PTR [esi + eax]
		mov	eax, DWORD PTR [ebx].rgbaColor[eax * 4]
		;
		mov	DWORD PTR [edi], eax
		add	edi, 4
		mov	edx, [ebx].ptBasePos.y
		mov	eax, [ebx].ptBasePos.x
		add	edx, [ebx].ptDltScanX.y
		add	eax, [ebx].ptDltScanX.x
		mov	[ebx].ptBasePos.y, edx
		mov	[ebx].ptBasePos.x, eax
		dec	ecx
	.ENDW
	ret

;
;	RGB-16 画像を変形サンプリング
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@Sample_RGB16_X:
	mov	edi, [ebx].pDrawLineBuf[0]
	test	ecx, ecx
	mov	edx, [ebx].ptBasePos.y
	mov	eax, [ebx].ptBasePos.x
	.WHILE	!ZERO?
		mov	esi, [ebx].pSrcLineAddr
		sar	edx, 16
		sar	eax, 16
		;
		.IF	edx >= [ebx].srcimg.dwImageHeight
			sar	edx, 31
			not	edx
			and	edx, [ebx].srcimg.dwImageHeight
			sub	edx, 1
			adc	edx, 0
		.ENDIF
		.IF	eax >= [ebx].srcimg.dwImageWidth
			sar	eax, 31
			not	eax
			and	eax, [ebx].srcimg.dwImageWidth
			sub	eax, 1
			adc	eax, 0
		.ENDIF
		mov	esi, DWORD PTR [esi + edx * 4]
		movzx	eax, WORD PTR [esi + eax * 2]
		mov	edx, eax
		mov	ebp, eax
		and	eax, 1FH
		shr	edx, 5
		shr	ebp, 10
		mov	al, TableScale5to8[eax]
		and	edx, 1FH
		and	ebp, 1FH
		mov	BYTE PTR [edi], al
		mov	al, TableScale5to8[edx]
		mov	ah, TableScale5to8[ebp]
		mov	BYTE PTR [edi + 1], al
		mov	BYTE PTR [edi + 2], ah
		mov	BYTE PTR [edi + 3], 0FFH
		;
		add	edi, 4
		mov	edx, [ebx].ptBasePos.y
		mov	eax, [ebx].ptBasePos.x
		add	edx, [ebx].ptDltScanX.y
		add	eax, [ebx].ptDltScanX.x
		mov	[ebx].ptBasePos.y, edx
		mov	[ebx].ptBasePos.x, eax
		dec	ecx
	.ENDW
	ret

;
;	RGB-24 画像を変形サンプリング
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@Sample_RGB24_X:
	mov	edi, [ebx].pDrawLineBuf[0]
	test	ecx, ecx
	mov	edx, [ebx].ptBasePos.y
	mov	eax, [ebx].ptBasePos.x
	.WHILE	!ZERO?
		mov	esi, [ebx].pSrcLineAddr
		sar	edx, 16
		sar	eax, 16
		;
		.IF	edx >= [ebx].srcimg.dwImageHeight
			sar	edx, 31
			not	edx
			and	edx, [ebx].srcimg.dwImageHeight
			sub	edx, 1
			adc	edx, 0
		.ENDIF
		.IF	eax >= [ebx].srcimg.dwImageWidth
			sar	eax, 31
			not	eax
			and	eax, [ebx].srcimg.dwImageWidth
			sub	eax, 1
			adc	eax, 0
		.ENDIF
		lea	eax, [eax + eax * 2]
		mov	esi, DWORD PTR [esi + edx * 4]
		add	esi, eax
		mov	al, BYTE PTR [esi]
		mov	ah, BYTE PTR [esi + 1]
		mov	dl, BYTE PTR [esi + 2]
		mov	BYTE PTR [edi], al
		mov	BYTE PTR [edi + 1], ah
		mov	BYTE PTR [edi + 2], dl
		mov	BYTE PTR [edi + 3], 0FFH
		add	edi, 4
		mov	edx, [ebx].ptBasePos.y
		mov	eax, [ebx].ptBasePos.x
		add	edx, [ebx].ptDltScanX.y
		add	eax, [ebx].ptDltScanX.x
		mov	[ebx].ptBasePos.y, edx
		mov	[ebx].ptBasePos.x, eax
		dec	ecx
	.ENDW
	ret

;
;	RGB-32 画像を変形サンプリング
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@Sample_RGB32_X:
	xor	ebp, ebp
	mov	edi, [ebx].pDrawLineBuf[0]
	.IF	!([ebx].srcimg.fdwFormatType & EIF_WITH_ALPHA)
		mov	ebp, 0FF000000H
	.ENDIF
	test	ecx, ecx
	mov	edx, [ebx].ptBasePos.y
	mov	eax, [ebx].ptBasePos.x
	.WHILE	!ZERO?
		mov	esi, [ebx].pSrcLineAddr
		sar	edx, 16
		sar	eax, 16
		;
		.IF	edx >= [ebx].srcimg.dwImageHeight
			sar	edx, 31
			not	edx
			and	edx, [ebx].srcimg.dwImageHeight
			sub	edx, 1
			adc	edx, 0
		.ENDIF
		.IF	eax >= [ebx].srcimg.dwImageWidth
			sar	eax, 31
			not	eax
			and	eax, [ebx].srcimg.dwImageWidth
			sub	eax, 1
			adc	eax, 0
		.ENDIF
		mov	esi, DWORD PTR [esi + edx * 4]
		mov	eax, DWORD PTR [esi + eax * 4]
		or	eax, ebp
		;
		mov	DWORD PTR [edi], eax
		add	edi, 4
		mov	edx, [ebx].ptBasePos.y
		mov	eax, [ebx].ptBasePos.x
		add	edx, [ebx].ptDltScanX.y
		add	eax, [ebx].ptDltScanX.x
		mov	[ebx].ptBasePos.y, edx
		mov	[ebx].ptBasePos.x, eax
		dec	ecx
	.ENDW
	ret

eglDrawImage@Sample_RGB32_X_MMX:
	xor	ebp, ebp
	movd	mm0, [ebx].ptBasePos.x
	movd	mm1, [ebx].ptBasePos.y
	movd	mm4, [ebx].ptDltScanX.x
	movd	mm5, [ebx].ptDltScanX.y
	.IF	!([ebx].srcimg.fdwFormatType & EIF_WITH_ALPHA)
		mov	ebp, 0FF000000H
	.ENDIF
	mov	edi, [ebx].pDrawLineBuf[0]
	test	ecx, ecx
	punpckldq	mm0, mm1
	punpckldq	mm4, mm5
	movq	mm1, mm0			; mm0 : 現在の座標
	movq	mm6, MMWORD PTR [ebx].tmxSizeMask
	movq	mm5, MMWORD PTR [ebx].tmxMulAddr
	paddd	mm1, mm4			; mm1 : 次の座標
	pxor	mm7, mm7
	mov	esi, DWORD PTR [ebx].tmxSrcImageAddr
	sub	ecx, 2
	.WHILE	!SIGN?
		movq	mm2, mm1
		psrad	mm0, 16
		psrad	mm2, 16
		packssdw	mm0, mm2
			paddd	mm1, mm4
		paddsw	mm0, mm6
			movq	mm2, mm1
		psubusw	mm0, mm6
			paddd	mm1, mm4
		pmaddwd	mm0, mm5
		movd	eax, mm0
		psrlq	mm0, 32
		movd	edx, mm0
			movq	mm0, mm2
		mov	eax, DWORD PTR [esi + eax]
		mov	edx, DWORD PTR [esi + edx]
		or	eax, ebp
		or	edx, ebp
		mov	DWORD PTR [edi], eax
		mov	DWORD PTR [edi + 4], edx
		add	edi, 8
		sub	ecx, 2
	.ENDW
	add	ecx, 2
	.IF	!ZERO?
		psrad	mm0, 16
		packssdw	mm0, mm7
		paddsw	mm0, mm6
		psubusw	mm0, mm6
		pmaddwd	mm0, mm5
		movd	eax, mm0
		mov	eax, DWORD PTR [esi + eax]
		or	eax, ebp
		mov	DWORD PTR [edi], eax
	.ENDIF
	ret

; 線形補完 - MMX
eglDrawImage@Sample_RGB32_XS_MMX:
	xor	ebp, ebp
	movd	mm0, [ebx].ptBasePos.x
	movd	mm1, [ebx].ptBasePos.y
	movd	mm4, [ebx].ptDltScanX.x
	movd	mm5, [ebx].ptDltScanX.y
	.IF	!([ebx].srcimg.fdwFormatType & EIF_WITH_ALPHA)
		mov	ebp, 0FF000000H
	.ENDIF
	mov	edi, [ebx].pDrawLineBuf[0]
	test	ecx, ecx
	punpckldq	mm0, mm1	; dwTemp[0] : 次の座標
	punpckldq	mm4, mm5	; dwTemp[8] : 座標差分
	movq	MMWORD PTR [ebx].dwTemp[0], mm0
	movq	MMWORD PTR [ebx].dwTemp[8], mm4
	mov	[ebx].dwTemp[16], ebp
	;
	; １ピクセル目の事前処理
	;
		paddd	mm4, mm0
	movq	mm3, mm0
	pslld	mm0, 16				; mm0 : ｘ座標の補完率
		movq	MMWORD PTR [ebx].dwTemp[0], mm4
	psrad	mm3, 16
	psrld	mm0, 16 + 1
	movq	mm2, mm3
	paddd	mm3, mmxDWord_1
		movq	mm4, mm0		; mm4 : ｙ座標の補完率
	packssdw	mm2, mm3		; mm2 : 基準点と右下の座標
	movq	mm5, MMWORD PTR [ebx].tmxSizeMask
		punpcklwd	mm0, mm0
	paddsw	mm2, mm5
		punpckldq	mm0, mm0
	psubusw	mm2, mm5
	mov	esi, [ebx].pSrcLineAddr
	movd	eax, mm2
	psrlq	mm2, 32
	mov	edx, eax
	shr	eax, 16
	and	edx, 0FFFFH
	mov	esi, DWORD PTR [esi + eax * 4]
		movd	eax, mm2
		mov	ebp, eax
		shr	eax, 16
		and	ebp, 0FFFFH
	movd	mm1, DWORD PTR [esi + edx * 4]	; mm1 : 上段左側ピクセル
	movd	mm3, DWORD PTR [esi + ebp * 4]	; mm3 : 上段右側ピクセル
		mov	esi, [ebx].pSrcLineAddr
		pxor	mm7, mm7
		psrlq	mm4, 32
		mov	esi, DWORD PTR [esi + eax * 4]
	punpcklbw	mm1, mm7
	punpcklbw	mm3, mm7
		movd	mm2, DWORD PTR [esi + edx * 4]	; mm2 : 下段左側ピクセル
		movd	mm6, DWORD PTR [esi + ebp * 4]	; mm6 : 下段右側ピクセル
	psubsw	mm3, mm1
		punpcklbw	mm2, mm7
	psllw	mm3, 1
		punpcklbw	mm6, mm7
	pmulhw	mm3, mm0
		psubsw	mm6, mm2
		punpcklwd	mm4, mm4
		psllw	mm6, 1
	paddsw	mm1, mm3			; mm1 : 上段補完ピクセル
		pmulhw	mm6, mm0
		punpckldq	mm4, mm4
	;
	dec	ecx
	.WHILE	!ZERO?
			movq	mm0, MMWORD PTR [ebx].dwTemp[0]
				paddsw	mm2, mm6		; mm2 : 下段補完ピクセル
				movq	mm6, mm4
			movq	mm4, MMWORD PTR [ebx].dwTemp[8]
				psubsw	mm2, mm1
		;
			paddd	mm4, mm0
				psllw	mm2, 1
		movq	mm3, mm0
		pslld	mm0, 16				; mm0 : ｘ座標の補完率
				pmulhw	mm2, mm6
			movq	MMWORD PTR [ebx].dwTemp[0], mm4
		psrad	mm3, 16
		psrld	mm0, 16 + 1
				paddsw	mm1, mm2
		movq	mm2, mm3
		paddd	mm3, mmxDWord_1
			movq	mm4, mm0		; mm4 : ｙ座標の補完率
		movq	mm5, MMWORD PTR [ebx].tmxSizeMask
		packssdw	mm2, mm3		; mm2 : 基準点と右下の座標
			punpcklwd	mm0, mm0
		paddsw	mm2, mm5
		mov	esi, [ebx].pSrcLineAddr
		psubusw	mm2, mm5
			punpckldq	mm0, mm0
				packuswb	mm1, mm7
		movd	eax, mm2
		psrlq	mm2, 32
				movd	mm3, [ebx].dwTemp[16]
		mov	edx, eax
		shr	eax, 16
		and	edx, 0FFFFH
		mov	esi, DWORD PTR [esi + eax * 4]
			movd	eax, mm2
				por	mm1, mm3
			mov	ebp, eax
			shr	eax, 16
			and	ebp, 0FFFFH
				movd	DWORD PTR [edi], mm1
		movd	mm1, DWORD PTR [esi + edx * 4]	; mm1 : 上段左側ピクセル
				add	edi, 4
		movd	mm3, DWORD PTR [esi + ebp * 4]	; mm3 : 上段右側ピクセル
			mov	esi, [ebx].pSrcLineAddr
		pxor	mm7, mm7
			movd	mm5, [ebx].dwTemp[16]
		punpcklbw	mm1, mm7
			mov	esi, DWORD PTR [esi + eax * 4]
			psrlq	mm4, 32
		punpcklbw	mm3, mm7
			movd	mm2, DWORD PTR [esi + edx * 4]	; mm2 : 下段左側ピクセル
			movd	mm6, DWORD PTR [esi + ebp * 4]	; mm6 : 下段右側ピクセル
		psubsw	mm3, mm1
			punpcklbw	mm2, mm7
		psllw	mm3, 1
			punpcklbw	mm6, mm7
		pmulhw	mm3, mm0
			psubsw	mm6, mm2
			punpcklwd	mm4, mm4
			psllw	mm6, 1
		dec	ecx
		paddsw	mm1, mm3			; mm1 : 上段補完ピクセル
			pmulhw	mm6, mm0
			punpckldq	mm4, mm4
		;
	.ENDW
	;
	; 最終ピクセルの終了処理
	;
	paddsw	mm2, mm6		; mm2 : 下段補完ピクセル
	psubsw	mm2, mm1
	psllw	mm2, 1
	pmulhw	mm2, mm4
	movd	mm3, [ebx].dwTemp[16]
	paddsw	mm1, mm2
	packuswb	mm1, mm7
	por	mm1, mm3
	movd	DWORD PTR [edi], mm1
	;
	ret

; 線形補完 - SSE
eglDrawImage@Sample_RGB32_XS_SSE:
	xor	ebp, ebp
	movd	mm0, [ebx].ptBasePos.x
	movd	mm1, [ebx].ptBasePos.y
	movd	mm4, [ebx].ptDltScanX.x
	movd	mm5, [ebx].ptDltScanX.y
	mov	eax, 0FF000000H
	test	[ebx].srcimg.fdwFormatType, EIF_WITH_ALPHA
	cmovz	ebp, eax
	mov	edi, [ebx].pDrawLineBuf[0]
	test	ecx, ecx
	punpckldq	mm0, mm1	; dwTemp[0] : 次の座標
	punpckldq	mm4, mm5	; dwTemp[8] : 座標差分
	movq	MMWORD PTR [ebx].dwTemp[0], mm0
	movq	MMWORD PTR [ebx].dwTemp[8], mm4
	mov	[ebx].dwTemp[16], ebp
	;
	; １ピクセル目の事前処理
	;
		paddd	mm4, mm0
	movq	mm3, mm0
	pslld	mm0, 16				; mm0 : ｘ座標の補完率
		movq	MMWORD PTR [ebx].dwTemp[0], mm4
	psrad	mm3, 16
	psrld	mm0, 16 + 1
	movq	mm2, mm3
	paddd	mm3, mmxDWord_1
		movq	mm4, mm0		; mm4 : ｙ座標の補完率
	packssdw	mm2, mm3		; mm2 : 基準点と右下の座標
	movq	mm5, MMWORD PTR [ebx].tmxSizeMask
		punpcklwd	mm0, mm0
	paddsw	mm2, mm5
		punpckldq	mm0, mm0
	psubusw	mm2, mm5
	mov	esi, [ebx].pSrcLineAddr
	movd	eax, mm2
	psrlq	mm2, 32
	mov	edx, eax
	shr	eax, 16
	and	edx, 0FFFFH
	mov	esi, DWORD PTR [esi + eax * 4]
		movd	eax, mm2
		mov	ebp, eax
		shr	eax, 16
		and	ebp, 0FFFFH
	movd	mm1, DWORD PTR [esi + edx * 4]	; mm1 : 上段左側ピクセル
	movd	mm3, DWORD PTR [esi + ebp * 4]	; mm3 : 上段右側ピクセル
		mov	esi, [ebx].pSrcLineAddr
		pxor	mm7, mm7
		psrlq	mm4, 32
		mov	esi, DWORD PTR [esi + eax * 4]
	punpcklbw	mm1, mm7
	punpcklbw	mm3, mm7
		movd	mm2, DWORD PTR [esi + edx * 4]	; mm2 : 下段左側ピクセル
		movd	mm6, DWORD PTR [esi + ebp * 4]	; mm6 : 下段右側ピクセル
	psubsw	mm3, mm1
		punpcklbw	mm2, mm7
	psllw	mm3, 1
		punpcklbw	mm6, mm7
	pmulhw	mm3, mm0
		psubsw	mm6, mm2
		punpcklwd	mm4, mm4
		psllw	mm6, 1
	paddsw	mm1, mm3			; mm1 : 上段補完ピクセル
		pmulhw	mm6, mm0
		punpckldq	mm4, mm4
	;
	dec	ecx
	.WHILE	!ZERO?
			movq	mm0, MMWORD PTR [ebx].dwTemp[0]
				paddsw	mm2, mm6		; mm2 : 下段補完ピクセル
				movq	mm6, mm4
			movq	mm4, MMWORD PTR [ebx].dwTemp[8]
				psubsw	mm2, mm1
		;
			paddd	mm4, mm0
				psllw	mm2, 1
		movq	mm3, mm0
		pslld	mm0, 16				; mm0 : ｘ座標の補完率
				pmulhw	mm2, mm6
			movq	MMWORD PTR [ebx].dwTemp[0], mm4
		psrad	mm3, 16
		psrld	mm0, 16 + 1
				paddsw	mm1, mm2
		movq	mm2, mm3
		paddd	mm3, mmxDWord_1
			movq	mm4, mm0		; mm4 : ｙ座標の補完率
		movq	mm5, MMWORD PTR [ebx].tmxSizeMask
		packssdw	mm2, mm3		; mm2 : 基準点と右下の座標
			pshufw	mm0, mm0, 0
		paddsw	mm2, mm5
		mov	esi, [ebx].pSrcLineAddr
		psubusw	mm2, mm5
				packuswb	mm1, mm7
		pextrw	eax, mm2, 1
				movd	mm3, [ebx].dwTemp[16]
		pextrw	edx, mm2, 0
		mov	esi, DWORD PTR [esi + eax * 4]
			pextrw	eax, mm2, 3
				por	mm1, mm3
			pextrw	ebp, mm2, 2
				movd	DWORD PTR [edi], mm1
		movd	mm1, DWORD PTR [esi + edx * 4]	; mm1 : 上段左側ピクセル
				add	edi, 4
		movd	mm3, DWORD PTR [esi + ebp * 4]	; mm3 : 上段右側ピクセル
			mov	esi, [ebx].pSrcLineAddr
		pxor	mm7, mm7
			movd	mm5, [ebx].dwTemp[16]
		punpcklbw	mm1, mm7
			mov	esi, DWORD PTR [esi + eax * 4]
		punpcklbw	mm3, mm7
			movd	mm2, DWORD PTR [esi + edx * 4]	; mm2 : 下段左側ピクセル
			movd	mm6, DWORD PTR [esi + ebp * 4]	; mm6 : 下段右側ピクセル
		psubsw	mm3, mm1
			punpcklbw	mm2, mm7
		psllw	mm3, 1
			punpcklbw	mm6, mm7
		pmulhw	mm3, mm0
			psubsw	mm6, mm2
		dec	ecx
			psllw	mm6, 1
		paddsw	mm1, mm3			; mm1 : 上段補完ピクセル
			pmulhw	mm6, mm0
			pshufw	mm4, mm4, 10101010B
		;
	.ENDW
	;
	; 最終ピクセルの終了処理
	;
	paddsw	mm2, mm6		; mm2 : 下段補完ピクセル
	psubsw	mm2, mm1
	psllw	mm2, 1
	pmulhw	mm2, mm4
	movd	mm3, [ebx].dwTemp[16]
	paddsw	mm1, mm2
	packuswb	mm1, mm7
	por	mm1, mm3
	movd	DWORD PTR [edi], mm1
	;
	ret


; ----------------------------------------------------------------------------
;	レンダリング関数
; ----------------------------------------------------------------------------
; レジスタ；
;	ebx : HEGL_DRAW_IMAGE
;	esi : 入力画像アドレス（RGBA-32）
;	edi : 出力画像アドレス
;	ebp : Z バッファアドレス（Z バッファのある場合のみ）
;	ecx : ピクセル数
;	ディレクションフラグは常に０
;	ebx, esp 以外の全ての汎用レジスタは破壊してもよい
; ----------------------------------------------------------------------------

;
;	Gray-8 への描画
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@Render_Gray8:
	test	ecx, ecx
	.WHILE	!ZERO?
		push	ecx
		movzx	eax, BYTE PTR [esi + 3]
		movzx	edx, BYTE PTR [edi]
		xor	eax, 0FFH
		inc	edx
		movzx	ecx, BYTE PTR [esi]
		imul	eax, edx
		movzx	edx, BYTE PTR [esi + 2]
		add	edx, ecx
		movzx	ecx, BYTE PTR [esi + 1]
		lea	edx, [edx + ecx * 2]
		pop	ecx
		shr	edx, 2
		shr	eax, 8
		add	al, dl
		sbb	ah, ah
		add	esi, 4
		or	al, ah
		mov	BYTE PTR [edi], al
		inc	edi
		dec	ecx
	.ENDW
	ret

;
;	RGB-24 への描画
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@Render_RGB24:
	test	ecx, ecx
	.WHILE	!ZERO?
		mov	al, BYTE PTR [esi]
		mov	ah, BYTE PTR [esi + 1]
		mov	dl, BYTE PTR [esi + 2]
		add	esi, 3
		mov	BYTE PTR [edi], al
		mov	BYTE PTR [edi + 1], ah
		mov	BYTE PTR [edi + 2], dl
		add	edi, 3
		dec	ecx
	.ENDW
	ret

;
;	RGB-24 への描画（合成）
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@Render_RGB24_B:
	test	ecx, ecx
	.WHILE	!ZERO?
		mov	eax, DWORD PTR [esi]
		add	esi, 4
		test	eax, eax
		mov	edx, eax
		.IF	!ZERO?
			shr	edx, 24
			.IF	!ZERO?
			.IF	edx == 0FFH
				;
				; 不透明描画
				;
				mov	WORD PTR [edi], ax
				shr	eax, 16
				mov	BYTE PTR [edi + 2], al
			.ELSE
				;
				; 半透明描画
				;
				mov	ebp, 0FFH
				xor	ebp, edx
				inc	ebp
				FOR	@INDEX, <0, 1, 2>
					movzx	edx, BYTE PTR [edi + @INDEX]
					imul	edx, ebp
					add	dh, al
					sbb	dl, dl
					shr	eax, 8
					or	dl, dh
					mov	BYTE PTR [edi + @INDEX], dl
				ENDM
			.ENDIF
			.ELSE
				;
				; 加算描画
				;
				add	al, BYTE PTR [edi]
				sbb	dl, dl
				add	ah, BYTE PTR [edi + 1]
				sbb	dh, dh
				or	dl, al
				or	dh, ah
				shr	eax, 16
				mov	BYTE PTR [edi], dl
				add	al, BYTE PTR [edi + 2]
				sbb	dl, dl
				mov	BYTE PTR [edi + 1], dh
				mov	BYTE PTR [edi + 2], dl
			.ENDIF
		.ENDIF
		add	edi, 3
		dec	ecx
	.ENDW
	ret

;
;	RGB-24 への描画（透明度付き合成）
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@Render_RGB24_BT:
	test	ecx, ecx
	.WHILE	!ZERO?
		movzx	eax, BYTE PTR [esi + 3]
		add	esi, 4
		test	eax, eax
		.IF	!ZERO?
			movzx	eax, [ebx].nBlueTone[eax]
			push	ecx
			;
			xor	eax, 0FFH
			movzx	edx, BYTE PTR [edi]
			inc	eax
			movzx	ecx, BYTE PTR [esi - 4]
			imul	edx, eax
			add	dh, [ebx].nBlueTone[ecx]
			sbb	dl, dl
			or	dl, dh
			mov	BYTE PTR [edi], dl
			;
			movzx	edx, BYTE PTR [edi + 1]
			movzx	ecx, BYTE PTR [esi - 3]
			imul	edx, eax
			add	dh, [ebx].nBlueTone[ecx]
			sbb	dl, dl
			or	dl, dh
			mov	BYTE PTR [edi + 1], dl
			;
			movzx	edx, BYTE PTR [edi + 2]
			movzx	ecx, BYTE PTR [esi - 2]
			imul	edx, eax
			add	dh, [ebx].nBlueTone[ecx]
			pop	ecx
			sbb	dl, dl
			or	dl, dh
			mov	BYTE PTR [edi + 2], dl
		.ELSEIF	(DWORD PTR [esi - 4]) != 0
			movzx	eax, BYTE PTR [esi - 4]
			push	ecx
			movzx	ecx, BYTE PTR [esi - 3]
			movzx	edx, BYTE PTR [esi - 2]
			mov	al, [ebx].nBlueTone[eax]
			mov	cl, [ebx].nBlueTone[ecx]
			mov	dl, [ebx].nBlueTone[edx]
			add	al, BYTE PTR [edi]
			sbb	ah, ah
			add	cl, BYTE PTR [edi + 1]
			sbb	ch, ch
			add	dl, BYTE PTR [edi + 2]
			sbb	dh, dh
			or	al, ah
			or	cl, ch
			or	dl, dh
			mov	BYTE PTR [edi], al
			mov	BYTE PTR [edi + 1], cl
			pop	ecx
			mov	BYTE PTR [edi + 2], dl
		.ENDIF
		add	edi, 3
		dec	ecx
	.ENDW
	ret

;
;	RGB-24 への描画（Z バッファ＆透明度付き合成）
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@Render_RGB24_BTZ:
	test	ecx, ecx
	.WHILE	!ZERO?
		mov	edx, DWORD PTR [ebx].rZOrder
		movzx	eax, BYTE PTR [esi + 3]
		add	esi, 4
		.IF	(DWORD PTR edx) <= (DWORD PTR [ebp])
		test	eax, eax
		.IF	!ZERO?
			mov	DWORD PTR [ebp - 4], edx
			movzx	eax, [ebx].nBlueTone[eax]
			push	ecx
			;
			xor	eax, 0FFH
			movzx	edx, BYTE PTR [edi]
			inc	eax
			movzx	ecx, BYTE PTR [esi - 4]
			imul	edx, eax
			add	dh, [ebx].nBlueTone[ecx]
			sbb	dl, dl
			or	dl, dh
			mov	BYTE PTR [edi], dl
			;
			movzx	edx, BYTE PTR [edi + 1]
			movzx	ecx, BYTE PTR [esi - 3]
			imul	edx, eax
			add	dh, [ebx].nBlueTone[ecx]
			sbb	dl, dl
			or	dl, dh
			mov	BYTE PTR [edi + 1], dl
			;
			movzx	edx, BYTE PTR [edi + 2]
			movzx	ecx, BYTE PTR [esi - 2]
			imul	edx, eax
			add	dh, [ebx].nBlueTone[ecx]
			pop	ecx
			sbb	dl, dl
			or	dl, dh
			mov	BYTE PTR [edi + 2], dl

		.ELSEIF	(DWORD PTR [esi - 4]) != 0
			movzx	eax, BYTE PTR [esi - 4]
			push	ecx
			movzx	ecx, BYTE PTR [esi - 3]
			movzx	edx, BYTE PTR [esi - 2]
			mov	al, [ebx].nBlueTone[eax]
			mov	cl, [ebx].nBlueTone[ecx]
			mov	dl, [ebx].nBlueTone[edx]
			add	al, BYTE PTR [edi]
			sbb	ah, ah
			add	cl, BYTE PTR [edi + 1]
			sbb	ch, ch
			add	dl, BYTE PTR [edi + 2]
			sbb	dh, dh
			or	al, ah
			or	cl, ch
			or	dl, dh
			mov	BYTE PTR [edi], al
			mov	BYTE PTR [edi + 1], cl
			pop	ecx
			mov	BYTE PTR [edi + 2], dl
		.ENDIF
		.ENDIF
		add	ebp, 4
		add	edi, 3
		dec	ecx
	.ENDW
	ret

;
;	RGB32 への描画（複製）
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@Render_RGB32:
	rep	movsd
	ret


;
;	RGBA32 への描画（合成）
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@Render_RGBA32_B:
	test	ecx, ecx
	.WHILE	!ZERO?
		mov	eax, DWORD PTR [esi]
		add	esi, 4
		mov	edx, eax
		shr	eax, 24
		.IF	!ZERO?
		.IF	eax == 0FFH
			mov	DWORD PTR [edi], edx
		.ELSE
			neg	eax
			push	ecx
			add	eax, 100H
			;
			movzx	ecx, BYTE PTR [edi]
			movzx	edx, BYTE PTR [edi + 1]
			imul	ecx, eax
			imul	edx, eax
			add	ch, BYTE PTR [esi - 4]
			sbb	cl, cl
			add	dh, BYTE PTR [esi - 3]
			sbb	dl, dl
			or	cl, ch
			or	dl, dh
			mov	BYTE PTR [edi], cl
			mov	BYTE PTR [edi + 1], dl
			;
			movzx	edx, BYTE PTR [edi + 3]
			movzx	ecx, BYTE PTR [edi + 2]
			xor	edx, 0FFH
			imul	ecx, eax
			imul	edx, eax
			add	ch, BYTE PTR [esi - 2]
			sbb	cl, cl
			not	dh
			or	cl, ch
			mov	BYTE PTR [edi + 3], dh
			mov	BYTE PTR [edi + 2], cl
			;
			pop	ecx
		.ENDIF
		.ELSEIF	edx != 0
			push	ecx
			mov	al, BYTE PTR [esi - 4]
			mov	cl, BYTE PTR [esi - 3]
			mov	dl, BYTE PTR [esi - 2]
			add	al, BYTE PTR [edi]
			sbb	ah, ah
			add	cl, BYTE PTR [edi + 1]
			sbb	ch, ch
			add	dl, BYTE PTR [edi + 2]
			sbb	dh, dh
			or	al, ah
			or	cl, ch
			or	dl, dh
			mov	BYTE PTR [edi], al
			mov	BYTE PTR [edi + 1], cl
			pop	ecx
			mov	BYTE PTR [edi + 2], dl
		.ENDIF
		add	edi, 4
		dec	ecx
	.ENDW
	ret

;
;	RGBA32 への描画（透明度つき合成）
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@Render_RGBA32_BT:
	test	ecx, ecx
	.WHILE	!ZERO?
		mov	eax, DWORD PTR [esi]
		add	esi, 4
		mov	edx, eax
		shr	eax, 24
		.IF	!ZERO?
			movzx	eax, [ebx].nBlueTone[eax]
			push	ecx
			xor	eax, 0FFH
			;
			movzx	ecx, BYTE PTR [edi]
			inc	eax
			movzx	edx, BYTE PTR [esi - 4]
			imul	ecx, eax
			add	ch, [ebx].nBlueTone[edx]
			sbb	cl, cl
			or	cl, ch
			mov	BYTE PTR [edi], cl
			;
			movzx	ecx, BYTE PTR [edi + 1]
			movzx	edx, BYTE PTR [esi - 3]
			imul	ecx, eax
			add	ch, [ebx].nBlueTone[edx]
			sbb	cl, cl
			or	cl, ch
			mov	BYTE PTR [edi + 1], cl
			;
			movzx	ecx, BYTE PTR [edi + 2]
			movzx	edx, BYTE PTR [esi - 2]
			imul	ecx, eax
			add	ch, [ebx].nBlueTone[edx]
			sbb	cl, cl
			or	cl, ch
			mov	BYTE PTR [edi + 2], cl
			;
			movzx	edx, BYTE PTR [edi + 3]
			pop	ecx
			xor	edx, 0FFH
			imul	edx, eax
			not	dh
			mov	BYTE PTR [edi + 3], dh
		.ELSEIF	edx != 0
			push	ecx
			movzx	eax, BYTE PTR [esi - 4]
			movzx	ecx, BYTE PTR [esi - 3]
			movzx	edx, BYTE PTR [esi - 2]
			mov	al, [ebx].nBlueTone[eax]
			mov	cl, [ebx].nBlueTone[ecx]
			mov	dl, [ebx].nBlueTone[edx]
			add	al, BYTE PTR [edi]
			sbb	ah, ah
			add	cl, BYTE PTR [edi + 1]
			sbb	ch, ch
			add	dl, BYTE PTR [edi + 2]
			sbb	dh, dh
			or	al, ah
			or	cl, ch
			or	dl, dh
			mov	BYTE PTR [edi], al
			mov	BYTE PTR [edi + 1], cl
			pop	ecx
			mov	BYTE PTR [edi + 2], dl
		.ENDIF
		add	edi, 4
		dec	ecx
	.ENDW
	ret

;
;	RGBA32 への描画（Z バッファ＆透明度つき合成）
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@Render_RGBA32_BTZ:
	test	ecx, ecx
	.WHILE	!ZERO?
		mov	edx, DWORD PTR [ebx].rZOrder
		mov	eax, DWORD PTR [esi]
		add	esi, 4
		.IF	(DWORD PTR edx) <= (DWORD PTR [ebp])
			mov	edx, eax
			shr	eax, 24
			.IF	!ZERO?
				push	ecx
				mov	ecx, DWORD PTR [ebx].rZOrder
				movzx	eax, [ebx].nBlueTone[eax]
				.IF	eax == 0FFH
					mov	DWORD PTR [ebp], ecx
				.ENDIF
				xor	eax, 0FFH
				;
				movzx	ecx, BYTE PTR [edi]
				inc	eax
				movzx	edx, BYTE PTR [esi - 4]
				imul	ecx, eax
				add	ch, [ebx].nBlueTone[edx]
				sbb	cl, cl
				or	cl, ch
				mov	BYTE PTR [edi], cl
				;
				movzx	ecx, BYTE PTR [edi + 1]
				movzx	edx, BYTE PTR [esi - 3]
				imul	ecx, eax
				add	ch, [ebx].nBlueTone[edx]
				sbb	cl, cl
				or	cl, ch
				mov	BYTE PTR [edi + 1], cl
				;
				movzx	ecx, BYTE PTR [edi + 2]
				movzx	edx, BYTE PTR [esi - 2]
				imul	ecx, eax
				add	ch, [ebx].nBlueTone[edx]
				sbb	cl, cl
				or	cl, ch
				mov	BYTE PTR [edi + 2], cl
				;
				movzx	edx, BYTE PTR [edi + 3]
				pop	ecx
				xor	edx, 0FFH
				imul	edx, eax
				not	dh
				mov	BYTE PTR [edi + 3], dh

			.ELSEIF	edx != 0
				push	ecx
				movzx	eax, BYTE PTR [esi - 4]
				movzx	ecx, BYTE PTR [esi - 3]
				movzx	edx, BYTE PTR [esi - 2]
				mov	al, [ebx].nBlueTone[eax]
				mov	cl, [ebx].nBlueTone[ecx]
				mov	dl, [ebx].nBlueTone[edx]
				add	al, BYTE PTR [edi]
				sbb	ah, ah
				add	cl, BYTE PTR [edi + 1]
				sbb	ch, ch
				add	dl, BYTE PTR [edi + 2]
				sbb	dh, dh
				or	al, ah
				or	cl, ch
				or	dl, dh
				mov	BYTE PTR [edi], al
				mov	BYTE PTR [edi + 1], cl
				pop	ecx
				mov	BYTE PTR [edi + 2], dl
			.ENDIF
		.ENDIF
		add	ebp, 4
		add	edi, 4
		dec	ecx
	.ENDW
	ret

;
;	RGB32 への描画（複製）
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@Render_RGB32_SSE:
	.IF	(ecx >= 80H) && !(edi & 03H)
		.WHILE	edi & 0FH
			movsd
			dec	ecx
		.ENDW
		.IF	!(esi & 0FH)
			sub	ecx, 8
			.WHILE	!SIGN?
				movaps	xmm0, [esi]
				movaps	xmm1, [esi + 16]
				add	esi, 32
				movntps	[edi], xmm0
				movntps	[edi + 16], xmm1
				add	edi, 32
				sub	ecx, 8
			.ENDW
			add	ecx, 8
		.ELSE
			sub	ecx, 8
			.WHILE	!SIGN?
				movups	xmm0, [esi]
				movups	xmm1, [esi + 16]
				add	esi, 32
				movntps	[edi], xmm0
				movntps	[edi + 16], xmm1
				add	edi, 32
				sub	ecx, 8
			.ENDW
			add	ecx, 8
		.ENDIF
	.ENDIF
	rep	movsd
	ret

;
;	RGBA32 への描画（合成）
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@Render_RGBA32_B_MMX:
	pxor	mm7, mm7
	movq	mm5, mmxConstFF000000
	movq	mm6, mmxConst00FFFFFF
	sub	ecx, 2
	.WHILE	!SIGN?
		mov	ebp, [ebx].dstimg.dwBytesPerLine
		mov	eax, DWORD PTR [esi]
		mov	edx, DWORD PTR [esi + 4]
		mov	ebp, eax
		movq	mm4, MMWORD PTR [esi]
		or	eax, edx
		.IF	!ZERO?
		.IF	(DWORD PTR eax) <= 00FFFFFFH
			paddusb	mm4, MMWORD PTR [edi]
			movq	MMWORD PTR [edi], mm4
		.ELSE
			mov	eax, ebp
			and	ebp, edx
			.IF	(DWORD PTR ebp) >= 0FF000000H
				movq	MMWORD PTR [edi], mm4
			.ELSE
				movq	mm0, MMWORD PTR [edi]
				shr	eax, 24
				shr	edx, 24
				pxor	mm0, mm5
				xor	eax, 0FFH
				xor	edx, 0FFH
				movq	mm1, mm0
				punpcklbw	mm0, mm7
				inc	eax
				inc	edx
				punpckhbw	mm1, mm7
				movd	mm2, eax
				movd	mm3, edx
				punpcklwd	mm2, mm2
				punpcklwd	mm3, mm3
				punpckldq	mm2, mm2
				punpckldq	mm3, mm3
				pmullw	mm0, mm2
				pmullw	mm1, mm3
				pand	mm4, mm6
				psrlw	mm0, 8
				psrlw	mm1, 8
				packuswb	mm0, mm1
				paddusb	mm0, mm4
				pxor	mm0, mm5
				movq	MMWORD PTR [edi], mm0
			.ENDIF
		.ENDIF
		.ENDIF
		add	esi, 8
		add	edi, 8
		sub	ecx, 2
	.ENDW
	add	ecx, 2
	.IF	!ZERO?
		mov	eax, DWORD PTR [esi]
		.IF	(DWORD PTR eax) >= 0FF000000H
			mov	DWORD PTR [edi], eax
		.ELSE
			movd	mm4, eax
			shr	eax, 24
			movd	mm0, DWORD PTR [edi]
			xor	eax, 0FFH
			pxor	mm0, mm5
			inc	eax
			punpcklbw	mm0, mm7
			movd	mm1, eax
			punpcklwd	mm1, mm1
			punpckldq	mm1, mm1
			pmullw	mm0, mm1
			pand	mm4, mm6
			psrlw	mm0, 8
			packuswb	mm0, mm7
			paddusb	mm0, mm4
			pxor	mm0, mm5
			movd	DWORD PTR [edi], mm0
		.ENDIF
	.ENDIF
	ret

ALIGN	10H
eglDrawImage@Render_RGBA32_B_SSE:
	pxor	mm7, mm7
	movq	mm5, mmxConstFF000000
	movq	mm6, mmxConst00FFFFFF
	sub	ecx, 2
	.WHILE	!SIGN?
		mov	ebp, [ebx].dstimg.dwBytesPerLine
		mov	eax, DWORD PTR [esi]
		mov	edx, DWORD PTR [esi + 4]
		prefetchnta	[edi + ebp]
		mov	ebp, eax
		movq	mm4, MMWORD PTR [esi]
		or	eax, edx
		.IF	!ZERO?
		.IF	(DWORD PTR eax) <= 00FFFFFFH
			paddusb	mm4, MMWORD PTR [edi]
			movq	MMWORD PTR [edi], mm4
		.ELSE
			mov	eax, ebp
			and	ebp, edx
			.IF	(DWORD PTR ebp) >= 0FF000000H
				movq	MMWORD PTR [edi], mm4
			.ELSE
				movq	mm0, MMWORD PTR [edi]
				shr	eax, 24
				shr	edx, 24
				pxor	mm0, mm5
				xor	eax, 0FFH
				xor	edx, 0FFH
				movq	mm1, mm0
				punpcklbw	mm0, mm7
				inc	eax
				inc	edx
				punpckhbw	mm1, mm7
				movd	mm2, eax
				movd	mm3, edx
				pshufw	mm2, mm2, 0
				pshufw	mm3, mm3, 0
				pmullw	mm0, mm2
				pmullw	mm1, mm3
				pand	mm4, mm6
				psrlw	mm0, 8
				psrlw	mm1, 8
				packuswb	mm0, mm1
				paddusb	mm0, mm4
				pxor	mm0, mm5
				movq	MMWORD PTR [edi], mm0
			.ENDIF
		.ENDIF
		.ENDIF
		add	esi, 8
		add	edi, 8
		sub	ecx, 2
	.ENDW
	add	ecx, 2
	.IF	!ZERO?
		mov	eax, DWORD PTR [esi]
		.IF	(DWORD PTR eax) >= 0FF000000H
			mov	DWORD PTR [edi], eax
		.ELSE
			movd	mm4, eax
			shr	eax, 24
			movd	mm0, DWORD PTR [edi]
			xor	eax, 0FFH
			pxor	mm0, mm5
			inc	eax
			punpcklbw	mm0, mm7
			movd	mm1, eax
			pshufw	mm1, mm1, 0
			pmullw	mm0, mm1
			pand	mm4, mm6
			psrlw	mm0, 8
			packuswb	mm0, mm7
			paddusb	mm0, mm4
			pxor	mm0, mm5
			movd	DWORD PTR [edi], mm0
		.ENDIF
	.ENDIF
	ret

;
;	RGBA32 への描画（z バッファ付き合成）
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@Render_RGBA32_BZ_SSE:
	pxor	mm7, mm7
	movq	mm5, mmxConstFF000000
	movq	mm6, mmxConst00FFFFFF
	sub	ecx, 8
	.WHILE	!SIGN?
		mov	edx, [ebx].zbuf.dwBytesPerLine
		mov	eax, [ebx].dstimg.dwBytesPerLine
		prefetchnta	[ebp + edx]
		prefetchnta	[edi + eax]
		push	ecx
		mov	ecx, 8
		.REPEAT
			movd	mm4, DWORD PTR [esi]
			mov	edx, [ebx].rZOrder
			movd	eax, mm4
			.IF	(SDWORD PTR edx) <= DWORD PTR [ebp]
			test	eax, eax
			.IF	!ZERO?
				.IF	(DWORD PTR eax) >= 0FF000000H
					mov	DWORD PTR [ebp], edx
					mov	DWORD PTR [edi], eax
				.ELSE
					movd	mm0, DWORD PTR [edi]
					shr	eax, 24
					pxor	mm0, mm5
					xor	eax, 0FFH
					punpcklbw	mm0, mm7
					inc	eax
					movd	mm2, eax
					pshufw	mm2, mm2, 0
					pmullw	mm0, mm2
					pand	mm4, mm6
					psrlw	mm0, 8
					packuswb	mm0, mm7
					paddusb	mm0, mm4
					pxor	mm0, mm5
					movd	DWORD PTR [edi], mm0
				.ENDIF
			.ENDIF
			.ENDIF
			add	ebp, 4
			add	esi, 4
			add	edi, 4
			dec	ecx
		.UNTIL	ZERO?
		pop	ecx
		sub	ecx, 8
	.ENDW
	add	ecx, 8
	.WHILE	!ZERO?
		movd	mm4, DWORD PTR [esi]
		mov	edx, [ebx].rZOrder
		movd	eax, mm4
		.IF	(SDWORD PTR edx) <= DWORD PTR [ebp]
		test	eax, eax
		.IF	!ZERO?
			.IF	(DWORD PTR eax) >= 0FF000000H
				mov	DWORD PTR [ebp], edx
				mov	DWORD PTR [edi], eax
			.ELSE
				movd	mm0, DWORD PTR [edi]
				shr	eax, 24
				pxor	mm0, mm5
				xor	eax, 0FFH
				punpcklbw	mm0, mm7
				inc	eax
				movd	mm2, eax
				pshufw	mm2, mm2, 0
				pmullw	mm0, mm2
				pand	mm4, mm6
				psrlw	mm0, 8
				packuswb	mm0, mm7
				paddusb	mm0, mm4
				pxor	mm0, mm5
				movd	DWORD PTR [edi], mm0
			.ENDIF
		.ENDIF
		.ENDIF
		add	ebp, 4
		add	esi, 4
		add	edi, 4
		dec	ecx
	.ENDW
	ret

;
;	RGBA32 への描画（透明度付き合成）
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@Render_RGBA32_BT_SSE:
	mov	eax, 100H
	pxor	mm7, mm7
	sub	eax, [ebx].nTrans
	movd	mm6, eax
	movq	mm2, mmxConstFF000000
	movq	mm3, mmxHighWord_100
	movq	mm5, mmxMaskLow3Words
	test	ecx, ecx
	pshufw	mm6, mm6, 0
	mov	edx, [ebx].dstimg.dwBytesPerLine
	.WHILE	!ZERO?
		mov	eax, DWORD PTR [esi]
		prefetchnta	[edi + edx]
		test	eax, eax
		.IF	!ZERO?
			movd	mm0, DWORD PTR [edi]
			movd	mm4, eax
			cmp	eax, 01000000H
			;
			.IF	CARRY?		; eax <= 00FFFFFFH
				punpcklbw	mm4, mm7
				pmullw	mm4, mm6
				punpcklbw	mm0, mm7
				psrlw	mm4, 8
				paddusw	mm0, mm4
				packuswb	mm0, mm7
				movd	DWORD PTR [edi], mm0
			.ELSE
				pxor	mm0, mm2
				punpcklbw	mm4, mm7
				punpcklbw	mm0, mm7
				pmullw	mm4, mm6
				movq	mm1, mm3
				psrlw	mm4, 8
				psubw	mm1, mm4
				pand	mm4, mm5
				pshufw	mm1, mm1, 11111111B
				pmullw	mm0, mm1
				psrlw	mm0, 8
				paddusw	mm0, mm4
				packuswb	mm0, mm7
				pxor	mm0, mm2
				movd	DWORD PTR [edi], mm0
			.ENDIF
		.ENDIF
		add	esi, 4
		add	edi, 4
		dec	ecx
	.ENDW
	ret

;
;	RGBA32 への描画（ｚバッファ＆透明度付き合成）
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@Render_RGBA32_BTZ_SSE:
	mov	eax, 100H
	pxor	mm7, mm7
	sub	eax, [ebx].nTrans
	movd	mm6, eax
	movq	mm2, mmxConstFF000000
	movq	mm3, mmxHighWord_100
	movq	mm5, mmxMaskLow3Words
	test	ecx, ecx
	pshufw	mm6, mm6, 0
	.WHILE	!ZERO?
		mov	eax, DWORD PTR [esi]
		mov	edx, [ebx].rZOrder
		test	eax, eax
		.IF	!ZERO?
		.IF	edx <= (DWORD PTR [ebp])
			movd	mm0, DWORD PTR [edi]
			movd	mm4, eax
			cmp	eax, 01000000H
			;
			.IF	CARRY?		; eax <= 00FFFFFFH
				punpcklbw	mm4, mm7
				pmullw	mm4, mm6
				punpcklbw	mm0, mm7
				psrlw	mm4, 8
				paddusw	mm0, mm4
				packuswb	mm0, mm7
				movd	DWORD PTR [edi], mm0
			.ELSE
				cmp	eax, 0FF000000H
				pxor	mm0, mm2
				cmovb	edx, DWORD PTR [ebp]
				punpcklbw	mm4, mm7
				punpcklbw	mm0, mm7
				pmullw	mm4, mm6
;				mov	DWORD PTR [ebp], edx
				movq	mm1, mm3
				psrlw	mm4, 8
				psubw	mm1, mm4
				pand	mm4, mm5
				pshufw	mm1, mm1, 11111111B
				pmullw	mm0, mm1
				psrlw	mm0, 8
				paddusw	mm0, mm4
				packuswb	mm0, mm7
				pxor	mm0, mm2
				movd	DWORD PTR [edi], mm0
			.ENDIF
		.ENDIF
		.ENDIF
		add	ebp, 4
		add	esi, 4
		add	edi, 4
		dec	ecx
	.ENDW
	ret


; ----------------------------------------------------------------------------
;	描画パラメータ適用関数
; ----------------------------------------------------------------------------
; レジスタ；
;	ebx : HEGL_DRAW_IMAGE
;	ecx : ピクセル数
;	ディレクションフラグは常に０
;	ebx, esp 以外の全ての汎用レジスタは破壊してもよい
; ----------------------------------------------------------------------------

;
;	ダミー
; ----------------------------------------------------------------------------
eglDrawImage@Apply_nothing:
	ret

;
;	透明度適用
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@Apply_Transparency:
	test	ecx, ecx
	mov	esi, [ebx].pDrawLineBuf[0]
	.WHILE	!ZERO?
		movzx	eax, BYTE PTR [esi]
		movzx	edx, BYTE PTR [esi + 1]
		movzx	edi, BYTE PTR [esi + 2]
		movzx	ebp, BYTE PTR [esi + 3]
		mov	al, [ebx].nBlueTone[eax]
		mov	ah, [ebx].nBlueTone[edx]
		mov	dl, [ebx].nBlueTone[edi]
		mov	dh, [ebx].nBlueTone[ebp]
		mov	BYTE PTR [esi], al
		mov	BYTE PTR [esi + 1], ah
		mov	BYTE PTR [esi + 2], dl
		mov	BYTE PTR [esi + 3], dh
		add	esi, 4
		dec	ecx
	.ENDW
	ret

ALIGN	10H
eglDrawImage@Apply_Transparency_MMX:
	mov	eax, 100H
	sub	eax, [ebx].nTrans
	mov	esi, [ebx].pDrawLineBuf[0]
	movd	mm6, eax
	pxor	mm7, mm7
	punpcklwd	mm6, mm6
	sub	ecx, 2
	punpckldq	mm6, mm6
	.WHILE	!SIGN?
		movq	mm0, MMWORD PTR [esi]
		movq	mm1, mm0
		punpcklbw	mm0, mm7
		punpckhbw	mm1, mm7
		pmullw	mm0, mm6
		pmullw	mm1, mm6
		psrlw	mm0, 8
		psrlw	mm1, 8
		packuswb	mm0, mm1
		movq	MMWORD PTR [esi], mm0
		add	esi, 8
		sub	ecx, 2
	.ENDW
	add	ecx, 2
	.IF	!ZERO?
		movd	mm0, DWORD PTR [esi]
		punpcklbw	mm0, mm7
		pmullw	mm0, mm6
		psrlw	mm0, 8
		packuswb	mm0, mm7
		movd	DWORD PTR [esi], mm0
	.ENDIF
	ret

ALIGN	10H
eglDrawImage@Apply_ITransparency:
	test	ecx, ecx
	mov	esi, [ebx].pDrawLineBuf[0]
	.WHILE	!ZERO?
		movzx	eax, BYTE PTR [esi]
		movzx	edx, BYTE PTR [esi + 1]
		movzx	edi, BYTE PTR [esi + 2]
		movzx	ebp, BYTE PTR [esi + 3]
		xor	eax, 0FFH
		xor	edx, 0FFH
		xor	edi, 0FFH
		xor	ebp, 0FFH
		mov	al, [ebx].nBlueTone[eax]
		mov	ah, [ebx].nBlueTone[edx]
		mov	dl, [ebx].nBlueTone[edi]
		mov	dh, [ebx].nBlueTone[ebp]
		not	al
		not	ah
		not	dl
		not	dh
		mov	BYTE PTR [esi], al
		mov	BYTE PTR [esi + 1], ah
		mov	BYTE PTR [esi + 2], dl
		mov	BYTE PTR [esi + 3], dh
		add	esi, 4
		dec	ecx
	.ENDW
	ret

ALIGN	10H
eglDrawImage@Apply_ITransparency_MMX:
	mov	eax, 100H
	sub	eax, [ebx].nTrans
	mov	esi, [ebx].pDrawLineBuf[0]
	movd	mm6, eax
	pxor	mm7, mm7
	punpcklwd	mm6, mm6
	sub	ecx, 2
	punpckldq	mm6, mm6
	pcmpeqw	mm5, mm5
	.WHILE	!SIGN?
		movq	mm0, MMWORD PTR [esi]
		pxor	mm0, mm5
		movq	mm1, mm0
		punpcklbw	mm0, mm7
		punpckhbw	mm1, mm7
		pmullw	mm0, mm6
		pmullw	mm1, mm6
		psrlw	mm0, 8
		psrlw	mm1, 8
		packuswb	mm0, mm1
		pxor	mm0, mm5
		movq	MMWORD PTR [esi], mm0
		add	esi, 8
		sub	ecx, 2
	.ENDW
	add	ecx, 2
	.IF	!ZERO?
		movd	mm0, DWORD PTR [esi]
		pxor	mm0, mm5
		punpcklbw	mm0, mm7
		pmullw	mm0, mm6
		psrlw	mm0, 8
		packuswb	mm0, mm7
		pxor	mm0, mm5
		movd	DWORD PTR [esi], mm0
	.ENDIF
	ret


;
;	色積算
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@Apply_MulColor:
	test	ecx, ecx
	push	ecx
	mov	esi, [ebx].pDrawLineBuf[0]
	.WHILE	!ZERO?
		movzx	eax, BYTE PTR [esi]
		movzx	edx, BYTE PTR [esi + 1]
		movzx	edi, [ebx].colorDraw.rgba.Blue
		movzx	ebp, [ebx].colorDraw.rgba.Green
		inc	eax
		inc	edx
		imul	eax, edi
		movzx	edi, BYTE PTR [esi + 2]
		imul	edx, ebp
		movzx	ebp, BYTE PTR [esi + 3]
		inc	edi
		inc	edx
		mov	BYTE PTR [esi], ah
		movzx	eax, [ebx].colorDraw.rgba.Red
		mov	BYTE PTR [esi + 1], dh
		movzx	edx, [ebx].colorDraw.rgba.Alpha
		imul	eax, edi
		imul	edx, ebp
		mov	BYTE PTR [esi + 2], ah
		mov	BYTE PTR [esi + 3], dh
		add	esi, 4
		dec	ecx
	.ENDW
	cmp	[ebx].nTrans, 0
	pop	ecx
	ja	eglDrawImage@Apply_Transparency
	ret

ALIGN	10H
eglDrawImage@Apply_MulColor_MMX:
	mov	eax, 100H
	pxor	mm7, mm7
	sub	eax, [ebx].nTrans
	movd	mm6, DWORD PTR [ebx].colorDraw.dwPixelCode
	movd	mm4, eax
	mov	esi, [ebx].pDrawLineBuf[0]
	punpcklwd	mm4, mm4
	pcmpeqw	mm5, mm5
	punpcklbw	mm6, mm7
	punpckldq	mm4, mm4
	sub	ecx, 2
	pmullw	mm6, mm4
	psrlw	mm6, 8
	psubw	mm6, mm5
	.WHILE	!SIGN?
		movq	mm0, MMWORD PTR [esi]
		movq	mm1, mm0
		punpcklbw	mm0, mm7
		punpckhbw	mm1, mm7
		pmullw	mm0, mm6
		pmullw	mm1, mm6
		psrlw	mm0, 8
		psrlw	mm1, 8
		packuswb	mm0, mm1
		movq	MMWORD PTR [esi], mm0
		add	esi, 8
		sub	ecx, 2
	.ENDW
	add	ecx, 2
	.IF	!ZERO?
		movd	mm0, DWORD PTR [esi]
		punpcklbw	mm0, mm7
		pmullw	mm0, mm6
		psrlw	mm0, 8
		packuswb	mm0, mm7
		movd	DWORD PTR [esi], mm0
	.ENDIF
	ret

;
;	色加算
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@Apply_AddColor:
	test	ecx, ecx
	push	ecx
	mov	esi, [ebx].pDrawLineBuf[0]
	mov	edi, ecx
	.WHILE	!ZERO?
		movzx	ebp, BYTE PTR [esi + 3]
		test	ebp, ebp
		.IF	!ZERO?
			.IF	ebp == 0FFH
				mov	al, BYTE PTR [esi]
				mov	dl, BYTE PTR [esi + 1]
				add	al, [ebx].colorDraw.rgba.Blue
				sbb	ah, ah
				add	dl, [ebx].colorDraw.rgba.Green
				sbb	dh, dh
				or	al, ah
				mov	ah, BYTE PTR [esi + 2]
				or	dl, dh
				add	ah, [ebx].colorDraw.rgba.Red
				mov	BYTE PTR [esi], al
				sbb	al, al
				mov	BYTE PTR [esi + 1], dl
				or	al, ah
				mov	BYTE PTR [esi + 2], al
			.ELSE
				inc	ebp
				movzx	eax, [ebx].colorDraw.rgba.Blue
				movzx	ecx, [ebx].colorDraw.rgba.Green
				movzx	edx, [ebx].colorDraw.rgba.Red
				imul	eax, ebp
				imul	ecx, ebp
				add	ah, BYTE PTR [esi]
				sbb	al, al
				imul	edx, ebp
				add	ch, BYTE PTR [esi + 1]
				sbb	cl, cl
				add	dh, BYTE PTR [esi + 2]
				sbb	dl, dl
				or	al, ah
				or	cl, ch
				or	dl, dh
				mov	BYTE PTR [esi], al
				mov	BYTE PTR [esi + 1], cl
				mov	BYTE PTR [esi + 2], dl
			.ENDIF
		.ENDIF
		add	esi, 4
		dec	edi
	.ENDW
	cmp	[ebx].nTrans, 0
	pop	ecx
	ja	eglDrawImage@Apply_Transparency
	ret

ALIGN	10H
eglDrawImage@Apply_AddColor_MMX:
	pxor	mm7, mm7
	pcmpeqw	mm4, mm4
	mov	eax, [ebx].colorDraw.dwPixelCode
	push	ecx
	and	eax, 00FFFFFFH
	sub	ecx, 2
	movd	mm5, eax
	movd	mm6, eax
	mov	esi, [ebx].pDrawLineBuf[0]
	punpcklbw	mm5, mm7
	punpckldq	mm6, mm6
	psubw	mm5, mm4
	.WHILE	!SIGN?
		mov	eax, DWORD PTR [esi]
		mov	edx, DWORD PTR [esi + 4]
		mov	ebp, eax
		or	eax, edx
		.IF	(DWORD PTR eax) >= 01000000H
			mov	eax, ebp
			and	ebp, edx
			shr	eax, 24
			shr	edx, 24
			movq	mm0, MMWORD PTR [esi]
			.IF	ebp >= 0FF000000H
				paddusb	mm0, mm6
			.ELSE
				movd	mm2, eax
				movd	mm3, edx
				punpcklwd	mm2, mm2
				punpcklwd	mm3, mm3
				punpckldq	mm2, mm2
				punpckldq	mm3, mm3
				pmullw	mm2, mm5
				pmullw	mm3, mm5
				psrlw	mm2, 8
				psrlw	mm3, 8
				packuswb	mm2, mm3
				paddusb	mm0, mm2
			.ENDIF
			movq	MMWORD PTR [esi], mm0
		.ENDIF
		add	esi, 8
		sub	ecx, 2
	.ENDW
	add	ecx, 2
	.IF	!ZERO?
		movzx	eax, BYTE PTR [esi + 3]
		test	eax, eax
		.IF	!ZERO?
			movd	mm1, eax
			movd	mm0, DWORD PTR [esi]
			punpcklwd	mm1, mm1
			punpckldq	mm1, mm1
			pmullw	mm1, mm5
			psrlw	mm1, 8
			packuswb	mm1, mm7
			paddusb	mm0, mm1
			movd	DWORD PTR [esi], mm0
		.ENDIF
	.ENDIF
	cmp	[ebx].nTrans, 0
	pop	ecx
	ja	eglDrawImage@Apply_Transparency_MMX
	ret

;
;	αチャネル積算
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@Product_Alpha:
	test	ecx, ecx
	mov	esi, [ebx].pDrawLineBuf[0]
	mov	edi, ecx
	push	ecx
	.WHILE	!ZERO?
		movzx	ebp, BYTE PTR [esi + 3]
		movzx	eax, BYTE PTR [esi]
		test	ebp, ebp
		movzx	ecx, BYTE PTR [esi + 1]
		movzx	edx, BYTE PTR [esi + 2]
		.IF	!ZERO?
		.IF	ebp != 0FFH
			inc	ebp
			imul	eax, ebp
			imul	ecx, ebp
			imul	edx, ebp
			mov	BYTE PTR [esi], ah
			mov	BYTE PTR [esi + 1], ch
			mov	BYTE PTR [esi + 2], dh
		.ENDIF
		.ELSE
			mov	DWORD PTR [esi], 0
		.ENDIF
		add	esi, 4
		dec	edi
	.ENDW
	cmp	[ebx].nTrans, 0
	pop	ecx
	ja	eglDrawImage@Apply_Transparency
	ret

ALIGN	10H
eglDrawImage@Product_Alpha_MMX:
	mov	eax, 100H
	mov	esi, [ebx].pDrawLineBuf[0]
	sub	eax, [ebx].nTrans
	.IF	eax == 100H
		test	ecx, ecx
		pxor	mm7, mm7
		.WHILE	!ZERO?
			mov	eax, DWORD PTR [esi]
			add	esi, 4
			movd	mm0, eax
			shr	eax, 24
			pand	mm0, mmxConst00FFFFFF
			.IF	!ZERO?
				inc	eax
				punpcklbw	mm0, mm7
				movd	mm1, eax
				.IF	eax != 0FFH
					punpcklwd	mm1, mm1
					dec	eax
					punpckldq	mm1, mm1
					shl	eax, 24
					pmullw	mm0, mm1
					movd	mm1, eax
					psrlw	mm0, 8
					packuswb	mm0, mm7
					por	mm0, mm1
					movd	DWORD PTR [esi - 4], mm0
				.ENDIF
			.ELSE
				mov	DWORD PTR [esi - 4], 0
			.ENDIF
			dec	ecx
		.ENDW
	.ELSE
		movd	mm4, eax
		sub	ecx, 2
		movq	mm5, mmxMaskLow3Words
		punpcklwd	mm4, mm4
		movq	mm6, mmxHighWord_100
		punpckldq	mm4, mm4
		.WHILE	!SIGN?
			movq	mm0, MMWORD PTR [esi]
			pxor	mm7, mm7
			movq	mm2, mm0
			punpcklbw	mm0, mm7
			punpckhbw	mm2, mm7
			pcmpeqw	mm7, mm7
			movq	mm1, mm0
			pmullw	mm0, mm4
			movq	mm3, mm2
			pmullw	mm2, mm4
			psrlq	mm1, 64-16
			psrlq	mm3, 64-16
			punpcklwd	mm1, mm1
			punpcklwd	mm3, mm3
			punpckldq	mm1, mm1
			punpckldq	mm3, mm3
			psubw	mm1, mm7
			psubw	mm3, mm7
			psrlw	mm0, 8
			psrlw	mm2, 8
			pand	mm1, mm5
			pand	mm3, mm5
			por	mm1, mm6
			por	mm3, mm6
			pmullw	mm0, mm1
			pmullw	mm2, mm3
			add	esi, 8
			psrlw	mm0, 8
			psrlw	mm2, 8
			packuswb	mm0, mm2
			movq	MMWORD PTR [esi - 8], mm0
			sub	ecx, 2
		.ENDW
		add	ecx, 2
		.IF	!ZERO?
			movd	mm0, DWORD PTR [esi]
			pxor	mm7, mm7
			punpcklbw	mm0, mm7
			pcmpeqw	mm7, mm7
			movq	mm1, mm0
			pmullw	mm0, mm4
			psrlq	mm1, 64-16
			punpcklwd	mm1, mm1
			punpckldq	mm1, mm1
			psubw	mm1, mm7
			psrlw	mm0, 8
			pand	mm1, mm5
			por	mm1, mm6
			pmullw	mm0, mm1
			psrlw	mm0, 8
			packuswb	mm0, mm0
			movd	DWORD PTR [esi], mm0
		.ENDIF
	.ENDIF
	ret

;
;	色マスク
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@Apply_ColorMask:
	test	ecx, ecx
	mov	esi, [ebx].pDrawLineBuf[0]
	mov	edi, ecx
	mov	edx, [ebx].colorDraw.dwPixelCode
	push	ecx
	.WHILE	!ZERO?
		.IF	(DWORD PTR [esi]) == edx
			mov	DWORD PTR [esi], 0
		.ENDIF
		add	esi, 4
		dec	edi
	.ENDW
	cmp	[ebx].nTrans, 0
	pop	ecx
	ja	eglDrawImage@Apply_Transparency
	ret

ALIGN	10H
eglDrawImage@Apply_ColorMask_MMX:
	test	ecx, ecx
	mov	esi, [ebx].pDrawLineBuf[0]
	mov	edi, ecx
	mov	edx, [ebx].colorDraw.dwPixelCode
	push	ecx
	.WHILE	!ZERO?
		.IF	(DWORD PTR [esi]) == edx
			mov	DWORD PTR [esi], 0
		.ENDIF
		add	esi, 4
		dec	edi
	.ENDW
	cmp	[ebx].nTrans, 0
	pop	ecx
	ja	eglDrawImage@Apply_Transparency_MMX
	ret


; ----------------------------------------------------------------------------
;	特殊合成描画
; ----------------------------------------------------------------------------

;
;	加算描画
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@Render_AddColor:
	test	ebp, ebp
	.IF	ZERO?

	test	ecx, ecx
	.WHILE	!ZERO?
		mov	al, BYTE PTR [edi]
		mov	dl, BYTE PTR [edi + 1]
		add	al, BYTE PTR [esi]
		sbb	ah, ah
		add	dl, BYTE PTR [esi + 1]
		sbb	dh, dh
		or	al, ah
		or	dl, dh
		mov	BYTE PTR [edi], al
		mov	al, BYTE PTR [edi + 2]
		mov	BYTE PTR [edi + 1], dl
		add	al, BYTE PTR [esi + 2]
		sbb	ah, ah
		or	al, ah
		mov	BYTE PTR [edi + 2], al
		add	esi, 4
		add	edi, 4
		dec	ecx
	.ENDW
	ret

	.ELSE

	test	ecx, ecx
	.WHILE	!ZERO?
		mov	eax, [ebx].rZOrder
		.IF	(DWORD PTR eax) <= (DWORD PTR [ebp])
			mov	al, BYTE PTR [edi]
			mov	dl, BYTE PTR [edi + 1]
			add	al, BYTE PTR [esi]
			sbb	ah, ah
			add	dl, BYTE PTR [esi + 1]
			sbb	dh, dh
			or	al, ah
			or	dl, dh
			mov	BYTE PTR [edi], al
			mov	al, BYTE PTR [edi + 2]
			mov	BYTE PTR [edi + 1], dl
			add	al, BYTE PTR [esi + 2]
			sbb	ah, ah
			or	al, ah
			mov	BYTE PTR [edi + 2], al
		.ENDIF
		add	ebp, 4
		add	esi, 4
		add	edi, 4
		dec	ecx
	.ENDW

	.ENDIF
	ret

ALIGN	10H
eglDrawImage@Render_AddColor8:
	test	ebp, ebp
	.IF	ZERO?

	test	ecx, ecx
	.WHILE	!ZERO?
		mov	al, BYTE PTR [edi]
		add	al, BYTE PTR [esi]
		sbb	ah, ah
		or	al, ah
		mov	BYTE PTR [edi], al
		add	esi, 4
		inc	edi
		dec	ecx
	.ENDW
	ret

	.ELSE

	test	ecx, ecx
	.WHILE	!ZERO?
		mov	eax, [ebx].rZOrder
		.IF	(DWORD PTR eax) <= (DWORD PTR [ebp])
			mov	al, BYTE PTR [edi]
			add	al, BYTE PTR [esi]
			sbb	ah, ah
			or	al, ah
			mov	BYTE PTR [edi], al
		.ENDIF
		add	ebp, 4
		add	esi, 4
		inc	edi
		dec	ecx
	.ENDW

	.ENDIF
	ret

ALIGN	10H
eglDrawImage@Render_AddColor_MMX:
	test	ebp, ebp
	movq	mm7, mmxConst00FFFFFF
	.IF	ZERO?

	sub	ecx, 4
	.WHILE	!SIGN?
		movq	mm2, MMWORD PTR [esi]
		movq	mm3, MMWORD PTR [esi + 8]
		movq	mm0, MMWORD PTR [edi]
		movq	mm1, MMWORD PTR [edi + 8]
		add	esi, 16
		pand	mm2, mm7
		pand	mm3, mm7
		paddusb	mm0, mm2
		paddusb	mm1, mm3
		movq	MMWORD PTR [edi], mm0
		movq	MMWORD PTR [edi + 8], mm1
		add	edi, 16
		sub	ecx, 4
	.ENDW
	add	ecx, 4
	.WHILE	!ZERO?
		movd	mm1, DWORD PTR [esi]
		movd	mm0, DWORD PTR [edi]
		pand	mm1, mm7
		paddusb	mm0, mm1
		movd	DWORD PTR [edi], mm0
		add	esi, 4
		add	edi, 4
		dec	ecx
	.ENDW
	ret

	.ELSE

	movd	mm6, DWORD PTR [ebx].rZOrder
	sub	ecx, 4
	punpckldq	mm6, mm6
	.WHILE	!SIGN?
		pcmpeqd	mm0, mm0
		movq	mm4, MMWORD PTR [ebp]
		movq	mm5, MMWORD PTR [ebp + 8]
		add	ebp, 16
		psubd	mm4, mm0
		psubd	mm5, mm0
		movq	mm2, MMWORD PTR [esi]
		pcmpgtd	mm4, mm6
		movq	mm3, MMWORD PTR [esi + 8]
		pcmpgtd	mm5, mm6
		pand	mm2, mm4
		movq	mm0, MMWORD PTR [edi]
		pand	mm3, mm5
		movq	mm1, MMWORD PTR [edi + 8]
		add	esi, 16
		paddusb	mm0, mm2
		paddusb	mm1, mm3
		movq	MMWORD PTR [edi], mm0
		movq	MMWORD PTR [edi + 8], mm1
		add	edi, 16
		sub	ecx, 4
	.ENDW
	add	ecx, 4
	mov	eax, [ebx].rZOrder
	.WHILE	!ZERO?
		.IF	(DWORD PTR eax) <= (DWORD PTR [ebp])
			movd	mm1, DWORD PTR [esi]
			movd	mm0, DWORD PTR [edi]
			pand	mm1, mm7
			paddusb	mm0, mm1
			movd	DWORD PTR [edi], mm0
		.ENDIF
		add	ebp, 4
		add	esi, 4
		add	edi, 4
		dec	ecx
	.ENDW

	.ENDIF
	ret

;
;	減算描画
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@Render_SubColor:
	test	ebp, ebp
	.IF	ZERO?

	test	ecx, ecx
	.WHILE	!ZERO?
		mov	al, BYTE PTR [edi]
		mov	dl, BYTE PTR [edi + 1]
		sub	al, BYTE PTR [esi]
		sbb	ah, ah
		sub	dl, BYTE PTR [esi + 1]
		sbb	dh, dh
		not	ah
		not	dh
		and	al, ah
		and	dl, dh
		mov	BYTE PTR [edi], al
		mov	al, BYTE PTR [edi + 2]
		mov	BYTE PTR [edi + 1], dl
		sub	al, BYTE PTR [esi + 2]
		sbb	ah, ah
		not	ah
		and	al, ah
		mov	BYTE PTR [edi + 2], al
		add	esi, 4
		add	edi, 4
		dec	ecx
	.ENDW
	ret

	.ELSE

	test	ecx, ecx
	.WHILE	!ZERO?
		mov	eax, [ebx].rZOrder
		.IF	(DWORD PTR eax) <= (DWORD PTR [ebp])
			mov	al, BYTE PTR [edi]
			mov	dl, BYTE PTR [edi + 1]
			sub	al, BYTE PTR [esi]
			sbb	ah, ah
			sub	dl, BYTE PTR [esi + 1]
			sbb	dh, dh
			not	ah
			not	dh
			and	al, ah
			and	dl, dh
			mov	BYTE PTR [edi], al
			mov	al, BYTE PTR [edi + 2]
			mov	BYTE PTR [edi + 1], dl
			sub	al, BYTE PTR [esi + 2]
			sbb	ah, ah
			not	ah
			and	al, ah
			mov	BYTE PTR [edi + 2], al
		.ENDIF
		add	ebp, 4
		add	esi, 4
		add	edi, 4
		dec	ecx
	.ENDW

	.ENDIF
	ret

ALIGN	10H
eglDrawImage@Render_SubColor8:
	test	ebp, ebp
	.IF	ZERO?

	test	ecx, ecx
	.WHILE	!ZERO?
		mov	al, BYTE PTR [edi]
		sub	al, BYTE PTR [esi]
		sbb	ah, ah
		not	ah
		and	al, ah
		mov	BYTE PTR [edi], al
		add	esi, 4
		inc	edi
		dec	ecx
	.ENDW
	ret

	.ELSE

	test	ecx, ecx
	.WHILE	!ZERO?
		mov	eax, [ebx].rZOrder
		.IF	(DWORD PTR eax) <= (DWORD PTR [ebp])
			mov	al, BYTE PTR [edi]
			sub	al, BYTE PTR [esi]
			sbb	ah, ah
			not	ah
			and	al, ah
			mov	BYTE PTR [edi], al
		.ENDIF
		add	ebp, 4
		add	esi, 4
		inc	edi
		dec	ecx
	.ENDW

	.ENDIF
	ret

ALIGN	10H
eglDrawImage@Render_SubColor_MMX:
	test	ebp, ebp
	movq	mm7, mmxConst00FFFFFF
	.IF	ZERO?

	sub	ecx, 4
	.WHILE	!SIGN?
		movq	mm2, MMWORD PTR [esi]
		movq	mm3, MMWORD PTR [esi + 8]
		movq	mm0, MMWORD PTR [edi]
		movq	mm1, MMWORD PTR [edi + 8]
		add	esi, 16
		pand	mm2, mm7
		pand	mm3, mm7
		psubusb	mm0, mm2
		psubusb	mm1, mm3
		movq	MMWORD PTR [edi], mm0
		movq	MMWORD PTR [edi + 8], mm1
		add	edi, 16
		sub	ecx, 4
	.ENDW
	add	ecx, 4
	.WHILE	!ZERO?
		movd	mm1, DWORD PTR [esi]
		movd	mm0, DWORD PTR [edi]
		pand	mm1, mm7
		psubusb	mm0, mm1
		movd	DWORD PTR [edi], mm0
		add	esi, 4
		add	edi, 4
		dec	ecx
	.ENDW
	ret

	.ELSE

	movd	mm6, DWORD PTR [ebx].rZOrder
	sub	ecx, 4
	punpckldq	mm6, mm6
	.WHILE	!SIGN?
		pcmpeqd	mm0, mm0
		movq	mm4, MMWORD PTR [ebp]
		movq	mm5, MMWORD PTR [ebp + 8]
		add	ebp, 16
		psubd	mm4, mm0
		psubd	mm5, mm0
		movq	mm2, MMWORD PTR [esi]
		pcmpgtd	mm4, mm6
		movq	mm3, MMWORD PTR [esi + 8]
		pcmpgtd	mm5, mm6
		movq	mm0, MMWORD PTR [edi]
		pand	mm2, mm4
		movq	mm1, MMWORD PTR [edi + 8]
		pand	mm3, mm5
		add	esi, 16
		pand	mm2, mm7
		pand	mm3, mm7
		psubusb	mm0, mm2
		psubusb	mm1, mm3
		movq	MMWORD PTR [edi], mm0
		movq	MMWORD PTR [edi + 8], mm1
		add	edi, 16
		sub	ecx, 4
	.ENDW
	add	ecx, 4
	mov	eax, [ebx].rZOrder
	.WHILE	!ZERO?
		.IF	(DWORD PTR eax) <= (DWORD PTR [ebp])
			movd	mm1, DWORD PTR [esi]
			movd	mm0, DWORD PTR [edi]
			pand	mm1, mm7
			psubusb	mm0, mm1
			movd	DWORD PTR [edi], mm0
		.ENDIF
		add	ebp, 4
		add	esi, 4
		add	edi, 4
		dec	ecx
	.ENDW

	.ENDIF
	ret

;
;	積算描画
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@Render_MulColor:
	test	ebp, ebp
	.IF	ZERO?

	test	ecx, ecx
	.WHILE	!ZERO?
		push	ecx
		movzx	eax, BYTE PTR [esi]
		movzx	edx, BYTE PTR [esi + 1]
		movzx	ecx, BYTE PTR [edi]
		movzx	ebp, BYTE PTR [edi + 1]
		inc	eax
		inc	edx
		imul	eax, ecx
		movzx	ecx, BYTE PTR [esi + 2]
		imul	edx, ebp
		movzx	ebp, BYTE PTR [edi + 2]
		inc	ecx
		imul	ecx, ebp
		mov	BYTE PTR [edi], ah
		mov	BYTE PTR [edi + 1], dh
		mov	BYTE PTR [edi + 2], ch
		pop	ecx
		add	esi, 4
		add	edi, 4
		dec	ecx
	.ENDW
	ret

	.ELSE

	test	ecx, ecx
	.WHILE	!ZERO?
		mov	eax, [ebx].rZOrder
		.IF	(DWORD PTR eax) <= (DWORD PTR [ebp])
			push	ecx
			push	ebp
			movzx	eax, BYTE PTR [esi]
			movzx	edx, BYTE PTR [esi + 1]
			movzx	ecx, BYTE PTR [edi]
			movzx	ebp, BYTE PTR [edi + 1]
			inc	eax
			inc	edx
			imul	eax, ecx
			movzx	ecx, BYTE PTR [esi + 2]
			imul	edx, ebp
			movzx	ebp, BYTE PTR [edi + 2]
			inc	ecx
			imul	ecx, ebp
			mov	BYTE PTR [edi], ah
			mov	BYTE PTR [edi + 1], dh
			mov	BYTE PTR [edi + 2], ch
			pop	ebp
			pop	ecx
		.ENDIF
		add	ebp, 4
		add	esi, 4
		add	edi, 4
		dec	ecx
	.ENDW

	.ENDIF
	ret

ALIGN	10H
eglDrawImage@Render_MulColor8:
	test	ebp, ebp
	.IF	ZERO?

	test	ecx, ecx
	.WHILE	!ZERO?
		movzx	eax, BYTE PTR [esi]
		movzx	edx, BYTE PTR [edi]
		inc	eax
		imul	eax, edx
		mov	BYTE PTR [edi], ah
		add	esi, 4
		inc	edi
		dec	ecx
	.ENDW
	ret

	.ELSE

	test	ecx, ecx
	.WHILE	!ZERO?
		mov	eax, [ebx].rZOrder
		.IF	(DWORD PTR eax) <= (DWORD PTR [ebp])
			movzx	eax, BYTE PTR [esi]
			movzx	edx, BYTE PTR [edi]
			inc	eax
			imul	eax, edx
			mov	BYTE PTR [edi], ah
		.ENDIF
		add	ebp, 4
		add	esi, 4
		inc	edi
		dec	ecx
	.ENDW

	.ENDIF
	ret


ALIGN	10H
eglDrawImage@Render_MulColor_MMX:
	test	ebp, ebp
	.IF	ZERO?

	sub	ecx, 2
	pxor	mm7, mm7
	movq	mm6, mmxConstFF000000
	pcmpeqw	mm5, mm5
	.WHILE	!SIGN?
		movq	mm0, MMWORD PTR [esi]
		movq	mm2, MMWORD PTR [edi]
		por	mm0, mm6
		movq	mm3, mm2
		movq	mm1, mm0
		punpcklbw	mm0, mm7
		punpcklbw	mm2, mm7
		punpckhbw	mm1, mm7
		punpckhbw	mm3, mm7
		psubw	mm0, mm5
		psubw	mm1, mm5
		pmullw	mm0, mm2
		pmullw	mm1, mm3
		psrlw	mm0, 8
		psrlw	mm1, 8
		packuswb	mm0, mm1
		movq	MMWORD PTR [edi], mm0
		add	esi, 8
		add	edi, 8
		sub	ecx, 2
	.ENDW
	add	ecx, 2
	.IF	!ZERO?
		movd	mm0, DWORD PTR [esi]
		movd	mm2, DWORD PTR [edi]
		por	mm0, mm6
		punpcklbw	mm0, mm7
		punpcklbw	mm2, mm7
		psubw	mm0, mm5
		pmullw	mm0, mm2
		psrlw	mm0, 8
		packuswb	mm0, mm7
		movd	DWORD PTR [edi], mm0
	.ENDIF

	.ELSE

	movd	mm6, DWORD PTR [ebx].rZOrder
	sub	ecx, 2
	pxor	mm7, mm7
	punpckldq	mm6, mm6
	pcmpeqw	mm5, mm5
	.WHILE	!SIGN?
		movq	mm4, MMWORD PTR [ebp]
		movq	mm0, MMWORD PTR [esi]
		psubd	mm4, mm5
		movq	mm2, MMWORD PTR [edi]
		pcmpgtd	mm4, mm6
		por	mm0, mmxConstFF000000
		movq	mm3, mm2
		por	mm0, mm4
		punpcklbw	mm2, mm7
		movq	mm1, mm0
		punpcklbw	mm0, mm7
		punpckhbw	mm3, mm7
		punpckhbw	mm1, mm7
		psubw	mm0, mm5
		psubw	mm1, mm5
		pmullw	mm0, mm2
		pmullw	mm1, mm3
		psrlw	mm0, 8
		psrlw	mm1, 8
		packuswb	mm0, mm1
		movq	MMWORD PTR [edi], mm0
		add	ebp, 8
		add	esi, 8
		add	edi, 8
		sub	ecx, 2
	.ENDW
	add	ecx, 2
	.IF	!ZERO?
		movd	mm4, DWORD PTR [ebp]
		movd	mm0, DWORD PTR [esi]
		psubd	mm4, mm5
		movd	mm2, DWORD PTR [edi]
		pcmpgtd	mm4, mm6
		por	mm0, mmxConstFF000000
		punpcklbw	mm2, mm7
		por	mm0, mm6
		punpcklbw	mm0, mm7
		psubw	mm0, mm5
		pmullw	mm0, mm2
		psrlw	mm0, 8
		packuswb	mm0, mm7
		movd	DWORD PTR [edi], mm0
	.ENDIF

	.ENDIF
	ret

;
;	覆い焼き描画
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@Render_DivColor:
	test	ebp, ebp
	.IF	ZERO?

	test	ecx, ecx
	.WHILE	!ZERO?
		push	ecx
		movzx	ebp, BYTE PTR [esi]
		movzx	edx, BYTE PTR [esi + 1]
		movzx	eax, BYTE PTR [edi]
		movzx	ecx, BYTE PTR [edi + 1]
		imul	eax, TableDivScale8[ebp*4]
		movzx	ebp, BYTE PTR [esi + 2]
		imul	ecx, TableDivScale8[edx*4]
		movzx	edx, BYTE PTR [edi + 2]
		shr	eax, 6
		imul	edx, TableDivScale8[ebp*4]
		shr	ecx, 6
		add	ah, 0FFH
		sbb	ah, ah
		add	ch, 0FFH
		sbb	ch, ch
		or	al, ah
		or	cl, ch
		mov	BYTE PTR [edi], al
		mov	BYTE PTR [edi + 1], cl
		pop	ecx
		shr	edx, 6
		add	dh, 0FFH
		sbb	dh, dh
		add	esi, 4
		or	dl, dh
		mov	BYTE PTR [edi + 2], dl
		add	edi, 4
		dec	ecx
	.ENDW
	ret

	.ELSE

	test	ecx, ecx
	.WHILE	!ZERO?
		mov	eax, [ebx].rZOrder
		.IF	(DWORD PTR eax) <= (DWORD PTR [ebp])
			push	ecx
			push	ebp
			movzx	ebp, BYTE PTR [esi]
			movzx	edx, BYTE PTR [esi + 1]
			movzx	eax, BYTE PTR [edi]
			movzx	ecx, BYTE PTR [edi + 1]
			imul	eax, TableDivScale8[ebp*4]
			movzx	ebp, BYTE PTR [esi + 2]
			imul	ecx, TableDivScale8[edx*4]
			movzx	edx, BYTE PTR [edi + 2]
			shr	eax, 6
			imul	edx, TableDivScale8[ebp*4]
			shr	ecx, 6
			add	ah, 0FFH
			sbb	ah, ah
			add	ch, 0FFH
			sbb	ch, ch
			or	al, ah
			or	cl, ch
			mov	BYTE PTR [edi], al
			mov	BYTE PTR [edi + 1], cl
			pop	ebp
			shr	edx, 6
			pop	ecx
			add	dh, 0FFH
			sbb	dh, dh
			or	dl, dh
			mov	BYTE PTR [edi + 2], dl
		.ENDIF
		add	ebp, 4
		add	esi, 4
		add	edi, 4
		dec	ecx
	.ENDW

	.ENDIF
	ret

ALIGN	10H
eglDrawImage@Render_DivColor8:
	test	ebp, ebp
	.IF	ZERO?

	test	ecx, ecx
	.WHILE	!ZERO?
		movzx	edx, BYTE PTR [esi]
		movzx	eax, BYTE PTR [edi]
		imul	eax, TableDivScale8[edx*4]
		shr	eax, 6
		add	ah, 0FFH
		sbb	ah, ah
		or	al, ah
		mov	BYTE PTR [edi], al
		add	esi, 4
		inc	edi
	.ENDW
	ret

	.ELSE

	test	ecx, ecx
	.WHILE	!ZERO?
		mov	eax, [ebx].rZOrder
		.IF	(DWORD PTR eax) <= (DWORD PTR [ebp])
			movzx	edx, BYTE PTR [esi]
			movzx	eax, BYTE PTR [edi]
			imul	eax, TableDivScale8[edx*4]
			shr	eax, 6
			add	ah, 0FFH
			sbb	ah, ah
			or	al, ah
			mov	BYTE PTR [edi], al
		.ENDIF
		add	ebp, 4
		add	esi, 4
		inc	edi
		dec	ecx
	.ENDW

	.ENDIF
	ret

ALIGN	10H
eglDrawImage@Render_DivColor_MMX:
	test	ebp, ebp
	.IF	ZERO?

	test	ecx, ecx
	pxor	mm7, mm7
	.WHILE	!ZERO?
		movzx	eax, BYTE PTR [esi]
		movzx	edx, BYTE PTR [esi + 1]
		movd	mm0, DWORD PTR [edi]
		movd	mm2, TableDivScale8[eax*4]
		movzx	eax, BYTE PTR [esi + 2]
		movd	mm3, TableDivScale8[edx*4]
		mov	edx, 40H
		punpcklbw	mm0, mm7
		punpckldq	mm2, mm3
		movd	mm3, TableDivScale8[eax*4]
		movd	mm4, edx
		movq	mm1, mm0
		punpcklwd	mm0, mm7
		punpckldq	mm3, mm4
		punpckhwd	mm1, mm7
		pmaddwd	mm0, mm2
		pmaddwd	mm1, mm3
		psrld	mm0, 6
		psrld	mm1, 6
		packssdw	mm0, mm1
		add	esi, 4
		packuswb	mm0, mm7
		movd	DWORD PTR [edi], mm0
		add	edi, 4
		dec	ecx
	.ENDW
	ret

	.ELSE

	test	ecx, ecx
	pxor	mm7, mm7
	.WHILE	!ZERO?
		mov	eax, [ebx].rZOrder
		.IF	(DWORD PTR eax) <= (DWORD PTR [ebp])
			movzx	eax, BYTE PTR [esi]
			movzx	edx, BYTE PTR [esi + 1]
			movd	mm0, DWORD PTR [edi]
			movd	mm2, TableDivScale8[eax*4]
			movzx	eax, BYTE PTR [esi + 2]
			movd	mm3, TableDivScale8[edx*4]
			mov	edx, 40H
			punpcklbw	mm0, mm7
			punpckldq	mm2, mm3
			movd	mm3, TableDivScale8[eax*4]
			movd	mm4, edx
			movq	mm1, mm0
			punpcklwd	mm0, mm7
			punpckldq	mm3, mm4
			punpckhwd	mm1, mm7
			pmaddwd	mm0, mm2
			pmaddwd	mm1, mm3
			psrld	mm0, 6
			psrld	mm1, 6
			packssdw	mm0, mm1
			packuswb	mm0, mm7
			movd	DWORD PTR [edi], mm0
		.ENDIF
		add	ebp, 4
		add	esi, 4
		add	edi, 4
		dec	ecx
	.ENDW
	ret

	.ENDIF
	ret


;
;	最大値選択
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@Render_MaxValue:
	test	ecx, ecx
	.WHILE	!ZERO?
		test	ebp, ebp
		mov	eax, [ebx].rZOrder
		.IF	!ZERO?
			cmp	eax, DWORD PTR [ebp]
			lea	ebp, [ebp + 4]
			ja	@f
		.ENDIF
		push	ecx
		mov	al, BYTE PTR [esi]
		mov	cl, BYTE PTR [esi + 1]
		mov	ah, BYTE PTR [edi]
		mov	ch, BYTE PTR [edi + 1]
		cmp	al, ah
		sbb	dl, dl
		cmp	cl, ch
		sbb	dh, dh
		and	ah, dl
		not	dl
		and	ch, dh
		not	dh
		and	al, dl
		and	cl, dh
		or	al, ah
		or	cl, ch
		mov	BYTE PTR [edi], al
		mov	BYTE PTR [edi + 1], cl
		;
		mov	al, BYTE PTR [esi]
		mov	cl, BYTE PTR [esi + 1]
		mov	ah, BYTE PTR [edi]
		mov	ch, BYTE PTR [edi + 1]
		cmp	al, ah
		sbb	dl, dl
		cmp	cl, ch
		sbb	dh, dh
		and	ah, dl
		not	dl
		and	ch, dh
		not	dh
		and	al, dl
		and	cl, dh
		or	al, ah
		or	cl, ch
		mov	BYTE PTR [edi], al
		mov	BYTE PTR [edi + 1], cl
		pop	ecx
		;
@@:		add	esi, 4
		add	edi, 4
		dec	ecx
	.ENDW
	ret

ALIGN	10H
eglDrawImage@Render_MaxValue8:
	test	ebp, ebp
	.IF	ZERO?

	test	ecx, ecx
	.WHILE	!ZERO?
		mov	al, BYTE PTR [esi]
		mov	ah, BYTE PTR [edi]
		cmp	al, ah
		sbb	dl, dl
		and	ah, dl
		not	dl
		and	al, dl
		or	al, ah
		mov	BYTE PTR [edi], al
		add	esi, 4
		inc	edi
		dec	ecx
	.ENDW
	ret

	.ELSE

	test	ecx, ecx
	.WHILE	!ZERO?
		mov	eax, [ebx].rZOrder
		.IF	(DWORD PTR eax) <= (DWORD PTR [ebp])
			mov	al, BYTE PTR [esi]
			mov	ah, BYTE PTR [edi]
			cmp	al, ah
			sbb	dl, dl
			and	ah, dl
			not	dl
			and	al, dl
			or	al, ah
			mov	BYTE PTR [edi], al
		.ENDIF
		add	ebp, 4
		add	esi, 4
		inc	edi
		dec	ecx
	.ENDW

	.ENDIF
	ret

ALIGN	10H
eglDrawImage@Render_MaxValueSSE:
	test	ebp, ebp
	.IF	ZERO?
		sub	ecx, 4
		.WHILE	!SIGN?
			movq	mm0, MMWORD PTR [edi]
			movq	mm1, MMWORD PTR [esi]
			movq	mm2, MMWORD PTR [edi + 8]
			movq	mm3, MMWORD PTR [esi + 8]
			pmaxub	mm0, mm1
			pmaxub	mm2, mm3
			movq	MMWORD PTR [edi], mm0
			movq	MMWORD PTR [edi + 8], mm2
			add	edi, 16
			add	esi, 16
			sub	ecx, 4
		.ENDW
		add	ecx, 4
	.ENDIF
	;
	mov	eax, [ebx].rZOrder
	test	ecx, ecx
	.WHILE	!ZERO?
		test	ebp, ebp
		.IF	!ZERO?
			cmp	eax, DWORD PTR [ebp]
			lea	ebp, [ebp + 4]
			ja	@f
		.ENDIF
		movd	mm0, DWORD PTR [edi]
		movd	mm1, DWORD PTR [esi]
		pmaxub	mm0, mm1
		movd	DWORD PTR [edi], mm0
@@:		add	edi, 4
		add	esi, 4
		dec	ecx
	.ENDW
	ret

ALIGN	10H
eglDrawImage@Render_MaxValue8_SSE:
	test	ebp, ebp
	jnz	eglDrawImage@Render_MaxValue8

	sub	ecx, 16
	.WHILE	!SIGN?
		movq	mm0, MMWORD PTR [edi]
		movq	mm1, MMWORD PTR [esi]
		movq	mm2, MMWORD PTR [edi + 8]
		movq	mm3, MMWORD PTR [esi + 8]
		pmaxub	mm0, mm1
		pmaxub	mm2, mm3
		movq	MMWORD PTR [edi], mm0
		movq	MMWORD PTR [edi + 8], mm2
		add	edi, 16
		add	esi, 16
		sub	ecx, 16
	.ENDW
	add	ecx, 16
	.WHILE	!ZERO?
		mov	al, BYTE PTR [esi]
		mov	ah, BYTE PTR [edi]
		cmp	al, ah
		sbb	dl, dl
		and	ah, dl
		not	dl
		and	al, dl
		or	al, ah
		mov	BYTE PTR [edi], al
		add	esi, 4
		inc	edi
		dec	ecx
	.ENDW
	ret


;
;	最小値選択
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@Render_MinValue:
	test	ecx, ecx
	.WHILE	!ZERO?
		test	ebp, ebp
		mov	eax, [ebx].rZOrder
		.IF	!ZERO?
			cmp	eax, DWORD PTR [ebp]
			lea	ebp, [ebp + 4]
			ja	@f
		.ENDIF
		push	ecx
		mov	al, BYTE PTR [esi]
		mov	cl, BYTE PTR [esi + 1]
		mov	ah, BYTE PTR [edi]
		mov	ch, BYTE PTR [edi + 1]
		cmp	al, ah
		sbb	dl, dl
		cmp	cl, ch
		sbb	dh, dh
		and	al, dl
		not	dl
		and	cl, dh
		not	dh
		and	ah, dl
		and	ch, dh
		or	al, ah
		or	cl, ch
		mov	BYTE PTR [edi], al
		mov	BYTE PTR [edi + 1], cl
		;
		mov	al, BYTE PTR [esi]
		mov	cl, BYTE PTR [esi + 1]
		mov	ah, BYTE PTR [edi]
		mov	ch, BYTE PTR [edi + 1]
		cmp	al, ah
		sbb	dl, dl
		cmp	cl, ch
		sbb	dh, dh
		and	al, dl
		not	dl
		and	cl, dh
		not	dh
		and	ah, dl
		and	ch, dh
		or	al, ah
		or	cl, ch
		mov	BYTE PTR [edi], al
		mov	BYTE PTR [edi + 1], cl
		pop	ecx
		;
@@:		add	esi, 4
		add	edi, 4
		dec	ecx
	.ENDW
	ret

ALIGN	10H
eglDrawImage@Render_MinValue8:
	test	ebp, ebp
	.IF	ZERO?

	test	ecx, ecx
	.WHILE	!ZERO?
		mov	al, BYTE PTR [esi]
		mov	ah, BYTE PTR [edi]
		cmp	al, ah
		sbb	dl, dl
		and	al, dl
		not	dl
		and	ah, dl
		or	al, ah
		mov	BYTE PTR [edi], al
		add	esi, 4
		inc	edi
		dec	ecx
	.ENDW
	ret

	.ELSE

	test	ecx, ecx
	.WHILE	!ZERO?
		mov	eax, [ebx].rZOrder
		.IF	(DWORD PTR eax) <= (DWORD PTR [ebp])
			mov	al, BYTE PTR [esi]
			mov	ah, BYTE PTR [edi]
			cmp	al, ah
			sbb	dl, dl
			and	al, dl
			not	dl
			and	ah, dl
			or	al, ah
			mov	BYTE PTR [edi], al
		.ENDIF
		add	ebp, 4
		add	esi, 4
		inc	edi
		dec	ecx
	.ENDW

	.ENDIF
	ret

ALIGN	10H
eglDrawImage@Render_MinValueSSE:
	test	ebp, ebp
	.IF	ZERO?
		sub	ecx, 4
		.WHILE	!SIGN?
			movq	mm0, MMWORD PTR [edi]
			movq	mm1, MMWORD PTR [esi]
			movq	mm2, MMWORD PTR [edi + 8]
			movq	mm3, MMWORD PTR [esi + 8]
			pminub	mm0, mm1
			pminub	mm2, mm3
			movq	MMWORD PTR [edi], mm0
			movq	MMWORD PTR [edi + 8], mm2
			add	edi, 16
			add	esi, 16
			sub	ecx, 4
		.ENDW
		add	ecx, 4
	.ENDIF
	;
	mov	eax, [ebx].rZOrder
	test	ecx, ecx
	.WHILE	!ZERO?
		test	ebp, ebp
		.IF	!ZERO?
			cmp	eax, DWORD PTR [ebp]
			lea	ebp, [ebp + 4]
			ja	@f
		.ENDIF
		movd	mm0, DWORD PTR [edi]
		movd	mm1, DWORD PTR [esi]
		pminub	mm0, mm1
		movd	DWORD PTR [edi], mm0
@@:		add	edi, 4
		add	esi, 4
		dec	ecx
	.ENDW
	ret

ALIGN	10H
eglDrawImage@Render_MinValue8_SSE:
	test	ebp, ebp
	jnz	eglDrawImage@Render_MinValue8

	sub	ecx, 16
	.WHILE	!SIGN?
		movq	mm0, MMWORD PTR [edi]
		movq	mm1, MMWORD PTR [esi]
		movq	mm2, MMWORD PTR [edi + 8]
		movq	mm3, MMWORD PTR [esi + 8]
		pminub	mm0, mm1
		pminub	mm2, mm3
		movq	MMWORD PTR [edi], mm0
		movq	MMWORD PTR [edi + 8], mm2
		add	edi, 16
		add	esi, 16
		sub	ecx, 16
	.ENDW
	add	ecx, 16
	.WHILE	!ZERO?
		mov	al, BYTE PTR [esi]
		mov	ah, BYTE PTR [edi]
		cmp	al, ah
		sbb	dl, dl
		and	al, dl
		not	dl
		and	ah, dl
		or	al, ah
		mov	BYTE PTR [edi], al
		add	esi, 4
		inc	edi
		dec	ecx
	.ENDW
	ret

;
;	反転乗算（スクリーン）描画
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@Render_NegMulColor:
	test	ecx, ecx
	.WHILE	!ZERO?
		test	ebp, ebp
		mov	eax, [ebx].rZOrder
		.IF	!ZERO?
			cmp	eax, DWORD PTR [ebp]
			lea	ebp, [ebp + 4]
			ja	@f
		.ENDIF
		;
		push	ebx
		push	ecx
		;
		movzx	eax, BYTE PTR [esi]
		movzx	edx, BYTE PTR [edi]
		xor	eax, 0FFH
		xor	edx, 0FFH
			movzx	ecx, BYTE PTR [esi + 1]
		inc	eax
			movzx	ebx, BYTE PTR [edi + 1]
		imul	eax, edx
			xor	ecx, 0FFH
			xor	ebx, 0FFH
			inc	ecx
			imul	ecx, ebx
		not	ah
			not	ch
		mov	BYTE PTR [edi], ah
			mov	BYTE PTR [edi + 1], ch
		;
		movzx	eax, BYTE PTR [esi + 2]
		movzx	edx, BYTE PTR [edi + 2]
		xor	eax, 0FFH
		xor	edx, 0FFH
		inc	eax
		imul	eax, edx
		not	ah
		mov	BYTE PTR [edi + 2], ah
		pop	ecx
		pop	ebx
		;
@@:		add	esi, 4
		add	edi, 4
		dec	ecx
	.ENDW
	ret

;
;	α直接合成
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@Render_MoveColor:
	test	ecx, ecx
	.WHILE	!ZERO?
		test	ebp, ebp
		mov	eax, [ebx].rZOrder
		.IF	!ZERO?
			cmp	eax, DWORD PTR [ebp]
			lea	ebp, [ebp + 4]
			ja	@f
			mov	DWORD PTR [ebp - 4], eax
		.ENDIF
		movzx	eax, BYTE PTR [edi]
		movzx	edx, BYTE PTR [edi + 1]
		mov	al, [ebx].nGreenTone[eax]
		mov	dl, [ebx].nGreenTone[edx]
		add	al, BYTE PTR [esi]
		sbb	ah, ah
		add	dl, BYTE PTR [esi + 1]
		sbb	dh, dh
		or	al, ah
		or	dl, dh
		mov	BYTE PTR [edi], al
		mov	BYTE PTR [edi + 1], dl
		movzx	eax, BYTE PTR [edi + 2]
		movzx	edx, BYTE PTR [edi + 3]
		mov	al, [ebx].nGreenTone[eax]
		mov	dl, [ebx].nGreenTone[edx]
		add	al, BYTE PTR [esi + 2]
		sbb	ah, ah
		add	dl, BYTE PTR [esi + 3]
		sbb	dh, dh
		or	al, ah
		or	dl, dh
		mov	BYTE PTR [edi + 2], al
		mov	BYTE PTR [edi + 3], dl
@@:		add	edi, 4
		add	esi, 4
		dec	ecx
	.ENDW
	ret

ALIGN	10H
eglDrawImage@Render_MoveColor_MMX:
	movd	mm6, [ebx].nTrans
	pxor	mm7, mm7
	punpcklwd	mm6, mm6
	mov	eax, [ebx].rZOrder
	punpckldq	mm6, mm6
	test	ecx, ecx
	.WHILE	!ZERO?
		test	ebp, ebp
		.IF	!ZERO?
			cmp	eax, DWORD PTR [ebp]
			lea	ebp, [ebp + 4]
			ja	@f
			mov	DWORD PTR [ebp - 4], eax
		.ENDIF
		movd	mm0, DWORD PTR [edi]
		movd	mm1, DWORD PTR [esi]
		punpcklbw	mm0, mm7
		pmullw	mm0, mm6
		psrlw	mm0, 8
		packuswb	mm0, mm7
		paddusb	mm0, mm1
		movd	DWORD PTR [edi], mm0
@@:		add	edi, 4
		add	esi, 4
		dec	ecx
	.ENDW
	ret

;
;	α乗算合成描画
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@Render_AMulColor:
	test	ecx, ecx
	.WHILE	!ZERO?
		test	ebp, ebp
		mov	eax, [ebx].rZOrder
		.IF	!ZERO?
			cmp	eax, DWORD PTR [ebp]
			lea	ebp, [ebp + 4]
			ja	@f
			mov	DWORD PTR [ebp - 4], eax
		.ENDIF
		push	ecx
		push	ebx
		movzx	edx, BYTE PTR [edi + 1]
		movzx	eax, BYTE PTR [edi]
		shl	edx, 16
		movzx	ecx, BYTE PTR [edi + 3]
		or	eax, edx
		shl	ecx, 16
		movzx	edx, BYTE PTR [edi + 2]
		or	edx, ecx
		movzx	ecx, BYTE PTR [esi + 3]
		inc	ecx
		imul	eax, ecx
		imul	edx, ecx
		mov	ebx, eax
		mov	ecx, edx
		shr	eax, 16
		shr	edx, 16
		add	bh, BYTE PTR [esi]
		sbb	bl, bl
		add	ah, BYTE PTR [esi + 1]
		sbb	al, al
		add	ch, BYTE PTR [esi + 2]
		sbb	cl, cl
		mov	BYTE PTR [edi + 3], dh
		or	bl, bh
		or	al, ah
		or	cl, ch
		mov	BYTE PTR [edi], bl
		pop	ebx
		mov	BYTE PTR [edi + 1], al
		mov	BYTE PTR [edi + 2], cl
		pop	ecx
@@:		add	edi, 4
		add	esi, 4
		dec	ecx
	.ENDW
	ret

ALIGN	10H
eglDrawImage@Render_AMulColor_MMX:
	pxor	mm7, mm7
	movq	mm6, mmxMaskLow3Words
	test	ecx, ecx
	.WHILE	!ZERO?
		test	ebp, ebp
		mov	eax, [ebx].rZOrder
		.IF	!ZERO?
			cmp	eax, DWORD PTR [ebp]
			lea	ebp, [ebp + 4]
			ja	@f
			mov	DWORD PTR [ebp - 4], eax
		.ENDIF
		movzx	eax, BYTE PTR [esi + 3]
		movd	mm0, DWORD PTR [edi]
		inc	eax
		movd	mm1, DWORD PTR [esi]
		movd	mm4, eax
		punpcklbw	mm0, mm7
		punpcklwd	mm4, mm4
		punpcklbw	mm1, mm7
		punpckldq	mm4, mm4
		pand	mm1, mm6
		pmullw	mm0, mm4
		psrlw	mm0, 8
		paddusw	mm0, mm1
		packuswb	mm0, mm7
		movd	DWORD PTR [edi], mm0
@@:		add	edi, 4
		add	esi, 4
		dec	ecx
	.ENDW
	ret

;
;	出力先α乗算描画
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@Render_DstMaskColor:
	test	ecx, ecx
	.WHILE	!ZERO?
		test	ebp, ebp
		mov	eax, [ebx].rZOrder
		.IF	!ZERO?
			cmp	eax, DWORD PTR [ebp]
			lea	ebp, [ebp + 4]
			ja	@f
			mov	DWORD PTR [ebp - 4], eax
		.ENDIF
		push	ecx
		push	ebx
		push	ebp
		movzx	edx, BYTE PTR [esi + 1]
		movzx	eax, BYTE PTR [esi]
		shl	edx, 16
		movzx	ecx, BYTE PTR [esi + 3]
		or	eax, edx
		shl	ecx, 16
		movzx	ebp, BYTE PTR [esi + 2]
		or	ebp, ecx
		movzx	ecx, BYTE PTR [edi + 3]
		inc	ecx
		imul	eax, ecx
		imul	ebp, ecx
		mov	ebx, eax
		mov	ecx, ebp
		shr	eax, 16
		shr	ebp, 16 + 8
			movzx	edx, BYTE PTR [edi]
			xor	ebp, 0FFH
			inc	ebp
			imul	edx, ebp
		add	bh, dh
			movzx	edx, BYTE PTR [edi + 1]
		sbb	bl, bl
			imul	edx, ebp
		add	ah, dh
			movzx	edx, BYTE PTR [edi + 2]
		sbb	al, al
			imul	edx, ebp
		add	ch, dh
		sbb	cl, cl
		or	bl, bh
		or	al, ah
		or	cl, ch
		pop	ebp
		mov	BYTE PTR [edi], bl
		pop	ebx
		mov	BYTE PTR [edi + 1], al
		mov	BYTE PTR [edi + 2], cl
		pop	ecx
@@:		add	edi, 4
		add	esi, 4
		dec	ecx
	.ENDW
	ret



; ----------------------------------------------------------------------------
;	描画関数
; ----------------------------------------------------------------------------

;
;	画像フォーマット変換
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@DrawImage_ConvertFormat	PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE

	mov	ebx, hDrawImage
	INVOKE	eglConvertFormat , ADDR [ebx].dstimg, ADDR [ebx].srcimg, 0
	ret

eglDrawImage@DrawImage_ConvertFormat	ENDP

;
;	{RGB32|RGBA32}->{RGB24|RGB32|RGBA32} 変形なし描画
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@DrawImage_DirectRGBA32	PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE

	mov	ebx, hDrawImage
	mov	[ebx].dwSaveRegEBP, ebp
	pushfd
	cld
	;
	mov	ecx, [ebx].srcimg.dwImageHeight
	mov	esi, [ebx].srcimg.ptrImageArray
	mov	edi, [ebx].dstimg.ptrImageArray
	mov	ebp, [ebx].zbuf.ptrImageArray
	test	ecx, ecx
	.WHILE	!ZERO?
		mov	[ebx].nLeftHeight, ecx
		mov	[ebx].ptrSrcLine, esi
		mov	[ebx].ptrDstLine, edi
		mov	[ebx].ptrZBufLine, ebp
		;
		mov	ecx, [ebx].srcimg.dwImageWidth
		call	[ebx].pfnDrawRendering
		;
		mov	esi, [ebx].ptrSrcLine
		mov	edi, [ebx].ptrDstLine
		mov	ebp, [ebx].ptrZBufLine
		mov	ecx, [ebx].nLeftHeight
		add	esi, [ebx].srcimg.dwBytesPerLine
		add	edi, [ebx].dstimg.dwBytesPerLine
		add	ebp, [ebx].zbuf.dwBytesPerLine
		dec	ecx
	.ENDW
	;
	.IF	ERI_EnabledProcessorType & ERI_USE_XMM_P3
		sfence
		emms
	.ELSEIF	ERI_EnabledProcessorType & ERI_USE_MMX_PENTIUM
		emms
	.ENDIF
	popfd
	mov	ebp, [ebx].dwSaveRegEBP
	xor	eax, eax
	ret

eglDrawImage@DrawImage_DirectRGBA32	ENDP

;
;	汎用変形なし描画
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@DrawImage_Normal		PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE

	mov	ebx, hDrawImage
	mov	[ebx].dwSaveRegEBP, ebp
	pushfd
	cld
	;
	mov	ecx, [ebx].srcimg.dwImageHeight
	mov	esi, [ebx].srcimg.ptrImageArray
	mov	edi, [ebx].dstimg.ptrImageArray
	mov	ebp, [ebx].zbuf.ptrImageArray
	test	ecx, ecx
	.WHILE	!ZERO?
		mov	[ebx].nLeftHeight, ecx
		mov	[ebx].ptrSrcLine, esi
		mov	[ebx].ptrDstLine, edi
		mov	[ebx].ptrZBufLine, ebp
		;
		mov	ecx, [ebx].srcimg.dwImageWidth
		call	[ebx].pfnDrawSampling
		;
		mov	ecx, [ebx].srcimg.dwImageWidth
		call	[ebx].pfnDrawApplying
		;
		mov	esi, [ebx].pDrawLineBuf[0]
		mov	edi, [ebx].ptrDstLine
		mov	ebp, [ebx].ptrZBufLine
		mov	ecx, [ebx].srcimg.dwImageWidth
		call	[ebx].pfnDrawRendering
		;
		mov	esi, [ebx].ptrSrcLine
		mov	edi, [ebx].ptrDstLine
		mov	ebp, [ebx].ptrZBufLine
		mov	ecx, [ebx].nLeftHeight
		add	esi, [ebx].srcimg.dwBytesPerLine
		add	edi, [ebx].dstimg.dwBytesPerLine
		add	ebp, [ebx].zbuf.dwBytesPerLine
		dec	ecx
	.ENDW
	;
	.IF	ERI_EnabledProcessorType & ERI_USE_XMM_P3
		sfence
		emms
	.ELSEIF	ERI_EnabledProcessorType & ERI_USE_MMX_PENTIUM
		emms
	.ENDIF
	popfd
	mov	ebp, [ebx].dwSaveRegEBP
	xor	eax, eax
	ret

eglDrawImage@DrawImage_Normal		ENDP

;
;	汎用変形描画
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@DrawImage_Transform	PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE

	mov	ebx, hDrawImage
	mov	[ebx].dwSaveRegEBP, ebp
	pushfd
	cld
	;
	mov	ecx, [ebx].ptBasePos.x
	mov	edx, [ebx].ptBasePos.y
	mov	[ebx].ptLineBase.x, ecx
	mov	[ebx].ptLineBase.y, edx
	;
	mov	eax, [ebx].dstimg.dwBitsPerPixel
	shr	eax, 3
	mov	[ebx].nBytesPerPixel, eax
	;
	mov	esi, [ebx].pRegion
	ASSUME	esi:PE3D_POLYGON_REGION
	mov	edi, [ebx].dstimg.ptrImageArray
	mov	ebp, [ebx].zbuf.ptrImageArray
	mov	ecx, [esi].nTopLine
	mov	edx, ecx
	imul	ecx, [ebx].dstimg.dwBytesPerLine
	imul	edx, [ebx].zbuf.dwBytesPerLine
	add	edi, ecx
	add	ebp, edx
	mov	[ebx].ptrDstLine, edi
	mov	[ebx].ptrZBufLine, ebp
	mov	ecx, [esi].nBottomLine
	sub	ecx, [esi].nTopLine
	inc	ecx
	.IF	!ZERO?
	lea	esi, [esi].plrLineRgn[0]
	ASSUME	esi:PTR E3D_POLY_LINE_REGION
	;
	.REPEAT
		xor	edx, edx
		mov	[ebx].ptrDstLine, edi
		mov	[ebx].ptrZBufLine, ebp
		test	ebp, ebp
		setnz	dl
		mov	[ebx].ptrRegionLine, esi
		mov	[ebx].nLeftHeight, ecx
		;
		neg	edx
		mov	ecx, [esi].dwReserved1
		mov	eax, [esi].nLeft
		add	ecx, 0FFFFH
		shr	ecx, 16
		add	eax, ecx
		mov	[ebx].dwTemp[0], eax
		mov	ecx, [ebx].ptDltScanX.x
		lea	ebp, [ebp + eax * 4]
		and	ebp, edx
		mov	edx, [ebx].ptDltScanX.y
		imul	ecx, eax
		imul	edx, eax
		imul	eax, [ebx].nBytesPerPixel
		add	[ebx].ptBasePos.x, ecx
		add	[ebx].ptBasePos.y, edx
		add	edi, eax
		;
		mov	ecx, [esi].nRight
;		inc	ecx
;		cmp	[esi].dwReserved2, 8000H
;		sbb	ecx, 0
		cmp	[ebx].rectClip.right, ecx
		sbb	ecx, 0
		sub	ecx, [ebx].dwTemp[0] ; [esi].nLeft
		.IF	!SIGN?
			push	edi
			push	ebp
			inc	ecx
			push	ecx
			call	[ebx].pfnDrawSampling
			;
			mov	ecx, DWORD PTR [esp]
			call	[ebx].pfnDrawApplying
			;
			pop	ecx
			pop	ebp
			pop	edi
			mov	esi, [ebx].pDrawLineBuf[0]
			call	[ebx].pfnDrawRendering
		.ENDIF
		;
		mov	eax, [ebx].ptLineBase.x
		mov	edx, [ebx].ptLineBase.y
		mov	esi, [ebx].ptrRegionLine
		mov	edi, [ebx].ptrDstLine
		mov	ebp, [ebx].ptrZBufLine
		add	eax, [ebx].ptDltScanY.x
		add	edx, [ebx].ptDltScanY.y
		mov	ecx, [ebx].nLeftHeight
		mov	[ebx].ptBasePos.x, eax
		mov	[ebx].ptLineBase.x, eax
		mov	[ebx].ptBasePos.y, edx
		mov	[ebx].ptLineBase.y, edx
		add	esi, (SIZEOF E3D_POLY_LINE_REGION)
		add	edi, [ebx].dstimg.dwBytesPerLine
		add	ebp, [ebx].zbuf.dwBytesPerLine
		dec	ecx
	.UNTIL	ZERO?
	.ENDIF
	;
	.IF	ERI_EnabledProcessorType & ERI_USE_XMM_P3
		sfence
		emms
	.ELSEIF	ERI_EnabledProcessorType & ERI_USE_MMX_PENTIUM
		emms
	.ENDIF
	popfd
	mov	ebp, [ebx].dwSaveRegEBP
	xor	eax, eax
	ret

eglDrawImage@DrawImage_Transform	ENDP


CodeSeg	ENDS

	END
