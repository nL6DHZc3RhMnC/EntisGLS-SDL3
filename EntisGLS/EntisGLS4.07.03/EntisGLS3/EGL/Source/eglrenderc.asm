
; ----------------------------------------------------------------------------
;                          Entis Graphic Library
; ----------------------------------------------------------------------------
;    Copyright (c) 2002-2004 Leshade Entis, Entis-soft. Al rights reserved.
; ----------------------------------------------------------------------------


	.686
	.XMM
	.MODEL	FLAT

	INCLUDE	experi.inc
	INCLUDE	egl.inc

LEFT_ERROR_RANGE	EQU	100H	; ライン左端・少数誤差幅（/10000H）

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
;	テクスチャなしシェーディング関数 486 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_LineNTX486	PROC	NEAR32 C

	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	mov	edi, [ebx].pLineBuf[0]
	ASSUME	edi:PTR E3D_TRANS_LINE_BUF
	;
	.IF	[ebx].nLineLeft[0] & 01H
		@INDEX = 0
		REPEAT	4
			mov	ax, [ebx].rgbNextColor[@INDEX*2]
			mov	dx, [ebx].rgbNextColor[@INDEX*2+8]
			sub	ax, [ebx].rgbDeltaColor[@INDEX*2]
			sub	dx, [ebx].rgbDeltaColor[@INDEX*2+8]
			mov	[ebx].rgbNextColor[@INDEX*2], ax
			mov	[ebx].rgbNextColor[@INDEX*2+8], dx
			@INDEX = @INDEX + 1
		ENDM
	.ENDIF
	;
	mov	ecx, [ebx].nLineRight[4]
	mov	edx, [ebx].nLineLeft[4]
	fld	[ebx].vTxxy.x
	and	ecx, NOT 01H
	and	edx, NOT 01H
	sub	ecx, edx
	fild	[ebx].nLineLeft[4]
	fmul	st, st(1)
	fadd	[ebx].rTxLineMod
	shr	ecx, 1
	ASSERT	<!!CARRY?>, "!(ecx & 01H)"
	inc	ecx
	.REPEAT
		@INDEX = 0
		REPEAT	2
			fld	[ebx].rTxoxyr
			fdiv	st, st(1)
			;
			@INDEX2 = 0
			REPEAT	3
				mov	ax, [ebx].rgbNextColor[@INDEX2*2]
				mov	dx, [ebx].rgbNextColor[@INDEX2*2+8]
				add	ax, [ebx].rgbDeltaColor[@INDEX2*2]
				add	dx, [ebx].rgbDeltaColor[@INDEX2*2+8]
				mov	[ebx].rgbNextColor[@INDEX2*2], ax
				mov	[ebx].rgbNextColor[@INDEX2*2+8], dx
				sar	ax, 7
				sar	dx, 7
				.IF	ax >= 100H
					sar	ax, 15
					not	ax
				.ENDIF
				.IF	dx >= 100H
					sar	dx, 15
					not	dx
				.ENDIF
				mov	[edi].rgbMul[@INDEX*4].rgb.Blue[@INDEX2], al
				mov	[edi].rgbAdd[@INDEX*4].rgb.Blue[@INDEX2], dl
				@INDEX2 = @INDEX2 + 1
			ENDM
			;
			fstp	[edi].rZValue[@INDEX*4]
			fadd	st, st(1)
			;
			@INDEX = @INDEX + 1
		ENDM
		;
		add	edi, (SIZEOF E3D_TRANS_LINE_BUF)
		dec	ecx
	.UNTIL	ZERO?
	fstp	st(0)
	fstp	st(0)
	;
	ASSUME	edi:NOTHING
	ASSUME	ebx:NOTHING
	ret

eglRenderPoly@RenderPolygon_LineNTX486	ENDP

;
;	テクスチャ（RGB）ありシェーディング関数 486 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_LineTX486	PROC	NEAR32 C

	call	eglRenderPoly@RenderPolygon_LineNTX486
	;
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	mov	edi, [ebx].pLineBuf[0]
	ASSUME	edi:PTR E3D_TRANS_LINE_BUF
	;
	fld	[ebx].vTxxy.x
	fld	[ebx].vTxDeltaX.y
	fld	[ebx].vTxDeltaX.x
	mov	ecx, [ebx].nLineRight[4]
	;
	fld	[ebx].rTxLineMod
	fld	[ebx].vTxLinePos.y
	fld	[ebx].vTxLinePos.x
	sub	ecx, [ebx].nLineLeft[4]
	;
	fild	[ebx].nLineLeft[4]
	fmul	st, st(4)
	faddp	st(1), st
	fild	[ebx].nLineLeft[4]
	fmul	st, st(5)
	faddp	st(2), st
	fild	[ebx].nLineLeft[4]
	fmul	st, st(6)
	faddp	st(3), st
	;
	fld	[ebx].rRcpTxxy
	fmul	st(5), st
	fmul	st(4), st
	fmul	st(2), st
	fmulp	st(1), st
	;
	shr	ecx, 1
	ASSERT	<!!CARRY?>, "!(ecx & 01H)"
	inc	ecx
	.REPEAT
		mov	[ebx].dib.nLeftWidth, ecx
		;
;		fld	[edi].rZValue[4]
;		fld	[ebx].vScreenPos.z
;		fmul	st(1), st
;		fmul	[edi].rZValue[0]
;		fstp	[edi].rZValue[0]
;		fstp	[edi].rZValue[4]
		;
		@INDEX = 0
		REPEAT	2
		;
		; テクスチャ座標を計算
		;
		fld1			; 母数係数計算
		fdiv	st, st(3)
		fld	st(1)		; x 座標計算
		fmul	st, st(1)
		fxch	st(1)		; y 座標計算
		fmul	st, st(3)
		;
		fxch	st(1)
		fistp	[ebx].ptTexturePos.x
		fistp	[ebx].ptTexturePos.y
		;
		; テクスチャ座標を進めつつピクセル情報を取得
		;
		fadd	st, st(3)
		fxch	st(1)
		mov	esi, [ebx].pTxLineAddr
		fadd	st, st(4)
		fxch	st(2)
		mov	edx, [ebx].ptTexturePos.y
		fadd	st, st(5)
		fxch	st(2)
		mov	ecx, edx
		mov	eax, [ebx].ptTexturePos.x
		sar	ecx, 31
		fxch	st(1)
		;
		not	ecx
		and	edx, ecx
		mov	ecx, eax
		.IF	edx >= (DWORD PTR [ebx].txtimg.dwImageHeight)
			mov	edx, [ebx].txtimg.dwImageHeight
			dec	edx
		.ENDIF
		sar	ecx, 31
		not	ecx
		and	eax, ecx
		.IF	eax >= (DWORD PTR [ebx].txtimg.dwImageWidth)
			mov	eax, [ebx].txtimg.dwImageWidth
			dec	eax
		.ENDIF
		;
		mov	ecx, [edi].rgbMul[@INDEX*4].dwPixelCode
		mov	esi, DWORD PTR [esi + edx * 4]
		movzx	edx, cl
		and	ecx, 00FFFFFFH
		lea	esi, [esi + eax * 4]
		.IF	ecx != 00FFFFFFH
			movzx	eax, BYTE PTR [esi]
			mov	cl, [edi].rgbAdd[@INDEX*4].rgb.Blue
			imul	eax, edx
			movzx	edx, [edi].rgbMul[@INDEX*4].rgb.Green
			add	ah, cl
			sbb	al, al
			movzx	ecx, BYTE PTR [esi + 1]
			or	al, ah
			imul	edx, ecx
			mov	[edi].rgbAdd[@INDEX*4].rgb.Blue, al
			movzx	ecx, [edi].rgbMul[@INDEX*4].rgb.Red
			movzx	eax, BYTE PTR [esi + 2]
			add	dh, [edi].rgbAdd[@INDEX*4].rgb.Green
			sbb	dl, dl
			imul	ecx, eax
			mov	al, [edi].rgbAdd[@INDEX*4].rgb.Red
			or	dl, dh
			add	ch, al
			mov	[edi].rgbAdd[@INDEX*4].rgb.Green, dl
			sbb	cl, cl
			mov	[edi].rgbMul[@INDEX*4].dwPixelCode, 0
			or	cl, ch
			mov	[edi].rgbAdd[@INDEX*4].rgb.Red, cl
		.ELSE
			mov	al, BYTE PTR [esi]
			mov	[edi].rgbMul[@INDEX*4].dwPixelCode, 0
			add	al, [edi].rgbAdd[@INDEX*4].rgb.Blue
			mov	cl, BYTE PTR [esi + 1]
			sbb	ah, ah
			add	cl, [edi].rgbAdd[@INDEX*4].rgb.Green
			mov	dl, BYTE PTR [esi + 2]
			sbb	ch, ch
			add	dl, [edi].rgbAdd[@INDEX*4].rgb.Red
			sbb	dh, dh
			or	al, ah
			or	cl, ch
			or	dl, dh
			mov	[edi].rgbAdd[@INDEX*4].rgb.Blue, al
			mov	[edi].rgbAdd[@INDEX*4].rgb.Green, cl
			mov	[edi].rgbAdd[@INDEX*4].rgb.Red, dl
		.ENDIF
		;
		@INDEX = @INDEX + 1
		ENDM
		;
		mov	ecx, [ebx].dib.nLeftWidth
		add	edi, (SIZEOF E3D_TRANS_LINE_BUF)
		dec	ecx
	.UNTIL	ZERO?
	;
	fstp	st(0)
	fstp	st(0)
	fstp	st(0)
	fstp	st(0)
	fstp	st(0)
	fstp	st(0)
	;
	ASSUME	edi:NOTHING
	ASSUME	ebx:NOTHING
	ret

