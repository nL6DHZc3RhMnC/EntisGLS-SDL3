
; ****************************************************************************
;             Entis Generalized Library System version 3
; ----------------------------------------------------------------------------
;	Copyright (C) 1998-2005 Leshade Entis.  All right reserved.
; ****************************************************************************


	.686
	.XMM
	.MODEL	FLAT

	INCLUDE	experi.inc
	INCLUDE	esl.inc

NULL	EQU	0
IF	@Version LE 615
MMWORD	TYPEDEF	QWORD
ENDIF


; ----------------------------------------------------------------------------
;	WAVEFORMATEX 構造体
; ----------------------------------------------------------------------------

WAVEFORMATEX	STRUCT
wFormatTag	WORD	?	; /* format type */
nChannels	WORD	?	; /* number of channels (i.e. mono, stereo...) */
nSamplesPerSec	DWORD	?	; /* sample rate */
nAvgBytesPerSec	DWORD	?	; /* for buffer estimation */
nBlockAlign	WORD	?	; /* block size of data */
wBitsPerSample	WORD	?	; /* number of bits per sample of mono data */
cbSize		WORD	?	; /* the count in bytes of the size of */
				; /* extra information (after cbSize) */
WAVEFORMATEX	ENDS

WAVE_FORMAT_PCM	EQU	1


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
;	変換サイズ取得
; ----------------------------------------------------------------------------
ALIGN	10H
glsSound_GetConversionSize	PROC	NEAR32 C PUBLIC USES ebx esi edi,
		pDstFormat:PTR WAVEFORMATEX,
		pSrcFormat:PTR WAVEFORMATEX,
		nSampleCount:DWORD,
		fSizeSpecification:DWORD

	mov	esi, pSrcFormat
	mov	edi, pDstFormat
	ASSUME	esi:PTR WAVEFORMATEX
	ASSUME	edi:PTR WAVEFORMATEX

	.IF	fSizeSpecification == 0
		;
		; 出力サンプル数から入力サンプル数を計算
		;
		mov	eax, nSampleCount
		mul	[esi].nSamplesPerSec
		div	[edi].nSamplesPerSec
		add	edx, 0FFFFFFFFH
		adc	eax, 0
		.IF	ZERO? && (nSampleCount != 0)
			mov	eax, 1
		.ENDIF

	.ELSE
		;
		; 入力サンプル数から出力サンプル数を計算
		;
		mov	eax, nSampleCount
		mul	[edi].nSamplesPerSec
		div	[esi].nSamplesPerSec
		.IF	(eax == 0) && (nSampleCount != 0)
			mov	eax, 1
		.ENDIF

	.ENDIF

	ASSUME	esi:NOTHING
	ASSUME	edi:NOTHING
	ret

glsSound_GetConversionSize	ENDP

;
;	音声フォーマット変換
; ----------------------------------------------------------------------------

