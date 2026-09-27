
; ----------------------------------------------------------------------------
;                          Entis Graphic Library
; ----------------------------------------------------------------------------
;   Copyright (c) 2002-2003 Leshade Entis, Entis-soft. Al rights reserved.
; ----------------------------------------------------------------------------


	.686
	.XMM
	.MODEL	FLAT

	INCLUDE	experi.inc
	INCLUDE	egl.inc

IF	0

; ----------------------------------------------------------------------------
;	データセグメント
; ----------------------------------------------------------------------------

ConstSeg	SEGMENT	PARA READONLY FLAT 'CONST'

ALIGN	10H
mmxConstFF000000	LABEL	QWORD
		DWORD	0FF000000H, 0FF000000H
mmxConst00FFFFFF	LABEL	QWORD
		DWORD	00FFFFFFH, 00FFFFFFH

ConstSeg	ENDS


; ----------------------------------------------------------------------------
;	コードセグメント
; ----------------------------------------------------------------------------

CodeSeg	SEGMENT	PARA READONLY FLAT 'CODE'

;
;	RGB32 -> RGB32 変形描画 486 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@DrawImage_RGB_X		PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE

	mov	ebx, hDrawImage
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF

	mov	ecx, [ebx].ptBasePos.x
	mov	edx, [ebx].ptBasePos.y
	mov	[ebx].ptLineBase.x, ecx
	mov	[ebx].ptLineBase.y, edx
	;
	mov	esi, [ebx].pRegion
	ASSUME	esi:PE3D_POLYGON_REGION
	mov	edi, [ebx].dstimg.ptrImageArray
	mov	ecx, [esi].nTopLine
	imul	ecx, [ebx].dstimg.dwBytesPerLine
	add	edi, ecx
	mov	[ebx].ptrDstLine, edi
	mov	ecx, [esi].nBottomLine
	sub	ecx, [esi].nTopLine
	inc	ecx
	lea	esi, [esi].plrLineRgn[0]
	ASSUME	esi:PTR E3D_POLY_LINE_REGION
	;
	.REPEAT
		mov	[ebx].ptrDstLine, edi
		mov	[ebx].ptrRegionLine, esi
		mov	[ebx].nLeftHeight, ecx
		;
		mov	eax, [esi].nLeft
		mov	ecx, [ebx].ptDltScanX.x
		mov	edx, [ebx].ptDltScanX.y
		lea	edi, [edi + eax * 4]
		imul	ecx, eax
		imul	edx, eax
		add	[ebx].ptBasePos.x, ecx
		add	[ebx].ptBasePos.y, edx
		;
		mov	ecx, [esi].nRight
		sub	ecx, [esi].nLeft
		inc	ecx
		mov	edx, [ebx].ptBasePos.y
		mov	eax, [ebx].ptBasePos.x
		ASSUME	esi:NOTHING
		.REPEAT
			mov	esi, [ebx].pSrcLineAddr
			sar	edx, 16
			sar	eax, 16
			;
			cmp	edx, [ebx].srcimg.dwImageHeight
			jae	Label_Continue
			mov	esi, DWORD PTR [esi + edx * 4]
			cmp	eax, [ebx].srcimg.dwImageWidth
			jae	Label_Continue
			;
			mov	eax, DWORD PTR [esi + eax * 4]
			or	eax, 0FF000000H
			mov	DWORD PTR [edi], eax
Label_Continue:
			add	edi, 4
			mov	edx, [ebx].ptBasePos.y
			mov	eax, [ebx].ptBasePos.x
			add	edx, [ebx].ptDltScanX.y
			add	eax, [ebx].ptDltScanX.x
			mov	[ebx].ptBasePos.y, edx
			mov	[ebx].ptBasePos.x, eax
			dec	ecx
		.UNTIL	ZERO?
		;
		mov	eax, [ebx].ptLineBase.x
		mov	edx, [ebx].ptLineBase.y
		mov	esi, [ebx].ptrRegionLine
		mov	edi, [ebx].ptrDstLine
		add	eax, [ebx].ptDltScanY.x
		add	edx, [ebx].ptDltScanY.y
		mov	ecx, [ebx].nLeftHeight
		mov	[ebx].ptBasePos.x, eax
		mov	[ebx].ptLineBase.x, eax
		mov	[ebx].ptBasePos.y, edx
		mov	[ebx].ptLineBase.y, edx
		add	esi, (SIZEOF E3D_POLY_LINE_REGION)
		add	edi, [ebx].dstimg.dwBytesPerLine
		dec	ecx
	.UNTIL	ZERO?

	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglDrawImage@DrawImage_RGB_X		ENDP

;
;	RGB32 -> RGB32 透明度つき変形描画 486 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@DrawImage_RGB_XTrans	PROC	NEAR32 C USES ebx esi edi,
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
			ADDR [ebx].nGreenTone, edx, 0
	;
	mov	ecx, [ebx].ptBasePos.x
	mov	edx, [ebx].ptBasePos.y
	mov	[ebx].ptLineBase.x, ecx
	mov	[ebx].ptLineBase.y, edx
	;
	mov	esi, [ebx].pRegion
	ASSUME	esi:PE3D_POLYGON_REGION
	mov	edi, [ebx].dstimg.ptrImageArray
	mov	ecx, [esi].nTopLine
	imul	ecx, [ebx].dstimg.dwBytesPerLine
	add	edi, ecx
	mov	[ebx].ptrDstLine, edi
	mov	ecx, [esi].nBottomLine
	sub	ecx, [esi].nTopLine
	inc	ecx
	lea	esi, [esi].plrLineRgn[0]
	ASSUME	esi:PTR E3D_POLY_LINE_REGION
	;
	.REPEAT
		mov	[ebx].ptrDstLine, edi
		mov	[ebx].ptrRegionLine, esi
		mov	[ebx].nLeftHeight, ecx
		;
		mov	eax, [esi].nLeft
		mov	ecx, [ebx].ptDltScanX.x
		mov	edx, [ebx].ptDltScanX.y
		lea	edi, [edi + eax * 4]
		imul	ecx, eax
		imul	edx, eax
		add	[ebx].ptBasePos.x, ecx
		add	[ebx].ptBasePos.y, edx
		;
		mov	ecx, [esi].nRight
		sub	ecx, [esi].nLeft
		inc	ecx
		mov	edx, [ebx].ptBasePos.y
		mov	eax, [ebx].ptBasePos.x
		ASSUME	esi:NOTHING
		.REPEAT
			mov	esi, [ebx].pSrcLineAddr
			sar	edx, 16
			sar	eax, 16
			cmp	edx, [ebx].srcimg.dwImageHeight
			jae	Label_Continue
			cmp	eax, [ebx].srcimg.dwImageWidth
			jae	Label_Continue
			;
			mov	esi, DWORD PTR [esi + edx * 4]
			mov	[ebx].nLeftWidth, ecx
			lea	esi, [esi + eax * 4]
			;
			movzx	eax, BYTE PTR [esi]
			movzx	edx, BYTE PTR [edi]
			mov	al, [ebx].nGreenTone[eax]
			add	al, [ebx].nBlueTone[edx]
			sbb	ah, ah
			movzx	ecx, BYTE PTR [esi + 1]
			movzx	edx, BYTE PTR [edi + 1]
			mov	cl, [ebx].nGreenTone[ecx]
			add	cl, [ebx].nBlueTone[edx]
			sbb	ch, ch
			or	al, ah
			or	cl, ch
			mov	BYTE PTR [edi], al
			movzx	eax, BYTE PTR [esi + 2]
			movzx	edx, BYTE PTR [edi + 2]
			mov	BYTE PTR [edi + 1], cl
			mov	al, [ebx].nGreenTone[eax]
			add	al, [ebx].nBlueTone[edx]
			movzx	edx, BYTE PTR [edi + 3]
			sbb	ah, ah
			xor	edx, 0FFH
			or	al, ah
			mov	dl, [ebx].nBlueTone[edx]
			mov	BYTE PTR [edi + 2], al
			not	dl
			mov	ecx, [ebx].nLeftWidth
			mov	BYTE PTR [edi + 3], dl
