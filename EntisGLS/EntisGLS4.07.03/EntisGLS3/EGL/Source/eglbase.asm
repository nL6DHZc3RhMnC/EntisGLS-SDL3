
; ----------------------------------------------------------------------------
;                          Entis Graphic Library
; ----------------------------------------------------------------------------
;    Copyright (c) 2002-2012 Leshade Entis, Entis-soft. Al rights reserved.
; ----------------------------------------------------------------------------


	.686
	.XMM
	.MODEL	FLAT

	INCLUDE	experi.inc
	INCLUDE	esl.inc
	INCLUDE	egl.inc


; ----------------------------------------------------------------------------
;	関数プロトタイプ
; ----------------------------------------------------------------------------

BITMAPINFOHEADER	STRUCT	1
biSize			DWORD	?
biWidth			SDWORD	?
biHeight		SDWORD	?
biPlanes		WORD	?
biBitCount		WORD	?
biCompression		DWORD	?
biSizeImage		DWORD	?
biXPelsPerMeter		SDWORD	?
biYPelsPerMeter		SDWORD	?
biClrUsed		DWORD	?
biClrImportant		DWORD	?
BITMAPINFOHEADER	ENDS
PBITMAPINFOHEADER	TYPEDEF	PTR BITMAPINFOHEADER

BI_RGB			EQU	0
BI_RLE8			EQU	1
BI_RLE4			EQU	2
BI_BITFIELDS		EQU	3

RGBQUAD			STRUCT	1
rgbBlue			BYTE	?
rgbGreen		BYTE	?
rgbRed			BYTE	?
rgbReserved		BYTE	?
RGBQUAD			ENDS
LPRGBQUAD		TYPEDEF	PTR RGBQUAD

BITMAPINFO		STRUCT	1
bmiHeader		BITMAPINFOHEADER	{}
bmiColors		RGBQUAD			1 Dup( {} )
BITMAPINFO		ENDS
LPBITMAPINFO		TYPEDEF	PTR BITMAPINFO
PBITMAPINFO		TYPEDEF	PTR BITMAPINFO

BITMAPINFO_100H		STRUCT	1
bmiHeader		BITMAPINFOHEADER	{}
bmiColors		RGBQUAD			100H Dup( {} )
BITMAPINFO_100H		ENDS

CreateCompatibleDC	PROTO	NEAR32 STDCALL, :HDC
CreateDIBSection	PROTO	NEAR32 STDCALL,
		:HDC, :PTR BITMAPINFO, :DWORD, :PTR PVOID, :PTR, :DWORD
DeleteObject		PROTO	NEAR32 STDCALL, :PTR
DeleteDC		PROTO	NEAR32 STDCALL, :PTR
SelectObject		PROTO	NEAR32 STDCALL, :HDC, :PTR
SetDIBColorTable	PROTO	NEAR32 STDCALL,
		:HDC, :DWORD, :DWORD, :PTR RGBQUAD

BitBlt			PROTO	NEAR32 STDCALL,
		:HDC, :SDWORD, :SDWORD, :SDWORD, :SDWORD,
		:HDC, :SDWORD, :SDWORD, :DWORD
StretchBlt		PROTO	NEAR32 STDCALL,
		:HDC, :SDWORD, :SDWORD, :SDWORD, :SDWORD,
		:HDC, :SDWORD, :SDWORD, :SDWORD, :SDWORD, :DWORD
StretchDIBits		PROTO	NEAR32 STDCALL,
		:HDC, :SDWORD, :SDWORD, :SDWORD, :SDWORD,
		:SDWORD, :SDWORD, :SDWORD, :SDWORD,
		:PTR, :PTR BITMAPINFO, :DWORD, :DWORD

DIB_RGB_COLORS	EQU	0	; /* color table in RGBs */
DIB_PAL_COLORS	EQU	1	; /* color table in palette indices */

SRCCOPY		EQU	00CC0020H	; /* dest = source                   */
SRCPAINT	EQU	00EE0086H	; /* dest = source OR dest           */
SRCAND		EQU	008800C6H	; /* dest = source AND dest          */
SRCINVERT	EQU	00660046H	; /* dest = source XOR dest          */
SRCERASE	EQU	00440328H	; /* dest = source AND (NOT dest )   */
NOTSRCCOPY	EQU	00330008H	; /* dest = (NOT source)             */
NOTSRCERASE	EQU	001100A6H	; /* dest = (NOT src) AND (NOT dest) */
MERGECOPY	EQU	00C000CAH	; /* dest = (source AND pattern)     */
MERGEPAINT	EQU	00BB0226H	; /* dest = (NOT source) OR dest     */
PATCOPY		EQU	00F00021H	; /* dest = pattern                  */
PATPAINT	EQU	00FB0A09H	; /* dest = DPSnoo                   */
PATINVERT	EQU	005A0049H	; /* dest = pattern XOR dest         */
DSTINVERT	EQU	00550009H	; /* dest = (NOT dest)               */
BLACKNESS	EQU	00000042H	; /* dest = BLACK                    */
WHITENESS	EQU	00FF0062H	; /* dest = WHITE                    */


; ----------------------------------------------------------------------------
;	データセグメント
; ----------------------------------------------------------------------------

DataSeg	SEGMENT	PARA FLAT 'DATA'

EGL_hImageHeap	HESLHEAP	0

eglNegateVector			PFN_eglNegateVector		OFFSET eglNegateVector_486
eglAddVector			PFN_eglAddVector		OFFSET eglAddVector_486
eglSubVector			PFN_eglSubVector		OFFSET eglSubVector_486
eglMultipleVector		PFN_eglMultipleVector		OFFSET eglMultipleVector_486
eglDivideVector			PFN_eglDivideVector		OFFSET eglDivideVector_486
eglAbsoluteVector		PFN_eglAbsoluteVector		OFFSET eglAbsoluteVector_486
eglVectorExteriorProduct	PFN_eglVectorExteriorProduct	OFFSET eglVectorExteriorProduct_486
eglVectorInnerProduct		PFN_eglVectorInnerProduct	OFFSET eglVectorInnerProduct_486
eglVectorRoundTo1		PFN_eglVectorRoundTo1		OFFSET eglVectorRoundTo1_486
eglMatrixNegate			PFN_eglMatrixNegate		OFFSET eglMatrixNegate_486
eglMatrixAdd			PFN_eglMatrixAdd		OFFSET eglMatrixAdd_486
eglMatrixSub			PFN_eglMatrixSub		OFFSET eglMatrixSub_486
eglMatrixMultiple		PFN_eglMatrixMultiple		OFFSET eglMatrixMultiple_486
eglMatrixDeterminant		PFN_eglMatrixDeterminant	OFFSET eglMatrixDeterminant_486
eglMatrixInverse		PFN_eglMatrixInverse		OFFSET eglMatrixInverse_486
eglMatrixRevolveOnX		PFN_eglMatrixRevolveOnX		OFFSET eglMatrixRevolveOnX_486
eglMatrixRevolveOnY		PFN_eglMatrixRevolveOnY		OFFSET eglMatrixRevolveOnY_486
eglMatrixRevolveOnZ		PFN_eglMatrixRevolveOnZ		OFFSET eglMatrixRevolveOnZ_486
eglMatrixRevolveByAngleOn	PFN_eglMatrixRevolveByAngleOn	OFFSET eglMatrixRevolveByAngleOn_486
eglMatrixRevolveForAngle	PFN_eglMatrixRevolveForAngle	OFFSET eglMatrixRevolveForAngle_486
eglMatrixMagnifyByVector	PFN_eglMatrixMagnifyByVector	OFFSET eglMatrixMagnifyByVector_486
eglMatrixRevolve		PFN_eglMatrixRevolve		OFFSET eglMatrixRevolve_486
eglMatrixRevolveBy		PFN_eglMatrixRevolveBy		OFFSET eglMatrixRevolveBy_486
eglMatrixRevolveVector		PFN_eglMatrixRevolveVector	OFFSET eglMatrixRevolveVector_486
eglMatrixRevolveVectors		PFN_eglMatrixRevolveVectors	OFFSET eglMatrixRevolveVectors_486
eglGetMinVector			PFN_eglGetMinVector		OFFSET eglGetMinVector_486
eglGetMaxVector			PFN_eglGetMaxVector		OFFSET eglGetMaxVector_486

DataSeg	ENDS

ConstSeg	SEGMENT	PARA READONLY FLAT 'CONST'

ALIGN	10H
eglGetPixelTable	LABEL	DWORD
	DWORD	OFFSET eglGetPixel_LBD0
	DWORD	OFFSET eglGetPixel_LBD1
	DWORD	(4-2) DUP( OFFSET eglGetPixel_LBD0 )
	DWORD	OFFSET eglGetPixel_LBD4
	DWORD	(8-5) DUP( OFFSET eglGetPixel_LBD0 )
	DWORD	OFFSET eglGetPixel_LBD8
	DWORD	(16-9) DUP( OFFSET eglGetPixel_LBD0 )
	DWORD	OFFSET eglGetPixel_LBD16
	DWORD	(24-17) DUP( OFFSET eglGetPixel_LBD0 )
	DWORD	OFFSET eglGetPixel_LBD24
	DWORD	(32-25) DUP( OFFSET eglGetPixel_LBD0 )
	DWORD	OFFSET eglGetPixel_LBD32

ALIGN	10H
eglSetPixelTable	LABEL	DWORD
	DWORD	OFFSET eglSetPixel_LBD0
	DWORD	OFFSET eglSetPixel_LBD1
	DWORD	(4-2) DUP( OFFSET eglSetPixel_LBD0 )
	DWORD	OFFSET eglSetPixel_LBD4
	DWORD	(8-5) DUP( OFFSET eglSetPixel_LBD0 )
	DWORD	OFFSET eglSetPixel_LBD8
	DWORD	(16-9) DUP( OFFSET eglSetPixel_LBD0 )
	DWORD	OFFSET eglSetPixel_LBD16
	DWORD	(24-17) DUP( OFFSET eglSetPixel_LBD0 )
	DWORD	OFFSET eglSetPixel_LBD24
	DWORD	(32-25) DUP( OFFSET eglSetPixel_LBD0 )
	DWORD	OFFSET eglSetPixel_LBD32

ALIGN	10H
rPI		REAL4	3.14159265358979323846264338327
rPI_minus	REAL4	-3.14159265358979323846264338327
rPI_x2		REAL4	6.2831853071795864769252867665
rPI_by180	REAL4	0.01745329251994329576923690768

ALIGN	10H
FactorialTable	LABEL	REAL4
		REAL4	1.0, 1.0, 2.0, 6.0
		REAL4	24.0, 120.0, 720.0, 5040.0
		REAL4	40320.0, 362880.0, 3628800.0, 39916800.0
		REAL4	479001600.0, 6227020800.0
		REAL4	87178291200.0, 1307674368000.0

ALIGN	10H
RcpFactorialTable	LABEL	REAL4
		REAL4	1.0, 1.0, 0.5
		REAL4	0.16666666666666666666666666667
		REAL4	0.04166666666666666666666666667
		REAL4	0.00833333333333333333333333333
		REAL4	0.00138888888888888888888888889
		REAL4	0.00019841269841269841269841269
		REAL4	2.48015873015873015873015873e-5
		REAL4	2.75573192239858906525573192e-6
		REAL4	2.75573192239858906525573192e-7
		REAL4	2.50521083854417187750521083e-8
		REAL4	2.08767569878680989792100903e-9
		REAL4	1.6059043836821614599392377e-10
		REAL4	1.1470745597729724713851697e-11
		REAL4	7.6471637318198164759011319e-13

ALIGN	10H
realConst1	REAL4	1.0

ALIGN	10H
xmmMaskLow3SS	LABEL	REAL4
	DWORD	3 DUP(-1), 0

ALIGN	10H
xmmRevolveX_SignMask	LABEL	REAL4
	DWORD	0, 0, 80000000H, 0

ALIGN	10H
xmmRevolveY_SignMask	LABEL	REAL4
	DWORD	0, 0, 80000000H, 0

ALIGN	10H
xmmRevolveZ_SignMask	LABEL	REAL4
	DWORD	0, 80000000H, 0, 0

ConstSeg	ENDS


; ----------------------------------------------------------------------------
;	コードセグメント
; ----------------------------------------------------------------------------

CodeSeg	SEGMENT	PARA READONLY FLAT 'CODE'

;
;	画像バッファ作成
; ----------------------------------------------------------------------------
ALIGN	10H
eglCreateImageBuffer	PROC	NEAR32 C USES ebx esi edi,
		fdwFormat:DWORD, dwWidth:DWORD, dwHeight:DWORD,
		dwBitsPerPixel:DWORD, dwFlags:DWORD

	LOCAL	pImageInf:PEGL_IMAGE_INFO
	LOCAL	dwBufSize:DWORD, dwLineBytes:DWORD, dwImageBytes:DWORD
	LOCAL	dwPaletteOffset:DWORD, dwLineAddrOffset:DWORD
	LOCAL	dwImageArrayOffset:DWORD
	LOCAL	hDC:HDC, hBitmap:HBITMAP, ptrBits:PVOID
	LOCAL	bmiInfo:BITMAPINFO_100H

	;
	; メモリサイズを計算
	;
	.IF	(SDWORD PTR dwWidth) <= 0
		mov	dwWidth, 1
	.ENDIF
	.IF	(SDWORD PTR dwHeight) <= 0
		mov	dwHeight, 1
	.ENDIF
	mov	eax, ((SIZEOF EGL_IMAGE_BUFF) + 0FH) AND (NOT 0FH)
	mov	dwLineAddrOffset, eax
	mov	ecx, dwHeight
	lea	eax, [eax + ecx * 4 + 0FH]
	and	eax, NOT 0FH
	mov	dwPaletteOffset, eax
	mov	ecx, dwBitsPerPixel
	.IF	ecx <= 8
		mov	edx, 1
		shl	edx, cl
		lea	eax, [eax + edx * 4 + 0FH]
		and	eax, NOT 0FH
	.ENDIF
	mov	dwBufSize, eax
	;
	.IF	dwFlags & EGL_IMAGE_HAS_DC
		;
		; ビットマップ情報を設定
		;
		mov	eax, dwWidth
		.IF	fdwFormat & EIF_SIDE_BY_SIDE
			add	eax, eax
		.ENDIF
		mul	dwBitsPerPixel
		add	eax, 007FH
		and	eax, NOT 007FH
		shr	eax, 3
		mov	dwLineBytes, eax
		mul	dwHeight
		add	eax, dwLineBytes
		mov	dwImageBytes, eax
		;
		mov	bmiInfo.bmiHeader.biSize, SIZEOF BITMAPINFOHEADER
		mov	eax, dwLineBytes
		xor	edx, edx
		shl	eax, 3
		div	dwBitsPerPixel
		mov	bmiInfo.bmiHeader.biWidth, eax
		mov	edx, dwHeight
		inc	edx
		mov	bmiInfo.bmiHeader.biHeight, edx
		mov	bmiInfo.bmiHeader.biPlanes, 1
		mov	edx, dwBitsPerPixel
		mov	bmiInfo.bmiHeader.biBitCount, dx
		mov	bmiInfo.bmiHeader.biCompression, 0
		mov	eax, dwImageBytes
		mov	bmiInfo.bmiHeader.biSizeImage, eax
		mov	bmiInfo.bmiHeader.biXPelsPerMeter, 0
		mov	bmiInfo.bmiHeader.biYPelsPerMeter, 0
		mov	bmiInfo.bmiHeader.biClrUsed, 0
		mov	bmiInfo.bmiHeader.biClrImportant, 0
		;
		; DIB セクションを作成
		;
		INVOKE	CreateCompatibleDC , 0
		mov	hDC, eax
		;
		INVOKE	CreateDIBSection ,
			hDC, ADDR bmiInfo, DIB_RGB_COLORS, ADDR ptrBits, 0, 0
		mov	hBitmap, eax
		;
		; メモリサイズを計算
		;
		.IF	eax == NULL ; ptrBits & 0FH
			and	dwFlags, NOT EGL_IMAGE_HAS_DC
			INVOKE	DeleteDC , hDC
			INVOKE	DeleteObject , hBitmap
		.ELSE
			INVOKE	SelectObject , hDC, hBitmap
			;
			mov	eax, ((SIZEOF EGL_IMAGE_BUFF) + 0FH) AND (NOT 0FH)
			mov	dwLineAddrOffset, eax
			mov	ecx, dwHeight
			lea	eax, [eax + ecx * 4 + 0FH]
			and	eax, NOT 0FH
			mov	dwPaletteOffset, eax
			mov	ecx, dwBitsPerPixel
			.IF	ecx <= 8
				mov	edx, 1
				shl	edx, cl
				lea	eax, [eax + edx * 4 + 0FH]
				and	eax, NOT 0FH
			.ENDIF
			mov	dwBufSize, eax
		.ENDIF
	.ENDIF
	.IF	!(dwFlags & EGL_IMAGE_HAS_DC)
		;
		; 画像バッファのサイズを計算
		;
		mov	eax, dwWidth
		.IF	fdwFormat & EIF_SIDE_BY_SIDE
			add	eax, eax
		.ENDIF
		mul	dwBitsPerPixel
		add	eax, 007FH
		adc	edx, 0
		.IF	!ZERO?
			xor	eax, eax
			ret
		.ENDIF
		and	eax, NOT 007FH
		shr	eax, 3
		mov	dwLineBytes, eax
		mul	dwHeight
		add	eax, dwLineBytes
		mov	dwImageBytes, eax
		;
		; 全メモリサイズを計算
		;
		mov	eax, dwBufSize
		mov	dwImageArrayOffset, eax
		add	eax, dwImageBytes
		mov	dwBufSize, eax
	.ENDIF
	;
	; メモリを確保
	;
	.IF	EGL_hImageHeap == NULL
		INVOKE	eslHeapCreate , 0, 0, ESL_HEAP_ZERO_INIT, NULL
		mov	EGL_hImageHeap, eax
	.ENDIF
	;
	mov	edx, dwBufSize
	add	edx, 20H
	INVOKE	eslHeapAllocate ,
			EGL_hImageHeap, edx, ESL_HEAP_ZERO_INIT
	.IF	eax == NULL
		ret
	.ENDIF
	lea	ebx, [eax + 1FH]
	and	ebx, NOT 0FH
	mov	pImageInf, ebx
	mov	[ebx - 10H], eax
	;
	; パラメータ初期化
	;
	ASSUME	ebx:PTR EGL_IMAGE_BUFF
	;
	mov	[ebx].dwInfoSize, (SIZEOF EGL_IMAGE_BUFF)
	mov	[ebx].ptrOffsetPixel, 0
	mov	[ebx].pPaletteEntries, NULL
	mov	[ebx].dwPaletteCount, 0
	mov	[ebx].dwClippedPixel, 0
	;
	mov	eax, fdwFormat
	mov	ecx, dwWidth
	mov	edx, dwHeight
	mov	edi, dwBitsPerPixel
	mov	[ebx].fdwFormatType, eax
	mov	[ebx].dwImageWidth, ecx
	mov	[ebx].dwImageHeight, edx
	mov	[ebx].dwBitsPerPixel, edi
	;
	.IF	edi <= 8
		mov	ecx, edi
		mov	edx, 1
		shl	edx, cl
		mov	eax, dwPaletteOffset
		add	eax, ebx
		mov	[ebx].pPaletteEntries, eax
		mov	[ebx].dwPaletteCount, edx
		;
		xor	ecx, ecx
		.REPEAT
			mov	DWORD PTR [eax], ecx
			add	ecx, 00010101H
			add	eax, 4
			dec	edx
		.UNTIL	ZERO?
	.ENDIF
	;
	mov	eax, dwLineBytes
	mov	ecx, dwImageBytes
	mov	edx, dwFlags
	mov	[ebx].dwBytesPerLine, eax
	mov	[ebx].dwSizeOfImage, ecx
	mov	[ebx].dwFlags, edx
	;
	; 画像バッファ設定
	;
	.IF	edx & EGL_IMAGE_HAS_DC
		mov	eax, hDC
		mov	ecx, hBitmap
		mov	edx, ptrBits
		mov	[ebx].hDC, eax
		add	edx, dwLineBytes
		mov	[ebx].hBitmap, ecx
		mov	[ebx].ptrImageArray, edx
		;
		.IF	[ebx].dwPaletteCount != 0
			INVOKE	SetDIBColorTable ,
				[ebx].hDC, 0,
					[ebx].dwPaletteCount,
					[ebx].pPaletteEntries
		.ENDIF
	.ELSE
		mov	eax, dwImageArrayOffset
		add	eax, ebx
		mov	[ebx].ptrImageArray, eax
	.ENDIF
	;
	; ラインアドレスリストを初期化
	;
	mov	edi, dwLineAddrOffset
	add	edi, ebx
	mov	ecx, [ebx].dwImageHeight
	mov	[ebx].pLineAddrEntry, edi
	mov	eax, [ebx].ptrImageArray
	mov	edx, [ebx].dwBytesPerLine
	test	ecx, ecx
	.WHILE	!ZERO?
		mov	DWORD PTR [edi], eax
		add	edi, SIZEOF DWORD
		add	eax, edx
		dec	ecx
	.ENDW
	;
	; 画像サイズのマスクを作成する
	;
	mov	ecx, [ebx].dwImageWidth
	mov	edx, [ebx].dwImageHeight
	inc	ecx
	inc	edx
	bsr	ecx, ecx
	bsr	edx, edx
	mov	eax, 1
	shl	eax, cl
	dec	eax
	mov	[ebx].dwWidthMask, eax
	mov	eax, 1
	mov	ecx, edx
	shl	eax, cl
	dec	eax
	mov	[ebx].dwHeightMask, eax

	ASSUME	ebx:NOTHING
	;
	mov	eax, ebx
	ret