ALIGN	10H
glsSound_ConvertPCMFormat	PROC	NEAR32 C PUBLIC USES ebx esi edi,
		pDstFormat:PTR WAVEFORMATEX,
		pDstPCM:PTR,
		pSrcFormat:PTR WAVEFORMATEX,
		pSrcPCM:PTR,
		nSampleCount:DWORD

	LOCAL	ptrBuf[2]:PTR
	LOCAL	nDstSampleCount:DWORD, nFreqScale:DWORD, nFreqStep:DWORD
	LOCAL	nDstSampleBits:DWORD, nSrcSampleBits:DWORD
	LOCAL	nDstChannels:DWORD, nSrcChannels:DWORD
	LOCAL	nDstFrequency:DWORD, nSrcFrequency:DWORD

	.IF	nSampleCount == 0
		xor	eax, eax
		ret
	.ENDIF
	;
	;	音声フォーマット取得
	; --------------------------------------------------------------------
	mov	esi, pSrcFormat
	mov	edi, pDstFormat
	mov	ptrBuf[0], NULL
	mov	ptrBuf[4], NULL
	ASSUME	esi:PTR WAVEFORMATEX
	ASSUME	edi:PTR WAVEFORMATEX
	;
	movzx	eax, [esi].nChannels
	movzx	edx, [edi].nChannels
	mov	nSrcChannels, eax
	mov	nDstChannels, edx
	.IF	((eax != 1) && (eax != 2)) || ((edx != 1) && (edx != 2))
		mov	eax, eslErrInvalidParam
		ret
	.ENDIF
	;
	movzx	eax, [esi].wBitsPerSample
	movzx	edx, [edi].wBitsPerSample
	mov	nSrcSampleBits, eax
	mov	nDstSampleBits, edx
	.IF	((eax != 8) && (eax != 16)) || ((edx != 8) && (edx != 16))
		mov	eax, eslErrInvalidParam
		ret
	.ENDIF
	;
	mov	eax, [esi].nSamplesPerSec
	mov	edx, [edi].nSamplesPerSec
	mov	nSrcFrequency, eax
	mov	nDstFrequency, edx
	;
	ASSUME	esi:NOTHING
	ASSUME	edi:NOTHING
	;
	; 入力サンプル数を取得
	;
	mov	eax, nSampleCount
	mov	nDstSampleCount, eax
	mul	nSrcFrequency
	div	nDstFrequency
	add	edx, 0FFFFFFFFH
	adc	eax, 0
	mov	nSampleCount, eax

	;
	;	ビット分解能・チャネル数を変換
	; --------------------------------------------------------------------
	mov	eax, nSrcChannels
	mov	ecx, nSrcFrequency
	mov	edx, nSrcSampleBits
	.IF	(eax != nDstChannels) || \
			(edx != nDstSampleBits) || (ecx != nDstFrequency)
		mov	eax, nSampleCount
		lea	eax, [eax * 4 + 10H]
		INVOKE	eslHeapAllocate , NULL, eax, 0
		mov	ptrBuf[0], eax
		mov	edi, eax
		;
		mov	esi, pSrcPCM
		mov	eax, nSrcChannels
		mov	edx, nSrcSampleBits
		mov	ecx, nSampleCount
		;
		.IF	edx == 8
		.IF	eax == 1
			;
			; 1 channel, 8 bit PCM -> 2 channel 16 bit PCM
			;
			.REPEAT
				movzx	eax, BYTE PTR [esi]
				inc	esi
				sub	eax, 80H
				shl	eax, 8
				mov	WORD PTR [edi], ax
				mov	WORD PTR [edi + 2], ax
				add	edi, 4
				dec	ecx
			.UNTIL	ZERO?
			mov	eax, ptrBuf[0]
		.ELSE
			;
			; 2 channel, 8 bit PCM -> 2 channel 16 bit PCM
			;
			.REPEAT
				movzx	eax, BYTE PTR [esi]
				movzx	edx, BYTE PTR [esi + 1]
				add	esi, 2
				sub	eax, 80H
				sub	edx, 80H
				shl	eax, 8
				shl	edx, 8
				mov	WORD PTR [edi], ax
				mov	WORD PTR [edi + 2], dx
				add	edi, 4
				dec	ecx
			.UNTIL	ZERO?
			mov	eax, ptrBuf[0]
		.ENDIF
		.ELSE
		.IF	eax == 1
			;
			; 1 channel, 16 bit PCM -> 2 channel 16 bit PCM
			;
			.REPEAT
				mov	ax, WORD PTR [esi]
				add	esi, 2
				mov	WORD PTR [edi], ax
				mov	WORD PTR [edi + 2], ax
				add	edi, 4
				dec	ecx
			.UNTIL	ZERO?
			mov	eax, ptrBuf[0]
		.ELSE
			;
			; 2 channel, 16 bit PCM -> 2 channel 16 bit PCM
			;
			.REPEAT
				mov	eax, DWORD PTR [esi]
				add	esi, 4
				mov	DWORD PTR [edi], eax
				add	edi, 4
				dec	ecx
			.UNTIL	ZERO?
			mov	eax, ptrBuf[0]
		.ENDIF
		.ENDIF
		;
		mov	edx, nSampleCount
		mov	nSrcChannels, 2
		mov	nSrcSampleBits, 16
		mov	pSrcPCM, eax
		mov	ecx, DWORD PTR [eax + edx * 4 - 4]
		mov	DWORD PTR [eax + edx * 4], ecx
	.ENDIF

	;
	;	周波数を変換
	; --------------------------------------------------------------------
	mov	edx, nSrcFrequency
	.IF	edx != nDstFrequency
		ASSERT	<(nSrcChannels == 2) && (nSrcSampleBits == 16)>, \
			"(nSrcChannels == 2) && (nSrcSampleBits == 16)"
		mov	eax, nDstSampleCount
		lea	eax, [eax * 4 + 10H]
		INVOKE	eslHeapAllocate , NULL, eax, 0
		mov	ptrBuf[4], eax
		mov	esi, pSrcPCM
		mov	edi, eax
		;
		; 周波数変換スケールを取得
		;
		mov	eax, nSrcFrequency
		mov	edx, nDstFrequency
		xor	ecx, ecx
		.IF	eax < edx
			.REPEAT
				inc	ecx
				shl	eax, 1
			.UNTIL	eax >= edx
		.ELSE
			.REPEAT
				dec	ecx
				shl	edx, 1
			.UNTIL	eax <= edx
		.ENDIF
		;
		.IF	eax == edx
			test	ecx, ecx
			.IF	SIGN?
				;
				; ダウンサンプリング
				;
				neg	ecx
				mov	edx, nDstSampleCount
				mov	nFreqScale, ecx
				.REPEAT
					mov	ebx, 1
					mov	ecx, nFreqScale
					push	edx
					shl	ebx, cl
					;
					xor	eax, eax
					xor	edx, edx
					.REPEAT
						movsx	ecx, WORD PTR [esi]
						add	eax, ecx
						movsx	ecx, WORD PTR [esi]
						add	esi, 4
						add	edx, ecx
						dec	ebx
					.UNTIL	ZERO?
					mov	ecx, nFreqScale
					sar	eax, cl
					sar	edx, cl
					mov	WORD PTR [edi], ax
					mov	WORD PTR [edi + 2], dx
					;
					pop	edx
					add	edi, 4
					dec	edx
				.UNTIL	ZERO?
			.ELSE
				;
				; アップサンプリング
				;
				mov	eax, 1
				mov	nFreqScale, ecx
				shl	eax, cl
				mov	ecx, nDstSampleCount
				mov	nFreqStep, eax
				xor	eax, eax
				xor	edx, edx
				sub	ecx, nFreqStep
				.WHILE	!SIGN?
					push	ecx
					.IF	ecx != 0
						movsx	ecx, SWORD PTR [esi]
						movsx	ebx, SWORD PTR [esi + 2]
						movsx	eax, SWORD PTR [esi + 4]
						movsx	edx, SWORD PTR [esi + 6]
						sub	eax, ecx
						mov	ecx, nFreqScale
						sub	edx, ebx
						sar	eax, cl
						.IF	SIGN?
							adc	eax, 0
						.ENDIF
						sar	edx, cl
						.IF	SIGN?
							adc	edx, 0
						.ENDIF
					.ENDIF
					mov	ebx, nFreqStep
					push	esi
					mov	cx, WORD PTR [esi]
					mov	si, WORD PTR [esi + 2]
					;
					.REPEAT
						mov	WORD PTR [edi], cx
						mov	WORD PTR [edi + 2], si
						add	edi, 4
						add	cx, ax
						.IF	OVERFLOW?
							mov	cx, ax
							sar	cx, 15
							xor	cx, 7FFFH
						.ENDIF
						add	si, dx
						.IF	OVERFLOW?
							mov	si, dx
							sar	si, 15
							xor	si, 7FFFH
						.ENDIF
						dec	ebx
					.UNTIL	ZERO?
					;
					pop	esi
					pop	ecx
					add	esi, 4
					sub	ecx, nFreqStep
				.ENDW
				add	ecx, nFreqStep
				.IF	!ZERO?
					mov	eax, DWORD PTR [esi]
					.REPEAT
						mov	DWORD PTR [edi], eax
						add	edi, 4
						dec	ecx
					.UNTIL	ZERO?
				.ENDIF
			.ENDIF
		.ELSE
			;
			; 汎用リサンプリング
			;
			mov	eax, nSampleCount
			xor	edx, edx
			shld	edx, eax, 16
			shl	eax, 16
			div	nDstSampleCount
			mov	nFreqScale, eax
			;
			xor	eax, eax
			xor	edx, edx
			mov	ecx, nDstSampleCount
			.REPEAT
				push	ecx
				push	edx
				push	eax
				;
				mov	ecx, eax
				shrd	eax, edx, 16
				and	ecx, 0FFFFH
				.IF	ZERO?
					mov	eax, DWORD PTR [esi + eax * 4]
					mov	DWORD PTR [edi], eax
				.ELSE
					shr	ecx, 4
					movsx	ebx, SWORD PTR [esi + eax * 4 + 4]
					movsx	edx, SWORD PTR [esi + eax * 4]
					sub	ebx, edx
					imul	ebx, ecx
					sar	ebx, 12
					add	ebx, edx
					movsx	edx, SWORD PTR [esi + eax * 4 + 6]
					movsx	eax, SWORD PTR [esi + eax * 4 + 2]
					sub	edx, eax
					imul	edx, ecx
					sar	edx, 12
					add	edx, eax
					mov	WORD PTR [edi], bx
					mov	WORD PTR [edi + 2], dx
				.ENDIF
				;
				pop	eax
				pop	edx
				pop	ecx
				add	eax, nFreqScale
				adc	edx, 0
				add	edi, 4
				dec	ecx
			.UNTIL	ZERO?
		.ENDIF
		;
		mov	eax, ptrBuf[4]
		mov	ecx, nDstSampleCount
		mov	edx, nDstFrequency
		mov	pSrcPCM, eax
		mov	nSampleCount, ecx
		mov	nSrcFrequency, edx
	.ENDIF

	;
	;	最終的に出力フォーマットに変換
	; --------------------------------------------------------------------
	mov	eax, nSrcChannels
	mov	ecx, nSrcFrequency
	mov	edx, nSrcSampleBits
	mov	esi, pSrcPCM
	mov	edi, pDstPCM
	;
	.IF	(eax == nDstChannels) && \
			(edx == nDstSampleBits) && (ecx == nDstFrequency)
		;
		; メモリ転送
		;
		imul	eax, edx
		mov	ecx, nSampleCount
		shr	eax, 3
		imul	ecx, eax
		;
		sub	ecx, 8
		.WHILE	!SIGN?
			mov	eax, DWORD PTR [esi]
			mov	edx, DWORD PTR [esi + 4]
			add	esi, 8
			mov	DWORD PTR [edi], eax
			mov	DWORD PTR [edi + 4], edx
			add	edi, 8
			sub	ecx, 8
		.ENDW
		add	ecx, 8
		.WHILE	!ZERO?
			mov	al, BYTE PTR [esi]
			inc	esi
			mov	BYTE PTR [edi], al
			inc	edi
			dec	ecx
		.ENDW
	.ELSE
		;
		; 16 bit stereo から任意フォーマットへ変換
		;
		ASSERT	<ecx == nDstFrequency>, "nSrcFrequency == nDstFrequency"
		ASSERT	<(eax == 2) && (edx == 16)>, "(eax == 2) && (edx == 16)"
		mov	eax, nDstChannels
		mov	edx, nDstSampleBits
		mov	ecx, nSampleCount
		.IF	eax == 1
		.IF	edx == 8
			;
			; 16 bit stereo -> 8 bit mono
			;
			.REPEAT
				movsx	eax, SWORD PTR [esi]
				movsx	edx, SWORD PTR [esi + 2]
				add	esi, 4
				add	eax, edx
				sar	eax, 9
				add	eax, 80H
				mov	BYTE PTR [edi], al
				inc	edi
				dec	ecx
			.UNTIL	ZERO?
		.ELSE
			;
			; 16 bit stereo -> 16 bit mono
			;
			.REPEAT
				movsx	eax, SWORD PTR [esi]
				movsx	edx, SWORD PTR [esi + 2]
				add	esi, 4
				add	eax, edx
				sar	eax, 1
				mov	SWORD PTR [edi], ax
				add	edi, 2
				dec	ecx
			.UNTIL	ZERO?
		.ENDIF
		.ELSE
		.IF	edx == 8
			;
			; 16 bit stereo -> 8 bit stereo
			;
			.REPEAT
				mov	ax, SWORD PTR [esi]
				mov	dx, SWORD PTR [esi + 2]
				add	esi, 4
				sar	ax, 8
				sar	dx, 8
				add	al, 80H
				add	dl, 80H
				mov	BYTE PTR [edi], al
				mov	BYTE PTR [edi + 1], dl
				add	edi, 2
				dec	ecx
			.UNTIL	ZERO?
		.ELSE
			;
			; 16 bit stereo -> 16 bit stereo
			;
			.REPEAT
				mov	eax, DWORD PTR [esi]
				add	esi, 4
				mov	DWORD PTR [edi], eax
				add	edi, 4
				dec	ecx
			.UNTIL	ZERO?
		.ENDIF
		.ENDIF
	.ENDIF

	;
	;	関数終了
	; --------------------------------------------------------------------
	.IF	ptrBuf[0] != NULL
		INVOKE	eslHeapFree , NULL, ptrBuf[0], 0
	.ENDIF
	.IF	ptrBuf[4] != NULL
		INVOKE	eslHeapFree , NULL, ptrBuf[4], 0
	.ENDIF

	xor	eax, eax
	ret