eglRenderPoly@RenderPolygon_LineTX486	ENDP

;
;	テクスチャ（RGBA）あり非シェーディング関数 486 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_LineATXNS486	PROC	NEAR32 C

	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	mov	edi, [ebx].pLineBuf[0]
	ASSUME	edi:PTR E3D_TRANS_LINE_BUF
	;
	fld	[ebx].vTxxy.x
	fld	[ebx].vTxDeltaX.y
	fld	[ebx].vTxDeltaX.x
	mov	ecx, [ebx].nLineRight[4]
	;
	fld	[ebx].rTxLineMod
	fld	[ebx].vTxLinePos.y
	fld	[ebx].vTxLinePos.x
	sub	ecx, [ebx].nLineLeft[4]
	;
	fild	[ebx].nLineLeft[4]
	fmul	st, st(4)
	faddp	st(1), st
	fild	[ebx].nLineLeft[4]
	fmul	st, st(5)
	faddp	st(2), st
	fild	[ebx].nLineLeft[4]
	fmul	st, st(6)
	faddp	st(3), st
	;
	fld	[ebx].rRcpTxxy
	fmul	st(5), st
	fmul	st(4), st
	fmul	st(2), st
	fmulp	st(1), st
	;
	shr	ecx, 1
	ASSERT	<!!CARRY?>, "!(ecx & 01H)"
	fld1			; 母数係数計算
	fdiv	st, st(3)
	inc	ecx
	.REPEAT
		mov	[ebx].dib.nLeftWidth, ecx
		;
		@INDEX = 0
		REPEAT	2
		;
		; テクスチャ座標を計算
		;
;		fld1			; 母数係数計算
;		fdiv	st, st(3)
		fld	[ebx].rTxoxyr	; ｚ座標計算
		fmul	st, st(1)
			mov	esi, [ebx].pTxLineAddr
		fstp	[edi].rZValue[@INDEX*4]
		fld	st(1)		; x 座標計算
		fmul	st, st(1)
		fxch	st(1)		; y 座標計算
		fmul	st, st(3)
		;
		fxch	st(1)
		fistp	[ebx].ptTexturePos.x
		fistp	[ebx].ptTexturePos.y
			mov	edx, [ebx].ptTexturePos.y
			mov	eax, [ebx].ptTexturePos.x
		;
		; テクスチャ座標を進めつつピクセル情報を取得
		;
		fadd	st, st(3)
			mov	ecx, edx
		fxch	st(1)
		fadd	st, st(4)
			sar	ecx, 31
		fxch	st(2)
		fadd	st, st(5)
			not	ecx
		fxch	st(2)
			and	edx, ecx
			mov	ecx, eax
		fxch	st(1)
		;
			fld1			; 次の母数係数計算
			fdiv	st, st(3)
		.IF	edx >= (DWORD PTR [ebx].txtimg.dwImageHeight)
			mov	edx, [ebx].txtimg.dwImageHeight
			dec	edx
		.ENDIF
		sar	ecx, 31
		not	ecx
		and	eax, ecx
		mov	esi, DWORD PTR [esi + edx * 4]
		.IF	eax >= (DWORD PTR [ebx].txtimg.dwImageWidth)
			mov	eax, [ebx].txtimg.dwImageWidth
			dec	eax
		.ENDIF
		;
		mov	edx, DWORD PTR [esi + eax * 4]
		mov	eax, edx
		and	edx, 0FFFFFFH
		shr	eax, 24
		mov	[edi].rgbAdd[@INDEX*4].dwPixelCode, edx
		not	eax
		mov	[edi].rgbMul[@INDEX*4].rgb.Blue, al
		mov	[edi].rgbMul[@INDEX*4].rgb.Green, al
		mov	[edi].rgbMul[@INDEX*4].rgb.Red, al
		;
		@INDEX = @INDEX + 1
		ENDM
		;
		mov	ecx, [ebx].dib.nLeftWidth
		add	edi, (SIZEOF E3D_TRANS_LINE_BUF)
		dec	ecx
	.UNTIL	ZERO?
	;
	fstp	st(0)
	fstp	st(0)
	fstp	st(0)
	fstp	st(0)
	fstp	st(0)
	fstp	st(0)
	fstp	st(0)
	;
	ASSUME	edi:NOTHING
	ASSUME	ebx:NOTHING
	ret

eglRenderPoly@RenderPolygon_LineATXNS486	ENDP

;
;	テクスチャ（RGBA）ありシェーディング関数 486 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_LineATX486	PROC	NEAR32 C

	call	eglRenderPoly@RenderPolygon_LineNTX486
	;
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	mov	edi, [ebx].pLineBuf[0]
	ASSUME	edi:PTR E3D_TRANS_LINE_BUF
	;
	fld	[ebx].vTxxy.x
	fld	[ebx].vTxDeltaX.y
	fld	[ebx].vTxDeltaX.x
	mov	ecx, [ebx].nLineRight[4]
	;
	fld	[ebx].rTxLineMod
	fld	[ebx].vTxLinePos.y
	fld	[ebx].vTxLinePos.x
	sub	ecx, [ebx].nLineLeft[4]
	;
	fild	[ebx].nLineLeft[4]
	fmul	st, st(4)
	faddp	st(1), st
	fild	[ebx].nLineLeft[4]
	fmul	st, st(5)
	faddp	st(2), st
	fild	[ebx].nLineLeft[4]
	fmul	st, st(6)
	faddp	st(3), st
	;
	fld	[ebx].rRcpTxxy
	fmul	st(5), st
	fmul	st(4), st
	fmul	st(2), st
	fmulp	st(1), st
	;
	shr	ecx, 1
	ASSERT	<!!CARRY?>, "!(ecx & 01H)"
	inc	ecx
	.REPEAT
		mov	[ebx].dib.nLeftWidth, ecx
		;
;		fld	[edi].rZValue[4]
;		fld	[ebx].vScreenPos.z
;		fmul	st(1), st
;		fmul	[edi].rZValue[0]
;		fstp	[edi].rZValue[0]
;		fstp	[edi].rZValue[4]
		;
		@INDEX = 0
		REPEAT	2
		;
		; テクスチャ座標を計算
		;
		fld1			; 母数係数計算
		fdiv	st, st(3)
		fld	st(1)		; x 座標計算
		fmul	st, st(1)
		fxch	st(1)		; y 座標計算
		fmul	st, st(3)
		;
		fxch	st(1)
		fistp	[ebx].ptTexturePos.x
		fistp	[ebx].ptTexturePos.y
		;
		; テクスチャ座標を進めつつピクセル情報を取得
		;
		fadd	st, st(3)
		fxch	st(1)
		mov	esi, [ebx].pTxLineAddr
		fadd	st, st(4)
		fxch	st(2)
		mov	edx, [ebx].ptTexturePos.y
		fadd	st, st(5)
		fxch	st(2)
		mov	ecx, edx
		mov	eax, [ebx].ptTexturePos.x
		sar	ecx, 31
		fxch	st(1)
		;
		not	ecx
		and	edx, ecx
		mov	ecx, eax
		.IF	edx >= (DWORD PTR [ebx].txtimg.dwImageHeight)
			mov	edx, [ebx].txtimg.dwImageHeight
			dec	edx
		.ENDIF
		sar	ecx, 31
		not	ecx
		and	eax, ecx
		.IF	eax >= (DWORD PTR [ebx].txtimg.dwImageWidth)
			mov	eax, [ebx].txtimg.dwImageWidth
			dec	eax
		.ENDIF
		;
		mov	ecx, [edi].rgbMul[@INDEX*4].dwPixelCode
		mov	esi, DWORD PTR [esi + edx * 4]
		movzx	edx, cl
		and	ecx, 00FFFFFFH
		lea	esi, [esi + eax * 4]
		.IF	ecx != 00FFFFFFH
			movzx	eax, BYTE PTR [esi]
			mov	cl, [edi].rgbAdd[@INDEX*4].rgb.Blue
			imul	eax, edx
			movzx	edx, [edi].rgbMul[@INDEX*4].rgb.Green
			add	ah, cl
			sbb	al, al
			movzx	ecx, BYTE PTR [esi + 1]
			or	al, ah
			imul	edx, ecx
			mov	[edi].rgbAdd[@INDEX*4].rgb.Blue, al
			movzx	ecx, [edi].rgbMul[@INDEX*4].rgb.Red
			movzx	eax, BYTE PTR [esi + 2]
			add	dh, [edi].rgbAdd[@INDEX*4].rgb.Green
			sbb	dl, dl
			imul	ecx, eax
			mov	al, [edi].rgbAdd[@INDEX*4].rgb.Red
			or	dl, dh
			add	ch, al
			movzx	eax, BYTE PTR [esi + 3]
			mov	[edi].rgbAdd[@INDEX*4].rgb.Green, dl
			sbb	cl, cl
			movzx	edx, [edi].rgbMul[@INDEX*4].rgb.Blue
			xor	eax, 0FFH
			or	cl, ch
			mov	[edi].rgbAdd[@INDEX*4].rgb.Red, cl
			mov	[edi].rgbMul[@INDEX*4].rgb.Blue, al
			mov	[edi].rgbMul[@INDEX*4].rgb.Green, al
			mov	[edi].rgbMul[@INDEX*4].rgb.Red, al
		.ELSE
			mov	al, BYTE PTR [esi]
			add	al, [edi].rgbAdd[@INDEX*4].rgb.Blue
			mov	cl, BYTE PTR [esi + 1]
			sbb	ah, ah
			add	cl, [edi].rgbAdd[@INDEX*4].rgb.Green
			mov	dl, BYTE PTR [esi + 2]
			sbb	ch, ch
			add	dl, [edi].rgbAdd[@INDEX*4].rgb.Red
			sbb	dh, dh
			or	al, ah
			or	cl, ch
			or	dl, dh
			mov	[edi].rgbAdd[@INDEX*4].rgb.Blue, al
			mov	al, BYTE PTR [esi + 3]
			mov	[edi].rgbAdd[@INDEX*4].rgb.Green, cl
			not	al
			mov	[edi].rgbAdd[@INDEX*4].rgb.Red, dl
			mov	[edi].rgbMul[@INDEX*4].rgb.Blue, al
			mov	[edi].rgbMul[@INDEX*4].rgb.Green, al
			mov	[edi].rgbMul[@INDEX*4].rgb.Red, al
		.ENDIF
		;
		@INDEX = @INDEX + 1
		ENDM
		;
		mov	ecx, [ebx].dib.nLeftWidth
		add	edi, (SIZEOF E3D_TRANS_LINE_BUF)
		dec	ecx
	.UNTIL	ZERO?
	;
	fstp	st(0)
	fstp	st(0)
	fstp	st(0)
	fstp	st(0)
	fstp	st(0)
	fstp	st(0)
	;
	ASSUME	edi:NOTHING
	ASSUME	ebx:NOTHING
	ret

