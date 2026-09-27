
; ----------------------------------------------------------------------------
;                          Entis Graphic Library
; ----------------------------------------------------------------------------
;   Copyright (c) 2002-2008 Leshade Entis, Entis-soft. Al rights reserved.
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
xmmConstant1		REAL4	4 DUP( 1.0 )
xmmConstant4000H	REAL4	4 DUP( 16384.0 )
xmmConstant10000H	REAL4	4 DUP( 65536.0 )
realConstant1	REAL4	1.0
realConstHalf	REAL4	0.5
realConstant128	REAL4	128.0
realConstant256	REAL4	256.0
realConst32768x65536	REAL4	2147483648.0		; 8000H * 10000H
realConst65536	REAL4	65536.0
realConstDiv65536	REAL4	0.0000152587890625	; 1.0/65536.0
nConstZERO	DWORD	0
nConstFFFF	DWORD	0FFFFH
mmxConstDW_0_FF	LABEL	MMWORD
		DWORD	0, 0FFH

ALIGN	10H
tblRcpLookup	LABEL	PACK_INT_REAL
	PACK_INT_REAL	{ 0, 0.0 }, { 40000000H, 1. }
	PACK_INT_REAL	{ 20000000H, 0.5 }, { 15555555H, 0.333333333 }
	PACK_INT_REAL	{ 10000000H, 0.25 }, { 0CCCCCCCH, 0.2 }
	PACK_INT_REAL	{ 0AAAAAAAH, 0.166666667 }, { 09249249H, 0.142857143 }
	PACK_INT_REAL	{ 08000000H, 0.125 }, { 071C71C7H, 0.111111111 }
	PACK_INT_REAL	{ 06666666H, 0.1 }, { 05D1745DH, 0.090909091 }
	PACK_INT_REAL	{ 05555555H, 0.083333333 }, { 04EC4EC4H, 0.076923077 }
	PACK_INT_REAL	{ 04924924H, 0.071428571 }, { 04444444H, 0.066666667 }
	PACK_INT_REAL	{ 04000000H, 0.0625 }, { 03C3C3C3H, 0.058823529 }
	PACK_INT_REAL	{ 038E38E3H, 0.055555556 }, { 035E50D7H, 0.052631579 }
	PACK_INT_REAL	{ 03333333H, 0.05 }, { 030C30C3H, 0.047619048 }
	PACK_INT_REAL	{ 02E8BA2EH, 0.045454545 }, { 02C8590BH, 0.043478261 }
	PACK_INT_REAL	{ 02AAAAAAH, 0.041666667 }, { 028F5C28H, 0.04 }
	PACK_INT_REAL	{ 02762762H, 0.038461538 }, { 025ED097H, 0.037037037 }
	PACK_INT_REAL	{ 02492492H, 0.035714286 }, { 0234F72CH, 0.034482759 }
	PACK_INT_REAL	{ 02222222H, 0.033333333 }, { 02108421H, 0.032258065 }
	PACK_INT_REAL	{ 02000000H, 0.03125 }, { 01F07C1FH, 0.03030303 }
	PACK_INT_REAL	{ 01E1E1E1H, 0.029411765 }, { 01D41D41H, 0.028571429 }
	PACK_INT_REAL	{ 01C71C71H, 0.027777778 }, { 01BACF91H, 0.027027027 }
	PACK_INT_REAL	{ 01AF286BH, 0.026315789 }, { 01A41A41H, 0.025641026 }
	PACK_INT_REAL	{ 01999999H, 0.025 }, { 018F9C18H, 0.024390244 }
	PACK_INT_REAL	{ 01861861H, 0.023809524 }, { 017D05F4H, 0.023255814 }
	PACK_INT_REAL	{ 01745D17H, 0.022727273 }, { 016C16C1H, 0.022222222 }
	PACK_INT_REAL	{ 01642C85H, 0.02173913 }, { 015C9882H, 0.021276596 }
	PACK_INT_REAL	{ 01555555H, 0.020833333 }, { 014E5E0AH, 0.020408163 }
	PACK_INT_REAL	{ 0147AE14H, 0.02 }, { 01414141H, 0.019607843 }
	PACK_INT_REAL	{ 013B13B1H, 0.019230769 }, { 013521CFH, 0.018867925 }
	PACK_INT_REAL	{ 012F684BH, 0.018518519 }, { 0129E412H, 0.018181818 }
	PACK_INT_REAL	{ 01249249H, 0.017857143 }, { 011F7047H, 0.01754386 }
	PACK_INT_REAL	{ 011A7B96H, 0.017241379 }, { 0115B1E5H, 0.016949153 }
	PACK_INT_REAL	{ 01111111H, 0.016666667 }, { 010C9714H, 0.016393443 }
	PACK_INT_REAL	{ 01084210H, 0.016129032 }, { 01041041H, 0.015873016 }
	PACK_INT_REAL	{ 01000000H, 0.015625 }, { 00FC0FC0H, 0.015384615 }
	PACK_INT_REAL	{ 00F83E0FH, 0.015151515 }, { 00F4898DH, 0.014925373 }
	PACK_INT_REAL	{ 00F0F0F0H, 0.014705882 }, { 00ED7303H, 0.014492754 }
	PACK_INT_REAL	{ 00EA0EA0H, 0.014285714 }, { 00E6C2B4H, 0.014084507 }
	PACK_INT_REAL	{ 00E38E38H, 0.013888889 }, { 00E07038H, 0.01369863 }
	PACK_INT_REAL	{ 00DD67C8H, 0.013513514 }, { 00DA740DH, 0.013333333 }
	PACK_INT_REAL	{ 00D79435H, 0.013157895 }, { 00D4C77BH, 0.012987013 }
	PACK_INT_REAL	{ 00D20D20H, 0.012820513 }, { 00CF6474H, 0.012658228 }
	PACK_INT_REAL	{ 00CCCCCCH, 0.0125 }, { 00CA4587H, 0.012345679 }
	PACK_INT_REAL	{ 00C7CE0CH, 0.012195122 }, { 00C565C8H, 0.012048193 }
	PACK_INT_REAL	{ 00C30C30H, 0.011904762 }, { 00C0C0C0H, 0.011764706 }
	PACK_INT_REAL	{ 00BE82FAH, 0.011627907 }, { 00BC5264H, 0.011494253 }
	PACK_INT_REAL	{ 00BA2E8BH, 0.011363636 }, { 00B81702H, 0.011235955 }
	PACK_INT_REAL	{ 00B60B60H, 0.011111111 }, { 00B40B40H, 0.010989011 }
	PACK_INT_REAL	{ 00B21642H, 0.010869565 }, { 00B02C0BH, 0.010752688 }
	PACK_INT_REAL	{ 00AE4C41H, 0.010638298 }, { 00AC7691H, 0.010526316 }
	PACK_INT_REAL	{ 00AAAAAAH, 0.010416667 }, { 00A8E83FH, 0.010309278 }
	PACK_INT_REAL	{ 00A72F05H, 0.010204082 }, { 00A57EB5H, 0.01010101 }
	PACK_INT_REAL	{ 00A3D70AH, 0.01 }, { 00A237C3H, 0.00990099 }
	PACK_INT_REAL	{ 00A0A0A0H, 0.009803922 }, { 009F1165H, 0.009708738 }
	PACK_INT_REAL	{ 009D89D8H, 0.009615385 }, { 009C09C0H, 0.00952381 }
	PACK_INT_REAL	{ 009A90E7H, 0.009433962 }, { 00991F1AH, 0.009345794 }
	PACK_INT_REAL	{ 0097B425H, 0.009259259 }, { 00964FDAH, 0.009174312 }
	PACK_INT_REAL	{ 0094F209H, 0.009090909 }, { 00939A85H, 0.009009009 }
	PACK_INT_REAL	{ 00924924H, 0.008928571 }, { 0090FDBCH, 0.008849558 }
	PACK_INT_REAL	{ 008FB823H, 0.00877193 }, { 008E7835H, 0.008695652 }
	PACK_INT_REAL	{ 008D3DCBH, 0.00862069 }, { 008C08C0H, 0.008547009 }
	PACK_INT_REAL	{ 008AD8F2H, 0.008474576 }, { 0089AE40H, 0.008403361 }
	PACK_INT_REAL	{ 00888888H, 0.008333333 }, { 008767ABH, 0.008264463 }
	PACK_INT_REAL	{ 00864B8AH, 0.008196721 }, { 00853408H, 0.008130081 }
	PACK_INT_REAL	{ 00842108H, 0.008064516 }, { 0083126EH, 0.008 }
	PACK_INT_REAL	{ 00820820H, 0.007936508 }, { 00810204H, 0.007874016 }
	PACK_INT_REAL	{ 00800000H, 0.0078125 }, { 007F01FCH, 0.007751938 }
	PACK_INT_REAL	{ 007E07E0H, 0.007692308 }, { 007D1196H, 0.007633588 }
	PACK_INT_REAL	{ 007C1F07H, 0.007575758 }, { 007B301EH, 0.007518797 }
	PACK_INT_REAL	{ 007A44C6H, 0.007462687 }, { 00795CEBH, 0.007407407 }
	PACK_INT_REAL	{ 00787878H, 0.007352941 }, { 0077975BH, 0.00729927 }
	PACK_INT_REAL	{ 0076B981H, 0.007246377 }, { 0075DED9H, 0.007194245 }
	PACK_INT_REAL	{ 00750750H, 0.007142857 }, { 007432D6H, 0.007092199 }
	PACK_INT_REAL	{ 0073615AH, 0.007042254 }, { 007292CCH, 0.006993007 }
	PACK_INT_REAL	{ 0071C71CH, 0.006944444 }, { 0070FE3CH, 0.006896552 }
	PACK_INT_REAL	{ 0070381CH, 0.006849315 }, { 006F74AEH, 0.006802721 }
	PACK_INT_REAL	{ 006EB3E4H, 0.006756757 }, { 006DF5B0H, 0.006711409 }
	PACK_INT_REAL	{ 006D3A06H, 0.006666667 }, { 006C80D9H, 0.006622517 }
	PACK_INT_REAL	{ 006BCA1AH, 0.006578947 }, { 006B15C0H, 0.006535948 }
	PACK_INT_REAL	{ 006A63BDH, 0.006493506 }, { 0069B406H, 0.006451613 }
	PACK_INT_REAL	{ 00690690H, 0.006410256 }, { 00685B4FH, 0.006369427 }
	PACK_INT_REAL	{ 0067B23AH, 0.006329114 }, { 00670B45H, 0.006289308 }
	PACK_INT_REAL	{ 00666666H, 0.00625 }, { 0065C393H, 0.00621118 }
	PACK_INT_REAL	{ 006522C3H, 0.00617284 }, { 006483EDH, 0.006134969 }
	PACK_INT_REAL	{ 0063E706H, 0.006097561 }, { 00634C06H, 0.006060606 }
	PACK_INT_REAL	{ 0062B2E4H, 0.006024096 }, { 00621B97H, 0.005988024 }
	PACK_INT_REAL	{ 00618618H, 0.005952381 }, { 0060F25DH, 0.00591716 }
	PACK_INT_REAL	{ 00606060H, 0.005882353 }, { 005FD017H, 0.005847953 }
	PACK_INT_REAL	{ 005F417DH, 0.005813953 }, { 005EB488H, 0.005780347 }
	PACK_INT_REAL	{ 005E2932H, 0.005747126 }, { 005D9F73H, 0.005714286 }
	PACK_INT_REAL	{ 005D1745H, 0.005681818 }, { 005C90A1H, 0.005649718 }
	PACK_INT_REAL	{ 005C0B81H, 0.005617978 }, { 005B87DDH, 0.005586592 }
	PACK_INT_REAL	{ 005B05B0H, 0.005555556 }, { 005A84F3H, 0.005524862 }
	PACK_INT_REAL	{ 005A05A0H, 0.005494505 }, { 005987B1H, 0.005464481 }
	PACK_INT_REAL	{ 00590B21H, 0.005434783 }, { 00588FE9H, 0.005405405 }
	PACK_INT_REAL	{ 00581605H, 0.005376344 }, { 00579D6EH, 0.005347594 }
	PACK_INT_REAL	{ 00572620H, 0.005319149 }, { 0056B015H, 0.005291005 }
	PACK_INT_REAL	{ 00563B48H, 0.005263158 }, { 0055C7B4H, 0.005235602 }
	PACK_INT_REAL	{ 00555555H, 0.005208333 }, { 0054E425H, 0.005181347 }
	PACK_INT_REAL	{ 0054741FH, 0.005154639 }, { 00540540H, 0.005128205 }
	PACK_INT_REAL	{ 00539782H, 0.005102041 }, { 00532AE2H, 0.005076142 }
	PACK_INT_REAL	{ 0052BF5AH, 0.005050505 }, { 005254E7H, 0.005025126 }
	PACK_INT_REAL	{ 0051EB85H, 0.005 }, { 0051832FH, 0.004975124 }
	PACK_INT_REAL	{ 00511BE1H, 0.004950495 }, { 0050B598H, 0.004926108 }
	PACK_INT_REAL	{ 00505050H, 0.004901961 }, { 004FEC04H, 0.004878049 }
	PACK_INT_REAL	{ 004F88B2H, 0.004854369 }, { 004F2656H, 0.004830918 }
	PACK_INT_REAL	{ 004EC4ECH, 0.004807692 }, { 004E6470H, 0.004784689 }
	PACK_INT_REAL	{ 004E04E0H, 0.004761905 }, { 004DA637H, 0.004739336 }
	PACK_INT_REAL	{ 004D4873H, 0.004716981 }, { 004CEB91H, 0.004694836 }
	PACK_INT_REAL	{ 004C8F8DH, 0.004672897 }, { 004C3464H, 0.004651163 }
	PACK_INT_REAL	{ 004BDA12H, 0.00462963 }, { 004B8097H, 0.004608295 }
	PACK_INT_REAL	{ 004B27EDH, 0.004587156 }, { 004AD012H, 0.00456621 }
	PACK_INT_REAL	{ 004A7904H, 0.004545455 }, { 004A22C0H, 0.004524887 }
	PACK_INT_REAL	{ 0049CD42H, 0.004504505 }, { 00497889H, 0.004484305 }
	PACK_INT_REAL	{ 00492492H, 0.004464286 }, { 0048D159H, 0.004444444 }
	PACK_INT_REAL	{ 00487EDEH, 0.004424779 }, { 00482D1CH, 0.004405286 }
	PACK_INT_REAL	{ 0047DC11H, 0.004385965 }, { 00478BBCH, 0.004366812 }
	PACK_INT_REAL	{ 00473C1AH, 0.004347826 }, { 0046ED29H, 0.004329004 }
	PACK_INT_REAL	{ 00469EE5H, 0.004310345 }, { 0046514EH, 0.004291845 }
	PACK_INT_REAL	{ 00460460H, 0.004273504 }, { 0045B81AH, 0.004255319 }
	PACK_INT_REAL	{ 00456C79H, 0.004237288 }, { 0045217CH, 0.004219409 }
	PACK_INT_REAL	{ 0044D720H, 0.004201681 }, { 00448D63H, 0.0041841 }
	PACK_INT_REAL	{ 00444444H, 0.004166667 }, { 0043FBC0H, 0.004149378 }
	PACK_INT_REAL	{ 0043B3D5H, 0.004132231 }, { 00436C82H, 0.004115226 }
	PACK_INT_REAL	{ 004325C5H, 0.004098361 }, { 0042DF9BH, 0.004081633 }
	PACK_INT_REAL	{ 00429A04H, 0.004065041 }, { 004254FCH, 0.004048583 }
	PACK_INT_REAL	{ 00421084H, 0.004032258 }, { 0041CC98H, 0.004016064 }
	PACK_INT_REAL	{ 00418937H, 0.004 }, { 0041465FH, 0.003984064 }
	PACK_INT_REAL	{ 00410410H, 0.003968254 }, { 0040C246H, 0.003952569 }
	PACK_INT_REAL	{ 00408102H, 0.003937008 }, { 00404040H, 0.003921569 }

ConstSeg	ENDS


; ----------------------------------------------------------------------------
;	コードセグメント
; ----------------------------------------------------------------------------

CodeSeg	SEGMENT	PARA READONLY FLAT 'CODE'

;
;	リージョン生成構造体
; ----------------------------------------------------------------------------
EGL_FIXED_COLOR	STRUCT			; x128 固定小数点
Blue		SWORD	?
Green		SWORD	?
Red		SWORD	?
Reserved	SWORD	?
EGL_FIXED_COLOR	ENDS

