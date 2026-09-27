
; ----------------------------------------------------------------------------
;                          Entis Graphic Library
; ----------------------------------------------------------------------------
;    Copyright (c) 2002-2007 Leshade Entis, Entis-soft. Al rights reserved.
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
rConstReal1		REAL4	1.0
rConstRealDiv65536	LABEL	REAL4
	DWORD	37800000H		; 2^-16
rConstMask80000000	DWORD	80000000H

pfnDrawApplyTable32MMX	LABEL	DWORD
	DWORD	OFFSET eglDrawImage@Apply_AddColor_MMX
	DWORD	OFFSET eglDrawImage@Apply_nothing
	DWORD	OFFSET eglDrawImage@Apply_MulColor_MMX
	DWORD	5 DUP( OFFSET eglDrawImage@Apply_nothing )
	DWORD	OFFSET eglDrawImage@Product_Alpha_MMX
	DWORD	OFFSET eglDrawImage@Apply_ColorMask_MMX
	DWORD	6 DUP( OFFSET eglDrawImage@Apply_nothing )

pfnDrawApplyTable32	LABEL	DWORD
	DWORD	OFFSET eglDrawImage@Apply_AddColor
	DWORD	OFFSET eglDrawImage@Apply_nothing
	DWORD	OFFSET eglDrawImage@Apply_MulColor
	DWORD	5 DUP( OFFSET eglDrawImage@Apply_nothing )
	DWORD	OFFSET eglDrawImage@Product_Alpha
	DWORD	OFFSET eglDrawImage@Apply_ColorMask
	DWORD	6 DUP( OFFSET eglDrawImage@Apply_nothing )

pfnDrawFuncTable32MMX	LABEL	DWORD
	DWORD	OFFSET eglDrawImage@Render_AddColor_MMX
	DWORD	OFFSET eglDrawImage@Render_SubColor_MMX
	DWORD	OFFSET eglDrawImage@Render_MulColor_MMX
	DWORD	OFFSET eglDrawImage@Render_DivColor_MMX
	DWORD	OFFSET eglDrawImage@Render_MaxValue
	DWORD	OFFSET eglDrawImage@Render_MinValue
	DWORD	OFFSET eglDrawImage@Render_NegMulColor
	DWORD	0
	DWORD	OFFSET eglDrawImage@Render_MoveColor_MMX
	DWORD	OFFSET eglDrawImage@Render_AMulColor_MMX
	DWORD	OFFSET eglDrawImage@Render_DstMaskColor
	DWORD	0, 0, 0, 0, 0

pfnDrawFuncTable32	LABEL	DWORD
	DWORD	OFFSET eglDrawImage@Render_AddColor
	DWORD	OFFSET eglDrawImage@Render_SubColor
	DWORD	OFFSET eglDrawImage@Render_MulColor
	DWORD	OFFSET eglDrawImage@Render_DivColor
	DWORD	OFFSET eglDrawImage@Render_MaxValue
	DWORD	OFFSET eglDrawImage@Render_MinValue
	DWORD	OFFSET eglDrawImage@Render_NegMulColor
	DWORD	0
	DWORD	OFFSET eglDrawImage@Render_MoveColor
	DWORD	OFFSET eglDrawImage@Render_AMulColor
	DWORD	OFFSET eglDrawImage@Render_DstMaskColor
	DWORD	0, 0, 0, 0, 0

pfnDrawFuncTable8	LABEL	DWORD
	DWORD	OFFSET eglDrawImage@Render_AddColor8
	DWORD	OFFSET eglDrawImage@Render_SubColor8
	DWORD	OFFSET eglDrawImage@Render_MulColor8
	DWORD	OFFSET eglDrawImage@Render_DivColor8
	DWORD	OFFSET eglDrawImage@Render_MaxValue8
	DWORD	OFFSET eglDrawImage@Render_MinValue8
	DWORD	0, 0
	DWORD	0, 0, 0, 0, 0, 0, 0, 0

ConstSeg	ENDS


; ----------------------------------------------------------------------------
;	コードセグメント
; ----------------------------------------------------------------------------

CodeSeg	SEGMENT	PARA READONLY FLAT 'CODE'

;
;	DrawImage オブジェクト生成
; ----------------------------------------------------------------------------
ALIGN	10H
eglCreateDrawImage		PROC	NEAR32 C USES ebx esi edi

	LOCAL	hHeap:HESLHEAP

	;
	; ヒープを作成し、メモリを確保
	;
	.IF	EGL_hImageHeap == NULL
		INVOKE	eslHeapCreate , 0, 0, ESL_HEAP_ZERO_INIT, NULL
		mov	EGL_hImageHeap, eax
	.ENDIF
	INVOKE	eslHeapCreate , 2000H, 0, ESL_HEAP_NO_SERIALIZE, EGL_hImageHeap
	mov	hHeap, eax
	;
	INVOKE	eslHeapAllocate ,
		hHeap, (SIZEOF EGL_DRAW_IMAGE_BUF) + 10H, ESL_HEAP_ZERO_INIT
	add	eax, 0FH
	and	eax, NOT 0FH
	mov	ebx, eax
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF
	;
	; 関数ポインタを初期化
	;
	mov	[ebx].pfnRelease, OFFSET eglDrawImage@Release
	mov	[ebx].pfnInitialize, OFFSET eglDrawImage@Initialize
	mov	[ebx].pfnGetDestination, OFFSET eglDrawImage@GetDestination
	mov	[ebx].pfnGetFunctionFlags, OFFSET eglDrawImage@GetFunctionFlags
	mov	[ebx].pfnSetFunctionFlags, OFFSET eglDrawImage@SetFunctionFlags
	mov	[ebx].pfnGetDrawingOffset, OFFSET eglDrawImage@GetDrawingOffset
	mov	[ebx].pfnSetDrawingOffset, OFFSET eglDrawImage@SetDrawingOffset
	mov	[ebx].pfnPrepareDraw, OFFSET eglDrawImage@PrepareDraw
	mov	[ebx].pfnDrawImage, OFFSET eglDrawImage@DrawImage_NotSupported
	mov	[ebx].pfnPrepareLine, OFFSET eglDrawImage@PrepareLine
	mov	[ebx].pfnPrepareFillRect, OFFSET eglDrawImage@PrepareFillRect
	mov	[ebx].pfnPrepareFillEllipse, OFFSET eglDrawImage@PrepareFillEllipse
	mov	[ebx].pfnPrepareFillPolygon, OFFSET eglDrawImage@PrepareFillPolygon
	mov	[ebx].pfnFillRegion, OFFSET eglDrawImage@FillRegion_Error
	mov	[ebx].pfnDrawRegion, OFFSET eglDrawImage@FillRegion_Error
	mov	[ebx].dwDrawFlags, 0
	;
	; 変数を初期化
	;
	mov	eax, hHeap
	mov	[ebx].hHeap, eax

	ASSUME	ebx:NOTHING
	mov	eax, ebx
	ret

eglCreateDrawImage		ENDP

;
;	DrawImage オブジェクトを解放する
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@Release			PROC	NEAR32 C USES ebx esi edi,
		hDrawImage:HEGL_DRAW_IMAGE

	mov	ebx, hDrawImage
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF

	INVOKE	eslHeapDestroy , [ebx].hHeap

	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglDrawImage@Release			ENDP

