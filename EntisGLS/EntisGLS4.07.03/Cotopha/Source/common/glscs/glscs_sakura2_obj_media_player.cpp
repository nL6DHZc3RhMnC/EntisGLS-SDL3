
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/


#include <sakuraglx/sakuraglx.h>
#include <sakura/ssys_module.h>
#include <sakuragl/sgl_media.h>
#include <sakuragl/sgl_erisa_lib.h>
#include <sakuragl/media/sgl_mei_media_player.h>
#include <glscs/glscs_sakura2_obj_audio_player.h>
#include <glscs/glscs_sakura2_obj_media_player.h>

#if	defined(__PLATFORM_WINDOWS__)
#include <sakuragl/sgl_dshow_audio_player.h>
#include <sakuragl/sgl_dshow_media_player.h>
#endif

using	namespace SSystem ;
using	namespace SakuraGL ;
using	namespace ECSSakura2 ;
using	namespace ECSSakura2Processor ;


//////////////////////////////////////////////////////////////////////////////
// メディアプレイヤー・オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( ECSSakura2::MediaPlayerObject, AudioPlayerObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
MediaPlayerObject::MediaPlayerObject( VirtualMachine * vm, const wchar_t * pwszType )
	: AudioPlayerObject( pwszType ), m_pVM( vm ), m_addrListener( 0 )
{
	m_player = new SGLMediaPlayer ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
MediaPlayerObject::~MediaPlayerObject( void )
{
}

// コールバック設定
//////////////////////////////////////////////////////////////////////////////
INT64 MediaPlayerObject::SetListenerHandler( INT64 addrListener )
{
	SGLMediaPlayer *
			pPlayer = ESLTypeCast<SGLMediaPlayer>( m_player ) ;
	INT64	addrLastListener = m_addrListener ;
	m_addrListener = addrListener ;
	if ( pPlayer != NULL )
	{
		if ( addrListener != 0 )
		{
			pPlayer->SetNotificationListener( this ) ;
		}
		else
		{
			pPlayer->SetNotificationListener( NULL ) ;
		}
	}
	return	addrLastListener ;
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLError MediaPlayerObject::Open
	( const wchar_t * pwszFilePath,
		uint64_t nFlags, SSystem::SEnvironmentInterface * pEnv )
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
	m_player = new SGLMediaPlayer ;
	err = m_player->Open( pwszFilePath, nFlags, pEnv ) ;
	if ( !err )
	{
		return	sglErrSuccess ;
	}
	delete	m_player ;
	m_player = NULL ;
	return	err ;
}

SakuraGL::SGLError MediaPlayerObject::Create
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
	m_player = new SGLMediaPlayer ;
	err = m_player->Create( file, false, nFlags ) ;
	if ( !err )
	{
		return	sglErrSuccess ;
	}
	delete	m_player ;
	m_player = NULL ;
	return	err ;
}

// 複製を生成
//////////////////////////////////////////////////////////////////////////////
AudioPlayerObject * MediaPlayerObject::ClonePlayer( void )
{
	MediaPlayerObject *	pPlayer = new MediaPlayerObject( m_pVM, m_pwszType ) ;
	if ( m_player != NULL )
	{
		pPlayer->m_player = m_player->ClonePlayer() ;
	}
	return	pPlayer ;
}

// 保存処理
//////////////////////////////////////////////////////////////////////////////
SSystem::SError MediaPlayerObject::SaveStatic
	( SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	AudioPlayerObject::SaveStatic( file, vm, context ) ;
	//
	file->Write( &m_addrListener, sizeof(INT64) ) ;
	//
	return	errSuccess ;
}

// 復元処理
//////////////////////////////////////////////////////////////////////////////
SSystem::SError MediaPlayerObject::LoadStatic
	( SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	AudioPlayerObject::LoadStatic( file, vm, context ) ;
	//
	file->Read( &m_addrListener, sizeof(INT64) ) ;
	//
	SGLMediaPlayer *
			pPlayer = ESLTypeCast<SGLMediaPlayer>( m_player ) ;
	if ( (pPlayer != NULL) && (m_addrListener != 0) )
	{
		pPlayer->SetNotificationListener( this ) ;
	}
	return	errSuccess ;
}

