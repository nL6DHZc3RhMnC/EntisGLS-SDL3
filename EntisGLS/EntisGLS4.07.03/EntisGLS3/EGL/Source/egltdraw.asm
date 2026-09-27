
; ----------------------------------------------------------------------------
;                          Entis Graphic Library
; ----------------------------------------------------------------------------
;    Copyright (c) 2002-2003 Leshade Entis, Entis-soft. Al rights reserved.
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
mmx00FF00000000	LABEL	MMWORD
		WORD	0, 0, 0, 0FFH
mmx0000FFFFFFFF	LABEL	MMWORD
		WORD	0FFFFH, 0FFFFH, 0FFFFH, 0
ALIGN	10H
mmxFF000000	LABEL	MMWORD
		BYTE	0, 0, 0, 0FFH, 0, 0, 0, 0FFH
		BYTE	0, 0, 0, 0FFH, 0, 0, 0, 0FFH
mmx00FFFFFF	LABEL	MMWORD
		BYTE	0FFH, 0FFH, 0FFH, 0, 0FFH, 0FFH, 0FFH, 0
		BYTE	0FFH, 0FFH, 0FFH, 0, 0FFH, 0FFH, 0FFH, 0

ConstSeg	ENDS


; ----------------------------------------------------------------------------
;	コードセグメント
; ----------------------------------------------------------------------------

CodeSeg	SEGMENT	PARA READONLY FLAT 'CODE'

;
;	RGB24 -> RGB24 通常描画 486 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@DrawImage_24to24		PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE

	mov	ebx, hDrawImage
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF

	pushfd
	cld
	mov	esi, [ebx].srcimg.ptrImageArray
	mov	edi, [ebx].dstimg.ptrImageArray
	mov	ecx, [ebx].srcimg.dwImageHeight
	test	ecx, ecx
	.WHILE	!ZERO?
		mov	[ebx].nLeftHeight, ecx
		mov	[ebx].ptrSrcLine, esi
		mov	[ebx].ptrDstLine, edi
		;
		mov	eax, [ebx].srcimg.dwBitsPerPixel
		mov	edx, [ebx].srcimg.fdwFormatType
		mov	ecx, [ebx].srcimg.dwImageWidth
		.IF	(eax == [ebx].dstimg.dwBitsPerPixel) && \
				(edx == [ebx].dstimg.fdwFormatType)
			imul	ecx, eax
			shr	ecx, 3
			mov	edx, ecx
			shr	ecx, 2
			and	edx, 03H
			rep	movs	DWORD PTR [edi], DWORD PTR [esi]
			mov	ecx, edx
			rep	movs	BYTE PTR [edi], BYTE PTR [esi]
		.ELSE
			xor	eax, eax
			.IF	!(edx & EIF_WITH_ALPHA)
				mov	eax, 0FF000000H
			.ENDIF
			mov	edx, eax
			test	ecx, ecx
			.WHILE	!ZERO?
				.IF	[ebx].srcimg.dwBitsPerPixel == 24
					movzx	eax, WORD PTR [esi]
					movzx	edx, BYTE PTR [esi + 2]
					or	eax, 0FF000000H
					shl	edx, 16
					add	esi, 3
					or	eax, edx
				.ELSE
					mov	eax, DWORD PTR [esi]
					add	esi, 4
					or	eax, edx
				.ENDIF
				.IF	[ebx].dstimg.dwBitsPerPixel == 24
					mov	WORD PTR [edi], ax
					shr	eax, 16
					mov	BYTE PTR [edi + 2], al
					add	edi, 3
				.ELSE
					mov	DWORD PTR [edi], eax
					add	edi, 4
				.ENDIF
				dec	ecx
			.ENDW
		.ENDIF
		;
		mov	esi, [ebx].ptrSrcLine
		mov	edi, [ebx].ptrDstLine
		mov	ecx, [ebx].nLeftHeight
		add	esi, [ebx].srcimg.dwBytesPerLine
		add	edi, [ebx].dstimg.dwBytesPerLine
		dec	ecx
	.ENDW
	popfd

	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglDrawImage@DrawImage_24to24		ENDP

