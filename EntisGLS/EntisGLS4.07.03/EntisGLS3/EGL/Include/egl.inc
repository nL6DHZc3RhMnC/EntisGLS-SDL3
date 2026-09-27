
; ----------------------------------------------------------------------------
;                          Entis Graphic Library
; ----------------------------------------------------------------------------
;   Copyright (c) 2002-2012 Leshade Entis, Entis-soft. Al rights reserved.
; ----------------------------------------------------------------------------


IFNDEF	__EGL_INC__
__EGL_INC__	EQU	1

INCLUDE	esl.inc

IF	@Version LE 615
MMWORD	TYPEDEF	QWORD
XMMWORD_PTR	TEXTEQU	<>
ELSE
XMMWORD_PTR	TEXTEQU	<XMMWORD PTR>
ENDIF

IFNDEF	NULL
NULL	EQU	0
ENDIF

IFNDEF	PVOID
PVOID	TYPEDEF	PTR
ENDIF

IFNDEF	HANDLE
HANDLE	TYPEDEF	PTR
ENDIF

IFNDEF	HDC
HDC	TYPEDEF	PTR
ENDIF

IFNDEF	HBITMAP
HBITMAP	TYPEDEF	PTR
ENDIF

CreateEventA		PROTO	NEAR32 STDCALL,
	lpEventAttributes:PTR, bManualReset:DWORD,
	bInitialState:DWORD, lpName:PTR BYTE
CloseHandle		PROTO	NEAR32 STDCALL, hObject:HANDLE
SetEvent		PROTO	NEAR32 STDCALL, hObject:HANDLE
ResetEvent		PROTO	NEAR32 STDCALL, hObject:HANDLE
WaitForSingleObject	PROTO	NEAR32 STDCALL,
	hHandle:HANDLE, dwMilliseconds:DWORD


; movsd を MASM6.15 でアセンブルできないみたい…
_movsd	MACRO	dst_rm:REQ, src_rm:REQ
	LOCAL	LABEL_BEGIN, LABEL_NEXT
	IF	@Version LE 615
LABEL_BEGIN:
		IF	(OPATTR(dst_rm)) AND 00010000B	 ; register
			IF	(OPATTR(src_rm)) AND 00010000B	 ; register
				movss	dst_rm, src_rm
			ELSE
				movss	dst_rm, REAL4 PTR (src_rm)
			ENDIF
		ELSE
			movss	REAL4 PTR (dst_rm), src_rm
		ENDIF
LABEL_NEXT:
		ORG	(OFFSET LABEL_BEGIN)
		BYTE	0F2H
		ORG	(OFFSET LABEL_NEXT)
	ELSE
		movsd	dst_rm, src_rm
	ENDIF
ENDM



;
;	パレット共用体
; ----------------------------------------------------------------------------

EGL_PALETTE	UNION
dwPixelCode	DWORD	?		; パレット番号／ピクセルコード
rZOrder32	REAL4	?		; Z 値
STRUCT	rgb				; RGB24 フォーマット
	Blue		BYTE	?
	Green		BYTE	?
	Red		BYTE	?
	Reserved	BYTE	?
ENDS
STRUCT	rgba				; RGBA32 フォーマット
	Blue		BYTE	?
	Green		BYTE	?
	Red		BYTE	?
	Alpha		BYTE	?
ENDS
STRUCT	yuv				; YUV24 フォーマット
	Y		BYTE	?
	U		BYTE	?
	V		BYTE	?
	Reserved	BYTE	?
ENDS
STRUCT	hsb				; HSB24 フォーマット
	Brightness	BYTE	?
	Saturation	BYTE	?
	Hue		BYTE	?
	Reserved	BYTE	?
ENDS
EGL_PALETTE	ENDS
PEGL_PALETTE	TYPEDEF	PTR EGL_PALETTE
PCEGL_PALETTE	TYPEDEF	PTR EGL_PALETTE


;
;	座標・サイズ・矩形
; ----------------------------------------------------------------------------

EGL_POINT	STRUCT
x		SDWORD	?
y		SDWORD	?
EGL_POINT	ENDS
PEGL_POINT	TYPEDEF	PTR EGL_POINT
PCEGL_POINT	TYPEDEF	PTR EGL_POINT

EGL_SIZE	STRUCT
w		SDWORD	?
h		SDWORD	?
EGL_SIZE	ENDS
PEGL_SIZE	TYPEDEF	PTR EGL_SIZE
PCEGL_SIZE	TYPEDEF	PTR EGL_SIZE

EGL_RECT	STRUCT
left		SDWORD	?
top		SDWORD	?
right		SDWORD	?
bottom		SDWORD	?
EGL_RECT	ENDS
PEGL_RECT	TYPEDEF	PTR EGL_RECT
PCEGL_RECT	TYPEDEF	PTR EGL_RECT

EGL_IMAGE_RECT	STRUCT
x		SDWORD	?
y		SDWORD	?
w		SDWORD	?
h		SDWORD	?
EGL_IMAGE_RECT	ENDS
PEGL_IMAGE_RECT		TYPEDEF	PTR EGL_IMAGE_RECT
PCEGL_IMAGE_RECT	TYPEDEF	PTR EGL_IMAGE_RECT

EGL_IMAGE_AXES	STRUCT
STRUCT	xAxis
	x	REAL4	?
	y	REAL4	?
ENDS
STRUCT	yAxis
	x	REAL4	?
	y	REAL4	?
ENDS
EGL_IMAGE_AXES	ENDS
PEGL_IMAGE_AXES		TYPEDEF	PTR EGL_IMAGE_AXES
PCEGL_IMAGE_AXES	TYPEDEF	PTR EGL_IMAGE_AXES


;
;	画像情報構造体
; ----------------------------------------------------------------------------

PROTO_IMAGE_BUFFER_Release	TYPEDEF	PROTO	NEAR32 C,
	pImageInf:PTR, pInterface:PTR
PFN_IMAGE_BUFFER_Release	TYPEDEF	PTR PROTO_IMAGE_BUFFER_Release

PROTO_IMAGE_BUFFER_UpdateBuffer	TYPEDEF	PROTO	NEAR32 C,
	pImageInf:PTR, pInterface:PTR, pRect:PCEGL_RECT
PFN_IMAGE_BUFFER_UpdateBuffer	TYPEDEF	PTR PROTO_IMAGE_BUFFER_UpdateBuffer

PROTO_IMAGE_BUFFER_CommitBuffer	TYPEDEF	PROTO	NEAR32 C,
	pImageInf:PTR, pInterface:PTR
PFN_IMAGE_BUFFER_CommitBuffer	TYPEDEF	PTR PROTO_IMAGE_BUFFER_CommitBuffer

PROTO_IMAGE_BUFFER_ReflectBuffer	TYPEDEF	PROTO	NEAR32 C,
	pImageInf:PTR, pInterface:PTR, pRect:PCEGL_RECT
PFN_IMAGE_BUFFER_ReflectBuffer	TYPEDEF	PTR PROTO_IMAGE_BUFFER_ReflectBuffer

EGL_IMAGE_BUFFER_INTERFACE	STRUCT
dwType				DWORD	?
ptrObject			PVOID	?
pNextInterface			PVOID	?
pfnRelease			PFN_IMAGE_BUFFER_Release	?
pfnUpdateBuffer			PFN_IMAGE_BUFFER_UpdateBuffer	?
pfnCommitBuffer			PFN_IMAGE_BUFFER_CommitBuffer	?
pfnReflectBuffer		PFN_IMAGE_BUFFER_ReflectBuffer	?
EGL_IMAGE_BUFFER_INTERFACE	ENDS
PEGL_IMAGE_BUFFER_INTERFACE	TYPEDEF	PTR EGL_IMAGE_BUFFER_INTERFACE

FUNC_UPDATE_IMAGE_BUFFER	TYPEDEF	PROTO	NEAR32 C,
	pImageInf:PTR, nAction:DWORD, pRect:PCEGL_RECT
PFUNC_UPDATE_IMAGE_BUFFER	TYPEDEF	PTR FUNC_UPDATE_IMAGE_BUFFER

EGL_IBA_RELEASE_IMAGE_BUFFER	EQU	-1
EGL_IBA_UPDATE_IMAGE_BUFFER	EQU	0
EGL_IBA_COMMIT_IMAGE_BUFFER	EQU	1
EGL_IBA_REFLECT_IMAGE_BUFFER	EQU	2

EGL_IMAGE_INFO	STRUCT
dwInfoSize	DWORD		?
fdwFormatType	DWORD		?
ptrOffsetPixel	DWORD		?
ptrImageArray	PVOID		?
pPaletteEntries	PEGL_PALETTE	?
dwPaletteCount	DWORD		?
dwImageWidth	DWORD		?
dwImageHeight	DWORD		?
dwBitsPerPixel	DWORD		?
dwBytesPerLine	SDWORD		?
dwSizeOfImage	SDWORD		?
dwClippedPixel	DWORD		?
EGL_IMAGE_INFO	ENDS

PEGL_IMAGE_INFO		TYPEDEF	PTR EGL_IMAGE_INFO
PCEGL_IMAGE_INFO	TYPEDEF	PTR EGL_IMAGE_INFO ;

EIF_RGB_BITMAP		EQU	00000001H
EIF_RGBA_BITMAP		EQU	04000001H
EIF_GRAY_BITMAP		EQU	00000002H
EIF_YUV_BITMAP		EQU	00000004H
EIF_HSB_BITMAP		EQU	00000006H
EIF_Z_BUFFER_R4		EQU	00002005H
EIF_TYPE_MASK		EQU	00FFFFFFH
EIF_WITH_PALETTE	EQU	01000000H
EIF_WITH_CLIPPING	EQU	02000000H
EIF_WITH_ALPHA		EQU	04000000H
EIF_SIDE_BY_SIDE	EQU	10000000H

EGL_IMAGE_BUFF	STRUCT
dwInfoSize	DWORD		?
fdwFormatType	DWORD		?
ptrOffsetPixel	DWORD		?
ptrImageArray	PVOID		?
pPaletteEntries	PEGL_PALETTE	?
dwPaletteCount	DWORD		?
dwImageWidth	DWORD		?
dwImageHeight	DWORD		?
dwBitsPerPixel	DWORD		?
dwBytesPerLine	SDWORD		?
dwSizeOfImage	SDWORD		?
dwClippedPixel	DWORD		?
pInterface	PEGL_IMAGE_BUFFER_INTERFACE	?
dwFlags		DWORD		?
hDC		HDC		?
hBitmap		HBITMAP		?
pLineAddrEntry	PVOID		?
dwWidthMask	DWORD		?
dwHeightMask	DWORD		?
EGL_IMAGE_BUFF	ENDS


;
;	n 次元ベクトル
; ----------------------------------------------------------------------------

E3D_VECTOR_2D	STRUCT
x		REAL4	?
y		REAL4	?
E3D_VECTOR_2D	ENDS
PE3D_VECTOR_2D	TYPEDEF	PTR E3D_VECTOR_2D
PCE3D_VECTOR_2D	TYPEDEF	PTR E3D_VECTOR_2D

E3D_VECTOR_2D_DF	STRUCT
x			REAL8	?
y			REAL8	?
E3D_VECTOR_2D_DF	ENDS

E3D_VECTOR	STRUCT
x		REAL4	?
y		REAL4	?
z		REAL4	?
E3D_VECTOR	ENDS
PE3D_VECTOR	TYPEDEF	PTR E3D_VECTOR
PCE3D_VECTOR	TYPEDEF	PTR E3D_VECTOR

E3D_VECTOR_DF	STRUCT
x		REAL8	?
y		REAL8	?
z		REAL8	?
E3D_VECTOR_DF	ENDS

E3D_VECTOR4	STRUCT
x		REAL4	?
y		REAL4	?
z		REAL4	?
d		REAL4	?
E3D_VECTOR4	ENDS
PE3D_VECTOR4	TYPEDEF	PTR E3D_VECTOR4
PCE3D_VECTOR4	TYPEDEF	PTR E3D_VECTOR4


;
;	3x3 変換行列
;	（アラインのため 3x4 ）
; ----------------------------------------------------------------------------

E3D_REV_MATRIX	STRUCT
matrix		REAL4	3 DUP( 4 DUP(?) )
E3D_REV_MATRIX	ENDS
PE3D_REV_MATRIX		TYPEDEF	PTR E3D_REV_MATRIX
PCE3D_REV_MATRIX	TYPEDEF	PTR E3D_REV_MATRIX


;
;	グローバル変数
; ----------------------------------------------------------------------------

EXTERNDEF	C EGL_hImageHeap:HESLHEAP


;
;	画像バッファ管理
; ----------------------------------------------------------------------------

eglCreateImageBuffer	PROTO	NEAR32 C,
	fdwFormat:DWORD, dwWidth:DWORD, dwHeight:DWORD,
	dwBitsPerPixel:DWORD, dwFlags:DWORD
eglCreateTextureInfo	PROTO	NEAR32 C,
	pImageInf:PCEGL_IMAGE_INFO, pClipRect:PCEGL_RECT, dwFlags:DWORD
eglDuplicateImageBuffer	PROTO	NEAR32 C,
	pImageInf:PCEGL_IMAGE_INFO, dwFlags:DWORD
eglGetStereoLeftImageBuffer	PROTO	NEAR32 C,
	pImageInf:PCEGL_IMAGE_INFO, pLeftImage:PEGL_IMAGE_INFO
eglAddImageBufferRef	PROTO	NEAR32 C,
	pImageInf:PEGL_IMAGE_INFO
eglDeleteImageBuffer	PROTO	NEAR32 C,
	pImageInf:PEGL_IMAGE_INFO
eglDrawToDC		PROTO	NEAR32 C,
	hDstDC:HDC, pImageInf:PEGL_IMAGE_INFO,
	nPosX:SDWORD, nPosY:SDWORD,
	pSizeToDraw:PCEGL_SIZE, pViewRect:PCEGL_RECT
eglGetDC		PROTO	NEAR32 C,
	pImageInf:PEGL_IMAGE_INFO
eglFillImage		PROTO	NEAR32 C,
	pImageInf:PEGL_IMAGE_INFO, colorFill:EGL_PALETTE

EGL_DRAW_RADIAN		EQU	0001H

EGL_IMAGE_HAS_DC	EQU	0001H
EGL_IMAGE_NO_DUP	EQU	0010H


;
;	基礎算術関数
; ----------------------------------------------------------------------------
PROTO_eglNegateVector		TYPEDEF	PROTO NEAR32 C, pv:PE3D_VECTOR
PROTO_eglAddVector		TYPEDEF	PROTO NEAR32 C,
			pv1:PE3D_VECTOR, pv2:PCE3D_VECTOR
PROTO_eglSubVector		TYPEDEF	PROTO NEAR32 C,
			pv1:PE3D_VECTOR, pv2:PCE3D_VECTOR
PROTO_eglMultipleVector		TYPEDEF	PROTO NEAR32 C, pv:PE3D_VECTOR, r:REAL4
PROTO_eglDivideVector		TYPEDEF	PROTO NEAR32 C, pv:PE3D_VECTOR, r:REAL4
PROTO_eglAbsoluteVector		TYPEDEF	PROTO NEAR32 C,
			pabs:PTR REAL4, pv:PCE3D_VECTOR
PROTO_eglVectorExteriorProduct	TYPEDEF	PROTO NEAR32 C,
			pv1:PE3D_VECTOR, pv2:PCE3D_VECTOR
PROTO_eglVectorInnerProduct	TYPEDEF	PROTO NEAR32 C,
			prs:PTR REAL4, pv1:PCE3D_VECTOR, pv2:PCE3D_VECTOR
PROTO_eglVectorRoundTo1		TYPEDEF	PROTO NEAR32 C, pv:PE3D_VECTOR
PROTO_eglMatrixNegate		TYPEDEF	PROTO NEAR32 C, matrix:PE3D_REV_MATRIX
PROTO_eglMatrixAdd		TYPEDEF	PROTO NEAR32 C,
			matDst:PE3D_REV_MATRIX, matSrc:PCE3D_REV_MATRIX
PROTO_eglMatrixSub		TYPEDEF	PROTO NEAR32 C,
			matDst:PE3D_REV_MATRIX, matSrc:PCE3D_REV_MATRIX
PROTO_eglMatrixMultiple		TYPEDEF	PROTO NEAR32 C,
			matDst:PE3D_REV_MATRIX, r:REAL4
PROTO_eglMatrixDeterminant		TYPEDEF	PROTO NEAR32 C,
			pDet:PTR REAL32, matrix:PCE3D_REV_MATRIX
PROTO_eglMatrixInverse		TYPEDEF	PROTO NEAR32 C,
			matDst:PE3D_REV_MATRIX, matSrc:PCE3D_REV_MATRIX
PROTO_eglMatrixRevolveOnX	TYPEDEF	PROTO NEAR32 C,
			matrix:PE3D_REV_MATRIX, rSin:REAL4, rCos:REAL4
PROTO_eglMatrixRevolveOnY	TYPEDEF	PROTO NEAR32 C,
			matrix:PE3D_REV_MATRIX, rSin:REAL4, rCos:REAL4
PROTO_eglMatrixRevolveOnZ	TYPEDEF	PROTO NEAR32 C,
			matrix:PE3D_REV_MATRIX, rSin:REAL4, rCos:REAL4
PROTO_eglMatrixRevolveByAngleOn	TYPEDEF	PROTO NEAR32 C,
			matrix:PE3D_REV_MATRIX, pAngle:PCE3D_VECTOR
