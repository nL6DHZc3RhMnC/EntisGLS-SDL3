
#include <antirrhinum/antirrhinum.h>
#include <sakuraglx/sprite/sglx_sprite_message.h>
#include <sakuragl/media/sgl_sound_software_mixer.h>

using namespace	SSystem ;
using namespace	Rosetta ;
using namespace	SakuraGL ;
using namespace	AntirrhinumGL ;



//////////////////////////////////////////////////////////////////////////////
// AGLMessageProcessor::UIMessage
//////////////////////////////////////////////////////////////////////////////

ESL_IMPLEMENT_CLASS_INFO( AntirrhinumGL::AGLMessageProcessor::UIMessage, SObject )



//////////////////////////////////////////////////////////////////////////////
// AGLMessageProcessor::UISelector
//////////////////////////////////////////////////////////////////////////////

ESL_IMPLEMENT_CLASS_INFO( AntirrhinumGL::AGLMessageProcessor::UISelector, SObject )



//////////////////////////////////////////////////////////////////////////////
// AGLMessageProcessor::VoicePlayer
//////////////////////////////////////////////////////////////////////////////

ESL_IMPLEMENT_CLASS_INFO
	( AntirrhinumGL::AGLMessageProcessor::VoicePlayer, SObject )

AGLMessageProcessor::VoicePlayer::VoicePlayer
	( SakuraGL::SGLAudioPlayer * pPlayer,
		const wchar_t * pwszFileName,
		const CharConfig * pcfg,
		SoundMarkList * pMarkerList, uint32_t nFlags )
	: m_flagStarted( false ),
		m_pPlayer( pPlayer ), m_strFileName( pwszFileName ),
		m_pCharCfg( pcfg ), m_pMarkers( pMarkerList ), m_nFlags( nFlags )
{
}

void AGLMessageProcessor::VoicePlayer::SetAudioPlayer( SakuraGL::SGLAudioPlayer * pPlayer )
{
	m_pPlayer = pPlayer ;
}

SakuraGL::SGLAudioPlayer * AGLMessageProcessor::VoicePlayer::GetAudioPlayer( void ) const
{
	return	m_pPlayer ;
}

const SSystem::SString& AGLMessageProcessor::VoicePlayer::GetFileName( void ) const
{
	return	m_strFileName ;
}

const AGLMessageProcessor::CharConfig * AGLMessageProcessor::VoicePlayer::GetCharConfig( void ) const
{
	return	m_pCharCfg ;
}

uint32_t AGLMessageProcessor::VoicePlayer::GetFlags( void ) const
{
	return	m_nFlags ;
}

const AGLMessageProcessor::SoundMarkList *
	AGLMessageProcessor::VoicePlayer::GetSoundMarkerList( void ) const
{
	return	m_pMarkers ;
}

const AGLMessageProcessor::SoundMarkList::Marker *
	AGLMessageProcessor::VoicePlayer::GetSoundMarkerAs( const wchar_t * pwszID ) const
{
	if ( m_pMarkers != nullptr )
	{
		return	m_pMarkers->GetMarkerAs( pwszID ) ;
	}
	return	nullptr ;
}



//////////////////////////////////////////////////////////////////////////////
// AGLMessageProcessor::Voice
//////////////////////////////////////////////////////////////////////////////

bool AGLMessageProcessor::Voice::IsPlaying( void ) const
{
	SGLAudioPlayer *	pPlayer = GetAudioPlayer() ;
	return	(pPlayer != nullptr) && pPlayer->IsPlaying() ;
}

uint32_t AGLMessageProcessor::Voice::GetSampleFrequency( void ) const
{
	SGLAudioPlayer *	pPlayer = GetAudioPlayer() ;
	return	(pPlayer != nullptr) ? pPlayer->GetSampleFrequency() : 1 ;
}

uint64_t AGLMessageProcessor::Voice::GetPlayingPosition( void ) const
{
	SGLAudioPlayer *	pPlayer = GetAudioPlayer() ;
	return	(pPlayer != nullptr) ? pPlayer->GetPosition() : 0 ;
}

SakuraGL::SGLAudioPlayer * AGLMessageProcessor::Voice::GetAudioPlayer( void ) const
{
	VoicePlayer *	pPlayer = GetReference() ;
	if ( pPlayer != nullptr )
	{
		return	pPlayer->GetAudioPlayer() ;
	}
	return	nullptr ;
}

const AGLMessageProcessor::CharConfig *
		AGLMessageProcessor::Voice::GetCharConfig( void ) const
{
	VoicePlayer *	pPlayer = GetReference() ;
	if ( pPlayer != nullptr )
	{
		return	pPlayer->GetCharConfig() ;
	}
	return	nullptr ;
}

const AGLMessageProcessor::SoundMarkList *
	AGLMessageProcessor::Voice::GetSoundMarkerList( void ) const
{
	VoicePlayer *	pPlayer = GetReference() ;
	if ( pPlayer != nullptr )
	{
		return	pPlayer->GetSoundMarkerList() ;
	}
	return	nullptr ;
}

const AGLMessageProcessor::SoundMarkList::Marker *
	AGLMessageProcessor::Voice::GetSoundMarkerAs( const wchar_t * pwszID ) const
{
	const SoundMarkList *	pMarkers = GetSoundMarkerList() ;
	if ( pMarkers != nullptr )
	{
		return	pMarkers->GetMarkerAs( pwszID ) ;
	}
	return	nullptr ;
}



//////////////////////////////////////////////////////////////////////////////
// AGLMessageProcessor::VoiceListener
//////////////////////////////////////////////////////////////////////////////

ESL_IMPLEMENT_CLASS_INFO( AntirrhinumGL::AGLMessageProcessor::VoiceListener, ESLObject )



//////////////////////////////////////////////////////////////////////////////
// シンプルな口パク用解析クラス
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
AGLMessageProcessor::SimpleVoiceAnalyzer::SimpleVoiceAnalyzer( void )
	: m_nUnitSamples( 0 )
{
}

// 複製
//////////////////////////////////////////////////////////////////////////////
const AGLMessageProcessor::SimpleVoiceAnalyzer&
	AGLMessageProcessor::SimpleVoiceAnalyzer::operator = ( const SimpleVoiceAnalyzer& sva )
{
	m_aVoiceVelocity = sva.m_aVoiceVelocity ;
	m_fmtVoice = sva.m_fmtVoice ;
	m_nUnitSamples = sva.m_nUnitSamples ;
	return	*this ;
}

// 解析
//////////////////////////////////////////////////////////////////////////////
bool AGLMessageProcessor::SimpleVoiceAnalyzer::AnalyzeVoice
			( SakuraGL::SGLAudioPlayer * pPlayer, double secUnit )
{
	SGLAudioBufferReader *
			pBufReader = ESLTypeCast<SGLAudioBufferReader>( pPlayer ) ;
	if ( pBufReader == nullptr )
	{
		return	false ;
	}
	//
	// 16bit, 1channel PCM に変換して取得
	//
	size_t	nDataBytes = pBufReader->GetStaticBufferSize() ;
	if ( nDataBytes == 0 )
	{
		return	false ;
	}
	pBufReader->GetAudioFormat( m_fmtVoice ) ;
	//
	SArray<int16_t>	bufPCM ;
	size_t			nSampleCount =
						nDataBytes
							/ (m_fmtVoice.bitsPerSample
									* m_fmtVoice.channels / 8) ;
	SGLSoundFormat	fmtVoice = m_fmtVoice ;
	if ( (m_fmtVoice.bitsPerSample != 16) || (m_fmtVoice.channels != 1) )
	{
		SArray<uint8_t>	bufTemp ;
		pBufReader->ReadStaticBuffer
			( bufTemp.GetArray( nDataBytes ), 0, nDataBytes ) ;
		//
		SGLSoundSoftwareMixer::MixBuffer	mixbuf ;
		fmtVoice.bitsPerSample = 16 ;
		fmtVoice.channels = 1 ;
		mixbuf.MixWave
			( fmtVoice, bufPCM.GetArray( nSampleCount ), nSampleCount,
				m_fmtVoice, bufTemp.GetArray(), nSampleCount ) ;
	}
	else
	{
		pBufReader->ReadStaticBuffer
			( bufPCM.GetArray( nSampleCount ),
					0, nSampleCount * sizeof(int16_t) ) ;
	}
	//
	// 音圧変位
	//
	const size_t		nUnitSample = (size_t) (fmtVoice.frequency * secUnit) + 1 ;
	const size_t		nUnitCount = (nSampleCount + nUnitSample - 1) / nUnitSample ;
	const int16_t *		pwPCM = bufPCM.GetArray() ;
	float32_t *			pVelocity = m_aVoiceVelocity.GetArray( nUnitCount ) ;
	//
	int16_t		nLastVol = 0 ;
	float32_t	vMax = 0.0 ;
	for ( size_t i = 0; i < nUnitCount; i ++ )
	{
		size_t		iBase = i * nUnitSample ;
		float32_t	v = 0.0 ;
		int			nCount = esl_min( (int) (nSampleCount - iBase), (int) nUnitSample ) ;
		for ( int j = 0; j < nCount; j ++ )
		{
			int16_t	s = pwPCM[iBase + j] ;
			v += esl_abs( s - nLastVol ) ;
			nLastVol = s ;
		}
		v /= nCount ;
		if ( v > 0.0 )
		{
			v = (float32_t) pow( v, 0.25f ) ;
		}
		pVelocity[i] = v ;
		vMax = esl_fmaxf( v, vMax ) ;
	}
	//
	// 正規化
	//
	float32_t	d = 1.0f ;
	if ( vMax > 0.0f )
	{
		d = 1.0f / vMax ;
	}
	for ( size_t i = 0; i < nUnitCount; i ++ )
	{
		pVelocity[i] *= d ;
	}
	m_aVoiceVelocity.FinishArray() ;
	m_nUnitSamples = nUnitSample ;
	return	true ;
}

// バッファ解放
//////////////////////////////////////////////////////////////////////////////
void AGLMessageProcessor::SimpleVoiceAnalyzer::Release( void )
{
	m_aVoiceVelocity.RemoveAll() ;
	m_nUnitSamples = 0 ;
}

// 口パク用係数取得 [0,1]
//////////////////////////////////////////////////////////////////////////////
float32_t AGLMessageProcessor::SimpleVoiceAnalyzer::GetVoiceVelocityInSamples( uint64_t iSample ) const
{
	if ( m_nUnitSamples == 0 )
	{
		return	0.0f ;
	}
	return	GetVoiceVelocityInUnits( (size_t) (iSample / m_nUnitSamples) ) ;
}

float32_t AGLMessageProcessor::SimpleVoiceAnalyzer::GetVoiceVelocityInUnits( size_t iUnit ) const
{
	if ( iUnit >= m_aVoiceVelocity.GetLength() )
	{
		return	0.0f ;
	}
	return	m_aVoiceVelocity.At( iUnit ) ;
}

// 口パク係数要素数
//////////////////////////////////////////////////////////////////////////////
size_t AGLMessageProcessor::SimpleVoiceAnalyzer::GetVoiceVelocityLength( void ) const
{
	return	m_aVoiceVelocity.GetLength() ;
}

size_t AGLMessageProcessor::SimpleVoiceAnalyzer::GetVoiceVelocityUnit( void ) const
{
	return	m_nUnitSamples ;
}

// 解析したフォーマット
//////////////////////////////////////////////////////////////////////////////
const SakuraGL::SGLSoundFormat&
	AGLMessageProcessor::SimpleVoiceAnalyzer::GetSoundFormat( void ) const
{
	return	m_fmtVoice ;
}



//////////////////////////////////////////////////////////////////////////////
// 音声位置マーカーファイル
//////////////////////////////////////////////////////////////////////////////

// ファイル読み込み
//////////////////////////////////////////////////////////////////////////////
SSystem::SError
	AGLMessageProcessor::SoundMarkList::LoadMarkerFile
					( const wchar_t * pwszFilePath )
{
	SXMLDocument	xmlDoc ;
	SError	err = xmlDoc.LoadDocument( pwszFilePath, xmlDoc ) ;
	if ( err )
	{
		return	err ;
	}
	SXMLDocument *	pxmlMarkers = xmlDoc.GetElementTagAs( L"marks" ) ;
	if ( pxmlMarkers == nullptr )
	{
		return	errFailed ;
	}
	return	ParseMarkerFile( *pxmlMarkers ) ;
}

SSystem::SError
	AGLMessageProcessor::SoundMarkList::ParseMarkerFile
					( const SSystem::SXMLDocument& xmlMarkers )
{
	m_ssaMarkers.RemoveAll() ;
	//
	for ( size_t i = 0; i < xmlMarkers.GetElementsCount(); i ++ )
	{
		const SXMLDocument *	pxmlTag = xmlMarkers.GetElementAt( i ) ;
		if ( (pxmlTag == nullptr)
			|| (pxmlTag->GetTag() != L"mark") )
		{
			continue ;
		}
		const SString *	pstrID = pxmlTag->GetAttributeAs( L"name" ) ;
		if ( pstrID == nullptr )
		{
			continue ;
		}
		Marker	marker ;
		marker.nPos = pxmlTag->GetAttrIntegerAs( L"pos" ) ;
		marker.nLength = pxmlTag->GetAttrIntegerAs( L"length" ) ;
		//
		m_ssaMarkers.Add( *pstrID, marker ) ;
	}
	return	errSuccess ;
}

// マーカー取得
//////////////////////////////////////////////////////////////////////////////
const AGLMessageProcessor::SoundMarkList::Marker *
	AGLMessageProcessor::SoundMarkList::GetMarkerAs( const wchar_t * pwszID ) const
{
	return	m_ssaMarkers.GetAs( pwszID ) ;
}

const AGLMessageProcessor::SoundMarkList::Marker *
	AGLMessageProcessor::SoundMarkList::GetMarkerAt( size_t i ) const
{
	return	m_ssaMarkers.GetAt( i ) ;
}

size_t AGLMessageProcessor::SoundMarkList::GetMarkerCount( void ) const
{
	return	m_ssaMarkers.GetLength() ;
}


//////////////////////////////////////////////////////////////////////////////
// メッセージ処理
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( AntirrhinumGL::AGLMessageProcessor, AGLEpicFuncProcessor )
AGL_IMPLEMENT_EPIC_PROCESSOR( AntirrhinumGL::AGLMessageProcessor )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
AGLMessageProcessor::AGLMessageProcessor( void )
	: AGLEpicFuncProcessor( m_pFirstFuncDesc, L"message" ),
		m_flagsBehavior( behaviorSkipAll | behaviorClickCancelAll ),
		m_nSkipEffectSpeed( 0 ),
		m_msecMsgWindowFade( 300 ),
		m_nMsgLogLimit( 0x100 ),
		m_iFirstVolLine( SGLAudioPlayer::lineUserFirst ),
		m_flagDelayClearFace( false ),
		m_flagWaitingMsgClick( false ),
		m_maskDisableSkip( 0 ),
		m_maskDisableClickSkip( 0 ),
		m_maskKeepClickSkip( 0 ),
		m_maskDisableButton( 0 ),
		m_flagSceneSkip( false ),
		m_idSceneSkipThread( 0 ),
		m_flagShowWindow( false ),
		m_flagShowSelector( false ),
		m_nSelectionTimeout( -1 )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
AGLMessageProcessor::~AGLMessageProcessor( void )
{
}

// 音量ライン設定
//////////////////////////////////////////////////////////////////////////////
void AGLMessageProcessor::SetFirstVolumeLine( size_t iLine, bool flagEnable )
{
	m_iFirstVolLine = iLine ;
}

// ボイス再生
//////////////////////////////////////////////////////////////////////////////
AGLMessageProcessor::Voice AGLMessageProcessor::PlayVoice
	( const wchar_t * pwszFileName, uint32_t nFlags )
{
	SString	strFileName = pwszFileName ;
	if ( SString( strFileName.GetFileExtensionPart() ).IsEmpty() )
	{
		strFileName += L".mio" ;
	}
	const CharConfig *	pcfg =
			GetCharVoiceFileOf( SString( strFileName.GetFileNamePart() ) ) ;
	VoicePlayer *	pVoice = LoadVoicePlayer( strFileName, pcfg, nFlags ) ;
	if ( pVoice == nullptr )
	{
		return	Voice() ;
	}
	Voice	voice( pVoice ) ;
	OnLoadedVoice( voice ) ;
	//
	SGLAudioPlayer *	pPlayer = pVoice->GetAudioPlayer() ;
	if ( pPlayer != nullptr )
	{
		float32_t	vols[2] = { 1.0f, 1.0f } ;
		OnVoiceVolume( voice, vols ) ;
		pPlayer->SetVolume( vols, 2 ) ;
	}
	//
	bool	flagStarted = false ;
	m_csSync.Lock() ;
	if ( !(nFlags & playNoStart) )
	{
		m_aPlaying.Add( pVoice ) ;
		//
		pPlayer = pVoice->GetAudioPlayer() ;
		if ( pPlayer != nullptr )
		{
			pPlayer->Play() ;
			pVoice->m_flagStarted = true ;
			flagStarted = true ;
		}
	}
	m_csSync.Unlock() ;
	//
	if ( flagStarted )
	{
		OnStartedVoice( voice ) ;
	}
	return	voice ;
}

void AGLMessageProcessor::PlayVoices
	( const wchar_t *const* ppwszFileNames, size_t nCount, uint32_t nFlags )
{
	for ( size_t i = 0; i < nCount; i ++ )
	{
		PlayVoice( ppwszFileNames[i], nFlags ) ;
	}
}

