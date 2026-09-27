
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/


#include <sakuraglx/sakuraglx.h>
#include <sakura/ssys_module.h>
#include <sakuragl/sgl_media.h>
#include <sakuragl/media/sgl_sound_recorder.h>
#include <glscs/glscs_sakura2_obj_sound.h>

#if	defined(__PLATFORM_WINDOWS__)
#include <sakuragl/sgl_direct_sound_player.h>

#elif	defined(__PLATFORM_ANDROID__)
#include <sakuragl/sgl_android_sound_player.h>

#endif

using	namespace SSystem ;
using	namespace SakuraGL ;
using	namespace ECSSakura2 ;
using	namespace ECSSakura2Processor ;


//////////////////////////////////////////////////////////////////////////////
// SoundPlayer オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2_CAST
( ECSSakura2::SoundPlayerObject,
	ECSVolatileObject, SGLSoundPlayerListener, m_player )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SoundPlayerObject::SoundPlayerObject
	( VirtualMachine * vm, const wchar_t * pwszType,
		SakuraGL::SGLSoundPlayerInterface * player, bool flagOwner )
: m_pVM(vm), m_pwszType(pwszType), m_player(player),
	m_flagOwner(flagOwner), m_addrListener(0)
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SoundPlayerObject::~SoundPlayerObject( void )
{
	if ( m_flagOwner )
	{
		delete	m_player ;
		m_player = NULL ;
		m_flagOwner = false ;
	}
}

// コールバック設定
//////////////////////////////////////////////////////////////////////////////
INT64 SoundPlayerObject::SetListenerHandler( INT64 addrListener )
{
	INT64	addrLastListener = m_addrListener ;
	m_addrListener = addrListener ;
	if ( addrListener != 0 )
	{
		m_player->SetListener( this ) ;
	}
	else
	{
		m_player->SetListener( NULL ) ;
	}
	return	addrLastListener ;
}

// 実行時型名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * SoundPlayerObject::GetTypeName( void ) const
{
	return	m_pwszType ;
}

// 保存処理
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SoundPlayerObject::SaveStatic
	( SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	ECSVolatileObject::SaveStatic( file, vm, context ) ;
	//
	file->Write( &m_addrListener, sizeof(INT64) ) ;
	//
	return	errSuccess ;
}

// 復元処理
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SoundPlayerObject::LoadStatic
	( SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	ECSVolatileObject::LoadStatic( file, vm, context ) ;
	//
	file->Read( &m_addrListener, sizeof(INT64) ) ;
	if ( (m_player != NULL) && (m_addrListener != 0) )
	{
		m_player->SetListener( this ) ;
	}
	return	errSuccess ;
}

// 復元後処理
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SoundPlayerObject::CommitAfterLoad
	( VirtualMachine * vm, Context * context )
{
	return	ECSVolatileObject::CommitAfterLoad( vm, context ) ;
}

// バッファへの出力タイミング
//////////////////////////////////////////////////////////////////////////////
void SoundPlayerObject::OnStreaming( SakuraGL::SoundPlayer * player )
{
	if ( m_addrListener != 0 )
	{
		StandardVM *	pVM = ESLTypeCast<StandardVM>( m_pVM ) ;
		if ( pVM != NULL )
		{
			Register	regArg[2] ;
			regArg[0].i = m_addrListener ;
			regArg[1].i = (INT64) m_dwHighAddr << 32 ;
			//
			pVM->CallAsyncVirtualOnSysThread
				( m_addrListener, vectorOnStreaming, regArg, 2 ) ;
		}
	}
}


//////////////////////////////////////////////////////////////////////////
// SoundRecorder オブジェクト
//////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO_CAST
	( ECSSakura2::SoundRecorderObject, ECSVolatileObject, m_recorder )

// 構築関数
//////////////////////////////////////////////////////////////////////////
SoundRecorderObject::SoundRecorderObject
	( const wchar_t * pwszType,
		SakuraGL::SGLSoundRecorderInterface * recorder, bool flagOwner )
	: m_pwszType(pwszType), m_recorder(recorder), m_flagOwner(flagOwner)
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////
SoundRecorderObject::~SoundRecorderObject( void )
{
	if ( m_flagOwner )
	{
		delete	m_recorder ;
		m_recorder = NULL ;
		m_flagOwner = false ;
	}
}

