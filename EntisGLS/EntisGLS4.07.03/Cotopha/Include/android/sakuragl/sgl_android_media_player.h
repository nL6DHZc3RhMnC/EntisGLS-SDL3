
#if	!defined(__SAKURAGL_MEDIA_ANDROID_MEDIA_PLAYER_H__)
#define	__SAKURAGL_MEDIA_ANDROID_MEDIA_PLAYER_H__	1

#include <esl/esl_java_object.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// Android MediaPlayer インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SGLAndroidMediaPlayer
				: public SGLMediaPlayerInterface, public JNI::JavaObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLAndroidMediaPlayer, SGLMediaPlayerInterface )
		// 構築関数
		SGLAndroidMediaPlayer( bool flagMovie = true ) ;
		// 消滅関数
		virtual ~SGLAndroidMediaPlayer( void ) ;

	protected:
		bool				m_flagMovie ;
		SSystem::SString	m_strMediaPath ;
		jmethodID			m_jmidOpenMovie ;
		jmethodID			m_jmidOpenMovieOnAssets ;
		jmethodID			m_jmidOpenAudio ;
		jmethodID			m_jmidOpenAudioOnAssets ;
		jmethodID			m_jmidClose ;
		jmethodID			m_jmidPlay ;
		jmethodID			m_jmidStop ;
		jmethodID			m_jmidPause ;
		jmethodID			m_jmidRestart ;
		jmethodID			m_jmidGetVolume ;
		jmethodID			m_jmidSetVolume ;
		jmethodID			m_jmidGetPlayingPosition ;
		jmethodID			m_jmidSeekPosition ;
		jmethodID			m_jmidGetSampleFrequency ;
		jmethodID			m_jmidGetTotalLength ;
		jmethodID			m_jmidIsPlaying ;
		jmethodID			m_jmidIsPaused ;
		jmethodID			m_jmidSetLoop ;
		jmethodID			m_jmidGetVideoWidth ;
		jmethodID			m_jmidGetVideoHeight ;

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
		// スレッド同期用ミューテックス設定
		virtual void SetUIThreadMutex( SSystem::SMutex * pMutex ) ;
	} ;

}

#endif

