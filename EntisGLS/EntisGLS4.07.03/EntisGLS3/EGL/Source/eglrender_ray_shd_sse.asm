
; ----------------------------------------------------------------------------
;                          Entis Graphic Library
; ----------------------------------------------------------------------------
;    Copyright (c) 2008-2011 Leshade Entis, Entis-soft. Al rights reserved.
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
xmmSignBit		DWORD	4 DUP( 80000000H )
rConstRcp128		REAL4	4 DUP( 0.0078125 )	; = 1.0 / 128.0
xmmPacked256		REAL4	4 DUP( 256.0 )
xmmPackedDiv256		REAL4	4 DUP( 0.00390625 )
xmmPacked4000H		REAL4	4 DUP( 16384.0 )
rConstRcp8000H		REAL4	4 DUP( 0.000030517578125 )	; = 1.0 / 8000H

ALIGN	10H
mmxConstMaskZeroHW	LABEL	MMWORD
			WORD	0FFFFH, 0FFFFH, 0FFFFH, 0
mmxConstMaskLW		LABEL	MMWORD
			WORD	0FFFFH, 0, 0, 0
mmxConst7FFF_1000	LABEL	MMWORD
			WORD	4 DUP( 7FFFH - 1000H )
mmxConst7FFF_100	LABEL	MMWORD
			WORD	4 DUP( 7FFFH - 100H )
mmxConst1		LABEL	MMWORD
			WORD	4 DUP( 1 )
mmxConst256		LABEL	MMWORD
			WORD	4 DUP( 100H )
mmxConst4000H		LABEL	MMWORD
			WORD	4 DUP( 4000H )

ConstSeg	ENDS


; ----------------------------------------------------------------------------
;	コードセグメント
; ----------------------------------------------------------------------------

CodeSeg	SEGMENT	PARA READONLY FLAT 'CODE'

;
;	レイトレーシングシェーディング SSE 専用コード
; ----------------------------------------------------------------------------
ALIGN	10H
eglRenderPoly@ShadeVectorsAndRayTraceSSE	PROC	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON,
	pSurfaceAttribute:PCE3D_SURFACE_ATTRIBUTE,
	pNormals:PCE3D_VECTOR4, pFocusPoints:PCE3D_VECTOR4,
	pColorsLooks:PTR E3D_COLOR, nCount:DWORD,
	pTargetPlane:PE3D_VECTOR4

	LOCAL	rmvp:EGL_RENDER_MESH_VIEW_POINT

	mov	ecx, hRenderPoly
	ASSUME	ecx:PTR EGL_RENDER_POLYGON_BUF
	mov	eax, [ecx].pMeshPolygonEntry
	mov	edx, [ecx].nMeshPolygonIndex
	mov	rmvp.pExceptingMesh, eax
	mov	rmvp.nExceptingMeshIndex, edx
	mov	edx, pTargetPlane
	movups	rmvp.vViewPosition, xmm0
	xorps	xmm0, xmm0
	.IF	edx != NULL
		movups	xmm0, [edx]
	.ENDIF
	movups	rmvp.vTargetPlane, xmm0

	INVOKE	eglRenderPoly@ShadeVectorsAndRaySSE,
			hRenderPoly, pSurfaceAttribute,
			pNormals, pFocusPoints, pColorsLooks, NULL, nCount,
			ADDR rmvp, [ecx].rrtpRayParam.dwRayReflectCount

	ASSUME	ecx:NOTHING
	ret

eglRenderPoly@ShadeVectorsAndRayTraceSSE	ENDP

