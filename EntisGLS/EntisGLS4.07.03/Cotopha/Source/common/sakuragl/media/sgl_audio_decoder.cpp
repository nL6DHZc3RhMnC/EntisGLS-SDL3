
#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl_media.h>
#include <sakuragl/sgl_erisa_lib.h>
#include <sakuragl/media/sgl_mio_audio_decoder.h>
#include <sakuragl/media/sgl_wav_audio_decoder.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// オーディオファイル・デコード・インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLAudioDecoderInterface, SObject )

// ファイル拡張子判定
//////////////////////////////////////////////////////////////////////////////
bool SGLAudioDecoderInterface::IsMatchableFileExtension( const wchar_t * pszExt )
{
	return	false ;
}


//////////////////////////////////////////////////////////////////////////////
// オーディオ・デコーダー管理
//////////////////////////////////////////////////////////////////////////////

// オーディオデコーダー配列
//////////////////////////////////////////////////////////////////////////////
ESL_DLL_DECL( SSystem::SObjectArray<SGLAudioDecoderInterface> *
				SGLAudioDecoderManager::m_arrayAudioDecoder = NULL ) ;

// 初期化
//////////////////////////////////////////////////////////////////////////////
void SGLAudioDecoderManager::Initialzie( void )
{
	if ( m_arrayAudioDecoder == NULL )
	{
		m_arrayAudioDecoder =
			new SSystem::SObjectArray<SGLAudioDecoderInterface> ;
		#if	defined(__COTOPHA__)
			m_arrayAudioDecoder->Add
					( new SGLAudioDecoder( new AudioDecoder, true ) ) ;
		#else
			m_arrayAudioDecoder->Add( new SGLWaveFormAudioDecoder ) ;
			m_arrayAudioDecoder->Add( new SGLMIOAudioDecoder ) ;
		#endif
	}
}

// 終了
//////////////////////////////////////////////////////////////////////////////
void SGLAudioDecoderManager::Finalize( void )
{
	delete	m_arrayAudioDecoder ;
	m_arrayAudioDecoder = NULL ;
}

// デコーダー追加登録
//////////////////////////////////////////////////////////////////////////////
void SGLAudioDecoderManager::RegisterDecoder( SGLAudioDecoderInterface * pDecoder )
{
	SSystem::QuickLock() ;
	if ( m_arrayAudioDecoder == NULL )
	{
		m_arrayAudioDecoder =
			new SSystem::SObjectArray<SGLAudioDecoderInterface> ;
	}
	m_arrayAudioDecoder->InsertAt( 0, pDecoder ) ;
	SSystem::QuickUnlock() ;
}

// 拡張子が合致するデコーダー取得
//////////////////////////////////////////////////////////////////////////////
SGLAudioDecoderInterface * SGLAudioDecoderManager::FindDecoder( const wchar_t * pszExt )
{
	SSystem::QuickLock() ;
	if ( m_arrayAudioDecoder == NULL )
	{
		return	NULL ;
	}
	const size_t	nCount = m_arrayAudioDecoder->GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		SGLAudioDecoderInterface *
					pDecoder = m_arrayAudioDecoder->GetAt( i ) ;
		if ( pDecoder != NULL )
		{
			if ( pDecoder->IsMatchableFileExtension( pszExt ) )
			{
				SSystem::QuickUnlock() ;
				return	pDecoder->NewDecoder() ;
			}
		}
	}
	SSystem::QuickUnlock() ;
	return	NULL ;
}