glsSound_ConvertPCMFormat	ENDP

;
;	音声データをミキシング
; ----------------------------------------------------------------------------
ALIGN	10H
glsSound_MixPCMSamples	PROC	NEAR32 C PUBLIC USES ebx esi edi,
		pDstFormat:PTR WAVEFORMATEX,
		pDstPCM:PTR,
		pSrcPCM:PTR,
		pDstVolume:PTR REAL4,
		nSampleCount:DWORD

	LOCAL	nChannels:DWORD, nSampleBits:DWORD
	LOCAL	nVolume[2]:DWORD
	LOCAL	mmxVolume:QWORD

	;
	; フォーマット取得
	;
	mov	edi, pDstFormat
	ASSUME	edi:PTR WAVEFORMATEX
	movzx	eax, [edi].nChannels
	movzx	edx, [edi].wBitsPerSample
	mov	nChannels, eax
	mov	nSampleBits, edx
	.IF	(eax != 1) && (eax != 2)
		mov	eax, eslErrInvalidParam
		ret
	.ENDIF
	.IF	(edx != 8) && (edx != 16)
		mov	eax, eslErrInvalidParam
		ret
	.ENDIF
	ASSUME	edi:NOTHING
	.IF	nSampleCount == 0
		xor	eax, eax
		ret
	.ENDIF
	;
	; 音量取得
	;
	mov	edi, pDstVolume
	xor	edx, edx
	mov	nVolume[0], 0
	mov	nVolume[4], 0
	.REPEAT
		mov	eax, DWORD PTR [edi + edx * 4]
		test	eax, 7FFFFFFFH
		.IF	!ZERO?
			mov	ecx, eax
			and	eax, 007FFFFFH
			shr	ecx, 23
			or	eax, 00800000H
			and	ecx, 0FFH
			add	eax, 00080000H
			sub	ecx, 7FH + 23 - 16
			.IF	SIGN?
				neg	ecx
				and	eax, 0FFF00000H
				shr	eax, cl
				.IF	eax > 10000H
					mov	eax, 10000H
				.ENDIF
			.ELSE
				mov	eax, 10000H
			.ENDIF
		.ELSE
			xor	eax, eax
		.ENDIF
		.IF	(DWORD PTR [edi + edx * 4]) & 80000000H
			neg	eax
		.ENDIF
		mov	nVolume[edx * 4], eax
		inc	edx
	.UNTIL	edx >= nChannels
	;
	.IF	(nVolume[0] == 0) && (nVolume[4] == 0)
		xor	eax, eax
		ret
	.ENDIF
	;
	; ミキシング
	;
	mov	eax, nChannels
	mov	edx, nSampleBits
	mov	esi, pSrcPCM
	mov	edi, pDstPCM
	mov	ecx, nSampleCount
	;
	.IF	edx == 8
	.IF	eax == 1
		;
		; 8 bit mono PCM
		;
		mov	ebx, nVolume[0]
		.REPEAT
			movzx	eax, BYTE PTR [esi]
			inc	esi
			sub	eax, 80H
			movzx	edx, BYTE PTR [edi]
			imul	eax, ebx
			sar	eax, 16
			add	eax, edx
			.IF	SIGN?
				xor	eax, eax
			.ELSEIF	eax >= 100H
				mov	eax, 0FFH
			.ENDIF
			mov	BYTE PTR [edi], al
			inc	edi
			dec	ecx
		.UNTIL	ZERO?
	.ELSE
		;
		; 8 bit stereo PCM
		;
		.REPEAT
			movzx	eax, BYTE PTR [esi]
			inc	esi
			sub	eax, 80H
			movzx	edx, BYTE PTR [edi]
			imul	eax, nVolume[0]
			sar	eax, 16
			add	eax, edx
			.IF	SIGN?
				xor	eax, eax
			.ELSEIF	eax >= 100H
				mov	eax, 0FFH
			.ENDIF
			mov	BYTE PTR [edi], al
			inc	edi
			;
			movzx	eax, BYTE PTR [esi]
			inc	esi
			sub	eax, 80H
			movzx	edx, BYTE PTR [edi]
			imul	eax, nVolume[4]
			sar	eax, 16
			add	eax, edx
			.IF	SIGN?
				xor	eax, eax
			.ELSEIF	eax >= 100H
				mov	eax, 0FFH
			.ENDIF
			mov	BYTE PTR [edi], al
			inc	edi
			;
			dec	ecx
		.UNTIL	ZERO?
	.ENDIF
	.ELSE
	.IF	eax == 1
		;
		; 16 bit mono PCM
		;
		.IF	ERI_EnabledProcessorType & ERI_USE_MMX_PENTIUM
			movd	mm7, nVolume[0]
			psrad	mm7, 2
			punpcklwd	mm7, mm7
			punpckldq	mm7, mm7
			;
			sub	ecx, 8
			.WHILE	!SIGN?
				movq	mm0, MMWORD PTR [esi]
				movq	mm1, MMWORD PTR [esi + 8]
				pmulhw	mm0, mm7
				movq	mm2, MMWORD PTR [edi]
				pmulhw	mm1, mm7
				movq	mm3, MMWORD PTR [edi+ 8]
				paddsw	mm0, mm0
				paddsw	mm1, mm1
				paddsw	mm0, mm0
				paddsw	mm1, mm1
				add	esi, 16
				paddsw	mm0, mm2
				paddsw	mm1, mm3
				movq	MMWORD PTR [edi], mm0
				movq	MMWORD PTR [edi + 8], mm1
				add	edi, 16
				sub	ecx, 8
			.ENDW
			add	ecx, 8
			emms
		.ENDIF
		;
		mov	ebx, nVolume[0]
		sar	ebx, 1
		test	ecx, ecx
		.WHILE	!ZERO?
			movsx	eax, SWORD PTR [esi]
			imul	eax, ebx
			add	esi, 2
			movsx	edx, SWORD PTR [edi]
			sar	eax, 16 - 1
			add	eax, edx
			.IF	(SDWORD PTR eax) < -8000H
				mov	eax, -8000H
			.ELSEIF	(SDWORD PTR eax) > 7FFFH
				mov	eax, 7FFFH
			.ENDIF
			mov	SWORD PTR [edi], ax
			add	edi, 2
			dec	ecx
		.ENDW
	.ELSE
		;
		; 16 bit stereo PCM
		;
		mov	eax, nVolume[0]
		mov	edx, nVolume[4]
		.IF	((SDWORD PTR eax) < 0FFF0H) || ((SDWORD PTR edx) < 0FFF0H)
		.IF	ERI_EnabledProcessorType & ERI_USE_MMX_PENTIUM
			movd	mm7, nVolume[0]
			movd	mm6, nVolume[4]
			psrad	mm7, 2
			psrad	mm6, 2
			punpcklwd	mm7, mm6
			punpckldq	mm7, mm7
			;
			sub	ecx, 4
			.WHILE	!SIGN?
				movq	mm0, MMWORD PTR [esi]
				movq	mm1, MMWORD PTR [esi + 8]
				pmulhw	mm0, mm7
				movq	mm2, MMWORD PTR [edi]
				pmulhw	mm1, mm7
				movq	mm3, MMWORD PTR [edi+ 8]
				paddsw	mm0, mm0
				paddsw	mm1, mm1
				paddsw	mm0, mm0
				paddsw	mm1, mm1
				add	esi, 16
				paddsw	mm0, mm2
				paddsw	mm1, mm3
				movq	MMWORD PTR [edi], mm0
				movq	MMWORD PTR [edi + 8], mm1
				add	edi, 16
				sub	ecx, 4
			.ENDW
			add	ecx, 4
			emms
		.ENDIF
		;
		test	ecx, ecx
		.WHILE	!ZERO?
			movsx	eax, SWORD PTR [esi]
			movsx	ebx, SWORD PTR [esi + 2]
			sar	eax, 1
			sar	ebx, 1
			imul	eax, nVolume[0]
			imul	ebx, nVolume[4]
			add	esi, 4
			movsx	edx, SWORD PTR [edi]
			sar	eax, 16 - 1
			sar	ebx, 16 - 1
			add	eax, edx
			movsx	edx, SWORD PTR [edi + 2]
			.IF	(SDWORD PTR eax) < -8000H
				mov	eax, -8000H
			.ELSEIF	(SDWORD PTR eax) > 7FFFH
				mov	eax, 7FFFH
			.ENDIF
			add	ebx, edx
			.IF	(SDWORD PTR ebx) < -8000H
				mov	ebx, -8000H
			.ELSEIF	(SDWORD PTR ebx) > 7FFFH
				mov	ebx, 7FFFH
			.ENDIF
			mov	SWORD PTR [edi], ax
			mov	SWORD PTR [edi + 2], bx
			add	edi, 4
			dec	ecx
		.ENDW
		;
		.ELSE
		.IF	ERI_EnabledProcessorType & ERI_USE_MMX_PENTIUM
			sub	ecx, 4
			.WHILE	!SIGN?
				movq	mm2, MMWORD PTR [edi]
				movq	mm3, MMWORD PTR [edi+ 8]
				movq	mm0, MMWORD PTR [esi]
				movq	mm1, MMWORD PTR [esi + 8]
				add	esi, 16
				paddsw	mm0, mm2
				paddsw	mm1, mm3
				movq	MMWORD PTR [edi], mm0
				movq	MMWORD PTR [edi + 8], mm1
				add	edi, 16
				sub	ecx, 4
			.ENDW
			add	ecx, 4
			emms
		.ENDIF
		;
		test	ecx, ecx
		push	ebp
		.WHILE	!ZERO?
			movsx	eax, SWORD PTR [edi]
			movsx	ebx, SWORD PTR [edi + 2]
			movsx	edx, SWORD PTR [esi]
			movsx	ebp, SWORD PTR [esi + 2]
			add	esi, 4
			add	eax, edx
			add	ebx, ebp
			add	eax, 8000H
			add	ebx, 8000H
			.IF	(DWORD PTR eax) >= 10000H
				sar	eax, 31
				not	eax
				and	eax, 0FFFFH
			.ENDIF
			.IF	(DWORD PTR ebx) >= 10000H
				sar	ebx, 31
				not	ebx
				and	ebx, 0FFFFH
			.ENDIF
			sub	eax, 8000H
			sub	ebx, 8000H
			mov	SWORD PTR [edi], ax
			mov	SWORD PTR [edi + 2], bx
			add	edi, 4
			dec	ecx
		.ENDW
		pop	ebp
		;
		.ENDIF
	.ENDIF
	.ENDIF

	xor	eax, eax
	ret

