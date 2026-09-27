
#if	!defined(__ROSETTA_MEDIA_H__)
#define	__ROSETTA_MEDIA_H__

#include <sakuragl/media/sgl_audio_player.h>
#include <sakuragl/media/sgl_media_composer.h>

namespace	Rosetta
{
	//////////////////////////////////////////////////////////////////////////
	// サウンド出力オブジェクトクラス
	//////////////////////////////////////////////////////////////////////////

	class	RSSoundPlayerClass	: public RGenericNativeObjectClass
	{
	public:
		// SoundPlayer.Format クラス
		class	FormatClass	: public RSClass
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( FormatClass, RSClass )
			// 構築関数
			FormatClass
				( RSClass * pClass, const wchar_t * pwszClassName = L"Format" ) ;
			// クラス固有仮想関数オーバーライドと
			// クラス static 変数のオーバーロード
			virtual void OverrideVirtuals( RSContext& context ) ;

		public:
			// long samplesToBytes( long samples )
			static RSObject * method_samplesToBytes
				( RSContext& context, void * pInstace,
					RSObject* pThis, RSObject** ppArg, size_t count ) ;
			// long bytesToSamples( long bytes )
			static RSObject * method_bytesToSamples
				( RSContext& context, void * pInstace,
					RSObject* pThis, RSObject** ppArg, size_t count ) ;
			// long samplesToMilliSec( long samples )
			static RSObject * method_samplesToMilliSec
				( RSContext& context, void * pInstace,
					RSObject* pThis, RSObject** ppArg, size_t count ) ;
			// long milliSecToSamples( long millisec )
			static RSObject * method_milliSecToSamples
				( RSContext& context, void * pInstace,
					RSObject* pThis, RSObject** ppArg, size_t count ) ;
		} ;
		// SoundPlayer.Listener オブジェクト
		class	Listener	: public RSGenericObject,
								public SakuraGL::SGLSoundPlayerListener
		{
		public:
			RSVirtualMachine *	m_vm ;

		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO2
				( Listener, RSGenericObject, SGLSoundPlayerListener )
			// 構築関数
			Listener
				( RSVirtualMachine * vm,
					RSClass * pClass, BasicType type = typeGenericObject )
					: RSGenericObject(pClass, type), m_vm(vm) {}
			// 消滅関数
			virtual ~Listener( void ) ;

		public:	// オブジェクト
			// 複製（参照の複製を含む）
			virtual RSObject * DuplicateObject( RSContext& context ) const ;
			// 複製（実体も可能な限り複製）
			virtual RSObject * CloneObject( RSContext& context ) const ;

		public:
			// バッファへの出力タイミング
			virtual void OnStreaming( SakuraGL::SoundPlayer * player ) ;
		} ;
		// SoundPlayer.Listener クラス
		class	ListenerClass	: public RSClass
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( ListenerClass, RSClass )
			// 構築関数
			ListenerClass
				( RSClass * pClass, const wchar_t * pwszClassName = L"Listener" ) ;
			// クラス固有仮想関数オーバーライドと
			// クラス static 変数のオーバーロード
			virtual void OverrideVirtuals( RSContext& context ) ;
		} ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSSoundPlayerClass, RGenericNativeObjectClass )
		// 構築関数
		RSSoundPlayerClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"SoundPlayer" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// ネイティブ型テスト
		virtual bool IsNativeObjectOf( ESLObject * pObj ) const ;
		// this オブジェクトのファイルを取得
		static SakuraGL::SGLSoundPlayerInterface *
			GetThisSoundPlayer( RSContext& context, RSObject* pThis ) ;

	public:
		// void <init>()
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean open( SoundPlayer.Format fmt )
		static RSObject * method_open
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean close()
		static RSObject * method_close
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean writeStatic( Uint8Pointer ptrSound, int nBytes )
		static RSObject * method_writeStatic
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean prepareStream( int nBytes = 0 )
		static RSObject * method_prepareStream
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int write( Uint8Pointer ptrSound, int nBytes )
		static RSObject * method_write
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean play( long nFlags = 0 )
		static RSObject * method_play
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean stop()
		static RSObject * method_stop
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean pause()
		static RSObject * method_pause
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean restart()
		static RSObject * method_restart
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean getVolume( float[] volumes, int nChannels )
		static RSObject * method_getVolume
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean setVolume( float[] volumes, int nChannels )
		static RSObject * method_setVolume
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean isPlaying()
		static RSObject * method_isPlaying
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean isPaused()
		static RSObject * method_isPaused
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// long getPlayingPosition()
		static RSObject * method_getPlayingPosition
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean seekPosition( long nPos )
		static RSObject * method_seekPosition
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setListener( SoundPlayer.Listener listener )
		static RSObject * method_setListener
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// オーディオファイル再生クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSAudioPlayerClass	: public RGenericNativeObjectClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSAudioPlayerClass, RGenericNativeObjectClass )
		// 構築関数
		RSAudioPlayerClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"AudioPlayer" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// ネイティブ型テスト
		virtual bool IsNativeObjectOf( ESLObject * pObj ) const ;
		// this オブジェクトのファイルを取得
		static SakuraGL::SGLAudioPlayer *
			GetThisAudioPlayer( RSContext& context, RSObject* pThis ) ;

	public:
		// void <init>()
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean open( String path, long nFlags = 0 )
		static RSObject * method_open
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean create( RandomAccessFile file, long nFlags = 0 )
		static RSObject * method_create
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// AudioPlayer clonePlayer()
		static RSObject * method_clonePlayer
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean close()
		static RSObject * method_close
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean play( long nFlags = 0 )
		static RSObject * method_play
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean stop()
		static RSObject * method_stop
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean setLoop( boolean flagLoop = true, long nStart = -1, long nEnd = -1 )
		static RSObject * method_setLoop
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean pause()
		static RSObject * method_pause
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean restart()
		static RSObject * method_restart
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean getVolume( float[] pVolumes, int nChannels )
		static RSObject * method_getVolume
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean setVolume( float[] pVolumes, int nChannels )
		static RSObject * method_setVolume
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean isPlaying()
		static RSObject * method_isPlaying
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean isPaused()
		static RSObject * method_isPaused
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int getSampleFrequency()
		static RSObject * method_getSampleFrequency
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// long getTotalLength()
		static RSObject * method_getTotalLength
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// long getPosition()
		static RSObject * method_getPosition
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void seekPosition( long nPos )
		static RSObject * method_seekPosition
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void beginFadeVolume( float[] pVolumes, int nChannels, int msecDuration )
		static RSObject * method_beginFadeVolume
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const boolean isVolumeFading()
		static RSObject * method_isVolumeFading
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void cancelFadeVolume()
		static RSObject * method_cancelFadeVolume
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void flushFadeVolume()
		static RSObject * method_flushFadeVolume
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const long getVolumeLineMask()
		static RSObject * method_getVolumeLineMask
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setVolumeLineMask( long maskLines )
		static RSObject * method_setVolumeLineMask
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setVolumeLine( int iLine )
		static RSObject * method_setVolumeLine
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void resetVolumeLine( int iLine )
		static RSObject * method_resetVolumeLine
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static void setLineVolume( int iLine, double volume )
		static RSObject * method_setLineVolume
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static double getLineVolume( int iLine )
		static RSObject * method_getLineVolume
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// MediaOptionalInfo クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSMediaOptionalInfoClass	: public RSClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSMediaOptionalInfoClass, RSClass )
		// 構築関数
		RSMediaOptionalInfoClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"MediaOptionalInfo" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;

	public:
		// Object -> SGLMediaOptionalInfo 変換
		static void FromObject
			( RSContext& context,
				SakuraGL::SGLMediaOptionalInfo& optinf, RSObject * pObj ) ;
		// Object <- SGLMediaOptionalInfo 変換
		static void ToObject
			( RSContext& context,
				RSObject * pObj, const SakuraGL::SGLMediaOptionalInfo& optinf ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// AudioInputStream クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSAudioInputStreamClass	: public RGenericNativeObjectClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSAudioInputStreamClass, RGenericNativeObjectClass )
		// 構築関数
		RSAudioInputStreamClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"AudioInputStream" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// ネイティブ型テスト
		virtual bool IsNativeObjectOf( ESLObject * pObj ) const ;
		// this オブジェクトのファイルを取得
		static SakuraGL::SGLAudioInputStream *
			GetThisAudioInputStream( RSContext& context, RSObject* pThis ) ;

	public:
		// boolean getAudioFormat( SoundPlayer.Format fmt )
		static RSObject * method_getAudioFormat
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean getAudioOptinalInfo( MediaOptionalInfo optinf )
		static RSObject * method_getAudioOptinalInfo
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const long getAudioLength()
		static RSObject * method_getAudioLength
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int readAudio( Uint8Pointer buf, int nSamples )
		static RSObject * method_readAudio
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean seekAudio( long nSamples )
		static RSObject * method_seekAudio
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// AudioOutputStream クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSAudioOutputStreamClass	: public RGenericNativeObjectClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSAudioOutputStreamClass, RGenericNativeObjectClass )
		// 構築関数
		RSAudioOutputStreamClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"AudioOutputStream" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// ネイティブ型テスト
		virtual bool IsNativeObjectOf( ESLObject * pObj ) const ;
		// this オブジェクトのファイルを取得
		static SakuraGL::SGLAudioOutputStream *
			GetThisAudioOutputStream( RSContext& context, RSObject* pThis ) ;

	public:
		// boolean prepareAudio
		//	( SoundPlayer.Format fmt,
		//		long nSamples = -1, MediaOptionalInfo optinf = null )
		static RSObject * method_prepareAudio
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int writeAudio( Uint8Pointer buf, int nSamples )
		static RSObject * method_writeAudio
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// VideoInputStream クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSVideoInputStreamClass	: public RGenericNativeObjectClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSVideoInputStreamClass, RGenericNativeObjectClass )
		// 構築関数
		RSVideoInputStreamClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"VideoInputStream" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// ネイティブ型テスト
		virtual bool IsNativeObjectOf( ESLObject * pObj ) const ;
		// this オブジェクトのファイルを取得
		static SakuraGL::SGLVideoInputStream *
			GetThisVideoInputStream( RSContext& context, RSObject* pThis ) ;

	public:
		// boolean getImageFormat( Image.BufferInfo fmt )
		static RSObject * method_getImageFormat
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean getVideoOptinalInfo( MediaOptionalInfo optinf )
		static RSObject * method_getVideoOptinalInfo
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const long getVideoLength()
		static RSObject * method_getVideoLength
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// const long getVideoDuration()
		static RSObject * method_getVideoDuration
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean readFrame( Image img )
		static RSObject * method_readFrame
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean seekFrame( long nFrames )
		static RSObject * method_seekFrame
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// VideoOutputStream クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSVideoOutputStreamClass	: public RGenericNativeObjectClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSVideoOutputStreamClass, RGenericNativeObjectClass )
		// 構築関数
		RSVideoOutputStreamClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"VideoOutputStream" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// ネイティブ型テスト
		virtual bool IsNativeObjectOf( ESLObject * pObj ) const ;
		// this オブジェクトのファイルを取得
		static SakuraGL::SGLVideoOutputStream *
			GetThisVideoOutputStream( RSContext& context, RSObject* pThis ) ;

	public:
		// boolean prepareVideo
		//	( Image.BufferInfo fmt,
		//		long nFrames, long nDuration,
		//		MediaOptionalInfo optinf = null )
		static RSObject * method_prepareVideo
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean writeFrame( Image img )
		static RSObject * method_writeFrame
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;

}

#endif

