
#include <antirrhinum/antirrhinum.h>

using namespace	SSystem ;
using namespace	Rosetta ;
using namespace	SakuraGL ;
using namespace	AntirrhinumGL ;


//////////////////////////////////////////////////////////////////////////////
// プレイリスト
//////////////////////////////////////////////////////////////////////////////

const SSystem::SXMLDocument::AttrInteger
	AGLSoundProcessor::Playlist::m_aiPlayType[4] =
{
	{ L"in_order", playInOrder },
	{ L"reverse", playReverse },
	{ L"random", playRandom },
	{ NULL, 0 },
} ;

// 構築関数
//////////////////////////////////////////////////////////////////////////////
AGLSoundProcessor::Playlist::Playlist( void )
{
	m_typePlay = playInOrder ;
}

// xml 形式プレイリストを読み込む (*.xml;*.wpl)
// <smil><body><seq><media src="...">...
//////////////////////////////////////////////////////////////////////////////
bool AGLSoundProcessor::Playlist::LoadPlaylist( const wchar_t * pwszFilePath )
{
	SXMLDocument	xmlDoc ;
	if ( xmlDoc.LoadDocument( pwszFilePath, xmlDoc ) )
	{
		return	false ;
	}
	SXMLDocument *	pxmlSmil = xmlDoc.GetElementTagAs( L"smil" ) ;
	if ( pxmlSmil == NULL )
	{
		return	false ;
	}
	SXMLDocument *	pxmlBody = pxmlSmil->GetElementTagAs( L"body" ) ;
	if ( pxmlBody == NULL )
	{
		return	false ;
	}
	SXMLDocument *	pxmlSeq = pxmlBody->GetElementTagAs( L"seq" ) ;
	if ( pxmlSeq == NULL )
	{
		return	false ;
	}
	m_typePlay = (PlayType) pxmlSeq->GetAttrSymbolizedIntegerAs
							( L"play_order", m_aiPlayType, m_typePlay ) ;
	//
	for ( size_t i = 0; i < pxmlSeq->GetElementsCount(); i ++ )
	{
		SXMLDocument *	pxmlMedia = pxmlSeq->GetElementAt( i ) ;
		if ( (pxmlMedia == NULL)
			|| (pxmlMedia->GetTag() != L"media") )
		{
			continue ;
		}
		const SString *	pstrSrc = pxmlMedia->GetAttributeAs( L"src" ) ;
		if ( pstrSrc != NULL )
		{
			Add( new SString( *pstrSrc ) ) ;
		}
	}
	return	true ;
}



//////////////////////////////////////////////////////////////////////////////
// 再生チャネル
//////////////////////////////////////////////////////////////////////////////

const SSystem::SXMLDocument::AttrInteger
	AGLSoundProcessor::Channel::m_aiLoopType[4] =
{
	{ L"no", loopNo },
	{ L"force", loopForce },
	{ L"auto", loopAuto },
	{ NULL, 0 },
} ;

// 構築関数
//////////////////////////////////////////////////////////////////////////////
AGLSoundProcessor::Channel::Channel( void )
{
	m_flagPlay = false ;
	m_iPlayCurrent = 0 ;
	m_typeLoop = loopNo ;
	m_maskVolLine = (1 << SGLAudioPlayer::lineMusic) ;
	m_fpVolume[0] = 1.0 ;
	m_fpVolume[1] = 1.0 ;
	m_random.InitializeSeed() ;
}

