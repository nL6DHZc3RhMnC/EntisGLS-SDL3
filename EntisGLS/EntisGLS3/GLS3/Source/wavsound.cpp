
/*****************************************************************************
				Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
		Copyright (c) 1998-2010 Leshade Entis. All rights reserved.
 ****************************************************************************/


#include <gls.h>
#include <math.h>
#include <dsound.h>


/*****************************************************************************
						音声出力デバイスクラス
 ****************************************************************************/

#define	WAVEMSG_PLAYED	(WM_USER+1)
static void CALLBACK WaveOutputDevProc
	( HWAVEOUT, UINT, DWORD, DWORD, DWORD ) ;

// クラス情報実装
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( EWaveOutDevice, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
EWaveOutDevice::EWaveOutDevice( void )
	: m_pWaveFormat( NULL ), m_hWaveOut( NULL ),
		m_hThread( NULL ), m_hThreadCreated( NULL )
{
	m_nTotalVolume[0] = -1 ;
	m_nTotalVolume[1] = -1 ;

	// クリティカルセクション初期化
	::InitializeCriticalSection( &m_cs );
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
EWaveOutDevice::~EWaveOutDevice( void )
{
	// デバイスを開いている時は閉じる
	Close( ) ;

	// クリティカルセクションを削除する
	::DeleteCriticalSection( &m_cs ) ;
}

// 音声出力デバイスを開く
//////////////////////////////////////////////////////////////////////////////
ESLError EWaveOutDevice::Open( const WAVEFORMATEX * pwfx )
{
	// デバイスを開いている時は閉じる
	Close( ) ;

	// 音声出力フォーマットを複製
	unsigned int	nFmtBytes = sizeof(WAVEFORMATEX) ;
	if ( pwfx->wFormatTag != WAVE_FORMAT_PCM )
		nFmtBytes += pwfx->cbSize ;
	m_pWaveFormat =
		(WAVEFORMATEX*) ::eslHeapAllocate( NULL, nFmtBytes, 0 ) ;
	::memcpy( m_pWaveFormat, pwfx, nFmtBytes ) ;

	// コールバックスレッド作成
	ESLAssert( m_hThreadCreated == NULL ) ;
	m_hThreadCreated = ::CreateEvent( NULL, TRUE, FALSE, NULL ) ;
	m_hThread = ::CreateThread
		( NULL, 0, &WaveCallbackServiceThread, this, 0, &m_idThread ) ;
	if ( m_hThread != NULL )
	{
		::WaitForSingleObject( m_hThreadCreated, INFINITE ) ;
	}

	// 音声出力デバイスを開く
	static const DWORD	fdwFlags[4] =
	{
		CALLBACK_FUNCTION,
		CALLBACK_FUNCTION | WAVE_ALLOWSYNC,
		CALLBACK_NULL,
		CALLBACK_NULL | WAVE_ALLOWSYNC
	} ;
	MMRESULT	mmResult ;
	for ( int i = 0; true; i ++ )
	{
		m_hWaveOut = NULL ;
		mmResult = ::waveOutOpen
			( &m_hWaveOut, WAVE_MAPPER, m_pWaveFormat,
				(DWORD) &WaveOutputDevProc, m_idThread, fdwFlags[i] ) ;
		if ( mmResult != MMSYSERR_NOERROR )
		{
			ESLTrace( "音声出力デバイスを開けませんでした。(WAVE_MAPPER)\n" ) ;
			UINT	i, nDevCount = ::waveOutGetNumDevs( ) ;
			for ( i = 0; i < nDevCount; i ++ )
			{
				mmResult = ::waveOutOpen
					( &m_hWaveOut, i, m_pWaveFormat,
						(DWORD) &WaveOutputDevProc, m_idThread, fdwFlags[i] ) ;
				if ( mmResult == MMSYSERR_NOERROR )
				{
					break ;
				}
				ESLTrace( "音声出力デバイスを開けませんでした。(%d)\n", i ) ;
			}
			if ( mmResult != MMSYSERR_NOERROR )
			{
				::eslHeapFree( NULL, m_pWaveFormat ) ;
				m_pWaveFormat = NULL ;
				return	ESLErrorMsg( "音声出力デバイスを開けませんでした。" ) ;
			}
		}

		// 初期ボリュームを取得
		DWORD	dwDevVolume ;
		mmResult = ::waveOutGetVolume( m_hWaveOut, &dwDevVolume ) ;
		if ( mmResult == MMSYSERR_NOERROR )
		{
			m_nInitVolume[0] = dwDevVolume & 0xFFFF ;
			m_nInitVolume[1] = dwDevVolume >> 16 ;
			if ( m_nTotalVolume[0] == -1 )
			{
				m_nTotalVolume[0] = m_nInitVolume[0] ;
				m_nTotalVolume[1] = m_nInitVolume[1] ;
			}
			else
			{
				::waveOutSetVolume
					( m_hWaveOut,
						(m_nTotalVolume[0] | (m_nTotalVolume[1] << 16)) ) ;
			}
			break ;
		}
		else
		{
			ESLTrace( "waveOutGetVolume 関数が失敗しました。(%d)\n", mmResult ) ;
			ESLTrace( "m_hWaveOut = %08X\n", m_hWaveOut ) ;
		}
		if ( i + 1 >= (sizeof(fdwFlags)/sizeof(fdwFlags[0])) )
		{
			break ;
		}
		if ( ::waveOutClose( m_hWaveOut ) != MMSYSERR_NOERROR )
		{
			ESLTrace( "音声出力デバイスを閉じられませんでした。\n" ) ;
		}
		m_hWaveOut = NULL ;
	}
	m_fDevPaused = false ;

	return	eslErrSuccess ;
}

// 音声出力デバイスを閉じる
//////////////////////////////////////////////////////////////////////////////
void EWaveOutDevice::Close( void )
{
	if ( m_pWaveFormat != NULL )
	{
		Lock( ) ;

		// ペンディング中の音声データをリセット
		::waveOutReset( m_hWaveOut ) ;

		// 初期ボリュームを復帰
		::waveOutSetVolume
			( m_hWaveOut,
				(m_nInitVolume[0] | (m_nInitVolume[1] << 16)) ) ;

		// 全ての音声バッファを解放可能にする
		for ( int i = (m_arrayPlayBuf.GetSize() - 1); i >= 0; i -- )
		{
			ESLTrace( "警告： 解放されていない"
						"音声出力バッファがあります。(%08XH)\n",
						m_arrayPlayBuf.GetAt(i) ) ;
			UnprepareBuffer( m_arrayPlayBuf.GetAt(i) ) ;
		}

		// 音声出力デバイスを閉じる
		if ( ::waveOutClose( m_hWaveOut ) != MMSYSERR_NOERROR )
		{
			ESLTrace( "音声出力デバイスを閉じられませんでした。\n" ) ;
		}
		m_hWaveOut = NULL ;

		// 音声フォーマットを解放
		::eslHeapFree( NULL, m_pWaveFormat ) ;
		m_pWaveFormat = NULL ;

		Unlock( ) ;
	}
	if ( m_hThread != NULL )
	{
		// コールバックスレッドを終了する
		::PostThreadMessage( m_idThread, WM_QUIT, 0, 0 ) ;
		::WaitForSingleObject( m_hThread, 3000 ) ;
		::CloseHandle( m_hThread ) ;
		m_hThread = NULL ;
	}
	if ( m_hThreadCreated != NULL )
	{
		::CloseHandle( m_hThreadCreated ) ;
		m_hThreadCreated = NULL ;
	}
}

// 音声バッファの出力準備
//////////////////////////////////////////////////////////////////////////////
HWAVEBUF EWaveOutDevice::PrepareBuffer
	( EWaveStreamBuffer * pWaveBuffer,
		const void * ptrBuffer, unsigned int nBufferLength )
{
	if ( m_pWaveFormat == NULL )
	{
		ESLTrace( "音声出力デバイスが開かれていません。\n" ) ;
		return	NULL ;
	}

	WAVEHDR *	pWaveHdr = new WAVEHDR ;
	pWaveHdr->lpData = (LPSTR) ptrBuffer ;
	pWaveHdr->dwBufferLength = nBufferLength ;
	pWaveHdr->dwBytesRecorded = nBufferLength ;
	pWaveHdr->dwUser = (DWORD) pWaveBuffer ;
	pWaveHdr->dwFlags = 0 ;
	pWaveHdr->dwLoops = 0 ;
	pWaveHdr->lpNext = 0 ;
	pWaveHdr->reserved = 0 ;

	if ( ::waveOutPrepareHeader
		( m_hWaveOut, pWaveHdr, sizeof(WAVEHDR) ) != MMSYSERR_NOERROR )
	{
		ESLTrace( "waveOutPrepareHeader 関数が失敗しました。\n" );
	}

	Lock( ) ;
	m_arrayPlayBuf.Add( pWaveHdr ) ;
	Unlock( ) ;

	return	pWaveHdr ;
}

// 音声バッファの終了
//////////////////////////////////////////////////////////////////////////////
ESLError EWaveOutDevice::UnprepareBuffer( HWAVEBUF hWaveBuf )
{
	if ( m_pWaveFormat == NULL )
	{
		ESLTrace( "音声出力デバイスが開かれていません。\n" ) ;
		return	ESLErrorMsg( "音声出力デバイスが開かれていません。" ) ;
	}

	Lock( ) ;
	for ( unsigned int i = 0; i < m_arrayPlayBuf.GetSize(); i ++ )
	{
		if ( m_arrayPlayBuf.GetAt(i) == hWaveBuf )
		{
			// 音声バッファの使用終了
			if( ::waveOutUnprepareHeader
				( m_hWaveOut, hWaveBuf, sizeof(WAVEHDR) ) != MMSYSERR_NOERROR )
			{
				ESLTrace( "waveOutUnprepareHeader 関数が失敗しました。\n" ) ;
			}
			// WAVEHDR 構造体を削除
			m_arrayPlayBuf.RemoveAt( i ) ;
			delete	hWaveBuf ;
			Unlock( ) ;
			return	eslErrSuccess ;
		}
	}
	Unlock( ) ;

	ESLTrace( "不正な音声バッファハンドルが指定されました。\n" ) ;
	return	ESLErrorMsg( "不正な音声バッファハンドルが指定されました。" ) ;
}

// 音声バッファを出力
//////////////////////////////////////////////////////////////////////////////
ESLError EWaveOutDevice::Play
	( EWaveStreamBuffer * pWaveBuffer, HWAVEBUF hWaveBuf )
{
	if ( m_pWaveFormat == NULL )
	{
		ESLTrace( "音声出力デバイスが開かれていません。\n" ) ;
		return	ESLErrorMsg( "音声出力デバイスが開かれていません。" ) ;
	}
	if ( hWaveBuf == NULL )
	{
		ESLTrace( "不正な音声バッファハンドルが指定されました。\n" ) ;
		return	ESLErrorMsg( "不正な音声バッファハンドルが指定されました。" ) ;
	}

	Lock( ) ;

	// 先頭のバッファを出力
	hWaveBuf->dwUser = (DWORD) pWaveBuffer ;
	if ( ::waveOutWrite
		( m_hWaveOut, hWaveBuf, sizeof(WAVEHDR) ) != MMSYSERR_NOERROR )
	{
		ESLTrace( "waveOutWrite 関数が失敗しました。\n" ) ;
	}

	// 次のバッファを出力
	HWAVEBUF	hNextBuf = pWaveBuffer->OnQueueNextBuffer( this ) ;
	if ( hNextBuf != NULL )
	{
		hNextBuf->dwUser = (DWORD) pWaveBuffer ;
		if ( ::waveOutWrite
			( m_hWaveOut, hNextBuf, sizeof(WAVEHDR) ) != MMSYSERR_NOERROR )
		{
			ESLTrace( "waveOutWrite 関数が失敗しました。\n" ) ;
		}
	}

	Unlock( ) ;

	return	eslErrSuccess ;
}

// 音声出力停止
//////////////////////////////////////////////////////////////////////////////
ESLError EWaveOutDevice::Stop( EWaveStreamBuffer * pWaveBuffer )
{
	if ( m_pWaveFormat == NULL )
	{
		ESLTrace( "音声出力デバイスが開かれていません。\n" ) ;
		return	ESLErrorMsg( "音声出力デバイスが開かれていません。" ) ;
	}

	// 音声出力をリセット
	if ( ::waveOutReset( m_hWaveOut ) != MMSYSERR_NOERROR )
	{
		ESLTrace( "waveOutReset 関数が失敗しました。\n" ) ;
	}

	return	eslErrSuccess ;
}

// 音声出力を一時停止
//////////////////////////////////////////////////////////////////////////////
ESLError EWaveOutDevice::Pause( EWaveStreamBuffer * pWaveBuffer )
{
	pWaveBuffer->m_fPauseFlag = 1 ;
	return	PauseDevice( ) ;
}

// 音声出力を再開
//////////////////////////////////////////////////////////////////////////////
ESLError EWaveOutDevice::Restart( EWaveStreamBuffer * pWaveBuffer )
{
	pWaveBuffer->m_fPauseFlag = 0 ;
	return	RestartDevice( ) ;
}

// 音声出力デバイスの出力一時停止
//////////////////////////////////////////////////////////////////////////////
ESLError EWaveOutDevice::PauseDevice( void )
{
	if ( m_pWaveFormat == NULL )
	{
		ESLTrace( "音声出力デバイスが開かれていません。\n" ) ;
		return	ESLErrorMsg( "音声出力デバイスが開かれていません。" ) ;
	}
	if ( ::waveOutPause( m_hWaveOut ) != MMSYSERR_NOERROR )
	{
		ESLTrace( "waveOutPause 関数が失敗しました。\n" ) ;
		return	eslErrGeneral ;
	}
	m_fDevPaused = true ;
	return	eslErrSuccess ;
}

// 音声出力デバイス出力再開
//////////////////////////////////////////////////////////////////////////////
ESLError EWaveOutDevice::RestartDevice( void )
{
	if ( m_pWaveFormat == NULL )
	{
		ESLTrace( "音声出力デバイスが開かれていません。\n" ) ;
		return	ESLErrorMsg( "音声出力デバイスが開かれていません。" ) ;
	}
	if ( m_fDevPaused )
	{
		if ( ::waveOutRestart( m_hWaveOut ) != MMSYSERR_NOERROR )
		{
			ESLTrace( "waveOutRestart 関数が失敗しました。\n" ) ;
			return	eslErrGeneral ;
		}
		m_fDevPaused = false ;
	}
	return	eslErrSuccess ;
}

// 現在の再生位置を取得
//////////////////////////////////////////////////////////////////////////////
UINT64 EWaveOutDevice::GetCurrentSample
				( const EWaveStreamBuffer * pWaveBuffer )
{
	if ( m_pWaveFormat == NULL )
	{
		ESLTrace( "音声出力デバイスが開かれていません。\n" ) ;
		return	0 ;
	}

	MMTIME	mmtime ;
	mmtime.wType = TIME_SAMPLES ;
	if ( ::waveOutGetPosition
		( m_hWaveOut, &mmtime, sizeof(MMTIME) ) != MMSYSERR_NOERROR )
	{
		ESLTrace( "waveOutGetPosition 関数が失敗しました。\n" ) ;
		return	0 ;
	}

	return	mmtime.u.sample ;
}

// トータル出力ボリュームを取得
//////////////////////////////////////////////////////////////////////////////
ESLError EWaveOutDevice::GetTotalVolume( unsigned int nVolume[] ) const
{
	nVolume[0] = m_nTotalVolume[0] ;
	nVolume[1] = m_nTotalVolume[1] ;
	return	eslErrSuccess ;
}

// トータル出力ボリュームを設定
//////////////////////////////////////////////////////////////////////////////
ESLError EWaveOutDevice::SetTotalVolume( const unsigned int nVolume[] )
{
	Lock( ) ;
	m_nTotalVolume[0] = nVolume[0] ;
	m_nTotalVolume[1] = nVolume[1] ;
	if ( m_pWaveFormat != NULL )
	{
		WAVEHDR *	pWaveHdr = (WAVEHDR*) m_arrayPlayBuf.GetAt( 0 ) ;
		if ( (pWaveHdr != NULL) && (pWaveHdr->dwUser != NULL) )
		{
			EWaveStreamBuffer *	pWaveBuffer =
				(EWaveStreamBuffer*) (pWaveHdr->dwUser) ;
			SetVolume( pWaveBuffer, pWaveBuffer->m_realVolume ) ;
		}
		else
		{
			if ( m_nTotalVolume[0] > 0xFFFF )
				m_nTotalVolume[0] = 0xFFFF ;
			if ( m_nTotalVolume[1] > 0xFFFF )
				m_nTotalVolume[1] = 0xFFFF ;
			if ( ::waveOutSetVolume
				( m_hWaveOut, (m_nTotalVolume[0]
						| (m_nTotalVolume[1] < 16)) ) != MMSYSERR_NOERROR )
			{
				ESLTrace( "waveOutSetVolume 関数が失敗しました。\n" ) ;
			}
		}
	}
	Unlock( ) ;
	return	eslErrSuccess ;
}

// ボリュームを取得
//////////////////////////////////////////////////////////////////////////////
ESLError EWaveOutDevice::GetVolume
	( EWaveStreamBuffer * pWaveBuffer, REAL32 nVolume[] ) const
{
	nVolume[0] = pWaveBuffer->m_realVolume[0] ;
	nVolume[1] = pWaveBuffer->m_realVolume[1] ;
	return	eslErrSuccess ;
}

// ボリュームを設定
//////////////////////////////////////////////////////////////////////////////
ESLError EWaveOutDevice::SetVolume
	( EWaveStreamBuffer * pWaveBuffer, const REAL32 nVolume[] )
{
	Lock( ) ;
	pWaveBuffer->m_realVolume[0] = nVolume[0] ;
	pWaveBuffer->m_realVolume[1] = nVolume[1] ;
	if ( m_pWaveFormat != NULL )
	{
		int	nLeftVol = (int) (nVolume[0] * m_nTotalVolume[0]),
			nRightVol = (int) (nVolume[1] * m_nTotalVolume[1]) ;
		if ( nLeftVol < 0 )
			nLeftVol = - nLeftVol ;
		if ( nRightVol < 0 )
			nRightVol = - nRightVol ;
		if ( nLeftVol > 0xFFFF )
			nLeftVol = 0xFFFF ;
		if ( nRightVol > 0xFFFF )
			nRightVol = 0xFFFF ;
		if ( ::waveOutSetVolume
			( m_hWaveOut, (nLeftVol | (nRightVol << 16)) ) != MMSYSERR_NOERROR )
		{
			ESLTrace( "waveOutSetVolume 関数が失敗しました。\n" ) ;
		}
	}
	Unlock( ) ;
	return	eslErrSuccess ;
}

// 音声出力デバイスの総数を取得
//////////////////////////////////////////////////////////////////////////////
unsigned int EWaveOutDevice::GetWaveInstalled( void )
{
	return	::waveOutGetNumDevs( ) ;
}

// スレッド排他アクセス
//////////////////////////////////////////////////////////////////////////////
void EWaveOutDevice::Lock( void ) const
{
	::EnterCriticalSection( (LPCRITICAL_SECTION) &m_cs ) ;
}

void EWaveOutDevice::Unlock( void ) const
{
	::LeaveCriticalSection( (LPCRITICAL_SECTION) &m_cs ) ;
}

// 音声バッファの再生が終了した
//////////////////////////////////////////////////////////////////////////////
void EWaveOutDevice::OnBufferPlayed( HWAVEBUF hWaveBuf )
{
	if ( hWaveBuf != NULL )
	{
		Lock( ) ;
		for ( int i = (m_arrayPlayBuf.GetSize() - 1); i >= 0; i -- )
		{
			if ( m_arrayPlayBuf.GetAt(i) == hWaveBuf )
			{
				EWaveStreamBuffer *	pWaveBuffer =
						(EWaveStreamBuffer*) (hWaveBuf->dwUser) ;
				if ( pWaveBuffer != NULL )
				{
					HWAVEBUF	hNextBuf =
						pWaveBuffer->OnQueueNextBuffer( this ) ;
					if ( hNextBuf != NULL )
					{
						hNextBuf->dwUser = (DWORD) pWaveBuffer ;
						if ( ::waveOutWrite
							( m_hWaveOut, hNextBuf, sizeof(WAVEHDR) ) != MMSYSERR_NOERROR )
						{
							ESLTrace( "waveOutWrite 関数が失敗しました。\n" ) ;
						}
					}
					pWaveBuffer->OnEndPlaying
						( hWaveBuf, hWaveBuf->lpData, hWaveBuf->dwBufferLength ) ;
				}
				break ;
			}
		}
		Unlock( ) ;
	}
}

// コールバック関数
//////////////////////////////////////////////////////////////////////////////
void CALLBACK WaveOutputDevProc
	( HWAVEOUT hWaveOut, UINT uMsg,
		DWORD dwInstance, DWORD dwParam1, DWORD dwParam2 )
{
	if ( uMsg == WOM_DONE )
	{
		::PostThreadMessage
			( dwInstance, WAVEMSG_PLAYED,
				(WPARAM)dwParam1, (LPARAM)dwParam2 ) ;
	}
}

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
DWORD WINAPI EWaveOutDevice::WaveCallbackServiceThread( LPVOID lpParam )
{
	EWaveOutDevice *	pWaveDev = (EWaveOutDevice*) lpParam ;
	//
	::glsInitializeTask( ) ;
	//
	MSG		msg ;
	::PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) ;
	ESLAssert( pWaveDev->m_hThreadCreated != NULL ) ;
	::SetEvent( pWaveDev->m_hThreadCreated ) ;
	//
	DWORD	dwResult ;
	dwResult = pWaveDev->WaveCallbackThread( ) ;
	//
	::glsCloseTask( ) ;
	//
	return	dwResult ;
}

DWORD EWaveOutDevice::WaveCallbackThread( void )
{
	MSG		msg ;
	while ( ::GetMessage( &msg, NULL, 0, 0 ) )
	{
		if ( (msg.hwnd == NULL) && (msg.message == WM_QUIT) )
		{
			break ;
		}
		if ( (msg.hwnd == NULL) && (msg.message == WAVEMSG_PLAYED) )
		{
			OnBufferPlayed( (HWAVEBUF) msg.wParam ) ;
		}
	}

	return	0 ;
}


/*****************************************************************************
					音声ミキシングチャネルオブジェクト
 ****************************************************************************/

static const GUID GLS_CLSID_DirectSound =
{ 0x47d4d946, 0x62e8, 0x11cf, { 0x93, 0xbc, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0 } } ;
static const GUID GLS_IID_IDirectSound =
{ 0x279AFA83, 0x4981, 0x11CE, { 0xA5, 0x21, 0x00, 0x20, 0xAF, 0x0B, 0xE5, 0x60 } } ;
static const GUID GLS_IID_IDirectSoundBuffer =
{ 0x279AFA85, 0x4981, 0x11CE, { 0xA5, 0x21, 0x00, 0x20, 0xAF, 0x0B, 0xE5, 0x60 } } ;
static const GUID GLS_IID_IDirectSoundNotify =
{ 0xb0210783, 0x89cd, 0x11d0, { 0xaf, 0x8, 0x0, 0xa0, 0xc9, 0x25, 0xcd, 0x16 } } ;

// 構築関数
//////////////////////////////////////////////////////////////////////////////
WAVE_MIXER_CHANNEL::WAVE_MIXER_CHANNEL( void )
	: m_pWaveBufObj( NULL ), m_nPauseFlag( 1 ),
		m_nPausedPosition( 0 ), m_pNextWaveHdr( NULL ),
		m_pSrcFormat( NULL ), m_idsbuf( NULL )
{
	m_pWaveBuffer[0] = NULL ;
	m_pWaveBuffer[1] = NULL ;
	m_nWaveBufLen[0] = 0 ;
	m_nWaveBufLen[1] = 0 ;
	for ( int i = 0; i < BUFFER_COUNT; i ++ )
	{
		m_hDirectSoundNotify[i] = NULL ;
	}
	m_nBufferSwitch = 0 ;
	m_nPlayPosBias = 0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
WAVE_MIXER_CHANNEL::~WAVE_MIXER_CHANNEL( void )
{
	// バッファ解放
	if ( m_pWaveBuffer[0] != NULL )
	{
		::eslHeapFree( NULL, m_pWaveBuffer[0] ) ;
	}
	if ( m_pWaveBuffer[1] != NULL )
	{
		::eslHeapFree( NULL, m_pWaveBuffer[1] ) ;
	}

	// ACM を閉じる
	if ( m_pSrcFormat != NULL )
	{
		if ( ::acmStreamClose( m_hACMStream, 0 ) )
		{
			ESLTrace( "acmStreamClose 関数が失敗しました。\n" );
		}
	}

	// DirectSound をリリースする
	if ( m_idsbuf != NULL )
	{
		m_idsbuf->Stop( ) ;
		m_idsbuf->Release( ) ;
	}
	for ( int i = 0; i < BUFFER_COUNT; i ++ )
	{
		if ( m_hDirectSoundNotify[i] != NULL )
		{
			::CloseHandle( m_hDirectSoundNotify[i] ) ;
		}
	}
}

// ミキサーチャネルを開く
//////////////////////////////////////////////////////////////////////////////
ESLError WAVE_MIXER_CHANNEL::Open
( EWaveMixingServer * pServer,
	EWaveStreamBuffer * pBufObj, WAVEHDR * pFirstBuffer )
{
	// 出力バッファを確保する
	//////////////////////////////////////////////////////////////////////////
	unsigned int	nBufferBytes =
		pServer->m_pWaveFormat->nBlockAlign * pServer->m_nBufferingSize ;
	m_pWaveBuffer[0] = ::eslHeapAllocate( NULL, nBufferBytes, 0 ) ;
	m_pWaveBuffer[1] = ::eslHeapAllocate( NULL, nBufferBytes, 0 ) ;
	::glsSound_CleanSoundBuffer
		( pServer->m_pWaveFormat,
			m_pWaveBuffer[0], pServer->m_nBufferingSize ) ;
	::glsSound_CleanSoundBuffer
		( pServer->m_pWaveFormat,
			m_pWaveBuffer[1], pServer->m_nBufferingSize ) ;

	// 変数初期化
	//////////////////////////////////////////////////////////////////////////
	m_pMixingServer = pServer ;
	m_pWaveBufObj = pBufObj ;
	m_nPauseFlag = pBufObj->m_fPauseFlag ;
	m_nWaveBufLen[0] = 0 ;
	m_nWaveBufLen[1] = 0 ;
	m_nPausedPosition = 0 ;
	m_nOffsetSrcBuf = 0 ;
	m_pNextWaveHdr = pFirstBuffer ;
	m_nOutputCounter = 0 ;
	m_pSrcFormat = NULL ;
	m_nBufferSwitch = 0 ;
	m_nPlayPosBias = 0 ;

	// ACM による変換を利用するか？
	//////////////////////////////////////////////////////////////////////////
	if ( pBufObj->m_pWaveFormat == NULL )
	{
		return	ESLErrorMsg( "無効な音声ストリームバッファを指定します。" ) ;
	}
	m_WaveFormat = *(pBufObj->m_pWaveFormat) ;
	if ( m_WaveFormat.wFormatTag != WAVE_FORMAT_PCM )
	{
		// ACM ストリームを開く
		//////////////////////////////////////////////////////////////////////
		//
		// PCM フォーマットを設定する
		m_pSrcFormat = pBufObj->m_pWaveFormat ;
		m_WaveFormat.wFormatTag = WAVE_FORMAT_PCM ;
		m_WaveFormat.wBitsPerSample = 16 ;
		m_WaveFormat.nBlockAlign = (m_WaveFormat.nChannels << 1) ;
		m_WaveFormat.nAvgBytesPerSec =
			m_WaveFormat.nSamplesPerSec * m_WaveFormat.nBlockAlign ;
		m_WaveFormat.cbSize = 0 ;
		//
		if ( ::acmStreamOpen
			( &m_hACMStream, NULL,
				m_pSrcFormat, &m_WaveFormat,
				NULL, 0, 0, ACM_STREAMOPENF_NONREALTIME ) )
		{
			ESLTrace( "ACM ストリームを開けませんでした。\n" ) ;
			return	ESLErrorMsg( "ACM ストリームを開けませんでした。" ) ;
		}
	}

	// DirectSound を利用するか？
	//////////////////////////////////////////////////////////////////////////
	if ( pServer->m_idsound != NULL )
	{
		DSBUFFERDESC	dsbd ;
		WAVEFORMATEX	wfx = *(pServer->m_pWaveFormat) ;
		::eslFillMemory( &dsbd, 0, sizeof(dsbd) ) ;
		dsbd.dwSize = sizeof(dsbd) ;
		dsbd.dwFlags =
			DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME
				| DSBCAPS_GLOBALFOCUS
				| DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GETCURRENTPOSITION2 ;
		dsbd.dwBufferBytes = nBufferBytes * BUFFER_COUNT ;
		dsbd.lpwfxFormat = &wfx ;
		//
		ESLAssert( m_idsbuf == NULL ) ;
		if ( pServer->m_idsound->
				CreateSoundBuffer( &dsbd, &m_idsbuf, NULL ) == DS_OK )
		{
			ESLAssert( m_idsbuf != NULL ) ;
		}
		else
		{
			m_idsbuf = NULL ;
			ESLTrace( "DirectSoundBuffer の生成に失敗しました。\n" ) ;
			return	ESLErrorMsg( "DirectSoundBuffer の生成に失敗しました。" ) ;
		}
		//
/*		IDirectSoundNotify *	idsn = NULL ;
		HRESULT	hrs ;
		if ( SUCCEEDED( hrs = m_idsbuf->QueryInterface
				( GLS_IID_IDirectSoundNotify, (void**) &idsn ) ) )
		{
			DSBPOSITIONNOTIFY	dsbpn[BUFFER_COUNT] ;
			for ( int i = 0; i < BUFFER_COUNT; i ++ )
			{
				m_hDirectSoundNotify[i] =
					::CreateEvent( NULL, TRUE, FALSE, NULL ) ;
				dsbpn[i].dwOffset = nBufferBytes * (i + 1) - 1 ;
				dsbpn[i].hEventNotify = m_hDirectSoundNotify[i] ;
			}
			if ( SUCCEEDED( idsn->SetNotificationPositions( BUFFER_COUNT, dsbpn ) ) )
			{
			}
			else
			{
				ESLTrace( "SetNotificationPositions に失敗しました。\n" ) ;
				return	ESLErrorMsg( "SetNotificationPositions に失敗しました。" ) ;
			}
			idsn->Release( ) ;
		}
		else
		{
			ESLTrace( "IDirectSoundNotify の生成に失敗しました。\n" ) ;
			return	ESLErrorMsg( "IDirectSoundNotify の生成に失敗しました。" ) ;
		}
*/	}

	return	eslErrSuccess ;
}

// 再利用のため初期化する
//////////////////////////////////////////////////////////////////////////////
ESLError WAVE_MIXER_CHANNEL::ReInitialize
	( EWaveMixingServer * pServer,
		EWaveStreamBuffer * pBufObj, WAVEHDR * pFirstBuffer )
{
	//
	// 出力バッファを初期化
	//
	if ( (m_pWaveBuffer[0] == NULL) || (m_pWaveBuffer[1] == NULL) )
	{
		return	eslErrGeneral ;
	}
	unsigned int	nBufferBytes =
		pServer->m_pWaveFormat->nBlockAlign * pServer->m_nBufferingSize ;
	if ( (::eslHeapGetLength( NULL, m_pWaveBuffer[0] ) < nBufferBytes)
		|| (::eslHeapGetLength( NULL, m_pWaveBuffer[0] ) < nBufferBytes) )
	{
		return	eslErrGeneral ;
	}
	::glsSound_CleanSoundBuffer
		( pServer->m_pWaveFormat,
			m_pWaveBuffer[0], pServer->m_nBufferingSize ) ;
	::glsSound_CleanSoundBuffer
		( pServer->m_pWaveFormat,
			m_pWaveBuffer[1], pServer->m_nBufferingSize ) ;

	//
	// 変数初期化
	//
	if ( (m_pMixingServer != pServer)
		|| (m_pWaveBufObj != pBufObj) )
	{
		return	eslErrGeneral ;
	}
	m_pMixingServer = pServer ;
	m_pWaveBufObj = pBufObj ;
	m_nPauseFlag = pBufObj->m_fPauseFlag ;
	m_nWaveBufLen[0] = 0 ;
	m_nWaveBufLen[1] = 0 ;
	m_nPausedPosition = 0 ;
	m_nOffsetSrcBuf = 0 ;
	m_pNextWaveHdr = pFirstBuffer ;
	m_nOutputCounter = 0 ;
	m_pSrcFormat = NULL ;
	m_nBufferSwitch = 0 ;
	m_nPlayPosBias = 0 ;

	//
	// 入力フォーマットチェック
	//
	if ( pBufObj->m_pWaveFormat == NULL )
	{
		return	eslErrGeneral ;
	}
	const WAVEFORMATEX *	pwfxBuf = pBufObj->m_pWaveFormat ;
	if ( (m_WaveFormat.nChannels != pwfxBuf->nChannels)
		|| (m_WaveFormat.nSamplesPerSec != pwfxBuf->nSamplesPerSec) )
	{
		return	eslErrGeneral ;
	}
	if ( pwfxBuf->wFormatTag != WAVE_FORMAT_PCM )
	{
		m_pSrcFormat = pBufObj->m_pWaveFormat ;
		m_WaveFormat.wFormatTag = WAVE_FORMAT_PCM ;
		m_WaveFormat.wBitsPerSample = 16 ;
		m_WaveFormat.nBlockAlign = (m_WaveFormat.nChannels << 1) ;
		m_WaveFormat.nAvgBytesPerSec =
			m_WaveFormat.nSamplesPerSec * m_WaveFormat.nBlockAlign ;
		m_WaveFormat.cbSize = 0 ;
		//
		if ( ::acmStreamOpen
			( &m_hACMStream, NULL,
				m_pSrcFormat, &m_WaveFormat,
				NULL, 0, 0, ACM_STREAMOPENF_NONREALTIME ) )
		{
			ESLTrace( "ACM ストリームを開けませんでした。\n" ) ;
			return	ESLErrorMsg( "ACM ストリームを開けませんでした。" ) ;
		}
	}
	else
	{
		if ( m_WaveFormat.wBitsPerSample != pwfxBuf->wBitsPerSample )
		{
			return	eslErrGeneral ;
		}
		if ( (pServer->m_idsound != NULL) && (m_idsbuf == NULL) )
		{
			return	eslErrGeneral ;
		}
		if ( m_idsbuf != NULL )
		{
			DWORD	dwStatus ;
			if ( m_idsbuf->GetStatus( &dwStatus ) == DS_OK )
			{
				if ( dwStatus & DSBSTATUS_BUFFERLOST )
				{
					if ( m_idsbuf->Restore() != DS_OK )
					{
						return	eslErrGeneral ;
					}
				}
			}
			m_idsbuf->Stop( ) ;
		}
	}
	return	eslErrSuccess ;
}

// ミキサーを停止する（再利用のため）
//////////////////////////////////////////////////////////////////////////////
ESLError WAVE_MIXER_CHANNEL::Stop( void )
{
	//
	// ACM を閉じる
	//
	if ( m_pSrcFormat != NULL )
	{
		if ( ::acmStreamClose( m_hACMStream, 0 ) )
		{
			ESLTrace( "acmStreamClose 関数が失敗しました。\n" );
		}
		m_pSrcFormat = NULL ;
	}
	//
	// DirectSound 停止
	//
	if ( m_idsbuf != NULL )
	{
		m_idsbuf->Stop( ) ;
	}
	return	eslErrSuccess ;
}

// 次の PCM 音声サンプルを取得する
//////////////////////////////////////////////////////////////////////////////
void * WAVE_MIXER_CHANNEL::GetNextWaveSamples
( unsigned int nBufIndex, unsigned int nOffsetSamples,
		unsigned int & nLenSamples, const WAVEFORMATEX * pMixingFormat )
{
	// 出力情報
	BYTE *			lpNextAddr ;
	unsigned int	nSrcBkAlign = m_WaveFormat.nBlockAlign ;
	unsigned int	nDstBkAlign = pMixingFormat->nBlockAlign ;
	unsigned int	nRestBytes = nLenSamples * nDstBkAlign ;
	unsigned int	nOutBytes = 0 ;

	if ( nBufIndex == -1 )
	{
		// 一時停止バッファに出力する
		//////////////////////////////////////////////////////////////////////
/*		unsigned int	nPausedBufLen = m_PausedBuffer.GetLength( ) ;
		if ( nPausedBufLen >= nRestBytes )	return	NULL ;
		unsigned int	nRestSamples =
			(nRestBytes - nPausedBufLen) / nDstBkAlign ;
		lpNextAddr = (BYTE*)
			m_PausedBuffer.PutBuffer( nRestSamples * nDstBkAlign ) ;*/
		lpNextAddr = (BYTE*)
			m_PausedBuffer.PutBuffer( nRestBytes ) ;
	}
	else
	{
		// ミキシングバッファに出力する
		//////////////////////////////////////////////////////////////////////
		lpNextAddr = ((BYTE*)m_pWaveBuffer[nBufIndex])
								+ nOffsetSamples * nDstBkAlign ;
		unsigned int	nPausedBufLen = m_PausedBuffer.GetLength( ) ;
		if ( nPausedBufLen > 0 )
		{
			// 一時停止バッファからコピーする
			//////////////////////////////////////////////////////////////////
			if ( nPausedBufLen >= nRestBytes )
			{
				m_PausedBuffer.Read( lpNextAddr, nRestBytes ) ;
				m_nOutputCounter += nLenSamples ;
				m_nWaveBufLen[nBufIndex] = nOffsetSamples + nLenSamples ;
				return	lpNextAddr ;
			}
			else
			{
				m_PausedBuffer.Read( lpNextAddr, nPausedBufLen ) ;
				lpNextAddr += nPausedBufLen ;
				nRestBytes -= nPausedBufLen ;
				nOutBytes += nPausedBufLen ;
			}
		}
	}

	// 展開ループ
	//////////////////////////////////////////////////////////////////////////
	while ( (nRestBytes > 0) && (m_pNextWaveHdr != NULL) )
	{
		// 現在のループの初期化を行う
		//////////////////////////////////////////////////////////////////////
		unsigned int	nRestSamples = nRestBytes / nDstBkAlign ;
		unsigned int	nDstSamples = nRestSamples ;
		unsigned int	nNeededSrcPCM =
			::glsSound_GetConversionSize
				( pMixingFormat, &m_WaveFormat, nRestSamples, 0 ) ;
		unsigned int	nNeededSrcBytes = nNeededSrcPCM * nSrcBkAlign ;

		if ( m_pSrcFormat == NULL )
		{
			// ACM を用いない変換
			//////////////////////////////////////////////////////////////////
			//
			// 入出力サンプル数の正規化
			if ( (m_nOffsetSrcBuf + nNeededSrcBytes)
					> m_pNextWaveHdr->dwBufferLength )
			{
				nNeededSrcBytes =
					m_pNextWaveHdr->dwBufferLength - m_nOffsetSrcBuf ;
				nNeededSrcPCM = nNeededSrcBytes / nSrcBkAlign ;
				nDstSamples = ::glsSound_GetConversionSize
					( pMixingFormat, &m_WaveFormat, nNeededSrcPCM, 1 ) ;
				//
				if ( (nDstSamples != 0) && (nDstSamples != nNeededSrcPCM) )
				{
					nDstSamples -- ;
				}
			}
			//
			// PCM サンプルを変換する
			::glsSound_ConvertPCMFormat
				( pMixingFormat, lpNextAddr, &m_WaveFormat,
					(m_pNextWaveHdr->lpData + m_nOffsetSrcBuf), nDstSamples ) ;
			//
			// 次へ進む
			unsigned int	nDstBytes = nDstSamples * nDstBkAlign ;
			m_nOffsetSrcBuf += nNeededSrcBytes ;
			lpNextAddr += nDstBytes ;
			nRestBytes -= nDstBytes ;
			nOutBytes += nDstBytes ;
		}
		else
		{
			// ACM で復号
			//////////////////////////////////////////////////////////////////
			//
			// ACM 出力バッファから変換する
			if ( m_ACMDstBuffer.GetLength() > 0 )
			{
				//
				// 入出力サンプル数を正規化する
				if ( nNeededSrcBytes > m_ACMDstBuffer.GetLength() )
				{
					nNeededSrcBytes = m_ACMDstBuffer.GetLength() ;
					nNeededSrcPCM = nNeededSrcBytes / nSrcBkAlign ;
					nDstSamples = ::glsSound_GetConversionSize
						( pMixingFormat, &m_WaveFormat, nNeededSrcPCM, 1 ) ;
				}
				//
				// PCM サンプルを変換する
				::glsSound_ConvertPCMFormat
					( pMixingFormat, lpNextAddr, &m_WaveFormat,
						m_ACMDstBuffer.GetBuffer( nNeededSrcBytes ), nDstSamples ) ;
				m_ACMDstBuffer.Release( nNeededSrcBytes ) ;
				//
				// 次へ進む
				unsigned int	nDstBytes = nDstSamples * nDstBkAlign ;
				lpNextAddr += nDstBytes ;
				nRestBytes -= nDstBytes ;
				nOutBytes += nDstBytes ;
				if ( nRestBytes == 0 )	break ;
			}

			//
			// 必要な ACM 出力バッファのサイズを計算する
			nRestSamples = nRestBytes / nDstBkAlign ;
			unsigned int	nNeededDstPCM =
				::glsSound_GetConversionSize
					( pMixingFormat, &m_WaveFormat, nRestSamples, 0 ) ;
			unsigned int	nNeededDstBytes = nNeededSrcPCM * nSrcBkAlign ;
			if ( nNeededDstBytes == 0 )	break ;
			//
			// 必要な ACM 入力バッファのサイズを計算する
			DWORD	dwNecessaryBytes = 0 ;
			for ( ; ; )
			{
				::acmStreamSize( m_hACMStream, nNeededDstBytes,
					&dwNecessaryBytes, ACM_STREAMSIZEF_DESTINATION ) ;
				if ( dwNecessaryBytes > 0 )	break ;
				nNeededDstBytes <<= 1 ;
			}
			//
			// 変換フラグを初期化する
			DWORD	fdwConversion ;
			if ( m_nOffsetSrcBuf == 0 )
				fdwConversion = ACM_STREAMCONVERTF_START ;
			else
				fdwConversion = ACM_STREAMCONVERTF_BLOCKALIGN ;
			//
			// ACM 入力バッファに入力音声バッファをコピーする
			if ( dwNecessaryBytes > m_ACMSrcBuffer.GetLength() )
			{
				dwNecessaryBytes -= m_ACMSrcBuffer.GetLength() ;
			}
			if ( dwNecessaryBytes >
				(m_pNextWaveHdr->dwBufferLength + m_nOffsetSrcBuf) )
			{
				dwNecessaryBytes =
					m_pNextWaveHdr->dwBufferLength - m_nOffsetSrcBuf ;
			}
			//
			m_ACMSrcBuffer.Write
				( m_pNextWaveHdr->lpData + m_nOffsetSrcBuf, dwNecessaryBytes ) ;
			m_nOffsetSrcBuf += dwNecessaryBytes ;
			//
			if ( m_nOffsetSrcBuf == m_pNextWaveHdr->dwBufferLength )
			{
				fdwConversion =
					ACM_STREAMCONVERTF_START | ACM_STREAMCONVERTF_END ;
			}
			//
			// 必要な ACM 出力バッファのサイズを計算する
			DWORD	dwDestinationBytes = 0 ;
			dwNecessaryBytes = m_ACMSrcBuffer.GetLength() ;
			if ( dwNecessaryBytes == 0 )	break ;
			for ( ; ; )
			{
				::acmStreamSize
					( m_hACMStream, dwNecessaryBytes,
						&dwDestinationBytes, ACM_STREAMSIZEF_SOURCE ) ;
				if ( dwDestinationBytes > 0 )	break ;
				dwNecessaryBytes <<= 1 ;
			}
			//
			// ACM 展開の準備
			ACMSTREAMHEADER	acmsh ;
			::memset( &acmsh, 0, sizeof(ACMSTREAMHEADER) ) ;
			acmsh.cbStruct = sizeof(ACMSTREAMHEADER) ;
			EPtrBuffer	ptrbuf = m_ACMSrcBuffer.GetBuffer() ;
			acmsh.pbSrc = (LPBYTE)ptrbuf.GetBuffer() ;
			acmsh.cbSrcLength = ptrbuf.GetLength() ;
			acmsh.pbDst = (LPBYTE)
				m_ACMDstBuffer.PutBuffer( dwDestinationBytes ) ;
			acmsh.cbDstLength = dwDestinationBytes ;
			if ( ::acmStreamPrepareHeader( m_hACMStream, &acmsh, 0 ) )
			{
				ESLTrace( "acmStreamPrepareeader 関数が失敗しました。\n" ) ;
				break ;
			}
			//
			// ACM 展開
			if ( ::acmStreamConvert
				( m_hACMStream, &acmsh, fdwConversion ) )
			{
				ESLTrace( "acmStreamConvert 関数が失敗しました。\n" ) ;
				break ;
			}
			//
			// ACM バッファのパラメータを正規化する
			m_ACMSrcBuffer.Release( acmsh.cbSrcLengthUsed ) ;
			m_ACMDstBuffer.Flush( acmsh.cbDstLengthUsed ) ;
			//
			// 展開の終了処理
			::acmStreamUnprepareHeader( m_hACMStream, &acmsh, 0 ) ;
		}

		//
		// 現在の入力音声バッファは空か？
		//////////////////////////////////////////////////////////////////////
		if ( m_pNextWaveHdr->dwBufferLength <= m_nOffsetSrcBuf )
		{
			// m_WasteWaveHdr に追加する
			*((UINT64*)&m_pNextWaveHdr->dwFlags) =
				((m_nOutputCounter + nOutBytes / nDstBkAlign)
					* m_WaveFormat.nSamplesPerSec
					/ pMixingFormat->nSamplesPerSec) ;
			m_pNextWaveHdr->dwUser = 0 ;
			m_WasteWaveHdr.Add( m_pNextWaveHdr ) ;

			// 次の音声バッファを取得する
			m_nOffsetSrcBuf = 0 ;
			if ( m_QueueWaveHdr.GetSize() > 0 )
			{
				m_pNextWaveHdr = m_QueueWaveHdr.GetAt(0) ;
				m_QueueWaveHdr.RemoveAt(0) ;
			}
			else
			{
				m_pNextWaveHdr =
					m_pWaveBufObj->OnQueueNextBuffer( m_pMixingServer ) ;
			}
		}
	}

	// 関数終了
	//////////////////////////////////////////////////////////////////////////
	if ( nBufIndex == -1 )
	{
		m_PausedBuffer.Flush( nOutBytes ) ;
		nLenSamples = m_PausedBuffer.GetLength() / nDstBkAlign ;
		return	NULL ;
	}
	else
	{
		nLenSamples = nOutBytes / nDstBkAlign; 
		m_nOutputCounter += nLenSamples ;
		if ( nOutBytes == 0 )	return	NULL ;
		m_nWaveBufLen[nBufIndex] = nOffsetSamples + nLenSamples ;
		return	((BYTE*)m_pWaveBuffer[nBufIndex])
							+ nOffsetSamples * nDstBkAlign ;
	}
}

// DirectSoundBuffer へ音量反映
//////////////////////////////////////////////////////////////////////////////
void WAVE_MIXER_CHANNEL::SetDirectSoundVolume( void )
{
	if ( (m_idsbuf != NULL) && (m_pWaveBufObj != NULL) )
	{
		double	rPanSign = -1, rPan, rVolume ;
		double	rEffVol[2] ;
		int		iPan = 0, iVolume = 1 ;
		rEffVol[0] =
			(double) m_pWaveBufObj->m_realVolume[0]
				* m_pMixingServer->m_nTotalVolume[0] / 0xFFFF ;
		rEffVol[1] =
			(double) m_pWaveBufObj->m_realVolume[1]
				* m_pMixingServer->m_nTotalVolume[1] / 0xFFFF ;
		if ( rEffVol[0] > rEffVol[1] )
		{
			iPan = 1 ;
			rPanSign = 1 ;
			iVolume = 0 ;
		}
		rPan = rEffVol[iPan] ;
		rVolume = rEffVol[iVolume] ;
		//
		LONG	lPan = DSBPAN_CENTER ;
		LONG	lVolume = DSBVOLUME_MIN ;
		if ( rVolume >= 1.0e-6 )
		{
			lVolume = (LONG) (2000.0 * log10( rVolume )) ;
			if ( lVolume >= DSBVOLUME_MAX )
			{
				lVolume = DSBVOLUME_MAX ;
			}
			else if ( lVolume < DSBVOLUME_MIN )
			{
				lVolume = DSBVOLUME_MIN ;
			}
			//
			rPan /= rVolume ;
			if ( rPan >= 1.0e-6 )
			{
				lPan = (LONG) (2000.0 * log10( rPan ) * rPanSign) ;
				if ( lPan < DSBPAN_LEFT )
				{
					lPan = DSBPAN_LEFT ;
				}
				else if ( lPan > DSBPAN_RIGHT )
				{
					lPan = DSBPAN_RIGHT ;
				}
			}
			else if ( rPanSign < 0 )
			{
				lPan = DSBPAN_RIGHT ;
			}
			else
			{
				lPan = DSBPAN_LEFT ;
			}
		}
		//
		m_idsbuf->SetVolume( lVolume ) ;
		m_idsbuf->SetPan( lPan ) ;
	}
}

// DirectSoundBuffer ストリーミング再生開始
//////////////////////////////////////////////////////////////////////////////
void WAVE_MIXER_CHANNEL::PlayDirectSound( void )
{
	if ( m_idsbuf != NULL )
	{
		SetDirectSoundVolume( ) ;
		//
		for ( int i = 0; i < BUFFER_COUNT; i ++ )
		{
//			::ResetEvent( m_hDirectSoundNotify[i] ) ;
			StreamingDirectSoundBuffer( i, m_pMixingServer->m_pWaveFormat ) ;
		}
		m_nBufferSwitch = 0 ;
		m_nPlayPosBias = m_nPausedPosition ;
		m_idsbuf->SetCurrentPosition( 0 ) ;
		m_idsbuf->Play( NULL, NULL, DSBPLAY_LOOPING ) ;
	}
}

// 現在の再生位置取得
//////////////////////////////////////////////////////////////////////////////
UINT64 WAVE_MIXER_CHANNEL::GetCurrentDirectSoundPosition( void )
{
	UINT64	nCurPos = m_nPlayPosBias ;
	if ( m_idsbuf != NULL )
	{
		DWORD	dwPlayPos = 0 ;
		if ( m_idsbuf->GetCurrentPosition( &dwPlayPos, NULL ) == DS_OK )
		{
			nCurPos +=
				dwPlayPos / m_pMixingServer->m_pWaveFormat->nBlockAlign ;
			//
			unsigned int	nBufferingBytes =
				m_pMixingServer->m_nBufferingSize
					* m_pMixingServer->m_pWaveFormat->nBlockAlign ;
			if ( dwPlayPos / nBufferingBytes < m_nBufferSwitch )
			{
				nCurPos +=
					m_pMixingServer->m_nBufferingSize * BUFFER_COUNT ;
			}
		}
		else
		{
			ESLTrace( "Failed to GetCurrentPosition.\n" ) ;
		}
	}
	return	nCurPos ;
}

// DirectSoundBuffer ストリーミング処理
//////////////////////////////////////////////////////////////////////////////
bool WAVE_MIXER_CHANNEL::StreamingDirectSoundBuffer
	( unsigned int nBufIndex, const WAVEFORMATEX * pMixingFormat )
{
	if ( m_idsbuf == NULL )
	{
		return	false ;
	}
	unsigned int	nSamples = m_pMixingServer->m_nBufferingSize ;
	void *	pWaveBuf =
		GetNextWaveSamples( nBufIndex, 0, nSamples, pMixingFormat ) ;
	//
	unsigned int	nBufferingBytes =
		m_pMixingServer->m_nBufferingSize * pMixingFormat->nBlockAlign ;
	void *	ptrSoundBuffer = NULL ;
	DWORD	dwSoundBufferBytes = 0 ;
	if ( m_idsbuf->Lock
		( nBufIndex * nBufferingBytes, nBufferingBytes,
			&ptrSoundBuffer, &dwSoundBufferBytes, NULL, NULL, 0 ) == DS_OK )
	{
		unsigned int	nCopyBytes = nSamples * pMixingFormat->nBlockAlign ;
		if ( nCopyBytes > dwSoundBufferBytes )
		{
			nCopyBytes = dwSoundBufferBytes ;
		}
		::eslMoveMemory( ptrSoundBuffer, pWaveBuf, nCopyBytes ) ;
		//
		if ( nCopyBytes < dwSoundBufferBytes )
		{
			::glsSound_CleanSoundBuffer
				( pMixingFormat,
					((BYTE*)ptrSoundBuffer) + nCopyBytes,
					(dwSoundBufferBytes - nCopyBytes)
								/ pMixingFormat->nBlockAlign ) ;
		}
		m_idsbuf->Unlock( ptrSoundBuffer, dwSoundBufferBytes, NULL, NULL ) ;
	}
	else
	{
		ESLTrace( "IDirectSoundBuffer::Lock に失敗しました。\n" ) ;
	}
	//
	return	(nSamples == m_pMixingServer->m_nBufferingSize) ;
}

void WAVE_MIXER_CHANNEL::OnTimerDirectSoundStreaming( void )
{
	//
	// バッファロストチェック
	//
	if ( m_idsbuf == NULL )
	{
		return ;
	}
	DWORD	dwStatus ;
	if ( m_idsbuf->GetStatus( &dwStatus ) == DS_OK )
	{
		if ( dwStatus & DSBSTATUS_BUFFERLOST )
		{
			if ( m_idsbuf->Restore() != DS_OK )
			{
				return ;
			}
		}
	}
	//
	// 区間再生終了チェック
	//
/*	if ( ::WaitForSingleObject
		( m_hDirectSoundNotify[m_nBufferSwitch], 0 ) == WAIT_TIMEOUT )
	{
		return ;
	}
	//
	::ResetEvent( m_hDirectSoundNotify[m_nBufferSwitch] ) ;
*/	//
	DWORD	dwPlayPos = 0 ;
	if ( m_idsbuf->GetCurrentPosition( &dwPlayPos, NULL ) == DS_OK )
	{
		unsigned int	nBufferingBytes =
			m_pMixingServer->m_nBufferingSize
				* m_pMixingServer->m_pWaveFormat->nBlockAlign ;
		if ( dwPlayPos / nBufferingBytes == m_nBufferSwitch )
		{
			return ;
		}
	}
	else
	{
		ESLTrace( "IDirectSoundBuffer::GetCurrentPosition に失敗しました。\n" ) ;
		return ;
	}
	StreamingDirectSoundBuffer
		( m_nBufferSwitch, m_pMixingServer->m_pWaveFormat ) ;
	//
	m_nBufferSwitch = (m_nBufferSwitch + 1) % BUFFER_COUNT ;
	if ( m_nBufferSwitch == 0 )
	{
		m_nPlayPosBias += m_pMixingServer->m_nBufferingSize * BUFFER_COUNT ;
	}
}


/*****************************************************************************
					音声ミキシングサーバーオブジェクト
 ****************************************************************************/

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( EWaveMixingServer, EWaveOutDevice )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
EWaveMixingServer::EWaveMixingServer( void )
{
	// 変数初期化
	m_pWaveBuffers[0] = NULL ;
	m_pWaveBuffers[1] = NULL ;
	m_eventEmptyWaveBuf = ::CreateEvent( NULL, FALSE, TRUE, NULL ) ;
	m_boolEmptyWaveBuf[0] = TRUE ;
	m_boolEmptyWaveBuf[1] = TRUE ;
	m_nPreparedSize[0] = 0 ;
	m_nPreparedSize[1] = 0 ;
	m_nQuantumTime = 33 ;
	m_nTimerID = 0 ;
	m_fCallbackMode = 0 ;
	m_nPrimaryIndex = 0 ;
	m_dwDevPausedTime = 0 ;
	//
	m_idsound = NULL ;
	m_idsbuf = NULL ;
	//
	m_pVirtualBuf = NULL ;
	m_fDeleteVirtualBuf = false ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
EWaveMixingServer::~EWaveMixingServer( void )
{
	if ( m_pWaveFormat || m_idsound )
	{
		Close( ) ;
	}
	//
	::CloseHandle( m_eventEmptyWaveBuf ) ;
	m_eventEmptyWaveBuf = NULL ;
}

// ミキシングサービスを開始する
//////////////////////////////////////////////////////////////////////////////
ESLError EWaveMixingServer::Open
	( unsigned int nBufferingTime,
		unsigned int nQuantumTime,
		unsigned int nFrequency,
		unsigned int nChannels,
		unsigned int nBitsPerSample )
{
	// 現在のミキシングを閉じる
	//////////////////////////////////////////////////////////////////////////
	if ( m_pWaveFormat )
	{
		Close( ) ;
	}

	// パラメータをチェック
	//////////////////////////////////////////////////////////////////////////
	if ( (nChannels != 1) && (nChannels != 2) )
	{
		return	ESLErrorMsg( "無効なチャネル数を指定しています。" ) ;
	}
	if ( (nBitsPerSample != 8) && (nBitsPerSample != 16) )
	{
		return	ESLErrorMsg( "無効なビット分解能を指定しています。" ) ;
	}

	// パラメータを設定
	//////////////////////////////////////////////////////////////////////////
	WAVEFORMATEX	wfx ;
	wfx.wFormatTag = WAVE_FORMAT_PCM ;
	wfx.nChannels = (WORD) nChannels ;
	wfx.nSamplesPerSec = nFrequency ;
	wfx.wBitsPerSample = (WORD) nBitsPerSample ;
	wfx.nBlockAlign = (WORD) (nChannels * (nBitsPerSample >> 3)) ;
	wfx.nAvgBytesPerSec = nFrequency * wfx.nBlockAlign ;
	wfx.cbSize = 0 ;
	//
	m_nQuantumTime = nQuantumTime ;
	if ( nBufferingTime < m_nQuantumTime )
	{
		m_nQuantumTime = nBufferingTime ;
	}

	// 音声出力デバイスを開く
	//////////////////////////////////////////////////////////////////////////
	Lock( ) ;
	ESLError	err = EWaveOutDevice::Open( &wfx ) ;
	if ( err != eslErrSuccess )
	{
		Unlock( ) ;
		Close() ;
		return	err ;
	}

	// バッファリングサイズを取得
	//////////////////////////////////////////////////////////////////////////
	m_nBufferingSize =
		(nBufferingTime * nFrequency / 1000) + (0x1000 - 1) ;
	m_nBufferingSize &= (~0xFFF) ;
	ESLAssert( m_nBufferingSize ) ;
	unsigned int	nBufLength = m_nBufferingSize * wfx.nBlockAlign ;

	// 音声出力開始
	//////////////////////////////////////////////////////////////////////////
	m_nOutputCounter = 0 ;
	m_boolEmptyWaveBuf[0] = FALSE ;
	m_boolEmptyWaveBuf[1] = FALSE ;
	::ResetEvent( m_eventEmptyWaveBuf ) ;
	::waveOutPause( m_hWaveOut ) ;
	m_nPrimaryIndex = 0 ;
	for ( unsigned int i = 0; i < 2; i ++ )
	{
		MMRESULT	mmr ;
		m_nPreparedSize[i] = m_nBufferingSize ;
		m_pWaveBuffers[i] = ::eslHeapAllocate( NULL, nBufLength, 0 ) ;
		::glsSound_CleanSoundBuffer
			( m_pWaveFormat, m_pWaveBuffers[i], m_nBufferingSize ) ;
		::memset( &m_WaveHeaders[i], 0, sizeof(WAVEHDR) ) ;
		m_WaveHeaders[i].lpData = (LPSTR)m_pWaveBuffers[i] ;
		m_WaveHeaders[i].dwBufferLength = nBufLength ;
//		m_WaveHeaders[i].dwBytesRecorded = nBufLength ;
		mmr = ::waveOutPrepareHeader
			( m_hWaveOut, &m_WaveHeaders[i], sizeof(WAVEHDR) ) ;
		if ( mmr != MMSYSERR_NOERROR )
		{
			ESLTrace( "waveOutPrepareHeader 関数が失敗しました。(%d)\n", mmr ) ;
			ESLTrace( "m_hWaveOut = %08X\n", m_hWaveOut ) ;
		}
		else
		{
			mmr = ::waveOutWrite
				( m_hWaveOut, &m_WaveHeaders[i], sizeof(WAVEHDR) ) ;
			if ( mmr != MMSYSERR_NOERROR )
			{
				ESLTrace( "waveOutWrite 関数が失敗しました。(%d)\n", mmr ) ;
				ESLTrace( "m_hWaveOut = %08X\n", m_hWaveOut ) ;
			}
		}
	}
	::waveOutRestart( m_hWaveOut ) ;
	m_dwBaseBufferTime = ::timeGetTime( ) ;
	m_dwDevPausedTime = 0 ;
	Unlock( ) ;

	return	eslErrSuccess ;
}

// ミキシングサービスを開始する (DirectSound)
//////////////////////////////////////////////////////////////////////////////
ESLError EWaveMixingServer::OpenDirectSound
	( unsigned int nBufferingTime,
		unsigned int nQuantumTime,
		unsigned int nFrequency,
		unsigned int nChannels,
		unsigned int nBitsPerSample )
{
	// 現在のミキシングを閉じる
	//////////////////////////////////////////////////////////////////////////
	if ( m_pWaveFormat )
	{
		Close( ) ;
	}

	// パラメータをチェック
	//////////////////////////////////////////////////////////////////////////
	if ( (nChannels != 1) && (nChannels != 2) )
	{
		return	ESLErrorMsg( "無効なチャネル数を指定しています。" ) ;
	}
	if ( (nBitsPerSample != 8) && (nBitsPerSample != 16) )
	{
		return	ESLErrorMsg( "無効なビット分解能を指定しています。" ) ;
	}

	// パラメータを設定
	//////////////////////////////////////////////////////////////////////////
	WAVEFORMATEX	wfx ;
	wfx.wFormatTag = WAVE_FORMAT_PCM ;
	wfx.nChannels = (WORD) nChannels ;
	wfx.nSamplesPerSec = nFrequency ;
	wfx.wBitsPerSample = (WORD) nBitsPerSample ;
	wfx.nBlockAlign = (WORD) (nChannels * (nBitsPerSample >> 3)) ;
	wfx.nAvgBytesPerSec = nFrequency * wfx.nBlockAlign ;
	wfx.cbSize = 0 ;
	//
	m_nQuantumTime = nQuantumTime ;
	if ( nBufferingTime < m_nQuantumTime )
	{
		m_nQuantumTime = nBufferingTime ;
	}
	//
	m_nBufferingSize =
		(nBufferingTime * nFrequency / 1000) + (0x1000 - 1) ;
	m_nBufferingSize &= (~0xFFF) ;
	ESLAssert( m_nBufferingSize ) ;
	//
	m_nOutputCounter = 0 ;
	m_boolEmptyWaveBuf[0] = FALSE ;
	m_boolEmptyWaveBuf[1] = FALSE ;
	::ResetEvent( m_eventEmptyWaveBuf ) ;
	m_nPrimaryIndex = 0 ;
	m_dwDevPausedTime = 0 ;

	// コールバックスレッド作成
	//////////////////////////////////////////////////////////////////////////
	ESLAssert( m_hThread == NULL ) ;
	ESLAssert( m_hThreadCreated == NULL ) ;
	m_hThreadCreated = ::CreateEvent( NULL, TRUE, FALSE, NULL ) ;
	m_hThread = ::CreateThread
		( NULL, 0, &WaveCallbackServiceThread, this, 0, &m_idThread ) ;
	if ( m_hThread != NULL )
	{
		::WaitForSingleObject( m_hThreadCreated, INFINITE ) ;
	}

	// DirectSound を生成する
	//////////////////////////////////////////////////////////////////////////
	//
	::CoInitialize( NULL ) ;
	//
	ESLAssert( m_idsound == NULL ) ;
	if ( SUCCEEDED( ::CoCreateInstance( GLS_CLSID_DirectSound, NULL,
			CLSCTX_ALL, GLS_IID_IDirectSound, (void**) &m_idsound ) ) )
	{
		ESLAssert( m_idsound != NULL ) ;
		m_idsound->Initialize( NULL ) ;
		//
		HWND	hWnd = GetDesktopWindow( ) ;
		m_idsound->SetCooperativeLevel( hWnd, DSSCL_PRIORITY ) ;
	}
	else
	{
		m_idsound = NULL ;
		Close() ;
		return	ESLErrorMsg( "DirectSound の生成に失敗しました。" ) ;
	}
	//
	DSBUFFERDESC	dsbd ;
	::eslFillMemory( &dsbd, 0, sizeof(dsbd) ) ;
	dsbd.dwSize = sizeof(dsbd) ;
	dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER ;
	dsbd.dwBufferBytes = 0 ;
	dsbd.lpwfxFormat = NULL ;
	//
	ESLAssert( m_idsbuf == NULL ) ;
	if ( m_idsound->CreateSoundBuffer( &dsbd, &m_idsbuf, NULL ) == DS_OK )
	{
		ESLAssert( m_idsbuf != NULL ) ;
	}
	else
	{
		m_idsbuf = NULL ;
		Close() ;
		return	ESLErrorMsg( "DirectSoundBuffer の生成に失敗しました。" ) ;
	}
	if ( m_idsbuf->SetFormat( &wfx ) != DS_OK )
	{
		ESLTrace( "IDirectSoundBuffer::SetFormat が失敗しました。\n" ) ;
	}
	m_nTotalVolume[0] = 0xFFFF ;
	m_nTotalVolume[1] = 0xFFFF ;

	// 音声出力フォーマットを複製
	//////////////////////////////////////////////////////////////////////////
	unsigned int	nFmtBytes = sizeof(WAVEFORMATEX) ;
	m_pWaveFormat =
		(WAVEFORMATEX*) ::eslHeapAllocate( NULL, nFmtBytes, 0 ) ;
	::eslMoveMemory( m_pWaveFormat, &wfx, nFmtBytes ) ;

	return	eslErrSuccess ;
}

// ミキシングサービスを開始する（仮想出力）
//////////////////////////////////////////////////////////////////////////////
ESLError EWaveMixingServer::OpenVirtual
	( EStreamBuffer * pOutputBuf,
		unsigned int nBufferingTime,
		unsigned int nQuantumTime,
		unsigned int nFrequency,
		unsigned int nChannels,
		unsigned int nBitsPerSample )
{
	// 現在のミキシングを閉じる
	//////////////////////////////////////////////////////////////////////////
	if ( m_pWaveFormat )
	{
		Close( ) ;
	}

	// パラメータをチェック
	//////////////////////////////////////////////////////////////////////////
	if ( (nChannels != 1) && (nChannels != 2) )
	{
		return	ESLErrorMsg( "無効なチャネル数を指定しています。" ) ;
	}
	if ( (nBitsPerSample != 8) && (nBitsPerSample != 16) )
	{
		return	ESLErrorMsg( "無効なビット分解能を指定しています。" ) ;
	}

	// パラメータを設定
	//////////////////////////////////////////////////////////////////////////
	WAVEFORMATEX	wfx ;
	wfx.wFormatTag = WAVE_FORMAT_PCM ;
	wfx.nChannels = (WORD) nChannels ;
	wfx.nSamplesPerSec = nFrequency ;
	wfx.wBitsPerSample = (WORD) nBitsPerSample ;
	wfx.nBlockAlign = (WORD) (nChannels * (nBitsPerSample >> 3)) ;
	wfx.nAvgBytesPerSec = nFrequency * wfx.nBlockAlign ;
	wfx.cbSize = 0 ;
	//
	m_nQuantumTime = nQuantumTime ;
	if ( nBufferingTime < m_nQuantumTime )
	{
		m_nQuantumTime = nBufferingTime ;
	}

	// 出力フォーマット設定
	//////////////////////////////////////////////////////////////////////////
	unsigned int	nFmtBytes = sizeof(WAVEFORMATEX) ;
	m_pWaveFormat =
		(WAVEFORMATEX*) ::eslHeapAllocate( NULL, nFmtBytes, 0 ) ;
	::memcpy( m_pWaveFormat, &wfx, nFmtBytes ) ;

	// バッファリングサイズを取得
	//////////////////////////////////////////////////////////////////////////
	m_nBufferingSize =
		(nBufferingTime * nFrequency / 1000) + (0x1000 - 1) ;
	m_nBufferingSize &= (~0xFFF) ;
	ESLAssert( m_nBufferingSize ) ;
	unsigned int	nBufLength = m_nBufferingSize * wfx.nBlockAlign ;

	// 仮想出力準備
	//////////////////////////////////////////////////////////////////////////
	m_nOutputCounter = 0 ;
	m_boolEmptyWaveBuf[0] = FALSE ;
	m_boolEmptyWaveBuf[1] = FALSE ;
	m_nPrimaryIndex = 0 ;
	for ( unsigned int i = 0; i < 2; i ++ )
	{
		m_nPreparedSize[i] = m_nBufferingSize ;
		m_pWaveBuffers[i] = ::eslHeapAllocate( NULL, nBufLength, 0 ) ;
		::glsSound_CleanSoundBuffer
			( m_pWaveFormat, m_pWaveBuffers[i], m_nBufferingSize ) ;
		::memset( &m_WaveHeaders[i], 0, sizeof(WAVEHDR) ) ;
		m_WaveHeaders[i].lpData = (LPSTR) m_pWaveBuffers[i] ;
		m_WaveHeaders[i].dwBufferLength = nBufLength ;
//		m_WaveHeaders[i].dwBytesRecorded = nBufLength ;
	}
	m_dwBaseBufferTime = ::timeGetTime( ) ;
	m_dwDevPausedTime = 0 ;
	//
	m_nTotalVolume[0] = 0xFFFF ;
	m_nTotalVolume[1] = 0xFFFF ;
	//
	m_pVirtualBuf = pOutputBuf ;
	m_dwVirtualLocalPos = 0 ;
	m_fDeleteVirtualBuf = false ;

	return	eslErrSuccess ;
}

ESLError EWaveMixingServer::OpenVirtualSync
	( unsigned int nBufferingTime,
		unsigned int nQuantumTime,
		unsigned int nFrequency,
		unsigned int nChannels,
		unsigned int nBitsPerSample )
{
	// 仮想出力設定
	//////////////////////////////////////////////////////////////////////////
	EStreamBuffer *	pbufTemp = new EStreamBuffer ;
	ESLError	err = OpenVirtual
		( pbufTemp, nBufferingTime, nQuantumTime,
				nFrequency, nChannels, nBitsPerSample ) ;
	if ( err )
	{
		delete	pbufTemp ;
		return	err ;
	}
	m_fDeleteVirtualBuf = true ;

	// コールバックスレッド作成
	//////////////////////////////////////////////////////////////////////////
	ESLAssert( m_hThread == NULL ) ;
	ESLAssert( m_hThreadCreated == NULL ) ;
	m_hThreadCreated = ::CreateEvent( NULL, TRUE, FALSE, NULL ) ;
	m_hThread = ::CreateThread
		( NULL, 0, &VirtualSyncThreadProc, this, 0, &m_idThread ) ;
	if ( m_hThread != NULL )
	{
		::WaitForSingleObject( m_hThreadCreated, INFINITE ) ;
	}

	return	eslErrSuccess ;
}

// 音声ミキシングサービスを終了する
//////////////////////////////////////////////////////////////////////////////
void EWaveMixingServer::Close( void )
{
	if ( m_idsound != NULL )
	{
		Lock( ) ;
		while ( m_listWaveChannel.GetSize() > 0 )
		{
			WAVE_MIXER_CHANNEL *	pwmixch = m_listWaveChannel.GetAt( 0 ) ;
			if ( (pwmixch != NULL) && (pwmixch->m_pWaveBufObj != NULL) )
			{
				Stop( pwmixch->m_pWaveBufObj ) ;
			}
			else
			{
				m_listWaveChannel.RemoveAt( 0 ) ;
			}
		}
		::eslHeapFree( NULL, m_pWaveFormat ) ;
		m_pWaveFormat = NULL ;
		Unlock( ) ;
	}
	else if ( m_pVirtualBuf != NULL )
	{
		// 仮想出力を閉じる
		//////////////////////////////////////////////////////////////////////
		if ( m_fDeleteVirtualBuf )
		{
			::PostThreadMessage( m_idThread, WM_QUIT, 0, 0 ) ;
			::WaitForSingleObject( m_hThread, INFINITE ) ;
			::CloseHandle( m_hThread ) ;
			m_hThread = NULL ;
		}
		while ( m_listWaveChannel.GetSize() > 0 )
		{
			WAVE_MIXER_CHANNEL *	pwmixch = m_listWaveChannel.GetAt( 0 ) ;
			if ( (pwmixch != NULL) && (pwmixch->m_pWaveBufObj != NULL) )
			{
				Stop( pwmixch->m_pWaveBufObj ) ;
			}
			else
			{
				m_listWaveChannel.RemoveAt( 0 ) ;
			}
		}
		//
		OnVirtualBufferPlayed( ) ;
		OnVirtualBufferPlayed( ) ;
		//
		m_boolEmptyWaveBuf[0] = FALSE ;
		m_boolEmptyWaveBuf[1] = FALSE ;
		//
		if ( m_fDeleteVirtualBuf )
		{
			delete	m_pVirtualBuf ;
		}
		m_pVirtualBuf = NULL ;
		m_fDeleteVirtualBuf = false ;
		//
		::eslHeapFree( NULL, m_pWaveFormat ) ;
		m_pWaveFormat = NULL ;
	}
	else if ( m_pWaveFormat != NULL )
	{
		// 音声出力デバイスを閉じる
		//////////////////////////////////////////////////////////////////////
		UINT	nSamplesPerSec = m_pWaveFormat->nSamplesPerSec ;
		Lock( ) ;
		::waveOutReset( m_hWaveOut ) ;
		::waveOutUnprepareHeader
			( m_hWaveOut, &m_WaveHeaders[0], sizeof(WAVEHDR) ) ;
		::waveOutUnprepareHeader
			( m_hWaveOut, &m_WaveHeaders[1], sizeof(WAVEHDR) ) ;
//		EWaveOutDevice::Close( ) ;

		// 全てのチャネルを削除
		//////////////////////////////////////////////////////////////////////
		if ( m_listWaveChannel.GetSize() > 0 )
		{
			ESLTrace( "警告：再生中のミキシングチャネルが存在します。\n" ) ;
			m_listWaveChannel.RemoveAll( ) ;
		}
		Unlock( ) ;

		// 再生が完全に終了するまで待機
		//////////////////////////////////////////////////////////////////////
		::WaitForSingleObject
			( m_eventEmptyWaveBuf,
				(DWORD) ((INT64) m_nBufferingSize * 2000 / nSamplesPerSec) ) ;
		::ResetEvent( m_eventEmptyWaveBuf ) ;
		m_boolEmptyWaveBuf[0] = FALSE ;
		m_boolEmptyWaveBuf[1] = FALSE ;
	}
	// ミキシングバッファを解放
	//////////////////////////////////////////////////////////////////////
	if ( m_pWaveBuffers[0] != NULL )
	{
		::eslHeapFree( NULL, m_pWaveBuffers[0] ) ;
	}
	if ( m_pWaveBuffers[1] != NULL )
	{
		::eslHeapFree( NULL, m_pWaveBuffers[1] ) ;
	}
	m_pWaveBuffers[0] = NULL ;
	m_pWaveBuffers[1] = NULL ;
	m_fCallbackMode = 0 ;
	//
	// DirectSound を解放
	//////////////////////////////////////////////////////////////////////
	if ( m_idsbuf != NULL )
	{
		m_idsbuf->Release( ) ;
		m_idsbuf = NULL ;
	}
	if ( m_idsound != NULL )
	{
		m_idsound->Release( ) ;
		m_idsound = NULL ;
//		::CoUninitialize( ) ;
	}
	EWaveOutDevice::Close( ) ;
}

// 現在のミキシング位置を取得
//////////////////////////////////////////////////////////////////////////////
unsigned int EWaveMixingServer::
	GetCurrentMixingPosition( unsigned int nOffsetTime )
{
	if ( m_pVirtualBuf != NULL )
	{
		return	m_dwVirtualLocalPos ;
//				+ (nOffsetTime * m_pWaveFormat->nSamplesPerSec / 1000) ;
	}
	else
	{
		DWORD	dwOffsetTime =
			::timeGetTime() - m_dwBaseBufferTime + nOffsetTime ;
		int	nOffsetCount =
			(int)(dwOffsetTime * m_pWaveFormat->nSamplesPerSec / 1000) ;
		if ( nOffsetCount <= 0 )
			return	0 ;
		return	((nOffsetCount + 0x07) & ~0x07) ;
	}
}

// 音声バッファを準備
//////////////////////////////////////////////////////////////////////////////
HWAVEBUF EWaveMixingServer::PrepareBuffer
	( EWaveStreamBuffer * pWaveBuffer,
		const void * ptrBuffer, unsigned int nBufferLength )
{
	if ( m_pWaveFormat == NULL )
	{
		ESLTrace( "音声出力デバイスが開かれていません。\n" ) ;
		return	NULL ;
	}

	if ( IsBadReadPtr( ptrBuffer, nBufferLength ) )
	{
		ESLTrace( "不正な音声バッファを出力しようとしました。\n" ) ;
		return	NULL ;
	}

	WAVEHDR *	pWaveHdr = new WAVEHDR ;
	pWaveHdr->lpData = (LPSTR) ptrBuffer ;
	pWaveHdr->dwBufferLength = nBufferLength ;
	pWaveHdr->dwBytesRecorded = nBufferLength ;
	pWaveHdr->dwUser = (DWORD) pWaveBuffer ;
	pWaveHdr->dwFlags = 0 ;
	pWaveHdr->dwLoops = 0 ;
	pWaveHdr->lpNext = 0 ;
	pWaveHdr->reserved = 0 ;

	Lock( ) ;
	m_arrayPlayBuf.Add( pWaveHdr ) ;
	Unlock( ) ;

	return	pWaveHdr ;
}

// 音声バッファを終了
//////////////////////////////////////////////////////////////////////////////
ESLError EWaveMixingServer::UnprepareBuffer( HWAVEBUF hWaveBuf )
{
	if ( m_pWaveFormat == NULL )
		return	ESLErrorMsg( "音声出力デバイスが開かれていません。" ) ;
	if ( hWaveBuf == NULL )
		return	ESLErrorMsg( "不正な音声バッファハンドルです。" ) ;

	Lock( ) ;
	for ( unsigned int i = 0; i < m_arrayPlayBuf.GetSize(); i ++ )
	{
		if ( m_arrayPlayBuf.GetAt(i) == hWaveBuf )
		{
			m_arrayPlayBuf.RemoveAt( i ) ;
			delete	hWaveBuf ;
			Unlock( ) ;
			return	eslErrSuccess ;
		}
	}
	Unlock( ) ;

	return	ESLErrorMsg( "不正な音声バッファハンドルです。" ) ;
}

// HWAVEMIX に関連する EWaveStreamBuffer を取得
//////////////////////////////////////////////////////////////////////////////
HWAVEMIX EWaveMixingServer::
	GetWaveMixChannel( const EWaveStreamBuffer * pWaveBuffer ) const
{
	HWAVEMIX	hWaveMix = NULL ;
	Lock( ) ;
	for ( unsigned int i = 0; i < m_listWaveChannel.GetSize(); i ++ )
	{
		WAVE_MIXER_CHANNEL *
			pMixChannel = m_listWaveChannel.GetAt( i ) ;
		if( pMixChannel->m_pWaveBufObj == pWaveBuffer )
		{
			hWaveMix = pMixChannel ;
			break ;
		}
	}
	Unlock( ) ;
	return	hWaveMix ;
}

// 音声バッファを出力
//////////////////////////////////////////////////////////////////////////////
ESLError EWaveMixingServer::Play
	( EWaveStreamBuffer * pWaveBuffer, HWAVEBUF hWaveBuf )
{
	return	DelayPlaying( pWaveBuffer, hWaveBuf, 66 ) ;
}

ESLError EWaveMixingServer::DelayPlaying
( EWaveStreamBuffer * pWaveBuffer, HWAVEBUF hWaveBuf, unsigned int nDelay )
{
	ESLError	err ;

	if ( m_pWaveFormat == NULL )
	{
		return	ESLErrorMsg( "音声出力デバイスが開かれていません。" ) ;
	}
	if ( hWaveBuf == NULL )
	{
		return	ESLErrorMsg( "無効な音声バッファハンドルが指定されています。" ) ;
	}

	//
	// 再生中の音声ミキサーチャネルを検索
	//////////////////////////////////////////////////////////////////////////
	Lock( ) ;
	HWAVEMIX	pMixChannel = GetWaveMixChannel( pWaveBuffer ) ;
	if ( pMixChannel != NULL )
	{
		if ( pMixChannel->m_pNextWaveHdr == NULL )
		{
			pMixChannel->m_pNextWaveHdr = hWaveBuf ;
			pMixChannel->m_nOffsetSrcBuf = 0 ;
			pMixChannel->m_nBaseSampleIndex
							+= m_nBufferingSize ;	// ミキシングの遅延
		}
		else
		{
			pMixChannel->m_QueueWaveHdr.Add( hWaveBuf ) ;
		}
		Unlock( ) ;
		return	eslErrSuccess ;
	}

	//
	// 新規にミキサーチャネルを作成
	//////////////////////////////////////////////////////////////////////////
	if ( pWaveBuffer->m_pWaveDevMixingBuf != NULL )
	{
		pMixChannel = pWaveBuffer->m_pWaveDevMixingBuf ;
		err = pMixChannel->ReInitialize( this, pWaveBuffer, hWaveBuf ) ;
		if ( err != eslErrSuccess )
		{
			delete	pMixChannel ;
			pWaveBuffer->m_pWaveDevMixingBuf = NULL ;
			//
			pMixChannel = new WAVE_MIXER_CHANNEL ;
			err = pMixChannel->Open( this, pWaveBuffer, hWaveBuf ) ;
			if ( err != eslErrSuccess )
			{
				delete	pMixChannel ;
				Unlock( ) ;
				return	err ;
			}
		}
	}
	else
	{
		pMixChannel = new WAVE_MIXER_CHANNEL ;
		err = pMixChannel->Open( this, pWaveBuffer, hWaveBuf ) ;
		if ( err != eslErrSuccess )
		{
			delete	pMixChannel ;
			Unlock( ) ;
			return	err ;
		}
	}
	m_listWaveChannel.Add( pMixChannel ) ;
	pWaveBuffer->m_pWaveDevMixingBuf = pMixChannel ;

	if ( !pMixChannel->m_nPauseFlag )
	{
		//
		// 現在の音声バッファにミックス
		//////////////////////////////////////////////////////////////////////
		if ( m_idsound != NULL )
		{
			pMixChannel->PlayDirectSound( ) ;
		}
		else
		{
			//
			// 先頭のミキシングバッファを準備
			unsigned int	nLenSamples = m_nBufferingSize ;
			pMixChannel->GetNextWaveSamples( -1, 0, nLenSamples, m_pWaveFormat ) ;
			//
			// 現在のミキシング位置を取得
			unsigned int	nNextMixPos = GetCurrentMixingPosition( nDelay ) ;
			pMixChannel->m_nBaseSampleIndex = m_nOutputCounter + nNextMixPos ;
			//
			// プライマリ音声バッファにミックス
			unsigned int	nOffsetPos, nMixSample ;
			void *			pWaveMixBuf ;
			if ( m_nPreparedSize[m_nPrimaryIndex] > nNextMixPos )
			{
				nOffsetPos = nNextMixPos ;
				nMixSample = m_nPreparedSize[m_nPrimaryIndex] - nOffsetPos ;
				nNextMixPos = 0 ;
				pWaveMixBuf = pMixChannel->GetNextWaveSamples
					( m_nPrimaryIndex, nOffsetPos, nMixSample, m_pWaveFormat ) ;
				if ( pWaveMixBuf != NULL )
				{
					::glsSound_MixPCMSamples
						( m_pWaveFormat,
							((BYTE*)m_pWaveBuffers[m_nPrimaryIndex])
								+ nOffsetPos * m_pWaveFormat->nBlockAlign,
							pWaveMixBuf, pWaveBuffer->m_realVolume, nMixSample ) ;
				}
			}
			else
			{
				nNextMixPos -= m_nPreparedSize[m_nPrimaryIndex] ;
			}
			//
			// セカンダリ音声バッファにミックス
			unsigned int	nSecondaryIndex =
								(m_nPrimaryIndex + 1) % 2 ;
			if ( m_nPreparedSize[nSecondaryIndex] > nNextMixPos )
			{
				nOffsetPos = nNextMixPos ;
				nMixSample = m_nPreparedSize[nSecondaryIndex] - nOffsetPos ;
				pWaveMixBuf = pMixChannel->GetNextWaveSamples
					( nSecondaryIndex, nOffsetPos, nMixSample, m_pWaveFormat ) ;
				if ( pWaveMixBuf != NULL )
				{
					::glsSound_MixPCMSamples
						( m_pWaveFormat,
							((BYTE*)m_pWaveBuffers[nSecondaryIndex])
								+ nOffsetPos * m_pWaveFormat->nBlockAlign,
							pWaveMixBuf, pWaveBuffer->m_realVolume, nMixSample ) ;
				}
			}
		}
	}
	Unlock( ) ;
	return	eslErrSuccess ;
}

// 音声出力を停止
//////////////////////////////////////////////////////////////////////////////
ESLError EWaveMixingServer::Stop( EWaveStreamBuffer * pWaveBuffer )
{
	//
	// ミックスされた音声バッファを復元
	//////////////////////////////////////////////////////////////////////////
	if ( m_idsound == NULL )
	{
		if ( GetWaveMixChannel( pWaveBuffer ) == NULL )
		{
			return	eslErrSuccess ;
		}
		//
		Lock( ) ;
		REAL32	rSaveVolume[2] ;
		rSaveVolume[0] = pWaveBuffer->m_realVolume[0] ;
		rSaveVolume[1] = pWaveBuffer->m_realVolume[1] ;
		REAL32	rVolume[2] = { 0, 0 } ;
		ESLError	err = SetVolume( pWaveBuffer, rVolume ) ;
		pWaveBuffer->m_realVolume[0] = rSaveVolume[0] ;
		pWaveBuffer->m_realVolume[1] = rSaveVolume[1] ;
		if ( err )
		{
			Unlock( ) ;
			return	err ;
		}
	}
	else
	{
		Lock( ) ;
	}

	//
	// WAVE_MIXER_CHANNEL を削除
	//////////////////////////////////////////////////////////////////////////
	for ( unsigned int i = 0; i < m_listWaveChannel.GetSize(); i ++ )
	{
		WAVE_MIXER_CHANNEL *
			pMixChannel = m_listWaveChannel.GetAt( i ) ;
		if ( pMixChannel->m_pWaveBufObj == pWaveBuffer )
		{
			//
			// 再生終了を通知
			//////////////////////////////////////////////////////////////////
			EWaveStreamBuffer *	pWaveBuffer = pMixChannel->m_pWaveBufObj ;
			WAVEHDR *			lpWaveHdr ;
			if ( pWaveBuffer != NULL )
			{
				while ( pMixChannel->m_WasteWaveHdr.GetSize() > 0 )
				{
					lpWaveHdr = pMixChannel->m_WasteWaveHdr.GetAt( 0 ) ;
					pMixChannel->m_WasteWaveHdr.RemoveAt( 0 ) ;
					pWaveBuffer->OnEndPlaying
						( lpWaveHdr, lpWaveHdr->lpData,
									lpWaveHdr->dwBufferLength ) ;
					UnprepareBuffer( lpWaveHdr ) ;
				}
				lpWaveHdr = pMixChannel->m_pNextWaveHdr ;
				if ( lpWaveHdr != NULL )
				{
					pWaveBuffer->OnEndPlaying
						( lpWaveHdr, lpWaveHdr->lpData,
								lpWaveHdr->dwBufferLength ) ;
					UnprepareBuffer( lpWaveHdr ) ;
				}
				while ( pMixChannel->m_QueueWaveHdr.GetSize() > 0 )
				{
					lpWaveHdr = pMixChannel->m_QueueWaveHdr.GetAt( 0 ) ;
					pMixChannel->m_QueueWaveHdr.RemoveAt( 0 ) ;
					pWaveBuffer->OnEndPlaying
						( lpWaveHdr, lpWaveHdr->lpData,
								lpWaveHdr->dwBufferLength ) ;
					UnprepareBuffer( lpWaveHdr ) ;
				}
			}
			//
			// WAVE_MIXER_CHANNEL を削除
			//////////////////////////////////////////////////////////////////
			pMixChannel->Stop( ) ;
			m_listWaveChannel.RemoveAt( i ) ;
			break ;
		}
	}
	Unlock( ) ;
	return	eslErrSuccess ;
}

// 音声ミキシングを一時停止
//////////////////////////////////////////////////////////////////////////////
ESLError EWaveMixingServer::Pause( EWaveStreamBuffer * pWaveBuffer )
{
	//
	// 関数準備
	//////////////////////////////////////////////////////////////////////////
	pWaveBuffer->m_fPauseFlag = TRUE ;
	if ( m_pWaveFormat == NULL )
	{
		return	eslErrSuccess ;
	}

	Lock( ) ;
	HWAVEMIX	hWaveMixer = GetWaveMixChannel( pWaveBuffer ) ;
	if ( hWaveMixer == NULL )
	{
		Unlock( ) ;
		return	eslErrSuccess ;
	}
	if ( hWaveMixer->m_nPauseFlag )
	{
		Unlock( ) ;
		return	eslErrSuccess ;
	}
	//
	// ミキシングバッファを保存
	//////////////////////////////////////////////////////////////////////////
	EStreamBuffer	tempStrmBuf ;
	if ( m_idsound != NULL )
	{
		//
		// DirectSound
		//
		IDirectSoundBuffer *	idsbuf = hWaveMixer->m_idsbuf ;
		if ( idsbuf != NULL )
		{
			DWORD	dwPlayPos = 0, dwLastBytes ;
			idsbuf->GetCurrentPosition( &dwPlayPos, NULL ) ;
			hWaveMixer->m_idsbuf->Stop( ) ;
			//
			unsigned int	nIndex = 0 ;
			unsigned int	nBlockAlign = m_pWaveFormat->nBlockAlign ;
			unsigned int	nBufferBytes = m_nBufferingSize * nBlockAlign ;
			//
			hWaveMixer->m_nPausedPosition =
				hWaveMixer->m_nPlayPosBias + dwPlayPos / nBlockAlign ;
			if ( dwPlayPos / nBufferBytes != hWaveMixer->m_nBufferSwitch )
			{
				if ( hWaveMixer->m_nBufferSwitch ==
						(WAVE_MIXER_CHANNEL::BUFFER_COUNT - 1) )
				{
					hWaveMixer->m_nPausedPosition +=
						m_nBufferingSize * WAVE_MIXER_CHANNEL::BUFFER_COUNT ;
				}
			}
			//
			if ( dwPlayPos >= nBufferBytes )
			{
				nIndex = 1 ;
				dwPlayPos -= nBufferBytes ;
			}
			if ( dwPlayPos < hWaveMixer->m_nWaveBufLen[nIndex] * nBlockAlign )
			{
				dwLastBytes =
					hWaveMixer->m_nWaveBufLen[nIndex]
									* nBlockAlign - dwPlayPos ;
				//
				// 一時バッファに保存
				BYTE *	pSrcBuffer =
					(BYTE*) hWaveMixer->m_pWaveBuffer[nIndex] ;
				tempStrmBuf.Write
					( pSrcBuffer + dwPlayPos, dwLastBytes ) ;
				hWaveMixer->m_nWaveBufLen[nIndex] = 0 ;
				//
				// 出力カウンタを補正
				hWaveMixer->m_nOutputCounter -= dwLastBytes / nBlockAlign ;
			}
			nIndex = (nIndex + 1) % 2 ;
			if ( hWaveMixer->m_nWaveBufLen[nIndex] > 0 )
			{
				//
				// 一時停止バッファに保存
				unsigned int	nLastSamples =
									hWaveMixer->m_nWaveBufLen[nIndex] ;
				tempStrmBuf.Write
					( hWaveMixer->m_pWaveBuffer[nIndex],
								nLastSamples * nBlockAlign ) ;
				hWaveMixer->m_nWaveBufLen[nIndex] = 0 ;
				//
				// 出力カウンタを補正
				hWaveMixer->m_nOutputCounter -= nLastSamples ;
			}
		}
	}
	else
	{
		//
		// ソフトウェアミキシング
		//
		unsigned int	nNextMixPos = GetCurrentMixingPosition( ) ;
		hWaveMixer->m_nPausedPosition =
			nNextMixPos + m_nOutputCounter - hWaveMixer->m_nBaseSampleIndex ;
		unsigned int	nMixSample ;
		REAL32		rVolume[2] = { 0, 0 } ;
		if ( nNextMixPos < hWaveMixer->m_nWaveBufLen[m_nPrimaryIndex] )
		{
			//
			// プライマリバッファの出力を復元
			unsigned int	nOffsetBytes =
					nNextMixPos * m_pWaveFormat->nBlockAlign ;
			nMixSample =
				hWaveMixer->m_nWaveBufLen[m_nPrimaryIndex] - nNextMixPos ;
			BYTE *	pSrcBuffer =
				(BYTE*) hWaveMixer->m_pWaveBuffer[m_nPrimaryIndex] ;
			BYTE *	pDstBuffer =
				(BYTE*) m_pWaveBuffers[m_nPrimaryIndex] ;
			::glsSound_RemixPCMSamples
				( m_pWaveFormat,
					pDstBuffer + nOffsetBytes, pSrcBuffer + nOffsetBytes,
					rVolume, pWaveBuffer->m_realVolume, nMixSample ) ;
			//
			// 一時停止バッファに保存
			tempStrmBuf.Write
				( pSrcBuffer + nOffsetBytes,
					nMixSample * m_pWaveFormat->nBlockAlign ) ;
			hWaveMixer->m_nWaveBufLen[m_nPrimaryIndex] = 0 ;
			//
			// 出力カウンタを補正
			hWaveMixer->m_nOutputCounter -= nMixSample ;
		}
		//
		unsigned int	nSecondaryIndex = (m_nPrimaryIndex + 1) % 2 ;
		if ( hWaveMixer->m_nWaveBufLen[nSecondaryIndex] > 0 )
		{
			//
			// セカンダリバッファの出力を復元
			nMixSample = hWaveMixer->m_nWaveBufLen[nSecondaryIndex] ;
			::glsSound_RemixPCMSamples
				( m_pWaveFormat,
					m_pWaveBuffers[nSecondaryIndex],
					hWaveMixer->m_pWaveBuffer[nSecondaryIndex],
					rVolume, pWaveBuffer->m_realVolume, nMixSample ) ;
			//
			// 一時停止バッファに保存
			tempStrmBuf.Write
				( hWaveMixer->m_pWaveBuffer[nSecondaryIndex],
					nMixSample * m_pWaveFormat->nBlockAlign ) ;
			hWaveMixer->m_nWaveBufLen[nSecondaryIndex] = 0 ;
			//
			// 出力カウンタを補正
			hWaveMixer->m_nOutputCounter -= nMixSample ;
		}
	}
	//
	// 一時停止バッファをストア
	EPtrBuffer	ptrbuf =
		hWaveMixer->m_PausedBuffer.GetBuffer( ) ;
	tempStrmBuf.Write( ptrbuf, ptrbuf.GetLength() ) ;
	hWaveMixer->m_PausedBuffer.Release( ptrbuf.GetLength() ) ;
	ptrbuf = tempStrmBuf.GetBuffer( ) ;
	hWaveMixer->m_PausedBuffer.Write( ptrbuf, ptrbuf.GetLength() ) ;
	//
	// 一時停止フラグを設定
	hWaveMixer->m_nPauseFlag = TRUE ;
	Unlock( ) ;
	return	eslErrSuccess ;
}

// 音声ミキシングを再開
//////////////////////////////////////////////////////////////////////////////
ESLError EWaveMixingServer::Restart( EWaveStreamBuffer * pWaveBuffer )
{
	//
	// 関数準備
	//////////////////////////////////////////////////////////////////////////
	pWaveBuffer->m_fPauseFlag = FALSE ;
	if ( m_pWaveFormat == NULL )
	{
		return	eslErrSuccess ;
	}

	Lock( ) ;
	HWAVEMIX	hWaveMixer = GetWaveMixChannel( pWaveBuffer ) ;
	if ( hWaveMixer == NULL )
	{
		Unlock( ) ;
		return	eslErrSuccess ;
	}
	if ( !hWaveMixer->m_nPauseFlag )
	{
		Unlock( ) ;
		return	eslErrSuccess ;
	}
	if ( m_idsound != NULL )
	{
		//
		// DirectSound へ出力
		//////////////////////////////////////////////////////////////////////
		hWaveMixer->PlayDirectSound( ) ;
	}
	else
	{
		//
		// 現在の音声バッファへミックス
		//////////////////////////////////////////////////////////////////////
		//
		// 先頭のミキシングバッファを準備
		unsigned int	nLenSamples = m_nBufferingSize;
		//
		// 現在のミキシング位置を取得
		unsigned int	nNextMixPos = GetCurrentMixingPosition( ) ;
		hWaveMixer->m_nBaseSampleIndex =
			m_nOutputCounter + nNextMixPos - hWaveMixer->m_nPausedPosition ;
		//
		// プライマリ音声バッファへミックス
		unsigned int	nOffsetPos, nMixSample ;
		void *			pWaveMixBuf ;
		if ( m_nPreparedSize[m_nPrimaryIndex] > nNextMixPos )
		{
			nOffsetPos = nNextMixPos ;
			nMixSample = m_nPreparedSize[m_nPrimaryIndex] - nOffsetPos ;
			pWaveMixBuf = hWaveMixer->GetNextWaveSamples
				( m_nPrimaryIndex, nOffsetPos, nMixSample, m_pWaveFormat ) ;
			if ( pWaveMixBuf != NULL )
			{
				::glsSound_MixPCMSamples
					( m_pWaveFormat,
						((BYTE*)m_pWaveBuffers[m_nPrimaryIndex])
							+ nOffsetPos * m_pWaveFormat->nBlockAlign,
						pWaveMixBuf, pWaveBuffer->m_realVolume, nMixSample ) ;
			}
		}
		//
		// セカンダリ音声バッファへミックス
		unsigned int	nSecondaryIndex =
							(m_nPrimaryIndex + 1) % 2 ;
		nOffsetPos = 0 ;
		nMixSample = m_nPreparedSize[nSecondaryIndex] - nOffsetPos ;
		pWaveMixBuf = hWaveMixer->GetNextWaveSamples
			( nSecondaryIndex, nOffsetPos, nMixSample, m_pWaveFormat ) ;
		if ( pWaveMixBuf != NULL )
		{
			::glsSound_MixPCMSamples
				( m_pWaveFormat, m_pWaveBuffers[nSecondaryIndex],
					pWaveMixBuf, pWaveBuffer->m_realVolume, nMixSample ) ;
		}
	}
	//
	// 一時停止フラグをクリア
	hWaveMixer->m_nPauseFlag = FALSE ;
	Unlock( ) ;
	return	eslErrSuccess ;
}

// 音声出力デバイスを一時停止
//////////////////////////////////////////////////////////////////////////////
ESLError EWaveMixingServer::PauseDevice( void )
{
	ESLError	err ;
	if ( m_idsound != NULL )
	{
		if ( m_idsbuf != NULL )
		{
			m_idsbuf->Stop( ) ;
		}
	}
	else if ( m_pVirtualBuf == NULL )
	{
		err = EWaveOutDevice::PauseDevice( ) ;
		m_dwDevPausedTime = ::timeGetTime( ) ;
		if ( m_dwDevPausedTime == 0 )
		{
			m_dwDevPausedTime = 1 ;
		}
	}
	return	err ;
}

// 音声出力デバイスの出力を再開
//////////////////////////////////////////////////////////////////////////////
ESLError EWaveMixingServer::RestartDevice( void )
{
	ESLError	err ;
	if ( m_idsound != NULL )
	{
		if ( m_idsbuf != NULL )
		{
			m_idsbuf->Play( 0, 0, DSBPLAY_LOOPING ) ;
		}
	}
	else if ( m_pVirtualBuf == NULL )
	{
		err = EWaveOutDevice::RestartDevice( ) ;
		if ( m_dwDevPausedTime != 0 )
		{
			m_dwBaseBufferTime += (::timeGetTime() - m_dwDevPausedTime) ;
			m_dwDevPausedTime = 0 ;
		}
	}
	return	err ;
}

// 現在の再生位置を取得
//////////////////////////////////////////////////////////////////////////////
UINT64 EWaveMixingServer::GetCurrentSample
		( const EWaveStreamBuffer * pWaveBuffer )
{
	if ( m_pWaveFormat == NULL )
	{
		ESLTrace( "音声出力デバイスが開かれていません。\n" ) ;
		return	0 ;
	}
	Lock( ) ;
	HWAVEMIX	hWaveMixer = GetWaveMixChannel( pWaveBuffer ) ;
	if ( hWaveMixer == NULL )
	{
		Unlock( ) ;
		return	0 ;
	}

	UINT64	nSampleIndex = 0 ;
	if ( hWaveMixer->m_nPauseFlag )
	{
		// hWaveMixer が一時停止されている時には
		// 現在の再生位置（サンプル数）は m_nPausedPosition
		nSampleIndex = hWaveMixer->m_nPausedPosition ;
	}
	else
	{
		// hWaveMixer が再生中の時には、オフセット位置を計算
		if ( m_idsound != NULL )
		{
			nSampleIndex = hWaveMixer->GetCurrentDirectSoundPosition( ) ;
		}
		else if ( m_pVirtualBuf != NULL )
		{
			nSampleIndex =
				m_nOutputCounter
					+ m_dwVirtualLocalPos
					- hWaveMixer->m_nBaseSampleIndex ;
			if ( (INT64) nSampleIndex < 0 )
			{
				nSampleIndex = 0 ;
			}
		}
		else
		{
			DWORD	dwOffsetTime = ::timeGetTime() - m_dwBaseBufferTime ;
			if ( m_dwDevPausedTime != 0 )
			{
				dwOffsetTime = m_dwDevPausedTime - m_dwBaseBufferTime ;
			}
			nSampleIndex =
				((UINT64) dwOffsetTime * m_pWaveFormat->nSamplesPerSec / 1000)
					+ m_nOutputCounter - hWaveMixer->m_nBaseSampleIndex ;
			if ( (INT64) nSampleIndex < 0 )
			{
				nSampleIndex = 0 ;
			}
//			else if ( nSampleIndex > m_nOutputCounter )
//				nSampleIndex = m_nOutputCounter ;
		}
	}

	// サンプリング周波数に変換
	UINT64	nCurrentSample =
		nSampleIndex
			* hWaveMixer->m_WaveFormat.nSamplesPerSec
			/ m_pWaveFormat->nSamplesPerSec ;

	Unlock( ) ;
	return	nCurrentSample ;
}

// デバイスの音量を設定
//////////////////////////////////////////////////////////////////////////////
ESLError EWaveMixingServer::SetTotalVolume( const unsigned int nVolume[] )
{
	m_nTotalVolume[0] = nVolume[0] ;
	if ( m_nTotalVolume[0] > 0xFFFF )
	{
		m_nTotalVolume[0] = 0xFFFF ;
	}
	m_nTotalVolume[1] = nVolume[1] ;
	if ( m_nTotalVolume[1] > 0xFFFF )
	{
		m_nTotalVolume[1] = 0xFFFF ;
	}

	if ( m_idsound != NULL )
	{
		Lock( ) ;
		for ( unsigned int i = 0; i < m_listWaveChannel.GetSize(); i ++ )
		{
			WAVE_MIXER_CHANNEL *	pwmixch = m_listWaveChannel.GetAt( i ) ;
			if ( pwmixch != NULL )
			{
				pwmixch->SetDirectSoundVolume( ) ;
			}
		}
		Unlock( ) ;
	}
	else if ( m_pVirtualBuf != NULL )
	{
	}
	else if ( m_pWaveFormat != NULL )
	{
		if ( ::waveOutSetVolume( m_hWaveOut,
			(m_nTotalVolume[0] | ((m_nTotalVolume[1]) << 16)) ) )
		{
			ESLTrace( "音声出力デバイスの音量を設定できませんでした。\n" ) ;
		}
	}
	return	eslErrSuccess ;
}

// チャネル音量を設定
//////////////////////////////////////////////////////////////////////////////
ESLError EWaveMixingServer::SetVolume
	( EWaveStreamBuffer * pWaveBuffer, const REAL32 nVolume[] )
{
	//
	// 関数準備
	//////////////////////////////////////////////////////////////////////////
	if ( m_pWaveFormat == NULL )
	{
		return	ESLErrorMsg( "音声出力デバイスが開かれていません。" ) ;
	}

	Lock( ) ;
	HWAVEMIX	hWaveMixer = GetWaveMixChannel( pWaveBuffer ) ;
	if ( hWaveMixer == NULL )
	{
		pWaveBuffer->m_realVolume[0] = nVolume[0] ;
		pWaveBuffer->m_realVolume[1] = nVolume[1] ;
		Unlock( ) ;
		return	eslErrSuccess ;
	}

	//
	// DirectSoundBuffer の音量を設定
	//////////////////////////////////////////////////////////////////////////
	if ( m_idsound != NULL )
	{
		pWaveBuffer->m_realVolume[0] = nVolume[0] ;
		pWaveBuffer->m_realVolume[1] = nVolume[1] ;
		//
		hWaveMixer->SetDirectSoundVolume( ) ;
		//
		Unlock( ) ;
		return	eslErrSuccess ;
	}

	//
	// 音声バッファの音量を補正
	//////////////////////////////////////////////////////////////////////////
	unsigned int	nNextMixPos = GetCurrentMixingPosition( ) ;
	unsigned int	nMixSample ;
	if ( nNextMixPos < hWaveMixer->m_nWaveBufLen[m_nPrimaryIndex] )
	{
		//
		// プライマリ音声バッファを補正
		unsigned int	nOffsetBytes =
				nNextMixPos * m_pWaveFormat->nBlockAlign ;
		nMixSample =
			hWaveMixer->m_nWaveBufLen[m_nPrimaryIndex] - nNextMixPos ;
		BYTE *	pSrcBuffer =
			(BYTE*) hWaveMixer->m_pWaveBuffer[m_nPrimaryIndex] ;
		BYTE *	pDstBuffer =
			(BYTE*) m_pWaveBuffers[m_nPrimaryIndex] ;
		::glsSound_RemixPCMSamples
			( m_pWaveFormat,
				pDstBuffer + nOffsetBytes, pSrcBuffer + nOffsetBytes,
				nVolume, pWaveBuffer->m_realVolume, nMixSample ) ;
	}
	//
	// セカンダリ音声バッファを補正
	unsigned int	nSecondaryIndex = (m_nPrimaryIndex + 1) % 2 ;
	if ( hWaveMixer->m_nWaveBufLen[nSecondaryIndex] > 0 )
	{
		::glsSound_RemixPCMSamples
			( m_pWaveFormat,
				m_pWaveBuffers[nSecondaryIndex],
				hWaveMixer->m_pWaveBuffer[nSecondaryIndex],
				nVolume, pWaveBuffer->m_realVolume,
				hWaveMixer->m_nWaveBufLen[nSecondaryIndex] ) ;
	}
	//
	// 音量を設定
	//////////////////////////////////////////////////////////////////////////
	pWaveBuffer->m_realVolume[0] = nVolume[0] ;
	pWaveBuffer->m_realVolume[1] = nVolume[1] ;

	Unlock( ) ;
	return	eslErrSuccess ;
}

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
DWORD EWaveMixingServer::WaveCallbackThread( void )
{
	::SetThreadPriority( ::GetCurrentThread(), THREAD_PRIORITY_HIGHEST );
	m_nTimerID = ::SetTimer( NULL, 0, m_nQuantumTime, NULL ) ;

	MSG		msg ;
	while ( ::GetMessage( &msg, NULL, 0, 0 ) )
	{
		if ( (msg.hwnd == NULL) && (msg.message == WM_QUIT) )
		{
			break ;
		}
		else if ( (msg.hwnd == NULL) && (msg.message == WM_TIMER) )
		{
			OnMixingTimer( ) ;
			//
			if ( m_idsound == NULL )
			{
				if ( m_fCallbackMode == -1 )
				{
					if ( m_WaveHeaders[m_nPrimaryIndex].dwFlags & WHDR_DONE )
					{
						OnBufferPlayed( &(m_WaveHeaders[m_nPrimaryIndex]) ) ;
					}
				}
				else if ( (m_fCallbackMode == 0)
						&& (m_WaveHeaders[0].dwFlags & WHDR_DONE)
						&& (m_WaveHeaders[1].dwFlags & WHDR_DONE) )
				{
					OnBufferPlayed( &(m_WaveHeaders[m_nPrimaryIndex]) ) ;
					m_fCallbackMode = -1 ;
				}
			}
		}
		else if ( (msg.hwnd == NULL) && (msg.message == WAVEMSG_PLAYED) )
		{
			if ( m_fCallbackMode != -1 )
			{
				OnBufferPlayed( (HWAVEBUF) msg.wParam ) ;
				m_fCallbackMode = 1 ;
			}
		}
		else
		{
			::TranslateMessage( &msg ) ;
			::DispatchMessage( &msg ) ;
		}
	}

	return	0 ;
}

// 音声バッファの再生が終了した
//////////////////////////////////////////////////////////////////////////////
void EWaveMixingServer::OnBufferPlayed( HWAVEBUF hWaveBuf )
{
	//
	// プライマリバッファの指標を取得
	//////////////////////////////////////////////////////////////////////////
	unsigned int	nSecondaryIndex ;
	if ( hWaveBuf == &m_WaveHeaders[0] )
		nSecondaryIndex = 0 ;
	else
		nSecondaryIndex = 1 ;

	//
	// プライマリバッファをクリア
	//////////////////////////////////////////////////////////////////////////
	Lock( ) ;
	m_nPrimaryIndex = (nSecondaryIndex + 1) % 2 ;
	m_nPreparedSize[nSecondaryIndex] = 0 ;
	//
	for ( unsigned int i = 0; i < m_listWaveChannel.GetSize(); i ++ )
	{
		WAVE_MIXER_CHANNEL *	pMixChannel = m_listWaveChannel.GetAt( i ) ;
		if ( pMixChannel != NULL )
		{
			pMixChannel->m_nWaveBufLen[nSecondaryIndex] = 0 ;
		}
	}

	if ( m_pWaveFormat != NULL )
	{
		//
		// プライマリバッファの準備
		//////////////////////////////////////////////////////////////////////
		if ( m_nPreparedSize[m_nPrimaryIndex] < m_nBufferingSize )
		{
			PrepareNextMixingBuffer( m_nPrimaryIndex,
				(m_nBufferingSize - m_nPreparedSize[m_nPrimaryIndex]) ) ;
		}

		//
		// セカンダリバッファの準備
		//////////////////////////////////////////////////////////////////////
		if ( ::waveOutUnprepareHeader
			( m_hWaveOut, &m_WaveHeaders[nSecondaryIndex],
							sizeof(WAVEHDR) ) != MMSYSERR_NOERROR )
		{
			ESLTrace( "waveOutUnprepareHeader 関数が失敗しました。\n" ) ;
			::memset( &m_WaveHeaders[nSecondaryIndex], 0, sizeof(WAVEHDR) ) ;
		}
		::waveOutPrepareHeader
			( m_hWaveOut, &m_WaveHeaders[nSecondaryIndex], sizeof(WAVEHDR) ) ;
		if ( ::waveOutWrite
			( m_hWaveOut, &m_WaveHeaders[nSecondaryIndex],
						sizeof(WAVEHDR) ) != MMSYSERR_NOERROR )
		{
			ESLTrace( "waveOutWrite 関数が失敗しました。\n" ) ;
		}
		//
		m_nOutputCounter += m_nBufferingSize ;
		m_dwBaseBufferTime = ::timeGetTime( ) ;
		unsigned int	nCurrentPos = GetCurrentMixingPosition( ) ;
		if ( nCurrentPos > 0 )
		{
			nCurrentPos = (nCurrentPos + 0x07) & ~0x07 ;
			if ( nCurrentPos > m_nBufferingSize )
			{
				nCurrentPos = m_nBufferingSize ;
			}
			PrepareNextMixingBuffer( nSecondaryIndex, nCurrentPos ) ;
		}
	}
	else
	{
		//
		// 音声バッファはミキシング中ではない
		//////////////////////////////////////////////////////////////////////
		m_boolEmptyWaveBuf[nSecondaryIndex] = TRUE ;
		if ( m_boolEmptyWaveBuf[0] && m_boolEmptyWaveBuf[1] )
		{
			::SetEvent( m_eventEmptyWaveBuf ) ;
		}
	}
	Unlock( ) ;
}

// ミキシングタイマーコールバック
//////////////////////////////////////////////////////////////////////////////
void EWaveMixingServer::OnMixingTimer( void )
{
	Lock( ) ;
	if ( m_idsound != NULL )
	{
		//
		// DirectSound ストリーミング処理
		//////////////////////////////////////////////////////////////////////
		//
		// バッファロストチェック
		//
		if ( m_idsbuf != NULL )
		{
			DWORD	dwStatus ;
			if ( m_idsbuf->GetStatus( &dwStatus ) == DS_OK )
			{
				if ( dwStatus & DSBSTATUS_BUFFERLOST )
				{
					if ( m_idsbuf->Restore() != DS_OK )
					{
					}
				}
			}
		}
		for ( unsigned int i = 0; i < m_listWaveChannel.GetSize(); i ++ )
		{
			WAVE_MIXER_CHANNEL *	pwmixch = m_listWaveChannel.GetAt( i ) ;
			if ( pwmixch != NULL )
			{
				//
				// チャネルごとのストリーミング処理
				//
				pwmixch->OnTimerDirectSoundStreaming( ) ;
				//
				// 再生終了バッファの判定
				//
				UINT64	nCurrentPos = pwmixch->GetCurrentDirectSoundPosition( ) ;
				unsigned int	nWasteCount = pwmixch->m_WasteWaveHdr.GetSize( ) ;
				nCurrentPos =
					nCurrentPos * pwmixch->m_WaveFormat.nSamplesPerSec
										/ m_pWaveFormat->nSamplesPerSec ;
				for ( unsigned int j = 0; j < nWasteCount; j ++ )
				{
					WAVEHDR *	lpWaveHdr = pwmixch->m_WasteWaveHdr.GetAt( j ) ;
					if ( lpWaveHdr->dwUser )
					{
						// この WAVEHDR は再生完了した
						pwmixch->m_pWaveBufObj->OnEndPlaying
							( lpWaveHdr, lpWaveHdr->lpData,
									lpWaveHdr->dwBufferLength ) ;
						pwmixch->m_WasteWaveHdr.RemoveAt( j -- ) ;
						nWasteCount = pwmixch->m_WasteWaveHdr.GetSize( ) ;
						UnprepareBuffer( lpWaveHdr ) ;
					}
					else if ( nCurrentPos >= *((UINT64*)&lpWaveHdr->dwFlags) + m_nBufferingSize )
					{
						lpWaveHdr->dwUser = 1 ;
					}
				}
				//
				// ミキサーチャネルの遅延削除
				//
				if ( (pwmixch->m_pNextWaveHdr == NULL)
						&& (pwmixch->m_WasteWaveHdr.GetSize() == 0) )
				{
					pwmixch->Stop( ) ;
					m_listWaveChannel.RemoveAt( i ) ;
					i -- ;
				}
			}
			else
			{
				pwmixch->Stop( ) ;
				m_listWaveChannel.RemoveAt( i ) ;
				i -- ;
			}
		}
	}
	else if ( m_pWaveFormat != NULL )
	{
		//
		// プライマリバッファを準備
		//////////////////////////////////////////////////////////////////////
		if ( m_nPreparedSize[m_nPrimaryIndex] < m_nBufferingSize )
		{
			PrepareNextMixingBuffer( m_nPrimaryIndex,
				(m_nBufferingSize - m_nPreparedSize[m_nPrimaryIndex]) ) ;
		}

		//
		// セカンダリバッファを準備
		//////////////////////////////////////////////////////////////////////
		unsigned int	nSecondaryIndex = (m_nPrimaryIndex + 1) % 2 ;
		unsigned int	nCurrentPos = GetCurrentMixingPosition( ) ;
		nCurrentPos = (nCurrentPos + 0x07) & ~0x07 ;
		if ( nCurrentPos > m_nBufferingSize )
		{
			nCurrentPos = m_nBufferingSize ;
		}
		if ( nCurrentPos > m_nPreparedSize[nSecondaryIndex] )
		{
			PrepareNextMixingBuffer( nSecondaryIndex,
				(nCurrentPos - m_nPreparedSize[nSecondaryIndex]) ) ;
		}
	}
	Unlock( ) ;
}

// 次のミキシングバッファを準備
//////////////////////////////////////////////////////////////////////////////
void EWaveMixingServer::PrepareNextMixingBuffer
	( unsigned int nBufIndex, unsigned int nOutputSamples )
{
	//
	// nOutputSamples を正規化
	//////////////////////////////////////////////////////////////////////////
	Lock( ) ;
	if ( (m_nPreparedSize[nBufIndex] + nOutputSamples) > m_nBufferingSize )
	{
		nOutputSamples = m_nBufferingSize - m_nPreparedSize[nBufIndex] ;
		if ( (signed int) nOutputSamples < 0 )
			nOutputSamples = 0 ;
	}
	if ( (signed int) nOutputSamples <= 0 )
	{
		Unlock( ) ;
		return ;
	}

	//
	// 関数準備
	//////////////////////////////////////////////////////////////////////////
	signed int	i, nChannels ;
	WAVEHDR *	lpWaveHdr ;
	void *	ptrWaveMixBuffer =
		(void*)((BYTE*)m_pWaveBuffers[nBufIndex]
			+ m_nPreparedSize[nBufIndex] * m_pWaveFormat->nBlockAlign) ;

	//
	// ミキシング準備
	//////////////////////////////////////////////////////////////////////////
	::glsSound_CleanSoundBuffer
		(
			m_pWaveFormat,
			ptrWaveMixBuffer,
			nOutputSamples
		) ;
	nChannels = m_listWaveChannel.GetSize( ) ;

	//
	// 音声ストリームミキシング
	//////////////////////////////////////////////////////////////////////////
	for ( i = (nChannels - 1); i >= 0; i -- )
	{
		WAVE_MIXER_CHANNEL *	lpMixer = m_listWaveChannel.GetAt( i ) ;
		if ( (lpMixer != NULL) && !(lpMixer->m_nPauseFlag) )
		{
			//
			// 次の音声データを取得
			//////////////////////////////////////////////////////////////////
			unsigned int	nGetSamples = nOutputSamples ;
			void *	ptrBuffer = lpMixer->GetNextWaveSamples
				(
					nBufIndex,
					m_nPreparedSize[nBufIndex],
					nGetSamples,
					m_pWaveFormat
				) ;
			//
			// PCM サンプルをミックス
			//////////////////////////////////////////////////////////////////
			if ( ptrBuffer != NULL )
			{
				::glsSound_MixPCMSamples
					(
						m_pWaveFormat,
						ptrWaveMixBuffer,
						ptrBuffer,
						lpMixer->m_pWaveBufObj->m_realVolume,
						nGetSamples
					) ;
			}
		}

		//
		// 再生終了バッファの判定
		//////////////////////////////////////////////////////////////////
		UINT64	nCurrentPos = GetCurrentSample( lpMixer->m_pWaveBufObj ) ;
		unsigned int	nWasteCount = lpMixer->m_WasteWaveHdr.GetSize( ) ;
		for ( unsigned int j = 0; j < nWasteCount; j ++ )
		{
			lpWaveHdr = lpMixer->m_WasteWaveHdr.GetAt( j ) ;
			if ( lpWaveHdr->dwUser )
			{
				// この WAVEHDR は再生完了した
				lpMixer->m_pWaveBufObj->OnEndPlaying
					( lpWaveHdr, lpWaveHdr->lpData,
							lpWaveHdr->dwBufferLength ) ;
				lpMixer->m_WasteWaveHdr.RemoveAt( j -- ) ;
				nWasteCount = lpMixer->m_WasteWaveHdr.GetSize( ) ;
				UnprepareBuffer( lpWaveHdr ) ;
			}
			else if ( nCurrentPos >= *((UINT64*)&lpWaveHdr->dwFlags) )
			{
				lpWaveHdr->dwUser = 1 ;
			}
		}

		//
		// ミキサーチャネルの遅延削除
		//////////////////////////////////////////////////////////////////
		if ( (lpMixer->m_pNextWaveHdr == NULL)
				&& (lpMixer->m_WasteWaveHdr.GetSize() == 0) )
		{
			lpMixer->Stop( ) ;
			m_listWaveChannel.RemoveAt( i ) ;
		}
	}

	// ポインタを進める
	m_nPreparedSize[nBufIndex] += nOutputSamples ;
	ESLAssert( m_nPreparedSize[nBufIndex] <= m_nBufferingSize ) ;

	Unlock( ) ;
}

// 音声仮想出力処理
//////////////////////////////////////////////////////////////////////////////
ESLError EWaveMixingServer::AdvanceVirtualPlay( DWORD dwSamples )
{
	if ( m_pVirtualBuf == NULL )
	{
		return	eslErrGeneral ;
	}
	while ( dwSamples )
	{
		//
		// 今ループでの処理サンプル数を計算
		//
		ESLAssert( m_dwVirtualLocalPos < m_nBufferingSize ) ;
		DWORD	dwAdvance = m_nBufferingSize - m_dwVirtualLocalPos ;
		if ( dwAdvance > dwSamples )
		{
			dwAdvance = dwSamples ;
		}
		dwSamples -= dwAdvance ;
		m_dwVirtualLocalPos += dwAdvance ;
		//
		// プライマリバッファが未処理の場合には準備
		//
		if ( m_nPreparedSize[m_nPrimaryIndex] < m_nBufferingSize )
		{
			PrepareNextMixingBuffer( m_nPrimaryIndex,
				(m_nBufferingSize - m_nPreparedSize[m_nPrimaryIndex]) ) ;
		}
		//
		// セカンダリバッファを準備
		//
		unsigned int	nSecondaryIndex = (m_nPrimaryIndex + 1) % 2 ;
		if ( m_dwVirtualLocalPos > m_nPreparedSize[nSecondaryIndex] )
		{
			PrepareNextMixingBuffer( nSecondaryIndex,
				(m_dwVirtualLocalPos - m_nPreparedSize[nSecondaryIndex]) ) ;
		}
		//
		// バッファ完了処理
		//
		if ( m_dwVirtualLocalPos >= m_nBufferingSize )
		{
			OnVirtualBufferPlayed( ) ;
		}
	}
	return	eslErrSuccess ;
}

// 音声バッファ仮想再生終了処理
//////////////////////////////////////////////////////////////////////////////
void EWaveMixingServer::OnVirtualBufferPlayed( void )
{
	if ( (m_pWaveFormat == NULL) || (m_pVirtualBuf == NULL) )
	{
		return ;
	}
	//
	// 完了バッファを出力
	//
	m_pVirtualBuf->Write
		( m_pWaveBuffers[m_nPrimaryIndex],
			m_nBufferingSize * m_pWaveFormat->nBlockAlign ) ;
	//
	// 完了バッファをクリアに設定
	//
	unsigned int	nSecondaryIndex = m_nPrimaryIndex ;
	m_nPrimaryIndex = (nSecondaryIndex + 1) % 2 ;
	m_nPreparedSize[nSecondaryIndex] = 0 ;
	//
	for ( unsigned int i = 0; i < m_listWaveChannel.GetSize(); i ++ )
	{
		WAVE_MIXER_CHANNEL *	pMixChannel = m_listWaveChannel.GetAt( i ) ;
		if ( pMixChannel != NULL )
		{
			pMixChannel->m_nWaveBufLen[nSecondaryIndex] = 0 ;
		}
	}
	//
	// バッファ準備
	//
	m_nOutputCounter += m_nBufferingSize ;
	m_dwVirtualLocalPos = 0 ;
	m_dwBaseBufferTime = ::timeGetTime( ) ;
}

// 仮想（無音）出力、タイミング同期処理用スレッド
//////////////////////////////////////////////////////////////////////////////
DWORD WINAPI EWaveMixingServer::VirtualSyncThreadProc( LPVOID lpParam )
{
	EWaveMixingServer *	pwmsDev = (EWaveMixingServer*) lpParam ;
	DWORD	dwResult ;
	//
	::glsInitializeTask( ) ;
	//
	dwResult = pwmsDev->VirtualSyncThread( ) ;
	//
	::glsCloseTask( ) ;
	//
	return	dwResult ;
}

DWORD WINAPI EWaveMixingServer::VirtualSyncThread( void )
{
	MSG		msg ;
	::PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) ;
	ESLAssert( m_hThreadCreated != NULL ) ;
	::SetEvent( m_hThreadCreated ) ;
	//
	DWORD	dwLastTime = ::timeGetTime() ;
	DWORD	dwOddSamples = 0 ;
	for ( ; ; )
	{
		if ( ::PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
		{
			if ( (msg.hwnd == NULL) && (msg.message == WM_QUIT) )
			{
				break ;
			}
		}
		//
		Lock() ;
		if ( m_pWaveFormat != NULL )
		{
			DWORD	dwCurrentTime = ::timeGetTime() ;
			DWORD	dwPastTime = dwCurrentTime - dwLastTime ;
			//
			UINT64	nSamples_x1000 =
				(UINT64) dwPastTime * m_pWaveFormat->nSamplesPerSec + dwOddSamples ;
			DWORD	dwSamples = (DWORD) (nSamples_x1000 / 1000) ;
			//
			if ( dwSamples >= 128 )
			{
				dwLastTime = dwCurrentTime ;
				dwOddSamples = (DWORD) (nSamples_x1000 % 1000) ;
				//
				AdvanceVirtualPlay( dwSamples ) ;
				//
				if ( m_pVirtualBuf != NULL )
				{
					while ( m_pVirtualBuf->GetLength() > 0 )
					{
						EPtrBuffer	ptrbuf =
							m_pVirtualBuf->GetBuffer( 0x4000 ) ;
						m_pVirtualBuf->Release( ptrbuf.GetLength() ) ;
					}
				}
			}
		}
		Unlock() ;
		::Sleep( 30 ) ;
	}
	//
	return	0 ;
}



/*****************************************************************************
						音声バッファオブジェクト
 ****************************************************************************/

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( EWaveStreamBuffer, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
EWaveStreamBuffer::EWaveStreamBuffer( void )
	: m_nFormatSize( 0 ), m_pWaveFormat( NULL ), m_fPauseFlag( 0 )
{
	m_realVolume[0] = (REAL32) 1.0 ;
	m_realVolume[1] = (REAL32) 1.0 ;
	m_pWaveDevMixingBuf = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
EWaveStreamBuffer::~EWaveStreamBuffer( void )
{
	if ( m_pWaveFormat != NULL )
	{
		::eslHeapFree( NULL, m_pWaveFormat ) ;
	}
	delete	m_pWaveDevMixingBuf ;
}

// 音声データの再生が完了した
//////////////////////////////////////////////////////////////////////////////
void EWaveStreamBuffer::OnEndPlaying
( HWAVEBUF hWaveBuf, void * ptrBuffer, unsigned int nBufferLength )
{
}

// 音声出力デバイスが次の音声バッファを要求している
//////////////////////////////////////////////////////////////////////////////
HWAVEBUF EWaveStreamBuffer::OnQueueNextBuffer( EWaveOutDevice * pWaveDev )
{
	return	NULL ;
}

// 音声フォーマット取得
//////////////////////////////////////////////////////////////////////////////
const WAVEFORMATEX * EWaveStreamBuffer::GetWaveFormat( void ) const
{
	return	m_pWaveFormat ;
}

// 音声フォーマットを設定
//////////////////////////////////////////////////////////////////////////////
ESLError EWaveStreamBuffer::SetWaveFormat( const WAVEFORMATEX * pwfx )
{
	delete	m_pWaveDevMixingBuf ;
	m_pWaveDevMixingBuf = NULL ;
	//
	if ( m_pWaveFormat != NULL )
	{
		::eslHeapFree( NULL, m_pWaveFormat ) ;
	}
	m_pWaveFormat = NULL ;
	m_nFormatSize = 0 ;
	if ( pwfx != NULL )
	{
		m_nFormatSize = sizeof(WAVEFORMATEX) ;
		if ( pwfx->wFormatTag != WAVE_FORMAT_PCM )
		{
			m_nFormatSize += pwfx->cbSize ;
		}
		m_pWaveFormat =
			(WAVEFORMATEX*)::eslHeapAllocate( NULL, m_nFormatSize, 0 ) ;
		::memmove( m_pWaveFormat, pwfx, m_nFormatSize ) ;
	}
	return	eslErrSuccess ;
}

// Sample - time translation
//////////////////////////////////////////////////////////////////////////////
unsigned int EWaveStreamBuffer::TimeToSample
	( const WAVEFORMATEX * pwfx, unsigned int nMilliSecond )
{
	return	(unsigned int)
		((INT64) nMilliSecond * pwfx->nSamplesPerSec / 1000) ;
}

unsigned int EWaveStreamBuffer::SampleToTime
	( const WAVEFORMATEX * pwfx, unsigned int nSamples )
{
	return	(unsigned int)
		((INT64) nSamples * 1000 / pwfx->nSamplesPerSec) ;
}

unsigned int EWaveStreamBuffer::TimeToSample( unsigned int nMilliSecond ) const
{
	if ( m_pWaveFormat != NULL )
		return	TimeToSample( m_pWaveFormat, nMilliSecond ) ;
	else
		return	0 ;
}

unsigned int EWaveStreamBuffer::SampleToTime( unsigned int nSamples ) const
{
	if ( m_pWaveFormat != NULL )
		return	SampleToTime( m_pWaveFormat, nSamples ) ;
	else
		return	0 ;
}

// PCM sample - raw bytes size translation
//////////////////////////////////////////////////////////////////////////////
unsigned int EWaveStreamBuffer::SampleToBytes
	( const WAVEFORMATEX * pwfx, unsigned int nSamples ) const
{
	unsigned long int	nRawBytes ;
	if ( pwfx->wFormatTag != WAVE_FORMAT_PCM )
	{
		// Open ACM stream
		HACMSTREAM	hACMStream ;
		WAVEFORMATEX	wfxDst = *pwfx ;
		wfxDst.wFormatTag = WAVE_FORMAT_PCM ;
		wfxDst.wBitsPerSample = 16 ;
		wfxDst.nBlockAlign = (WORD)(wfxDst.nChannels << 1) ;
		wfxDst.nAvgBytesPerSec =
			wfxDst.nSamplesPerSec * wfxDst.nBlockAlign ;
		nRawBytes = nSamples * wfxDst.nChannels * wfxDst.wBitsPerSample >> 3 ;
		WAVEFORMATEX *	pwfxSrc ;
		unsigned int	nFormatSize = sizeof(WAVEFORMATEX) ;
		if ( pwfx->wFormatTag != WAVE_FORMAT_PCM )
			nFormatSize += pwfx->cbSize ;
		pwfxSrc = (WAVEFORMATEX*)::eslHeapAllocate( NULL, nFormatSize, FALSE ) ;
		::memmove( pwfxSrc, pwfx, nFormatSize ) ;
		if ( ::acmStreamOpen( &hACMStream, NULL,
			pwfxSrc, &wfxDst, NULL, NULL, 0, ACM_STREAMOPENF_NONREALTIME ) )
		{
			// Error
			nRawBytes = 0 ;
		}
		else
		{
			// Get source size
			::acmStreamSize( hACMStream,
				nRawBytes, &nRawBytes, ACM_STREAMSIZEF_DESTINATION ) ;
			// Close ACM stream
			::acmStreamClose( hACMStream, 0 ) ;
		}
		::eslHeapFree( NULL, pwfxSrc ) ;
	}
	else
	{
		nRawBytes =
			nSamples * pwfx->nChannels * pwfx->wBitsPerSample >> 3 ;
	}
	return	nRawBytes ;
}

unsigned int EWaveStreamBuffer::BytesToSample
	( const WAVEFORMATEX * pwfx, unsigned int nBytes ) const
{
	unsigned long int	nRawSamples ;
	if ( pwfx->wFormatTag != WAVE_FORMAT_PCM )
	{
		// With ACM
		// Open ACM stream
		HACMSTREAM	hACMStream ;
		WAVEFORMATEX	wfxDst = *pwfx ;
		wfxDst.wFormatTag = WAVE_FORMAT_PCM ;
		wfxDst.wBitsPerSample = 16 ;
		wfxDst.nBlockAlign = (WORD)(wfxDst.nChannels << 1) ;
		wfxDst.nAvgBytesPerSec =
			wfxDst.nSamplesPerSec * wfxDst.nBlockAlign ;
		wfxDst.cbSize = 0 ;
		WAVEFORMATEX *	pwfxSrc ;
		unsigned int	nFormatSize = sizeof(WAVEFORMATEX) ;
		if ( pwfx->wFormatTag != WAVE_FORMAT_PCM )
			nFormatSize += pwfx->cbSize ;
		pwfxSrc = (WAVEFORMATEX*)::eslHeapAllocate( NULL, nFormatSize, FALSE ) ;
		::memmove( pwfxSrc, pwfx, nFormatSize ) ;
		if ( ::acmStreamOpen( &hACMStream, NULL,
			pwfxSrc, &wfxDst, NULL, NULL, 0, ACM_STREAMOPENF_NONREALTIME ) )
		{
			// Error
			nRawSamples = 0 ;
		}
		else
		{
			// Get source size
			::acmStreamSize( hACMStream,
				nBytes, &nRawSamples, ACM_STREAMSIZEF_SOURCE ) ;
			nRawSamples /= (wfxDst.nChannels * wfxDst.wBitsPerSample >> 3) ;
			// Close ACM stream
			::acmStreamClose( hACMStream, 0 ) ;
		}
		::eslHeapFree( NULL, pwfxSrc ) ;
	}
	else
	{
		nRawSamples = nBytes / (pwfx->nChannels * pwfx->wBitsPerSample >> 3) ;
	}
	return	nRawSamples ;
}

unsigned int EWaveStreamBuffer::SampleToBytes( unsigned int nSamples ) const
{
	if ( m_pWaveFormat != NULL )
		return	SampleToBytes( m_pWaveFormat, nSamples ) ;
	else
		return	0 ;
}

unsigned int EWaveStreamBuffer::BytesToSample( unsigned int nBytes ) const
{
	if ( m_pWaveFormat != NULL )
		return	BytesToSample( m_pWaveFormat, nBytes ) ;
	else
		return	0 ;
}


/*****************************************************************************
						生音声データオブジェクト
 ****************************************************************************/

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( EWaveSound, EWaveStreamBuffer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
EWaveSound::EWaveSound( EWaveOutDevice * pWaveDev )
	: m_pWaveDevice( pWaveDev ),
		m_hDoneEvent( NULL ), m_hwndNotifyDone( NULL ),
		m_flagRepeat( false ), m_flagPlaying( false ),
		m_fBufDelete( false ), m_pWaveBuffer( NULL ), m_nWaveLength( 0 ),
		m_hLastWaveBuf( NULL ), m_nLastStartedPos( 0 ), m_nOutputBais( 0 )
{
	::InitializeCriticalSection( &m_cs );
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
EWaveSound::~EWaveSound( void )
{
	Delete( ) ;
	::DeleteCriticalSection( &m_cs ) ;
}

// 音声データの再生が完了した
//////////////////////////////////////////////////////////////////////////////
void EWaveSound::OnEndPlaying
	( HWAVEBUF hWaveBuf, void * ptrBuffer, unsigned int nBufferLength )
{
	Lock( ) ;
	//
	// 再生終了
	//
	if ( (m_hLastWaveBuf == hWaveBuf) && m_flagFinished )
	{
		m_flagPlaying = FALSE ;
		m_flagFinished = false ;
		m_hLastWaveBuf = NULL ;
		//
		// イベントハンドル m_hDoneEvent セット
		//
		if ( m_hDoneEvent )
		{
			::SetEvent( m_hDoneEvent ) ;
		}
		//
		// ウィンドウメッセージ送信
		//
		if ( m_hwndNotifyDone )
		{
			::PostMessage( m_hwndNotifyDone,
				m_msgNotifyDone, (WPARAM) this, m_paramNotifyDone ) ;
		}
	}

	Unlock( ) ;
}

// 音声出力デバイスが次の音声バッファを要求している
//////////////////////////////////////////////////////////////////////////////
HWAVEBUF EWaveSound::OnQueueNextBuffer( EWaveOutDevice * pWaveDev )
{
	Lock( ) ;
	//
	// 次の出力位置を計算
	//
	if ( m_nOutputPos >= m_nEndingPos )
	{
		if ( m_flagRepeat )
		{
			if ( m_nEndingPos > m_nRewindingPos )
			{
				m_nOutputBais += (m_nEndingPos - m_nLastStartedPos) ;
			}
			m_nOutputPos = m_nRewindingPos ;
			m_nLastStartedPos = m_nRewindingPos ;
		}
		else
		{
			Unlock( ) ;
			m_flagFinished = true ;
			return	NULL ;
		}
	}
	ESLAssert( m_nOutputPos <= m_nEndingPos ) ;
	//
	// 出力位置とサイズをバイト単位で計算
	//
	HWAVEBUF	hWaveBuf = NULL ;
	if ( m_pWaveFormat != NULL )
	{
		DWORD		dwNextPos = SampleToBytes( m_nOutputPos ) ;
		DWORD		dwNextSize ;
		if ( m_pWaveFormat->wFormatTag != WAVE_FORMAT_PCM )
		{
			dwNextSize = SampleToBytes( m_nEndingPos ) - dwNextPos ;
			m_nOutputPos = m_nEndingPos ;
		}
		else if ( m_nOutputPos + m_nOutputSize <= m_nEndingPos )
		{
			dwNextSize = SampleToBytes( m_nOutputSize ) ;
			m_nOutputPos += m_nOutputSize ;
		}
		else
		{
			dwNextSize = SampleToBytes( m_nEndingPos ) - dwNextPos ;
			m_nOutputPos = m_nEndingPos ;
		}
		if ( dwNextSize != 0 )
		{
			hWaveBuf = pWaveDev->PrepareBuffer
				( this, ((BYTE*)m_pWaveBuffer) + dwNextPos, dwNextSize ) ;
			if ( hWaveBuf == NULL )
			{
				Unlock( ) ;
				m_flagFinished = true ;
				return	NULL ;
			}
			m_hLastWaveBuf = hWaveBuf ;
		}
		else
		{
			Unlock( ) ;
			m_flagFinished = true ;
			return	NULL ;
		}
	}
	Unlock( ) ;
	//
	return	hWaveBuf ;
}

// 内容を削除
//////////////////////////////////////////////////////////////////////////////
void EWaveSound::Delete( void )
{
	Lock( ) ;

	// 再生中の場合には停止する
	if ( m_flagPlaying && m_pWaveDevice )
	{
		m_flagPlaying = FALSE ;
		Unlock( ) ;
		m_pWaveDevice->Stop( this ) ;
		Lock( ) ;
	}

	// 音声フォーマットを削除
	if ( m_pWaveFormat )
	{
		::eslHeapFree( NULL, m_pWaveFormat ) ;
	}
	m_pWaveFormat = NULL ;

	// 音声バッファを解放
	if ( m_pWaveBuffer && m_fBufDelete )
	{
		::eslHeapFree( NULL, m_pWaveBuffer ) ;
	}
	m_fBufDelete = 0 ;
	m_pWaveBuffer = NULL ;

	Unlock( ) ;
}

// ウェーブフォームオーディファイル読みこみ
//////////////////////////////////////////////////////////////////////////////
ESLError EWaveSound::ReadWave( ESLFileObject & file )
{
	Lock( ) ;

	// 現在の内容を削除
	Delete( ) ;

	// === Windows Wave Form Audio Header ===
	// 'RIFF' : 4 Bytes     : RIFF Header
	//      ? : Double Word : File Length
	// 'WAVE' : 4 Bytes     : WAVE Header
	static const DWORD *	pdwRIFF = (const DWORD *) "RIFF" ;
	static const DWORD *	pdwWAVE = (const DWORD *) "WAVE" ;
	DWORD	dwWavHdr[3] ;
	if ( file.Read( dwWavHdr, sizeof(dwWavHdr) ) != sizeof(dwWavHdr) )
	{
		Unlock( ) ;
		return	ESLErrorMsg( "音声ヘッダの読みこみに失敗しました。" ) ;
	}
	if ( (dwWavHdr[0] != *pdwRIFF) || (dwWavHdr[2] != *pdwWAVE) )
	{
		Unlock( ) ;
		return	ESLErrorMsg( "無効な音声ヘッダです。" ) ;
	}

	// === Wave Format Header ===
	// 'fmt ' : 4 Bytes     : fmt Chunk
	//      ? : Double Word : Chunk Length
	static const DWORD *	pdwfmt = (const DWORD *) "fmt " ;
	for ( ; ; )
	{
		if ( file.Read( dwWavHdr, (sizeof(DWORD)*2) ) < (sizeof(DWORD)*2) )
		{
			Unlock( ) ;
			return	ESLErrorMsg( "無効な音声ヘッダです。" ) ;
		}
		if ( dwWavHdr[0] == *pdwfmt )
		{
			break ;
		}
		file.Seek( dwWavHdr[1], ESLFileObject::FromCurrent ) ;
	}

	m_nFormatSize = dwWavHdr[1] ;
	m_pWaveFormat =
		(LPWAVEFORMATEX) ::eslHeapAllocate( NULL, dwWavHdr[1], 0 ) ;
	if ( file.Read( m_pWaveFormat, dwWavHdr[1] ) != dwWavHdr[1] )
	{
		Unlock( ) ;
		return	ESLErrorMsg( "音声フォーマットの読み込みに失敗しました。" ) ;
	}

	// === Wave Data ===
	// 'data' : 4 Bytes     : data chunk
	//      ? : Double Word : Chunk Length
	static const DWORD *	pdwdata = (const DWORD *) "data" ;
	for ( ; ; )
	{
		if ( file.Read( dwWavHdr, (sizeof(DWORD)*2) ) < (sizeof(DWORD)*2) )
		{
			Unlock( ) ;
			return	ESLErrorMsg( "無効な音声データヘッダです。" ) ;
		}
		if ( dwWavHdr[0] == *pdwdata )
		{
			break ;
		}
		file.Seek( dwWavHdr[1], ESLFileObject::FromCurrent ) ;
	}

	m_fBufDelete = 1 ;
	m_nWaveLength = dwWavHdr[1] ;
	m_pWaveBuffer = ::eslHeapAllocate( NULL, dwWavHdr[1], 0 ) ;
	if ( file.Read( m_pWaveBuffer, dwWavHdr[1] ) != dwWavHdr[1] )
	{
		Unlock( ) ;
		return	ESLErrorMsg( "音声データの読み込みに失敗しました。" ) ;
	}

	Unlock( ) ;
	return	eslErrSuccess ;
}

// ウェーブフォームオーディファイル書き出し
//////////////////////////////////////////////////////////////////////////////
ESLError EWaveSound::WriteWave( ESLFileObject & file ) const
{
	ESLAssert( m_pWaveBuffer && m_pWaveFormat ) ;
	Lock( ) ;

	static const char	chrRIFF[] = "RIFF" ;
	static const char	chrWAVE[] = "WAVE" ;
	static const char	chrfmt[] = "fmt " ;
	static const char	chrdata[] = "data" ;
	DWORD	dwFormatSize = m_nFormatSize ;
	DWORD	dwDataLength = m_nWaveLength ;
	DWORD	dwFileLength =
		4 + (4 + sizeof(DWORD) + dwFormatSize)
		  + (4 + sizeof(DWORD) + dwDataLength) ;

	// Write 'RIFF' main chunk
	file.Write( &chrRIFF[0], 4 ) ;
	file.Write( &dwFileLength, sizeof(DWORD) ) ;
	file.Write( &chrWAVE[0], 4 ) ;

	// Write 'fmt ' sub-chunk
	file.Write( &chrfmt[0], 4 ) ;
	file.Write( &dwFormatSize, sizeof(DWORD) ) ;

	file.Write( m_pWaveFormat, m_nFormatSize ) ;

	// Write 'data' sub-chunk
	file.Write( &chrdata[0], 4 ) ;
	file.Write( &dwDataLength, sizeof(DWORD) ) ;

	file.Write( m_pWaveBuffer, m_nWaveLength ) ;

	Unlock( ) ;
	return	eslErrSuccess ;
}

// Set wave data
//////////////////////////////////////////////////////////////////////////////
ESLError EWaveSound::SetWaveData
	( const WAVEFORMATEX * ptrWaveFormat,
		const void * ptrBuffer, unsigned int nBufLength )
{
	// 現在の内容を削除する
	Delete( ) ;

	// 音声フォーマットを設定
	ESLError	err ;
	err = SetWaveFormat( ptrWaveFormat ) ;
	if ( err )
		return	err ;

	// 音声データを複製
	Lock( ) ;
	m_fBufDelete = 1 ;
	m_nWaveLength = nBufLength ;
	m_pWaveBuffer = ::eslHeapAllocate( NULL, m_nWaveLength, 0 ) ;
	try
	{
		::memmove( m_pWaveBuffer, ptrBuffer, m_nWaveLength ) ;
	}
	catch ( ... )
	{
		Unlock( ) ;
		return	ESLErrorMsg( "バッファのサイズが不正です。" ) ;
	}
	Unlock( ) ;
	return	eslErrSuccess ;
}

// 音声データ関連付け
//////////////////////////////////////////////////////////////////////////////
ESLError EWaveSound::AttachWaveSound( const EWaveSound & wavbuf )
{
	// 現在の内容を削除する
	Delete( ) ;

	// 音声フォーマットを設定
	ESLError	err ;
	err = SetWaveFormat( wavbuf.GetWaveFormat() ) ;
	if ( err )
		return	err ;

	// 音声データを関連付ける
	Lock( ) ;
	m_fBufDelete = 0 ;
	m_nWaveLength = wavbuf.m_nWaveLength ;
	m_pWaveBuffer = wavbuf.m_pWaveBuffer ;
	Unlock( ) ;

	return	eslErrSuccess ;
}

ESLError EWaveSound::AttachWaveSound
	( const WAVEFORMATEX * pwfx, const void * ptrBuf, unsigned int nBufLen )
{
	// 現在の内容を削除する
	Delete( ) ;

	// 音声フォーマットを設定
	ESLError	err ;
	err = SetWaveFormat( pwfx ) ;
	if ( err )
		return	err ;

	// 音声データを関連付ける
	Lock( ) ;
	m_fBufDelete = 0 ;
	m_nWaveLength = nBufLen ;
	m_pWaveBuffer = (void*) ptrBuf ;
	Unlock( ) ;

	return	eslErrSuccess ;
}

// 出力デバイス関連付け
//////////////////////////////////////////////////////////////////////////////
void EWaveSound::AttachWaveDevice( EWaveOutDevice * pWaveDev )
{
	if ( pWaveDev != m_pWaveDevice )
	{
		delete	m_pWaveDevMixingBuf ;
		m_pWaveDevMixingBuf = NULL ;
	}
	m_pWaveDevice = pWaveDev ;
}

// 音声再生開始
//////////////////////////////////////////////////////////////////////////////
ESLError EWaveSound::PlayWave( bool fRepeat, HANDLE hEvent )
{
	return	PlayWithDelayFrom( -1, fRepeat, hEvent, 33, 0 ) ;
}

// 音声再生開始
//////////////////////////////////////////////////////////////////////////////
ESLError EWaveSound::PlayMusic
	( unsigned int nIntroSamples, bool fRepeat, HANDLE hEvent )
{
	return	EWaveSound::PlayWithDelayFrom
				( nIntroSamples, fRepeat, hEvent, 33, 0 ) ;
}

// 遅延再生
//////////////////////////////////////////////////////////////////////////////
ESLError EWaveSound::PlayWithDelayFrom
	( unsigned int nIntroSamples, bool fRepeat, HANDLE hEvent,
				unsigned int nDelay, unsigned int nStartPos )
{
	ESLAssert( m_pWaveBuffer && m_pWaveFormat ) ;

	if ( (m_pWaveDevice == NULL)
		|| (m_pWaveBuffer == NULL) || (m_pWaveFormat == NULL) )
	{
		return	eslErrGeneral ;
	}
	//
	// 再生中の場合には停止する
	//
	if ( IsPlaying() )
	{
		m_pWaveDevice->Stop( this ) ;
	}
	//
	// フラグ設定
	//
	Lock( ) ;
	if ( (signed int) nIntroSamples < 0 )
	{
		nIntroSamples = 0 ;
	}
	if ( nStartPos > GetWaveSamples() )
	{
		nStartPos = 0 ;
	}
	m_flagFinished = false ;
	m_flagRepeat = fRepeat ;
	m_hDoneEvent = hEvent ;
	m_nRewindingPos = nIntroSamples ;
	m_nLastStartedPos = nStartPos ;
	m_nEndingPos = BytesToSample( GetWaveLength() ) ;
	m_nOutputPos = m_nEndingPos ;
	m_nOutputSize = 0x1000 ;
	m_hLastWaveBuf = NULL ;
	m_nOutputBais = 0 ;
	//
	// データ出力
	//
	unsigned int	nStartBytes = SampleToBytes( nStartPos ) ;
	if ( nStartBytes >= m_nWaveLength )
	{
		nStartBytes = 0 ;
	}
	m_hLastWaveBuf =
		m_pWaveDevice->PrepareBuffer
			( this, ((BYTE*)m_pWaveBuffer) + nStartBytes,
							m_nWaveLength - nStartBytes ) ;
	Unlock( ) ;

	ESLError	err ;
	if ( m_pWaveDevice->IsKindOf( ESL_RUNTIME_CLASS(EWaveMixingServer) ) )
	{
		err = ((EWaveMixingServer*)m_pWaveDevice)->
					DelayPlaying( this, m_hLastWaveBuf, nDelay ) ;
	}
	else
	{
		err = m_pWaveDevice->Play( this, m_hLastWaveBuf ) ;
	}
	m_flagPlaying = (err == eslErrSuccess) ;

	return	err ;
}

// 再生開始
//////////////////////////////////////////////////////////////////////////////
ESLError EWaveSound::PlayFrom
	( unsigned int nStartPos, unsigned int nPlayEnd,
				bool fRepeat, unsigned int nRewindPos )
{
	ESLAssert( m_pWaveBuffer && m_pWaveFormat ) ;
	if ( (m_pWaveDevice == NULL)
		|| (m_pWaveBuffer == NULL) || (m_pWaveFormat == NULL) )
	{
		return	eslErrGeneral ;
	}
	//
	// 再生中の場合には停止する
	//
	if ( IsPlaying() )
	{
		m_pWaveDevice->Stop( this ) ;
	}
	//
	// リピートパラメータ設定
	//
	Lock( ) ;
	if ( nStartPos > GetWaveSamples() )
	{
		nStartPos = 0 ;
	}
	if ( (signed int) nRewindPos < 0 )
	{
		nRewindPos = 0 ;
	}
	if ( nPlayEnd == (unsigned int) -1 )
	{
		nPlayEnd = BytesToSample( GetWaveLength() ) ;
	}
	m_flagFinished = false ;
	m_flagRepeat = fRepeat ;
	m_nRewindingPos = nRewindPos ;
	m_nEndingPos = nPlayEnd ;
	m_nOutputPos = nStartPos ;
	m_nLastStartedPos = nStartPos ;
	m_nOutputSize = m_pWaveFormat->nSamplesPerSec / 8 ;
	m_hLastWaveBuf = NULL ;
	m_nOutputBais = 0 ;
	//
	// データ出力
	//
	m_hLastWaveBuf = OnQueueNextBuffer( m_pWaveDevice ) ;
	Unlock( ) ;
	//
	ESLError	err = eslErrSuccess ;
	if ( m_hLastWaveBuf != NULL )
	{
		err = m_pWaveDevice->Play( this, m_hLastWaveBuf ) ;
	}
	m_flagPlaying = (err == eslErrSuccess) ;

	return	err ;
}

// 再生停止
//////////////////////////////////////////////////////////////////////////////
ESLError EWaveSound::StopWave( void )
{
	if ( m_pWaveDevice == NULL )
	{
		return	eslErrGeneral ;
	}
	ESLError	err = eslErrSuccess ;
	Lock( ) ;
	if ( m_flagPlaying )
	{
		m_flagPlaying = false ;
		Unlock( ) ;
		err = m_pWaveDevice->Stop( this ) ;
		m_fPauseFlag = FALSE ;
		return	err ;
	}
	else
	{
		Unlock( ) ;
		return	err ;
	}
}

// 再生一時停止
//////////////////////////////////////////////////////////////////////////////
ESLError EWaveSound::PauseWave( void )
{
	if ( m_pWaveDevice == NULL )
	{
		return	eslErrGeneral ;
	}
	ESLError	err = eslErrSuccess ;
	if ( m_flagPlaying )
	{
		err = m_pWaveDevice->Pause( this ) ;
	}
	return	err ;
}

// 再生再開
//////////////////////////////////////////////////////////////////////////////
ESLError EWaveSound::RestartWave( void )
{
	if ( m_pWaveDevice == NULL )
	{
		return	eslErrGeneral ;
	}
	ESLError	err = eslErrSuccess ;
	if ( m_flagPlaying )
	{
		err = m_pWaveDevice->Restart( this ) ;
	}
	return	err ;
}

// 再生ループポイント設定
//////////////////////////////////////////////////////////////////////////////
ESLError EWaveSound::SetRewindingPortion
	( unsigned int nRewindPos, unsigned int nEndPos, bool fRepeat )
{
	Lock( ) ;
	if ( nEndPos == (unsigned int) -1 )
	{
		m_nEndingPos = BytesToSample( GetWaveLength() ) ;
	}
	m_flagRepeat = fRepeat ;
	m_nRewindingPos = nRewindPos ;
	m_nEndingPos = nEndPos ;
	Unlock( ) ;
	return	eslErrSuccess ;
}

// 出力ボリューム取得
//////////////////////////////////////////////////////////////////////////////
ESLError EWaveSound::GetVolume
	( REAL32 & rLeftVolume, REAL32 & rRightVolume ) const
{
	rLeftVolume = m_realVolume[0] ;
	rRightVolume = m_realVolume[1] ;
	return	eslErrSuccess ;
}

// 出力ボリューム設定
//////////////////////////////////////////////////////////////////////////////
ESLError EWaveSound::SetVolume
	( REAL32 rLeftVolume, REAL32 rRightVolume )
{
	REAL32	rVolume[2] ;
	rVolume[0] = rLeftVolume ;
	rVolume[1] = rRightVolume ;
	if ( m_pWaveDevice != NULL )
	{
		m_pWaveDevice->SetVolume( this, rVolume ) ;
	}
	return	eslErrSuccess ;
}

// 現在の再生位置を取得
//////////////////////////////////////////////////////////////////////////////
unsigned long int EWaveSound::GetCurrentSample( void ) const
{
	if ( m_pWaveDevice != NULL )
	{
		UINT64	nCurrent ;
		nCurrent = m_pWaveDevice->GetCurrentSample( this ) ;
		if ( nCurrent >= m_nOutputBais )
		{
			nCurrent -= m_nOutputBais ;
			nCurrent += m_nLastStartedPos ;
		}
		else
		{
			ESLAssert( m_nRewindingPos < m_nEndingPos ) ;
			if ( m_nRewindingPos < m_nEndingPos )
			{
				ESLAssert( (INT64) (nCurrent - m_nOutputBais) < 0 ) ;
				UINT64	nOdd =
					(m_nOutputBais - nCurrent)
										% (m_nEndingPos - m_nRewindingPos) ;
				nCurrent = m_nEndingPos - nOdd ;
			}
		}
		return	(unsigned long int) nCurrent ;
	}
	return	0 ;
}

// スレッド排他アクセス
//////////////////////////////////////////////////////////////////////////////
void EWaveSound::Lock( void ) const
{
	::EnterCriticalSection( (LPCRITICAL_SECTION) &m_cs ) ;
}

void EWaveSound::Unlock( void ) const
{
	::LeaveCriticalSection( (LPCRITICAL_SECTION) &m_cs ) ;
}


/*****************************************************************************
						3D 音声効果オブジェクト
 ****************************************************************************/

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( E3DSoundEffect, EWaveSound )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
E3DSoundEffect::E3DSoundEffect( EWaveOutDevice * pWaveDev )
	: EWaveSound( pWaveDev ),
		m_rVolume(1.0), m_vPosition(0,0,0), m_vMotion(0,0,0)
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
E3DSoundEffect::~E3DSoundEffect( void )
{
}

// ボリューム設定
//////////////////////////////////////////////////////////////////////////////
void E3DSoundEffect::SetVolume( double rVolume )
{
	m_rVolume = rVolume ;
}

// 位置を設定
//////////////////////////////////////////////////////////////////////////////
void E3DSoundEffect::SetPosition
	( const E3D_VECTOR & vPosition, bool fAutoMotion )
{
	if ( fAutoMotion )
	{
		m_vMotion = vPosition - m_vPosition ;
	}
	m_vPosition = vPosition ;
}

// 運動量を設定
//////////////////////////////////////////////////////////////////////////////
void E3DSoundEffect::SetMotion( const E3D_VECTOR & vMotion )
{
	m_vMotion = vMotion ;
}

// 音量を反映する
//////////////////////////////////////////////////////////////////////////////
ESLError E3DSoundEffect::ApplyVolume( void )
{
	//
	// 位置から音量を計算
	//
	double	rVol = 1.0 ;
	double	rPos = m_vPosition.Absolute( ) ;
	if ( rPos > m_rVolume )
	{
		rVol = m_rVolume / rPos ;
	}
	//
	// 運動量から音量に変化を持たせる
	//
	E3D_VECTOR	vMoved = m_vPosition + m_vMotion ;
	double	rMovedPos = vMoved.Absolute( ) ;
	if ( rMovedPos > rPos * rVol )
	{
		rVol *= rPos / rMovedPos ;
	}
	else
	{
		rVol = 1.0 ;
	}
	//
	// 座標から左右にパンする
	//
	double	x = m_vPosition.x / rPos ;
	REAL32	rRightVol = (REAL32)((x + 1.0) * rVol) ;
	REAL32	rLeftVol = (REAL32)((1.0 - x) * rVol) ;
	//
	// 音量を反映する
	//
	return	EWaveSound::SetVolume( rLeftVol, rRightVol ) ;
}


/*****************************************************************************
							MIO 再生オブジェクト
 ****************************************************************************/

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( MIOSoundStream, EWaveSound )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
MIOSoundStream::MIOSoundStream( void )
{
	m_pmMode = pmStaticPlay ;
	m_pfile = NULL ;
	m_hPlayedEvent = ::CreateEvent( NULL, TRUE, TRUE, NULL ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
MIOSoundStream::~MIOSoundStream( void )
{
	Close( ) ;
	//
	::CloseHandle( m_hPlayedEvent ) ;
}

// 音声データの再生が完了した
//////////////////////////////////////////////////////////////////////////////
void MIOSoundStream::OnEndPlaying
	( HWAVEBUF hWaveBuf, void * ptrBuffer, unsigned int nBufferLength )
{
	Lock( ) ;

	unsigned int	nIndex ;
	EStreamBuffer *	pBuf =
		m_itaWaveBuf.GetAs( (ULONG_PTR) ptrBuffer, &nIndex ) ;
	if ( pBuf != NULL )
	{
		m_pWaveDevice->UnprepareBuffer( hWaveBuf ) ;
		m_itaWaveBuf.RemoveAt( nIndex ) ;
		if ( m_itaWaveBuf.GetSize() == 0 )
		{
			::SetEvent( m_hPlayedEvent ) ;
			//
			if ( !m_flagRepeat )
			{
				m_flagPlaying = FALSE ;
				//
				if ( m_hDoneEvent )
				{
					::SetEvent( m_hDoneEvent ) ;
				}
				//
				if ( m_hwndNotifyDone )
				{
					::PostMessage( m_hwndNotifyDone,
						m_msgNotifyDone, (WPARAM) this, m_paramNotifyDone ) ;
				}
			}
		}
	}
	else
	{
		EWaveSound::OnEndPlaying( hWaveBuf, ptrBuffer, nBufferLength ) ;
	}

	Unlock( ) ;
}

// 音声出力デバイスが次の音声バッファを要求している
//////////////////////////////////////////////////////////////////////////////
HWAVEBUF MIOSoundStream::OnQueueNextBuffer( EWaveOutDevice * pWaveDev )
{
	Lock( ) ;

	HWAVEBUF	hWaveBuf = NULL ;
	if ( m_pmMode != pmStaticPlay )
	{
		void *	ptrWaveBuf = NULL ;
		DWORD	dwBytes = 0 ;
		EStreamBuffer *	pBuf = NULL ;
		//
		if ( m_flagPlaying )
		{
			if ( m_nOutputPos >= m_nEndingPos )
			{
				if ( m_flagRepeat && (m_nRewindingPos < m_nEndingPos) )
				{
					DWORD	dwOffset = 0 ;
					ptrWaveBuf = m_miodp.GetWaveBufferFrom
						( m_nRewindingPos, dwBytes, dwOffset ) ;
					if ( ptrWaveBuf != NULL )
					{
						DWORD	dwDataLen = dwBytes - dwOffset ;
						DWORD	dwSamples = BytesToSample( dwDataLen ) ;
						m_nOutputPos = m_nRewindingPos ;
						if ( m_nOutputPos + dwSamples > m_nEndingPos )
						{
							dwSamples = m_nEndingPos - m_nOutputPos ;
							dwDataLen = SampleToBytes( dwSamples ) ;
						}
						m_nOutputPos += dwSamples ;
						//
						BYTE *	pbytSrc = ((BYTE*) ptrWaveBuf) + dwOffset ;
						pBuf = new EStreamBuffer ;
						::eslMoveMemory
							( pBuf->PutBuffer(dwDataLen), pbytSrc, dwDataLen ) ;
						pBuf->Flush( dwDataLen ) ;
					}
					m_nOutputBais += (m_nEndingPos - m_nRewindingPos) ;
				}
			}
			else
			{
				ptrWaveBuf = m_miodp.GetNextWaveBuffer( dwBytes ) ;
				if ( ptrWaveBuf != NULL )
				{
					DWORD	dwSamples = BytesToSample( dwBytes ) ;
					if ( m_nOutputPos + dwSamples > m_nEndingPos )
					{
						dwSamples = m_nEndingPos - m_nOutputPos ;
						dwBytes = SampleToBytes( dwSamples ) ;
					}
					m_nOutputPos += dwSamples ;
					//
					pBuf = new EStreamBuffer ;
					::eslMoveMemory
						( pBuf->PutBuffer(dwBytes), ptrWaveBuf, dwBytes ) ;
					pBuf->Flush( dwBytes ) ;
				}
			}
		}
		//
		if ( ptrWaveBuf != NULL )
		{
			m_miodp.DeleteWaveBuffer( ptrWaveBuf ) ;
		}
		if ( pBuf != NULL )
		{
			EPtrBuffer	ptrbuf = pBuf->GetBuffer( ) ;
			const void *	ptrBuf = ptrbuf.GetBuffer( ) ;
			unsigned int	nBufLen = ptrbuf.GetLength( ) ;
			m_itaWaveBuf.Add( (ULONG_PTR) ptrBuf, pBuf ) ;
			::ResetEvent( m_hPlayedEvent ) ;
			//
			hWaveBuf = pWaveDev->PrepareBuffer( this, ptrBuf, nBufLen ) ;
		}
	}
	else
	{
		hWaveBuf = EWaveSound::OnQueueNextBuffer( pWaveDev ) ;
	}

	Unlock( ) ;
	return	hWaveBuf ;
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
ESLError MIOSoundStream::Open
	( ESLFileObject & file, MIOSoundStream::PlayMode pmMode )
{
	//
	// 現在のファイルを閉じる
	//
	Close( ) ;
	//
	// MIO ファイルを開く
	//
	ESLFileObject *	pifile ;
	unsigned int nPreloadSize ;
	if ( pmMode == pmDynamicRead )
	{
		pifile = &file ;
		nPreloadSize = 0 ;
	}
	else
	{
		DWORD	dwSize = file.GetLength() ;
		EMemoryFile *	pmemfile = new EMemoryFile ;
		pmemfile->Create( dwSize ) ;
		file.Read( pmemfile->GetBuffer(), dwSize ) ;
		pmemfile->Seek( dwSize, ESLFileObject::FromBegin ) ;
		pmemfile->SetEndOfFile( ) ;
		pmemfile->Seek( 0, ESLFileObject::FromBegin ) ;
		m_pfile = pmemfile ;
		pifile = pmemfile ;
		nPreloadSize = -1 ;
	}
	if ( m_miodp.Open( pifile, nPreloadSize ) )
	{
		return	ESLErrorMsg( "MIO ファイルを開けませんでした。" ) ;
	}
	//
	const ERIFile &	erif = m_miodp.GetERIFile( ) ;
	m_nRewoundSample = 0 ;
	m_nLoopEndSample = (ULONG) -1 ;
	if ( erif.m_fdwReadMask & ERIFile::rmDescription )
	{
		ERIFile::ETagInfo	taginf ;
		taginf.CreateTagInfo( erif.m_wstrDescription ) ;
		m_nRewoundSample = taginf.GetRewindPoint( ) ;
		m_nLoopEndSample = (ULONG) taginf.GetLoopEndPoint( ) ;
	}
	if ( m_nLoopEndSample == (ULONG) -1 )
	{
		m_nLoopEndSample = m_miodp.GetTotalSampleCount() ;
	}
	//
	// 音声フォーマットを設定
	//
	WAVEFORMATEX	wfx ;
	wfx.wFormatTag = WAVE_FORMAT_PCM ;
	wfx.nChannels = (WORD) m_miodp.GetChannelCount( ) ;
	wfx.nSamplesPerSec = m_miodp.GetFrequency( ) ;
	wfx.wBitsPerSample = (WORD) m_miodp.GetBitsPerSample( ) ;
	wfx.nBlockAlign = wfx.wBitsPerSample * wfx.nChannels / 8 ;
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign ;
	wfx.cbSize = 0 ;
	ESLError	err = SetWaveFormat( &wfx ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// 静的再生の為の処理
	//
	if ( pmMode == pmStaticPlay )
	{
		//
		// バッファ確保
		//
		m_fBufDelete = 1 ;
		m_nWaveLength = m_miodp.GetTotalSampleCount() * wfx.nBlockAlign ;
		m_pWaveBuffer =
			::eslHeapAllocate( NULL, m_nWaveLength, ESL_HEAP_ZERO_INIT ) ;
		//
		// データを展開
		//
		void *	ptrWaveBuf ;
		DWORD	dwBytes, dwOffset ;
		ptrWaveBuf = m_miodp.GetWaveBufferFrom( 0, dwBytes, dwOffset ) ;
		if ( ptrWaveBuf != NULL )
		{
			BYTE *	ptrNextBuf = (BYTE*) m_pWaveBuffer ;
			DWORD	dwDataBytes = 0 ;
			if ( dwDataBytes + dwBytes > m_nWaveLength )
			{
				dwBytes = m_nWaveLength - dwDataBytes ;
			}
			::eslMoveMemory( ptrNextBuf, ptrWaveBuf, dwBytes ) ;
			m_miodp.DeleteWaveBuffer( ptrWaveBuf ) ;
			ptrNextBuf += dwBytes ;
			dwDataBytes += dwBytes ;
			//
			while ( !m_miodp.IsNextDataRewound() )
			{
				ptrWaveBuf = m_miodp.GetNextWaveBuffer( dwBytes ) ;
				if ( ptrWaveBuf == NULL )
					break ;
				//
				if ( dwDataBytes + dwBytes > m_nWaveLength )
				{
					dwBytes = m_nWaveLength - dwDataBytes ;
				}
				::eslMoveMemory( ptrNextBuf, ptrWaveBuf, dwBytes ) ;
				m_miodp.DeleteWaveBuffer( ptrWaveBuf ) ;
				ptrNextBuf += dwBytes ;
				dwDataBytes += dwBytes ;
			}
		}
		//
		m_miodp.Close( ) ;
		delete	m_pfile ;
		m_pfile = NULL ;
	}
	m_pmMode = pmMode ;
	//
	return	eslErrSuccess ;
}

// ファイルを閉じる
//////////////////////////////////////////////////////////////////////////////
void MIOSoundStream::Close( void )
{
	if ( m_pmMode != pmStaticPlay )
	{
		//
		// 再生中の場合には停止する
		//
		if ( IsPlaying() )
		{
			StopWave( ) ;
		}
		//
		// MIO 再生オブジェクトを終了する
		//
		m_miodp.Close( ) ;
	}
	delete	m_pfile ;
	m_pfile = NULL ;
	m_pmMode = pmStaticPlay ;
	//
	// EWaveSound 互換バッファを削除する
	//
	EWaveSound::Delete( ) ;
}

// 遅延再生
//////////////////////////////////////////////////////////////////////////////
ESLError MIOSoundStream::PlayWithDelayFrom
	( unsigned int nIntroSamples, bool fRepeat, HANDLE hEvent,
				unsigned int nDelay, unsigned int nStartPos )
{
	if ( m_pmMode == pmStaticPlay )
	{
		if ( m_pWaveBuffer != NULL )
		{
			if ( fRepeat
				&& (m_nLoopEndSample < BytesToSample( GetWaveLength() )) )
			{
				return	EWaveSound::PlayFrom
					( 0, m_nLoopEndSample, fRepeat, nIntroSamples ) ;
			}
			else
			{
				return	EWaveSound::PlayWithDelayFrom
					( nIntroSamples, fRepeat, hEvent, nDelay, nStartPos ) ;
			}
		}
		return	eslErrGeneral ;
	}
	if ( m_pWaveDevice == NULL )
	{
		return	eslErrGeneral ;
	}
	//
	// 再生開始
	//
	m_hDoneEvent = hEvent ;
	return	PlayFrom( 0, -1, fRepeat, nIntroSamples ) ;
}

// 再生開始
//////////////////////////////////////////////////////////////////////////////
ESLError MIOSoundStream::PlayFrom
	( unsigned int nStartPos, unsigned int nPlayEnd,
				bool fRepeat, unsigned int nRewindPos )
{
	if ( m_pmMode == pmStaticPlay )
	{
		if ( m_pWaveBuffer != NULL )
		{
			return	EWaveSound::PlayFrom
				( nStartPos, nPlayEnd, fRepeat, nRewindPos ) ;
		}
		return	eslErrGeneral ;
	}
	if ( m_pWaveDevice == NULL )
	{
		return	eslErrGeneral ;
	}
	//
	// 再生中の時には停止する
	//
	if ( IsPlaying() )
	{
		m_pWaveDevice->Stop( this ) ;
	}
	//
	// パラメータ設定
	//
	Lock( ) ;
	if ( (signed int) nPlayEnd < 0 )
	{
		nPlayEnd = m_nLoopEndSample ;
	}
	if ( (signed int) nRewindPos < 0 )
	{
		nRewindPos = m_nRewoundSample ;
	}
	m_flagRepeat = fRepeat ;
	m_nOutputPos = nStartPos ;
	m_nLastStartedPos = nStartPos ;
	m_nEndingPos = nPlayEnd ;
	m_nRewindingPos = nRewindPos ;
	//
	// データ出力
	//
	void *	ptrWaveBuf ;
	DWORD	dwBytes, dwOffset ;
	ptrWaveBuf = m_miodp.GetWaveBufferFrom( nStartPos, dwBytes, dwOffset ) ;
	Unlock( ) ;
	//
	if ( ptrWaveBuf == NULL )
	{
		return	ESLErrorMsg( "音声データを取得できませんでした。" ) ;
	}
	//
	Lock( ) ;
	BYTE *	pbytSrc = ((BYTE*) ptrWaveBuf) + dwOffset ;
	DWORD	dwDataLen = dwBytes - dwOffset ;
	DWORD	dwSamples = BytesToSample( dwDataLen ) ;
	if ( m_nOutputPos + dwSamples > m_nEndingPos )
	{
		if ( m_nOutputPos < m_nEndingPos )
		{
			dwSamples = m_nEndingPos - m_nOutputPos ;
		}
		else
		{
			dwSamples = 0 ;
		}
		dwDataLen = SampleToBytes( dwSamples ) ;
		m_nOutputPos = m_nEndingPos ;
	}
	else
	{
		m_nOutputPos += dwSamples ;
	}
	EStreamBuffer *	pBuf = new EStreamBuffer ;
	::eslMoveMemory
		( pBuf->PutBuffer(dwDataLen), pbytSrc, dwDataLen ) ;
	m_miodp.DeleteWaveBuffer( ptrWaveBuf ) ;
	pBuf->Flush( dwDataLen ) ;
	EPtrBuffer	ptrbuf = pBuf->GetBuffer( ) ;
	const void *	ptrBuf = ptrbuf.GetBuffer( ) ;
	unsigned int	nBufLen = ptrbuf.GetLength( ) ;
	m_itaWaveBuf.Add( (ULONG_PTR) ptrBuf, pBuf ) ;
	::ResetEvent( m_hPlayedEvent ) ;
	Unlock( ) ;
	//
	m_flagPlaying = true ;
	HWAVEBUF	hWaveBuf =
		m_pWaveDevice->PrepareBuffer( this, ptrBuf, nBufLen ) ;
	ESLError	err = m_pWaveDevice->Play( this, hWaveBuf ) ;
	m_flagPlaying = (err == eslErrSuccess) ;

	return	err ;
}

// 再生停止
//////////////////////////////////////////////////////////////////////////////
ESLError MIOSoundStream::StopWave( void )
{
	if ( m_pmMode != pmStaticPlay )
	{
		ESLError	err = eslErrSuccess ;
		Lock( ) ;
		if ( m_flagPlaying && m_pWaveDevice )
		{
			m_flagPlaying = false ;
			Unlock( ) ;
			err = m_pWaveDevice->Stop( this ) ;
			::WaitForSingleObject( m_hPlayedEvent, 1000 ) ;
			m_fPauseFlag = false ;
			return	err ;
		}
		else
		{
			Unlock( ) ;
			return	err ;
		}
	}
	else
	{
		return	EWaveSound::StopWave( ) ;
	}
}

// 音声データ関連付け
//////////////////////////////////////////////////////////////////////////////
ESLError MIOSoundStream::AttachWaveSound( const EWaveSound & wavbuf )
{
	const MIOSoundStream *	pBuf =
		ESLTypeCast<MIOSoundStream,EWaveSound>( &wavbuf ) ;
	if ( pBuf != NULL )
	{
		if ( (pBuf->m_pmMode == pmDynamicPlay) && (pBuf->m_pfile != NULL) )
		{
			EMemoryFile *	pmemfile = new EMemoryFile ;
			EMemoryFile *	psrcmemfile =
				ESLTypeCast<EMemoryFile>( pBuf->m_pfile ) ;
			if ( psrcmemfile != NULL )
			{
				pmemfile->Open
					( psrcmemfile->GetBuffer(), psrcmemfile->GetLength() ) ;
				ESLError	err = Open( *pmemfile, pmDynamicRead ) ;
				if ( err )
				{
					return	err ;
				}
				m_pfile = pmemfile ;
				m_pmMode = pmDynamicPlay ;
				return	eslErrSuccess ;
			}
			return	eslErrGeneral ;
		}
		else if ( pBuf->m_pmMode == pmDynamicRead )
		{
			ESLFileObject *	pfile = pBuf->m_miodp.GetERIFile().GetOriginalFile() ;
			if ( pfile != NULL )
			{
				pfile = pfile->Duplicate( ) ;
				if ( pfile != NULL )
				{
					ESLError	err = Open( *pfile, pmDynamicRead ) ;
					if ( err )
					{
						return	err ;
					}
					m_pfile = pfile ;
					m_pmMode = pmDynamicRead ;
					return	eslErrSuccess ;
				}
			}
			return	eslErrGeneral ;
		}
	}
	Close( ) ;
	return	EWaveSound::AttachWaveSound( wavbuf ) ;
}
