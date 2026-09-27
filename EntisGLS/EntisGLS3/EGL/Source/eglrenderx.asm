
; ----------------------------------------------------------------------------
;                          Entis Graphic Library
; ----------------------------------------------------------------------------
;    Copyright (c) 2002-2010 Leshade Entis, Entis-soft. Al rights reserved.
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
mmxAlphaMask	LABEL	MMWORD
		WORD	00H, 00H, 00H, 0FFH
mmxWord3XorFFMask	LABEL	MMWORD
		WORD	0FFH, 0FFH, 0FFH, 00H
mmxColorMask	LABEL	MMWORD
		BYTE	0FFH, 0FFH, 0FFH, 0, 0FFH, 0FFH, 0FFH, 0
mmxTrimMask	LABEL	MMWORD
		BYTE	80H, 80H, 80H, 0, 80H, 80H, 80H, 0
mmxBiasZ	LABEL	MMWORD
		DWORD	80H, 80H
mmxWhiteColorWords	LABEL	MMWORD
		WORD	7FFFH, 7FFFH, 7FFFH, 0
mmxConst2000x3_0	LABEL	MMWORD
		WORD	3 DUP( 2000H ), 0

ALIGN	10H
xmmScale16bit		REAL4	4 DUP( 65536.0 )
xmmConst1		REAL4	4 DUP( 1.0 )
xmmConstHalf		REAL4	4 DUP( 0.5 )
xmmConstDiv10000H	REAL4	4 DUP( 0.0000152587890625 )	; 1 / 65536
xmmBiasScaleZ		REAL4	4 DUP( 3.5 )
xmmBiasZ		REAL4	4 DUP( 1.00125 )
xmmBiasDeltaZ		REAL4	4 DUP( 0.000244140625 )
xmmMaskSignFlag		DWORD	4 DUP( 7FFFFFFFH )
xmmSignFlag		DWORD	4 DUP( 80000000H )
xmmMask1_0_1_0		DWORD	2 DUP( 0FFFFFFFFH, 0 )
xmmConst1_0_1_0		REAL4	2 DUP( 1.0, 0.0 )

xmm2ConstHalf		REAL8	2 DUP( 0.5 )

ConstSeg	ENDS


; ----------------------------------------------------------------------------
;	コードセグメント
; ----------------------------------------------------------------------------

CodeSeg	SEGMENT	PARA READONLY FLAT 'CODE'

;
;	z 座標計算関数 SSE2 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_LineZ_SSE2	PROC	NEAR32 C

	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	mov	edi, [ebx].pLineBuf[0]
	ASSUME	edi:PTR E3D_TRANS_LINE_BUF
	;
	mov	eax, [ebx].nLineLeft[4]
	mov	ecx, [ebx].nLineRight[4]
	mov	edx, eax
	and	ecx, NOT 01H
	and	edx, NOT 01H
	_movsd	xmm0, [ebx].vTxxy_d.x
	sub	ecx, edx
	lea	esi, [esi + eax * 4]
	_movsd	xmm1, [ebx].rTxLineMod_d
	shr	ecx, 1
	_movsd	xmm7, [ebx].rTxoxyr_d
	ASSERT	<!!CARRY?>, "!(ecx & 01H)"
	inc	ecx
	;
	cvtsi2sd	xmm2, [ebx].nLineLeft[4]
;	addsd	xmm2, xmm2ConstHalf	; xmm2 = left + 0.5
	movapd	xmm3, xmm1		; xmm3 = rTxLineMod_d
	mulsd	xmm2, xmm0		; xmm2 = (left + 0.5) * vTxxy_d.x
	addsd	xmm3, xmm0		; xmm3 = rTxLineMod_d + vTxxy_d.x
	addsd	xmm0, xmm0		; xmm0 = vTxxy_d.x * 2
	shufpd	xmm7, xmm7, 0		; xmm7 = { rTxoxyr_d, rTxoxyr_d }
	addsd	xmm1, xmm2		; xmm1 = rTxLineMod_d + (left + 0.5) * vTxxy_d.x
	addsd	xmm3, xmm2		; xmm3 = rTxLineMod_d + (left + 1.5) * vTxxy_d.x
	shufpd	xmm0, xmm0, 0		; xmm0 = { vTxxy_d.x * 2, vTxxy_d.x * 2 }
	unpcklpd	xmm1, xmm3	; xmm1 = rTxLineMod_d + vTxxy_d.x * { (left + 0.5), (left + 1.5) }
	;
	.REPEAT
		movapd	xmm2, xmm7	; xmm2 = xmm7 / xmm1
		divpd	xmm2, xmm1
		addpd	xmm1, xmm0	; xmm1 = xmm1 + xmm0
		cvtpd2ps	xmm3, xmm2
		movlps	QWORD PTR [edi].rZValue[0], xmm3
		add	edi, (SIZEOF E3D_TRANS_LINE_BUF)
		dec	ecx
	.UNTIL	ZERO?
	;
	ASSUME	edi:NOTHING
	ASSUME	ebx:NOTHING
Label_Exit:
	ret

eglRenderPoly@RenderPolygon_LineZ_SSE2	ENDP

;
;	テクスチャなしシェーディング関数 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_LineNTXSSE	PROC	NEAR32 C

	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	mov	edi, [ebx].pLineBuf[0]
	ASSUME	edi:PTR E3D_TRANS_LINE_BUF
	mov	esi, [ebx].dib.ptrZBufLine
	;
		mov	eax, [ebx].nLineLeft[0]
	movq	mm0, MMWORD PTR [ebx].rgbNextColor[0]
		and	eax, 01H
	movq	mm1, MMWORD PTR [ebx].rgbNextColor[8]
		neg	eax
	movq	mm2, MMWORD PTR [ebx].rgbDeltaColor[0]
		movd	mm6, eax
	movq	mm3, MMWORD PTR [ebx].rgbDeltaColor[8]
		punpckldq	mm6, mm6
		movq	mm7, mm6
		pand	mm6, mm2
		pand	mm7, mm3
		psubsw	mm0, mm6
		psubsw	mm1, mm7
	;
	mov	eax, [ebx].nLineLeft[4]
	mov	ecx, [ebx].nLineRight[4]
	mov	edx, eax
	and	ecx, NOT 01H
	and	edx, NOT 01H
	movss	xmm0, [ebx].vTxxy.x
	sub	ecx, edx
	lea	esi, [esi + eax * 4]
	movss	xmm1, [ebx].rTxLineMod
	shr	ecx, 1
	movss	xmm7, [ebx].rTxoxyr
	ASSERT	<!!CARRY?>, "!(ecx & 01H)"
	inc	ecx
	;
	cvtsi2ss	xmm2, [ebx].nLineLeft[4]
;	addss	xmm2, xmmConstHalf
	movaps	xmm3, xmm1
	mulss	xmm2, xmm0
	addss	xmm3, xmm0
	addss	xmm0, xmm0
	shufps	xmm7, xmm7, 0
	addss	xmm1, xmm2
	addss	xmm3, xmm2
	shufps	xmm0, xmm0, 0
	unpcklps	xmm1, xmm3
	;
	.REPEAT
		prefetchnta	[esi]
		rcpps	xmm2, xmm1		; xmm2 <- xmm7 / xmm1	
		movaps	xmm3, xmm1
		addps	xmm1, xmm0		; xmm1 <- xmm1 + xmm0
		mulps	xmm3, xmm2
			movq	mm4, mm0
			paddsw	mm0, mm2
			movq	mm5, mm1
			movq	mm7, mmxConst2000x3_0
			paddsw	mm1, mm3
			paddsw	mm4, mm7
			paddsw	mm5, mm7
			psraw	mm4, 7
			psraw	mm5, 7
		rcpps	xmm3, xmm3
			movq	mm6, mm0
			paddsw	mm0, mm2
			paddsw	mm6, mm7
;			movq	mm7, mm1
			paddsw	mm7, mm1
			paddsw	mm1, mm3
		mulps	xmm2, xmm3
			psraw	mm6, 7
			psraw	mm7, 7
			packuswb	mm4, mm6
			packuswb	mm5, mm7
			movq	MMWORD PTR [edi].rgbMul, mm4
		mulps	xmm2, xmm7
		add	esi, 8
			movq	MMWORD PTR [edi].rgbAdd, mm5
		movlps	QWORD PTR [edi].rZValue[0], xmm2
		add	edi, (SIZEOF E3D_TRANS_LINE_BUF)
		dec	ecx
	.UNTIL	ZERO?
	;
	ASSUME	edi:NOTHING
	ASSUME	ebx:NOTHING
Label_Exit:
	ret

eglRenderPoly@RenderPolygon_LineNTXSSE	ENDP