PROTO_eglMatrixRevolveForAngle	TYPEDEF	PROTO NEAR32 C,
			matrix:PE3D_REV_MATRIX, pAngle:PCE3D_VECTOR
PROTO_eglMatrixMagnifyByVector	TYPEDEF	PROTO NEAR32 C,
			matrix:PE3D_REV_MATRIX, pv:PCE3D_VECTOR
PROTO_eglMatrixRevolve		TYPEDEF	PROTO NEAR32 C,
			matrix:PCE3D_REV_MATRIX, matDst:PE3D_REV_MATRIX
PROTO_eglMatrixRevolveBy	TYPEDEF	PROTO NEAR32 C,
			matDst:PE3D_REV_MATRIX, matSrc:PCE3D_REV_MATRIX
PROTO_eglMatrixRevolveVector	TYPEDEF	PROTO NEAR32 C,
			matrix:PCE3D_REV_MATRIX, pv:PE3D_VECTOR
PROTO_eglMatrixRevolveVectors	TYPEDEF	PROTO NEAR32 C,
			matrix:PCE3D_REV_MATRIX, pvDst:PE3D_VECTOR4,
			pvSrc:PCE3D_VECTOR4, pvOrigin:PE3D_VECTOR, nCount:DWORD
PROTO_eglGetMinVector		TYPEDEF	PROTO NEAR32 C,
			pvMin:PE3D_VECTOR4, pvList:PCE3D_VECTOR, nCount:DWORD
PROTO_eglGetMaxVector		TYPEDEF	PROTO NEAR32 C,
			pvMax:PE3D_VECTOR4, pvList:PCE3D_VECTOR, nCount:DWORD

PFN_eglNegateVector		TYPEDEF	PTR PROTO_eglNegateVector
PFN_eglAddVector		TYPEDEF	PTR PROTO_eglAddVector
PFN_eglSubVector		TYPEDEF	PTR PROTO_eglSubVector
PFN_eglMultipleVector		TYPEDEF	PTR PROTO_eglMultipleVector
PFN_eglDivideVector		TYPEDEF	PTR PROTO_eglDivideVector
PFN_eglAbsoluteVector		TYPEDEF	PTR PROTO_eglAbsoluteVector
PFN_eglVectorExteriorProduct	TYPEDEF	PTR PROTO_eglVectorExteriorProduct
PFN_eglVectorInnerProduct	TYPEDEF	PTR PROTO_eglVectorInnerProduct
PFN_eglVectorRoundTo1		TYPEDEF	PTR PROTO_eglVectorRoundTo1
PFN_eglMatrixNegate		TYPEDEF	PTR PROTO_eglMatrixNegate
PFN_eglMatrixAdd		TYPEDEF	PTR PROTO_eglMatrixAdd
PFN_eglMatrixSub		TYPEDEF PTR PROTO_eglMatrixSub
PFN_eglMatrixMultiple		TYPEDEF	PTR PROTO_eglMatrixMultiple
PFN_eglMatrixDeterminant	TYPEDEF PTR PROTO_eglMatrixDeterminant
PFN_eglMatrixInverse		TYPEDEF	PTR PROTO_eglMatrixInverse
PFN_eglMatrixRevolveOnX		TYPEDEF	PTR PROTO_eglMatrixRevolveOnX
PFN_eglMatrixRevolveOnY		TYPEDEF	PTR PROTO_eglMatrixRevolveOnY
PFN_eglMatrixRevolveOnZ		TYPEDEF	PTR PROTO_eglMatrixRevolveOnZ
PFN_eglMatrixRevolveByAngleOn	TYPEDEF	PTR PROTO_eglMatrixRevolveByAngleOn
PFN_eglMatrixRevolveForAngle	TYPEDEF	PTR PROTO_eglMatrixRevolveForAngle
PFN_eglMatrixMagnifyByVector	TYPEDEF	PTR PROTO_eglMatrixMagnifyByVector
PFN_eglMatrixRevolve		TYPEDEF	PTR PROTO_eglMatrixRevolve
PFN_eglMatrixRevolveBy		TYPEDEF	PTR PROTO_eglMatrixRevolveBy
PFN_eglMatrixRevolveVector	TYPEDEF	PTR PROTO_eglMatrixRevolveVector
PFN_eglMatrixRevolveVectors	TYPEDEF	PTR PROTO_eglMatrixRevolveVectors
PFN_eglGetMinVector		TYPEDEF	PTR PROTO_eglGetMinVector
PFN_eglGetMaxVector		TYPEDEF	PTR PROTO_eglGetMaxVector

EXTERNDEF	C eglNegateVector:PFN_eglNegateVector
EXTERNDEF	C eglAddVector:PFN_eglAddVector
EXTERNDEF	C eglSubVector:PFN_eglSubVector
EXTERNDEF	C eglMultipleVector:PFN_eglMultipleVector
EXTERNDEF	C eglDivideVector:PFN_eglDivideVector
EXTERNDEF	C eglAbsoluteVector:PFN_eglAbsoluteVector
EXTERNDEF	C eglVectorExteriorProduct:PFN_eglVectorExteriorProduct
EXTERNDEF	C eglVectorInnerProduct:PFN_eglVectorInnerProduct
EXTERNDEF	C eglVectorRoundTo1:PFN_eglVectorRoundTo1
EXTERNDEF	C eglMatrixNegate:PFN_eglMatrixNegate
EXTERNDEF	C eglMatrixAdd:PFN_eglMatrixAdd
EXTERNDEF	C eglMatrixSub:PFN_eglMatrixSub
EXTERNDEF	C eglMatrixMultiple:PFN_eglMatrixMultiple
EXTERNDEF	C eglMatrixDeterminant:PFN_eglMatrixDeterminant
EXTERNDEF	C eglMatrixInverse:PFN_eglMatrixInverse
EXTERNDEF	C eglMatrixRevolveOnX:PFN_eglMatrixRevolveOnX
EXTERNDEF	C eglMatrixRevolveOnY:PFN_eglMatrixRevolveOnY
EXTERNDEF	C eglMatrixRevolveOnZ:PFN_eglMatrixRevolveOnZ
EXTERNDEF	C eglMatrixRevolveByAngleOn:PFN_eglMatrixRevolveByAngleOn
EXTERNDEF	C eglMatrixRevolveForAngle:PFN_eglMatrixRevolveForAngle
EXTERNDEF	C eglMatrixMagnifyByVector:PFN_eglMatrixMagnifyByVector
EXTERNDEF	C eglMatrixRevolve:PFN_eglMatrixRevolve
EXTERNDEF	C eglMatrixRevolveBy:PFN_eglMatrixRevolveBy
EXTERNDEF	C eglMatrixRevolveVector:PFN_eglMatrixRevolveVector
EXTERNDEF	C eglMatrixRevolveVectors:PFN_eglMatrixRevolveVectors
EXTERNDEF	C eglGetMinVector:PFN_eglGetMinVector
EXTERNDEF	C eglGetMaxVector:PFN_eglGetMaxVector

eglInitializeMathFunctions	PROTO	NEAR32 C
eglNegateVector_486		PROTO NEAR32 C, pv:PE3D_VECTOR
eglAddVector_486		PROTO NEAR32 C,
			pv1:PE3D_VECTOR, pv2:PCE3D_VECTOR
eglSubVector_486		PROTO NEAR32 C,
			pv1:PE3D_VECTOR, pv2:PCE3D_VECTOR
eglMultipleVector_486		PROTO NEAR32 C, pv:PE3D_VECTOR, r:REAL4
eglDivideVector_486		PROTO NEAR32 C, pv:PE3D_VECTOR, r:REAL4
eglAbsoluteVector_486		PROTO NEAR32 C,
			pabs:PTR REAL4, pv:PCE3D_VECTOR
eglVectorExteriorProduct_486	PROTO NEAR32 C,
			pv1:PE3D_VECTOR, pv2:PCE3D_VECTOR
eglVectorInnerProduct_486	PROTO NEAR32 C,
			prs:PTR REAL4, pv1:PCE3D_VECTOR, pv2:PCE3D_VECTOR
eglVectorRoundTo1_486		PROTO NEAR32 C, pv:PE3D_VECTOR
eglMatrixNegate_486		PROTO NEAR32 C, matrix:PE3D_REV_MATRIX
eglMatrixAdd_486		PROTO NEAR32 C,
			matDst:PE3D_REV_MATRIX, matSrc:PCE3D_REV_MATRIX
eglMatrixSub_486		PROTO NEAR32 C,
			matDst:PE3D_REV_MATRIX, matSrc:PCE3D_REV_MATRIX
eglMatrixMultiple_486		PROTO NEAR32 C,
			matDst:PE3D_REV_MATRIX, r:REAL4
eglMatrixDeterminant_486	PROTO NEAR32 C,
			pDet:PTR REAL32, matrix:PCE3D_REV_MATRIX
eglMatrixInverse_486		PROTO NEAR32 C,
			matDst:PE3D_REV_MATRIX, matSrc:PCE3D_REV_MATRIX
eglMatrixRevolveOnX_486		PROTO NEAR32 C,
			matrix:PE3D_REV_MATRIX, rSin:REAL4, rCos:REAL4
eglMatrixRevolveOnY_486		PROTO NEAR32 C,
			matrix:PE3D_REV_MATRIX, rSin:REAL4, rCos:REAL4
eglMatrixRevolveOnZ_486		PROTO NEAR32 C,
			matrix:PE3D_REV_MATRIX, rSin:REAL4, rCos:REAL4
eglMatrixRevolveByAngleOn_486	PROTO NEAR32 C,
			matrix:PE3D_REV_MATRIX, pAngle:PCE3D_VECTOR
eglMatrixRevolveForAngle_486	PROTO NEAR32 C,
			matrix:PE3D_REV_MATRIX, pAngle:PCE3D_VECTOR
eglMatrixMagnifyByVector_486	PROTO NEAR32 C,
			matrix:PE3D_REV_MATRIX, pv:PCE3D_VECTOR
eglMatrixRevolve_486		PROTO NEAR32 C,
			matrix:PCE3D_REV_MATRIX, matDst:PE3D_REV_MATRIX
eglMatrixRevolveBy_486		PROTO NEAR32 C,
			matDst:PE3D_REV_MATRIX, matSrc:PCE3D_REV_MATRIX
eglMatrixRevolveVector_486	PROTO NEAR32 C,
			matrix:PCE3D_REV_MATRIX, pv:PE3D_VECTOR
eglMatrixRevolveVectors_486	PROTO NEAR32 C,
			matrix:PCE3D_REV_MATRIX, pvDst:PE3D_VECTOR4,
			pvSrc:PCE3D_VECTOR4, pvOrigin:PE3D_VECTOR, nCount:DWORD
eglGetMinVector_486		PROTO NEAR32 C,
			pvMin:PE3D_VECTOR4, pvList:PCE3D_VECTOR, nCount:DWORD
eglGetMaxVector_486		PROTO NEAR32 C,
			pvMax:PE3D_VECTOR4, pvList:PCE3D_VECTOR, nCount:DWORD

eglAddVector_SSE		PROTO NEAR32 C,
			pv1:PE3D_VECTOR, pv2:PCE3D_VECTOR
eglSubVector_SSE		PROTO NEAR32 C,
			pv1:PE3D_VECTOR, pv2:PCE3D_VECTOR
eglMultipleVector_SSE		PROTO NEAR32 C, pv:PE3D_VECTOR, r:REAL4
eglDivideVector_SSE		PROTO NEAR32 C, pv:PE3D_VECTOR, r:REAL4
eglAbsoluteVector_SSE		PROTO NEAR32 C,
			pabs:PTR REAL4, pv:PCE3D_VECTOR
eglVectorExteriorProduct_SSE	PROTO NEAR32 C,
			pv1:PE3D_VECTOR, pv2:PCE3D_VECTOR
eglVectorInnerProduct_SSE	PROTO NEAR32 C,
			prs:PTR REAL4, pv1:PCE3D_VECTOR, pv2:PCE3D_VECTOR
eglVectorRoundTo1_SSE		PROTO NEAR32 C, pv:PE3D_VECTOR
eglMatrixAdd_SSE		PROTO NEAR32 C,
			matDst:PE3D_REV_MATRIX, matSrc:PCE3D_REV_MATRIX
eglMatrixSub_SSE		PROTO NEAR32 C,
			matDst:PE3D_REV_MATRIX, matSrc:PCE3D_REV_MATRIX
eglMatrixMultiple_SSE		PROTO NEAR32 C,
			matDst:PE3D_REV_MATRIX, r:REAL4
eglMatrixDeterminant_SSE	PROTO NEAR32 C,
			pDet:PTR REAL32, matrix:PCE3D_REV_MATRIX
eglMatrixInverse_SSE		PROTO NEAR32 C,
			matDst:PE3D_REV_MATRIX, matSrc:PCE3D_REV_MATRIX
eglMatrixRevolveOnX_SSE		PROTO NEAR32 C,
			matrix:PE3D_REV_MATRIX, rSin:REAL4, rCos:REAL4
eglMatrixRevolveOnY_SSE		PROTO NEAR32 C,
			matrix:PE3D_REV_MATRIX, rSin:REAL4, rCos:REAL4
eglMatrixRevolveOnZ_SSE		PROTO NEAR32 C,
			matrix:PE3D_REV_MATRIX, rSin:REAL4, rCos:REAL4
eglMatrixRevolveByAngleOn_SSE	PROTO NEAR32 C,
			matrix:PE3D_REV_MATRIX, pAngle:PCE3D_VECTOR
eglMatrixRevolveForAngle_SSE	PROTO NEAR32 C,
			matrix:PE3D_REV_MATRIX, pAngle:PCE3D_VECTOR
eglMatrixMagnifyByVector_SSE	PROTO NEAR32 C,
			matrix:PE3D_REV_MATRIX, pv:PCE3D_VECTOR
eglMatrixRevolve_SSE		PROTO NEAR32 C,
			matrix:PCE3D_REV_MATRIX, matDst:PE3D_REV_MATRIX
eglMatrixRevolveBy_SSE		PROTO NEAR32 C,
			matDst:PE3D_REV_MATRIX, matSrc:PCE3D_REV_MATRIX
eglMatrixRevolveVector_SSE	PROTO NEAR32 C,
			matrix:PCE3D_REV_MATRIX, pv:PE3D_VECTOR
eglMatrixRevolveVectors_SSE	PROTO NEAR32 C,
			matrix:PCE3D_REV_MATRIX, pvDst:PE3D_VECTOR4,
			pvSrc:PCE3D_VECTOR4, pvOrigin:PE3D_VECTOR, nCount:DWORD
eglGetMinVector_SSE		PROTO NEAR32 C,
			pvMin:PE3D_VECTOR4, pvList:PCE3D_VECTOR, nCount:DWORD
eglGetMaxVector_SSE		PROTO NEAR32 C,
			pvMax:PE3D_VECTOR4, pvList:PCE3D_VECTOR, nCount:DWORD



;
;	画像情報
; ----------------------------------------------------------------------------

eglGetClippedImageInfo		PROTO	NEAR32 C,
	pClippedImage:PEGL_IMAGE_INFO,
	pOriginalImage:PCEGL_IMAGE_INFO,
	pClippingRect:PCEGL_IMAGE_RECT
eglGetOverlappedRectangle	PROTO	NEAR32 C,
	pImageRect:PEGL_IMAGE_RECT,
	pDstViewRect:PCEGL_RECT, pSrcViewRect:PCEGL_RECT,
	pSrcViewOffset:PCEGL_POINT, pSrcViewMaxSize:PCEGL_SIZE
eglGetRevolvedAxes		PROTO	NEAR32 C,
	pImageAxes:PEGL_IMAGE_AXES,
	pBasePosition:PCEGL_POINT, pCenterOffset:PCEGL_POINT,
	rHorzRate:REAL4, rVertRate:REAL4,
	rRevAngle:REAL4, rCrossAngle:REAL4, nRadianFlag:DWORD
eglBaseVectorFromMapping2D	PROTO	NEAR32 C,
	pBaseVector:PEGL_IMAGE_AXES,
	pvMappedOrigin:PE3D_VECTOR_2D,
	pvMapped:PCE3D_VECTOR_2D, pvMapping:PCE3D_VECTOR_2D
eglMapping2DVectors		PROTO	NEAR32 C,
	pBaseVector:PCEGL_IMAGE_AXES,
	pvMappedOrigin:PCE3D_VECTOR_2D,
	pvMapped:PE3D_VECTOR_2D, pvMapping:PCE3D_VECTOR_2D, nCount:DWORD
eglReverseVertically		PROTO	NEAR32 C,
	pImageInf:PEGL_IMAGE_INFO
eglGetPixel			PROTO	NEAR32 C,
	pImageInf:PCEGL_IMAGE_INFO, nPosX:SDWORD, nPosY:SDWORD
eglSetPixel			PROTO	NEAR32 C,
	pImageInf:PEGL_IMAGE_INFO,
	nPosX:SDWORD, nPosY:SDWORD, colorPixel:EGL_PALETTE


;
;	カラーコード変換
; ----------------------------------------------------------------------------

eglRGBtoYUV	PROTO	NEAR32 C, pColor:PTR EGL_PALETTE
eglYUVtoRGB	PROTO	NEAR32 C, pColor:PTR EGL_PALETTE
eglRGBtoHSB	PROTO	NEAR32 C, pColor:PTR EGL_PALETTE
eglHSBtoRGB	PROTO	NEAR32 C, pColor:PTR EGL_PALETTE
eglBlendRGBA	PROTO	NEAR32 C, rgba1:EGL_PALETTE, rgba2:EGL_PALETTE


