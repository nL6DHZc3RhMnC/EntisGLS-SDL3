
#if	!defined(__SAKURAGL_MEDIA_MIO_AUDIO_DECODER_H__)
#define	__SAKURAGL_MEDIA_MIO_AUDIO_DECODER_H__	1

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// MIO オーディオ・デコーダー
	//////////////////////////////////////////////////////////////////////////

	class	SGLMIOAudioDecoder	: public SGLAudioDecoderInterface
	{
	protected:
		ERISA::SGLSoundFilePlayer	m_miofile ;
		SSystem::SArray<uint8_t>	m_bufWave ;
		bool						m_flagNextSeek ;
		uint32_t					m_nOffsetBytes ;
		uint64_t					m_posNextSeek ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLMIOAudioDecoder, SGLAudioDecoderInterface )
		// 構築関数
		SGLMIOAudioDecoder( void ) ;
		// 消滅関数
		virtual ~SGLMIOAudioDecoder( void ) ;
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
