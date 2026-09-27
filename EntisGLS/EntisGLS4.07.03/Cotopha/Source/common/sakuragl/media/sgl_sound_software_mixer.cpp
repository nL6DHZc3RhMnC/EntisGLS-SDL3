
#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl_media.h>
#include <sakuragl/sgl_erisa_lib.h>
#include <sakuragl/media/sgl_sound_software_mixer.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////
// PCM フォーマット変換
//////////////////////////////////////////////////////////////////////////

SGLError SakuraGL::sglDecodeSoundTo16bitsPCM
	( int16_t * pwDst, size_t nDstStride,  size_t nDstChStride,
		const SGLSoundFormat& fmtSrc,
		const uint8_t * pSrcPCM, size_t nSamples, size_t nChannels )
{
	size_t	nSrcChannels = fmtSrc.channels ;
	for ( size_t i = 0; i < nChannels; i ++ )
	{
		int16_t *	pwDstNext = pwDst + nDstChStride * i ;
		if ( fmtSrc.bitsPerSample == 16 )
		{
			const int16_t * pwSrcNext = ((const int16_t*) pSrcPCM) + i ;
			for ( size_t j = 0; j < nSamples; j ++ )
			{
				*pwDstNext = *pwSrcNext ;
				pwDstNext += nDstStride ;
				pwSrcNext += nSrcChannels ;
			}
		}
		else if ( fmtSrc.bitsPerSample == 24 )
		{
			const uint8_t * pSrcNext = pSrcPCM + i * 3 ;
			for ( size_t j = 0; j < nSamples; j ++ )
			{
				*pwDstNext = (int16_t) (((int16_t) pSrcNext[2] << 8) | pSrcNext[1]) ;
				pwDstNext += nDstStride ;
				pSrcNext += nSrcChannels * 3 ;
			}
		}
		else if ( fmtSrc.bitsPerSample == 32 )
		{
			if ( fmtSrc.format == formatSoundIEEEFloat )
			{
				const float32_t *	pfpSrcNext = ((const float32_t*) pSrcPCM) + i ;
				for ( size_t j = 0; j < nSamples; j ++ )
				{
					int32_t	v = eslRoundR32ToInt( *pfpSrcNext * 0x8000 ) ;
					*pwDstNext = (int16_t) esl_clampi( v, -0x8000, 0x7FFF ) ;
					pwDstNext += nDstStride ;
					pfpSrcNext += nSrcChannels ;
				}
			}
			else
			{
				const int32_t *	pnSrcNext = ((const int32_t*) pSrcPCM) + i ;
				for ( size_t j = 0; j < nSamples; j ++ )
				{
					*pwDstNext = (int16_t) (*pnSrcNext >> 16) ;
					pwDstNext += nDstStride ;
					pnSrcNext += nSrcChannels ;
				}
			}
		}
		else if ( fmtSrc.bitsPerSample == 8 )
		{
			const uint8_t *	pSrcNext = pSrcPCM + i ;
			for ( size_t j = 0; j < nSamples; j ++ )
			{
				*pwDstNext = ((int16_t) *pSrcNext - 0x80) << 8 ;
				pwDstNext += nDstStride ;
				pSrcNext += nSrcChannels ;
			}
		}
		else
		{
			return	sglErrFailed ;
		}
	}
	return	sglErrSuccess ;
}

SGLError SakuraGL::sglDecodeSoundTo32bitsPCM
	( float32_t * pfpDst, size_t nDstStride,  size_t nDstChStride,
		const SGLSoundFormat& fmtSrc,
		const uint8_t * pSrcPCM, size_t nSamples, size_t nChannels )
{
	size_t	nSrcChannels = fmtSrc.channels ;
	for ( size_t i = 0; i < nChannels; i ++ )
	{
		float32_t *	pfpDstNext = pfpDst + nDstChStride * i ;
		if ( fmtSrc.bitsPerSample == 16 )
		{
			const int16_t * pwSrcNext = ((const int16_t*) pSrcPCM) + i ;
			for ( size_t j = 0; j < nSamples; j ++ )
			{
				*pfpDstNext = ((float32_t) *pwSrcNext) / 0x8000 ;
				pfpDstNext += nDstStride ;
				pwSrcNext += nSrcChannels ;
			}
		}
		else if ( fmtSrc.bitsPerSample == 24 )
		{
			const uint8_t * pSrcNext = pSrcPCM + i * 3 ;
			for ( size_t j = 0; j < nSamples; j ++ )
			{
				int32_t	v = ((int32_t) pSrcNext[2] << 24)
							| ((int32_t) pSrcNext[1] << 16)
							| ((int32_t) pSrcNext[0] << 8) ;
				*pfpDstNext = (float32_t) (v >> 8) / 0x800000 ;
				pfpDstNext += nDstStride ;
				pSrcNext += nSrcChannels * 3 ;
			}
		}
		else if ( fmtSrc.bitsPerSample == 32 )
		{
			if ( fmtSrc.format == formatSoundIEEEFloat )
			{
				const float32_t *	pfpSrcNext = ((const float32_t*) pSrcPCM) + i ;
				for ( size_t j = 0; j < nSamples; j ++ )
				{
					*pfpDstNext = *pfpSrcNext ;
					pfpDstNext += nDstStride ;
					pfpSrcNext += nSrcChannels ;
				}
			}
			else
			{
				const int32_t *	pnSrcNext = ((const int32_t*) pSrcPCM) + i ;
				for ( size_t j = 0; j < nSamples; j ++ )
				{
					*pfpDstNext = (float32_t) *pnSrcNext / (float32_t) 0x80000000 ;
					pfpDstNext += nDstStride ;
					pnSrcNext += nSrcChannels ;
				}
			}
		}
		else if ( fmtSrc.bitsPerSample == 8 )
		{
			const uint8_t *	pSrcNext = pSrcPCM + i ;
			for ( size_t j = 0; j < nSamples; j ++ )
			{
				*pfpDstNext = ((float32_t) *pSrcNext - 0x80) / 0x80 ;
				pfpDstNext += nDstStride ;
				pSrcNext += nSrcChannels ;
			}
		}
		else
		{
			return	sglErrFailed ;
		}
	}
	return	sglErrSuccess ;
}