;
;	フィルタリング関数
; ----------------------------------------------------------------------------

eglConvertFormat	PROTO	NEAR32 C,
	pDstImage:PEGL_IMAGE_INFO, pSrcImage:PCEGL_IMAGE_INFO, dwFlags:DWORD
eglCalculateToneTable	PROTO	NEAR32 C,
	pToneBuf:PTR, nTone:SDWORD, nFlag:DWORD
eglApplyToneTable	PROTO	NEAR32 C,
	pDstImage:PEGL_IMAGE_INFO, pSrcImage:PCEGL_IMAGE_INFO,
	pBlueTone:PTR, pGreenTone:PTR, pRedTone:PTR, pAlphaTone:PTR
eglSetColorTone		PROTO	NEAR32 C,
	pDstImage:PEGL_IMAGE_INFO, pSrcImage:PCEGL_IMAGE_INFO,
	nBlueTone:SDWORD, nGreenTone:SDWORD, nRedTone:SDWORD, nAlphaTone:SDWORD
eglMakeGrayTone	PROTO	NEAR32 C,
	pDstImage:PEGL_IMAGE_INFO, pSrcImage:PCEGL_IMAGE_INFO
eglEnlargeDouble	PROTO	NEAR32 C,
	pDstImage:PEGL_IMAGE_INFO,
	pSrcImage:PCEGL_IMAGE_INFO, dwFlags:DWORD
eglReduceHalf		PROTO	NEAR32 C,
	pDstImage:PEGL_IMAGE_INFO,
	pSrcImage:PCEGL_IMAGE_INFO, dwFlags:DWORD
eglBlendAlphaChannel	PROTO	NEAR32 C,
	pRGBA32:PEGL_IMAGE_INFO, pSrcRGB:PCEGL_IMAGE_INFO,
	pSrcAlpha:PCEGL_IMAGE_INFO, dwFlags:DWORD,
	nAlphaBase:SDWORD, nCoefficient:DWORD
eglUnpackAlphaChannel	PROTO	NEAR32 C,
	pDstRGB:PEGL_IMAGE_INFO, pDstAlpha:PEGL_IMAGE_INFO,
	pSrcRGBA32:PCEGL_IMAGE_INFO, dwFlags:DWORD

EGL_TONE_BRIGHTNESS	EQU	0000H
EGL_TONE_INVERSION	EQU	0001H
EGL_TONE_LIGHT		EQU	0002H

EGL_BAC_MULTIPLY	EQU	0001H
EGL_BAC_ADD_ALPHA	EQU	0002H
EGL_BAC_MULTIPLY_ALPHA	EQU	0004H


;
;	シェーディング色情報
; ----------------------------------------------------------------------------
E3D_COLOR	STRUCT
rgbMul		EGL_PALETTE	{ }
rgbAdd		EGL_PALETTE	{ }
E3D_COLOR	ENDS
PE3D_COLOR	TYPEDEF	PTR E3D_COLOR
PCE3D_COLOR	TYPEDEF	PTR E3D_COLOR

;
;	リージョン
; ----------------------------------------------------------------------------

E3D_VECTOR4_SW	STRUCT
x		SWORD	?
y		SWORD	?
z		SWORD	?
d		SWORD	?
E3D_VECTOR4_SW	ENDS

E3D_POLY_LINE_REGION	STRUCT
nLeft			SDWORD		?
dwReserved1		DWORD		?
nRight			SDWORD		?
dwReserved2		DWORD		?
rgbaLeft		E3D_COLOR	{ }
rgbaRight		E3D_COLOR	{ }
vLeft			E3D_VECTOR4_SW	{ }
vRight			E3D_VECTOR4_SW	{ }
E3D_POLY_LINE_REGION	ENDS
PE3D_POLY_LINE_REGION	TYPEDEF	PTR E3D_POLY_LINE_REGION

E3D_POLYGON_REGION	STRUCT
nTopLine		SDWORD			?
nBottomLine		SDWORD			?
plrLineRgn		E3D_POLY_LINE_REGION	1 DUP({})
E3D_POLYGON_REGION	ENDS
PE3D_POLYGON_REGION	TYPEDEF	PTR E3D_POLYGON_REGION


;
;	画像描画
; ----------------------------------------------------------------------------

EGL_DRAW_DEST	STRUCT
pDstImage	PEGL_IMAGE_INFO		?
rectDstClip	EGL_RECT		{}
pZBuffer	PEGL_IMAGE_INFO		?
EGL_DRAW_DEST	ENDS

EGL_DRAW_PARAM	STRUCT
dwFlags		DWORD			?
ptBasePos	EGL_POINT		{}
pSrcImage	PEGL_IMAGE_INFO		?
pViewRect	PCEGL_RECT		?
rgbDimColor	EGL_PALETTE		{}
rgbLightColor	EGL_PALETTE		{}
nTransparency	DWORD			?
rZOrder		REAL4			?
pImageAxes	PCEGL_IMAGE_AXES	?
rgbColorParam1	EGL_PALETTE		{}
nVertexCount	DWORD			?
pVertexPos	PCE3D_VECTOR_2D		?
EGL_DRAW_PARAM	ENDS

EGL_DRAW_BLEND_ALPHA	EQU	0001H
EGL_DRAW_GLOW_LIGHT	EQU	0002H
EGL_WITH_Z_ORDER	EQU	0004H
EGL_WITH_AXES		EQU	0008H
EGL_SMOOTH_STRETCH	EQU	0010H
EGL_UNSMOOTH_STRETCH	EQU	0020H
EGL_FIXED_POSITION	EQU	0040H
EGL_POLYGON_SHAPED	EQU	0080H

EGL_APPLY_C_ADD		EQU	00800000H
EGL_APPLY_C_MUL		EQU	00820000H
EGL_APPLY_A_MUL		EQU	00880000H
EGL_APPLY_C_MASK	EQU	00890000H
EGL_DRAW_F_ADD		EQU	80000000H
EGL_DRAW_F_SUB		EQU	81000000H
EGL_DRAW_F_MUL		EQU	82000000H
EGL_DRAW_F_DIV		EQU	83000000H
EGL_DRAW_F_MAX		EQU	84000000H
EGL_DRAW_F_MIN		EQU	85000000H
EGL_DRAW_F_SCREEN	EQU	86000000H
EGL_DRAW_A_MOVE		EQU	88000000H
EGL_DRAW_A_MUL		EQU	89000000H
EGL_DRAW_D_MASK		EQU	8A000000H

EGL_FILL_INVERSION	EQU	00000100H

HEGL_DRAW_IMAGE	TYPEDEF	PTR
PTR_NEAR32	TYPEDEF	PTR NEAR32

DrawImage@Release		TYPEDEF	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
DrawImage@Initialize		TYPEDEF	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE, pDstImage:PEGL_IMAGE_INFO,
	pClipRect:PCEGL_RECT, pZBuffer:PEGL_IMAGE_INFO
DrawImage@GetDestination	TYPEDEF	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE, pDrawDest:PTR EGL_DRAW_DEST
DrawImage@GetFunctionFlags	TYPEDEF	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
DrawImage@SetFunctionFlags	TYPEDEF	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE, dwFlags:DWORD
DrawImage@GetDrawingOffset	TYPEDEF	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE, pDrawOffset:PTR EGL_POINT
DrawImage@SetDrawingOffset	TYPEDEF	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE, pDrawOffset:PTR EGL_POINT
DrawImage@PrepareDraw		TYPEDEF	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE, pDrawParam:PTR EGL_DRAW_PARAM
DrawImage@DrawImage		TYPEDEF	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
DrawImage@PrepareLine		TYPEDEF	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE,
	x1:SDWORD, y1:SDWORD, x2:SDWORD, y2:SDWORD,
	colorDraw:EGL_PALETTE, nTransparency:DWORD, dwFlags:DWORD
DrawImage@PrepareFillRect	TYPEDEF	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE,
	pDrawRect:PCEGL_RECT, colorDraw:EGL_PALETTE,
	nTransparency:DWORD, dwFlags:DWORD
DrawImage@PrepareFillEllipse	TYPEDEF	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE,
	pCenter:PCEGL_POINT, pRadius:PCEGL_SIZE,
	colorDraw:EGL_PALETTE, nTransparency:DWORD, dwFlags:DWORD
DrawImage@PrepareFillPolygon	TYPEDEF	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE,
	pVertexes:PCEGL_POINT, nCount:DWORD,
	colorDraw:EGL_PALETTE, nTransparency:DWORD, dwFlags:DWORD
DrawImage@FillRegion		TYPEDEF	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
DrawImage@DrawRegion		TYPEDEF	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE

PDrawImage@Release		TYPEDEF	PTR DrawImage@Release
PDrawImage@Initialize		TYPEDEF	PTR DrawImage@Initialize
PDrawImage@GetDestination	TYPEDEF	PTR DrawImage@GetDestination
PDrawImage@GetFunctionFlags	TYPEDEF	PTR DrawImage@GetFunctionFlags
PDrawImage@SetFunctionFlags	TYPEDEF	PTR DrawImage@SetFunctionFlags
PDrawImage@GetDrawingOffset	TYPEDEF	PTR DrawImage@GetDrawingOffset
PDrawImage@SetDrawingOffset	TYPEDEF	PTR DrawImage@SetDrawingOffset
PDrawImage@PrepareDraw		TYPEDEF	PTR DrawImage@PrepareDraw
PDrawImage@DrawImage		TYPEDEF	PTR DrawImage@DrawImage
PDrawImage@PrepareLine		TYPEDEF	PTR DrawImage@PrepareLine
PDrawImage@PrepareFillRect	TYPEDEF	PTR DrawImage@PrepareFillRect
PDrawImage@PrepareFillEllipse	TYPEDEF	PTR DrawImage@PrepareFillEllipse
PDrawImage@PrepareFillPolygon	TYPEDEF	PTR DrawImage@PrepareFillPolygon
PDrawImage@FillRegion		TYPEDEF	PTR DrawImage@FillRegion
PDrawImage@DrawRegion		TYPEDEF	PTR DrawImage@DrawRegion

EGL_DRAW_IMAGE		STRUCT
pfnRelease		PDrawImage@Release		?
pfnInitialize		PDrawImage@Initialize		?
pfnGetDestination	PDrawImage@GetDestination	?
pfnGetFunctionFlags	PDrawImage@GetFunctionFlags	?
pfnSetFunctionFlags	PDrawImage@SetFunctionFlags	?
pfnGetDrawingOffset	PDrawImage@GetDrawingOffset	?
pfnSetDrawingOffset	PDrawImage@SetDrawingOffset	?
			DWORD				?
pfnPrepareDraw		PDrawImage@PrepareDraw		?
pfnDrawImage		PDrawImage@DrawImage		?
			DWORD				?
pfnPrepareLine		PDrawImage@PrepareLine		?
pfnPrepareFillRect	PDrawImage@PrepareFillRect	?
pfnPrepareFillEllipse	PDrawImage@PrepareFillEllipse	?
pfnPrepareFillPolygon	PDrawImage@PrepareFillPolygon	?
pfnFillRegion		PDrawImage@FillRegion		?
pfnDrawRegion		PDrawImage@DrawRegion		?
EGL_DRAW_IMAGE		ENDS

eglCreateDrawImage		PROTO	NEAR32 C


;
;	画像描画用バッファ
; ----------------------------------------------------------------------------

EGL_DRAW_IMAGE_BUF	STRUCT

pfnRelease		PDrawImage@Release		?
pfnInitialize		PDrawImage@Initialize		?
pfnGetDestination	PDrawImage@GetDestination	?
pfnGetFunctionFlags	PDrawImage@GetFunctionFlags	?
pfnSetFunctionFlags	PDrawImage@SetFunctionFlags	?
pfnGetDrawingOffset	PDrawImage@GetDrawingOffset	?
pfnSetDrawingOffset	PDrawImage@SetDrawingOffset	?
			DWORD				?
pfnPrepareDraw		PDrawImage@PrepareDraw		?
pfnDrawImage		PDrawImage@DrawImage		?
			DWORD				?
pfnPrepareLine		PDrawImage@PrepareLine		?
pfnPrepareFillRect	PDrawImage@PrepareFillRect	?
pfnPrepareFillEllipse	PDrawImage@PrepareFillEllipse	?
pfnPrepareFillPolygon	PDrawImage@PrepareFillPolygon	?
pfnFillRegion		PDrawImage@FillRegion		?
pfnDrawRegion		PDrawImage@DrawRegion		?

pfnDrawSampling		PTR_NEAR32			?
pfnDrawApplying		PTR_NEAR32			?
pfnDrawRendering	PTR_NEAR32			?
pfnDrawRegionLine	PTR_NEAR32			?
dwDrawFlags		DWORD				?

ALIGN	10H
rgbaColor	EGL_PALETTE		100H DUP({})
nBlueTone	BYTE			100H DUP(?)
nGreenTone	BYTE			100H DUP(?)
nRedTone	BYTE			100H DUP(?)
dwTemp		DWORD			10H DUP(?)

ptDrawOffset	EGL_POINT		{ }

ptBasePos	EGL_POINT		{ }
ptLineBase	EGL_POINT		{ }
ptDltScanX	EGL_POINT		{ }
ptDltScanY	EGL_POINT		{ }
colorDraw	EGL_PALETTE		{ }

ALIGN	8
tmxSizeMask	WORD			4 DUP( ? )	; MMX 用画像サイズ飽和マスク（x10000H fixed）
tmxMulAddr	WORD			4 DUP( ? )	; MMX 用画像アドレス算出用
tmxSrcImageAddr	DWORD			2 DUP( ? )

hHeap		HESLHEAP		?
dwSaveRegEBP	DWORD			?
pDstImage	PEGL_IMAGE_INFO		?
rectClip	EGL_RECT		{ }
pZBuffer	PEGL_IMAGE_INFO		?

pRegion		PE3D_POLYGON_REGION	?
pSrcLineAddr	PVOID			?, ?

dwFlags		DWORD			?
dstimg		EGL_IMAGE_INFO		{ }
zbuf		EGL_IMAGE_INFO		{ }
srcimg		EGL_IMAGE_INFO		{ }
nTrans		DWORD			?
rZOrder		REAL4			?

xPos		SDWORD			?
yPos		SDWORD			?
nLeftWidth	DWORD			?
nLeftHeight	DWORD			?
nBytesPerPixel	DWORD			?, ?
ptrDstLine	PVOID			?
ptrZBufLine	PVOID			?
ptrSrcLine	PVOID			?
ptrRegionLine	PE3D_POLY_LINE_REGION	?
pDrawLineBuf	PVOID			?, ?

EGL_DRAW_IMAGE_BUF	ENDS

eglDrawImage@Release			PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@Initialize			PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE, pDstImage:PEGL_IMAGE_INFO,
	pClipRect:PCEGL_RECT, pZBuffer:PEGL_IMAGE_INFO
eglDrawImage@GetDestination		PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE, pDrawDest:PTR EGL_DRAW_DEST
eglDrawImage@GetFunctionFlags		PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@SetFunctionFlags		PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE, dwFlags:DWORD
eglDrawImage@GetDrawingOffset		PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE, pDrawOffset:PTR EGL_POINT
eglDrawImage@SetDrawingOffset		PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE, pDrawOffset:PTR EGL_POINT
eglDrawImage@PrepareDraw		PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE, pDrawParam:PTR EGL_DRAW_PARAM
eglDrawImage@DrawImage_NotSupported	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@DrawImage_NoDraw		PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@DrawImage_8to8		PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@DrawImage_8to8_Clip	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@DrawImage_8to24		PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@DrawImage_8to24_Trans	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@DrawImage_8to24_Clip	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@DrawImage_8to24_CTrans	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@DrawImage_GlowAdd		PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@DrawImage_GlowAdd_SSE	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@DrawImage_Glow		PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@DrawImage_Glow_SSE		PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@DrawImage_GlowBlend	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@DrawImage_24to24		PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@DrawImage_24to24_SSE	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@DrawImage_24to24_Trans	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@DrawImage_RGB32_Trans_SSE	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@DrawImage_RGBA		PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@DrawImage_RGBA_SSE		PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@DrawImage_RGBA_Trans	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@DrawImage_RGBA_Trans_SSE	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@DrawImage_RGBA_Blend	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@DrawImage_RGBA_Blend_SSE	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@DrawImage_RGBA_BTrans	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@DrawImage_RGBA_BTrans_SSE	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@DrawImage_RGB_X		PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@DrawImage_RGB_XTrans	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@DrawImage_RGB_XTZBuf	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@DrawImage_RGB_X_SSE	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@DrawImage_RGB_XTrans_SSE	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@DrawImage_RGB_XZBuf_SSE	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@DrawImage_RGB_XTZBuf_SSE	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@DrawImage_RGBA_X		PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@DrawImage_RGBA_XTZBuf	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@DrawImage_RGBA_X_SSE	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@DrawImage_RGBA_XTrans_SSE	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@DrawImage_RGBA_XZBuf_SSE	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@DrawImage_RGBA_XTZBuf_SSE	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE

eglDrawImage@DrawImage_ConvertFormat	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@DrawImage_DirectRGBA32	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@DrawImage_Normal		PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@DrawImage_Transform	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE

EXTERNDEF	C eglDrawImage@Sample_Index8:NEAR32
EXTERNDEF	C eglDrawImage@Sample_Gray8:NEAR32
EXTERNDEF	C eglDrawImage@Sample_RGB16:NEAR32
EXTERNDEF	C eglDrawImage@Sample_RGB24:NEAR32
EXTERNDEF	C eglDrawImage@Sample_RGB32:NEAR32

