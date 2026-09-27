
#if	!defined(__SAKURAGL_MEI_MEDIA_COMPOSER_H__)
#define	__SAKURAGL_MEI_MEDIA_COMPOSER_H__	1

#include <sakura/ssys_queue_buffer.h>
#include <sakuragl/media/sgl_sound_player.h>
#include <sakuragl/media/sgl_media_composer.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// MEI ファイル入力ラッパー
	//////////////////////////////////////////////////////////////////////////

	class	SGLMEIMediaInputStream
				: public SGLAudioInputStream,
					public SGLVideoInputStream,
					public ERISA::SGLMovieFilePlayer
	{
	protected:
		bool					m_flagSound ;
		SGLSoundFormat			m_fmtSound ;
		SSystem::SQueueBuffer	m_qbufSound ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO3
			( SGLMEIMediaInputStream,
				SGLAudioInputStream, SGLVideoInputStream, SGLMovieFilePlayer )
		// 構築関数
		SGLMEIMediaInputStream( void ) ;
		// 消滅関数
		virtual ~SGLMEIMediaInputStream( void ) ;

	public:
		// ファイルを開く
		SGLError Open( const wchar_t * pwszFilePath ) ;
		SGLError Open
			( SSystem::SFileInterface * pfile, bool flagOwnFile = false ) ;
		// ファイルを閉じる
		SGLError Close( void ) ;

	public:	// SGLAudioInputStream 実装
		// サウンド形式取得
		virtual SGLError GetAudioFormat( SGLSoundFormat& fmt ) ;
		// メディア補助情報取得
		virtual SGLError GetAudioOptinalInfo( SGLMediaOptionalInfo& optinf ) ;
		// オーディオストリーム全長取得（未定は-1）[samples]
		virtual int64_t GetAudioLength( void ) const ;
		// オーディオストリーム読み込み [samples]
		virtual size_t ReadAudio( void * ptrBuf, size_t nSamples ) ;
		// オーディオストリーム位置変更
		virtual SGLError SeekAudio( uint64_t nSamples ) ;

	public:	// SGLVideoInputStream 実装
		// ビデオ画像形式取得
		virtual SGLError GetImageFormat( SGLImageInfo& imginf ) ;
		// メディア補助情報取得
		virtual SGLError GetVideoOptinalInfo( SGLMediaOptionalInfo& optinf ) ;
		// ビデオストリーム全長取得（未定は-1）[frames]
		virtual int64_t GetVideoLength( void ) const ;
		// ビデオストリーム全長取得（未定は-1）[millisecond]
		virtual int64_t GetVideoDuration( void ) const ;
		// ビデオストリーム読み込み
		virtual SGLError ReadFrame
			( const SGLImageInfo& imginf, uint8_t * pbytBuf ) ;
		// ビデオストリーム位置変更
		virtual SGLError SeekFrame( uint64_t nFrames ) ;

	protected:	// ERISA::SGLMovieFilePlayer 実装
		// 音声出力要求
		virtual bool RequestWaveOut
			( uint32_t channels, uint32_t frequency, uint32_t bps ) ;
		// 音声出力終了
		virtual void CloseWaveOut( void ) ;
		// 音声データ出力
		virtual void PushWaveBuffer( const void * ptrWaveBuf, size_t nBytes ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// MIO ファイル出力ラッパー
	//////////////////////////////////////////////////////////////////////////

	class	SGLMIOAudioOutputStream	: public SGLAudioOutputStream
	{
	protected:
		ERISA::SGLMediaFileWriter			m_mfw ;
		ERISA::MIO_INFO_HEADER				m_mih ;
		ERISA::SGLSoundEncoder::Parameter	m_sencParam ;
		uint64_t							m_nSamples ;
		bool								m_fOpenFile ;
		bool								m_fBeginStream ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLMIOAudioOutputStream, SGLAudioOutputStream ) ;
		// 構築関数
		SGLMIOAudioOutputStream( void ) ;
		// 消滅関数
		virtual ~SGLMIOAudioOutputStream( void ) ;

	public:
		// ファイルを開く
		virtual SGLError Open( const wchar_t * pwszFilePath ) ;
		virtual SGLError Open
			( SSystem::SFileInterface * pfile, bool flagOwnFile = false ) ;
		// ファイルを閉じる
		virtual SGLError Close( void ) ;

	public:
		// 圧縮プリセット設定
		void SetSoundCompressionPreset( ERISA::SGLSoundEncoder::PresetParameter pp ) ;
		// 音声情報ヘッダを設定する（圧縮パラメータのみ）
		void SetMioInfoHeader
				( const ERISA::MIO_INFO_HEADER & mih ) ;
		// 音声の圧縮パラメータを設定する
		void SetSoundCompressionParameter
				( const ERISA::SGLSoundEncoder::Parameter & param ) ;

	protected:
		// ヘッダ書き出し
		virtual SGLError WriteHeaders( const SGLMediaOptionalInfo * pOptInf = NULL ) ;
		// オプション情報変換
		SSystem::SString FormatMediaOption
				( const SGLMediaOptionalInfo * pOptInf ) ;

	public:	// SGLAudioOutputStream 実装
		// オーディオ出力ストリーム準備
		virtual SGLError PrepareAudio
			( const SGLSoundFormat& fmt, int64_t nSamples = -1,
						const SGLMediaOptionalInfo * pOptInf = NULL ) ;
		// オーディオストリーム出力 [samples]
		virtual size_t WriteAudio( const void * ptrBuf, size_t nSamples ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// MEI ファイル出力ラッパー
	//////////////////////////////////////////////////////////////////////////

	class	SGLMEIMediaOutputStream
				: public SGLMIOAudioOutputStream, public SGLVideoOutputStream
	{
	protected:
		ERISA::ERI_INFO_HEADER				m_eih ;
		ERISA::SGLImageEncoder::Parameter	m_iencParam ;
		uint64_t							m_nFrames ;
		size_t								m_nKeyFrame ;
		size_t								m_nBFrames ;
		int64_t								m_nRateFPS ;
		int64_t								m_nScaleFPS ;
		bool								m_fPreparedAudio ;
		bool								m_fPreparedVideo ;
		SSystem::SString					m_strDescription ;
		SGLImage							m_imgFrameBuf ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
			( SGLMEIMediaOutputStream,
				SGLMIOAudioOutputStream, SGLVideoOutputStream ) ;
		// 構築関数
		SGLMEIMediaOutputStream( void ) ;
		// 消滅関数
		virtual ~SGLMEIMediaOutputStream( void ) ;

	public:
		// ファイルを開く
		virtual SGLError Open
			( SSystem::SFileInterface * pfile, bool flagOwnFile = false ) ;
		// ファイルを閉じる
		virtual SGLError Close( void ) ;

	public:
		// キーフレーム間隔を設定
		void SetKeyFrame( size_t nKeyFrame, size_t nBFrames ) ;
		// 圧縮プリセット設定
		void SetImageCompressionPreset
				( ERISA::SGLImageEncoder::PresetParameter pp ) ;
		// 音声情報ヘッダを設定する（圧縮パラメータのみ）
		void SetEriInfoHeader
				( const ERISA::ERI_INFO_HEADER & eih ) ;
		// 音声の圧縮パラメータを設定する
		void SetImageCompressionParameter
				( const ERISA::SGLImageEncoder::Parameter & param ) ;

	protected:
		// ヘッダ書き出し
		virtual SGLError WriteFileHeader( void ) ;

	public:	// SGLAudioOutputStream 実装
		// オーディオ出力ストリーム準備
		virtual SGLError PrepareAudio
			( const SGLSoundFormat& fmt, int64_t nSamples = -1,
						const SGLMediaOptionalInfo * pOptInf = NULL ) ;
		// オーディオストリーム出力 [samples]
		virtual size_t WriteAudio( const void * ptrBuf, size_t nSamples ) ;

	public:	// SGLVideoOutputStream 実装
		// ビデオ出力ストリーム準備
		virtual SGLError PrepareVideo
			( const SGLImageInfo& imginf,
				int64_t nFrames, int64_t nDuration,
				const SGLMediaOptionalInfo * pOptInf = NULL ) ;
		// ビデオストリーム出力
		virtual SGLError WriteFrame
			( const SGLImageInfo& imginf, const uint8_t * pbytBuf ) ;
	} ;

}

#endif