// ファイルに適合するデコーダーを生成
//////////////////////////////////////////////////////////////////////////////
SGLAudioDecoderInterface *
	SGLAudioDecoderManager::OpenDecoder
		( const wchar_t * pwszFilePath,
			SSystem::SEnvironmentInterface * pEnv )
{
	SSystem::QuickLock() ;
	if ( m_arrayAudioDecoder == NULL )
	{
		SSystem::QuickUnlock() ;
		return	NULL ;
	}
	SString	strFilePath = pwszFilePath ;
	SString	strFileExt = strFilePath.GetFileExtensionPart() ;
	SGLAudioDecoderInterface *	pDecoder = FindDecoder( strFileExt ) ;
	if ( pDecoder != NULL )
	{
		if ( !pDecoder->Open( pwszFilePath, pEnv ) )
		{
			SSystem::QuickUnlock() ;
			return	pDecoder ;
		}
	}
	const size_t	nCount = m_arrayAudioDecoder->GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		pDecoder = m_arrayAudioDecoder->GetAt( i ) ;
		if ( pDecoder != NULL )
		{
			pDecoder = pDecoder->NewDecoder() ;
			if ( pDecoder != NULL )
			{
				if ( !pDecoder->Open( pwszFilePath, pEnv ) )
				{
					SSystem::QuickUnlock() ;
					return	pDecoder ;
				}
				delete	pDecoder ;
			}
		}
	}
	SSystem::QuickUnlock() ;
	return	NULL ;
}

SGLAudioDecoderInterface *
	SGLAudioDecoderManager::CreateDecoder
		( SSystem::SFileInterface * file, bool flagOwner )
{
	SSystem::QuickLock() ;
	if ( m_arrayAudioDecoder == NULL )
	{
		SSystem::QuickUnlock() ;
		return	NULL ;
	}
	const size_t	nCount = m_arrayAudioDecoder->GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		SGLAudioDecoderInterface *
					pDecoder = m_arrayAudioDecoder->GetAt( i ) ;
		if ( pDecoder != NULL )
		{
			pDecoder = pDecoder->NewDecoder() ;
			if ( pDecoder != NULL )
			{
				int64_t	posStart = file->GetPosition() ;
				if ( !pDecoder->Create( file, flagOwner ) )
				{
					SSystem::QuickUnlock() ;
					return	pDecoder ;
				}
				file->Seek( posStart ) ;
				delete	pDecoder ;
			}
		}
	}
	SSystem::QuickUnlock() ;
	return	NULL ;
}


