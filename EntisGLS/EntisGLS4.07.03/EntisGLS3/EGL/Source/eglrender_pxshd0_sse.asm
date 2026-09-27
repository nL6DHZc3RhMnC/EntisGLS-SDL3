
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
mmxWord3XorFFMask	LABEL	MMWORD
		WORD	0FFH, 0FFH, 0FFH, 00H

ALIGN	10H
xmmScale16bit		REAL4	4 DUP( 65536.0 )


ConstSeg	ENDS


; ----------------------------------------------------------------------------
;	コードセグメント
; ----------------------------------------------------------------------------

CodeSeg	SEGMENT	PARA READONLY FLAT 'CODE'

;
;	テクスチャ（RGBA）シェーディング無し関数 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_LineNS_ATX_SSE	PROC	NEAR32 C

	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	mov	[ebx].dib.dwSaveRegEBP, ebp
	mov	edi, [ebx].pLineBuf[0]
	ASSUME	edi:PTR E3D_TRANS_LINE_BUF
	mov	ebp, [ebx].dib.ptrZBufLine
	;
	; パラメータ読み込み
	;
	mov	edx, [ebx].nLineLeft[4]
	mov	ecx, [ebx].nLineRight[4]
	mov	eax, edx
	and	ecx, NOT 01H
	and	eax, NOT 01H
	movlps	xmm0, [ebx].vTxLinePos
	movss	xmm1, [ebx].rTxLineMod
	movlps	xmm2, [ebx].vTxDeltaX
	movss	xmm3, [ebx].vTxxy.x
	;
	; ライン左端パラメータ補正
	;
	cvtsi2ss	xmm4, edx
	sub	ecx, eax
	lea	ebp, [ebp + edx * 4]
	shufps	xmm4, xmm4, 0
	movaps	xmm5, xmm4
	mulps	xmm4, xmm2
	mulss	xmm5, xmm3
	addps	xmm0, xmm4
	addss	xmm1, xmm5
	;
	; パラメータ・ペアリング
	;
	movaps	xmm4, xmm0
	movaps	xmm5, xmm1
	addps	xmm4, xmm2
	addss	xmm5, xmm3
	movlhps	xmm0, xmm4
	unpcklps	xmm1, xmm5
	addps	xmm2, xmm2
	addps	xmm3, xmm3
	movlhps	xmm2, xmm2
	unpcklps	xmm3, xmm3
	;
	xor	eax, eax
	test	[ebx].txtimg.fdwFormatType, EIF_WITH_ALPHA
	setnz	al
	neg	eax
	movd	mm1, eax
	punpckldq	mm1, mm1
	;
	shr	ecx, 1
	ASSERT	<!!CARRY?>, "!(ecx & 01H)"
	inc	ecx
	.REPEAT
		;
		; テクスチャ座標用パラメータを進める
		; また、第1ピクセルのシェーディングパラメータを準備する
		;
		prefetcht0	[ebp]
		rcpps	xmm4, xmm1		; xmm4 <- 1 / xmm1
			movaps	xmm6, xmm0
			movaps	xmm7, xmm1
			addps	xmm0, xmm2
		mulps	xmm7, xmm4
			addps	xmm1, xmm3
		rcpps	xmm7, xmm7
		mulps	xmm4, xmm7
		add	ebp, 8
		;
		; ｚ値とテクスチャマッピング用パラメータ計算のための準備を行う
		;
		movss	xmm5, [ebx].rTxoxyr
		movss	xmm7, [ebx].rRcpTxxy
		shufps	xmm5, xmm5, 0
		shufps	xmm7, xmm7, 0
		mulps	xmm5, xmm4		; xmm5 = z 値
		unpcklps	xmm4, xmm4
		mulps	xmm4, xmm7		; xmm4 = マッピング座標
		mulps	xmm4, xmm6
		;
		; テクスチャ画像を取得する
		;
		cvtps2pi	mm7, xmm4
		movhlps	xmm4, xmm4
			movq	mm5, MMWORD PTR [ebx].txSizeMask[8]
		cvtps2pi	mm6, xmm4
		packssdw	mm7, mm6
		movq	mm6, mm7
		paddusw	mm7, mm5
		psraw	mm6, 15
		psubusw	mm7, mm5
			movq	mm5, MMWORD PTR [ebx].txImageAddr
		pandn	mm6, mm7
		pmaddwd	mm6, MMWORD PTR [ebx].txMulAddr
			movlps	QWORD PTR [edi].rZValue, xmm5
		paddd	mm6, mm5
		;
		; 第1ピクセルを計算する
		;	mm4:mm5 <- 第1ピクセルの結果
		; 第2ピクセルを計算し、パラメータを進める
		;	mm2:mm3 <- 第2ピクセルの結果
		;
		movd	eax, mm6
		psrlq	mm6, 32
		pxor	mm7, mm7
			movd	edx, mm6
		pcmpeqd	mm0, mm0
		movd	mm4, DWORD PTR [eax]
			movd	mm2, DWORD PTR [edx]
		punpcklbw	mm4, mm7
		punpcklbw	mm2, mm7
			pshufw	mm5, mm4, 11111111B
			pshufw	mm3, mm2, 11111111B
		psrld	mm0, 8
			pxor	mm5, mmxWord3XorFFMask
			pxor	mm3, mmxWord3XorFFMask
		;
		; 結果を書き出す
		;
		packuswb	mm5, mm3
		packuswb	mm4, mm2
		pand	mm5, mm0
		pand	mm4, mm0
		pand	mm5, mm1
		movq	MMWORD PTR [edi].rgbAdd[0], mm4
		movq	MMWORD PTR [edi].rgbMul[0], mm5
		add	edi, (SIZEOF E3D_TRANS_LINE_BUF)
		;
		dec	ecx
	.UNTIL	ZERO?

	mov	ebp, [ebx].dib.dwSaveRegEBP
	ASSUME	edi:NOTHING
	ASSUME	ebx:NOTHING
	ret