// 再生中の全ボイス停止
//////////////////////////////////////////////////////////////////////////////
void AGLMessageProcessor::StopAllVoices( uint32_t nFadeout )
{
	m_csSync.Lock() ;
	while ( m_aPlaying.GetLength() > 0 )
	{
		VoicePlayer *	pVoice =
			m_aPlaying.DetachAt( m_aPlaying.GetLength() - 1 ) ;
		if ( pVoice != nullptr )
		{
			if ( nFadeout > 0 )
			{
				ESLAssert( pVoice->m_pPlayer != nullptr ) ;
				float32_t	vols[2] = { 0.0f, 0.0f }  ;
				pVoice->m_pPlayer->BeginFadeVolume( vols, 2, nFadeout ) ;
				m_aFadeout.Add( pVoice ) ;
			}
			else
			{
				Voice	voice( pVoice ) ;
				m_csSync.Unlock() ;
				//
				OnFinishVoice( voice ) ;
				//
				m_csSync.Lock() ;
				delete	pVoice ;
			}
		}
	}
	m_csSync.Unlock() ;
}

// 再生中か？
//////////////////////////////////////////////////////////////////////////////
bool AGLMessageProcessor::IsPlayingVoice( const Voice& voice ) const
{
	SSmartLock<const SCriticalSection>	lock( &m_csSync ) ;
	VoicePlayer *	pVoice = voice.GetReference() ;
	if ( (pVoice != nullptr)
		&& (pVoice->m_pPlayer != nullptr) )
	{
		return	(m_aPlaying.FindPtr( pVoice ) >= 0)
				&& pVoice->m_pPlayer->IsPlaying() ;
	}
	return	false ;
}

bool AGLMessageProcessor::ArePlayingAnyVoices( void ) const
{
	SSmartLock<const SCriticalSection>	lock( &m_csSync ) ;
	for ( size_t i = 0; i < m_aPlaying.GetLength(); i ++ )
	{
		VoicePlayer *	pVoice = m_aPlaying.GetAt( i ) ;
		if ( (pVoice != nullptr)
			&& (pVoice->m_pPlayer != nullptr) )
		{
			if ( pVoice->m_pPlayer->IsPlaying() )
			{
				return	true ;
			}
		}
	}
	return	false ;
}

// ボイス再生位置通過判定
//////////////////////////////////////////////////////////////////////////////
bool AGLMessageProcessor::IsVoicePastMark( const wchar_t * pwszID ) const
{
	SSmartLock<const SCriticalSection>	lock( &m_csSync ) ;
	bool	flagAnyPlaying = false ;
	for ( size_t i = 0; i < m_aPlaying.GetLength(); i ++ )
	{
		VoicePlayer *	pVoice = m_aPlaying.GetAt( i ) ;
		if ( (pVoice != nullptr)
			&& (pVoice->m_pPlayer != nullptr)
			&& pVoice->m_pPlayer->IsPlaying() )
		{
			flagAnyPlaying = true ;
			//
			const SoundMarkList::Marker *
							pMarker = pVoice->GetSoundMarkerAs( pwszID ) ;
			if ( pMarker != nullptr )
			{
				uint64_t	nPlayingPos = pVoice->m_pPlayer->GetPosition() ;
				if ( nPlayingPos >= pMarker->nPos )
				{
					return	true ;
				}
			}
		}
	}
	return	!flagAnyPlaying ;
}

// 音量設定
//////////////////////////////////////////////////////////////////////////////
SSystem::SError AGLMessageProcessor::SetVoiceVolume
	( Voice& voice, double volLeft, double volRight ) const
{
	SSmartLock<const SCriticalSection>	lock( &m_csSync ) ;
	VoicePlayer *	pVoice = voice.GetReference() ;
	if ( (pVoice != nullptr)
		&& (pVoice->m_pPlayer != nullptr) )
	{
		float32_t	vols[2] = { (float32_t) volLeft, (float32_t) volRight } ;
		OnVoiceVolume( voice, vols ) ;
		pVoice->m_pPlayer->SetVolume( vols, 2 ) ;
		return	errSuccess ;
	}
	return	errFailed ;
}

// 再生開始
//////////////////////////////////////////////////////////////////////////////
SSystem::SError AGLMessageProcessor::StartVoice( Voice& voice )
{
	m_csSync.Lock() ;
	VoicePlayer *	pVoice = voice.GetReference() ;
	if ( (pVoice != nullptr)
		&& (pVoice->m_pPlayer != nullptr) )
	{
		if ( m_aPlaying.FindPtr( pVoice ) < 0 )
		{
			m_aPlaying.Add( pVoice ) ;
		}
		pVoice->m_pPlayer->Play() ;
		pVoice->m_flagStarted = true ;
		m_csSync.Unlock() ;
		//
		OnStartedVoice( voice ) ;
		return	errSuccess ;
	}
	m_csSync.Unlock() ;
	return	errFailed ;
}

// 停止
//////////////////////////////////////////////////////////////////////////////
SSystem::SError
	AGLMessageProcessor::StopVoice( Voice& voice, uint32_t nFadeout )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	VoicePlayer *	pVoice = voice.GetReference() ;
	ssize_t			iVoice = m_aPlaying.FindPtr( pVoice ) ;
	if ( iVoice >= 0 )
	{
		if ( nFadeout > 0 )
		{
			ESLAssert( pVoice->m_pPlayer != nullptr ) ;
			float32_t	vols[2] = { 0.0f, 0.0f }  ;
			pVoice->m_pPlayer->BeginFadeVolume( vols, 2, nFadeout ) ;
			m_aPlaying.DetachAt( (size_t) iVoice ) ;
			m_aFadeout.Add( pVoice ) ;
		}
		else
		{
			m_csSync.Unlock() ;
			//
			OnFinishVoice( voice ) ;
			//
			m_csSync.Lock() ;
			m_aPlaying.RemoveAt( (size_t) iVoice ) ;
		}
		return	errSuccess ;
	}
	return	errFailed ;
}

// ファイルロード
//////////////////////////////////////////////////////////////////////////////
AGLMessageProcessor::VoicePlayer *
	AGLMessageProcessor::LoadVoicePlayer
		( const wchar_t * pwszFileName,
				const CharConfig * pcfg, uint32_t nFlags )
{
	SGLAudioPlayer *	pPlayer = new SGLAudioPlayer ;
	if ( pPlayer->Open
		( pwszFileName, SGLAudioPlayerInterface::modeOpenStatic ) )
	{
		delete	pPlayer ;
		return	nullptr ;
	}
	SString			strFileName = pwszFileName ;
	SString			strFileDir = strFileName.GetFileDirectoryPart() ;
	SString			strFileTitle = strFileName.GetFileTitlePart() ;
	SoundMarkList *	pMarkers = new SoundMarkList ;
	if ( pMarkers->LoadMarkerFile
		( strFileDir.OffsetFilePath( strFileTitle + L".wmk" ) ) )
	{
		delete	pMarkers ;
		pMarkers = nullptr ;
	}
	return	new VoicePlayer( pPlayer, pwszFileName, pcfg, pMarkers, nFlags ) ;
}

// ロード完了後処理
//////////////////////////////////////////////////////////////////////////////
void AGLMessageProcessor::OnLoadedVoice
	( const AGLMessageProcessor::Voice& voice )
{
	SGLAudioPlayer *	pPlayer = voice.GetAudioPlayer() ;
	const CharConfig *	pcfg = voice.GetCharConfig() ;
	if ( (pPlayer != nullptr) && (pcfg != nullptr) )
	{
		pPlayer->SetVolumeLineMask
			( (1 << SGLAudioPlayer::lineVoice)
				| (1 << (m_iFirstVolLine + pcfg->m_iVolLineOffset)) ) ;
	}
	//
	AGLKernel::EpicProcessorIterator	iter = m_pKernel->FirstEpicProcessor() ;
	for ( ; ; )
	{
		VoiceListener *	pListener = m_pKernel->NextEpicProcessor<VoiceListener>( iter ) ;
		if ( pListener == nullptr )
		{
			break ;
		}
		pListener->OnLoadedVoice( voice ) ;
	}
}

// ボイス音量効果
//////////////////////////////////////////////////////////////////////////////
void AGLMessageProcessor::OnVoiceVolume( const Voice& voice, float32_t vols[2] ) const
{
	AGLKernel::EpicProcessorIterator	iter = m_pKernel->FirstEpicProcessor() ;
	for ( ; ; )
	{
		VoiceListener *	pListener = m_pKernel->NextEpicProcessor<VoiceListener>( iter ) ;
		if ( pListener == nullptr )
		{
			break ;
		}
		pListener->OnVoiceVolume( voice, vols ) ;
	}
}

// ボイス再生開始時処理
//////////////////////////////////////////////////////////////////////////////
void AGLMessageProcessor::OnStartedVoice
	( const AGLMessageProcessor::Voice& voice )
{
	AGLKernel::EpicProcessorIterator	iter = m_pKernel->FirstEpicProcessor() ;
	for ( ; ; )
	{
		VoiceListener *	pListener = m_pKernel->NextEpicProcessor<VoiceListener>( iter ) ;
		if ( pListener == nullptr )
		{
			break ;
		}
		pListener->OnStartedVoice( voice ) ;
	}
}

// 再生終了時処理
//////////////////////////////////////////////////////////////////////////////
void AGLMessageProcessor::OnFinishVoice
	( const AGLMessageProcessor::Voice& voice )
{
	AGLKernel::EpicProcessorIterator	iter = m_pKernel->FirstEpicProcessor() ;
	for ( ; ; )
	{
		VoiceListener *	pListener = m_pKernel->NextEpicProcessor<VoiceListener>( iter ) ;
		if ( pListener == nullptr )
		{
			break ;
		}
		pListener->OnFinishedVoice( voice ) ;
	}
}

// ファイル名マッチング
//////////////////////////////////////////////////////////////////////////////
const AGLMessageProcessor::CharConfig *
	AGLMessageProcessor::GetCharIndexOf( ssize_t iChar ) const
{
	for ( size_t i = 0; i < m_aCharConfig.GetLength(); i ++ )
	{
		CharConfig *	pcfg = m_aCharConfig.GetAt( i ) ;
		ESLAssert( pcfg != nullptr ) ;
		if ( (pcfg != nullptr)
			&& (pcfg->m_nIndex == iChar) )
		{
			return	pcfg ;
		}
	}
	return	nullptr ;
}

const AGLMessageProcessor::CharConfig *
	AGLMessageProcessor::GetCharIdOf( const wchar_t * pwszID ) const
{
	for ( size_t i = 0; i < m_aCharConfig.GetLength(); i ++ )
	{
		CharConfig *	pcfg = m_aCharConfig.GetAt( i ) ;
		ESLAssert( pcfg != nullptr ) ;
		if ( (pcfg != nullptr)
			&& (pcfg->m_idChar == pwszID) )
		{
			return	pcfg ;
		}
	}
	return	nullptr ;
}

const AGLMessageProcessor::CharConfig *
	AGLMessageProcessor::GetCharBsFileOf( const wchar_t * pwszFileName ) const
{
	SParserErrorTracer	perrTrace ;
	SStringParser		sparsFileName ;
	SString				strFileName = pwszFileName ;
	sparsFileName.AttachString( strFileName ) ;
	//
	for ( size_t i = 0; i < m_aCharConfig.GetLength(); i ++ )
	{
		CharConfig *	pcfg = m_aCharConfig.GetAt( i ) ;
		ESLAssert( pcfg != nullptr ) ;
		if ( pcfg == nullptr )
		{
			continue ;
		}
		if ( !pcfg->m_strBsLFile.IsEmpty()
			&& (strFileName.CompareLeftNoCase( pcfg->m_strBsLFile ) != 0) )
		{
			continue ;
		}
		for ( size_t j = 0; j < pcfg->m_aBsFileMatching.GetLength(); j ++ )
		{
			SUsageMatcher *	pMatcher = pcfg->m_aBsFileMatching.GetAt( j ) ;
			ESLAssert( pMatcher != nullptr ) ;
			if ( pMatcher == nullptr )
			{
				continue ;
			}
			sparsFileName.SeekIndex( 0 ) ;
			if ( !pMatcher->IsMatchedWith( sparsFileName, nullptr, perrTrace ) )
			{
				return	pcfg ;
			}
		}
	}
	return	nullptr ;
}

const AGLMessageProcessor::CharConfig *
	AGLMessageProcessor::GetCharVoiceFileOf( const wchar_t * pwszFileName ) const
{
	SParserErrorTracer	perrTrace ;
	SStringParser		sparsFileName ;
	SString				strFileName = pwszFileName ;
	sparsFileName.AttachString( strFileName ) ;
	//
	for ( size_t i = 0; i < m_aCharConfig.GetLength(); i ++ )
	{
		CharConfig *	pcfg = m_aCharConfig.GetAt( i ) ;
		ESLAssert( pcfg != nullptr ) ;
		if ( pcfg == nullptr )
		{
			continue ;
		}
		if ( !pcfg->m_strVoiceLFile.IsEmpty()
			&& (strFileName.CompareLeftNoCase( pcfg->m_strVoiceLFile ) != 0) )
		{
			continue ;
		}
		bool	flagException = false ;
		for ( size_t j = 0; j < pcfg->m_aVoiceFileUnmatching.GetLength(); j ++ )
		{
			SUsageMatcher *	pMatcher = pcfg->m_aVoiceFileUnmatching.GetAt( j ) ;
			ESLAssert( pMatcher != nullptr ) ;
			if ( pMatcher == nullptr )
			{
				continue ;
			}
			sparsFileName.SeekIndex( 0 ) ;
			if ( !pMatcher->IsMatchedWith( sparsFileName, nullptr, perrTrace ) )
			{
				flagException = true ;
				break ;
			}
		}
		if ( flagException )
		{
			continue ;
		}
		for ( size_t j = 0; j < pcfg->m_aVoiceFileMatching.GetLength(); j ++ )
		{
			SUsageMatcher *	pMatcher = pcfg->m_aVoiceFileMatching.GetAt( j ) ;
			ESLAssert( pMatcher != nullptr ) ;
			if ( pMatcher == nullptr )
			{
				continue ;
			}
			sparsFileName.SeekIndex( 0 ) ;
			if ( !pMatcher->IsMatchedWith( sparsFileName, nullptr, perrTrace ) )
			{
				return	pcfg ;
			}
		}
	}
	return	nullptr ;
}

