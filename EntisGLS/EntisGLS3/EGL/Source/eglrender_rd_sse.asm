
; ----------------------------------------------------------------------------
;                          Entis Graphic Library
; ----------------------------------------------------------------------------
;   Copyright (c) 2002-2012 Leshade Entis, Entis-soft. Al rights reserved.
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
rConst1		REAL4	4 DUP( 1.0 )
rConst1_pd	REAL8	2 DUP( 1.0 )
rConst512	REAL4	4 DUP( 512.0 )
rConst65536	REAL4	4 DUP( 65536.0 )
rConstHalfPI	REAL4	4 DUP( 1.57079632679489661923132169164 )
;rConstLittle	DWORD	4 DUP( 33800000H )	; = 2^-24
rConstLittle	REAL4	4 DUP( 0.0000000000001 )
;rConstSinLittle	DWORD	4 DUP( 3B800000H )	; = 1/256 ≒ sin(0.23[deg])
rConstSinLittle	REAL4	4 DUP( 0.0000000000001 )
xmmMaskLS1	DWORD	80000000H, 0, 0, 0
xmmMaskSign	DWORD	4 DUP( 80000000H )
xmmMaskSign_pd	QWORD	2 DUP( 8000000000000000H )
xmmMaskAbs	DWORD	4 DUP( 7FFFFFFFH )
mmxFEFF		WORD	4 DUP( 0FEFFH )
mmxConst2000x3_0	LABEL	MMWORD
		WORD	3 DUP( 2000H ), 0

ConstSeg	ENDS


; ----------------------------------------------------------------------------
;	コードセグメント
; ----------------------------------------------------------------------------

CodeSeg	SEGMENT	PARA READONLY FLAT 'CODE'

;
;	無限平面レンダリング SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_PlaneSSE	PROC	NEAR32 C USES ebx esi edi,
	hRenderPoly:HEGL_RENDER_POLYGON

	mov	ebx, hRenderPoly
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF

	;
	;	リージョン情報セットアップ
	; --------------------------------------------------------------------
	mov	esi, [ebx].dib.pRegion
	ASSUME	esi:PTR E3D_POLYGON_REGION
	mov	eax, [esi].nTopLine
	mov	edx, eax
	mov	[ebx].yLinePos, eax
	imul	eax, [ebx].dib.dstimg.dwBytesPerLine
	imul	edx, [ebx].dib.zbuf.dwBytesPerLine
	add	eax, [ebx].dib.dstimg.ptrImageArray
	add	edx, [ebx].dib.zbuf.ptrImageArray
	mov	[ebx].dib.ptrDstLine, eax
	mov	[ebx].dib.ptrZBufLine, edx
	;
	mov	eax, [esi].nBottomLine
	sub	eax, [esi].nTopLine
	inc	eax
	mov	[ebx].dib.nLeftHeight, eax
	;
	lea	esi, [esi].plrLineRgn[0]
	mov	[ebx].dib.ptrRegionLine, esi
	;
	mov	eax, [ebx].vTxBasePos.x
	mov	edx, [ebx].vTxBasePos.y
	mov	ecx, [ebx].rTxtxy
	mov	[ebx].vTxLinePos.x, eax
	mov	[ebx].vTxLinePos.y, edx
	mov	[ebx].rTxLineMod, ecx

	;
	;	描画ループ
	; --------------------------------------------------------------------
	mov	ecx, [ebx].dib.nLeftHeight
	.REPEAT
		mov	[ebx].dib.nLeftHeight, ecx
		mov	[ebx].dib.ptrRegionLine, esi
		;
		;	ライン情報をセットアップ
		; ------------------------------------------------------------
		ASSUME	esi:PTR E3D_POLY_LINE_REGION
		mov	eax, [esi].nLeft
		mov	edx, [esi].dwReserved1
			movq	mm0, MMWORD PTR [esi].vLeft
			movq	mm1, MMWORD PTR [esi].vRight
		mov	[ebx].nLineLeft[0], eax
		mov	[ebx].nLeftDecimal, edx
			movq	MMWORD PTR [ebx].vLineLeftNormal, mm0
			movq	MMWORD PTR [ebx].vLineRightNormal, mm1
		and	eax, NOT 01H
		mov	[ebx].nLineLeft[4], eax
		mov	eax, [esi].nRight
		mov	edx, [esi].dwReserved2
		mov	[ebx].nLineRight[0], eax
		add	eax, 01H
		mov	[ebx].nRightDecimal, edx
		and	eax, NOT 01H
		add	eax, 2
		mov	[ebx].nLineRight[4], eax
		;
		mov	ecx, [ebx].nLineRight[0]
		sub	ecx, [ebx].nLineLeft[0]
		.IF	ZERO?
			pxor	mm4, mm4
			movd	mm0, [esi].rgbaLeft.rgbMul.dwPixelCode
			movd	mm1, [esi].rgbaLeft.rgbAdd.dwPixelCode
			movd	mm2, [esi].rgbaRight.rgbMul.dwPixelCode
			movd	mm3, [esi].rgbaRight.rgbAdd.dwPixelCode
			punpcklbw	mm0, mm4
			punpcklbw	mm1, mm4
			punpcklbw	mm2, mm4
			punpcklbw	mm3, mm4
			psllw	mm0, 7
			psllw	mm1, 7
			psllw	mm2, 7
			psllw	mm3, 7
			psubw	mm2, mm0
			psubw	mm3, mm1
			psubsw	mm0, mmxConst2000x3_0
			psubsw	mm1, mmxConst2000x3_0
			movq	MMWORD PTR [ebx].rgbNextColor[0], mm0
			movq	MMWORD PTR [ebx].rgbNextColor[8], mm1
			movq	MMWORD PTR [ebx].rgbDeltaColor[0], mm2
			movq	MMWORD PTR [ebx].rgbDeltaColor[8], mm3
		.ELSE
			mov	eax, 8000H
			inc	ecx
			cvtsi2ss	xmm0, eax
			cvtsi2ss	xmm1, ecx
			pxor	mm4, mm4
			divss	xmm0, xmm1
			movd	mm0, [esi].rgbaLeft.rgbMul.dwPixelCode
			movd	mm1, [esi].rgbaLeft.rgbAdd.dwPixelCode
			movd	mm2, [esi].rgbaRight.rgbMul.dwPixelCode
			movd	mm3, [esi].rgbaRight.rgbAdd.dwPixelCode
			punpcklbw	mm0, mm4
			punpcklbw	mm1, mm4
			punpcklbw	mm2, mm4
			punpcklbw	mm3, mm4
			psubw	mm2, mm0
			psllw	mm0, 7
			psubw	mm3, mm1
			psllw	mm1, 7
			psubsw	mm0, mmxConst2000x3_0
			psubsw	mm1, mmxConst2000x3_0
			movq	MMWORD PTR [ebx].rgbNextColor[0], mm0
			cvtps2pi	mm7, xmm0
			movq	MMWORD PTR [ebx].rgbNextColor[8], mm1
			pshufw	mm7, mm7, 0
			movq	mm0, mm2
			movq	mm1, mm3
			pmulhw	mm2, mm7
			pmullw	mm0, mm7
			pmulhw	mm3, mm7
			pmullw	mm1, mm7
			psllw	mm2, 8
			psrlw	mm0, 8
			psllw	mm3, 8
			psrlw	mm1, 8
			por	mm2, mm0
			por	mm3, mm1
			movq	MMWORD PTR [ebx].rgbDeltaColor[0], mm2
			movq	MMWORD PTR [ebx].rgbDeltaColor[8], mm3
		.ENDIF
		ASSUME	esi:NOTHING
		;
		;	一時ｚバッファ初期化
		; ------------------------------------------------------------