// 復元後処理
//////////////////////////////////////////////////////////////////////////////
SSystem::SError MediaPlayerObject::CommitAfterLoad
	( VirtualMachine * vm, Context * context )
{
	return	AudioPlayerObject::CommitAfterLoad( vm, context ) ;
}

// フレーム更新通知
//////////////////////////////////////////////////////////////////////////////
void MediaPlayerObject::OnFrameUpdate( MediaPlayer * player )
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
				( m_addrListener, vectorOnFrameUpdate, regArg, 2 ) ;
		}
	}
}

// 再生区間終端到達
//////////////////////////////////////////////////////////////////////////////
void MediaPlayerObject::OnEndOfDuration( MediaPlayer * player )
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
				( m_addrListener, vectorOnEndOfDuration, regArg, 2 ) ;
		}
	}
}



//////////////////////////////////////////////////////////////////////////////
// MediaPlayer スタブ
//////////////////////////////////////////////////////////////////////////////

#if	!defined(ENTISGLS4_DLL_IMPORT)

// new SakuraGL::MediaPlayer
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_NEW_OBJECT(SakuraGL_MediaPlayer, context, cls_id)
{
	return	new MediaPlayerObject
				( context->m_pSakura2VM, L"SakuraGL::MediaPlayer" ) ;
}

// SGLError GetVideoSize( SGLSize& sizeVideo ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_MediaPlayer_GetVideoSize, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm,
			SGLMediaPlayerInterface, pPlayer,
			arg, MediaPlayer::GetVideoSize ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, SGLSize, sizeVideo,
			arg[1].i, sizeVideo at MediaPlayer::GetVideoSize ) ;
	//
	context->m_regset[regAcc].i = pPlayer->GetVideoSize( *sizeVideo ) ;
	//
	return	NULL ;
}

// SGLError SetVideoView
//	( Window* pWindow,
//		const SGLImageRect& rectVideo, uint64_t nFlags = 0 ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_MediaPlayer_SetVideoView, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm,
			SGLMediaPlayerInterface, pPlayer,
			arg, MediaPlayer::SetVideoView ) ;
	SGLAbstractWindow *	pWindow =
		ESLTypeCast<SGLAbstractWindow>
			( vm->AtomicObjectFromAddress( (DWORD)(arg[1].i >> 32) ) ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, const SGLImageRect, rectVideo,
			arg[2].i, rectVideo at MediaPlayer::SetVideoView ) ;
	//
	context->m_regset[regAcc].i =
			pPlayer->SetVideoView( pWindow, *rectVideo, arg[3].i ) ;
	//
	return	NULL ;
}

//SGLError DrawVideo
//	( PaintContext* pPaint, const SGLImageRect& rectDst,
//		uint32_t nFlags = 0, uint32_t nTransparency = 0 ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_MediaPlayer_DrawVideo, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm,
			SGLMediaPlayerInterface, pPlayer,
			arg, MediaPlayer::DrawVideo ) ;
	SGLPaintContextInterface *	pPaint =
		ESLTypeCast<SGLPaintContextInterface>
			( vm->AtomicObjectFromAddress( (DWORD)(arg[1].i >> 32) ) ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, const SGLImageRect, rectDst,
			arg[2].i, rectDst at MediaPlayer::DrawVideo ) ;
	//
	context->m_regset[regAcc].i =
			pPlayer->DrawVideo
				( pPaint, *rectDst,
					(uint32_t) arg[3].i, (uint32_t) arg[4].i ) ;
	//
	return	NULL ;
}

// SGLMediaPlayerFrameNotification *
//   SetNotificationListener( SGLMediaPlayerFrameNotification * pListener ) ;
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SakuraGL_MediaPlayer_SetNotificationListener, context, arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm,
			MediaPlayerObject, pPlayer,
			arg, MediaPlayer::SetNotificationListener ) ;
	//
	context->m_regset[regAcc].i = pPlayer->SetListenerHandler( arg[1].i ) ;
	//
	return	NULL ;
}

#endif