// 動作フラグ (BehaviorFlag 組み合わせ)
//////////////////////////////////////////////////////////////////////////////
uint32_t AGLMessageProcessor::GetBehaviorFlags( void )
{
	return	m_flagsBehavior ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SSystem::SError AGLMessageProcessor::Serialize
		( SSystem::SXMLDocument& xmlTag, AGLKernel * pKernel )
{
	xmlTag.SetAttrHexIntegerAs( L"disable_skip", m_maskDisableSkip ) ;
	xmlTag.SetAttrHexIntegerAs( L"disable_click_skip", m_maskDisableClickSkip ) ;
	xmlTag.SetAttrHexIntegerAs( L"keep_click_skip", m_maskKeepClickSkip ) ;
	xmlTag.SetAttrHexIntegerAs( L"disable_button", m_maskDisableButton ) ;
	xmlTag.SetAttrIntegerAs( L"enable_scene_skip", m_flagSceneSkip ? 1 : 0 ) ;
	xmlTag.SetAttrIntegerAs( L"scene_skip_thread", m_idSceneSkipThread ) ;
	xmlTag.SetAttributeAs( L"jump_scene_skip", m_strJumpSceneSkip ) ;

	xmlTag.SetAttributeAs( L"msg_window_id", m_strMsgWindowID ) ;

	UIMessage *	puiMsg = m_refMsgWindow ;
	xmlTag.SetAttrIntegerAs
		( L"show_msg_window",
				((puiMsg != nullptr) && puiMsg->IsShowWindow()) ? 1 : 0 ) ;

	xmlTag.SetAttributeAs( L"face", m_strCurFace ) ;
	xmlTag.SetAttributeAs( L"name", m_strCurName ) ;
	xmlTag.SetAttributeAs( L"message", m_strCurMessage ) ;

	xmlTag.SetAttrIntegerAs( L"selector", m_flagShowSelector ? 1 : 0 ) ;
	xmlTag.SetAttrIntegerAs( L"sel_timeout", m_nSelectionTimeout ) ;
	xmlTag.SetAttributeAs( L"sel_target", m_strSelResultTarget ) ;

	SXMLDocument *	pxmlSelector = xmlTag.CreateElementTagAs( L"selector" ) ;
	for ( size_t i = 0; i < m_aMenuItems.GetLength(); i ++ )
	{
		MenuItem *	pMenu = m_aMenuItems.GetAt( i ) ;
		ESLAssert( pMenu != nullptr ) ;
		if ( pMenu != nullptr )
		{
			pMenu->m_xmlMenuCmd.SetTag( L"menu_item" ) ;
			pMenu->m_xmlMenuCmd.SetAttributeAs( L"text", pMenu->m_strText ) ;
			pMenu->m_xmlMenuCmd.SetAttributeAs( L"jump", pMenu->m_strJump ) ;
			pMenu->m_xmlMenuCmd.SetAttrIntegerAs( L"value", pMenu->m_nValue ) ;
			pMenu->m_xmlMenuCmd.SetAttrIntegerAs( L"history", pMenu->m_flagHistory ? 1 : 0 ) ;
			//
			pxmlSelector->AddElement( new SXMLDocument( pMenu->m_xmlMenuCmd ) ) ;
		}
	}

	return	errSuccess ;
}

// デシリアライズ
//////////////////////////////////////////////////////////////////////////////
SSystem::SError AGLMessageProcessor::Deserialize
		( const SSystem::SXMLDocument& xmlTag, AGLKernel * pKernel )
{
	m_maskDisableSkip =
		(uint32_t) xmlTag.GetAttrHexIntegerAs
						( L"disable_skip", m_maskDisableSkip ) ;
	m_maskDisableClickSkip =
		(uint32_t) xmlTag.GetAttrHexIntegerAs
						( L"disable_click_skip", m_maskDisableClickSkip ) ;
	m_maskKeepClickSkip =
		(uint32_t) xmlTag.GetAttrHexIntegerAs
						( L"keep_click_skip", m_maskKeepClickSkip ) ;
	m_maskDisableButton =
		(uint32_t) xmlTag.GetAttrHexIntegerAs
						( L"disable_button", m_maskDisableButton ) ;
	m_flagSceneSkip =
		(xmlTag.GetAttrIntegerAs( L"enable_scene_skip", 0 ) != 0) ;
	m_idSceneSkipThread =
		(uint32_t) xmlTag.GetAttrIntegerAs
						( L"scene_skip_thread", m_idSceneSkipThread ) ;
	m_strJumpSceneSkip =
		xmlTag.GetAttrStringAs( L"jump_scene_skip", m_strJumpSceneSkip ) ;

	m_strMsgWindowID = xmlTag.GetAttrStringAs( L"msg_window_id", m_strMsgWindowID ) ;
	m_flagShowWindow = (xmlTag.GetAttrIntegerAs( L"show_msg_window" ) != 0) ;

	m_strCurFace = xmlTag.GetAttrStringAs( L"face", m_strCurFace ) ;
	m_strCurName = xmlTag.GetAttrStringAs( L"name", m_strCurName ) ;
	m_strCurMessage = xmlTag.GetAttrStringAs( L"message", m_strCurMessage ) ;

	m_flagShowSelector = (xmlTag.GetAttrIntegerAs( L"selector", 0 ) != 0) ;
	m_nSelectionTimeout =
		(int32_t) xmlTag.GetAttrIntegerAs( L"sel_timeout", m_nSelectionTimeout ) ;
	m_strSelResultTarget =
			xmlTag.GetAttrStringAs( L"sel_target", m_strSelResultTarget ) ;

	SXMLDocument *	pxmlSelector = xmlTag.GetElementTagAs( L"selector" ) ;
	m_aMenuItems.RemoveAll() ;
	if ( pxmlSelector != nullptr )
	{
		for ( size_t i = 0; i < pxmlSelector->GetElementsCount(); i ++ )
		{
			SXMLDocument *	pxmlMenu = pxmlSelector->GetElementAt( i ) ;
			if ( (pxmlMenu == nullptr) || (pxmlMenu->GetTag() != L"menu_item") )
			{
				continue ;
			}
			MenuItem *	pMenu = new MenuItem ;
			m_aMenuItems.Add( pMenu ) ;
			//
			pMenu->m_strText = pxmlMenu->GetAttrStringAs( L"text" ) ;
			pMenu->m_strJump = pxmlMenu->GetAttrStringAs( L"jump" ) ;
			pMenu->m_nValue = (int) pxmlMenu->GetAttrIntegerAs( L"value" ) ;
			pMenu->m_iMsgIndex = (size_t) pxmlMenu->GetAttrIntegerAs( L"msg_index" ) ;
			pMenu->m_flagHistory = (pxmlMenu->GetAttrIntegerAs( L"history" ) != 0) ;
			pMenu->m_xmlMenuCmd = *pxmlMenu ;
		}
	}

	m_aMsgLog.RemoveAll() ;

	return	errSuccess ;
}

// デシリアライズ後の参照解決処理
//////////////////////////////////////////////////////////////////////////////
SSystem::SError AGLMessageProcessor::AfterDeserialize( AGLKernel * pKernel )
{
	m_refMsgWindow = GetMessageUI( m_strMsgWindowID ) ;
	ClearCurrentMessageWindow() ;
	ApplyDisableToggleButtonFlags() ;

	ShowMessageWindow( m_flagShowWindow, 0 ) ;
	DisplayFace( m_strCurFace ) ;
	DisplayName( m_strCurName ) ;
	DisplayMessage( m_strCurMessage, true ) ;

	if ( m_flagShowSelector )
	{
		BuildSelector() ;
	}

	return	errSuccess ;
}

// 設定
//////////////////////////////////////////////////////////////////////////////
void AGLMessageProcessor::LoadConfiguration
	( const SSystem::SXMLDocument& xmlConfig )
{
	const SXMLDocument *	pxmlCharDef =
				xmlConfig.GetElementTagAs( L"character_definition" ) ;
	if ( pxmlCharDef != nullptr )
	{
		LoadCharacterDefinition( *pxmlCharDef ) ;
	}
	const SXMLDocument *	pxmlMessage =
				xmlConfig.GetElementTagAs( L"message" ) ;
	if ( pxmlMessage != nullptr )
	{
		LoadMessageConfig( *pxmlMessage ) ;
	}
}

void AGLMessageProcessor::LoadCharacterDefinition
	( const SSystem::SXMLDocument& xmlCharDef )
{
	m_aCharConfig.RemoveAll() ;
	//
	for ( size_t i = 0; i < xmlCharDef.GetElementsCount(); i ++ )
	{
		const SXMLDocument *	pxmlChar = xmlCharDef.GetElementAt( i ) ;
		if ( (pxmlChar == nullptr)
			|| (pxmlChar->GetTag() != L"character") )
		{
			continue ;
		}
		CharConfig *	pcfg = new CharConfig ;
		pcfg->m_nIndex = (ssize_t) pxmlChar->GetAttrIntegerAs( L"index", 0 ) ;
		pcfg->m_idChar = pxmlChar->GetAttrStringAs( L"id" ) ;
		pcfg->m_strName = pxmlChar->GetAttrStringAs( L"name" ) ;
		//
		for ( int i = 0; i < languageTypeCount; i ++ )
		{
			SString	strNameAttr ;
			strNameAttr.Format( L"name_%s", g_pwszLanguageSignatures[i] ) ;
			//
			const SString *	pstrNameLang = pxmlChar->GetAttributeAs( strNameAttr ) ;
			if ( pstrNameLang != nullptr )
			{
				pcfg->m_isoaName.SetAs( i, new SString( *pstrNameLang ) ) ;
			}
		}
		//
		pcfg->m_strFaceLFile = pxmlChar->GetAttrStringAs( L"face_file_lead" ) ;
		pcfg->m_strBsLFile = pxmlChar->GetAttrStringAs( L"bs_file_lead" ) ;
		pcfg->m_strVoiceLFile = pxmlChar->GetAttrStringAs( L"voice_file_lead" ) ;
		pcfg->m_iVolLineOffset =
			(size_t) pxmlChar->GetAttrIntegerAs( L"volume_line" ) ;
		pcfg->m_argbText =
			(uint32_t) pxmlChar->GetAttrHexIntegerAs( L"text_color", 0xFFFFFFFF ) ;
		pcfg->m_argbBorder =
			(uint32_t) pxmlChar->GetAttrHexIntegerAs( L"border_color", 0xFF000000 ) ;
		pcfg->m_argbShadow =
			(uint32_t) pxmlChar->GetAttrHexIntegerAs( L"shadow_color", 0x80000000 ) ;
		//
		const SXMLDocument *	pxmlBsFiles =
				pxmlChar->GetElementTagAs( L"bs_file_filter" ) ;
		if ( pxmlBsFiles != nullptr )
		{
			ParseCharConfigMatchingList
				( pcfg->m_aBsFileMatching, pxmlBsFiles ) ;
		}
		//
		const SXMLDocument *	pxmlVoiceFiles =
				pxmlChar->GetElementTagAs( L"voice_file_filter" ) ;
		if ( pxmlVoiceFiles != nullptr )
		{
			ParseCharConfigMatchingList
				( pcfg->m_aVoiceFileMatching, pxmlVoiceFiles ) ;
		}
		//
		const SXMLDocument *	pxmlNoVoiceFiles =
				pxmlChar->GetElementTagAs( L"voice_file_exception" ) ;
		if ( pxmlNoVoiceFiles != nullptr )
		{
			ParseCharConfigMatchingList
				( pcfg->m_aVoiceFileUnmatching, pxmlNoVoiceFiles ) ;
		}
		//
		m_aCharConfig.Add( pcfg ) ;
	}
}

void AGLMessageProcessor::LoadMessageConfig
	( const SSystem::SXMLDocument& xmlMessage )
{
	static const SXMLDocument::AttrInteger	s_aiBehaviorFlags[] =
	{
		{ L"stop_skip_no_read", behaviorStopSkipNoRead },
		{ L"stop_fast_skip_no_read", behaviorStopFastSkipNoRead },
		{ L"stop_skip_selector", behaviorStopSkipSelector },
		{ L"save_in_log", behaviorSaveInLog },
		{ L"nosave_in_skip", behaviorNoSaveInSkip },
		{ L"nosave_in_fast_skip", behaviorNoSaveInFastSkip },
		{ L"click_cancel_auto", behaviorClickCancelAuto },
		{ L"click_cancel_skip", behaviorClickCancelSkip },
		{ L"click_cancel_fast_skip", behaviorClickCancelFastSkip },
		{ nullptr, 0 },
	} ;
	for ( size_t i = 0; s_aiBehaviorFlags[i].pszSymbol != nullptr; i ++ )
	{
		uint32_t	nFlag = (uint32_t) s_aiBehaviorFlags[i].nValue ;
		if ( xmlMessage.GetAttrIntegerAs
			( s_aiBehaviorFlags[i].pszSymbol,
				(m_flagsBehavior & nFlag) ? 1 : 0 ) != 0 )
		{
			m_flagsBehavior |= nFlag ;
		}
		else
		{
			m_flagsBehavior &= ~nFlag ;
		}
	}
	m_nSkipEffectSpeed =
		(uint32_t) esl_clampi
			( (int) xmlMessage.GetAttrIntegerAs
					( L"skip_effect_speed", (int) m_nSkipEffectSpeed ), 0, 100 ) ;
	m_msecMsgWindowFade =
		(uint32_t) xmlMessage.GetAttrIntegerAs
					( L"msg_window_fade_time", m_msecMsgWindowFade ) ;
	m_nMsgLogLimit =
		(size_t) xmlMessage.GetAttrIntegerAs( L"msg_log_limit", m_nMsgLogLimit ) ;
	m_iFirstVolLine =
		(size_t) xmlMessage.GetAttrIntegerAs
			( L"base_char_vol_line",
				m_iFirstVolLine - SGLAudioPlayer::lineUserFirst )
		+ SGLAudioPlayer::lineUserFirst ;
}

void AGLMessageProcessor::ParseCharConfigMatchingList
	( SSystem::SObjectArray<SSystem::SUsageMatcher>& aMatchers,
		const SXMLDocument * pxmlFileFilters )
{
	SParserErrorTracer	perrTrace ;
	for ( size_t i = 0; i < pxmlFileFilters->GetElementsCount(); i ++ )
	{
		const SXMLDocument *
			pxmlMatcher = pxmlFileFilters->GetElementAt( i ) ;
		if ( (pxmlMatcher == nullptr)
			|| (pxmlMatcher->GetTag() != L"filter") )
		{
			continue ;
		}
		SUsageMatcher *	pMatcher = new SUsageMatcher ;
		if ( pMatcher->ParseUsage
			( pxmlMatcher->GetAttrStringAs( L"usage" ), perrTrace ) )
		{
			delete	pMatcher ;
			continue ;
		}
		aMatchers.Add( pMatcher ) ;
	}
}

// タイマ処理 (実行フレーム前処理)
//////////////////////////////////////////////////////////////////////////////
void AGLMessageProcessor::OnKernelTimer( void )
{
	CleanupFadeoutVoices() ;
	OnTimerClearFace() ;
	OnTimerKeyWaitDisplay() ;
	OnTimerSceneJump() ;
	OnTimerMessageWindow() ;
	OnTimerSelector() ;
}

void AGLMessageProcessor::CleanupFadeoutVoices( void )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	for ( size_t i = 0; i < m_aFadeout.GetLength(); i ++ )
	{
		VoicePlayer *	pVoice = m_aFadeout.GetAt( i ) ;
		if ( (pVoice == nullptr)
			|| (pVoice->m_pPlayer == nullptr)
			|| !pVoice->m_pPlayer->IsPlaying()
			|| !pVoice->m_pPlayer->IsVolumeFading() )
		{
			m_aFadeout.DetachAt( i -- ) ;
			//
			Voice	voice( pVoice ) ;
			m_csSync.Unlock() ;
			//
			OnFinishVoice( voice ) ;
			//
			m_csSync.Lock() ;
			delete	pVoice ;
		}
	}
}

void AGLMessageProcessor::OnTimerClearFace( void )
{
	if ( m_flagDelayClearFace )
	{
		UIMessage *	puiMsg = m_refMsgWindow ;
		if ( puiMsg != nullptr )
		{
			puiMsg->ClearFace() ;
		}
		m_flagDelayClearFace = false ;
	}
}

void AGLMessageProcessor::OnTimerKeyWaitDisplay( void )
{
	UIMessage *	puiMsg = m_refMsgWindow ;
	if ( puiMsg != nullptr )
	{
		UIMessage::KeyWaitType	keyWait = UIMessage::keyWaitHide ;
		if ( IsSkipMode() )
		{
			keyWait = UIMessage::keyWaitInSkip ;
		}
		else if ( IsAutoMode() )
		{
			keyWait = UIMessage::keyWaitInAuto ;
		}
		else if ( m_flagWaitingMsgClick )
		{
			keyWait = UIMessage::keyWaitClick ;
		}
		if ( puiMsg->CurrentKeyWait() != keyWait )
		{
			puiMsg->ShowKeyWait( keyWait ) ;
		}
	}
}

void AGLMessageProcessor::OnTimerSceneJump( void )
{
	UIMessage *	puiMsg = m_refMsgWindow ;
	if ( m_flagSceneSkip && (puiMsg != nullptr)
		&& puiMsg->IsToggleButton( UIMessage::toggleSceneSkip ) )
	{
		AGLThread *	pThread = m_pKernel->GetThreadByID( m_idSceneSkipThread ) ;
		if ( pThread != nullptr )
		{
			if ( pThread->PostInterrupter
				( nullptr, m_strJumpSceneSkip, false ) == errSuccess )
			{
				m_flagSceneSkip = false ;
				puiMsg->SetToggleButton( UIMessage::toggleSceneSkip, false ) ;
				puiMsg->EnableToggleButton( UIMessage::toggleSceneSkip, false ) ;
			}
		}
	}
}

void AGLMessageProcessor::OnTimerMessageWindow( void )
{
	UIMessage *	puiMsg = m_refMsgWindow ;
	if ( puiMsg != nullptr )
	{
		puiMsg->OnAntirrhinumTimer() ;
	}
}

void AGLMessageProcessor::OnTimerSelector( void )
{
	UISelector *	puiSel = m_refSelector ;
	if ( puiSel != nullptr )
	{
		puiSel->OnAntirrhinumTimer() ;
	}
}

// ゲーム開始時処理
//////////////////////////////////////////////////////////////////////////////
void AGLMessageProcessor::InitializeGame( void )
{
	UIMessage *	puiMsg = GetMessageUI( nullptr ) ;
	m_refMsgWindow = puiMsg ;
	m_strMsgWindowID = (const wchar_t*) nullptr ;
	if ( puiMsg != nullptr )
	{
		puiMsg->ShowWindow( false, 0 ) ;
		puiMsg->SetToggleButton( UIMessage::toggleAuto, false ) ;
		puiMsg->SetToggleButton( UIMessage::toggleSkip, false ) ;
		puiMsg->SetToggleButton( UIMessage::toggleFastSkip, false ) ;
		puiMsg->SetToggleButton( UIMessage::toggleSceneSkip, false ) ;
		puiMsg->EnableToggleButton( UIMessage::toggleSceneSkip, false ) ;
	}
	//
	m_flagDelayClearFace = false ;
	m_flagWaitingMsgClick = false ;
	m_maskDisableSkip = 0 ;
	m_maskDisableClickSkip = 0 ;
	m_maskKeepClickSkip = 0 ;
	m_maskDisableButton = 0 ;
	m_flagSceneSkip = false ;
	m_flagShowWindow = false ;
	//
	m_aMsgLog.RemoveAll() ;
	//
	ClearCurrentMessageWindow() ;
	ApplyDisableToggleButtonFlags() ;
}

// ゲーム終了前フェードアウト処理
//////////////////////////////////////////////////////////////////////////////
void AGLMessageProcessor::FadeoutGame( uint32_t msecFadeout )
{
	UIMessage *	puiMsg = m_refMsgWindow ;
	if ( (puiMsg != nullptr) && puiMsg->IsShowWindow() )
	{
		puiMsg->ShowWindow( false, msecFadeout ) ;
	}
	m_flagShowWindow = false ;
}

// ゲーム終了時処理
//////////////////////////////////////////////////////////////////////////////
void AGLMessageProcessor::ReleaseGame( void )
{
	UIMessage *	puiMsg = m_refMsgWindow ;
	if ( puiMsg != nullptr )
	{
		puiMsg->ShowWindow( false, 0 ) ;
		puiMsg->SetToggleButton( UIMessage::toggleAuto, false ) ;
		puiMsg->SetToggleButton( UIMessage::toggleSkip, false ) ;
		puiMsg->SetToggleButton( UIMessage::toggleFastSkip, false ) ;
		puiMsg->SetToggleButton( UIMessage::toggleSceneSkip, false ) ;
		puiMsg->EnableToggleButton( UIMessage::toggleSceneSkip, false ) ;
	}
	m_flagShowWindow = false ;
}