SGLError SakuraGL::sglEncodeSoundFrom16bitsPCM
	( const SGLSoundFormat& fmtDst, uint8_t * pDstPCM,
		const int16_t * pwSrc,
		size_t nSrcStride, size_t nSrcChStride,
		size_t nSamples, size_t nChannels )
{
	size_t	nDstChannels = fmtDst.channels ;
	for ( size_t i = 0; i < nChannels; i ++ )
	{
		const int16_t *	pwSrcNext = pwSrc + nSrcChStride * i ;
		if ( fmtDst.bitsPerSample == 16 )
		{
			int16_t * pwDstNext = ((int16_t*) pDstPCM) + i ;
			for ( size_t j = 0; j < nSamples; j ++ )
			{
				*pwDstNext = *pwSrcNext ;
				pwSrcNext += nSrcStride ;
				pwDstNext += nDstChannels ;
			}
		}
		else if ( fmtDst.bitsPerSample == 24 )
		{
			uint8_t * pDstNext = pDstPCM + i * 3 ;
			for ( size_t j = 0; j < nSamples; j ++ )
			{
				int32_t	v = *pwSrcNext ;
				pDstNext[2] = (uint8_t) (v >> 8) ;
				pDstNext[1] = (uint8_t) (v & 0xFF) ;
				pDstNext[0] = 0 ;
				pwSrcNext += nSrcStride ;
				pDstNext += nDstChannels * 3 ;
			}
		}
		else if ( fmtDst.bitsPerSample == 32 )
		{
			float32_t *	pfpDstNext = ((float32_t*) pDstPCM) + i ;
			for ( size_t j = 0; j < nSamples; j ++ )
			{
				*pfpDstNext = ((float32_t) *pwSrcNext) / 0x8000 ;
				pwSrcNext += nSrcStride ;
				pfpDstNext += nDstChannels ;
			}
		}
		else if ( fmtDst.bitsPerSample == 8 )
		{
			uint8_t *	pDstNext = pDstPCM + i ;
			for ( size_t j = 0; j < nSamples; j ++ )
			{
				*pDstNext = (uint8_t) ((*pwSrcNext >> 8) + 0x80) ;
				pwSrcNext += nSrcStride ;
				pDstNext += nDstChannels ;
			}
		}
		else
		{
			return	sglErrFailed ;
		}
	}
	return	sglErrSuccess ;
}

SGLError SakuraGL::sglEncodeSoundFrom32bitsPCM
	( const SGLSoundFormat& fmtDst, uint8_t * pDstPCM,
		const float32_t * pfpSrc,
		size_t nSrcStride, size_t nSrcChStride,
		size_t nSamples, size_t nChannels )
{
	size_t	nDstChannels = fmtDst.channels ;
	for ( size_t i = 0; i < nChannels; i ++ )
	{
		const float32_t *	pfpSrcNext = pfpSrc + nSrcChStride * i ;
		if ( fmtDst.bitsPerSample == 16 )
		{
			int16_t * pwDstNext = ((int16_t*) pDstPCM) + i ;
			for ( size_t j = 0; j < nSamples; j ++ )
			{
				int32_t	v = eslRoundR32ToInt( *pfpSrcNext * 0x8000 ) ;
				*pwDstNext = (int16_t) esl_clampi( v, -0x8000, 0x7FFF ) ;
				pfpSrcNext += nSrcStride ;
				pwDstNext += nDstChannels ;
			}
		}
		else if ( fmtDst.bitsPerSample == 24 )
		{
			uint8_t * pDstNext = pDstPCM + i * 3 ;
			for ( size_t j = 0; j < nSamples; j ++ )
			{
				int32_t	v = eslRoundR32ToInt( *pfpSrcNext * 0x800000 ) ;
				v = esl_clampi( v, -0x800000, 0x7FFFFF ) ;
				pDstNext[2] = (uint8_t) (v >> 16) ;
				pDstNext[1] = (uint8_t) (v >> 8) ;
				pDstNext[0] = (uint8_t) (v & 0xFF) ;
				pfpSrcNext += nSrcStride ;
				pDstNext += nDstChannels * 3 ;
			}
		}
		else if ( fmtDst.bitsPerSample == 32 )
		{
			if ( fmtDst.format == formatSoundIEEEFloat )
			{
				float32_t *	pfpDstNext = ((float32_t*) pDstPCM) + i ;
				for ( size_t j = 0; j < nSamples; j ++ )
				{
					*pfpDstNext = *pfpSrcNext ;
					pfpSrcNext += nSrcStride ;
					pfpDstNext += nDstChannels ;
				}
			}
			else
			{
				int32_t *	pnDstNext = ((int32_t*) pDstPCM) + i ;
				for ( size_t j = 0; j < nSamples; j ++ )
				{
					*pnDstNext = eslRoundR32ToInt( *pfpSrcNext * (float32_t) 0x80000000 ) ;
					pfpSrcNext += nSrcStride ;
					pnDstNext += nDstChannels ;
				}
			}
		}
		else if ( fmtDst.bitsPerSample == 8 )
		{
			uint8_t *	pDstNext = pDstPCM + i ;
			for ( size_t j = 0; j < nSamples; j ++ )
			{
				int32_t	v = eslRoundR32ToInt( *pfpSrcNext * 0x80 + 0x80 ) ;
				v = esl_clampi( v, 0, 0xFF ) ;
				*pDstNext = (uint8_t) v ;
				pfpSrcNext += nSrcStride ;
				pDstNext += nDstChannels ;
			}
		}
		else
		{
			return	sglErrFailed ;
		}
	}
	return	sglErrSuccess ;
}

SGLError SakuraGL::sglEncodeSoundFrom32bitsPCM
	( const SGLSoundFormat& fmtDst, uint8_t * pDstPCM,
		const int32_t * pnSrc,
		size_t nSrcStride, size_t nSrcChStride,
		size_t nSamples, size_t nChannels )
{
	size_t	nDstChannels = fmtDst.channels ;
	for ( size_t i = 0; i < nChannels; i ++ )
	{
		const int32_t *	pnSrcNext = pnSrc + nSrcChStride * i ;
		if ( fmtDst.bitsPerSample == 16 )
		{
			int16_t * pwDstNext = ((int16_t*) pDstPCM) + i ;
			for ( size_t j = 0; j < nSamples; j ++ )
			{
				*pwDstNext = (int16_t) (*pnSrcNext >> 16) ;
				pnSrcNext += nSrcStride ;
				pwDstNext += nDstChannels ;
			}
		}
		else if ( fmtDst.bitsPerSample == 24 )
		{
			uint8_t * pDstNext = pDstPCM + i * 3 ;
			for ( size_t j = 0; j < nSamples; j ++ )
			{
				int32_t	v = *pnSrcNext >> 8 ;
				pDstNext[2] = (uint8_t) (v >> 16) ;
				pDstNext[1] = (uint8_t) (v >> 8) ;
				pDstNext[0] = (uint8_t) (v & 0xFF) ;
				pnSrcNext += nSrcStride ;
				pDstNext += nDstChannels * 3 ;
			}
		}
		else if ( fmtDst.bitsPerSample == 32 )
		{
			if ( fmtDst.format == formatSoundIEEEFloat )
			{
				float32_t *	pfpDstNext = ((float32_t*) pDstPCM) + i ;
				for ( size_t j = 0; j < nSamples; j ++ )
				{
					*pfpDstNext = (float32_t) *pnSrcNext / (float32_t) 0x80000000 ;
					pnSrcNext += nSrcStride ;
					pfpDstNext += nDstChannels ;
				}
			}
			else
			{
				int32_t *	pnDstNext = ((int32_t*) pDstPCM) + i ;
				for ( size_t j = 0; j < nSamples; j ++ )
				{
					*pnDstNext = *pnSrcNext ;
					pnSrcNext += nSrcStride ;
					pnDstNext += nDstChannels ;
				}
			}
		}
		else if ( fmtDst.bitsPerSample == 8 )
		{
			uint8_t *	pDstNext = pDstPCM + i ;
			for ( size_t j = 0; j < nSamples; j ++ )
			{
				int32_t	v = (*pnSrcNext >> 24) + 0x80 ;
				*pDstNext = (uint8_t) v ;
				pnSrcNext += nSrcStride ;
				pDstNext += nDstChannels ;
			}
		}
		else
		{
			return	sglErrFailed ;
		}
	}
	return	sglErrSuccess ;
}


