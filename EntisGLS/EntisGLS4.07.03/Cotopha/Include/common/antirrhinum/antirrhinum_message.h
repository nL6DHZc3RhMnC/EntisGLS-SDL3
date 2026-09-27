
#if	!defined(__ANTIRRHINUM_MESSAGE_H__)
#define	__ANTIRRHINUM_MESSAGE_H__

namespace	AntirrhinumGL
{
	//////////////////////////////////////////////////////////////////////////
	// メッセージ／ボイス処理
	//////////////////////////////////////////////////////////////////////////

	class	AGLMessageProcessor	: public AGLEpicFuncProcessor
	{
	public:
		// キャラクター設定
		class	CharConfig
		{
		public:
			ssize_t					m_nIndex ;		// インデックス
			SSystem::SString		m_idChar ;		// キャラクタ識別子（空文字列でモノローグ設定）
			SSystem::SString		m_strName ;		// キャラクタ表示名（デフォルト）
			SSystem::SIntSortObjectArray
				<SSystem::SString>	m_isoaName ;	// キャラクタ表示名（LanguageType 毎）
			SSystem::SString		m_strFaceLFile ;
			SSystem::SString		m_strBsLFile ;
			SSystem::SString		m_strVoiceLFile ;
			SSystem::SObjectArray<SSystem::SUsageMatcher>
									m_aBsFileMatching ;
			SSystem::SObjectArray<SSystem::SUsageMatcher>
									m_aVoiceFileMatching ;
			SSystem::SObjectArray<SSystem::SUsageMatcher>
									m_aVoiceFileUnmatching ;
			size_t					m_iVolLineOffset ;
			SakuraGL::SGLPalette	m_argbText ;
			SakuraGL::SGLPalette	m_argbBorder ;
			SakuraGL::SGLPalette	m_argbShadow ;
		} ;

		// 動作フラグ
		enum	BehaviorFlag
		{
			behaviorStopSkipNoRead		= 0x00000001,	// 未読メッセージで SKIP モード停止
			behaviorSkipTime			= 0x00000002,	// SKIP モードで syncTypeTime をスキップ
			behaviorSkipMessage			= 0x00000004,	// SKIP モードで syncTypeMessage をスキップ
			behaviorSkipEffect			= 0x00000008,	// SKIP モードで syncTypeEffect をスキップ
			behaviorSkipEvent			= 0x00000010,	// SKIP モードで syncTypeEvent をスキップ
			behaviorStopFastSkipNoRead	= 0x00000100,	// 未読メッセージで FAST SKIP モード停止
			behaviorFastSkipTime		= 0x00000200,	// FAST SKIP モードで syncTypeTime をスキップ
			behaviorFastSkipMessage		= 0x00000400,	// FAST SKIP モードで syncTypeMessage をスキップ
			behaviorFastSkipEffect		= 0x00000800,	// FAST SKIP モードで syncTypeEffect をスキップ
			behaviorFastSkipEvent		= 0x00001000,	// FAST SKIP モードで syncTypeEvent をスキップ
			behaviorSkipAll				= 0x00001E1E,
			behaviorStopSkipSelector	= 0x00002000,	// 選択肢で SKIP / FAST SKIP モード停止
			behaviorSaveInLog			= 0x00001000,	// メッセージログにセーブデータを保存
			behaviorNoSaveInSkip		= 0x00002000,	// SKIP モード時にはログにセーブしない
			behaviorNoSaveInFastSkip	= 0x00004000,	// FAST SKIP モード時にはログにセーブしない
			behaviorClickCancelAuto		= 0x00010000,	// クリックで AUTO モード解除
			behaviorClickCancelSkip		= 0x00020000,	// クリックで SKIP モード解除
			behaviorClickCancelFastSkip	= 0x00040000,	// クリックで FAST SKIP モード解除
			behaviorClickCancelAll		= 0x00070000,
		} ;

		// 選択肢
		class	UISelector	: public SSystem::SObject
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( UISelector, SObject )
			// 選択肢初期化
			virtual void ResetSelector( void ) = 0 ;
			// 選択肢項目追加
			virtual void AddSelectorItem
				( const wchar_t * pwszText,
					int nValue, bool flagHistorySelected,
					const SSystem::SXMLDocument& xmlOptions ) = 0 ;
			// 表示開始
			virtual void StartSelection( void ) = 0 ;
			// 表示終了
			virtual void EndSelection( void ) = 0 ;
			// 表示中のタイマー処理（スクリプト駆動）
			virtual void OnAntirrhinumTimer( void ) = 0 ;
			// 表示効果（フェードイン・フェードアウト・スクロール等）中か？
			virtual bool IsPendingShowEffect( void ) = 0 ;
			// 時間制限の表示更新
			virtual void DisplayTimeout( uint32_t msecPast, uint32_t msecTotal ) = 0 ;
			// ユーザー選択されたか？
			virtual bool IsUserSeleced( int& nSelected ) = 0 ;
		} ;

