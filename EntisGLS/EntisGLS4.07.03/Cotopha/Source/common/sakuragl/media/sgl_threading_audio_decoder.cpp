
#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl_media.h>
#include <sakuragl/media/sgl_threading_audio_decoder.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// スレッディング・オーディオ・デコーダー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLThreadingAudioDecoder, SGLAudioDecoderInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLThreadingAudioDecoder::SGLThreadingAudioDecoder
				( SGLAudioDecoderInterface * pDecoder, bool flagOwner )
{
	m_pDecoder = pDecoder ;
	m_flagOwner = flagOwner ;
	//
	m_flagThreading = false ;
	m_signalDone.Initialize( false ) ;
	m_nDeocdedSize = 0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLThreadingAudioDecoder::~SGLThreadingAudioDecoder( void )
{
	SGLThreadingAudioDecoder::Close() ;
	//
	if ( m_flagOwner )
	{
		delete	m_pDecoder ;
	}
	m_pDecoder = NULL ;
	m_flagOwner = false ;
}

// ペンディング状態同期
//////////////////////////////////////////////////////////////////////////////
void SGLThreadingAudioDecoder::SyncPending( void )
{
	if ( m_flagThreading )
	{
		m_signalDone.Wait() ;
	}
}

// イベント発行
//////////////////////////////////////////////////////////////////////////////
void SGLThreadingAudioDecoder::IssueThreadEvent( void )
{
	if ( m_flagThreading )
	{
		m_signalDone.Wait() ;
		m_flagThreading = false ;
	}
	m_signalDone.ResetSignal() ;
	if ( SThread::BeginStockThread
			( &DecodeNextThreadProc, this ) != NULL )
	{
		m_flagThreading = true ;
	}
}

// ファイル拡張子判定
//////////////////////////////////////////////////////////////////////////////
bool SGLThreadingAudioDecoder::IsMatchableFileExtension( const wchar_t * pszExt )
{
	if ( m_pDecoder == NULL )
	{
		return	false ;
	}
	return	m_pDecoder->IsMatchableFileExtension( pszExt ) ;
}

// デコーダー生成
//////////////////////////////////////////////////////////////////////////////
SGLAudioDecoderInterface * SGLThreadingAudioDecoder::NewDecoder( void ) const
{
	if ( m_pDecoder == NULL )
	{
		return	new SGLThreadingAudioDecoder( NULL, false ) ;
	}
	return	new SGLThreadingAudioDecoder( m_pDecoder->NewDecoder(), true ) ;
}

// デコーダーを開く
//////////////////////////////////////////////////////////////////////////////
SGLError SGLThreadingAudioDecoder::Open
	( const wchar_t * pwszFilePath, SSystem::SEnvironmentInterface * pEnv )
{
	SGLThreadingAudioDecoder::Close() ;
	//
	if ( m_pDecoder == NULL )
	{
		return	sglErrFailed ;
	}
	return	m_pDecoder->Open( pwszFilePath, pEnv ) ;
}

SGLError SGLThreadingAudioDecoder::Create
	( SSystem::SFileInterface * file, bool flagOwner )
{
	SGLThreadingAudioDecoder::Close() ;
	//
	if ( m_pDecoder == NULL )
	{
		return	sglErrFailed ;
	}
	return	m_pDecoder->Create( file, flagOwner ) ;
}

// デコーダーを閉じる
//////////////////////////////////////////////////////////////////////////////
SGLError SGLThreadingAudioDecoder::Close( void )
{
	if ( m_flagThreading )
	{
		m_signalDone.Wait() ;
		m_flagThreading = false ;
	}
	if ( m_pDecoder == NULL )
	{
		return	sglErrFailed ;
	}
	return	m_pDecoder->Close() ;
}

// サウンドフォーマットを取得する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLThreadingAudioDecoder::GetFormat( SGLSoundFormat & fmt )
{
	if ( m_pDecoder == NULL )
	{
		return	sglErrFailed ;
	}
	return	m_pDecoder->GetFormat( fmt ) ;
}

// オプショナル情報を取得する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLThreadingAudioDecoder::GetOptinalInfo
				( SGLAudioDecoderInterface::OptionalInfo & optinf )
{
	if ( m_pDecoder == NULL )
	{
		return	sglErrFailed ;
	}
	return	m_pDecoder->GetOptinalInfo( optinf ) ;
}

// 全長 [/samples] を取得する
//////////////////////////////////////////////////////////////////////////////
uint64_t SGLThreadingAudioDecoder::GetTotalLength( void ) const
{
	if ( m_pDecoder == NULL )
	{
		return	sglErrFailed ;
	}
	return	m_pDecoder->GetTotalLength() ;
}

// デコード開始位置 [/samples] を移動する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLThreadingAudioDecoder::SeekPosition( uint64_t nPos )
{
	SyncPending() ;
	//
	if ( m_pDecoder == NULL )
	{
		return	sglErrFailed ;
	}
	SGLError	err = m_pDecoder->SeekPosition( nPos ) ;
	if ( !err )
	{
		IssueThreadEvent() ;
	}
	return	err ;
}

// 次のデータをデコード
//////////////////////////////////////////////////////////////////////////////
size_t SGLThreadingAudioDecoder::DecodeNext( void )
{
	if ( m_pDecoder == NULL )
	{
		return	0 ;
	}
	//
	size_t	nDecoded ;
	if ( m_flagThreading )
	{
		SyncPending() ;
		nDecoded = m_nDeocdedSize ;
	}
	else
	{
		nDecoded = m_pDecoder->DecodeNext() ;
	}
	m_bufDecoded.SetLength( nDecoded ) ;
	m_pDecoder->ReadDecodedBuffer
			( m_bufDecoded.GetArray(), nDecoded ) ;
	m_bufDecoded.FinishArray() ;
	//
	IssueThreadEvent() ;
	return	nDecoded ;
}

// デコードデータを取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLThreadingAudioDecoder::ReadDecodedBuffer
		( void * ptrPCM, size_t nBytes, size_t nOffset )
{
	if ( m_pDecoder == NULL )
	{
		return	0 ;
	}
	if ( nOffset >= m_bufDecoded.GetLength() )
	{
		return	0 ;
	}
	if ( nOffset + nBytes > m_bufDecoded.GetLength() )
	{
		nBytes = m_bufDecoded.GetLength() - nOffset ;
	}
	memmove( ptrPCM, m_bufDecoded.GetConstArray() + nOffset, nBytes ) ;
	//
	return	nBytes ;
}

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
void SGLThreadingAudioDecoder::DecodeNextThreadProc( void * pInstance )
{
	SGLThreadingAudioDecoder *
			pThis = (SGLThreadingAudioDecoder*) pInstance ;
	pThis->m_nDeocdedSize = pThis->m_pDecoder->DecodeNext() ;
	pThis->m_signalDone.SetSignal() ;
}

