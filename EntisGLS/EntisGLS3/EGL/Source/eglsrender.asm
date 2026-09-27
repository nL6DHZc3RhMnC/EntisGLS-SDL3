
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

ALIGN	10H
rConstRcp2	REAL4	4 DUP( 0.5 )		; = 1.0 / 2.0
rConstRcp8	REAL4	4 DUP( 0.125 )		; = 1.0 / 8.0
rConstRcpM8	REAL4	4 DUP( -0.125 )		; = - 1.0 / 8.0
rConstRcp128	REAL4	4 DUP( 0.0078125 )	; = 1.0 / 128.0
rConstRcp256	REAL4	4 DUP( 0.00390625 )	; = 1.0 / 256.0
rConstRcp512	REAL4	4 DUP( 0.001953125 )	; = 1.0 / 512.0
rConstRcp8000H	REAL4	4 DUP( 0.000030517578125 )	; = 1.0 / 8000H
rConst1		REAL4	4 DUP( 1.0 )
rConst1minusGap	REAL4	4 DUP( 0.9990234375 )	; = 1.0 - 1.0 / 400H
rConst256	REAL4	4 DUP( 256.0 )
xmmMaskSign	DWORD	4 DUP( 80000000H )
xmmMaskSignBit	DWORD	4 DUP( 7FFFFFFFH )

ALIGN	10H
mmxConstMaskZeroHW	LABEL	MMWORD
		WORD	0FFFFH, 0FFFFH, 0FFFFH, 0
mmxConstMaskLW		LABEL	MMWORD
		WORD	0FFFFH, 0, 0, 0
mmxConst7FFF_100	LABEL	MMWORD
		WORD	4 DUP( 7FFFH - 100H )
mmxConst7FFF_1000	LABEL	MMWORD
		WORD	4 DUP( 7FFFH - 1000H )
mmxConst256	LABEL	MMWORD
		WORD	4 DUP( 100H )

ALIGN	10H
xmmPacked256		REAL4	4 DUP( 256.0 )
xmmPacked1000H		REAL4	4 DUP( 4096.0 )
realConstRcp10000H	REAL4	4 DUP( 0.0000152587890625 )


ConstSeg	ENDS


; ----------------------------------------------------------------------------
;	コードセグメント
; ----------------------------------------------------------------------------

CodeSeg	SEGMENT	PARA READONLY FLAT 'CODE'

;
;	新シェーディング 486 互換コード
; ----------------------------------------------------------------------------

@SaturateNumber	MACRO	reg_name:REQ, low_num:REQ, high_num:REQ, bit_count:=<32>
	IF	low_num
		sub	reg_name, low_num
		.IF	reg_name > (high_num - low_num)
			sar	reg_name, (bit_count - 1)
			not	reg_name
			and	reg_name, (high_num - low_num)
		.ENDIF
		add	reg_name, low_num
	ELSE
		.IF	reg_name > high_num
			sar	reg_name, (bit_count - 1)
			not	reg_name
			and	reg_name, high_num
		.ENDIF
	ENDIF
ENDM

	.486
	.387

ALIGN	10H
eglRenderPoly@ShadeVectors486	PROC	NEAR32 C USES ebx esi edi,
	hRenderPoly:HEGL_RENDER_POLYGON,
	pSurfaceAttribute:PCE3D_SURFACE_ATTRIBUTE,
	pNormals:PCE3D_VECTOR4, pFocusPoints:PCE3D_VECTOR4,
	pColorsLooks:PTR E3D_COLOR, nCount:DWORD

	LOCAL	dwShadingFlags:DWORD		; シェーディングフラグ
	LOCAL	nMaskDoubleSided:DWORD		; 両面ポリゴン用マスク
	LOCAL	nDiffusion:SDWORD		; 拡散反射強度
	LOCAL	nSpecular:SDWORD		; 鏡面反射強度
;	LOCAL	rgbDiffusion:E3D_PALLET_PW	; 拡散反射強度（パック表現）
;	LOCAL	rgbSpecular:E3D_PALLET_PW	; 鏡面反射強度（パック表現）
	LOCAL	rgbAmbient[4]:SDWORD		; 環境光
	LOCAL	rgbLuminanceMul[4]:SDWORD	; 発光色
	LOCAL	rgbLuminanceAdd[4]:SDWORD	; 発光色
	LOCAL	nDeepness:SDWORD		; 透明深度
	LOCAL	nTransparency:SDWORD		; 透明度
	LOCAL	pfnTransparent:PTR NEAR32	; 透明度と透明深度適用

	LOCAL	vcLight:E3D_VECTOR4		; 正規化済光線
	LOCAL	rBrightness:REAL4		; 光源輝度
	LOCAL	rgbMulColor[4]:SDWORD		; 拡散反射 RGB 色成分
	LOCAL	rgbAddColor[4]:SDWORD		; 鏡面反射 RGB 色成分
	LOCAL	vNormal[4]:SDWORD		; 正規化済法線（x-100H）
	LOCAL	vcNormal:E3D_VECTOR4		; 正規化済法線（x-100H）
	LOCAL	vFocus[4]:SDWORD		; 正規化済視線（x100H）
	LOCAL	vcFocus:E3D_VECTOR4		; 正規化済視線（x100H）
	LOCAL	vFocusPos:E3D_VECTOR4		; 焦点座標
	LOCAL	rFocusParam:REAL4		; 視線ベクトルと法線ベクトルの内積

	LOCAL	nTemp[4]:DWORD

	;
	;	表面属性共通パラメータ計算
	; ---------------------------------------------------------------------
	mov	ebx, hRenderPoly
	mov	esi, pSurfaceAttribute
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	ASSUME	esi:PCE3D_SURFACE_ATTRIBUTE
	mov	eax, [esi].dwShadingFlags
	mov	dwShadingFlags, eax
	test	eax, E3DSAF_SHADING_MASK
	jz	Label_Exit
	;
	xor	edx, edx
	test	eax, E3DSAF_SINGLE_SIDE_PLANE
	setz	dl
	neg	edx
	mov	nMaskDoubleSided, edx
	;
	mov	eax, [esi].nDiffusion
	mov	edx, [esi].nSpecular
	mov	nDiffusion, eax
	mov	nSpecular, edx
	;
	@INDEX = 0
	FOR	@MEMBER, <Blue, Green, Red>
		mov	ecx, [esi].nAmbient
		movzx	edx, [ebx].rgbShadowLight.@MEMBER
		movzx	eax, [ebx].rgbAmbientLight.@MEMBER
		imul	ecx, edx
		sar	ecx, 8
		add	eax, ecx
		mov	rgbAmbient[@INDEX], eax
		@INDEX = @INDEX + 4
	ENDM
	;
	@INDEX = 0
	FOR	@MEMBER, <Blue, Green, Red>
		movzx	eax, [esi].rgbaShade.rgbMul.rgb.@MEMBER
		movzx	edx, [ebx].rgbShadowLight.@MEMBER
		imul	eax, edx
		shr	eax, 8
		mov	rgbLuminanceMul[@INDEX], eax
		@INDEX = @INDEX + 4
	ENDM
	@INDEX = 0
	FOR	@MEMBER, <Blue, Green, Red>
		movzx	eax, [esi].rgbaShade.rgbAdd.rgb.@MEMBER
		movzx	edx, [ebx].rgbShadowLight.@MEMBER
		imul	eax, edx
		shr	eax, 8
		mov	rgbLuminanceAdd[@INDEX], eax
		@INDEX = @INDEX + 4
	ENDM
	;
	mov	eax, OFFSET SubFunc_DeepnessDummy
	mov	ecx, [esi].nDeepness
	mov	edx, [esi].nTransparency
	mov	nDeepness, ecx
	mov	nTransparency, edx
	.IF	(SDWORD PTR ecx) != 0
		.IF	dwShadingFlags & E3DSAF_TEXTURE_MAPPING
			mov	eax, OFFSET SubFunc_DeepnessTexture
		.ELSE
			mov	eax, OFFSET SubFunc_DeepnessNoTexture
		.ENDIF
	.ELSEIF	 (SDWORD PTR edx) > 0
		.IF	!(dwShadingFlags & E3DSAF_TEXTURE_MAPPING)
			mov	eax, OFFSET SubFunc_TransparentNoTexture
