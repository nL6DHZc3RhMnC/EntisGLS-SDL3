
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/


namespace	ECSSakura2
{
	//////////////////////////////////////////////////////////////////////////
	// メディアプレイヤー・オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	MediaPlayerObject	: public AudioPlayerObject,
									public SakuraGL::SGLMediaPlayerFrameNotification
	{
	protected:
		VirtualMachine *	m_pVM ;
		INT64				m_addrListener ;

		enum	ListenerVectorIndex
		{
			vectorOnFrameUpdate,
			vectorOnEndOfDuration,
		} ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( MediaPlayerObject, AudioPlayerObject )
		// 構築関数
		MediaPlayerObject( VirtualMachine * vm, const wchar_t * pwszType ) ;
		// 消滅関数
		virtual ~MediaPlayerObject( void ) ;
		// コールバック設定
		INT64 SetListenerHandler( INT64 addrListener ) ;

	public:
		// ファイルを開く
		virtual SakuraGL::SGLError Open
			( const wchar_t * pwszFilePath, uint64_t nFlags = 0,
					SSystem::SEnvironmentInterface * pEnv = NULL ) ;
		virtual SakuraGL::SGLError Create
			( SSystem::SFileInterface * file, uint64_t nFlags = 0 ) ;
		// 複製を生成
		virtual AudioPlayerObject * ClonePlayer( void ) ;

	public:
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

	public:
		// フレーム更新通知
		virtual void OnFrameUpdate( SakuraGL::MediaPlayer * player ) ;
		// 再生区間終端到達
		virtual void OnEndOfDuration( SakuraGL::MediaPlayer * player ) ;

	} ;

}


//////////////////////////////////////////////////////////////////////////////
// MediaPlayer スタブ
//////////////////////////////////////////////////////////////////////////////

// new SakuraGL::MediaPlayer
ECS_LIB_DECLARE_EXPORT_NEW_OBJECT(SakuraGL_MediaPlayer) ;

// SGLError GetVideoSize( SGLSize& sizeVideo ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_MediaPlayer_GetVideoSize) ;

// SGLError SetVideoView
//	( Window* pWindow,
//		const SGLImageRect& rectVideo, uint64_t nFlags = 0 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_MediaPlayer_SetVideoView) ;

//SGLError DrawVideo
//	( PaintContext* pPaint, const SGLImageRect& rectDst,
//		uint32_t nFlags = 0, uint32_t nTransparency = 0 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_MediaPlayer_DrawVideo) ;

// SGLMediaPlayerFrameNotification *
//   SetNotificationListener( SGLMediaPlayerFrameNotification * pListener ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_MediaPlayer_SetNotificationListener) ;

