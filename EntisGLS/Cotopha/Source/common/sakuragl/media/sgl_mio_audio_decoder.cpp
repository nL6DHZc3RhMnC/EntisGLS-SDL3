
#include <sakuragl/sakuragl.h>
#include <sakuragl/sgl_media.h>
#include <sakuragl/sgl_erisa_lib.h>
#include <sakuragl/media/sgl_mio_audio_decoder.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// MIO オーディオ・デコーダー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLMIOAudioDecoder, SGLAudioDecoderInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLMIOAudioDecoder::SGLMIOAudioDecoder( void )
{
	m_flagNextSeek = true ;
	m_posNextSeek = 0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLMIOAudioDecoder::~SGLMIOAudioDecoder( void )
{
}

// ファイル拡張子判定
//////////////////////////////////////////////////////////////////////////////
bool SGLMIOAudioDecoder::IsMatchableFileExtension( const wchar_t * pszExt )
{
	return	(SString::CompareNoCase( pszExt, L"mio" ) == 0) ;
}

// デコーダー生成
//////////////////////////////////////////////////////////////////////////////
SGLAudioDecoderInterface * SGLMIOAudioDecoder::NewDecoder( void ) const
{
	return	new SGLMIOAudioDecoder ;
}

// デコーダーを開く
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMIOAudioDecoder::Open
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
	SGLError	err = SGLMIOAudioDecoder::Create( pFile, true ) ;
	if ( err )
	{
		delete	pFile ;
		return	err ;
	}
	return	sglErrSuccess ;
}

SGLError SGLMIOAudioDecoder::Create
	( SSystem::SFileInterface * file, bool flagOwner )
{
	SError	err = m_miofile.OpenSoundFile( file, flagOwner ) ;
	if ( err )
	{
		return	(SGLError) err ;
	}
	m_flagNextSeek = true ;
	m_posNextSeek = 0 ;
	return	sglErrSuccess ;
}

// デコーダーを閉じる
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMIOAudioDecoder::Close( void )
{
	m_miofile.Close() ;
	return	sglErrSuccess ;
}

// サウンドフォーマットを取得する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMIOAudioDecoder::GetFormat( SGLSoundFormat & fmt )
{
	fmt.format = formatSoundLinearPCM ;
	fmt.frequency = m_miofile.GetFrequency() ;
	fmt.channels = m_miofile.GetChannelCount() ;
	fmt.bitsPerSample = m_miofile.GetBitsPerSample() ;
	return	sglErrSuccess ;
}

// オプショナル情報を取得する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMIOAudioDecoder::GetOptinalInfo
	( SGLAudioDecoderInterface::OptionalInfo & optinf )
{
	optinf.nFlags = 0 ;
	//
	const ERISA::SGLMediaFile &	media = m_miofile.GetMediaFile() ;
	if ( media.m_flagsRead & ERISA::SGLMediaFile::readDescription )
	{
		ERISA::SGLMediaFile::STagInfo	taginf ;
		taginf.ParseTagInfo( media.m_strDescription ) ;
		//
		if ( taginf.GetTagContents
				( ERISA::SGLMediaFile::tagRewindPoint ) != NULL )
		{
			int64_t	nLoopStart = taginf.GetRewindPoint() ;
			optinf.nLoopStart = nLoopStart ;
			if ( nLoopStart >= 0 )
			{
				optinf.nFlags |= SGLAudioDecoderInterface::flagLoopStart ;
			}
		}
		int64_t	nLoopEnd = taginf.GetLoopEndPoint() ;
		optinf.nLoopEnd = nLoopEnd ;
		if ( nLoopEnd >= 0 )
		{
			optinf.nFlags |= SGLAudioDecoderInterface::flagLoopEnd ;
		}
		ERISA::SGLMediaFile::STagEntry *
			pTagTitle = taginf.GetTagAs
							( ERISA::SGLMediaFile::m_pwszTagName
									[ERISA::SGLMediaFile::tagTitle] ) ;
		if ( pTagTitle != NULL )
		{
			optinf.nFlags |= SGLAudioDecoderInterface::flagTitle ;
			optinf.pszTitle = pTagTitle->m_contents.GetArray() ;
			pTagTitle->m_contents.FinishArray() ;
		}
		ERISA::SGLMediaFile::STagEntry *
			pTagVocalPlayer = taginf.GetTagAs
							( ERISA::SGLMediaFile::m_pwszTagName
									[ERISA::SGLMediaFile::tagVocalPlayer] ) ;
		if ( pTagVocalPlayer != NULL )
		{
			optinf.nFlags |= SGLAudioDecoderInterface::flagVocalPlayer ;
			optinf.pszVocalPlayer = pTagVocalPlayer->m_contents.GetArray() ;
			pTagVocalPlayer->m_contents.FinishArray() ;
		}
		ERISA::SGLMediaFile::STagEntry *
			pTagComposer = taginf.GetTagAs
							( ERISA::SGLMediaFile::m_pwszTagName
									[ERISA::SGLMediaFile::tagComposer] ) ;
		if ( pTagComposer != NULL )
		{
			optinf.nFlags |= SGLAudioDecoderInterface::flagComposer ;
			optinf.pszComposer = pTagComposer->m_contents.GetArray() ;
			pTagComposer->m_contents.FinishArray() ;
		}
		ERISA::SGLMediaFile::STagEntry *
			pTagArranger = taginf.GetTagAs
							( ERISA::SGLMediaFile::m_pwszTagName
									[ERISA::SGLMediaFile::tagArranger] ) ;
		if ( pTagArranger != NULL )
		{
			optinf.nFlags |= SGLAudioDecoderInterface::flagArranger ;
			optinf.pszArranger = pTagArranger->m_contents.GetArray() ;
			pTagArranger->m_contents.FinishArray() ;
		}
	}
	return	sglErrSuccess ;
}

// 全長 [/samples] を取得する
//////////////////////////////////////////////////////////////////////////////
uint64_t SGLMIOAudioDecoder::GetTotalLength( void ) const
{
	return	m_miofile.GetTotalSampleCount() ;
}

// デコード開始位置 [/samples] を移動する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLMIOAudioDecoder::SeekPosition( uint64_t nPos )
{
	m_flagNextSeek = true ;
	m_posNextSeek = nPos ;
	return	sglErrSuccess ;
}

// 次のデータをデコード
//////////////////////////////////////////////////////////////////////////////
size_t SGLMIOAudioDecoder::DecodeNext( void )
{
	m_nOffsetBytes = 0 ;
	if ( m_flagNextSeek )
	{
		if ( m_miofile.GetWaveBufferFrom
				( m_posNextSeek, m_bufWave, m_nOffsetBytes ) == NULL )
		{
			return	0 ;
		}
		m_flagNextSeek = false ;
	}
	else if ( m_miofile.IsNextDataRewound() )
	{
		return	0 ;
	}
	else if ( m_miofile.GetNextWaveBuffer( m_bufWave ) == NULL )
	{
		return	0 ;
	}
	return	m_bufWave.GetLength() - m_nOffsetBytes ;
}

// デコードデータを取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLMIOAudioDecoder::ReadDecodedBuffer
		( void * ptrPCM, size_t nBytes, size_t nOffset )
{
	nOffset += m_nOffsetBytes ;
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

