
; ----------------------------------------------------------------------------
;                          Entis Graphic Library
;						last update 2002/06/27
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


ConstSeg	ENDS


; ----------------------------------------------------------------------------
;	コードセグメント
; ----------------------------------------------------------------------------

CodeSeg	SEGMENT	PARA READONLY FLAT 'CODE'

;
;	サポートされていない機能
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@DrawImage_NotSupported	PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE

	mov	eax, eslErrNotSupported
	ret

eglDrawImage@DrawImage_NotSupported	ENDP


;
;	描画領域がない
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@DrawImage_NoDraw		PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE

	xor	eax, eax
	ret

eglDrawImage@DrawImage_NoDraw		ENDP


;
;	INDEX8 -> INDEX8 描画
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@DrawImage_8to8		PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE

	mov	ebx, hDrawImage
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF

	mov	edx, [ebx].srcimg.dwImageHeight
	mov	esi, [ebx].srcimg.ptrImageArray
	mov	edi, [ebx].dstimg.ptrImageArray
	or	edx, edx
	.WHILE	!ZERO?
		mov	[ebx].ptrDstLine, edi
		mov	[ebx].ptrSrcLine, esi
		;
		mov	ecx, [ebx].srcimg.dwImageWidth
		sub	ecx, 4
		.WHILE	!SIGN?
			mov	eax, DWORD PTR [esi]
			add	esi, 4
			mov	DWORD PTR [edi], eax
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
		mov	edi, [ebx].ptrDstLine
		mov	esi, [ebx].ptrSrcLine
		add	edi, [ebx].dstimg.dwBytesPerLine
		add	esi, [ebx].srcimg.dwBytesPerLine
		dec	edx
	.ENDW

	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglDrawImage@DrawImage_8to8		ENDP


;
;	INDEX8 -> INDEX8 クリップつき描画
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@DrawImage_8to8_Clip	PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE

	mov	ebx, hDrawImage
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF

	mov	ecx, [ebx].srcimg.dwImageHeight
	mov	esi, [ebx].srcimg.ptrImageArray
	mov	edi, [ebx].dstimg.ptrImageArray
	or	ecx, ecx
	.WHILE	!ZERO?
		mov	[ebx].nLeftHeight, ecx
		mov	[ebx].ptrDstLine, edi
		mov	[ebx].ptrSrcLine, esi
		;
		mov	ecx, [ebx].srcimg.dwImageWidth
		mov	edx, [ebx].srcimg.dwClippedPixel
		or	ecx, ecx
		.WHILE	!ZERO?
			mov	al, BYTE PTR [esi]
			inc	esi
			.IF	al != dl
				mov	BYTE PTR [edi], al
			.ENDIF
			inc	edi
			dec	ecx
		.ENDW
		;
		mov	edi, [ebx].ptrDstLine
		mov	esi, [ebx].ptrSrcLine
		mov	ecx, [ebx].nLeftHeight
		add	edi, [ebx].dstimg.dwBytesPerLine
		add	esi, [ebx].srcimg.dwBytesPerLine
		dec	ecx
	.ENDW

	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglDrawImage@DrawImage_8to8_Clip	ENDP


;
;	INDEX8 -> RGB24 通常描画
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@DrawImage_8to24		PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE

	mov	ebx, hDrawImage
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF

	mov	edx, [ebx].srcimg.pPaletteEntries
	mov	ecx, [ebx].srcimg.dwImageHeight
	mov	esi, [ebx].srcimg.ptrImageArray
	mov	edi, [ebx].dstimg.ptrImageArray
	or	ecx, ecx
	.WHILE	!ZERO?
		mov	[ebx].ptrDstLine, edi
		mov	[ebx].ptrSrcLine, esi
		mov	[ebx].nLeftHeight, ecx
		;
		mov	ecx, [ebx].srcimg.dwImageWidth
		.IF	[ebx].dstimg.dwBitsPerPixel == 24
			or	ecx, ecx
			.WHILE	!ZERO?
				movzx	eax, BYTE PTR [esi]
				inc	esi
				mov	eax, DWORD PTR [edx + eax * 4]
				mov	WORD PTR [edi], ax
				shr	eax, 16
				mov	BYTE PTR [edi], al
				add	edi, 3
				dec	ecx
			.ENDW
		.ELSE
			push	ebp
			mov	ebp, edx
			sub	ecx, 2
			.WHILE	!SIGN?
				movzx	eax, BYTE PTR [esi]
				movzx	edx, BYTE PTR [esi + 1]
				add	esi, 2
				mov	eax, DWORD PTR [ebp + eax * 4]
				mov	edx, DWORD PTR [ebp + edx * 4]
				or	eax, 0FF000000H
				or	edx, 0FF000000H
				mov	DWORD PTR [edi], eax
				mov	DWORD PTR [edi + 4], edx
				add	edi, 8
				sub	ecx, 2
			.ENDW
			add	ecx, 2
			.WHILE	!ZERO?
				movzx	eax, BYTE PTR [esi]
				inc	esi
				mov	eax, DWORD PTR [ebp + eax * 4]
				or	eax, 0FF000000H
				mov	DWORD PTR [edi], eax
				add	edi, 4
				dec	ecx
			.ENDW
			mov	edx, ebp
			pop	ebp
		.ENDIF
		;
		mov	edi, [ebx].ptrDstLine
		mov	esi, [ebx].ptrSrcLine
		mov	ecx, [ebx].nLeftHeight
		add	edi, [ebx].dstimg.dwBytesPerLine
		add	esi, [ebx].srcimg.dwBytesPerLine
		dec	ecx
	.ENDW

	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglDrawImage@DrawImage_8to24		ENDP


