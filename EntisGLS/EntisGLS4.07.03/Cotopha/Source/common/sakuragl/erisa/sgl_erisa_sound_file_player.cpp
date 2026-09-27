
/*****************************************************************************
                         E R I S A - L i b r a r y
 -----------------------------------------------------------------------------
    Copyright (C) 2002-2013 Leshade Entis, Entis-soft. All rights reserved.
 *****************************************************************************/


#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl_erisa_lib.h>

using namespace SSystem ;
using namespace SakuraGL ;
using namespace ERISA ;


//////////////////////////////////////////////////////////////////////////////
// レコード先読みオブジェクト
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSoundFilePlayer::PreloadBuffer::PreloadBuffer( size_t nLength )
{
	SetLength( nLength ) ;
}

SGLSoundFilePlayer::PreloadBuffer::PreloadBuffer
			( const SGLSoundFilePlayer::PreloadBuffer& src )
	: SByteBuffer( src ),
		m_miodh( src.m_miodh ), m_nKeySample( src.m_nKeySample )
{
}


//////////////////////////////////////////////////////////////////////////////
// MIOファイルストリーム再生オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ERISA::SGLSoundFilePlayer, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSoundFilePlayer::SGLSoundFilePlayer( void )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSoundFilePlayer::~SGLSoundFilePlayer( void )
{
	SGLSoundFilePlayer::Close() ;
}

// MIO ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLSoundFilePlayer::OpenSoundFile
	( SSystem::SFileInterface * pFile, bool flagOwner )
{
	Close() ;
	//
	// ERIファイルを開く（ストリームレコードまで開く）
	//////////////////////////////////////////////////////////////////////////
	if ( m_erif.OpenMediaFile( pFile, SGLMediaFile::openStream, flagOwner ) )
	{
		return	errFailed ;
	}
	//
	// 展開オブジェクトを初期化する
	//////////////////////////////////////////////////////////////////////////
	if ( !(m_erif.m_flagsRead & SGLMediaFile::readSoundInfo) )
	{
		return	errFailed ;
	}
	if ( m_decoder.Initialize( m_erif.m_mioInfoHeader ) )
	{
		return	errFailed ;
	}
	m_bstream = new SGLDecodeBitStream( 0x4000 ) ;
	//
	// 先読みバッファ配列を確保
	//////////////////////////////////////////////////////////////////////////
	m_queueSound.SetLimit( 2 ) ;
	m_nCurrentSample = 0 ;
	//
	PreloadBuffer *	pBuffer = LoadSoundStream( m_nCurrentSample ) ;
	if ( pBuffer != NULL )
	{
		AddPreloadBuffer( pBuffer ) ;
		m_nCurrentSample += pBuffer->m_miodh.dwSampleCount ;
	}
	return	errSuccess ;
}

// MIO ファイルを閉じる
//////////////////////////////////////////////////////////////////////////////
void SGLSoundFilePlayer::Close( void )
{
	//
	// 先読みキューをクリアする
	//
	m_queueSound.RemoveAll( ) ;
	//
	// キーポイント配列をクリアする
	//
	m_arrayKeySample.RemoveAll( ) ;
	//
	// 展開オブジェクトを削除する
	//
	m_decoder.Delete( ) ;
	m_bstream = NULL ;
	//
	// ファイルを閉じる
	//
	m_erif.Close( ) ;
}

// 指定サンプルへ移動し、初めのブロックのデータを取得する
//////////////////////////////////////////////////////////////////////////////
uint8_t * SGLSoundFilePlayer::GetWaveBufferFrom
	( uint64_t nSample,
		SSystem::SArray<uint8_t> & bufWave, uint32_t & nOffsetBytes )
{
	//
	// シーク
	//////////////////////////////////////////////////////////////////////////
	SeekKeySample( nSample, m_nCurrentSample ) ;
	//
	// 先頭のデータを取得して展開する
	//////////////////////////////////////////////////////////////////////////
	SSmartPointer<PreloadBuffer>	pBuffer = GetPreloadBuffer() ;
	if ( pBuffer == NULL )
	{
		return	NULL ;
	}
	if ( (nSample < pBuffer->m_nKeySample) ||
		(nSample >= pBuffer->m_nKeySample + pBuffer->m_miodh.dwSampleCount) )
	{
		return	NULL ;
	}
	uint32_t	nSampleBytes = GetChannelCount() * GetBitsPerSample() / 8 ;
	size_t		nBytes = pBuffer->m_miodh.dwSampleCount * nSampleBytes ;
	nOffsetBytes = (uint32_t) ((nSample - pBuffer->m_nKeySample) * nSampleBytes) ;
	//
	uint8_t *	ptrWaveBuf ;
	bufWave.SetLength( nBytes ) ;
	ptrWaveBuf = bufWave.GetArray() ;
	//
	m_bstream->AttachInputStream( (PreloadBuffer*) pBuffer ) ;
	//
	if ( m_decoder.DecodeSound( *m_bstream, pBuffer->m_miodh, ptrWaveBuf ) )
	{
		bufWave.FinishArray() ;
		return	NULL ;
	}
	bufWave.FinishArray() ;
	return	ptrWaveBuf ;
}