;
;	DrawImage オブジェクトを初期化する
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@Initialize			PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE, pDstImage:PEGL_IMAGE_INFO,
	pClipRect:PCEGL_RECT, pZBuffer:PEGL_IMAGE_INFO

	mov	ebx, hDrawImage
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF

	.IF	[ebx].pRegion != NULL
		INVOKE	eslHeapFree , [ebx].hHeap, [ebx].pRegion, 0
		mov	[ebx].pRegion, NULL
	.ENDIF
	.IF	[ebx].pDrawLineBuf[4] != NULL
		INVOKE	eslHeapFree , [ebx].hHeap, [ebx].pDrawLineBuf[4], 0
		mov	[ebx].pDrawLineBuf[0], NULL
		mov	[ebx].pDrawLineBuf[4], NULL
	.ENDIF

	mov	esi, pDstImage
	mov	edi, pZBuffer
	mov	[ebx].pDstImage, esi
	mov	[ebx].pZBuffer, edi
	;
	ASSUME	esi:PEGL_IMAGE_INFO
	ASSUME	edi:PEGL_IMAGE_INFO
	;
	.IF	esi == NULL
		mov	eax, eslErrGeneral
		ret
	.ENDIF
	mov	ecx, [esi].dwImageWidth
	mov	edx, [esi].dwImageHeight
	.IF	edi != NULL
		.IF	([edi].dwImageWidth != ecx) \
				|| ([edi].dwImageHeight != edx)
			mov	eax, eslErrGeneral
			ret
		.ENDIF
	.ENDIF
	;
	imul	edx, (SIZEOF E3D_POLY_LINE_REGION)
	add	edx, (SIZEOF E3D_POLYGON_REGION)
	INVOKE	eslHeapAllocate , [ebx].hHeap, edx, 0
	mov	[ebx].pRegion, eax
	;
	mov	ecx, [esi].dwImageWidth
	shl	ecx, 2
	add	ecx, 20H
	INVOKE	eslHeapAllocate , [ebx].hHeap, ecx, 0
	mov	[ebx].pDrawLineBuf[4], eax
	add	eax, 0FH
	and	eax, NOT 0FH
	mov	[ebx].pDrawLineBuf[0], eax
	;
	mov	edi, pClipRect
	ASSUME	edi:PCEGL_RECT
	.IF	edi != NULL
		mov	ecx, [edi].left
		mov	edx, [edi].right
		.IF	(SDWORD PTR ecx) > (SDWORD PTR edx)
			xchg	ecx, edx
		.ENDIF
		test	ecx, ecx
		.IF	SIGN?
			xor	ecx, ecx
		.ELSEIF	(SDWORD PTR ecx) >= (SDWORD PTR [esi].dwImageWidth)
			mov	ecx, [esi].dwImageWidth
			dec	ecx
		.ENDIF
		test	edx, edx
		.IF	SIGN?
			xor	edx, edx
		.ELSEIF	(SDWORD PTR edx) >= (SDWORD PTR [esi].dwImageWidth)
			mov	edx, [esi].dwImageWidth
			dec	edx
		.ENDIF
		mov	[ebx].rectClip.left, ecx
		mov	[ebx].rectClip.right, edx
		;
		mov	ecx, [edi].top
		mov	edx, [edi].bottom
		.IF	(SDWORD PTR ecx) > (SDWORD PTR edx)
			xchg	ecx, edx
		.ENDIF
		test	ecx, ecx
		.IF	SIGN?
			xor	ecx, ecx
		.ELSEIF	(SDWORD PTR ecx) >= (SDWORD PTR [esi].dwImageHeight)
			mov	ecx, [esi].dwImageHeight
			dec	ecx
		.ENDIF
		test	edx, edx
		.IF	SIGN?
			xor	edx, edx
		.ELSEIF	(SDWORD PTR edx) >= (SDWORD PTR [esi].dwImageHeight)
			mov	edx, [esi].dwImageHeight
			dec	edx
		.ENDIF
		mov	[ebx].rectClip.top, ecx
		mov	[ebx].rectClip.bottom, edx
	.ELSE
		mov	[ebx].rectClip.left, 0
		mov	[ebx].rectClip.top, 0
		mov	ecx, [esi].dwImageWidth
		mov	edx, [esi].dwImageHeight
		dec	ecx
		dec	edx
		mov	[ebx].rectClip.right, ecx
		mov	[ebx].rectClip.bottom, edx
	.ENDIF
	;
	ASSUME	esi:NOTHING
	ASSUME	edi:NOTHING

	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglDrawImage@Initialize			ENDP

;
;	画像出力先バッファ取得
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@GetDestination		PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE, pDrawDest:PTR EGL_DRAW_DEST

	mov	ebx, hDrawImage
	mov	edi, pDrawDest
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF
	ASSUME	edi:PTR EGL_DRAW_DEST

	mov	eax, [ebx].pDstImage
	mov	edx, [ebx].pZBuffer
	mov	[edi].pDstImage, eax
	mov	[edi].pZBuffer, edx
	mov	eax, [ebx].rectClip.left
	mov	edx, [ebx].rectClip.top
	mov	[edi].rectDstClip.left, eax
	mov	[edi].rectDstClip.top, edx
	mov	eax, [ebx].rectClip.right
	mov	edx, [ebx].rectClip.bottom
	mov	[edi].rectDstClip.right, eax
	mov	[edi].rectDstClip.bottom, edx

	ASSUME	ebx:NOTHING
	ASSUME	edi:NOTHING
	xor	eax, eax
	ret

eglDrawImage@GetDestination		ENDP

;
;	描画機能フラグを取得
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@GetFunctionFlags		PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE

	mov	ebx, hDrawImage
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF
	mov	eax, [ebx].dwDrawFlags
	ASSUME	ebx:NOTHING
	ret

eglDrawImage@GetFunctionFlags		ENDP

;
;	描画機能フラグを設定
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@SetFunctionFlags		PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE, dwFlags:DWORD

	mov	ebx, hDrawImage
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF
	mov	eax, dwFlags
	and	eax, EGL_SMOOTH_STRETCH OR EGL_UNSMOOTH_STRETCH
	mov	[ebx].dwDrawFlags, eax
	ASSUME	ebx:NOTHING
	ret

eglDrawImage@SetFunctionFlags		ENDP

;
;	描画オフセット座標取得
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@GetDrawingOffset		PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE, pDrawOffset:PTR EGL_POINT

	mov	ebx, hDrawImage
	mov	edi, pDrawOffset
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF
	ASSUME	edi:PTR EGL_POINT
	mov	eax, [ebx].ptDrawOffset.x
	mov	edx, [ebx].ptDrawOffset.y
	mov	[edi].x, eax
	mov	[edi].y, edx
	ASSUME	ebx:NOTHING
	ASSUME	edi:NOTHING
	ret

eglDrawImage@GetDrawingOffset		ENDP

;
;	描画オフセット座標設定
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@SetDrawingOffset		PROC	NEAR32 C USES ebx esi edi,
	hDrawImage:HEGL_DRAW_IMAGE, pDrawOffset:PTR EGL_POINT

	mov	ebx, hDrawImage
	mov	esi, pDrawOffset
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF
	ASSUME	esi:PTR EGL_POINT
	mov	eax, [esi].x
	mov	edx, [esi].y
	mov	[ebx].ptDrawOffset.x, eax
	mov	[ebx].ptDrawOffset.y, edx
	ASSUME	ebx:NOTHING
	ASSUME	edi:NOTHING
	ret

eglDrawImage@SetDrawingOffset		ENDP

;
;	画像描画準備
; ----------------------------------------------------------------------------
ALIGN	10H
eglDrawImage@PrepareDraw		PROC	NEAR32 C USES ebx esi edi,
		hDrawImage:HEGL_DRAW_IMAGE, pDrawParam:PTR EGL_DRAW_PARAM

	LOCAL	dwSaveESP:DWORD
	LOCAL	dwTemp[4]:DWORD
	LOCAL	rRcpD:REAL4
	LOCAL	irectClip:EGL_IMAGE_RECT
	LOCAL	ptBasePos:EGL_POINT
	LOCAL	sizeSrc:EGL_SIZE
	LOCAL	egliax:EGL_IMAGE_AXES
	LOCAL	xLength:REAL4, yLength:REAL4
	LOCAL	vRevRect[4]:E3D_VECTOR_2D

	mov	ebx, hDrawImage
	mov	esi, pDrawParam
	ASSUME	ebx:PTR EGL_DRAW_IMAGE_BUF
	ASSUME	esi:PTR EGL_DRAW_PARAM
	mov	[ebx].pfnDrawImage, OFFSET eglDrawImage@DrawImage_NotSupported
	.IF	[esi].nTransparency >= 100H
		mov	[ebx].pfnDrawImage, OFFSET eglDrawImage@DrawImage_NoDraw
		xor	eax, eax
		ret
	.ENDIF

	;
	;	変形描画を行うか判定
	; --------------------------------------------------------------------
	mov	eax, [ebx].dwDrawFlags
	and	eax, EGL_SMOOTH_STRETCH OR EGL_UNSMOOTH_STRETCH
	or	eax, [esi].dwFlags
	.IF	[ebx].pZBuffer == NULL
		and	eax, NOT EGL_WITH_Z_ORDER
	.ENDIF
	mov	[ebx].dwFlags, eax