;		.ELSE
;			mov	eax, OFFSET SubFunc_TransparentTexture
		.ENDIF
	.ENDIF
	mov	pfnTransparent, eax
	;
	ASSUME	esi:NOTHING

	;
	;	ループ開始
	; ---------------------------------------------------------------------
	mov	ecx, nCount
	test	ecx, ecx
	.WHILE	!ZERO?
	;
	mov	nCount, ecx

	;
	;	法線及び焦点ベクトルの正規化と内積の計算
	; ---------------------------------------------------------------------
	mov	esi, pNormals
	mov	edi, pFocusPoints
	ASSUME	esi:PCE3D_VECTOR4
	ASSUME	edi:PCE3D_VECTOR4
	;
	fld	[esi].z			; 法線正規化
	fld	[esi].y
	fld	[esi].x
	fld	st(2)
	fmul	st(0), st
	fld	st(2)
	fmul	st(0), st
	fld	st(2)
	fmul	st(0), st
	faddp	st(2), st
	faddp	st(1), st
	fsqrt
	fld	xmmPacked256
	fchs
	fdivrp	st(1), st
	fmul	st(1), st
	fmul	st(2), st
	fmulp	st(3), st
	fist	vNormal[0]
	fstp	vcNormal.x
	fist	vNormal[4]
	fstp	vcNormal.y
	fist	vNormal[8]
	fstp	vcNormal.z
	;
	fld	[edi].z			; 視線ベクトル正規化
	fst	vFocusPos.z
	fld	[edi].y
	fst	vFocusPos.y
	fld	[edi].x
	fst	vFocusPos.x
	fld	st(2)
	fmul	st(0), st
	fld	st(2)
	fmul	st(0), st
	fld	st(2)
	fmul	st(0), st
	faddp	st(2), st
	faddp	st(1), st
	fsqrt
	fld	xmmPacked256
	fdivrp	st(1), st
	fmul	st(1), st
	fmul	st(2), st
	fmulp	st(3), st
	fist	vFocus[0]
	fstp	vcFocus.x
	fist	vFocus[4]
	fstp	vcFocus.y
	fist	vFocus[8]
	fstp	vcFocus.z
	;
	fld	vcNormal.x		; 内積計算
	fmul	vcFocus.x
	fld	vcNormal.y
	fmul	vcFocus.y
	fld	vcNormal.z
	fmul	vcFocus.z
	faddp	st(2), st
	faddp	st(1), st
	fmul	realConstRcp10000H
	fchs
	fstp	rFocusParam
	;
	;	パラメータ初期化
	; ---------------------------------------------------------------------
	mov	eax, rgbAmbient[0]
	mov	ecx, rgbAmbient[4]
	mov	edx, rgbAmbient[8]
	mov	rgbMulColor[0], eax
	mov	rgbMulColor[4], ecx
	mov	rgbMulColor[8], edx
	xor	eax, eax
	mov	rgbAddColor[0], eax
	mov	rgbAddColor[4], eax
	mov	rgbAddColor[8], eax

	;
	;	無限遠光源の計算
	; ---------------------------------------------------------------------
	mov	ecx, [ebx].nVectorLightCount
	mov	esi, [ebx].pVectorLights
	ASSUME	esi:PE3D_VECTOR_LIGHT_ENTRY
	test	ecx, ecx
	.WHILE	!ZERO?
		push	ecx
		;
		;	拡散反射光成分計算
		; ------------------------------------------------------------
		;
		; 光線ベクトルと法線の内積を計算
		;
		movsx	eax, [esi].vLight.x
		imul	eax, vNormal[0]
		movsx	ecx, [esi].vLight.y
		imul	ecx, vNormal[4]
		movsx	edx, [esi].vLight.z
		imul	edx, vNormal[8]
		add	eax, ecx
		add	eax, edx		; eax <= 輝度 * 10000H
		mov	nTemp[0], eax
		;
		; 判定マスク生成
		;
		imul	eax, nDiffusion
		sar	eax, 8
		mov	edx, eax
		mov	edi, eax
		xor	eax, rFocusParam
		sar	edx, 31
		sar	eax, 31			; eax <= 光面が見えている
		and	edx, nMaskDoubleSided	; edx <= ポリゴンの裏側
		and	eax, edi
		xor	edi, edi
		xor	eax, edx
		sub	eax, edx
		.IF	SIGN?
			mov	eax, edi
		.ENDIF
		.IF	eax > 100000H
			mov	eax, 100000H	; eax <= 輝度 * 10000H [0～100000H]
		.ENDIF
		;
		; 拡散反射成分加算
		;
		movsx	ecx, [esi].rgbColor.Blue
			movsx	edx, [esi].rgbColor.Green
				movsx	edi, [esi].rgbColor.Red
		imul	ecx, eax
			imul	edx, eax
				imul	edi, eax
		sar	ecx, 16
			sar	edx, 16
				sar	edi, 16
		add	rgbMulColor[0], ecx
			add	rgbMulColor[4], edx
				add	rgbMulColor[8], edi
		;
		;	鏡面反射光成分計算
		; ------------------------------------------------------------
		.IF	nSpecular > 0
			;
			; 反射光ベクトルを計算
			;	L' = L - 2 * <L|A> / (|A|^2) * A
			;	   = L - <L|A> / 8000H * A
			;
			; cos^2 θ を計算
			;
			mov	edx, vNormal[0]
				mov	ecx, vNormal[4]
			imul	edx, nTemp[0]
				imul	ecx, nTemp[0]
			movsx	eax, [esi].vLight.x
				movsx	edi, [esi].vLight.y
			sar	edx, 15
				sar	ecx, 15
			sub	eax, edx
					mov	edx, vNormal[8]
					imul	edx, nTemp[0]
				sub	edi, ecx
					movsx	ecx, [esi].vLight.z
			imul	eax, vFocus[0]
					sar	edx, 15
				imul	edi, vFocus[4]
					sub	ecx, edx
			add	eax, edi
					imul	ecx, vFocus[8]
			add	eax, ecx
			sar	eax, 8
			.IF	SIGN?
				;
				; 鏡面反射光成分加算
				;
				imul	eax, eax
				imul	eax, nSpecular
				sar	eax, 8
				.IF	eax > 10000H
					mov	eax, 10000H
				.ENDIF
				movzx	ecx, [esi].rgbColor.Blue
					movzx	edx, [esi].rgbColor.Green
						movzx	edi, [esi].rgbColor.Red
				imul	ecx, eax
					imul	edx, eax
						imul	edi, eax
				sar	ecx, 16
					sar	edx, 16
						sar	edi, 16
				add	rgbAddColor[0], ecx
					add	rgbAddColor[4], edx
						add	rgbAddColor[8], edi
			.ENDIF
		.ENDIF
		;
		pop	ecx
		add	esi, (SIZEOF E3D_VECTOR_LIGHT_ENTRY)
		dec	ecx
	.ENDW

	;
	;	点光源の計算
	; ---------------------------------------------------------------------
	mov	ecx, [ebx].nPointLightCount
	mov	esi, [ebx].pPointLights
	ASSUME	esi:PE3D_POINT_LIGHT_ENTRY
	test	ecx, ecx
	.WHILE	!ZERO?
		push	ecx
		;
		;	光線ベクトルの正規化
		; ------------------------------------------------------------
		mov	eax, pFocusPoints
		ASSUME	eax:PTR E3D_VECTOR4
		fld	[eax].z
		fsub	[esi].vLight.z
		fld	[eax].y
		fsub	[esi].vLight.y
		fld	[eax].x
		fsub	[esi].vLight.x
		;
		fld	st(2)
		fmul	st(0), st
		fld	st(2)
		fmul	st(0), st
		fld	st(2)
		fmul	st(0), st
		faddp	st(2), st
		faddp	st(1), st
		fsqrt
		fld1
		fdivrp	st(1), st
		;
		fmul	st(1), st
		fmul	st(2), st
		fmul	st(3), st
		fmul	[esi].rBrightness
		fstp	rBrightness
		fstp	vcLight.x
		fstp	vcLight.y
		fstp	vcLight.z
		;
		;	拡散反射光成分計算
		; ------------------------------------------------------------
		;
		; 光線ベクトルと法線の内積を計算
		;
		fld	vcNormal.x
		fmul	vcLight.x
		fld	vcNormal.y
		fmul	vcLight.y
		fld	vcNormal.z
		fmul	vcLight.z
		faddp	st(2), st
		faddp	st(1), st
		;
		fst	nTemp[0]
		fmul	rBrightness
		fild	nDiffusion
		fmulp	st(1), st		; 輝度 * 10000H
		;
		; 判定マスク生成
		;
		fistp	nTemp[4]
		mov	eax, nTemp[4]
		mov	edx, eax
		mov	edi, eax
		xor	eax, rFocusParam
		sar	edx, 31
		sar	eax, 31			; eax <= 光面が見えている
		and	edx, nMaskDoubleSided	; edx <= ポリゴンの裏側
		and	eax, edi
		xor	edi, edi
		xor	eax, edx
		sub	eax, edx
		.IF	SIGN?
			mov	eax, edi
		.ENDIF
		.IF	eax > 100000H
			mov	eax, 100000H	; eax <= 輝度 * 10000H [0～100000H]
		.ENDIF
		;
		; 拡散反射成分加算
		;
		movsx	ecx, [esi].rgbColor.Blue
			movsx	edx, [esi].rgbColor.Green
				movsx	edi, [esi].rgbColor.Red
		imul	ecx, eax
			imul	edx, eax
				imul	edi, eax
		sar	ecx, 16
			sar	edx, 16
				sar	edi, 16
		add	rgbMulColor[0], ecx
			add	rgbMulColor[4], edx
				add	rgbMulColor[8], edi
		;
		;	鏡面反射光成分計算
		; ------------------------------------------------------------
		.IF	nSpecular > 0
			;
			; 反射光ベクトルを計算
			;	L' = L - 2 * <L|A> / (|A|^2) * A
			;	   = L - <L|A> / 8000H * A
			; cos^2 θ を計算
			;
			fld	vcLight.x
			fld	nTemp[0]
			fmul	rConstRcp8000H
			fst	nTemp[0]
			fmul	vcNormal.x
			fsubp	st(1), st
			fmul	vcFocus.x
			;
			fld	vcLight.y
			fld	nTemp[0]
			fmul	vcNormal.y
			fsubp	st(1), st
			fmul	vcFocus.y
			faddp	st(1), st
			;
			fld	vcLight.z
			fld	nTemp[0]
			fmul	vcNormal.z
			fsubp	st(1), st
			fmul	vcFocus.z
			faddp	st(1), st	; cosθ * 100H
			;
			fistp	nTemp[8]
			mov	eax, nTemp[8]
			test	eax, eax
			.IF	SIGN?
				;
				; 鏡面反射光成分加算
				;
				imul	eax, eax
				imul	eax, nSpecular
				sar	eax, 8
				.IF	eax > 10000H
					mov	eax, 10000H
				.ENDIF
				movzx	ecx, [esi].rgbColor.Blue
					movzx	edx, [esi].rgbColor.Green
						movzx	edi, [esi].rgbColor.Red
				imul	ecx, eax
					imul	edx, eax
						imul	edi, eax
				sar	ecx, 16
					sar	edx, 16
						sar	edi, 16
				add	rgbAddColor[0], ecx
					add	rgbAddColor[4], edx
						add	rgbAddColor[8], edi
			.ENDIF
		.ENDIF
		;
		pop	ecx
		add	esi, (SIZEOF E3D_POINT_LIGHT_ENTRY)
		dec	ecx
	.ENDW

	;
	;	拡散反射光・鏡面反射光適用
	; --------------------------------------------------------------------
	mov	esi, pColorsLooks
	ASSUME	esi:PTR E3D_COLOR
	;
	; 発光成分(ADD) = 発光成分 * 拡散反射 + 鏡面反射
	; 透明度成分(MUL) = 透明度成分 * 拡散反射
	;
	movzx	edi, [esi].rgbAdd.rgb.Blue
	movzx	ecx, [esi].rgbMul.rgb.Blue
	mov	eax, rgbMulColor[0]
	mov	edx, rgbAddColor[0]
	sub	edi, rgbLuminanceAdd[0]
	sub	ecx, rgbLuminanceMul[0]
	imul	edi, eax
	imul	ecx, eax
	sar	edi, 8
	sar	ecx, 8
	add	edx, edi
	add	ecx, rgbLuminanceMul[0]
	add	edx, rgbLuminanceAdd[0]
	mov	rgbMulColor[0], ecx
	mov	rgbAddColor[0], edx
	;
	movzx	edi, [esi].rgbAdd.rgb.Green
	movzx	ecx, [esi].rgbMul.rgb.Green
	mov	eax, rgbMulColor[4]
	mov	edx, rgbAddColor[4]
	sub	edi, rgbLuminanceAdd[4]
	sub	ecx, rgbLuminanceMul[4]
	imul	edi, eax
	imul	ecx, eax
	sar	edi, 8
	sar	ecx, 8
	add	edx, edi
	add	ecx, rgbLuminanceMul[4]
	add	edx, rgbLuminanceAdd[4]
	mov	rgbMulColor[4], ecx
	mov	rgbAddColor[4], edx
	;
	movzx	edi, [esi].rgbAdd.rgb.Red
	movzx	ecx, [esi].rgbMul.rgb.Red
	mov	eax, rgbMulColor[8]
	mov	edx, rgbAddColor[8]
	sub	edi, rgbLuminanceAdd[8]
	sub	ecx, rgbLuminanceMul[8]
	imul	edi, eax
	imul	ecx, eax
	sar	edi, 8
	sar	ecx, 8
	add	edx, edi
	add	ecx, rgbLuminanceMul[8]
	add	edx, rgbLuminanceAdd[8]
	mov	rgbMulColor[8], ecx
	mov	rgbAddColor[8], edx

	;
	;	擬似フォッグ適用
	; --------------------------------------------------------------------
	fld	vFocusPos.z
	fsub	[ebx].rFogBiasZ
	fmul	[ebx].rFogDeepness
	fistp	nTemp[0]
	mov	eax, nTemp[0]
	.IF	(SDWORD PTR eax) > 0
		.IF	(SDWORD PTR eax) < 100H
			;
			; 発光成分(ADD) = (フォッグ色 - 発光成分) * Z適用度 + 発光成分
			; 透明度成分(MUL) = 透明度成分 * (1 - Z適用度)
			;
			movzx	ecx, [ebx].rgbFogColor.Blue
				movzx	edx, [ebx].rgbFogColor.Green
					movzx	edi, [ebx].rgbFogColor.Red
			sub	ecx, rgbAddColor[0]
				sub	edx, rgbAddColor[4]
					sub	edi, rgbAddColor[8]
			imul	ecx, eax
				imul	edx, eax
					imul	edi, eax
			sar	ecx, 8
				sar	edx, 8
					sar	edi, 8
			add	rgbAddColor[0], ecx
				add	rgbAddColor[4], edx
					add	rgbAddColor[8], edi
			;
			neg	eax
			add	eax, 100H
			mov	ecx, rgbMulColor[0]
				mov	edx, rgbMulColor[4]
					mov	edi, rgbMulColor[8]
			imul	ecx, eax
				imul	edx, eax
					imul	edi, eax
			sar	ecx, 8
				sar	edx, 8
					sar	edi, 8
			mov	rgbMulColor[0], ecx
				mov	rgbMulColor[4], edx
					mov	rgbMulColor[8], edi
		.ELSE
			movzx	eax, [ebx].rgbFogColor.Blue
			movzx	ecx, [ebx].rgbFogColor.Green
			movzx	edx, [ebx].rgbFogColor.Red
			mov	rgbAddColor[0], eax
			mov	rgbAddColor[4], ecx
			mov	rgbAddColor[8], edx
			xor	eax, eax
			mov	rgbMulColor[0], eax
			mov	rgbMulColor[4], ecx
			mov	rgbMulColor[8], edx
		.ENDIF
	.ENDIF

	;
	;	透明度および透明深度適用
	; --------------------------------------------------------------------
	mov	ecx, nTransparency
	call	pfnTransparent

	;
	;	色情報出力
	; --------------------------------------------------------------------
	mov	eax, rgbMulColor[0]
	sub	eax, 0FFH
	mov	edx, eax
	sar	eax, 31
	not	eax
	and	eax, edx
	add	rgbAddColor[0], eax
	;
	mov	eax, rgbMulColor[4]
	sub	eax, 0FFH
	mov	edx, eax
	sar	eax, 31
	not	eax
	and	eax, edx
	add	rgbAddColor[4], eax
	;
	mov	eax, rgbMulColor[8]
	sub	eax, 0FFH
	mov	edx, eax
	sar	eax, 31
	not	eax
	and	eax, edx
	add	rgbAddColor[8], eax
	;
	mov	eax, rgbMulColor[0]
	@SaturateNumber	eax, 0, 0FFH
	mov	[esi].rgbMul.rgb.Blue, al
	;
	mov	eax, rgbMulColor[4]
	@SaturateNumber	eax, 0, 0FFH
	mov	[esi].rgbMul.rgb.Green, al
	;
	mov	eax, rgbMulColor[8]
	@SaturateNumber	eax, 0, 0FFH
	mov	[esi].rgbMul.rgb.Red, al
	;
	mov	eax, rgbAddColor[0]
	@SaturateNumber	eax, 0, 0FFH
	mov	[esi].rgbAdd.rgb.Blue, al
	;
	mov	eax, rgbAddColor[4]
	@SaturateNumber	eax, 0, 0FFH
	mov	[esi].rgbAdd.rgb.Green, al
	;
	mov	eax, rgbAddColor[8]
	@SaturateNumber	eax, 0, 0FFH
	mov	[esi].rgbAdd.rgb.Red, al

	mov	eax, pNormals
	mov	edx, pFocusPoints
	mov	ecx, nCount
	add	eax, (SIZEOF E3D_VECTOR4)
	add	edx, (SIZEOF E3D_VECTOR4)
	add	esi, (SIZEOF E3D_COLOR)
	mov	pNormals, eax
	mov	pFocusPoints, edx
	mov	pColorsLooks, esi

	ASSUME	esi:NOTHING
	dec	ecx

	.ENDW
	;
