
; ----------------------------------------------------------------------------
;                          Entis Graphic Library
; ----------------------------------------------------------------------------
;    Copyright (c) 2009 Leshade Entis, Entis-soft. Al rights reserved.
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
xmmSignBit		DWORD	4 DUP( 80000000H )
xmmPacked128		REAL4	4 DUP( 128.0 )
xmmPacked256		REAL4	4 DUP( 256.0 )
xmmPackedDiv256		REAL4	4 DUP( 0.00390625 )
xmmPacked4000H		REAL4	4 DUP( 16384.0 )
rConstRcp128		REAL4	4 DUP( 0.0078125 )	; = 1.0 / 128.0
rConstRcp8000H		REAL4	4 DUP( 0.000030517578125 )	; = 1.0 / 8000H

mmxConstDW_FFH	LABEL	MMWORD
		DWORD	2 DUP( 0FFH )
mmxConstW_0_0_1_0	LABEL	MMWORD
		WORD	0, 0, 1, 0
mmxConstW_0_1_0_1	LABEL	MMWORD
		WORD	0, 1, 0, 1

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
eglRenderPoly@ShadeByGPURayTraceSSE	PROC	NEAR32 C USES ebx esi edi,
	hRenderPoly:HEGL_RENDER_POLYGON,
	pShadeColor:PTR E3D_COLOR,
	pRayHit:PTR E3D_GPU_RAY_HIT_ENTRY,
	nReflectionCount:DWORD, nAppendTracingCount:DWORD

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

	LOCAL	vTargetPlane:E3D_VECTOR4	; 平面法線ベクトル
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
	LOCAL	rShadowMaxDistance:REAL4

	LOCAL	vHitPos:E3D_VECTOR4
	LOCAL	rgbColorShade:E3D_COLOR
	LOCAL	rgbColorTemp:E3D_COLOR
	LOCAL	rgbaTexture:EGL_PALETTE
	LOCAL	pUVs:PE3D_VECTOR_2D
	LOCAL	pNormals:PE3D_VECTOR4
	LOCAL	pVertexes:PE3D_VECTOR4
	LOCAL	psfAttr:PE3D_SURFACE_ATTRIBUTE
	LOCAL	pNextRayHit:PTR E3D_GPU_RAY_HIT_ENTRY
	LOCAL	nRayHitLightEntryStepBytes:DWORD
	LOCAL	nRayCounter:DWORD

	mov	esi, pRayHit
	mov	dwSaveESP, esp
	ASSUME	esi:PTR E3D_GPU_RAY_HIT_ENTRY
