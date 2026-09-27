
; ----------------------------------------------------------------------------
;                          Entis Graphic Library
; ----------------------------------------------------------------------------
;   Copyright (c) 2002-2008 Leshade Entis, Entis-soft. Al rights reserved.
; ----------------------------------------------------------------------------


	.486
	.387
	.MODEL	FLAT

	INCLUDE	experi.inc
	INCLUDE	egl.inc


; ----------------------------------------------------------------------------
;	データセグメント
; ----------------------------------------------------------------------------

ConstSeg	SEGMENT	PARA READONLY FLAT 'CONST'

ALIGN	10H
rConst1		REAL4	4 DUP( 1.0 )
rConst256	REAL4	4 DUP( 256.0 )
rConst65536	REAL4	4 DUP( 65536.0 )
rConstHalfPI	REAL4	4 DUP( 1.57079632679489661923132169164 )
xmmMaskSign	DWORD	4 DUP( 80000000H )
mmxFEFF		WORD	4 DUP( 0FEFFH )

ALIGN	10H
pfnRenderFuncTable486	LABEL	PRenderPoly@RenderPolygon
		DWORD	OFFSET eglRenderPoly@RenderPolygon_NoDraw
		DWORD	OFFSET eglRenderPoly@RenderPolygon_Texture486
		DWORD	OFFSET eglRenderPoly@RenderPolygon_TxtNZ486
		DWORD	OFFSET eglRenderPoly@RenderPolygon_TxtRZ486
		DWORD	OFFSET eglRenderPoly@RenderPolygon_Texture486

ConstSeg	ENDS


; ----------------------------------------------------------------------------
;	コードセグメント
; ----------------------------------------------------------------------------

CodeSeg	SEGMENT	PARA READONLY FLAT 'CODE'

;
;	テクスチャマッピング差分パラメータ算出マクロ
; ----------------------------------------------------------------------------
@RenderPoly@PrepareTexture	MACRO	_x:=<x>, _y:=<y>, _z:=<z>
	;
	;	パラメータ計算 486 互換コード
	; ------------------------------------------------------------
	;
	; ｘ座標差分
	;	x ; vTxxy.x * ( vTxo.z * vTxy.y - vTxo.y * vTxy.z )
	;	y ; vTxxy.x * ( vTxo.y * vTxx.z - vTxo.z * vTxx.y )
	; ｙ座標差分
	;	x ; vTxxy.y * ( vTxo.z * vTxy.y - vTxo.y * vTxy.z )
	;	y ; vTxxy.y * ( vTxo.y * vTxx.z - vTxo.z * vTxx.y )
	;
	fld	[ebx].vTxo._y
	fmul	[ebx].vTxx._z
	fld	[ebx].vTxo._z
	fmul	[ebx].vTxx._y
	fsubp	st(1), st
	fld	[ebx].vTxo._z
	fmul	[ebx].vTxy._y
	fld	[ebx].vTxo._y
	fmul	[ebx].vTxy._z
	fsubp	st(1), st
	;
	fld	st(1)
	fld	st(1)
	fld	[ebx].vTxxy.x
	fmul	st(1), st
	fmulp	st(2), st
	IFIDNI		<_x>, <y>
		fld	[ebx].vTxx.z
		fld	[ebx].vTxy.z
		fld	[ebx].rTxoxy
		fmul	st(1), st
		fmulp	st(2), st
		fsubp	st(2), st
		faddp	st(2), st
	ELSEIFIDNI	<_x>, <z>
		fld	[ebx].vTxx.y
		fld	[ebx].vTxy.y
		fld	[ebx].rTxoxy
		fmul	st(1), st
		fmulp	st(2), st
		faddp	st(2), st
		fsubp	st(2), st
	ENDIF
	fstp	[ebx].vTxDeltaX.x
	fstp	[ebx].vTxDeltaX.y
	;
	fld	[ebx].vTxxy.y
	fmul	st(1), st
	fmulp	st(2), st
	IFIDNI		<_x>, <x>
		fld	[ebx].vTxx.z
		fld	[ebx].vTxy.z
		fld	[ebx].rTxoxy
		fmul	st(1), st
		fmulp	st(2), st
		faddp	st(2), st
		fsubp	st(2), st
	ELSEIFIDNI	<_x>, <z>
		fld	[ebx].vTxx.x
		fld	[ebx].vTxy.x
		fld	[ebx].rTxoxy
		fmul	st(1), st
		fmulp	st(2), st
		fsubp	st(2), st
		faddp	st(2), st
	ENDIF
	fstp	[ebx].vTxDeltaY.x
	fstp	[ebx].vTxDeltaY.y
	;
	; 母数の差分
	;
	fld1
	fld	[ebx].vTxxy._x
	fst	[ebx].rDeltaTxxy
	fdivp	st(1), st
	fstp	[ebx].rRcpTxxy
	;
	; 基準座標の計算
	;	X0 = (vTxBase, vTxxy)		; 内積
	;	X1 = rTxoxy * vTxBase.y - X0 * vTxo.y
	;	X2 = rTxoxy * vTxBase.z - X0 * vTxo.z
	;	x' = (X1 * vTxy.z - X2 * vTxy.y) / (X0 * vTxxy.x)
	;	y' = (X2 * vTxx.y - X1 * vTxx.z) / (X0 * vTxxy.x)
	;
	mov	edi, [ebx].dib.pRegion
	fld	[ebx].vScreenPos.z
	fst	[ebx].vTxBase.z
	ASSUME	edi:PTR E3D_POLYGON_REGION
	fild	[edi].nTopLine
	ASSUME	edi:NOTHING
	fld	[ebx].vScreenPos.y
	fsubp	st(1), st
	fst	[ebx].vTxBase.y
	fld	[ebx].vScreenPos.x
	fchs
	fst	[ebx].vTxBase.x
	;
	fld	[ebx].vTxxy.x
	fld	[ebx].vTxxy.y
	fld	[ebx].vTxxy.z
	fxch	st(2)
	fmulp	st(3), st
	fmulp	st(3), st
	fmulp	st(3), st
	faddp	st(1), st
	faddp	st(1), st
	fst	[ebx].rTxtxy
	;
	fld	[ebx].rTxoxy
	fmul	[ebx].vTxBase._z
	fld	[ebx].vTxo._z
	fmul	st, st(2)
	fsubp	st(1), st
	fxch	st(1)
	fld	[ebx].rTxoxy
	fmul	[ebx].vTxBase._y
	fxch	st(1)
	fmul	[ebx].vTxo._y
	fsubp	st(1), st	; st(0) = X1,  st(1) = X2
	;
	fld	[ebx].vTxx._y
	fmul	st, st(2)	; *= X2
	fld	[ebx].vTxx._z
	fmul	st, st(2)	; *= X1
	fsubp	st(1), st
	fld	[ebx].vTxy._z
	fmul	st, st(2)	; *= X1
	fld	[ebx].vTxy._y
	fmul	st, st(4)	; *= X2
	fsubp	st(1), st
	fstp	st(2)
	fstp	st(2)
	fstp	[ebx].vTxBasePos.x
	fstp	[ebx].vTxBasePos.y
ENDM

;
;	テクスチャパラメータ計算
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@PrepareParameterWithTexture486	PROC	NEAR32 C

	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	ASSUME	esi:PTR E3D_TEXTURE_MAPINFO

	;
	; ベクトルを複製する
	;
	FOR	@MEMBER, <x, y, z>
		mov	eax, [esi].vOriginPos.@MEMBER
		mov	ecx, [esi].vAxisX.@MEMBER
		mov	edx, [esi].vAxisY.@MEMBER
		mov	[ebx].vTxo.@MEMBER, eax
		mov	[ebx].vTxx.@MEMBER, ecx
		mov	[ebx].vTxy.@MEMBER, edx
	ENDM
	;
	; 外積を計算する : [vTxx, vTxy]
	;
	@INDEX = 0
	FOR	@MEMBER, <x, y, z>
		@INDEX1 = (@INDEX + 1) MOD 3
		@INDEX2 = (@INDEX + 2) MOD 3
		fld	[ebx].vTxx.x[@INDEX1*4]
		fmul	[ebx].vTxy.x[@INDEX2*4]
		fld	[ebx].vTxx.x[@INDEX2*4]
		fmul	[ebx].vTxy.x[@INDEX1*4]
		fsubp	st(1), st
		fstp	[ebx].vTxxy.@MEMBER
		@INDEX = @INDEX + 1
	ENDM
	;
	; 内積を計算する : (vTxo,vTxxy)
	;
	fld	[ebx].vTxo.x
	fld	[ebx].vTxo.y
	fld	[ebx].vTxo.z
	fld	[ebx].vTxxy.z
	fld	[ebx].vTxxy.y
	fld	[ebx].vTxxy.x
	fxch	st(2)
	fmulp	st(3), st
	fmulp	st(3), st
	fmulp	st(3), st
	faddp	st(1), st
	faddp	st(1), st
	fst	[ebx].rTxoxy
	;
	fmul	[ebx].vScreenPos.z
	fst	[ebx].rTxoxyr
	fld1
	fdivrp	st(1), st
	fstp	[ebx].rRcpTxoxyr
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
		@RenderPoly@PrepareTexture	x, y, z
		xor	eax, eax
		ret

	.ELSEIF	ecx > 33800000H			; abs(vTxxy.y) > 2^-24
		@RenderPoly@PrepareTexture	y, z, x
		xor	eax, eax
		ret

	.ELSE
		@RenderPoly@PrepareTexture	z, x, y

	.ENDIF

	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglRenderPoly@PrepareParameterWithTexture486	ENDP


