
; ----------------------------------------------------------------------------
;                          Entis Graphic Library
; ----------------------------------------------------------------------------
;   Copyright (c) 2002-2009 Leshade Entis, Entis-soft. Al rights reserved.
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
rConstLittle	DWORD	4 DUP( 33800000H )	; = 2^-24
rConstSinLittle	DWORD	4 DUP( 3B800000H )	; = 1/256 ≒ sin(0.23[deg])
xmmMaskLS1	DWORD	80000000H, 0, 0, 0
xmmMaskSign	DWORD	4 DUP( 80000000H )
xmmMaskSign_pd	QWORD	2 DUP( 8000000000000000H )
xmmMaskAbs	DWORD	4 DUP( 7FFFFFFFH )
mmxFEFF		WORD	4 DUP( 0FEFFH )

ALIGN	10H
pfnPixelShaderTextureNoShade	LABEL	PRenderPoly@RenderPolygonSub
	; index = 発光テクスチャ：タイリング：補間：アルファチャネル
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineNS_ATX_SSE	; 0000
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineNS_ATX_SSE	; 0001
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineNS_SAX_SSE	; 0010
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineNS_SAX_SSE	; 0011
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineNS_TATX_SSE	; 0100
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineNS_TATX_SSE	; 0101
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineNS_STAX_SSE	; 0110
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineNS_STAX_SSE	; 0111
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineNS_ATX_SSE	; o000
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineNS_ATX_SSE	; o001
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineNS_SAX_SSE	; o010
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineNS_SAX_SSE	; o011
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineNS_TATX_SSE	; o100
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineNS_TATX_SSE	; o101
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineNS_STAX_SSE	; o110
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineNS_STAX_SSE	; o111

	; index = SSE2 : 発光テクスチャ：タイリング：補間：アルファチャネル
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineNS_ATX_SSE2	; 10000
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineNS_ATX_SSE2	; 10001
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineNS_SAX_SSE2	; 10010
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineNS_SAX_SSE2	; 10011
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineNS_TATX_SSE2	; 10100
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineNS_TATX_SSE2	; 10101
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineNS_STAX_SSE2	; 10110
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineNS_STAX_SSE2	; 10111
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineNS_ATX_SSE2	; 1o000
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineNS_ATX_SSE2	; 1o001
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineNS_SAX_SSE2	; 1o010
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineNS_SAX_SSE2	; 1o011
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineNS_TATX_SSE2	; 1o100
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineNS_TATX_SSE2	; 1o101
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineNS_STAX_SSE2	; 1o110
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineNS_STAX_SSE2	; 1o111

ALIGN	10H
pfnPixelShaderTextureGouraud	LABEL	PRenderPoly@RenderPolygonSub
	; index = 発光テクスチャ：タイリング：補間：アルファチャネル
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineTXSSE	; 0000
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineATXSSE	; 0001
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineSXSSE	; 0010
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineSAXSSE	; 0011
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineTTXSSE	; 0100
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineTATXSSE	; 0101
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineSTXSSE	; 0110
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineSTAXSSE	; 0111
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineTXSSE	; o000
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineATXSSE	; o001
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineSXSSE	; o010
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineSAXSSE	; o011
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineTLTXSSE	; 1100
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineTATXSSE	; o101
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineSTLXSSE	; 1110
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineSTAXSSE	; o111

	; index = SSE2: 発光テクスチャ：タイリング：補間：アルファチャネル
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineTXSSE2	; 10000
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineATXSSE2	; 10001
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineSXSSE2	; 10010
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineSAXSSE2	; 10011
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineTTXSSE2	; 10100
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineTATXSSE2	; 10101
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineSTXSSE2	; 10110
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineSTAXSSE2	; 10111
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineTXSSE2	; 1o000
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineATXSSE2	; 1o001
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineSXSSE2	; 1o010
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineSAXSSE2	; 1o011
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineTLTXSSE2	; 11100
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineTATXSSE2	; 1o101
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineSTLXSSE2	; 11110
	DWORD	OFFSET eglRenderPoly@RenderPolygon_LineSTAXSSE2	; 1o111

ALIGN	10H
pfnPixelRenderTextureFuncTable	LABEL	PRenderPoly@RenderPolygonSub
	; index = ｚバッファ：透明度：アルファチャネル：トリム
	DWORD	OFFSET eglRenderPoly@RenderPolygon_StoreSSEC	; 0000
	DWORD	RPT_RENDER_POLY_NORMAL
	DWORD	OFFSET eglRenderPoly@RenderPolygon_StoreSSEC	; 0001
	DWORD	RPT_RENDER_POLY_NORMAL
	DWORD	OFFSET eglRenderPoly@RenderPolygon_StoreASSE	; 0010
	DWORD	RPT_RENDER_POLY_NORMAL
	DWORD	OFFSET eglRenderPoly@RenderPolygon_StoreMSSE	; 0011
	DWORD	RPT_RENDER_POLY_NORMAL
	DWORD	OFFSET eglRenderPoly@RenderPolygon_StoreTSSE	; 0100
	DWORD	RPT_RENDER_POLY_NORMAL
	DWORD	OFFSET eglRenderPoly@RenderPolygon_StoreTSSE	; 0101
	DWORD	RPT_RENDER_POLY_NORMAL
	DWORD	OFFSET eglRenderPoly@RenderPolygon_StoreATSSE	; 0110
	DWORD	RPT_RENDER_POLY_NORMAL
	DWORD	OFFSET eglRenderPoly@RenderPolygon_StoreMTSSE	; 0111
	DWORD	RPT_RENDER_POLY_NORMAL

	; index = ｚバッファ無し：透明度：アルファチャネル：トリム
	DWORD	OFFSET eglRenderPoly@RenderPolygon_StoreNZSSE	; 1000
	DWORD	RPT_RENDER_POLY_NO_ZBUF
	DWORD	OFFSET eglRenderPoly@RenderPolygon_StoreNZSSE	; 1001
	DWORD	RPT_RENDER_POLY_NO_ZBUF
	DWORD	OFFSET eglRenderPoly@RenderPolygon_StoreANZSSE	; 1010
	DWORD	RPT_RENDER_POLY_NO_ZBUF
	DWORD	OFFSET eglRenderPoly@RenderPolygon_StoreMSSE	; 1011
	DWORD	RPT_RENDER_POLY_V_NO_Z
	DWORD	OFFSET eglRenderPoly@RenderPolygon_StoreTNZSSE	; 1100
	DWORD	RPT_RENDER_POLY_NO_ZBUF
	DWORD	OFFSET eglRenderPoly@RenderPolygon_StoreTNZSSE	; 1101
	DWORD	RPT_RENDER_POLY_NO_ZBUF
	DWORD	OFFSET eglRenderPoly@RenderPolygon_StoreATSSE	; 1110
	DWORD	RPT_RENDER_POLY_V_NO_Z
	DWORD	OFFSET eglRenderPoly@RenderPolygon_StoreMTSSE	; 1111
	DWORD	RPT_RENDER_POLY_V_NO_Z

	; index = ｚ比較のみ：透明度：アルファチャネル：トリム
	DWORD	OFFSET eglRenderPoly@RenderPolygon_StoreSSEC	; 10000
	DWORD	RPT_RENDER_POLY_CMP_Z
	DWORD	OFFSET eglRenderPoly@RenderPolygon_StoreSSEC	; 10001
	DWORD	RPT_RENDER_POLY_CMP_Z
	DWORD	OFFSET eglRenderPoly@RenderPolygon_StoreARZSSE	; 10010
	DWORD	RPT_RENDER_POLY_NORMAL
	DWORD	OFFSET eglRenderPoly@RenderPolygon_StoreMSSE	; 10011
	DWORD	RPT_RENDER_POLY_CMP_Z
	DWORD	OFFSET eglRenderPoly@RenderPolygon_StoreTSSE	; 10100
	DWORD	RPT_RENDER_POLY_CMP_Z
	DWORD	OFFSET eglRenderPoly@RenderPolygon_StoreTSSE	; 10101
	DWORD	RPT_RENDER_POLY_CMP_Z
	DWORD	OFFSET eglRenderPoly@RenderPolygon_StoreATSSE	; 10110
	DWORD	RPT_RENDER_POLY_CMP_Z
	DWORD	OFFSET eglRenderPoly@RenderPolygon_StoreMTSSE	; 10111
	DWORD	RPT_RENDER_POLY_CMP_Z