;	.IF	eax & EGL_WITH_Z_ORDER
;		or	eax, EGL_WITH_AXES
;		mov	[ebx].dwFlags, eax
;	.ENDIF
	.IF	[esi].dwFlags & EGL_POLYGON_SHAPED
		.IF	([esi].nVertexCount < 3) || ([esi].pVertexPos == NULL)
			mov	eax, eslErrInvalidParam
			ret
		.ENDIF
		or	eax, EGL_WITH_AXES
		mov	[ebx].dwFlags, eax
	.ELSEIF	[esi].dwFlags & EGL_FIXED_POSITION
		.IF	([esi].dwFlags & EGL_SMOOTH_STRETCH) \
				&& !([esi].dwFlags & EGL_UNSMOOTH_STRETCH)
			.IF	([esi].ptBasePos.x & 0FFFFH) \
					|| ([esi].ptBasePos.y & 0FFFFH)
				or	eax, EGL_WITH_AXES
				mov	[ebx].dwFlags, eax
			.ENDIF
		.ENDIF
	.ENDIF
	.IF	[esi].pImageAxes != NULL
		mov	edi, [esi].pImageAxes
		ASSUME	edi:PTR EGL_IMAGE_AXES
		mov	ecx, [edi].xAxis.y
		mov	edx, [edi].yAxis.x
		mov	egliax.xAxis.y, ecx
		mov	egliax.yAxis.x, edx
		and	ecx, 7FFFFFFFH
		and	edx, 7FFFFFFFH
		.IF	(ecx < 37800000H) && (edx < 37800000H)
			.IF	!(ERI_EnabledProcessorType & ERI_USE_XMM_P3)
				fld	[edi].xAxis.x
				fst	egliax.xAxis.x
				fld	[edi].yAxis.y
				fst	egliax.yAxis.y
				fld1
				fsub	st(2), st
				fsubp	st(1), st
				fstp	REAL4 PTR dwTemp[0]
				fstp	REAL4 PTR dwTemp[4]
				mov	ecx, dwTemp[0]
				mov	edx, dwTemp[4]
				and	ecx, 7FFFFFFFH
				and	edx, 7FFFFFFFH
				.IF	(ecx >= 37800000H) || (edx >= 37800000H)
					or	eax, EGL_WITH_AXES
					mov	[ebx].dwFlags, eax
				.ENDIF
			.ELSE
				movss	xmm0, [edi].xAxis.x
				movss	xmm1, [edi].yAxis.y
				movss	xmm2, rConstReal1
				movss	egliax.xAxis.x, xmm0
				movss	egliax.yAxis.y, xmm1
				subss	xmm0, xmm2
				subss	xmm1, xmm2
				movss	REAL4 PTR dwTemp[0], xmm0
				movss	REAL4 PTR dwTemp[4], xmm1
				mov	ecx, dwTemp[0]
				mov	edx, dwTemp[4]
				and	ecx, 7FFFFFFFH
				and	edx, 7FFFFFFFH
				.IF	(ecx >= 37800000H) || (edx >= 37800000H)
					or	eax, EGL_WITH_AXES
					mov	[ebx].dwFlags, eax
				.ENDIF
			.ENDIF
		.ELSE
			or	eax, EGL_WITH_AXES
			mov	[ebx].dwFlags, eax
			;
			mov	ecx, [edi].xAxis.x
			mov	edx, [edi].yAxis.y
			mov	egliax.xAxis.x, ecx
			mov	egliax.yAxis.y, edx
		.ENDIF
		ASSUME	edi:NOTHING
	.ELSE
		.IF	eax & EGL_WITH_AXES
			mov	egliax.xAxis.x, 3F800000H
			mov	egliax.yAxis.y, 3F800000H
			mov	egliax.xAxis.y, 0
			mov	egliax.yAxis.x, 0
		.ENDIF
	.ENDIF
	;
	.IF	!([ebx].dwFlags & EGL_WITH_AXES)
	;
	;	変形無し描画
	; --------------------------------------------------------------------
	;
	; 有効な矩形を取得
	;
	mov	ecx, [esi].pSrcImage
	ASSUME	ecx:PEGL_IMAGE_INFO
	mov	eax, [ecx].dwImageWidth
	mov	edx, [ecx].dwImageHeight
	mov	sizeSrc.w, eax
	mov	sizeSrc.h, edx
	ASSUME	ecx:NOTHING
	mov	eax, [esi].ptBasePos.x
	mov	edx, [esi].ptBasePos.y
	.IF	[esi].dwFlags & EGL_FIXED_POSITION
		sar	eax, 16
		adc	eax, 0
		sar	edx, 16
		adc	edx, 0
	.ENDIF
	add	eax, [ebx].ptDrawOffset.x
	add	edx, [ebx].ptDrawOffset.y
	mov	ptBasePos.x, eax
	mov	ptBasePos.y, edx
	INVOKE	eglGetOverlappedRectangle ,
			ADDR irectClip, ADDR [ebx].rectClip,
			[esi].pViewRect, ADDR ptBasePos, ADDR sizeSrc
	test	eax, eax
	.IF	ZERO?			; 有効な描画領域なし
		mov	[ebx].pfnDrawImage, \
				OFFSET eglDrawImage@DrawImage_NoDraw
		ret
	.ENDIF
	;
	; 画像バッファをクリップ
	;
	INVOKE	eglGetClippedImageInfo ,
			ADDR [ebx].srcimg, [esi].pSrcImage, ADDR irectClip
	test	eax, eax
	.IF	!ZERO?			; 有効な描画領域なし
		mov	[ebx].pfnDrawImage, \
				OFFSET eglDrawImage@DrawImage_NoDraw
		ret
	.ENDIF
	;
	mov	eax, [esi].pViewRect
	mov	ecx, irectClip.x
	mov	edx, irectClip.y
	.IF	eax != NULL
		ASSUME	eax:PCEGL_RECT
		sub	ecx, [eax].left
		sub	edx, [eax].top
		ASSUME	eax:NOTHING
	.ENDIF
	add	ecx, ptBasePos.x
	add	edx, ptBasePos.y
	mov	irectClip.x, ecx
	mov	irectClip.y, edx
	;
	INVOKE	eglGetClippedImageInfo ,
			ADDR [ebx].dstimg, [ebx].pDstImage, ADDR irectClip
	test	eax, eax
	.IF	!ZERO?			; 有効な描画領域なし
		mov	[ebx].pfnDrawImage, \
				OFFSET eglDrawImage@DrawImage_NoDraw
		ret
	.ENDIF
	;
	mov	[ebx].zbuf.ptrImageArray, NULL
	mov	[ebx].zbuf.dwBytesPerLine, 0
	.IF	[ebx].dwFlags & EGL_WITH_Z_ORDER
		INVOKE	eglGetClippedImageInfo ,
			ADDR [ebx].zbuf, [ebx].pZBuffer, ADDR irectClip
	.ENDIF
	;
	;	通常画像・変形なし描画
	; --------------------------------------------------------------------
	;
	; 特殊描画条件判定
	;
	mov	eax, [ebx].dwFlags
	.IF	!(eax & (EGL_DRAW_GLOW_LIGHT OR 0FFFF0000H))
	;
	.IF	eax & EGL_DRAW_BLEND_ALPHA
		.IF	!([ebx].srcimg.fdwFormatType & EIF_WITH_ALPHA)
			and	eax, NOT EGL_DRAW_BLEND_ALPHA
			mov	[ebx].dwFlags, eax
		.ENDIF
	.ELSEIF	[ebx].srcimg.fdwFormatType & EIF_WITH_ALPHA
		.IF	!([ebx].dstimg.fdwFormatType & EIF_WITH_ALPHA)
			or	eax, EGL_DRAW_BLEND_ALPHA
			mov	[ebx].dwFlags, eax
		.ENDIF
	.ENDIF
	mov	ecx, [ebx].srcimg.dwBitsPerPixel
	mov	edx, [ebx].dstimg.dwBitsPerPixel
	;
	.IF	ecx == 8
		.IF	edx == 8
			.IF	[ebx].srcimg.fdwFormatType & EIF_WITH_CLIPPING
				mov	eax, OFFSET eglDrawImage@DrawImage_8to8_Clip
			.ELSE
				mov	eax, OFFSET eglDrawImage@DrawImage_8to8
			.ENDIF
		.ELSEIF	(edx == 24) || (edx == 32)
			mov	eax, [esi].nTransparency
			.IF	eax == 0
				.IF	[ebx].srcimg.fdwFormatType & EIF_WITH_CLIPPING
					mov	eax, OFFSET eglDrawImage@DrawImage_8to24_Clip
				.ELSE
					mov	eax, OFFSET eglDrawImage@DrawImage_8to24
				.ENDIF
			.ELSEIF	eax < 100H
				mov	[ebx].nTrans, eax
				.IF	[ebx].srcimg.fdwFormatType & EIF_WITH_CLIPPING
					mov	eax, OFFSET eglDrawImage@DrawImage_8to24_CTrans
				.ELSE
					mov	eax, OFFSET eglDrawImage@DrawImage_8to24_Trans
				.ENDIF
			.ELSE
				mov	eax, OFFSET eglDrawImage@DrawImage_NoDraw
			.ENDIF
;		.ELSE
;			mov	eax, eslErrGeneral
;			ret
		.ENDIF
		;
		mov	[ebx].pfnDrawImage, eax
		xor	eax, eax
		ret

	.ELSEIF	([esi].nTransparency == 0) && \
		!([ebx].dwFlags & (EGL_DRAW_BLEND_ALPHA OR EGL_WITH_Z_ORDER))
		xor	eax, eax
		.IF	(ecx == edx)
			.IF	ERI_EnabledProcessorType & ERI_USE_XMM_P3
				mov	eax, OFFSET eglDrawImage@DrawImage_24to24_SSE
			.ELSE
				mov	eax, OFFSET eglDrawImage@DrawImage_24to24
			.ENDIF
		.ELSEIF	(ecx == 24) || (edx == 24)
			mov	eax, OFFSET eglDrawImage@DrawImage_24to24
		.ENDIF
		.IF	eax != NULL
			mov	[ebx].pfnDrawImage, eax
			xor	eax, eax
			ret
		.ENDIF

	.ENDIF
