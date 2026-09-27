
#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl_media.h>

#if	defined(__PLATFORM_WINDOWS__)
#include <sakuragl/sgl_direct_sound_player.h>

#elif	defined(__PLATFORM_ANDROID__)
#include <sakuragl/sgl_android_sound_player.h>

#endif

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// PCM サウンド出力インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSoundPlayerInterface, SObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSoundPlayerInterface::SGLSoundPlayerInterface( void )
{
	m_pListener = NULL ;
}

// コールバック設定
//////////////////////////////////////////////////////////////////////////////
SGLSoundPlayerListener *
	SGLSoundPlayerInterface::SetListener( SGLSoundPlayerListener * listener )
{
	SGLSoundPlayerListener *	pLastListener = m_pListener ;
	m_pListener = listener ;
	return	pLastListener ;
}


//////////////////////////////////////////////////////////////////////////////
// PCM サウンド出力リスナー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_NV_CLASS_INFO( SakuraGL::SGLSoundPlayerListener )



//////////////////////////////////////////////////////////////////////////////
// PCM サウンド出力ラッパー
//////////////////////////////////////////////////////////////////////////////

SGLSoundPlayer::PFUNC_NEW_PLAYER	SGLSoundPlayer::m_pfnNewPlayer = NULL ;
void *								SGLSoundPlayer::m_pNewPlayerInstance = NULL ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
#if	defined(__COTOPHA__)
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSoundPlayer, SGLSoundPlayerInterface )
#else
ESL_IMPLEMENT_CLASS_INFO_CAST
		( SakuraGL::SGLSoundPlayer, SGLSoundPlayerInterface, m_pPlayer )
#endif

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSoundPlayer::~SGLSoundPlayer( void )
{
	if ( m_flagOwner )
	{
		delete	m_pPlayer ;
		m_pPlayer = NULL ;
		m_flagOwner = false ;
	}
}

// 代入
//////////////////////////////////////////////////////////////////////////////
SoundPlayer * SGLSoundPlayer::SetSoundPlayer
				( SoundPlayer * pPlayer, bool flagOwner )
{
	if ( m_flagOwner )
	{
		delete	m_pPlayer ;
	}
	m_pPlayer = pPlayer ;
	m_flagOwner = flagOwner ;
	return	m_pPlayer ;
}

// プレイヤー生成関数設定
//////////////////////////////////////////////////////////////////////////////
void SGLSoundPlayer::SetPlayerCreator
	( SGLSoundPlayer::PFUNC_NEW_PLAYER pfnNewPlayer, void * pInstance )
{
	QuickLock() ;
	m_pfnNewPlayer = pfnNewPlayer ;
	m_pNewPlayerInstance = pInstance ;
	QuickUnlock() ;
}

// フォーマットを指定して出力を準備する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSoundPlayer::Open( const SGLSoundFormat& fmt )
{
	if ( m_pPlayer == NULL )
	{
		QuickLock() ;
		if ( m_pfnNewPlayer != NULL )
		{
			m_pPlayer = m_pfnNewPlayer( m_pNewPlayerInstance ) ;
			QuickUnlock() ;
		}
		else
		{
			QuickUnlock() ;
			#if	defined(__COTOPHA__)
				m_pPlayer = new SoundPlayer ;
			#elif	defined(__PLATFORM_WINDOWS__)
				m_pPlayer = new SGLDirectSoundPlayer ;
			#elif	defined(__PLATFORM_ANDROID__)
				m_pPlayer = new SGLAndroidSoundPlayer ;
			#else
				#error no implement SGLSoundPlayer::Open
				return	sglErrFailed ;
			#endif
		}
		if ( m_pListener != NULL )
		{
			m_pPlayer->SetListener( m_pListener ) ;
		}
		m_flagOwner = true ;
	}
	return	m_pPlayer->Open( fmt ) ;
}

// 出力用に準備したサウンド出力を解放する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSoundPlayer::Close( void )
{
	if ( m_pPlayer == NULL )
	{
		return	sglErrSuccess ;
	}
	return	m_pPlayer->Close() ;
}