EGL_COLOR_PARAM	STRUCT
rgbMul		EGL_FIXED_COLOR	{ }
rgbAdd		EGL_FIXED_COLOR	{ }
EGL_COLOR_PARAM	ENDS

EGL_SIDE_PARAM	STRUCT
nIndex		DWORD		?
nLeftHeight	DWORD		?
floatPosX	REAL4		?
floatDltX	REAL4		?
floatNextPosX	REAL4		?
floatNextPosY	REAL4		?
nNextPosX	SDWORD		?
nNextPosY	SDWORD		?
nLastPosX	DWORD		?
nLastPosY	DWORD		?
lastColor	E3D_COLOR	{ }
nextColor	E3D_COLOR	{ }
curColor	EGL_COLOR_PARAM	{ }
dltColor	EGL_COLOR_PARAM	{ }
lastNormal	E3D_VECTOR4	{ }
nextNormal	E3D_VECTOR4	{ }
curNormal	E3D_VECTOR4	{ }
dltNormal	E3D_VECTOR4	{ }
EGL_SIDE_PARAM	ENDS

EGL_PACKED_VERTEX	STRUCT
vector		E3D_VECTOR_2D	{ }
color		E3D_COLOR	{ }
normal		E3D_VECTOR4	{ }
EGL_PACKED_VERTEX	ENDS

;
;	多角形リージョンを生成
; ----------------------------------------------------------------------------
ALIGN	10H
eglNormalizePolygonRegion	PROC	NEAR32 C USES ebx esi edi,
	pPolyRegion:PE3D_POLYGON_REGION,
	pClipRect:PCEGL_RECT, nVertexCount:DWORD,
	pPolyVertexes:PCE3D_VECTOR_2D,
	pVertexColors:PCE3D_COLOR, pNormals:PCE3D_VECTOR4

	LOCAL	dwSaveESP:DWORD
	LOCAL	dwClipFlag:DWORD, nLoopCounter:DWORD
	LOCAL	vxLastPoint:E3D_VECTOR_2D
	LOCAL	clrLastColor:E3D_COLOR
	LOCAL	pSrcVertexes:PTR EGL_PACKED_VERTEX
	LOCAL	pDstVertexes:PTR EGL_PACKED_VERTEX
	LOCAL	pckvLastVertex:EGL_PACKED_VERTEX
	LOCAL	nVertexCount_x32:DWORD
	LOCAL	pTempVertex:PTR E3D_VECTOR_2D
	LOCAL	pTempColor:PTR E3D_COLOR
	LOCAL	pSrcNextColor:PCE3D_COLOR
	LOCAL	pDstNextColor:PTR E3D_COLOR
	LOCAL	rMinX[2]:REAL4, rMaxX[2]:REAL4
	LOCAL	rMinY[2]:REAL4, rMaxY[2]:REAL4
	LOCAL	rectClip:EGL_RECT
	LOCAL	rClipPos:REAL4, nClipPos:DWORD
	LOCAL	spSideParam1:EGL_SIDE_PARAM
	LOCAL	spSideParam2:EGL_SIDE_PARAM
	LOCAL	rgbaSide[2]:E3D_COLOR
	LOCAL	nTopLine:SDWORD
	LOCAL	fxTop_x10000H:SDWORD, nTopIndex_x32:DWORD, nTopIndex:DWORD
	LOCAL	fxTop_Left:SDWORD, fxTop_Right:SDWORD
	LOCAL	fxTop_LeftIndex_x32:SDWORD, fxTop_RightIndex_x32:SDWORD
	LOCAL	fxTop_LeftIndex:SDWORD, fxTop_RightIndex:SDWORD
	LOCAL	nCurrentY:SDWORD, dwFixedRcpY:DWORD
	LOCAL	nCurrentX[2]:SDWORD
	LOCAL	ptrNextLine:PTR E3D_POLY_LINE_REGION
	LOCAL	nTemp[4]:DWORD

	.IF	(nVertexCount < 3) || (pPolyRegion == NULL)
Label_ErrorExit:
		xor	eax, eax
		ret
	.ENDIF
	mov	dwSaveESP, esp
	;
	;	初期化
	; --------------------------------------------------------------------
	test	ERI_EnabledProcessorType, ERI_USE_XMM_P3
	jz	Label_Start486
	;
	; クリップ領域：浮動小数点形式に変換
	;
	mov	esi, pClipRect
	test	esi, esi
	jz	Label_ErrorExit
	;
	ASSUME	esi:PCEGL_RECT
	movq	mm0, MMWORD PTR [esi]
	movss		xmm7, realConstant1
	movq	mm1, MMWORD PTR [esi + 8]
	cvtsi2ss	xmm0, [esi].left
	cvtsi2ss	xmm1, [esi].right
	movq	MMWORD PTR rectClip[0], mm0
	cvtsi2ss	xmm2, [esi].top
	movq	MMWORD PTR rectClip[8], mm1
	cvtsi2ss	xmm3, [esi].bottom
	addss	xmm1, xmm7
	addss	xmm3, xmm7
	movss	rMinX, xmm0
	movss	rMaxX, xmm1
	movss	rMinY, xmm2
	movss	rMaxY, xmm3
	;
	; 入力データを EGL_PACKED_VERTEX 形式にフォーマット
	;
	IF	(SIZEOF EGL_PACKED_VERTEX) NE 32
		.ERR
	ENDIF
	@PCKVERTEX_SIZE = (SIZEOF EGL_PACKED_VERTEX)
	@PCKVERTEX_BITS = 5
	mov	ecx, nVertexCount
	shl	ecx, @PCKVERTEX_BITS		; * (SIZEOF EGL_PACKED_VERTEX)
	lea	eax, [ecx + ecx * 2]
	sub	esp, eax
	and	esp, NOT 0FH
	mov	edi, esp
	mov	pSrcVertexes, edi
	lea	eax, [edi + ecx]
	mov	pDstVertexes, eax
	;
	mov	esi, pPolyVertexes
	mov	ebx, pVertexColors
	ASSUME	esi:PCE3D_VECTOR_2D
	ASSUME	ebx:PCE3D_COLOR
	ASSUME	edi:PTR EGL_PACKED_VERTEX
	;
	mov	ecx, nVertexCount
	.IF	ebx != NULL
		xorps	xmm0, xmm0
		.REPEAT
			movq	mm0, MMWORD PTR [esi]
			movq	mm1, MMWORD PTR [ebx]
			add	esi, (SIZEOF E3D_VECTOR_2D)
			add	ebx, (SIZEOF E3D_COLOR)
			movq	MMWORD PTR [edi].vector, mm0
			movq	MMWORD PTR [edi].color, mm1
			movaps	[edi].normal, xmm0
			add	edi, (SIZEOF EGL_PACKED_VERTEX)
			dec	ecx
		.UNTIL	ZERO?
	.ELSE
		pxor	mm1, mm1
		xorps	xmm0, xmm0
		.REPEAT
			movq	mm0, MMWORD PTR [esi]
			add	esi, (SIZEOF E3D_VECTOR_2D)
			movq	MMWORD PTR [edi].vector, mm0
			movq	MMWORD PTR [edi].color, mm1
			movaps	[edi].normal, xmm0
			add	edi, (SIZEOF EGL_PACKED_VERTEX)
			dec	ecx
		.UNTIL	ZERO?
	.ENDIF
	;
	movq	MMWORD PTR pckvLastVertex.vector, mm0
	movq	MMWORD PTR pckvLastVertex.color, mm1
	;
	mov	esi, pNormals
	ASSUME	esi:PCE3D_VECTOR4
	.IF	esi != NULL
		mov	ecx, nVertexCount
		mov	edi, pSrcVertexes
		.REPEAT
			movups	xmm0, [esi]
			add	esi, (SIZEOF E3D_VECTOR4)
			movaps	xmm1, xmm0
			mulps	xmm1, xmm1
			movhlps	xmm2, xmm1
			addss	xmm2, xmm1
			shufps	xmm1, xmm1, 1
			addss	xmm2, xmm1
			sqrtss	xmm2, xmm2
			shufps	xmm2, xmm2, 0
			divps	xmm0, xmm2
			movaps	[edi].normal, xmm0
			add	edi, (SIZEOF EGL_PACKED_VERTEX)
			dec	ecx
		.UNTIL	ZERO?
	.ENDIF
	movups	pckvLastVertex.normal, xmm0
	;
	;	ｙ座標をクリップ
	; --------------------------------------------------------------------
	movss	xmm0, pckvLastVertex.vector.y
	xor	edx, edx
	comiss	xmm0, rMinY
	sbb	edx, 0
	movss	xmm1, rMaxY
	comiss	xmm1, xmm0
	adc	edx, 0
	mov	dwClipFlag, edx
	;
	mov	ecx, nVertexCount
	mov	esi, pSrcVertexes
	mov	edi, pDstVertexes
	ASSUME	esi:PTR EGL_PACKED_VERTEX
	.REPEAT
		;
		; 現在の座標のチェック
		;
		movss	xmm0, [esi].vector.y
		xor	edx, edx
		comiss	xmm0, rMinY
		mov	nLoopCounter, ecx
		sbb	edx, 0
		movss	xmm1, rMaxY
		comiss	xmm1, xmm0
		adc	edx, 0
		;
		; クリップ処理
		;
		.IF	ZERO?	; edx == 0
			.IF	dwClipFlag == edx
				call	@SubFunc_CopyVector_SSE
			.ELSE	; dwClipFlag != 0
				lea	ecx, rMinY
				lea	edx, rMaxY
				cmp	dwClipFlag, 0
				cmovg	ecx, edx
				call	@SubFunc_ClipVectorY_SSE
				mov	dwClipFlag, 0
				call	@SubFunc_CopyVector_SSE
			.ENDIF

		.ELSEIF	edx != dwClipFlag
			cmp	dwClipFlag, 0
			.IF	!ZERO?
				push	edx
				lea	ecx, rMinY
				lea	edx, rMaxY
				cmovg	ecx, edx
				call	@SubFunc_ClipVectorY_SSE
				pop	edx
			.ENDIF
			test	edx, edx
			mov	dwClipFlag, edx
			lea	ecx, rMinY
			lea	edx, rMaxY
			cmovns	ecx, edx
			call	@SubFunc_ClipVectorY_SSE
		.ENDIF
		;
		movq	mm0, MMWORD PTR [esi].vector
		movq	mm1, MMWORD PTR [esi].color
		movaps	xmm0, [esi].normal
		mov	ecx, nLoopCounter
		add	esi, (SIZEOF EGL_PACKED_VERTEX)
		movq	MMWORD PTR pckvLastVertex.vector, mm0
		movq	MMWORD PTR pckvLastVertex.color, mm1
		movups	pckvLastVertex.normal, xmm0
		;
		dec	ecx
	.UNTIL	ZERO?
	;
	mov	esi, pDstVertexes
	sub	edi, esi
	shr	edi, @PCKVERTEX_BITS
	mov	nVertexCount, edi
	.IF	edi < 3