;		mov	eax, [ebx].pTempZBuffer[0]
;		.IF	[ebx].dib.zbuf.ptrImageArray == eax
;			mov	edi, [ebx].dib.ptrZBufLine
;			mov	eax, [ebx].nLineLeft[4]
;			mov	ecx, [ebx].nLineRight[4]
;			lea	edi, [edi + eax * 4]
;			sub	ecx, eax
;			mov	eax, 7F000000H
;			shr	ecx, 1
;			movd	mm0, eax
;			inc	ecx
;			punpckldq	mm0, mm0
;			.REPEAT
;				movq	MMWORD PTR [edi], mm0
;				add	edi, 8
;				dec	ecx
;			.UNTIL	ZERO?
;		.ENDIF
		;
		;	ラインバッファにレンダリング
		; ------------------------------------------------------------
		call	[ebx].pfnLineFunc
		;
		;	擬似フォッグ適用
		; ------------------------------------------------------------
		mov	eax, [ebx].rRenderFogDeepness
		mov	ecx, [ebx].nLineRight[4]
		mov	esi, [ebx].pLineBuf[0]
		movss	xmm7, [ebx].rRenderFogDeepness
		sub	ecx, [ebx].nLineLeft[4]
		shufps	xmm7, xmm7, 0
		pxor	mm7, mm7
		shr	ecx, 1
		test	eax, eax
		.IF	!ZERO?
		inc	ecx
		xorps	xmm0, xmm0
		ASSUME	esi:PTR E3D_TRANS_LINE_BUF
		@LINE_STEP = (SIZEOF E3D_TRANS_LINE_BUF)
		sub	ecx, 2
		.WHILE	!SIGN?
				movlps	xmm0, QWORD PTR [esi].rZValue[0]
			movq	mm0, MMWORD PTR [esi].rgbAdd[0]
				movhps	xmm0, QWORD PTR [esi + @LINE_STEP].rZValue[0]
			movq	mm2, MMWORD PTR [ebx].rgbRenderFogColor
				mulps	xmm0, xmm7
			movq	mm1, mm0
			punpcklbw	mm0, mm7
			movq	mm3, mm2
				cvtps2pi	mm4, xmm0
				movhlps	xmm0, xmm0
			punpckhbw	mm1, mm7
				cvtps2pi	mm5, xmm0
			movq	mm6, MMWORD PTR mmxFEFF
				packssdw	mm4, mm5
			psubsw	mm2, mm0
				paddusw	mm4, mm6
			psubsw	mm3, mm1
				psubusw	mm4, mm6
			;
			psrlw	mm4, 1
			movq	mm5, mm4
			movq	mm6, mm4
			pshufw	mm4, mm4, 00000000B
			pshufw	mm5, mm5, 01010101B
			pmullw	mm2, mm4
				movq	mm4, MMWORD PTR [esi + @LINE_STEP].rgbAdd[0]
			pmullw	mm3, mm5
				movq	mm5, mm4
				punpcklbw	mm4, mm7
				punpckhbw	mm5, mm7
			;
			psraw	mm2, 7
			psraw	mm3, 7
			paddsw	mm0, mm2
				movq	mm2, MMWORD PTR [ebx].rgbRenderFogColor
			paddsw	mm1, mm3
				movq	mm3, mm2
				psubsw	mm2, mm4
			packuswb	mm0, mm1
				psubsw	mm3, mm5
			movq	MMWORD PTR [esi].rgbAdd[0], mm0
			;
			pshufw	mm0, mm6, 10101010B
			pshufw	mm6, mm6, 11111111B
			pmullw	mm2, mm0
				add	esi, @LINE_STEP * 2
			pmullw	mm3, mm6
				sub	ecx, 2
			psraw	mm2, 7
			psraw	mm3, 7
			paddsw	mm2, mm4
			paddsw	mm3, mm5
			packuswb	mm2, mm3
			movq	MMWORD PTR [esi - @LINE_STEP].rgbAdd[0], mm2
		.ENDW
		add	ecx, 2
		.IF	!ZERO?
;		.REPEAT
			movlps	xmm0, QWORD PTR [esi].rZValue[0]
			movq	mm0, MMWORD PTR [esi].rgbAdd[0]
			mulps	xmm0, xmm7
			movq	mm1, mm0
			punpcklbw	mm0, mm7
			movq	mm2, MMWORD PTR [ebx].rgbRenderFogColor
			punpckhbw	mm1, mm7
			cvtps2pi	mm4, xmm0
			movq	mm5, MMWORD PTR mmxFEFF
			packssdw	mm4, mm4
			paddusw	mm4, mm5
			movq	mm3, mm2
			psubusw	mm4, mm5
			psubsw	mm2, mm0
			psrlw	mm4, 1
			psubsw	mm3, mm1
			pshufw	mm5, mm4, 01010101B
			pshufw	mm4, mm4, 0
			pmullw	mm2, mm4
			pmullw	mm3, mm5
			psraw	mm2, 7
			psraw	mm3, 7
			paddsw	mm0, mm2
			paddsw	mm1, mm3
			add	esi, @LINE_STEP
			packuswb	mm0, mm1
;			dec	ecx
			movq	MMWORD PTR [esi - @LINE_STEP].rgbAdd[0], mm0
;		.UNTIL	ZERO?
		.ENDIF
		ASSUME	esi:NOTHING
		.ENDIF
		;
		;	ランバッファを出力
		; ------------------------------------------------------------
		call	[ebx].pfnStoreFunc
		;
		;	次の行へ移動
		; ------------------------------------------------------------
		movlps	xmm0, [ebx].vTxLinePos
		movlps	xmm1, [ebx].vTxDeltaY
		movss	xmm2, [ebx].rTxLineMod
		mov	eax, [ebx].dib.ptrDstLine
		movss	xmm3, [ebx].vTxxy.y
		mov	edx, [ebx].dib.ptrZBufLine
		addps	xmm0, xmm1
		add	eax, [ebx].dib.dstimg.dwBytesPerLine
		add	edx, [ebx].dib.zbuf.dwBytesPerLine
		addss	xmm2, xmm3
		mov	[ebx].dib.ptrDstLine, eax
		mov	[ebx].dib.ptrZBufLine, edx
		mov	esi, [ebx].dib.ptrRegionLine
		mov	ecx, [ebx].dib.nLeftHeight
		movlps	[ebx].vTxLinePos, xmm0
		add	esi, (SIZEOF E3D_POLY_LINE_REGION)
		movss	[ebx].rTxLineMod, xmm2
		inc	[ebx].yLinePos
		dec	ecx
	.UNTIL	ZERO?

	ASSUME	ebx:NOTHING
	emms
	sfence
	xor	eax, eax
	ret

eglRenderPoly@RenderPolygon_PlaneSSE	ENDP