eglCreateImageBuffer	ENDP

;
;	テクスチャ画像情報を構築
; ----------------------------------------------------------------------------
eglCreateTextureInfo	PROC	NEAR32 C USES ebx esi edi,
	pImageInf:PCEGL_IMAGE_INFO, pClipRect:PCEGL_RECT, dwFlags:DWORD

	LOCAL	eiiClip:EGL_IMAGE_INFO
	LOCAL	irClip:EGL_IMAGE_RECT

	mov	esi, pImageInf
	mov	edi, pClipRect
	ASSUME	esi:PCEGL_IMAGE_INFO
	ASSUME	edi:PCEGL_RECT
	;
	; クリッピング領域の取得
	;
	.IF	edi == NULL
		xor	eax, eax
		mov	irClip.x, eax
		mov	irClip.y, eax
		mov	ecx, [esi].dwImageWidth
		mov	edx, [esi].dwImageHeight
		mov	irClip.w, ecx
		mov	irClip.h, edx
	.ELSE
		mov	ecx, [edi].right
		mov	edx, [edi].bottom
		mov	eax, [edi].left
		mov	ebx, [edi].top
		inc	ecx
		inc	edx
		mov	irClip.x, eax
		mov	irClip.y, ebx
		sub	ecx, eax
		sub	edx, ebx
		mov	irClip.w, ecx
		mov	irClip.h, edx
	.ENDIF
	or	ecx, edx
	.IF	SIGN?
		xor	eax, eax
		ret
	.ENDIF
	test	ecx, edx
	.IF	ZERO?
		xor	eax, eax
		ret
	.ENDIF
	ASSUME	edi:NOTHING
	;
	; 画像バッファの複製の判定
	;
	.IF	([esi].dwBitsPerPixel != 32) && !(dwFlags & EGL_IMAGE_NO_DUP)
		;
		; クリップされた元画像の情報を取得する
		;
		INVOKE	eglGetClippedImageInfo , ADDR eiiClip, esi, ADDR irClip
		.IF	eax != eslErrSuccess
			xor	eax, eax
			ret
		.ENDIF
		;
		; 画像バッファを生成する
		;
		mov	edx, EIF_RGB_BITMAP
		.IF	[esi].fdwFormatType & \
				(EIF_WITH_CLIPPING OR EIF_WITH_ALPHA)
			mov	edx, EIF_RGBA_BITMAP
		.ENDIF
		INVOKE	eglCreateImageBuffer ,
				edx, irClip.w, irClip.h, 32, dwFlags
		mov	ebx, eax
		ASSUME	ebx:PEGL_IMAGE_INFO
		;
		; バッファを複製する
		;
		.IF	eax != NULL
			INVOKE	eglConvertFormat , ebx, ADDR eiiClip, 0
		.ENDIF
		mov	eax, ebx
		ret
	.ENDIF
	;
	; メモリを確保
	;
	.IF	EGL_hImageHeap == NULL
		INVOKE	eslHeapCreate , 0, 0, ESL_HEAP_ZERO_INIT, NULL
		mov	EGL_hImageHeap, eax
	.ENDIF
	;
	mov	ecx, irClip.h
	mov	edx, ((SIZEOF EGL_IMAGE_BUFF) + 0FH) AND (NOT 0FH)
	lea	edx, [edx + ecx * 4 + 0FH]
	and	edx, NOT 0FH
	add	edx, 20H
	;
	INVOKE	eslHeapAllocate ,
			EGL_hImageHeap, edx, ESL_HEAP_ZERO_INIT
	lea	ebx, [eax + 1FH]
	and	ebx, NOT 0FH
	mov	PVOID PTR [ebx - 10H], eax
	;
	; パラメータ初期化
	;
	INVOKE	eglGetClippedImageInfo , ebx, esi, ADDR irClip
	ASSUME	esi:NOTHING
	ASSUME	ebx:PTR EGL_IMAGE_BUFF
	mov	[ebx].dwInfoSize, (SIZEOF EGL_IMAGE_BUFF)
	;
	; ラインアドレスリストを初期化
	;
	lea	edi, [ebx + (((SIZEOF EGL_IMAGE_BUFF) + 0FH) AND (NOT 0FH))]
	mov	ecx, [ebx].dwImageHeight
	mov	[ebx].pLineAddrEntry, edi
	mov	eax, [ebx].ptrImageArray
	mov	edx, [ebx].dwBytesPerLine
	test	ecx, ecx
	.WHILE	!ZERO?
		mov	DWORD PTR [edi], eax
		add	edi, SIZEOF DWORD
		add	eax, edx
		dec	ecx
	.ENDW
	;
	; 画像サイズのマスクを作成する
	;
	mov	ecx, [ebx].dwImageWidth
	mov	edx, [ebx].dwImageHeight
	inc	ecx
	inc	edx
	bsr	ecx, ecx
	bsr	edx, edx
	mov	eax, 1
	shl	eax, cl
	dec	eax
	mov	[ebx].dwWidthMask, eax
	mov	eax, 1
	mov	ecx, edx
	shl	eax, cl
	dec	eax
	mov	[ebx].dwHeightMask, eax

	ASSUME	ebx:NOTHING
	;
	mov	eax, ebx
	ret

eglCreateTextureInfo	ENDP

;
;	画像バッファを複製
; ----------------------------------------------------------------------------
ALIGN	10H
eglDuplicateImageBuffer	PROC	NEAR32 C USES ebx esi edi,
		pImageInf:PCEGL_IMAGE_INFO, dwFlags:DWORD

	LOCAL	draw_param:EGL_DRAW_PARAM

	mov	esi, pImageInf
	ASSUME	esi:PCEGL_IMAGE_INFO
	;
	; 画像バッファを作成
	;
	INVOKE	eglCreateImageBuffer ,
			[esi].fdwFormatType, [esi].dwImageWidth,
			[esi].dwImageHeight, [esi].dwBitsPerPixel, dwFlags
	mov	ebx, eax
	.IF	eax == NULL
		ret
	.ENDIF
	ASSUME	ebx:PCEGL_IMAGE_INFO
	.IF	[esi].dwBytesPerLine < 0
		INVOKE	eglReverseVertically , ebx
	.ENDIF
	.IF	([esi].fdwFormatType & EIF_WITH_PALETTE) \
			&& ([esi].pPaletteEntries != NULL) \
			&& ([ebx].pPaletteEntries != NULL)
		mov	eax, [esi].dwPaletteCount
		mov	edx, [ebx].dwPaletteCount
		.IF	eax > edx
			mov	eax, edx
		.ENDIF
		shl	eax, 2
		INVOKE	eslMoveMemory ,
			[ebx].pPaletteEntries, [esi].pPaletteEntries, eax
	.ENDIF
	ASSUME	esi:NOTHING
	;
	; 画像を複製
	;
	INVOKE	eglCreateDrawImage
	;
	mov	edi, eax
	ASSUME	edi:PTR EGL_DRAW_IMAGE
	;
	INVOKE	[edi].pfnInitialize , edi, ebx, NULL, NULL
	;
	mov	eax, pImageInf
	mov	draw_param.dwFlags, 0
	mov	draw_param.ptBasePos.x, 0
	mov	draw_param.ptBasePos.y, 0
	mov	draw_param.pSrcImage, eax
	mov	draw_param.pViewRect, NULL
	mov	draw_param.nTransparency, 0
	mov	draw_param.pImageAxes, NULL
	;
	INVOKE	[edi].pfnPrepareDraw , edi, ADDR draw_param
	INVOKE	[edi].pfnDrawImage , edi
	INVOKE	[edi].pfnRelease , edi
	;
	ASSUME	ebx:NOTHING
	;
	mov	eax, ebx
	ret

eglDuplicateImageBuffer	ENDP

;
;	ステレオ画像の左バッファ参照を取得する
; ----------------------------------------------------------------------------
ALIGN	10H
eglGetStereoLeftImageBuffer	PROC	NEAR32 C USES ebx esi edi,
	pImageInf:PCEGL_IMAGE_INFO, pLeftImage:PEGL_IMAGE_INFO

	mov	ebx, pImageInf
	mov	edi, pLeftImage
	.IF	(ebx == NULL) || (edi == NULL)
		mov	eax, eslErrGeneral
		ret
	.ENDIF
	ASSUME	ebx:PTR EGL_IMAGE_BUFF
	ASSUME	edi:PTR EGL_IMAGE_INFO

	.IF	!([ebx].fdwFormatType & EIF_SIDE_BY_SIDE)
		mov	eax, eslErrGeneral
		ret
	.ENDIF

	mov	[edi].dwInfoSize, (SIZEOF EGL_IMAGE_INFO)

	FOR	@MEMBER, <fdwFormatType, ptrOffsetPixel, \
			ptrImageArray, pPaletteEntries, dwPaletteCount, \
			dwImageWidth, dwImageHeight, dwBitsPerPixel, \
			dwBytesPerLine, dwSizeOfImage, dwClippedPixel>
		mov	eax, [ebx].@MEMBER
		mov	[edi].@MEMBER, eax
	ENDM

	mov	eax, [edi].dwImageWidth
	imul	eax, [edi].dwBitsPerPixel
	shr	eax, 3
	add	eax, [edi].ptrImageArray
	mov	[edi].ptrImageArray, eax

	ASSUME	ebx:NOTHING
	ASSUME	edi:NOTHING
	xor	eax, eax
	ret

eglGetStereoLeftImageBuffer	ENDP

;
;	画像バッファ参照カウンタを加算する
; ----------------------------------------------------------------------------
ALIGN	10H
eglAddImageBufferRef	PROC	NEAR32 C USES ebx esi edi,
	pImageInf:PEGL_IMAGE_INFO

	mov	ebx, pImageInf
	.IF	ebx == NULL
		mov	eax, eslErrGeneral
		ret
	.ENDIF
	ASSUME	ebx:PTR EGL_IMAGE_BUFF
	.IF	(ebx & 0FH) || ([ebx].dwInfoSize < (SIZEOF EGL_IMAGE_BUFF))
		TRACE	<"無効な画像バッファを参照しようとしました。", 0AH>
		mov	eax, eslErrGeneral
		ret
	.ENDIF
	;
	lock	inc	DWORD PTR [ebx - 0CH]
	;
	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglAddImageBufferRef	ENDP

;
;	画像バッファを削除
; ----------------------------------------------------------------------------
ALIGN	10H
eglDeleteImageBuffer	PROC	NEAR32 C USES ebx esi edi,
		pImageInf:PEGL_IMAGE_INFO

	;
	; ポインタの検証
	;
	mov	ebx, pImageInf
	.IF	ebx == NULL
		mov	eax, eslErrGeneral
		ret
	.ENDIF
	ASSUME	ebx:PTR EGL_IMAGE_BUFF
	.IF	(ebx & 0FH) || ([ebx].dwInfoSize < (SIZEOF EGL_IMAGE_BUFF))
		TRACE	<"無効な画像バッファを解放しようとしました。", 0AH>
		mov	eax, eslErrGeneral
		ret
	.ENDIF
	;
	; 参照カウンタデクリメント
	;
	mov	eax, -1
	lock	xadd	DWORD PTR [ebx - 0CH], eax
	.IF	(SDWORD PTR eax) > 0
		xor	eax, eax
		ret
	.ENDIF
	;
	; コールバック関数へ通知
	;
	mov	esi, [ebx].pInterface
	.WHILE	esi != NULL
		ASSUME	esi:PEGL_IMAGE_BUFFER_INTERFACE
		mov	edi, [esi].pNextInterface
		INVOKE	[esi].pfnRelease, ebx, esi
		mov	esi, edi
	.ENDW
	;
	; DIB オブジェクトを削除
	;
	.IF	[ebx].dwFlags & EGL_IMAGE_HAS_DC
		INVOKE	DeleteDC , [ebx].hDC
		INVOKE	DeleteObject , [ebx].hBitmap
	.ENDIF
	;
	; メモリを解放
	;
	ASSUME	ebx:NOTHING
	INVOKE	eslHeapFree , EGL_hImageHeap, (PVOID PTR [ebx - 10H]), 0

	xor	eax, eax
	ret

eglDeleteImageBuffer	ENDP

;
;	画像をデバイスコンテキストに描画
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawToDC		PROC	NEAR32 C PUBLIC USES ebx esi edi,
		hDstDC:HDC, pImageInf:PEGL_IMAGE_INFO,
		nPosX:SDWORD, nPosY:SDWORD,
		pSizeToDraw:PCEGL_SIZE, pViewRect:PCEGL_RECT

	LOCAL	sizeDst:EGL_SIZE, rectView:EGL_IMAGE_RECT
	LOCAL	ptrImageArray:PVOID
	LOCAL	nSrcBytesPerLine:DWORD, fTopDownFlag:DWORD
	LOCAL	bmiSrc:BITMAPINFO_100H

	xor	eax, eax
	mov	ebx, pImageInf
	ASSUME	ebx:PEGL_IMAGE_INFO
	.IF	[ebx].dwInfoSize >= (SIZEOF EGL_IMAGE_BUFF)
		mov	eax, (EGL_IMAGE_BUFF PTR [ebx]).dwFlags
	.ENDIF
	;
	.IF	!(eax & EGL_IMAGE_HAS_DC)
		;
		; BITMAPINFO 構造体に変換
		;
		mov	eax, [ebx].dwImageWidth
		mov	ecx, [ebx].dwImageHeight
		mov	edx, [ebx].dwBitsPerPixel
		mov	bmiSrc.bmiHeader.biSize, SIZEOF BITMAPINFOHEADER
		mov	bmiSrc.bmiHeader.biWidth, eax
		mov	bmiSrc.bmiHeader.biHeight, ecx
		mov	bmiSrc.bmiHeader.biPlanes, 1
		mov	bmiSrc.bmiHeader.biBitCount, dx
		mov	bmiSrc.bmiHeader.biCompression, 0
		mov	bmiSrc.bmiHeader.biXPelsPerMeter, 0
		mov	bmiSrc.bmiHeader.biYPelsPerMeter, 0
		mov	bmiSrc.bmiHeader.biClrUsed, 0
		mov	bmiSrc.bmiHeader.biClrImportant, 0
		;
		; 左下のアドレスを取得
		;
		mov	eax, [ebx].dwBytesPerLine
		mov	edx, [ebx].ptrImageArray
		or	eax, eax
		.IF	SIGN?
			dec	ecx
			imul	eax, ecx
			add	edx, eax
			mov	eax, [ebx].dwBytesPerLine
			neg	eax
		.ENDIF
		mov	ptrImageArray, edx
		;
		; 実際のメモリ上の幅を計算
		;
		mov	ecx, eax
		imul	ecx, [ebx].dwImageHeight
		xor	edx, edx
		shld	edx, eax, 3
		shl	eax, 3
		div	[ebx].dwBitsPerPixel
		mov	bmiSrc.bmiHeader.biSizeImage, ecx
		mov	bmiSrc.bmiHeader.biWidth, eax
		;
		; 描画元矩形を計算
		;
		mov	esi, pViewRect
		.IF	esi != NULL
			ASSUME	esi:PCEGL_RECT
			mov	eax, [esi].left
			mov	edx, [esi].top
			mov	rectView.x, eax
			mov	rectView.y, edx
			mov	eax, [esi].right
			mov	edx, [esi].bottom
			sub	eax, [esi].left
			sub	edx, [esi].top
			inc	eax
			inc	edx
			ASSUME	esi:NOTHING
		.ELSE
			mov	rectView.x, 0
			mov	rectView.y, 0
			mov	eax, [ebx].dwImageWidth
			mov	edx, [ebx].dwImageHeight
		.ENDIF
		;
		mov	rectView.w, eax
		mov	rectView.h, edx
		;
		.IF	((SDWORD PTR eax) <= 0) || ((SDWORD PTR edx) <= 0)
			xor	eax, eax
			ret
		.ENDIF
		;
		; 出力先サイズを計算
		;
		mov	esi, pSizeToDraw
		.IF	esi != NULL
			ASSUME	esi:PCEGL_SIZE
			mov	eax, [esi].w
			mov	edx, [esi].h
			ASSUME	esi:NOTHING
		.ENDIF
		;
		mov	sizeDst.w, eax
		mov	sizeDst.h, edx
		;
		; パレットテーブルを複製
		;
		mov	esi, [ebx].pPaletteEntries
		mov	ecx, [ebx].dwPaletteCount
		.IF	(esi != NULL) && (ecx != 0)
			.IF	ecx > 100H
				mov	ecx, 100H
			.ENDIF
			lea	edi, bmiSrc.bmiColors[0]
			.REPEAT
				mov	eax, DWORD PTR [esi]
				add	esi, (SIZEOF DWORD)
				mov	DWORD PTR [edi], eax
				add	edi, (SIZEOF DWORD)
				dec	ecx
			.UNTIL	ZERO?
		.ENDIF
		;
		; 描画
		;
		mov	eax, [ebx].dwImageHeight
		mov	edx, rectView.y
		add	edx, rectView.h
		sub	eax, edx
		mov	rectView.y, eax
		;
		INVOKE	StretchDIBits ,
			hDstDC, nPosX, nPosY, sizeDst.w, sizeDst.h,
			rectView.x, rectView.y, rectView.w, rectView.h,
			ptrImageArray, ADDR bmiSrc, DIB_RGB_COLORS, SRCCOPY
		;
		.IF	eax == 0FFFFFFFFH
			mov	eax, eslErrGeneral
		.Else
			xor	eax, eax
		.ENDIF
	.ELSE
		ASSUME	ebx:PTR EGL_IMAGE_BUFF
		;
		; 描画元矩形を計算
		;
		mov	esi, pViewRect
		.IF	esi != NULL
			ASSUME	esi:PCEGL_RECT
			mov	eax, [esi].left
			mov	edx, [esi].top
			mov	rectView.x, eax
			mov	rectView.y, edx
			mov	eax, [esi].right
			mov	edx, [esi].bottom
			sub	eax, [esi].left
			sub	edx, [esi].top
			inc	eax
			inc	edx
			ASSUME	esi:NOTHING
		.ELSE
			mov	rectView.x, 0
			mov	rectView.y, 0
			mov	eax, [ebx].dwImageWidth
			mov	edx, [ebx].dwImageHeight
		.ENDIF
		;
		mov	rectView.w, eax
		mov	rectView.h, edx
		;
		.IF	((SDWORD PTR eax) <= 0) || ((SDWORD PTR edx) <= 0)
			xor	eax, eax
			ret
		.ENDIF
		;
		; 出力先サイズを取得
		;
		mov	esi, pSizeToDraw
		.IF	esi != NULL
			ASSUME	esi:PCEGL_SIZE
			mov	ecx, [esi].w
			mov	edx, [esi].h
			.IF	(ecx == rectView.w) && (edx == rectView.h)
				xor	esi, esi
			.ENDIF
			ASSUME	esi:NOTHING
		.ENDIF
		.IF	esi != NULL
			;
			; ストレッチ描画
			;
			ASSUME	esi:PCEGL_SIZE
			INVOKE	StretchBlt ,
				hDstDC, nPosX, nPosY,
				[esi].w, [esi].h,
				[ebx].hDC, rectView.x, rectView.y,
				rectView.w, rectView.h, SRCCOPY
			.IF	eax != 0
				xor	eax, eax
			.ELSE
				mov	eax, eslErrGeneral
			.ENDIF
			ASSUME	esi:NOTHING
		.ELSE
			;
			; 描画
			;
			INVOKE	BitBlt ,
				hDstDC, nPosX, nPosY,
				rectView.w, rectView.h,
				[ebx].hDC, rectView.x, rectView.y, SRCCOPY
			.IF	eax != 0
				xor	eax, eax
			.ELSE
				mov	eax, eslErrGeneral
			.ENDIF
		.ENDIF
	.ENDIF
	;
	ASSUME	ebx:NOTHING
	ret