eglRenderPoly@RenderPolygon_LineATX486	ENDP

;
;	テクスチャ（RGB タイリング）ありシェーディング関数 486 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_LineTTX486	PROC	NEAR32 C

	call	eglRenderPoly@RenderPolygon_LineNTX486
	;
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	mov	edi, [ebx].pLineBuf[0]
	ASSUME	edi:PTR E3D_TRANS_LINE_BUF
	;
	fld	[ebx].vTxxy.x
	fld	[ebx].vTxDeltaX.y
	fld	[ebx].vTxDeltaX.x
	mov	ecx, [ebx].nLineRight[4]
	;
	fld	[ebx].rTxLineMod
	fld	[ebx].vTxLinePos.y
	fld	[ebx].vTxLinePos.x
	sub	ecx, [ebx].nLineLeft[4]
	;
	fild	[ebx].nLineLeft[4]
	fmul	st, st(4)
	faddp	st(1), st
	fild	[ebx].nLineLeft[4]
	fmul	st, st(5)
	faddp	st(2), st
	fild	[ebx].nLineLeft[4]
	fmul	st, st(6)
	faddp	st(3), st
	;
	fld	[ebx].rRcpTxxy
	fmul	st(5), st
	fmul	st(4), st
	fmul	st(2), st
	fmulp	st(1), st
	;
	shr	ecx, 1
	ASSERT	<!!CARRY?>, "!(ecx & 01H)"
	inc	ecx
	.REPEAT
		mov	[ebx].dib.nLeftWidth, ecx
		;
;		fld	[edi].rZValue[4]
;		fld	[ebx].vScreenPos.z
;		fmul	st(1), st
;		fmul	[edi].rZValue[0]
;		fstp	[edi].rZValue[0]
;		fstp	[edi].rZValue[4]
		;
		@INDEX = 0
		REPEAT	2
		;
		; テクスチャ座標を計算
		;
		fld1			; 母数係数計算
		fdiv	st, st(3)
		fld	st(1)		; x 座標計算
		fmul	st, st(1)
		fxch	st(1)		; y 座標計算
		fmul	st, st(3)
		;
		fxch	st(1)
		fistp	[ebx].ptTexturePos.x
		fistp	[ebx].ptTexturePos.y
		;
		; テクスチャ座標を進めつつピクセル情報を取得
		;
		fadd	st, st(3)
		fxch	st(1)
		mov	esi, [ebx].pTxLineAddr
		fadd	st, st(4)
		fxch	st(2)
		mov	edx, [ebx].ptTexturePos.y
		fadd	st, st(5)
		fxch	st(2)
		mov	eax, [ebx].ptTexturePos.x
		fxch	st(1)
		and	dx, WORD PTR [ebx].txSizeMask[2]
		and	ax, WORD PTR [ebx].txSizeMask[0]
		movzx	edx, dx
		movzx	eax, ax
		;
		mov	ecx, [edi].rgbMul[@INDEX*4].dwPixelCode
		mov	esi, DWORD PTR [esi + edx * 4]
		movzx	edx, cl
		and	ecx, 00FFFFFFH
		lea	esi, [esi + eax * 4]
		.IF	ecx != 00FFFFFFH
			movzx	eax, BYTE PTR [esi]
			mov	cl, [edi].rgbAdd[@INDEX*4].rgb.Blue
			imul	eax, edx
			movzx	edx, [edi].rgbMul[@INDEX*4].rgb.Green
			add	ah, cl
			sbb	al, al
			movzx	ecx, BYTE PTR [esi + 1]
			or	al, ah
			imul	edx, ecx
			mov	[edi].rgbAdd[@INDEX*4].rgb.Blue, al
			movzx	ecx, [edi].rgbMul[@INDEX*4].rgb.Red
			movzx	eax, BYTE PTR [esi + 2]
			add	dh, [edi].rgbAdd[@INDEX*4].rgb.Green
			sbb	dl, dl
			imul	ecx, eax
			mov	al, [edi].rgbAdd[@INDEX*4].rgb.Red
			or	dl, dh
			add	ch, al
			mov	[edi].rgbAdd[@INDEX*4].rgb.Green, dl
			sbb	cl, cl
			mov	[edi].rgbMul[@INDEX*4].dwPixelCode, 0
			or	cl, ch
			mov	[edi].rgbAdd[@INDEX*4].rgb.Red, cl
		.ELSE
			mov	al, BYTE PTR [esi]
			mov	[edi].rgbMul[@INDEX*4].dwPixelCode, 0
			add	al, [edi].rgbAdd[@INDEX*4].rgb.Blue
			mov	cl, BYTE PTR [esi + 1]
			sbb	ah, ah
			add	cl, [edi].rgbAdd[@INDEX*4].rgb.Green
			mov	dl, BYTE PTR [esi + 2]
			sbb	ch, ch
			add	dl, [edi].rgbAdd[@INDEX*4].rgb.Red
			sbb	dh, dh
			or	al, ah
			or	cl, ch
			or	dl, dh
			mov	[edi].rgbAdd[@INDEX*4].rgb.Blue, al
			mov	[edi].rgbAdd[@INDEX*4].rgb.Green, cl
			mov	[edi].rgbAdd[@INDEX*4].rgb.Red, dl
		.ENDIF
		;
		@INDEX = @INDEX + 1
		ENDM
		;
		mov	ecx, [ebx].dib.nLeftWidth
		add	edi, (SIZEOF E3D_TRANS_LINE_BUF)
		dec	ecx
	.UNTIL	ZERO?
	;
	fstp	st(0)
	fstp	st(0)
	fstp	st(0)
	fstp	st(0)
	fstp	st(0)
	fstp	st(0)
	;
	ASSUME	edi:NOTHING
	ASSUME	ebx:NOTHING
	ret

eglRenderPoly@RenderPolygon_LineTTX486	ENDP

;
;	テクスチャ（RGBA タイリング）ありシェーディング関数 486 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_LineTATX486	PROC	NEAR32 C

	call	eglRenderPoly@RenderPolygon_LineNTX486
	;
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	mov	edi, [ebx].pLineBuf[0]
	ASSUME	edi:PTR E3D_TRANS_LINE_BUF
	;
	fld	[ebx].vTxxy.x
	fld	[ebx].vTxDeltaX.y
	fld	[ebx].vTxDeltaX.x
	mov	ecx, [ebx].nLineRight[4]
	;
	fld	[ebx].rTxLineMod
	fld	[ebx].vTxLinePos.y
	fld	[ebx].vTxLinePos.x
	sub	ecx, [ebx].nLineLeft[4]
	;
	fild	[ebx].nLineLeft[4]
	fmul	st, st(4)
	faddp	st(1), st
	fild	[ebx].nLineLeft[4]
	fmul	st, st(5)
	faddp	st(2), st
	fild	[ebx].nLineLeft[4]
	fmul	st, st(6)
	faddp	st(3), st
	;
	fld	[ebx].rRcpTxxy
	fmul	st(5), st
	fmul	st(4), st
	fmul	st(2), st
	fmulp	st(1), st
	;
	shr	ecx, 1
	ASSERT	<!!CARRY?>, "!(ecx & 01H)"
	inc	ecx
	.REPEAT
		mov	[ebx].dib.nLeftWidth, ecx
		;