glsSound_MixPCMSamples	ENDP

;
;	音声バッファを初期化
; ----------------------------------------------------------------------------
ALIGN	10H
glsSound_CleanSoundBuffer	PROC	NEAR32 C PUBLIC USES ebx esi edi,
		pSoundFormat:PTR WAVEFORMATEX,
		pSoundBuffer:PTR, nSampleCount:DWORD

	mov	ebx, pSoundFormat
	ASSUME	ebx:PTR WAVEFORMATEX
	mov	ecx, nSampleCount
	movzx	edx, [ebx].nChannels
	imul	ecx, edx
	movzx	edx, [ebx].wBitsPerSample
	.IF	edx == 8
		mov	eax, 80808080H
	.ELSEIF	edx == 16
		shl	ecx, 1
		xor	eax, eax
	.ELSE
		mov	eax, eslErrInvalidParam
		ret
	.ENDIF
	ASSUME	ebx:NOTHING

	mov	edi, pSoundBuffer
	sub	ecx, 8
	.WHILE	!SIGN?
		mov	DWORD PTR [edi], eax
		mov	DWORD PTR [edi + 4], eax
		add	edi, 8
		sub	ecx, 8
	.ENDW
	add	ecx, 8
	.WHILE	!ZERO?
		mov	BYTE PTR [edi], al
		inc	edi
		dec	ecx
	.ENDW

	xor	eax, eax
	ret