//////////////////////////////////////////////////////////////////////////////
// ソフトウェアミキサ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSoundSoftwareMixer, SObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSoundSoftwareMixer::SGLSoundSoftwareMixer( void )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSoundSoftwareMixer::~SGLSoundSoftwareMixer( void )
{
}

// 入力ライン追加
//////////////////////////////////////////////////////////////////////////////
void SGLSoundSoftwareMixer::AddInputLine( SGLSoundMixerInputInterface * pLine )
{
	m_csSync.Lock() ;
	if ( m_aLines.FindPtr( pLine ) < 0 )
	{
		m_aLines.Add( pLine ) ;
	}
	m_csSync.Unlock() ;
}

// 入力ライン削除
//////////////////////////////////////////////////////////////////////////////
void SGLSoundSoftwareMixer::DetachInputLine( SGLSoundMixerInputInterface * pLine )
{
	m_csSync.Lock() ;
	size_t	iLast = 0 ;
	for ( ; ; )
	{
		ssize_t	i = m_aLines.FindPtr( pLine, iLast ) ;
		if ( i < 0 )
		{
			break ;
		}
		m_aLines.RemoveAt( (size_t) i ) ;
		iLast = (size_t) i ;
	}
	m_csSync.Unlock() ;
}

// 出力フォーマット設定
//////////////////////////////////////////////////////////////////////////////
void SGLSoundSoftwareMixer::SetOutputFormat( const SGLSoundFormat& fmt )
{
	m_fmtOut = fmt ;
}

// 出力フォーマット取得
//////////////////////////////////////////////////////////////////////////////
const SGLSoundFormat& SGLSoundSoftwareMixer::GetOutputFormat( void ) const
{
	return	m_fmtOut ;
}

// ミキシング実行
//////////////////////////////////////////////////////////////////////////////
void SGLSoundSoftwareMixer::MixSound( uint32_t msecTime )
{
	size_t	nOutSamples = (size_t) m_fmtOut.MilliSecToSamples( msecTime ) ;
	size_t	nOutBytes = (size_t) m_fmtOut.SamplesToBytes( nOutSamples ) ;
	if ( nOutSamples == 0 )
	{
		return ;
	}
	SArray<uint8_t>	bufInTemp ;
	MixBuffer		mixBuf ;
	m_csSync.Lock() ;
	void *	ptrOutBuf = m_qbufOut.PutBuffer( nOutBytes ) ;
	if ( m_fmtOut.bitsPerSample == 8 )
	{
		eslFillMemory( ptrOutBuf, 0x80, nOutBytes ) ;
	}
	else
	{
		eslFillMemory( ptrOutBuf, 0, nOutBytes ) ;
	}
	for ( size_t i = 0; i < m_aLines.GetLength(); i ++ )
	{
		SGLSoundMixerInputInterface *	pLine = m_aLines.GetAt( i ) ;
		if ( pLine == NULL )
		{
			continue ;
		}
		const SGLSoundFormat&	fmtIn = pLine->GetFormat() ;
		size_t	nInSamples = nOutSamples * fmtIn.frequency / m_fmtOut.frequency ;
		if ( nInSamples == 0 )
		{
			nInSamples = 1 ;
		}
		size_t		nInBytes = (size_t) fmtIn.SamplesToBytes( nInSamples ) ;
		uint8_t *	pbytInBuf = bufInTemp.GetArray( nInBytes ) ;
		size_t		nReadBytes = pLine->ReadStream( pbytInBuf, nInBytes ) ;
		size_t		nReadSamples = (size_t) fmtIn.BytesToSamples( nReadBytes ) ;
		size_t		nTempOutSamples = nOutSamples ;
		if ( nReadBytes < nInBytes )
		{
			nTempOutSamples =
				nReadSamples * m_fmtOut.frequency / fmtIn.frequency ;
			if ( nTempOutSamples > nOutSamples )
			{
				nTempOutSamples = nOutSamples ;
			}
		}
		mixBuf.MixWave
			( m_fmtOut, ptrOutBuf, nTempOutSamples,
						fmtIn, pbytInBuf, nReadSamples ) ;
		bufInTemp.FinishArray() ;
	}
	m_qbufOut.FlushBuffer( nOutBytes ) ;
	m_csSync.Unlock() ;
}

// 出力バッファに蓄積されたデータ量取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLSoundSoftwareMixer::GetOutputDataBytes( void ) const
{
	size_t	nDataBytes ;
	m_csSync.Lock() ;
	nDataBytes = (size_t) m_qbufOut.GetLength() ;
	m_csSync.Unlock() ;
	return	nDataBytes ;
}

// 出力バッファからデータ読み出し
//////////////////////////////////////////////////////////////////////////////
size_t SGLSoundSoftwareMixer::ReadOutputData( void * ptrSound, size_t nBytes )
{
	size_t	nReadBytes ;
	m_csSync.Lock() ;
	nReadBytes = m_qbufOut.Read( ptrSound, nBytes ) ;
	m_csSync.Unlock() ;
	return	nReadBytes ;
}

void SGLSoundSoftwareMixer::MixBuffer::MixWave
	( const SGLSoundFormat& fmtOut,
		void * ptrOutBuf, size_t nOutSamples,
		const SGLSoundFormat& fmtIn,
		const void * ptrInBuf, size_t nInSamples )
{
	//
	// 入力フォーマットを16ビットに正規化する
	//
	if ( fmtIn.bitsPerSample != 16 )
	{
		int16_t *	pDstBuf =
			m_bufSrc1.GetArray( nInSamples * fmtIn.channels ) ;
		NormalizeTo16bits( pDstBuf, fmtIn, ptrInBuf, nInSamples ) ;
		m_bufSrc1.FinishArray() ;
		ptrInBuf = pDstBuf ;
	}
	//
	// チャネル数を正規化する
	//
	if ( fmtOut.channels != fmtIn.channels )
	{
		int16_t *	pDstBuf =
			m_bufSrc2.GetArray( nInSamples * fmtOut.channels ) ;
		NormalizeChannels
			( pDstBuf, fmtOut.channels,
				(const int16_t*) ptrInBuf, nInSamples, fmtIn.channels ) ;
		m_bufSrc2.FinishArray() ;
		ptrInBuf = pDstBuf ;
	}
	//
	// 周波数を変換する
	//
	if ( nOutSamples != nInSamples )
	{
		int16_t *	pDstBuf =
			m_bufSrc3.GetArray( nOutSamples * fmtOut.channels ) ;
		NormalizeFrequency
			( pDstBuf, nOutSamples,
				(const int16_t*) ptrInBuf, nInSamples, fmtOut.channels ) ;
		m_bufSrc3.FinishArray() ;
		ptrInBuf = pDstBuf ;
	}
	//
	// 出力フォーマットを16ビットに正規化する
	//
	int16_t *	pDstBuf = (int16_t*) ptrOutBuf ;
	if ( fmtOut.bitsPerSample != 16 )
	{
		pDstBuf = m_bufDst.GetArray( nOutSamples * fmtOut.channels ) ;
		NormalizeTo16bits( pDstBuf, fmtOut, ptrOutBuf, nOutSamples ) ;
		m_bufDst.FinishArray() ;
	}
	//
	// 合成
	//
	const int16_t *	pSrcBuf = (const int16_t*) ptrInBuf ;
	size_t	nSamples = nOutSamples * fmtOut.channels ;
	for ( size_t i = 0; i < nSamples; i ++ )
	{
		int32_t	v = (int32_t) pDstBuf[i] + pSrcBuf[i] + 0x8000 ;
		if ( (uint32_t) v > 0x10000 )
		{
			v = ~(v >> 31) & 0xFFFF ;
		}
		pDstBuf[i] = (int16_t) (v - 0x8000) ;
	}
	//
	// 出力フォーマットを正規化する
	//
	if ( fmtOut.bitsPerSample != 16 )
	{
		NormalizeFrom16bits( ptrOutBuf, fmtOut, pDstBuf, nOutSamples ) ;
	}
}

