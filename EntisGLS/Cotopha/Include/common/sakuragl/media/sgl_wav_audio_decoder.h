
#if	!defined(__SAKURAGL_MEDIA_WAV_AUDIO_DECODER_H__)
#define	__SAKURAGL_MEDIA_WAV_AUDIO_DECODER_H__	1

#include <sakura/ssys_queue_buffer.h>

#if	defined(__PLATFORM_WINDOWS__)
#include <vfw.h>
#endif

namespace	SakuraGL
{
#if	defined(__PLATFORM_WINDOWS__)

	//////////////////////////////////////////////////////////////////////////
	// ACM デコードストリーム
	//////////////////////////////////////////////////////////////////////////

	class	SGLWaveACMDecodeStream	: public	ESLObject
	{
	protected:
		HACMSTREAM					m_hACMStream ;
		SSystem::SQueueBuffer		m_bufACMStream ;
		SSystem::SQueueBuffer		m_bufPCMStream ;
		WAVEFORMATEX				m_wfxAudioPCM ;
		bool						m_flagStartBlock ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLWaveACMDecodeStream, ESLObject )
		// 構築関数
		SGLWaveACMDecodeStream( void ) ;
		// 消滅関数
		virtual ~SGLWaveACMDecodeStream( void ) ;

	public:
		// デコード準備
		SGLError PrepareStream( const WAVEFORMATEX * pwfxSrc ) ;
		// デーコード処理終了
		void CloseStream( void ) ;
		// ストリームバッファの初期化
		void FlushStream( void ) ;
		// デコード出力フォーマット
		const WAVEFORMATEX& GetOutputFormat( void ) const ;
		// ソースデータ追加
		size_t WriteCompressedAudio( const void * ptrData, size_t nBytes ) ;
		// 待ち行列のデコード済みデータサイズ取得
		size_t GetDecompressedBytes( void ) const ;
		// デコードデータ取得
		size_t ReadWaveData( void * ptrData, size_t nBytes ) ;
		// デコードサイズ見積もり
		size_t EstimateDecodedSize( size_t nSrcBytes ) const ;
		// 入力サイズ見積もり
		size_t EstimateSourceSize( size_t nDstBytes ) const ;

	} ;

#endif


	//////////////////////////////////////////////////////////////////////////
	// Windows Wave Form オーディオ・デコーダー
	//////////////////////////////////////////////////////////////////////////

	class	SGLWaveFormAudioDecoder	: public SGLAudioDecoderInterface
	{
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
		SSystem::SFileInterface *	m_pFile ;
		bool						m_flagFileOwner ;
		WAVEFORMAT *				m_pwfx ;
		bool						m_flagCompressed ;
		int64_t						m_posWaveBase ;
		uint32_t					m_sizeWave ;
		uint32_t					m_posWave ;
		SSystem::SArray<uint8_t>	m_bufWave ;

		#if	defined(__PLATFORM_WINDOWS__)
		SGLWaveACMDecodeStream		m_acmDecoder ;
		#endif

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLWaveFormAudioDecoder, SGLAudioDecoderInterface )
		// 構築関数
		SGLWaveFormAudioDecoder( void ) ;
		// 消滅関数
		virtual ~SGLWaveFormAudioDecoder( void ) ;
		// ファイル拡張子判定
		virtual bool IsMatchableFileExtension( const wchar_t * pszExt ) ;
		// デコーダー生成
		virtual SGLAudioDecoderInterface * NewDecoder( void ) const ;
		// デコーダーを開く
		virtual SGLError Open
			( const wchar_t * pwszFilePath,
					SSystem::SEnvironmentInterface * pEnv = NULL ) ;
		virtual SGLError Create
			( SSystem::SFileInterface * file, bool flagOwner = true ) ;
		// デコーダーを閉じる
		virtual SGLError Close( void ) ;
		// サウンドフォーマットを取得する
		virtual SGLError GetFormat( SGLSoundFormat & fmt ) ;
		// オプショナル情報を取得する
		virtual SGLError GetOptinalInfo( OptionalInfo & optinf ) ;
		// 全長 [/samples] を取得する
		virtual uint64_t GetTotalLength( void ) const ;
		// デコード開始位置 [/samples] を移動する
		virtual SGLError SeekPosition( uint64_t nPos ) ;
		// 次のデータをデコード
		virtual size_t DecodeNext( void ) ;
		// デコードデータを取得
		virtual size_t ReadDecodedBuffer
				( void * ptrPCM, size_t nBytes, size_t nOffset = 0 ) ;
	} ;

}

#endif