ALIGN	10H
eglRenderPoly@PrepareParameterWithoutTexture486	PROC	NEAR32 C

	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	ASSUME	esi:PTR E3D_PLANE_PARAMETER

	;
	; vTxxy = 法線ベクトル
	; vTxBase = 基準座標
	; rTxtxy = (vTxxy, vTxBase) 内積
	; rTxoxy = - d * r : 係数
	;
	mov	eax, [esi].x		; 法線ベクトル複製
	mov	ecx, [esi].y
	mov	edx, [esi].z
	mov	[ebx].vTxxy.x, eax
	mov	[ebx].vTxxy.y, ecx
	mov	[ebx].vTxxy.z, edx
	mov	edi, [ebx].dib.pRegion	; 基準座標取得
	mov	eax, [ebx].vScreenPos.x
	ASSUME	edi:PTR E3D_POLYGON_REGION
	fild	[edi].nTopLine
	ASSUME	edi:NOTHING
	xor	eax, 80000000H
	fsub	[ebx].vScreenPos.y
	mov	[ebx].vTxBase.x, eax
	fstp	[ebx].vTxBase.y
	mov	eax, [ebx].vScreenPos.z
	mov	[ebx].vTxBase.z, eax
	fld	[ebx].vTxxy.x		; 内積計算
	fld	[ebx].vTxxy.y
	fld	[ebx].vTxxy.z
	fld	[ebx].vTxBase.z
	fld	[ebx].vTxBase.y
	fld	[ebx].vTxBase.x
	fxch	st(2)
	fmulp	st(3), st
	fmulp	st(3), st
	fmulp	st(3), st
	faddp	st(1), st
	faddp	st(1), st
	fstp	[ebx].rTxtxy
	fld	[esi].d			; 係数計算
	fmul	[ebx].vTxBase.z
	fchs
	fst	[ebx].rTxoxyr
	fld1
	fdivrp	st(1), st
	fstp	[ebx].rRcpTxoxyr

	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	ret

eglRenderPoly@PrepareParameterWithoutTexture486	ENDP

;
;	レンダリング関数準備＆選択
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@SelectRenderPolygonFunc486 PROC	NEAR32 C USES esi edi,
	pPolyEntry:PCE3D_POLYGON_ENTRY

	LOCAL	nRenderFuncType:DWORD

;	mov	ebx, hRenderPoly
	mov	esi, pPolyEntry
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	ASSUME	esi:PCE3D_POLYGON_ENTRY

	mov	edi, [esi].pAttr
	ASSUME	edi:PE3D_SURFACE_ATTRIBUTE
	mov	eax, [esi].dwTransparency
	mov	edx, [edi].dwShadingFlags
	ASSUME	edi:NOTHING
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
	;	ポリゴン
	; --------------------------------------------------------------------
	.IF	[ebx].nTextureApply != 0
		push	edx
		mov	edx, [ebx].nTextureApply
		neg	edx
		INVOKE	eglCalculateToneTable ,
				ADDR [ebx].dib.nGreenTone[0],
				edx, EGL_TONE_BRIGHTNESS
		mov	edx, [ebx].nTextureApply
		sub	edx, 100H
		INVOKE	eglCalculateToneTable ,
				ADDR [ebx].dib.nBlueTone[0],
				edx, EGL_TONE_BRIGHTNESS
		pop	edx
	.ENDIF
	.IF	edx & E3DSAF_TEXTURE_MAPPING
		mov	nRenderFuncType, RPT_RENDER_POLY_NORMAL
		.IF	edx & E3DSAF_ZBUF_ONLY_COMPARE
			mov	nRenderFuncType, RPT_RENDER_POLY_CMP_Z
		.ENDIF
		mov	ecx, OFFSET eglRenderPoly@RenderPolygon_Antialias486
		.IF	!([ebx].dwFunctionFlags & E3D_FLAG_ANTIALIAS_SIDE_EDGE)
			xor	ecx, ecx
		.ENDIF
		;
		.IF	edx & E3DSAF_TEXTURE_TILING
			.IF	[ebx].txtimg.fdwFormatType & EIF_WITH_ALPHA
				mov	eax, OFFSET eglRenderPoly@RenderPolygon_LineTATX486
				xor	ecx, ecx
			.ELSEIF	[ebx].lmnimg.dwInfoSize == 0
				mov	eax, OFFSET eglRenderPoly@RenderPolygon_LineTTX486
			.ELSE
				mov	eax, OFFSET eglRenderPoly@RenderPolygon_LineTLTX486
			.ENDIF
		.ELSE
			mov	eax, [esi].dwProjectedCount
			mov	edi, [esi].pVertexColors
			ASSUME	edi:PTR E3D_COLOR
			test	eax, eax
			.WHILE	!ZERO?
				.BREAK	.IF	[edi].rgbMul.dwPixelCode != 0FFFFFFH
				.BREAK	.IF	[edi].rgbAdd.dwPixelCode != 0
				add	edi, SIZEOF E3D_COLOR
				dec	eax
			.ENDW
			.IF	ZERO?
				mov	eax, OFFSET eglRenderPoly@RenderPolygon_LineATXNS486
				xor	ecx, ecx
			.ELSE
			.IF	[ebx].txtimg.fdwFormatType & EIF_WITH_ALPHA
				mov	eax, OFFSET eglRenderPoly@RenderPolygon_LineATX486
				xor	ecx, ecx
			.ELSE
				mov	eax, OFFSET eglRenderPoly@RenderPolygon_LineTX486
			.ENDIF
			.ENDIF
			ASSUME	edi:NOTHING
		.ENDIF
		mov	[ebx].pfnLineFunc, eax
		mov	[ebx].pfnAntialius, ecx
		;
		.IF	[ebx].txtimg.fdwFormatType & EIF_WITH_ALPHA
			.IF	[ebx].nTextureApply != 0
				mov	[ebx].pfnAntialius, NULL
				.IF	edx & E3DSAF_TEXTURE_TRIM
					mov	eax, OFFSET eglRenderPoly@RenderPolygon_StoreMT486
				.ELSE
					mov	eax, OFFSET eglRenderPoly@RenderPolygon_StoreAT486
				.ENDIF
			.ELSE
				.IF	edx & E3DSAF_TEXTURE_TRIM
					mov	eax, OFFSET eglRenderPoly@RenderPolygon_StoreM486
				.ELSE
					mov	eax, OFFSET eglRenderPoly@RenderPolygon_StoreA486
				.ENDIF
			.ENDIF
		.ELSE
			.IF	[ebx].nTextureApply != 0
				mov	[ebx].pfnAntialius, NULL
				mov	eax, OFFSET eglRenderPoly@RenderPolygon_StoreT486
			.ELSE
				.IF	!(edx & E3DSAF_NO_ZBUFFER)
					mov	eax, OFFSET eglRenderPoly@RenderPolygon_Store486
				.ELSE
					mov	nRenderFuncType, RPT_RENDER_POLY_NO_ZBUF
					mov	eax, OFFSET eglRenderPoly@RenderPolygon_StoreNZ486
					mov	[ebx].pfnAntialius, NULL
				.ENDIF
			.ENDIF
		.ENDIF
		mov	[ebx].pfnStoreFunc, eax
	.ELSE
		mov	nRenderFuncType, RPT_RENDER_POLY_NORMAL
		.IF	edx & E3DSAF_ZBUF_ONLY_COMPARE
			mov	nRenderFuncType, RPT_RENDER_POLY_CMP_Z
		.ENDIF
		;
		mov	[ebx].pfnLineFunc, \
			OFFSET eglRenderPoly@RenderPolygon_LineNTX486
		.IF	[ebx].nTextureApply != 0
			mov	eax, OFFSET eglRenderPoly@RenderPolygon_StoreAT486
		.ELSE
			mov	eax, OFFSET eglRenderPoly@RenderPolygon_StoreA486
		.ENDIF
		;
		mov	edx, OFFSET eglRenderPoly@RenderPolygon_Antialias486
		.IF	!([ebx].dwFunctionFlags & E3D_FLAG_ANTIALIAS_SIDE_EDGE)
			xor	edx, edx
		.ENDIF
		.IF	[ebx].nTextureApply == 0
			mov	ecx, [esi].dwProjectedCount
			mov	edi, [esi].pVertexColors
			ASSUME	edi:PTR E3D_COLOR
			test	ecx, ecx
			.WHILE	!ZERO?
				.IF	[edi].rgbMul.dwPixelCode & 0FFFFFFH
					xor	edx, edx
					.BREAK
				.ENDIF
				add	edi, SIZEOF E3D_COLOR
				dec	ecx
			.ENDW
			.IF	ecx == 0
				.IF	!([ebx].dwShadingFlags & E3DSAF_NO_ZBUFFER)
					mov	eax, OFFSET eglRenderPoly@RenderPolygon_Store486
				.ELSE
					mov	eax, OFFSET eglRenderPoly@RenderPolygon_StoreNZ486
				.ENDIF
			.ENDIF
			ASSUME	edi:NOTHING
		.ELSE
			xor	edx, edx
		.ENDIF
		;
		mov	[ebx].pfnAntialius, edx
		mov	[ebx].pfnStoreFunc, eax
	.ENDIF

	ASSUME	ebx:NOTHING
	ASSUME	esi:NOTHING
	mov	eax, nRenderFuncType
	ret

eglRenderPoly@SelectRenderPolygonFunc486 ENDP


