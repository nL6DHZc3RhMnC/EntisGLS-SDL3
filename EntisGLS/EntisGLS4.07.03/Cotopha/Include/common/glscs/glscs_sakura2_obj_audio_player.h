
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/

#if	!defined(__GLSCS_SAKURA2_OBJECT_AUDIO_PLAYER_H__)
#define	__GLSCS_SAKURA2_OBJECT_AUDIO_PLAYER_H__

namespace	ECSSakura2
{
	//////////////////////////////////////////////////////////////////////////
	// オーディオプレイヤー・オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	AudioPlayerObject	: public ECSVolatileObject
	{
	protected:
		const wchar_t *						m_pwszType ;
		SakuraGL::SGLAudioPlayerInterface *	m_player ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( AudioPlayerObject, ECSVolatileObject )
		// 構築関数
		AudioPlayerObject( const wchar_t * pwszType ) ;
		// 消滅関数
		virtual ~AudioPlayerObject( void ) ;

	public:
		// 実行時型名
		virtual const wchar_t * GetTypeName( void ) const ;

	public:
		// ファイルを開く
		virtual SakuraGL::SGLError Open
			( const wchar_t * pwszFilePath, uint64_t nFlags = 0,
					SSystem::SEnvironmentInterface * pEnv = NULL ) ;
		virtual SakuraGL::SGLError Create
			( SSystem::SFileInterface * file, uint64_t nFlags = 0 ) ;
		// 複製を生成
		virtual AudioPlayerObject * ClonePlayer( void ) ;
		// ファイルを閉じる
		virtual SakuraGL::SGLError Close( void ) ;
		// SGLAudioPlayerInterface 取得
		SakuraGL::SGLAudioPlayerInterface * GetAudioPlayer( void ) const
		{
			return	m_player ;
		}

	} ;

}


//////////////////////////////////////////////////////////////////////////////
// AudioPlayer スタブ
//////////////////////////////////////////////////////////////////////////////

// new SakuraGL::AudioPlayer
ECS_LIB_DECLARE_EXPORT_NEW_OBJECT(SakuraGL_AudioPlayer) ;

// SGLError Open
//	( const wchar_t * pwszFilePath, uint64_t nFlags = 0 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_AudioPlayer_Open) ;

// SGLError Create
//	( SSystem::File * pFile, uint64_t nFlags = 0 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_AudioPlayer_Create) ;

// AudioPlayer * ClonePlayer( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_AudioPlayer_ClonePlayer) ;

// SGLError Close( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_AudioPlayer_Close) ;

// SGLError Play( uint64_t nFlags = 0 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_AudioPlayer_Play) ;

// SGLError Stop( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_AudioPlayer_Stop) ;

// SGLError SetLoop
//	( bool fLoop = true, int64_t nStart = -1, int64_t nEnd = -1 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_AudioPlayer_SetLoop) ;

// SGLError Pause( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_AudioPlayer_Pause) ;

// SGLError Restart( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_AudioPlayer_Restart) ;

// SGLError GetVolume( float32_t* pVolumes, size_t nChannels ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_AudioPlayer_GetVolume) ;

// SGLError SetVolume( const float32_t* pVolumes, size_t nChannels ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_AudioPlayer_SetVolume) ;

// bool IsPlaying( void ) const ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_AudioPlayer_IsPlaying) ;

// bool IsPaused( void ) const ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_AudioPlayer_IsPaused) ;

// uint32_t GetSampleFrequency( void ) const ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_AudioPlayer_GetSampleFrequency) ;

// uint64_t GetTotalLength( void ) const ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_AudioPlayer_GetTotalLength) ;

// uint64_t GetPosition( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_AudioPlayer_GetPosition) ;

// void SeekPosition( uint64_t nPos ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_AudioPlayer_SeekPosition) ;


#endif