;
;	テクスチャマッピングポリゴンレンダリング SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_TextureSSE	PROC	NEAR32 C USES ebx esi edi,
	hRenderPoly:HEGL_RENDER_POLYGON

	mov	ebx, hRenderPoly
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF

	;
	;	リージョン情報セットアップ
	; --------------------------------------------------------------------
	mov	esi, [ebx].dib.pRegion
	ASSUME	esi:PTR E3D_POLYGON_REGION
	mov	eax, [esi].nTopLine
	mov	edx, eax
	mov	[ebx].yLinePos, eax
	imul	eax, [ebx].dib.dstimg.dwBytesPerLine
	imul	edx, [ebx].dib.zbuf.dwBytesPerLine
	add	eax, [ebx].dib.dstimg.ptrImageArray
	add	edx, [ebx].dib.zbuf.ptrImageArray
	mov	[ebx].dib.ptrDstLine, eax
	mov	[ebx].dib.ptrZBufLine, edx
	;
	mov	eax, [esi].nBottomLine
	sub	eax, [esi].nTopLine
	inc	eax
	mov	[ebx].dib.nLeftHeight, eax
	;
	lea	esi, [esi].plrLineRgn[0]
	mov	[ebx].dib.ptrRegionLine, esi
	;
	.IF	[ebx].dwFunctionFlags & E3D_FLAG_ENABLE_SSE2
		movapd	xmm0, [ebx].vTxBasePos_d
		_movsd	xmm1, [ebx].rTxtxy_d
		movapd	[ebx].vTxLinePos_d, xmm0
		_movsd	[ebx].rTxLineMod_d, xmm1
		cvtpd2ps	xmm0, xmm0
		cvtsd2ss	xmm1, xmm1
		movlps	QWORD PTR [ebx].vTxLinePos, xmm0
		movss	[ebx].rTxLineMod, xmm1
	.ELSE
		mov	eax, [ebx].vTxBasePos.x
		mov	edx, [ebx].vTxBasePos.y
		mov	ecx, [ebx].rTxtxy
		mov	[ebx].vTxLinePos.x, eax
		mov	[ebx].vTxLinePos.y, edx
		mov	[ebx].rTxLineMod, ecx
	.ENDIF

	;
	;	描画ループ
	; --------------------------------------------------------------------
	mov	ecx, [ebx].dib.nLeftHeight
	.REPEAT
		mov	[ebx].dib.nLeftHeight, ecx
		mov	[ebx].dib.ptrRegionLine, esi
		;
		;	ライン情報をセットアップ
		; ------------------------------------------------------------
		ASSUME	esi:PTR E3D_POLY_LINE_REGION
		mov	eax, [esi].nLeft
		movzx	edx, WORD PTR [esi].dwReserved1
			movq	mm0, MMWORD PTR [esi].vLeft
			movq	mm1, MMWORD PTR [esi].vRight
		mov	[ebx].nLineLeft[0], eax
		mov	[ebx].nLeftDecimal, edx
			movq	MMWORD PTR [ebx].vLineLeftNormal, mm0
			movq	MMWORD PTR [ebx].vLineRightNormal, mm1
		and	eax, NOT 01H
		mov	[ebx].nLineLeft[4], eax
		mov	eax, [esi].nRight
		movzx	edx, WORD PTR [esi].dwReserved2
		mov	[ebx].nLineRight[0], eax
		add	eax, 01H
		mov	[ebx].nRightDecimal, edx
		and	eax, NOT 01H
		add	eax, 2
		mov	[ebx].nLineRight[4], eax
		;
		mov	eax, [ebx].nRightDecimal
		mov	ecx, [ebx].nLineRight[0]
		sub	eax, [ebx].nLeftDecimal
		sbb	ecx, [ebx].nLineLeft[0]
		inc	ecx
		.IF	ecx < SizeOf_tblRcpLookup
			movd	mm7, tblRcpLookup[ecx*8].int_part
			pxor	mm4, mm4
			psrld	mm7, 15
			movd	mm0, [esi].rgbaLeft.rgbMul.dwPixelCode
			movd	mm1, [esi].rgbaLeft.rgbAdd.dwPixelCode
			movd	mm2, [esi].rgbaRight.rgbMul.dwPixelCode
			movd	mm3, [esi].rgbaRight.rgbAdd.dwPixelCode
		.ELSE
			mov	eax, 8000H
			cvtsi2ss	xmm0, eax
			cvtsi2ss	xmm1, ecx
			divss	xmm0, xmm1
			pxor	mm4, mm4
			movd	mm0, [esi].rgbaLeft.rgbMul.dwPixelCode
			movd	mm1, [esi].rgbaLeft.rgbAdd.dwPixelCode
			movd	mm2, [esi].rgbaRight.rgbMul.dwPixelCode
			movd	mm3, [esi].rgbaRight.rgbAdd.dwPixelCode
			cvtps2pi	mm7, xmm0
		.ENDIF
		punpcklbw	mm0, mm4
		punpcklbw	mm1, mm4
		punpcklbw	mm2, mm4
		punpcklbw	mm3, mm4
		psubw	mm2, mm0
		psllw	mm0, 7
		psubw	mm3, mm1
		psllw	mm1, 7
		;movq	MMWORD PTR [ebx].rgbNextColor[0], mm0
		;movq	MMWORD PTR [ebx].rgbNextColor[8], mm1
		pshufw	mm7, mm7, 0
		movq	mm4, mm2
		movq	mm5, mm3
		pmulhw	mm2, mm7
			movd	mm6, [ebx].nLeftDecimal
		pmullw	mm4, mm7
			psrld	mm6, 1
		pmulhw	mm3, mm7
			pshufw	mm6, mm6, 0
		pmullw	mm5, mm7
		psllw	mm2, 8
		psrlw	mm4, 8
		psllw	mm3, 8
		psrlw	mm5, 8
		por	mm2, mm4
		por	mm3, mm5
		movq	MMWORD PTR [ebx].rgbDeltaColor[0], mm2
			pmulhw	mm2, mm6
		movq	MMWORD PTR [ebx].rgbDeltaColor[8], mm3
			pmulhw	mm3, mm6
		;
			psubsw	mm0, mmxConst2000x3_0
			psubsw	mm1, mmxConst2000x3_0
			psubsw	mm0, mm2
			psubsw	mm1, mm3
			psubsw	mm0, mm2
			psubsw	mm1, mm3
			movq	MMWORD PTR [ebx].rgbNextColor[0], mm0
			movq	MMWORD PTR [ebx].rgbNextColor[8], mm1
		;
		ASSUME	esi:NOTHING
		;
		.IF	[ebx].dwFunctionFlags & E3D_FLAG_ENABLE_SSE2
			;
			;	ラインバッファにレンダリング
			; ----------------------------------------------------
			call	[ebx].pfnLineFunc
			;
;			call	eglRenderPoly@RenderPolygon_LineZ_SSE2
			;
			;	ランバッファを出力
			; ----------------------------------------------------
			call	[ebx].pfnStoreFunc
			;
			;	アンチエイリアス
			; ----------------------------------------------------
			mov	eax, [ebx].pfnAntialius
			test	eax, eax
			.IF	!ZERO?
				call	eax
			.ENDIF
			;
			;	次の行へ移動
			; ----------------------------------------------------
			movapd	xmm0, [ebx].vTxLinePos_d
			movapd	xmm1, [ebx].vTxDeltaY_d
			_movsd	xmm2, [ebx].rTxLineMod_d
				mov	eax, [ebx].dib.ptrDstLine
			_movsd	xmm3, [ebx].vTxxy_d.y
				mov	edx, [ebx].dib.ptrZBufLine
			addpd	xmm0, xmm1
				add	eax, [ebx].dib.dstimg.dwBytesPerLine
				add	edx, [ebx].dib.zbuf.dwBytesPerLine
			addsd	xmm2, xmm3
				mov	[ebx].dib.ptrDstLine, eax
				mov	[ebx].dib.ptrZBufLine, edx
				mov	esi, [ebx].dib.ptrRegionLine
				mov	ecx, [ebx].dib.nLeftHeight
			movapd	[ebx].vTxLinePos_d, xmm0
			cvtpd2ps	xmm4, xmm0
				add	esi, (SIZEOF E3D_POLY_LINE_REGION)
			_movsd	[ebx].rTxLineMod_d, xmm2
			cvtsd2ss	xmm5, xmm2
			movlps	[ebx].vTxLinePos, xmm4
			movss	[ebx].rTxLineMod, xmm5
		.ELSE
			;
			;	ラインバッファにレンダリング
			; ----------------------------------------------------
			call	[ebx].pfnLineFunc
			;
			;	ランバッファを出力
			; ----------------------------------------------------
			call	[ebx].pfnStoreFunc
			;
			;	アンチエイリアス
			; ----------------------------------------------------
			mov	eax, [ebx].pfnAntialius
			test	eax, eax
			.IF	!ZERO?
				call	eax
			.ENDIF
			;
			;	次の行へ移動
			; ----------------------------------------------------
			movlps	xmm0, [ebx].vTxLinePos
			movlps	xmm1, [ebx].vTxDeltaY
			movss	xmm2, [ebx].rTxLineMod
			mov	eax, [ebx].dib.ptrDstLine
			movss	xmm3, [ebx].vTxxy.y
			mov	edx, [ebx].dib.ptrZBufLine
			addps	xmm0, xmm1
			add	eax, [ebx].dib.dstimg.dwBytesPerLine
			add	edx, [ebx].dib.zbuf.dwBytesPerLine
			addss	xmm2, xmm3
			mov	[ebx].dib.ptrDstLine, eax
			mov	[ebx].dib.ptrZBufLine, edx
			mov	esi, [ebx].dib.ptrRegionLine
			mov	ecx, [ebx].dib.nLeftHeight
			movlps	[ebx].vTxLinePos, xmm0
			add	esi, (SIZEOF E3D_POLY_LINE_REGION)
			movss	[ebx].rTxLineMod, xmm2
		.ENDIF
		;
		inc	[ebx].yLinePos
		dec	ecx
	.UNTIL	ZERO?

	ASSUME	ebx:NOTHING
	emms
	sfence
	xor	eax, eax
	ret

eglRenderPoly@RenderPolygon_TextureSSE	ENDP