Label_Exit:
	xor	eax, eax
	ret

ALIGN	10H
SubFunc_DeepnessNoTexture:
	;
	;	透明深度適用
	; --------------------------------------------------------------------
	fld	rFocusParam
	fmul	st(0), st
	fild	nDeepness
	fmulp	st(1), st
	fistp	nTemp[0]
	add	ecx, nTemp[0]
	.IF	(SDWORD PTR ecx) > 0

SubFunc_TransparentNoTexture:
	;
	;	透明度適用
	; --------------------------------------------------------------------
	neg	ecx
	add	ecx, 100H
	.IF	SIGN?
		xor	ecx, ecx
	.ENDIF
	;
	mov	eax, rgbMulColor[0]
		mov	edx, rgbMulColor[4]
			mov	edi, rgbMulColor[8]
	sub	eax, 100H
		sub	edx, 100H
			sub	edi, 100H
	imul	eax, ecx
		imul	edx, ecx
			imul	edi, ecx
	sar	eax, 8
		sar	edx, 8
			sar	edi, 8
	add	eax, 100H
		add	edx, 100H
			add	edi, 100H
	mov	rgbMulColor[0], eax
		mov	rgbMulColor[4], edx
			mov	rgbMulColor[8], edi
	;
	mov	eax, rgbAddColor[0]
		mov	edx, rgbAddColor[4]
			mov	edi, rgbAddColor[8]
	imul	eax, ecx
		imul	edx, ecx
			imul	edi, ecx
	sar	eax, 8
		sar	edx, 8
			sar	edi, 8
	mov	rgbAddColor[0], eax
		mov	rgbAddColor[4], edx
			mov	rgbAddColor[8], edi

	.ENDIF
SubFunc_DeepnessDummy:
	BYTE	0C3H	; ret

ALIGN	10H
SubFunc_DeepnessTexture:
	;
	;	透明深度適用
	; --------------------------------------------------------------------
	fld	rFocusParam
	fmul	st(0), st
	fild	nDeepness
	fmulp	st(1), st
	fistp	nTemp[0]
	add	ecx, nTemp[0]
	.IF	(SDWORD PTR ecx) > 0

SubFunc_TransparentTexture:
	;
	;	透明度適用
	; --------------------------------------------------------------------
	neg	ecx
	add	ecx, 100H
	.IF	SIGN?
		xor	ecx, ecx
	.ENDIF
	;
	mov	eax, rgbMulColor[0]
		mov	edx, rgbMulColor[4]
			mov	edi, rgbMulColor[8]
	imul	eax, ecx
		imul	edx, ecx
			imul	edi, ecx
	sar	eax, 8
		sar	edx, 8
			sar	edi, 8
	mov	rgbMulColor[0], eax
		mov	rgbMulColor[4], edx
			mov	rgbMulColor[8], edi
	;
	mov	eax, rgbAddColor[0]
		mov	edx, rgbAddColor[4]
			mov	edi, rgbAddColor[8]
	imul	eax, ecx
		imul	edx, ecx
			imul	edi, ecx
	sar	eax, 8
		sar	edx, 8
			sar	edi, 8
	mov	rgbAddColor[0], eax
		mov	rgbAddColor[4], edx
			mov	rgbAddColor[8], edi

	.ENDIF

	BYTE	0C3H	; ret

eglRenderPoly@ShadeVectors486	ENDP


;
;	新シェーディング SSE 専用コード
; ----------------------------------------------------------------------------

	.686
	.XMM

ALIGN	10H
eglRenderPoly@ShadeVectorsProcSSE	PROC	NEAR32 C USES ebx esi edi,
	hRenderPoly:HEGL_RENDER_POLYGON,
	pSurfaceAttribute:PCE3D_SURFACE_ATTRIBUTE,
	pNormals:PCE3D_VECTOR4, pFocusPoints:PCE3D_VECTOR4,
	pColorsLooks:PTR E3D_COLOR, nCount:DWORD

	INVOKE	eglRenderPoly@ShadeVectorsSSE,
			hRenderPoly, pSurfaceAttribute,
			pNormals, pFocusPoints, pColorsLooks, nCount, NULL
	emms
	ret

eglRenderPoly@ShadeVectorsProcSSE	ENDP