// 次の音声データがストリームの先頭であるか？
//////////////////////////////////////////////////////////////////////////////
bool SGLSoundFilePlayer::IsNextDataRewound( void )
{
	if ( m_queueSound.GetLength() == 0 )
	{
		PreloadBuffer *	pBuffer = LoadSoundStream( m_nCurrentSample ) ;
		if ( pBuffer != NULL )
		{
			AddPreloadBuffer( pBuffer ) ;
			m_nCurrentSample += pBuffer->m_miodh.dwSampleCount ;
		}
	}
	bool	flagRewound = false ;
	if ( m_queueSound.GetLength() != 0 )
	{
		PreloadBuffer *	pBuffer = m_queueSound.GetAt( 0 ) ;
		if ( (pBuffer != NULL) && (pBuffer->m_nKeySample == 0) )
		{
			flagRewound = true ;
		}
	}
	return	flagRewound ;
}

// 次の音声データを取得
//////////////////////////////////////////////////////////////////////////////
uint8_t * SGLSoundFilePlayer::GetNextWaveBuffer
					( SSystem::SArray<uint8_t> & bufWave )
{
	SSmartPointer<PreloadBuffer>	pBuffer = GetPreloadBuffer( ) ;
	if ( pBuffer == NULL )
	{
		return	NULL ;
	}
	uint32_t	nSampleBytes = GetChannelCount() * GetBitsPerSample() / 8 ;
	uint32_t	nBytes = pBuffer->m_miodh.dwSampleCount * nSampleBytes ;
	//
	uint8_t *	ptrWaveBuf ;
	bufWave.SetLength( nBytes ) ;
	ptrWaveBuf = bufWave.GetArray() ;
	//
	m_bstream->AttachInputStream( (PreloadBuffer*) pBuffer ) ;
	//
	if ( m_decoder.DecodeSound( *m_bstream, pBuffer->m_miodh, ptrWaveBuf ) )
	{
		bufWave.FinishArray() ;
		return	NULL ;
	}
	bufWave.FinishArray() ;
	return	ptrWaveBuf ;
}

// SGLMediaFile オブジェクトを取得する
//////////////////////////////////////////////////////////////////////////////
const SGLMediaFile & SGLSoundFilePlayer::GetMediaFile( void ) const
{
	return	m_erif ;
}

// チャネル数を取得する
//////////////////////////////////////////////////////////////////////////////
DWORD SGLSoundFilePlayer::GetChannelCount( void ) const
{
	return	m_erif.m_mioInfoHeader.dwChannelCount ;
}

// サンプリング周波数を取得する
//////////////////////////////////////////////////////////////////////////////
DWORD SGLSoundFilePlayer::GetFrequency( void ) const
{
	return	m_erif.m_mioInfoHeader.dwSamplesPerSec ;
}

// サンプリングビット分解能を取得する
//////////////////////////////////////////////////////////////////////////////
DWORD SGLSoundFilePlayer::GetBitsPerSample( void ) const
{
	return	m_erif.m_mioInfoHeader.dwBitsPerSample ;
}

// 全体の長さ（サンプル数）を取得する
//////////////////////////////////////////////////////////////////////////////
DWORD SGLSoundFilePlayer::GetTotalSampleCount( void ) const
{
	return	m_erif.m_mioInfoHeader.dwAllSampleCount ;
}