Label_Continue:		;
			add	edi, 4
			mov	edx, [ebx].ptBasePos.y
			mov	eax, [ebx].ptBasePos.x
			add	edx, [ebx].ptDltScanX.y
			add	eax, [ebx].ptDltScanX.x
			mov	[ebx].ptBasePos.y, edx
			mov	[ebx].ptBasePos.x, eax
			dec	ecx
		.UNTIL	ZERO?
		;
		mov	eax, [ebx].ptLineBase.x
		mov	edx, [ebx].ptLineBase.y
		mov	esi, [ebx].ptrRegionLine
		mov	edi, [ebx].ptrDstLine
		add	eax, [ebx].ptDltScanY.x
		add	edx, [ebx].ptDltScanY.y
		mov	ecx, [ebx].nLeftHeight
		mov	[ebx].ptBasePos.x, eax
		mov	[ebx].ptLineBase.x, eax
		mov	[ebx].ptBasePos.y, edx
		mov	[ebx].ptLineBase.y, edx
		add	esi, (SIZEOF E3D_POLY_LINE_REGION)
		add	edi, [ebx].dstimg.dwBytesPerLine
		dec	ecx
	.UNTIL	ZERO?

	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglDrawImage@DrawImage_RGB_XTrans	ENDP

;
;	RGB32 -> RGB32 透明度つき Z バッファ変形描画 486 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@DrawImage_RGB_XTZBuf	PROC	NEAR32 C USES ebx esi edi,
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
			ADDR [ebx].nGreenTone, edx, 0
	;
	mov	[ebx].dwSaveRegEBP, ebp
	;
	mov	ecx, [ebx].ptBasePos.x
	mov	edx, [ebx].ptBasePos.y
	mov	[ebx].ptLineBase.x, ecx
	mov	[ebx].ptLineBase.y, edx
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
	lea	esi, [esi].plrLineRgn[0]
	ASSUME	esi:PTR E3D_POLY_LINE_REGION
	;
	.REPEAT
		mov	[ebx].ptrDstLine, edi
		mov	[ebx].ptrZBufLine, ebp
		mov	[ebx].ptrRegionLine, esi
		mov	[ebx].nLeftHeight, ecx
		;
		mov	eax, [esi].nLeft
		mov	ecx, [ebx].ptDltScanX.x
		mov	edx, [ebx].ptDltScanX.y
		lea	edi, [edi + eax * 4]
		lea	ebp, [ebp + eax * 4]
		imul	ecx, eax
		imul	edx, eax
		add	[ebx].ptBasePos.x, ecx
		add	[ebx].ptBasePos.y, edx
		;
		mov	ecx, [esi].nRight
		sub	ecx, [esi].nLeft
		inc	ecx
		ASSUME	esi:NOTHING
		.REPEAT
			mov	[ebx].nLeftWidth, ecx
			mov	esi, [ebx].pSrcLineAddr
			mov	edx, [ebx].ptBasePos.y
			mov	eax, [ebx].ptBasePos.x
			sar	edx, 16
			sar	eax, 16
			cmp	edx, [ebx].srcimg.dwImageHeight
			jae	Label_Continue
			mov	esi, DWORD PTR [esi + edx * 4]
			cmp	eax, [ebx].srcimg.dwImageWidth
			jae	Label_Continue
			;
			mov	ecx, [ebx].rZOrder
			lea	esi, [esi + eax * 4]
			.IF	[ebx].dwFlags & EGL_WITH_Z_ORDER
				cmp	ecx, DWORD PTR [ebp]
				ja	Label_Continue
			.ENDIF
			;
			.IF	[ebx].nTrans == 0
				mov	eax, DWORD PTR [esi]
				or	eax, 0FF000000H
				mov	DWORD PTR [ebp], ecx
				mov	DWORD PTR [edi], eax
			.ELSE
				movzx	eax, BYTE PTR [esi]
				movzx	edx, BYTE PTR [edi]
				mov	al, [ebx].nGreenTone[eax]
				add	al, [ebx].nBlueTone[edx]
				sbb	ah, ah
				movzx	ecx, BYTE PTR [esi + 1]
				movzx	edx, BYTE PTR [edi + 1]
				mov	cl, [ebx].nGreenTone[ecx]
				add	cl, [ebx].nBlueTone[edx]
				sbb	ch, ch
				or	al, ah
				or	cl, ch
				mov	BYTE PTR [edi], al
				movzx	eax, BYTE PTR [esi + 2]
				movzx	edx, BYTE PTR [edi + 2]
				mov	BYTE PTR [edi + 1], cl
				mov	al, [ebx].nGreenTone[eax]
				add	al, [ebx].nBlueTone[edx]
				movzx	edx, BYTE PTR [edi + 3]
				sbb	ah, ah
				xor	edx, 0FFH
				or	al, ah
				mov	dl, [ebx].nBlueTone[edx]
				mov	BYTE PTR [edi + 2], al
				not	dl
				mov	BYTE PTR [edi + 3], dl
			.ENDIF
Label_Continue:		;
			mov	ecx, [ebx].nLeftWidth
			add	ebp, 4
			add	edi, 4
			mov	edx, [ebx].ptBasePos.y
			mov	eax, [ebx].ptBasePos.x
			add	edx, [ebx].ptDltScanX.y
			add	eax, [ebx].ptDltScanX.x
			mov	[ebx].ptBasePos.y, edx
			mov	[ebx].ptBasePos.x, eax
			;
			dec	ecx
		.UNTIL	ZERO?
		;
		mov	eax, [ebx].ptLineBase.x
		mov	edx, [ebx].ptLineBase.y
		mov	esi, [ebx].ptrRegionLine
		mov	edi, [ebx].ptrDstLine
		add	eax, [ebx].ptDltScanY.x
		add	edx, [ebx].ptDltScanY.y
		mov	ecx, [ebx].nLeftHeight
		mov	[ebx].ptBasePos.x, eax
		mov	[ebx].ptLineBase.x, eax
		mov	[ebx].ptBasePos.y, edx
		mov	[ebx].ptLineBase.y, edx
		add	esi, (SIZEOF E3D_POLY_LINE_REGION)
		add	edi, [ebx].dstimg.dwBytesPerLine
		dec	ecx
	.UNTIL	ZERO?

	mov	ebp, [ebx].dwSaveRegEBP
	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglDrawImage@DrawImage_RGB_XTZBuf	ENDP