;
;	テクスチャ（RGB）シェーディング関数 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_LineTXSSE	PROC	NEAR32 C

	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	mov	[ebx].dib.dwSaveRegEBP, ebp
	mov	edi, [ebx].pLineBuf[0]
	ASSUME	edi:PTR E3D_TRANS_LINE_BUF
	mov	ebp, [ebx].dib.ptrZBufLine
	;
	; パラメータ読み込み
	;
		mov	eax, [ebx].nLineLeft[0]
	movq	mm0, MMWORD PTR [ebx].rgbNextColor[0]
		and	eax, 01H
	movq	mm1, MMWORD PTR [ebx].rgbNextColor[8]
		neg	eax
	movq	mm2, MMWORD PTR [ebx].rgbDeltaColor[0]
		movd	mm6, eax
	movq	mm3, MMWORD PTR [ebx].rgbDeltaColor[8]
		punpckldq	mm6, mm6
		movq	mm7, mm6
		pand	mm6, mm2
		pand	mm7, mm3
		psubsw	mm0, mm6
		psubsw	mm1, mm7
		paddsw	mm0, mmxConst2000x3_0
		paddsw	mm1, mmxConst2000x3_0
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
	addss	xmm3, xmm3
	movlhps	xmm2, xmm2
	unpcklps	xmm3, xmm3
	;
	shr	ecx, 1
	ASSERT	<!!CARRY?>, "!(ecx & 01H)"
	inc	ecx
	;
	; テクスチャ座標用パラメータを進める
	; また、第1ピクセルのシェーディングパラメータを準備する
	;
	rcpps	xmm4, xmm1		; xmm4 <- 1 / xmm1
		movaps	xmm6, xmm0
		movaps	xmm7, xmm1
		addps	xmm0, xmm2
	mulps	xmm7, xmm4
		addps	xmm1, xmm3
	rcpps	xmm7, xmm7
	mulps	xmm4, xmm7
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
	.REPEAT
		;
		; テクスチャ画像を取得する
		;
		; テクスチャ座標用パラメータを進める
		; また、第1ピクセルのシェーディングパラメータを準備する
		;
		prefetchnta	[ebp]
			movq	mm4, mm0
			movq	mm5, mm1
		cvtps2pi	mm7, xmm4
		movhlps	xmm4, xmm4
			psraw	mm5, 7
			paddsw	mm0, mm2
			paddsw	mm1, mm3
			add	ebp, 8
			movq	MMWORD PTR [ebx].rgbNextColor[8], mm5
		cvtps2pi	mm6, xmm4
			movq	mm5, MMWORD PTR [ebx].txSizeMask[8]
				rcpps	xmm4, xmm1		; xmm4 <- 1 / xmm1
				movaps	xmm6, xmm0
				movaps	xmm7, xmm1
				addps	xmm0, xmm2
		packssdw	mm7, mm6
				mulps	xmm7, xmm4
		movq	mm6, mm7
		paddusw	mm7, mm5
		psraw	mm6, 15
				addps	xmm1, xmm3
		psubusw	mm7, mm5
			movq	mm5, MMWORD PTR [ebx].txImageAddr
		pandn	mm6, mm7
				rcpps	xmm7, xmm7
		pmaddwd	mm6, MMWORD PTR [ebx].txMulAddr
			movlps	QWORD PTR [edi].rZValue, xmm5
				mulps	xmm4, xmm7
		paddd	mm6, mm5
		;
		; 第1ピクセルを計算する
		; 第2ピクセルのための準備を行う
		;	mm5 <- 第1ピクセルの結果
		;
		; ｚ値とテクスチャマッピング用パラメータ計算のための準備を行う
		;
				movss	xmm5, [ebx].rTxoxyr
				movss	xmm7, [ebx].rRcpTxxy
		movd	eax, mm6
		psrlq	mm6, 32
				shufps	xmm5, xmm5, 0
		pxor	mm7, mm7
		movd	mm5, DWORD PTR [eax]
				shufps	xmm7, xmm7, 0
			movd	edx, mm6
		punpcklbw	mm5, mm7
				mulps	xmm5, xmm4		; xmm5 = z 値
		psllw	mm5, 1
			movq	mm6, MMWORD PTR [ebx].rgbNextColor[8]
		pmulhw	mm5, mm4
				unpcklps	xmm4, xmm4
			movd	mm4, DWORD PTR [edx]
		paddsw	mm5, mm6
		;
		; 第2ピクセルを計算し、パラメータを進める
		;	mm4 <- 第2ピクセルの結果
		;
				mulps	xmm4, xmm7		; xmm4 = マッピング座標
			movq	mm6, mm1
		punpcklbw	mm4, mm7
			psraw	mm6, 7
		psllw	mm4, 1
			paddsw	mm1, mm3
		pmulhw	mm4, mm0
			paddsw	mm0, mm2
				mulps	xmm4, xmm6
		paddsw	mm4, mm6
		;
		; 結果を書き出す
		;
		packuswb	mm5, mm4
		movq	MMWORD PTR [edi].rgbMul, mm7
		movq	MMWORD PTR [edi].rgbAdd, mm5
		add	edi, (SIZEOF E3D_TRANS_LINE_BUF)
		;
		dec	ecx
	.UNTIL	ZERO?

	mov	ebp, [ebx].dib.dwSaveRegEBP
	ASSUME	edi:NOTHING
	ASSUME	ebx:NOTHING
	ret

eglRenderPoly@RenderPolygon_LineTXSSE	ENDP

;
;	テクスチャ（RGBA）シェーディング関数 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_LineATXSSE	PROC	NEAR32 C

	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	mov	[ebx].dib.dwSaveRegEBP, ebp
	mov	edi, [ebx].pLineBuf[0]
	ASSUME	edi:PTR E3D_TRANS_LINE_BUF
	mov	ebp, [ebx].dib.ptrZBufLine
	;
	; パラメータ読み込み
	;
		mov	eax, [ebx].nLineLeft[0]
	movq	mm0, MMWORD PTR [ebx].rgbNextColor[0]
		and	eax, 01H
	movq	mm1, MMWORD PTR [ebx].rgbNextColor[8]
		neg	eax
	movq	mm2, MMWORD PTR [ebx].rgbDeltaColor[0]
		movd	mm6, eax
	movq	mm3, MMWORD PTR [ebx].rgbDeltaColor[8]
		punpckldq	mm6, mm6
		movq	mm7, mm6
		pand	mm6, mm2
		pand	mm7, mm3
		psubsw	mm0, mm6
		psubsw	mm1, mm7
		paddsw	mm0, mmxConst2000x3_0
		paddsw	mm1, mmxConst2000x3_0
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
			movq	mm2, mm0
			movq	mm3, mm1
		rcpps	xmm7, xmm7
			paddsw	mm0, MMWORD PTR [ebx].rgbDeltaColor[0]
			paddsw	mm1, MMWORD PTR [ebx].rgbDeltaColor[8]
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
;		movss	xmm7, [ebx].vScreenPos.z
;		shufps	xmm7, xmm7, 0
		mulps	xmm4, xmm6
;		mulps	xmm5, xmm7
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
		; 第2ピクセルのための準備を行う
		;	mm4:mm5 <- 第1ピクセルの結果
		;
		movd	eax, mm6
		psrlq	mm6, 32
		pxor	mm7, mm7
		movd	mm4, DWORD PTR [eax]
			movd	edx, mm6
		punpcklbw	mm4, mm7
		psllw	mm4, 1
			pshufw	mm5, mm4, 11111111B
		pmulhw	mm4, mm2
			movd	mm2, DWORD PTR [edx]
			pmulhw	mm3, mm5
			psrlw	mm5, 1
		paddsw	mm4, mm3
			pxor	mm5, mmxWord3XorFFMask
		;
		; 第2ピクセルを計算し、パラメータを進める
		;	mm2:mm3 <- 第2ピクセルの結果
		;
		punpcklbw	mm2, mm7
			movq	mm6, mm1
		psllw	mm2, 1
			pshufw	mm3, mm2, 11111111B
		pmulhw	mm2, mm0
			pmulhw	mm6, mm3
			paddsw	mm0, MMWORD PTR [ebx].rgbDeltaColor[0]
			psrlw	mm3, 1
			pxor	mm3, mmxWord3XorFFMask
		paddsw	mm2, mm6
			paddsw	mm1, MMWORD PTR [ebx].rgbDeltaColor[8]
		;
		; 結果を書き出す
		;
		packuswb	mm4, mm2
		packuswb	mm5, mm3
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

eglRenderPoly@RenderPolygon_LineATXSSE	ENDP