ALIGN	10H
eglRenderPoly@ShadeVectorsSSE	PROC	NEAR32 C USES ebx esi edi,
	hRenderPoly:HEGL_RENDER_POLYGON,
	pSurfaceAttribute:PCE3D_SURFACE_ATTRIBUTE,
	pNormals:PCE3D_VECTOR4, pFocusPoints:PCE3D_VECTOR4,
	pColorsLooks:PTR E3D_COLOR, nCount:DWORD,
	pTargetPlane:PE3D_VECTOR4

	LOCAL	dwShadingFlags:DWORD		; シェーディングフラグ
	LOCAL	nMaskDoubleSided:DWORD		; 両面ポリゴン用マスク
	LOCAL	nDiffusion:SDWORD		; 拡散反射強度
	LOCAL	nSpecular:SDWORD		; 鏡面反射強度
	LOCAL	nSpecularSizeScale:SDWORD	; 鏡面反射サイズ係数 (x10000H)
	LOCAL	nSpecularSizeBias:SDWORD
	LOCAL	nShadowAlpha:DWORD		; シャドウの薄さ(x100H)
	LOCAL	rgbDiffusion:E3D_PALLET_PW	; 拡散反射強度（パック表現）
	LOCAL	rgbSpecular:E3D_PALLET_PW	; 鏡面反射強度（パック表現）
	LOCAL	rgbAmbient:E3D_PALLET_PW	; 環境光
	LOCAL	rgbLuminance:E3D_COLOR_PW	; 発光色
	LOCAL	nDeepness:SDWORD		; 透明深度
	LOCAL	nTransparency:SDWORD		; 透明度
	LOCAL	pfnTransparent:PTR NEAR32	; 透明度と透明深度適用

	LOCAL	vcLight:E3D_VECTOR4		; 正規化済光線
	LOCAL	rBrightness:REAL4		; 光源輝度
	LOCAL	rgbMulColor:E3D_PALLET_PW	; 拡散反射 RGB 色成分
	LOCAL	rgbAddColor:E3D_PALLET_PW	; 鏡面反射 RGB 色成分
	LOCAL	vNormal:E3D_VECTOR_PW		; 正規化済法線（x-100H）
	LOCAL	vcNormal:E3D_VECTOR4		; 正規化済法線（x-100H）
	LOCAL	vFocus:E3D_VECTOR_PW		; 正規化済視線（x100H）
	LOCAL	vcFocus:E3D_VECTOR4		; 正規化済視線（x100H）
	LOCAL	vFocusPos:E3D_VECTOR4		; 焦点座標
	LOCAL	rFocusParam:REAL4		; 視線ベクトルと法線ベクトルの内積
	LOCAL	rFocusPlaneParam:REAL4		; 視線ベクトルと平面法線ベクトルの内積
	LOCAL	nTemp:DWORD

	;
	;	表面属性共通パラメータ計算
	; ---------------------------------------------------------------------
	mov	ebx, hRenderPoly
	mov	esi, pSurfaceAttribute
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	ASSUME	esi:PCE3D_SURFACE_ATTRIBUTE
	mov	eax, [esi].dwShadingFlags
	mov	dwShadingFlags, eax
	test	eax, E3DSAF_SHADING_MASK
	jz	Label_Exit
	;
	xor	edx, edx
	test	eax, E3DSAF_SINGLE_SIDE_PLANE
	setz	dl
	neg	edx
	mov	nMaskDoubleSided, edx
	;
	movq	mm6, mmxConst7FFF_1000
	movq	mm7, mmxConstMaskZeroHW
	movd	mm0, [esi].nDiffusion
	movd	mm1, [esi].nSpecular
		movd	mm2, [esi].nAmbient
	paddsw	mm0, mm6
	paddsw	mm1, mm6
		mov	edx, [esi].nSpecularSize
		mov	ecx, 100H
	psubusw	mm0, mm6
	psubusw	mm1, mm6
		cmp	edx, ecx
		cmova	edx, ecx
		shl	edx, 8
		mov	nSpecularSizeBias, edx
		shr	edx, 8
		xor	eax, eax
		sub	ecx, edx
		setz	al
		xor	edx, edx
		add	ecx, eax
		mov	eax, 10000H
	pand	mm0, mmxConstMaskLW
	pand	mm1, mmxConstMaskLW
		div	ecx
		movq	mm5, MMWORD PTR [ebx].rgbShadowLight
		pshufw	mm2, mm2, 0
		psllw	mm5, 4
		psllw	mm2, 4
		pmulhw	mm2, mm5
	movd	nDiffusion, mm0
	movd	nSpecular, mm1
		paddsw	mm2, MMWORD PTR [ebx].rgbAmbientLight
	pshufw	mm0, mm0, 0
	pshufw	mm1, mm1, 0
	pand	mm0, mm7
	pand	mm1, mm7
	movq	MMWORD PTR rgbDiffusion, mm0
	movq	MMWORD PTR rgbSpecular, mm1
		movq	MMWORD PTR rgbAmbient, mm2
	;
	movq	mm6, MMWORD PTR [esi].rgbaShade
	pxor	mm4, mm4
	movq	mm7, mm6
	punpcklbw	mm6, mm4
	punpckhbw	mm7, mm4
	movq	mm5, MMWORD PTR [ebx].rgbShadowLight
		neg	eax
	pmullw	mm6, mm5
	pmullw	mm7, mm5
	psrlw	mm6, 8
	psrlw	mm7, 8
	movq	MMWORD PTR rgbLuminance.rgbMul, mm6
	movq	MMWORD PTR rgbLuminance.rgbAdd, mm7
		mov	nSpecularSizeScale, eax
	;
	mov	eax, OFFSET SubFunc_DeepnessDummy
	mov	ecx, [esi].nDeepness
	mov	edx, [esi].nTransparency
	mov	nDeepness, ecx
	mov	nTransparency, edx
	.IF	(SDWORD PTR ecx) != 0
		mov	eax, dwShadingFlags
		and	eax, (E3DSAF_PHONG_SHADE OR E3DSAF_GOURAUD_SHADE)
		.IF	(dwShadingFlags & E3DSAF_TEXTURE_MAPPING) \
					&& (eax != E3DSAF_PHONG_SHADE)
			mov	eax, OFFSET SubFunc_DeepnessTexture
		.ELSE
			mov	eax, OFFSET SubFunc_DeepnessNoTexture
		.ENDIF
	.ELSEIF	 (SDWORD PTR edx) > 0
		mov	edx, dwShadingFlags
		and	edx, (E3DSAF_PHONG_SHADE OR E3DSAF_GOURAUD_SHADE)
		.IF	!(dwShadingFlags & E3DSAF_TEXTURE_MAPPING) \
				|| (edx == E3DSAF_PHONG_SHADE)
			mov	eax, OFFSET SubFunc_TransparentNoTexture
;		.ELSE
;			mov	eax, OFFSET SubFunc_TransparentTexture
		.ENDIF
	.ENDIF
	mov	pfnTransparent, eax
	;
	ASSUME	esi:NOTHING

	;
	;	ループ開始
	; ---------------------------------------------------------------------
	mov	ecx, nCount
	test	ecx, ecx
	.WHILE	!ZERO?
	;
	mov	nCount, ecx

	;
	;	法線及び焦点ベクトルの正規化と内積の計算
	; ---------------------------------------------------------------------
	mov	esi, pNormals
	mov	edi, pFocusPoints
	movups	xmm4, [esi]
	movups	xmm0, [edi]
		movaps	xmm7, xmm4
		movaps	xmm3, xmm0
		movups	vFocusPos, xmm0
	mulps	xmm0, xmm0
		mulps	xmm4, xmm4
	movhlps	xmm1, xmm0
		movhlps	xmm5, xmm4
	movss	xmm2, xmm0
		movss	xmm6, xmm4
	shufps	xmm0, xmm0, 1
		shufps	xmm4, xmm4, 1
	addss	xmm1, xmm2
		addss	xmm5, xmm6
	addss	xmm0, xmm1
		addss	xmm4, xmm5
	rsqrtss	xmm0, xmm0
		rsqrtss	xmm4, xmm4
	shufps	xmm0, xmm0, 0
		shufps	xmm4, xmm4, 0
	mulps	xmm0, xmm3			; 正規化済焦点（視線）ベクトル
				mov	eax, pTargetPlane
		mulps	xmm4, xmm7		; 正規化済法線ベクトル
			movaps	xmm1, xmm0
				test	eax, eax
			mulps	xmm0, xmm4
				cmovz	eax, pNormals
	mulps	xmm1, xmmPacked256
			movhlps	xmm2, xmm0
		mulps	xmm4, xmmPacked256
			movss	xmm3, xmm0
			shufps	xmm0, xmm0, 1
				movups	xmm7, vFocusPos
			addss	xmm2, xmm3
	movups	vcFocus, xmm1
			addss	xmm0, xmm2
				movups	xmm6, [eax]
		xorps	xmm4, xmmMaskSign
			movss	rFocusParam, xmm0
		movups	vcNormal, xmm4
				mulps	xmm6, xmm7
	movhlps	xmm0, xmm1
		movhlps	xmm5, xmm4
	cvtps2pi	mm1, xmm1
		cvtps2pi	mm4, xmm4
				movhlps	xmm7, xmm6
	cvtps2pi	mm0, xmm0
		cvtps2pi	mm5, xmm5
	packssdw	mm1, mm0
		packssdw	mm4, mm5
				addss	xmm7, xmm6
				shufps	xmm6, xmm6, 1
	pand	mm1, mmxConstMaskZeroHW
		pand	mm4, mmxConstMaskZeroHW
				addss	xmm6, xmm7
	movq	MMWORD PTR vFocus, mm1
		movq	MMWORD PTR vNormal, mm4
				movss	rFocusPlaneParam, xmm6

	;
	;	パラメータ初期化
	; ---------------------------------------------------------------------
	movq	mm6, MMWORD PTR rgbAmbient
	pxor	mm7, mm7