ALIGN	10H
pfnRenderFuncTableSSE	LABEL	PRenderPoly@RenderPolygon
	DWORD	OFFSET eglRenderPoly@RenderPolygon_NoDraw
	DWORD	OFFSET eglRenderPoly@RenderPolygon_TextureSSE
	DWORD	OFFSET eglRenderPoly@RenderPolygon_TxtNZSSE
	DWORD	OFFSET eglRenderPoly@RenderPolygon_TxtRZSSE
	DWORD	OFFSET eglRenderPoly@RenderPolygon_TxtVNZSSE

ConstSeg	ENDS


; ----------------------------------------------------------------------------
;	コードセグメント
; ----------------------------------------------------------------------------

CodeSeg	SEGMENT	PARA READONLY FLAT 'CODE'

@RenderPoly@PrepareTextureSSE	MACRO	_x:=<x>, _y:=<y>, _z:=<z>
	;
	;	パラメータ計算 SSE2 専用コード
	; ------------------------------------------------------------
	;
	; ｘ座標差分
	;	x ; vTxxy.x * ( vTxo.z * vTxy.y - vTxo.y * vTxy.z )
	;	y ; vTxxy.x * ( vTxo.y * vTxx.z - vTxo.z * vTxx.y )
	; ｙ座標差分
	;	x ; vTxxy.y * ( vTxo.z * vTxy.y - vTxo.y * vTxy.z )
	;	y ; vTxxy.y * ( vTxo.y * vTxx.z - vTxo.z * vTxx.y )
	;
	movss	xmm0, [ebx].vTxo._z
	movss	xmm1, [ebx].vTxo._y
	movss	xmm2, [ebx].vTxy._y
	movss	xmm3, [ebx].vTxx._z
	mulss	xmm2, xmm0
	mulss	xmm3, xmm1
	movss	xmm4, [ebx].vTxy._z
	movss	xmm5, [ebx].vTxx._y
	mulss	xmm4, xmm1
	mulss	xmm5, xmm0
	movss	xmm0, [ebx].vTxxy.x
	movss	xmm1, [ebx].vTxxy.y
	subss	xmm2, xmm4
	shufps	xmm0, xmm0, 0
	subss	xmm3, xmm5
	shufps	xmm1, xmm1, 0
	unpcklps	xmm2, xmm3
	mulps	xmm0, xmm2
	mulps	xmm1, xmm2
	IFIDNI		<_x>, <y>
		movss	xmm2, [ebx].vTxy.z
		movss	xmm3, [ebx].vTxx.z
		movss	xmm5, xmmMaskSign
		movss	xmm4, [ebx].rTxoxy
		xorps	xmm2, xmm5
		shufps	xmm4, xmm4, 0
		unpcklps	xmm2, xmm3
		mulps	xmm2, xmm4
		addps	xmm0, xmm2
	ELSEIFIDNI	<_x>, <z>
		movss	xmm2, [ebx].vTxy.y
		movss	xmm3, [ebx].vTxx.y
		movss	xmm5, xmmMaskSign
		movss	xmm4, [ebx].rTxoxy
		xorps	xmm3, xmm5
		shufps	xmm4, xmm4, 0
		unpcklps	xmm2, xmm3
		mulps	xmm2, xmm4
		addps	xmm0, xmm2
	ENDIF
	IFIDNI		<_x>, <x>
		movss	xmm2, [ebx].vTxy.z
		movss	xmm3, [ebx].vTxx.z
		movss	xmm5, xmmMaskSign
		movss	xmm4, [ebx].rTxoxy
		xorps	xmm3, xmm5
		shufps	xmm4, xmm4, 0
		unpcklps	xmm2, xmm3
		mulps	xmm2, xmm4
		addps	xmm1, xmm2
	ELSEIFIDNI	<_x>, <z>
		movss	xmm2, [ebx].vTxy.x
		movss	xmm3, [ebx].vTxx.x
		movss	xmm5, xmmMaskSign
		movss	xmm4, [ebx].rTxoxy
		xorps	xmm2, xmm5
		shufps	xmm4, xmm4, 0
		unpcklps	xmm2, xmm3
		mulps	xmm2, xmm4
		addps	xmm1, xmm2
	ENDIF
	movlps	QWORD PTR [ebx].vTxDeltaX, xmm0
	movlps	QWORD PTR [ebx].vTxDeltaY, xmm1
	;
	; 母数の差分
	;
	movss	xmm1, [ebx].vTxxy._x
	movss	xmm0, rConst1
	movss	[ebx].rDeltaTxxy, xmm1
	divss	xmm0, xmm1
	movss	[ebx].rRcpTxxy, xmm0
	;
	; 基準座標の計算
	;	X0 = (vTxBase, vTxxy)		; 内積
	;	X1 = rTxoxy * vTxBase.y - X0 * vTxo.y
	;	X2 = rTxoxy * vTxBase.z - X0 * vTxo.z
	;	x' = (X1 * vTxy.z - X2 * vTxy.y) / (X0 * vTxxy.x)
	;	y' = (X2 * vTxx.y - X1 * vTxx.z) / (X0 * vTxxy.x)
	;
	xorps	xmm0, xmm0
	mov	edi, [ebx].dib.pRegion
	xorps	xmm1, xmm1
	ASSUME	edi:PTR E3D_POLYGON_REGION
	cvtsi2ss	xmm0, [edi].nTopLine
	ASSUME	edi:NOTHING
	movlps	xmm1, QWORD PTR [ebx].vScreenPos.x
	shufps	xmm0, xmm0, 11100001B
	movss	xmm2, [ebx].vScreenPos.z
	subps	xmm0, xmm1		; xmm0 = ( x, y, r )
	movaps	xmm1, [ebx].vTxxy
	movlhps	xmm0, xmm2
	movaps	[ebx].vTxBase, xmm0
	mulps	xmm0, xmm1
	movaps	xmm1, xmm0
	shufps	xmm0, xmm0, 1
	movhlps	xmm2, xmm1
	addss	xmm1, xmm0
	movss	xmm0, [ebx].rTxoxy	; xmm0 = rTxoxy
	addss	xmm1, xmm2		; xmm1 = X0
	movss	[ebx].rTxtxy, xmm1
	;
	movss	xmm2, [ebx].vTxBase._y
	movss	xmm3, [ebx].vTxBase._z
	mulss	xmm2, xmm0
	mulss	xmm3, xmm0
	movss	xmm4, [ebx].vTxo._y
	movss	xmm5, [ebx].vTxo._z
	mulss	xmm4, xmm1
	mulss	xmm5, xmm1
	subss	xmm2, xmm4		; xmm2 = X1
	subss	xmm3, xmm5		; xmm3 = X2
	;
	movss	xmm4, [ebx].vTxy._z
	movss	xmm5, [ebx].vTxx._y
	mulss	xmm4, xmm2
	mulss	xmm5, xmm3
	movss	xmm6, [ebx].vTxy._y
	movss	xmm7, [ebx].vTxx._z
	mulss	xmm6, xmm3
	mulss	xmm7, xmm2
	subss	xmm4, xmm6
	subss	xmm5, xmm7
	unpcklps	xmm4, xmm5
	movlps	QWORD PTR [ebx].vTxBasePos, xmm4
ENDM

