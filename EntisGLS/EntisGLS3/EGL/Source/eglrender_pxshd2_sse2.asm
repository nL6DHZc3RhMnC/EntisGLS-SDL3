
; ----------------------------------------------------------------------------
;                          Entis Graphic Library
; ----------------------------------------------------------------------------
;    Copyright (c) 2002-2009 Leshade Entis, Entis-soft. Al rights reserved.
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
mmxConst2000x3_0	LABEL	MMWORD
		WORD	3 DUP( 2000H ), 0

ALIGN	10H
xmmConst1_d	REAL8	1.0, 1.0
xmmConstHalf_d	REAL8	2 DUP( 0.5 )
xmmScale16bit_d	REAL8	2 DUP( 65536.0 )

ConstSeg	ENDS


; ----------------------------------------------------------------------------
;	コードセグメント
; ----------------------------------------------------------------------------

CodeSeg	SEGMENT	PARA READONLY FLAT 'CODE'

;
;	テクスチャ（RGB タイリング）シェーディング関数 SSE2 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_LineTTXSSE2	PROC	NEAR32 C

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
	movapd	xmm0, [ebx].vTxLinePos_d
	_movsd	xmm1, [ebx].rTxLineMod_d
	movapd	xmm2, [ebx].vTxDeltaX_d
	_movsd	xmm3, [ebx].vTxxy_d.x
	;
	; ライン左端パラメータ補正
	;
	cvtsi2sd	xmm4, edx
	sub	ecx, eax
	lea	ebp, [ebp + edx * 4]
	shufpd	xmm4, xmm4, 0
	movapd	xmm5, xmm4
	mulpd	xmm4, xmm2
	mulsd	xmm5, xmm3
	addpd	xmm0, xmm4
	addsd	xmm1, xmm5
	;
	; パラメータ・ペアリング
	;
;	movaps	xmm4, xmm0
	movapd	xmm5, xmm1
;	addps	xmm4, xmm2
	addsd	xmm5, xmm3
;	movlhps	xmm0, xmm4
	unpcklpd	xmm1, xmm5
;	addps	xmm2, xmm2
	addpd	xmm3, xmm3
;	movlhps	xmm2, xmm2
	unpcklpd	xmm3, xmm3
	;
	shr	ecx, 1
	ASSERT	<!!CARRY?>, "!(ecx & 01H)"
	inc	ecx
	;
	; テクスチャ座標用パラメータを進める
	; また、第1ピクセルのシェーディングパラメータを準備する
	;
	movapd	xmm4, xmmConst1_d
	divpd	xmm4, xmm1		; xmm4 <- 1 / xmm1
		movapd	xmm6, xmm0
		addpd	xmm0, xmm2
		addpd	xmm1, xmm3
	;
	; ｚ値とテクスチャマッピング用パラメータ計算のための準備を行う
	;
	_movsd	xmm5, [ebx].rTxoxyr_d
	_movsd	xmm7, [ebx].rRcpTxxy_d
	shufpd	xmm5, xmm5, 0
	shufpd	xmm7, xmm7, 0
	;
	.REPEAT
		mulpd	xmm5, xmm4		; xmm5 = z 値
		movhpd	[ebx].qwTemp[0], xmm4
		unpcklpd	xmm4, xmm4
		mulpd	xmm4, xmm6
			movapd	xmm6, xmm0
			addpd	xmm0, xmm2
		mulpd	xmm4, xmm7		; xmm4 = マッピング座標 (0)
			mulpd	xmm6, xmm7
		;
		; テクスチャ画像を取得する
		;
		; テクスチャ座標用パラメータを進める
		; また、第1ピクセルのシェーディングパラメータを準備する
		;
		prefetchnta	[ebp]
			movq	mm4, mm0
			movq	mm5, mm1
		cvtpd2pi	mm6, xmm4
		_movsd	xmm4, [ebx].qwTemp[0]
		unpcklpd	xmm4, xmm4
		mulpd	xmm4, xmm6		; xmm4 = マッピング座標 (1)
			paddsw	mm0, mm2
			paddsw	mm1, mm3
			psraw	mm5, 7
			add	ebp, 8
			movq	MMWORD PTR [ebx].rgbNextColor[8], mm5
		cvtpd2pi	mm7, xmm4
				movapd	xmm4, xmmConst1_d	; xmm4 <- 1 / xmm1
			movq	mm5, MMWORD PTR [ebx].txSizeMask[0]
				divpd	xmm4, xmm1
				movapd	xmm6, xmm0
				addpd	xmm0, xmm2
		packssdw	mm6, mm7
			movq	mm7, MMWORD PTR [ebx].txMulAddr
				addpd	xmm1, xmm3
		pand	mm6, mm5
			cvtpd2ps	xmm5, xmm5
			movq	mm5, MMWORD PTR [ebx].txImageAddr
		pmaddwd	mm6, mm7
			movlps	QWORD PTR [edi].rZValue, xmm5
		paddd	mm6, mm5
		;
		; 第1ピクセルを計算する
		; 第2ピクセルのための準備を行う
		;	mm5 <- 第1ピクセルの結果
		;
		; ｚ値とテクスチャマッピング用パラメータ計算のための準備を行う
		;
				_movsd	xmm5, [ebx].rTxoxyr_d
				_movsd	xmm7, [ebx].rRcpTxxy_d
		movd	eax, mm6
		psrlq	mm6, 32
				shufpd	xmm5, xmm5, 0
		pxor	mm7, mm7
		movd	mm5, DWORD PTR [eax]
				shufpd	xmm7, xmm7, 0
			movd	edx, mm6
		punpcklbw	mm5, mm7
		psllw	mm5, 1
			movq	mm6, MMWORD PTR [ebx].rgbNextColor[8]
		pmulhw	mm5, mm4
			movd	mm4, DWORD PTR [edx]
		paddsw	mm5, mm6
		;
		; 第2ピクセルを計算し、パラメータを進める
		;	mm4 <- 第2ピクセルの結果
		;
			movq	mm6, mm1
		punpcklbw	mm4, mm7
			psraw	mm6, 7
		psllw	mm4, 1
			paddsw	mm1, mm3
		pmulhw	mm4, mm0
			paddsw	mm0, mm2
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

