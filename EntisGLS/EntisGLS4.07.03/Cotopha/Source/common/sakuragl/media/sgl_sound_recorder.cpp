
#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl_media.h>
#include <sakuragl/media/sgl_sound_recorder.h>

#if	defined(__PLATFORM_WINDOWS__)
#include <sakuragl/sgl_mme_sound_recorder.h>

#elif	defined(__PLATFORM_ANDROID__)
#include <sakuragl/sgl_android_sound_recorder.h>

#endif

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// PCM サウンド入力インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLSoundRecorderInterface, SObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSoundRecorderInterface::SGLSoundRecorderInterface( void )
{
}


//////////////////////////////////////////////////////////////////////////////
// PCM サウンド入力ラッパー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::SGLSoundRecorder, SGLSoundRecorderInterface )

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSoundRecorder::~SGLSoundRecorder( void )
{
	if ( m_flagOwner )
	{
		delete	m_pRecorder ;
		m_pRecorder = NULL ;
		m_flagOwner = false ;
	}
}

// 代入
//////////////////////////////////////////////////////////////////////////////
SoundRecorder * SGLSoundRecorder::SetSoundRecorder
	( SoundRecorder * pRecorder, bool flagOwner )
{
	if ( m_flagOwner )
	{
		delete	m_pRecorder ;
	}
	m_pRecorder = pRecorder ;
	m_flagOwner = flagOwner ;
	return	m_pRecorder ;
}

// デバイス列挙
//////////////////////////////////////////////////////////////////////////////
size_t SGLSoundRecorder::EnumerateDevices
		( uint16_t * pwNameBuf, size_t nNameBufLength )
{
	if ( m_pRecorder == NULL )
	{
		#if	defined(__COTOPHA__)
			m_pRecorder = new SoundRecorder ;
		#elif	defined(__PLATFORM_WINDOWS__)
			if ( SGLWinCoreAudioRecorder::IsSupported() )
			{
				m_pRecorder = new SGLWinCoreAudioRecorder ;
			}
			else
			{
				m_pRecorder = new SGLWin32MMSoundRecorder ;
			}
		#elif	defined(__PLATFORM_ANDROID__)
			m_pRecorder = new SGLAndroidSoundRecorder ;
		#else
			#error no implement SGLSoundRecorder::EnumerateDevices
			return	0 ;
		#endif
		m_flagOwner = true ;
	}
	return	m_pRecorder->EnumerateDevices( pwNameBuf, nNameBufLength ) ;
}

size_t SGLSoundRecorder::EnumerateDevices
	( SSystem::SObjectArray<SSystem::SString>& aDevNames )
{
	SArray<uint16_t>	bufName ;
	uint16_t *	pwNameBuf = bufName.GetArray( 0x10000 ) ;
	//
	size_t	nCount = EnumerateDevices( pwNameBuf, 0x10000 ) ;
	//
	while ( pwNameBuf[0] != 0 )
	{
		SString *	pstrName = new SString( pwNameBuf ) ;
		aDevNames.Add( pstrName ) ;
		pwNameBuf += pstrName->GetLength() + 1 ;
	}
	bufName.FinishArray() ;
	return	nCount ;
}

// フォーマットを指定して入力を準備する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSoundRecorder::Open( size_t iDevice, const SGLSoundFormat& fmt )
{
	if ( m_pRecorder == NULL )
	{
		#if	defined(__COTOPHA__)
			m_pRecorder = new SoundRecorder ;
		#elif	defined(__PLATFORM_WINDOWS__)
			if ( SGLWinCoreAudioRecorder::IsSupported() )
			{
				m_pRecorder = new SGLWinCoreAudioRecorder ;
			}
			else
			{
				m_pRecorder = new SGLWin32MMSoundRecorder ;
			}
		#elif	defined(__PLATFORM_ANDROID__)
			m_pRecorder = new SGLAndroidSoundRecorder ;
		#else
			#error no implement SGLSoundRecorder::Open
			return	sglErrFailed ;
		#endif
		m_flagOwner = true ;
	}
	return	m_pRecorder->Open( iDevice, fmt ) ;
}

// 入力用に準備したサウンド入力を解放する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSoundRecorder::Close( void )
{
	if ( m_pRecorder == NULL )
	{
		return	sglErrSuccess ;
	}
	return	m_pRecorder->Close() ;
}

// ストリームバッファを準備する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSoundRecorder::PrepareStream( size_t nBytes )
{
	if ( m_pRecorder == NULL )
	{
		return	sglErrFailed ;
	}
	return	m_pRecorder->PrepareStream( nBytes ) ;
}

// ストリームバッファから読み出す
//////////////////////////////////////////////////////////////////////////////
size_t SGLSoundRecorder::Read( void * ptrSound, size_t nBytes )
{
	if ( m_pRecorder == NULL )
	{
		return	0 ;
	}
	return	m_pRecorder->Read( ptrSound, nBytes ) ;
}

// 録音を開始する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSoundRecorder::Start( uint64_t nFlags )
{
	if ( m_pRecorder == NULL )
	{
		return	sglErrFailed ;
	}
	return	m_pRecorder->Start( nFlags ) ;
}

// 録音を停止する
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSoundRecorder::Stop( void )
{
	if ( m_pRecorder == NULL )
	{
		return	sglErrFailed ;
	}
	return	m_pRecorder->Stop() ;
}

// 音量取得 [L/R]
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSoundRecorder::GetVolume( float32_t* pVolumes, size_t nChannels )
{
	if ( m_pRecorder == NULL )
	{
		return	sglErrFailed ;
	}
	return	m_pRecorder->GetVolume( pVolumes, nChannels ) ;
}

// 音量設定 [L/R]
//////////////////////////////////////////////////////////////////////////////
SGLError SGLSoundRecorder::SetVolume( const float32_t* pVolumes, size_t nChannels )
{
	if ( m_pRecorder == NULL )
	{
		return	sglErrFailed ;
	}
	return	m_pRecorder->SetVolume( pVolumes, nChannels ) ;
}

// 録音中か？
//////////////////////////////////////////////////////////////////////////////
bool SGLSoundRecorder::IsRecording( void ) const
{
	if ( m_pRecorder == NULL )
	{
		return	false ;
	}
	return	m_pRecorder->IsRecording() ;
}