;
;	RGB24 -> RGB24 通常描画 SSE 専用コード
; ----------------------------------------------------------------------------
eglDrawImage@DrawImage_24to24_SSE	PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE

	mov	ebx, hDrawImage
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF

	mov	esi, [ebx].srcimg.ptrImageArray
	mov	edi, [ebx].dstimg.ptrImageArray
	mov	ecx, [ebx].srcimg.dwImageHeight
	mov	edx, [ebx].srcimg.dwImageWidth
	imul	edx, [ebx].srcimg.dwBitsPerPixel
	.IF	([ebx].dstimg.dwBitsPerPixel != 32) || \
			!([ebx].dstimg.fdwFormatType & EIF_WITH_ALPHA)
		xorps	xmm7, xmm7
		pxor	mm7, mm7
	.ELSEIF	([ebx].srcimg.fdwFormatType & EIF_WITH_ALPHA)
		xorps	xmm7, xmm7
		pxor	mm7, mm7
	.ELSE
		movaps	xmm7, XMMWORD_PTR mmxFF000000
		movq	mm7, mmxFF000000
	.ENDIF
	shr	edx, 3
	or	ecx, ecx
	.WHILE	!ZERO?
		mov	eax, [ebx].srcimg.dwBytesPerLine
		mov	[ebx].nLeftHeight, ecx
		mov	[ebx].ptrSrcLine, esi
		mov	[ebx].ptrDstLine, edi
		;
		mov	ecx, edx
		.IF	!(esi & 0FH) && !(edi & 0FH)
			sub	ecx, 20H
			.WHILE	!SIGN?
				movaps	xmm0, [esi]
				prefetchnta	[esi + eax]
				orps	xmm0, xmm7
				movaps	xmm1, [esi + 10H]
				movntps	[edi], xmm0
				orps	xmm1, xmm7
				add	esi, 20H
				movntps	[edi + 10H], xmm1
				add	edi, 20H
				sub	ecx, 20H
			.ENDW
			add	ecx, 20H
		.ELSE
			sub	ecx, 20H
			.WHILE	!SIGN?
				movq	mm0, MMWORD PTR [esi]
				prefetchnta	[esi + eax]
				movq	mm1, MMWORD PTR [esi + 08H]
				por	mm0, mm7
				movq	mm2, MMWORD PTR [esi + 10H]
				por	mm1, mm7
				movq	mm3, MMWORD PTR [esi + 18H]
				por	mm2, mm7
				add	esi, 20H
				movntq	MMWORD PTR [edi], mm0
				por	mm3, mm7
				movntq	MMWORD PTR [edi + 08H], mm1
				movntq	MMWORD PTR [edi + 10H], mm2
				movntq	MMWORD PTR [edi + 18H], mm3
				add	edi, 20H
				sub	ecx, 20H
			.ENDW
			add	ecx, 20H
		.ENDIF
		sub	ecx, 4
		.WHILE	!SIGN?
			movd	mm0, DWORD PTR [esi]
			add	esi, 4
			por	mm0, mm7
			movd	DWORD PTR [edi], mm0
			add	edi, 4
			sub	ecx, 4
		.ENDW
		add	ecx, 4
		.WHILE	!ZERO?
			mov	al, BYTE PTR [esi]
			inc	esi
			mov	BYTE PTR [edi], al
			inc	edi
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
	sfence
	emms
	xor	eax, eax
	ret

eglDrawImage@DrawImage_24to24_SSE	ENDP

IF	0