;
;	RGB32 -> RGB32 変形描画 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@DrawImage_RGB_X_SSE	PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE

	mov	ebx, hDrawImage
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF

	mov	ecx, [ebx].ptBasePos.x
	mov	edx, [ebx].ptBasePos.y
	mov	[ebx].ptLineBase.x, ecx
	mov	[ebx].ptLineBase.y, edx
	;
	mov	esi, [ebx].pRegion
	ASSUME	esi:PE3D_POLYGON_REGION
	mov	edi, [ebx].dstimg.ptrImageArray
	mov	ecx, [esi].nTopLine
	imul	ecx, [ebx].dstimg.dwBytesPerLine
	add	edi, ecx
	mov	[ebx].ptrDstLine, edi
	mov	ecx, [esi].nBottomLine
	sub	ecx, [esi].nTopLine
	inc	ecx
	lea	esi, [esi].plrLineRgn[0]
	ASSUME	esi:PTR E3D_POLY_LINE_REGION
	;
	.REPEAT
		mov	[ebx].ptrDstLine, edi
		mov	[ebx].ptrRegionLine, esi
		mov	[ebx].nLeftHeight, ecx
		;
		mov	eax, [esi].nLeft
		mov	ecx, [ebx].ptDltScanX.x
		mov	edx, [ebx].ptDltScanX.y
		lea	edi, [edi + eax * 4]
		imul	ecx, eax
		imul	edx, eax
		add	[ebx].ptBasePos.x, ecx
		add	[ebx].ptBasePos.y, edx
		;
		mov	ecx, [esi].nRight
		movq	mm0, MMWORD PTR [ebx].ptBasePos
		sub	ecx, [esi].nLeft
		movq	mm7, MMWORD PTR [ebx].ptDltScanX
		inc	ecx
		ASSUME	esi:NOTHING
		.REPEAT
			movq	mm1, mm0
			psrad	mm0, 16
			mov	esi, [ebx].pSrcLineAddr
			paddd	mm1, mm7
			movd	eax, mm0
			psrlq	mm0, 32
			cmp	eax, [ebx].srcimg.dwImageWidth
			movd	edx, mm0
			jae	Label_Continue
			cmp	edx, [ebx].srcimg.dwImageHeight
			jae	Label_Continue
			mov	esi, DWORD PTR [esi + edx * 4]
			mov	eax, DWORD PTR [esi + eax * 4]
			or	eax, 0FF000000H
			mov	DWORD PTR [edi], eax
Label_Continue:		movq	mm0, mm1
			add	edi, 4
			dec	ecx
		.UNTIL	ZERO?
		;
		mov	eax, [ebx].ptLineBase.x
		mov	edx, [ebx].ptLineBase.y
		mov	esi, [ebx].ptrRegionLine
		mov	edi, [ebx].ptrDstLine
		add	eax, [ebx].ptDltScanY.x
		add	edx, [ebx].ptDltScanY.y
		mov	ecx, [ebx].nLeftHeight
		mov	[ebx].ptBasePos.x, eax
		mov	[ebx].ptLineBase.x, eax
		mov	[ebx].ptBasePos.y, edx
		mov	[ebx].ptLineBase.y, edx
		add	esi, (SIZEOF E3D_POLY_LINE_REGION)
		add	edi, [ebx].dstimg.dwBytesPerLine
		dec	ecx
	.UNTIL	ZERO?

	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	emms
	xor	eax, eax
	ret

eglDrawImage@DrawImage_RGB_X_SSE	ENDP

;
;	RGB32 -> RGB32 透明度つき変形描画 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@DrawImage_RGB_XTrans_SSE	PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE

	mov	ebx, hDrawImage
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF

	mov	ecx, [ebx].ptBasePos.x
	mov	edx, [ebx].ptBasePos.y
	mov	[ebx].ptLineBase.x, ecx
	mov	[ebx].ptLineBase.y, edx
	;
	mov	esi, [ebx].pRegion
	ASSUME	esi:PE3D_POLYGON_REGION
	mov	edi, [ebx].dstimg.ptrImageArray
	mov	ecx, [esi].nTopLine
	imul	ecx, [ebx].dstimg.dwBytesPerLine
	add	edi, ecx
	mov	[ebx].ptrDstLine, edi
	mov	ecx, [esi].nBottomLine
	sub	ecx, [esi].nTopLine
	inc	ecx
	lea	esi, [esi].plrLineRgn[0]
	ASSUME	esi:PTR E3D_POLY_LINE_REGION
	;
	.REPEAT
		mov	[ebx].ptrDstLine, edi
		mov	[ebx].ptrRegionLine, esi
		mov	[ebx].nLeftHeight, ecx
		;
		mov	eax, [esi].nLeft
		mov	ecx, [ebx].ptDltScanX.x
		mov	edx, [ebx].ptDltScanX.y
		lea	edi, [edi + eax * 4]
		imul	ecx, eax
		imul	edx, eax
		add	[ebx].ptBasePos.x, ecx
		add	[ebx].ptBasePos.y, edx
		;
		mov	ecx, [esi].nRight
		mov	eax, 100H
		movd	mm6, [ebx].nTrans
		sub	eax, [ebx].nTrans
		movq	mm0, MMWORD PTR [ebx].ptBasePos
		movd	mm5, eax
		sub	ecx, [esi].nLeft
		pshufw	mm6, mm6, 0
		pshufw	mm5, mm5, 0
		movq	mm7, MMWORD PTR [ebx].ptDltScanX
		inc	ecx
		pxor	mm4, mm4
		ASSUME	esi:NOTHING
		.REPEAT
			movq	mm1, mm0
			psrad	mm0, 16
			mov	esi, [ebx].pSrcLineAddr
			paddd	mm1, mm7
			movd	eax, mm0
			psrlq	mm0, 32
			cmp	eax, [ebx].srcimg.dwImageWidth
			movd	edx, mm0
			jae	Label_Continue
			cmp	edx, [ebx].srcimg.dwImageHeight
			jae	Label_Continue
			mov	esi, DWORD PTR [esi + edx * 4]
			movd		mm3, DWORD PTR [edi]
			movd		mm2, DWORD PTR [esi + eax * 4]
			pxor		mm3, mmxConstFF000000
			pand		mm2, mmxConst00FFFFFF
			punpcklbw	mm3, mm4
			punpcklbw	mm2, mm4
			pmullw		mm3, mm6
			pmullw		mm2, mm5
			paddusw		mm2, mm3
			psrlw		mm2, 8
			packuswb	mm2, mm4
			pxor		mm2, mmxConstFF000000
			movd		DWORD PTR [edi], mm2
Label_Continue:		movq	mm0, mm1
			add	edi, 4
			dec	ecx
		.UNTIL	ZERO?
		;
		mov	eax, [ebx].ptLineBase.x
		mov	edx, [ebx].ptLineBase.y
		mov	esi, [ebx].ptrRegionLine
		mov	edi, [ebx].ptrDstLine
		add	eax, [ebx].ptDltScanY.x
		add	edx, [ebx].ptDltScanY.y
		mov	ecx, [ebx].nLeftHeight
		mov	[ebx].ptBasePos.x, eax
		mov	[ebx].ptLineBase.x, eax
		mov	[ebx].ptBasePos.y, edx
		mov	[ebx].ptLineBase.y, edx
		add	esi, (SIZEOF E3D_POLY_LINE_REGION)
		add	edi, [ebx].dstimg.dwBytesPerLine
		dec	ecx
	.UNTIL	ZERO?

	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	emms
	xor	eax, eax
	ret