IF	0
	.ELSEIF	((ecx == 24) || (ecx == 32)) && ((edx == 24) || (edx == 32))
		.IF	!([ebx].srcimg.fdwFormatType & EIF_WITH_ALPHA)
			mov	eax, [esi].nTransparency
			.IF	eax == 0
				.IF	(ecx == edx) && \
					(ERI_EnabledProcessorType & ERI_USE_XMM_P3)
					mov	eax, OFFSET eglDrawImage@DrawImage_24to24_SSE
				.ELSE
					mov	eax, OFFSET eglDrawImage@DrawImage_24to24
				.ENDIF
			.ELSEIF	eax < 100H
				mov	[ebx].nTrans, eax
				.IF	(ecx == 32) && (edx == 32) && \
					(ERI_EnabledProcessorType & ERI_USE_XMM_P3)
					mov	eax, OFFSET eglDrawImage@DrawImage_RGB32_Trans_SSE
				.ELSE
					mov	eax, OFFSET eglDrawImage@DrawImage_24to24_Trans
				.ENDIF
			.ELSE
				mov	eax, OFFSET eglDrawImage@DrawImage_NoDraw
			.ENDIF
		.ELSEIF	!([ebx].dwFlags & EGL_DRAW_BLEND_ALPHA) || \
				!([ebx].dstimg.fdwFormatType & EIF_WITH_ALPHA)
			mov	eax, [esi].nTransparency
			.IF	eax == 0
				.IF	[ebx].dstimg.fdwFormatType & EIF_WITH_ALPHA
				.IF	(ecx == edx) && \
					(ERI_EnabledProcessorType & ERI_USE_XMM_P3)
					mov	eax, OFFSET eglDrawImage@DrawImage_24to24_SSE
				.ELSE
					mov	eax, OFFSET eglDrawImage@DrawImage_24to24
				.ENDIF
				.ELSE
				.IF	(edx == 32) && \
					(ERI_EnabledProcessorType & ERI_USE_XMM_P3)
					mov	eax, OFFSET eglDrawImage@DrawImage_RGBA_SSE
				.ELSE
					mov	eax, OFFSET eglDrawImage@DrawImage_RGBA
				.ENDIF
				.ENDIF
			.ELSEIF	eax < 100H
				mov	[ebx].nTrans, eax
				.IF	(edx == 32) && \
					(ERI_EnabledProcessorType & ERI_USE_XMM_P3)
					mov	eax, OFFSET eglDrawImage@DrawImage_RGBA_Trans_SSE
				.ELSE
					mov	eax, OFFSET eglDrawImage@DrawImage_RGBA_Trans
				.ENDIF
			.ELSE
				mov	eax, OFFSET eglDrawImage@DrawImage_NoDraw
			.ENDIF
		.ELSE
			mov	eax, [esi].nTransparency
			.IF	eax == 0
				.IF	ERI_EnabledProcessorType & ERI_USE_XMM_P3
					mov	eax, OFFSET eglDrawImage@DrawImage_RGBA_Blend_SSE
				.ELSE
					mov	eax, OFFSET eglDrawImage@DrawImage_RGBA_Blend
				.ENDIF
			.ELSEIF	eax < 100H
				mov	[ebx].nTrans, eax
				.IF	ERI_EnabledProcessorType & ERI_USE_XMM_P3
					mov	eax, OFFSET eglDrawImage@DrawImage_RGBA_BTrans_SSE
				.ELSE
					mov	eax, OFFSET eglDrawImage@DrawImage_RGBA_BTrans
				.ENDIF
			.ELSE
				mov	eax, OFFSET eglDrawImage@DrawImage_NoDraw
			.ENDIF
		.ENDIF
	.ELSEIF	(ecx == 16) && (edx == 16)
		.IF	ERI_EnabledProcessorType & ERI_USE_XMM_P3
			mov	eax, OFFSET eglDrawImage@DrawImage_24to24_SSE
		.ELSE
			mov	eax, OFFSET eglDrawImage@DrawImage_24to24
		.ENDIF
	.ELSE
		mov	eax, eslErrGeneral
		ret
	.ENDIF
	;
	mov	[ebx].pfnDrawImage, eax
	;
	xor	eax, eax
	ret
ENDIF
	;
	.ENDIF
	;
	; ------------------------------------------------------- 変形無し描画
	.ELSE
	;
	;	変形描画
	; --------------------------------------------------------------------
	mov	[ebx].pfnDrawImage, \
			OFFSET eglDrawImage@DrawImage_NoDraw
	mov	dwTemp[0], 10000H
	.IF	!(ERI_EnabledProcessorType & ERI_USE_XMM_P3)
		;
		; 画像変形行列計算
		;	    | a1  b1 |
		;	D = |        |
		;	    | a2  b2 |
		fld	egliax.xAxis.x
		fmul	egliax.yAxis.y
		fld	egliax.xAxis.y
		fmul	egliax.yAxis.x
		fsubp	st(1), st
		fstp	rRcpD
		mov	eax, rRcpD
		and	eax, 7FFFFFFFH
		.IF	eax < 37800000H		; eax < 2^-16
			xor	eax, eax
			ret
		.ENDIF
		fild	dwTemp[0]
		fdiv	rRcpD
		fstp	rRcpD
		;
		;	delta X-axis x =   b2 / D
		;	delta X-axis y = - a2/ D
		;
		fld	egliax.yAxis.y
		fmul	rRcpD
		fistp	[ebx].ptDltScanX.x
		fld	egliax.xAxis.y
		fchs
		fmul	rRcpD
		fistp	[ebx].ptDltScanX.y
		;
		;	delta Y-axis x = - b1 / D
		;	delta Y-axis y =   a1 / D
		;
		fld	egliax.yAxis.x
		fchs
		fmul	rRcpD
		fistp	[ebx].ptDltScanY.x
		fld	egliax.xAxis.x
		fmul	rRcpD
		fistp	[ebx].ptDltScanY.y
	.ELSE
		;
		; 画像変形行列計算
		;	    | a1  b1 |
		;	D = |        |
		;	    | a2  b2 |
		movss	xmm0, egliax.xAxis.x
		mulss	xmm0, egliax.yAxis.y
		movss	xmm1, egliax.xAxis.y
		mulss	xmm1, egliax.yAxis.x
		subss	xmm0, xmm1
		movss	rRcpD, xmm0
		mov	eax, rRcpD
		and	eax, 7FFFFFFFH
		.IF	eax < 37800000H		; eax < 2^-16
			xor	eax, eax
			ret
		.ENDIF
		cvtsi2ss	xmm1, dwTemp[0]
		divss	xmm1, xmm0
		movss	rRcpD, xmm1
		;
		;	delta X-axis x =   b2 / D
		;	delta X-axis y = - a2/ D
		;
		movss	xmm0, rConstMask80000000
		movss	xmm2, egliax.yAxis.y
		movss	xmm3, egliax.xAxis.y
		mulss	xmm2, xmm1
		xorps	xmm3, xmm0
		mulss	xmm3, xmm1
		cvtss2si	eax, xmm2
		cvtss2si	edx, xmm3
		mov	[ebx].ptDltScanX.x, eax
		mov	[ebx].ptDltScanX.y, edx
		;
		;	delta Y-axis x = - b1 / D
		;	delta Y-axis y =   a1 / D
		;
		movss	xmm2, egliax.yAxis.x
		movss	xmm3, egliax.xAxis.x
		xorps	xmm2, xmm0
		mulss	xmm2, xmm1
		mulss	xmm3, xmm1
		cvtss2si	eax, xmm2
		cvtss2si	edx, xmm3
		mov	[ebx].ptDltScanY.x, eax
		mov	[ebx].ptDltScanY.y, edx
	.ENDIF
	;
	; 出力先画像を取得する
	;
	mov	edi, [ebx].pDstImage
	mov	irectClip.x, 0
	mov	irectClip.y, 0
	ASSUME	edi:PEGL_IMAGE_INFO
	mov	ecx, [edi].dwImageWidth
	mov	edx, [edi].dwImageHeight
	mov	irectClip.w, ecx
	mov	irectClip.h, edx
	ASSUME	edi:NOTHING
	;
	INVOKE	eglGetClippedImageInfo ,
			ADDR [ebx].dstimg, edi, ADDR irectClip
	test	eax, eax
	.IF	!ZERO?
		mov	[ebx].pfnDrawImage, \
				OFFSET eglDrawImage@DrawImage_NoDraw
		xor	eax, eax
		ret
	.ENDIF
	;
	; Z バッファを取得する
	;
	mov	[ebx].zbuf.ptrImageArray, NULL
	mov	[ebx].zbuf.dwBytesPerLine, 0
	.IF	[ebx].dwFlags & EGL_WITH_Z_ORDER
		INVOKE	eglGetClippedImageInfo ,
			ADDR [ebx].zbuf, [ebx].pZBuffer, ADDR irectClip
	.ENDIF
	;
	; 入力画像を正規化する
	;
	mov	irectClip.x, 0
	mov	irectClip.y, 0
	mov	edi, [esi].pSrcImage
	.IF	edi == NULL
		mov	eax, eslErrGeneral
		ret
	.ENDIF
	ASSUME	edi:PEGL_IMAGE_INFO
	mov	ecx, [edi].dwImageWidth
	mov	edx, [edi].dwImageHeight
	mov	irectClip.w, ecx
	mov	irectClip.h, edx
	ASSUME	edi:NOTHING
	;
	mov	edi, [esi].pViewRect
	.IF	edi != NULL
		ASSUME	edi:PCEGL_RECT
		mov	ecx, [edi].left
		mov	edx, [edi].top
		.IF	((SDWORD PTR ecx) >= irectClip.w) || \
				((SDWORD PTR edx) >= irectClip.h)
			mov	[ebx].pfnDrawImage, \
					OFFSET eglDrawImage@DrawImage_NoDraw
			xor	eax, eax
			ret
		.ENDIF
		or	ecx, ecx
		.IF	!SIGN?
			mov	irectClip.x, ecx
		.ENDIF
		or	edx, edx
		.IF	!SIGN?
			mov	irectClip.y, edx
		.ENDIF
		;
		mov	ecx, [edi].right
		mov	edx, [edi].bottom
		.IF	((SDWORD PTR ecx) < irectClip.x) || \
				((SDWORD PTR edx) < irectClip.y)
			mov	[ebx].pfnDrawImage, \
					OFFSET eglDrawImage@DrawImage_NoDraw
			xor	eax, eax
			ret
		.ENDIF
		inc	ecx
		inc	edx
		.IF	(SDWORD PTR ecx) > irectClip.w
			mov	ecx, irectClip.w
		.ENDIF
		.IF	(SDWORD PTR edx) > irectClip.h
			mov	edx, irectClip.h
		.ENDIF
		sub	ecx, irectClip.x
		sub	edx, irectClip.y
		mov	irectClip.w, ecx
		mov	irectClip.h, edx
		ASSUME	edi:NOTHING
	.ENDIF
	;
	INVOKE	eglGetClippedImageInfo ,
			ADDR [ebx].srcimg, [esi].pSrcImage, ADDR irectClip
	;
	mov	edx, [esi].pSrcImage
	ASSUME	edx:PTR EGL_IMAGE_BUFF
	.IF	([edx].dwInfoSize == (SIZEOF EGL_IMAGE_BUFF)) \
			&& ([esi].pViewRect == NULL)
		mov	eax, [edx].pLineAddrEntry
		mov	[ebx].pSrcLineAddr, eax
		ASSUME	edx:NOTHING
	.ELSE
		.IF	[ebx].pSrcLineAddr[4] != NULL
			INVOKE	eslHeapFree , [ebx].hHeap, [ebx].pSrcLineAddr[4], 0
		.ENDIF
		mov	ecx, [ebx].srcimg.dwImageHeight
		shl	ecx, 2
		INVOKE	eslHeapAllocate , [ebx].hHeap, ecx, 0
		mov	[ebx].pSrcLineAddr, eax
		mov	[ebx].pSrcLineAddr[4], eax
		;
		mov	edx, [ebx].srcimg.ptrImageArray
		mov	ecx, [ebx].srcimg.dwImageHeight
		.REPEAT
			mov	DWORD PTR [eax], edx
			add	eax, 4
			add	edx, [ebx].srcimg.dwBytesPerLine
			dec	ecx
		.UNTIL	ZERO?
	.ENDIF
	;
	.IF	[esi].dwFlags & EGL_FIXED_POSITION
		mov	eax, [ebx].ptDrawOffset.x
		mov	edx, [ebx].ptDrawOffset.y
		shl	eax, 16
		shl	edx, 16
		add	eax, [esi].ptBasePos.x
		add	edx, [esi].ptBasePos.y
		mov	ptBasePos.x, eax
		mov	ptBasePos.y, edx
	.ELSE
		mov	eax, [ebx].ptDrawOffset.x
		mov	edx, [ebx].ptDrawOffset.y
		add	eax, [esi].ptBasePos.x
		add	edx, [esi].ptBasePos.y
		shl	eax, 16
		shl	edx, 16
		mov	ptBasePos.x, eax
		mov	ptBasePos.y, edx
	.ENDIF
	.IF	!([esi].dwFlags & EGL_POLYGON_SHAPED)
	;
	; --------- 回転矩形を取得する
	;
	.IF	!(ERI_EnabledProcessorType & ERI_USE_XMM_P3)
		fld	egliax.xAxis.x		; x 軸正規化
		fmul	st(0), st
		fld	egliax.xAxis.y
		fmul	st(0), st
		faddp	st(1), st
		fsqrt
		fld	egliax.xAxis.x
		fdiv	st, st(1)
		fstp	egliax.xAxis.x
		fld	egliax.xAxis.y
		fdiv	st, st(1)
		fstp	egliax.xAxis.y
		fild	irectClip.w
		fmulp	st(1), st