eglDrawToDC		ENDP

;
;	デバイスコンテキストを取得する
; ----------------------------------------------------------------------------
ALIGN	10H
eglGetDC		PROC	NEAR32 C USES ebx esi edi,
	pImageInf:PEGL_IMAGE_INFO

	mov	ebx, pImageInf
	ASSUME	ebx:PTR EGL_IMAGE_BUFF
	xor	eax, eax
	.IF	[ebx].dwInfoSize == (SIZEOF EGL_IMAGE_BUFF)
		.IF	[ebx].dwFlags & EGL_IMAGE_HAS_DC
			mov	eax, [ebx].hDC
		.ENDIF
	.ENDIF
	ASSUME	ebx:NOTHING
	ret

eglGetDC		ENDP

;
;	画像を指定のカラーコードで塗りつぶす
; ----------------------------------------------------------------------------
ALIGN	10H
eglFillImage		PROC	NEAR32 C USES ebx esi edi,
	pImageInf:PEGL_IMAGE_INFO, colorFill:EGL_PALETTE

	LOCAL	dwTemp:DWORD

	mov	ebx, pImageInf
	ASSUME	ebx:PTR EGL_IMAGE_BUFF
	.IF	[ebx].dwInfoSize < (SIZEOF EGL_IMAGE_BUFF)
		TRACE	\
			<"eglFillImage 関数は、画像バッファの", \
				"一部に対してフィルを行えません。", 0AH>
		mov	eax, eslErrGeneral
		ret
	.ENDIF

	mov	ecx, [ebx].dwBitsPerPixel
	mov	eax, colorFill.dwPixelCode
	xor	edx, edx
	.IF	ecx == 8
		and	eax, 0FFH
		mov	edx, eax
		shl	eax, 8
		or	eax, edx
		mov	edx, eax
		shl	eax, 16
		or	eax, edx
		xor	edx, edx
	.ELSEIF	ecx == 16
		and	eax, 0FFFFH
		mov	edx, eax
		shl	eax, 16
		or	eax, edx
		xor	edx, edx
	.ELSEIF	ecx == 24
		inc	edx
	.ELSEIF	ecx != 32
		TRACE	<"未対応のフォーマットです。", 0AH>
		mov	eax, eslErrGeneral
		ret
	.ENDIF
	;
	mov	edi, [ebx].ptrImageArray
	mov	esi, [ebx].dwBytesPerLine
	;
	.IF	edx == 0
	.IF	!(edi & 0FH) && !(esi & 0FH) && \
			(ERI_EnabledProcessorType & ERI_USE_XMM_P3)
		;
		; XMM 専用フィル
		;
		mov	dwTemp, eax
		movss	xmm0, dwTemp
		shufps	xmm0, xmm0, 00000000B
		;
		mov	ecx, esi
		sar	ecx, 4
		.IF	SIGN?
			neg	ecx
		.ENDIF
		;
		mov	edx, [ebx].dwImageHeight
		or	edx, edx
		.WHILE	!ZERO?
			mov	eax, ecx
			mov	dwTemp, edi
			or	eax, eax
			.WHILE	!ZERO?
				movntps	[edi], xmm0
				add	edi, 16
				dec	eax
			.ENDW
			mov	edi, dwTemp
			add	edi, esi
			dec	edx
		.ENDW
		;
		sfence
	.ELSE
		;
		; 486 互換フィル (DWORD 単位の転送)
		;
		imul	ecx, [ebx].dwImageWidth
		shr	ecx, 3 + 2
		;
		mov	edx, [ebx].dwImageHeight
		or	edx, edx
		.WHILE	!ZERO?
			mov	ebx, ecx
			mov	dwTemp, edi
			or	ebx, ebx
			.WHILE	!ZERO?
				mov	DWORD PTR [edi], eax
				add	edi, 4
				dec	ebx
			.ENDW
			mov	edi, dwTemp
			add	edi, esi
			dec	edx
		.ENDW
	.ENDIF
	.ELSE
		;
		; 486 互換フィル (ピクセル単位の転送)
		;
		ASSERT	<[ebx].dwBitsPerPixel == 24>, "[ebx].dwBitsPerPixel == 24"
		mov	edx, [ebx].dwImageHeight
		or	edx, edx
		.WHILE	!ZERO?
			mov	ecx, [ebx].dwImageWidth
			mov	dwTemp, edi
			or	ecx, ecx
			.WHILE	!ZERO?
				mov	WORD PTR [edi], ax
				ror	eax, 16
				mov	BYTE PTR [edi + 2], al
				add	edi, 3
				ror	eax, 16
				dec	ecx
			.ENDW
			mov	edi, dwTemp
			add	edi, esi
			dec	edx
		.ENDW
	.ENDIF

	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglFillImage		ENDP

;
;	クリップした画像情報を取得
; ----------------------------------------------------------------------------
ALIGN	10H
eglGetClippedImageInfo		PROC	NEAR32 C USES ebx esi edi,
		pClippedImage:PEGL_IMAGE_INFO,
		pOriginalImage:PCEGL_IMAGE_INFO,
		pClippingRect:PCEGL_IMAGE_RECT

	LOCAL	rectClip:EGL_IMAGE_RECT

	mov	ebx, pClippingRect
	ASSUME	ebx:PCEGL_IMAGE_RECT
	;
	; 矩形を正規化
	;
	mov	ecx, [ebx].x
	mov	edx, [ebx].y
	mov	eax, [ebx].w
	mov	edi, [ebx].h
	test	ecx, ecx
	.IF	SIGN?
		add	eax, ecx
		xor	ecx, ecx
	.ENDIF
	test	edx, edx
	.IF	SIGN?
		add	edi, edx
		xor	edx, edx
	.ENDIF
	mov	rectClip.x, ecx
	mov	rectClip.y, edx
	;
	; サイズを正規化
	;
	mov	esi, pOriginalImage
	ASSUME	esi:PCEGL_IMAGE_INFO
	add	ecx, eax
	add	edx, edi
	.IF	esi == NULL
		mov	eax, eslErrGeneral
		ret
	.ENDIF
	.IF	(SDWORD PTR ecx) > (SDWORD PTR [esi].dwImageWidth)
		mov	ecx, [esi].dwImageWidth
	.ENDIF
	.IF	(SDWORD PTR edx) > (SDWORD PTR [esi].dwImageHeight)
		mov	edx, [esi].dwImageHeight
	.ENDIF
	sub	ecx, rectClip.x
	sub	edx, rectClip.y
	mov	rectClip.w, ecx
	mov	rectClip.h, edx
	ASSUME	ebx:NOTHING
	;
	or	ecx, edx
	.IF	SIGN?
		TRACE	<"クリップされた矩形が空です。", 0AH>
		mov	eax, eslErrGeneral
		ret
	.ENDIF
	test	ecx, edx
	.IF	ZERO?
		TRACE	<"クリップされた矩形が空です。", 0AH>
		mov	eax, eslErrGeneral
		ret
	.ENDIF
	;
	; 出力先の画像情報を複製
	;
	mov	edi, pClippedImage
	ASSUME	edi:PEGL_IMAGE_INFO
	;
	mov	[edi].dwInfoSize, SIZEOF EGL_IMAGE_INFO
	FOR	@MEMBER, <fdwFormatType, ptrOffsetPixel, \
			pPaletteEntries, dwPaletteCount, \
			dwBitsPerPixel, dwBytesPerLine, dwClippedPixel>
		mov	eax, [esi].@MEMBER
		mov	[edi].@MEMBER, eax
	ENDM
	;
	; 画像サイズとオフセットを計算
	;
	mov	eax, [esi].ptrImageArray
	mov	edx, rectClip.y
	mov	ecx, rectClip.x
	imul	edx, [esi].dwBytesPerLine
	imul	ecx, [esi].dwBitsPerPixel
	shr	ecx, 3
	add	edx, ecx
	add	eax, edx
	mov	[edi].ptrImageArray, eax
	;
	mov	eax, rectClip.w
	mov	edx, rectClip.h
	mov	[edi].dwImageWidth, eax
	mov	[edi].dwImageHeight, edx
	;
	imul	edx, [esi].dwBytesPerLine
	mov	[edi].dwSizeOfImage, edx

	ASSUME	edi:NOTHING
	ASSUME	esi:NOTHING
	xor	eax, eax
	ret

eglGetClippedImageInfo		ENDP

;
;	重なっている矩形を取得
; ----------------------------------------------------------------------------
ALIGN	10H
eglGetOverlappedRectangle	PROC	NEAR32 C USES ebx esi edi,
		pImageRect:PEGL_IMAGE_RECT,
		pDstViewRect:PCEGL_RECT, pSrcViewRect:PCEGL_RECT,
		pSrcViewOffset:PCEGL_POINT, pSrcViewMaxSize:PCEGL_SIZE

	LOCAL	ptDst:EGL_POINT

	mov	edi, pImageRect
	ASSUME	edi:PEGL_IMAGE_RECT
	;
	; 入力矩形を変換
	;
	mov	esi, pSrcViewRect
	ASSUME	esi:PCEGL_RECT
	.IF	esi != NULL
		mov	ecx, [esi].right
		mov	edx, [esi].bottom
		mov	eax, [esi].left
		mov	ebx, [esi].top
		inc	ecx
		inc	edx
		test	eax, eax
		.IF	SIGN?
			xor	eax, eax
		.ENDIF
		test	ebx, ebx
		.IF	SIGN?
			xor	ebx, ebx
		.ENDIF
	.ELSE
		mov	esi, pSrcViewMaxSize
		xor	eax, eax
		xor	ebx, ebx
		.IF	esi == NULL
			xor	eax, eax
			ret
		.ENDIF
		ASSUME	esi:PCEGL_SIZE
		mov	ecx, [esi].w
		mov	edx, [esi].h
		ASSUME	esi:NOTHING
	.ENDIF
	mov	[edi].x, eax
	mov	[edi].y, ebx
	;
	; 入力最大サイズを適用
	;
	mov	esi, pSrcViewMaxSize
	ASSUME	esi:PCEGL_SIZE
	.IF	esi != NULL
		.IF	(SDWORD PTR ecx) > [esi].w
			mov	ecx, [esi].w
		.ENDIF
		.IF	(SDWORD PTR edx) > [esi].h
			mov	edx, [esi].h
		.ENDIF
	.ENDIF
	sub	ecx, eax
	sub	edx, ebx
	mov	[edi].w, ecx
	mov	[edi].h, edx
	;
	; 出力先オフセットを適用
	;
	mov	esi, pSrcViewOffset
	ASSUME	esi:PCEGL_POINT
	.IF	esi != NULL
		mov	eax, [esi].x
		mov	ebx, [esi].y
		lea	ecx, [ecx + eax - 1]
		lea	edx, [edx + ebx - 1]
	.ELSE
		xor	eax, eax
		xor	ebx, ebx
		dec	ecx
		dec	edx
	.ENDIF
	;
	mov	ptDst.x, eax
	mov	ptDst.y, ebx
	;
	; 出力先クリップエリアを適用
	;
	mov	esi, pDstViewRect
	ASSUME	esi:PCEGL_RECT
	.IF	esi != NULL
		.IF	(SDWORD PTR ecx) > [esi].right
			sub	ecx, [esi].right
			sub	[edi].w, ecx
		.ENDIF
		.IF	(SDWORD PTR edx) > [esi].bottom
			sub	edx, [esi].bottom
			sub	[edi].h, edx
		.ENDIF
		.IF	(SDWORD PTR eax) < [esi].left
			sub	eax, [esi].left
			sub	[edi].x, eax
			add	[edi].w, eax
		.ENDIF
		.IF	(SDWORD PTR ebx) < [esi].top
			sub	ebx, [esi].top
			sub	[edi].y, ebx
			add	[edi].h, ebx
		.ENDIF
	.ENDIF
	ASSUME	esi:NOTHING
	;
	mov	ecx, [edi].w
	mov	edx, [edi].h
	mov	eax, edi
	.IF	((SDWORD PTR ecx) <= 0) || ((SDWORD PTR edx) <= 0)
		xor	eax, eax
	.ENDIF

	ASSUME	edi:NOTHING
	ret

eglGetOverlappedRectangle	ENDP

;
;	回転軸を計算
; ----------------------------------------------------------------------------
ALIGN	10H
eglGetRevolvedAxes		PROC	NEAR32 C USES ebx esi edi,
		pImageAxes:PEGL_IMAGE_AXES,
		pBasePosition:PEGL_POINT, pCenterOffset:PCEGL_POINT,
		rHorzRate:REAL4, rVertRate:REAL4,
		rRevAngle:REAL4, rCrossAngle:REAL4, nRadianFlag:DWORD

	LOCAL	nInt180:SDWORD

	mov	edi, pImageAxes
	ASSUME	edi:PEGL_IMAGE_AXES

IF	0
	.IF	ERI_EnabledProcessorType & ERI_USE_XMM_P3
		;
		; 回転角をラジアン単位に変換
		;
		movss	xmm6, rRevAngle
		movss	xmm7, rCrossAngle
		movss	xmm0, rPI_by180
		addss	xmm7, xmm6
		;
		.IF	!(nRadianFlag & EGL_DRAW_RADIAN)
			mulss	xmm6, xmm0
			mulss	xmm7, xmm0
		.ENDIF
		;
		; ｘ軸を計算
		;
		movss	xmm0, xmm6
		call	eglSineCosineSSE
		movss	xmm6, rHorzRate
		shufps	xmm6, xmm6, 0
		mulps	xmm6, xmm0
		movlps	QWORD PTR [edi].xAxis, xmm6
		;
		; ｙ軸を計算
		;
		movss	xmm0, xmm7
		call	eglSineCosineSSE
		movss	xmm7, rVertRate
		shufps	xmm7, xmm7, 0
		mulps	xmm7, xmm0
		movlps	QWORD PTR [edi].yAxis, xmm7
		;
		; 描画開始座標を計算
		;
		mov	esi, pCenterOffset
		mov	ebx, pBasePosition
		ASSUME	ebx:PEGL_POINT
		.IF	esi != NULL
			cvtpi2ps	xmm1, QWORD PTR [esi]
			movlhps		xmm6, xmm7
			unpcklps	xmm1, xmm1
			mulps		xmm1, xmm6
			cvtpi2ps	xmm0, QWORD PTR [ebx]
			subps		xmm0, xmm1
			movhlps		xmm1, xmm1
			subps		xmm0, xmm1
			cvtss2si	eax, xmm0
			shufps		xmm0, xmm0, 1
			mov		[ebx].x, eax
			cvtss2si	eax, xmm0
			mov		[ebx].y, eax
		.ENDIF
		xor	eax, eax
		ret

	.ELSE