Label_ErrorExit2:
		emms
		mov	esp, dwSaveESP
		xor	eax, eax
		ret
	.ENDIF
	;
	;	ｘ座標をクリップ
	; --------------------------------------------------------------------
	mov	eax, edi
	shl	edi, @PCKVERTEX_BITS + 1	; * (SIZEOF EGL_PACKED_VERTEX) * 2
	mov	pSrcVertexes, esi
	sub	esp, edi
	and	esp, NOT 0FH
	mov	pDstVertexes, esp
	mov	edi, esp
	;
	mov	ecx, eax
	shl	eax, @PCKVERTEX_BITS
	movq	mm0, MMWORD PTR [esi + eax - @PCKVERTEX_SIZE].vector
	movq	mm1, MMWORD PTR [esi + eax - @PCKVERTEX_SIZE].color
	movaps	xmm0, [esi + eax - @PCKVERTEX_SIZE].normal
	movq	MMWORD PTR pckvLastVertex.vector, mm0
	movq	MMWORD PTR pckvLastVertex.color, mm1
	movups	pckvLastVertex.normal, xmm0
	;
	movss	xmm0, pckvLastVertex.vector.x
	xor	edx, edx
	comiss	xmm0, rMinX
	sbb	edx, 0
	movss	xmm1, rMaxX
	comiss	xmm1, xmm0
	adc	edx, 0
	mov	dwClipFlag, edx
	;
	.REPEAT
		;
		; 現在の座標のチェック
		;
		movss	xmm0, [esi].vector.x
		xor	edx, edx
		comiss	xmm0, rMinX
		mov	nLoopCounter, ecx
		sbb	edx, 0
		movss	xmm1, rMaxX
		comiss	xmm1, xmm0
		adc	edx, 0
		;
		; クリップ処理
		;
		.IF	ZERO?	; edx == 0
			.IF	dwClipFlag == edx
				call	@SubFunc_CopyVector_SSE
			.ELSE	; dwClipFlag != 0
				lea	ecx, rMinX
				lea	edx, rMaxX
				cmp	dwClipFlag, 0
				cmovg	ecx, edx
				call	@SubFunc_ClipVectorX_SSE
				mov	dwClipFlag, 0
				call	@SubFunc_CopyVector_SSE
			.ENDIF

		.ELSEIF	edx != dwClipFlag
			cmp	dwClipFlag, 0
			.IF	!ZERO?
				push	edx
				lea	ecx, rMinX
				lea	edx, rMaxX
				cmovg	ecx, edx
				call	@SubFunc_ClipVectorX_SSE
				pop	edx
			.ENDIF
			test	edx, edx
			mov	dwClipFlag, edx
			lea	ecx, rMinX
			lea	edx, rMaxX
			cmovns	ecx, edx
			call	@SubFunc_ClipVectorX_SSE
		.ENDIF
		;
		movq	mm0, MMWORD PTR [esi].vector
		movq	mm1, MMWORD PTR [esi].color
		movaps	xmm0, [esi].normal
		mov	ecx, nLoopCounter
		add	esi, (SIZEOF EGL_PACKED_VERTEX)
		movq	MMWORD PTR pckvLastVertex.vector, mm0
		movq	MMWORD PTR pckvLastVertex.color, mm1
		movups	pckvLastVertex.normal, xmm0
		;
		dec	ecx
	.UNTIL	ZERO?
	;
	mov	esi, pDstVertexes
	sub	edi, esi
	shr	edi, @PCKVERTEX_BITS
	mov	nVertexCount, edi
	cmp	edi, 3
	jb	Label_ErrorExit2
	;
	;	リージョン作成
	; --------------------------------------------------------------------
	;
	; 座標を整数に変換
	;
	mov	ecx, edi
	movss	xmm4, realConst65536
	sub	ecx, 2
	shufps	xmm4, xmm4, 00000000B
	.WHILE	!SIGN?
		movlps		xmm0, QWORD PTR [esi].vector
		movhps		xmm0, QWORD PTR [esi + @PCKVERTEX_SIZE].vector
		mulps		xmm0, xmm4
		cvtps2pi	mm0, xmm0
		shufps		xmm0, xmm0, 01001110B
		cvtps2pi	mm1, xmm0
		movq		MMWORD PTR [esi].vector, mm0
		movq		MMWORD PTR [esi + @PCKVERTEX_SIZE].vector, mm1
		add		esi, (SIZEOF EGL_PACKED_VERTEX) * 2
		sub		ecx, 2
	.ENDW
	add	ecx, 2
	.IF	!ZERO?
		movlps		xmm0, QWORD PTR [esi].vector
		mulps		xmm0, xmm4
		cvtps2pi	mm0, xmm0
		movq		MMWORD PTR [esi].vector, mm0
	.ENDIF
	;
	; 最小ｙ座標を持つ頂点を取得
	;
	mov	esi, pDstVertexes
	ASSUME	esi:PTR EGL_PACKED_VERTEX
	mov	edi, nVertexCount
	xor	ecx, ecx
	mov	eax, 7FFFFFFFH
	xor	edx, edx
	shl	edi, @PCKVERTEX_BITS
	.REPEAT
		mov	ebx, [esi + ecx].vector.y
		cmp	ebx, eax
		cmovl	eax, ebx
		cmovl	edx, ecx
		add	ecx, (SIZEOF EGL_PACKED_VERTEX)
	.UNTIL	ecx >= edi
	;
	; 初期座標設定
	;
	mov	fxTop_x10000H, eax
	mov	nTopIndex_x32, edx
	sar	eax, 16
	mov	nCurrentY, eax
	xor	eax, eax
	mov	nVertexCount_x32, edi
	mov	spSideParam1.nIndex, edx
	mov	spSideParam1.nLeftHeight, eax
	mov	spSideParam2.nIndex, edx
	mov	spSideParam2.nLeftHeight, eax
	;
	lea	edi, spSideParam1
	call	@SubFunc_CopySideParam
	lea	edi, spSideParam2
	call	@SubFunc_CopySideParam
	;
	inc	nCurrentY
	mov	ecx, spSideParam1.nIndex
	mov	esi, pDstVertexes
	call	@SubFunc_NextSide1First
	jnz	@Label_1LineException
	;
	mov	ecx, spSideParam2.nIndex
	mov	esi, pDstVertexes
	call	@SubFunc_NextSide2First
	jnz	Label_ErrorExit2
	;
	; リージョン作成開始
	;
	mov	edi, pPolyRegion
	ASSUME	edi:PTR E3D_POLYGON_REGION
	lea	edi, [edi].plrLineRgn[0]
	ASSUME	edi:PTR E3D_POLY_LINE_REGION
	mov	ptrNextLine, edi
	;
	; １ライン目処理
	;
	mov	ecx, nCurrentY
	movzx	eax, WORD PTR fxTop_x10000H
	lea	edx, [ecx - 1]
	mov	nTopLine, ecx
	.IF	(eax == 0) && (edx == rectClip.top)
		dec	ecx
		.IF	((SDWORD PTR ecx) >= rectClip.top) \
				&& ((SDWORD PTR ecx) <= rectClip.bottom)
			mov	nTopLine, ecx
			;
			; １ライン目の幅を取得する
			;
			mov	esi, pDstVertexes
			ASSUME	esi:PTR EGL_PACKED_VERTEX
			mov	eax, nTopIndex_x32
			mov	ebx, eax
			mov	ecx, [esi + eax].vector.x
			mov	fxTop_LeftIndex_x32, eax
			mov	fxTop_RightIndex_x32, eax
			mov	fxTop_Left, ecx
			mov	fxTop_Right, ecx
			;
			mov	edx, nTopLine
			mov	edi, nVertexCount
			shl	edx, 16
			xor	ecx, ecx
			shl	edi, @PCKVERTEX_BITS
			.REPEAT
				.IF	(SDWORD PTR [esi + ecx].vector.y) <= (SDWORD PTR edx)
					mov	eax, [esi + ecx].vector.x
					.IF	(SDWORD PTR eax) < fxTop_Left
						mov	fxTop_Left, eax
						mov	fxTop_LeftIndex_x32, ecx
					.ELSEIF	fxTop_Right < (SDWORD PTR eax)
						mov	fxTop_Right, eax
						mov	fxTop_RightIndex_x32, ecx
					.ENDIF
				.ENDIF
				add	ecx, (SIZEOF EGL_PACKED_VERTEX)
			.UNTIL	ecx >= edi
			;
			; ラインを出力する
			;
			mov	ecx, fxTop_LeftIndex_x32
			mov	edx, fxTop_RightIndex_x32
				movups	xmm0, [esi + ecx].normal
				movups	xmm1, [esi + edx].normal
				movaps	xmm2, xmmConstant4000H
			movq	mm0, MMWORD PTR [esi + ecx].color
			movq	mm1, MMWORD PTR [esi + edx].color
				mulps	xmm0, xmm2
				mulps	xmm1, xmm2
			mov	eax, [esi + ecx].vector.x
			mov	edx, [esi + edx].vector.x
				cvtps2pi	mm4, xmm0
				cvtps2pi	mm5, xmm1
				movhlps		xmm0, xmm0
				movhlps		xmm1, xmm1
				cvtps2pi	mm6, xmm0
				cvtps2pi	mm7, xmm1
				packssdw	mm4, mm6
				packssdw	mm5, mm7
			ASSUME	esi:NOTHING
			;
			dec	edx
			movzx	ecx, ax
			movzx	ebx, dx
			sar	eax, 16
			sar	edx, 16
			;
			.IF	(SDWORD PTR eax) > rectClip.right
				mov	eax, rectClip.right
				xor	ecx, ecx
				mov	edx, eax
				mov	ebx, 0FFFFH
			.ELSEIF	(SDWORD PTR edx) < rectClip.left
				mov	eax, rectClip.left
				xor	ecx, ecx
				mov	edx, eax
				mov	ebx, 0FFFFH
			.ELSE
				.IF	(SDWORD PTR eax) < rectClip.left
					mov	eax, rectClip.left
					xor	ecx, ecx
				.ENDIF
				.IF	(SDWORD PTR edx) > rectClip.right
					mov	edx, rectClip.right
					mov	ebx, 0FFFFH
				.ENDIF
			.ENDIF
			;
			mov	edi, ptrNextLine
			mov	[edi].nLeft, eax
			mov	[edi].dwReserved1, ecx
			mov	[edi].nRight, edx
			mov	[edi].dwReserved2, ebx
			movq	MMWORD PTR [edi].rgbaLeft, mm0
			movq	MMWORD PTR [edi].rgbaRight, mm1
			movq	MMWORD PTR [edi].vLeft, mm4
			movq	MMWORD PTR [edi].vRight, mm5
			add	edi, (SIZEOF E3D_POLY_LINE_REGION)
		.ENDIF
	.ENDIF
	;
	movss	xmm4, spSideParam1.floatPosX
	movss	xmm5, spSideParam2.floatPosX
	movss	xmm6, spSideParam1.floatDltX
	movss	xmm7, spSideParam2.floatDltX
	movq	mm4, MMWORD PTR spSideParam1.curColor.rgbMul
	movq	mm5, MMWORD PTR spSideParam1.curColor.rgbAdd
	movq	mm6, MMWORD PTR spSideParam2.curColor.rgbMul
	movq	mm7, MMWORD PTR spSideParam2.curColor.rgbAdd
	;
	mov	eax, nCurrentY
	cmp	eax, rectClip.bottom
	mov	ptrNextLine, edi
	jg	@Label_RegionFinished
	;
	cmp	pNormals, NULL
	jnz	@Label_RegionLoop_WithNormal_SSE
@Label_RegionLoop_SSE:
		;
		; 座標・色情報正規化
		;	座標差分処理
		;	色差分処理
		;
			movq	mm0, mm4
			movq	mm1, mm5
		cvtss2si	ecx, xmm4
		addss	xmm4, xmm6
			movq	mm2, mm6
			movq	mm3, mm7
		cvtss2si	edx, xmm5
		addss	xmm5, xmm7
			psraw	mm0, 7
		movzx	eax, cx
			psraw	mm1, 7
		movzx	esi, dx
			psraw	mm2, 7
			psraw	mm3, 7
		sar	ecx, 16
			packuswb	mm0, mm1
		sar	edx, 16
			packuswb	mm2, mm3
		;
		; ライン情報書き出し
		;
		mov	ebx, rectClip.left
		xor	edi, edi
		cmp	ecx, ebx
		cmovl	ecx, ebx
			paddsw	mm4, MMWORD PTR spSideParam1.dltColor.rgbMul
		cmovl	eax, edi
			paddsw	mm5, MMWORD PTR spSideParam1.dltColor.rgbAdd
		cmp	edx, ebx
		cmovl	edx, ebx
		mov	ebx, rectClip.right
		cmovl	esi, edi
		mov	edi, 0FFFFH
		cmp	ecx, ebx
		cmovg	ecx, ebx
			paddsw	mm6, MMWORD PTR spSideParam2.dltColor.rgbMul
		cmovg	eax, edi
			paddsw	mm7, MMWORD PTR spSideParam2.dltColor.rgbAdd
		cmp	edx, ebx
		cmovg	edx, ebx
		cmovg	esi, edi
		;
		mov	edi, ptrNextLine
		.IF	(SDWORD PTR ecx) <= (SDWORD PTR edx)
			.IF	!ZERO?
@@:				mov	[edi].nLeft, ecx
				mov	[edi].dwReserved1, eax
;#				sub	si, 1
;#				sbb	edx, 0
				mov	[edi].nRight, edx
				mov	[edi].dwReserved2, esi
				movq	MMWORD PTR [edi].rgbaLeft, mm0
				movq	MMWORD PTR [edi].rgbaRight, mm2
			.ELSE
				cmp	eax, esi
				jbe	@b
				jmp	@f
			.ENDIF
		.ELSE
@@:			mov	[edi].nLeft, edx
			mov	[edi].dwReserved1, esi
;#			sub	ax, 1
;#			sbb	ecx, 0
			mov	[edi].nRight, ecx
			mov	[edi].dwReserved2, eax
			movq	MMWORD PTR [edi].rgbaLeft, mm2
			movq	MMWORD PTR [edi].rgbaRight, mm0
		.ENDIF
		;
		; ｙ座標更新
		;
		mov	eax, nCurrentY
			xor	ecx, ecx
			cmp	eax, rectClip.top
			setge	cl
		inc	eax
			neg	ecx
		mov	nCurrentY, eax
			and	ecx, (SIZEOF E3D_POLY_LINE_REGION)
		add	edi, ecx
			cmp	eax, rectClip.bottom
		mov	ptrNextLine, edi
			jg	@Label_RegionFinished
		;
		; 座標差分処理
		; 色差分処理
		;
;		addss	xmm4, xmm6
;			movq	mm0, MMWORD PTR spSideParam1.dltColor.rgbMul
;			movq	mm1, MMWORD PTR spSideParam1.dltColor.rgbAdd
;		addss	xmm5, xmm7
;			movq	mm2, MMWORD PTR spSideParam2.dltColor.rgbMul
;			movq	mm3, MMWORD PTR spSideParam2.dltColor.rgbAdd
;			paddsw	mm4, mm0
;			paddsw	mm5, mm1
;			paddsw	mm6, mm2
;			paddsw	mm7, mm3
		;
		; 頂点更新
		;
		mov	eax, spSideParam1.nLeftHeight
		mov	ecx, spSideParam1.nIndex
		dec	eax
		mov	esi, pDstVertexes
		mov	spSideParam1.nLeftHeight, eax
		.IF	ZERO?
			movss	spSideParam1.floatPosX, xmm4
			movss	spSideParam2.floatPosX, xmm5
			movq	MMWORD PTR spSideParam1.curColor.rgbMul, mm4
			movq	MMWORD PTR spSideParam1.curColor.rgbAdd, mm5
			movq	MMWORD PTR spSideParam2.curColor.rgbMul, mm6
			movq	MMWORD PTR spSideParam2.curColor.rgbAdd, mm7
			;
			call	@SubFunc_NextSide1
			jnz	@Label_FinishRegion
			;
			movss	xmm4, spSideParam1.floatPosX
			movss	xmm5, spSideParam2.floatPosX
			movss	xmm6, spSideParam1.floatDltX
			movss	xmm7, spSideParam2.floatDltX
			movq	mm4, MMWORD PTR spSideParam1.curColor.rgbMul
			movq	mm5, MMWORD PTR spSideParam1.curColor.rgbAdd
			movq	mm6, MMWORD PTR spSideParam2.curColor.rgbMul
			movq	mm7, MMWORD PTR spSideParam2.curColor.rgbAdd
		.ENDIF
		;
		mov	eax, spSideParam2.nLeftHeight
		mov	ecx, spSideParam2.nIndex
		dec	eax
		mov	esi, pDstVertexes
		mov	spSideParam2.nLeftHeight, eax
		.IF	ZERO?
			movss	spSideParam1.floatPosX, xmm4
			movss	spSideParam2.floatPosX, xmm5
			movq	MMWORD PTR spSideParam1.curColor.rgbMul, mm4
			movq	MMWORD PTR spSideParam1.curColor.rgbAdd, mm5
			movq	MMWORD PTR spSideParam2.curColor.rgbMul, mm6
			movq	MMWORD PTR spSideParam2.curColor.rgbAdd, mm7
			;
			call	@SubFunc_NextSide2
			jnz	@Label_FinishRegion
			;
			movss	xmm4, spSideParam1.floatPosX
			movss	xmm5, spSideParam2.floatPosX
			movss	xmm6, spSideParam1.floatDltX
			movss	xmm7, spSideParam2.floatDltX
			movq	mm4, MMWORD PTR spSideParam1.curColor.rgbMul
			movq	mm5, MMWORD PTR spSideParam1.curColor.rgbAdd
			movq	mm6, MMWORD PTR spSideParam2.curColor.rgbMul
			movq	mm7, MMWORD PTR spSideParam2.curColor.rgbAdd
		.ENDIF
	jmp	@Label_RegionLoop_SSE

ALIGN	10H
@Label_RegionLoop_WithNormal_SSE:
		;
		; 座標・色情報正規化
		;	座標差分処理
		;	色差分処理
		;
			movq	mm0, mm4
			movq	MMWORD PTR spSideParam1.curColor.rgbMul, mm4
			movq	mm1, mm5
		cvtss2si	ecx, xmm4
		addss	xmm4, xmm6
			movq	mm2, mm6
			movq	mm3, mm7
		cvtss2si	edx, xmm5
		addss	xmm5, xmm7
			psraw	mm0, 7
		movzx	eax, cx
			psraw	mm1, 7
		movzx	esi, dx
			psraw	mm2, 7
				movups		xmm0, spSideParam1.curNormal
			psraw	mm3, 7
				movups		xmm1, spSideParam2.curNormal
		sar	ecx, 16
			packuswb	mm0, mm1
				cvtps2pi	mm1, xmm0
				movhlps		xmm2, xmm0
		sar	edx, 16
			packuswb	mm2, mm3
				cvtps2pi	mm3, xmm2
				movups		xmm2, spSideParam1.dltNormal
				movhlps		xmm3, xmm1
				packssdw	mm1, mm3
				cvtps2pi	mm3, xmm1
				cvtps2pi	mm4, xmm3
				movups		xmm3, spSideParam2.dltNormal
				addps		xmm0, xmm2
				addps		xmm1, xmm3
				packssdw	mm3, mm4
			movq	mm4, MMWORD PTR spSideParam1.curColor.rgbMul
				movups		spSideParam1.curNormal, xmm0
				movups		spSideParam2.curNormal, xmm1
		;
		; ライン情報書き出し
		;
		mov	ebx, rectClip.left
		xor	edi, edi
		cmp	ecx, ebx
		cmovl	ecx, ebx
			paddsw	mm4, MMWORD PTR spSideParam1.dltColor.rgbMul
		cmovl	eax, edi
			paddsw	mm5, MMWORD PTR spSideParam1.dltColor.rgbAdd
		cmp	edx, ebx
		cmovl	edx, ebx
		mov	ebx, rectClip.right
		cmovl	esi, edi
		mov	edi, 0FFFFH
		cmp	ecx, ebx
		cmovg	ecx, ebx
			paddsw	mm6, MMWORD PTR spSideParam2.dltColor.rgbMul
		cmovg	eax, edi
			paddsw	mm7, MMWORD PTR spSideParam2.dltColor.rgbAdd
		cmp	edx, ebx
		cmovg	edx, ebx
		cmovg	esi, edi
		;
		mov	edi, ptrNextLine
		.IF	(SDWORD PTR ecx) <= (SDWORD PTR edx)
			.IF	!ZERO?
@@:				mov	[edi].nLeft, ecx
				mov	[edi].dwReserved1, eax
;#				sub	si, 1
;#				sbb	edx, 0
				mov	[edi].nRight, edx
				mov	[edi].dwReserved2, esi
				movq	MMWORD PTR [edi].rgbaLeft, mm0
				movq	MMWORD PTR [edi].rgbaRight, mm2
				movq	MMWORD PTR [edi].vLeft, mm1
				movq	MMWORD PTR [edi].vRight, mm3
			.ELSE
				cmp	eax, esi
				jbe	@b
				jmp	@f
			.ENDIF
		.ELSE
@@:			mov	[edi].nLeft, edx
			mov	[edi].dwReserved1, esi