;		fld1
;		fsubp	st(1), st
		fstp	xLength
		;
		fld	egliax.yAxis.x		; y 軸正規化
		fmul	st(0), st
		fld	egliax.yAxis.y
		fmul	st(0), st
		faddp	st(1), st
		fsqrt
		fld	egliax.yAxis.x
		fdiv	st, st(1)
		fstp	egliax.yAxis.x
		fld	egliax.yAxis.y
		fdiv	st, st(1)
		fstp	egliax.yAxis.y
		fild	irectClip.h
		fmulp	st(1), st
;		fld1
;		fsubp	st(1), st
		fstp	yLength
		;
		fild	ptBasePos.y		; 頂点座標計算
		fild	ptBasePos.x
		fld	rConstRealDiv65536
		fmul	st[2], st
		fmulp	st[1], st
		fstp	vRevRect[0].x
		fstp	vRevRect[0].y
		;
		fld	egliax.xAxis.y
		fld	egliax.xAxis.x
		fld	xLength
		fmul	st(2), st
		fmulp	st(1), st
		fld	vRevRect[0].y
		fld	vRevRect[0].x
		faddp	st(2), st
		faddp	st(2), st
		fstp	vRevRect[8].x
		fstp	vRevRect[8].y
		;
		fld	egliax.yAxis.y
		fld	egliax.yAxis.x
		fld	yLength
		fmul	st(2), st
		fmulp	st(1), st
		fld	vRevRect[8].y
		fld	vRevRect[8].x
		fadd	st, st(2)
		fstp	vRevRect[16].x
		fadd	st, st(2)
		fstp	vRevRect[16].y
		;
		fadd	vRevRect[0].x
		fstp	vRevRect[24].x
		fadd	vRevRect[0].y
		fstp	vRevRect[24].y
	.ELSE
		movups	xmm0, egliax		; x, y 軸正規化
;		mov	eax, 1
		movaps	xmm6, xmm0
		mulps	xmm0, xmm0
;		cvtsi2ss	xmm7, eax
;		shufps	xmm7, xmm7, 0
		movaps	xmm1, xmm0
		shufps	xmm0, xmm0, 10110001B
		addps	xmm0, xmm1
		sqrtps	xmm0, xmm0
		divps	xmm6, xmm0		; xmm6 = x, y 正規化軸
		cvtpi2ps	xmm5, QWORD PTR irectClip.w
		shufps	xmm5, xmm5, 01010000B
		mulps	xmm5, xmm0