;
;	テクスチャマッピング差分パラメータ算出マクロ
; ----------------------------------------------------------------------------
@RenderPoly@PrepareTextureSSE2	MACRO	_x:=<x>, _y:=<y>, _z:=<z>
	;
	;	パラメータ計算 SSE 専用コード
	; ------------------------------------------------------------
	;
	; ｘ座標差分
	;	x ; vTxxy.x * ( vTxo.z * vTxy.y - vTxo.y * vTxy.z )
	;	y ; vTxxy.x * ( vTxo.y * vTxx.z - vTxo.z * vTxx.y )
	; ｙ座標差分
	;	x ; vTxxy.y * ( vTxo.z * vTxy.y - vTxo.y * vTxy.z )
	;	y ; vTxxy.y * ( vTxo.y * vTxx.z - vTxo.z * vTxx.y )
	;
	cvtss2sd	xmm0, [ebx].vTxo._z
	cvtss2sd	xmm1, [ebx].vTxo._y
	cvtss2sd	xmm2, [ebx].vTxy._y
	cvtss2sd	xmm3, [ebx].vTxx._z
	mulsd		xmm2, xmm0
	mulsd		xmm3, xmm1
	cvtss2sd	xmm4, [ebx].vTxy._z
	cvtss2sd	xmm5, [ebx].vTxx._y
	mulsd		xmm4, xmm1
	mulsd		xmm5, xmm0
	_movsd		xmm0, [ebx].vTxxy_d.x
	_movsd		xmm1, [ebx].vTxxy_d.y
	subsd		xmm2, xmm4
	shufpd		xmm0, xmm0, 0
	subsd		xmm3, xmm5
	shufpd		xmm1, xmm1, 0
	unpcklpd	xmm2, xmm3
	mulpd		xmm0, xmm2
	mulpd		xmm1, xmm2
	IFIDNI		<_x>, <y>
		cvtss2sd	xmm2, [ebx].vTxy.z
		cvtss2sd	xmm3, [ebx].vTxx.z
		_movsd		xmm5, xmmMaskSign_pd
		_movsd		xmm4, [ebx].rTxoxy_d
		xorpd		xmm2, xmm5
		shufpd		xmm4, xmm4, 0
		unpcklpd	xmm2, xmm3
		mulpd		xmm2, xmm4
		addpd		xmm0, xmm2
	ELSEIFIDNI	<_x>, <z>
		cvtss2sd	xmm2, [ebx].vTxy.y
		cvtss2sd	xmm3, [ebx].vTxx.y
		_movsd		xmm5, xmmMaskSign_pd
		_movsd		xmm4, [ebx].rTxoxy_d
		xorpd		xmm3, xmm5
		shufpd		xmm4, xmm4, 0
		unpcklpd	xmm2, xmm3
		mulpd		xmm2, xmm4
		addpd		xmm0, xmm2
	ENDIF
	IFIDNI		<_x>, <x>
		cvtss2sd	xmm2, [ebx].vTxy.z
		cvtss2sd	xmm3, [ebx].vTxx.z
		_movsd		xmm5, xmmMaskSign_pd
		_movsd		xmm4, [ebx].rTxoxy_d
		xorpd		xmm3, xmm5
		shufpd		xmm4, xmm4, 0
		unpcklpd	xmm2, xmm3
		mulpd		xmm2, xmm4
		addpd		xmm1, xmm2
	ELSEIFIDNI	<_x>, <z>
		cvtss2sd	xmm2, [ebx].vTxy.x
		cvtss2sd	xmm3, [ebx].vTxx.x
		_movsd		xmm5, xmmMaskSign_pd
		_movsd		xmm4, [ebx].rTxoxy_d
		xorpd		xmm2, xmm5
		shufpd		xmm4, xmm4, 0
		unpcklpd	xmm2, xmm3
		mulpd		xmm2, xmm4
		addpd		xmm1, xmm2
	ENDIF
	movapd		[ebx].vTxDeltaX_d, xmm0
	movapd		[ebx].vTxDeltaY_d, xmm1
	cvtpd2ps	xmm0, xmm0
	cvtpd2ps	xmm1, xmm1
	movlps		QWORD PTR [ebx].vTxDeltaX, xmm0
	movlps		QWORD PTR [ebx].vTxDeltaY, xmm1
	;
	; 母数の差分
	;
	_movsd	xmm1, [ebx].vTxxy_d.&_x
	_movsd	xmm0, rConst1_pd
	cvtsd2ss	xmm2, xmm1
	_movsd	[ebx].rDeltaTxxy_d, xmm1
	movss	[ebx].rDeltaTxxy, xmm2
	divsd	xmm0, xmm1
	_movsd	[ebx].rRcpTxxy_d, xmm0
	cvtsd2ss	xmm1, xmm0
	movss	[ebx].rRcpTxxy, xmm1
	;
	; 基準座標の計算
	;	X0 = (vTxBase, vTxxy)		; 内積
	;	X1 = rTxoxy * vTxBase.y - X0 * vTxo.y
	;	X2 = rTxoxy * vTxBase.z - X0 * vTxo.z
	;	x' = (X1 * vTxy.z - X2 * vTxy.y) / (X0 * vTxxy.x)
	;	y' = (X2 * vTxx.y - X1 * vTxx.z) / (X0 * vTxxy.x)
	;
	xorpd		xmm0, xmm0
	mov		edi, [ebx].dib.pRegion
	ASSUME		edi:PTR E3D_POLYGON_REGION
	cvtsi2sd	xmm0, [edi].nTopLine
	ASSUME		edi:NOTHING
	cvtps2pd	xmm1, QWORD PTR [ebx].vScreenPos.x
	shufpd		xmm0, xmm0, 00000001B
	movss		xmm5, [ebx].vScreenPos.z
	subpd		xmm0, xmm1		; xmm2:xmm0 = ( x, y, r )
	cvtss2sd	xmm2, xmm5
	movapd		xmm1, [ebx].vTxxy_d.x
	_movsd		xmm3, [ebx].vTxxy_d.z
	;
	cvtpd2ps	xmm4, xmm0
	movlhps		xmm4, xmm5
	movaps		[ebx].vTxBase, xmm4
	;
	mulpd		xmm1, xmm0
	mulsd		xmm2, xmm3
	_movsd		xmm0, [ebx].rTxoxy_d	; xmm0 = rTxoxy_d
	addsd		xmm2, xmm1
	shufpd		xmm1, xmm1, 1
	addsd		xmm1, xmm2		; xmm1 = X0
	_movsd		[ebx].rTxtxy_d, xmm1
	cvtsd2ss	xmm2, xmm1
	movss		[ebx].rTxtxy, xmm2
	;
	cvtss2sd	xmm2, [ebx].vTxBase._y
	cvtss2sd	xmm3, [ebx].vTxBase._z
	mulsd		xmm2, xmm0
	mulsd		xmm3, xmm0
	cvtss2sd	xmm4, [ebx].vTxo._y
	cvtss2sd	xmm5, [ebx].vTxo._z
	mulsd		xmm4, xmm1
	mulsd		xmm5, xmm1
	subsd		xmm2, xmm4		; xmm2 = X1
	subsd		xmm3, xmm5		; xmm3 = X2
	;
	cvtss2sd	xmm4, [ebx].vTxy._z
	cvtss2sd	xmm5, [ebx].vTxx._y
	mulsd		xmm4, xmm2
	mulsd		xmm5, xmm3
	cvtss2sd	xmm6, [ebx].vTxy._y
	cvtss2sd	xmm7, [ebx].vTxx._z
	mulsd		xmm6, xmm3
	mulsd		xmm7, xmm2
	subsd		xmm4, xmm6
	subsd		xmm5, xmm7
	unpcklpd	xmm4, xmm5
	movapd		[ebx].vTxBasePos_d, xmm4
	cvtpd2ps	xmm4, xmm4
	movlps		QWORD PTR [ebx].vTxBasePos, xmm4
ENDM