;
;	INDEX8 -> RGB24 透明度つき描画
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@DrawImage_8to24_Trans	PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE

	mov	ebx, hDrawImage
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF

	mov	edx, [ebx].nTrans
	sub	edx, 100H
	INVOKE	eglCalculateToneTable ,
			ADDR [ebx].nBlueTone, edx, 0
	;
	mov	edx, [ebx].nTrans
	neg	edx
	INVOKE	eglCalculateToneTable ,
			ADDR [ebx].nRedTone, edx, 0
	;
	xor	ecx, ecx
	mov	esi, [ebx].srcimg.pPaletteEntries
	.REPEAT
		ASSUME	esi:PTR EGL_PALETTE
		FOR	@MEMBER, <Blue, Green, Red>
			movzx	eax, [esi + ecx*4].rgb.@MEMBER
			mov	al, [ebx].nRedTone[eax]
			mov	[ebx].rgbaColor[ecx*4].rgb.@MEMBER, al
		ENDM
		ASSUME	esi:NOTHING
		inc	ecx
	.UNTIL	ecx >= 100H
	;
	mov	eax, [ebx].dstimg.dwBitsPerPixel
	shr	eax, 3
	mov	[ebx].nBytesPerPixel, eax
	;
	mov	ecx, [ebx].srcimg.dwImageHeight
	mov	esi, [ebx].srcimg.ptrImageArray
	mov	edi, [ebx].dstimg.ptrImageArray
	or	ecx, ecx
	.WHILE	!ZERO?
		mov	[ebx].ptrSrcLine, esi
		mov	[ebx].ptrDstLine, edi
		mov	[ebx].nLeftHeight, ecx
		;
		mov	ecx, [ebx].srcimg.dwImageWidth
		or	ecx, ecx
		.WHILE	!ZERO?
			mov	[ebx].nLeftWidth, ecx
			;
			movzx	eax, BYTE PTR [esi]
			inc	esi
			;
			movzx	ecx, BYTE PTR [edi]
			mov	dl, [ebx].rgbaColor[eax*4].rgb.Blue
			add	dl, [ebx].nBlueTone[ecx]
			mov	BYTE PTR [edi], dl
			;
			movzx	ecx, BYTE PTR [edi + 1]
			mov	dl, [ebx].rgbaColor[eax*4].rgb.Green
			add	dl, [ebx].nBlueTone[ecx]
			mov	BYTE PTR [edi + 1], dl
			;
			movzx	ecx, BYTE PTR [edi + 2]
			mov	dl, [ebx].rgbaColor[eax*4].rgb.Red
			add	dl, [ebx].nBlueTone[ecx]
			mov	BYTE PTR [edi + 2], dl
			;
			mov	ecx, [ebx].nLeftWidth
			add	edi, [ebx].nBytesPerPixel
			;
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

eglDrawImage@DrawImage_8to24_Trans	ENDP