glsSound_CleanSoundBuffer	ENDP

;
;	音量を変更
; ----------------------------------------------------------------------------
ALIGN	10H
glsSound_RemixPCMSamples	PROC	NEAR32 C PUBLIC USES ebx esi edi,
		pDstFormat:PTR WAVEFORMATEX,
		pDstPCM:PTR,
		pSrcPCM:PTR,
		pNewVolume:PTR REAL4,
		pOldVolume:PTR REAL4,
		nSampleCount:DWORD

	LOCAL	nChannels:DWORD
	LOCAL	nDeltaVolume[4]:REAL4

	;
	; 音量の差を計算
	;
	mov	ebx, pDstFormat
	mov	esi, pNewVolume
	mov	edi, pOldVolume
	ASSUME	ebx:PTR WAVEFORMATEX
	;
	movzx	ecx, [ebx].nChannels
	mov	nChannels, ecx
	;
	.IF	ecx == 1
		mov	eax, DWORD PTR [esi]
		mov	edx, DWORD PTR [edi]
		add	eax, 00080000H
		add	edx, 00080000H
		and	eax, 0FFF00000H
		and	edx, 0FFF00000H
		.IF	eax == edx
			xor	eax, eax
			ret
		.ENDIF
		mov	nDeltaVolume[0], eax
		mov	nDeltaVolume[4], edx
		fld	nDeltaVolume[0]
		fsub	nDeltaVolume[4]
		fstp	nDeltaVolume[0]
		mov	eax, nDeltaVolume[0]
		.IF	eax & 000FFFFFH
			xor	eax, eax
		.ENDIF

	.ELSEIF	ecx == 2
		mov	eax, DWORD PTR [esi]
		mov	edx, DWORD PTR [edi]
		add	eax, 00080000H
		add	edx, 00080000H
		and	eax, 0FFF00000H
		and	edx, 0FFF00000H
		mov	nDeltaVolume[0], eax
		mov	nDeltaVolume[8], edx
		;
		mov	eax, DWORD PTR [esi + 4]
		mov	edx, DWORD PTR [edi + 4]
		add	eax, 00080000H
		add	edx, 00080000H
		and	eax, 0FFF00000H
		and	edx, 0FFF00000H
		mov	nDeltaVolume[4], eax
		mov	nDeltaVolume[12], edx
		;
		mov	eax, nDeltaVolume[0]
		mov	edx, nDeltaVolume[4]
		.IF	(eax == nDeltaVolume[8]) && (edx == nDeltaVolume[12])
			xor	eax, eax
			ret
		.ENDIF
		;
		fld	nDeltaVolume[0]
		fsub	nDeltaVolume[8]
		fstp	nDeltaVolume[0]
		fld	nDeltaVolume[4]
		fsub	nDeltaVolume[12]
		fstp	nDeltaVolume[4]
		mov	eax, nDeltaVolume[0]
		mov	edx, nDeltaVolume[4]
		.IF	(eax & 000FFFFFH) || (edx & 000FFFFFH)
			xor	eax, eax
		.ENDIF

	.ELSE
		mov	eax, eslErrInvalidParam
		ret
	.ENDIF
	;
	ASSUME	ebx:NOTHING

	.IF	eax == 0
		;
		; 一度元の音量で差し引いてから、新たにミックス
		;
		.IF	nChannels == 1
			mov	eax, DWORD PTR [edi]
			xor	eax, 80000000H
			mov	nDeltaVolume[0], eax
		.ELSE
			mov	eax, DWORD PTR [edi]
			mov	edx, DWORD PTR [edi + 4]
			xor	eax, 80000000H
			xor	edx, 80000000H
			mov	nDeltaVolume[0], eax
			mov	nDeltaVolume[4], edx
		.ENDIF
		;
		INVOKE	glsSound_MixPCMSamples ,
				pDstFormat, pDstPCM, pSrcPCM,
				ADDR nDeltaVolume[0], nSampleCount
		;
		INVOKE	glsSound_MixPCMSamples ,
				pDstFormat, pDstPCM, pSrcPCM,
				pNewVolume, nSampleCount

	.ELSE
		;
		; 音量の差を加算
		;
		INVOKE	glsSound_MixPCMSamples ,
				pDstFormat, pDstPCM, pSrcPCM,
				ADDR nDeltaVolume[0], nSampleCount
	.ENDIF

	ret

glsSound_RemixPCMSamples	ENDP


CodeSeg	ENDS

	END