eglRenderPoly@RenderPolygon_LineTTXSSE2	ENDP

;
;	テクスチャ（RGBA タイリング）シェーディング関数 SSE2 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_LineTATXSSE2	PROC	NEAR32 C

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
	movapd	xmm0, [ebx].vTxLinePos_d
	_movsd	xmm1, [ebx].rTxLineMod_d
	movapd	xmm2, [ebx].vTxDeltaX_d
	_movsd	xmm3, [ebx].vTxxy_d.x
	;
	; ライン左端パラメータ補正
	;
	cvtsi2sd	xmm4, edx
	sub	ecx, eax
	lea	ebp, [ebp + edx * 4]
	shufpd	xmm4, xmm4, 0
	movapd	xmm5, xmm4
	mulpd	xmm4, xmm2
	mulsd	xmm5, xmm3
	addpd	xmm0, xmm4
	addsd	xmm1, xmm5
	;
	; パラメータ・ペアリング
	;
;	movaps	xmm4, xmm0
	movapd	xmm5, xmm1
;	addps	xmm4, xmm2
	addsd	xmm5, xmm3
;	movlhps	xmm0, xmm4
	unpcklpd	xmm1, xmm5
;	addps	xmm2, xmm2
	addpd	xmm3, xmm3
;	movlhps	xmm2, xmm2
	unpcklpd	xmm3, xmm3
	;
	shr	ecx, 1
	ASSERT	<!!CARRY?>, "!(ecx & 01H)"
	inc	ecx
	;
	; テクスチャ座標用パラメータを進める
	; また、第1ピクセルのシェーディングパラメータを準備する
	;
	movapd	xmm4, xmmConst1_d
	divpd	xmm4, xmm1		; xmm4 <- 1 / xmm1
		movapd	xmm6, xmm0
		addpd	xmm0, xmm2
		addpd	xmm1, xmm3
	;
	; ｚ値とテクスチャマッピング用パラメータ計算のための準備を行う
	;
	_movsd	xmm5, [ebx].rTxoxyr_d
	_movsd	xmm7, [ebx].rRcpTxxy_d
	shufpd	xmm5, xmm5, 0
	shufpd	xmm7, xmm7, 0
	;
	.REPEAT
		mulpd	xmm5, xmm4		; xmm5 = z 値
		movhpd	[ebx].qwTemp[0], xmm4
		unpcklpd	xmm4, xmm4
			movq	mm2, mm0
			movq	mm3, mm1
		mulpd	xmm4, xmm6
			movapd	xmm6, xmm0
			addpd	xmm0, xmm2
			paddsw	mm0, MMWORD PTR [ebx].rgbDeltaColor[0]
			paddsw	mm1, MMWORD PTR [ebx].rgbDeltaColor[8]
		mulpd	xmm4, xmm7		; xmm4 = マッピング座標 (0)
		prefetcht0	[ebp]
			mulpd	xmm6, xmm7
		add	ebp, 8
		;
		; テクスチャ画像を取得する
		;
		cvtpd2pi	mm6, xmm4
		_movsd	xmm4, [ebx].qwTemp[0]
		unpcklpd	xmm4, xmm4
		mulpd	xmm4, xmm6		; xmm4 = マッピング座標 (1)
			movq	mm5, MMWORD PTR [ebx].txSizeMask[0]
		cvtpd2pi	mm7, xmm4
				movapd	xmm4, xmmConst1_d	; xmm4 <- 1 / xmm1
		packssdw	mm6, mm7
				divpd	xmm4, xmm1
				movapd	xmm6, xmm0
				addpd	xmm0, xmm2
			cvtpd2ps	xmm5, xmm5
			movq	mm7, MMWORD PTR [ebx].txMulAddr
		pand	mm6, mm5
			movq	mm5, MMWORD PTR [ebx].txImageAddr
		pmaddwd	mm6, mm7
				addpd	xmm1, xmm3
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
				_movsd	xmm5, [ebx].rTxoxyr_d
				_movsd	xmm7, [ebx].rRcpTxxy_d
		movd	mm4, DWORD PTR [eax]
			movd	edx, mm6
		punpcklbw	mm4, mm7
		psllw	mm4, 1
			pshufw	mm5, mm4, 11111111B
		pmulhw	mm4, mm2
			pmulhw	mm3, mm5
			movd	mm2, DWORD PTR [edx]
				shufpd	xmm5, xmm5, 0
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
				shufpd	xmm7, xmm7, 0
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