;		subps	xmm5, xmm7		; xmm5 = x, y 軸長さ
		;
		mulps	xmm5, xmm6
		;
		cvtpi2ps	xmm0, QWORD PTR ptBasePos
		movss	xmm7, rConstRealDiv65536
		shufps	xmm7, xmm7, 0
		mulps	xmm0, xmm7
		movlps	QWORD PTR vRevRect[0], xmm0
		;
		shufps	xmm0, xmm0, 01000100B
		addps	xmm0, xmm5
		movlps	QWORD PTR vRevRect[8], xmm0
		movhps	QWORD PTR vRevRect[24], xmm0
		;
		shufps	xmm5, xmm5, 01001110B
		addps	xmm0, xmm5
		movlps	QWORD PTR vRevRect[16], xmm0
	.ENDIF
	;
	; ポリゴンリージョンを作成
	;
	INVOKE	eglNormalizePolygonRegion ,
		[ebx].pRegion, ADDR [ebx].rectClip,
			4, ADDR vRevRect[0], NULL, NULL
	;
	.ELSE
	;
	; --------- 出力先矩形を作成
	;
	mov	eax, [ebx].ptDrawOffset.x
	mov	edx, [ebx].ptDrawOffset.y
	or	eax, edx
	.IF	ZERO?
		INVOKE	eglNormalizePolygonRegion ,
			[ebx].pRegion, ADDR [ebx].rectClip,
			[esi].nVertexCount, [esi].pVertexPos, NULL, NULL
	.ELSE
		mov	dwSaveESP, esp
		mov	ecx, [esi].nVertexCount
		lea	eax, [ecx * (SIZEOF E3D_VECTOR_2D)]
		sub	esp, eax
		mov	edi, [esi].pVertexPos
		mov	eax, esp
		mov	dwTemp[0], eax
		ASSUME	edi:PTR E3D_VECTOR_2D
		ASSUME	eax:PTR E3D_VECTOR_2D
		;
		.IF	!(ERI_EnabledProcessorType & ERI_USE_XMM_P3)
			.REPEAT
				fild	[ebx].ptDrawOffset.x
				fadd	[edi].x
				fstp	[eax].x
				fild	[ebx].ptDrawOffset.y
				fadd	[edi].y
				fstp	[eax].y
				add	edi, (SIZEOF E3D_VECTOR_2D)
				add	eax, (SIZEOF E3D_VECTOR_2D)
				dec	ecx
			.UNTIL	ZERO?
		.ELSE
			cvtpi2ps	xmm7, QWORD PTR [ebx].ptDrawOffset
			shufps		xmm7, xmm7, 01000100B
			sub	ecx, 2
			.WHILE	!SIGN?
				movups	xmm0, XMMWORD_PTR [edi]
				addps	xmm0, xmm7
				add	edi, (SIZEOF E3D_VECTOR_2D) * 2
				movups	XMMWORD_PTR [eax], xmm0
				add	eax, (SIZEOF E3D_VECTOR_2D) * 2
				sub	ecx, 2
			.ENDW
			add	ecx, 2
			.IF	!ZERO?
				movlps	xmm0, [edi]
				addps	xmm0, xmm7
				movlps	[eax], xmm0
			.ENDIF
		.ENDIF
		;
		ASSUME	edi:NOTHING
		ASSUME	eax:NOTHING
		;
		mov	ecx, dwTemp[0]
		INVOKE	eglNormalizePolygonRegion ,
			[ebx].pRegion, ADDR [ebx].rectClip,
			[esi].nVertexCount, ecx, NULL, NULL
		;
		mov	esp, dwSaveESP
	.ENDIF
	;
	.ENDIF
	;
	mov	edi, eax
	or	edi, edi
	.IF	ZERO?
		mov	[ebx].pfnDrawImage, \
				OFFSET eglDrawImage@DrawImage_NoDraw
		xor	eax, eax
		ret
	.ENDIF
	;
	; ベース座標の計算
	;
	mov	edi, [ebx].pRegion
	ASSUME	edi:PE3D_POLYGON_REGION
	mov	edi, [edi].nTopLine
	ASSUME	edi:NOTHING
	;
	shl	edi, 16
	sub	edi, ptBasePos.y
	mov	eax, [ebx].ptDltScanY.x
	imul	edi
	shld	edx, eax, 16
	mov	[ebx].ptBasePos.x, edx
	mov	eax, [ebx].ptDltScanY.y
	imul	edi
	shld	edx, eax, 16
	mov	[ebx].ptBasePos.y, edx
	;
	mov	edi, ptBasePos.x
	mov	eax, [ebx].ptDltScanX.x
	imul	edi
	shld	edx, eax, 16
	sub	[ebx].ptBasePos.x, edx
	mov	eax, [ebx].ptDltScanX.y
	imul	edi
	shld	edx, eax, 16
	sub	[ebx].ptBasePos.y, edx

IF	0
	;
	; 各描画組み合わせを判定
	;
	mov	ecx, [ebx].srcimg.dwBitsPerPixel
	mov	edx, [ebx].dstimg.dwBitsPerPixel
	.IF	(ecx != 32) || (edx != 32)
		mov	eax, eslErrGeneral
		ret
	.ENDIF
	;
	mov	eax, [esi].nTransparency
	mov	edx, [esi].rZOrder
	mov	[ebx].nTrans, eax
	mov	[ebx].rZOrder, edx
	;
	.IF	!(ERI_EnabledProcessorType & ERI_USE_XMM_P3)
	.IF	!([ebx].srcimg.fdwFormatType & EIF_WITH_ALPHA)
		.IF	!([ebx].dwFlags & EGL_WITH_Z_ORDER)
			.IF	eax == 0
				mov	eax, OFFSET eglDrawImage@DrawImage_RGB_X
			.ELSEIF	eax < 100H
				mov	eax, OFFSET eglDrawImage@DrawImage_RGB_XTrans
			.ELSE
				mov	eax, OFFSET eglDrawImage@DrawImage_NoDraw
			.ENDIF
		.ELSEIF	eax < 100H
			mov	eax, eglDrawImage@DrawImage_RGB_XTZBuf
		.ELSE
			mov	eax, OFFSET eglDrawImage@DrawImage_NoDraw
		.ENDIF
	.ELSE
		.IF	!([ebx].dwFlags & EGL_WITH_Z_ORDER) && (eax == 0)
			mov	eax, OFFSET eglDrawImage@DrawImage_RGBA_X
		.ELSEIF	eax < 100H
			mov	eax, OFFSET eglDrawImage@DrawImage_RGBA_XTZBuf
		.ELSE
			mov	eax, OFFSET eglDrawImage@DrawImage_NoDraw
		.ENDIF
	.ENDIF
	.ELSE
	.IF	!([ebx].srcimg.fdwFormatType & EIF_WITH_ALPHA)
		.IF	!([ebx].dwFlags & EGL_WITH_Z_ORDER)
			.IF	eax == 0
				mov	eax, OFFSET eglDrawImage@DrawImage_RGB_X_SSE
			.ELSEIF	eax < 100H
				mov	eax, OFFSET eglDrawImage@DrawImage_RGB_XTrans_SSE
			.ELSE
				mov	eax, OFFSET eglDrawImage@DrawImage_NoDraw
			.ENDIF
		.ELSE
			.IF	eax == 0
				mov	eax, OFFSET eglDrawImage@DrawImage_RGB_XZBuf_SSE
			.ELSEIF	eax < 100H
				mov	eax, OFFSET eglDrawImage@DrawImage_RGB_XTZBuf_SSE
			.ELSE
				mov	eax, OFFSET eglDrawImage@DrawImage_NoDraw
			.ENDIF
		.ENDIF
	.ELSE
		.IF	!([ebx].dwFlags & EGL_WITH_Z_ORDER)
			.IF	eax == 0
				mov	eax, OFFSET eglDrawImage@DrawImage_RGBA_X_SSE
			.ELSEIF	eax < 100H
				mov	eax, OFFSET eglDrawImage@DrawImage_RGBA_XTrans_SSE
			.ELSE
				mov	eax, OFFSET eglDrawImage@DrawImage_NoDraw
			.ENDIF
		.ELSE
			.IF	eax == 0
				mov	eax, OFFSET eglDrawImage@DrawImage_RGBA_XZBuf_SSE
			.ELSEIF	eax < 100H
				mov	eax, OFFSET eglDrawImage@DrawImage_RGBA_XTZBuf_SSE
			.ELSE
				mov	eax, OFFSET eglDrawImage@DrawImage_NoDraw
			.ENDIF
		.ENDIF
	.ENDIF
	.ENDIF
	;
	mov	[ebx].pfnDrawImage, eax
ENDIF
	.ENDIF
	; ----------------------------------------------------------- 変形描画
	;
	;	前段処理・特殊条件判定終了
	; --------------------------------------------------------------------

	mov	eax, [esi].nTransparency
	mov	edx, [esi].rZOrder
	mov	[ebx].nTrans, eax
	mov	[ebx].rZOrder, edx
	;
	.IF	[ebx].dwFlags & EGL_DRAW_GLOW_LIGHT
	;
	;	グレイスケール描画判定
	; --------------------------------------------------------------------
	mov	ecx, [ebx].srcimg.dwBitsPerPixel