		// メッセージ出力抽象インターフェース
		class	UIMessage	: public SSystem::SObject
		{
		public:
			// キー待ちアイコン
			enum	KeyWaitType
			{
				keyWaitHide,		// 非表示
				keyWaitClick,		// クリック待ち
				keyWaitInAuto,		// AUTO モード
				keyWaitInSkip,		// SKIP モード
			} ;
			// トグルボタン
			enum	ToggleButtonIndex
			{
				toggleAuto,			// オートモード
				toggleSkip,			// スキップモード
				toggleFastSkip,		// 高速スキップモード
				toggleSceneSkip,	// シーンスキップボタン
				toggleHWKeySkip,	// ハードウェアキー・スキップ
			} ;
			// クラス情報
			ESL_DECLARE_CLASS_INFO( UIMessage, SObject )
			// 表示中のタイマー処理（スクリプト駆動）
			virtual void OnAntirrhinumTimer( void ) = 0 ;
			// ウィンドウの表示状態取得
			virtual bool IsShowWindow( void ) = 0 ;
			// ウィンドウの表示／非表示状態変更（フェード開始）
			virtual void ShowWindow( bool flagShow, uint32_t msecTime ) = 0 ;
			// ウィンドウのフェード中か？
			virtual bool IsPendingShowEffect( void ) = 0 ;
			// ウィンドウのフェード処理の即時完了
			virtual void FinishShowWindow( void ) = 0 ;
			// キャラクター毎に吹き出しを生成するような場合、吹き出しの準備
			// また文字色などの設定の反映（NULL の場合モノローグなどのデフォルト処理）
			virtual void PrepareMessage( const CharConfig * pcfg, bool flagReadMsg ) = 0 ;
			// フェイス画像の表示
			virtual void DisplayFace( const wchar_t * pwszFace ) = 0 ;
			// 名前の表示
			virtual void DisplayName( const wchar_t * pwszName ) = 0 ;
			// メッセージ出力開始（追加出力）
			virtual void StartMessage( const wchar_t * pwszMsg ) = 0 ;
			// メッセージ出力処理中か？
			virtual bool IsPendingMessage( void ) = 0 ;
			// 現在のメッセージの AUTO タイムアウト時間計算
			virtual uint32_t GetCurrentAutoTimeout( void ) = 0 ;
			// メッセージ出力の即時完了
			virtual void FinishMessage( void ) = 0 ;
			// 現在表示されているメッセージのプレーンテキスト取得
			virtual const wchar_t * GetMessagePlainText( void ) = 0 ;
			// フェイス画像の消去
			virtual void ClearFace( void ) = 0 ;
			// 名前の消去
			virtual void ClearName( void ) = 0 ;
			// メッセージの消去（フェードアウト）
			virtual void ClearMessage( size_t nFadeout = 0 ) = 0 ;
			// メッセージのフェードアウト中か？
			virtual bool IsPendingClearMessage( void ) = 0 ;
			// キー待ちアイコンの表示状態設定
			virtual void ShowKeyWait( KeyWaitType type ) = 0 ;
			// 現在のキー待ちアイコンの表示状態設定
			virtual KeyWaitType CurrentKeyWait( void ) = 0 ;
			// トグルUI状態取得
			virtual bool IsToggleButton( ToggleButtonIndex tbi ) = 0 ;
			// トグルUI状態設定
			virtual void SetToggleButton( ToggleButtonIndex tbi, bool flagPushed ) = 0 ;
			// トグルUI禁止状態設定
			virtual void EnableToggleButton( ToggleButtonIndex tbi, bool flagEnabled ) = 0 ;
			// 選択肢取得
			virtual UISelector * GetSelector( void ) = 0 ;
			// 選択肢解放
			virtual void ReleaseSelector( UISelector * pSelector ) = 0 ;
		} ;

		// 音声位置マーカーファイル
		class	SoundMarkList
		{
		public:
			struct	Marker
			{
				uint64_t	nPos ;		// [samples]
				uint64_t	nLength ;	// [samples]
			} ;

		protected:
			SSystem::SStrSortArray<Marker>	m_ssaMarkers ;

		public:
			// ファイル読み込み
			SSystem::SError LoadMarkerFile( const wchar_t * pwszFilePath ) ;
			SSystem::SError ParseMarkerFile( const SSystem::SXMLDocument& xmlMarkers ) ;
			// マーカー取得
			const Marker * GetMarkerAs( const wchar_t * pwszID ) const ;
			const Marker * GetMarkerAt( size_t i ) const ;
			size_t GetMarkerCount( void ) const ;
		} ;

		// ボイスプレイヤー
		class	VoicePlayer	: public SSystem::SObject
		{
		protected:
			bool							m_flagStarted ;
			SSystem::SSmartPointer
				<SakuraGL::SGLAudioPlayer>	m_pPlayer ;
			SSystem::SString				m_strFileName ;
			const CharConfig *				m_pCharCfg ;
			SSystem::SSmartPointer
						<SoundMarkList>		m_pMarkers ;
			uint32_t						m_nFlags ;

		public:
			ESL_DECLARE_CLASS_INFO( VoicePlayer, SObject )
			VoicePlayer
				( SakuraGL::SGLAudioPlayer * pPlayer,
					const wchar_t * pwszFileName,
					const CharConfig * pcfg,
					SoundMarkList * pMarkerList, uint32_t nFlags ) ;
			void SetAudioPlayer( SakuraGL::SGLAudioPlayer * pPlayer ) ;
			SakuraGL::SGLAudioPlayer * GetAudioPlayer( void ) const ;
			const SSystem::SString& GetFileName( void ) const ;
			const CharConfig * GetCharConfig( void ) const ;
			const SoundMarkList * GetSoundMarkerList( void ) const ;
			const SoundMarkList::Marker * GetSoundMarkerAs( const wchar_t * pwszID ) const ;
			uint32_t GetFlags( void ) const ;