;		fld	[edi].rZValue[4]
;		fld	[ebx].vScreenPos.z
;		fmul	st(1), st
;		fmul	[edi].rZValue[0]
;		fstp	[edi].rZValue[0]
;		fstp	[edi].rZValue[4]
		;
		@INDEX = 0
		REPEAT	2
		;
		; テクスチャ座標を計算
		;
		fld1			; 母数係数計算
		fdiv	st, st(3)
		fld	st(1)		; x 座標計算
		fmul	st, st(1)
		fxch	st(1)		; y 座標計算
		fmul	st, st(3)
		;
		fxch	st(1)
		fistp	[ebx].ptTexturePos.x
		fistp	[ebx].ptTexturePos.y
		;
		; テクスチャ座標を進めつつピクセル情報を取得
		;
		fadd	st, st(3)
		fxch	st(1)
		mov	esi, [ebx].pTxLineAddr
		fadd	st, st(4)
		fxch	st(2)
		mov	edx, [ebx].ptTexturePos.y
		fadd	st, st(5)
		fxch	st(2)
		mov	eax, [ebx].ptTexturePos.x
		fxch	st(1)
		and	dx, [ebx].txSizeMask[2]
		and	ax, [ebx].txSizeMask[0]
		movzx	edx, dx
		movzx	eax, ax
		;
		mov	ecx, [edi].rgbMul[@INDEX*4].dwPixelCode
		mov	esi, DWORD PTR [esi + edx * 4]
		movzx	edx, cl
		and	ecx, 00FFFFFFH
		lea	esi, [esi + eax * 4]
		.IF	ecx != 00FFFFFFH
			movzx	eax, BYTE PTR [esi]
			mov	cl, [edi].rgbAdd[@INDEX*4].rgb.Blue
			imul	eax, edx
			movzx	edx, [edi].rgbMul[@INDEX*4].rgb.Green
			add	ah, cl
			sbb	al, al
			movzx	ecx, BYTE PTR [esi + 1]
			or	al, ah
			imul	edx, ecx
			mov	[edi].rgbAdd[@INDEX*4].rgb.Blue, al
			movzx	ecx, [edi].rgbMul[@INDEX*4].rgb.Red
			movzx	eax, BYTE PTR [esi + 2]
			add	dh, [edi].rgbAdd[@INDEX*4].rgb.Green
			sbb	dl, dl
			imul	ecx, eax
			mov	al, [edi].rgbAdd[@INDEX*4].rgb.Red
			or	dl, dh
			add	ch, al
			movzx	eax, BYTE PTR [esi + 3]
			mov	[edi].rgbAdd[@INDEX*4].rgb.Green, dl
			sbb	cl, cl
			movzx	edx, [edi].rgbMul[@INDEX*4].rgb.Blue
			xor	eax, 0FFH
			or	cl, ch
			mov	[edi].rgbAdd[@INDEX*4].rgb.Red, cl
			;
			imul	edx, eax
			movzx	ecx, [edi].rgbMul[@INDEX*4].rgb.Green
			mov	[edi].rgbMul[@INDEX*4].rgb.Blue, dh
			imul	ecx, eax
			movzx	edx, [edi].rgbMul[@INDEX*4].rgb.Red
			mov	[edi].rgbMul[@INDEX*4].rgb.Green, ch
			imul	edx, eax
			mov	[edi].rgbMul[@INDEX*4].rgb.Red, dh
		.ELSE
			mov	al, BYTE PTR [esi]
			add	al, [edi].rgbAdd[@INDEX*4].rgb.Blue
			mov	cl, BYTE PTR [esi + 1]
			sbb	ah, ah
			add	cl, [edi].rgbAdd[@INDEX*4].rgb.Green
			mov	dl, BYTE PTR [esi + 2]
			sbb	ch, ch
			add	dl, [edi].rgbAdd[@INDEX*4].rgb.Red
			sbb	dh, dh
			or	al, ah
			or	cl, ch
			or	dl, dh
			mov	[edi].rgbAdd[@INDEX*4].rgb.Blue, al
			mov	al, BYTE PTR [esi + 3]
			mov	[edi].rgbAdd[@INDEX*4].rgb.Green, cl
			not	al
			mov	[edi].rgbAdd[@INDEX*4].rgb.Red, dl
			mov	[edi].rgbMul[@INDEX*4].rgb.Blue, al
			mov	[edi].rgbMul[@INDEX*4].rgb.Green, al
			mov	[edi].rgbMul[@INDEX*4].rgb.Red, al
		.ENDIF
		;
		@INDEX = @INDEX + 1
		ENDM
		;
		mov	ecx, [ebx].dib.nLeftWidth
		add	edi, (SIZEOF E3D_TRANS_LINE_BUF)
		dec	ecx
	.UNTIL	ZERO?
	;
	fstp	st(0)
	fstp	st(0)
	fstp	st(0)
	fstp	st(0)
	fstp	st(0)
	fstp	st(0)
	;
	ASSUME	edi:NOTHING
	ASSUME	ebx:NOTHING
	ret

eglRenderPoly@RenderPolygon_LineTATX486	ENDP

;
;	テクスチャ（RGB＋発光）ありシェーディング関数 486 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_LineTLTX486	PROC	NEAR32 C

	call	eglRenderPoly@RenderPolygon_LineNTX486
	;
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	mov	edi, [ebx].pLineBuf[0]
	ASSUME	edi:PTR E3D_TRANS_LINE_BUF
	;
	fld	[ebx].vTxxy.x
	fld	[ebx].vTxDeltaX.y
	fld	[ebx].vTxDeltaX.x
	mov	ecx, [ebx].nLineRight[4]
	;
	fld	[ebx].rTxLineMod
	fld	[ebx].vTxLinePos.y
	fld	[ebx].vTxLinePos.x
	sub	ecx, [ebx].nLineLeft[4]
	;
	fild	[ebx].nLineLeft[4]
	fmul	st, st(4)
	faddp	st(1), st
	fild	[ebx].nLineLeft[4]
	fmul	st, st(5)
	faddp	st(2), st
	fild	[ebx].nLineLeft[4]
	fmul	st, st(6)
	faddp	st(3), st
	;
	fld	[ebx].rRcpTxxy
	fmul	st(5), st
	fmul	st(4), st
	fmul	st(2), st
	fmulp	st(1), st
	;
	shr	ecx, 1
	ASSERT	<!!CARRY?>, "!(ecx & 01H)"
	inc	ecx
	.REPEAT
		mov	[ebx].dib.nLeftWidth, ecx
		;
