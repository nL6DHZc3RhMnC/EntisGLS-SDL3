
; ----------------------------------------------------------------------------
;                          Entis Graphic Library
; ----------------------------------------------------------------------------
;      Copyright (c) 2002 Leshade Entis, Entis-soft. Al rights reserved.
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

ALIGN	8
mmxMaskRGB	LABEL	MMWORD
		BYTE	0FFH, 0FFH, 0FFH, 00H, 0FFH, 0FFH, 0FFH, 00H

ConstSeg	ENDS


; ----------------------------------------------------------------------------
;	コードセグメント
; ----------------------------------------------------------------------------

CodeSeg	SEGMENT	PARA READONLY FLAT 'CODE'

;
;	GRAY8 -> RGB24 透明度つき加算描画 486 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@DrawImage_GlowAdd		PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE

	mov	ebx, hDrawImage
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF

	mov	eax, [ebx].dstimg.dwBitsPerPixel
	shr	eax, 3
	mov	[ebx].nBytesPerPixel, eax
	mov	ecx, [ebx].srcimg.dwImageHeight
	mov	esi, [ebx].srcimg.ptrImageArray
	mov	edi, [ebx].dstimg.ptrImageArray
	test	ecx, ecx
	.WHILE	!ZERO?
		mov	[ebx].ptrSrcLine, esi
		mov	[ebx].ptrDstLine, edi
		mov	[ebx].nLeftHeight, ecx
		;
		mov	ecx, [ebx].srcimg.dwImageWidth
		test	ecx, ecx
		.WHILE	!ZERO?
			mov	[ebx].nLeftWidth, ecx
			;
			movzx	edx, BYTE PTR [esi]
			inc	esi
			test	edx, edx
			.IF	!ZERO?
				movzx	eax, BYTE PTR [edi]
				movzx	ecx, BYTE PTR [edi + 1]
				add	al, [ebx].rgbaColor[edx*4].rgb.Blue
				sbb	ah, ah
				add	cl, [ebx].rgbaColor[edx*4].rgb.Green
				sbb	ch, ch
				or	al, ah
				or	cl, ch
				mov	BYTE PTR [edi], al
				movzx	eax, BYTE PTR [edi + 2]
				mov	BYTE PTR [edi + 1], cl
				add	al, [ebx].rgbaColor[edx*4].rgb.Red
				sbb	ah, ah
				or	al, ah
				mov	BYTE PTR [edi + 2], al
			.ENDIF
			;
			mov	ecx, [ebx].nLeftWidth
			add	edi, [ebx].nBytesPerPixel
			dec	ecx
		.ENDW
		;
		mov	esi, [ebx].ptrSrcLine
		mov	edi, [ebx].ptrDstLine
		mov	ecx, [ebx].nLeftHeight
		add	esi, [ebx].srcimg.dwBytesPerLine
		add	edi, [ebx].dstimg.dwBytesPerLine
		dec	ecx
	.ENDW

	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglDrawImage@DrawImage_GlowAdd		ENDP

;
;	GRAY8 -> RGB32 透明度つき加算描画 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@DrawImage_GlowAdd_SSE	PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE

	mov	ebx, hDrawImage
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF

	mov	[ebx].dwSaveRegEBP, ebp
	mov	ecx, [ebx].srcimg.dwImageHeight
	mov	esi, [ebx].srcimg.ptrImageArray
	mov	edi, [ebx].dstimg.ptrImageArray
	pcmpeqw		mm7, mm7
	pxor	mm6, mm6
	psllw	mm7, 8
	or	ecx, ecx
	.WHILE	!ZERO?
		prefetchnta	[esi]
		mov	[ebx].ptrSrcLine, esi
		prefetchnta	[edi]
		mov	[ebx].ptrDstLine, edi
		mov	[ebx].nLeftHeight, ecx
		;
		mov	ecx, [ebx].srcimg.dwImageWidth
		movq	mm7, mmxMaskRGB
		sub	ecx, 2
		.WHILE	!SIGN?
			movzx	eax, BYTE PTR [esi]
			movzx	edx, BYTE PTR [esi + 1]
			prefetchnta	[esi + 20H]
			mov	ebp, eax
			prefetchnta	[edi + 20H]
			add	esi, 2
			or	ebp, edx
			.IF	!ZERO?
				movd		mm4, [ebx].rgbaColor[eax*4]
				movd		mm5, [ebx].rgbaColor[edx*4]
				movq		mm0, MMWORD PTR [edi]
				punpckldq	mm4, mm5
				pand		mm4, mm7
				paddusb		mm0, mm4
				movq		MMWORD PTR [edi], mm0
			.ENDIF
			add	edi, 8
			sub	ecx, 2
		.ENDW
		add	ecx, 2
		.IF	!ZERO?
			movzx	eax, BYTE PTR [esi]
			inc	esi
			test	eax, eax
			.IF	!ZERO?
				movd		mm4, [ebx].rgbaColor[eax*4]
				movd		mm0, DWORD PTR [edi]
				pand		mm4, mm7
				paddusb		mm0, mm4
				movd		DWORD PTR [edi], mm0
			.ENDIF
			add	edi, 4
			dec	ecx
		.ENDIF
		;
		mov	esi, [ebx].ptrSrcLine
		mov	edi, [ebx].ptrDstLine
		mov	ecx, [ebx].nLeftHeight
		add	esi, [ebx].srcimg.dwBytesPerLine
		add	edi, [ebx].dstimg.dwBytesPerLine
		dec	ecx
	.ENDW

	emms
	mov	ebp, [ebx].dwSaveRegEBP
	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglDrawImage@DrawImage_GlowAdd_SSE	ENDP

