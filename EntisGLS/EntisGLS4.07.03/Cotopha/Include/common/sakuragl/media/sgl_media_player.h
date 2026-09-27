
#include <sakuragl/sgl_window.h>

#if	!defined(__SAKURAGL_MEDIA_MEDIA_PLAYER_H__)
#define	__SAKURAGL_MEDIA_MEDIA_PLAYER_H__	1

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// メディアファイル再生インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SGLMediaPlayerFrameNotification ;
	class	SGLMediaPlayerInterface	: public SGLAudioPlayerInterface
	{
	public:
		// SetVideoView フラグ
		enum	VideoViewFlag
		{
			flagPostUpdate		= 0x0001,	// 可能であればフレームを直接描画せず
											// pWindow->PostUpdate() を呼び出す
			flagUpdateWindow	= 0x0002,	// 同様に pWindow->UpdateWindow() を呼び出す
		} ;
		// クラス情報
		ESL_DECLARE_CLASS_INFO
			( SGLMediaPlayerInterface, SGLAudioPlayerInterface )
		// ビデオサイズを取得する
		virtual SGLError GetVideoSize( SGLSize& sizeVideo ) = 0 ;
		// 表示先を設定する
		virtual SGLError SetVideoView
			( SGLAbstractWindow* pWindow,
				const SGLImageRect& rectVideo, uint64_t nFlags = 0 ) = 0 ;
		// 現在のフレームを描画する
		virtual SGLError DrawVideo
			( SGLPaintContextInterface* pPaint,
				const SGLImageRect& rectDst,
				uint32_t nFlags = 0, uint32_t nTransparency = 0 ) = 0 ;
		// メディア再生通知リスナ設定
		virtual SGLMediaPlayerFrameNotification *
			SetNotificationListener
				( SGLMediaPlayerFrameNotification * pListener ) = 0 ;
		// ビデオストリーム取得
		virtual SGLVideoInputStream * GetVideoStream( void ) = 0 ;
		virtual void ReleaseVideoStream( SGLVideoInputStream * pStream ) = 0 ;
	} ;

	#if	defined(__COTOPHA__)
	class	native MediaPlayer	: public AudioPlayer
	{
	public:
		// SetVideoView フラグ
		enum	VideoViewFlag
		{
			flagPostUpdate		= 0x0001,	// 可能であればフレームを直接描画せず
											// pWindow->PostUpdate() を呼び出す
			flagUpdateWindow	= 0x0002,	// 同様に pWindow->UpdateWindow() を呼び出す
		} ;
		// ビデオサイズを取得する
		native SGLError GetVideoSize( SGLSize& sizeVideo ) ;
		// 表示先を設定する
		native SGLError SetVideoView
			( Window* pWindow,
				const SGLImageRect& rectVideo, uint64_t nFlags = 0 ) ;
		// 現在のフレームを描画する
		native SGLError DrawVideo
			( PaintContext* pPaint, const SGLImageRect& rectDst,
				uint32_t nFlags = 0, uint32_t nTransparency = 0 ) ;
		// メディア再生通知リスナ設定
		native SGLMediaPlayerFrameNotification *
			SetNotificationListener
				( SGLMediaPlayerFrameNotification * pListener ) ;
	} ;
	#else
	typedef	SGLMediaPlayerInterface	MediaPlayer ;
	#endif


	//////////////////////////////////////////////////////////////////////////
	// メディアファイル再生通知リスナ
	//////////////////////////////////////////////////////////////////////////

	class	SGLMediaPlayerFrameNotification
	{
	public:
		// クラス情報
		ESL_DECLARE_NV_CLASS_INFO( SGLMediaPlayerFrameNotification )
		// フレーム更新通知
		#if	defined(__COTOPHA__)
		virtual void OnFrameUpdateCaller( MediaPlayer * player ) ;
		#endif
		virtual void OnFrameUpdate( SGLMediaPlayerInterface * player ) = 0 ;
		// 再生区間終端到達
		virtual void OnEndOfDuration( SGLMediaPlayerInterface * player ) = 0 ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// メディアファイル再生ラッパー
	//////////////////////////////////////////////////////////////////////////

	class	SGLMediaPlayer	: public SGLMediaPlayerInterface
	{
	protected:
		MediaPlayer *	m_pMedia ;
		bool			m_flagOwner ;

		#if	defined(__COTOPHA__)
		SSystem::SFileInterface *	m_pFile ;
		bool						m_flagFileOwner ;
		#endif

		SGLAudioPlayer	m_audio ;		// ライン音量反映用
		size_t			m_iAudioLine ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLMediaPlayer, SGLMediaPlayerInterface )
		// 構築関数
		SGLMediaPlayer( void )
				: m_pMedia(NULL), m_flagOwner(false),
					m_iAudioLine(SGLAudioPlayer::lineComposition) {}
		SGLMediaPlayer( MediaPlayer* pPlayer, bool flagOwner = false )
				: m_audio(pPlayer,false),
					m_pMedia(pPlayer), m_flagOwner(flagOwner),
					m_iAudioLine(SGLAudioPlayer::lineComposition) {}
		// 消滅関数
		virtual ~SGLMediaPlayer( void ) ;
		// 代入
		MediaPlayer * operator = ( MediaPlayer * pPlayer )
		{
			return	SetMediaPlayer( pPlayer ) ;
		}
		MediaPlayer * SetMediaPlayer
				( MediaPlayer * pPlayer, bool flagOwner = false ) ;
		// ポインタ変換
		MediaPlayer * operator -> ( void ) const
		{
			return	m_pMedia ;
		}
		MediaPlayer * GetPlayer( void ) const
		{
			return	m_pMedia ;
		}
		operator MediaPlayer * ( void ) const
		{
			return	m_pMedia ;
		}
		// 音量ライン設定
		void SetAudioVolumeLine( size_t iLine ) ;
		// 音量ライン取得
		size_t GetAudioVolumeLine( void ) const ;

	public:
		// 指定ファイルを開く
		virtual SGLError Open
			( const wchar_t * pwszFilePath, uint64_t nFlags = 0,
					SSystem::SEnvironmentInterface * pEnv = NULL ) ;
		virtual SGLError Create
			( SSystem::SFileInterface * file,
				bool flagOwner = true, uint64_t nFlags = 0 ) ;
		// データを参照する複製プレイヤー生成
		virtual SGLAudioPlayerInterface * ClonePlayer( void ) ;
		// ファイルを閉じる
		virtual SGLError Close( void ) ;
		// 再生を開始する
		virtual SGLError Play( uint64_t nFlags = 0 ) ;
		// 再生を停止する
		virtual SGLError Stop( void ) ;
		// ループポイント[/sample] を設定する
		virtual SGLError SetLoop
			( bool fLoop = true, int64_t nStart = -1, int64_t nEnd = -1 ) ;
		// 再生を一時停止する
		virtual SGLError Pause( void ) ;
		// 再生を再開する
		virtual SGLError Restart( void ) ;
		// 音量取得 [L/R]
		virtual SGLError GetVolume( float32_t* pVolumes, size_t nChannels ) ;
		// 音量設定 [L/R]
		virtual SGLError SetVolume( const float32_t* pVolumes, size_t nChannels ) ;
		// 再生中か？
		virtual bool IsPlaying( void ) const ;
		// 一時停止中か？
		virtual bool IsPaused( void ) const ;
		// メディアのサンプル周波数を取得する
		virtual uint32_t GetSampleFrequency( void ) const ;
		// メディアの全長 [/sample] を取得する
		virtual uint64_t GetTotalLength( void ) const ;
		// 再生位置 [/sample] を取得する
		virtual uint64_t GetPosition( void ) ;
		// 再生位置 [/sample] を変更する
		virtual void SeekPosition( uint64_t nPos ) ;
		// オーディオストリーム取得
		virtual SGLAudioInputStream * GetAudioStream( void ) ;
		virtual void ReleaseAudioStream( SGLAudioInputStream * pStream ) ;
		// スレッド同期用ミューテックス設定
		virtual void SetUIThreadMutex( SSystem::SMutex * pMutex ) ;

	public:
		// ビデオサイズを取得する
		virtual SGLError GetVideoSize( SGLSize& sizeVideo ) ;
		// 表示先を設定する
		virtual SGLError SetVideoView
			( SGLAbstractWindow* pWindow,
				const SGLImageRect& rectVideo, uint64_t nFlags = 0 ) ;
		// 現在のフレームを描画する
		virtual SGLError DrawVideo
			( SGLPaintContextInterface* pPaint,
				const SGLImageRect& rectDst,
				uint32_t nFlags = 0, uint32_t nTransparency = 0 ) ;
		// メディア再生通知リスナ設定
		virtual SGLMediaPlayerFrameNotification *
			SetNotificationListener
				( SGLMediaPlayerFrameNotification * pListener ) ;
		// ビデオストリーム取得
		virtual SGLVideoInputStream * GetVideoStream( void ) ;
		virtual void ReleaseVideoStream( SGLVideoInputStream * pStream ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// ビデオキャプチャー・インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SGLVideoCaptureInterface	: public ESLObject
	{
	public:
		enum	SupportedFlag
		{
			supportedVideoDecoder	= 0x01,
		} ;
		struct	Resolution
		{
			int			iFormat ;
			SGLSize		sizeFrame ;
			double		framesPerSec ;
			uint32_t	nSupported ;
			uint32_t	nVideoFormat ;		// enum SGLImageFormatFlag
		} ;
		class	DeviceInfo	: public ESLObject
		{
		public:
			SSystem::SString			m_strDevID ;
			SSystem::SString			m_strDevName ;
			SSystem::SArray<Resolution>	m_Resolutions ;
		public:
			ESL_DECLARE_CLASS_INFO( DeviceInfo, ESLObject )
			DeviceInfo( void ) ;
			virtual ~DeviceInfo( void ) ;
		} ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLVideoCaptureInterface, ESLObject )
		// デバイス検索
		virtual DeviceInfo *
			GetCaptureDeviceInfo( const wchar_t * pwszDevID ) = 0 ;
		// デバイス列挙
		virtual void EnumerateCaptureDevices
			( SSystem::SObjectArray<DeviceInfo>& aDevInfo ) = 0 ;
		// キャプチャーデバイスを開く
		virtual SGLError OpenCapture
			( const wchar_t * pwszDevName = NULL, int iFormat = 0 ) = 0 ;
		virtual SGLError OpenCapture
			( const DeviceInfo* pDevInfo, int iFormat ) = 0 ;

	public:
		// インスタンス生成
		static SGLMediaPlayerInterface * CreatePlayer( void ) ;
	} ;

}

#endif