;		fld	[edi].rZValue[4]
;		fld	[ebx].vScreenPos.z
;		fmul	st(1), st
;		fmul	[edi].rZValue[0]
;		fstp	[edi].rZValue[0]
;		fstp	[edi].rZValue[4]
		;
		@INDEX = 0
		REPEAT	2
		;
		; テクスチャ座標を計算
		;
		fld1			; 母数係数計算
		fdiv	st, st(3)
		fld	st(1)		; x 座標計算
		fmul	st, st(1)
		fxch	st(1)		; y 座標計算
		fmul	st, st(3)
		;
		fxch	st(1)
		fistp	[ebx].ptTexturePos.x
		fistp	[ebx].ptTexturePos.y
		;
		; テクスチャ座標を進めつつピクセル情報を取得
		;
		fadd	st, st(3)
		fxch	st(1)
		mov	esi, [ebx].pTxLineAddr
		fadd	st, st(4)
		fxch	st(2)
		mov	edx, [ebx].ptTexturePos.y
		fadd	st, st(5)
		fxch	st(2)
		mov	eax, [ebx].ptTexturePos.x
		fxch	st(1)
		and	dx, [ebx].txSizeMask[2]
		and	ax, [ebx].txSizeMask[0]
		movzx	edx, dx
		movzx	eax, ax
		;
		mov	ecx, [edi].rgbMul[@INDEX*4].dwPixelCode
		mov	[ebx].ptTexturePos.y, edx
		mov	esi, DWORD PTR [esi + edx * 4]
		mov	[ebx].ptTexturePos.x, eax
		movzx	edx, cl
		and	ecx, 00FFFFFFH
		lea	esi, [esi + eax * 4]
		.IF	ecx != 00FFFFFFH
			movzx	eax, BYTE PTR [esi]
			mov	cl, [edi].rgbAdd[@INDEX*4].rgb.Blue
			imul	eax, edx
			movzx	edx, [edi].rgbMul[@INDEX*4].rgb.Green
			add	ah, cl
			sbb	al, al
			movzx	ecx, BYTE PTR [esi + 1]
			or	al, ah
			imul	edx, ecx
			mov	[edi].rgbAdd[@INDEX*4].rgb.Blue, al
			movzx	ecx, [edi].rgbMul[@INDEX*4].rgb.Red
			movzx	eax, BYTE PTR [esi + 2]
			add	dh, [edi].rgbAdd[@INDEX*4].rgb.Green
			sbb	dl, dl
			imul	ecx, eax
			mov	al, [edi].rgbAdd[@INDEX*4].rgb.Red
			or	dl, dh
			add	ch, al
			movzx	eax, BYTE PTR [esi + 3]
			mov	[edi].rgbAdd[@INDEX*4].rgb.Green, dl
			sbb	cl, cl
			movzx	edx, [edi].rgbMul[@INDEX*4].rgb.Blue
			xor	eax, 0FFH
			or	cl, ch
			mov	[edi].rgbAdd[@INDEX*4].rgb.Red, cl
			;
			imul	edx, eax
			movzx	ecx, [edi].rgbMul[@INDEX*4].rgb.Green
			mov	[edi].rgbMul[@INDEX*4].rgb.Blue, dh
			imul	ecx, edx
			movzx	edx, [edi].rgbMul[@INDEX*4].rgb.Red
			mov	[edi].rgbMul[@INDEX*4].rgb.Green, ch
			imul	edx, eax
			mov	esi, [ebx].pLmLineAddr
			mov	[edi].rgbMul[@INDEX*4].rgb.Red, dh
		.ELSE
			mov	al, BYTE PTR [esi]
			add	al, [edi].rgbAdd[@INDEX*4].rgb.Blue
			mov	cl, BYTE PTR [esi + 1]
			sbb	ah, ah
			add	cl, [edi].rgbAdd[@INDEX*4].rgb.Green
			mov	dl, BYTE PTR [esi + 2]
			sbb	ch, ch
			add	dl, [edi].rgbAdd[@INDEX*4].rgb.Red
			sbb	dh, dh
			or	al, ah
			or	cl, ch
			or	dl, dh
			mov	[edi].rgbAdd[@INDEX*4].rgb.Blue, al
			mov	al, BYTE PTR [esi + 3]
			mov	[edi].rgbAdd[@INDEX*4].rgb.Green, cl
			not	al
			mov	[edi].rgbAdd[@INDEX*4].rgb.Red, dl
			mov	esi, [ebx].pLmLineAddr
			mov	[edi].rgbMul[@INDEX*4].rgb.Blue, al
			mov	[edi].rgbMul[@INDEX*4].rgb.Green, al
			mov	[edi].rgbMul[@INDEX*4].rgb.Red, al
		.ENDIF
		;
		mov	ecx, [ebx].nLiminousApply
		mov	edx, [ebx].ptTexturePos.y
		mov	eax, [ebx].ptTexturePos.x
		mov	esi, DWORD PTR [esi + edx * 4]
		.IF	ecx == 100H
			mov	cl, [edi].rgbAdd[@INDEX*4].rgb.Blue
			mov	dl, [edi].rgbAdd[@INDEX*4].rgb.Green
			lea	esi, [esi + eax * 4]
			mov	al, [edi].rgbAdd[@INDEX*4].rgb.Red
			add	cl, BYTE PTR [esi]
			sbb	ch, ch
			add	dl, BYTE PTR [esi + 1]
			sbb	dh, dh
			add	al, BYTE PTR [esi + 2]
			sbb	ah, ah
			or	cl, ch
			or	dl, dh
			or	al, ah
			mov	[edi].rgbAdd[@INDEX*4].rgb.Blue, cl
			mov	[edi].rgbAdd[@INDEX*4].rgb.Green, dl
			mov	[edi].rgbAdd[@INDEX*4].rgb.Red, al
		.ELSE
			lea	esi, [esi + eax * 4]
			movzx	eax, BYTE PTR [esi]
			movzx	edx, BYTE PTR [esi + 1]
			imul	eax, ecx
			imul	edx, ecx
			add	ah, [edi].rgbAdd[@INDEX*4].rgb.Blue
			sbb	al, al
			add	dh, [edi].rgbAdd[@INDEX*4].rgb.Green
			sbb	dl, dl
			or	al, ah
			or	dl, dh
			mov	[edi].rgbAdd[@INDEX*4].rgb.Blue, al
			movzx	eax, BYTE PTR [esi + 2]
			mov	[edi].rgbAdd[@INDEX*4].rgb.Green, dl
			imul	eax, ecx
			add	ah, [edi].rgbAdd[@INDEX*4].rgb.Red
			sbb	al, al
			or	al, ah
			mov	[edi].rgbAdd[@INDEX*4].rgb.Red, al
		.ENDIF
		;
		@INDEX = @INDEX + 1
		ENDM
		;
		mov	ecx, [ebx].dib.nLeftWidth
		add	edi, (SIZEOF E3D_TRANS_LINE_BUF)
		dec	ecx
	.UNTIL	ZERO?
	;
	fstp	st(0)
	fstp	st(0)
	fstp	st(0)
	fstp	st(0)
	fstp	st(0)
	fstp	st(0)
	;
	ASSUME	edi:NOTHING
	ASSUME	ebx:NOTHING
	ret

eglRenderPoly@RenderPolygon_LineTLTX486	ENDP

;
;	レンダリングフィル（RGB）関数 486 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_Store486	PROC	NEAR32 C

	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	mov	[ebx].dib.dwSaveRegEBP, ebp
	mov	esi, [ebx].pLineBuf[0]
	ASSUME	esi:PTR E3D_TRANS_LINE_BUF
	mov	ebp, [ebx].dib.ptrZBufLine
	mov	edi, [ebx].dib.ptrDstLine
	;
	mov	ecx, [ebx].nLineLeft[0]
	.IF	[ebx].nLeftDecimal > LEFT_ERROR_RANGE
		inc	ecx
		.IF	!(ecx & 01H)
			add	esi, (SIZEOF E3D_TRANS_LINE_BUF)
		.ENDIF
	.ENDIF
	cmp	ecx, [ebx].nLineRight[0]
	ja	Label_Exit
	;
	lea	ebp, [ebp + ecx * 4]
	lea	edi, [edi + ecx * 4]
	;
	.WHILE	ecx <= [ebx].nLineRight[0]
		.IF	!(ecx & 0001H)
			inc	ecx
			mov	eax, [esi].rZValue[0]
			mov	edx, [esi].rgbAdd[0].dwPixelCode
			.IF	eax <= (DWORD PTR [ebp])
				mov	DWORD PTR [edi], edx
				mov	DWORD PTR [ebp], eax
			.ENDIF
			add	ebp, 4
			add	edi, 4
		.ELSE
			inc	ecx
			mov	eax, [esi].rZValue[4]
			mov	edx, [esi].rgbAdd[4].dwPixelCode
			.IF	eax <= (DWORD PTR [ebp])
				mov	DWORD PTR [edi], edx
				mov	DWORD PTR [ebp], eax
			.ENDIF
			add	esi, (SIZEOF E3D_TRANS_LINE_BUF)
			add	ebp, 4
			add	edi, 4
		.ENDIF
	.ENDW
	;
Label_Exit:
	mov	ebp, [ebx].dib.dwSaveRegEBP
	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	ret

eglRenderPoly@RenderPolygon_Store486	ENDP

;
;	レンダリングフィル（RGB・ｚ比較無）関数 486 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_StoreNZ486	PROC	NEAR32 C

	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	mov	[ebx].dib.dwSaveRegEBP, ebp
	mov	esi, [ebx].pLineBuf[0]
	ASSUME	esi:PTR E3D_TRANS_LINE_BUF
	mov	ebp, [ebx].dib.ptrZBufLine
	mov	edi, [ebx].dib.ptrDstLine
	;
	mov	ecx, [ebx].nLineLeft[0]
	.IF	[ebx].nLeftDecimal > LEFT_ERROR_RANGE
		inc	ecx
		.IF	!(ecx & 01H)
			add	esi, (SIZEOF E3D_TRANS_LINE_BUF)
		.ENDIF
	.ENDIF
	cmp	ecx, [ebx].nLineRight[0]
	ja	Label_Exit
	;
	lea	ebp, [ebp + ecx * 4]
	lea	edi, [edi + ecx * 4]
	;
	.IF	ecx & 01H
		mov	eax, [esi].rgbAdd[4].dwPixelCode
		add	esi, (SIZEOF E3D_TRANS_LINE_BUF)
		mov	DWORD PTR [edi], eax
		add	edi, 4
		inc	ecx
	.ENDIF
	;
	dec	ecx
	mov	edx, [ebx].nLineRight[0]
	sub	edx, ecx
	mov	ecx, edx
	sub	ecx, 2
	.WHILE	!SIGN?
		mov	eax, [esi].rgbAdd[0].dwPixelCode
		mov	edx, [esi].rgbAdd[4].dwPixelCode
		add	esi, (SIZEOF E3D_TRANS_LINE_BUF)
		mov	DWORD PTR [edi], eax
		mov	DWORD PTR [edi + 4], edx
		add	edi, 8
		sub	ecx, 2
	.ENDW
	;
	add	ecx, 2
	.IF	!ZERO?
		mov	eax, [esi].rgbAdd[0].dwPixelCode
		mov	DWORD PTR [edi], eax
	.ENDIF
	;
Label_Exit:
	mov	ebp, [ebx].dib.dwSaveRegEBP
	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	ret

eglRenderPoly@RenderPolygon_StoreNZ486	ENDP

