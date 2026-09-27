
#if	!defined(__SAKURAGL_WIN_AVI_COMPOSER_H__)
#define	__SAKURAGL_WIN_AVI_COMPOSER_H__	1

#include <sakuraglx/sakuraglx.h>
#include <sakura/ssys_queue_buffer.h>
#include <sakuragl/media/sgl_sound_player.h>
#include <sakuragl/media/sgl_wav_audio_decoder.h>
#include <sakuragl/media/sgl_media_composer.h>
#include <sakuraglx/extra/sglx_media_streamer.h>
#include <vfw.h>
#include <aviriff.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// Windows Wave Form Audio ファイル入力
	//////////////////////////////////////////////////////////////////////////

	class	SGLWindowWaveFileReader	: public SGLAudioInputStream
	{
	protected:
		SGLWaveFormAudioDecoder	m_decoder ;
		SSystem::SQueueBuffer	m_bufNext ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLWindowWaveFileReader, SGLAudioInputStream )
		// 構築関数
		SGLWindowWaveFileReader( void ) ;
		// 消滅関数
		virtual ~SGLWindowWaveFileReader( void ) ;

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
	} ;


	//////////////////////////////////////////////////////////////////////////
	// Windows AVI ファイル入力ラッパー
	//////////////////////////////////////////////////////////////////////////

	class	SGLWindowsAVIReader
				: public SGLAudioInputStream, public SGLVideoInputStream
	{
	public:
		class	AVIStream
		{
		public:
			AVISTREAMHEADER			m_avish ;
			SSystem::SByteBuffer	m_bufHeader ;
			uint64_t				m_nNextPos ;
		} ;

	protected:
		SSystem::SFileInterface *	m_pfile ;		// 入力ファイル
		bool						m_flagOwnFile ;
		AVIMAINHEADER				m_avimh ;		// メインヘッダ
		SSystem::SArray<uint64_t>	m_arrPosRIFF ;	// RIFF の先頭位置（AVI2.0 用）

		struct	VideoFramePosInfo
		{
			uint64_t	nFilePos ;		// ビデオフレームのファイル位置
			uint64_t	nSoundPCMPos ;	// オーディオサンプル数

			VideoFramePosInfo( uint64_t nPos, uint64_t nPCM )
				: nFilePos( nPos ), nSoundPCMPos( nPCM ) { }
		} ;

		SSystem::SArray<AVIINDEXENTRY>
									m_arrIndexEntries ;	// AVI 1.0 のインデックス
		SSystem::SArray<size_t>		m_arrVideoIndex ;	// 映像フレームのインデックス
		SSystem::SArray<VideoFramePosInfo>
									m_arrPosFrame ;
		size_t						m_nNextFrame ;
		uint64_t					m_nNextAudioPCMs ;

		struct	RIFF_CHUNK
		{
			uint32_t	nChunkID ;
			uint32_t	nBytes ;
			uint64_t	nBeginPos ;
		} ;
		SSystem::SArray<RIFF_CHUNK>	m_arrChunk ;

		AVIStream *					m_psVideo ;		// Video ストリーム
		BITMAPINFOHEADER *			m_pbmihVideo ;
		BITMAPINFOHEADER			m_bmiOut ;
		SSystem::SArray<uint8_t>	m_bufVideoBuf ;
		SSystem::SArray<uint8_t>	m_bufTempBuf ;
		SGLImage					m_imgTempBuf ;

		AVIStream *					m_psAudio ;		// Audio ストリーム
		WAVEFORMATEX *				m_pwfxAudio ;
		WAVEFORMATEX				m_wfxAudioPCM ;
		SGLSoundFormat				m_fmtAudio ;
		SGLSoundFormat				m_fmtAudioPCM ;
		bool						m_flagNeedConversionPCM ;
		bool						m_flagAudioSeeked ;

		HIC							m_hicDecompress ;
		HACMSTREAM					m_hACMStream ;
		SSystem::SQueueBuffer		m_bufACMStream ;
		SSystem::SQueueBuffer		m_bufPCMStream ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
			( SGLWindowsAVIReader, SGLAudioInputStream, SGLVideoInputStream )
		// 構築関数
		SGLWindowsAVIReader( void ) ;
		// 消滅関数
		virtual ~SGLWindowsAVIReader( void ) ;

	public:
		// ファイルを開く
		SGLError Open( const wchar_t * pwszFilePath ) ;
		SGLError Open
			( SSystem::SFileInterface * pfile, bool flagOwnFile = false ) ;
		// ファイルを閉じる
		SGLError Close( void ) ;

	protected:
		// ファイルから読み込み
		size_t Read( void * ptrBuf, size_t nBytes ) ;
		// チャンク長取得
		uint32_t GetLength( void ) const ;
		// チャンクを開く
		SGLError DescendChunk( const char * pszChunkID = NULL ) ;
		// チャンクを抜ける
		SGLError AscendChunk( void ) ;
		// 現在のチャンク名を取得する
		uint32_t GetCurrentChunkID( void ) const ;
		// 現在のチャンクの一致判定
		bool IsEqualCurrentChunkID( const char * pszChunk ) const
		{
			return	IsEqualChunkID( GetCurrentChunkID(), pszChunk ) ;
		}
		// チャンクの一致判定
		static bool IsEqualChunkID( uint32_t nID, const char * pszID ) ;
		// 映像フレームチャンクか？
		static bool IsVideoFrameChunkID( uint32_t nID ) ;
		// 音声データチャンクか？
		static bool IsAudioFrameChunkID( uint32_t nID ) ;

	protected:
		// ストリームの次のデータまでシークして読み込む
		SGLError ReadNextStreamData
			( SSystem::SQueueBuffer & bufData, AVIStream * pavis,
				AVIStream * pavisAnother = NULL, uint64_t * pAnotherBytes = NULL ) ;
		// ストリームの次のデータまでシークする
		SGLError SeekNextStreamData
			( AVIStream * pavis, AVIStream * pavisAnother, uint64_t * pAnotherBytes ) ;
		// LIST movi チャンクを開く
		SGLError DescendRIFF_LIST_movie( void ) ;
		SGLError DescendLIST_movie( void ) ;
		// 映像フレームを展開するために画像バッファを生成
		SGLError CreateDecompressVideoBuffer( void ) ;
		// 映像フレームを展開
		SGLError DecompressVideoFrame
			( const SGLImageInfo& imginf,
				uint8_t * pbytBuf, SSystem::SQueueBuffer & bufData ) ;
		SGLError DecompressVideoFrameVCM( LPVOID lpData ) ;
		// PCM 音声データのフォーマットを取得
		const WAVEFORMATEX * GetDecompressedAudioFormat( void ) const
		{
			if ( m_pwfxAudio == NULL )
			{
				return	NULL ;
			}
			return	&m_wfxAudioPCM ;
		}
		// 音声データを展開して取得します
		SGLError DecompressAudioData
			( SSystem::SQueueBuffer & bufPCM, SSystem::SQueueBuffer & bufData ) ;
		// フレーム位置をシーク
		SGLError SeekFrameFilePosition( uint64_t& nPos, uint64_t nFrame ) ;
		SGLError SeekAudioFrameFilePosition
			( uint64_t& nPos, uint64_t nFrame, uint64_t nSamples ) ;
		// キーフレームを検索
		size_t FindKeyFrame( size_t iFrame ) const ;

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

	} ;


	//////////////////////////////////////////////////////////////////////////
	// Window Wave Form Audio ファイル出力
	//////////////////////////////////////////////////////////////////////////

	class	SGLWindowWaveFileWriter	: public SGLAudioOutputStream
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLWindowWaveFileWriter, SGLAudioOutputStream )
		// 構築関数
		SGLWindowWaveFileWriter( void ) ;
		// 消滅関数
		virtual ~SGLWindowWaveFileWriter( void ) ;

	public:
		struct	WAVE_RIFF_HEADER
		{
			uint8_t		idRIFF[4] ;		// 'RIFF'
			uint32_t	nFileLength ;	// File Length
			uint8_t		idWAVE[4] ;		// 'WAVE'
		} ;
		struct	RIFF_CHUNK_HEADER
		{
			uint8_t		idChunk[4] ;
			uint32_t	nLength ;
		} ;
		struct	WAVEFORMAT
		{
			uint16_t	wFormatTag ;
			uint16_t	wChannels ;
			uint32_t	nFrequency ;
			uint32_t	nBytesPerSec ;
			uint16_t	wBlockAlign ;
			uint16_t	wBitsPerSample ;
		} ;
		enum	WaveFormat
		{
			formatWavePCM	= 1,
		} ;

	protected:
		SSystem::SFileInterface *	m_pfile ;
		bool						m_flagAutoDelFile ;
		bool						m_flagWrittenFormat ;
		bool						m_flagDescendChunk ;
		int64_t						m_posChunkBase ;
		WAVEFORMAT					m_wfmt ;

	public:
		// ファイルを開く
		SGLError Open( const wchar_t * pwszFilePath ) ;
		SGLError Open
			( SSystem::SFileInterface * pfile, bool flagAutoDelete = false ) ;
		// ファイルを閉じる
		SGLError Close( void ) ;

	public:
		// チャンク生成
		SGLError DescendChunk( const char * pChunkID ) ;
		// チャンク終了
		SGLError AscendChunk( void ) ;

	public:	// SGLAudioOutputStream 実装
		// オーディオ出力ストリーム準備
		virtual SGLError PrepareAudio
			( const SGLSoundFormat& fmt, int64_t nSamples = -1,
					const SGLMediaOptionalInfo * pOptInf = NULL ) ;
		// オーディオストリーム出力 [samples]
		virtual size_t WriteAudio( const void * ptrBuf, size_t nSamples ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// Windows AVI ファイル出力ラッパー
	//////////////////////////////////////////////////////////////////////////

	class	SGLWindowsAVIWriter
				: public SGLAudioOutputStream, public SGLVideoOutputStream
	{
	protected:
		PAVIFILE					m_pavif ;
		PAVISTREAM					m_psVideo ;
		PAVISTREAM					m_psEditTemp ;
		PAVISTREAM					m_psCmpTemp ;
		PAVISTREAM					m_psAudio ;
		COMPVARS					m_cmpvars ;
		bool						m_fCmpVars ;

		BITMAPINFOHEADER			m_bmih ;
		WAVEFORMATEX				m_wfx ;
		LONG						m_iNextFrame ;
		LONG						m_iNextPCM ;
		SSystem::SArray<uint8_t>	m_bufPixels ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
			( SGLWindowsAVIWriter, SGLAudioOutputStream, SGLVideoOutputStream )
		// 構築関数
		SGLWindowsAVIWriter( void ) ;
		// 消滅関数
		virtual ~SGLWindowsAVIWriter( void ) ;

	public:
		// 圧縮オプションの選択
		SGLError CompressorChoose
			( SGLAbstractWindow * pWindow,
				const wchar_t * pwszCaption, const SGLImageInfo& imginf ) ;
		// ファイルを開く
		SGLError Open( const wchar_t * pwszFilePath ) ;
		// ファイルを閉じる
		SGLError Close( void ) ;

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
			( const SGLImageInfo& imginf, const uint8_t * pbytImage ) ;

	protected:
		// 圧縮オプション UI 表示パラメータ
		struct	COMPRESSOR_CHOOSE_PARAM
		{
			SGLError				err ;
			SGLWindowsAVIWriter *	pWriter ;
			HWND					hWnd ;
			LPSTR					pszCaption ;
			BITMAPINFOHEADER		bmih ;
		} ;
		static void CallCompressorChoose( void * pInstance ) ;
		// BITMAPINFOHEADER へ変換
		void ToBitmapInfoHeader
			( BITMAPINFOHEADER& bmih, const SGLImageInfo& imginf ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// ウィンドウ AVI キャプチャー
	//////////////////////////////////////////////////////////////////////////

	class	SGLWindowsAVICapture	: public SGLSyncWindowCapture
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLWindowsAVICapture, SGLSyncWindowCapture )
		// 構築関数
		SGLWindowsAVICapture( void ) ;
		// 消滅関数
		virtual ~SGLWindowsAVICapture( void ) ;

	protected:
		SGLWindowsAVIWriter	m_aviWriter ;

	public:
		// キャプチャー開始
		SGLError BeginCapture
			( SGLWindowSprite * pWindow,
				const wchar_t * pwszAviPath,
				bool flagCompressOpt,
				uint32_t nFlags, size_t nFramesPerSec,
				const SGLSoundFormat& fmtSoundOut ) ;
		// キャプチャー終了
		SGLError EndCapture( void ) ;

	} ;

}

#endif
