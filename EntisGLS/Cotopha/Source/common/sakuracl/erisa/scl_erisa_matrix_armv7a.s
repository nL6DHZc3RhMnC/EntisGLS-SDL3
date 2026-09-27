
	.global	ERISA_sclwConvertArraySWordToSByte_ARMv7A
	.global	ERISA_sclwConvertArraySWordToByte_ARMv7A
	.global	ERISA_sclwFastIDCT8x8_ARMv7A
	.global	ERISA_sclwubConvertYUVSubBlock8x8_ARMv7A
	.global	ERISA_sclwsbConvertYUVSubBlock8x8_ARMv7A
	.global	ERISA_sclwsbConvertYUVSubBlock8x8to16x16_ARMv7A
	.global	ERISA_sclbConvertYUVtoRGB8x8_ARMv7A
	.global	ERISA_sclbAddYUVtoRGB8x8_ARMv7A
	.global	ERISA_Sampling16x16RGBMovePBlock1_ARMv7A
	.global	ERISA_Sampling16x16RGBMovePBlock2_ARMv7A
	.global	ERISA_Sampling16x16RGBMovePBlock3_ARMv7A
	.global	ERISA_Sampling16x16RGBMoveBBlock0_ARMv7A
	.global	ERISA_Sampling16x16RGBMoveBBlock1_ARMv7A
	.global	ERISA_Sampling16x16RGBMoveBBlock2_ARMv7A
	.global	ERISA_Sampling16x16RGBMoveBBlock3_ARMv7A
	.global	ERISA_ImageFilterHalf1111_ARMv7A

	.text
	.align	4

/******************************************************************************
			16ビット符号あり整数を8ビット符号あり整数に変換
 -----------------------------------------------------------------------------
	void ERISA_sclwConvertArraySWordToSByte_ARMv7A
	(
		int8_t *		ptrDst,
		const int16_t *	ptrSrc,
		size_t			nCount
	)
 *****************************************************************************/

ERISA_sclwConvertArraySWordToSByte_ARMv7A:
	CMP		R2, #0
	BEQ		ERISA_sclwConvertArraySWordToSByte_ARMv7A_Label1

	ERISA_sclwConvertArraySWordToSByte_ARMv7A_Loop1:
		LDRH	R3, [R1]
		ADD		R1, R1, #2
		SSAT16	R3, #8, R3
		STRB	R3, [R0]
		ADD		R0, R0, #1
		SUBS	R2, R2, #1
	BNE		ERISA_sclwConvertArraySWordToSByte_ARMv7A_Loop1

ERISA_sclwConvertArraySWordToSByte_ARMv7A_Label1:
	BX		LR


/******************************************************************************
			16ビット符号あり整数を8ビット符号なし整数に変換
 -----------------------------------------------------------------------------
	void ERISA_sclwConvertArraySWordToByte_ARMv7A
	(
		uint8_t *		ptrDst,
		const int16_t *	ptrSrc,
		size_t			nCount
	)
 *****************************************************************************/

ERISA_sclwConvertArraySWordToByte_ARMv7A:
	CMP		R2, #0
	BEQ		ERISA_sclwConvertArraySWordToByte_ARMv7A_Label1

	ERISA_sclwConvertArraySWordToByte_ARMv7A_Loop1:
		LDRH	R3, [R1]
		ADD		R1, R1, #2
		USAT16	R3, #8, R3
		STRB	R3, [R0]
		ADD		R0, R0, #1
		SUBS	R2, R2, #1
	BNE		ERISA_sclwConvertArraySWordToByte_ARMv7A_Loop1

ERISA_sclwConvertArraySWordToByte_ARMv7A_Label1:
	BX		LR


/******************************************************************************
						高速 2 次元逆 DCT 変換
 -----------------------------------------------------------------------------
	void ERISA_sclwFastIDCT8x8_ARMv7A
	(
		int16_t *		ptrDst8x8,
		const int16_t *	ptrParamIDCT8x8,
	)
 *****************************************************************************/