// 待機関数を（ユーザー入力等により）即時に脱出すべきか判定する
//////////////////////////////////////////////////////////////////////////////
bool AGLMessageProcessor::ShouldAbortSync( SynchronismType type )
{
	if ( m_maskDisableSkip & (1 << type) )
	{
		return	false ;
	}
	UIMessage *	puiMsg = m_refMsgWindow ;
	if ( puiMsg != nullptr )
	{
		if ( !(m_maskDisableButton & (1 << UIMessage::toggleHWKeySkip))
				&& puiMsg->IsToggleButton( UIMessage::toggleHWKeySkip ) )
		{
			return	true ;
		}
		uint32_t	flagsBehavior = GetBehaviorFlags() ;
		if ( !(m_maskDisableButton & (1 << UIMessage::toggleFastSkip))
				&& puiMsg->IsToggleButton( UIMessage::toggleFastSkip ) )
		{
			if ( ((type == syncTypeTime) && (flagsBehavior & behaviorFastSkipTime))
				|| ((type == syncTypeMessage) && (flagsBehavior & behaviorFastSkipMessage))
				|| ((type == syncTypeEffect) && (flagsBehavior & behaviorFastSkipEffect))
				|| ((type == syncTypeEvent) && (flagsBehavior & behaviorFastSkipEvent)) )
			{
				return	true ;
			}
		}
		if ( !(m_maskDisableButton & (1 << UIMessage::toggleSkip))
				&& puiMsg->IsToggleButton( UIMessage::toggleSkip ) )
		{
			if ( ((type == syncTypeTime) && (flagsBehavior & behaviorSkipTime))
				|| ((type == syncTypeMessage) && (flagsBehavior & behaviorSkipMessage))
				|| ((type == syncTypeEffect) && (flagsBehavior & behaviorSkipEffect))
				|| ((type == syncTypeEvent) && (flagsBehavior & behaviorSkipEvent)) )
			{
				return	true ;
			}
		}
	}
	if ( m_maskDisableClickSkip & (1 << type) )
	{
		return	false ;
	}
	return	GetPushedKeyState( keyClickNext ) ;
}

// 待機関数を ShouldAbortSync を理由に脱出したことの通知
//////////////////////////////////////////////////////////////////////////////
void AGLMessageProcessor::NotifyAbortedSync( SynchronismType type )
{
	if ( GetPushedKeyState( keyClickNext ) )
	{
		UIMessage *	puiMsg = m_refMsgWindow ;
		if ( puiMsg != nullptr )
		{
			uint32_t	flagsBehavior = GetBehaviorFlags() ;
			if ( flagsBehavior & behaviorClickCancelAuto )
			{
				puiMsg->SetToggleButton( UIMessage::toggleAuto, false ) ;
			}
			if ( flagsBehavior & behaviorClickCancelSkip )
			{
				puiMsg->SetToggleButton( UIMessage::toggleSkip, false ) ;
			}
			if ( flagsBehavior & behaviorClickCancelFastSkip )
			{
				puiMsg->SetToggleButton( UIMessage::toggleFastSkip, false ) ;
			}
		}
	}
	if ( !(m_maskKeepClickSkip & (1 << type)) )
	{
		ClearPushedKeyState( keyClickNext ) ;
	}
}

// フェード処理などの効果継続時間の効果
//////////////////////////////////////////////////////////////////////////////
uint32_t AGLMessageProcessor::EffectTime( uint32_t msecTime, SynchronismType type )
{
	if ( IsSkipMode() )
	{
		if ( (!(m_maskDisableButton & (1 << UIMessage::toggleFastSkip))
						&& IsToggleButton( UIMessage::toggleFastSkip ))
			|| (!(m_maskDisableButton & (1 << UIMessage::toggleHWKeySkip))
						&& IsToggleButton( UIMessage::toggleHWKeySkip )) )
		{
			return	0 ;
		}
		return	msecTime * m_nSkipEffectSpeed / 100 ;
	}
	return	msecTime ;
}

// コマンド実装
//////////////////////////////////////////////////////////////////////////////
IMPL_ANTIRRHINUM_PROC(AGLMessageProcessor,voice)
{
	StopAllVoices( 0 ) ;
	PlayVoice
		( EvaluateExprInText( thread, code.GetAttrStringAs( L"src" ) ) ) ;
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLMessageProcessor,voices)
{
	SPointerArray<const wchar_t>	aVoiceFiles ;
	for ( size_t i = 0; true; i ++ )
	{
		SString	strAttrName ;
		strAttrName.Format( L"src%d", i ) ;
		//
		const SString *	pstrSrc = code.GetAttributeAs( strAttrName ) ;
		if ( pstrSrc == nullptr )
		{
			break ;
		}
		aVoiceFiles.Add( EvaluateExprInText( thread, *pstrSrc ) ) ;
	}
	StopAllVoices( 0 ) ;
	PlayVoices( aVoiceFiles.GetConstArray(), aVoiceFiles.GetLength() ) ;
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLMessageProcessor,voice_stop)
{
	StopAllVoices( (uint32_t) code.GetAttrIntegerAs( L"fadeout", 500 ) ) ;
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLMessageProcessor,voice_sync)
{
	const SString *	pstrID = code.GetAttributeAs( L"id" ) ;
	if ( thread.IsPermittedSkip( syncTypeEvent )
		&& m_pKernel->ShouldAbortSync( syncTypeEvent ) )
	{
		if ( pstrID == nullptr )
		{
			StopAllVoices( 0 ) ;
		}
		m_pKernel->NotifyAbortedSync( syncTypeEvent ) ;
		return	codeProcessed ;
	}
	if ( pstrID != nullptr )
	{
		if ( !IsVoicePastMark( *pstrID ) )
		{
			return	codePending ;
		}
	}
	else
	{
		if ( ArePlayingAnyVoices() )
		{
			return	codePending ;
		}
	}
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLMessageProcessor,msg_window_id)
{
	const SString *	pstrID = code.GetAttributeAs( L"id" ) ;
	if ( (pstrID == nullptr) || pstrID->IsEmpty() )
	{
		if ( !m_strMsgWindowID.IsEmpty() )
		{
			ClearCurrentMessageWindow() ;
			//
			m_strMsgWindowID = (const wchar_t*) nullptr ;
			m_refMsgWindow = GetMessageUI( nullptr ) ;
			//
			ClearCurrentMessageWindow() ;
			ApplyDisableToggleButtonFlags() ;
		}
	}
	else if ( m_strMsgWindowID != *pstrID )
	{
		ClearCurrentMessageWindow() ;
		//
		m_strMsgWindowID = EvaluateExprInText( thread, *pstrID ) ;
		m_refMsgWindow = GetMessageUI( *pstrID ) ;
		//
		ClearCurrentMessageWindow() ;
		ApplyDisableToggleButtonFlags() ;
	}
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLMessageProcessor,msg_show_window)
{
	UIMessage *	puiMsg = m_refMsgWindow ;
	if ( puiMsg == nullptr )
	{
		return	codeProcessed ;
	}
	bool		flagShow = (code.GetAttrIntegerAs( L"show", 1 ) != 0) ;
	uint32_t	msecTime = (uint32_t) code.GetAttrIntegerAs( L"time", 300 ) ;
	if ( puiMsg->IsShowWindow() == flagShow )
	{
		return	codeProcessed ;
	}
	SXMLDocument *	pxmlStorage = thread.GetLocalStrageAs( code.GetTag() ) ;
	if ( pxmlStorage->GetAttributeAs( L"waiting" ) == nullptr )
	{
		pxmlStorage->SetAttrIntegerAs( L"waiting", 1 ) ;
		//
		if ( !flagShow )
		{
			ClearCurrentMessageWindow() ;
		}
		puiMsg->ShowWindow( flagShow, m_pKernel->EffectTime( msecTime ) ) ;
		m_flagShowWindow = flagShow ;
	}
	else if ( puiMsg->IsPendingShowEffect() )
	{
		if ( !(thread.IsPermittedSkip( syncTypeEffect )
			&& m_pKernel->ShouldAbortSync( syncTypeEffect )) )
		{
			return	codePending ;
		}
		m_pKernel->NotifyAbortedSync( syncTypeEffect ) ;
		puiMsg->FinishShowWindow() ;
	}
	thread.ClearLocalStrage( pxmlStorage ) ;
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLMessageProcessor,msg_set_face)
{
	UIMessage *	puiMsg = m_refMsgWindow ;
	if ( puiMsg == nullptr )
	{
		return	codeProcessed ;
	}
	const SString *	pstrFace = code.GetAttributeAs( L"face" ) ;
	if ( pstrFace != nullptr )
	{
		m_strCurFace = EvaluateExprInText( thread, *pstrFace ) ;
		puiMsg->DisplayFace( m_strCurFace ) ;
		m_flagDelayClearFace = false ;
	}
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLMessageProcessor,msg_clear_face)
{
	m_flagDelayClearFace = true ;	// 表示のチラつき抑制の為、遅延クリア
	m_strCurFace = L"" ;
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLMessageProcessor,msg_pause)
{
	SXMLDocument *	pxmlStorage = thread.GetLocalStrageAs( code.GetTag() ) ;
	if ( pxmlStorage->GetAttributeAs( L"tick_start" ) == nullptr )
	{
		pxmlStorage->SetAttrIntegerAs( L"tick_start", thread.GetThreadTick() ) ;
		//
		ssize_t	iMsgIndex = (ssize_t) code.GetAttrIntegerAs( L"index", -1 ) ;
		if ( iMsgIndex >= 0 )
		{
			TestReadMessage
				( thread.GetCurrentModuleFileTitle(), (size_t) iMsgIndex ) ;
		}
		if ( !(m_maskKeepClickSkip & (1 << syncTypeMessage)) )
		{
			ClearPushedKeyState( keyClickNext ) ;
		}
	}
	return	WaitClickMessage( thread, code, pxmlStorage ) ;
}

IMPL_ANTIRRHINUM_PROC(AGLMessageProcessor,msg_sync)
{
	SXMLDocument *	pxmlStorage = thread.GetLocalStrageAs( code.GetTag() ) ;
	if ( pxmlStorage->GetAttributeAs( L"tick_start" ) == nullptr )
	{
		pxmlStorage->SetAttrIntegerAs( L"tick_start", thread.GetThreadTick() ) ;
	}
	const int64_t	msecTimeout = code.GetAttrIntegerAs( L"timeout", -1 ) ;
	if ( msecTimeout > 0 )
	{
		if ( thread.GetThreadTick() >=
			pxmlStorage->GetAttrIntegerAs( L"tick_start" ) + msecTimeout )
		{
			thread.ClearLocalStrage( pxmlStorage ) ;
			return	codeProcessed ;
		}
	}
	UIMessage *	puiMsg = m_refMsgWindow ;
	if ( puiMsg != nullptr )
	{
		if ( ArePlayingAnyVoices()
			|| puiMsg->IsPendingMessage() )
		{
			return	codePending ;
		}
	}
	thread.ClearLocalStrage( pxmlStorage ) ;
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLMessageProcessor,msg)
{
	SXMLDocument *	pxmlStorage = thread.GetLocalStrageAs( code.GetTag() ) ;
	if ( pxmlStorage->GetAttributeAs( L"tick_start" ) == nullptr )
	{
		pxmlStorage->SetAttrIntegerAs( L"tick_start", thread.GetThreadTick() ) ;
		//
		StartMessageOutput( thread, code ) ;
		//
		bool	flagAsync = (code.GetAttrIntegerAs( L"async", 0 ) != 0) ;
		if ( flagAsync )
		{
			thread.ClearLocalStrage( pxmlStorage ) ;
			return	codeProcessed ;
		}
		if ( !(m_maskKeepClickSkip & (1 << syncTypeMessage)) )
		{
			ClearPushedKeyState( keyClickNext ) ;
		}
	}
	return	WaitClickMessage( thread, code, pxmlStorage ) ;
}

IMPL_ANTIRRHINUM_PROC(AGLMessageProcessor,msg_clear)
{
	DisplayName( L"" ) ;
	DisplayMessage( L"", true ) ;
	return	codeProcessed ;
}

static const SXMLDocument::AttrInteger	s_aiSkipSynchronismType[] =
{
	{ L"all", (1 << syncTypeTime)
				| (1 << syncTypeMessage)
				| (1 << syncTypeEffect)
				| (1 << syncTypeEvent) },
	{ L"time", 1 << syncTypeTime },
	{ L"message", 1 << syncTypeMessage },
	{ L"effect", 1 << syncTypeEffect },
	{ L"event", 1 << syncTypeEvent },
	{ L"0", 0 },
	{ L"no", 0 },
	{ nullptr, 0 },
} ;

static const SXMLDocument::AttrInteger	s_aiToggleButtonMask[] =
{
	{ L"all", (1 << AGLMessageProcessor::UIMessage::toggleAuto)
				| (1 << AGLMessageProcessor::UIMessage::toggleSkip)
				| (1 << AGLMessageProcessor::UIMessage::toggleFastSkip)
				| (1 << AGLMessageProcessor::UIMessage::toggleHWKeySkip) },
	{ L"auto", 1 << AGLMessageProcessor::UIMessage::toggleAuto },
	{ L"skip", 1 << AGLMessageProcessor::UIMessage::toggleSkip },
	{ L"fast_skip", 1 << AGLMessageProcessor::UIMessage::toggleFastSkip },
	{ L"key_skip", 1 << AGLMessageProcessor::UIMessage::toggleHWKeySkip },
	{ nullptr, 0 },
} ;

IMPL_ANTIRRHINUM_PROC(AGLMessageProcessor,enable_skip)
{
	uint32_t	nSysMask =
		(uint32_t) code.GetAttrComplexIntegerAs( L"sys", s_aiSkipSynchronismType, 0 ) ;
	uint32_t	nClickMask =
		(uint32_t) code.GetAttrComplexIntegerAs( L"click", s_aiSkipSynchronismType, 0 ) ;
	uint32_t	nButtonMask =
		(uint32_t) code.GetAttrComplexIntegerAs( L"button", s_aiToggleButtonMask, 0 ) ;
	//
	m_maskDisableSkip &= ~nSysMask ;
	m_maskDisableClickSkip &= ~nClickMask ;
	m_maskDisableButton &= ~nButtonMask ;
	//
	ApplyDisableToggleButtonFlags() ;
	//
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLMessageProcessor,disable_skip)
{
	uint32_t	nSysMask =
		(uint32_t) code.GetAttrComplexIntegerAs( L"sys", s_aiSkipSynchronismType, 0 ) ;
	uint32_t	nClickMask =
		(uint32_t) code.GetAttrComplexIntegerAs( L"click", s_aiSkipSynchronismType, 0 ) ;
	uint32_t	nButtonMask =
		(uint32_t) code.GetAttrComplexIntegerAs( L"button", s_aiToggleButtonMask, 0 ) ;
	//
	m_maskDisableSkip |= nSysMask ;
	m_maskDisableClickSkip |= nClickMask ;
	m_maskDisableButton |= nButtonMask ;
	//
	ApplyDisableToggleButtonFlags() ;
	//
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLMessageProcessor,keep_skip)
{
	uint32_t	nSyncMask =
		(uint32_t) code.GetAttrComplexIntegerAs( L"flags", s_aiSkipSynchronismType, 0 ) ;
	m_maskKeepClickSkip = nSyncMask ;
	if ( nSyncMask == 0 )
	{
		ClearPushedKeyState( keyClickNext ) ;
	}
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLMessageProcessor,scene_skip)
{
	const SString *	pstrJumpLabel = code.GetAttributeAs( L"jump" ) ;
	m_flagSceneSkip = ((pstrJumpLabel != nullptr) && !pstrJumpLabel->IsEmpty()) ;
	if ( m_flagSceneSkip )
	{
		m_strJumpSceneSkip = *pstrJumpLabel ;
		m_idSceneSkipThread = thread.GetThreadID() ;
	}
	else
	{
		m_strJumpSceneSkip = L"" ;
	}
	UIMessage *	puiMsg = m_refMsgWindow ;
	if ( puiMsg != nullptr )
	{
		puiMsg->SetToggleButton( UIMessage::toggleSceneSkip, false ) ;
		puiMsg->EnableToggleButton( UIMessage::toggleSceneSkip, m_flagSceneSkip ) ;
	}
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLMessageProcessor,begin_selector)
{
	m_aMenuItems.RemoveAll() ;
	//
	m_strSelResultTarget = code.GetAttrStringAs( L"result_target" ) ;
	m_nSelectionTimeout = (int32_t) code.GetAttrIntegerAs( L"timeout", 0 ) ;
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLMessageProcessor,add_menu_item)
{
	const SString *	pstrCond = code.GetAttributeAs( L"cond" ) ;
	if ( (pstrCond != nullptr)
		&& !EvaluateBoolExpression( thread, code, *pstrCond, true ) )
	{
		return	codeProcessed ;
	}
	MenuItem *	pMenu = new MenuItem ;
	pMenu->m_strText = EvaluateExprInText( thread, code.GetAttrStringAs( L"text" ) ) ;
	pMenu->m_strJump = EvaluateExprInText( thread, code.GetAttrStringAs( L"jump" ) ) ;
	pMenu->m_nValue = (int) code.GetAttrIntegerAs( L"value", m_aMenuItems.GetLength() ) ;
	pMenu->m_iMsgIndex = (size_t) code.GetAttrIntegerAs( L"msg_index" ) ;
	pMenu->m_flagHistory =
		IsReadMessage( thread.GetCurrentModuleFileTitle(), pMenu->m_iMsgIndex ) ;
	pMenu->m_xmlMenuCmd = code ;
	//
	m_aMenuItems.Add( pMenu ) ;
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLMessageProcessor,end_selector)
{
	if ( m_aMenuItems.GetLength() == 0 )
	{
		m_flagShowSelector = false ;
		return	codeProcessed ;
	}
	SXMLDocument *	pxmlStorage = thread.GetLocalStrageAs( code.GetTag() ) ;
	if ( pxmlStorage->GetAttributeAs( L"tick_start" ) == nullptr )
	{
		pxmlStorage->SetAttrIntegerAs( L"tick_start", thread.GetThreadTick() ) ;
		pxmlStorage->SetAttributeAs( L"state", L"start" ) ;
		//
		BuildSelector() ;
	}
	UISelector *	puiSel = m_refSelector ;
	if ( puiSel == nullptr )
	{
		thread.ClearLocalStrage( pxmlStorage ) ;
		m_flagShowSelector = false ;
		return	codeProcessed ;
	}
	SString	strState = pxmlStorage->GetAttrStringAs( L"state" ) ;
	if ( strState == L"start" )
	{
		if ( m_nSelectionTimeout > 0 )
		{
			int64_t	msecPast = thread.GetThreadTick()
								- pxmlStorage->GetAttrIntegerAs( L"tick_start" ) ;
			if ( msecPast > m_nSelectionTimeout )
			{
				msecPast = m_nSelectionTimeout ;
				pxmlStorage->SetAttributeAs( L"state", L"timeout" ) ;
			}
			puiSel->DisplayTimeout
				( (uint32_t) msecPast, (uint32_t) m_nSelectionTimeout ) ;
			//
			if ( msecPast >= m_nSelectionTimeout )
			{
				puiSel->EndSelection() ;
				return	codePending ;
			}
		}
		if ( puiSel->IsPendingShowEffect() )
		{
			return	codePending ;
		}
		int	nSelected = 0 ;
		if ( puiSel->IsUserSeleced( nSelected ) )
		{
			puiSel->EndSelection() ;
			pxmlStorage->SetAttributeAs( L"state", L"selected" ) ;
			pxmlStorage->SetAttrIntegerAs( L"selected", nSelected ) ;
		}
		return	codePending ;
	}
	else
	{
		if ( puiSel->IsPendingShowEffect() )
		{
			return	codePending ;
		}
		CodeProcessResult	cpr = codeProcessed ;
		if ( strState == L"selected" )
		{
			MenuItem *	pSelMenu = nullptr ;
			int	nSelected = (int) pxmlStorage->GetAttrIntegerAs( L"selected" ) ;
			for ( size_t i = 0; i < m_aMenuItems.GetLength(); i ++ )
			{
				MenuItem *	pMenu = m_aMenuItems.GetAt( i ) ;
				if ( (pMenu != nullptr) && (pMenu->m_nValue == nSelected) )
				{
					pSelMenu = pMenu ;
					break ;
				}
			}
			if ( pSelMenu != nullptr )
			{
				SetReadMessage( thread.GetCurrentModuleFileTitle(), pSelMenu->m_iMsgIndex ) ;
				//
				if ( !pSelMenu->m_strJump.IsEmpty() )
				{
					if ( !thread.JumpCodeLabel( pSelMenu->m_strJump, 0 ) )
					{
						cpr = codeControlled ;
					}
				}
			}
			if ( !m_strSelResultTarget.IsEmpty() )
			{
				AGLScriptObject	objResult =
						EvaluateExpression( thread, m_strSelResultTarget ) ;
				if ( !objResult.IsNull() )
				{
					objResult.PutInteger( nSelected ) ;
				}
			}
		}
		UIMessage *	puiMsg = m_refMsgWindow ;
		if ( puiMsg != nullptr )
		{
			puiMsg->ReleaseSelector( puiSel ) ;
		}
		m_aMenuItems.RemoveAll() ;
		m_refSelector = nullptr ;
		m_flagShowSelector = false ;
		thread.ClearLocalStrage( pxmlStorage ) ;
		return	cpr ;
	}
}