;
;	レンダリング準備関数 486 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@PrepareRender	PROC	NEAR32 C USES ebx esi edi,
	hRenderPoly:HEGL_RENDER_POLYGON, pPolyEntry:PCE3D_POLYGON_ENTRY

	LOCAL	dp:EGL_DRAW_PARAM
	LOCAL	axes:EGL_IMAGE_AXES
	LOCAL	rect:EGL_IMAGE_RECT
	LOCAL	ptCenter:EGL_POINT
	LOCAL	dwTemp[4]:DWORD

	mov	ebx, hRenderPoly
	mov	esi, pPolyEntry
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	ASSUME	esi:PCE3D_POLYGON_ENTRY
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
			fld	[esi].surface.mesh.vMinMesh.z
			fcomp	[edi].txmap.rThresholdZ
			xor	ecx, ecx
			xor	edx, edx
			fstsw	ax
			.IF	ax & 0100H	; vMinMesh.z < rThresholdZ
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
			fld1
			mov	eax, 1
			mov	ecx, [ebx].nTextureUVScale
			shl	eax, cl
			mov	dwTemp[0], eax
			fidiv	dwTemp[0]
			fstp	[ebx].rTextureUVScale
		.ELSE
			INVOKE	eglRenderPoly@PrepareImageBuffer , edi, NULL, NULL
		.ENDIF
		ASSUME	edi:NOTHING
		;
		; レンダリング関数選別
		;
		INVOKE	eglRenderPoly@SelectRenderPolygonFunc486 , esi
		cmp	eax, RPT_RENDER_POLY_ERROR
		je	Label_ExitNoDraw
		mov	[ebx].nRenderFuncType, eax
		;
		mov	[ebx].rp.pfnRenderPolygon, \
				OFFSET eglRenderPoly@RenderPolygon_Mesh486
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
		fld	rConst65536
		fld	[esi].surface.image.vImageBase.x
		fmul	st, st(1)
		fistp	ptCenter.x
		fld	[esi].surface.image.vImageBase.y
		fmul	st, st(1)
		fistp	ptCenter.y
		fld	[esi].surface.image.vCenter.x
		fmul	st, st(1)
		fistp	dp.ptBasePos.x
		fld	[esi].surface.image.vCenter.y
		fmul	st, st(1)
		fistp	dp.ptBasePos.y
		fstp	st(0)
		INVOKE	eglGetRevolvedAxes ,
				ADDR axes, ADDR dp.ptBasePos, ADDR ptCenter,
				[esi].surface.image.vEnlarge.x,
				[esi].surface.image.vEnlarge.y,
				[esi].surface.image.rRevolveAngle, rConstHalfPI, 1
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
			[esi].pProjVertexes, [esi].pVertexColors, NULL
	;
	test	eax, eax
	jz	Label_ExitNoDraw
	;
	;	テクスチャパラメータを計算する
	; --------------------------------------------------------------------
	.IF	[esi].dwTypeFlag & E3D_TEXTURE_POLYGON
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
		INVOKE	eglRenderPoly@PrepareParameterWithTexture486
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
		INVOKE	eglRenderPoly@PrepareParameterWithoutTexture486

	.ENDIF
	;
	;	レンダリング関数を振り分ける
	; --------------------------------------------------------------------
	mov	esi, pPolyEntry
	.IF	[esi].dwTypeFlag != E3D_INFINITE_PLANE
		INVOKE	eglRenderPoly@SelectRenderPolygonFunc486 , esi
		mov	eax, pfnRenderFuncTable486[eax*4]
		mov	[ebx].rp.pfnRenderPolygon, eax
		xor	eax, eax
		ret

	.ELSE
		;
		;	無限平面
		; --------------------------------------------------------------------
		mov	[ebx].rp.pfnRenderPolygon, \
			OFFSET eglRenderPoly@RenderPolygon_Plane486
		mov	[ebx].pfnAntialius, NULL
		;
		.IF	edx & E3DSAF_TEXTURE_MAPPING
			.IF	[ebx].lmnimg.dwInfoSize == 0
				mov	eax, OFFSET eglRenderPoly@RenderPolygon_LineTTX486
			.ELSE
				mov	eax, OFFSET eglRenderPoly@RenderPolygon_LineTLTX486
			.ENDIF
		.ELSE
			mov	[ebx].rp.pfnRenderPolygon, \
				OFFSET eglRenderPoly@RenderPolygon_NoDraw
			mov	eax, OFFSET eglRenderPoly@RenderPolygon_LineNTX486
		.ENDIF
		mov	[ebx].pfnStoreFunc, \
				OFFSET eglRenderPoly@RenderPolygon_Store486
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

eglRenderPoly@PrepareRender	ENDP


