
#if	!defined(__SAKURAGL_ERISA_SOUND_FILE_PLAYER_H__)
#define	__SAKURAGL_ERISA_SOUND_FILE_PLAYER_H__

namespace	ERISA
{
	//////////////////////////////////////////////////////////////////////////
	// MIOファイルストリーム再生オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	SGLSoundFilePlayer	: public	ESLObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLSoundFilePlayer, ESLObject )
		// 構築関数
		SGLSoundFilePlayer( void ) ;
		// 消滅関数
		virtual ~SGLSoundFilePlayer( void ) ;

	protected:
		// レコード先読みオブジェクト
		class	PreloadBuffer	: public	SSystem::SByteBuffer
		{
		public:
			ERISA::MIO_DATA_HEADER	m_miodh ;
			uint64_t				m_nKeySample ;
		public:
			// 構築関数
			PreloadBuffer( size_t nLength ) ;
			PreloadBuffer( const PreloadBuffer& src ) ;
		} ;

		// キーフレームポイントオブジェクト
		struct	KeyPoint
		{
			uint64_t	m_nKeySample ;
			uint64_t	m_nRecOffset ;

			// 構築関数
			KeyPoint( void ) { }
			KeyPoint( const KeyPoint & key )
				: m_nKeySample(key.m_nKeySample),
					m_nRecOffset(key.m_nRecOffset ) { }
			// 代入
			const KeyPoint & operator = ( const KeyPoint & key )
			{
				m_nKeySample = key.m_nKeySample ;
				m_nRecOffset = key.m_nRecOffset ;
				return	*this ;
			}
		} ;

		// ERI メディアファイル
		SGLMediaFile			m_erif ;
		// 展開オブジェクト
		SGLSoundDecoder			m_decoder ;
		SSystem::SSmartPointer<SGLDecodeBitStream>
								m_bstream ;
		// 先読みキュー
		SSystem::SObjectArray<PreloadBuffer>
								m_queueSound ;
		uint64_t				m_nCurrentSample ;	// 現在読み込んでいるサンプル数
		// 音声シーク用キーポイント配列
		SSystem::SArray<KeyPoint>
								m_arrayKeySample ;

	public:
		// MIO ファイルを開く
		virtual SSystem::SError OpenSoundFile
			( SSystem::SFileInterface * pFile, bool flagOwner = false ) ;
		// MIO ファイルを閉じる
		virtual void Close( void ) ;

		// 指定サンプルへ移動し、初めのブロックのデータを取得する
		virtual uint8_t * GetWaveBufferFrom
			( uint64_t nSample,
				SSystem::SArray<uint8_t> & bufWave, uint32_t & nOffsetBytes ) ;
		// 次の音声データがストリームの先頭であるか？
		virtual bool IsNextDataRewound( void ) ;
		// 次の音声データを取得
		virtual uint8_t *
				GetNextWaveBuffer( SSystem::SArray<uint8_t> & bufWave ) ;

	public:
		// SGLMediaFile オブジェクトを取得する
		const SGLMediaFile & GetMediaFile( void ) const ;
		// チャネル数を取得する
		DWORD GetChannelCount( void ) const ;
		// サンプリング周波数を取得する
		DWORD GetFrequency( void ) const ;
		// サンプリングビット分解能を取得する
		DWORD GetBitsPerSample( void ) const ;
		// 全体の長さ（サンプル数）を取得する
		DWORD GetTotalSampleCount( void ) const ;

	protected:
		// 先読みバッファを取得する
		virtual PreloadBuffer * GetPreloadBuffer( void ) ;
		// 先読みバッファに追加する
		virtual void AddPreloadBuffer( PreloadBuffer * pBuffer ) ;

	protected:
		// 音声データレコードを読み込む
		PreloadBuffer * LoadSoundStream( uint64_t & nCurrentSample ) ;
		// キーフレームポイントを追加する
		void AddKeySample( const KeyPoint & key ) ;
		// 指定のキーフレームを検索する
		const KeyPoint * SearchKeySample( uint64_t nKeySample ) ;
		// 指定のサンプルを含むブロックを読み込む
		void SeekKeySample( uint64_t nSample, uint64_t & nCurrentSample ) ;
	} ;

}

#endif