Label_Start:
	mov	edi, [esi].ppeMesh		; ターゲットが無い
	mov	eax, 1
	test	edi, edi
	jz	Label_Exit

	ASSUME	edi:PE3D_POLYGON_ENTRY
	mov	eax, [edi].dwShadingFlags
	mov	edx, [edi].pAttr
	mov	dwShadingFlags, eax
	mov	psfAttr, edx

	;
	;	法線・UV 座標計算
	; ---------------------------------------------------------------------
	mov	ecx, [edi].surface.mesh.pUVMap
	mov	edx, [edi].pNormals
	mov	eax, [edi].pVertexes
	mov	ebx, [edi].pVertexColors
	mov	pUVs, ecx
	mov	pNormals, edx
	mov	pVertexes, eax
	;
	mov	ecx, [esi].iPolyEntry
	mov	eax, [edi].surface.mesh.pMesh
	shl	ecx, 4
	lea	eax, (E3D_PRIMITIVE_MESH_LIST PTR [eax]).mpEntries[ecx]
	ASSUME	eax:PTR E3D_PRIMITIVE_MESH_POLY
	mov	ecx, [eax].dwIndex[4]
	mov	edx, [eax].dwIndex[8]
	mov	eax, [eax].dwIndex[0]
	ASSUME	eax:NOTHING
	ASSUME	edi:NOTHING
	;
	sub	ecx, eax		; ecx = (i1 - i0) * 8
	sub	edx, eax		; edx = (i2 - i0) * 8
	mov	edi, pVertexes
	shl	eax, 3
	shl	ecx, 3
	shl	edx, 3
	lea	edi, [edi + eax * 2]	; edi = 頂点(0) ポインタ
	lea	ebx, [ebx + eax]	; ebx = 頂点色(0) ポインタ
	add	eax, pUVs		; eax = UV(0) 座標ポインタ
	;
	movaps	xmm5, [edi]		; xmm5 = ポリゴン平面法線
	movaps	xmm1, [edi + ecx * 2]
	movaps	xmm2, [edi + edx * 2]
			pxor	mm4, mm4
			movd	mm6, [ebx]
			movd	mm7, [ebx + 4]
			movd	mm0, [ebx + ecx]
			movd	mm1, [ebx + ecx + 4]
			movd	mm2, [ebx + edx]
			movd	mm3, [ebx + edx + 4]
	subps	xmm1, xmm5
	subps	xmm2, xmm5
			punpcklbw	mm6, mm4
			punpcklbw	mm7, mm4
			punpcklbw	mm0, mm4
			punpcklbw	mm1, mm4
			punpcklbw	mm2, mm4
			punpcklbw	mm3, mm4
	movaps	xmm5, xmm1
	movaps	xmm4, xmm2
			movlps	xmm6, QWORD PTR [esi].vUV
	shufps	xmm1, xmm1, 11010010B
	shufps	xmm2, xmm2, 11001001B
			mulps	xmm6, xmmPacked128
	shufps	xmm5, xmm5, 11001001B
	shufps	xmm4, xmm4, 11010010B
	mulps	xmm1, xmm2
			psubw	mm0, mm6
			psubw	mm1, mm7
			psubw	mm2, mm6
			psubw	mm3, mm7
	mulps	xmm5, xmm4
			cvtps2pi	mm4, xmm6
	subps	xmm5, xmm1
			movq	mm5, mm4
			pshufw	mm4, mm4, 00000000B
			pshufw	mm5, mm5, 10101010B
			pmullw	mm0, mm4
			pmullw	mm1, mm4
	;
	.IF	pNormals != NULL
		sub	edi, pVertexes
		add	edi, pNormals
		movaps	xmm0, [edi]	; xmm0 = normal0
		movaps	xmm1, [edi + ecx * 2]
		movaps	xmm2, [edi + edx * 2]
		;
		movaps	xmm7, xmm5
		mulps	xmm5, xmm0
		movss	xmm3, xmm5
		movhlps	xmm4, xmm5
		shufps	xmm5, xmm5, 1
		addss	xmm3, xmm4
		addss	xmm5, xmm3
		shufps	xmm5, xmm5, 0
		andps	xmm5, xmmSignBit
		xorps	xmm5, xmm7
	.ELSE
		movaps	xmm0, xmm5
		movaps	xmm1, xmm5
		movaps	xmm2, xmm5
	.ENDIF
	;
			pmullw	mm2, mm5
			pmullw	mm3, mm5
			psraw	mm0, 7
			psraw	mm1, 7
	subps	xmm1, xmm0
		movss	xmm6, [esi].vUV.x
			psraw	mm2, 7
			psraw	mm3, 7
	subps	xmm2, xmm0
		movss	xmm7, [esi].vUV.y
			paddsw	mm6, mm0
			paddsw	mm7, mm1
		shufps	xmm6, xmm6, 0
		shufps	xmm7, xmm7, 0
	movups	vTargetPlane, xmm5
			paddsw	mm6, mm2
			paddsw	mm7, mm3
	mulps	xmm1, xmm6
			packuswb	mm6, mm7
	mulps	xmm2, xmm7
			movq	MMWORD PTR rgbColorShade, mm6
	;
	.IF	pUVs != NULL
		movlps	xmm3, QWORD PTR [eax]
		movlps	xmm4, QWORD PTR [eax + ecx]
		movlps	xmm5, QWORD PTR [eax + edx]
	.ENDIF
	subps	xmm4, xmm3
	subps	xmm5, xmm3
	mulps	xmm4, xmm6
	mulps	xmm5, xmm7
	;
	addps	xmm0, xmm1		; xmm0 = 法線ベクトル
		addps	xmm3, xmm4	; xmm3 = UV 座標
	addps	xmm0, xmm2
		addps	xmm3, xmm5
	movups	vcNormal, xmm0
	;
	; 裏面ポリゴン判定
	;
	.IF	dwShadingFlags & E3DSAF_SINGLE_SIDE_PLANE
		movups	xmm0, vTargetPlane
		movups	xmm2, [esi].vRay
		mulps	xmm0, xmm2
		xorps	xmm2, xmm2
		movss	xmm6, xmm0
		movhlps	xmm7, xmm0
		shufps	xmm0, xmm0, 1
		addss	xmm6, xmm7
		addss	xmm0, xmm6
		comiss	xmm0, xmm2
		.IF	!CARRY?
			;
			; ポリゴン裏面
			;
			mov	ebx, hRenderPoly
			ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
				mov	edx, nReflectionCount
			mov	ecx, [ebx].nVectorLightCount
				mov	eax, 1
			add	ecx, [ebx].nPointLightCount
				test	edx, edx
			.IF	!ZERO?
				imul	ecx, edx
					dec	edx
				imul	ecx, (SIZEOF E3D_GPU_RAY_HIT_ENTRY)
					mov	nReflectionCount, edx
					mov	eax, pRayHit
				lea	esi, [esi + ecx + (SIZEOF E3D_GPU_RAY_HIT_ENTRY)]
				mov	pRayHit, esi
					mov	[esi].pParent, eax
				jmp	Label_Start

			.ELSEIF	nAppendTracingCount > 0
				mov	eax, [esi].pParent
				add	esi, (SIZEOF E3D_GPU_RAY_HIT_ENTRY)
				mov	pRayHit, esi
				mov	[esi].pParent, eax
				dec	nAppendTracingCount
				jmp	Label_Start
			.ENDIF
			jmp	Label_Exit
			ASSUME	ebx:NOTHING
		.ENDIF
	.ENDIF
	;
	;	テクスチャピクセルサンプリング
	; ---------------------------------------------------------------------
	mov	eax, dwShadingFlags
	mov	edx, psfAttr
	.IF	eax & E3DSAF_TEXTURE_MAPPING
	.IF	eax & E3DSAF_TEXTURE_SMOOTH
		mulps		xmm3, xmmPacked256
		mov	edx, (E3D_SURFACE_ATTRIBUTE PTR [edx]).txmap.pTextureImage
		ASSUME	edx:PTR EGL_IMAGE_BUFF
		.IF	(edx != NULL) && \
				([edx].dwInfoSize == (SIZEOF EGL_IMAGE_BUFF))
			movd	mm6, [edx].dwBitsPerPixel
			movd	mm7, [edx].dwBytesPerLine
			cvtps2pi	mm0, xmm3
			.IF	eax & E3DSAF_TEXTURE_TILING
					movq		mm5, MMWORD PTR [edx].dwWidthMask
					psrld		mm6, 3
				movq		mm2, mm0
				psrad		mm0, 8
					packssdw	mm5, mm5
				packssdw	mm0, mm0
					punpcklwd	mm6, mm7
				paddw		mm0, mmxConstW_0_0_1_0
				movq		mm1, mm0
					punpckldq	mm6, mm6
				pand		mm0, mm5
				paddw		mm1, mmxConstW_0_1_0_1
				pmaddwd		mm0, mm6
				pand		mm1, mm5
				movd		mm7, [edx].ptrImageArray
				pmaddwd		mm1, mm6
				punpckldq	mm7, mm7
			.ELSE
					pcmpeqd		mm4, mm4
					movq		mm5, MMWORD PTR [edx].dwImageWidth
				movq		mm2, mm0
				psrad		mm0, 8
					paddd		mm5, mm4
				packssdw	mm0, mm0
					pxor		mm5, mm4
				paddw		mm0, mmxConstW_0_0_1_0
					psrld		mm6, 3
					packssdw	mm5, mm5
				movq		mm3, mm0
				movq		mm1, mm0
					punpcklwd	mm6, mm7
				paddusw		mm3, mm5
				psraw		mm0, 15
					punpckldq	mm6, mm6
				psubusw		mm3, mm5
				paddw		mm1, mmxConstW_0_1_0_1
				pandn		mm0, mm3
				;
				movq		mm3, mm1
				pmaddwd		mm0, mm6
				paddusw		mm3, mm5
				psraw		mm1, 15
				movd		mm7, [edx].ptrImageArray
				psubusw		mm3, mm5
				pandn		mm1, mm3
				punpckldq	mm7, mm7
				pmaddwd		mm1, mm6
			.ENDIF
			;
			paddd		mm0, mm7	; addr px00 : px01
			movd		eax, mm0
			psrlq		mm0, 32
			paddd		mm1, mm7	; addr px10 : px11
			movd		ecx, mm0
			;
			pand		mm2, mmxConstDW_FFH
			pxor		mm7, mm7
			movd		mm4, DWORD PTR [eax]
			movd		eax, mm1
			psrlq		mm1, 32
			movd		mm5, DWORD PTR [ecx]
			movd		ecx, mm1
			punpcklbw	mm4, mm7	; px00
			psrld		mm2, 1
			movd		mm0, DWORD PTR [eax]
			movd		mm1, DWORD PTR [ecx]
			punpcklbw	mm5, mm7	; px01
			punpcklbw	mm0, mm7	; px10
			punpcklbw	mm1, mm7	; px11
			;
			pshufw		mm3, mm2, 00000000B
			psubw		mm5, mm4
			psubw		mm1, mm0
			pmullw		mm5, mm3
			pmullw		mm1, mm3
				mov	ecx, 0FF000000H
				xor	eax, eax
			pshufw		mm2, mm2, 10101010B
			psraw		mm5, 7
			psraw		mm1, 7
			paddw		mm4, mm5
			paddw		mm0, mm1
			;
			psubw		mm0, mm4
			pmullw		mm0, mm2
				test	[edx].fdwFormatType, EIF_WITH_ALPHA
				cmovz	eax, ecx
			psraw		mm0, 7
				movd	mm1, eax
			paddw		mm0, mm4
			packuswb	mm0, mm0
			;
			por		mm0, mm1
			movq		mm1, mm0
			movd		ecx, mm0
			psrad		mm0, 31
			movq		mm2, mm0
			pand		mm0, mm1
			pslld		mm2, 24
			por		mm0, mm2
			;
			test	dwShadingFlags, E3DSAF_TEXTURE_TRIM
			movd	eax, mm0
			cmovz	eax, ecx
			mov	rgbaTexture.dwPixelCode, eax
		.ENDIF
	.ELSE
		cvtps2pi	mm4, xmm3
		mov	edx, (E3D_SURFACE_ATTRIBUTE PTR [edx]).txmap.pTextureImage
		ASSUME	edx:PTR EGL_IMAGE_BUFF
		.IF	(edx != NULL) && \
				([edx].dwInfoSize == (SIZEOF EGL_IMAGE_BUFF))
			movd	mm6, [edx].dwBitsPerPixel
			movd	mm7, [edx].dwBytesPerLine
			.IF	eax & E3DSAF_TEXTURE_TILING
				pand	mm4, MMWORD PTR [edx].dwWidthMask
					psrld	mm6, 3
				packssdw	mm4, mm4
					punpcklwd	mm6, mm7
				pmaddwd	mm4, mm6
			.ELSE
				pcmpeqd	mm1, mm1
				movq	mm0, MMWORD PTR [edx].dwImageWidth
				paddd	mm0, mm1
					packssdw	mm4, mm4
				pxor	mm0, mm1
					movq	mm5, mm4
				packssdw	mm0, mm0
					paddusw	mm5, mm0
					psraw	mm4, 15
					psubusw	mm5, mm0
						psrld	mm6, 3
				pandn	mm4, mm5
						punpcklwd	mm6, mm7
				pmaddwd	mm4, mm6
			.ENDIF
			movd	eax, mm4
			add	eax, [edx].ptrImageArray
			mov	ecx, 0FF000000H
			mov	eax, DWORD PTR [eax]
			or	ecx, eax
			test	[edx].fdwFormatType, EIF_WITH_ALPHA
			cmovz	eax, ecx
			mov	edx, 0FF000000H
			mov	ecx, eax
			sar	eax, 31
			and	edx, eax
			and	eax, ecx
			or	eax, edx
			test	dwShadingFlags, E3DSAF_TEXTURE_TRIM
			cmovz	eax, ecx
			mov	rgbaTexture.dwPixelCode, eax
		.ENDIF
	.ENDIF
	.ENDIF
	;
	;	表面属性パラメータ計算
	; ---------------------------------------------------------------------
	mov	ebx, hRenderPoly
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	mov	eax, dwShadingFlags
	test	eax, E3DSAF_SHADING_MASK
	jz	Label_NoShading			; シェーディング無し分岐
	;
	add	esi, (SIZEOF E3D_GPU_RAY_HIT_ENTRY)
	mov	pNextRayHit, esi
	mov	esi, psfAttr
	ASSUME	esi:PCE3D_SURFACE_ATTRIBUTE
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
		pshufw	mm2, mm2, 0
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
		neg	eax
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
	mov	eax, OFFSET SubFunc_BlendTextureDummy
	mov	pfnBeforeTexture, eax
	mov	pfnAfterTexture, eax
	.IF	dwShadingFlags & E3DSAF_TEXTURE_MAPPING
		mov	eax, OFFSET SubFunc_BlendTexture
		.IF	dwShadingFlags & E3DSAF_GOURAUD_SHADE
			mov	pfnAfterTexture, eax
		.ELSE
			mov	pfnBeforeTexture, eax
		.ENDIF
	.ENDIF
	;
	.IF	nReflectionCount > 0
		mov	eax, OFFSET SubFunc_ShadowHitTest
		mov	edx, OFFSET SubFunc_ShadowHitTestAlpha
		test	[ebx].rrtpRayParam.dwFlags, E3D_RAYTRACE_SHADOW_ALPHA
		cmovnz	eax, edx
		mov	pfnShadowing, eax
		;
		mov	eax, OFFSET SubFunc_Reflection
		mov	edx, OFFSET SubFunc_Refraction
		test	dwShadingFlags, E3DSAF_RAY_REFRACTING
		cmovnz	eax, edx
		mov	edx, OFFSET SubFunc_ReflectionDummy
		test	dwShadingFlags, (E3DSAF_RAY_REFLECTING OR E3DSAF_RAY_REFRACTING)
		cmovz	eax, edx
		mov	pfnReflection, eax
		;
		imul	eax, nReflectionCount, (SIZEOF E3D_GPU_RAY_HIT_ENTRY)
		mov	nRayHitLightEntryStepBytes, eax
	.ELSE
		and	dwShadingFlags, NOT E3DSAF_RAY_REFLECTING
		mov	pfnShadowing, OFFSET SubFunc_ShadowHitTestDummy
		mov	pfnReflection, OFFSET SubFunc_ReflectionDummy
		mov	nRayHitLightEntryStepBytes, 0
		;
		.IF	nAppendTracingCount > 0
			mov	eax, OFFSET SubFunc_ReflectionDummy
			mov	edx, OFFSET SubFunc_Refraction
			test	dwShadingFlags, E3DSAF_RAY_REFRACTING
			cmovnz	eax, edx
			mov	pfnReflection, eax
		.ENDIF
	.ENDIF

	;
	;	法線及び焦点ベクトルの正規化と内積の計算
	; ---------------------------------------------------------------------
	mov	esi, pRayHit
	ASSUME	esi:PTR E3D_GPU_RAY_HIT_ENTRY
	mov	eax, [esi].pParent
	movups	xmm4, vcNormal
	movups	xmm0, [esi].vHitPos
	movups	xmm1, (E3D_GPU_RAY_HIT_ENTRY PTR [eax]).vHitPos
		movaps	xmm7, xmm4
		movups	vHitPos, xmm0
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
				movups	xmm6, vTargetPlane
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
	movq	mm6, MMWORD PTR rgbAmbient
	pxor	mm7, mm7