;
;	レンダリング準備関数
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@PrepareRenderParam	PROC	NEAR32 C USES ebx esi edi,
	hRenderPoly:HEGL_RENDER_POLYGON, pRenderParam:PCE3D_RENDER_PARAM

	LOCAL	pPolyEntry:PE3D_POLYGON_ENTRY
	LOCAL	vRenderPos:E3D_VECTOR4
	LOCAL	pVertexes:PE3D_VECTOR4
	LOCAL	pUVMaps:PE3D_VECTOR_2D
	LOCAL	nVertexCount:DWORD
	LOCAL	pPrimitive:PE3D_PRIMITIVE_POLYGON

	mov	ebx, hRenderPoly
	mov	esi, pRenderParam
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	ASSUME	esi:PCE3D_RENDER_PARAM

	;
	;	ヒープ作成
	; --------------------------------------------------------------------
	.IF	[ebx].hStackHeap == NULL
		INVOKE	eslStackHeapCreate , 4000H, 0, 0
		mov	[ebx].hStackHeap, eax
	.ELSE
		INVOKE	eslStackHeapFree , [ebx].hStackHeap
	.ENDIF
	;
	;	表面属性設定
	; --------------------------------------------------------------------
	mov	eax, [esi].dwFlags
	and	eax, E3DSAF_TEXTURE_TILING \
			OR E3DSAF_TEXTURE_TRIM OR E3DSAF_TEXTURE_SMOOTH
	.IF	[esi].pSrcImage != NULL
		or	eax, E3DSAF_TEXTURE_MAPPING
	.ENDIF
	mov	[ebx].rpSurfAttr.dwShadingFlags, eax
	mov	eax, [esi].rgbaColor.rgbMul.dwPixelCode
	mov	edx, [esi].rgbaColor.rgbAdd.dwPixelCode
	mov	[ebx].rpSurfAttr.rgbaColor.rgbMul.dwPixelCode, eax
	mov	[ebx].rpSurfAttr.rgbaColor.rgbAdd.dwPixelCode, edx
	mov	eax, [esi].pSrcImage
	mov	edx, [esi].pLuminousImage
	mov	[ebx].rpSurfAttr.txmap.pTextureImage, eax
	mov	[ebx].rpSurfAttr.txmap.pLuminousImage, edx
	mov	[ebx].rpSurfAttr.txmap.rThresholdZ, 7F000000H
	mov	[ebx].rpSurfAttr.txmap.nSmallScale, 0
	mov	[ebx].rpSurfAttr.txmap.pSmallImage, eax
	mov	[ebx].rpSurfAttr.txmap.pSmallLuminous, edx
	mov	eax, [esi].nTextureApply
	mov	edx, [esi].nLuminousApply
	mov	[ebx].rpSurfAttr.nTextureApply, eax
	mov	[ebx].rpSurfAttr.nLuminousApply, edx
	mov	eax, [esi].dwTransparency
	mov	[ebx].rpSurfAttr.nTransparency, eax
	xor	eax, eax
	FOR	@MEMBER, <nAmbient, nDiffusion, \
			nSpecular, nSpecularSize, nDeepness>
		mov	[ebx].rpSurfAttr.@MEMBER, eax
	ENDM

	.IF	[esi].dwFlags & E3DRP_RENDER_IMAGE
		;
		;	画像ポリゴン
		; ------------------------------------------------------------
		;
		; UV 座標保持用バッファ確保
		;
		mov	eax, [esi].rev.nViewVertexes
		.IF	(SDWORD PTR eax) < 3
			mov	eax, 4
		.ENDIF
		mov	nVertexCount, eax
		shl	eax, 3		; * (SIZEOF E3D_VECTOR_2D)
		INVOKE	eslStackHeapAllocate , [ebx].hStackHeap, eax
		mov	pUVMaps, eax
		mov	edi, eax
		;
		; UV 座標設定
		;
		ASSUME	edi:PE3D_VECTOR_2D
		mov	ecx, [esi].rev.nViewVertexes
		.IF	(SDWORD PTR ecx) < 3
			xor	eax, eax
			xor	edx, edx
			mov	[edi].x, eax
			mov	[edi].y, edx
			mov	[edi][08H].y, edx
			mov	[edi][18H].x, eax
			;
			mov	edx, [ebx].rpSurfAttr.txmap.pTextureImage
			ASSUME	edx:PEGL_IMAGE_INFO
			fild	[edx].dwImageWidth
			fld1
			fsubp	st(1), st
			fstp	[edi][08H].x
			fild	[edx].dwImageHeight
			fld1
			fsubp	st(1), st
			fstp	[edi][18H].y
			ASSUME	edx:NOTHING
			;
			mov	eax, [edi][08H].x
			mov	edx, [edi][18H].y
			mov	[edi][10H].x, eax
			mov	[edi][10H].y, edx
		.ELSE
			mov	edx, [esi].rev.pViewVertexes
			ASSUME	edx:PCE3D_VECTOR_2D
			.REPEAT
				mov	eax, [edx].x
				mov	[edi].x, eax
				mov	eax, [edx].y
				mov	[edi].y, eax
				add	edx, (SIZEOF E3D_VECTOR_2D)
				add	edi, (SIZEOF E3D_VECTOR_2D)
				dec	ecx
			.UNTIL	ZERO?
			ASSUME	edx:NOTHING
		.ENDIF
		ASSUME	edi:NOTHING
		;
		; 頂点座標用バッファ確保
		;
		mov	eax, nVertexCount
		shl	eax, 4		; * (SIZEOF E3D_VECTOR4)
		INVOKE	eslStackHeapAllocate , [ebx].hStackHeap, eax
		mov	pVertexes, eax
		mov	edi, eax
		;
		; 頂点座標設定
		;
		mov	edx, pUVMaps
		mov	ecx, nVertexCount
		xor	eax, eax
		ASSUME	edx:PE3D_VECTOR_2D
		ASSUME	edi:PE3D_VECTOR4
		.REPEAT
			fld	[edx].x
			fsub	[esi].rev.vRevCenter.x
			fld	[edx].y
			fsub	[esi].rev.vRevCenter.y
			fxch	st(1)
			fstp	[edi].x
			fstp	[edi].y
			mov	[edi].z, eax
			mov	[edi].d, eax
			add	edx, (SIZEOF E3D_VECTOR_2D)
			add	edi, (SIZEOF E3D_VECTOR4)
			dec	ecx
		.UNTIL	ZERO?
		ASSUME	edx:NOTHING
		ASSUME	edi:NOTHING
		;
		; プリミティブ情報用バッファ確保
		;
		mov	eax, nVertexCount
		shl	eax, 4		; * (SIZEOF E3D_PRIMITIVE_VERTEX)
		add	eax, (SIZEOF E3D_PRIMITIVE_POLYGON)
		INVOKE	eslStackHeapAllocate , [ebx].hStackHeap, eax
		mov	pPrimitive, eax
		mov	edi, eax
		;
		; プリミティブ情報設定
		;
		ASSUME	edi:PE3D_PRIMITIVE_POLYGON
		mov	eax, E3D_FLAT_POLYGON
		.IF	[ebx].rpSurfAttr.txmap.pTextureImage != NULL
			or	eax, E3D_TEXTURE_POLYGON
		.ENDIF
		lea	ecx, [ebx].rpSurfAttr
		mov	edx, nVertexCount
		mov	[edi].dwTypeFlag, eax
		mov	[edi].pSurfaceAttr, ecx
		mov	[edi].dwVertexCount, edx
		lea	edi, [edi].polygon[0]
		;
		xor	ecx, ecx
		mov	edx, pUVMaps
		ASSUME	edi:PTR E3D_PRIMITIVE_VERTEX
		ASSUME	edx:PE3D_VECTOR_2D
		.REPEAT
			mov	eax, ecx
			shl	eax, 4	; * (SIZEOF E3D_VECTOR4)
			add	eax, pVertexes
			mov	[edi].vertex, eax
			mov	[edi].normal, 0
			mov	eax, [edx].x
			mov	[edi].uv_map.x, eax
			mov	eax, [edx].y
			mov	[edi].uv_map.y, eax
			add	edx, (SIZEOF E3D_VECTOR_2D)
			add	edi, (SIZEOF E3D_PRIMITIVE_VERTEX)
			inc	ecx
		.UNTIL	ecx >= nVertexCount
		ASSUME	edi:NOTHING
		ASSUME	ecx:NOTHING
		;
		;	行列回転変換
		; ------------------------------------------------------------
		mov	eax, [esi].rev.vRenderPos.x
		mov	edx, [esi].rev.vRenderPos.y
		mov	ecx, [esi].rev.vRenderPos.z
		.IF	[esi].dwFlags & E3DRP_Z_ORDER_SCREEN
			mov	ecx, [ebx].vScreenPos.z
		.ENDIF
		mov	vRenderPos.x, eax
		mov	vRenderPos.y, edx
		mov	vRenderPos.z, ecx
		mov	vRenderPos.d, 0
		;
		.IF	!([esi].dwFlags & E3DRP_NO_SCREEN_ORIGIN)
			fld	vRenderPos.x
			fsub	[ebx].vScreenPos.x
			fstp	vRenderPos.x
			fld	vRenderPos.y
			fsub	[ebx].vScreenPos.y
			fstp	vRenderPos.y
		.ENDIF
		;
		.IF	[esi].rev.pRevMatrix != NULL
			INVOKE	[ebx].rp.pfnPrepareMatrix ,
				ebx, [esi].rev.pRevMatrix, ADDR vRenderPos, NULL
			INVOKE	[ebx].rp.pfnRevolveMatrix ,
				ebx, pVertexes, pVertexes, nVertexCount
		.ELSE
			mov	edi, pVertexes
			mov	ecx, nVertexCount
			ASSUME	edi:PE3D_VECTOR4
			.REPEAT
				FOR	@MEMBER, <x, y, z>
					fld	[edi].@MEMBER
					fadd	vRenderPos.@MEMBER
					fstp	[edi].@MEMBER
				ENDM
				add	edi, (SIZEOF E3D_VECTOR4)
				dec	ecx
			.UNTIL	ZERO?
			ASSUME	edi:NOTHING
		.ENDIF
	.ELSE
		;
		;	プリミティブパラメータ指定
		; ------------------------------------------------------------
		;
		; 頂点座標用バッファ確保
		;
		mov	eax, [esi].poly.nViewVertexes
		mov	nVertexCount, eax
		shl	eax, 4		; * (SIZEOF E3D_VECTOR4)
		INVOKE	eslStackHeapAllocate , [ebx].hStackHeap, eax
		mov	pVertexes, eax
		mov	edi, eax
		;
		; 頂点座標設定
		;
		mov	edx, [esi].poly.pVertexes
		mov	ecx, [esi].poly.nViewVertexes
		xor	eax, eax
		ASSUME	edx:PE3D_VECTOR4
		ASSUME	edi:PE3D_VECTOR4
		.REPEAT
			.IF	[esi].dwFlags & E3DRP_NO_SCREEN_ORIGIN
				mov	eax, [edx].x
				mov	[edi].x, eax
				mov	eax, [edx].y
				mov	[edi].y, eax
			.ELSE
				fld	[edx].x
				fsub	[ebx].vScreenPos.x
				fstp	[edi].x
				fld	[edx].y
				fsub	[ebx].vScreenPos.y
				fstp	[edi].y
			.ENDIF
			.IF	[esi].dwFlags & E3DRP_Z_ORDER_SCREEN
				mov	eax, [ebx].vScreenPos.z
			.ELSE
				mov	eax, [edx].z
			.ENDIF
			mov	[edi].z, eax
			mov	[edi].d, 0
			add	edx, (SIZEOF E3D_VECTOR4)
			add	edi, (SIZEOF E3D_VECTOR4)
			dec	ecx
		.UNTIL	ZERO?
		ASSUME	edx:NOTHING
		ASSUME	edi:NOTHING
		;
		; プリミティブ情報用バッファ確保
		;
		mov	eax, nVertexCount
		shl	eax, 4		; * (SIZEOF E3D_PRIMITIVE_VERTEX)
		add	eax, (SIZEOF E3D_PRIMITIVE_POLYGON)
		INVOKE	eslStackHeapAllocate , [ebx].hStackHeap, eax
		mov	pPrimitive, eax
		mov	edi, eax
		;
		; プリミティブ情報設定
		;
		ASSUME	edi:PE3D_PRIMITIVE_POLYGON
		mov	eax, E3D_FLAT_POLYGON
		.IF	[ebx].rpSurfAttr.txmap.pTextureImage != NULL
			or	eax, E3D_TEXTURE_POLYGON
		.ENDIF
		lea	ecx, [ebx].rpSurfAttr
		mov	edx, [esi].poly.nViewVertexes
		mov	[edi].dwTypeFlag, eax
		mov	[edi].pSurfaceAttr, ecx
		mov	[edi].dwVertexCount, edx
		lea	edi, [edi].polygon[0]
		;
		xor	ecx, ecx
		mov	edx, [esi].poly.pViewVertexes
		ASSUME	edi:PTR E3D_PRIMITIVE_VERTEX
		ASSUME	edx:PCE3D_VECTOR_2D
		.REPEAT
			mov	eax, ecx
			shl	eax, 4	; * (SIZEOF E3D_VECTOR4)
			add	eax, pVertexes
			mov	[edi].vertex, eax
			test	edx, edx
			mov	[edi].normal, 0
			.IF	!ZERO?
				mov	eax, [edx].x
				mov	[edi].uv_map.x, eax
				mov	eax, [edx].y
				mov	[edi].uv_map.y, eax
				add	edx, (SIZEOF E3D_VECTOR_2D)
			.ENDIF
			add	edi, (SIZEOF E3D_PRIMITIVE_VERTEX)
			inc	ecx
		.UNTIL	ecx >= nVertexCount
		ASSUME	edi:NOTHING
		ASSUME	ecx:NOTHING
	.ENDIF
	;
	;	レンダリングポリゴン情報生成
	; --------------------------------------------------------------------
	INVOKE	[ebx].rp.pfnCreatePolygonEntry ,
			ebx, [ebx].hStackHeap, pPrimitive
	mov	pPolyEntry, eax
	.IF	eax == NULL
		mov	[ebx].rp.pfnRenderPolygon, \
				OFFSET eglRenderPoly@RenderPolygon_NoDraw
		xor	eax, eax
		ret
	.ENDIF
	;
	;	頂点色適用
	; --------------------------------------------------------------------
	mov	edi, eax
	ASSUME	edi:PE3D_POLYGON_ENTRY
	mov	eax, [ebx].rpSurfAttr.nTransparency
	mov	[edi].dwTransparency, eax
	.IF	[esi].dwFlags & E3DRP_RENDER_IMAGE
		mov	ecx, nVertexCount
		.IF	ecx > [edi].dwVertexCount
			mov	ecx, [edi].dwVertexCount
		.ENDIF
		mov	edx, [esi].rev.pVertexColors
	.ELSE
		mov	ecx, [esi].poly.nViewVertexes
		.IF	ecx > [edi].dwVertexCount
			mov	ecx, [edi].dwVertexCount
		.ENDIF
		mov	edx, [esi].poly.pVertexColors
	.ENDIF
	.IF	edx != NULL
		mov	edi, [edi].pVertexColors
		ASSUME	edx:PE3D_COLOR
		ASSUME	edi:PE3D_COLOR
		.REPEAT
			mov	eax, [edx].rgbMul.dwPixelCode
			mov	[edi].rgbMul.dwPixelCode, eax
			mov	eax, [edx].rgbAdd.dwPixelCode
			mov	[edi].rgbAdd.dwPixelCode, eax
			add	edx, (SIZEOF E3D_COLOR)
			add	edi, (SIZEOF E3D_COLOR)
			dec	ecx
		.UNTIL	ZERO?
		ASSUME	edx:NOTHING
	.ENDIF
	ASSUME	edi:NOTHING
	;
	;	透視変換・クリッピング
	; --------------------------------------------------------------------
	INVOKE	[ebx].rp.pfnMakeUpPolygon ,
			ebx, [ebx].hStackHeap, pPolyEntry
	.IF	eax == NULL
		mov	[ebx].rp.pfnRenderPolygon, \
				OFFSET eglRenderPoly@RenderPolygon_NoDraw
		xor	eax, eax
		ret
	.ENDIF
	mov	pPolyEntry, eax
	;
	;	レンダリング準備
	; --------------------------------------------------------------------
	mov	edi, [ebx].dib.pZBuffer
	.IF	!([esi].dwFlags & EGL_WITH_Z_ORDER)
		mov	[ebx].dib.pZBuffer, NULL
	.ENDIF
	INVOKE	[ebx].rp.pfnPrepareRender , ebx, pPolyEntry
	mov	[ebx].dib.pZBuffer, edi
	;
	push	eax
	INVOKE	eslStackHeapFree , [ebx].hStackHeap
	pop	eax

	ASSUME	ebx:NOTHING
	ASSUME	esi:NOTHING
	ret

