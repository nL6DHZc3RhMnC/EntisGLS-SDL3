
#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl_media.h>
#include <sakuragl/media/sgl_wav_audio_decoder.h>
#include <math.h>

using namespace SSystem ;
using namespace SakuraGL ;


#if	defined(__PLATFORM_WINDOWS__)

//////////////////////////////////////////////////////////////////////////////
// ACM デコードストリーム
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLWaveACMDecodeStream, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLWaveACMDecodeStream::SGLWaveACMDecodeStream( void )
{
	m_hACMStream = NULL ;
	m_flagStartBlock = false ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLWaveACMDecodeStream::~SGLWaveACMDecodeStream( void )
{
	CloseStream() ;
}

// デコード準備
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWaveACMDecodeStream::PrepareStream( const WAVEFORMATEX * pwfxSrc )
{
	CloseStream() ;
	//
	m_wfxAudioPCM = *pwfxSrc ;
	if ( m_wfxAudioPCM.wFormatTag != WAVE_FORMAT_PCM )
	{
		m_wfxAudioPCM.wFormatTag = WAVE_FORMAT_PCM ;
		m_wfxAudioPCM.wBitsPerSample = 16 ;
		m_wfxAudioPCM.nBlockAlign = m_wfxAudioPCM.nChannels * 2 ;
		m_wfxAudioPCM.nAvgBytesPerSec =
				m_wfxAudioPCM.nSamplesPerSec * m_wfxAudioPCM.nBlockAlign ;
		m_wfxAudioPCM.cbSize = 0 ;
		//
		if ( ::acmStreamOpen
			( &m_hACMStream, NULL,
				(WAVEFORMATEX*) pwfxSrc, &m_wfxAudioPCM,
				NULL, 0, 0, ACM_STREAMOPENF_NONREALTIME ) )
		{
			ESLTrace( "Failed to Open ACM stream.\n" ) ;
			return	sglErrFailed ;
		}
		m_flagStartBlock = true ;
	}
	return	sglErrSuccess ;
}

// デーコード処理終了
//////////////////////////////////////////////////////////////////////////////
void SGLWaveACMDecodeStream::CloseStream( void )
{
	FlushStream() ;
	//
	if ( m_hACMStream != NULL )
	{
		acmStreamClose( m_hACMStream, 0 ) ;
		m_hACMStream = NULL ;
	}
}

// ストリームバッファの初期化
//////////////////////////////////////////////////////////////////////////////
void SGLWaveACMDecodeStream::FlushStream( void )
{
	m_bufACMStream.ClearAll() ;
	m_bufPCMStream.ClearAll() ;
	m_flagStartBlock = true ;
}

// デコード出力フォーマット
//////////////////////////////////////////////////////////////////////////////
const WAVEFORMATEX& SGLWaveACMDecodeStream::GetOutputFormat( void ) const
{
	return	m_wfxAudioPCM ;
}

// ソースデータ追加
//////////////////////////////////////////////////////////////////////////////
size_t SGLWaveACMDecodeStream::WriteCompressedAudio( const void * ptrData, size_t nBytes )
{
	if ( m_hACMStream != NULL )
	{
		//
		// 変換フラグを初期化する
		//
		DWORD	fdwConversion ;
		if ( m_flagStartBlock )
		{
			fdwConversion = ACM_STREAMCONVERTF_START ;
			m_flagStartBlock = false ;
		}
		else
		{
			fdwConversion = ACM_STREAMCONVERTF_BLOCKALIGN ;
		}
		//
		// 入力データ取得
		//
		m_bufACMStream.Write( ptrData, nBytes ) ;
		//
		size_t			nSrcBytes = (size_t) m_bufACMStream.GetLength() ;
		const uint8_t *	ptrSrcData = m_bufACMStream.GetBuffer( nBytes ) ;
		//
		// 必要な ACM 出力バッファのサイズを計算する
		//
		DWORD	dwDestinationBytes = 0 ;
		DWORD	dwNecessaryBytes = (DWORD) nSrcBytes ;
		if ( dwNecessaryBytes == 0 )
		{
			return	nBytes ;
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
			if ( dwNecessaryBytes == 0 )
			{
				return	nBytes ;
			}
		}
		//
		// ACM 展開の準備
		//
		ACMSTREAMHEADER	acmsh ;
		::memset( &acmsh, 0, sizeof(ACMSTREAMHEADER) ) ;
		acmsh.cbStruct = sizeof(ACMSTREAMHEADER) ;
		//
		acmsh.pbSrc = (LPBYTE) ptrSrcData ;
		acmsh.cbSrcLength = (DWORD) nSrcBytes ;
		acmsh.pbDst = (LPBYTE) m_bufPCMStream.PutBuffer( dwDestinationBytes ) ;
		acmsh.cbDstLength = dwDestinationBytes ;
		if ( ::acmStreamPrepareHeader( m_hACMStream, &acmsh, 0 ) )
		{
			ESLTrace( "failed to acmStreamPrepareeader.\n" ) ;
		}
		//
		// ACM 展開
		if ( ::acmStreamConvert
			( m_hACMStream, &acmsh, fdwConversion ) )
		{
			ESLTrace( "failed to acmStreamConvert.\n" ) ;
		}
		//
		// ACM バッファのパラメータを正規化する
		//
		m_bufACMStream.ReleaseBuffer( (ssize_t) acmsh.cbSrcLengthUsed ) ;
		m_bufPCMStream.FlushBuffer( (size_t) acmsh.cbDstLengthUsed ) ;
		//
		// 展開の終了処理
		//
		::acmStreamUnprepareHeader( m_hACMStream, &acmsh, 0 ) ;
		//
		return	nBytes ;
	}
	else
	{
		return	m_bufPCMStream.Write( ptrData, nBytes ) ;
	}
}

// 待ち行列のデコード済みデータサイズ取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLWaveACMDecodeStream::GetDecompressedBytes( void ) const
{
	return	(size_t) m_bufACMStream.GetLength() ;
}

// デコードデータ取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLWaveACMDecodeStream::ReadWaveData( void * ptrData, size_t nBytes )
{
	return	m_bufACMStream.Read( ptrData, nBytes ) ;
}

// デコードサイズ見積もり
//////////////////////////////////////////////////////////////////////////////
size_t SGLWaveACMDecodeStream::EstimateDecodedSize( size_t nSrcBytes ) const
{
	if ( m_hACMStream != NULL )
	{
		DWORD	dwDestinationBytes = 0 ;
		DWORD	dwNecessaryBytes = (DWORD) nSrcBytes ;
		::acmStreamSize
			( m_hACMStream, dwNecessaryBytes,
				&dwDestinationBytes, ACM_STREAMSIZEF_SOURCE ) ;
		return	(size_t) dwDestinationBytes ;
	}
	else
	{
		return	nSrcBytes ;
	}
}

// 入力サイズ見積もり
//////////////////////////////////////////////////////////////////////////////
size_t SGLWaveACMDecodeStream::EstimateSourceSize( size_t nDstBytes ) const
{
	if ( m_hACMStream != NULL )
	{
		DWORD	dwSourceBytes = 0 ;
		DWORD	dwDestinationBytes = (DWORD) nDstBytes ;
		::acmStreamSize
			( m_hACMStream, dwDestinationBytes,
				&dwSourceBytes, ACM_STREAMSIZEF_DESTINATION ) ;
		return	(size_t) dwSourceBytes ;
	}
	else
	{
		return	nDstBytes ;
	}
}

#endif



//////////////////////////////////////////////////////////////////////////////
// Windows Wave Form オーディオ・デコーダー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLWaveFormAudioDecoder, SGLAudioDecoderInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLWaveFormAudioDecoder::SGLWaveFormAudioDecoder( void )
{
	m_pFile = NULL ;
	m_pwfx = NULL ;
	m_flagCompressed = false ;
	m_flagFileOwner = false ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLWaveFormAudioDecoder::~SGLWaveFormAudioDecoder( void )
{
	if ( m_flagFileOwner )
	{
		delete	m_pFile ;
	}
	if ( m_pwfx != NULL )
	{
		esl_free( m_pwfx ) ;
		m_pwfx = NULL ;
	}
	m_flagFileOwner = false ;
	m_pFile = NULL ;
}

// ファイル拡張子判定
//////////////////////////////////////////////////////////////////////////////
bool SGLWaveFormAudioDecoder::IsMatchableFileExtension( const wchar_t * pszExt )
{
	return	(SString::CompareNoCase( pszExt, L"wav" ) == 0) ;
}

// デコーダー生成
//////////////////////////////////////////////////////////////////////////////
SGLAudioDecoderInterface * SGLWaveFormAudioDecoder::NewDecoder( void ) const
{
	return	new SGLWaveFormAudioDecoder ;
}

// デコーダーを開く
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWaveFormAudioDecoder::Open
	( const wchar_t * pwszFilePath, SSystem::SEnvironmentInterface * pEnv )
{
	SFileInterface *	pFile = NULL ;
	if ( pEnv != NULL )
	{
		pFile = pEnv->NewOpenFile( pwszFilePath, SFileOpener::shareRead ) ;
	}
	else
	{
		pFile = SFileOpener::DefaultNewOpenFile
						( pwszFilePath, SFileOpener::shareRead ) ;
	}
	if ( pFile == NULL )
	{
		return	sglErrFailed ;
	}
	SGLError	err = SGLWaveFormAudioDecoder::Create( pFile, true ) ;
	if ( err )
	{
		delete	pFile ;
		return	err ;
	}
	return	sglErrSuccess ;
}

SGLError SGLWaveFormAudioDecoder::Create
	( SSystem::SFileInterface * file, bool flagOwner )
{
	//
	// ヘッダ読み込み
	//
	WAVE_RIFF_HEADER	hdr ;
	if ( file->Read
		( &hdr, sizeof(WAVE_RIFF_HEADER) ) < sizeof(WAVE_RIFF_HEADER) )
	{
		return	sglErrFailed ;
	}
	if ( (hdr.idRIFF[0] != 'R') | (hdr.idRIFF[1] != 'I')
		| (hdr.idRIFF[2] != 'F') | (hdr.idRIFF[3] != 'F')
		| (hdr.idWAVE[0] != 'W') | (hdr.idWAVE[1] != 'A')
		| (hdr.idWAVE[2] != 'V') | (hdr.idWAVE[3] != 'E') )
	{
		return	sglErrFailed ;
	}
	//
	// フォーマット読み込み
	//
	RIFF_CHUNK_HEADER	chunk ;
	for ( ; ; )
	{
		if ( file->Read
			( &chunk, sizeof(RIFF_CHUNK_HEADER) ) < sizeof(RIFF_CHUNK_HEADER) )
		{
			return	sglErrFailed ;
		}
		if ( (chunk.idChunk[0] == 'f') & (chunk.idChunk[1] == 'm')
			& (chunk.idChunk[2] == 't') & (chunk.idChunk[3] == ' ') )
		{
			break ;
		}
		file->Seek( chunk.nLength, SFileInterface::FromCurrent ) ;
	}
	size_t	sizeFormat = chunk.nLength ;
	m_pwfx =
		(WAVEFORMAT*) esl_realloc
			( m_pwfx, esl_max((int) sizeFormat, (int) sizeof(WAVEFORMAT)) ) ;
	if ( file->Read( m_pwfx, sizeFormat ) < sizeFormat )
	{
		return	sglErrFailed ;
	}
	m_flagCompressed = false ;
	if ( m_pwfx->wFormatTag != formatWavePCM )
	{
		#if	defined(__PLATFORM_WINDOWS__)
		if ( m_acmDecoder.PrepareStream( (WAVEFORMATEX*) m_pwfx ) )
		#endif
		{
			return	sglErrFailed ;
		}
		m_flagCompressed = true ;
	}
	//
	// ウェーブデータを探す
	//
	for ( ; ; )
	{
		if ( file->Read
			( &chunk, sizeof(RIFF_CHUNK_HEADER) ) < sizeof(RIFF_CHUNK_HEADER) )
		{
			return	sglErrFailed ;
		}
		if ( (chunk.idChunk[0] == 'd') & (chunk.idChunk[1] == 'a')
			& (chunk.idChunk[2] == 't') & (chunk.idChunk[3] == 'a') )
		{
			break ;
		}
		file->Seek( chunk.nLength, SFileInterface::FromCurrent ) ;
	}
	if ( m_flagFileOwner )
	{
		delete	m_pFile ;
	}
	m_pFile = file ;
	m_flagFileOwner = flagOwner ;
	m_posWaveBase = file->GetPosition() ;
	m_sizeWave = chunk.nLength ;
	m_posWave = 0 ;
	//
	return	sglErrSuccess ;
}

// デコーダーを閉じる
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWaveFormAudioDecoder::Close( void )
{
	if ( m_flagCompressed )
	{
		#if	defined(__PLATFORM_WINDOWS__)
		m_acmDecoder.CloseStream() ;
		#endif
		m_flagCompressed = false ;
	}
	if ( m_flagFileOwner )
	{
		delete	m_pFile ;
	}
	m_flagFileOwner = false ;
	m_pFile = NULL ;
	//
	if ( m_pwfx != NULL )
	{
		esl_free( m_pwfx ) ;
		m_pwfx = NULL ;
	}
	m_bufWave.FreeArray() ;
	return	sglErrSuccess ;
}

// サウンドフォーマットを取得する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWaveFormAudioDecoder::GetFormat( SGLSoundFormat & fmt )
{
	if ( m_pwfx == NULL )
	{
		return	sglErrFailed ;
	}
	fmt.format = formatSoundLinearPCM ;
	fmt.frequency = m_pwfx->nFrequency ;
	fmt.channels = m_pwfx->wChannels ;
	fmt.bitsPerSample = m_pwfx->wBitsPerSample ;
	return	sglErrSuccess ;
}

// オプショナル情報を取得する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWaveFormAudioDecoder::GetOptinalInfo
				( SGLAudioDecoderInterface::OptionalInfo & optinf )
{
	optinf.nFlags = 0 ;
	return	sglErrSuccess ;
}

// 全長 [/samples] を取得する
//////////////////////////////////////////////////////////////////////////////
uint64_t SGLWaveFormAudioDecoder::GetTotalLength( void ) const
{
	if ( m_pwfx == NULL )
	{
		return	0 ;
	}
	if ( m_flagCompressed )
	{
		#if	defined(__PLATFORM_WINDOWS__)
			const WAVEFORMATEX&	wfx = m_acmDecoder.GetOutputFormat() ;
			if ( wfx.nBlockAlign == 0 )
			{
				return	0 ;
			}
			return	m_acmDecoder.EstimateDecodedSize( m_sizeWave ) / wfx.nBlockAlign ;
		#else
			return	0 ;
		#endif
	}
	else
	{
		if ( (m_pwfx->wBitsPerSample * m_pwfx->wChannels) == 0 )
		{
			return	0 ;
		}
		return	(uint64_t) (m_sizeWave << 3)
							/ (m_pwfx->wBitsPerSample * m_pwfx->wChannels) ;
	}
}

// デコード開始位置 [/samples] を移動する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWaveFormAudioDecoder::SeekPosition( uint64_t nPos )
{
	if ( (m_pFile == NULL) || (m_pwfx == NULL) )
	{
		return	sglErrFailed ;
	}
	if ( m_flagCompressed )
	{
		#if	defined(__PLATFORM_WINDOWS__)
			const WAVEFORMATEX&	wfx = m_acmDecoder.GetOutputFormat() ;
			size_t	nDstBytes = (size_t) nPos * wfx.nBlockAlign ;
			size_t	nSrcBytes = m_acmDecoder.EstimateSourceSize( nDstBytes ) ;
			if ( nSrcBytes > m_sizeWave )
			{
				nSrcBytes = m_sizeWave ;
			}
			m_posWave = (uint32_t) nSrcBytes ;
			m_acmDecoder.FlushStream() ;
			m_bufWave.SetLength( 0 ) ;
		#else
			return	sglErrFailed ;
		#endif
	}
	else
	{
		if ( (m_pwfx->wBitsPerSample * m_pwfx->wChannels) == 0 )
		{
			return	sglErrFailed ;
		}
		uint64_t	nSamples =
				(uint64_t) (m_sizeWave << 3)
							/ (m_pwfx->wBitsPerSample * m_pwfx->wChannels) ;
		if ( nPos > nSamples )
		{
			nPos = nSamples ;
		}
		m_posWave = (uint32_t) (nPos * ((m_pwfx->wBitsPerSample
												* m_pwfx->wChannels) >> 3)) ;
	}
	m_pFile->Seek( m_posWaveBase + m_posWave ) ;
	return	sglErrSuccess ;
}

// 次のデータをデコード
//////////////////////////////////////////////////////////////////////////////
size_t SGLWaveFormAudioDecoder::DecodeNext( void )
{
	if ( m_pFile == NULL )
	{
		return	0 ;
	}
	size_t	nNextBytes = 0x4000 ;
	if ( m_flagCompressed )
	{
		#if	defined(__PLATFORM_WINDOWS__)
			size_t	nNextSrcBytes = 0 ;
			for ( ; ; )
			{
				nNextSrcBytes =
					m_acmDecoder.EstimateSourceSize( nNextBytes ) ;
				if ( nNextSrcBytes > 0 )
				{
					break ;
				}
				nNextBytes <<= 1 ;
			}
			nNextBytes = m_acmDecoder.EstimateDecodedSize( nNextSrcBytes ) ;
			//
			if ( m_posWave + nNextSrcBytes > m_sizeWave )
			{
				nNextSrcBytes = m_sizeWave - m_posWave ;
			}
			SArray<uint8_t>	bufSrcTemp ;
			nNextSrcBytes =
				m_pFile->Read
					( bufSrcTemp.GetArray(nNextSrcBytes), nNextSrcBytes ) ;
			bufSrcTemp.FinishArray() ;
			m_posWave += (uint32_t) nNextSrcBytes ;
			//
			m_acmDecoder.WriteCompressedAudio
				( bufSrcTemp.GetConstArray(), nNextSrcBytes ) ;
			//
			nNextBytes = m_acmDecoder.GetDecompressedBytes() ;
			m_bufWave.SetLength( nNextBytes ) ;
			nNextBytes =
				m_acmDecoder.ReadWaveData( m_bufWave.GetArray(), nNextBytes ) ;
			m_bufWave.FinishArray() ;
		#else
			return	0 ;
		#endif
	}
	else
	{
		if ( m_posWave + nNextBytes > m_sizeWave )
		{
			nNextBytes = m_sizeWave - m_posWave ;
		}
		m_bufWave.SetLength( nNextBytes ) ;
		nNextBytes = m_pFile->Read( m_bufWave.GetArray(), nNextBytes ) ;
		m_bufWave.FinishArray() ;
		m_posWave += (uint32_t) nNextBytes ;
	}
	return	nNextBytes ;
}

// デコードデータを取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLWaveFormAudioDecoder::ReadDecodedBuffer
		( void * ptrPCM, size_t nBytes, size_t nOffset )
{
	if ( m_bufWave.GetLength() <= nOffset )
	{
		return	0 ;
	}
	if ( m_bufWave.GetLength() < nOffset + nBytes )
	{
		nBytes = m_bufWave.GetLength() - nOffset ;
	}
	memmove( ptrPCM, m_bufWave.GetConstArray() + nOffset, nBytes ) ;
	return	nBytes ;
}