eglRenderPoly@RenderPolygon_LineTATXSSE2	ENDP

;
;	テクスチャ（RGB＋発光 タイリング）シェーディング関数 SSE2 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_LineTLTXSSE2	PROC	NEAR32 C

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
	movapd	xmm0, [ebx].vTxLinePos_d
	_movsd	xmm1, [ebx].rTxLineMod_d
	movapd	xmm2, [ebx].vTxDeltaX_d
	_movsd	xmm3, [ebx].vTxxy_d.x
	;
	; ライン左端パラメータ補正
	;
	cvtsi2sd	xmm4, edx
	sub	ecx, eax
	lea	ebp, [ebp + edx * 4]
	shufpd	xmm4, xmm4, 0
	movapd	xmm5, xmm4
	mulpd	xmm4, xmm2
	mulsd	xmm5, xmm3
	addpd	xmm0, xmm4
	addsd	xmm1, xmm5
	;
	; パラメータ・ペアリング
	;
;	movaps	xmm4, xmm0
	movapd	xmm5, xmm1
;	addps	xmm4, xmm2
	addsd	xmm5, xmm3
;	movlhps	xmm0, xmm4
	unpcklpd	xmm1, xmm5
;	addps	xmm2, xmm2
	addpd	xmm3, xmm3
;	movlhps	xmm2, xmm2
	unpcklpd	xmm3, xmm3
	;
	shr	ecx, 1
	ASSERT	<!!CARRY?>, "!(ecx & 01H)"
	inc	ecx
	;
	; テクスチャ座標用パラメータを進める
	; また、第1ピクセルのシェーディングパラメータを準備する
	;
	movapd	xmm4, xmmConst1_d
	divpd	xmm4, xmm1		; xmm4 <- 1 / xmm1
		movapd	xmm6, xmm0
		addpd	xmm0, xmm2
		addpd	xmm1, xmm3
	;
	; ｚ値とテクスチャマッピング用パラメータ計算のための準備を行う
	;
	_movsd	xmm5, [ebx].rTxoxyr_d
	_movsd	xmm7, [ebx].rRcpTxxy_d
	shufpd	xmm5, xmm5, 0
	shufpd	xmm7, xmm7, 0
	;
	.REPEAT
		mulpd	xmm5, xmm4		; xmm5 = z 値
		movhpd	[ebx].qwTemp[0], xmm4
		unpcklpd	xmm4, xmm4
			movq	mm2, mm0
			movq	mm3, mm1
		mulpd	xmm4, xmm6
			movapd	xmm6, xmm0
			addpd	xmm0, xmm2
			paddsw	mm0, MMWORD PTR [ebx].rgbDeltaColor[0]
			paddsw	mm1, MMWORD PTR [ebx].rgbDeltaColor[8]
		mulpd	xmm4, xmm7		; xmm4 = マッピング座標 (0)
			psraw	mm3, 7
		prefetcht0	[ebp]
			mulpd	xmm6, xmm7
		add	ebp, 8
		;
		; テクスチャ画像を取得する
		;
		cvtpd2pi	mm6, xmm4
		_movsd	xmm4, [ebx].qwTemp[0]
		unpcklpd	xmm4, xmm4
		mulpd	xmm4, xmm6		; xmm4 = マッピング座標 (1)
			movq	mm4, MMWORD PTR [ebx].txSizeMask[0]
			movq	mm5, MMWORD PTR [ebx].lmSizeMask[0]
		cvtpd2pi	mm7, xmm4
				movapd	xmm4, xmmConst1_d	; xmm4 <- 1 / xmm1
		packssdw	mm6, mm7
				divpd	xmm4, xmm1
				movapd	xmm6, xmm0
				addpd	xmm0, xmm2
			cvtpd2ps	xmm5, xmm5
			movq	mm7, MMWORD PTR [ebx].txMulAddr
		pand	mm4, mm6
		pand	mm5, mm6
				addpd	xmm1, xmm3
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
				_movsd	xmm5, [ebx].rTxoxyr_d
		movd	eax, mm4
		movd	edx, mm5
				_movsd	xmm7, [ebx].rRcpTxxy_d
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
				shufpd	xmm5, xmm5, 0
		pmulhw	mm6, mm2
			movq	mm2, mm0
			paddsw	mm0, MMWORD PTR [ebx].rgbDeltaColor[0]
		pmullw	mm7, mm5
			movd	mm5, DWORD PTR [eax]
				shufpd	xmm7, xmm7, 0
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