// 先読みバッファを取得する
//////////////////////////////////////////////////////////////////////////////
SGLSoundFilePlayer::PreloadBuffer *
	SGLSoundFilePlayer::GetPreloadBuffer( void )
{
	while ( m_queueSound.GetLength() <= 1 )
	{
		PreloadBuffer *	pBuffer = LoadSoundStream( m_nCurrentSample ) ;
		if ( pBuffer == NULL )
		{
			break ;
		}
		AddPreloadBuffer( pBuffer ) ;
		m_nCurrentSample += pBuffer->m_miodh.dwSampleCount ;
	}
	PreloadBuffer *	pBuffer = NULL ;
	if ( m_queueSound.GetLength() != 0 )
	{
		pBuffer = m_queueSound.GetAt( 0 ) ;
		m_queueSound.DetachAt( 0 ) ;
	}
	return	pBuffer ;
}

// 先読みバッファに追加する
//////////////////////////////////////////////////////////////////////////////
void SGLSoundFilePlayer::AddPreloadBuffer
			( SGLSoundFilePlayer::PreloadBuffer * pBuffer )
{
	if ( m_queueSound.GetLength() < m_queueSound.GetLimit() )
	{
		m_queueSound.Add( pBuffer ) ;
	}
	else
	{
		delete	pBuffer ;
	}
}

// 音声データレコードを読み込む
//////////////////////////////////////////////////////////////////////////////
SGLSoundFilePlayer::PreloadBuffer *
	SGLSoundFilePlayer::LoadSoundStream( uint64_t & nCurrentSample )
{
	KeyPoint	keypoint ;
	uint64_t	nRecPosition = m_erif.GetPosition( ) ;
	while ( m_erif.DescendChunk( "SoundStm" ) )
	{
		if ( nCurrentSample == 0 )
		{
			// 1つも音声レコードが無い場合はエラー
			return	NULL ;
		}
		// レコードの終端に到達したら
		// 自動的に先頭に移動
		const KeyPoint *	pKeyPoint = SearchKeySample( nCurrentSample ) ;
		if ( pKeyPoint == NULL )
		{
			keypoint.m_nKeySample = nCurrentSample ;
			keypoint.m_nRecOffset = nRecPosition ;
			AddKeySample( keypoint ) ;
		}
		nCurrentSample = 0 ;
		m_erif.Seek( 0 ) ;
	}
	//
	// 音声データレコードを読み込む
	//
	size_t	nDataBytes = (size_t) m_erif.GetLength() ;
	PreloadBuffer *
		pBuffer = new PreloadBuffer
					( nDataBytes - sizeof(ERISA::MIO_DATA_HEADER) ) ;
	//
	pBuffer->m_nKeySample = nCurrentSample ;
	//
	m_erif.Read
		( &(pBuffer->m_miodh), sizeof(ERISA::MIO_DATA_HEADER) ) ;
	m_erif.Read
		( pBuffer->GetArray(),
				nDataBytes - sizeof(ERISA::MIO_DATA_HEADER) ) ;
	pBuffer->FinishArray() ;
	//
	m_erif.AscendChunk() ;
	//
	// キーポイントの設定
	//
	if ( pBuffer->m_miodh.bytFlags & mioDataLeadBlock )
	{
		const KeyPoint *	pKeyPoint = SearchKeySample( nCurrentSample ) ;
		if ( pKeyPoint == NULL )
		{
			keypoint.m_nKeySample = nCurrentSample ;
			keypoint.m_nRecOffset = nRecPosition ;
			AddKeySample( keypoint ) ;
		}
	}
	return	pBuffer ;
}

// キーフレームポイントを追加する
//////////////////////////////////////////////////////////////////////////////
void SGLSoundFilePlayer::AddKeySample
			( const SGLSoundFilePlayer::KeyPoint & key )
{
	m_arrayKeySample.Add( key ) ;
}