;	movq	MMWORD PTR rgbMulColor, mm6
;	movq	MMWORD PTR rgbAddColor, mm7
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
		mov	dwSaveBrightness[0], eax
		movss	dwSaveBrightness[4], xmm6
		movq	MMWORD PTR rgbMulColor, mm6
		movq	MMWORD PTR rgbAddColor, mm7
		push	esi
		push	ecx
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
			; cosθ を計算
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
		mov	eax, pNextRayHit
		mov	ebx, hRenderPoly
		add	eax, nRayHitLightEntryStepBytes
		add	esi, (SIZEOF E3D_VECTOR_LIGHT_ENTRY)
		mov	pNextRayHit, eax
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
		movlps	xmm2, QWORD PTR [esi].vLight
		movss	xmm3, [esi].vLight.z
		movups	xmm0, vHitPos
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
		mov	dwSaveBrightness[0], eax
		movss	dwSaveBrightness[4], xmm6
		movq	MMWORD PTR rgbMulColor, mm6
		movq	MMWORD PTR rgbAddColor, mm7
		push	esi
		push	ecx
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
		mov	eax, pNextRayHit
		mov	ebx, hRenderPoly
		add	eax, nRayHitLightEntryStepBytes
		add	esi, (SIZEOF E3D_POINT_LIGHT_ENTRY)
		mov	pNextRayHit, eax
		dec	ecx
	.ENDW
	ASSUME	esi:NOTHING

	;
	;	拡散反射光・鏡面反射光適用
	; --------------------------------------------------------------------
	;
	; 発光成分(ADD) = 発光成分 * 拡散反射 + 鏡面反射
	; 透明度成分(MUL) = 透明度成分 * 拡散反射
	;
			movq	mm5, mm6
			pxor	mm3, mm3
	movd	mm0, rgbColorShade.rgbAdd.dwPixelCode
		movd	mm1, rgbColorShade.rgbMul.dwPixelCode
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
	mov	eax, pShadeColor
	packuswb	mm6, mm7
	movq	MMWORD PTR [eax], mm6
	ASSUME	ebx:NOTHING
	xor	eax, eax