eglRenderPoly@PrepareRenderParam	ENDP

;
;	レンダリング関数（エラー）
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_Err		PROC	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON

	mov	eax, eslErrGeneral
	ret

eglRenderPoly@RenderPolygon_Err	ENDP

;
;	レンダリング関数（描画なし）
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_NoDraw	PROC	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON

	xor	eax, eax
	ret

eglRenderPoly@RenderPolygon_NoDraw	ENDP

;
;	レンダリング関数（2D 描画）
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_Image	PROC	NEAR32 C USES ebx esi edi,
	hRenderPoly:HEGL_RENDER_POLYGON

	mov	ebx, hRenderPoly
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	INVOKE	[ebx].dib.pfnDrawImage, ADDR [ebx].dib
	ASSUME	ebx:NOTHING
	ret

eglRenderPoly@RenderPolygon_Image	ENDP

;
;	無限平面レンダリング 486 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_Plane486	PROC	NEAR32 C USES ebx esi edi,
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
		mov	[ebx].nLineLeft[0], eax
		mov	[ebx].nLeftDecimal, edx
		and	eax, NOT 01H
		mov	[ebx].nLineLeft[4], eax
		mov	eax, [esi].nRight
		mov	edx, [esi].dwReserved2
		mov	[ebx].nLineRight[0], eax
		add	eax, 01H
		mov	[ebx].nRightDecimal, edx
		and	eax, NOT 01H
		mov	[ebx].nLineRight[4], eax
		;
		mov	ecx, [ebx].nLineRight[0]
		xor	edx, edx
		sub	ecx, [ebx].nLineLeft[0]
		mov	eax, 8000H
		inc	ecx
		div	ecx
		;
		@INDEX = 0
		FOR	@MEMBER, <Blue, Green, Red>
			movzx	edx, [esi].rgbaLeft.rgbMul.rgb.@MEMBER
			movzx	ecx, [esi].rgbaRight.rgbMul.rgb.@MEMBER
			sub	ecx, edx
			shl	edx, 7
			imul	ecx, eax
			mov	[ebx].rgbNextColor[@INDEX*2], dx
			movzx	edx, [esi].rgbaLeft.rgbAdd.rgb.@MEMBER
			sar	ecx, 8
			mov	[ebx].rgbDeltaColor[@INDEX*2], cx
			movzx	ecx, [esi].rgbaRight.rgbAdd.rgb.@MEMBER
			sub	ecx, edx
			shl	edx, 7
			imul	ecx, eax
			sar	ecx, 8
			mov	[ebx].rgbNextColor[@INDEX*2+8], dx
			mov	[ebx].rgbDeltaColor[@INDEX*2+8], cx
			@INDEX = @INDEX + 1
		ENDM
		ASSUME	esi:NOTHING
		;
		;	一時ｚバッファ初期化
		; ------------------------------------------------------------
		mov	eax, [ebx].pTempZBuffer[0]
		.IF	[ebx].dib.zbuf.ptrImageArray == eax
			mov	edi, [ebx].dib.ptrZBufLine
			mov	eax, [ebx].nLineLeft[4]
			mov	ecx, [ebx].nLineRight[4]
			lea	edi, [edi + eax * 4]
			sub	ecx, eax
			shr	ecx, 1
			mov	eax, 7F000000H
			inc	ecx
			.REPEAT
				mov	DWORD PTR [edi], eax
				mov	DWORD PTR [edi + 4], eax
				add	edi, 8
				dec	ecx
			.UNTIL	ZERO?
		.ENDIF
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
		sub	ecx, [ebx].nLineLeft[4]
		fld	[ebx].rRenderFogDeepness
		shr	ecx, 1
		test	eax, eax
		.IF	!ZERO?
		inc	ecx
		ASSUME	esi:PTR E3D_TRANS_LINE_BUF
		.REPEAT
			fld	[esi].rZValue[0]
			fld	[esi].rZValue[4]
			fxch	st(1)
			fmul	st, st(2)
			fxch	st(1)
			fmul	st, st(2)
			fxch	st(1)
			fistp	[ebx].nRenderFogApply[0]
			fistp	[ebx].nRenderFogApply[4]
			;
			FOR	@INDEX, <0, 4>
			mov	edi, [ebx].nRenderFogApply[@INDEX]
			.IF	edi != 0
			.IF	edi < 100H
				push	ecx
				neg	edi
				movzx	eax, [esi].rgbAdd[@INDEX].rgb.Blue
				add	edi, 100H
				movzx	ecx, [esi].rgbAdd[@INDEX].rgb.Green
				sub	ax, [ebx].rgbRenderFogColor[0]
				shr	edi, 1
				movzx	edx, [esi].rgbAdd[@INDEX].rgb.Red
				imul	ax, di
				sub	cx, [ebx].rgbRenderFogColor[2]
				imul	cx, di
				sub	dx, [ebx].rgbRenderFogColor[4]
				sar	ax, 7
				imul	dx, di
				add	ax, [ebx].rgbRenderFogColor[0]
				sar	cx, 7
				sar	dx, 7
				add	cx, [ebx].rgbRenderFogColor[2]
				add	dx, [ebx].rgbRenderFogColor[4]
				mov	[esi].rgbAdd[@INDEX].rgb.Blue, al
				mov	[esi].rgbAdd[@INDEX].rgb.Green, cl
				pop	ecx
				mov	[esi].rgbAdd[@INDEX].rgb.Red, dl
			.ELSE
				mov	al, BYTE PTR [ebx].rgbRenderFogColor[0]
				mov	ah, BYTE PTR [ebx].rgbRenderFogColor[2]
				mov	dl, BYTE PTR [ebx].rgbRenderFogColor[4]
				mov	[esi].rgbAdd[@INDEX].rgb.Blue, al
				mov	[esi].rgbAdd[@INDEX].rgb.Green, ah
				mov	[esi].rgbAdd[@INDEX].rgb.Red, dl
			.ENDIF
			.ENDIF
			ENDM
			;
			add	esi, (SIZEOF E3D_TRANS_LINE_BUF)
			dec	ecx
		.UNTIL	ZERO?
		ASSUME	esi:NOTHING
		.ENDIF
		fstp	st(0)
		;
		;	ランバッファを出力
		; ------------------------------------------------------------
		call	[ebx].pfnStoreFunc
		;
		;	次の行へ移動
		; ------------------------------------------------------------
		fld	[ebx].vTxLinePos.y
		fld	[ebx].vTxLinePos.x
		mov	eax, [ebx].dib.ptrDstLine
		fld	[ebx].vTxDeltaY.x
		mov	edx, [ebx].dib.ptrZBufLine
		fld	[ebx].vTxDeltaY.y
		fxch	st(1)
		faddp	st(2), st
		add	eax, [ebx].dib.dstimg.dwBytesPerLine
		add	edx, [ebx].dib.zbuf.dwBytesPerLine
		faddp	st(2), st
		mov	[ebx].dib.ptrDstLine, eax
		mov	[ebx].dib.ptrZBufLine, edx
		fstp	[ebx].vTxLinePos.x
		fstp	[ebx].vTxLinePos.y
		;
		fld	[ebx].rTxLineMod
		mov	esi, [ebx].dib.ptrRegionLine
		fadd	[ebx].vTxxy.y
		mov	ecx, [ebx].dib.nLeftHeight
		add	esi, (SIZEOF E3D_POLY_LINE_REGION)
		fstp	[ebx].rTxLineMod
		dec	ecx
	.UNTIL	ZERO?

	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglRenderPoly@RenderPolygon_Plane486	ENDP