;
;	テクスチャ（タイリング無し RGB 補完）シェーディング関数 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_LineSXSSE	PROC	NEAR32 C

	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	mov	[ebx].dib.dwSaveRegEBP, ebp
	mov	edi, [ebx].pLineBuf[0]
	ASSUME	edi:PTR E3D_TRANS_LINE_BUF
	mov	ebp, [ebx].dib.ptrZBufLine
	;
	; パラメータ読み込み
	;
		mov	eax, [ebx].nLineLeft[0]
	movq	mm0, MMWORD PTR [ebx].rgbNextColor[0]
		and	eax, 01H
	movq	mm1, MMWORD PTR [ebx].rgbNextColor[8]
		neg	eax
	movq	mm2, MMWORD PTR [ebx].rgbDeltaColor[0]
		movd	mm6, eax
	movq	mm3, MMWORD PTR [ebx].rgbDeltaColor[8]
		punpckldq	mm6, mm6
		movq	mm7, mm6
		pand	mm6, mm2
		pand	mm7, mm3
		psubsw	mm0, mm6
		psubsw	mm1, mm7
		paddsw	mm0, mmxConst2000x3_0
		paddsw	mm1, mmxConst2000x3_0
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
	shr	ecx, 1
	ASSERT	<!!CARRY?>, "!(ecx & 01H)"
	inc	ecx
	mov	[ebx].dib.nLeftWidth, ecx
		;
		; テクスチャ座標用パラメータを進める
		; また、第1ピクセルのシェーディングパラメータを準備する
		;
		rcpps	xmm4, xmm1		; xmm4 <- 1 / xmm1
			movaps	xmm6, xmm0
			movaps	xmm7, xmm1
		mulps	xmm7, xmm4
			addps	xmm0, xmm2
			movq	MMWORD PTR [ebx].rgbNextColor[0], mm0
		rcpps	xmm7, xmm7
			addps	xmm1, xmm3
			movq	MMWORD PTR [ebx].rgbNextColor[8], mm1
		mulps	xmm4, xmm7
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
	mov	ecx, [ebx].dib.nLeftWidth
	.REPEAT
		mov	[ebx].dib.nLeftWidth, ecx
		;
		; 第1ピクセルのマッピング座標を取得する
		;
		cvtps2pi	mm6, xmm4
		movhlps	xmm4, xmm4
			pshufw	mm2, mm6, 00000000B
		movq	mm4, mm6
			pshufw	mm3, mm6, 10101010B
				pcmpeqd	mm7, mm7
		psrad	mm4, 16
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
					;
					; テクスチャ座標用パラメータを進める
					; また、第1ピクセルのシェーディングパラメータを準備する
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
					prefetchnta	[ebp]
			punpcklbw	mm0, mm7
			punpcklbw	mm1, mm7
			psubsw	mm1, mm0
		paddsw	mm5, mm4
			psllw	mm1, 1
				cvtps2pi	mm4, xmm4
			pmulhw	mm1, mm2
					rcpps	xmm4, xmm1		; xmm4 <- 1 / xmm1
						movaps	xmm6, xmm0
				pshufw	mm2, mm4, 00000000B
				pshufw	mm6, mm4, 10101010B
						movaps	xmm7, xmm1
			paddsw	mm1, mm0
		;
			movq	mm7, mm4
					mulps	xmm7, xmm4
						addps	xmm0, xmm2
				pcmpeqd	mm0, mm0
			psrld	mm7, 16
		psubsw	mm1, mm5
				psrld	mm0, 31
		psllw	mm1, 1
				paddsw	mm0, mm7
		pmulhw	mm1, mm3
					rcpps	xmm7, xmm7
						addps	xmm1, xmm3
			packssdw	mm7, mm0
			movq	mm0, MMWORD PTR [ebx].txSizeMask[8]
			psrad	mm4, 31
			paddusw	mm7, mm0
			packssdw	mm4, mm4
					mulps	xmm4, xmm7
					add	ebp, 8
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
					;
					; ｚ値とテクスチャマッピング用パラメータ計算のための準備を行う
					;
		pextrw	ecx, mm4, 1
					movss	xmm5, [ebx].rTxoxyr
					movss	xmm7, [ebx].rRcpTxxy
		pextrw	edx, mm4, 2
					shufps	xmm5, xmm5, 0
		mov	esi, DWORD PTR [esi + ecx * 4]
			movq	MMWORD PTR [edi].rgbAdd[0], mm1
					shufps	xmm7, xmm7, 0
					mulps	xmm5, xmm4		; xmm5 = z 値
		;
		pextrw	ecx, mm4, 3
					unpcklps	xmm4, xmm4
		movd	mm4, DWORD PTR [esi + eax * 4]
		movd	mm5, DWORD PTR [esi + edx * 4]
		pxor	mm7, mm7
			mov	esi, [ebx].pTxLineAddr
					mulps	xmm4, xmm7		; xmm4 = マッピング座標
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
					mulps	xmm4, xmm6
			pmulhw	mm3, mm2
				movq	mm2, MMWORD PTR [edi].rgbAdd[0]
				movq	mm1, MMWORD PTR [ebx].rgbNextColor[8]
			paddsw	mm3, mm0
				movq	mm0, MMWORD PTR [ebx].rgbNextColor[0]
		;
		; シェーディングを施す
		;
		psubsw	mm3, mm5
			psllw	mm2, 1
		psllw	mm3, 1
			pmulhw	mm2, mm0	; 第1ピクセル
		pmulhw	mm3, mm6
			movq	mm4, mm1
			paddsw	mm0, MMWORD PTR [ebx].rgbDeltaColor[0]
			psraw	mm4, 7
		paddsw	mm3, mm5
			paddsw	mm1, MMWORD PTR [ebx].rgbDeltaColor[8]
		psllw	mm3, 1
			paddsw	mm2, mm4
		;
		pmulhw	mm3, mm0		; 第2ピクセル
			movq	mm5, mm1
			paddsw	mm0, MMWORD PTR [ebx].rgbDeltaColor[0]
			psraw	mm5, 7
		paddsw	mm3, mm5
			paddsw	mm1, MMWORD PTR [ebx].rgbDeltaColor[8]
		;
		; 結果を書き出す
		;
		packuswb	mm2, mm3
		mov	ecx, [ebx].dib.nLeftWidth
		movq	MMWORD PTR [ebx].rgbNextColor[0], mm0
		movq	MMWORD PTR [edi].rgbMul[0], mm7
		movq	MMWORD PTR [edi].rgbAdd[0], mm2
		add	edi, (SIZEOF E3D_TRANS_LINE_BUF)
		movq	MMWORD PTR [ebx].rgbNextColor[8], mm1
		;
		dec	ecx
	.UNTIL	ZERO?

	mov	ebp, [ebx].dib.dwSaveRegEBP
	ASSUME	edi:NOTHING
	ASSUME	ebx:NOTHING
	ret

eglRenderPoly@RenderPolygon_LineSXSSE	ENDP

;
;	テクスチャ（タイリング無しRGBA 補完）シェーディング関数 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_LineSAXSSE	PROC	NEAR32 C

	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	mov	[ebx].dib.dwSaveRegEBP, ebp
	mov	edi, [ebx].pLineBuf[0]
	ASSUME	edi:PTR E3D_TRANS_LINE_BUF
	mov	ebp, [ebx].dib.ptrZBufLine
	;
	; パラメータ読み込み
	;
		mov	eax, [ebx].nLineLeft[0]
	movq	mm0, MMWORD PTR [ebx].rgbNextColor[0]
		and	eax, 01H
	movq	mm1, MMWORD PTR [ebx].rgbNextColor[8]
		neg	eax
	movq	mm2, MMWORD PTR [ebx].rgbDeltaColor[0]
		movd	mm6, eax
	movq	mm3, MMWORD PTR [ebx].rgbDeltaColor[8]
		punpckldq	mm6, mm6
		movq	mm7, mm6
		pand	mm6, mm2
		pand	mm7, mm3
		psubsw	mm0, mm6
		psubsw	mm1, mm7
		paddsw	mm0, mmxConst2000x3_0
		paddsw	mm1, mmxConst2000x3_0
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
			movq	MMWORD PTR [ebx].rgbNextColor[0], mm0
		rcpps	xmm7, xmm7
			addps	xmm1, xmm3
			movq	MMWORD PTR [ebx].rgbNextColor[8], mm1
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
				movq	mm1, MMWORD PTR [ebx].rgbNextColor[8]
			paddsw	mm3, mm0
				movq	mm0, MMWORD PTR [ebx].rgbNextColor[0]
		;
		; シェーディングを施す
		;
			psllw	mm2, 1
		psubsw	mm3, mm5
			pshufw	mm7, mm2, 11111111B
		psllw	mm3, 1
			pmulhw	mm2, mm0	; 第1ピクセル
		pmulhw	mm3, mm6
			movq	mm4, mm1
			paddsw	mm0, MMWORD PTR [ebx].rgbDeltaColor[0]
			pmulhw	mm4, mm7
			psrlw	mm7, 1
		paddsw	mm3, mm5
			pxor	mm7, mmxWord3XorFFMask
			paddsw	mm1, MMWORD PTR [ebx].rgbDeltaColor[8]
		psllw	mm3, 1
			paddsw	mm2, mm4
		pshufw	mm6, mm3, 11111111B
		;
			movq	mm5, mm1
		pmulhw	mm3, mm0		; 第2ピクセル
			pmulhw	mm5, mm6
			psrlw	mm6, 1
			paddsw	mm0, MMWORD PTR [ebx].rgbDeltaColor[0]
			pxor	mm6, mmxWord3XorFFMask
		paddsw	mm3, mm5
			paddsw	mm1, MMWORD PTR [ebx].rgbDeltaColor[8]
		;
		; 結果を書き出す
		;
		packuswb	mm2, mm3
		packuswb	mm7, mm6
		movq	MMWORD PTR [ebx].rgbNextColor[0], mm0
		mov	ecx, [ebx].dib.nLeftWidth
		movq	MMWORD PTR [edi].rgbAdd[0], mm2
		movq	MMWORD PTR [ebx].rgbNextColor[8], mm1
		movq	MMWORD PTR [edi].rgbMul[0], mm7
		add	edi, (SIZEOF E3D_TRANS_LINE_BUF)
		;
		dec	ecx
	.UNTIL	ZERO?

	mov	ebp, [ebx].dib.dwSaveRegEBP
	ASSUME	edi:NOTHING
	ASSUME	ebx:NOTHING
	ret

eglRenderPoly@RenderPolygon_LineSAXSSE	ENDP