void SGLSoundSoftwareMixer::MixBuffer::NormalizeTo16bits
	( int16_t * pDstBuf,
		const SGLSoundFormat& fmtIn,
		const void * ptrInBuf, size_t nInSamples )
{
	size_t	nSamples = nInSamples * fmtIn.channels ;
	if ( fmtIn.bitsPerSample == 8 )
	{
		const uint8_t *	pbytInBuf = (const uint8_t*) ptrInBuf ;
		for ( size_t i = 0; i < nSamples; i ++ )
		{
			pDstBuf[i] = ((int16_t) pbytInBuf[i] - 0x80) << 8 ;
		}
	}
	else if ( fmtIn.bitsPerSample == 16 )
	{
		eslCopyMemory( pDstBuf, ptrInBuf, nSamples * sizeof(int16_t) ) ;
	}
	else if ( fmtIn.bitsPerSample == 24 )
	{
		const uint8_t *	pbytInBuf = (const uint8_t*) ptrInBuf ;
		for ( size_t i = 0; i < nSamples; i ++ )
		{
			pDstBuf[i] = pbytInBuf[1] | ((int16_t) pbytInBuf[2] << 8) ;
			pbytInBuf += 3 ;
		}
	}
	else if ( fmtIn.bitsPerSample == 32 )
	{
		if ( fmtIn.format == formatSoundIEEEFloat )
		{
			const float32_t *	pfpInBuf = (const float32_t*) ptrInBuf ;
			for ( size_t i = 0; i < nSamples; i ++ )
			{
				pDstBuf[i] =
					(int16_t) esl_clampi
						( eslRoundR32ToInt( pfpInBuf[i] * 0x8000 ),
														-0x8000, 0x7FFF ) ;
			}
		}
		else
		{
			const int32_t *	pnInBuf = (const int32_t*) ptrInBuf ;
			for ( size_t i = 0; i < nSamples; i ++ )
			{
				pDstBuf[i] = (int16_t) (pnInBuf[i] >> 16) ;
			}
		}
	}
	else
	{
		for ( size_t i = 0; i < nSamples; i ++ )
		{
			pDstBuf[i] = 0 ;
		}
	}
}

void SGLSoundSoftwareMixer::MixBuffer::NormalizeFrom16bits
	( void * pDstBuf,
		const SGLSoundFormat& fmtOut,
		const int16_t * ptrInBuf, size_t nInSamples )
{
	size_t	nSamples = nInSamples * fmtOut.channels ;
	if ( fmtOut.bitsPerSample == 8 )
	{
		uint8_t *	pbytOutBuf = (uint8_t*) pDstBuf ;
		for ( size_t i = 0; i < nSamples; i ++ )
		{
			pbytOutBuf[i] = (uint8_t) ((ptrInBuf[i] >> 8) + 0x80) ;
		}
	}
	else if ( fmtOut.bitsPerSample == 16 )
	{
		eslCopyMemory( pDstBuf, ptrInBuf, nSamples * sizeof(int16_t) ) ;
	}
	else if ( fmtOut.bitsPerSample == 24 )
	{
		uint8_t *	pbytOutBuf = (uint8_t*) pDstBuf ;
		for ( size_t i = 0; i < nSamples; i ++ )
		{
			pbytOutBuf[0] = 0 ;
			pbytOutBuf[1] = (uint8_t) ptrInBuf[i] ;
			pbytOutBuf[2] = (uint8_t) (ptrInBuf[i] >> 8) ;
			pbytOutBuf += 3 ;
		}
	}
	else if ( fmtOut.bitsPerSample == 32 )
	{
		if ( fmtOut.format == formatSoundIEEEFloat )
		{
			float32_t *	pfpOutBuf = (float32_t*) pDstBuf ;
			float32_t	d = 1.0f / 0x8000 ;
			for ( size_t i = 0; i < nSamples; i ++ )
			{
				pfpOutBuf[i] = (float32_t) ptrInBuf[i] * d ;
			}
		}
		else
		{
			int32_t *	pnOutBuf = (int32_t*) pDstBuf ;
			for ( size_t i = 0; i < nSamples; i ++ )
			{
				pnOutBuf[i] = (int32_t) ptrInBuf[i] << 16 ;
			}
		}
	}
}

void SGLSoundSoftwareMixer::MixBuffer::NormalizeChannels
	( int16_t * pDstBuf, size_t nDstChannel,
		const int16_t * pInBuf, size_t nInSamples, size_t nSrcChannel )
{
	if ( nDstChannel < nSrcChannel )
	{
		int32_t	nRcp = 0x10000 / (int32_t) (nSrcChannel - nDstChannel + 1) ;
		size_t	nCopyChannels = nDstChannel - 1 ;
		for ( size_t i = 0; i < nInSamples; i ++ )
		{
			for ( size_t j = 0; j < nCopyChannels; j ++ )
			{
				pDstBuf[j] = pInBuf[j] ;
			}
			int32_t	n = 0 ;
			for ( size_t j = nCopyChannels; j < nSrcChannel; j ++ )
			{
				n += pInBuf[j] ;
			}
			pDstBuf[nCopyChannels] = (int16_t) ((n * nRcp) >> 16) ;
			pDstBuf += nDstChannel ;
			pInBuf += nSrcChannel ;
		}
	}
	else
	{
		int32_t	nRcp = 0x10000 / (int32_t) nSrcChannel ;
		for ( size_t i = 0; i < nInSamples; i ++ )
		{
			int32_t	n = 0 ;
			for ( size_t j = 0; j < nSrcChannel; j ++ )
			{
				n += (pDstBuf[j] = pInBuf[j]) ;
			}
			int16_t	v = (int16_t) ((n * nRcp) >> 16) ;
			for ( size_t j = nSrcChannel; j < nDstChannel; j ++ )
			{
				pDstBuf[j] = v ;
			}
			pDstBuf += nDstChannel ;
			pInBuf += nSrcChannel ;
		}
	}
}

