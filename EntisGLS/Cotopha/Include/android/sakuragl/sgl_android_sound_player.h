
#if	!defined(__SAKURAGL_MEDIA_ANDROID_SOUND_PLAYER_H__)
#define	__SAKURAGL_MEDIA_ANDROID_SOUND_PLAYER_H__	1

#include <esl/esl_java_object.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// Android AudioTrack 出力インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SGLAndroidSoundPlayer
				: public SGLSoundPlayerInterface, public JNI::JavaObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
			( SGLAndroidSoundPlayer, SGLSoundPlayerInterface, JavaObject )
		// 構築関数
		SGLAndroidSoundPlayer( void ) ;
		// 消滅関数
		virtual ~SGLAndroidSoundPlayer( void ) ;

	protected:
		SGLSoundFormat	m_fmt ;
		jmethodID		m_jmidSetFormat ;
		jmethodID		m_jmidClose ;
		jmethodID		m_jmidWriteStaticByte ;
		jmethodID		m_jmidWriteStaticWord ;
		jmethodID		m_jmidPrepareStreaming ;
		jmethodID		m_jmidWriteStreamingByte ;
		jmethodID		m_jmidWriteStreamingWord ;
		jmethodID		m_jmidPlay ;
		jmethodID		m_jmidStop ;
		jmethodID		m_jmidPause ;
		jmethodID		m_jmidRestart ;
		jmethodID		m_jmidGetVolume ;
		jmethodID		m_jmidSetVolume ;
		jmethodID		m_jmidGetPlayingPosition ;
		jmethodID		m_jmidSeekPosition ;
		jmethodID		m_jmidIsPlaying ;
		jmethodID		m_jmidIsPaused ;
		jmethodID		m_jmidSetStreamingListener ;

	public:
		// フォーマットを指定して出力を準備する
		virtual SGLError Open( const SGLSoundFormat& fmt ) ;
		// 出力用に準備したサウンド出力を解放する
		virtual SGLError Close( void ) ;
		// スタティックバッファを準備して書き込む
		virtual SGLError WriteStatic( const void * ptrSound, size_t nBytes ) ;
		// ストリームバッファを準備する
		virtual SGLError PrepareStream( size_t nBytes ) ;
		// ストリームバッファへ書き出す
		virtual size_t Write( const void * ptrSound, size_t nBytes ) ;
		// 再生を開始する
		virtual SGLError Play( uint64_t nFlags = 0 ) ;
		// 再生を停止する
		virtual SGLError Stop( void ) ;
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
		// 再生済みサンプル数を取得する
		virtual uint64_t GetPlayingPosition( void ) ;
		// 再生位置 [/bytes] を設定する（スタティックバッファのみ）
		virtual SGLError SeekPosition( uint64_t nPos ) ;
		// コールバック設定
		virtual SGLSoundPlayerListener *
					SetListener( SGLSoundPlayerListener * listener ) ;

	} ;

}

#endif