;
;	テクスチャ（RGB タイリング）シェーディング関数 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_LineTTXSSE	PROC	NEAR32 C

	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	mov	[ebx].dib.dwSaveRegEBP, ebp
	mov	edi, [ebx].pLineBuf[0]
	ASSUME	edi:PTR E3D_TRANS_LINE_BUF
	mov	ebp, [ebx].dib.ptrZBufLine
	;
	; パラメータ読み込み
	;
		mov	eax, [ebx].nLineLeft[0]
	movq	mm0, MMWORD PTR [ebx].rgbNextColor[0]
		and	eax, 01H
	movq	mm1, MMWORD PTR [ebx].rgbNextColor[8]
		neg	eax
	movq	mm2, MMWORD PTR [ebx].rgbDeltaColor[0]
		movd	mm6, eax
	movq	mm3, MMWORD PTR [ebx].rgbDeltaColor[8]
		punpckldq	mm6, mm6
		movq	mm7, mm6
		pand	mm6, mm2
		pand	mm7, mm3
		psubsw	mm0, mm6
		psubsw	mm1, mm7
		paddsw	mm0, mmxConst2000x3_0
		paddsw	mm1, mmxConst2000x3_0
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
	shr	ecx, 1
	ASSERT	<!!CARRY?>, "!(ecx & 01H)"
	inc	ecx
	;
	; テクスチャ座標用パラメータを進める
	; また、第1ピクセルのシェーディングパラメータを準備する
	;
	rcpps	xmm4, xmm1		; xmm4 <- 1 / xmm1
		movaps	xmm6, xmm0
		movaps	xmm7, xmm1
		addps	xmm0, xmm2
	mulps	xmm7, xmm4
		addps	xmm1, xmm3
	rcpps	xmm7, xmm7
	mulps	xmm4, xmm7
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
	.REPEAT
		;
		; テクスチャ画像を取得する
		;
		; テクスチャ座標用パラメータを進める
		; また、第1ピクセルのシェーディングパラメータを準備する
		;
		prefetchnta	[ebp]
			movq	mm4, mm0
			movq	mm5, mm1
		cvtps2pi	mm6, xmm4
		movhlps	xmm4, xmm4
			paddsw	mm0, mm2
			paddsw	mm1, mm3
			psraw	mm5, 7
			add	ebp, 8
			movq	MMWORD PTR [ebx].rgbNextColor[8], mm5
		cvtps2pi	mm7, xmm4
			movq	mm5, MMWORD PTR [ebx].txSizeMask[0]
				rcpps	xmm4, xmm1		; xmm4 <- 1 / xmm1
				movaps	xmm6, xmm0
				movaps	xmm7, xmm1
				addps	xmm0, xmm2
		packssdw	mm6, mm7
			movq	mm7, MMWORD PTR [ebx].txMulAddr
				mulps	xmm7, xmm4
		pand	mm6, mm5
				addps	xmm1, xmm3
			movq	mm5, MMWORD PTR [ebx].txImageAddr
		pmaddwd	mm6, mm7
				rcpps	xmm7, xmm7
			movlps	QWORD PTR [edi].rZValue, xmm5
		paddd	mm6, mm5
				mulps	xmm4, xmm7
		;
		; 第1ピクセルを計算する
		; 第2ピクセルのための準備を行う
		;	mm5 <- 第1ピクセルの結果
		;
		; ｚ値とテクスチャマッピング用パラメータ計算のための準備を行う
		;
				movss	xmm5, [ebx].rTxoxyr
				movss	xmm7, [ebx].rRcpTxxy
		movd	eax, mm6
		psrlq	mm6, 32
				shufps	xmm5, xmm5, 0
		pxor	mm7, mm7
		movd	mm5, DWORD PTR [eax]
				shufps	xmm7, xmm7, 0
			movd	edx, mm6
		punpcklbw	mm5, mm7
				mulps	xmm5, xmm4		; xmm5 = z 値
		psllw	mm5, 1
			movq	mm6, MMWORD PTR [ebx].rgbNextColor[8]
		pmulhw	mm5, mm4
				unpcklps	xmm4, xmm4
			movd	mm4, DWORD PTR [edx]
		paddsw	mm5, mm6
		;
		; 第2ピクセルを計算し、パラメータを進める
		;	mm4 <- 第2ピクセルの結果
		;
				mulps	xmm4, xmm7		; xmm4 = マッピング座標
			movq	mm6, mm1
		punpcklbw	mm4, mm7
			psraw	mm6, 7
		psllw	mm4, 1
			paddsw	mm1, mm3
		pmulhw	mm4, mm0
			paddsw	mm0, mm2
				mulps	xmm4, xmm6
		paddsw	mm4, mm6
		;
		; 結果を書き出す
		;
		packuswb	mm5, mm4
		movq	MMWORD PTR [edi].rgbMul, mm7
		movq	MMWORD PTR [edi].rgbAdd, mm5
		add	edi, (SIZEOF E3D_TRANS_LINE_BUF)
		;
		dec	ecx
	.UNTIL	ZERO?

	mov	ebp, [ebx].dib.dwSaveRegEBP
	ASSUME	edi:NOTHING
	ASSUME	ebx:NOTHING
	ret

eglRenderPoly@RenderPolygon_LineTTXSSE	ENDP

;
;	テクスチャ（RGBA タイリング）シェーディング関数 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_LineTATXSSE	PROC	NEAR32 C

	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	mov	[ebx].dib.dwSaveRegEBP, ebp
	mov	edi, [ebx].pLineBuf[0]
	ASSUME	edi:PTR E3D_TRANS_LINE_BUF
	mov	ebp, [ebx].dib.ptrZBufLine
	;
	; パラメータ読み込み
	;
		mov	eax, [ebx].nLineLeft[0]
	movq	mm0, MMWORD PTR [ebx].rgbNextColor[0]
		and	eax, 01H
	movq	mm1, MMWORD PTR [ebx].rgbNextColor[8]
		neg	eax
	movq	mm2, MMWORD PTR [ebx].rgbDeltaColor[0]
		movd	mm6, eax
	movq	mm3, MMWORD PTR [ebx].rgbDeltaColor[8]
		punpckldq	mm6, mm6
		movq	mm7, mm6
		pand	mm6, mm2
		pand	mm7, mm3
		psubsw	mm0, mm6
		psubsw	mm1, mm7
		paddsw	mm0, mmxConst2000x3_0
		paddsw	mm1, mmxConst2000x3_0
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
			movq	mm2, mm0
			movq	mm3, mm1
		rcpps	xmm7, xmm7
			paddsw	mm0, MMWORD PTR [ebx].rgbDeltaColor[0]
			paddsw	mm1, MMWORD PTR [ebx].rgbDeltaColor[8]
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
;		movss	xmm7, [ebx].vScreenPos.z
;		shufps	xmm7, xmm7, 0
		mulps	xmm4, xmm6
;		mulps	xmm5, xmm7
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
		; 第2ピクセルのための準備を行う
		;	mm4:mm5 <- 第1ピクセルの結果
		;
		movd	eax, mm6
		psrlq	mm6, 32
		pxor	mm7, mm7
		movd	mm4, DWORD PTR [eax]
			movd	edx, mm6
		punpcklbw	mm4, mm7
		psllw	mm4, 1
			pshufw	mm5, mm4, 11111111B
		pmulhw	mm4, mm2
			pmulhw	mm3, mm5
			movd	mm2, DWORD PTR [edx]
			psrlw	mm5, 1
			pxor	mm5, mmxWord3XorFFMask
		paddsw	mm4, mm3
		;
		; 第2ピクセルを計算し、パラメータを進める
		;	mm2:mm3 <- 第2ピクセルの結果
		;
		punpcklbw	mm2, mm7
			movq	mm6, mm1
		psllw	mm2, 1
			pshufw	mm3, mm2, 11111111B
		pmulhw	mm2, mm0
			pmulhw	mm6, mm3
			paddsw	mm0, MMWORD PTR [ebx].rgbDeltaColor[0]
			psrlw	mm3, 1
			pxor	mm3, mmxWord3XorFFMask
		paddsw	mm2, mm6
			paddsw	mm1, MMWORD PTR [ebx].rgbDeltaColor[8]
		;
		; 結果を書き出す
		;
		packuswb	mm4, mm2
		packuswb	mm5, mm3
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

eglRenderPoly@RenderPolygon_LineTATXSSE	ENDP

;
;	テクスチャ（RGB＋発光 タイリング）シェーディング関数 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_LineTLTXSSE	PROC	NEAR32 C

	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	mov	[ebx].dib.dwSaveRegEBP, ebp
	mov	edi, [ebx].pLineBuf[0]
	ASSUME	edi:PTR E3D_TRANS_LINE_BUF
	mov	ebp, [ebx].dib.ptrZBufLine
	;
	; パラメータ読み込み
	;
		mov	eax, [ebx].nLineLeft[0]
	movq	mm0, MMWORD PTR [ebx].rgbNextColor[0]
		and	eax, 01H
	movq	mm1, MMWORD PTR [ebx].rgbNextColor[8]
		neg	eax
	movq	mm2, MMWORD PTR [ebx].rgbDeltaColor[0]
		movd	mm6, eax
	movq	mm3, MMWORD PTR [ebx].rgbDeltaColor[8]
		punpckldq	mm6, mm6
		movq	mm7, mm6
		pand	mm6, mm2
		pand	mm7, mm3
		psubsw	mm0, mm6
		psubsw	mm1, mm7
		paddsw	mm0, mmxConst2000x3_0
		paddsw	mm1, mmxConst2000x3_0
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
			movq	mm2, mm0
			movq	mm3, mm1
		rcpps	xmm7, xmm7
			paddsw	mm0, MMWORD PTR [ebx].rgbDeltaColor[0]
			paddsw	mm1, MMWORD PTR [ebx].rgbDeltaColor[8]
		mulps	xmm4, xmm7
			psraw	mm3, 7
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
;		movss	xmm7, [ebx].vScreenPos.z
;		shufps	xmm7, xmm7, 0
		mulps	xmm4, xmm6
