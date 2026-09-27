
/*****************************************************************************
             Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
		Copyright (c) 1998-2007 Leshade Entis. All rights reserved.
 ****************************************************************************/


#if	!defined(__WAVSOUND_H__)
#define	__WAVSOUND_H__

class	EWaveOutDevice ;
	class	EWaveMixingServer ;
class	EWaveStreamBuffer ;
	class	EWaveSound ;
		class	E3DSoundEffect ;
		class	MIOSoundStream ;

/*****************************************************************************
					音声ミキシング低水準関数
 ****************************************************************************/

extern	"C"
{
	unsigned int glsSound_GetConversionSize
		(
			const WAVEFORMATEX *	pDstFormat ,
			const WAVEFORMATEX *	pSrcFormat ,
			unsigned int			nSampleCount ,
			unsigned int			fSizeSpecification = 0
		) ;

	ESLError glsSound_ConvertPCMFormat
		(
			const WAVEFORMATEX *	pDstFormat ,
			void *					pDstPCM ,
			const WAVEFORMATEX *	pSrcFormat ,
			const void *			pSrcPCM ,
			unsigned int			nSampleCount
		) ;

	ESLError glsSound_MixPCMSamples
		(
			const WAVEFORMATEX *	pDstFormat ,
			void *					pDstPCM ,
			const void *			pSrcPCM ,
			const REAL32 *			pDstVolume ,
			unsigned int			nSampleCount
		) ;

	ESLError glsSound_CleanSoundBuffer
		(
			const WAVEFORMATEX *	pSoundFormat ,
			void *					pSoundBuffer ,
			unsigned int			nSampleCount
		) ;

	ESLError glsSound_RemixPCMSamples
		(
			const WAVEFORMATEX *	pDstFormat ,
			void *					pDstPCM ,
			const void *			pSrcPCM ,
			const REAL32 *			pNewVolume ,
			const REAL32 *			pOldVolume ,
			unsigned int			nSampleCount
		) ;

} ;


/*****************************************************************************
						音声出力デバイスクラス
 ****************************************************************************/

typedef	WAVEHDR *	HWAVEBUF ;

