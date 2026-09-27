
#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl_media.h>
#include <sakuragl/sgl_mme_sound_recorder.h>
#include <sakuracl/erisa/sgl_erisa_crypt_math.h>

#include <functiondiscoverykeys_devpkey.h>
#include <GuidDef.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// Win32 マルチメディア API サウンド入力インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLWin32MMSoundRecorder, SGLSoundRecorderInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLWin32MMSoundRecorder::SGLWin32MMSoundRecorder( void )
{
	m_hWaveIn = NULL ;
	m_fRecording = false ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLWin32MMSoundRecorder::~SGLWin32MMSoundRecorder( void )
{
	Close() ;
}

// デバイス列挙
//////////////////////////////////////////////////////////////////////////////
size_t SGLWin32MMSoundRecorder::EnumerateDevices
		( uint16_t * pwNameBuf, size_t nNameBufLength )
{
	size_t	nCount = 0 ;
	size_t	iNameBuf = 0 ;
	UINT	nDevs = waveInGetNumDevs() ;
	for ( UINT i = 0; i < nDevs; i ++ )
	{
		WAVEINCAPS	wic ;
		if ( waveInGetDevCaps
			( i, &wic, sizeof(WAVEINCAPS) ) == MMSYSERR_NOERROR )
		{
			SString				strName = wic.szPname ;
			const uint16_t *	pwName = strName.GetConstArray() ;
			for ( size_t j = 0; iNameBuf < nNameBufLength; j ++ )
			{
				if ( (pwNameBuf[iNameBuf ++] = pwName[j]) == 0 )
				{
					nCount ++ ;
					break ;
				}
			}
		}
	}
	return	nCount ;
}

// フォーマットを指定して入力を準備する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWin32MMSoundRecorder::Open( size_t iDevice, const SGLSoundFormat& fmt )
{
	Close() ;
	//
	m_wfx.wFormatTag = WAVE_FORMAT_PCM ;
	m_wfx.nChannels = (WORD) fmt.channels ;
	m_wfx.nSamplesPerSec = fmt.frequency ;
	m_wfx.wBitsPerSample = (WORD) fmt.bitsPerSample ;
	m_wfx.nBlockAlign = (WORD) (fmt.channels * (fmt.bitsPerSample >> 3)) ;
	m_wfx.nAvgBytesPerSec = fmt.frequency * m_wfx.nBlockAlign ;
	m_wfx.cbSize = 0 ;
	//
	if ( waveInOpen
		( &m_hWaveIn, (UINT) iDevice, &m_wfx,
			(DWORD_PTR) &SGLWin32MMSoundRecorder::waveInProc,
			(DWORD_PTR) this, CALLBACK_FUNCTION ) != MMSYSERR_NOERROR )
	{
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

// 入力用に準備したサウンド入力を解放する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWin32MMSoundRecorder::Close( void )
{
	if ( m_fRecording )
	{
		Stop() ;
	}
	if ( m_hWaveIn != NULL )
	{
		waveInClose( m_hWaveIn ) ;
		m_hWaveIn = NULL ;
	}
	m_csStream.Lock() ;
	m_qbufStream.ClearAll() ;
	m_csStream.Unlock() ;
	return	sglErrSuccess ;
}

// ストリームバッファを準備する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWin32MMSoundRecorder::PrepareStream( size_t nBytes )
{
	if ( m_hWaveIn == NULL )
	{
		return	sglErrFailed ;
	}
	nBytes -= (nBytes % m_wfx.nBlockAlign) ;
	if ( nBytes == 0 )
	{
		nBytes = (m_wfx.nSamplesPerSec / 16) * m_wfx.nBlockAlign ;
	}
	m_bufRecord.SetLength( nBytes * 2 ) ;
	return	sglErrSuccess ;
}

// ストリームバッファから読み出す
//////////////////////////////////////////////////////////////////////////////
size_t SGLWin32MMSoundRecorder::Read( void * ptrSound, size_t nBytes )
{
	size_t	nRead ;
	m_csStream.Lock() ;
	nRead = m_qbufStream.Read( ptrSound, nBytes ) ;
	m_csStream.Unlock() ;
	return	nRead ;
}

// 録音を開始する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWin32MMSoundRecorder::Start( uint64_t nFlags )
{
	if ( m_hWaveIn == NULL )
	{
		return	sglErrFailed ;
	}
	if ( m_fRecording )
	{
		return	sglErrSuccess ;
	}
	size_t		i ;
	size_t		nBytes = m_bufRecord.GetLength() >> 1 ;
	uint8_t *	pbytBuf = m_bufRecord.GetArray() ;
	//
	for ( i = 0; i < 2; i ++ )
	{
		memset( &m_wavhdr[i], 0, sizeof(WAVEHDR) ) ;
		m_wavhdr[i].lpData = (LPSTR) pbytBuf ;
		m_wavhdr[i].dwBufferLength = (DWORD) nBytes ;
		if ( waveInPrepareHeader
			( m_hWaveIn, &m_wavhdr[i], sizeof(WAVEHDR) ) != MMSYSERR_NOERROR )
		{
			return	sglErrFailed ;
		}
		pbytBuf += nBytes ;
	}
	m_bufRecord.FinishArray() ;
	//
	waveInReset( m_hWaveIn ) ;
	//
	for ( i = 0; i < 2; i ++ )
	{
		m_wavhdr[i].dwBytesRecorded = 0 ;
		waveInAddBuffer( m_hWaveIn, &m_wavhdr[i], sizeof(WAVEHDR) ) ;
	}
	waveInStart( m_hWaveIn ) ;
	m_fRecording = true ;
	return	sglErrSuccess ;
}

// 録音を停止する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWin32MMSoundRecorder::Stop( void )
{
	if ( m_hWaveIn == NULL )
	{
		return	sglErrFailed ;
	}
	if ( !m_fRecording )
	{
		return	sglErrSuccess ;
	}
	waveInStop( m_hWaveIn ) ;
	m_fRecording = false ;
	//
	for ( size_t i = 0; i < 2; i ++ )
	{
		waveInUnprepareHeader
			( m_hWaveIn, &m_wavhdr[i], sizeof(WAVEHDR) ) ;
	}
	return	sglErrSuccess ;
}

// 音量取得 [L/R]
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWin32MMSoundRecorder::GetVolume( float32_t* pVolumes, size_t nChannels )
{
	for ( size_t i = 0; i < nChannels; i ++ )
	{
		pVolumes[i] = 1.0f ;
	}
	return	sglErrSuccess ;
}

// 音量設定 [L/R]
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWin32MMSoundRecorder::SetVolume( const float32_t* pVolumes, size_t nChannels )
{
	return	sglErrFailed ;
}

// 録音中か？
//////////////////////////////////////////////////////////////////////////////
bool SGLWin32MMSoundRecorder::IsRecording( void ) const
{
	return	m_fRecording ;
}

// コールバック関数
//////////////////////////////////////////////////////////////////////////////
void CALLBACK SGLWin32MMSoundRecorder::waveInProc
	( HWAVEIN hwi, UINT uMsg,
		DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD dwParam2 )
{
	SGLWin32MMSoundRecorder *
			prec = (SGLWin32MMSoundRecorder*) dwInstance ;
	if ( uMsg == WIM_DATA )
	{
		WAVEHDR *	pwh = (WAVEHDR*) dwParam1 ;
		prec->m_csStream.Lock() ;
		prec->m_qbufStream.Write( pwh->lpData, pwh->dwBytesRecorded ) ;
		prec->m_csStream.Unlock() ;
		//
		pwh->dwBytesRecorded = 0 ;
		waveInAddBuffer( prec->m_hWaveIn, pwh, sizeof(WAVEHDR) ) ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// WASAPI サウンド入力インターフェース (Windows Vista 以降でのみ動作)
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLWinCoreAudioRecorder, SGLSoundRecorderInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLWinCoreAudioRecorder::SGLWinCoreAudioRecorder( void )
{
	m_pCapDevice = NULL ;
	m_pAudioClient = NULL ;
	m_pCaptureClient = NULL ;
	m_maskTargetDevices = deviceCapture | deviceRender ;
	m_flagReady = false ;
	m_flagRecording = false ;
	m_nCapSamples = 0 ;
	m_nOutSamples = 0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLWinCoreAudioRecorder::~SGLWinCoreAudioRecorder( void )
{
	Close() ;
}

// デバイス列挙
//////////////////////////////////////////////////////////////////////////////
size_t SGLWinCoreAudioRecorder::EnumerateDevices
		( uint16_t * pwNameBuf, size_t nNameBufLength )
{
	const PROPERTYKEY	PKEY_Device_FriendlyName =
		{ { 0xa45c254e, 0xdf1c, 0x4efd,
			{ 0x80, 0x20, 0x67, 0xd1, 0x46, 0xa8, 0x50, 0xe0 } }, 14 } ;
	//
	const EDataFlow	dataFlow[2] =
	{
		eCapture, eRender,
	} ;
	size_t	iDev = 0 ;
	size_t	iName = 0 ;
	for ( int iDataFlow = 0; iDataFlow < 2; iDataFlow ++ )
	{
		IMMDeviceEnumerator *	pEnum = NULL ;
		if ( ::CoCreateInstance
			( __uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL,
				__uuidof(IMMDeviceEnumerator), (void**) &pEnum ) != S_OK )
		{
			continue ;
		}
		IMMDeviceCollection *	pEndCollect = NULL ;
		if ( pEnum->EnumAudioEndpoints
			( dataFlow[iDataFlow], DEVICE_STATE_ACTIVE, &pEndCollect ) != S_OK )
		{
			pEnum->Release() ;
			continue ;
		}
		pEnum->Release() ;
		//
		UINT	nCount = 0 ;
		pEndCollect->GetCount( &nCount ) ;
		if ( nCount == 0 )
		{
			pEndCollect->Release() ;
			continue ;
		}
		for ( UINT i = 0; (i < nCount) && (iName < nNameBufLength); i ++ )
		{
			IMMDevice *	pItem = NULL ;
			pEndCollect->Item( i, &pItem ) ;
			if ( pItem == NULL )
			{
				continue ;
			}
			IPropertyStore *	pPropStore = NULL ;
			PROPVARIANT			varName ;
			PropVariantInit( &varName ) ;
			pItem->OpenPropertyStore( STGM_READ, &pPropStore ) ;
			pPropStore->GetValue( PKEY_Device_FriendlyName, &varName ) ;
			for ( size_t j = 0; iName < nNameBufLength; j ++ )
			{
				if ( (pwNameBuf[iName ++]
						= (uint16_t) varName.pwszVal[j]) == 0 )
				{
					iDev ++ ;
					break ;
				}
			}
			PropVariantClear( &varName ) ;
			pItem->Release();
		}
		pEndCollect->Release() ;
	}
	return	iDev ;
}

// フォーマットを指定して入力を準備する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWinCoreAudioRecorder::Open( size_t iDevice, const SGLSoundFormat& fmt )
{
	Close() ;
	//
	// デバイス選択
	//
	IMMDeviceEnumerator *	pEnum = NULL ;
	if ( ::CoCreateInstance
		( __uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL,
			__uuidof(IMMDeviceEnumerator), (void**) &pEnum ) != S_OK )
	{
		return	sglErrFailed ;
	}
	const EDataFlow	dataFlow[2] =
	{
		eCapture, eRender,
	} ;
	IMMDeviceCollection *	pEndCollect = NULL ;
	int	iDataFlow ;
	for ( iDataFlow = 0; iDataFlow < 2; iDataFlow ++ )
	{
		if ( pEnum->EnumAudioEndpoints
			( dataFlow[iDataFlow], DEVICE_STATE_ACTIVE, &pEndCollect ) != S_OK )
		{
			continue ;
		}
		//
		UINT	nCount = 0 ;
		pEndCollect->GetCount( &nCount ) ;
		if ( iDevice >= nCount )
		{
			iDevice -= nCount ;
			pEndCollect->Release() ;
			pEndCollect = NULL ;
		}
		else
		{
			break ;
		}
	}
	pEnum->Release() ;
	if ( pEndCollect == NULL )
	{
		return	sglErrFailed ;
	}
	//
	IMMDevice *	pItem = NULL ;
	pEndCollect->Item( (UINT) iDevice, &pItem ) ;
	pEndCollect->Release() ;
	if ( pItem == NULL )
	{
		return	sglErrFailed ;
	}
	//
	// AudioClient インターフェース取得
	//
	m_pCapDevice = pItem ;
	if ( m_pCapDevice->Activate
		( __uuidof(IAudioClient),
			CLSCTX_ALL, NULL, (void**)&m_pAudioClient ) != S_OK )
	{
		m_pCapDevice->Release() ;
		m_pAudioClient = NULL ;
		m_pCapDevice = NULL ;
		return	sglErrFailed ;
	}
	//
	// キャプチャフォーマット設定
	//
	WAVEFORMATEXTENSIBLE	wfxeTemp ;
	WAVEFORMATEXTENSIBLE *	pwfxe = NULL ;
	AUDCLNT_SHAREMODE		shareMode = AUDCLNT_SHAREMODE_EXCLUSIVE ;
	DWORD					dwStreamFlags = 0 ;
	m_pAudioClient->GetMixFormat( (WAVEFORMATEX**) &pwfxe ) ;
//	pwfxe->SubFormat = KSDATAFORMAT_SUBTYPE_PCM ;
//	pwfxe->Samples.wValidBitsPerSample = 24 ;
	//
	if ( iDataFlow == 1 )
	{
		shareMode = AUDCLNT_SHAREMODE_SHARED ;
		dwStreamFlags = AUDCLNT_STREAMFLAGS_LOOPBACK ;
	}
	if ( m_pAudioClient->IsFormatSupported
		( shareMode, (WAVEFORMATEX*) pwfxe, NULL ) != S_OK )
	{
		WAVEFORMATEXTENSIBLE *	pwfxeClosest = NULL ;
		HRESULT	hr = m_pAudioClient->IsFormatSupported
			( AUDCLNT_SHAREMODE_SHARED,
				(WAVEFORMATEX*) pwfxe, (WAVEFORMATEX**) &pwfxeClosest ) ;
		if ( hr != S_OK )
		{
			if ( (hr == S_FALSE) && (pwfxeClosest != NULL) )
			{
				*pwfxe = *pwfxeClosest ;
				if ( pwfxeClosest != &wfxeTemp )
				{
					CoTaskMemFree( pwfxeClosest ) ;
				}
				hr = m_pAudioClient->IsFormatSupported
						( AUDCLNT_SHAREMODE_SHARED,
								(WAVEFORMATEX*) pwfxe, NULL ) ;
				ESLAssert( hr == S_OK ) ;
			}
			else
			{
				if ( pwfxe != &wfxeTemp )
				{
					CoTaskMemFree( pwfxe ) ;
				}
				return	sglErrFailed ;
			}
		}
		shareMode = AUDCLNT_SHAREMODE_SHARED ;
	}
	m_fmtCapture.format = formatSoundLinearPCM ;
	m_fmtCapture.frequency = pwfxe->Format.nSamplesPerSec ;
	m_fmtCapture.channels = pwfxe->Format.nChannels ;
	m_fmtCapture.bitsPerSample = pwfxe->Format.wBitsPerSample ;
	//
	if ( pwfxe->Format.wFormatTag == WAVE_FORMAT_EXTENSIBLE )
	{
		if ( IsEqualGUID( pwfxe->SubFormat, KSDATAFORMAT_SUBTYPE_IEEE_FLOAT ) )
		{
			m_fmtCapture.format = formatSoundIEEEFloat ;
		}
	}
	else
	{
		if ( pwfxe->Format.wFormatTag == WAVE_FORMAT_IEEE_FLOAT )
		{
			m_fmtCapture.format = formatSoundIEEEFloat ;
		}
	}
	//
	REFERENCE_TIME	minDevPeriod = 0 ;
	if ( m_pAudioClient->GetDevicePeriod( NULL, &minDevPeriod ) != S_OK )
	{
		if ( pwfxe != &wfxeTemp )
		{
			CoTaskMemFree( pwfxe ) ;
		}
		return	sglErrFailed ;
	}
	HRESULT	res ;
	res = m_pAudioClient->Initialize
		( shareMode, dwStreamFlags,
			minDevPeriod, minDevPeriod,
			(WAVEFORMATEX*) pwfxe, &GUID_NULL ) ;
	if ( FAILED( res ) )
	{
		if( res == AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED )
		{
			UINT32	nFrames = 0 ;
			m_pAudioClient->GetBufferSize( &nFrames ) ;
			minDevPeriod = (REFERENCE_TIME)
				( 10000.0 * 1000 * nFrames
						/ pwfxe->Format.nSamplesPerSec + 0.5) ;
			//
			m_pAudioClient->Release() ;
			m_pAudioClient = NULL ;
			//
			if ( m_pCapDevice->Activate
				( __uuidof(IAudioClient),
					CLSCTX_ALL, NULL, (void**)&m_pAudioClient ) != S_OK )
			{
				m_pCapDevice->Release() ;
				m_pAudioClient = NULL ;
				m_pCapDevice = NULL ;
				return	sglErrFailed ;
			}
			res = m_pAudioClient->Initialize
				( shareMode, dwStreamFlags,
					minDevPeriod, minDevPeriod,
					(WAVEFORMATEX*) pwfxe, &GUID_NULL ) ;
		}
		if ( FAILED( res ) )
		{
			if ( pwfxe != &wfxeTemp )
			{
				CoTaskMemFree( pwfxe ) ;
			}
			return	sglErrFailed ;
		}
	}
	if ( pwfxe != &wfxeTemp )
	{
		CoTaskMemFree( pwfxe ) ;
	}
	//
	// キャプチャーインターフェース取得
	//
	if ( m_pAudioClient->GetService
		( __uuidof(IAudioCaptureClient), (void**) &m_pCaptureClient ) != S_OK )
	{
		return	sglErrFailed ;
	}
	//
	m_flagReady = true ;
	m_fmtOutput = fmt ;
	m_lcmFreqConvert = SakuraCL::ComputeLCM<uint64_t>
					( m_fmtOutput.frequency, m_fmtCapture.frequency ) ;
	//
	return	sglErrSuccess ;
}

// 入力用に準備したサウンド入力を解放する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWinCoreAudioRecorder::Close( void )
{
	Stop() ;
	//
	if ( m_pCaptureClient != NULL )
	{
		m_pCaptureClient->Release() ;
		m_pCaptureClient = NULL ;
	}
	if ( m_pAudioClient != NULL )
	{
		m_pAudioClient->Release() ;
		m_pAudioClient = NULL ;
	}
	if ( m_pCapDevice != NULL )
	{
		m_pCapDevice->Release() ;
		m_pCapDevice = NULL ;
	}
	m_flagReady = false ;
	return	sglErrSuccess ;
}

// ストリームバッファを準備する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWinCoreAudioRecorder::PrepareStream( size_t nBytes )
{
	return	m_flagReady ? sglErrSuccess : sglErrFailed ;
}

// ストリームバッファから読み出す
//////////////////////////////////////////////////////////////////////////////
size_t SGLWinCoreAudioRecorder::Read( void * ptrSound, size_t nBytes )
{
	if ( !m_flagReady )
	{
		return	0 ;
	}
	size_t	nOutSampleAlign =
				m_fmtOutput.channels * m_fmtOutput.bitsPerSample / 8 ;
	size_t	nInSampleAlign =
				m_fmtCapture.channels * m_fmtCapture.bitsPerSample / 8 ;
	if ( (nOutSampleAlign == 0) || (nInSampleAlign == 0) )
	{
		return	 0 ;
	}
	size_t	nOutSamples = nBytes / nOutSampleAlign ;
	size_t	nInSamples =
		(size_t) ((uint64_t) nOutSamples
						* m_fmtCapture.frequency / m_fmtOutput.frequency) ;
	size_t	nInBytes = nInSamples * nInSampleAlign ;
	//
	// 録音バッファから PCM 16bit 形式でサンプリング
	//
	m_csSync.Lock() ;
	//
	const uint8_t *	pbytInData = m_queRecorded.GetBuffer( nInBytes ) ;
	nInSamples = nInBytes / nInSampleAlign ;
//	nOutSamples =
//		(size_t) ((uint64_t) nInSamples
//						* m_fmtOutput.frequency / m_fmtCapture.frequency) ;
	uint64_t	nOutNextSamples = (m_nCapSamples + nInSamples)
									* m_fmtOutput.frequency / m_fmtCapture.frequency ;
	if ( (nInSamples == 0) || (nOutNextSamples <= m_nOutSamples) )
	{
		m_queRecorded.ReleaseBuffer( 0 ) ;
		m_csSync.Unlock() ;
		return	0 ;
	}
	nOutSamples = (size_t) (nOutNextSamples - m_nOutSamples) ;
	//
	size_t		nChannels = m_fmtOutput.channels ;
	int16_t *	pwPCM16 =
		m_bufConvertPCM16.GetArray( nInSamples * nChannels ) ;
	if ( m_fmtCapture.bitsPerSample == 24 )
	{
		for ( size_t ch = 0; ch < nChannels; ch ++ )
		{
			int16_t *		pwDst = pwPCM16 + ch ;
			const uint8_t *	pbytSrc = pbytInData + ch * 3 ;
			size_t			pitchSrc = m_fmtCapture.channels * 3 ;
			for ( size_t i = 0; i < nInSamples; i ++ )
			{
				*pwDst = (int16_t) (((int16_t) pbytSrc[2] << 8) | pbytSrc[1]) ;
				pwDst += nChannels ;
				pbytSrc += pitchSrc ;
			}
		}
	}
	else if ( m_fmtCapture.bitsPerSample == 32 )
	{
//		if ( m_fmtCapture.format == formatSoundIEEEFloat )
		{
			for ( size_t ch = 0; ch < nChannels; ch ++ )
			{
				int16_t *			pwDst = pwPCM16 + ch ;
				const float32_t *	pfpSrc = ((float32_t*) pbytInData) + ch ;
				size_t				pitchSrc = m_fmtCapture.channels ;
				for ( size_t i = 0; i < nInSamples; i ++ )
				{
					int32_t	v = eslRoundR32ToInt( *pfpSrc * 0x8000 ) ;
					if ( v < -0x8000 )
					{
						v = -0x8000 ;
					}
					else if ( v > 0x7FFF )
					{
						v = 0x7FFF ;
					}
					*pwDst = (int16_t) v ;
					pwDst += nChannels ;
					pfpSrc += pitchSrc ;
				}
			}
		}
		/*
		else
		{
			for ( size_t ch = 0; ch < nChannels; ch ++ )
			{
				int16_t *		pwDst = pwPCM16 + ch ;
				const int32_t *	pdwSrc = ((int32_t*) pbytInData) + ch ;
				size_t			pitchSrc = m_fmtCapture.channels ;
				for ( size_t i = 0; i < nInSamples; i ++ )
				{
					*pwDst = (int16_t) (*pdwSrc >> 8) ;
					pwDst += nChannels ;
					pdwSrc += pitchSrc ;
				}
			}
		}
		*/
	}
	else if ( m_fmtCapture.bitsPerSample == 16 )
	{
		for ( size_t ch = 0; ch < nChannels; ch ++ )
		{
			int16_t *		pwDst = pwPCM16 + ch ;
			const int16_t *	pwSrc = ((int16_t*) pbytInData) + ch ;
			size_t			pitchSrc = m_fmtCapture.channels ;
			for ( size_t i = 0; i < nInSamples; i ++ )
			{
				*pwDst = *pwSrc ;
				pwDst += nChannels ;
				pwSrc += pitchSrc ;
			}
		}
	}
	else if ( m_fmtCapture.bitsPerSample == 8 )
	{
		for ( size_t ch = 0; ch < nChannels; ch ++ )
		{
			int16_t *		pwDst = pwPCM16 + ch ;
			const uint8_t *	pbytSrc = pbytInData + ch ;
			size_t			pitchSrc = m_fmtCapture.channels ;
			for ( size_t i = 0; i < nInSamples; i ++ )
			{
				*pwDst = ((int16_t) *pbytSrc - 0x80) << 8 ;
				pwDst += nChannels ;
				pbytSrc += pitchSrc ;
			}
		}
	}
	m_bufConvertPCM16.FinishArray() ;
	/*
	size_t	nReleaseSamples =
		(size_t) ((uint64_t) nOutSamples
						* m_fmtCapture.frequency / m_fmtOutput.frequency) ;
	size_t	nReleaseBytes = nReleaseSamples * nInSampleAlign ;
	//
	m_queRecorded.ReleaseBuffer( (ssize_t) nReleaseBytes ) ;
	*/
	m_queRecorded.ReleaseBuffer( (ssize_t) (nInSamples * nInSampleAlign) ) ;
	//
	m_nCapSamples += nInSamples ;
	m_nOutSamples += nOutSamples ;
	//
	/*
	if ( m_lcmFreqConvert > 0 )
	{
		m_nCapSamples %= m_lcmFreqConvert ;
		m_nOutSamples %= m_lcmFreqConvert ;
	}
	*/
	//
	m_csSync.Unlock() ;
	//
	// 出力周波数に変換
	//
	int16_t *	pwFreqPCM =
			m_bufConvertFreq.GetArray( nOutSamples * nChannels ) ;
	//
	uint64_t	fxPitchFreq =
			((uint64_t) (nInSamples - 1) << 32) / nOutSamples ;
	for ( size_t ch = 0; ch < nChannels; ch ++ )
	{
		int16_t *	pwDst = pwFreqPCM + ch ;
		int16_t *	pwSrc = pwPCM16 + ch ;
		uint64_t	fxI = 0 ;
		for ( size_t i = 0; i < nOutSamples; i ++, fxI += fxPitchFreq )
		{
			size_t	j = (size_t) (fxI >> 32) ;
			size_t	t = (size_t) (fxI >> 24) & 0xFF ;
			ESLAssert( j + 1 < nInSamples ) ;
			j *= nChannels ;
			//
			int16_t	v0 = pwSrc[j] ;
			int16_t	v1 = pwSrc[j + nChannels] ;
			//
			*pwDst = (int16_t) (v0 + (((v1 - v0) * t) >> 8)) ;
			//
			pwDst += nChannels ;
		}
	}
	m_bufConvertFreq.FinishArray() ;
	//
	// 出力フォーマットに変換
	//
	if ( m_fmtOutput.bitsPerSample == 16 )
	{
		ESLAssert( nOutSamples * nOutSampleAlign <= nBytes ) ;
		eslMoveMemory
			( ptrSound, pwFreqPCM,
				nOutSamples * nOutSampleAlign ) ;
		return	nOutSamples * nOutSampleAlign ;
	}
	else if ( m_fmtOutput.bitsPerSample == 8 )
	{
		uint8_t *	pbytDst = (uint8_t*) ptrSound ;
		int16_t *	pwSrc = pwFreqPCM ;
		size_t		nAllSamples = nOutSamples * nChannels ;
		for ( size_t i = 0; i < nAllSamples; i ++ )
		{
			pbytDst[i] = (uint8_t) ((pwSrc[i] >> 8) + 0x80) ;
		}
		return	nOutSamples * nOutSampleAlign ;
	}
	else
	{
		return	0 ;
	}
}

// 録音を開始する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWinCoreAudioRecorder::Start( uint64_t nFlags )
{
	if ( !m_flagReady )
	{
		return	sglErrFailed ;
	}
	if ( m_flagRecording )
	{
		return	sglErrSuccess ;
	}
	if ( m_pAudioClient->Start() != S_OK )
	{
		return	sglErrFailed ;
	}
	m_flagStopRecord = false ;
	m_nCapSamples = 0 ;
	m_nOutSamples = 0 ;
	if ( m_thread.BeginThread( this ) )
	{
		m_pAudioClient->Stop() ;
		return	sglErrFailed ;
	}
	m_flagRecording = true ;
	return	sglErrSuccess ;
}

// 録音を停止する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWinCoreAudioRecorder::Stop( void )
{
	if ( m_flagRecording )
	{
		m_flagStopRecord = true ;
		m_thread.Wait() ;
		m_thread.Delete() ;
		//
		m_pAudioClient->Stop() ;
		m_flagRecording = false ;
	}
	return	sglErrSuccess ;
}

// 音量取得 [L/R]
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWinCoreAudioRecorder::GetVolume( float32_t* pVolumes, size_t nChannels )
{
	return	sglErrFailed ;
}

// 音量設定 [L/R]
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWinCoreAudioRecorder::SetVolume( const float32_t* pVolumes, size_t nChannels )
{
	return	sglErrFailed ;
}

// 録音中か？
//////////////////////////////////////////////////////////////////////////////
bool SGLWinCoreAudioRecorder::IsRecording( void ) const
{
	return	m_flagRecording ;
}

// ターゲットデバイス
//////////////////////////////////////////////////////////////////////////////
uint32_t SGLWinCoreAudioRecorder::GetTargetDevices( void ) const
{
	return	m_maskTargetDevices ;
}

void SGLWinCoreAudioRecorder::SetTargetDevices( uint32_t maskDev )
{
	m_maskTargetDevices = maskDev ;
}

// 対応しているか？
//////////////////////////////////////////////////////////////////////////////
bool SGLWinCoreAudioRecorder::IsSupported( void )
{
	UINT	nTotalDevs = 0 ;
	IMMDeviceEnumerator *	pEnum = NULL ;
	if ( ::CoCreateInstance
		( __uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL,
			__uuidof(IMMDeviceEnumerator), (void**) &pEnum ) == S_OK )
	{
		IMMDeviceCollection *	pEndCollect = NULL ;
		if ( pEnum->EnumAudioEndpoints
			( eCapture, DEVICE_STATE_ACTIVE, &pEndCollect ) == S_OK )
		{
			UINT	nCount = 0 ;
			pEndCollect->GetCount( &nCount ) ;
			pEndCollect->Release() ;
			nTotalDevs += nCount ;
		}
		if ( pEnum->EnumAudioEndpoints
			( eRender, DEVICE_STATE_ACTIVE, &pEndCollect ) == S_OK )
		{
			UINT	nCount = 0 ;
			pEndCollect->GetCount( &nCount ) ;
			pEndCollect->Release() ;
			nTotalDevs += nCount ;
		}
		//
		pEnum->Release() ;
	}
	return	(nTotalDevs > 0) ;
}

// 録音スレッド関数
//////////////////////////////////////////////////////////////////////////////
void SGLWinCoreAudioRecorder::Run( void )
{
	while ( !m_flagStopRecord )
	{
		if ( m_pCaptureClient != NULL )
		{
			UINT32	nPacketLen ;
			if ( m_pCaptureClient->GetNextPacketSize( &nPacketLen ) != S_OK )
			{
				SleepMilliSec( 1 ) ;
				continue ;
			}
			while ( !m_flagStopRecord && (nPacketLen != 0) )
			{
				LPBYTE	pData = NULL ;
				UINT32	nFrames = 0 ;
				DWORD	dwFlags = 0 ;
				if ( m_pCaptureClient->GetBuffer
					( &pData, &nFrames, &dwFlags, NULL, NULL ) != S_OK )
				{
					break ;
				}
				size_t	nBytes =
					nFrames * (m_fmtCapture.channels
								* m_fmtCapture.bitsPerSample >> 3) ;
				m_csSync.Lock() ;
				m_queRecorded.Write( pData, nBytes ) ;
				m_csSync.Unlock() ;
				m_pCaptureClient->ReleaseBuffer( nFrames ) ;
				//
				if ( m_pCaptureClient->GetNextPacketSize( &nPacketLen ) != S_OK )
				{
					break ;
				}
			}
		}
		SleepMilliSec( 1 ) ;
	}
}