ENDIF
		;
		; 回転角をラジアン単位に変換
		;
		fld	rCrossAngle
		fld	rRevAngle
		fadd	st(1), st
		;
		.IF	!(nRadianFlag & EGL_DRAW_RADIAN)
			fld	rPI_by180
			fmul	st(2), st
			fmulp	st(1), st
		.ENDIF
		;
		; ｘ軸を計算
		;
		fsincos
		fld	rHorzRate
		fmul	st(2), st
		fmulp	st(1), st
		fstp	[edi].xAxis.x
		fstp	[edi].xAxis.y
		;
		; ｙ軸を計算
		;
		fsincos
		fld	rVertRate
		fmul	st(2), st
		fmulp	st(1), st
		fstp	[edi].yAxis.x
		fstp	[edi].yAxis.y
		;
		; 描画開始座標を計算
		;
		mov	esi, pCenterOffset
		mov	ebx, pBasePosition
		ASSUME	esi:PCEGL_POINT
		ASSUME	ebx:PEGL_POINT
		.IF	esi != NULL
			fild	[ebx].y
			fild	[ebx].x
			;
			fld	[edi].xAxis.y
			fld	[edi].xAxis.x
			fild	[esi].x
			fchs
			fmul	st(2), st
			fmulp	st(1), st
			faddp	st(2), st
			faddp	st(2), st
			;
			fld	[edi].yAxis.y
			fld	[edi].yAxis.x
			fild	[esi].y
			fchs
			fmul	st(2), st
			fmulp	st(1), st
			faddp	st(2), st
			faddp	st(2), st
			;
			fistp	[ebx].x
			fistp	[ebx].y
		.ENDIF
		ASSUME	esi:NOTHING
		ASSUME	ebx:NOTHING
		ASSUME	edi:NOTHING
;	.ENDIF
	xor	eax, eax
	ret

eglGetRevolvedAxes		ENDP


;
;	cos, sin を計算
; ----------------------------------------------------------------------------
ALIGN	10H
;
; SSE 専用
; input;
;	xmm00 <- radian
; output;
;	xmm00 <- cos(radian)
;	xmm01 <- sin(radian)
;	xmm02, xmm03 <- 不定
; destroyed;
;	xmm1～xmm4, eax
;
eglSineCosineSSE	PROC	NEAR32 C

	; sin(x) = x - x^3/(3*2) + x^5/(5*4*3*2) - x^7/(7*6*5*4*3*2) …
	; cos(x) = 1 - x^2/2 + x^4/(4*3*2) - x^6/(6*5*4*3*2) …

	comiss	xmm0, rPI
	ja	Label_OutOfRange
	comiss	xmm0, rPI_minus
	jb	Label_OutOfRange
Label_ReturnOutOfRange:
	movss	xmm4, xmm0
	movss	xmm0, FactorialTable[0]
	movss	xmm1, xmm4
	shufps	xmm4, xmm4, 0
	mulps	xmm4, xmm4
	unpcklps	xmm0, xmm1
	movaps	xmm1, xmm0
	movaps	xmm2, xmm4		; xmm2 = x^2
	mulps	xmm1, xmm4
	mulps	xmm4, xmm4		; xmm4 = x^4
	movlhps	xmm0, xmm1		; xmm0 = 1, x, x^2, x^3
	;
	movaps	xmm1, xmm0
	mulps	xmm0, RcpFactorialTable[0]
	;
	mulps	xmm1, xmm4		; xmm1 = x^4, x^5, x^6, x^7
	movaps	xmm2, xmm1
	mulps	xmm1, RcpFactorialTable[10H]
	;
	mulps	xmm2, xmm4		; xmm2 = x^8, x^9, x^10, x^11
	movaps	xmm3, xmm2
	mulps	xmm2, RcpFactorialTable[20H]
	;
	mulps	xmm3, xmm4
	mulps	xmm3, RcpFactorialTable[30H]
	;
	addps	xmm0, xmm1
	addps	xmm2, xmm3
	addps	xmm0, xmm2
	;
	movhlps	xmm1, xmm0
	subps	xmm0, xmm1
	ret

Label_OutOfRange:
	movss	xmm1, xmm0
	divss	xmm1, rPI_x2
	cvtss2si	eax, xmm1
	cvtsi2ss	xmm1, eax
	mulss	xmm1, rPI_x2
	subss	xmm0, xmm1
	jmp	Label_ReturnOutOfRange

eglSineCosineSSE	ENDP


ALIGN	10H
eglSineCosine	PROC	NEAR32 C,
	pCosSin:PTR REAL4, rRadian:REAL4

	.IF	ERI_EnabledProcessorType & ERI_USE_XMM_P3
		movss	xmm0, rRadian
		call	eglSineCosineSSE
		mov	eax, pCosSin
		movlps	QWORD PTR [eax], xmm0
		ret
	.ELSE
		mov	eax, pCosSin
		fld	rRadian
		fsincos
		fstp	REAL4 PTR [eax]
		fstp	REAL4 PTR [eax + 4]
	.ENDIF

	ret


eglSineCosine	ENDP


;
;	座標マッピングから基底ベクトル算出
; ----------------------------------------------------------------------------
ALIGN	10H
eglBaseVectorFromMapping2D	PROC	NEAR32 C USES ebx esi edi,
	pBaseVector:PEGL_IMAGE_AXES,
	pvMappedOrigin:PE3D_VECTOR_2D,
	pvMapped:PCE3D_VECTOR_2D, pvMapping:PCE3D_VECTOR_2D

	LOCAL	u1:E3D_VECTOR_2D, u2:E3D_VECTOR_2D
	LOCAL	v1:E3D_VECTOR_2D, v2:E3D_VECTOR_2D
	LOCAL	d:REAL8, t:REAL4

	; u1 = pvMapping[1] - pvMapping[0]
	; u2 = pvMapping[2] - pvMapping[0]
	; v1 = pvMapped[1] - pvMapped[0]
	; v2 = pvMapped[2] - pvMapped[0]
	; d = u1.x * u2.y - u1.y * u2.x
	; a : pBaseVector->xAxis
	; b : pBaseVector->yAxis
	; a.x = (v1.x * u2.y - v2.x * u1.y) / d
	; a.y = (v1.y * u2.y - v2.y * u1.y) / d
	; b.x = (v2.x * u1.x - v1.x * u2.x) / d
	; b.y = (v2.y * u1.x - v1.y * u2.x) / d
	; o : pvMappedOrigin
	; o = pvMapped[0] - (pvMapping[0].x * a + pvMapping[0].y * b)

	mov	esi, pvMapping
	mov	edi, pvMapped
	mov	ebx, pBaseVector
	mov	ecx, pvMappedOrigin
	ASSUME	esi:PCE3D_VECTOR_2D
	ASSUME	edi:PCE3D_VECTOR_2D
	ASSUME	ebx:PEGL_IMAGE_AXES
	ASSUME	ecx:PE3D_VECTOR_2D

	.IF	ERI_EnabledProcessorType & ERI_USE_XMM_P3
		movlps	xmm0, QWORD PTR [esi]
		movlps	xmm1, QWORD PTR [edi]
		movups	xmm4, XMMWORD_PTR [esi + 8]
		movups	xmm5, XMMWORD_PTR [edi + 8]
		shufps	xmm0, xmm0, 01000100B
		shufps	xmm1, xmm1, 01000100B
		subps	xmm4, xmm0		; xmm4 = u2 : u1
		subps	xmm5, xmm1		; xmm5 = v2 : v1
		;
		movaps	xmm0, xmm4
		movaps	xmm1, xmm4
		movaps	xmm2, xmm4
		movaps	xmm6, xmm5		; xmm6 = v2.y : v2.x : v1.y : v1.x
		;
			shufps	xmm4, xmm4, 1011B
		;
		shufps	xmm0, xmm0, 00001111B	; xmm0 = u1.x : u1.x : u2.y : u2.y
			mulps	xmm4, xmm2
		shufps	xmm1, xmm1, 10100101B	; xmm1 = u2.x : u2.x : u1.y : u1.y
		shufps	xmm5, xmm5, 01001110B	; xmm5 = v1.y : v1.x : v2.y : v2.x
		mulps	xmm0, xmm6
			movss	xmm2, xmm4
			shufps	xmm4, xmm4, 01B
		mulps	xmm1, xmm5
			subss	xmm2, xmm4		; xmm2 = d
		subps	xmm0, xmm1
			shufps	xmm2, xmm2, 0
		divps	xmm0, xmm2		; xmm0 = b : a
		;
		movups	[ebx], xmm0
		;
		movlps	xmm4, QWORD PTR [esi]
		movlps	xmm5, QWORD PTR [edi]
		unpcklps	xmm4, xmm4
		mulps	xmm0, xmm4
		movhlps	xmm1, xmm0
		addps	xmm0, xmm1
		subps	xmm5, xmm0
		movlps	QWORD PTR [ecx], xmm5
		;
		xor	eax, eax
		ret

	.ELSE
		@INDEX = 8
		FOR	@DST, <u1, u2>
			FOR	@MEMBER, <x, y>
				fld	[esi + @INDEX].@MEMBER
				fsub	[esi].@MEMBER
				fstp	@DST.@MEMBER
			ENDM
			@INDEX = @INDEX + 8
		ENDM
		@INDEX = 8
		FOR	@DST, <v1, v2>
			FOR	@MEMBER, <x, y>
				fld	[edi + @INDEX].@MEMBER
				fsub	[edi].@MEMBER
				fstp	@DST.@MEMBER
			ENDM
			@INDEX = @INDEX + 8
		ENDM
		;
		fld	u1.x
		fmul	u2.y
		fld	u1.y
		fmul	u2.x
		fsubp	st(1), st
		fst	t
		mov	eax, t
		and	eax, 7FFFFFFFH
		.IF	eax < 37800000H		; eax < 2^-16
			fstp	st
			mov	eax, eslErrGeneral
			ret
		.ENDIF
		fld1
		fdivrp	st(1), st
		fstp	d
		;
		FOR	@MEMBER, <x, y>
			fld	v1.@MEMBER
			fmul	u2.y
			fld	v2.@MEMBER
			fmul	u1.y
			fsubp	st(1), st
			fmul	d
			fstp	[ebx].xAxis.@MEMBER
		ENDM
		FOR	@MEMBER, <x, y>
			fld	v2.@MEMBER
			fmul	u1.x
			fld	v1.@MEMBER
			fmul	u2.x
			fsubp	st(1), st
			fmul	d
			fstp	[ebx].yAxis.@MEMBER
		ENDM
		;
		FOR	@MEMBER, <x, y>
			fld	[esi].x
			fmul	[ebx].xAxis.@MEMBER
			fld	[esi].y
			fmul	[ebx].yAxis.@MEMBER
			faddp	st(1), st
			fld	[edi].@MEMBER
			fsubrp	st(1), st
			fstp	[ecx].@MEMBER
		ENDM

	.ENDIF

	ASSUME	esi:NOTHING
	ASSUME	edi:NOTHING
	ASSUME	ebx:NOTHING
	ASSUME	ecx:NOTHING

	xor	eax, eax
	ret

eglBaseVectorFromMapping2D	ENDP


;
;	基底ベクトルから写像ベクトル計算
; ----------------------------------------------------------------------------
ALIGN	10H
eglMapping2DVectors		PROC	NEAR32 C USES ebx esi edi,
	pBaseVector:PCEGL_IMAGE_AXES,
	pvMappedOrigin:PCE3D_VECTOR_2D,
	pvMapped:PE3D_VECTOR_2D, pvMapping:PCE3D_VECTOR_2D, nCount:DWORD

	mov	esi, pvMapping
	mov	edi, pvMapped
	mov	ebx, pBaseVector
	mov	ecx, pvMappedOrigin
	mov	edx, nCount
	ASSUME	esi:PCE3D_VECTOR_2D
	ASSUME	edi:PCE3D_VECTOR_2D
	ASSUME	ebx:PEGL_IMAGE_AXES
	ASSUME	ecx:PE3D_VECTOR_2D

	.IF	ERI_EnabledProcessorType & ERI_USE_XMM_P3
		movups	xmm7, XMMWORD_PTR [ebx]
		movlps	xmm6, QWORD PTR [ecx]
		shufps	xmm6, xmm6, 01000100B
		;
		sub	edx, 2
		.WHILE	!SIGN?
			movups	xmm0, XMMWORD_PTR [esi]
			add	esi, (SIZEOF E3D_VECTOR_2D) * 2
			;
			movaps	xmm1, xmm0
			unpcklps	xmm0, xmm0
			unpckhps	xmm1, xmm1
			;
			mulps	xmm0, xmm7
			mulps	xmm1, xmm7
			movaps	xmm2, xmm0
			movlhps	xmm0, xmm1
			movhlps	xmm1, xmm2
			addps	xmm0, xmm1
			addps	xmm0, xmm6
			movups	XMMWORD_PTR [edi], xmm0
			add	edi, (SIZEOF E3D_VECTOR_2D) * 2
			;
			sub	edx, 2
		.ENDW
		add	edx, 2
		.IF	!ZERO?
			movlps	xmm0, QWORD PTR [esi]
			unpcklps	xmm0, xmm0
			mulps	xmm0, xmm7
			movhlps	xmm1, xmm0
			addps	xmm0, xmm1
			addps	xmm0, xmm6
			movlps	QWORD PTR [edi], xmm0
		.ENDIF
		xor	eax, eax
		ret

	.ELSE
		test	edx, edx
		.WHILE	!ZERO?
			fld	[esi].x
			fld	[esi].y
			add	esi, (SIZEOF E3D_VECTOR_2D)
			;
			fld	[ebx].xAxis.x
			fmul	st, st(2)
			fld	[ebx].yAxis.x
			fmul	st, st(2)
			faddp	st(1), st
			fadd	[ecx].x
			fstp	[edi].x
			;
			fld	[ebx].xAxis.y
			fmulp	st(2), st
			fmul	[ebx].yAxis.y
			faddp	st(1), st
			fadd	[ecx].y
			fstp	[edi].y
			add	edi, (SIZEOF E3D_VECTOR_2D)
			;
			dec	edx
		.ENDW

	.ENDIF

	ASSUME	esi:NOTHING
	ASSUME	edi:NOTHING
	ASSUME	ebx:NOTHING
	ASSUME	ecx:NOTHING

	xor	eax, eax
	ret

eglMapping2DVectors		ENDP


;
;	上下反転する
; ----------------------------------------------------------------------------
ALIGN	10H
eglReverseVertically		PROC	NEAR32 C USES ebx esi edi,
		pImageInf:PEGL_IMAGE_INFO

	mov	ebx, pImageInf
	ASSUME	ebx:PEGL_IMAGE_INFO

	mov	ecx, [ebx].dwImageHeight
	mov	edx, [ebx].dwBytesPerLine
	mov	esi, [ebx].ptrImageArray
	mov	edi, [ebx].dwSizeOfImage
	dec	ecx
	imul	ecx, edx
	neg	edx
	neg	edi
	add	esi, ecx
	mov	[ebx].dwBytesPerLine, edx
	mov	[ebx].dwSizeOfImage, edi
	mov	[ebx].ptrImageArray, esi

	.IF	[ebx].dwInfoSize == (SIZEOF EGL_IMAGE_BUFF)
		mov	ecx, [ebx].dwImageHeight
		mov	edi, (EGL_IMAGE_BUFF PTR [ebx]).pLineAddrEntry
		mov	eax, [ebx].ptrImageArray
		mov	edx, [ebx].dwBytesPerLine
		test	ecx, ecx
		.WHILE	!ZERO?
			mov	DWORD PTR [edi], eax
			add	edi, SIZEOF DWORD
			add	eax, edx
			dec	ecx
		.ENDW
	.ENDIF

	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglReverseVertically		ENDP

;
;	ピクセルを取得
; ----------------------------------------------------------------------------
ALIGN	10H
eglGetPixel			PROC	NEAR32 C USES ebx esi,
		pImageInf:PCEGL_IMAGE_INFO, nPosX:SDWORD, nPosY:SDWORD

	mov	ebx, pImageInf
	ASSUME	ebx:PCEGL_IMAGE_INFO
	mov	eax, [ebx].dwBitsPerPixel
	mov	ecx, nPosX
	mov	esi, nPosY
	xor	edx, edx
	cmp	ecx, [ebx].dwImageWidth
	adc	edx, 0
	cmp	esi, [ebx].dwImageHeight
	adc	edx, 0
	cmp	eax, 33
	adc	edx, 0
	imul	esi, [ebx].dwBytesPerLine
	cmp	edx, 3
	sbb	edx, edx
	not	edx
	and	eax, edx
	add	esi, [ebx].ptrImageArray
	jmp	eglGetPixelTable[eax * 4]
	ASSUME	ebx:NOTHING

ALIGN	10H
eglGetPixel_LBD32::
	mov	eax, DWORD PTR [esi + ecx * 4]
	ret

ALIGN	10H
eglGetPixel_LBD24::
	lea	ecx, [ecx + ecx * 2]
	movzx	eax, WORD PTR [esi + ecx + 1]
	movzx	edx, BYTE PTR [esi + ecx]
	shl	eax, 8
	or	eax, edx
	ret

ALIGN	10H
eglGetPixel_LBD8::
	movzx	eax, BYTE PTR [esi + ecx]
	ret

eglGetPixel_LBD16::
	movzx	eax, WORD PTR [esi + ecx * 2]
	ret

eglGetPixel_LBD1::
	mov	edx, ecx
	and	ecx, 07H
	shr	edx, 3
	xor	ecx, 07H
	movzx	eax, BYTE PTR [esi + edx]
	shr	eax, cl
	and	eax, 01H
	ret

eglGetPixel_LBD4::
	mov	edx, ecx
	and	ecx, 01H
	shr	edx, 1
	xor	ecx, 01H
	movzx	eax, BYTE PTR [esi + edx]
	shl	ecx, 2
	shr	eax, cl
	and	eax, 0FH
	ret

eglGetPixel_LBD0::
	TRACE	<"eglGetPixel : 未対応のフォーマット、又は画面外です。", 0AH>
	xor	eax, eax
	ret

eglGetPixel			ENDP

;
;	ピクセルを設定
; ----------------------------------------------------------------------------
ALIGN	10H
eglSetPixel			PROC	NEAR32 C USES ebx esi,
		pImageInf:PEGL_IMAGE_INFO,
		nPosX:SDWORD, nPosY:SDWORD, colorPixel:EGL_PALETTE

	mov	ebx, pImageInf
	ASSUME	ebx:PCEGL_IMAGE_INFO
	mov	ecx, nPosX
	mov	esi, nPosY
	mov	edx, [ebx].dwBitsPerPixel
	xor	eax, eax
	cmp	ecx, [ebx].dwImageWidth
	adc	eax, 0
	cmp	esi, [ebx].dwImageHeight
	adc	eax, 0
	cmp	edx, 33
	adc	eax, 0
	imul	esi, [ebx].dwBytesPerLine
	cmp	eax, 3
	sbb	eax, eax
	not	eax
	add	esi, [ebx].ptrImageArray
	and	edx, eax
	mov	eax, colorPixel.dwPixelCode
	jmp	eglSetPixelTable[edx * 4]
	ASSUME	ebx:NOTHING

ALIGN	10H
eglSetPixel_LBD32::
	mov	DWORD PTR [esi + ecx * 4], eax
	xor	eax, eax
	ret

ALIGN	10H
eglSetPixel_LBD24::
	lea	ecx, [ecx + ecx * 2]
	mov	WORD PTR [esi + ecx], ax
	shr	eax, 16
	mov	BYTE PTR [esi + ecx + 2], al
	xor	eax, eax
	ret