;
;	テクスチャマッピングポリゴンレンダリング（z 比較無） SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_TxtNZSSE	PROC	NEAR32 C USES ebx esi edi,
	hRenderPoly:HEGL_RENDER_POLYGON

	mov	ebx, hRenderPoly
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF

	;
	;	リージョン情報セットアップ
	; --------------------------------------------------------------------
	mov	esi, [ebx].dib.pRegion
	ASSUME	esi:PTR E3D_POLYGON_REGION
	mov	eax, [esi].nTopLine
	mov	edx, eax
	mov	[ebx].yLinePos, eax
	imul	eax, [ebx].dib.dstimg.dwBytesPerLine
	imul	edx, [ebx].dib.zbuf.dwBytesPerLine
	add	eax, [ebx].dib.dstimg.ptrImageArray
	add	edx, [ebx].dib.zbuf.ptrImageArray
	mov	[ebx].dib.ptrDstLine, eax
	mov	[ebx].dib.ptrZBufLine, edx
	;
	mov	eax, [esi].nBottomLine
	sub	eax, [esi].nTopLine
	inc	eax
	mov	[ebx].dib.nLeftHeight, eax
	;
	lea	esi, [esi].plrLineRgn[0]
	mov	[ebx].dib.ptrRegionLine, esi
	;
	mov	eax, [ebx].vTxBasePos.x
	mov	edx, [ebx].vTxBasePos.y
	mov	ecx, [ebx].rTxtxy
	mov	[ebx].vTxLinePos.x, eax
	mov	[ebx].vTxLinePos.y, edx
	mov	[ebx].rTxLineMod, ecx

	;
	;	描画ループ
	; --------------------------------------------------------------------
	mov	ecx, [ebx].dib.nLeftHeight
	.REPEAT
		mov	[ebx].dib.nLeftHeight, ecx
		mov	[ebx].dib.ptrRegionLine, esi
		;
		;	ライン情報をセットアップ
		; ------------------------------------------------------------
		ASSUME	esi:PTR E3D_POLY_LINE_REGION
		mov	eax, [esi].nLeft
		movzx	edx, WORD PTR [esi].dwReserved1
			movq	mm0, MMWORD PTR [esi].vLeft
			movq	mm1, MMWORD PTR [esi].vRight
		mov	[ebx].nLineLeft[0], eax
		mov	[ebx].nLeftDecimal, edx
			movq	MMWORD PTR [ebx].vLineLeftNormal, mm0
			movq	MMWORD PTR [ebx].vLineRightNormal, mm1
		and	eax, NOT 01H
		mov	[ebx].nLineLeft[4], eax
		mov	eax, [esi].nRight
		movzx	edx, WORD PTR [esi].dwReserved2
		mov	[ebx].nLineRight[0], eax
		add	eax, 01H
		mov	[ebx].nRightDecimal, edx
		and	eax, NOT 01H
		add	eax, 2
		mov	[ebx].nLineRight[4], eax
		;
		mov	eax, [ebx].nRightDecimal
		mov	ecx, [ebx].nLineRight[0]
		sub	eax, [ebx].nLeftDecimal
		sbb	ecx, [ebx].nLineLeft[0]
		inc	ecx
		.IF	ecx < SizeOf_tblRcpLookup
			movd	mm7, tblRcpLookup[ecx*8].int_part
			pxor	mm4, mm4
			psrld	mm7, 15
			movd	mm0, [esi].rgbaLeft.rgbMul.dwPixelCode
			movd	mm1, [esi].rgbaLeft.rgbAdd.dwPixelCode
			movd	mm2, [esi].rgbaRight.rgbMul.dwPixelCode
			movd	mm3, [esi].rgbaRight.rgbAdd.dwPixelCode
		.ELSE
			mov	eax, 8000H
			cvtsi2ss	xmm0, eax
			cvtsi2ss	xmm1, ecx
			divss	xmm0, xmm1
			pxor	mm4, mm4
			movd	mm0, [esi].rgbaLeft.rgbMul.dwPixelCode
			movd	mm1, [esi].rgbaLeft.rgbAdd.dwPixelCode
			movd	mm2, [esi].rgbaRight.rgbMul.dwPixelCode
			movd	mm3, [esi].rgbaRight.rgbAdd.dwPixelCode
			cvtps2pi	mm7, xmm0
		.ENDIF
		punpcklbw	mm0, mm4
		punpcklbw	mm1, mm4
		punpcklbw	mm2, mm4
		punpcklbw	mm3, mm4
		psubw	mm2, mm0
		psllw	mm0, 7
		psubw	mm3, mm1
		psllw	mm1, 7
		;movq	MMWORD PTR [ebx].rgbNextColor[0], mm0
		;movq	MMWORD PTR [ebx].rgbNextColor[8], mm1
		pshufw	mm7, mm7, 0
		movq	mm4, mm2
		movq	mm5, mm3
		pmulhw	mm2, mm7
			movd	mm6, [ebx].nLeftDecimal
		pmullw	mm4, mm7
			psrld	mm6, 1
		pmulhw	mm3, mm7
			pshufw	mm6, mm6, 0
		pmullw	mm5, mm7
		psllw	mm2, 8
		psrlw	mm4, 8
		psllw	mm3, 8
		psrlw	mm5, 8
		por	mm2, mm4
		por	mm3, mm5
		movq	MMWORD PTR [ebx].rgbDeltaColor[0], mm2
			pmulhw	mm2, mm6
		movq	MMWORD PTR [ebx].rgbDeltaColor[8], mm3
			pmulhw	mm3, mm6
		;
			psubsw	mm0, mmxConst2000x3_0
			psubsw	mm1, mmxConst2000x3_0
			psubsw	mm0, mm2
			psubsw	mm1, mm3
			psubsw	mm0, mm2
			psubsw	mm1, mm3
			movq	MMWORD PTR [ebx].rgbNextColor[0], mm0
			movq	MMWORD PTR [ebx].rgbNextColor[8], mm1
		ASSUME	esi:NOTHING
		;
		;	ラインバッファにレンダリング
		; ------------------------------------------------------------
		call	[ebx].pfnLineFunc
		;
		;	ランバッファを出力
		; ------------------------------------------------------------
		call	[ebx].pfnStoreFunc
		;
		;	次の行へ移動
		; ------------------------------------------------------------
		movlps	xmm0, [ebx].vTxLinePos
		movlps	xmm1, [ebx].vTxDeltaY
		movss	xmm2, [ebx].rTxLineMod
		mov	eax, [ebx].dib.ptrDstLine
		movss	xmm3, [ebx].vTxxy.y
		mov	edx, [ebx].dib.ptrZBufLine
		addps	xmm0, xmm1
		add	eax, [ebx].dib.dstimg.dwBytesPerLine
		add	edx, [ebx].dib.zbuf.dwBytesPerLine
		addss	xmm2, xmm3
		mov	[ebx].dib.ptrDstLine, eax
		mov	[ebx].dib.ptrZBufLine, edx
		mov	esi, [ebx].dib.ptrRegionLine
		mov	ecx, [ebx].dib.nLeftHeight
		movlps	[ebx].vTxLinePos, xmm0
		add	esi, (SIZEOF E3D_POLY_LINE_REGION)
		movss	[ebx].rTxLineMod, xmm2
		inc	[ebx].yLinePos
		dec	ecx
	.UNTIL	ZERO?

	ASSUME	ebx:NOTHING
	emms
	sfence
	xor	eax, eax
	ret

eglRenderPoly@RenderPolygon_TxtNZSSE	ENDP