ALIGN	10H
eglRenderPoly@ShadeVectorsAndRaySSE	PROC	NEAR32 C USES ebx esi edi,
	hRenderPoly:HEGL_RENDER_POLYGON,
	pSurfaceAttribute:PCE3D_SURFACE_ATTRIBUTE,
	pNormals:PCE3D_VECTOR4, pFocusPoints:PCE3D_VECTOR4,
	pColorsLooks:PTR E3D_COLOR,
	prgbaTextureImage:PTR EGL_PALETTE, nCount:DWORD,
	pViewPoint:PTR EGL_RENDER_MESH_VIEW_POINT, dwReflectCount:DWORD,

	LOCAL	dwShadingFlags:DWORD		; シェーディングフラグ
	LOCAL	nMaskDoubleSided:DWORD		; 両面ポリゴン用マスク
	LOCAL	nDiffusion:SDWORD		; 拡散反射強度
	LOCAL	nSpecular:SDWORD		; 鏡面反射強度
	LOCAL	nSpecularSizeScale:SDWORD	; 鏡面反射サイズ係数 (x10000H)
	LOCAL	nSpecularSizeBias:SDWORD
	LOCAL	rgbDiffusion:E3D_PALLET_PW	; 拡散反射強度（パック表現）
	LOCAL	rgbSpecular:E3D_PALLET_PW	; 鏡面反射強度（パック表現）
	LOCAL	rgbAmbient:E3D_PALLET_PW	; 環境光
	LOCAL	rgbLuminance:E3D_COLOR_PW	; 発光色
	LOCAL	nDeepness:SDWORD		; 透明深度
	LOCAL	nTransparency:SDWORD		; 透明度
	LOCAL	nReflection:DWORD		; 反射率
	LOCAL	nRefraction:REAL4		; 屈折率
	LOCAL	pfnTransparent:PTR NEAR32	; 透明度と透明深度適用関数
	LOCAL	pfnBeforeTexture:PTR NEAR32	; テクスチャ適用関数
	LOCAL	pfnAfterTexture:PTR NEAR32
	LOCAL	pfnShadowing:PTR NEAR32		; 影判定関数
	LOCAL	pfnReflection:PTR NEAR32	; 反射・屈折適用関数

	LOCAL	vcLight:E3D_VECTOR4		; 正規化済光線
	LOCAL	rBrightness:REAL4		; 光源輝度
	LOCAL	rgbMulColor:E3D_PALLET_PW	; 拡散反射 RGB 色成分
	LOCAL	rgbAddColor:E3D_PALLET_PW	; 鏡面反射 RGB 色成分
	LOCAL	vNormal:E3D_VECTOR_PW		; 正規化済法線（x-100H）
	LOCAL	vcNormal:E3D_VECTOR4		; 正規化済法線（x-100H）
	LOCAL	vFocus:E3D_VECTOR_PW		; 正規化済視線（x100H）
	LOCAL	vcFocus:E3D_VECTOR4		; 正規化済視線（x100H）
	LOCAL	vFocusPos:E3D_VECTOR4		; 焦点座標
	LOCAL	rFocusPosDistance:REAL4
	LOCAL	rFocusParam:REAL4		; 視線ベクトルと法線ベクトルの内積
	LOCAL	rFocusPlaneParam:REAL4		; 視線ベクトルと平面法線ベクトルの内積

	LOCAL	dwSaveESP:DWORD
	LOCAL	dwTemp[3]:DWORD
	LOCAL	dwSaveBrightness[2]:DWORD
	LOCAL	mmxShadowColor:QWORD		; x4000H
	LOCAL	prmhpParam:PTR EGL_RENDER_MESH_HIT_PARAM
	LOCAL	dwShadowingCount:DWORD
	LOCAL	pvLightRays:PTR E3D_VECTOR4
	LOCAL	pvNextLightRays:PTR E3D_VECTOR4
	LOCAL	rmvpTemp:EGL_RENDER_MESH_VIEW_POINT
	LOCAL	rmhsSurface:EGL_RENDER_MESH_HIT_SURFACE
	LOCAL	clrTemp:E3D_COLOR
	LOCAL	vRay:E3D_VECTOR4
	LOCAL	vRayOrigin:E3D_VECTOR4
	LOCAL	rErrorGap:REAL4
	LOCAL	rShadowMaxDistance:REAL4
	LOCAL	rShadowDistance:REAL4
	LOCAL	nRayCounter:DWORD

	;
	;	表面属性共通パラメータ計算
	; ---------------------------------------------------------------------
	mov	ebx, hRenderPoly
	mov	esi, pSurfaceAttribute
	mov	dwSaveESP, esp
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	ASSUME	esi:PCE3D_SURFACE_ATTRIBUTE
	mov	eax, [esi].dwShadingFlags
	test	eax, E3DSAF_SHADING_MASK
	jz	Label_NoShading
	and	eax, [ebx].dwMaskShadingFlags
	or	eax, [ebx].dwAddShadingFlags
	mov	dwShadingFlags, eax
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
	movq	mm6, mmxConst7FFF_100
	pxor	mm7, mm7
	movd	mm0, [esi].nReflection
	movss	xmm0, [esi].nRefraction
	mov	nDeepness, ecx
	pcmpgtd	mm7, mm0
	paddsw	mm0, mm6
	mov	nTransparency, edx
	psubsw	mm0, mm6
	movss	nRefraction, xmm0
	pandn	mm7, mm0
	movd	nReflection, mm7
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
	xor	edx, edx
	mov	ecx, prgbaTextureImage
	test	dwShadingFlags, E3DSAF_TEXTURE_MAPPING
	mov	eax, OFFSET SubFunc_BlendTextureDummy
	cmovnz	edx, ecx
	mov	pfnBeforeTexture, eax
	mov	pfnAfterTexture, eax
	test	edx, edx
	.IF	!ZERO?
		mov	eax, OFFSET SubFunc_BlendTexture
		.IF	dwShadingFlags & E3DSAF_GOURAUD_SHADE
			mov	pfnAfterTexture, eax
		.ELSE
			mov	pfnBeforeTexture, eax
		.ENDIF
	.ENDIF
	;
	mov	eax, OFFSET SubFunc_ShadowHitTestDummy
	.IF	([ebx].nRayShadowingListCount != 0) && (dwReflectCount > 0)
		mov	eax, OFFSET SubFunc_ShadowHitTest
		mov	edx, OFFSET SubFunc_ShadowHitTestAlpha
		test	[ebx].rrtpRayParam.dwFlags, E3D_RAYTRACE_SHADOW_ALPHA
		cmovnz	eax, edx
	.ENDIF
	mov	pfnShadowing, eax
	;
	mov	eax, OFFSET SubFunc_Reflection
	mov	edx, OFFSET SubFunc_Refraction
	test	dwShadingFlags, E3DSAF_RAY_REFRACTING
	cmovnz	eax, edx
	mov	edx, OFFSET SubFunc_ReflectionDummy
	test	dwShadingFlags, (E3DSAF_RAY_REFLECTING OR E3DSAF_RAY_REFRACTING)
	cmovz	eax, edx
	cmp	dwReflectCount, 0
	cmovz	eax, edx
	mov	pfnReflection, eax
	;
	mov	ecx, dwReflectCount
	add	ecx, [ebx].rrtpRayParam.dwAppendShadowingCount
	mov	esi, pViewPoint
	ASSUME	esi:PTR EGL_RENDER_MESH_VIEW_POINT
	mov	dwShadowingCount, ecx
	imul	eax, ecx, (SIZEOF EGL_RENDER_MESH_HIT_ENTRY)
	add	eax, (SIZEOF EGL_RENDER_MESH_HIT_PARAM)
	sub	esp, eax
	and	esp, NOT 0FH
	mov	prmhpParam, esp
	mov	edi, esp
	ASSUME	edi:PTR EGL_RENDER_MESH_HIT_PARAM
	movq	mm0, MMWORD PTR [esi].pExceptingMesh
	movd	mm1, [ebx].rrtpRayParam.rShadowingDistance
	pxor	mm2, mm2
	pxor	mm3, mm3
	.IF	dwShadingFlags & E3DSAF_NO_SALF_SHADOW
		movd	mm3, [esi].pExceptingMesh
	.ENDIF
	psllq	mm1, 32
	movq	MMWORD PTR [edi].rRangeMin, mm1
	movq	MMWORD PTR [edi].pExceptingMesh, mm0
	movq	MMWORD PTR [edi].pExceptingMeshEntry, mm3
	mov	[edi].nResultLimit, ecx
	;
	imul	eax, ecx, (SIZEOF E3D_VECTOR4)
	sub	esp, eax
	mov	pvLightRays, esp
	;
	mov	ecx, [ebx].nVectorLightCount
	test	ecx, ecx
	mov	esi, [ebx].pVectorLights
	ASSUME	esi:PE3D_VECTOR_LIGHT_ENTRY
	mov	edi, pvLightRays
	.WHILE	!ZERO?
		movups	xmm0, [esi].vcLight
		xorps	xmm0, xmmSignBit
		mulps	xmm0, xmmPackedDiv256
		movups	XMMWORD_PTR [edi], xmm0
		add	esi, (SIZEOF E3D_VECTOR_LIGHT_ENTRY)
		add	edi, (SIZEOF E3D_VECTOR4)
		dec	ecx
	.ENDW
	;
	ASSUME	edi:NOTHING
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
	mov	eax, pViewPoint
	movups	xmm4, [esi]
	movups	xmm0, [edi]
	movups	xmm1, (EGL_RENDER_MESH_VIEW_POINT PTR [eax]).vViewPosition
		movaps	xmm7, xmm4
		subps	xmm0, xmm1
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
	movss	rFocusPosDistance, xmm0
	shufps	xmm0, xmm0, 0
		shufps	xmm4, xmm4, 0
	mulps	xmm0, xmm3			; 正規化済焦点（視線）ベクトル
		mulps	xmm4, xmm7		; 正規化済法線ベクトル
			movaps	xmm1, xmm0
			mulps	xmm0, xmm4
	mulps	xmm1, xmmPacked256
			movhlps	xmm2, xmm0
		mulps	xmm4, xmmPacked256
			movss	xmm3, xmm0
			shufps	xmm0, xmm0, 1
				movups	xmm7, vFocusPos
			addss	xmm2, xmm3
	movups	vcFocus, xmm1
			addss	xmm0, xmm2
				movups	xmm6, (EGL_RENDER_MESH_VIEW_POINT PTR [eax]).vTargetPlane
		xorps	xmm4, xmmSignBit
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
	mov	eax, pvLightRays
	mov	edx, [ebx].rrtpRayParam.rShadowingDistance
	movq	mm6, MMWORD PTR rgbAmbient
	pxor	mm7, mm7
