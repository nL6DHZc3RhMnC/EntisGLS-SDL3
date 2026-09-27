
#if	!defined(__SAKURAGL_MEDIA_SOUND_PLAYER_H__)
#define	__SAKURAGL_MEDIA_SOUND_PLAYER_H__	1

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// PCM サウンド出力フォーマット
	//////////////////////////////////////////////////////////////////////////

	enum	SGLSoundFormatFlag
	{
		formatSoundLinearPCM	= 0,
		formatSoundIEEEFloat	= 1,
	} ;
	struct	SGLSoundFormat
	{
		uint32_t	format ;
		uint32_t	frequency ;
		uint32_t	channels ;
		uint32_t	bitsPerSample ;

		#if	!defined(__COTOPHA__)
		SGLSoundFormat( void )
			: format(0), frequency(0), channels(0), bitsPerSample(0) {}
		#endif
		uint64_t SamplesToBytes( uint64_t nSamples ) const
		{
			return	(nSamples * bitsPerSample * channels) >> 3 ;
		}
		uint64_t BytesToSamples( uint64_t nBytes ) const
		{
			if ( (bitsPerSample * channels) != 0 )
				return	(nBytes << 3) / (bitsPerSample * channels) ;
			return	0 ;
		}
		uint64_t SamplesToMilliSec( uint64_t nSamples ) const
		{
			if ( frequency != 0 )
				return	nSamples * 1000 / frequency ;
			return	0 ;
		}
		uint64_t MilliSecToSamples( uint64_t nMilliSec ) const
		{
			return	nMilliSec * frequency / 1000 ;
		}
	} ;


	//////////////////////////////////////////////////////////////////////////
	// PCM サウンド出力インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SGLSoundPlayerListener ;
	class	SGLSoundPlayerInterface	: public SSystem::SObject
	{
	protected:
		SGLSoundPlayerListener *	m_pListener ;

	public:
		// 再生フラグ
		enum	PlayFlags
		{
			flagPlayLoop	= 0x0001,
		} ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLSoundPlayerInterface, SObject )
		// 構築関数
		SGLSoundPlayerInterface( void ) ;
		// フォーマットを指定して出力を準備する
		virtual SGLError Open( const SGLSoundFormat& fmt ) = 0 ;
		// 出力用に準備したサウンド出力を解放する
		virtual SGLError Close( void ) = 0 ;
		// スタティックバッファを準備して書き込む
		virtual SGLError WriteStatic( const void * ptrSound, size_t nBytes ) = 0 ;
		// ストリームバッファを準備する
		virtual SGLError PrepareStream( size_t nBytes = 0 ) = 0 ;
		// ストリームバッファへ書き出す
		virtual size_t Write( const void * ptrSound, size_t nBytes ) = 0 ;
		// 再生を開始する
		virtual SGLError Play( uint64_t nFlags = 0 ) = 0 ;
		// 再生を停止する
		virtual SGLError Stop( void ) = 0 ;
		// 再生を一時停止する
		virtual SGLError Pause( void ) = 0 ;
		// 再生を再開する
		virtual SGLError Restart( void ) = 0 ;
		// 音量取得 [L/R]
		virtual SGLError GetVolume( float32_t* pVolumes, size_t nChannels ) = 0 ;
		// 音量設定 [L/R]
		virtual SGLError SetVolume( const float32_t* pVolumes, size_t nChannels ) = 0 ;
		// 再生中か？
		virtual bool IsPlaying( void ) const = 0 ;
		// 一時停止中か？
		virtual bool IsPaused( void ) const = 0 ;
		// 再生済みサンプル数を取得する
		virtual uint64_t GetPlayingPosition( void ) = 0 ;
		// 再生位置 [/bytes] を設定する（スタティックバッファのみ）
		virtual SGLError SeekPosition( uint64_t nPos ) = 0 ;
		// コールバック設定
		virtual SGLSoundPlayerListener *
					SetListener( SGLSoundPlayerListener * listener ) ;

	} ;

	#if	defined(__COTOPHA__)
	class	native SoundPlayer	: public SSystem::VolatileObject
	{
	public:
		// 再生フラグ
		enum	PlayFlags
		{
			flagPlayLoop	= 0x0001,
		} ;
		// フォーマットを指定して出力を準備する
		native SGLError Open( const SGLSoundFormat& fmt ) ;
		// 出力用に準備したサウンド出力を解放する
		native SGLError Close( void ) ;
		// スタティックバッファを準備して書き込む
		native SGLError WriteStatic( const void * ptrSound, size_t nBytes ) ;
		// ストリームバッファを準備する
		native SGLError PrepareStream( size_t nBytes = 0 ) ;
		// ストリームバッファへ書き出す
		native size_t Write( const void * ptrSound, size_t nBytes ) ;
		// 再生を開始する
		native SGLError Play( uint64_t nFlags = 0 ) ;
		// 再生を停止する
		native SGLError Stop( void ) ;
		// 再生を一時停止する
		native SGLError Pause( void ) ;
		// 再生を再開する
		native SGLError Restart( void ) ;
		// 音量取得 [L/R]
		native SGLError GetVolume( float32_t* pVolumes, size_t nChannels ) ;
		// 音量設定 [L/R]
		native SGLError SetVolume( const float32_t* pVolumes, size_t nChannels ) ;
		// 再生中か？
		native bool IsPlaying( void ) const ;
		// 一時停止中か？
		native bool IsPaused( void ) const ;
		// 再生済みサンプル数を取得する
		native uint64_t GetPlayingPosition( void ) ;
		// 再生位置 [/bytes] を設定する（スタティックバッファのみ）
		native SGLError SeekPosition( uint64_t nPos ) ;
		// コールバック設定
		native SGLSoundPlayerListener *
					SetListener( SGLSoundPlayerListener * listener ) ;
	} ;
	#else
	typedef	SGLSoundPlayerInterface	SoundPlayer ;
	#endif


	//////////////////////////////////////////////////////////////////////////
	// PCM サウンド出力リスナー
	//////////////////////////////////////////////////////////////////////////

	class	SGLSoundPlayerListener
	{
	public:
		// クラス情報
		ESL_DECLARE_NV_CLASS_INFO( SGLSoundPlayerListener )
		// バッファへの出力タイミング
		virtual void OnStreaming( SoundPlayer * player ) = 0 ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// PCM サウンド出力ラッパー
	//////////////////////////////////////////////////////////////////////////

	class	SGLSoundPlayer	: public SGLSoundPlayerInterface
	{
	protected:
		SoundPlayer *	m_pPlayer ;
		bool			m_flagOwner ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLSoundPlayer, SGLSoundPlayerInterface )
		// 構築関数
		SGLSoundPlayer( void ) : m_pPlayer(NULL), m_flagOwner(false) {}
		SGLSoundPlayer( SoundPlayer* pPlayer, bool flagOwner = false )
						: m_pPlayer(pPlayer), m_flagOwner(flagOwner) {}
		// 消滅関数
		virtual ~SGLSoundPlayer( void ) ;
		// 代入
		SoundPlayer * operator = ( SoundPlayer * pPlayer )
		{
			return	SetSoundPlayer( pPlayer ) ;
		}
		SoundPlayer * SetSoundPlayer
				( SoundPlayer * pPlayer, bool flagOwner = false ) ;
		// ポインタ変換
		SoundPlayer * operator -> ( void ) const
		{
			return	m_pPlayer ;
		}
		SoundPlayer * GetPlayer( void ) const
		{
			return	m_pPlayer ;
		}
		operator SoundPlayer * ( void ) const
		{
			return	m_pPlayer ;
		}

	public:
		typedef SGLSoundPlayerInterface * (*PFUNC_NEW_PLAYER)( void * pInstance ) ;

		// プレイヤー生成関数設定
		static void SetPlayerCreator
				( PFUNC_NEW_PLAYER pfnNewPlayer, void * pInstance ) ;

	protected:
		static PFUNC_NEW_PLAYER	m_pfnNewPlayer ;
		static void *			m_pNewPlayerInstance ;

	public:
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
	} ;


}

#endif