;
;	RGB24 -> RGB24 透明度あり描画 486 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@DrawImage_24to24_Trans	PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE

	mov	ebx, hDrawImage
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF

	mov	edx, [ebx].nTrans
	neg	edx
	INVOKE	eglCalculateToneTable ,
			ADDR [ebx].nBlueTone, edx, 0
	;
	mov	edx, [ebx].nTrans
	sub	edx, 100H
	INVOKE	eglCalculateToneTable ,
			ADDR [ebx].nGreenTone, edx, 0
	;
	mov	eax, [ebx].srcimg.dwBitsPerPixel
	mov	edx, [ebx].dstimg.dwBitsPerPixel
	shr	eax, 3
	shr	edx, 3
	mov	[ebx].nBytesPerPixel[0], eax
	mov	[ebx].nBytesPerPixel[4], edx
	;
	mov	ecx, [ebx].srcimg.dwImageHeight
	mov	esi, [ebx].srcimg.ptrImageArray
	mov	edi, [ebx].dstimg.ptrImageArray
	test	ecx, ecx
	.WHILE	!ZERO?
		mov	[ebx].nLeftHeight, ecx
		mov	[ebx].ptrSrcLine, esi
		mov	[ebx].ptrDstLine, edi
		;
		mov	ecx, [ebx].srcimg.dwImageWidth
		.IF	[ebx].nBytesPerPixel[4] == 4
			test	ecx, ecx
			.WHILE	!ZERO?
				movzx	eax, BYTE PTR [esi]
				movzx	edx, BYTE PTR [edi]
				mov	al, [ebx].nBlueTone[eax]
				add	al, [ebx].nGreenTone[edx]
				mov	BYTE PTR [edi], al
				;
				movzx	eax, BYTE PTR [esi + 1]
				movzx	edx, BYTE PTR [edi + 1]
				mov	al, [ebx].nBlueTone[eax]
				add	al, [ebx].nGreenTone[edx]
				mov	BYTE PTR [edi + 1], al
				;
				movzx	eax, BYTE PTR [esi + 2]
				movzx	edx, BYTE PTR [edi + 2]
				mov	al, [ebx].nBlueTone[eax]
				add	esi, [ebx].nBytesPerPixel[0]
				add	al, [ebx].nGreenTone[edx]
				mov	dl, [ebx].nBlueTone[0FFH]
				mov	BYTE PTR [edi + 2], al
				mov	BYTE PTR [edi + 3], dl
				add	edi, 4
				;
				dec	ecx
			.ENDW
		.ELSE
			test	ecx, ecx
			.WHILE	!ZERO?
				movzx	eax, BYTE PTR [esi]
				movzx	edx, BYTE PTR [edi]
				mov	al, [ebx].nBlueTone[eax]
				add	al, [ebx].nGreenTone[edx]
				mov	BYTE PTR [edi], al
				;
				movzx	eax, BYTE PTR [esi + 1]
				movzx	edx, BYTE PTR [edi + 1]
				mov	al, [ebx].nBlueTone[eax]
				add	al, [ebx].nGreenTone[edx]
				mov	BYTE PTR [edi + 1], al
				;
				movzx	eax, BYTE PTR [esi + 2]
				movzx	edx, BYTE PTR [edi + 2]
				mov	al, [ebx].nBlueTone[eax]
				add	esi, [ebx].nBytesPerPixel[0]
				add	al, [ebx].nGreenTone[edx]
				mov	BYTE PTR [edi + 2], al
				add	edi, [ebx].nBytesPerPixel[4]
				;
				dec	ecx
			.ENDW
		.ENDIF
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

eglDrawImage@DrawImage_24to24_Trans	ENDP