;
;	テクスチャマッピングポリゴンレンダリング（ｚ比較のみ）SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_TxtRZSSE	PROC	NEAR32 C USES ebx esi edi,
	hRenderPoly:HEGL_RENDER_POLYGON

	mov	ebx, hRenderPoly
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF

	;
	;	リージョン情報セットアップ
	; --------------------------------------------------------------------
	mov	esi, [ebx].dib.pRegion
	ASSUME	esi:PTR E3D_POLYGON_REGION
	mov	eax, [esi].nTopLine
	mov	edx, eax
	mov	[ebx].yLinePos, eax
	imul	eax, [ebx].dib.dstimg.dwBytesPerLine
	imul	edx, [ebx].dib.zbuf.dwBytesPerLine
	add	eax, [ebx].dib.dstimg.ptrImageArray
	add	edx, [ebx].dib.zbuf.ptrImageArray
	mov	[ebx].dib.ptrDstLine, eax
	mov	[ebx].dib.ptrZBufLine, edx
	;
	mov	eax, [esi].nBottomLine
	sub	eax, [esi].nTopLine
	inc	eax
	mov	[ebx].dib.nLeftHeight, eax
	;
	lea	esi, [esi].plrLineRgn[0]
	mov	[ebx].dib.ptrRegionLine, esi
	;
	.IF	[ebx].dwFunctionFlags & E3D_FLAG_ENABLE_SSE2
		movapd	xmm0, [ebx].vTxBasePos_d
		_movsd	xmm1, [ebx].rTxtxy_d
		movapd	[ebx].vTxLinePos_d, xmm0
		_movsd	[ebx].rTxLineMod_d, xmm1
		cvtpd2ps	xmm0, xmm0
		cvtsd2ss	xmm1, xmm1
		movlps	QWORD PTR [ebx].vTxLinePos, xmm0
		movss	[ebx].rTxLineMod, xmm1
	.ELSE
		mov	eax, [ebx].vTxBasePos.x
		mov	edx, [ebx].vTxBasePos.y
		mov	ecx, [ebx].rTxtxy
		mov	[ebx].vTxLinePos.x, eax
		mov	[ebx].vTxLinePos.y, edx
		mov	[ebx].rTxLineMod, ecx
	.ENDIF

	;
	;	描画ループ
	; --------------------------------------------------------------------
	mov	ecx, [ebx].dib.nLeftHeight
	.REPEAT
		mov	[ebx].dib.nLeftHeight, ecx
		mov	[ebx].dib.ptrRegionLine, esi
		;
		;	ライン情報をセットアップ
		; ------------------------------------------------------------
		ASSUME	esi:PTR E3D_POLY_LINE_REGION
		mov	eax, [esi].nLeft
		movzx	edx, WORD PTR [esi].dwReserved1
			movq	mm0, MMWORD PTR [esi].vLeft
			movq	mm1, MMWORD PTR [esi].vRight
		mov	[ebx].nLineLeft[0], eax
		mov	[ebx].nLeftDecimal, edx
			movq	MMWORD PTR [ebx].vLineLeftNormal, mm0
			movq	MMWORD PTR [ebx].vLineRightNormal, mm1
		and	eax, NOT 01H
		mov	[ebx].nLineLeft[4], eax
		mov	eax, [esi].nRight
		movzx	edx, WORD PTR [esi].dwReserved2
		mov	[ebx].nLineRight[0], eax
		add	eax, 01H
		mov	[ebx].nRightDecimal, edx
		and	eax, NOT 01H
		add	eax, 2
		mov	[ebx].nLineRight[4], eax
		;
		mov	eax, [ebx].nRightDecimal
		mov	ecx, [ebx].nLineRight[0]
		sub	eax, [ebx].nLeftDecimal
		sbb	ecx, [ebx].nLineLeft[0]
		inc	ecx
		.IF	ecx < SizeOf_tblRcpLookup
			movd	mm7, tblRcpLookup[ecx*8].int_part
			pxor	mm4, mm4
			psrld	mm7, 15
			movd	mm0, [esi].rgbaLeft.rgbMul.dwPixelCode
			movd	mm1, [esi].rgbaLeft.rgbAdd.dwPixelCode
			movd	mm2, [esi].rgbaRight.rgbMul.dwPixelCode
			movd	mm3, [esi].rgbaRight.rgbAdd.dwPixelCode
		.ELSE
			mov	eax, 8000H
			cvtsi2ss	xmm0, eax
			cvtsi2ss	xmm1, ecx
			divss	xmm0, xmm1
			pxor	mm4, mm4
			movd	mm0, [esi].rgbaLeft.rgbMul.dwPixelCode
			movd	mm1, [esi].rgbaLeft.rgbAdd.dwPixelCode
			movd	mm2, [esi].rgbaRight.rgbMul.dwPixelCode
			movd	mm3, [esi].rgbaRight.rgbAdd.dwPixelCode
			cvtps2pi	mm7, xmm0
		.ENDIF
		punpcklbw	mm0, mm4
		punpcklbw	mm1, mm4
		punpcklbw	mm2, mm4
		punpcklbw	mm3, mm4
		psubw	mm2, mm0
		psllw	mm0, 7
		psubw	mm3, mm1
		psllw	mm1, 7
		;movq	MMWORD PTR [ebx].rgbNextColor[0], mm0
		;movq	MMWORD PTR [ebx].rgbNextColor[8], mm1
		pshufw	mm7, mm7, 0
		movq	mm4, mm2
		movq	mm5, mm3
		pmulhw	mm2, mm7
			movd	mm6, [ebx].nLeftDecimal
		pmullw	mm4, mm7
			psrld	mm6, 1
		pmulhw	mm3, mm7
			pshufw	mm6, mm6, 0
		pmullw	mm5, mm7
		psllw	mm2, 8
		psrlw	mm4, 8
		psllw	mm3, 8
		psrlw	mm5, 8
		por	mm2, mm4
		por	mm3, mm5
		movq	MMWORD PTR [ebx].rgbDeltaColor[0], mm2
			pmulhw	mm2, mm6
		movq	MMWORD PTR [ebx].rgbDeltaColor[8], mm3
			pmulhw	mm3, mm6
		;
			psubsw	mm0, mmxConst2000x3_0
			psubsw	mm1, mmxConst2000x3_0
			psubsw	mm0, mm2
			psubsw	mm1, mm3
			psubsw	mm0, mm2
			psubsw	mm1, mm3
			movq	MMWORD PTR [ebx].rgbNextColor[0], mm0
			movq	MMWORD PTR [ebx].rgbNextColor[8], mm1
		ASSUME	esi:NOTHING
		;
		;	一時ｚバッファ初期化
		; ------------------------------------------------------------
		mov	edi, [ebx].pTempZBuffer[0]
		mov	edx, [ebx].dib.ptrZBufLine
		mov	eax, [ebx].nLineLeft[4]
		mov	ecx, [ebx].nLineRight[4]
		lea	edi, [edi + eax * 4]
		lea	edx, [edx + eax * 4]
		sub	ecx, eax
		shr	ecx, 1
		inc	ecx
		.REPEAT
			movq	mm0, MMWORD PTR [edx]
			add	edx, 8
			movq	MMWORD PTR [edi], mm0
			add	edi, 8
			dec	ecx
		.UNTIL	ZERO?
		;
		mov	eax, [ebx].pTempZBuffer[0]
		mov	edx, [ebx].dib.ptrZBufLine
		mov	[ebx].dib.ptrZBufLine, eax
		push	edx
		;
		;	ラインバッファにレンダリング
		; ------------------------------------------------------------
		call	[ebx].pfnLineFunc
		;
		;	ランバッファを出力
		; ------------------------------------------------------------
		call	[ebx].pfnStoreFunc
		;
		;	次の行へ移動
		; ------------------------------------------------------------
		.IF	[ebx].dwFunctionFlags & E3D_FLAG_ENABLE_SSE2
			movapd	xmm0, [ebx].vTxLinePos_d
			movapd	xmm1, [ebx].vTxDeltaY_d
			_movsd	xmm2, [ebx].rTxLineMod_d
				mov	eax, [ebx].dib.ptrDstLine
			_movsd	xmm3, [ebx].vTxxy_d.y
;				mov	edx, [ebx].dib.ptrZBufLine
				pop	edx
			addpd	xmm0, xmm1
				add	eax, [ebx].dib.dstimg.dwBytesPerLine
				add	edx, [ebx].dib.zbuf.dwBytesPerLine
			addsd	xmm2, xmm3
				mov	[ebx].dib.ptrDstLine, eax
				mov	[ebx].dib.ptrZBufLine, edx
				mov	esi, [ebx].dib.ptrRegionLine
				mov	ecx, [ebx].dib.nLeftHeight
			movapd	[ebx].vTxLinePos_d, xmm0
			cvtpd2ps	xmm4, xmm0
				add	esi, (SIZEOF E3D_POLY_LINE_REGION)
			_movsd	[ebx].rTxLineMod_d, xmm2
			cvtsd2ss	xmm5, xmm2
			movlps	[ebx].vTxLinePos, xmm4
			movss	[ebx].rTxLineMod, xmm5
		.ELSE
			movlps	xmm0, [ebx].vTxLinePos
			movlps	xmm1, [ebx].vTxDeltaY
			movss	xmm2, [ebx].rTxLineMod
			mov	eax, [ebx].dib.ptrDstLine
			movss	xmm3, [ebx].vTxxy.y
;			mov	edx, [ebx].dib.ptrZBufLine
			pop	edx
			addps	xmm0, xmm1
			add	eax, [ebx].dib.dstimg.dwBytesPerLine
			add	edx, [ebx].dib.zbuf.dwBytesPerLine
			addss	xmm2, xmm3
			mov	[ebx].dib.ptrDstLine, eax
			mov	[ebx].dib.ptrZBufLine, edx
			mov	esi, [ebx].dib.ptrRegionLine
			mov	ecx, [ebx].dib.nLeftHeight
			movlps	[ebx].vTxLinePos, xmm0
			add	esi, (SIZEOF E3D_POLY_LINE_REGION)
			movss	[ebx].rTxLineMod, xmm2
		.ENDIF
		inc	[ebx].yLinePos
		dec	ecx
	.UNTIL	ZERO?

	ASSUME	ebx:NOTHING
	emms
	sfence
	xor	eax, eax
	ret

eglRenderPoly@RenderPolygon_TxtRZSSE	ENDP