// 指定のキーフレームを検索する
//////////////////////////////////////////////////////////////////////////////
const SGLSoundFilePlayer::KeyPoint *
	SGLSoundFilePlayer::SearchKeySample( uint64_t nKeySample )
{
	size_t		iFirst, iMiddle, iEnd ;
	KeyPoint *	pFoundKey = NULL ;
	//
	if ( m_arrayKeySample.GetLength() == 0 )
	{
		return	NULL ;
	}
	iFirst = 0 ;
	iMiddle = 0 ;
	iEnd = (size_t) m_arrayKeySample.GetLength() - 1 ;
	//
	for ( ; ; )
	{
		if ( iFirst >= iEnd )
		{
			pFoundKey = NULL ;
			ESLAssert( iMiddle < m_arrayKeySample.GetLength() ) ;
			if ( m_arrayKeySample.At(iMiddle).m_nKeySample == nKeySample )
			{
				pFoundKey = m_arrayKeySample.GetAt( iMiddle ) ;
			}
			else if ( m_arrayKeySample.At(iMiddle).m_nKeySample > nKeySample )
			{
				while ( iMiddle > 0 )
				{
					if ( m_arrayKeySample.At(-- iMiddle).m_nKeySample <= nKeySample )
					{
						pFoundKey = m_arrayKeySample.GetAt( iMiddle ) ;
						break ;
					}
				}
			}
			else
			{
				while ( iMiddle < m_arrayKeySample.GetLength() - 1 )
				{
					if ( m_arrayKeySample.At(iMiddle + 1).m_nKeySample == nKeySample )
					{
						pFoundKey = m_arrayKeySample.GetAt( iMiddle + 1 ) ;
						break ;
					}
					else if ( m_arrayKeySample.At(iMiddle + 1).m_nKeySample > nKeySample )
					{
						pFoundKey = m_arrayKeySample.GetAt( iMiddle ) ;
						break ;
					}
					++ iMiddle ;
				}
			}
			break ;
		}
		//
		iMiddle = (iFirst + iEnd) >> 1 ;
		pFoundKey = m_arrayKeySample.GetAt( iMiddle ) ;
		//
		if ( pFoundKey->m_nKeySample == nKeySample )
		{
			break ;
		}
		if ( pFoundKey->m_nKeySample > nKeySample )
		{
			iEnd = iMiddle - 1 ;
		}
		else
		{
			iFirst = iMiddle + 1 ;
		}
	}
	return	pFoundKey ;
}