;	movq	MMWORD PTR rgbMulColor, mm6
;	movq	MMWORD PTR rgbAddColor, mm7
	mov	pvNextLightRays, eax
	mov	rShadowMaxDistance, edx
	;
	call	pfnBeforeTexture

	;
	;	無限遠光源の計算
	; ---------------------------------------------------------------------
	mov	ecx, [ebx].nVectorLightCount
	mov	esi, [ebx].pVectorLights
	ASSUME	esi:PE3D_VECTOR_LIGHT_ENTRY
	test	ecx, ecx
	.WHILE	!ZERO?
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
		; 影判定
		;
		mov	ebx, pvNextLightRays
		mov	dwSaveBrightness[0], eax
		movss	dwSaveBrightness[4], xmm6
		movups	xmm0, XMMWORD_PTR [ebx]
		movq	MMWORD PTR rgbMulColor, mm6
		movq	MMWORD PTR rgbAddColor, mm7
		push	esi
		push	ecx
		movups	vRay, xmm0
		call	pfnShadowing
		pop	ecx
		pop	esi
		movq	mm6, MMWORD PTR rgbMulColor
		movq	mm7, MMWORD PTR rgbAddColor
		movss	xmm6, dwSaveBrightness[4]
		;
		; 拡散反射成分加算
		;
		;movd	mm4, dwSaveBrightness[0]
		movq	mm5, MMWORD PTR [esi].rgbColor
		;pshufw	mm4, mm4, 01000000B
				movups	xmm4, [esi].vcLight
			movq	mm2, mm5
		pmulhw	mm5, mm4
			pmullw	mm2, mm4
				movups	xmm5, vcNormal
				shufps	xmm6, xmm6, 0
		movq	mm4, mm2
		punpcklwd	mm2, mm5
		punpckhwd	mm4, mm5
				mulps	xmm5, xmm6
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
				movq	mm2, mmxShadowColor
				psllw	mm0, 2
				pmulhuw	mm0, mm2
				paddsw	mm7, mm0
			.ENDIF
		.ENDIF
		;
Label_VectorLight_Continue:
		mov	eax, pvNextLightRays
		mov	ebx, hRenderPoly
		add	eax, (SIZEOF E3D_VECTOR4)
		add	esi, (SIZEOF E3D_VECTOR_LIGHT_ENTRY)
		mov	pvNextLightRays, eax
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
			sqrtss	xmm1, xmm0
		rsqrtss	xmm0, xmm0
		;
			minss	xmm1, [ebx].rrtpRayParam.rShadowingDistance
		shufps	xmm0, xmm0, 0
			movss	rShadowMaxDistance, xmm1
		mulps	xmm4, xmm0
		mulss	xmm0, [esi].rBrightness
		movups	vcLight, xmm4
;		movss	rBrightness, xmm0
		;
		;	拡散反射光成分計算
		; ------------------------------------------------------------
		;
		; 光線ベクトルと法線の内積を計算
		;
		movups	xmm1, vcNormal
		;movaps	xmm5, xmm1		; xmm5 <= vcNormal
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
			movss	dwTemp, xmm1