// 実行時型名
//////////////////////////////////////////////////////////////////////////
const wchar_t * SoundRecorderObject::GetTypeName( void ) const
{
	return	m_pwszType ;
}


//////////////////////////////////////////////////////////////////////////////
// SoundPlayer スタブ
//////////////////////////////////////////////////////////////////////////////

#if	!defined(ENTISGLS4_DLL_IMPORT)

// new SakuraGL::SoundPlayer
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_NEW_OBJECT( SakuraGL_SoundPlayer, context, cls_id )
{
	SakuraGL::SGLSoundPlayerInterface *	pPlayer = NULL ;
	#if	defined(__PLATFORM_WINDOWS__)
		pPlayer = new SakuraGL::SGLDirectSoundPlayer ;
	#elif	defined(__PLATFORM_ANDROID__)
		pPlayer = new SakuraGL::SGLAndroidSoundPlayer ;
	#else
		#warning	no implement SakuraGL::SoundPlayer
	#endif
	return	new SoundPlayerObject
		( context->m_pSakura2VM, L"SakuraGL::SoundPlayer", pPlayer, true ) ;
}

// SGLError SoundPlayer::Open( const SGLSoundFormat& fmt ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_SoundPlayer_Open, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLSoundPlayerInterface, pPlayer, arg, SoundPlayer::Open ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, SGLSoundFormat, pFormat,
				arg[1].i, fmt at SoundPlayer::Open ) ;
	//
	context->m_regset[regAcc].i = pPlayer->Open( *pFormat ) ;
	//
	return	NULL ;
}

// SGLError Close( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_SoundPlayer_Close, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLSoundPlayerInterface, pPlayer, arg, SoundPlayer::Close ) ;
	//
	context->m_regset[regAcc].i = pPlayer->Close() ;
	//
	return	NULL ;
}

// SGLError WriteStatic( const void * ptrSound, size_t nBytes ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_SoundPlayer_WriteStatic, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLSoundPlayerInterface, pPlayer,
						arg, SoundPlayer::WriteStatic ) ;
	const size_t	nBytes = (size_t) arg[2].i ;
	ECS_DECLARE_SYSCALL_ARRAYVAR
		( context, uint8_t, ptrSound, arg[1].i,
			nBytes, ptrSound at SoundPlayer::WriteStatic ) ;
	//
	context->m_regset[regAcc].i = pPlayer->WriteStatic( ptrSound, nBytes ) ;
	//
	return	NULL ;
}

// SGLError PrepareStream( size_t nBytes ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_SoundPlayer_PrepareStream, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLSoundPlayerInterface, pPlayer,
						arg, SoundPlayer::PrepareStream ) ;
	//
	context->m_regset[regAcc].i = pPlayer->PrepareStream( (size_t) arg[1].i ) ;
	//
	return	NULL ;
}

// size_t Write( const void * ptrSound, size_t nBytes ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_SoundPlayer_Write, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLSoundPlayerInterface, pPlayer,
								arg, SoundPlayer::Write ) ;
	const size_t	nBytes = (size_t) arg[2].i ;
	ECS_DECLARE_SYSCALL_ARRAYVAR
		( context, uint8_t, ptrSound, arg[1].i,
			nBytes, ptrSound at SoundPlayer::WriteStatic ) ;
	//
	context->m_regset[regAcc].i = pPlayer->Write( ptrSound, nBytes ) ;
	//
	return	NULL ;
}

// SGLError Play( uint64_t nFlags = 0 ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_SoundPlayer_Play, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLSoundPlayerInterface, pPlayer, arg, SoundPlayer::Play ) ;
	//
	context->m_regset[regAcc].i = pPlayer->Play( arg[1].i ) ;
	//
	return	NULL ;
}

// SGLError Stop( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_SoundPlayer_Stop, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLSoundPlayerInterface, pPlayer, arg, SoundPlayer::Stop ) ;
	//
	context->m_regset[regAcc].i = pPlayer->Stop() ;
	//
	return	NULL ;
}