eglRenderPoly@RenderPolygon_LineNS_ATX_SSE	ENDP

;
;	テクスチャ（タイリング無しRGBA 補完）シェーディング無し関数 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_LineNS_SAX_SSE	PROC	NEAR32 C

	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	mov	[ebx].dib.dwSaveRegEBP, ebp
	mov	edi, [ebx].pLineBuf[0]
	ASSUME	edi:PTR E3D_TRANS_LINE_BUF
	mov	ebp, [ebx].dib.ptrZBufLine
	;
	; パラメータ読み込み
	;
	mov	edx, [ebx].nLineLeft[4]
	mov	ecx, [ebx].nLineRight[4]
	mov	eax, edx
	and	ecx, NOT 01H
	and	eax, NOT 01H
	movlps	xmm0, [ebx].vTxLinePos
	movss	xmm1, [ebx].rTxLineMod
	movlps	xmm2, [ebx].vTxDeltaX
	movss	xmm3, [ebx].vTxxy.x
	;
	; ライン左端パラメータ補正
	;
	cvtsi2ss	xmm4, edx
	sub	ecx, eax
	lea	ebp, [ebp + edx * 4]
	shufps	xmm4, xmm4, 0
	movaps	xmm5, xmm4
	mulps	xmm4, xmm2
	mulss	xmm5, xmm3
	addps	xmm0, xmm4
	addss	xmm1, xmm5
	;
	; パラメータ・ペアリング
	;
	movaps	xmm4, xmm0
	movaps	xmm5, xmm1
	addps	xmm4, xmm2
	addss	xmm5, xmm3
	movlhps	xmm0, xmm4
	unpcklps	xmm1, xmm5
	addps	xmm2, xmm2
	addps	xmm3, xmm3
	movlhps	xmm2, xmm2
	unpcklps	xmm3, xmm3
	movaps	xmm7, xmmScale16bit
	mulps	xmm0, xmm7
	mulps	xmm2, xmm7
	;
	xor	eax, eax
	test	[ebx].txtimg.fdwFormatType, EIF_WITH_ALPHA
	setnz	al
	neg	eax
	movd	mm1, eax
	punpckldq	mm1, mm1
	movq	[ebx].rgbPixelAlphaMask, mm1
	;
	shr	ecx, 1
	ASSERT	<!!CARRY?>, "!(ecx & 01H)"
	inc	ecx
	.REPEAT
		;
		; テクスチャ座標用パラメータを進める
		; また、第1ピクセルのシェーディングパラメータを準備する
		;
		prefetcht0	[ebp]
		rcpps	xmm4, xmm1		; xmm4 <- 1 / xmm1
			movaps	xmm6, xmm0
			movaps	xmm7, xmm1
		mulps	xmm7, xmm4
			addps	xmm0, xmm2
		rcpps	xmm7, xmm7
			addps	xmm1, xmm3
		mulps	xmm4, xmm7
		add	ebp, 8
		;
		; ｚ値とテクスチャマッピング用パラメータ計算のための準備を行う
		;
		movss	xmm5, [ebx].rTxoxyr
		movss	xmm7, [ebx].rRcpTxxy
		shufps	xmm5, xmm5, 0
		shufps	xmm7, xmm7, 0
		mulps	xmm5, xmm4		; xmm5 = z 値
		unpcklps	xmm4, xmm4
		mulps	xmm4, xmm7		; xmm4 = マッピング座標
		mulps	xmm4, xmm6
		;
		; 第1ピクセルのマッピング座標を取得する
		;
		mov	[ebx].dib.nLeftWidth, ecx
		cvtps2pi	mm6, xmm4
		movhlps	xmm4, xmm4
			pshufw	mm2, mm6, 00000000B
		movq	mm4, mm6
			pshufw	mm3, mm6, 10101010B
				pcmpeqd	mm7, mm7
		psrld	mm4, 16
				psrld	mm7, 31
		movq	mm5, MMWORD PTR [ebx].txSizeMask[8]
				paddsw	mm7, mm4
		psrad	mm6, 31
		packssdw	mm4, mm7
		packssdw	mm6, mm6
		paddusw	mm4, mm5
		pxor	mm7, mm7
		psubusw	mm4, mm5
			psrlw	mm2, 1
		pandn	mm6, mm4
			movlps	QWORD PTR [edi].rZValue, xmm5
		pextrw	eax, mm6, 0
			psrlw	mm3, 1
		mov	esi, [ebx].pTxLineAddr
		pextrw	ecx, mm6, 1
		pextrw	edx, mm6, 2
		mov	esi, DWORD PTR [esi + ecx * 4]
		;
		; 第1ピクセルの値を補完する
		;
			pextrw	ecx, mm6, 3
		movd	mm4, DWORD PTR [esi + eax * 4]
		movd	mm5, DWORD PTR [esi + edx * 4]
			mov	esi, [ebx].pTxLineAddr
		punpcklbw	mm4, mm7
		punpcklbw	mm5, mm7
			mov	esi, DWORD PTR [esi + ecx * 4]
		psubsw	mm5, mm4
			movd	mm0, DWORD PTR [esi + eax * 4]
		psllw	mm5, 1
			movd	mm1, DWORD PTR [esi + edx * 4]
		pmulhw	mm5, mm2
			punpcklbw	mm0, mm7
			punpcklbw	mm1, mm7
			psubsw	mm1, mm0
		paddsw	mm5, mm4
			psllw	mm1, 1
				cvtps2pi	mm4, xmm4
			pmulhw	mm1, mm2
				pshufw	mm2, mm4, 00000000B
				pshufw	mm6, mm4, 10101010B
			paddsw	mm1, mm0
		;
			movq	mm7, mm4
				pcmpeqd	mm0, mm0
			psrld	mm7, 16
		psubsw	mm1, mm5
				psrld	mm0, 31
		psllw	mm1, 1
				paddsw	mm0, mm7
		pmulhw	mm1, mm3
			packssdw	mm7, mm0
			movq	mm0, MMWORD PTR [ebx].txSizeMask[8]
			psrad	mm4, 31
			paddusw	mm7, mm0
			packssdw	mm4, mm4
			psubusw	mm7, mm0
				psrlw	mm2, 1
			pandn	mm4, mm7
				psrlw	mm6, 1
			pextrw	eax, mm4, 0
			mov	esi, [ebx].pTxLineAddr
		paddsw	mm1, mm5
		;
		; 第2ピクセルの値を補完する
		;
		pextrw	ecx, mm4, 1
		pextrw	edx, mm4, 2
		mov	esi, DWORD PTR [esi + ecx * 4]
			movq	MMWORD PTR [edi].rgbAdd[0], mm1
		;
		pextrw	ecx, mm4, 3
		movd	mm4, DWORD PTR [esi + eax * 4]
		movd	mm5, DWORD PTR [esi + edx * 4]
		pxor	mm7, mm7
			mov	esi, [ebx].pTxLineAddr
		punpcklbw	mm4, mm7
		punpcklbw	mm5, mm7
			mov	esi, DWORD PTR [esi + ecx * 4]
		psubsw	mm5, mm4
			movd	mm0, DWORD PTR [esi + eax * 4]
		psllw	mm5, 1
			movd	mm3, DWORD PTR [esi + edx * 4]
		pmulhw	mm5, mm2
			punpcklbw	mm0, mm7
			punpcklbw	mm3, mm7
			psubsw	mm3, mm0
		paddsw	mm5, mm4
			psllw	mm3, 1
			pmulhw	mm3, mm2
				movq	mm2, MMWORD PTR [edi].rgbAdd[0]
			paddsw	mm3, mm0
		;
		; シェーディングを施す
		;
		psubsw	mm3, mm5
			pshufw	mm7, mm2, 11111111B
		psllw	mm3, 1
			pcmpeqd	mm0, mm0
		pmulhw	mm3, mm6
			pxor	mm7, mmxWord3XorFFMask
		paddsw	mm3, mm5
		pshufw	mm6, mm3, 11111111B
			psrld	mm0, 8
			pxor	mm6, mmxWord3XorFFMask
		;
		; 結果を書き出す
		;
		packuswb	mm2, mm3
		packuswb	mm7, mm6
		pand	mm2, mm0
		pand	mm7, mm0
		mov	ecx, [ebx].dib.nLeftWidth
		pand	mm7, [ebx].rgbPixelAlphaMask
		movq	MMWORD PTR [edi].rgbAdd[0], mm2
		movq	MMWORD PTR [edi].rgbMul[0], mm7
		add	edi, (SIZEOF E3D_TRANS_LINE_BUF)
		;
		dec	ecx
	.UNTIL	ZERO?

	mov	ebp, [ebx].dib.dwSaveRegEBP
	ASSUME	edi:NOTHING
	ASSUME	ebx:NOTHING
	ret