;#			sub	ax, 1
;#			sbb	ecx, 0
			mov	[edi].nRight, ecx
			mov	[edi].dwReserved2, eax
			movq	MMWORD PTR [edi].rgbaLeft, mm2
			movq	MMWORD PTR [edi].rgbaRight, mm0
			movq	MMWORD PTR [edi].vLeft, mm3
			movq	MMWORD PTR [edi].vRight, mm1
		.ENDIF
		;
		; ｙ座標更新
		;
		mov	eax, nCurrentY
			xor	ecx, ecx
			cmp	eax, rectClip.top
			setge	cl
		inc	eax
			neg	ecx
		mov	nCurrentY, eax
			and	ecx, (SIZEOF E3D_POLY_LINE_REGION)
		add	edi, ecx
			cmp	eax, rectClip.bottom
		mov	ptrNextLine, edi
			jg	@Label_RegionFinished
		;
		; 頂点更新
		;
		mov	eax, spSideParam1.nLeftHeight
		mov	ecx, spSideParam1.nIndex
		dec	eax
		mov	esi, pDstVertexes
		mov	spSideParam1.nLeftHeight, eax
		.IF	ZERO?
			movss	spSideParam1.floatPosX, xmm4
			movss	spSideParam2.floatPosX, xmm5
			movq	MMWORD PTR spSideParam1.curColor.rgbMul, mm4
			movq	MMWORD PTR spSideParam1.curColor.rgbAdd, mm5
			movq	MMWORD PTR spSideParam2.curColor.rgbMul, mm6
			movq	MMWORD PTR spSideParam2.curColor.rgbAdd, mm7
			;
			call	@SubFunc_NextSide1
			jnz	@Label_FinishRegion
			;
			movss	xmm4, spSideParam1.floatPosX
			movss	xmm5, spSideParam2.floatPosX
			movss	xmm6, spSideParam1.floatDltX
			movss	xmm7, spSideParam2.floatDltX
			movq	mm4, MMWORD PTR spSideParam1.curColor.rgbMul
			movq	mm5, MMWORD PTR spSideParam1.curColor.rgbAdd
			movq	mm6, MMWORD PTR spSideParam2.curColor.rgbMul
			movq	mm7, MMWORD PTR spSideParam2.curColor.rgbAdd
		.ENDIF
		;
		mov	eax, spSideParam2.nLeftHeight
		mov	ecx, spSideParam2.nIndex
		dec	eax
		mov	esi, pDstVertexes
		mov	spSideParam2.nLeftHeight, eax
		.IF	ZERO?
			movss	spSideParam1.floatPosX, xmm4
			movss	spSideParam2.floatPosX, xmm5
			movq	MMWORD PTR spSideParam1.curColor.rgbMul, mm4
			movq	MMWORD PTR spSideParam1.curColor.rgbAdd, mm5
			movq	MMWORD PTR spSideParam2.curColor.rgbMul, mm6
			movq	MMWORD PTR spSideParam2.curColor.rgbAdd, mm7
			;
			call	@SubFunc_NextSide2
			jnz	@Label_FinishRegion
			;
			movss	xmm4, spSideParam1.floatPosX
			movss	xmm5, spSideParam2.floatPosX
			movss	xmm6, spSideParam1.floatDltX
			movss	xmm7, spSideParam2.floatDltX
			movq	mm4, MMWORD PTR spSideParam1.curColor.rgbMul
			movq	mm5, MMWORD PTR spSideParam1.curColor.rgbAdd
			movq	mm6, MMWORD PTR spSideParam2.curColor.rgbMul
			movq	mm7, MMWORD PTR spSideParam2.curColor.rgbAdd
		.ENDIF
	jmp	@Label_RegionLoop_WithNormal_SSE

@Label_FinishRegion:
IF	0
	;
	; 最終ライン出力
	;
	movzx	eax, WORD PTR spSideParam1.nNextPosY
	movzx	edx, WORD PTR spSideParam2.nNextPosY
	mov	ecx, nCurrentY
	and	eax, edx
	.IF	eax > 8000H
		inc	ecx
		mov	eax, spSideParam1.nNextPosX
		mov	edx, spSideParam2.nNextPosX
		mov	nCurrentY, ecx
		movq	mm0, MMWORD PTR spSideParam1.nextColor
		movq	mm1, MMWORD PTR spSideParam2.nextColor
		;
		.IF	(SDWORD PTR eax) > (SDWORD PTR edx)
			mov	ecx, eax
			movq	mm2, mm0
			mov	eax, edx
			movq	mm0, mm1
			mov	edx, ecx
			movq	mm1, mm2
		.ENDIF
		dec	edx
		movzx	ecx, ax
		movzx	ebx, dx
		sar	eax, 16
		sar	edx, 16
		;
		mov	edi, ptrNextLine
		.IF	(SDWORD PTR eax) > rectClip.right
			mov	eax, rectClip.right
			xor	ecx, ecx
			mov	edx, eax
			mov	ebx, 0FFFFH
		.ELSEIF	(SDWORD PTR edx) < rectClip.left
			mov	eax, rectClip.left
			xor	ecx, ecx
			mov	edx, eax
			mov	ebx, 0FFFFH
		.ELSE
			.IF	(SDWORD PTR eax) < rectClip.left
				mov	eax, rectClip.left
				xor	ecx, ecx
			.ENDIF
			.IF	(SDWORD PTR edx) > rectClip.right
				mov	edx, rectClip.right
				mov	ebx, 0FFFFH
			.ENDIF
		.ENDIF
		;
		mov	[edi].nLeft, eax
		mov	[edi].dwReserved1, ecx
		mov	[edi].nRight, edx
		mov	[edi].dwReserved2, ebx
		movq	MMWORD PTR [edi].rgbaLeft, mm0
		movq	MMWORD PTR [edi].rgbaRight, mm1
	.ENDIF
ENDIF

@Label_RegionFinished:
	;
	; 有効幅決定
	;
	mov	edi, pPolyRegion
	ASSUME	edi:PTR E3D_POLYGON_REGION
	mov	eax, nTopLine
	mov	edx, nCurrentY
	dec	edx
	.IF	(SDWORD PTR eax) < rectClip.top
		mov	eax, rectClip.top
	.ELSEIF	(SDWORD PTR eax) > rectClip.bottom
		mov	pPolyRegion, NULL
	.ENDIF
	.IF	((SDWORD PTR edx) < rectClip.top) \
			|| ((SDWORD PTR edx) < (SDWORD PTR eax))
		mov	pPolyRegion, NULL
	.ENDIF
	mov	[edi].nTopLine, eax
	mov	[edi].nBottomLine, edx
	ASSUME	edi:NOTHING

@Label_Exit:
	emms
	mov	esp, dwSaveESP
	mov	eax, pPolyRegion
	ret


;
;	１行間だけのリージョン作成
; ----------------------------------------------------------------------------
@Label_1LineException:
	mov	eax, fxTop_x10000H
	sar	eax, 16
	mov	nTopLine, eax
	;
	; １ライン目の幅を取得する
	;
	mov	esi, pDstVertexes
	ASSUME	esi:PTR EGL_PACKED_VERTEX
	mov	eax, nTopIndex_x32
	mov	ebx, eax
	mov	ecx, [esi + eax].vector.x
	mov	fxTop_LeftIndex_x32, eax
	mov	fxTop_RightIndex_x32, eax
	mov	fxTop_Left, ecx
	mov	fxTop_Right, ecx
	;
	mov	edx, nTopLine
	mov	edi, nVertexCount
	shl	edx, 16
	xor	ecx, ecx
	shl	edi, @PCKVERTEX_BITS
	.REPEAT
		.IF	(SDWORD PTR [esi + ecx].vector.y) <= (SDWORD PTR edx)
			mov	eax, [esi + ecx].vector.x
			.IF	(SDWORD PTR eax) < fxTop_Left
				mov	fxTop_Left, eax
				mov	fxTop_LeftIndex_x32, ecx
			.ELSEIF	fxTop_Right < (SDWORD PTR eax)
				mov	fxTop_Right, eax
				mov	fxTop_RightIndex_x32, ecx
			.ENDIF
		.ENDIF
		add	ecx, (SIZEOF EGL_PACKED_VERTEX)
	.UNTIL	ecx >= edi
	;
	xor	ecx, ecx
	.REPEAT
		.BREAK	.IF	(SDWORD PTR [esi + ecx].vector.y) > (SDWORD PTR edx)
		add	ecx, (SIZEOF EGL_PACKED_VERTEX)
	.UNTIL	ecx >= edi
	cmp	ecx, edi
	jae	Label_ErrorExit2
	;
	; ラインを出力する
	;
	mov	eax, nTopLine
	cmp	eax, rectClip.bottom
	jg	Label_ErrorExit2
	cmp	eax, rectClip.top
	jl	Label_ErrorExit2
	mov	edi, pPolyRegion
	ASSUME	edi:PTR E3D_POLYGON_REGION
	mov	[edi].nTopLine, eax
	mov	[edi].nBottomLine, eax
	lea	edi, [edi].plrLineRgn[0]
	ASSUME	edi:PTR E3D_POLY_LINE_REGION
	;
	mov	ecx, fxTop_LeftIndex_x32
	mov	edx, fxTop_RightIndex_x32
		movups	xmm0, [esi + ecx].normal
		movups	xmm1, [esi + edx].normal
		movaps	xmm2, xmmConstant4000H
	movq	mm0, MMWORD PTR [esi + ecx].color
	movq	mm1, MMWORD PTR [esi + edx].color
		mulps	xmm0, xmm2
		mulps	xmm1, xmm2
	mov	eax, [esi + ecx].vector.x
	mov	edx, [esi + edx].vector.x
		cvtps2pi	mm4, xmm0
		cvtps2pi	mm5, xmm1
		movhlps		xmm0, xmm0
		movhlps		xmm1, xmm1
		cvtps2pi	mm6, xmm0
		cvtps2pi	mm7, xmm1
		packssdw	mm4, mm6
		packssdw	mm5, mm7
	ASSUME	esi:NOTHING
	;
	dec	edx
	movzx	ecx, ax
	movzx	ebx, dx
	sar	eax, 16
	sar	edx, 16
	;
	.IF	(SDWORD PTR eax) > rectClip.right
		mov	eax, rectClip.right
		xor	ecx, ecx
		mov	edx, eax
		mov	ebx, 0FFFFH
	.ELSEIF	(SDWORD PTR edx) < rectClip.left
		mov	eax, rectClip.left
		xor	ecx, ecx
		mov	edx, eax
		mov	ebx, 0FFFFH
	.ELSE
		.IF	(SDWORD PTR eax) < rectClip.left
			mov	eax, rectClip.left
			xor	ecx, ecx
		.ENDIF
		.IF	(SDWORD PTR edx) > rectClip.right
			mov	edx, rectClip.right
			mov	ebx, 0FFFFH
		.ENDIF
	.ENDIF
	;
	mov	[edi].nLeft, eax
	mov	[edi].dwReserved1, ecx
	mov	[edi].nRight, edx
	mov	[edi].dwReserved2, ebx
	movq	MMWORD PTR [edi].rgbaLeft, mm0
	movq	MMWORD PTR [edi].rgbaRight, mm1
	movq	MMWORD PTR [edi].vLeft, mm4
	movq	MMWORD PTR [edi].vRight, mm5
	ASSUME	edi:NOTHING
	jmp	@Label_Exit


;
;	座標をコピーして進めるサブ関数
; ----------------------------------------------------------------------------
ALIGN	10H
@SubFunc_CopyVector_SSE:
	ASSUME	edi:PTR EGL_PACKED_VERTEX
	ASSUME	esi:PTR EGL_PACKED_VERTEX
	movq	mm0, MMWORD PTR [esi].vector
	movq	mm1, MMWORD PTR [esi].color
	movups	xmm0, [esi].normal
	movq	MMWORD PTR [edi].vector, mm0
	movq	MMWORD PTR [edi].color, mm1
	movups	[edi].normal, xmm0
	add	edi, (SIZEOF EGL_PACKED_VERTEX)
	ASSUME	edi:NOTHING
	ASSUME	esi:NOTHING
	BYTE	0C3H	; ret

;
;	色をクリップするサブ関数
; ----------------------------------------------------------------------------
ALIGN	10H
@SubFunc_ClipColor_SSE:
	ASSUME	edi:PTR EGL_PACKED_VERTEX
	ASSUME	esi:PTR EGL_PACKED_VERTEX
	mulss		xmm7, realConstant128
	movq		mm0, MMWORD PTR [esi].color
	cvtss2si	ebx, xmm7
	movq		mm2, MMWORD PTR pckvLastVertex.color
	pxor		mm7, mm7
	movd		mm6, ebx
	movq		mm1, mm0
	movq		mm3, mm2
	punpcklbw	mm0, mm7
	punpcklbw	mm2, mm7
	pshufw		mm6, mm6, 0
	punpckhbw	mm1, mm7
	punpckhbw	mm3, mm7
	psubw		mm0, mm2
	psubw		mm1, mm3
	pmullw		mm0, mm6
	pmullw		mm1, mm6
	psraw		mm0, 7
	psraw		mm1, 7
	paddsw		mm0, mm2
	paddsw		mm1, mm3
	packuswb	mm0, mm1
	movq		MMWORD PTR [edi].color, mm0
	add		edi, (SIZEOF EGL_PACKED_VERTEX)
	ASSUME	edi:NOTHING
	ASSUME	esi:NOTHING
	BYTE		0C3H	; ret

;
;	Ｙ座標をクリップするサブ関数
; ----------------------------------------------------------------------------
IF	0
ALIGN	10H
;@SubFunc_ClipVectorY_SSE:
	ASSUME	esi:PTR EGL_PACKED_VERTEX
	ASSUME	edi:PTR EGL_PACKED_VERTEX
	movss	xmm2, [esi].vector.y
	movss	xmm0, REAL4 PTR [ecx]
	movss	xmm1, pckvLastVertex.vector.y
	movss	[edi].vector.y, xmm0
	subss	xmm2, xmm1		; xmm2 = y1 - y0
	subss	xmm0, xmm1		; xmm0 = r - y0
	movss	xmm4, [esi].vector.x
	divss	xmm0, xmm2		; xmm0 = (r - x0) / (y1 - y0)
	movss	xmm5, pckvLastVertex.vector.x
	subss	xmm4, xmm5		; xmm4 = x1 - x0
	movss	xmm7, xmm0
	mulss	xmm0, xmm4
	cmp	pVertexColors, 0
	addss	xmm0, xmm5
	movss	[edi].vector.x, xmm0
	jnz	@SubFunc_ClipColor_SSE
	add	edi, (SIZEOF EGL_PACKED_VERTEX)
	BYTE	0C3H	; ret
	ASSUME	esi:NOTHING
	ASSUME	edi:NOTHING
ENDIF

ALIGN	10H
@SubFunc_ClipVectorY_SSE:
	ASSUME	esi:PTR EGL_PACKED_VERTEX
	ASSUME	edi:PTR EGL_PACKED_VERTEX
	movss	xmm2, [esi].vector.y
	movss	xmm0, REAL4 PTR [ecx]
	movss	xmm1, pckvLastVertex.vector.y
	movss	[edi].vector.y, xmm0
	subss	xmm2, xmm1		; xmm2 = y1 - y0
	subss	xmm0, xmm1		; xmm0 = r - y0
	movss	xmm4, [esi].vector.x
	divss	xmm0, xmm2		; xmm0 = (r - y0) / (y1 - y0)
	movss	xmm5, pckvLastVertex.vector.x
	movups	xmm3, [esi].normal
	movups	xmm6, pckvLastVertex.normal
	subss	xmm4, xmm5		; xmm4 = x1 - x0
	subps	xmm3, xmm6
	movss	xmm7, xmm0
	shufps	xmm0, xmm0, 0
	mulss	xmm4, xmm0
	mulps	xmm3, xmm0
	cmp	pVertexColors, 0
	addss	xmm4, xmm5
	addps	xmm3, xmm6
	movss	[edi].vector.x, xmm4
	movups	[edi].normal, xmm3
	jnz	@SubFunc_ClipColor_SSE
	add	edi, (SIZEOF EGL_PACKED_VERTEX)
	BYTE	0C3H	; ret
	ASSUME	esi:NOTHING
	ASSUME	edi:NOTHING

;
;	Ｘ座標をクリップするサブ関数
; ----------------------------------------------------------------------------
IF	0
ALIGN	10H
@SubFunc_ClipVectorX_SSE:
	ASSUME	esi:PTR EGL_PACKED_VERTEX
	ASSUME	edi:PTR EGL_PACKED_VERTEX
	movss	xmm2, [esi].vector.x
	movss	xmm0, REAL4 PTR [ecx]
	movss	xmm1, pckvLastVertex.vector.x
	movss	[edi].vector.x, xmm0
	movaps	xmm7, xmm0
	subss	xmm2, xmm1
	subss	xmm0, xmm1
	movss	xmm4, [esi].vector.y
	divss	xmm0, xmm2
	movss	xmm5, pckvLastVertex.vector.y
	subss	xmm4, xmm5
	movss	xmm7, xmm0
	mulss	xmm0, xmm4
	cmp	pVertexColors, 0
	addss	xmm0, xmm5
	movss	[edi].vector.y, xmm0
	jnz	@SubFunc_ClipColor_SSE
	add	edi, (SIZEOF EGL_PACKED_VERTEX)
	BYTE	0C3H	; ret
	ASSUME	esi:NOTHING
	ASSUME	edi:NOTHING