;
;	テクスチャマッピングポリゴンレンダリング 486 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_Texture486	PROC	NEAR32 C USES ebx esi edi,
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
		mov	[ebx].nLineLeft[0], eax
		mov	[ebx].nLeftDecimal, edx
		and	eax, NOT 01H
		mov	[ebx].nLineLeft[4], eax
		mov	eax, [esi].nRight
		mov	edx, [esi].dwReserved2
		mov	[ebx].nLineRight[0], eax
		add	eax, 01H
		mov	[ebx].nRightDecimal, edx
		and	eax, NOT 01H
		mov	[ebx].nLineRight[4], eax
		;
		mov	ecx, [ebx].nLineRight[0]
		xor	edx, edx
		sub	ecx, [ebx].nLineLeft[0]
		mov	eax, 8000H
		inc	ecx
		div	ecx
		;
		@INDEX = 0
		FOR	@MEMBER, <Blue, Green, Red>
			movzx	edx, [esi].rgbaLeft.rgbMul.rgb.@MEMBER
			movzx	ecx, [esi].rgbaRight.rgbMul.rgb.@MEMBER
			sub	ecx, edx
			shl	edx, 7
			imul	ecx, eax
			mov	[ebx].rgbNextColor[@INDEX*2], dx
			movzx	edx, [esi].rgbaLeft.rgbAdd.rgb.@MEMBER
			sar	ecx, 8
			mov	[ebx].rgbDeltaColor[@INDEX*2], cx
			movzx	ecx, [esi].rgbaRight.rgbAdd.rgb.@MEMBER
			sub	ecx, edx
			shl	edx, 7
			imul	ecx, eax
			sar	ecx, 8
			mov	[ebx].rgbNextColor[@INDEX*2+8], dx
			mov	[ebx].rgbDeltaColor[@INDEX*2+8], cx
			@INDEX = @INDEX + 1
		ENDM
		ASSUME	esi:NOTHING
		;
		;	一時ｚバッファ初期化
		; ------------------------------------------------------------
		mov	eax, [ebx].pTempZBuffer[0]
		.IF	[ebx].dib.zbuf.ptrImageArray == eax
			mov	edi, [ebx].dib.ptrZBufLine
			mov	eax, [ebx].nLineLeft[4]
			mov	ecx, [ebx].nLineRight[4]
			lea	edi, [edi + eax * 4]
			sub	ecx, eax
			shr	ecx, 1
			mov	eax, 7F000000H
			inc	ecx
			.REPEAT
				mov	DWORD PTR [edi], eax
				mov	DWORD PTR [edi + 4], eax
				add	edi, 8
				dec	ecx
			.UNTIL	ZERO?
		.ENDIF
		;
		;	ラインバッファにレンダリング
		; ------------------------------------------------------------
		call	[ebx].pfnLineFunc
		;
		;	ランバッファを出力
		; ------------------------------------------------------------
		call	[ebx].pfnStoreFunc
		;
		;	アンチエイリアス
		; ------------------------------------------------------------
		mov	eax, [ebx].pfnAntialius
		.IF	eax != NULL
			call	eax
		.ENDIF
		;
		;	次の行へ移動
		; ------------------------------------------------------------
		fld	[ebx].vTxLinePos.y
		fld	[ebx].vTxLinePos.x
		mov	eax, [ebx].dib.ptrDstLine
		fld	[ebx].vTxDeltaY.x
		mov	edx, [ebx].dib.ptrZBufLine
		fld	[ebx].vTxDeltaY.y
		fxch	st(1)
		faddp	st(2), st
		add	eax, [ebx].dib.dstimg.dwBytesPerLine
		add	edx, [ebx].dib.zbuf.dwBytesPerLine
		faddp	st(2), st
		mov	[ebx].dib.ptrDstLine, eax
		mov	[ebx].dib.ptrZBufLine, edx
		fstp	[ebx].vTxLinePos.x
		fstp	[ebx].vTxLinePos.y
		;
		fld	[ebx].rTxLineMod
		mov	esi, [ebx].dib.ptrRegionLine
		fadd	[ebx].vTxxy.y
		mov	ecx, [ebx].dib.nLeftHeight
		add	esi, (SIZEOF E3D_POLY_LINE_REGION)
		fstp	[ebx].rTxLineMod
		dec	ecx
	.UNTIL	ZERO?

	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglRenderPoly@RenderPolygon_Texture486	ENDP

;
;	テクスチャマッピングポリゴンレンダリング（ｚ無比較） 486 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_TxtNZ486	PROC	NEAR32 C USES ebx esi edi,
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
		mov	[ebx].nLineLeft[0], eax
		mov	[ebx].nLeftDecimal, edx
		and	eax, NOT 01H
		mov	[ebx].nLineLeft[4], eax
		mov	eax, [esi].nRight
		mov	edx, [esi].dwReserved2
		mov	[ebx].nLineRight[0], eax
		add	eax, 01H
		mov	[ebx].nRightDecimal, edx
		and	eax, NOT 01H
		mov	[ebx].nLineRight[4], eax
		;
		mov	ecx, [ebx].nLineRight[0]
		xor	edx, edx
		sub	ecx, [ebx].nLineLeft[0]
		mov	eax, 8000H
		inc	ecx
		div	ecx
		;
		@INDEX = 0
		FOR	@MEMBER, <Blue, Green, Red>
			movzx	edx, [esi].rgbaLeft.rgbMul.rgb.@MEMBER
			movzx	ecx, [esi].rgbaRight.rgbMul.rgb.@MEMBER
			sub	ecx, edx
			shl	edx, 7
			imul	ecx, eax
			mov	[ebx].rgbNextColor[@INDEX*2], dx
			movzx	edx, [esi].rgbaLeft.rgbAdd.rgb.@MEMBER
			sar	ecx, 8
			mov	[ebx].rgbDeltaColor[@INDEX*2], cx
			movzx	ecx, [esi].rgbaRight.rgbAdd.rgb.@MEMBER
			sub	ecx, edx
			shl	edx, 7
			imul	ecx, eax
			sar	ecx, 8
			mov	[ebx].rgbNextColor[@INDEX*2+8], dx
			mov	[ebx].rgbDeltaColor[@INDEX*2+8], cx
			@INDEX = @INDEX + 1
		ENDM
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
		fld	[ebx].vTxLinePos.y
		fld	[ebx].vTxLinePos.x
		mov	eax, [ebx].dib.ptrDstLine
		fld	[ebx].vTxDeltaY.x
		mov	edx, [ebx].dib.ptrZBufLine
		fld	[ebx].vTxDeltaY.y
		fxch	st(1)
		faddp	st(2), st
		add	eax, [ebx].dib.dstimg.dwBytesPerLine
		add	edx, [ebx].dib.zbuf.dwBytesPerLine
		faddp	st(2), st
		mov	[ebx].dib.ptrDstLine, eax
		mov	[ebx].dib.ptrZBufLine, edx
		fstp	[ebx].vTxLinePos.x
		fstp	[ebx].vTxLinePos.y
		;
		fld	[ebx].rTxLineMod
		mov	esi, [ebx].dib.ptrRegionLine
		fadd	[ebx].vTxxy.y
		mov	ecx, [ebx].dib.nLeftHeight
		add	esi, (SIZEOF E3D_POLY_LINE_REGION)
		fstp	[ebx].rTxLineMod
		dec	ecx
	.UNTIL	ZERO?

	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglRenderPoly@RenderPolygon_TxtNZ486	ENDP