;
;	GRAY8 -> RGB24 透明度つき描画 486 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@DrawImage_Glow		PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE

	mov	ebx, hDrawImage
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF

	mov	eax, [ebx].dstimg.dwBitsPerPixel
	shr	eax, 3
	mov	[ebx].nBytesPerPixel, eax
	mov	ecx, [ebx].srcimg.dwImageHeight
	mov	esi, [ebx].srcimg.ptrImageArray
	mov	edi, [ebx].dstimg.ptrImageArray
	test	ecx, ecx
	.WHILE	!ZERO?
		mov	[ebx].ptrSrcLine, esi
		mov	[ebx].ptrDstLine, edi
		mov	[ebx].nLeftHeight, ecx
		;
		mov	ecx, [ebx].srcimg.dwImageWidth
		test	ecx, ecx
		.WHILE	!ZERO?
			mov	[ebx].nLeftWidth, ecx
			;
			movzx	edx, BYTE PTR [esi]
			inc	esi
			test	edx, edx
			.IF	!ZERO?
				mov	ecx, edx
				movzx	edx, [ebx].rgbaColor[ecx*4].rgba.Alpha
				movzx	eax, BYTE PTR [edi]
				xor	edx, 0FFH
				;
				imul	eax, edx
				add	eax, 7FH
				shr	eax, 8
				add	al, [ebx].rgbaColor[ecx*4].rgb.Blue
				sbb	ah, ah
				or	al, ah
				mov	BYTE PTR [edi], al
				;
				movzx	eax, BYTE PTR [edi + 1]
				imul	eax, edx
				add	eax, 7FH
				shr	eax, 8
				adc	al, [ebx].rgbaColor[ecx*4].rgb.Green
				sbb	ah, ah
				or	al, ah
				mov	BYTE PTR [edi + 1], al
				;
				movzx	eax, BYTE PTR [edi + 2]
				imul	eax, edx
				add	eax, 7FH
				shr	eax, 8
				adc	al, [ebx].rgbaColor[ecx*4].rgb.Red
				sbb	ah, ah
				or	al, ah
				mov	BYTE PTR [edi + 2], al
			.ENDIF
			;
			mov	ecx, [ebx].nLeftWidth
			add	edi, [ebx].nBytesPerPixel
			dec	ecx
		.ENDW
		;
		mov	esi, [ebx].ptrSrcLine
		mov	edi, [ebx].ptrDstLine
		mov	ecx, [ebx].nLeftHeight
		add	esi, [ebx].srcimg.dwBytesPerLine
		add	edi, [ebx].dstimg.dwBytesPerLine
		dec	ecx
	.ENDW

	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglDrawImage@DrawImage_Glow		ENDP