// SGLError Pause( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_SoundPlayer_Pause, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLSoundPlayerInterface, pPlayer, arg, SoundPlayer::Pause ) ;
	//
	context->m_regset[regAcc].i = pPlayer->Pause() ;
	//
	return	NULL ;
}

// SGLError Restart( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_SoundPlayer_Restart, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLSoundPlayerInterface, pPlayer, arg, SoundPlayer::Restart ) ;
	//
	context->m_regset[regAcc].i = pPlayer->Restart() ;
	//
	return	NULL ;
}

// SGLError GetVolume( float32_t* pVolumes, size_t nChannels ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_SoundPlayer_GetVolume, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLSoundPlayerInterface, pPlayer,
								arg, SoundPlayer::GetVolume ) ;
	const size_t	nChannels = (size_t) arg[2].i ;
	ECS_DECLARE_SYSCALL_ARRAYVAR
		( context, float32_t, pVolumes, arg[1].i,
			nChannels, pVolumes at SoundPlayer::GetVolume ) ;
	//
	context->m_regset[regAcc].i = pPlayer->GetVolume( pVolumes, nChannels ) ;
	//
	return	NULL ;
}

// SGLError SetVolume( const float32_t* pVolumes, size_t nChannels ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_SoundPlayer_SetVolume, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLSoundPlayerInterface, pPlayer,
								arg, SoundPlayer::SetVolume ) ;
	const size_t	nChannels = (size_t) arg[2].i ;
	ECS_DECLARE_SYSCALL_ARRAYVAR
		( context, float32_t, pVolumes, arg[1].i,
			nChannels, pVolumes at SoundPlayer::GetVolume ) ;
	//
	context->m_regset[regAcc].i = pPlayer->SetVolume( pVolumes, nChannels ) ;
	//
	return	NULL ;
}

// bool IsPlaying( void ) const ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_SoundPlayer_IsPlaying, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLSoundPlayerInterface, pPlayer,
								arg, SoundPlayer::IsPlaying ) ;
	//
	context->m_regset[regAcc].i = pPlayer->IsPlaying() ? -1 : 0 ;
	//
	return	NULL ;
}

// bool IsPaused( void ) const ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_SoundPlayer_IsPaused, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLSoundPlayerInterface, pPlayer,
								arg, SoundPlayer::IsPaused ) ;
	//
	context->m_regset[regAcc].i = pPlayer->IsPaused() ? -1 : 0 ;
	//
	return	NULL ;
}

// uint64_t GetPlayingPosition( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_SoundPlayer_GetPlayingPosition, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLSoundPlayerInterface, pPlayer,
							arg, SoundPlayer::GetPlayingPosition ) ;
	//
	context->m_regset[regAcc].i = pPlayer->GetPlayingPosition() ;
	//
	return	NULL ;
}

// SGLError SeekPosition( uint64_t nPos ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_SoundPlayer_SeekPosition, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLSoundPlayerInterface, pPlayer,
							arg, SoundPlayer::SeekPosition ) ;
	//
	context->m_regset[regAcc].i = pPlayer->SeekPosition( arg[1].i ) ;
	//
	return	NULL ;
}

// SGLSoundPlayerListener *
//			SetListener( SGLSoundPlayerListener * listener ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_SoundPlayer_SetListener, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SoundPlayerObject, pPlayer,
							arg, SoundPlayer::SetListener ) ;
	//
	context->m_regset[regAcc].i = pPlayer->SetListenerHandler( arg[1].i ) ;
	//
	return	NULL ;
}


//////////////////////////////////////////////////////////////////////////////
// SoundRecorder スタブ
//////////////////////////////////////////////////////////////////////////////

// new SakuraGL::SoundPlayer
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_NEW_OBJECT( SakuraGL_SoundRecorder, context, cls_id )
{
	return	new SoundRecorderObject
		( L"SakuraGL::SoundRecorder", new SGLSoundRecorder, true ) ;
}