;
;	画像バッファセットアップ
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@PrepareImageBuffer	PROC	NEAR32 C USES edi,
	pAttr:PE3D_SURFACE_ATTRIBUTE,
	pTexture:PEGL_IMAGE_INFO, pLuminousImage:PEGL_IMAGE_INFO

	LOCAL	rect:EGL_IMAGE_RECT

	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	;
	; 出力先画像バッファ設定
	;
	mov	edi, [ebx].dib.pDstImage
	ASSUME	edi:PTR EGL_IMAGE_INFO
	mov	eax, [edi].dwImageWidth
	mov	edx, [edi].dwImageHeight
	ASSUME	edi:NOTHING
	mov	rect.x, 0
	mov	rect.y, 0
	mov	rect.w, eax
	mov	rect.h, edx
	;
	INVOKE	eglGetClippedImageInfo ,
			ADDR [ebx].dib.dstimg, edi, ADDR rect
	;
	; ｚバッファ設定
	;
	mov	edx, [ebx].dib.pZBuffer
	mov	eax, pAttr
	ASSUME	eax:PE3D_SURFACE_ATTRIBUTE
	.IF	(eax != NULL) && ([eax].dwShadingFlags & E3DSAF_NO_ZBUFFER)
		xor	edx, edx
	.ENDIF
	.IF	edx != NULL
		INVOKE	eglGetClippedImageInfo ,
			ADDR [ebx].dib.zbuf, edx, ADDR rect
	.ELSE
		mov	[ebx].dib.zbuf.dwInfoSize, (SIZEOF EGL_IMAGE_INFO)
		mov	[ebx].dib.zbuf.fdwFormatType, EIF_Z_BUFFER_R4
		mov	[ebx].dib.zbuf.ptrOffsetPixel, 0
		mov	eax, [ebx].pTempZBuffer[0]
		mov	[ebx].dib.zbuf.ptrImageArray, eax
		mov	[ebx].dib.zbuf.pPaletteEntries, 0
		mov	[ebx].dib.zbuf.dwPaletteCount, 0
		mov	eax, rect.w
		mov	edx, rect.h
		mov	[ebx].dib.zbuf.dwImageWidth, eax
		mov	[ebx].dib.zbuf.dwImageHeight, edx
		mov	[ebx].dib.zbuf.dwBitsPerPixel, 32
		mov	[ebx].dib.zbuf.dwBytesPerLine, 0
		mov	[ebx].dib.zbuf.dwSizeOfImage, 0
		mov	[ebx].dib.zbuf.dwClippedPixel, 0
	.ENDIF
	;
	; テクスチャ設定
	;
	mov	edi, pTexture
	ASSUME	edi:PTR EGL_IMAGE_BUFF
	.IF	edi != NULL
		.IF	([edi].dwInfoSize != (SIZEOF EGL_IMAGE_BUFF)) \
					|| ([edi].dwBitsPerPixel != 32)
			mov	eax, eslErrInvalidParam
			ret
		.ENDIF
		mov	eax, [edi].dwImageWidth
		mov	edx, [edi].dwImageHeight
		mov	rect.x, 0
		mov	rect.y, 0
		mov	rect.w, eax
		mov	rect.h, edx
		INVOKE	eglGetClippedImageInfo ,
				ADDR [ebx].txtimg, edi, ADDR rect
		;
		mov	ecx, [edi].pLineAddrEntry
		mov	eax, [edi].dwWidthMask
		mov	edx, [edi].dwHeightMask
		mov	[ebx].pTxLineAddr, ecx
		mov	[ebx].txSizeMask[0], ax
		mov	[ebx].txSizeMask[2], dx
		mov	[ebx].txSizeMask[4], ax
		mov	[ebx].txSizeMask[6], dx
		mov	eax, [edi].dwImageWidth
		mov	edx, [edi].dwImageHeight
		dec	eax
		dec	edx
		not	eax
		not	edx
		mov	[ebx].txSizeMask[8], ax
		mov	[ebx].txSizeMask[10], dx
		mov	[ebx].txSizeMask[12], ax
		mov	[ebx].txSizeMask[14], dx
		mov	[ebx].txSizeMaskDW.w, eax
		mov	[ebx].txSizeMaskDW.h, edx
		mov	eax, [edi].dwBitsPerPixel
		mov	edx, [edi].dwBytesPerLine
		shl	edx, 16
		shr	eax, 3
		or	eax, edx
		mov	DWORD PTR [ebx].txMulAddr[0], eax
		mov	DWORD PTR [ebx].txMulAddr[4], eax
		mov	eax, [edi].ptrImageArray
		mov	[ebx].txImageAddr[0], eax
		mov	[ebx].txImageAddr[4], eax
		;
		; 発光テクスチャ画像設定
		;
		mov	edi, pLuminousImage
		.IF	edi != NULL
			.IF	([edi].dwInfoSize != (SIZEOF EGL_IMAGE_BUFF)) \
						|| ([edi].dwBitsPerPixel != 32)
				mov	eax, eslErrInvalidParam
				ret
			.ENDIF
			mov	eax, [edi].dwImageWidth
			mov	edx, [edi].dwImageHeight
			mov	rect.x, 0
			mov	rect.y, 0
			mov	rect.w, eax
			mov	rect.h, edx
			INVOKE	eglGetClippedImageInfo ,
					ADDR [ebx].lmnimg, edi, ADDR rect
			;
			mov	ecx, [edi].pLineAddrEntry
			mov	eax, [edi].dwWidthMask
			mov	edx, [edi].dwHeightMask
			mov	[ebx].pLmLineAddr, ecx
			mov	[ebx].lmSizeMask[0], ax
			mov	[ebx].lmSizeMask[2], dx
			mov	[ebx].lmSizeMask[4], ax
			mov	[ebx].lmSizeMask[6], dx
			mov	[ebx].lmSizeMaskDW.w, eax
			mov	[ebx].lmSizeMaskDW.h, edx
			mov	eax, [edi].dwImageWidth
			mov	edx, [edi].dwImageHeight
			dec	eax
			dec	edx
			not	eax
			not	edx
			mov	[ebx].lmSizeMask[8], ax
			mov	[ebx].lmSizeMask[10], dx
			mov	[ebx].lmSizeMask[12], ax
			mov	[ebx].lmSizeMask[14], dx
			mov	eax, [edi].dwBitsPerPixel
			mov	edx, [edi].dwBytesPerLine
			shl	edx, 16
			shr	eax, 3
			or	eax, edx
			mov	DWORD PTR [ebx].lmMulAddr[0], eax
			mov	DWORD PTR [ebx].lmMulAddr[4], eax
			mov	eax, [edi].ptrImageArray
			mov	[ebx].lmImageAddr[0], eax
			mov	[ebx].lmImageAddr[4], eax
		.ELSE
			mov	[ebx].lmnimg.dwInfoSize, 0
			mov	[ebx].pLmLineAddr, 0
		.ENDIF
		;
		; テクスチャ適用度を設定する
		;
		mov	edi, pAttr
		ASSUME	edi:PTR E3D_SURFACE_ATTRIBUTE
		mov	edx, [edi].nLuminousApply
		mov	eax, [edi].nTextureApply
		neg	edx
		mov	[ebx].nTextureApply, eax
		add	edx, 100H
		.IF	SIGN?
			xor	edx, edx
		.ENDIF
		mov	[ebx].nLiminousApply, edx
		ASSUME	edi:NOTHING
	.ENDIF

	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglRenderPoly@PrepareImageBuffer	ENDP