// メッセージウィンドウの表示状態設定
//////////////////////////////////////////////////////////////////////////////
void AGLMessageProcessor::ShowMessageWindow( bool flagShow, uint32_t msecTime )
{
	UIMessage *	puiMsg = m_refMsgWindow ;
	if ( puiMsg != nullptr )
	{
		puiMsg->ShowWindow( flagShow, msecTime ) ;
	}
	m_flagShowWindow = flagShow ;
}

// メッセージウィンドウのフェイス画像表示
//////////////////////////////////////////////////////////////////////////////
void AGLMessageProcessor::DisplayFace( const wchar_t * pwszFaceFile )
{
	m_strCurFace = pwszFaceFile ;
	//
	UIMessage *	puiMsg = m_refMsgWindow ;
	if ( puiMsg != nullptr )
	{
		if ( !m_strCurFace.IsEmpty() )
		{
			puiMsg->DisplayFace( m_strCurFace ) ;
		}
		else
		{
			puiMsg->ClearFace() ;
		}
		m_flagDelayClearFace = false ;
	}
}

// メッセージウィンドウの名前文字列設定
//////////////////////////////////////////////////////////////////////////////
void AGLMessageProcessor::DisplayName( const wchar_t * pwszName )
{
	m_strCurName = pwszName ;
	//
	UIMessage *	puiMsg = m_refMsgWindow ;
	if ( puiMsg != nullptr )
	{
		if ( !m_strCurName.IsEmpty() )
		{
			puiMsg->DisplayName( m_strCurName ) ;
		}
		else
		{
			puiMsg->ClearName() ;
		}
	}
}

// メッセージウィンドウのメッセージ文字列設定
//////////////////////////////////////////////////////////////////////////////
void AGLMessageProcessor::DisplayMessage( const wchar_t * pwszMessage, bool flagFinish )
{
	m_strCurMessage = pwszMessage ;
	//
	UIMessage *	puiMsg = m_refMsgWindow ;
	if ( puiMsg != nullptr )
	{
		puiMsg->ClearMessage() ;
		//
		if ( !m_strCurMessage.IsEmpty() )
		{
			puiMsg->StartMessage( m_strCurMessage ) ;
			//
			if ( flagFinish )
			{
				puiMsg->FinishMessage() ;
			}
		}
	}
}

// 現在のメッセージウィンドウの状態をクリア
//////////////////////////////////////////////////////////////////////////////
void AGLMessageProcessor::ClearCurrentMessageWindow( void )
{
	UIMessage *	puiMsg = m_refMsgWindow ;
	if ( puiMsg != nullptr )
	{
		puiMsg->ShowKeyWait( UIMessage::keyWaitHide ) ;
		puiMsg->ClearName() ;
		puiMsg->ClearMessage() ;
		puiMsg->ClearFace() ;
		m_strCurName = L"" ;
		m_strCurFace = L"" ;
		m_strCurMessage = L"" ;
		m_flagDelayClearFace = false ;
	}
}

// トグルボタンの有効・禁止状態反映
//////////////////////////////////////////////////////////////////////////////
void AGLMessageProcessor::ApplyDisableToggleButtonFlags( void )
{
	UIMessage *	puiMsg = m_refMsgWindow ;
	if ( puiMsg != nullptr )
	{
		static const UIMessage::ToggleButtonIndex	s_tbiButtons[] =
		{
			UIMessage::toggleAuto,
			UIMessage::toggleSkip,
			UIMessage::toggleFastSkip,
			UIMessage::toggleSceneSkip,
			UIMessage::toggleHWKeySkip,
		} ;
		const size_t	nCount = sizeof(s_tbiButtons) / sizeof(s_tbiButtons[0]) ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			UIMessage::ToggleButtonIndex	tbi = s_tbiButtons[i] ;
			puiMsg->EnableToggleButton
				( tbi, (m_maskDisableButton & (1 << tbi)) == 0 ) ;
		}
		puiMsg->EnableToggleButton
			( UIMessage::toggleSceneSkip, m_flagSceneSkip ) ;
	}
}

// トグルボタン状態取得
//////////////////////////////////////////////////////////////////////////////
bool AGLMessageProcessor::IsToggleButton( UIMessage::ToggleButtonIndex tbi ) const
{
	UIMessage *	puiMsg = m_refMsgWindow ;
	if ( puiMsg != nullptr )
	{
		return	puiMsg->IsToggleButton( tbi ) ;
	}
	return	false ;
}

// スキップモードか？（SKIP | FAST SKIP | SKIP キー押下）
//////////////////////////////////////////////////////////////////////////////
bool AGLMessageProcessor::IsSkipMode( void ) const
{
	UIMessage *	puiMsg = m_refMsgWindow ;
	if ( puiMsg != nullptr )
	{
		static const UIMessage::ToggleButtonIndex	s_tbiSkipButtons[] =
		{
			UIMessage::toggleSkip,
			UIMessage::toggleFastSkip,
			UIMessage::toggleHWKeySkip,
		} ;
		const size_t	nCount = sizeof(s_tbiSkipButtons) / sizeof(s_tbiSkipButtons[0]) ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			UIMessage::ToggleButtonIndex	tbi = s_tbiSkipButtons[i] ;
			if ( !(m_maskDisableButton & (1 << tbi))
							&& puiMsg->IsToggleButton( tbi ) )
			{
				return	true ;
			}
		}
	}
	return	false ;
}

// オートモードーか？
//////////////////////////////////////////////////////////////////////////////
bool AGLMessageProcessor::IsAutoMode( void ) const
{
	return	IsToggleButton( UIMessage::toggleAuto ) ;
}

// 既読フラグ取得
//////////////////////////////////////////////////////////////////////////////
bool AGLMessageProcessor::IsReadMessage( const wchar_t * pwszScript, size_t iMsgIndex )
{
	AGLVariablesProcessor *	paglVars =
			m_pKernel->GetEpicProcessor<AGLVariablesProcessor>() ;
	if ( paglVars != nullptr )
	{
		return	paglVars->GetReadFlag( pwszScript, iMsgIndex ) ;
	}
	return	false ;
}

// 既読フラグ設定
//////////////////////////////////////////////////////////////////////////////
void AGLMessageProcessor::SetReadMessage( const wchar_t * pwszScript, size_t iMsgIndex )
{
	AGLVariablesProcessor *	paglVars =
			m_pKernel->GetEpicProcessor<AGLVariablesProcessor>() ;
	if ( paglVars != nullptr )
	{
		paglVars->SetReadFlag( pwszScript, iMsgIndex ) ;
	}
}

// 既読フラグを取得し、対応する処理と既読フラグの設定を行う
//////////////////////////////////////////////////////////////////////////////
bool AGLMessageProcessor::TestReadMessage( const wchar_t * pwszScript, size_t iMsgIndex )
{
	AGLVariablesProcessor *	paglVars =
			m_pKernel->GetEpicProcessor<AGLVariablesProcessor>() ;
	if ( paglVars != nullptr )
	{
		if ( paglVars->GetReadFlag( pwszScript, iMsgIndex ) )
		{
			return	true ;
		}
		UIMessage *	puiMsg = m_refMsgWindow ;
		if ( puiMsg != nullptr )
		{
			if ( GetBehaviorFlags() & behaviorStopSkipNoRead )
			{
				puiMsg->SetToggleButton( UIMessage::toggleSkip, false ) ;
			}
			if ( GetBehaviorFlags() & behaviorStopFastSkipNoRead )
			{
				puiMsg->SetToggleButton( UIMessage::toggleFastSkip, false ) ;
			}
		}
		paglVars->SetReadFlag( pwszScript, (size_t) iMsgIndex ) ;
	}
	return	false ;
}

// メッセージ出力開始
//////////////////////////////////////////////////////////////////////////////
void AGLMessageProcessor::StartMessageOutput( AGLThread& thread, const AGLCode& code )
{
	const ssize_t	iMsgIndex = (ssize_t) code.GetAttrIntegerAs( L"index", -1 ) ;
	const ssize_t	iChar = (ssize_t) code.GetAttrIntegerAs( L"char_idx", -1 ) ;
	bool	flagRadMsg = false ;
	//
	if ( iMsgIndex >= 0 )
	{
		// 既読フラグ
		flagRadMsg =
			TestReadMessage
				( thread.GetCurrentModuleFileTitle(), (size_t) iMsgIndex ) ;
	}
	const CharConfig *	pcfg = GetCharIndexOf( iChar ) ;
	if ( pcfg == nullptr )
	{
		pcfg = GetCharIdOf( code.GetAttrStringAs( L"char_id" ) ) ;
	}
	//
	UIMessage *		puiMsg = m_refMsgWindow ;
	if ( puiMsg != nullptr )
	{
		bool	flagAddMsg = (code.GetAttrIntegerAs( L"add_msg", 0 ) != 0) ;
		if ( !flagAddMsg )
		{
			puiMsg->ClearMessage() ;
			m_strCurMessage = L"" ;
		}
		if ( !puiMsg->IsShowWindow() )
		{
			puiMsg->ShowWindow( true, m_pKernel->EffectTime( m_msecMsgWindowFade ) ) ;
			m_flagShowWindow = true ;
		}
		//
		// メッセージ出力準備
		//
		puiMsg->PrepareMessage( pcfg, flagRadMsg ) ;
		//
		// フェイス画像表示
		//
		const SString *	pstrFace = code.GetAttributeAs( L"face" ) ;
		if ( pstrFace != nullptr )
		{
			m_strCurFace = EvaluateExprInText( thread, *pstrFace ) ;
			puiMsg->DisplayFace( m_strCurFace ) ;
			m_flagDelayClearFace = false ;
		}
		//
		// 名前表示
		//
		SString			strDispName ;
		const SString *	pstrName = code.GetAttributeAs( _TX(L"\x1b[en]name_en\0name\0") ) ;
		if ( (pstrName != nullptr) && !pstrName->IsEmpty() )
		{
			strDispName = EvaluateExprInText( thread, *pstrName ) ;
		}
		else if ( pcfg != nullptr )
		{
			pstrName = pcfg->m_isoaName.GetAs( g_languageTarget ) ;
			if ( pstrName != nullptr )
			{
				strDispName = EvaluateExprInText( thread, *pstrName ) ;
			}
			else if ( !pcfg->m_strName.IsEmpty() )
			{
				strDispName = EvaluateExprInText( thread, pcfg->m_strName ) ;
			}
		}
		if ( !strDispName.IsEmpty() )
		{
			puiMsg->DisplayName( strDispName ) ;
			m_strCurName = strDispName ;
		}
		else
		{
			puiMsg->ClearName() ;
			m_strCurName = L"" ;
		}
		//
		// ボイス再生
		//
		SObjectArray<SString>	aVoices ;
		SObjectArray<Voice>		aPlayers ;
		const SString *	pstrVoices = code.GetAttributeAs( L"voices" ) ;
		if ( pstrVoices != nullptr )
		{
			SStringParser	sparsVoices ;
			sparsVoices.AttachString( *pstrVoices ) ;
			while ( !sparsVoices.IsIndexOverflow() )
			{
				SString	strVoiceFile =
					EvaluateExprInText( thread, sparsVoices.GetEnclosedString( L'|' ) ) ;
				aVoices.Add( new SString( strVoiceFile ) ) ;
				//
				if ( !IsSkipMode() )
				{
					Voice	voice = PlayVoice( strVoiceFile, playNoStart ) ;
					aPlayers.Add( new Voice( voice ) ) ;
				}
			}
			if ( aPlayers.GetLength() > 0 )
			{
				StopAllVoices( 0 ) ;
				for ( size_t i = 0; i < aPlayers.GetLength(); i ++ )
				{
					StartVoice( aPlayers.At(i) ) ;
				}
			}
		}
		//
		// メッセージ文字列
		//
		const SString *	pstrMsg = code.GetAttributeAs( _TX(L"\x1b[en]text_en\0text\0") ) ;
		if ( (pstrMsg == nullptr) || pstrMsg->IsEmpty() )
		{
			pstrMsg = code.GetAttributeAs( L"text" ) ;
		}
		SString	strMsgText ;
		if ( pstrMsg != nullptr )
		{
			strMsgText = EvaluateExprInText( thread, *pstrMsg ) ;
		}
		puiMsg->StartMessage( strMsgText ) ;
		m_strCurMessage += strMsgText ;
		//
		// ログ追加
		//
		MessageLog *	pLog = new MessageLog ;
		pLog->m_pCharCfg = pcfg ;
		pLog->m_strFace = m_strCurFace ;
		pLog->m_strName = strDispName ;
		pLog->m_strMessage = strMsgText ;
		pLog->m_aVoices = aVoices ;
		//
		const uint32_t	flagsBehavior = GetBehaviorFlags() ;
		if ( (flagsBehavior & behaviorSaveInLog)
			&& (!(flagsBehavior & behaviorNoSaveInSkip)
					|| !puiMsg->IsToggleButton( UIMessage::toggleSkip ))
			&& (!(flagsBehavior & behaviorNoSaveInFastSkip)
					|| !puiMsg->IsToggleButton( UIMessage::toggleFastSkip )) )
		{
			pLog->m_pxmlSaved = new SXMLDocument ;
			pLog->m_pxmlSaved->SetTag( L"save" ) ;
			m_pKernel->Serialize( *(pLog->m_pxmlSaved) ) ;
		}
		m_csSync.Lock() ;
		m_aMsgLog.Add( pLog ) ;
		if ( m_aMsgLog.GetLength() >= m_nMsgLogLimit )
		{
			m_aMsgLog.RemoveAt(0) ;
		}
		m_csSync.Unlock() ;
	}
}