EXTERNDEF	C eglDrawImage@Sample_Index8_X:NEAR32
EXTERNDEF	C eglDrawImage@Sample_Gray8_X:NEAR32
EXTERNDEF	C eglDrawImage@Sample_RGB16_X:NEAR32
EXTERNDEF	C eglDrawImage@Sample_RGB24_X:NEAR32
EXTERNDEF	C eglDrawImage@Sample_RGB32_X:NEAR32
EXTERNDEF	C eglDrawImage@Sample_RGB32_X_MMX:NEAR32
EXTERNDEF	C eglDrawImage@Sample_RGB32_XS_MMX:NEAR32
EXTERNDEF	C eglDrawImage@Sample_RGB32_XS_SSE:NEAR32
EXTERNDEF	C eglDrawImage@Sample_RGB32_XS_SSE2:NEAR32

EXTERNDEF	C eglDrawImage@Render_Gray8:NEAR32
EXTERNDEF	C eglDrawImage@Render_RGB24:NEAR32
EXTERNDEF	C eglDrawImage@Render_RGB24_B:NEAR32
EXTERNDEF	C eglDrawImage@Render_RGB24_BT:NEAR32
EXTERNDEF	C eglDrawImage@Render_RGB24_BTZ:NEAR32
EXTERNDEF	C eglDrawImage@Render_RGB32:NEAR32
EXTERNDEF	C eglDrawImage@Render_RGBA32_B:NEAR32
EXTERNDEF	C eglDrawImage@Render_RGBA32_BT:NEAR32
EXTERNDEF	C eglDrawImage@Render_RGBA32_BTZ:NEAR32
EXTERNDEF	C eglDrawImage@Render_RGB32_SSE:NEAR32
EXTERNDEF	C eglDrawImage@Render_RGBA32_B_MMX:NEAR32
EXTERNDEF	C eglDrawImage@Render_RGBA32_B_SSE:NEAR32
EXTERNDEF	C eglDrawImage@Render_RGBA32_BT_SSE:NEAR32
EXTERNDEF	C eglDrawImage@Render_RGBA32_BZ_SSE:NEAR32
EXTERNDEF	C eglDrawImage@Render_RGBA32_BTZ_SSE:NEAR32

EXTERNDEF	C eglDrawImage@Apply_nothing:NEAR32
EXTERNDEF	C eglDrawImage@Apply_Transparency:NEAR32
EXTERNDEF	C eglDrawImage@Apply_Transparency_MMX:NEAR32
EXTERNDEF	C eglDrawImage@Apply_ITransparency:NEAR32
EXTERNDEF	C eglDrawImage@Apply_ITransparency_MMX:NEAR32
EXTERNDEF	C eglDrawImage@Apply_MulColor:NEAR32
EXTERNDEF	C eglDrawImage@Apply_MulColor_MMX:NEAR32
EXTERNDEF	C eglDrawImage@Apply_AddColor:NEAR32
EXTERNDEF	C eglDrawImage@Apply_AddColor_MMX:NEAR32
EXTERNDEF	C eglDrawImage@Product_Alpha:NEAR32
EXTERNDEF	C eglDrawImage@Product_Alpha_MMX:NEAR32
EXTERNDEF	C eglDrawImage@Apply_ColorMask:NEAR32
EXTERNDEF	C eglDrawImage@Apply_ColorMask_MMX:NEAR32

EXTERNDEF	C eglDrawImage@Render_AddColor:NEAR32
EXTERNDEF	C eglDrawImage@Render_AddColor8:NEAR32
EXTERNDEF	C eglDrawImage@Render_AddColor_MMX:NEAR32
EXTERNDEF	C eglDrawImage@Render_SubColor:NEAR32
EXTERNDEF	C eglDrawImage@Render_SubColor8:NEAR32
EXTERNDEF	C eglDrawImage@Render_SubColor_MMX:NEAR32
EXTERNDEF	C eglDrawImage@Render_MulColor:NEAR32
EXTERNDEF	C eglDrawImage@Render_MulColor8:NEAR32
EXTERNDEF	C eglDrawImage@Render_MulColor_MMX:NEAR32
EXTERNDEF	C eglDrawImage@Render_DivColor:NEAR32
EXTERNDEF	C eglDrawImage@Render_DivColor8:NEAR32
EXTERNDEF	C eglDrawImage@Render_DivColor_MMX:NEAR32
EXTERNDEF	C eglDrawImage@Render_MaxValue:NEAR32
EXTERNDEF	C eglDrawImage@Render_MaxValue8:NEAR32
EXTERNDEF	C eglDrawImage@Render_MinValue:NEAR32
EXTERNDEF	C eglDrawImage@Render_MinValue8:NEAR32
EXTERNDEF	C eglDrawImage@Render_NegMulColor:NEAR32
EXTERNDEF	C eglDrawImage@Render_MoveColor:NEAR32
EXTERNDEF	C eglDrawImage@Render_MoveColor_MMX:NEAR32
EXTERNDEF	C eglDrawImage@Render_AMulColor:NEAR32
EXTERNDEF	C eglDrawImage@Render_AMulColor_MMX:NEAR32
EXTERNDEF	C eglDrawImage@Render_DstMaskColor:NEAR32


eglDrawImage@PrepareLine		PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE,
	x1:SDWORD, y1:SDWORD, x2:SDWORD, y2:SDWORD,
	colorDraw:EGL_PALETTE, nTransparency:DWORD, dwFlags:DWORD
eglDrawImage@PrepareFillRect		PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE,
	pDrawRect:PCEGL_RECT, colorDraw:EGL_PALETTE,
	nTransparency:DWORD, dwFlags:DWORD
eglDrawImage@PrepareFillEllipse		PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE,
	pCenter:PCEGL_POINT, pRadius:PCEGL_SIZE,
	colorDraw:EGL_PALETTE, nTransparency:DWORD, dwFlags:DWORD
eglDrawImage@PrepareFillPolygon		PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE,
	pVertexes:PCEGL_POINT, nCount:DWORD,
	colorDraw:EGL_PALETTE, nTransparency:DWORD, dwFlags:DWORD
eglDrawImage@FillRegion_Error		PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@FillRegion_NoDraw		PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@FillRegion_486		PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@FillRegion_Blend_486	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@FillRegion_Blend_MMX	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@FillRegion_Inversion_486	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@DrawRegion_486		PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@DrawRegion_Blend_486	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@DrawRegion_Inversion_486	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE



;
;	ポリゴンの表面属性
; ----------------------------------------------------------------------------

E3D_TEXTURE_MAPPING	STRUCT
pTextureImage		PEGL_IMAGE_INFO	?	; テクスチャ画像
pLuminousImage		PEGL_IMAGE_INFO	?	; 発光テクスチャ
rThresholdZ		REAL4		?	; 縮小画像に切り替えるための閾値
nSmallScale		DWORD		?	; 縮小画像のスケール (1/2^n)
pSmallImage		PEGL_IMAGE_INFO	?	; 縮小テクスチャ画像
pSmallLuminous		PEGL_IMAGE_INFO	?	; 縮小発光テクスチャ
E3D_TEXTURE_MAPPING	ENDS

E3D_ENVIRONMENT_MAPPING	STRUCT
pUpperImage		PEGL_IMAGE_INFO	 ?	; 上半球
pUnderImage		PEGL_IMAGE_INFO	 ?	; 下半球
dwFlags			DWORD		?
dwReserved		DWORD		3 DUP(?)
E3D_ENVIRONMENT_MAPPING	ENDS

E3D_TEXTURE_ENVIRONMENT_MAPPING	UNION
txmap			E3D_TEXTURE_MAPPING	{}	; テクスチャマッピング
envmap			E3D_ENVIRONMENT_MAPPING	{}	; 環境マッピング
E3D_TEXTURE_ENVIRONMENT_MAPPING	ENDS

E3D_SURFACE_ATTRIBUTE	STRUCT
dwShadingFlags		DWORD		?	; シェーディングフラグ
			DWORD		?
rgbaColor		E3D_COLOR	{}	; 基本色
			E3D_TEXTURE_ENVIRONMENT_MAPPING	{}
;UNION
;	txmap	E3D_TEXTURE_MAPPING	{}	; テクスチャマッピング
;	envmap	E3D_ENVIRONMENT_MAPPING	{}	; 環境マッピング
;ENDS
nTextureApply		DWORD		?	; テクスチャ適用度
nLuminousApply		DWORD		?	; 発光テクスチャ適用度
nAmbient		SDWORD		?	; 環境光
nDiffusion		SDWORD		?	; 拡散光
nSpecular		SDWORD		?	; 反射光
nSpecularSize		SDWORD		?
nTransparency		SDWORD		?	; 透明度
nDeepness		SDWORD		?	; 透明深度係数
rgbaShade		E3D_COLOR	{}	; 影色 (ver.3.09 以降)
nReflection		DWORD		?	; 反射
nRefraction		REAL4		?	; 屈折
E3D_SURFACE_ATTRIBUTE	ENDS
PE3D_SURFACE_ATTRIBUTE	TYPEDEF	PTR E3D_SURFACE_ATTRIBUTE
PCE3D_SURFACE_ATTRIBUTE	TYPEDEF	PTR E3D_SURFACE_ATTRIBUTE

E3DSAF_NO_SHADING		EQU	00000000H	; シェーディング無し
E3DSAF_FLAT_SHADE		EQU	00000001H	; フラットシェーディング（未使用）
E3DSAF_GOURAUD_SHADE		EQU	00000002H	; グーローシェーディング
E3DSAF_PHONG_SHADE		EQU	00000004H	; フォンシェーディング（未使用）
E3DSAF_PHONG_SHADE_BEFORE_TEXTURE	EQU	00000006H
E3DSAF_RAY_SHADOWING		EQU	00000010H	; 影を落とす (ver.3.09 以降)
E3DSAF_RAY_REFLECTING		EQU	00000020H	; 反射を有効にする (ver.3.09 以降)
E3DSAF_RAY_REFRACTING		EQU	00000040H	; 屈折を有効にする (ver.3.09 以降)
E3DSAF_RAY_TRACING		EQU	00000074H	; レイトレーシング (ver.3.09 以降)
E3DSAF_RAY_TRACING_BEFORE_TEXTURE	EQU	00000076H
E3DSAF_SHADING_MASK		EQU	000000FFH
E3DSAF_TEXTURE_TILING		EQU	00000100H	; テクスチャをタイリング
E3DSAF_TEXTURE_TRIM		EQU	00000200H	; トリミングテクスチャ
E3DSAF_TEXTURE_SMOOTH		EQU	00000400H	; テクスチャ補完拡大
E3DSAF_TEXTURE_MAPPING		EQU	00001000H	; テクスチャマッピング
E3DSAF_ENVIRONMENT_MAP		EQU	00002000H	; 環境マッピング
E3DSAF_GENVIRONMENT_MAP		EQU	00004000H	; グローバル環境マッピング
E3DSAF_SINGLE_SIDE_PLANE	EQU	00010000H	; 片面ポリゴン
E3DSAF_NO_ZBUFFER		EQU	00020000H	; ｚ比較を行わないで描画
E3DSAF_ZBUF_ONLY_COMPARE	EQU	00040000H	; ｚ比較のみ（ｚバッファへ書き込まない）
E3DSAF_NO_SHADOW_OBJECT		EQU	01000000H	; 他のオブジェクトに影を落とさない（レイトレーシングモードのみ）
E3DSAF_NO_SALF_SHADOW		EQU	02000000H	; 自分自身のメッシュに陰を落とさない（レイトレーシングモードのみ）
E3DSAF_NO_REFLECT_OBJECT	EQU	04000000H	; 他のオブジェクトに映りこまない（レイトレーシングモードのみ）
E3DSAF_GLOBAL_REFLECT_OBJECT	EQU	08000000H	; 距離に関係なく他のオブジェクトに移りこむ（レイトレーシングモードのみ）


;
;	光源
; ----------------------------------------------------------------------------

E3D_SHADOW_MAP_INFO	STRUCT
vLightPos	E3D_VECTOR4	{}	; 光源座標
vLightRay	E3D_VECTOR4	{}	; 光線ベクトル
vOriginPos	E3D_VECTOR4	{}	; シャドウマップの原点
vAxisX		E3D_VECTOR4	{}	; シャドウマップのｘ基底ベクトル
vAxisY		E3D_VECTOR4	{}	; シャドウマップのｙ基底ベクトル
pShadowMap	PEGL_IMAGE_INFO	?	; シャドウマップ画像
rFixErrorGap	REAL4		?	; z 演算誤差固定比率（1/4096 等）
rVarErrorGap	REAL4		?	; 角度に応じた z 演算誤差比率（1/512 等）
rMapDistance	REAL4		?	; vLightPos からシャドウマップまでの距離
E3D_SHADOW_MAP_INFO	ENDS
PE3D_SHADOW_MAP_INFO	TYPEDEF	PTR E3D_SHADOW_MAP_INFO

E3D_LIGHT_ENTRY	STRUCT
dwLightType	DWORD		?
rgbColor	EGL_PALETTE	{}
UNION
	rBrightness	REAL4	?
	rFogDeepness	REAL4	?
ENDS
UNION
	dwReserved	DWORD	?
	rFogDistance	REAL4	?
ENDS
UNION
	vecLight	E3D_VECTOR4	{}
	STRUCT	ShadowMap
		vecLight	E3D_VECTOR		{}
		pMapInfo	PE3D_SHADOW_MAP_INFO	?
	ENDS
ENDS
E3D_LIGHT_ENTRY	ENDS
PE3D_LIGHT_ENTRY	TYPEDEF	PTR E3D_LIGHT_ENTRY
PCE3D_LIGHT_ENTRY	TYPEDEF	PTR E3D_LIGHT_ENTRY

E3D_FOG_LIGHT		EQU	00000000H	; 擬似フォッグ
E3D_AMBIENT_LIGHT	EQU	00000001H	; 環境光
E3D_VECTOR_LIGHT	EQU	00000002H	; 無限遠光源
E3D_SHADOW_LIGHT	EQU	00000003H	; 影色用環境光源
E3D_POINT_LIGHT		EQU	00000004H	; 点光源
E3D_SENSITIVITY_LIGHT	EQU	00000010H	; 光感度
E3D_LIGHT_TYPE_MASK	EQU	0000000FH
E3D_LIGHT_SHADOW_MAP	EQU	80000000H	; シャドウマップ


;
;	テクスチャマッピング用パラメータ
; ----------------------------------------------------------------------------

E3D_TEXTURE_MAPINFO	STRUCT
pTextureImage		PEGL_IMAGE_INFO	?	; 通常テクスチャ
pLuminousImage		PEGL_IMAGE_INFO	?	; 発光テクスチャ
nTextureApply		DWORD		?	; テクスチャ適用度
nLuminousApply		DWORD		?	; 発光テクスチャ適用度
vOriginPos		E3D_VECTOR4	{}	; マッピング用パラメータ
vAxisX			E3D_VECTOR4	{}
vAxisY			E3D_VECTOR4	{}
rFogDeepness		REAL4		?	; 擬似フォッグ用
rgbFogColor		EGL_PALETTE	{}
E3D_TEXTURE_MAPINFO	ENDS


;
;	ポリゴンプリミティブ情報
; ----------------------------------------------------------------------------

E3D_PRIMITIVE_VERTEX	STRUCT
vertex			PE3D_VECTOR4	?	; 頂点座標
normal			PE3D_VECTOR4	?	; 法線（スムースポリゴンのみ）
UNION
	uv_map		E3D_VECTOR_2D	{}	; UV 座標（テクスチャポリゴンのみ）
	color		E3D_COLOR	{}	; 頂点色（頂点色ポリゴンのみ）
ENDS
E3D_PRIMITIVE_VERTEX	ENDS

E3D_PRIMITIVE_INFINITE_PLANE	STRUCT
vertex				PE3D_VECTOR4	?	; 平面基準点
xAxis				PE3D_VECTOR4	?	; テクスチャｘ軸
yAxis				PE3D_VECTOR4	?	; テクスチャｙ軸
rFogDeepness			REAL4		?	; 擬似フォッグ用
rgbFogColor			EGL_PALETTE	{}
E3D_PRIMITIVE_INFINITE_PLANE	ENDS

E3D_PRIMITIVE_IMAGE	STRUCT
vCenter			PE3D_VECTOR4	?	; 中心座標
vEnlarge		PE3D_VECTOR4	?	; 拡大率
vImageBase		E3D_VECTOR_2D	{}	; 画像の基準座標
dwTransparency		DWORD		?	; 透明度
rRevolveAngle		REAL4		?	; 回転角度 [rad]
pImageInf		PEGL_IMAGE_INFO	?	; 画像バッファ（必要であれば）
E3D_PRIMITIVE_IMAGE	ENDS

E3D_PRIMITIVE_MESH_POLY	STRUCT
dwVertexCount	DWORD		?		; メッシュエントリの頂点数
dwIndex		DWORD		3 DUP(?)	; 頂点の指標
E3D_PRIMITIVE_MESH_POLY	ENDS

PE3D_PRIMITIVE_MESH_POLY	TYPEDEF	PTR E3D_PRIMITIVE_MESH_POLY

E3D_PRIMITIVE_MESH	STRUCT
vertices	PE3D_VECTOR4	?		; 頂点座標配列
normals		PE3D_VECTOR4	?		; 法線配列
UNION
	uv_map	E3D_VECTOR_2D	1 DUP({})	; UV 座標配列（テクスチャ・非テクスチャ共）
	color	E3D_COLOR	1 DUP({})	; 頂点色
