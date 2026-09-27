
#if	!defined(__SAKURAGL_MEDIA_THREADING_AUDIO_DECODER_H__)
#define	__SAKURAGL_MEDIA_THREADING_AUDIO_DECODER_H__	1

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// スレッディング・オーディオ・デコーダー
	//////////////////////////////////////////////////////////////////////////

	class	SGLThreadingAudioDecoder
				: public SGLAudioDecoderInterface
	{
	protected:
		SGLAudioDecoderInterface *	m_pDecoder ;
		bool						m_flagOwner ;
		bool						m_flagThreading ;
		SSystem::SSignalEvent		m_signalDone ;
		size_t						m_nDeocdedSize ;
		SSystem::SArray<uint8_t>	m_bufDecoded ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO
			( SGLThreadingAudioDecoder, SGLAudioDecoderInterface )
		// 構築関数
		SGLThreadingAudioDecoder
			( SGLAudioDecoderInterface * pDecoder, bool flagOwner = false ) ;
		// 消滅関数
		virtual ~SGLThreadingAudioDecoder( void ) ;
		// ペンディング状態同期
		void SyncPending( void ) ;
		// イベント発行
		void IssueThreadEvent( void ) ;

	public:	// SGLAudioDecoderInterface
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

	protected:
		// スレッド関数
		static void DecodeNextThreadProc( void * pInstance ) ;

	} ;

}

#endif