eglRenderPoly@RenderPolygon_LineTLTXSSE2	ENDP

;
;	テクスチャ（RGB 補完 タイリング）シェーディング関数 SSE2 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_LineSTXSSE2	PROC	NEAR32 C

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
	movapd	xmm0, [ebx].vTxLinePos_d
	_movsd	xmm1, [ebx].rTxLineMod_d
	movapd	xmm2, [ebx].vTxDeltaX_d
	_movsd	xmm3, [ebx].vTxxy_d.x
	;
	; ライン左端パラメータ補正
	;
	cvtsi2sd	xmm4, edx
	sub	ecx, eax
	lea	ebp, [ebp + edx * 4]
	shufpd	xmm4, xmm4, 0
	movapd	xmm5, xmm4
	mulpd	xmm4, xmm2
	mulsd	xmm5, xmm3
	addpd	xmm0, xmm4
	addsd	xmm1, xmm5
	;
	; パラメータ・ペアリング
	;
;	movaps	xmm4, xmm0
	movapd	xmm5, xmm1
;	addps	xmm4, xmm2
	addsd	xmm5, xmm3
;	movlhps	xmm0, xmm4
	unpcklpd	xmm1, xmm5
;	addps	xmm2, xmm2
	addpd	xmm3, xmm3
;	movlhps	xmm2, xmm2
	unpcklpd	xmm3, xmm3
	movapd	xmm7, xmmScale16bit_d
	mulpd	xmm0, xmm7
	mulpd	xmm2, xmm7
	;
	shr	ecx, 1
	ASSERT	<!!CARRY?>, "!(ecx & 01H)"
	inc	ecx
	mov	[ebx].dib.nLeftWidth, ecx
	;
	; テクスチャ座標用パラメータを進める
	; また、第1ピクセルのシェーディングパラメータを準備する
	;
	movapd	xmm4, xmmConst1_d
		movapd	xmm6, xmm0
	divpd	xmm4, xmm1		; xmm4 <- 1 / xmm1
		addpd	xmm0, xmm2
		movq	MMWORD PTR [ebx].rgbNextColor[0], mm0
		addpd	xmm1, xmm3
		movq	MMWORD PTR [ebx].rgbNextColor[8], mm1
	;
	; ｚ値とテクスチャマッピング用パラメータ計算のための準備を行う
	;
	_movsd	xmm5, [ebx].rTxoxyr_d
	_movsd	xmm7, [ebx].rRcpTxxy_d
	shufpd	xmm5, xmm5, 0
	shufpd	xmm7, xmm7, 0
	mulpd	xmm5, xmm4		; xmm5 = z 値
	movhpd	[ebx].qwTemp[0], xmm4
	unpcklpd	xmm4, xmm4
	mulpd	xmm4, xmm6
		movapd	xmm6, xmm0
		addpd	xmm0, xmm2
	mulpd	xmm4, xmm7		; xmm4 = マッピング座標 (0)
		mulpd	xmm6, xmm7
	;
	mov	ecx, [ebx].dib.nLeftWidth
	.REPEAT
		mov	[ebx].dib.nLeftWidth, ecx
		;
		; 第1ピクセルのマッピング座標を取得する
		;
		cvtpd2pi	mm6, xmm4
		_movsd	xmm4, [ebx].qwTemp[0]
			pshufw	mm2, mm6, 00000000B
			pshufw	mm3, mm6, 10101010B
		psrld	mm6, 16
			psrlw	mm2, 1
		movd	eax, mm6
		psrlq	mm6, 32
			psrlw	mm3, 1
		mov	esi, [ebx].pTxLineAddr
			cvtpd2ps	xmm5, xmm5
		movd	ecx, mm6
		lea	edx, [eax + 1]
		and	cx, [ebx].txSizeMask[2]
		and	eax, DWORD PTR [ebx].txSizeMask[0]
			movzx	ecx, cx
		and	edx, DWORD PTR [ebx].txSizeMask[0]
			and	eax, 0FFFFH
			and	edx, 0FFFFH
				unpcklpd	xmm4, xmm4
		mov	esi, DWORD PTR [esi + ecx * 4]
		;
		; 第1ピクセルの値を補完する
		;
					;
					; テクスチャ座標用パラメータを進める
					; また、第1ピクセルのシェーディングパラメータを準備する
					;
			inc	ecx
			movlps	QWORD PTR [edi].rZValue, xmm5
		pxor	mm7, mm7
		movd	mm4, DWORD PTR [esi + eax * 4]
			and	cx, [ebx].txSizeMask[2]
		movd	mm5, DWORD PTR [esi + edx * 4]
			mov	esi, [ebx].pTxLineAddr
			movzx	ecx, cx
				mulpd	xmm4, xmm6	; xmm4 = マッピング座標 (1)
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
				cvtpd2pi	mm4, xmm4
			pmulhw	mm1, mm2
					movapd	xmm4, xmmConst1_d	; xmm4 <- 1 / xmm1
					movapd	xmm6, xmm0
				pshufw	mm2, mm4, 00000000B
				pshufw	mm6, mm4, 10101010B
			paddsw	mm1, mm0
					divpd	xmm4, xmm1
		;
			psrld	mm4, 16
		psubsw	mm1, mm5
					addpd	xmm0, xmm2
			movd	eax, mm4
		psllw	mm1, 1
			psrlq	mm4, 32
		pmulhw	mm1, mm3
					addpd	xmm1, xmm3
			mov	esi, [ebx].pTxLineAddr
				psrlw	mm2, 1
				psrlw	mm6, 1
		paddsw	mm1, mm5
					add	ebp, 8
		;
		; 第2ピクセルの値を補完する
		;
					;
					; ｚ値とテクスチャマッピング用パラメータ計算のための準備を行う
					;
					_movsd	xmm5, [ebx].rTxoxyr_d
					_movsd	xmm7, [ebx].rRcpTxxy_d
		movd	ecx, mm4
		lea	edx, [eax + 1]
		and	cx, [ebx].txSizeMask[2]
		and	eax, DWORD PTR [ebx].txSizeMask[0]
					shufpd	xmm5, xmm5, 0
			movzx	ecx, cx
		and	edx, DWORD PTR [ebx].txSizeMask[0]
			and	eax, 0FFFFH
					shufpd	xmm7, xmm7, 0
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
		psubsw	mm3, mm5
			psllw	mm2, 1
		psllw	mm3, 1
			pmulhw	mm2, mm0	; 第1ピクセル
					mulpd	xmm5, xmm4	; xmm5 = z 値
					movhpd	[ebx].qwTemp[0], xmm4
		pmulhw	mm3, mm6
					unpcklpd	xmm4, xmm4
			movq	mm4, mm1
			paddsw	mm0, MMWORD PTR [ebx].rgbDeltaColor[0]
			psraw	mm4, 7
					mulpd	xmm4, xmm6
						movapd	xmm6, xmm0
		paddsw	mm3, mm5
			paddsw	mm1, MMWORD PTR [ebx].rgbDeltaColor[8]
		psllw	mm3, 1
						addpd	xmm0, xmm2
			paddsw	mm2, mm4
		;
		pmulhw	mm3, mm0		; 第2ピクセル
			movq	mm5, mm1
			paddsw	mm0, MMWORD PTR [ebx].rgbDeltaColor[0]
			psraw	mm5, 7
					mulpd	xmm4, xmm7	; xmm4 = マッピング座標'(0)
		paddsw	mm3, mm5
						mulpd	xmm6, xmm7
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