;
;	RGB32 -> RGB32 透明度あり描画 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@DrawImage_RGB32_Trans_SSE	PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE

	mov	ebx, hDrawImage
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF

	mov	edx, 100H
	movd	mm7, [ebx].nTrans
	sub	edx, [ebx].nTrans
	pxor	mm5, mm5
	movd	mm6, edx
	pshufw	mm7, mm7, 0
	pshufw	mm6, mm6, 0
	;
	mov	ecx, [ebx].srcimg.dwImageHeight
	mov	esi, [ebx].srcimg.ptrImageArray
	mov	edi, [ebx].dstimg.ptrImageArray
	test	ecx, ecx
	prefetchnta	[esi]
	prefetchnta	[edi]
	.WHILE	!ZERO?
		mov	eax, [ebx].srcimg.dwBytesPerLine
		mov	edx, [ebx].dstimg.dwBytesPerLine
		mov	[ebx].nLeftHeight, ecx
		mov	[ebx].ptrSrcLine, esi
		mov	[ebx].ptrDstLine, edi
		;
		.IF	[ebx].dstimg.fdwFormatType & EIF_WITH_ALPHA
			mov	ecx, [ebx].srcimg.dwImageWidth
			sub	ecx, 2
			.WHILE	!SIGN?
				movq		mm0, MMWORD PTR [esi]
				prefetchnta	[esi + eax]
				movq		mm2, MMWORD PTR [edi]
				por		mm0, mmxFF000000
				pand		mm2, mmx00FFFFFF
				movq		mm1, mm0
				punpcklbw	mm0, mm5
				movq		mm3, mm2
				punpcklbw	mm2, mm5
				pmullw		mm0, mm6
				punpckhbw	mm1, mm5
				pmullw		mm2, mm7
				punpckhbw	mm3, mm5
				add		esi, 8
				pmullw		mm1, mm6
				prefetchnta	[edi + edx]
				pmullw		mm3, mm7
				paddusw		mm0, mm2
				paddusw		mm1, mm3
				psrlw		mm0, 8
				psrlw		mm1, 8
				packuswb	mm0, mm1
				movq		MMWORD PTR [edi], mm0
				add		edi, 8
				sub	ecx, 2
			.ENDW
			add	ecx, 2
			.IF	!ZERO?
				FOR	@DUMMY, <0, 1, 2>
					movzx	edx, BYTE PTR [edi + @DUMMY]
					movzx	eax, BYTE PTR [esi + @DUMMY]
					sub	edx, eax
					imul	edx, [ebx].nTrans
					sar	edx, 8
					adc	eax, edx
					add	eax, 0FFFFFF00H
					sbb	ah, ah
					or	al, ah
					mov	BYTE PTR [edi + @DUMMY], al
				ENDM
				mov	eax, 0FFH
				imul	eax, [ebx].nTrans
				shr	eax, 8
				mov	BYTE PTR [edi + 3], al
			.ENDIF
		.ELSE
			mov	ecx, [ebx].srcimg.dwImageWidth
			sub	ecx, 2
			.WHILE	!SIGN?
				movq		mm0, MMWORD PTR [esi]
				prefetchnta	[esi + eax]
				movq		mm2, MMWORD PTR [edi]
				movq		mm1, mm0
				punpcklbw	mm0, mm5
				movq		mm3, mm2
				punpcklbw	mm2, mm5
				pmullw		mm0, mm6
				punpckhbw	mm1, mm5
				pmullw		mm2, mm7
				punpckhbw	mm3, mm5
				add		esi, 8
				pmullw		mm1, mm6
				prefetchnta	[edi + edx]
				pmullw		mm3, mm7
				paddusw		mm0, mm2
				paddusw		mm1, mm3
				psrlw		mm0, 8
				psrlw		mm1, 8
				packuswb	mm0, mm1
				movq		MMWORD PTR [edi], mm0
				add		edi, 8
				sub	ecx, 2
			.ENDW
			add	ecx, 2
			.IF	!ZERO?
				FOR	@DUMMY, <0, 1, 2>
					movzx	edx, BYTE PTR [edi + @DUMMY]
					movzx	eax, BYTE PTR [esi + @DUMMY]
					sub	edx, eax
					imul	edx, [ebx].nTrans
					sar	edx, 8
					adc	eax, edx
					add	eax, 0FFFFFF00H
					sbb	ah, ah
					or	al, ah
					mov	BYTE PTR [edi + @DUMMY], al
				ENDM
			.ENDIF
		.ENDIF
		;
		mov	esi, [ebx].ptrSrcLine
		mov	edi, [ebx].ptrDstLine
		mov	ecx, [ebx].nLeftHeight
		add	esi, [ebx].srcimg.dwBytesPerLine
		add	edi, [ebx].dstimg.dwBytesPerLine
		dec	ecx
	.ENDW

	ASSUME	ebx:NOTHING
	emms
	xor	eax, eax
	ret

eglDrawImage@DrawImage_RGB32_Trans_SSE	ENDP

