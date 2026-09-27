
; ----------------------------------------------------------------------------
;                          Entis Graphic Library
; ----------------------------------------------------------------------------
;   Copyright (c) 2002-2011 Leshade Entis, Entis-soft. Al rights reserved.
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

;
;	RGB <-> YUV 変換係数
; ----------------------------------------------------------------------------
ALIGN	10H
mmxCvtRGBtoYUV	LABEL	MMWORD
	SWORD	2048,  8192, -1365,   0	; { 1/8, 1/2, -1/12 } * 4000H * B
	SWORD	9557, -5461, -6372,   0	; { 7/12, -1/3, -7/18 } * 4000H * G
	SWORD	4779, -2731,  7737,   0	; { 7/24, -1/6, 17/36 } * 4000H * R
	SWORD	  0,   0,   0,   0

;	SWORD	 32, 128, -21,   0	; { 1/8, 1/2, -1/12 } * 100H * B
;	SWORD	149, -85,-100,   0	; { 7/12, -1/3, -7/18 } * 100H * G
;	SWORD	 75, -43, 121,   0	; { 7/24, -1/6, 17/36 } * 100H * R
;	SWORD	  0,   0,   0,   0

mmxCvtYUVtoRGB	LABEL	MMWORD
	SWORD	256, 256, 256,   0	; { 1.0, 1.0, 1.0 } * 100H * Y
	SWORD	448, -96,   0,   0	; { 7/4, -3/8, 0.0 } * 100H * U
	SWORD	  0,-192, 384,   0	; { 0.0, -3/4, 3/2 } * 100H * V
	SWORD	  0,   0,   0,   0

mmxCvtBaseYUV	LABEL	MMWORD
	SWORD	  0, 80H, 80H,   0

mmx00FFW	LABEL	MMWORD
	WORD	0FFH, 0FFH, 0FFH, 0FFH
mmxAlphaMask	LABEL	MMWORD
	DWORD	0FF000000H, 0FF000000H
mmx0101W	LABEL	MMWORD
	WORD	101H, 101H, 101H, 101H
mmxMaskRGBChannel	LABEL	MMWORD
	WORD	0FFFFH, 0FFFFH, 0FFFFH, 0000H
mmxAlphaChannelScale	LABEL	MMWORD
	WORD	0, 0, 0, 100H
mmxAlphaChannelBias	LABEL	MMWORD
	WORD	1, 1, 1, 0


ConstSeg	ENDS


; ----------------------------------------------------------------------------
;	コードセグメント
; ----------------------------------------------------------------------------

CodeSeg	SEGMENT	PARA READONLY FLAT 'CODE'

;
;	RGB から YUV へ変換
; ----------------------------------------------------------------------------
ALIGN	10H
eglRGBtoYUV	PROC	NEAR32 C USES ebx esi edi, pColor:PTR EGL_PALETTE

	mov	edi, pColor
	ASSUME	edi:PTR EGL_PALETTE

	.IF	ERI_EnabledProcessorType & ERI_USE_XMM_P3
		;
		; XMM 専用コード
		;
		movzx	eax, [edi].rgb.Blue
		movzx	ecx, [edi].rgb.Green
		movzx	edx, [edi].rgb.Red
		movd	mm0, eax
		movd	mm1, ecx
		movd	mm2, edx
		pshufw	mm0, mm0, 10000000B
		pshufw	mm1, mm1, 10000000B
		pshufw	mm2, mm2, 10000000B
		psllw	mm0, 2+5
		psllw	mm1, 2+5
		psllw	mm2, 2+5
		pmulhw	mm0, mmxCvtRGBtoYUV[0]
		movd	mm4, [edi].dwPixelCode
		pmulhw	mm1, mmxCvtRGBtoYUV[8]
		psrld	mm4, 24
		pmulhw	mm2, mmxCvtRGBtoYUV[16]
		pslld	mm4, 24
		paddsw	mm0, mm1
		paddsw	mm0, mm2
		psraw	mm0, 5
		paddsw	mm0, mmxCvtBaseYUV
		packuswb	mm0, mm0
		por	mm0, mm4
		movd	[edi].dwPixelCode, mm0
		emms
		xor	eax, eax
		ret
	.ELSE
		;
		; 486 互換コード
		;
		movzx	eax, [edi].rgb.Blue
		movzx	ebx, [edi].rgb.Green
		movzx	ecx, [edi].rgb.Red
		;
		mov	edx, eax
		imul	esi, ebx, 149
		shl	edx, 5
		add	edx, esi
		imul	esi, ecx, 75
		add	edx, esi
		test	edx, 0FFFF0000H
		.IF	!ZERO?
			mov	edx, 0FF00H
		.ENDIF
		mov	[edi].yuv.Y, dh
		;
		mov	edx, eax
		imul	esi, ebx, -85
		shl	edx, 7
		add	edx, esi
		imul	esi, ecx, -43
		lea	edx, [edx + esi + 8000H]
		test	edx, 0FFFF0000H
		.IF	!ZERO?
			setns	dh
			neg	dh
		.ENDIF
		mov	[edi].yuv.U, dh
		;
		imul	edx, eax, -21
		imul	esi, ebx, -100
		add	edx, esi
		imul	esi, ecx, 121
		lea	edx, [edx + esi + 8000H]
		test	edx, 0FFFF0000H
		.IF	!ZERO?
			setns	dh
			neg	dh
		.ENDIF
		mov	[edi].yuv.V, dh
	.ENDIF

	ASSUME	edi:NOTHING
	xor	eax, eax
	ret

eglRGBtoYUV	ENDP

;
;	YUV から RGB へ変換
; ----------------------------------------------------------------------------
ALIGN	10H
eglYUVtoRGB	PROC	NEAR32 C USES ebx esi edi, pColor:PTR EGL_PALETTE

	LOCAL	dwTemp:DWORD

	mov	edi, pColor
	ASSUME	edi:PTR EGL_PALETTE

	.IF	0 ; ERI_EnabledProcessorType & ERI_USE_XMM_P3
		;
		; XMM 専用コード
		;
		movzx	ecx, [edi].yuv.U
		movzx	edx, [edi].yuv.V
		movzx	eax, [edi].yuv.Y
		sub	ecx, 80H
		sub	edx, 80H
		movd	mm0, eax
		movd	mm1, ecx
		movd	mm2, edx
		pshufw	mm1, mm1, 10000000B
		pshufw	mm2, mm2, 10000000B
		movd	mm4, [edi].dwPixelCode
		pmullw	mm1, mmxCvtYUVtoRGB[8]
		psrld	mm4, 24
		pmullw	mm2, mmxCvtYUVtoRGB[16]
		pslld	mm4, 24
		pshufw	mm0, mm0, 10000000B
		psraw	mm1, 8
		psraw	mm2, 8
		paddsw	mm0, mm1
		paddsw	mm0, mm2
		packuswb	mm0, mm0
		por	mm0, mm4
		movd	[edi].dwPixelCode, mm0
		emms
		xor	eax, eax
		ret
	.ELSE
		;
		; 486 互換コード
		;
		movzx	edx, [edi].yuv.V
		movzx	ecx, [edi].yuv.U
		movzx	eax, [edi].yuv.Y		; eax = Y
		lea	esi, [edx + edx * 2 - 180H]	; esi = V * 3
		lea	ebx, [ecx + ecx * 2 - 180H]	; ebx = U * 3
		mov	dwTemp, eax
		sar	esi, 1
		adc	eax, esi
		test	eax, 0FFFFFF00H
		.IF	!ZERO?
			setns	al
			neg	al
		.ENDIF
		mov	[edi].rgb.Red, al
		;
		mov	eax, dwTemp
		sar	esi, 1
		sbb	eax, esi
		lea	esi, [ebx + ecx * 4 - 200H]	; esi = U * 7
		sar	ebx, 3
		sbb	eax, ebx
		test	eax, 0FFFFFF00H
		.IF	!ZERO?
			setns	al
			neg	al
		.ENDIF
		mov	[edi].rgb.Green, al
		;
		mov	eax, dwTemp
		sar	esi, 2
		adc	eax, esi
		test	eax, 0FFFFFF00H
		.IF	!ZERO?
			setns	al
			neg	al
		.ENDIF
		mov	[edi].rgb.Blue, al
	.ENDIF

	ASSUME	edi:NOTHING
	xor	eax, eax
	ret

eglYUVtoRGB	ENDP

