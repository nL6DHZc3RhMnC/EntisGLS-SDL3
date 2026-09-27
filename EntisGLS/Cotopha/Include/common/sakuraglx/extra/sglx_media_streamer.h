
#if	!defined(__SAKURAGLX_MEDIA_STREAMER_H__)
#define	__SAKURAGLX_MEDIA_STREAMER_H__	1

#include <sakura/ssys_socket.h>
#include <sakura/ssys_smart_buffer.h>
#include <sakuragl/sgl_erisa_lib.h>
#include <sakuragl/sgl_media.h>
#include <sakuragl/media/sgl_sound_recorder.h>
#include <sakuragl/media/sgl_sound_software_mixer.h>

#if	defined(__PLATFORM_WINDOWS__)
#include <sakuragl/sgl_win_screen_capture.h>
#endif

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// ストリーミング・サーバー
	//////////////////////////////////////////////////////////////////////////

	class	SGLMediaStreamingServer
				: public ESLObject, public SSystem::SProcedure
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
			( SGLMediaStreamingServer, ESLObject, SProcedure )
		// 構築関数
		SGLMediaStreamingServer( void ) ;
		// 消滅関数
		virtual ~SGLMediaStreamingServer( void ) ;

	public:
		// 映像ブロックセット (8x8 ピクセル, 2x2 ブロック)
		struct	ImageBlockset444
		{
			int16_t	b[4][64] ;
			int16_t	g[4][64] ;
			int16_t	r[4][64] ;
		} ;
		struct	ImageBlockset411
		{
			int16_t	y[4][64] ;
			int16_t	u[64] ;
			int16_t	v[64] ;
		} ;
		// DCT ジグザグ走査
		static const size_t	m_indexZigzagTable[64] ;

	public:
		// UDP 送信データヘッダ
		struct	UDPDataHeader
		{
			uint32_t	typeData ;
			uint32_t	bytesData ;
			uint32_t	nCRC32 ;
			uint32_t	padAlign ;
			uint64_t	idSerial ;
		} ;
		enum	UDPDataType
		{
			udpDataNull		= 0,
			udpDataSound	= 1,
			udpDataImage	= 2,
			updDataCursor	= 3,
		} ;
		// 画像データヘッダ
		struct	UDPImageHeader
		{
			SGLSize		sizeFrame ;
			uint32_t	nStartBlock ;
			uint32_t	nSubframeBlocks ;
			uint32_t	nFlags ;
			uint8_t		nParamScale ;
			uint8_t		nPadding[3] ;
		} ;
		enum	UDPImageFlag
		{
			udpImageDifferential	= 0x0001,
		} ;
		// カーソルデータヘッダ
		struct	UDPCursorHeader
		{
			SGLPoint	ptCursor ;
			uint32_t	nFlags ;
			uint32_t	idCursor ;
			SGLSize		sizeCursor ;
			SGLPoint	ptHotspot ;
		} ;
		enum	UDPCursorFlag
		{
			udpCursorShow	= 0x0001,
			udpCursorImage	= 0x0002,
		} ;

	protected:
	#if	defined(__PLATFORM_WINDOWS__)
	public:
		class	Instance ;

	protected:
		// 音声ストリーム用スレッド関数
		class	SoundProcedure	: public SSystem::SProcedure
		{
		protected:
			Instance *	m_instance ;

		public:
			// 構築関数
			SoundProcedure( Instance * instance ) ;

		public:	// SProcedure 実装
			// スレッド関数
			virtual void Run( void ) ;
			// 完了後の処理
			virtual void Finalize( void ) ;
		} ;

		// 映像ストリーム用スレッド関数
		class	VideoProcedure	: public SSystem::SProcedure
		{
		protected:
			Instance *	m_instance ;

		public:
			// 構築関数
			VideoProcedure( Instance * instance ) ;

		public:	// SProcedure 実装
			// スレッド関数
			virtual void Run( void ) ;
			// 完了後の処理
			virtual void Finalize( void ) ;
		} ;

	public:
		// 接続インスタンス
		class	Instance
					: public ESLObject, public SSystem::SProcedure
		{
		protected:
			atomic_int_t	m_countRef ;

		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO2( Instance, ESLObject, SProcedure )
			// 構築関数
			Instance( SGLMediaStreamingServer * pmss ) ;
			Instance( const Instance& inst ) ;
			// 消滅関数
			virtual ~Instance( void ) ;
			// 参照カウンタ加算
			void AddRef( void ) ;
			// 参照カウンタ減少・削除
			void ReleaseRef( void ) ;

			// 開始
			SGLError Start( SSystem::SSocket& socket ) ;
			// ランダムトーイン一致判定
			bool IsEqualToken( const wchar_t * pwszToken ) const ;
			// IP 一致判定
			bool IsEqualUDPIP( const uint8_t * pbytIP, size_t nIPBytes ) const ;
			// 強制停止
			void AsyncAbort( void ) ;

		protected:
			SGLMediaStreamingServer *		m_pmss ;
			SSystem::SString				m_strClientIP ;
			SSystem::SString				m_strRandomToken ;
			SSystem::SArray<uint8_t>		m_bufUDPIP ;

			// 送受信ソケット
			SSystem::SAsyncSocket			m_socketTCP ;
			SSystem::SThread				m_threadTCP ;
			bool							m_flagAbort ;
			SSystem::SArray<uint8_t>		m_bufLine ;
			SSystem::STimeCounter			m_timerLastRecv ;

			// 送信レート制御用
			SSystem::STimeCounter			m_timerSendCurrent ;	// 送信レート制御用カウンタ
			uint32_t						m_bytesSendCurrent ;	// 送信されたデータ容量
			uint32_t						m_bytesIFrameSend ;		// 送信されたIフレーム容量
			uint32_t						m_msecIFrameSend ;		// Iフレームの受信に要した時間
			uint32_t						m_bytesSendMinLimitRate ;	// 送信最低速度
			uint32_t						m_bytesBestSendRate ;	// 最高送信速度 [bytes/sec]

			// 送信キュー
			SSystem::SObjectArray
				< SSystem::SArray<uint8_t> >
											m_queSoundData ;
			SSystem::SCriticalSection		m_csSyncSound ;

			SSystem::SObjectArray
				< SSystem::SArray<uint8_t> >
											m_queVideoData ;

			// 音声ストリーム送信用
			bool							m_flagSound ;
			bool							m_flagKeySound ;
			SSystem::SThread				m_threadSound ;
			uint64_t						m_nSoundPacketID ;
			SSystem::STimeCounter			m_timerResendLimit ;
			uint32_t						m_msecResendLimit ;
			SGLSoundRecorder				m_recSound ;
			SGLSoundFormat					m_fmtSound ;
			size_t							m_nSoundInterval ;
			size_t							m_nSoundStreamSamples ;
			SSystem::SQueueBuffer			m_qbufRecSound ;
			ERISA::MIO_INFO_HEADER			m_mioHeader ;
			ERISA::SGLSoundEncoder			m_encSound ;

			// 映像ストリーム送信用
			bool							m_flagImage ;
			bool							m_flagShouldIFrame ;
			SSystem::STimeCounter			m_timerLastFrame ;
			SSystem::STimeCounter			m_timerLastKeyFrame ;
			SSystem::SCriticalSection		m_csSyncFrame ;
			SSystem::SThread				m_threadImage ;

			#if	defined(__PLATFORM_WINDOWS__)
			struct	WindowEntry
			{
				HWND			hwnd ;
				SGLImageRect	rectDisplay ;
			} ;
			SSystem::SArray<WindowEntry>	m_lstQueryWindow ;
			SGLScreenCapture				m_scrnCapture ;
			HWND							m_hwndCapture ;
			SGLImageRect					m_rectCapture ;
			HCURSOR							m_hLastCursor ;
			uint32_t						m_nNoChangeCursor ;
			#endif
			SGLImage						m_imgLastFrame ;
			SGLImage						m_imgCurFrameTemp ;
			uint64_t						m_idFrameID ;
			bool							m_flagIFrame ;

			int								m_nImageScale ;
			int16_t							m_fxDCTParam[4][64] ;
			uint32_t						m_limitMinDiffBlock ;	// 差分ブロックの二乗和がこの値以下なら変化なしとする
			uint32_t						m_limitMaxDiffBlock ;	// 差分ブロックの二乗和がこの値以上なら独立フレームとする
			uint32_t						m_limitMaxDiffThreshold ;	// 差分ブロックの二乗和の最大値がこの値以上なら独立フレームとする
			uint32_t						m_msecFrameInterval ;	// フレームの更新間隔 [ms]
			uint32_t						m_msecRefreshInterval ;	// Iフレーム間隔　[ms]
			uint32_t						m_bytesTargetIFrame ;	// Iフレームのターゲット容量

			uint32_t						m_bytesLastIFrame ;		// 最近のIフレームの容量
			uint8_t							m_nPScaleLastIFrame ;	// 最近のIフレームの圧縮パラメータスケール
			size_t							m_countTryHQIFrame ;	// 画質を落としている状態での IFrame 数

			SSystem::SSmartBuffer			m_sbufVideoTemp ;
			SSystem::SArray<uint8_t>		m_bufVideoTemp ;
			SSystem::SArray<uint8_t>		m_bufVideoDiffBlock ;
			SSystem::SArray<uint8_t>		m_bufVideoReqIFrame ;

			// マウスカーソル送信用
			uint64_t						m_idCursorID ;
			SSystem::STimeCounter			m_timerLastCursor ;

		public:	// SProcedure 実装
			// スレッド関数
			virtual void Run( void ) ;
			// 完了後の処理
			virtual void Finalize( void ) ;

		protected:
			// サウンドストリーム処理
			void OnStreamSound( void ) ;
			// 映像ストリーム処理
			uint32_t OnStreamVideo( void ) ;
			// カーソルデータ処理
			void OnStreamCursor( void ) ;
			// 未送信の映像データを送信
			bool SendVideoStream( void ) ;

		protected:
			// HBITMAP を RGB32 画像へ変換
			static SGLImageBuffer *
						ConvertHBITMAPtoImageBuffer( HBITMAP hBitmap ) ;
			// カーソルのカラー画像にマスク画像を合成
			static void MakeBlendCursorImage
				( SGLImageBuffer * pimgColor, SGLImageBuffer * pimgMask ) ;

		protected:
			// サンプリング
			static void SampleImageBlockset
				( ImageBlockset444& bsRGB,
					const SGLImageBuffer& imgFrame,
					uint32_t xPos, uint32_t yPos ) ;
			// 差分処理
			static uint32_t DifferenceImageBlockset
				( uint32_t& nMaxDiffPow2,
					ImageBlockset444& bsDiff,
					const ImageBlockset444& bsLast ) ;
			static void IDifferenceImageBlockset
				( ImageBlockset444& bsDiff, const ImageBlockset444& bsLast ) ;
			// RGB -> YUV 変換
			static void ConvertBlocksetRGBtoYUV( ImageBlockset444& bsRGB ) ;
			// 4:4:4 -> 4:1:1 変換
			static void ConvertBlockset444to411
				( ImageBlockset411& bs411, const ImageBlockset444& bs444 ) ;
			// DCT 変換・量子化・ジグザグ走査・符号化
			void EncodeBlockset411
				( ERISA::SGLGammaEncodeContext& encoder,
					ImageBlockset411& bs411,
					uint8_t nParamScale, int iDCTParam = 0 ) ;
			// 送信
			void SendVideoSubframe
				( const UDPImageHeader& imgHeader, SSystem::SSmartBuffer& sbuf ) ;

		public:
			// UDP コマンド処理
			void DispatchUDPCommand
				( const SSystem::SXMLDocument& xmlCmd,
					const uint8_t * pbytIP, size_t nIPBytes ) ;
		protected:
			// コマンド処理
			void DispatchCommand( const SSystem::SString& strLine ) ;
			// 音声ストリーミング開始
			bool BeginSound
				( int nRate, int nBufSamples, int nFrequency, int nChannels ) ;
			// 音声ストリーミング終了
			bool EndSound( void ) ;
			// 映像DCT係数設定
			void UpdateDCTParam( int nIndex ) ;
			// 映像ストリーミング開始
			bool BeginVideo( void ) ;
			// 映像ストリーミング終了
			bool EndVideo( void ) ;

		protected:
			// TCP/IP で1行送信
			void SendMessageLine( const wchar_t * pwszMsg ) ;
			// TCP/IP で1行受信
			SGLError RecvMessageLine( SSystem::SString& strLine ) ;

			friend class	SoundProcedure ;
			friend class	VideoProcedure ;
		} ;
		friend class Instance ;
	#endif

	public:
		// UDP 受信スレッド
		class	UDPProcedure
					: public ESLObject, public SSystem::SProcedure
		{
		protected:
			SGLMediaStreamingServer *	m_pmss ;

		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO2( UDPProcedure, ESLObject, SProcedure )
			// 構築関数
			UDPProcedure( SGLMediaStreamingServer * pmss ) ;
			// 消滅関数
			virtual ~UDPProcedure( void ) ;

		public:	// SProcedure 実装
			// スレッド関数
			virtual void Run( void ) ;

		} ;
		friend class UDPProcedure ;

	protected:
		// 設定
		size_t						m_nLimitConnection ;	// 最大接続数
		SSystem::SString			m_strSoundRecDevice ;	// 録音デバイス

		uint32_t					m_nListenTCPPort ;		// TCP/IP ポート番号
		uint32_t					m_nListenUDPPort ;		// UDP/IP ポート番号
		SSystem::SString			m_strLoginPass ;		// パスワード

		size_t						m_msecSoundKeyInterval ;

	protected:
		// 接続
		bool						m_flagListening ;
		SSystem::SSignalEvent		m_eventExit ;

		SSystem::SThread			m_threadListen ;
		SSystem::SSocket			m_socketListen ;

		SSystem::SThread			m_threadUDP ;
		SSystem::SSmartPointer<UDPProcedure>
									m_procUDP ;
		SSystem::SSocket			m_socketUDP ;

		SSystem::SCriticalSection	m_csSync ;
		#if	defined(__PLATFORM_WINDOWS__)
		SSystem::SObjectArray<Instance>
									m_lstInstance ;
		#endif

		// 拒否接続元情報
		struct	DenyConnection
		{
			size_t	nFailedLogin ;
		} ;
		SSystem::SStrSortArray<DenyConnection>	m_ssaDenyClient ;

	public:
		// セットアップ
		SGLError SetupServer
			( uint32_t nTCPPort = 186, uint32_t nUDPPort = 187 ) ;
		// 終了
		SGLError Shutdown( void ) ;
		// 最大接続数設定
		void SetMaxConnection( size_t nLimitConnection ) ;
		// パスワード設定
		void SetPassword( const wchar_t * pwszPassword ) ;
		// 録音デバイス設定
		void SetSoundRecorderDevice( const wchar_t * pwszDevName ) ;

	protected:
		#if	defined(__PLATFORM_WINDOWS__)
		// インスタンス追加
		bool AddInstance( Instance * pInstance ) ;
		// インスタンス削除
		void RemoveInstance( Instance * pInstance ) ;
		#endif
		// 拒否接続元か？
		bool IsDenyClient( const wchar_t * pwszClientAddr ) const ;
		// ログイン結果反映
		void ClientLoginResult
			( const wchar_t * pwszClientAddr, bool fSuccessed ) ;

	public:	// SProcedure 実装
		// スレッド関数
		virtual void Run( void ) ;

	protected:
		// UDP データヘッダ
		struct	RecvUDPHeader
		{
			uint32_t	nBodyBytes ;
			uint32_t	nBodyCRC32 ;
		} ;
		// UDP 受信データ処理
		void DispatchUDPCommand
			( const uint8_t * pbytData, size_t nDataBytes,
				const uint8_t * pbytIP, size_t nIPBytes ) ;
		#if	defined(__PLATFORM_WINDOWS__)
		// IP が一致する接続取得
		Instance * GetInstanceUDPIPOf
			( const uint8_t * pbytIP, size_t nIPBytes ) const ;
		#endif
		// UDP 受信データ検証
		static bool VerifyUDPData
			( const uint8_t * pbytData, size_t nDataBytes ) ;

	public:
		// UDP データ送信
		size_t SendUDPData
			( const void * ptrBuf, size_t nBytes,
				void * ptrAddrTo, size_t nAddrBytes ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// ストリーミング・クライアント
	//////////////////////////////////////////////////////////////////////////

	class	SGLMediaStreamingClient
				: public ESLObject, public SSystem::SProcedure
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2( SGLMediaStreamingClient, ESLObject, SProcedure )
		// 構築関数
		SGLMediaStreamingClient( void ) ;
		// 消滅関数
		virtual ~SGLMediaStreamingClient( void ) ;

	public:
		// カーソルデータヘッダ
		typedef	SGLMediaStreamingServer::UDPCursorHeader	CursorDataHeader ;

		enum	UDPCursorFlag
		{
			udpCursorShow	= SGLMediaStreamingServer::udpCursorShow,
			udpCursorImage	= SGLMediaStreamingServer::udpCursorImage,
		} ;

		// リスナ
		class	Listener	: public ESLObject
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( Listener, ESLObject ) ;
			// サウンド出力準備
			virtual void PrepareSoundStream( const SGLSoundFormat& fmt ) ;
			// サウンドストリーム
			virtual void OnSoundStream
				( const void * ptrWavePCM, size_t nBytes, bool fHeader ) ;
			// 映像ストリーム
			virtual void OnVideoUpdate( SGLImage& imgFrame ) ;
			// マウスカーソル
			virtual void OnMouseCursor
				( const CursorDataHeader& hdrCursor, SGLImage * pimgCursor ) ;
			// 切断した
			virtual void OnDisconnected( void ) ;
		} ;

	protected:
		// 映像デコードスレッド
		class	VideoDecoderThread	: public SSystem::SProcedure
		{
		public:
			SSystem::SThread			m_thread ;
			SGLMediaStreamingClient *	m_pmsc ;
			bool						m_flagPrimary ;
			bool						m_flagExit ;

		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( VideoDecoderThread, SProcedure )
			// 構築関数
			VideoDecoderThread
				( SGLMediaStreamingClient * pmsc, bool flagPrimary ) ;
			// 消滅関数
			virtual ~VideoDecoderThread( void ) ;

		public:
			// スレッド関数
			virtual void Run( void ) ;
		} ;

		// UDP 受信データヘッダ
		typedef	SGLMediaStreamingServer::UDPDataHeader	RecvUDPHeader ;

		// 受信データ
		class	UDPRecvData	: public SSystem::SArray<uint8_t>
		{
		public:
			RecvUDPHeader * GetHeader( void ) const
			{
				return	(RecvUDPHeader*) GetArray() ;
			}
		} ;
		// 送信データ
		typedef	UDPRecvData	UDPSendData ;

		friend class VideoDecoderThread ;

	protected:
		SSystem::SAsyncSocket	m_socketTCP ;
		SSystem::SSocket		m_socketUDP ;
		SSystem::SThread		m_thread ;
		SSystem::SSignalEvent	m_eventExit ;
		bool					m_flagConnectTCP ;
		bool					m_flagConnectUDP ;
		bool					m_flagThread ;

		bool					m_initDecSound ;
		bool					m_flagKeySound ;
		bool					m_flagSoundFirst ;
		uint64_t				m_idSoundPacketID ;
		ERISA::MIO_INFO_HEADER	m_mioHeader ;
		ERISA::SGLSoundDecoder	m_decSound ;

		int16_t					m_fxDCTParam[4][64] ;
		int						m_fxIDCTParam[4][64] ;

		uint64_t					m_idVideoPacketID ;
		bool						m_flagVideoIFrame ;
		bool						m_flagPostViewUpdate ;
		SGLImage					m_imgLastFrame ;
		SSystem::SArray<uint8_t>	m_bufBlockDiffCounter ;
		uint32_t					m_bytesLastIFrame ;
		uint32_t					m_msecLastIFrameDuration ;
		SSystem::STimeCounter		m_timerIFrameRecv ;

		bool						m_flagBlockReqIFrame ;	// 再送信を要求するブロックが存在するか？
		SSystem::SArray<uint8_t>	m_bufBlockReqArray ;	// 再送信を要求するブロックのフラグマップ

		SSystem::SObjectArray<VideoDecoderThread>
									m_lstVideoThreads ;

		atomic_int_t				m_countDecodingVideo ;
		SSystem::SSignalEvent		m_signFreeDecodingVideo ;

		SSystem::SCriticalSection	m_csSync ;
		SSystem::SSignalEvent		m_signRecvVideoFrames ;
		SSystem::SObjectArray<UDPRecvData>
									m_queRecvVideoFrames ;

		SSystem::SObjectArray<UDPSendData>
									m_queDelaySendUDP ;

		uint64_t					m_idCursorPacketID ;
		SGLImage					m_imgLastCursor ;

		Listener *				m_pListener ;

		SSystem::STimeCounter		m_timerTrace ;

	public:
		// リスナ設定
		void AttachListener( Listener * pListener ) ;
		// 接続
		SGLError Connect
			( const wchar_t * pwszHostAddr,
				const wchar_t * pwszPassword,
				uint32_t nTCPPort = 186, uint32_t nUDPPort = 187 ) ;
		// 切断
		SGLError Close( void ) ;
		// サウンドストリーミング開始
		SGLError BeginSound
			( int nRate, int nBufSamples, int nFrequency, int nChannels ) ;
		// サウンドストリーミング停止
		SGLError EndSound( void ) ;
		// 映像ストリーミング開始
		SGLError BeginVideo( void ) ;
		// 映像ストリーミング終了
		SGLError EndVideo( void ) ;
		// 映像DCT係数設定
		SGLError UpdateDCTParam( int nIndex, const int * pParamDCT ) ;
		// 映像パラメータ送信
		SGLError SendVideoParam
			( int msecKeyFrame, int msecFrame = -1,
				int minDiffBlock = -1,
				int maxDiffBlock = -1, int thresholdDiffBlock = -1 ) ;
		// 映像スケール設定
		SGLError SetVideoScale( int nScale = 0 ) ;
		// ウィンドウリスト取得
		SGLError QueryWindowList
			( SSystem::SObjectArray<SSystem::SString>& lstWindowName ) ;
		// ウィンドウ選択
		SGLError SelectWindow( int iWindow = -1 ) ;
		// マウス座標送信
		void PostMousePoint( int xPos, int yPos, bool fDelta ) ;
		// キーイベント送信
		void PostKeyboardEvent
			( int nVirtKey, int nFlags, bool fRelease ) ;
		// マウス操作イベント
		enum	MouseEventCode
		{
			mouseLeftDown,
			mouseLeftUp,
			mouseRightDown,
			mouseRightUp,
			mouseMiddleDown,
			mouseMiddleUp,
			mouseWheel,
			mouseXButton1Down,
			mouseXButton1Up,
			mouseXButton2Down,
			mouseXButton2Up,
		} ;
		// マウスイベント送信
		void PostMouseEvent( int code, int nDelta ) ;
		// TCP 送信待ち行列データバイト数取得
		size_t GetPendingPostQueueBytes( void ) const ;

	public:	// SProcedure 実装
		// スレッド関数
		virtual void Run( void ) ;

	protected:
		// 映像フレームデータ追加
		void AddVideoFrameQueue( RecvUDPHeader * pHeader ) ;

	protected:
		enum	ResponseCode
		{
			responseTimeout	= -1,
			responseInvalid	= -2,
			responseSuccess	= 100,
			responseError	= 900,
		} ;
		// TCP レスポンス受信
		int RecvTCPResponse
			( SSystem::SString& strMsg, uint32_t msecTimeout = 10000 ) ;
		// TCP レスポンスをエラーコードに変換
		static SGLError ResponseToError( int nResCode ) ;
		// TCP １行受信
		SGLError RecvResponseLine
			( SSystem::SString& strLine, uint32_t msecTimeout = 10000 ) ;

	protected:
		// UDP データヘッダ
		struct	SendUDPHeader
		{
			uint32_t	nBodyBytes ;
			uint32_t	nBodyCRC32 ;
		} ;
		// UDP コマンド送信
		SGLError SendUDPCommand( SSystem::SXMLDocument& xmlCmd ) ;
		// UDP コマンド生成
		void MakeUDPCommand
			( SSystem::SArray<uint8_t>& bufData,
						SSystem::SXMLDocument& xmlCmd ) ;

		// UDP 受信データ検証
		static bool VerifyUDPData
			( const uint8_t * pbytData, size_t nDataBytes ) ;

		// 音声ストリームデータ処理
		void OnStreamSound( RecvUDPHeader * pHeader ) ;

		// 映像ブロックセット (8x8 ピクセル, 2x2 ブロック)
		typedef	SGLMediaStreamingServer::ImageBlockset444	ImageBlockset444 ;
		typedef	SGLMediaStreamingServer::ImageBlockset411	ImageBlockset411 ;

		// 映像ストリームデータ処理
		void OnStreamVideo( RecvUDPHeader * pHeader, bool flagPrimary ) ;
		// Iフレーム受信容量送信
		void SendIFrameBytes( void ) ;
		// Iフレーム再送信要求設定
		void MakeRequestIFrameEncoded( void ) ;
		// 復号・逆量子化・ジグザグ走査・DCT 変換
		void DecodeBlockset411
			( ERISA::SGLGammaDecodeContext& decoder,
				ImageBlockset411& bs411,
				uint8_t nParamScale, int iDCTParam = 0 ) ;
		// 4:1:1 -> 4:4:4 変換
		static void ConvertBlockset411to444
			( ImageBlockset444& bs444, const ImageBlockset411& bs411 ) ;
		// YUV -> RGB 変換
		static void ConvertBlocksetYUVtoRGB( ImageBlockset444& bsYUV ) ;
		// 画像復帰
		static void RestoreIFrameBlockset
			( const SGLImageBuffer& imgFrame,
				ImageBlockset444& bsRGB, uint32_t xPos, uint32_t yPos ) ;
		static void RestorePFrameBlockset
			( const SGLImageBuffer& imgFrame,
				ImageBlockset444& bsRGB, uint32_t xPos, uint32_t yPos ) ;

		// カーソルデータ処理
		void OnMouseCursor( RecvUDPHeader * pHeader ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// ウィンドウ同期動画キャプチャー
	//////////////////////////////////////////////////////////////////////////

	class	SGLSyncWindowCapture
					: public ESLObject, public SSystem::SProcedure
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2( SGLSyncWindowCapture, ESLObject, SProcedure )
		// 構築関数
		SGLSyncWindowCapture( void ) ;
		// 消滅関数
		virtual ~SGLSyncWindowCapture( void ) ;

	protected:
		SGLWindowSprite *		m_pWindow ;
		SGLVideoOutputStream *	m_pVideoOut ;
		SGLAudioOutputStream *	m_pAudioOut ;

		bool					m_flagCapturing ;
		bool					m_flagExitCapture ;
		uint32_t				m_nFlags ; 
		size_t					m_nMilliSecPerFrame ;
		SGLSize					m_sizeVideo ;
		SGLImageInfo			m_infVideo ;
		SGLSoundFormat			m_fmtMixOut ;
		SGLSoundSoftwareMixer	m_mixSound ;
		SSystem::SThread		m_thread ;

	public:
		// ターゲットウィンドウ設定
		void AttachTargetWindow( SGLWindowSprite * pWindow ) ;
		// 出力先設定
		void AttachOutputStream
			( SGLVideoOutputStream * pVideo, SGLAudioOutputStream * pAudio ) ;
		// キャプチャ動作フラグ
		enum	CaptureFlag
		{
			captureNoWait	= 0x0001,
			captureRealTime	= 0x0002,
		} ;
		// キャプチャー開始
		SGLError BeginCapture
			( uint32_t nFlags,
				size_t nMilliSecPerFrame,
				const SGLImageInfo& infVideo,
				const SGLSoundFormat& fmtSoundOut ) ;
		// キャプチャー終了
		SGLError EndCapture( void ) ;

	public:	// SProcedure 実装
		// スレッド関数
		virtual void Run( void ) ;

	protected:
		// ミキサ出力サウンドプレイヤー生成関数
		static SGLSoundPlayerInterface * new_SoundPlayer( void * pInstance ) ;

	} ;

}

#endif