;
;	テクスチャマッピングポリゴンレンダリング SSE 専用コード（ｚ比較無効化）
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_TxtVNZSSE	PROC	NEAR32 C USES ebx esi edi,
	hRenderPoly:HEGL_RENDER_POLYGON

	mov	ebx, hRenderPoly
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF

	;
	;	リージョン情報セットアップ
	; --------------------------------------------------------------------
	mov	esi, [ebx].dib.pRegion
	ASSUME	esi:PTR E3D_POLYGON_REGION
	mov	eax, [esi].nTopLine
	mov	edx, eax
	mov	[ebx].yLinePos, eax
	imul	eax, [ebx].dib.dstimg.dwBytesPerLine
	imul	edx, [ebx].dib.zbuf.dwBytesPerLine
	add	eax, [ebx].dib.dstimg.ptrImageArray
	add	edx, [ebx].dib.zbuf.ptrImageArray
	mov	[ebx].dib.ptrDstLine, eax
	mov	[ebx].dib.ptrZBufLine, edx
	;
	mov	eax, [esi].nBottomLine
	sub	eax, [esi].nTopLine
	inc	eax
	mov	[ebx].dib.nLeftHeight, eax
	;
	lea	esi, [esi].plrLineRgn[0]
	mov	[ebx].dib.ptrRegionLine, esi
	;
	mov	eax, [ebx].vTxBasePos.x
	mov	edx, [ebx].vTxBasePos.y
	mov	ecx, [ebx].rTxtxy
	mov	[ebx].vTxLinePos.x, eax
	mov	[ebx].vTxLinePos.y, edx
	mov	[ebx].rTxLineMod, ecx

	;
	;	描画ループ
	; --------------------------------------------------------------------
	mov	ecx, [ebx].dib.nLeftHeight
	.REPEAT
		mov	[ebx].dib.nLeftHeight, ecx
		mov	[ebx].dib.ptrRegionLine, esi
		;
		;	ライン情報をセットアップ
		; ------------------------------------------------------------
		ASSUME	esi:PTR E3D_POLY_LINE_REGION
		mov	eax, [esi].nLeft
		movzx	edx, WORD PTR [esi].dwReserved1
			movq	mm0, MMWORD PTR [esi].vLeft
			movq	mm1, MMWORD PTR [esi].vRight
		mov	[ebx].nLineLeft[0], eax
		mov	[ebx].nLeftDecimal, edx
			movq	MMWORD PTR [ebx].vLineLeftNormal, mm0
			movq	MMWORD PTR [ebx].vLineRightNormal, mm1
		and	eax, NOT 01H
		mov	[ebx].nLineLeft[4], eax
		mov	eax, [esi].nRight
		movzx	edx, WORD PTR [esi].dwReserved2
		mov	[ebx].nLineRight[0], eax
		add	eax, 01H
		mov	[ebx].nRightDecimal, edx
		and	eax, NOT 01H
		add	eax, 2
		mov	[ebx].nLineRight[4], eax
		;
		mov	eax, [ebx].nRightDecimal
		mov	ecx, [ebx].nLineRight[0]
		sub	eax, [ebx].nLeftDecimal
		sbb	ecx, [ebx].nLineLeft[0]
		inc	ecx
		.IF	ecx < SizeOf_tblRcpLookup
			movd	mm7, tblRcpLookup[ecx*8].int_part
			pxor	mm4, mm4
			psrld	mm7, 15
			movd	mm0, [esi].rgbaLeft.rgbMul.dwPixelCode
			movd	mm1, [esi].rgbaLeft.rgbAdd.dwPixelCode
			movd	mm2, [esi].rgbaRight.rgbMul.dwPixelCode
			movd	mm3, [esi].rgbaRight.rgbAdd.dwPixelCode
		.ELSE
			mov	eax, 8000H
			cvtsi2ss	xmm0, eax
			cvtsi2ss	xmm1, ecx
			divss	xmm0, xmm1
			pxor	mm4, mm4
			movd	mm0, [esi].rgbaLeft.rgbMul.dwPixelCode
			movd	mm1, [esi].rgbaLeft.rgbAdd.dwPixelCode
			movd	mm2, [esi].rgbaRight.rgbMul.dwPixelCode
			movd	mm3, [esi].rgbaRight.rgbAdd.dwPixelCode
			cvtps2pi	mm7, xmm0
		.ENDIF
		punpcklbw	mm0, mm4
		punpcklbw	mm1, mm4
		punpcklbw	mm2, mm4
		punpcklbw	mm3, mm4
		psubw	mm2, mm0
		psllw	mm0, 7
		psubw	mm3, mm1
		psllw	mm1, 7
		;movq	MMWORD PTR [ebx].rgbNextColor[0], mm0
		;movq	MMWORD PTR [ebx].rgbNextColor[8], mm1
		pshufw	mm7, mm7, 0
		movq	mm4, mm2
		movq	mm5, mm3
		pmulhw	mm2, mm7
			movd	mm6, [ebx].nLeftDecimal
		pmullw	mm4, mm7
			psrld	mm6, 1
		pmulhw	mm3, mm7
			pshufw	mm6, mm6, 0
		pmullw	mm5, mm7
		psllw	mm2, 8
		psrlw	mm4, 8
		psllw	mm3, 8
		psrlw	mm5, 8
		por	mm2, mm4
		por	mm3, mm5
		movq	MMWORD PTR [ebx].rgbDeltaColor[0], mm2
			pmulhw	mm2, mm6
		movq	MMWORD PTR [ebx].rgbDeltaColor[8], mm3
			pmulhw	mm3, mm6
		;
			psubsw	mm0, mmxConst2000x3_0
			psubsw	mm1, mmxConst2000x3_0
			psubsw	mm0, mm2
			psubsw	mm1, mm3
			psubsw	mm0, mm2
			psubsw	mm1, mm3
			movq	MMWORD PTR [ebx].rgbNextColor[0], mm0
			movq	MMWORD PTR [ebx].rgbNextColor[8], mm1
		;
		ASSUME	esi:NOTHING
		;
		;	一時ｚバッファ初期化
		; ------------------------------------------------------------
		mov	edi, [ebx].dib.ptrZBufLine
		mov	eax, [ebx].nLineLeft[4]
		mov	ecx, [ebx].nLineRight[4]
		lea	edi, [edi + eax * 4]
		sub	ecx, eax
		mov	eax, 7F000000H
		shr	ecx, 1
		movd	mm0, eax
		inc	ecx
		punpckldq	mm0, mm0
		sub	ecx, 4
		.WHILE	!SIGN?
			movq	MMWORD PTR [edi], mm0
			movq	MMWORD PTR [edi + 8], mm0
			movq	MMWORD PTR [edi + 16], mm0
			movq	MMWORD PTR [edi + 24], mm0
			add	edi, 32
			sub	ecx, 4
		.ENDW
		add	ecx, 4
		.WHILE	!ZERO?
			movq	MMWORD PTR [edi], mm0
			add	edi, 8
			dec	ecx
		.ENDW
		;
		;	ラインバッファにレンダリング
		; ------------------------------------------------------------
		call	[ebx].pfnLineFunc
		;
		;	ランバッファを出力
		; ------------------------------------------------------------
		call	[ebx].pfnStoreFunc
		;
		;	次の行へ移動
		; ------------------------------------------------------------
		movlps	xmm0, [ebx].vTxLinePos
		movlps	xmm1, [ebx].vTxDeltaY
		movss	xmm2, [ebx].rTxLineMod
		mov	eax, [ebx].dib.ptrDstLine
		movss	xmm3, [ebx].vTxxy.y
		mov	edx, [ebx].dib.ptrZBufLine
		addps	xmm0, xmm1
		add	eax, [ebx].dib.dstimg.dwBytesPerLine
		add	edx, [ebx].dib.zbuf.dwBytesPerLine
		addss	xmm2, xmm3
		mov	[ebx].dib.ptrDstLine, eax
		mov	[ebx].dib.ptrZBufLine, edx
		mov	esi, [ebx].dib.ptrRegionLine
		mov	ecx, [ebx].dib.nLeftHeight
		movlps	[ebx].vTxLinePos, xmm0
		add	esi, (SIZEOF E3D_POLY_LINE_REGION)
		movss	[ebx].rTxLineMod, xmm2
		inc	[ebx].yLinePos
		dec	ecx
	.UNTIL	ZERO?

	ASSUME	ebx:NOTHING
	emms
	sfence
	xor	eax, eax
	ret

eglRenderPoly@RenderPolygon_TxtVNZSSE	ENDP