eglRenderPoly@RenderPolygon_LineNS_SAX_SSE	ENDP

;
;	テクスチャ（RGBA タイリング）シェーディング無し関数 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_LineNS_TATX_SSE	PROC	NEAR32 C

	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	mov	[ebx].dib.dwSaveRegEBP, ebp
	mov	edi, [ebx].pLineBuf[0]
	ASSUME	edi:PTR E3D_TRANS_LINE_BUF
	mov	ebp, [ebx].dib.ptrZBufLine
	;
	; パラメータ読み込み
	;
	mov	edx, [ebx].nLineLeft[4]
	mov	ecx, [ebx].nLineRight[4]
	mov	eax, edx
	and	ecx, NOT 01H
	and	eax, NOT 01H
	movlps	xmm0, [ebx].vTxLinePos
	movss	xmm1, [ebx].rTxLineMod
	movlps	xmm2, [ebx].vTxDeltaX
	movss	xmm3, [ebx].vTxxy.x
	;
	; ライン左端パラメータ補正
	;
	cvtsi2ss	xmm4, edx
	sub	ecx, eax
	lea	ebp, [ebp + edx * 4]
	shufps	xmm4, xmm4, 0
	movaps	xmm5, xmm4
	mulps	xmm4, xmm2
	mulss	xmm5, xmm3
	addps	xmm0, xmm4
	addss	xmm1, xmm5
	;
	; パラメータ・ペアリング
	;
	movaps	xmm4, xmm0
	movaps	xmm5, xmm1
	addps	xmm4, xmm2
	addss	xmm5, xmm3
	movlhps	xmm0, xmm4
	unpcklps	xmm1, xmm5
	addps	xmm2, xmm2
	addps	xmm3, xmm3
	movlhps	xmm2, xmm2
	unpcklps	xmm3, xmm3
	;
	xor	eax, eax
	test	[ebx].txtimg.fdwFormatType, EIF_WITH_ALPHA
	setnz	al
	neg	eax
	movd	mm1, eax
	punpckldq	mm1, mm1
	;
	shr	ecx, 1
	ASSERT	<!!CARRY?>, "!(ecx & 01H)"
	inc	ecx
	.REPEAT
		;
		; テクスチャ座標用パラメータを進める
		; また、第1ピクセルのシェーディングパラメータを準備する
		;
		prefetcht0	[ebp]
		rcpps	xmm4, xmm1		; xmm4 <- 1 / xmm1
			movaps	xmm6, xmm0
			movaps	xmm7, xmm1
			addps	xmm0, xmm2
		mulps	xmm7, xmm4
			addps	xmm1, xmm3
		rcpps	xmm7, xmm7
		mulps	xmm4, xmm7
		add	ebp, 8
		;
		; ｚ値とテクスチャマッピング用パラメータ計算のための準備を行う
		;
		movss	xmm5, [ebx].rTxoxyr
		movss	xmm7, [ebx].rRcpTxxy
		shufps	xmm5, xmm5, 0
		shufps	xmm7, xmm7, 0
		mulps	xmm5, xmm4		; xmm5 = z 値
		unpcklps	xmm4, xmm4
		mulps	xmm4, xmm7		; xmm4 = マッピング座標
		mulps	xmm4, xmm6
		;
		; テクスチャ画像を取得する
		;
		cvtps2pi	mm6, xmm4
		movhlps	xmm4, xmm4
			movq	mm5, MMWORD PTR [ebx].txSizeMask[0]
		cvtps2pi	mm7, xmm4
		packssdw	mm6, mm7
			movq	mm7, MMWORD PTR [ebx].txMulAddr
		pand	mm6, mm5
			movq	mm5, MMWORD PTR [ebx].txImageAddr
		pmaddwd	mm6, mm7
			movlps	QWORD PTR [edi].rZValue, xmm5
		paddd	mm6, mm5
		;
		; 第1ピクセルを計算する
		;	mm4:mm5 <- 第1ピクセルの結果
		; 第2ピクセルを計算し、パラメータを進める
		;	mm2:mm3 <- 第2ピクセルの結果
		;
		movd	eax, mm6
		psrlq	mm6, 32
		pxor	mm7, mm7
			movd	edx, mm6
		pcmpeqd	mm0, mm0
		movd	mm4, DWORD PTR [eax]
			movd	mm2, DWORD PTR [edx]
		punpcklbw	mm4, mm7
		punpcklbw	mm2, mm7
			pshufw	mm5, mm4, 11111111B
			pshufw	mm3, mm2, 11111111B
		psrld	mm0, 8
			pxor	mm5, mmxWord3XorFFMask
			pxor	mm3, mmxWord3XorFFMask
		;
		; 結果を書き出す
		;
		packuswb	mm5, mm3
		packuswb	mm4, mm2
		pand	mm5, mm0
		pand	mm4, mm0
		pand	mm5, mm1
		movq	MMWORD PTR [edi].rgbAdd[0], mm4
		movq	MMWORD PTR [edi].rgbMul[0], mm5
		add	edi, (SIZEOF E3D_TRANS_LINE_BUF)
		;
		dec	ecx
	.UNTIL	ZERO?

	mov	ebp, [ebx].dib.dwSaveRegEBP
	ASSUME	edi:NOTHING
	ASSUME	ebx:NOTHING
	ret

