
#include <sakuragl/sakuragl.h>
#include <sakuragl/sgl_window.h>
#include <sakuragl/sgl_media.h>
#include <sakuragl/sgl_win_avi_composer.h>
#include <sakuragl/sgl2d/sgl_image_conversion.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// Windows Wave Form Audio ファイル入力
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLWindowWaveFileReader, SGLAudioInputStream )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLWindowWaveFileReader::SGLWindowWaveFileReader( void )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLWindowWaveFileReader::~SGLWindowWaveFileReader( void )
{
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowWaveFileReader::Open( const wchar_t * pwszFilePath )
{
	return	m_decoder.Open( pwszFilePath ) ;
}

SGLError SGLWindowWaveFileReader::Open
	( SSystem::SFileInterface * pfile, bool flagOwnFile )
{
	return	m_decoder.Create( pfile, flagOwnFile ) ;
}

// ファイルを閉じる
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowWaveFileReader::Close( void )
{
	return	m_decoder.Close() ;
}

// サウンド形式取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowWaveFileReader::GetAudioFormat( SGLSoundFormat& fmt )
{
	return	m_decoder.GetFormat( fmt ) ;
}

// メディア補助情報取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowWaveFileReader::GetAudioOptinalInfo( SGLMediaOptionalInfo& optinf )
{
	SGLAudioDecoderInterface::OptionalInfo	oi ;
	SGLError	err = m_decoder.GetOptinalInfo( oi ) ;
	if ( err )
	{
		return	err ;
	}
	optinf.m_nFlags = 0 ;
	if ( oi.nFlags & SGLAudioDecoderInterface::flagLoopStart )
	{
		optinf.m_nFlags |= SGLMediaOptionalInfo::flagLoopStart ;
		optinf.m_nLoopStart = oi.nLoopStart ;
	}
	if ( oi.nFlags & SGLAudioDecoderInterface::flagLoopEnd )
	{
		optinf.m_nFlags |= SGLMediaOptionalInfo::flagLoopEnd ;
		optinf.m_nLoopEnd = oi.nLoopEnd ;
	}
	if ( oi.nFlags & SGLAudioDecoderInterface::flagTitle )
	{
		optinf.m_nFlags |= SGLMediaOptionalInfo::flagTitle ;
		optinf.m_strTitle = oi.pszTitle ;
	}
	if ( oi.nFlags & SGLAudioDecoderInterface::flagVocalPlayer )
	{
		optinf.m_nFlags |= SGLMediaOptionalInfo::flagVocalPlayer ;
		optinf.m_strPlayer = oi.pszVocalPlayer ;
	}
	if ( oi.nFlags & SGLAudioDecoderInterface::flagComposer )
	{
		optinf.m_nFlags |= SGLMediaOptionalInfo::flagComposer ;
		optinf.m_strComposer = oi.pszComposer ;
	}
	if ( oi.nFlags & SGLAudioDecoderInterface::flagArranger )
	{
		optinf.m_nFlags |= SGLMediaOptionalInfo::flagArranger ;
		optinf.m_strArranger = oi.pszArranger ;
	}
	return	err ;
}

// オーディオストリーム全長取得（未定は-1）[samples]
//////////////////////////////////////////////////////////////////////////////
int64_t SGLWindowWaveFileReader::GetAudioLength( void ) const
{
	return	(int64_t) m_decoder.GetTotalLength() ;
}

// オーディオストリーム読み込み [samples]
//////////////////////////////////////////////////////////////////////////////
size_t SGLWindowWaveFileReader::ReadAudio( void * ptrBuf, size_t nSamples )
{
	SGLSoundFormat	fmt ;
	if ( m_decoder.GetFormat( fmt ) )
	{
		return	0 ;
	}
	size_t	nReadSamples = 0 ;
	size_t	nBlockAlign = fmt.bitsPerSample * fmt.channels / 8 ;
	if ( nBlockAlign == 0 )
	{
		return	0 ;
	}
	while ( nSamples != 0 )
	{
		size_t	nBytes = nSamples * nBlockAlign ;
		size_t	nReadBytes = m_bufNext.Read( ptrBuf, nBytes ) ;
		if ( nReadBytes == 0 )
		{
			size_t	nNextBytes = m_decoder.DecodeNext() ;
			if ( nNextBytes == 0 )
			{
				break ;
			}
			nNextBytes =
				m_decoder.ReadDecodedBuffer
					( m_bufNext.PutBuffer( nNextBytes ), nNextBytes ) ;
			m_bufNext.FlushBuffer( nNextBytes ) ;
		}
		else
		{
			size_t	nReadSamples = nReadBytes / nBlockAlign ;
			nReadSamples += nReadSamples ;
			if ( nSamples <= nReadSamples )
			{
				break ;
			}
			nSamples -= nReadSamples ;
			ptrBuf = ((uint8_t*)nSamples) + nReadBytes ;
		}
	}
	return	nReadSamples ;
}

// オーディオストリーム位置変更
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowWaveFileReader::SeekAudio( uint64_t nSamples )
{
	m_bufNext.ClearAll() ;
	return	m_decoder.SeekPosition( nSamples ) ;
}



//////////////////////////////////////////////////////////////////////////////
// Windows AVI ファイル入力ラッパー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::SGLWindowsAVIReader, SGLAudioInputStream, SGLVideoInputStream )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLWindowsAVIReader::SGLWindowsAVIReader( void )
{
	m_pfile = NULL ;
	m_flagOwnFile = false ;
	m_nNextFrame = 0 ;
	m_nNextAudioPCMs = 0 ;
	//
	m_psVideo = NULL ;
	m_pbmihVideo = NULL ;
	//
	m_psAudio = NULL ;
	m_pwfxAudio = NULL ;
	m_flagNeedConversionPCM = false ;
	m_flagAudioSeeked = true ;
	//
	m_hicDecompress = NULL ;
	m_hACMStream = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLWindowsAVIReader::~SGLWindowsAVIReader( void )
{
	Close() ;
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowsAVIReader::Open( const wchar_t * pwszFilePath )
{
	SFileInterface *	pFile =
		SFileOpener::DefaultNewOpenFile
			( pwszFilePath, SFileOpener::shareRead ) ;
	if ( pFile == NULL )
	{
		return	sglErrFailed ;
	}
	return	Open( pFile, true ) ;
}

SGLError SGLWindowsAVIReader::Open
	( SSystem::SFileInterface * pfile, bool flagOwnFile )
{
	Close() ;
	//
	// AVI ファイルチェック ＆ RIFF 位置取得
	//
	uint32_t	nChunk[3] ;
	m_pfile = pfile ;
	m_flagOwnFile = flagOwnFile ;
	do
	{
		int64_t	nPos = m_pfile->GetPosition( ) ;
		m_pfile->Read( &nChunk[0], sizeof(uint32_t) * 3 ) ;
		if ( !IsEqualChunkID( nChunk[0], "RIFF" ) )
		{
			if ( m_arrPosRIFF.GetLength() == 0 )
			{
				ESLTrace( "RIFF チャンクが見つかりません\n" ) ;
				return	sglErrFailed ;
			}
			break ;
		}
		if ( !IsEqualChunkID( nChunk[2], "AVI " )
			&& !IsEqualChunkID( nChunk[2], "AVIX" ) )
		{
			ESLTrace( "AVI ファイルではありません\n" ) ;
			return	sglErrFailed ;
		}
		m_pfile->Seek
			( ((nChunk[1] + 1) & ~0x01) - 4, SFileInterface::FromCurrent ) ;
		m_arrPosRIFF.Add( (uint64_t) nPos ) ;
	}
	while ( m_pfile->GetPosition() < m_pfile->GetLength() ) ;
	//
	m_arrPosRIFF.Add( (uint64_t) m_pfile->GetPosition() ) ;
	//
	// ヘッダを読み込む
	//
	m_pfile->Seek( 0, SFileInterface::FromBegin ) ;
	DescendChunk( ) ;
	ESLAssert( IsEqualCurrentChunkID("RIFF") ) ;
	//
	m_pfile->Seek( 4, SFileInterface::FromCurrent ) ;		// skip AVI
	//
	DescendChunk( ) ;			// LIST (size) hdrl
	if ( !IsEqualCurrentChunkID( "LIST" ) )
	{
		ESLTrace( "LIST チャンクが見つかりません\n" ) ;
		return	sglErrFailed ;
	}
	Read( &nChunk[0], sizeof(uint32_t) ) ;
	if ( !IsEqualChunkID( nChunk[0], "hdrl" ) )
	{
		ESLTrace( "ヘッダが見つかりません\n" ) ;
		return	sglErrFailed ;
	}
	//
	DescendChunk( ) ;			// avih
	if ( !IsEqualCurrentChunkID( "avih" ) )
	{
		ESLTrace( "avih チャンクが見つかりません\n" ) ;
		return	sglErrFailed ;
	}
	m_avimh.fcc = (FOURCC) GetCurrentChunkID( ) ;
	m_avimh.cb = (DWORD) GetLength() - 8 ;
	Read( ((uint8_t*)&m_avimh) + 8, sizeof(m_avimh) - 8 ) ;
	//
	AscendChunk( ) ;
	//
	// ストリームを読み込む
	//
	for ( ; ; )
	{
		if ( DescendChunk( "LIST" ) )
		{
			break ;
		}
		Read( &nChunk[0], sizeof(uint32_t) ) ;
		if ( IsEqualChunkID( nChunk[0], "strl" ) )
		{
			AVIStream *	pStream = new AVIStream ;
			for ( ; ; )
			{
				if ( DescendChunk() )
				{
					break ;
				}
				if ( IsEqualCurrentChunkID( "strh" ) )
				{
					pStream->m_avish.fcc = (FOURCC) GetCurrentChunkID( ) ;
					pStream->m_avish.cb = (DWORD) GetLength() ;
					Read( ((uint8_t*)&(pStream->m_avish)) + 8,
										sizeof(AVISTREAMHEADER) - 8 ) ;
				}
				else if ( IsEqualCurrentChunkID( "strf" ) )
				{
					size_t	nBytes = (size_t) GetLength() ;
					pStream->m_bufHeader.ReadFromFile
									( *m_pfile, (ssize_t) nBytes ) ;
				}
				AscendChunk( ) ;
			}
			if ( (pStream->m_avish.fccType == streamtypeVIDEO)
											&& (m_psVideo == NULL) )
			{
				m_psVideo = pStream ;
				m_pbmihVideo =
					(BITMAPINFOHEADER*) m_psVideo->m_bufHeader.GetArrayPtr() ;
			}
			else if ( (pStream->m_avish.fccType == streamtypeAUDIO)
											&& (m_psAudio == NULL) )
			{
				m_psAudio = pStream ;
				m_pwfxAudio =
					(WAVEFORMATEX*) m_psAudio->m_bufHeader.GetArrayPtr() ;
			}
			else
			{
				delete	pStream ;
			}
		}
		AscendChunk( ) ;
	}
	//
	AscendChunk( ) ;			// LIST (size) hdrl
	//
	// movi データを探す
	//
	if ( DescendLIST_movie() != sglErrSuccess )
	{
		ESLTrace( "LIST movi チャンクが見つかりません\n" ) ;
		return	sglErrFailed ;
	}
	m_nNextFrame = 0 ;
	m_nNextAudioPCMs= 0 ;
	m_arrPosFrame.Add
		( VideoFramePosInfo( (uint64_t) m_pfile->GetPosition(), m_nNextAudioPCMs ) ) ;
	//
	if ( m_psVideo != NULL )
	{
		m_psVideo->m_nNextPos = (uint64_t) m_pfile->GetPosition( ) ;
		//
		CreateDecompressVideoBuffer() ;
	}
	if ( m_psAudio != NULL )
	{
		m_psAudio->m_nNextPos = (uint64_t) m_pfile->GetPosition( ) ;
		m_flagAudioSeeked = true ;
		//
		ESLAssert( m_pwfxAudio != NULL ) ;
		m_wfxAudioPCM = *m_pwfxAudio ;
		//
		m_fmtAudioPCM.format = formatSoundLinearPCM ;
		m_fmtAudioPCM.frequency = m_wfxAudioPCM.nSamplesPerSec ;
		m_fmtAudioPCM.channels = m_wfxAudioPCM.nChannels ;
		m_fmtAudioPCM.bitsPerSample = m_wfxAudioPCM.wBitsPerSample ;
		//
		if ( m_wfxAudioPCM.wFormatTag != WAVE_FORMAT_PCM )
		{
			m_wfxAudioPCM.wFormatTag = WAVE_FORMAT_PCM ;
			m_wfxAudioPCM.wBitsPerSample = 16 ;
			m_wfxAudioPCM.nBlockAlign = m_wfxAudioPCM.nChannels * 2 ;
			m_wfxAudioPCM.nAvgBytesPerSec =
						m_wfxAudioPCM.nSamplesPerSec * m_wfxAudioPCM.nBlockAlign ;
			m_wfxAudioPCM.cbSize = 0 ;
			//
			m_fmtAudioPCM.bitsPerSample = 16 ;
			//
			if ( ::acmStreamOpen
				( &m_hACMStream, NULL,
					m_pwfxAudio, &m_wfxAudioPCM,
					NULL, 0, 0, ACM_STREAMOPENF_NONREALTIME ) )
			{
				ESLTrace( "Failed to Open ACM stream.\n" ) ;
				//
				if ( m_pwfxAudio->wFormatTag == WAVE_FORMAT_EXTENSIBLE )
				{
					WAVEFORMATEXTENSIBLE *	pwfe = (WAVEFORMATEXTENSIBLE*) m_pwfxAudio ;
					if ( IsEqualGUID( KSDATAFORMAT_SUBTYPE_IEEE_FLOAT, pwfe->SubFormat ) )
					{
						m_fmtAudio = m_fmtAudioPCM ;
						m_fmtAudio.format = formatSoundIEEEFloat ;
						m_fmtAudio.bitsPerSample = m_pwfxAudio->wBitsPerSample ;
						m_flagNeedConversionPCM = true ;
					}
					else if ( IsEqualGUID( KSDATAFORMAT_SUBTYPE_PCM, pwfe->SubFormat ) )
					{
						m_fmtAudio = m_fmtAudioPCM ;
						m_fmtAudio.bitsPerSample = m_pwfxAudio->wBitsPerSample ;
						m_flagNeedConversionPCM = true ;
					}
				}
			}
		}
	}
	//
	// idx1 チャンクを探す
	//
	int64_t	nFirstPos = m_pfile->GetPosition( ) ;
	if ( DescendChunk( "idx1" ) == sglErrSuccess )
	{
		size_t	nLength = GetLength() / sizeof(AVIINDEXENTRY) ;
		m_arrIndexEntries.SetLength( nLength ) ;
		Read( m_arrIndexEntries.GetArray(), nLength * sizeof(AVIINDEXENTRY) ) ;
		m_arrIndexEntries.FinishArray() ;
		//
		m_arrVideoIndex.SetLimit( nLength ) ;
		for ( size_t i = 0; i < nLength; i ++ )
		{
			const AVIINDEXENTRY&	idx = m_arrIndexEntries.At(i) ;
			if ( IsVideoFrameChunkID( idx.ckid ) )
			{
				m_arrVideoIndex.Add( i ) ;
			}
		}
		//
		AscendChunk() ;		// idx1
	}
	//
	m_pfile->Seek( nFirstPos, SFileInterface::FromBegin ) ;
	return	sglErrSuccess ;
}

// ファイルを閉じる
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowsAVIReader::Close( void )
{
	if ( m_flagOwnFile )
	{
		delete	m_pfile ;
		m_flagOwnFile = false ;
	}
	m_pfile = NULL ;
	m_arrPosRIFF.RemoveAll( ) ;
	m_arrIndexEntries.RemoveAll( ) ;
	m_arrVideoIndex.RemoveAll( ) ;
	m_arrPosFrame.RemoveAll( ) ;
	m_arrChunk.RemoveAll( ) ;
	//
	delete	m_psVideo ;
	m_psVideo = NULL ;
	m_pbmihVideo = NULL ;
	//
	delete	m_psAudio ;
	m_psAudio = NULL ;
	m_pwfxAudio = NULL ;
	//
	if ( m_hicDecompress != NULL )
	{
		ICDecompressEnd( m_hicDecompress ) ;
		ICClose( m_hicDecompress ) ;
		m_hicDecompress = NULL ;
	}
	if ( m_hACMStream != NULL )
	{
		acmStreamClose( m_hACMStream, 0 ) ;
		m_hACMStream = NULL ;
	}
	m_bufACMStream.ClearAll( ) ;
	//
	return	sglErrSuccess ;
}

// ファイルから読み込み
//////////////////////////////////////////////////////////////////////////////
size_t SGLWindowsAVIReader::Read( void * ptrBuf, size_t nBytes )
{
	ESLAssert( m_pfile != NULL ) ;
	if ( m_pfile == NULL )
	{
		return	0 ;
	}
	RIFF_CHUNK *	pChunk = m_arrChunk.GetLastAt() ;
	ESLAssert( pChunk != NULL ) ;
	if ( pChunk == NULL )
	{
		return	m_pfile->Read( ptrBuf, nBytes ) ;
	}
	int64_t	nPos = m_pfile->GetPosition() - pChunk->nBeginPos ;
	if ( nPos + nBytes > pChunk->nBytes )
	{
		if ( nPos >= pChunk->nBytes )
		{
			return	0 ;
		}
		nBytes = (size_t) (pChunk->nBytes - nPos) ;
	}
	return	m_pfile->Read( ptrBuf, nBytes ) ;
}

// チャンク長取得
//////////////////////////////////////////////////////////////////////////////
uint32_t SGLWindowsAVIReader::GetLength( void ) const
{
	RIFF_CHUNK *	pChunk = m_arrChunk.GetLastAt() ;
	ESLAssert( pChunk != NULL ) ;
	if ( pChunk == NULL )
	{
		return	(uint32_t) m_pfile->GetLength() ;
	}
	return	pChunk->nBytes ;
}

// チャンクを開く
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowsAVIReader::DescendChunk( const char * pszChunkID )
{
	uint32_t	nChunk[2] ;
	ESLAssert( m_pfile != NULL ) ;
	if ( m_pfile == NULL )
	{
		return	sglErrFailed ;
	}
	for ( ; ; )
	{
		if ( m_pfile->Read( &nChunk[0], sizeof(uint32_t) * 2 ) < sizeof(uint32_t) * 2 )
		{
			return	sglErrFailed ;
		}
		if ( pszChunkID != NULL )
		{
			if ( !IsEqualChunkID( nChunk[0], pszChunkID ) )
			{
				if ( nChunk[1] & 0x80000000 )
				{
					return	sglErrFailed ;
				}
				m_pfile->Seek( ((nChunk[1] + 1) & ~ 0x01),
									SFileInterface::FromCurrent ) ;
				continue ;
			}
		}
		RIFF_CHUNK	chunk ;
		chunk.nChunkID = nChunk[0] ;
		chunk.nBytes = nChunk[1] ;
		chunk.nBeginPos = m_pfile->GetPosition( ) ;
		m_arrChunk.Add( chunk ) ;
		break ;
	}
	return	sglErrSuccess ;
}

// チャンクを抜ける
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowsAVIReader::AscendChunk( void )
{
	ESLAssert( m_pfile != NULL ) ;
	if ( m_pfile == NULL )
	{
		return	sglErrFailed ;
	}
	RIFF_CHUNK *	pChunk = m_arrChunk.GetLastAt() ;
	if ( pChunk == NULL )
	{
		return	sglErrFailed ;
	}
	m_pfile->Seek
		( pChunk->nBeginPos + ((pChunk->nBytes + 0x01) & ~0x01) ) ;
	//
	m_arrChunk.RemoveAt( m_arrChunk.GetLength() - 1 ) ;
	//
	return	sglErrSuccess ;
}

// 現在のチャンク名を取得する
//////////////////////////////////////////////////////////////////////////////
uint32_t SGLWindowsAVIReader::GetCurrentChunkID( void ) const
{
	RIFF_CHUNK *	pChunk = m_arrChunk.GetLastAt() ;
	ESLAssert( pChunk != NULL ) ;
	if ( pChunk == NULL )
	{
		return	0 ;
	}
	return	pChunk->nChunkID ;
}

// チャンクの一致判定
//////////////////////////////////////////////////////////////////////////////
bool SGLWindowsAVIReader::IsEqualChunkID( uint32_t nID, const char * pszID )
{
	return	(MAKEFOURCC( pszID[0], pszID[1], pszID[2], pszID[3] ) == nID) ;
}

// 映像フレームチャンクか？
//////////////////////////////////////////////////////////////////////////////
bool SGLWindowsAVIReader::IsVideoFrameChunkID( uint32_t nID )
{
	return	IsEqualChunkID( nID, "00dc" )
			|| IsEqualChunkID( nID, "00db" )
			|| IsEqualChunkID( nID, "01dc" )
			|| IsEqualChunkID( nID, "01db" ) ;
}

// 音声データチャンクか？
//////////////////////////////////////////////////////////////////////////////
bool SGLWindowsAVIReader::IsAudioFrameChunkID( uint32_t nID )
{
	return	IsEqualChunkID( nID, "00wb" )
			|| IsEqualChunkID( nID, "01wb" )
			|| IsEqualChunkID( nID, "02wb" )
			|| IsEqualChunkID( nID, "03wb" ) ;
}

// ストリームの次のデータまでシークして読み込む
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowsAVIReader::ReadNextStreamData
	( SSystem::SQueueBuffer & bufData,
		SGLWindowsAVIReader::AVIStream * pavis,
		AVIStream * pavisAnother, uint64_t * pAnotherBytes )
{
	SGLError	err = SeekNextStreamData( pavis, pavisAnother, pAnotherBytes ) ;
	if ( !err )
	{
		size_t	nBytes = (size_t) GetLength( ) ;
		nBytes = Read( bufData.PutBuffer(nBytes), nBytes ) ;
		bufData.FlushBuffer( nBytes ) ;
		AscendChunk( ) ;
		pavis->m_nNextPos = m_pfile->GetPosition( ) ;
	}
	return	err ;
}

// ストリームの次のデータまでシークする
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowsAVIReader::SeekNextStreamData
	( SGLWindowsAVIReader::AVIStream * pavis,
		SGLWindowsAVIReader::AVIStream * pavisAnother, uint64_t * pAnotherBytes )
{
	ESLAssert( m_pfile != NULL ) ;
	if ( m_pfile == NULL )
	{
		return	sglErrFailed ;
	}
	//
	// ストリームの位置までシーク
	//
	uint64_t			nPos = (uint32_t) m_pfile->GetPosition( ) ;
	const uint64_t *	pPosRIFFs = m_arrPosRIFF.GetConstArray() ;
	size_t				nCount = m_arrPosRIFF.GetLength() ;
	for ( size_t i = 1; i < nCount; i ++ )
	{
		if ( (pPosRIFFs[i - 1] < pavis->m_nNextPos)
					&& (pavis->m_nNextPos <= pPosRIFFs[i]) )
		{
			if ( (nPos <= pPosRIFFs[i - 1])
					|| (pPosRIFFs[i] < nPos) )
			{
				m_arrChunk.RemoveAll( ) ;
				m_pfile->Seek( pPosRIFFs[i - 1] ) ;
				if ( DescendRIFF_LIST_movie( ) )
				{
					return	sglErrFailed ;
				}
			}
		}
	}
	m_pfile->Seek( pavis->m_nNextPos ) ;
	//
	// 目的のチャンクを探す
	//
	if ( pAnotherBytes != NULL )
	{
		*pAnotherBytes = 0 ;
	}
	for ( ; ; )
	{
		for ( ; ; )
		{
			if ( DescendChunk() )
			{
				break ;
			}
			uint32_t	nChunkID = GetCurrentChunkID( ) ;
			bool		fFoundChunk = false ;
			bool		fAnotherChunk = false ;
			if ( IsVideoFrameChunkID( nChunkID ) )
			{
				if ( pavis->m_avish.fccType == streamtypeVIDEO )
				{
					fFoundChunk = true ;
				}
				if ( (pavisAnother != NULL)
					&& (pavisAnother->m_avish.fccType == streamtypeVIDEO) )
				{
					fAnotherChunk = true ;
				}
			}
			else if ( IsAudioFrameChunkID( nChunkID ) )
			{
				if ( pavis->m_avish.fccType == streamtypeAUDIO )
				{
					fFoundChunk = true ;
				}
				if ( (pavisAnother != NULL)
					&& (pavisAnother->m_avish.fccType == streamtypeAUDIO) )
				{
					fAnotherChunk = true ;
				}
			}
			if ( fFoundChunk )
			{
				return	sglErrSuccess ;
			}
			if ( fAnotherChunk && (pAnotherBytes != NULL) )
			{
				*pAnotherBytes += GetLength() ;
			}
			AscendChunk( ) ;
		}
		AscendChunk( ) ;		// LIST
		AscendChunk( ) ;		// RIFF
		//
		if ( DescendRIFF_LIST_movie() )
		{
			return	sglErrFailed ;
		}
	}
}

// LIST movi チャンクを開く
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowsAVIReader::DescendRIFF_LIST_movie( void )
{
	for ( ; ; )
	{
		if ( DescendChunk( "RIFF" ) )
		{
			return	sglErrFailed ;
		}
		do
		{
			uint32_t	nAVIX ;
			Read( &nAVIX, sizeof(uint32_t) ) ;
			if ( !IsEqualChunkID( nAVIX, "AVI " )
				&& !IsEqualChunkID( nAVIX, "AVIX" ) )
			{
				break ;
			}
			if ( DescendLIST_movie() == sglErrSuccess )
			{
				return	sglErrSuccess ;
			}
		}
		while ( false ) ;
		//
		AscendChunk( ) ;
	}
}

SGLError SGLWindowsAVIReader::DescendLIST_movie( void )
{
	for ( ; ; )
	{
		if ( DescendChunk( "LIST" ) )
		{
			break ;
		}
		uint32_t	nMOVI ;
		Read( &nMOVI, sizeof(uint32_t) ) ;
		if ( IsEqualChunkID( nMOVI, "movi" ) )
		{
			return	sglErrSuccess ;
		}
		AscendChunk( ) ;
	}
	return	sglErrFailed ;
}

// 映像フレームを展開するために画像バッファを生成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowsAVIReader::CreateDecompressVideoBuffer( void )
{
	if ( (m_psVideo == NULL) || (m_pbmihVideo == NULL) )
	{
		return	sglErrFailed ;
	}
	eslFillMemory( &m_bmiOut, 0, sizeof(BITMAPINFOHEADER) ) ;
	m_bmiOut.biSize = sizeof(BITMAPINFOHEADER) ;
	m_bmiOut.biWidth = m_pbmihVideo->biWidth ;
	m_bmiOut.biHeight = m_pbmihVideo->biHeight ;
	m_bmiOut.biBitCount = m_pbmihVideo->biBitCount ;
	m_bmiOut.biPlanes = 1 ;
	m_bmiOut.biCompression = BI_RGB ;
	m_bmiOut.biSizeImage =
		(((m_bmiOut.biWidth * m_bmiOut.biBitCount
							+ 0x1F) & ~0x1F) >> 3) * m_bmiOut.biHeight ;
	//
	m_bufVideoBuf.SetLength( m_bmiOut.biSizeImage ) ;
	m_bufTempBuf.SetLength( m_pbmihVideo->biSizeImage ) ;
	//
	if ( m_hicDecompress != NULL )
	{
		ICClose( m_hicDecompress ) ;
	}
	if ( m_pbmihVideo->biCompression != BI_RGB )
	{
		m_bmiOut.biBitCount = 24 ;
		m_hicDecompress =
			ICLocate( ICTYPE_VIDEO, m_psVideo->m_avish.fccHandler,
							m_pbmihVideo, &m_bmiOut, ICMODE_DECOMPRESS ) ;
		if ( m_hicDecompress == NULL )
		{
			ESLTrace( "Failed to create VCM for %08X\n", m_pbmihVideo->biCompression ) ;
		}
		else
		{
			ICDecompressBegin( m_hicDecompress, m_pbmihVideo, &m_bmiOut ) ;
		}
	}
	return	sglErrSuccess ;
}

// 映像フレームを展開
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowsAVIReader::DecompressVideoFrame
	( const SGLImageInfo& imginf,
			uint8_t * pbytBuf, SSystem::SQueueBuffer & bufData )
{
	size_t				nBytes = (size_t) bufData.GetLength() ;
	const uint8_t *		ptrData = bufData.GetBuffer( nBytes ) ;
	BITMAPINFOHEADER *	pbmih = m_pbmihVideo ;
	//
	if ( m_hicDecompress != NULL )
	{
		SGLError	err = DecompressVideoFrameVCM( (LPVOID) ptrData ) ;
		if ( err )
		{
			bufData.ReleaseBuffer( (ssize_t) nBytes ) ;
			return	err ;
		}
		pbmih = &m_bmiOut ;
		ptrData = m_bufVideoBuf.GetConstArray() ;
		nBytes = m_bufVideoBuf.GetLength() ;
	}
	SGLImageBuffer	imgbufSrc ;
	switch ( pbmih->biBitCount )
	{
	case	8:
	case	16:
	case	24:
		imgbufSrc.format = formatImageRGB ;
		break ;
	case	32:
		imgbufSrc.format = formatImageARGB ;
		break ;
	default:
		return	sglErrNotSupported ;
	}
	imgbufSrc.depth = pbmih->biBitCount ;
	imgbufSrc.width = pbmih->biWidth ;
	imgbufSrc.height = pbmih->biHeight ;
	imgbufSrc.pitchPixel = pbmih->biBitCount >> 3 ;
	imgbufSrc.pitchLine =
		((pbmih->biWidth * pbmih->biBitCount + 0x1F) & ~0x1F) >> 3 ;
	imgbufSrc.ptrBuffer = const_cast<uint8_t*>( ptrData ) ;
	//
	if ( (ssize_t)imgbufSrc.pitchLine * pbmih->biHeight > (ssize_t) nBytes )
	{
		imgbufSrc.height = (uint32_t) (nBytes / imgbufSrc.pitchLine) ;
		if ( imgbufSrc.height == 0 )
		{
			return	sglErrFailed ;
		}
	}
	imgbufSrc.ptrBuffer += imgbufSrc.pitchLine * (int32_t) (imgbufSrc.height - 1) ;
	imgbufSrc.pitchLine = - imgbufSrc.pitchLine ;
	//
	SGLImageBuffer	imgbufDst = imginf ;
	imgbufDst.ptrBuffer = pbytBuf ;
	//
	sglConvertImageBuffer( imgbufDst, imgbufSrc ) ;
	//
	m_bufVideoBuf.FinishArray() ;
	bufData.ReleaseBuffer( (ssize_t) nBytes ) ;
	return	sglErrSuccess ;
}

SGLError SGLWindowsAVIReader::DecompressVideoFrameVCM( LPVOID lpData )
{
	DWORD	dwResult = ICERR_ERROR ;
	__try
	{
		dwResult = ICDecompress
					( m_hicDecompress, 0,
						m_pbmihVideo, lpData,
						&m_bmiOut, m_bufVideoBuf.GetArray() ) ;
	}
	__except ( EXCEPTION_EXECUTE_HANDLER )
	{
		dwResult = ICERR_ERROR ;
	}
	if ( dwResult != ICERR_OK )
	{
		m_bufVideoBuf.FinishArray() ;
		return	sglErrFailed ;
	}
	m_bufVideoBuf.FinishArray() ;
	return	sglErrSuccess ;
}

// 音声データを展開して取得します
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowsAVIReader::DecompressAudioData
	( SSystem::SQueueBuffer & bufPCM, SSystem::SQueueBuffer & bufData )
{
	size_t			nBytes = (size_t) bufData.GetLength() ;
	const uint8_t *	ptrData = bufData.GetBuffer( nBytes ) ;
	//
	if ( m_hACMStream != NULL )
	{
		//
		// 変換フラグを初期化する
		//
		DWORD	fdwConversion = 0 ;
		if ( m_bufACMStream.GetLength() == 0 )
		{
			fdwConversion =
				m_flagAudioSeeked ?
					ACM_STREAMCONVERTF_START : ACM_STREAMCONVERTF_BLOCKALIGN ;
		}
		m_flagAudioSeeked = false ;
		//
		// ACM 入力バッファに入力音声バッファをコピーする
		//
		m_bufACMStream.Write( ptrData, nBytes ) ;
		bufData.ReleaseBuffer( (ssize_t) nBytes ) ;
		//
		// 必要な ACM 出力バッファのサイズを計算する
		//
		DWORD	dwDestinationBytes = 0 ;
		DWORD	dwNecessaryBytes = (DWORD) m_bufACMStream.GetLength() ;
		if ( dwNecessaryBytes == 0 )
		{
			return	sglErrSuccess ;
		}
		for ( ; ; )
		{
			::acmStreamSize
				( m_hACMStream, dwNecessaryBytes,
					&dwDestinationBytes, ACM_STREAMSIZEF_SOURCE ) ;
			if ( dwDestinationBytes > 0 )
			{
				break ;
			}
			dwNecessaryBytes <<= 1 ;
		}
		//
		// ACM 展開の準備
		//
		ACMSTREAMHEADER	acmsh ;
		::memset( &acmsh, 0, sizeof(ACMSTREAMHEADER) ) ;
		acmsh.cbStruct = sizeof(ACMSTREAMHEADER) ;
		//
		nBytes = (size_t) m_bufACMStream.GetLength() ;
		ptrData = m_bufACMStream.GetBuffer( nBytes ) ;
		acmsh.pbSrc = (LPBYTE) ptrData ;
		acmsh.cbSrcLength = (DWORD) nBytes ;
		acmsh.pbDst = (LPBYTE) bufPCM.PutBuffer( dwDestinationBytes ) ;
		acmsh.cbDstLength = dwDestinationBytes ;
		if ( ::acmStreamPrepareHeader( m_hACMStream, &acmsh, 0 ) )
		{
			ESLTrace( "acmStreamPrepareeader 関数が失敗しました。\n" ) ;
		}
		//
		// ACM 展開
		if ( ::acmStreamConvert
			( m_hACMStream, &acmsh, fdwConversion ) )
		{
			ESLTrace( "acmStreamConvert 関数が失敗しました。\n" ) ;
		}
		//
		// ACM バッファのパラメータを正規化する
		//
		m_bufACMStream.ReleaseBuffer( (ssize_t) acmsh.cbSrcLengthUsed ) ;
		bufPCM.FlushBuffer( (size_t) acmsh.cbDstLengthUsed ) ;
		//
		// 展開の終了処理
		//
		::acmStreamUnprepareHeader( m_hACMStream, &acmsh, 0 ) ;
	}
	else if ( m_flagNeedConversionPCM )
	{
		//
		// PCM 形式変換
		//
		const size_t	nSamples = (size_t) m_fmtAudio.BytesToSamples( nBytes ) ;
		const size_t	nDstBytes = (size_t) m_fmtAudioPCM.SamplesToBytes( nSamples ) ;
		uint8_t *		pbytDst = bufPCM.PutBuffer( nDstBytes ) ;
		//
		if ( (m_fmtAudio.format == formatSoundIEEEFloat)
			&& (m_fmtAudio.bitsPerSample == 32) )
		{
			sglEncodeSoundFrom32bitsPCM
				( m_fmtAudioPCM, pbytDst,
					(const float32_t*) ptrData,
					m_fmtAudio.channels, 1,
					nSamples, m_fmtAudio.channels ) ;
		}
		else if ( (m_fmtAudio.format == formatSoundLinearPCM)
				&& (m_fmtAudio.bitsPerSample == 32) )
		{
			sglEncodeSoundFrom32bitsPCM
				( m_fmtAudioPCM, pbytDst,
					(const int32_t*) ptrData,
					m_fmtAudio.channels, 1,
					nSamples, m_fmtAudio.channels ) ;
		}
		bufPCM.FlushBuffer( nDstBytes ) ;
		bufData.ReleaseBuffer( (ssize_t) nBytes ) ;
	}
	else
	{
		//
		// PCM コピー
		//
		bufPCM.Write( ptrData, nBytes ) ;
		bufData.ReleaseBuffer( (ssize_t) nBytes ) ;
	}
	return	sglErrSuccess ;
}

// フレーム位置をシーク
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowsAVIReader::SeekFrameFilePosition( uint64_t& nPos, uint64_t nFrame )
{
	if ( nFrame >= m_arrPosFrame.GetLength() )
	{
		ESLAssert( m_arrPosFrame.GetLength() >= 1 ) ;
		if ( m_arrPosFrame.GetLength() == 0 )
		{
			return	sglErrFailed ;
		}
		VideoFramePosInfo	vfpi = *(m_arrPosFrame.GetLastAt()) ;
		m_psVideo->m_nNextPos = vfpi.nFilePos ;
		m_nNextFrame = m_arrPosFrame.GetLength() - 1 ;
		m_nNextAudioPCMs = vfpi.nSoundPCMPos ;
		//
		do
		{
			uint64_t	nPassAudioBytes = 0 ;
			if ( SeekNextStreamData( m_psVideo, m_psAudio, &nPassAudioBytes ) )
			{
				return	sglErrFailed ;
			}
			AscendChunk( ) ;
			m_psVideo->m_nNextPos = m_pfile->GetPosition( ) ;
			//
			m_nNextFrame ++ ;
			if ( (m_pwfxAudio != NULL) && (m_pwfxAudio->nBlockAlign != 0) )
			{
				m_nNextAudioPCMs += nPassAudioBytes / m_pwfxAudio->nBlockAlign ;
			}
			ESLAssert( m_nNextFrame == m_arrPosFrame.GetLength() ) ;
			m_arrPosFrame.Add
				( VideoFramePosInfo( m_psVideo->m_nNextPos, m_nNextAudioPCMs ) ) ;
		}
		while ( nFrame >= m_arrPosFrame.GetLength() ) ;
	}
	if ( nFrame >= m_arrPosFrame.GetLength() )
	{
		return	sglErrFailed ;
	}
	VideoFramePosInfo	vfpi = m_arrPosFrame.At( (size_t) nFrame ) ;
	nPos = vfpi.nFilePos ;
	m_nNextAudioPCMs = vfpi.nSoundPCMPos ;
	return	sglErrSuccess ;
}

SGLError SGLWindowsAVIReader::SeekAudioFrameFilePosition
	( uint64_t& nPos, uint64_t nFrame, uint64_t nSamples )
{
	SGLError	err = SeekFrameFilePosition( nPos, nFrame ) ;
	if ( err )
	{
		return	err ;
	}
	size_t	iAudioFrame = (size_t) nFrame ;
	while ( iAudioFrame > 0 )
	{
		VideoFramePosInfo	vfpi = m_arrPosFrame.At( iAudioFrame ) ;
		if ( vfpi.nSoundPCMPos < nSamples )
		{
			break ;
		}
		iAudioFrame -- ;
	}
	if ( iAudioFrame != nFrame )
	{
		err = SeekFrameFilePosition( nPos, iAudioFrame ) ;
	}
	return	err ;
}

// キーフレームを検索
//////////////////////////////////////////////////////////////////////////////
size_t SGLWindowsAVIReader::FindKeyFrame( size_t iFrame ) const
{
	if ( (m_arrIndexEntries.GetLength() == 0)
		|| (m_arrVideoIndex.GetLength() == 0)
		|| (iFrame == 0)
		|| (iFrame >= m_arrVideoIndex.GetLength()) )
	{
		return	iFrame ;
	}
	ESLAssert( m_arrVideoIndex.At(0) < m_arrIndexEntries.GetLength() ) ;
	if ( !(m_arrIndexEntries.At(m_arrVideoIndex.At(0)).dwFlags & AVIIF_KEYFRAME) )
	{
		return	iFrame ;
	}
	while ( iFrame > 0 )
	{
		size_t	iChunk = m_arrVideoIndex.At( iFrame ) ;
		ESLAssert( iChunk < m_arrIndexEntries.GetLength() ) ;
		if ( m_arrIndexEntries.At(iChunk).dwFlags & AVIIF_KEYFRAME )
		{
			return	iFrame ;
		}
		iFrame -- ;
	}
	return	iFrame ;
}

// サウンド形式取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowsAVIReader::GetAudioFormat( SGLSoundFormat& fmt )
{
	if ( m_pwfxAudio == NULL )
	{
		return	sglErrFailed ;
	}
	fmt = m_fmtAudioPCM ;
	return	sglErrSuccess ;
}

// メディア補助情報取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowsAVIReader::GetAudioOptinalInfo( SGLMediaOptionalInfo& optinf )
{
	optinf.m_nFlags = 0 ;
	return	sglErrSuccess ;
}

// オーディオストリーム全長取得（未定は-1）[samples]
//////////////////////////////////////////////////////////////////////////////
int64_t SGLWindowsAVIReader::GetAudioLength( void ) const
{
	if ( m_psAudio == NULL )
	{
		return	0 ;
	}
	return	m_psAudio->m_avish.dwLength ;
}

// オーディオストリーム読み込み [samples]
//////////////////////////////////////////////////////////////////////////////
size_t SGLWindowsAVIReader::ReadAudio( void * ptrBuf, size_t nSamples )
{
	if ( (m_psAudio == NULL) || (m_pwfxAudio == NULL) )
	{
		return	0 ;
	}
	SQueueBuffer	bufData ;
	size_t			nReadSamples = 0 ;
	size_t			nBytes = nSamples * m_wfxAudioPCM.nBlockAlign ;
	while ( nBytes > 0 )
	{
		size_t	nReadBytes = m_bufPCMStream.Read( ptrBuf, nBytes ) ;
		ptrBuf = (((uint8_t*)ptrBuf) + nReadBytes) ;
		nReadSamples += nReadBytes / m_wfxAudioPCM.nBlockAlign ;
		if ( nBytes <= nReadBytes )
		{
			break ;
		}
		nBytes -= nReadBytes ;
		//
		if ( ReadNextStreamData( bufData, m_psAudio ) )
		{
			ESLTrace( "音声データの読み込みに失敗しました\n" ) ;
			break ;
		}
		if ( DecompressAudioData( m_bufPCMStream, bufData ) )
		{
			ESLTrace( "音声データの展開に失敗しました。\n" ) ;
			break ;
		}
	}
	return	nReadSamples ;
}

// オーディオストリーム位置変更
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowsAVIReader::SeekAudio( uint64_t nSamples )
{
	if ( (m_psAudio == NULL) || (m_pwfxAudio == NULL) )
	{
		return	sglErrFailed ;
	}
	int64_t	nTotalFrames = GetVideoLength() ;
	int64_t	nVideoDuration = GetVideoDuration() ;
	int64_t	nFrame = (nSamples * 1000 / m_pwfxAudio->nSamplesPerSec)
										* nTotalFrames / nVideoDuration ;
	//
	SGLError	err = SeekAudioFrameFilePosition
						( m_psAudio->m_nNextPos, nFrame, nSamples ) ;
	if ( err )
	{
		return	err ;
	}
	m_bufACMStream.ClearAll() ;
	m_flagAudioSeeked = true ;
	return	sglErrSuccess ;
}

// ビデオ画像形式取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowsAVIReader::GetImageFormat( SGLImageInfo& imginf )
{
	if ( m_pbmihVideo == NULL )
	{
		return	sglErrFailed ;
	}
	if ( m_bmiOut.biBitCount == 32 )
	{
		imginf.format = formatImageARGB ;
	}
	else
	{
		imginf.format = formatImageRGB ;
	}
	imginf.depth = m_bmiOut.biBitCount ;
	imginf.width = m_bmiOut.biWidth ;
	imginf.height = m_bmiOut.biHeight ;
	imginf.ptOrigin.x = 0 ;
	imginf.ptOrigin.y = 0 ;
	imginf.colorClip = 0 ;
	imginf.pitchPixel = (m_bmiOut.biBitCount + 0x07) >> 3 ;
	imginf.pitchLine = ((imginf.width * imginf.depth + 0x1F) & ~0x1F) >> 3 ;
	return	sglErrSuccess ;
}

// メディア補助情報取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowsAVIReader::GetVideoOptinalInfo( SGLMediaOptionalInfo& optinf )
{
	optinf.m_nFlags = 0 ;
	return	sglErrSuccess ;
}

// ビデオストリーム全長取得（未定は-1）[frames]
//////////////////////////////////////////////////////////////////////////////
int64_t SGLWindowsAVIReader::GetVideoLength( void ) const
{
	if ( m_psVideo != NULL )
	{
		return	m_psVideo->m_avish.dwLength ;
	}
	return	m_avimh.dwTotalFrames ;
}

// ビデオストリーム全長取得（未定は-1）[millisecond]
//////////////////////////////////////////////////////////////////////////////
int64_t SGLWindowsAVIReader::GetVideoDuration( void ) const
{
	if ( (m_psVideo != NULL) && (m_psVideo->m_avish.dwScale >= 1) )
	{
		return	(int64_t)m_psVideo->m_avish.dwLength
					* 1000 * m_psVideo->m_avish.dwScale
							/ m_psVideo->m_avish.dwRate ;
	}
	return	GetVideoLength() * m_avimh.dwMicroSecPerFrame / 1000 ;
}

// ビデオストリーム読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowsAVIReader::ReadFrame
	( const SGLImageInfo& imginf, uint8_t * pbytBuf )
{
	if ( m_psVideo == NULL )
	{
		return	sglErrFailed ;
	}
	SQueueBuffer	bufVideo ;
	uint64_t		nPassAudioBytes = 0 ;
	if ( ReadNextStreamData( bufVideo, m_psVideo, m_psAudio, &nPassAudioBytes ) )
	{
		return	sglErrFailed ;
	}
	if ( DecompressVideoFrame( imginf, pbytBuf, bufVideo ) )
	{
		ESLTrace( "映像フレームの展開に失敗しました。\n" ) ;
		return	sglErrFailed ;
	}
	m_nNextFrame ++ ;
	if ( (m_pwfxAudio != NULL) && (m_pwfxAudio->nBlockAlign != 0) )
	{
		m_nNextAudioPCMs += nPassAudioBytes / m_pwfxAudio->nBlockAlign ;
	}
	if ( m_nNextFrame >= m_arrPosFrame.GetLength() )
	{
		ESLAssert( m_nNextFrame == m_arrPosFrame.GetLength() ) ;
		m_arrPosFrame.Add
			( VideoFramePosInfo( m_psVideo->m_nNextPos, m_nNextAudioPCMs ) ) ;
	}
	return	sglErrSuccess ;
}

// ビデオストリーム位置変更
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowsAVIReader::SeekFrame( uint64_t nFrames )
{
	if ( m_psVideo == NULL )
	{
		return	sglErrFailed ;
	}
	if ( nFrames == m_nNextFrame )
	{
		return	sglErrSuccess ;
	}
	size_t	iKeyFrame = FindKeyFrame( (size_t) nFrames ) ;
	if ( iKeyFrame < (size_t) nFrames )
	{
		if ( (iKeyFrame > m_nNextFrame) || (nFrames < m_nNextFrame) )
		{
			// キーフレームへシーク
			SGLError	err = SeekFrameFilePosition( m_psVideo->m_nNextPos, iKeyFrame ) ;
			if ( err )
			{
				return	err ;
			}
			m_nNextFrame = iKeyFrame ;
		}
		// デコードのためのバッファ準備
		if ( m_imgTempBuf.GetImageSize() != SGLSize( m_bmiOut.biWidth, m_bmiOut.biHeight ) )
		{
			m_imgTempBuf.CreateImage
				( m_bmiOut.biWidth, m_bmiOut.biHeight, formatImageRGB, 32 ) ;
		}
		SGLImageInfo	imginf ;
		uint8_t *		pbytBuf = m_imgTempBuf.LockBuffer( imginf, SGLImageObject::lockWrite ) ;
		//
		// 指定フレーム直前まで順次デコード
		SGLError		err = sglErrSuccess ;
		STimeCounter	timer ;
		while ( nFrames > m_nNextFrame )
		{
			err = ReadFrame( imginf, pbytBuf ) ;
			if ( err )
			{
				break ;
			}
			if ( timer.GetTime() > 500 )
			{
				break ;
			}
		}
		m_imgTempBuf.UnlockBuffer( SGLImageObject::lockWrite ) ;
		if ( nFrames < m_nNextFrame )
		{
			err = SeekFrameFilePosition( m_psVideo->m_nNextPos, nFrames ) ;
		}
		return	err ;
	}
	else
	{
		// 指定フレームはキーフレームなので直接シーク
		SGLError	err = SeekFrameFilePosition( m_psVideo->m_nNextPos, nFrames ) ;
		if ( err )
		{
			return	err ;
		}
		m_nNextFrame = (size_t) nFrames ;
	}
	return	sglErrSuccess ;
}



//////////////////////////////////////////////////////////////////////////////
// Window Wave Form Audio ファイル出力
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLWindowWaveFileWriter, SGLAudioOutputStream )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLWindowWaveFileWriter::SGLWindowWaveFileWriter( void )
{
	m_pfile = NULL ;
	m_flagAutoDelFile = false ;
	m_flagWrittenFormat = false ;
	m_flagDescendChunk = false ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLWindowWaveFileWriter::~SGLWindowWaveFileWriter( void )
{
	Close() ;
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowWaveFileWriter::Open( const wchar_t * pwszFilePath )
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

SGLError SGLWindowWaveFileWriter::Open
	( SSystem::SFileInterface * pfile, bool flagAutoDelete )
{
	Close() ;
	//
	// RIFF ヘッダ
	//
	WAVE_RIFF_HEADER	wrhdr ;
	wrhdr.idRIFF[0] = (uint8_t) 'R' ;
	wrhdr.idRIFF[1] = (uint8_t) 'I' ;
	wrhdr.idRIFF[2] = (uint8_t) 'F' ;
	wrhdr.idRIFF[3] = (uint8_t) 'F' ;
	wrhdr.nFileLength = 0 ;
	wrhdr.idWAVE[0] = (uint8_t) 'W' ;
	wrhdr.idWAVE[1] = (uint8_t) 'A' ;
	wrhdr.idWAVE[2] = (uint8_t) 'V' ;
	wrhdr.idWAVE[3] = (uint8_t) 'E' ;
	//
	if ( pfile->Write
		( &wrhdr, sizeof(WAVE_RIFF_HEADER) ) < sizeof(WAVE_RIFF_HEADER) )
	{
		if ( flagAutoDelete )
		{
			delete	pfile ;
		}
		return	sglErrFailed ;
	}
	m_pfile = pfile ;
	m_flagAutoDelFile = flagAutoDelete ;
	m_flagWrittenFormat = false ;
	m_flagDescendChunk = false ;
	return	sglErrSuccess ;
}

// ファイルを閉じる
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowWaveFileWriter::Close( void )
{
	if ( m_flagDescendChunk )
	{
		AscendChunk() ;
	}
	if ( m_pfile != NULL )
	{
		uint32_t	nFileLength = (uint32_t) m_pfile->GetLength() - 8 ;
		m_pfile->Seek( 4 ) ;
		m_pfile->Write( &nFileLength, sizeof(uint32_t) ) ;
		m_pfile->Seek( m_pfile->GetLength() ) ;
	}
	if ( m_flagAutoDelFile )
	{
		delete	m_pfile ;
	}
	m_flagAutoDelFile = false ;
	m_flagWrittenFormat = false ;
	m_pfile = NULL ;
	return	sglErrSuccess ;
}

// チャンク生成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowWaveFileWriter::DescendChunk( const char * pChunkID )
{
	if ( m_pfile == NULL )
	{
		return	sglErrFailed ;
	}
	RIFF_CHUNK_HEADER	rchdr ;
	rchdr.idChunk[0] = (uint8_t) pChunkID[0] ;
	rchdr.idChunk[1] = (uint8_t) pChunkID[1] ;
	rchdr.idChunk[2] = (uint8_t) pChunkID[2] ;
	rchdr.idChunk[3] = (uint8_t) pChunkID[3] ;
	rchdr.nLength = 0 ;
	if ( m_pfile->Write
		( &rchdr, sizeof(RIFF_CHUNK_HEADER) ) < sizeof(RIFF_CHUNK_HEADER) )
	{
		return	sglErrFailed ;
	}
	m_flagDescendChunk = true ;
	m_posChunkBase = m_pfile->GetPosition() ;
	return	sglErrSuccess ;
}

// チャンク終了
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowWaveFileWriter::AscendChunk( void )
{
	if ( m_pfile == NULL )
	{
		return	sglErrFailed ;
	}
	if ( m_flagDescendChunk )
	{
		int64_t		posEnd = m_pfile->GetPosition() ;
		uint32_t	nLength = (uint32_t) (posEnd - m_posChunkBase) ;
		m_pfile->Seek( m_posChunkBase - 4 ) ;
		m_pfile->Write( &nLength, sizeof(uint32_t) ) ;
		m_pfile->Seek( posEnd ) ;
		m_flagDescendChunk = false ;
	}
	return	sglErrSuccess ;
}

// オーディオ出力ストリーム準備
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowWaveFileWriter::PrepareAudio
	( const SGLSoundFormat& fmt, int64_t nSamples,
			const SGLMediaOptionalInfo * pOptInf )
{
	if ( m_pfile == NULL )
	{
		return	sglErrFailed ;
	}
	if ( m_flagWrittenFormat )
	{
		return	sglErrFailed ;
	}
	SGLError	err = DescendChunk( "fmt " ) ;
	if ( err )
	{
		return	err ;
	}
	//
	m_wfmt.wFormatTag = formatWavePCM ;
	m_wfmt.wChannels = (uint16_t) fmt.channels ;
	m_wfmt.nFrequency = fmt.frequency ;
	m_wfmt.wBitsPerSample = (uint16_t) fmt.bitsPerSample ;
	m_wfmt.wBlockAlign = m_wfmt.wChannels * m_wfmt.wBitsPerSample / 8 ;
	m_wfmt.nBytesPerSec = m_wfmt.nFrequency * m_wfmt.wBlockAlign ;
	m_pfile->Write( &m_wfmt, sizeof(WAVEFORMAT) ) ;
	//
	AscendChunk() ;
	//
	err = DescendChunk( "data" ) ;
	if ( err )
	{
		return	err ;
	}
	m_flagWrittenFormat = true ;
	return	sglErrSuccess ;
}

// オーディオストリーム出力 [samples]
//////////////////////////////////////////////////////////////////////////////
size_t SGLWindowWaveFileWriter::WriteAudio
			( const void * ptrBuf, size_t nSamples )
{
	if ( m_pfile == NULL )
	{
		return	sglErrFailed ;
	}
	if ( !m_flagWrittenFormat )
	{
		return	sglErrFailed ;
	}
	size_t	nBytes = nSamples * m_wfmt.wBlockAlign ;
	nBytes = m_pfile->Write( ptrBuf, nBytes ) ;
	if ( m_wfmt.wBlockAlign == 0 )
	{
		return	0 ;
	}
	return	nBytes / m_wfmt.wBlockAlign ;
}



//////////////////////////////////////////////////////////////////////////////
// Windows AVI ファイル出力ラッパー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLWindowsAVIWriter, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLWindowsAVIWriter::SGLWindowsAVIWriter( void )
{
	m_pavif = NULL ;
	m_psVideo = NULL ;
	m_psEditTemp = NULL ;
	m_psCmpTemp = NULL ;
	m_psAudio = NULL ;
	m_fCmpVars = false ;
	//
	AVIFileInit() ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLWindowsAVIWriter::~SGLWindowsAVIWriter( void )
{
	Close() ;
	//
	if ( m_fCmpVars )
	{
		ICCompressorFree( &m_cmpvars ) ;
		m_fCmpVars = false ;
	}
	AVIFileExit() ;
}

// 圧縮オプションの選択
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowsAVIWriter::CompressorChoose
	( SGLAbstractWindow * pWindow,
			const wchar_t * pwszCaption, const SGLImageInfo& imginf )
{
	COMPRESSOR_CHOOSE_PARAM	ccp ;
	SArray<char>			bufCaption ;
	ccp.pWriter = this ;
	ccp.hWnd = NULL ;
	ccp.pszCaption = NULL ;
	ToBitmapInfoHeader( ccp.bmih, imginf ) ;
	//
	if ( pwszCaption != NULL )
	{
		ccp.pszCaption =
			(char*) SString(pwszCaption).EncodeDefaultTo( bufCaption ) ;
	}
	if ( !m_fCmpVars )
	{
		memset( &m_cmpvars, 0, sizeof(COMPVARS) ) ;
		m_cmpvars.cbSize = sizeof(COMPVARS) ;
		m_cmpvars.dwFlags = ICMF_COMPVARS_VALID ;
		m_cmpvars.fccHandler = comptypeDIB ;
		m_cmpvars.lQ = ICQUALITY_DEFAULT ;
	}
	if ( pWindow != NULL )
	{
		ccp.hWnd = pWindow->GetWindowHandle() ;
		//
		SProcedureCaller	procChoose
			( &SGLWindowsAVIWriter::CallCompressorChoose, &ccp ) ;
		SSyncProcedure		syncProc( &procChoose, false ) ;
		//
		if ( !pWindow->PostUIThread( &syncProc ) )
		{
			syncProc.WaitDone() ;
			m_fCmpVars = true ;
			return	ccp.err ;
		}
		return	sglErrFailed ;
	}
	else
	{
		if ( !ICCompressorChoose
			( NULL, ICMF_CHOOSE_DATARATE | ICMF_CHOOSE_KEYFRAME,
					&ccp.bmih, NULL, &m_cmpvars, ccp.pszCaption ) )
		{
			return	sglErrFailed ;
		}
		m_fCmpVars = true ;
	}
	return	sglErrSuccess ;
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowsAVIWriter::Open( const wchar_t * pwszFilePath )
{
	SString	strFilePath = pwszFilePath ;
	if ( AVIFileOpen
		( &m_pavif, strFilePath.ToCharArray().GetConstArray(),
			OF_CREATE | OF_WRITE | OF_SHARE_DENY_NONE, NULL ) != AVIERR_OK )
	{
		return sglErrFailed ;
	}
	return	sglErrSuccess ;
}

// ファイルを閉じる
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowsAVIWriter::Close( void )
{
	if ( m_psVideo != NULL )
	{
		PAVISTREAM	psEdit = NULL ;
		if ( CreateEditableStream( &psEdit, m_psVideo ) == AVIERR_OK )
		{
			AVISTREAMINFO	si ;
			AVIStreamInfo( psEdit, &si, sizeof(AVISTREAMINFO) ) ;
			si.dwLength =
				(DWORD) ((uint64_t) m_iNextFrame * si.dwRate / si.dwScale) ;
			EditStreamSetInfo( psEdit, &si, sizeof(AVISTREAMINFO) ) ;
			AVIStreamRelease( psEdit ) ;
		}
		//
		AVIStreamRelease( m_psVideo ) ;
		m_psVideo = NULL ;
	}
	if ( m_psEditTemp != NULL )
	{
		AVIStreamRelease( m_psEditTemp ) ;
		m_psEditTemp = NULL ;
	}
	if ( m_psCmpTemp != NULL )
	{
		AVIStreamRelease( m_psCmpTemp ) ;
		m_psCmpTemp = NULL ;
	}
	if ( m_psAudio != NULL )
	{
		PAVISTREAM	psEdit = NULL ;
		if ( CreateEditableStream( &psEdit, m_psAudio ) == AVIERR_OK )
		{
			AVISTREAMINFO	si ;
			AVIStreamInfo( psEdit, &si, sizeof(AVISTREAMINFO) ) ;
			si.dwLength = m_iNextPCM ;
			EditStreamSetInfo( psEdit, &si, sizeof(AVISTREAMINFO) ) ;
			AVIStreamRelease( psEdit ) ;
		}
		//
		AVIStreamRelease( m_psAudio ) ;
		m_psAudio = NULL ;
	}
	if ( m_pavif != NULL )
	{
		AVIFileRelease( m_pavif ) ;
		m_pavif = NULL ;
	}
	return	sglErrSuccess ;
}

// オーディオ出力ストリーム準備
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowsAVIWriter::PrepareAudio
	( const SGLSoundFormat& fmt,
		int64_t nSamples, const SGLMediaOptionalInfo * pOptInf )
{
	if ( m_pavif == NULL )
	{
		return	sglErrFailed ;
	}
	memset( &m_wfx, 0, sizeof(WAVEFORMATEX) ) ;
	m_wfx.wFormatTag = WAVE_FORMAT_PCM ;
	m_wfx.nChannels = (WORD) fmt.channels ;
	m_wfx.nSamplesPerSec = (DWORD) fmt.frequency ;
	m_wfx.nBlockAlign =
			(WORD) (fmt.channels * fmt.bitsPerSample) >> 3 ;
	m_wfx.nAvgBytesPerSec =
			(DWORD) (fmt.frequency * m_wfx.nBlockAlign) ;
	m_wfx.wBitsPerSample = (WORD) fmt.bitsPerSample ;
	m_wfx.cbSize = 0 ;
	//
	AVISTREAMINFO	si ;
	memset( &si, 0, sizeof(AVISTREAMINFO) ) ;
	si.fccType = streamtypeAUDIO ;
	si.fccHandler = AVICOMPRESSF_INTERLEAVE ;
	si.dwScale = m_wfx.nBlockAlign ;
	si.dwRate = m_wfx.nAvgBytesPerSec ;
	si.dwLength = ((nSamples < 0) ? 0 : (DWORD) nSamples) ;
	si.dwQuality = (DWORD) -1 ;
	si.dwSampleSize = m_wfx.nBlockAlign ;
	strcpy_s( si.szName, sizeof(si.szName), "Audio stream" ) ;
	//
	if ( AVIFileCreateStream( m_pavif, &m_psAudio, &si ) != AVIERR_OK )
	{
		return	sglErrFailed ;
	}
	//
	if ( AVIStreamSetFormat
		( m_psAudio, 0, &m_wfx, sizeof(WAVEFORMATEX) ) != AVIERR_OK )
	{
		return	sglErrFailed ;
	}
	m_iNextPCM = 0 ;
	return	sglErrSuccess ;
}

// オーディオストリーム出力 [samples]
//////////////////////////////////////////////////////////////////////////////
size_t SGLWindowsAVIWriter::WriteAudio( const void * ptrBuf, size_t nSamples )
{
	if ( m_psAudio == NULL )
	{
		return	0 ;
	}
	LONG	nBytes = (LONG) nSamples * m_wfx.nBlockAlign ;
	LONG	nWrittenSamples = 0 ;
	if ( AVIStreamWrite
		( m_psAudio, m_iNextPCM, (LONG) nSamples,
			(LPVOID) ptrBuf, nBytes,
			AVIIF_KEYFRAME, &nWrittenSamples, NULL ) != AVIERR_OK )
	{
		return	0 ;
	}
	m_iNextPCM += nWrittenSamples ;
	return	(size_t) nWrittenSamples ;
}

// ビデオ画像形式取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowsAVIWriter::PrepareVideo
	( const SGLImageInfo& imginf,
		int64_t nFrames, int64_t nDuration,
		const SGLMediaOptionalInfo * pOptInf )
{
 	if ( m_pavif == NULL )
	{
		return	sglErrFailed ;
	}
	AVISTREAMINFO	si ;
	memset( &si, 0, sizeof(AVISTREAMINFO) ) ;
	si.fccType = streamtypeVIDEO ;
	si.fccHandler = m_fCmpVars ? m_cmpvars.fccHandler : comptypeDIB ;
	si.dwScale = 0x10000 ;
	si.dwRate = (DWORD) (nFrames * (0x10000 * 1000) / nDuration) ;
	si.dwLength = 0 ;
	si.dwQuality = (DWORD) -1 ;
	si.rcFrame.right = imginf.width ;
	si.rcFrame.bottom = imginf.height ;
	strcpy_s( si.szName, sizeof(si.szName), "Video stream" ) ;
	//
	if ( AVIFileCreateStream( m_pavif, &m_psVideo, &si ) != AVIERR_OK )
	{
		return	sglErrFailed ;
	}
	//
	if ( m_fCmpVars )
	{
		AVICOMPRESSOPTIONS	opt ;
		memset( &opt, 0, sizeof(AVICOMPRESSOPTIONS) ) ;
		opt.fccType = streamtypeVIDEO ;
		opt.fccHandler = m_cmpvars.fccHandler ;
		opt.dwKeyFrameEvery = m_cmpvars.lKey ;
		opt.dwQuality = m_cmpvars.lQ ;
		opt.dwBytesPerSecond = m_cmpvars.lDataRate;
		opt.dwFlags = (m_cmpvars.lDataRate > 0 ? AVICOMPRESSF_DATARATE : 0)
						| (m_cmpvars.lKey > 0 ? AVICOMPRESSF_KEYFRAMES : 0) ;
		opt.lpFormat = NULL ;
		opt.cbFormat = 0 ;
		opt.lpParms = m_cmpvars.lpState ;
		opt.cbParms = m_cmpvars.cbState ;
		opt.dwInterleaveEvery = 0 ;
		//
		PAVISTREAM	psTemp ;
		if ( AVIMakeCompressedStream
			( &psTemp, m_psVideo, &opt, NULL ) != AVIERR_OK )
		{
			return sglErrFailed ;
		}
		m_psCmpTemp = m_psVideo ;
		m_psVideo = psTemp ;
	}
	//
	ToBitmapInfoHeader( m_bmih, imginf ) ;
	if ( AVIStreamSetFormat
		( m_psVideo, 0, &m_bmih, sizeof(BITMAPINFOHEADER) ) != AVIERR_OK )
	{
		return	sglErrFailed ;
	}
	//
	m_iNextFrame = 0 ;
	return	sglErrSuccess ;
}

// ビデオストリーム出力
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowsAVIWriter::WriteFrame
	( const SGLImageInfo& imginf, const uint8_t * pbytImage )
{
	if ( m_psVideo == NULL )
	{
		return	sglErrFailed ;
	}
	BITMAPINFOHEADER	bmih ;
	ToBitmapInfoHeader( bmih, imginf ) ;
	//
	uint8_t *	pbytBuf = m_bufPixels.GetArray( bmih.biSizeImage ) ;
	size_t		nLineBytes = bmih.biSizeImage / bmih.biHeight ;
	uint8_t *	pbytNext = pbytBuf + (nLineBytes * (bmih.biHeight - 1)) ;
	size_t		nLinePixelBytes = (imginf.width * imginf.depth + 0x07) >> 3 ;
	//
	for ( LONG y = 0; y < bmih.biHeight; y ++ )
	{
		memmove( pbytNext, pbytImage, nLinePixelBytes ) ;
		pbytNext -= nLineBytes ;
		pbytImage += imginf.pitchLine ;
	}
	m_bufPixels.FinishArray() ;
	//
	if ( AVIStreamWrite
		( m_psVideo, m_iNextFrame, 1, pbytBuf, bmih.biSizeImage,
							AVIIF_KEYFRAME, NULL, NULL ) != AVIERR_OK )
	{
		return	sglErrFailed ;
	}
	m_iNextFrame ++ ;
	return	sglErrSuccess ;
}

// 圧縮オプション UI 表示パラメータ
//////////////////////////////////////////////////////////////////////////////
void SGLWindowsAVIWriter::CallCompressorChoose( void * pInstance )
{
	::AVIFileInit() ;
	//
	COMPRESSOR_CHOOSE_PARAM *	pccp = (COMPRESSOR_CHOOSE_PARAM*) pInstance ;
	pccp->err = sglErrSuccess ;
	if ( !ICCompressorChoose
		( pccp->hWnd, ICMF_CHOOSE_DATARATE | ICMF_CHOOSE_KEYFRAME,
				&(pccp->bmih), NULL,
				&(pccp->pWriter->m_cmpvars), pccp->pszCaption ) )
	{
		pccp->err = sglErrFailed ;
	}
	::AVIFileExit() ;
}

// BITMAPINFOHEADER へ変換
//////////////////////////////////////////////////////////////////////////////
void SGLWindowsAVIWriter::ToBitmapInfoHeader
	( BITMAPINFOHEADER& bmih, const SGLImageInfo& imginf )
{
	memset( &bmih, 0, sizeof(BITMAPINFOHEADER) ) ;
	bmih.biSize = sizeof(BITMAPINFOHEADER) ;
	bmih.biWidth = (LONG) imginf.width ;
	bmih.biHeight = (LONG) imginf.height ;
	bmih.biPlanes = 1 ;
	bmih.biBitCount = imginf.depth ;
	bmih.biCompression = BI_RGB ;
	bmih.biSizeImage =
		(((imginf.width * imginf.depth + 0x1F) >> 5) << 2) * imginf.height ;
}



//////////////////////////////////////////////////////////////////////////////
// ウィンドウ AVI キャプチャー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLWindowsAVICapture, SGLSyncWindowCapture )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLWindowsAVICapture::SGLWindowsAVICapture( void )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLWindowsAVICapture::~SGLWindowsAVICapture( void )
{
}

// キャプチャー開始
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowsAVICapture::BeginCapture
	( SGLWindowSprite * pWindow,
		const wchar_t * pwszAviPath,
		bool flagCompressOpt,
		uint32_t nFlags, size_t nFramesPerSec,
		const SGLSoundFormat& fmtSoundOut )
{
	HWND	hWnd = pWindow->GetWindowHandle() ;
	RECT	rectClient ;
	::GetClientRect( hWnd, &rectClient ) ;
	//
	SGLImageInfo	imginf ;
	imginf.format = formatImageRGB ;
	imginf.depth = 24 ;
	imginf.width = (uint32_t) (rectClient.right - rectClient.left) ;
	imginf.height = (uint32_t) (rectClient.bottom - rectClient.top) ;
	imginf.pitchPixel = imginf.depth / 8 ;
	imginf.pitchLine = imginf.width * imginf.pitchPixel ;
	//
	if ( flagCompressOpt )
	{
		m_aviWriter.CompressorChoose( pWindow, L"圧縮選択", imginf ) ;
	}
	if ( m_aviWriter.Open( pwszAviPath ) )
	{
		return	sglErrFailed ;
	}
	if ( m_aviWriter.PrepareAudio( fmtSoundOut ) )
	{
		m_aviWriter.Close() ;
		return	sglErrFailed ;
	}
	if ( m_aviWriter.PrepareVideo( imginf, nFramesPerSec, 1000 ) )
	{
		m_aviWriter.Close() ;
		return	sglErrFailed ;
	}
	AttachTargetWindow( pWindow ) ;
	AttachOutputStream( &m_aviWriter, &m_aviWriter ) ;
	//
	if ( SGLSyncWindowCapture::BeginCapture
				( nFlags, 1000 / nFramesPerSec, imginf, fmtSoundOut ) )
	{
		m_aviWriter.Close() ;
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

// キャプチャー終了
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowsAVICapture::EndCapture( void )
{
	SGLSyncWindowCapture::EndCapture() ;
	m_aviWriter.Close() ;
	return	sglErrSuccess ;
}