eglDrawImage@DrawImage_RGB_XTrans_SSE	ENDP

;
;	RGB32 -> RGB32 Z バッファつき変形描画 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@DrawImage_RGB_XZBuf_SSE	PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE

	mov	ebx, hDrawImage
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF

	mov	[ebx].dwSaveRegEBP, ebp
	;
	mov	ecx, [ebx].ptBasePos.x
	mov	edx, [ebx].ptBasePos.y
	mov	[ebx].ptLineBase.x, ecx
	mov	[ebx].ptLineBase.y, edx
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
	lea	esi, [esi].plrLineRgn[0]
	ASSUME	esi:PTR E3D_POLY_LINE_REGION
	;
	.REPEAT
		mov	[ebx].ptrDstLine, edi
		mov	[ebx].ptrZBufLine, ebp
		mov	[ebx].ptrRegionLine, esi
		mov	[ebx].nLeftHeight, ecx
		;
		mov	eax, [esi].nLeft
		mov	ecx, [ebx].ptDltScanX.x
		mov	edx, [ebx].ptDltScanX.y
		lea	edi, [edi + eax * 4]
		lea	ebp, [ebp + eax * 4]
		imul	ecx, eax
		imul	edx, eax
		add	[ebx].ptBasePos.x, ecx
		add	[ebx].ptBasePos.y, edx
		;
		mov	ecx, [esi].nRight
		movq	mm0, MMWORD PTR [ebx].ptBasePos
		sub	ecx, [esi].nLeft
		movq	mm7, MMWORD PTR [ebx].ptDltScanX
		inc	ecx
		ASSUME	esi:NOTHING
		.REPEAT
			movq	mm1, mm0
			psrad	mm0, 16
			mov	esi, [ebx].pSrcLineAddr
			paddd	mm1, mm7
			movd	eax, mm0
			psrlq	mm0, 32
			cmp	eax, [ebx].srcimg.dwImageWidth
			movd	edx, mm0
			jae	Label_Continue
			cmp	edx, [ebx].srcimg.dwImageHeight
			jae	Label_Continue
			mov	esi, DWORD PTR [esi + edx * 4]
			mov	edx, DWORD PTR [ebx].rZOrder
			cmp	edx, DWORD PTR [ebp]
			ja	Label_Continue
			mov	eax, DWORD PTR [esi + eax * 4]
			mov	DWORD PTR [ebp], edx
			or	eax, 0FF000000H
			mov	DWORD PTR [edi], eax
Label_Continue:		movq	mm0, mm1
			add	ebp, 4
			add	edi, 4
			dec	ecx
		.UNTIL	ZERO?
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

	emms
	mov	ebp, [ebx].dwSaveRegEBP
	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglDrawImage@DrawImage_RGB_XZBuf_SSE	ENDP

;
;	RGB32 -> RGB32 Z バッファ透明度つき変形描画 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@DrawImage_RGB_XTZBuf_SSE	PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE

	mov	ebx, hDrawImage
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF

	mov	[ebx].dwSaveRegEBP, ebp
	;
	mov	ecx, [ebx].ptBasePos.x
	mov	edx, [ebx].ptBasePos.y
	mov	[ebx].ptLineBase.x, ecx
	mov	[ebx].ptLineBase.y, edx
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
	lea	esi, [esi].plrLineRgn[0]
	ASSUME	esi:PTR E3D_POLY_LINE_REGION
	;
	.REPEAT
		mov	[ebx].ptrDstLine, edi
		mov	[ebx].ptrZBufLine, ebp
		mov	[ebx].ptrRegionLine, esi
		mov	[ebx].nLeftHeight, ecx
		;
		mov	eax, [esi].nLeft
		mov	ecx, [ebx].ptDltScanX.x
		mov	edx, [ebx].ptDltScanX.y
		lea	edi, [edi + eax * 4]
		lea	ebp, [ebp + eax * 4]
		imul	ecx, eax
		imul	edx, eax
		add	[ebx].ptBasePos.x, ecx
		add	[ebx].ptBasePos.y, edx
		;
		mov	ecx, [esi].nRight
		movd	mm6, [ebx].nTrans
		movq	mm0, MMWORD PTR [ebx].ptBasePos
		sub	ecx, [esi].nLeft
		pshufw	mm6, mm6, 0
		movq	mm7, MMWORD PTR [ebx].ptDltScanX
		inc	ecx
		pxor	mm5, mm5
		ASSUME	esi:NOTHING
		.REPEAT
			movq	mm1, mm0
			psrad	mm0, 16
			mov	esi, [ebx].pSrcLineAddr
			paddd	mm1, mm7
			movd	eax, mm0
			psrlq	mm0, 32
			cmp	eax, [ebx].srcimg.dwImageWidth
			movd	edx, mm0
			jae	Label_Continue
			cmp	edx, [ebx].srcimg.dwImageHeight
			jae	Label_Continue
			mov	esi, DWORD PTR [esi + edx * 4]
			mov	edx, DWORD PTR [ebx].rZOrder
			cmp	edx, DWORD PTR [ebp]
			ja	Label_Continue
			movd		mm3, DWORD PTR [edi]
			movd		mm2, DWORD PTR [esi + eax * 4]
			pxor		mm3, mmxConstFF000000
			pand		mm2, mmxConst00FFFFFF
			punpcklbw	mm3, mm5
			punpcklbw	mm2, mm5
			pmullw		mm3, mm6
			pmullw		mm2, mm6
			paddusw		mm2, mm3
			psrlw		mm2, 8
			packuswb	mm2, mm5
			pxor		mm2, mmxConstFF000000
			movd		DWORD PTR [edi], mm2
Label_Continue:		movq	mm0, mm1
			add	ebp, 4
			add	edi, 4
			dec	ecx
		.UNTIL	ZERO?
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

	emms
	mov	ebp, [ebx].dwSaveRegEBP
	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglDrawImage@DrawImage_RGB_XTZBuf_SSE	ENDP