// 構築関数
//////////////////////////////////////////////////////////////////////////////
AGLSoundProcessor::Channel::~Channel( void )
{
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
bool AGLSoundProcessor::Channel::OpenFile( const wchar_t * pwszFileName )
{
	m_player = NULL ;
	m_playlist = NULL ;
	m_flagPlay = false ;
	m_iPlayCurrent = 0 ;
	m_strFileName = pwszFileName ;
	//
	SString	strFileExt = m_strFileName.GetFileExtensionPart() ;
	if ( (strFileExt.CompareNoCase( L"xml" ) == 0)
		|| (strFileExt.CompareNoCase( L"wpl" ) == 0) )
	{
		m_playlist = new Playlist ;
		if ( !m_playlist->LoadPlaylist( pwszFileName ) )
		{
			m_playlist = NULL ;
			return	false ;
		}
		return	true ;
	}
	else
	{
		if ( strFileExt == L"" )
		{
			if ( m_strFileName.GetLastAt(0) != L'.' )
			{
				m_strFileName += L"." ;
			}
			m_strFileName += L"mio" ;
		}
		m_player = new SGLAudioPlayer ;
		if ( m_player->Open( m_strFileName ) )
		{
			m_player = NULL ;
			return	false ;
		}
		ApplyVolumeToPlay() ;
	}
	return	true ;
}

// 音量設定
//////////////////////////////////////////////////////////////////////////////
void AGLSoundProcessor::Channel::SetVolume( double volLeft, double volRight )
{
	m_fpVolume[0] = volLeft ;
	m_fpVolume[1] = volRight ;
	//
	ApplyVolumeToPlay() ;
}

void AGLSoundProcessor::Channel::FadeVolume( double volLeft, double volRight, uint32_t msecFade )
{
	if ( m_player != NULL )
	{
		float32_t	vol[2] =
		{
			(float32_t) m_fpVolume[0],
			(float32_t) m_fpVolume[1],
		} ;
		m_player->BeginFadeVolume( vol, 2, msecFade ) ;
	}
	m_fpVolume[0] = volLeft ;
	m_fpVolume[1] = volRight ;
}

// ループ設定
//////////////////////////////////////////////////////////////////////////////
void AGLSoundProcessor::Channel::SetLoop( LoopType type )
{
	m_typeLoop = type ;
}

// 音量ライン設定
//////////////////////////////////////////////////////////////////////////////
void AGLSoundProcessor::Channel::SetVolumeLine( uint32_t maskLine )
{
	m_maskVolLine = maskLine ;
	//
	ApplyVolumeToPlay() ;
}

// 再生開始
//////////////////////////////////////////////////////////////////////////////
void AGLSoundProcessor::Channel::Play( void )
{
	if ( m_flagPlay )
	{
		return ;
	}
	if ( m_playlist != NULL )
	{
		if ( m_playlist->GetLength() == 0 )
		{
			return ;
		}
		switch ( m_playlist->m_typePlay )
		{
		case	Playlist::playInOrder:
			StartPlaylistAt( 0 ) ;
			break ;
		case	Playlist::playReverse:
			StartPlaylistAt( m_playlist->GetLength() - 1 ) ;
			break ;
		case	Playlist::playRandom:
			m_random.InitializeSeed() ;
			StartPlaylistAt
				( (size_t) m_random.Randomize
							( (uint32_t) m_playlist->GetLength() ) ) ;
			break ;
		}
		m_flagPlay = true ;
	}
	else if ( m_player != NULL )
	{
		switch ( m_typeLoop )
		{
		case	loopNo:
			m_player->SetLoop( false ) ;
			break ;
		case	loopForce:
			m_player->SetLoop( true ) ;
			break ;
		default:
			break ;
		}
		if ( m_player->Play() )
		{
			m_flagPlay = false ;
		}
		else
		{
			m_flagPlay = true ;
		}
	}
	else
	{
		m_flagPlay = false ;
	}
}

// 停止
//////////////////////////////////////////////////////////////////////////////
void AGLSoundProcessor::Channel::Stop( uint32_t msecFadeout )
{
	if ( !m_flagPlay )
	{
		return ;
	}
	if ( m_player != NULL )
	{
		if ( msecFadeout == 0 )
		{
			m_player->Stop() ;
		}
		else
		{
			float32_t	vol[2] = { 0.0f, 0.0f } ;
			m_player->BeginFadeVolume( vol, 2, msecFadeout ) ;
		}
	}
	m_flagPlay = false ;
}

// 再生中か？
//////////////////////////////////////////////////////////////////////////////
bool AGLSoundProcessor::Channel::IsPlaying( void ) const
{
	if ( m_playlist != NULL )
	{
		return	m_flagPlay ;
	}
	if ( m_flagPlay && (m_player != NULL) )
	{
		return	m_player->IsPlaying() ;
	}
	return	false ;
}

// プレイヤーデタッチ
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLAudioPlayer *
	AGLSoundProcessor::Channel::DetachPlayer( void )
{
	return	m_player.Detach() ;
}

// 音量設定をプレイヤーに反映
//////////////////////////////////////////////////////////////////////////////
void AGLSoundProcessor::Channel::ApplyVolumeToPlay( void )
{
	if ( m_player != NULL )
	{
		m_player->SetVolumeLineMask( m_maskVolLine ) ;
		m_player->SetStereoVolume
			( (float32_t) m_fpVolume[0], (float32_t) m_fpVolume[1] ) ;
	}
}

// プレイリストの指定番号のファイルの再生を開始
//////////////////////////////////////////////////////////////////////////////
void AGLSoundProcessor::Channel::StartPlaylistAt( size_t i )
{
	if ( m_playlist == NULL )
	{
		return ;
	}
	m_iPlayCurrent = i ;
	//
	SString *	pstrFile = m_playlist->GetAt( i ) ;
	if ( pstrFile != NULL )
	{
		m_player = new SGLAudioPlayer ;
		if ( !m_player->Open( *pstrFile ) )
		{
			ApplyVolumeToPlay() ;
			m_player->Play() ;
		}
	}
}

// タイマ処理
//////////////////////////////////////////////////////////////////////////////
void AGLSoundProcessor::Channel::OnTimer( AGLSoundProcessor& soundProc )
{
	if ( m_flagPlay && (m_playlist != NULL) )
	{
		if ( (m_player == NULL)
			|| !m_player->IsPlaying() )
		{
			bool	flagEndOfList = false ;
			switch ( m_playlist->m_typePlay )
			{
			case	Playlist::playInOrder:
				flagEndOfList = ((++ m_iPlayCurrent) >= m_playlist->GetLength()) ;
				break ;
			case	Playlist::playReverse:
				flagEndOfList = ((m_iPlayCurrent --) == 0) ;
				break ;
			case	Playlist::playRandom:
				m_iPlayCurrent =
					( (size_t) m_random.Randomize
								( (uint32_t) m_playlist->GetLength() ) ) ;
				break ;
			}
			if ( !flagEndOfList )
			{
				StartPlaylistAt( m_iPlayCurrent ) ;
			}
			else if ( m_typeLoop == loopForce )
			{
				switch ( m_playlist->m_typePlay )
				{
				case	Playlist::playInOrder:
					StartPlaylistAt( 0 ) ;
					break ;
				case	Playlist::playReverse:
					StartPlaylistAt( m_playlist->GetLength() - 1 ) ;
					break ;
				case	Playlist::playRandom:
					m_random.InitializeSeed() ;
					StartPlaylistAt
						( (size_t) m_random.Randomize
									( (uint32_t) m_playlist->GetLength() ) ) ;
					break ;
				}
			}
			else
			{
				m_flagPlay = false ;
			}
		}
	}
}

// 保存
//////////////////////////////////////////////////////////////////////////////
void AGLSoundProcessor::Channel::Serialize( SSystem::SXMLDocument& xmlDoc )
{
	xmlDoc.SetTag( L"channel" ) ;
	xmlDoc.SetAttributeAs( L"file", m_strFileName ) ;
	if ( m_playlist != NULL )
	{
		xmlDoc.SetAttrIntegerAs( L"current", m_iPlayCurrent ) ;
	}
	xmlDoc.SetAttrSymbolizedIntegerAs( L"loop", m_aiLoopType, m_typeLoop ) ;
	xmlDoc.SetAttrHexIntegerAs( L"vol_lines", m_maskVolLine ) ;
	xmlDoc.SetAttrRealAs( L"vol_left", m_fpVolume[0] ) ;
	xmlDoc.SetAttrRealAs( L"vol_right", m_fpVolume[1] ) ;
	//
	if ( IsPlaying() )
	{
		xmlDoc.SetAttrIntegerAs( L"play", 1 ) ;
		//
		if ( m_player != NULL )
		{
			xmlDoc.SetAttrIntegerAs( L"pos", m_player->GetPosition() ) ;
		}
	}
}

// 復元
//////////////////////////////////////////////////////////////////////////////
void AGLSoundProcessor::Channel::Deserialize( const SSystem::SXMLDocument& xmlDoc )
{
	m_typeLoop = (LoopType)
		xmlDoc.GetAttrSymbolizedIntegerAs( L"loop", m_aiLoopType, m_typeLoop ) ;
	m_maskVolLine = (uint32_t)
		xmlDoc.GetAttrHexIntegerAs( L"vol_lines", m_maskVolLine ) ;
	m_fpVolume[0] = xmlDoc.GetAttrRealAs( L"vol_left", m_fpVolume[0] ) ;
	m_fpVolume[1] = xmlDoc.GetAttrRealAs( L"vol_right", m_fpVolume[1] ) ;
	//
	SString	strFileName = xmlDoc.GetAttrStringAs( L"file" ) ;
	OpenFile( strFileName ) ;
	//
	if ( m_playlist != NULL )
	{
		StartPlaylistAt
			( (size_t) xmlDoc.GetAttrIntegerAs( L"current", 0 ) ) ;
	}
	if ( xmlDoc.GetAttrIntegerAs( L"play" ) != 0 )
	{
		if ( m_player != NULL )
		{
			m_player->SeekPosition( xmlDoc.GetAttrIntegerAs( L"pos" ) ) ;
			//
			if ( m_playlist == NULL )
			{
				switch ( m_typeLoop )
				{
				case	loopNo:
					m_player->SetLoop( false ) ;
					break ;
				case	loopForce:
					m_player->SetLoop( true ) ;
					break ;
				default:
					break ;
				}
			}
			m_player->Play() ;
		}
	}
}



//////////////////////////////////////////////////////////////////////////////
// サウンド処理
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO
	( AntirrhinumGL::AGLSoundProcessor, AGLEpicFuncProcessor )
AGL_IMPLEMENT_EPIC_PROCESSOR( AntirrhinumGL::AGLSoundProcessor )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
AGLSoundProcessor::AGLSoundProcessor( void )
	: AGLEpicFuncProcessor( m_pFirstFuncDesc, L"sound" )
{
	SetVolumeLineName( SGLAudioPlayer::lineTotal, L"total" ) ;
	SetVolumeLineName( SGLAudioPlayer::lineTotal2nd, L"total2" ) ;
	SetVolumeLineName( SGLAudioPlayer::lineComposition, L"composition" ) ;
	SetVolumeLineName( SGLAudioPlayer::lineSystem, L"system" ) ;
	SetVolumeLineName( SGLAudioPlayer::lineMusic, L"music" ) ;
	SetVolumeLineName( SGLAudioPlayer::lineSound, L"sound" ) ;
	SetVolumeLineName( SGLAudioPlayer::lineVoice, L"voice" ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
AGLSoundProcessor::~AGLSoundProcessor( void )
{
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
bool AGLSoundProcessor::OpenFile
		( size_t iChannel, const wchar_t * pwszFileName )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	return	SafeChannelAt(iChannel)->OpenFile( pwszFileName ) ;
}

// 音量設定
//////////////////////////////////////////////////////////////////////////////
void AGLSoundProcessor::SetVolume( size_t iChannel, double volLeft, double volRight )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	SafeChannelAt(iChannel)->SetVolume( volLeft, volRight ) ;
}

void AGLSoundProcessor::FadeVolume
	( size_t iChannel, double volLeft, double volRight, uint32_t msecFade )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	SafeChannelAt(iChannel)->FadeVolume( volLeft, volRight, msecFade ) ;
}

// ループ設定
//////////////////////////////////////////////////////////////////////////////
void AGLSoundProcessor::SetLoop( size_t iChannel, Channel::LoopType type )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	SafeChannelAt(iChannel)->SetLoop( type ) ;
}