;		mulps	xmm5, xmm7
		;
		; テクスチャ画像を取得する
		;
		cvtps2pi	mm6, xmm4
		movhlps	xmm4, xmm4
			movq	mm4, MMWORD PTR [ebx].txSizeMask[0]
			movq	mm5, MMWORD PTR [ebx].lmSizeMask[0]
		cvtps2pi	mm7, xmm4
		packssdw	mm6, mm7
			movq	mm7, MMWORD PTR [ebx].txMulAddr
		pand	mm4, mm6
		pand	mm5, mm6
			movq	mm6, MMWORD PTR [ebx].lmMulAddr
		pmaddwd	mm4, mm7
			movq	mm7, MMWORD PTR [ebx].txImageAddr
		pmaddwd	mm5, mm6
			movq	mm6, MMWORD PTR [ebx].lmImageAddr
		movlps	QWORD PTR [edi].rZValue, xmm5
		paddd	mm4, mm7
		paddd	mm5, mm6
		;
		; 第1ピクセルを計算する
		;	mm6 <- 第1ピクセルの結果
		;
		movd	eax, mm4
		movd	edx, mm5
		psrlq	mm4, 32
		psrlq	mm5, 32
		movd	mm6, DWORD PTR [eax]
		movd	mm7, DWORD PTR [edx]
		movd	eax, mm4
		pxor	mm4, mm4
		movd	edx, mm5
		movd	mm5, DWORD PTR [ebx].nLiminousApply
		punpcklbw	mm6, mm4
		punpcklbw	mm7, mm4
		pshufw	mm5, mm5, 0
		psllw	mm6, 1
		pmulhw	mm6, mm2
			movq	mm2, mm0
			paddsw	mm0, MMWORD PTR [ebx].rgbDeltaColor[0]
		pmullw	mm7, mm5
			movd	mm5, DWORD PTR [eax]
		paddsw	mm6, mm3
			movq	mm3, mm1
		psrlw	mm7, 8
			paddsw	mm1, MMWORD PTR [ebx].rgbDeltaColor[8]
		paddsw	mm6, mm7
		;
		; 第2ピクセルを計算する
		;	mm7 <- 第2ピクセルの結果
		;
		movd	mm7, DWORD PTR [edx]
		punpcklbw	mm5, mm4
		punpcklbw	mm7, mm4
		movd	mm4, DWORD PTR [ebx].nLiminousApply
		psllw	mm5, 1
		pshufw	mm4, mm4, 0
		pmulhw	mm5, mm2
		psraw	mm3, 7
		pmullw	mm7, mm4
		paddsw	mm5, mm3
		psrlw	mm7, 8
		paddsw	mm7, mm5
		;
		; 結果を書き出す
		;
		packuswb	mm6, mm7
		pxor	mm7, mm7
		movq	MMWORD PTR [edi].rgbAdd[0], mm6
		movq	MMWORD PTR [edi].rgbMul[0], mm7
		add	edi, (SIZEOF E3D_TRANS_LINE_BUF)
		;
		dec	ecx
	.UNTIL	ZERO?

	mov	ebp, [ebx].dib.dwSaveRegEBP
	ASSUME	edi:NOTHING
	ASSUME	ebx:NOTHING
	ret

eglRenderPoly@RenderPolygon_LineTLTXSSE	ENDP

;
;	テクスチャ（RGB 補完 タイリング）シェーディング関数 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_LineSTXSSE	PROC	NEAR32 C

	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	mov	[ebx].dib.dwSaveRegEBP, ebp
	mov	edi, [ebx].pLineBuf[0]
	ASSUME	edi:PTR E3D_TRANS_LINE_BUF
	mov	ebp, [ebx].dib.ptrZBufLine
	;
	; パラメータ読み込み
	;
		mov	eax, [ebx].nLineLeft[0]
	movq	mm0, MMWORD PTR [ebx].rgbNextColor[0]
		and	eax, 01H
	movq	mm1, MMWORD PTR [ebx].rgbNextColor[8]
		neg	eax
	movq	mm2, MMWORD PTR [ebx].rgbDeltaColor[0]
		movd	mm6, eax
	movq	mm3, MMWORD PTR [ebx].rgbDeltaColor[8]
		punpckldq	mm6, mm6
		movq	mm7, mm6
		pand	mm6, mm2
		pand	mm7, mm3
		psubsw	mm0, mm6
		psubsw	mm1, mm7
		paddsw	mm0, mmxConst2000x3_0
		paddsw	mm1, mmxConst2000x3_0
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
	shr	ecx, 1
	ASSERT	<!!CARRY?>, "!(ecx & 01H)"
	inc	ecx
	mov	[ebx].dib.nLeftWidth, ecx
		;
		; テクスチャ座標用パラメータを進める
		; また、第1ピクセルのシェーディングパラメータを準備する
		;
		rcpps	xmm4, xmm1		; xmm4 <- 1 / xmm1
			movaps	xmm6, xmm0
			movaps	xmm7, xmm1
		mulps	xmm7, xmm4
			addps	xmm0, xmm2
			movq	MMWORD PTR [ebx].rgbNextColor[0], mm0
		rcpps	xmm7, xmm7
			addps	xmm1, xmm3
			movq	MMWORD PTR [ebx].rgbNextColor[8], mm1
		mulps	xmm4, xmm7
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
	mov	ecx, [ebx].dib.nLeftWidth
	.REPEAT
		mov	[ebx].dib.nLeftWidth, ecx
		;
		; 第1ピクセルのマッピング座標を取得する
		;
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
					;
					; テクスチャ座標用パラメータを進める
					; また、第1ピクセルのシェーディングパラメータを準備する
					;
			inc	ecx
		pxor	mm7, mm7
		movd	mm4, DWORD PTR [esi + eax * 4]
			and	cx, [ebx].txSizeMask[2]
		movd	mm5, DWORD PTR [esi + edx * 4]
			mov	esi, [ebx].pTxLineAddr
			movzx	ecx, cx
					prefetchnta	[ebp]
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
					rcpps	xmm4, xmm1		; xmm4 <- 1 / xmm1
			pmulhw	mm1, mm2
						movaps	xmm6, xmm0
				pshufw	mm2, mm4, 00000000B
						movaps	xmm7, xmm1
				pshufw	mm6, mm4, 10101010B
			paddsw	mm1, mm0
		;
					mulps	xmm7, xmm4
						addps	xmm0, xmm2
			psrld	mm4, 16
		psubsw	mm1, mm5
			movd	eax, mm4
		psllw	mm1, 1
			psrlq	mm4, 32
		pmulhw	mm1, mm3
					rcpps	xmm7, xmm7
						addps	xmm1, xmm3
			mov	esi, [ebx].pTxLineAddr
				psrlw	mm2, 1
				psrlw	mm6, 1
		paddsw	mm1, mm5
					mulps	xmm4, xmm7
					add	ebp, 8
		;
		; 第2ピクセルの値を補完する
		;
					;
					; ｚ値とテクスチャマッピング用パラメータ計算のための準備を行う
					;
					movss	xmm5, [ebx].rTxoxyr
					movss	xmm7, [ebx].rRcpTxxy
		movd	ecx, mm4
		lea	edx, [eax + 1]
		and	cx, [ebx].txSizeMask[2]
		and	eax, DWORD PTR [ebx].txSizeMask[0]
					shufps	xmm5, xmm5, 0
			movzx	ecx, cx
		and	edx, DWORD PTR [ebx].txSizeMask[0]
			and	eax, 0FFFFH
					shufps	xmm7, xmm7, 0
			and	edx, 0FFFFH
		mov	esi, DWORD PTR [esi + ecx * 4]
			movq	MMWORD PTR [edi].rgbAdd[0], mm1
					mulps	xmm5, xmm4		; xmm5 = z 値
		;
		inc	ecx
		movd	mm4, DWORD PTR [esi + eax * 4]
			and	cx, [ebx].txSizeMask[2]
					unpcklps	xmm4, xmm4
		movd	mm5, DWORD PTR [esi + edx * 4]
			mov	esi, [ebx].pTxLineAddr
			movzx	ecx, cx
		punpcklbw	mm4, mm7
					mulps	xmm4, xmm7		; xmm4 = マッピング座標
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
					mulps	xmm4, xmm6
		paddsw	mm5, mm4
			psllw	mm3, 1
			pmulhw	mm3, mm2
				movq	mm2, MMWORD PTR [edi].rgbAdd[0]
				movq	mm1, MMWORD PTR [ebx].rgbNextColor[8]
			paddsw	mm3, mm0
				movq	mm0, MMWORD PTR [ebx].rgbNextColor[0]
		;
		; シェーディングを施す
		;
		psubsw	mm3, mm5
			psllw	mm2, 1
		psllw	mm3, 1
			pmulhw	mm2, mm0	; 第1ピクセル
		pmulhw	mm3, mm6
			movq	mm4, mm1
			paddsw	mm0, MMWORD PTR [ebx].rgbDeltaColor[0]
			psraw	mm4, 7
		paddsw	mm3, mm5
			paddsw	mm1, MMWORD PTR [ebx].rgbDeltaColor[8]
		psllw	mm3, 1
			paddsw	mm2, mm4
		;
		pmulhw	mm3, mm0		; 第2ピクセル
			movq	mm5, mm1
			paddsw	mm0, MMWORD PTR [ebx].rgbDeltaColor[0]
			psraw	mm5, 7
		paddsw	mm3, mm5
			paddsw	mm1, MMWORD PTR [ebx].rgbDeltaColor[8]
		;
		; 結果を書き出す
		;
		packuswb	mm2, mm3
		movq	MMWORD PTR [ebx].rgbNextColor[0], mm0
		mov	ecx, [ebx].dib.nLeftWidth
		movq	MMWORD PTR [edi].rgbAdd[0], mm2
		movq	MMWORD PTR [edi].rgbMul[0], mm7
		add	edi, (SIZEOF E3D_TRANS_LINE_BUF)
		movq	MMWORD PTR [ebx].rgbNextColor[8], mm1
		;
		dec	ecx
	.UNTIL	ZERO?

	mov	ebp, [ebx].dib.dwSaveRegEBP
	ASSUME	edi:NOTHING
	ASSUME	ebx:NOTHING
	ret

eglRenderPoly@RenderPolygon_LineSTXSSE	ENDP

