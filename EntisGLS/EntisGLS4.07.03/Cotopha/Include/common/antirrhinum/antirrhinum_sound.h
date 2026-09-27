
#if	!defined(__ANTIRRHINUM_SOUND_H__)
#define	__ANTIRRHINUM_SOUND_H__

#include <sakuracl/scl_erisa_lib.h>

namespace	AntirrhinumGL
{
	//////////////////////////////////////////////////////////////////////////
	// サウンド処理
	//////////////////////////////////////////////////////////////////////////

	class	AGLSoundProcessor	: public AGLEpicFuncProcessor
	{
	public:
		// プレイリスト
		class	Playlist	: public SSystem::SObjectArray<SSystem::SString>
		{
		public:
			enum	PlayType
			{
				playInOrder,
				playReverse,
				playRandom,
			} ;
			PlayType	m_typePlay ;

			static const SSystem::SXMLDocument::AttrInteger	m_aiPlayType[4] ;

		public:
			// 構築関数
			Playlist( void ) ;
			// xml 形式プレイリストを読み込む (*.xml;*.wpl)
			// <smil><body><seq><media src="...">...
			bool LoadPlaylist( const wchar_t * pwszFilePath ) ;
		} ;

		// 再生チャネル
		class	Channel
		{
		public:
			enum	LoopType
			{
				loopNo,
				loopForce,
				loopAuto,
			} ;
			SSystem::SSmartPointer
				<SakuraGL::SGLAudioPlayer>		m_player ;
			SSystem::SSmartPointer<Playlist>	m_playlist ;
			bool								m_flagPlay ;
			size_t								m_iPlayCurrent ;
			SakuraCL::SCLRandomizer				m_random ;
			SSystem::SString					m_strFileName ;
			LoopType							m_typeLoop ;
			uint32_t							m_maskVolLine ;
			double								m_fpVolume[2] ;

			static const SSystem::SXMLDocument::AttrInteger	m_aiLoopType[4] ;

		public:
			// 構築関数
			Channel( void ) ;
			// 構築関数
			~Channel( void ) ;

		public:
			// ファイルを開く
			bool OpenFile( const wchar_t * pwszFileName ) ;
			// 音量設定
			void SetVolume( double volLeft, double volRight ) ;
			void FadeVolume( double volLeft, double volRight, uint32_t msecFade ) ;
			// ループ設定
			void SetLoop( LoopType type ) ;
			// 音量ライン設定
			void SetVolumeLine( uint32_t maskLine ) ;
			// 再生開始
			void Play( void ) ;
			// 停止
			void Stop( uint32_t msecFadeout ) ;
			// 再生中か？
			bool IsPlaying( void ) const ;
			// プレイヤーデタッチ
			SakuraGL::SGLAudioPlayer * DetachPlayer( void ) ;

		protected:
			// 音量設定をプレイヤーに反映
			void ApplyVolumeToPlay( void ) ;
			// プレイリストの指定番号のファイルの再生を開始
			void StartPlaylistAt( size_t i ) ;

		public:
			// タイマ処理
			void OnTimer( AGLSoundProcessor& soundProc ) ;
			// 保存
			void Serialize( SSystem::SXMLDocument& xmlDoc ) ;
			// 復元
			void Deserialize( const SSystem::SXMLDocument& xmlDoc ) ;
		} ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( AGLSoundProcessor, AGLEpicFuncProcessor )
		AGL_DECLARE_EPIC_PROCESSOR( AGLSoundProcessor )
		// 構築関数
		AGLSoundProcessor( void ) ;
		// 消滅関数
		virtual ~AGLSoundProcessor( void ) ;

	protected:
		SSystem::SCriticalSection		m_csSync ;
		SSystem::SObjectArray<Channel>	m_aChannels ;
		SSystem::SObjectArray
			<SakuraGL::SGLAudioPlayer>	m_aFadeout ;

		SSystem::SObjectArray<SSystem::SString>	m_aVolLineNames ;

	public:
		// ファイルを開く
		bool OpenFile( size_t iChannel, const wchar_t * pwszFileName ) ;
		// 音量設定
		void SetVolume( size_t iChannel, double volLeft, double volRight ) ;
		void FadeVolume
			( size_t iChannel, double volLeft, double volRight, uint32_t msecFade ) ;
		// ループ設定
		void SetLoop( size_t iChannel, Channel::LoopType type ) ;
		// 音量ライン設定
		void SetVolumeLine( size_t iChannel, uint32_t maskLine ) ;
		// 再生開始
		void PlayChannel( size_t iChannel ) ;
		// 停止
		void StopChannel( size_t iChannel, uint32_t msecFadeout ) ;
		void StopAllChannels( uint32_t msecFadeout ) ;
		// 解放
		void ReleaseChannel( size_t iChannel ) ;
		void ReleaseAllChannels( void ) ;

	public:
		// チャネル取得
		Channel * SafeChannelAt( size_t iChannel ) ;
		// フェードアウトリストに追加
		void AddFadeoutPlayer( SakuraGL::SGLAudioPlayer * pPlayer ) ;

	public:
		// 音量ライン名設定
		void SetVolumeLineName( size_t i, const wchar_t * pwszName ) ;
		// 音量ライン番号取得
		size_t GetVolumeLineAs
			( const wchar_t * pwszName, size_t iDefault ) const ;

	public:	// AGLObject
		// シリアライズ
		virtual SSystem::SError Serialize
				( SSystem::SXMLDocument& xmlTag, AGLKernel * pKernel ) ;
		// デシリアライズ
		virtual SSystem::SError Deserialize
				( const SSystem::SXMLDocument& xmlTag, AGLKernel * pKernel ) ;
		// デシリアライズ後の参照解決処理
		virtual SSystem::SError AfterDeserialize( AGLKernel * pKernel ) ;

	public:	// AGLEpicProcessor
		// 設定
		virtual void LoadConfiguration
			( const SSystem::SXMLDocument& xmlConfig ) ;
		// タイマ処理 (実行フレーム前処理)
		virtual void OnKernelTimer( void ) ;
		// ゲーム終了前フェードアウト処理
		virtual void FadeoutGame( uint32_t msecFadeout ) ;
		// ゲーム終了時処理
		virtual void ReleaseGame( void ) ;

	public:
		// コマンド実装
		DECL_ANTIRRHINUM_PROC(AGLSoundProcessor,sound_play)
		DECL_ANTIRRHINUM_PROC(AGLSoundProcessor,sound_stop)
		DECL_ANTIRRHINUM_PROC(AGLSoundProcessor,sound_volume)
		DECL_ANTIRRHINUM_PROC(AGLSoundProcessor,sound_wait)

	} ;

}

#endif