class	EWaveOutDevice : public	ESLObject
{
public:
	// 構築関数
	EWaveOutDevice( void ) ;
	// 消滅関数
	virtual ~EWaveOutDevice( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( EWaveOutDevice, ESLObject )

protected:	// データメンバ
	// 音声出力デバイスハンドル
	WAVEFORMATEX *		m_pWaveFormat ;
	HWAVEOUT			m_hWaveOut ;

	// 音声出力コールバックスレッド
	HANDLE				m_hThread ;
	DWORD				m_idThread ;
	HANDLE				m_hThreadCreated ;

	// デバイス音量
	unsigned int		m_nInitVolume[2] ;
	unsigned int		m_nTotalVolume[2] ;
	bool				m_fDevPaused ;

	// 音声出力バッファの配列
	EPtrObjArray<WAVEHDR>	m_arrayPlayBuf ;

	// クリティカルセクション
	CRITICAL_SECTION	m_cs ;

public:		// 操作
	// 音声出力デバイスを開く
	ESLError Open( const WAVEFORMATEX * pwfx ) ;
	// 音声出力デバイスを閉じる
	virtual void Close( void ) ;
	// 出力フォーマット取得
	WAVEFORMATEX * GetWaveFormat( void ) const
		{
			return	m_pWaveFormat ;
		}

	// 音声バッファの出力準備
	virtual HWAVEBUF PrepareBuffer
		( EWaveStreamBuffer * pWaveBuffer,
			const void * ptrBuffer, unsigned int nBufferLength ) ;
	// 音声バッファの終了
	virtual ESLError UnprepareBuffer( HWAVEBUF hWaveBuf ) ;

	// 音声バッファ出力
	virtual ESLError Play
		( EWaveStreamBuffer * pWaveBuffer, HWAVEBUF hWaveBuf ) ;
	// 音声出力停止
	virtual ESLError Stop( EWaveStreamBuffer * pWaveBuffer ) ;

	// 音声ストリーム制御
	virtual ESLError Pause( EWaveStreamBuffer * pWaveBuffer ) ;
	virtual ESLError Restart( EWaveStreamBuffer * pWaveBuffer ) ;
	virtual ESLError PauseDevice( void ) ;
	virtual ESLError RestartDevice( void ) ;

public:		// デバイス属性
	// 現在の再生位置を取得
	virtual UINT64 GetCurrentSample
			( const EWaveStreamBuffer * pWaveBuffer ) ;

	// 出力ボリューム
	virtual ESLError GetTotalVolume( unsigned int nVolume[] ) const ;
	virtual ESLError SetTotalVolume( const unsigned int nVolume[] ) ;
	virtual ESLError GetVolume
		( EWaveStreamBuffer * pWaveBuffer, REAL32 nVolume[] ) const ;
	virtual ESLError SetVolume
		( EWaveStreamBuffer * pWaveBuffer, const REAL32 nVolume[] ) ;

	// 音声出力デバイスの総数を取得
	static unsigned int GetWaveInstalled( void ) ;

public:		// スレッド排他アクセス
	void Lock( void ) const ;
	void Unlock( void ) const ;

protected:	// コールバック関数
	// スレッド関数
	static DWORD WINAPI WaveCallbackServiceThread( LPVOID lpParam ) ;
	// スレッド関数
	virtual DWORD WaveCallbackThread( void ) ;
	// 音声出力バッファの再生完了
	virtual void OnBufferPlayed( WAVEHDR * lpWaveHdr ) ;

} ;


/*****************************************************************************
					音声ミキシングチャネルオブジェクト
 ****************************************************************************/

struct	WAVE_MIXER_CHANNEL
{
public:
	// 一般的なパラメータ
	//////////////////////////////////////////////////////////////////////////
	EWaveMixingServer * m_pMixingServer ;	// Pointer to EWaveMixingServer
	EWaveStreamBuffer *	m_pWaveBufObj ;		// Pointer to EWaveStreamBuffer
	WAVEFORMATEX		m_WaveFormat ;		// PCM wave format to decode
	unsigned int		m_nPauseFlag ;		// Un-paused(0), Paused(1)
	unsigned __int64	m_nBaseSampleIndex ;// Base sample index

	enum
	{
		BUFFER_COUNT = 2
	} ;

	// Wave Output Buffer
	// To mix into mixing buffer of service class
	void *				m_pWaveBuffer[2] ;	// Pointers to wave output buffer
	unsigned int		m_nWaveBufLen[2] ;	// Valid length of m_pWaveBuffer in samples
	// Paused Pending Buffer
	// Buffer of pending when pased
	unsigned __int64	m_nPausedPosition ;	// Samples position when paused
	EStreamBuffer		m_PausedBuffer ;	// Wave buffer when paused
	// 
	// Source Buffer Entries
	unsigned int		m_nOffsetSrcBuf ;	// Output bytes of m_pNextWaveHdr
	WAVEHDR *			m_pNextWaveHdr ;	// Next source wave buffer
	unsigned __int64	m_nOutputCounter ;	// Output sample count
	EPtrObjArray<WAVEHDR>	m_QueueWaveHdr ;	// Input queue

	// Waste buffer
	EPtrObjArray<WAVEHDR>	m_WasteWaveHdr ;	// Waste wave buffer

	// DirectSound
	struct IDirectSoundBuffer *	m_idsbuf ;		// DirectSound buffer
	HANDLE				m_hDirectSoundNotify[BUFFER_COUNT] ;
	unsigned int		m_nBufferSwitch ;
	UINT64				m_nPlayPosBias ;

	// ACM (Audio Compression Manager) 用パラメータ
	//////////////////////////////////////////////////////////////////////////
	// ACM source and destination buffers
	// If m_pSrcFormat isn't null(0), following parameters are valid.
	HACMSTREAM			m_hACMStream ;		// ACM stream handle
	LPWAVEFORMATEX		m_pSrcFormat ;		// Source wave format
	EStreamBuffer		m_ACMDstBuffer ;	// Destination buffer
	EStreamBuffer		m_ACMSrcBuffer ;	// Source buffer

public:
	// 構築関数
	WAVE_MIXER_CHANNEL( void ) ;
	// 消滅関数
	~WAVE_MIXER_CHANNEL( void ) ;