			friend class AGLMessageProcessor ;
		} ;

		class	Voice	: public SSystem::SSmartReference<VoicePlayer>
		{
		public:
			Voice( void ) {}
			Voice( const Voice& voice )
				: SSystem::SSmartReference<VoicePlayer>( voice ) {}
			Voice( VoicePlayer* vplayer )
				: SSystem::SSmartReference<VoicePlayer>( vplayer ) {}
			bool IsPlaying( void ) const ;
			uint32_t GetSampleFrequency( void ) const ;
			uint64_t GetPlayingPosition( void ) const ;
			SakuraGL::SGLAudioPlayer * GetAudioPlayer( void ) const ;
			const CharConfig * GetCharConfig( void ) const ;
			const SoundMarkList * GetSoundMarkerList( void ) const ;
			const SoundMarkList::Marker * GetSoundMarkerAs( const wchar_t * pwszID ) const ;
		} ;

		// ボイス再生通知
		class	VoiceListener	: public ESLObject
		{
		public:
			ESL_DECLARE_CLASS_INFO( VoiceListener, ESLObject )
			virtual void OnLoadedVoice( const Voice& voice ) = 0 ;
			virtual void OnVoiceVolume( const Voice& voice, float32_t vols[2] ) = 0 ;
			virtual void OnStartedVoice( const Voice& voice ) = 0 ;
			virtual void OnFinishedVoice( const Voice& voice ) = 0 ;
		} ;

		// シンプルな口パク用解析クラス
		class	SimpleVoiceAnalyzer
		{
		protected:
			SSystem::SArray<float32_t>	m_aVoiceVelocity ;
			SakuraGL::SGLSoundFormat	m_fmtVoice ;
			size_t						m_nUnitSamples ;
		public:
			// 構築関数
			SimpleVoiceAnalyzer( void ) ;
			// 複製
			const SimpleVoiceAnalyzer& operator = ( const SimpleVoiceAnalyzer& sva ) ;
			// 解析
			bool AnalyzeVoice( SakuraGL::SGLAudioPlayer * pPlayer, double secUnit ) ;
			// バッファ解放
			void Release( void ) ;
			// 口パク用係数取得 [0,1]
			float32_t GetVoiceVelocityInSamples( uint64_t iSample ) const ;
			float32_t GetVoiceVelocityInUnits( size_t iUnit ) const ;
			// 口パク係数要素数
			size_t GetVoiceVelocityLength( void ) const ;
			size_t GetVoiceVelocityUnit( void ) const ;
			// 解析したフォーマット
			const SakuraGL::SGLSoundFormat& GetSoundFormat( void ) const ;
		} ;

		// メッセージ・ログ
		class	MessageLog
		{
		public:
			const CharConfig *			m_pCharCfg ;
			SSystem::SString			m_strFace ;
			SSystem::SString			m_strName ;
			SSystem::SString			m_strMessage ;
			SSystem::SObjectArray
				<SSystem::SString>		m_aVoices ;
			SSystem::SSmartPointer
				<SSystem::SXMLDocument>	m_pxmlSaved ;
		public:
			MessageLog( void ) : m_pCharCfg(NULL) {}
		} ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( AGLMessageProcessor, AGLEpicFuncProcessor )
		AGL_DECLARE_EPIC_PROCESSOR( AGLMessageProcessor )
		// 構築関数
		AGLMessageProcessor( void ) ;
		// 消滅関数
		virtual ~AGLMessageProcessor( void ) ;

	protected:
		// キャラクター設定
		SSystem::SObjectArray<CharConfig>	m_aCharConfig ;

		// 各種動作設定
		uint32_t	m_flagsBehavior ;		// 動作フラグ
		uint32_t	m_nSkipEffectSpeed ;	// スキップモード時の効果時間比率 [/100]
		uint32_t	m_msecMsgWindowFade ;	// メッセージウィンドウのデフォルト・フェード時間
		size_t		m_nMsgLogLimit ;		// メッセージログ最大数
		size_t		m_iFirstVolLine ;		// 音量ライン・基準番号

		// ボイス再生
		SSystem::SCriticalSection			m_csSync ;
		SSystem::SObjectArray<VoicePlayer>	m_aPlaying ;
		SSystem::SObjectArray<VoicePlayer>	m_aFadeout ;

		// メッセージウィンドウ
		SSystem::SString					m_strMsgWindowID ;
		SSystem::SSmartReference<UIMessage>	m_refMsgWindow ;

		bool		m_flagDelayClearFace ;
		bool		m_flagWaitingMsgClick ;

		uint32_t	m_maskDisableSkip ;			// スキップを禁止する SynchronismType 列挙子のビットマスク
		uint32_t	m_maskDisableClickSkip ;	// クリックでスキップを禁止する SynchronismType 列挙子のビットマスク
		uint32_t	m_maskKeepClickSkip ;		// クリックでキー入力フラグをクリアしない SynchronismType 列挙子のビットマスク
		uint32_t	m_maskDisableButton ;		// トグルボタンの禁止状態
		bool		m_flagSceneSkip ;			// シーンスキップボタン
		uint32_t	m_idSceneSkipThread ;
		SSystem::SString	m_strJumpSceneSkip ;