;
;	レンダリング透明度付フィル（RGB）関数 486 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_StoreT486	PROC	NEAR32 C

	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	mov	[ebx].dib.dwSaveRegEBP, ebp
	mov	esi, [ebx].pLineBuf[0]
	ASSUME	esi:PTR E3D_TRANS_LINE_BUF
	mov	ebp, [ebx].dib.ptrZBufLine
	mov	edi, [ebx].dib.ptrDstLine
	;
	mov	ecx, [ebx].nLineLeft[0]
	.IF	([ebx].nLeftDecimal > 0) || (ecx > [ebx].dib.rectClip.left)
		inc	ecx
		.IF	!(ecx & 01H)
			add	esi, (SIZEOF E3D_TRANS_LINE_BUF)
		.ENDIF
	.ENDIF
	cmp	ecx, [ebx].nLineRight[0]
	ja	Label_Exit
	;
	lea	ebp, [ebp + ecx * 4]
	lea	edi, [edi + ecx * 4]
	;
	fld	[esi].rZValue[0]
	fsub	[esi].rZValue[4]
	mov	eax, [esi].rZValue[0]
	fabs
	fadd	st, st(0)
	sub	eax, 04800000H
	fstp	REAL4 PTR [ebx].dib.dwTemp[0]
	.IF	(SDWORD PTR eax) > (SDWORD PTR [ebx].dib.dwTemp[0])
		mov	[ebx].dib.dwTemp[0], eax
	.ENDIF
	;
	.WHILE	ecx <= [ebx].nLineRight[0]
		.IF	!(ecx & 0001H)
			fld	[esi].rZValue[0]
			fsub	[esi].rZValue[4]
			mov	eax, [esi].rZValue[0]
			fabs
			fadd	st, st(0)
			sub	eax, 04800000H
			fst	REAL4 PTR [ebx].dib.dwTemp[0]
			.IF	(SDWORD PTR eax) > (SDWORD PTR [ebx].dib.dwTemp[0])
				mov	[ebx].dib.dwTemp[0], eax
				fstp	st(0)
				fld	REAL4 PTR [ebx].dib.dwTemp[0]
			.ENDIF
			inc	ecx
			mov	eax, [esi].rZValue[0]
			fadd	[esi].rZValue[0]
			mov	edx, [esi].rgbMul[0].dwPixelCode
			fsub	REAL4 PTR [ebp]
			fstp	REAL4 PTR [ebx].dib.dwTemp[4]
			;
			.IF	[ebx].dib.dwTemp[4] & 80000000H
				mov	DWORD PTR [ebp], eax
				mov	[ebx].dib.nLeftWidth, ecx
				movzx	eax, BYTE PTR [edi]
				movzx	ecx, [esi].rgbAdd[0].rgb.Blue
				mov	al, [ebx].dib.nBlueTone[eax]
				mov	cl, [ebx].dib.nGreenTone[ecx]
				movzx	edx, BYTE PTR [edi + 1]
				add	al, cl
				movzx	ecx, [esi].rgbAdd[0].rgb.Green
				mov	BYTE PTR [edi], al
				mov	dl, [ebx].dib.nBlueTone[edx]
				mov	cl, [ebx].dib.nGreenTone[ecx]
				movzx	eax, BYTE PTR [edi + 2]
				add	dl, cl
				movzx	ecx, [esi].rgbAdd[0].rgb.Red
				mov	BYTE PTR [edi + 1], dl
				mov	al, [ebx].dib.nBlueTone[eax]
				mov	cl, [ebx].dib.nGreenTone[ecx]
				add	al, cl
				mov	ecx, [ebx].dib.nLeftWidth
				mov	BYTE PTR [edi + 2], al
			.ENDIF
			add	ebp, 4
			add	edi, 4
		.ELSE
			fld	REAL4 PTR [ebx].dib.dwTemp[0]
			mov	eax, [esi].rZValue[4]
			fadd	[esi].rZValue[4]
			fsub	REAL4 PTR [ebp]
			inc	ecx
			fstp	REAL4 PTR [ebx].dib.dwTemp[4]
			;
			.IF	[ebx].dib.dwTemp[4] & 80000000H
				mov	DWORD PTR [ebp], eax
				mov	[ebx].dib.nLeftWidth, ecx
				movzx	eax, BYTE PTR [edi]
				movzx	ecx, [esi].rgbAdd[4].rgb.Blue
				mov	al, [ebx].dib.nBlueTone[eax]
				mov	cl, [ebx].dib.nGreenTone[ecx]
				movzx	edx, BYTE PTR [edi + 1]
				add	al, cl
				movzx	ecx, [esi].rgbAdd[4].rgb.Green
				mov	BYTE PTR [edi], al
				mov	dl, [ebx].dib.nBlueTone[edx]
				mov	cl, [ebx].dib.nGreenTone[ecx]
				movzx	eax, BYTE PTR [edi + 2]
				add	dl, cl
				movzx	ecx, [esi].rgbAdd[4].rgb.Red
				mov	BYTE PTR [edi + 1], dl
				mov	al, [ebx].dib.nBlueTone[eax]
				mov	cl, [ebx].dib.nGreenTone[ecx]
				add	al, cl
				mov	ecx, [ebx].dib.nLeftWidth
				mov	BYTE PTR [edi + 2], al
			.ENDIF
			add	esi, (SIZEOF E3D_TRANS_LINE_BUF)
			add	ebp, 4
			add	edi, 4
		.ENDIF
	.ENDW
	;
Label_Exit:
	mov	ebp, [ebx].dib.dwSaveRegEBP
	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	ret

eglRenderPoly@RenderPolygon_StoreT486	ENDP

;
;	レンダリングフィル（RGBA トリム）関数 486 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_StoreM486	PROC	NEAR32 C

	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	mov	[ebx].dib.dwSaveRegEBP, ebp
	mov	esi, [ebx].pLineBuf[0]
	ASSUME	esi:PTR E3D_TRANS_LINE_BUF
	mov	ebp, [ebx].dib.ptrZBufLine
	mov	edi, [ebx].dib.ptrDstLine
	;
	mov	ecx, [ebx].nLineLeft[0]
	.IF	[ebx].nLeftDecimal > LEFT_ERROR_RANGE
		inc	ecx
		.IF	!(ecx & 01H)
			add	esi, (SIZEOF E3D_TRANS_LINE_BUF)
		.ENDIF
	.ENDIF
	cmp	ecx, [ebx].nLineRight[0]
	ja	Label_Exit
	;
	lea	ebp, [ebp + ecx * 4]
	lea	edi, [edi + ecx * 4]
	;
	.WHILE	ecx <= [ebx].nLineRight[0]
		.IF	!(ecx & 0001H)
			inc	ecx
			mov	eax, [esi].rZValue[0]
			mov	edx, [esi].rgbAdd[0].dwPixelCode
			.IF	!([esi].rgbMul[0].dwPixelCode & 00808080H)
				.IF	eax <= (DWORD PTR [ebp])
					mov	DWORD PTR [edi], edx
					mov	DWORD PTR [ebp], eax
				.ENDIF
			.ENDIF
			add	ebp, 4
			add	edi, 4
		.ELSE
			inc	ecx
			mov	eax, [esi].rZValue[4]
			mov	edx, [esi].rgbAdd[4].dwPixelCode
			.IF	!([esi].rgbMul[4].dwPixelCode & 00808080H)
				.IF	eax <= (DWORD PTR [ebp])
					mov	DWORD PTR [edi], edx
					mov	DWORD PTR [ebp], eax
				.ENDIF
			.ENDIF
			add	esi, (SIZEOF E3D_TRANS_LINE_BUF)
			add	ebp, 4
			add	edi, 4
		.ENDIF
	.ENDW
	;
Label_Exit:
	mov	ebp, [ebx].dib.dwSaveRegEBP
	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	ret

eglRenderPoly@RenderPolygon_StoreM486	ENDP