ENDIF

ALIGN	10H
@SubFunc_ClipVectorX_SSE:
	ASSUME	esi:PTR EGL_PACKED_VERTEX
	ASSUME	edi:PTR EGL_PACKED_VERTEX
	movss	xmm2, [esi].vector.x
	movss	xmm0, REAL4 PTR [ecx]
	movss	xmm1, pckvLastVertex.vector.x
	movss	[edi].vector.x, xmm0
	movaps	xmm7, xmm0
	subss	xmm2, xmm1
	subss	xmm0, xmm1
	movss	xmm4, [esi].vector.y
	divss	xmm0, xmm2
	movss	xmm5, pckvLastVertex.vector.y
	movups	xmm3, [esi].normal
	movups	xmm6, pckvLastVertex.normal
	subss	xmm4, xmm5
	subps	xmm3, xmm6
	movss	xmm7, xmm0
	shufps	xmm0, xmm0, 0
	mulss	xmm4, xmm0
	mulps	xmm3, xmm0
	cmp	pVertexColors, 0
	addss	xmm4, xmm5
	addps	xmm3, xmm6
	movss	[edi].vector.y, xmm4
	movups	[edi].normal, xmm3
	jnz	@SubFunc_ClipColor_SSE
	add	edi, (SIZEOF EGL_PACKED_VERTEX)
	BYTE	0C3H	; ret
	ASSUME	esi:NOTHING
	ASSUME	edi:NOTHING

;
;	次の頂点へ進めるサブ関数
; ----------------------------------------------------------------------------
ALIGN	10H
@SubFunc_NextSide1:
	ASSUME	esi:PTR EGL_PACKED_VERTEX
	cmp	ecx, spSideParam2.nIndex
	jz	@SubFunc_NextSideExit
@SubFunc_NextSide1First:
	add	ecx, (SIZEOF EGL_PACKED_VERTEX)
	xor	eax, eax
	cmp	ecx, nVertexCount_x32
	cmovge	ecx, eax
	lea	esi, [esi + ecx]
	mov	spSideParam1.nIndex, ecx
	mov	ecx, [esi].vector.y
	lea	edi, spSideParam1
	sar	ecx, 16
	sub	ecx, nCurrentY
	jz	@SubFunc_NextSideParam1
	jns	@SubFunc_NextSideParam
	call	@SubFunc_CopySideParam
	mov	ecx, spSideParam1.nIndex
	mov	esi, pDstVertexes
	cmp	ecx, spSideParam2.nIndex
	jnz	@SubFunc_NextSide1First
	ASSUME	edi:PTR EGL_SIDE_PARAM
	xor	eax, eax
	movq	mm0, MMWORD PTR [edi].nLastPosX
	movq	mm1, MMWORD PTR [edi].lastColor
	movups	xmm0, [edi].lastNormal
	inc	eax
	movq	MMWORD PTR [edi].nNextPosX, mm0
	movq	MMWORD PTR [edi].nextColor, mm1
	movups	[edi].nextNormal, xmm0
	BYTE	0C3H	; ret

ALIGN	10H
@SubFunc_NextSide2:
	ASSUME	esi:PTR EGL_PACKED_VERTEX
	cmp	ecx, spSideParam1.nIndex
	jz	@SubFunc_NextSideExit
@SubFunc_NextSide2First:
	mov	eax, nVertexCount_x32
	sub	eax, (SIZEOF EGL_PACKED_VERTEX)
	sub	ecx, (SIZEOF EGL_PACKED_VERTEX)
	cmovs	ecx, eax
	lea	esi, [esi + ecx]
	mov	spSideParam2.nIndex, ecx
	mov	ecx, [esi].vector.y
	lea	edi, spSideParam2
	sar	ecx, 16
	sub	ecx, nCurrentY
	jz	@SubFunc_NextSideParam1
	jns	@SubFunc_NextSideParam
	call	@SubFunc_CopySideParam
	mov	ecx, spSideParam2.nIndex
	mov	esi, pDstVertexes
	cmp	ecx, spSideParam1.nIndex
	jnz	@SubFunc_NextSide2First
	ASSUME	edi:PTR EGL_SIDE_PARAM
	xor	eax, eax
	movq	mm0, MMWORD PTR [edi].nLastPosX
	movq	mm1, MMWORD PTR [edi].lastColor
	movups	xmm0, [edi].lastNormal
	inc	eax
	movq	MMWORD PTR [edi].nNextPosX, mm0
	movq	MMWORD PTR [edi].nextColor, mm1
	movups	[edi].nextNormal, xmm0
	BYTE	0C3H	; ret

@SubFunc_NextSideExit:
	xor	eax, eax
	inc	eax
	BYTE	0C3H	; ret

ALIGN	10H
IF	0
@SubFunc_CopySideParam:
	ASSUME	edi:PTR EGL_SIDE_PARAM
	mov	edx, [edi].nIndex
	mov	esi, pDstVertexes
	movq	mm3, MMWORD PTR [edi].nNextPosX
	movq	mm4, MMWORD PTR [edi].nextColor
		movq	mm2, MMWORD PTR [esi + edx].vector
		movq	mm0, MMWORD PTR [esi + edx].color
	cvtpi2ps	xmm0, mm2
		pxor	mm7, mm7
	movq	MMWORD PTR [edi].nLastPosX, mm3
	movq	MMWORD PTR [edi].lastColor, mm4
	movq	MMWORD PTR [edi].nNextPosX, mm2
	movq	MMWORD PTR [edi].nextColor, mm0
		movq	mm5, mm4
	;
		punpcklbw	mm4, mm7
	movss	[edi].floatPosX, xmm0
	mov	[edi].floatDltX, 0
		punpckhbw	mm5, mm7
	movlps	QWORD PTR [edi].floatNextPosX, xmm0
		psllw	mm4, 7
		psllw	mm5, 7
		movq	MMWORD PTR [edi].curColor.rgbMul, mm4
		movq	MMWORD PTR [edi].curColor.rgbAdd, mm5
		movq	MMWORD PTR [edi].dltColor.rgbMul, mm7
		movq	MMWORD PTR [edi].dltColor.rgbAdd, mm7
	;
	ASSUME	edi:NOTHING
	BYTE	0C3H	; ret
ENDIF

@SubFunc_CopySideParam:
	ASSUME	edi:PTR EGL_SIDE_PARAM
	mov	edx, [edi].nIndex
	mov	esi, pDstVertexes
	movq	mm3, MMWORD PTR [edi].nNextPosX
	movq	mm4, MMWORD PTR [edi].nextColor
	movups	xmm4, [edi].nextNormal
		movq	mm2, MMWORD PTR [esi + edx].vector
		movq	mm0, MMWORD PTR [esi + edx].color
		movups	xmm5, [esi + edx].normal
	cvtpi2ps	xmm0, mm2
		pxor	mm7, mm7
		xorps	xmm7, xmm7
		mulps	xmm5, xmmConstant4000H
	movq	MMWORD PTR [edi].nLastPosX, mm3
	movq	MMWORD PTR [edi].lastColor, mm4
	movups	[edi].lastNormal, xmm4
	movq	MMWORD PTR [edi].nNextPosX, mm2
	movq	MMWORD PTR [edi].nextColor, mm0
	movups	[edi].nextNormal, xmm5
		movq	mm5, mm4
	;
		punpcklbw	mm4, mm7
	movss	[edi].floatPosX, xmm0
	mov	[edi].floatDltX, 0
		punpckhbw	mm5, mm7
	movlps	QWORD PTR [edi].floatNextPosX, xmm0
		psllw	mm4, 7
		psllw	mm5, 7
		movq	MMWORD PTR [edi].curColor.rgbMul, mm4
		movq	MMWORD PTR [edi].curColor.rgbAdd, mm5
		movq	MMWORD PTR [edi].dltColor.rgbMul, mm7
		movq	MMWORD PTR [edi].dltColor.rgbAdd, mm7
		movups	[edi].curNormal, xmm4
		movups	[edi].dltNormal, xmm7
	;
	ASSUME	edi:NOTHING
	BYTE	0C3H	; ret

ALIGN	10H
@SubFunc_NextSideParam1:
IF	0
	ASSUME	edi:PTR EGL_SIDE_PARAM
	mov	[edi].nLeftHeight, 1
	;
	movq	mm1, MMWORD PTR [esi].vector
	movq	mm0, MMWORD PTR [edi].nNextPosX
	movzx	edx, WORD PTR [edi].nNextPosY
	cvtpi2ps	xmm7, mm1
	mov	eax, 0FFFFH
	movq	MMWORD PTR [edi].nNextPosX, mm1
	movq	MMWORD PTR [edi].nLastPosX, mm0
	psubd	mm1, mm0
	sub	eax, edx
	movlps	QWORD PTR [edi].floatNextPosX, xmm7
	cvtpi2ps	xmm1, mm1
	movss	xmm0, xmm1
	movss	xmm2, realConstant128
	cvtsi2ss	xmm3, eax
	shufps	xmm1, xmm1, 1
	divss	xmm3, xmm1
		movq	mm2, MMWORD PTR [esi].color
		movq	mm4, MMWORD PTR [edi].nextColor
		pxor	mm7, mm7
		movq	MMWORD PTR [edi].nextColor, mm2
		movq	MMWORD PTR [edi].lastColor, mm4
		movq	mm3, mm2
		movq	mm5, mm4
		punpcklbw	mm2, mm7
		punpcklbw	mm4, mm7
		punpckhbw	mm3, mm7
		punpckhbw	mm5, mm7
		psubw	mm2, mm4
		psubw	mm3, mm5
		psllw	mm4, 7
		psllw	mm5, 7
		cvtpi2ps	xmm4, mm0
	mulss	xmm2, xmm3
	mulss	xmm0, xmm3
	cvtps2pi	mm6, xmm2
	pshufw	mm6, mm6, 0
	addss	xmm4, xmm0
	;
	pmullw	mm2, mm6
		movq	MMWORD PTR [edi].dltColor.rgbMul, mm7
		movq	MMWORD PTR [edi].dltColor.rgbAdd, mm7
	pmullw	mm3, mm6
		movss	[edi].floatPosX, xmm4
		mov	[edi].floatDltX, 0
	paddsw	mm4, mm2
	paddsw	mm5, mm3
	movq	MMWORD PTR [edi].curColor.rgbMul, mm4
	movq	MMWORD PTR [edi].curColor.rgbAdd, mm5
	;
	ASSUME	edi:NOTHING
	xor	eax, eax
	BYTE	0C3H	; ret
ENDIF

	ASSUME	edi:PTR EGL_SIDE_PARAM
	mov	[edi].nLeftHeight, 1
	;
	movq	mm1, MMWORD PTR [esi].vector
			movups	xmm6, [esi].normal
	movq	mm0, MMWORD PTR [edi].nNextPosX
	movzx	edx, WORD PTR [edi].nNextPosY
			movups	xmm5, [edi].nextNormal
			mulps	xmm6, xmmConstant4000H
	cvtpi2ps	xmm7, mm1
	mov	eax, 0FFFFH
	movq	MMWORD PTR [edi].nNextPosX, mm1
	movq	MMWORD PTR [edi].nLastPosX, mm0
			movups	[edi].nextNormal, xmm6
			movups	[edi].lastNormal, xmm5
	psubd	mm1, mm0
			subps	xmm6, xmm5
	sub	eax, edx
	movlps	QWORD PTR [edi].floatNextPosX, xmm7
	cvtpi2ps	xmm1, mm1
	movss	xmm0, xmm1
	movss	xmm2, realConstant128
	cvtsi2ss	xmm3, eax
	shufps	xmm1, xmm1, 1
	divss	xmm3, xmm1		; (1 - y0.decimal) / (y1 - y0)
		movq	mm2, MMWORD PTR [esi].color
		movq	mm4, MMWORD PTR [edi].nextColor
		pxor	mm7, mm7
		movq	MMWORD PTR [edi].nextColor, mm2
		movq	MMWORD PTR [edi].lastColor, mm4
		movq	mm3, mm2
		movq	mm5, mm4
		punpcklbw	mm2, mm7
		punpcklbw	mm4, mm7
		punpckhbw	mm3, mm7
		punpckhbw	mm5, mm7
		psubw	mm2, mm4
		psubw	mm3, mm5
		psllw	mm4, 7
		psllw	mm5, 7
		cvtpi2ps	xmm4, mm0	; xmm4 <- x0, y0
	mulss	xmm2, xmm3
	mulss	xmm0, xmm3		; (x1 - x0) * (1 - y0.decimal) / (y1 - y0)
	shufps	xmm3, xmm3, 0
	mulps	xmm6, xmm3
	cvtps2pi	mm6, xmm2	; 128 * (1 - y0.decimal) / (y1 - y0)
	pshufw	mm6, mm6, 0
	addss	xmm4, xmm0
	addps	xmm6, xmm5
	;
	pmullw	mm2, mm6
		movq	MMWORD PTR [edi].dltColor.rgbMul, mm7
		movq	MMWORD PTR [edi].dltColor.rgbAdd, mm7
	pmullw	mm3, mm6
		movss	[edi].floatPosX, xmm4
		mov	[edi].floatDltX, 0
	paddsw	mm4, mm2
	paddsw	mm5, mm3
	movq	MMWORD PTR [edi].curColor.rgbMul, mm4
	movq	MMWORD PTR [edi].curColor.rgbAdd, mm5
	;
		xorps	xmm5, xmm5
		movups	[edi].curNormal, xmm6
		movups	[edi].dltNormal, xmm5
	;
	ASSUME	edi:NOTHING
	xor	eax, eax
	BYTE	0C3H	; ret


ALIGN	10H
@SubFunc_NextSideParam:
IF	0
	ASSUME	edi:PTR EGL_SIDE_PARAM
	movq	mm1, MMWORD PTR [esi].vector
	movq	mm0, MMWORD PTR [edi].nNextPosX
			movzx	edx, WORD PTR [edi].nNextPosY
		inc	ecx
	cvtpi2ps	xmm7, mm1
	movq	MMWORD PTR [edi].nNextPosX, mm1
	psubd	mm1, mm0
	movq	MMWORD PTR [edi].nLastPosX, mm0
		mov	[edi].nLeftHeight, ecx
	cvtpi2ps	xmm1, mm1
	movss	xmm2, realConst32768x65536
	movlps	QWORD PTR [edi].floatNextPosX, xmm7
	movss	xmm0, xmm1
	shufps	xmm1, xmm1, 01010101B
			mov	eax, 0FFFFH
	unpcklps	xmm0, xmm2
	divps	xmm0, xmm1
			sub	eax, edx
	cvtpi2ps	xmm2, mm0
		cvtsi2ss	xmm4, eax
			pxor		mm7, mm7
			movq		mm0, MMWORD PTR [esi].color
				movq	mm2, MMWORD PTR [edi].nextColor
			movq	mm1, mm0
			movq	MMWORD PTR [edi].nextColor, mm0
				movq	mm3, mm2
				movq	MMWORD PTR [edi].lastColor, mm2
			punpcklbw	mm0, mm7
				punpcklbw	mm2, mm7
			punpckhbw	mm1, mm7
				punpckhbw	mm3, mm7
		mulss	xmm4, realConstDiv65536
			psllw		mm0, 7
			psllw		mm1, 7
		cvtps2pi	mm6, xmm0
				psllw		mm2, 7
				psllw		mm3, 7
		psrlq	mm6, 32
	mulss	xmm0, realConst65536
			pshufw	mm6, mm6, 0
				movq	MMWORD PTR [edi].curColor.rgbMul, mm2
				movq	MMWORD PTR [edi].curColor.rgbAdd, mm3
			psubsw		mm0, mm2
			psubsw		mm1, mm3
	;
	mulss	xmm4, xmm0
	movss	[edi].floatDltX, xmm0
			pmulhw		mm0, mm6
			pmulhw		mm1, mm6
	addss	xmm2, xmm4
			paddsw		mm0, mm0
			paddsw		mm1, mm1
	movss	[edi].floatPosX, xmm2
	;
			movq	MMWORD PTR [edi].dltColor.rgbMul, mm0
			movq	MMWORD PTR [edi].dltColor.rgbAdd, mm1
	;
			shr	eax, 1
			movd	mm5, eax
			pshufw	mm5, mm5, 0
			pmulhw	mm0, mm5
			pmulhw	mm1, mm5
			paddsw	mm2, mm0
			paddsw	mm3, mm1
			paddsw	mm2, mm0
			paddsw	mm3, mm1
			movq	MMWORD PTR [edi].curColor.rgbMul, mm2
			movq	MMWORD PTR [edi].curColor.rgbAdd, mm3
	;
	xor	eax, eax
	BYTE	0C3H	; ret