//////////////////////////////////////////////////////////////////////////////
// オーディオ・デコーダー・ラッパー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLAudioDecoder, SGLAudioDecoderInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLAudioDecoder::SGLAudioDecoder( AudioDecoder * pDecoder, bool flagOwner )
{
	m_pDecoder = pDecoder ;
	m_flagOwner = flagOwner ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLAudioDecoder::~SGLAudioDecoder( void )
{
	if ( m_flagOwner )
	{
		delete	m_pDecoder ;
	}
	m_pDecoder = NULL ;
	m_flagOwner = false ;

	#if	defined(__COTOPHA__)
	if ( m_flagFileOwner )
	{
		delete	m_pFile ;
	}
	m_pFile = NULL ;
	m_flagFileOwner = false ;
	#endif
}

// オブジェクト関連付け
//////////////////////////////////////////////////////////////////////////////
void SGLAudioDecoder::AttachAudioDecoder
		( AudioDecoder * pDecoder, bool flagOwner )
{
	if ( m_flagOwner )
	{
		delete	m_pDecoder ;
	}
	m_pDecoder = pDecoder ;
	m_flagOwner = flagOwner ;

	#if	defined(__COTOPHA__)
	if ( m_flagFileOwner )
	{
		delete	m_pFile ;
	}
	m_pFile = NULL ;
	m_flagFileOwner = false ;
	#endif
}

// ファイル拡張子判定
//////////////////////////////////////////////////////////////////////////////
bool SGLAudioDecoder::IsMatchableFileExtension( const wchar_t * pszExt )
{
	#if	!defined(__COTOPHA__)
	if ( m_pDecoder != NULL )
	{
		return	m_pDecoder->IsMatchableFileExtension( pszExt ) ;
	}
	#endif
	return	true ;
}

// デコーダー生成
//////////////////////////////////////////////////////////////////////////////
SGLAudioDecoderInterface * SGLAudioDecoder::NewDecoder( void ) const
{
	#if	defined(__COTOPHA__)
		return	new SGLAudioDecoder( new AudioDecoder, true ) ;
	#else
		if ( m_pDecoder != NULL )
		{
			return	m_pDecoder->NewDecoder() ;
		}
		return	NULL ;
	#endif
}

// デコーダーを開く
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAudioDecoder::Open
	( const wchar_t * pwszFilePath, SSystem::SEnvironmentInterface * pEnv )
{
	#if	defined(__COTOPHA__)
		if ( m_pDecoder == NULL )
		{
			m_pDecoder = new AudioDecoder ;
		}
		return	m_pDecoder->Open( pwszFilePath ) ;
	#else
		if ( m_pDecoder != NULL )
		{
			return	m_pDecoder->Open( pwszFilePath, pEnv ) ;
		}
		return	sglErrFailed ;
	#endif
}

SGLError SGLAudioDecoder::Create( SSystem::SFileInterface * file, bool flagOwner )
{
	#if	defined(__COTOPHA__)
		File *	pFile = file->GetFileObject() ;
		if ( pFile != NULL )
		{
			if ( m_pDecoder == NULL )
			{
				m_pDecoder = new AudioDecoder ;
			}
			if ( m_flagFileOwner )
			{
				delete	m_pFile ;
			}
			m_pFile = file ;
			m_flagFileOwner = flagOwner ;
			return	m_pDecoder->Create( pFile ) ;
		}
	#else
		if ( m_pDecoder != NULL )
		{
			return	m_pDecoder->Create( file, flagOwner ) ;
		}
	#endif
	return	sglErrFailed ;
}

// デコーダーを閉じる
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAudioDecoder::Close( void )
{
	#if	defined(__COTOPHA__)
		AttachAudioDecoder( NULL, false ) ;
	#else
		if ( m_pDecoder != NULL )
		{
			return	m_pDecoder->Close() ;
		}
	#endif
	return	sglErrSuccess ;
}

// サウンドフォーマットを取得する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAudioDecoder::GetFormat( SGLSoundFormat & fmt )
{
	if ( m_pDecoder != NULL )
	{
		return	m_pDecoder->GetFormat( fmt ) ;
	}
	return	sglErrFailed ;
}

// オプショナル情報を取得する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAudioDecoder::GetOptinalInfo
				( SGLAudioDecoderInterface::OptionalInfo & optinf )
{
	if ( m_pDecoder != NULL )
	{
		return	m_pDecoder->GetOptinalInfo( optinf ) ;
	}
	return	sglErrFailed ;
}

// 全長 [/samples] を取得する
//////////////////////////////////////////////////////////////////////////////
uint64_t SGLAudioDecoder::GetTotalLength( void ) const
{
	if ( m_pDecoder != NULL )
	{
		return	m_pDecoder->GetTotalLength() ;
	}
	return	0 ;
}

// デコード開始位置 [/samples] を移動する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAudioDecoder::SeekPosition( uint64_t nPos )
{
	if ( m_pDecoder != NULL )
	{
		return	m_pDecoder->SeekPosition( nPos ) ;
	}
	return	sglErrFailed ;
}

// 次のデータをデコード
//////////////////////////////////////////////////////////////////////////////
size_t SGLAudioDecoder::DecodeNext( void )
{
	if ( m_pDecoder != NULL )
	{
		return	m_pDecoder->DecodeNext() ;
	}
	return	0 ;
}

// デコードデータを取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLAudioDecoder::ReadDecodedBuffer
		( void * ptrPCM, size_t nBytes, size_t nOffset )
{
	if ( m_pDecoder != NULL )
	{
		return	m_pDecoder->ReadDecodedBuffer( ptrPCM, nBytes, nOffset ) ;
	}
	return	0 ;
}