ALIGN	10H
eglSetPixel_LBD8::
	mov	BYTE PTR [esi + ecx], al
	xor	eax, eax
	ret

eglSetPixel_LBD16::
	mov	WORD PTR [esi + ecx * 2], ax
	xor	eax, eax
	ret

eglSetPixel_LBD1::
	mov	ebx, ecx
	and	ecx, 07H
	and	eax, 01H
	shr	ebx, 3
	xor	ecx, 07H
	mov	dl, 0FEH
	shl	al, cl
	rol	dl, cl
	and	dl, BYTE PTR [esi + ebx]
	or	dl, al
	xor	eax, eax
	mov	BYTE PTR [esi + ebx], dl
	ret

eglSetPixel_LBD4::
	mov	ebx, ecx
	and	ecx, 01H
	and	eax, 0FH
	xor	ecx, 01H
	shr	ebx, 1
	shl	ecx, 2
	mov	dl, 0F0H
	shl	al, cl
	rol	dl, cl
	and	dl, BYTE PTR [esi + ebx]
	or	dl, al
	xor	eax, eax
	mov	BYTE PTR [esi + ebx], dl
	ret

eglSetPixel_LBD0::
	TRACE	<"eglSetPixel : 未対応のフォーマット、又は画面外です。", 0AH>
	mov	eax, eslErrGeneral
	ret

eglSetPixel			ENDP


;
;	基本算術関数セットアップ
; ----------------------------------------------------------------------------
eglInitializeMathFunctions	PROC	NEAR32 C

	.IF	ERI_EnabledProcessorType & ERI_USE_XMM_P3

	mov	eglNegateVector,		OFFSET eglNegateVector_486
	mov	eglAddVector,			OFFSET eglAddVector_SSE
	mov	eglSubVector,			OFFSET eglSubVector_SSE
	mov	eglMultipleVector,		OFFSET eglMultipleVector_SSE
	mov	eglDivideVector,		OFFSET eglDivideVector_SSE
	mov	eglAbsoluteVector,		OFFSET eglAbsoluteVector_SSE
	mov	eglVectorExteriorProduct,	OFFSET eglVectorExteriorProduct_SSE
	mov	eglVectorInnerProduct,		OFFSET eglVectorInnerProduct_SSE
	mov	eglVectorRoundTo1,		OFFSET eglVectorRoundTo1_SSE
	mov	eglMatrixNegate,		OFFSET eglMatrixNegate_486
	mov	eglMatrixAdd,			OFFSET eglMatrixAdd_SSE
	mov	eglMatrixSub,			OFFSET eglMatrixSub_SSE
	mov	eglMatrixMultiple,		OFFSET eglMatrixMultiple_SSE
	mov	eglMatrixDeterminant,		OFFSET eglMatrixDeterminant_SSE
	mov	eglMatrixInverse,		OFFSET eglMatrixInverse_SSE
	mov	eglMatrixRevolveOnX,		OFFSET eglMatrixRevolveOnX_SSE
	mov	eglMatrixRevolveOnY,		OFFSET eglMatrixRevolveOnY_SSE
	mov	eglMatrixRevolveOnZ,		OFFSET eglMatrixRevolveOnZ_SSE
	mov	eglMatrixRevolveByAngleOn,	OFFSET eglMatrixRevolveByAngleOn_SSE
	mov	eglMatrixRevolveForAngle,	OFFSET eglMatrixRevolveForAngle_SSE
	mov	eglMatrixMagnifyByVector,	OFFSET eglMatrixMagnifyByVector_SSE
	mov	eglMatrixRevolve,		OFFSET eglMatrixRevolve_SSE
	mov	eglMatrixRevolveBy,		OFFSET eglMatrixRevolveBy_SSE
	mov	eglMatrixRevolveVector,		OFFSET eglMatrixRevolveVector_SSE
	mov	eglMatrixRevolveVectors,	OFFSET eglMatrixRevolveVectors_SSE
	mov	eglGetMinVector,		OFFSET eglGetMinVector_SSE
	mov	eglGetMaxVector,		OFFSET eglGetMaxVector_SSE

	.ELSE

	mov	eglNegateVector,		OFFSET eglNegateVector_486
	mov	eglAddVector,			OFFSET eglAddVector_486
	mov	eglSubVector,			OFFSET eglSubVector_486
	mov	eglMultipleVector,		OFFSET eglMultipleVector_486
	mov	eglDivideVector,		OFFSET eglDivideVector_486
	mov	eglAbsoluteVector,		OFFSET eglAbsoluteVector_486
	mov	eglVectorExteriorProduct,	OFFSET eglVectorExteriorProduct_486
	mov	eglVectorInnerProduct,		OFFSET eglVectorInnerProduct_486
	mov	eglVectorRoundTo1,		OFFSET eglVectorRoundTo1_486
	mov	eglMatrixNegate,		OFFSET eglMatrixNegate_486
	mov	eglMatrixAdd,			OFFSET eglMatrixAdd_486
	mov	eglMatrixSub,			OFFSET eglMatrixSub_486
	mov	eglMatrixMultiple,		OFFSET eglMatrixMultiple_486
	mov	eglMatrixDeterminant,		OFFSET eglMatrixDeterminant_486
	mov	eglMatrixInverse,		OFFSET eglMatrixInverse_486
	mov	eglMatrixRevolveOnX,		OFFSET eglMatrixRevolveOnX_486
	mov	eglMatrixRevolveOnY,		OFFSET eglMatrixRevolveOnY_486
	mov	eglMatrixRevolveOnZ,		OFFSET eglMatrixRevolveOnZ_486
	mov	eglMatrixRevolveByAngleOn,	OFFSET eglMatrixRevolveByAngleOn_486
	mov	eglMatrixRevolveForAngle,	OFFSET eglMatrixRevolveForAngle_486
	mov	eglMatrixMagnifyByVector,	OFFSET eglMatrixMagnifyByVector_486
	mov	eglMatrixRevolve,		OFFSET eglMatrixRevolve_486
	mov	eglMatrixRevolveBy,		OFFSET eglMatrixRevolveBy_486
	mov	eglMatrixRevolveVector,		OFFSET eglMatrixRevolveVector_486
	mov	eglMatrixRevolveVectors,	OFFSET eglMatrixRevolveVectors_486
	mov	eglGetMinVector,		OFFSET eglGetMinVector_486
	mov	eglGetMaxVector,		OFFSET eglGetMaxVector_486

	.ENDIF

	ret

eglInitializeMathFunctions	ENDP


;	ベクトル反転
; ----------------------------------------------------------------------------
eglNegateVector_486		PROC	NEAR32 C, pv:PE3D_VECTOR
	mov	eax, pv
	ASSUME	eax:PE3D_VECTOR
	xor	[eax].x, 80000000H
	xor	[eax].y, 80000000H
	xor	[eax].z, 80000000H
	ASSUME	eax:NOTHING
	ret
eglNegateVector_486		ENDP


;	ベクトル加算
; ----------------------------------------------------------------------------
eglAddVector_486		PROC	NEAR32 C,
			pv1:PE3D_VECTOR, pv2:PCE3D_VECTOR
	mov	eax, pv1
	mov	edx, pv2
	ASSUME	eax:PE3D_VECTOR
	ASSUME	edx:PCE3D_VECTOR
	fld	[eax].x
	fadd	[edx].x
	fstp	[eax].x
	fld	[eax].y
	fadd	[edx].y
	fstp	[eax].y
	fld	[eax].z
	fadd	[edx].z
	fstp	[eax].z
	ASSUME	eax:NOTHING
	ASSUME	edx:NOTHING
	ret
eglAddVector_486		ENDP

;	ベクトル減算
; ----------------------------------------------------------------------------
eglSubVector_486		PROC	NEAR32 C,
			pv1:PE3D_VECTOR, pv2:PCE3D_VECTOR
	mov	eax, pv1
	mov	edx, pv2
	ASSUME	eax:PE3D_VECTOR
	ASSUME	edx:PCE3D_VECTOR
	fld	[eax].x
	fsub	[edx].x
	fstp	[eax].x
	fld	[eax].y
	fsub	[edx].y
	fstp	[eax].y
	fld	[eax].z
	fsub	[edx].z
	fstp	[eax].z
	ASSUME	eax:NOTHING
	ASSUME	edx:NOTHING
	ret
eglSubVector_486		ENDP

;	ベクトル×スカラー
; ----------------------------------------------------------------------------
eglMultipleVector_486		PROC	NEAR32 C, pv:PE3D_VECTOR, r:REAL4
	mov	eax, pv
	ASSUME	eax:PE3D_VECTOR
	fld	[eax].x
	fld	[eax].y
	fld	[eax].z
	fld	r
	fmul	st(1), st
	fmul	st(2), st
	fmulp	st(3), st
	fstp	[eax].z
	fstp	[eax].y
	fstp	[eax].x
	ASSUME	eax:NOTHING
	ret
eglMultipleVector_486		ENDP


;	ベクトル／スカラー
; ----------------------------------------------------------------------------
eglDivideVector_486		PROC	NEAR32 C, pv:PE3D_VECTOR, r:REAL4
	mov	eax, pv
	ASSUME	eax:PE3D_VECTOR
	fld	[eax].x
	fld	[eax].y
	fld	[eax].z
	fld	r
	fdiv	st(1), st
	fdiv	st(2), st
	fdivp	st(3), st
	fstp	[eax].z
	fstp	[eax].y
	fstp	[eax].x
	ASSUME	eax:NOTHING
	ret
eglDivideVector_486		ENDP


;	ベクトル絶対値
; ----------------------------------------------------------------------------
eglAbsoluteVector_486		PROC	NEAR32 C,
			pabs:PTR REAL4, pv:PCE3D_VECTOR
	mov	eax, pv
	ASSUME	eax:PE3D_VECTOR
	fld	[eax].x
	fmul	st, st(0)
	fld	[eax].y
	fmul	st, st(0)
	fld	[eax].z
	fmul	st, st(0)
	mov	eax, pabs
	ASSUME	eax:PTR REAL4
	faddp	st(2), st
	faddp	st(1), st
	fsqrt
	fstp	[eax]
	ASSUME	eax:NOTHING
	ret
eglAbsoluteVector_486		ENDP


;	ベクトル外積
; ----------------------------------------------------------------------------
eglVectorExteriorProduct_486	PROC	NEAR32 C,
			pv1:PE3D_VECTOR, pv2:PCE3D_VECTOR
	mov	eax, pv1
	mov	edx, pv2
	ASSUME	eax:PE3D_VECTOR
	ASSUME	edx:PCE3D_VECTOR
	fld	[eax].z
	fld	[eax].y
	fld	[eax].x
	;
	fld	[edx].z
	fmul	st, st(2) ; y
	fld	[edx].y
	fmul	st, st(4) ; z
	fsubp	st(1), st
	fstp	[eax].x
	;
	fld	[edx].x
	fmul	st, st(3) ; z
	fld	[edx].z
	fmul	st, st(2) ; x
	fsubp	st(1), st
	fstp	[eax].y
	;
	fld	[edx].y
	fmulp	st(1), st
	fld	[edx].x
	fmulp	st(2), st
	fsubrp	st(1), st
	fstp	[eax].z
	fstp	st(0)
	ASSUME	eax:NOTHING
	ASSUME	edx:NOTHING
	ret
eglVectorExteriorProduct_486	ENDP


;	ベクトル内積
; ----------------------------------------------------------------------------
eglVectorInnerProduct_486	PROC	NEAR32 C,
			prs:PTR REAL4, pv1:PCE3D_VECTOR, pv2:PCE3D_VECTOR
	mov	eax, pv1
	mov	edx, pv2
	ASSUME	eax:PCE3D_VECTOR
	ASSUME	edx:PCE3D_VECTOR
	fld	[eax].x
	fmul	[edx].x
	fld	[eax].y
	fmul	[edx].y
	fld	[eax].z
	fmul	[edx].z
	mov	eax, prs
	ASSUME	eax:PTR REAL4
	faddp	st(1), st
	faddp	st(1), st
	fstp	[eax]
	ASSUME	eax:NOTHING
	ASSUME	edx:NOTHING
	ret
eglVectorInnerProduct_486	ENDP


;	ベクトル正規化
; ----------------------------------------------------------------------------
eglVectorRoundTo1_486		PROC	NEAR32 C, pv:PE3D_VECTOR
	LOCAL	r:REAL4
	mov	eax, pv
	ASSUME	eax:PE3D_VECTOR
	fld	[eax].z
	fld	[eax].y
	fld	[eax].x
	fld	st(2)
	fmul	st, st(0)
	fld	st(2)
	fmul	st, st(0)
	fld	st(2)
	fmul	st, st(0)
	faddp	st(1), st
	faddp	st(1), st
	fst	r
	.IF	(DWORD PTR r) > 00800000H
		fsqrt
		fld1
		fdivrp	st(1), st
		fmul	st(1), st
		fmul	st(2), st
		fmulp	st(3), st
		fstp	[eax].x
		fstp	[eax].y
		fstp	[eax].z
	.ELSE
		fstp	st(0)
		fstp	st(0)
		fstp	st(0)
		fstp	st(0)
	.ENDIF
	;
	ASSUME	eax:NOTHING
	ret
eglVectorRoundTo1_486		ENDP


;	行列符号反転
; ----------------------------------------------------------------------------
eglMatrixNegate_486		PROC	NEAR32 C, matrix:PE3D_REV_MATRIX
	mov	eax, matrix
	mov	ecx, 4 * 3
	.REPEAT
		xor	DWORD PTR [eax], 80000000H
		add	eax, 4
		dec	ecx
	.UNTIL	ZERO?
	ret
eglMatrixNegate_486		ENDP


;	行列加算
; ----------------------------------------------------------------------------
eglMatrixAdd_486		PROC	NEAR32 C,
			matDst:PE3D_REV_MATRIX, matSrc:PCE3D_REV_MATRIX
	mov	eax, matDst
	mov	edx, matSrc
	ASSUME	eax:PTR REAL4
	ASSUME	edx:PTR REAL4
	mov	ecx, 3
	.REPEAT
		fld	[eax]
		fadd	[edx]
		fstp	[eax]
		fld	[eax + 4]
		fadd	[edx + 4]
		fstp	[eax + 4]
		fld	[eax + 8]
		fadd	[edx + 8]
		fstp	[eax + 8]
		add	eax, 16
		add	edx, 16
		dec	ecx
	.UNTIL	ZERO?
	ASSUME	eax:NOTHING
	ASSUME	edx:NOTHING
	ret
eglMatrixAdd_486		ENDP


;	行列減算
; ----------------------------------------------------------------------------
eglMatrixSub_486		PROC	NEAR32 C,
			matDst:PE3D_REV_MATRIX, matSrc:PCE3D_REV_MATRIX
	mov	eax, matDst
	mov	edx, matSrc
	ASSUME	eax:PTR REAL4
	ASSUME	edx:PTR REAL4
	mov	ecx, 3
	.REPEAT
		fld	[eax]
		fsub	[edx]
		fstp	[eax]
		fld	[eax + 4]
		fsub	[edx + 4]
		fstp	[eax + 4]
		fld	[eax + 8]
		fsub	[edx + 8]
		fstp	[eax + 8]
		add	eax, 16
		add	edx, 16
		dec	ecx
	.UNTIL	ZERO?
	ASSUME	eax:NOTHING
	ASSUME	edx:NOTHING
	ret
eglMatrixSub_486		ENDP


;	行列×スカラー
; ----------------------------------------------------------------------------
eglMatrixMultiple_486		PROC	NEAR32 C,
			matDst:PE3D_REV_MATRIX, r:REAL4
	mov	eax, matDst
	ASSUME	eax:PTR REAL4
	mov	ecx, 3
	.REPEAT
		fld	[eax]
		fmul	r
		fstp	[eax]
		fld	[eax + 4]
		fmul	r
		fstp	[eax + 4]
		fld	[eax + 8]
		fmul	r
		fstp	[eax + 8]
		add	eax, 16
		dec	ecx
	.UNTIL	ZERO?
	ASSUME	eax:NOTHING
	ret
eglMatrixMultiple_486		ENDP


;	行列式
; ----------------------------------------------------------------------------
eglMatrixDeterminant_486	PROC	NEAR32 C,
			pDet:PTR REAL32, matrix:PCE3D_REV_MATRIX
	mov	eax, matrix
	mov	edx, pDet
	ASSUME	eax:PTR REAL4
	ASSUME	edx:PTR REAL4
	;
	fld	[eax + 14H]
	fmul	[eax + 28H]
	fld	[eax + 24H]
	fmul	[eax + 18H]
	fsubp	st(1), st
	fmul	[eax]
	;
	fld	[eax + 18H]
	fmul	[eax + 20H]
	fld	[eax + 28H]
	fmul	[eax + 10H]
	fsubp	st(1), st
	fmul	[eax + 04H]
	;
	fld	[eax + 10H]
	fmul	[eax + 24H]
	fld	[eax + 20H]
	fmul	[eax + 14H]
	fsubp	st(1), st
	fmul	[eax + 08H]
	;
	faddp	st(1), st
	faddp	st(1), st
	fstp	[edx]
	;
	ASSUME	eax:NOTHING
	ASSUME	edx:NOTHING
	ret
eglMatrixDeterminant_486	ENDP


;	逆行列
; ----------------------------------------------------------------------------
eglMatrixInverse_486		PROC NEAR32 C,
			matDst:PE3D_REV_MATRIX, matSrc:PCE3D_REV_MATRIX
	mov	eax, matDst
	mov	ecx, matSrc
	ASSUME	eax:PE3D_REV_MATRIX
	ASSUME	ecx:PCE3D_REV_MATRIX
	;         | a1 a2 a3 |
	; D = 1 / | b1 b2 b3 |
	;         | c1 c2 c3 |
	fld1
	FOR	@BASE, <0, 1, 2>
		@LINE = 0
		FOR	@INDEX, <0, 1, 2>
			@INDEX_SUB = (@BASE + @INDEX) MOD 3
			fld	[ecx].matrix[@LINE][@INDEX_SUB*4]
			@LINE = @LINE + 10H
		ENDM
		fxch	st(2)
		fmulp	st(1), st
		fmulp	st(1), st
		;
		@LINE = 0
		FOR	@INDEX, <0, 2, 1>
			@INDEX_SUB = (@BASE + @INDEX) MOD 3
			fld	[ecx].matrix[@LINE][@INDEX_SUB*4]
			@LINE = @LINE + 10H
		ENDM
		fxch	st(2)
		fmulp	st(1), st
		fmulp	st(1), st
		fsubp	st(1), st
	ENDM
	faddp	st(1), st
	faddp	st(1), st
	fdivp	st(1), st
	;
	; A' = [B, C],  B' = [C, A],  C' = [A, B]
	;
	xor	edx, edx
	FOR	@BASE, <0, 1, 2>
		@LINE1 = ((@BASE + 1) MOD 3) * 10H
		@LINE2 = ((@BASE + 2) MOD 3) * 10H
		FOR	@INDEX, <0, 1, 2>
			@INDEX_SUB1 = ((@INDEX + 1) MOD 3) * 4
			@INDEX_SUB2 = ((@INDEX + 2) MOD 3) * 4
			fld	[ecx].matrix[@LINE1][@INDEX_SUB1]
			fmul	[ecx].matrix[@LINE2][@INDEX_SUB2]
			fld	[ecx].matrix[@LINE1][@INDEX_SUB2]
			fmul	[ecx].matrix[@LINE2][@INDEX_SUB1]
			fsubp	st(1), st
			fmul	st, st(1)
			fstp	[eax].matrix[@INDEX*10H][@BASE*4]
		ENDM
	ENDM
	fstp	st(0)
	;
	ASSUME	eax:NOTHING
	ASSUME	ecx:NOTHING
	ret