;			cvtss2si	ebx, xmm1
		mulss	xmm1, xmm2		; xmm1 <= 輝度 * 10000H
		;
		; 判定マスク生成
		;
		mov	edx, rFocusPlaneParam
		mov	ebx, dwTemp
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
		; 影判定
		;
		movaps	xmm0, xmmSignBit
		mov	dwSaveBrightness[0], eax
		movss	dwSaveBrightness[4], xmm6
		xorps	xmm0, xmm4
		movq	MMWORD PTR rgbMulColor, mm6
		movq	MMWORD PTR rgbAddColor, mm7
		push	esi
		push	ecx
		movups	vRay, xmm0
		call	pfnShadowing
		pop	ecx
		pop	esi
		movq	mm6, MMWORD PTR rgbMulColor
		movq	mm7, MMWORD PTR rgbAddColor
		;
		; 拡散反射成分加算
		;
		;movd	mm4, dwSaveBrightness[0]
		movq	mm1, MMWORD PTR [esi].rgbColor
		;pshufw	mm4, mm4, 01000000B
		psllw	mm1, 2 + 4
		pmulhw	mm4, mm1
		paddsw	mm6, mm4
		;
		;	鏡面反射光成分計算
		; ------------------------------------------------------------
		.IF	nSpecular > 0
			;
			; 反射光ベクトルを計算
			;	L' = L - 2 * <L|A> / (|A|^2) * A
			;	   = L - <L|A> / 8000H * A
			;
			movss	xmm6, dwSaveBrightness[4]
			movups	xmm4, vcLight
			mulss	xmm6, rConstRcp128
			mulps	xmm4, xmmPacked256
			movups	xmm5, vcNormal
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
				movq	mm2, mmxShadowColor
				psllw	mm0, 2
				pmulhuw	mm0, mm2
				paddsw	mm7, mm0
			.ENDIF
		.ENDIF
		;
Label_PointLight_Continue:
		mov	ebx, hRenderPoly
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

	;
	;	擬似フォッグ適用
	; --------------------------------------------------------------------
	movss	xmm1, rFocusPosDistance
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
	;	反射と屈折
	; --------------------------------------------------------------------
	call	pfnReflection

	;
	;	色情報出力
	; --------------------------------------------------------------------
	mov	eax, pNormals
	mov	edx, pFocusPoints
	mov	ecx, nCount
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
	mov	esp, dwSaveESP
	xor	eax, eax
	ret


Label_NoShading:
	xor	edx, edx
	mov	ecx, prgbaTextureImage
	test	eax, E3DSAF_TEXTURE_MAPPING
	cmovnz	edx, ecx
	.IF	edx != NULL
		mov	ecx, nCount
		mov	esi, pColorsLooks
		mov	eax, prgbaTextureImage
		ASSUME	esi:PTR E3D_COLOR
		test	ecx, ecx
		.WHILE	!ZERO?
			pxor		mm4, mm4
			pcmpeqd		mm2, mm2
			movd		mm6, [esi].rgbMul.dwPixelCode
			movd		mm7, [esi].rgbAdd.dwPixelCode
			punpcklbw	mm6, mm4
			punpcklbw	mm7, mm4
			;
			psubsw		mm6, mm2
			movd		mm0, DWORD PTR [eax]
			add		eax, 4
			punpcklbw	mm0, mm4
			psllw		mm0, 7
			movq		mm3, mm2
			pshufw		mm2, mm0, 11111111B
			paddsw		mm6, mm6
			paddsw		mm7, mm7
			pmulhw		mm6, mm0
			pmulhw		mm7, mm2
			psrlw		mm3, 8
			psrlw		mm2, 7
			pxor		mm3, mm2
			paddsw		mm7, mm6
			;
			packuswb	mm3, mm7
			movq		MMWORD PTR [esi], mm3
			add		esi, 8
			;
			dec	ecx
		.ENDW
		ASSUME	esi:NOTHING
	.ENDIF
	;
	mov	esp, dwSaveESP
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
	jmp	pfnAfterTexture
;	BYTE	0C3H	; ret

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

	jmp	pfnAfterTexture
;	BYTE	0C3H	; ret


ALIGN	10H
SubFunc_BlendTexture:
	;
	;	テクスチャ適用関数
	; --------------------------------------------------------------------
	; mm6 = シェーディング乗算成分
	; mm7 = シェーディング加算成分
	; mm6' = 0FFH - (テクスチャα)
	; mm7' = mm6 * (テクスチャRGB) + mm7 * (テクスチャα)
	; ※ mm7 * (テクスチャα) 項は、テクスチャの透明度によるマスクの為
	mov		eax, prgbaTextureImage
	pcmpeqd		mm2, mm2
	pxor		mm1, mm1
	psubsw		mm6, mm2
	movd		mm0, DWORD PTR [eax]
	add		eax, 4
	punpcklbw	mm0, mm1
	mov		prgbaTextureImage, eax
	psllw		mm0, 7
	movq		mm3, mm2
	pshufw		mm2, mm0, 11111111B
	paddsw		mm6, mm6
	paddsw		mm7, mm7
	pmulhw		mm6, mm0
	pmulhw		mm7, mm2
	psrlw		mm3, 8
	psrlw		mm2, 7
	pxor		mm3, mm2
	paddsw		mm7, mm6
	movq		mm6, mm3

SubFunc_BlendTextureDummy:
	BYTE	0C3H	; ret


ALIGN	10H
SubFunc_ShadowHitTestDummy:
	;
	;	影当たり判定ダミー関数
	; --------------------------------------------------------------------
	movq	mm5, mmxConst4000H
	movd	mm4, dwSaveBrightness[0]
	movq	mmxShadowColor, mm5
	pshufw	mm4, mm4, 01000000B
	BYTE	0C3H	; ret


ALIGN	10H
SubFunc_ShadowHitTest:
	;
	;	影当たり判定
	; --------------------------------------------------------------------
	mov	edx, pFocusPoints
		movups	xmm1, vcNormal
		movss	xmm2, dwSaveBrightness[4]
		movaps	xmm3, xmmSignBit
	mov	ecx, (E3D_VECTOR4 PTR [edx]).z
		mulps	xmm1, xmmPackedDiv256
	and	ecx, 7FFFFFFFH
		shufps	xmm2, xmm2, 0
	sub	ecx, (10 SHL 23)
	sbb	eax, eax
	mov	edi, prmhpParam
	mov	ebx, hRenderPoly
	ASSUME	edi:PTR EGL_RENDER_MESH_HIT_PARAM
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	not	eax
	and	ecx, eax
	mov	eax, rShadowMaxDistance
		movups	xmm0, [edx]
	mov	[edi].rRangeMin, ecx
	mov	[edi].rRangeMax, eax
		movss	xmm4, [edi].rRangeMin
		andps	xmm2, xmm3
		shufps	xmm4, xmm4, 0
		xorps	xmm1, xmm2
		mulps	xmm1, xmm4
