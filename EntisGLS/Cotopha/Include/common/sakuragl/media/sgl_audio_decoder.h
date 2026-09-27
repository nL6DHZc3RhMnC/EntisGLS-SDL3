
#if	!defined(__SAKURAGL_MEDIA_AUDIO_DECODER_H__)
#define	__SAKURAGL_MEDIA_AUDIO_DECODER_H__	1

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// オーディオファイル・デコード・インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SGLAudioDecoderInterface	: public SSystem::SObject
	{
	public:
		enum	OptionalFlag
		{
			flagLoopStart	= 0x0001,
			flagLoopEnd		= 0x0002,
			flagTitle		= 0x0004,
			flagVocalPlayer	= 0x0008,
			flagComposer	= 0x0010,
			flagArranger	= 0x0020,
		} ;
		struct	OptionalInfo
		{
			uint64_t	nFlags ;			// complex enum OptionalFlag
			uint64_t	nLoopStart ;		// ループポイント
			uint64_t	nLoopEnd ;
			uint16_t*	pszTitle ;			// 曲名
			uint16_t*	pszVocalPlayer ;	// ボーカル・演奏者
			uint16_t*	pszComposer ;		// 作曲者
			uint16_t*	pszArranger ;		// 編曲者

			OptionalInfo( void ) : nFlags(0) {}
		} ;
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLAudioDecoderInterface, SObject )
		// 構築関数（デフォルト）
		SGLAudioDecoderInterface( void ) {}
		SGLAudioDecoderInterface( const SGLAudioDecoderInterface& decoder ) {}
		// ファイル拡張子判定
		virtual bool IsMatchableFileExtension( const wchar_t * pszExt ) ;
		// デコーダー生成
		virtual SGLAudioDecoderInterface * NewDecoder( void ) const = 0 ;
		// デコーダーを開く
		virtual SGLError Open
			( const wchar_t * pwszFilePath,
					SSystem::SEnvironmentInterface * pEnv = NULL ) = 0 ;
		virtual SGLError Create
			( SSystem::SFileInterface * file, bool flagOwner = true ) = 0 ;
		// デコーダーを閉じる
		virtual SGLError Close( void ) = 0 ;
		// サウンドフォーマットを取得する
		virtual SGLError GetFormat( SGLSoundFormat & fmt ) = 0 ;
		// オプショナル情報を取得する
		virtual SGLError GetOptinalInfo( OptionalInfo & optinf ) = 0 ;
		// 全長 [/samples] を取得する
		virtual uint64_t GetTotalLength( void ) const = 0 ;
		// デコード開始位置 [/samples] を移動する
		virtual SGLError SeekPosition( uint64_t nPos ) = 0 ;
		// 次のデータをデコード
		virtual size_t DecodeNext( void ) = 0 ;
		// デコードデータを取得
		virtual size_t ReadDecodedBuffer
				( void * ptrPCM, size_t nBytes, size_t nOffset = 0 ) = 0 ;
	} ;

	#if	defined(__COTOPHA__)
	class	native AudioDecoder	: public SSystem::VolatileObject
	{
	public:
		// デコーダー生成
		native SGLError Open( const wchar_t * pwszFilePath ) ;
		native SGLError Create( SSystem::File * pFile ) ;
		// デコーダー解放
		native SGLError Close( void ) ;
		// サウンドフォーマットを取得する
		native SGLError GetFormat( SGLSoundFormat & fmt ) ;
		// オプショナル情報を取得する
		native SGLError GetOptinalInfo
				( SGLAudioDecoderInterface::OptionalInfo & optinf ) ;
		// 全長 [/samples] を取得する
		native uint64_t GetTotalLength( void ) const ;
		// デコード開始位置 [/samples] を移動する
		native SGLError SeekPosition( uint64_t nPos ) ;
		// 次のデータをデコード
		native size_t DecodeNext( void ) ;
		// デコードデータを取得
		native size_t ReadDecodedBuffer
				( void * ptrPCM, size_t nBytes, size_t nOffset = 0 ) ;
	} ;
	#else
	typedef	SGLAudioDecoderInterface	AudioDecoder ;
	#endif


	//////////////////////////////////////////////////////////////////////////
	// オーディオ・デコーダー管理
	//////////////////////////////////////////////////////////////////////////

	class	SGLAudioDecoderManager
	{
	public:
		// オーディオデコーダー配列
		static ESL_DLL_EXPORT SSystem::SObjectArray<SGLAudioDecoderInterface> *	m_arrayAudioDecoder ;

	public:
		// 初期化
		static void Initialzie( void ) ;
		// 終了
		static void Finalize( void ) ;
		// デコーダー追加登録
		static void RegisterDecoder( SGLAudioDecoderInterface * pDecoder ) ;
		// 拡張子が合致するデコーダー生成
		static SGLAudioDecoderInterface * FindDecoder( const wchar_t * pszExt ) ;
		// ファイルに適合するデコーダーを生成
		static SGLAudioDecoderInterface *
				OpenDecoder( const wchar_t * pwszFilePath,
								SSystem::SEnvironmentInterface * pEnv ) ;
		static SGLAudioDecoderInterface *
			CreateDecoder( SSystem::SFileInterface * file, bool flagOwner ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// オーディオ・デコーダー・ラッパー
	//////////////////////////////////////////////////////////////////////////

	class	SGLAudioDecoder : public SGLAudioDecoderInterface
	{
	protected:
		AudioDecoder *	m_pDecoder ;
		bool			m_flagOwner ;
		#if	defined(__COTOPHA__)
		SSystem::SFileInterface *	m_pFile ;
		bool						m_flagFileOwner ;
		#endif

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLAudioDecoder, SGLAudioDecoderInterface )
		// 構築関数
		SGLAudioDecoder( AudioDecoder * pDecoder, bool flagOwner = false ) ;
		// 消滅関数
		virtual ~SGLAudioDecoder( void ) ;
		// オブジェクト関連付け
		void AttachAudioDecoder
				( AudioDecoder * pDecoder, bool flagOwner = false ) ;

	public:
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