;
;	RGBA32 -> RGB24 描画 486 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@DrawImage_RGBA		PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE

	mov	ebx, hDrawImage
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF

	mov	eax, [ebx].dstimg.dwBitsPerPixel
	shr	eax, 3
	mov	[ebx].nBytesPerPixel, eax
	mov	ecx, [ebx].srcimg.dwImageHeight
	mov	esi, [ebx].srcimg.ptrImageArray
	mov	edi, [ebx].dstimg.ptrImageArray
	or	ecx, ecx
	.WHILE	!ZERO?
		mov	[ebx].nLeftHeight, ecx
		mov	[ebx].ptrSrcLine, esi
		mov	[ebx].ptrDstLine, edi
		;
		mov	ecx, [ebx].srcimg.dwImageWidth
		or	ecx, ecx
		.WHILE	!ZERO?
			movzx	eax, BYTE PTR [esi + 3]
			add	esi, 4
			test	eax, eax
			.IF	!ZERO?
			.IF	eax == 0FFH
				mov	ax, WORD PTR [esi - 4]
				mov	dl, BYTE PTR [esi - 2]
				mov	WORD PTR [edi], ax
				mov	BYTE PTR [edi + 2], dl
			.ELSE
				xor	eax, 0FFH
				mov	[ebx].nLeftWidth, ecx
				movzx	edx, BYTE PTR [edi]
				inc	eax
				movzx	ecx, BYTE PTR [edi + 1]
				imul	edx, eax
				imul	ecx, eax
				add	dh, BYTE PTR [esi - 4]
				sbb	dl, dl
				add	ch, BYTE PTR [esi - 3]
				sbb	cl, cl
				or	dh, dl
				or	ch, cl
				mov	BYTE PTR [edi], dh
				movzx	edx, BYTE PTR [edi + 2]
				mov	BYTE PTR [edi + 1], ch
				imul	edx, eax
				mov	ecx, [ebx].nLeftWidth
				add	dh, BYTE PTR [esi - 2]
				sbb	dl, dl
				or	dh, dl
				mov	BYTE PTR [edi + 2], dh
			.ENDIF
			.ELSEIF	(DWORD PTR [esi - 4]) != 0
				mov	al, BYTE PTR [edi]
				mov	dl, BYTE PTR [edi + 1]
				add	al, BYTE PTR [esi - 4]
				sbb	ah, ah
				add	dl, BYTE PTR [esi - 3]
				sbb	dh, dh
				or	al, ah
				or	dl, dh
				mov	BYTE PTR [edi], al
				mov	al, BYTE PTR [edi + 2]
				mov	BYTE PTR [edi + 1], dl
				add	al, BYTE PTR [esi - 2]
				sbb	ah, ah
				or	al, ah
				mov	BYTE PTR [edi + 2], al
			.ENDIF
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

eglDrawImage@DrawImage_RGBA		ENDP

;
;	RGBA32 -> RGB32 描画 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@DrawImage_RGBA_SSE		PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE

	mov	ebx, hDrawImage
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF

	mov	ecx, [ebx].srcimg.dwImageHeight
	mov	esi, [ebx].srcimg.ptrImageArray
	mov	edi, [ebx].dstimg.ptrImageArray
	or	ecx, ecx
	.WHILE	!ZERO?
		prefetchnta	[esi]
		prefetchnta	[edi]
		mov	[ebx].nLeftHeight, ecx
		mov	[ebx].ptrSrcLine, esi
		mov	[ebx].ptrDstLine, edi
		;
		mov	ecx, [ebx].srcimg.dwImageWidth
		or	ecx, ecx
		.WHILE	!ZERO?
			mov	eax, DWORD PTR [esi]
			prefetchnta	[esi + 20H]
			rol	eax, 8
			add	esi, 4
			test	eax, eax
			.IF	!ZERO?
			prefetchnta	[edi + 20H]
			.IF	al == 0FFH
				ror	eax, 8
				mov	DWORD PTR [edi], eax
			.ELSEIF	al != 0
				movzx		edx, al
				shr		eax, 8
				xor		edx, 0FFH
				movd		mm0, DWORD PTR [edi]
				inc		edx
				movd		mm2, eax
				movd		mm1, edx
				pxor		mm7, mm7
				pshufw		mm1, mm1, 0
				punpcklbw	mm0, mm7
				pmullw		mm0, mm1
				psrlw		mm0, 8
				packuswb	mm0, mm7
				paddusb		mm0, mm2
				movd		DWORD PTR [edi], mm0
			.ELSEIF	ZERO?	; al == 0
				shr	eax, 8
				movd	mm0, DWORD PTR [edi]
				movd	mm1, eax
				paddusb	mm0, mm1
				movd	DWORD PTR [edi], mm0
			.ENDIF
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

	ASSUME	ebx:NOTHING
	emms
	xor	eax, eax
	ret

eglDrawImage@DrawImage_RGBA_SSE		ENDP