// メッセージクリック待ち
//////////////////////////////////////////////////////////////////////////////
AntirrhinumGL::CodeProcessResult
	AGLMessageProcessor::WaitClickMessage
		( AGLThread& thread, const AGLCode& code, SXMLDocument * pxmlStorage )
{
	int64_t	msecTimeout = code.GetAttrIntegerAs( L"timeout", -1 ) ;
	int64_t	msecFadeout = code.GetAttrIntegerAs( L"fadeout", 0 ) ;
	bool	flagKeepMsg = (code.GetAttrIntegerAs( L"keep_msg", 0 ) != 0) ;
	bool	flagKeepVoice = (code.GetAttrIntegerAs( L"keep_voice", 0 ) != 0) ;
	bool	flagRadMsg = false ;
	//
	bool		flagPending = true ;
	UIMessage *	puiMsg = m_refMsgWindow ;
	if ( pxmlStorage->GetAttrIntegerAs( L"clicked", 0 ) == 0 )
	{
		bool	flagClicked = false ;
		if ( msecTimeout > 0 )
		{
			if ( thread.GetThreadTick() >=
				pxmlStorage->GetAttrIntegerAs( L"tick_start" ) + msecTimeout )
			{
				flagClicked = true ;
			}
		}
		if ( (puiMsg != nullptr) && puiMsg->IsToggleButton( UIMessage::toggleAuto ) )
		{
			if ( !puiMsg->IsPendingMessage() )
			{
				if ( pxmlStorage->GetAttributeAs( L"tick_done_msg" ) == nullptr )
				{
					pxmlStorage->SetAttrIntegerAs( L"tick_done_msg", thread.GetThreadTick() ) ;
				}
				if ( (thread.GetThreadTick() >=
						pxmlStorage->GetAttrIntegerAs( L"tick_start" )
									+ puiMsg->GetCurrentAutoTimeout())
					&& (thread.GetThreadTick() >=
						pxmlStorage->GetAttrIntegerAs( L"tick_done_msg" ) + 1000)
					&& !ArePlayingAnyVoices() )
				{
					flagClicked = true ;
				}
			}
		}
		if ( thread.IsPermittedSkip( syncTypeMessage )
			&& m_pKernel->ShouldAbortSync( syncTypeMessage ) )
		{
			m_pKernel->NotifyAbortedSync( syncTypeMessage ) ;
			flagClicked = true ;
		}
		m_flagWaitingMsgClick |= (puiMsg != nullptr) && !puiMsg->IsPendingMessage() ;
		//
		if ( flagClicked )
		{
			if ( (puiMsg != nullptr) && puiMsg->IsPendingMessage() )
			{
				puiMsg->FinishMessage() ;
			}
			else
			{
				pxmlStorage->SetAttrIntegerAs( L"clicked", 1 ) ;
				if ( !IsSkipMode() && (puiMsg != nullptr)
							&& !flagKeepMsg && (msecFadeout > 0) )
				{
					puiMsg->ClearMessage( m_pKernel->EffectTime( (uint32_t) msecFadeout ) ) ;
				}
				else
				{
					if ( (puiMsg != nullptr) && !flagKeepMsg )
					{
						puiMsg->ClearName() ;
						puiMsg->ClearMessage( 0 ) ;
						m_strCurFace = L"" ;
						m_strCurName = L"" ;
						m_strCurMessage = L"" ;
						m_flagDelayClearFace = true ;
					}
					if ( !flagKeepVoice )
					{
						StopAllVoices( IsSkipMode() ? 0 : m_pKernel->EffectTime( 300 ) ) ;
					}
					flagPending = false ;
				}
			}
		}
	}
	else if ( msecFadeout > 0 )
	{
		m_flagWaitingMsgClick = false ;
		if ( (puiMsg != nullptr) && !puiMsg->IsPendingClearMessage() )
		{
			puiMsg->ClearName() ;
			m_strCurFace = L"" ;
			m_strCurName = L"" ;
			m_strCurMessage = L"" ;
			m_flagDelayClearFace = true ;
			flagPending = false ;
		}
		if ( thread.IsPermittedSkip( syncTypeMessage )
			&& m_pKernel->ShouldAbortSync( syncTypeMessage ) )
		{
			m_pKernel->NotifyAbortedSync( syncTypeMessage ) ;
			//
			if ( puiMsg != nullptr )
			{
				puiMsg->ClearName() ;
				puiMsg->ClearMessage( 0 ) ;
			}
			m_strCurFace = L"" ;
			m_strCurName = L"" ;
			m_strCurMessage = L"" ;
			m_flagDelayClearFace = true ;
			flagPending = false ;
		}
	}
	if ( flagPending )
	{
		return	codePending ;
	}
	m_flagWaitingMsgClick = false ;
	thread.ClearLocalStrage( pxmlStorage ) ;
	return	codeProcessed ;
}

// 選択肢表示構築
//////////////////////////////////////////////////////////////////////////////
void AGLMessageProcessor::BuildSelector( void )
{
	UIMessage *	puiMsg = m_refMsgWindow ;
	if ( puiMsg == nullptr )
	{
		return ;
	}
	UISelector *	puiSel = puiMsg->GetSelector() ;
	m_refSelector = puiSel ;
	if ( puiSel == nullptr )
	{
		return ;
	}
	puiSel->ResetSelector() ;
	for ( size_t i = 0; i < m_aMenuItems.GetLength(); i ++ )
	{
		MenuItem *	pMenu = m_aMenuItems.GetAt(i) ;
		if ( pMenu != nullptr )
		{
			puiSel->AddSelectorItem
				( pMenu->m_strText, pMenu->m_nValue,
					pMenu->m_flagHistory, pMenu->m_xmlMenuCmd ) ;
		}
	}
	puiSel->StartSelection() ;
	m_flagShowSelector = true ;
}




//////////////////////////////////////////////////////////////////////////////
// AGLStdMessageProcessor 選択肢設定
//////////////////////////////////////////////////////////////////////////////

// 構築
AGLStdMessageProcessor::StdSelectorConfig::StdSelectorConfig( void )
	: m_pSkin( nullptr ), m_pScreen( nullptr ), m_nPriority( 0 ),
		m_ptBaseOffset( 0, 0 ), m_ptOffsetByCount( 0, 0 ),
		m_ptOffsetByIndex( 0, 0 ),
		m_msecItemFadeTime( 300 ), m_msecItemFadeDelay( 100 ),
		m_ptOffsetFadein( 0, 0 ), m_ptOffsetFadeout( 0, 0 )
{
}

// 全設定
void AGLStdMessageProcessor::StdSelectorConfig::SetSelectorConfig( const StdSelectorConfig& cfg )
{
	m_pSkin = cfg.m_pSkin ;
	m_pScreen = cfg.m_pScreen ;
	m_nPriority = cfg.m_nPriority ;
	m_strFormID = cfg.m_strFormID ;
	m_strHistoryFormID = cfg.m_strHistoryFormID ;
	m_strTextID = cfg.m_strTextID ;
	m_strButtonID = cfg.m_strButtonID ;
	m_ptBaseOffset = cfg.m_ptBaseOffset ;
	m_ptOffsetByCount = cfg.m_ptOffsetByCount ;
	m_ptOffsetByIndex = cfg.m_ptOffsetByIndex ;
	m_msecItemFadeTime = cfg.m_msecItemFadeTime ;
	m_msecItemFadeDelay = cfg.m_msecItemFadeDelay ;
	m_ptOffsetFadein = cfg.m_ptOffsetFadein ;
	m_ptOffsetFadeout = cfg.m_ptOffsetFadeout ;
}

// SGLSkinManager 設定
void AGLStdMessageProcessor::StdSelectorConfig::AttachSkinManager( SGLSkinManager * pSkin )
{
	m_pSkin = pSkin ;
}

// 表示先設定
void AGLStdMessageProcessor::StdSelectorConfig::AttachScreen( SGLSprite * pScreen, int32_t nPriority )
{
	m_pScreen = pScreen ;
	m_nPriority = nPriority ;
}

// フォーム設定
void AGLStdMessageProcessor::StdSelectorConfig::SetFormConfig
	( const wchar_t * pwszFormID,
		const wchar_t * pwszHisFormID,
		const wchar_t * pwszTextID,
		const wchar_t * pwszButtonID,
		const SGLPoint& ptBaseOffset,
		const SGLPoint& ptOffsetByCount,
		const SGLPoint& ptOffsetByIndex )
{
	m_strFormID = pwszFormID ;
	m_strHistoryFormID = (pwszHisFormID != nullptr) ? pwszHisFormID : pwszFormID ;
	m_strTextID = pwszTextID ;
	m_strButtonID = pwszButtonID ;
	m_ptBaseOffset = ptBaseOffset ;
	m_ptOffsetByCount = ptOffsetByCount ;
	m_ptOffsetByIndex = ptOffsetByIndex ;
}

// 表示時間設定
void AGLStdMessageProcessor::StdSelectorConfig::SetFadeTime
	( uint32_t msecFadeTime, uint32_t msecFadeDelay )
{
	m_msecItemFadeTime = msecFadeTime ;
	m_msecItemFadeDelay = msecFadeDelay ;
}

// フェードイン・アニメーション
void AGLStdMessageProcessor::StdSelectorConfig::SetFadeOffset
	( const SGLPoint& ptOffsetFadein, const SGLPoint& ptOffsetFadeout )
{
	m_ptOffsetFadein = ptOffsetFadein ;
	m_ptOffsetFadeout = ptOffsetFadeout ;
}


//////////////////////////////////////////////////////////////////////////////
// AGLStdMessageProcessor 選択肢実装
//////////////////////////////////////////////////////////////////////////////

// クラス情報
ESL_IMPLEMENT_CLASS_INFO( AntirrhinumGL::AGLStdMessageProcessor::StdSelector, UISelector )

// 構築
AGLStdMessageProcessor::StdSelector::StdSelector( void )
	: m_status( statusDone ), m_iFadeStarted( 0 ),
		m_flagSelected( false ), m_iSeleced( 0 )
{
}

// アニメーション効果
SGLPoint AGLStdMessageProcessor::StdSelector::GetAnimationOffset( size_t iMenu, bool flagFadeout )
{
	return	flagFadeout ? m_ptOffsetFadeout : m_ptOffsetFadein ;
}

// 選択肢初期化
void AGLStdMessageProcessor::StdSelector::ResetSelector( void )
{
	m_aMenuItems.RemoveAll() ;
	m_aMenuPoints.RemoveAll() ;
	m_nMenuValues.RemoveAll() ;
	m_status = statusDone ;
	m_iFadeStarted = 0 ;
	m_flagSelected = false ;
	m_iSeleced = 0 ;
	m_timer.Reset() ;
}

// 選択肢項目追加
void AGLStdMessageProcessor::StdSelector::AddSelectorItem
	( const wchar_t * pwszText,
		int nValue, bool flagHistorySelected,
		const SSystem::SXMLDocument& xmlOptions )
{
	if ( m_pSkin != nullptr )
	{
		SGLSprite *	pForm =
			m_pSkin->CreateFormedSprite
				( flagHistorySelected ? m_strHistoryFormID : m_strFormID ) ;
		if ( pForm != nullptr )
		{
			ESLAssert( m_aMenuItems.GetLength() == m_aMenuPoints.GetLength() ) ;
			ESLAssert( m_aMenuItems.GetLength() == m_nMenuValues.GetLength() ) ;
			pForm->SetSpriteText( m_strTextID, pwszText ) ;
			m_aMenuItems.Add( pForm ) ;
			m_aMenuPoints.Add( pForm->GetPosition2D() ) ;
			m_nMenuValues.Add( nValue ) ;
		}
	}
}

// 表示開始
void AGLStdMessageProcessor::StdSelector::StartSelection( void )
{
	m_status = statusFadein ;
	m_iFadeStarted = 0 ;
	m_flagSelected = false ;
	m_timer.Reset() ;
	//
	OnAntirrhinumTimer() ;
}

// 表示中のタイマー処理（スクリプト駆動）
void AGLStdMessageProcessor::StdSelector::OnAntirrhinumTimer( void )
{
	if ( (m_status == statusFadein)
		|| (m_status == statusFadeout) )
	{
		uint32_t	msecPast = (uint32_t) m_timer.GetTime() ;
		while ( (m_iFadeStarted < m_aMenuItems.GetLength())
				&& (msecPast >= m_iFadeStarted * m_msecItemFadeDelay) )
		{
			SGLSprite *	pSprite = m_aMenuItems.GetAt( m_iFadeStarted ) ;
			ESLAssert( pSprite != nullptr ) ;
			if ( (m_pScreen != nullptr) && (pSprite != nullptr) )
			{
				S2DDVector	vMenuPos = m_aMenuPoints.At(m_iFadeStarted) ;
				SGLPoint	ptMenuOffset =
								m_ptBaseOffset
									+ m_ptOffsetByCount * (int) m_aMenuItems.GetLength()
									+ m_ptOffsetByIndex * (int) m_iFadeStarted ;
				vMenuPos.x += ptMenuOffset.x ;
				vMenuPos.y += ptMenuOffset.y ;
				//
				SGLPoint	ptAnimeOffset =
					GetAnimationOffset( m_iFadeStarted, (m_status == statusFadeout) ) ;
				//
				m_pScreen->LockTrace( __FILE__, __LINE__ ) ;
				pSprite->ChangePriority( m_nPriority ) ;
				pSprite->SetTransparency( 0x100 ) ;
				pSprite->SetPosition
					( vMenuPos.x + ptAnimeOffset.x, vMenuPos.y + ptAnimeOffset.y ) ;
				pSprite->SetVisible( true ) ;
				pSprite->SetActionLinearTo
					( m_msecItemFadeTime, 0, &vMenuPos, nullptr,
						(m_status == statusFadein) ? 2.0 : 0.0,
						(m_status == statusFadein) ? 0.0 : 2.0 ) ;
				m_pScreen->AddChild( pSprite ) ;
				m_pScreen->Unlock() ;
			}
			m_iFadeStarted ++ ;
		}
		if ( m_iFadeStarted >= m_aMenuItems.GetLength() )
		{
			if ( m_status == statusFadein )
			{
				m_status = statusSelecting ;
			}
			else
			{
				if ( m_pScreen != nullptr )
				{
					m_pScreen->LockTrace( __FILE__, __LINE__ ) ;
					for ( size_t i = 0; i < m_aMenuItems.GetLength(); i ++ )
					{
						SGLSprite *	pSprite = m_aMenuItems.GetAt( i ) ;
						ESLAssert( pSprite != nullptr ) ;
						pSprite->DetachChild( pSprite ) ;
					}
					m_pScreen->Unlock() ;
				}
				m_status = statusDone ;
			}
		}
	}
}

// 表示終了
void AGLStdMessageProcessor::StdSelector::EndSelection( void )
{
	m_status = statusFadeout ;
	m_iFadeStarted = 0 ;
	m_timer.Reset() ;
	//
	if ( m_pScreen != nullptr )
	{
		m_pScreen->LockTrace( __FILE__, __LINE__ ) ;
		for ( size_t i = 0; i < m_aMenuItems.GetLength(); i ++ )
		{
			SGLSprite *	pSprite = m_aMenuItems.GetAt( i ) ;
			ESLAssert( pSprite != nullptr ) ;
			pSprite->SetSpriteEnable( m_strButtonID, false ) ;
		}
		m_pScreen->Unlock() ;
	}
	//
	OnAntirrhinumTimer() ;
}

// 表示効果（フェードイン・フェードアウト・スクロール等）中か？
bool AGLStdMessageProcessor::StdSelector::IsPendingShowEffect( void )
{
	if ( (m_status == statusFadein)
		|| (m_status == statusFadeout) )
	{
		return	true ;
	}
	for ( size_t i = 0; i < m_aMenuItems.GetLength(); i ++ )
	{
		SGLSprite *	pSprite = m_aMenuItems.GetAt( i ) ;
		ESLAssert( pSprite != nullptr ) ;
		if ( pSprite->IsAction() )
		{
			return	true ;
		}
	}
	return	false ;
}

// 時間制限の表示更新
void AGLStdMessageProcessor::StdSelector::DisplayTimeout( uint32_t msecPast, uint32_t msecTotal )
{
}

// ユーザー選択されたか？
bool AGLStdMessageProcessor::StdSelector::IsUserSeleced( int& nSelected )
{
	if ( !m_flagSelected )
	{
		for ( size_t i = 0; i < m_aMenuItems.GetLength(); i ++ )
		{
			SGLSprite *	pSprite = m_aMenuItems.GetAt( i ) ;
			ESLAssert( pSprite != nullptr ) ;
			if ( pSprite->IsSpriteButtonChecked( m_strButtonID ) )
			{
				m_flagSelected = true ;
				m_iSeleced = i ;
				break ;
			}
		}
	}
	if ( m_flagSelected )
	{
		if ( m_iSeleced < m_nMenuValues.GetLength() )
		{
			nSelected = m_nMenuValues.At(m_iSeleced) ;
		}
		else
		{
			nSelected = -1 ;
		}
		return	true ;
	}
	return	false ;
}


//////////////////////////////////////////////////////////////////////////////
// AGLStdMessageProcessor メッセージウィンドウ設定
//////////////////////////////////////////////////////////////////////////////

// 構築
AGLStdMessageProcessor::StdMessageConfig::StdMessageConfig( void )
	: m_ptFaceOffset( 0, 0 ),
		m_nFacePriority( 0 ),
		m_msecFaceFadeTime( 300 ),
		m_nAutoSpeed( 0x80 ),
		m_flagMsgWndPos( false )
{
}

// 全設定
void AGLStdMessageProcessor::StdMessageConfig::SetMessageConfig( const StdMessageConfig& cfg )
{
	for ( int i = 0; i < msgElementCount; i ++ )
	{
		m_refElement[i] = cfg.m_refElement[i] ;
	}
	m_nFacePriority = cfg.m_nFacePriority ;
	m_ptFaceOffset = cfg.m_ptFaceOffset ;
	m_msecFaceFadeTime = cfg.m_msecFaceFadeTime ;
	m_nAutoSpeed = cfg.m_nAutoSpeed ;
	m_flagMsgWndPos = cfg.m_flagMsgWndPos ;
	m_posMsgWindow[0] = cfg.m_posMsgWindow[0] ;
	m_posMsgWindow[1] = cfg.m_posMsgWindow[1] ;
}

// ウィンドウ要素関連付け
void AGLStdMessageProcessor::StdMessageConfig::AttachSpriteElement
	( MessageSpriteElement mseIndex, SGLSprite * pSprite )
{
	m_refElement[mseIndex] = pSprite ;
}

void AGLStdMessageProcessor::StdMessageConfig::AttachFormItemElement
	( MessageSpriteElement mseIndex, SakuraGL::SGLBasicForm::Item * pItem )
{
	m_refElement[mseIndex] = pItem ;
}

// フェイス画像設定
void AGLStdMessageProcessor::StdMessageConfig::SetFaceConfig
	( const SGLPoint& ptOffset, int32_t nPriority, uint32_t msecFadeTime )
{
	m_ptFaceOffset = ptOffset ;
	m_nFacePriority = nPriority ;
	m_msecFaceFadeTime = msecFadeTime ;
}