eglRenderPoly@RenderPolygon_LineSTXSSE2	ENDP

;
;	テクスチャ（RGBA 補完 タイリング）シェーディング関数 SSE2 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_LineSTAXSSE2	PROC	NEAR32 C

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
	movapd	xmm0, [ebx].vTxLinePos_d
	_movsd	xmm1, [ebx].rTxLineMod_d
	movapd	xmm2, [ebx].vTxDeltaX_d
	_movsd	xmm3, [ebx].vTxxy_d.x
	;
	; ライン左端パラメータ補正
	;
	cvtsi2sd	xmm4, edx
	sub	ecx, eax
	lea	ebp, [ebp + edx * 4]
	shufpd	xmm4, xmm4, 0
	movapd	xmm5, xmm4
	mulpd	xmm4, xmm2
	mulsd	xmm5, xmm3
	addpd	xmm0, xmm4
	addsd	xmm1, xmm5
	;
	; パラメータ・ペアリング
	;
;	movaps	xmm4, xmm0
	movapd	xmm5, xmm1
;	addps	xmm4, xmm2
	addsd	xmm5, xmm3
;	movlhps	xmm0, xmm4
	unpcklpd	xmm1, xmm5
;	addps	xmm2, xmm2
	addpd	xmm3, xmm3
;	movlhps	xmm2, xmm2
	unpcklpd	xmm3, xmm3
	movapd	xmm7, xmmScale16bit_d
	mulpd	xmm0, xmm7
	mulpd	xmm2, xmm7
	;
	shr	ecx, 1
	ASSERT	<!!CARRY?>, "!(ecx & 01H)"
	inc	ecx