;
;	テクスチャパラメータ計算
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@PrepareParameterWithTextureSSE	PROC	NEAR32 C

	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	ASSUME	esi:PTR E3D_TEXTURE_MAPINFO

	.IF	[ebx].dwFunctionFlags & E3D_FLAG_ENABLE_SSE2

	;
	;	SSE2 コード
	; --------------------------------------------------------------------
	;
	; ベクトルを複製する
	;
	movups		xmm0, [esi].vOriginPos
	movups		xmm1, [esi].vAxisX
	movups		xmm2, [esi].vAxisY
	movaps		[ebx].vTxo, xmm0
	movaps		[ebx].vTxx, xmm1
	movaps		[ebx].vTxy, xmm2
	;
	; xmm0, xmm2 の外積を計算する
	;
	movaps		xmm3, xmm1
	movaps		xmm4, xmm2
	shufps		xmm1, xmm1, 11001001B
	shufps		xmm2, xmm2, 11010010B
	shufps		xmm3, xmm3, 11010010B
	shufps		xmm4, xmm4, 11001001B
	cvtps2pd	xmm5, xmm1
	cvtps2pd	xmm6, xmm2
	cvtps2pd	xmm7, xmm3
	cvtps2pd	xmm0, xmm4
	mulpd		xmm5, xmm6
	shufps		xmm1, xmm1, 2
	shufps		xmm2, xmm2, 2
	shufps		xmm3, xmm3, 2
	shufps		xmm4, xmm4, 2
	mulpd		xmm7, xmm0
	cvtss2sd	xmm1, xmm1
	cvtss2sd	xmm2, xmm2
	cvtss2sd	xmm3, xmm3
	cvtss2sd	xmm4, xmm4
	mulsd		xmm1, xmm2
	mulsd		xmm3, xmm4
	subpd		xmm5, xmm7
	subsd		xmm1, xmm3
	movapd		[ebx].vTxxy_d.x, xmm5
	_movsd		[ebx].vTxxy_d.z, xmm1
	cvtpd2ps	xmm0, xmm5
	cvtsd2ss	xmm2, xmm1
	movlps		QWORD PTR [ebx].vTxxy.x, xmm0
	movss		[ebx].vTxxy.z, xmm2
	;
	; vTxo, vTxxy の内積を計算する
	;
	cvtps2pd	xmm0, QWORD PTR [ebx].vTxo.x
	cvtss2sd	xmm2, [ebx].vTxo.z
	mulpd		xmm0, xmm5
	mulsd		xmm2, xmm1
	addsd		xmm2, xmm0
	shufpd		xmm0, xmm0, 1
	addsd		xmm0, xmm2
	_movsd		[ebx].rTxoxy_d, xmm0
	cvtss2sd	xmm1, [ebx].vScreenPos.z
	cvtsd2ss	xmm4, xmm0
	mulsd		xmm0, xmm1
	movss		[ebx].rTxoxy, xmm4
	_movsd		[ebx].rTxoxyr_d, xmm0
	cvtsd2ss	xmm4, xmm0
	movss		[ebx].rTxoxyr, xmm4
	_movsd		xmm1, rConst1_pd
	divsd		xmm1, xmm0
	_movsd		[ebx].rRcpTxoxyr_d, xmm1
	cvtsd2ss	xmm2, xmm1
	movss		[ebx].rRcpTxoxyr, xmm2
	;
	mov	eax, [ebx].rTxoxy
	and	eax, 7FFFFFFFH
	.IF	eax >= 70000000H
		mov	eax, eslErrContinue
		ret
	.ENDIF
	;
	; 差分パラメータを計算する
	;
	mov	eax, [ebx].vTxxy.x
	mov	ecx, [ebx].vTxxy.y
	mov	edx, [ebx].vTxxy.z
	and	eax, 7FFFFFFFH
	and	ecx, 7FFFFFFFH
	and	edx, 7FFFFFFFH
	.IF	eax > ecx
		.IF	eax < edx
			xor	eax, eax
			xor	ecx, ecx
		.ENDIF
	.ELSE
		.IF	ecx > edx
			xor	eax, eax
			xor	edx, edx
		.ELSE
			xor	eax, eax
			xor	ecx, ecx
		.ENDIF
	.ENDIF
	.IF	eax > 33800000H			; abs(vTxxy.x) > 2^-24
		@RenderPoly@PrepareTextureSSE2	x, y, z
		xor	eax, eax
		ret

	.ELSEIF	ecx > 33800000H			; abs(vTxxy.y) > 2^-24
		@RenderPoly@PrepareTextureSSE2	y, z, x
		xor	eax, eax
		ret

	.ELSE
		@RenderPoly@PrepareTextureSSE2	z, x, y

	.ENDIF

	xor	eax, eax
	ret

	.ELSE
	;
	;	SSE コード
	; --------------------------------------------------------------------
	;
	; ベクトルを複製する
	;
	movups	xmm0, [esi].vOriginPos
	movups	xmm1, [esi].vAxisX
	movups	xmm2, [esi].vAxisY
	movaps	[ebx].vTxo, xmm0
	movaps	[ebx].vTxx, xmm1
	movaps	[ebx].vTxy, xmm2
	;
	; xmm1, xmm2 の外積を計算する
	;
	movaps	xmm3, xmm1
	movaps	xmm4, xmm2
	shufps	xmm1, xmm1, 11001001B
	shufps	xmm2, xmm2, 11010010B
	shufps	xmm3, xmm3, 11010010B
	shufps	xmm4, xmm4, 11001001B
	mulps	xmm1, xmm2
	mulps	xmm3, xmm4
	subps	xmm1, xmm3
	movaps	[ebx].vTxxy, xmm1
	;
	; xmm1, xmm0 の内積を計算する
	;
	mulps	xmm0, xmm1
	movaps	xmm1, xmm0
	shufps	xmm0, xmm0, 1
	movhlps	xmm2, xmm1
	addss	xmm0, xmm1
	addss	xmm0, xmm2
	movss	[ebx].rTxoxy, xmm0
	movss	xmm1, [ebx].vScreenPos.z
	mulss	xmm0, xmm1
	movss	xmm1, rConst1
	mov	eax, 7FFFFFFFH
	movss	[ebx].rTxoxyr, xmm0
	and	eax, DWORD PTR [ebx].rTxoxyr
	divss	xmm1, xmm0
	movss	[ebx].rRcpTxoxyr, xmm1
	;
	mov	eax, [ebx].rTxoxy
	and	eax, 7FFFFFFFH
	.IF	eax >= 70000000H
		mov	eax, eslErrContinue
		ret
	.ENDIF
	;
	; 差分パラメータを計算する
	;
	mov	eax, [ebx].vTxxy.x
	mov	ecx, [ebx].vTxxy.y
	mov	edx, [ebx].vTxxy.z
	and	eax, 7FFFFFFFH
	and	ecx, 7FFFFFFFH
	and	edx, 7FFFFFFFH
	.IF	eax > ecx
		.IF	eax < edx
			xor	eax, eax
			xor	ecx, ecx
		.ENDIF
	.ELSE
		.IF	ecx > edx
			xor	eax, eax
			xor	edx, edx
		.ELSE
			xor	eax, eax
			xor	ecx, ecx
		.ENDIF
	.ENDIF
	.IF	eax > 33800000H			; abs(vTxxy.x) > 2^-24
		@RenderPoly@PrepareTextureSSE	x, y, z
		xor	eax, eax
		ret

	.ELSEIF	ecx > 33800000H			; abs(vTxxy.y) > 2^-24
		@RenderPoly@PrepareTextureSSE	y, z, x
		xor	eax, eax
		ret

	.ELSE
		@RenderPoly@PrepareTextureSSE	z, x, y

	.ENDIF

	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

	.ENDIF

eglRenderPoly@PrepareParameterWithTextureSSE	ENDP


ALIGN	10H
eglRenderPoly@PrepareParameterWithoutTextureSSE	PROC	NEAR32 C

	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	ASSUME	esi:PTR E3D_PLANE_PARAMETER

	.IF	[ebx].dwFunctionFlags & E3D_FLAG_ENABLE_SSE2

	;
	;	SSE2 コード
	; --------------------------------------------------------------------
	;
	; vTxxy = 法線ベクトル
	; vTxBase = 基準座標
	; rTxtxy = (vTxxy, vTxBase) 内積
	; rTxoxy = - d * r : 係数
	;
	movups		xmm4, [esi]
	mov		edi, [ebx].dib.pRegion
	xorps		xmm0, xmm0
	xorps		xmm1, xmm1
	ASSUME		edi:PTR E3D_POLYGON_REGION
	cvtsi2ss	xmm0, [edi].nTopLine
	ASSUME		edi:NOTHING
	movlps		xmm1, QWORD PTR [ebx].vScreenPos.x
	shufps		xmm0, xmm0, 11100001B
	movss		xmm2, [ebx].vScreenPos.z
	subps		xmm0, xmm1
	movaps		[ebx].vTxxy, xmm4
	movlhps		xmm0, xmm2
	movaps		[ebx].vTxBase, xmm0
	;
	movhlps		xmm5, xmm4
	movhlps		xmm1, xmm0
	cvtps2pd	xmm4, xmm4
	cvtps2pd	xmm0, xmm0
	cvtps2pd	xmm5, xmm5
	cvtss2sd	xmm1, xmm1
	movapd		[ebx].vTxxy_d.x, xmm4
	_movsd		[ebx].vTxxy_d.z, xmm5
	;
	mulpd		xmm0, xmm4
	mulsd		xmm1, xmm5
		shufpd		xmm5, xmm5, 1
		cvtss2sd	xmm2, xmm2
	addsd		xmm1, xmm0
	shufpd		xmm0, xmm0, 1
		mulsd		xmm5, xmm2
	addsd		xmm1, xmm0
		movapd		xmm4, xmmMaskSign_pd
	_movsd		[ebx].rTxtxy_d, xmm1
		xorpd		xmm4, xmm5
	cvtsd2ss	xmm1, xmm1
		_movsd		xmm0, rConst1_pd
	movss		[ebx].rTxtxy, xmm1
		divsd		xmm0, xmm4
		_movsd		[ebx].rTxoxyr_d, xmm4
		cvtsd2ss	xmm4, xmm4
		movss		[ebx].rTxoxyr, xmm4
		_movsd		[ebx].rRcpTxoxyr_d, xmm0
		cvtsd2ss	xmm0, xmm0
		movss		[ebx].rRcpTxoxyr, xmm0

	ret


	.ELSE
	;
	;	SSE コード
	; --------------------------------------------------------------------
	;
	; vTxxy = 法線ベクトル
	; vTxBase = 基準座標
	; rTxtxy = (vTxxy, vTxBase) 内積
	; rTxoxy = - d * r : 係数
	;
	movups	xmm4, [esi]
	mov	edi, [ebx].dib.pRegion
	xorps	xmm0, xmm0
	xorps	xmm1, xmm1
	ASSUME	edi:PTR E3D_POLYGON_REGION
	cvtsi2ss	xmm0, [edi].nTopLine
	ASSUME	edi:NOTHING
	movlps	xmm1, QWORD PTR [ebx].vScreenPos.x
	shufps	xmm0, xmm0, 11100001B
	movss	xmm2, [ebx].vScreenPos.z
	subps	xmm0, xmm1
	movaps	[ebx].vTxxy, xmm4
	movlhps	xmm0, xmm2
	movaps	[ebx].vTxBase, xmm0
	mulps	xmm0, xmm4
	shufps	xmm4, xmm4, 11B
	movaps	xmm1, xmm0
	shufps	xmm0, xmm0, 1
	mulss	xmm4, xmm2
	movhlps	xmm2, xmm1
	addss	xmm1, xmm0
	addss	xmm1, xmm2
	movss	xmm5, xmmMaskSign	; = 80000000H
	movss	[ebx].rTxtxy, xmm1
	xorps	xmm4, xmm5
	movss	xmm0, rConst1
	movss	[ebx].rTxoxyr, xmm4
	divss	xmm0, xmm4
	movss	[ebx].rRcpTxoxyr, xmm0

	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	ret

	.ENDIF