;
;	メッシュレンダリング SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_MeshSSE	PROC	NEAR32 C USES ebx esi edi,
	hRenderPoly:HEGL_RENDER_POLYGON

	LOCAL	nLeftPolyCount:DWORD
	LOCAL	pNextMeshEntry:PTR E3D_PRIMITIVE_MESH_POLY
	LOCAL	rRadius:REAL4

	LOCAL	dwSavedESP:DWORD
	LOCAL	pMeshVertices:PE3D_VECTOR4
	LOCAL	pMeshNormals:PE3D_VECTOR4
	LOCAL	pMeshUVMaps:PE3D_VECTOR_2D
	LOCAL	pMeshColors:PE3D_COLOR
	LOCAL	txmap:E3D_TEXTURE_MAPINFO
	LOCAL	plane:E3D_PLANE_PARAMETER
	LOCAL	pVertices:PE3D_VECTOR4
	LOCAL	pVertices2D:PE3D_VECTOR_2D
	LOCAL	pUVMap:PE3D_VECTOR_2D
	LOCAL	pVertexColors:PE3D_COLOR
	LOCAL	pNormals[2]:PE3D_VECTOR4
	LOCAL	fZClipping:DWORD
	LOCAL	dwVertexCount:DWORD
	LOCAL	dwLoopCounter:DWORD

	;
	;	変数初期化
	; --------------------------------------------------------------------
	mov	ebx, hRenderPoly
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF

	mov	dwSavedESP, esp
	mov	pVertices, 0
	mov	pVertexColors, 0

	mov	esi, [ebx].pMeshPolygonEntry
	ASSUME	esi:PTR E3D_POLYGON_ENTRY

	.IF	([esi].dwShadingFlags & E3DSAF_RAY_SHADOWING) && \
					([ebx].nRayShadowings != 0)
		movss	xmm0, [esi].surface.mesh.rMeshRadius
		addss	xmm0, [ebx].rrtpRayParam.rShadowingDistance
		movss	rRadius, xmm0
		INVOKE	eglRenderPoly@MeshList@CollectMeshSSE ,
			[ebx].ppRayShadowingList,
			[ebx].pRayShadowing, [ebx].nRayShadowings,
			ADDR [esi].vCenter, rRadius
		mov	[ebx].nRayShadowingListCount, eax
	.ENDIF
	.IF	([esi].dwShadingFlags & \
			(E3DSAF_RAY_REFLECTING OR E3DSAF_RAY_REFRACTING)) && \
					([ebx].nRayReflections != 0)
		movss	xmm0, [esi].surface.mesh.rMeshRadius
		addss	xmm0, [ebx].rrtpRayParam.rRayTracingDistance
		movss	rRadius, xmm0
		INVOKE	eglRenderPoly@MeshList@CollectMeshSSE ,
			[ebx].ppRayReflectionList,
			[ebx].pRayReflection, [ebx].nRayReflections,
			ADDR [esi].vCenter, rRadius
		mov	[ebx].nRayReflectionListCount, eax
	.ENDIF

	mov	edx, [esi].surface.mesh.pMesh
	ASSUME	edx:PTR E3D_PRIMITIVE_MESH_LIST
	mov	eax, [esi].pVertexes
	mov	pMeshVertices, eax
	mov	eax, [esi].pNormals
	mov	pMeshNormals, eax
	mov	eax, [esi].surface.mesh.pUVMap
	mov	pMeshUVMaps, eax
	mov	eax, [esi].pVertexColors
	mov	pMeshColors, eax
	;
	mov	ecx, [edx].dwPolyCount
	lea	edx, [edx].mpEntries[0]
	mov	nLeftPolyCount, ecx
	mov	pNextMeshEntry, edx
	;
	test	ecx, ecx
	mov	[ebx].nMeshPolygonIndex, 0
	.WHILE	!ZERO?
		;
		;	ポリゴン座標セットアップ
		; ------------------------------------------------------------
		;
		; バッファをスタックに確保
		;
		ASSUME	edx:PTR E3D_PRIMITIVE_MESH_POLY
		mov	ecx, [edx].dwVertexCount
		mov	esp, dwSavedESP
		mov	eax, ecx
		mov	dwVertexCount, ecx
		shl	eax, 4		; *= (SIZEOF E3D_VECTOR)
		sub	esp, eax
		and	esp, NOT 0FH
		mov	pVertices, esp
		;
		sub	esp, eax
		and	esp, NOT 0FH
		mov	pNormals, esp
		;
		lea	eax, [ecx * (SIZEOF E3D_VECTOR_2D)]
		sub	esp, eax
		and	esp, NOT 0FH
		mov	pUVMap, esp
		;
		lea	eax, [ecx * (SIZEOF E3D_COLOR)]
		sub	esp, eax
		and	esp, NOT 0FH
		mov	pVertexColors, esp
		;
		; 頂点複製
		;
		cmp	[edx].dwVertexCount, 3
		jb	Label_Continue
		mov	edi, pVertices
		ASSUME	edi:PE3D_VECTOR4
		mov	esi, pMeshVertices
		ASSUME	esi:NOTHING
		xor	ecx, ecx
		movss	xmm6, [ebx].rZMinClip
		movss	xmm7, xmm6
		.REPEAT	
			mov	eax, [edx].dwIndex[ecx*4]
			shl	eax, 4
			movups	xmm0, [esi + eax]
			movhlps	xmm1, xmm0
			movaps	[edi], xmm0
			minss	xmm6, xmm1
			maxss	xmm7, xmm1
			add	edi, (SIZEOF E3D_VECTOR4)
			inc	ecx
		.UNTIL	ecx >= [edx].dwVertexCount
		ASSUME	edi:NOTHING
		;
		comiss	xmm7, [ebx].rZMinClip
		jbe	Label_Continue
		comiss	xmm6, [ebx].rZMinClip
		sbb	eax, eax
		mov	fZClipping, eax
		;
		; 頂点色複製
		;
		mov	edi, pVertexColors
		mov	esi, pMeshColors
		xor	ecx, ecx
		.REPEAT	
			mov	eax, [edx].dwIndex[ecx*4]
			movq	mm0, QWORD PTR [esi + eax*8]
			movq	QWORD PTR [edi], mm0
			add	edi, (SIZEOF E3D_COLOR)
			inc	ecx
		.UNTIL	ecx >= [edx].dwVertexCount
		;
		; 法線複製
		;
		mov	edi, pNormals
		mov	esi, pMeshNormals
		xor	ecx, ecx
		.REPEAT	
			mov	eax, [edx].dwIndex[ecx*4]
			shl	eax, 4
			movups	xmm0, [esi + eax]
			movaps	[edi], xmm0
			add	edi, (SIZEOF E3D_VECTOR4)
			inc	ecx
		.UNTIL	ecx >= [edx].dwVertexCount
		;
		;	裏面ポリゴン判定
		; ------------------------------------------------------------
		;
		; 平面パラメータ計算
		;
		mov	eax, pVertices
		movaps	xmm0, [eax]
		movaps	xmm1, [eax + 10H]
		movaps	xmm2, [eax + 20H]
		;
		INVOKE	eglRenderPoly@CalcPlaneParameterSSE
		jc	Label_Continue
		;
		movups	plane, xmm6
		movaps	[ebx].vTargetPlaneParam, xmm6
		;
		movaps	xmm0, [eax]		; 視線と面の成す角度を求める
		movaps	xmm5, xmm6		; 角度が小さい場合には表示しない
		mulps	xmm0, xmm0
		movaps	xmm1, xmm0
		movhlps	xmm2, xmm0
		shufps	xmm1, xmm1, 1
		addss	xmm0, xmm2
		shufps	xmm5, xmm5, 3
		addss	xmm0, xmm1
		sqrtss	xmm0, xmm0
		movss	xmm3, xmmMaskAbs
		divss	xmm5, xmm0
		;
;		.IF	[ebx].dwShadingFlags & E3DSAF_SINGLE_SIDE_PLANE
			mov	eax, [edx].dwIndex[0]
			shl	eax, 4
			add	eax, pMeshNormals
			movaps	xmm1, [eax]
			mulps	xmm6, xmm1
			movhlps	xmm2, xmm6
			addss	xmm2, xmm6
			shufps	xmm6, xmm6, 1
			addss	xmm6, xmm2
			mov	eax, plane.d
			movss	[ebx].dib.dwTemp[0], xmm6
			test	[ebx].dwShadingFlags, E3DSAF_SINGLE_SIDE_PLANE
			cmovz	eax, [ebx].dib.dwTemp[0]
			shufps	xmm6, xmm6, 0
			xor	eax, [ebx].dib.dwTemp[0]
			js	Label_Continue
			andps	xmm6, xmmMaskSign
			xorps	xmm6, [ebx].vTargetPlaneParam
			movaps	[ebx].vTargetPlaneParam, xmm6