Label_Exit:
	mov	esp, dwSaveESP
	ret


;
;	シェーディング無し処理
; ----------------------------------------------------------------------------
Label_NoShading:
	mov	edi, pShadeColor
	ASSUME	edi:PTR E3D_COLOR
	.IF	eax & E3DSAF_TEXTURE_MAPPING
		pxor		mm4, mm4
		pcmpeqd		mm2, mm2
		movd		mm6, rgbColorShade.rgbMul.dwPixelCode
		movd		mm7, rgbColorShade.rgbAdd.dwPixelCode
		punpcklbw	mm6, mm4
		punpcklbw	mm7, mm4
		;
		psubsw		mm6, mm2
		movd		mm0, rgbaTexture.dwPixelCode
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
		movq		MMWORD PTR [edi], mm3
		xor	eax, eax
		jmp	Label_Exit
	.ELSE
		movq	mm0, MMWORD PTR rgbColorShade
		movq	MMWORD PTR [edi], mm0
	.ENDIF
	ASSUME	edi:NOTHING
	;
	xor	eax, eax
	jmp	Label_Exit

;
;	透明度処理
; ----------------------------------------------------------------------------
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
	pcmpeqd		mm2, mm2
	pxor		mm1, mm1
	psubsw		mm6, mm2
	movd		mm0, DWORD PTR rgbaTexture
	punpcklbw	mm0, mm1
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