eglRenderPoly@PrepareParameterWithoutTextureSSE	ENDP


;
;	レンダリング関数準備＆選択
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@SelectRenderPolygonFuncSSE PROC	NEAR32 C USES esi edi,
	pPolyEntry:PCE3D_POLYGON_ENTRY

	LOCAL	nRenderFuncType:DWORD
	LOCAL	nPixelShaderType:DWORD
	LOCAL	nPixelRenderType:DWORD

;	mov	ebx, hRenderPoly
	mov	esi, pPolyEntry
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	ASSUME	esi:PCE3D_POLYGON_ENTRY
	;
	mov	edi, [esi].pAttr
	mov	[ebx].pSurfaceAttr, edi
	;
	mov	eax, [esi].dwTransparency
	mov	edx, [esi].dwShadingFlags
	mov	ecx, [esi].dwTypeFlag
	;
;	.IF	!([ebx].dwFunctionFlags & E3D_FLAG_TEXTURE_SMOOTHING)
;		and	edx, NOT E3DSAF_TEXTURE_SMOOTH
;	.ENDIF
	.IF	[ebx].dib.zbuf.dwBytesPerLine == 0
		or	edx, E3DSAF_NO_ZBUFFER
	.ENDIF
	mov	[ebx].nTextureApply, eax
	mov	[ebx].dwShadingFlags, edx
	.IF	eax >= 100H
		mov	eax, RPT_RENDER_POLY_ERROR
		ret
	.ENDIF
	;
	.IF	edx & E3DSAF_TEXTURE_MAPPING
		mov	eax, edx
		and	eax, NOT E3DSAF_ZBUF_ONLY_COMPARE
		test	edx, E3DSAF_NO_ZBUFFER
		cmovnz	edx, eax
		;
		mov	ecx, OFFSET eglRenderPoly@RenderPolygon_AntialiasSSE
		xor	eax, eax
		test	[ebx].dwFunctionFlags, E3D_FLAG_ANTIALIAS_SIDE_EDGE
		cmovz	ecx, eax
		;
		; ピクセルシェーディング関数選択
		;
		xor	edi, edi
		;xor	eax, eax
		test	[ebx].txtimg.fdwFormatType, EIF_WITH_ALPHA
		cmovnz	ecx, eax
		setnz	al
		or	edi, eax
		;
		xor	eax, eax
		test	edx, E3DSAF_TEXTURE_SMOOTH
		setnz	al
		shl	eax, 1
		or	edi, eax
		;
		xor	eax, eax
		test	edx, E3DSAF_TEXTURE_TILING
		setnz	al
		shl	eax, 2
		or	edi, eax
		;
		xor	eax, eax
		cmp	[ebx].lmnimg.dwInfoSize, 0
		setnz	al
		shl	eax, 3
		or	edi, eax
		;
		xor	eax, eax
		test	edx, E3DSAF_NO_ZBUFFER
		setz	al
		shl	eax, 4
		or	eax, edi
		;
		test	[ebx].dwFunctionFlags, E3D_FLAG_ENABLE_SSE2
		cmovnz	edi, eax
		;
		mov	nPixelShaderType, edi
		;
		.IF	edx & E3DSAF_SHADING_MASK
			mov	eax, pfnPixelShaderTextureGouraud[edi * 4]
		.ELSE
			mov	eax, pfnPixelShaderTextureNoShade[edi * 4]
		.ENDIF
		mov	[ebx].pfnLineFunc, eax
		;
		; ピクセルレンダラ関数選択
		;
		xor	edi, edi
		xor	eax, eax
		test	edx, E3DSAF_TEXTURE_TRIM
		setnz	al
		or	edi, eax
		;
		xor	eax, eax
		test	[ebx].txtimg.fdwFormatType, EIF_WITH_ALPHA
		setnz	al
		shl	eax, 1
		or	edi, eax
		;
		xor	eax, eax
		cmp	[ebx].nTextureApply, 0
		cmovnz	ecx, eax
		setnz	al
		shl	eax, 2
		or	edi, eax
		;
		xor	eax, eax
		test	edx, E3DSAF_NO_ZBUFFER
		cmovnz	ecx, eax
		setnz	al
		shl	eax, 3
		or	edi, eax
		;
		xor	eax, eax
		test	edx, E3DSAF_ZBUF_ONLY_COMPARE
		cmovnz	ecx, eax
		setnz	al
		shl	eax, 4
		or	edi, eax
		mov	nPixelRenderType, edi
		;
		mov	eax, pfnPixelRenderTextureFuncTable[edi * 8]
		mov	edi, pfnPixelRenderTextureFuncTable[edi * 8 + 4]
		mov	[ebx].pfnAntialius, ecx
		mov	[ebx].pfnStoreFunc, eax
		mov	nRenderFuncType, edi
		;
		; フォンシェーディング判定
		;
		mov	edx, [ebx].dwShadingFlags
		.IF	edx & E3DSAF_PHONG_SHADE
			mov	eax, OFFSET eglRenderPoly@ShadeVectorsSSE
			mov	ecx, OFFSET eglRenderPoly@ShadeVectorsAndRayTraceSSE
			test	edx, (E3DSAF_RAY_SHADOWING \
					OR E3DSAF_RAY_REFLECTING \
					OR E3DSAF_RAY_REFRACTING)
			cmovnz	eax, ecx
			mov	[ebx].pfnLineShading, eax
			;
			mov	eax, [ebx].pfnLineFunc
			mov	[ebx].pfnLineSubFunc, eax
			.IF	!(edx & E3DSAF_RAY_SHADOWING \
					OR E3DSAF_RAY_REFLECTING \
					OR E3DSAF_RAY_REFRACTING)
				.IF	edx & E3DSAF_GOURAUD_SHADE
					mov	[ebx].pfnLineFunc, \
						OFFSET eglRenderPoly@RenderPolygon_LineShadingTexture
					;
					mov	eax, nPixelShaderType
					mov	eax, pfnPixelShaderTextureNoShade[eax * 4]
					mov	[ebx].pfnLineSubFunc, eax
				.ELSE
					mov	[ebx].pfnLineFunc, \
						OFFSET eglRenderPoly@RenderPolygon_LineShading
					;
					mov	eax, nPixelRenderType
					mov	edi, [ebx].pSurfaceAttr
					and	eax, NOT 0100B
					ASSUME	edi:PE3D_SURFACE_ATTRIBUTE
					.IF	([edi].nDeepness != 0) \
							|| ([edi].nTransparency != 0)
						or	eax, 0010B
						mov	[ebx].pfnAntialius, NULL
					.ENDIF
					mov	edi, pfnPixelRenderTextureFuncTable[eax * 8]
					mov	eax, pfnPixelRenderTextureFuncTable[eax * 8 + 4]
					mov	[ebx].pfnStoreFunc, edi
					mov	nRenderFuncType, eax
					ASSUME	edi:NOTHING
				.ENDIF
			.ELSE
				mov	[ebx].pfnLineFunc, \
					OFFSET eglRenderPoly@RenderPolygon_LineRayTracingTexture
				;
				mov	eax, nPixelShaderType
				mov	eax, pfnPixelShaderTextureNoShade[eax * 4]
				mov	[ebx].pfnLineSubFunc, eax
				;
				.IF	edx & E3DSAF_RAY_REFRACTING
					mov	edi, nPixelRenderType
					and	edi, NOT 0110B
					mov	eax, pfnPixelRenderTextureFuncTable[edi * 8]
					mov	edi, pfnPixelRenderTextureFuncTable[edi * 8 + 4]
					mov	[ebx].pfnStoreFunc, eax
					mov	nRenderFuncType, edi
				.ENDIF
			.ENDIF
		.ENDIF
		mov	eax, nRenderFuncType
		ret
	.ELSE
		mov	ecx, OFFSET eglRenderPoly@RenderPolygon_LineNTXSSE
		mov	eax, OFFSET eglRenderPoly@RenderPolygon_LineNTXSSE2
		test	[ebx].dwFunctionFlags, E3D_FLAG_ENABLE_SSE2
		cmovnz	ecx, eax
		xor	eax, eax
		;mov	nRenderFuncType, RPT_RENDER_POLY_NORMAL
		.IF	edx & E3DSAF_NO_ZBUFFER
			mov	eax, 1000B
			mov	ecx, OFFSET eglRenderPoly@RenderPolygon_LineNTXSSE
			;mov	nRenderFuncType, RPT_RENDER_POLY_V_NO_Z
		.ELSEIF	edx & E3DSAF_ZBUF_ONLY_COMPARE
			mov	eax, 10000B
			;mov	nRenderFuncType, RPT_RENDER_POLY_CMP_Z
		.ENDIF
		;
		mov	[ebx].pfnLineFunc, ecx
		.IF	[ebx].nTextureApply != 0
			or	eax, 0110B
			;mov	eax, OFFSET eglRenderPoly@RenderPolygon_StoreATSSE
		;.ELSEIF	edx & E3DSAF_ZBUF_ONLY_COMPARE
		;	mov	nRenderFuncType, RPT_RENDER_POLY_NORMAL
		;	mov	eax, OFFSET eglRenderPoly@RenderPolygon_StoreARZSSE
		.ELSE
			or	eax, 0010B
			;mov	eax, OFFSET eglRenderPoly@RenderPolygon_StoreASSE
		.ENDIF
		;
		mov	edx, OFFSET eglRenderPoly@RenderPolygon_AntialiasSSE
		.IF	!([ebx].dwFunctionFlags & E3D_FLAG_ANTIALIAS_SIDE_EDGE)
			xor	edx, edx
		.ENDIF
		.IF	[ebx].nTextureApply == 0
			mov	ecx, [esi].dwProjectedCount
			mov	edi, [esi].pVertexColors
			ASSUME	edi:PTR E3D_COLOR
			test	ecx, ecx
			.WHILE	!ZERO?
				.BREAK	.IF	[edi].rgbMul.dwPixelCode & 0FFFFFFH
				add	edi, SIZEOF E3D_COLOR
				dec	ecx
			.ENDW
			ASSUME	edi:NOTHING
			.IF	[ebx].dwShadingFlags & E3DSAF_PHONG_SHADE
				mov	edi, [ebx].pSurfaceAttr
				ASSUME	edi:PTR E3D_SURFACE_ATTRIBUTE
				.IF	([edi].nTransparency != 0) \
						|| ([edi].nDeepness != 0)
					mov	ecx, 1
				.ENDIF
				ASSUME	edi:NOTHING
			.ENDIF
			.IF	ecx == 0
				and	eax, NOT 0110B
				;.IF	!([ebx].dwShadingFlags & E3DSAF_NO_ZBUFFER)
				;	mov	eax, OFFSET eglRenderPoly@RenderPolygon_StoreSSEC
				;.ELSE
				;	mov	eax, OFFSET eglRenderPoly@RenderPolygon_StoreNZSSE
				;.ENDIF
			.ELSE
				xor	edx, edx
			.ENDIF
		.ELSE
			xor	edx, edx
		.ENDIF
		;
		mov	nPixelRenderType, eax
		mov	ecx, pfnPixelRenderTextureFuncTable[eax * 8]
		mov	eax, pfnPixelRenderTextureFuncTable[eax * 8 + 4]
		;
		mov	[ebx].pfnAntialius, edx
		mov	[ebx].pfnStoreFunc, ecx
		mov	nRenderFuncType, eax
	.ENDIF
	;
	mov	edx, [ebx].dwShadingFlags
	.IF	edx & E3DSAF_PHONG_SHADE
		mov	eax, [ebx].pfnLineFunc
		mov	[ebx].pfnLineSubFunc, eax
		mov	[ebx].pfnLineFunc, \
			OFFSET eglRenderPoly@RenderPolygon_LineShading
		;
		mov	eax, OFFSET eglRenderPoly@ShadeVectorsSSE
		mov	ecx, OFFSET eglRenderPoly@ShadeVectorsAndRayTraceSSE
		test	edx, (E3DSAF_RAY_SHADOWING \
				OR E3DSAF_RAY_REFLECTING \
				OR E3DSAF_RAY_REFRACTING)
		cmovnz	eax, ecx
		mov	[ebx].pfnLineShading, eax
		;
		.IF	edx & E3DSAF_RAY_REFRACTING
			mov	edi, nPixelRenderType
			and	edi, NOT 0110B
			mov	eax, pfnPixelRenderTextureFuncTable[edi * 8]
			mov	edi, pfnPixelRenderTextureFuncTable[edi * 8 + 4]
			mov	[ebx].pfnStoreFunc, eax
			mov	nRenderFuncType, edi
		.ENDIF
	.ENDIF

	ASSUME	ebx:NOTHING
	ASSUME	esi:NOTHING
	mov	eax, nRenderFuncType
	ret