void SGLSoundSoftwareMixer::MixBuffer::NormalizeFrequency
	( int16_t * pDstBuf, size_t nOutSamples,
		const int16_t * pInBuf, size_t nInSamples, size_t nSrcChannel )
{
	if ( nOutSamples == 0 )
	{
		return ;
	}
	else if ( nOutSamples == 1 )
	{
		for ( size_t i = 0; i < nSrcChannel; i ++ )
		{
			pDstBuf[i] = pInBuf[i] ;
		}
		return ;
	}
	int64_t	fxRate = (int64_t) (nInSamples - 1) * 0x10000 / (nOutSamples - 1) ;
	int64_t	fxSrc = 0 ;
	for ( size_t i = 0; i < nOutSamples; i ++, fxSrc += fxRate )
	{
		size_t	iSrc = (size_t) (fxSrc >> 16) ;
		int32_t	nBlend = (int32_t) (fxSrc & 0xFFFF) ;
		//
		const int16_t *	pSrcSample = pInBuf + (iSrc * nSrcChannel) ;
		if ( nBlend > 0 )
		{
			ESLAssert( iSrc + 1 < nInSamples ) ;
			for ( size_t j = 0; j < nSrcChannel; j ++ )
			{
				int32_t	v0 = *pSrcSample ;
				int32_t	v1 = pSrcSample[nSrcChannel] ;
				int32_t	d = ((v1 - v0) * nBlend) >> 16 ;
				pDstBuf[j] = (int16_t) (v0 + d) ;
				pSrcSample ++ ;
			}
		}
		else
		{
			for ( size_t j = 0; j < nSrcChannel; j ++ )
			{
				pDstBuf[j] = pSrcSample[j] ;
			}
		}
		pDstBuf += nSrcChannel ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// サウンドフィルター・インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSoundFilterInterface, ESLObject )



//////////////////////////////////////////////////////////////////////////////
// ミキサ入力ライン
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLSoundMixerLinePlayer, SGLSoundPlayerInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSoundMixerLinePlayer::SGLSoundMixerLinePlayer( SGLSoundSoftwareMixer * pMixer )
{
	m_refMixer = pMixer ;
	m_nPosition = 0 ;
	m_flagOpened = false ;
	m_flagPlaying = false ;
	m_flagPaused = false ;
	m_flagStatic = false ;
	m_flagLoop = false ;
	m_iStaticPos = 0 ;
	m_fpVolume[0] = 1.0f ;
	m_fpVolume[1] = 1.0f ;
	m_pListener = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSoundMixerLinePlayer::~SGLSoundMixerLinePlayer( void )
{
	Close() ;
}

// フォーマットを指定して出力を準備する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSoundMixerLinePlayer::Open( const SGLSoundFormat& fmt )
{
	m_format = fmt ;
	m_nPosition = 0 ;
	m_flagOpened = true ;
	//
	for ( size_t i = 0; i < m_aFilters.GetLength(); i ++ )
	{
		SGLSoundFilterInterface *	pFilter = m_aFilters.GetAt( i ) ;
		if ( pFilter != NULL )
		{
			pFilter->Open( fmt ) ;
		}
	}
	return	sglErrSuccess ;
}

// 出力用に準備したサウンド出力を解放する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSoundMixerLinePlayer::Close( void )
{
	if ( m_flagPlaying )
	{
		Stop() ;
	}
	m_qbufWave.ClearAll() ;
	m_qbufFilter.ClearAll() ;
	m_flagOpened = false ;
	m_flagStatic = false ;
	m_bufStaticWave.RemoveAll() ;
	return	sglErrSuccess ;
}

// スタティックバッファを準備して書き込む
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSoundMixerLinePlayer::WriteStatic( const void * ptrSound, size_t nBytes )
{
	m_qbufWave.ClearAll() ;
	m_nPosition = 0 ;
	//
	m_flagStatic = true ;
	m_flagLoop = false ;
	m_bufStaticWave.RemoveAll() ;
	m_bufStaticWave.AddArray( (const uint8_t*) ptrSound, nBytes ) ;
	m_iStaticPos = 0 ;
	return	sglErrSuccess ;
}

// ストリームバッファを準備する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSoundMixerLinePlayer::PrepareStream( size_t nBytes )
{
	m_qbufWave.ClearAll() ;
	m_nPosition = 0 ;
	//
	m_flagStatic = false ;
	m_bufStaticWave.RemoveAll() ;
	return	sglErrSuccess ;
}

// ストリームバッファへ書き出す
//////////////////////////////////////////////////////////////////////////////
size_t SGLSoundMixerLinePlayer::Write( const void * ptrSound, size_t nBytes )
{
	m_csSync.Lock() ;
	nBytes = m_qbufWave.Write( ptrSound, nBytes ) ;
	m_csSync.Unlock() ;
	return	nBytes ;
}

// 再生を開始する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSoundMixerLinePlayer::Play( uint64_t nFlags )
{
	m_flagPlaying = true ;
	m_flagLoop = ((nFlags & flagPlayLoop) != 0) ;
	//
	SGLSoundSoftwareMixer *	pMixer = m_refMixer ;
	if ( pMixer != NULL )
	{
		pMixer->AddInputLine( this ) ;
	}
	//
	return	sglErrSuccess ;
}

// 再生を停止する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSoundMixerLinePlayer::Stop( void )
{
	m_flagPlaying = false ;
	//
	SGLSoundSoftwareMixer *	pMixer = m_refMixer ;
	if ( pMixer != NULL )
	{
		pMixer->DetachInputLine( this ) ;
	}
	//
	return	sglErrSuccess ;
}

// 再生を一時停止する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSoundMixerLinePlayer::Pause( void )
{
	m_flagPaused = true ;
	return	sglErrSuccess ;
}

// 再生を再開する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSoundMixerLinePlayer::Restart( void )
{
	m_flagPaused = false ;
	return	sglErrSuccess ;
}

// 音量取得 [L/R]
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSoundMixerLinePlayer::GetVolume( float32_t* pVolumes, size_t nChannels )
{
	for ( size_t i = 0; i < nChannels; i ++ )
	{
		if ( i < 2 )
		{
			pVolumes[i] = m_fpVolume[i] ;
		}
		else
		{
			pVolumes[i] = 0.0f ;
		}
	}
	return	sglErrSuccess ;
}

// 音量設定 [L/R]
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSoundMixerLinePlayer::SetVolume( const float32_t* pVolumes, size_t nChannels )
{
	for ( size_t i = 0; (i < 2) && (i < nChannels); i ++ )
	{
		m_fpVolume[i] = pVolumes[i] ;
	}
	return	sglErrSuccess ;
}

// 再生中か？
//////////////////////////////////////////////////////////////////////////////
bool SGLSoundMixerLinePlayer::IsPlaying( void ) const
{
	return	m_flagPlaying ;
}

// 一時停止中か？
//////////////////////////////////////////////////////////////////////////////
bool SGLSoundMixerLinePlayer::IsPaused( void ) const
{
	return	m_flagPaused ;
}

// 再生済みサンプル数を取得する
//////////////////////////////////////////////////////////////////////////////
uint64_t SGLSoundMixerLinePlayer::GetPlayingPosition( void )
{
	return	m_nPosition ;
}

// 再生位置 [/bytes] を設定する（スタティックバッファのみ）
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSoundMixerLinePlayer::SeekPosition( uint64_t nPos )
{
	m_csSync.Lock() ;
	m_iStaticPos = (size_t) nPos ;
	m_csSync.Unlock() ;
	return	sglErrSuccess ;
}

// コールバック設定
//////////////////////////////////////////////////////////////////////////////
SGLSoundPlayerListener *
	SGLSoundMixerLinePlayer::SetListener( SGLSoundPlayerListener * listener )
{
	m_csSync.Lock() ;
	listener = SGLSoundPlayerInterface::SetListener( listener ) ;
	m_csSync.Unlock() ;
	return	listener ;
}

// フォーマット取得
//////////////////////////////////////////////////////////////////////////////
const SGLSoundFormat& SGLSoundMixerLinePlayer::GetFormat( void ) const
{
	return	m_format ;
}

// ストリーミング取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLSoundMixerLinePlayer::ReadStream( void * ptrSound, size_t nBytes )
{
	size_t	nBlockAlign = m_format.channels * m_format.bitsPerSample / 8 ;
	size_t	nReadBytes = 0 ;
	while ( nBytes > 0 )
	{
		SArray<uint8_t>	bufStream ;
		uint8_t *		pbytTemp = bufStream.GetArray( nBytes ) ;
		size_t			nQueRead = 0 ;
		m_csSync.Lock() ;
		//
		nQueRead = m_qbufWave.Read( pbytTemp, nBytes ) ;
		bufStream.FinishArray() ;
		bufStream.SetLength( nQueRead ) ;
		//
		FilterSoundStream( bufStream ) ;
		//
		nQueRead = m_qbufFilter.Read( ptrSound, nBytes ) ;
		EffectVolume( ptrSound, nQueRead ) ;
		//
		m_csSync.Unlock() ;
		nReadBytes += nQueRead ;
		//
		if ( nQueRead >= nBytes )
		{
			break ;
		}
		ptrSound = ((uint8_t*) ptrSound) + nQueRead ;
		nBytes -= nQueRead ;
		if ( nBytes < nBlockAlign )
		{
			break ;
		}
		//
		if ( m_flagStatic )
		{
			size_t	nBlockAlign = m_format.channels
									* m_format.bitsPerSample / 8 ;
			if ( nBlockAlign == 0 )
			{
				break ;
			}
			size_t	nPlayPos = m_iStaticPos * nBlockAlign ;
			if ( nPlayPos >= m_bufStaticWave.GetLength() )
			{
				if ( !m_flagLoop )
				{
					Stop() ;
					break ;
				}
				nPlayPos = 0 ;
			}
			size_t	nLeftBytes = m_bufStaticWave.GetLength() - nPlayPos ;
			m_csSync.Lock() ;
			m_qbufWave.Write
				( m_bufStaticWave.GetConstArray() + nPlayPos, nLeftBytes ) ;
			m_iStaticPos = (nPlayPos + nLeftBytes) / nBlockAlign ;
			m_csSync.Unlock() ;
		}
		else
		{
			if ( m_pListener == NULL )
			{
				break ;
			}
			m_pListener->OnStreaming( this ) ;
		}
		//
		m_csSync.Lock() ;
		if ( m_qbufWave.GetLength() == 0 )
		{
			m_csSync.Unlock() ;
			break ;
		}
		m_csSync.Unlock() ;
	}
	return	nReadBytes ;
}

// フィルターを追加
//////////////////////////////////////////////////////////////////////////////
size_t SGLSoundMixerLinePlayer::AddFilter( SGLSoundFilterInterface * pFilter )
{
	size_t	i ;
	m_csSync.Lock() ;
	i = m_aFilters.Add( pFilter ) ;
	if ( m_flagOpened )
	{
		pFilter->Open( m_format ) ;
	}
	m_csSync.Unlock() ;
	return	i ;
}

size_t SGLSoundMixerLinePlayer::InserFilter( size_t i, SGLSoundFilterInterface * pFilter )
{
	m_csSync.Lock() ;
	if ( i > m_aFilters.GetLength() )
	{
		i = m_aFilters.GetLength() ;
	}
	m_aFilters.InsertAt( i, pFilter ) ;
	if ( m_flagOpened )
	{
		pFilter->Open( m_format ) ;
	}
	m_csSync.Unlock() ;
	return	i ;
}

// フィルターを削除
//////////////////////////////////////////////////////////////////////////////
void SGLSoundMixerLinePlayer::DetachFilter( SGLSoundFilterInterface * pFilter )
{
	m_csSync.Lock() ;
	ssize_t	i = m_aFilters.FindPtr( pFilter ) ;
	if ( i >= 0 )
	{
		m_aFilters.RemoveAt( (size_t) i ) ;
	}
	m_csSync.Unlock() ;
}

void SGLSoundMixerLinePlayer::DetachFilterAt( size_t i )
{
	m_csSync.Lock() ;
	m_aFilters.RemoveAt( i ) ;
	m_csSync.Unlock() ;
}

void SGLSoundMixerLinePlayer::DetachAllFilters( void )
{
	m_csSync.Lock() ;
	m_aFilters.RemoveAll() ;
	m_csSync.Unlock() ;
}

// フィルター処理
//////////////////////////////////////////////////////////////////////////////
void SGLSoundMixerLinePlayer::FilterSoundStream( SSystem::SArray<uint8_t>& bufStream )
{
	for ( size_t i = 0; i < m_aFilters.GetLength(); i ++ )
	{
		SGLSoundFilterInterface *	pFilter = m_aFilters.GetAt( i ) ;
		if ( pFilter == NULL )
		{
			continue ;
		}
		size_t	nBytes = bufStream.GetLength() ;
		pFilter->FilterStream( bufStream.GetConstArray(), nBytes ) ;
		//
		size_t	nFilteredBytes = pFilter->GetBufferedBytes() ;
		pFilter->ReadStream
			( bufStream.GetArray(nFilteredBytes), nFilteredBytes ) ;
		bufStream.FinishArray() ;
		bufStream.SetLength( nFilteredBytes ) ;
	}
	m_qbufFilter.Write( bufStream.GetConstArray(), bufStream.GetLength() ) ;
}

// 音量反映処理
//////////////////////////////////////////////////////////////////////////////
void SGLSoundMixerLinePlayer::EffectVolume( void * ptrSound, size_t nBytes )
{
	size_t	nBlockAlign = m_format.channels * m_format.bitsPerSample / 8 ;
	if ( nBlockAlign <= 0 )
	{
		return ;
	}
	size_t	nSamples = nBytes / nBlockAlign ;
	size_t	nChannels = m_format.channels ;
	if ( m_format.bitsPerSample == 16 )
	{
		int16_t *	pwSound = (int16_t*) ptrSound ;
		for ( size_t i = 0; i < nChannels; i ++, pwSound ++ )
		{
			int32_t	fxVol ;
			if ( i < 2 )
			{
				fxVol = eslRoundR32ToInt( m_fpVolume[i] * 0x10000 ) ;
			}
			else
			{
				fxVol =
					eslRoundR32ToInt
						( (m_fpVolume[0] + m_fpVolume[1]) * 0x8000 ) ;
			}
			if ( fxVol >= 0x100000 )
			{
				fxVol = 0x100000 ;
			}
			int16_t *	pwNext = pwSound ;
			for ( size_t j = 0; j < nSamples; j ++, pwNext += nChannels )
			{
				*pwNext = (int16_t) esl_clampi
					( (int) (((int64_t) *pwNext * fxVol) >> 16), -0x7FFF, 0x7FFF ) ;
			}
		}
	}
	else if ( m_format.bitsPerSample == 8 )
	{
		uint8_t *	pbytSound = (uint8_t*) ptrSound ;
		for ( size_t i = 0; i < nChannels; i ++, pbytSound ++ )
		{
			int32_t	fxVol ;
			if ( i < 2 )
			{
				fxVol = eslRoundR32ToInt( m_fpVolume[i] * 0x100 ) ;
			}
			else
			{
				fxVol =
					eslRoundR32ToInt
						( (m_fpVolume[0] + m_fpVolume[1]) * 0x80 ) ;
			}
			if ( fxVol >= 0x100 )
			{
				fxVol = 0x100 ;
			}
			uint8_t *	pbytNext = pbytSound ;
			for ( size_t j = 0; j < nSamples; j ++, pbytNext += nChannels )
			{
				*pbytNext =
					(uint8_t) ((((int32_t) *pbytNext - 0x80) * fxVol) >> 8) + 0x80 ;
			}
		}
	}
}


//////////////////////////////////////////////////////////////////////////////
// 周波数フィルタ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLSoundEqualizerFilter, SGLSoundFilterInterface )
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLSoundEqualizerProcessor, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSoundEqualizerFilter::SGLSoundEqualizerFilter( void )
{
	m_nSampleAlign = 0 ;
	m_nDCTDegree = 0 ;
	m_nMatrixDiv = 1 ;
	m_nMatrixSize = 1 ;
	m_nBlockSize = 1 ;
	m_nPreBufferCount = 0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSoundEqualizerFilter::~SGLSoundEqualizerFilter( void )
{
}

// ブロックサイズと窓を設定
//////////////////////////////////////////////////////////////////////////////
void SGLSoundEqualizerFilter::Initialize( int nDCTDegree, int nDivCount )
{
	m_nDCTDegree = (size_t) nDCTDegree ;
	m_nMatrixDiv = (size_t) nDivCount ;
	m_nMatrixSize = (size_t) 1 << m_nDCTDegree ;
	m_nBlockSize = m_nMatrixSize / m_nMatrixDiv ;
	//
	SArray<float32_t>	bufKaiser ;
	float32_t *			pfpKDB = m_bufKDB.GetArray( m_nMatrixSize ) ;
	const size_t		nHalfMatrix = m_nMatrixSize / 2 ;
	ERISA::sclfGenerateKaiserBesselDerivedWindow
		( pfpKDB, bufKaiser.GetArray( nHalfMatrix + 1 ),
					(float32_t) m_nMatrixDiv, nHalfMatrix ) ;
	bufKaiser.FinishArray() ;
	//
	for ( size_t i = 0; i < nHalfMatrix; i ++ )
	{
		pfpKDB[nHalfMatrix + i] = pfpKDB[nHalfMatrix - i - 1] ;
	}
	m_bufKDB.FinishArray() ;
}

// イコライザを追加
//////////////////////////////////////////////////////////////////////////////
size_t SGLSoundEqualizerFilter::AddEqualizer( SGLSoundEqualizerProcessor * pEq )
{
	return	m_aEqualizers.Add( pEq ) ;
}

size_t SGLSoundEqualizerFilter::InserEqualizer( size_t i, SGLSoundEqualizerProcessor * pEq )
{
	if ( i > m_aEqualizers.GetLength() )
	{
		i = m_aEqualizers.GetLength() ;
	}
	m_aEqualizers.InsertAt( i, pEq ) ;
	return	i ;
}

// イコライザを削除
//////////////////////////////////////////////////////////////////////////////
void SGLSoundEqualizerFilter::DetachEqualizer( SGLSoundEqualizerProcessor * pEq )
{
	ssize_t	i = m_aEqualizers.FindPtr( pEq ) ;
	if ( i >= 0 )
	{
		m_aEqualizers.RemoveAt( (size_t) i ) ;
	}
}

void SGLSoundEqualizerFilter::DetachEqualizerAt( size_t i )
{
	m_aEqualizers.RemoveAt( i ) ;
}

void SGLSoundEqualizerFilter::DetachAllEqualizers( void )
{
	m_aEqualizers.RemoveAll() ;
}

// フォーマット設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSoundEqualizerFilter::Open( const SGLSoundFormat& fmt )
{
	m_fmtSound = fmt ;
	m_nSampleAlign = fmt.channels * fmt.bitsPerSample / 8 ;
	//
	m_bufAccIDCT.SetLength( 0 ) ;
	m_bufAccIDCT.SetLength( m_nMatrixSize * fmt.channels ) ;
	m_bufStreamBlock.SetLength( 0 ) ;
	m_bufStreamBlock.SetLength( m_nMatrixSize * fmt.channels ) ;
	m_bufWorkDCT.SetLength( m_nMatrixSize ) ;
	m_bufWorkIDCT.SetLength( m_nMatrixSize ) ;
	m_bufWorkTemp1.SetLength( m_nMatrixSize ) ;
	m_bufWorkTemp2.SetLength( m_nMatrixSize ) ;
	//
	m_nPreBufferCount = m_nMatrixSize ;
	//
	uint8_t *	pPreBuf =
					m_qbufEqIn.PutBuffer( m_nBlockSize * m_nSampleAlign ) ;
	eslFillMemory( pPreBuf, 0, m_nBlockSize * m_nSampleAlign ) ;
	m_qbufEqIn.FlushBuffer( m_nBlockSize * m_nSampleAlign ) ;
	//
	return	sglErrSuccess ;
}

// ストリームの完了
//////////////////////////////////////////////////////////////////////////////
void SGLSoundEqualizerFilter::FlushStream( void )
{
	SArray<uint8_t>	bufTemp ;
	size_t	nTempBytes = m_nMatrixSize * m_nSampleAlign ;
	FilterStream( bufTemp.GetArray( nTempBytes ), nTempBytes ) ;
	bufTemp.FinishArray() ;
}

// フィルタ処理
//////////////////////////////////////////////////////////////////////////////
size_t SGLSoundEqualizerFilter::FilterStream( const void * ptrSound, size_t nBytes )
{
	m_qbufEqIn.Write( ptrSound, nBytes ) ;
	//
	for ( ; ; )
	{
		size_t	nLeftBuf = (size_t) m_qbufEqIn.GetLength() / m_nSampleAlign ;
		if ( nLeftBuf < m_nBlockSize )
		{
			break ;
		}
		size_t		nInBytes = m_nBlockSize * m_nSampleAlign ;
		uint8_t *	pInBuf = (uint8_t*) m_qbufEqIn.GetBuffer( nInBytes ) ;
		//
		for ( size_t iCh = 0; iCh < m_fmtSound.channels; iCh ++ )
		{
			AddStreamToDCTBuffer( iCh, pInBuf ) ;
			ProcessEqualizer( iCh ) ;
			OutputEqualizer( iCh ) ;
		}
		BlockStreamOutput() ;
		//
		m_qbufEqIn.ReleaseBuffer( (ssize_t) nInBytes ) ;
	}
	return	nBytes ;
}

// フィルタ処理したデータのサイズを取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLSoundEqualizerFilter::GetBufferedBytes( void )
{
	return	(size_t) m_qbufEqOut.GetLength() ;
}

// フィルタ処理したデータを取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLSoundEqualizerFilter::ReadStream( void * ptrSound, size_t nBytes )
{
	return	m_qbufEqOut.Read( ptrSound, nBytes ) ;
}

// DCT バッファの末尾に入力データを追加
//////////////////////////////////////////////////////////////////////////////
void SGLSoundEqualizerFilter::AddStreamToDCTBuffer( size_t iChannel, const uint8_t * pInBuf )
{
	//
	// 入力 PCM をサンプリング
	//
	ESLAssert( iChannel < m_fmtSound.channels ) ;
	ESLAssert( (iChannel + 1) * m_nMatrixSize <= m_bufStreamBlock.GetLength() ) ;
	float32_t *	pStreamBlock = m_bufStreamBlock.GetAt( iChannel * m_nMatrixSize ) ;
	ESLAssert( pStreamBlock != NULL ) ;
	//
	const size_t	nMatrixSize = m_nMatrixSize ;
	const size_t	iBlockOffset = nMatrixSize - m_nBlockSize ;
	const size_t	nSrcSampleBytes = m_fmtSound.bitsPerSample / 8 ;
	eslMoveMemory
		( pStreamBlock, pStreamBlock + m_nBlockSize,
			(nMatrixSize - m_nBlockSize) * sizeof(float32_t) ) ;
	sglDecodeSoundTo32bitsPCM
		( pStreamBlock + iBlockOffset, 1, 0, m_fmtSound,
			pInBuf + (iChannel * nSrcSampleBytes), m_nBlockSize, 1 ) ;
	//
	// 窓関数
	//
	float32_t *	pBufTemp2 = m_bufWorkTemp2.GetArray( nMatrixSize ) ;
	eslCopyMemory
		( pBufTemp2, pStreamBlock, nMatrixSize * sizeof(float32_t) ) ;
	//
	if ( m_nMatrixDiv > 1 )
	{
		const float32_t *	pfpKDB = m_bufKDB.GetConstArray() ;
		float32_t	k = (float32_t) (4.0 / (m_nBlockSize * m_nMatrixDiv * m_nMatrixDiv)) ;
		for ( size_t i = 0; i < nMatrixSize; i ++ )
		{
			pBufTemp2[i] *= pfpKDB[i] * k ;
		}
	}
	else
	{
		float32_t	k = (float32_t) (2.0 / m_nBlockSize) ;
		for ( size_t i = 0; i < nMatrixSize; i ++ )
		{
			pBufTemp2[i] *= k ;
		}
	}
	//
	// DCT 変換
	//
	float32_t *	pBufDCT = m_bufWorkDCT.GetArray( nMatrixSize ) ;
	float32_t *	pBufTemp1 = m_bufWorkTemp1.GetArray( nMatrixSize ) ;
	//
	ERISA::sclfFastDCT
		( pBufDCT, 1, pBufTemp2, pBufTemp1, m_nDCTDegree ) ;
	//
	m_bufWorkDCT.FinishArray() ;
	m_bufWorkTemp1.FinishArray() ;
	m_bufWorkTemp2.FinishArray() ;
}

// フィルタ処理
//////////////////////////////////////////////////////////////////////////////
void SGLSoundEqualizerFilter::ProcessEqualizer( size_t iChannel )
{
	float32_t *	pBufDCT = m_bufWorkDCT.GetArray() ;
	//
	for ( size_t i = 0; i < m_aEqualizers.GetLength(); i ++ )
	{
		SGLSoundEqualizerProcessor *	pEq = m_aEqualizers.GetAt( i ) ;
		if ( pEq != NULL )
		{
			pEq->OnEqualizer( iChannel, pBufDCT, m_nMatrixSize ) ;
		}
	}
	m_bufWorkDCT.FinishArray() ;
}

// 処理後のDCT級数を逆変換して出力バッファに加算
//////////////////////////////////////////////////////////////////////////////
void SGLSoundEqualizerFilter::OutputEqualizer( size_t iChannel )
{
	//
	// 逆 DCT 変換
	//
	const size_t	nMatrixSize = m_nMatrixSize ;
	float32_t *	pBufDCT = m_bufWorkDCT.GetArray( nMatrixSize ) ;
	float32_t *	pBufIDCT = m_bufWorkIDCT.GetArray( nMatrixSize ) ;
	float32_t *	pBufTemp1 = m_bufWorkTemp1.GetArray( nMatrixSize ) ;
	//
	ERISA::sclfFastIDCT
		( pBufIDCT, pBufDCT, 1, pBufTemp1, m_nDCTDegree ) ;
	//
	// 出力バッファに加算
	//
	ESLAssert( iChannel < m_fmtSound.channels ) ;
	ESLAssert( (iChannel + 1) * m_nMatrixSize <= m_bufAccIDCT.GetLength() ) ;
	float32_t *	pAccIDCT = m_bufAccIDCT.GetAt( iChannel * nMatrixSize ) ;
	ESLAssert( pAccIDCT != NULL ) ;
	//
	eslMoveMemory
		( pAccIDCT, pAccIDCT + m_nBlockSize,
			(nMatrixSize - m_nBlockSize) * sizeof(float32_t) ) ;
	eslFillMemory
		( pAccIDCT + (nMatrixSize - m_nBlockSize),
			0, m_nBlockSize * sizeof(float32_t) ) ;
	//
	for ( size_t i = 0; i < nMatrixSize; i ++ )
	{
		pAccIDCT[i] += pBufIDCT[i] ;
	}
	//
	m_bufWorkDCT.FinishArray() ;
	m_bufWorkIDCT.FinishArray() ;
	m_bufWorkTemp1.FinishArray() ;
}

// 1ブロックを出力バッファにストリーム出力
//////////////////////////////////////////////////////////////////////////////
void SGLSoundEqualizerFilter::BlockStreamOutput( void )
{
	if ( m_nPreBufferCount >= m_nBlockSize )
	{
		m_nPreBufferCount -= m_nBlockSize ;
		return ;
	}
	const float32_t *	pAccIDCT = m_bufAccIDCT.GetConstArray() ;
	uint8_t *			pDstBuf = m_qbufEqOut.PutBuffer( m_nBlockSize * m_nSampleAlign ) ;
	//
	sglEncodeSoundFrom32bitsPCM
		( m_fmtSound, pDstBuf, pAccIDCT,
			1, m_nMatrixSize, m_nBlockSize, m_fmtSound.channels ) ;
	//
	m_qbufEqOut.FlushBuffer( m_nBlockSize * m_nSampleAlign ) ;
}