;
;	RGBA32 -> RGB32 変形描画 486 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@DrawImage_RGBA_X		PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE

	mov	ebx, hDrawImage
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF

	mov	ecx, [ebx].ptBasePos.x
	mov	edx, [ebx].ptBasePos.y
	mov	[ebx].ptLineBase.x, ecx
	mov	[ebx].ptLineBase.y, edx
	;
	mov	esi, [ebx].pRegion
	ASSUME	esi:PE3D_POLYGON_REGION
	mov	edi, [ebx].dstimg.ptrImageArray
	mov	ecx, [esi].nTopLine
	imul	ecx, [ebx].dstimg.dwBytesPerLine
	add	edi, ecx
	mov	[ebx].ptrDstLine, edi
	mov	ecx, [esi].nBottomLine
	sub	ecx, [esi].nTopLine
	inc	ecx
	lea	esi, [esi].plrLineRgn[0]
	ASSUME	esi:PTR E3D_POLY_LINE_REGION
	;
	.REPEAT
		mov	[ebx].ptrDstLine, edi
		mov	[ebx].ptrRegionLine, esi
		mov	[ebx].nLeftHeight, ecx
		;
		mov	eax, [esi].nLeft
		mov	ecx, [ebx].ptDltScanX.x
		mov	edx, [ebx].ptDltScanX.y
		lea	edi, [edi + eax * 4]
		imul	ecx, eax
		imul	edx, eax
		add	[ebx].ptBasePos.x, ecx
		add	[ebx].ptBasePos.y, edx
		;
		mov	ecx, [esi].nRight
		sub	ecx, [esi].nLeft
		inc	ecx
		mov	edx, [ebx].ptBasePos.y
		mov	eax, [ebx].ptBasePos.x
		ASSUME	esi:NOTHING
		.REPEAT
			mov	esi, [ebx].pSrcLineAddr
			sar	edx, 16
			sar	eax, 16
			;
			cmp	edx, [ebx].srcimg.dwImageHeight
			jae	Label_Continue
			mov	esi, DWORD PTR [esi + edx * 4]
			cmp	eax, [ebx].srcimg.dwImageWidth
			lea	esi, [esi + eax * 4]
			jae	Label_Continue
			;
			movzx	edx, BYTE PTR [esi + 3]
			xor	edx, 0FFH
			mov	eax, DWORD PTR [esi]
			jnz	Label_BlendDraw
				mov	DWORD PTR [edi], eax
				jmp	Label_Continue
Label_BlendDraw:
			cmp		edx, 0FFH
			jz		Label_AddDraw
				inc	edx
				mov	[ebx].nLeftWidth, ecx
				movzx	ecx, BYTE PTR [edi]
				movzx	eax, BYTE PTR [edi + 1]
				imul	ecx, edx
				imul	eax, edx
				add	ch, BYTE PTR [esi]
				sbb	cl, cl
				add	ah, BYTE PTR [esi + 1]
				sbb	al, al
				or	cl, ch
				or	al, ah
				mov	BYTE PTR [edi], cl
				movzx	ecx, BYTE PTR [edi + 2]
				mov	BYTE PTR [edi + 1], al
				movzx	eax, BYTE PTR [edi + 3]
				imul	ecx, edx
				xor	edx, 0FFH
				xor	eax, 0FFH
				add	ch, BYTE PTR [esi + 2]
				imul	eax, edx
				sbb	cl, cl
				add	eax, 0FFH
				or	cl, ch
				mov	BYTE PTR [edi + 2], cl
				not	eax
				mov	ecx, [ebx].nLeftWidth
				mov	BYTE PTR [edi + 3], ah
				jmp	Label_Continue
Label_AddDraw:
				add	al, BYTE PTR [edi]
				sbb	dl, dl
				add	ah, BYTE PTR [edi + 1]
				sbb	dh, dh
				or	al, dl
				or	ah, dh
				mov	BYTE PTR [edi], al
				mov	al, BYTE PTR [esi + 2]
				mov	BYTE PTR [edi + 1], ah
				add	al, BYTE PTR [edi + 2]
				sbb	ah, ah
				or	al, ah
				mov	BYTE PTR [edi + 3], 0FFH
				mov	BYTE PTR [edi + 2], al
Label_Continue:
			add	edi, 4
			mov	edx, [ebx].ptBasePos.y
			mov	eax, [ebx].ptBasePos.x
			add	edx, [ebx].ptDltScanX.y
			add	eax, [ebx].ptDltScanX.x
			mov	[ebx].ptBasePos.y, edx
			mov	[ebx].ptBasePos.x, eax
			dec	ecx
		.UNTIL	ZERO?
		;
		mov	eax, [ebx].ptLineBase.x
		mov	edx, [ebx].ptLineBase.y
		mov	esi, [ebx].ptrRegionLine
		mov	edi, [ebx].ptrDstLine
		add	eax, [ebx].ptDltScanY.x
		add	edx, [ebx].ptDltScanY.y
		mov	ecx, [ebx].nLeftHeight
		mov	[ebx].ptBasePos.x, eax
		mov	[ebx].ptLineBase.x, eax
		mov	[ebx].ptBasePos.y, edx
		mov	[ebx].ptLineBase.y, edx
		add	esi, (SIZEOF E3D_POLY_LINE_REGION)
		add	edi, [ebx].dstimg.dwBytesPerLine
		dec	ecx
	.UNTIL	ZERO?

	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglDrawImage@DrawImage_RGBA_X		ENDP

;
;	RGBA32 -> RGB32 Z バッファ透明度つき変形描画 486 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@DrawImage_RGBA_XTZBuf	PROC	NEAR32 C USES ebx esi edi,
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
			ADDR [ebx].nGreenTone, edx, 0
	;