;
;	RGBA32 -> RGB24 透明度つき描画 486 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@DrawImage_RGBA_Trans	PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE

	mov	ebx, hDrawImage
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF

	mov	edx, [ebx].nTrans
	neg	edx
	INVOKE	eglCalculateToneTable ,
			ADDR [ebx].nBlueTone, edx, 0
	;
	mov	eax, [ebx].dstimg.dwBitsPerPixel
	shr	eax, 3
	mov	[ebx].nBytesPerPixel, eax
	mov	ecx, [ebx].srcimg.dwImageHeight
	mov	esi, [ebx].srcimg.ptrImageArray
	mov	edi, [ebx].dstimg.ptrImageArray
	or	ecx, ecx
	.WHILE	!ZERO?
		mov	[ebx].nLeftHeight, ecx
		mov	[ebx].ptrSrcLine, esi
		mov	[ebx].ptrDstLine, edi
		;
		mov	ecx, [ebx].srcimg.dwImageWidth
		or	ecx, ecx
		.WHILE	!ZERO?
			movzx	eax, BYTE PTR [esi + 3]
			add	esi, 4
			test	eax, eax
			.IF	!ZERO?
				movzx	eax, [ebx].nBlueTone[eax]
				mov	[ebx].nLeftWidth, ecx
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
				mov	ecx, [ebx].nLeftWidth
				sbb	dl, dl
				or	dl, dh
				mov	BYTE PTR [edi + 2], dl
			.ELSEIF	(DWORD PTR [esi - 4]) != 0
				movzx	eax, BYTE PTR [esi - 4]
				mov	[ebx].nLeftWidth, ecx
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
				mov	ecx, [ebx].nLeftWidth
				mov	BYTE PTR [edi + 2], dl
			.ENDIF
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

eglDrawImage@DrawImage_RGBA_Trans	ENDP

;
;	RGBA32 -> RGB24 透明度つき描画 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@DrawImage_RGBA_Trans_SSE	PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE

	mov	ebx, hDrawImage
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF

	mov	edx, 100H
	sub	edx, [ebx].nTrans
	movd	mm7, edx
	pshufw	mm7, mm7, 0
	;
	mov	ecx, [ebx].srcimg.dwImageHeight
	mov	esi, [ebx].srcimg.ptrImageArray
	mov	edi, [ebx].dstimg.ptrImageArray
	or	ecx, ecx
	.WHILE	!ZERO?
		prefetchnta	[esi]
		prefetchnta	[edi]
		mov	[ebx].nLeftHeight, ecx
		mov	[ebx].ptrSrcLine, esi
		mov	[ebx].ptrDstLine, edi
		;
		mov	ecx, [ebx].srcimg.dwImageWidth
		or	ecx, ecx
		.WHILE	!ZERO?
			movzx	eax, BYTE PTR [esi + 3]
			prefetchnta	[esi + 20H]
			add	esi, 4
			prefetchnta	[edi + 20H]
			test	eax, eax
			.IF	!ZERO?
;			.IF	eax == 0FFH
;				movd		mm0, DWORD PTR [esi - 4]
;				pxor		mm2, mm2
;				movd		mm1, DWORD PTR [edi]
;				punpcklbw	mm0, mm2
;				punpcklbw	mm1, mm2
;				psubsw		mm0, mm1
;				psraw		mm0, 1
;				pmullw		mm0, mm7
;				psraw		mm0, 7
;				paddsw		mm0, mm1
;				packuswb	mm0, mm2
;				movd		DWORD PTR [edi], mm0
;			.ELSE
				movd		mm6, eax
				movd		mm1, DWORD PTR [edi]
				pshufw		mm6, mm6, 0
				movd		mm0, DWORD PTR [esi - 4]
				pmullw		mm6, mm7
				pxor		mm5, mm5
				pcmpeqw		mm4, mm4
				punpcklbw	mm0, mm5
				pxor		mm6, mm4
				punpcklbw	mm1, mm5
				pmullw		mm0, mm7
				pmulhuw		mm1, mm6
				psrlw		mm0, 8
				paddusw		mm0, mm1
				packuswb	mm0, mm5
				movd		DWORD PTR [edi], mm0