ERISA_sclwFastIDCT8x8_ARMv7A:
	PUSH	{R4-R11}
	SUB		SP, SP, #128		/* int16_t wTemp[64] */

	MOV		R4, SP				/* pDst = &wTemp[0] */
	MOV		R5, R0				/* pSrc = ptrDst8x8 */
	MOV		R6, #0				/* i = 0 */

	sclwFastIDCT8x8_ARMv7a_Loop11:
		MOV		R7, R1			/* pwIDCT = ptrParamIDCT8x8 */
		MOV		R8, #0			/* j = 0 */

		sclwFastIDCT8x8_ARMv7a_Loop12:
			MOV		R9, #0			/* acc = 0 */
				MOV		R10, #0

			LDR		R2, [R7, #0]			/* += pwIDCT[0] * pSrc[0] */
			LDR		R3, [R5, #0]			/* += pwIDCT[1] * pSrc[1] */
				LDR		R11, [R7, #16]
			SMLAD	R9, R2, R3, R9
				SMLAD	R10, R11, R3, R10

			LDR		R2, [R7, #4]			/* += pwIDCT[2] * pSrc[2] */
			LDR		R3, [R5, #4]			/* += pwIDCT[3] * pSrc[3] */
				LDR		R11, [R7, #20]
			SMLAD	R9, R2, R3, R9
				SMLAD	R10, R11, R3, R10

			LDR		R2, [R7, #8]			/* += pwIDCT[4] * pSrc[4] */
			LDR		R3, [R5, #8]			/* += pwIDCT[5] * pSrc[5] */
				LDR		R11, [R7, #24]
			SMLAD	R9, R2, R3, R9
				SMLAD	R10, R11, R3, R10

			LDR		R2, [R7, #12]			/* += pwIDCT[6] * pSrc[6] */
			LDR		R3, [R5, #12]			/* += pwIDCT[7] * pSrc[7] */
				LDR		R11, [R7, #28]
			SMLAD	R9, R2, R3, R9
				SMLAD	R10, R11, R3, R10

			ASR		R9, R9, #14				/* pDst[j] = acc >> 14 */
			STRH	R9, [R4, +R8]
				ADD		R8, R8, #16			/* j += 8*2 */
				ASR		R10, R10, #14		/* pDst[j] = acc >> 14 */
				STRH	R10, [R4, +R8]

			ADD		R8, R8, #16			/* j += 8*2 */
			ADD		R7, R7, #32			/* pwIDCT += 8 * 2 */

		CMP		R8, #128			/* whlie ( j < 8*8*2 ) */
		BLT		sclwFastIDCT8x8_ARMv7a_Loop12

		ADD		R5, R5, #16		/* pSrc += 8 */
		ADD		R4, R4, #2		/* pDst += 1 */

	ADD		R6, R6, #1			/* whlie ( ++ i < 8 ) */
	CMP		R6, #8
	BLT		sclwFastIDCT8x8_ARMv7a_Loop11

	MOV		R4, R0				/* pDst = ptrDst8x8 */
	MOV		R5, SP				/* pSrc = &wTemp[0] */
	MOV		R6, #0				/* i = 0 */

	sclwFastIDCT8x8_ARMv7a_Loop21:
		MOV		R7, R1			/* pwIDCT = ptrParamIDCT8x8 */
		MOV		R8, #0			/* j = 0 */

		sclwFastIDCT8x8_ARMv7a_Loop22:
			MOV		R9, #0			/* acc = 0 */
				MOV		R10, #0

			LDR		R2, [R7, #0]	/* += pwIDCT[0] * pSrc[0] */
			LDR		R3, [R5, #0]	/* += pwIDCT[1] * pSrc[1] */
				LDR		R11, [R7, #16]
			SMLAD	R9, R2, R3, R9
				SMLAD	R10, R11, R3, R10

			LDR		R2, [R7, #4]	/* += pwIDCT[2] * pSrc[2] */
			LDR		R3, [R5, #4]	/* += pwIDCT[3] * pSrc[3] */
				LDR		R11, [R7, #20]
			SMLAD	R9, R2, R3, R9
				SMLAD	R10, R11, R3, R10

			LDR		R2, [R7, #8]	/* += pwIDCT[4] * pSrc[4] */
			LDR		R3, [R5, #8]	/* += pwIDCT[5] * pSrc[5] */
				LDR		R11, [R7, #24]
			SMLAD	R9, R2, R3, R9
				SMLAD	R10, R11, R3, R10

			LDR		R2, [R7, #12]	/* += pwIDCT[6] * pSrc[6] */
			LDR		R3, [R5, #12]	/* += pwIDCT[7] * pSrc[7] */
				LDR		R11, [R7, #28]
			SMLAD	R9, R2, R3, R9
				SMLAD	R10, R11, R3, R10

			ASR		R9, R9, #16		/* pDst[j] = acc >> 16 */
			STRH	R9, [R4, +R8]
				ADD		R8, R8, #16			/* j += 8*2 */
				ASR		R10, R10, #16		/* pDst[j] = acc >> 16 */
				STRH	R10, [R4, +R8]

			ADD		R8, R8, #16			/* j += 8*2 */
			ADD		R7, R7, #32		/* pwIDCT += 8 * 2 */

		CMP		R8, #128			/* whlie ( j < 8*8*2 ) */
		BLT		sclwFastIDCT8x8_ARMv7a_Loop22

		ADD		R5, R5, #16		/* pSrc += 8 */
		ADD		R4, R4, #2		/* pDst += 1 */

	ADD		R6, R6, #1			/* whlie ( ++ i < 8 ) */
	CMP		R6, #8
	BLT		sclwFastIDCT8x8_ARMv7a_Loop21

	ADD		SP, SP, #128
	POP		{R4-R11}
	BX		LR


/******************************************************************************
	4:4:4, 4:1:1  8x8->8x8  中間画像バッファに 1 チャネル書き出す
 -----------------------------------------------------------------------------
	void ERISA_sclwubConvertYUVSubBlock8x8_ARMv7A
	(
		uint8_t *		ptrYUVBlock,
		ssize_t			nYUVLineBytes,
		const int16_t *	pwSrcCahnnel
	)
 *****************************************************************************/

ERISA_sclwubConvertYUVSubBlock8x8_ARMv7A:
	PUSH	{R4-R7}
	MOV		R7, #8

	ERISA_sclwubConvertYUVSubBlock8x8_ARMv7A_Loop:

		LDR		R3, [R2]
		LDR		R4, [R2, #4]
		LDR		R5, [R2, #8]
		LDR		R6, [R2, #12]
		ADD		R2, R2, #16
		USAT16	R3, #8, R3
		USAT16	R4, #8, R4
		USAT16	R5, #8, R5
		USAT16	R6, #8, R6
		ORR		R3, R3, R3, LSR #8
		ORR		R4, R4, R4, LSR #8
		ORR		R5, R5, R5, LSR #8
		ORR		R6, R6, R6, LSR #8
		BFI		R3, R4, #16, #16
		BFI		R5, R6, #16, #16
		STR		R3, [R0]
		STR		R5, [R0, #4]
		ADD		R0, R0, R1

		SUBS	R7, R7, #1
	BNE		ERISA_sclwubConvertYUVSubBlock8x8_ARMv7A_Loop

	POP		{R4-R7}
	BX		LR


/******************************************************************************
	4:4:4, 4:1:1  8x8->8x8  中間画像バッファに 1 チャネル書き出す（符号あり）
 -----------------------------------------------------------------------------
	void ERISA_sclwsbConvertYUVSubBlock8x8_ARMv7A
	(
		int8_t *		ptrYUVBlock,
		ssize_t			nYUVLineBytes,
		const int16_t *	pwSrcCahnnel
	)
 *****************************************************************************/

ERISA_sclwsbConvertYUVSubBlock8x8_ARMv7A:
	PUSH	{R4-R7}
	MOV		R7, #8

	ERISA_sclwsbConvertYUVSubBlock8x8_ARMv7A_Loop:

		LDR		R3, [R2]
		LDR		R4, [R2, #4]
		LDR		R5, [R2, #8]
		LDR		R6, [R2, #12]
		ADD		R2, R2, #16
		SSAT16	R3, #8, R3
		SSAT16	R4, #8, R4
		SSAT16	R5, #8, R5
		SSAT16	R6, #8, R6
		UXTB16	R3, R3
		UXTB16	R4, R4
		UXTB16	R5, R5
		UXTB16	R6, R6
		ORR		R3, R3, R3, LSR #8
		ORR		R4, R4, R4, LSR #8
		ORR		R5, R5, R5, LSR #8
		ORR		R6, R6, R6, LSR #8
		BFI		R3, R4, #16, #16
		BFI		R5, R6, #16, #16
		STR		R3, [R0]
		STR		R5, [R0, #4]
		ADD		R0, R0, R1

		SUBS	R7, R7, #1
	BNE		ERISA_sclwsbConvertYUVSubBlock8x8_ARMv7A_Loop

	POP		{R4-R7}
	BX		LR


/******************************************************************************
	4:1:1  8x8->16x16  中間画像バッファに 1 チャネル書き出す（符号あり）
 -----------------------------------------------------------------------------
	void ERISA_sclwsbConvertYUVSubBlock8x8to16x16_ARMv7A
	(
		int8_t *		ptrYUVBlock,
		ssize_t			nYUVBlockBytes,
		ssize_t			nYUVLineBytes,
		const int16_t *	pwSrcCahnnel
	)
 *****************************************************************************/

ERISA_sclwsbConvertYUVSubBlock8x8to16x16_ARMv7A:
	PUSH	{R4-R8}
	MOV		R8, #8
	SUB		R2, R2, R1

	ERISA_sclwsbConvertYUVSubBlock8x8to16x16_ARMv7A_Loop:

		LDR		R4, [R3]
		LDR		R5, [R3, #4]
		LDR		R6, [R3, #8]
		LDR		R7, [R3, #12]
		ADD		R3, R3, #16
		SSAT16	R4, #8, R4
		SSAT16	R5, #8, R5
		SSAT16	R6, #8, R6
		SSAT16	R7, #8, R7
		UXTB16	R4, R4
		UXTB16	R5, R5
		UXTB16	R6, R6
		UXTB16	R7, R7
		ORR		R4, R4, R4, LSL #8
		ORR		R5, R5, R5, LSL #8
		ORR		R6, R6, R6, LSL #8
		ORR		R7, R7, R7, LSL #8

		STR		R4, [R0]
		STR		R5, [R0, #4]
		ADD		R0, R0, R1
		STR		R6, [R0]
		STR		R7, [R0, #4]
		ADD		R0, R0, R2

		STR		R4, [R0]
		STR		R5, [R0, #4]
		ADD		R0, R0, R1
		STR		R6, [R0]
		STR		R7, [R0, #4]
		ADD		R0, R0, R2

		SUBS	R8, R8, #1
	BNE		ERISA_sclwsbConvertYUVSubBlock8x8to16x16_ARMv7A_Loop

	POP		{R4-R8}
	BX		LR


/******************************************************************************
		YUV->RGB 8x8 変換（{Y[8],U[8],V[8],A[8]},...->{R,G,B,A},...）
 -----------------------------------------------------------------------------
	void ERISA_sclbConvertYUVtoRGB8x8_ARMv7A
	(
		uint8_t *		ptrRGBLine,
		const int8_t *	ptrYUVLine,
		size_t			widthInPacked
	)
 *****************************************************************************/

ERISA_sclbConvertYUVtoRGB8x8_ARMv7A:
	PUSH	{R4-R12}
	CMP		R2, #0
	BEQ		ERISA_sclbConvertYUVtoRGB8x8_ARMv7A_LabelExit

	ERISA_sclbConvertYUVtoRGB8x8_ARMv7A_Loop1:
		PUSH	{R2}
		MOV		R2, #8
		ERISA_sclbConvertYUVtoRGB8x8_ARMv7A_Loop2:
			LDRB	R4, [R1, #0]			/* Y */
				LDRB	R3, [R1, #1]
			LDRSB	R5, [R1, #8]			/* U */
				LDRSB	R9, [R1, #9]
			LDRSB	R6, [R1, #16]			/* V */
				LDRSB	R10, [R1, #17]
			LSL		R7, R5, #3				/* R7 = U*7 */
				LSL		R11, R9, #3
			SUB		R7, R5
				SUB		R11, R9
			ADD		R5, R5, R5, LSL #1		/* R5 = U*3 */
				ADD		R9, R9, R9, LSL #1
			ADD		R6, R6, R6, LSL #1		/* R6 = V*3 */
				ADD		R10, R10, R10, LSL #1
			ADD		R8, R4, R7, ASR #2		/* B = Y + U*7 / 4 */
				ADD		R12, R3, R11, ASR #2
			ADD		R5, R5, R6, LSL #1		/* R5 = U*3 + V*6 */
				ADD		R9, R9, R10, LSL #1
			ADD		R6, R4, R6, ASR #1		/* R = Y + V*3 / 2 */
				ADD		R10, R3, R10, ASR #1
			SUB		R5, R4, R5, ASR #3		/* G = Y - (U*3 + V*6) / 8 */
				SUB		R9, R3, R9, ASR #3
			LDRSB	R7, [R1, #24]			/* A */
				LDRSB	R11, [R1, #25]
			BFI		R8, R6, #16, #16
			BFI		R5, R7, #16, #16
				BFI		R12, R10, #16, #16
				BFI		R9, R11, #16, #16
			USAT16	R8, #8, R8
			USAT16	R5, #8, R5
				USAT16	R12, #8, R12
				USAT16	R9, #8, R9
			ORR		R8, R8, R5, LSL #8
				ORR		R12, R12, R9, LSL #8
			STR		R8, [R0, #0]
				STR		R12, [R0, #4]
			ADD		R1, R1, #2
			ADD		R0, R0, #8
			SUBS	R2, R2, #2
		BNE		ERISA_sclbConvertYUVtoRGB8x8_ARMv7A_Loop2

		POP		{R2}
		ADD		R1, R1, #24
		SUBS	R2, R2, #1
	BNE		ERISA_sclbConvertYUVtoRGB8x8_ARMv7A_Loop1

ERISA_sclbConvertYUVtoRGB8x8_ARMv7A_LabelExit:
	POP		{R4-R12}
	BX		LR


/******************************************************************************
	YUV->RGB 8x8 変換＆飽和加算（{Y[8],U[8],V[8],A[8]},...->{R,G,B,A},...）
 -----------------------------------------------------------------------------
	void ERISA_sclbAddYUVtoRGB8x8_ARMv7A
	(
		uint8_t *		ptrRGBLine,
		const int8_t *	ptrYUVLine,
		size_t			widthInPacked
	)
 *****************************************************************************/

ERISA_sclbAddYUVtoRGB8x8_ARMv7A:
	PUSH	{R4-R12}
	CMP		R2, #0
	BEQ		ERISA_sclbAddYUVtoRGB8x8_ARMv7A_LabelExit

	ERISA_sclbAddYUVtoRGB8x8_ARMv7A_Loop1:
		PUSH	{R2}
		MOV		R2, #8
		ERISA_sclbAddYUVtoRGB8x8_ARMv7A_Loop2:
			LDRSB	R4, [R1, #0]			/* Y */
				LDRSB	R3, [R1, #1]
			LDRSB	R5, [R1, #8]			/* U */
				LDRSB	R9, [R1, #9]
			LDRSB	R6, [R1, #16]			/* V */
				LDRSB	R10, [R1, #17]
			LSL		R7, R5, #3				/* R7 = U*7 */
				LSL		R11, R9, #3
			SUB		R7, R5
				SUB		R11, R9
			ADD		R5, R5, R5, LSL #1		/* R5 = U*3 */
				ADD		R9, R9, R9, LSL #1
			ADD		R6, R6, R6, LSL #1		/* R6 = V*3 */
				ADD		R10, R10, R10, LSL #1
			ADD		R8, R4, R7, ASR #2		/* B = Y + U*7 / 4 */
				ADD		R12, R3, R11, ASR #2
			ADD		R5, R5, R6, LSL #1		/* R5 = U*3 + V*6 */
				ADD		R9, R9, R10, LSL #1
			ADD		R6, R4, R6, ASR #1		/* R = Y + V*3 / 2 */
				ADD		R10, R3, R10, ASR #1
			SUB		R5, R4, R5, ASR #3		/* G = Y - (U*3 + V*6) / 8 */
				SUB		R9, R3, R9, ASR #3
			LDRSB	R7, [R1, #24]			/* A */
				LDRSB	R11, [R1, #25]

			BFI		R8, R6, #16, #16
			BFI		R5, R7, #16, #16
				BFI		R12, R10, #16, #16
				BFI		R9, R11, #16, #16

			SADD16	R8, R8, R8
			SADD16	R5, R5, R5
			SADD16	R12, R12, R12
			SADD16	R9, R9, R9

			LDR		R4, [R0, #0]
				LDR		R3, [R0, #4]
			UXTAB16	R8, R8, R4
			UXTAB16	R5, R5, R4, ROR #8
				UXTAB16	R12, R12, R3
				UXTAB16	R9, R9, R3, ROR #8

			USAT16	R8, #8, R8
			USAT16	R5, #8, R5
				USAT16	R12, #8, R12
				USAT16	R9, #8, R9
			ORR		R8, R8, R5, LSL #8
				ORR		R12, R12, R9, LSL #8
			STR		R8, [R0, #0]
				STR		R12, [R0, #4]
			ADD		R1, R1, #2
			ADD		R0, R0, #8

			SUBS	R2, R2, #2
		BNE		ERISA_sclbAddYUVtoRGB8x8_ARMv7A_Loop2

		POP		{R2}
		ADD		R1, R1, #24
		SUBS	R2, R2, #1
	BNE		ERISA_sclbAddYUVtoRGB8x8_ARMv7A_Loop1

ERISA_sclbAddYUVtoRGB8x8_ARMv7A_LabelExit:
	POP		{R4-R12}
	BX		LR


/******************************************************************************
			動画 Pブロック・サンプリング関数（水平方向半整数画素）
 -----------------------------------------------------------------------------
	void ERISA_Sampling16x16RGBMovePBlock1_ARMv7A
	(
		uint8_t *		pDstImage,
		int32_t			pitchDstLine,
		const uint8_t *	pSrcImage,
		int32_t			pitchSrcLine
	)
 *****************************************************************************/

ERISA_Sampling16x16RGBMovePBlock1_ARMv7A:
	PUSH	{R4-R8}
	MOV		R4, #16
	SUB		R1, R1, #64
	SUB		R3, R3, #64

	ERISA_Sampling16x16RGBMovePBlock1_ARMv7A_Loop1:
		LDR		R5, [R2]
		MOV		R8, #4

		ERISA_Sampling16x16RGBMovePBlock1_ARMv7A_Loop2:
			LDR		R6, [R2, #4]
			LDR		R7, [R2, #8]
				UHADD8	R5, R5, R6
				UHADD8	R6, R6, R7
					STR		R5, [R0]
			LDR		R5, [R2, #12]
					STR		R6, [R0, #4]
			LDR		R6, [R2, #16]
				UHADD8	R7, R7, R5
				UHADD8	R5, R5, R6
					STR		R7, [R0, #8]
			ADD		R2, R2, #16
					STR		R5, [R0, #12]
			MOV		R5, R6
			ADD		R0, R0, #16
			SUBS	R8, R8, #1
		BNE		ERISA_Sampling16x16RGBMovePBlock1_ARMv7A_Loop2

		ADD		R0, R0, R1
		ADD		R2, R2, R3
		SUBS	R4, R4, #1
	BNE		ERISA_Sampling16x16RGBMovePBlock1_ARMv7A_Loop1

	POP		{R4-R8}
	BX		LR


/******************************************************************************
			動画 Pブロック・サンプリング関数（垂直方向半整数画素）
 -----------------------------------------------------------------------------
	void ERISA_Sampling16x16RGBMovePBlock2_ARMv7A
	(
		uint8_t *		pDstImage,
		int32_t			pitchDstLine,
		const uint8_t *	pSrcImage,
		int32_t			pitchSrcLine
	)
 *****************************************************************************/

ERISA_Sampling16x16RGBMovePBlock2_ARMv7A:
	PUSH	{R4-R9}
	MOV		R5, #16
	ADD		R4, R2, R3
	SUB		R1, R1, #64
	SUB		R3, R3, #64

	ERISA_Sampling16x16RGBMovePBlock2_ARMv7A_Loop1:
		MOV		R9, #4

		ERISA_Sampling16x16RGBMovePBlock2_ARMv7A_Loop2:
			LDR		R6, [R2]
			LDR		R7, [R4]
			LDR		R8, [R2, #4]
				UHADD8	R6, R6, R7
			LDR		R7, [R4, #4]
					STR		R6, [R0]
						LDR		R6, [R2, #8]
				UHADD8	R8, R8, R7
						LDR		R7, [R4, #8]
					STR		R8, [R0, #4]
						LDR		R8, [R2, #12]
							UHADD8	R6, R6, R7
						LDR		R7, [R4, #12]
								STR		R6, [R0, #8]
							UHADD8	R8, R8, R7
			ADD		R2, R2, #16
			ADD		R4, R4, #16
					STR		R8, [R0, #12]
			ADD		R0, R0, #16
			SUBS	R9, R9, #1
		BNE		ERISA_Sampling16x16RGBMovePBlock2_ARMv7A_Loop2

		ADD		R0, R0, R1
		ADD		R2, R2, R3
		ADD		R4, R4, R3
		SUBS	R5, R5, #1
	BNE		ERISA_Sampling16x16RGBMovePBlock2_ARMv7A_Loop1

	POP		{R4-R9}
	BX		LR


/******************************************************************************
		動画 Pブロック・サンプリング関数（水平・垂直方向半整数画素）
 -----------------------------------------------------------------------------
	void ERISA_Sampling16x16RGBMovePBlock3_ARMv7A
	(
		uint8_t *		pDstImage,
		int32_t			pitchDstLine,
		const uint8_t *	pSrcImage,
		int32_t			pitchSrcLine
	)
 *****************************************************************************/

ERISA_Sampling16x16RGBMovePBlock3_ARMv7A:
	PUSH	{R4-R11}
	MOV		R5, #16
	ADD		R4, R2, R3
	SUB		R1, R1, #64
	SUB		R3, R3, #64

	ERISA_Sampling16x16RGBMovePBlock3_ARMv7A_Loop1:
		LDR		R6, [R2]
		LDR		R7, [R4]
			UHADD8	R6, R6, R7
		MOV		R9, #4

		ERISA_Sampling16x16RGBMovePBlock3_ARMv7A_Loop2:

			LDR		R7, [R2, #4]
			LDR		R8, [R4, #4]
				UHADD8	R7, R7, R8
						LDR		R10, [R2, #8]
						LDR		R11, [R4, #8]
					UHADD8	R6, R6, R7
							UHADD8	R10, R10, R11
					STR		R6, [R0]
								UHADD8	R6, R7, R10
								STR		R6, [R0, #4]
			LDR		R7, [R2, #12]
			LDR		R8, [R4, #12]
				UHADD8	R7, R7, R8
			LDR		R10, [R2, #16]
			LDR		R11, [R4, #16]
					UHADD8	R6, R7, R10
				UHADD8	R10, R10, R11
					STR		R6, [R0, #8]
			ADD		R2, R2, #16
					UHADD8	R6, R7, R10
			ADD		R4, R4, #16
					STR		R6, [R0, #12]
			ADD		R0, R0, #16
					MOV		R6, R10
			SUBS	R9, R9, #1
		BNE		ERISA_Sampling16x16RGBMovePBlock3_ARMv7A_Loop2

		ADD		R0, R0, R1
		ADD		R2, R2, R3
		ADD		R4, R4, R3
		SUBS	R5, R5, #1
	BNE		ERISA_Sampling16x16RGBMovePBlock3_ARMv7A_Loop1

	POP		{R4-R11}
	BX		LR


/******************************************************************************
					動画 Bブロック・サンプリング関数
 -----------------------------------------------------------------------------
	void ERISA_Sampling16x16RGBMoveBBlock0_ARMv7A
	(
		uint8_t *		pDstImage,
		int32_t			pitchDstLine,
		const uint8_t *	pSrcImage,
		int32_t			pitchSrcLine
	)
 *****************************************************************************/

ERISA_Sampling16x16RGBMoveBBlock0_ARMv7A:
	PUSH	{R4-R9}
	MOV		R4, #16
	SUB		R1, R1, #64
	SUB		R3, R3, #64

	ERISA_Sampling16x16RGBMoveBBlock0_ARMv7A_Loop1:
		MOV		R5, #4

		ERISA_Sampling16x16RGBMoveBBlock0_ARMv7A_Loop2:
			LDR		R6, [R0]
			LDR		R7, [R2]
			LDR		R8, [R0, #4]
			LDR		R9, [R2, #4]
				UHADD8	R6, R6, R7
				UHADD8	R8, R8, R9
					STR		R6, [R0]
					STR		R8, [R0, #4]
			LDR		R6, [R0, #8]
			LDR		R7, [R2, #8]
			LDR		R8, [R0, #12]
			LDR		R9, [R2, #12]
				UHADD8	R6, R6, R7
				UHADD8	R8, R8, R9
					STR		R6, [R0, #8]
					STR		R8, [R0, #12]
			ADD		R0, R0, #16
			ADD		R2, R2, #16
			SUBS	R5, R5, #1
		BNE		ERISA_Sampling16x16RGBMoveBBlock0_ARMv7A_Loop2

		ADD		R0, R0, R1
		ADD		R2, R2, R3
		SUBS	R4, R4, #1
	BNE		ERISA_Sampling16x16RGBMoveBBlock0_ARMv7A_Loop1

	POP		{R4-R9}
	BX		LR


/******************************************************************************
			動画 Bブロック・サンプリング関数（水平方向半整数画素）
 -----------------------------------------------------------------------------
	void ERISA_Sampling16x16RGBMoveBBlock1_ARMv7A
	(
		uint8_t *		pDstImage,
		int32_t			pitchDstLine,
		const uint8_t *	pSrcImage,
		int32_t			pitchSrcLine
	)
 *****************************************************************************/

ERISA_Sampling16x16RGBMoveBBlock1_ARMv7A:
	PUSH	{R4-R10}
	MOV		R4, #16
	SUB		R1, R1, #64
	SUB		R3, R3, #64

	ERISA_Sampling16x16RGBMoveBBlock1_ARMv7A_Loop1:
		LDR		R6, [R2]
		MOV		R5, #8

		ERISA_Sampling16x16RGBMoveBBlock1_ARMv7A_Loop2:
			LDR		R7, [R2, #4]
			LDR		R8, [R0]
			LDR		R9, [R2, #8]
				UHADD8	R6, R6, R7
			LDR		R10, [R0, #4]
					UHADD8	R8, R8, R6
				UHADD8	R7, R7, R9
			MOV		R6, R9
						STR		R8, [R0]
					UHADD8	R10, R10, R7
			ADD		R2, R2, #16
						STR		R10, [R0, #4]
			ADD		R0, R0, #8
			SUBS	R5, R5, #1
		BNE		ERISA_Sampling16x16RGBMoveBBlock1_ARMv7A_Loop2

		ADD		R0, R0, R1
		ADD		R2, R2, R3
		SUBS	R4, R4, #1
	BNE		ERISA_Sampling16x16RGBMoveBBlock1_ARMv7A_Loop1

	POP		{R4-R10}
	BX		LR


/******************************************************************************
			動画 Bブロック・サンプリング関数（垂直方向半整数画素）
 -----------------------------------------------------------------------------
	void ERISA_Sampling16x16RGBMoveBBlock2_ARMv7A
	(
		uint8_t *		pDstImage,
		int32_t			pitchDstLine,
		const uint8_t *	pSrcImage,
		int32_t			pitchSrcLine
	)
 *****************************************************************************/

ERISA_Sampling16x16RGBMoveBBlock2_ARMv7A:
	PUSH	{R4-R10}
	MOV		R5, #16
	ADD		R4, R2, R3
	SUB		R1, R1, #64
	SUB		R3, R3, #64

	ERISA_Sampling16x16RGBMoveBBlock2_ARMv7A_Loop1:
		MOV		R9, #8

		ERISA_Sampling16x16RGBMoveBBlock2_ARMv7A_Loop2:
			LDR		R6, [R2]
			LDR		R7, [R4]
			LDR		R8, [R0]
				UHADD8	R6, R6, R7
			LDR		R10, [R0, #4]
					UHADD8	R6, R6, R8
			LDR		R8, [R2, #4]
			LDR		R7, [R4, #4]
			ADD		R2, R2, #8
			ADD		R4, R4, #8
				UHADD8	R8, R8, R7
						STR		R6, [R0]
					UHADD8	R8, R8, R10
						STR		R8, [R0, #4]
			ADD		R0, R0, #8
			SUBS	R9, R9, #1
		BNE		ERISA_Sampling16x16RGBMoveBBlock2_ARMv7A_Loop2

		ADD		R0, R0, R1
		ADD		R2, R2, R3
		ADD		R4, R4, R3
		SUBS	R5, R5, #1
	BNE		ERISA_Sampling16x16RGBMoveBBlock2_ARMv7A_Loop1

	POP		{R4-R10}
	BX		LR


/******************************************************************************
		動画 Pブロック・サンプリング関数（水平・垂直方向半整数画素）
 -----------------------------------------------------------------------------
	void ERISA_Sampling16x16RGBMoveBBlock3_ARMv7A
	(
		uint8_t *		pDstImage,
		int32_t			pitchDstLine,
		const uint8_t *	pSrcImage,
		int32_t			pitchSrcLine
	)
 *****************************************************************************/

ERISA_Sampling16x16RGBMoveBBlock3_ARMv7A:
	PUSH	{R4-R11}
	MOV		R5, #16
	ADD		R4, R2, R3
	SUB		R1, R1, #64
	SUB		R3, R3, #64

	ERISA_Sampling16x16RGBMoveBBlock3_ARMv7A_Loop1:
		LDR		R6, [R2]
		LDR		R7, [R4]
			UHADD8	R6, R6, R7
		MOV		R9, #8

		ERISA_Sampling16x16RGBMoveBBlock3_ARMv7A_Loop2:

			LDR		R7, [R2, #4]
			LDR		R8, [R4, #4]
				UHADD8	R7, R7, R8
			LDR		R8, [R2, #8]
					LDR		R10, [R0]
					UHADD8	R6, R6, R7
			LDR		R11, [R4, #8]
						UHADD8	R6, R6, R10
				UHADD8	R11, R11, R8
					LDR		R10, [R0]
							STR		R6, [R0]
					UHADD8	R7, R7, R11
			ADD		R2, R2, #8
						UHADD8	R7, R7, R10
			ADD		R4, R4, #8
							STR		R7, [R0, #4]
			ADD		R0, R0, #8
							MOV		R6, R11
			SUBS	R9, R9, #1
		BNE		ERISA_Sampling16x16RGBMoveBBlock3_ARMv7A_Loop2

		ADD		R0, R0, R1
		ADD		R2, R2, R3
		ADD		R4, R4, R3
		SUBS	R5, R5, #1
	BNE		ERISA_Sampling16x16RGBMoveBBlock3_ARMv7A_Loop1

	POP		{R4-R11}
	BX		LR


/******************************************************************************
						画像半画素フィルタ
 -----------------------------------------------------------------------------
	void ERISA_ImageFilterHalf1111_ARMv7A
	(
		uint8_t *		pDstImage,
		int32_t			pitchDstLine,
		const uint8_t *	pSrcImage,
		int32_t			pitchSrcLine,
		size_t			widthImage,
		size_t			heightImage
	)
 *****************************************************************************/

arg_widthImage		= (8+0)*4
arg_heightImage		= (8+1)*4
local_pDstImage		= (-1)*4
local_pitchDstLine	= (-2)*4
local_pSrcImage		= (-3)*4
local_pitchSrcLine	= (-4)*4
local_yLoop			= (-5)*4
local_size			= (5)*4

ERISA_ImageFilterHalf1111_ARMv7A:
	PUSH	{R4-R11}
	MOV		R11, SP
	SUB		SP, SP, #local_size
	STR		R1, [R11, #local_pitchDstLine]
	STR		R3, [R11, #local_pitchSrcLine]

	LDR		R9, [R11, #arg_heightImage]
	CMP		R9, #0
	BEQ		ERISA_ImageFilterHalf1111_ARMv7A_LabelExit
	LDR		R10, [R11, #arg_widthImage]
	CMP		R10, #0
	BEQ		ERISA_ImageFilterHalf1111_ARMv7A_LabelExit

	SUBS	R9, R9, #1
	BEQ		ERISA_ImageFilterHalf1111_ARMv7A_Loop1_Break

	ERISA_ImageFilterHalf1111_ARMv7A_Loop1:
		LDR		R10, [R11, #arg_widthImage]
		STR		R0, [R11, #local_pDstImage]
		STR		R2, [R11, #local_pSrcImage]
		STR		R9, [R11, #local_yLoop]
		ADD		R4, R2, R3

		SUBS	R10, R10, #1
		BEQ		ERISA_ImageFilterHalf1111_ARMv7A_Loop2_Break
		LDR		R5, [R2]
		LDR		R6, [R4]
		UHADD8	R5, R5, R6

		LSR		R8, R10, #2
		AND		R10, R10, #0x03
		CMP		R8, #0
		BEQ		ERISA_ImageFilterHalf1111_ARMv7A_Loop2_x4_Break

		ERISA_ImageFilterHalf1111_ARMv7A_Loop2_x4:
			LDR		R1, [R2]
			LDR		R6, [R2, #4]
			LDR		R3, [R4]
			LDR		R7, [R4, #4]
				UHADD8	R1, R1, R3
			LDR		R3, [R4, #8]
				UHADD8	R6, R6, R7
			LDR		R7, [R4, #12]
					UHADD8	R5, R5, R1
					UHADD8	R1, R1, R6
						STR		R5, [R0]
						STR		R1, [R0, #4]
						MOV		R5, R6
			LDR		R1, [R2, #8]
			LDR		R6, [R2, #12]
				UHADD8	R1, R1, R3
				UHADD8	R6, R6, R7
			ADD		R2, #16
					UHADD8	R5, R5, R1
			ADD		R4, #16
					UHADD8	R1, R1, R6
						STR		R5, [R0, #8]
						STR		R1, [R0, #12]
						MOV		R5, R6
			ADD		R0, #16
			SUBS	R8, R8, #1
		BNE		ERISA_ImageFilterHalf1111_ARMv7A_Loop2_x4

	ERISA_ImageFilterHalf1111_ARMv7A_Loop2_x4_Break:
		CMP		R10, #0
		BEQ		ERISA_ImageFilterHalf1111_ARMv7A_Loop2_Break

		ERISA_ImageFilterHalf1111_ARMv7A_Loop2:
			LDR		R6, [R2, #4]!
			LDR		R7, [R4, #4]!
				UHADD8	R6, R6, R7
					UHADD8	R5, R5, R6
						STR		R5, [R0], #4
						MOV		R5, R6
			SUBS	R10, R10, #1
		BNE		ERISA_ImageFilterHalf1111_ARMv7A_Loop2

	ERISA_ImageFilterHalf1111_ARMv7A_Loop2_Break:
		LDR		R5, [R2]
		STR		R5, [R0]

		LDR		R0, [R11, #local_pDstImage]
		LDR		R1, [R11, #local_pitchDstLine]
		LDR		R2, [R11, #local_pSrcImage]
		LDR		R3, [R11, #local_pitchSrcLine]
		LDR		R9, [R11, #local_yLoop]
		ADD		R0, R0, R1
		ADD		R2, R2, R3
		SUBS	R9, R9, #1
	BNE		ERISA_ImageFilterHalf1111_ARMv7A_Loop1

ERISA_ImageFilterHalf1111_ARMv7A_Loop1_Break:
	LDR		R10, [R11, #arg_widthImage]

	ERISA_ImageFilterHalf1111_ARMv7A_Loop3:
		LDR		R4, [R2], #4
		STR		R4, [R0], #4
		SUBS	R10, R10, #1
	BNE		ERISA_ImageFilterHalf1111_ARMv7A_Loop3

ERISA_ImageFilterHalf1111_ARMv7A_LabelExit:
	ADD		SP, SP, #local_size
	POP		{R4-R11}
	BX		LR



