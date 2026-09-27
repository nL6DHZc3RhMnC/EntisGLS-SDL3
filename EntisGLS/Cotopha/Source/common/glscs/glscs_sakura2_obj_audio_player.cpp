
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/


#include <sakuraglx/sakuraglx.h>
#include <sakura/ssys_module.h>
#include <sakuragl/sgl_media.h>
#include <glscs/glscs_sakura2_obj_audio_player.h>
#include <sakuragl/media/sgl_audio_decoding_player.h>

#if	defined(__PLATFORM_WINDOWS__)
#include <sakuragl/sgl_dshow_audio_player.h>
#endif

using	namespace SSystem ;
using	namespace SakuraGL ;
using	namespace ECSSakura2 ;
using	namespace ECSSakura2Processor ;


//////////////////////////////////////////////////////////////////////////////
// オーディプレイヤー・オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO_CAST
	( ECSSakura2::AudioPlayerObject, ECSVolatileObject, m_player )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
AudioPlayerObject::AudioPlayerObject( const wchar_t * pwszType )
{
	m_pwszType = pwszType ;
	m_player = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
AudioPlayerObject::~AudioPlayerObject( void )
{
	delete	m_player ;
	m_player = NULL ;
}

// 実行時型名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * AudioPlayerObject::GetTypeName( void ) const
{
	return	m_pwszType ;
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLError AudioPlayerObject::Open
	( const wchar_t * pwszFilePath, uint64_t nFlags,
			SSystem::SEnvironmentInterface * pEnv )
{
	SGLError	err ;
	if ( m_player != NULL )
	{
		if ( !m_player->Open( pwszFilePath, nFlags, pEnv ) )
		{
			return	sglErrSuccess ;
		}
		delete	m_player ;
	}
	m_player = new SGLAudioDecodingPlayer ;
	err = m_player->Open( pwszFilePath, nFlags, pEnv ) ;
	if ( !err )
	{
		return	sglErrSuccess ;
	}
	delete	m_player ;
	m_player = NULL ;
	//
	#if	defined(__PLATFORM_WINDOWS__)
		m_player = new SGLDirectShowAudioPlayer ;
		if ( !m_player->Open( pwszFilePath, nFlags, pEnv ) )
		{
			return	sglErrSuccess ;
		}
		delete	m_player ;
		m_player = NULL ;
	#endif
	return	err ;
}

SakuraGL::SGLError AudioPlayerObject::Create
	( SSystem::SFileInterface * file, uint64_t nFlags )
{
	if ( file == NULL )
	{
		return	sglErrFailed ;
	}
	SGLError	err ;
	int64_t		pos = file->GetPosition() ;
	if ( m_player != NULL )
	{
		if ( !m_player->Create( file, false, nFlags ) )
		{
			return	sglErrSuccess ;
		}
		delete	m_player ;
		file->Seek( pos ) ;
	}
	m_player = new SGLAudioDecodingPlayer ;
	err = m_player->Create( file, false, nFlags ) ;
	if ( !err )
	{
		return	sglErrSuccess ;
	}
	delete	m_player ;
	m_player = NULL ;
	file->Seek( pos ) ;
	//
	#if	defined(__PLATFORM_WINDOWS__)
		m_player = new SGLDirectShowAudioPlayer ;
		if ( m_player->Create( file, false, nFlags ) )
		{
			return	sglErrSuccess ;
		}
		delete	m_player ;
		m_player = NULL ;
		file->Seek( pos ) ;
	#endif
	return	err ;
}

// 複製を生成
//////////////////////////////////////////////////////////////////////////////
AudioPlayerObject * AudioPlayerObject::ClonePlayer( void )
{
	AudioPlayerObject *	pPlayer = new AudioPlayerObject( m_pwszType ) ;
	if ( m_player != NULL )
	{
		pPlayer->m_player = m_player->ClonePlayer() ;
	}
	return	pPlayer ;
}

// ファイルを閉じる
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLError AudioPlayerObject::Close( void )
{
	if ( m_player != NULL )
	{
		return	m_player->Close() ;
	}
	return	sglErrSuccess ;
}



//////////////////////////////////////////////////////////////////////////////
// AudioPlayer スタブ
//////////////////////////////////////////////////////////////////////////////

#if	!defined(ENTISGLS4_DLL_IMPORT)

// new SakuraGL::AudioPlayer
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_NEW_OBJECT(SakuraGL_AudioPlayer, context, cls_id)
{
	return	new AudioPlayerObject( L"SakuraGL::AudioPlayer" ) ;
}

// SGLError Open
//	( const wchar_t * pwszFilePath, uint64_t nFlags = 0 ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_AudioPlayer_Open, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, AudioPlayerObject, pPlayer, arg, AudioPlayer::Open ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, const uint16_t, pszFilePath,
				arg[1].i, pszFilePath at AudioPlayer::Open ) ;
	//
	SString	strFilePath = pszFilePath ;
	context->m_regset[regAcc].i =
		pPlayer->Open( strFilePath, arg[2].i, vm->GetEnvironment() ) ;
	//
	return	NULL ;
}