// オートモード速度（待ち時間）
void AGLStdMessageProcessor::StdMessageConfig::SetAutoModeSpeed( uint32_t nAutoSpeed )
{
	m_nAutoSpeed = nAutoSpeed ;
}

// オートモード速度と文字数からタイムアウト時間計算
uint32_t AGLStdMessageProcessor::StdMessageConfig::GetAutoMessageTime( uint32_t nAutoSpeed, uint32_t nCharCount )
{
	return	((nCharCount * 100) + 500) * m_nAutoSpeed / 0x100 + 500 ;
}

// 非表示・表示位置を設定
void AGLStdMessageProcessor::StdMessageConfig::SetShowPosition( const Position& posHide, const Position& posShow )
{
	m_flagMsgWndPos = true ;
	m_posMsgWindow[0] = posHide ;
	m_posMsgWindow[1] = posShow ;
}



//////////////////////////////////////////////////////////////////////////////
// AGLStdMessageProcessor メッセージウィンドウ実装
//////////////////////////////////////////////////////////////////////////////

// クラス情報
ESL_IMPLEMENT_CLASS_INFO( AntirrhinumGL::AGLStdMessageProcessor::StdMessage, UIMessage )

// 構築
AGLStdMessageProcessor::StdMessage::StdMessage( AGLStdMessageProcessor * pStdMsg )
	: m_pStdMsg( pStdMsg ), m_flagShowWindow( false ),
		m_keyWaitCurrent( keyWaitHide ),
		m_pcfgNextChar( nullptr ), m_flagReadMsg( false ),
		m_nLastAddedMsgChars( 0 ), m_flagFadeoutMsg( false )
{
}

// 表示状態取得
bool AGLStdMessageProcessor::StdMessage::IsElementVisible( MessageSpriteElement mseIndex ) const
{
	SGLSprite *	pSprite = m_refElement[mseIndex].GetRef<SGLSprite>() ;
	if ( pSprite != nullptr )
	{
		return	pSprite->IsVisible() ;
	}
	else
	{
		SGLBasicForm::Item *
			pItem = m_refElement[mseIndex].GetRef<SGLBasicForm::Item>() ;
		if ( pItem != nullptr )
		{
			return	pItem->IsVisible() ;
		}
	}
	return	false ;
}

// 表示状態設定
void AGLStdMessageProcessor::StdMessage::SetElementVisible
	( MessageSpriteElement mseIndex, bool flagVisible )
{
	SGLSprite *	pSprite = m_refElement[mseIndex].GetRef<SGLSprite>() ;
	if ( pSprite != nullptr )
	{
		pSprite->SetVisible( flagVisible ) ;
	}
	else
	{
		SGLBasicForm::Item *
			pItem = m_refElement[mseIndex].GetRef<SGLBasicForm::Item>() ;
		if ( pItem != nullptr )
		{
			pItem->SetVisible( flagVisible ) ;
		}
	}
}

// 移動アニメーション制御
bool AGLStdMessageProcessor::StdMessage::MoveElementPosition
	( MessageSpriteElement mseIndex, const Position& pos, uint32_t msecTime )
{
	SGLSprite *	pSprite = m_refElement[mseIndex].GetRef<SGLSprite>() ;
	if ( pSprite != nullptr )
	{
		if ( msecTime != 0 )
		{
			pSprite->SetActionLinearTo
				( msecTime, pos.nTransparency,
					&(pos.vPosition), &(pos.vZoom), 0.0, 2.0 ) ;
		}
		else
		{
			pSprite->SetPosition( pos.vPosition.x, pos.vPosition.y ) ;
			pSprite->SetZoom( pos.vZoom.x, pos.vZoom.y ) ;
			pSprite->SetTransparency( pos.nTransparency ) ;
		}
		return	true ;
	}
	else
	{
		SGLBasicForm::Item *
			pItem = m_refElement[mseIndex].GetRef<SGLBasicForm::Item>() ;
		if ( pItem != nullptr )
		{
			SGLBasicForm *	pForm = pItem->GetParentForm() ;
			if ( pForm != nullptr )
			{
				S2DVector	vMove = pos.vPosition ;
				S2DVector	vZoom = pos.vZoom ;
				if ( msecTime != 0 )
				{
					pForm->AddItemMoveAnimation
						( pItem, msecTime, vMove,
							pos.nTransparency, &vZoom, nullptr, 0.0f, 2.0f ) ;
				}
				else
				{
					pItem->SetPosition( vMove ) ;
					pItem->SetZoom( vZoom ) ;
					pItem->SetTransparency( pos.nTransparency ) ;
				}
				return	true ;
			}
		}
	}
	return	false ;
}

bool AGLStdMessageProcessor::StdMessage::FadeElementTransparency
	( MessageSpriteElement mseIndex, uint32_t nTransparency, uint32_t msecTime )
{
	SGLSprite *	pSprite = m_refElement[mseIndex].GetRef<SGLSprite>() ;
	if ( pSprite != nullptr )
	{
		if ( msecTime != 0 )
		{
			pSprite->SetActionLinearTo
				( msecTime, nTransparency,nullptr, nullptr, 1.0, 1.0 ) ;
		}
		else
		{
			pSprite->SetTransparency( nTransparency ) ;
		}
		return	true ;
	}
	else
	{
		SGLBasicForm::Item *
			pItem = m_refElement[mseIndex].GetRef<SGLBasicForm::Item>() ;
		if ( pItem != nullptr )
		{
			SGLBasicForm *	pForm = pItem->GetParentForm() ;
			if ( pForm != nullptr )
			{
				if ( msecTime != 0 )
				{
					pForm->AddItemAnimation
						( pItem, msecTime, nullptr,
							&nTransparency, nullptr, nullptr, 0.0f, 2.0f ) ;
				}
				else
				{
					pItem->SetTransparency( nTransparency ) ;
				}
				return	true ;
			}
		}
	}
	return	false ;
}

bool AGLStdMessageProcessor::StdMessage::IsMovingElementPosition( MessageSpriteElement mseIndex ) const
{
	SGLSprite *	pSprite = m_refElement[mseIndex].GetRef<SGLSprite>() ;
	if ( pSprite != nullptr )
	{
		return	pSprite->IsAction() ;
	}
	else
	{
		SGLBasicForm::Item *
			pItem = m_refElement[mseIndex].GetRef<SGLBasicForm::Item>() ;
		if ( pItem != nullptr )
		{
			SGLBasicForm *	pForm = pItem->GetParentForm() ;
			if ( pForm != nullptr )
			{
				return	(pForm->FindItemAnimation( pItem ) != nullptr) ;
			}
		}
	}
	return	false ;
}

bool AGLStdMessageProcessor::StdMessage::FinishMovingElementPosition( MessageSpriteElement mseIndex )
{
	SGLSprite *	pSprite = m_refElement[mseIndex].GetRef<SGLSprite>() ;
	if ( pSprite != nullptr )
	{
		pSprite->FlushAction() ;
		return	true ;
	}
	else
	{
		SGLBasicForm::Item *
			pItem = m_refElement[mseIndex].GetRef<SGLBasicForm::Item>() ;
		if ( pItem != nullptr )
		{
			SGLBasicForm *	pForm = pItem->GetParentForm() ;
			if ( pForm != nullptr )
			{
				pForm->FinishAnimation( pItem ) ;
				return	true ;
			}
		}
	}
	return	false ;
}

// 表示中のタイマー処理（スクリプト駆動）
void AGLStdMessageProcessor::StdMessage::OnAntirrhinumTimer( void )
{
	SGLSprite *	pWindow = m_refElement[msgElementWindow].GetRef<SGLSprite>() ;
	if ( pWindow != nullptr )
	{
		pWindow->LockTrace( __FILE__, __LINE__ ) ;
		if ( (m_pFaceFadeout != nullptr)
			&& m_pFaceFadeout->IsAction() )
		{
			pWindow->DetachChild( m_pFaceFadeout ) ;
			m_pFaceFadeout = nullptr ;
			m_strFaceFadeout = L"" ;
		}
		pWindow->Unlock() ;
	}
}

// ウィンドウの表示状態取得
bool AGLStdMessageProcessor::StdMessage::IsShowWindow( void )
{
	if ( m_flagShowWindow )
	{
		if ( IsElementVisible( msgElementWindow ) )
		{
			return	true ;
		}
		m_flagShowWindow = false ;
	}
	return	m_flagShowWindow ;
}

// ウィンドウの表示／非表示状態変更（フェード開始）
void AGLStdMessageProcessor::StdMessage::ShowWindow( bool flagShow, uint32_t msecTime )
{
	if ( m_flagShowWindow != flagShow )
	{
		FinishMovingElementPosition( msgElementWindow ) ;
		if ( m_flagMsgWndPos )
		{
			const Position&	pos = m_posMsgWindow[flagShow ? 1 : 0] ;
			MoveElementPosition( msgElementWindow, pos, msecTime ) ;
		}
		else
		{
			FadeElementTransparency
				( msgElementWindow, (flagShow ? 0 : 0x100), msecTime ) ;
		}
		m_flagShowWindow = flagShow ;
	}
	else if ( msecTime == 0 )
	{
		FinishMovingElementPosition( msgElementWindow ) ;
	}
}

// ウィンドウのフェード中か？
bool AGLStdMessageProcessor::StdMessage::IsPendingShowEffect( void )
{
	return	IsMovingElementPosition( msgElementWindow ) ;
}

// ウィンドウのフェード処理の即時完了
void AGLStdMessageProcessor::StdMessage::FinishShowWindow( void )
{
	FinishMovingElementPosition( msgElementWindow ) ;
}

// キャラクター毎に吹き出しを生成するような場合、吹き出しの準備
// また文字色などの設定の反映（nullptr の場合モノローグなどのデフォルト処理）
void AGLStdMessageProcessor::StdMessage::PrepareMessage( const CharConfig * pcfg, bool flagReadMsg )
{
	m_pcfgNextChar = pcfg ;
	m_flagReadMsg = flagReadMsg ;
}

// フェイス画像の表示
void AGLStdMessageProcessor::StdMessage::DisplayFace( const wchar_t * pwszFace )
{
	if ( m_strFaceFile == pwszFace )
	{
		return ;
	}
	SGLSprite *	pWindow = m_refElement[msgElementWindow].GetRef<SGLSprite>() ;
	if ( (m_pFaceFadeout != nullptr) && (m_strFaceFadeout == pwszFace) )
	{
		// フェードアウト中の画像を元に戻す
		if ( (pWindow != nullptr) && (m_pFaceSprite != nullptr) )
		{
			pWindow->DetachChild( m_pFaceSprite ) ;
		}
		m_pFaceFadeout->CancelAction() ;
		m_pFaceFadeout->SetTransparency( 0 ) ;
		m_pFaceFadeout->ChangePriority( m_nFacePriority ) ;
		//
		m_pFaceSprite = m_pFaceFadeout.Detach() ;
		m_strFaceFile = m_strFaceFadeout ;
		m_strFaceFadeout = L"" ;
		return ;
	}
	if ( (m_pFaceSprite != nullptr) && (m_pFaceFadeout != nullptr) )
	{
		// フェードアウト中の画像を即時消去する
		if ( pWindow != nullptr )
		{
			pWindow->DetachChild( m_pFaceFadeout ) ;
		}
		m_pFaceFadeout = nullptr ;
		m_strFaceFadeout = L"" ;
	}
	if ( m_pFaceSprite != nullptr )
	{
		// 現在のフェイス画像をフェードアウトする
		m_pFaceSprite->SetActionLinearTo
			( m_msecFaceFadeTime, 0x100, nullptr, nullptr, 1.0, 1.0 ) ;
		m_pFaceSprite->ChangePriority( m_nFacePriority + 1 ) ;
		//
		ESLAssert( m_pFaceFadeout == nullptr ) ;
		m_pFaceFadeout = m_pFaceSprite.Detach() ;
		m_strFaceFadeout = m_strFaceFile ;
		m_strFaceFile = L"" ;
	}
	if ( (pwszFace != nullptr) && (pwszFace[0] != 0) )
	{
		// 新しいフェイス画像を読み込む
		SString	strFaceFile = pwszFace ;
		m_strFaceFile = strFaceFile ;
		if ( SString(strFaceFile.GetFileExtensionPart()).IsEmpty() )
		{
			strFaceFile + L".eri" ;
		}
		ESLAssert( m_pFaceSprite == nullptr ) ;
		m_pFaceSprite = new SGLSprite ;
		if ( m_pFaceSprite->LoadImage( strFaceFile ) != sglErrFailed )
		{
			ESLTrace( "failed to StdMessage::DisplayFace %s\n", strFaceFile.ToCharArray().GetConstArray() ) ;
		}
		m_pFaceSprite->SetPosition( m_ptFaceOffset.x, m_ptFaceOffset.y ) ;
		m_pFaceSprite->ChangePriority( m_nFacePriority ) ;
		m_pFaceSprite->SetVisible( true ) ;
		//
		if ( pWindow != nullptr )
		{
			pWindow->Lock() ;
			pWindow->AddChild( m_pFaceSprite );
			pWindow->Unlock() ;
		}
	}
}

// 名前の表示
void AGLStdMessageProcessor::StdMessage::DisplayName( const wchar_t * pwszName )
{
	SetElementVisible( msgElementNameFrame, true ) ;

	SGLSprite *	pSpriteName = m_refElement[msgElementName].GetRef<SGLSprite>() ;
	if ( pSpriteName != nullptr )
	{
		pSpriteName->SetText( pwszName ) ;
		pSpriteName->SetVisible( true ) ;
	}
}

// メッセージ出力開始（追加出力）
void AGLStdMessageProcessor::StdMessage::StartMessage( const wchar_t * pwszMsg )
{
	SGLSpriteMessage *	pMessage =
			m_refElement[msgElementMessage].GetRef<SGLSpriteMessage>() ;
	if ( pMessage != nullptr )
	{
		if ( m_flagFadeoutMsg )
		{
			pMessage->FlushAction() ;
			pMessage->ClearMessage() ;
		}
		SGLSpriteMessage::LetteringContext	context ;
		size_t	nLastCharCount = pMessage->GetMessageCharacterCount() ;
		pMessage->SaveLetteringContext( context ) ;
		//
		if ( m_pcfgNextChar != nullptr )
		{
			pMessage->SetTextColor( m_pcfgNextChar->m_argbText ) ;
		}
		pMessage->AddMessageXML( pwszMsg ) ;
		pMessage->SetTransparency( 0 ) ;
		//
		pMessage->RestoreLetteringContext( context ) ;
		m_nLastAddedMsgChars = pMessage->GetMessageCharacterCount() - nLastCharCount ;
	}
}

// メッセージ出力処理中か？
bool AGLStdMessageProcessor::StdMessage::IsPendingMessage( void )
{
	SGLSpriteMessage *	pMessage =
			m_refElement[msgElementMessage].GetRef<SGLSpriteMessage>() ;
	if ( pMessage != nullptr )
	{
		return	pMessage->IsMessagePending() ;
	}
	return	false ;
}

// 現在のメッセージの AUTO タイムアウト時間計算
uint32_t AGLStdMessageProcessor::StdMessage::GetCurrentAutoTimeout( void )
{
	return	GetAutoMessageTime( m_nAutoSpeed, (uint32_t) m_nLastAddedMsgChars ) ;
}

// メッセージ出力の即時完了
void AGLStdMessageProcessor::StdMessage::FinishMessage( void )
{
	SGLSpriteMessage *	pMessage =
			m_refElement[msgElementMessage].GetRef<SGLSpriteMessage>() ;
	if ( pMessage != nullptr )
	{
		pMessage->FlushMessage() ;
	}
}

// 現在表示されているメッセージのプレーンテキスト取得
const wchar_t * AGLStdMessageProcessor::StdMessage::GetMessagePlainText( void )
{
	SGLSpriteMessage *	pMessage =
			m_refElement[msgElementMessage].GetRef<SGLSpriteMessage>() ;
	if ( pMessage != nullptr )
	{
		m_strMsgPlainText = pMessage->GetPlainText() ;
	}
	return	m_strMsgPlainText ;
}

// フェイス画像の消去
void AGLStdMessageProcessor::StdMessage::ClearFace( void )
{
	DisplayFace( nullptr ) ;
}

// 名前の消去
void AGLStdMessageProcessor::StdMessage::ClearName( void )
{
	SGLSprite *	pSpriteName = m_refElement[msgElementName].GetRef<SGLSprite>() ;
	if ( pSpriteName != nullptr )
	{
		pSpriteName->SetVisible( false ) ;
	}
	SGLSprite *	pSpriteFrame = m_refElement[msgElementNameFrame].GetRef<SGLSprite>() ;
	if ( pSpriteFrame != nullptr )
	{
		pSpriteFrame->SetVisible( false ) ;
	}
}

// メッセージの消去（フェードアウト）
void AGLStdMessageProcessor::StdMessage::ClearMessage( size_t nFadeout )
{
	SGLSpriteMessage *	pMessage =
			m_refElement[msgElementMessage].GetRef<SGLSpriteMessage>() ;
	if ( pMessage != nullptr )
	{
		if ( nFadeout == 0 )
		{
			pMessage->ClearMessage() ;
			m_flagFadeoutMsg = false ;
		}
		else
		{
			pMessage->FlushAction() ;
			pMessage->SetActionLinearTo
				( (uint32_t) nFadeout, 0x100, nullptr, nullptr, 1.0, 1.0 ) ;
			m_flagFadeoutMsg = true ;
		}
	}
}