eglRenderPoly@RenderPolygon_LineNS_TATX_SSE	ENDP

;
;	テクスチャ（RGBA 補完 タイリング）シェーディング無し関数 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_LineNS_STAX_SSE	PROC	NEAR32 C

	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	mov	[ebx].dib.dwSaveRegEBP, ebp
	mov	edi, [ebx].pLineBuf[0]
	ASSUME	edi:PTR E3D_TRANS_LINE_BUF
	mov	ebp, [ebx].dib.ptrZBufLine
	;
	; パラメータ読み込み
	;
	mov	edx, [ebx].nLineLeft[4]
	mov	ecx, [ebx].nLineRight[4]
	mov	eax, edx
	and	ecx, NOT 01H
	and	eax, NOT 01H
	movlps	xmm0, [ebx].vTxLinePos
	movss	xmm1, [ebx].rTxLineMod
	movlps	xmm2, [ebx].vTxDeltaX
	movss	xmm3, [ebx].vTxxy.x
	;
	; ライン左端パラメータ補正
	;
	cvtsi2ss	xmm4, edx
	sub	ecx, eax
	lea	ebp, [ebp + edx * 4]
	shufps	xmm4, xmm4, 0
	movaps	xmm5, xmm4
	mulps	xmm4, xmm2
	mulss	xmm5, xmm3
	addps	xmm0, xmm4
	addss	xmm1, xmm5
	;
	; パラメータ・ペアリング
	;
	movaps	xmm4, xmm0
	movaps	xmm5, xmm1
	addps	xmm4, xmm2
	addss	xmm5, xmm3
	movlhps	xmm0, xmm4
	unpcklps	xmm1, xmm5
	addps	xmm2, xmm2
	addps	xmm3, xmm3
	movlhps	xmm2, xmm2
	unpcklps	xmm3, xmm3
	movaps	xmm7, xmmScale16bit
	mulps	xmm0, xmm7
	mulps	xmm2, xmm7
	;
	xor	eax, eax
	test	[ebx].txtimg.fdwFormatType, EIF_WITH_ALPHA
	setnz	al
	neg	eax
	movd	mm1, eax
	punpckldq	mm1, mm1
	movq	[ebx].rgbPixelAlphaMask, mm1
	;
	shr	ecx, 1
	ASSERT	<!!CARRY?>, "!(ecx & 01H)"
	inc	ecx
	.REPEAT
		;
		; テクスチャ座標用パラメータを進める
		; また、第1ピクセルのシェーディングパラメータを準備する
		;
		prefetcht0	[ebp]
		rcpps	xmm4, xmm1		; xmm4 <- 1 / xmm1
			movaps	xmm6, xmm0
			movaps	xmm7, xmm1
		mulps	xmm7, xmm4
			addps	xmm0, xmm2
		rcpps	xmm7, xmm7
			addps	xmm1, xmm3
		mulps	xmm4, xmm7
		add	ebp, 8
		;
		; ｚ値とテクスチャマッピング用パラメータ計算のための準備を行う
		;
		movss	xmm5, [ebx].rTxoxyr
		movss	xmm7, [ebx].rRcpTxxy
		shufps	xmm5, xmm5, 0
		shufps	xmm7, xmm7, 0
		mulps	xmm5, xmm4		; xmm5 = z 値
		unpcklps	xmm4, xmm4
		mulps	xmm4, xmm7		; xmm4 = マッピング座標
		mulps	xmm4, xmm6
		;
		; 第1ピクセルのマッピング座標を取得する
		;
		mov	[ebx].dib.nLeftWidth, ecx
		cvtps2pi	mm6, xmm4
		movhlps	xmm4, xmm4
			pshufw	mm2, mm6, 00000000B
			pshufw	mm3, mm6, 10101010B
		psrld	mm6, 16
			psrlw	mm2, 1
			movlps	QWORD PTR [edi].rZValue, xmm5
		movd	eax, mm6
		psrlq	mm6, 32
			psrlw	mm3, 1
		mov	esi, [ebx].pTxLineAddr
		movd	ecx, mm6
		lea	edx, [eax + 1]
		and	cx, [ebx].txSizeMask[2]
		and	eax, DWORD PTR [ebx].txSizeMask[0]
			movzx	ecx, cx
		and	edx, DWORD PTR [ebx].txSizeMask[0]
			and	eax, 0FFFFH
			and	edx, 0FFFFH
		mov	esi, DWORD PTR [esi + ecx * 4]
		;
		; 第1ピクセルの値を補完する
		;
			inc	ecx
		pxor	mm7, mm7
		movd	mm4, DWORD PTR [esi + eax * 4]
			and	cx, [ebx].txSizeMask[2]
		movd	mm5, DWORD PTR [esi + edx * 4]
			mov	esi, [ebx].pTxLineAddr
			movzx	ecx, cx
		punpcklbw	mm4, mm7
		punpcklbw	mm5, mm7
			mov	esi, DWORD PTR [esi + ecx * 4]
		psubsw	mm5, mm4
			movd	mm0, DWORD PTR [esi + eax * 4]
		psllw	mm5, 1
			movd	mm1, DWORD PTR [esi + edx * 4]
		pmulhw	mm5, mm2
			punpcklbw	mm0, mm7
			punpcklbw	mm1, mm7
			psubsw	mm1, mm0
		paddsw	mm5, mm4
			psllw	mm1, 1
				cvtps2pi	mm4, xmm4
			pmulhw	mm1, mm2
				pshufw	mm2, mm4, 00000000B
				pshufw	mm6, mm4, 10101010B
			paddsw	mm1, mm0
		;
			psrld	mm4, 16
		psubsw	mm1, mm5
			movd	eax, mm4
		psllw	mm1, 1
			psrlq	mm4, 32
		pmulhw	mm1, mm3
			mov	esi, [ebx].pTxLineAddr
				psrlw	mm2, 1
				psrlw	mm6, 1
		paddsw	mm1, mm5
		;
		; 第2ピクセルの値を補完する
		;
		movd	ecx, mm4
		lea	edx, [eax + 1]
		and	cx, [ebx].txSizeMask[2]
		and	eax, DWORD PTR [ebx].txSizeMask[0]
			movzx	ecx, cx
		and	edx, DWORD PTR [ebx].txSizeMask[0]
			and	eax, 0FFFFH
			and	edx, 0FFFFH
		mov	esi, DWORD PTR [esi + ecx * 4]
			movq	MMWORD PTR [edi].rgbAdd[0], mm1
		;
		inc	ecx
		movd	mm4, DWORD PTR [esi + eax * 4]
			and	cx, [ebx].txSizeMask[2]
		movd	mm5, DWORD PTR [esi + edx * 4]
			mov	esi, [ebx].pTxLineAddr
			movzx	ecx, cx
		punpcklbw	mm4, mm7
		punpcklbw	mm5, mm7
			mov	esi, DWORD PTR [esi + ecx * 4]
		psubsw	mm5, mm4
			movd	mm0, DWORD PTR [esi + eax * 4]
		psllw	mm5, 1
			movd	mm3, DWORD PTR [esi + edx * 4]
		pmulhw	mm5, mm2
			punpcklbw	mm0, mm7
			punpcklbw	mm3, mm7
			psubsw	mm3, mm0
		paddsw	mm5, mm4
			psllw	mm3, 1
			pmulhw	mm3, mm2
				movq	mm2, MMWORD PTR [edi].rgbAdd[0]