		// メッセージウィンドウの現在の表示状態
		bool				m_flagShowWindow ;
		SSystem::SString	m_strCurFace ;
		SSystem::SString	m_strCurName ;
		SSystem::SString	m_strCurMessage ;

		// メッセージログ
		SSystem::SObjectArray<MessageLog>	m_aMsgLog ;

		// 選択肢
		class	MenuItem
		{
		public:
			SSystem::SString		m_strText ;
			SSystem::SString		m_strJump ;
			int						m_nValue ;
			size_t					m_iMsgIndex ;
			bool					m_flagHistory ;
			SSystem::SXMLDocument	m_xmlMenuCmd ;
		} ;
		bool									m_flagShowSelector ;
		int32_t									m_nSelectionTimeout ;
		SSystem::SString						m_strSelResultTarget ;
		SSystem::SObjectArray<MenuItem>			m_aMenuItems ;
		SSystem::SSmartReference<UISelector>	m_refSelector ;

	public:
		// 音量ライン設定
		void SetFirstVolumeLine( size_t iLine, bool flagEnable ) ;
		// ボイス再生
		enum	PlayVoiceFlag
		{
			playSystemVoice	= 0x0001,
			playNoStart		= 0x0010,
		} ;
		virtual Voice PlayVoice
			( const wchar_t * pwszFileName, uint32_t nFlags = 0 ) ;
		virtual void PlayVoices
			( const wchar_t *const* ppwszFileNames, size_t nCount, uint32_t nFlags = 0 ) ;
		// 再生中の全ボイス停止
		virtual void StopAllVoices( uint32_t nFadeout = 500 ) ;

	public:
		// ボイス再生中か？
		virtual bool IsPlayingVoice( const Voice& voice ) const ;
		virtual bool ArePlayingAnyVoices( void ) const ;
		// ボイス再生位置通過判定
		virtual bool IsVoicePastMark( const wchar_t * pwszID ) const ;
		// ボイス音量設定
		virtual SSystem::SError SetVoiceVolume
			( Voice& voice, double volLeft, double volRight ) const ;
		// ボイス再生開始
		virtual SSystem::SError StartVoice( Voice& voice ) ;
		// ボイス停止
		virtual SSystem::SError
			StopVoice( Voice& voice, uint32_t nFadeout = 500 ) ;

	public:
		// ボイスファイルロード
		virtual VoicePlayer *
			LoadVoicePlayer
				( const wchar_t * pwszFileName,
					const CharConfig * pcfg, uint32_t nFlags ) ;
		// ボイスロード完了後処理
		virtual void OnLoadedVoice( const Voice& voice ) ;
		// ボイス音量効果
		virtual void OnVoiceVolume( const Voice& voice, float32_t vols[2] ) const ;
		// ボイス再生開始時処理
		virtual void OnStartedVoice( const Voice& voice ) ;
		// ボイス再生終了時処理
		virtual void OnFinishVoice( const Voice& voice ) ;

	public:
		// ファイル名マッチング
		const CharConfig * GetCharIndexOf( ssize_t iChar ) const ;
		const CharConfig * GetCharIdOf( const wchar_t * pwszID ) const ;
		const CharConfig * GetCharBsFileOf( const wchar_t * pwszFileName ) const ;
		const CharConfig * GetCharVoiceFileOf( const wchar_t * pwszFileName ) const ;

	public:	// User Interface Implementation
		// UIMessage 取得
		virtual UIMessage * GetMessageUI( const wchar_t * pwszID ) = 0 ;
		// 動作フラグ (BehaviorFlag 組み合わせ)
		virtual uint32_t GetBehaviorFlags( void ) ;
		// 操作キー入力
		enum	OperationKey
		{
			keyClickNext,
		} ;
		virtual bool GetPushedKeyState( OperationKey key = keyClickNext ) = 0 ;
		virtual void ClearPushedKeyState( OperationKey key = keyClickNext ) = 0 ;

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
		void LoadCharacterDefinition
			( const SSystem::SXMLDocument& xmlCharDef ) ;
		void LoadMessageConfig
			( const SSystem::SXMLDocument& xmlMessage ) ;
	protected:
		static void ParseCharConfigMatchingList
			( SSystem::SObjectArray<SSystem::SUsageMatcher>& aMatchers,
				const SSystem::SXMLDocument * pxmlFileFilters ) ;

	public:
		// タイマ処理 (実行フレーム前処理)
		virtual void OnKernelTimer( void ) ;
		void CleanupFadeoutVoices( void ) ;
		void OnTimerClearFace( void ) ;
		void OnTimerKeyWaitDisplay( void ) ;
		void OnTimerSceneJump( void ) ;
		void OnTimerMessageWindow( void ) ;
		void OnTimerSelector( void ) ;

		// ゲーム開始時処理
		virtual void InitializeGame( void ) ;
		// ゲーム終了前フェードアウト処理
		virtual void FadeoutGame( uint32_t msecFadeout ) ;
		// ゲーム終了時処理
		virtual void ReleaseGame( void ) ;
		// 待機関数を（ユーザー入力等により）即時に脱出すべきか判定する
		virtual bool ShouldAbortSync( SynchronismType type ) ;
		// 待機関数を ShouldAbortSync を理由に脱出したことの通知
		virtual void NotifyAbortedSync( SynchronismType type ) ;
		// フェード処理などの効果継続時間の効果
		virtual uint32_t EffectTime
			( uint32_t msecTime, SynchronismType type = syncTypeEffect ) ;