;	movq	MMWORD PTR rgbMulColor, mm6
;	movq	MMWORD PTR rgbAddColor, mm7

	;
	;	無限遠光源の計算
	; ---------------------------------------------------------------------
	mov	ecx, [ebx].nVectorLightCount
	mov	esi, [ebx].pVectorLights
	ASSUME	esi:PE3D_VECTOR_LIGHT_ENTRY
	test	ecx, ecx
	.WHILE	!ZERO?
		;
		;	シャドウマッピングテスト
		; ------------------------------------------------------------
		mov	edi, [esi].ShadowMap.pShadowMap
		.IF	edi != NULL
			;
			; xmm1 = vDelta = vFocusPos - vLightPos
			;			 = 光源方向へのベクトル
			;
			movups	xmm0, [esi].ShadowMap.vLightPos
			movups	xmm1, vFocusPos
				movaps	xmm5, xmm0
			subps	xmm1, xmm0
			;
			; シャドウマップ平面との交点計算
			;   xmm2 = z = <vDelta | vLightRay>
			;   xmm3 = t = rMapDistance / z
			;   xmm4 = vHitPos = vDelta * t + vLightPos
			;
			movups	xmm2, [esi].ShadowMap.vLightRay
			movaps	xmm4, xmm1
			mulps	xmm2, xmm1
				movss	xmm3, [esi].ShadowMap.rMapDistance
			movhlps	xmm1, xmm2
			movss	xmm7, xmm2
			shufps	xmm2, xmm2, 1
			addss	xmm1, xmm7
			addss	xmm2, xmm1
				divss	xmm3, xmm2
			shufps	xmm3, xmm3, 0
			mulps	xmm4, xmm3
			addps	xmm4, xmm5
			;
			; シャドウマップ UV の計算
			;   vHitMapPos = vHitPos - vOriginPos
			;   xmm0 = xMap = <vHitMapPos | vAxisX>
			;   xmm1 = yMap = <vHitMapPos | vAxisY>
			;
			movups	xmm5, [esi].ShadowMap.vOriginPos
			movups	xmm6, [esi].ShadowMap.vAxisX
			movups	xmm7, [esi].ShadowMap.vAxisY
			subps	xmm4, xmm5
			mulps	xmm6, xmm4
			mulps	xmm7, xmm4
			;
			movhlps	xmm4, xmm6
			movhlps	xmm5, xmm7
			movss	xmm0, xmm6
			movss	xmm1, xmm7
			shufps	xmm6, xmm6, 1
			shufps	xmm7, xmm7, 1
			addss	xmm0, xmm4
			addss	xmm1, xmm5
			addss	xmm0, xmm6
			addss	xmm1, xmm7
			;
			; z 値比較
			;
			ASSUME	edi:PEGL_IMAGE_INFO
			cvttss2si	eax, xmm0
			cvttss2si	edx, xmm1
			;
			.IF	(eax < DWORD PTR [edi].dwImageWidth) && \
					(edx < DWORD PTR [edi].dwImageHeight)
				cvtsi2ss	xmm4, eax
					movups	xmm6, [esi].vcLight
				cvtsi2ss	xmm5, edx
					movups	xmm7, vcNormal
				subss	xmm0, xmm4
					mulps	xmm6, rConstRcp256
				subss	xmm1, xmm5
					mulps	xmm7, rConstRcp256
				;
				push	ecx
				lea	ecx, [edx + 1]
					cmp	ecx, [edi].dwImageHeight
						mulps	xmm6, xmm7
					cmovae	ecx, edx
				imul	edx, [edi].dwBytesPerLine
					imul	ecx, [edi].dwBytesPerLine
						movhlps	xmm3, xmm6
				lea	ebx, [eax + 1]
						movss	xmm1, xmm6
						shufps	xmm6, xmm6, 1
				add	edx, [edi].ptrImageArray
						addss	xmm1, xmm3
					add	ecx, [edi].ptrImageArray
				cmp	ebx, [edi].dwImageWidth
				cmovae	ebx, eax
						addss	xmm1, xmm6
						mulps	xmm1, xmm1
						movss	xmm3, rConst1
						movss	xmm0, rConst1
				;
				movss	xmm4, REAL4 PTR [edx + eax * 4]
						subss	xmm3, xmm1	; xmm3 = cos^2
				movss	xmm5, REAL4 PTR [edx + ebx * 4]
						subss	xmm0, [esi].ShadowMap.rFixErrorGap
				movss	xmm6, REAL4 PTR [ecx + eax * 4]
						mulss	xmm3, [esi].ShadowMap.rVarErrorGap
				movss	xmm7, REAL4 PTR [ecx + ebx * 4]
				;
				maxss	xmm4, xmm5
						subss	xmm0, xmm3
				maxss	xmm6, xmm7
						mulss	xmm2, xmm0
				maxss	xmm4, xmm6
				pop	ecx
				comiss	xmm2, xmm4
				jnc	Label_VectorLight_Continue
			.ENDIF
			ASSUME	edi:NOTHING
		.ENDIF
		;
		;	拡散反射光成分計算
		; ------------------------------------------------------------
		;
		; 光線ベクトルと法線の内積を計算
		;
		movq	mm0, MMWORD PTR [esi].vLight
		movq	mm1, MMWORD PTR vNormal
		pmaddwd	mm0, mm1
			movups	xmm4, [esi].vcLight
			movups	xmm6, vcNormal
		movq	mm2, mm0
		psrlq	mm0, 32
			movaps	xmm5, xmm6
			mulps	xmm6, xmm4
		paddd	mm0, mm2		; mm0 <= 輝度 * 10000H
		;
		; 判定マスク生成
		;
		mov	edx, rFocusPlaneParam
		movd	eax, mm0
		not	edx
		mov	edi, eax
		and	edx, nMaskDoubleSided
		sar	edi, 31
		xor	edx, eax
		js	Label_VectorLight_Continue
		xor	eax, edi
		sub	eax, edi
			movhlps	xmm7, xmm6
		imul	eax, nDiffusion
			addss	xmm7, xmm6
			shufps	xmm6, xmm6, 1
		mov	edx, 4000H
		sar	eax, 8 + 2 + 4
			addss	xmm6, xmm7
		cmp	eax, edx
			mulss	xmm6, rConstRcp8000H
		cmova	eax, edx		; eax <= 輝度 * 400H [0～4000H]
		;
		; 拡散反射成分加算
		;
		movd	mm4, eax
		movq	mm5, MMWORD PTR [esi].rgbColor
				shufps	xmm6, xmm6, 0
		pshufw	mm4, mm4, 01000000B
			movq	mm2, mm5
		pmulhw	mm5, mm4
			pmullw	mm2, mm4
				mulps	xmm5, xmm6
		movq	mm4, mm2
		punpcklwd	mm2, mm5
		punpckhwd	mm4, mm5
		psrld	mm2, 10
			psrld	mm4, 10
		packssdw	mm2, mm4
		paddsw	mm6, mm2
		;
		;	鏡面反射光成分計算
		; ------------------------------------------------------------
		.IF	nSpecular > 0
			;
			; 反射光ベクトルを計算
			;	L' = L - 2 * <L|A> / (|A|^2) * A
			;	   = L - <L|A> / 8000H * A
			;
			subps	xmm4, xmm5
			;
			; cos^2 θ を計算
			;
			movups	xmm2, vcFocus
			mulps	xmm4, xmm2
			movhlps	xmm2, xmm4
			movss	xmm3, xmm4
			shufps	xmm4, xmm4, 1
			addss	xmm2, xmm3
			addss	xmm4, xmm2	; cosθ * 10000H
			;
			cvtss2si	eax, xmm4
			add	eax, nSpecularSizeBias
			.IF	SIGN?
				;
				; 鏡面反射光成分加算
				;
				imul	eax, nSpecular
				imul	nSpecularSizeScale
				shrd	eax, edx, 8 + 8 + 2
				mov	edx, 4000H
				movq	mm0, MMWORD PTR [esi].rgbColor
				cmp	eax, edx
				cmova	eax, edx
				psllw	mm0, 2
				movd	mm1, eax
				pshufw	mm1, mm1, 0
				pmulhuw	mm0, mm1
				paddsw	mm7, mm0
			.ENDIF
		.ENDIF
		;
Label_VectorLight_Continue:
		mov	ebx, hRenderPoly
		add	esi, (SIZEOF E3D_VECTOR_LIGHT_ENTRY)
		dec	ecx
	.ENDW

	;
	;	点光源の計算
	; ---------------------------------------------------------------------
	mov	ecx, [ebx].nPointLightCount
	mov	esi, [ebx].pPointLights
	ASSUME	esi:PE3D_POINT_LIGHT_ENTRY
	test	ecx, ecx
	.WHILE	!ZERO?
		;
		;	光線ベクトルの正規化
		; ------------------------------------------------------------
		mov	eax, pFocusPoints
		movlps	xmm2, QWORD PTR [esi].vLight
		movss	xmm3, [esi].vLight.z
		movups	xmm0, [eax]
		movlhps	xmm2, xmm3
		subps	xmm0, xmm2
		;
		movaps	xmm4, xmm0
		mulps	xmm0, xmm0
		movhlps	xmm1, xmm0
		movss	xmm2, xmm0
		shufps	xmm0, xmm0, 1
		addss	xmm1, xmm2
		addss	xmm0, xmm1
		rsqrtss	xmm0, xmm0
		;
		shufps	xmm0, xmm0, 0
		mulps	xmm4, xmm0
		mulss	xmm0, [esi].rBrightness
;		movups	vcLight, xmm4
;		movss	rBrightness, xmm0
		;
		;	拡散反射光成分計算
		; ------------------------------------------------------------
		;
		; 光線ベクトルと法線の内積を計算
		;
		movups	xmm1, vcNormal
		movaps	xmm5, xmm1		; xmm5 <= vcNormal
		mulps	xmm1, xmm4
		movhlps	xmm2, xmm1
		movss	xmm3, xmm1
		shufps	xmm1, xmm1, 1
		addss	xmm2, xmm3
		addss	xmm1, xmm2
		movss	xmm6, xmm1		; xmm6 <= <L|A>
		;
		mulss	xmm1, xmm0		; xmm1 <= 輝度 * 100H
		;
		cvtsi2ss	xmm2, nDiffusion
			movss	nTemp, xmm1
;			cvtss2si	ebx, xmm1
		mulss	xmm1, xmm2		; xmm1 <= 輝度 * 10000H
		;
		; 判定マスク生成
		;
		mov	edx, rFocusPlaneParam
		mov	ebx, nTemp
		not	edx
		mov	edi, ebx
		and	edx, nMaskDoubleSided
		sar	edi, 31
		xor	edx, ebx
		js	Label_PointLight_Continue
		cvtss2si	eax, xmm1
		xor	eax, edi
		sub	eax, edi
		mov	edx, 4000H
		sar	eax, 2 + 4
		cmp	eax, edx
		cmova	eax, edx		; eax <= 輝度 * 400H [0～4000H]
		;
		; 拡散反射成分加算
		;
		movd	mm0, eax
		movq	mm1, MMWORD PTR [esi].rgbColor
		pshufw	mm0, mm0, 01000000B
		psllw	mm1, 2 + 4
		pmulhw	mm0, mm1
		paddsw	mm6, mm0
		;
		;	鏡面反射光成分計算
		; ------------------------------------------------------------
		.IF	nSpecular > 0
			;
			; 反射光ベクトルを計算
			;	L' = L - 2 * <L|A> / (|A|^2) * A
			;	   = L - <L|A> / 8000H * A
			;
			mulss	xmm6, rConstRcp128
			mulps	xmm4, xmmPacked256
			shufps	xmm6, xmm6, 0
			mulps	xmm5, xmm6
			subps	xmm4, xmm5
			;
			; cos^2 θ を計算
			;
			movups	xmm2, vcFocus
			mulps	xmm4, xmm2
			movhlps	xmm2, xmm4
			movss	xmm3, xmm4
			shufps	xmm4, xmm4, 1
			addss	xmm2, xmm3
			addss	xmm4, xmm2	; cosθ * 10000H
			;
			cvtss2si	eax, xmm4
			add	eax, nSpecularSizeBias
			.IF	SIGN?
				;
				; 鏡面反射光成分加算
				;
				imul	eax, nSpecular
				imul	nSpecularSizeScale
				shrd	eax, edx, 8 + 8 + 2
				mov	edx, 4000H
				movq	mm0, MMWORD PTR [esi].rgbColor
				cmp	eax, edx
				cmova	eax, edx
				psllw	mm0, 2
				movd	mm1, eax
				pshufw	mm1, mm1, 0
				pmulhuw	mm0, mm1
				paddsw	mm7, mm0
			.ENDIF
		.ENDIF
		;
Label_PointLight_Continue:
		mov	ebx, hRenderPoly
		add	esi, (SIZEOF E3D_POINT_LIGHT_ENTRY)
		dec	ecx
	.ENDW

	.IF	!(dwShadingFlags & (E3DSAF_ENVIRONMENT_MAP OR E3DSAF_GENVIRONMENT_MAP))
		;
		;	拡散反射光・鏡面反射光適用
		; ------------------------------------------------------------