;
;	レンダリング透明度付フィル（RGBA トリム）関数 486 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_StoreMT486	PROC	NEAR32 C

	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	mov	[ebx].dib.dwSaveRegEBP, ebp
	mov	esi, [ebx].pLineBuf[0]
	ASSUME	esi:PTR E3D_TRANS_LINE_BUF
	mov	ebp, [ebx].dib.ptrZBufLine
	mov	edi, [ebx].dib.ptrDstLine
	;
	mov	ecx, [ebx].nLineLeft[0]
	.IF	([ebx].nLeftDecimal > 0) || (ecx > [ebx].dib.rectClip.left)
		inc	ecx
		.IF	!(ecx & 01H)
			add	esi, (SIZEOF E3D_TRANS_LINE_BUF)
		.ENDIF
	.ENDIF
	cmp	ecx, [ebx].nLineRight[0]
	ja	Label_Exit
	;
	lea	ebp, [ebp + ecx * 4]
	lea	edi, [edi + ecx * 4]
	;
	fld	[esi].rZValue[0]
	fsub	[esi].rZValue[4]
	mov	eax, [esi].rZValue[0]
	fabs
	fadd	st, st(0)
	sub	eax, 04800000H
	fstp	REAL4 PTR [ebx].dib.dwTemp[0]
	.IF	(SDWORD PTR eax) > (SDWORD PTR [ebx].dib.dwTemp[0])
		mov	[ebx].dib.dwTemp[0], eax
	.ENDIF
	;
	.WHILE	ecx <= [ebx].nLineRight[0]
		.IF	!(ecx & 0001H)
			fld	[esi].rZValue[0]
			fsub	[esi].rZValue[4]
			mov	eax, [esi].rZValue[0]
			fabs
			fadd	st, st(0)
			sub	eax, 04800000H
			fst	REAL4 PTR [ebx].dib.dwTemp[0]
			.IF	(SDWORD PTR eax) > (SDWORD PTR [ebx].dib.dwTemp[0])
				mov	[ebx].dib.dwTemp[0], eax
				fstp	st(0)
				fld	REAL4 PTR [ebx].dib.dwTemp[0]
			.ENDIF
			fadd	[esi].rZValue[0]
			inc	ecx
			fsub	REAL4 PTR [ebp]
			mov	eax, [esi].rZValue[0]
			fstp	REAL4 PTR [ebx].dib.dwTemp[4]
			.IF	!([esi].rgbMul[0].dwPixelCode & 00808080H)
				.IF	[ebx].dib.dwTemp[4] & 80000000H
					mov	DWORD PTR [ebp], eax
					mov	[ebx].dib.nLeftWidth, ecx
					movzx	eax, BYTE PTR [edi]
					movzx	ecx, [esi].rgbAdd[0].rgb.Blue
					mov	al, [ebx].dib.nBlueTone[eax]
					mov	cl, [ebx].dib.nGreenTone[ecx]
					movzx	edx, BYTE PTR [edi + 1]
					add	al, cl
					movzx	ecx, [esi].rgbAdd[0].rgb.Green
					mov	BYTE PTR [edi], al
					mov	dl, [ebx].dib.nBlueTone[edx]
					mov	cl, [ebx].dib.nGreenTone[ecx]
					movzx	eax, BYTE PTR [edi + 2]
					add	dl, cl
					movzx	ecx, [esi].rgbAdd[0].rgb.Red
					mov	BYTE PTR [edi + 1], dl
					mov	al, [ebx].dib.nBlueTone[eax]
					mov	cl, [ebx].dib.nGreenTone[ecx]
					add	al, cl
					mov	ecx, [ebx].dib.nLeftWidth
					mov	BYTE PTR [edi + 2], al
				.ENDIF
			.ENDIF
			add	ebp, 4
			add	edi, 4
		.ELSE
			fld	REAL4 PTR [ebx].dib.dwTemp[0]
			fadd	[esi].rZValue[4]
			inc	ecx
			fsub	REAL4 PTR [ebp]
			mov	eax, [esi].rZValue[4]
			fstp	REAL4 PTR [ebx].dib.dwTemp[4]
			.IF	!([esi].rgbMul[4].dwPixelCode & 00808080H)
				.IF	[ebx].dib.dwTemp[4] & 80000000H
					mov	DWORD PTR [ebp], eax
					mov	[ebx].dib.nLeftWidth, ecx
					movzx	eax, BYTE PTR [edi]
					movzx	ecx, [esi].rgbAdd[4].rgb.Blue
					mov	al, [ebx].dib.nBlueTone[eax]
					mov	cl, [ebx].dib.nGreenTone[ecx]
					movzx	edx, BYTE PTR [edi + 1]
					add	al, cl
					movzx	ecx, [esi].rgbAdd[4].rgb.Green
					mov	BYTE PTR [edi], al
					mov	dl, [ebx].dib.nBlueTone[edx]
					mov	cl, [ebx].dib.nGreenTone[ecx]
					movzx	eax, BYTE PTR [edi + 2]
					add	dl, cl
					movzx	ecx, [esi].rgbAdd[4].rgb.Red
					mov	BYTE PTR [edi + 1], dl
					mov	al, [ebx].dib.nBlueTone[eax]
					mov	cl, [ebx].dib.nGreenTone[ecx]
					add	al, cl
					mov	ecx, [ebx].dib.nLeftWidth
					mov	BYTE PTR [edi + 2], al
				.ENDIF
			.ENDIF
			add	esi, (SIZEOF E3D_TRANS_LINE_BUF)
			add	ebp, 4
			add	edi, 4
		.ENDIF
	.ENDW
	;
Label_Exit:
	mov	ebp, [ebx].dib.dwSaveRegEBP
	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	ret

eglRenderPoly@RenderPolygon_StoreMT486	ENDP

;
;	レンダリングフィル（RGBA）関数 486 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_StoreA486	PROC	NEAR32 C

	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	mov	[ebx].dib.dwSaveRegEBP, ebp
	mov	esi, [ebx].pLineBuf[0]
	ASSUME	esi:PTR E3D_TRANS_LINE_BUF
	mov	ebp, [ebx].dib.ptrZBufLine
	mov	edi, [ebx].dib.ptrDstLine
	;
	mov	ecx, [ebx].nLineLeft[0]
	.IF	([ebx].nLeftDecimal > 0) || (ecx > [ebx].dib.rectClip.left)
		inc	ecx
		.IF	!(ecx & 01H)
			add	esi, (SIZEOF E3D_TRANS_LINE_BUF)
		.ENDIF
	.ENDIF
	cmp	ecx, [ebx].nLineRight[0]
	ja	Label_Exit
	;
	lea	ebp, [ebp + ecx * 4]
	lea	edi, [edi + ecx * 4]
	;
	fld	[esi].rZValue[0]
	fsub	[esi].rZValue[4]
	mov	eax, [esi].rZValue[0]
	fabs
	fadd	st, st(0)
	sub	eax, 04800000H
	fstp	REAL4 PTR [ebx].dib.dwTemp[0]
	.IF	(SDWORD PTR eax) > (SDWORD PTR [ebx].dib.dwTemp[0])
		mov	[ebx].dib.dwTemp[0], eax
	.ENDIF
	;
	.WHILE	ecx <= [ebx].nLineRight[0]
		.IF	!(ecx & 0001H)
			fld	[esi].rZValue[0]
			fsub	[esi].rZValue[4]
			mov	eax, [esi].rZValue[0]
			fabs
			fadd	st, st(0)
			sub	eax, 04800000H
			fst	REAL4 PTR [ebx].dib.dwTemp[0]
			.IF	(SDWORD PTR eax) > (SDWORD PTR [ebx].dib.dwTemp[0])
				mov	[ebx].dib.dwTemp[0], eax
				fstp	st(0)
				fld	REAL4 PTR [ebx].dib.dwTemp[0]
			.ENDIF
			inc	ecx
			mov	eax, [esi].rZValue[0]
			fadd	[esi].rZValue[0]
			mov	edx, [esi].rgbMul[0].dwPixelCode
			fsub	REAL4 PTR [ebp]
			and	edx, 00FFFFFFH
			fstp	REAL4 PTR [ebx].dib.dwTemp[4]
			.IF	ZERO?
				mov	edx, [esi].rgbAdd[0].dwPixelCode
				.IF	[ebx].dib.dwTemp[4] & 80000000H
					mov	DWORD PTR [edi], edx
					mov	DWORD PTR [ebp], eax
				.ENDIF
			.ELSEIF	[ebx].dib.dwTemp[4] & 80000000H
			and	edx, 00FCFCFCH
			.IF	([esi].rgbAdd[0].dwPixelCode & 00FFFFFFH) \
						|| (edx != 00FCFCFCH)
				mov	[ebx].dib.nLeftWidth, ecx
				movzx	edx, [esi].rgbMul[0].rgb.Blue
				movzx	ecx, BYTE PTR [edi]
				inc	edx
				mov	DWORD PTR [ebp], eax
				imul	ecx, edx
				movzx	eax, [esi].rgbMul[0].rgb.Green
				movzx	edx, BYTE PTR [edi + 1]
				inc	eax
				imul	edx, eax
				add	ch, [esi].rgbAdd[0].rgb.Blue
				sbb	cl, cl
				add	dh, [esi].rgbAdd[0].rgb.Green
				sbb	dl, dl
				or	cl, ch
				movzx	eax, [esi].rgbMul[0].rgb.Red
				or	dl, dh
				mov	BYTE PTR [edi], cl
				movzx	ecx, BYTE PTR [edi + 2]
				inc	eax
				mov	BYTE PTR [edi + 1], dl
				imul	eax, ecx
				add	ah, [esi].rgbAdd[0].rgb.Red
				sbb	al, al
				mov	ecx, [ebx].dib.nLeftWidth
				or	al, ah
				mov	BYTE PTR [edi + 2], al
			.ENDIF
			.ENDIF
			add	ebp, 4
			add	edi, 4
		.ELSE
			fld	REAL4 PTR [ebx].dib.dwTemp[0]
			inc	ecx
			mov	eax, [esi].rZValue[4]
			fadd	[esi].rZValue[4]
			mov	edx, [esi].rgbMul[4].dwPixelCode
			fsub	REAL4 PTR [ebp]
			and	edx, 00FFFFFFH
			fstp	REAL4 PTR [ebx].dib.dwTemp[4]
			.IF	ZERO?
				mov	edx, [esi].rgbAdd[4].dwPixelCode
				.IF	[ebx].dib.dwTemp[4] & 80000000H
					mov	DWORD PTR [edi], edx
					mov	DWORD PTR [ebp], eax
				.ENDIF
			.ELSEIF	[ebx].dib.dwTemp[4] & 80000000H
			and	edx, 00FCFCFCH
			.IF	([esi].rgbAdd[4].dwPixelCode & 00FFFFFFH) \
						|| (edx != 00FCFCFCH)
				mov	[ebx].dib.nLeftWidth, ecx
				movzx	edx, [esi].rgbMul[0].rgb.Blue
				movzx	ecx, BYTE PTR [edi]
				mov	DWORD PTR [ebp], eax
				imul	ecx, edx
				movzx	eax, [esi].rgbMul[4].rgb.Green
				movzx	edx, BYTE PTR [edi + 1]
				imul	edx, eax
				add	ch, [esi].rgbAdd[4].rgb.Blue
				sbb	cl, cl
				add	dh, [esi].rgbAdd[4].rgb.Green
				sbb	dl, dl
				or	cl, ch
				movzx	eax, [esi].rgbMul[4].rgb.Red
				or	dl, dh
				mov	BYTE PTR [edi], cl
				movzx	ecx, BYTE PTR [edi + 2]
				mov	BYTE PTR [edi + 1], dl
				imul	eax, ecx
				add	ah, [esi].rgbAdd[4].rgb.Red
				sbb	al, al
				mov	ecx, [ebx].dib.nLeftWidth
				or	al, ah
				mov	BYTE PTR [edi + 2], al
			.ENDIF
			.ENDIF
			add	esi, (SIZEOF E3D_TRANS_LINE_BUF)
			add	ebp, 4
			add	edi, 4
		.ENDIF
	.ENDW
	;