eglMatrixInverse_486		ENDP


;	行列回転
; ----------------------------------------------------------------------------
eglMatrixRevolveOnX_486		PROC	NEAR32 C,
			matrix:PE3D_REV_MATRIX, rSin:REAL4, rCos:REAL4
	; ( a11 a12 a13 )   ( 1    0       0    )
	; ( a21 a22 a23 ) X ( 0  cos(x)  sin(x) )
	; ( a31 a32 a33 )   ( 0 -sin(x)  cos(x) )
	mov	eax, matrix
	ASSUME	eax:PTR REAL4
	FOR	@INDEX, <0, 16, 32>
		fld	[eax + @INDEX + 8]
		fld	[eax + @INDEX + 4]
		fld	st(0)
		fmul	rCos
		fld	st(2)
		fmul	rSin
		fsubp	st(1), st
		fstp	[eax + @INDEX + 4]
		fmul	rSin
		fxch	st(1)
		fmul	rCos
		faddp	st(1), st
		fstp	[eax + @INDEX + 8]
	ENDM
	ASSUME	eax:NOTHING
	ret
eglMatrixRevolveOnX_486		ENDP

eglMatrixRevolveOnY_486		PROC	NEAR32 C,
			matrix:PE3D_REV_MATRIX, rSin:REAL4, rCos:REAL4
	; ( a11 a12 a13 )   (  cos(y)  0  sin(y) )
	; ( a21 a22 a23 ) X (    0     1    0    )
	; ( a31 a32 a33 )   ( -sin(y)  0  cos(y) )
	mov	eax, matrix
	ASSUME	eax:PTR REAL4
	FOR	@INDEX, <0, 16, 32>
		fld	[eax + @INDEX + 8]
		fld	[eax + @INDEX + 0]
		fld	st(0)
		fmul	rCos
		fld	st(2)
		fmul	rSin
		fsubp	st(1), st
		fstp	[eax + @INDEX + 0]
		fmul	rSin
		fxch	st(1)
		fmul	rCos
		faddp	st(1), st
		fstp	[eax + @INDEX + 8]
	ENDM
	ASSUME	eax:NOTHING
	ret
eglMatrixRevolveOnY_486		ENDP

eglMatrixRevolveOnZ_486		PROC	NEAR32 C,
			matrix:PE3D_REV_MATRIX, rSin:REAL4, rCos:REAL4
	; ( a11 a12 a13 )   ( cos(z) -sin(z)  0 )
	; ( a21 a22 a23 ) X ( sin(z)  cos(z)  0 )
	; ( a31 a32 a33 )   (   0       0     1 )
	mov	eax, matrix
	ASSUME	eax:PTR REAL4
	FOR	@INDEX, <0, 16, 32>
		fld	[eax + @INDEX + 4]
		fld	[eax + @INDEX + 0]
		fld	st(0)
		fmul	rCos
		fld	st(2)
		fmul	rSin
		faddp	st(1), st
		fstp	[eax + @INDEX + 0]
		fmul	rSin
		fxch	st(1)
		fmul	rCos
		fsubrp	st(1), st
		fstp	[eax + @INDEX + 4]
	ENDM
	ASSUME	eax:NOTHING
	ret
eglMatrixRevolveOnZ_486		ENDP

eglMatrixRevolveByAngleOn_486	PROC	NEAR32 C,
			matrix:PE3D_REV_MATRIX, pAngle:PCE3D_VECTOR
	LOCAL	rSqrtXZ:REAL4, rSqrtXYZ:REAL4
	LOCAL	rSin:REAL4, rCos:REAL4
	mov	edx, pAngle
	ASSUME	edx:PCE3D_VECTOR
	fld	[edx].y
	fmul	st, st(0)
	fld	[edx].x
	fmul	st, st(0)
	fld	[edx].z
	fmul	st, st(0)
	faddp	st(1), st
	fadd	st(1), st
	fsqrt
	fstp	rSqrtXZ
	fsqrt
	fstp	rSqrtXYZ
	.IF	(DWORD PTR rSqrtXYZ) > 00800000H
		fld	[edx].y
		fchs
		fdiv	rSqrtXYZ
		fstp	rSin
		fld	rSqrtXZ
		fdiv	rSqrtXYZ
		fstp	rCos
		INVOKE	eglMatrixRevolveOnX_486, matrix, rSin, rCos
	.ENDIF
	mov	edx, pAngle
	.IF	(DWORD PTR rSqrtXZ) > 00800000H
		fld	[edx].x
		fchs
		fdiv	rSqrtXZ
		fstp	rSin
		fld	[edx].z
		fdiv	rSqrtXZ
		fstp	rCos
		INVOKE	eglMatrixRevolveOnY_486, matrix, rSin, rCos
	.ENDIF
	ASSUME	edx:NOTHING
	ret
eglMatrixRevolveByAngleOn_486	ENDP

eglMatrixRevolveForAngle_486	PROC	NEAR32 C,
			matrix:PE3D_REV_MATRIX, pAngle:PCE3D_VECTOR
	LOCAL	rSqrtXZ:REAL4, rSqrtXYZ:REAL4
	LOCAL	rSin:REAL4, rCos:REAL4
	mov	edx, pAngle
	ASSUME	edx:PCE3D_VECTOR
	fld	[edx].y
	fmul	st, st(0)
	fld	[edx].x
	fmul	st, st(0)
	fld	[edx].z
	fmul	st, st(0)
	faddp	st(1), st
	fadd	st(1), st
	fsqrt
	fstp	rSqrtXZ
	fsqrt
	fstp	rSqrtXYZ
	.IF	(DWORD PTR rSqrtXZ) > 00800000H
		fld	[edx].x
		fdiv	rSqrtXZ
		fstp	rSin
		fld	[edx].z
		fdiv	rSqrtXZ
		fstp	rCos
		INVOKE	eglMatrixRevolveOnY_486, matrix, rSin, rCos
	.ENDIF
	mov	edx, pAngle
	.IF	(DWORD PTR rSqrtXYZ) > 00800000H
		fld	[edx].y
		fdiv	rSqrtXYZ
		fstp	rSin
		fld	rSqrtXZ
		fdiv	rSqrtXYZ
		fstp	rCos
		INVOKE	eglMatrixRevolveOnX_486, matrix, rSin, rCos
	.ENDIF
	ASSUME	edx:NOTHING
	ret
eglMatrixRevolveForAngle_486	ENDP

eglMatrixMagnifyByVector_486	PROC	NEAR32 C,
			matrix:PE3D_REV_MATRIX, pv:PCE3D_VECTOR
	mov	eax, matrix
	mov	edx, pv
	ASSUME	eax:PTR REAL4
	ASSUME	edx:PCE3D_VECTOR
	fld	[eax + 32 + 0]
	fld	[eax + 16 + 0]
	fld	[eax]
	fld	[edx].x
	fmul	st(1), st
	fmul	st(2), st
	fmulp	st(3), st
	fstp	[eax]
	fstp	[eax + 16 + 0]
	fstp	[eax + 32 + 0]
	;
	fld	[eax + 32 + 4]
	fld	[eax + 16 + 4]
	fld	[eax +  0 + 4]
	fld	[edx].y
	fmul	st(1), st
	fmul	st(2), st
	fmulp	st(3), st
	fstp	[eax +  0 + 4]
	fstp	[eax + 16 + 4]
	fstp	[eax + 32 + 4]
	;
	fld	[eax + 32 + 8]
	fld	[eax + 16 + 8]
	fld	[eax +  0 + 8]
	fld	[edx].z
	fmul	st(1), st
	fmul	st(2), st
	fmulp	st(3), st
	fstp	[eax +  0 + 8]
	fstp	[eax + 16 + 8]
	fstp	[eax + 32 + 8]
	ASSUME	eax:NOTHING
	ASSUME	edx:NOTHING
	ret
eglMatrixMagnifyByVector_486	ENDP

eglMatrixRevolve_486		PROC	NEAR32 C USES esi edi,
			matrix:PCE3D_REV_MATRIX, matDst:PE3D_REV_MATRIX
	LOCAL	matTemp:E3D_REV_MATRIX
	pushfd
	cld
	mov	ecx, 4 * 3
	mov	esi, matDst
	lea	edi, matTemp
	rep	movsd
	;
	mov	ecx, 4 * 3
	mov	esi, matrix
	mov	edi, matDst
	rep	movsd
	popfd
	;
	INVOKE	eglMatrixRevolveBy_486, matDst, ADDR matTemp
	ret
eglMatrixRevolve_486		ENDP


eglMatrixRevolveBy_486		PROC	NEAR32 C,
			matDst:PE3D_REV_MATRIX, matSrc:PCE3D_REV_MATRIX
	mov	eax, matDst
	mov	edx, matSrc
	ASSUME	eax:PTR REAL4
	ASSUME	edx:PTR REAL4
	FOR	@INDEX, <0, 16, 32>
		fld	[eax + @INDEX + 8]
		fld	[eax + @INDEX + 4]
		fld	[eax + @INDEX + 0]
		fld	[edx + 32 + 0]
		fmul	st, st(3)
		fld	[edx + 16 + 0]
		fmul	st, st(3)
		fld	[edx]
		fmul	st, st(3)
		faddp	st(1), st
		faddp	st(1), st
		fstp	[eax + @INDEX + 0]
		;
		fld	[edx + 32 + 4]
		fmul	st, st(3)
		fld	[edx + 16 + 4]
		fmul	st, st(3)
		fld	[edx +  0 + 4]
		fmul	st, st(3)
		faddp	st(1), st
		faddp	st(1), st
		fstp	[eax + @INDEX + 4]
		;
		fmul	[edx +  0 + 8]
		fld	[edx + 16 + 8]
		fmulp	st(2), st
		fld	[edx + 32 + 8]
		fmulp	st(3), st
		faddp	st(1), st
		faddp	st(1), st
		fstp	[eax + @INDEX + 8]
	ENDM
	ASSUME	eax:NOTHING
	ASSUME	edx:NOTHING
	ret
eglMatrixRevolveBy_486		ENDP

eglMatrixRevolveVector_486	PROC	NEAR32 C,
			matrix:PCE3D_REV_MATRIX, pv:PE3D_VECTOR
	mov	eax, matrix
	mov	edx, pv
	ASSUME	eax:PTR REAL4
	ASSUME	edx:PE3D_VECTOR
	fld	[edx].z
	fld	[edx].y
	fld	[edx].x
	;
	fld	[eax + 8]
	fmul	st, st(3)
	fld	[eax + 4]
	fmul	st, st(3)
	fld	[eax]
	fmul	st, st(3)
	faddp	st(1), st
	faddp	st(1), st
	fstp	[edx].x
	;
	fld	[eax + 16 + 8]
	fmul	st, st(3)
	fld	[eax + 16 + 4]
	fmul	st, st(3)
	fld	[eax + 16]
	fmul	st, st(3)
	faddp	st(1), st
	faddp	st(1), st
	fstp	[edx].y
	;
	fmul	[eax + 32]
	fld	[eax + 32 + 4]
	fmulp	st(2), st
	fld	[eax + 32 + 8]
	fmulp	st(3), st
	faddp	st(1), st
	faddp	st(1), st
	fstp	[edx].z
	;
	ASSUME	eax:NOTHING
	ASSUME	edx:NOTHING
	ret
eglMatrixRevolveVector_486	ENDP


eglMatrixRevolveVectors_486	PROC	NEAR32 C USES ebx esi edi,
		matrix:PCE3D_REV_MATRIX, pvDst:PE3D_VECTOR4,
		pvSrc:PCE3D_VECTOR4, pvOrigin:PE3D_VECTOR, nCount:DWORD
	LOCAL	vOrigin:E3D_VECTOR

	mov	ebx, pvOrigin
	.IF	ebx != NULL
		ASSUME	ebx:PE3D_VECTOR
		mov	eax, [ebx].x
		mov	vOrigin.x, eax
		mov	eax, [ebx].y
		mov	vOrigin.y, eax
		mov	eax, [ebx].z
		mov	vOrigin.z, eax
		ASSUME	ebx:NOTHING
	.ELSE
		xor	eax, eax
		mov	vOrigin.x, eax
		mov	vOrigin.y, eax
		mov	vOrigin.z, eax
	.ENDIF
	mov	eax, matrix
	mov	esi, pvSrc
	mov	edi, pvDst
	mov	ecx, nCount
	ASSUME	eax:PTR REAL4
	ASSUME	esi:PE3D_VECTOR4
	ASSUME	edi:PE3D_VECTOR4
	test	ecx, ecx
	.WHILE	!ZERO?
		fld	[esi].z
		fld	[esi].y
		fld	[esi].x
		add	esi, (SIZEOF E3D_VECTOR4)
		;
		fld	[eax + 8]
		fmul	st, st(3)
		fld	[eax + 4]
		fmul	st, st(3)
		fld	[eax]
		fmul	st, st(3)
		faddp	st(1), st
		faddp	st(1), st
		fadd	vOrigin.x
		fstp	[edi].x
		;
		fld	[eax + 16 + 8]
		fmul	st, st(3)
		fld	[eax + 16 + 4]
		fmul	st, st(3)
		fld	[eax + 16]
		fmul	st, st(3)
		faddp	st(1), st
		faddp	st(1), st
		fadd	vOrigin.y
		fstp	[edi].y
		;
		fmul	[eax + 32]
		fld	[eax + 32 + 4]
		fmulp	st(2), st
		fld	[eax + 32 + 8]
		fmulp	st(3), st
		faddp	st(1), st
		faddp	st(1), st
		fadd	vOrigin.z
		fstp	[edi].z
		mov	[edi].d, 0
		;
		add	edi, (SIZEOF E3D_VECTOR4)
		dec	ecx
	.ENDW
	;
	ASSUME	eax:NOTHING
	ASSUME	esi:NOTHING
	ASSUME	edi:NOTHING
	ret
eglMatrixRevolveVectors_486	ENDP


eglGetMinVector_486		PROC	NEAR32 C USES ebx esi edi,
			pvMin:PE3D_VECTOR4, pvList:PCE3D_VECTOR, nCount:DWORD

	mov	ecx, nCount
	mov	esi, pvList
	test	ecx, ecx
	.IF	!ZERO?
		push	ebp
		ASSUME	esi:PCE3D_VECTOR4
		mov	eax, [esi].x
		mov	ebx, [esi].y
		mov	edx, [esi].z
		mov	edi, eax
		mov	ebp, ebx
		sar	edi, 31
		sar	ebp, 31
		shr	edi, 1
		shr	ebp, 1
		xor	eax, edi
		xor	ebx, ebp
		mov	edi, edx
		sar	edi, 31
		shr	edi, 1
		xor	edx, edi
		;
		add	esi, (SIZEOF E3D_VECTOR4)
		dec	ecx
		.WHILE	!ZERO?
			mov	edi, [esi].x
			mov	ebp, edi
			sar	edi, 31
			shr	edi, 1
			xor	edi, ebp
			.IF	(SDWORD PTR eax) > (SDWORD PTR edi)
				mov	eax, edi
			.ENDIF
			;
			mov	edi, [esi].y
			mov	ebp, edi
			sar	edi, 31
			shr	edi, 1
			xor	edi, ebp
			.IF	(SDWORD PTR ebx) > (SDWORD PTR edi)
				mov	ebx, edi
			.ENDIF
			;
			mov	edi, [esi].z
			mov	ebp, edi
			sar	edi, 31
			shr	edi, 1
			xor	edi, ebp
			.IF	(SDWORD PTR edx) > (SDWORD PTR edi)
				mov	edx, edi
			.ENDIF
			;
			add	esi, (SIZEOF E3D_VECTOR4)
			dec	ecx
		.ENDW
		ASSUME	esi:NOTHING
		;
		mov	edi, eax
		mov	ebp, ebx
		sar	edi, 31
		sar	ebp, 31
		shr	edi, 1
		shr	ebp, 1
		xor	eax, edi
		xor	ebx, ebp
		mov	edi, edx
		sar	edi, 31
		shr	edi, 1
		xor	edx, edi
		;
		pop	ebp
		mov	esi, pvMin
		ASSUME	esi:PE3D_VECTOR4
		mov	[esi].x, eax
		mov	[esi].y, ebx
		mov	[esi].z, edx
		mov	[esi].d, 0
		ASSUME	esi:NOTHING
	.ENDIF
	ret

eglGetMinVector_486		ENDP


eglGetMaxVector_486		PROC NEAR32 C USES ebx esi edi,
			pvMax:PE3D_VECTOR4, pvList:PCE3D_VECTOR, nCount:DWORD

	mov	ecx, nCount
	mov	esi, pvList
	test	ecx, ecx
	.IF	!ZERO?
		push	ebp
		ASSUME	esi:PCE3D_VECTOR4
		mov	eax, [esi].x
		mov	ebx, [esi].y
		mov	edx, [esi].z
		mov	edi, eax
		mov	ebp, ebx
		sar	edi, 31
		sar	ebp, 31
		shr	edi, 1
		shr	ebp, 1
		xor	eax, edi
		xor	ebx, ebp
		mov	edi, edx
		sar	edi, 31
		shr	edi, 1
		xor	edx, edi
		;
		add	esi, (SIZEOF E3D_VECTOR4)
		dec	ecx
		.WHILE	!ZERO?
			mov	edi, [esi].x
			mov	ebp, edi
			sar	edi, 31
			shr	edi, 1
			xor	edi, ebp
			.IF	(SDWORD PTR eax) < (SDWORD PTR edi)
				mov	eax, edi
			.ENDIF
			;
			mov	edi, [esi].y
			mov	ebp, edi
			sar	edi, 31
			shr	edi, 1
			xor	edi, ebp
			.IF	(SDWORD PTR ebx) < (SDWORD PTR edi)
				mov	ebx, edi
			.ENDIF
			;
			mov	edi, [esi].z
			mov	ebp, edi
			sar	edi, 31
			shr	edi, 1
			xor	edi, ebp
			.IF	(SDWORD PTR edx) < (SDWORD PTR edi)
				mov	edx, edi
			.ENDIF
			;
			add	esi, (SIZEOF E3D_VECTOR4)
			dec	ecx
		.ENDW
		ASSUME	esi:NOTHING
		;
		mov	edi, eax
		mov	ebp, ebx
		sar	edi, 31
		sar	ebp, 31
		shr	edi, 1
		shr	ebp, 1
		xor	eax, edi
		xor	ebx, ebp
		mov	edi, edx
		sar	edi, 31
		shr	edi, 1
		xor	edx, edi
		;
		pop	ebp
		mov	esi, pvMax
		ASSUME	esi:PE3D_VECTOR4
		mov	[esi].x, eax
		mov	[esi].y, ebx
		mov	[esi].z, edx
		mov	[esi].d, 0
		ASSUME	esi:NOTHING
	.ENDIF
	ret