	public:
		// コマンド実装
		DECL_ANTIRRHINUM_PROC(AGLMessageProcessor,voice)
		DECL_ANTIRRHINUM_PROC(AGLMessageProcessor,voices)
		DECL_ANTIRRHINUM_PROC(AGLMessageProcessor,voice_stop)
		DECL_ANTIRRHINUM_PROC(AGLMessageProcessor,voice_sync)
		DECL_ANTIRRHINUM_PROC(AGLMessageProcessor,msg_window_id)
		DECL_ANTIRRHINUM_PROC(AGLMessageProcessor,msg_show_window)
		DECL_ANTIRRHINUM_PROC(AGLMessageProcessor,msg_set_face)
		DECL_ANTIRRHINUM_PROC(AGLMessageProcessor,msg_clear_face)
		DECL_ANTIRRHINUM_PROC(AGLMessageProcessor,msg_pause)
		DECL_ANTIRRHINUM_PROC(AGLMessageProcessor,msg_sync)
		DECL_ANTIRRHINUM_PROC(AGLMessageProcessor,msg)
		DECL_ANTIRRHINUM_PROC(AGLMessageProcessor,msg_clear)
		DECL_ANTIRRHINUM_PROC(AGLMessageProcessor,enable_skip)
		DECL_ANTIRRHINUM_PROC(AGLMessageProcessor,disable_skip)
		DECL_ANTIRRHINUM_PROC(AGLMessageProcessor,keep_skip)
		DECL_ANTIRRHINUM_PROC(AGLMessageProcessor,scene_skip)
		DECL_ANTIRRHINUM_PROC(AGLMessageProcessor,begin_selector)
		DECL_ANTIRRHINUM_PROC(AGLMessageProcessor,add_menu_item)
		DECL_ANTIRRHINUM_PROC(AGLMessageProcessor,end_selector)