;			.ENDIF
			.ELSEIF	(DWORD PTR [esi - 4]) != 0
				movd		mm0, DWORD PTR [esi - 4]
				pxor		mm2, mm2
				movd		mm1, DWORD PTR [edi]
				punpcklbw	mm0, mm2
				punpcklbw	mm1, mm2
				pmullw		mm0, mm7
				psrlw		mm0, 8
				pand		mm0, mmx0000FFFFFFFF
				paddusw		mm0, mm1
				packuswb	mm0, mm2
				movd		DWORD PTR [edi], mm0
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

	ASSUME	ebx:NOTHING
	emms
	xor	eax, eax
	ret

eglDrawImage@DrawImage_RGBA_Trans_SSE	ENDP

;
;	RGBA32 -> RGBA32 ブレンド描画 486 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@DrawImage_RGBA_Blend	PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE

	mov	ebx, hDrawImage
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF

	mov	ecx, [ebx].srcimg.dwImageHeight
	mov	esi, [ebx].srcimg.ptrImageArray
	mov	edi, [ebx].dstimg.ptrImageArray
	or	ecx, ecx
	.WHILE	!ZERO?
		mov	[ebx].nLeftHeight, ecx
		mov	[ebx].ptrSrcLine, esi
		mov	[ebx].ptrDstLine, edi
		;
		mov	ecx, [ebx].srcimg.dwImageWidth
		or	ecx, ecx
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
				mov	[ebx].nLeftWidth, ecx
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
				mov	ecx, [ebx].nLeftWidth
			.ENDIF
			.ELSEIF	edx != 0
				mov	[ebx].nLeftWidth, ecx
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
				mov	ecx, [ebx].nLeftWidth
				mov	BYTE PTR [edi + 2], dl
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

	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglDrawImage@DrawImage_RGBA_Blend	ENDP

;
;	RGBA32 -> RGBA32 ブレンド描画 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@DrawImage_RGBA_Blend_SSE	PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE

	mov	ebx, hDrawImage
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF

	mov	ecx, [ebx].srcimg.dwImageHeight
	mov	esi, [ebx].srcimg.ptrImageArray
	mov	edi, [ebx].dstimg.ptrImageArray
	or	ecx, ecx
	.WHILE	!ZERO?
		mov	[ebx].nLeftHeight, ecx
		mov	[ebx].ptrSrcLine, esi
		mov	[ebx].ptrDstLine, edi
		;
		mov	ecx, [ebx].srcimg.dwImageWidth
		or	ecx, ecx
		.WHILE	!ZERO?
			movzx	eax, BYTE PTR [esi + 3]
			mov	edx, 100H
			add	esi, 4
			test	eax, eax
			.IF	!ZERO?
			.IF	eax == 0FFH
				mov	edx, DWORD PTR [esi - 4]
				mov	DWORD PTR [edi], edx
			.ELSE
				pxor		mm6, mm6
				sub		edx, eax
				movd		mm1, DWORD PTR [edi]
				mov		eax, edx
				movd		mm7, edx
				movzx		edx, BYTE PTR [edi + 3]
				movd		mm0, DWORD PTR [esi - 4]
				xor		edx, 0FFH
				punpcklbw	mm1, mm6
				imul		eax, edx
				pshufw		mm7, mm7, 0
				pslld		mm0, 8
				pmullw		mm1, mm7
				shr		eax, 8
				psrlw		mm1, 8
				xor		eax, 0FFH
				pinsrw		mm1, eax, 3
				psrld		mm0, 8
				packuswb	mm1, mm6
				paddusb		mm0, mm1
				movd		DWORD PTR [edi], mm0
			.ENDIF
			.ELSEIF	(DWORD PTR [esi - 4]) != 0
				movd	mm1, DWORD PTR [edi]
				movd	mm0, DWORD PTR [esi - 4]
				paddusb	mm0, mm1
				movd	DWORD PTR [edi], mm0
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

	ASSUME	ebx:NOTHING
	emms
	xor	eax, eax
	ret

eglDrawImage@DrawImage_RGBA_Blend_SSE	ENDP

