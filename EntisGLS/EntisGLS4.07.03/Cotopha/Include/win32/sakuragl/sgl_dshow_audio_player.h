
#if	!defined(__SAKURAGL_MEDIA_DSHOW_AUDIO_PLAYER_H__)
#define	__SAKURAGL_MEDIA_DSHOW_AUDIO_PLAYER_H__	1

#include <dshow.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// DirectShow オーディオファイル再生インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SGLDirectShowAudioPlayer
					: public SGLAudioPlayerInterface,
							public SSystem::SProcedure
	{
	protected:
		SSystem::SString		m_strFilePath ;

		bool					m_flagPlayed ;
		bool					m_flagPaused ;
		bool					m_flagLoop ;
		int64_t					m_msLoopStart ;
		int64_t					m_msLoopEnd ;

		bool					m_flagThreading ;
		SSystem::SThread		m_thread ;
		SSystem::SSignalEvent	m_eventQuit ;
		SSystem::SMutex *		m_pMutexUI ;

		// DirectShow 再生用
		struct IGraphBuilder *	m_pGraphBuilder ;
		struct IMediaControl *	m_pMediaControl ;
		struct IVideoWindow *	m_pVideoWindow ;
		struct IBasicAudio *	m_pBasicAudio ;
		struct IMediaPosition *	m_pMediaPosition ;
		struct IMediaEvent *	m_pMediaEvent ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
			( SGLDirectShowAudioPlayer, SGLAudioPlayerInterface, SProcedure )
		// 構築関数
		SGLDirectShowAudioPlayer( void ) ;
		// 消滅関数
		virtual ~SGLDirectShowAudioPlayer( void ) ;

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
		// ループ処理用スレッドを開始する
		void BeginLoopingThread( void ) ;
		// ループ処理用スレッドを終了する
		void CloseLoopingThread( void ) ;

	public:	// SProcedure 実装
		// スレッド関数
		virtual void Run( void ) ;
		// ループ処理実装
		virtual bool OnLoopingThread( void ) ;
		// 終端到達
		virtual void NotifyEndOfPlayingDuration( void ) ;
	} ;

}

#endif