eglRenderPoly@SelectRenderPolygonFuncSSE ENDP


;
;	レンダリング準備 SSE 専用関数
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@PrepareRenderSSE	PROC	NEAR32 C USES ebx esi edi,
	hRenderPoly:HEGL_RENDER_POLYGON, pPolyEntry:PCE3D_POLYGON_ENTRY

	LOCAL	dp:EGL_DRAW_PARAM
	LOCAL	axes:EGL_IMAGE_AXES
	LOCAL	rect:EGL_IMAGE_RECT
	LOCAL	ptCenter:EGL_POINT

	mov	esi, pPolyEntry
	mov	ebx, hRenderPoly
	ASSUME	esi:PCE3D_POLYGON_ENTRY
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	mov	[ebx].rp.pfnRenderPolygon, \
			OFFSET eglRenderPoly@RenderPolygon_Err

	mov	eax, [esi].dwTypeFlag
	.IF	eax & E3D_MESH_POLYGON
		; ------------------------------------------------------------
		;	メッシュ描画準備
		; ------------------------------------------------------------
		;
		; テクスチャ画像情報取得
		;
		mov	[ebx].pMeshPolygonEntry, esi
		mov	edi, [esi].pAttr
		ASSUME	edi:PTR E3D_SURFACE_ATTRIBUTE
		;
		.IF	eax & E3D_TEXTURE_POLYGON
			;
			; 画像バッファ設定
			;
			movss	xmm0, [esi].surface.mesh.vMinMesh.z
			xor	ecx, ecx
			xor	edx, edx
			comiss	xmm0, [edi].txmap.rThresholdZ
			.IF	CARRY?
				xor	eax, eax
				mov	ecx, [edi].txmap.pTextureImage
				mov	edx, [edi].txmap.pLuminousImage
			.ELSE
				mov	eax, [edi].txmap.nSmallScale
				mov	ecx, [edi].txmap.pSmallImage
				mov	edx, [edi].txmap.pSmallLuminous
			.ENDIF
			test	ecx, ecx
			jz	Label_ExitNoDraw
			mov	[ebx].nTextureUVScale, eax
			;
			INVOKE	eglRenderPoly@PrepareImageBuffer , edi, ecx, edx
			test	eax, eax
			jnz	Label_ExitNoDraw
			;
			; テクスチャマッピングスケール設定
			;
			mov	eax, 1
			cvtsi2ss	xmm0, eax
			mov	ecx, [ebx].nTextureUVScale
			shl	eax, cl
			cvtsi2ss	xmm1, eax
			divss	xmm0, xmm1
			movss	[ebx].rTextureUVScale,xmm0
		.ELSE
			INVOKE	eglRenderPoly@PrepareImageBuffer , edi, NULL, NULL
		.ENDIF
		ASSUME	edi:NOTHING
		;
		; レンダリング関数選別
		;
		INVOKE	eglRenderPoly@SelectRenderPolygonFuncSSE , esi
		cmp	eax, RPT_RENDER_POLY_ERROR
		je	Label_ExitNoDraw
		mov	[ebx].nRenderFuncType, eax
		;
		mov	[ebx].rp.pfnRenderPolygon, \
				OFFSET eglRenderPoly@RenderPolygon_MeshSSE
		xor	eax, eax
		ret


	.ELSEIF	eax == E3D_IMAGE_PRIMITIVE
		; ------------------------------------------------------------
		;	2D 画像描画準備
		; ------------------------------------------------------------
		;
		; 描画パラメータを設定する
		;
		mov	edi, [esi].surface.image.pInfo
		ASSUME	edi:PEGL_IMAGE_INFO
		.IF	edi == NULL
			mov	eax, eslErrInvalidParam
			ret
		.ENDIF
		.IF	[ebx].dib.pZBuffer != NULL
			mov	dp.dwFlags, (EGL_WITH_Z_ORDER OR EGL_WITH_AXES OR EGL_FIXED_POSITION)
		.ELSE
			mov	dp.dwFlags, EGL_WITH_AXES OR EGL_FIXED_POSITION
		.ENDIF
		mov	eax, [esi].pAttr
		.IF	eax != NULL
			ASSUME	eax:PE3D_SURFACE_ATTRIBUTE
			.IF	[eax].dwShadingFlags & E3DSAF_NO_ZBUFFER
				and	dp.dwFlags, NOT EGL_WITH_Z_ORDER
			.ENDIF
			ASSUME	eax:NOTHING
		.ENDIF
		;
		movss	xmm4, rConst65536
		movlps	xmm0, QWORD PTR [esi].surface.image.vImageBase
		shufps	xmm4, xmm4, 0
		movhps	xmm0, QWORD PTR [esi].surface.image.vCenter
		mulps	xmm0, xmm4
		movhlps	xmm1, xmm0
		cvtps2pi	mm0, xmm0
		cvtps2pi	mm1, xmm1
		movq	MMWORD PTR ptCenter, mm0
		movq	MMWORD PTR dp.ptBasePos, mm1
		emms
		;
		INVOKE	eglGetRevolvedAxes ,
				ADDR axes, ADDR dp.ptBasePos, ADDR ptCenter,
				[esi].surface.image.vEnlarge.x,
				[esi].surface.image.vEnlarge.y,
				[esi].surface.image.rRevolveAngle, rConstHalfPI, 1
		;
		mov	eax, [esi].dwTransparency
		mov	edx, [esi].vCenter.z
		lea	ecx, axes
		mov	dp.pSrcImage, edi
		mov	dp.pViewRect, NULL
		mov	dp.nTransparency, eax
		mov	dp.rZOrder, edx
		mov	dp.pImageAxes, ecx
		;
		; 2D 描画準備関数へ
		;
		mov	[ebx].rp.pfnRenderPolygon, \
				OFFSET eglRenderPoly@RenderPolygon_Image
		;
		INVOKE	[ebx].dib.pfnPrepareDraw , ADDR [ebx].dib, ADDR dp
		;
		ret
	.ENDIF

	; --------------------------------------------------------------------
	;	3D 描画関数
	; --------------------------------------------------------------------
	;
	; リージョンを作成する
	;
	INVOKE	eglNormalizePolygonRegion ,
			[ebx].dib.pRegion, ADDR [ebx].dib.rectClip,
			[esi].dwProjectedCount,
			[esi].pProjVertexes,
			[esi].pVertexColors, [esi].pNormals
	test	eax, eax
	jz	Label_ExitNoDraw
	;
	;	テクスチャパラメータを計算する
	; --------------------------------------------------------------------
	movups	xmm0, [esi].plane
	movaps	[ebx].vTargetPlaneParam, xmm0
	;