;	mov	[edi].rRangeMin, 0
		subps	xmm0, xmm1
		movups	vRayOrigin, xmm0
	mov	[edi].nResultLimit, 1
	;
	INVOKE	eglRenderPoly@MeshList@IsHitSegmentSSE,
			[ebx].ppRayShadowingList,
			[ebx].nRayShadowingListCount,
			edi, ADDR vRay, ADDR vRayOrigin
	;
	cmp	[edi].nResultCount, 0
	jz	SubFunc_ShadowHitTestDummy
	;
	xorps		xmm4, xmm4
	movss		xmm3, [edi].rmheResults.rDistance
	movss		xmm0, [ebx].rRcpShadowDistance
	maxss		xmm3, xmm4
	cvtsi2ss	xmm1, dwSaveBrightness[0]
	mulss		xmm0, xmm3
	movss		xmm2, xmmPacked4000H
	shufps		xmm0, xmm0, 0
	unpcklps	xmm1, xmm2
	mulps		xmm0, xmm1
	cvtps2pi	mm5, xmm0
	pshufw		mm4, mm5, 11000000B
	pshufw		mm5, mm5, 11101010B
	movq		mmxShadowColor, mm5

	ASSUME	edi:NOTHING
	ASSUME	ebx:NOTHING
	BYTE	0C3H	; ret


ALIGN	10H
SubFunc_ShadowHitTestAlpha:
	;
	;	影当たり判定（透明度考慮）
	; --------------------------------------------------------------------
	mov	edi, prmhpParam
	ASSUME	edi:PTR EGL_RENDER_MESH_HIT_PARAM
	mov	eax, dwShadowingCount
	mov	edx, pFocusPoints
	movq	mm0, mmxConst4000H
		movups	xmm1, vcNormal
		movss	xmm2, dwSaveBrightness[4]
		movaps	xmm3, xmmSignBit
	mov	[edi].nResultLimit, eax
	mov	rShadowDistance, 0
	movq	mmxShadowColor, mm0
	;
	mov	ecx, (E3D_VECTOR4 PTR [edx]).z
		mulps	xmm1, xmmPackedDiv256
	and	ecx, 7FFFFFFFH
		shufps	xmm2, xmm2, 0
	sub	ecx, (10 SHL 23)
	sbb	eax, eax
	mov	ebx, hRenderPoly
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	not	eax
	and	ecx, eax
	mov	eax, rShadowMaxDistance
		movups	xmm0, [edx]
	mov	rErrorGap, ecx
	mov	[edi].rRangeMin, ecx
	mov	[edi].rRangeMax, eax
		movss	xmm4, [edi].rRangeMin
		andps	xmm2, xmm3
		shufps	xmm4, xmm4, 0
		xorps	xmm1, xmm2
		mulps	xmm1, xmm4
;	mov	[edi].rRangeMin, 0
		subps	xmm0, xmm1
		movups	vRayOrigin, xmm0
	;
	INVOKE	eglRenderPoly@MeshList@IsHitSegmentSSE,
			[ebx].ppRayShadowingList,
			[ebx].nRayShadowingListCount,
			edi, ADDR vRay, ADDR vRayOrigin
	;
	mov	eax, [edi].nResultCount
	lea	edi, [edi].rmheResults
	test	eax, eax
	jz	SubFunc_ShadowHitTestDummy
	;
Label_ShadowHitTestAlpha_LoopBegin:
	ASSUME	edi:PTR EGL_RENDER_MESH_HIT_ENTRY
	mov	nRayCounter, eax
	;
	INVOKE	eglRenderPoly@Polygon@GetHitPointSurface,
			hRenderPoly, ADDR rmhsSurface, edi
	;
	.IF	rmhsSurface.dwShadingFlags & E3DSAF_TEXTURE_MAPPING
;			mov	ecx, nRayCounter
		pcmpeqb	mm6, mm6
;			mov	eax, 1
;			cmp	rmhsSurface.rgbaTexture.dwPixelCode, 0FE000000H
;			cmovae	ecx, eax
			xor	edx, edx
		movd	mm0, rmhsSurface.rgbaTexture.dwPixelCode
;			mov	nRayCounter, ecx
			mov	eax, rmhsSurface.nDeepness
		pxor	mm0, mm6
			mov	ecx, rmhsSurface.nTransparency
		psrld	mm0, 24
			test	rmhsSurface.dwShadingFlags, E3DSAF_GOURAUD_SHADE
		pshufw	mm0, mm0, 01000000B
			cmovnz	eax, edx
	.ELSE
		mov	edx, rmhsSurface.clrSurface.rgbMul.dwPixelCode
;			mov	eax, 1
;			mov	ecx, nRayCounter
;			and	edx, 00FFFFFFH
;			cmovz	ecx, eax
		movd	mm0, edx
		pxor	mm7, mm7