;
;	テクスチャマッピングポリゴンレンダリング（ｚ比較のみ）486 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_TxtRZ486	PROC	NEAR32 C USES ebx esi edi,
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
		mov	[ebx].nLineLeft[0], eax
		mov	[ebx].nLeftDecimal, edx
		and	eax, NOT 01H
		mov	[ebx].nLineLeft[4], eax
		mov	eax, [esi].nRight
		mov	edx, [esi].dwReserved2
		mov	[ebx].nLineRight[0], eax
		add	eax, 01H
		mov	[ebx].nRightDecimal, edx
		and	eax, NOT 01H
		mov	[ebx].nLineRight[4], eax
		;
		mov	ecx, [ebx].nLineRight[0]
		xor	edx, edx
		sub	ecx, [ebx].nLineLeft[0]
		mov	eax, 8000H
		inc	ecx
		div	ecx
		;
		@INDEX = 0
		FOR	@MEMBER, <Blue, Green, Red>
			movzx	edx, [esi].rgbaLeft.rgbMul.rgb.@MEMBER
			movzx	ecx, [esi].rgbaRight.rgbMul.rgb.@MEMBER
			sub	ecx, edx
			shl	edx, 7
			imul	ecx, eax
			mov	[ebx].rgbNextColor[@INDEX*2], dx
			movzx	edx, [esi].rgbaLeft.rgbAdd.rgb.@MEMBER
			sar	ecx, 8
			mov	[ebx].rgbDeltaColor[@INDEX*2], cx
			movzx	ecx, [esi].rgbaRight.rgbAdd.rgb.@MEMBER
			sub	ecx, edx
			shl	edx, 7
			imul	ecx, eax
			sar	ecx, 8
			mov	[ebx].rgbNextColor[@INDEX*2+8], dx
			mov	[ebx].rgbDeltaColor[@INDEX*2+8], cx
			@INDEX = @INDEX + 1
		ENDM
		ASSUME	esi:NOTHING
		;
		;	一時ｚバッファ複製
		; ------------------------------------------------------------
		pushfd
		cld
		push	esi
		mov	edi, [ebx].pTempZBuffer[0]
		mov	esi, [ebx].dib.ptrZBufLine
		mov	eax, [ebx].nLineLeft[4]
		mov	ecx, [ebx].nLineRight[4]
		lea	edi, [edi + eax * 4]
		lea	esi, [esi + eax * 4]
		sub	ecx, eax
		add	ecx, 2
		rep	movsd
		pop	esi
		popfd
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
		fld	[ebx].vTxLinePos.y
		fld	[ebx].vTxLinePos.x
		mov	eax, [ebx].dib.ptrDstLine
		fld	[ebx].vTxDeltaY.x
;		mov	edx, [ebx].dib.ptrZBufLine
		pop	edx
		fld	[ebx].vTxDeltaY.y
		fxch	st(1)
		faddp	st(2), st
		add	eax, [ebx].dib.dstimg.dwBytesPerLine
		add	edx, [ebx].dib.zbuf.dwBytesPerLine
		faddp	st(2), st
		mov	[ebx].dib.ptrDstLine, eax
		mov	[ebx].dib.ptrZBufLine, edx
		fstp	[ebx].vTxLinePos.x
		fstp	[ebx].vTxLinePos.y
		;
		fld	[ebx].rTxLineMod
		mov	esi, [ebx].dib.ptrRegionLine
		fadd	[ebx].vTxxy.y
		mov	ecx, [ebx].dib.nLeftHeight
		add	esi, (SIZEOF E3D_POLY_LINE_REGION)
		fstp	[ebx].rTxLineMod
		dec	ecx
	.UNTIL	ZERO?

	ASSUME	ebx:NOTHING
	xor	eax, eax
	ret

eglRenderPoly@RenderPolygon_TxtRZ486	ENDP

;
;	メッシュレンダリング 486 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@RenderPolygon_Mesh486	PROC	NEAR32 C USES ebx esi edi,
	hRenderPoly:HEGL_RENDER_POLYGON

	LOCAL	nLeftPolyCount:DWORD
	LOCAL	pNextMeshEntry:PTR E3D_PRIMITIVE_MESH_POLY
	LOCAL	v1:E3D_VECTOR, v2:E3D_VECTOR
	LOCAL	rTemp:REAL4, nTemp:DWORD
	LOCAL	vNext:E3D_VECTOR4, clrNext:E3D_COLOR
	LOCAL	vLast:E3D_VECTOR4, clrLast:E3D_COLOR
	LOCAL	nZClipLoopCount:DWORD, rZMinClip:REAL4

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
	LOCAL	rZMaxValue:REAL4, rZMinValue:REAL4
	LOCAL	fZClipping:DWORD
	LOCAL	dwVertexCount:DWORD

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
	ASSUME	esi:NOTHING
	;
	mov	ecx, [edx].dwPolyCount
	lea	edx, [edx].mpEntries[0]
	mov	nLeftPolyCount, ecx
	mov	pNextMeshEntry, edx
	;
	test	ecx, ecx
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
		xor	ecx, ecx
		mov	eax, [ebx].rZMinClip
		mov	esi, eax
		sar	eax, 31
		shr	eax, 1
		xor	eax, esi
		mov	rZMaxValue, eax
		mov	rZMinValue, eax
		.REPEAT	
			mov	esi, [edx].dwIndex[ecx*4]
			shl	esi, 4
			add	esi, pMeshVertices
			push	ecx
			ASSUME	esi:PE3D_VECTOR4
			mov	eax, [esi].x
			mov	[edi].x, eax
			mov	eax, [esi].y
			mov	[edi].y, eax
			mov	eax, [esi].z
			mov	[edi].z, eax
			mov	ecx, eax
			mov	[edi].d, 0
			sar	eax, 31
			shr	eax, 1
			xor	eax, ecx
			.IF	(SDWORD PTR eax) < (SDWORD PTR rZMinValue)
				mov	rZMinValue, eax
			.ENDIF
			.IF	(SDWORD PTR eax) > (SDWORD PTR rZMaxValue)
				mov	rZMaxValue, eax
			.ENDIF
			pop	ecx
			add	edi, (SIZEOF E3D_VECTOR4)
			inc	ecx
		.UNTIL	ecx >= [edx].dwVertexCount
		ASSUME	edi:NOTHING
		ASSUME	esi:NOTHING
		;
		mov	eax, [ebx].rZMinClip
		mov	esi, eax
		sar	eax, 31
		shr	eax, 1
		xor	eax, esi
		cmp	rZMaxValue, eax
		jle	Label_Continue
		xor	ecx, ecx
		cmp	rZMinValue, eax
		setl	cl
		mov	fZClipping, ecx
		;
		; 頂点色複製
		;
		mov	edi, pVertexColors
		xor	ecx, ecx
		.REPEAT	
			mov	esi, pMeshColors
			mov	eax, [edx].dwIndex[ecx*4]
			lea	esi, [esi + eax*8]
			mov	eax, DWORD PTR [esi]
			mov	DWORD PTR [edi], eax
			mov	eax, DWORD PTR [esi + 4]
			mov	DWORD PTR [edi + 4], eax
			add	edi, (SIZEOF E3D_COLOR)
			inc	ecx
		.UNTIL	ecx >= [edx].dwVertexCount
		;
		;	裏面ポリゴン判定
		; ------------------------------------------------------------
		;
		; 平面パラメータ計算
		;
		mov	ecx, pVertices
		ASSUME	ecx:PE3D_VECTOR4
		FOR	@MEMBER, <x, y, z>		; v1 を取得
			fld	[ecx + 10H].@MEMBER
			fsub	[ecx].@MEMBER
			fstp	v1.@MEMBER
		ENDM
		FOR	@MEMBER, <x, y, z>		; v2 を取得
			fld	[ecx + 20H].@MEMBER
			fsub	[ecx].@MEMBER
			fstp	v2.@MEMBER
		ENDM
		;
		fld	v1.y
		fmul	v2.z
		fld	v2.y
		fmul	v1.z
		fsubp	st(1), st
		fst	plane.x
		;
		fld	v1.z
		fmul	v2.x
		fld	v2.z
		fmul	v1.x
		fsubp	st(1), st
		fst	plane.y
		;
		fld	v1.x
		fmul	v2.y
		fld	v2.x
		fmul	v1.y
		fsubp	st(1), st
		fst	plane.z
		;
		fld	[ecx].x
		fld	[ecx].y
		fld	[ecx].z
		fmulp	st(3), st
		fmulp	st(3), st
		fmulp	st(3), st
		faddp	st(1), st
		faddp	st(1), st
		fchs
		ASSUME	ecx:NOTHING
		;
		; 法線を正規化
		;
		fld	plane.x
		fld	plane.y
		fld	plane.z
		fld	st(2)
		fmul	st(0), st
		fld	st(2)
		fmul	st(0), st
		fld	st(2)
		fmul	st(0), st
		faddp	st(1), st
		faddp	st(1), st
		fsqrt
		fst	rTemp
		.IF	(DWORD PTR rTemp) < 33800000H	; r < 2^-24
			fstp	st(0)
			fstp	st(0)
			fstp	st(0)
			fstp	st(0)
			fstp	st(0)
			jmp	Label_Continue
		.ENDIF
		fld1
		fdivrp	st(1), st
		;
		fmul	st(1), st
		fmul	st(2), st
		fmul	st(3), st
		fmulp	st(4), st
		fstp	plane.z
		fstp	plane.y
		fstp	plane.x
		fstp	plane.d
		;
		.IF	[ebx].dwShadingFlags & E3DSAF_SINGLE_SIDE_PLANE
			mov	eax, [edx].dwIndex[0]
			shl	eax, 4
			add	eax, pMeshNormals
			ASSUME	eax:PE3D_VECTOR4
			fld	plane.x
			fmul	[eax].x
			fld	plane.y
			fmul	[eax].y
			fld	plane.z
			fmul	[eax].z
			faddp	st(1), st
			faddp	st(1), st
			fstp	rTemp
			mov	eax, plane.d
			xor	eax, rTemp
			js	Label_Continue
			ASSUME	eax:NOTHING
		.ENDIF
		;
		;	基本パラメータ計算
		; ------------------------------------------------------------
		.IF	[ebx].dwShadingFlags & E3DSAF_TEXTURE_MAPPING
			;
			; UV 座標複製
			;
			mov	esi, pMeshUVMaps
			ASSUME	esi:PTR E3D_VECTOR_2D
			mov	edi, pUVMap
			ASSUME	edi:PTR E3D_VECTOR_2D
			mov	eax, [edx].dwIndex[0]
			fld	[esi + eax*8].x
			fmul	[ebx].rTextureUVScale
			fstp	[edi].x
			fld	[esi + eax*8].y
			fmul	[ebx].rTextureUVScale
			fstp	[edi].y
			;
			mov	eax, [edx].dwIndex[4]
			fld	[esi + eax*8].x
			fmul	[ebx].rTextureUVScale
			fstp	[edi + 8].x
			fld	[esi + eax*8].y
			fmul	[ebx].rTextureUVScale
			fstp	[edi + 8].y
			;
			mov	eax, [edx].dwIndex[8]
			fld	[esi + eax*8].x
			fmul	[ebx].rTextureUVScale
			fstp	[edi + 16].x
			fld	[esi + eax*8].y
			fmul	[ebx].rTextureUVScale
			fstp	[edi + 16].y
			ASSUME	esi:NOTHING
			ASSUME	edi:NOTHING
			;
			; テクスチャマッピング行列計算
			;
			INVOKE	eglRenderPoly@SetTextureParameter486 ,
					ADDR txmap, pVertices, pUVMap
		.ENDIF
		;
		;	ｚクリッピング
		; ------------------------------------------------------------
		.IF	fZClipping != 0
			mov	eax, [ebx].rZMinClip
			mov	rZMinClip, eax
			ASSUME	ebx:NOTHING
			mov	ecx, dwVertexCount
			mov	esi, pVertices
			mov	ebx, pVertexColors
			shl	ecx, 4
			sub	esp, ecx
			and	esp, NOT 0FH
			mov	pVertexColors, esp
			lea	eax, [ecx * 2]
			sub	esp, eax
			and	esp, NOT 0FH
			mov	pVertices, esp
			;
			mov	eax, dwVertexCount
			mov	edx, pVertexColors
			mov	edi, pVertices
			;
			ASSUME	esi:PTR E3D_VECTOR4
			ASSUME	ebx:PTR E3D_COLOR
			ASSUME	edi:PTR E3D_VECTOR4
			ASSUME	edx:PTR E3D_COLOR
			mov	nZClipLoopCount, eax
			shl	eax, 3
			mov	ecx, [esi + eax*2 - 10H].x
			mov	vLast.x, ecx
			mov	ecx, [esi + eax*2 - 10H].y
			mov	vLast.y, ecx
			mov	ecx, [esi + eax*2 - 10H].z
			mov	vLast.z, ecx
			mov	ecx, [ebx + eax - 8].rgbMul.dwPixelCode
			mov	clrLast.rgbMul.dwPixelCode, ecx
			mov	ecx, [ebx + eax - 8].rgbAdd.dwPixelCode
			mov	clrLast.rgbAdd.dwPixelCode, ecx
			;
			fld	vLast.z
			fcomp	rZMinClip
			fstsw	ax
			test	ax, 0100H
			jnz	Label_ZClipLoop2