Label_Exit:
	mov	ebp, [ebx].dib.dwSaveRegEBP
	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	ret

eglRenderPoly@RenderPolygon_StoreA486	ENDP

;
;	レンダリング透明度付フィル（RGBA）関数 486 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_StoreAT486	PROC	NEAR32 C

	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	mov	[ebx].dib.dwSaveRegEBP, ebp
	mov	esi, [ebx].pLineBuf[0]
	ASSUME	esi:PTR E3D_TRANS_LINE_BUF
	mov	ebp, [ebx].dib.ptrZBufLine
	mov	edi, [ebx].dib.ptrDstLine
	;
	mov	ecx, [ebx].nLineLeft[0]
	.IF	([ebx].nLeftDecimal > 0) || (ecx > [ebx].dib.rectClip.left)
		inc	ecx
		.IF	!(ecx & 01H)
			add	esi, (SIZEOF E3D_TRANS_LINE_BUF)
		.ENDIF
	.ENDIF
	cmp	ecx, [ebx].nLineRight[0]
	ja	Label_Exit
	;
	lea	ebp, [ebp + ecx * 4]
	lea	edi, [edi + ecx * 4]
	;
	fld	[esi].rZValue[0]
	fsub	[esi].rZValue[4]
	mov	eax, [esi].rZValue[0]
	fabs
	fadd	st, st(0)
	sub	eax, 04800000H
	fstp	REAL4 PTR [ebx].dib.dwTemp[0]
	.IF	(SDWORD PTR eax) > (SDWORD PTR [ebx].dib.dwTemp[0])
		mov	[ebx].dib.dwTemp[0], eax
	.ENDIF
	;
	.WHILE	ecx <= [ebx].nLineRight[0]
		.IF	!(ecx & 0001H)
			fld	[esi].rZValue[0]
			fsub	[esi].rZValue[4]
			mov	eax, [esi].rZValue[0]
			fabs
			fadd	st, st(0)
			sub	eax, 04800000H
			fst	REAL4 PTR [ebx].dib.dwTemp[0]
			.IF	(SDWORD PTR eax) > (SDWORD PTR [ebx].dib.dwTemp[0])
				mov	[ebx].dib.dwTemp[0], eax
				fstp	st(0)
				fld	REAL4 PTR [ebx].dib.dwTemp[0]
			.ENDIF
			inc	ecx
			mov	eax, [esi].rZValue[0]
			fadd	[esi].rZValue[0]
			mov	edx, [esi].rgbMul[0].dwPixelCode
			fsub	REAL4 PTR [ebp]
			and	edx, 00FCFCFCH
			fstp	REAL4 PTR [ebx].dib.dwTemp[4]
			.IF	([ebx].dib.dwTemp[4] & 80000000H) \
				&& ((edx != 00FCFCFCH) || \
					([esi].rgbAdd[0].dwPixelCode & 00FFFFFFH))
				mov	[ebx].dib.nLeftWidth, ecx
				mov	DWORD PTR [ebp], eax
				@INDEX = 0
				FOR	@MEMBER, <Blue, Green, Red>
					movzx	edx, [esi].rgbMul[0].rgb.@MEMBER
					movzx	ecx, [esi].rgbAdd[0].rgb.@MEMBER
					xor	edx, 0FFH
					mov	cl, [ebx].dib.nGreenTone[ecx]
					movzx	edx, [ebx].dib.nGreenTone[edx]
					movzx	eax, BYTE PTR [edi + @INDEX]
					xor	edx, 0FFH
					imul	eax, edx
					add	ah, cl
					sbb	al, al
					or	al, ah
					mov	BYTE PTR [edi + @INDEX], al
					@INDEX = @INDEX + 1
				ENDM
				mov	ecx, [ebx].dib.nLeftWidth
			.ENDIF
			add	ebp, 4
			add	edi, 4
		.ELSE
			fld	REAL4 PTR [ebx].dib.dwTemp[0]
			inc	ecx
			fadd	[esi].rZValue[4]
			mov	eax, [esi].rZValue[4]
			mov	edx, [esi].rgbMul[4].dwPixelCode
			fsub	REAL4 PTR [ebp]
			and	edx, 00FCFCFCH
			fstp	REAL4 PTR [ebx].dib.dwTemp[4]
			.IF	([ebx].dib.dwTemp[4] & 80000000H) \
				&& ((edx != 00FCFCFCH) || \
					([esi].rgbAdd[4].dwPixelCode & 00FFFFFFH))
				mov	[ebx].dib.nLeftWidth, ecx
				mov	DWORD PTR [ebp], eax
				@INDEX = 0
				FOR	@MEMBER, <Blue, Green, Red>
					movzx	edx, [esi].rgbMul[4].rgb.@MEMBER
					movzx	ecx, [esi].rgbAdd[4].rgb.@MEMBER
					xor	edx, 0FFH
					mov	cl, [ebx].dib.nGreenTone[ecx]
					movzx	edx, [ebx].dib.nGreenTone[edx]
					movzx	eax, BYTE PTR [edi + @INDEX]
					xor	edx, 0FFH
					imul	eax, edx
					add	ah, cl
					sbb	al, al
					or	al, ah
					mov	BYTE PTR [edi + @INDEX], al
					@INDEX = @INDEX + 1
				ENDM
				mov	ecx, [ebx].dib.nLeftWidth
			.ENDIF
			add	esi, (SIZEOF E3D_TRANS_LINE_BUF)
			add	ebp, 4
			add	edi, 4
		.ENDIF
	.ENDW
	;
Label_Exit:
	mov	ebp, [ebx].dib.dwSaveRegEBP
	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	ret

eglRenderPoly@RenderPolygon_StoreAT486	ENDP

;
;	アンチエイリアス関数 486 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_Antialias486	PROC	NEAR32 C

	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	mov	[ebx].dib.dwSaveRegEBP, ebp
	mov	esi, [ebx].pLineBuf[0]
	ASSUME	esi:PTR E3D_TRANS_LINE_BUF
	mov	ebp, [ebx].dib.ptrZBufLine
	mov	edi, [ebx].dib.ptrDstLine
	;
	mov	ecx, [ebx].nLineLeft[0]
	inc	ecx
	.IF	(SDWORD PTR ecx) <= (SDWORD PTR [ebx].nLineRight[0])
		;
		; 左側アンチエイリアス
		;
		.IF	(SDWORD PTR ecx) > [ebx].dib.rectClip.left
			lea	ebp, [ebp + ecx * 4]
			lea	edi, [edi + ecx * 4 - 4]
			;
			mov	edx, ecx
			and	ecx, NOT 01H
			sub	ecx, [ebx].nLineLeft[4]
			mov	eax, [ebx].nLeftDecimal
			shr	ecx, 1
			IF	(SIZEOF E3D_TRANS_LINE_BUF) NE 24
				.ERR
			ENDIF
			xor	eax, 0FFFFH
			lea	ecx, [ecx + ecx * 2]
			and	edx, 01H
			lea	esi, [esi + ecx * 8]
			mov	ecx, DWORD PTR [esi].rZValue[edx * 4]
			shr	eax, 8
			.IF	(DWORD PTR [ebp]) == edx
				lea	esi, [esi].rgbAdd[ecx * 4]
				@INDEX = 0
				REPEAT	3
					movzx	edx, BYTE PTR [edi + @INDEX]
					movzx	ecx, BYTE PTR [esi + @INDEX]
					sub	ecx, edx
					imul	ecx, eax
					add	ch, dl
					mov	BYTE PTR [edi + @INDEX], ch
					@INDEX = @INDEX + 1
				ENDM
			.ENDIF
		.ENDIF
		;
		; 右側アンチエイリアス
		;
		mov	ebp, [ebx].dib.ptrZBufLine
		mov	edi, [ebx].dib.ptrDstLine
		mov	esi, [ebx].pLineBuf[0]
		mov	ecx, [ebx].nLineRight[0]
		mov	eax, [ebx].nRightDecimal
		lea	ebp, [ebp + ecx * 4]
		lea	edi, [edi + ecx * 4 + 4]
		;
		.IF	ecx < [ebx].dib.rectClip.right
			mov	edx, ecx
			and	ecx, NOT 01H
			sub	ecx, [ebx].nLineLeft[4]
			shr	ecx, 1
			IF	(SIZEOF E3D_TRANS_LINE_BUF) NE 24
				.ERR
			ENDIF
			lea	ecx, [ecx + ecx * 2]
			and	edx, 01H
			lea	esi, [esi + ecx * 8]
			mov	ecx, DWORD PTR [esi].rZValue[edx * 4]
			shr	eax, 8
			.IF	(DWORD PTR [ebp]) == ecx
				lea	esi, [esi].rgbAdd[edx * 4]
				@INDEX = 0
				REPEAT	3
					movzx	edx, BYTE PTR [edi + @INDEX]
					movzx	ecx, BYTE PTR [esi + @INDEX]
					sub	ecx, edx
					imul	ecx, eax
					add	ch, dl
					mov	BYTE PTR [edi + @INDEX], ch
					@INDEX = @INDEX + 1
				ENDM
			.ENDIF
		.ENDIF
	.ENDIF

	mov	ebp, [ebx].dib.dwSaveRegEBP
	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	ret

eglRenderPoly@RenderPolygon_Antialias486	ENDP


CodeSeg	ENDS

	END
