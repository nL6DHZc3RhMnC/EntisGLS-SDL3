
	.global	ERISA_sclwShuffleVectorMul8x8_ARM_NEON
	.global	ERISA_sclwFastIDCT8x8_ARM_NEON
	.global	ERISA_sclbConvertYUVtoRGB8x8_ARM_NEON
	.global	ERISA_sclbAddYUVtoRGB8x8_ARM_NEON
	.global	ERISA_sclfMoveVertexAndCubeRange_ARM_NEON

	.text
	.align	4

/******************************************************************************
					サブブロック逆量子化
 -----------------------------------------------------------------------------
	void ERISA_sclwShuffleVectorMul8x8_ARM_NEON
	(
		int16_t *		ptrDstArray,
		uint32_t *		ptrDstIndex,
		const int16_t *	ptrSrcArray,
		const int16_t *	ptrMulParams,
		int				nCoefficient
	)
 *****************************************************************************/

ERISA_sclwShuffleVectorMul8x8_ARM_NEON:
	PUSH	{R4-R10}

	LDR			R4, [SP, #28]
	VDUP.32		Q3, R4

	MOV			R4, #8
	ERISA_sclwShuffleVectorMul8x8_ARM_NEON_Loop1:

		VLDR		D0, [R2]
		VLDR		D1, [R3]
		VLDR		D2, [R2, #8]
		VLDR		D3, [R3, #8]
		VMULL.S16	Q0, D0, D1
			LDR			R7, [R1]
			LDR			R8, [R1, #4]
		VMULL.S16	Q1, D2, D3
			ADD			R7, R0, R7, LSL #1
			ADD			R8, R0, R8, LSL #1

			LDR			R9, [R1, #8]
		VSHL.S32	Q0, Q0, Q3
			LDR			R10, [R1, #12]
		VMOV		R5, R6, D0
			ADD			R9, R0, R9, LSL #1
			ADD			R10, R0, R10, LSL #1
		STRH		R5, [R7]
		STRH		R6, [R8]
			LDR			R7, [R1, #16]
			LDR			R8, [R1, #20]
		VMOV		R5, R6, D1
			ADD			R7, R0, R7, LSL #1
			ADD			R8, R0, R8, LSL #1
				VSHL.S32	Q1, Q1, Q3
		STRH		R5, [R9]
		STRH		R6, [R10]

			LDR			R9, [R1, #24]
			LDR			R10, [R1, #28]
		VMOV		R5, R6, D2
			ADD			R9, R0, R9, LSL #1
			ADD			R10, R0, R10, LSL #1
		STRH		R5, [R7]
		STRH		R6, [R8]
		VMOV		R5, R6, D3
			ADD			R1, R1, #32
			ADD			R2, R2, #16
			ADD			R3, R3, #16
		STRH		R5, [R9]
		STRH		R6, [R10]

		SUBS		R4, R4, #1
	BNE		ERISA_sclwShuffleVectorMul8x8_ARM_NEON_Loop1

	POP		{R4-R10}
	BX		LR


/******************************************************************************
						高速 2 次元逆 DCT 変換
 -----------------------------------------------------------------------------
	void ERISA_sclwFastIDCT8x8_ARM_NEON
	(
		int16_t *		ptrDst8x8,
		const int16_t *	ptrParamIDCT8x8,
	)
 *****************************************************************************/

ERISA_sclwFastIDCT8x8_ARM_NEON:
	PUSH	{R4-R8}
	SUB		SP, SP, #128		/* int16_t wTemp[64] */

	MOV		R4, SP				/* pDst = &wTemp[0] */
	MOV		R5, R0				/* pSrc = ptrDst8x8 */
	MOV		R6, #0				/* i = 0 */

	ERISA_sclwFastIDCT8x8_ARM_NEON_Loop11:
		MOV		R7, R1			/* pwIDCT = ptrParamIDCT8x8 */
		MOV		R8, #0			/* j = 0 */

		ERISA_sclwFastIDCT8x8_ARM_NEON_Loop12:
			VLDR		D0, [R7, #0]
			VLDR		D2, [R7, #8]
			VLDR		D5, [R5, #0]
			VLDR		D7, [R5, #8]
			VMULL.S16	Q0, D0, D5
			VMULL.S16	Q1, D2, D7
				VLDR		D4, [R7, #16]
				VLDR		D6, [R7, #24]
				VMULL.S16	Q2, D4, D5
			VADD.I32	Q0, Q1
				VMULL.S16	Q3, D6, D7
			VADD.I32	D0, D1
			VMOV		R2, R3, D0
				VADD.I32	Q2, Q3
			ADD		R2, R2, R3
				VADD.I32	D4, D5
			ASR		R2, R2, #14			/* pDst[j] = acc >> 14 */
			STRH	R2, [R4, +R8]
				VMOV		R2, R3, D4
			ADD		R8, R8, #16			/* j += 8*2 */
				ADD		R2, R2, R3
			ADD		R7, R7, #32			/* pwIDCT += 8*2*2 */
				ASR		R2, R2, #14
				STRH	R2, [R4, +R8]
		ADD		R8, R8, #16				/* j += 8*2 */
		CMP		R8, #128				/* whlie ( j < 8*8*2 ) */
		BLT		ERISA_sclwFastIDCT8x8_ARM_NEON_Loop12

		ADD		R5, R5, #16		/* pSrc += 8 */
		ADD		R4, R4, #2		/* pDst += 1 */

	ADD		R6, R6, #1			/* whlie ( ++ i < 8 ) */
	CMP		R6, #8
	BLT		ERISA_sclwFastIDCT8x8_ARM_NEON_Loop11

	MOV		R4, R0				/* pDst = ptrDst8x8 */
	MOV		R5, SP				/* pSrc = &wTemp[0] */
	MOV		R6, #0				/* i = 0 */

	ERISA_sclwFastIDCT8x8_ARM_NEON_Loop21:
		MOV		R7, R1			/* pwIDCT = ptrParamIDCT8x8 */
		MOV		R8, #0			/* j = 0 */

		ERISA_sclwFastIDCT8x8_ARM_NEON_Loop22:
			VLDR		D0, [R7, #0]
			VLDR		D2, [R7, #8]
			VLDR		D5, [R5, #0]
			VLDR		D7, [R5, #8]
			VMULL.S16	Q0, D0, D5
			VMULL.S16	Q1, D2, D7
				VLDR		D4, [R7, #16]
				VLDR		D6, [R7, #24]
				VMULL.S16	Q2, D4, D5
			VADD.I32	Q0, Q1
				VMULL.S16	Q3, D6, D7
			VADD.I32	D0, D1
			VMOV		R2, R3, D0
				VADD.I32	Q2, Q3
			ADD		R2, R2, R3
				VADD.I32	D4, D5
			ASR		R2, R2, #16		/* pDst[j] = acc >> 16 */
			STRH	R2, [R4, +R8]
				VMOV		R2, R3, D4
			ADD		R8, R8, #16			/* j += 8*2 */
				ADD		R2, R2, R3
			ADD		R7, R7, #32			/* pwIDCT += 8*2*2 */
				ASR		R2, R2, #16
				STRH	R2, [R4, +R8]
		ADD		R8, R8, #16			/* j += 8*2 */
		CMP		R8, #128			/* whlie ( j < 8*8*2 ) */
		BLT		ERISA_sclwFastIDCT8x8_ARM_NEON_Loop22

		ADD		R5, R5, #16		/* pSrc += 8 */
		ADD		R4, R4, #2		/* pDst += 1 */

	ADD		R6, R6, #1			/* whlie ( ++ i < 8 ) */
	CMP		R6, #8
	BLT		ERISA_sclwFastIDCT8x8_ARM_NEON_Loop21

	ADD		SP, SP, #128
	POP		{R4-R8}
	BX		LR



/******************************************************************************
		YUV->RGB 8x8 変換（{Y[8],U[8],V[8],A[8]},...->{R,G,B,A},...）
 -----------------------------------------------------------------------------
	void ERISA_sclbConvertYUVtoRGB8x8_ARM_NEON
	(
		uint8_t *		ptrRGBLine,
		const int8_t *	ptrYUVLine,
		size_t			widthInPacked
	)
 *****************************************************************************/

ERISA_sclbConvertYUVtoRGB8x8_ARM_NEON:
	VPUSH	{S16-S31}
	CMP		R2, #0
	BEQ		ERISA_sclbConvertYUVtoRGB8x8_ARM_NEON_LabelExit

	ERISA_sclbConvertYUVtoRGB8x8_ARM_NEON_Loop1:

		VLDR		D0, [R1]
		VEOR		D1, D1, D1
		VLDR		D3, [R1, #8]
		VEOR		D2, D2, D2
		VLDR		D5, [R1, #16]
		VEOR		D4, D4, D4
		VLDR		D6, [R1, #24]
		VZIP.8		D0, D1			/* Q0 := y */
		VZIP.8		D2, D3			/* Q1 := u3 */
		VZIP.8		D4, D5			/* Q2 := v3 */
									/* Q3 := a */
		VSHR.S16	Q1, Q1, #8
		VSHR.S16	Q2, Q2, #8

		VSHL.I16	Q4, Q1, #3		/* Q4 = u7 = (u3 << 3) - u3 */
		VSHL.I16	Q5, Q1, #1
		VSHL.I16	Q6, Q2, #1
		VSUB.I16	Q4, Q4, Q1
		VADD.I16	Q1, Q1, Q5
		VADD.I16	Q2, Q2, Q6

		VSHR.S16	Q4, Q4, #2		/* Q4 = y + (u7 >> 2) */
		VADD.I16	Q1, Q1, Q2		/* Q5 = y - ((u3 + v3 + v3) >> 3) */
		VADD.I16	Q4, Q4, Q0
		VADD.I16	Q1, Q1, Q2
		VMOV		Q5, Q0
		VSHR.S16	Q1, Q1, #3
		VSHR.S16	Q2, Q2, #1		/* Q2 = y + (v3 >> 1) */
		VSUB.I16	Q5, Q5, Q1
		VADD.I16	Q2, Q2, Q0

		VQSHLU.S16	Q4, Q4, #8
		VQSHLU.S16	Q5, Q5, #8
		VQSHLU.S16	Q2, Q2, #8
		VUZP.8		D8, D9
		VUZP.8		D10, D11
		VUZP.8		D4, D5
		VMOV		D8, D9
		VMOV		D9, D11
		VMOV		D4, D5
		VMOV		D5, D6
		VZIP.8		D8, D9	/* Q4 = g7:b7:g6:b6:g5:b5:g4:b4:g3:b3:g2:b2:g1:b1:g0:b0 */
		VZIP.8		D4, D5	/* Q2 = a7:r7:a6:r6:a5:r5:a4:r4:a3:r3:a2:r2:a1:r1:a0:r0 */
		VZIP.16		Q4, Q2	/* Q4 = a3:r3:g3:b3:a2:r2:g2:b2:a1:r1:g1:b1:a0:r0:g0:b0 */
							/* Q2 = a7:r7:g7:b7:a6:r6:g6:b6:a5:r5:g5:b5:a4:r4:g4:b4 */
		VSTR		D8, [R0]
		VSTR		D9, [R0, #8]
		VSTR		D4, [R0, #16]
		VSTR		D5, [R0, #24]

		ADD			R0, R0, #32
		ADD			R1, R1, #32
		SUBS		R2, R2, #1
	BNE		ERISA_sclbConvertYUVtoRGB8x8_ARM_NEON_Loop1

ERISA_sclbConvertYUVtoRGB8x8_ARM_NEON_LabelExit:
	VPOP	{S16-S31}
	BX		LR



/******************************************************************************
	YUV->RGB 8x8 変換＆飽和加算（{Y[8],U[8],V[8],A[8]},...->{R,G,B,A},...）
 -----------------------------------------------------------------------------
	void ERISA_sclbAddYUVtoRGB8x8_ARM_NEON
	(
		uint8_t *		ptrRGBLine,
		const int8_t *	ptrYUVLine,
		size_t			widthInPacked
	)
 *****************************************************************************/

ERISA_sclbAddYUVtoRGB8x8_ARM_NEON:
	VPUSH	{S16-S31}
	CMP		R2, #0
	BEQ		ERISA_sclbAddYUVtoRGB8x8_ARM_NEON_LabelExit

	ERISA_sclbAddYUVtoRGB8x8_ARM_NEON_Loop1:

		VLDR		D1, [R1]
		VEOR		D0, D0, D0
		VLDR		D3, [R1, #8]
		VEOR		D2, D2, D2
		VLDR		D5, [R1, #16]
		VEOR		D4, D4, D4
		VLDR		D7, [R1, #24]
		VEOR		D6, D6, D6
		VZIP.8		D0, D1
		VZIP.8		D2, D3
		VZIP.8		D4, D5
		VZIP.8		D6, D7
		VSHR.S16	Q0, Q0, #7		/* Q0 := y */
		VSHR.S16	Q1, Q1, #7		/* Q1 := u3 */
		VSHR.S16	Q2, Q2, #7		/* Q2 := v3 */
		VSHR.S16	Q3, Q3, #7		/* Q3 := a */

		VMOV		Q4, Q1
		VMOV		Q5, Q1
		VMOV		Q6, Q2
		VSHL.I16	Q4, Q4, #3		/* Q4 = u7 = (u3 << 3) - u3 */
		VSHL.I16	Q5, Q5, #1
		VSHL.I16	Q6, Q6, #1
		VSUB.I16	Q4, Q4, Q1
		VADD.I16	Q1, Q1, Q5
		VADD.I16	Q2, Q2, Q6

		VSHR.S16	Q4, Q4, #2		/* Q4 = y + (u7 >> 2) */
		VADD.I16	Q1, Q1, Q2		/* Q5 = y - ((u3 + v3 + v3) >> 3) */
		VADD.I16	Q4, Q4, Q0
		VADD.I16	Q1, Q1, Q2
		VMOV		Q5, Q0
		VSHR.S16	Q1, Q1, #3
		VSHR.S16	Q2, Q2, #1		/* Q2 = y + (v3 >> 1) */
		VSUB.I16	Q5, Q5, Q1
		VADD.I16	Q2, Q2, Q0

		VZIP.16		Q4, Q5			/* Q4 = g3:b3:g2:b2:g1:b1:g0:b0 */
		VMOV		Q0, Q5			/* Q0 = g7:b7:g6:b6:g5:b5:g4:b4 */
		VZIP.16		Q2, Q3			/* Q2 = a3:r3:a2:r2:a1:r1:a0:r0 */
		VMOV		Q6, Q3			/* Q6 = a7:r7:a6:r6:a5:r5:a4:r4 */

		VZIP.32		Q4, Q2			/* Q4 = a1:r1:g1:b1:a0:r0:g0:b0 */
		VMOV		Q5, Q2			/* Q5 = a3:r3:g3:b3:a2:r2:g2:b2 */
		VLDR		D4, [R0]
		VEOR		D5, D5, D5
		VLDR		D2, [R0, #8]
		VEOR		D3, D3, D3
		VZIP.8		D4, D5
		VZIP.8		D2, D3
		VADD.I16	Q4, Q2
		VADD.I16	Q5, Q1
		VQSHLU.S16	Q4, Q4, #8
		VQSHLU.S16	Q5, Q5, #8
		VUZP.8		Q4, Q5
		VSTR		D10, [R0]
		VSTR		D11, [R0, #8]

		VZIP.32		Q0, Q6			/* Q0 = a5:r5:g5:b5:a4:r4:g4:b4 */
		VMOV		Q5, Q6			/* Q5 = a7:r7:g7:b7:a6:r6:g6:b6 */
		VLDR		D4, [R0, #16]
		VEOR		D5, D5, D5
		VLDR		D2, [R0, #24]
		VEOR		D3, D3, D3
		VZIP.8		D4, D5
		VZIP.8		D2, D3
		VADD.I16	Q0, Q2
		VADD.I16	Q5, Q1
		VQSHLU.S16	Q0, Q0, #8
		VQSHLU.S16	Q5, Q5, #8
		VUZP.8		Q0, Q5
		VSTR		D10, [R0, #16]
		VSTR		D11, [R0, #24]

		ADD			R0, R0, #32
		ADD			R1, R1, #32
		SUBS		R2, R2, #1
	BNE		ERISA_sclbAddYUVtoRGB8x8_ARM_NEON_Loop1

ERISA_sclbAddYUVtoRGB8x8_ARM_NEON_LabelExit:
	VPOP	{S16-S31}
	BX		LR





/******************************************************************************
	ベクトル配列の複製と最大・最小値の取得
 -----------------------------------------------------------------------------
	void ERISA_sclfMoveVertexAndCubeRange_ARM_NEON
	(
		S3DVector4 *		pvMaxMin,
		S3DVector4 *		pvDst,
		const S3DVector4 *	pvSrc,
		size_t				nCount
	)
 *****************************************************************************/

ERISA_sclfMoveVertexAndCubeRange_ARM_NEON:

	PUSH	{R4-R5}

	LDR		R4, [R2]
	LDR		R5, [R2, #4]
	STR		R4, [R1]
	STR		R5, [R1, #4]
	VMOV	D0, R4, R5

	LDR		R4, [R2, #8]
	LDR		R5, [R2, #12]
	STR		R4, [R1, #8]
	STR		R5, [R1, #12]
	VMOV	D1, R4, R5

	VMOV	D2, D0
	VMOV	D3, D1
	ADD		R2, R2, #16
	ADD		R1, R1, #16
	SUBS	R3, R3, #1
	BEQ		ERISA_sclfMoveVertexAndCuveRange_ARM_NEON_LoopEnd

ERISA_sclfMoveVertexAndCuveRange_ARM_NEON_LoopBegin:
		LDR		R4, [R2]
		LDR		R5, [R2, #4]
		STR		R4, [R1]
		STR		R5, [R1, #4]
		VMOV	D4, R4, R5

		LDR		R4, [R2, #8]
		LDR		R5, [R2, #12]
		STR		R4, [R1, #8]
		STR		R5, [R1, #12]
		VMOV	D5, R4, R5

		VMAX.F32	D0, D0, D4
		VMAX.F32	D1, D1, D5
		VMIN.F32	D2, D2, D4
		VMIN.F32	D3, D3, D5

		ADD		R2, R2, #16
		ADD		R1, R1, #16
		SUBS	R3, R3, #1
	BNE		ERISA_sclfMoveVertexAndCuveRange_ARM_NEON_LoopBegin

ERISA_sclfMoveVertexAndCuveRange_ARM_NEON_LoopEnd:

	VMOV	R4, R5, D0
	STR		R4, [R0]
	STR		R5, [R0, #4]
	VMOV	R4, R5, D1
	STR		R4, [R0, #4]

	VMOV	R4, R5, D2
	STR		R4, [R0, #16]
	STR		R5, [R0, #20]
	VMOV	R4, R5, D3
	STR		R4, [R0, #24]

	POP		{R4-R5}
	BX		LR