ENDS
E3D_PRIMITIVE_MESH	ENDS

E3D_PRIMITIVE_MESH_LIST	STRUCT
dwMeshBytes	DWORD		?			; E3D_PRIMITIVE_MESH_LIST データサイズ
dwPolyCount	DWORD		?			; メッシュに含まれるポリゴン数
mpEntries	E3D_PRIMITIVE_MESH_POLY	1 DUP({})	;
E3D_PRIMITIVE_MESH_LIST	ENDS

PE3D_PRIMITIVE_MESH_LIST	TYPEDEF	PTR E3D_PRIMITIVE_MESH_LIST

E3D_PRIMITIVE_POLYGON	STRUCT
dwTypeFlag		DWORD			?
pSurfaceAttr		PE3D_SURFACE_ATTRIBUTE	?
dwVertexCount		DWORD			?
dwDataSize		DWORD			?
UNION
	polygon		E3D_PRIMITIVE_VERTEX		1 DUP({})
	infinite_plane	E3D_PRIMITIVE_INFINITE_PLANE	{}
	image		E3D_PRIMITIVE_IMAGE		{}
	mesh		E3D_PRIMITIVE_MESH		{}
ENDS
E3D_PRIMITIVE_POLYGON	ENDS

PE3D_PRIMITIVE_POLYGON	TYPEDEF	PTR E3D_PRIMITIVE_POLYGON
PCE3D_PRIMITIVE_POLYGON	TYPEDEF	PTR E3D_PRIMITIVE_POLYGON

E3D_FLAT_POLYGON		EQU	00000000H
E3D_SMOOTH_POLYGON		EQU	00000001H
E3D_TEXTURE_POLYGON		EQU	00000002H
E3D_VERTEX_COLOR_POLYGON	EQU	00000004H
E3D_POLYGON_PRIMITIVE_MASK	EQU	00000007H
E3D_INFINITE_PLANE		EQU	0000000AH
E3D_MESH_POLYGON		EQU	00000010H
E3D_IMAGE_PRIMITIVE		EQU	00000100H

;
;	平面パラメータ
; ----------------------------------------------------------------------------

E3D_PLANE_PARAMETER	TYPEDEF	E3D_VECTOR4
PE3D_PLANE_PARAMETER	TYPEDEF	PTR E3D_PLANE_PARAMETER
PCE3D_PLANE_PARAMETER	TYPEDEF	PTR E3D_PLANE_PARAMETER


;
;	レンダリングポリゴン情報
; ----------------------------------------------------------------------------

E3D_POLYGON_ENTRY	STRUCT
dwTypeFlag		DWORD			?
dwTransparency		DWORD			?
pAttr			PE3D_SURFACE_ATTRIBUTE	?
dwShadingFlags		DWORD			?
UNION	surface
	STRUCT	poly
		txmap		E3D_TEXTURE_MAPINFO	{}
	ENDS
	STRUCT	image
		pInfo		PEGL_IMAGE_INFO		?
		rRevolveAngle	REAL4			?
		vCenter		E3D_VECTOR_2D		{}
		vEnlarge	E3D_VECTOR_2D		{}
		vImageBase	E3D_VECTOR_2D		{}
	ENDS
	STRUCT	mesh
		vMinMesh	E3D_VECTOR4			{}
		vMaxMesh	E3D_VECTOR4			{}
		pMesh		PE3D_PRIMITIVE_MESH_LIST	?
		pUVMap		PE3D_VECTOR_2D			?
		rMeshRadius	REAL4				?
		pMeshReserved	PVOID				?
		pMeshReserved2	PVOID				?
	ENDS
ENDS
plane			E3D_PLANE_PARAMETER	{}
vCenter			E3D_VECTOR4		{}
dwVertexCount		DWORD			?
pVertexes		PE3D_VECTOR4		?
pNormals		PE3D_VECTOR4		?
dwProjectedCount	DWORD			?
pProjVertexes		PE3D_VECTOR_2D		?
pVertexColors		PE3D_COLOR		?
E3D_POLYGON_ENTRY	ENDS
PE3D_POLYGON_ENTRY	TYPEDEF	PTR E3D_POLYGON_ENTRY
PCE3D_POLYGON_ENTRY	TYPEDEF	PTR E3D_POLYGON_ENTRY
PPE3D_POLYGON_ENTRY	TYPEDEF	PTR PE3D_POLYGON_ENTRY


;
;	3D 描画パラメータ
; ----------------------------------------------------------------------------

E3D_RENDER_PARAM	STRUCT
dwFlags			DWORD			?
dwTransparency		DWORD			?
rgbaColor		E3D_COLOR		{}
pSrcImage		PEGL_IMAGE_INFO		?
pLuminousImage		PEGL_IMAGE_INFO		?
nTextureApply		DWORD			?
nLuminousApply		DWORD			?
UNION
	STRUCT	rev
		vRenderPos	E3D_VECTOR		{}
		vRevCenter	E3D_VECTOR_2D		{}
		nViewVertexes	SDWORD			?
		pViewVertexes	PCE3D_VECTOR_2D		?
		pVertexColors	PCE3D_COLOR		?
		pRevMatrix	PCE3D_REV_MATRIX	?
	ENDS
	STRUCT	poly
		nViewVertexes	SDWORD			?
		pVertexes	PCE3D_VECTOR4		?
		pViewVertexes	PCE3D_VECTOR_2D		?
		pVertexColors	PCE3D_COLOR		?
	ENDS
ENDS
E3D_RENDER_PARAM	ENDS
PE3D_RENDER_PARAM	TYPEDEF	PTR E3D_RENDER_PARAM
PCE3D_RENDER_PARAM	TYPEDEF	PTR E3D_RENDER_PARAM

E3DRP_RENDER_IMAGE	EQU	0010H
E3DRP_RENDER_POLYGON	EQU	0000H
E3DRP_Z_ORDER_SCREEN	EQU	0020H
E3DRP_NO_SCREEN_ORIGIN	EQU	0040H


;
;	レイトレーシングパラメータ
; ----------------------------------------------------------------------------

EGL_RENDER_RAY_TRACE_PARAM	STRUCT
dwFlags				DWORD	?	; フラグ (EGLRenderRayTraceFlag)
rShadowingDistance		REAL4	?	; 影を落とす有効距離
rRayTracingDistance		REAL4	?	; 光線追跡有効距離
dwRayReflectCount		DWORD	?	; 光線追跡回数
dwAppendShadowingCount		DWORD	?	; 陰光線追跡回数（追加）
EGL_RENDER_RAY_TRACE_PARAM	ENDS

E3D_RAYTRACE_SHADOWING		EQU	0001H	; 影を落とす
E3D_RAYTRACE_SHADOW_ALPHA	EQU	0002H	; 影に透明度を考慮する
E3D_RAYTRACE_SHADOWING_APPEND	EQU	0004H	; dwAppendShadowingCount を有効にする
E3D_RAYTRACE_REFLECTION		EQU	0010H	; 反射を有効にする
E3D_RAYTRACE_REFRACTION		EQU	0020H	; 屈折を有効にする
E3D_RAYTRACE_ONLY_GLOBAL_REF	EQU	0040H	; 大域反射・屈折のみを有効にする

;
;	ポリゴン当たり判定用データ構造体
; ----------------------------------------------------------------------------

EGL_RENDER_POLY_MATRIX_PCK4	STRUCT
xVertexA			REAL4	4 DUP(?)	; 基準頂点 A
yVertexA			REAL4	4 DUP(?)
zVertexA			REAL4	4 DUP(?)
xNormal				REAL4	4 DUP(?)	; (A - B) * (A - C) / (|A - B| * |A - C|)
yNormal				REAL4	4 DUP(?)
zNormal				REAL4	4 DUP(?)
xVertexB			REAL4	4 DUP(?)	; 頂点 B
yVertexB			REAL4	4 DUP(?)
zVertexB			REAL4	4 DUP(?)
xVertexC			REAL4	4 DUP(?)	; 頂点 C
yVertexC			REAL4	4 DUP(?)
zVertexC			REAL4	4 DUP(?)
xAB				REAL4	4 DUP(?)	; (A - B) / |A - B|
yAB				REAL4	4 DUP(?)
zAB				REAL4	4 DUP(?)
xAC				REAL4	4 DUP(?)	; (A - C) / |A - C|
yAC				REAL4	4 DUP(?)
zAC				REAL4	4 DUP(?)
xBC				REAL4	4 DUP(?)	; (B - C) / |B - C|
yBC				REAL4	4 DUP(?)
zBC				REAL4	4 DUP(?)
max_ab_ac_sqr			REAL4	4 DUP(?)	; max(|A - B|,|A - C|)^2
error_gap			REAL4	4 DUP(?)	; |A| * error_scale
cos_abc				REAL4	4 DUP(?)	; cos (∠ABC)
cos_acb				REAL4	4 DUP(?)	; cos (∠ACB)
dwReserved			DWORD	4 DUP(?)
EGL_RENDER_POLY_MATRIX_PCK4	ENDS
PEGL_RENDER_POLY_MATRIX_PCK4	TYPEDEF	PTR EGL_RENDER_POLY_MATRIX_PCK4	

EGL_RENDER_POLY_SPHERE_PCK4	STRUCT
xCenter				REAL4	4 DUP(?)	; 基準頂点
yCenter				REAL4	4 DUP(?)
zCenter				REAL4	4 DUP(?)
rRadius				REAL4	4 DUP(?)	; 有効半径
EGL_RENDER_POLY_SPHERE_PCK4	ENDS

EGL_RENDER_MESH_MATRIX_PCK4	STRUCT
xCenter				REAL4	4 DUP(?)	; 基準頂点
yCenter				REAL4	4 DUP(?)
zCenter				REAL4	4 DUP(?)
rRadius				REAL4	4 DUP(?)	; 有効半径
pMesh				PE3D_POLYGON_ENTRY	4 DUP(?)
EGL_RENDER_MESH_MATRIX_PCK4	ENDS
PEGL_RENDER_MESH_MATRIX_PCK4	TYPEDEF	PTR EGL_RENDER_MESH_MATRIX_PCK4
PPEGL_RENDER_MESH_MATRIX_PCK4	TYPEDEF	PTR PEGL_RENDER_MESH_MATRIX_PCK4

EGL_RENDER_MESH_HIT_ENTRY	STRUCT
vHitPosition			E3D_VECTOR		{ }
pHitMesh			PE3D_POLYGON_ENTRY	?
nHitMeshIndex			DWORD			?
rDistance			REAL4			?
EGL_RENDER_MESH_HIT_ENTRY	ENDS

EGL_RENDER_MESH_HIT_PARAM	STRUCT
rRangeMin			REAL4			?
rRangeMax			REAL4			?
pExceptingMesh			PE3D_POLYGON_ENTRY	?
nExceptingMeshIndex		DWORD			?
pExceptingMeshEntry		PE3D_POLYGON_ENTRY	?
				DWORD			?
nResultCount			DWORD			?
nResultLimit			DWORD			?
rmheResults			EGL_RENDER_MESH_HIT_ENTRY {}
EGL_RENDER_MESH_HIT_PARAM	ENDS

;EGL_RENDER_MESH_HIT_PARAM	STRUCT
;rRangeMin			REAL4			?
;rRangeMax			REAL4			?
;pLastHitMesh			PE3D_POLYGON_ENTRY	?
;nLastHitMeshIndex		DWORD			?
;pExceptingMesh			PE3D_POLYGON_ENTRY	?
;nExceptingMeshIndex		DWORD			?
;pExceptingMeshEntry		PE3D_POLYGON_ENTRY	?
;				DWORD			?
;vHitPosition			E3D_VECTOR4		{ }
;EGL_RENDER_MESH_HIT_PARAM	ENDS

EGL_RENDER_MESH_HIT_PARAM_AND_RAY	STRUCT
rmhp	EGL_RENDER_MESH_HIT_PARAM	{ }
vRay	E3D_VECTOR4			{ }
EGL_RENDER_MESH_HIT_PARAM_AND_RAY	ENDS

EGL_RENDER_MESH_VIEW_POINT	STRUCT
vViewPosition			E3D_VECTOR4		{ }
vTargetPlane			E3D_VECTOR4		{ }
pExceptingMesh			PE3D_POLYGON_ENTRY	?
nExceptingMeshIndex		DWORD			?
EGL_RENDER_MESH_VIEW_POINT	ENDS

EGL_RENDER_MESH_HIT_SURFACE	STRUCT
vNormal				E3D_VECTOR4	{ }
vPlane				E3D_VECTOR4	{ }
uv				E3D_VECTOR_2D	{ }
uvTexture			E3D_VECTOR_2D	{ }
clrSurface			E3D_COLOR	{ }
pAttr				PE3D_SURFACE_ATTRIBUTE	?
dwShadingFlags			DWORD		?
rgbaTexture			EGL_PALETTE	{ }
nTransparency			DWORD		?
nDeepness			SDWORD		?
EGL_RENDER_MESH_HIT_SURFACE	ENDS


;
;	レイトレーシング GPGPU 処理用インターフェース
; ----------------------------------------------------------------------------

E3D_GPU_PLUGIN_PROPERTY	STRUCT
maxThreadsPerBlock	SDWORD	?
maxThreadsDim		SDWORD	3 DUP(?)
maxGridSize		SDWORD	3 DUP(?)
maxReflection		SDWORD	?
E3D_GPU_PLUGIN_PROPERTY	ENDS


;
;	ポリゴン描画
; ----------------------------------------------------------------------------

HEGL_RENDER_POLYGON	TYPEDEF	PTR

RenderPoly@Release		TYPEDEF	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON
RenderPoly@Initialize		TYPEDEF	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON,
	pDstImage:PEGL_IMAGE_INFO, pClipRect:PCEGL_RECT,
	pZBuffer:PEGL_IMAGE_INFO, pScreenPos:PCE3D_VECTOR
RenderPoly@InitializeToReference	TYPEDEF	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON,
	hRefRenderPoly:HEGL_RENDER_POLYGON,
	pClipRect:PCEGL_RECT, dwFlags:DWORD
RenderPoly@GetScreenPos		TYPEDEF	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON
RenderPoly@GetDrawImage		TYPEDEF	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON
RenderPoly@GetFunctionFlags	TYPEDEF	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON
RenderPoly@SetFunctionFlags	TYPEDEF	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON, dwFlags:DWORD
RenderPoly@PrepareMatrix	TYPEDEF	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON,
	matrix:PTR E3D_REV_MATRIX,
	pOrigin:PCE3D_VECTOR, pEnlarge:PCE3D_VECTOR
RenderPoly@RevolveMatrix	TYPEDEF	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON,
	pDst:PE3D_VECTOR4, pSrc:PCE3D_VECTOR4, nVectorCount:DWORD
RenderPoly@SetZClipRange	TYPEDEF	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON, rMin:REAL4, rMax:REAL4
RenderPoly@SetEnvironmentMapping	TYPEDEF	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON,
	matrix:PTR E3D_REV_MATRIX,
	envmap:PTR E3D_ENVIRONMENT_MAPPING
RenderPoly@PrepareLight		TYPEDEF	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON, hStackHeap:HSTACKHEAP,
	nLightCount:DWORD, pLightEntries:PCE3D_LIGHT_ENTRY
RenderPoly@CretaePolygonEntry	TYPEDEF	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON,
	hStackHeap:HSTACKHEAP, pPrimitive:PCE3D_PRIMITIVE_POLYGON
RenderPoly@MakeUpPolygon	TYPEDEF	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON,
	hStackHeap:HSTACKHEAP, pPolyEntry:PTR E3D_POLYGON_ENTRY
RenderPoly@ApplyAttribute	TYPEDEF	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON,
	pPolyEntry:PTR E3D_POLYGON_ENTRY,
	pColor:PCE3D_COLOR, nTransparency:DWORD
RenderPoly@GetExternalRect	TYPEDEF	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON,
	pExtRect:PTR EGL_RECT,
	pPolyEntry:PTR PCE3D_POLYGON_ENTRY, nCount:DWORD
RenderPoly@ShadeReflectLights	TYPEDEF	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON,
	pSurfaceAttribute:PCE3D_SURFACE_ATTRIBUTE,
	pNormals:PCE3D_VECTOR4, pFocusPoints:PCE3D_VECTOR4,
	pColorsLooks:PTR E3D_COLOR, nCount:DWORD
RenderPoly@ProjectScreen	TYPEDEF	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON,
	pDst:PE3D_VECTOR_2D, pSrc:PCE3D_VECTOR4, nVertexCount:DWORD
RenderPoly@SortPolygonEntry	TYPEDEF PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON,
	ppPolygons:PTR PE3D_POLYGON_ENTRY,
	nPolygonCount:DWORD, dwSortingFlags:DWORD
RenderPoly@PrepareRender	TYPEDEF	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON, pPolyEntry:PCE3D_POLYGON_ENTRY
RenderPoly@PrepareRenderParam	TYPEDEF	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON, pRenderParam:PCE3D_RENDER_PARAM
RenderPoly@RenderPolygon	TYPEDEF	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON
RenderPoly@SetRayTracingParameter	TYPEDEF PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON,
	prrtpParam:PTR EGL_RENDER_RAY_TRACE_PARAM
