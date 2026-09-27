
#if	!defined(__SAKURAGL_MEDIA_MEDIA_COMPOSER_H__)
#define	__SAKURAGL_MEDIA_MEDIA_COMPOSER_H__	1

#include <sakuragl/media/sgl_audio_decoder.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// メディア補助情報
	//////////////////////////////////////////////////////////////////////////

	class	SGLMediaOptionalInfo	: public ESLObject
	{
	public:
		enum	MemberIndexMask
		{
			flagLoopStart	= 0x0001,
			flagLoopEnd		= 0x0002,
			flagTitle		= 0x0004,
			flagVocalPlayer	= 0x0008,
			flagComposer	= 0x0010,
			flagArranger	= 0x0020,
		} ;
		uint64_t			m_nFlags ;
		uint64_t			m_nLoopStart ;		// ループポイント
		uint64_t			m_nLoopEnd ;
		SSystem::SString	m_strTitle ;		// タイトル
		SSystem::SString	m_strPlayer ;		// ボーカル・演奏者
		SSystem::SString	m_strComposer ;		// 作曲者
		SSystem::SString	m_strArranger ;		// 編曲者

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLMediaOptionalInfo, ESLObject )
		// 構築関数
		SGLMediaOptionalInfo( void ) ;
		SGLMediaOptionalInfo( const SGLMediaOptionalInfo& optinf ) ;
		// 代入
		const SGLMediaOptionalInfo& operator = ( const SGLMediaOptionalInfo& optinf ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// オーディオ入力ストリーム
	//////////////////////////////////////////////////////////////////////////

	class	SGLAudioInputStream	: public SSystem::SObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLAudioInputStream, SObject ) ;
		// サウンド形式取得
		virtual SGLError GetAudioFormat( SGLSoundFormat& fmt ) = 0 ;
		// メディア補助情報取得
		virtual SGLError GetAudioOptinalInfo( SGLMediaOptionalInfo& optinf ) = 0 ;
		// オーディオストリーム全長取得（未定は-1）[samples]
		virtual int64_t GetAudioLength( void ) const = 0 ;
		// オーディオストリーム読み込み [samples]
		virtual size_t ReadAudio( void * ptrBuf, size_t nSamples ) = 0 ;
		// オーディオストリーム位置変更
		virtual SGLError SeekAudio( uint64_t nSamples ) = 0 ;

	} ;

	class	SGLAudioDecoderStream	: public SGLAudioInputStream
	{
	protected:
		SGLAudioDecoderInterface *	m_pDecoder ;
		bool						m_flagOwnDecoder ;
		SGLSoundFormat				m_fmtAudio ;
		size_t						m_nDecodedBytes ;
		size_t						m_nReadNext ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLAudioDecoderStream, SGLAudioInputStream )
		// 構築関数
		SGLAudioDecoderStream( void ) ;
		SGLAudioDecoderStream
			( SGLAudioDecoderInterface * pDecoder, bool flagOwnDecoder = false ) ;
		// 消滅関数
		virtual ~SGLAudioDecoderStream( void ) ;
		// オーディオファイルを開く
		SGLError OpenAudioFile( const wchar_t * pwszFilePath ) ;
		SGLError OpenAudioFile( SSystem::SFileInterface * file, bool flagOwner ) ;
		// オープン済みデコーダーを関連付ける
		void AttachAudioDecoder
			( SGLAudioDecoderInterface * pDecoder, bool flagOwnDecoder ) ;
		// デコーダーを解放する
		void Release( void ) ;

	public:
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
	// オーディオ出力ストリーム
	//////////////////////////////////////////////////////////////////////////

	class	SGLAudioOutputStream	: public SSystem::SObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLAudioOutputStream, SObject ) ;
		// オーディオ出力ストリーム準備
		virtual SGLError PrepareAudio
			( const SGLSoundFormat& fmt, int64_t nSamples = -1,
					const SGLMediaOptionalInfo * pOptInf = NULL ) = 0 ;
		// オーディオストリーム出力 [samples]
		virtual size_t WriteAudio( const void * ptrBuf, size_t nSamples ) = 0 ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// ビデオ入力ストリーム
	//////////////////////////////////////////////////////////////////////////

	class	SGLVideoInputStream	: public SSystem::SObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLVideoInputStream, SObject ) ;
		// ビデオ画像形式取得
		virtual SGLError GetImageFormat( SGLImageInfo& imginf ) = 0 ;
		// メディア補助情報取得
		virtual SGLError GetVideoOptinalInfo( SGLMediaOptionalInfo& optinf ) = 0 ;
		// ビデオストリーム全長取得（未定は-1）[frames]
		virtual int64_t GetVideoLength( void ) const = 0 ;
		// ビデオストリーム全長取得（未定は-1）[millisecond]
		virtual int64_t GetVideoDuration( void ) const = 0 ;
		// ビデオストリーム読み込み
		virtual SGLError ReadFrame
			( const SGLImageInfo& imginf, uint8_t * pbytBuf ) = 0 ;
		// ビデオストリーム位置変更
		virtual SGLError SeekFrame( uint64_t nFrames ) = 0 ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// ビデオ出力ストリーム
	//////////////////////////////////////////////////////////////////////////

	class	SGLVideoOutputStream	: public SSystem::SObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLVideoOutputStream, SObject ) ;
		// ビデオ出力ストリーム準備
		virtual SGLError PrepareVideo
			( const SGLImageInfo& imginf,
				int64_t nFrames, int64_t nDuration,
				const SGLMediaOptionalInfo * pOptInf = NULL ) = 0 ;
		// ビデオストリーム出力
		virtual SGLError WriteFrame
			( const SGLImageInfo& imginf, const uint8_t * pbytBuf ) = 0 ;
	} ;

}

#endif