;
;	RGBA32 -> RGBA32 透明度つきブレンド描画 486 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@DrawImage_RGBA_BTrans	PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE

	mov	ebx, hDrawImage
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF

	mov	edx, [ebx].nTrans
	neg	edx
	INVOKE	eglCalculateToneTable ,
			ADDR [ebx].nBlueTone, edx, 0
	;
	xor	ecx, ecx
	.REPEAT
		mov	eax, DWORD PTR [ebx].nBlueTone[ecx]
		mov	edx, DWORD PTR [ebx].nBlueTone[ecx + 4]
		not	eax
		not	edx
		mov	DWORD PTR [ebx].nGreenTone[ecx], eax
		mov	DWORD PTR [ebx].nGreenTone[ecx + 4], edx
		add	ecx, 8
	.UNTIL	ecx >= 100H
	;
	mov	ecx, [ebx].srcimg.dwImageHeight
	mov	esi, [ebx].srcimg.ptrImageArray
	mov	edi, [ebx].dstimg.ptrImageArray
	or	ecx, ecx
	.WHILE	!ZERO?
		mov	[ebx].nLeftHeight, ecx
		mov	[ebx].ptrSrcLine, esi
		mov	[ebx].ptrDstLine, edi
		;
		mov	ecx, [ebx].srcimg.dwImageWidth
		or	ecx, ecx
		.WHILE	!ZERO?
			mov	eax, DWORD PTR [esi]
			add	esi, 4
			mov	edx, eax
			shr	eax, 24
			.IF	!ZERO?
				movzx	eax, [ebx].nGreenTone[eax]
				mov	[ebx].nLeftWidth, ecx
				inc	eax
				;
				movzx	ecx, BYTE PTR [edi]
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
				movzx	edx, BYTE PTR [edi]
				mov	ecx, [ebx].nLeftWidth
				imul	edx, eax
				not	dh
				mov	BYTE PTR [edi + 3], dh
			.ELSEIF	edx != 0
				mov	[ebx].nLeftWidth, ecx
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
				mov	ecx, [ebx].nLeftWidth
				mov	BYTE PTR [edi + 2], dl
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

	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglDrawImage@DrawImage_RGBA_BTrans	ENDP

;
;	RGBA32 -> RGBA32 透明度つきブレンド描画 SSE 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@DrawImage_RGBA_BTrans_SSE	PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE

	mov	ebx, hDrawImage
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF

	mov	edx, 100H
	sub	edx, [ebx].nTrans
	movd	mm7, edx
	pshufw	mm7, mm7, 0
	;
	mov	ecx, [ebx].srcimg.dwImageHeight
	mov	esi, [ebx].srcimg.ptrImageArray
	mov	edi, [ebx].dstimg.ptrImageArray
	or	ecx, ecx
	.WHILE	!ZERO?
		mov	[ebx].nLeftHeight, ecx
		mov	[ebx].ptrSrcLine, esi
		mov	[ebx].ptrDstLine, edi
		;
		mov	ecx, [ebx].srcimg.dwImageWidth
		or	ecx, ecx
		.WHILE	!ZERO?
			movzx	eax, BYTE PTR [esi + 3]
			add	esi, 4
			test	eax, eax
			.IF	!ZERO?
				movd		mm6, eax
				movd		mm1, DWORD PTR [edi]
				pxor		mm5, mm5
				movd		mm0, DWORD PTR [esi - 4]
				pshufw		mm6, mm6, 0
				punpcklbw	mm1, mm5
				punpcklbw	mm0, mm5
				pmullw		mm6, mm7
				pxor		mm1, mmx00FF00000000
				pcmpeqw		mm4, mm4
				pmullw		mm0, mm7
				pxor		mm6, mm4
				pmulhuw		mm1, mm6
				pand		mm0, mmx0000FFFFFFFF
				psrlw		mm0, 8
				paddusw		mm0, mm1
				packuswb	mm0, mm5
				movd		eax, mm0
				xor		eax, 0FF000000H
				mov		DWORD PTR [edi], eax
			.ELSEIF	(DWORD PTR [esi - 4]) != 0
				movd		mm0, DWORD PTR [esi - 4]
				pxor		mm4, mm4
				movd		mm1, DWORD PTR [edi]
				punpcklbw	mm0, mm4
				pmullw		mm0, mm7
				psrlw		mm0, 8
				pinsrw		mm0, eax, 3
				packuswb	mm0, mm4
				paddusb		mm0, mm1
				movd		DWORD PTR [edi], mm0
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

	ASSUME	ebx:NOTHING
	emms
	xor	eax, eax
	ret

eglDrawImage@DrawImage_RGBA_BTrans_SSE	ENDP

ENDIF

CodeSeg	ENDS

	END