;
;	GRAY8 -> RGB32 透明度つき描画 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@DrawImage_Glow_SSE		PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE

	mov	ebx, hDrawImage
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF

	mov	[ebx].dwSaveRegEBP, ebp
	mov	ecx, [ebx].srcimg.dwImageHeight
	mov	esi, [ebx].srcimg.ptrImageArray
	mov	edi, [ebx].dstimg.ptrImageArray
	pcmpeqw		mm7, mm7
	pxor	mm6, mm6
	psllw	mm7, 8
	or	ecx, ecx
	.WHILE	!ZERO?
		prefetchnta	[esi]
		mov	[ebx].ptrSrcLine, esi
		prefetchnta	[edi]
		mov	[ebx].ptrDstLine, edi
		mov	[ebx].nLeftHeight, ecx
		;
		mov	ecx, [ebx].srcimg.dwImageWidth
		sub	ecx, 2
		.WHILE	!SIGN?
			movzx	eax, BYTE PTR [esi]
			movzx	edx, BYTE PTR [esi + 1]
			prefetchnta	[esi + 20H]
			mov	ebp, eax
			prefetchnta	[edi + 20H]
			add	esi, 2
			or	ebp, edx
			.IF	!ZERO?
				movq		mm2, MMWORD PTR [edi]
				movd		mm4, [ebx].rgbaColor[eax*4]
				movd		mm5, [ebx].rgbaColor[edx*4]
				movq		mm3, mm2
				punpcklbw	mm2, mm6
				pshufw		mm0, mm4, 01010101B
				punpckhbw	mm3, mm6
				pshufw		mm1, mm5, 01010101B
				punpckldq	mm4, mm5
				pand		mm0, mm7
				pand		mm1, mm7
				pxor		mm0, mm7
				pxor		mm1, mm7
				pmulhuw		mm0, mm2
				pmulhuw		mm1, mm3
				packuswb	mm0, mm1
				paddusb		mm0, mm4
				movq		MMWORD PTR [edi], mm0
			.ENDIF
			add	edi, 8
			sub	ecx, 2
		.ENDW
		add	ecx, 2
		.IF	!ZERO?
			movzx	eax, BYTE PTR [esi]
			inc	esi
			test	eax, eax
			.IF	!ZERO?
				movd		mm4, [ebx].rgbaColor[eax*4]
				movd		mm2, DWORD PTR [edi]
				pshufw		mm0, mm4, 01010101B
				punpcklbw	mm2, mm6
				pand		mm0, mm7
				pxor		mm0, mm7
				pmulhuw		mm0, mm2
				packuswb	mm0, mm6
				paddusb		mm0, mm4
				movd		DWORD PTR [edi], mm0
			.ENDIF
			add	edi, 4
			dec	ecx
		.ENDIF
		;
		mov	esi, [ebx].ptrSrcLine
		mov	edi, [ebx].ptrDstLine
		mov	ecx, [ebx].nLeftHeight
		add	esi, [ebx].srcimg.dwBytesPerLine
		add	edi, [ebx].dstimg.dwBytesPerLine
		dec	ecx
	.ENDW

	emms
	mov	ebp, [ebx].dwSaveRegEBP
	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglDrawImage@DrawImage_Glow_SSE		ENDP

;
;	GRAY8 -> RGBA32 透明度つきブレンド描画 486 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@DrawImage_GlowBlend	PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE

	mov	ebx, hDrawImage
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF

	mov	[ebx].dwSaveRegEBP, ebp
	mov	ecx, [ebx].srcimg.dwImageHeight
	mov	esi, [ebx].srcimg.ptrImageArray
	mov	edi, [ebx].dstimg.ptrImageArray
	test	ecx, ecx
	.WHILE	!ZERO?
		mov	[ebx].ptrSrcLine, esi
		mov	[ebx].ptrDstLine, edi
		mov	[ebx].nLeftHeight, ecx
		;
		mov	ecx, [ebx].srcimg.dwImageWidth
		test	ecx, ecx
		.WHILE	!ZERO?
			movzx	ebp, BYTE PTR [esi]
			inc	esi
			test	ebp, ebp
			.IF	!ZERO?
				mov	[ebx].nLeftWidth, ecx
				mov	ecx, ebp
				movzx	ebp, [ebx].rgbaColor[ecx*4].rgba.Alpha
				xor	ebp, 0FFH
				;
				movzx	eax, BYTE PTR [edi]
				inc	ebp
				movzx	edx, BYTE PTR [edi + 1]
				imul	eax, ebp
				imul	edx, ebp
				shr	eax, 8
				adc	al, [ebx].rgbaColor[ecx*4].rgb.Blue
				sbb	ah, ah
				shr	edx, 8
				adc	dl, [ebx].rgbaColor[ecx*4].rgb.Green
				sbb	dh, dh
				or	al, ah
				or	dl, dh
				mov	BYTE PTR [edi], al
				mov	BYTE PTR [edi + 1], dl
				;
				movzx	edx, BYTE PTR [edi + 3]
				movzx	eax, BYTE PTR [edi + 2]
				xor	edx, 0FFH
				imul	eax, ebp
				imul	edx, ebp
				shr	eax, 8
				adc	al, [ebx].rgbaColor[ecx*4].rgb.Red
				sbb	ah, ah
				shr	edx, 8
				or	al, ah
				not	edx
				mov	BYTE PTR [edi + 2], al
				mov	BYTE PTR [edi + 3], dl
				mov	ecx, [ebx].nLeftWidth
			.ENDIF
			add	edi, 4
			dec	ecx
		.ENDW
		;
		mov	esi, [ebx].ptrSrcLine
		mov	edi, [ebx].ptrDstLine
		mov	ecx, [ebx].nLeftHeight
		add	esi, [ebx].srcimg.dwBytesPerLine
		add	edi, [ebx].dstimg.dwBytesPerLine
		dec	ecx
	.ENDW

	mov	ebp, [ebx].dwSaveRegEBP
	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglDrawImage@DrawImage_GlowBlend	ENDP


CodeSeg	ENDS

	END