;	mov	edx, [ebx].dstimg.dwBitsPerPixel
	.IF	(ecx != 8) ;|| ((edx != 24) && (edx != 32))
		mov	eax, eslErrGeneral
		ret
	.ENDIF
	;
	; パレットテーブル初期化
	;
	.IF	ERI_EnabledProcessorType & ERI_USE_XMM_P3
		mov	eax, [esi].rgbLightColor.dwPixelCode
		mov	edx, [esi].rgbDimColor.dwPixelCode
		pxor	mm7, mm7
		or	eax, 0FF000000H
		and	edx,  00FFFFFFH
		movd	mm0, eax
		movd	mm1, edx
		punpcklbw	mm0, mm7
		punpcklbw	mm1, mm7
		psubw	mm0, mm1	; mm0 = 色差分
		psllw	mm1, 8		; mm1 = 色値
		paddw	mm1, mm0
		movq	mm2, mm1
		;
		mov	edx, [esi].nTransparency
		mov	eax, 100H
		.IF	edx >= 100H
			mov	edx, 100H
		.ENDIF
		mov	[ebx].nTrans, edx
		sub	eax, edx
		;
		pxor	mm4, mm4	; mm4 = αチャネル
		movd	mm5, eax	; mm5 = αチャネル差分
		pshufw	mm5, mm5, 0
		xor	ecx, ecx
		;
		.REPEAT
			pextrw	eax, mm4, 0
				psrlw	mm2, 8
			paddw	mm1, mm0
				pmulhuw	mm2, mm4
			shr	eax, 8
				paddw	mm4, mm5
			pinsrw	mm2, eax, 3
			packuswb	mm2, mm7
			movd	[ebx].rgbaColor[ecx*4].dwPixelCode, mm2
			inc	ecx
			movq	mm2, mm1
		.UNTIL	ecx >= 100H
		;
		emms
	.ELSE
		FOR	@MEMBER, <Blue, Green, Red>
			movzx	eax, [esi].rgbLightColor.rgb.@MEMBER
			movzx	edx, [esi].rgbDimColor.rgb.@MEMBER
			sub	eax, edx
			shl	edx, 8
			xor	ecx, ecx
			.REPEAT
				mov	[ebx].rgbaColor[ecx*4].rgba.@MEMBER, dh
				inc	ecx
				add	edx, eax
			.UNTIL	ecx >= 100H
		ENDM
		;
		mov	edx, [esi].nTransparency
		mov	eax, 100H
		.IF	edx >= 100H
			mov	edx, 100H
		.ENDIF
		mov	[ebx].nTrans, edx
		;
		sub	eax, edx
		xor	edx, edx
		xor	edi, edi
		.REPEAT
			mov	[ebx].rgbaColor[edi*4].rgba.Alpha, dh
			FOR	@MEMBER, <Blue, Green, Red>
				movzx	ecx, [ebx].rgbaColor[edi*4].rgba.@MEMBER
				imul	ecx, edx
				shr	ecx, 16
				adc	ecx, 0
				mov	[ebx].rgbaColor[edi*4].rgba.@MEMBER, cl
			ENDM
			inc	edi
			add	edx, eax
		.UNTIL	edi >= 100H
	.ENDIF
	;
	; 描画関数判定
	;
	mov	edx, [ebx].dstimg.dwBitsPerPixel
	.IF	!([ebx].dwFlags & EGL_WITH_AXES) && ((edx == 24) || (edx == 32))
	.IF	(!([ebx].dwFlags & EGL_DRAW_BLEND_ALPHA) || \
			([ebx].dstimg.dwBitsPerPixel != 32)) && \
			!([ebx].dstimg.fdwFormatType & EIF_WITH_ALPHA)
		.IF	[ebx].dwFlags & EGL_DRAW_BLEND_ALPHA
			.IF	([ebx].dstimg.dwBitsPerPixel == 32) && \
					(ERI_EnabledProcessorType & ERI_USE_XMM_P3)
				mov	[ebx].pfnDrawImage, \
					OFFSET eglDrawImage@DrawImage_Glow_SSE
			.ELSE
				mov	[ebx].pfnDrawImage, \
					OFFSET eglDrawImage@DrawImage_Glow
			.ENDIF
		.ELSE
			.IF	([ebx].dstimg.dwBitsPerPixel == 32) && \
					(ERI_EnabledProcessorType & ERI_USE_XMM_P3)
				mov	[ebx].pfnDrawImage, \
					OFFSET eglDrawImage@DrawImage_GlowAdd_SSE
			.ELSE
				mov	[ebx].pfnDrawImage, \
					OFFSET eglDrawImage@DrawImage_GlowAdd
			.ENDIF
		.ENDIF
		xor	eax, eax
		ret
	.ELSEIF	!(ERI_EnabledProcessorType & ERI_USE_XMM_P3)
		mov	[ebx].pfnDrawImage, \
				OFFSET eglDrawImage@DrawImage_GlowBlend
		xor	eax, eax
		ret
	.ENDIF
	;
	.ENDIF
	;
	.IF	!([ebx].dwFlags & EGL_DRAW_BLEND_ALPHA)
		mov	ecx, 100H/2
		xor	esi, esi
		.REPEAT
			mov	eax, DWORD PTR [ebx].rgbaColor[esi]
			mov	edx, DWORD PTR [ebx].rgbaColor[esi+4]
			and	eax, 00FFFFFFH
			and	edx, 00FFFFFFH
			mov	DWORD PTR [ebx].rgbaColor[esi], eax
			mov	DWORD PTR [ebx].rgbaColor[esi+4], edx
			add	esi, 8
			dec	ecx
		.UNTIL	ZERO?
		or	[ebx].dwFlags, EGL_DRAW_BLEND_ALPHA
	.ENDIF
	mov	[ebx].nTrans, 0
	;
	; --------------------------------------------- グレイスケール特殊描画
	.ELSE
		;
		; 描画フラグの正規化
		;
		mov	eax, [ebx].dwFlags
		.IF	eax & EGL_DRAW_BLEND_ALPHA
			.IF	!([ebx].srcimg.fdwFormatType & EIF_WITH_ALPHA) \
					&& !([ebx].dwFlags & EGL_WITH_AXES)
				and	eax, NOT EGL_DRAW_BLEND_ALPHA
				mov	[ebx].dwFlags, eax
			.ENDIF
		.ELSEIF	[ebx].srcimg.fdwFormatType & EIF_WITH_ALPHA
			.IF	!([ebx].dstimg.fdwFormatType & EIF_WITH_ALPHA)
				or	eax, EGL_DRAW_BLEND_ALPHA
				mov	[ebx].dwFlags, eax
			.ENDIF
;		.ELSEIF	[ebx].dwFlags & EGL_WITH_AXES
;			or	eax, EGL_DRAW_BLEND_ALPHA
;			mov	[ebx].dwFlags, eax
		.ENDIF
	.ENDIF
	;
	mov	ecx, [ebx].srcimg.dwBitsPerPixel
	mov	edx, [ebx].dstimg.dwBitsPerPixel
	;
	.IF	(edx != 8) && (edx != 24) && (edx != 32)
		.IF	[ebx].dwFlags & EGL_WITH_AXES
			mov	eax, eslErrGeneral
			ret
		.ENDIF
		mov	[ebx].pfnDrawImage, \
				OFFSET eglDrawImage@DrawImage_ConvertFormat
		xor	eax, eax
		ret
	.ENDIF
	;
	; サンプリング関数判別
	;
	.IF	[ebx].dwFlags & EGL_WITH_AXES
		mov	[ebx].pfnDrawImage, OFFSET eglDrawImage@DrawImage_Transform
		;
		mov	eax, [ebx].srcimg.dwImageWidth
		mov	edx, [ebx].srcimg.dwImageHeight
		dec	eax
		dec	edx
		xor	eax, 7FFFH
		xor	edx, 7FFFH
		mov	[ebx].tmxSizeMask[0], ax
		mov	[ebx].tmxSizeMask[2], dx
		mov	[ebx].tmxSizeMask[4], ax
		mov	[ebx].tmxSizeMask[6], dx
		mov	eax, [ebx].srcimg.dwBitsPerPixel
		mov	edx, [ebx].srcimg.dwBytesPerLine
		shr	eax, 3
		shl	edx, 16
		or	eax, edx
		mov	DWORD PTR [ebx].tmxMulAddr[0], eax
		mov	DWORD PTR [ebx].tmxMulAddr[4], eax
		mov	eax, [ebx].srcimg.ptrImageArray
		mov	[ebx].tmxSrcImageAddr[0], eax
		mov	[ebx].tmxSrcImageAddr[4], eax
		;
		.IF	ecx == 32
			.IF	ERI_EnabledProcessorType & ERI_USE_MMX_PENTIUM
				mov	eax, [ebx].dwFlags
				.IF	(eax & EGL_SMOOTH_STRETCH) && !(eax & EGL_UNSMOOTH_STRETCH)
