
#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl_media.h>

using namespace SSystem ;
using namespace SakuraGL ;


// クラス情報
//////////////////////////////////////////////////////////////////////////////

ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLAudioInputStream, SObject ) ;
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLAudioOutputStream, SObject ) ;
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLVideoInputStream, SObject ) ;
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLVideoOutputStream, SObject ) ;


//////////////////////////////////////////////////////////////////////////////
// メディア補助情報
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLMediaOptionalInfo, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLMediaOptionalInfo::SGLMediaOptionalInfo( void )
{
	m_nFlags = 0 ;
	m_nLoopStart = 0 ;
	m_nLoopEnd = 0 ;
}

SGLMediaOptionalInfo::SGLMediaOptionalInfo( const SGLMediaOptionalInfo& optinf )
{
	m_nFlags = optinf.m_nFlags ;
	m_nLoopStart = optinf.m_nLoopStart ;
	m_nLoopEnd = optinf.m_nLoopEnd ;
	m_strTitle = optinf.m_strTitle ;
	m_strPlayer = optinf.m_strPlayer ;
	m_strComposer = optinf.m_strComposer ;
	m_strArranger = optinf.m_strArranger ;
}

// 代入
//////////////////////////////////////////////////////////////////////////////
const SGLMediaOptionalInfo&
	SGLMediaOptionalInfo::operator = ( const SGLMediaOptionalInfo& optinf )
{
	m_nFlags = optinf.m_nFlags ;
	m_nLoopStart = optinf.m_nLoopStart ;
	m_nLoopEnd = optinf.m_nLoopEnd ;
	m_strTitle = optinf.m_strTitle ;
	m_strPlayer = optinf.m_strPlayer ;
	m_strComposer = optinf.m_strComposer ;
	m_strArranger = optinf.m_strArranger ;
	return	*this ;
}


//////////////////////////////////////////////////////////////////////////////
// オーディオ入力ストリーム
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLAudioDecoderStream, SGLAudioInputStream )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLAudioDecoderStream::SGLAudioDecoderStream( void )
{
	m_pDecoder = NULL ;
	m_flagOwnDecoder = false ;
	m_nDecodedBytes = 0 ;
	m_nReadNext = 0 ;
}