;	mov	[ebx].dib.nLeftWidth, ecx
	;
	; テクスチャ座標用パラメータを進める
	; また、第1ピクセルのシェーディングパラメータを準備する
	;
	movapd	xmm4, xmmConst1_d
		movapd	xmm6, xmm0
	divpd	xmm4, xmm1		; xmm4 <- 1 / xmm1
		addpd	xmm0, xmm2
		movq	MMWORD PTR [ebx].rgbNextColor[0], mm0
		addpd	xmm1, xmm3
		movq	MMWORD PTR [ebx].rgbNextColor[8], mm1
	;
	; ｚ値とテクスチャマッピング用パラメータ計算のための準備を行う
	;
	_movsd	xmm5, [ebx].rTxoxyr_d
	_movsd	xmm7, [ebx].rRcpTxxy_d
	shufpd	xmm5, xmm5, 0
	shufpd	xmm7, xmm7, 0
	;
	mulpd	xmm5, xmm4		; xmm5 = z 値
	movhpd	[ebx].qwTemp[0], xmm4
	unpcklpd	xmm4, xmm4
	mulpd	xmm4, xmm6
		movapd	xmm6, xmm0
		addpd	xmm0, xmm2
	mulpd	xmm4, xmm7		; xmm4 = マッピング座標 (0)
		mulpd	xmm6, xmm7
	;
