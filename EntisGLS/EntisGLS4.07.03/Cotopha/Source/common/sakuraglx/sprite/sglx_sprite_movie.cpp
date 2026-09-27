
#include <sakuraglx/sakuraglx.h>
#include <sakuraglx/sprite/sglx_sprite_movie.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// 動画再生スプライト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSpriteMovie, SGLSprite )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteMovie::SGLSpriteMovie( void )
	: m_pVideoViewWindow( NULL ),
		m_flagEndOfDuration(false),
		m_flagDrawDirect(false), m_flagMovieLoop(false),
		m_nMovieLoopStart(0), m_nMovieLoopEnd(0)
{
}

SGLSpriteMovie::SGLSpriteMovie( const SGLSpriteMovie& movie )
	: SGLSprite( movie ), m_pVideoViewWindow( NULL ),
		m_flagEndOfDuration(false),
		m_flagDrawDirect(false), m_flagMovieLoop(false),
		m_nMovieLoopStart(0), m_nMovieLoopEnd(0)
{
	if ( movie.m_player != NULL )
	{
		SGLAudioPlayerInterface *	pAudio = movie.m_player->ClonePlayer() ;
		m_strMovieFile = movie.m_strMovieFile ;
		m_player = ESLTypeCast<SGLMediaPlayer>( pAudio ) ;
		if ( m_player == NULL )
		{
			delete	pAudio ;
		}
	}
	else if ( movie.m_strMovieFile.IsEmpty() )
	{
		OpenMovieFile( movie.m_strMovieFile ) ;
	}
	if ( m_player != NULL )
	{
		SetMovieLoop
			( movie.m_flagMovieLoop,
				movie.m_nMovieLoopStart, movie.m_nMovieLoopEnd ) ;
		SeekMovie( movie.GetMoviePosition() ) ;
		if ( movie.IsMoviePlaying() )
		{
			uint32_t	nFlags = 0 ;
			if ( movie.m_flagDrawDirect )
			{
				nFlags |= playDrawDirect ;
			}
			PlayMovie( nFlags ) ;
			if ( movie.IsMoviePaused() )
			{
				PauseMovie() ;
			}
		}
	}
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSpriteMovie::~SGLSpriteMovie( void )
{
	DetachSyncTimeout( 100 ) ;
}

// 動画ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteMovie::OpenMovieFile( const wchar_t * pwszMoviePath )
{
	if ( m_player != NULL )
	{
		CloseMovieFile() ;
	}
	SGLMediaPlayer *	player = new SGLMediaPlayer ;
	SGLError	err = player->Open( pwszMoviePath ) ;
	if ( err )
	{
		delete	player ;
		return	err ;
	}
	LockTrace( __FILE__, __LINE__ ) ;
	m_strMovieFile = pwszMoviePath ;
	m_player = player ;
	m_flagEndOfDuration = false ;
	player->SetNotificationListener( this ) ;
	player->GetVideoSize( m_sizeMovieFrame ) ;
	NotifyUpdate() ;
	Unlock() ;
	return	sglErrSuccess ;
}

// 動画ファイルを閉じる
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteMovie::CloseMovieFile( void )
{
	if ( m_player != NULL )
	{
		SGLMediaPlayer *	player ;
		LockTrace( __FILE__, __LINE__ ) ;
		NotifyUpdate() ;
		player = m_player.Detach() ;
		Unlock() ;
		delete	player ;
	}
	m_strMovieFile.FreeArray() ;
	m_flagMovieLoop = false ;
	m_sizeMovieFrame.w = 0 ;
	m_sizeMovieFrame.h = 0 ;
	return	sglErrSuccess ;
}

// 再生を開始する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteMovie::PlayMovie( uint64_t nFlags )
{
	SGLMediaPlayer *	player = m_player ;
	if ( player != NULL )
	{
		m_flagDrawDirect = ((nFlags & playDrawDirect) != 0) ;
		UpdateVideoView() ;
		m_flagEndOfDuration = false ;
		return	player->Play( nFlags ) ;
	}
	return	sglErrFailed ;
}

// 停止する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteMovie::StopMovie( void )
{
	SGLMediaPlayer *	player = m_player ;
	if ( player != NULL )
	{
		return	player->Stop() ;
	}
	return	sglErrFailed ;
}

// ループポイント[/frame] を設定する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteMovie::SetMovieLoop
	( bool fLoop, int64_t nStart, int64_t nEnd )
{
	SGLMediaPlayer *	player = m_player ;
	if ( player != NULL )
	{
		LockTrace( __FILE__, __LINE__ ) ;
		player->SetLoop( fLoop, nStart, nEnd ) ;
		m_flagMovieLoop = fLoop ;
		m_nMovieLoopStart = nStart ;
		m_nMovieLoopEnd = nEnd ;
		Unlock() ;
		return	sglErrSuccess ;
	}
	return	sglErrFailed ;
}

// ループ設定取得
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteMovie::IsMovieLoop( int64_t * pStart, int64_t * pEnd ) const
{
	if ( pStart != NULL )
	{
		*pStart = m_nMovieLoopStart ;
	}
	if ( pEnd != NULL )
	{
		*pEnd = m_nMovieLoopEnd ;
	}
	return	m_flagMovieLoop ;
}

// 再生を一時停止する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteMovie::PauseMovie( void )
{
	SGLMediaPlayer *	player = m_player ;
	if ( player != NULL )
	{
		return	player->Pause() ;
	}
	return	sglErrFailed ;
}

// 再生を再開する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteMovie::RestartMovie( void )
{
	SGLMediaPlayer *	player = m_player ;
	if ( player != NULL )
	{
		UpdateVideoView() ;
		return	player->Restart() ;
	}
	return	sglErrFailed ;
}

// 音量取得 [L/R]
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteMovie::GetVolume( float32_t* pVolumes, size_t nChannels )
{
	SGLMediaPlayer *	player = m_player ;
	if ( player != NULL )
	{
		return	player->GetVolume( pVolumes, nChannels ) ;
	}
	return	sglErrFailed ;
}

// 音量設定 [L/R]
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteMovie::SetVolume( const float32_t* pVolumes, size_t nChannels )
{
	SGLMediaPlayer *	player = m_player ;
	if ( player != NULL )
	{
		return	player->SetVolume( pVolumes, nChannels ) ;
	}
	return	sglErrFailed ;
}

// 音量を反映させるラインをクリアする（lineTotal,lineTotal2nd 以外）
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMovie::ClearAllVolumeLines( void )
{
	SGLAudioPlayer *	pAudio = ESLTypeCast<SGLAudioPlayer>( m_player.Ptr() ) ;
	if ( pAudio != NULL )
	{
		pAudio->ClearAllVolumeLines() ;
	}
}

// 音量ライン追加設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMovie::SetVolumeLine( size_t iLine )
{
	SGLAudioPlayer *	pAudio = ESLTypeCast<SGLAudioPlayer>( m_player.Ptr() ) ;
	if ( pAudio != NULL )
	{
		pAudio->SetVolumeLine( iLine ) ;
	}
}

// 再生中か？
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteMovie::IsMoviePlaying( void ) const
{
	SGLMediaPlayer *	player = m_player ;
	if ( player != NULL )
	{
		return	player->IsPlaying() ;
	}
	return	false ;
}

// 一時停止中か？
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteMovie::IsMoviePaused( void ) const
{
	SGLMediaPlayer *	player = m_player ;
	if ( player != NULL )
	{
		return	player->IsPaused() ;
	}
	return	false ;
}

// メディアのサンプル周波数を取得する
//////////////////////////////////////////////////////////////////////////////
uint32_t SGLSpriteMovie::GetMovieFrequency( void ) const
{
	SGLMediaPlayer *	player = m_player ;
	if ( player != NULL )
	{
		return	player->GetSampleFrequency() ;
	}
	return	0 ;
}

// メディアの全長 [/sample] を取得する
//////////////////////////////////////////////////////////////////////////////
uint64_t SGLSpriteMovie::GetMovieLength( void ) const
{
	SGLMediaPlayer *	player = m_player ;
	if ( player != NULL )
	{
		return	player->GetTotalLength() ;
	}
	return	0 ;
}

// 再生位置 [/sample] を取得する
//////////////////////////////////////////////////////////////////////////////
uint64_t SGLSpriteMovie::GetMoviePosition( void ) const
{
	SGLMediaPlayer *	player = m_player ;
	if ( player != NULL )
	{
		return	player->GetPosition() ;
	}
	return	0 ;
}

// 再生位置 [/sample] を変更する
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMovie::SeekMovie( uint64_t nPos )
{
	SGLMediaPlayer *	player = m_player ;
	if ( player != NULL )
	{
		player->SeekPosition( nPos ) ;
	}
}

// ビデオサイズを取得する
//////////////////////////////////////////////////////////////////////////////
const SGLSize& SGLSpriteMovie::GetMovieSize( void ) const
{
	return	m_sizeMovieFrame ;
}

// 再生終端到達判定
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteMovie::HasEndedOfDuration( void ) const
{
	return	m_flagEndOfDuration ;
}

// 終端到達フラグをクリア
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMovie::ResetEndOfDuration( void )
{
	m_flagEndOfDuration = false ;
}

// ビデオの表示設定
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMovie::UpdateVideoView( void )
{
	SGLMediaPlayer *	player = m_player ;
	if ( player != NULL )
	{
		uint64_t	nFlags = 0 ;
		if ( !m_flagDrawDirect )
		{
			nFlags |= MediaPlayer::flagUpdateWindow ;
		}
		GetVideoRectOnWindow( m_rectVideoView ) ;
		//
		SGLWindowSprite *	pWindow = SGLWindowSprite::WindowOf( this ) ;
		m_pVideoViewWindow = pWindow ;
		if ( pWindow != NULL )
		{
			player->SetVideoView( pWindow, m_rectVideoView, nFlags ) ;
		}
		else
		{
			player->SetVideoView( NULL, m_rectVideoView, nFlags ) ;
		}
	}
}

// ビデオの表示領域取得
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMovie::GetVideoRectOnWindow( SGLImageRect& rectVideo ) const
{
	SGLRect	rectMovie ;
	rectMovie.SetPosition( SGLPoint( 0, 0 ) ) ;
	rectMovie.SetSize( m_sizeMovieFrame ) ;
	//
	SGLSprite *	pSprite = (SGLSprite*) this ;
	LockTrace( __FILE__, __LINE__ ) ;
	while ( pSprite != NULL )
	{
		pSprite->LocalToGlobalRect( rectMovie ) ;
		pSprite = pSprite->GetParent() ;
	}
	Unlock() ;
	rectVideo = rectMovie ;
}

// 描画
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMovie::Draw
	( S3DRenderContextInterface& render,
		const SGLSprite::Virtual3DParam* pV3D,
		SGLSprite::Stereo3DView s3dView ) const
{
	SGLMediaPlayer *	player = m_player ;
	if ( player != NULL )
	{
		SGLPaintParam	pp ;
		SGLAffine		affine ;
		if ( GetPaintParam( pp, affine, pV3D, s3dView ) )
		{
			SGLImageRect	rectVideo ;
			rectVideo.SetPosition( SGLPoint( 0, 0 ) ) ;
			rectVideo.SetSize( m_sizeMovieFrame ) ;
			//
			render.PushTransformation() ;
			render.AppendTransformation( affine, pp.nTransparency ) ;
			player->DrawVideo( &render, rectVideo, pp.nFlags ) ;
			render.PopTransformation() ;
		}
	}
	SGLSprite::Draw( render, pV3D, s3dView ) ;
}

// 外接矩形取得
//////////////////////////////////////////////////////////////////////////////
bool SGLSpriteMovie::GetRectangle( SGLRect& rectExt ) const
{
	SGLMediaPlayer *	player = m_player ;
	if ( player != NULL )
	{
		SGLRect	rectVideo ;
		rectVideo.SetPosition( SGLPoint( 0, 0 ) ) ;
		rectVideo.SetSize( m_sizeMovieFrame ) ;
		//
		if ( LocalToGlobalRect( rectVideo ) )
		{
			if ( SGLSprite::GetRectangle( rectExt ) )
			{
				rectExt |= rectVideo ;
			}
			else
			{
				rectExt = rectVideo ;
			}
			return	true ;
		}
	}
	return	SGLSprite::GetRectangle( rectExt ) ;
}

// フレーム更新通知
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMovie::OnFrameUpdate( SGLMediaPlayerInterface * player )
{
	SGLImageRect	rectVideo ;
	GetVideoRectOnWindow( rectVideo ) ;
	if ( (m_pVideoViewWindow == NULL) || (m_rectVideoView != rectVideo) )
	{
		UpdateVideoView() ;
	}
	NotifyUpdate() ;
}

// 再生区間終端到達
//////////////////////////////////////////////////////////////////////////////
void SGLSpriteMovie::OnEndOfDuration( SGLMediaPlayerInterface * player )
{
	m_flagEndOfDuration = true ;
}

// Rosetta 用クラス
//////////////////////////////////////////////////////////////////////////////
const wchar_t * SGLSpriteMovie::GetRSClassName( void ) const
{
	return	L"MovieSprite" ;
}

// Loquaty 用クラス
//////////////////////////////////////////////////////////////////////////////
const wchar_t * SGLSpriteMovie::GetLQClassName( void ) const
{
	return	L"EntisGLS4.MovieSprite" ;
}

// 複製
//////////////////////////////////////////////////////////////////////////////
SGLObject * SGLSpriteMovie::DuplicateObject( void )
{
	return	new SGLSpriteMovie( *this ) ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteMovie::OnSave( SSystem::SFileInterface& file )
{
	SGLError	err = SGLSprite::OnSave( file ) ;
	if ( err )
	{
		return	err ;
	}
	file.WriteString( m_strMovieFile ) ;
	//
	uint32_t	nFlags = 0 ;
	uint32_t	nLineMask = 1 ;
	uint64_t	nPos = 0 ;
	float32_t	nVolumes[8] = { 1, 1, 1, 1, 1, 1, 1, 1 } ;
	SGLMediaPlayer *	player = m_player ;
	if ( player != NULL )
	{
		if ( player->IsPlaying() )
		{
			nFlags |= 0x01 ;
			if ( player->IsPaused() )
			{
				nFlags |= 0x02 ;
			}
		}
		nPos = player->GetPosition() ;
		//
		SGLAudioPlayer *	pAudio = ESLTypeCast<SGLAudioPlayer>( player ) ;
		if ( pAudio != NULL )
		{
			nLineMask = pAudio->GetVolumeLineMask() ;
		}
		player->GetVolume( nVolumes, 8 ) ;
	}
	if ( m_flagMovieLoop )
	{
		nFlags |= 0x10 ;
	}
	file.Write( &nFlags, sizeof(uint32_t) ) ;
	file.Write( &nLineMask, sizeof(uint32_t) ) ;
	file.Write( &nPos, sizeof(uint64_t) ) ;
	file.Write( &nVolumes[0], sizeof(float32_t) * 8 ) ;
	file.Write( &m_nMovieLoopStart, sizeof(int64_t) ) ;
	file.Write( &m_nMovieLoopEnd, sizeof(int64_t) ) ;
	return	sglErrSuccess ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteMovie::OnRestore( SSystem::SFileInterface& file )
{
	SGLError	err = SGLSprite::OnRestore( file ) ;
	if ( err )
	{
		return	err ;
	}
	SString		strMovieFile ;
	uint32_t	nFlags = 0 ;
	uint32_t	nLineMask = 1 ;
	uint64_t	nPos = 0 ;
	float32_t	nVolumes[8] = { 1, 1, 1, 1, 1, 1, 1, 1 } ;
	int64_t		nLoopStart, nLoopEnd ;
	//
	file.ReadString( strMovieFile ) ;
	file.Read( &nFlags, sizeof(uint32_t) ) ;
	file.Read( &nLineMask, sizeof(uint32_t) ) ;
	file.Read( &nPos, sizeof(uint64_t) ) ;
	file.Read( &nVolumes[0], sizeof(float32_t) * 8 ) ;
	file.Read( &nLoopStart, sizeof(int64_t) ) ;
	file.Read( &nLoopEnd, sizeof(int64_t) ) ;
	//
	m_flagRestorePlaying = false ;
	m_flagRestorePaused = false ;
	//
	if ( !strMovieFile.IsEmpty() )
	{
		if ( !OpenMovieFile( strMovieFile ) )
		{
			SGLAudioPlayer *
				pAudio = ESLTypeCast<SGLAudioPlayer>( m_player.Ptr() ) ;
			if ( pAudio != NULL )
			{
				pAudio->SetVolumeLineMask( nLineMask ) ;
			}
			SetMovieLoop
				( ((nFlags & 0x10) != 0), nLoopStart, nLoopEnd ) ;
			SetVolume( &nVolumes[0], 8 ) ;
			SeekMovie( nPos ) ;
			//
			m_flagRestorePlaying = ((nFlags & 0x01) != 0) ;
			m_flagRestorePaused = ((nFlags & 0x02) != 0) ;
		}
	}
	return	sglErrSuccess ;
}

// 復元後処理
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSpriteMovie::OnAfterRestore( void )
{
	SGLError	err = SGLSprite::OnAfterRestore() ;
	if ( err )
	{
		return	err ;
	}
	if ( m_flagRestorePlaying )
	{
		PlayMovie() ;
		if ( m_flagRestorePaused )
		{
			PauseMovie() ;
		}
	}
	return	sglErrSuccess ;
}