Label_NormalShading:
		mov	esi, pColorsLooks
		ASSUME	esi:PTR E3D_COLOR
		;
		; 発光成分(ADD) = 発光成分 * 拡散反射 + 鏡面反射
		; 透明度成分(MUL) = 透明度成分 * 拡散反射
		;
				movq	mm5, mm6
				pxor	mm3, mm3
		movd	mm0, [esi].rgbAdd.dwPixelCode
			movd	mm1, [esi].rgbMul.dwPixelCode
				psubsw	mm5, mmxConst256
		pxor	mm2, mm2
				pcmpgtw	mm3, mm5
		movq	mm4, MMWORD PTR rgbLuminance.rgbMul
				pandn	mm3, mm5
		movq	mm5, MMWORD PTR rgbLuminance.rgbAdd
		punpcklbw	mm0, mm2
			punpcklbw	mm1, mm2
		psubsw	mm0, mm5
			psubsw	mm1, mm4

	.ELSE
		;
		;	拡散反射光・鏡面反射光適用（環境マッピング有り）
		; ------------------------------------------------------------
		;
		; マッピング画像取得
		;
		PUSHCONTEXT	ASSUMES
		mov	esi, pSurfaceAttribute
		.IF	dwShadingFlags & E3DSAF_GENVIRONMENT_MAP
			mov	esi, [ebx].genvmap.pUpperImage
		.ELSE
			ASSUME	esi:PCE3D_SURFACE_ATTRIBUTE
			mov	esi, [esi].envmap.pUpperImage
		.ENDIF
		test	esi, esi
		jz	Label_NormalShading
		ASSUME	esi:PEGL_IMAGE_INFO
		;
		; 視線反射ベクトル計算
		;	L' = vcFocus + 2 * <vcFocus|vcNormal> / (|vcNormal|^2) * vcNormal
		;	   = vcFocus + <vcFocus|vcNormal> / 8000H * vcNormal
		;
		movups	xmm0, vcFocus
		movups	xmm1, vcNormal
		movaps	xmm2, xmm0
				pcmpeqd	mm5, mm5
		mulps	xmm0, xmm1
				movq	mm4, QWORD PTR [esi].dwImageWidth
		movss	xmm4, xmm0
				paddd	mm4, mm5
		movhlps	xmm5, xmm0
				cvtpi2ps	xmm6, mm4
				packssdw	mm4, mm4
		shufps	xmm0, xmm0, 1
				psubusw	mm5, mm4
				mov	eax, 4
		addss	xmm4, xmm5
				movd	mm3, eax
				movd	mm2, [esi].dwBytesPerLine
		addss	xmm0, xmm4
				punpcklwd	mm3, mm2
		mulss	xmm0, rConstRcp8000H
		shufps	xmm0, xmm0, 0
		mulps	xmm0, xmm1
				mulps	xmm6, rConstRcp2
		addps	xmm0, xmm2
		mulps	xmm0, rConstRcp256
		;
		; ベクトル回転
		;
		movaps	xmm1, xmm0
		movaps	xmm2, xmm0
		mulps	xmm0, [ebx].envmat.matrix[0]
		mulps	xmm1, [ebx].envmat.matrix[10H]
		mulps	xmm2, [ebx].envmat.matrix[20H]
		addps	xmm0, xmm1
		addps	xmm0, xmm2
		;
		; テクスチャ取得
		;
		mulps	xmm0, xmm6
		addps	xmm0, xmm6
		;
		cvtps2pi	mm4, xmm0
		packssdw	mm4, mm4
		paddusw		mm4, mm5
		psubusw		mm4, mm5
		pmaddwd		mm4, mm3
		;
		mov	ecx, [esi].ptrImageArray
		movd	eax, mm4
		;
		mov	esi, pColorsLooks
		POPCONTEXT	ASSUMES
		;
		pxor	mm3, mm3
		pcmpeqw	mm4, mm4
		movd	mm0, DWORD PTR [ecx + eax]
		movd	mm1, [esi].rgbMul.dwPixelCode
		movd	mm2, [esi].rgbAdd.dwPixelCode
		punpcklbw	mm0, mm3
		punpcklbw	mm1, mm3
		punpcklbw	mm2, mm3
		psubw		mm1, mm4
		pmullw	mm0, mm1
			movq	mm5, mm6
		psrlw	mm0, 8
			psubsw	mm5, mmxConst256
		paddsw	mm0, mm2
			pcmpgtw	mm3, mm5
		pxor	mm1, mm1
			movq	mm4, MMWORD PTR rgbLuminance.rgbMul
		packsswb	mm0, mm0
			pandn	mm3, mm5
			movq	mm5, MMWORD PTR rgbLuminance.rgbAdd
		punpcklbw	mm0, mm3
			psubsw	mm1, mm4
			psubsw	mm0, mm5
	.ENDIF

	;
	;	擬似フォッグ適用
	; --------------------------------------------------------------------
	movss	xmm1, vFocusPos.z
		psllw	mm0, 7
		paddsw	mm6, mm6
		psllw	mm1, 7
	movss	xmm0, [ebx].rFogDeepness
		pmulhw	mm0, mm6
	subss	xmm1, [ebx].rFogBiasZ
		pmulhw	mm6, mm1
			paddsw	mm7, mm3
	mulss	xmm0, xmm1
		paddsw	mm7, mm0
		paddsw	mm6, mm4
		paddsw	mm7, mm5
	cvtss2si	eax, xmm0
	.IF	(SDWORD PTR eax) > 0
		.IF	(SDWORD PTR eax) < 100H
			;
			; 発光成分(ADD) = (フォッグ色 - 発光成分) * Z適用度 + 発光成分
			; 透明度成分(MUL) = 透明度成分 * (1 - Z適用度)
			;
			shl	eax, 7
			movq	mm0, MMWORD PTR [ebx].rgbFogColor
			movd	mm2, eax
				neg	eax
			pshufw	mm2, mm2, 0
				add	eax, 8000H
			psubsw	mm0, mm7
				movd	mm3, eax
			paddsw	mm0, mm0
				paddsw	mm6, mm6
			pmulhw	mm0, mm2
				pshufw	mm3, mm3, 0
				pmulhw	mm6, mm3
			paddsw	mm7, mm0
		.ELSE
			movq	mm7, MMWORD PTR [ebx].rgbFogColor
			pxor	mm6, mm6
		.ENDIF
	.ENDIF

	;
	;	透明度および透明深度適用
	; --------------------------------------------------------------------
	mov	eax, nDeepness
	mov	ecx, nTransparency
	call	pfnTransparent

	;
	;	色情報出力
	; --------------------------------------------------------------------
;		movq	mm5, mm6
;		pxor	mm4, mm4
;		psubsw	mm5, mmxConst256
	mov	eax, pNormals
;		pcmpgtw	mm4, mm5
	mov	edx, pFocusPoints
;		pandn	mm4, mm5
	mov	ecx, nCount
;		paddsw	mm7, mm4
	;
	add	eax, (SIZEOF E3D_VECTOR4)
		packuswb	mm6, mm7
	add	edx, (SIZEOF E3D_VECTOR4)
		movq	MMWORD PTR [esi], mm6
	add	esi, (SIZEOF E3D_COLOR)
	mov	pNormals, eax
	mov	pFocusPoints, edx
	mov	pColorsLooks, esi

	ASSUME	esi:NOTHING
	dec	ecx

	.ENDW
	;
Label_Exit:
;	emms
	xor	eax, eax
	ret

ALIGN	10H
SubFunc_DeepnessNoTexture:
	;
	;	透明深度適用
	; --------------------------------------------------------------------
	movss	xmm0, rFocusParam
	cvtsi2ss	xmm1, eax
	mulss	xmm0, xmm0
	mulss	xmm0, xmm1
	cvtss2si	eax, xmm0
	add	ecx, eax
	.IF	(SDWORD PTR ecx) > 0

SubFunc_TransparentNoTexture:
	;
	;	透明度適用
	; --------------------------------------------------------------------
	xor	eax, eax
	sub	ecx, 100H
	movq	mm1, mmxConst256
	neg	ecx
	psubsw	mm6, mm1
	cmovs	ecx, eax
	;
	paddsw	mm6, mm6
	shl	ecx, 7
	paddsw	mm7, mm7
	movd	mm0, ecx
	pshufw	mm0, mm0, 11000000B
	;
	pmulhw	mm6, mm0
	pmulhw	mm7, mm0
	paddsw	mm6, mm1

	.ENDIF
SubFunc_DeepnessDummy:
	BYTE	0C3H	; ret

ALIGN	10H
SubFunc_DeepnessTexture:
	;
	;	透明深度適用
	; --------------------------------------------------------------------
	movss	xmm0, rFocusParam
	cvtsi2ss	xmm1, eax
	mulss	xmm0, xmm0
	mulss	xmm0, xmm1
	cvtss2si	eax, xmm0
	add	ecx, eax
	.IF	(SDWORD PTR ecx) > 0

SubFunc_TransparentTexture:
	;
	;	透明度適用
	; --------------------------------------------------------------------
	sub	ecx, 100H
	xor	eax, eax
	neg	ecx
	cmovs	ecx, eax
	;
	paddsw	mm6, mm6
	shl	ecx, 7
	paddsw	mm7, mm7
	movd	mm0, ecx
	pshufw	mm0, mm0, 11000000B
	;
	pmulhw	mm6, mm0
	pmulhw	mm7, mm0

	.ENDIF

	BYTE	0C3H	; ret


eglRenderPoly@ShadeVectorsSSE	ENDP


;
;	透視変換 486 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@ProjectScreen486	PROC	NEAR32 C USES ebx esi edi,
	hRenderPoly:HEGL_RENDER_POLYGON,
	pDst:PE3D_VECTOR_2D, pSrc:PCE3D_VECTOR4, nVertexCount:DWORD

	mov	ecx, nVertexCount
	mov	ebx, hRenderPoly
	mov	esi, pSrc
	mov	edi, pDst
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	ASSUME	esi:PCE3D_VECTOR4
	ASSUME	edi:PE3D_VECTOR_2D

	test	ecx, ecx
	.WHILE	!ZERO?
		fld	[esi].x
		fld	[esi].y
		;
		fld	[ebx].vScreenPos.z
		fld	[esi].z
		fdivp	st(1), st
		add	esi, (SIZEOF E3D_VECTOR4)
		;
		fmul	st(1), st
		fmulp	st(2), st
		;
		fld	[ebx].vScreenPos.x
		fld	[ebx].vScreenPos.y
		faddp	st(2), st
		faddp	st(2), st
		;
		fxch	st(1)
		fstp	[edi].x
		fstp	[edi].y
		add	edi, (SIZEOF E3D_VECTOR_2D)
		;
		dec	ecx
	.ENDW

	ASSUME	ebx:NOTHING
	ASSUME	esi:NOTHING
	ASSUME	edi:NOTHING
	xor	eax, eax
	ret