;
;	INDEX8 -> RGB24 クリップあり描画
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@DrawImage_8to24_Clip	PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE

	mov	ebx, hDrawImage
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF

	mov	[ebx].dwSaveRegEBP, ebp
	mov	ebp, [ebx].srcimg.pPaletteEntries
	mov	ecx, [ebx].srcimg.dwImageHeight
	mov	esi, [ebx].srcimg.ptrImageArray
	mov	edi, [ebx].dstimg.ptrImageArray
	or	ecx, ecx
	.WHILE	!ZERO?
		mov	[ebx].ptrDstLine, edi
		mov	[ebx].ptrSrcLine, esi
		mov	[ebx].nLeftHeight, ecx
		;
		mov	ecx, [ebx].srcimg.dwImageWidth
		mov	edx, [ebx].srcimg.dwClippedPixel
		.IF	[ebx].dstimg.dwBitsPerPixel == 24
			or	ecx, ecx
			.WHILE	!ZERO?
				movzx	eax, BYTE PTR [esi]
				inc	esi
				.IF	al != dl
					mov	eax, DWORD PTR [ebp + eax * 4]
					mov	WORD PTR [edi], ax
					shr	eax, 16
					mov	BYTE PTR [edi], al
				.ENDIF
				add	edi, 3
				dec	ecx
			.ENDW
		.ELSE
			or	ecx, ecx
			.WHILE	!ZERO?
				movzx	eax, BYTE PTR [esi]
				inc	esi
				.IF	al != dl
					mov	eax, DWORD PTR [ebp + eax * 4]
					mov	DWORD PTR [edi], eax
				.ENDIF
				add	edi, 4
				dec	ecx
			.ENDW
		.ENDIF
		;
		mov	edi, [ebx].ptrDstLine
		mov	esi, [ebx].ptrSrcLine
		mov	ecx, [ebx].nLeftHeight
		add	edi, [ebx].dstimg.dwBytesPerLine
		add	esi, [ebx].srcimg.dwBytesPerLine
		dec	ecx
	.ENDW

	mov	ebp, [ebx].dwSaveRegEBP
	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglDrawImage@DrawImage_8to24_Clip	ENDP


;
;	INDEX8 -> RGB24 クリップあり半透明描画
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@DrawImage_8to24_CTrans	PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE

	mov	ebx, hDrawImage
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF

	mov	edx, [ebx].nTrans
	sub	edx, 100H
	INVOKE	eglCalculateToneTable ,
			ADDR [ebx].nBlueTone, edx, 0
	;
	mov	edx, [ebx].nTrans
	neg	edx
	INVOKE	eglCalculateToneTable ,
			ADDR [ebx].nRedTone, edx, 0
	;
	xor	ecx, ecx
	mov	esi, [ebx].srcimg.pPaletteEntries
	.REPEAT
		ASSUME	esi:PTR EGL_PALETTE
		FOR	@MEMBER, <Blue, Green, Red>
			movzx	eax, [esi + ecx*4].rgb.@MEMBER
			mov	al, [ebx].nRedTone[eax]
			mov	[ebx].rgbaColor[ecx*4].rgb.@MEMBER, al
		ENDM
		ASSUME	esi:NOTHING
		inc	ecx
	.UNTIL	ecx >= 100H
	;
	mov	eax, [ebx].dstimg.dwBitsPerPixel
	shr	eax, 3
	mov	[ebx].nBytesPerPixel, eax
	;
	mov	ecx, [ebx].srcimg.dwImageHeight
	mov	esi, [ebx].srcimg.ptrImageArray
	mov	edi, [ebx].dstimg.ptrImageArray
	or	ecx, ecx
	.WHILE	!ZERO?
		mov	[ebx].ptrSrcLine, esi
		mov	[ebx].ptrDstLine, edi
		mov	[ebx].nLeftHeight, ecx
		;
		mov	ecx, [ebx].srcimg.dwImageWidth
		or	ecx, ecx
		.WHILE	!ZERO?
			mov	[ebx].nLeftWidth, ecx
			;
			movzx	eax, BYTE PTR [esi]
			inc	esi
			.IF	al != (BYTE PTR [ebx].srcimg.dwClippedPixel)
				;
				movzx	ecx, BYTE PTR [edi]
				mov	dl, [ebx].rgbaColor[eax*4].rgb.Blue
				add	dl, [ebx].nBlueTone[ecx]
				mov	BYTE PTR [edi], dl
				;
				movzx	ecx, BYTE PTR [edi + 1]
				mov	dl, [ebx].rgbaColor[eax*4].rgb.Green
				add	dl, [ebx].nBlueTone[ecx]
				mov	BYTE PTR [edi + 1], dl
				;
				movzx	ecx, BYTE PTR [edi + 2]
				mov	dl, [ebx].rgbaColor[eax*4].rgb.Red
				add	dl, [ebx].nBlueTone[ecx]
				mov	BYTE PTR [edi + 2], dl
				;
			.ENDIF
			mov	ecx, [ebx].nLeftWidth
			add	edi, [ebx].nBytesPerPixel
			;
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

eglDrawImage@DrawImage_8to24_CTrans	ENDP


CodeSeg	ENDS

	END