;
;	影当たり判定
; ----------------------------------------------------------------------------
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
	mov	edi, pNextRayHit
	mov	ebx, hRenderPoly
	ASSUME	edi:PTR E3D_GPU_RAY_HIT_ENTRY
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	;
	cmp	[edi].ppeMesh, 0
	jz	SubFunc_ShadowHitTestDummy
	;
	xorps		xmm4, xmm4
	movss		xmm3, [edi].rDistance
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
	mov	edi, pNextRayHit
	mov	ebx, hRenderPoly
	ASSUME	edi:PTR E3D_GPU_RAY_HIT_ENTRY
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	;
	movq	mm0, mmxConst4000H
	mov	eax, nReflectionCount
	movq	mmxShadowColor, mm0
	mov	nRayCounter, eax
	;
Label_ShadowHitTestAlpha_Loop:
	cmp	[edi].ppeMesh, 0
	jz	Label_ShadowHitTestAlpha_LoopNext
	;
	mov	eax, pRayHit
	mov	[edi].pParent, eax
	;
	INVOKE	eglRenderPoly@ShadeByGPURayTraceSSE ,
			ebx, ADDR rgbColorTemp, edi, 0, 0
	test	eax, eax
	jnz	Label_ShadowHitTestAlpha_LoopNext
	;
		movd	mm0, rgbColorTemp.rgbMul.dwPixelCode
		pxor	mm7, mm7
	movss		xmm0, [ebx].rRcpShadowDistance
	xorps		xmm7, xmm7
	movss		xmm1, [edi].rDistance
		punpcklbw	mm0, mm7
	maxss		xmm1, xmm7
	movss		xmm6, [ebx].rrtpRayParam.rShadowingDistance
		movq		mm1, mm0
		movd		mm2, DWORD PTR mmxShadowColor[0]
		movd		mm3, DWORD PTR mmxShadowColor[4]
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
	movlhps		xmm2, xmm3		; xmm2 = 陰色
	shufps		xmm6, xmm6, 0
			shufps	xmm0, xmm0, 11000000B
	mulps		xmm2, xmm6
		cvtpi2ps	xmm4, mm2
		cvtpi2ps	xmm5, mm3
	addps		xmm2, xmm1		; xmm2 = 陰色/256 * (rShadowingDistance - rDistance) + rDistance
		movlhps		xmm4, xmm5	; xmm4 = mmxShadowColor
	mulps		xmm0, xmm2		; xmm0 = (陰色/256 * (rShadowingDistance - rDistance) + rDistance) / rShadowingDistance
	mulps		xmm0, xmm4
	;
	cvtps2pi	mm0, xmm0
	movhlps		xmm0, xmm0
	cvtps2pi	mm1, xmm0
	packssdw	mm0, mm1
	movq		mmxShadowColor, mm0
	;