eglRenderPoly@ProjectScreen486	ENDP

;
;	透視変換 SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@ProjectScreenSSE	PROC	NEAR32 C USES ebx esi edi,
	hRenderPoly:HEGL_RENDER_POLYGON,
	pDst:PE3D_VECTOR_2D, pSrc:PCE3D_VECTOR4, nVertexCount:DWORD

	mov	ecx, nVertexCount
	mov	ebx, hRenderPoly
	mov	esi, pSrc
	mov	edi, pDst
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	ASSUME	esi:PCE3D_VECTOR4
	ASSUME	edi:PE3D_VECTOR_2D

	movlps	xmm6, QWORD PTR [ebx].vScreenPos.x
	movss	xmm7, [ebx].vScreenPos.z
	movlhps	xmm6, xmm6
	shufps	xmm7, xmm7, 0

	sub	ecx, 2
	.WHILE	!SIGN?
		movss	xmm4, [esi].z
		movlps	xmm0, QWORD PTR [esi].x
		movss	xmm5, [esi][10H].z
		movhps	xmm0, QWORD PTR [esi][10H].x
		shufps	xmm4, xmm5, 0
		mulps	xmm0, xmm7
		divps	xmm0, xmm4
		addps	xmm0, xmm6
		add	esi, 20H
		movups	XMMWORD_PTR [edi], xmm0
		add	edi, 10H
		sub	ecx, 2
	.ENDW

	add	ecx, 2
	.IF	!ZERO?
		xorps	xmm0, xmm0
		movss	xmm4, [esi].z
		movlps	xmm0, QWORD PTR [esi].x
		shufps	xmm4, xmm4, 0
		mulps	xmm0, xmm7
		divps	xmm0, xmm4
		addps	xmm0, xmm6
		movlps	[edi], xmm0
	.ENDIF

	ASSUME	ebx:NOTHING
	ASSUME	esi:NOTHING
	ASSUME	edi:NOTHING
	xor	eax, eax
	ret

eglRenderPoly@ProjectScreenSSE	ENDP

;
;	ポリゴンエントリをソート 486 互換コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@SortPolygonEntry486	PROC	NEAR32 C USES ebx esi edi,
	hRenderPoly:HEGL_RENDER_POLYGON,
	ppPolygons:PTR PE3D_POLYGON_ENTRY,
	nPolygonCount:DWORD, dwSortingFlags:DWORD

	LOCAL	iFirst:SDWORD, iEnd:SDWORD
	LOCAL	nTransCount:DWORD
	LOCAL	pPolygon:PE3D_POLYGON_ENTRY
	LOCAL	nZValue:DWORD
	LOCAL	pMeshVertics:PE3D_VECTOR4
	LOCAL	nMeshPolyCount:DWORD
	LOCAL	pMeshPolyList:PTR PVOID
	LOCAL	pMeshPolyZList:PTR REAL4
	LOCAL	pMeshDataCopy:PTR E3D_PRIMITIVE_MESH_POLY

	;
	;	ソートテーブルをセットアップ
	; --------------------------------------------------------------------
	;
	; メモリを確保
	;
	mov	ebx, hRenderPoly
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	mov	eax, nPolygonCount
	.IF	eax > [ebx].nSortTableLength
		.IF	[ebx].pSortTable != NULL
			INVOKE	eslHeapFree , [ebx].dib.hHeap, [ebx].pSortTable, 0
		.ENDIF
		mov	eax, nPolygonCount
		shl	eax, 2
		INVOKE	eslHeapAllocate , [ebx].dib.hHeap, eax, 0
		mov	edx, nPolygonCount
		mov	[ebx].pSortTable, eax
		mov	[ebx].nSortTableLength, edx
	.ENDIF
	;
	; 半透明ポリゴンと不透明ポリゴンを振り分ける
	;
	mov	edx, nPolygonCount
	xor	eax, eax
	.IF	edx <= 1
		xor	eax, eax
	.ENDIF
	dec	edx
	mov	iFirst, eax
	mov	iEnd, edx
	;
	mov	esi, [ebx].pSortTable
	mov	edi, ppPolygons
	mov	ecx, iFirst
	ASSUME	ebx:NOTHING
	.WHILE	1
		;
		; 透明ポリゴンを検索する
		;
		.WHILE	(SDWORD PTR ecx) <= iEnd
			mov	ebx, [edi + ecx * 4]
			test	ebx, ebx
			.IF	!ZERO?
				call	SubFunc_TestTransparency
				.BREAK	.IF	!ZERO?
				mov	DWORD PTR [esi + ecx * 4], edx
			.ENDIF
			inc	ecx
		.ENDW
		mov	iFirst, ecx
		.BREAK	.IF	(SDWORD PTR ecx) > iEnd
		;
		; 不透明ポリゴンを検索する
		;
		mov	ecx, iEnd
		mov	pPolygon, ebx
		mov	nZValue, edx
		.WHILE	(SDWORD PTR ecx) >= iFirst
			mov	ebx, [edi + ecx * 4]
			test	ebx, ebx
			.IF	!ZERO?
				call	SubFunc_TestTransparency
				.BREAK	.IF	ZERO?
				mov	DWORD PTR [esi + ecx * 4], edx
			.ENDIF
			dec	ecx
		.ENDW
		mov	iEnd, ecx
		.BREAK	.IF	(SDWORD PTR ecx) < iFirst
		;
		mov	eax, pPolygon
		mov	pPolygon, ebx
		mov	ebx, eax
		mov	eax, nZValue
		mov	nZValue, edx
		mov	edx, eax
		mov	[edi + ecx * 4], ebx
		mov	[esi + ecx * 4], edx
		dec	ecx
		mov	iEnd, ecx
		mov	ecx, iFirst
		mov	ebx, pPolygon
		mov	edx, nZValue
		mov	[edi + ecx * 4], ebx
		mov	[esi + ecx * 4], edx
		inc	ecx
	.ENDW
	;
	;	不透明ポリゴンをソートする
	; --------------------------------------------------------------------
	.IF	(dwSortingFlags & E3D_SORT_OPAQUE) && (iFirst > 0)
		;
		; ヌル要素を削除する
		;
		xor	ecx, ecx
		xor	edx, edx
		.REPEAT
			mov	ebx, [edi + ecx * 4]
			mov	eax, [esi + ecx * 4]
			inc	ecx
			test	ebx, ebx
			.IF	!ZERO?
				mov	[edi + edx * 4], ebx
				mov	[esi + edx * 4], eax
				inc	edx
				;
				ASSUME	ebx:PTR E3D_POLYGON_ENTRY
				.IF	[ebx].dwTypeFlag & E3D_MESH_POLYGON
					push	ecx
					push	edx
					push	esi
					push	edi
					mov	esi, ebx
					call	SubFunc_SortMesh
					pop	edi
					pop	esi
					pop	edx
					pop	ecx
				.ENDIF
				ASSUME	ebx:NOTHING
			.ENDIF
		.UNTIL	ecx >= iFirst
		;
		; ソートする
		;
		mov	ecx, edx
		call	SubFunc_SortPolygonEntry
	.ENDIF
	;
	;	透明ポリゴンをソートする
	; --------------------------------------------------------------------
	mov	eax, nPolygonCount
	sub	eax, iFirst
	.IF	(dwSortingFlags & E3D_SORT_TRANSPARENT) && (eax >= 1)
		;
		; ヌル要素を削除する
		; メッシュポリゴンのソートを行う
		;
		mov	nTransCount, eax
		mov	eax, iFirst
		xor	ecx, ecx
		xor	edx, edx
		lea	edi, [edi + eax * 4]
		lea	esi, [esi + eax * 4]
		.REPEAT
			mov	ebx, [edi + ecx * 4]
			mov	eax, [esi + ecx * 4]
			inc	ecx
			test	ebx, ebx
			.IF	!ZERO?
				mov	[edi + edx * 4], ebx
				mov	[esi + edx * 4], eax
				inc	edx
				;
				ASSUME	ebx:PTR E3D_POLYGON_ENTRY
				.IF	[ebx].dwTypeFlag & E3D_MESH_POLYGON
					push	ecx
					push	edx
					push	esi
					push	edi
					mov	esi, ebx
					call	SubFunc_SortMesh
					pop	edi
					pop	esi
					pop	edx
					pop	ecx
				.ENDIF
				ASSUME	ebx:NOTHING
			.ENDIF
		.UNTIL	ecx >= nTransCount
		;
		; ソートする
		;
		mov	ecx, edx
		call	SubFunc_SortPolygonEntry
	.ENDIF
	;
	xor	eax, eax
	ret

ALIGN	10H
SubFunc_TestTransparency:
	;
	;	ｚ値と透明ポリゴンの判定
	; --------------------------------------------------------------------
	ASSUME	ebx:PE3D_POLYGON_ENTRY
	mov	edx, [ebx].vCenter.z
	mov	eax, [ebx].dwTypeFlag
	test	edx, edx
	.IF	SIGN?
		neg	edx
		or	edx, 80000000H
	.ENDIF
	.IF	eax == E3D_IMAGE_PRIMITIVE
		mov	eax, [ebx].surface.image.pInfo
		.IF	eax != NULL
			ASSUME	eax:PTR EGL_IMAGE_INFO
			.IF	[eax].fdwFormatType & EIF_WITH_ALPHA
				mov	eax, -1
			.ELSE
				mov	eax, [ebx].dwTransparency
			.ENDIF
			ASSUME	eax:NOTHING
		.ELSE
			mov	eax, [ebx].dwTransparency
		.ENDIF
	.ELSEIF	eax == E3D_INFINITE_PLANE
		mov	eax, [ebx].surface.poly.txmap.pTextureImage
		.IF	eax != NULL
			ASSUME	eax:PTR EGL_IMAGE_INFO
			.IF	[eax].fdwFormatType & EIF_WITH_ALPHA
				mov	eax, -1
			.ELSE
				mov	eax, [ebx].dwTransparency
			.ENDIF
			ASSUME	eax:NOTHING
		.ELSE
			mov	eax, [ebx].dwTransparency
		.ENDIF
		mov	edx, 7F000000H
	.ELSE
		push	esi
		push	ecx
		.IF	eax & E3D_MESH_POLYGON
			mov	ecx, [ebx].pAttr
			.IF	ecx != NULL
				mov	ecx, (E3D_SURFACE_ATTRIBUTE PTR [ecx]).txmap.pTextureImage
			.ENDIF
		.ELSE
			mov	ecx, [ebx].surface.poly.txmap.pTextureImage
		.ENDIF
		mov	eax, [ebx].dwShadingFlags
		and	eax, (E3DSAF_PHONG_SHADE OR E3DSAF_RAY_REFRACTING)
		cmp	eax, (E3DSAF_PHONG_SHADE OR E3DSAF_RAY_REFRACTING)
		mov	eax, 0
		jz	Label_ExitPolygonTesting
		mov	eax, [ebx].dwTransparency
		mov	esi, [ebx].pVertexColors
		ASSUME	ecx:PTR EGL_IMAGE_INFO
		.IF	(ecx != NULL) && ([ebx].dwTypeFlag & E3D_TEXTURE_POLYGON)
			.IF	[ecx].fdwFormatType & EIF_WITH_ALPHA
				.IF	!([ebx].dwShadingFlags & E3DSAF_TEXTURE_TRIM)
					mov	eax, -1
				.ENDIF
			.ENDIF
		.ELSE
			.IF	[ebx].dwShadingFlags & E3DSAF_PHONG_SHADE
				mov	ecx, [ebx].pAttr
				mov	eax, -1
				ASSUME	ecx:PE3D_SURFACE_ATTRIBUTE
				.IF	ecx != NULL
					cmp	[ecx].nTransparency, 0
					jnz	Label_ExitPolygonTesting
					cmp	[ecx].nDeepness, 0
					jnz	Label_ExitPolygonTesting
				.ENDIF
				xor	eax, eax
				ASSUME	ecx:NOTHING
			.ENDIF
			mov	ecx, [ebx].dwProjectedCount
			ASSUME	esi:PE3D_COLOR
			.REPEAT
				or	eax, [esi].rgbMul.dwPixelCode
				and	eax, 00FFFFFFH
				.BREAK	.IF	!ZERO?
				add	esi, (SIZEOF E3D_COLOR)
				dec	ecx
			.UNTIL	ZERO?
		.ENDIF
		ASSUME	esi:NOTHING
