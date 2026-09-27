
#if	!defined(__SAKURAGL_MEDIA_SOUND_SOFTWARE_MIXER_H__)
#define	__SAKURAGL_MEDIA_SOUND_SOFTWARE_MIXER_H__	1

#include <sakura/ssys_queue_buffer.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// ミキサ入力ラインインターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SGLSoundMixerInputInterface
	{
	public:
		// フォーマット取得
		virtual const SGLSoundFormat& GetFormat( void ) const = 0 ;
		// ストリーミング取得
		virtual size_t ReadStream( void * ptrSound, size_t nBytes ) = 0 ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// PCM フォーマット変換
	//////////////////////////////////////////////////////////////////////////

	SGLError sglDecodeSoundTo16bitsPCM
		( int16_t * pwDst, size_t nDstStride,  size_t nDstChStride,
			const SGLSoundFormat& fmtSrc,
			const uint8_t * pSrcPCM, size_t nSamples, size_t nChannels ) ;
	SGLError sglDecodeSoundTo32bitsPCM
		( float32_t * pfpDst, size_t nDstStride,  size_t nDstChStride,
			const SGLSoundFormat& fmtSrc,
			const uint8_t * pSrcPCM, size_t nSamples, size_t nChannels ) ;
	SGLError sglEncodeSoundFrom16bitsPCM
		( const SGLSoundFormat& fmtDst, uint8_t * pDstPCM,
			const int16_t * pwSrc,
			size_t nSrcStride, size_t nSrcChStride,
			size_t nSamples, size_t nChannels ) ;
	SGLError sglEncodeSoundFrom32bitsPCM
		( const SGLSoundFormat& fmtDst, uint8_t * pDstPCM,
			const float32_t * pfpSrc,
			size_t nSrcStride, size_t nSrcChStride,
			size_t nSamples, size_t nChannels ) ;
	SGLError sglEncodeSoundFrom32bitsPCM
		( const SGLSoundFormat& fmtDst, uint8_t * pDstPCM,
			const int32_t * pnSrc,
			size_t nSrcStride, size_t nSrcChStride,
			size_t nSamples, size_t nChannels ) ;


	//////////////////////////////////////////////////////////////////////////
	// ソフトウェアミキサ
	//////////////////////////////////////////////////////////////////////////

	class	SGLSoundSoftwareMixer	: public SSystem::SObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLSoundSoftwareMixer, SObject )
		// 構築関数
		SGLSoundSoftwareMixer( void ) ;
		// 消滅関数
		virtual ~SGLSoundSoftwareMixer( void ) ;

	public:
		class	MixBuffer
		{
		public:
			SSystem::SArray<int16_t>	m_bufSrc1 ;		// 入力16bit化
			SSystem::SArray<int16_t>	m_bufSrc2 ;		// 入力チャネル数正規化
			SSystem::SArray<int16_t>	m_bufSrc3 ;		// 入力周波数変換
			SSystem::SArray<int16_t>	m_bufDst ;		// 出力16bit化
		public:
			void MixWave
				( const SGLSoundFormat& fmtOut,
					void * ptrOutBuf, size_t nOutSamples,
					const SGLSoundFormat& fmtIn,
					const void * ptrInBuf, size_t nInSamples ) ;
			void NormalizeTo16bits
				( int16_t * pDstBuf,
					const SGLSoundFormat& fmtIn,
					const void * ptrInBuf, size_t nInSamples ) ;
			void NormalizeFrom16bits
				( void * pDstBuf,
					const SGLSoundFormat& fmtOut,
					const int16_t * ptrInBuf, size_t nInSamples ) ;
			void NormalizeChannels
				( int16_t * pDstBuf, size_t nDstChannel,
					const int16_t * pInBuf, size_t nInSamples, size_t nSrcChannel ) ;
			void NormalizeFrequency
				( int16_t * pDstBuf, size_t nOutSamples,
					const int16_t * pInBuf, size_t nInSamples, size_t nSrcChannel ) ;
		} ;

	protected:
		SGLSoundFormat				m_fmtOut ;
		SSystem::SQueueBuffer		m_qbufOut ;
		SSystem::SPointerArray<SGLSoundMixerInputInterface>
									m_aLines ;
		SSystem::SCriticalSection	m_csSync ;

	public:
		// 入力ライン追加
		void AddInputLine( SGLSoundMixerInputInterface * pLine ) ;
		// 入力ライン削除
		void DetachInputLine( SGLSoundMixerInputInterface * pLine ) ;

	public:
		// 出力フォーマット設定
		void SetOutputFormat( const SGLSoundFormat& fmt ) ;
		// 出力フォーマット取得
		const SGLSoundFormat& GetOutputFormat( void ) const ;
		// ミキシング実行
		void MixSound( uint32_t msecTime ) ;
		// 出力バッファに蓄積されたデータ量取得
		size_t GetOutputDataBytes( void ) const ;
		// 出力バッファからデータ読み出し
		size_t ReadOutputData( void * ptrSound, size_t nBytes ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// サウンドフィルター・インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SGLSoundFilterInterface	: public ESLObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLSoundFilterInterface, ESLObject )
		// フォーマット設定
		virtual SGLError Open( const SGLSoundFormat& fmt ) = 0 ;
		// ストリームの完了
		virtual void FlushStream( void ) = 0 ;
		// フィルタ処理
		virtual size_t FilterStream( const void * ptrSound, size_t nBytes ) = 0 ;
		// フィルタ処理したデータのサイズを取得
		virtual size_t GetBufferedBytes( void ) = 0 ;
		// フィルタ処理したデータを取得
		virtual size_t ReadStream( void * ptrSound, size_t nBytes ) = 0 ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// ミキサ入力ライン
	//////////////////////////////////////////////////////////////////////////

	class	SGLSoundMixerLinePlayer
				: public SGLSoundPlayerInterface,
					public SGLSoundMixerInputInterface
	{
	protected:
		SSystem::SSmartReference<SGLSoundSoftwareMixer>
									m_refMixer ;
		SGLSoundFormat				m_format ;
		SSystem::SCriticalSection	m_csSync ;
		SSystem::SQueueBuffer		m_qbufWave ;
		uint64_t					m_nPosition ;
		bool						m_flagOpened ;
		bool						m_flagPlaying ;
		bool						m_flagPaused ;
		bool						m_flagStatic ;
		bool						m_flagLoop ;
		SSystem::SArray<uint8_t>	m_bufStaticWave ;
		size_t						m_iStaticPos ;
		float32_t					m_fpVolume[2] ;
		SSystem::SPointerArray<SGLSoundFilterInterface>
									m_aFilters ;
		SSystem::SQueueBuffer		m_qbufFilter ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLSoundMixerLinePlayer, SGLSoundPlayerInterface )
		// 構築関数
		SGLSoundMixerLinePlayer( SGLSoundSoftwareMixer * pMixer ) ;
		// 消滅関数
		virtual ~SGLSoundMixerLinePlayer( void ) ;

	public:	// SGLSoundPlayerInterface
		// フォーマットを指定して出力を準備する
		virtual SGLError Open( const SGLSoundFormat& fmt ) ;
		// 出力用に準備したサウンド出力を解放する
		virtual SGLError Close( void ) ;
		// スタティックバッファを準備して書き込む
		virtual SGLError WriteStatic( const void * ptrSound, size_t nBytes ) ;
		// ストリームバッファを準備する
		virtual SGLError PrepareStream( size_t nBytes = 0 ) ;
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

	public:
		// フォーマット取得
		virtual const SGLSoundFormat& GetFormat( void ) const ;
		// ストリーミング取得
		virtual size_t ReadStream( void * ptrSound, size_t nBytes ) ;

	public:
		// フィルターを追加
		size_t AddFilter( SGLSoundFilterInterface * pFilter ) ;
		size_t InserFilter( size_t i, SGLSoundFilterInterface * pFilter ) ;
		// フィルターを削除
		void DetachFilter( SGLSoundFilterInterface * pFilter ) ;
		void DetachFilterAt( size_t i ) ;
		void DetachAllFilters( void ) ;

	protected:
		// フィルター処理
		void FilterSoundStream( SSystem::SArray<uint8_t>& bufStream ) ;
		// 音量反映処理
		void EffectVolume( void * ptrSound, size_t nBytes ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 周波数フィルタ
	//////////////////////////////////////////////////////////////////////////

	class	SGLSoundEqualizerProcessor	: public ESLObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLSoundEqualizerProcessor, ESLObject )
		// イコライザ処理
		virtual void OnEqualizer
			( size_t iChannel, float32_t * pfpDCT, size_t nDCTSize ) = 0 ;
	} ;

	class	SGLSoundEqualizerFilter	: public SGLSoundFilterInterface
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO
			( SGLSoundEqualizerFilter, SGLSoundFilterInterface )
		// 構築関数
		SGLSoundEqualizerFilter( void ) ;
		// 消滅関数
		virtual ~SGLSoundEqualizerFilter( void ) ;

	protected:
		SGLSoundFormat	m_fmtSound ;
		size_t			m_nSampleAlign ;
		size_t			m_nDCTDegree ;		// 8, 9, 10, 11, 12
		size_t			m_nMatrixDiv ;		// 2, 4, 8, ...
		size_t			m_nMatrixSize ;		// 256, 512, 1024, ...
		size_t			m_nBlockSize ;		// m_nMatrixSize / m_nMatrixDiv
		SSystem::SArray<float32_t>
						m_bufKDB ;			// カイザー・ベッセル派生 (KBD) 窓
		SSystem::SArray<float32_t>
						m_bufAccIDCT ;
		SSystem::SArray<float32_t>
						m_bufStreamBlock ;
		SSystem::SArray<float32_t>
						m_bufWorkDCT ;
		SSystem::SArray<float32_t>
						m_bufWorkIDCT ;
		SSystem::SArray<float32_t>
						m_bufWorkTemp1 ;
		SSystem::SArray<float32_t>
						m_bufWorkTemp2 ;
		SSystem::SQueueBuffer
						m_qbufEqIn ;		// 入力バッファ
		SSystem::SQueueBuffer
						m_qbufEqOut ;		// 出力バッファ
		size_t			m_nPreBufferCount ;

		SSystem::SPointerArray
			<SGLSoundEqualizerProcessor>
						m_aEqualizers ;

	public:
		// ブロックサイズと窓を設定
		void Initialize( int nDCTDegree, int nDivCount ) ;

	public:
		// イコライザを追加
		size_t AddEqualizer( SGLSoundEqualizerProcessor * pEq ) ;
		size_t InserEqualizer( size_t i, SGLSoundEqualizerProcessor * pEq ) ;
		// イコライザを削除
		void DetachEqualizer( SGLSoundEqualizerProcessor * pEq ) ;
		void DetachEqualizerAt( size_t i ) ;
		void DetachAllEqualizers( void ) ;

	public:	// SGLSoundFilterInterface
		// フォーマット設定
		virtual SGLError Open( const SGLSoundFormat& fmt ) ;
		// ストリームの完了
		virtual void FlushStream( void ) ;
		// フィルタ処理
		virtual size_t FilterStream( const void * ptrSound, size_t nBytes ) ;
		// フィルタ処理したデータのサイズを取得
		virtual size_t GetBufferedBytes( void ) ;
		// フィルタ処理したデータを取得
		virtual size_t ReadStream( void * ptrSound, size_t nBytes ) ;

	protected:
		// DCT バッファの末尾に入力データを追加
		void AddStreamToDCTBuffer( size_t iChannel, const uint8_t * pInBuf ) ;
		// フィルタ処理
		void ProcessEqualizer( size_t iChannel ) ;
		// 処理後のDCT級数を逆変換して出力バッファに加算
		void OutputEqualizer( size_t iChannel ) ;
		// 1ブロックを出力バッファにストリーム出力
		void BlockStreamOutput( void ) ;

	} ;

}

#endif