;	xor	ecx, ecx
;	.REPEAT
;		mov	eax, DWORD PTR [ebx].nGreenTone[ecx]
;		mov	edx, DWORD PTR [ebx].nGreenTone[ecx + 4]
;		not	eax
;		not	edx
;		mov	DWORD PTR [ebx].nRedTone[ecx], eax
;		mov	DWORD PTR [ebx].nRedTone[ecx + 4], edx
;		add	ecx, 8
;	.UNTIL	ecx >= 100H
	;
	mov	[ebx].dwSaveRegEBP, ebp
	;
	mov	ecx, [ebx].ptBasePos.x
	mov	edx, [ebx].ptBasePos.y
	mov	[ebx].ptLineBase.x, ecx
	mov	[ebx].ptLineBase.y, edx
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
	lea	esi, [esi].plrLineRgn[0]
	ASSUME	esi:PTR E3D_POLY_LINE_REGION
	;
	.REPEAT
		mov	[ebx].ptrDstLine, edi
		mov	[ebx].ptrZBufLine, ebp
		mov	[ebx].ptrRegionLine, esi
		mov	[ebx].nLeftHeight, ecx
		;
		mov	eax, [esi].nLeft
		mov	ecx, [ebx].ptDltScanX.x
		mov	edx, [ebx].ptDltScanX.y
		lea	edi, [edi + eax * 4]
		lea	ebp, [ebp + eax * 4]
		imul	ecx, eax
		imul	edx, eax
		add	[ebx].ptBasePos.x, ecx
		add	[ebx].ptBasePos.y, edx
		;
		mov	ecx, [esi].nRight
		sub	ecx, [esi].nLeft
		inc	ecx
		ASSUME	esi:NOTHING
		.REPEAT
			mov	[ebx].nLeftWidth, ecx
			mov	esi, [ebx].pSrcLineAddr
			mov	edx, [ebx].ptBasePos.y
			mov	eax, [ebx].ptBasePos.x
			sar	edx, 16
			sar	eax, 16
			cmp	edx, [ebx].srcimg.dwImageHeight
			jae	Label_Continue
			mov	esi, DWORD PTR [esi + edx * 4]
			cmp	eax, [ebx].srcimg.dwImageWidth
			jae	Label_Continue
			;
			mov	ecx, [ebx].rZOrder
			lea	esi, [esi + eax * 4]
			.IF	[ebx].dwFlags & EGL_WITH_Z_ORDER
				cmp	ecx, DWORD PTR [ebp]
				ja	Label_Continue
			.ENDIF
			;
			movzx	edx, BYTE PTR [esi + 3]
			test	edx, edx
			.IF	ZERO?
				movzx	eax, BYTE PTR [esi]
				movzx	ecx, BYTE PTR [esi + 1]
				movzx	edx, BYTE PTR [esi + 2]
				mov	al, [ebx].nGreenTone[eax]
				mov	cl, [ebx].nGreenTone[ecx]
				mov	dl, [ebx].nGreenTone[edx]
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
				mov	BYTE PTR [edi + 2], dl
			.ELSEIF	edx == 0FFH
				.IF	[ebx].dwFlags & EGL_WITH_Z_ORDER
					mov	DWORD PTR [ebp], ecx
				.ENDIF
				movzx	edx, BYTE PTR [edi]
				movzx	eax, BYTE PTR [esi]
				movzx	ecx, BYTE PTR [edi + 1]
				mov	dl, [ebx].nBlueTone[edx]
				mov	cl, [ebx].nBlueTone[ecx]
				add	dl, [ebx].nGreenTone[eax]
				movzx	eax, BYTE PTR [esi + 1]
				mov	BYTE PTR [edi], dl
				movzx	edx, BYTE PTR [edi + 2]
				add	cl, [ebx].nGreenTone[eax]
				movzx	eax, BYTE PTR [esi + 2]
				mov	BYTE PTR [edi + 1], cl
				movzx	ecx, BYTE PTR [edi + 3]
				mov	dl, [ebx].nBlueTone[edx]
				xor	ecx, 0FFH
				add	dl, [ebx].nGreenTone[eax]
				mov	cl, [ebx].nBlueTone[ecx]
				mov	BYTE PTR [edi + 2], dl
				not	cl
				mov	BYTE PTR [edi + 3], cl
			.ELSE
				movzx	edx, [ebx].nGreenTone[edx]
				movzx	ecx, BYTE PTR [edi]
				xor	edx, 0FFH
				movzx	eax, BYTE PTR [esi]
				imul	ecx, edx
				add	ch, [ebx].nGreenTone[eax]
				sbb	cl, cl
				or	cl, ch
				mov	BYTE PTR [edi], cl
				movzx	ecx, BYTE PTR [edi + 1]
				movzx	eax, BYTE PTR [esi + 1]
				imul	ecx, edx
				add	ch, [ebx].nGreenTone[eax]
				sbb	cl, cl
				or	cl, ch
				mov	BYTE PTR [edi + 1], cl
				movzx	ecx, BYTE PTR [edi + 2]
				movzx	eax, BYTE PTR [esi + 2]
				imul	ecx, edx
				add	ch, [ebx].nGreenTone[eax]
				movzx	eax, BYTE PTR [edi + 3]
				sbb	cl, cl
				xor	eax, 0FFH
				imul	eax, edx
				or	cl, ch
				mov	BYTE PTR [edi + 2], cl
				add	eax, 0FFH
				xor	eax, 0FF00H
				mov	BYTE PTR [edi + 3], ah
			.ENDIF
Label_Continue:		;
			mov	ecx, [ebx].nLeftWidth
			add	ebp, 4
			add	edi, 4
			mov	edx, [ebx].ptBasePos.y
			mov	eax, [ebx].ptBasePos.x
			add	edx, [ebx].ptDltScanX.y
			add	eax, [ebx].ptDltScanX.x
			mov	[ebx].ptBasePos.y, edx
			mov	[ebx].ptBasePos.x, eax
			;
			dec	ecx
		.UNTIL	ZERO?
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

	mov	ebp, [ebx].dwSaveRegEBP
	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglDrawImage@DrawImage_RGBA_XTZBuf	ENDP

;
;	RGBA32 -> RGB32 変形描画 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@DrawImage_RGBA_X_SSE	PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE

	mov	ebx, hDrawImage
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF

	mov	ecx, [ebx].ptBasePos.x
	mov	edx, [ebx].ptBasePos.y
	mov	[ebx].ptLineBase.x, ecx
	mov	[ebx].ptLineBase.y, edx
	;
	mov	esi, [ebx].pRegion
	ASSUME	esi:PE3D_POLYGON_REGION
	mov	edi, [ebx].dstimg.ptrImageArray
	mov	ecx, [esi].nTopLine
	imul	ecx, [ebx].dstimg.dwBytesPerLine
	add	edi, ecx
	mov	[ebx].ptrDstLine, edi
	mov	ecx, [esi].nBottomLine
	sub	ecx, [esi].nTopLine
	inc	ecx
	lea	esi, [esi].plrLineRgn[0]
	ASSUME	esi:PTR E3D_POLY_LINE_REGION
	;
	.REPEAT
		mov	[ebx].ptrDstLine, edi
		mov	[ebx].ptrRegionLine, esi
		mov	[ebx].nLeftHeight, ecx
		;
		mov	eax, [esi].nLeft
		mov	ecx, [ebx].ptDltScanX.x
		mov	edx, [ebx].ptDltScanX.y
		lea	edi, [edi + eax * 4]
		imul	ecx, eax
		imul	edx, eax
		add	[ebx].ptBasePos.x, ecx
		add	[ebx].ptBasePos.y, edx
		;
		pxor	mm6, mm6
		mov	ecx, [esi].nRight
		movq	mm0, MMWORD PTR [ebx].ptBasePos
		sub	ecx, [esi].nLeft
		movq	mm7, MMWORD PTR [ebx].ptDltScanX
		inc	ecx
		ASSUME	esi:NOTHING
		.REPEAT
			movq	mm1, mm0
			psrad	mm0, 16
			mov	esi, [ebx].pSrcLineAddr
			paddd	mm1, mm7
			movd	eax, mm0
			psrlq	mm0, 32
			cmp	eax, [ebx].srcimg.dwImageWidth
			movd	edx, mm0
			jae	Label_Continue
			cmp	edx, [ebx].srcimg.dwImageHeight
			jae	Label_Continue
			mov		esi, DWORD PTR [esi + edx * 4]
			movzx		edx, BYTE PTR [esi + eax * 4 + 3]
			movd		mm3, DWORD PTR [edi]
			movd		mm2, DWORD PTR [esi + eax * 4]
			xor		edx, 0FFH
				mov	esi, [ebx].dstimg.dwBytesPerLine
			jnz		Label_BlendDraw
				movd		DWORD PTR [edi], mm2
				prefetchnta	[esi + edi]
				jmp		Label_Continue
Label_BlendDraw:
			cmp		edx, 0FFH
			jz		Label_WriteContinue
				pxor		mm3, mmxConstFF000000
				movd		mm4, edx
				punpcklbw	mm3, mm6
				pshufw		mm4, mm4, 10000000B
				pmullw		mm3, mm4
				psrlw		mm3, 8
				packuswb	mm3, mm6
				pand		mm2, mmxConst00FFFFFF
				pxor		mm3, mmxConstFF000000
Label_WriteContinue:
				paddusb		mm2, mm3
				movd		DWORD PTR [edi], mm2
				prefetchnta	[esi + edi]