ENDIF

	ASSUME	edi:PTR EGL_SIDE_PARAM
	movq	mm1, MMWORD PTR [esi].vector
			movups	xmm6, [esi].normal
	movq	mm0, MMWORD PTR [edi].nNextPosX
			movzx	edx, WORD PTR [edi].nNextPosY
			movups	xmm5, [edi].nextNormal
			mulps	xmm6, xmmConstant4000H
		inc	ecx
	cvtpi2ps	xmm7, mm1
	movq	MMWORD PTR [edi].nNextPosX, mm1
	psubd	mm1, mm0
	movq	MMWORD PTR [edi].nLastPosX, mm0
			movups	[edi].nextNormal, xmm6
			movups	[edi].lastNormal, xmm5
			subps	xmm6, xmm5
		mov	[edi].nLeftHeight, ecx
	cvtpi2ps	xmm1, mm1
	movss	xmm2, realConst32768x65536
	movlps	QWORD PTR [edi].floatNextPosX, xmm7
	movss	xmm0, xmm1
	shufps	xmm1, xmm1, 01010101B
			mov	eax, 0FFFFH
	unpcklps	xmm0, xmm2
	movhps	xmm0, QWORD PTR xmmConstant10000H
	divps	xmm0, xmm1			; { x1 - x0, 8000H, 1, 1 } / (y1 - y0)
			sub	eax, edx
	cvtpi2ps	xmm2, mm0		; xmm2 <- x0, y0
		cvtsi2ss	xmm4, eax	; xmm4 <- (1 - y0.decimal)
			pxor		mm7, mm7
			movq		mm0, MMWORD PTR [esi].color
				movq	mm2, MMWORD PTR [edi].nextColor
			movq	mm1, mm0
			movq	MMWORD PTR [edi].nextColor, mm0
				movq	mm3, mm2
				movq	MMWORD PTR [edi].lastColor, mm2
			punpcklbw	mm0, mm7
				punpcklbw	mm2, mm7
			punpckhbw	mm1, mm7
				punpckhbw	mm3, mm7
		mulss	xmm4, realConstDiv65536
			psllw		mm0, 7
			psllw		mm1, 7
		cvtps2pi	mm6, xmm0
		shufps	xmm4, xmm4, 0
	movhlps	xmm7, xmm0			; xmm7 = 1 / (y1 - y0)
				psllw		mm2, 7
				psllw		mm3, 7
		psrlq	mm6, 32
	mulss	xmm0, realConst65536		; xmm0 = delta X
	shufps	xmm7, xmm7, 0
			pshufw	mm6, mm6, 0	; mm6 <- 8000H / (y1 - y0)
				movq	MMWORD PTR [edi].curColor.rgbMul, mm2
				movq	MMWORD PTR [edi].curColor.rgbAdd, mm3
	mulps	xmm6, xmm7			; xmm6 <- delta Normal
			psubsw		mm0, mm2
			psubsw		mm1, mm3
	;
	movups	[edi].dltNormal, xmm6
	mulps	xmm6, xmm4
	mulss	xmm4, xmm0		; xmm4 <- (1 - y0.decimal) * delta X
	movss	[edi].floatDltX, xmm0
			pmulhw		mm0, mm6
			pmulhw		mm1, mm6
	addps	xmm5, xmm6
	addss	xmm2, xmm4
			paddsw		mm0, mm0
			paddsw		mm1, mm1
	movss	[edi].floatPosX, xmm2
	movups	[edi].curNormal, xmm5
	;
			movq	MMWORD PTR [edi].dltColor.rgbMul, mm0
			movq	MMWORD PTR [edi].dltColor.rgbAdd, mm1
	;
			shr	eax, 1		; eax = (1 - y0.decimal) * 8000H
			movd	mm5, eax
			pshufw	mm5, mm5, 0
			pmulhw	mm0, mm5
			pmulhw	mm1, mm5
			paddsw	mm2, mm0
			paddsw	mm3, mm1
			paddsw	mm2, mm0
			paddsw	mm3, mm1
			movq	MMWORD PTR [edi].curColor.rgbMul, mm2
			movq	MMWORD PTR [edi].curColor.rgbAdd, mm3
	;
	xor	eax, eax
	BYTE	0C3H	; ret

	ASSUME	ebx:NOTHING
	ASSUME	esi:NOTHING
	ASSUME	ebx:NOTHING
	ASSUME	edi:NOTHING


;
;	レガシーコード
; ----------------------------------------------------------------------------
	.486
	.387
ALIGN	10H
Label_Start486:
	mov	esi, pClipRect
	test	esi, esi
	jz	Label_ErrorExit
	ASSUME	esi:PCEGL_RECT
	FOR	@MEMBER, <left, top, right, bottom>
		mov	eax, [esi].@MEMBER
		mov	rectClip.@MEMBER, eax
	ENDM
	fild	[esi].left
	fstp	rMinX
	fild	[esi].right
	fld1
	faddp	st(1), st
	fstp	rMaxX
	fild	[esi].top
	fstp	rMinY
	fild	[esi].bottom
	fld1
	faddp	st(1), st
	fstp	rMaxY
	;
	mov	eax, rMinX
	mov	ebx, rMaxX
	mov	ecx, rMinY
	mov	edx, rMaxY
	FOR	@REG, <eax, ebx, ecx, edx>
		test	@REG, @REG
		.IF	SIGN?
			neg	@REG
			or	@REG, 80000000H
		.ENDIF
	ENDM
	mov	rMinX[4], eax
	mov	rMaxX[4], ebx
	mov	rMinY[4], ecx
	mov	rMaxY[4], edx
	ASSUME	esi:NOTHING
	;
	;	ｙ座標をクリップ
	; --------------------------------------------------------------------
	IF	(SIZEOF E3D_VECTOR_2D) NE 8
		.ERR
	ENDIF
	IF	(SIZEOF E3D_COLOR) NE 8
		.ERR
	ENDIF
	mov	eax, nVertexCount
	shl	eax, 3 + 1	; * (SIZEOF E3D_VECTOR_2D) * 2
	sub	esp, eax
	mov	pTempVertex, esp
	mov	edi, pTempVertex
	mov	esi, pPolyVertexes
	;
	mov	ebx, pVertexColors
	mov	pTempColor, NULL
	mov	pDstNextColor, NULL
	mov	pSrcNextColor, ebx
	mov	ecx, nVertexCount
	ASSUME	edi:PTR E3D_VECTOR_2D
	ASSUME	esi:PCE3D_VECTOR_2D
	;
	; 終端座標のチェック
	;
	.IF	ebx != NULL
		sub	esp, eax
		mov	pTempColor, esp
		mov	pDstNextColor, esp
		;
		ASSUME	ebx:PTR E3D_COLOR
		mov	eax, [ebx + ecx * 8 - 8].rgbMul.dwPixelCode
		mov	edx, [ebx + ecx * 8 - 8].rgbAdd.dwPixelCode
		mov	clrLastColor.rgbMul.dwPixelCode, eax
		mov	clrLastColor.rgbAdd.dwPixelCode, edx
		ASSUME	ebx:NOTHING
	.ENDIF
	mov	eax, DWORD PTR [esi + ecx * 8 - 8].x
	mov	ebx, DWORD PTR [esi + ecx * 8 - 8].y
	mov	DWORD PTR vxLastPoint.x, eax
	mov	DWORD PTR vxLastPoint.y, ebx
	;
	test	ebx, ebx
	.IF	SIGN?
		neg	ebx
		or	ebx, 80000000H
	.ENDIF
	xor	edx, edx
	.IF	(SDWORD PTR ebx) < (SDWORD PTR rMinY[4])
		dec	edx
	.ELSEIF	(SDWORD PTR ebx) > (SDWORD PTR rMaxY[4])
		inc	edx
	.ENDIF
	mov	dwClipFlag, edx
	;
	call	SubFunc_ClipYLoop
	;
	ASSUME	edi:NOTHING
	ASSUME	esi:NOTHING
	;
	mov	eax, pTempVertex
	mov	ebx, pTempColor
	sub	edi, eax
	shr	edi, 3
	mov	pPolyVertexes, eax
	mov	nVertexCount, edi
	mov	pVertexColors, ebx
	.IF	edi < 3
Label_ErrorExit_2:
		mov	esp, dwSaveESP
		xor	eax, eax
		ret
	.ENDIF
	;
	;	ｘ座標をクリップ
	; --------------------------------------------------------------------
	IF	(SIZEOF E3D_VECTOR_2D) NE 8
		.ERR
	ENDIF
	IF	(SIZEOF E3D_COLOR) NE 8
		.ERR
	ENDIF
	mov	eax, nVertexCount
	shl	eax, 3 + 1	; * (SIZEOF E3D_VECTOR_2D) * 2
	sub	esp, eax
	mov	pTempVertex, esp
	mov	edi, pTempVertex
	mov	esi, pPolyVertexes
	;
	mov	ebx, pVertexColors
	mov	pTempColor, NULL
	mov	pDstNextColor, NULL
	mov	pSrcNextColor, ebx
	mov	ecx, nVertexCount
	ASSUME	edi:PTR E3D_VECTOR_2D
	ASSUME	esi:PCE3D_VECTOR_2D
	;
	; 終端座標のチェック
	;
	.IF	ebx != NULL
		sub	esp, eax
		mov	pTempColor, esp
		mov	pDstNextColor, esp
		;
		ASSUME	ebx:PTR E3D_COLOR
		mov	eax, [ebx + ecx * 8 - 8].rgbMul.dwPixelCode
		mov	edx, [ebx + ecx * 8 - 8].rgbAdd.dwPixelCode
		mov	clrLastColor.rgbMul.dwPixelCode, eax
		mov	clrLastColor.rgbAdd.dwPixelCode, edx
		ASSUME	ebx:NOTHING
	.ENDIF
	mov	eax, DWORD PTR [esi + ecx * 8 - 8].x
	mov	ebx, DWORD PTR [esi + ecx * 8 - 8].y
	mov	DWORD PTR vxLastPoint.x, eax
	mov	DWORD PTR vxLastPoint.y, ebx
	;
	test	eax, eax
	.IF	SIGN?
		neg	eax
		or	eax, 80000000H
	.ENDIF
	xor	edx, edx
	.IF	(SDWORD PTR eax) < (SDWORD PTR rMinX[4])
		dec	edx
	.ELSEIF	(SDWORD PTR eax) > (SDWORD PTR rMaxX[4])
		inc	edx
	.ENDIF
	mov	dwClipFlag, edx
	;
	call	SubFunc_ClipXLoop
	;
	ASSUME	edi:NOTHING
	ASSUME	esi:NOTHING
	;
	mov	eax, pTempVertex
	mov	ebx, pTempColor
	sub	edi, eax
	shr	edi, 3
	mov	pPolyVertexes, eax
	mov	nVertexCount, edi
	mov	pVertexColors, ebx
	;
	cmp	edi, 3
	jb	Label_ErrorExit_2
	;
	;	リージョン作成
	; --------------------------------------------------------------------
	;
	; 座標を整数に変換
	;
	mov	esi, pPolyVertexes
	ASSUME	esi:PCE3D_VECTOR_2D
	mov	ecx, nVertexCount
	.REPEAT
		fld	[esi].x
		fmul	realConst65536
		fld	[esi].y
		fmul	realConst65536
		fxch	st(1)
		fistp	SDWORD PTR [esi].x
		fistp	SDWORD PTR [esi].y
		add	esi, (SIZEOF E3D_VECTOR_2D)
		dec	ecx
	.UNTIL	ZERO?
	;
	; 最小ｙ座標を持つ頂点を取得
	;
	mov	esi, pPolyVertexes
	ASSUME	esi:PTR EGL_POINT
	xor	ecx, ecx
	mov	eax, 7FFFFFFFH
	xor	edx, edx
	.REPEAT
		mov	ebx, [esi + ecx * 8].y
		.IF	(SDWORD PTR eax) > (SDWORD PTR ebx)
			mov	eax, ebx
			mov	edx, ecx
		.ENDIF
		inc	ecx
	.UNTIL	ecx >= nVertexCount
	;
	; 初期座標設定
	;
	mov	ebx, pVertexColors
	ASSUME	ebx:PCE3D_COLOR
	;
	mov	fxTop_x10000H, eax
	mov	nTopIndex, edx
	sar	eax, 16
	mov	nCurrentY, eax
	;
	xor	eax, eax
	mov	spSideParam1.nIndex, edx
	mov	spSideParam1.nLeftHeight, eax
	mov	spSideParam2.nIndex, edx
	mov	spSideParam2.nLeftHeight, eax
	;
	lea	edi, spSideParam1
	call	SubFunc_CopySideParam
	lea	edi, spSideParam2
	call	SubFunc_CopySideParam
	ASSUME	esi:NOTHING
	;
	inc	nCurrentY
	mov	ecx, spSideParam1.nIndex
	mov	esi, pPolyVertexes
	call	SubFunc_NextSide1First
	jnz	Label_ErrorExit_2
	;
	mov	ecx, spSideParam2.nIndex
	mov	esi, pPolyVertexes
	call	SubFunc_NextSide2First
	jnz	Label_ErrorExit_2
	;
	; リージョン作成
	;
	mov	edi, pPolyRegion
	ASSUME	edi:PTR E3D_POLYGON_REGION
	lea	edi, [edi].plrLineRgn[0]
	ASSUME	edi:PTR E3D_POLY_LINE_REGION
	mov	ptrNextLine, edi
	;
	; １ライン目処理
	;
	mov	ecx, nCurrentY
	movzx	eax, WORD PTR fxTop_x10000H
	lea	edx, [ecx - 1]
	mov	nTopLine, ecx
	.IF	(eax == 0) && (edx == rectClip.top)
		dec	ecx
		.IF	((SDWORD PTR ecx) >= rectClip.top) \
				&& ((SDWORD PTR ecx) <= rectClip.bottom)
			mov	nTopLine, ecx
			;
			; １ライン目の幅を取得する
			;
			mov	esi, pPolyVertexes
			ASSUME	esi:PTR EGL_POINT
			mov	eax, nTopIndex
			mov	ebx, eax
			mov	ecx, [esi + eax * 8].x
			mov	fxTop_LeftIndex, eax
			mov	fxTop_RightIndex, eax
			mov	fxTop_Left, ecx
			mov	fxTop_Right, ecx
			;
			mov	edx, nTopLine
			mov	edi, nVertexCount
			shl	edx, 16
			xor	ecx, ecx
			.REPEAT
				.IF	(SDWORD PTR [esi + ecx * 8].y) <= (SDWORD PTR edx)
					mov	eax, [esi + ecx * 8].x
					.IF	(SDWORD PTR eax) < fxTop_Left
						mov	fxTop_Left, eax
						mov	fxTop_LeftIndex, ecx
					.ELSEIF	fxTop_Right < (SDWORD PTR eax)
						mov	fxTop_Right, eax
						mov	fxTop_RightIndex, ecx
					.ENDIF
				.ENDIF
				inc	ecx
			.UNTIL	ecx >= edi
			;
			; ラインを出力する
			;
			mov	ecx, fxTop_LeftIndex
			mov	edx, fxTop_RightIndex
			mov	eax, [esi + ecx * 8].x
			mov	edx, [esi + edx * 8].x
			ASSUME	esi:NOTHING
			;
			dec	edx
			movzx	ecx, ax
			movzx	ebx, dx
			sar	eax, 16
			sar	edx, 16
			;
			.IF	(SDWORD PTR eax) > rectClip.right
				mov	eax, rectClip.right
				xor	ecx, ecx
				mov	edx, eax
				mov	ebx, 0FFFFH
			.ELSEIF	(SDWORD PTR edx) < rectClip.left
				mov	eax, rectClip.left
				xor	ecx, ecx
				mov	edx, eax
				mov	ebx, 0FFFFH
			.ELSE
				.IF	(SDWORD PTR eax) < rectClip.left
					mov	eax, rectClip.left
					xor	ecx, ecx
				.ENDIF
				.IF	(SDWORD PTR edx) > rectClip.right
					mov	edx, rectClip.right
					mov	ebx, 0FFFFH
				.ENDIF
			.ENDIF
			;
			mov	edi, ptrNextLine
			mov	[edi].nLeft, eax
			mov	[edi].dwReserved1, ecx
			mov	[edi].nRight, edx
			mov	[edi].dwReserved2, ebx
			;
			mov	ebx, pVertexColors
			.IF	ebx != NULL
				mov	ecx, fxTop_LeftIndex
				mov	eax, DWORD PTR [ebx + ecx * 8]
				mov	edx, DWORD PTR [ebx + ecx * 8 + 4]
				mov	DWORD PTR [edi].rgbaLeft[0], eax
				mov	DWORD PTR [edi].rgbaLeft[4], edx
				mov	ecx, fxTop_RightIndex
				mov	eax, DWORD PTR [ebx + ecx * 8]
				mov	edx, DWORD PTR [ebx + ecx * 8 + 4]
				mov	DWORD PTR [edi].rgbaRight[0], eax
				mov	DWORD PTR [edi].rgbaRight[4], edx
			.ENDIF
			add	edi, (SIZEOF E3D_POLY_LINE_REGION)
		.ENDIF
	.ENDIF
	;
	;	486 互換リージョン生成コード
	; --------------------------------------------------------------------
	@INDEX = 0
	FOR	@SIDE, <spSideParam1, spSideParam2>
		FOR	@CLR, <rgbMul, rgbAdd>
			mov	ax, @SIDE.curColor.@CLR.Blue
			mov	bx, @SIDE.curColor.@CLR.Green
			mov	cx, @SIDE.curColor.@CLR.Red
			sar	ax, 7
			sar	bx, 7
			sar	cx, 7
			mov	rgbaSide[@INDEX].@CLR.rgb.Blue, al
			mov	rgbaSide[@INDEX].@CLR.rgb.Green, bl
			mov	rgbaSide[@INDEX].@CLR.rgb.Red, cl
		ENDM
		;
		@INDEX = @INDEX + (SIZEOF E3D_COLOR)
	ENDM
	;
	mov	eax, nCurrentY
	cmp	eax, rectClip.bottom
	mov	ptrNextLine, edi
	jg	Label_RegionFinished
	;