;
;	テクスチャ（RGBA 補完 タイリング）シェーディング関数 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_LineSTAXSSE	PROC	NEAR32 C

	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	mov	[ebx].dib.dwSaveRegEBP, ebp
	mov	edi, [ebx].pLineBuf[0]
	ASSUME	edi:PTR E3D_TRANS_LINE_BUF
	mov	ebp, [ebx].dib.ptrZBufLine
	;
	; パラメータ読み込み
	;
		mov	eax, [ebx].nLineLeft[0]
	movq	mm0, MMWORD PTR [ebx].rgbNextColor[0]
		and	eax, 01H
	movq	mm1, MMWORD PTR [ebx].rgbNextColor[8]
		neg	eax
	movq	mm2, MMWORD PTR [ebx].rgbDeltaColor[0]
		movd	mm6, eax
	movq	mm3, MMWORD PTR [ebx].rgbDeltaColor[8]
		punpckldq	mm6, mm6
		movq	mm7, mm6
		pand	mm6, mm2
		pand	mm7, mm3
		psubsw	mm0, mm6
		psubsw	mm1, mm7
		paddsw	mm0, mmxConst2000x3_0
		paddsw	mm1, mmxConst2000x3_0
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
			movq	MMWORD PTR [ebx].rgbNextColor[0], mm0
		rcpps	xmm7, xmm7
			addps	xmm1, xmm3
			movq	MMWORD PTR [ebx].rgbNextColor[8], mm1
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
;		movss	xmm7, [ebx].vScreenPos.z
;		shufps	xmm7, xmm7, 0
		mulps	xmm4, xmm6
;		mulps	xmm5, xmm7
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
				movq	mm1, MMWORD PTR [ebx].rgbNextColor[8]
			paddsw	mm3, mm0
				movq	mm0, MMWORD PTR [ebx].rgbNextColor[0]
		;
		; シェーディングを施す
		;
			psllw	mm2, 1
		psubsw	mm3, mm5
			pshufw	mm7, mm2, 11111111B
		psllw	mm3, 1
			pmulhw	mm2, mm0	; 第1ピクセル
		pmulhw	mm3, mm6
			movq	mm4, mm1
			paddsw	mm0, MMWORD PTR [ebx].rgbDeltaColor[0]
			pmulhw	mm4, mm7
			psrlw	mm7, 1
		paddsw	mm3, mm5
			pxor	mm7, mmxWord3XorFFMask
			paddsw	mm1, MMWORD PTR [ebx].rgbDeltaColor[8]
		psllw	mm3, 1
			paddsw	mm2, mm4
		pshufw	mm6, mm3, 11111111B
		;
			movq	mm5, mm1
		pmulhw	mm3, mm0		; 第2ピクセル
			pmulhw	mm5, mm6
			psrlw	mm6, 1
			paddsw	mm0, MMWORD PTR [ebx].rgbDeltaColor[0]
			pxor	mm6, mmxWord3XorFFMask
		paddsw	mm3, mm5
			paddsw	mm1, MMWORD PTR [ebx].rgbDeltaColor[8]
		;
		; 結果を書き出す
		;
		packuswb	mm2, mm3
		packuswb	mm7, mm6
		movq	MMWORD PTR [ebx].rgbNextColor[0], mm0
		mov	ecx, [ebx].dib.nLeftWidth
		movq	MMWORD PTR [edi].rgbAdd[0], mm2
		movq	MMWORD PTR [ebx].rgbNextColor[8], mm1
		movq	MMWORD PTR [edi].rgbMul[0], mm7
		add	edi, (SIZEOF E3D_TRANS_LINE_BUF)
		;
		dec	ecx
	.UNTIL	ZERO?

	mov	ebp, [ebx].dib.dwSaveRegEBP
	ASSUME	edi:NOTHING
	ASSUME	ebx:NOTHING
	ret

eglRenderPoly@RenderPolygon_LineSTAXSSE	ENDP

;
;	テクスチャ（RGB＋発光補完）シェーディング関数 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_LineSTLXSSE	PROC	NEAR32 C

	call	eglRenderPoly@RenderPolygon_LineTLTXSSE
	ret

eglRenderPoly@RenderPolygon_LineSTLXSSE	ENDP

;
;	フォンシェーディング関数
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_LineShading	PROC	NEAR32 C

	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF

	;
	; ピクセルシェーディング
	;
	call	[ebx].pfnLineSubFunc

	;
	; 座標・色・法線をセットアップ
	;
	cvtsi2ss	xmm0, [ebx].nLineLeft[4]
	cvtsi2ss	xmm1, [ebx].yLinePos
		movq	mm1, MMWORD PTR [ebx].vLineLeftNormal
		movq	mm3, MMWORD PTR [ebx].vLineRightNormal
	movlps		xmm2, QWORD PTR [ebx].vScreenPos
	unpcklps	xmm0, xmm1		; xmm0 <- x, y
		punpcklwd	mm0, mm1
		punpckhwd	mm1, mm1
	subps		xmm0, xmm2
		punpcklwd	mm2, mm3
		punpckhwd	mm3, mm3
	movss		xmm3, [ebx].vScreenPos.z
	movhps		xmm0, QWORD PTR xmmConst1_0_1_0
	shufps		xmm3, xmm3, 00000000B
	divps		xmm0, xmm3	; { x, y, 1, 0 } / vScreenPos.z
		psrad	mm0, 16
		psrad	mm1, 16
		psrad	mm2, 16
		psrad	mm3, 16
		cvtpi2ps	xmm6, mm0
		cvtpi2ps	xmm4, mm1
		cvtpi2ps	xmm7, mm2
		cvtpi2ps	xmm5, mm3
		movlhps		xmm6, xmm4	; xmm6 <- left normal
		movlhps		xmm7, xmm5
		subps		xmm7, xmm6	; xmm7 <- delta normal
	mov	ecx, [ebx].nLineRight[0]
	mov	esi, [ebx].pLineBuf[0]
	mov	edi, [ebx].pLineVectorBuf
	sub	ecx, [ebx].nLineLeft[0]
	mov	eax, [ebx].pLineNormalBuf
	mov	edx, [ebx].pLineColorBuf
	inc	ecx
	;
	movhlps		xmm1, xmm0	; xmm1 <- { 1, 0 } / vScreenPos.z
		cvtsi2ss	xmm4, ecx
	mov	eax, [ebx].pLineNormalBuf
	movaps		xmm2, xmm0
		shufps		xmm4, xmm4, 0
	addps		xmm2, xmm1
		divps		xmm7, xmm4
	mov	ecx, [ebx].nLineRight[4]
		cvtsi2ss	xmm5, [ebx].nLeftDecimal
	sub	ecx, [ebx].nLineLeft[4]
	movlhps		xmm0, xmm2	; xmm0 <- { x, y } / vScreenPos.z
	movlhps		xmm1, xmm1
		mulss		xmm5, xmmConstDiv10000H
	addps		xmm1, xmm1
	;
	shr	ecx, 1
	movaps	xmm4, xmmMask1_0_1_0
	inc	ecx
	;
	shufps	xmm5, xmm5, 0
	.IF	[ebx].nLineLeft[0] & 1
		subps	xmm6, xmm7
	.ENDIF
	mulps	xmm5, xmm7
	subps	xmm6, xmm5
	;
	ASSUME	esi:PTR E3D_TRANS_LINE_BUF
	ASSUME	edi:PTR E3D_VECTOR4
	ASSUME	eax:PTR E3D_VECTOR4
	ASSUME	edx:PTR E3D_COLOR
	;
	.REPEAT
			movaps		[eax], xmm6
			addps		xmm6, xmm7
		movlps		xmm3, QWORD PTR [esi].rZValue[0]
		movaps		xmm2, xmm0
		addps		xmm0, xmm1
			movq	mm0, MMWORD PTR [esi].rgbMul[0]
			movq	mm2, MMWORD PTR [esi].rgbAdd[0]
			movaps		[eax + (SIZEOF E3D_VECTOR4)], xmm6
			addps		xmm6, xmm7
		unpcklps	xmm3, xmm3
			movq	mm1, mm0
		mulps		xmm2, xmm3
			punpckldq	mm0, mm2
			punpckhdq	mm1, mm2
		andps		xmm3, xmm4
		;
		movlps		QWORD PTR [edi].x, xmm2
		movlps		QWORD PTR [edi].z, xmm3
			movq	MMWORD PTR [edx], mm0
		movhps		QWORD PTR [edi + (SIZEOF E3D_VECTOR4)].x, xmm2
		movhps		QWORD PTR [edi + (SIZEOF E3D_VECTOR4)].z, xmm3
			movq	MMWORD PTR [edx + (SIZEOF E3D_COLOR)], mm1
		;
		add	esi, (SIZEOF E3D_TRANS_LINE_BUF)
		add	edi, (SIZEOF E3D_VECTOR4) * 2
		add	eax, (SIZEOF E3D_VECTOR4) * 2
		add	edx, (SIZEOF E3D_COLOR) * 2
		;
		dec	ecx
	.UNTIL	ZERO?
	;
	ASSUME	esi:NOTHING
	ASSUME	edi:NOTHING
	ASSUME	eax:NOTHING
	ASSUME	edx:NOTHING
	;
	; シェーディング実行
	;
	mov	ecx, [ebx].nLineRight[4]
	sub	ecx, [ebx].nLineLeft[4]
	add	ecx, 2
	;
	INVOKE	[ebx].pfnLineShading ,
			ebx, [ebx].pSurfaceAttr,
			[ebx].pLineNormalBuf, [ebx].pLineVectorBuf,
			[ebx].pLineColorBuf, ecx, ADDR [ebx].vTargetPlaneParam
	;
	; シェーディング済みの色をラインバッファに戻す
	;
	mov	ecx, [ebx].nLineRight[4]
	mov	esi, [ebx].pLineColorBuf
	sub	ecx, [ebx].nLineLeft[4]
	mov	edi, [ebx].pLineBuf[0]
	;
	ASSUME	esi:PTR E3D_COLOR
	ASSUME	edi:PTR E3D_TRANS_LINE_BUF
	;
	shr	ecx, 1
	inc	ecx
	;
	.REPEAT
		movq	mm0, MMWORD PTR [esi]
		movq	mm1, MMWORD PTR [esi + (SIZEOF E3D_COLOR)]
		movq	mm2, mm0
		punpckldq	mm0, mm1
		punpckhdq	mm2, mm1
		movq	MMWORD PTR [edi].rgbMul[0], mm0
		movq	MMWORD PTR [edi].rgbAdd[0], mm2
		;
		add	esi, (SIZEOF E3D_COLOR) * 2
		add	edi, (SIZEOF E3D_TRANS_LINE_BUF)
		dec	ecx
	.UNTIL	ZERO?

	ASSUME	ebx:NOTHING
	ret