// スタティックバッファを準備して書き込む
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSoundPlayer::WriteStatic( const void * ptrSound, size_t nBytes )
{
	if ( m_pPlayer == NULL )
	{
		return	sglErrFailed ;
	}
	return	m_pPlayer->WriteStatic( ptrSound, nBytes ) ;
}

// ストリームバッファを準備する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSoundPlayer::PrepareStream( size_t nBytes )
{
	if ( m_pPlayer == NULL )
	{
		return	sglErrFailed ;
	}
	return	m_pPlayer->PrepareStream( nBytes ) ;
}

// ストリームバッファへ書き出す
//////////////////////////////////////////////////////////////////////////////
size_t SGLSoundPlayer::Write( const void * ptrSound, size_t nBytes )
{
	if ( m_pPlayer == NULL )
	{
		return	0 ;
	}
	return	m_pPlayer->Write( ptrSound, nBytes ) ;
}

// 再生を開始する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSoundPlayer::Play( uint64_t nFlags )
{
	if ( m_pPlayer == NULL )
	{
		return	sglErrFailed ;
	}
	return	m_pPlayer->Play( nFlags ) ;
}

// 再生を停止する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSoundPlayer::Stop( void )
{
	if ( m_pPlayer == NULL )
	{
		return	sglErrFailed ;
	}
	return	m_pPlayer->Stop() ;
}

// 再生を一時停止する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSoundPlayer::Pause( void )
{
	if ( m_pPlayer == NULL )
	{
		return	sglErrFailed ;
	}
	return	m_pPlayer->Pause() ;
}

// 再生を再開する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSoundPlayer::Restart( void )
{
	if ( m_pPlayer == NULL )
	{
		return	sglErrFailed ;
	}
	return	m_pPlayer->Restart() ;
}

// 音量取得 [L/R]
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSoundPlayer::GetVolume( float32_t* pVolumes, size_t nChannels )
{
	if ( m_pPlayer == NULL )
	{
		return	sglErrFailed ;
	}
	return	m_pPlayer->GetVolume( pVolumes, nChannels ) ;
}

// 音量設定 [L/R]
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSoundPlayer::SetVolume( const float32_t* pVolumes, size_t nChannels )
{
	if ( m_pPlayer == NULL )
	{
		return	sglErrFailed ;
	}
	return	m_pPlayer->SetVolume( pVolumes, nChannels ) ;
}

// 再生中か？
//////////////////////////////////////////////////////////////////////////////
bool SGLSoundPlayer::IsPlaying( void ) const
{
	if ( m_pPlayer == NULL )
	{
		return	false ;
	}
	return	m_pPlayer->IsPlaying() ;
}

// 一時停止中か？
//////////////////////////////////////////////////////////////////////////////
bool SGLSoundPlayer::IsPaused( void ) const
{
	if ( m_pPlayer == NULL )
	{
		return	false ;
	}
	return	m_pPlayer->IsPaused() ;
}

// 再生済みサンプル数を取得する
//////////////////////////////////////////////////////////////////////////////
uint64_t SGLSoundPlayer::GetPlayingPosition( void )
{
	if ( m_pPlayer == NULL )
	{
		return	0 ;
	}
	return	m_pPlayer->GetPlayingPosition() ;
}

// 再生位置 [/bytes] を設定する（スタティックバッファのみ）
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSoundPlayer::SeekPosition( uint64_t nPos )
{
	if ( m_pPlayer == NULL )
	{
		return	sglErrFailed ;
	}
	return	m_pPlayer->SeekPosition( nPos ) ;
}

// コールバック設定
//////////////////////////////////////////////////////////////////////////////
SGLSoundPlayerListener *
		SGLSoundPlayer::SetListener( SGLSoundPlayerListener * listener )
{
	if ( m_pPlayer == NULL )
	{
		return	SGLSoundPlayerInterface::SetListener( listener ) ;
	}
	SGLSoundPlayerInterface::SetListener( listener ) ;
	return	m_pPlayer->SetListener( listener ) ;
}