RenderPoly@AttachRayTracingTarget	TYPEDEF	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON, hStackHeap:HSTACKHEAP,
	ppShadowingPoly:PTR PE3D_POLYGON_ENTRY, nShadowingCount:DWORD,
	ppRayTracingPoly:PTR PE3D_POLYGON_ENTRY, nRayTracingCount:DWORD,
	ppGlobalReflection:PTR PE3D_POLYGON_ENTRY, nGlobalReflections:DWORD
RenderPoly@CreatePolygonEntryRT	TYPEDEF	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON, hStackHeap:HSTACKHEAP,
	pPrimitive:PCE3D_PRIMITIVE_POLYGON, pdwResult:PTR DWORD
RenderPoly@PrepareRenderRayTracing	TYPEDEF	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON, pRenderRect:PCEGL_RECT
RenderPoly@RenderRayTracing	TYPEDEF	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON, hSyncRender:HEGL_RENDER_POLYGON
RenderPoly@AbortRenderRayTracing	TYPEDEF	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON
RenderPoly@GetProgressRayTracing	TYPEDEF	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON, pdwPixelCount:PTR DWORD
RenderPoly@AllocateRayTraceRect		TYPEDEF	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON,
	pRenderRect:PTR EGL_RECT,
	nLineCount:DWORD, nOffset:DWORD

RenderPoly@RenderPolygonSub	TYPEDEF	PROTO	NEAR32 C

PRenderPoly@Release		TYPEDEF	PTR RenderPoly@Release
PRenderPoly@Initialize		TYPEDEF	PTR RenderPoly@Initialize
PRenderPoly@InitializeToReference	\
				TYPEDEF	PTR RenderPoly@InitializeToReference
PRenderPoly@GetScreenPos	TYPEDEF	PTR RenderPoly@GetScreenPos
PRenderPoly@GetDrawImage	TYPEDEF	PTR RenderPoly@GetDrawImage
PRenderPoly@GetFunctionFlags	TYPEDEF	PTR RenderPoly@GetFunctionFlags
PRenderPoly@SetFunctionFlags	TYPEDEF	PTR RenderPoly@SetFunctionFlags
PRenderPoly@PrepareMatrix	TYPEDEF	PTR RenderPoly@PrepareMatrix
PRenderPoly@RevolveMatrix	TYPEDEF	PTR RenderPoly@RevolveMatrix
PRenderPoly@SetZClipRange	TYPEDEF	PTR RenderPoly@SetZClipRange
PRenderPoly@SetEnvironmentMapping	\
				TYPEDEF	PTR RenderPoly@SetEnvironmentMapping
PRenderPoly@PrepareLight	TYPEDEF	PTR RenderPoly@PrepareLight
PRenderPoly@CretaePolygonEntry	TYPEDEF	PTR RenderPoly@CretaePolygonEntry
PRenderPoly@MakeUpPolygon	TYPEDEF	PTR RenderPoly@MakeUpPolygon
PRenderPoly@ApplyAttribute	TYPEDEF	PTR RenderPoly@ApplyAttribute
PRenderPoly@GetExternalRect	TYPEDEF	PTR RenderPoly@GetExternalRect
PRenderPoly@ShadeReflectLights	TYPEDEF	PTR RenderPoly@ShadeReflectLights
PRenderPoly@ProjectScreen	TYPEDEF	PTR RenderPoly@ProjectScreen
PRenderPoly@SortPolygonEntry	TYPEDEF	PTR RenderPoly@SortPolygonEntry
PRenderPoly@PrepareRender	TYPEDEF	PTR RenderPoly@PrepareRender
PRenderPoly@PrepareRenderParam	TYPEDEF	PTR RenderPoly@PrepareRenderParam
PRenderPoly@RenderPolygon	TYPEDEF	PTR RenderPoly@RenderPolygon
PRenderPoly@SetRayTracingParameter	\
				TYPEDEF	PTR RenderPoly@SetRayTracingParameter
PRenderPoly@AttachRayTracingTarget	\
				TYPEDEF	PTR RenderPoly@AttachRayTracingTarget
PRenderPoly@CreatePolygonEntryRT	\
				TYPEDEF	PTR RenderPoly@CreatePolygonEntryRT
PRenderPoly@PrepareRenderRayTracing	\
				TYPEDEF	PTR RenderPoly@PrepareRenderRayTracing
PRenderPoly@RenderRayTracing	TYPEDEF	PTR RenderPoly@RenderRayTracing
PRenderPoly@AbortRenderRayTracing	\
				TYPEDEF	PTR RenderPoly@AbortRenderRayTracing
PRenderPoly@GetProgressRayTracing	\
				TYPEDEF	PTR RenderPoly@GetProgressRayTracing
PRenderPoly@AllocateRayTraceRect	\
				TYPEDEF	PTR RenderPoly@AllocateRayTraceRect


PRenderPoly@RenderPolygonSub	TYPEDEF	PTR PRenderPoly@RenderPolygonSub

EGL_RENDER_POLYGON		STRUCT
pfnRelease			PRenderPoly@Release			?
pfnInitialize			PRenderPoly@Initialize			?
pfnInitializeToReference	PRenderPoly@InitializeToReference	?
pfnGetScreenPos			PRenderPoly@GetScreenPos		?
pfnGetDrawImage			PRenderPoly@GetDrawImage		?
pfnGetFunctionFlags		PRenderPoly@GetFunctionFlags		?
pfnSetFunctionFlags		PRenderPoly@SetFunctionFlags		?
pfnPrepareMatrix		PRenderPoly@PrepareMatrix		?
pfnRevolveMatrix		PRenderPoly@RevolveMatrix		?
pfnSetZClipRange		PRenderPoly@SetZClipRange		?
pfnSetEnvironmentMapping	PRenderPoly@SetEnvironmentMapping	?
pfnPrepareLight			PRenderPoly@PrepareLight		?
pfnCreatePolygonEntry		PRenderPoly@CretaePolygonEntry		?
pfnMakeUpPolygon		PRenderPoly@MakeUpPolygon		?
pfnApplyAttribute		PRenderPoly@ApplyAttribute		?
pfnGetExternalRect		PRenderPoly@GetExternalRect		?
pfnShadeReflectLights		PRenderPoly@ShadeReflectLights		?
pfnProjectScreen		PRenderPoly@ProjectScreen		?
pfnSortPolygonEntry		PRenderPoly@SortPolygonEntry		?
pfnPrepareRender		PRenderPoly@PrepareRender		?
pfnPrepareRenderParam		PRenderPoly@PrepareRenderParam		?
pfnRenderPolygon		PRenderPoly@RenderPolygon		?
pfnSetRayTracingParameter	PRenderPoly@SetRayTracingParameter	?
pfnAttachRayTracingTarget	PRenderPoly@AttachRayTracingTarget	?
pfnCreatePolygonEntryRT		PRenderPoly@CreatePolygonEntryRT	?
pfnPrepareRenderRayTracing	PRenderPoly@PrepareRenderRayTracing	?
pfnRenderRayTracing		PRenderPoly@RenderRayTracing		?
pfnAbortRenderRayTracing	PRenderPoly@AbortRenderRayTracing	?
pfnGetProgressRayTracing	PRenderPoly@GetProgressRayTracing	?
pfnAllocateRayTraceRect		PRenderPoly@AllocateRayTraceRect	?
EGL_RENDER_POLYGON		ENDS

E3D_FLAG_INIT_REF_GPGPU_BUF	EQU	0100H

E3D_FLAG_ANTIALIAS_SIDE_EDGE	EQU	0001H
E3D_FLAG_TEXTURE_SMOOTHING	EQU	0002H
E3D_FLAG_PHONG_SHADING		EQU	0004H
E3D_FLAG_RAY_SHADOWING		EQU	0010H
E3D_FLAG_RAY_REFLECTING		EQU	0020H
E3D_FLAG_RAY_REFRACTING		EQU	0040H
E3D_FLAG_RAY_TRACING		EQU	0074H
E3D_FLAG_ENABLE_SSE2		EQU	0100H

E3D_SORT_TRANSPARENT	EQU	0001H
E3D_SORT_OPAQUE		EQU	0002H

E3D_RTCPE_RESULT_NO_RENDER	EQU	0	; 視界外
E3D_RTCPE_RESULT_SHOULD_RENDER	EQU	1	; 視界内
E3D_RTCPE_RESULT_NO_SHADOWING	EQU	2	; 影オブジェクト対象外
E3D_RTCPE_RESULT_NO_REFLECTING	EQU	4	; 反射オブジェクト対象外
E3D_RTCPE_RESULT_NO_RAYTRACING	EQU	6

eglNormalizePolygonRegion	PROTO	NEAR32 C,
	pPolyRegion:PE3D_POLYGON_REGION,
	pClipRect:PCEGL_RECT, nVertexCount:DWORD,
	pPolyVertexes:PCE3D_VECTOR_2D,
	pVertexColors:PCE3D_COLOR, pNormals:PCE3D_VECTOR4
eglCreateRenderPolygon		PROTO	NEAR32 C
eglCurrentRenderPolygon		PROTO	NEAR32 C

;
; 逆数テーブル用
;
PACK_INT_REAL	STRUCT	1
int_part	SDWORD	?		; fixed : 40000000H / i
real_part	REAL4	?
PACK_INT_REAL	ENDS
tblRcpLookup	TEXTEQU	<tblRcpTableLookup256>
EXTERNDEF	C tblRcpLookup:PACK_INT_REAL
SizeOf_tblRcpLookup	EQU	100H


;
;	ポリゴン描画用バッファ
; ----------------------------------------------------------------------------

E3D_TRANS_LINE_BUF	STRUCT
rgbMul			EGL_PALETTE	2 DUP({})
rgbAdd			EGL_PALETTE	2 DUP({})
rZValue			REAL4		2 DUP(?)
E3D_TRANS_LINE_BUF	ENDS


E3D_PACKED_LIGHT	STRUCT
vLights			E3D_VECTOR4	3 DUP({})
vBrightness		E3D_VECTOR4	{}
rgbColor		EGL_PALETTE	4 DUP({})
E3D_PACKED_LIGHT	ENDS
PE3D_PACKED_LIGHT	TYPEDEF	PTR E3D_PACKED_LIGHT


E3D_VECTOR_PW	STRUCT
x		SWORD	?	; x100H
y		SWORD	?
z		SWORD	?
d		SWORD	?
E3D_VECTOR_PW	ENDS

E3D_PALLET_PW	STRUCT
Blue		SWORD	?
Green		SWORD	?
Red		SWORD	?
Alpha		SWORD	?
E3D_PALLET_PW	ENDS

E3D_COLOR_PW	STRUCT
rgbMul		E3D_PALLET_PW	{}
rgbAdd		E3D_PALLET_PW	{}
E3D_COLOR_PW	ENDS

E3D_VECTOR_LIGHT_ENTRY	STRUCT
vLight		E3D_VECTOR_PW	{}	; [0～100H] , d = 0
rgbColor	E3D_PALLET_PW	{}	; [0～7FFFH]（スケーリング無し）
vcLight		E3D_VECTOR4	{}	; x256
ShadowMap	E3D_SHADOW_MAP_INFO	{}
E3D_VECTOR_LIGHT_ENTRY	ENDS
PE3D_VECTOR_LIGHT_ENTRY	TYPEDEF	PTR E3D_VECTOR_LIGHT_ENTRY

E3D_POINT_LIGHT_ENTRY	STRUCT
vLight		E3D_VECTOR	{}
rBrightness	REAL4		?
rgbColor	E3D_PALLET_PW	{}	; [0～0FFH]（スケーリング無し）
E3D_POINT_LIGHT_ENTRY	ENDS
PE3D_POINT_LIGHT_ENTRY	TYPEDEF	PTR E3D_POINT_LIGHT_ENTRY

RenderPoly@ShadeReflectLightsProc	TYPEDEF	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON,
	pSurfaceAttribute:PCE3D_SURFACE_ATTRIBUTE,
	pNormals:PCE3D_VECTOR4, pFocusPoints:PCE3D_VECTOR4,
	pColorsLooks:PTR E3D_COLOR, nCount:DWORD,
	pTargetPlane:PE3D_VECTOR4
PRenderPoly@ShadeReflectLightsProc	TYPEDEF PTR RenderPoly@ShadeReflectLightsProc


;
;	EGL_RENDER_POLYGON_BUF 構造体
; ----------------------------------------------------------------------------

EGL_RENDER_POLYGON_BUF	STRUCT

; RenderPolygon インターフェース
rp			EGL_RENDER_POLYGON	{}
pfnLineFunc		PRenderPoly@RenderPolygonSub	?
pfnStoreFunc		PRenderPoly@RenderPolygonSub	?
pfnAntialius		PRenderPoly@RenderPolygonSub	?
pfnLineSubFunc		PRenderPoly@RenderPolygonSub	?	; フォンシェーディング用
pfnLineShading		PRenderPoly@ShadeReflectLightsProc	?	; フォンシェーディング用シェーディング関数

; DrawImage インターフェース
ALIGN	10H
dib			EGL_DRAW_IMAGE_BUF	{}
qwTemp			QWORD	10H DUP(?)

; 座標変換行列パラメータ
ALIGN	10H
revmat			E3D_REV_MATRIX		{}
vRevOrg			E3D_VECTOR4		{}

; 画面情報
vScreenPos		E3D_VECTOR		{}	; 修正済みスクリーン座標
vPureScreenPos		E3D_VECTOR		{}	; 無修正スクリーン座標
dwFunctionFlags		DWORD			?
dwMaskShadingFlags	DWORD			?
dwAddShadingFlags	DWORD			?
rZMinClip		REAL4			?
rZMaxClip		REAL4			?

; グローバル環境マッピングパラメータ
ALIGN	10H
envmat			E3D_REV_MATRIX		{}
genvmap			E3D_ENVIRONMENT_MAPPING	{}

; 光源情報
rgbAmbientLight		E3D_PALLET_PW		{}
rgbShadowLight		E3D_PALLET_PW		{}
nVectorLightCount	DWORD			?
pVectorLights		PE3D_VECTOR_LIGHT_ENTRY	2 DUP( ? )
nPointLightCount	DWORD			?
pPointLights		PE3D_POINT_LIGHT_ENTRY	2 DUP( ? )
rgbFogColor		E3D_PALLET_PW		{}
rFogDeepness		REAL4			?
rFogBiasZ		REAL4			?

; レイトレーシングパラメータ
rrtpRayParam		EGL_RENDER_RAY_TRACE_PARAM	{ }
rRcpShadowDistance	REAL4				?
pRayShadowing		PEGL_RENDER_MESH_MATRIX_PCK4	?
nRayShadowings		DWORD				?
ppRayShadowingList	PPEGL_RENDER_MESH_MATRIX_PCK4	?, ?
nRayShadowingListCount	DWORD				?
pRayReflection		PEGL_RENDER_MESH_MATRIX_PCK4	?
nRayReflections		DWORD				?
ppRayReflectionList	PPEGL_RENDER_MESH_MATRIX_PCK4	?, ?
nRayReflectionListCount	DWORD				?
pGlobalRayReflection	PEGL_RENDER_MESH_MATRIX_PCK4	?
nGlobalRayReflections	DWORD				?

; ソート用バッファ
pSortTable		PVOID			?
nSortTableLength	DWORD			?
pSortMeshBuffer		PVOID			?
nSortMeshBufBytes	DWORD			?

; GPGPU プラグインインターフェース
;pGPUPluginInterface		PE3D_GPU_PLUGIN_INTERFACE	?
;hGPUBufRayTracing		HE3D_GPU_PLUGIN_BUFFER		?, ?
;nAppendRayTracingCount		DWORD				?

; 一時ｚバッファ用バッファ
pTempZBuffer		PVOID			?, ?

; 簡易パラメータ描画用バッファ
hStackHeap		HSTACKHEAP		?
rpSurfAttr		E3D_SURFACE_ATTRIBUTE	{}

; メッシュ描画用ポリゴンエントリ
pMeshPolygonEntry	PE3D_POLYGON_ENTRY	?
nMeshPolygonIndex	DWORD			?
nRenderFuncType		DWORD			?
nTextureUVScale		DWORD			?
rTextureUVScale		REAL4			?

; レンダリングパラメータ
txtimg			EGL_IMAGE_INFO		{}	; 通常テクスチャ
lmnimg			EGL_IMAGE_INFO		{}	; 発光テクスチャ
pSurfaceAttr		PE3D_SURFACE_ATTRIBUTE	?	; 表面属性
pTxLineAddr		PVOID			?
pLmLineAddr		PVOID			?
nTextureApply		DWORD			?	; テクスチャ適用度
nLiminousApply		DWORD			?	; 発光テクスチャ適用度
dwShadingFlags		DWORD			?	; シェーディングフラグ
rRenderFogDeepness	REAL4			?
nRenderFogApply		SDWORD			?, ?
ALIGN	8
rgbRenderFogColor	WORD			4 DUP(?)
rgbPixelAlphaMask	QWORD			?

; テクスチャパラメータ
ALIGN	10H
vTxo			E3D_VECTOR4		{}	; 基準点
vTxx			E3D_VECTOR4		{}	; ｘ軸
vTxy			E3D_VECTOR4		{}	; ｙ軸
vTxxy			E3D_VECTOR4		{}	; [vTxx,vTxy] 外積
rTxoxy			REAL4			?	; (vTxo,vTxxy) 内積
rTxtxy			REAL4			?	; (vTxBase,vTxxy)
			REAL4			?
