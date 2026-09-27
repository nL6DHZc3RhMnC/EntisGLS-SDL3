
#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl_media.h>
#include <sakuragl/sgl_direct_sound_player.h>
#include <sakuragl/sgl_dshow_media_player.h>
#include <sakuragl/sgl2d/sgl_image_conversion.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// ウィンドウ UI スレッド関数呼び出し
//////////////////////////////////////////////////////////////////////////////

SSystem::SError SGLDirectShowMediaPlayer::UISyncProcedure::Wait( void )
{
	atomic_int_t	nRelock = m_pMutexUI->UnlockAll() ;
	SError			err = m_eventDone.Wait() ;
	m_pMutexUI->Relock( nRelock ) ;
	return	err ;
}

void SGLDirectShowMediaPlayer::UISyncProcedure::Run( void )
{
	m_errResult = (m_pPlayer->*m_pfnCall)() ;
}

void SGLDirectShowMediaPlayer::UISyncProcedure::Finalize( void )
{
	m_eventDone.SetSignal() ;
}


//////////////////////////////////////////////////////////////////////////////
// DirectShow メディアファイル再生インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::SGLDirectShowMediaPlayer,
		SGLMediaPlayerInterface, SGLDirectShowAudioPlayer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLDirectShowMediaPlayer::SGLDirectShowMediaPlayer( void )
{
	m_pBasicVideo = NULL ;
	m_pWindow = NULL ;
	m_pEnv = NULL ;
	m_pMutexUI = SSystem::g_mutexGlobal ;
	m_pListener = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLDirectShowMediaPlayer::~SGLDirectShowMediaPlayer( void )
{
	SGLDirectShowMediaPlayer::Close() ;
}

// ファイルを開く（UIスレッド専用）
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirectShowMediaPlayer::OpenOnUIThread( void )
{
	SGLError	err ;
	SString	strFilePath = m_strFilePath ;
	err = SGLDirectShowAudioPlayer::Open( strFilePath, 0, m_pEnv ) ;
	if ( err )
	{
		return	err ;
	}
	if ( m_pGraphBuilder != NULL )
	{
		m_pGraphBuilder->QueryInterface
			( IID_IBasicVideo, (void **) &m_pBasicVideo ) ;
	}
	return	sglErrSuccess ;
}

// 再生開始関数（UIスレッド専用）
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirectShowMediaPlayer::PlayOnUIThread( void )
{
	SetViewOnUIThread() ;
	SGLError	err = SGLDirectShowAudioPlayer::Play() ;
	if ( !err )
	{
		if ( !m_flagThreading )
		{
			BeginLoopingThread() ;
		}
	}
	return	err ;
}

// 表示位置設定関数（UIスレッド専用）
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirectShowMediaPlayer::SetViewOnUIThread( void )
{
	if ( m_pWindow != NULL )
	{
		OAHWND	hWnd = (OAHWND) m_pWindow->GetWindowHandle() ;
		m_pVideoWindow->put_Owner( hWnd ) ;
		m_pVideoWindow->put_MessageDrain( hWnd ) ;
		m_pVideoWindow->put_WindowStyle
				( WS_VISIBLE | WS_CHILD
					| WS_CLIPSIBLINGS | WS_CLIPCHILDREN ) ;
		//
		SGLImageRect	rctRender, rctDisplay ;
		SGLSize			sizeDisplay ;
		bool			fStretching = false ;
		if ( !m_pWindow->GetDisplaySize( sizeDisplay )
			&& !m_pWindow->GetInternalDisplayPosition( rctRender, rctDisplay ) )
		{
			fStretching = (rctDisplay.x != 0) || (rctDisplay.y != 0)
							|| (rctDisplay.w != sizeDisplay.w)
							|| (rctDisplay.h != sizeDisplay.h) ;
		}
		if ( fStretching )
		{
			if ( (sizeDisplay.w != 0) && (sizeDisplay.h != 0) )
			{
				m_pVideoWindow->SetWindowPosition
					( m_rectDstView.x + rctDisplay.x,
						m_rectDstView.y + rctDisplay.y,
						m_rectDstView.w * rctDisplay.w / sizeDisplay.w,
						m_rectDstView.h * rctDisplay.h / sizeDisplay.h ) ;
			}
		}
		else
		{
			m_pVideoWindow->SetWindowPosition
				( m_rectDstView.x, m_rectDstView.y,
					m_rectDstView.w, m_rectDstView.h ) ;
		}
		m_pVideoWindow->put_Visible( OATRUE ) ;
		m_pVideoWindow->put_WindowState( SW_SHOW ) ;
	}
	return	sglErrSuccess ;
}

// SGLWindowsAVIReader 取得
//////////////////////////////////////////////////////////////////////////////
SGLWindowsAVIReader * SGLDirectShowMediaPlayer::GetAVIMediaStream( void )
{
	if ( (m_pAVIReader == NULL) && !m_strFilePath.IsEmpty() )
	{
		m_pAVIReader = new SGLWindowsAVIReader ;
		if ( m_pAVIReader->Open( m_strFilePath ) )
		{
			m_pAVIReader = NULL ;
		}
	}
	return	m_pAVIReader ;
}

// 指定ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirectShowMediaPlayer::Open
	( const wchar_t * pwszFilePath,
		uint64_t nFlags, SSystem::SEnvironmentInterface * pEnv )
{
	m_strFilePath = pwszFilePath ;
	m_pEnv = pEnv ;
	//
	if ( m_pWindow != NULL )
	{
		UISyncProcedure	procOpen
			( this, &SGLDirectShowMediaPlayer::OpenOnUIThread, m_pMutexUI ) ;
		if ( m_pWindow->PostUIThread( &procOpen ) )
		{
			return	sglErrFailed ;
		}
		procOpen.Wait() ;
		return	procOpen.m_errResult ;
	}
	else
	{
		return	OpenOnUIThread() ;
	}
}

SGLError SGLDirectShowMediaPlayer::Create
	( SSystem::SFileInterface * file, bool flagOwner, uint64_t nFlags )
{
	return	SGLDirectShowAudioPlayer::Create( file, flagOwner, nFlags ) ;
}

// データを参照する複製プレイヤー生成
//////////////////////////////////////////////////////////////////////////////
SGLAudioPlayerInterface * SGLDirectShowMediaPlayer::ClonePlayer( void )
{
	SGLDirectShowMediaPlayer *	pPlayer = new SGLDirectShowMediaPlayer ;
	if ( !m_strFilePath.IsEmpty() )
	{
		pPlayer->Open( m_strFilePath ) ;
	}
	pPlayer->SetUIThreadMutex( m_pMutexUI ) ;
	return	(SGLMediaPlayerInterface*) pPlayer ;
}

// ファイルを閉じる
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirectShowMediaPlayer::Close( void )
{
	if ( m_pBasicVideo != NULL )
	{
		m_pBasicVideo->Release() ;
		m_pBasicVideo = NULL ;
	}
	return	SGLDirectShowAudioPlayer::Close() ;
}

// 再生を開始する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirectShowMediaPlayer::Play( uint64_t nFlags )
{
	if ( m_pWindow != NULL )
	{
		UISyncProcedure	procPlay
			( this, &SGLDirectShowMediaPlayer::PlayOnUIThread, m_pMutexUI ) ;
		if ( m_pWindow->PostUIThread( &procPlay ) )
		{
			return	sglErrFailed ;
		}
		procPlay.Wait() ;
		return	procPlay.m_errResult ;
	}
	else
	{
		return	SGLDirectShowAudioPlayer::Play( nFlags ) ;
	}
}

// 再生を停止する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirectShowMediaPlayer::Stop( void )
{
	return	SGLDirectShowAudioPlayer::Stop() ;
}

// ループポイント[/sample] を設定する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirectShowMediaPlayer::SetLoop
	( bool fLoop, int64_t nStart, int64_t nEnd )
{
	return	SGLDirectShowAudioPlayer::SetLoop( fLoop, nStart, nEnd ) ;
}

// 再生を一時停止する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirectShowMediaPlayer::Pause( void )
{
	return	SGLDirectShowAudioPlayer::Pause() ;
}

// 再生を再開する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirectShowMediaPlayer::Restart( void )
{
	return	SGLDirectShowAudioPlayer::Restart() ;
}

// 音量取得 [L/R]
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirectShowMediaPlayer::GetVolume( float32_t* pVolumes, size_t nChannels )
{
	return	SGLDirectShowAudioPlayer::GetVolume( pVolumes, nChannels ) ;
}

// 音量設定 [L/R]
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirectShowMediaPlayer::SetVolume( const float32_t* pVolumes, size_t nChannels )
{
	return	SGLDirectShowAudioPlayer::SetVolume( pVolumes, nChannels ) ;
}

// 再生中か？
//////////////////////////////////////////////////////////////////////////////
bool SGLDirectShowMediaPlayer::IsPlaying( void ) const
{
	return	SGLDirectShowAudioPlayer::IsPlaying() ;
}

// 一時停止中か？
//////////////////////////////////////////////////////////////////////////////
bool SGLDirectShowMediaPlayer::IsPaused( void ) const
{
	return	SGLDirectShowAudioPlayer::IsPaused() ;
}

// メディアのサンプル周波数を取得する
//////////////////////////////////////////////////////////////////////////////
uint32_t SGLDirectShowMediaPlayer::GetSampleFrequency( void ) const
{
	return	SGLDirectShowAudioPlayer::GetSampleFrequency() ;
}

// メディアの全長 [/sample] を取得する
//////////////////////////////////////////////////////////////////////////////
uint64_t SGLDirectShowMediaPlayer::GetTotalLength( void ) const
{
	return	SGLDirectShowAudioPlayer::GetTotalLength() ;
}

// 再生位置 [/sample] を取得する
//////////////////////////////////////////////////////////////////////////////
uint64_t SGLDirectShowMediaPlayer::GetPosition( void )
{
	return	SGLDirectShowAudioPlayer::GetPosition() ;
}

// 再生位置 [/sample] を変更する
//////////////////////////////////////////////////////////////////////////////
void SGLDirectShowMediaPlayer::SeekPosition( uint64_t nPos )
{
	SGLDirectShowAudioPlayer::SeekPosition( nPos ) ;
}

// オーディオストリーム取得
//////////////////////////////////////////////////////////////////////////////
SGLAudioInputStream * SGLDirectShowMediaPlayer::GetAudioStream( void )
{
	return	GetAVIMediaStream() ;
}

void SGLDirectShowMediaPlayer::ReleaseAudioStream( SGLAudioInputStream * pStream )
{
}

// スレッド同期用ミューテックス設定
//////////////////////////////////////////////////////////////////////////////
void SGLDirectShowMediaPlayer::SetUIThreadMutex( SSystem::SMutex * pMutex )
{
	SGLDirectShowAudioPlayer::SetUIThreadMutex( pMutex ) ;
}

// ビデオサイズを取得する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirectShowMediaPlayer::GetVideoSize( SGLSize& sizeVideo )
{
	if ( m_pBasicVideo == NULL )
	{
		return	sglErrFailed ;
	}
	long	nWidth = 0, nHeight = 0 ;
	m_pBasicVideo->get_VideoWidth( &nWidth ) ;
	m_pBasicVideo->get_VideoHeight( &nHeight ) ;
	sizeVideo.w = nWidth ;
	sizeVideo.h = nHeight ;
	return	sglErrSuccess ;
}

// 表示先を設定する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirectShowMediaPlayer::SetVideoView
	( SGLAbstractWindow* pWindow,
		const SGLImageRect& rectVideo, uint64_t nFlags )
{
	m_pMutexUI->LockTrace( __FILE__, __LINE__ ) ;
	m_pWindow = pWindow ;
	m_rectDstView = rectVideo ;
	m_pMutexUI->Unlock() ;
	//
	if ( m_pVideoWindow != NULL )
	{
		if ( pWindow != NULL )
		{
			UISyncProcedure	procSetView
				( this, &SGLDirectShowMediaPlayer::SetViewOnUIThread, m_pMutexUI ) ;
			if ( m_pWindow->PostUIThread( &procSetView ) )
			{
				return	sglErrFailed ;
			}
			procSetView.Wait() ;
			return	sglErrSuccess ;
		}
		else
		{
			m_pVideoWindow->put_Visible( OAFALSE ) ;
		}
	}
	return	sglErrSuccess ;
}

// 現在のフレームを描画する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDirectShowMediaPlayer::DrawVideo
	( SGLPaintContextInterface* pPaint,
		const SGLImageRect& rectDst, uint32_t nFlags, uint32_t nTransparency )
{
	if ( m_pVideoWindow != NULL )
	{
		if ( m_pWindow != NULL )
		{
			UISyncProcedure	procSetView
				( this, &SGLDirectShowMediaPlayer::SetViewOnUIThread, m_pMutexUI ) ;
			m_rectDstView = rectDst ;
			if ( m_pWindow->PostUIThread( &procSetView ) )
			{
				return	sglErrFailed ;
			}
			procSetView.Wait() ;
			return	sglErrSuccess ;
		}
	}
	return	sglErrFailed ;
}

// メディア再生通知リスナ設定
//////////////////////////////////////////////////////////////////////////////
SGLMediaPlayerFrameNotification *
	SGLDirectShowMediaPlayer::SetNotificationListener
			( SGLMediaPlayerFrameNotification * pListener )
{
	SGLMediaPlayerFrameNotification *	pLast = m_pListener ;
	m_pListener = pListener ;
	return	pLast ;
}

// ビデオストリーム取得
//////////////////////////////////////////////////////////////////////////////
SGLVideoInputStream * SGLDirectShowMediaPlayer::GetVideoStream( void )
{
	return	GetAVIMediaStream() ;
}

void SGLDirectShowMediaPlayer::ReleaseVideoStream( SGLVideoInputStream * pStream )
{
}

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
void SGLDirectShowMediaPlayer::Run( void )
{
	m_msecLastRepos = CurrentMilliSec() ;
	SGLDirectShowAudioPlayer::Run() ;
}

// ループ処理実装
//////////////////////////////////////////////////////////////////////////////
bool SGLDirectShowMediaPlayer::OnLoopingThread( void )
{
	int64_t	msecCurrent = CurrentMilliSec() ;
	if ( msecCurrent - m_msecLastRepos > 1000 )
	{
		if ( (m_pVideoWindow != NULL) && (m_pWindow != NULL) )
		{
			UISyncProcedure	procSetView
				( this, &SGLDirectShowMediaPlayer::SetViewOnUIThread, m_pMutexUI ) ;
			if ( !m_pWindow->PostUIThread( &procSetView ) )
			{
				procSetView.Wait() ;
			}
			m_msecLastRepos = CurrentMilliSec() ;
		}
	}
	if ( m_pListener != NULL )
	{
		m_pListener->OnFrameUpdate( this ) ;
	}
	return	SGLDirectShowAudioPlayer::OnLoopingThread() ;
}

// 終端到達
//////////////////////////////////////////////////////////////////////////////
void SGLDirectShowMediaPlayer::NotifyEndOfPlayingDuration( void )
{
	if ( m_pListener != NULL )
	{
		m_pListener->OnEndOfDuration( this ) ;
	}
}



//////////////////////////////////////////////////////////////////////////////
// DirectShow ファイル出力ピン
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLDSFileSource::FileOutPin::FileOutPin
	( const wchar_t * pwszName, SGLDSFileSource * pFilter )
: m_nRef( 1 ), m_strName( pwszName ),
	m_pFilter( pFilter ), m_fQueryAsyncReader( false ), m_pConnected( NULL )
{
	memset( &m_mtNull, 0, sizeof(m_mtNull) ) ;
	m_mtNull.majortype = MEDIATYPE_Stream ;
	m_mtNull.subtype = GUID_NULL ;
	m_mtNull.bFixedSizeSamples = TRUE ;
	m_mtNull.bTemporalCompression = FALSE ;
	m_mtNull.lSampleSize = 1 ;
	m_mtNull.formattype = GUID_NULL ;
	m_mtNull.pUnk = NULL ;
	m_mtNull.cbFormat = 0 ;
	m_mtNull.pbFormat = NULL ;
	//
	m_fConnectMedia = false ;
	memset( &m_mtConnectMedia, 0, sizeof(m_mtConnectMedia) ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLDSFileSource::FileOutPin::~FileOutPin( void )
{
	for ( size_t i = 0; i < m_lstStreamTypes.GetLength(); i ++ )
	{
		AM_MEDIA_TYPE *	pmt = m_lstStreamTypes.GetAt( i ) ;
		if ( pmt != NULL )
		{
			SGLDSRenderMediaPlayer::FreeMediaType( *pmt ) ;
		}
	}
	SGLDSRenderMediaPlayer::FreeMediaType( m_mtNull ) ;
	SGLDSRenderMediaPlayer::FreeMediaType( m_mtConnectMedia ) ;
}

// 接続試行
//////////////////////////////////////////////////////////////////////////////
HRESULT SGLDSFileSource::FileOutPin::AttemptConnection
	( IPin * pReceivePin, const AM_MEDIA_TYPE * pmt )
{
    PIN_DIRECTION	pd ;
    pReceivePin->QueryDirection( &pd ) ;
	if ( pd == PINDIR_OUTPUT )
	{
		return	VFW_E_INVALID_DIRECTION ;
	}
	//
	m_pConnected = pReceivePin ;
	pReceivePin->AddRef() ;
	//
	HRESULT	hr = SetMediaType( pmt ) ;
	if ( SUCCEEDED( hr ) )
	{
		m_fQueryAsyncReader = false ;
		hr = pReceivePin->ReceiveConnection( this, pmt ) ;
		if ( SUCCEEDED( hr ) )
		{
			if ( m_fQueryAsyncReader )
			{
				return	hr ;
			}
			pReceivePin->Disconnect() ;
			hr = E_FAIL ;
		}
	}
	if ( m_pConnected != NULL )
	{
		m_pConnected->Release() ;
		m_pConnected = NULL ;
	}
	return	hr ;
}

HRESULT SGLDSFileSource::FileOutPin::TryMediaTypes
	( IPin * pReceivePin,
		__in_opt const AM_MEDIA_TYPE * pmt, IEnumMediaTypes *pEnum )
{
	HRESULT	hr = pEnum->Reset() ;
	if ( FAILED(hr) )
	{
		return	hr ;
	}
	//
	AM_MEDIA_TYPE *	pmType = NULL ;
	ULONG			nCount ;
	HRESULT			hrErr = S_OK ;
	//
	for ( ; ; )
	{
		hr = pEnum->Next( 1, &pmType, &nCount ) ;
		if ( hr != S_OK )
		{
			if ( hrErr == S_OK )
			{
				hrErr = VFW_E_NO_ACCEPTABLE_TYPES ;
			}
			return	hrErr ;
		}
		if ( (pmType != NULL)
			&& ((pmt == NULL)
				|| (((pmt->majortype == GUID_NULL)
						|| (pmt->majortype == pmType->majortype))
					&& ((pmt->subtype == GUID_NULL)
						|| (pmt->subtype == pmType->subtype))) ) )
		{
			hr = AttemptConnection( pReceivePin, pmType ) ;
			if ( FAILED(hr)
				&& SUCCEEDED(hrErr)
				&& (hr != E_FAIL)
				&& (hr != E_INVALIDARG)
				&& (hr != VFW_E_TYPE_NOT_ACCEPTED) )
			{
				hrErr = hr ;
			}
		}
		else
		{
			hr = VFW_E_NO_ACCEPTABLE_TYPES ;
		}
		if ( pmType != NULL )
		{
			SGLDSRenderMediaPlayer::DeleteMediaType( pmType ) ;
			pmType = NULL ;
		}
		if ( hr == S_OK )
		{
			return	hr ;
		}
	}
}

// メディアタイプ取得
//////////////////////////////////////////////////////////////////////////////
const AM_MEDIA_TYPE * SGLDSFileSource::FileOutPin::GetMediaTypeAt( size_t i ) const
{
	return	m_lstStreamTypes.GetAt( i ) ;
}

size_t SGLDSFileSource::FileOutPin::GetMediaTypeCount( void ) const
{
	return	m_lstStreamTypes.GetLength() ;
}

// メディアタイプ設定
//////////////////////////////////////////////////////////////////////////////
HRESULT SGLDSFileSource::FileOutPin::SetMediaType( const AM_MEDIA_TYPE * pmt )
{
	SGLDSRenderMediaPlayer::FreeMediaType( m_mtConnectMedia ) ;
	//
	HRESULT	hr =
		SGLDSRenderMediaPlayer::CopyMediaType( &m_mtConnectMedia, pmt ) ;
	if ( FAILED( hr ) )
	{
		m_fConnectMedia = false ;
		return	hr ;
	}
	m_fConnectMedia = true ;
	return	hr ;
}

// IUnknown
//////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE SGLDSFileSource::FileOutPin::QueryInterface( REFIID riid, void ** ppObj )
{
    if ( IsEqualIID(riid, IID_IPin) )
	{
        *ppObj = (IPin*) this ;
	}
	else if ( IsEqualIID(riid, IID_IAMAsyncReaderTimestampScaling) )
	{
		IAMAsyncReaderTimestampScaling *
					parts = m_pFilter->GetTimestampScaling() ;
        *ppObj = parts ;
		parts->AddRef() ;
		return	S_OK ;
	}
	else if ( IsEqualIID(riid, IID_IAsyncReader) )
	{
		IAsyncReader *	pReader = m_pFilter->GetAsyncReader() ;
		m_fQueryAsyncReader = true ;
        *ppObj = pReader ;
		pReader->AddRef() ;
		return	S_OK ;
	}
	else if ( IsEqualIID(riid, IID_IUnknown) )
	{
        *ppObj = (IUnknown*) this ;
	}
    else
    {
        *ppObj = NULL ;
        return	E_NOINTERFACE ;
    }
    AddRef() ;
    return	S_OK ;
}

ULONG STDMETHODCALLTYPE SGLDSFileSource::FileOutPin::AddRef( void )
{
	return	AtomicAdd( &m_nRef, 1 ) ;
}

ULONG STDMETHODCALLTYPE SGLDSFileSource::FileOutPin::Release( void )
{
	ULONG	nRef = (ULONG) AtomicSub( &m_nRef, 1 ) ;
	if ( nRef == 0 )
	{
		delete	this ;
	}
	return	nRef ;
}

// IPin
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP SGLDSFileSource::FileOutPin::Connect
	( IPin * pReceivePin, __in_opt const AM_MEDIA_TYPE * pmt )
{
	if ( pReceivePin == NULL )
	{
		return	E_POINTER ;
	}
	SSmartLock<SCriticalSection>	lock( &(m_pFilter->m_csSync) ) ;
	if ( m_pConnected != NULL )
	{
		return	VFW_E_ALREADY_CONNECTED ;
	}
	if ( !IsStopped() )
	{
		return	VFW_E_NOT_STOPPED ;
	}
	if ( (pmt != NULL)
		&& !SGLDSRenderMediaPlayer::MediaTypeIsPartiallySpecified( pmt ) )
	{
		return	AttemptConnection( pReceivePin, pmt ) ;
	}
	HRESULT	hrErr = VFW_E_NO_ACCEPTABLE_TYPES ;
	for ( int i = 0; i < 2; i ++ )
	{
		IEnumMediaTypes *	pemtEnum = NULL ;
		HRESULT	hr ;
		if ( i == 0 )
		{
			hr = pReceivePin->EnumMediaTypes( &pemtEnum ) ;
		}
		else
		{
			hr = EnumMediaTypes( &pemtEnum ) ;
		}
		if ( SUCCEEDED(hr) )
		{
			hr = TryMediaTypes( pReceivePin, pmt, pemtEnum ) ;
			pemtEnum->Release() ;
			//
			if ( SUCCEEDED(hr) )
			{
				return	NOERROR ;
			}
			else
			{
				if ( (hr != E_FAIL)
					&& (hr != E_INVALIDARG)
					&& (hr != VFW_E_TYPE_NOT_ACCEPTED) )
				{
					hrErr = hr ;
				}
			}
		}
	}
	return	hrErr ;
}

STDMETHODIMP SGLDSFileSource::FileOutPin::ReceiveConnection
	( IPin * pConnector, const AM_MEDIA_TYPE * pmt )
{
	return	E_NOTIMPL ;
}

STDMETHODIMP SGLDSFileSource::FileOutPin::Disconnect( void )
{
	SSmartLock<SCriticalSection>	lock( &(m_pFilter->m_csSync) ) ;
	if ( !IsStopped() )
	{
		return	VFW_E_NOT_STOPPED ;
	}
	m_fConnectMedia = false ;
	if ( m_pConnected != NULL )
	{
		m_pConnected->Release() ;
		m_pConnected = NULL ;
		//
		return	S_OK ;
	}
	return	S_FALSE ;
}

STDMETHODIMP SGLDSFileSource::FileOutPin::ConnectedTo( __deref_out IPin ** ppPin )
{
	if ( ppPin == NULL )
	{
		return	E_POINTER ;
	}
	IPin *	pPin = m_pConnected ;
	*ppPin = pPin ;
	if ( pPin != NULL )
	{
		pPin->AddRef() ;
		return	S_OK ;
	}
	else
	{
		return	VFW_E_NOT_CONNECTED ;
	}
}

STDMETHODIMP SGLDSFileSource::FileOutPin::ConnectionMediaType( __out AM_MEDIA_TYPE * pmt )
{
	if ( pmt == NULL )
	{
		return	E_POINTER ;
	}
	SSmartLock<SCriticalSection>	lock( &(m_pFilter->m_csSync) ) ;
	if ( IsConnected() && m_fConnectMedia )
	{
		SGLDSRenderMediaPlayer::CopyMediaType( pmt, &m_mtConnectMedia ) ;
		return	S_OK ;
	}
	else
	{
		return	VFW_E_NOT_CONNECTED ;
	}
}

STDMETHODIMP SGLDSFileSource::FileOutPin::QueryPinInfo( __out PIN_INFO * pInfo )
{
	if ( pInfo == NULL )
	{
		return	E_POINTER ;
	}
	pInfo->pFilter = m_pFilter ;
	if ( m_pFilter != NULL )
	{
		m_pFilter->AddRef() ;
	}
	if ( !m_strName.IsEmpty() )
	{
        StringCchCopyW( pInfo->achName, NUMELMS(pInfo->achName), m_strName ) ;
	}
	else
	{
        pInfo->achName[0] = 0 ;
	}
    pInfo->dir = PINDIR_OUTPUT ;
	return	NOERROR ;
}

STDMETHODIMP SGLDSFileSource::FileOutPin::QueryDirection( __out PIN_DIRECTION * pPinDir )
{
	if ( pPinDir == NULL )
	{
		return	E_POINTER ;
	}
	*pPinDir = PINDIR_OUTPUT ;
	return	NOERROR ;
}

STDMETHODIMP SGLDSFileSource::FileOutPin::QueryId( __deref_out LPWSTR * Id )
{
	size_t	nBytes = (m_strName.GetLength() + 1) * sizeof(wchar_t) ;
	*Id = (LPWSTR) CoTaskMemAlloc( nBytes ) ;
	if ( *Id == NULL )
	{
		return	E_OUTOFMEMORY ;
	}
    memmove( *Id, (const wchar_t*) m_strName, nBytes ) ;
	return	NOERROR ;
}

STDMETHODIMP SGLDSFileSource::FileOutPin::QueryAccept( const AM_MEDIA_TYPE * pmt )
{
	return	E_NOTIMPL ;
}

STDMETHODIMP SGLDSFileSource::FileOutPin::EnumMediaTypes
		( __deref_out IEnumMediaTypes ** ppEnum )
{
	if ( ppEnum == NULL )
	{
		return	E_POINTER ;
	}
	*ppEnum = new SGLDSFileSource::EnumMediaTypes( this ) ;
	return	NOERROR ;
}

STDMETHODIMP SGLDSFileSource::FileOutPin::QueryInternalConnections
	( __out_ecount_part(*nPin,*nPin) IPin* *apPin, __inout ULONG *nPin )
{
	return	E_NOTIMPL ;
}

STDMETHODIMP SGLDSFileSource::FileOutPin::EndOfStream( void )
{
	return	E_UNEXPECTED ;
}

STDMETHODIMP SGLDSFileSource::FileOutPin::BeginFlush( void )
{
	return	E_UNEXPECTED ;
}

STDMETHODIMP SGLDSFileSource::FileOutPin::EndFlush( void )
{
	return	E_UNEXPECTED ;
}

STDMETHODIMP SGLDSFileSource::FileOutPin::NewSegment
	( REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate )
{
	return	E_UNEXPECTED ;
}



//////////////////////////////////////////////////////////////////////////////
// メディアタイプ列挙
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLDSFileSource::EnumMediaTypes::EnumMediaTypes( FileOutPin * pPin )
{
	m_nRef = 1 ;
	m_pPin = pPin ;
	m_iPos = 0 ;
}

// IUnknown
//////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE SGLDSFileSource::EnumMediaTypes::QueryInterface( REFIID riid, void ** ppObj )
{
    if ( IsEqualIID(riid, IID_IUnknown)
		|| IsEqualIID(riid, IID_IEnumMediaTypes) )
	{
        *ppObj = (IEnumMediaTypes*) this ;
	}
    else
    {
        *ppObj = NULL ;
        return	E_NOINTERFACE ;
    }
    AddRef() ;
    return	S_OK ;
}

ULONG STDMETHODCALLTYPE SGLDSFileSource::EnumMediaTypes::AddRef( void )
{
	return	AtomicAdd( &m_nRef, 1 ) ;
}

ULONG STDMETHODCALLTYPE SGLDSFileSource::EnumMediaTypes::Release( void )
{
	ULONG	nRef = (ULONG) AtomicSub( &m_nRef, 1 ) ;
	if ( nRef == 0 )
	{
		delete	this ;
	}
	return	nRef ;
}

// IEnumMediaTypes
//////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE SGLDSFileSource::EnumMediaTypes::Next
	( ULONG cMediaTypes,
		__out_ecount(cMediaTypes) AM_MEDIA_TYPE ** ppMediaTypes,
		__out_opt ULONG * pcFetched )
{
	size_t	nCount = m_pPin->GetMediaTypeCount() ;
	if ( ppMediaTypes == NULL )
	{
		return	E_POINTER ;
	}
	if ( pcFetched != NULL )
	{
		*pcFetched = 0 ;
	}
	else if ( m_iPos + cMediaTypes > nCount )
	{
		return	E_INVALIDARG ;
	}
	ULONG	nFetched = 0 ;
	for ( ULONG i = 0; i < cMediaTypes; i ++ )
	{
		const AM_MEDIA_TYPE *
				pmtSrc = m_pPin->GetMediaTypeAt( m_iPos ++ ) ;
		if ( pmtSrc == NULL )
		{
			break ;
		}
		//
		AM_MEDIA_TYPE *	pmt =
			(AM_MEDIA_TYPE*) CoTaskMemAlloc( sizeof(AM_MEDIA_TYPE) ) ;
		if ( pmt == NULL )
		{
			break ;
		}
		SGLDSRenderMediaPlayer::CopyMediaType( pmt, pmtSrc ) ;
		//
		ppMediaTypes[i] = pmt ;
		nFetched ++ ;
	}
	if ( pcFetched != NULL )
	{
		*pcFetched = nFetched ;
	}
	return	(nFetched == cMediaTypes) ? NOERROR : S_FALSE ;
}

HRESULT STDMETHODCALLTYPE SGLDSFileSource::EnumMediaTypes::Skip( ULONG cMediaTypes )
{
	m_iPos += cMediaTypes ;
	return	(m_iPos <= m_pPin->GetMediaTypeCount()) ? S_OK : S_FALSE ;
}

HRESULT STDMETHODCALLTYPE SGLDSFileSource::EnumMediaTypes::Reset( void )
{
	m_iPos = 0 ;
	return	NOERROR ;
}

HRESULT STDMETHODCALLTYPE SGLDSFileSource::EnumMediaTypes::Clone
	( __deref_out IEnumMediaTypes **ppEnum )
{
	if ( ppEnum == NULL )
	{
		return	E_POINTER ;
	}
	*ppEnum = new EnumMediaTypes( m_pPin ) ;
	return	NOERROR ;
}


//////////////////////////////////////////////////////////////////////////////
// Pin 列挙
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLDSFileSource::EnumOutPins::EnumOutPins( SGLDSFileSource * pFilter )
{
	m_nRef = 1 ;
	m_pFilter = pFilter ;
	m_iPos = 0 ;
	//
	pFilter->AddRef() ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLDSFileSource::EnumOutPins::~EnumOutPins( void )
{
	m_pFilter->Release() ;
}

// IUnknown
//////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE SGLDSFileSource::EnumOutPins::QueryInterface( REFIID riid, void ** ppObj )
{
    if ( IsEqualIID(riid, IID_IUnknown)
		|| IsEqualIID(riid, IID_IEnumPins) )
	{
        *ppObj = (IEnumPins*) this ;
	}
    else
    {
        *ppObj = NULL ;
        return	E_NOINTERFACE ;
    }
    AddRef() ;
    return	S_OK ;
}

ULONG STDMETHODCALLTYPE SGLDSFileSource::EnumOutPins::AddRef( void )
{
	return	AtomicAdd( &m_nRef, 1 ) ;
}

ULONG STDMETHODCALLTYPE SGLDSFileSource::EnumOutPins::Release( void )
{
	ULONG	nRef = (ULONG) AtomicSub( &m_nRef, 1 ) ;
	if ( nRef == 0 )
	{
		delete	this ;
	}
	return	nRef ;
}

// IEnumPins
//////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE SGLDSFileSource::EnumOutPins::Next
	( ULONG cPins, __out_ecount_part(cPins, *pcFetched) IPin ** ppPins, __out_opt ULONG * pcFetched )
{
	if ( ppPins == NULL )
	{
		return	E_POINTER ;
	}
	if ( pcFetched != NULL )
	{
		*pcFetched = 0 ;
	}
	else if ( cPins > 1 )
	{
		return	E_INVALIDARG ;
	}
	if ( m_iPos ++ != 0 )
	{
		return	S_FALSE ;
	}
	FileOutPin *	pPin = m_pFilter->GetOutputPin() ;
	if ( pPin != NULL )
	{
		pPin->AddRef() ;
	}
	ppPins[0] = pPin ;
	//
	if ( pcFetched != NULL )
	{
		*pcFetched = 1 ;
	}
	return	(cPins == 1) ? NOERROR : S_FALSE ;
}

HRESULT STDMETHODCALLTYPE SGLDSFileSource::EnumOutPins::Skip( ULONG cPins )
{
	if ( cPins == 0 )
	{
		return	S_OK ;
	}
	m_iPos += cPins ;
	return	(m_iPos <= 1) ? S_OK : S_FALSE ;
}

HRESULT STDMETHODCALLTYPE SGLDSFileSource::EnumOutPins::Reset( void )
{
	m_iPos = 0 ;
	return	NOERROR ;
}

HRESULT STDMETHODCALLTYPE SGLDSFileSource::EnumOutPins::Clone( __out IEnumPins ** ppEnum )
{
	if ( ppEnum == NULL )
	{
		return	E_POINTER ;
	}
	*ppEnum = new EnumOutPins( m_pFilter ) ;
	return	NOERROR ;
}



//////////////////////////////////////////////////////////////////////////////
// DirectShow ファイル入力フィルタ
//////////////////////////////////////////////////////////////////////////////

// {A117CA82-DF84-45CE-A922-3F013D94C723}
const GUID	SGLDSFileSource::CLSID_SGLDSFileSource =
{ 0xa117ca82, 0xdf84, 0x45ce, { 0xa9, 0x22, 0x3f, 0x1, 0x3d, 0x94, 0xc7, 0x23 } } ;

const GUID	SGLDSFileSource::IID_IAMAsyncReaderTimestampScaling =
{ 0xcf7b26fc, 0x9a00, 0x485b, { 0x81, 0x47, 0x3e, 0x78, 0x9d, 0x5e, 0x8f, 0x67 } } ;

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLDSFileSource::SGLDSFileSource
	( const wchar_t * pwszName,
		SSystem::SFileInterface * pFile, bool flagOwnFile )
: m_nRef( 1 ), m_strName( pwszName )
{
	m_state = State_Stopped ;
	m_fTimestampRaw = FALSE ;
	m_pPin = new FileOutPin( L"Out", this ) ;
	m_pClock = NULL ;
	m_pGraph = NULL ;
	//
	m_sigFlushing.Initialize( false ) ;
	m_sigReqQueue.Initialize( false ) ;
	m_sigDoneQueue.Initialize( false ) ;
	//
	m_procAsync.AttachFilter( this ) ;
	m_sigExitThread.Initialize( false ) ;
	//
	m_pFile = pFile ;
	m_fOwnFile = flagOwnFile ;
	//
	SArray<uint8_t>	bufFileHeader ;
	pFile->Seek( 0 ) ;
	pFile->Read( bufFileHeader.GetArray(0x100), 0x100 ) ;
	pFile->Seek( 0 ) ;
	bufFileHeader.FinishArray() ;
	//
	SearchMatchMediaType( bufFileHeader ) ;
	//
	AM_MEDIA_TYPE *	pmtNull = new AM_MEDIA_TYPE ;
	SGLDSRenderMediaPlayer::CopyMediaType( pmtNull, &(m_pPin->m_mtNull) ) ;
	m_pPin->m_lstStreamTypes.Add( pmtNull ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLDSFileSource::~SGLDSFileSource( void )
{
	if ( m_pPin != NULL )
	{
		delete	m_pPin ;
		m_pPin = NULL ;
	}
	if ( m_pClock != NULL )
	{
		m_pClock->Release() ;
		m_pClock = NULL ;
	}
	m_sigExitThread.SetSignal() ;
	if ( m_threadAsync.IsRunning() )
	{
		m_threadAsync.Wait() ;
	}
	if ( m_fOwnFile )
	{
		delete	m_pFile ;
		m_fOwnFile = false ;
	}
	m_pFile = NULL ;
}

// メディアタイプをレジストリから検索する
//////////////////////////////////////////////////////////////////////////////
bool SGLDSFileSource::SearchMatchMediaType
	( const SSystem::SArray<uint8_t>& bufFileHeader )
{
	//
	// HKEY_CLASSES_ROOT\Media Type\{MEDIATYPE_Stream} を開く
	//
	wchar_t	wszStreamGuid[0x100] ;
	::StringFromGUID2( MEDIATYPE_Stream, wszStreamGuid, 0xFF ) ;
	//
	SString	strStreamPath = L"Media Type\\" ;
	strStreamPath += wszStreamGuid ;
	//
	SRegistryKey	keyStream ;
	if ( keyStream.OpenKey( HKEY_CLASSES_ROOT, strStreamPath ) )
	{
		return	false ;
	}
	//
	// サブタイプ列挙
	//
	SObjectArray<SString>	lstSubNames ;
	keyStream.EnumerateSubKeys( lstSubNames ) ;
	//
	bool	fMatched = false ;
	for ( size_t i = 0; i < lstSubNames.GetLength(); i ++ )
	{
		const SString *	pstrName = lstSubNames.GetAt( i ) ;
		ESLAssert( pstrName != NULL ) ;
		if ( pstrName == NULL )
		{
			continue ;
		}
		CLSID	clsidSubType ;
		if ( ::CLSIDFromString( *pstrName, &clsidSubType ) != NOERROR )
		{
			continue ;
		}
		SRegistryKey	keySubType ;
		if ( keySubType.OpenKey( keyStream, *pstrName, KEY_READ ) )
		{
			continue ;
		}
		if ( IsMatchStreamMediaType( keySubType, bufFileHeader ) )
		{
			#if	defined(__DEBUG__)
			wchar_t	wszSubtypeGuid[0x100] ;
			::StringFromGUID2( clsidSubType, wszSubtypeGuid, 0xFF ) ;
			SString	strSubtypeGuid = wszSubtypeGuid ;
			ESLTrace( "found stream subtype = %s\n",
							strSubtypeGuid.ToCharArray().GetConstArray() ) ;
			#endif
			//
			AM_MEDIA_TYPE *	pmtType = new AM_MEDIA_TYPE ;
			memset( pmtType, 0, sizeof(AM_MEDIA_TYPE) ) ;
			pmtType->majortype = MEDIATYPE_Stream ;
			pmtType->subtype = clsidSubType ;
			pmtType->bFixedSizeSamples = TRUE ;
			pmtType->bTemporalCompression = FALSE ;
			pmtType->lSampleSize = 1 ;
			pmtType->formattype = GUID_NULL ;
			pmtType->pUnk = NULL ;
			pmtType->cbFormat = 0 ;
			pmtType->pbFormat = NULL ;
			m_pPin->m_lstStreamTypes.Add( pmtType ) ;
			fMatched = true ;
		}
	}
	return	fMatched ;
}

bool SGLDSFileSource::IsMatchStreamMediaType
	( SRegistryKey& keySubType,
		const SSystem::SArray<uint8_t>& bufFileHeader )
{
	SObjectArray<SString>	lstValueNames ;
	keySubType.EnumerateValueNames( lstValueNames ) ;
	//
	for ( size_t i = 0; i < lstValueNames.GetLength(); i ++ )
	{
		const SString *	pstrName = lstValueNames.GetAt( i ) ;
		ESLAssert( pstrName != NULL ) ;
		if ( (pstrName == NULL)
			|| (pstrName->CompareNoCase( L"Source Filter" ) == 0) )
		{
			continue ;
		}
		SString	strBinForm = keySubType.GetString( *pstrName ) ;
		if ( strBinForm.IsEmpty() )
		{
			continue ;
		}
		SStringParser	sparsForm ;
		sparsForm.AttachString( strBinForm ) ;
		//
		bool	flagMatch = false ;
		while ( sparsForm.PassSpace() )
		{
			StreamBinaryFormat	sbf ;
			if ( sbf.ParseFormat( sparsForm )
				&& sbf.IsMatch( bufFileHeader ) )
			{
				flagMatch = true ;
				sparsForm.HasToComeChar( L"," ) ;
			}
			else
			{
				flagMatch = false ;
				break ;
			}
		}
		if ( flagMatch )
		{
			return	true ;
		}
	}
	return	false ;
}

// フォーマット解釈
//////////////////////////////////////////////////////////////////////////////
bool SGLDSFileSource::StreamBinaryFormat::ParseFormat( SStringParser& sparsForm )
{
	int	type ;
	type = sparsForm.IsNextNumber() ;
	if ( type == SStringParser::numberInvalid )
	{
		return	false ;
	}
	m_nOffset = (size_t) sparsForm.NextInteger( type ) ;
	//
	if ( sparsForm.HasToComeChar( L"," ) != L',' )
	{
		return	false ;
	}
	type = sparsForm.IsNextNumber() ;
	if ( type == SStringParser::numberInvalid )
	{
		return	false ;
	}
	size_t	nLength = (size_t) sparsForm.NextInteger( type ) ;
	if ( nLength == 0 )
	{
		return	false ;
	}
	if ( sparsForm.HasToComeChar( L"," ) != L',' )
	{
		return	false ;
	}
	if ( sparsForm.HasToComeChar( L"," ) != L',' )
	{
		if ( !ParseBinary( sparsForm, m_bufMask, nLength ) )
		{
			return	false ;
		}
		if ( sparsForm.HasToComeChar( L"," ) != L',' )
		{
			return	false ;
		}
	}
	else
	{
		for ( size_t i = 0; i < nLength; i ++ )
		{
			m_bufMask.Add( 0xFF ) ;
		}
	}
	//
	if ( !ParseBinary( sparsForm, m_bufValue, nLength ) )
	{
		return	false ;
	}
	return	true ;
}

bool SGLDSFileSource::StreamBinaryFormat::ParseBinary
	( SStringParser& sparsForm,
		SSystem::SArray<uint8_t>& bufBinary, size_t nLength )
{
	sparsForm.PassSpace() ;
	//
	for ( size_t i = 0; i < nLength; i ++ )
	{
		wchar_t	wchH = sparsForm.GetCharacter() ;
		wchar_t	wchL = sparsForm.GetCharacter() ;
		uint8_t	v = 0 ;
		if ( (wchH >= L'0') && (wchH <= L'9') )
		{
			v = (uint8_t) (wchH - L'0') ;
		}
		else if ( (wchH >= L'A') && (wchH <= L'F') )
		{
			v = (uint8_t) (wchH - L'A' + 10) ;
		}
		else if ( (wchH >= L'a') && (wchH <= L'f') )
		{
			v = (uint8_t) (wchH - L'a' + 10) ;
		}
		else
		{
			return	false ;
		}
		v <<= 4 ;
		if ( (wchL >= L'0') && (wchL <= L'9') )
		{
			v |= (uint8_t) (wchL - L'0') ;
		}
		else if ( (wchL >= L'A') && (wchL <= L'F') )
		{
			v |= (uint8_t) (wchL - L'A' + 10) ;
		}
		else if ( (wchL >= L'a') && (wchL <= L'f') )
		{
			v |= (uint8_t) (wchL - L'a' + 10) ;
		}
		else
		{
			return	false ;
		}
		bufBinary.Add( v ) ;
	}
	return	true ;
}

// マッチング
//////////////////////////////////////////////////////////////////////////////
bool SGLDSFileSource::StreamBinaryFormat::IsMatch( const SSystem::SArray<uint8_t>& bufStream )
{
	ESLAssert( m_bufMask.GetLength() == m_bufValue.GetLength() ) ;
	for ( size_t i = 0; i < m_bufValue.GetLength(); i ++ )
	{
		if ( m_nOffset + i >= bufStream.GetLength() )
		{
			return	false ;
		}
		uint8_t	strm = bufStream.At( m_nOffset + i ) ;
		uint8_t	mask = m_bufMask.At( i ) ;
		uint8_t	form = m_bufValue.At( i ) ;
		if ( (strm & mask) != (form & mask) )
		{
			return	false ;
		}
	}
	return	true ;
}

// IAMAsyncReaderTimestampScaling 取得
//////////////////////////////////////////////////////////////////////////////
IAMAsyncReaderTimestampScaling * SGLDSFileSource::GetTimestampScaling( void )
{
	return	this ;
}

// IAsyncReader 取得
//////////////////////////////////////////////////////////////////////////////
IAsyncReader * SGLDSFileSource::GetAsyncReader( void )
{
	return	this ;
}

// IPin 取得
//////////////////////////////////////////////////////////////////////////////
SGLDSFileSource::FileOutPin * SGLDSFileSource::GetOutputPin( void )
{
	return	m_pPin ;
}

// 非同期読み込み
//////////////////////////////////////////////////////////////////////////////
void SGLDSFileSource::ThreadAsyncReaderProc( void )
{
	for ( ; ; )
	{
		HANDLE	hEvents[2] ;
		hEvents[0] = m_sigExitThread.GetHandle() ;
		hEvents[1] = m_sigReqQueue.GetHandle() ;
		//
		DWORD	dwWaitResult =
			::WaitForMultipleObjects( 2, &hEvents[0], FALSE, 10 ) ;
		if ( dwWaitResult == WAIT_OBJECT_0 )
		{
			break ;
		}
		if ( dwWaitResult == WAIT_OBJECT_0 + 1 )
		{
			RequestEntry *	pre = NULL ;
			{
				SSmartLock<SCriticalSection>	lock( &m_csQueue ) ;
				pre = m_queRequests.DetachAt( 0 ) ;
				if ( m_queRequests.GetLength() == 0 )
				{
					m_sigReqQueue.ResetSignal() ;
				}
			}
			if ( pre != NULL )
			{
				pre->hrRead = SyncReadAligned( pre->pSample ) ;
				//
				SSmartLock<SCriticalSection>	lock( &m_csQueue ) ;
				m_queDones.Add( pre ) ;
				m_sigDoneQueue.SetSignal() ;
			}
		}
	}
}

// IUnknown
//////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE SGLDSFileSource::QueryInterface( REFIID riid, void ** ppObj )
{
	if ( IsEqualIID(riid, IID_IBaseFilter) )
	{
        *ppObj = (IBaseFilter*) this ;
	}
	else if ( IsEqualIID(riid, IID_IMediaFilter) )
	{
        *ppObj = (IMediaFilter*) this ;
	}
	else if ( IsEqualIID(riid, IID_IPersist) )
	{
        *ppObj = (IPersist*) this ;
	}
	else if ( IsEqualIID(riid, IID_IAMAsyncReaderTimestampScaling) )
	{
        *ppObj = (IAMAsyncReaderTimestampScaling*) this ;
	}
	else if ( IsEqualIID(riid, IID_IAsyncReader) )
	{
		*ppObj = (IAsyncReader*) this ;
		m_pPin->m_fQueryAsyncReader = true ;
	}
	else if ( IsEqualIID(riid, IID_IPin) )
	{
		IPin *	pPin = GetOutputPin() ;
		*ppObj = pPin ;
		pPin->AddRef() ;
		return	S_OK ;
	}
	else if ( IsEqualIID(riid, IID_IUnknown) )
	{
		*ppObj = (IBaseFilter*) this ;
	}
	else
	{
        *ppObj = NULL ;
        return	E_NOINTERFACE ;
	}
    AddRef() ;
    return	S_OK ;
}

ULONG STDMETHODCALLTYPE SGLDSFileSource::AddRef( void )
{
	return	AtomicAdd( &m_nRef, 1 ) ;
}

ULONG STDMETHODCALLTYPE SGLDSFileSource::Release( void )
{
	ULONG	nRef = (ULONG) AtomicSub( &m_nRef, 1 ) ;
	if ( nRef == 0 )
	{
		delete	this ;
	}
	return	nRef ;
}

// IPersist
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP SGLDSFileSource::GetClassID( __out CLSID *pClsID )
{
    if ( pClsID == NULL )
	{
		return	E_POINTER ;
	}
    *pClsID = CLSID_SGLDSFileSource ;
    return	NOERROR ;
}

// IMediaFilter
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP SGLDSFileSource::GetState( DWORD dwMSecs, __out FILTER_STATE * pState )
{
	if ( pState == NULL )
	{
		return	E_POINTER ;
	}
	*pState = m_state ;
    return	NOERROR ;
}

STDMETHODIMP SGLDSFileSource::SetSyncSource( __in_opt IReferenceClock *pClock )
{
	SSmartLock<SCriticalSection>	lock( &m_csObj ) ;
	if ( pClock != NULL)
	{
		pClock->AddRef() ;
	}
	if ( m_pClock != NULL )
	{
		m_pClock->Release() ;
	}
	m_pClock = pClock ;
	return	NOERROR ;
}

STDMETHODIMP SGLDSFileSource::GetSyncSource( __deref_out_opt IReferenceClock **pClock )
{
	if ( pClock == NULL )
	{
		return	E_POINTER ;
	}
	SSmartLock<SCriticalSection>	lock( &m_csObj ) ;
	if ( m_pClock != NULL )
	{
		m_pClock->AddRef() ;
	}
	*pClock = m_pClock ;
	return	NOERROR ;
}

STDMETHODIMP SGLDSFileSource::Stop( void )
{
	SSmartLock<SCriticalSection>	lock( &m_csObj ) ;
	m_state = State_Stopped ;
	return	NOERROR ;
}

STDMETHODIMP SGLDSFileSource::Pause( void )
{
	SSmartLock<SCriticalSection>	lock( &m_csObj ) ;
	m_state = State_Paused ;
	return	NOERROR ;
}

STDMETHODIMP SGLDSFileSource::Run( REFERENCE_TIME tStart )
{
	SSmartLock<SCriticalSection>	lock( &m_csObj ) ;
	m_state = State_Running ;
	return	NOERROR ;
}

// IBaseFilter
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP SGLDSFileSource::EnumPins( __deref_out IEnumPins ** ppEnum )
{
	if ( ppEnum == NULL )
	{
		return	E_POINTER ;
	}
	*ppEnum = new SGLDSFileSource::EnumOutPins( this ) ;
	return	NOERROR ;
}

STDMETHODIMP SGLDSFileSource::FindPin( LPCWSTR Id, __deref_out IPin ** ppPin )
{
	SSmartLock<SCriticalSection>	lock( &m_csObj ) ;
	FileOutPin *	pPin = GetOutputPin() ;
	if ( pPin != NULL )
	{
		if ( pPin->GetName() == Id )
		{
			*ppPin = pPin ;
			pPin->AddRef() ;
			return	S_OK ;
		}
	}
	*ppPin = NULL ;
	return	VFW_E_NOT_FOUND ;
}

STDMETHODIMP SGLDSFileSource::QueryFilterInfo( __out FILTER_INFO * pInfo )
{
	if ( pInfo == NULL )
	{
		return	E_POINTER ;
	}
	if ( !m_strName.IsEmpty() )
	{
        StringCchCopyW( pInfo->achName, NUMELMS(pInfo->achName), m_strName ) ;
	}
	else
	{
        pInfo->achName[0] = 0 ;
	}
	pInfo->pGraph = m_pGraph ;
	if ( m_pGraph != NULL )
	{
		m_pGraph->AddRef() ;
	}
	return	NOERROR ;
}

STDMETHODIMP SGLDSFileSource::JoinFilterGraph
	( __inout_opt IFilterGraph * pGraph, __in_opt LPCWSTR pName )
{
	SSmartLock<SCriticalSection>	lock( &m_csObj ) ;
	m_pGraph = pGraph ;
	m_strName = pName ;
	return	NOERROR ;
}

STDMETHODIMP SGLDSFileSource::QueryVendorInfo( __deref_out LPWSTR* pVendorInfo )
{
	return	E_NOTIMPL ;
}

// IAMAsyncReaderTimestampScaling
//////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE
			SGLDSFileSource::GetTimestampMode( __out BOOL *pfRaw )
{
	if ( pfRaw == NULL )
	{
		return	E_POINTER ;
	}
	*pfRaw = m_fTimestampRaw ;
	return	S_OK ;
}

HRESULT STDMETHODCALLTYPE
			SGLDSFileSource::SetTimestampMode( BOOL fRaw )
{
	m_fTimestampRaw = fRaw ;
	return	S_OK ;
}

// IAsyncReader
//////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE SGLDSFileSource::RequestAllocator
	( IMemAllocator * pPreferred,
		__in ALLOCATOR_PROPERTIES * pProps,
		__out IMemAllocator ** ppActual )
{
	if ( ppActual == NULL )
	{
		return	E_POINTER ;
	}
	if ( pPreferred != NULL )
	{
		pPreferred->AddRef() ;
	}
	else
	{
		HRESULT	hr = CoCreateInstance
			( CLSID_MemoryAllocator,
				NULL, CLSCTX_INPROC,
				IID_IMemAllocator, (void **) &pPreferred ) ;
		if ( FAILED(hr) )
		{
			return	hr ;
		}
	}
	ALLOCATOR_PROPERTIES	propIn = *pProps ;
	if ( propIn.cBuffers == 0 )
	{
		propIn.cBuffers = 4 ;
	}
	if ( propIn.cbBuffer == 0 )
	{
		propIn.cbBuffer = 0x4000 ;
	}
	if ( propIn.cbAlign == 0 )
	{
		propIn.cbAlign = 1 ;
	}
	ALLOCATOR_PROPERTIES	propOut ;
	HRESULT	hr = pPreferred->SetProperties( &propIn, &propOut ) ;
	if ( FAILED(hr) )
	{
		pPreferred->Release();
		return	hr ;
	}
	*ppActual = pPreferred ;
	return	S_OK ;
}

HRESULT STDMETHODCALLTYPE SGLDSFileSource::Request
	( IMediaSample * pSample, DWORD_PTR dwUser )
{
	if ( pSample == NULL )
	{
		return	E_POINTER ;
	}
	SSmartLock<SCriticalSection>	lock( &m_csQueue ) ;
	if ( m_sigFlushing.Wait(0) == errSuccess )
	{
		return	VFW_E_WRONG_STATE ;
	}
	RequestEntry *	pre = new RequestEntry ;
	pre->pSample = pSample ;
	pre->dwUser = dwUser ;
	m_queRequests.Add( pre ) ;
	m_sigReqQueue.SetSignal() ;
	//
	if ( !m_threadAsync.IsRunning() )
	{
		m_threadAsync.BeginThread( &m_procAsync ) ;
	}
	//
	return	S_OK ;
}

HRESULT STDMETHODCALLTYPE SGLDSFileSource::WaitForNext
	( DWORD dwTimeout,
		__out_opt IMediaSample ** ppSample,
		__out DWORD_PTR * pdwUser )
{
	if ( (ppSample == NULL) || (pdwUser == NULL) )
	{
		return	E_POINTER ;
	}
	//
	HANDLE	hEvents[2] ;
	hEvents[0] = m_sigDoneQueue.GetHandle() ;
	hEvents[1] = m_sigFlushing.GetHandle() ;
	//
	DWORD	dwResult =
		WaitForMultipleObjects( 2, &hEvents[0], FALSE, dwTimeout ) ;
	//
	RequestEntry *	pre = NULL ;
	{
		SSmartLock<SCriticalSection>	lock( &m_csQueue ) ;
		pre = m_queDones.DetachAt( 0 ) ;
		if ( m_queDones.GetLength() == 0 )
		{
			m_sigDoneQueue.ResetSignal() ;
		}
	}
	if ( (dwResult == WAIT_OBJECT_0 + 1)
		&& (pre != NULL) && (pre->pSample != NULL) )
	{
		IMediaSample *	pms = pre->pSample ;
		*ppSample = pre->pSample ;
		*pdwUser = pre->dwUser ;
		/*
		REFERENCE_TIME	rtStart, rtEnd ;
		if ( SUCCEEDED( pms->GetTime( &rtStart, &rtEnd ) ) )
		{
			pms->SetTime( &rtStart, &rtStart ) ;
		}
		pms->SetActualDataLength( 0 ) ;
		*/
		return	VFW_E_WRONG_STATE ;
	}
	if ( (pre == NULL) || (pre->pSample == NULL) )
	{
		*ppSample = NULL ;
		*pdwUser = 0 ;
		return	VFW_E_TIMEOUT ;
	}
	IMediaSample *	pms = pre->pSample ;
	DWORD_PTR		dwUser = pre->dwUser ;
	HRESULT			hrRead = pre->hrRead ;
	delete	pre ;
	//
	ESLAssert( pms != NULL ) ;
	*ppSample = pms ;
	*pdwUser = dwUser ;
	return	(hrRead == S_FALSE) ? S_OK : hrRead ;
}

HRESULT STDMETHODCALLTYPE SGLDSFileSource::SyncReadAligned
	( IMediaSample * pSample )
{
	if ( pSample == NULL )
	{
		return	E_POINTER ;
	}
	REFERENCE_TIME	rtStart, rtEnd ;
	if ( FAILED( pSample->GetTime( &rtStart, &rtEnd ) ) )
	{
		return	E_FAIL ;
	}
	if ( !m_fTimestampRaw )
	{
		rtStart /= 10000000 ;
		rtEnd   /= 10000000 ;
	}
	SSmartLock<SCriticalSection>	lock( &m_csFile ) ;
	int64_t	nFileLen = m_pFile->GetLength() ;
	if ( nFileLen < 0 )
	{
		return	E_FAIL ;
	}
	bool	fEndOfFile = false ;
	if ( rtEnd > nFileLen )
	{
		rtEnd = nFileLen ;
		fEndOfFile = true ;
	}
	if ( rtEnd <= rtStart )
	{
		return	E_FAIL ;
	}
	size_t	nBytes = (size_t) (rtEnd - rtStart) ;
	if ( nBytes > (size_t) pSample->GetSize() )
	{
		return	E_FAIL ;
	}
	BYTE *	pDstBuf = NULL ;
	HRESULT	hr = pSample->GetPointer( &pDstBuf ) ;
	if ( FAILED( hr ) )
	{
		return	hr ;
	}
	m_pFile->Seek( rtStart ) ;
	m_pFile->Read( pDstBuf, nBytes ) ;
	//
	if ( !m_fTimestampRaw )
	{
		rtStart *= 10000000 ;
		rtEnd   *= 10000000 ;
	}
	pSample->SetTime( &rtStart, &rtEnd ) ;
	pSample->SetActualDataLength( (long) nBytes ) ;
	pSample->SetSyncPoint( TRUE ) ;
	//
	return	fEndOfFile ? S_FALSE : S_OK ;
}

HRESULT STDMETHODCALLTYPE SGLDSFileSource::SyncRead
	( LONGLONG llPosition,
		LONG lLength, __out_bcount(lLength) BYTE * pBuffer )
{
	SSmartLock<SCriticalSection>	lock( &m_csFile ) ;
	int64_t	nFileLen = m_pFile->GetLength() ;
	if ( nFileLen < 0 )
	{
		return	E_FAIL ;
	}
	HRESULT	hr = S_OK ;
	size_t	nBytes = (size_t) lLength ;
	if ( llPosition + (long) nBytes > nFileLen )
	{
		if ( llPosition >= nFileLen )
		{
			return	E_FAIL ;
		}
		nBytes = (size_t) (nFileLen - llPosition) ;
		hr = S_FALSE ;
	}
	m_pFile->Seek( llPosition ) ;
	m_pFile->Read( pBuffer, nBytes ) ;
	//
	return	hr ;
}

HRESULT STDMETHODCALLTYPE SGLDSFileSource::Length
	( __out LONGLONG * pTotal, __out LONGLONG * pAvailable )
{
	if ( (pTotal == NULL) || (pAvailable == NULL) )
	{
		return	E_POINTER ;
	}
	int64_t	nFileLen = m_pFile->GetLength() ;
	*pTotal = nFileLen ;
	*pAvailable = nFileLen ;
	return	S_OK ;
}

HRESULT STDMETHODCALLTYPE SGLDSFileSource::BeginFlush( void )
{
	SSmartLock<SCriticalSection>	lock( &m_csQueue ) ;
	if ( m_sigFlushing.Wait(0) == errSuccess )
	{
		return	S_FALSE ;
	}
	else
	{
		m_sigFlushing.SetSignal() ;
		return	S_OK ;
	}
}

HRESULT STDMETHODCALLTYPE SGLDSFileSource::EndFlush( void )
{
	SSmartLock<SCriticalSection>	lock( &m_csQueue ) ;
	if ( m_sigFlushing.Wait(0) == errSuccess )
	{
		m_sigFlushing.ResetSignal() ;
		return	S_OK ;
	}
	else
	{
		return	S_FALSE ;
	}
}



//////////////////////////////////////////////////////////////////////////////
// 位置インターフェース実装
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLDSRenderMediaPlayer::PosPassThru::PosPassThru( IUnknown * pOwner, IPin * pPin )
{
	m_nRef = 1 ;
	m_pOwner = pOwner ;
	m_pti = NULL ;
	m_pPin = pPin ;
	//
	m_nStartMedia = 0 ;
	m_nEndMedia = 0 ;
	m_fMediaTime = false ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLDSRenderMediaPlayer::PosPassThru::~PosPassThru( void )
{
}

// Dispatch 補助関数
//////////////////////////////////////////////////////////////////////////////
HRESULT SGLDSRenderMediaPlayer::PosPassThru::GetTypeInfo
	( REFIID riid, UINT itinfo, LCID lcid, __deref_out ITypeInfo ** pptinfo )
{
	if ( pptinfo == NULL )
	{
		return	E_POINTER ;
	}
	*pptinfo = NULL ;
	//
	if ( itinfo != 0 )
	{
		return	TYPE_E_ELEMENTNOTFOUND ;
	}
	if ( m_pti == NULL )
	{
		HMODULE	hLib = m_hLibOleAut32 ;
		if ( hLib == NULL )
		{
			hLib = LoadLibrary( "OleAut32.dll" ) ;
			if ( hLib == NULL )
			{
				return	MAKE_HRESULT( SEVERITY_ERROR, FACILITY_WIN32, GetLastError() ) ;
			}
			m_hLibOleAut32 = hLib ;
		}
		API_LoadRegTypeLib	apiLoadRegTypeLib =
			(API_LoadRegTypeLib) GetProcAddress( hLib, "LoadRegTypeLib" ) ;
		if ( apiLoadRegTypeLib == NULL )
		{
			return	MAKE_HRESULT( SEVERITY_ERROR, FACILITY_WIN32, GetLastError() ) ;
		}
		ITypeLib *	ptlib = NULL ;
		HRESULT		hr =
			apiLoadRegTypeLib( LIBID_QuartzTypeLib, 1, 0, lcid, &ptlib ) ;
		if ( FAILED( hr ) )
		{
			API_LoadTypeLib	apiLoadTypeLib =
				(API_LoadTypeLib) GetProcAddress( hLib, "LoadTypeLib" ) ;
			if ( apiLoadTypeLib == NULL )
			{
				return	MAKE_HRESULT( SEVERITY_ERROR, FACILITY_WIN32, GetLastError() ) ;
			}
			hr = apiLoadTypeLib( L"control.tlb", &ptlib ) ;
			if ( FAILED( hr ) )
			{
				return	hr ;
			}
		}
		//
		hr = ptlib->GetTypeInfoOfGuid( riid, &m_pti ) ;
		ptlib->Release() ;
		//
		if ( FAILED( hr ) )
		{
			return	hr ;
		}
	}
	*pptinfo = m_pti ;
	m_pti->AddRef() ;
	//
	return	S_OK ;
}

// 接続先 IMediaPosition 取得
//////////////////////////////////////////////////////////////////////////////
IMediaPosition * SGLDSRenderMediaPlayer::PosPassThru::GetPeerPosition( void )
{
	IPin *	pConnected = NULL ;
	HRESULT	hr = m_pPin->ConnectedTo( &pConnected ) ;
	if ( FAILED( hr ) )
	{
		return	NULL ;
	}
	IMediaPosition *	pms = NULL ;
	hr = pConnected->QueryInterface( IID_IMediaPosition, (void**) &pms ) ;
	pConnected->Release() ;
	if ( FAILED( hr ) )
	{
		return	NULL ;
	}
	return	pms ;
}

// 接続先 IMediaSeeking 取得
//////////////////////////////////////////////////////////////////////////////
IMediaSeeking * SGLDSRenderMediaPlayer::PosPassThru::GetPeerSeeking( void )
{
	IPin *	pConnected = NULL ;
	HRESULT	hr = m_pPin->ConnectedTo( &pConnected ) ;
	if ( FAILED( hr ) )
	{
		return	NULL ;
	}
	IMediaSeeking *	pms = NULL ;
	hr = pConnected->QueryInterface( IID_IMediaSeeking, (void**) &pms ) ;
	pConnected->Release() ;
	if ( FAILED( hr ) )
	{
		return	NULL ;
	}
	return	pms ;
}

// メディア時間設定
//////////////////////////////////////////////////////////////////////////////
HRESULT SGLDSRenderMediaPlayer::PosPassThru::RegisterMediaTime( IMediaSample * pms )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	//
	LONGLONG	nStart, nEnd ;
	HRESULT		hr = pms->GetTime( &nStart, &nEnd ) ;
	if ( FAILED( hr ) )
	{
		return	hr ;
	}
	m_nStartMedia = nStart ;
	m_nEndMedia = nEnd ;
	m_fMediaTime = true ;
	return	NOERROR ;
}

HRESULT SGLDSRenderMediaPlayer::PosPassThru::RegisterMediaTime( LONGLONG nStart, LONGLONG nEnd )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	m_nStartMedia = nStart ;
	m_nEndMedia = nEnd ;
	m_fMediaTime = true ;
	return	NOERROR ;
}

// メディア時間取得
//////////////////////////////////////////////////////////////////////////////
HRESULT SGLDSRenderMediaPlayer::PosPassThru::GetMediaTime
	( __out LONGLONG *pStartTime, __out_opt LONGLONG *pEndTime )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	if ( !m_fMediaTime )
	{
		return	E_FAIL ;
	}
	HRESULT	hr =
		ConvertTimeFormat
			( pStartTime, 0, m_nStartMedia, &TIME_FORMAT_MEDIA_TIME ) ;
	if ( pEndTime && SUCCEEDED( hr ) )
	{
		hr = ConvertTimeFormat
				( pEndTime, 0, m_nEndMedia, &TIME_FORMAT_MEDIA_TIME ) ;
	}
	return	hr ;
}

// メディア時間リセット
//////////////////////////////////////////////////////////////////////////////
void SGLDSRenderMediaPlayer::PosPassThru::ResetMediaTime( void )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	m_nStartMedia = 0 ;
	m_nEndMedia = 0 ;
	m_fMediaTime = false ;
}

// ストリーム終端
//////////////////////////////////////////////////////////////////////////////
void SGLDSRenderMediaPlayer::PosPassThru::EndOfStream( void )
{
	if ( m_fMediaTime )
	{
		LONGLONG	nStop ;
		if ( SUCCEEDED( GetStopPosition( &nStop ) ) )
		{
			SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
			m_nStartMedia = nStop ;
			m_nEndMedia = nStop ;
		}
	}
}

// IUnknown
//////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE SGLDSRenderMediaPlayer::PosPassThru::QueryInterface( REFIID riid, void ** ppObj )
{
    if ( IsEqualIID(riid, IID_IMediaSeeking) )
	{
		*ppObj = (IMediaSeeking*) this ;
	}
    else if ( IsEqualIID(riid, IID_IMediaPosition) )
	{
		*ppObj = (IMediaPosition*) this ;
	}
    else if ( IsEqualIID(riid, IID_IDispatch) )
	{
		*ppObj = (IDispatch*) this ;
	}
    else
    {
        *ppObj = NULL ;
        return	E_NOINTERFACE ;
    }
    AddRef() ;
    return	S_OK ;
}

ULONG STDMETHODCALLTYPE SGLDSRenderMediaPlayer::PosPassThru::AddRef( void )
{
	return	AtomicAdd( &m_nRef, 1 ) ;
}

ULONG STDMETHODCALLTYPE SGLDSRenderMediaPlayer::PosPassThru::Release( void )
{
	ULONG	nRef = (ULONG) AtomicSub( &m_nRef, 1 ) ;
	if ( nRef == 0 )
	{
		delete	this ;
	}
	return	nRef ;
}

// IDispatch
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP SGLDSRenderMediaPlayer::PosPassThru::GetTypeInfoCount( __out UINT * pctinfo )
{
	if ( pctinfo == NULL )
	{
		return	E_POINTER ;
	}
	*pctinfo = 1 ;
	return	S_OK ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::PosPassThru::GetTypeInfo
	( UINT itinfo, LCID lcid, __deref_out ITypeInfo ** pptinfo )
{
	return	GetTypeInfo( IID_IMediaPosition, itinfo, lcid, pptinfo ) ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::PosPassThru::GetIDsOfNames
	( REFIID riid, __in_ecount(cNames) LPOLESTR * rgszNames,
		UINT cNames, LCID lcid, __out_ecount(cNames) DISPID * rgdispid )
{
	ITypeInfo *	pti = NULL ;
	HRESULT		hr = GetTypeInfo( IID_IMediaPosition, 0, lcid, &pti ) ;
	//
	if ( SUCCEEDED( hr ) )
	{
		hr = pti->GetIDsOfNames( rgszNames, cNames, rgdispid ) ;
		pti->Release() ;
	}
	return	hr ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::PosPassThru::Invoke
	( DISPID dispidMember, REFIID riid, LCID lcid,
		WORD wFlags, __in DISPPARAMS * pdispparams,
		__out_opt VARIANT * pvarResult,
		__out_opt EXCEPINFO * pexcepinfo, __out_opt UINT * puArgErr )
{
	if ( riid == IID_NULL )
	{
		return	DISP_E_UNKNOWNINTERFACE ;
	}
	ITypeInfo *	pti = NULL ;
	HRESULT		hr = GetTypeInfo( 0, lcid, &pti ) ;
	if ( FAILED( hr ) )
	{
		return	hr ;
	}
	hr = pti->Invoke
		( (IMediaPosition*) this, dispidMember,
			wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr ) ;
	pti->Release() ;
	//
	return	hr ;
}

// IMediaSeeking
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP SGLDSRenderMediaPlayer::PosPassThru::GetCapabilities( __out DWORD * pCapabilities )
{
	IMediaSeeking *	pms = GetPeerSeeking() ;
	if ( pms == NULL )
	{
		return	E_NOTIMPL ;
	}
	HRESULT	hr = pms->GetCapabilities( pCapabilities ) ;
	pms->Release() ;
	return	hr ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::PosPassThru::CheckCapabilities( __inout DWORD * pCapabilities )
{
	IMediaSeeking *	pms = GetPeerSeeking() ;
	if ( pms == NULL )
	{
		return	E_NOTIMPL ;
	}
	HRESULT	hr = pms->CheckCapabilities( pCapabilities ) ;
	pms->Release() ;
	return	hr ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::PosPassThru::SetTimeFormat(const GUID * pFormat )
{
	IMediaSeeking *	pms = GetPeerSeeking() ;
	if ( pms == NULL )
	{
		return	E_NOTIMPL ;
	}
	HRESULT	hr = pms->SetTimeFormat( pFormat ) ;
	pms->Release() ;
	return	hr ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::PosPassThru::GetTimeFormat(__out GUID *pFormat )
{
	IMediaSeeking *	pms = GetPeerSeeking() ;
	if ( pms == NULL )
	{
		return	E_NOTIMPL ;
	}
	HRESULT	hr = pms->GetTimeFormat( pFormat ) ;
	pms->Release() ;
	return	hr ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::PosPassThru::IsUsingTimeFormat(const GUID * pFormat )
{
	IMediaSeeking *	pms = GetPeerSeeking() ;
	if ( pms == NULL )
	{
		return	E_NOTIMPL ;
	}
	HRESULT	hr = pms->IsUsingTimeFormat( pFormat ) ;
	pms->Release() ;
	return	hr ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::PosPassThru::IsFormatSupported( const GUID * pFormat )
{
	IMediaSeeking *	pms = GetPeerSeeking() ;
	if ( pms == NULL )
	{
		return	E_NOTIMPL ;
	}
	HRESULT	hr = pms->IsFormatSupported( pFormat ) ;
	pms->Release() ;
	return	hr ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::PosPassThru::QueryPreferredFormat( __out GUID *pFormat )
{
	IMediaSeeking *	pms = GetPeerSeeking() ;
	if ( pms == NULL )
	{
		return	E_NOTIMPL ;
	}
	HRESULT	hr = pms->QueryPreferredFormat( pFormat ) ;
	pms->Release() ;
	return	hr ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::PosPassThru::ConvertTimeFormat
	(__out LONGLONG * pTarget, 
	   __in_opt const GUID * pTargetFormat,
	   LONGLONG Source,  __in_opt const GUID * pSourceFormat )
{
	IMediaSeeking *	pms = GetPeerSeeking() ;
	if ( pms == NULL )
	{
		return	E_NOTIMPL ;
	}
	HRESULT	hr = pms->ConvertTimeFormat
					( pTarget, pTargetFormat, Source, pSourceFormat ) ;
	pms->Release() ;
	return	hr ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::PosPassThru::SetPositions
	( __inout_opt LONGLONG * pCurrent, DWORD CurrentFlags,
			__inout_opt LONGLONG * pStop, DWORD StopFlags )
{
	IMediaSeeking *	pms = GetPeerSeeking() ;
	if ( pms == NULL )
	{
		return	E_NOTIMPL ;
	}
	HRESULT	hr = pms->SetPositions
					( pCurrent, CurrentFlags, pStop, StopFlags ) ;
	pms->Release() ;
	return	hr ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::PosPassThru::GetPositions( __out_opt LONGLONG * pCurrent, __out_opt LONGLONG * pStop )
{
	IMediaSeeking *	pms = GetPeerSeeking() ;
	if ( pms == NULL )
	{
		return	E_NOTIMPL ;
	}
	HRESULT	hr = pms->GetPositions( pCurrent, pStop ) ;
	pms->Release() ;
	return	hr ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::PosPassThru::GetCurrentPosition( __out LONGLONG * pCurrent )
{
	HRESULT	hr = GetMediaTime( pCurrent, NULL ) ;
	if ( SUCCEEDED( hr ) )
	{
		return	NOERROR ;
	}
	IMediaSeeking *	pms = GetPeerSeeking() ;
	if ( pms == NULL )
	{
		return	E_NOTIMPL ;
	}
	hr = pms->GetCurrentPosition( pCurrent ) ;
	pms->Release() ;
	return	hr ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::PosPassThru::GetStopPosition( __out LONGLONG * pStop )
{
	IMediaSeeking *	pms = GetPeerSeeking() ;
	if ( pms == NULL )
	{
		return	E_NOTIMPL ;
	}
	HRESULT	hr = pms->GetStopPosition( pStop ) ;
	pms->Release() ;
	return	hr ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::PosPassThru::SetRate( double dRate )
{
	if ( dRate == 0.0 )
	{
		return	E_INVALIDARG ;
	}
	IMediaSeeking *	pms = GetPeerSeeking() ;
	if ( pms == NULL )
	{
		return	E_NOTIMPL ;
	}
	HRESULT	hr = pms->SetRate( dRate ) ;
	pms->Release() ;
	return	hr ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::PosPassThru::GetRate( __out double * pdRate )
{
	IMediaSeeking *	pms = GetPeerSeeking() ;
	if ( pms == NULL )
	{
		return	E_NOTIMPL ;
	}
	HRESULT	hr = pms->GetRate( pdRate ) ;
	pms->Release() ;
	return	hr ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::PosPassThru::GetDuration( __out LONGLONG *pDuration )
{
	IMediaSeeking *	pms = GetPeerSeeking() ;
	if ( pms == NULL )
	{
		return	E_NOTIMPL ;
	}
	HRESULT	hr = pms->GetDuration( pDuration ) ;
	pms->Release() ;
	return	hr ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::PosPassThru::GetAvailable( __out_opt LONGLONG *pEarliest, __out_opt LONGLONG *pLatest )
{
	IMediaSeeking *	pms = GetPeerSeeking() ;
	if ( pms == NULL )
	{
		return	E_NOTIMPL ;
	}
	HRESULT	hr = pms->GetAvailable( pEarliest, pLatest ) ;
	pms->Release() ;
	return	hr ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::PosPassThru::GetPreroll( __out LONGLONG *pllPreroll )
{
	IMediaSeeking *	pms = GetPeerSeeking() ;
	if ( pms == NULL )
	{
		return	E_NOTIMPL ;
	}
	HRESULT	hr = pms->GetPreroll( pllPreroll ) ;
	pms->Release() ;
	return	hr ;
}

// IMediaPosition
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP SGLDSRenderMediaPlayer::PosPassThru::get_Duration(__out REFTIME * plength)
{
	IMediaPosition *	pmp = GetPeerPosition() ;
	if ( pmp == NULL )
	{
		return	E_NOTIMPL ;
	}
	HRESULT	hr = pmp->get_Duration( plength ) ;
	pmp->Release() ;
	return	hr ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::PosPassThru::put_CurrentPosition(REFTIME llTime)
{
	IMediaPosition *	pmp = GetPeerPosition() ;
	if ( pmp == NULL )
	{
		return	E_NOTIMPL ;
	}
	HRESULT	hr = pmp->put_CurrentPosition( llTime ) ;
	pmp->Release() ;
	return	hr ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::PosPassThru::get_StopTime(__out REFTIME * pllTime)
{
	IMediaPosition *	pmp = GetPeerPosition() ;
	if ( pmp == NULL )
	{
		return	E_NOTIMPL ;
	}
	HRESULT	hr = pmp->get_StopTime( pllTime ) ;
	pmp->Release() ;
	return	hr ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::PosPassThru::put_StopTime(REFTIME llTime)
{
	IMediaPosition *	pmp = GetPeerPosition() ;
	if ( pmp == NULL )
	{
		return	E_NOTIMPL ;
	}
	HRESULT	hr = pmp->put_StopTime( llTime ) ;
	pmp->Release() ;
	return	hr ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::PosPassThru::get_PrerollTime(__out REFTIME * pllTime)
{
	IMediaPosition *	pmp = GetPeerPosition() ;
	if ( pmp == NULL )
	{
		return	E_NOTIMPL ;
	}
	HRESULT	hr = pmp->get_PrerollTime( pllTime ) ;
	pmp->Release() ;
	return	hr ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::PosPassThru::put_PrerollTime(REFTIME llTime)
{
	IMediaPosition *	pmp = GetPeerPosition() ;
	if ( pmp == NULL )
	{
		return	E_NOTIMPL ;
	}
	HRESULT	hr = pmp->put_PrerollTime( llTime ) ;
	pmp->Release() ;
	return	hr ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::PosPassThru::get_Rate(__out double * pdRate)
{
	IMediaPosition *	pmp = GetPeerPosition() ;
	if ( pmp == NULL )
	{
		return	E_NOTIMPL ;
	}
	HRESULT	hr = pmp->get_Rate( pdRate ) ;
	pmp->Release() ;
	return	hr ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::PosPassThru::put_Rate(double dRate)
{
	IMediaPosition *	pmp = GetPeerPosition() ;
	if ( pmp == NULL )
	{
		return	E_NOTIMPL ;
	}
	HRESULT	hr = pmp->put_Rate( dRate ) ;
	pmp->Release() ;
	return	hr ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::PosPassThru::get_CurrentPosition(__out REFTIME * pllTime)
{
	IMediaPosition *	pmp = GetPeerPosition() ;
	if ( pmp == NULL )
	{
		return	E_NOTIMPL ;
	}
	HRESULT	hr = pmp->get_CurrentPosition( pllTime ) ;
	pmp->Release() ;
	return	hr ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::PosPassThru::CanSeekForward(__out LONG *pCanSeekForward)
{
	IMediaPosition *	pmp = GetPeerPosition() ;
	if ( pmp == NULL )
	{
		return	E_NOTIMPL ;
	}
	HRESULT	hr = pmp->CanSeekForward( pCanSeekForward ) ;
	pmp->Release() ;
	return	hr ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::PosPassThru::CanSeekBackward(__out LONG *pCanSeekBackward)
{
	IMediaPosition *	pmp = GetPeerPosition() ;
	if ( pmp == NULL )
	{
		return	E_NOTIMPL ;
	}
	HRESULT	hr = pmp->CanSeekBackward( pCanSeekBackward ) ;
	pmp->Release() ;
	return	hr ;
}



//////////////////////////////////////////////////////////////////////////////
// レンダリングフィルタ入力ピン実装
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLDSRenderMediaPlayer::InputPin::InputPin
		( Filter * pFilter, const wchar_t * pwszName )
	: m_strName( pwszName )
{
	m_nRef = 1 ;
	m_pConnected = NULL ;
	m_pFilter = pFilter ;
	m_pAlloc = NULL ;
	m_pQCtrl = NULL ;
	//
	m_rtStart = 0 ;
	m_rtStop = 0x7FFFFFFFFFFFFFFF ;
	m_fpRate = 1.0 ;
	//
	m_fReadOnly = false ;
	m_fFlushing = false ;
	m_fRuntimeError = false ;
	//
	memset( &m_mtType, 0, sizeof(m_mtType) ) ;
	m_mtType.lSampleSize = 1 ;
	m_mtType.bFixedSizeSamples = TRUE ;
	//
	memset( &m_propSample, 0, sizeof(m_propSample) ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLDSRenderMediaPlayer::InputPin::~InputPin( void )
{
	if ( m_pAlloc != NULL )
	{
		m_pAlloc->Release() ;
		m_pAlloc = NULL ;
	}
	FreeMediaType( m_mtType ) ;
}

// アロケータ生成
//////////////////////////////////////////////////////////////////////////////
IMemAllocator * SGLDSRenderMediaPlayer::InputPin::CreateAllocator( void )
{
	IMemAllocator *	pAlloc = NULL ;
	CoCreateInstance
		( CLSID_MemoryAllocator,
			0, CLSCTX_INPROC_SERVER,
			IID_IMemAllocator, (void **) &pAlloc ) ;
	return	pAlloc ;
}

// ストリーム検査
//////////////////////////////////////////////////////////////////////////////
HRESULT SGLDSRenderMediaPlayer::InputPin::CheckStreaming( void ) const
{
	ESLAssert( IsConnected() ) ;
	if ( IsStopped() )
	{
		return	VFW_E_WRONG_STATE ;
	}
	if ( m_fFlushing )
	{
		return	S_FALSE ;
	}
	if ( m_fRuntimeError )
	{
		return	VFW_E_RUNTIME_ERROR ;
	}
	return	S_OK ;
}

// 接続破棄
//////////////////////////////////////////////////////////////////////////////
HRESULT SGLDSRenderMediaPlayer::InputPin::BreakConnect( void )
{
	HRESULT	hr = m_pFilter->OnBreakConnect() ;
	if ( FAILED( hr ) )
	{
		return	hr ;
	}
	if ( m_pAlloc != NULL )
	{
		HRESULT	hr = m_pAlloc->Decommit() ;
		if ( FAILED( hr ) )
		{
			return	hr ;
		}
		m_pAlloc->Release() ;
		m_pAlloc = NULL ;
	}
	return	S_OK ;
}

// 接続完了
//////////////////////////////////////////////////////////////////////////////
HRESULT SGLDSRenderMediaPlayer::InputPin::CompleteConnect( IPin * pReceivePin )
{
	HRESULT	hr = m_pFilter->OnCompleteConnect( pReceivePin ) ;
	if ( FAILED( hr ) )
	{
		return	hr ;
	}
	return	NOERROR ;
}

// メディアタイプ取得
//////////////////////////////////////////////////////////////////////////////
HRESULT SGLDSRenderMediaPlayer::InputPin::GetMediaType
	( size_t i, __inout AM_MEDIA_TYPE * pmt )
{
	return	E_UNEXPECTED ;
}

// メディアタイプ設定
//////////////////////////////////////////////////////////////////////////////
HRESULT SGLDSRenderMediaPlayer::InputPin::SetMediaType( const AM_MEDIA_TYPE * pmt )
{
	FreeMediaType( m_mtType ) ;
	//
	HRESULT	hr = CopyMediaType( &m_mtType, pmt ) ;
	if ( FAILED( hr ) )
	{
		return	hr ;
	}
	return	m_pFilter->OnSetMediaType( pmt ) ;
}

// 受け入れ可能なメディアタイプ判定
//////////////////////////////////////////////////////////////////////////////
HRESULT SGLDSRenderMediaPlayer::InputPin::CheckMediaType( const AM_MEDIA_TYPE * pmt )
{
	return	m_pFilter->CheckMediaType( pmt ) ;
}

// 接続検査
//////////////////////////////////////////////////////////////////////////////
HRESULT SGLDSRenderMediaPlayer::InputPin::CheckConnect( IPin * pPin )
{
    PIN_DIRECTION pd;
    pPin->QueryDirection( &pd ) ;
	//
	if ( pd == PINDIR_INPUT )
	{
		return	VFW_E_INVALID_DIRECTION ;
	}
	return	NOERROR ;
}

// 停止状態から変化時に呼び出される
//////////////////////////////////////////////////////////////////////////////
HRESULT SGLDSRenderMediaPlayer::InputPin::Active( void )
{
	return	m_pFilter->OnActive() ;
}

// 停止状態へ変化時に呼び出される
//////////////////////////////////////////////////////////////////////////////
HRESULT SGLDSRenderMediaPlayer::InputPin::Inactive( void )
{
	m_fRuntimeError = false ;
	return	m_pFilter->OnInactive() ;
}

// フィルタからの実行通知
//////////////////////////////////////////////////////////////////////////////
HRESULT SGLDSRenderMediaPlayer::InputPin::Run( REFERENCE_TIME tStart )
{
	return	NOERROR ;
}

// 接続試行
//////////////////////////////////////////////////////////////////////////////
HRESULT SGLDSRenderMediaPlayer::InputPin::AttemptConnection
	( IPin * pReceivePin, const AM_MEDIA_TYPE * pmt )
{
	HRESULT	hr = CheckConnect( pReceivePin ) ;
	if ( FAILED(hr) )
	{
		BreakConnect() ;
		return	hr ;
	}
	hr = CheckMediaType( pmt ) ;
	if ( hr == NOERROR )
	{
		m_pConnected = pReceivePin ;
		pReceivePin->AddRef() ;
		//
		hr = SetMediaType( pmt ) ;
		if ( SUCCEEDED( hr ) )
		{
			hr = pReceivePin->ReceiveConnection( this, pmt ) ;
			if ( SUCCEEDED( hr ) )
			{
				hr = CompleteConnect( pReceivePin ) ;
				if ( SUCCEEDED( hr ) )
				{
					return	hr ;
				}
				pReceivePin->Disconnect() ;
			}
		}
	}
	else
	{
		if ( SUCCEEDED(hr)
			|| (hr == E_FAIL)
			|| (hr == E_INVALIDARG) )
		{
			hr = VFW_E_TYPE_NOT_ACCEPTED ;
		}
	}
	//
	BreakConnect() ;
	//
	if ( m_pConnected != NULL )
	{
		m_pConnected->Release() ;
		m_pConnected = NULL ;
	}
	return	hr ;
}

HRESULT SGLDSRenderMediaPlayer::InputPin::TryMediaTypes
	( IPin * pReceivePin,
		__in_opt const AM_MEDIA_TYPE * pmt, IEnumMediaTypes * pEnum )
{
	HRESULT	hr = pEnum->Reset() ;
	if ( FAILED(hr) )
	{
		return	hr ;
	}
	//
	AM_MEDIA_TYPE *	pmType = NULL ;
	ULONG			nCount ;
	HRESULT			hrErr = S_OK ;
	//
	for ( ; ; )
	{
		hr = pEnum->Next( 1, &pmType, &nCount ) ;
		if ( hr != S_OK )
		{
			if ( hrErr == S_OK )
			{
				hrErr = VFW_E_NO_ACCEPTABLE_TYPES ;
			}
			return	hrErr ;
		}
		if ( (pmType != NULL)
			&& ((pmt == NULL)
				|| MediaTypeMatchesPartial( pmType, pmt)) )
		{
			hr = AttemptConnection( pReceivePin, pmType ) ;
			if ( FAILED(hr)
				&& SUCCEEDED(hrErr)
				&& (hr != E_FAIL)
				&& (hr != E_INVALIDARG)
				&& (hr != VFW_E_TYPE_NOT_ACCEPTED) )
			{
				hrErr = hr ;
			}
		}
		else
		{
			hr = VFW_E_NO_ACCEPTABLE_TYPES ;
		}
		if ( pmType != NULL )
		{
			DeleteMediaType( pmType ) ;
			pmType = NULL ;
		}
		if ( hr != S_OK )
		{
			return	hr ;
		}
	}
}

// メディア承諾
//////////////////////////////////////////////////////////////////////////////
HRESULT SGLDSRenderMediaPlayer::InputPin::AgreeMediaType
	( IPin * pReceivePin, const AM_MEDIA_TYPE * pmt )
{
	if ( (pmt != NULL) && !MediaTypeIsPartiallySpecified( pmt ) )
	{
		return	AttemptConnection( pReceivePin, pmt ) ;
	}
	HRESULT	hrErr = VFW_E_NO_ACCEPTABLE_TYPES ;
	for ( int i = 0; i < 2; i ++ )
	{
		IEnumMediaTypes *	pemtEnum = NULL ;
		HRESULT	hr ;
		if ( i == 0 )
		{
			hr = pReceivePin->EnumMediaTypes( &pemtEnum ) ;
		}
		else
		{
			hr = EnumMediaTypes( &pemtEnum ) ;
		}
		if ( SUCCEEDED(hr) )
		{
			hr = TryMediaTypes( pReceivePin, pmt, pemtEnum ) ;
			pemtEnum->Release() ;
			//
			if ( SUCCEEDED(hr) )
			{
				return	NOERROR ;
			}
			else
			{
				if ( (hr != E_FAIL)
					&& (hr != E_INVALIDARG)
					&& (hr != VFW_E_TYPE_NOT_ACCEPTED) )
				{
					hrErr = hr ;
				}
			}
		}
	}
	return	hrErr ;
}

// メディア受け取り処理
//////////////////////////////////////////////////////////////////////////////
HRESULT SGLDSRenderMediaPlayer::InputPin::ReceiveWithoutNotify( IMediaSample * pSample )
{
	HRESULT	hr = S_OK ;
	do
	{
		if ( pSample == NULL )
		{
			hr = E_POINTER ;
			break ;
		}
		HRESULT	hr = CheckStreaming() ;
		if ( hr != S_OK )
		{
			break ;
		}
		IMediaSample2 *	pSample2 = NULL ;
		if ( SUCCEEDED( pSample->QueryInterface
						( IID_IMediaSample2, (void**) &pSample2 ) ) )
		{
			hr = pSample2->GetProperties
					( sizeof(m_propSample), (PBYTE) &m_propSample ) ;
			pSample2->Release() ;
			if ( FAILED(hr) )
			{
				break ;
			}
		}
		else
		{
			m_propSample.cbData = sizeof(m_propSample) ;
			m_propSample.dwTypeSpecificFlags = 0 ;
			m_propSample.dwStreamId = AM_STREAM_MEDIA ;
			m_propSample.dwSampleFlags = 0 ;
			if ( pSample->IsDiscontinuity() == S_OK )
			{
				m_propSample.dwSampleFlags |= AM_SAMPLE_DATADISCONTINUITY ;
			}
			if ( pSample->IsPreroll() == S_OK )
			{
				m_propSample.dwSampleFlags |= AM_SAMPLE_PREROLL ;
			}
			if ( pSample->IsSyncPoint() == S_OK )
			{
				m_propSample.dwSampleFlags |= AM_SAMPLE_SPLICEPOINT ;
			}
			if ( SUCCEEDED( pSample->GetTime
					( &m_propSample.tStart, &m_propSample.tStop ) ) )
			{
				m_propSample.dwSampleFlags |=
								AM_SAMPLE_TIMEVALID | AM_SAMPLE_STOPVALID ;
			}
			if ( pSample->GetMediaType( &m_propSample.pMediaType ) == S_OK )
			{
				m_propSample.dwSampleFlags |= AM_SAMPLE_TYPECHANGED ;
			}
			pSample->GetPointer( &m_propSample.pbBuffer ) ;
			m_propSample.lActual = pSample->GetActualDataLength() ;
			m_propSample.cbBuffer = pSample->GetSize() ;
		}
		if ( !(m_propSample.dwSampleFlags & AM_SAMPLE_TYPECHANGED) )
		{
			hr = NOERROR ;
			break ;
		}
		hr = CheckMediaType( m_propSample.pMediaType ) ;
		if ( hr == NOERROR )
		{
			hr = NOERROR ;
			break ;
		}
		m_fRuntimeError = true ;
		EndOfStream() ;
		m_pFilter->NotifyEvent( EC_ERRORABORT, VFW_E_TYPE_NOT_ACCEPTED, 0 ) ;
		hr = VFW_E_INVALIDMEDIATYPE ;
	}
	while ( false ) ;
	return	hr ;
}

// IUnknown
//////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE SGLDSRenderMediaPlayer::InputPin::QueryInterface( REFIID riid, void ** ppObj )
{
    if ( IsEqualIID(riid, IID_IMemInputPin) )
	{
        *ppObj = (IMemInputPin*) this ;
	}
	else if ( IsEqualIID(riid, IID_IQualityControl) )
	{
        *ppObj = (IQualityControl*) this ;
	}
	else if ( IsEqualIID(riid, IID_IPin) )
	{
        *ppObj = (IPin*) this ;
	}
    else
    {
        *ppObj = NULL ;
        return	E_NOINTERFACE ;
    }
    AddRef() ;
    return	S_OK ;
}

ULONG STDMETHODCALLTYPE SGLDSRenderMediaPlayer::InputPin::AddRef( void )
{
	return	AtomicAdd( &m_nRef, 1 ) ;
}

ULONG STDMETHODCALLTYPE SGLDSRenderMediaPlayer::InputPin::Release( void )
{
	ULONG	nRef = (ULONG) AtomicSub( &m_nRef, 1 ) ;
	if ( nRef == 0 )
	{
		delete	this ;
	}
	return	nRef ;
}

// IPin
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP SGLDSRenderMediaPlayer::InputPin::Connect
	( IPin * pReceivePin, __in_opt const AM_MEDIA_TYPE *pmt )
{
	if ( pReceivePin == NULL )
	{
		return	E_POINTER ;
	}
	SSmartLock<SCriticalSection>	lock( &(m_pFilter->m_csSync) ) ;
	//
	if ( m_pConnected != NULL )
	{
		return	VFW_E_ALREADY_CONNECTED ;
	}
	if ( !IsStopped() )
	{
		return	VFW_E_NOT_STOPPED ;
	}
	HRESULT	hr = AgreeMediaType( pReceivePin, pmt ) ;
	if ( FAILED(hr) )
	{
		BreakConnect() ;
		return	hr ;
	}
	return	NOERROR ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::InputPin::ReceiveConnection
	( IPin * pConnector, const AM_MEDIA_TYPE *pmt )
{
	if ( (pConnector == NULL) || (pmt == NULL) )
	{
		return	E_POINTER ;
	}
	SSmartLock<SCriticalSection>	lock( &(m_pFilter->m_csSync) ) ;
	//
	if ( m_pConnected != NULL )
	{
		return	VFW_E_ALREADY_CONNECTED ;
	}
	if ( !IsStopped() )
	{
		return	VFW_E_NOT_STOPPED ;
	}
	HRESULT	hr = CheckConnect( pConnector ) ;
	if ( FAILED( hr ) )
	{
		BreakConnect() ;
		return	hr ;
	}
	hr = CheckMediaType( pmt ) ;
	if ( hr != NOERROR )
	{
		BreakConnect() ;
		//
		if ( SUCCEEDED(hr)
			|| (hr == E_FAIL)
			|| (hr == E_INVALIDARG) )
		{
			hr = VFW_E_TYPE_NOT_ACCEPTED ;
		}
		return	hr ;
	}
	m_pConnected = pConnector ;
	pConnector->AddRef() ;
	//
	hr = SetMediaType( pmt ) ;
	if ( SUCCEEDED( hr ) )
	{
		hr = CompleteConnect( pConnector ) ;
		if ( SUCCEEDED( hr ) )
		{
			return	NOERROR ;
		}
	}
	//
	m_pConnected->Release() ;
	m_pConnected = NULL ;
	//
	BreakConnect() ;
	//
	return	hr ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::InputPin::Disconnect( void )
{
	SSmartLock<SCriticalSection>	lock( &(m_pFilter->m_csSync) ) ;
	if ( !IsStopped() )
	{
		return	VFW_E_NOT_STOPPED ;
	}
	if ( m_pConnected != NULL )
	{
		BreakConnect() ;
		//
		m_pConnected->Release() ;
		m_pConnected = NULL ;
		//
		return	S_OK ;
	}
	return	S_FALSE ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::InputPin::ConnectedTo( __deref_out IPin ** ppPin )
{
	if ( ppPin == NULL )
	{
		return	E_POINTER ;
	}
	IPin *	pPin = m_pConnected ;
	*ppPin = pPin ;
	if ( pPin != NULL )
	{
		pPin->AddRef() ;
		return	S_OK ;
	}
	else
	{
		return	VFW_E_NOT_CONNECTED ;
	}
}

STDMETHODIMP SGLDSRenderMediaPlayer::InputPin::ConnectionMediaType( __out AM_MEDIA_TYPE * pmt )
{
	if ( pmt == NULL )
	{
		return	E_POINTER ;
	}
	SSmartLock<SCriticalSection>	lock( &(m_pFilter->m_csSync) ) ;
	if ( IsConnected() )
	{
		CopyMediaType( pmt, &m_mtType ) ;
		return	S_OK ;
	}
	else
	{
		return	VFW_E_NOT_CONNECTED ;
	}
}

STDMETHODIMP SGLDSRenderMediaPlayer::InputPin::QueryPinInfo( __out PIN_INFO * pInfo )
{
	if ( pInfo == NULL )
	{
		return	E_POINTER ;
	}
	pInfo->pFilter = m_pFilter ;
	if ( m_pFilter != NULL )
	{
		m_pFilter->AddRef() ;
	}
	if ( !m_strName.IsEmpty() )
	{
        StringCchCopyW( pInfo->achName, NUMELMS(pInfo->achName), m_strName ) ;
	}
	else
	{
        pInfo->achName[0] = 0 ;
	}
    pInfo->dir = PINDIR_INPUT ;
	return	NOERROR ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::InputPin::QueryDirection( __out PIN_DIRECTION * pPinDir )
{
	if ( pPinDir == NULL )
	{
		return	E_POINTER ;
	}
	*pPinDir = PINDIR_INPUT ;
	return	NOERROR ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::InputPin::QueryId( __deref_out LPWSTR * Id )
{
	size_t	nBytes = (m_strName.GetLength() + 1) * sizeof(wchar_t) ;
	*Id = (LPWSTR) CoTaskMemAlloc( nBytes ) ;
	if ( *Id == NULL )
	{
		return	E_OUTOFMEMORY ;
	}
    memmove( *Id, (const wchar_t*) m_strName, nBytes ) ;
	return	NOERROR ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::InputPin::QueryAccept( const AM_MEDIA_TYPE * pmt )
{
	if ( pmt == NULL )
	{
		return	E_POINTER ;
	}
	HRESULT	hr = CheckMediaType( pmt ) ;
	if ( FAILED( hr ) )
	{
		return	S_FALSE ;
	}
	return	hr ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::InputPin::EnumMediaTypes
		( __deref_out IEnumMediaTypes ** ppEnum )
{
	if ( ppEnum == NULL )
	{
		return	E_POINTER ;
	}
	*ppEnum = new SGLDSRenderMediaPlayer::EnumMediaTypes( this ) ;
	return	NOERROR ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::InputPin::QueryInternalConnections
	( __out_ecount_part(*nPin,*nPin) IPin* *apPin, __inout ULONG *nPin )
{
	return	E_NOTIMPL ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::InputPin::EndOfStream( void )
{
	SSmartLock<SCriticalSection>	lock( &(m_pFilter->m_csObj) ) ;
	{
		SSmartLock<SCriticalSection>	lock( &(m_pFilter->m_csSync) ) ;

		HRESULT	hr = CheckStreaming() ;
		if ( hr != NOERROR )
		{
			return	hr ;
		}

		hr = m_pFilter->OnEndOfStream() ;
		if ( SUCCEEDED( hr ) )
		{
			return	S_OK ;
		}
		return	hr ;
	}
}

STDMETHODIMP SGLDSRenderMediaPlayer::InputPin::BeginFlush( void )
{
	SSmartLock<SCriticalSection>	lock( &(m_pFilter->m_csObj) ) ;
	{
		SSmartLock<SCriticalSection>	lock( &(m_pFilter->m_csSync) ) ;
		m_fFlushing = true ;
		//
		m_pFilter->OnBeginFlush() ;
	}
	return	m_pFilter->ResetEndOfStream() ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::InputPin::EndFlush( void )
{
	SSmartLock<SCriticalSection>	lock( &(m_pFilter->m_csObj) ) ;
	{
		SSmartLock<SCriticalSection>	lock( &(m_pFilter->m_csSync) ) ;

		HRESULT	hr = m_pFilter->OnEndFlush() ;
		if ( SUCCEEDED( hr ) )
		{
			m_fFlushing = false ;
			m_fRuntimeError = false ;
		}
		return	hr ;
	}
}

STDMETHODIMP SGLDSRenderMediaPlayer::InputPin::NewSegment
	( REFERENCE_TIME tStart, REFERENCE_TIME tStop, double dRate )
{
	m_rtStart = tStart ;
	m_rtStop = tStop ;
	m_fpRate = dRate ;
	return	S_OK ;
}

// IQualityControl
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP SGLDSRenderMediaPlayer::InputPin::Notify( IBaseFilter * pSender, Quality q )
{
	if ( pSender == NULL )
	{
		return	E_POINTER ;
	}
	return	NOERROR ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::InputPin::SetSink( IQualityControl * piqc )
{
	SSmartLock<SCriticalSection>	lock( &(m_pFilter->m_csSync) ) ;
	m_pQCtrl = piqc ;
	return	NOERROR ;
}

// IMemInputPin
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP SGLDSRenderMediaPlayer::InputPin::GetAllocator
		( __deref_out IMemAllocator ** ppAllocator )
{
	if ( ppAllocator == NULL )
	{
		return	E_POINTER ;
	}
	ESLAssert( m_pFilter != NULL ) ;
	SSmartLock<SCriticalSection>	lock( &(m_pFilter->m_csSync) ) ;
	if ( m_pAlloc == NULL )
	{
		m_pAlloc = CreateAllocator() ;
	}
	*ppAllocator = m_pAlloc ;
	m_pAlloc->AddRef() ;
	return	NOERROR ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::InputPin::NotifyAllocator
		( IMemAllocator * pAllocator, BOOL bReadOnly )
{
	if ( pAllocator == NULL )
	{
		return	E_POINTER ;
	}
	ESLAssert( m_pFilter != NULL ) ;
	SSmartLock<SCriticalSection>	lock( &(m_pFilter->m_csSync) ) ;
	//
	IMemAllocator *	pOldAlloc = m_pAlloc ;
	m_pAlloc = pAllocator ;
	pAllocator->AddRef() ;
	//
	if ( pOldAlloc != NULL )
	{
		pOldAlloc->Release() ;
	}
	m_fReadOnly = (bReadOnly != false) ;
	//
	return	NOERROR ;
}

HRESULT STDMETHODCALLTYPE SGLDSRenderMediaPlayer::InputPin::GetAllocatorRequirements
		( __out  ALLOCATOR_PROPERTIES *pProps )
{
	return	E_NOTIMPL ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::InputPin::Receive( IMediaSample * pSample )
{
//	HRESULT	hr = ReceiveWithoutNotify( pSample ) ;
	HRESULT	hr = m_pFilter->Receive( pSample ) ;
	if ( FAILED( hr ) )
	{
		SSmartLock<SCriticalSection>	lock( &(m_pFilter->m_csObj) ) ;
		if ( !IsStopped() && !m_fFlushing
			&& !m_pFilter->m_fAbort && !m_fRuntimeError )
		{
			m_pFilter->NotifyEvent( EC_ERRORABORT, hr, 0 ) ;
			//
			SSmartLock<SCriticalSection>	lock( &(m_pFilter->m_csSync) ) ;
			if ( m_pFilter->IsStreaming()
				&& !m_pFilter->IsEndOfStreamDelivered() )
			{
				m_pFilter->NotifyEndOfStream() ;
			}
			m_fRuntimeError = true ;
		}
	}
	//
	return	hr ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::InputPin::ReceiveMultiple
		( __in_ecount(nSamples) IMediaSample ** ppSamples,
			long nSamples, __out long * pSamplesProcessed )
{
	if ( ppSamples == NULL )
	{
		return	E_POINTER ;
	}
	HRESULT	hr = S_OK ;
	*pSamplesProcessed = 0 ;
	for ( long i = 0; i < nSamples; i ++ )
	{
		hr = Receive( ppSamples[i] ) ;
		if ( hr != S_OK )
		{
			break ;
		}
		*pSamplesProcessed = i + 1 ;
	}
	return	hr ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::InputPin::ReceiveCanBlock( void )
{
	return	S_OK ;
}



//////////////////////////////////////////////////////////////////////////////
// メディアタイプ列挙実装
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLDSRenderMediaPlayer::EnumMediaTypes::EnumMediaTypes( InputPin * pPin )
{
	m_nRef = 1 ;
	m_pPin = pPin ;
	m_iPos = 0 ;
}

// IUnknown
//////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE SGLDSRenderMediaPlayer::EnumMediaTypes::QueryInterface( REFIID riid, void ** ppObj )
{
    if ( IsEqualIID(riid, IID_IUnknown)
		|| IsEqualIID(riid, IID_IEnumMediaTypes) )
	{
        *ppObj = (IEnumMediaTypes*) this ;
	}
    else
    {
        *ppObj = NULL ;
        return	E_NOINTERFACE ;
    }
    AddRef() ;
    return	S_OK ;
}

ULONG STDMETHODCALLTYPE SGLDSRenderMediaPlayer::EnumMediaTypes::AddRef( void )
{
	return	AtomicAdd( &m_nRef, 1 ) ;
}

ULONG STDMETHODCALLTYPE SGLDSRenderMediaPlayer::EnumMediaTypes::Release( void )
{
	ULONG	nRef = (ULONG) AtomicSub( &m_nRef, 1 ) ;
	if ( nRef == 0 )
	{
		delete	this ;
	}
	return	nRef ;
}

// IEnumMediaTypes
//////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE SGLDSRenderMediaPlayer::EnumMediaTypes::Next
	( ULONG cMediaTypes,
		__out_ecount(cMediaTypes) AM_MEDIA_TYPE ** ppMediaTypes,
		__out_opt ULONG * pcFetched )
{
	if ( ppMediaTypes == NULL )
	{
		return	E_POINTER ;
	}
	if ( pcFetched != NULL )
	{
		*pcFetched = 0 ;
	}
	else if ( cMediaTypes > 1 )
	{
		return	E_INVALIDARG ;
	}
	ULONG	nFetched = 0 ;
	for ( ULONG i = 0; i < cMediaTypes; i ++ )
	{
		AM_MEDIA_TYPE	mt ;
		HRESULT	hr = m_pPin->GetMediaType( m_iPos ++, &mt ) ;
		if ( hr != S_OK )
		{
			break ;
		}
		AM_MEDIA_TYPE *	pmt =
			(AM_MEDIA_TYPE*) CoTaskMemAlloc( sizeof(AM_MEDIA_TYPE) ) ;
		if ( pmt == NULL )
		{
			break ;
		}
		CopyMediaType( pmt, &mt ) ;
		FreeMediaType( mt ) ;
		//
		ppMediaTypes[i] = pmt ;
		nFetched ++ ;
	}
	if ( pcFetched != NULL )
	{
		*pcFetched = nFetched ;
	}
	return	(nFetched == cMediaTypes) ? NOERROR : S_FALSE ;
}

HRESULT STDMETHODCALLTYPE SGLDSRenderMediaPlayer::EnumMediaTypes::Skip( ULONG cMediaTypes )
{
	if ( cMediaTypes == 0 )
	{
		return	S_OK ;
	}
	m_iPos += cMediaTypes ;
	//
	AM_MEDIA_TYPE	mt ;
	HRESULT	hr = m_pPin->GetMediaType( m_iPos - 1, &mt ) ;
	FreeMediaType( mt ) ;
	//
	return	(hr == S_OK) ? S_OK : S_FALSE ;
}

HRESULT STDMETHODCALLTYPE SGLDSRenderMediaPlayer::EnumMediaTypes::Reset( void )
{
	m_iPos = 0 ;
	return	NOERROR ;
}

HRESULT STDMETHODCALLTYPE SGLDSRenderMediaPlayer::EnumMediaTypes::Clone
	( __deref_out IEnumMediaTypes ** ppEnum )
{
	if ( ppEnum == NULL )
	{
		return	E_POINTER ;
	}
	*ppEnum = new EnumMediaTypes( m_pPin ) ;
	return	NOERROR ;
}



//////////////////////////////////////////////////////////////////////////////
// レンダリングフィルタ実装
//////////////////////////////////////////////////////////////////////////////

// {D20A1D95-705D-4454-9B2F-B3D7DDB85F83}
const GUID	SGLDSRenderMediaPlayer::CLSID_DSRenderMediaPlayer_RenderFilter =
	{ 0xd20a1d95, 0x705d, 0x4454, { 0x9b, 0x2f, 0xb3, 0xd7, 0xdd, 0xb8, 0x5f, 0x83 } };

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLDSRenderMediaPlayer::Filter::Filter
	( const wchar_t * pwszName, SGLDSRenderMediaPlayer * pPlayer )
		: m_strName( pwszName ), m_pPlayer( pPlayer )
{
	m_nRef = 1 ;
	m_pPos = NULL ;
	m_pInputPin = NULL ;
	m_pSample = NULL ;
	m_pSink = NULL ;
	m_pClock = NULL ;
	m_pGraph = NULL ;
	m_pQCtrl = NULL ;
	m_state = State_Stopped ;
	//
	m_fAbort = false ;
	m_fStreaming = false ;
	m_fEndOfStream = false ;
	m_fStreamComplete = false ;
	m_fRepaint = true ;
	m_fReceiving = false ;
	m_dwAdviseCookie = 0 ;
	//
	m_sigRunning.Initialize( true ) ;
	m_sigRender.Initialize( false ) ;
	m_sigRenderNoWait.Initialize( true ) ;
	m_idTimerDelayEOS = 0 ;
	m_rtStampSample = 0 ;
	m_nRefTimeOffset = 0 ;
	//
	ResetStreamingTimes() ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLDSRenderMediaPlayer::Filter::~Filter( void )
{
	StopStreaming() ;
	ClearPendingSample() ;
	//
	if ( m_pPos != NULL )
	{
		delete	m_pPos ;
		m_pPos = NULL ;
	}
	if ( m_pInputPin != NULL )
	{
		delete	m_pInputPin ;
		m_pInputPin = NULL ;
	}
	if ( m_pClock != NULL )
	{
		m_pClock->Release() ;
		m_pClock = NULL ;
	}
}

// フレーム取得
//////////////////////////////////////////////////////////////////////////////
SGLImageObject * SGLDSRenderMediaPlayer::Filter::LockVideoFrame( void )
{
	m_csFrame.Lock() ;
	return	&m_imgVideo ;
}

void SGLDSRenderMediaPlayer::Filter::UnlockVideoFrame( void )
{
	m_csFrame.Unlock() ;
}

// PosPassThru 取得
//////////////////////////////////////////////////////////////////////////////
HRESULT SGLDSRenderMediaPlayer::Filter::GetMediaPosition( REFIID riid, __deref_out void **ppv )
{
	HRESULT	hr ;
	SSmartLock<SCriticalSection>	lock( &m_csObj ) ;
	if ( m_pPos != NULL )
	{
		hr = m_pPos->QueryInterface( riid, ppv ) ;
		return	hr ;
	}
	IPin *	pPin = GetPin() ;
	if ( pPin == NULL )
	{
		return	E_OUTOFMEMORY ;
	}
	m_pPos = new PosPassThru( (IBaseFilter*) this, pPin ) ;
	hr = m_pPos->QueryInterface( riid, ppv ) ;
	return	hr ;
}

// InputPin 取得
//////////////////////////////////////////////////////////////////////////////
SGLDSRenderMediaPlayer::InputPin *
	SGLDSRenderMediaPlayer::Filter::GetPin( void )
{
	InputPin *	pPin ;
	SSmartLock<SCriticalSection>	lock( &m_csObj ) ;
	if ( m_pInputPin == NULL )
	{
		m_pInputPin = new InputPin( this, L"In" ) ;
	}
	pPin = m_pInputPin ;
	return	pPin ;
}

// イベント通知
//////////////////////////////////////////////////////////////////////////////
HRESULT SGLDSRenderMediaPlayer::Filter::NotifyEvent
	( long EventCode, LONG_PTR EventParam1, LONG_PTR EventParam2 )
{
	IMediaEventSink * pSink = m_pSink ;
	if ( pSink != NULL )
	{
		if ( EventCode == EC_COMPLETE )
		{
			EventParam2 = (LONG_PTR) (IBaseFilter*) this ;
		}
		return	pSink->Notify( EventCode, EventParam1, EventParam2 ) ;
	}
	else
	{
		return	E_NOTIMPL ;
	}
}

// 時間差の飽和処理
//////////////////////////////////////////////////////////////////////////////
int SGLDSRenderMediaPlayer::Filter::ClampTimeDiff( REFERENCE_TIME rt )
{
	if ( rt < - (50 * UNITS) )
	{
		return	-(50 * UNITS) ;
	}
	else if ( rt > 50 * UNITS )
	{
		return	50 * UNITS ;
	}
	else
	{
		return	(int) rt ;
	}
}

// 接続破棄
//////////////////////////////////////////////////////////////////////////////
HRESULT SGLDSRenderMediaPlayer::Filter::OnBreakConnect( void )
{
	if ( m_pQCtrl != NULL )
	{
		m_pQCtrl->Release() ;
		m_pQCtrl = NULL ;
	}
	if ( !m_pInputPin->IsConnected() )
	{
		return	S_FALSE ;
	}
	if ( m_state != State_Stopped )
	{
		return	VFW_E_NOT_STOPPED ;
	}
	//
	SetRepaintFlag( false ) ;
	ResetEndOfStream() ;
	ClearPendingSample() ;
	m_fAbort = false ;
	//
	if ( m_state == State_Running )
	{
		StopStreaming() ;
	}
	return	NOERROR ;
}

// 接続完了
//////////////////////////////////////////////////////////////////////////////
HRESULT SGLDSRenderMediaPlayer::Filter::OnCompleteConnect( IPin * pReceivePin )
{
	SSmartLock<SCriticalSection>	lock( &m_csObj ) ;

	m_fAbort = false ;

	if ( m_state == State_Running )
	{
		HRESULT	hr = StartStreaming() ;
		if ( FAILED( hr ) )
		{
			return	hr ;
		}
		SetRepaintFlag( false ) ;
	}
	else
	{
		SetRepaintFlag( true ) ;
	}
	return	NOERROR ;
}

// メディアタイプ設定
//////////////////////////////////////////////////////////////////////////////
HRESULT SGLDSRenderMediaPlayer::Filter::OnSetMediaType( const AM_MEDIA_TYPE * pmt )
{
	if ( (pmt->pbFormat == NULL) || (pmt->cbFormat < sizeof(VIDEOINFOHEADER)) )
	{
		return	S_FALSE ;
	}
	memmove( &m_vihHeader, pmt->pbFormat, sizeof(VIDEOINFOHEADER) ) ;
	//
	uint32_t	nFormat = formatImageRGB ;
	uint32_t	nPixelDepth = m_vihHeader.bmiHeader.biBitCount ;
	if ( nPixelDepth < 24 )
	{
		nPixelDepth = 32 ;
	}
	if ( MEDIASUBTYPE_HASALPHA( *pmt ) )
	{
		nFormat = formatImageARGB ;
	}
	ConvertVideoSubTypeGUID
		( m_formatSrcVideo, m_depthSrcVideo, pmt->subtype ) ;
	//
	m_csFrame.Lock() ;
	m_imgVideo.CreateImage
		( m_vihHeader.bmiHeader.biWidth,
			esl_abs( (SDWORD) m_vihHeader.bmiHeader.biHeight ),
			nFormat, nPixelDepth ) ;
	m_csFrame.Unlock() ;
	//
	return	NOERROR ;
}

// 受け入れ可能なメディアタイプ判定
//////////////////////////////////////////////////////////////////////////////
HRESULT SGLDSRenderMediaPlayer::Filter::CheckMediaType( const AM_MEDIA_TYPE * pmt )
{
	if ( !IsEqualGUID( pmt->formattype, FORMAT_VideoInfo ) )
	{
		return	S_FALSE ;
	}
	if ( IsEqualGUID( pmt->majortype, MEDIATYPE_Video ) )
	{
		uint32_t	format, depth ;
		if ( ConvertVideoSubTypeGUID( format, depth, pmt->subtype ) )
		{
			return	S_OK ;
		}
	}
	return	S_FALSE ;
}

// 停止状態から変化時に呼び出される
//////////////////////////////////////////////////////////////////////////////
HRESULT SGLDSRenderMediaPlayer::Filter::OnActive( void )
{
	return	NOERROR ;
}

// 停止状態へ変化時に呼び出される
//////////////////////////////////////////////////////////////////////////////
HRESULT SGLDSRenderMediaPlayer::Filter::OnInactive( void )
{
	if ( m_pPos != NULL )
	{
		m_pPos->ResetMediaTime() ;
	}
	ClearPendingSample() ;
	return	NOERROR ;
}

// ストリーミング開始時
//////////////////////////////////////////////////////////////////////////////
void SGLDSRenderMediaPlayer::Filter::OnStartStreaming( void )
{
	ResetStreamingTimes() ;
}

// ストリーミング停止時
//////////////////////////////////////////////////////////////////////////////
void SGLDSRenderMediaPlayer::Filter::OnStopStreaming( void )
{
	m_timerStreaming.Freeze() ;
}

// ストリーム終端時
//////////////////////////////////////////////////////////////////////////////
HRESULT SGLDSRenderMediaPlayer::Filter::OnEndOfStream( void )
{
	if ( m_state == State_Stopped )
	{
		return	NOERROR ;
	}
	m_fEndOfStream = true ;
	if ( m_pSample != NULL )
	{
		return	NOERROR ;
	}
	m_sigRunning.SetSignal() ;
	if ( m_fStreaming )
	{
		DelayNotifyEndOfStream() ;
		//
		if ( m_pPlayer != NULL )
		{
			m_pPlayer->OnEndOfStream() ;
		}
	}
	return	NOERROR ;
}

// フラッシュ開始
//////////////////////////////////////////////////////////////////////////////
HRESULT SGLDSRenderMediaPlayer::Filter::OnBeginFlush( void )
{
	if ( m_state == State_Paused )
	{
		m_sigRunning.ResetSignal() ;
	}
	EnableRenderWait( false ) ;
	CancelSampleAdvice() ;
	ClearPendingSample() ;
	WaitForReceiveToComplete() ;
	return	NOERROR ;
}

// フラッシュ終了
//////////////////////////////////////////////////////////////////////////////
HRESULT SGLDSRenderMediaPlayer::Filter::OnEndFlush( void )
{
	if ( m_pPos != NULL )
	{
		m_pPos->ResetMediaTime() ;
	}
	EnableRenderWait( true ) ;
	return	NOERROR ;
}

// 描画イベント待ち開始
//////////////////////////////////////////////////////////////////////////////
void SGLDSRenderMediaPlayer::Filter::OnStartWatingRender( void )
{
}

// 描画イベント待ち終了
//////////////////////////////////////////////////////////////////////////////
void SGLDSRenderMediaPlayer::Filter::OnEndWatingRender( void )
{
	REFERENCE_TIME	rtStream ;
	REFERENCE_TIME	rtCurrent = timeGetTime() * 10000 ;
	rtStream = rtCurrent + m_nRefTimeOffset - m_rtStart ;
	if ( m_rtStampSample == 0 )
	{
		m_nLate = 0 ;
		m_nFrame = 0 ;
	}
	else
	{
		m_nLate = (int) (rtStream - m_rtStampSample) ;
		m_nFrame = (int) (rtCurrent - m_rtStampSample) ;
	}
	m_rtStampSample = rtCurrent ;
}

// 描画開始
//////////////////////////////////////////////////////////////////////////////
void SGLDSRenderMediaPlayer::Filter::OnRenderStart( IMediaSample * pms )
{
	RecordFrameLateness( m_nLate, m_nFrame ) ;
	m_timerRendering.Reset() ;
}

// 描画終了
//////////////////////////////////////////////////////////////////////////////
void SGLDSRenderMediaPlayer::Filter::OnRenderEnd( IMediaSample * pms )
{
	int	nRendering =
		(int) eslRoundR64ToLInt( m_timerRendering.GetRealTime() * 10000 ) ;
	if ( (nRendering < m_nRenderAvg * 2)
		|| (nRendering < m_nRenderLast * 2) )
	{
		m_nRenderAvg = (nRendering + m_nRenderAvg * 3) / 4 ;
	}
	m_nRenderLast = nRendering ;
	//
	WaitThrottle() ;
}

// 描画実行
//////////////////////////////////////////////////////////////////////////////
HRESULT SGLDSRenderMediaPlayer::Filter::DoRenderSample( IMediaSample * pms )
{
	HRESULT		hr ;
	LPBYTE		pbSrc = NULL ;
	const long	nActualSize = pms->GetActualDataLength() ;
	hr = pms->GetPointer( &pbSrc ) ;
	if ( SUCCEEDED( hr ) && (pbSrc != NULL) )
	{
		SGLImageBuffer	imgVideoSrc ;
		imgVideoSrc.format = m_formatSrcVideo ;
		imgVideoSrc.width = (uint32_t) m_vihHeader.bmiHeader.biWidth ;
		imgVideoSrc.height =
			(uint32_t) esl_abs( (SDWORD) m_vihHeader.bmiHeader.biHeight ) ;
		imgVideoSrc.depth = m_vihHeader.bmiHeader.biBitCount ;
		imgVideoSrc.pitchPixel = imgVideoSrc.depth >> 3 ;
		imgVideoSrc.pitchLine = nActualSize / (long) imgVideoSrc.height ;
		imgVideoSrc.ptrBuffer = (uint8_t*) pbSrc ;
		//
		if ( (m_vihHeader.bmiHeader.biCompression == BI_RGB)
			&& ((SDWORD) m_vihHeader.bmiHeader.biHeight >= 0) )
		{
			imgVideoSrc.ptrBuffer +=
					imgVideoSrc.pitchLine * (imgVideoSrc.height - 1) ;
			imgVideoSrc.pitchLine = - imgVideoSrc.pitchLine ;
		}
		//
		SGLImageBuffer	imgVideoDst ;
		m_csFrame.Lock() ;
		imgVideoDst.ptrBuffer =
			m_imgVideo.LockBuffer( imgVideoDst, SGLImageObject::lockWrite ) ;
		sglConvertImageBuffer( imgVideoDst, imgVideoSrc ) ;
		m_imgVideo.UnlockBuffer( SGLImageObject::lockWrite ) ;
		m_csFrame.Unlock() ;
		//
		if ( m_pPlayer != NULL )
		{
			m_pPlayer->OnUpdateVideoFrame() ;
		}
	}
	return	hr ;
}

// 描画準備
//////////////////////////////////////////////////////////////////////////////
void SGLDSRenderMediaPlayer::Filter::PrepareRender( void )
{
}

// ストリーミング制御変数初期化
//////////////////////////////////////////////////////////////////////////////
void SGLDSRenderMediaPlayer::Filter::ResetStreamingTimes( void )
{
	m_rtStampSample = 0 ;
	m_rtLastDraw = -1000 ;
	m_nEarliness = 0 ;
	m_nWaitAvg = 0 ;
	m_nFrameAvg = -1 ;
	m_nDuration = 0 ;
	m_nConsecutiveFrames = 0 ;
	//
	m_nRenderAvg = 0 ;
	m_nRenderLast = 0 ;
	m_nThrottle = 0 ;
	m_nDroppedFrames = 0 ;
	m_nDrawnFrames = 0 ;
	//
	m_nLate = 0 ;
	m_nFrame = 0 ;
	m_nTotAcc = 0 ;
	m_nSumSqrLate = 0 ;
	m_nSumFrameTime = 0 ;
	m_nSumSqrFrameTime = 0 ;
	//
	m_timerStreaming.Reset() ;
}

// ストリーミング開始
//////////////////////////////////////////////////////////////////////////////
HRESULT SGLDSRenderMediaPlayer::Filter::StartStreaming( void )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	if ( m_fStreaming )
	{
		return	NOERROR ;
	}
	m_fStreaming = true ;
	//
	timeBeginPeriod( 1 ) ;
	OnStartStreaming() ;
	//
	if ( m_pSample == NULL )
	{
		return	DelayNotifyEndOfStream() ;
	}
	if ( !ScheduleSampleAdvice( m_pSample ) )
	{
		m_sigRender.SetSignal() ;
	}
	return	NOERROR ;
}

// ストリーミング停止
//////////////////////////////////////////////////////////////////////////////
void SGLDSRenderMediaPlayer::Filter::StopStreaming( void )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	m_fStreamComplete = false ;
	//
	if ( m_fStreaming )
	{
		m_fStreaming = false ;
		//
		OnStopStreaming() ;
		timeEndPeriod( 1 ) ;
	}
}

// ストリーミング中判定
//////////////////////////////////////////////////////////////////////////////
bool SGLDSRenderMediaPlayer::Filter::IsStreaming( void )
{
	return	m_fStreaming ;
}

// ストリーム終端判定
//////////////////////////////////////////////////////////////////////////////
bool SGLDSRenderMediaPlayer::Filter::IsEndOfStream( void )
{
	return	m_fEndOfStream ;
}

bool SGLDSRenderMediaPlayer::Filter::IsEndOfStreamDelivered( void )
{
	return	m_fStreamComplete ;
}

// タイマーコールバック関数
//////////////////////////////////////////////////////////////////////////////
void CALLBACK SGLDSRenderMediaPlayer::Filter::TimerProcDelayEndOfStream
	( UINT uTimerID, UINT uMsg,
		DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2 )
{
	SGLDSRenderMediaPlayer::Filter *
		pFilter = (SGLDSRenderMediaPlayer::Filter*) dwUser ;
	//
	SSmartLock<SCriticalSection>	lock( &(pFilter->m_csSync) ) ;
	if ( pFilter->m_idTimerDelayEOS != 0 )
	{
		pFilter->m_idTimerDelayEOS = 0 ;
		pFilter->DelayNotifyEndOfStream() ;
	}
}

// ストリーム終端通知（遅延あり）
//////////////////////////////////////////////////////////////////////////////
HRESULT SGLDSRenderMediaPlayer::Filter::DelayNotifyEndOfStream( void )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	if ( !m_fEndOfStream || m_fStreamComplete || m_idTimerDelayEOS )
	{
		return	NOERROR ;
	}
	if ( m_pClock == NULL )
	{
		return	NotifyEndOfStream() ;
	}
	REFERENCE_TIME	rtStop = m_rtStart + m_rtStop ;
	REFERENCE_TIME	rtCurrent ;
	m_pClock->GetTime( &rtCurrent ) ;
	long	nDelay = (long) ((rtStop - rtCurrent) / 10000) ;
	if ( nDelay < 50 )
	{
		return	NotifyEndOfStream() ;
	}
	m_idTimerDelayEOS =
		timeSetEvent
			( (UINT) nDelay, 10,
				&SGLDSRenderMediaPlayer::Filter::TimerProcDelayEndOfStream,
				(DWORD_PTR) this, TIME_ONESHOT ) ;
	if ( m_idTimerDelayEOS == 0 )
	{
		return	NotifyEndOfStream() ;
	}
	return	NOERROR ;
}

// ストリーム終端通知
//////////////////////////////////////////////////////////////////////////////
HRESULT SGLDSRenderMediaPlayer::Filter::NotifyEndOfStream( void )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	if ( !m_fStreaming )
	{
		return	NOERROR ;
	}
	m_idTimerDelayEOS = 0 ;

	if ( m_pPos != NULL )
	{
		m_pPos->EndOfStream() ;
	}
	m_fStreamComplete = true ;

	return	NotifyEvent
		( EC_COMPLETE, S_OK, (LONG_PTR) ((IBaseFilter *)this) ) ;
}

// ストリーム終了時リセット
//////////////////////////////////////////////////////////////////////////////
HRESULT SGLDSRenderMediaPlayer::Filter::ResetEndOfStream( void )
{
	KillTimerDelayNotifyEOS() ;
	//
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	m_fEndOfStream = false ;
	m_fStreamComplete = false ;
	m_rtStop = 0 ;
	return	NOERROR ;
}

// 遅延通知タイマキャンセル
//////////////////////////////////////////////////////////////////////////////
void SGLDSRenderMediaPlayer::Filter::KillTimerDelayNotifyEOS( void )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	if ( m_idTimerDelayEOS != 0 )
	{
		timeKillEvent( m_idTimerDelayEOS ) ;
		m_idTimerDelayEOS = 0 ;
	}
}

// 受信完了待ち
//////////////////////////////////////////////////////////////////////////////
void SGLDSRenderMediaPlayer::Filter::WaitForReceiveToComplete( void )
{
	for ( ; ; )
	{
		if ( !m_fReceiving )
		{
			break ;
		}
		MSG	msg ;
		PeekMessage( &msg, NULL, WM_NULL, WM_NULL, PM_NOREMOVE ) ;
		SleepMilliSec( 1 ) ;
	}
	if ( HIWORD( GetQueueStatus(QS_POSTMESSAGE) ) & QS_POSTMESSAGE )
	{
		PostThreadMessage( GetCurrentThreadId(), WM_NULL, 0, 0 ) ;
	}
}

// 描画イベント待機
//////////////////////////////////////////////////////////////////////////////
HRESULT SGLDSRenderMediaPlayer::Filter::WaitForRenderSignal( void )
{
	HANDLE	hEvents[2] =
	{
		m_sigRender.GetHandle(),
		m_sigRenderNoWait.GetHandle()
	} ;

	OnStartWatingRender() ;

	DWORD	dwWaitResult ;
	for ( ; ; )
	{
		dwWaitResult = WaitForMultipleObjects( 2, hEvents, FALSE, 100 ) ;
		if ( dwWaitResult != WAIT_TIMEOUT )
		{
			break ;
		}
	}

	OnEndWatingRender() ;

	if ( dwWaitResult == WAIT_OBJECT_0 + 1 )
	{
		return	VFW_E_STATE_CHANGED ;
	}
	m_dwAdviseCookie = 0 ;
	return	NOERROR ;
}

// 描画イベント待機設定
//////////////////////////////////////////////////////////////////////////////
void SGLDSRenderMediaPlayer::Filter::EnableRenderWait( bool fRenderWait )
{
	if ( fRenderWait )
	{
		m_sigRenderNoWait.ResetSignal() ;
	}
	else
	{
		m_sigRenderNoWait.SetSignal() ;
	}
}

// スロットリング待機
//////////////////////////////////////////////////////////////////////////////
void SGLDSRenderMediaPlayer::Filter::WaitThrottle( void )
{
	if ( m_nThrottle > 0 )
	{
		DWORD	msWait = (DWORD) m_nThrottle / 10000 ;
		Sleep( msWait ) ;
	}
	else
	{
		Sleep( 0 ) ;
	}
}

// 描画時間イベント発生時処理 SignalTimerFired
//////////////////////////////////////////////////////////////////////////////
void SGLDSRenderMediaPlayer::Filter::OnSignalRenderEvent( void )
{
	m_dwAdviseCookie = 0 ;
}

// 描画時間イベントのスケジュール
//////////////////////////////////////////////////////////////////////////////
bool SGLDSRenderMediaPlayer::Filter::ScheduleSampleAdvice( IMediaSample * pms )
{
	if ( pms == NULL )
	{
		m_nDroppedFrames ++ ;
		return	false ;
	}
	REFERENCE_TIME	rtStart, rtEnd ;
	HRESULT	hr = GetSampleTimes( pms, &rtStart, &rtEnd ) ;
	if ( FAILED( hr ) )
	{
		m_nDroppedFrames ++ ;
		return	false ;
	}
	if ( hr == S_OK )
	{
		m_sigRender.SetSignal() ;
		return	true ;
	}
	ESLAssert( m_pClock != NULL ) ;
	ESLAssert( m_dwAdviseCookie == 0 ) ;
	hr = m_pClock->AdviseTime
		( m_rtStampSample, m_rtStart,
			(HANDLE) m_sigRender.GetHandle(), &m_dwAdviseCookie ) ;
	if ( SUCCEEDED( hr ) )
	{
		return	true ;
	}
	m_nDroppedFrames ++ ;
	return	false ;
}

// 描画時間イベント通知のキャンセル
//////////////////////////////////////////////////////////////////////////////
HRESULT SGLDSRenderMediaPlayer::Filter::CancelSampleAdvice( void )
{
	DWORD_PTR	dwCookie = m_dwAdviseCookie ;
	if ( m_dwAdviseCookie != 0 )
	{
		m_pClock->Unadvise( m_dwAdviseCookie ) ;
		m_dwAdviseCookie = 0 ;
	}
	m_sigRender.ResetSignal() ;
	return	((dwCookie != 0) ? S_OK : S_FALSE) ;
}

// サンプル時間取得
//////////////////////////////////////////////////////////////////////////////
HRESULT SGLDSRenderMediaPlayer::Filter::GetSampleTimes
	( IMediaSample * pms,
		__out REFERENCE_TIME * prtStart, __out REFERENCE_TIME * prtEnd )
{
	ESLAssert( m_dwAdviseCookie == 0 ) ;
	ESLAssert( pms != NULL ) ;

	if ( SUCCEEDED( pms->GetTime( prtStart, prtEnd ) ) )
	{
		if ( *prtEnd < *prtStart )
		{
			return	VFW_E_START_TIME_AFTER_END ;
		}
	}
	else
	{
		return	S_OK ;
	}
	if ( m_pClock == NULL )
	{
		return	S_OK ;
	}
	return	ShouldSampleDrawImmediately( pms, prtStart, prtEnd ) ;
}

// 即時描画すべきか？
//////////////////////////////////////////////////////////////////////////////
HRESULT SGLDSRenderMediaPlayer::Filter::ShouldSampleDrawImmediately
	( IMediaSample * pms,
		__out REFERENCE_TIME * prtStart, __out REFERENCE_TIME * prtEnd )
{
	ESLAssert( m_pClock != NULL ) ;

	if ( *prtStart >= 80000 )
	{
		*prtStart -= 80000 ;
		*prtEnd -= 80000 ;
	}
	m_rtStampSample = *prtStart ;

	REFERENCE_TIME	rtStream ;
	m_pClock->GetTime( &rtStream ) ;
	m_nRefTimeOffset = rtStream - timeGetTime() * 10000 ;
	rtStream -= m_rtStart ;

	const int	nLate = ClampTimeDiff( rtStream - *prtStart ) ;
	HRESULT		hr = NotifyQuality( nLate, rtStream ) ;
	const bool	fHandleQuality = (hr == S_OK) ;
	const int	nDuration = (int) (*prtEnd - *prtStart) ;

	if ( (nDuration < (m_nDuration - m_nDuration/32))
		|| ((m_nDuration + (m_nDuration/32)) > nDuration) )
	{
		m_nFrameAvg = nDuration ;
		m_nDuration = nDuration ;
	}
	bool	fDroppedFrame =
				(fHandleQuality
					&& (pms->IsDiscontinuity() == S_OK))
				|| (m_nConsecutiveFrames == -1) ;

	if ( nLate > 0 )
	{
		m_nEarliness = 0 ;
	}
	else if ( (nLate >= m_nEarliness) || fDroppedFrame )
	{
		m_nEarliness = nLate ;
	}
	else
	{
		m_nEarliness -= m_nEarliness / 8 ;
	}
	//
	int	nWaitAvg = (nLate < 0) ? -nLate : 0 ;
	nWaitAvg = (nWaitAvg + m_nWaitAvg * 3) / 4 ;
	//
	int	nFrame = (int) (rtStream - m_rtLastDraw) ;
	if ( rtStream - m_rtLastDraw > 10000000 )
	{
		nFrame = 10000000 ;
	}
	//
	if ( (m_nRenderAvg * 3 <= m_nFrameAvg)
		|| (fHandleQuality ? (nLate <= nDuration * 4)
								: (nLate * 2 < nDuration))
		|| (m_nWaitAvg > 80000)
		|| ((rtStream - m_rtLastDraw) > UNITS) )
	{
		HRESULT	hr ;
		bool	fPlayASAP = false ;
		if ( fDroppedFrame )
		{
			fPlayASAP = true ;
		}
		else if ( (m_nFrameAvg > nDuration + nDuration / 16)
				&& (nLate > nDuration * -10) )
		{
			fPlayASAP = true ;
		}
		else if ( (nLate + nDuration > 0) && (m_nWaitAvg <= 20000) )
		{
			fPlayASAP = true ;
		}
		if ( nLate < -9000000 )
		{
			fPlayASAP = false ;
		}
		if ( fPlayASAP )
		{
			m_nConsecutiveFrames = 0 ;
			m_nWaitAvg = m_nWaitAvg * 3 / 4 ;
			m_nFrameAvg = (nFrame + m_nFrameAvg * 3) / 4 ;
			m_rtLastDraw = rtStream ;
			m_nLate = nLate ;
			m_nFrame = nFrame ;
			if ( m_nEarliness > nLate )
			{
				m_nEarliness = nLate ;
			}
			hr = S_OK ;
		}
		else
		{
			m_nConsecutiveFrames ++ ;
			m_nFrameAvg = nDuration ;
			//
			if ( m_nEarliness < -m_nFrameAvg )
			{
				*prtStart += -m_nFrameAvg ;
			}
			else
			{
				*prtStart += m_nEarliness ;
			}
			int	nDelay = - nLate ;
			//
			m_nWaitAvg = nWaitAvg ;
			//
			if ( nDelay > 0 )
			{
				hr = S_FALSE ;
				nFrame = ClampTimeDiff( *prtStart - m_rtLastDraw ) ;
				m_rtLastDraw = *prtStart ;
			}
			else
			{
				hr = S_OK ;
				m_rtLastDraw = rtStream ;
			}
			int	nAccuracy = nLate ;
			if ( nDelay > 0 )
			{
				nAccuracy = ClampTimeDiff( *prtStart - m_rtStampSample ) ;
			}
			m_nLate = nAccuracy ;
			m_nFrame = nFrame ;
		}
		return	hr ;
	}
	m_nWaitAvg = nWaitAvg ;
	m_nConsecutiveFrames = -1 ;
	return	E_FAIL ;				// フレームドロップ
}

// 再描画フラグ設定
//////////////////////////////////////////////////////////////////////////////
void SGLDSRenderMediaPlayer::Filter::SetRepaintFlag( bool fRepaint )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	m_fRepaint = fRepaint ;
}

// 再描画イベントを通知
//////////////////////////////////////////////////////////////////////////////
void SGLDSRenderMediaPlayer::Filter::NotifyRepaintEvent( void )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	ESLAssert( m_pInputPin != NULL ) ;
	//
	if ( !m_fAbort
		&& m_pInputPin->IsConnected()
		&& !m_pInputPin->IsFlushing()
		&& !IsEndOfStream() && m_fRepaint )
	{
		NotifyEvent( EC_REPAINT, (LONG_PTR) ((IPin*) m_pInputPin), 0 ) ;
		SetRepaintFlag( false ) ;
	}
}

// クオリティ通知
//////////////////////////////////////////////////////////////////////////////
HRESULT SGLDSRenderMediaPlayer::Filter::NotifyQuality( REFERENCE_TIME rtLate, REFERENCE_TIME rtStream )
{
	Quality	q ;

	q.TimeStamp = rtStream ;

	if ( m_nFrameAvg < 0 )
	{
		q.Type = Famine ;
	}
	else if ( m_nFrameAvg > m_nRenderAvg * 2 )
	{
		q.Type = Famine ;
	}
	else
	{
		q.Type = Flood ;
	}
	q.Proportion = 1000 ;

	if ( m_nFrameAvg < 0 )
	{
	}
	else if ( rtLate > 0 )
	{
		q.Proportion = 1000 - (int) ((rtLate) / (UNITS/1000)) ;
		if ( q.Proportion < 500 )
		{
			q.Proportion = 500 ;
		}
	}
	else if ( (m_nWaitAvg > 20000) && (rtLate < -20000) )
	{
		if ( m_nWaitAvg >= m_nFrameAvg )
		{
			q.Proportion = 2000 ;
		}
		else
		{
			if ( m_nFrameAvg + 20000 > m_nWaitAvg )
			{
				q.Proportion =
					1000 * (m_nFrameAvg
							/ (m_nFrameAvg + 20000 - m_nWaitAvg)) ;
			}
			else
			{
				q.Proportion = 2000 ;
			}
		}
		if ( q.Proportion > 2000 )
		{
			q.Proportion = 2000 ;
		}
	}

	q.Late = rtLate + m_nRenderAvg / 2 ;

	if ( m_pQCtrl == NULL )
	{
		IPin *	pOutPin = m_pInputPin->GetConnected() ;
		if ( pOutPin != NULL )
		{
			pOutPin->QueryInterface
				( IID_IQualityControl, (void**) &m_pQCtrl ) ;
		}
	}
	if ( m_pQCtrl != NULL )
	{
		return	m_pQCtrl->Notify( (IBaseFilter*) this, q ) ;
	}
	return	S_FALSE ;
}

// 遅延記録
//////////////////////////////////////////////////////////////////////////////
void SGLDSRenderMediaPlayer::Filter::RecordFrameLateness( int nLate, int nFrame )
{
	int	msLate = nLate / 10000 ;
	if ( (msLate > 1000) || (msLate < -1000) )
	{
		if ( m_nDrawnFrames <= 1 )
		{
			msLate = 0 ;
		}
		else if ( msLate > 0 )
		{
			msLate = 1000 ;
		}
		else
		{
			msLate = -1000 ;
		}
	}
	if ( m_nDrawnFrames > 1 )
	{
		m_nTotAcc += msLate ;
		m_nSumSqrLate += msLate * msLate ;
	}
	if ( m_nDrawnFrames > 2 )
	{
		int	msFrame = nFrame / 10000 ;
		if ( (msFrame > 1000) || (msFrame < 0) )
		{
			msFrame = 1000 ;
		}
		m_nSumFrameTime += msFrame ;
		m_nSumSqrFrameTime += msFrame * msFrame ;
	}
	m_nDrawnFrames ++ ;
}

// ステータス変化完了処理
//////////////////////////////////////////////////////////////////////////////
HRESULT SGLDSRenderMediaPlayer::Filter::CompleteStateChange( FILTER_STATE stateOld )
{
	if ( !m_pInputPin->IsConnected() || IsEndOfStream() )
	{
		m_sigRunning.SetSignal() ;
		return	S_OK ;
	}
	if ( HaveCurrentSample()
		&& (stateOld != State_Stopped) )
	{
		m_sigRunning.SetSignal() ;
		return	S_OK ;
	}
	m_sigRunning.ResetSignal() ;
	return	S_FALSE ;
}

// サンプル解放
//////////////////////////////////////////////////////////////////////////////
void SGLDSRenderMediaPlayer::Filter::ClearPendingSample( void )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	if ( m_pSample != NULL )
	{
		m_pSample->Release() ;
		m_pSample = NULL ;
	}
}

// フレーム標準偏差
//////////////////////////////////////////////////////////////////////////////
HRESULT SGLDSRenderMediaPlayer::Filter::GetStandardDeviations
	( int nSamples, __out int *piResult,
			LONGLONG nSumSq, LONGLONG nTot )
{
	if ( piResult == NULL )
	{
		return	E_POINTER ;
	}
	SSmartLock<SCriticalSection>	lock( &m_csObj ) ;
	if ( m_pClock == NULL )
	{
		*piResult = 0 ;
		return	NOERROR ;
	}
	if ( nSamples <= 1 )
	{
		*piResult = 0 ;
	}
	else
	{
		*piResult =
			(int) eslRoundR64ToLInt
				( sqrt( (nSumSq - ((double) nTot * nTot / nSamples))
													/ (nSamples - 1) ) ) ;
	}
	return	NOERROR ;
}

// 先頭サンプル受信事
//////////////////////////////////////////////////////////////////////////////
void SGLDSRenderMediaPlayer::Filter::OnReceiveFirstSample( IMediaSample * pms )
{
}

// サンプル受け取り準備
//////////////////////////////////////////////////////////////////////////////
HRESULT SGLDSRenderMediaPlayer::Filter::PrepareReceive( IMediaSample * pms )
{
	SSmartLock<SCriticalSection>	lockObj( &m_csObj ) ;
	m_fReceiving = true ;

	HRESULT	hr = m_pInputPin->ReceiveWithoutNotify( pms ) ;
	if ( hr != NOERROR )
	{
		m_fReceiving = false ;
		return	E_FAIL ;
	}
	if ( m_pInputPin->SampleProps().pMediaType != NULL )
	{
		hr = m_pInputPin->SetMediaType
			( m_pInputPin->SampleProps().pMediaType ) ;
		if ( FAILED( hr ) )
		{
			m_fReceiving = false ;
			return	hr ;
		}
	}

	SSmartLock<SCriticalSection>	lockSync( &m_csSync ) ;

	if ( (m_pSample != NULL) || m_fEndOfStream || m_fAbort )
	{
		m_sigRunning.SetSignal() ;
		m_fReceiving = false ;
		return	E_UNEXPECTED ;
	}
	if ( m_pPos != NULL )
	{
		m_pPos->RegisterMediaTime( pms ) ;
	}
	if ( m_fStreaming && !ScheduleSampleAdvice( pms ) )
	{
		m_fReceiving = false ;
		return	VFW_E_SAMPLE_REJECTED ;
	}
	m_rtStop = m_pInputPin->SampleProps().tStop ;
	m_pSample = pms ;
	pms->AddRef() ;

	if ( !m_fStreaming )
	{
		SetRepaintFlag( true ) ;
	}
	return	NOERROR ;
}

HRESULT SGLDSRenderMediaPlayer::Filter::Receive( IMediaSample * pms )
{
	HRESULT	hr = PrepareReceive( pms ) ;
	if ( FAILED( hr ) )
	{
		if ( hr == VFW_E_SAMPLE_REJECTED )
		{
			return	NOERROR ;
		}
		return	hr ;
	}
	if ( m_state == State_Paused )
	{
		PrepareRender() ;
		m_fReceiving = false ;
		{
			SSmartLock<SCriticalSection>	lockObj( &m_csObj ) ;
			if ( m_state == State_Stopped )
			{
				return	NOERROR ;
			}
			m_fReceiving = true ;
			//
			SSmartLock<SCriticalSection>	lockSync( &m_csSync ) ;
			OnReceiveFirstSample( pms ) ;
		}
		m_sigRunning.SetSignal() ;
	}
	hr = WaitForRenderSignal() ;
	if ( FAILED( hr ) )
	{
		m_fReceiving = false ;
		return	NOERROR ;
	}
	//
	PrepareRender() ;
	m_fReceiving = false ;
	//
	SSmartLock<SCriticalSection>	lockObj( &m_csObj ) ;
	if ( m_state == State_Stopped )
	{
		return	NOERROR ;
	}
	SSmartLock<SCriticalSection>	lockSync( &m_csSync ) ;
	Render( pms ) ;
	ClearPendingSample() ;
	DelayNotifyEndOfStream() ;
	CancelSampleAdvice() ;
	return	NOERROR ;
}

// サンプルがあるか？
//////////////////////////////////////////////////////////////////////////////
bool SGLDSRenderMediaPlayer::Filter::HaveCurrentSample( void )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	return	(m_pSample != NULL) ;
}

// サンプル取得
//////////////////////////////////////////////////////////////////////////////
IMediaSample * SGLDSRenderMediaPlayer::Filter::GetCurrentSample( void )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	if ( m_pSample != NULL )
	{
		m_pSample->AddRef() ;
	}
	return	m_pSample ;
}

// 描画処理
//////////////////////////////////////////////////////////////////////////////
HRESULT SGLDSRenderMediaPlayer::Filter::Render( IMediaSample * pms )
{
	if ( pms == NULL )
	{
		return	S_FALSE ;
	}
	if ( !m_fStreaming )
	{
		return	S_FALSE ;
	}
	OnRenderStart( pms ) ;
	DoRenderSample( pms ) ;
	OnRenderEnd( pms ) ;
	return	NOERROR ;
}

// IUnknown
//////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE SGLDSRenderMediaPlayer::Filter::QueryInterface( REFIID riid, void ** ppObj )
{
    if ( IsEqualIID(riid, IID_IBaseFilter) )
	{
        *ppObj = (IBaseFilter*) this ;
	}
    else if ( IsEqualIID(riid, IID_IMediaFilter) )
	{
        *ppObj = (IMediaFilter*) this ;
	}
    else if ( IsEqualIID(riid, IID_IPersist) )
	{
        *ppObj = (IPersist*) this ;
	}
    else if ( IsEqualIID(riid, IID_IUnknown) )
	{
        *ppObj = (IUnknown*) ((IBaseFilter*) this) ;
	}
	else if ( IsEqualIID(riid, IID_IAMovieSetup) )
	{
        *ppObj = (IAMovieSetup*) this ;
	}
	else if ( IsEqualIID(riid, IID_IQualProp) )
	{
        *ppObj = (IQualProp*) this ;
	}
	else if ( IsEqualIID(riid, IID_IQualityControl) )
	{
        *ppObj = (IQualityControl*) this ;
	}
	else if ( IsEqualIID(riid, IID_IMediaPosition)
			|| IsEqualIID(riid, IID_IMediaSeeking)
			|| IsEqualIID(riid, IID_IDispatch) )
	{
		return	GetMediaPosition( riid, ppObj ) ;
	}
    else
    {
        *ppObj = NULL ;
        return	E_NOINTERFACE ;
    }
    AddRef() ;
    return	S_OK ;
}

ULONG STDMETHODCALLTYPE SGLDSRenderMediaPlayer::Filter::AddRef( void )
{
	return	AtomicAdd( &m_nRef, 1 ) ;
}

ULONG STDMETHODCALLTYPE SGLDSRenderMediaPlayer::Filter::Release( void )
{
	ULONG	nRef = (ULONG) AtomicSub( &m_nRef, 1 ) ;
	if ( nRef == 0 )
	{
		delete	this ;
	}
	return	nRef ;
}

// IPersist
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP SGLDSRenderMediaPlayer::Filter::GetClassID( __out CLSID *pClsID )
{
    if ( pClsID == NULL )
	{
		return	E_POINTER ;
	}
    *pClsID = CLSID_DSRenderMediaPlayer_RenderFilter ;
    return	NOERROR ;
}

// IMediaFilter
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP SGLDSRenderMediaPlayer::Filter::GetState( DWORD dwMSecs, __out FILTER_STATE * pState )
{
	if ( pState == NULL )
	{
		return	E_POINTER ;
	}
	if ( m_sigRunning.Wait
		( (dwMSecs == INFINITE) ? Synchronism::Infinite : dwMSecs ) == errTimeout )
	{
		*pState = m_state ;
		return	VFW_S_STATE_INTERMEDIATE ;
	}
	*pState = m_state ;
    return	NOERROR ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::Filter::SetSyncSource( __in_opt IReferenceClock *pClock )
{
	SSmartLock<SCriticalSection>	lock( &m_csObj ) ;
	if ( pClock != NULL)
	{
		pClock->AddRef() ;
	}
	if ( m_pClock != NULL )
	{
		m_pClock->Release() ;
	}
	m_pClock = pClock ;
	return	NOERROR ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::Filter::GetSyncSource( __deref_out_opt IReferenceClock **pClock )
{
	if ( pClock == NULL )
	{
		return	E_POINTER ;
	}
	SSmartLock<SCriticalSection>	lock( &m_csObj ) ;
	if ( m_pClock != NULL )
	{
		m_pClock->AddRef() ;
	}
	*pClock = m_pClock ;
	return	NOERROR ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::Filter::Stop( void )
{
	SSmartLock<SCriticalSection>	lock( &m_csObj ) ;
	if ( m_state == State_Stopped )
	{
		return	NOERROR ;
	}
	if ( !m_pInputPin->IsConnected() )
	{
		m_state = State_Stopped ;
		return	NOERROR ;
	}
	m_pInputPin->Inactive() ;
	m_state = State_Stopped ;
	//
	if ( m_pInputPin->GetAllocator() != NULL )
	{
		m_pInputPin->GetAllocator()->Decommit() ;
	}
	//
	SetRepaintFlag( true ) ;
	StopStreaming() ;
	EnableRenderWait( false ) ;
	ResetEndOfStream() ;
	CancelSampleAdvice() ;
	//
	m_sigRunning.SetSignal() ;
	WaitForReceiveToComplete() ;
	m_fAbort = false ;
	//
	return	NOERROR ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::Filter::Pause( void )
{
	SSmartLock<SCriticalSection>	lock( &m_csObj ) ;
	//
	FILTER_STATE	stateOld = m_state ;
	if ( m_state == State_Paused )
	{
		return	CompleteStateChange( stateOld ) ;
	}
	if ( !m_pInputPin->IsConnected() )
	{
		m_state = State_Paused ;
		return	CompleteStateChange( stateOld ) ;
	}
	if ( m_state == State_Stopped )
	{
		if ( m_pInputPin->IsConnected() )
		{
			HRESULT	hr = m_pInputPin->Active() ;
			if ( FAILED( hr ) )
			{
				return	hr ;
			}
		}
	}
	m_state = State_Paused ;
	//
	SetRepaintFlag( true ) ;
	StopStreaming() ;
	EnableRenderWait( true ) ;
	ResetEndOfStream() ;
	CancelSampleAdvice() ;
	//
	if ( m_pInputPin->GetAllocator() != NULL )
	{
		m_pInputPin->GetAllocator()->Decommit() ;
	}
	if ( stateOld == State_Stopped )
	{
		m_fAbort = false ;
		ClearPendingSample() ;
	}
	return	CompleteStateChange( stateOld ) ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::Filter::Run( REFERENCE_TIME tStart )
{
	SSmartLock<SCriticalSection>	lock( &m_csObj ) ;
	//
	FILTER_STATE	stateOld = m_state ;
	if ( m_state == State_Running )
	{
		return	NOERROR ;
	}
	if ( !m_pInputPin->IsConnected() )
	{
		NotifyEvent( EC_COMPLETE, S_OK, (LONG_PTR) ((IBaseFilter*) this) ) ;
		m_state = State_Running ;
		return	NOERROR ;
	}
	m_sigRunning.SetSignal() ;
	//
	m_rtStart = tStart ;
	if ( m_state == State_Stopped )
	{
		HRESULT	hr = Pause() ;
		if ( FAILED( hr ) )
		{
			return	hr ;
		}
	}
	if ( m_state != State_Running )
	{
		InputPin *	pPin = GetPin() ;
		if ( pPin != NULL )
		{
			if ( pPin->IsConnected() )
			{
				HRESULT	hr = pPin->Run( tStart ) ;
				if ( FAILED( hr ) )
				{
					return	hr ;
				}
			}
		}
	}
	m_state = State_Running ;
	//
	EnableRenderWait( true ) ;
	SetRepaintFlag( false ) ;
	//
	if ( m_pInputPin->GetAllocator() != NULL )
	{
		m_pInputPin->GetAllocator()->Commit() ;
	}
	if ( stateOld == State_Stopped )
	{
		m_fAbort = false ;
		ClearPendingSample() ;
	}
	return	StartStreaming() ;
}

// IBaseFilter
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP SGLDSRenderMediaPlayer::Filter::EnumPins( __deref_out IEnumPins ** ppEnum )
{
	if ( ppEnum == NULL )
	{
		return	E_POINTER ;
	}
	*ppEnum = new SGLDSRenderMediaPlayer::EnumPins( this ) ;
	return	NOERROR ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::Filter::FindPin( LPCWSTR Id, __deref_out IPin ** ppPin )
{
	SSmartLock<SCriticalSection>	lock( &m_csObj ) ;
	InputPin *	pPin = GetPin() ;
	if ( pPin != NULL )
	{
		if ( pPin->GetName() == Id )
		{
			*ppPin = pPin ;
			pPin->AddRef() ;
			return	S_OK ;
		}
	}
	*ppPin = NULL ;
	return	VFW_E_NOT_FOUND ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::Filter::QueryFilterInfo( __out FILTER_INFO * pInfo )
{
	if ( pInfo == NULL )
	{
		return	E_POINTER ;
	}
	if ( !m_strName.IsEmpty() )
	{
        StringCchCopyW( pInfo->achName, NUMELMS(pInfo->achName), m_strName ) ;
	}
	else
	{
        pInfo->achName[0] = 0 ;
	}
	pInfo->pGraph = m_pGraph ;
	if ( m_pGraph != NULL )
	{
		m_pGraph->AddRef() ;
	}
	return	NOERROR ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::Filter::JoinFilterGraph
	( __inout_opt IFilterGraph * pGraph, __in_opt LPCWSTR pName )
{
	if ( (pGraph == NULL) && (m_pGraph != NULL) )
	{
        NotifyEvent
			( EC_WINDOW_DESTROYED, (LPARAM) ((IBaseFilter*) this), 0 ) ;
	}
	SSmartLock<SCriticalSection>	lock( &m_csObj ) ;
	m_pGraph = pGraph ;
	if ( m_pGraph != NULL )
	{
		HRESULT	hr = m_pGraph->QueryInterface
						( IID_IMediaEventSink, (void**) &m_pSink ) ;
		if ( FAILED( hr ) )
		{
			m_pSink = NULL ;
		}
		else
		{
			m_pSink->Release() ;
		}
	}
	else
	{
		m_pSink = NULL ;
	}
	m_strName = pName ;
	return	NOERROR ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::Filter::QueryVendorInfo( __deref_out LPWSTR* pVendorInfo )
{
	return	E_NOTIMPL ;
}

// IAMovieSetup
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP SGLDSRenderMediaPlayer::Filter::Register( void )
{
	return	NOERROR ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::Filter::Unregister( void )
{
	return	NOERROR ;
}

// IQualProp
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP SGLDSRenderMediaPlayer::Filter::get_FramesDroppedInRenderer( __out int * cFramesDropped )
{
	if ( cFramesDropped == NULL )
	{
		return	E_POINTER ;
	}
	SSmartLock<SCriticalSection>	lock( &m_csObj ) ;
	*cFramesDropped = m_nDroppedFrames ;
	return	NOERROR ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::Filter::get_FramesDrawn( __out int * pcFramesDrawn )
{
	if ( pcFramesDrawn == NULL )
	{
		return	E_POINTER ;
	}
	SSmartLock<SCriticalSection>	lock( &m_csObj ) ;
	*pcFramesDrawn = m_nDrawnFrames ;
	return	NOERROR ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::Filter::get_AvgFrameRate( __out int * piAvgFrameRate )
{
	if ( piAvgFrameRate == NULL )
	{
		return	E_POINTER ;
	}
	SSmartLock<SCriticalSection>	lock( &m_csObj ) ;
	double	t = m_timerStreaming.GetRealTime() ;
	if ( t >= 1.0e-8 )
	{
		*piAvgFrameRate =
			(int) eslRoundR64ToLInt( 100000 * m_nDrawnFrames / t ) ;
	}
	else
	{
		*piAvgFrameRate = 0 ;
	}
	return	NOERROR ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::Filter::get_Jitter( __out int * piJitter )
{
	return	GetStandardDeviations
				( m_nDrawnFrames - 2, piJitter, m_nSumSqrFrameTime, m_nSumFrameTime ) ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::Filter::get_AvgSyncOffset( __out int * piAvg )
{
	if ( piAvg == NULL )
	{
		return	E_POINTER ;
	}
	SSmartLock<SCriticalSection>	lock( &m_csObj ) ;
	if ( m_pClock == NULL )
	{
		*piAvg = 0 ;
		return	NOERROR ;
	}
	if ( m_nDrawnFrames <= 1 )
	{
		*piAvg = 0 ;
	}
	else
	{
		*piAvg = m_nTotAcc / (m_nDrawnFrames - 1) ;
	}
	return	NOERROR ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::Filter::get_DevSyncOffset( __out int * piDev )
{
	return	GetStandardDeviations
				( m_nDrawnFrames - 1, piDev, m_nSumSqrLate, m_nTotAcc ) ;
}

// IQualityControl
//////////////////////////////////////////////////////////////////////////////
STDMETHODIMP SGLDSRenderMediaPlayer::Filter::SetSink( IQualityControl * piqc )
{
	m_pQCtrl = piqc ;
	return	NOERROR ;
}

STDMETHODIMP SGLDSRenderMediaPlayer::Filter::Notify( IBaseFilter * pSelf, Quality q )
{
	if ( q.Proportion >= 1000 )
	{
		m_nThrottle = 0 ;
	}
	else
	{
		m_nThrottle = -330000 + (388880000 / (q.Proportion + 167)) ;
	}
	return	NOERROR ;
}



//////////////////////////////////////////////////////////////////////////////
// Pin 列挙実装
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLDSRenderMediaPlayer::EnumPins::EnumPins( Filter * pFilter )
{
	m_nRef = 1 ;
	m_pFilter = pFilter ;
	m_iPos = 0 ;
	//
	pFilter->AddRef() ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLDSRenderMediaPlayer::EnumPins::~EnumPins( void )
{
	m_pFilter->Release() ;
}

// IUnknown
//////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE SGLDSRenderMediaPlayer::EnumPins::QueryInterface( REFIID riid, void ** ppObj )
{
    if ( IsEqualIID(riid, IID_IUnknown)
		|| IsEqualIID(riid, IID_IEnumPins) )
	{
        *ppObj = (IEnumPins*) this ;
	}
    else
    {
        *ppObj = NULL ;
        return	E_NOINTERFACE ;
    }
    AddRef() ;
    return	S_OK ;
}

ULONG STDMETHODCALLTYPE SGLDSRenderMediaPlayer::EnumPins::AddRef( void )
{
	return	AtomicAdd( &m_nRef, 1 ) ;
}

ULONG STDMETHODCALLTYPE SGLDSRenderMediaPlayer::EnumPins::Release( void )
{
	ULONG	nRef = (ULONG) AtomicSub( &m_nRef, 1 ) ;
	if ( nRef == 0 )
	{
		delete	this ;
	}
	return	nRef ;
}

// IEnumPins
//////////////////////////////////////////////////////////////////////////////
HRESULT STDMETHODCALLTYPE SGLDSRenderMediaPlayer::EnumPins::Next
( ULONG cPins, __out_ecount_part(cPins, *pcFetched) IPin ** ppPins, __out_opt ULONG * pcFetched )
{
	if ( ppPins == NULL )
	{
		return	E_POINTER ;
	}
	if ( pcFetched != NULL )
	{
		*pcFetched = 0 ;
	}
	else if ( cPins > 1 )
	{
		return	E_INVALIDARG ;
	}
	if ( m_iPos ++ != 0 )
	{
		return	S_FALSE ;
	}
	InputPin *	pPin = m_pFilter->GetPin() ;
	if ( pPin != NULL )
	{
		pPin->AddRef() ;
	}
	ppPins[0] = pPin ;
	//
	if ( pcFetched != NULL )
	{
		*pcFetched = 1 ;
	}
	return	(cPins == 1) ? NOERROR : S_FALSE ;
}

HRESULT STDMETHODCALLTYPE SGLDSRenderMediaPlayer::EnumPins::Skip( ULONG cPins )
{
	if ( cPins == 0 )
	{
		return	S_OK ;
	}
	m_iPos += cPins ;
	return	(m_iPos <= 1) ? S_OK : S_FALSE ;
}

HRESULT STDMETHODCALLTYPE SGLDSRenderMediaPlayer::EnumPins::Reset( void )
{
	m_iPos = 0 ;
	return	NOERROR ;
}

HRESULT STDMETHODCALLTYPE SGLDSRenderMediaPlayer::EnumPins::Clone( __out IEnumPins ** ppEnum )
{
	if ( ppEnum == NULL )
	{
		return	E_POINTER ;
	}
	*ppEnum = new EnumPins( m_pFilter ) ;
	return	NOERROR ;
}




HMODULE	SGLDSRenderMediaPlayer::m_hLibOleAut32 = NULL ;

// メディアマッチング
//////////////////////////////////////////////////////////////////////////////
bool SGLDSRenderMediaPlayer::MediaTypeMatchesPartial
	( const AM_MEDIA_TYPE * pmt0, const AM_MEDIA_TYPE * pmt1 )
{
	if ( (pmt1->majortype != GUID_NULL)
		&& (pmt0->majortype != pmt1->majortype) )
	{
		return	false ;
	}
	if ( (pmt1->subtype != GUID_NULL)
		&& (pmt0->subtype != pmt1->subtype) )
	{
		return	false ;
	}
	if ( pmt1->formattype != GUID_NULL )
	{
		if ( pmt0->formattype != pmt1->formattype )
		{
			return	false ;
		}
		if ( pmt0->cbFormat != pmt1->cbFormat )
		{
			return	false ;
		}
		if ( (pmt0->cbFormat != 0)
			&& (memcmp( pmt0->pbFormat,
						pmt1->pbFormat, pmt0->cbFormat) != 0) )
		{
			return	false ;
		}
	}
	return	true ;
}

// 部分的なメディアタイプか判定
//////////////////////////////////////////////////////////////////////////////
bool SGLDSRenderMediaPlayer::MediaTypeIsPartiallySpecified( const AM_MEDIA_TYPE * pmt )
{
	return	(pmt->majortype == GUID_NULL)
				|| (pmt->formattype == GUID_NULL) ;
}

// AM_MEDIA_TYPE 操作
//////////////////////////////////////////////////////////////////////////////
void SGLDSRenderMediaPlayer::DeleteMediaType( __inout_opt AM_MEDIA_TYPE * pmt )
{
	if ( pmt != NULL )
	{
		FreeMediaType( *pmt ) ;
		CoTaskMemFree( pmt ) ;
	}
}

HRESULT SGLDSRenderMediaPlayer::CopyMediaType
	(__out AM_MEDIA_TYPE * pmtDst, const AM_MEDIA_TYPE * pmtSrc )
{
	*pmtDst = *pmtSrc;
	if ( pmtSrc->cbFormat != 0 )
	{
		ESLAssert( pmtSrc->pbFormat != NULL ) ;
		pmtDst->pbFormat = (PBYTE) CoTaskMemAlloc( pmtSrc->cbFormat ) ;
		if ( pmtDst->pbFormat == NULL )
		{
			pmtDst->cbFormat = 0;
			return	E_OUTOFMEMORY ;
		}
		else
		{
			memmove
				( pmtDst->pbFormat,
					pmtSrc->pbFormat,
					pmtDst->cbFormat ) ;
		}
	}
	if ( pmtDst->pUnk != NULL )
	{
		pmtDst->pUnk->AddRef() ;
	}
	return	S_OK ;
}

void SGLDSRenderMediaPlayer::FreeMediaType(__inout AM_MEDIA_TYPE& mt )
{
	if ( mt.cbFormat != 0 )
	{
		CoTaskMemFree( mt.pbFormat ) ;
		//
		mt.cbFormat = 0 ;
		mt.pbFormat = NULL ;
	}
	if ( mt.pUnk != NULL )
	{
		mt.pUnk->Release() ;
		mt.pUnk = NULL ;
	}
}

// 映像フォーマットを GUID から EntisGLS 画像フォーマットへ変換
//////////////////////////////////////////////////////////////////////////////
bool SGLDSRenderMediaPlayer::ConvertVideoSubTypeGUID
	( uint32_t& format, uint32_t& depth, const GUID& guid )
{
	if ( IsEqualGUID( guid, MEDIASUBTYPE_RGB24 ) )
	{
		format = formatImageRGB ;
		depth = 24 ;
	}
	else if ( IsEqualGUID( guid, MEDIASUBTYPE_RGB32 ) )
	{
		format = formatImageRGB ;
		depth = 32 ;
	}
	else if ( IsEqualGUID( guid, MEDIASUBTYPE_ARGB32 ) )
	{
		format = formatImageARGB ;
		depth = 32 ;
	}
	else if ( IsEqualGUID( guid, MEDIASUBTYPE_YUY2 ) )
	{
		format = formatImageYUV2 ;
		depth = 16 ;
	}
	else if ( IsEqualGUID( guid, MEDIASUBTYPE_YVYU ) )
	{
		format = formatImageYVYU ;
		depth = 16 ;
	}
	else if ( IsEqualGUID( guid, MEDIASUBTYPE_UYVY ) )
	{
		format = formatImageUYVY ;
		depth = 16 ;
	}
	else if ( IsEqualGUID( guid, MEDIASUBTYPE_YUYV ) )
	{
		format = formatImageYUYV ;
		depth = 16 ;
	}
	else
	{
		format = 0 ;
		depth = 0 ;
		return	false ;
	}
	return	true ;
}



//////////////////////////////////////////////////////////////////////////////
// DirectShow メディアファイル再生インターフェース（内部描画）
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLDSRenderMediaPlayer, SGLMediaPlayerInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLDSRenderMediaPlayer::SGLDSRenderMediaPlayer( void )
{
	m_pGraphBuilder = NULL ;
	m_pMediaControl = NULL ;
	m_pVideoWindow = NULL ;
	m_pBasicAudio = NULL ;
	m_pBasicVideo = NULL ;
	m_pMediaPosition = NULL ;
	m_pMediaEvent = NULL ;
	//
	m_pSrcFile = NULL ;
	m_pFilter = NULL ;
	m_eventAbort.Initialize( false ) ;
	m_pMutexUI = SSystem::g_mutexGlobal ;
	//
	m_flagPlayed = false ;
	m_flagPaused = false ;
	m_flagLoop = false ;
	//
	m_pWindow = NULL ;
	//
	m_pListener = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLDSRenderMediaPlayer::~SGLDSRenderMediaPlayer( void )
{
	SGLDSRenderMediaPlayer::Close() ;
}

// フレーム更新通知
//////////////////////////////////////////////////////////////////////////////
void SGLDSRenderMediaPlayer::OnUpdateVideoFrame( void )
{
	if ( SystemLock() )
	{
		if ( m_pListener != NULL )
		{
			m_pListener->OnFrameUpdate( this ) ;
		}
		else if ( m_pWindow != NULL )
		{
			S3DRenderContextInterface *
					render = m_pWindow->GetRenderContext() ;
			if ( render != NULL )
			{
				DrawVideo( render, m_rectDstView ) ;
				m_pWindow->ReleaseRenderContext( render ) ;
			}
		}
		SystemUnlock() ;
	}
}

// 区間終端通知
//////////////////////////////////////////////////////////////////////////////
void SGLDSRenderMediaPlayer::OnEndOfStream( void )
{
	if ( SystemLock() )
	{
		if ( m_pListener != NULL )
		{
			m_pListener->OnEndOfDuration( this ) ;
		}
		if ( m_flagLoop )
		{
			SystemUnlock() ;
			//
			if ( m_msLoopStart <= 0 )
			{
				SeekPosition( 0 ) ;
			}
			else
			{
				SeekPosition( m_msLoopStart ) ;
			}
			if ( m_pMediaControl != NULL )
			{
				m_pMediaControl->Run() ;
			}
		}
		else
		{
			m_flagPlayed = false ;
			SystemUnlock() ;
		}
	}
}

// 中断判定つき画面ミューテックス
//////////////////////////////////////////////////////////////////////////////
bool SGLDSRenderMediaPlayer::SystemLock( void )
{
	while ( m_pMutexUI->LockTrace( __FILE__, __LINE__, 10 ) != errSuccess )
	{
		if ( m_eventAbort.Wait( 0 ) == errSuccess )
		{
			return	false ;
		}
	}
	return	true ;
}

void SGLDSRenderMediaPlayer::SystemUnlock( void )
{
	m_pMutexUI->Unlock() ;
}

// ファイルパス取得
//////////////////////////////////////////////////////////////////////////////
bool SGLDSRenderMediaPlayer::NormalizeFilePath
	( const wchar_t * pwszFilePath,
		SSystem::SEnvironmentInterface * pEnv )
{
	if ( SFileOpener::DefaultDirectPathOf( m_strFilePath, pwszFilePath ) )
	{
		SSmartPointer<SFileInterface>	pfile ;
		if ( pEnv != NULL )
		{
			pfile = pEnv->NewOpenFile( pwszFilePath, SFileOpener::shareRead ) ;
		}
		else
		{
			pfile = SFileOpener::DefaultNewOpenFile
							( pwszFilePath, SFileOpener::shareRead ) ;
		}
		SFile *	pFile = ESLTypeCast<SFile>( pfile.Ptr() ) ;
		if ( pFile != NULL )
		{
			m_strFilePath = pFile->GetFilePath() ;
		}
		else
		{
			return	false ;
		}
	}
	return	true ;
}

// Pin 取得
//////////////////////////////////////////////////////////////////////////////
IPin * SGLDSRenderMediaPlayer::GetPinOf
			( IBaseFilter * pFilter, PIN_DIRECTION dir )
{
	IEnumPins *	pEnum = NULL ;
	if ( FAILED( pFilter->EnumPins( &pEnum ) ) )
	{
		return	NULL ;
	}
	for ( ; ; )
	{
		ULONG	nCount = 0 ;
		IPin *	pPin = NULL ;
		HRESULT	hr = pEnum->Next( 1, &pPin, &nCount ) ;
		if ( hr != S_OK )
		{
			break ;
		}
		PIN_DIRECTION	dirEnum ;
		pPin->QueryDirection( &dirEnum ) ;
		if ( dir == dirEnum )
		{
			pEnum->Release() ;
			return	pPin ;
		}
	}
	pEnum->Release() ;
	return	NULL ;
}

// SGLWindowsAVIReader 取得
//////////////////////////////////////////////////////////////////////////////
SGLWindowsAVIReader * SGLDSRenderMediaPlayer::GetAVIMediaStream( void )
{
	if ( (m_pAVIReader == NULL) && !m_strFilePath.IsEmpty() )
	{
		m_pAVIReader = new SGLWindowsAVIReader ;
		if ( m_pAVIReader->Open( m_strFilePath ) )
		{
			m_pAVIReader = NULL ;
		}
	}
	return	m_pAVIReader ;
}

// 指定ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDSRenderMediaPlayer::Open
	( const wchar_t * pwszFilePath, uint64_t nFlags,
				SSystem::SEnvironmentInterface * pEnv )
{
	Close() ;
	//
	// DirectShow オブジェクト生成
	//
	CoCreateInstance
		( CLSID_FilterGraph, NULL, CLSCTX_INPROC,
				IID_IGraphBuilder, (void **) &m_pGraphBuilder ) ;
	if ( m_pGraphBuilder == NULL )
	{
		return	sglErrFailed ;
	}
	//
	// ファイルを開く／グラフ構築
	//
	if ( !NormalizeFilePath( pwszFilePath, pEnv ) )
	{
		Close() ;
		return	sglErrFailed ;
	}
	const wchar_t *	pwszFilterName = L"Renderer" ;
	m_pFilter = new Filter( pwszFilterName, this ) ;
	m_pGraphBuilder->AddFilter( m_pFilter, pwszFilterName ) ;
	//
	HRESULT	hr = m_pGraphBuilder->RenderFile( m_strFilePath, NULL ) ;
	if ( hr )
	{
		Close() ;
		return	sglErrFailed ;
	}
	//
	// インターフェース取得
	//
	m_pGraphBuilder->QueryInterface
		( IID_IMediaControl, (void **) &m_pMediaControl ) ;
	if ( m_pMediaControl == NULL )
	{
		return	sglErrFailed ;
	}
	m_pGraphBuilder->QueryInterface
		( IID_IVideoWindow, (void **) &m_pVideoWindow ) ;
	m_pGraphBuilder->QueryInterface
		( IID_IBasicAudio, (void **) &m_pBasicAudio ) ;
	m_pGraphBuilder->QueryInterface
		( IID_IBasicVideo, (void **) &m_pBasicVideo ) ;
	m_pGraphBuilder->QueryInterface
		( IID_IMediaPosition, (void **) &m_pMediaPosition ) ;
	m_pGraphBuilder->QueryInterface
		( IID_IMediaEvent, (void **) &m_pMediaEvent ) ;
	if ( m_pVideoWindow != NULL )
	{
		m_pVideoWindow->put_AutoShow( OAFALSE ) ;
		m_pVideoWindow->put_Visible( OATRUE ) ;
	}
	return	sglErrSuccess ;
}

SGLError SGLDSRenderMediaPlayer::Create
	( SSystem::SFileInterface * file, bool flagOwner, uint64_t nFlags )
{
	Close() ;
	//
	// DirectShow オブジェクト生成
	//
	CoCreateInstance
		( CLSID_FilterGraph, NULL, CLSCTX_INPROC,
				IID_IGraphBuilder, (void **) &m_pGraphBuilder ) ;
	if ( m_pGraphBuilder == NULL )
	{
		return	sglErrFailed ;
	}
	//
	// グラフ構築
	//
	m_pSrcFile = new SGLDSFileSource( L"SourceFile", file, flagOwner ) ;
	m_pGraphBuilder->AddFilter( m_pSrcFile, L"SourceFile" ) ;
	//
	m_pFilter = new Filter( L"Renderer", this ) ;
	m_pGraphBuilder->AddFilter( m_pFilter, L"Renderer" ) ;
	//
	IPin *	pPin = m_pSrcFile->GetOutputPin() ;
	pPin->AddRef() ;
	HRESULT	hr = m_pGraphBuilder->Render( pPin ) ;
	if ( hr )
	{
		Close() ;
		return	sglErrFailed ;
	}
	//
	// インターフェース取得
	//
	m_pGraphBuilder->QueryInterface
		( IID_IMediaControl, (void **) &m_pMediaControl ) ;
	if ( m_pMediaControl == NULL )
	{
		return	sglErrFailed ;
	}
	m_pGraphBuilder->QueryInterface
		( IID_IVideoWindow, (void **) &m_pVideoWindow ) ;
	m_pGraphBuilder->QueryInterface
		( IID_IBasicAudio, (void **) &m_pBasicAudio ) ;
	m_pGraphBuilder->QueryInterface
		( IID_IBasicVideo, (void **) &m_pBasicVideo ) ;
	m_pGraphBuilder->QueryInterface
		( IID_IMediaPosition, (void **) &m_pMediaPosition ) ;
	m_pGraphBuilder->QueryInterface
		( IID_IMediaEvent, (void **) &m_pMediaEvent ) ;
	if ( m_pVideoWindow != NULL )
	{
		m_pVideoWindow->put_AutoShow( OAFALSE ) ;
		m_pVideoWindow->put_Visible( OAFALSE ) ;
	}
	SFile *	pFile = ESLTypeCast<SFile>( file ) ;
	if ( pFile != nullptr )
	{
		m_strFilePath = pFile->GetFilePath() ;
	}
	return	sglErrSuccess ;
}

// データを参照する複製プレイヤー生成
//////////////////////////////////////////////////////////////////////////////
SGLAudioPlayerInterface * SGLDSRenderMediaPlayer::ClonePlayer( void )
{
	SGLDSRenderMediaPlayer *	pPlayer = new SGLDSRenderMediaPlayer ;
	if ( !m_strFilePath.IsEmpty() )
	{
		pPlayer->Open( m_strFilePath ) ;
	}
	pPlayer->SetUIThreadMutex( m_pMutexUI ) ;
	return	pPlayer ;
}

// ファイルを閉じる
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDSRenderMediaPlayer::Close( void )
{
	if ( m_flagPlayed )
	{
		Stop() ;
	}
	if ( m_pMediaEvent != NULL )
	{
		m_pMediaEvent->Release() ;
		m_pMediaEvent = NULL ;
	}
	if ( m_pMediaPosition != NULL )
	{
		m_pMediaPosition->Release() ;
		m_pMediaPosition = NULL ;
	}
	if ( m_pBasicVideo != NULL )
	{
		m_pBasicVideo->Release() ;
		m_pBasicVideo = NULL ;
	}
	if ( m_pBasicAudio != NULL )
	{
		m_pBasicAudio->Release() ;
		m_pBasicAudio = NULL ;
	}
	if ( m_pVideoWindow != NULL )
	{
		m_pVideoWindow->Release() ;
		m_pVideoWindow = NULL ;
	}
	if ( m_pMediaControl != NULL )
	{
		m_pMediaControl->Release() ;
		m_pMediaControl = NULL ;
	}
	if ( m_pGraphBuilder != NULL )
	{
		m_pGraphBuilder->Release() ;
		m_pGraphBuilder = NULL ;
	}
	if ( m_pFilter != NULL )
	{
		m_pFilter->Release() ;
		m_pFilter = NULL ;
	}
	if ( m_pSrcFile != NULL )
	{
		m_pSrcFile->Release() ;
		m_pSrcFile = NULL ;
	}
	m_strFilePath.FreeArray() ;
	//
	return	sglErrSuccess ;
}

// 再生を開始する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDSRenderMediaPlayer::Play( uint64_t nFlags )
{
	if ( m_pMediaControl == NULL )
	{
		return	sglErrFailed ;
	}
	SGLError	err = sglErrSuccess ;
	if ( m_pMediaControl->Run() == S_OK )
	{
		OAFilterState	state = State_Stopped ;
		if ( (m_pMediaControl->GetState( 1000, &state ) != S_OK)
			|| (state != State_Running) )
		{
			err = sglErrFailed ;
		}
	}
	//
	m_pMutexUI->LockTrace( __FILE__, __LINE__ ) ;
	m_flagPlayed = true ;
	m_flagPaused = false ;
	m_pMutexUI->Unlock() ;
	//
	return	err ;
}

// 再生を停止する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDSRenderMediaPlayer::Stop( void )
{
	if ( m_pMediaControl == NULL )
	{
		return	sglErrFailed ;
	}
	m_eventAbort.SetSignal() ;
	m_pMediaControl->Stop() ;
	//
	m_pMutexUI->LockTrace( __FILE__, __LINE__ ) ;
	m_flagPlayed = false ;
	m_eventAbort.ResetSignal() ;
	m_pMutexUI->Unlock() ;
	//
	return	sglErrSuccess ;
}

// ループポイント[/sample] を設定する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDSRenderMediaPlayer::SetLoop
	( bool fLoop, int64_t nStart, int64_t nEnd )
{
	m_pMutexUI->LockTrace( __FILE__, __LINE__ ) ;
	m_flagLoop = fLoop ;
	m_msLoopStart = nStart ;
	m_msLoopEnd = nEnd ;
	//
	if ( m_pMediaPosition != NULL )
	{
		REFTIME	rtDuration ;
		if ( m_pMediaPosition->get_Duration( &rtDuration ) == S_OK )
		{
			REFTIME	rtLoopEnd = rtDuration ;
			if ( m_flagLoop || (m_msLoopEnd > 0) )
			{
				rtLoopEnd = (double) m_msLoopEnd / 1000.0 ;
				if ( rtLoopEnd >= rtDuration )
				{
					rtLoopEnd = rtDuration ;
				}
			}
			m_pMediaPosition->put_StopTime( rtLoopEnd ) ;
		}
	}
	m_pMutexUI->Unlock() ;
	return	sglErrSuccess ;
}

// 再生を一時停止する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDSRenderMediaPlayer::Pause( void )
{
	if ( m_pMediaControl == NULL )
	{
		return	sglErrFailed ;
	}
	m_pMediaControl->Pause() ;
	//
	m_pMutexUI->LockTrace( __FILE__, __LINE__ ) ;
	m_flagPaused = true ;
	m_pMutexUI->Unlock() ;
	//
	return	sglErrSuccess ;
}

// 再生を再開する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDSRenderMediaPlayer::Restart( void )
{
	if ( m_pMediaControl == NULL )
	{
		return	sglErrFailed ;
	}
	SGLError	err = sglErrSuccess ;
	if ( m_pMediaControl->Run() == S_OK )
	{
		OAFilterState	state = State_Stopped ;
		if ( (m_pMediaControl->GetState( 1000, &state ) != S_OK)
			|| (state != State_Running) )
		{
			err = sglErrFailed ;
		}
	}
	//
	m_pMutexUI->LockTrace( __FILE__, __LINE__ ) ;
	m_flagPaused = false ;
	m_pMutexUI->Unlock() ;
	//
	return	err ;
}

// 音量取得 [L/R]
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDSRenderMediaPlayer::GetVolume( float32_t* pVolumes, size_t nChannels )
{
	if ( m_pBasicAudio == NULL )
	{
		return	sglErrFailed ;
	}
	long		lVolume, lBalance ;
	float32_t	volume[2] ;
	m_pMutexUI->LockTrace( __FILE__, __LINE__ ) ;
	if ( m_pBasicAudio->get_Volume( &lVolume ) != S_OK )
	{
		lVolume = 0 ;
	}
	if ( m_pBasicAudio->get_Balance( &lBalance ) != S_OK )
	{
		lBalance = 0 ;
	}
	m_pMutexUI->Unlock() ;
	//
	SGLDirectSoundPlayer::VolumePanToStereo( &volume[0], lVolume, lBalance ) ;
	if ( nChannels >= 1 )
	{
		pVolumes[0] = volume[0] ;
		if ( nChannels >= 2 )
		{
			pVolumes[1] = volume[1] ;
		}
	}
	return	sglErrSuccess ;
}

// 音量設定 [L/R]
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDSRenderMediaPlayer::SetVolume( const float32_t* pVolumes, size_t nChannels )
{
	if ( m_pBasicAudio == NULL )
	{
		return	sglErrFailed ;
	}
	long		lVolume, lBalance ;
	float32_t	volume[2] = { 1.0, 1.0 } ;
	if ( nChannels >= 2 )
	{
		volume[0] = pVolumes[0] ;
		volume[1] = pVolumes[1] ;
	}
	else if ( nChannels >= 1 )
	{
		volume[0] = pVolumes[0] ;
		volume[1] = pVolumes[0] ;
	}
	SGLDirectSoundPlayer::VolumeStereoToPan( lVolume, lBalance, &volume[0] ) ;
	//
	m_pMutexUI->LockTrace( __FILE__, __LINE__ ) ;
	m_pBasicAudio->put_Volume( lVolume ) ;
	m_pBasicAudio->put_Balance( lBalance ) ;
	m_pMutexUI->Unlock() ;
	//
	return	sglErrSuccess ;
}

// 再生中か？
//////////////////////////////////////////////////////////////////////////////
bool SGLDSRenderMediaPlayer::IsPlaying( void ) const
{
	if ( m_flagPlayed && (m_pFilter != NULL) )
	{
		InputPin *	pPin = m_pFilter->GetPin() ;
		if ( (pPin != NULL) && !pPin->IsConnected() )
		{
			bool	flagStopped = false ;
			m_pMutexUI->LockTrace( __FILE__, __LINE__ ) ;
			if ( m_pMediaEvent != NULL )
			{
				long		lEventCode ;
				LONG_PTR	lParam1, lParam2 ;
				if ( m_pMediaEvent->GetEvent
						( &lEventCode, &lParam1, &lParam2, 0 ) == S_OK )
				{
					if ( lEventCode == EC_COMPLETE )
					{
						flagStopped = true ;
					}
					m_pMediaEvent->FreeEventParams
							( lEventCode, lParam1, lParam2 ) ;
				}
			}
			m_pMutexUI->Unlock() ;
			//
			if ( flagStopped )
			{
				((SGLDSRenderMediaPlayer*)this)->OnEndOfStream() ;
			}
		}
	}
	return	m_flagPlayed ;
}

// 一時停止中か？
//////////////////////////////////////////////////////////////////////////////
bool SGLDSRenderMediaPlayer::IsPaused( void ) const
{
	return	m_flagPaused ;
}

// メディアのサンプル周波数を取得する
//////////////////////////////////////////////////////////////////////////////
uint32_t SGLDSRenderMediaPlayer::GetSampleFrequency( void ) const
{
	return	1000 ;
}

// メディアの全長 [/sample] を取得する
//////////////////////////////////////////////////////////////////////////////
uint64_t SGLDSRenderMediaPlayer::GetTotalLength( void ) const
{
	if ( m_pMediaPosition != NULL )
	{
		REFTIME	rtTime ;
		if ( m_pMediaPosition->get_Duration( &rtTime ) == S_OK )
		{
			return	(uint64_t) (rtTime * 1000.0) ;
		}
	}
	return	0 ;
}

// 再生位置 [/sample] を取得する
//////////////////////////////////////////////////////////////////////////////
uint64_t SGLDSRenderMediaPlayer::GetPosition( void )
{
	if ( m_pMediaPosition != NULL )
	{
		REFTIME	rtTime ;
		if ( m_pMediaPosition->get_CurrentPosition( &rtTime ) == S_OK )
		{
			return	(uint64_t) (rtTime * 1000.0) ;
		}
	}
	return	0 ;
}

// 再生位置 [/sample] を変更する
//////////////////////////////////////////////////////////////////////////////
void SGLDSRenderMediaPlayer::SeekPosition( uint64_t nPos )
{
	if ( m_pMediaPosition != NULL )
	{
		m_pMutexUI->LockTrace( __FILE__, __LINE__ ) ;
		REFTIME	rtTime = (double) nPos / 1000.0 ;
		m_pMediaPosition->put_CurrentPosition( rtTime ) ;
		m_pMutexUI->Unlock() ;
	}
}

// オーディオストリーム取得
//////////////////////////////////////////////////////////////////////////////
SGLAudioInputStream * SGLDSRenderMediaPlayer::GetAudioStream( void )
{
	return	GetAVIMediaStream() ;
}

void SGLDSRenderMediaPlayer::ReleaseAudioStream( SGLAudioInputStream * pStream )
{
}

// スレッド同期用ミューテックス設定
//////////////////////////////////////////////////////////////////////////////
void SGLDSRenderMediaPlayer::SetUIThreadMutex( SSystem::SMutex * pMutex )
{
	m_pMutexUI = pMutex ;
}

// ビデオサイズを取得する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDSRenderMediaPlayer::GetVideoSize( SGLSize& sizeVideo )
{
	sizeVideo.w = 0 ;
	sizeVideo.h = 0 ;
	//
	if ( m_pFilter == NULL )
	{
		return	sglErrFailed ;
	}
	SGLImageObject *	pImage = m_pFilter->LockVideoFrame() ;
	if ( pImage != NULL )
	{
		sizeVideo = pImage->GetImageSize() ;
		m_pFilter->UnlockVideoFrame() ;
		return	sglErrSuccess ;
	}
	return	sglErrFailed ;
}

// 表示先を設定する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDSRenderMediaPlayer::SetVideoView
	( SGLAbstractWindow* pWindow,
		const SGLImageRect& rectVideo, uint64_t nFlags )
{
	m_pMutexUI->LockTrace( __FILE__, __LINE__ ) ;
	m_pWindow = pWindow ;
	m_rectDstView = rectVideo ;
	m_pMutexUI->Unlock() ;
	return	sglErrSuccess ;
}

// 現在のフレームを描画する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDSRenderMediaPlayer::DrawVideo
	( SGLPaintContextInterface* pPaint,
		const SGLImageRect& rectDst,
		uint32_t nFlags, uint32_t nTransparency )
{
	SGLError			err = sglErrFailed ;
	if ( m_pFilter != NULL )
	{
		SGLImageObject *	pImage = m_pFilter->LockVideoFrame() ;
		if ( pImage != NULL )
		{
			SGLImageInfo	imginf ;
			pImage->GetImageInfo( imginf ) ;
			//
			SGLPaintParam	ppPaint ;
			SGLAffine		affine ;
			ppPaint.nFlags = nFlags ;
			ppPaint.nTransparency = nTransparency ;
			ppPaint.SetAffine
				( affine, rectDst.x, rectDst.y, 0, 0,
					(double) rectDst.w / imginf.width,
					(double) rectDst.h / imginf.height ) ;
			//
			err = pPaint->DrawImage( ppPaint, pImage ) ;
			//
			m_pFilter->UnlockVideoFrame() ;
		}
	}
	return	err ;
}

// メディア再生通知リスナ設定
//////////////////////////////////////////////////////////////////////////////
SGLMediaPlayerFrameNotification *
	SGLDSRenderMediaPlayer::SetNotificationListener
		( SGLMediaPlayerFrameNotification * pListener )
{
	SGLMediaPlayerFrameNotification *	pLast ;
	m_pMutexUI->LockTrace( __FILE__, __LINE__ ) ;
	pLast = m_pListener ;
	m_pListener = pListener ;
	m_pMutexUI->Unlock() ;
	return	pLast ;
}

// ビデオストリーム取得
//////////////////////////////////////////////////////////////////////////////
SGLVideoInputStream * SGLDSRenderMediaPlayer::GetVideoStream( void )
{
	return	GetAVIMediaStream() ;
}

void SGLDSRenderMediaPlayer::ReleaseVideoStream( SGLVideoInputStream * pStream )
{
}



//////////////////////////////////////////////////////////////////////////////
// DirectShow カメラキャプチャー再生インターフェース（内部描画）
//////////////////////////////////////////////////////////////////////////////

// SGLDSVideoCapturePlayer::DeviceInfo
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLDSVideoCapturePlayer::DSDeviceInfo, DeviceInfo )

SGLDSVideoCapturePlayer::DSDeviceInfo::DSDeviceInfo( void )
{
	m_pSrcFilter = NULL ;
	m_pCapture = NULL ;
	m_pStreamConfig = NULL ;
}

SGLDSVideoCapturePlayer::DSDeviceInfo::~DSDeviceInfo( void )
{
	if ( m_pStreamConfig != NULL )
	{
		m_pStreamConfig->Release() ;
		m_pStreamConfig = NULL ;
	}
	if ( m_pSrcFilter != NULL )
	{
		m_pSrcFilter->Release() ;
		m_pSrcFilter = NULL ;
	}
	if ( m_pCapture != NULL )
	{
		m_pCapture->Release() ;
		m_pCapture = NULL ;
	}
}

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::SGLDSVideoCapturePlayer,
			SGLDSRenderMediaPlayer, SGLVideoCaptureInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLDSVideoCapturePlayer::SGLDSVideoCapturePlayer( void )
{
	m_pCaptureSrc = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLDSVideoCapturePlayer::~SGLDSVideoCapturePlayer( void )
{
	Close() ;
}

// デバイス検索
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDSVideoCapturePlayer::GetDeviceInfo
	( SGLDSVideoCapturePlayer::DSDeviceInfo& devInfo, const wchar_t * pwszDevID )
{
	ICreateDevEnum *	pDevEnum = NULL ;
	CoCreateInstance
		( CLSID_SystemDeviceEnum, 
			NULL, CLSCTX_INPROC,
			IID_ICreateDevEnum, (void**)&pDevEnum ) ;
	if ( pDevEnum == NULL )
	{
		return	sglErrFailed ;
	}
	IEnumMoniker *	pEnum = NULL ;
	pDevEnum->CreateClassEnumerator
		( CLSID_VideoInputDeviceCategory, &pEnum, 0 ) ;
	if( pEnum == NULL )
	{
		pDevEnum->Release() ;
		return	sglErrFailed ;
	}
	ULONG			cFetched ;
	IMoniker *		pMoniker = NULL ;
	while( pEnum->Next( 1, &pMoniker, &cFetched ) == S_OK )
	{
		IPropertyBag *	pProp = NULL ;
		VARIANT			var ;
		pMoniker->BindToStorage
				( 0, 0, IID_IPropertyBag, (void**)&pProp) ;
		//
		var.vt = VT_BSTR ;
		pProp->Read( L"DevicePath", &var, 0 ) ;
		//
		SString	strDevID = var.bstrVal ;
		VariantClear( &var ) ;
		//
		if ( (pwszDevID == NULL)
			|| (strDevID == pwszDevID) )
		{
			devInfo.m_strDevID = strDevID ;
			//
			var.vt = VT_BSTR ;
			pProp->Read( L"FriendlyName", &var, 0 ) ;
			devInfo.m_strDevName = var.bstrVal ;
			VariantClear( &var ) ;
			//
			IBaseFilter *	pFilter = NULL ;
			pMoniker->BindToObject
				( 0, 0, IID_IBaseFilter, (void**)&pFilter ) ;
			devInfo.m_pSrcFilter = pFilter ;
			//
			EnumerateDeviceResolutions( devInfo ) ;
			return	sglErrSuccess ;
		}
	}
	return	sglErrFailed ;
}

// デバイス列挙
//////////////////////////////////////////////////////////////////////////////
void SGLDSVideoCapturePlayer::EnumerateDevices
	( SSystem::SObjectArray<SGLDSVideoCapturePlayer::DeviceInfo>& aDevInfo )
{
	aDevInfo.RemoveAll() ;
	//
	ICreateDevEnum *	pDevEnum = NULL ;
	CoCreateInstance
		( CLSID_SystemDeviceEnum, 
			NULL, CLSCTX_INPROC,
			IID_ICreateDevEnum, (void**)&pDevEnum ) ;
	if ( pDevEnum == NULL )
	{
		return ;
	}
	IEnumMoniker *	pEnum = NULL ;
	pDevEnum->CreateClassEnumerator
		( CLSID_VideoInputDeviceCategory, &pEnum, 0 ) ;
	if( pEnum == NULL )
	{
		pDevEnum->Release() ;
		return ;
	}
	ULONG			cFetched ;
	IMoniker *		pMoniker = NULL ;
	while( pEnum->Next( 1, &pMoniker, &cFetched ) == S_OK )
	{
		DSDeviceInfo *	pdi = new DSDeviceInfo ;
		aDevInfo.Add( pdi ) ;
		//
		IPropertyBag *	pProp = NULL ;
		VARIANT			var ;
		pMoniker->BindToStorage
				( 0, 0, IID_IPropertyBag, (void**)&pProp) ;
		//
		var.vt = VT_BSTR ;
		pProp->Read( L"DevicePath", &var, 0 ) ;
		pdi->m_strDevID = var.bstrVal ;
		VariantClear( &var ) ;
		//
		var.vt = VT_BSTR ;
		pProp->Read( L"FriendlyName", &var, 0 ) ;
		pdi->m_strDevName = var.bstrVal ;
		VariantClear( &var ) ;
		//
		IBaseFilter *	pFilter = NULL ;
		pMoniker->BindToObject
			( 0, 0, IID_IBaseFilter, (void**)&pFilter ) ;
		pdi->m_pSrcFilter = pFilter ;
		//
		EnumerateDeviceResolutions( *pdi ) ;
	}
	pEnum->Release() ;
	pDevEnum->Release() ;
}

void SGLDSVideoCapturePlayer::EnumerateDeviceResolutions
		( SGLDSVideoCapturePlayer::DSDeviceInfo& devInfo )
{
	ICaptureGraphBuilder2 *	pCapture = NULL ;
	CoCreateInstance
		( CLSID_CaptureGraphBuilder2, 
			NULL, CLSCTX_INPROC,
			IID_ICaptureGraphBuilder2, (void **) &pCapture ) ;
	if ( pCapture == NULL )
	{
		return ;
	}
	devInfo.m_pCapture = pCapture ;
	//
	IAMStreamConfig *	pConfig = NULL ;
	HRESULT	hr = pCapture->FindInterface
					( &PIN_CATEGORY_CAPTURE, 
						0, devInfo.m_pSrcFilter,
						IID_IAMStreamConfig, (void**) &pConfig ) ;
	if ( pConfig == NULL )
	{
		return ;
	}
	devInfo.m_pStreamConfig = pConfig ;
	//
	int nCount = 0 ;
	int	nSize = 0 ;
	hr = pConfig->GetNumberOfCapabilities( &nCount, &nSize ) ;
	if ( nSize != sizeof(VIDEO_STREAM_CONFIG_CAPS) )
	{
		return ;
	}
	devInfo.m_Resolutions.RemoveAll() ;
	for ( int i = 0; i < nCount; i ++ )
	{
		VIDEO_STREAM_CONFIG_CAPS	vscc ;
		AM_MEDIA_TYPE *				pmt = NULL ;
		hr = pConfig->GetStreamCaps( i, &pmt, (BYTE*) &vscc ) ;
		if ( FAILED(hr) )
		{
			continue ;
		}
		if ( (pmt->majortype == MEDIATYPE_Video)
			&& (pmt->formattype == FORMAT_VideoInfo)
			&& (pmt->cbFormat >= sizeof(VIDEOINFOHEADER))
			&& (pmt->pbFormat != NULL) )
		{
			VIDEOINFOHEADER *	pvih =
					(VIDEOINFOHEADER*) pmt->pbFormat ;
			Resolution	res ;
			res.iFormat = i ;
			res.sizeFrame.w = (int32_t) pvih->bmiHeader.biWidth ;
			res.sizeFrame.h = (int32_t) pvih->bmiHeader.biHeight ;
			res.framesPerSec = 10000000.0 / pvih->AvgTimePerFrame ;
			res.nVideoFormat = 0 ;
			res.nSupported = 0 ;
			//
			uint32_t	depth ;
			if ( ConvertVideoSubTypeGUID
					( res.nVideoFormat, depth, pmt->subtype ) )
			{
				res.nSupported |= supportedVideoDecoder ;
			}
			devInfo.m_Resolutions.Add( res ) ;
		}
		DeleteMediaType( pmt ) ;
	}
}

// デバイス検索
//////////////////////////////////////////////////////////////////////////////
SGLVideoCaptureInterface::DeviceInfo *
	SGLDSVideoCapturePlayer::GetCaptureDeviceInfo( const wchar_t * pwszDevID )
{
	DSDeviceInfo *	pDSDevInfo = new DSDeviceInfo ;
	SGLError	err = GetDeviceInfo( *pDSDevInfo, pwszDevID ) ;
	if ( err )
	{
		delete	pDSDevInfo ;
		return	NULL ;
	}
	return	pDSDevInfo ;
}

// デバイス列挙
//////////////////////////////////////////////////////////////////////////////
void SGLDSVideoCapturePlayer::EnumerateCaptureDevices
	( SSystem::SObjectArray<SGLVideoCaptureInterface::DeviceInfo>& aDevInfo )
{
	EnumerateDevices( aDevInfo ) ;
}

// キャプチャーデバイスを開く
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDSVideoCapturePlayer::OpenCapture
	( const wchar_t * pwszDevName, int iFormat )
{
	DSDeviceInfo	devInfo ;
	SGLError		err = GetDeviceInfo( devInfo, pwszDevName ) ;
	if ( err )
	{
		return	err ;
	}
	return	OpenCapture( &devInfo, iFormat ) ;
}

SGLError SGLDSVideoCapturePlayer::OpenCapture
	( const SGLVideoCaptureInterface::DeviceInfo* pDevInfo, int iFormat )
{
	Close() ;
	//
	// デバイス・フォーマット選択
	//
	const DSDeviceInfo *
			pDSDevInfo = ESLTypeCast<DSDeviceInfo>( pDevInfo ) ;
	if ( pDSDevInfo == NULL )
	{
		return	sglErrFailed ;
	}
	if ( (pDSDevInfo->m_pSrcFilter == NULL)
		|| (pDSDevInfo->m_pCapture == NULL)
		|| (pDSDevInfo->m_pStreamConfig == NULL) )
	{
		return	sglErrFailed ;
	}
	Resolution *	pRes = NULL ;
	for ( size_t i = 0; i < pDSDevInfo->m_Resolutions.GetLength(); i ++ )
	{
		Resolution *	pr = pDSDevInfo->m_Resolutions.GetAt( i ) ;
		if ( (pr != NULL) && (pr->iFormat == iFormat) )
		{
			pRes = pr ;
			break ;
		}
	}
	if ( pRes == NULL )
	{
		return	sglErrFailed ;
	}
	IAMStreamConfig *			pConfig = pDSDevInfo->m_pStreamConfig ;
	VIDEO_STREAM_CONFIG_CAPS	vscc ;
	AM_MEDIA_TYPE *				pmt = NULL ;
	if ( SUCCEEDED( pConfig->GetStreamCaps
						( pRes->iFormat, &pmt, (BYTE*) &vscc ) ) )
	{
		pConfig->SetFormat( pmt ) ;
		DeleteMediaType( pmt ) ;
	}
	//
	// DirectShow オブジェクト生成
	//
	CoCreateInstance
		( CLSID_FilterGraph, NULL, CLSCTX_INPROC,
				IID_IGraphBuilder, (void **) &m_pGraphBuilder ) ;
	if ( m_pGraphBuilder == NULL )
	{
		return	sglErrFailed ;
	}
	//
	// グラフ構築
	//
	m_pCaptureSrc = pDSDevInfo->m_pSrcFilter ;
	m_pCaptureSrc->AddRef() ;
	m_pGraphBuilder->AddFilter( m_pCaptureSrc, L"VideoCapture" ) ;
	//
	m_pFilter = new Filter( L"Renderer", this ) ;
	m_pGraphBuilder->AddFilter( m_pFilter, L"Renderer" ) ;
	//
	IPin *	pPin = GetPinOf( m_pCaptureSrc, PINDIR_OUTPUT ) ;
	pPin->AddRef() ;
	HRESULT	hr = m_pGraphBuilder->Render( pPin ) ;
	if ( hr )
	{
		Close() ;
		return	sglErrFailed ;
	}
	//
	// インターフェース取得
	//
	m_pGraphBuilder->QueryInterface
		( IID_IMediaControl, (void **) &m_pMediaControl ) ;
	if ( m_pMediaControl == NULL )
	{
		return	sglErrFailed ;
	}
	m_pGraphBuilder->QueryInterface
		( IID_IVideoWindow, (void **) &m_pVideoWindow ) ;
	m_pGraphBuilder->QueryInterface
		( IID_IBasicAudio, (void **) &m_pBasicAudio ) ;
	m_pGraphBuilder->QueryInterface
		( IID_IBasicVideo, (void **) &m_pBasicVideo ) ;
	m_pGraphBuilder->QueryInterface
		( IID_IMediaPosition, (void **) &m_pMediaPosition ) ;
	m_pGraphBuilder->QueryInterface
		( IID_IMediaEvent, (void **) &m_pMediaEvent ) ;
	if ( m_pVideoWindow != NULL )
	{
		m_pVideoWindow->put_AutoShow( OAFALSE ) ;
		m_pVideoWindow->put_Visible( OAFALSE ) ;
	}
	return	sglErrSuccess ;
}

// ファイルを閉じる
//////////////////////////////////////////////////////////////////////////////
SGLError SGLDSVideoCapturePlayer::Close( void )
{
	SGLDSRenderMediaPlayer::Close() ;
	//
	if ( m_pCaptureSrc != NULL )
	{
		m_pCaptureSrc->Release() ;
		m_pCaptureSrc = NULL ;
	}
	return	sglErrSuccess ;
}

