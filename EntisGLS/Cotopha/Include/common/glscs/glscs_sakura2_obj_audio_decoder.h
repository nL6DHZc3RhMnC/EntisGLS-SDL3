
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/

#if	!defined(__GLSCS_SAKURA2_OBJECT_AUDIO_DECODER_H__)
#define	__GLSCS_SAKURA2_OBJECT_AUDIO_DECODER_H__

namespace	ECSSakura2
{
	//////////////////////////////////////////////////////////////////////////
	// AudioDecoder オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	AudioDecoderObject	: public ECSVolatileObject
	{
	public:
		struct	OPTIONAL_INFO
		{
			uint64_t	nFlags ;			// complex enum OptionalFlag
			uint64_t	nLoopStart ;		// ループポイント
			uint64_t	nLoopEnd ;
			uint64_t	pszTitle ;			// 曲名
			uint64_t	pszVocalPlayer ;	// ボーカル・演奏者
			uint64_t	pszComposer ;		// 作曲者
			uint64_t	pszArranger ;		// 編曲者
		} ;

	protected:
		const wchar_t *							m_pwszType ;
		SakuraGL::SGLAudioDecoderInterface *	m_decoder ;
		bool									m_flagOwner ;
		SSystem::SArray<uint8_t>				m_bufOptionalInf ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( AudioDecoderObject, ECSVolatileObject )
		// 構築関数
		AudioDecoderObject
			( const wchar_t * pwszType,
				SakuraGL::SGLAudioDecoderInterface * decoder, bool flagOwner ) ;
		// 消滅関数
		virtual ~AudioDecoderObject( void ) ;
		// デコーダー関連付け
		void AttachAudioDecoder
			( SakuraGL::SGLAudioDecoderInterface * decoder, bool flagOwner ) ;

	public:
		// メモリマッピング
		virtual LinearAddressCache *
				GetSegmentBuffer( LinearAddressCache & seg ) ;
		// 実行時型名
		virtual const wchar_t * GetTypeName( void ) const ;

	public:
		// オプショナルフォーマット構築
		void MakeOptionalInfo
			( OPTIONAL_INFO& optDst,
				const SakuraGL::SGLAudioDecoderInterface::OptionalInfo& optSrc ) ;
		uint64_t AddOptionalInfoString( const uint16_t * pszInfo ) ;
	} ;

}


//////////////////////////////////////////////////////////////////////////////
// AudioDecoder スタブ
//////////////////////////////////////////////////////////////////////////////

// new SakuraGL::AudioDecoder
ECS_LIB_DECLARE_EXPORT_NEW_OBJECT(SakuraGL_AudioDecoder) ;

// SGLError AudioDecoder::Open( const wchar_t * pwszFilePath ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_AudioDecoder_Open) ;

// SGLError AudioDecoder::Create( SSystem::File * pFile ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_AudioDecoder_Create) ;

// SGLError AudioDecoder::Close( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_AudioDecoder_Close) ;

// SGLError AudioDecoder::GetFormat( SGLSoundFormat & fmt ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_AudioDecoder_GetFormat) ;

// SGLError AudioDecoder::GetOptinalInfo( OptionalInfo & optinf ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_AudioDecoder_GetOptinalInfo) ;

// uint64_t AudioDecoder::GetTotalLength( void ) const ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_AudioDecoder_GetTotalLength) ;

// SGLError AudioDecoder::SeekPosition( uint64_t nPos ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_AudioDecoder_SeekPosition) ;

// size_t AudioDecoder::DecodeNext( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_AudioDecoder_DecodeNext) ;

// size_t AudioDecoder::ReadDecodedBuffer
//		( void * ptrPCM, size_t nBytes, size_t nOffset = 0 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_AudioDecoder_ReadDecodedBuffer) ;

#endif