	public:
		// メッセージウィンドウの表示状態設定
		void ShowMessageWindow( bool flagShow, uint32_t msecTime ) ;
		// メッセージウィンドウのフェイス画像表示
		void DisplayFace( const wchar_t * pwszFaceFile ) ;
		// メッセージウィンドウの名前文字列設定
		void DisplayName( const wchar_t * pwszName ) ;
		// メッセージウィンドウのメッセージ文字列設定
		void DisplayMessage( const wchar_t * pwszMessage, bool flagFinish = false ) ;
		// 現在のメッセージウィンドウの状態をクリア
		void ClearCurrentMessageWindow( void ) ;
		// トグルボタンの有効・禁止状態反映
		void ApplyDisableToggleButtonFlags( void ) ;
		// トグルボタン状態取得
		bool IsToggleButton( UIMessage::ToggleButtonIndex tbi ) const ;
		// スキップモードか？（SKIP | FAST SKIP | SKIP キー押下）
		bool IsSkipMode( void ) const ;
		// オートモードーか？
		bool IsAutoMode( void ) const ;
		// 既読フラグ取得
		bool IsReadMessage( const wchar_t * pwszScript, size_t iMsgIndex ) ;
		// 既読フラグ設定
		void SetReadMessage( const wchar_t * pwszScript, size_t iMsgIndex ) ;
		// 既読フラグを取得し、対応する処理と既読フラグの設定を行う
		bool TestReadMessage( const wchar_t * pwszScript, size_t iMsgIndex ) ;
		// メッセージ出力開始
		void StartMessageOutput( AGLThread& thread, const AGLCode& code ) ;
		// メッセージクリック待ち
		AntirrhinumGL::CodeProcessResult WaitClickMessage
			( AGLThread& thread, const AGLCode& code,
								SSystem::SXMLDocument * pxmlStorage ) ;
		// 選択肢表示構築
		void BuildSelector( void ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// ユーザー入力標準実装
	//////////////////////////////////////////////////////////////////////////

	class	AGLStdMessageProcessor	: public AGLMessageProcessor
	{
	public:
		// 選択肢設定
		class	StdSelectorConfig
		{
		public:
			SakuraGL::SGLSkinManager *	m_pSkin ;
			SakuraGL::SGLSprite *		m_pScreen ;
			int32_t						m_nPriority ;
			SSystem::SString			m_strFormID ;
			SSystem::SString			m_strHistoryFormID ;
			SSystem::SString			m_strTextID ;
			SSystem::SString			m_strButtonID ;
			SakuraGL::SGLPoint			m_ptBaseOffset ;
			SakuraGL::SGLPoint			m_ptOffsetByCount ;
			SakuraGL::SGLPoint			m_ptOffsetByIndex ;
			uint32_t					m_msecItemFadeTime ;
			uint32_t					m_msecItemFadeDelay ;
			SakuraGL::SGLPoint			m_ptOffsetFadein ;
			SakuraGL::SGLPoint			m_ptOffsetFadeout ;

		public:
			// 構築
			StdSelectorConfig( void ) ;
			// 全設定
			void SetSelectorConfig( const StdSelectorConfig& cfg ) ;
			// SGLSkinManager 設定
			void AttachSkinManager( SakuraGL::SGLSkinManager * pSkin ) ;
			// 表示先設定
			void AttachScreen( SakuraGL::SGLSprite * pScreen, int32_t nPriority ) ;
			// フォーム設定
			void SetFormConfig
				( const wchar_t * pwszFormID,
					const wchar_t * pwszHisFormID,
					const wchar_t * pwszTextID,
					const wchar_t * pwszButtonID,
					const SakuraGL::SGLPoint& ptBaseOffset,
					const SakuraGL::SGLPoint& ptOffsetByCount,
					const SakuraGL::SGLPoint& ptOffsetByIndex ) ;
			// 表示時間設定
			void SetFadeTime
				( uint32_t msecFadeTime, uint32_t msecFadeDelay ) ;
			// フェードイン・アニメーション
			void SetFadeOffset
				( const SakuraGL::SGLPoint& ptOffsetFadein,
						const SakuraGL::SGLPoint& ptOffsetFadeout ) ;
		} ;

		// 選択肢実装
		class	StdSelector	: public UISelector, public StdSelectorConfig
		{
		protected:
			SSystem::SObjectArray<SakuraGL::SGLSprite>	m_aMenuItems ;
			SSystem::SArray<SakuraGL::S2DDVector>		m_aMenuPoints ;
			SSystem::SArray<int>						m_nMenuValues ;

			enum	Status
			{
				statusDone,
				statusFadein,
				statusFadeout,
				statusSelecting,
			} ;
			Status					m_status ;
			size_t					m_iFadeStarted ;
			bool					m_flagSelected ;
			size_t					m_iSeleced ;
			SSystem::STimeCounter	m_timer ;

		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( StdSelector, UISelector )
			// 構築
			StdSelector( void ) ;
			// アニメーション効果
			virtual SakuraGL::SGLPoint GetAnimationOffset( size_t iMenu, bool flagFadeout ) ;

		public:
			// 選択肢初期化
			virtual void ResetSelector( void ) ;
			// 選択肢項目追加
			virtual void AddSelectorItem
				( const wchar_t * pwszText,
					int nValue, bool flagHistorySelected,
					const SSystem::SXMLDocument& xmlOptions ) ;
			// 表示開始
			virtual void StartSelection( void ) ;
			// 表示終了
			virtual void EndSelection( void ) ;
			// 表示中のタイマー処理（スクリプト駆動）
			virtual void OnAntirrhinumTimer( void ) ;
			// 表示効果（フェードイン・フェードアウト・スクロール等）中か？
			virtual bool IsPendingShowEffect( void ) ;
			// 時間制限の表示更新
			virtual void DisplayTimeout( uint32_t msecPast, uint32_t msecTotal ) ;
			// ユーザー選択されたか？
			virtual bool IsUserSeleced( int& nSelected ) ;
		} ;

		// メッセージウィンドウ要素
		enum	MessageSpriteElement
		{
			msgElementWindow,
			msgElementMessage,
			msgElementName,
			msgElementNameFrame,
			msgElementIconClick,
			msgElementIconAuto,
			msgElementIconSkip,
			msgElementCount,
		} ;

		// スプライト位置情報
		struct	Position
		{
			SakuraGL::S2DDVector	vPosition ;
			SakuraGL::S2DDVector	vZoom ;
			uint32_t				nTransparency ;
		} ;

		// メッセージウィンドウ設定
		class	StdMessageConfig
		{
		public:
			SSystem::SSyncReference	m_refElement[msgElementCount] ;
			SakuraGL::SGLPoint		m_ptFaceOffset ;
			int32_t					m_nFacePriority ;
			uint32_t				m_msecFaceFadeTime ;
			uint32_t				m_nAutoSpeed ;
			bool					m_flagMsgWndPos ;
			Position				m_posMsgWindow[2] ;	// 非表示, 表示位置

		public:
			// 構築
			StdMessageConfig( void ) ;
			// 全設定
			void SetMessageConfig( const StdMessageConfig& cfg ) ;
			// ウィンドウ要素関連付け
			void AttachSpriteElement
				( MessageSpriteElement mseIndex, SakuraGL::SGLSprite * pSprite ) ;
			void AttachFormItemElement
				( MessageSpriteElement mseIndex, SakuraGL::SGLBasicForm::Item * pItem ) ;
			// フェイス画像設定
			void SetFaceConfig
				( const SakuraGL::SGLPoint& ptOffset,
						int32_t nPriority, uint32_t msecFadeTime ) ;
			// オートモード速度（待ち時間）
			void SetAutoModeSpeed( uint32_t nAutoSpeed ) ;
			// オートモード速度と文字数からタイムアウト時間計算
			virtual uint32_t GetAutoMessageTime( uint32_t nAutoSpeed, uint32_t nCharCount ) ;
			// 非表示・表示位置を設定
			void SetShowPosition( const Position& posHide, const Position& posShow ) ;
		} ;

		// メッセージウィンドウ実装
		class	StdMessage	: public UIMessage, public StdMessageConfig
		{
		protected:
			AGLStdMessageProcessor *			m_pStdMsg ;
			SSystem::SSmartPointer<UISelector>	m_pSelector ;

			bool								m_flagShowWindow ;

			SSystem::SString					m_strFaceFile ;
			SSystem::SString					m_strFaceFadeout ;
			SSystem::SSmartPointer
					<SakuraGL::SGLSprite>		m_pFaceSprite ;
			SSystem::SSmartPointer
					<SakuraGL::SGLSprite>		m_pFaceFadeout ;

			KeyWaitType							m_keyWaitCurrent ;

			const CharConfig *					m_pcfgNextChar ;
			bool								m_flagReadMsg ;

			size_t								m_nLastAddedMsgChars ;
			SSystem::SString					m_strMsgPlainText ;

			bool								m_flagFadeoutMsg ;

		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( StdMessage, UIMessage )
			// 構築
			StdMessage( AGLStdMessageProcessor * pStdMsg ) ;
			// 表示状態取得
			bool IsElementVisible( MessageSpriteElement mseIndex ) const ;
			// 表示状態設定
			void SetElementVisible
				( MessageSpriteElement mseIndex, bool flagVisible ) ;
			// 移動アニメーション制御
			bool MoveElementPosition
				( MessageSpriteElement mseIndex, const Position& pos, uint32_t msecTime ) ;
			bool FadeElementTransparency
				( MessageSpriteElement mseIndex, uint32_t nTransparency, uint32_t msecTime ) ;
			bool IsMovingElementPosition( MessageSpriteElement mseIndex ) const ;
			bool FinishMovingElementPosition( MessageSpriteElement mseIndex ) ;

		public:
			// 表示中のタイマー処理（スクリプト駆動）
			virtual void OnAntirrhinumTimer( void ) ;
			// ウィンドウの表示状態取得
			virtual bool IsShowWindow( void ) ;
			// ウィンドウの表示／非表示状態変更（フェード開始）
			virtual void ShowWindow( bool flagShow, uint32_t msecTime ) ;
			// ウィンドウのフェード中か？
			virtual bool IsPendingShowEffect( void ) ;
			// ウィンドウのフェード処理の即時完了
			virtual void FinishShowWindow( void ) ;
			// キャラクター毎に吹き出しを生成するような場合、吹き出しの準備
			// また文字色などの設定の反映（NULL の場合モノローグなどのデフォルト処理）
			virtual void PrepareMessage( const CharConfig * pcfg, bool flagReadMsg ) ;
			// フェイス画像の表示
			virtual void DisplayFace( const wchar_t * pwszFace ) ;
			// 名前の表示
			virtual void DisplayName( const wchar_t * pwszName ) ;
			// メッセージ出力開始（追加出力）
			virtual void StartMessage( const wchar_t * pwszMsg ) ;
			// メッセージ出力処理中か？
			virtual bool IsPendingMessage( void ) ;
			// 現在のメッセージの AUTO タイムアウト時間計算
			virtual uint32_t GetCurrentAutoTimeout( void ) ;
			// メッセージ出力の即時完了
			virtual void FinishMessage( void ) ;
			// 現在表示されているメッセージのプレーンテキスト取得
			virtual const wchar_t * GetMessagePlainText( void ) ;
			// フェイス画像の消去
			virtual void ClearFace( void ) ;
			// 名前の消去
			virtual void ClearName( void ) ;
			// メッセージの消去（フェードアウト）
			virtual void ClearMessage( size_t nFadeout = 0 ) ;
			// メッセージのフェードアウト中か？
			virtual bool IsPendingClearMessage( void ) ;
			// キー待ちアイコンの表示状態設定
			virtual void ShowKeyWait( KeyWaitType type ) ;
			// 現在のキー待ちアイコンの表示状態設定
			virtual KeyWaitType CurrentKeyWait( void ) ;
			// トグルUI状態取得
			virtual bool IsToggleButton( ToggleButtonIndex tbi ) ;
			// トグルUI状態設定
			virtual void SetToggleButton( ToggleButtonIndex tbi, bool flagPushed ) ;
			// トグルUI禁止状態設定
			virtual void EnableToggleButton( ToggleButtonIndex tbi, bool flagEnabled ) ;
			// 選択肢取得
			virtual UISelector * GetSelector( void ) ;
			// 選択肢解放
			virtual void ReleaseSelector( UISelector * pSelector ) ;
		} ;

		class	UIToggle	: public ESLObject
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( UIToggle, ESLObject )
			// トグルUI状態取得
			virtual bool IsToggle( void ) = 0 ;
			// トグルUI状態設定
			virtual void SetToggle( bool flagPushed ) = 0 ;
			// トグルUI禁止状態設定
			virtual void EnableToggle( bool flagEnabled ) = 0 ;
		} ;

		class	UIToggleSprite	: public UIToggle
		{
		protected:
			SSystem::SReferenceArray
					<SakuraGL::SGLSprite>	m_aSprites ;
			uint32_t						m_nDisabledTransparency ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( UIToggleSprite, UIToggle )
			// 構築
			UIToggleSprite( void ) ;
			UIToggleSprite( SakuraGL::SGLSprite * pSprite ) ;
			// ボタン追加
			void AddSprite( SakuraGL::SGLSprite * pSprite ) ;
			// トグルUI状態取得
			virtual bool IsToggle( void ) ;
			// トグルUI状態設定
			virtual void SetToggle( bool flagPushed ) ;
			// トグルUI禁止状態設定
			virtual void EnableToggle( bool flagEnabled ) ;
			// 禁止状態の表示透明度設定
			void SetDisabledTransparency( uint32_t nTransparency ) ;
		} ;

		class	UIToggleFormButton	: public UIToggle
		{
		protected:
			SSystem::SReferenceArray
				<SakuraGL::SGLBasicForm::Button>	m_aButtons ;
			uint32_t								m_nDisabledTransparency ;
		
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( UIToggleFormButton, UIToggle )
			// 構築
			UIToggleFormButton( void ) ;
			UIToggleFormButton( SakuraGL::SGLBasicForm::Button * pButton ) ;
			// ボタン追加
			void AddButton( SakuraGL::SGLBasicForm::Button * pButton ) ;
			// トグルUI状態取得
			virtual bool IsToggle( void ) ;
			// トグルUI状態設定
			virtual void SetToggle( bool flagPushed ) ;
			// トグルUI禁止状態設定
			virtual void EnableToggle( bool flagEnabled ) ;
			// 禁止状態の表示透明度設定
			void SetDisabledTransparency( uint32_t nTransparency ) ;
		} ;

		class	UIToggleKey	: public UIToggle
		{
		protected:
			SakuraGL::SGLVirtualInput *	m_pInput ;
			size_t						m_iButton ;
			size_t						m_iJoyStick ;
			bool						m_flagEnabled ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( UIToggleKey, UIToggle )
			// 構築
			UIToggleKey( void ) ;
			UIToggleKey( SakuraGL::SGLVirtualInput * pInput,
							size_t iButton, size_t iJoyStick = 0 ) ;
			// キー設定
			void SetVirtualJouButton
				( SakuraGL::SGLVirtualInput * pInput,
							size_t iButton, size_t iJoyStick = 0 ) ;
			// トグルUI状態取得
			virtual bool IsToggle( void ) ;
			// トグルUI状態設定
			virtual void SetToggle( bool flagPushed ) ;
			// トグルUI禁止状態設定
			virtual void EnableToggle( bool flagEnabled ) ;
		} ;

	protected:
		SSystem::SStrSortObjectArray<StdMessage>		m_ssoaMsgWindow ;
		SSystem::SObjectArray<UIToggle>					m_aToggle ;
		SSystem::SObjectArray<UIToggle>					m_aOpKeys ;

		StdSelectorConfig								m_cfgSelector ;
		StdMessageConfig								m_cfgMessage ;
		SSystem::SStrSortObjectArray<StdMessageConfig>	m_ssoaMsgConfig ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( AGLStdMessageProcessor, AGLMessageProcessor )
		// 構築関数
		AGLStdMessageProcessor( void ) ;
		// 消滅関数
		virtual ~AGLStdMessageProcessor( void ) ;

	public:	// UI 実装
		// UIMessage 取得
		virtual UIMessage * GetMessageUI( const wchar_t * pwszID ) ;
		// 操作キー入力
		virtual bool GetPushedKeyState( OperationKey key = keyClickNext ) ;
		virtual void ClearPushedKeyState( OperationKey key = keyClickNext ) ;

	public:	// UI 設定
		// トグルボタン
		void SetToggleButton
			( UIMessage::ToggleButtonIndex tbi, UIToggle * pToggle ) ;
		void SetToggleButtonAsSprite
			( UIMessage::ToggleButtonIndex tbi, SakuraGL::SGLSprite * pSprite ) ;
		void SetToggleButtonAsForm
			( UIMessage::ToggleButtonIndex tbi, SakuraGL::SGLBasicForm::Button * pButton ) ;
		void SetToggleButtonAsKey
			( UIMessage::ToggleButtonIndex tbi,
				SakuraGL::SGLVirtualInput * pInput,
				size_t iButton, size_t iJoyStick = 0 ) ;
		// 操作キー
		void SetOperationKey( OperationKey key, UIToggle * pToggle ) ;
		void SetOperationKey
			( OperationKey key,
				SakuraGL::SGLVirtualInput * pInput,
				size_t iButton, size_t iJoyStick = 0 ) ;

	public:	// メイン・メッセージウィンドウ・ボタン
		// トグルUI状態取得
		virtual bool IsToggleButton( UIMessage::ToggleButtonIndex tbi ) ;
		// トグルUI状態設定
		virtual void SetToggleButton( UIMessage::ToggleButtonIndex tbi, bool flagPushed ) ;
		// トグルUI禁止状態設定
		virtual void EnableToggleButton( UIMessage::ToggleButtonIndex tbi, bool flagEnabled ) ;

	public:
		// メッセージウィンドウ取得／作成
		virtual StdMessage * GetMessageWindow( const wchar_t * pwszID ) ;
		// メッセージウィンドウの削除
		virtual void RemoveMessageWindow( const wchar_t * pwszID ) ;
		virtual void RemoveAllMessageWindows( void ) ;
		// メッセージウィンドウ生成
		virtual StdMessage * NewMessageWindow( const wchar_t * pwszID ) ;
		// 選択肢オブジェクト生成
		virtual UISelector * NewSelector( void ) ;

	public:
		// デフォルトメッセージウィンドウの設定
		void SetDefaultMessageConfig( const StdMessageConfig& cfg ) ;
		// 指定メッセージウィンドウの設定
		void SetMessageConfigAs( const wchar_t * pwszID, const StdMessageConfig& cfg ) ;
		// デフォルト選択肢の設定
		void SetDefaultSelectorConfig( const StdSelectorConfig& cfg ) ;

	} ;

}

#endif
