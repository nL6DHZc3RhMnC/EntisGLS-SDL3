
#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl_media.h>
#include <sakuragl/sgl_direct_sound_player.h>

#include <mmdeviceapi.h>
#include <audioclient.h>

#include <functiondiscoverykeys_devpkey.h>
#include <GuidDef.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// DirectSound 出力インターフェース
//////////////////////////////////////////////////////////////////////////////

// DirectSound
ESL_DLL_DECL( IDirectSound *		SGLDirectSoundPlayer::m_idsound = NULL ) ;		// DirectSound object
ESL_DLL_DECL( IDirectSoundBuffer *	SGLDirectSoundPlayer::m_idsbPrimary = NULL ) ;	// primary DirectSound buffer

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLDirectSoundPlayer, SGLSoundPlayerInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLDirectSoundPlayer::SGLDirectSoundPlayer( void )
{
	m_idsbuf = NULL ;
	m_iBuffering = 0 ;
	m_iPlayingBuf = -1 ;
	m_flagStreaming = false ;
	m_flagPlayed = false ;
	m_flagPaused = false ;
	m_flagThread = false ;
	m_pThread = NULL ;
	m_mutexSync.Initialize() ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLDirectSoundPlayer::~SGLDirectSoundPlayer( void )
{
	Close() ;
}

// フォーマットを指定して出力を準備する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirectSoundPlayer::Open( const SGLSoundFormat& fmt )
{
	Close() ;
	ConvertWaveFormat( m_wfxFormat, fmt ) ;
	return	sglErrSuccess ;
}

// 出力用に準備したサウンド出力を解放する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirectSoundPlayer::Close( void )
{
	if ( m_flagThread )
	{
		EndListenerThread() ;
	}
	if ( m_idsbuf != NULL )
	{
		m_idsbuf->Stop() ;
		m_idsbuf->Release() ;
		m_idsbuf = NULL ;
	}
	for ( int i = 0; i < countBuffer; i ++ )
	{
		m_bufSound[i].FreeArray() ;
		m_bufSound[i].m_pbytBuffer = 0 ;
		m_bufSound[i].m_sizeBuffer = 0 ;
		m_bufSound[i].m_sizeStuffed = 0 ;
	}
	m_iBuffering = 0 ;
	m_iPlayingBuf = -1 ;
	m_flagStreaming = false ;
	m_flagPlayed = false ;
	m_flagPaused = false ;
	return	sglErrSuccess ;
}

// スタティックバッファを準備して書き込む
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirectSoundPlayer::WriteStatic( const void * ptrSound, size_t nBytes )
{
	Close() ;
	//
	// バッファ生成
	//
	DSBUFFERDESC	dsbd ;
	memset( &dsbd, 0, sizeof(dsbd) ) ;
	dsbd.dwSize = sizeof(dsbd) ;
	dsbd.dwFlags =
		DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME
			| DSBCAPS_GLOBALFOCUS
			| DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GETCURRENTPOSITION2 ;
	dsbd.dwBufferBytes = (DWORD) (nBytes + m_wfxFormat.nBlockAlign) ;
	dsbd.lpwfxFormat = &m_wfxFormat ;
	//
	ESLAssert( m_idsound != NULL ) ;
	ESLAssert( m_idsbuf == NULL ) ;
	if ( m_idsound == NULL )
	{
		SGLDirectSoundPlayer::Initialize() ;
		if ( m_idsound == NULL )
		{
			return	sglErrFailed ;
		}
	}
	if ( m_idsound->CreateSoundBuffer( &dsbd, &m_idsbuf, NULL ) == DS_OK )
	{
		ESLAssert( m_idsbuf != NULL ) ;
	}
	else
	{
		ESLTrace( "failed to create DirectSoundBuffer\n" ) ;
		return	sglErrFailed ;
	}
	m_flagStreaming = false ;
	//
	// バッファに書き込む
	//
	ESLAssert( m_bufSound[0].GetLength() == 0 ) ;
	m_bufSound[0].AddArray( (const uint8_t*) ptrSound, m_wfxFormat.nBlockAlign ) ;
	m_bufSound[0].AddArray( (const uint8_t*) ptrSound, nBytes ) ;
	m_bufSound[0].m_pbytBuffer = m_bufSound[0].GetArrayPtr() ;
	m_bufSound[0].m_sizeBuffer = m_bufSound[0].GetLength() ;
	m_bufSound[0].m_sizeStuffed = m_bufSound[0].m_sizeBuffer ;
	//
	m_idsbuf->SetCurrentPosition( m_wfxFormat.nBlockAlign ) ;
	//
	return	UpdateDirectSoundBuffer( 0, m_bufSound[0].m_sizeBuffer ) ;
}

// ストリームバッファを準備する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirectSoundPlayer::PrepareStream( size_t nBytes )
{
	Close() ;
	//
	// バッファ生成
	//
	DSBUFFERDESC	dsbd ;
	memset( &dsbd, 0, sizeof(dsbd) ) ;
	dsbd.dwSize = sizeof(dsbd) ;
	dsbd.dwFlags =
		DSBCAPS_CTRLFREQUENCY | DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME
			| DSBCAPS_GLOBALFOCUS
			| DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GETCURRENTPOSITION2 ;
	if ( nBytes == 0 )
	{
		nBytes = 0x20000 ;
	}
	m_nBufferSize = ((nBytes + 0x0F) & ~0x0F) / countBuffer ;
	dsbd.dwBufferBytes = (DWORD) (m_nBufferSize * countBuffer) ;
	dsbd.lpwfxFormat = &m_wfxFormat ;
	//
	ESLAssert( m_idsound != NULL ) ;
	ESLAssert( m_idsbuf == NULL ) ;
	if ( m_idsound == NULL )
	{
		SGLDirectSoundPlayer::Initialize() ;
		if ( m_idsound == NULL )
		{
			return	sglErrFailed ;
		}
	}
	if ( m_idsound->CreateSoundBuffer( &dsbd, &m_idsbuf, NULL ) == DS_OK )
	{
		ESLAssert( m_idsbuf != NULL ) ;
	}
	else
	{
		ESLTrace( "failed to create DirectSoundBuffer\n" ) ;
		return	sglErrFailed ;
	}
	m_flagStreaming = true ;
	//
	// バッファを準備
	//
	InitializeStreamingBuffer() ;
	//
	return	sglErrSuccess ;
}

// ストリームバッファへ書き出す
//////////////////////////////////////////////////////////////////////////////
size_t SGLDirectSoundPlayer::Write( const void * ptrSound, size_t nBytes )
{
	if ( !m_flagStreaming )
	{
		return	0 ;
	}
	if ( m_mutexSync.LockTrace( __FILE__, __LINE__, 100 ) != errSuccess )
	{
		ESLTrace( "timeout at SGLDirectSoundPlayer::Write\n" ) ;
		return	0 ;
	}
	size_t	nOffset = m_bufSound[m_iBuffering].m_sizeStuffed ;
	if ( m_flagPlayed )
	{
		//
		// 再生位置取得
		//
		DWORD	dwPlayPos = 0 ;
		size_t	iPlayingBuf = 0 ;
		if ( PollingCurrentPosition( iPlayingBuf, dwPlayPos ) == sglErrSuccess )
		{
			//
			// 位置補正
			//
			if ( m_iPlayingBuf == (ssize_t) m_iBuffering )
			{
				dwPlayPos += m_wfxFormat.nBlockAlign
								* m_wfxFormat.nSamplesPerSec / 1024 ;	// 約 1 [ms]
				dwPlayPos += (m_wfxFormat.nBlockAlign
								- (dwPlayPos % m_wfxFormat.nBlockAlign)) ;
				if ( nOffset < dwPlayPos )
				{
					nOffset = dwPlayPos ;
					if ( nOffset >= m_bufSound[m_iBuffering].m_sizeBuffer )
					{
						nOffset = m_bufSound[m_iBuffering].m_sizeBuffer ;
					}
				}
			}
		}
	}
	//
	// バッファへ書き出す
	//
	ESLAssert( nOffset <= m_bufSound[m_iBuffering].m_sizeBuffer ) ;
	size_t	nWrittenBytes = 0 ;
	const uint8_t *	pbytSound = (const uint8_t*) ptrSound ;
	while ( nBytes > 0 )
	{
		if ( nOffset < m_bufSound[m_iBuffering].m_sizeBuffer )
		{
			size_t	nCopyBytes =
						m_bufSound[m_iBuffering].m_sizeBuffer - nOffset ;
			if ( nCopyBytes > nBytes )
			{
				nCopyBytes = nBytes ;
			}
			memmove( m_bufSound[m_iBuffering].m_pbytBuffer
								+ nOffset, pbytSound, nCopyBytes ) ;
			if ( nOffset > m_bufSound[m_iBuffering].m_sizeStuffed )
			{
//				m_nBasePosition -=
//					nOffset - m_bufSound[m_iBuffering].m_sizeStuffed ;
			}
			m_bufSound[m_iBuffering].m_sizeStuffed = nOffset + nCopyBytes ;
			//
			UpdateDirectSoundBuffer
				( m_iBuffering * m_nBufferSize + nOffset, nCopyBytes ) ;
			//
			nWrittenBytes += nCopyBytes ;
			pbytSound += nCopyBytes ;
			nBytes -= nCopyBytes ;
			nOffset += nCopyBytes ;
			//
			if ( nBytes == 0 )
			{
				break ;
			}
		}
		else
		{
			ESLAssert( nOffset >= m_bufSound[m_iBuffering].m_sizeBuffer ) ;
			ESLAssert( nBytes > 0 ) ;
			size_t	iNextBuffer = (m_iBuffering + 1) % countBuffer ;
			if ( (ssize_t) iNextBuffer == m_iPlayingBuf )
			{
				break ;
			}
			nOffset = m_bufSound[iNextBuffer].m_sizeStuffed ;
			if ( nOffset >= m_bufSound[m_iBuffering].m_sizeBuffer )
			{
				break ;
			}
			m_iBuffering = iNextBuffer ;
		}
	}
	m_mutexSync.Unlock() ;
	//
	return	nWrittenBytes ;
}

// 再生を開始する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirectSoundPlayer::Play( uint64_t nFlags )
{
	if ( m_idsbuf == NULL )
	{
		return	sglErrFailed ;
	}
	if ( !m_flagStreaming )
	{
		//
		// static buffer 再生開始
		//
		if ( m_flagPlayed )
		{
			m_idsbuf->Stop() ;
		}
		m_flagPlayed = false ;
		m_flagPaused = false ;
		m_flagLooping = false ;
		//
		NormalizeLostBuffer() ;
		//
		DWORD	dwFlags = 0 ;
		if ( nFlags & flagPlayLoop )
		{
			dwFlags |= DSBPLAY_LOOPING ;
			m_flagLooping = true ;
		}
		DWORD	dwPos = 0 ;
		m_idsbuf->GetCurrentPosition( &dwPos, NULL ) ;
		if ( dwPos == 0 )
		{
			m_idsbuf->SetCurrentPosition( m_wfxFormat.nBlockAlign ) ;
		}
		if ( m_idsbuf->Play( NULL, NULL, dwFlags ) != DS_OK )
		{
			return	sglErrFailed ;
		}
		m_flagPlayed = true ;
		m_nBasePosition = 0 ;
	}
	else
	{
		//
		// streaming buffer 再生開始
		//
		if ( m_flagPlayed )
		{
			return	sglErrFailed ;
		}
		EndListenerThread() ;
		//
		SSystem::LockTrace( __FILE__, __LINE__ ) ;
		InitializeStreamingBuffer() ;
		CallbackListenerForBuffer( 0, m_bufSound[0].m_sizeBuffer ) ;
		//
		m_flagPlayed = false ;
		m_flagPaused = false ;
		m_flagLooping = false ;
		//
		NormalizeLostBuffer() ;
		UpdateDirectSoundBuffer( 0, m_nBufferSize * 2 ) ;
		//
		m_idsbuf->SetCurrentPosition( 0 ) ;
		m_iPlayingBuf = 0 ;
		if ( m_idsbuf->Play( NULL, NULL, DSBPLAY_LOOPING ) != DS_OK )
		{
			SSystem::Unlock() ;
			return	sglErrFailed ;
		}
		m_flagPlayed = true ;
		m_nBasePosition = 0 ;
		//
		if ( m_pListener != NULL )
		{
			BeginListenerThread() ;
		}
		SSystem::Unlock() ;
	}
	return	sglErrSuccess ;
}

// 再生を停止する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirectSoundPlayer::Stop( void )
{
	if ( m_idsbuf == NULL )
	{
		return	sglErrFailed ;
	}
	EndListenerThread() ;
	//
	if ( !m_flagPlayed )
	{
		return	sglErrFailed ;
	}
	m_idsbuf->Stop() ;
	//
	if ( !m_flagStreaming )
	{
		ESLVerify( m_idsbuf->SetCurrentPosition( 0 ) == DS_OK ) ;
	}
	//
	m_iBuffering = 0 ;
	m_iPlayingBuf = -1 ;
	m_flagPlayed = false ;
	m_flagPaused = false ;
	//
	return	sglErrSuccess ;
}

// 再生を一時停止する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirectSoundPlayer::Pause( void )
{
	if ( m_idsbuf == NULL )
	{
		return	sglErrFailed ;
	}
	if ( !m_flagPlayed || m_flagPaused )
	{
		return	sglErrFailed ;
	}
	//
	// 再生停止
	//
	EndListenerThread() ;
	//
	if ( !m_flagStreaming )
	{
		DWORD	dwPos = 0 ;
		if ( m_idsbuf->GetCurrentPosition( &dwPos, NULL ) == DS_OK )
		{
			if ( dwPos == 0 )
			{
				m_dwPausedPosition = (DWORD) m_bufSound[0].m_sizeBuffer ;
				if ( m_dwPausedPosition >= m_wfxFormat.nBlockAlign )
				{
					m_dwPausedPosition -= m_wfxFormat.nBlockAlign ;
				}
			}
			else
			{
				m_dwPausedPosition = dwPos ;
			}
		}
	}
	else
	{
		size_t	iPlayingBuf = 0 ;
		PollingCurrentPosition( iPlayingBuf, m_dwPausedPosition ) ;
		m_dwPausedPosition += (DWORD) (iPlayingBuf * m_nBufferSize) ;
	}
	//
	m_idsbuf->Stop() ;
	//
	m_flagPaused = true ;
	//
	return	sglErrSuccess ;
}

// 再生を再開する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirectSoundPlayer::Restart( void )
{
	if ( m_idsbuf == NULL )
	{
		return	sglErrFailed ;
	}
	if ( !m_flagPlayed || !m_flagPaused )
	{
		return	sglErrFailed ;
	}
	//
	// 再生再開
	//
	if ( !m_flagStreaming )
	{
		NormalizeLostBuffer() ;
		//
		DWORD	dwFlags = 0 ;
		if ( m_flagLooping )
		{
			dwFlags |= DSBPLAY_LOOPING ;
		}
		m_idsbuf->SetCurrentPosition( m_dwPausedPosition ) ;
		if ( m_idsbuf->Play( NULL, NULL, dwFlags ) != DS_OK )
		{
			return	sglErrFailed ;
		}
		m_flagPaused = false ;
	}
	else
	{
		NormalizeLostBuffer() ;
		//
		m_idsbuf->SetCurrentPosition( m_dwPausedPosition ) ;
		if ( m_idsbuf->Play( NULL, NULL, DSBPLAY_LOOPING ) != DS_OK )
		{
			return	sglErrFailed ;
		}
		m_flagPaused = false ;
		//
		if ( m_pListener != NULL )
		{
			BeginListenerThread() ;
		}
	}
	return	sglErrSuccess ;
}

// 音量取得 [L/R]
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirectSoundPlayer::GetVolume( float32_t* pVolumes, size_t nChannels )
{
	if ( m_idsbuf == NULL )
	{
		return	sglErrFailed ;
	}
	LONG	lPan = DSBPAN_CENTER ;
	LONG	lVolume = DSBVOLUME_MAX ;
	if ( m_idsbuf->GetPan( &lPan ) != DS_OK )
	{
		lPan = DSBPAN_CENTER ;
	}
	if ( m_idsbuf->GetVolume( &lVolume ) != DS_OK )
	{
		lVolume = DSBVOLUME_MAX ;
	}
	float32_t	volume[2] ;
	VolumePanToStereo( volume, lVolume, lPan ) ;
	//
	if ( nChannels == 1 )
	{
		pVolumes[0] = volume[0] ;
	}
	else if ( nChannels >= 2 )
	{
		pVolumes[0] = volume[0] ;
		pVolumes[1] = volume[1] ;
	}
	return	sglErrSuccess ;
}

// 音量設定 [L/R]
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirectSoundPlayer::SetVolume( const float32_t* pVolumes, size_t nChannels )
{
	if ( m_idsbuf == NULL )
	{
		return	sglErrFailed ;
	}
	LONG	lPan = DSBPAN_CENTER ;
	LONG	lVolume = DSBVOLUME_MAX ;
	//
	float32_t	volume[2] = { 1.0, 1.0 } ;
	if ( nChannels >= 2 )
	{
		volume[0] = pVolumes[0] ;
		volume[1] = pVolumes[1] ;
	}
	else if ( nChannels >= 1 )
	{
		volume[0] = pVolumes[0] ;
		volume[1] = pVolumes[0] ;
	}
	VolumeStereoToPan( lVolume, lPan, volume ) ;
	//
	m_idsbuf->SetVolume( lVolume ) ;
	m_idsbuf->SetPan( lPan ) ;
	//
	return	sglErrSuccess ;
}

// 再生中か？
//////////////////////////////////////////////////////////////////////////////
bool SGLDirectSoundPlayer::IsPlaying( void ) const
{
	if ( m_idsbuf == NULL )
	{
		return	false ;
	}
	if ( m_flagPlayed )
	{
		if ( !m_flagStreaming && !m_flagPaused )
		{
			if ( m_flagLooping )
			{
				return	true ;
			}
			DWORD	dwPos = 0 ;
			if ( m_idsbuf->GetCurrentPosition( &dwPos, NULL ) != DS_OK )
			{
				return	false ;
			}
			return	(dwPos > 0) && (dwPos < m_bufSound[0].m_sizeBuffer) ;
		}
		return	true ;
	}
	return	false ;
}

// 一時停止中か？
//////////////////////////////////////////////////////////////////////////////
bool SGLDirectSoundPlayer::IsPaused( void ) const
{
	return	m_flagPaused ;
}

// 再生済みサンプル数を取得する
//////////////////////////////////////////////////////////////////////////////
uint64_t SGLDirectSoundPlayer::GetPlayingPosition( void )
{
	uint64_t	nPlayingPos = 0 ;
	size_t		iPlayingBuf ;
	DWORD		dwPos ;
	m_mutexSync.LockTrace( __FILE__, __LINE__ ) ;
	if ( m_flagPaused )
	{
		nPlayingPos = m_nBasePosition + m_dwPausedPosition ;
		if ( m_wfxFormat.nBlockAlign != 0 )
		{
			nPlayingPos /= m_wfxFormat.nBlockAlign ;
		}
	}
	else
	{
		if ( PollingCurrentPosition( iPlayingBuf, dwPos ) == sglErrSuccess )
		{
			nPlayingPos = m_nBasePosition
							+ iPlayingBuf * m_nBufferSize + dwPos ;
			if ( m_wfxFormat.nBlockAlign != 0 )
			{
				nPlayingPos /= m_wfxFormat.nBlockAlign ;
			}
		}
	}
	m_mutexSync.Unlock() ;
	return	nPlayingPos ;
}

// 再生位置 [/bytes] を設定する（スタティックバッファのみ）
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirectSoundPlayer::SeekPosition( uint64_t nPos )
{
	SGLError	err = sglErrFailed ;
	m_mutexSync.LockTrace( __FILE__, __LINE__ ) ;
	if ( !m_flagStreaming && (m_idsbuf != NULL) )
	{
		m_idsbuf->SetCurrentPosition( (DWORD) nPos + m_wfxFormat.nBlockAlign ) ;
		err = sglErrSuccess ;
	}
	m_mutexSync.Unlock() ;
	return	err ;
}

// コールバック設定
//////////////////////////////////////////////////////////////////////////////
SGLSoundPlayerListener *
	SGLDirectSoundPlayer::SetListener( SGLSoundPlayerListener * listener )
{
	SGLSoundPlayerListener *	pListener ;
	m_mutexSync.LockTrace( __FILE__, __LINE__ ) ;
	pListener = SGLSoundPlayerInterface::SetListener( listener ) ;
	m_mutexSync.Unlock() ;
	return	pListener ;
}

// Volume と Pan から左右チャネルの音量へ変換
//////////////////////////////////////////////////////////////////////////////
void SGLDirectSoundPlayer::VolumePanToStereo
	( float32_t* pVolumes, LONG lVolume, LONG lPan )
{
	double	fpVolume = 1.0 ;
	if ( lVolume < DSBVOLUME_MAX )
	{
		if ( lVolume > DSBVOLUME_MIN )
		{
			fpVolume = pow( 10.0, (double) lVolume / 2000.0 ) ;
		}
		else
		{
			fpVolume = 0.0 ;
		}
	}
	bool	flagRight = false ;
	if ( lPan > DSBPAN_CENTER )
	{
		flagRight = true ;
		lPan = - lPan ;
	}
	double	fpPan = 1.0 ;
	if ( lPan < DSBPAN_CENTER )
	{
		if ( lPan > DSBPAN_LEFT )
		{
			fpPan = pow( 10.0, (double) lPan / 2000.0 ) ;
		}
		else
		{
			fpPan = 0.0 ;
		}
	}
	fpPan *= fpVolume ;
	//
	if ( flagRight )
	{
		pVolumes[0] = (float32_t) fpPan ;
		pVolumes[1] = (float32_t) fpVolume ;
	}
	else
	{
		pVolumes[0] = (float32_t) fpVolume ;
		pVolumes[1] = (float32_t) fpPan ;
	}
}

// 左右チャネルの音量から Volume と Pan へ変換
//////////////////////////////////////////////////////////////////////////////
void SGLDirectSoundPlayer::VolumeStereoToPan
	( LONG& lVolume, LONG& lPan, const float32_t* pVolumes )
{
	double	fpVolume = 1.0 ;
	double	fpPan = 1.0 ;
	double	fpPanSign = -1.0 ;
	//
	fpVolume = pVolumes[0] ;
	fpPan = fpVolume ;
	//
	if ( pVolumes[0] > pVolumes[1] )
	{
		fpVolume = pVolumes[0] ;
		fpPan = pVolumes[1] ;
		fpPanSign = 1 ;
	}
	else
	{
		fpVolume = pVolumes[1] ;
		fpPan = pVolumes[0] ;
	}
	//
	lVolume = DSBVOLUME_MIN ;
	lPan = DSBPAN_CENTER ;
	//
	if ( fpVolume >= 1.0e-6 )
	{
		lVolume = (LONG) (2000.0 * log10( fpVolume )) ;
		if ( lVolume >= DSBVOLUME_MAX )
		{
			lVolume = DSBVOLUME_MAX ;
		}
		else if ( lVolume < DSBVOLUME_MIN )
		{
			lVolume = DSBVOLUME_MIN ;
		}
		fpPan /= fpVolume ;
		if ( fpPan >= 1.0e-6 )
		{
			lPan = (LONG) (2000.0 * log10( fpPan ) * fpPanSign) ;
			if ( lPan < DSBPAN_LEFT )
			{
				lPan = DSBPAN_LEFT ;
			}
			else if ( lPan > DSBPAN_RIGHT )
			{
				lPan = DSBPAN_RIGHT ;
			}
		}
		else if ( fpPanSign < 0 )
		{
			lPan = DSBPAN_RIGHT ;
		}
		else
		{
			lPan = DSBPAN_LEFT ;
		}
	}
}

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
void SGLDirectSoundPlayer::StreamingThreadProc( void * pInstance )
{
	((SGLDirectSoundPlayer*)pInstance)->StreamingProc() ;
}

void SGLDirectSoundPlayer::StreamingProc( void )
{
	//
	// ポーリング間隔を決定
	//
	size_t	nQuantumTime = 16 ;
	if ( (m_wfxFormat.nBlockAlign != 0)
					&& (m_wfxFormat.nSamplesPerSec != 0) ) 
	{
		size_t	nBufferTime =
					m_nBufferSize / m_wfxFormat.nBlockAlign
							* 1000 / m_wfxFormat.nSamplesPerSec ;
		if ( nBufferTime < nQuantumTime )
		{
			nQuantumTime = nBufferTime ;
			if ( nQuantumTime == 0 )
			{
				nQuantumTime = 1 ;
			}
		}
	}
	//
	// ポーリングループ
	//
	while ( m_signalExit.Wait( nQuantumTime ) == errTimeout )
	{
		if ( m_mutexSync.LockTrace( __FILE__, __LINE__, 10 ) == errSuccess )
		{
			NormalizeLostBuffer() ;
			//
			size_t	iPlayingBuf = 0 ;
			DWORD	dwPos = 0 ;
			if ( PollingCurrentPosition( iPlayingBuf, dwPos ) == sglErrSuccess )
			{
				if ( ((ssize_t) iPlayingBuf == m_iPlayingBuf)
					|| (m_bufSound[m_iBuffering].m_sizeStuffed < dwPos) )
				{
					if ( m_pListener != NULL )
					{
						m_mutexSync.Unlock() ;
						//
						bool	flagLocked = true ;
/*						if ( SSystem::Lock( nQuantumTime ) != errSuccess )
						{
							ESLTrace( "Lock timeout for OnStreaming\n" ) ;
							flagLocked = false ;
							for ( ; ; )
							{
								if ( SSystem::Lock( nQuantumTime ) == errSuccess )
								{
									flagLocked = true ;
									break ;
								}
								if ( m_signalExit.Wait( nQuantumTime ) == errSuccess )
								{
									break ;
								}
								ESLTrace( "waiting for OnStreaming...\n" ) ;
							}
						}
*/						if ( flagLocked )
						{
							m_pListener->OnStreaming( this ) ;
//							SSystem::Unlock() ;
							continue ;
						}
						else
						{
							break ;
						}
					}
				}
			}
			m_mutexSync.Unlock() ;
		}
	}
	//
	m_doneThread.SetSignal() ;
}

// 再生位置ポーリングと情報更新
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirectSoundPlayer::PollingCurrentPosition
	( size_t& iPlayingBuf, DWORD& dwPos )
{
	iPlayingBuf = 0 ;
	dwPos = 0 ;
	if ( m_idsbuf->GetCurrentPosition( &dwPos, NULL ) == DS_OK )
	{
		if ( m_flagStreaming )
		{
			m_mutexSync.LockTrace( __FILE__, __LINE__ ) ;
			for ( size_t i = 0; i < countBuffer; i ++ )
			{
				if ( dwPos < m_bufSound[i].m_sizeBuffer )
				{
					iPlayingBuf = i ;
					break ;
				}
				dwPos -= (DWORD) m_bufSound[i].m_sizeBuffer ;
			}
			//
			// バッファ・スイッチ
			//
			if ( m_iPlayingBuf != (ssize_t) iPlayingBuf )
			{
				memset( m_bufSound[m_iPlayingBuf].m_pbytBuffer,
							0, m_bufSound[m_iPlayingBuf].m_sizeBuffer ) ;
				m_bufSound[m_iPlayingBuf].m_sizeStuffed = 0 ;
				//
				UpdateDirectSoundBuffer
						( m_iPlayingBuf * m_nBufferSize, m_nBufferSize ) ;
				//
				m_iPlayingBuf = (ssize_t) iPlayingBuf ;
				if ( iPlayingBuf == 0 )
				{
					m_nBasePosition += m_nBufferSize * countBuffer ;
				}
			}
			m_mutexSync.Unlock() ;
		}
	}
	else
	{
		ESLTrace( "failed IDirectSoundBuffer::GetCurrentPosition\n" ) ;
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

// バッファロスト正常化
//////////////////////////////////////////////////////////////////////////////
void SGLDirectSoundPlayer::NormalizeLostBuffer( void )
{
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
}

// バッファを初期化
//////////////////////////////////////////////////////////////////////////////
void SGLDirectSoundPlayer::InitializeStreamingBuffer( void )
{
	for ( int i = 0; i < countBuffer; i ++ )
	{
		m_bufSound[i].SetLength( m_nBufferSize ) ;
		m_bufSound[i].m_pbytBuffer = m_bufSound[i].GetArrayPtr() ;
		m_bufSound[i].m_sizeBuffer = m_bufSound[i].GetLength() ;
		m_bufSound[i].m_sizeStuffed = 0 ;
		memset( m_bufSound[i].m_pbytBuffer, 0, m_bufSound[i].m_sizeBuffer ) ;
	}
	m_iBuffering = 0 ;
	m_iPlayingBuf = -1 ;
	m_nBasePosition = 0 ;
}

// バッファが指定位置まで満たされるまでリスナを呼び出す
//////////////////////////////////////////////////////////////////////////////
void SGLDirectSoundPlayer::CallbackListenerForBuffer( size_t iBuffer, size_t nBytes )
{
	atomic_int_t	countLocked = SSystem::UnlockAll() ;
	m_mutexSync.LockTrace( __FILE__, __LINE__ ) ;
	if ( m_pListener != NULL )
	{
		while ( m_bufSound[iBuffer].m_sizeStuffed < nBytes )
		{
			size_t	sizeLastStuffed = m_bufSound[iBuffer].m_sizeStuffed ;
			m_mutexSync.Unlock() ;
			SSystem::LockTrace( __FILE__, __LINE__ ) ;
			m_pListener->OnStreaming( this ) ;
			SSystem::Unlock() ;
			m_mutexSync.LockTrace( __FILE__, __LINE__ ) ;
			if ( sizeLastStuffed == m_bufSound[iBuffer].m_sizeStuffed )
			{
				break ;
			}
		}
	}
	m_mutexSync.Unlock() ;
	SSystem::Relock( countLocked ) ;
}

// DirectSoundBuffer にデータを更新する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirectSoundPlayer::UpdateDirectSoundBuffer( size_t nOffset, size_t nBytes )
{
	if ( m_idsbuf == NULL )
	{
		return	sglErrFailed ;
	}
	size_t	nBaseOffset = 0 ;
	for ( size_t i = 0; i < countBuffer; i ++ )
	{
		if ( nOffset < m_bufSound[i].m_sizeBuffer )
		{
			size_t	nCopyBytes = m_bufSound[i].m_sizeBuffer - nOffset ;
			if ( nCopyBytes > nBytes )
			{
				nCopyBytes = nBytes ;
			}
			void *	ptrBuffer = NULL ;
			DWORD	dwBufferBytes = 0 ;
			if ( m_idsbuf->Lock
				( (DWORD) (nBaseOffset + nOffset), (DWORD) nCopyBytes,
					&ptrBuffer, &dwBufferBytes, NULL, NULL, 0 ) == DS_OK )
			{
				memmove( ptrBuffer, m_bufSound[i].m_pbytBuffer + nOffset, nCopyBytes ) ;
				m_idsbuf->Unlock( ptrBuffer, dwBufferBytes, NULL, NULL ) ;
			}
			nOffset += nCopyBytes ;
			nBytes -= nCopyBytes ;
			//
			if ( (nBytes == 0)
				|| (nOffset < m_bufSound[i].m_sizeBuffer) )
			{
				break ;
			}
		}
		nOffset -= m_bufSound[i].m_sizeBuffer ;
		nBaseOffset += m_bufSound[i].m_sizeBuffer ;
	}
	return	sglErrSuccess ;
}

// スレッド開始
//////////////////////////////////////////////////////////////////////////////
void SGLDirectSoundPlayer::BeginListenerThread( void )
{
	if ( !m_flagThread )
	{
		m_signalExit.Initialize( false ) ;
		m_doneThread.Initialize( false ) ;
		m_pThread = SThread::BeginStockThread
							( &StreamingThreadProc, this ) ;
		if ( m_pThread != NULL )
		{
			m_flagThread = true ;
		}
		else
		{
			ESLTrace( "failed to BeginThread for "
						"SGLDirectSoundPlayer listener.\n" ) ;
		}
	}
}

// スレッド終了
//////////////////////////////////////////////////////////////////////////////
void SGLDirectSoundPlayer::EndListenerThread( void )
{
	if ( m_flagThread )
	{
		m_signalExit.SetSignal() ;
		if ( (m_pThread != NULL) && !m_pThread->IsCurrentThread() )
		{
			m_doneThread.Wait() ;
			m_signalExit.Delete() ;
			m_doneThread.Delete() ;
			m_pThread = NULL ;
			m_flagThread = false ;
		}
	}
}

// 初期化
//////////////////////////////////////////////////////////////////////////////
static const GUID SGL_CLSID_DirectSound =
{ 0x47d4d946, 0x62e8, 0x11cf, { 0x93, 0xbc, 0x44, 0x45, 0x53, 0x54, 0x0, 0x0 } } ;
static const GUID SGL_IID_IDirectSound =
{ 0x279AFA83, 0x4981, 0x11CE, { 0xA5, 0x21, 0x00, 0x20, 0xAF, 0x0B, 0xE5, 0x60 } } ;

SGLError SGLDirectSoundPlayer::Initialize
			( const SGLSoundFormat& fmt, LPCGUID pcGuidDev )
{
	QuickLock() ;
	if ( m_idsbPrimary == NULL )
	{
		ESLAssert( m_idsound == NULL ) ;
		if ( SUCCEEDED( ::CoCreateInstance
			( SGL_CLSID_DirectSound, NULL,
				CLSCTX_ALL, SGL_IID_IDirectSound, (void**) &m_idsound ) ) )
		{
			ESLAssert( m_idsound != NULL ) ;
			HRESULT	hr = m_idsound->Initialize( pcGuidDev ) ;
			if ( !SUCCEEDED( hr ) )
			{
				ESLTrace( "Failed to IDirectSound::Initialize() (#%08X)\n", hr ) ;
				hr = m_idsound->Initialize( NULL ) ;
			}
			//
			HWND	hWnd = GetDesktopWindow( ) ;
			hr = m_idsound->SetCooperativeLevel( hWnd, DSSCL_PRIORITY ) ;
			if ( hr != DS_OK )
			{
				ESLTrace( "Failed to IDirectSound::SetCooperativeLevel(DSSCL_NORMAL) (#%08X)\n", hr ) ;
			}
		}
		else
		{
			m_idsound = NULL ;
			QuickUnlock() ;
			return	sglErrFailed ;
		}
		//
		DSBUFFERDESC	dsbd ;
		::eslFillMemory( &dsbd, 0, sizeof(dsbd) ) ;
		dsbd.dwSize = sizeof(dsbd) ;
		dsbd.dwFlags = DSBCAPS_PRIMARYBUFFER ;
		dsbd.dwBufferBytes = 0 ;
		dsbd.lpwfxFormat = NULL ;
		//
		ESLAssert( m_idsbPrimary == NULL ) ;
		if ( m_idsound->CreateSoundBuffer( &dsbd, &m_idsbPrimary, NULL ) == DS_OK )
		{
			ESLAssert( m_idsbPrimary != NULL ) ;
		}
		else
		{
			m_idsbPrimary = NULL ;
			QuickUnlock() ;
			return	sglErrFailed ;
		}
	}
	WAVEFORMATEX	wfx ;
	ConvertWaveFormat( wfx, fmt ) ;
	if ( m_idsbPrimary->SetFormat( &wfx ) != DS_OK )
	{
		ESLTrace( "failed IDirectSoundBuffer::SetFormat\n" ) ;
	}
	QuickUnlock() ;
	return	sglErrSuccess ;
}

SGLError SGLDirectSoundPlayer::Initialize( void )
{
	SGLSoundFormat	fmtSound ;
	fmtSound.format = SakuraGL::formatSoundLinearPCM ;
	fmtSound.frequency = 44100 ;
	fmtSound.channels = 2 ;
	fmtSound.bitsPerSample = 16 ;
	return	SGLDirectSoundPlayer::Initialize( fmtSound ) ;
}

// 終了
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirectSoundPlayer::Finalize( void )
{
	QuickLock() ;
	if ( m_idsbPrimary != NULL )
	{
		m_idsbPrimary->Release() ;
		m_idsbPrimary = NULL ;
	}
	if ( m_idsound != NULL )
	{
		m_idsound->Release() ;
		m_idsound = NULL ;
	}
	QuickUnlock() ;
	return	sglErrSuccess ;
}

// 再生デバイス列挙
//////////////////////////////////////////////////////////////////////////////
void SGLDirectSoundPlayer::EnumerateDSDevices
	( SSystem::SObjectArray<GUID>& aDevGUIDs,
		SSystem::SObjectArray<SSystem::SString>& aDevNames )
{
	EnumContext	ctx ;
	ctx.pDevGUIDs = &aDevGUIDs ;
	ctx.pDevNames = &aDevNames ;
	//
	DirectSoundEnumerate
		( &SGLDirectSoundPlayer::DSEnumCallback, &ctx ) ;
}

BOOL CALLBACK SGLDirectSoundPlayer::DSEnumCallback
	( LPGUID lpGuid, LPCSTR lpcstrDescription,
		LPCSTR lpcstrModule, LPVOID lpContext )
{
	EnumContext *	pctx = (EnumContext*) lpContext ;
	if ( lpGuid != nullptr )
	{
		pctx->pDevGUIDs->Add( new GUID(*lpGuid) ) ;
	}
	else
	{
		pctx->pDevGUIDs->Add( nullptr ) ;
	}
	pctx->pDevNames->Add( new SString(lpcstrDescription) ) ;
	return	TRUE ;
}

void SGLDirectSoundPlayer::EnumerateDevices
	( SSystem::SArray<GUID>& aDevGUIDs,
		SSystem::SObjectArray<SSystem::SString>& aDevNames )
{
	const PROPERTYKEY	PKEY_AudioEndpoint_GUID =
		{ { 0x1da5d803, 0xd492, 0x4edd,
			{ 0x8c, 0x23, 0xe0, 0xc0, 0xff, 0xee, 0x7f, 0x0e } }, 4 } ;
	//
	IMMDeviceEnumerator *	pEnum = NULL ;
	if ( ::CoCreateInstance
		( __uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL,
			__uuidof(IMMDeviceEnumerator), (void**) &pEnum ) != S_OK )
	{
		return ;
	}
	IMMDeviceCollection *	pEndCollect = NULL ;
	if ( pEnum->EnumAudioEndpoints
		( eRender, DEVICE_STATE_ACTIVE, &pEndCollect ) != S_OK )
	{
		pEnum->Release() ;
		return ;
	}
	pEnum->Release() ;
	//
	UINT	nCount = 0 ;
	pEndCollect->GetCount( &nCount ) ;
	if ( nCount == 0 )
	{
		pEndCollect->Release() ;
		return ;
	}
	for ( UINT i = 0; i < nCount; i ++ )
	{
		IMMDevice *	pItem = NULL ;
		pEndCollect->Item( i, &pItem ) ;
		if ( pItem == NULL )
		{
			continue ;
		}
		IPropertyStore *	pPropStore = NULL ;
		PROPVARIANT			varGUID ;
		PROPVARIANT			varName ;
		PropVariantInit( &varGUID ) ;
		PropVariantInit( &varName ) ;
		pItem->OpenPropertyStore( STGM_READ, &pPropStore ) ;
		if ( SUCCEEDED(pPropStore->GetValue
						( PKEY_AudioEndpoint_GUID, &varGUID ))
			&& SUCCEEDED(pPropStore->GetValue
						( PKEY_Device_FriendlyName, &varName )) )
		{
			aDevGUIDs.Add( *(varGUID.puuid) ) ;
			aDevNames.Add( new SString( varName.pwszVal ) ) ;
		}
		PropVariantClear( &varGUID ) ;
		PropVariantClear( &varName ) ;
		pItem->Release();
	}
	pEndCollect->Release() ;
}

// SGLSoundFormat -> WAVEFORMATEX 変換
//////////////////////////////////////////////////////////////////////////////
void SGLDirectSoundPlayer::ConvertWaveFormat
	( WAVEFORMATEX& wfx, const SGLSoundFormat& fmt )
{
	wfx.wFormatTag = WAVE_FORMAT_PCM ;
	wfx.nChannels = (WORD) fmt.channels ;
	wfx.nSamplesPerSec = fmt.frequency ;
	wfx.wBitsPerSample = (WORD) fmt.bitsPerSample ;
	wfx.nBlockAlign = (WORD) (fmt.channels * (fmt.bitsPerSample >> 3)) ;
	wfx.nAvgBytesPerSec = fmt.frequency * wfx.nBlockAlign ;
	wfx.cbSize = 0 ;
}