eglGetMaxVector_486		ENDP


;	ベクトル加算 SSE
; ----------------------------------------------------------------------------
eglAddVector_SSE		PROC	NEAR32 C,
			pv1:PE3D_VECTOR, pv2:PCE3D_VECTOR
	mov	eax, pv1
	mov	edx, pv2
	ASSUME	eax:PE3D_VECTOR
	ASSUME	edx:PCE3D_VECTOR
	movlps	xmm0, QWORD PTR [eax]
	movlps	xmm1, QWORD PTR [edx]
	movss	xmm2, [eax].z
	movss	xmm3, [edx].z
	addps	xmm0, xmm1
	addss	xmm2, xmm3
	movlps	QWORD PTR [eax], xmm0
	movss	[eax].z, xmm2
	ASSUME	eax:NOTHING
	ASSUME	edx:NOTHING
	ret
eglAddVector_SSE		ENDP

;	ベクトル減算 SSE
; ----------------------------------------------------------------------------
eglSubVector_SSE		PROC	NEAR32 C,
			pv1:PE3D_VECTOR, pv2:PCE3D_VECTOR
	mov	eax, pv1
	mov	edx, pv2
	ASSUME	eax:PE3D_VECTOR
	ASSUME	edx:PCE3D_VECTOR
	movlps	xmm0, QWORD PTR [eax]
	movlps	xmm1, QWORD PTR [edx]
	movss	xmm2, [eax].z
	movss	xmm3, [edx].z
	subps	xmm0, xmm1
	subss	xmm2, xmm3
	movlps	QWORD PTR [eax], xmm0
	movss	[eax].z, xmm2
	ASSUME	eax:NOTHING
	ASSUME	edx:NOTHING
	ret
eglSubVector_SSE		ENDP

;	ベクトル×スカラー SSE
; ----------------------------------------------------------------------------
eglMultipleVector_SSE		PROC	NEAR32 C, pv:PE3D_VECTOR, r:REAL4
	mov	eax, pv
	ASSUME	eax:PE3D_VECTOR
	movss	xmm4, r
	movlps	xmm0, QWORD PTR [eax]
	shufps	xmm4, xmm4, 0
	movss	xmm1, [eax].z
	mulps	xmm0, xmm4
	mulss	xmm1, xmm4
	movlps	QWORD PTR [eax], xmm0
	movss	[eax].z, xmm1
	ASSUME	eax:NOTHING
	ret
eglMultipleVector_SSE		ENDP


;	ベクトル／スカラー SSE
; ----------------------------------------------------------------------------
eglDivideVector_SSE		PROC	NEAR32 C, pv:PE3D_VECTOR, r:REAL4
	mov	eax, pv
	ASSUME	eax:PE3D_VECTOR
	movss	xmm4, r
	movlps	xmm0, QWORD PTR [eax]
	movss	xmm1, [eax].z
	shufps	xmm4, xmm4, 0
	movlhps	xmm0, xmm1
	divps	xmm0, xmm4
	movlps	QWORD PTR [eax], xmm0
	movhlps	xmm1, xmm0
	movss	[eax].z, xmm1
	ASSUME	eax:NOTHING
	ret
eglDivideVector_SSE		ENDP


;	ベクトル絶対値 SSE
; ----------------------------------------------------------------------------
eglAbsoluteVector_SSE		PROC	NEAR32 C,
			pabs:PTR REAL4, pv:PCE3D_VECTOR
	mov	eax, pv
	ASSUME	eax:PE3D_VECTOR
	movlps	xmm0, QWORD PTR [eax]
	movss	xmm1, [eax].z
	mulps	xmm0, xmm0
	mulss	xmm1, xmm1
	addss	xmm1, xmm0
	shufps	xmm0, xmm0, 1
	addss	xmm0, xmm1
	mov	eax, pabs
	ASSUME	eax:PTR REAL4
	sqrtss	xmm0, xmm0
	movss	[eax], xmm0
	ASSUME	eax:NOTHING
	ret
eglAbsoluteVector_SSE		ENDP


;	ベクトル外積 SSE
; ----------------------------------------------------------------------------
eglVectorExteriorProduct_SSE	PROC	NEAR32 C,
			pv1:PE3D_VECTOR, pv2:PCE3D_VECTOR
	mov	eax, pv1
	mov	edx, pv2
	ASSUME	eax:PE3D_VECTOR
	ASSUME	edx:PCE3D_VECTOR
	;
	movlps	xmm6, QWORD PTR [eax]
	movlps	xmm7, QWORD PTR [edx]
	movss	xmm0, [eax].z
	movss	xmm1, [edx].z
	movlhps	xmm6, xmm0
	movlhps	xmm7, xmm1
	;
	movaps	xmm0, xmm6
	movaps	xmm1, xmm7
	shufps	xmm6, xmm6, 11010010B
	shufps	xmm7, xmm7, 11001001B
	shufps	xmm0, xmm0, 11001001B
	shufps	xmm1, xmm1, 11010010B
	mulps	xmm6, xmm7
	mulps	xmm0, xmm1
	subps	xmm0, xmm6
	;
	movlps	QWORD PTR [eax], xmm0
	movhlps	xmm1, xmm0
	movss	[eax].z, xmm1
	;
	ASSUME	eax:NOTHING
	ASSUME	edx:NOTHING
	ret
eglVectorExteriorProduct_SSE	ENDP


;	ベクトル内積 SSE
; ----------------------------------------------------------------------------
eglVectorInnerProduct_SSE	PROC	NEAR32 C,
			prs:PTR REAL4, pv1:PCE3D_VECTOR, pv2:PCE3D_VECTOR
	mov	eax, pv1
	mov	edx, pv2
	ASSUME	eax:PCE3D_VECTOR
	ASSUME	edx:PCE3D_VECTOR
	movlps	xmm0, QWORD PTR [eax]
	movlps	xmm1, QWORD PTR [edx]
	movss	xmm2, [eax].z
	movss	xmm3, [edx].z
	mulps	xmm0, xmm1
	mulss	xmm2, xmm3
	mov	eax, prs
	ASSUME	eax:PTR REAL4
	addss	xmm2, xmm0
	shufps	xmm0, xmm0, 1
	addss	xmm0, xmm2
	movss	[eax], xmm0
	ASSUME	eax:NOTHING
	ASSUME	edx:NOTHING
	ret
eglVectorInnerProduct_SSE	ENDP


;	ベクトル正規化 SSE
; ----------------------------------------------------------------------------
eglVectorRoundTo1_SSE		PROC	NEAR32 C, pv:PE3D_VECTOR
	LOCAL	r:REAL4
	mov	eax, pv
	ASSUME	eax:PE3D_VECTOR
	movlps	xmm2, QWORD PTR [eax]
	movss	xmm3, [eax].z
	movaps	xmm0, xmm2
	mulps	xmm2, xmm2
	movlhps	xmm0, xmm3
	mulss	xmm3, xmm3
	;
	addss	xmm3, xmm2
	shufps	xmm2, xmm2, 1
	addss	xmm2, xmm3
	sqrtss	xmm2, xmm2
	shufps	xmm2, xmm2, 0
	;
	divps	xmm0, xmm2
	movlps	QWORD PTR [eax], xmm0
	movhlps	xmm1, xmm0
	movss	[eax].z, xmm1
	ASSUME	eax:NOTHING
	ret
eglVectorRoundTo1_SSE		ENDP


;	行列加算 SSE
; ----------------------------------------------------------------------------
eglMatrixAdd_SSE		PROC	NEAR32 C,
			matDst:PE3D_REV_MATRIX, matSrc:PCE3D_REV_MATRIX
	mov	eax, matDst
	mov	edx, matSrc
	movups	xmm0, [eax]
	movups	xmm4, [edx]
	addps	xmm0, xmm4
	movups	xmm1, [eax + 10H]
	movups	xmm5, [edx + 10H]
	addps	xmm1, xmm5
	movups	xmm2, [eax + 20H]
	movups	xmm6, [edx + 20H]
	addps	xmm2, xmm6
	movups	[eax], xmm0
	movups	[eax + 10H], xmm1
	movups	[eax + 20H], xmm2
	ret
eglMatrixAdd_SSE		ENDP


;	行列減算 SSE
; ----------------------------------------------------------------------------
eglMatrixSub_SSE		PROC	NEAR32 C,
			matDst:PE3D_REV_MATRIX, matSrc:PCE3D_REV_MATRIX
	mov	eax, matDst
	mov	edx, matSrc
	movups	xmm0, [eax]
	movups	xmm4, [edx]
	subps	xmm0, xmm4
	movups	xmm1, [eax + 10H]
	movups	xmm5, [edx + 10H]
	subps	xmm1, xmm5
	movups	xmm2, [eax + 20H]
	movups	xmm6, [edx + 20H]
	subps	xmm2, xmm6
	movups	[eax], xmm0
	movups	[eax + 10H], xmm1
	movups	[eax + 20H], xmm2
	ret
eglMatrixSub_SSE		ENDP


;	行列×スカラー SSE
; ----------------------------------------------------------------------------
eglMatrixMultiple_SSE		PROC	NEAR32 C,
			matDst:PE3D_REV_MATRIX, r:REAL4
	movss	xmm4, r
	mov	eax, matDst
	shufps	xmm4, xmm4, 0
	movups	xmm0, [eax]
	mulps	xmm0, xmm4
	movups	xmm1, [eax + 10H]
	mulps	xmm1, xmm4
	movups	xmm2, [eax + 20H]
	mulps	xmm2, xmm4
	movups	[eax], xmm0
	movups	[eax + 10H], xmm1
	movups	[eax + 20H], xmm2
	ret
eglMatrixMultiple_SSE		ENDP


;	行列式 SSE
; ----------------------------------------------------------------------------
eglMatrixDeterminant_SSE	PROC	NEAR32 C,
			pDet:PTR REAL32, matrix:PCE3D_REV_MATRIX
	mov	eax, matrix
	mov	edx, pDet
	ASSUME	edx:PTR REAL4
	movups	xmm6, [eax + 10H]
	movups	xmm7, [eax + 20H]
	movaps	xmm2, xmm6
	movaps	xmm3, xmm7
	shufps	xmm6, xmm6, 11010010B
	shufps	xmm7, xmm7, 11001001B
	shufps	xmm2, xmm2, 11001001B
	shufps	xmm3, xmm3, 11010010B
	mulps	xmm6, xmm7
	movups	xmm0, [eax]
	mulps	xmm2, xmm3
	subps	xmm2, xmm6
	mulps	xmm0, xmm2
	movhlps	xmm1, xmm0
	addss	xmm1, xmm0
	shufps	xmm0, xmm0, 1
	addss	xmm0, xmm1
	movss	[edx], xmm0
	ASSUME	edx:NOTHING
	ret
eglMatrixDeterminant_SSE	ENDP


;	逆行列 SSE
; ----------------------------------------------------------------------------
eglMatrixInverse_SSE	PROC	NEAR32 C,
			matDst:PE3D_REV_MATRIX, matSrc:PCE3D_REV_MATRIX
	mov	eax, matDst
	mov	ecx, matSrc
	ASSUME	eax:PE3D_REV_MATRIX
	ASSUME	ecx:PCE3D_REV_MATRIX
	;         | a1 a2 a3 |
	; D = 1 / | b1 b2 b3 |
	;         | c1 c2 c3 |
	movups	xmm6, XMMWORD_PTR [ecx + 10H]
	movups	xmm7, XMMWORD_PTR [ecx + 20H]
	movaps	xmm2, xmm6
	movaps	xmm3, xmm7
	shufps	xmm6, xmm6, 11010010B
	shufps	xmm7, xmm7, 11001001B
	shufps	xmm2, xmm2, 11001001B
	shufps	xmm3, xmm3, 11010010B
	mulps	xmm6, xmm7
	movups	xmm0, XMMWORD_PTR [ecx]
	mulps	xmm2, xmm3
	subps	xmm2, xmm6
	mulps	xmm0, xmm2
	movhlps	xmm1, xmm0
	addss	xmm1, xmm0
	shufps	xmm0, xmm0, 1
	addss	xmm1, xmm0
	;
	movss	xmm0, realConst1
	divss	xmm0, xmm1
	shufps	xmm0, xmm0, 0
	andps	xmm0, xmmMaskLow3SS
	;
	; A' = [B, C],  B' = [C, A],  C' = [A, B]
	;
	movups	xmm6, XMMWORD_PTR [ecx + 10H]
	movups	xmm7, XMMWORD_PTR [ecx + 20H]
	movaps	xmm1, xmm6
	movaps	xmm3, xmm7
	shufps	xmm6, xmm6, 11010010B
	shufps	xmm7, xmm7, 11001001B
	shufps	xmm1, xmm1, 11001001B
	shufps	xmm3, xmm3, 11010010B
	mulps	xmm6, xmm7
	mulps	xmm1, xmm3
	subps	xmm1, xmm6
	mulps	xmm1, xmm0			; xmm1 = 0. z1 y1 x1
	;
	movups	xmm6, XMMWORD_PTR [ecx + 20H]
	movups	xmm7, XMMWORD_PTR [ecx]
	movaps	xmm2, xmm6
	movaps	xmm3, xmm7
	shufps	xmm6, xmm6, 11010010B
	shufps	xmm7, xmm7, 11001001B
	shufps	xmm2, xmm2, 11001001B
	shufps	xmm3, xmm3, 11010010B
	mulps	xmm6, xmm7
	mulps	xmm2, xmm3
	subps	xmm2, xmm6
	mulps	xmm2, xmm0			; xmm2 = 0. z2 y2 x2
	;
	movups	xmm6, XMMWORD_PTR [ecx]
	movups	xmm7, XMMWORD_PTR [ecx + 10H]
	movaps	xmm3, xmm6
	movaps	xmm4, xmm7
	shufps	xmm6, xmm6, 11010010B
	shufps	xmm7, xmm7, 11001001B
	shufps	xmm3, xmm3, 11001001B
	shufps	xmm4, xmm4, 11010010B
	mulps	xmm6, xmm7
	mulps	xmm3, xmm4
	subps	xmm3, xmm6
	mulps	xmm3, xmm0			; xmm3 = 0. z3 y3 x3
	;
	movaps	xmm0, xmm1			; xmm0 = 0. z1 y1 x1
	shufps	xmm0, xmm2, 01000100B		; xmm0 = y2 x2 y1 x1
	shufps	xmm1, xmm2, 11101110B		; xmm1 = 0. z2 0. z1
	movaps	xmm4, xmm0
	shufps	xmm0, xmm3, 11001000B		; xmm0 = 0. x3 x2 x1
	shufps	xmm4, xmm3, 11011101B		; xmm4 = 0. y3 y2 y1
	shufps	xmm1, xmm3, 11101000B		; xmm1 = 0. z3 z2 z1
	;
	movups	XMMWORD_PTR [eax], xmm0
	movups	XMMWORD_PTR [eax + 10H], xmm4
	movups	XMMWORD_PTR [eax + 20H], xmm1
	;
	ASSUME	eax:NOTHING
	ASSUME	ecx:NOTHING
	ret
eglMatrixInverse_SSE	ENDP


;	行列回転 SSE
; ----------------------------------------------------------------------------
eglMatrixRevolveOnX_SSE		PROC	NEAR32 C,
			matrix:PE3D_REV_MATRIX, rSin:REAL4, rCos:REAL4
	; ( a11 a12 a13 )   ( 1    0       0    )
	; ( a21 a22 a23 ) X ( 0  cos(x)  sin(x) )
	; ( a31 a32 a33 )   ( 0 -sin(x)  cos(x) )
	mov	eax, matrix
	movss	xmm6, rCos
	movss	xmm7, rSin
	unpcklps	xmm6, xmm7
		movlps	xmm0, QWORD PTR [eax + 4]
	shufps	xmm6, xmm6, 00010100B
	xorps	xmm6, xmmRevolveX_SignMask
		unpcklps	xmm0, xmm0
		movlps	xmm2, QWORD PTR [eax + 14H]
	;
	mulps	xmm0, xmm6
		unpcklps	xmm2, xmm2
		movlps	xmm4, QWORD PTR [eax + 24H]
	mulps	xmm2, xmm6
		unpcklps	xmm4, xmm4
	mulps	xmm4, xmm6
	;
	movhlps	xmm1, xmm0
	movhlps	xmm3, xmm2
	movhlps	xmm5, xmm4
	addps	xmm0, xmm1
	addps	xmm2, xmm3
	addps	xmm4, xmm5
	;
	movlps	QWORD PTR [eax + 4], xmm0
	movlps	QWORD PTR [eax + 14H], xmm2
	movlps	QWORD PTR [eax + 24H], xmm4
	ret
eglMatrixRevolveOnX_SSE		ENDP

eglMatrixRevolveOnY_SSE		PROC	NEAR32 C,
			matrix:PE3D_REV_MATRIX, rSin:REAL4, rCos:REAL4
	; ( a11 a12 a13 )   (  cos(y)  0  sin(y) )
	; ( a21 a22 a23 ) X (    0     1    0    )
	; ( a31 a32 a33 )   ( -sin(y)  0  cos(y) )
	mov	eax, matrix
	ASSUME	eax:PTR REAL4
	movss	xmm6, rCos
	movss	xmm7, rSin
	unpcklps	xmm6, xmm7
		movups	xmm0, [eax]
	shufps	xmm6, xmm6, 00010100B
	xorps	xmm6, xmmRevolveY_SignMask
		shufps	xmm0, xmm0, 10100000B
		movups	xmm2, [eax + 10H]
	;
	mulps	xmm0, xmm6
		shufps	xmm2, xmm2, 10100000B
		movups	xmm4, [eax + 20H]
	mulps	xmm2, xmm6
		shufps	xmm4, xmm4, 10100000B
	mulps	xmm4, xmm6
	;
	movhlps	xmm1, xmm0
	movhlps	xmm3, xmm2
	movhlps	xmm5, xmm4
	addps	xmm0, xmm1
	addps	xmm2, xmm3
	addps	xmm4, xmm5
	;
	movss	[eax], xmm0
	shufps	xmm0, xmm0, 1
	movss	[eax + 10H], xmm2
	shufps	xmm2, xmm2, 1
	movss	[eax + 20H], xmm4
	shufps	xmm4, xmm4, 1
	movss	[eax+ 08H], xmm0
	movss	[eax+ 18H], xmm2
	movss	[eax+ 28H], xmm4
	ASSUME	eax:NOTHING
	ret