Label_Continue:
			movq	mm0, mm1
			add	edi, 4
			dec	ecx
		.UNTIL	ZERO?
		;
		mov	eax, [ebx].ptLineBase.x
		mov	edx, [ebx].ptLineBase.y
		mov	esi, [ebx].ptrRegionLine
		mov	edi, [ebx].ptrDstLine
		add	eax, [ebx].ptDltScanY.x
		add	edx, [ebx].ptDltScanY.y
		mov	ecx, [ebx].nLeftHeight
		mov	[ebx].ptBasePos.x, eax
		mov	[ebx].ptLineBase.x, eax
		mov	[ebx].ptBasePos.y, edx
		mov	[ebx].ptLineBase.y, edx
		add	esi, (SIZEOF E3D_POLY_LINE_REGION)
		add	edi, [ebx].dstimg.dwBytesPerLine
		dec	ecx
	.UNTIL	ZERO?

	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	emms
	xor	eax, eax
	ret

eglDrawImage@DrawImage_RGBA_X_SSE	ENDP

;
;	RGBA32 -> RGB32 透明度つき変形描画 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@DrawImage_RGBA_XTrans_SSE	PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE

	mov	ebx, hDrawImage
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF

	mov	ecx, [ebx].ptBasePos.x
	mov	edx, [ebx].ptBasePos.y
	mov	[ebx].ptLineBase.x, ecx
	mov	[ebx].ptLineBase.y, edx
	;
	mov	esi, [ebx].pRegion
	ASSUME	esi:PE3D_POLYGON_REGION
	mov	edi, [ebx].dstimg.ptrImageArray
	mov	ecx, [esi].nTopLine
	imul	ecx, [ebx].dstimg.dwBytesPerLine
	add	edi, ecx
	mov	[ebx].ptrDstLine, edi
	mov	ecx, [esi].nBottomLine
	sub	ecx, [esi].nTopLine
	inc	ecx
	lea	esi, [esi].plrLineRgn[0]
	ASSUME	esi:PTR E3D_POLY_LINE_REGION
	;
	.REPEAT
		mov	[ebx].ptrDstLine, edi
		mov	[ebx].ptrRegionLine, esi
		mov	[ebx].nLeftHeight, ecx
		;
		mov	eax, [esi].nLeft
		mov	ecx, [ebx].ptDltScanX.x
		mov	edx, [ebx].ptDltScanX.y
		lea	edi, [edi + eax * 4]
		imul	ecx, eax
		imul	edx, eax
		add	[ebx].ptBasePos.x, ecx
		add	[ebx].ptBasePos.y, edx
		;
		pxor	mm6, mm6
		mov	edx, 100H
		mov	ecx, [esi].nRight
		movq	mm0, MMWORD PTR [ebx].ptBasePos
		sub	edx, [ebx].nTrans
		sub	ecx, [esi].nLeft
		movd	mm5, edx
		mov	[ebx].dwTemp[0], edx
		movq	mm7, MMWORD PTR [ebx].ptDltScanX
		inc	ecx
		pshufw	mm5, mm5, 0
		ASSUME	esi:NOTHING
		.REPEAT
			movq	mm1, mm0
			psrad	mm0, 16
			mov	esi, [ebx].pSrcLineAddr
			paddd	mm1, mm7
			movd	eax, mm0
			psrlq	mm0, 32
			cmp	eax, [ebx].srcimg.dwImageWidth
			movd	edx, mm0
			jae	Label_Continue
			cmp	edx, [ebx].srcimg.dwImageHeight
			jae	Label_Continue
			mov		esi, DWORD PTR [esi + edx * 4]
			movzx		edx, BYTE PTR [esi + eax * 4 + 3]
			movd		mm3, DWORD PTR [edi]
			test		edx, edx
			movd		mm2, DWORD PTR [esi + eax * 4]
			mov	esi, [ebx].dstimg.dwBytesPerLine
			.IF	ZERO?
				punpcklbw	mm2, mm6
				pmullw		mm2, mm5
				psrlw		mm2, 8
				packuswb	mm2, mm6
				paddusb		mm2, mm3
			.ELSEIF	edx == 0FFH
				pand		mm2, mmxConst00FFFFFF
				pxor		mm3, mmxConstFF000000
				movd		mm4, [ebx].nTrans
				punpcklbw	mm2, mm6
				punpcklbw	mm3, mm6
				pshufw		mm4, mm4, 0
				pmullw		mm2, mm5
				pmullw		mm3, mm4
				psrlw		mm2, 8
				psrlw		mm3, 8
				paddusw		mm2, mm3
				packuswb	mm2, mm6
				pxor		mm2, mmxConstFF000000
			.ELSE
				imul		edx, [ebx].dwTemp[0]
				pand		mm2, mmxConst00FFFFFF
				pxor		mm3, mmxConstFF000000
				punpcklbw	mm2, mm6
				punpcklbw	mm3, mm6
				xor		edx, 0FFFFH
				pmullw		mm2, mm5
				movd		mm4, edx
				pshufw		mm4, mm4, 0
				pmulhuw		mm3, mm4
				psrlw		mm2, 8
				paddusw		mm2, mm3
				packuswb	mm2, mm6
				pxor		mm2, mmxConstFF000000
			.ENDIF
			movd		DWORD PTR [edi], mm2
			prefetchnta	[edi + esi]
Label_Continue:
			movq	mm0, mm1
			add	edi, 4
			dec	ecx
		.UNTIL	ZERO?
		;
		mov	eax, [ebx].ptLineBase.x
		mov	edx, [ebx].ptLineBase.y
		mov	esi, [ebx].ptrRegionLine
		mov	edi, [ebx].ptrDstLine
		add	eax, [ebx].ptDltScanY.x
		add	edx, [ebx].ptDltScanY.y
		mov	ecx, [ebx].nLeftHeight
		mov	[ebx].ptBasePos.x, eax
		mov	[ebx].ptLineBase.x, eax
		mov	[ebx].ptBasePos.y, edx
		mov	[ebx].ptLineBase.y, edx
		add	esi, (SIZEOF E3D_POLY_LINE_REGION)
		add	edi, [ebx].dstimg.dwBytesPerLine
		dec	ecx
	.UNTIL	ZERO?

	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	emms
	xor	eax, eax
	ret

eglDrawImage@DrawImage_RGBA_XTrans_SSE	ENDP