Label_ExitPolygonTesting:
		pop	ecx
		pop	esi
	.ENDIF
	ASSUME	ebx:NOTHING
	test	eax, eax
	BYTE	0C3H	; ret

ALIGN	10H
SubFunc_SortMesh:
	;
	;	メッシュ内ポリゴンのソート
	; --------------------------------------------------------------------
	mov	ebx, hRenderPoly
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	ASSUME	esi:PTR E3D_POLYGON_ENTRY
	;
	; ソート用バッファを確保
	;
	mov	eax, [esi].pVertexes
	mov	pMeshVertics, eax
	mov	esi, [esi].surface.mesh.pMesh
	ASSUME	esi:PTR E3D_PRIMITIVE_MESH_LIST
	mov	eax, [esi].dwPolyCount
	shl	eax, 3		; *= (SIZEOF DWORD) * 2
	add	eax, [esi].dwMeshBytes
	;
	.IF	[ebx].nSortMeshBufBytes < eax
		mov	edi, eax
		.IF	[ebx].pSortMeshBuffer != NULL
			INVOKE	eslHeapFree ,
				[ebx].dib.hHeap, [ebx].pSortMeshBuffer, 0
		.ENDIF
		;
		INVOKE	eslHeapAllocate , [ebx].dib.hHeap, edi, 0
		mov	[ebx].pSortMeshBuffer, eax
		mov	[ebx].nSortMeshBufBytes, edi
	.ENDIF
	;
	mov	eax, [ebx].pSortMeshBuffer
	mov	ecx, [esi].dwPolyCount
	mov	pMeshPolyList, eax
	mov	nMeshPolyCount, ecx
	lea	eax, [eax + ecx * (SIZEOF PVOID)]
	mov	pMeshPolyZList, eax
	lea	eax, [eax + ecx * (SIZEOF REAL4)]
	mov	pMeshDataCopy, eax
	;
	; データをセットアップ
	;
	lea	esi, [esi].mpEntries[0]
	push	esi
	mov	edi, pMeshDataCopy
	.IF	ERI_EnabledProcessorType & ERI_USE_XMM_P3
		xor	ecx, ecx
		.WHILE	ecx < nMeshPolyCount
			mov	eax, pMeshPolyList
			push	ecx
			mov	DWORD PTR [eax + ecx * 4], edi
			;
			ASSUME	esi:PTR E3D_PRIMITIVE_MESH_POLY
			ASSUME	edi:PTR E3D_PRIMITIVE_MESH_POLY
			mov	ecx, [esi].dwVertexCount
			xorps	xmm0, xmm0
			mov	[edi].dwVertexCount, ecx
			cvtsi2ss	xmm1, ecx
			test	ecx, ecx
			lea	esi, [esi].dwIndex[0]
			lea	edi, [edi].dwIndex[0]
			ASSUME	esi:PTR DWORD
			ASSUME	edi:PTR DWORD
			.WHILE	!ZERO?
				mov	eax, [esi]
				add	esi, (SIZEOF DWORD)
				mov	[edi], eax
				add	edi, (SIZEOF DWORD)
				shl	eax, 4
				add	eax, pMeshVertics
				addss	xmm0, (E3D_VECTOR4 PTR [eax]).z
				dec	ecx
			.ENDW
			;
			divss	xmm0, xmm1
			mov	eax, pMeshPolyZList
			pop	ecx
			movss	REAL4 PTR [eax + ecx * 4], xmm0
			inc	ecx
		.ENDW
	.ELSE
		xor	ecx, ecx
		.WHILE	ecx < nMeshPolyCount
			mov	eax, pMeshPolyList
			push	ecx
			mov	DWORD PTR [eax + ecx * 4], edi
			;
			ASSUME	esi:PTR E3D_PRIMITIVE_MESH_POLY
			ASSUME	edi:PTR E3D_PRIMITIVE_MESH_POLY
			mov	ecx, [esi].dwVertexCount
			fldz
			fild	[esi].dwVertexCount
			mov	[edi].dwVertexCount, ecx
			test	ecx, ecx
			lea	esi, [esi].dwIndex[0]
			lea	edi, [edi].dwIndex[0]
			ASSUME	esi:PTR DWORD
			ASSUME	edi:PTR DWORD
			.WHILE	!ZERO?
				mov	eax, [esi]
				add	esi, (SIZEOF DWORD)
				mov	[edi], eax
				add	edi, (SIZEOF DWORD)
				shl	eax, 4
				add	eax, pMeshVertics
				fld	(E3D_VECTOR4 PTR [eax]).z
				faddp	st(2), st
				dec	ecx
			.ENDW
			;
			fdivp	st(1), st
			mov	eax, pMeshPolyZList
			pop	ecx
			fstp	REAL4 PTR [eax + ecx * 4]
			inc	ecx
		.ENDW
	.ENDIF
	ASSUME	esi:NOTHING
	ASSUME	edi:NOTHING
	;
	; ｚ座標を整数比較できるように変換
	;
	mov	ecx, nMeshPolyCount
	mov	esi, pMeshPolyZList
	test	ecx, ecx
	.WHILE	!ZERO?
		mov	eax, DWORD PTR [esi]
		mov	edx, eax
		sar	eax, 31
		shr	eax, 1
		xor	eax, edx
		mov	DWORD PTR [esi], eax
		add	esi, (SIZEOF DWORD)
		dec	ecx
	.ENDW
	;
	; ソートを実行
	;
	mov	esi, pMeshPolyZList
	mov	edi, pMeshPolyList
	mov	ecx, nMeshPolyCount
	call	SubFunc_SortPolygonEntry
	;
	; メッシュを再構築
	;
	pop	edi
	pushfd
	cld
	mov	edx, pMeshPolyList
	mov	eax, nMeshPolyCount
	test	eax, eax
	.WHILE	!ZERO?
		mov	esi, DWORD PTR [edx]
		add	edx, (SIZEOF DWORD)
		ASSUME	esi:PTR E3D_PRIMITIVE_MESH_POLY
		mov	ecx, [esi].dwVertexCount
		ASSUME	esi:NOTHING
		inc	ecx
		rep	movsd
		dec	eax	
	.ENDW
	;
	popfd
	ASSUME	esi:NOTHING
	BYTE	0C3H	; ret

ALIGN	10H
SubFunc_SortPolygonEntry:
	;
	;	ｚ値でクイックソート
	; --------------------------------------------------------------------
	test	ecx, ecx
	jz	Label_ExitSortPolygonEntry
	ASSERT	<!!SIGN?>, "ecx >= 0"
	js	Label_ExitSortPolygonEntry
	push	ecx				; ESP[0] = 要素の総数
	mov	eax, [esi + ecx * 4 - 4]	; 基準値を取り出す
	mov	ebx, [edi + ecx * 4 - 4]
	dec	ecx
	xor	edx, edx
	mov	nZValue, eax
	mov	pPolygon, ebx
	;
	; 2つの区間に分離
	;
Label_FindLargeValue:
		mov	eax, [esi + edx * 4]
		mov	ebx, [edi + edx * 4]
		cmp	eax, nZValue
		jl	Label_BreakLargeValue
		inc	edx
		cmp	edx, ecx
	jl	Label_FindLargeValue
	jmp	Label_NextSort
	;
Label_BreakLargeValue:
	mov	[esi + ecx * 4], eax
	mov	[edi + ecx * 4], ebx
	dec	ecx
	cmp	ecx, edx
	jbe	Label_NextSort
Label_FindSmallValue:
		mov	eax, [esi + ecx * 4]
		mov	ebx, [edi + ecx * 4]
		cmp	eax, nZValue
		jg	Label_BreakSmallValue
		dec	ecx
		cmp	ecx, edx
	ja	Label_FindSmallValue
	jmp	Label_NextSort
	;
Label_BreakSmallValue:
	mov	[esi + edx * 4], eax
	mov	[edi + edx * 4], ebx
	inc	edx
	cmp	edx, ecx
	jb	Label_FindLargeValue
	;
Label_NextSort:
	mov	eax, nZValue
	mov	ebx, pPolygon
	mov	[esi + ecx * 4], eax
	mov	[edi + ecx * 4], ebx
	;
	; 前半の区間をさらにソート
	;
	.IF	ecx >= 2
		push	ecx
		call	SubFunc_SortPolygonEntry
		pop	ecx
	.ENDIF
	;
	; 後半の区間を更にソート
	;
	pop	edx
	inc	ecx
	sub	edx, ecx
	.IF	edx >= 2
		push	esi
		push	edi
		lea	esi, [esi + ecx * 4]
		lea	edi, [edi + ecx * 4]
		mov	ecx, edx
		call	SubFunc_SortPolygonEntry
		pop	edi
		pop	esi
	.ENDIF
	;
Label_ExitSortPolygonEntry:
	BYTE	0C3H	; ret

eglRenderPoly@SortPolygonEntry486	ENDP


CodeSeg	ENDS

	END