eglRenderPoly@RenderPolygon_LineShading	ENDP

;
;	フォンシェーディング関数（テクスチャーマッピング）
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_LineShadingTexture	PROC	NEAR32 C

	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF

	;
	; ライン色補間
	;
		mov	eax, [ebx].nLineLeft[0]
	movq	mm0, MMWORD PTR [ebx].rgbNextColor[0]
		and	eax, 01H
	movq	mm1, MMWORD PTR [ebx].rgbNextColor[8]
		neg	eax
	movq	mm2, MMWORD PTR [ebx].rgbDeltaColor[0]
		movd	mm6, eax
	movq	mm3, MMWORD PTR [ebx].rgbDeltaColor[8]
		punpckldq	mm6, mm6
		movq	mm7, mm6
	mov	ecx, [ebx].nLineRight[4]
		pand	mm6, mm2
	mov	edx, [ebx].pLineColorBuf
		pand	mm7, mm3
	sub	ecx, [ebx].nLineLeft[4]
		psubsw	mm0, mm6
	shr	ecx, 1
		psubsw	mm1, mm7
		paddsw	mm0, mmxConst2000x3_0
		paddsw	mm1, mmxConst2000x3_0
	mov	edx, [ebx].pLineColorBuf
	inc	ecx
	;
	.REPEAT
		movq	mm4, mm0
		paddsw	mm0, mm2
		movq	mm5, mm1
		paddsw	mm1, mm3
		psraw	mm4, 7
		psraw	mm5, 7
		movq	mm6, mm0
		paddsw	mm0, mm2
		movq	mm7, mm1
		paddsw	mm1, mm3
		psraw	mm6, 7
		psraw	mm7, 7
		packuswb	mm4, mm5
		packuswb	mm6, mm7
		movq	MMWORD PTR [edx], mm4
		movq	MMWORD PTR [edx + (SIZEOF E3D_COLOR)], mm6
		add	edx, (SIZEOF E3D_COLOR) * 2
		dec	ecx
	.UNTIL	ZERO?
	;
	movq	mm0, mmxWhiteColorWords
	pxor	mm1, mm1
	movq	MMWORD PTR [ebx].rgbNextColor[0], mm0
	movq	MMWORD PTR [ebx].rgbNextColor[8], mm1
	movq	MMWORD PTR [ebx].rgbDeltaColor[0], mm1
	movq	MMWORD PTR [ebx].rgbDeltaColor[8], mm1
	;
	; ピクセルシェーディング
	;
	call	[ebx].pfnLineSubFunc

	;
	; 座標・色・法線をセットアップ
	;
	cvtsi2ss	xmm0, [ebx].nLineLeft[4]
	cvtsi2ss	xmm1, [ebx].yLinePos
		movq	mm1, MMWORD PTR [ebx].vLineLeftNormal
		movq	mm3, MMWORD PTR [ebx].vLineRightNormal
	movlps		xmm2, QWORD PTR [ebx].vScreenPos
	unpcklps	xmm0, xmm1		; xmm0 <- x, y
		punpcklwd	mm0, mm1
		punpckhwd	mm1, mm1
	subps		xmm0, xmm2
		punpcklwd	mm2, mm3
		punpckhwd	mm3, mm3
	movss		xmm3, [ebx].vScreenPos.z
	movhps		xmm0, QWORD PTR xmmConst1_0_1_0
	shufps		xmm3, xmm3, 00000000B
	divps		xmm0, xmm3	; { x, y, 1, 0 } / vScreenPos.z
		psrad	mm0, 16
		psrad	mm1, 16
		psrad	mm2, 16
		psrad	mm3, 16
		cvtpi2ps	xmm6, mm0
		cvtpi2ps	xmm4, mm1
		cvtpi2ps	xmm7, mm2
		cvtpi2ps	xmm5, mm3
		movlhps		xmm6, xmm4	; xmm6 <- left normal
		movlhps		xmm7, xmm5
		subps		xmm7, xmm6	; xmm7 <- delta normal
	mov	ecx, [ebx].nLineRight[0]
	mov	esi, [ebx].pLineBuf[0]
	mov	edi, [ebx].pLineVectorBuf
	sub	ecx, [ebx].nLineLeft[0]
	mov	eax, [ebx].pLineNormalBuf
	inc	ecx
	;
	movhlps		xmm1, xmm0	; xmm1 <- { 1, 0 } / vScreenPos.z
		cvtsi2ss	xmm4, ecx
	mov	eax, [ebx].pLineNormalBuf
	movaps		xmm2, xmm0
		shufps		xmm4, xmm4, 0
	addps		xmm2, xmm1
		divps		xmm7, xmm4
	mov	ecx, [ebx].nLineRight[4]
		cvtsi2ss	xmm5, [ebx].nLeftDecimal
	sub	ecx, [ebx].nLineLeft[4]
	movlhps		xmm0, xmm2	; xmm0 <- { x, y } / vScreenPos.z
	movlhps		xmm1, xmm1
		mulss		xmm5, xmmConstDiv10000H
	addps		xmm1, xmm1
	;
	shr	ecx, 1
	movaps	xmm4, xmmMask1_0_1_0
	inc	ecx
	;
	shufps	xmm5, xmm5, 0
	.IF	[ebx].nLineLeft[0] & 1
		subps	xmm6, xmm7
	.ENDIF
	mulps	xmm5, xmm7
	subps	xmm6, xmm5
	;
	ASSUME	esi:PTR E3D_TRANS_LINE_BUF
	ASSUME	edi:PTR E3D_VECTOR4
	ASSUME	eax:PTR E3D_VECTOR4
	;
	.REPEAT
			movaps		[eax], xmm6
		movlps		xmm3, QWORD PTR [esi].rZValue[0]
			addps		xmm6, xmm7
		movaps		xmm2, xmm0
		addps		xmm0, xmm1
		unpcklps	xmm3, xmm3
			movaps		[eax + (SIZEOF E3D_VECTOR4)], xmm6
		mulps		xmm2, xmm3
			addps		xmm6, xmm7
		andps		xmm3, xmm4
		;
		movlps		QWORD PTR [edi].x, xmm2
		movlps		QWORD PTR [edi].z, xmm3
		movhps		QWORD PTR [edi + (SIZEOF E3D_VECTOR4)].x, xmm2
		movhps		QWORD PTR [edi + (SIZEOF E3D_VECTOR4)].z, xmm3
		;
		add	esi, (SIZEOF E3D_TRANS_LINE_BUF)
		add	edi, (SIZEOF E3D_VECTOR4) * 2
		add	eax, (SIZEOF E3D_VECTOR4) * 2
		;
		dec	ecx
	.UNTIL	ZERO?
	;
	ASSUME	esi:NOTHING
	ASSUME	edi:NOTHING
	ASSUME	eax:NOTHING
	ASSUME	edx:NOTHING
	;
	; シェーディング実行
	;
	mov	ecx, [ebx].nLineRight[4]
	sub	ecx, [ebx].nLineLeft[4]
	add	ecx, 2
	;
	INVOKE	[ebx].pfnLineShading ,
			ebx, [ebx].pSurfaceAttr,
			[ebx].pLineNormalBuf, [ebx].pLineVectorBuf,
			[ebx].pLineColorBuf, ecx, ADDR [ebx].vTargetPlaneParam
	;
	; シェーディング済みの色をラインバッファに戻す
	;
	mov	ecx, [ebx].nLineRight[4]
	mov	esi, [ebx].pLineColorBuf
	sub	ecx, [ebx].nLineLeft[4]
	mov	edi, [ebx].pLineBuf[0]
	;
	ASSUME	esi:PTR E3D_COLOR
	ASSUME	edi:PTR E3D_TRANS_LINE_BUF
	;
	shr	ecx, 1
	pxor	mm7, mm7
	inc	ecx
	pcmpeqw		mm6, mm6
	;
	.REPEAT
		movd		mm5, DWORD PTR [edi].rgbMul[0]
		movq		mm0, MMWORD PTR [esi]
		movd		mm4, DWORD PTR [edi].rgbAdd[0]
		pxor		mm5, mm6
		movq		mm1, mm0
		punpcklbw	mm0, mm7
		punpcklbw	mm4, mm7
		punpcklbw	mm5, mm7
		punpckhbw	mm1, mm7
		psubsw		mm0, mm6
		psubsw		mm5, mm6
			movq		mm2, MMWORD PTR [esi + (SIZEOF E3D_COLOR)]
		pmullw		mm4, mm0
			movd		mm0, DWORD PTR [edi].rgbAdd[4]
			movq		mm3, mm2
		pmullw		mm1, mm5
			movd		mm5, DWORD PTR [edi].rgbMul[4]
			punpcklbw	mm2, mm7
			punpckhbw	mm3, mm7
			pxor		mm5, mm6
			punpcklbw	mm0, mm7
			punpcklbw	mm5, mm7
			psubsw		mm2, mm6
			psubsw		mm5, mm6
			pmullw		mm0, mm2
			pmullw		mm3, mm5
		psrlw		mm4, 8
		psrlw		mm1, 8
			psrlw		mm0, 8
			psrlw		mm3, 8
		paddsw		mm4, mm1
			paddsw		mm0, mm3
		;
		add	esi, (SIZEOF E3D_COLOR) * 2
		packuswb	mm4, mm0
		movq	MMWORD PTR [edi].rgbAdd[0], mm4
		add	edi, (SIZEOF E3D_TRANS_LINE_BUF)
		dec	ecx
	.UNTIL	ZERO?

	ASSUME	ebx:NOTHING
	ret