;					.IF	ERI_EnabledProcessorType & ERI_USE_SSE2
;						mov	eax, OFFSET eglDrawImage@Sample_RGB32_XS_SSE2
;					.ELSEIF	ERI_EnabledProcessorType & ERI_USE_XMM_P3
					.IF	ERI_EnabledProcessorType & ERI_USE_XMM_P3
						mov	eax, OFFSET eglDrawImage@Sample_RGB32_XS_SSE
					.ELSE
						mov	eax, OFFSET eglDrawImage@Sample_RGB32_XS_MMX
					.ENDIF
				.ELSE
					mov	eax, OFFSET eglDrawImage@Sample_RGB32_X_MMX
				.ENDIF
			.ELSE
				mov	eax, OFFSET eglDrawImage@Sample_RGB32_X
			.ENDIF
		.ELSEIF	ecx == 8
			.IF	[ebx].dwFlags & EGL_DRAW_GLOW_LIGHT
				mov	eax, OFFSET eglDrawImage@Sample_Gray8_X
			.ELSE
				mov	eax, OFFSET eglDrawImage@Sample_Index8_X
			.ENDIF
		.ELSEIF	ecx == 24
			mov	eax, OFFSET eglDrawImage@Sample_RGB24_X
		.ELSEIF	ecx == 16
			mov	eax, OFFSET eglDrawImage@Sample_RGB16_X
		.ELSE
			mov	eax, eslErrGeneral
			ret
		.ENDIF
	.ELSE
		mov	[ebx].pfnDrawImage, OFFSET eglDrawImage@DrawImage_Normal
		;
		.IF	ecx == 32
			.IF	([ebx].srcimg.fdwFormatType & EIF_WITH_ALPHA) \
					&& !([ebx].dwFlags & 0FFFF0000H)
				mov	[ebx].pfnDrawImage, \
					OFFSET eglDrawImage@DrawImage_DirectRGBA32
			.ENDIF
			mov	eax, OFFSET eglDrawImage@Sample_RGB32
		.ELSEIF	ecx == 8
			.IF	[ebx].dwFlags & EGL_DRAW_GLOW_LIGHT
				mov	eax, OFFSET eglDrawImage@Sample_Gray8
			.ELSE
				mov	eax, OFFSET eglDrawImage@Sample_Index8
			.ENDIF
		.ELSEIF	ecx == 24
			mov	eax, OFFSET eglDrawImage@Sample_RGB24
		.ELSEIF	ecx == 16
			mov	eax, OFFSET eglDrawImage@Sample_RGB16
		.ELSE
			mov	eax, eslErrGeneral
			ret
		.ENDIF
	.ENDIF
	;
	mov	[ebx].pfnDrawSampling, eax
	;
	; 適用関数
	;
	mov	esi, pDrawParam
	ASSUME	esi:PTR EGL_DRAW_PARAM
	mov	edi, [ebx].dwFlags
	mov	eax, [esi].rgbColorParam1.dwPixelCode
	mov	[ebx].colorDraw.dwPixelCode, eax
	mov	eax, OFFSET eglDrawImage@Apply_nothing
	mov	edx, [ebx].dstimg.dwBitsPerPixel
	;
	and	edi, 00FF0000H
	.IF	!ZERO?
		.IF	edi == EGL_APPLY_C_MASK
			or	[ebx].dwFlags, EGL_DRAW_BLEND_ALPHA
		.ENDIF
		and	edi, 007F0000H
		shr	edi, 16
		.IF	(DWORD PTR edi) < 16
			.IF	ERI_EnabledProcessorType & ERI_USE_MMX_PENTIUM
				mov	eax, pfnDrawApplyTable32MMX[edi * 4]
			.ELSE
				mov	eax, pfnDrawApplyTable32[edi * 4]
			.ENDIF
		.ENDIF
	.ENDIF
	;
	mov	[ebx].pfnDrawApplying, eax
	;
	; レンダリング関数判別
	;
	mov	edi, [ebx].dwFlags
	mov	ecx, [ebx].nTrans
	and	edi, 0FF000000H
	.IF	ZERO?	; ------------------------------------ 通常描画
	.IF	eax != (OFFSET eglDrawImage@Apply_nothing)
		neg	ecx
		INVOKE	eglCalculateToneTable ,
				ADDR [ebx].nBlueTone, ecx, 0
		xor	ecx, ecx
	.ENDIF
	.IF	(edx == 32) && (ERI_EnabledProcessorType & ERI_USE_XMM_P3)
		;
		; SSE アクセラレーション
		;
		mov	eax, [ebx].dwFlags
		.IF	eax & EGL_WITH_Z_ORDER
			.IF	ecx == 0
				mov	eax, OFFSET eglDrawImage@Render_RGBA32_BZ_SSE
			.ELSE
				mov	eax, OFFSET eglDrawImage@Render_RGBA32_BTZ_SSE
			.ENDIF
		.ELSEIF	ecx > 0
			mov	eax, OFFSET eglDrawImage@Render_RGBA32_BT_SSE
		.ELSEIF	eax & EGL_DRAW_BLEND_ALPHA
			mov	eax, OFFSET eglDrawImage@Render_RGBA32_B_SSE
		.ELSE
			mov	eax, OFFSET eglDrawImage@Render_RGB32_SSE
		.ENDIF
	.ELSE
		.IF	(ecx > 0) || ([ebx].dwFlags & EGL_WITH_Z_ORDER)
			;
			; 透明度処理用テーブル生成
			;
			push	ecx
			neg	ecx
			INVOKE	eglCalculateToneTable ,
					ADDR [ebx].nBlueTone, ecx, 0
			;
			mov	ecx, [esp]
			sub	ecx, 100H
			INVOKE	eglCalculateToneTable ,
					ADDR [ebx].nGreenTone, ecx, 0
			pop	ecx
		.ENDIF
		;
		.IF	[ebx].dstimg.dwBitsPerPixel == 32
			mov	eax, [ebx].dwFlags
			.IF	eax & EGL_WITH_Z_ORDER
				mov	eax, OFFSET eglDrawImage@Render_RGBA32_BTZ
			.ELSEIF	ecx > 0
				mov	eax, OFFSET eglDrawImage@Render_RGBA32_BT
			.ELSEIF	eax & EGL_DRAW_BLEND_ALPHA
				.IF	ERI_EnabledProcessorType & ERI_USE_MMX_PENTIUM
					mov	eax, OFFSET eglDrawImage@Render_RGBA32_B_MMX
				.ELSE
					mov	eax, OFFSET eglDrawImage@Render_RGBA32_B
				.ENDIF
			.ELSE
				mov	eax, OFFSET eglDrawImage@Render_RGB32
			.ENDIF
		.ELSEIF	[ebx].dstimg.dwBitsPerPixel == 24
			mov	eax, [ebx].dwFlags
			.IF	eax & EGL_WITH_Z_ORDER
				mov	eax, OFFSET eglDrawImage@Render_RGB24_BTZ
			.ELSEIF	ecx > 0
				mov	eax, OFFSET eglDrawImage@Render_RGB24_BT
			.ELSEIF	eax & EGL_DRAW_BLEND_ALPHA
				mov	eax, OFFSET eglDrawImage@Render_RGB24_B
			.ELSE
				mov	eax, OFFSET eglDrawImage@Render_RGB24
			.ENDIF
		.ELSEIF	[ebx].dstimg.dwBitsPerPixel == 8
			mov	eax, OFFSET eglDrawImage@Render_Gray8
		.ENDIF
	.ENDIF
	.ELSE			; --------------------------------- 特殊描画
	.IF	[ebx].nTrans > 0
		mov	eax, [ebx].pfnDrawApplying
		.IF	ERI_EnabledProcessorType & ERI_USE_MMX_PENTIUM
			.IF	eax == (OFFSET eglDrawImage@Apply_nothing)
				mov	eax, OFFSET eglDrawImage@Apply_Transparency_MMX
			.ENDIF
		.ELSE
			mov	edx, [ebx].nTrans
			neg	edx
			INVOKE	eglCalculateToneTable ,
					ADDR [ebx].nBlueTone, edx, 0
			mov	edx, [ebx].nTrans
			sub	edx, 100H
			INVOKE	eglCalculateToneTable ,
					ADDR [ebx].nGreenTone, edx, 0
			;
			mov	eax, [ebx].pfnDrawApplying
			.IF	eax == (OFFSET eglDrawImage@Apply_nothing)
				mov	eax, OFFSET eglDrawImage@Apply_Transparency
			.ENDIF
		.ENDIF
		mov	[ebx].pfnDrawApplying, eax
	.ENDIF
	and	edi, 7F000000H
	shr	edi, 24
	.IF	edi < 16
		mov	edx, [ebx].dstimg.dwBitsPerPixel
		.IF	edi == 2 ; ((EGL_DRAW_F_MUL AND 7F000000H) SHR 24)
			.IF	[ebx].pfnDrawApplying == \
					(OFFSET eglDrawImage@Apply_Transparency_MMX)
				mov	[ebx].pfnDrawApplying, \
					OFFSET eglDrawImage@Apply_ITransparency_MMX
			.ELSEIF	[ebx].pfnDrawApplying == \
					(OFFSET eglDrawImage@Apply_Transparency)
				mov	[ebx].pfnDrawApplying, \
					OFFSET eglDrawImage@Apply_ITransparency
			.ENDIF
		.ENDIF
		.IF	edx == 32
			.IF	ERI_EnabledProcessorType & ERI_USE_MMX_PENTIUM
				mov	eax, pfnDrawFuncTable32MMX[edi * 4]
			.ELSE
				.IF	edi == 8 ; ((EGL_DRAW_A_MOVE AND 7F000000H) SHR 24)
					mov	edx, [ebx].nTrans
					neg	edx
					INVOKE	eglCalculateToneTable ,
							ADDR [ebx].nBlueTone, edx, 0
					mov	edx, [ebx].nTrans
					sub	edx, 100H
					INVOKE	eglCalculateToneTable ,
							ADDR [ebx].nGreenTone, edx, 0
					mov	[ebx].pfnDrawApplying, \
						OFFSET eglDrawImage@Apply_Transparency
				.ENDIF
				mov	eax, pfnDrawFuncTable32[edi * 4]
			.ENDIF
		.ELSEIF	edx == 8
			mov	eax, pfnDrawFuncTable8[edi * 4]
		.ELSE
			mov	eax, eslErrGeneral
			ret
		.ENDIF
		.IF	eax == 0
			mov	eax, eslErrGeneral
			ret
		.ENDIF
	.ELSE
		mov	eax, eslErrGeneral
		ret
	.ENDIF
	.ENDIF
	;
	mov	[ebx].pfnDrawRendering, eax

	ASSUME	ebx:NOTHING
	ASSUME	esi:NOTHING
	xor	eax, eax
	ret

eglDrawImage@PrepareDraw		ENDP


CodeSeg	ENDS

	END