;			mov	nRayCounter, ecx
		punpcklbw	mm0, mm7
			mov	eax, rmhsSurface.nDeepness
			mov	ecx, rmhsSurface.nTransparency
	.ENDIF
	.IF	eax != 0		; 透明深度適用
		movups	xmm0, vRay
		movups	xmm1, rmhsSurface.vNormal
		mulps	xmm0, xmm1
		cvtsi2ss	xmm4, eax
		movhlps	xmm2, xmm0
		addss	xmm2, xmm0
		shufps	xmm0, xmm0, 1
		addss	xmm0, xmm2
		mulss	xmm0, xmm0
		mulss	xmm4, xmm0
		cvtss2si	eax, xmm4
		add	ecx, eax
	.ENDIF
	.IF	(SDWORD PTR ecx) > 0	; 透明度適用
		xor	eax, eax
		sub	ecx, 100H
		movq	mm1, mmxConst256
		neg	ecx
		psubsw	mm0, mm1
		cmovs	ecx, eax
		;
		paddsw	mm0, mm0
		shl	ecx, 7
		movd	mm2, ecx
		pshufw	mm2, mm2, 11000000B
		;
		pmulhw	mm0, mm2
		paddsw	mm0, mm1
	.ENDIF
	;
		movq	mm4, mm0
		mov	ecx, 1
		mov	eax, nRayCounter
		packuswb	mm4, mm4
	;
	movss		xmm0, [ebx].rRcpShadowDistance
		movd	edx, mm4
	xorps		xmm7, xmm7
		and	edx, 00FFFFFFH
	movss		xmm1, [edi].rDistance
;		cmovz	eax, ecx
		.IF	!ZERO?
			nop
		.ENDIF
	maxss		xmm1, xmm7
	movss		xmm6, [ebx].rrtpRayParam.rShadowingDistance
		pxor		mm7, mm7
		movq		mm1, mm0
;	addss		xmm1, rShadowDistance
		movd		mm2, DWORD PTR mmxShadowColor[0]
		movd		mm3, DWORD PTR mmxShadowColor[4]
;	movss		rShadowDistance, xmm1
	subss		xmm6, xmm1
	;
	punpcklwd	mm0, mm7
		punpcklwd	mm2, mm7
	punpckhwd	mm1, mm7
		punpcklwd	mm3, mm7
			mulss	xmm6, xmmPackedDiv256
	cvtpi2ps	xmm2, mm0
	cvtpi2ps	xmm3, mm1
			shufps	xmm1, xmm1, 11000000B
	movlhps		xmm2, xmm3
	shufps		xmm6, xmm6, 0
			shufps	xmm0, xmm0, 11000000B
	mulps		xmm2, xmm6
		cvtpi2ps	xmm4, mm2
		cvtpi2ps	xmm5, mm3
	addps		xmm2, xmm1
		movlhps		xmm4, xmm5
	mulps		xmm0, xmm2
	mulps		xmm0, xmm4
	;
	cvtps2pi	mm0, xmm0
	movhlps		xmm0, xmm0
;		mov	eax, nRayCounter
	cvtps2pi	mm1, xmm0
		add	edi, (SIZEOF EGL_RENDER_MESH_HIT_ENTRY)
		dec	eax
	packssdw	mm0, mm1
	movq		mmxShadowColor, mm0
	;
	jnz	Label_ShadowHitTestAlpha_LoopBegin

Label_ShadowHitTestAlpha_LoopEnd:
	pcmpeqw	mm1, mm1
	movq	mm0, mmxConst4000H
	paddw	mm0, mm1
	pxor	mm0, mm1
	movq	mm4, mmxShadowColor
		movd	mm5, dwSaveBrightness[0]
	paddusw	mm4, mm0
		pshufw	mm5, mm5, 11000000B
	psubusw	mm4, mm0
		psllw	mm5, 2
	movq	mmxShadowColor, mm4
	pmulhw	mm4, mm5

	ASSUME	edi:NOTHING
	ASSUME	ebx:NOTHING
	BYTE	0C3H	; ret


ALIGN	10H
SubFunc_Refraction:
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	movq	mm0, mmxConst1
	;
	;	屈折処理
	; --------------------------------------------------------------------
	;
	; 屈折光線ベクトルを計算
	;
	mov	edi, prmhpParam
	ASSUME	edi:PTR EGL_RENDER_MESH_HIT_PARAM
	movss	xmm2, nRefraction
	movups	xmm1, vcNormal
			pcmpgtw	mm0, mm6
	movups	xmm0, vcFocus
	shufps	xmm2, xmm2, 0
			packsswb	mm0, mm0
		mov	edx, pViewPoint
	mulps	xmm2, xmm1
			movd	eax, mm0
			test	eax, eax
			jnz	SubFunc_Reflection
		ASSUME	edx:PTR EGL_RENDER_MESH_VIEW_POINT
		mov	eax, [edx].pExceptingMesh
		mov	ecx, [edx].nExceptingMeshIndex
	addps	xmm0, xmm2
		mov	[edi].pExceptingMesh, eax
		mov	[edi].nExceptingMeshIndex, ecx
		mov	[edi].pExceptingMeshEntry, NULL
		mov	[edi].nResultLimit, 1
		ASSUME	edx:NOTHING
	;
	; 光線ベクトルの正規化と、臨界角の判定
	;
	movaps	xmm4, xmm0
	movaps	xmm3, xmm0
	mulps	xmm0, xmm0
		mulps	xmm4, xmm2
	movhlps	xmm2, xmm0
		movhlps	xmm5, xmm4
	addss	xmm2, xmm0
	shufps	xmm0, xmm0, 1
		addss	xmm5, xmm4
		shufps	xmm4, xmm4, 1
	addss	xmm0, xmm2
		addss	xmm4, xmm5
	rsqrtss	xmm0, xmm0
		movss	dwTemp[0], xmm4
	shufps	xmm0, xmm0, 0
		mov	eax, dwTemp[0]
	mulps	xmm0, xmm3
		xor	eax, rFocusParam
		jns	SubFunc_Reflection	; 臨界角
	movups	vRay, xmm0
	;
	; 屈折光線追跡
	;
		mulps	xmm1, xmmPackedDiv256
	mov	edx, pFocusPoints
		movss	xmm0, rFocusParam
		movaps	xmm3, xmmSignBit
	mov	ecx, (E3D_VECTOR4 PTR [edx]).z
		shufps	xmm0, xmm0, 0
	and	ecx, 7FFFFFFFH
		andps	xmm0, xmm3
		movups	xmm4, [edx]
	sub	ecx, (9 SHL 23)
	sbb	eax, eax
	not	eax
	and	ecx, eax
	mov	eax, [ebx].rrtpRayParam.rRayTracingDistance
		xorps	xmm1, xmm0
	mov	[edi].rRangeMin, ecx
	mov	[edi].rRangeMax, eax
		movss	xmm2, [edi].rRangeMin
		shufps	xmm2, xmm2, 0
		mulps	xmm2, xmm1
	movq	MMWORD PTR rgbMulColor, mm6
	movq	MMWORD PTR rgbAddColor, mm7
		subps	xmm4, xmm2
	mov	eax, dwReflectCount
		movups	vRayOrigin, xmm4
	mov	nRayCounter, eax
		movups	rmvpTemp.vViewPosition, xmm4
	;