rTxoxyr			REAL4			?	; rTxoxy * r
rRcpTxoxyr		REAL4			?	; 1 / rTxoxyr
ALIGN	10H
vTxBase			E3D_VECTOR4		{}
vTxBasePos		E3D_VECTOR_2D		{}	; テクスチャ座標
vTxDeltaX		E3D_VECTOR_2D		{}	; ｘ座標差分
vTxDeltaY		E3D_VECTOR_2D		{}	; ｙ座標差分
rDeltaTxxy		REAL4			?	; 母係数
rRcpTxxy		REAL4			?	; 母数係数の逆数
			REAL4			?, ?

ALIGN	10H
vTxxy_d			E3D_VECTOR_DF		{}	; [vTxx,vTxy] 外積
rTxoxy_d		REAL8			?	; (vTxo,vTxxy) 内積
rTxtxy_d		REAL8			?	; (vTxBase,vTxxy)
			REAL8			?
rTxoxyr_d		REAL8			?	; rTxoxy_d * r
rRcpTxoxyr_d		REAL8			?	; 1 / rTxoxyr
ALIGN	10H
vTxBasePos_d		E3D_VECTOR_2D_DF	{}	; テクスチャ座標
vTxDeltaX_d		E3D_VECTOR_2D_DF	{}	; ｘ座標差分
vTxDeltaY_d		E3D_VECTOR_2D_DF	{}	; ｙ座標差分
rDeltaTxxy_d		REAL8			?	; 母係数
rRcpTxxy_d		REAL8			?	; 母数係数の逆数

txSizeMask		WORD			4 DUP( ? )	; サイズマスク
			WORD			4 DUP( ? )
txSizeMaskDW		EGL_SIZE		{}
lmSizeMask		WORD			4 DUP( ? )
			WORD			4 DUP( ? )
lmSizeMaskDW		EGL_SIZE		{}
txMulAddr		SWORD			4 DUP(?)
txImageAddr		DWORD			?, ?
lmMulAddr		SWORD			4 DUP(?)
lmImageAddr		DWORD			?, ?

ALIGN	10H
vTxLinePos_d		E3D_VECTOR_2D_DF	{}
rTxLineMod_d		REAL8			?
vTxLinePos		E3D_VECTOR_2D		{}	; vTxBasePos + vTxDeltaY * y
vTxNextPos		E3D_VECTOR_2D		{}
ptTexturePos		EGL_POINT		{}
rTxLineMod		REAL4			?	; rTxtxy + vTxxy.y * y
rTxNextMod		REAL4			?
yLinePos		SDWORD			?
			SDWORD			?	; dummy
nLineLeft		SDWORD			?, ?
nLineRight		SDWORD			?, ?
nLeftDecimal		DWORD			?		; x10000H FIXED
nRightDecimal		DWORD			?		; x10000H FIXED
rgbNextColor		SWORD			8 DUP( ? )	; x80H FIXED
rgbDeltaColor		SWORD			8 DUP( ? )	; x80H FIXED
vLineLeftNormal		E3D_VECTOR4_SW		{ }
vLineRightNormal	E3D_VECTOR4_SW		{ }

ALIGN	10H
vTargetPlaneParam	E3D_VECTOR4		{ }
nRenderedPixelCount	DWORD			?
nAbortRenderingFlag	DWORD			?

; 描画用ラインバッファ
pLineBuf		PVOID			?, ?
pLineVectorBuf		PE3D_VECTOR4		?, ?
pLineNormalBuf		PE3D_VECTOR4		?
pLineColorBuf		PE3D_COLOR		?
pLineTextureBuf		PEGL_PALETTE		?

; レイトレーシング用一時データ
pRenderSyncBuf		PVOID			?
yNextRenderSyncBuf	DWORD			?
rctRenderRayTracing	EGL_RECT		{ }
rmvpViewPoint		EGL_RENDER_MESH_VIEW_POINT	{ }

EGL_RENDER_POLYGON_BUF	ENDS


;
;	ポリゴン描画用関数群
; ----------------------------------------------------------------------------

;
; パラメータ計算用サブ関数
;
eglRenderPoly@SetTextureParameter486	PROTO	NEAR32 C,
	ptxmap:PTR E3D_TEXTURE_MAPINFO,
	vertex:PTR E3D_VECTOR4, uvmap:PTR E3D_VECTOR_2D

eglRenderPoly@SetTextureParameterSSE	PROTO	NEAR32 C,
	ptxmap:PTR E3D_TEXTURE_MAPINFO,
	vertex:PTR E3D_VECTOR4, uvmap:PTR E3D_VECTOR_2D
eglRenderPoly@CalcPlaneParameterSSE	PROTO	NEAR32 C

eglRenderPoly@PrepareParameterWithTextureSSE	PROTO	NEAR32 C
eglRenderPoly@PrepareParameterWithoutTextureSSE	PROTO	NEAR32 C


;
; HEGL_RENDER_POLYGON インターフェース
;
eglDrawImage@Release_Dummy	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE
eglDrawImage@Initialize_Dummy	PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE, pDstImage:PEGL_IMAGE_INFO,
	pClipRect:PCEGL_RECT, pZBuffer:PEGL_IMAGE_INFO
eglRenderPoly@SetDrawingOffset		PROTO	NEAR32 C,
	hDrawImage:HEGL_DRAW_IMAGE, pDrawOffset:PTR EGL_POINT
eglRenderPoly@Release		PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON
eglRenderPoly@Initialize	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON,
	pDstImage:PEGL_IMAGE_INFO, pClipRect:PCEGL_RECT,
	pZBuffer:PEGL_IMAGE_INFO, pScreenPos:PCE3D_VECTOR
eglRenderPoly@InitializeToReference	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON,
	hRefRenderPoly:HEGL_RENDER_POLYGON,
	pClipRect:PCEGL_RECT, dwFlags:DWORD
eglRenderPoly@GetScreenPos	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON
eglRenderPoly@GetDrawImage	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON
eglRenderPoly@GetFunctionFlags	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON
eglRenderPoly@SetFunctionFlags	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON, dwFlags:DWORD
eglRenderPoly@PrepareMatrix486	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON,
	matrix:PTR E3D_REV_MATRIX,
	pOrigin:PCE3D_VECTOR, pEnlarge:PCE3D_VECTOR
eglRenderPoly@RevolveMatrix486	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON,
	pDst:PE3D_VECTOR4, pSrc:PCE3D_VECTOR4, nVectorCount:DWORD
eglRenderPoly@PrepareMatrixSSE	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON,
	matrix:PTR E3D_REV_MATRIX,
	pOrigin:PCE3D_VECTOR, pEnlarge:PCE3D_VECTOR
eglRenderPoly@RevolveMatrixSSE	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON,
	pDst:PE3D_VECTOR4, pSrc:PCE3D_VECTOR4, nVectorCount:DWORD
eglRenderPoly@SetZClipRange	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON, rMin:REAL4, rMax:REAL4
eglRenderPoly@SetEnvironmentMapping	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON,
	matrix:PTR E3D_REV_MATRIX,
	envmap:PTR E3D_ENVIRONMENT_MAPPING
eglRenderPoly@PrepareLight	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON, hStackHeap:HSTACKHEAP,
	nLightCount:DWORD, pLightEntries:PCE3D_LIGHT_ENTRY
eglRenderPoly@CretaePolygonEntry486	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON,
	hStackHeap:HSTACKHEAP, pPrimitive:PCE3D_PRIMITIVE_POLYGON
eglRenderPoly@CretaePolygonEntrySSE	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON,
	hStackHeap:HSTACKHEAP, pPrimitive:PCE3D_PRIMITIVE_POLYGON
eglRenderPoly@MakeUpPolygon486	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON,
	hStackHeap:HSTACKHEAP, pPolyEntry:PTR E3D_POLYGON_ENTRY
eglRenderPoly@MakeUpPolygonSSE	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON,
	hStackHeap:HSTACKHEAP, pPolyEntry:PTR E3D_POLYGON_ENTRY
eglRenderPoly@ApplyAttribute486	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON,
	pPolyEntry:PTR E3D_POLYGON_ENTRY,
	pColor:PCE3D_COLOR, nTransparency:DWORD
eglRenderPoly@ApplyAttributeSSE	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON,
	pPolyEntry:PTR E3D_POLYGON_ENTRY,
	pColor:PCE3D_COLOR, nTransparency:DWORD
eglRenderPoly@GetExternalRect486	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON,
	pExtRect:PTR EGL_RECT,
	pPolyEntry:PTR PCE3D_POLYGON_ENTRY, nCount:DWORD
eglRenderPoly@GetExternalRectSSE	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON,
	pExtRect:PTR EGL_RECT,
	pPolyEntry:PTR PCE3D_POLYGON_ENTRY, nCount:DWORD
eglRenderPoly@ShadeVectors486	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON,
	pSurfaceAttribute:PCE3D_SURFACE_ATTRIBUTE,
	pNormals:PCE3D_VECTOR4, pFocusPoints:PCE3D_VECTOR4,
	pColorsLooks:PTR E3D_COLOR, nCount:DWORD
eglRenderPoly@ShadeVectorsProcSSE	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON,
	pSurfaceAttribute:PCE3D_SURFACE_ATTRIBUTE,
	pNormals:PCE3D_VECTOR4, pFocusPoints:PCE3D_VECTOR4,
	pColorsLooks:PTR E3D_COLOR, nCount:DWORD
eglRenderPoly@ProjectScreen486	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON,
	pDst:PE3D_VECTOR_2D, pSrc:PCE3D_VECTOR4, nVertexCount:DWORD
eglRenderPoly@ProjectScreenSSE	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON,
	pDst:PE3D_VECTOR_2D, pSrc:PCE3D_VECTOR4, nVertexCount:DWORD
eglRenderPoly@SortPolygonEntry486	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON,
	ppPolygons:PTR PE3D_POLYGON_ENTRY,
	nPolygonCount:DWORD, dwSortingFlags:DWORD
eglRenderPoly@PrepareRender	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON, pPolyEntry:PCE3D_POLYGON_ENTRY
eglRenderPoly@PrepareRenderSSE	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON, pPolyEntry:PCE3D_POLYGON_ENTRY
eglRenderPoly@PrepareRenderParam	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON, pRenderParam:PCE3D_RENDER_PARAM

;
; HEGL_RENDER_POLYGON インターフェース (レイトレーシング 486 ダミー関数)
;
eglRenderPoly@SetRayTracingParameter486	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON,
	prrtpParam:PTR EGL_RENDER_RAY_TRACE_PARAM
eglRenderPoly@AttachRayTracingTarget486	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON, hStackHeap:HSTACKHEAP,
	ppShadowingPoly:PTR PE3D_POLYGON_ENTRY, nShadowingCount:DWORD,
	ppRayTracingPoly:PTR PE3D_POLYGON_ENTRY, nRayTracingCount:DWORD,
	ppGlobalReflection:PTR PE3D_POLYGON_ENTRY, nGlobalReflections:DWORD
eglRenderPoly@CreatePolygonEntryRT_486	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON, hStackHeap:HSTACKHEAP,
	pPrimitive:PCE3D_PRIMITIVE_POLYGON, pdwResult:PTR DWORD
eglRenderPoly@PrepareRenderRayTracing_486	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON, pRenderRect:PCEGL_RECT
eglRenderPoly@RenderRayTracing_486	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON, hSyncRender:HEGL_RENDER_POLYGON
eglRenderPoly@AbortRenderRayTracing_486	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON
eglRenderPoly@GetProgressRayTracing_486	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON, pdwPixelCount:PTR DWORD

;
; HEGL_RENDER_POLYGON インターフェース (レイトレーシング SSE 関数)
;
eglRenderPoly@SetRayTracingParameterSSE	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON,
	prrtpParam:PTR EGL_RENDER_RAY_TRACE_PARAM
eglRenderPoly@AttachRayTracingTargetSSE	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON, hStackHeap:HSTACKHEAP,
	ppShadowingPoly:PTR PE3D_POLYGON_ENTRY, nShadowingCount:DWORD,
	ppRayTracingPoly:PTR PE3D_POLYGON_ENTRY, nRayTracingCount:DWORD,
	ppGlobalReflection:PTR PE3D_POLYGON_ENTRY, nGlobalReflections:DWORD
eglRenderPoly@CreatePolygonEntryRT_SSE	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON, hStackHeap:HSTACKHEAP,
	pPrimitive:PCE3D_PRIMITIVE_POLYGON, pdwResult:PTR DWORD
eglRenderPoly@PrepareRenderRayTracing_SSE	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON, pRenderRect:PCEGL_RECT
eglRenderPoly@RenderRayTracing_SSE	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON, hSyncRender:HEGL_RENDER_POLYGON
eglRenderPoly@AbortRenderRayTracing_SSE	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON
eglRenderPoly@GetProgressRayTracing_SSE	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON, pdwPixelCount:PTR DWORD
eglRenderPoly@AllocateRayTraceRect	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON,
	pRenderRect:PTR EGL_RECT,
	nLineCount:DWORD, nOffset:DWORD

eglRenderPoly@PerformTraceRaySSE	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON,
	pRayHitResults:PTR E3D_GPU_RAY_HIT_ENTRY,
	pRays:PTR E3D_GPU_RAY_VECTOR_ENTRY,
	pExceptionMesh:PE3D_POLYGON_ENTRY,
	nExceptingPolyIndex:DWORD,
	nReflectionCount:DWORD


;
; PrepareRender サブ関数
;
eglRenderPoly@PrepareImageBuffer	PROTO	NEAR32 C,
	pAttr:PE3D_SURFACE_ATTRIBUTE,
	pTexture:PEGL_IMAGE_INFO, pLuminousImage:PEGL_IMAGE_INFO

eglRenderPoly@SelectRenderPolygonFuncSSE	PROTO	NEAR32 C,
	pPolyEntry:PCE3D_POLYGON_ENTRY

;
; RenderPolygon 実装関数群
;
RPT_RENDER_POLY_ERROR	EQU	0
RPT_RENDER_POLY_NORMAL	EQU	1	; 通常（z 比較＋書き込み有り）
RPT_RENDER_POLY_NO_ZBUF	EQU	2	; z 比較無し
RPT_RENDER_POLY_CMP_Z	EQU	3	; z 比較のみ（通常サブ関数用サンク）
RPT_RENDER_POLY_V_NO_Z	EQU	4	; z 比較無し（通常サブ関数用サンク）

pfnRenderFuncTableSSE	TEXTEQU		<pfnRenderFuncTableSSE@RenderPolygon>
EXTERNDEF	pfnRenderFuncTableSSE:PRenderPoly@RenderPolygon

eglRenderPoly@RenderPolygon_Err	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON
eglRenderPoly@RenderPolygon_NoDraw	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON
eglRenderPoly@RenderPolygon_Image	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON
eglRenderPoly@RenderPolygon_Plane486	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON
eglRenderPoly@RenderPolygon_PlaneSSE	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON
eglRenderPoly@RenderPolygon_Texture486	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON
eglRenderPoly@RenderPolygon_TxtNZ486	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON
eglRenderPoly@RenderPolygon_TxtRZ486	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON
eglRenderPoly@RenderPolygon_TextureSSE	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON
eglRenderPoly@RenderPolygon_TxtNZSSE	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON
eglRenderPoly@RenderPolygon_TxtRZSSE	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON
eglRenderPoly@RenderPolygon_TxtVNZSSE	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON

eglRenderPoly@RenderPolygon_Mesh486	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON
eglRenderPoly@RenderPolygon_MeshSSE	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON


;
; ポリゴンラインセットアップ関数
;
eglRenderPoly@RenderPolygon_LineNTX486	PROTO	NEAR32 C	; テクスチャなし
eglRenderPoly@RenderPolygon_LineTX486	PROTO	NEAR32 C	; テクスチャ（タイリングなし）
eglRenderPoly@RenderPolygon_LineATX486	PROTO	NEAR32 C	; RGBA テクスチャ（タイリングなし）
eglRenderPoly@RenderPolygon_LineATXNS486	PROTO	NEAR32 C	; RGBA テクスチャ（タイリングなし・シェーディングなし）
eglRenderPoly@RenderPolygon_LineTTX486	PROTO	NEAR32 C	; テクスチャ
eglRenderPoly@RenderPolygon_LineTATX486	PROTO	NEAR32 C	; RGBA テクスチャ
eglRenderPoly@RenderPolygon_LineTLTX486	PROTO	NEAR32 C	; 発光テクスチャ
;
; ポリゴンラインストア関数
;
eglRenderPoly@RenderPolygon_Store486	PROTO	NEAR32 C	; 通常
eglRenderPoly@RenderPolygon_StoreT486	PROTO	NEAR32 C	; 透明度付
eglRenderPoly@RenderPolygon_StoreM486	PROTO	NEAR32 C	; マスク付
eglRenderPoly@RenderPolygon_StoreMT486	PROTO	NEAR32 C	; マスク透明度付
eglRenderPoly@RenderPolygon_StoreA486	PROTO	NEAR32 C	; アルファ付
eglRenderPoly@RenderPolygon_StoreAT486	PROTO	NEAR32 C	; アルファ透明度付
eglRenderPoly@RenderPolygon_StoreNZ486	PROTO	NEAR32 C	; 通常（ｚ無）
;
; アンチエイリアス関数
;
eglRenderPoly@RenderPolygon_Antialias486	PROTO	NEAR32 C