Label_ZClipLoop1:
				mov	eax, [esi].x
				mov	ecx, [esi].y
				mov	vNext.x, eax
				mov	vNext.y, ecx
				mov	eax, [esi].z
				mov	ecx, [ebx].rgbMul.dwPixelCode
				mov	vNext.z, eax
				mov	clrNext.rgbMul.dwPixelCode, ecx
				mov	eax, [ebx].rgbAdd.dwPixelCode
				mov	clrNext.rgbAdd.dwPixelCode, eax
				add	esi, 10H
				add	ebx, 8
				fld	vNext.z
				fcomp	rZMinClip
				fstsw	ax
				.IF	!(ax & 0100H)
Label_ZClipLoop3:
					mov	eax, vNext.x
					mov	ecx, vNext.y
					mov	vLast.x, eax
					mov	vLast.y, ecx
					mov	[edi].x, eax
					mov	[edi].y, ecx
					mov	eax, vNext.z
					mov	ecx, clrNext.rgbMul.dwPixelCode
					mov	vLast.z, eax
					mov	[edi].z, eax
					mov	clrLast.rgbMul.dwPixelCode, ecx
					mov	[edx].rgbMul.dwPixelCode, ecx
					mov	eax, clrNext.rgbAdd.dwPixelCode
					mov	clrLast.rgbAdd.dwPixelCode, eax
					mov	[edx].rgbAdd.dwPixelCode, eax
					add	edi, 10H
					add	edx, 8
					dec	nZClipLoopCount
					jnz	Label_ZClipLoop1
					jmp	Label_ExitZClip
				.ENDIF
				;
				call	Label_ZClipFunc
				;
				mov	eax, vNext.x
				mov	ecx, vNext.y
				mov	vLast.x, eax
				mov	vLast.y, ecx
				mov	eax, vNext.z
				mov	ecx, clrNext.rgbMul.dwPixelCode
				mov	vLast.z, eax
				mov	clrLast.rgbMul.dwPixelCode, ecx
				mov	eax, clrNext.rgbAdd.dwPixelCode
				mov	clrLast.rgbAdd.dwPixelCode, eax
				;
				dec	nZClipLoopCount
				jz	Label_ExitZClip
Label_ZClipLoop2:
				mov	eax, [esi].x
				mov	ecx, [esi].y
				mov	vNext.x, eax
				mov	vNext.y, ecx
				mov	eax, [esi].z
				mov	ecx, [ebx].rgbMul.dwPixelCode
				mov	vNext.z, eax
				mov	clrNext.rgbMul.dwPixelCode, ecx
				mov	eax, [ebx].rgbAdd.dwPixelCode
				mov	clrNext.rgbAdd.dwPixelCode, eax
				add	esi, 10H
				add	ebx, 8
				fld	vNext.z
				fcomp	rZMinClip
				fstsw	ax
				.IF	(ax & 0100H)
					mov	eax, vNext.x
					mov	ecx, vNext.y
					mov	vLast.x, eax
					mov	vLast.y, ecx
					mov	eax, vNext.z
					mov	ecx, clrNext.rgbMul.dwPixelCode
					mov	vLast.z, eax
					mov	clrLast.rgbMul.dwPixelCode, ecx
					mov	eax, clrNext.rgbAdd.dwPixelCode
					mov	clrLast.rgbAdd.dwPixelCode, eax
					dec	nZClipLoopCount
					jnz	Label_ZClipLoop2
					jmp	Label_ExitZClip
				.ENDIF
				;
				call	Label_ZClipFunc
				jmp	Label_ZClipLoop3
Label_ZClipFunc:
			fld	vNext.z
			fsub	rZMinClip
			fld	vNext.z
			fsub	vLast.z
			fdivp	st(1), st
			fld	rConst256
			fmul	st, st(1)
			fistp	nTemp
			;
			fld	vLast.x
			fsub	vNext.x
			fmul	st, st(1)
			fadd	vNext.x
			fstp	[edi].x
			;
			fld	vLast.y
			fsub	vNext.y
			fmulp	st(1), st
			fadd	vNext.y
			fstp	[edi].y
			;
			mov	eax, rZMinClip
			mov	[edi].z, eax
			mov	[edi].d, 0
			;
			FOR	@MEMBER, \
				<rgbMul.rgb.Blue, rgbMul.rgb.Green, rgbMul.rgb.Red, \
				  rgbAdd.rgb.Blue, rgbAdd.rgb.Green, rgbAdd.rgb.Red>
				movzx	eax, clrLast.@MEMBER
				movzx	ecx, clrNext.@MEMBER
				sub	eax, ecx
				imul	eax, nTemp
				sar	eax, 8
				add	eax, ecx
				.IF	(DWORD PTR eax) >= 100H
					sar	eax, 31
					not	eax
				.ENDIF
				mov	[edx].@MEMBER, al
			ENDM
			;
			add	edi, 10H
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
		INVOKE	eglRenderPoly@ProjectScreen486 ,
				ebx, pVertices2D, pVertices, dwVertexCount
		;
		; リージョンを作成する
		;
		INVOKE	eglNormalizePolygonRegion ,
			[ebx].dib.pRegion, ADDR [ebx].dib.rectClip,
			dwVertexCount, pVertices2D, pVertexColors, NULL
		test	eax, eax
		jz	Label_Continue
		;
		;	レンダリングパラメータセットアップ
		; ------------------------------------------------------------
		.IF	[ebx].dwShadingFlags & E3DSAF_TEXTURE_MAPPING
			lea	esi, txmap
			INVOKE	eglRenderPoly@PrepareParameterWithTexture486
			test	eax, eax
			jnz	Label_Continue
		.ELSE
			lea	esi, plane
			INVOKE	eglRenderPoly@PrepareParameterWithoutTexture486
		.ENDIF
		;
		;	レンダリング実行
		; ------------------------------------------------------------
		mov	eax, [ebx].nRenderFuncType
		INVOKE	pfnRenderFuncTable486[eax*4] , ebx
Label_Continue:
		;
		;	次のポリゴンへ
		; ------------------------------------------------------------
		mov	edx, pNextMeshEntry
		ASSUME	edx:PTR E3D_PRIMITIVE_MESH_POLY
		mov	ecx, nLeftPolyCount
		mov	eax, [edx].dwVertexCount
		dec	ecx
		mov	esi, [ebx].pMeshPolygonEntry
		lea	edx, [edx].dwIndex[eax*4]
		mov	nLeftPolyCount, ecx
		mov	pNextMeshEntry, edx
	.ENDW

	ASSUME	ebx:NOTHING
	mov	esp, dwSavedESP
	xor	eax, eax
	ret

eglRenderPoly@RenderPolygon_Mesh486	ENDP


CodeSeg	ENDS

	END
