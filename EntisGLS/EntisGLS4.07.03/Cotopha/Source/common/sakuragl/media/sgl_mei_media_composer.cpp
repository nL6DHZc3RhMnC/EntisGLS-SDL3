
#include <sakuragl/sakuragl.h>
#include <sakuragl/sgl_erisa_lib.h>
#include <sakuragl/media/sgl_mei_media_composer.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// MEI ファイル入力ラッパー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO3
( SakuraGL::SGLMEIMediaInputStream,
		SGLAudioInputStream, SGLVideoInputStream, SGLMovieFilePlayer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLMEIMediaInputStream::SGLMEIMediaInputStream( void )
{
	m_flagSound = false ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLMEIMediaInputStream::~SGLMEIMediaInputStream( void )
{
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMEIMediaInputStream::Open( const wchar_t * pwszFilePath )
{
	SFileInterface *	pfile =
		SFileOpener::DefaultNewOpenFile
				( pwszFilePath, SFileOpener::shareRead ) ;
	if ( pfile == NULL )
	{
		return	sglErrFailed ;
	}
	return	Open( pfile, true ) ;
}

SGLError SGLMEIMediaInputStream::Open
	( SSystem::SFileInterface * pfile, bool flagOwnFile )
{
	Close() ;
	//
	if ( SGLMovieFilePlayer::OpenMovieFile( pfile, flagOwnFile ) )
	{
		return	sglErrFailed ;
	}
	if ( m_flagSound )
	{
		SGLMovieFilePlayer::BeginWaveStreaming() ;
	}
	return	sglErrSuccess ;
}

// ファイルを閉じる
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMEIMediaInputStream::Close( void )
{
	if ( m_flagSound )
	{
		SGLMovieFilePlayer::EndWaveStreaming() ;
		m_flagSound = false ;
	}
	m_qbufSound.ClearAll() ;
	//
	SGLMovieFilePlayer::Close() ;
	//
	return	sglErrSuccess ;
}

// サウンド形式取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMEIMediaInputStream::GetAudioFormat( SGLSoundFormat& fmt )
{
	if ( m_flagSound )
	{
		fmt = m_fmtSound ;
		return	sglErrSuccess ;
	}
	return	sglErrFailed ;
}

// メディア補助情報取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMEIMediaInputStream::GetAudioOptinalInfo( SGLMediaOptionalInfo& optinf )
{
	optinf.m_nFlags = 0 ;
	return	sglErrSuccess ;
}

// オーディオストリーム全長取得（未定は-1）[samples]
//////////////////////////////////////////////////////////////////////////////
int64_t SGLMEIMediaInputStream::GetAudioLength( void ) const
{
	if ( m_erif.m_flagsRead & ERISA::SGLMediaFile::readSoundInfo )
	{
		return	m_erif.m_mioInfoHeader.dwAllSampleCount ;
	}
	return	0 ;
}

// オーディオストリーム読み込み [samples]
//////////////////////////////////////////////////////////////////////////////
size_t SGLMEIMediaInputStream::ReadAudio( void * ptrBuf, size_t nSamples )
{
	if ( !m_flagSound )
	{
		return	0 ;
	}
	size_t	nBlockBytes =
				m_fmtSound.channels * m_fmtSound.bitsPerSample / 8 ;
	if ( nBlockBytes == 0 )
	{
		return	0 ;
	}
	size_t	nBytes = nSamples * nBlockBytes ;
	return	m_qbufSound.Read( ptrBuf, nBytes ) / nBlockBytes ;
}

// オーディオストリーム位置変更
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMEIMediaInputStream::SeekAudio( uint64_t nSamples )
{
	return	sglErrFailed ;
}

// ビデオ画像形式取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMEIMediaInputStream::GetImageFormat( SGLImageInfo& imginf )
{
	SGLImageObject *	pImage = SGLMovieFilePlayer::CurrentFrame() ;
	if ( pImage == NULL )
	{
		return	sglErrFailed ;
	}
	return	pImage->GetImageInfo( imginf ) ;
}

// メディア補助情報取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMEIMediaInputStream::GetVideoOptinalInfo( SGLMediaOptionalInfo& optinf )
{
	optinf.m_nFlags = 0 ;
	//
	if ( m_erif.m_flagsRead & ERISA::SGLMediaFile::readDescription )
	{
		ERISA::SGLMediaFile::STagInfo	taginf ;
		taginf.ParseTagInfo( m_erif.m_strDescription ) ;
		//
		int64_t	nRewindPoint = taginf.GetRewindPoint( 0 ) ;
		if ( nRewindPoint >= 0 )
		{
			optinf.m_nFlags |= SGLMediaOptionalInfo::flagLoopStart ;
			optinf.m_nLoopStart = (uint64_t) nRewindPoint ;
		}
		int64_t	nLoopEnd = taginf.GetLoopEndPoint() ;
		if ( nLoopEnd >= 0 )
		{
			optinf.m_nFlags |= SGLMediaOptionalInfo::flagLoopEnd ;
			optinf.m_nLoopEnd = (uint64_t) nLoopEnd ;
		}
		const wchar_t *	pwszTitle =
				taginf.GetTagContents( ERISA::SGLMediaFile::tagVocalPlayer ) ;
		if ( pwszTitle != NULL )
		{
			optinf.m_nFlags |= SGLMediaOptionalInfo::flagTitle ;
			optinf.m_strTitle = pwszTitle ;
		}
		const wchar_t *	pwszPlayer =
				taginf.GetTagContents( ERISA::SGLMediaFile::tagVocalPlayer ) ;
		if ( pwszPlayer != NULL )
		{
			optinf.m_nFlags |= SGLMediaOptionalInfo::flagVocalPlayer ;
			optinf.m_strPlayer = pwszPlayer ;
		}
		const wchar_t *	pwszComposer =
				taginf.GetTagContents( ERISA::SGLMediaFile::tagComposer ) ;
		if ( pwszComposer != NULL )
		{
			optinf.m_nFlags |= SGLMediaOptionalInfo::flagComposer ;
			optinf.m_strComposer = pwszComposer ;
		}
		const wchar_t *	pwszArranger =
				taginf.GetTagContents( ERISA::SGLMediaFile::tagArranger ) ;
		if ( pwszArranger != NULL )
		{
			optinf.m_nFlags |= SGLMediaOptionalInfo::flagArranger ;
			optinf.m_strArranger = pwszArranger ;
		}
	}
	//
	return	sglErrSuccess ;
}

// ビデオストリーム全長取得（未定は-1）[frames]
//////////////////////////////////////////////////////////////////////////////
int64_t SGLMEIMediaInputStream::GetVideoLength( void ) const
{
	return	(int64_t) SGLMovieFilePlayer::GetAllFrameCount() ;
}

// ビデオストリーム全長取得（未定は-1）[millisecond]
//////////////////////////////////////////////////////////////////////////////
int64_t SGLMEIMediaInputStream::GetVideoDuration( void ) const
{
	return	(int64_t) SGLMovieFilePlayer::GetTotalTime() ;
}

// ビデオストリーム読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMEIMediaInputStream::ReadFrame
	( const SGLImageInfo& imginf, uint8_t * pbytBuf )
{
	SGLImageObject *	pImage = SGLMovieFilePlayer::CurrentFrame() ;
	if ( pImage == NULL )
	{
		return	sglErrFailed ;
	}
	SGLError	err =
		pImage->ReadFrameBuffer
				( imginf, pbytBuf, pImage->GetSelectedFrame() ) ;
	SGLMovieFilePlayer::SeekToNextFrame() ;
	return	err ;
}

// ビデオストリーム位置変更
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMEIMediaInputStream::SeekFrame( uint64_t nFrames )
{
	if ( m_flagSound )
	{
		SGLMovieFilePlayer::EndWaveStreaming() ;
		m_qbufSound.ClearAll() ;
	}
	SError	err = SGLMovieFilePlayer::SeekToFrame( nFrames ) ;
	//
	if ( m_flagSound )
	{
		SGLMovieFilePlayer::BeginWaveStreaming() ;
	}
	return	err ? sglErrFailed : sglErrSuccess ;
}

// 音声出力要求
//////////////////////////////////////////////////////////////////////////////
bool SGLMEIMediaInputStream::RequestWaveOut
	( uint32_t channels, uint32_t frequency, uint32_t bps )
{
	m_flagSound = true ;
	m_fmtSound.format = formatSoundLinearPCM ;
	m_fmtSound.frequency = frequency ;
	m_fmtSound.channels = channels ;
	m_fmtSound.bitsPerSample = bps ;
	m_qbufSound.ClearAll() ;
	return	true ;
}

// 音声出力終了
//////////////////////////////////////////////////////////////////////////////
void SGLMEIMediaInputStream::CloseWaveOut( void )
{
	m_qbufSound.ClearAll() ;
}

// 音声データ出力
//////////////////////////////////////////////////////////////////////////////
void SGLMEIMediaInputStream::PushWaveBuffer( const void * ptrWaveBuf, size_t nBytes )
{
	m_qbufSound.Write( ptrWaveBuf, nBytes ) ;
}


//////////////////////////////////////////////////////////////////////////
// MIO ファイル出力ラッパー
//////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLMIOAudioOutputStream, SGLAudioOutputStream ) ;

// 構築関数
//////////////////////////////////////////////////////////////////////////
SGLMIOAudioOutputStream::SGLMIOAudioOutputStream( void )
{
	eslFillMemory( &m_mih, 0, sizeof(ERISA::ERI_INFO_HEADER) ) ;
	SetSoundCompressionPreset( ERISA::SGLSoundEncoder::ppVBR128kbps ) ;
	m_fOpenFile = false ;
	m_fBeginStream = false ;
	m_nSamples = 0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////
SGLMIOAudioOutputStream::~SGLMIOAudioOutputStream( void )
{
	SGLMIOAudioOutputStream::Close() ;
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////
SGLError SGLMIOAudioOutputStream::Open( const wchar_t * pwszFilePath )
{
	SFileInterface *	pfile =
		SFileOpener::DefaultNewOpenFile
				( pwszFilePath, SFileOpener::modeCreate ) ;
	if ( pfile == NULL )
	{
		return	sglErrFailed ;
	}
	return	Open( pfile, true ) ;
}

SGLError SGLMIOAudioOutputStream::Open
	( SSystem::SFileInterface * pfile, bool flagOwnFile )
{
	Close() ;
	//
	if ( m_mfw.OpenMediaFile
		( pfile, ERISA::SGLMediaFileWriter::fidSound, flagOwnFile ) )
	{
		return	sglErrFailed ;
	}
	m_fOpenFile = true ;
	return	sglErrSuccess ;
}

// ファイルを閉じる
//////////////////////////////////////////////////////////////////////////
SGLError SGLMIOAudioOutputStream::Close( void )
{
	if ( m_fBeginStream )
	{
		m_mfw.EndStream
			( (uint32_t) (m_nSamples * 1000 / m_mih.dwSamplesPerSec) ) ;
		m_fBeginStream = false ;
	}
	if ( m_fOpenFile )
	{
		m_mfw.Close() ;
		m_fOpenFile = false ;
	}
	return	sglErrSuccess ;
}

// 圧縮プリセット設定
//////////////////////////////////////////////////////////////////////////
void SGLMIOAudioOutputStream::SetSoundCompressionPreset
			( ERISA::SGLSoundEncoder::PresetParameter pp )
{
	m_sencParam.LoadPresetParam( pp, m_mih ) ;
}

// 音声情報ヘッダを設定する（圧縮パラメータのみ）
//////////////////////////////////////////////////////////////////////////
void SGLMIOAudioOutputStream::SetMioInfoHeader
		( const ERISA::MIO_INFO_HEADER & mih )
{
	m_mih = mih ;
}

// 音声の圧縮パラメータを設定する
//////////////////////////////////////////////////////////////////////////
void SGLMIOAudioOutputStream::SetSoundCompressionParameter
		( const ERISA::SGLSoundEncoder::Parameter & param )
{
	m_sencParam = param ;
}

// ヘッダ書き出し
//////////////////////////////////////////////////////////////////////////
SGLError SGLMIOAudioOutputStream::WriteHeaders
				( const SGLMediaOptionalInfo * pOptInf )
{
	if ( m_mfw.WriteMioInfoHeader( m_mih ) )
	{
		return	sglErrFailed ;
	}
	if ( pOptInf != NULL )
	{
		m_mfw.WriteDescription( FormatMediaOption( pOptInf ) ) ;
	}
	return	sglErrSuccess ;
}

// オプション情報変換
//////////////////////////////////////////////////////////////////////////
SSystem::SString SGLMIOAudioOutputStream::FormatMediaOption( const SGLMediaOptionalInfo * pOptInf )
{
	ERISA::SGLMediaFile::STagInfo	taginf ;
	if ( pOptInf->m_nFlags & SGLMediaOptionalInfo::flagLoopStart )
	{
		taginf.AddTag
			( ERISA::SGLMediaFile::tagRewindPoint,
						SString(pOptInf->m_nLoopStart) ) ;
	}
	if ( pOptInf->m_nFlags & SGLMediaOptionalInfo::flagLoopEnd )
	{
		taginf.AddTag
			( ERISA::SGLMediaFile::tagLoopEndPoint,
						SString(pOptInf->m_nLoopEnd) ) ;
	}
	if ( pOptInf->m_nFlags & SGLMediaOptionalInfo::flagTitle )
	{
		taginf.AddTag
			( ERISA::SGLMediaFile::tagTitle, pOptInf->m_strTitle ) ;
	}
	if ( pOptInf->m_nFlags & SGLMediaOptionalInfo::flagVocalPlayer )
	{
		taginf.AddTag
			( ERISA::SGLMediaFile::tagVocalPlayer, pOptInf->m_strPlayer ) ;
	}
	if ( pOptInf->m_nFlags & SGLMediaOptionalInfo::flagComposer )
	{
		taginf.AddTag
			( ERISA::SGLMediaFile::tagComposer, pOptInf->m_strComposer ) ;
	}
	if ( pOptInf->m_nFlags & SGLMediaOptionalInfo::flagArranger )
	{
		taginf.AddTag
			( ERISA::SGLMediaFile::tagArranger, pOptInf->m_strArranger ) ;
	}
	SString	strDesc ;
	taginf.FormatTagInfo( strDesc ) ;
	return	strDesc ;
}

// オーディオ出力ストリーム準備
//////////////////////////////////////////////////////////////////////////
SGLError SGLMIOAudioOutputStream::PrepareAudio
	( const SGLSoundFormat& fmt, int64_t nSamples,
				const SGLMediaOptionalInfo * pOptInf )
{
	m_mih.dwChannelCount = fmt.channels ;
	m_mih.dwSamplesPerSec = fmt.frequency ;
	m_mih.dwBitsPerSample = fmt.bitsPerSample ;
	m_mih.dwAllSampleCount = (DWORD) nSamples ;
	//
	if ( m_mfw.BeginFileHeader( 15, 16, 3 ) )
	{
		return	sglErrFailed ;
	}
	if ( WriteHeaders( pOptInf ) )
	{
		return	sglErrFailed ;
	}
	m_mfw.EndFileHeader() ;
	//
	m_mfw.SetSoundCompressionParameter( m_sencParam ) ;
	//
	if ( m_mfw.BeginStream() )
	{
		return	sglErrFailed ;
	}
	m_fBeginStream = true ;
	m_nSamples = 0 ;
	return	sglErrSuccess ;
}

// オーディオストリーム出力 [samples]
//////////////////////////////////////////////////////////////////////////
size_t SGLMIOAudioOutputStream::WriteAudio
				( const void * ptrBuf, size_t nSamples )
{
	if ( !m_fBeginStream )
	{
		return	0 ;
	}
	if ( m_mfw.WriteWaveData( ptrBuf, nSamples ) )
	{
		return	0 ;
	}
	m_nSamples += nSamples ;
	return	nSamples ;
}


//////////////////////////////////////////////////////////////////////////////
// MEI ファイル出力ラッパー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::SGLMEIMediaOutputStream,
			SGLMIOAudioOutputStream, SGLVideoOutputStream ) ;

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLMEIMediaOutputStream::SGLMEIMediaOutputStream( void )
{
	eslFillMemory( &m_eih, 0, sizeof(ERISA::ERI_INFO_HEADER) ) ;
	SetImageCompressionPreset( ERISA::SGLImageEncoder::ppHighQuality ) ;
	m_nFrames = 0 ;
	m_nKeyFrame = 15 ;
	m_nBFrames = 3 ;
	m_nRateFPS = 30 ;
	m_nScaleFPS = 1 ;
	m_fPreparedAudio = false ;
	m_fPreparedVideo = false ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLMEIMediaOutputStream::~SGLMEIMediaOutputStream( void )
{
	SGLMEIMediaOutputStream::Close() ;
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMEIMediaOutputStream::Open
	( SSystem::SFileInterface * pfile, bool flagOwnFile )
{
	Close() ;
	//
	if ( m_mfw.OpenMediaFile
		( pfile, ERISA::SGLMediaFileWriter::fidMovie, flagOwnFile ) )
	{
		return	sglErrFailed ;
	}
	m_fOpenFile = true ;
	return	sglErrSuccess ;
}

// ファイルを閉じる
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMEIMediaOutputStream::Close( void )
{
	if ( m_fBeginStream )
	{
		m_mfw.EndStream
			( (uint32_t) (m_nFrames * 1000 * m_nScaleFPS / m_nRateFPS) ) ;
		m_fBeginStream = false ;
	}
	if ( m_fOpenFile )
	{
		m_mfw.Close() ;
		m_fOpenFile = false ;
	}
	m_fPreparedAudio = false ;
	m_fPreparedVideo = false ;
	m_strDescription.FreeArray() ;
	return	sglErrSuccess ;
}

// キーフレーム間隔を設定
//////////////////////////////////////////////////////////////////////////////
void SGLMEIMediaOutputStream::SetKeyFrame( size_t nKeyFrame, size_t nBFrames )
{
	m_nKeyFrame = nKeyFrame ;
	m_nBFrames = nBFrames ;
}

// 圧縮プリセット設定
//////////////////////////////////////////////////////////////////////////////
void SGLMEIMediaOutputStream::SetImageCompressionPreset
		( ERISA::SGLImageEncoder::PresetParameter pp )
{
	m_iencParam.LoadPresetParam( pp, m_eih ) ;
}

// 音声情報ヘッダを設定する（圧縮パラメータのみ）
//////////////////////////////////////////////////////////////////////////////
void SGLMEIMediaOutputStream::SetEriInfoHeader
		( const ERISA::ERI_INFO_HEADER & eih )
{
	m_eih = eih ;
}

// 音声の圧縮パラメータを設定する
//////////////////////////////////////////////////////////////////////////////
void SGLMEIMediaOutputStream::SetImageCompressionParameter
		( const ERISA::SGLImageEncoder::Parameter & param )
{
	m_iencParam = param ;
}

// ヘッダ書き出し
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMEIMediaOutputStream::WriteFileHeader( void )
{
	if ( !m_fOpenFile )
	{
		return	sglErrFailed ;
	}
	if ( m_fBeginStream )
	{
		return	sglErrFailed ;
	}
	if ( m_mfw.BeginFileHeader( m_nKeyFrame, 16, m_nBFrames ) )
	{
		return	sglErrFailed ;
	}
	if ( m_fPreparedVideo )
	{
		if ( m_mfw.WriteEriInfoHeader( m_eih ) )
		{
			return	sglErrFailed ;
		}
	}
	if ( m_fPreparedAudio )
	{
		if ( m_mfw.WriteMioInfoHeader( m_mih ) )
		{
			return	sglErrFailed ;
		}
	}
	if ( !m_strDescription.IsEmpty() )
	{
		m_mfw.WriteDescription( m_strDescription ) ;
	}
	m_mfw.EndFileHeader() ;
	//
	if ( m_fPreparedVideo )
	{
		m_mfw.SetImageCompressionParameter( m_iencParam ) ;
	}
	if ( m_fPreparedAudio )
	{
		m_mfw.SetSoundCompressionParameter( m_sencParam ) ;
	}
	//
	if ( m_mfw.BeginStream() )
	{
		return	sglErrFailed ;
	}
	m_fBeginStream = true ;
	m_nSamples = 0 ;
	m_nFrames = 0 ;
	return	sglErrSuccess ;
}

// オーディオ出力ストリーム準備
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMEIMediaOutputStream::PrepareAudio
	( const SGLSoundFormat& fmt, int64_t nSamples,
				const SGLMediaOptionalInfo * pOptInf )
{
	m_mih.dwChannelCount = fmt.channels ;
	m_mih.dwSamplesPerSec = fmt.frequency ;
	m_mih.dwBitsPerSample = fmt.bitsPerSample ;
	m_mih.dwAllSampleCount = (DWORD) nSamples ;
	m_fPreparedAudio = true ;
	return	sglErrSuccess ;
}

// オーディオストリーム出力 [samples]
//////////////////////////////////////////////////////////////////////////////
size_t SGLMEIMediaOutputStream::WriteAudio( const void * ptrBuf, size_t nSamples )
{
	if ( !m_fBeginStream )
	{
		WriteFileHeader() ;
	}
	return	SGLMIOAudioOutputStream::WriteAudio( ptrBuf, nSamples ) ;
}

// ビデオ出力ストリーム準備
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMEIMediaOutputStream::PrepareVideo
	( const SGLImageInfo& imginf,
		int64_t nFrames, int64_t nDuration,
		const SGLMediaOptionalInfo * pOptInf )
{
	m_eih.fdwFormatType = imginf.format ;
	m_eih.nImageWidth = imginf.width ;
	m_eih.nImageHeight = imginf.height ;
	m_eih.dwBitsPerPixel = imginf.depth ;
	m_eih.dwClippedPixel = imginf.colorClip ;
	m_fPreparedVideo = true ;
	m_imgFrameBuf.CreateImage
		( imginf.width, imginf.height, imginf.format, imginf.depth ) ;
	//
	int64_t	gcd ;
	m_nRateFPS = nFrames * 1000 ;
	m_nScaleFPS = nDuration ;
	gcd = SakuraCL::ComputeGCD<int64_t>( m_nRateFPS, m_nScaleFPS ) ;
	ESLAssert( m_nRateFPS % gcd == 0 ) ;
	ESLAssert( m_nScaleFPS % gcd == 0 ) ;
	m_nRateFPS /= gcd ;
	m_nScaleFPS /= gcd ;
	//
	return	sglErrSuccess ;
}

// ビデオストリーム出力
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMEIMediaOutputStream::WriteFrame
	( const SGLImageInfo& imginf, const uint8_t * pbytBuf )
{
	if ( !m_fBeginStream )
	{
		WriteFileHeader() ;
	}
	SGLImageBuffer	ibufDst ;
	SGLImageBuffer	ibufSrc = imginf ;
	ibufDst.ptrBuffer = m_imgFrameBuf.LockBuffer( ibufDst ) ;
	ibufSrc.ptrBuffer = (uint8_t*) pbytBuf ;
	sglCopyImageBuffer( ibufDst, ibufSrc ) ;
	m_imgFrameBuf.UnlockBuffer() ;
	//
	if ( m_mfw.WriteImageData( m_imgFrameBuf ) )
	{
		return	sglErrFailed ;
	}
	m_nFrames ++ ;
	return	sglErrSuccess ;
}