;
; ポリゴンラインセットアップ関数 SSE
;
eglRenderPoly@RenderPolygon_LineNTXSSE	PROTO	NEAR32 C	; テクスチャなし
eglRenderPoly@RenderPolygon_LineTXSSE	PROTO	NEAR32 C	; テクスチャ（タイリングなし）
eglRenderPoly@RenderPolygon_LineATXSSE	PROTO	NEAR32 C	; RGBA テクスチャ（タイリングなし）
eglRenderPoly@RenderPolygon_LineSXSSE	PROTO	NEAR32 C	; テクスチャ（タイリングなし補完）
eglRenderPoly@RenderPolygon_LineSAXSSE	PROTO	NEAR32 C	; RGBA テクスチャ（タイリングなし補完）
eglRenderPoly@RenderPolygon_LineTTXSSE	PROTO	NEAR32 C	; テクスチャ
eglRenderPoly@RenderPolygon_LineTATXSSE	PROTO	NEAR32 C	; RGBA テクスチャ
eglRenderPoly@RenderPolygon_LineTLTXSSE	PROTO	NEAR32 C	; 発光テクスチャ
eglRenderPoly@RenderPolygon_LineSTXSSE	PROTO	NEAR32 C	; テクスチャ（補完）
eglRenderPoly@RenderPolygon_LineSTAXSSE	PROTO	NEAR32 C	; RGBA テクスチャ（補完）
eglRenderPoly@RenderPolygon_LineSTLXSSE	PROTO	NEAR32 C	; 発光テクスチャ（補完）
;
; ポリゴンラインセットアップ関数 SSE（シェーディング無し）
;
eglRenderPoly@RenderPolygon_LineNS_ATX_SSE	PROTO	NEAR32 C	; RGBA テクスチャ（タイリングなし）
eglRenderPoly@RenderPolygon_LineNS_SAX_SSE	PROTO	NEAR32 C	; RGBA テクスチャ（タイリングなし補完）
eglRenderPoly@RenderPolygon_LineNS_TATX_SSE	PROTO	NEAR32 C	; RGBA テクスチャ
eglRenderPoly@RenderPolygon_LineNS_STAX_SSE	PROTO	NEAR32 C	; RGBA テクスチャ（補完）
;
; ポリゴンラインセットアップ関数 SSE2
;
eglRenderPoly@RenderPolygon_LineZ_SSE2		PROTO	NEAR32 C	; ｚ座標計算のみ
eglRenderPoly@RenderPolygon_LineNTXSSE2		PROTO	NEAR32 C	; テクスチャなし
eglRenderPoly@RenderPolygon_LineTXSSE2		PROTO	NEAR32 C	; テクスチャ（タイリングなし）
eglRenderPoly@RenderPolygon_LineATXSSE2		PROTO	NEAR32 C	; RGBA テクスチャ（タイリングなし）
eglRenderPoly@RenderPolygon_LineSXSSE2		PROTO	NEAR32 C	; テクスチャ（タイリングなし補完）
eglRenderPoly@RenderPolygon_LineSAXSSE2		PROTO	NEAR32 C	; RGBA テクスチャ（タイリングなし補完）
eglRenderPoly@RenderPolygon_LineTTXSSE2		PROTO	NEAR32 C	; テクスチャ
eglRenderPoly@RenderPolygon_LineTATXSSE2	PROTO	NEAR32 C	; RGBA テクスチャ
eglRenderPoly@RenderPolygon_LineTLTXSSE2	PROTO	NEAR32 C	; 発光テクスチャ
eglRenderPoly@RenderPolygon_LineSTXSSE2		PROTO	NEAR32 C	; テクスチャ（補完）
eglRenderPoly@RenderPolygon_LineSTAXSSE2	PROTO	NEAR32 C	; RGBA テクスチャ（補完）
eglRenderPoly@RenderPolygon_LineSTLXSSE2	PROTO	NEAR32 C	; 発光テクスチャ（補完）
;
; ポリゴンラインセットアップ関数 SSE2（シェーディング無し）
;
eglRenderPoly@RenderPolygon_LineNS_ATX_SSE2	PROTO	NEAR32 C	; RGBA テクスチャ（タイリングなし）
eglRenderPoly@RenderPolygon_LineNS_SAX_SSE2	PROTO	NEAR32 C	; RGBA テクスチャ（タイリングなし補完）
eglRenderPoly@RenderPolygon_LineNS_TATX_SSE2	PROTO	NEAR32 C	; RGBA テクスチャ
eglRenderPoly@RenderPolygon_LineNS_STAX_SSE2	PROTO	NEAR32 C	; RGBA テクスチャ（補完）
;
; ポリゴンラインセットアップ関数（フォンシェーディング）
;
eglRenderPoly@RenderPolygon_LineShading		PROTO	NEAR32 C
eglRenderPoly@RenderPolygon_LineShadingTexture	PROTO	NEAR32 C
eglRenderPoly@RenderPolygon_LineRayTracingTexture	PROTO	NEAR32 C
;
; ポリゴンラインストア関数
;
;eglRenderPoly@RenderPolygon_StoreSSE	PROTO	NEAR32 C	; 通常
eglRenderPoly@RenderPolygon_StoreSSEC	PROTO	NEAR32 C	; RGB 通常
;eglRenderPoly@RenderPolygon_StoreNTXSSE	PROTO	NEAR32 C	; RGBA テクスチャなし通常
eglRenderPoly@RenderPolygon_StoreTSSE	PROTO	NEAR32 C	; RGB 透明度付
eglRenderPoly@RenderPolygon_StoreMSSE	PROTO	NEAR32 C	; RGBA トリム
eglRenderPoly@RenderPolygon_StoreMTSSE	PROTO	NEAR32 C	; RGBA トリム透明度付
eglRenderPoly@RenderPolygon_StoreASSE	PROTO	NEAR32 C	; RGBA 通常
eglRenderPoly@RenderPolygon_StoreATSSE	PROTO	NEAR32 C	; RGBA 透明度付
eglRenderPoly@RenderPolygon_StoreNZSSE	PROTO	NEAR32 C	; RGB 通常（ｚ無）
eglRenderPoly@RenderPolygon_StoreTNZSSE	PROTO	NEAR32 C	; RGB 透明度付（ｚ無）
eglRenderPoly@RenderPolygon_StoreANZSSE	PROTO	NEAR32 C	; RGBA 通常（ｚ無）
eglRenderPoly@RenderPolygon_StoreARZSSE	PROTO	NEAR32 C	; RGBA 通常（ｚ比較のみ）
;
; アンチエイリアス関数
;
eglRenderPoly@RenderPolygon_AntialiasSSE	PROTO	NEAR32 C

;
; フォンシェーディング関数
;
eglRenderPoly@ShadeVectorsSSE	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON,
	pSurfaceAttribute:PCE3D_SURFACE_ATTRIBUTE,
	pNormals:PCE3D_VECTOR4, pFocusPoints:PCE3D_VECTOR4,
	pColorsLooks:PTR E3D_COLOR, nCount:DWORD,
	pTargetPlane:PE3D_VECTOR4

;
; フォンシェーディング関数（レイトレーシング）
;
eglRenderPoly@ShadeVectorsAndRayTraceSSE	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON,
	pSurfaceAttribute:PCE3D_SURFACE_ATTRIBUTE,
	pNormals:PCE3D_VECTOR4, pFocusPoints:PCE3D_VECTOR4,
	pColorsLooks:PTR E3D_COLOR, nCount:DWORD,
	pTargetPlane:PE3D_VECTOR4

eglRenderPoly@ShadeVectorsAndRaySSE	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON,
	pSurfaceAttribute:PCE3D_SURFACE_ATTRIBUTE,
	pNormals:PCE3D_VECTOR4, pFocusPoints:PCE3D_VECTOR4,
	pColorsLooks:PTR E3D_COLOR,
	prgbaTextureImage:PTR EGL_PALETTE, nCount:DWORD,
	pViewPoint:PTR EGL_RENDER_MESH_VIEW_POINT, dwReflectCount:DWORD,

eglRenderPoly@ShadeByGPURayTraceSSE	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON,
	pShadeColor:PTR E3D_COLOR,
	pRayHit:PTR E3D_GPU_RAY_HIT_ENTRY,
	nReflectionCount:DWORD, nAppendTracingCount:DWORD

;
; ポリゴンとの当たり判定
;
eglRenderPoly@Polygons@IsHitSegmentSSE	PROTO	NEAR32 C,
	pMesh:PE3D_POLYGON_ENTRY,
	prmhp:PTR EGL_RENDER_MESH_HIT_PARAM,
	pvRay:PE3D_VECTOR4, pvPos:PE3D_VECTOR4
;
; 特定のポリゴンと当たり判定を行う
;
;eglRenderPoly@Polygon@IsHitLastSegmentSSE	PROTO	NEAR32 C,
;	prmhp:PTR EGL_RENDER_MESH_HIT_PARAM,
;	pvRay:PE3D_VECTOR4, pvPos:PE3D_VECTOR4
;
; 光線とポリゴンの交差点の情報を取得する
;
eglRenderPoly@Polygon@GetHitPointSurface	PROTO	NEAR32 C,
	hRenderPoly:HEGL_RENDER_POLYGON,
	prmhs:PTR EGL_RENDER_MESH_HIT_SURFACE,
	prmhe:PTR EGL_RENDER_MESH_HIT_ENTRY
;
; メッシュとの当たり判定
;
eglRenderPoly@MeshArray@IsHitSegmentSSE	PROTO	NEAR32 C,
	prmmpMesh:PEGL_RENDER_MESH_MATRIX_PCK4, nCount:DWORD,
	prmhp:PTR EGL_RENDER_MESH_HIT_PARAM,
	pvRay:PE3D_VECTOR4, pvPos:PE3D_VECTOR4

eglRenderPoly@MeshList@IsHitSegmentSSE	PROTO	NEAR32 C,
	prmmpMesh:PTR PEGL_RENDER_MESH_MATRIX_PCK4, nQCount:DWORD,
	prmhp:PTR EGL_RENDER_MESH_HIT_PARAM,
	pvRay:PE3D_VECTOR4, pvPos:PE3D_VECTOR4
;
; 指定範囲内のメッシュを収集する
;
eglRenderPoly@MeshList@CollectMeshSSE	PROTO	NEAR32 C,
	prmmpDst:PTR PEGL_RENDER_MESH_MATRIX_PCK4,
	prmmpSrc:PEGL_RENDER_MESH_MATRIX_PCK4, nCount:DWORD,
	pvPos:PE3D_VECTOR4, rRadius:REAL4


;
;	モデル当たり判定
; ----------------------------------------------------------------------------

HEGL_MODEL_MATRIX	TYPEDEF	PTR

ModelMatrix@Release			TYPEDEF	PROTO	NEAR32 C,
	hMatrix:HEGL_MODEL_MATRIX
ModelMatrix@Initialize			TYPEDEF	PROTO	NEAR32 C,
	hMatrix:HEGL_MODEL_MATRIX,
	pModel:PTR PCE3D_PRIMITIVE_POLYGON, nPolyCount:DWORD
ModelMatrix@IsHitInclusiveSphere	TYPEDEF	PROTO	NEAR32 C,
	hMatrix:HEGL_MODEL_MATRIX,
	pSphere:PTR E3D_VECTOR, rRadius:REAL4, pHitResult:PTR SDWORD
ModelMatrix@IsHitAgainstSphere		TYPEDEF	PROTO	NEAR32 C,
	hMatrix:HEGL_MODEL_MATRIX,
	pSphere:PTR E3D_VECTOR, rRadius:REAL4,
	pHitResult:PTR SDWORD, pHitPos:PTR E3D_VECTOR,
	pHitNormal:PTR E3D_VECTOR, pReflection:PTR E3D_VECTOR
ModelMatrix@IsCrossingSegment		TYPEDEF	PROTO	NEAR32 C,
	hMatrix:HEGL_MODEL_MATRIX,
	pPos0:PTR E3D_VECTOR, pPos1:PTR E3D_VECTOR, rRadius:REAL4,
	pHitResult:PTR SDWORD, pHitPos:PTR E3D_VECTOR,
	pHitNormal:PTR E3D_VECTOR, pReflection:PTR E3D_VECTOR

PModelMatrix@Release			TYPEDEF	PTR ModelMatrix@Release
PModelMatrix@Initialize			TYPEDEF	PTR ModelMatrix@Initialize
PModelMatrix@IsHitInclusiveSphere	TYPEDEF	PTR ModelMatrix@IsHitInclusiveSphere
PModelMatrix@IsHitAgainstSphere		TYPEDEF	PTR ModelMatrix@IsHitAgainstSphere
PModelMatrix@IsCrossingSegment		TYPEDEF	PTR ModelMatrix@IsCrossingSegment

EGL_MODEL_MATRIX	STRUCT
pfnRelease		PModelMatrix@Release			?
pfnInitialize		PModelMatrix@Initialize			?
pfnIsHitInclusiveSphere	PModelMatrix@IsHitInclusiveSphere	?
pfnIsHitAgainstSphere	PModelMatrix@IsHitAgainstSphere		?
pfnIsCrossingSegment	PModelMatrix@IsCrossingSegment		?
EGL_MODEL_MATRIX	ENDS


;
;	モデル当たり判定用バッファ
; ----------------------------------------------------------------------------

EGL_POLY_MATRIX_INFO	STRUCT
vertex			E3D_VECTOR4	3 DUP( {} )	; 頂点ABC, 及び辺の長さ
				; [0].x, y, z = 頂点Aの座標
				; [0].d       = 線分ABの長さ
				; [1].d       = 線分BCの長さ
				; [2].d       = 線分ACの長さ
normal			E3D_VECTOR4	{}		; 法線
cos_abc			REAL4		?		; cos (∠ABC)
cos_acb			REAL4		?		; cos (∠ACB)
			REAL4		?, ?
EGL_POLY_MATRIX_INFO	ENDS
PEGL_POLY_MATRIX_INFO	TYPEDEF	PTR EGL_POLY_MATRIX_INFO
@POLY_MATRIX_SIZE	EQU	(SIZEOF EGL_POLY_MATRIX_INFO)

EGL_MODEL_MATRIX_BUF	STRUCT

; EGL_MODEL_MATRIX インターフェース
mmat			EGL_MODEL_MATRIX	{}

; モデル全体サイズ
vMatrixSphere		E3D_VECTOR4		{}
vMinModelPos		E3D_VECTOR4		{}
vMaxModelPos		E3D_VECTOR4		{}
fModelSized		DWORD			?

; モデル情報
nPolygonCount		DWORD			?
pPolyMatrix		PEGL_POLY_MATRIX_INFO	?
pPolyMatrixBuf		PVOID			?

EGL_MODEL_MATRIX_BUF	ENDS

;
; 当たり判定関数
;
eglCreateModelMatrix			PROTO	NEAR32 C
eglModelMatrix@Release			PROTO	NEAR32 C,
	hMatrix:HEGL_MODEL_MATRIX
eglModelMatrix@Initialize486		PROTO	NEAR32 C,
	hMatrix:HEGL_MODEL_MATRIX,
	pModel:PTR PCE3D_PRIMITIVE_POLYGON, nPolyCount:DWORD
eglModelMatrix@IsHitInclusiveSphere486	PROTO	NEAR32 C,
	hMatrix:HEGL_MODEL_MATRIX,
	pSphere:PTR E3D_VECTOR, rRadius:REAL4, pHitResult:PTR SDWORD
eglModelMatrix@IsHitAgainstSphere486	PROTO	NEAR32 C,
	hMatrix:HEGL_MODEL_MATRIX,
	pSphere:PTR E3D_VECTOR, rRadius:REAL4,
	pHitResult:PTR SDWORD, pHitPos:PTR E3D_VECTOR,
	pHitNormal:PTR E3D_VECTOR, pReflection:PTR E3D_VECTOR
eglModelMatrix@IsCrossingSegment486	PROTO	NEAR32 C,
	hMatrix:HEGL_MODEL_MATRIX,
	pPos0:PTR E3D_VECTOR, pPos1:PTR E3D_VECTOR, rErrorGap:REAL4,
	pHitResult:PTR SDWORD, pHitPos:PTR E3D_VECTOR,
	pHitNormal:PTR E3D_VECTOR, pReflection:PTR E3D_VECTOR
eglModelMatrix@InitializeSSE		PROTO	NEAR32 C,
	hMatrix:HEGL_MODEL_MATRIX,
	pModel:PTR PCE3D_PRIMITIVE_POLYGON, nPolyCount:DWORD
eglModelMatrix@IsHitInclusiveSphereSSE	PROTO	NEAR32 C,
	hMatrix:HEGL_MODEL_MATRIX,
	pSphere:PTR E3D_VECTOR, rRadius:REAL4, pHitResult:PTR SDWORD
eglModelMatrix@IsHitAgainstSphereSSE	PROTO	NEAR32 C,
	hMatrix:HEGL_MODEL_MATRIX,
	pSphere:PTR E3D_VECTOR, rRadius:REAL4,
	pHitResult:PTR SDWORD, pHitPos:PTR E3D_VECTOR,
	pHitNormal:PTR E3D_VECTOR, pReflection:PTR E3D_VECTOR
eglModelMatrix@IsCrossingSegmentSSE	PROTO	NEAR32 C,
	hMatrix:HEGL_MODEL_MATRIX,
	pPos0:PTR E3D_VECTOR, pPos1:PTR E3D_VECTOR, rErrorGap:REAL4,
	pHitResult:PTR SDWORD, pHitPos:PTR E3D_VECTOR,
	pHitNormal:PTR E3D_VECTOR, pReflection:PTR E3D_VECTOR


ENDIF