Label_RegionLoop_486:
		;
		; 座標差分処理
		;
		fld	spSideParam1.floatPosX
		fist	nCurrentX[0]
		fld	spSideParam2.floatPosX
		fist	nCurrentX[4]
		fxch	st(1)
		fadd	spSideParam1.floatDltX
		fxch	st(1)
		fadd	spSideParam2.floatDltX
		fxch	st(1)
		fstp	spSideParam1.floatPosX
		fstp	spSideParam2.floatPosX
		mov	eax, nCurrentX[0]
		mov	ebx, nCurrentX[4]
		movzx	ecx, ax
		movzx	edx, bx
		sar	eax, 16
		sar	ebx, 16
		;
		; ライン情報出力
		;
		mov	edi, ptrNextLine
		.IF	(SDWORD PTR eax) < rectClip.left
			mov	eax, rectClip.left
			xor	ecx, ecx
		.ELSEIF	(SDWORD PTR eax) > rectClip.right
			mov	eax, rectClip.right
			xor	ecx, ecx
		.ENDIF
		.IF	(SDWORD PTR ebx) < rectClip.left
			mov	ebx, rectClip.left
			xor	edx, edx
		.ELSEIF	(SDWORD PTR ebx) > rectClip.right
			mov	ebx, rectClip.right
			mov	edx, 0FFFFH
		.ENDIF
		;
		.IF	((SDWORD PTR eax) < (SDWORD PTR ebx)) \
				|| ((eax == ebx) && (ecx < edx))
			mov	[edi].nLeft, eax
			mov	[edi].dwReserved1, ecx
;			sub	dx, 1
;			sbb	ebx, 0
			mov	[edi].nRight, ebx
			mov	[edi].dwReserved2, edx
			;
			mov	eax, rgbaSide[0].rgbMul.dwPixelCode
			mov	ebx, rgbaSide[0].rgbAdd.dwPixelCode
			mov	ecx, rgbaSide[8].rgbMul.dwPixelCode
			mov	edx, rgbaSide[8].rgbAdd.dwPixelCode
		.ELSE
			mov	[edi].nLeft, ebx
			mov	[edi].dwReserved1, edx
;			sub	cx, 1
;			sbb	eax, 0
			mov	[edi].nRight, eax
			mov	[edi].dwReserved2, ecx
			;
			mov	eax, rgbaSide[8].rgbMul.dwPixelCode
			mov	ebx, rgbaSide[8].rgbAdd.dwPixelCode
			mov	ecx, rgbaSide[0].rgbMul.dwPixelCode
			mov	edx, rgbaSide[0].rgbAdd.dwPixelCode
		.ENDIF
		;
		mov	[edi].rgbaLeft.rgbMul.dwPixelCode, eax
		mov	[edi].rgbaLeft.rgbAdd.dwPixelCode, ebx
		mov	[edi].rgbaRight.rgbMul.dwPixelCode, ecx
		mov	[edi].rgbaRight.rgbAdd.dwPixelCode, edx
		;
		; ｙ座標更新
		;
		mov	eax, nCurrentY
		mov	edi, ptrNextLine
		.IF	((SDWORD PTR eax) >= rectClip.top) && \
				((SDWORD PTR eax) <= rectClip.bottom)
			add	edi, (SIZEOF E3D_POLY_LINE_REGION)
		.ENDIF
		inc	eax
		mov	nCurrentY, eax
		cmp	eax, rectClip.bottom
		mov	ptrNextLine, edi
		jg	Label_RegionFinished
		;
		; 色差分処理
		;
		.IF	pVertexColors != NULL
			@INDEX = 0
			FOR	@SIDE, <spSideParam1, spSideParam2>
				FOR	@CLR, <rgbMul, rgbAdd>
					movsx	eax, @SIDE.curColor.@CLR.Blue
					movsx	ebx, @SIDE.curColor.@CLR.Green
					movsx	ecx, @SIDE.curColor.@CLR.Red
					movsx	edx, @SIDE.dltColor.@CLR.Blue
					movsx	esi, @SIDE.dltColor.@CLR.Green
					movsx	edi, @SIDE.dltColor.@CLR.Red
					add	eax, edx
					add	ebx, esi
					add	ecx, edi
					.IF	eax >= 8000H
						sar	eax, 31
						not	eax
						and	eax, 7FFFH
					.ENDIF
					.IF	ebx >= 8000H
						sar	ebx, 31
						not	ebx
						and	ebx, 7FFFH
					.ENDIF
					.IF	ecx >= 8000H
						sar	ecx, 31
						not	ecx
						and	ecx, 7FFFH
					.ENDIF
					mov	@SIDE.curColor.@CLR.Blue, ax
					mov	@SIDE.curColor.@CLR.Green, bx
					mov	@SIDE.curColor.@CLR.Red, cx
					sar	eax, 7
					sar	ebx, 7
					sar	ecx, 7
					mov	rgbaSide[@INDEX].@CLR.rgb.Blue, al
					mov	rgbaSide[@INDEX].@CLR.rgb.Green, bl
					mov	rgbaSide[@INDEX].@CLR.rgb.Red, cl
				ENDM
				;
				@INDEX = @INDEX + (SIZEOF E3D_COLOR)
			ENDM
		.ENDIF
		;
		; 頂点更新
		;
		mov	eax, spSideParam1.nLeftHeight
		mov	ecx, spSideParam1.nIndex
		dec	eax
		mov	esi, pPolyVertexes
		mov	spSideParam1.nLeftHeight, eax
		.IF	ZERO?
			call	SubFunc_NextSide1
			jnz	Label_RegionFinished
			;
			FOR	@CLR, <rgbMul, rgbAdd>
				mov	ax, spSideParam1.curColor.@CLR.Blue
				mov	bx, spSideParam1.curColor.@CLR.Green
				mov	cx, spSideParam1.curColor.@CLR.Red
				sar	ax, 7
				sar	bx, 7
				sar	cx, 7
				mov	rgbaSide[0].@CLR.rgb.Blue, al
				mov	rgbaSide[0].@CLR.rgb.Green, bl
				mov	rgbaSide[0].@CLR.rgb.Red, cl
			ENDM
		.ENDIF
		;
		mov	eax, spSideParam2.nLeftHeight
		mov	ecx, spSideParam2.nIndex
		dec	eax
		mov	esi, pPolyVertexes
		mov	spSideParam2.nLeftHeight, eax
		.IF	ZERO?
			call	SubFunc_NextSide2
			jnz	Label_RegionFinished
			;
			FOR	@CLR, <rgbMul, rgbAdd>
				mov	ax, spSideParam2.curColor.@CLR.Blue
				mov	bx, spSideParam2.curColor.@CLR.Green
				mov	cx, spSideParam2.curColor.@CLR.Red
				sar	ax, 7
				sar	bx, 7
				sar	cx, 7
				mov	rgbaSide[8].@CLR.rgb.Blue, al
				mov	rgbaSide[8].@CLR.rgb.Green, bl
				mov	rgbaSide[8].@CLR.rgb.Red, cl
			ENDM
		.ENDIF
	jmp	Label_RegionLoop_486

Label_RegionFinished:
	;
	; 有効幅決定
	;
	mov	edi, pPolyRegion
	ASSUME	edi:PTR E3D_POLYGON_REGION
	mov	eax, nTopLine
	mov	edx, nCurrentY
	dec	edx
	.IF	(SDWORD PTR eax) < rectClip.top
		mov	eax, rectClip.top
	.ELSEIF	(SDWORD PTR eax) > rectClip.bottom
		mov	pPolyRegion, NULL
	.ENDIF
	.IF	((SDWORD PTR edx) < rectClip.top) \
			|| ((SDWORD PTR edx) < (SDWORD PTR eax))
		mov	pPolyRegion, NULL
	.ENDIF
	mov	[edi].nTopLine, eax
	mov	[edi].nBottomLine, edx
	ASSUME	edi:NOTHING

Label_Exit:
	mov	esp, dwSaveESP
	mov	eax, pPolyRegion
	ret


;
;	ｙ座標をクリップ
; ----------------------------------------------------------------------------
ALIGN	10H
SubFunc_ClipYLoop:
	ASSUME	esi:PCE3D_VECTOR_2D
	.REPEAT
		;
		; 現在の座標のチェック
		;
		mov	nLoopCounter, ecx
		mov	eax, DWORD PTR [esi].x
		mov	ebx, DWORD PTR [esi].y
		push	ebx
		test	ebx, ebx
		.IF	SIGN?
			neg	ebx
			or	ebx, 80000000H
		.ENDIF
		xor	edx, edx
		.IF	(SDWORD PTR ebx) < (SDWORD PTR rMinY[4])
			dec	edx
		.ELSEIF	(SDWORD PTR ebx) > (SDWORD PTR rMaxY[4])
			inc	edx
		.ENDIF
		pop	ebx
		;
		; クリップ処理
		;
		.IF	edx != dwClipFlag
			.IF	dwClipFlag == -1
				lea	ecx, rMinY
				call	SubFunc_ClipVectorY
			.ELSEIF	dwClipFlag == 1
				lea	ecx, rMaxY
				call	SubFunc_ClipVectorY
			.ENDIF
			test	edx, edx
			mov	dwClipFlag, edx
			.IF	SIGN?
				lea	ecx, rMinY
				call	SubFunc_ClipVectorY
			.ELSEIF	ZERO?
				call	SubFunc_CopyVector
			.ELSE
				lea	ecx, rMaxY
				call	SubFunc_ClipVectorY
			.ENDIF
		.ELSEIF	edx == 0
			call	SubFunc_CopyVector
		.ENDIF
		;
		mov	edx, pSrcNextColor
		mov	ecx, nLoopCounter
		add	esi, (SIZEOF E3D_VECTOR_2D)
		mov	DWORD PTR vxLastPoint.x, eax
		mov	DWORD PTR vxLastPoint.y, ebx
		.IF	edx != NULL
			ASSUME	edx:PTR E3D_COLOR
			mov	eax, [edx].rgbMul.dwPixelCode
			mov	clrLastColor.rgbMul.dwPixelCode, eax
			mov	eax, [edx].rgbAdd.dwPixelCode
			mov	clrLastColor.rgbAdd.dwPixelCode, eax
			ASSUME	edx:NOTHING
			add	edx, (SIZEOF E3D_COLOR)
			mov	pSrcNextColor, edx
		.ENDIF
		;
		dec	ecx
	.UNTIL	ZERO?
	BYTE	0C3H	; ret
	ASSUME	esi:NOTHING


;
;	ｘ座標をクリップ
; ----------------------------------------------------------------------------
ALIGN	10H
SubFunc_ClipXLoop:
	ASSUME	esi:PCE3D_VECTOR_2D
	.REPEAT
		;
		; 現在の座標のチェック
		;
		mov	nLoopCounter, ecx
		mov	eax, DWORD PTR [esi].x
		mov	ebx, DWORD PTR [esi].y
		push	eax
		test	eax, eax
		.IF	SIGN?
			neg	eax
			or	eax, 80000000H
		.ENDIF
		xor	edx, edx
		.IF	(SDWORD PTR eax) < (SDWORD PTR rMinX[4])
			dec	edx
		.ELSEIF	(SDWORD PTR eax) > (SDWORD PTR rMaxX[4])
			inc	edx
		.ENDIF
		pop	eax
		;
		; クリップ処理
		;
		.IF	edx != dwClipFlag
			.IF	dwClipFlag == -1
				lea	ecx, rMinX
				call	SubFunc_ClipVectorX
			.ELSEIF	dwClipFlag == 1
				lea	ecx, rMaxX
				call	SubFunc_ClipVectorX
			.ENDIF
			test	edx, edx
			mov	dwClipFlag, edx
			.IF	SIGN?
				lea	ecx, rMinX
				call	SubFunc_ClipVectorX
			.ELSEIF	ZERO?
				call	SubFunc_CopyVector
			.ELSE
				lea	ecx, rMaxX
				call	SubFunc_ClipVectorX
			.ENDIF
		.ELSEIF	edx == 0
			call	SubFunc_CopyVector
		.ENDIF
		;
		mov	edx, pSrcNextColor
		mov	ecx, nLoopCounter
		add	esi, (SIZEOF E3D_VECTOR_2D)
		mov	DWORD PTR vxLastPoint.x, eax
		mov	DWORD PTR vxLastPoint.y, ebx
		.IF	edx != NULL
			ASSUME	edx:PTR E3D_COLOR
			mov	eax, [edx].rgbMul.dwPixelCode
			mov	clrLastColor.rgbMul.dwPixelCode, eax
			mov	eax, [edx].rgbAdd.dwPixelCode
			mov	clrLastColor.rgbAdd.dwPixelCode, eax
			ASSUME	edx:NOTHING
			add	edx, (SIZEOF E3D_COLOR)
			mov	pSrcNextColor, edx
		.ENDIF
		;
		dec	ecx
	.UNTIL	ZERO?
	BYTE	0C3H	; ret
	ASSUME	esi:NOTHING


;
;	座標をコピーして進めるサブ関数
; ----------------------------------------------------------------------------
ALIGN	10H
SubFunc_CopyVector:
	ASSUME	edi:PTR E3D_VECTOR_2D
	push	ebx
	mov	DWORD PTR [edi].x, eax
	mov	DWORD PTR [edi].y, ebx
	mov	ebx, pSrcNextColor
	add	edi, (SIZEOF E3D_VECTOR_2D)
	.IF	ebx != NULL
		push	eax
		push	edx
		ASSUME	ebx:PTR E3D_COLOR
		mov	eax, [ebx].rgbMul.dwPixelCode
		mov	edx, [ebx].rgbAdd.dwPixelCode
		mov	ebx, pDstNextColor
		mov	[ebx].rgbMul.dwPixelCode, eax
		mov	[ebx].rgbAdd.dwPixelCode, edx
		ASSUME	ebx:NOTHING
		add	ebx, (SIZEOF E3D_COLOR)
		mov	pDstNextColor, ebx
		pop	edx
		pop	eax
	.ENDIF
	pop	ebx
	ASSUME	edi:NOTHING
	BYTE	0C3H	; ret