eglMatrixRevolveOnY_SSE		ENDP

eglMatrixRevolveOnZ_SSE		PROC	NEAR32 C,
			matrix:PE3D_REV_MATRIX, rSin:REAL4, rCos:REAL4
	; ( a11 a12 a13 )   ( cos(z) -sin(z)  0 )
	; ( a21 a22 a23 ) X ( sin(z)  cos(z)  0 )
	; ( a31 a32 a33 )   (   0       0     1 )

	mov	eax, matrix
	movss	xmm6, rCos
	movss	xmm7, rSin
	unpcklps	xmm6, xmm7
		movlps	xmm0, QWORD PTR [eax]
	shufps	xmm6, xmm6, 00010100B
	xorps	xmm6, xmmRevolveZ_SignMask
		unpcklps	xmm0, xmm0
		movlps	xmm2, QWORD PTR [eax + 10H]
	;
	mulps	xmm0, xmm6
		unpcklps	xmm2, xmm2
		movlps	xmm4, QWORD PTR [eax + 20H]
	mulps	xmm2, xmm6
		unpcklps	xmm4, xmm4
	mulps	xmm4, xmm6
	;
	movhlps	xmm1, xmm0
	movhlps	xmm3, xmm2
	movhlps	xmm5, xmm4
	addps	xmm0, xmm1
	addps	xmm2, xmm3
	addps	xmm4, xmm5
	;
	movlps	QWORD PTR [eax], xmm0
	movlps	QWORD PTR [eax + 10H], xmm2
	movlps	QWORD PTR [eax + 20H], xmm4
	ret
eglMatrixRevolveOnZ_SSE		ENDP

eglMatrixRevolveByAngleOn_SSE	PROC	NEAR32 C,
			matrix:PE3D_REV_MATRIX, pAngle:PCE3D_VECTOR
	LOCAL	rSqrtXZ:REAL4, rSqrtXYZ:REAL4
	LOCAL	rSinCos[2]:REAL4
	mov	edx, pAngle
	ASSUME	edx:PCE3D_VECTOR
	movlps	xmm0, QWORD PTR [edx]
	movss	xmm1, [edx].z
	movlhps	xmm0, xmm1
	mulps	xmm0, xmm0
	movhlps	xmm1, xmm0
	addss	xmm1, xmm0
	shufps	xmm0, xmm0, 1
	addss	xmm0, xmm1
	;
	sqrtss	xmm1, xmm1
	sqrtss	xmm0, xmm0
	movss	rSqrtXZ, xmm1
	movss	rSqrtXYZ, xmm0
	.IF	(DWORD PTR rSqrtXYZ) > 00800000H
		movss	xmm2, [edx].y
		shufps	xmm0, xmm0, 0
		unpcklps	xmm2, xmm1
		divps	xmm2, xmm0
		movlps	QWORD PTR rSinCos[0], xmm2
		mov	eax, rSinCos[0]
		xor	eax, 80000000H
		INVOKE	eglMatrixRevolveOnX_SSE, matrix, eax, rSinCos[4]
	.ENDIF
	mov	edx, pAngle
	.IF	(DWORD PTR rSqrtXZ) > 00800000H
		movss	xmm0, [edx].x
		movss	xmm1, [edx].z
		movss	xmm2, rSqrtXZ
		unpcklps	xmm0, xmm1
		shufps	xmm2, xmm2, 0
		divps	xmm0, xmm2
		movlps	QWORD PTR rSinCos[0], xmm0
		mov	eax, rSinCos[0]
		xor	eax, 80000000H
		INVOKE	eglMatrixRevolveOnY_SSE, matrix, eax, rSinCos[4]
	.ENDIF
	ASSUME	edx:NOTHING
	ret
eglMatrixRevolveByAngleOn_SSE	ENDP

eglMatrixRevolveForAngle_SSE	PROC	NEAR32 C,
			matrix:PE3D_REV_MATRIX, pAngle:PCE3D_VECTOR
	LOCAL	rSqrtXZ:REAL4, rSqrtXYZ:REAL4
	LOCAL	rSinCos[2]:REAL4
	mov	edx, pAngle
	ASSUME	edx:PCE3D_VECTOR
	movlps	xmm0, QWORD PTR [edx]
	movss	xmm1, [edx].z
	movlhps	xmm0, xmm1
	mulps	xmm0, xmm0
	movhlps	xmm1, xmm0
	addss	xmm1, xmm0
	shufps	xmm0, xmm0, 1
	addss	xmm0, xmm1
	;
	sqrtss	xmm1, xmm1
	sqrtss	xmm0, xmm0
	movss	rSqrtXZ, xmm1
	movss	rSqrtXYZ, xmm0
	.IF	(DWORD PTR rSqrtXZ) > 00800000H
		movss	xmm0, [edx].x
		movss	xmm2, [edx].z
		shufps	xmm1, xmm1, 0
		unpcklps	xmm0, xmm2
		divps	xmm0, xmm1
		movlps	QWORD PTR rSinCos[0], xmm0
		INVOKE	eglMatrixRevolveOnY_SSE, matrix, rSinCos[0], rSinCos[4]
	.ENDIF
	mov	edx, pAngle
	.IF	(DWORD PTR rSqrtXYZ) > 00800000H
		movss	xmm0, [edx].y
		movss	xmm1, rSqrtXZ
		movss	xmm2, rSqrtXYZ
		unpcklps	xmm0, xmm1
		shufps	xmm2, xmm2, 0
		divps	xmm0, xmm2
		movlps	QWORD PTR rSinCos[0], xmm0
		INVOKE	eglMatrixRevolveOnX_SSE, matrix, rSinCos[0], rSinCos[4]
	.ENDIF
	ASSUME	edx:NOTHING
	ret
eglMatrixRevolveForAngle_SSE	ENDP

eglMatrixMagnifyByVector_SSE	PROC	NEAR32 C,
			matrix:PE3D_REV_MATRIX, pv:PCE3D_VECTOR
	mov	eax, matrix
	mov	edx, pv
	ASSUME	edx:PCE3D_VECTOR
	movups	xmm0, [eax]
	movlps	xmm4, QWORD PTR [edx]
	movss	xmm5, [edx].z
	movups	xmm1, [eax + 10H]
	movlhps	xmm4, xmm5
	movups	xmm2, [eax + 20H]
	mulps	xmm0, xmm4
	mulps	xmm1, xmm4
	mulps	xmm2, xmm4
	movups	[eax], xmm0
	movups	[eax + 10H], xmm1
	movups	[eax + 20H], xmm2
	ASSUME	edx:NOTHING
	ret
eglMatrixMagnifyByVector_SSE	ENDP

eglMatrixRevolve_SSE		PROC	NEAR32 C USES esi edi,
			matrix:PCE3D_REV_MATRIX, matDst:PE3D_REV_MATRIX
	LOCAL	matTemp:E3D_REV_MATRIX
	pushfd
	cld
	mov	ecx, 4 * 3
	mov	esi, matDst
	lea	edi, matTemp
	rep	movsd
	;
	mov	ecx, 4 * 3
	mov	esi, matrix
	mov	edi, matDst
	rep	movsd
	popfd
	;
	INVOKE	eglMatrixRevolveBy_SSE, matDst, ADDR matTemp
	ret
eglMatrixRevolve_SSE		ENDP


eglMatrixRevolveBy_SSE		PROC	NEAR32 C,
			matDst:PE3D_REV_MATRIX, matSrc:PCE3D_REV_MATRIX
	mov	edx, matSrc
	mov	eax, matDst
	movups	xmm4, XMMWORD_PTR [edx]
	movups	xmm5, XMMWORD_PTR [edx + 10H]
	movups	xmm6, XMMWORD_PTR [edx + 20H]
	;
	movss	xmm0, REAL4 PTR [eax]
	movss	xmm1, REAL4 PTR [eax + 4]
	movss	xmm2, REAL4 PTR [eax + 8]
	shufps	xmm0, xmm0, 0
	shufps	xmm1, xmm1, 0
	shufps	xmm2, xmm2, 0
	mulps	xmm0, xmm4
	mulps	xmm1, xmm5
	mulps	xmm2, xmm6
	addps	xmm0, xmm1
	addps	xmm0, xmm2
	movups	XMMWORD_PTR [eax], xmm0
	;
	movss	xmm0, REAL4 PTR [eax + 10H]
	movss	xmm1, REAL4 PTR [eax + 14H]
	movss	xmm2, REAL4 PTR [eax + 18H]
	shufps	xmm0, xmm0, 0
	shufps	xmm1, xmm1, 0
	shufps	xmm2, xmm2, 0
	mulps	xmm0, xmm4
	mulps	xmm1, xmm5
	mulps	xmm2, xmm6
	addps	xmm0, xmm1
	addps	xmm0, xmm2
	movups	XMMWORD_PTR [eax + 10H], xmm0
	;
	movss	xmm0, REAL4 PTR [eax + 20H]
	movss	xmm1, REAL4 PTR [eax + 24H]
	movss	xmm2, REAL4 PTR [eax + 28H]
	shufps	xmm0, xmm0, 0
	shufps	xmm1, xmm1, 0
	shufps	xmm2, xmm2, 0
	mulps	xmm0, xmm4
	mulps	xmm1, xmm5
	mulps	xmm2, xmm6
	addps	xmm0, xmm1
	addps	xmm0, xmm2
	movups	XMMWORD_PTR [eax + 20H], xmm0
	ret
eglMatrixRevolveBy_SSE		ENDP

eglMatrixRevolveVector_SSE	PROC	NEAR32 C,
			matrix:PCE3D_REV_MATRIX, pv:PE3D_VECTOR
	mov	eax, matrix
	mov	edx, pv
	ASSUME	edx:PE3D_VECTOR
	movups	xmm0, [eax]
	movlps	xmm4, QWORD PTR [edx]
	movss	xmm5, [edx].z
	movlhps	xmm4, xmm5
	mulps	xmm0, xmm4
	movups	xmm1, [eax + 10H]
	mulps	xmm1, xmm4
		movhlps	xmm7, xmm0
	movups	xmm2, [eax + 20H]
		addss	xmm7, xmm0
		shufps	xmm0, xmm0, 1
	mulps	xmm2, xmm4
			movhlps	xmm6, xmm1
		addss	xmm0, xmm7
	movhlps	xmm5, xmm2
			addss	xmm6, xmm1
			shufps	xmm1, xmm1, 1
	addss	xmm5, xmm2
	shufps	xmm2, xmm2, 1
		movss	[edx].x, xmm0
			addss	xmm1, xmm6
	addss	xmm2, xmm5
			movss	[edx].y, xmm1
	movss	[edx].z, xmm2
	ASSUME	edx:NOTHING
	ret
eglMatrixRevolveVector_SSE	ENDP


eglMatrixRevolveVectors_SSE	PROC	NEAR32 C USES esi edi,
		matrix:PCE3D_REV_MATRIX, pvDst:PE3D_VECTOR4,
		pvSrc:PCE3D_VECTOR4, pvOrigin:PE3D_VECTOR, nCount:DWORD
	LOCAL	vOrigin[2]:E3D_VECTOR4

	mov	eax, matrix
	mov	esi, pvSrc
	mov	edi, pvDst
	;
	; 行列をスウィズリングして、xmm0, xmm1, xmm2 に読み込む
	;
	ASSUME	eax:PTR QWORD
	prefetchnta	[esi]
	xorps	xmm1, xmm1
	xorps	xmm3, xmm3
	prefetchnta	[esi + 20H*1]
	movlps	xmm0, [eax][0][0]		; xmm0 = y2 x2 y1 x1
	movlps	xmm1, [eax][20H][0]		; xmm1 = 0. 0. y3 x3
	movlps	xmm2, [eax][0][8]		; xmm2 = -- z2 -- z1
	movlps	xmm3, [eax][20H][8]		; xmm3 = 0. 0. -- z3
	prefetchnta	[esi + 20H*2]
	movhps	xmm0, [eax][10H][0]
	prefetchnta	[esi + 20H*3]
	movhps	xmm2, [eax][10H][8]
	prefetchnta	[esi + 20H*4]
	movaps	xmm4, xmm0
	prefetchnta	[esi + 20H*5]
	shufps	xmm0, xmm1, 10001000B		; xmm0 = 0. x3 x2 x1
	shufps	xmm4, xmm1, 11011101B		; xmm4 = 0. y3 y2 y1
	prefetchnta	[esi + 20H*6]
	shufps	xmm2, xmm3, 10001000B		; xmm2 = 0. z3 z2 z1
	movaps	xmm1, xmm4			; xmm1 = 0. y3 y2 y1
	prefetchnta	[esi + 20H*7]
	ASSUME	eax:NOTHING
	;
	; 原点計算
	;
	lea	eax, vOrigin[0FH]
	mov	ecx, pvOrigin
	and	eax, NOT 0FH
	ASSUME	eax:PE3D_VECTOR4
	.IF	ecx != NULL
		ASSUME	ecx:PE3D_VECTOR
		mov	edx, [ecx].x
		mov	[eax].x, edx
		mov	edx, [ecx].y
		mov	[eax].y, edx
		mov	edx, [ecx].z
		mov	[eax].z, edx
		mov	[eax].d, 0
		ASSUME	ecx:NOTHING
	.ELSE
		xor	edx, edx
		mov	[eax].x, edx
		mov	[eax].y, edx
		mov	[eax].z, edx
		mov	[eax].d, edx
	.ENDIF
	ASSUME	eax:NOTHING
	;
	; 順次ベクトル変換
	;
	ASSUME	esi:PE3D_VECTOR4
	mov	ecx, nCount
	.IF	!(edi & 0FH) && (ecx > 32)
		sub	ecx, 2
		.WHILE	!SIGN?
			prefetchnta	[esi + 20H*8]
			movss	xmm3, [esi].x
			movss	xmm4, [esi].y
			movss	xmm5, [esi].z
			shufps	xmm3, xmm3, 0
			shufps	xmm4, xmm4, 0
			shufps	xmm5, xmm5, 0
			mulps	xmm3, xmm0
				movss	xmm6, [esi][10H].x
				movss	xmm7, [esi][10H].y
			mulps	xmm4, xmm1
				shufps	xmm6, xmm6, 0
				shufps	xmm7, xmm7, 0
			mulps	xmm5, xmm2
			addps	xmm3, xmm4
				mulps	xmm6, xmm0
			addps	xmm5, [eax]
				movss	xmm4, [esi][10H].z
				mulps	xmm7, xmm1
				shufps	xmm4, xmm4, 0
			addps	xmm3, xmm5
				mulps	xmm4, xmm2
				addps	xmm6, xmm7
				addps	xmm4, [eax]
			movntps	[edi], xmm3
				addps	xmm4, xmm6
			add	esi, 20H
				movntps	[edi][10H], xmm4
			add	edi, 20H
			sub	ecx, 2
		.ENDW
		add	ecx, 2
		.IF	!ZERO?
			movss	xmm4, [esi].x
			movss	xmm5, [esi].y
			movss	xmm6, [esi].z
			shufps	xmm4, xmm4, 0
			shufps	xmm5, xmm5, 0
			shufps	xmm6, xmm6, 0
			mulps	xmm4, xmm0
			mulps	xmm5, xmm1
			mulps	xmm6, xmm2
			addps	xmm4, xmm5
			addps	xmm6, [eax]
			addps	xmm4, xmm6
			movntps	[edi], xmm4
		.ENDIF
	.ELSE
		sub	ecx, 2
		.WHILE	!SIGN?
			movss	xmm3, [esi].x
			movss	xmm4, [esi].y
			movss	xmm5, [esi].z
			shufps	xmm3, xmm3, 0
			shufps	xmm4, xmm4, 0
			shufps	xmm5, xmm5, 0
			mulps	xmm3, xmm0
				movss	xmm6, [esi][10H].x
				movss	xmm7, [esi][10H].y
			mulps	xmm4, xmm1
				shufps	xmm6, xmm6, 0
				shufps	xmm7, xmm7, 0
			mulps	xmm5, xmm2
			addps	xmm3, xmm4
				mulps	xmm6, xmm0
			addps	xmm5, [eax]
				movss	xmm4, [esi][10H].z
				mulps	xmm7, xmm1
				shufps	xmm4, xmm4, 0
			addps	xmm3, xmm5
				mulps	xmm4, xmm2
				addps	xmm6, xmm7
				addps	xmm4, [eax]
			movups	[edi], xmm3
				addps	xmm4, xmm6
			add	esi, 20H
				movups	[edi][10H], xmm4
			add	edi, 20H
			sub	ecx, 2
		.ENDW
		add	ecx, 2
		.IF	!ZERO?
			movss	xmm4, [esi].x
			movss	xmm5, [esi].y
			movss	xmm6, [esi].z
			shufps	xmm4, xmm4, 0
			shufps	xmm5, xmm5, 0
			shufps	xmm6, xmm6, 0
			mulps	xmm4, xmm0
			mulps	xmm5, xmm1
			mulps	xmm6, xmm2
			addps	xmm4, xmm5
			addps	xmm6, [eax]
			addps	xmm4, xmm6
			movups	[edi], xmm4
		.ENDIF
	.ENDIF
	ASSUME	esi:NOTHING
	ret

eglMatrixRevolveVectors_SSE	ENDP


eglGetMinVector_SSE		PROC NEAR32 C,
			pvMin:PE3D_VECTOR4, pvList:PCE3D_VECTOR, nCount:DWORD

	mov	ecx, nCount
	mov	eax, pvList
	test	ecx, ecx
	.IF	!ZERO?
		movups	xmm0, [eax]
		add	eax, (SIZEOF E3D_VECTOR4)
		dec	ecx
		.WHILE	!ZERO?
			movups	xmm1, [eax]
			add	eax, (SIZEOF E3D_VECTOR4)
			dec	ecx
			minps	xmm0, xmm1
		.ENDW
		mov	edx, pvMin
		andps	xmm0, xmmMaskLow3SS
		movups	[edx], xmm0
	.ENDIF
	ret

eglGetMinVector_SSE		ENDP


eglGetMaxVector_SSE		PROC NEAR32 C,
			pvMax:PE3D_VECTOR4, pvList:PCE3D_VECTOR, nCount:DWORD

	mov	ecx, nCount
	mov	eax, pvList
	test	ecx, ecx
	.IF	!ZERO?
		movups	xmm0, [eax]
		add	eax, (SIZEOF E3D_VECTOR4)
		dec	ecx
		.WHILE	!ZERO?
			movups	xmm1, [eax]
			add	eax, (SIZEOF E3D_VECTOR4)
			dec	ecx
			maxps	xmm0, xmm1
		.ENDW
		mov	edx, pvMax
		andps	xmm0, xmmMaskLow3SS
		movups	[edx], xmm0
	.ENDIF
	ret

eglGetMaxVector_SSE		ENDP



CodeSeg	ENDS

	END