// 音量ライン設定
//////////////////////////////////////////////////////////////////////////////
void AGLSoundProcessor::SetVolumeLine( size_t iChannel, uint32_t maskLine )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	SafeChannelAt(iChannel)->SetVolumeLine( maskLine ) ;
}

// 再生開始
//////////////////////////////////////////////////////////////////////////////
void AGLSoundProcessor::PlayChannel( size_t iChannel )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	SafeChannelAt(iChannel)->Play() ;
}

// 停止
//////////////////////////////////////////////////////////////////////////////
void AGLSoundProcessor::StopChannel( size_t iChannel, uint32_t msecFadeout )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	//
	Channel *	pChannel = SafeChannelAt(iChannel) ;
	pChannel->Stop( msecFadeout ) ;
	//
	if ( msecFadeout > 0 )
	{
		AddFadeoutPlayer( pChannel->DetachPlayer() ) ;
	}
}

void AGLSoundProcessor::StopAllChannels( uint32_t msecFadeout )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	//
	for ( size_t i = 0; i < m_aChannels.GetLength(); i ++ )
	{
		if ( m_aChannels.GetAt(i) != NULL )
		{
			StopChannel( i, msecFadeout ) ;
		}
	}
}

// 解放
//////////////////////////////////////////////////////////////////////////////
void AGLSoundProcessor::ReleaseChannel( size_t iChannel )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	if ( iChannel < m_aChannels.GetLength() )
	{
		m_aChannels.SetAt( iChannel, NULL ) ;
	}
}

