
#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl_media.h>
#include <sakuragl/media/sgl_audio_decoding_player.h>
#include <sakuragl/media/sgl_threading_audio_decoder.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// AudioPlayer 標準実装
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO4
( SakuraGL::SGLAudioDecodingPlayer,
		SGLAudioPlayerInterface,
		SGLAudioBufferReader,
		SGLAudioInputStream, SGLSoundPlayerListener )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLAudioDecodingPlayer::SGLAudioDecodingPlayer( void )
{
	m_flagOpened = modeOpenAuto ;
	m_posNextDecode = 0 ;
	m_nPCMSamples = 0 ;
	m_posStreamPCM = 0 ;
	m_sizeThreshold = 0x400000 ;		// 4MB
	m_pStreamingListener = NULL ;
	m_pMutexUI = SSystem::g_mutexGlobal ;
	//
	for ( int i = 0; i < 0x10; i ++ )
	{
		m_volumes[i] = 1.0f ;
	}
	m_flagPlayerReady = false ;
	m_flagPlayed = false ;
	m_flagPaused = false ;
	m_flagLoop = false ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLAudioDecodingPlayer::~SGLAudioDecodingPlayer( void )
{
	SGLAudioDecodingPlayer::Close() ;
}

// modeOpenAuto/modeOpenAutoStatic の場合の閾値を設定
//////////////////////////////////////////////////////////////////////////////
void SGLAudioDecodingPlayer::SetMemorySizeThreashold( size_t threshold )
{
	m_sizeThreshold = threshold ;
}

// 指定ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAudioDecodingPlayer::Open
	( const wchar_t * pwszFilePath,
		uint64_t nFlags, SSystem::SEnvironmentInterface * pEnv )
{
	Close() ;
	//
	m_decoder = SGLAudioDecoderManager::OpenDecoder( pwszFilePath, pEnv ) ;
	if ( m_decoder == NULL )
	{
		return	sglErrFailed ;
	}
	m_strFilePath = pwszFilePath ;
	//
	m_flagOpened = modeOpenAuto ;
	return	NormalizeAudioStatic( nFlags ) ;
}

SGLError SGLAudioDecodingPlayer::Create
	( SSystem::SFileInterface * file, bool flagOwner, uint64_t nFlags )
{
	Close() ;
	//
	// modeOpenDynamicOnMemory 判定
	//
	uint64_t	mode = nFlags & modeOpenMask ;
	bool	fDynamicOnMemory =
				(nFlags == modeOpenDynamicOnMemory)
					| (nFlags == modeOpenAutoStatic) ;
	if ( (nFlags == modeOpenAuto)
			&& (file->GetLength() <= (int64_t) m_sizeThreshold) )
	{
		fDynamicOnMemory = true ;
	}
	m_flagOpened = modeOpenAuto ;
	m_refFile = file ;
	//
	if ( fDynamicOnMemory )
	{
		m_bufFile.SetLength( (size_t) file->GetLength() ) ;
		file->Read( m_bufFile.GetArray(), m_bufFile.GetLength() ) ;
		m_memfile.AttachMemory
			( m_bufFile.GetArray(), m_bufFile.GetLength() ) ;
		m_bufFile.FinishArray() ;
		//
		if ( flagOwner )
		{
			delete	file ;
		}
		file = &m_memfile ;
		flagOwner = false ;
		//
		m_refFile = NULL ;
		m_flagOpened = modeOpenDynamicOnMemory ;
	}
	//
	// デコーダー生成
	//
	m_decoder = SGLAudioDecoderManager::CreateDecoder( file, flagOwner ) ;
	if ( m_decoder == NULL )
	{
		return	sglErrFailed ;
	}
	return	NormalizeAudioStatic( nFlags ) ;
}

// データを参照する複製プレイヤー生成
//////////////////////////////////////////////////////////////////////////////
SGLAudioPlayerInterface * SGLAudioDecodingPlayer::ClonePlayer( void )
{
	SGLAudioDecodingPlayer *	pPlayer = new SGLAudioDecodingPlayer ;
	pPlayer->CreateReferenceTo( *this ) ;
	pPlayer->SetUIThreadMutex( m_pMutexUI ) ;
	return	pPlayer ;
}

SGLError SGLAudioDecodingPlayer::CreateReferenceTo
						( const SGLAudioDecodingPlayer& adp )
{
	Close() ;
	//
	m_flagOpened = modeOpenStatic ;
	m_fmtPCM = adp.m_fmtPCM ;
	m_optAudio = adp.m_optAudio ;
	m_nPCMSamples = adp.m_nPCMSamples ;
	m_memfile = adp.m_memfile ;
	m_sizeThreshold = adp.m_sizeThreshold ;
	//
	if ( m_flagOpened == modeOpenStatic )
	{
		return	PreapreSoundPlayer() ;
	}
	else if ( m_flagOpened == modeOpenDynamicOnMemory )
	{
		m_memfile.Seek( 0 ) ;
		SGLAudioDecoderInterface *	decoder =
				SGLAudioDecoderManager::CreateDecoder( &m_memfile, false ) ;
		if ( decoder == NULL )
		{
			return	sglErrFailed ;
		}
		m_decoder = new SGLThreadingAudioDecoder( decoder, true ) ;
		ProcessAfterCreateDecoder() ;
	}
	else if ( m_flagOpened != modeOpenAuto )
	{
		m_flagOpened = modeOpenAuto ;
		if ( adp.m_refFile != NULL )
		{
			m_pMutexUI->LockTrace( __FILE__, __LINE__ ) ;
			SFileInterface *	pfile = adp.m_refFile->Duplicate() ;
			m_pMutexUI->Unlock() ;
			if ( pfile != NULL )
			{
				pfile->Seek( 0 ) ;
				return	Create( pfile, true ) ;
			}
		}
		else if ( !adp.m_strFilePath.IsEmpty() )
		{
			return	Open( adp.m_strFilePath ) ;
		}
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

// デコーダー生成後処理
//////////////////////////////////////////////////////////////////////////////
void SGLAudioDecodingPlayer::ProcessAfterCreateDecoder( void )
{
	m_decoder->GetFormat( m_fmtPCM ) ;
	m_decoder->GetOptinalInfo( m_optAudio ) ;
	m_nPCMSamples = m_decoder->GetTotalLength() ;
	m_posNextDecode = 0 ;
}

// PCM 静的展開判定・処理
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAudioDecodingPlayer::NormalizeAudioStatic( uint64_t nFlags )
{
	ProcessAfterCreateDecoder() ;
	//
	// modeOpenStatic 判定
	//
	uint64_t	mode = nFlags & modeOpenMask ;
	bool	fOpenStatic = (nFlags == modeOpenStatic) ;
	if ( (nFlags == modeOpenAuto) | (nFlags == modeOpenAutoStatic) )
	{
		if ( m_fmtPCM.SamplesToBytes
				( m_decoder->GetTotalLength() ) <= m_sizeThreshold )
		{
			fOpenStatic = true ;
		}
	}
	if ( fOpenStatic )
	{
		size_t	nTotalBytes = (size_t) m_fmtPCM.SamplesToBytes( m_nPCMSamples ) ;
		m_bufPCM.SetLength( nTotalBytes ) ;
		//
		SGLAudioDecoderInterface *	decoder = m_decoder ;
		size_t	nNextPos = 0 ;
		decoder->SeekPosition( 0 ) ;
		//
		while ( nNextPos < nTotalBytes )
		{
			size_t	nBytes = decoder->DecodeNext() ;
			if ( nBytes == 0 )
			{
				break ;
			}
			if ( nNextPos + nBytes > nTotalBytes )
			{
				nBytes = nTotalBytes - nNextPos ;
			}
			nNextPos += decoder->ReadDecodedBuffer
							( m_bufPCM.GetArray() + nNextPos, nBytes ) ;
		}
		m_strFilePath.FreeArray() ;
		m_refFile = NULL ;
		m_bufFile.FreeArray() ;
		m_decoder = NULL ;
		//
		m_memfile.AttachMemory
			( m_bufPCM.GetArray(), m_bufPCM.GetLength() ) ;
		m_bufPCM.FinishArray() ;
		//
		m_flagOpened = modeOpenStatic ;
	}
	else
	{
		if ( m_flagOpened == modeOpenAuto )
		{
			m_flagOpened = modeOpenDynamicRead ;
		}
		SGLThreadingAudioDecoder *
			decoder = new SGLThreadingAudioDecoder
								( m_decoder.Detach(), true ) ;
		m_decoder = decoder ;
	}
	return	sglErrSuccess ;
}

// ファイルを閉じる
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAudioDecodingPlayer::Close( void )
{
	Stop() ;
	//
	if ( m_flagPlayerReady )
	{
		m_player.Close() ;
		m_flagPlayerReady = false ;
	}
	//
	m_flagOpened = modeOpenAuto ;
	m_strFilePath.FreeArray() ;
	m_refFile = NULL ;
	m_decoder = NULL ;
	m_posNextDecode = 0 ;
	m_nPCMSamples = 0 ;
	m_bufPCM.FreeArray() ;
	m_bufFile.FreeArray() ;
	m_memfile.AttachMemory( NULL, 0 ) ;
	//
	return	sglErrSuccess ;
}

// 再生を開始する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAudioDecodingPlayer::Play( uint64_t nFlags )
{
	if ( m_flagPlayed )
	{
		Stop() ;
	}
	//
	if ( m_flagOpened == modeOpenAuto )
	{
		return	sglErrFailed ;
	}
	if ( PreapreSoundPlayer() )
	{
		return	sglErrFailed ;
	}
	if ( m_flagOpened == modeOpenStatic )
	{
		if ( m_flagLoop )
		{
			if ( m_player.Play( SGLSoundPlayerInterface::flagPlayLoop ) )
			{
				return	sglErrFailed ;
			}
		}
		else
		{
			if ( m_player.Play( 0 ) )
			{
				return	sglErrFailed ;
			}
		}
		m_nPlayingBase = 0 ;
		m_nLastPlayingBase = 0 ;
		m_nAccWrittenSamples = 0 ;
	}
	else
	{
		m_nPlayingBase = m_posNextDecode ;
		m_nLastPlayingBase = m_posNextDecode ;
		m_nAccWrittenSamples = 0 ;
		//
		if ( m_player.Play( 0 ) )
		{
			return	sglErrFailed ;
		}
		if ( m_pStreamingListener != NULL )
		{
			m_pStreamingListener->OnBeginStreaming( this ) ;
		}
	}
	m_flagPlayed = true ;
	m_flagPaused = false ;
	return	sglErrSuccess ;
}

// オーディオデバイスを準備する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAudioDecodingPlayer::PreapreSoundPlayer( void )
{
	if ( !m_flagPlayerReady )
	{
		if ( m_player.Open( m_fmtPCM ) )
		{
			return	sglErrFailed ;
		}
		if ( m_flagOpened == modeOpenStatic )
		{
			if ( m_flagLoop )
			{
				size_t	nStart = (size_t) m_fmtPCM.SamplesToBytes(m_nLoopStart) ;
				size_t	nEnd = (size_t) m_fmtPCM.SamplesToBytes(m_nLoopEnd) ;
				if ( (int64_t) nStart >= m_memfile.GetLength() )
				{
					nStart = (size_t) m_memfile.GetLength() ;
				}
				if ( (int64_t) nEnd >= m_memfile.GetLength() )
				{
					nEnd = (size_t) m_memfile.GetLength() ;
				}
				m_player.WriteStatic
					( m_memfile.GetMemory() + nStart, nEnd - nStart ) ;
			}
			else
			{
				m_player.WriteStatic
					( m_memfile.GetMemory(),
							(size_t) m_memfile.GetLength() ) ;
			}
			m_player.SetListener( NULL ) ;
		}
		else
		{
			m_player.PrepareStream() ;
			m_player.SetListener( this ) ;
		}
		if ( m_fmtPCM.channels <= 0x10 )
		{
			m_player.SetVolume( &m_volumes[0], m_fmtPCM.channels ) ;
		}
		else
		{
			m_player.SetVolume( &m_volumes[0], 0x10 ) ;
		}
		m_flagPlayerReady = true ;
	}
	return	sglErrSuccess ;
}

// 再生を停止する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAudioDecodingPlayer::Stop( void )
{
	if ( m_flagPlayed )
	{
//		atomic_int_t	countLocked = SSystem::UnlockAll() ;
		m_player.Stop() ;
		//
		m_csSync.Lock() ;
		if ( m_flagPlayed )
		{
			if ( m_flagOpened != modeOpenStatic )
			{
				m_bufPCM.SetLength( 0 ) ;
				m_posStreamPCM = 0 ;
			}
			if ( m_decoder != NULL )
			{
				m_decoder->SeekPosition( 0 ) ;
				m_posNextDecode = 0 ;
			}
			m_flagPlayed = false ;
			m_flagPaused = false ;
		}
		m_csSync.Unlock() ;
//		SSystem::Relock( countLocked ) ;
		//
		if ( m_pStreamingListener != NULL )
		{
			m_pStreamingListener->OnEndStreaming( this ) ;
		}
	}
	return	sglErrSuccess ;
}

// ループポイント[/sample] を設定する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAudioDecodingPlayer::SetLoop
	( bool fLoop, int64_t nStart, int64_t nEnd )
{
	if ( fLoop )
	{
		if ( nStart < 0 )
		{
			nStart = 0 ;
			if ( (m_flagOpened != modeOpenAuto) &&
				(m_optAudio.nFlags & SGLAudioDecoderInterface::flagLoopStart) )
			{
				nStart = m_optAudio.nLoopStart ;
			}
		}
		if ( nEnd < 0 )
		{
			nEnd = m_nPCMSamples ;
			if ( (m_flagOpened != modeOpenAuto) &&
				(m_optAudio.nFlags & SGLAudioDecoderInterface::flagLoopEnd) )
			{
				nEnd = m_optAudio.nLoopEnd ;
			}
		}
		if ( m_flagOpened == modeOpenStatic )
		{
			if ( !m_flagLoop
				|| (m_flagLoop && ((m_nLoopStart != (uint64_t) nStart)
									|| (m_nLoopEnd != (uint64_t) nEnd))) )
			{
				if ( m_flagPlayerReady )
				{
					m_player.Close() ;
					m_flagPlayerReady = false ;
				}
			}
			m_flagLoop = fLoop ;
			m_nLoopStart = nStart ;
			m_nLoopEnd = nEnd ;
			//
			PreapreSoundPlayer() ;
		}
		m_nLoopStart = nStart ;
		m_nLoopEnd = nEnd ;
	}
	m_flagLoop = fLoop ;
	return	sglErrSuccess ;
}

// 再生を一時停止する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAudioDecodingPlayer::Pause( void )
{
	if ( !m_flagPlayed )
	{
		return	sglErrFailed ;
	}
	if ( m_player.Pause() )
	{
		return	sglErrFailed ;
	}
	m_flagPaused = true ;
	return	sglErrSuccess ;
}

// 再生を再開する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAudioDecodingPlayer::Restart( void )
{
	if ( !m_flagPaused )
	{
		return	sglErrFailed ;
	}
	if ( m_player.Restart() )
	{
		return	sglErrFailed ;
	}
	m_flagPaused = false ;
	return	sglErrSuccess ;
}

// 音量取得 [L/R]
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAudioDecodingPlayer::GetVolume( float32_t* pVolumes, size_t nChannels )
{
	if ( nChannels > 0x10 )
	{
		nChannels = 0x10 ;
	}
	if ( m_flagPlayerReady )
	{
		m_player.GetVolume( m_volumes, nChannels ) ;
	}
	for ( size_t i = 0; i < nChannels; i ++ )
	{
		pVolumes[i] = m_volumes[i];
	}
	return	sglErrSuccess ;
}

// 音量設定 [L/R]
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAudioDecodingPlayer::SetVolume( const float32_t* pVolumes, size_t nChannels )
{
	if ( nChannels > 0x10 )
	{
		nChannels = 0x10 ;
	}
	for ( size_t i = 0; i < nChannels; i ++ )
	{
		m_volumes[i] = pVolumes[i] ;
	}
	if ( m_flagPlayerReady )
	{
		return	m_player.SetVolume( m_volumes, nChannels ) ;
	}
	return	sglErrSuccess ;
}

// 再生中か？
//////////////////////////////////////////////////////////////////////////////
bool SGLAudioDecodingPlayer::IsPlaying( void ) const
{
	return	m_flagPlayed && m_player.IsPlaying() ;
}

// 一時停止中か？
//////////////////////////////////////////////////////////////////////////////
bool SGLAudioDecodingPlayer::IsPaused( void ) const
{
	return	m_flagPaused ;
}

// メディアのサンプル周波数を取得する
//////////////////////////////////////////////////////////////////////////////
uint32_t SGLAudioDecodingPlayer::GetSampleFrequency( void ) const
{
	return	m_fmtPCM.frequency ;
}

// メディアの全長 [/sample] を取得する
//////////////////////////////////////////////////////////////////////////////
uint64_t SGLAudioDecodingPlayer::GetTotalLength( void ) const
{
	return	m_nPCMSamples ;
}

// 再生位置 [/sample] を取得する
//////////////////////////////////////////////////////////////////////////////
uint64_t SGLAudioDecodingPlayer::GetPosition( void )
{
	if ( m_flagPlayed )
	{
		ESLAssert( m_flagPlayerReady ) ;
		uint64_t	nPos = m_player.GetPlayingPosition() ;
		if ( m_flagLoop )
		{
			if ( nPos + m_nLastPlayingBase >= m_nLoopEnd )
			{
				return	nPos + m_nPlayingBase ;
			}
			else
			{
				return	nPos + m_nLastPlayingBase ;
			}
		}
		else
		{
			return	nPos + m_nPlayingBase ;
		}
	}
	else
	{
		return	m_posNextDecode ;
	}
}

// 再生位置 [/sample] を変更する
//////////////////////////////////////////////////////////////////////////////
void SGLAudioDecodingPlayer::SeekPosition( uint64_t nPos )
{
	if ( m_flagOpened != modeOpenAuto )
	{
		if ( m_flagOpened == modeOpenStatic )
		{
			PreapreSoundPlayer() ;
			m_player.SeekPosition( m_fmtPCM.SamplesToBytes(nPos) ) ;
		}
		else if ( m_decoder != NULL )
		{
			m_csSync.Lock() ;
			if ( !m_decoder->SeekPosition( nPos ) )
			{
				m_posNextDecode = nPos ;
			}
			m_bufPCM.SetLength( 0 ) ;
			m_posStreamPCM = 0 ;
			m_csSync.Unlock() ;
		}
	}
}

// オーディオストリーム取得
//////////////////////////////////////////////////////////////////////////////
SGLAudioInputStream * SGLAudioDecodingPlayer::GetAudioStream( void )
{
	return	this ;
}

void SGLAudioDecodingPlayer::ReleaseAudioStream( SGLAudioInputStream * pStream )
{
}

// スレッド同期用ミューテックス設定
//////////////////////////////////////////////////////////////////////////////
void SGLAudioDecodingPlayer::SetUIThreadMutex( SSystem::SMutex * pMutex )
{
	m_pMutexUI = pMutex ;
}

// サウンドフォーマットを取得する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAudioDecodingPlayer::GetAudioFormat( SGLSoundFormat & fmt )
{
	if ( m_flagOpened == modeOpenAuto )
	{
		return	sglErrFailed ;
	}
	fmt = m_fmtPCM ;
	return	sglErrSuccess ;
}

// 静的バッファサイズ [byte] を取得する
//////////////////////////////////////////////////////////////////////////////
size_t SGLAudioDecodingPlayer::GetStaticBufferSize( void ) const
{
	if ( m_flagOpened == modeOpenStatic )
	{
		return	(size_t) m_memfile.GetLength() ;
//		return	m_bufPCM.GetLength() ;
	}
	return	0 ;
}

// 静的バッファから読み出す
//////////////////////////////////////////////////////////////////////////////
size_t SGLAudioDecodingPlayer::ReadStaticBuffer
	( void * ptrBuf, size_t nPos, size_t nBytes ) const
{
	size_t	nBufLength = (size_t) m_memfile.GetLength() ;
	if ( nPos + nBytes > nBufLength )
	{
		if ( nPos >= nBufLength )
		{
			return	0 ;
		}
		nBytes = nBufLength - nPos ;
	}
	eslCopyMemory( ptrBuf, m_memfile.GetMemory() + nPos, nBytes ) ;
	return	nBytes ;
	/*
	if ( nPos + nBytes > m_bufPCM.GetLength() )
	{
		if ( nPos >= m_bufPCM.GetLength() )
		{
			return	0 ;
		}
		nBytes = m_bufPCM.GetLength() - nPos ;
	}
	eslCopyMemory( ptrBuf, m_bufPCM.GetConstArray() + nPos, nBytes ) ;
	return	nBytes ;
	*/
}

// ストリーミングリスナ設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAudioDecodingPlayer::AttachStreamingListener( StreamListener * pListener )
{
	m_pStreamingListener = pListener ;
	//
	if ( m_flagOpened == modeOpenStatic )
	{
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

// メディア補助情報取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAudioDecodingPlayer::GetAudioOptinalInfo( SGLMediaOptionalInfo& optinf )
{
	if ( m_flagOpened == modeOpenAuto )
	{
		return	sglErrFailed ;
	}
	optinf.m_nFlags = 0 ;
	if ( m_optAudio.nFlags & SGLAudioDecoderInterface::flagLoopStart )
	{
		optinf.m_nFlags |= SGLMediaOptionalInfo::flagLoopStart ;
		optinf.m_nLoopStart = m_optAudio.nLoopStart ;
	}
	if ( m_optAudio.nFlags & SGLAudioDecoderInterface::flagLoopEnd )
	{
		optinf.m_nFlags |= SGLMediaOptionalInfo::flagLoopEnd ;
		optinf.m_nLoopEnd = m_optAudio.nLoopEnd ;
	}
	if ( m_optAudio.nFlags & SGLAudioDecoderInterface::flagTitle )
	{
		optinf.m_nFlags |= SGLMediaOptionalInfo::flagTitle ;
		optinf.m_strTitle = m_optAudio.pszTitle ;
	}
	if ( m_optAudio.nFlags & SGLAudioDecoderInterface::flagVocalPlayer )
	{
		optinf.m_nFlags |= SGLMediaOptionalInfo::flagVocalPlayer ;
		optinf.m_strPlayer = m_optAudio.pszVocalPlayer ;
	}
	if ( m_optAudio.nFlags & SGLAudioDecoderInterface::flagComposer )
	{
		optinf.m_nFlags |= SGLMediaOptionalInfo::flagComposer ;
		optinf.m_strComposer = m_optAudio.pszComposer ;
	}
	if ( m_optAudio.nFlags & SGLAudioDecoderInterface::flagArranger )
	{
		optinf.m_nFlags |= SGLMediaOptionalInfo::flagArranger ;
		optinf.m_strArranger = m_optAudio.pszArranger ;
	}
	return	sglErrSuccess ;
}

// オーディオストリーム全長取得（未定は-1）[samples]
//////////////////////////////////////////////////////////////////////////////
int64_t SGLAudioDecodingPlayer::GetAudioLength( void ) const
{
	return	GetTotalLength() ;
}

// オーディオストリーム読み込み [samples]
//////////////////////////////////////////////////////////////////////////////
size_t SGLAudioDecodingPlayer::ReadAudio( void * ptrBuf, size_t nSamples )
{
	const size_t	nBlockBytes = m_fmtPCM.channels * m_fmtPCM.bitsPerSample / 8 ;
	size_t			nReadBytes = 0 ;
	if ( m_flagOpened == modeOpenStatic )
	{
		//
		// スタティックバッファ
		//
		nReadBytes =
			ReadStaticBuffer
				( ptrBuf, m_posStreamPCM, nSamples * nBlockBytes ) ;
		m_posStreamPCM += nReadBytes ;
	}
	else
	{
		//
		// ストリーミング
		//
		SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
		while ( nReadBytes < nSamples * nBlockBytes )
		{
			if ( m_posStreamPCM < m_bufPCM.GetLength() )
			{
				// バッファからコピー
				size_t	nLeftBytes = m_bufPCM.GetLength() - m_posStreamPCM ;
				size_t	nCopyBytes = nLeftBytes ;
				if ( nCopyBytes > nSamples * nBlockBytes )
				{
					nCopyBytes = nSamples * nBlockBytes ;
				}
				eslCopyMemory
					( ptrBuf, m_bufPCM.GetConstArray() + m_posStreamPCM, nCopyBytes ) ;
				m_posStreamPCM += nCopyBytes ;
				nReadBytes += nCopyBytes ;
				if ( m_posStreamPCM < m_bufPCM.GetLength() )
				{
					break ;
				}
				m_posStreamPCM = 0 ;
				m_bufPCM.SetLength( 0 ) ;
			}
			//
			// 次のデータデコード
			//
			size_t	nBytes = m_decoder->DecodeNext() ;
			if ( nBytes == 0 )
			{
				break ;
			}
			m_posNextDecode += m_fmtPCM.BytesToSamples( nBytes ) ;
			//
			ESLAssert( m_posStreamPCM == 0 ) ;
			m_bufPCM.SetLength( nBytes ) ;
			m_decoder->ReadDecodedBuffer( m_bufPCM.GetArray(), nBytes ) ;
			m_bufPCM.FinishArray() ;
		}
	}
	return	nReadBytes / nBlockBytes ;
}

// オーディオストリーム位置変更
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAudioDecodingPlayer::SeekAudio( uint64_t nSamples )
{
	const size_t	nBlockBytes = m_fmtPCM.channels * m_fmtPCM.bitsPerSample / 8 ;
	if ( m_flagOpened == modeOpenStatic )
	{
		m_posStreamPCM = (size_t) nSamples * nBlockBytes ;
		return	sglErrSuccess ;
	}
	else if ( m_decoder != NULL )
	{
		m_csSync.Lock() ;
		if ( !m_decoder->SeekPosition( nSamples ) )
		{
			m_posNextDecode = nSamples ;
		}
		m_bufPCM.SetLength( 0 ) ;
		m_posStreamPCM = 0 ;
		m_csSync.Unlock() ;
		return	sglErrSuccess ;
	}
	return	sglErrFailed ;
}

// バッファへの出力タイミング
//////////////////////////////////////////////////////////////////////////////
void SGLAudioDecodingPlayer::OnStreaming( SoundPlayer * player )
{
	//
	// バッファに残っているデータを書き出し
	//
	m_csSync.Lock() ;
	if ( m_posStreamPCM < m_bufPCM.GetLength() )
	{
		size_t	nWritten =
			player->Write
				( m_bufPCM.GetConstArray() + m_posStreamPCM,
						m_bufPCM.GetLength() - m_posStreamPCM ) ;
		if ( m_pStreamingListener != NULL )
		{
			m_pStreamingListener->OnStreaming
				( this, m_bufPCM.GetConstArray() + m_posStreamPCM, nWritten ) ;
		}
		m_posStreamPCM += nWritten ;
		if ( m_posStreamPCM < m_bufPCM.GetLength() )
		{
			m_csSync.Unlock() ;
			return ;
		}
		m_posStreamPCM = 0 ;
		m_bufPCM.SetLength( 0 ) ;
	}
	//
	// 次のデータデコード
	//
	size_t	nBytes = m_decoder->DecodeNext() ;
	if ( nBytes == 0 )
	{
		if ( m_flagLoop )
		{
			m_nLastPlayingBase = m_nPlayingBase ;
			m_nPlayingBase = m_nLoopStart - m_nAccWrittenSamples ;
			//
			m_decoder->SeekPosition( m_nLoopStart ) ;
			m_posNextDecode = m_nLoopStart ;
			nBytes = m_decoder->DecodeNext() ;
		}
		if ( nBytes == 0 )
		{
			m_csSync.Unlock() ;
			if ( GetPosition() >= m_decoder->GetTotalLength() )
			{
				Stop() ;
			}
			return ;
		}
	}
	//
	// データ書き出し
	//
	bool		fLoopEnd = false ;
	uint64_t	nSamples = m_fmtPCM.BytesToSamples( nBytes ) ;
	if ( m_flagLoop && (m_posNextDecode + nSamples >= m_nLoopEnd) )
	{
		fLoopEnd = true ;
		nSamples = m_nLoopEnd - m_posNextDecode ;
		nBytes = (size_t) m_fmtPCM.SamplesToBytes( nSamples ) ;
	}
	m_posNextDecode += nSamples ;
	m_nAccWrittenSamples += nSamples ;
	//
	m_bufPCM.SetLength( nBytes ) ;
	m_decoder->ReadDecodedBuffer( m_bufPCM.GetArray(), nBytes ) ;
	m_bufPCM.FinishArray() ;
	//
	size_t	nWritten = player->Write( m_bufPCM.GetConstArray(), nBytes ) ;
	m_posStreamPCM = nWritten ;
	if ( m_pStreamingListener != NULL )
	{
		m_pStreamingListener->OnStreaming
			( this, m_bufPCM.GetConstArray(), nWritten ) ;
	}
	//
	// ループ処理
	//
	if ( fLoopEnd )
	{
		m_nLastPlayingBase = m_nPlayingBase ;
		m_nPlayingBase = m_nLoopStart - m_nAccWrittenSamples ;
		//
		m_decoder->SeekPosition( m_nLoopStart ) ;
		m_posNextDecode = m_nLoopStart ;
	}
	m_csSync.Unlock() ;
}