// 指定のサンプルを含むブロックを読み込む
//////////////////////////////////////////////////////////////////////////////
void SGLSoundFilePlayer::SeekKeySample
		( uint64_t nSample, uint64_t & nCurrentSample )
{
	//
	// 既に先読みキューに読み込まれていないか判断
	//////////////////////////////////////////////////////////////////////////
	size_t	iLoaded = 0 ;
	size_t	iLeadBlock = 0 ;
	while ( iLoaded < m_queueSound.GetLength() )
	{
		PreloadBuffer *	pBuffer = m_queueSound.GetAt( iLoaded ) ;
		if ( pBuffer != NULL )
		{
			if ( pBuffer->m_miodh.bytFlags & mioDataLeadBlock )
			{
				iLeadBlock = iLoaded ;
			}
			if ( (pBuffer->m_nKeySample <= nSample)
				&& ((nSample - pBuffer->m_nKeySample)
						< pBuffer->m_miodh.dwSampleCount) )
			{
				break ;
			}
		}
		++ iLoaded ;
	}
	//
	// 既に読み込まれている場合にはそこまでシークする
	//
	SSystem::SArray<uint8_t>	bufTemp ;
	if ( iLoaded < m_queueSound.GetLength() )
	{
		//
		// 最も近いリードブロックまで破棄する
		//
		size_t	i ;
		m_queueSound.Remove( 0, iLeadBlock ) ;
		//
		// 特定のブロックまでシークする
		//
		for ( i = iLeadBlock; i < iLoaded; i ++ )
		{
			SSmartPointer<PreloadBuffer>	pBuffer = GetPreloadBuffer() ;
			if ( pBuffer == NULL )
			{
				break ;
			}
			size_t	nSampleBytes =
						GetChannelCount() * GetBitsPerSample() / 8 ;
			size_t	nBytes =
						pBuffer->m_miodh.dwSampleCount * nSampleBytes ;
			//
			uint8_t *	ptrWaveBuf ;
			bufTemp.SetLength( nBytes ) ;
			ptrWaveBuf = bufTemp.GetArray() ;
			//
			m_bstream->AttachInputStream( (PreloadBuffer*) pBuffer ) ;
			m_decoder.DecodeSound( *m_bstream, pBuffer->m_miodh, ptrWaveBuf ) ;
			bufTemp.FinishArray() ;
		}
		return ;
	}
	//
	// 既に読み込まれているブロックを破棄する
	//
	m_queueSound.SetLength( 0 ) ;
	//
	// リストに指定のサンプルを含むキーが登録されているか検索し、
	// 登録されていない場合には、シークする
	//////////////////////////////////////////////////////////////////////////
	const KeyPoint *	pKeyPoint = SearchKeySample( nSample ) ;
	if ( pKeyPoint == NULL )
	{
		if ( m_arrayKeySample.GetLength() > 0 )
		{
			pKeyPoint = m_arrayKeySample.GetLastAt( 0 ) ;
			m_erif.Seek( pKeyPoint->m_nRecOffset ) ;
			nCurrentSample = pKeyPoint->m_nKeySample ;
		}
		//
		// 各レコードを順次読み込む
		//
		for ( ; ; )
		{
			uint64_t	nRecPosition = m_erif.GetPosition() ;
			if ( m_erif.DescendChunk( "SoundStm" ) )
			{
				return ;
			}
			ERISA::MIO_DATA_HEADER	miodh ;
			m_erif.Read( &miodh, sizeof(miodh) ) ;
			m_erif.AscendChunk( ) ;
			//
			// キーポイントの設定
			//
			if ( miodh.bytFlags & mioDataLeadBlock )
			{
				pKeyPoint = SearchKeySample( nCurrentSample ) ;
				if ( pKeyPoint == NULL )
				{
					KeyPoint	keypoint ;
					keypoint.m_nKeySample = nCurrentSample ;
					keypoint.m_nRecOffset = nRecPosition ;
					AddKeySample( keypoint ) ;
					//
					pKeyPoint = m_arrayKeySample.GetLastAt( 0 ) ;
				}
			}
			//
			// 位置の更新
			//
			if ( (nCurrentSample <= nSample) &&
					((nSample - nCurrentSample) < miodh.dwSampleCount) )
			{
				break ;
			}
			nCurrentSample += miodh.dwSampleCount ;
		}
	}
	//
	// 指定のキーポイントからシークする
	//////////////////////////////////////////////////////////////////////////
	if ( pKeyPoint == NULL )
	{
		return ;
	}
	nCurrentSample = pKeyPoint->m_nKeySample ;
	m_erif.Seek( pKeyPoint->m_nRecOffset ) ;
	//
	for ( ; ; )
	{
		if ( m_erif.DescendChunk( "SoundStm" ) )
		{
			return ;
		}
		//
		// 音声データレコードを読み込む
		//
		const size_t	nDataBytes = (size_t) m_erif.GetLength( ) ;
		PreloadBuffer *	pBuffer =
			new PreloadBuffer( nDataBytes - sizeof(ERISA::MIO_DATA_HEADER) ) ;
		//
		pBuffer->m_nKeySample = nCurrentSample ;
		//
		m_erif.Read
			( &(pBuffer->m_miodh), sizeof(ERISA::MIO_DATA_HEADER) ) ;
		m_erif.Read
			( pBuffer->GetArray(),
				nDataBytes - sizeof(ERISA::MIO_DATA_HEADER) ) ;
		pBuffer->FinishArray() ;
		//
		m_erif.AscendChunk( ) ;
		//
		// 位置の更新
		//
		if ( (nCurrentSample <= nSample) &&
			((nSample - nCurrentSample) < pBuffer->m_miodh.dwSampleCount) )
		{
			nCurrentSample += pBuffer->m_miodh.dwSampleCount ;
			AddPreloadBuffer( pBuffer ) ;
			break ;
		}
		nCurrentSample += pBuffer->m_miodh.dwSampleCount ;
		//
		// データを展開して破棄する
		//
		size_t	nSampleBytes = GetChannelCount() * GetBitsPerSample() / 8 ;
		size_t	nBytes = pBuffer->m_miodh.dwSampleCount * nSampleBytes ;
		//
		uint8_t *	ptrWaveBuf ;
		bufTemp.SetLength( nBytes ) ;
		ptrWaveBuf = bufTemp.GetArray() ;
		//
		m_bstream->AttachInputStream( pBuffer ) ;
		m_decoder.DecodeSound( *m_bstream, pBuffer->m_miodh, ptrWaveBuf ) ;
		bufTemp.FinishArray() ;
		//
		delete	pBuffer ;
	}
}