;		.ENDIF
		;
		andps	xmm5, xmm3
		comiss	xmm5, rConstSinLittle
		jc	Label_Continue
		;
		;	基本パラメータ計算
		; ------------------------------------------------------------
		.IF	[ebx].dwShadingFlags & E3DSAF_TEXTURE_MAPPING
			;
			; UV 座標複製
			;
			movss	xmm4, [ebx].rTextureUVScale
			mov	esi, pMeshUVMaps
			mov	edi, pUVMap
			mov	eax, [edx].dwIndex[0]
			shufps	xmm4, xmm4, 0
			movlps	xmm0, QWORD PTR [esi + eax*8]
			mov	eax, [edx].dwIndex[4]
			movhps	xmm0, QWORD PTR [esi + eax*8]
			mov	eax, [edx].dwIndex[8]
			mulps	xmm0, xmm4
			movaps	[edi], xmm0
			movlps	xmm0, QWORD PTR [esi + eax*8]
			mulps	xmm0, xmm4
			movlps	QWORD PTR [edi + 10H], xmm0
			;
			; テクスチャマッピング行列計算
			;
			INVOKE	eglRenderPoly@SetTextureParameterSSE ,
					ADDR txmap, pVertices, pUVMap
		.ENDIF
		;
		;	ｚクリッピング
		; ------------------------------------------------------------
		.IF	fZClipping != 0
			movss	xmm7, [ebx].rZMinClip	; xmm7 = min clip z
			ASSUME	ebx:NOTHING
			mov	ecx, dwVertexCount
			mov	esi, pVertices		; esi = src vertex
			mov	ebx, pVertexColors	; ebx = src color
			mov	eax, pNormals[0]
			mov	pNormals[4], eax
			shl	ecx, 4
			sub	esp, ecx
			and	esp, NOT 0FH
			mov	pVertexColors, esp
			lea	eax, [ecx * 2]
			sub	esp, eax
			and	esp, NOT 0FH
			mov	pVertices, esp
			sub	esp, eax
			and	esp, NOT 0FH
			mov	pNormals[0], esp
			;
			mov	eax, dwVertexCount
			mov	edx, pVertexColors	; edx = dst color
			mov	edi, pVertices		; edi = dst vertex
			mov	dwLoopCounter, eax
			movaps	xmm4, [esi + ecx - 10H]	; xmm4 = last vertex
			movq	mm4, QWORD PTR [ebx + eax*8 - 8]
			mov	eax, pNormals[4]	; eax = src normal
			movhlps	xmm5, xmm4
			movaps	xmm6, [eax + ecx - 10H]	; xmm6 = last normal
			mov	ecx, pNormals[0]	; ecx = dst normal
			;
			comiss	xmm5, xmm7
			jb	Label_ZClipLoop2
Label_ZClipLoop1:		movaps	xmm0, [esi]
				add	esi, 10H
				movq	mm0, QWORD PTR [ebx]
				add	ebx, 8
				movaps	xmm3, [eax]
				add	eax, 10H
				movhlps	xmm1, xmm0
				comiss	xmm1, xmm7
				.IF	!CARRY?
Label_ZClipLoop3:			movaps	[edi], xmm0
					add	edi, 10H
					movq	QWORD PTR [edx], mm0
					add	edx, 8
					movaps	[ecx], xmm3
					add	ecx, 10H
					movaps	xmm4, xmm0
					movq	mm4, mm0
					movaps	xmm6, xmm3
					dec	dwLoopCounter
					jnz	Label_ZClipLoop1
					jmp	Label_ExitZClip
				.ENDIF
				;
				call	Label_ZClipFunc
				;
				movaps	xmm4, xmm0
				movq	mm4, mm0
				movaps	xmm6, xmm3
				dec	dwLoopCounter
				jz	Label_ExitZClip
Label_ZClipLoop2:
				movaps	xmm0, [esi]
				add	esi, 10H
				movq	mm0, QWORD PTR [ebx]
				add	ebx, 8
				movaps	xmm3, [eax]
				add	eax, 10H
				movhlps	xmm1, xmm0
				comiss	xmm1, xmm7
				.IF	CARRY?
					movaps	xmm4, xmm0
					movq	mm4, mm0
					movaps	xmm6, xmm3
					dec	dwLoopCounter
					jnz	Label_ZClipLoop2
					jmp	Label_ExitZClip
				.ENDIF
				;
				call	Label_ZClipFunc
				jmp	Label_ZClipLoop3
Label_ZClipFunc:
			movss	xmm2, xmm1
			movhlps	xmm5, xmm4
			subss	xmm1, xmm7	; xmm1 = z1 - r
			subss	xmm2, xmm5	; xmm2 = z1 - z0
			divss	xmm1, xmm2	; xmm1 = (z1 - r) / (z1 - z0)
				pxor	mm1, mm1
				movq	mm2, mm0
				movq	mm3, mm0
				movq	mm6, mm4
				movq	mm7, mm4
			subps	xmm4, xmm0
				punpcklbw	mm2, mm1
			subps	xmm6, xmm3
				punpckhbw	mm3, mm1
			shufps	xmm1, xmm1, 0
				punpcklbw	mm6, mm1
				punpckhbw	mm7, mm1
			mulps	xmm4, xmm1	; xmm4 = (p0 - p1) * xmm1
				psubw		mm6, mm2
			mulps	xmm6, xmm1
				psubw		mm7, mm3
			mulss	xmm1, rConst512
				psllw		mm6, 7
				psllw		mm7, 7
			addps	xmm4, xmm0
				cvtps2pi	mm1, xmm1
				pshufw		mm1, mm1, 0
			addps	xmm6, xmm3
				pmulhw		mm6, mm1
				pmulhw		mm7, mm1
				paddw		mm6, mm2
				paddw		mm7, mm3
				packuswb	mm6, mm7
			movaps	[edi], xmm4
			add	edi, 10H
			movaps	[ecx], xmm6
			add	ecx, 10H
				movq	QWORD PTR [edx], mm6
				add	edx, 8
			;
			BYTE	0C3H	; ret

Label_ExitZClip:
			sub	edi, pVertices
			shr	edi, 4
			mov	dwVertexCount, edi
			;
			mov	ebx, hRenderPoly
			ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
		.ENDIF
		;
		;	透視変換・リージョン作成
		; ------------------------------------------------------------
		;
		; 透視変換
		;
		mov	eax, dwVertexCount
		shl	eax, 3
		sub	esp, eax
		and	esp, NOT 0FH
		mov	pVertices2D, esp
		;
		INVOKE	eglRenderPoly@ProjectScreenSSE ,
				ebx, pVertices2D, pVertices, dwVertexCount
		;
		; リージョンを作成する
		;
		INVOKE	eglNormalizePolygonRegion ,
			[ebx].dib.pRegion, ADDR [ebx].dib.rectClip,
			dwVertexCount, pVertices2D, pVertexColors, pNormals
		test	eax, eax
		jz	Label_Continue
		;
		;	レンダリングパラメータセットアップ
		; ------------------------------------------------------------
		.IF	[ebx].dwShadingFlags & E3DSAF_TEXTURE_MAPPING
			lea	esi, txmap
			INVOKE	eglRenderPoly@PrepareParameterWithTextureSSE
			test	eax, eax
			jnz	Label_Continue
		.ELSE
			lea	esi, plane
			INVOKE	eglRenderPoly@PrepareParameterWithoutTextureSSE
		.ENDIF
		;
		;	レンダリング実行
		; ------------------------------------------------------------
		mov	eax, [ebx].nRenderFuncType
		INVOKE	pfnRenderFuncTableSSE[eax*4] , ebx
Label_Continue:
		;
		;	次のポリゴンへ
		; ------------------------------------------------------------
		mov	edx, pNextMeshEntry
		ASSUME	edx:PTR E3D_PRIMITIVE_MESH_POLY
		inc	[ebx].nMeshPolygonIndex
		mov	ecx, nLeftPolyCount
		mov	eax, [edx].dwVertexCount
		dec	ecx
		mov	esi, [ebx].pMeshPolygonEntry
		lea	edx, [edx].dwIndex[eax*4]
		mov	nLeftPolyCount, ecx
		mov	pNextMeshEntry, edx
	.ENDW

	ASSUME	ebx:NOTHING
	emms
	mov	esp, dwSavedESP
	xor	eax, eax
	ret

eglRenderPoly@RenderPolygon_MeshSSE	ENDP

CodeSeg	ENDS

	END