	// ミキサーチャネルを開く
	ESLError Open( EWaveMixingServer * pServer,
			EWaveStreamBuffer * pBufObj, WAVEHDR * pFirstBuffer ) ;
	// 再利用のため初期化する
	ESLError ReInitialize( EWaveMixingServer * pServer,
			EWaveStreamBuffer * pBufObj, WAVEHDR * pFirstBuffer ) ;

	// ミキサーを停止する（再利用のため）
	ESLError Stop( void ) ;

	// 次の PCM 音声サンプルを取得
	void * GetNextWaveSamples
		( unsigned int nBufIndex, unsigned int nOffsetSamples,
			unsigned int & nLenSamples, const WAVEFORMATEX * pMixingFormat ) ;

	// DirectSoundBuffer へ音量反映
	void SetDirectSoundVolume( void ) ;
	// DirectSoundBuffer ストリーミング再生開始
	void PlayDirectSound( void ) ;
	// 現在の再生位置取得
	UINT64 GetCurrentDirectSoundPosition( void ) ;
	// DirectSoundBuffer ストリーミング処理
	bool StreamingDirectSoundBuffer
		( unsigned int nBufIndex, const WAVEFORMATEX * pMixingFormat ) ;
	void OnTimerDirectSoundStreaming( void ) ;

} ;


/*****************************************************************************
					音声ミキシングサーバーオブジェクト
 ****************************************************************************/

typedef	WAVE_MIXER_CHANNEL *	HWAVEMIX ;

class	EWaveMixingServer : public	EWaveOutDevice
{
public:
	// 構築関数
	EWaveMixingServer( void ) ;
	// 消滅関数
	virtual ~EWaveMixingServer( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( EWaveMixingServer, EWaveOutDevice )

protected:	// データメンバ
	unsigned int		m_nBufferingSize ;		// Size of m_pWaveBuffer in samples
	UINT64				m_nOutputCounter ;		// Output counter in samples
	void *				m_ptrReserved1 ;
	unsigned int		m_nPrimaryIndex ;		// Primary index of m_WaveHeaders
	DWORD				m_dwBaseBufferTime ;	// Windows time last buffer played
	DWORD				m_dwDevPausedTime ;		// Windows time when device paused

	void *				m_pWaveBuffers[2] ;		// Output wave buffer
	WAVEHDR				m_WaveHeaders[2] ;		// 
	unsigned int		m_nPreparedSize[2] ;	// Prepared size of m_pWaveBuffers in sample

	HANDLE				m_eventEmptyWaveBuf ;	// Be set to signaled when m_WaveHeaders is empty
	BOOL				m_boolEmptyWaveBuf[2] ;	// 

	unsigned int		m_nQuantumTime ;		// Quantum time
	unsigned int		m_nTimerID ;			// Timer ID
	BOOL				m_fCallbackMode ;

	struct IDirectSound *	m_idsound ;			// DirectSound object
	struct IDirectSoundBuffer *	m_idsbuf ;		// primary DirectSound buffer

	EStreamBuffer *		m_pVirtualBuf ;			// Virtual output buffer
	DWORD				m_dwVirtualLocalPos ;
	bool				m_fDeleteVirtualBuf ;

	// Registered WAVE_MIXER_CHANNEL objects
	EPtrObjArray<WAVE_MIXER_CHANNEL>
						m_listWaveChannel ;		// Array of WAVE_MIXER_CHANNEL object

public:
	// ミキシングサービスを開始する
	ESLError Open
		( unsigned int nBufferingTime = 500,
			unsigned int nQuantumTime = 33,
			unsigned int nFrequency = 44100,
			unsigned int nChannels = 2,
			unsigned int nBitsPerSample = 16 ) ;
	ESLError OpenDirectSound
		( unsigned int nBufferingTime = 371,
			unsigned int nQuantumTime = 33,
			unsigned int nFrequency = 44100,
			unsigned int nChannels = 2,
			unsigned int nBitsPerSample = 16 ) ;
	ESLError OpenVirtual
		( EStreamBuffer * pOutputBuf,
			unsigned int nBufferingTime = 500,
			unsigned int nQuantumTime = 33,
			unsigned int nFrequency = 44100,
			unsigned int nChannels = 2,
			unsigned int nBitsPerSample = 16 ) ;
	ESLError OpenVirtualSync
		( unsigned int nBufferingTime = 500,
			unsigned int nQuantumTime = 33,
			unsigned int nFrequency = 44100,
			unsigned int nChannels = 2,
			unsigned int nBitsPerSample = 16 ) ;
	// ミキシングサービスを終了する
	virtual void Close( void ) ;

protected:
	// 現在のミキシング位置を取得する
	unsigned int GetCurrentMixingPosition( unsigned int nOffsetTime = 33 ) ;

public:
	// 音声バッファの出力準備
	virtual HWAVEBUF PrepareBuffer
		( EWaveStreamBuffer * pWaveBuffer,
			const void * ptrBuffer, unsigned int nBufferLength ) ;
	// 音声バッファの出力終了
	virtual ESLError UnprepareBuffer( HWAVEBUF hWaveBuf ) ;

protected:
	// EWaveStreamBuffer に関連する HWAVEMIX を取得
	HWAVEMIX GetWaveMixChannel
		( const EWaveStreamBuffer * pWaveBuffer ) const ;

public:
	// 音声バッファを出力
	virtual ESLError Play
		( EWaveStreamBuffer * pWaveBuffer, HWAVEBUF hWaveBuf ) ;
	ESLError DelayPlaying
		( EWaveStreamBuffer * pWaveBuffer,
			HWAVEBUF hWaveBuf, unsigned int nDelay = 66 ) ;
	// 音声出力を停止
	virtual ESLError Stop( EWaveStreamBuffer * pWaveBuffer ) ;

	// 音声ストリーム制御
	virtual ESLError Pause( EWaveStreamBuffer * pWaveBuffer ) ;
	virtual ESLError Restart( EWaveStreamBuffer * pWaveBuffer ) ;
	virtual ESLError PauseDevice( void ) ;
	virtual ESLError RestartDevice( void ) ;

public:		// デバイス属性
	// 現在の再生位置を取得
	virtual UINT64 GetCurrentSample
			( const EWaveStreamBuffer * pWaveBuffer ) ;

	// 出力ボリューム
	virtual ESLError SetTotalVolume( const unsigned int nVolume[] ) ;
	virtual ESLError SetVolume
		( EWaveStreamBuffer * pWaveBuffer, const REAL32 nVolume[] ) ;

protected:	// コールバック関数
	// スレッド関数
	virtual DWORD WaveCallbackThread( void ) ;
	// 音声バッファ再生完了
	virtual void OnBufferPlayed( HWAVEBUF hWaveBuf ) ;
	// ミキシングタイマーコールバック
	virtual void OnMixingTimer( void ) ;
	// 次のミキシングバッファを準備
	void PrepareNextMixingBuffer
		( unsigned int nBufIndex, unsigned int nOutputSamples ) ;

public:	// 仮想出力用関数群
	// 音声仮想出力処理
	ESLError AdvanceVirtualPlay( DWORD dwSamples ) ;

protected:
	// 音声バッファ仮想再生終了処理
	void OnVirtualBufferPlayed( void ) ;
	// 仮想（無音）出力、タイミング同期処理用スレッド
	static DWORD WINAPI VirtualSyncThreadProc( LPVOID lpParam ) ;
	DWORD WINAPI VirtualSyncThread( void ) ;

	friend	WAVE_MIXER_CHANNEL ;
} ;


/*****************************************************************************
						音声バッファオブジェクト
 ****************************************************************************/

class	EWaveStreamBuffer : public	ESLObject
{
public:
	// 構築関数
	EWaveStreamBuffer( void ) ;
	// 消滅関数
	virtual ~EWaveStreamBuffer( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( EWaveStreamBuffer, ESLObject )

protected:
	// 音声データフォーマット
	unsigned int			m_nFormatSize ;		// Size of m_pWaveFormat in bytes
	WAVEFORMATEX *			m_pWaveFormat ;
	// 一時停止フラグ
	unsigned int			m_fPauseFlag ;		// Un-paused(0), Paused(1)
	// 出力ボリューム
	REAL32					m_realVolume[2] ;
	// 出力用ミキシングバッファ
	WAVE_MIXER_CHANNEL *	m_pWaveDevMixingBuf ;

protected:	// コールバック関数
	// 音声データの再生が完了した
	virtual void OnEndPlaying
		( HWAVEBUF hWaveBuf, void * ptrBuffer, unsigned int nBufferLength ) ;
	// 音声出力デバイスが次の音声バッファを要求している
	virtual HWAVEBUF OnQueueNextBuffer( EWaveOutDevice * pWaveDev ) ;

public:
	// 音声フォーマットを取得
	const WAVEFORMATEX * GetWaveFormat( void ) const ;
	// 音声フォーマットを設定
	ESLError SetWaveFormat( const WAVEFORMATEX * pwfx ) ;
	// 一時停止フラグを取得
	unsigned int IsPaused( void ) const
		{
			return	m_fPauseFlag ;
		}

public:		// Sample - time translation
	static unsigned int TimeToSample
		( const WAVEFORMATEX * pwfx, unsigned int nMilliSecond ) ;
	static unsigned int SampleToTime
		( const WAVEFORMATEX * pwfx, unsigned int nSamples ) ;
	unsigned int TimeToSample( unsigned int nMilliSecond ) const ;
	unsigned int SampleToTime( unsigned int nSamples ) const ;

public:		// PCM sample - raw bytes size translation
	unsigned int SampleToBytes
		( const WAVEFORMATEX * pwfx, unsigned int nSamples ) const ;
	unsigned int BytesToSample
		( const WAVEFORMATEX * pwfx, unsigned int nBytes ) const ;
	unsigned int SampleToBytes( unsigned int nSamples ) const ;
	unsigned int BytesToSample( unsigned int nBytes ) const ;

	friend	EWaveOutDevice ;
	friend	EWaveMixingServer ;
	friend	WAVE_MIXER_CHANNEL ;
} ;


/*****************************************************************************
						生音声データオブジェクト
 ****************************************************************************/

class	EWaveSound : public	EWaveStreamBuffer
{
public:
	// 構築関数
	EWaveSound( EWaveOutDevice * pWaveDev = NULL ) ;
	// 消滅関数
	virtual ~EWaveSound( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( EWaveSound, EWaveStreamBuffer )

protected:	// データメンバ
	EWaveOutDevice *	m_pWaveDevice ;		// Pointer to EWaveOutDevice

	HANDLE				m_hDoneEvent ;		// Event signal when the sound played
	HWND				m_hwndNotifyDone ;	// Window notify when the sound played
	UINT				m_msgNotifyDone ;	// Message send to m_hwndNotifyDone
	LPARAM				m_paramNotifyDone ;	// Parameter send to m_hwndNotifyDone

	bool				m_flagPlaying ;		// Is playing now ?
	bool				m_flagFinished ;	// Is output finished
	bool				m_flagRepeat ;		// Will be sound repeat?

	bool				m_fBufDelete ;		// this is owner of m_pWaveBuffer
	unsigned int		m_nWaveLength ;		// Length of m_pWaveBuffer in bytes
	void *				m_pWaveBuffer ;		// Pointer to wave data

	unsigned int		m_nRewindingPos ;	// rewinding position in samples
	unsigned int		m_nEndingPos ;		// ending position in samples
											// (last sample index + 1)
	unsigned int		m_nOutputPos ;		// output position
	unsigned int		m_nOutputSize ;		// quantum output size in samples
	unsigned int		m_nLastStartedPos ;
	HWAVEBUF			m_hLastWaveBuf ;
	UINT64				m_nOutputBais ;

	CRITICAL_SECTION	m_cs ;				// Critical section object

protected:	// 再生コールバック
	// 音声データの再生が完了した
	virtual void OnEndPlaying
		( HWAVEBUF hWaveBuf, void * ptrBuffer, unsigned int nBufferLength ) ;
	// 音声出力デバイスが次の音声バッファを要求している
	virtual HWAVEBUF OnQueueNextBuffer( EWaveOutDevice * pWaveDev ) ;

public:		// 操作関数
	// 内容削除
	virtual void Delete( void ) ;
	// ウェーブフォームオーディファイル読みこみ
	ESLError ReadWave( ESLFileObject & file ) ;
	// ウェーブフォームオーディファイル書き出し
	ESLError WriteWave( ESLFileObject & file ) const ;
	// 音声データ設定
	ESLError SetWaveData
		( const WAVEFORMATEX * ptrWaveFormat,
			const void * ptrBuffer, unsigned int nBufLength ) ;
	// 音声データ関連付け
	virtual ESLError AttachWaveSound( const EWaveSound & wavbuf ) ;
	ESLError AttachWaveSound
		( const WAVEFORMATEX * pwfx,
			const void * ptrBuf, unsigned int nBufLen ) ;

	// 出力デバイス関連付け
	void AttachWaveDevice( EWaveOutDevice * pWaveDev ) ;
	// 音声再生開始
	virtual ESLError PlayWave
		( bool fRepeat = false, HANDLE hEvent = NULL ) ;
	// 音声再生開始
	virtual ESLError PlayMusic
		( unsigned int nIntroSamples = 0,
			bool fRepeat = true, HANDLE hEvent = NULL ) ;
	// 遅延再生
	virtual ESLError PlayWithDelayFrom
		( unsigned int nIntroSamples = -1,
			bool fRepeat = false, HANDLE hEvent = NULL,
			unsigned int nDelay = 33, unsigned int nStartPos = 0 ) ;
	// 再生開始
	virtual ESLError PlayFrom
		( unsigned int nStartPos = 0, unsigned int nPlayEnd = -1,
				bool fRepeat = false, unsigned int nRewindPos = -1 ) ;
	// 再生停止
	virtual ESLError StopWave( void ) ;
	// 再生一時停止
	virtual ESLError PauseWave( void ) ;
	// 再生再開
	virtual ESLError RestartWave( void ) ;
	// 再生ループポイント設定
	virtual ESLError SetRewindingPortion
		( unsigned int nRewindPos,
			unsigned int nEndPos = -1, bool fRepeat = true ) ;

public:		// 属性
	// 出力ボリューム取得
	virtual ESLError GetVolume
		( REAL32 & rLeftVolume, REAL32 & rRightVolume ) const ;
	// 出力ボリューム設定
	virtual ESLError SetVolume
		( REAL32 rLeftVolume, REAL32 rRightVolume ) ;
	// 現在の再生位置を取得
	unsigned long int GetCurrentSample( void ) const ;

	// 現在出力している位置を取得
	unsigned long int GetCurrentOutput( void ) const
		{
			return	m_nOutputPos ;
		}
	// リピートフラグ設定
	void SetRepeat( bool fRepeat = true )
		{
			m_flagRepeat = fRepeat ;
		}
	// リピートフラグ取得
	bool IsRepeated( void ) const
		{
			return	m_flagRepeat ;
		}
	// 再生中か？
	bool IsPlaying( void ) const
		{
			return	m_flagPlaying ;
		}
	// 再生終了時に送るウィンドウメッセージを設定
	void SetWindowToNotifyDone
			( HWND hwndNotify, UINT uMsg = MM_WOM_DONE, LPARAM lParam = 0 )
		{
			m_hwndNotifyDone = hwndNotify;
			m_msgNotifyDone = uMsg;
			m_paramNotifyDone = lParam;
		}

	// 音声データの長さ取得
	unsigned int GetWaveLength( void ) const
		{
			return	m_nWaveLength ;
		}
	unsigned int GetWaveSamples( void ) const
		{
			if ( m_pWaveFormat && m_pWaveFormat->nBlockAlign )
				return	m_nWaveLength / m_pWaveFormat->nBlockAlign ;
			else
				return	0 ;
		}
	// 音声データ取得
	const void * GetWaveBuffer( void ) const
		{
			return	m_pWaveBuffer ;
		}

	// スレッド排他アクセス
	void Lock( void ) const ;
	void Unlock( void ) const ;

} ;


/*****************************************************************************
						3D 音声効果オブジェクト
 ****************************************************************************/

class	E3DSoundEffect	: public	EWaveSound
{
public:
	// 構築関数
	E3DSoundEffect( EWaveOutDevice * pWaveDev ) ;
	// 消滅関数
	virtual ~E3DSoundEffect( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( E3DSoundEffect, EWaveSound )

protected:
	double		m_rVolume ;
	E3DVector	m_vPosition ;
	E3DVector	m_vMotion ;

public:
	// ボリューム設定
	void SetVolume( double rVolume ) ;
	// 位置を設定
	void SetPosition
		( const E3D_VECTOR & vPosition, bool fAutoMotion = true ) ;
	// 運動量を設定
	void SetMotion( const E3D_VECTOR & vMotion ) ;
	// 音量を反映する
	ESLError ApplyVolume( void ) ;

} ;


/*****************************************************************************
							MIO 再生オブジェクト
 ****************************************************************************/

class	MIOSoundStream	: public	EWaveSound
{
public:
	// 構築関数
	MIOSoundStream( void ) ;
	// 消滅関数
	virtual ~MIOSoundStream( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( MIOSoundStream, EWaveSound )

protected:
	MIODynamicPlayer	m_miodp ;			// MIO デコーダ
	ESLFileObject *		m_pfile ;			// ファイルイメージ

	ULONG				m_nRewoundSample ;	// 巻き戻し位置
	ULONG				m_nLoopEndSample ;	// ループ終端

	HANDLE				m_hPlayedEvent ;	// 再生が終了した
	ETagSortArray<ULONG_PTR,EStreamBuffer>
						m_itaWaveBuf ;		// 音声バッファ配列

protected:	// 再生コールバック
	// 音声データの再生が完了した
	virtual void OnEndPlaying
		( HWAVEBUF hWaveBuf, void * ptrBuffer, unsigned int nBufferLength ) ;
	// 音声出力デバイスが次の音声バッファを要求している
	virtual HWAVEBUF OnQueueNextBuffer( EWaveOutDevice * pWaveDev ) ;

public:
	// 再生モード
	enum	PlayMode
	{
		pmDynamicRead,
		pmDynamicPlay,
		pmStaticPlay
	} ;
	PlayMode			m_pmMode ;
	// ファイルを開く
	ESLError Open( ESLFileObject & file, PlayMode pmMode ) ;
	// ファイルを閉じる
	void Close( void ) ;
	// MIODynamicPlayer オブジェクト参照
	const MIODynamicPlayer & GetMIOPlayer( void ) const
		{
			return	m_miodp ;
		}
	// 遅延再生
	virtual ESLError PlayWithDelayFrom
		( unsigned int nIntroSamples = -1,
			bool fRepeat = FALSE, HANDLE hEvent = NULL,
			unsigned int nDelay = 33, unsigned int nStartPos = 0 ) ;
	// 再生開始
	virtual ESLError PlayFrom
		( unsigned int nStartPos = 0, unsigned int nPlayEnd = -1,
				bool fRepeat = false, unsigned int nRewindPos = -1 ) ;
	// 再生停止
	virtual ESLError StopWave( void ) ;
	// 音声データ関連付け
	virtual ESLError AttachWaveSound( const EWaveSound & wavbuf ) ;

public:
	// 巻き戻し位置取得
	ULONG GetRewoundPosition( void ) const
		{
			return	m_nRewoundSample ;
		}
	// ループ終端取得
	ULONG GetLoopEndPosition( void ) const
		{
			return	m_nLoopEndSample ;
		}

} ;



#endif