Label_RefractionContinue:
	INVOKE	eglRenderPoly@MeshList@IsHitSegmentSSE,
			[ebx].ppRayReflectionList,
			[ebx].nRayReflectionListCount,
			edi, ADDR vRay, ADDR vRayOrigin
	;
	.IF	[edi].nResultCount == 0
		mov	[edi].rRangeMax, 7F000000H
		INVOKE	eglRenderPoly@MeshArray@IsHitSegmentSSE,
				[ebx].pGlobalRayReflection,
				[ebx].nGlobalRayReflections,
				edi, ADDR vRay, ADDR vRayOrigin
		;
		cmp	[edi].nResultCount, 0
		pxor	mm6, mm6
		movq	mm7, MMWORD PTR rgbAddColor
		jz	SubFunc_Reflection
	.ENDIF
	;
	; 屈折シェーディング
	;
	mov	eax, [edi].rmheResults.pHitMesh
	mov	edx, [edi].rmheResults.nHitMeshIndex
	mov	rmvpTemp.pExceptingMesh, eax
	mov	rmvpTemp.nExceptingMeshIndex, edx
	;
	INVOKE	eglRenderPoly@Polygon@GetHitPointSurface,
			hRenderPoly, ADDR rmhsSurface, ADDR [edi].rmheResults
	;
	movups	xmm0, rmhsSurface.vPlane
	mov	eax, rmhsSurface.dwShadingFlags
	mov	edx, dwReflectCount
	mov	ecx, rmhsSurface.pAttr
	dec	edx
	movups	rmvpTemp.vTargetPlane, xmm0
	;
	.IF	eax & E3DSAF_SINGLE_SIDE_PLANE
		movups	xmm0, rmhsSurface.vNormal
		movups	xmm1, vRay
		mulps	xmm0, xmm1
		xorps	xmm2, xmm2
		movups	xmm3, XMMWORD_PTR [edi].rmheResults.vHitPosition
		movhlps	xmm1, xmm0
		addss	xmm1, xmm0
		shufps	xmm0, xmm0, 1
		addss	xmm0, xmm1
		comiss	xmm0, xmm2
		.IF	!CARRY?
			;
			; ポリゴン裏面
			;
			mov	ecx, [ebx].rrtpRayParam.rRayTracingDistance
			mov	eax, [edi].rmheResults.pHitMesh
			mov	edx, [edi].rmheResults.nHitMeshIndex
			movups	vRayOrigin, xmm3
;			mov	[edi].rRangeMin, 0
			mov	[edi].rRangeMax, ecx
			mov	[edi].nResultCount, 0
			mov	[edi].pExceptingMesh, eax
			mov	[edi].nExceptingMeshIndex, edx
			dec	nRayCounter
			jnz	Label_RefractionContinue
			pxor	mm6, mm6
			movq	mm7, MMWORD PTR rgbAddColor
			jmp	SubFunc_Reflection
		.ENDIF
	.ENDIF
	;
	INVOKE	eglRenderPoly@ShadeVectorsAndRaySSE,
			ebx, ecx,
			ADDR rmhsSurface.vNormal,
			ADDR [edi].rmheResults.vHitPosition,
			ADDR rmhsSurface.clrSurface,
			ADDR rmhsSurface.rgbaTexture, 1,
			ADDR rmvpTemp, edx
	;
	movd	mm0, rmhsSurface.clrSurface.rgbAdd.dwPixelCode
	pxor	mm6, mm6
	movq	mm5, MMWORD PTR rgbMulColor
	movq	mm7, MMWORD PTR rgbAddColor
	punpcklbw	mm0, mm6
	paddsw	mm5, mm5
	psllw	mm0, 7
	pmulhw	mm5, mm0
	paddsw	mm7, mm5

SubFunc_Reflection:
	cmp	nReflection, 0
	jz	SubFunc_ReflectionDummy
	test	dwShadingFlags, E3DSAF_RAY_REFLECTING
	jz	SubFunc_ReflectionDummy
	;
	;	反射処理
	; --------------------------------------------------------------------
	;
	; 反射ベクトルを計算
	;	L' = L - 2 * <L|A> / (|A|^2) * A
	;	   = L - <L|A> / 8000H * A
	;
	mov	edi, prmhpParam
	ASSUME	edi:PTR EGL_RENDER_MESH_HIT_PARAM
		mov	edx, pFocusPoints
	movups	xmm4, vcFocus
	movups	xmm6, vcNormal
	movaps	xmm5, xmm6
		mov	ecx, (E3D_VECTOR4 PTR [edx]).z
	mulps	xmm6, xmm4
		and	ecx, 7FFFFFFFH
		movups	xmm0, [edx]
		sub	ecx, (9 SHL 23)
		sbb	eax, eax
	movhlps	xmm7, xmm6
		not	eax
	addss	xmm7, xmm6
	shufps	xmm6, xmm6, 1
		and	ecx, eax
		mov	eax, [ebx].rrtpRayParam.rRayTracingDistance
	addss	xmm6, xmm7
		movaps	xmm7, xmmSignBit
		mov	[edi].rRangeMin, ecx
		mov	[edi].rRangeMax, eax
		movss	xmm1, [edi].rRangeMin
		andps	xmm7, xmm6
	mulss	xmm6, rConstRcp8000H
		movups	xmm2, vcNormal
		mulss	xmm1, xmmPackedDiv256
		mov	edx, pViewPoint
		movaps	xmm3, xmm2
	shufps	xmm6, xmm6, 0
		shufps	xmm1, xmm1, 11000000B
		shufps	xmm7, xmm7, 0
	mulps	xmm5, xmm6
		ASSUME	edx:PTR EGL_RENDER_MESH_VIEW_POINT
		mov	eax, [edx].pExceptingMesh
		mov	ecx, [edx].nExceptingMeshIndex
		ASSUME	edx:NOTHING
		mulps	xmm2, xmm1
		mov	[edi].pExceptingMesh, eax
		mov	[edi].nExceptingMeshIndex, ecx
		mov	[edi].pExceptingMeshEntry, NULL
		mov	[edi].nResultLimit, 1
	subps	xmm4, xmm5
		xorps	xmm2, xmm7
	mulps	xmm4, xmmPackedDiv256
		subps	xmm0, xmm2
	movq	MMWORD PTR rgbMulColor, mm6
	movq	MMWORD PTR rgbAddColor, mm7
	mov	eax, dwReflectCount
	movups	vRay, xmm4
		movups	vRayOrigin, xmm0
	mov	nRayCounter, eax
		movups	rmvpTemp.vViewPosition, xmm0
	;