;
;	RGBA32 -> RGB32 Z バッファつき変形描画 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@DrawImage_RGBA_XZBuf_SSE	PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE

	mov	ebx, hDrawImage
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF

	mov	[ebx].dwSaveRegEBP, ebp
	;
	mov	ecx, [ebx].ptBasePos.x
	mov	edx, [ebx].ptBasePos.y
	mov	[ebx].ptLineBase.x, ecx
	mov	[ebx].ptLineBase.y, edx
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
	lea	esi, [esi].plrLineRgn[0]
	ASSUME	esi:PTR E3D_POLY_LINE_REGION
	;
	.REPEAT
		mov	[ebx].ptrDstLine, edi
		mov	[ebx].ptrZBufLine, ebp
		mov	[ebx].ptrRegionLine, esi
		mov	[ebx].nLeftHeight, ecx
		;
		mov	eax, [esi].nLeft
		mov	ecx, [ebx].ptDltScanX.x
		mov	edx, [ebx].ptDltScanX.y
		lea	edi, [edi + eax * 4]
		lea	ebp, [ebp + eax * 4]
		imul	ecx, eax
		imul	edx, eax
		add	[ebx].ptBasePos.x, ecx
		add	[ebx].ptBasePos.y, edx
		;
		pxor	mm6, mm6
		mov	ecx, [esi].nRight
		movq	mm0, MMWORD PTR [ebx].ptBasePos
		sub	ecx, [esi].nLeft
		movq	mm7, MMWORD PTR [ebx].ptDltScanX
		inc	ecx
		ASSUME	esi:NOTHING
		.REPEAT
			mov	eax, [ebx].rZOrder
			movq	mm1, mm0
			psrad	mm0, 16
			mov	esi, [ebx].pSrcLineAddr
			paddd	mm1, mm7
			cmp	eax, DWORD PTR [ebp]
			movd	eax, mm0
			ja	Label_Continue
			psrlq	mm0, 32
			cmp	eax, [ebx].srcimg.dwImageWidth
			movd	edx, mm0
			jae	Label_Continue
			cmp	edx, [ebx].srcimg.dwImageHeight
			mov	esi, DWORD PTR [esi + edx * 4]
			jae	Label_Continue
			movzx		edx, BYTE PTR [esi + eax * 4]
			movd		mm3, DWORD PTR [edi]
			xor		edx, 0FFH
			movd		mm2, DWORD PTR [esi + eax * 4]
				mov	esi, [ebx].dstimg.dwBytesPerLine
				mov	eax, [ebx].zbuf.dwBytesPerLine
			jnz		Label_BlendDraw
				mov		edx, [ebx].rZOrder
				movd		DWORD PTR [edi], mm2
				mov		DWORD PTR [ebp], edx
				prefetchnta	[edi + esi]
				prefetchnta	[ebp + eax]
				jmp		Label_Continue
Label_BlendDraw:
			cmp		edx, 0FFH
			jz		Label_WriteContinue
				pxor		mm3, mmxConstFF000000
				movd		mm4, edx
				punpcklbw	mm3, mm6
				pshufw		mm4, mm4, 0
				pmullw		mm3, mm4
				psrlw		mm3, 8
				packuswb	mm3, mm6
				pand		mm2, mmxConst00FFFFFF
				pxor		mm3, mmxConstFF000000
Label_WriteContinue:
				paddusb		mm2, mm3
				movd		DWORD PTR [edi], mm2
				prefetchnta	[edi + esi]
				prefetchnta	[ebp + eax]
Label_Continue:
			movq	mm0, mm1
			add	ebp, 4
			add	edi, 4
			dec	ecx
		.UNTIL	ZERO?
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

	emms
	mov	ebp, [ebx].dwSaveRegEBP
	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglDrawImage@DrawImage_RGBA_XZBuf_SSE	ENDP

;
;	RGBA32 -> RGB32 Z バッファ透明度つき変形描画 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@DrawImage_RGBA_XTZBuf_SSE	PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE

	mov	ebx, hDrawImage
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF

	mov	[ebx].dwSaveRegEBP, ebp
	;
	mov	ecx, [ebx].ptBasePos.x
	mov	edx, [ebx].ptBasePos.y
	mov	[ebx].ptLineBase.x, ecx
	mov	[ebx].ptLineBase.y, edx
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
	lea	esi, [esi].plrLineRgn[0]
	ASSUME	esi:PTR E3D_POLY_LINE_REGION
	;
	.REPEAT
		mov	[ebx].ptrDstLine, edi
		mov	[ebx].ptrZBufLine, ebp
		mov	[ebx].ptrRegionLine, esi
		mov	[ebx].nLeftHeight, ecx
		;
		mov	eax, [esi].nLeft
		mov	ecx, [ebx].ptDltScanX.x
		mov	edx, [ebx].ptDltScanX.y
		lea	edi, [edi + eax * 4]
		lea	ebp, [ebp + eax * 4]
		imul	ecx, eax
		imul	edx, eax
		add	[ebx].ptBasePos.x, ecx
		add	[ebx].ptBasePos.y, edx
		;
		mov	edx, 100H
		pxor	mm6, mm6
		mov	ecx, [esi].nRight
		sub	edx, [ebx].nTrans
		movq	mm0, MMWORD PTR [ebx].ptBasePos
		sub	ecx, [esi].nLeft
		movd	mm5, edx
		mov	[ebx].dwTemp[0], edx
		movq	mm7, MMWORD PTR [ebx].ptDltScanX
		inc	ecx
		pshufw	mm5, mm5, 0
		ASSUME	esi:NOTHING
		.REPEAT
			mov	eax, [ebx].rZOrder
			movq	mm1, mm0
			psrad	mm0, 16
			mov	esi, [ebx].pSrcLineAddr
			paddd	mm1, mm7
			cmp	eax, DWORD PTR [ebp]
			movd	eax, mm0
			ja	Label_Continue
			psrlq	mm0, 32
			cmp	eax, [ebx].srcimg.dwImageWidth
			movd	edx, mm0
			jae	Label_Continue
			cmp	edx, [ebx].srcimg.dwImageHeight
			jae	Label_Continue
			mov		esi, DWORD PTR [esi + edx * 4]
			movzx		edx, BYTE PTR [esi + eax * 4 + 3]
			movd		mm3, DWORD PTR [edi]
			test		edx, edx
			movd		mm2, DWORD PTR [esi + eax * 4]
			.IF	ZERO?
				punpcklbw	mm2, mm6
				pmullw		mm2, mm5
				psrlw		mm2, 8
				packuswb	mm2, mm6
				paddusb		mm2, mm3
			.ELSEIF	edx == 0FFH
				pand		mm2, mmxConst00FFFFFF
				pxor		mm3, mmxConstFF000000
				movd		mm4, [ebx].nTrans
				punpcklbw	mm2, mm6
				mov		eax, [ebx].rZOrder
				punpcklbw	mm3, mm6
				pshufw		mm4, mm4, 0
				pmullw		mm2, mm5
				pmullw		mm3, mm4
				psrlw		mm2, 8
				psrlw		mm3, 8
				mov		DWORD PTR [ebp], eax
				paddusw		mm2, mm3
				packuswb	mm2, mm6
				pxor		mm2, mmxConstFF000000
			.ELSE
				imul		edx, [ebx].dwTemp[0]
				pand		mm2, mmxConst00FFFFFF
				pxor		mm3, mmxConstFF000000
				punpcklbw	mm2, mm6
				punpcklbw	mm3, mm6
				xor		edx, 0FFFFH
				pmullw		mm2, mm5
				movd		mm4, edx
				pshufw		mm4, mm4, 0
				pmulhuw		mm3, mm4
				psrlw		mm2, 8
				paddusw		mm2, mm3
				packuswb	mm2, mm6
				pxor		mm2, mmxConstFF000000
			.ENDIF
			movd		DWORD PTR [edi], mm2
Label_Continue:
			movq	mm0, mm1
			add	ebp, 4
			add	edi, 4
			dec	ecx
		.UNTIL	ZERO?
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

	emms
	mov	ebp, [ebx].dwSaveRegEBP
	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglDrawImage@DrawImage_RGBA_XTZBuf_SSE	ENDP


CodeSeg	ENDS

ENDIF

	END