;				movq	mm1, MMWORD PTR [ebx].rgbNextColor[8]
			paddsw	mm3, mm0
;				movq	mm0, MMWORD PTR [ebx].rgbNextColor[0]
		;
		; シェーディングを施す
		;
		psubsw	mm3, mm5
			pshufw	mm7, mm2, 11111111B
		psllw	mm3, 1
			pcmpeqd	mm0, mm0
		pmulhw	mm3, mm6
			pxor	mm7, mmxWord3XorFFMask
		paddsw	mm3, mm5
		pshufw	mm6, mm3, 11111111B
			psrld	mm0, 8
			pxor	mm6, mmxWord3XorFFMask
		;
		; 結果を書き出す
		;
		packuswb	mm2, mm3
		packuswb	mm7, mm6
		pand	mm2, mm0
		pand	mm7, mm0
		mov	ecx, [ebx].dib.nLeftWidth
		pand	mm7, [ebx].rgbPixelAlphaMask
		movq	MMWORD PTR [edi].rgbAdd[0], mm2
		movq	MMWORD PTR [edi].rgbMul[0], mm7
		add	edi, (SIZEOF E3D_TRANS_LINE_BUF)
		;
		dec	ecx
	.UNTIL	ZERO?

	mov	ebp, [ebx].dib.dwSaveRegEBP
	ASSUME	edi:NOTHING
	ASSUME	ebx:NOTHING
	ret

eglRenderPoly@RenderPolygon_LineNS_STAX_SSE	ENDP


CodeSeg	ENDS

	END