Label_ReflectionContinue:
	;
	; 反射光線追跡
	;
	INVOKE	eglRenderPoly@MeshList@IsHitSegmentSSE,
			[ebx].ppRayReflectionList,
			[ebx].nRayReflectionListCount,
			edi, ADDR vRay, ADDR vRayOrigin
	;
	.IF	[edi].nResultCount == 0
		mov	[edi].rRangeMax, 7F000000H
		INVOKE	eglRenderPoly@MeshArray@IsHitSegmentSSE,
				[ebx].pGlobalRayReflection,
				[ebx].nGlobalRayReflections,
				edi, ADDR vRay, ADDR vRayOrigin
		;
		cmp	[edi].nResultCount, 0
		jz	Label_ReflectionExitNoReflection
	.ENDIF
	;
	; 反射シェーディング
	;
	mov	eax, [edi].rmheResults.pHitMesh
	mov	edx, [edi].rmheResults.nHitMeshIndex
	mov	rmvpTemp.pExceptingMesh, eax
	mov	rmvpTemp.nExceptingMeshIndex, edx
	;
	INVOKE	eglRenderPoly@Polygon@GetHitPointSurface,
			hRenderPoly, ADDR rmhsSurface, ADDR [edi].rmheResults
	;
	movups	xmm0, rmhsSurface.vPlane
	mov	eax, rmhsSurface.dwShadingFlags
	mov	edx, dwReflectCount
	mov	ecx, rmhsSurface.pAttr
	dec	edx
	movups	rmvpTemp.vTargetPlane, xmm0
	;
	.IF	eax & E3DSAF_SINGLE_SIDE_PLANE
		movups	xmm0, rmhsSurface.vNormal
		movups	xmm1, vRay
		mulps	xmm0, xmm1
		xorps	xmm2, xmm2
		movups	xmm3, XMMWORD_PTR [edi].rmheResults.vHitPosition
		movhlps	xmm1, xmm0
		addss	xmm1, xmm0
		shufps	xmm0, xmm0, 1
		addss	xmm0, xmm1
		comiss	xmm0, xmm2
		.IF	!CARRY?
			;
			; ポリゴン裏面
			;
			mov	ecx, [ebx].rrtpRayParam.rRayTracingDistance
			mov	eax, [edi].rmheResults.pHitMesh
			mov	edx, [edi].rmheResults.nHitMeshIndex
			movups	vRayOrigin, xmm3
;			mov	[edi].rRangeMin, 0
			mov	[edi].rRangeMax, ecx
			mov	[edi].nResultCount, 0
			mov	[edi].pExceptingMesh, eax
			mov	[edi].nExceptingMeshIndex, edx
			dec	nRayCounter
			jnz	Label_ReflectionContinue
			jmp	Label_ReflectionExitNoReflection
		.ENDIF
	.ENDIF
	;
	INVOKE	eglRenderPoly@ShadeVectorsAndRaySSE,
			ebx, ecx,
			ADDR rmhsSurface.vNormal,
			ADDR [edi].rmheResults.vHitPosition,
			ADDR rmhsSurface.clrSurface,
			ADDR rmhsSurface.rgbaTexture, 1,
			ADDR rmvpTemp, edx
	;
	movd	mm2, nReflection
	movd	mm5, rmhsSurface.clrSurface.rgbMul.dwPixelCode
	pcmpeqw	mm4, mm4
	pxor	mm6, mm6
		movd	mm0, rmhsSurface.clrSurface.rgbAdd.dwPixelCode
	pshufw	mm2, mm2, 01000000B
	pxor	mm5, mm4
	punpcklbw	mm5, mm6
		punpcklbw	mm0, mm6
	pmullw	mm5, mm2
		movq	mm7, MMWORD PTR rgbAddColor
		movq	mm6, MMWORD PTR rgbMulColor
	pxor	mm5, mm4
		pmullw	mm0, mm2
	psrlw	mm5, 1
	paddsw	mm7, mm7
	pmulhw	mm7, mm5
		psrlw	mm0, 8
	paddsw	mm7, mm0

SubFunc_ReflectionDummy:
	BYTE	0C3H	; ret


Label_ReflectionExitNoReflection:
;	mov	eax, 100H
	movq	mm6, MMWORD PTR rgbMulColor
;	sub	eax, nReflection
	movq	mm7, MMWORD PTR rgbAddColor
;	movd	mm0, eax
;	paddsw	mm7, mm7
;	pshufw	mm0, mm0, 11000000B
;	psllw	mm0, 7
;	pmulhw	mm7, mm0

	BYTE	0C3H	; ret


eglRenderPoly@ShadeVectorsAndRaySSE	ENDP


CodeSeg	ENDS

	END