;
;	色をクリップするサブ関数
; ----------------------------------------------------------------------------
ALIGN	10H
SubFunc_ClipColor:
	fld	rClipPos
	push	ebx
	push	edx
	fmul	realConstant256
	mov	ebx, pSrcNextColor
	mov	edx, pDstNextColor
	fistp	nClipPos
	ASSUME	ebx:PTR E3D_COLOR
	ASSUME	edx:PTR E3D_COLOR
	push	eax
	push	ecx
	FOR	@MEMBER, \
		<rgbMul.rgb.Blue, rgbMul.rgb.Green, rgbMul.rgb.Red, \
		 rgbAdd.rgb.Blue, rgbAdd.rgb.Green, rgbAdd.rgb.Red>
		movzx	eax, [ebx].@MEMBER
		movzx	ecx, clrLastColor.@MEMBER
		sub	eax, ecx
		imul	eax, nClipPos
		sar	eax, 8
		add	eax, ecx
		cmp	eax, 100H
		.IF	!CARRY?
			sar	eax, 31
			not	eax
		.ENDIF
		mov	[edx].@MEMBER, al
	ENDM
	add	edx, (SIZEOF E3D_COLOR)
	pop	ecx
	pop	eax
	mov	pDstNextColor, edx
	ASSUME	ebx:NOTHING
	ASSUME	edx:NOTHING
	pop	edx
	pop	ebx
	BYTE	0C3H	; ret


;
;	Ｙ座標をクリップするサブ関数
; ----------------------------------------------------------------------------
ALIGN	10H
SubFunc_ClipVectorY:
	ASSUME	esi:PCE3D_VECTOR_2D
	ASSUME	edi:PTR E3D_VECTOR_2D
	fld	REAL4 PTR [ecx]
	fld	st(0)
	fsub	vxLastPoint.y
	fld	[esi].y
	fsub	vxLastPoint.y
	fdivp	st(1), st
	fst	rClipPos
	fld	[esi].x
	fsub	vxLastPoint.x
	fmulp	st(1), st
	fadd	vxLastPoint.x
	fstp	[edi].x
	fstp	[edi].y
	add	edi, (SIZEOF E3D_VECTOR_2D)
	cmp	pSrcNextColor, 0
	jnz	SubFunc_ClipColor
	BYTE	0C3H	; ret
	ASSUME	esi:NOTHING
	ASSUME	edi:NOTHING

;
;	Ｘ座標をクリップするサブ関数
; ----------------------------------------------------------------------------
ALIGN	10H
SubFunc_ClipVectorX:
	ASSUME	esi:PCE3D_VECTOR_2D
	ASSUME	edi:PTR E3D_VECTOR_2D
	fld	REAL4 PTR [ecx]
	fld	st(0)
	fsub	vxLastPoint.x
	fld	[esi].x
	fsub	vxLastPoint.x
	fdivp	st(1), st
	fst	rClipPos
	fld	[esi].y
	fsub	vxLastPoint.y
	fmulp	st(1), st
	fadd	vxLastPoint.y
	fstp	[edi].y
	fstp	[edi].x
	add	edi, (SIZEOF E3D_VECTOR_2D)
	cmp	pSrcNextColor, 0
	jnz	SubFunc_ClipColor
	BYTE	0C3H	; ret
	ASSUME	esi:NOTHING
	ASSUME	edi:NOTHING

;
;	次の頂点へ進めるサブ関数
; ----------------------------------------------------------------------------
ALIGN	10H
SubFunc_NextSide1:
	ASSUME	esi:PTR EGL_POINT
;	mov	ecx, spSideParam1.nIndex
;	mov	esi, pPolyVertexes
	cmp	ecx, spSideParam2.nIndex
	jz	SubFunc_NextSideExit
SubFunc_NextSide1First:
	inc	ecx
	.IF	ecx >= nVertexCount
		xor	ecx, ecx
	.ENDIF
	lea	esi, [esi + ecx * 8]
	mov	spSideParam1.nIndex, ecx
	mov	ecx, [esi].y
	lea	edi, spSideParam1
	sar	ecx, 16
	sub	ecx, nCurrentY
	jz	SubFunc_NextSideParam1
	jns	SubFunc_NextSideParam
	call	SubFunc_CopySideParam
	mov	ecx, spSideParam1.nIndex
	mov	esi, pPolyVertexes
	cmp	ecx, spSideParam2.nIndex
	jnz	SubFunc_NextSide1First
	;
	ASSUME	edi:PTR EGL_SIDE_PARAM
	mov	eax, [edi].nLastPosX
	mov	edx, [edi].nLastPosY
	mov	[edi].nNextPosX, eax
	mov	[edi].nNextPosY, edx
	mov	eax, [edi].lastColor.rgbMul.dwPixelCode
	mov	edx, [edi].lastColor.rgbAdd.dwPixelCode
	mov	[edi].nextColor.rgbMul.dwPixelCode, eax
	mov	[edi].nextColor.rgbAdd.dwPixelCode, edx
	xor	eax, eax
	inc	eax
	ASSUME	esi:NOTHING
	BYTE	0C3H	; ret

ALIGN	10H
SubFunc_NextSide2:
	ASSUME	esi:PTR EGL_POINT
;	mov	ecx, spSideParam2.nIndex
;	mov	esi, pPolyVertexes
	cmp	ecx, spSideParam1.nIndex
	jz	SubFunc_NextSideExit
SubFunc_NextSide2First:
	dec	ecx
	.IF	SIGN?
		mov	ecx, nVertexCount
		dec	ecx
	.ENDIF
	lea	esi, [esi + ecx * 8]
	mov	spSideParam2.nIndex, ecx
	mov	ecx, [esi].y
	lea	edi, spSideParam2
	sar	ecx, 16
	sub	ecx, nCurrentY
	jz	SubFunc_NextSideParam1
	jns	SubFunc_NextSideParam
	call	SubFunc_CopySideParam
	mov	ecx, spSideParam2.nIndex
	mov	esi, pPolyVertexes
	cmp	ecx, spSideParam1.nIndex
	jnz	SubFunc_NextSide2First
	;
	ASSUME	edi:PTR EGL_SIDE_PARAM
	mov	eax, [edi].nLastPosX
	mov	edx, [edi].nLastPosY
	mov	[edi].nNextPosX, eax
	mov	[edi].nNextPosY, edx
	mov	eax, [edi].lastColor.rgbMul.dwPixelCode
	mov	edx, [edi].lastColor.rgbAdd.dwPixelCode
	mov	[edi].nextColor.rgbMul.dwPixelCode, eax
	mov	[edi].nextColor.rgbAdd.dwPixelCode, edx
	xor	eax, eax
	inc	eax
	ASSUME	esi:NOTHING
	BYTE	0C3H	; ret

SubFunc_NextSideExit:
	xor	eax, eax
	inc	eax
	BYTE	0C3H	; ret

ALIGN	10H
SubFunc_CopySideParam:
	ASSUME	edi:PTR EGL_SIDE_PARAM
	ASSUME	ebx:PCE3D_COLOR
	ASSUME	esi:PTR EGL_POINT
	mov	edx, [edi].nIndex
	mov	esi, pPolyVertexes
	mov	ebx, pVertexColors
	;
	mov	eax, [edi].nNextPosX
	mov	ecx, [edi].nNextPosY
	mov	[edi].nLastPosX, eax
	mov	[edi].nLastPosY, ecx
	mov	eax, [edi].nextColor.rgbMul.dwPixelCode
	mov	ecx, [edi].nextColor.rgbAdd.dwPixelCode
	mov	[edi].lastColor.rgbMul.dwPixelCode, eax
	mov	[edi].lastColor.rgbAdd.dwPixelCode, ecx
	;
	fild	[esi + edx * 8].y
	fild	[esi + edx * 8].x
	mov	eax, [esi + edx * 8].x
	mov	ecx, [esi + edx * 8].y
	fst	[edi].floatPosX
	mov	[edi].nNextPosX, eax
	mov	[edi].nNextPosY, ecx
	fstp	[edi].floatNextPosX
	fstp	[edi].floatNextPosY
	mov	[edi].floatDltX, 0
	;
	.IF	ebx != NULL
		lea	ebx, [ebx + edx * 8]
		mov	eax, [ebx].rgbMul.dwPixelCode
		mov	edx, [ebx].rgbAdd.dwPixelCode
		mov	[edi].nextColor.rgbMul.dwPixelCode, eax
		mov	[edi].nextColor.rgbAdd.dwPixelCode, edx
		;
		movzx	eax, [edi].lastColor.rgbMul.rgb.Blue
		movzx	ecx, [edi].lastColor.rgbMul.rgb.Green
		movzx	edx, [edi].lastColor.rgbMul.rgb.Red
		shl	eax, 7
		shl	ecx, 7
		shl	edx, 7
		mov	[edi].curColor.rgbMul.Blue, ax
		mov	[edi].curColor.rgbMul.Green, cx
		mov	[edi].curColor.rgbMul.Red, dx
		movzx	eax, [edi].lastColor.rgbAdd.rgb.Blue
		movzx	ecx, [edi].lastColor.rgbAdd.rgb.Green
		movzx	edx, [edi].lastColor.rgbAdd.rgb.Red
		shl	eax, 7
		shl	ecx, 7
		shl	edx, 7
		mov	[edi].curColor.rgbAdd.Blue, ax
		mov	[edi].curColor.rgbAdd.Green, cx
		mov	[edi].curColor.rgbAdd.Red, dx
		mov	DWORD PTR [edi].dltColor.rgbMul[0], 0
		mov	DWORD PTR [edi].dltColor.rgbMul[4], 0
		mov	DWORD PTR [edi].dltColor.rgbAdd[0], 0
		mov	DWORD PTR [edi].dltColor.rgbAdd[4], 0
	.ENDIF
	ASSUME	ebx:NOTHING
	BYTE	0C3H	; ret

ALIGN	10H
SubFunc_NextSideParam1:
	ASSUME	esi:PTR EGL_POINT
	ASSUME	edi:PTR EGL_SIDE_PARAM
	mov	[edi].nLeftHeight, 1
	;
	mov	eax, [edi].nNextPosX
	mov	edx, [edi].nNextPosY
	mov	[edi].nLastPosX, eax
	mov	[edi].nLastPosY, edx
	and	edx, 0FFFFH
	neg	edx
	add	edx, 0FFFFH
	mov	nTemp[8], edx
	;
	mov	eax, [esi].x
	mov	edx, [esi].y
	mov	[edi].nNextPosX, eax
	mov	[edi].nNextPosY, edx
	sub	eax, [edi].nLastPosX
	sub	edx, [edi].nLastPosY
	mov	nTemp[0], eax
	mov	nTemp[4], edx
	;
	fild	[esi].x
	fstp	[edi].floatNextPosX
	fild	[esi].y
	fstp	[edi].floatNextPosY
	;
	fild	nTemp[8]
	fidiv	nTemp[4]
	;
	ASSUME	ebx:PCE3D_COLOR
	mov	ebx, pVertexColors
	.IF	ebx != NULL
		mov	ecx, [edi].nIndex
		lea	ebx, [ebx + ecx * 8]
		mov	eax, [edi].nextColor.rgbMul.dwPixelCode
		mov	edx, [edi].nextColor.rgbAdd.dwPixelCode
		mov	[edi].lastColor.rgbMul.dwPixelCode, eax
		mov	[edi].lastColor.rgbAdd.dwPixelCode, edx
		mov	eax, [ebx].rgbMul.dwPixelCode
		mov	edx, [ebx].rgbAdd.dwPixelCode
		mov	[edi].nextColor.rgbMul.dwPixelCode, eax
		mov	[edi].nextColor.rgbAdd.dwPixelCode, edx
		;
		fld	realConstant128
		fmul	st, st(1)
		fistp	nTemp[12]
		;
		FOR	@C, <Blue, Green, Red>
			movzx	eax, [edi].nextColor.rgbMul.rgb.@C
			movzx	ecx, [edi].lastColor.rgbMul.rgb.@C
			sub	eax, ecx
			shl	ecx, 7
			imul	eax, nTemp[12]
				movzx	edx, [edi].nextColor.rgbAdd.rgb.@C
			add	eax, ecx
				movzx	ecx, [edi].lastColor.rgbAdd.rgb.@C
			mov	[edi].curColor.rgbMul.@C, ax
				sub	edx, ecx
				shl	ecx, 7
				imul	edx, nTemp[12]
				add	edx, ecx
				mov	[edi].curColor.rgbAdd.@C, dx
		ENDM
		;
		xor	eax, eax
		mov	DWORD PTR [edi].dltColor.rgbMul[0], eax
		mov	DWORD PTR [edi].dltColor.rgbMul[4], eax
		mov	DWORD PTR [edi].dltColor.rgbAdd[0], eax
		mov	DWORD PTR [edi].dltColor.rgbAdd[4], eax
	.ENDIF
	ASSUME	ebx:NOTHING
	;
	fimul	nTemp[0]
	fiadd	[edi].nLastPosX
	fstp	[edi].floatPosX
	mov	[edi].floatDltX, 0
	;
	ASSUME	edi:NOTHING
	xor	eax, eax
	BYTE	0C3H	; ret

ALIGN	10H
SubFunc_NextSideParam:
	ASSUME	esi:PTR EGL_POINT
	ASSUME	edi:PTR EGL_SIDE_PARAM
	inc	ecx
	mov	[edi].nLeftHeight, ecx
	;
	mov	eax, [edi].nNextPosX
	mov	edx, [esi].x
	mov	[edi].nLastPosX, eax
	mov	[edi].nNextPosX, edx
	sub	edx, eax
	mov	nTemp[0], edx
	;
	mov	eax, [edi].nNextPosY
	mov	edx, [esi].y
	mov	[edi].nLastPosY, eax
	mov	[edi].nNextPosY, edx
	sub	edx, eax
	mov	nTemp[4], edx
	;
	fild	[edi].nNextPosX
	fstp	[edi].floatNextPosX
	fild	[edi].nNextPosY
	fstp	[edi].floatNextPosY
	;
	fld1
	fidiv	nTemp[4]
	;
	mov	eax, 0FFFFH
	mov	edx, [edi].nLastPosY
	and	edx, eax
	sub	eax, edx
	mov	nTemp[4], eax
	;
	ASSUME	ebx:PCE3D_COLOR
	mov	ebx, pVertexColors
	.IF	ebx != NULL
		mov	ecx, [edi].nIndex
		lea	ebx, [ebx + ecx * 8]
		mov	eax, [edi].nextColor.rgbMul.dwPixelCode
		mov	edx, [edi].nextColor.rgbAdd.dwPixelCode
		mov	[edi].lastColor.rgbMul.dwPixelCode, eax
		mov	[edi].lastColor.rgbAdd.dwPixelCode, edx
		mov	eax, [ebx].rgbMul.dwPixelCode
		mov	edx, [ebx].rgbAdd.dwPixelCode
		mov	[edi].nextColor.rgbMul.dwPixelCode, eax
		mov	[edi].nextColor.rgbAdd.dwPixelCode, edx
		;
		fld	realConst32768x65536
		fmul	st, st(1)
		fistp	nTemp[12]
		;
		FOR	@C, <Blue, Green, Red>
			movzx	eax, [edi].nextColor.rgbMul.rgb.@C
			movzx	ecx, [edi].lastColor.rgbMul.rgb.@C
			sub	eax, ecx
			imul	eax, nTemp[12]
			shl	ecx, 7
			sar	eax, 15 - 7
			mov	[edi].dltColor.rgbMul.@C, ax
			imul	eax, nTemp[4]
			sar	eax, 16
			add	eax, ecx
			mov	[edi].curColor.rgbMul.@C, ax
			;
			movzx	eax, [edi].nextColor.rgbAdd.rgb.@C
			movzx	ecx, [edi].lastColor.rgbAdd.rgb.@C
			sub	eax, ecx
			imul	eax, nTemp[12]
			shl	ecx, 7
			sar	eax, 15 - 7
			mov	[edi].dltColor.rgbAdd.@C, ax
			imul	eax, nTemp[4]
			sar	eax, 16
			add	eax, ecx
			mov	[edi].curColor.rgbAdd.@C, ax
		ENDM
	.ENDIF
	ASSUME	ebx:NOTHING
	;
	fimul	nTemp[0]
	fld	realConst65536
	fmul	st, st(1)
	fstp	[edi].floatDltX
	;
	fimul	nTemp[4]
	fiadd	[edi].nLastPosX
	fstp	[edi].floatPosX
	;
	ASSUME	esi:NOTHING
	ASSUME	edi:NOTHING
	xor	eax, eax
	BYTE	0C3H	; ret

eglNormalizePolygonRegion	ENDP


CodeSeg	ENDS

	END