SGLAudioDecoderStream::SGLAudioDecoderStream
	( SGLAudioDecoderInterface * pDecoder, bool flagOwnDecoder )
{
	m_pDecoder = NULL ;
	m_flagOwnDecoder = false ;
	AttachAudioDecoder( pDecoder, flagOwnDecoder ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLAudioDecoderStream::~SGLAudioDecoderStream( void )
{
	SGLAudioDecoderStream::Release() ;
}

// オーディオファイルを開く
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAudioDecoderStream::OpenAudioFile( const wchar_t * pwszFilePath )
{
	Release() ;
	//
	SGLAudioDecoderInterface *	pDecoder =
		SGLAudioDecoderManager::OpenDecoder
			( pwszFilePath, SEnvironmentInterface::GetInstance() ) ;
	if ( pDecoder == NULL )
	{
		return	sglErrFailed ;
	}
	AttachAudioDecoder( pDecoder, true ) ;
	return	sglErrSuccess ;
}

SGLError SGLAudioDecoderStream::OpenAudioFile
			( SSystem::SFileInterface * file, bool flagOwner )
{
	Release() ;
	//
	SGLAudioDecoderInterface *	pDecoder =
		SGLAudioDecoderManager::CreateDecoder( file, flagOwner ) ;
	if ( pDecoder == NULL )
	{
		if ( flagOwner )
		{
			delete	file ;
		}
		return	sglErrFailed ;
	}
	AttachAudioDecoder( pDecoder, true ) ;
	return	sglErrSuccess ;
}

// オープン済みデコーダーを関連付ける
//////////////////////////////////////////////////////////////////////////////
void SGLAudioDecoderStream::AttachAudioDecoder
	( SGLAudioDecoderInterface * pDecoder, bool flagOwnDecoder )
{
	Release() ;
	//
	m_pDecoder = pDecoder ;
	m_flagOwnDecoder = flagOwnDecoder ;
	m_nDecodedBytes = 0 ;
	m_nReadNext = 0 ;
	//
	if ( pDecoder != NULL )
	{
		pDecoder->GetFormat( m_fmtAudio ) ;
	}
}

// デコーダーを解放する
//////////////////////////////////////////////////////////////////////////////
void SGLAudioDecoderStream::Release( void )
{
	if ( m_flagOwnDecoder )
	{
		delete	m_pDecoder ;
		m_flagOwnDecoder = false ;
	}
	m_pDecoder = NULL ;
}

// サウンド形式取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAudioDecoderStream::GetAudioFormat( SGLSoundFormat& fmt )
{
	if ( m_pDecoder == NULL )
	{
		return	sglErrFailed ;
	}
	return	m_pDecoder->GetFormat( fmt ) ;
}

// メディア補助情報取得
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAudioDecoderStream::GetAudioOptinalInfo
							( SGLMediaOptionalInfo& optinf )
{
	if ( m_pDecoder == NULL )
	{
		return	sglErrFailed ;
	}
	SGLAudioDecoderInterface::OptionalInfo	optAudio ;
	SGLError	err = m_pDecoder->GetOptinalInfo( optAudio ) ;
	if ( err )
	{
		return	err ;
	}
	optinf.m_nFlags = 0 ;
	if ( optAudio.nFlags & SGLAudioDecoderInterface::flagLoopStart )
	{
		optinf.m_nFlags |= SGLMediaOptionalInfo::flagLoopStart ;
		optinf.m_nLoopStart = optAudio.nLoopStart ;
	}
	if ( optAudio.nFlags & SGLAudioDecoderInterface::flagLoopEnd )
	{
		optinf.m_nFlags |= SGLMediaOptionalInfo::flagLoopEnd ;
		optinf.m_nLoopEnd = optAudio.nLoopEnd ;
	}
	if ( optAudio.nFlags & SGLAudioDecoderInterface::flagTitle )
	{
		optinf.m_nFlags |= SGLMediaOptionalInfo::flagTitle ;
		optinf.m_strTitle = optAudio.pszTitle ;
	}
	if ( optAudio.nFlags & SGLAudioDecoderInterface::flagVocalPlayer )
	{
		optinf.m_nFlags |= SGLMediaOptionalInfo::flagVocalPlayer ;
		optinf.m_strPlayer = optAudio.pszVocalPlayer ;
	}
	if ( optAudio.nFlags & SGLAudioDecoderInterface::flagComposer )
	{
		optinf.m_nFlags |= SGLMediaOptionalInfo::flagComposer ;
		optinf.m_strComposer = optAudio.pszComposer ;
	}
	if ( optAudio.nFlags & SGLAudioDecoderInterface::flagArranger )
	{
		optinf.m_nFlags |= SGLMediaOptionalInfo::flagArranger ;
		optinf.m_strArranger = optAudio.pszArranger ;
	}
	return	sglErrSuccess ;
}

// オーディオストリーム全長取得（未定は-1）[samples]
//////////////////////////////////////////////////////////////////////////////
int64_t SGLAudioDecoderStream::GetAudioLength( void ) const
{
	if ( m_pDecoder == NULL )
	{
		return	0 ;
	}
	return	(int64_t) m_pDecoder->GetTotalLength() ;
}

// オーディオストリーム読み込み [samples]
//////////////////////////////////////////////////////////////////////////////
size_t SGLAudioDecoderStream::ReadAudio( void * ptrBuf, size_t nSamples )
{
	if ( m_pDecoder == NULL )
	{
		return	0 ;
	}
	size_t	nSampleBytes = ((m_fmtAudio.bitsPerSample
									* m_fmtAudio.channels) >> 3) ;
	if ( nSampleBytes == 0 )
	{
		return	0 ;
	}
	size_t	nBytes = nSamples * nSampleBytes ;
	size_t	nResultBytes = 0 ;
	for ( ; ; )
	{
		if ( m_nReadNext < m_nDecodedBytes )
		{
			size_t	nReadBytes = m_nDecodedBytes - m_nReadNext;
			if ( nResultBytes + nReadBytes > nBytes )
			{
				if ( nResultBytes >= nBytes )
				{
					break ;
				}
				nReadBytes = nBytes - nResultBytes ;
			}
			nReadBytes = m_pDecoder->ReadDecodedBuffer
								( ptrBuf, nReadBytes, m_nReadNext ) ;
			nResultBytes += nReadBytes ;
			ptrBuf = ((uint8_t*)ptrBuf) + nReadBytes ;
			m_nReadNext += nReadBytes ;
			if ( nResultBytes >= nBytes )
			{
				break ;
			}
		}
		m_nDecodedBytes = m_pDecoder->DecodeNext() ;
		m_nReadNext = 0 ;
		if ( m_nDecodedBytes == 0 )
		{
			break ;
		}
	}
	return	nResultBytes / nSampleBytes ;
}

// オーディオストリーム位置変更
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAudioDecoderStream::SeekAudio( uint64_t nSamples )
{
	if ( m_pDecoder == NULL )
	{
		return	sglErrFailed ;
	}
	m_nDecodedBytes = 0 ;
	m_nReadNext = 0 ;
	return	m_pDecoder->SeekPosition( nSamples ) ;
}


