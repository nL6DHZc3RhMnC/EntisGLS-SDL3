
#if	!defined(__SAKURAGL_MEDIA_DSHOW_MEDIA_PLAYER_H__)
#define	__SAKURAGL_MEDIA_DSHOW_MEDIA_PLAYER_H__	1

#include <strmif.h>
#include <amvideo.h>
#include <control.h>

#include <sakura/ssys_win_registry.h>
#include <sakuragl/sgl_win_avi_composer.h>
#include <sakuragl/sgl_dshow_audio_player.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// DirectShow メディアファイル再生インターフェース（直接表示）
	//////////////////////////////////////////////////////////////////////////

	class	SGLDirectShowMediaPlayer
				: public SGLMediaPlayerInterface,
					public SGLDirectShowAudioPlayer
	{
	protected:
		struct IBasicVideo *	m_pBasicVideo ;

		SGLAbstractWindow *		m_pWindow ;
		SGLImageRect			m_rectDstView ;

		SSystem::SString		m_strFilePath ;
		SSystem::SEnvironmentInterface *	m_pEnv ;

		int64_t					m_msecLastRepos ;

		SGLMediaPlayerFrameNotification *	m_pListener ;

		SSystem::SSmartPointer<SGLWindowsAVIReader>
								m_pAVIReader ;

		// ウィンドウ UI スレッド関数呼び出し
		typedef	SGLError (SGLDirectShowMediaPlayer::*PTR_FUNCTION)( void ) ;
		class	UISyncProcedure	: public SSystem::SProcedure
		{
		protected:
			SSystem::SSignalEvent		m_eventDone ;
			SGLDirectShowMediaPlayer *	m_pPlayer ;
			PTR_FUNCTION				m_pfnCall ;
			SSystem::SMutex *			m_pMutexUI ;
		public:
			SGLError					m_errResult ;
		public:
			UISyncProcedure
				( SGLDirectShowMediaPlayer * pPlayer,
					PTR_FUNCTION pfnCall, SSystem::SMutex * pMutex )
					: m_pPlayer( pPlayer ),
						m_pfnCall( pfnCall ), m_pMutexUI( pMutex )
			{
				m_eventDone.Initialize( false ) ;
			}
			SSystem::SError Wait( void ) ;
			virtual void Run( void ) ;
			virtual void Finalize( void ) ;
		} ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
			( SGLDirectShowMediaPlayer,
				SGLMediaPlayerInterface, SGLDirectShowAudioPlayer )
		// 構築関数
		SGLDirectShowMediaPlayer( void ) ;
		// 消滅関数
		virtual ~SGLDirectShowMediaPlayer( void ) ;

	protected:
		// ファイルを開く（UIスレッド専用）
		SGLError OpenOnUIThread( void ) ;
		// 再生開始関数（UIスレッド専用）
		SGLError PlayOnUIThread( void ) ;
		// 表示位置設定関数（UIスレッド専用）
		SGLError SetViewOnUIThread( void ) ;

		// SGLWindowsAVIReader 取得
		SGLWindowsAVIReader * GetAVIMediaStream( void ) ;

	public:
		// 指定ファイルを開く
		virtual SGLError Open
			( const wchar_t * pwszFilePath, uint64_t nFlags = 0,
					SSystem::SEnvironmentInterface * pEnv = NULL ) ;
		virtual SGLError Create
			( SSystem::SFileInterface * file,
				bool flagOwner = true, uint64_t nFlags = 0 ) ;
		// データを参照する複製プレイヤー生成
		virtual SGLAudioPlayerInterface * ClonePlayer( void ) ;
		// ファイルを閉じる
		virtual SGLError Close( void ) ;
		// 再生を開始する
		virtual SGLError Play( uint64_t nFlags = 0 ) ;
		// 再生を停止する
		virtual SGLError Stop( void ) ;
		// ループポイント[/sample] を設定する
		virtual SGLError SetLoop
			( bool fLoop = true, int64_t nStart = -1, int64_t nEnd = -1 ) ;
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
		// メディアのサンプル周波数を取得する
		virtual uint32_t GetSampleFrequency( void ) const ;
		// メディアの全長 [/sample] を取得する
		virtual uint64_t GetTotalLength( void ) const ;
		// 再生位置 [/sample] を取得する
		virtual uint64_t GetPosition( void ) ;
		// 再生位置 [/sample] を変更する
		virtual void SeekPosition( uint64_t nPos ) ;
		// オーディオストリーム取得
		virtual SGLAudioInputStream * GetAudioStream( void ) ;
		virtual void ReleaseAudioStream( SGLAudioInputStream * pStream ) ;
		// スレッド同期用ミューテックス設定
		virtual void SetUIThreadMutex( SSystem::SMutex * pMutex ) ;

	public:
		// ビデオサイズを取得する
		virtual SGLError GetVideoSize( SGLSize& sizeVideo ) ;
		// 表示先を設定する
		virtual SGLError SetVideoView
			( SGLAbstractWindow* pWindow,
				const SGLImageRect& rectVideo, uint64_t nFlags = 0 ) ;
		// 現在のフレームを描画する
		virtual SGLError DrawVideo
			( SGLPaintContextInterface* pPaint,
				const SGLImageRect& rectDst,
				uint32_t nFlags = 0, uint32_t nTransparency = 0 ) ;
		// メディア再生通知リスナ設定
		virtual SGLMediaPlayerFrameNotification *
			SetNotificationListener
				( SGLMediaPlayerFrameNotification * pListener ) ;
		// ビデオストリーム取得
		virtual SGLVideoInputStream * GetVideoStream( void ) ;
		virtual void ReleaseVideoStream( SGLVideoInputStream * pStream ) ;

	public:	// SProcedure 実装
		// スレッド関数
		virtual void Run( void ) ;
		// ループ処理実装
		virtual bool OnLoopingThread( void ) ;
		// 終端到達
		virtual void NotifyEndOfPlayingDuration( void ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// DirectShow ファイル入力フィルタ
	//////////////////////////////////////////////////////////////////////////

	class	SGLDSFileSource
				: public IBaseFilter,
					public IAMAsyncReaderTimestampScaling,
					public IAsyncReader
	{
	public:
		// 出力ピン
		class	FileOutPin : public IPin
		{
		protected:
			atomic_int_t		m_nRef ;

			SSystem::SString	m_strName ;
			SSystem::SObjectArray<AM_MEDIA_TYPE>
								m_lstStreamTypes ;
			AM_MEDIA_TYPE		m_mtNull ;

			SGLDSFileSource *	m_pFilter ;
			bool				m_fQueryAsyncReader ;
			IPin *				m_pConnected ;
			bool				m_fConnectMedia ;
			AM_MEDIA_TYPE		m_mtConnectMedia ;

			friend class SGLDSFileSource ;

		public:
			// 構築関数
			FileOutPin
				( const wchar_t * pwszName, SGLDSFileSource * pFilter ) ;
			// 消滅関数
			virtual ~FileOutPin( void ) ;

		public:
			// 名前
			const SSystem::SString& GetName( void ) const
			{
				return	m_strName ;
			}
			// 接続状態
			bool IsConnected( void ) const
			{
				return	(m_pConnected != NULL) ;
			}
			// 停止判定
			bool IsStopped( void ) const
			{
				return	(m_pFilter->m_state == State_Stopped) ;
			}
			// 接続試行
			virtual HRESULT AttemptConnection
				( IPin * pReceivePin, const AM_MEDIA_TYPE * pmt ) ;
			virtual HRESULT TryMediaTypes
				( IPin * pReceivePin,
					__in_opt const AM_MEDIA_TYPE * pmt, IEnumMediaTypes *pEnum ) ;
			// メディアタイプ取得
			const AM_MEDIA_TYPE * GetMediaTypeAt( size_t i ) const ;
			size_t GetMediaTypeCount( void ) const ;
			// メディアタイプ設定
			HRESULT SetMediaType( const AM_MEDIA_TYPE * pmt ) ;

		public:
			// IUnknown
			virtual HRESULT STDMETHODCALLTYPE QueryInterface( REFIID riid, void ** ppObj ) ;
			virtual ULONG STDMETHODCALLTYPE AddRef( void ) ;
			virtual ULONG STDMETHODCALLTYPE Release( void ) ;

			// IPin
			virtual STDMETHODIMP Connect
				( IPin * pReceivePin, __in_opt const AM_MEDIA_TYPE * pmt ) ;
			virtual STDMETHODIMP ReceiveConnection
				( IPin * pConnector, const AM_MEDIA_TYPE * pmt ) ;
			virtual STDMETHODIMP Disconnect( void ) ;
			virtual STDMETHODIMP ConnectedTo( __deref_out IPin ** ppPin ) ;
			virtual STDMETHODIMP ConnectionMediaType( __out AM_MEDIA_TYPE * pmt ) ;
			virtual STDMETHODIMP QueryPinInfo( __out PIN_INFO * pInfo ) ;
			virtual STDMETHODIMP QueryDirection( __out PIN_DIRECTION * pPinDir ) ;
			virtual STDMETHODIMP QueryId( __deref_out LPWSTR * Id ) ;
			virtual STDMETHODIMP QueryAccept( const AM_MEDIA_TYPE * pmt ) ;
			virtual STDMETHODIMP EnumMediaTypes
					( __deref_out IEnumMediaTypes ** ppEnum ) ;
			virtual STDMETHODIMP QueryInternalConnections
				( __out_ecount_part(*nPin,*nPin) IPin* *apPin, __inout ULONG *nPin ) ;
			virtual STDMETHODIMP EndOfStream( void ) ;
		    virtual STDMETHODIMP BeginFlush( void ) ;
			virtual STDMETHODIMP EndFlush( void ) ;
			virtual STDMETHODIMP NewSegment
				( REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate ) ;
		} ;

		// メディアタイプ列挙
		class	EnumMediaTypes	: public IEnumMediaTypes
		{
		protected:
			atomic_int_t	m_nRef ;
			FileOutPin *	m_pPin ;
			size_t			m_iPos ;

		public:
			// 構築関数
			EnumMediaTypes( FileOutPin * pPin ) ;

		public:
			// IUnknown
			virtual HRESULT STDMETHODCALLTYPE QueryInterface( REFIID riid, void ** ppObj ) ;
			virtual ULONG STDMETHODCALLTYPE AddRef( void ) ;
			virtual ULONG STDMETHODCALLTYPE Release( void ) ;

			// IEnumMediaTypes
			virtual HRESULT STDMETHODCALLTYPE Next
				( ULONG cMediaTypes,
					__out_ecount(cMediaTypes) AM_MEDIA_TYPE ** ppMediaTypes,
					__out_opt ULONG * pcFetched ) ;
			virtual HRESULT STDMETHODCALLTYPE Skip( ULONG cMediaTypes ) ;
			virtual HRESULT STDMETHODCALLTYPE Reset( void ) ;
			virtual HRESULT STDMETHODCALLTYPE Clone
				( __deref_out IEnumMediaTypes **ppEnum ) ;
		} ;

		// Pin 列挙
		class	EnumOutPins	: public IEnumPins
		{
		protected:
			atomic_int_t		m_nRef ;
			SGLDSFileSource *	m_pFilter ;
			size_t				m_iPos ;
		public:
			// 構築関数
			EnumOutPins( SGLDSFileSource * pFilter ) ;
			// 消滅関数
			~EnumOutPins( void ) ;
		public:
			// IUnknown
			virtual HRESULT STDMETHODCALLTYPE QueryInterface( REFIID riid, void ** ppObj ) ;
			virtual ULONG STDMETHODCALLTYPE AddRef( void ) ;
			virtual ULONG STDMETHODCALLTYPE Release( void ) ;

			// IEnumPins
			virtual HRESULT STDMETHODCALLTYPE Next
				( ULONG cPins,
					__out_ecount_part(cPins, *pcFetched) IPin ** ppPins,
					__out_opt ULONG * pcFetched ) ;
			virtual HRESULT STDMETHODCALLTYPE Skip( ULONG cPins ) ;
			virtual HRESULT STDMETHODCALLTYPE Reset( void ) ;
			virtual HRESULT STDMETHODCALLTYPE Clone( __out IEnumPins ** ppEnum ) ;
		} ;

	protected:
		atomic_int_t		m_nRef ;
		SSystem::SString	m_strName ;
		FILTER_STATE		m_state ;
		BOOL				m_fTimestampRaw  ;
		FileOutPin *		m_pPin ;
		IReferenceClock *	m_pClock ;
		IFilterGraph *		m_pGraph ;

		SSystem::SSignalEvent		m_sigFlushing ;
		SSystem::SSignalEvent		m_sigReqQueue ;
		SSystem::SSignalEvent		m_sigDoneQueue ;
		SSystem::SCriticalSection	m_csObj ;
		SSystem::SCriticalSection	m_csSync ;
		SSystem::SCriticalSection	m_csQueue ;
		SSystem::SCriticalSection	m_csFile ;

		struct	RequestEntry
		{
			IMediaSample *	pSample ;
			DWORD_PTR		dwUser ;
			HRESULT			hrRead ;
		} ;
		SSystem::SObjectArray<RequestEntry>	m_queRequests ;
		SSystem::SObjectArray<RequestEntry>	m_queDones ;

		class	AsyncReaderProc	: public SSystem::SProcedure
		{
		protected:
			SGLDSFileSource *	m_pFilter ;
		public:
			void AttachFilter( SGLDSFileSource * pFilter )
			{
				m_pFilter = pFilter ;
			}
			virtual void Run( void )
			{
				if ( m_pFilter != NULL )
				{
					m_pFilter->ThreadAsyncReaderProc() ;
				}
			}
		} ;
		friend class AsyncReaderProc ;

		SSystem::SThread			m_threadAsync ;
		AsyncReaderProc				m_procAsync ;
		SSystem::SSignalEvent		m_sigExitThread ;

		SSystem::SFileInterface *	m_pFile ;
		bool						m_fOwnFile ;

		static const GUID	CLSID_SGLDSFileSource ;
		static const GUID	IID_IAMAsyncReaderTimestampScaling ;

	public:
		// 構築関数
		SGLDSFileSource
			( const wchar_t * pwszName,
				SSystem::SFileInterface * pFile, bool flagOwnFile ) ;
		// 消滅関数
		virtual ~SGLDSFileSource( void ) ;

	protected:
		// メディアタイプをレジストリから検索する
		bool SearchMatchMediaType
			( const SSystem::SArray<uint8_t>& bufFileHeader ) ;
		bool IsMatchStreamMediaType
			( SSystem::SRegistryKey& keySubType,
				const SSystem::SArray<uint8_t>& bufFileHeader ) ;

		// ストリームタイプ
		class	StreamBinaryFormat
		{
		public:
			size_t						m_nOffset ;
			SSystem::SArray<uint8_t>	m_bufMask ;
			SSystem::SArray<uint8_t>	m_bufValue ;
		public:
			// フォーマット解釈
			bool ParseFormat( SSystem::SStringParser& sparsForm ) ;
			bool ParseBinary
				( SSystem::SStringParser& sparsForm,
					SSystem::SArray<uint8_t>& bufBinary, size_t nLength ) ;
			// マッチング
			bool IsMatch( const SSystem::SArray<uint8_t>& bufStream ) ;
		} ;

	public:
		// IAMAsyncReaderTimestampScaling 取得
		IAMAsyncReaderTimestampScaling * GetTimestampScaling( void ) ;
		// IAsyncReader 取得
		IAsyncReader * GetAsyncReader( void ) ;
		// IPin 取得
		FileOutPin * GetOutputPin( void ) ;

	protected:
		// 非同期読み込み
		void ThreadAsyncReaderProc( void ) ;

	public:
		// IUnknown
		virtual HRESULT STDMETHODCALLTYPE QueryInterface( REFIID riid, void ** ppObj ) ;
		virtual ULONG STDMETHODCALLTYPE AddRef( void ) ;
		virtual ULONG STDMETHODCALLTYPE Release( void ) ;

		// IPersist
		virtual STDMETHODIMP GetClassID( __out CLSID *pClsID ) ;

		// IMediaFilter
		virtual STDMETHODIMP GetState( DWORD dwMSecs, __out FILTER_STATE * pState ) ;
		virtual STDMETHODIMP SetSyncSource( __in_opt IReferenceClock *pClock ) ;
		virtual STDMETHODIMP GetSyncSource( __deref_out_opt IReferenceClock **pClock ) ;
		virtual STDMETHODIMP Stop( void ) ;
		virtual STDMETHODIMP Pause( void ) ;
		virtual STDMETHODIMP Run( REFERENCE_TIME tStart ) ;

		// IBaseFilter
		virtual STDMETHODIMP EnumPins( __deref_out IEnumPins ** ppEnum ) ;
		virtual STDMETHODIMP FindPin( LPCWSTR Id, __deref_out IPin ** ppPin ) ;
		virtual STDMETHODIMP QueryFilterInfo( __out FILTER_INFO * pInfo ) ;
		virtual STDMETHODIMP JoinFilterGraph
			( __inout_opt IFilterGraph * pGraph, __in_opt LPCWSTR pName ) ;
		virtual STDMETHODIMP QueryVendorInfo( __deref_out LPWSTR* pVendorInfo ) ;

		// IAMAsyncReaderTimestampScaling
		virtual HRESULT STDMETHODCALLTYPE
					GetTimestampMode( __out BOOL *pfRaw ) ;
		virtual HRESULT STDMETHODCALLTYPE
					SetTimestampMode( BOOL fRaw ) ;

		// IAsyncReader
		virtual HRESULT STDMETHODCALLTYPE RequestAllocator
			( IMemAllocator * pPreferred,
				__in ALLOCATOR_PROPERTIES * pProps,
				__out IMemAllocator ** ppActual ) ;
		virtual HRESULT STDMETHODCALLTYPE Request
			( IMediaSample * pSample, DWORD_PTR dwUser ) ;
		virtual HRESULT STDMETHODCALLTYPE WaitForNext
			( DWORD dwTimeout,
				__out_opt IMediaSample ** ppSample,
				__out DWORD_PTR * pdwUser ) ;
		virtual HRESULT STDMETHODCALLTYPE SyncReadAligned
			( IMediaSample * pSample) ;
		virtual HRESULT STDMETHODCALLTYPE SyncRead
			( LONGLONG llPosition,
				LONG lLength, __out_bcount(lLength) BYTE * pBuffer ) ;
		virtual HRESULT STDMETHODCALLTYPE Length
			( __out LONGLONG * pTotal, __out LONGLONG * pAvailable ) ;
		virtual HRESULT STDMETHODCALLTYPE BeginFlush( void ) ;
		virtual HRESULT STDMETHODCALLTYPE EndFlush( void ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// DirectShow メディアファイル再生インターフェース（内部描画）
	//////////////////////////////////////////////////////////////////////////

	class	SGLDSRenderMediaPlayer : public SGLMediaPlayerInterface
	{
	protected:
		static const GUID	CLSID_DSRenderMediaPlayer_RenderFilter ;
		static HMODULE	m_hLibOleAut32 ;

		class	Filter ;

		// 位置インターフェース
		class	PosPassThru	: public IMediaSeeking, public IMediaPosition
		{
		protected:
			atomic_int_t	m_nRef ;
			IUnknown *		m_pOwner ;
			ITypeInfo *		m_pti;
		    IPin *			m_pPin ;

			LONGLONG		m_nStartMedia ;
			LONGLONG		m_nEndMedia ;
			bool			m_fMediaTime ;

			SSystem::SCriticalSection	m_csSync ;

			typedef HRESULT (STDAPICALLTYPE *API_LoadTypeLib)
				( const OLECHAR FAR *szFile,
					__deref_out ITypeLib FAR* FAR* pptlib ) ;
			typedef HRESULT (STDAPICALLTYPE *API_LoadRegTypeLib)
				( REFGUID rguid, WORD wVerMajor, WORD wVerMinor,
					LCID lcid, __deref_out ITypeLib FAR* FAR* pptlib ) ;

		public:
			// 構築関数
			PosPassThru( IUnknown * pOwner, IPin * pPin ) ;
			// 消滅関数
			~PosPassThru( void ) ;

		protected:
			// Dispatch 補助関数
			HRESULT GetTypeInfo
				( REFIID riid, UINT itinfo,
					LCID lcid, __deref_out ITypeInfo ** pptinfo ) ;
			// 接続先 IMediaPosition 取得
			IMediaPosition * GetPeerPosition( void ) ;
			// 接続先 IMediaSeeking 取得
			IMediaSeeking * GetPeerSeeking( void ) ;
			// メディア時間設定
			HRESULT RegisterMediaTime( IMediaSample * pms ) ;
			HRESULT RegisterMediaTime( LONGLONG nStart, LONGLONG nEnd ) ;
			// メディア時間取得
			HRESULT GetMediaTime
				( __out LONGLONG *pStartTime, __out_opt LONGLONG *pEndTime ) ;
			// メディア時間リセット
			void ResetMediaTime( void ) ;
			// ストリーム終端
			void EndOfStream( void ) ;

		public:
			// IUnknown
			virtual HRESULT STDMETHODCALLTYPE QueryInterface( REFIID riid, void ** ppObj ) ;
			virtual ULONG STDMETHODCALLTYPE AddRef( void ) ;
			virtual ULONG STDMETHODCALLTYPE Release( void ) ;

			// IMediaSeeking
			virtual STDMETHODIMP GetCapabilities( __out DWORD * pCapabilities );
			virtual STDMETHODIMP CheckCapabilities( __inout DWORD * pCapabilities );
			virtual STDMETHODIMP SetTimeFormat(const GUID * pFormat);
			virtual STDMETHODIMP GetTimeFormat(__out GUID *pFormat);
			virtual STDMETHODIMP IsUsingTimeFormat(const GUID * pFormat);
			virtual STDMETHODIMP IsFormatSupported( const GUID * pFormat);
			virtual STDMETHODIMP QueryPreferredFormat( __out GUID *pFormat);
			virtual STDMETHODIMP ConvertTimeFormat(__out LONGLONG * pTarget, 
										   __in_opt const GUID * pTargetFormat,
										   LONGLONG Source, 
										   __in_opt const GUID * pSourceFormat );
			virtual STDMETHODIMP SetPositions( __inout_opt LONGLONG * pCurrent, DWORD CurrentFlags
									 , __inout_opt LONGLONG * pStop, DWORD StopFlags );

			virtual STDMETHODIMP GetPositions( __out_opt LONGLONG * pCurrent, __out_opt LONGLONG * pStop );
			virtual STDMETHODIMP GetCurrentPosition( __out LONGLONG * pCurrent );
			virtual STDMETHODIMP GetStopPosition( __out LONGLONG * pStop );
			virtual STDMETHODIMP SetRate( double dRate);
			virtual STDMETHODIMP GetRate( __out double * pdRate);
			virtual STDMETHODIMP GetDuration( __out LONGLONG *pDuration);
			virtual STDMETHODIMP GetAvailable( __out_opt LONGLONG *pEarliest, __out_opt LONGLONG *pLatest );
			virtual STDMETHODIMP GetPreroll( __out LONGLONG *pllPreroll );

			// IDispatch
			virtual STDMETHODIMP GetTypeInfoCount(__out UINT * pctinfo) ;
			virtual STDMETHODIMP GetTypeInfo
				( UINT itinfo, LCID lcid, __deref_out ITypeInfo ** pptinfo) ;
			virtual STDMETHODIMP GetIDsOfNames
				( REFIID riid, __in_ecount(cNames) LPOLESTR * rgszNames,
					UINT cNames, LCID lcid, __out_ecount(cNames) DISPID * rgdispid) ;
			virtual STDMETHODIMP Invoke
				( DISPID dispidMember, REFIID riid, LCID lcid,
					WORD wFlags, __in DISPPARAMS * pdispparams,
					__out_opt VARIANT * pvarResult,
					__out_opt EXCEPINFO * pexcepinfo, __out_opt UINT * puArgErr) ;

			// IMediaPosition
			virtual STDMETHODIMP get_Duration(__out REFTIME * plength);
			virtual STDMETHODIMP put_CurrentPosition(REFTIME llTime);
			virtual STDMETHODIMP get_StopTime(__out REFTIME * pllTime);
			virtual STDMETHODIMP put_StopTime(REFTIME llTime);
			virtual STDMETHODIMP get_PrerollTime(__out REFTIME * pllTime);
			virtual STDMETHODIMP put_PrerollTime(REFTIME llTime);
			virtual STDMETHODIMP get_Rate(__out double * pdRate);
			virtual STDMETHODIMP put_Rate(double dRate);
			virtual STDMETHODIMP get_CurrentPosition(__out REFTIME * pllTime);
			virtual STDMETHODIMP CanSeekForward(__out LONG *pCanSeekForward);
			virtual STDMETHODIMP CanSeekBackward(__out LONG *pCanSeekBackward);

			friend class Filter ;
		} ;

		// 入力ピン
		class	InputPin	: public IPin,
								public IQualityControl, public IMemInputPin
		{
		protected:
			atomic_int_t		m_nRef ;

			SSystem::SString	m_strName ;
			AM_MEDIA_TYPE		m_mtType ;

			IPin *				m_pConnected ;
			Filter *			m_pFilter ;
			IMemAllocator *		m_pAlloc ;
			IQualityControl *	m_pQCtrl ;

			REFERENCE_TIME		m_rtStart ;
			REFERENCE_TIME		m_rtStop ;
			double				m_fpRate ;

			bool				m_fReadOnly ;
			bool				m_fFlushing ;
			bool				m_fRuntimeError ;

			AM_SAMPLE2_PROPERTIES	m_propSample ;

		public:
			// 構築関数
			InputPin( Filter * pFilter, const wchar_t * pwszName ) ;
			// 消滅関数
			~InputPin( void ) ;

		public:
			// アロケータ生成
			virtual IMemAllocator * CreateAllocator( void ) ;
			// アロケータ取得
			IMemAllocator * GetAllocator( void ) const
			{
				return	m_pAlloc ;
			}
			// 名前取得
			const SSystem::SString& GetName( void ) const
			{
				return	m_strName ;
			}
			// 接続状態
			bool IsConnected( void ) const
			{
				return	(m_pConnected != NULL) ;
			}
			IPin * GetConnected( void ) const
			{
				return	m_pConnected ;
			}
			// 停止判定
			bool IsStopped( void ) const
			{
				return	(m_pFilter->m_state == State_Stopped) ;
			}
			// フラッシュ中
			bool IsFlushing( void ) const
			{
				return	m_fFlushing ;
			}
			// サンプルプロパティ
			const AM_SAMPLE2_PROPERTIES & SampleProps( void ) const
			{
				ESLAssert( m_propSample.cbData != 0 ) ;
				return	m_propSample ;
			}
			// ストリーム検査
			virtual HRESULT CheckStreaming( void ) const ;
			// 接続破棄
			virtual HRESULT BreakConnect( void ) ;
			// 接続完了
			virtual HRESULT CompleteConnect( IPin * pReceivePin ) ;
			// メディアタイプ取得
			virtual HRESULT GetMediaType
				( size_t i, __inout AM_MEDIA_TYPE * pmt ) ;
			// メディアタイプ設定
			virtual HRESULT SetMediaType( const AM_MEDIA_TYPE * pmt ) ;
			// 受け入れ可能なメディアタイプ判定
			virtual HRESULT CheckMediaType( const AM_MEDIA_TYPE * pmt ) ;
			// 接続検査
			virtual HRESULT CheckConnect( IPin * pPin ) ;
			// 停止状態から変化時に呼び出される
			virtual HRESULT Active( void ) ;
			// 停止状態へ変化時に呼び出される
			virtual HRESULT Inactive( void ) ;
			// フィルタからの実行通知
			virtual HRESULT Run( REFERENCE_TIME tStart ) ;
			// 接続試行
			virtual HRESULT AttemptConnection
				( IPin * pReceivePin, const AM_MEDIA_TYPE * pmt ) ;
			virtual HRESULT TryMediaTypes
				( IPin * pReceivePin,
					__in_opt const AM_MEDIA_TYPE * pmt, IEnumMediaTypes *pEnum ) ;
			// メディア承諾
			virtual HRESULT AgreeMediaType
				( IPin * pReceivePin, const AM_MEDIA_TYPE * pmt ) ;
			// メディア受け取り処理
			HRESULT ReceiveWithoutNotify( IMediaSample * pSample ) ;

		public:
			// IUnknown
			virtual HRESULT STDMETHODCALLTYPE QueryInterface( REFIID riid, void ** ppObj ) ;
			virtual ULONG STDMETHODCALLTYPE AddRef( void ) ;
			virtual ULONG STDMETHODCALLTYPE Release( void ) ;

			// IPin
			virtual STDMETHODIMP Connect
				( IPin * pReceivePin, __in_opt const AM_MEDIA_TYPE *pmt ) ;
			virtual STDMETHODIMP ReceiveConnection
				( IPin * pConnector, const AM_MEDIA_TYPE *pmt ) ;
			virtual STDMETHODIMP Disconnect( void ) ;
			virtual STDMETHODIMP ConnectedTo( __deref_out IPin ** ppPin ) ;
			virtual STDMETHODIMP ConnectionMediaType( __out AM_MEDIA_TYPE * pmt ) ;
			virtual STDMETHODIMP QueryPinInfo( __out PIN_INFO * pInfo ) ;
			virtual STDMETHODIMP QueryDirection( __out PIN_DIRECTION * pPinDir ) ;
			virtual STDMETHODIMP QueryId( __deref_out LPWSTR * Id ) ;
			virtual STDMETHODIMP QueryAccept( const AM_MEDIA_TYPE * pmt ) ;
			virtual STDMETHODIMP EnumMediaTypes
					( __deref_out IEnumMediaTypes ** ppEnum ) ;
			virtual STDMETHODIMP QueryInternalConnections
				( __out_ecount_part(*nPin,*nPin) IPin* *apPin, __inout ULONG *nPin ) ;
			virtual STDMETHODIMP EndOfStream( void ) ;
		    virtual STDMETHODIMP BeginFlush( void ) ;
			virtual STDMETHODIMP EndFlush( void ) ;
			virtual STDMETHODIMP NewSegment
				( REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate ) ;

			// IQualityControl
			virtual STDMETHODIMP Notify( IBaseFilter * pSender, Quality q ) ;
			virtual STDMETHODIMP SetSink( IQualityControl * piqc ) ;

			// IMemInputPin
			virtual STDMETHODIMP GetAllocator
					( __deref_out IMemAllocator ** ppAllocator ) ;
			virtual STDMETHODIMP NotifyAllocator
					( IMemAllocator * pAllocator, BOOL bReadOnly ) ;
			virtual HRESULT STDMETHODCALLTYPE GetAllocatorRequirements
					( __out  ALLOCATOR_PROPERTIES *pProps ) ;
			virtual STDMETHODIMP Receive( IMediaSample * pSample ) ;
			virtual STDMETHODIMP ReceiveMultiple
					( __in_ecount(nSamples) IMediaSample ** ppSamples,
						long nSamples, __out long * pSamplesProcessed ) ;
			virtual STDMETHODIMP ReceiveCanBlock( void ) ;
		} ;

		// メディアタイプ列挙
		class	EnumMediaTypes	: public IEnumMediaTypes
		{
		protected:
			atomic_int_t	m_nRef ;
			InputPin *		m_pPin ;
			size_t			m_iPos ;
		public:
			// 構築関数
			EnumMediaTypes( InputPin * pPin ) ;
		public:
			// IUnknown
			virtual HRESULT STDMETHODCALLTYPE QueryInterface( REFIID riid, void ** ppObj ) ;
			virtual ULONG STDMETHODCALLTYPE AddRef( void ) ;
			virtual ULONG STDMETHODCALLTYPE Release( void ) ;

			// IEnumMediaTypes
			virtual HRESULT STDMETHODCALLTYPE Next
				( ULONG cMediaTypes,
					__out_ecount(cMediaTypes) AM_MEDIA_TYPE ** ppMediaTypes,
					__out_opt ULONG * pcFetched ) ;
			virtual HRESULT STDMETHODCALLTYPE Skip( ULONG cMediaTypes ) ;
			virtual HRESULT STDMETHODCALLTYPE Reset( void ) ;
			virtual HRESULT STDMETHODCALLTYPE Clone
				( __deref_out IEnumMediaTypes **ppEnum ) ;
		} ;

		// レンダリングフィルタ実装
		class	Filter : public IBaseFilter, public IAMovieSetup,
							public IQualProp, public IQualityControl
		{
		protected:
			atomic_int_t		m_nRef ;
			SSystem::SString	m_strName ;

			SGLDSRenderMediaPlayer *	m_pPlayer ;

			SSystem::SCriticalSection	m_csObj ;
			SSystem::SCriticalSection	m_csSync ;
			SSystem::SCriticalSection	m_csFrame ;

			PosPassThru *		m_pPos ;
			InputPin *			m_pInputPin ;
			IMediaSample *		m_pSample ;
			IMediaEventSink *	m_pSink ;
			IReferenceClock *	m_pClock ;
			IFilterGraph *		m_pGraph ;
			IQualityControl *	m_pQCtrl ;

			FILTER_STATE		m_state ;
			REFERENCE_TIME		m_rtStart ;
			REFERENCE_TIME		m_rtStop ;

			bool				m_fAbort ;
			bool				m_fStreaming ;
			bool				m_fEndOfStream ;
			bool				m_fStreamComplete ;
			bool				m_fRepaint ;
			bool				m_fReceiving ;
			DWORD				m_dwAdviseCookie ;

			SSystem::SSignalEvent	m_sigRunning ;
			SSystem::SSignalEvent	m_sigRender ;
			SSystem::SSignalEvent	m_sigRenderNoWait ;
			UINT					m_idTimerDelayEOS ;

			REFERENCE_TIME		m_rtStampSample ;
			REFERENCE_TIME		m_rtLastDraw ;
			int64_t				m_nRefTimeOffset ;
			int					m_nEarliness ;
			int					m_nWaitAvg ;
			int					m_nFrameAvg ;
			int					m_nDuration ;
			int					m_nConsecutiveFrames ;
			//
			int					m_nRenderAvg ;
			int					m_nRenderLast ;
			int					m_nThrottle ;
			int					m_nDroppedFrames ;
			int					m_nDrawnFrames ;
			//
			int					m_nLate ;
			int					m_nFrame ;
			int					m_nTotAcc ;
			int					m_nSumSqrLate ;
			int					m_nSumFrameTime ;
			int					m_nSumSqrFrameTime ;

			SSystem::STimeCounter	m_timerRendering ;
			SSystem::STimeCounter	m_timerStreaming ;

			enum	Units
			{
				MILLISECONDS	= 1000,
			/*	NANOSECONDS		= 1000000000, */
				UNITS			= 10000000 /* NANOSECONDS / 100 */,
			} ;

			VIDEOINFOHEADER		m_vihHeader ;
			SGLImage			m_imgVideo ;
			uint32_t			m_formatSrcVideo ;
			uint32_t			m_depthSrcVideo ;

			friend class InputPin ;

		public:
			// 構築関数
			Filter( const wchar_t * pwszName, SGLDSRenderMediaPlayer * pPlayer ) ;
			// 消滅関数
			~Filter( void ) ;

		public:
			// フレーム取得
			SGLImageObject * LockVideoFrame( void ) ;
			void UnlockVideoFrame( void ) ;

		public:
			// PosPassThru 取得
			HRESULT GetMediaPosition( REFIID riid, __deref_out void **ppv ) ;
			// InputPin 取得
			InputPin * GetPin( void ) ;
			// イベント通知
			HRESULT NotifyEvent
				( long EventCode, LONG_PTR EventParam1, LONG_PTR EventParam2 ) ;
			// 時間差の飽和処理
			static int ClampTimeDiff( REFERENCE_TIME rt ) ;

		public:
			// 接続破棄
			virtual HRESULT OnBreakConnect( void ) ;
			// 接続完了
			virtual HRESULT OnCompleteConnect( IPin * pReceivePin ) ;
			// メディアタイプ設定
			virtual HRESULT OnSetMediaType( const AM_MEDIA_TYPE * pmt ) ;
			// 受け入れ可能なメディアタイプ判定
			virtual HRESULT CheckMediaType( const AM_MEDIA_TYPE * pmt ) ;
			// 停止状態から変化時に呼び出される
			virtual HRESULT OnActive( void ) ;
			// 停止状態へ変化時に呼び出される
			virtual HRESULT OnInactive( void ) ;
			// ストリーミング開始時
			virtual void OnStartStreaming( void ) ;
			// ストリーミング停止時
			virtual void OnStopStreaming( void ) ;
			// ストリーム終端時
			virtual HRESULT OnEndOfStream( void ) ;
			// フラッシュ開始
		    virtual HRESULT OnBeginFlush( void ) ;
			// フラッシュ終了
			virtual HRESULT OnEndFlush( void ) ;
			// 描画イベント待ち開始
			virtual void OnStartWatingRender( void ) ;
			// 描画イベント待ち終了
			virtual void OnEndWatingRender( void ) ;
			// 描画開始
			virtual void OnRenderStart( IMediaSample * pms ) ;
			// 描画終了
			virtual void OnRenderEnd( IMediaSample * pms ) ;
			// 描画実行
			virtual HRESULT DoRenderSample( IMediaSample * pms ) ;
			// 描画準備
		    virtual void PrepareRender( void ) ;

		public:
			// ストリーミング制御変数初期化
			virtual void ResetStreamingTimes( void ) ;
			// ストリーミング開始
			virtual HRESULT StartStreaming( void ) ;
			// ストリーミング停止
			virtual void StopStreaming( void ) ;
			// ストリーミング中判定
			virtual bool IsStreaming( void ) ;
			// ストリーム終端判定
			virtual bool IsEndOfStream( void ) ;
			virtual bool IsEndOfStreamDelivered( void ) ;
		protected:
			// タイマーコールバック関数
			static void CALLBACK TimerProcDelayEndOfStream
				( UINT uTimerID, UINT uMsg,
					DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2 ) ;
		public:
			// ストリーム終端通知（遅延あり）
			virtual HRESULT DelayNotifyEndOfStream( void ) ;
			// ストリーム終端通知
			virtual HRESULT NotifyEndOfStream( void ) ;
			// ストリーム終了時リセット
			virtual HRESULT ResetEndOfStream( void ) ;
			// 遅延通知タイマキャンセル
			void KillTimerDelayNotifyEOS( void ) ;
			// 受信完了待ち
			void WaitForReceiveToComplete( void ) ;
			// 描画イベント待機
			HRESULT WaitForRenderSignal( void ) ;
			// 描画イベント待機設定
			void EnableRenderWait( bool fRenderWait ) ;
			// スロットリング待機
			void WaitThrottle( void ) ;
			// 描画時間イベント発生時処理
			virtual void OnSignalRenderEvent( void ) ;
			// 描画時間イベントのスケジュール
			bool ScheduleSampleAdvice( IMediaSample * pms ) ;
			// 描画時間イベント通知のキャンセル
			HRESULT CancelSampleAdvice( void ) ;
			// サンプル時間取得
			virtual HRESULT GetSampleTimes
				( IMediaSample * pms,
					__out REFERENCE_TIME * prtStart,
					__out REFERENCE_TIME * prtEnd ) ;
			// 即時描画すべきか？
			HRESULT ShouldSampleDrawImmediately
				( IMediaSample * pms,
					__out REFERENCE_TIME * prtStart,
					__out REFERENCE_TIME * prtEnd ) ;
			// 再描画フラグ設定
			void SetRepaintFlag( bool fRepaint ) ;
			// 再描画イベントを通知
			void NotifyRepaintEvent( void ) ;
			// クオリティ通知
			HRESULT NotifyQuality( REFERENCE_TIME rtLate, REFERENCE_TIME rtStream ) ;
			// 遅延記録
			void RecordFrameLateness( int nLate, int nFrame ) ;
			// ステータス変化完了処理
			HRESULT CompleteStateChange( FILTER_STATE stateOld ) ;
			// サンプル解放
			void ClearPendingSample( void ) ;
			// フレーム標準偏差
			HRESULT GetStandardDeviations
				( int nSamples, __out int *piResult,
						LONGLONG nSumSq, LONGLONG nTot ) ;

		public:
			// 先頭サンプル受信事
		    virtual void OnReceiveFirstSample( IMediaSample * pms ) ;
			// サンプル受け取り準備
			virtual HRESULT PrepareReceive( IMediaSample * pms ) ;
			virtual HRESULT Receive( IMediaSample * pms ) ;
			// サンプルがあるか？
			virtual bool HaveCurrentSample( void ) ;
			// サンプル取得
			virtual IMediaSample * GetCurrentSample( void ) ;
			// 描画処理
			virtual HRESULT Render( IMediaSample * pms ) ;

		public:
			// IUnknown
			virtual HRESULT STDMETHODCALLTYPE QueryInterface( REFIID riid, void ** ppObj ) ;
			virtual ULONG STDMETHODCALLTYPE AddRef( void ) ;
			virtual ULONG STDMETHODCALLTYPE Release( void ) ;

			// IPersist
			virtual STDMETHODIMP GetClassID( __out CLSID *pClsID ) ;

			// IMediaFilter
			virtual STDMETHODIMP GetState( DWORD dwMSecs, __out FILTER_STATE * pState ) ;
			virtual STDMETHODIMP SetSyncSource( __in_opt IReferenceClock *pClock ) ;
			virtual STDMETHODIMP GetSyncSource( __deref_out_opt IReferenceClock **pClock ) ;
			virtual STDMETHODIMP Stop( void ) ;
			virtual STDMETHODIMP Pause( void ) ;
			virtual STDMETHODIMP Run( REFERENCE_TIME tStart ) ;

			// IBaseFilter
			virtual STDMETHODIMP EnumPins( __deref_out IEnumPins ** ppEnum ) ;
			virtual STDMETHODIMP FindPin( LPCWSTR Id, __deref_out IPin ** ppPin ) ;
			virtual STDMETHODIMP QueryFilterInfo( __out FILTER_INFO * pInfo ) ;
			virtual STDMETHODIMP JoinFilterGraph
				( __inout_opt IFilterGraph * pGraph, __in_opt LPCWSTR pName ) ;
			virtual STDMETHODIMP QueryVendorInfo( __deref_out LPWSTR* pVendorInfo ) ;

			// IAMovieSetup
			virtual STDMETHODIMP Register( void ) ;
			virtual STDMETHODIMP Unregister( void ) ;

			// IQualProp property
			virtual STDMETHODIMP get_FramesDroppedInRenderer( __out int * cFramesDropped ) ;
			virtual STDMETHODIMP get_FramesDrawn( __out int * pcFramesDrawn ) ;
			virtual STDMETHODIMP get_AvgFrameRate( __out int * piAvgFrameRate ) ;
			virtual STDMETHODIMP get_Jitter( __out int * piJitter ) ;
			virtual STDMETHODIMP get_AvgSyncOffset( __out int * piAvg ) ;
			virtual STDMETHODIMP get_DevSyncOffset( __out int * piDev ) ;

			// IQualityControl
			virtual STDMETHODIMP SetSink( IQualityControl * piqc ) ;
			virtual STDMETHODIMP Notify( IBaseFilter * pSelf, Quality q ) ;

		} ;
		friend class Filter ;

		// Pin 列挙
		class	EnumPins	: public IEnumPins
		{
		protected:
			atomic_int_t	m_nRef ;
			Filter *		m_pFilter ;
			size_t			m_iPos ;
		public:
			// 構築関数
			EnumPins( Filter * pFilter ) ;
			// 消滅関数
			~EnumPins( void ) ;
		public:
			// IUnknown
			virtual HRESULT STDMETHODCALLTYPE QueryInterface( REFIID riid, void ** ppObj ) ;
			virtual ULONG STDMETHODCALLTYPE AddRef( void ) ;
			virtual ULONG STDMETHODCALLTYPE Release( void ) ;

			// IEnumPins
			virtual HRESULT STDMETHODCALLTYPE Next
				( ULONG cPins,
					__out_ecount_part(cPins, *pcFetched) IPin ** ppPins,
					__out_opt ULONG * pcFetched ) ;
			virtual HRESULT STDMETHODCALLTYPE Skip( ULONG cPins ) ;
			virtual HRESULT STDMETHODCALLTYPE Reset( void ) ;
			virtual HRESULT STDMETHODCALLTYPE Clone( __out IEnumPins ** ppEnum ) ;
		} ;

	public:
		// メディアマッチング
		static bool MediaTypeMatchesPartial
			( const AM_MEDIA_TYPE * pmt0, const AM_MEDIA_TYPE * pmt1 ) ;
		// 部分的なメディアタイプか判定
		static bool MediaTypeIsPartiallySpecified( const AM_MEDIA_TYPE * pmt ) ;
		// AM_MEDIA_TYPE 操作
		static void DeleteMediaType( __inout_opt AM_MEDIA_TYPE * pmt ) ;
		static HRESULT CopyMediaType
			(__out AM_MEDIA_TYPE * pmtDst, const AM_MEDIA_TYPE * pmtSrc ) ;
		static void FreeMediaType( __inout AM_MEDIA_TYPE& mt ) ;
		// 映像フォーマットを GUID から EntisGLS 画像フォーマットへ変換
		static bool ConvertVideoSubTypeGUID
			( uint32_t& format, uint32_t& depth, const GUID& guid ) ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLDSRenderMediaPlayer, SGLMediaPlayerInterface )
		// 構築関数
		SGLDSRenderMediaPlayer( void ) ;
		// 消滅関数
		virtual ~SGLDSRenderMediaPlayer( void ) ;

	protected:
		// DirectShow 再生用
		struct IGraphBuilder *	m_pGraphBuilder ;
		struct IMediaControl *	m_pMediaControl ;
		struct IVideoWindow *	m_pVideoWindow ;
		struct IBasicAudio *	m_pBasicAudio ;
		struct IBasicVideo *	m_pBasicVideo ;
		struct IMediaPosition *	m_pMediaPosition ;
		struct IMediaEvent *	m_pMediaEvent ;

		SGLDSFileSource *		m_pSrcFile ;
		Filter *				m_pFilter ;
		SSystem::SSignalEvent	m_eventAbort ;
		SSystem::SMutex *		m_pMutexUI ;

		// ファイルパス
		SSystem::SString		m_strFilePath ;

		// 再生ステータス
		bool					m_flagPlayed ;
		bool					m_flagPaused ;
		bool					m_flagLoop ;
		int64_t					m_msLoopStart ;
		int64_t					m_msLoopEnd ;

		// 表示領域
		SGLAbstractWindow *		m_pWindow ;
		SGLImageRect			m_rectDstView ;

		// 通知リスナ
		SGLMediaPlayerFrameNotification *	m_pListener ;

		SSystem::SSmartPointer<SGLWindowsAVIReader>
								m_pAVIReader ;


	protected:
		// フレーム更新通知
		void OnUpdateVideoFrame( void ) ;
		// 区間終端通知
		void OnEndOfStream( void ) ;
		// 中断判定つき画面ミューテックス
		bool SystemLock( void ) ;
		void SystemUnlock( void ) ;
		// ファイルパス取得
		bool NormalizeFilePath
			( const wchar_t * pwszFilePath,
				SSystem::SEnvironmentInterface * pEnv ) ;
		// Pin 取得
		IPin * GetPinOf( IBaseFilter * pFilter, PIN_DIRECTION dir ) ;

		// SGLWindowsAVIReader 取得
		SGLWindowsAVIReader * GetAVIMediaStream( void ) ;

	public:
		// 指定ファイルを開く
		virtual SGLError Open
			( const wchar_t * pwszFilePath, uint64_t nFlags = 0,
					SSystem::SEnvironmentInterface * pEnv = NULL ) ;
		virtual SGLError Create
			( SSystem::SFileInterface * file,
				bool flagOwner = true, uint64_t nFlags = 0 ) ;
		// データを参照する複製プレイヤー生成
		virtual SGLAudioPlayerInterface * ClonePlayer( void ) ;
		// ファイルを閉じる
		virtual SGLError Close( void ) ;
		// 再生を開始する
		virtual SGLError Play( uint64_t nFlags = 0 ) ;
		// 再生を停止する
		virtual SGLError Stop( void ) ;
		// ループポイント[/sample] を設定する
		virtual SGLError SetLoop
			( bool fLoop = true, int64_t nStart = -1, int64_t nEnd = -1 ) ;
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
		// メディアのサンプル周波数を取得する
		virtual uint32_t GetSampleFrequency( void ) const ;
		// メディアの全長 [/sample] を取得する
		virtual uint64_t GetTotalLength( void ) const ;
		// 再生位置 [/sample] を取得する
		virtual uint64_t GetPosition( void ) ;
		// 再生位置 [/sample] を変更する
		virtual void SeekPosition( uint64_t nPos ) ;
		// オーディオストリーム取得
		virtual SGLAudioInputStream * GetAudioStream( void ) ;
		virtual void ReleaseAudioStream( SGLAudioInputStream * pStream ) ;
		// スレッド同期用ミューテックス設定
		virtual void SetUIThreadMutex( SSystem::SMutex * pMutex ) ;

	public:
		// ビデオサイズを取得する
		virtual SGLError GetVideoSize( SGLSize& sizeVideo ) ;
		// 表示先を設定する
		virtual SGLError SetVideoView
			( SGLAbstractWindow* pWindow,
				const SGLImageRect& rectVideo, uint64_t nFlags = 0 ) ;
		// 現在のフレームを描画する
		virtual SGLError DrawVideo
			( SGLPaintContextInterface* pPaint,
				const SGLImageRect& rectDst,
				uint32_t nFlags = 0, uint32_t nTransparency = 0 ) ;
		// メディア再生通知リスナ設定
		virtual SGLMediaPlayerFrameNotification *
			SetNotificationListener
				( SGLMediaPlayerFrameNotification * pListener ) ;
		// ビデオストリーム取得
		virtual SGLVideoInputStream * GetVideoStream( void ) ;
		virtual void ReleaseVideoStream( SGLVideoInputStream * pStream ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// DirectShow カメラキャプチャー再生インターフェース（内部描画）
	//////////////////////////////////////////////////////////////////////////

	class	SGLDSVideoCapturePlayer
						: public SGLDSRenderMediaPlayer,
							public SGLVideoCaptureInterface
	{
	protected:
		IBaseFilter *	m_pCaptureSrc ;

	public:
		class	DSDeviceInfo
					: public SGLVideoCaptureInterface::DeviceInfo
		{
		protected:
			IBaseFilter *				m_pSrcFilter ;
			ICaptureGraphBuilder2 *		m_pCapture ;
			IAMStreamConfig *			m_pStreamConfig ;
		public:
			ESL_DECLARE_CLASS_INFO( DSDeviceInfo, DeviceInfo )
			DSDeviceInfo( void ) ;
			~DSDeviceInfo( void ) ;

			friend class SGLDSVideoCapturePlayer ;
		} ;
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
			( SGLDSVideoCapturePlayer,
				SGLDSRenderMediaPlayer, SGLVideoCaptureInterface )
		// 構築関数
		SGLDSVideoCapturePlayer( void ) ;
		// 消滅関数
		virtual ~SGLDSVideoCapturePlayer( void ) ;

	public:
		// デバイス検索
		static SGLError GetDeviceInfo
			( DSDeviceInfo& devInfo, const wchar_t * pwszDevID ) ;
		// デバイス列挙
		static void EnumerateDevices
			( SSystem::SObjectArray
				<SGLVideoCaptureInterface::DeviceInfo>& aDevInfo ) ;
	protected:
		static void EnumerateDeviceResolutions( DSDeviceInfo& devInfo ) ;

	public:
		// デバイス検索
		virtual DeviceInfo *
			GetCaptureDeviceInfo( const wchar_t * pwszDevID ) ;
		// デバイス列挙
		virtual void EnumerateCaptureDevices
			( SSystem::SObjectArray<DeviceInfo>& aDevInfo ) ;
		// キャプチャーデバイスを開く
		virtual SGLError OpenCapture
			( const wchar_t * pwszDevName = NULL, int iFormat = 0 ) ;
		virtual SGLError OpenCapture
			( const DeviceInfo* pDevInfo, int iFormat ) ;
		// ファイルを閉じる
		virtual SGLError Close( void ) ;

	} ;

}


#endif