;	.IF	[esi].dwTypeFlag & E3D_TEXTURE_POLYGON
	.IF	[esi].dwShadingFlags & E3DSAF_TEXTURE_MAPPING
		;
		; 画像バッファを設定する
		;
		cmp	[esi].surface.poly.txmap.pTextureImage, NULL
		jz	Label_ExitNoDraw
		;
		INVOKE	eglRenderPoly@PrepareImageBuffer ,
				[esi].pAttr,
				[esi].surface.poly.txmap.pTextureImage,
				[esi].surface.poly.txmap.pLuminousImage
		test	eax, eax
		jnz	Label_ExitNoDraw
		;
		; テクスチャ適用度を取得する
		;
		mov	edx, [esi].surface.poly.txmap.nLuminousApply
		mov	eax, [esi].surface.poly.txmap.nTextureApply
		neg	edx
		mov	[ebx].nTextureApply, eax
		add	edx, 100H
		.IF	SIGN?
			xor	edx, edx
		.ENDIF
		mov	[ebx].nLiminousApply, edx
		;
		; フォッグパラメータを取得する
		;
		mov	eax, [esi].surface.poly.txmap.rFogDeepness
		.IF	eax != 0
			add	eax, 04000000H				; *= 256.0
		.ENDIF
		mov	[ebx].rRenderFogDeepness, eax
		movzx	eax, [esi].surface.poly.txmap.rgbFogColor.rgb.Blue
		movzx	ecx, [esi].surface.poly.txmap.rgbFogColor.rgb.Green
		movzx	edx, [esi].surface.poly.txmap.rgbFogColor.rgb.Red
		mov	[ebx].rgbRenderFogColor[0], ax
		mov	[ebx].rgbRenderFogColor[2], cx
		mov	[ebx].rgbRenderFogColor[4], dx
		mov	[ebx].rgbRenderFogColor[6], 0
		;
		; 行列パラメータを計算する
		;
		lea	esi, [esi].surface.poly.txmap
		INVOKE	eglRenderPoly@PrepareParameterWithTextureSSE
		test	eax, eax
		jnz	Label_ExitNoDraw

	.ELSE
		;
		;	非テクスチャポリゴンの場合
		; ------------------------------------------------------------
		;
		; 画像バッファを設定する
		;
		INVOKE	eglRenderPoly@PrepareImageBuffer ,
				[esi].pAttr, NULL, NULL
		;
		; レンダリング用パラメータを計算する
		;
		lea	esi, [esi].plane
		INVOKE	eglRenderPoly@PrepareParameterWithoutTextureSSE

	.ENDIF
	;
	;	レンダリング関数を振り分ける
	; --------------------------------------------------------------------
	mov	esi, pPolyEntry
	.IF	[esi].dwTypeFlag != E3D_INFINITE_PLANE
		INVOKE	eglRenderPoly@SelectRenderPolygonFuncSSE , esi
		mov	eax, pfnRenderFuncTableSSE[eax*4]
		mov	[ebx].rp.pfnRenderPolygon, eax
		xor	eax, eax
		ret

	.ELSE
		;
		;	無限平面
		; ------------------------------------------------------------
		mov	edi, [esi].pAttr
		ASSUME	edi:PE3D_SURFACE_ATTRIBUTE
		mov	eax, [esi].dwTransparency
		mov	edx, [edi].dwShadingFlags
		ASSUME	edi:NOTHING
		mov	ecx, [esi].dwTypeFlag
		;
		.IF	!([ebx].dwFunctionFlags & E3D_FLAG_TEXTURE_SMOOTHING)
			and	edx, NOT E3DSAF_TEXTURE_SMOOTH
		.ENDIF
		mov	[ebx].nTextureApply, eax
		mov	[ebx].dwShadingFlags, edx
		.IF	eax >= 100H
			mov	[ebx].rp.pfnRenderPolygon, \
					OFFSET eglRenderPoly@RenderPolygon_NoDraw
			xor	eax, eax
			ret
		.ENDIF
		;
		mov	[ebx].rp.pfnRenderPolygon, \
			OFFSET eglRenderPoly@RenderPolygon_PlaneSSE
		mov	[ebx].pfnAntialius, NULL
		;
		.IF	edx & E3DSAF_TEXTURE_MAPPING
			.IF	edx & E3DSAF_TEXTURE_SMOOTH
				.IF	[ebx].lmnimg.dwInfoSize == 0
					mov	eax, OFFSET eglRenderPoly@RenderPolygon_LineSTXSSE
				.ELSE
					mov	eax, OFFSET eglRenderPoly@RenderPolygon_LineSTLXSSE
				.ENDIF
			.ELSE
				.IF	[ebx].lmnimg.dwInfoSize == 0
					mov	eax, OFFSET eglRenderPoly@RenderPolygon_LineTTXSSE
				.ELSE
					mov	eax, OFFSET eglRenderPoly@RenderPolygon_LineTLTXSSE
				.ENDIF
			.ENDIF
		.ELSE
			mov	[ebx].rp.pfnRenderPolygon, \
				OFFSET eglRenderPoly@RenderPolygon_NoDraw
			mov	eax, OFFSET eglRenderPoly@RenderPolygon_LineNTXSSE
		.ENDIF
		.IF	[ebx].dib.zbuf.dwBytesPerLine != 0
			mov	[ebx].pfnStoreFunc, \
				OFFSET eglRenderPoly@RenderPolygon_StoreSSEC
			;	OFFSET eglRenderPoly@RenderPolygon_StoreSSE
		.ELSE
			mov	[ebx].pfnStoreFunc, \
				OFFSET eglRenderPoly@RenderPolygon_StoreNZSSE
		.ENDIF
		mov	[ebx].pfnLineFunc, eax

	.ENDIF

	ASSUME	esi:NOTHING
	xor	eax, eax
	ret

Label_ExitNoDraw:
	mov	[ebx].rp.pfnRenderPolygon, \
		OFFSET eglRenderPoly@RenderPolygon_NoDraw
	xor	eax, eax
	ret

	ASSUME	ebx:NOTHING

eglRenderPoly@PrepareRenderSSE	ENDP




CodeSeg	ENDS

	END