// SGLError Create
//	( SSystem::File * pFile, uint64_t nFlags = 0 ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_AudioPlayer_Create, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, AudioPlayerObject, pPlayer, arg, AudioPlayer::Create ) ;
	SFileInterface *	file =
		ESLTypeCast<SFileInterface>
			( vm->AtomicObjectFromAddress( arg[1].h32 ) ) ;
	//
	context->m_regset[regAcc].i = pPlayer->Create( file, arg[2].i ) ;
	//
	return	NULL ;
}

// AudioPlayer * ClonePlayer( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_AudioPlayer_ClonePlayer, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, AudioPlayerObject, pPlayer, arg, AudioPlayer::ClonePlayer ) ;
	//
	AudioPlayerObject *	pColne = pPlayer->ClonePlayer() ;
	//
	AssertLock() ;
	context->m_regset[regAcc].i =
				vm->AllocateHeapObjectAddress( pColne ) ;
	AssertUnlock() ;
	//
	return	NULL ;
}

// SGLError Close( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_AudioPlayer_Close, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, AudioPlayerObject, pPlayer,
							arg, AudioPlayer::Close ) ;
	//
	context->m_regset[regAcc].i = pPlayer->Close() ;
	//
	return	NULL ;
}

// SGLError Play( uint64_t nFlags = 0 ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_AudioPlayer_Play, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, AudioPlayerObject, pPlayer,
								arg, AudioPlayer::Play ) ;
	//
	SGLAudioPlayerInterface *	pAudio = pPlayer->GetAudioPlayer() ;
	context->m_regset[regAcc].i = sglErrFailed ;
	if ( pAudio != NULL )
	{
		context->m_regset[regAcc].i = pAudio->Play( arg[1].i ) ;
	}
	return	NULL ;
}

// SGLError Stop( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_AudioPlayer_Stop, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, AudioPlayerObject, pPlayer,
								arg, AudioPlayer::Stop ) ;
	//
	SGLAudioPlayerInterface *	pAudio = pPlayer->GetAudioPlayer() ;
	context->m_regset[regAcc].i = sglErrFailed ;
	if ( pAudio != NULL )
	{
		context->m_regset[regAcc].i = pAudio->Stop() ;
	}
	//
	return	NULL ;
}

// SGLError SetLoop
//	( bool fLoop = true, int64_t nStart = -1, int64_t nEnd = -1 ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_AudioPlayer_SetLoop, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, AudioPlayerObject, pPlayer,
								arg, AudioPlayer::SetLoop ) ;
	//
	SGLAudioPlayerInterface *	pAudio = pPlayer->GetAudioPlayer() ;
	context->m_regset[regAcc].i = sglErrFailed ;
	if ( pAudio != NULL )
	{
		context->m_regset[regAcc].i =
			pAudio->SetLoop( (arg[1].i != 0), arg[2].i, arg[3].i ) ;
	}
	//
	return	NULL ;
}

// SGLError Pause( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_AudioPlayer_Pause, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, AudioPlayerObject, pPlayer,
								arg, AudioPlayer::Pause ) ;
	//
	SGLAudioPlayerInterface *	pAudio = pPlayer->GetAudioPlayer() ;
	context->m_regset[regAcc].i = sglErrFailed ;
	if ( pAudio != NULL )
	{
		context->m_regset[regAcc].i = pAudio->Pause() ;
	}
	//
	return	NULL ;
}

// SGLError Restart( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_AudioPlayer_Restart, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, AudioPlayerObject, pPlayer,
								arg, AudioPlayer::Restart ) ;
	//
	SGLAudioPlayerInterface *	pAudio = pPlayer->GetAudioPlayer() ;
	context->m_regset[regAcc].i = sglErrFailed ;
	if ( pAudio != NULL )
	{
		context->m_regset[regAcc].i = pAudio->Restart() ;
	}
	//
	return	NULL ;
}