// size_t EnumerateDevices
//		( uint16_t * pwNameBuf, size_t nNameBufLength ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_SoundRecorder_EnumerateDevices, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLSoundRecorderInterface,
					pRec, arg, SoundRecorder::EnumerateDevices ) ;
	ECS_DECLARE_SYSCALL_ARRAYVAR
		( context, uint16_t, pwNameBuf,
			arg[1].i, arg[2].i, pwNameBuf at SoundRecorder::EnumerateDevices ) ;
	//
	context->m_regset[regAcc].i =
			pRec->EnumerateDevices( pwNameBuf, (size_t) arg[2].i ) ;
	//
	return	NULL ;
}

// SGLError Open( size_t iDevice, const SGLSoundFormat& fmt ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_SoundRecorder_Open, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLSoundRecorderInterface, pRec, arg, SoundRecorder::Open ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, SGLSoundFormat, pFormat,
				arg[2].i, fmt at SoundRecorder::Open ) ;
	//
	context->m_regset[regAcc].i = pRec->Open( (size_t) arg[1].i, *pFormat ) ;
	//
	return	NULL ;
}

// SGLError Close( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_SoundRecorder_Close, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLSoundRecorderInterface, pRec, arg, SoundRecorder::Close ) ;
	//
	context->m_regset[regAcc].i = pRec->Close() ;
	//
	return	NULL ;
}

// SGLError PrepareStream( size_t nBytes = 0 ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_SoundRecorder_PrepareStream, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLSoundRecorderInterface, pRec, arg, SoundRecorder::PrepareStream ) ;
	//
	context->m_regset[regAcc].i = pRec->PrepareStream( (size_t) arg[1].i ) ;
	//
	return	NULL ;
}

// size_t Read( void * ptrSound, size_t nBytes ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_SoundRecorder_Read, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLSoundRecorderInterface, pRec, arg, SoundRecorder::Read ) ;
	ECS_DECLARE_SYSCALL_ARRAYVAR
		( context, uint8_t, ptrSound,
			arg[1].i, arg[2].i, ptrSound at SoundRecorder::Read ) ;
	//
	context->m_regset[regAcc].i = pRec->Read( ptrSound, (size_t) arg[2].i ) ;
	//
	return	NULL ;
}

// SGLError Start( uint64_t nFlags = 0 ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_SoundRecorder_Start, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLSoundRecorderInterface, pRec, arg, SoundRecorder::Start ) ;
	//
	context->m_regset[regAcc].i = pRec->Start( arg[1].i ) ;
	//
	return	NULL ;
}

// SGLError Stop( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_SoundRecorder_Stop, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLSoundRecorderInterface, pRec, arg, SoundRecorder::Stop ) ;
	//
	context->m_regset[regAcc].i = pRec->Stop() ;
	//
	return	NULL ;
}

// SGLError GetVolume( float32_t* pVolumes, size_t nChannels ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_SoundRecorder_GetVolume, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLSoundRecorderInterface, pRec, arg, SoundRecorder::GetVolume ) ;
	ECS_DECLARE_SYSCALL_ARRAYVAR
		( context, float32_t, pVolumes,
			arg[1].i, arg[2].i, pVolumes at SoundRecorder::GetVolume ) ;
	//
	context->m_regset[regAcc].i = pRec->GetVolume( pVolumes, (size_t) arg[2].i ) ;
	//
	return	NULL ;
}

// SGLError SetVolume( const float32_t* pVolumes, size_t nChannels ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_SoundRecorder_SetVolume, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLSoundRecorderInterface, pRec, arg, SoundRecorder::SetVolume ) ;
	ECS_DECLARE_SYSCALL_ARRAYVAR
		( context, float32_t, pVolumes,
			arg[1].i, arg[2].i, pVolumes at SoundRecorder::SetVolume ) ;
	//
	context->m_regset[regAcc].i = pRec->SetVolume( pVolumes, (size_t) arg[2].i ) ;
	//
	return	NULL ;
}

// bool IsRecording( void ) const ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL( SakuraGL_SoundRecorder_IsRecording, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SGLSoundRecorderInterface, pRec, arg, SoundRecorder::IsRecording ) ;
	//
	context->m_regset[regAcc].i = pRec->IsRecording() ? -1 : 0 ;
	//
	return	NULL ;
}

#endif