// メッセージのフェードアウト中か？
bool AGLStdMessageProcessor::StdMessage::IsPendingClearMessage( void )
{
	SGLSpriteMessage *	pMessage =
			m_refElement[msgElementMessage].GetRef<SGLSpriteMessage>() ;
	if ( pMessage != nullptr )
	{
		if ( m_flagFadeoutMsg && pMessage->IsAction() )
		{
			return	true ;
		}
		if ( m_flagFadeoutMsg )
		{
			pMessage->ClearMessage() ;
			pMessage->SetTransparency( 0 ) ;
			m_flagFadeoutMsg = false ;
		}
	}
	return	false ;
}

// キー待ちアイコンの表示状態設定
void AGLStdMessageProcessor::StdMessage::ShowKeyWait( KeyWaitType type )
{
	if ( m_keyWaitCurrent != type )
	{
		SetElementVisible( msgElementIconAuto, (type == keyWaitInSkip) ) ;
		SetElementVisible( msgElementIconSkip, (type == keyWaitInAuto) ) ;
		SetElementVisible( msgElementIconClick, (type == keyWaitClick) ) ;
		m_keyWaitCurrent = type ;
	}
}

// 現在のキー待ちアイコンの表示状態設定
AGLMessageProcessor::UIMessage::KeyWaitType
	AGLStdMessageProcessor::StdMessage::CurrentKeyWait( void )
{
	return	m_keyWaitCurrent ;
}

// トグルUI状態取得
bool AGLStdMessageProcessor::StdMessage::IsToggleButton( ToggleButtonIndex tbi )
{
	return	m_pStdMsg->IsToggleButton( tbi ) ;
}

// トグルUI状態設定
void AGLStdMessageProcessor::StdMessage::SetToggleButton( ToggleButtonIndex tbi, bool flagPushed )
{
	m_pStdMsg->SetToggleButton( tbi, flagPushed ) ;
}

// トグルUI禁止状態設定
void AGLStdMessageProcessor::StdMessage::EnableToggleButton( ToggleButtonIndex tbi, bool flagEnabled )
{
	m_pStdMsg->EnableToggleButton( tbi, flagEnabled ) ;
}

// 選択肢取得
AGLStdMessageProcessor::UISelector * AGLStdMessageProcessor::StdMessage::GetSelector( void )
{
	if ( m_pSelector == nullptr )
	{
		m_pSelector = m_pStdMsg->NewSelector() ;
	}
	return	m_pSelector ;
}

// 選択肢解放
void AGLStdMessageProcessor::StdMessage::ReleaseSelector( UISelector * pSelector )
{
	if ( m_pSelector.Ptr() == pSelector )
	{
		m_pSelector = nullptr ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// AGLStdMessageProcessor::UIToggle
//////////////////////////////////////////////////////////////////////////////

// クラス情報
ESL_IMPLEMENT_CLASS_INFO( AntirrhinumGL::AGLStdMessageProcessor::UIToggle, ESLObject )



//////////////////////////////////////////////////////////////////////////////
// AGLStdMessageProcessor::UIToggleSprite
//////////////////////////////////////////////////////////////////////////////

// クラス情報
ESL_IMPLEMENT_CLASS_INFO( AntirrhinumGL::AGLStdMessageProcessor::UIToggleSprite, UIToggle )

// 構築
AGLStdMessageProcessor::UIToggleSprite::UIToggleSprite( void )
	: m_nDisabledTransparency( 0 )
{
}

AGLStdMessageProcessor::UIToggleSprite::UIToggleSprite( SakuraGL::SGLSprite * pSprite )
	: m_nDisabledTransparency( 0 )
{
	m_aSprites.Add( pSprite ) ;
}

// ボタン追加
void AGLStdMessageProcessor::UIToggleSprite::AddSprite( SakuraGL::SGLSprite * pSprite )
{
	m_aSprites.Add( pSprite ) ;
}

// トグルUI状態取得
bool AGLStdMessageProcessor::UIToggleSprite::IsToggle( void )
{
	for ( size_t i = 0; i < m_aSprites.GetLength(); i ++ )
	{
		SGLSprite *	pSprite = m_aSprites.GetAt( i ) ;
		if ( pSprite != nullptr )
		{
			if ( pSprite->IsButtonChecked() )
			{
				return	true ;
			}
		}
	}
	return	false ;
}

// トグルUI状態設定
void AGLStdMessageProcessor::UIToggleSprite::SetToggle( bool flagPushed )
{
	for ( size_t i = 0; i < m_aSprites.GetLength(); i ++ )
	{
		SGLSprite *	pSprite = m_aSprites.GetAt( i ) ;
		if ( pSprite != nullptr )
		{
			pSprite->CheckButton( flagPushed ) ;
		}
	}
}

// トグルUI禁止状態設定
void AGLStdMessageProcessor::UIToggleSprite::EnableToggle( bool flagEnabled )
{
	for ( size_t i = 0; i < m_aSprites.GetLength(); i ++ )
	{
		SGLSprite *	pSprite = m_aSprites.GetAt( i ) ;
		if ( pSprite != nullptr )
		{
			pSprite->SetEnable( flagEnabled ) ;
			//
			if ( m_nDisabledTransparency != 0 )
			{
				pSprite->SetTransparency
					( flagEnabled ? 0 : m_nDisabledTransparency ) ;
			}
		}
	}
}

// 禁止状態の表示透明度設定
void AGLStdMessageProcessor::UIToggleSprite::SetDisabledTransparency( uint32_t nTransparency )
{
	m_nDisabledTransparency = nTransparency ;
}


//////////////////////////////////////////////////////////////////////////////
// AGLStdMessageProcessor::UIToggleFormButton
//////////////////////////////////////////////////////////////////////////////

// クラス情報
ESL_IMPLEMENT_CLASS_INFO( AntirrhinumGL::AGLStdMessageProcessor::UIToggleFormButton, UIToggle )

// 構築
AGLStdMessageProcessor::UIToggleFormButton::UIToggleFormButton( void )
	: m_nDisabledTransparency( 0 )
{
}

AGLStdMessageProcessor::UIToggleFormButton::UIToggleFormButton( SakuraGL::SGLBasicForm::Button * pButton )
	: m_nDisabledTransparency( 0 )
{
	m_aButtons.Add( pButton ) ;
}

// ボタン追加
void AGLStdMessageProcessor::UIToggleFormButton::AddButton( SakuraGL::SGLBasicForm::Button * pButton )
{
	m_aButtons.Add( pButton ) ;
}

// トグルUI状態取得
bool AGLStdMessageProcessor::UIToggleFormButton::IsToggle( void )
{
	for ( size_t i = 0; i < m_aButtons.GetLength(); i ++ )
	{
		SGLBasicForm::Button *	pButton = m_aButtons.GetAt( i ) ;
		if ( pButton != nullptr )
		{
			if ( pButton->IsPushed() )
			{
				return	true ;
			}
		}
	}
	return	false ;
}

// トグルUI状態設定
void AGLStdMessageProcessor::UIToggleFormButton::SetToggle( bool flagPushed )
{
	LockTrace( __FILE__, __LINE__ ) ;
	for ( size_t i = 0; i < m_aButtons.GetLength(); i ++ )
	{
		SGLBasicForm::Button *	pButton = m_aButtons.GetAt( i ) ;
		if ( pButton != nullptr )
		{
			pButton->SetTogglePushed( flagPushed ) ;
		}
	}
	Unlock() ;
}

// トグルUI禁止状態設定
void AGLStdMessageProcessor::UIToggleFormButton::EnableToggle( bool flagEnabled )
{
	LockTrace( __FILE__, __LINE__ ) ;
	for ( size_t i = 0; i < m_aButtons.GetLength(); i ++ )
	{
		SGLBasicForm::Button *	pButton = m_aButtons.GetAt( i ) ;
		if ( pButton != nullptr )
		{
			pButton->SetDisable( !flagEnabled ) ;
			//
			if ( m_nDisabledTransparency != 0 )
			{
				pButton->SetTransparency
					( flagEnabled ? 0 : m_nDisabledTransparency ) ;
			}
		}
	}
	Unlock() ;
}

// 禁止状態の表示透明度設定
void AGLStdMessageProcessor::UIToggleFormButton::SetDisabledTransparency( uint32_t nTransparency )
{
	m_nDisabledTransparency = nTransparency ;
}


//////////////////////////////////////////////////////////////////////////////
// AGLStdMessageProcessor::UIToggleKey
//////////////////////////////////////////////////////////////////////////////

// クラス情報
ESL_IMPLEMENT_CLASS_INFO( AntirrhinumGL::AGLStdMessageProcessor::UIToggleKey, UIToggle )

// 構築
AGLStdMessageProcessor::UIToggleKey::UIToggleKey( void )
	: m_pInput( nullptr ), m_iButton( 0 ), m_iJoyStick( 0 ), m_flagEnabled( true )
{
}

AGLStdMessageProcessor::UIToggleKey::UIToggleKey
	( SakuraGL::SGLVirtualInput * pInput,
				size_t iButton, size_t iJoyStick )
	: m_pInput( pInput ), m_iButton( iButton ),
		m_iJoyStick( iJoyStick ), m_flagEnabled( true )
{
}

// キー設定
void AGLStdMessageProcessor::UIToggleKey::SetVirtualJouButton
	( SakuraGL::SGLVirtualInput * pInput,
				size_t iButton, size_t iJoyStick )
{
	m_pInput = pInput ;
	m_iButton = iButton ;
	m_iJoyStick = iJoyStick ;
}

// トグルUI状態取得
bool AGLStdMessageProcessor::UIToggleKey::IsToggle( void )
{
	if ( m_flagEnabled && (m_pInput != nullptr) )
	{
		return	(m_pInput->GetJoyButtonPushed( m_iButton, m_iJoyStick ) > 0) ;
	}
	return	false ;
}

// トグルUI状態設定
void AGLStdMessageProcessor::UIToggleKey::SetToggle( bool flagPushed )
{
	if ( m_pInput != nullptr )
	{
		m_pInput->ResetJoyButtonPushed( m_iButton, m_iJoyStick ) ;
	}
}

// トグルUI禁止状態設定
void AGLStdMessageProcessor::UIToggleKey::EnableToggle( bool flagEnabled )
{
	m_flagEnabled = flagEnabled ;
}


//////////////////////////////////////////////////////////////////////////////
// ユーザー入力標準実装
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( AntirrhinumGL::AGLStdMessageProcessor, AGLMessageProcessor )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
AGLStdMessageProcessor::AGLStdMessageProcessor( void )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
AGLStdMessageProcessor::~AGLStdMessageProcessor( void )
{
}

// UIMessage 取得
//////////////////////////////////////////////////////////////////////////////
AGLMessageProcessor::UIMessage *
	AGLStdMessageProcessor::GetMessageUI( const wchar_t * pwszID )
{
	return	GetMessageWindow( pwszID ) ;
}

// 操作キー入力
//////////////////////////////////////////////////////////////////////////////
bool AGLStdMessageProcessor::GetPushedKeyState
		( AGLMessageProcessor::OperationKey key )
{
	UIToggle *	pToggle = m_aOpKeys.GetAt( key ) ;
	if ( pToggle != nullptr )
	{
		return	pToggle->IsToggle() ;
	}
	return	false ;
}

void AGLStdMessageProcessor::ClearPushedKeyState
		( AGLMessageProcessor::OperationKey key )
{
	UIToggle *	pToggle = m_aOpKeys.GetAt( key ) ;
	if ( pToggle != nullptr )
	{
		pToggle->SetToggle( false ) ;
	}
}

// トグルボタン
//////////////////////////////////////////////////////////////////////////////
void AGLStdMessageProcessor::SetToggleButton
	( AGLMessageProcessor::UIMessage::ToggleButtonIndex tbi, UIToggle * pToggle )
{
	m_aToggle.SetAt( tbi, pToggle ) ;
}

void AGLStdMessageProcessor::SetToggleButtonAsSprite
	( AGLMessageProcessor::UIMessage::ToggleButtonIndex tbi, SakuraGL::SGLSprite * pSprite )
{
	m_aToggle.SetAt( tbi, new UIToggleSprite( pSprite ) ) ;
}

void AGLStdMessageProcessor::SetToggleButtonAsForm
	(AGLMessageProcessor:: UIMessage::ToggleButtonIndex tbi, SakuraGL::SGLBasicForm::Button * pButton )
{
	m_aToggle.SetAt( tbi, new UIToggleFormButton( pButton ) ) ;
}

void AGLStdMessageProcessor::SetToggleButtonAsKey
	( AGLMessageProcessor::UIMessage::ToggleButtonIndex tbi,
		SakuraGL::SGLVirtualInput * pInput, size_t iButton, size_t iJoyStick )
{
	m_aToggle.SetAt( tbi, new UIToggleKey( pInput, iButton, iJoyStick ) ) ;
}

// 操作キー
//////////////////////////////////////////////////////////////////////////////
void AGLStdMessageProcessor::SetOperationKey
	( AGLMessageProcessor::OperationKey key, UIToggle * pToggle )
{
	m_aOpKeys.SetAt( key, pToggle ) ;
}

void AGLStdMessageProcessor::SetOperationKey
	( AGLMessageProcessor::OperationKey key,
		SakuraGL::SGLVirtualInput * pInput, size_t iButton, size_t iJoyStick )
{
	m_aOpKeys.SetAt( key, new UIToggleKey( pInput, iButton, iJoyStick ) ) ;
}

// トグルUI状態取得
//////////////////////////////////////////////////////////////////////////////
bool AGLStdMessageProcessor::IsToggleButton( UIMessage::ToggleButtonIndex tbi )
{
	UIToggle *	pToggle = m_aToggle.GetAt( tbi ) ;
	if ( pToggle != nullptr )
	{
		return	pToggle->IsToggle() ;
	}
	return	false ;
}

// トグルUI状態設定
//////////////////////////////////////////////////////////////////////////////
void AGLStdMessageProcessor::SetToggleButton( UIMessage::ToggleButtonIndex tbi, bool flagPushed )
{
	UIToggle *	pToggle = m_aToggle.GetAt( tbi ) ;
	if ( pToggle != nullptr )
	{
		pToggle->SetToggle( flagPushed ) ;
	}
}

// トグルUI禁止状態設定
//////////////////////////////////////////////////////////////////////////////
void AGLStdMessageProcessor::EnableToggleButton( UIMessage::ToggleButtonIndex tbi, bool flagEnabled )
{
	UIToggle *	pToggle = m_aToggle.GetAt( tbi ) ;
	if ( pToggle != nullptr )
	{
		pToggle->EnableToggle( flagEnabled ) ;
	}
}

// メッセージウィンドウ取得／作成
//////////////////////////////////////////////////////////////////////////////
AGLStdMessageProcessor::StdMessage *
	AGLStdMessageProcessor::GetMessageWindow( const wchar_t * pwszID )
{
	StdMessage *	pStdMsg = m_ssoaMsgWindow.GetAs( pwszID ) ;
	if ( pStdMsg == nullptr )
	{
		pStdMsg = NewMessageWindow( pwszID ) ;
		if ( pStdMsg != nullptr )
		{
			m_ssoaMsgWindow.SetAs( pwszID, pStdMsg ) ;
		}
	}
	return	pStdMsg ;
}

// メッセージウィンドウの削除
//////////////////////////////////////////////////////////////////////////////
void AGLStdMessageProcessor::RemoveMessageWindow( const wchar_t * pwszID )
{
	m_ssoaMsgWindow.RemoveAs( pwszID ) ;
}

void AGLStdMessageProcessor::RemoveAllMessageWindows( void )
{
	m_ssoaMsgWindow.RemoveAll() ;
}

// メッセージウィンドウ生成
//////////////////////////////////////////////////////////////////////////////
AGLStdMessageProcessor::StdMessage *
	AGLStdMessageProcessor::NewMessageWindow( const wchar_t * pwszID )
{
	if ( (pwszID == nullptr) || (pwszID[0] == 0) )
	{
		StdMessage *	pStdMsg = new StdMessage( this ) ;
		pStdMsg->SetMessageConfig( m_cfgMessage ) ;
		return	pStdMsg ;
	}
	StdMessageConfig *	pMsgCfg = m_ssoaMsgConfig.GetAs( pwszID ) ;
	if ( pMsgCfg != nullptr )
	{
		StdMessage *	pStdMsg = new StdMessage( this ) ;
		pStdMsg->SetMessageConfig( *pMsgCfg ) ;
		return	pStdMsg ;
	}
	return	nullptr ;
}

// 選択肢オブジェクト生成
//////////////////////////////////////////////////////////////////////////////
AGLMessageProcessor::UISelector * AGLStdMessageProcessor::NewSelector( void )
{
	StdSelector *	pStdSel = new StdSelector() ;
	pStdSel->SetSelectorConfig( m_cfgSelector ) ;
	return	pStdSel ;
}

// デフォルトメッセージウィンドウの設定
//////////////////////////////////////////////////////////////////////////////
void AGLStdMessageProcessor::SetDefaultMessageConfig( const StdMessageConfig& cfg )
{
	m_cfgMessage.SetMessageConfig( cfg ) ;
}

// 指定メッセージウィンドウの設定
//////////////////////////////////////////////////////////////////////////////
void AGLStdMessageProcessor::SetMessageConfigAs
	( const wchar_t * pwszID, const StdMessageConfig& cfg )
{
	if ( (pwszID == nullptr) || (pwszID[0] == 0) )
	{
		SetDefaultMessageConfig( cfg ) ;
		return ;
	}
	else
	{
		StdMessageConfig *	pMsgCfg = new StdMessageConfig ;
		pMsgCfg->SetMessageConfig( cfg ) ;
		m_ssoaMsgConfig.SetAs( pwszID, pMsgCfg ) ;
	}
}

// デフォルト選択肢の設定
//////////////////////////////////////////////////////////////////////////////
void AGLStdMessageProcessor::SetDefaultSelectorConfig( const StdSelectorConfig& cfg )
{
	m_cfgSelector.SetSelectorConfig( cfg ) ;
}


