
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/

#if	!defined(__GLSCS_SAKURA2_OBJECT_SOUND_H__)
#define	__GLSCS_SAKURA2_OBJECT_SOUND_H__

namespace	ECSSakura2
{
	//////////////////////////////////////////////////////////////////////////
	// SoundPlayer オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	SoundPlayerObject
				: public ECSVolatileObject,
					public SakuraGL::SGLSoundPlayerListener
	{
	protected:
		VirtualMachine *					m_pVM ;
		const wchar_t *						m_pwszType ;
		SakuraGL::SGLSoundPlayerInterface *	m_player ;
		bool								m_flagOwner ;
		INT64								m_addrListener ;

		enum	ListenerVectorIndex
		{
			vectorOnStreaming,
		} ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
				( SoundPlayerObject, ECSVolatileObject, SGLSoundPlayerListener )
		// 構築関数
		SoundPlayerObject
			( VirtualMachine * vm, const wchar_t * pwszType,
				SakuraGL::SGLSoundPlayerInterface * player, bool flagOwner ) ;
		// 消滅関数
		virtual ~SoundPlayerObject( void ) ;
		// コールバック設定
		INT64 SetListenerHandler( INT64 addrListener ) ;

	public:
		// 実行時型名
		virtual const wchar_t * GetTypeName( void ) const ;
		// 保存処理
		virtual SSystem::SError SaveStatic
			( SFileInterface * file,
				VirtualMachine * vm, Context * context ) ;
		// 復元処理
		virtual SSystem::SError LoadStatic
			( SFileInterface * file,
				VirtualMachine * vm, Context * context ) ;
		// 復元後処理
		virtual SSystem::SError CommitAfterLoad
			( VirtualMachine * vm, Context * context ) ;

	public:	// SGLSoundPlayerListener 実装
		// バッファへの出力タイミング
		virtual void OnStreaming( SakuraGL::SoundPlayer * player ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// SoundRecorder オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	SoundRecorderObject	: public ECSVolatileObject
	{
	protected:
		const wchar_t *							m_pwszType ;
		SakuraGL::SGLSoundRecorderInterface *	m_recorder ;
		bool									m_flagOwner ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SoundRecorderObject, ECSVolatileObject )
		// 構築関数
		SoundRecorderObject
			( const wchar_t * pwszType,
				SakuraGL::SGLSoundRecorderInterface * recorder, bool flagOwner ) ;
		// 消滅関数
		virtual ~SoundRecorderObject( void ) ;

	public:
		// 実行時型名
		virtual const wchar_t * GetTypeName( void ) const ;
	} ;

}


//////////////////////////////////////////////////////////////////////////////
// SoundPlayer スタブ
//////////////////////////////////////////////////////////////////////////////

// new SakuraGL::SoundPlayer
ECS_LIB_DECLARE_EXPORT_NEW_OBJECT(SakuraGL_SoundPlayer) ;

// SGLError SoundPlayer::Open( const SGLSoundFormat& fmt ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_SoundPlayer_Open) ;

// SGLError Close( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_SoundPlayer_Close) ;

// SGLError WriteStatic( const void * ptrSound, size_t nBytes ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_SoundPlayer_WriteStatic) ;

// SGLError PrepareStream( size_t nBytes ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_SoundPlayer_PrepareStream) ;

// size_t Write( const void * ptrSound, size_t nBytes ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_SoundPlayer_Write) ;

// SGLError Play( uint64_t nFlags = 0 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_SoundPlayer_Play) ;

// SGLError Stop( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_SoundPlayer_Stop) ;

// SGLError Pause( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_SoundPlayer_Pause) ;

// SGLError Restart( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_SoundPlayer_Restart) ;

// SGLError GetVolume( float32_t* pVolumes, size_t nChannels ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_SoundPlayer_GetVolume) ;

// SGLError SetVolume( const float32_t* pVolumes, size_t nChannels ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_SoundPlayer_SetVolume) ;

// bool IsPlaying( void ) const ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_SoundPlayer_IsPlaying) ;

// bool IsPaused( void ) const ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_SoundPlayer_IsPaused) ;

// uint64_t GetPlayingPosition( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_SoundPlayer_GetPlayingPosition) ;

// SGLError SeekPosition( uint64_t nPos ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_SoundPlayer_SeekPosition) ;

// SGLSoundPlayerListener *
//			SetListener( SGLSoundPlayerListener * listener ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_SoundPlayer_SetListener) ;


//////////////////////////////////////////////////////////////////////////////
// SoundRecorder スタブ
//////////////////////////////////////////////////////////////////////////////

// new SakuraGL::SoundPlayer
ECS_LIB_DECLARE_EXPORT_NEW_OBJECT(SakuraGL_SoundRecorder) ;

// size_t EnumerateDevices
//		( uint16_t * pwNameBuf, size_t nNameBufLength ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_SoundRecorder_EnumerateDevices) ;

// SGLError Open( size_t iDevice, const SGLSoundFormat& fmt ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_SoundRecorder_Open) ;

// SGLError Close( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_SoundRecorder_Close) ;

// SGLError PrepareStream( size_t nBytes = 0 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_SoundRecorder_PrepareStream) ;

// size_t Read( void * ptrSound, size_t nBytes ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_SoundRecorder_Read) ;

// SGLError Start( uint64_t nFlags = 0 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_SoundRecorder_Start) ;

// SGLError Stop( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_SoundRecorder_Stop) ;

// SGLError GetVolume( float32_t* pVolumes, size_t nChannels ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_SoundRecorder_GetVolume) ;

// SGLError SetVolume( const float32_t* pVolumes, size_t nChannels ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_SoundRecorder_SetVolume) ;

// bool IsRecording( void ) const ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_SoundRecorder_IsRecording) ;



#endif