eglRenderPoly@RenderPolygon_LineShadingTexture	ENDP


;
;	レイトレーシング関数（テクスチャーマッピング）
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_LineRayTracingTexture	PROC	NEAR32 C

	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF

	;
	; ライン色補間
	;
		mov	eax, [ebx].nLineLeft[0]
	movq	mm0, MMWORD PTR [ebx].rgbNextColor[0]
		and	eax, 01H
	movq	mm1, MMWORD PTR [ebx].rgbNextColor[8]
		neg	eax
	movq	mm2, MMWORD PTR [ebx].rgbDeltaColor[0]
		movd	mm6, eax
	movq	mm3, MMWORD PTR [ebx].rgbDeltaColor[8]
		punpckldq	mm6, mm6
		movq	mm7, mm6
	mov	ecx, [ebx].nLineRight[4]
		pand	mm6, mm2
	mov	edx, [ebx].pLineColorBuf
		pand	mm7, mm3
	sub	ecx, [ebx].nLineLeft[4]
		psubsw	mm0, mm6
	shr	ecx, 1
		psubsw	mm1, mm7
		paddsw	mm0, mmxConst2000x3_0
		paddsw	mm1, mmxConst2000x3_0
	mov	edx, [ebx].pLineColorBuf
	inc	ecx
	;
	.REPEAT
		movq	mm4, mm0
		paddsw	mm0, mm2
		movq	mm5, mm1
		paddsw	mm1, mm3
		psraw	mm4, 7
		psraw	mm5, 7
		movq	mm6, mm0
		paddsw	mm0, mm2
		movq	mm7, mm1
		paddsw	mm1, mm3
		psraw	mm6, 7
		psraw	mm7, 7
		packuswb	mm4, mm5
		packuswb	mm6, mm7
		movq	MMWORD PTR [edx], mm4
		movq	MMWORD PTR [edx + (SIZEOF E3D_COLOR)], mm6
		add	edx, (SIZEOF E3D_COLOR) * 2
		dec	ecx
	.UNTIL	ZERO?
	;
	movq	mm0, mmxWhiteColorWords
	pxor	mm1, mm1
	movq	MMWORD PTR [ebx].rgbNextColor[0], mm0
	movq	MMWORD PTR [ebx].rgbNextColor[8], mm1
	movq	MMWORD PTR [ebx].rgbDeltaColor[0], mm1
	movq	MMWORD PTR [ebx].rgbDeltaColor[8], mm1
	;
	; ピクセルシェーディング
	;
	call	[ebx].pfnLineSubFunc

	;
	; 座標・色・法線をセットアップ
	;
	cvtsi2ss	xmm0, [ebx].nLineLeft[4]
	cvtsi2ss	xmm1, [ebx].yLinePos
		movq	mm1, MMWORD PTR [ebx].vLineLeftNormal
		movq	mm3, MMWORD PTR [ebx].vLineRightNormal
	movlps		xmm2, QWORD PTR [ebx].vScreenPos
	unpcklps	xmm0, xmm1		; xmm0 <- x, y
		punpcklwd	mm0, mm1
		punpckhwd	mm1, mm1
	subps		xmm0, xmm2
		punpcklwd	mm2, mm3
		punpckhwd	mm3, mm3
	movss		xmm3, [ebx].vScreenPos.z
	movhps		xmm0, QWORD PTR xmmConst1_0_1_0
	shufps		xmm3, xmm3, 00000000B
	divps		xmm0, xmm3	; { x, y, 1, 0 } / vScreenPos.z
		psrad	mm0, 16
		psrad	mm1, 16
		psrad	mm2, 16
		psrad	mm3, 16
		cvtpi2ps	xmm6, mm0
		cvtpi2ps	xmm4, mm1
		cvtpi2ps	xmm7, mm2
		cvtpi2ps	xmm5, mm3
		movlhps		xmm6, xmm4	; xmm6 <- left normal
		movlhps		xmm7, xmm5
		subps		xmm7, xmm6	; xmm7 <- delta normal
	mov	ecx, [ebx].nLineRight[0]
	mov	esi, [ebx].pLineBuf[0]
	mov	edi, [ebx].pLineVectorBuf
	sub	ecx, [ebx].nLineLeft[0]
	mov	eax, [ebx].pLineNormalBuf
	mov	edx, [ebx].pLineTextureBuf
	inc	ecx
	;
	movhlps		xmm1, xmm0	; xmm1 <- { 1, 0 } / vScreenPos.z
		cvtsi2ss	xmm4, ecx
	mov	eax, [ebx].pLineNormalBuf
	movaps		xmm2, xmm0
		shufps		xmm4, xmm4, 0
	addps		xmm2, xmm1
		divps		xmm7, xmm4
	mov	ecx, [ebx].nLineRight[4]
		cvtsi2ss	xmm5, [ebx].nLeftDecimal
	sub	ecx, [ebx].nLineLeft[4]
	movlhps		xmm0, xmm2	; xmm0 <- { x, y } / vScreenPos.z
	movlhps		xmm1, xmm1
		mulss		xmm5, xmmConstDiv10000H
	addps		xmm1, xmm1
	;
	shr	ecx, 1
	movaps	xmm4, xmmMask1_0_1_0
	inc	ecx
	;
	shufps	xmm5, xmm5, 0
	.IF	[ebx].nLineLeft[0] & 1
		subps	xmm6, xmm7
	.ENDIF
	mulps	xmm5, xmm7
	subps	xmm6, xmm5
	;
	ASSUME	esi:PTR E3D_TRANS_LINE_BUF
	ASSUME	edi:PTR E3D_VECTOR4
	ASSUME	eax:PTR E3D_VECTOR4
	;
	pcmpeqd	mm6, mm6
	pcmpeqd	mm7, mm7
	psrld	mm6, 8
	;
	.REPEAT
			movaps		[eax], xmm6
		movlps		xmm3, QWORD PTR [esi].rZValue[0]
			addps		xmm6, xmm7
				movq	mm0, MMWORD PTR [esi].rgbMul
		movaps		xmm2, xmm0
		addps		xmm0, xmm1
				movq	mm1, MMWORD PTR [esi].rgbAdd
		unpcklps	xmm3, xmm3
			movaps		[eax + (SIZEOF E3D_VECTOR4)], xmm6
		mulps		xmm2, xmm3
				pxor	mm0, mm7
				pand	mm1, mm6
			addps		xmm6, xmm7
				pslld	mm0, 24
		andps		xmm3, xmm4
				por	mm0, mm1
		;
		movlps		QWORD PTR [edi].x, xmm2
		movlps		QWORD PTR [edi].z, xmm3
				movq	MMWORD PTR [edx], mm0
		movhps		QWORD PTR [edi + (SIZEOF E3D_VECTOR4)].x, xmm2
		movhps		QWORD PTR [edi + (SIZEOF E3D_VECTOR4)].z, xmm3
		;
		add	esi, (SIZEOF E3D_TRANS_LINE_BUF)
		add	edi, (SIZEOF E3D_VECTOR4) * 2
		add	eax, (SIZEOF E3D_VECTOR4) * 2
		add	edx, (SIZEOF EGL_PALETTE) * 2
		;
		dec	ecx
	.UNTIL	ZERO?
	;
	ASSUME	esi:NOTHING
	ASSUME	edi:NOTHING
	ASSUME	eax:NOTHING
	ASSUME	edx:NOTHING
	;
	; シェーディング実行
	;
	movups	xmm1, [ebx].vTargetPlaneParam
		mov	ecx, [ebx].nLineRight[4]
	xorps	xmm0, xmm0
		sub	ecx, [ebx].nLineLeft[4]
	movups	[ebx].rmvpViewPoint.vViewPosition, xmm0
		add	ecx, 2
	movups	[ebx].rmvpViewPoint.vTargetPlane, xmm1
	;
	mov	eax, [ebx].pMeshPolygonEntry
	mov	edx, [ebx].nMeshPolygonIndex
	mov	[ebx].rmvpViewPoint.pExceptingMesh, eax
	mov	[ebx].rmvpViewPoint.nExceptingMeshIndex, edx
	;
	INVOKE	eglRenderPoly@ShadeVectorsAndRaySSE ,
			ebx, [ebx].pSurfaceAttr,
			[ebx].pLineNormalBuf, [ebx].pLineVectorBuf,
			[ebx].pLineColorBuf, [ebx].pLineTextureBuf, ecx,
			ADDR [ebx].rmvpViewPoint,
			[ebx].rrtpRayParam.dwRayReflectCount
	;
	; シェーディング済みの色をラインバッファに戻す
	;
	mov	ecx, [ebx].nLineRight[4]
	mov	esi, [ebx].pLineColorBuf
	sub	ecx, [ebx].nLineLeft[4]
	mov	edi, [ebx].pLineBuf[0]
	;
	ASSUME	esi:PTR E3D_COLOR
	ASSUME	edi:PTR E3D_TRANS_LINE_BUF
	;
	shr	ecx, 1
	inc	ecx
	;
	.REPEAT
		movq		mm0, MMWORD PTR [esi]
		movq		mm2, MMWORD PTR [esi + (SIZEOF E3D_COLOR)]
		add		esi, (SIZEOF E3D_COLOR) * 2
		movq		mm1, mm0
		punpckldq	mm0, mm2
		punpckhdq	mm1, mm2
		movq		MMWORD PTR [edi].rgbMul, mm0
		movq		MMWORD PTR [edi].rgbAdd, mm1
		add		edi, (SIZEOF E3D_TRANS_LINE_BUF)
		dec		ecx
	.UNTIL	ZERO?

	ASSUME	ebx:NOTHING
	ret

eglRenderPoly@RenderPolygon_LineRayTracingTexture	ENDP


CodeSeg	ENDS

	END