;	mov	ecx, [ebx].dib.nLeftWidth
	.REPEAT
		;
		; 第1ピクセルのマッピング座標を取得する
		;
		mov	[ebx].dib.nLeftWidth, ecx
		cvtpd2pi	mm6, xmm4
				prefetcht0	[ebp]
		_movsd	xmm4, [ebx].qwTemp[0]
				add	ebp, 8
			pshufw	mm2, mm6, 00000000B
			pshufw	mm3, mm6, 10101010B
		psrld	mm6, 16
			psrlw	mm2, 1
		movd	eax, mm6
		psrlq	mm6, 32
			psrlw	mm3, 1
		mov	esi, [ebx].pTxLineAddr
			cvtpd2ps	xmm5, xmm5
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
				unpcklpd	xmm4, xmm4
			movlps	QWORD PTR [edi].rZValue, xmm5
		pxor	mm7, mm7
		movd	mm4, DWORD PTR [esi + eax * 4]
			and	cx, [ebx].txSizeMask[2]
		movd	mm5, DWORD PTR [esi + edx * 4]
				mulpd	xmm4, xmm6	; xmm4 = マッピング座標 (1)
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
				cvtpd2pi	mm4, xmm4
			pmulhw	mm1, mm2
					movapd	xmm4, xmmConst1_d	; xmm4 <- 1 / xmm1
					movapd	xmm6, xmm0
				pshufw	mm2, mm4, 00000000B
				pshufw	mm6, mm4, 10101010B
			paddsw	mm1, mm0
					divpd	xmm4, xmm1
		;
			psrld	mm4, 16
		psubsw	mm1, mm5
					addpd	xmm0, xmm2
			movd	eax, mm4
		psllw	mm1, 1
			psrlq	mm4, 32
		pmulhw	mm1, mm3
			mov	esi, [ebx].pTxLineAddr
					addpd	xmm1, xmm3
				psrlw	mm2, 1
				psrlw	mm6, 1
		paddsw	mm1, mm5
		;
		; 第2ピクセルの値を補完する
		;
					;
					; ｚ値とテクスチャマッピング用パラメータ計算のための準備を行う
					;
		movd	ecx, mm4
					_movsd	xmm5, [ebx].rTxoxyr_d
					_movsd	xmm7, [ebx].rRcpTxxy_d
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
					shufpd	xmm5, xmm5, 0
		inc	ecx
		movd	mm4, DWORD PTR [esi + eax * 4]
			and	cx, [ebx].txSizeMask[2]
		movd	mm5, DWORD PTR [esi + edx * 4]
			mov	esi, [ebx].pTxLineAddr
			movzx	ecx, cx
					shufpd	xmm7, xmm7, 0
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
					mulpd	xmm5, xmm4	; xmm5 = z 値
					movhpd	[ebx].qwTemp[0], xmm4
		pmulhw	mm3, mm6
					unpcklpd	xmm4, xmm4
			movq	mm4, mm1
			paddsw	mm0, MMWORD PTR [ebx].rgbDeltaColor[0]
			pmulhw	mm4, mm7
			psrlw	mm7, 1
					mulpd	xmm4, xmm6
						movapd	xmm6, xmm0
		paddsw	mm3, mm5
			pxor	mm7, mmxWord3XorFFMask
			paddsw	mm1, MMWORD PTR [ebx].rgbDeltaColor[8]
		psllw	mm3, 1
						addpd	xmm0, xmm2
			paddsw	mm2, mm4
		pshufw	mm6, mm3, 11111111B
		;
			movq	mm5, mm1
		pmulhw	mm3, mm0		; 第2ピクセル
			pmulhw	mm5, mm6
			psrlw	mm6, 1
			paddsw	mm0, MMWORD PTR [ebx].rgbDeltaColor[0]
					mulpd	xmm4, xmm7	; xmm4 = マッピング座標'(0)
			pxor	mm6, mmxWord3XorFFMask
		paddsw	mm3, mm5
			paddsw	mm1, MMWORD PTR [ebx].rgbDeltaColor[8]
		;
		; 結果を書き出す
		;
		packuswb	mm2, mm3
		packuswb	mm7, mm6
						mulpd	xmm6, xmm7
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

eglRenderPoly@RenderPolygon_LineSTAXSSE2	ENDP


;
;	テクスチャ（RGB＋発光補完）シェーディング関数 SSE2 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_LineSTLXSSE2	PROC	NEAR32 C

	call	eglRenderPoly@RenderPolygon_LineTLTXSSE2
	ret

eglRenderPoly@RenderPolygon_LineSTLXSSE2	ENDP

CodeSeg	ENDS

	END