Label_ShadowHitTestAlpha_LoopNext:
	mov	eax, nRayCounter
	add	edi, (SIZEOF E3D_GPU_RAY_HIT_ENTRY)
	dec	eax
	mov	nRayCounter, eax
	jnz	Label_ShadowHitTestAlpha_Loop
	;
Label_ShadowHitTestAlpha_LoopExit:
	;
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
	;
	;	屈折処理
	; --------------------------------------------------------------------
	movq		mm0, mmxConst1
	pcmpgtw		mm0, mm6
	packsswb	mm0, mm0
	movd		eax, mm0
	test		eax, eax
	jnz		SubFunc_Reflection
	;
Label_RefractionContinue:
	;
	; 屈折シェーディング
	;
	mov	edi, pNextRayHit
	ASSUME	edi:PTR E3D_GPU_RAY_HIT_ENTRY
	ASSUME	ebx:PTR EGL_RENDER_POLYGON_BUF
	;
	cmp	[edi].ppeMesh, 0
	jz	SubFunc_Reflection
	;
	mov	edx, nReflectionCount
	mov	ecx, nAppendTracingCount
	.IF	edx != 0
		mov	eax, pRayHit
		dec	edx
		mov	[edi].pParent, eax
	.ELSE
		mov	eax, pRayHit
		dec	ecx
		mov	eax, (E3D_GPU_RAY_HIT_ENTRY PTR [eax]).pParent
		mov	[edi].pParent, eax
	.ENDIF
	;
	movq	MMWORD PTR rgbMulColor, mm6
	movq	MMWORD PTR rgbAddColor, mm7
	;
	INVOKE	eglRenderPoly@ShadeByGPURayTraceSSE ,
		ebx, ADDR rgbColorTemp, edi, edx, ecx
	;
	movq	mm6, MMWORD PTR rgbMulColor
	movq	mm7, MMWORD PTR rgbAddColor
	;
	test	eax, eax
	jnz	SubFunc_Reflection
	;
	movd	mm0, rgbColorTemp.rgbAdd.dwPixelCode
	movq	mm5, mm6
	pxor	mm6, mm6
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
	; 次のエントリまでポインタを進める
	;
	mov	edx, [ebx].nVectorLightCount
	mov	ecx, nReflectionCount
	add	edx, [ebx].nPointLightCount
	dec	ecx
	;
	E3D_GPU_RAY_HIT_ENTRY@CountOfBlock	edx, ecx
	;
	add	eax, nAppendTracingCount
	imul	eax, (SIZEOF E3D_GPU_RAY_HIT_ENTRY)
	;
	mov	edi, pNextRayHit
	ASSUME	edi:PTR E3D_GPU_RAY_HIT_ENTRY
	add	edi, eax
	;
	movq	MMWORD PTR rgbMulColor, mm6
	movq	MMWORD PTR rgbAddColor, mm7
	;
	mov	eax, pRayHit
	cmp	[edi].ppeMesh, NULL
	jz	Label_ReflectionExitNoReflection
	;
	; 反射シェーディング
	;
	mov	edx, nReflectionCount
	mov	[edi].pParent, eax
	dec	edx
	;
	INVOKE	eglRenderPoly@ShadeByGPURayTraceSSE ,
			ebx, ADDR rgbColorTemp, edi, edx, 0
	test	eax, eax
	jnz	Label_ReflectionExitNoReflection
	;
	movd	mm2, nReflection
	movd	mm5, rgbColorTemp.rgbMul.dwPixelCode
	pcmpeqw	mm4, mm4
	pxor	mm6, mm6
		movd	mm0, rgbColorTemp.rgbAdd.dwPixelCode
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
	movq	mm6, MMWORD PTR rgbMulColor
	movq	mm7, MMWORD PTR rgbAddColor
	BYTE	0C3H	; ret


eglRenderPoly@ShadeByGPURayTraceSSE	ENDP

CodeSeg	ENDS

ENDIF

	END