;
;	RGB から HSB へ変換
; ----------------------------------------------------------------------------
ALIGN	10H
eglRGBtoHSB	PROC	NEAR32 C USES ebx esi edi, pColor:PTR EGL_PALETTE

	Local	nRed:DWord, nGreen:DWord, nBlue:DWord
	Local	nSaturation:DWord, nBrightness:DWord
	Local	maxC:DWord, minC:DWord

	;
	;	関数準備
	; --------------------------------------------------------------------
	mov	edi, pColor
	ASSUME	edi:PTR EGL_PALETTE
	movzx	eax, [edi].rgb.Red
	movzx	ecx, [edi].rgb.Green
	movzx	edx, [edi].rgb.Blue
	mov	nRed, eax
	mov	nGreen, ecx
	mov	nBlue, edx

	;
	;	最大値と最小値を取得
	; --------------------------------------------------------------------
	.IF	eax < ecx
		xchg	eax, ecx
	.ENDIF
	.IF	ecx < edx
		xchg	ecx, edx
		.IF	eax < ecx
			xchg	eax, ecx
		.ENDIF
	.ENDIF
	mov	maxC, eax
	mov	minC, edx

	;
	;	計算：128 * maxC + 127 * minC
	; --------------------------------------------------------------------
	mov	ecx, edx
	shl	edx, 7
	shl	eax, 7
	sub	edx, ecx
	add	eax, edx

	;
	;	輝度と彩度を計算
	; --------------------------------------------------------------------
	.IF	eax <= (128 * 255)
		;
		; 輝度 = (128 * maxC + 127 * minC) / 255
		;
		mov	edx, eax	; eax = eax / 255
		shl	eax, 8
		add	eax, edx
		shr	eax, 16
		adc	eax, 0
		mov	[edi].hsb.Brightness, al
		mov	nBrightness, eax
		;
		; 彩度 = (minC - Brightness) / (-128 * Brightness) * 128
		;      = (Brightness - minC) / Brightness
		;
		mov	ecx, minC
		.IF	(eax != 0) && (ecx != maxC)
			mov	ecx, eax
			sub	eax, minC
			mov	edx, eax
			shl	eax, 8
			sub	eax, edx
			cdq
			idiv	ecx
		.ELSE
			xor	eax, eax
		.ENDIF
	.ELSE
		;               128 * maxC + 127 * minC - 255*255
		; Brightness = ----------------------------------- * 128 + 255
		;                           255 * 127
		mov	ecx, 255 * 127
		sub	eax, 255 * 255
		shl	eax, 7
		cdq
		idiv	ecx
		add	eax, 255
		mov	[edi].hsb.Brightness, al
		mov	nBrightness, eax
		;               128 * minC - 255 - 127 * Brihtness
		; Saturation = ------------------------------------
		;                    -128 * (255 - Brightness)
		mov	ecx, minC
		.IF	(eax != 255) && (ecx != maxC)
			mov	edx, eax
			shl	eax, 7
			mov	ecx, minC
			sub	eax, edx
			shl	ecx, 7
			sub	ecx, eax
			mov	eax, nBrightness
			sub	eax, 255
			shl	eax, 7
			xchg	eax, ecx
			shl	eax, 8
			cdq
			idiv	ecx
		.ELSE
			xor	eax, eax
		.ENDIF
	.ENDIF
	mov	[edi].hsb.Saturation, al
	mov	nSaturation, eax

	;
	;	輝度成分を RGB から除去する
	; --------------------------------------------------------------------
	mov	ecx, nBrightness
	.IF	ecx <= 128
	.IF	ecx != 0
		;
		; Color = 128 * Color' / Brightness
		;
		mov	eax, 128 * 100H
		xor	edx, edx
		div	ecx
		shr	ecx, 1
		cmp	ecx, edx
		adc	eax, 0
		mov	ecx, eax
		;
		FOR	@TEMP, <nRed, nGreen, nBlue>
			mov	eax, @TEMP
			imul	eax, ecx
			shr	eax, 8
			mov	@TEMP, eax
		ENDM
	.ENDIF
	.ELSE ;	ecx >= 128
	.IF	ecx != 256
		;
		; Color = (Color' - 255) / (255 - Brightness) * 128 + 256
		;
		neg	ecx
		mov	eax, 128 * 100H
		add	ecx, 255
		xor	edx, edx
		div	ecx
		shr	ecx, 1
		cmp	ecx, edx
		adc	eax, 0
		mov	ecx, eax
		;
		FOR	@TEMP, <nRed, nGreen, nBlue>
			mov	eax, @TEMP
			sub	eax, 0FFH
			imul	eax, ecx
			sar	eax, 8
			add	eax, 0FFH
			mov	@TEMP, eax
		ENDM
	.ENDIF
	.ENDIF

	;
	;	彩度成分を RGB から除去する
	; --------------------------------------------------------------------
	mov	ecx, nSaturation
	.IF	ecx != 0
		;
		; Color' = (Color - 128) / Saturation + 128
		;
		mov	eax, 0FF00H
		xor	edx, edx
		div	ecx
		shr	ecx, 1
		cmp	ecx, edx
		adc	eax, 0
		mov	ecx, eax
		;
		FOR	@TEMP, <nRed, nGreen, nBlue>
			mov	eax, @TEMP
			sub	eax, 128
			imul	eax, ecx
			sar	eax, 8
			adc	eax, 128
			mov	@TEMP, eax
		ENDM
	.ENDIF

	;
	;	色相を計算する
	; --------------------------------------------------------------------
	;
	;	  0 :  43 :  85 : 128 : 171 : 213 : 256
	;	 R ------------>        <----------- R
	;	 <---------- G ------------>
	;	            <----------- B ----------->
	;
	mov	eax, nRed
	mov	ecx, nGreen
	mov	edx, nBlue
	.IF	(SDWORD PTR eax) < (SDWORD PTR ecx)
		.IF	(SDWORD PTR ecx) < (SDWORD PTR edx)
			mov	edx, ecx
			mov	ecx, eax
			mov	eax, 171
		.ELSE
			mov	ecx, edx
			mov	edx, eax
			mov	eax, 85
		.ENDIF
	.ELSE
		.IF	(SDWORD PTR eax) < (SDWORD PTR edx)
			mov	edx, ecx
			mov	ecx, eax
			mov	eax, 171
		.ELSE
			xor	eax, eax
		.ENDIF
	.ENDIF
	;
	.IF	(SDWORD PTR ecx) >= (SDWORD PTR edx)
		imul	ecx, 43
		shr	ecx, 8
		adc	eax, ecx
	.ELSE
		imul	edx, 43
		sar	edx, 8
		sbb	eax, edx
	.ENDIF
	;
	mov	[edi].hsb.Hue, al

	ASSUME	edi:NOTHING
	xor	eax, eax
	ret

eglRGBtoHSB	ENDP

;
;	HSB から RGB へ変換
; ----------------------------------------------------------------------------

@eglHSBtoRGB_HueToCode	MACRO
	mov	ecx, eax
	not	ecx
	sar	ecx, 31
	xor	eax, ecx
	sub	eax, ecx
	add	eax, 86
	imul	eax, (0FF00H / 43)
	sar	eax, 8
	.IF	SIGN?
		xor	eax, eax
	.ELSEIF	eax >= 100H
		mov	eax, 0FFH
	.ENDIF
ENDM

ALIGN	10H
eglHSBtoRGB	PROC	NEAR32 C USES ebx esi edi, pColor:PTR EGL_PALETTE

	Local	nRed:DWord, nGreen:DWord, nBlue:DWord

	mov	edi, pColor
	ASSUME	edi:PTR EGL_PALETTE

	;
	;	色相を RGB へ変換する
	; --------------------------------------------------------------------
	;
	;	  0 :  43 :  85 : 128 : 171 : 213 : 256
	;	 R ------>              <----------- R
	;	 <---------- G ------------>
	;	            <----------- B ----------->
	;
	movzx	edx, [edi].hsb.Hue
	;
	; 赤色
	;
	.IF	edx <= 128
		mov	eax, edx
	.ELSE
		mov	eax, 256
		sub	eax, edx
	.ENDIF
	@eglHSBtoRGB_HueToCode
	sub	eax, 128
	mov	nRed, eax
	;
	; 緑色
	;
	lea	eax, [edx - 85]
	@eglHSBtoRGB_HueToCode
	sub	eax, 128
	mov	nGreen, eax
	;
	; 青色
	;
	lea	eax, [edx - 170]
	@eglHSBtoRGB_HueToCode
	sub	eax, 128
	mov	nBlue, eax

	;
	;	彩度を適用
	; --------------------------------------------------------------------
	movzx	ecx, [edi].hsb.Saturation
	FOR	@TEMP, <nRed, nGreen, nBlue>
		mov	eax, @TEMP
		imul	eax, ecx
		sar	eax, 8
		add	eax, 128
		mov	@TEMP, eax
	ENDM

	;
	;	輝度を適用
	; --------------------------------------------------------------------
	movzx	edx, [edi].hsb.Brightness
	.IF	edx <= 128
		FOR	@TEMP, <Red, Green, Blue>
			mov	eax, n&@TEMP&
			imul	eax, edx
			shr	eax, 7
			mov	[edi].rgb.@TEMP, al
		ENDM
	.ELSE
		neg	edx
		add	edx, 0FFH
		FOR	@TEMP, <Red, Green, Blue>
			mov	eax, n&@TEMP&
			sub	eax, 0FFH
			imul	eax, edx
			shr	eax, 7
			add	eax, 0FFH
			mov	[edi].rgb.@TEMP, al
		ENDM
	.ENDIF

	ASSUME	edi:PTR EGL_PALETTE
	xor	eax, eax
	ret

eglHSBtoRGB	ENDP

;
;	RGBA カラーコードをブレンドする
; ----------------------------------------------------------------------------
ALIGN	10H
eglBlendRGBA	PROC	NEAR32 C USES ebx esi edi,
			rgba1:EGL_PALETTE, rgba2:EGL_PALETTE

	LOCAL	rgbaResult:EGL_PALETTE

	.IF	ERI_EnabledProcessorType & ERI_USE_XMM_P3
		;
		; XMM 専用コード
		;
		movd		mm4, rgba1
		movd		mm5, rgba2
		movq		mm0, mm4
		movq		mm1, mm5
		psrld		mm4, 24
		psrld		mm5, 24
		pxor		mm2, mm2
		movq		mm6, mmx00FFW
		movq		mm7, mmx0101W
		punpcklbw	mm0, mm2
		punpcklbw	mm1, mm2
		pxor		mm4, mm6
		pxor		mm5, mm6
		pshufw		mm3, mm5, 10000000B
		pmullw		mm0, mm3
		pmullw		mm4, mm5
		pmulhuw		mm0, mm7
		pmulhuw		mm4, mm7
		paddusw		mm0, mm5
		pxor		mm4, mm6
		packuswb	mm0, mm2
		pslld		mm4, 24
		por		mm0, mm4
		movd		eax, mm0
		emms
		ret
	.ELSE
		;
		; 486 互換コード
		;
		movzx	ecx, rgba2.rgba.Alpha
		xor	ecx, 0FFH
		;
		FOR	@TEMP, <Red, Green, Blue>
			movzx	eax, rgba1.rgba.@TEMP
			mov	dl, rgba2.rgba.@TEMP
			imul	eax, ecx
			shr	eax, 8
			adc	al, dl
			sbb	dh, dh
			or	al, dh
			mov	rgbaResult.rgba.@TEMP, al
		ENDM
		;
		movzx	eax, rgba1.rgba.Alpha
		xor	eax, 0FFH
		imul	eax, ecx
		mov	edx, rgbaResult.dwPixelCode
		shr	eax, 8
		adc	eax, 0
		and	edx, 00FFFFFFH
		xor	eax, 0FFH
		shl	eax, 24
		or	eax, edx
	.ENDIF

	ret

eglBlendRGBA	ENDP

;
;	フォーマット変換
; ----------------------------------------------------------------------------

@CvtPixelFunc	TYPEDEF	PROTO	NEAR32 C, pColor:PTR EGL_PALETTE
@PCvtPixelFunc	TYPEDEF	PTR @CvtPixelFunc

ALIGN	10H
eglConvertFormat	PROC	NEAR32 C USES ebx esi edi,
	pDstImage:PEGL_IMAGE_INFO, pSrcImage:PCEGL_IMAGE_INFO, dwFlags:DWORD

	LOCAL	pfnCvtPixelFunc:@PCvtPixelFunc
	LOCAL	palPixelCode:EGL_PALETTE, palAlpha:EGL_PALETTE, nRandomSeed:DWORD
	LOCAL	nFirstPal:DWORD, nFirstDist:DWORD
	LOCAL	nSecondPal:DWORD, nSecondDist:DWORD
	LOCAL	fdwSrcFormatType:DWORD, fdwDstFormatType:DWORD
	LOCAL	nImageWidth:DWORD, nImageHeight:DWORD
	LOCAL	nSrcPixelBits:DWORD, nDstPixelBits:DWORD
	LOCAL	nSrcClippedPixel:DWORD
	LOCAL	nSrcLineBytes:SDWORD, nDstLineBytes:SDWORD
	LOCAL	ptrSrcLine:PVOID, ptrDstLine:PVOID
	LOCAL	nLeftHeight:DWORD, nLeftWidth:DWORD

	mov	edi, pDstImage
	mov	esi, pSrcImage
	ASSUME	edi:PEGL_IMAGE_INFO
	ASSUME	esi:PCEGL_IMAGE_INFO
	;
	; 画像サイズの検証
	;
	mov	ecx, [edi].dwImageWidth
	mov	edx, [edi].dwImageHeight
	.IF	(ecx != [esi].dwImageWidth) || (edx != [esi].dwImageHeight)
		TRACE	<"画像サイズが一致しません。", 0AH>
		mov	eax, eslErrGeneral
		ret
	.ENDIF
	mov	nImageWidth, ecx
	mov	nImageHeight, edx
	;
	; 色空間の変換か？
	;
	mov	eax, [edi].fdwFormatType
	mov	edx, [esi].fdwFormatType
	mov	fdwDstFormatType, eax
	mov	fdwSrcFormatType, edx
	and	eax, EIF_TYPE_MASK
	and	edx, EIF_TYPE_MASK
	.IF	(eax == EIF_GRAY_BITMAP) || (edx == EIF_GRAY_BITMAP)
		;
		;	色空間変換（グレイスケール）
		; ------------------------------------------------------------
		.IF	(eax == EIF_GRAY_BITMAP) && (edx == EIF_RGB_BITMAP)
		;
		; ビット深度検証
		;
		mov	ecx, [edi].dwBitsPerPixel
		mov	edx, [esi].dwBitsPerPixel
		.IF	(ecx == 8) && ((edx == 24) || (edx == 32))
		;
		shr	ecx, 3
		shr	edx, 3
		mov	nDstPixelBits, ecx
		mov	nSrcPixelBits, edx
		;
		; 変換準備
		;
		mov	eax, [edi].dwBytesPerLine
		mov	edx, [esi].dwBytesPerLine
		mov	nDstLineBytes, eax
		mov	nSrcLineBytes, edx
		mov	edi, [edi].ptrImageArray
		mov	esi, [esi].ptrImageArray
		PUSHCONTEXT	ASSUMES
		ASSUME	edi:NOTHING
		ASSUME	esi:NOTHING
		;
		; 変換ループ
		;
		mov	ecx, nImageHeight
		or	ecx, ecx
		.WHILE	!ZERO?
			mov	nLeftHeight, ecx
			mov	ptrSrcLine, esi
			mov	ptrDstLine, edi
			mov	ecx, nImageWidth
			or	ecx, ecx
			.WHILE	!ZERO?
				mov	nLeftWidth, ecx
				;
				movzx	eax, BYTE PTR [esi + 1]
				movzx	edx, BYTE PTR [esi + 2]
				lea	eax, [eax + eax * 2]
				shl	edx, 1
				add	eax, edx
				movzx	edx, BYTE PTR [esi]
				add	esi, nSrcPixelBits
				add	eax, edx
				;
				imul	eax, 2AAAH
				add	eax, 8000H
				shr	eax, 16
				;
				mov	BYTE PTR [edi], al
				inc	edi
				;
				mov	ecx, nLeftWidth
				dec	ecx
			.ENDW
			mov	esi, ptrSrcLine
			mov	edi, ptrDstLine
			mov	ecx, nLeftHeight
			add	esi, nSrcLineBytes
			add	edi, nDstLineBytes
			dec	ecx
		.ENDW
		;
		POPCONTEXT	ASSUMES
		;
		xor	eax, eax
		ret
		;
		.ENDIF
		.ENDIF

	.ELSEIF	(eax != EIF_RGB_BITMAP) || (edx != EIF_RGB_BITMAP)
		;
		;	色空間変換
		; ------------------------------------------------------------
		.IF	(eax != EIF_RGB_BITMAP) && (edx != EIF_RGB_BITMAP)
			TRACE	<"対応していない色空間変換です。", 0AH>
			mov	eax, eslErrGeneral
			ret
		.ENDIF
		;
		; 変換アルゴリズム設定
		;
		.IF	eax == EIF_YUV_BITMAP
			mov	edx, OFFSET eglRGBtoYUV
		.ELSEIF	eax == EIF_HSB_BITMAP
			mov	edx, OFFSET eglRGBtoHSB
		.ELSEIF	edx == EIF_YUV_BITMAP
			mov	edx, OFFSET eglYUVtoRGB
		.ELSEIF	edx == EIF_HSB_BITMAP
			mov	edx, OFFSET eglHSBtoRGB
		.ELSE
			TRACE	<"対応していない色空間変換です。", 0AH>
			mov	eax, eslErrGeneral
			ret
		.ENDIF
		mov	pfnCvtPixelFunc, edx
		;
		; ビット深度検証
		;
		mov	ecx, [edi].dwBitsPerPixel
		mov	edx, [esi].dwBitsPerPixel
		.IF	((ecx != 24) && (ecx != 32)) \
				|| ((edx != 24) && (edx != 32))
			TRACE	<"色空間変換は24ビット精度でなければなりません。", 0AH>
			mov	eax, eslErrGeneral
			ret
		.ENDIF
		mov	nDstPixelBits, ecx
		mov	nSrcPixelBits, edx
		;
		; 変換準備
		;
		mov	eax, [edi].dwBytesPerLine
		mov	edx, [esi].dwBytesPerLine
		mov	nDstLineBytes, eax
		mov	nSrcLineBytes, edx
		mov	edi, [edi].ptrImageArray
		mov	esi, [esi].ptrImageArray
		PUSHCONTEXT	ASSUMES
		ASSUME	edi:NOTHING
		ASSUME	esi:NOTHING
		;
		; 変換ループ
		;
		mov	ecx, nImageHeight
		or	ecx, ecx
		.WHILE	!ZERO?
			mov	nLeftHeight, ecx
			mov	ptrSrcLine, esi
			mov	ptrDstLine, edi
			mov	ecx, nImageWidth
			or	ecx, ecx
			.WHILE	!ZERO?
				mov	nLeftWidth, ecx
				;
				.IF	nSrcPixelBits == 24
					movzx	eax, WORD PTR [esi]
					movzx	edx, BYTE PTR [esi + 2]
					add	esi, 3
					shl	edx, 16
					or	eax, edx
				.ELSE
					mov	eax, DWORD PTR [esi]
					add	esi, 4
				.ENDIF
				mov	palPixelCode, eax
				;
				INVOKE	pfnCvtPixelFunc , ADDR palPixelCode
				;
				mov	eax, palPixelCode
				.IF	nDstPixelBits == 24
					mov	WORD PTR [edi], ax
					shr	eax, 16
					mov	BYTE PTR [edi], al
					add	edi, 3
				.ELSE
					mov	DWORD PTR [edi], eax
					add	edi, 4
				.ENDIF
				;
				mov	ecx, nLeftWidth
				dec	ecx
			.ENDW
			mov	esi, ptrSrcLine
			mov	edi, ptrDstLine
			mov	ecx, nLeftHeight
			add	esi, nSrcLineBytes
			add	edi, nDstLineBytes
			dec	ecx
		.ENDW
		;
		POPCONTEXT	ASSUMES
		;
		xor	eax, eax
		ret
	.ENDIF
	;
	; ビット深度変換
	;
	mov	ecx, [edi].dwBitsPerPixel
	mov	edx, [esi].dwBitsPerPixel
	mov	nDstPixelBits, ecx
	mov	nSrcPixelBits, edx
	.IF	(edx == 8) && ((ecx == 24) || (ecx == 32))
		;
		;	INDEX8 -> { RGB24 | RGB32 } の変換
		; ------------------------------------------------------------
		;
		; 変換準備
		;
		mov	eax, [esi].dwClippedPixel
		mov	nSrcClippedPixel, eax
		mov	eax, [edi].dwBytesPerLine
		mov	edx, [esi].dwBytesPerLine
		mov	nDstLineBytes, eax
		mov	nSrcLineBytes, edx
		mov	ebx, [esi].pPaletteEntries
		mov	edi, [edi].ptrImageArray
		mov	esi, [esi].ptrImageArray
		PUSHCONTEXT	ASSUMES
		ASSUME	edi:NOTHING
		ASSUME	esi:NOTHING
		;
		.IF	ebx == NULL
			TRACE	<"パレットテーブルがありません。", 0AH>
			mov	eax, eslErrGeneral
			ret
		.ENDIF
		;
		; 変換ループ
		;
		mov	edx, nImageHeight
		or	edx, edx
		.WHILE	!ZERO?
			mov	nLeftHeight, edx
			mov	ptrSrcLine, esi
			mov	ptrDstLine, edi
			mov	edx, nImageWidth
			mov	ecx, nDstPixelBits
			.IF	ecx == 24
				or	edx, edx
				.WHILE	!ZERO?
					movzx	eax, BYTE PTR [esi]
					inc	esi
					mov	eax, [ebx + eax * 4]
					mov	WORD PTR [edi], ax
					shr	eax, 16
					mov	BYTE PTR [edi], al
					add	edi, 3
					dec	edx
				.ENDW
			.ELSEIF	(fdwDstFormatType & EIF_WITH_ALPHA)
				mov	ecx, -1
				.IF	fdwSrcFormatType & EIF_WITH_CLIPPING
					mov	ecx, nSrcClippedPixel
				.ENDIF
				or	edx, edx
				.WHILE	!ZERO?
					movzx	eax, BYTE PTR [esi]
					inc	esi
					cmp	eax, ecx
					.IF	ZERO?
						xor	eax, eax
					.ELSE
						mov	eax, [ebx + eax * 4]
						or	eax, 0FF000000H
					.ENDIF
					mov	DWORD PTR [edi], eax
					add	edi, 4
					dec	edx
				.ENDW
			.ELSE
				or	edx, edx
				.WHILE	!ZERO?
					movzx	eax, BYTE PTR [esi]
					inc	esi
					mov	eax, [ebx + eax * 4]
					mov	DWORD PTR [edi], eax
					add	edi, 4
					dec	edx
				.ENDW
			.ENDIF
			mov	esi, ptrSrcLine
			mov	edi, ptrDstLine
			mov	edx, nLeftHeight
			add	esi, nSrcLineBytes
			add	edi, nDstLineBytes
			dec	edx
		.ENDW
		;
		POPCONTEXT	ASSUMES
		;
		xor	eax, eax
		ret
	.ENDIF
	.IF	(edx == 16) && ((ecx == 24) || (ecx == 32))
		;
		;	RGB16 -> { RGB24 | RGB32 } の変換
		; ------------------------------------------------------------
		;
		; 変換準備
		;
		mov	eax, [edi].dwBytesPerLine
		mov	edx, [esi].dwBytesPerLine
		mov	nDstLineBytes, eax
		mov	nSrcLineBytes, edx
		mov	edi, [edi].ptrImageArray
		mov	esi, [esi].ptrImageArray
		PUSHCONTEXT	ASSUMES
		ASSUME	edi:NOTHING
		ASSUME	esi:NOTHING
		;
		; 変換ループ
		;
		mov	edx, nImageHeight
		or	edx, edx
		.WHILE	!ZERO?
			mov	nLeftHeight, edx
			mov	ptrSrcLine, esi
			mov	ptrDstLine, edi
			mov	edx, nImageWidth
			mov	ecx, nDstPixelBits
			shr	ecx, 3
			or	edx, edx
			.WHILE	!ZERO?
				movzx	eax, WORD PTR [esi]
				add	esi, 2
				mov	ebx, eax
				mov	edx, eax
				shl	eax, 9
				shl	ebx, 6
				shl	edx, 3
				and	eax, 0F80000H
				and	ebx,  00F800H
				and	edx,  0000F8H
				or	eax, ebx
				or	eax, edx
				mov	edx, eax
				shr	eax, 5
				and	eax, 070707H
				add	eax, edx
				.IF	ecx == 3
					mov	WORD PTR [edi], ax
					shr	eax, 16
					mov	BYTE PTR [edi], al
				.ELSE
					mov	DWORD PTR [edi], eax
				.ENDIF
				add	edi, ecx
				dec	edx
			.ENDW
			mov	esi, ptrSrcLine
			mov	edi, ptrDstLine
			mov	edx, nLeftHeight
			add	esi, nSrcLineBytes
			add	edi, nDstLineBytes
			dec	edx
		.ENDW
		;
		POPCONTEXT	ASSUMES
		;
		xor	eax, eax
		ret
	.ENDIF
	.IF	(ecx == 16) && ((edx == 24) || (edx == 32))
		;
		;	{ RGB24 | RGB32 } -> RGB16 の変換
		; ------------------------------------------------------------
		;
		; 変換準備
		;
		mov	eax, [edi].dwBytesPerLine
		mov	edx, [esi].dwBytesPerLine
		mov	nDstLineBytes, eax
		mov	nSrcLineBytes, edx
		mov	edi, [edi].ptrImageArray
		mov	esi, [esi].ptrImageArray
		PUSHCONTEXT	ASSUMES
		ASSUME	edi:NOTHING
		ASSUME	esi:NOTHING
		;
		; 変換ループ
		;
		mov	edx, nImageHeight
		mov	ebx, 87654321H		; 乱数用
		or	edx, edx
		.WHILE	!ZERO?
			mov	nLeftHeight, edx
			mov	ptrSrcLine, esi
			mov	ptrDstLine, edi
			mov	edx, nImageWidth
			or	edx, edx
			.WHILE	!ZERO?
				mov	nLeftWidth, edx
				;
				xor	edx, edx
				movzx	eax, BYTE PTR [esi]
				lea	ebx, [ebx + ebx * 8 + 843AB539H]
				shrd	ecx, eax, 3
				shr	eax, 3
				cmp	ebx, ecx
				adc	eax, 0
				cmp	eax, 1FH
				sbb	eax, 0
				or	edx, eax
				;
				movzx	eax, BYTE PTR [esi + 1]
				lea	ebx, [ebx + ebx * 8 + 843AB539H]
				shrd	ecx, eax, 3
				shr	eax, 3
				cmp	ebx, ecx
				adc	eax, 0
				cmp	eax, 1FH
				sbb	eax, 0
				shl	eax, 5
				or	edx, eax
				;
				movzx	eax, BYTE PTR [esi + 1]
				lea	ebx, [ebx + ebx * 8 + 843AB539H]
				shrd	ecx, eax, 3
				shr	eax, 3
				cmp	ebx, ecx
				adc	eax, 0
				cmp	eax, 1FH
				sbb	eax, 0
				mov	ecx, nSrcPixelBits
				shl	eax, 10
				shr	ecx, 3
				or	edx, eax
				;
				add	esi, ecx
				mov	WORD PTR [edi], dx
				;
				mov	edx, nLeftWidth
				add	edi, 2
				dec	edx
			.ENDW
			mov	esi, ptrSrcLine
			mov	edi, ptrDstLine
			mov	edx, nLeftHeight
			add	esi, nSrcLineBytes
			add	edi, nDstLineBytes
			dec	edx
		.ENDW
		;
		POPCONTEXT	ASSUMES
		;
		xor	eax, eax
		ret
	.ENDIF
	.IF	(edx == 4) || (edx == 1)
		;
		;	{ INDEX1 | INDEX4 } -> { * } の変換
		; ------------------------------------------------------------
		;
		; 変換準備
		;
		.IF	fdwSrcFormatType & EIF_WITH_CLIPPING
			mov	eax, [esi].dwClippedPixel
		.ELSE
			mov	eax, -1
		.ENDIF
		mov	nSrcClippedPixel, eax
		;
		mov	eax, [edi].dwBytesPerLine
		mov	edx, [esi].dwBytesPerLine
		mov	nDstLineBytes, eax
		mov	nSrcLineBytes, edx
		mov	ebx, [esi].pPaletteEntries
		;
		.IF	ebx == NULL
			TRACE	<"パレットテーブルがありません。", 0AH>
			mov	eax, eslErrGeneral
			ret
		.ENDIF
		;
		; 変換ループ
		;
		xor	edx, edx
		.WHILE	edx < nImageHeight
			mov	nLeftHeight, edx
			;
			xor	edx, edx
			.WHILE	edx < nImageWidth
				mov	nLeftWidth, edx
				;
				INVOKE	eglGetPixel , esi, edx, nLeftHeight
				.IF	eax == nSrcClippedPixel
					xor	eax, eax
				.ELSEIF	nDstPixelBits >= 24
					mov	eax, DWORD PTR [ebx + eax * 4]
					.IF	fdwDstFormatType & EIF_WITH_ALPHA
						or	eax, 0FF000000H
					.ENDIF
				.ENDIF
				;
				mov	palPixelCode, eax
				INVOKE	eglSetPixel ,
					edi, nLeftWidth, nLeftHeight, palPixelCode
				;
				mov	edx, nLeftWidth
				inc	edx
			.ENDW
			;
			mov	edx, nLeftHeight
			inc	edx
		.ENDW
		;
		xor	eax, eax
		ret
	.ENDIF
	.IF	((ecx == 24) || (ecx == 32)) && ((edx == 24) || (edx == 32))
		;
		;	{ RGB24 | RGB32 } -> { RGB24 | RGB32 } の複製
		; ------------------------------------------------------------
		;
		; 変換準備
		;
		mov	eax, [edi].dwBytesPerLine
		mov	edx, [esi].dwBytesPerLine
		mov	nDstLineBytes, eax
		mov	nSrcLineBytes, edx
		mov	edi, [edi].ptrImageArray
		mov	esi, [esi].ptrImageArray
		PUSHCONTEXT	ASSUMES
		ASSUME	edi:NOTHING
		ASSUME	esi:NOTHING
		;
		; 変換ループ
		;
		mov	edx, nImageHeight
		or	edx, edx
		.WHILE	!ZERO?
			mov	nLeftHeight, edx
			mov	ptrSrcLine, esi
			mov	ptrDstLine, edi
			mov	edx, nImageWidth
			mov	ebx, nSrcPixelBits
			mov	ecx, nDstPixelBits
			push	ebp
			.IF	!(fdwSrcFormatType & EIF_WITH_ALPHA)
				mov	ebp, 0FF000000H
			.ELSE
				xor	ebp, ebp
			.ENDIF
			or	edx, edx
			.WHILE	!ZERO?
				.IF	ebx == 24
					movzx	ebx, BYTE PTR [esi + 2]
					movzx	eax, WORD PTR [esi]
					shl	ebx, 16
					add	esi, 3
					or	eax, ebx
					mov	ebx, 24
				.ELSE
					mov	eax, DWORD PTR [esi]
					add	esi, 4
				.ENDIF
				or	eax, ebp
				.IF	ecx == 24
					mov	WORD PTR [edi], ax
					shr	eax, 16
					mov	BYTE PTR [edi + 2], al
					add	edi, 3
				.ELSE
					mov	DWORD PTR [edi], eax
					add	edi, 4
				.ENDIF
				dec	edx
			.ENDW
			pop	ebp
			mov	esi, ptrSrcLine
			mov	edi, ptrDstLine
			mov	edx, nLeftHeight
			add	esi, nSrcLineBytes
			add	edi, nDstLineBytes
			dec	edx
		.ENDW
		POPCONTEXT	ASSUMES
		;
		xor	eax, eax
		ret
	.ENDIF
	;
	;	汎用変換
	; --------------------------------------------------------------------
	;
	; 変換準備
	;
	.IF	fdwSrcFormatType & EIF_WITH_CLIPPING
		mov	eax, [esi].dwClippedPixel
	.ELSE
		mov	eax, -1
	.ENDIF
	mov	nSrcClippedPixel, eax
	;
	mov	eax, [edi].dwBytesPerLine
	mov	edx, [esi].dwBytesPerLine
	mov	nDstLineBytes, eax
	mov	nSrcLineBytes, edx
	;
	; 変換ループ
	;
	xor	edx, edx
	.WHILE	edx < nImageHeight
		mov	nLeftHeight, edx
		;
		xor	edx, edx
		.WHILE	edx < nImageWidth
			mov	nLeftWidth, edx
			;
			INVOKE	eglGetPixel , esi, edx, nLeftHeight
			mov	edx, fdwSrcFormatType
			mov	ecx, [esi].pPaletteEntries
			.IF	(edx & EIF_WITH_CLIPPING) \
					&& (eax == nSrcClippedPixel)
				xor	eax, eax
			.ELSEIF	ecx != NULL
				.IF	eax < [esi].dwPaletteCount
					mov	eax, [ecx + eax * 4]
					or	eax, 0FF000000H
				.ELSE
					xor	eax, eax
				.ENDIF
			.ELSEIF	!(edx & EIF_WITH_ALPHA)
				or	eax, 0FF000000H
			.ENDIF
			;
			mov	edx, fdwDstFormatType
			mov	ecx, [edi].pPaletteEntries
			mov	palPixelCode, eax
			;
			.IF	edx == EIF_GRAY_BITMAP
				mov	edx, eax
				movzx	ecx, ah
				shl	edx, 8
				movzx	eax, al
				shr	edx, 24
				lea	ecx, [ecx + ecx * 2]
				lea	eax, [eax + edx * 2]
				add	eax, ecx
				imul	eax, 43		; 100H / 6
				shr	eax, 8
				.IF	eax >= 100H
					mov	eax, 0FFH
				.ENDIF
				mov	palPixelCode, eax
			.ELSEIF	(edx & EIF_WITH_PALETTE) && (ecx != NULL)
				mov	nFirstPal, 0
				mov	nFirstDist, 3FFFFH
				mov	nSecondPal, 0
				mov	nSecondDist, 3FFFFH
				xor	edx, edx
				ASSUME	ecx:PTR EGL_PALETTE
				.WHILE	edx < [edi].dwPaletteCount
					movzx	eax, palPixelCode.rgb.Blue
					movzx	ebx, [ecx + edx * 4].rgb.Blue
					movzx	esi,  palPixelCode.rgb.Green
					sub	eax, ebx
					movzx	edi, [ecx + edx * 4].rgb.Green
					imul	eax, eax
					movzx	ebx, palPixelCode.rgb.Red
					sub	esi, edi
					movzx	edi, [ecx + edx * 4].rgb.Red
					imul	esi, esi
					sub	ebx, edi
					imul	ebx, ebx
					add	eax, esi
					add	eax, ebx
					.IF	eax < nSecondDist
						.IF	eax < nFirstDist
							mov	esi, nFirstDist
							mov	edi, nFirstPal
							mov	nFirstDist, eax
							mov	nFirstPal, edx
							mov	nSecondDist, esi
							mov	nSecondPal, edi
						.ELSE
							mov	nSecondDist, eax
							mov	nSecondPal, edx
						.ENDIF
					.ENDIF
					inc	edx
				.ENDW
				ASSUME	ecx:NOTHING
				mov	ecx, nRandomSeed
				mov	edi, pDstImage
				lea	ecx, [ecx + ecx * 4 + 351A29F3H]
				mov	esi, pSrcImage
				mov	nRandomSeed, ecx
				and	ecx, 3FFFFH
				.IF	ecx >= nFirstDist
					mov	eax, nFirstPal
				.ELSE
					mov	eax, nSecondPal
				.ENDIF
				mov	palPixelCode, eax
			.ENDIF
			;
			INVOKE	eglSetPixel ,
				edi, nLeftWidth, nLeftHeight, palPixelCode
			;
			mov	edx, nLeftWidth
			inc	edx
		.ENDW
		;
		mov	edx, nLeftHeight
		inc	edx
	.ENDW
	;
	xor	eax, eax
	ret

eglConvertFormat	ENDP

;
;	トーンテーブルを生成します
; ----------------------------------------------------------------------------
ALIGN	10H
eglCalculateToneTable	PROC	NEAR32 C USES ebx esi edi,
		pToneBuf:PTR, nTone:SDWORD, nFlag:DWORD

	mov	edx, nTone
	mov	edi, pToneBuf

	test	edx, edx
	.IF	ZERO? && (nFlag != EGL_TONE_INVERSION)
		;
		; 変換無しトーンテーブル
		;
		mov	eax, 03020100H
		mov	ecx, 100H / 4
		.REPEAT
			mov	DWORD PTR [edi], eax
			add	edi, 4
			add	eax, 04040404H
			dec	ecx
		.UNTIL	ZERO?
		xor	eax, eax
		ret

	.ELSEIF	nFlag == EGL_TONE_BRIGHTNESS
		;
		; 輝度トーン
		;
		test	edx, edx
		.IF	!SIGN?
			.IF	edx > 100H
				mov	edx, 100H
			.ENDIF
			lea	eax, [edx - 1]
			neg	edx
			mov	ecx, 100H
			shl	eax, 8
			add	edx, 100H
		.ELSE
			.IF	(SDWORD PTR edx) < (SDWORD PTR -100H)
				mov	edx, -100H
			.ENDIF
			add	edx, 100H
			xor	eax, eax
			mov	ecx, 100H
		.ENDIF

	.ELSEIF	nFlag == EGL_TONE_INVERSION
		;
		; 輝度反転トーン
		;
		.IF	(SDWORD PTR edx) > 100H
			mov	edx, 100H
		.ELSEIF	(SDWORD PTR edx) < -100H
			mov	edx, -100H
		.ENDIF
		mov	eax, edx
		sar	eax, 1
		add	eax, 80H
		test	edx, edx
		.IF	!SIGN?
			dec	eax
		.ENDIF
		neg	edx
		shl	eax, 8
		mov	ecx, 100H

	.ELSEIF	nFlag == EGL_TONE_LIGHT
		;
		; ライトトーン
		;
		xor	ecx, ecx
		mov	eax, edx
		test	eax, eax
		.IF	SIGN?
			.REPEAT
				mov	BYTE PTR [edi + ecx], 0
				inc	ecx
				inc	eax
				.BREAK	.IF	ZERO?
			.UNTIL	ecx >= 100H
		.ENDIF
		;
		.WHILE	ecx < 100H
			.BREAK	.IF	eax >= 100H
			mov	BYTE PTR [edi + ecx], al
			inc	ecx
			inc	eax
		.ENDW
		;
		.WHILE	ecx < 100H
			mov	BYTE PTR [edi + ecx], 0FFH
			inc	ecx
		.ENDW
		;
		xor	eax, eax
		ret

	.ELSE
		mov	eax, eslErrGeneral
		ret

	.ENDIF

	.REPEAT
		mov	BYTE PTR [edi], ah
		inc	edi
		add	eax, edx
		dec	ecx
	.UNTIL	ZERO?
	xor	eax, eax
	ret

eglCalculateToneTable	ENDP

;
;	トーンテーブルを反映する
; ----------------------------------------------------------------------------
ALIGN	10H
eglApplyToneTable	PROC	NEAR32 C USES ebx esi edi,
		pDstImage:PEGL_IMAGE_INFO, pSrcImage:PCEGL_IMAGE_INFO,
		pBlueTone:PTR, pGreenTone:PTR, pRedTone:PTR, pAlphaTone:PTR

	LOCAL	nImageWidth:DWORD, nImageHeight:DWORD
	LOCAL	nLeftWidth:DWORD, nLeftHeight:DWORD
	LOCAL	nSrcLineBytes:SDWORD, nDstLineBytes:SDWORD
	LOCAL	ptrSrcLine:PVOID, ptrDstLine:PVOID

	mov	esi, pSrcImage
	mov	edi, pDstImage
	ASSUME	esi:PEGL_IMAGE_INFO
	ASSUME	edi:PEGL_IMAGE_INFO
	;
	; 画像サイズを検証
	;
	mov	ecx, [esi].dwImageWidth
	mov	edx, [esi].dwImageHeight
	.IF	(ecx != [edi].dwImageWidth) || (edx != [edi].dwImageHeight)
		TRACE	<"画像サイズが一致しません。", 0AH>
		mov	eax, eslErrGeneral
		ret
	.ENDIF
	mov	nImageWidth, ecx
	mov	nImageHeight, edx
	;
	mov	ecx, [esi].dwBytesPerLine
	mov	edx, [edi].dwBytesPerLine
	mov	nSrcLineBytes, ecx
	mov	nDstLineBytes, edx
	;
	; ビット深度を検証
	;
	mov	ecx, [esi].dwBitsPerPixel
	.IF	ecx != [edi].dwBitsPerPixel
		TRACE	<"ビット深度が一致しません。", 0AH>
	.ENDIF
	;
	.IF	ecx == 8
		;
		;	グレイスケール変換
		; ------------------------------------------------------------
		mov	esi, [esi].ptrImageArray
		mov	edi, [edi].ptrImageArray
		mov	ebx, pBlueTone
		PUSHCONTEXT	ASSUMES
		mov	edx, nImageHeight
		or	edx, edx
		.WHILE	!ZERO?
			mov	ecx, nImageWidth
			mov	ptrSrcLine, esi
			mov	ptrDstLine, edi
			;
			or	ecx, ecx
			.WHILE	!ZERO?
				movzx	eax, BYTE PTR [esi]
				inc	esi
				mov	al, BYTE PTR [ebx + eax]
				mov	BYTE PTR [edi], al
				inc	edi
				dec	ecx
			.ENDW
			;
			mov	esi, ptrSrcLine
			mov	edi, ptrDstLine
			add	esi, nSrcLineBytes
			add	edi, nDstLineBytes
			dec	edx
		.ENDW
		POPCONTEXT	ASSUMES
		xor	eax, eax
		ret
	.ENDIF
	.IF	ecx == 24
		;
		;	RGB24 変換
		; ------------------------------------------------------------
		mov	esi, [esi].ptrImageArray
		mov	edi, [edi].ptrImageArray
		PUSHCONTEXT	ASSUMES
		mov	edx, nImageHeight
		or	edx, edx
		.WHILE	!ZERO?
			mov	nLeftHeight, edx
			mov	edx, nImageWidth
			mov	ptrSrcLine, esi
			mov	ptrDstLine, edi
			;
			or	edx, edx
			.WHILE	!ZERO?
				mov	nLeftWidth, edx
				;
				mov	ebx, pBlueTone
				movzx	eax, BYTE PTR [esi]
				mov	ecx, pGreenTone
				movzx	edx, BYTE PTR [esi + 1]
				mov	al, BYTE PTR [ebx + eax]
				mov	ebx, pRedTone
				mov	BYTE PTR [edi], al
				movzx	eax, BYTE PTR [esi + 2]
				add	esi, 3
				mov	dl, BYTE PTR [ecx + edx]
				mov	al, BYTE PTR [ebx + eax]
				mov	BYTE PTR [edi + 1], dl
				mov	BYTE PTR [edi + 2], al
				mov	edx, nLeftWidth
				add	edi, 3
				;
				dec	edx
			.ENDW
			;
			mov	esi, ptrSrcLine
			mov	edi, ptrDstLine
			mov	edx, nLeftHeight
			add	esi, nSrcLineBytes
			add	edi, nDstLineBytes
			dec	edx
		.ENDW
		POPCONTEXT	ASSUMES
		xor	eax, eax
		ret
	.ENDIF
	.IF	ecx == 32
		;
		;	RGBA32 変換
		; ------------------------------------------------------------
		mov	esi, [esi].ptrImageArray
		mov	edi, [edi].ptrImageArray
		PUSHCONTEXT	ASSUMES
		mov	edx, nImageHeight
		or	edx, edx
		.WHILE	!ZERO?
			mov	nLeftHeight, edx
			mov	edx, nImageWidth
			mov	ptrSrcLine, esi
			mov	ptrDstLine, edi
			;
			or	edx, edx
			.WHILE	!ZERO?
				mov	nLeftWidth, edx
				;
				mov	ebx, pBlueTone
				movzx	eax, BYTE PTR [esi]
				mov	ecx, pGreenTone
				movzx	edx, BYTE PTR [esi + 1]
				mov	al, BYTE PTR [ebx + eax]
				mov	ebx, pRedTone
				mov	BYTE PTR [edi], al
				movzx	eax, BYTE PTR [esi + 2]
				mov	dl, BYTE PTR [ecx + edx]
				mov	ecx, pAlphaTone
				mov	BYTE PTR [edi + 1], dl
				movzx	edx, BYTE PTR [esi + 3]
				add	esi, 4
				mov	al, BYTE PTR [ebx + eax]
				mov	dl, BYTE PTR [ecx + edx]
				mov	BYTE PTR [edi + 2], al
				mov	BYTE PTR [edi + 3], dl
				mov	edx, nLeftWidth
				add	edi, 4
				;
				dec	edx
			.ENDW
			;
			mov	esi, ptrSrcLine
			mov	edi, ptrDstLine
			mov	edx, nLeftHeight
			add	esi, nSrcLineBytes
			add	edi, nDstLineBytes
			dec	edx
		.ENDW
		POPCONTEXT	ASSUMES
		xor	eax, eax
		ret
	.ENDIF

	ASSUME	esi:NOTHING
	ASSUME	edi:NOTHING

	TRACE	<"未対応のフォーマットです。", 0AH>
	mov	eax, eslErrGeneral
	ret

eglApplyToneTable	ENDP

;
;	カラートーンを適用する
; ----------------------------------------------------------------------------
ALIGN	10H
eglSetColorTone		PROC	NEAR32 C USES ebx esi edi,
		pDstImage:PEGL_IMAGE_INFO, pSrcImage:PCEGL_IMAGE_INFO,
		nBlue:SDWORD, nGreen:SDWORD,
		nRed:SDWORD, nAlpha:SDWORD

	LOCAL	nBlueTone[100H]:BYTE
	LOCAL	nGreenTone[100H]:BYTE
	LOCAL	nRedTone[100H]:BYTE
	LOCAL	nAlphaTone[100H]:BYTE

	mov	esi, pSrcImage
	ASSUME	esi:PCEGL_IMAGE_INFO
	mov	eax, [esi].dwBitsPerPixel
	.IF	eax == 8
		INVOKE	eglCalculateToneTable , ADDR nBlueTone, nBlue, 0
	.ELSEIF	eax == 24
		INVOKE	eglCalculateToneTable , ADDR nBlueTone, nBlue, 0
		INVOKE	eglCalculateToneTable , ADDR nGreenTone, nGreen, 0
		INVOKE	eglCalculateToneTable , ADDR nRedTone, nRed, 0
	.ELSEIF	eax == 32
		INVOKE	eglCalculateToneTable , ADDR nBlueTone, nBlue, 0
		INVOKE	eglCalculateToneTable , ADDR nGreenTone, nGreen, 0
		INVOKE	eglCalculateToneTable , ADDR nRedTone, nRed, 0
		INVOKE	eglCalculateToneTable , ADDR nAlphaTone, nAlpha, 0
	.ENDIF
	ASSUME	esi:NOTHING

	INVOKE	eglApplyToneTable ,
			pDstImage, pSrcImage,
			ADDR nBlueTone, ADDR nGreenTone,
			ADDR nRedTone, ADDR nAlphaTone
	ret

eglSetColorTone		ENDP

;
;	画像をグレイスケール化する
; ----------------------------------------------------------------------------
ALIGN	10H
eglMakeGrayTone	PROC	NEAR32 C USES ebx esi edi,
	pDstImage:PEGL_IMAGE_INFO, pSrcImage:PCEGL_IMAGE_INFO

	LOCAL	nImageWidth:DWORD, nImageHeight:DWORD
	LOCAL	nLeftWidth:DWORD, nLeftHeight:DWORD
	LOCAL	nSrcLineBytes:SDWORD, nDstLineBytes:SDWORD
	LOCAL	nPixelPitch:DWORD
	LOCAL	ptrSrcLine:PVOID, ptrDstLine:PVOID

	mov	esi, pSrcImage
	mov	edi, pDstImage
	ASSUME	esi:PEGL_IMAGE_INFO
	ASSUME	edi:PEGL_IMAGE_INFO
	;
	; 画像サイズを検証
	;
	mov	ecx, [esi].dwImageWidth
	mov	edx, [esi].dwImageHeight
	.IF	(ecx != [edi].dwImageWidth) || (edx != [edi].dwImageHeight)
		TRACE	<"画像サイズが一致しません。", 0AH>
		mov	eax, eslErrGeneral
		ret
	.ENDIF
	mov	nImageWidth, ecx
	mov	nImageHeight, edx
	;
	mov	ecx, [esi].dwBytesPerLine
	mov	edx, [edi].dwBytesPerLine
	mov	nSrcLineBytes, ecx
	mov	nDstLineBytes, edx
	;
	; ビット深度を検証
	;
	mov	ecx, [esi].dwBitsPerPixel
	.IF	ecx != [edi].dwBitsPerPixel
		TRACE	<"ビット深度が一致しません。", 0AH>
	.ENDIF
	;
	.IF	ecx >= 24
		shr	ecx, 3
		mov	nPixelPitch, ecx
		;
		mov	esi, [esi].ptrImageArray
		mov	edi, [edi].ptrImageArray
		PUSHCONTEXT	ASSUMES
		mov	edx, nImageHeight
		or	edx, edx
		.WHILE	!ZERO?
			mov	nLeftHeight, edx
			mov	edx, nImageWidth
			mov	ptrSrcLine, esi
			mov	ptrDstLine, edi
			;
			or	edx, edx
			.WHILE	!ZERO?
				movzx	eax, BYTE PTR [esi + 1]
				movzx	ecx, BYTE PTR [esi + 2]
				lea	eax, [eax + eax * 2]
				shl	ecx, 1
				add	eax, ecx
				movzx	ecx, BYTE PTR [esi]
				add	esi, nPixelPitch
				add	eax, ecx
				;
				imul	eax, 2AAAH
				add	eax, 8000H
				shr	eax, 16
				;
				mov	BYTE PTR [edi], al
				mov	BYTE PTR [edi + 1], al
				mov	BYTE PTR [edi + 2], al
				add	edi, nPixelPitch
				;
				dec	edx
			.ENDW
			;
			mov	esi, ptrSrcLine
			mov	edi, ptrDstLine
			mov	edx, nLeftHeight
			add	esi, nSrcLineBytes
			add	edi, nDstLineBytes
			dec	edx
		.ENDW
		POPCONTEXT	ASSUMES
		xor	eax, eax
		ret
	.ENDIF

	ASSUME	esi:NOTHING
	ASSUME	edi:NOTHING

	TRACE	<"未対応のフォーマットです。", 0AH>
	mov	eax, eslErrGeneral
	ret

eglMakeGrayTone	ENDP

;
;	2 倍に拡大
; ----------------------------------------------------------------------------
ALIGN	10H
eglEnlargeDouble	PROC	NEAR32 C USES ebx esi edi,
	pDstImage:PEGL_IMAGE_INFO,
	pSrcImage:PCEGL_IMAGE_INFO, dwFlags:DWORD

	LOCAL	nImageWidth:DWORD, nImageHeight:DWORD
	LOCAL	nDstImageWidth:DWORD, nDstImageHeight:DWORD
	LOCAL	nLeftHeight:DWORD
	LOCAL	ptrSrcLine:PVOID, ptrDstLine:PVOID
	LOCAL	nDstLineBytes:SDWORD, nSrcLineBytes:SDWORD
	LOCAL	nBitsPerPixel:DWORD
	LOCAL	ptrDstImage:PVOID
	LOCAL	fHorzOddPixel:DWORD, fVertOddLine:DWORD

	mov	esi, pSrcImage
	mov	edi, pDstImage
	ASSUME	esi:PCEGL_IMAGE_INFO
	ASSUME	edi:PEGL_IMAGE_INFO

	;
	; パラメータ取得
	;
	mov	eax, [edi].dwImageWidth
	mov	ebx, [edi].dwImageHeight
	mov	ecx, [esi].dwImageWidth
	mov	edx, [esi].dwImageHeight
	mov	nDstImageWidth, eax
	mov	nDstImageHeight, ebx
	inc	eax
	inc	ebx
	mov	nImageWidth, ecx
	mov	nImageHeight, edx
	shr	eax, 1
	shr	ebx, 1
	.IF	(ecx != eax) || (edx != ebx)
		mov	eax, eslErrInvalidParam
		ret
	.ENDIF
	;
	mov	eax, [edi].dwImageWidth
	mov	ebx, [edi].dwImageHeight
	and	eax, 1
	and	ebx, 1
	mov	fHorzOddPixel, eax
	mov	fVertOddLine, ebx
	;
	mov	ecx, [esi].dwBitsPerPixel
	mov	nBitsPerPixel, ecx
	.IF	[edi].dwBitsPerPixel != ecx
		mov	eax, eslErrInvalidParam
		ret
	.ENDIF
	;
	mov	ecx, [esi].dwBytesPerLine
	mov	edx, [edi].dwBytesPerLine
	mov	nSrcLineBytes, ecx
	mov	nDstLineBytes, edx
	mov	esi, [esi].ptrImageArray
	mov	edi, [edi].ptrImageArray
	mov	ptrDstImage, edi
	;
	ASSUME	esi:NOTHING
	ASSUME	edi:NOTHING

	;
	;	水平方向拡大
	; --------------------------------------------------------------------
	mov	ecx, nImageHeight
	.REPEAT
		mov	nLeftHeight, ecx
		mov	ptrSrcLine, esi
		mov	ptrDstLine, edi
		;
		mov	eax, nBitsPerPixel
		mov	ecx, nImageWidth
		sub	ecx, fHorzOddPixel
		;
		.IF	eax == 32
			;
			; 32 ビットフォーマット
			;
			.IF	ERI_EnabledProcessorType & ERI_USE_XMM_P3
				prefetchnta	[esi]
				prefetchnta	[esi + 20H]
				movq	mm0, MMWORD PTR [esi]
				prefetchnta	[esi + 40H]
				sub	ecx, 4
				.WHILE	!SIGN?
					prefetchnta	[esi + 60H]
					movq	mm1, MMWORD PTR [esi]
					movq	mm4, MMWORD PTR [esi + 8]
					add	esi, 10H
					movq	mm5, mm1
					punpckldq	mm0, mm1
					psrlq	mm5, 32
					pavgb	mm0, mm1
					punpckldq	mm5, mm4
					movq	mm2, mm0
					pavgb	mm5, mm4
					punpckldq	mm0, mm1
					movq	mm6, mm5
					punpckldq	mm5, mm4
					punpckhdq	mm2, mm1
					punpckhdq	mm6, mm4
					movntq	MMWORD PTR [edi], mm0
					movntq	MMWORD PTR [edi + 08H], mm2
					movntq	MMWORD PTR [edi + 10H], mm5
					movntq	MMWORD PTR [edi + 18H], mm6
					movq	mm0, mm6
					add	edi, 20H
					psrlq	mm0, 32
					sub	ecx, 4
				.ENDW
				add	ecx, 4
			.ENDIF
			test	ecx, ecx
			.WHILE	!ZERO?
				mov	eax, DWORD PTR [esi]
				add	esi, 4
				mov	DWORD PTR [edi], eax
				mov	DWORD PTR [edi + 4], eax
				add	edi, 8
				dec	ecx
			.ENDW
			.IF	fHorzOddPixel != 0
				mov	eax, DWORD PTR [esi]
				mov	DWORD PTR [edi], eax
			.ENDIF

		.ELSEIF	eax == 24
			;
			; 24 ビットフォーマット
			;
			test	ecx, ecx
			.WHILE	!ZERO?
				mov	al, BYTE PTR [esi]
				mov	ah, BYTE PTR [esi + 1]
				mov	dl, BYTE PTR [esi + 2]
				add	esi, 3
				mov	BYTE PTR [edi], al
				mov	BYTE PTR [edi + 1], ah
				mov	BYTE PTR [edi + 2], dl
				mov	BYTE PTR [edi + 3], al
				mov	BYTE PTR [edi + 4], ah
				mov	BYTE PTR [edi + 5], dl
				add	edi, 6
				dec	ecx
			.ENDW
			.IF	fHorzOddPixel != 0
				mov	al, BYTE PTR [esi]
				mov	ah, BYTE PTR [esi + 1]
				mov	dl, BYTE PTR [esi + 2]
				mov	BYTE PTR [edi], al
				mov	BYTE PTR [edi + 1], ah
				mov	BYTE PTR [edi + 2], dl
			.ENDIF

		.ELSEIF	eax == 8
			;
			; 8 ビットフォーマット
			;
			test	ecx, ecx
			.WHILE	!ZERO?
				mov	al, BYTE PTR [esi]
				inc	esi
				mov	BYTE PTR [edi], al
				mov	BYTE PTR [edi + 1], al
				add	edi, 2
				dec	ecx
			.ENDW
			.IF	fHorzOddPixel != 0
				mov	al, BYTE PTR [esi]
				mov	BYTE PTR [edi], al
			.ENDIF

		.ELSEIF	eax == 16
			;
			; 16 ビットフォーマット
			;
			test	ecx, ecx
			.WHILE	!ZERO?
				mov	ax, WORD PTR [esi]
				add	esi, 2
				mov	WORD PTR [edi], ax
				mov	WORD PTR [edi + 2], ax
				add	edi, 4
				dec	ecx
			.ENDW
			.IF	fHorzOddPixel != 0
				mov	ax, WORD PTR [esi]
				mov	WORD PTR [edi], ax
			.ENDIF

		.ENDIF
		;
		mov	esi, ptrSrcLine
		mov	edi, ptrDstLine
		mov	eax, nDstLineBytes
		mov	ecx, nLeftHeight
		add	esi, nSrcLineBytes
		lea	edi, [edi + eax * 2]
		dec	ecx
	.UNTIL	ZERO?
	;
	;	垂直方向拡大
	; --------------------------------------------------------------------
	.IF	ERI_EnabledProcessorType & ERI_USE_XMM_P3
		sfence
	.ENDIF
	mov	ecx, nImageHeight
	mov	edi, ptrDstImage
	dec	ecx
	.WHILE	!ZERO?
		mov	nLeftHeight, ecx
		mov	ptrDstLine, edi
		;
		mov	esi, nDstLineBytes
		mov	eax, nBitsPerPixel
		mov	ecx, nDstImageWidth
		;
		.IF	eax == 32
			;
			; 32 ビットフォーマット
			;
			.IF	ERI_EnabledProcessorType & ERI_USE_XMM_P3
				prefetchnta	[edi]
				prefetcht0	[edi + esi * 2]
				prefetchnta	[edi + 20H]
				prefetcht0	[edi + esi * 2 + 20H]
				prefetchnta	[edi + 40H]
				prefetcht0	[edi + esi * 2 + 40H]
				sub	ecx, 8
				.WHILE	!SIGN?
					prefetchnta	[edi + 60H]
					movq	mm0, MMWORD PTR [edi]
					movq	mm1, MMWORD PTR [edi + 08H]
					movq	mm2, MMWORD PTR [edi + 10H]
					movq	mm3, MMWORD PTR [edi + 18H]
					prefetcht0	[edi + esi * 2 + 60H]
					movq	mm4, MMWORD PTR [edi + esi * 2]
					movq	mm5, MMWORD PTR [edi + esi * 2 + 08H]
					movq	mm6, MMWORD PTR [edi + esi * 2 + 10H]
					movq	mm7, MMWORD PTR [edi + esi * 2 + 18H]
					;
					pavgb	mm0, mm4
					pavgb	mm1, mm5
					pavgb	mm2, mm6
					pavgb	mm3, mm7
					;
					movntq	MMWORD PTR [edi + esi], mm0
					movntq	MMWORD PTR [edi + esi + 08H], mm1
					movntq	MMWORD PTR [edi + esi + 10H], mm2
					movntq	MMWORD PTR [edi + esi + 18H], mm3
					add	edi, 20H
					sub	ecx, 8
				.ENDW
				add	ecx, 8
			.ENDIF
			shl	ecx, 2

		.ELSEIF	eax == 24
			;
			; 24 ビットフォーマット
			;
			lea	ecx, [ecx + ecx * 2]

		.ELSEIF	eax == 8
			;
			; 8 ビットフォーマット
			;

		.ELSEIF	eax == 16
			;
			; 16 ビットフォーマット
			;
			shl	ecx, 1

		.ENDIF
		;
		sub	ecx, 8
		.WHILE	!SIGN?
			mov	eax, DWORD PTR [edi]
			mov	edx, DWORD PTR [edi + 4]
			mov	DWORD PTR [edi + esi], eax
			mov	DWORD PTR [edi + esi + 4], edx
			add	edi, 8
			sub	ecx, 8
		.ENDW
		add	ecx, 8
		.WHILE	!ZERO?
			mov	al, BYTE PTR [edi]
			mov	BYTE PTR [edi + esi], al
			inc	edi
			dec	ecx
		.ENDW
		;
		mov	edi, ptrDstLine
		mov	eax, nDstLineBytes
		mov	ecx, nLeftHeight
		add	esi, nSrcLineBytes
		lea	edi, [edi + eax * 2]
		dec	ecx
	.ENDW
	;
	;	最終ラインの複製
	; --------------------------------------------------------------------
	.IF	fVertOddLine == 0
		mov	esi, nDstLineBytes
		mov	eax, nBitsPerPixel
		mov	ecx, nDstImageWidth
		imul	ecx, eax
		shr	ecx, 3
		sub	ecx, 8
		.WHILE	!SIGN?
			mov	eax, DWORD PTR [edi]
			mov	edx, DWORD PTR [edi + 4]
			mov	DWORD PTR [edi + esi], eax
			mov	DWORD PTR [edi + esi + 4], edx
			add	edi, 8
			sub	ecx, 8
		.ENDW
		add	ecx, 8
		.WHILE	!ZERO?
			mov	al, BYTE PTR [edi]
			mov	BYTE PTR [edi + esi], al
			inc	edi
			dec	ecx
		.ENDW
	.ENDIF
	;
	.IF	ERI_EnabledProcessorType & ERI_USE_XMM_P3
		emms
	.ENDIF
	xor	eax, eax
	ret

eglEnlargeDouble	ENDP


;
;	半分に縮小
; ----------------------------------------------------------------------------
eglReduceHalf		PROC	NEAR32 C USES ebx esi edi,
	pDstImage:PEGL_IMAGE_INFO,
	pSrcImage:PCEGL_IMAGE_INFO, dwFlags:DWORD

	LOCAL	nImageWidth:DWORD, nImageHeight:DWORD
	LOCAL	nLeftHeight:DWORD
	LOCAL	ptrSrcLine:PVOID, ptrDstLine:PVOID
	LOCAL	nDstLineBytes:SDWORD, nSrcLineBytes:SDWORD
	LOCAL	nBitsPerPixel:DWORD
	LOCAL	fdwFormatType:DWORD

	mov	esi, pSrcImage
	mov	edi, pDstImage
	ASSUME	esi:PCEGL_IMAGE_INFO
	ASSUME	edi:PEGL_IMAGE_INFO

	;
	; パラメータ取得
	;
	mov	ecx, [esi].dwImageWidth
	mov	edx, [esi].dwImageHeight
	shr	ecx, 1
	shr	edx, 1
	mov	nImageWidth, ecx
	mov	nImageHeight, edx
	.IF	(ecx != [edi].dwImageWidth) || (edx != [edi].dwImageHeight)
		mov	eax, eslErrInvalidParam
		ret
	.ENDIF
	;
	mov	ecx, [esi].dwBitsPerPixel
	mov	nBitsPerPixel, ecx
	.IF	[edi].dwBitsPerPixel != ecx
		mov	eax, eslErrInvalidParam
		ret
	.ENDIF
	;
	mov	eax, [esi].fdwFormatType
	mov	ecx, [esi].dwBytesPerLine
	mov	edx, [edi].dwBytesPerLine
	mov	fdwFormatType, eax
	mov	nSrcLineBytes, ecx
	mov	nDstLineBytes, edx
	mov	esi, [esi].ptrImageArray
	mov	edi, [edi].ptrImageArray
	;
	ASSUME	esi:NOTHING
	ASSUME	edi:NOTHING

	;
	;	垂直方向ループ
	;
	mov	ecx, nImageHeight
	.REPEAT
		mov	nLeftHeight, ecx
		mov	ptrSrcLine, esi
		mov	ptrDstLine, edi
		;
		mov	eax, nBitsPerPixel
		mov	edx, nSrcLineBytes
		mov	ecx, nImageWidth
		.IF	eax == 32
			.IF	(fdwFormatType != EIF_Z_BUFFER_R4) \
				&& (ERI_EnabledProcessorType & ERI_USE_XMM_P3)
				pxor	mm7, mm7
				.REPEAT
					movq	mm0, QWORD PTR [esi]
					movq	mm2, QWORD PTR [esi + edx]
					movq	mm1, mm0
					movq	mm3, mm2
					punpcklbw	mm0, mm7
					punpckhbw	mm1, mm7
					punpcklbw	mm2, mm7
					punpckhbw	mm3, mm7
					paddw	mm0, mm1
					paddw	mm2, mm3
					paddw	mm0, mm2
					psrlw	mm0, 2
					add	esi, 4 * 2
					packuswb	mm0, mm0
					movd	DWORD PTR [edi], mm0
					add	edi, 4
					;
					dec	ecx
				.UNTIL	ZERO?
			.ELSE
				.REPEAT
					mov	eax, DWORD PTR [esi]
					add	esi, 4 * 2
					mov	DWORD PTR [edi], eax
					add	edi, 4
					dec	ecx
				.UNTIL	ZERO?
			.ENDIF

		.ELSEIF	eax == 24
			.REPEAT
				mov	ax, WORD PTR [esi]
				mov	dl, BYTE PTR [esi + 2]
				add	esi, 3 * 2
				mov	WORD PTR [edi], ax
				mov	BYTE PTR [edi + 2], dl
				add	edi, 3
				dec	ecx
			.UNTIL	ZERO?

		.ELSEIF	eax == 16
			.REPEAT
				mov	ax, WORD PTR [esi]
				add	esi, 2 * 2
				mov	WORD PTR [edi], ax
				add	edi, 2
				dec	ecx
			.UNTIL	ZERO?

		.ELSEIF	eax == 8
			.REPEAT
				mov	al, BYTE PTR [esi]
				add	esi, 1 * 2
				mov	BYTE PTR [edi], al
				inc	edi
				dec	ecx
			.UNTIL	ZERO?

		.ENDIF
		;
		mov	eax, nSrcLineBytes
		mov	esi, ptrSrcLine
		mov	edi, ptrDstLine
		mov	ecx, nLeftHeight
		lea	esi, [esi + eax * 2]
		add	edi, nDstLineBytes
		dec	ecx
	.UNTIL	ZERO?

	.IF	ERI_EnabledProcessorType & ERI_USE_XMM_P3
		emms
	.ENDIF

	xor	eax, eax
	ret

eglReduceHalf		ENDP


;
;	αチャネルを合成する
; ----------------------------------------------------------------------------
ALIGN	10H
eglBlendAlphaChannel	PROC	NEAR32 C USES ebx esi edi,
		pRGBA32:PEGL_IMAGE_INFO, pSrcRGB:PCEGL_IMAGE_INFO,
		pSrcAlpha:PCEGL_IMAGE_INFO, dwFlags:DWORD,
		nAlphaBase:SDWORD, nCoefficient:DWORD

	LOCAL	nImageWidth:DWORD, nImageHeight:DWORD
	LOCAL	nLeftWidth:DWORD, nLeftHeight:DWORD
	LOCAL	nSrcPixelBytes:DWORD
	LOCAL	nDstLineBytes:SDWORD, nSrcLineBytes:SDWORD
	LOCAL	nAlphaLineBytes:SDWORD
	LOCAL	ptrDstLine:PVOID, ptrSrcLine:PVOID, ptrAlphaLine:PVOID
	LOCAL	pfnSubFunc:DWORD
	LOCAL	fUsedSSE:DWORD
	LOCAL	mmxCoefficient:QWORD, mmxAlphaBase:QWORD

	mov	edi, pRGBA32
	mov	esi, pSrcRGB
	mov	ebx, pSrcAlpha
	ASSUME	edi:PEGL_IMAGE_INFO
	ASSUME	esi:PCEGL_IMAGE_INFO
	ASSUME	ebx:PCEGL_IMAGE_INFO
	.IF	(esi != NULL) && (ebx != NULL)
		;
		; 画像サイズ検証
		;
		mov	ecx, [edi].dwImageWidth
		mov	edx, [edi].dwImageHeight
		;
		.IF	ecx > [esi].dwImageWidth
			mov	ecx, [esi].dwImageWidth
		.ENDIF
		.IF	ecx > [ebx].dwImageWidth
			mov	ecx, [esi].dwImageWidth
		.ENDIF
		.IF	edx > [esi].dwImageHeight
			mov	edx, [esi].dwImageHeight
		.ENDIF
		.IF	edx > [ebx].dwImageHeight
			mov	edx, [ebx].dwImageHeight
		.ENDIF
;		.IF	(ecx != [esi].dwImageWidth) || (edx != [esi].dwImageHeight) \
;			|| (ecx != [ebx].dwImageWidth) || (edx != [ebx].dwImageHeight)
;			TRACE	<"画像サイズが一致しません。", 0AH>
;			mov	eax, eslErrGeneral
;			ret
;		.ENDIF
		mov	nImageWidth, ecx
		mov	nImageHeight, edx
		;
		; ビット深度検証
		;
		mov	eax, [edi].dwBitsPerPixel
		mov	ecx, [esi].dwBitsPerPixel
		mov	edx, [ebx].dwBitsPerPixel
		.IF	(eax != 32) || (edx != 8) || ((ecx != 24) && (ecx != 32))
			TRACE	<"ビット深度が不正です。", 0AH>
			mov	eax, eslErrGeneral
			ret
		.ENDIF
		shr	ecx, 3
		mov	nSrcPixelBytes, ecx
		mov	[edi].fdwFormatType, EIF_RGBA_BITMAP
		;
		; 処理関数設定
		;
		mov	eax, [esi].fdwFormatType
		mov	edx, dwFlags
		mov	fUsedSSE, 0
		.IF	eax & EIF_WITH_ALPHA
			;
			; RGBA * (RGBA + ALPHA) 合成
			;
			.IF	!(edx & EGL_BAC_ADD_ALPHA)
				mov	nAlphaBase, 0
				or	edx, EGL_BAC_ADD_ALPHA
			.ENDIF
			.IF	!(edx & EGL_BAC_MULTIPLY_ALPHA)
				mov	nCoefficient, 10H
				or	edx, EGL_BAC_MULTIPLY_ALPHA
			.ENDIF
			.IF	(nSrcPixelBytes == 4) \
					&& (ERI_EnabledProcessorType & ERI_USE_XMM_P3)
				mov	pfnSubFunc, OFFSET SubFunc_BlendMultiplyAlpha_SSE
				mov	fUsedSSE, 1
			.ELSE
				mov	pfnSubFunc, OFFSET SubFunc_BlendMultiplyAlpha
			.ENDIF
		.ELSEIF	edx & (EGL_BAC_ADD_ALPHA OR EGL_BAC_MULTIPLY_ALPHA)
			.IF	!(edx & EGL_BAC_ADD_ALPHA)
				mov	nAlphaBase, 0
			.ENDIF
			.IF	!(edx & EGL_BAC_MULTIPLY_ALPHA)
				mov	nCoefficient, 10H
			.ENDIF
			.IF	(nSrcPixelBytes == 4) \
					&& (ERI_EnabledProcessorType & ERI_USE_XMM_P3)
				mov	pfnSubFunc, OFFSET SubFunc_PackMultiplyAlpha_SSE
				mov	fUsedSSE, 1
			.ELSE
				mov	pfnSubFunc, OFFSET SubFunc_PackMultiplyAlpha
			.ENDIF
		.ELSEIF	edx & EGL_BAC_MULTIPLY
			.IF	(nSrcPixelBytes == 4) \
					&& (ERI_EnabledProcessorType & ERI_USE_XMM_P3)
				mov	pfnSubFunc, OFFSET SubFunc_PackMultiplySSE
				mov	fUsedSSE, 1
			.ELSE
				mov	pfnSubFunc, OFFSET SubFunc_PackMultiply
			.ENDIF
		.ELSE
			mov	pfnSubFunc, OFFSET SubFunc_PackOnly
		.ENDIF
		;
		; 処理準備
		;
		mov	eax, [edi].dwBytesPerLine
		mov	ecx, [esi].dwBytesPerLine
		mov	edx, [ebx].dwBytesPerLine
		mov	nDstLineBytes, eax
		mov	nSrcLineBytes, ecx
		mov	nAlphaLineBytes, edx
		;
		mov	edi, [edi].ptrImageArray
		mov	esi, [esi].ptrImageArray
		mov	ebx, [ebx].ptrImageArray

	.ELSE
		;
		; フォーマット検証
		;
		mov	ecx, [edi].dwImageWidth
		mov	edx, [edi].dwImageHeight
		mov	nImageWidth, ecx
		mov	nImageHeight, edx
		;
		.IF	([edi].dwBitsPerPixel != 32) || \
				([edi].fdwFormatType != EIF_RGBA_BITMAP)
			TRACE	<"出力先フォーマットが不正です。", 0AH>
			mov	eax, eslErrGeneral
			ret
		.ENDIF
		;
		; 処理関数設定
		;
		mov	fUsedSSE, 0
		.IF	ERI_EnabledProcessorType & ERI_USE_XMM_P3
			mov	pfnSubFunc, OFFSET SubFunc_MultiplySSE
			mov	fUsedSSE, 1
		.ELSE
			mov	pfnSubFunc, OFFSET SubFunc_Multiply
		.ENDIF
		;
		; 処理準備
		;
		mov	eax, [edi].dwBytesPerLine
		mov	nDstLineBytes, eax
		mov	nSrcLineBytes, 0
		mov	nAlphaLineBytes, 0
		;
		mov	edi, [edi].ptrImageArray
		xor	esi, esi
		xor	ebx, ebx
	.ENDIF
	;
	; αチャネル合成
	;
	mov	ecx, nImageHeight
	or	ecx, ecx
	.WHILE	!ZERO?
		mov	nLeftHeight, ecx
		mov	ptrDstLine, edi
		mov	ptrSrcLine, esi
		mov	ptrAlphaLine, ebx
		;
		mov	ecx, nImageWidth
		or	ecx, ecx
		.IF	!ZERO?
			call	pfnSubFunc
		.ENDIF
		;
		mov	edi, ptrDstLine
		mov	esi, ptrSrcLine
		mov	ebx, ptrAlphaLine
		mov	ecx, nLeftHeight
		add	edi, nDstLineBytes
		add	esi, nSrcLineBytes
		add	ebx, nAlphaLineBytes
		dec	ecx
	.ENDW

	ASSUME	edi:NOTHING
	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	.IF	fUsedSSE != 0
		sfence
		emms
	.ENDIF
	xor	eax, eax
	ret

ALIGN	10H
SubFunc_PackOnly:
		mov	al, BYTE PTR [esi]
		mov	ah, BYTE PTR [esi + 1]
		mov	dl, BYTE PTR [esi + 2]
		add	esi, nSrcPixelBytes
		mov	dh, BYTE PTR [ebx]
		inc	ebx
		mov	BYTE PTR [edi], al
		mov	BYTE PTR [edi + 1], ah
		mov	BYTE PTR [edi + 2], dl
		mov	BYTE PTR [edi + 3], dh
		add	edi, 4
		dec	ecx
	jnz	SubFunc_PackOnly
	BYTE	0C3H	; ret

ALIGN	10H
SubFunc_PackMultiply:
		movzx	edx, BYTE PTR [ebx]
		inc	ebx
		test	edx, edx
		mov	eax, edx
		.IF	ZERO?
			mov	DWORD PTR [edi], edx
		.ELSEIF	edx == 0FFH
			movzx	eax, BYTE PTR [esi + 2]
			movzx	edx, WORD PTR [esi]
			shl	eax, 16
			or	eax, edx
			or	eax, 0FF000000H
			mov	DWORD PTR [edi], eax
		.ELSE
			mov	BYTE PTR [edi + 3], dl
			shl	edx, 8
			or	edx, eax
			;
			FOR	@TEMP, <0, 1, 2>
				movzx	eax, BYTE PTR [esi + @TEMP]
				imul	eax, edx
				shr	eax, 16
				adc	eax, 0
				mov	BYTE PTR [edi + @TEMP], al
			ENDM
		.ENDIF
		;
		add	esi, nSrcPixelBytes
		add	edi, 4
		dec	ecx
	jnz	SubFunc_PackMultiply
	BYTE	0C3H	; ret

ALIGN	10H
SubFunc_PackMultiplySSE:
		movzx		edx, BYTE PTR [ebx]
		inc		ebx
		test		edx, edx
		movd		mm0, DWORD PTR [esi]
		.IF	ZERO?
			mov		DWORD PTR [edi], edx
			add		esi, 4
		.ELSEIF	edx == 0FFH
			por		mm0, mmxAlphaMask
			add		esi, 4
			movd		DWORD PTR [edi], mm0
		.ELSE
			lea		eax, [edx + 1]
			pxor		mm7, mm7
			movd		mm1, eax
			punpcklbw	mm0, mm7
			pshufw		mm1, mm1, 0
			pmullw		mm0, mm1
			add		esi, 4
			psrlw		mm0, 8
			pinsrw		mm0, edx, 3
			packuswb	mm0, mm7
			movd		DWORD PTR [edi], mm0
		.ENDIF
		add		edi, 4
		dec		ecx
	jnz	SubFunc_PackMultiplySSE
	BYTE	0C3H	; ret

ALIGN	10H
SubFunc_Multiply:
		movzx	edx, BYTE PTR [edi + 3]
		mov	eax, edx
		shl	edx, 8
		or	edx, eax
		;
		FOR	@TEMP, <0, 1, 2>
			movzx	eax, BYTE PTR [edi + @TEMP]
			imul	eax, edx
			shr	eax, 16
			adc	eax, 0
			mov	BYTE PTR [edi + @TEMP], al
		ENDM
		;
		add	edi, 4
		dec	ecx
	jnz	SubFunc_Multiply
	BYTE	0C3H	; ret

ALIGN	10H
SubFunc_MultiplySSE:
	movq	mm4, mmxMaskRGBChannel
	movq	mm5, mmxAlphaChannelScale
	movq	mm6, mmxAlphaChannelBias
	pxor	mm7, mm7
Loop_SubFunc_MultiplySSE:
		movd		mm0, DWORD PTR [edi]
		punpcklbw	mm0, mm7
		pshufw		mm1, mm0, 11111111B
		paddw		mm1, mm6
		pand		mm1, mm4
		por		mm1, mm5
		pmullw		mm0, mm1
		psrlw		mm0, 8
		packuswb	mm0, mm7
		movd		DWORD PTR [edi], mm0
		add		edi, 4
		dec		ecx
	jnz	Loop_SubFunc_MultiplySSE
	BYTE	0C3H	; ret

ALIGN	10H
SubFunc_PackMultiplyAlpha:
		movzx	edx, BYTE PTR [ebx]
		inc	ebx
		imul	edx, nCoefficient
		shr	edx, 4
		adc	edx, nAlphaBase
		test	edx, 0FFFFFF00H
		.IF	!ZERO?
			sar	edx, 31
			not	edx
			and	edx, 0FFH
		.ENDIF
		;
		test	edx, edx
		.IF	ZERO?
			mov	DWORD PTR [edi], edx
		.ELSEIF	edx == 0FFH
			movzx	eax, BYTE PTR [esi + 2]
			movzx	edx, WORD PTR [esi]
			shl	eax, 16
			or	eax, edx
			or	eax, 0FF000000H
			mov	DWORD PTR [edi], eax
		.ELSE
			mov	BYTE PTR [edi + 3], dl
			FOR	@TEMP, <0, 1, 2>
				movzx	eax, BYTE PTR [esi + @TEMP]
				imul	eax, edx
				shr	eax, 8
				adc	eax, 0
				mov	BYTE PTR [edi + @TEMP], al
			ENDM
		.ENDIF
		;
		add	esi, nSrcPixelBytes
		add	edi, 4
		dec	ecx
	jnz	SubFunc_PackMultiplyAlpha
	BYTE	0C3H	; ret

ALIGN	10H
SubFunc_BlendMultiplyAlpha:
		movzx	edx, BYTE PTR [ebx]
		inc	ebx
		imul	edx, nCoefficient
		shr	edx, 4
		adc	edx, nAlphaBase
		test	edx, 0FFFFFF00H
		.IF	!ZERO?
			sar	edx, 31
			not	edx
			and	edx, 0FFH
		.ENDIF
		;
		test	edx, edx
		.IF	ZERO?
			mov	DWORD PTR [edi], edx
		.ELSEIF	edx == 0FFH
			.IF	nSrcPixelBytes == 3
				movzx	eax, BYTE PTR [esi + 2]
				movzx	edx, WORD PTR [esi]
				shl	eax, 16
				or	eax, edx
				or	eax, 0FF000000H
			.ELSE
				mov	eax, DWORD PTR [esi]
			.ENDIF
			mov	DWORD PTR [edi], eax
		.ELSE
			FOR	@TEMP, <0, 1, 2>
				movzx	eax, BYTE PTR [esi + @TEMP]
				imul	eax, edx
				shr	eax, 8
				adc	eax, 0
				mov	BYTE PTR [edi + @TEMP], al
			ENDM
			.IF	nSrcPixelBytes == 3
				mov	eax, edx
			.ELSE
				movzx	eax, BYTE PTR [esi + 3]
				imul	eax, edx
				mov	edx, eax
				shr	eax, 8
				add	eax, edx
				shr	eax, 8
				adc	eax, 0
			.ENDIF
			mov	BYTE PTR [edi + 3], al
		.ENDIF
		;
		add	esi, nSrcPixelBytes
		add	edi, 4
		dec	ecx
	jnz	SubFunc_BlendMultiplyAlpha
	BYTE	0C3H	; ret

ALIGN	10H
SubFunc_PackMultiplyAlpha_SSE:
	prefetchnta	[ebx]
	prefetchnta	[esi]
	mov	edx, nAlphaBase
	mov	eax, nCoefficient
	movd	mm5, eax
	movd	mm6, edx
	pshufw	mm5, mm5, 00000000B
	pshufw	mm6, mm6, 00000000B
	movq	mmxCoefficient, mm5
	movq	mmxAlphaBase, mm6
Loop_PackMultiplyAlpha_SSE:
		cmp	ecx, 4
		jb	SubFunc_PackMultiplyAlpha
		;
		pxor		mm7, mm7
		movd		mm0, DWORD PTR [ebx]
		prefetchnta	[ebx + 32]
		punpcklbw	mm0, mm7
		movq		mm1, mm0
		punpcklwd	mm0, mm7
		punpckhwd	mm1, mm7
		pmaddwd		mm0, mm5	; * nCoefficient
		prefetchnta	[esi + 32]
		pmaddwd		mm1, mm5
		add		ebx, 4
		psrad		mm0, 4
		psrad		mm1, 4
		packssdw	mm0, mm1
		paddsw		mm0, mm6	; + nAlphaBase
		packuswb	mm0, mm7
		movd	eax, mm0
		punpcklbw	mm0, mm7
		;
		test	eax, eax
		.IF	ZERO?
			movntq		MMWORD PTR [edi], mm0
			movntq		MMWORD PTR [edi + 8], mm0
		.ELSEIF	eax == 0FFFFFFFFH
			movq		mm2, mmxAlphaMask
			movq		mm0, MMWORD PTR [esi]
			movq		mm1, MMWORD PTR [esi + 8]
			por		mm0, mm2
			por		mm1, mm2
			movntq		MMWORD PTR [edi], mm0
			movntq		MMWORD PTR [edi + 8], mm1
		.ELSE
			movq		mm1, MMWORD PTR [esi]
			pshufw		mm3, mm0, 00000000B
			pshufw		mm4, mm0, 01010101B
			movq		mm2, mm1
			punpcklbw	mm1, mm7
			pextrw		eax, mm0, 0
			punpckhbw	mm2, mm7
			pextrw		edx, mm0, 1
			;
			pmullw		mm1, mm3
			movq		mm3, mmx00FFW
			shl		eax, 8
			pmullw		mm2, mm4
			shl		edx, 8
			;
			paddusw		mm1, mm3
			paddusw		mm2, mm3
			pinsrw		mm1, eax, 3
			pinsrw		mm2, edx, 3
			;
			psrlw		mm1, 8
			psrlw		mm2, 8
			packuswb	mm1, mm2
			movntq		MMWORD PTR [edi], mm1
			;
			movq		mm1, MMWORD PTR [esi + 8]
			pshufw		mm3, mm0, 10101010B
			pshufw		mm4, mm0, 11111111B
			movq		mm2, mm1
			punpcklbw	mm1, mm7
			pextrw		eax, mm0, 2
			punpckhbw	mm2, mm7
			pextrw		edx, mm0, 3
			;
			pmullw		mm1, mm3
			movq		mm3, mmx00FFW
			shl		eax, 8
			pmullw		mm2, mm4
			shl		edx, 8
			;
			paddusw		mm1, mm3
			paddusw		mm2, mm3
			pinsrw		mm1, eax, 3
			pinsrw		mm2, edx, 3
			;
			psrlw		mm1, 8
			psrlw		mm2, 8
			packuswb	mm1, mm2
			movntq		MMWORD PTR [edi + 8], mm1
		.ENDIF
		;
		add	esi, 16
		add	edi, 16
		sub	ecx, 4
	jnz	Loop_PackMultiplyAlpha_SSE
	BYTE	0C3H	; ret

SubFunc_BlendMultiplyAlpha_SSE:
	prefetchnta	[ebx]
	prefetchnta	[esi]
	mov	edx, nAlphaBase
	mov	eax, nCoefficient
	movd	mm5, eax
	movd	mm6, edx
	pshufw	mm5, mm5, 00000000B
	pshufw	mm6, mm6, 00000000B
	movq	mmxCoefficient, mm5
	movq	mmxAlphaBase, mm6
Loop_BlendMultiplyAlpha_SSE:
		cmp	ecx, 4
		jb	SubFunc_BlendMultiplyAlpha
		;
		pxor		mm7, mm7
		movd		mm0, DWORD PTR [ebx]
		prefetchnta	[ebx + 32]
		punpcklbw	mm0, mm7
		movq		mm1, mm0
		punpcklwd	mm0, mm7
		punpckhwd	mm1, mm7
		pmaddwd		mm0, mm5	; * nCoefficient
		prefetchnta	[esi + 32]
		pmaddwd		mm1, mm5
		add		ebx, 4
		psrad		mm0, 4
		psrad		mm1, 4
		packssdw	mm0, mm1
		paddsw		mm0, mm6	; + nAlphaBase
		packuswb	mm0, mm7
		movd	eax, mm0
		punpcklbw	mm0, mm7
		;
		test	eax, eax
		.IF	ZERO?
			movq		MMWORD PTR [edi], mm0
			movq		MMWORD PTR [edi + 8], mm0
		.ELSEIF	eax == 0FFFFFFFFH
			movq		mm0, MMWORD PTR [esi]
			movq		mm1, MMWORD PTR [esi + 8]
			movq		MMWORD PTR [edi], mm0
			movq		MMWORD PTR [edi + 8], mm1
		.ELSE
			movq		mm1, MMWORD PTR [esi]
			pshufw		mm3, mm0, 00000000B
			pshufw		mm4, mm0, 01010101B
			movq		mm2, mm1
			punpcklbw	mm1, mm7
			punpckhbw	mm2, mm7
			;
			pmullw		mm1, mm3
			movq		mm3, mmx00FFW
			pmullw		mm2, mm4
			;
			paddusw		mm1, mm3
			paddusw		mm2, mm3
			;
			psrlw		mm1, 8
			psrlw		mm2, 8
			packuswb	mm1, mm2
			movq		MMWORD PTR [edi], mm1
			;
			movq		mm1, MMWORD PTR [esi + 8]
			pshufw		mm3, mm0, 10101010B
			pshufw		mm4, mm0, 11111111B
			movq		mm2, mm1
			punpcklbw	mm1, mm7
			punpckhbw	mm2, mm7
			;
			pmullw		mm1, mm3
			movq		mm3, mmx00FFW
			pmullw		mm2, mm4
			;
			paddusw		mm1, mm3
			paddusw		mm2, mm3
			;
			psrlw		mm1, 8
			psrlw		mm2, 8
			packuswb	mm1, mm2
			movq		MMWORD PTR [edi + 8], mm1
		.ENDIF
		;
		add	esi, 16
		add	edi, 16
		sub	ecx, 4
	jnz	Loop_BlendMultiplyAlpha_SSE
	BYTE	0C3H	; ret

eglBlendAlphaChannel	ENDP

;
;	αチャネルを分離する
; ----------------------------------------------------------------------------
ALIGN	10H
eglUnpackAlphaChannel	PROC	NEAR32 C USES ebx esi edi,
	pDstRGB:PEGL_IMAGE_INFO, pDstAlpha:PEGL_IMAGE_INFO,
	pSrcRGBA32:PCEGL_IMAGE_INFO, dwFlags:DWORD

	LOCAL	ptrDstRGBLine:PTR, ptrDstAlphaLine:PTR
	LOCAL	ptrSrcRGBALine:PTR
	LOCAL	nDstRGBLineBytes:SDWORD, nDstAlphaLineBytes:SDWORD
	LOCAL	nSrcRGBALineBytes:SDWORD
	LOCAL	nDstRGBPixelBytes:DWORD
	LOCAL	nImageWidth:DWORD, nImageHeight:DWORD
	LOCAL	nLeftWidth:DWORD, nLeftHeight:DWORD
	LOCAL	dwRcpAlpha[100H]:DWORD

	mov	esi, pSrcRGBA32
	mov	edi, pDstRGB
	mov	ebx, pDstAlpha
	;
	ASSUME	esi:PCEGL_IMAGE_INFO
	ASSUME	edi:PEGL_IMAGE_INFO
	ASSUME	ebx:PEGL_IMAGE_INFO
	.IF	[esi].dwBitsPerPixel != 32
		TRACE	<"入力画像のビット深度が不正です。", 0AH>
		mov	eax, eslErrGeneral
		ret
	.ENDIF
	mov	eax, [esi].dwBytesPerLine
	mov	ecx, [esi].dwImageWidth
	mov	edx, [esi].dwImageHeight
	mov	nSrcRGBALineBytes, eax
	mov	nImageWidth, ecx
	mov	nImageHeight, edx
	.IF	(ecx == 0) || (edx == 0)
		xor	eax, eax
		ret
	.ENDIF
	.IF	ebx != NULL
		.IF	[ebx].dwBitsPerPixel != 8
			TRACE	<"αチャネルのビット深度が不正です。", 0AH>
			mov	eax, eslErrGeneral
			ret
		.ENDIF
		.IF	([ebx].dwImageWidth != ecx) \
				|| ([ebx].dwImageHeight != edx)
			TRACE	<"αチャネルの画像サイズが不正です。", 0AH>
			mov	eax, eslErrGeneral
			ret
		.ENDIF
		mov	eax, [ebx].dwBytesPerLine
		mov	nDstAlphaLineBytes, eax
	.ENDIF
	;
	.IF	edi != NULL
		mov	eax, [edi].dwBitsPerPixel
		.IF	(eax != 24) && (eax != 32)
			TRACE	<"RGB画像のビット深度が不正です。", 0AH>
			mov	eax, eslErrGeneral
			ret
		.ENDIF
		shr	eax, 3
		mov	nDstRGBPixelBytes, eax
		.IF	([edi].dwImageWidth != ecx) \
				|| ([edi].dwImageHeight != edx)
			TRACE	<"RGB画像のサイズが不正です。", 0AH>
			mov	eax, eslErrGeneral
			ret
		.ENDIF
		mov	eax, [edi].dwBytesPerLine
		mov	nDstRGBLineBytes, eax
	.ENDIF
	;
	.IF	edi != NULL
	.IF	dwFlags & EGL_BAC_MULTIPLY
		mov	ecx, 1
		mov	dwRcpAlpha[0], 100H
		.REPEAT
			mov	eax, 1000000H
			xor	edx, edx
			div	ecx
			mov	dwRcpAlpha[ecx * 4], eax
			inc	ecx
		.UNTIL	ecx >= 100H
		;
		.IF	ebx != NULL
			mov	edi, [edi].ptrImageArray
			mov	ebx, [ebx].ptrImageArray
			mov	esi, [esi].ptrImageArray
			;
			mov	edx, nImageHeight
			.REPEAT
				mov	nLeftHeight, edx
				mov	ptrDstRGBLine, edi
				mov	ptrDstAlphaLine, ebx
				mov	ptrSrcRGBALine, esi
				mov	edx, nImageWidth
				;
				.REPEAT
					movzx	eax, BYTE PTR [esi + 3]
					mov	BYTE PTR [ebx], al
					inc	ebx
					.IF	eax != 0FFH
					test	eax, eax
					.IF	!ZERO?
						mov	eax, dwRcpAlpha[eax * 4]
						movzx	ecx, BYTE PTR [esi]
						imul	ecx, eax
						shr	ecx, 16
						add	ecx, 0FFFFFF00H
						sbb	ch, ch
						or	cl, ch
						mov	BYTE PTR [edi], cl
						movzx	ecx, BYTE PTR [esi + 1]
						imul	ecx, eax
						shr	ecx, 16
						add	ecx, 0FFFFFF00H
						sbb	ch, ch
						or	cl, ch
						mov	BYTE PTR [edi + 1], cl
						movzx	ecx, BYTE PTR [esi + 2]
						imul	ecx, eax
						shr	ecx, 16
						add	ecx, 0FFFFFF00H
						sbb	ch, ch
						or	cl, ch
						mov	BYTE PTR [edi + 2], cl
					.ENDIF
					.ENDIF
					add	esi, 4
					add	edi, nDstRGBPixelBytes
					dec	edx
				.UNTIL	ZERO?
				;
				mov	edx, nLeftHeight
				mov	edi, ptrDstRGBLine
				mov	ebx, ptrDstAlphaLine
				mov	esi, ptrSrcRGBALine
				add	edi, nDstRGBLineBytes
				add	ebx, nDstAlphaLineBytes
				add	esi, nSrcRGBALineBytes
				dec	edx
			.UNTIL	ZERO?
		.ELSE
			mov	edi, [edi].ptrImageArray
			mov	esi, [esi].ptrImageArray
			;
			mov	edx, nImageHeight
			.REPEAT
				mov	nLeftHeight, edx
				mov	ptrDstRGBLine, edi
				mov	ptrSrcRGBALine, esi
				mov	edx, nImageWidth
				;
				.REPEAT
					movzx	eax, BYTE PTR [esi + 3]
					.IF	eax != 0FFH
					test	eax, eax
					.IF	!ZERO?
						mov	eax, dwRcpAlpha[eax * 4]
						movzx	ecx, BYTE PTR [esi]
						imul	ecx, eax
						shr	ecx, 16
						add	ecx, 0FFFFFF00H
						sbb	ch, ch
						or	cl, ch
						mov	BYTE PTR [edi], cl
						movzx	ecx, BYTE PTR [esi + 1]
						imul	ecx, eax
						shr	ecx, 16
						add	ecx, 0FFFFFF00H
						sbb	ch, ch
						or	cl, ch
						mov	BYTE PTR [edi + 1], cl
						movzx	ecx, BYTE PTR [esi + 2]
						imul	ecx, eax
						shr	ecx, 16
						add	ecx, 0FFFFFF00H
						sbb	ch, ch
						or	cl, ch
						mov	BYTE PTR [edi + 2], cl
					.ENDIF
					.ENDIF
					add	esi, 4
					add	edi, nDstRGBPixelBytes
					dec	edx
				.UNTIL	ZERO?
				;
				mov	edx, nLeftHeight
				mov	edi, ptrDstRGBLine
				mov	esi, ptrSrcRGBALine
				add	edi, nDstRGBLineBytes
				add	esi, nSrcRGBALineBytes
				dec	edx
			.UNTIL	ZERO?
		.ENDIF
	.ELSE
		.IF	ebx != NULL
			mov	edi, [edi].ptrImageArray
			mov	ebx, [ebx].ptrImageArray
			mov	esi, [esi].ptrImageArray
			;
			mov	edx, nImageHeight
			.REPEAT
				mov	nLeftHeight, edx
				mov	ptrDstRGBLine, edi
				mov	ptrDstAlphaLine, ebx
				mov	ptrSrcRGBALine, esi
				mov	edx, nImageWidth
				;
				.REPEAT
					mov	al, BYTE PTR [esi + 3]
					mov	cx, WORD PTR [esi]
					mov	ah, BYTE PTR [esi + 2]
					add	esi, 4
					mov	BYTE PTR [ebx], al
					inc	ebx
					mov	WORD PTR [edi], cx
					mov	BYTE PTR [edi + 2], ah
					add	edi, nDstRGBPixelBytes
					dec	edx
				.UNTIL	ZERO?
				;
				mov	edx, nLeftHeight
				mov	edi, ptrDstRGBLine
				mov	ebx, ptrDstAlphaLine
				mov	esi, ptrSrcRGBALine
				add	edi, nDstRGBLineBytes
				add	ebx, nDstAlphaLineBytes
				add	esi, nSrcRGBALineBytes
				dec	edx
			.UNTIL	ZERO?
		.ELSE
			mov	edi, [edi].ptrImageArray
			mov	esi, [esi].ptrImageArray
			;
			mov	edx, nImageHeight
			.REPEAT
				mov	nLeftHeight, edx
				mov	ptrDstRGBLine, edi
				mov	ptrSrcRGBALine, esi
				mov	edx, nImageWidth
				;
				.REPEAT
					mov	ax, WORD PTR [esi]
					mov	cl, BYTE PTR [esi + 2]
					add	esi, 4
					mov	WORD PTR [edi], ax
					mov	BYTE PTR [edi + 2], cl
					add	edi, nDstRGBPixelBytes
					dec	edx
				.UNTIL	ZERO?
				;
				mov	edx, nLeftHeight
				mov	edi, ptrDstRGBLine
				mov	esi, ptrSrcRGBALine
				add	edi, nDstRGBLineBytes
				add	esi, nSrcRGBALineBytes
				dec	edx
			.UNTIL	ZERO?
		.ENDIF
	.ENDIF
	.ELSE
	.IF	ebx != NULL
		mov	ebx, [ebx].ptrImageArray
		mov	esi, [esi].ptrImageArray
		;
		mov	edx, nImageHeight
		.REPEAT
			mov	nLeftHeight, edx
			mov	ptrDstAlphaLine, ebx
			mov	ptrSrcRGBALine, esi
			mov	edx, nImageWidth
			;
			.REPEAT
				mov	al, BYTE PTR [esi + 3]
				add	esi, 4
				mov	BYTE PTR [ebx], al
				inc	ebx
				dec	edx
			.UNTIL	ZERO?
			;
			mov	edx, nLeftHeight
			mov	ebx, ptrDstAlphaLine
			mov	esi, ptrSrcRGBALine
			add	ebx, nDstAlphaLineBytes
			add	esi, nSrcRGBALineBytes
			dec	edx
		.UNTIL	ZERO?
	.ENDIF
	.ENDIF
	;
	xor	eax, eax
	ret

eglUnpackAlphaChannel	ENDP


CodeSeg	ENDS

	END