// SGLError GetVolume( float32_t* pVolumes, size_t nChannels ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_AudioPlayer_GetVolume, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, AudioPlayerObject, pPlayer,
								arg, AudioPlayer::GetVolume ) ;
	ECS_DECLARE_SYSCALL_ARRAYVAR
		( context, float32_t, pVolumes,
				arg[1].i, arg[2].i, AudioPlayer::GetVolume ) ;
	//
	SGLAudioPlayerInterface *	pAudio = pPlayer->GetAudioPlayer() ;
	context->m_regset[regAcc].i = sglErrFailed ;
	if ( pAudio != NULL )
	{
		context->m_regset[regAcc].i =
				pAudio->GetVolume( pVolumes, (size_t) arg[2].i ) ;
	}
	//
	return	NULL ;
}

// SGLError SetVolume( const float32_t* pVolumes, size_t nChannels ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_AudioPlayer_SetVolume, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, AudioPlayerObject, pPlayer,
								arg, AudioPlayer::SetVolume ) ;
	ECS_DECLARE_SYSCALL_ARRAYVAR
		( context, float32_t, pVolumes,
				arg[1].i, arg[2].i, AudioPlayer::SetVolume ) ;
	//
	SGLAudioPlayerInterface *	pAudio = pPlayer->GetAudioPlayer() ;
	context->m_regset[regAcc].i = sglErrFailed ;
	if ( pAudio != NULL )
	{
		context->m_regset[regAcc].i =
				pAudio->SetVolume( pVolumes, (size_t) arg[2].i ) ;
	}
	//
	return	NULL ;
}

// bool IsPlaying( void ) const ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_AudioPlayer_IsPlaying, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, AudioPlayerObject, pPlayer,
								arg, AudioPlayer::IsPlaying ) ;
	//
	SGLAudioPlayerInterface *	pAudio = pPlayer->GetAudioPlayer() ;
	context->m_regset[regAcc].i = 0 ;
	if ( pAudio != NULL )
	{
		context->m_regset[regAcc].i = pAudio->IsPlaying() ? -1 : 0 ;
	}
	//
	return	NULL ;
}

// bool IsPaused( void ) const ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_AudioPlayer_IsPaused, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, AudioPlayerObject, pPlayer,
								arg, AudioPlayer::IsPaused ) ;
	//
	SGLAudioPlayerInterface *	pAudio = pPlayer->GetAudioPlayer() ;
	context->m_regset[regAcc].i = 0 ;
	if ( pAudio != NULL )
	{
		context->m_regset[regAcc].i = pAudio->IsPaused() ? -1 : 0 ;
	}
	//
	return	NULL ;
}

// uint32_t GetSampleFrequency( void ) const ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_AudioPlayer_GetSampleFrequency, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, AudioPlayerObject, pPlayer,
								arg, AudioPlayer::GetSampleFrequency ) ;
	//
	SGLAudioPlayerInterface *	pAudio = pPlayer->GetAudioPlayer() ;
	context->m_regset[regAcc].i = 0 ;
	if ( pAudio != NULL )
	{
		context->m_regset[regAcc].i = pAudio->GetSampleFrequency() ;
	}
	//
	return	NULL ;
}

// uint64_t GetTotalLength( void ) const ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_AudioPlayer_GetTotalLength, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, AudioPlayerObject, pPlayer,
								arg, AudioPlayer::GetTotalLength ) ;
	//
	SGLAudioPlayerInterface *	pAudio = pPlayer->GetAudioPlayer() ;
	context->m_regset[regAcc].i = 0 ;
	if ( pAudio != NULL )
	{
		context->m_regset[regAcc].i = pAudio->GetTotalLength() ;
	}
	//
	return	NULL ;
}

// uint64_t GetPosition( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_AudioPlayer_GetPosition, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, AudioPlayerObject, pPlayer,
								arg, AudioPlayer::GetPosition ) ;
	//
	SGLAudioPlayerInterface *	pAudio = pPlayer->GetAudioPlayer() ;
	context->m_regset[regAcc].i = 0 ;
	if ( pAudio != NULL )
	{
		context->m_regset[regAcc].i = pAudio->GetPosition() ;
	}
	//
	return	NULL ;
}

// void SeekPosition( uint64_t nPos ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_AudioPlayer_SeekPosition, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, AudioPlayerObject, pPlayer,
								arg, AudioPlayer::SeekPosition ) ;
	//
	SGLAudioPlayerInterface *	pAudio = pPlayer->GetAudioPlayer() ;
	if ( pAudio != NULL )
	{
		pAudio->SeekPosition( arg[1].i ) ;
	}
	//
	return	NULL ;
}

#endif