void AGLSoundProcessor::ReleaseAllChannels( void )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	m_aChannels.RemoveAll() ;
}

// チャネル取得
//////////////////////////////////////////////////////////////////////////////
AGLSoundProcessor::Channel *
		AGLSoundProcessor::SafeChannelAt( size_t iChannel )
{
	SSmartLock<SCriticalSection>	lock( &m_csSync ) ;
	//
	Channel *	pChannel = m_aChannels.GetAt( iChannel ) ;
	if ( pChannel == NULL )
	{
		pChannel = new Channel ;
		m_aChannels.SetAt( iChannel, pChannel ) ;
	}
	return	pChannel ;
}

// フェードアウトリストに追加
//////////////////////////////////////////////////////////////////////////////
void AGLSoundProcessor::AddFadeoutPlayer( SakuraGL::SGLAudioPlayer * pPlayer )
{
	m_csSync.Lock() ;
	m_aFadeout.Add( pPlayer ) ;
	m_csSync.Unlock() ;
}

// 音量ライン名設定
//////////////////////////////////////////////////////////////////////////////
void AGLSoundProcessor::SetVolumeLineName( size_t i, const wchar_t * pwszName )
{
	m_aVolLineNames.SetAt( i, new SString( pwszName ) ) ;
}

// 音量ライン番号取得
//////////////////////////////////////////////////////////////////////////////
size_t AGLSoundProcessor::GetVolumeLineAs
	( const wchar_t * pwszName, size_t iDefault ) const
{
	for ( size_t i = 0; i < m_aVolLineNames.GetLength(); i ++ )
	{
		SString *	pstrLineName = m_aVolLineNames.GetAt( i ) ;
		if ( (pstrLineName != NULL)
			&& (*pstrLineName == pwszName) )
		{
			return	i ;
		}
	}
	if ( m_pKernel != NULL )
	{
		SString	strErrMsg ;
		strErrMsg.Format( L"undefined sound volume line id: \'%s\'\n", pwszName ) ;
		m_pKernel->OutputTrace( strErrMsg ) ;
	}
	return	iDefault ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SSystem::SError AGLSoundProcessor::Serialize
		( SSystem::SXMLDocument& xmlTag, AGLKernel * pKernel )
{
	for ( size_t i = 0; i < m_aChannels.GetLength(); i ++ )
	{
		Channel *	pChannel = m_aChannels.GetAt( i ) ;
		if ( pChannel == NULL )
		{
			continue ;
		}
		SXMLDocument *	pxmlChannel = new SXMLDocument ;
		pChannel->Serialize( *pxmlChannel ) ;
		//
		pxmlChannel->SetTag( L"channel" ) ;
		pxmlChannel->SetAttrIntegerAs( L"index", i ) ;
		xmlTag.AddElement( pxmlChannel ) ;
	}
	return	errSuccess ;
}

// デシリアライズ
//////////////////////////////////////////////////////////////////////////////
SSystem::SError AGLSoundProcessor::Deserialize
		( const SSystem::SXMLDocument& xmlTag, AGLKernel * pKernel )
{
	m_csSync.Lock() ;
	m_aChannels.RemoveAll() ;
	m_aFadeout.RemoveAll() ;
	m_csSync.Unlock() ;
	//
	for ( size_t i = 0; i < xmlTag.GetElementsCount(); i ++ )
	{
		const SXMLDocument *	pxmlChannel = xmlTag.GetElementAt( i ) ;
		if ( (pxmlChannel == NULL)
			|| (pxmlChannel->GetTag() != L"channel") )
		{
			continue ;
		}
		size_t	iChannel = (size_t) pxmlChannel->GetAttrIntegerAs( L"index" ) ;
		SafeChannelAt( iChannel )->Deserialize( *pxmlChannel ) ;
	}
	return	errSuccess ;
}

// デシリアライズ後の参照解決処理
//////////////////////////////////////////////////////////////////////////////
SSystem::SError AGLSoundProcessor::AfterDeserialize( AGLKernel * pKernel )
{
	return	errSuccess ;
}

// 設定
//////////////////////////////////////////////////////////////////////////////
void AGLSoundProcessor::LoadConfiguration
	( const SSystem::SXMLDocument& xmlConfig )
{
	const SXMLDocument *
			pxmlSound = xmlConfig.GetElementTagAs( L"sound" ) ;
	if ( pxmlSound == NULL )
	{
		return ;
	}
	const SXMLDocument *
			pxmlLineDef = pxmlSound->GetElementTagAs( L"volume_lines" ) ;
	if ( pxmlLineDef != NULL )
	{
		for ( size_t i = 0; i < pxmlLineDef->GetElementsCount(); i ++ )
		{
			const SXMLDocument *	pxmlLine = pxmlLineDef->GetElementAt( i ) ;
			if ( (pxmlLine == NULL)
				|| (pxmlLine->GetTag() != L"line") )
			{
				continue ;
			}
			size_t	iUserLine = (size_t) pxmlLine->GetAttrIntegerAs( L"num" ) ;
			SString	strName = pxmlLine->GetAttrStringAs( L"id" ) ;
			if ( !strName.IsEmpty() )
			{
				SetVolumeLineName
					( SGLAudioPlayer::lineUserFirst + iUserLine, strName ) ;
			}
		}
	}
}

// タイマ処理 (実行フレーム前処理)
//////////////////////////////////////////////////////////////////////////////
void AGLSoundProcessor::OnKernelTimer( void )
{
	m_csSync.Lock() ;
	for ( size_t i = 0; i < m_aFadeout.GetLength(); i ++ )
	{
		SGLAudioPlayer *	pPlayer = m_aFadeout.GetAt( i ) ;
		if ( (pPlayer == NULL)
			|| !pPlayer->IsPlaying()
			|| !pPlayer->IsVolumeFading() )
		{
			m_aFadeout.RemoveAt( i -- )  ;
		}
	}
	for ( size_t i = 0; i < m_aChannels.GetLength(); i ++ )
	{
		Channel *	pChannel = m_aChannels.GetAt( i ) ;
		if ( pChannel != NULL )
		{
			pChannel->OnTimer( *this ) ;
		}
	}
	m_csSync.Unlock() ;
}

// ゲーム終了前フェードアウト処理
//////////////////////////////////////////////////////////////////////////////
void AGLSoundProcessor::FadeoutGame( uint32_t msecFadeout )
{
	StopAllChannels( msecFadeout ) ;
}

// ゲーム終了時処理
//////////////////////////////////////////////////////////////////////////////
void AGLSoundProcessor::ReleaseGame( void )
{
	ReleaseAllChannels() ;

	m_csSync.Lock() ;
	m_aFadeout.RemoveAll() ;
	m_csSync.Unlock() ;
}

// コマンド実装
//////////////////////////////////////////////////////////////////////////////
IMPL_ANTIRRHINUM_PROC(AGLSoundProcessor,sound_play)
{
	size_t		iChannel = (size_t) code.GetAttrIntegerAs( L"channel" ) ;
	Channel *	pChannel = SafeChannelAt( iChannel ) ;
	if ( pChannel->IsPlaying() )
	{
		pChannel->Stop( 500 ) ;
		AddFadeoutPlayer( pChannel->DetachPlayer() ) ;
	}
	if ( pChannel->OpenFile
		( EvaluateExprInText( thread, code.GetAttrStringAs( L"src" ) ) ) )
	{
		double	vol = code.GetAttrRealAs( L"volume", 1.0 ) ;
		pChannel->SetVolume( vol, vol ) ;
		//
		size_t	iVolLine = SGLAudioPlayer::lineMusic ;
		const SString *	pstrLine = code.GetAttributeAs( L"line" ) ;
		if ( pstrLine != NULL )
		{
			iVolLine = GetVolumeLineAs( *pstrLine, iVolLine ) ;
		}
		pChannel->SetVolumeLine( (uint32_t) (1 << iVolLine) ) ;
		//
		pChannel->SetLoop
			( code.GetAttrIntegerAs( L"loop", 0 )
				? Channel::loopForce : Channel::loopNo ) ;
		//
		pChannel->Play() ;
	}
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLSoundProcessor,sound_stop)
{
	size_t		iChannel = (size_t) code.GetAttrIntegerAs( L"channel" ) ;
	Channel *	pChannel = m_aChannels.GetAt( iChannel ) ;
	if ( pChannel == NULL )
	{
		return	codeProcessed ;
	}
	uint32_t	msecFadeout = (uint32_t) code.GetAttrIntegerAs( L"fadeout" ) ;
	//
	StopChannel( iChannel, msecFadeout ) ;
	ReleaseChannel( iChannel ) ;
	//
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLSoundProcessor,sound_volume)
{
	size_t		iChannel = (size_t) code.GetAttrIntegerAs( L"channel" ) ;
	double		vol = code.GetAttrRealAs( L"volume", 1.0 ) ;
	uint32_t	msecFade = (uint32_t) code.GetAttrIntegerAs( L"fade", 0 ) ;
	//
	FadeVolume( iChannel, vol, vol, msecFade ) ;
	//
	return	codeProcessed ;
}

IMPL_ANTIRRHINUM_PROC(AGLSoundProcessor,sound_wait)
{
	size_t		iChannel = (size_t) code.GetAttrIntegerAs( L"channel" ) ;
	Channel *	pChannel = m_aChannels.GetAt( iChannel ) ;
	if ( pChannel == NULL )
	{
		return	codeProcessed ;
	}
	if ( thread.IsPermittedSkip( syncTypeEvent )
		&& m_pKernel->ShouldAbortSync( syncTypeEvent ) )
	{
		pChannel->Stop( 0 ) ;
		m_pKernel->NotifyAbortedSync( syncTypeEvent ) ;
		return	codeProcessed ;
	}
	if ( pChannel->IsPlaying() )
	{
		return	codePending ;
	}
	return	codeProcessed ;
}
