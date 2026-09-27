
/*****************************************************************************
                         E R I S A - L i b r a r y
 -----------------------------------------------------------------------------
    Copyright (C) 2002-2013 Leshade Entis, Entis-soft. All rights reserved.
 *****************************************************************************/


#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl_erisa_lib.h>
#include <sakuragl/sgl2d/sgl_image_buf_object.h>

using namespace SSystem ;
using namespace SakuraGL ;
using namespace ERISA ;


//////////////////////////////////////////////////////////////////////////////
// タグ情報
//////////////////////////////////////////////////////////////////////////////

// タグ情報文字列
//////////////////////////////////////////////////////////////////////////////
const wchar_t *	SGLMediaFile::m_pwszTagName[SGLMediaFile::tagMax] =
{
	L"title", L"vocal-player", L"composer", L"arranger",
	L"source", L"track", L"release-date", L"genre",
	L"rewind-point", L"end-loop-point", L"hot-spot", L"resolution",
	L"comment", L"words", L"reference-file",
} ;

// タグ情報を解釈
//////////////////////////////////////////////////////////////////////////////
void SGLMediaFile::STagInfo::ParseTagInfo( const wchar_t * pwszDesc )
{
	DeleteContents() ;
	//
	if ( pwszDesc == NULL )
	{
		return ;
	}
	if ( pwszDesc[0] != L'#' )
	{
		STagEntry *	pTag = new STagEntry ;
		pTag->m_tag = L"comment" ;
		pTag->m_contents = pwszDesc ;
		m_tags.Add( pTag ) ;
		return ;
	}
	SStringParser	sparsDesc( pwszDesc ) ;
	while ( !sparsDesc.IsIndexOverflow() )
	{
		if ( sparsDesc.HasToComeChar( L"#" ) != L'#' )
		{
			break ;
		}
		STagEntry *	pTag = new STagEntry ;
		m_tags.Add( pTag ) ;
		//
		sparsDesc.NextString( pTag->m_tag ) ;
		//
		sparsDesc.SeekToNextLine() ;
		sparsDesc.MarkIndex() ;
		//
		while ( !sparsDesc.IsIndexOverflow() )
		{
			if ( sparsDesc.CurrentCharacter() == L'#' )
			{
				if ( sparsDesc.OffsetAt(1) != L'#' )
				{
					break ;
				}
				pTag->m_contents += sparsDesc.SubStringFromMark() ;
				sparsDesc.GetCharacter() ;
				sparsDesc.MarkIndex() ;
			}
			sparsDesc.SeekToNextLine() ;
		}
		pTag->m_contents += sparsDesc.SubStringFromMark() ;
	}
}

// タグ情報をフォーマット
//////////////////////////////////////////////////////////////////////////////
void SGLMediaFile::STagInfo::FormatTagInfo( SSystem::SString& strDesc ) const
{
	uint16_t *	pwBuf = strDesc.LockBuffer( 1 ) ;
	pwBuf[0] = 0xFEFF ;
	strDesc.UnlockBuffer( 1 ) ;
	//
	const size_t	nCount = m_tags.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		STagEntry *	pTag = m_tags.GetAt( i ) ;
		if ( pTag == NULL )
		{
			continue ;
		}
		//
		// タグ名
		//
		strDesc += L'#' ;
		strDesc += pTag->m_tag ;
		strDesc += L"\r\n" ;
		//
		// 内容
		//
		size_t	iLast = 0 ;
		for ( ; ; )
		{
			ssize_t	iNext = pTag->m_contents.Find( L"\r\n#" ) ;
			if ( iNext >= 0 )
			{
				strDesc += pTag->m_contents.Middle( iLast, (ssize_t) (iNext - iLast) ) ;
				strDesc += L"\r\n##" ;
			}
			else
			{
				strDesc += pTag->m_contents.Middle( iLast ) ;
				break ;
			}
			iLast = iNext + 3 ;
		}
		if ( strDesc.Right(2) != L"\r\n" )
		{
			strDesc += L"\r\n" ;
		}
	}
}

// タグを追加する
//////////////////////////////////////////////////////////////////////////////
void SGLMediaFile::STagInfo::AddTag
	( SGLMediaFile::TagIndex iTag, const wchar_t * pwszContents )
{
	STagEntry *	pTag = new STagEntry ;
	pTag->m_tag = m_pwszTagName[iTag] ;
	pTag->m_contents = pwszContents ;
	m_tags.Add( pTag ) ;
}

// タグ情報のクリア
//////////////////////////////////////////////////////////////////////////////
void SGLMediaFile::STagInfo::DeleteContents( void )
{
	m_tags.RemoveAll( ) ;
}

// タグ情報取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t *
	SGLMediaFile::STagInfo::GetTagContents( const wchar_t * pwszTag ) const
{
	const size_t	nCount = m_tags.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		STagEntry *	pTag = m_tags.GetAt( i ) ;
		ESLAssert( pTag != NULL ) ;
		if ( pTag->m_tag == pwszTag )
		{
			return	pTag->m_contents ;
		}
	}
	return	NULL ;
}

const wchar_t *
	SGLMediaFile::STagInfo::GetTagContents
				( SGLMediaFile::TagIndex iTag ) const
{
	return	GetTagContents( SGLMediaFile::m_pwszTagName[iTag] ) ;
}

SGLMediaFile::STagEntry *
	SGLMediaFile::STagInfo::GetTagAs( const wchar_t * pwszTag ) const
{
	const size_t	nCount = m_tags.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		STagEntry *	pTag = m_tags.GetAt( i ) ;
		ESLAssert( pTag != NULL ) ;
		if ( pTag->m_tag == pwszTag )
		{
			return	pTag ;
		}
	}
	return	NULL ;
}

// トラック番号を取得
//////////////////////////////////////////////////////////////////////////////
int SGLMediaFile::STagInfo::GetTrackNumber( void ) const
{
	const wchar_t *	pwszTrack = GetTagContents( tagTrack ) ;
	if ( pwszTrack != NULL )
	{
		SStringParser	sparsTrack( pwszTrack ) ;
		int	typeNum = sparsTrack.IsNextNumber() ;
		if ( typeNum == SStringParser::numberInvalid )
		{
			return	-1 ;
		}
		return	(int) sparsTrack.NextInteger( typeNum ) ;
	}
	return	-1 ;
}

// リリース年月日を取得
//////////////////////////////////////////////////////////////////////////////
SSystem::SError
	SGLMediaFile::STagInfo::GetReleaseDate( SSystem::DATE_TIME& date ) const
{
	const wchar_t *	pwszDate = GetTagContents( tagReleaseDate ) ;
	if ( pwszDate == NULL )
	{
		return	errFailed ;
	}
	date.nYear = 0 ;
	date.nMonth = 0 ;
	date.nDay = 0 ;
	date.nWeek = 0 ;
	date.nHour = 0 ;
	date.nMinute = 0 ;
	date.nSecond = 0 ;
	date.nMilliSec = 0 ;
	//
	SStringParser	sparsDate( pwszDate ) ;
	int	typeNum ;
	typeNum = sparsDate.IsNextNumber() ;
	if ( typeNum == SStringParser::numberInvalid )
	{
		return	errFailed ;
	}
	date.nYear = (uint16_t) sparsDate.NextInteger( typeNum ) ;
	//
	if ( sparsDate.HasToComeChar( L"/" ) != L'/' )
	{
		return	errFailed ;
	}
	//
	typeNum = sparsDate.IsNextNumber() ;
	if ( typeNum == SStringParser::numberInvalid )
	{
		return	errFailed ;
	}
	date.nMonth = (uint16_t) sparsDate.NextInteger( typeNum ) ;
	//
	if ( sparsDate.HasToComeChar( L"/" ) != L'/' )
	{
		return	errFailed ;
	}
	typeNum = sparsDate.IsNextNumber() ;
	if ( typeNum == SStringParser::numberInvalid )
	{
		return	errFailed ;
	}
	date.nDay = (uint16_t) sparsDate.NextInteger( typeNum ) ;

	return	errSuccess ;
}

// ループポイントを取得
//////////////////////////////////////////////////////////////////////////////
int64_t SGLMediaFile::STagInfo::GetRewindPoint( size_t iEntry ) const
{
	const wchar_t *	pwszRewindPoint = GetTagContents( tagRewindPoint ) ;
	if ( pwszRewindPoint != NULL )
	{
		SStringParser	sparsRewind( pwszRewindPoint ) ;
		for ( size_t i = 0; i < iEntry; i ++ )
		{
			if ( !sparsRewind.SeekString( L"," ) )
			{
				return	-1 ;
			}
			sparsRewind.HasToComeChar( L"," ) ;
		}
		int	typeNum = sparsRewind.IsNextNumber() ;
		if ( typeNum == SStringParser::numberInvalid )
		{
			return	-1 ;
		}
		return	sparsRewind.NextInteger( typeNum ) ;
	}
	return	-1 ;
}

int64_t SGLMediaFile::STagInfo::GetLoopEndPoint( void ) const
{
	const wchar_t *	pwszLoopEndPoint = GetTagContents( tagLoopEndPoint ) ;
	if ( pwszLoopEndPoint != NULL )
	{
		SStringParser	sparsLoopEnd( pwszLoopEndPoint ) ;
		int	typeNum = sparsLoopEnd.IsNextNumber() ;
		if ( typeNum == SStringParser::numberInvalid )
		{
			return	-1 ;
		}
		return	sparsLoopEnd.NextInteger( typeNum ) ;
	}
	return	GetRewindPoint( 1 ) ;
}

// ホットスポットを取得
//////////////////////////////////////////////////////////////////////////////
SSystem::SError
	SGLMediaFile::STagInfo::GetHotSpot( SakuraGL::SGLPoint& ptHotSpot ) const
{
	const wchar_t *	pwszHotSpot = GetTagContents( tagHotSpot ) ;
	if ( pwszHotSpot == NULL )
	{
		return	errFailed ;
	}
	ptHotSpot.x = 0 ;
	ptHotSpot.y = 0 ;
	//
	SStringParser	sparsHotSpot( pwszHotSpot ) ;
	int	typeNum ;
	typeNum = sparsHotSpot.IsNextNumber() ;
	if ( typeNum == SStringParser::numberInvalid )
	{
		return	errFailed ;
	}
	ptHotSpot.x = (int32_t) sparsHotSpot.NextInteger( typeNum ) ;
	//
	if ( sparsHotSpot.HasToComeChar( L"," ) != L',' )
	{
		return	errFailed ;
	}
	//
	typeNum = sparsHotSpot.IsNextNumber() ;
	if ( typeNum == SStringParser::numberInvalid )
	{
		return	errFailed ;
	}
	ptHotSpot.y = (int32_t) sparsHotSpot.NextInteger( typeNum ) ;

	return	errSuccess ;
}

// 解像度を取得
//////////////////////////////////////////////////////////////////////////////
long int SGLMediaFile::STagInfo::GetResolution( void ) const
{
	const wchar_t *	pwszResolution = GetTagContents( tagResolution ) ;
	if ( pwszResolution != NULL )
	{
		SStringParser	sparsResolution( pwszResolution ) ;
		int	typeNum = sparsResolution.IsNextNumber() ;
		if ( typeNum == SStringParser::numberInvalid )
		{
			return	-1 ;
		}
		return	(long int) (sparsResolution.NextRealNumber(typeNum) * 100) ;
	}
	return	-1 ;
}


//////////////////////////////////////////////////////////////////////////////
// ERI メディアファイル
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ERISA::SGLMediaFile, SChunkFile )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLMediaFile::SGLMediaFile( void )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLMediaFile::~SGLMediaFile( void )
{
}

// メディアファイルを開く
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLMediaFile::OpenMediaFile
	( SSystem::SFileInterface * pFile,
		SGLMediaFile::OpenType type, bool flagOwner, long int nFlags )
{
	//
	// 読み込みフラグをクリア
	//////////////////////////////////////////////////////////////////////////
	m_flagsRead = 0 ;
	//
	m_tablePalette.RemoveAll() ;
	m_tableSequence.RemoveAll() ;
	//
	// EMCヘッダを読み込む
	//////////////////////////////////////////////////////////////////////////
	SError	err ;
	err = SChunkFile::OpenChunkFile( pFile, flagOwner, nFlags, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	if ( type == openRoot )
	{
		return	errSuccess ;
	}
	//
	// 情報ヘッダレコードを読み込む
	//////////////////////////////////////////////////////////////////////////
	err = DescendChunk( "Header  " ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// ファイルヘッダを読み込む
	//
	err = DescendChunk( "FileHdr " ) ;
	if ( err )
	{
		return	err ;
	}
	if ( Read
		( &m_eriFileHeader,
			sizeof(ERISA::ERI_FILE_HEADER) ) < sizeof(ERISA::ERI_FILE_HEADER) )
	{
		ESLTrace( "Failed to read ERI file header.\n" ) ;
		return	errFailed ;
	}
	AscendChunk( ) ;
	//
	m_flagsRead |= readFileHeader ;
	//
	// バージョン情報のチェック
	//
	if ( m_eriFileHeader.dwVersion > 0x00020100 )
	{
		ESLTrace( "Invalid ERI file version.\n" ) ;
		return	errFailed ;
	}
	//
	// 画像情報ヘッダを読み込む
	//
	for ( ; ; )
	{
		if ( DescendChunk( ) )
		{
			break ;
		}
		if ( IsEqualCurrentChunkID( "PrevwInf" ) )
		{
			// プレビュー画像情報ヘッダ
			if ( Read
				( &m_eriPreviewInfo,
					sizeof(ERISA::ERI_INFO_HEADER) )
							== sizeof(ERISA::ERI_INFO_HEADER) )
			{
				m_flagsRead |= readPreviewInfo ;
			}
		}
		else if ( IsEqualCurrentChunkID( "ImageInf" ) )
		{
			// 画像情報ヘッダ
			if ( Read
				( &m_eriInfoHeader,
					sizeof(ERISA::ERI_INFO_HEADER) )
							== sizeof(ERISA::ERI_INFO_HEADER) )
			{
				m_flagsRead |= readImageInfo ;
			}
		}
		else if ( IsEqualCurrentChunkID( "SoundInf" ) )
		{
			// 音声情報ヘッダ
			if ( Read
				( &m_mioInfoHeader,
					sizeof(ERISA::MIO_INFO_HEADER) )
							== sizeof(ERISA::MIO_INFO_HEADER) )
			{
				m_flagsRead |= readSoundInfo ;
			}
		}
		else if ( IsEqualCurrentChunkID( "Sequence" ) )
		{
			// シーケンステーブルレコード
			size_t	nSeqLength =
				(size_t) GetCurrentChunkLength() / sizeof(SEQUENCE_DELTA) ;
			size_t	nBytes = nSeqLength * sizeof(SEQUENCE_DELTA) ;
			//
			m_tableSequence.SetLength( nSeqLength ) ;
			//
			if ( Read( m_tableSequence.GetArray(), nBytes ) == nBytes )
			{
				m_flagsRead |= readSequenceTable ;
			}
			m_tableSequence.FinishArray() ;
		}
		else
		{
			// 著作権情報・コメントなど
			int	nType = -1 ;
			if ( IsEqualCurrentChunkID( "cpyright" ) )
			{
				nType = 0 ;
				m_flagsRead |= readCopyright ;
			}
			else if ( IsEqualCurrentChunkID( "descript" ) )
			{
				nType = 1 ;
				m_flagsRead |= readDescription ;
			}
			if ( nType >= 0 )
			{
				SByteBuffer	bufDesc ;
				bufDesc.ReadFromFile( *this ) ;
				//
				const size_t	nDescLength = (size_t) bufDesc.GetLength() ;
				const uint8_t *	ptrDesc = bufDesc.GetConstArray() ;
				if ( (bufDesc.GetLength() >= 2) &&
					(ptrDesc[0] == 0xff) && (ptrDesc[1] == 0xfe) )
				{
					if ( nType == 0 )
					{
						m_strCopyright.SetString
							( (const uint16_t*) (ptrDesc + 2),
									(ssize_t) (nDescLength / 2) - 1 ) ;
					}
					else
					{
						m_strDescription.SetString
							( (const uint16_t*) (ptrDesc + 2),
									(ssize_t) (nDescLength / 2) - 1 ) ;
					}
				}
				else
				{
					if ( nType == 0 )
					{
						Charset::Decode
							( m_strCopyright,
								Charset::encodingUTF8,
									ptrDesc, (ssize_t) nDescLength ) ;
					}
					else
					{
						Charset::Decode
							( m_strDescription,
								Charset::encodingUTF8,
									ptrDesc, (ssize_t) nDescLength ) ;
					}
				}
			}
		}
		//
		// 次のレコードへ
		//
		AscendChunk( ) ;
	}
	//
	AscendChunk( ) ;
	//
	// 圧縮オプションのチェック
	//
	if ( !(m_flagsRead & readImageInfo) && !(m_flagsRead & readSoundInfo) )
	{
		return	errFailed ;
	}
	if ( type == readHeader )
	{
		return	errSuccess ;
	}
	//
	// ストリームレコードを開く
	//////////////////////////////////////////////////////////////////////////
	err = DescendChunk( "Stream  " ) ;
	if ( err )
	{
		return	err ;
	}
	if ( type == openStream )
	{
		return	errSuccess ;
	}
	//
	// 画像データレコードを捜索
	//
	for ( ; ; )
	{
		err = DescendChunk( ) ;
		if ( err )
		{
			return	err ;
		}
		if ( IsEqualCurrentChunkID( "ImageFrm" ) )
		{
			break ;
		}
		if ( IsEqualCurrentChunkID( "Palette " ) )
		{
			// パレットテーブル読み込み
			size_t	nPaletteLength =
				(size_t) GetCurrentChunkLength() / sizeof(SGLPalette) ;
			m_tablePalette.SetLength( nPaletteLength ) ;
			//
			eslFillMemory
				( m_tablePalette.GetArray(),
					0, nPaletteLength * sizeof(SGLPalette) ) ;
			Read( m_tablePalette.GetArray(),
					nPaletteLength * sizeof(SGLPalette) ) ;
			m_tablePalette.FinishArray() ;
			//
			m_flagsRead |= readPaletteTable ;
		}
		AscendChunk( ) ;
	}
	return	errSuccess ;
}



//////////////////////////////////////////////////////////////////////////////
// ERI メディアファイル出力
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ERISA::SGLMediaFileWriter, SGLMediaFile )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLMediaFileWriter::SGLMediaFileWriter( void )
{
	m_wsStatus = wsNotOpened ;
	//
	m_fWithSeqTable = false ;
	m_nKeyFrame = 15 ;
	m_nBidirectKey = 3 ;
	m_nKeyWave = 60 ;
	m_nFrameCount = 0 ;
	m_nWaveCount = 0 ;
	m_nDiffFrames = 0 ;
	//
	m_fposMioHeader = 0 ;
	m_nOutputWaveSamples = 0 ;
	//
	m_fKeyWaveBlock = false ;
	m_nWaveBufSamples = 0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLMediaFileWriter::~SGLMediaFileWriter( void )
{
	Close() ;
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLMediaFileWriter::OpenMediaFile
	( SSystem::SFileInterface * pFile,
		SGLMediaFileWriter::MediaFileIdentity fidType,
		bool flagOwner, long int nFlags )
{
	//
	// 既にファイルを開いている場合には閉じる
	//
	Close( ) ;
	//
	// ERI ファイルを開く
	//
	static const char	szFileDetails[3][0x30] =
	{
		"Entis Rasterized Image",
		"Music Interleaved and Orthogonal transformed",
		"Moving Entis Image"
	} ;
	SChunkFile::FILE_HEADER	fhdr ;
	fhdr.SetHeaderInfo
		( SChunkFile::fidRasterizedImage, &szFileDetails[fidType][0] ) ;
	if ( OpenChunkFile( pFile, flagOwner, SFileOpener::modeCreateFile, &fhdr ) )
	{
		return	errFailed ;
	}
	//
	// 成功
	//
	m_wsStatus = wsOpened ;

	return	errSuccess ;
}

// リソースを解放する
//////////////////////////////////////////////////////////////////////////////
void SGLMediaFileWriter::Close( void )
{
	//
	// レコードを閉じる
	//
	if ( m_wsStatus == wsWritingStream )
	{
		EndFileHeader( ) ;
	}
	else if ( m_wsStatus == wsWritingHeader )
	{
		EndStream( 0 ) ;
	}
	//
	// 圧縮オブジェクト解放
	//
	if ( m_pencImage1 != NULL )
	{
		m_pencImage1->Delete() ;
		m_pencImage1 = NULL ;
	}
	if ( m_pencImage2 != NULL )
	{
		m_pencImage2->Delete() ;
		m_pencImage2 = NULL ;
	}
	if ( m_pencSound != NULL )
	{
		m_pencSound->Delete() ;
		m_pencSound = NULL ;
	}
	//
	// フレームバッファ解放
	//
	m_pLastImage = NULL ;
	m_pNextImage = NULL ;
	m_pCurImage = NULL ;
	m_arrFrameBuf.RemoveAll() ;
	//
	// ファイルを閉じる
	//
	if ( m_wsStatus != wsNotOpened )
	{
		SGLMediaFile::Close( ) ;
		m_wsStatus = wsNotOpened ;
	}
}

// ファイルヘッダを開く
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLMediaFileWriter::BeginFileHeader
	( size_t nKeyFrame, size_t nKeyWave, size_t nBidirectKey )
{
	//
	// パラメータの検証と設定
	//
	if ( m_wsStatus != wsOpened )
	{
		return	errFailed ;
	}
	m_nKeyFrame = nKeyFrame ;
	m_nBidirectKey = nBidirectKey ;
	m_nKeyWave = nKeyWave ;
	if ( m_nBidirectKey == 0 )
	{
		m_nBidirectKey = 1 ;
	}
	//
	// ヘッダレコードを開く
	//
	SError	err ;
	err = DescendChunk( "Header  " ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// ファイルヘッダを書き出す（ダミー出力）
	//
	err = DescendChunk( "FileHdr " ) ;
	if ( err )
	{
		return	err ;
	}
	m_eriFileHeader.dwVersion = eriFileStandardVersinon ;
	m_eriFileHeader.dwContainedFlag = 0 ;
	m_eriFileHeader.dwKeyFrameCount = (DWORD) m_nKeyFrame ;
	m_eriFileHeader.dwFrameCount = 0 ;
	m_eriFileHeader.dwAllFrameTime = 0 ;
	Write( &m_eriFileHeader, sizeof(ERISA::ERI_FILE_HEADER) ) ;
	AscendChunk( ) ;
	m_flagsRead = readFileHeader ;
	//
	m_fWithSeqTable = false ;
	m_nFrameCount = 0 ;
	m_nWaveCount = 0 ;
	m_fposMioHeader = 0 ;
	m_nOutputWaveSamples = 0 ;
	m_fKeyWaveBlock = true ;
	m_nWaveBufSamples = 0 ;
	m_bufWaveBuffer.ClearAll( ) ;
	//
	// 成功
	//
	m_wsStatus = wsWritingHeader ;

	return	errSuccess ;
}

// プレビュー画像情報ヘッダを書き出す
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLMediaFileWriter::WritePreviewInfo( const ERISA::ERI_INFO_HEADER & eih )
{
	if ( m_wsStatus != wsWritingHeader )
	{
		return	errFailed ;
	}
	SError	err ;
	err = DescendChunk( "PrevwInf" ) ;
	if ( err )
	{
		return	err ;
	}
	m_eriPreviewInfo = eih ;
	Write( &m_eriPreviewInfo, sizeof(ERISA::ERI_INFO_HEADER) ) ;
	AscendChunk( ) ;
	m_flagsRead |= readPreviewInfo ;

	return	errSuccess ;
}

// 画像情報ヘッダを書き出す
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLMediaFileWriter::WriteEriInfoHeader( const ERISA::ERI_INFO_HEADER & eih )
{
	if ( m_wsStatus != wsWritingHeader )
	{
		return	errFailed ;
	}
	SError	err ;
	err = DescendChunk( "ImageInf" ) ;
	if ( err )
	{
		return	err ;
	}
	m_eriInfoHeader = eih ;
	Write( &m_eriInfoHeader, sizeof(ERISA::ERI_INFO_HEADER) ) ;
	AscendChunk( ) ;
	m_flagsRead |= readImageInfo ;
	//
	m_eriFileHeader.dwContainedFlag |= eriFileContainImage ;

	return	errSuccess ;
}

// 音声情報ヘッダを書き出す
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLMediaFileWriter::WriteMioInfoHeader( const ERISA::MIO_INFO_HEADER & mih )
{
	if ( m_wsStatus != wsWritingHeader )
	{
		return	errFailed ;
	}
	m_fposMioHeader = GetPosition( ) ;
	//
	SError	err ;
	err = DescendChunk( "SoundInf" ) ;
	if ( err )
	{
		return	err ;
	}
	m_mioInfoHeader = mih ;
	Write( &m_mioInfoHeader, sizeof(ERISA::MIO_INFO_HEADER) ) ;
	AscendChunk( ) ;
	m_flagsRead |= readSoundInfo ;
	//
	m_eriFileHeader.dwContainedFlag |= eriFileContainWave ;

	return	errSuccess ;
}

// 著作権情報を書き出す
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLMediaFileWriter::WriteCopyright( const wchar_t * pwszCopyright, ssize_t nLength )
{
	if ( m_wsStatus != wsWritingHeader )
	{
		return	errFailed ;
	}
	SError	err ;
	err = DescendChunk( "cpyright" ) ;
	if ( err )
	{
		return	err ;
	}
	m_strCopyright = SString( pwszCopyright, nLength ) ;
	uint8_t	uh[2] = { 0xff, 0xfe } ;
	Write( &uh[0], 2 ) ;
	Write( m_strCopyright.GetConstArray(),
			m_strCopyright.GetLength() * sizeof(uint16_t) ) ;
	AscendChunk( ) ;
	m_flagsRead |= readCopyright ;

	return	errSuccess ;
}

// コメントを書き出す
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLMediaFileWriter::WriteDescription( const wchar_t * pwszDescription, ssize_t nLength )
{
	if ( m_wsStatus != wsWritingHeader )
	{
		return	errFailed ;
	}
	SError	err ;
	err = DescendChunk( "descript" ) ;
	if ( err )
	{
		return	err ;
	}
	m_strDescription = SString( pwszDescription, nLength ) ;
	uint8_t	uh[2] = { 0xff, 0xfe } ;
	Write( &uh[0], 2 ) ;
	Write( m_strDescription.GetConstArray(),
			m_strDescription.GetLength() * sizeof(uint16_t) ) ;
	AscendChunk( ) ;
	m_flagsRead |= readDescription ;

	return	errSuccess ;
}

// シーケンステーブルを書き出す
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLMediaFileWriter::WriteSequenceTable
	( SGLMediaFile::SEQUENCE_DELTA * pSequence, size_t nLength )
{
	if ( m_wsStatus != wsWritingHeader )
	{
		return	errFailed ;
	}
	SError	err ;
	err = DescendChunk( "Sequence" ) ;
	if ( err )
	{
		return	err ;
	}
	Write( pSequence, nLength * sizeof(SGLMediaFile::SEQUENCE_DELTA) ) ;
	AscendChunk( ) ;
	//
	m_tableSequence.RemoveAll() ;
	m_tableSequence.AddArray( pSequence, nLength ) ;
	m_flagsRead |= readSequenceTable ;
	//
	m_eriFileHeader.dwFrameCount = (DWORD) nLength ;
	m_fWithSeqTable = true ;

	return	errSuccess ;
}

// ファイルヘッダを閉じる
//////////////////////////////////////////////////////////////////////////////
void SGLMediaFileWriter::EndFileHeader( void )
{
	if ( m_wsStatus == wsWritingHeader )
	{
		m_fposEndOfHeader = GetPosition( ) ;
		AscendChunk( ) ;
		m_wsStatus = wsOpened ;
	}
}

// 画像の圧縮パラメータを設定する
//////////////////////////////////////////////////////////////////////////////
void SGLMediaFileWriter::SetImageCompressionParameter
		( const SGLImageEncoder::Parameter & iencp )
{
	m_iencp_i = iencp ;
	m_iencp_p = iencp ;
	m_iencp_p.m_fpYScaleDC *= iencp.m_fpPFrameScale ;
	m_iencp_p.m_fpCScaleDC *= iencp.m_fpPFrameScale ;
	m_iencp_p.m_fpYScaleLow = m_iencp_p.m_fpYScaleDC * 0.75F ;
	m_iencp_p.m_fpCScaleLow = m_iencp_p.m_fpCScaleDC * 0.75F ;
	m_iencp_p.m_fpYScaleHigh = m_iencp_p.m_fpYScaleDC * 0.5F ;
	m_iencp_p.m_fpCScaleHigh = m_iencp_p.m_fpCScaleDC * 0.5F ;
	m_iencp_p.m_nYLPFThreshold = 64 - (64 - m_iencp_p.m_nYLPFThreshold) / 3 ;
	m_iencp_p.m_nCLPFThreshold = 64 - (64 - m_iencp_p.m_nCLPFThreshold) / 3 ;
	m_iencp_p.m_nMaxFrameSize =
		(size_t) (iencp.m_nMaxFrameSize * iencp.m_fpPFrameScale * 0.65) ;
	m_iencp_p.m_nMinFrameSize =
		(size_t) (iencp.m_nMinFrameSize * iencp.m_fpPFrameScale * 0.4) ;
	m_iencp_b = iencp ;
	m_iencp_b.m_fpYScaleDC *= iencp.m_fpBFrameScale ;
	m_iencp_b.m_fpCScaleDC *= iencp.m_fpBFrameScale ;
	m_iencp_b.m_fpYScaleLow = m_iencp_b.m_fpYScaleDC * 0.75F ;
	m_iencp_b.m_fpCScaleLow = m_iencp_b.m_fpCScaleDC * 0.75F ;
	m_iencp_b.m_fpYScaleHigh = m_iencp_b.m_fpYScaleDC * 0.5F ;
	m_iencp_b.m_fpCScaleHigh = m_iencp_b.m_fpCScaleDC * 0.5F ;
	m_iencp_b.m_nYLPFThreshold = (m_iencp_p.m_nYLPFThreshold + 64) / 2 ;
	m_iencp_b.m_nCLPFThreshold = (m_iencp_p.m_nCLPFThreshold + 64) / 2 ;
	m_iencp_b.m_nMaxFrameSize =
		(size_t) (iencp.m_nMaxFrameSize * iencp.m_fpBFrameScale * 0.5) ;
	m_iencp_b.m_nMinFrameSize =
		(size_t) (iencp.m_nMinFrameSize * iencp.m_fpBFrameScale * 0.333) ;
	//
	if ( m_pencImage1 != NULL )
	{
		m_pencImage1->SetCompressionParameter( m_iencp_i ) ;
	}
	if ( m_pencImage2 != NULL )
	{
		m_pencImage2->SetCompressionParameter( m_iencp_p ) ;
	}
}

// 音声の圧縮パラメータを設定する
//////////////////////////////////////////////////////////////////////////////
void SGLMediaFileWriter::SetSoundCompressionParameter
		( const SGLSoundEncoder::Parameter & sencp )
{
	m_sencp = sencp ;
	//
	if ( m_pencSound != NULL )
	{
		m_pencSound->SetCompressionParameter( sencp ) ;
	}
}

// ストリームを開始する
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLMediaFileWriter::BeginStream( void )
{
	//
	// ステータスチェック
	//
	if ( m_wsStatus != wsOpened )
	{
		return	errFailed ;
	}
	//
	// 圧縮オブジェクト初期化
	//
	if ( m_eriFileHeader.dwContainedFlag & eriFileContainImage )
	{
		m_pencImage1 = new SGLImageEncoder ;
		m_pencImage2 = new SGLImageEncoder ;
		if ( m_pencImage1->Initialize( m_eriInfoHeader )
			|| m_pencImage2->Initialize( m_eriInfoHeader ) )
		{
			ESLTrace( "画像エンコーダの初期化に失敗しました。\n" ) ;
			return	errFailed ;
		}
		m_pencImage1->SetCompressionParameter( m_iencp_i ) ;
		m_pencImage2->SetCompressionParameter( m_iencp_p ) ;
	}
	if ( m_eriFileHeader.dwContainedFlag & eriFileContainWave )
	{
		m_pencSound = new SGLSoundEncoder ;
		if ( m_pencSound->Initialize( m_mioInfoHeader ) )
		{
			ESLTrace( "音声エンコーダの初期化に失敗しました。\n" ) ;
			return	errFailed ;
		}
		m_pencSound->SetCompressionParameter( m_sencp ) ;
	}
	//
	// 差分処理用バッファ生成
	//
	if ( m_eriFileHeader.dwContainedFlag & eriFileContainImage )
	{
		m_pLastImage = CreateImageBuffer( m_eriInfoHeader ) ;
		m_pNextImage = CreateImageBuffer( m_eriInfoHeader ) ;
		m_pCurImage = CreateImageBuffer( m_eriInfoHeader ) ;
	}
	//
	// ストリームレコードを開く
	//
	m_wsStatus = wsWritingStream ;
	//
	if ( DescendChunk( "Stream  " ) )
	{
		EndStream( 0 ) ;
		return	errFailed ;
	}
	return	errSuccess ;
}

// パレットテーブルを書き出す
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLMediaFileWriter::WritePaletteTable
	( const SakuraGL::SGLPalette * paltbl, size_t nLength )
{
	if ( m_wsStatus != wsWritingStream )
	{
		return	errFailed ;
	}
	if ( DescendChunk( "Palette " ) )
	{
		return	errFailed ;
	}
	Write( paltbl, nLength * sizeof(SGLPalette) ) ;
	AscendChunk( ) ;
	//
	m_eriFileHeader.dwContainedFlag |= eriFileContainPalette ;
	//
	m_tablePalette.RemoveAll() ;
	m_tablePalette.AddArray( paltbl, nLength ) ;

	return	errSuccess ;
}

// プレビュー画像を出力する
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLMediaFileWriter::WritePreviewData
		( SakuraGL::SGLImageObject & image, uint32_t flagsEncode )
{
	if ( m_wsStatus != wsWritingStream )
	{
		return	errFailed ;
	}
	if ( DescendChunk( "Preview " ) )
	{
		return	errFailed ;
	}
	//
	// 画像圧縮
	//
	SError	errResult = errFailed ;
	do
	{
		SGLEncodeBitStream	bstream( 0x10000 ) ;
		SGLImageEncoder		encoder ;
		bstream.AttachOutputStream( this ) ;
		//
		if ( encoder.Initialize( m_eriPreviewInfo ) )
		{
			break ;
		}
		SGLImageInfo	imginf ;
		uint8_t *		pImageBuf =
			image.LockBuffer( imginf, SGLImageObject::lockRead ) ;
		//
		if ( encoder.EncodeImage
				( imginf, pImageBuf, bstream, flagsEncode ) )
		{
			image.UnlockBuffer( SGLImageObject::lockRead ) ;
			break ;
		}
		image.UnlockBuffer( SGLImageObject::lockRead ) ;
		//
		errResult = errSuccess ;
	}
	while ( false ) ;
	//
	AscendChunk( ) ;

	return	errResult ;
}

// 音声データを出力する
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLMediaFileWriter::WriteWaveData
		( const void * ptrWaveBuf, size_t nSampleCount )
{
	if ( m_wsStatus != wsWritingStream )
	{
		return	errFailed ;
	}
	if ( m_eriFileHeader.dwContainedFlag & eriFileContainImage )
	{
		size_t	nWaveBytes =
			nSampleCount * (m_mioInfoHeader.dwChannelCount
							* m_mioInfoHeader.dwBitsPerSample / 8) ;
		m_bufWaveBuffer.Write( ptrWaveBuf, nWaveBytes ) ;
		m_nWaveBufSamples += nSampleCount ;
		return	errSuccess ;
	}
	if ( DescendChunk( "SoundStm" ) )
	{
		return	errFailed ;
	}
	//
	// 音声圧縮
	//
	SError	errResult = errFailed ;
	MIO_DATA_HEADER	miodh ;
	miodh.bytVersion = 1 ;
	miodh.bytFlags = 0 ;
	miodh.bytReserved1 = 0 ;
	miodh.bytReserved2 = 0 ;
	miodh.dwSampleCount = (DWORD) nSampleCount ;
	if ( ((m_nKeyWave == 0) && (m_nWaveCount == 0))
				|| (m_nWaveCount % m_nKeyWave) == 0 )
	{
		miodh.bytFlags = mioDataLeadBlock ;
	}
	if ( Write( &miodh, sizeof(MIO_DATA_HEADER) ) == sizeof(MIO_DATA_HEADER) )
	{
		SGLEncodeBitStream	bstream( 0x10000 ) ;
		bstream.AttachOutputStream( this ) ;
		if ( !m_pencSound->EncodeSound( bstream, miodh, ptrWaveBuf ) )
		{
			++ m_nWaveCount ;
			m_nOutputWaveSamples += nSampleCount ;
			errResult = errSuccess ;
		}
	}
	AscendChunk( ) ;

	return	errResult ;
}

// 画像データを出力する
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLMediaFileWriter::WriteImageData
		( SakuraGL::SGLImageObject & image, uint32_t flagsEncode )
{
	if ( m_wsStatus != wsWritingStream )
	{
		return	errFailed ;
	}
	//
	// キーフレームかどうかを判定
	//
	bool	fKeyFrame = (m_nFrameCount == 0) ;
	if ( m_nKeyFrame != 0 )
	{
		fKeyFrame = fKeyFrame || ((m_nFrameCount % m_nKeyFrame) == 0) ;
	}
	if ( !fKeyFrame
		&& (m_eriInfoHeader.fdwTransformation != eriTransformationLossless) )
	{
		if ( ((m_nDiffFrames + 1) % m_nBidirectKey) != 0 )
		{
			//
			// B ピクチャは一旦バッファに蓄積しておく
			//
			SGLImageObject *
				pFrameBuf = CreateImageBuffer( m_eriInfoHeader ) ;
			SGLImageInfo	imginf ;
			uint8_t *	ptrBuffer =
				pFrameBuf->LockBuffer( imginf, SGLImageObject::lockWrite ) ;
			image.ReadFrameBuffer
				( imginf, ptrBuffer, image.GetSelectedFrame() ) ;
			pFrameBuf->UnlockBuffer( SGLImageObject::lockWrite ) ;
			//
			m_arrEncFlags.Add( flagsEncode ) ;
			m_arrFrameBuf.Add( pFrameBuf ) ;
			m_nFrameCount ++ ;
			m_nDiffFrames ++ ;
			return	errSuccess ;
		}
	}
	//
	// 差分処理フレームを圧縮
	//
	SSmartBuffer		sbufIFrame ;
	SSmartBuffer		sbufPFrame ;
	SGLImageBuffer		imgCurFrame ;
	SGLImageSmartBuffer	isbCurFrame( imgCurFrame, m_pCurImage.Ptr() ) ;
	//
	if ( !fKeyFrame )
	do
	{
		//
		// 差分処理
		//
		uint32_t	flagDifEnc =
						flagsEncode & ~SGLImageEncoder::flagDifferential ;
		//
		SGLImageBuffer		imgLastFrame ;
		SGLImageSmartBuffer	isbLastFrame
				( imgLastFrame, m_pLastImage.Ptr(), SGLImageObject::lockRead ) ;
		//
		image.ReadFrameBuffer
			( imgCurFrame, imgCurFrame.ptrBuffer, image.GetSelectedFrame() ) ;
		if ( m_eriInfoHeader.fdwTransformation == eriTransformationLossless )
		{
			eriWrapAroundSubImageBuffer( imgCurFrame, imgLastFrame ) ;
		}
		else
		{
			int	nAbsMaxDiff = 0x7FFFFFFF ;
			if ( flagsEncode & SGLImageEncoder::flagNoMoveVector )
			{
				nAbsMaxDiff =
					SGLImageEncoder::MakeSubtractionBlock
									( imgCurFrame, imgLastFrame ) ;
			}
			else
			{
				m_pencImage2->ProcessMovingVector
					( imgCurFrame, imgLastFrame, nAbsMaxDiff ) ;
			}
			if ( nAbsMaxDiff >= m_iencp_i.m_nAMDFThreshold )
			{
				fKeyFrame = true ;
				m_pencImage2->ClearMovingVector( ) ;
				break ;
			}
			flagDifEnc |= SGLImageEncoder::flagDifferential ;
		}
		//
		// 差分フレーム圧縮開始
		//
		SGLEncodeBitStream	bstream( 0x10000 ) ;
		bstream.AttachOutputStream( &sbufPFrame ) ;
		//
		m_pencImage2->SetCompressionParameter( m_iencp_p ) ;
		if ( m_pencImage2->EncodeImage
			( imgCurFrame, imgCurFrame.ptrBuffer, bstream, flagDifEnc ) )
		{
			return	errFailed ;
		}
	}
	while ( false ) ;
	//
	// 独立フレームを圧縮
	//
	SGLEncodeBitStream	bstream( 0x10000 ) ;
	bstream.AttachOutputStream( &sbufIFrame ) ;
	//
	SError	errResult ;
	m_pencImage1->SetCompressionParameter( m_iencp_i ) ;
	image.ReadFrameBuffer
		( imgCurFrame, imgCurFrame.ptrBuffer, image.GetSelectedFrame() ) ;
	errResult =
		m_pencImage1->EncodeImage
			( imgCurFrame, imgCurFrame.ptrBuffer, bstream, flagsEncode ) ;
	if ( errResult )
	{
		return	errResult ;
	}
	//
	// 圧縮されたデータを取得する
	//
	SSmartBuffer *	psbufData = NULL ;
	const char *	pszRecID = NULL ;
	//
	if ( fKeyFrame )
	{
		psbufData = &sbufIFrame ;
		pszRecID = "ImageFrm" ;
		m_nDiffFrames = 0 ;
	}
	else
	{
		int64_t	nSize1 = sbufIFrame.GetLength() ;
		int64_t	nSize2 = sbufPFrame.GetLength() ;
		m_nDiffFrames ++ ;
		if ( m_eriInfoHeader.fdwTransformation != eriTransformationLossless )
		{
			nSize2 = nSize2 * ((m_nDiffFrames / m_nBidirectKey) + 7) / 8 ;
		}
		if ( nSize1 < nSize2 )
		{
			fKeyFrame = true ;
			psbufData = &sbufIFrame ;
			pszRecID = "ImageFrm" ;
			m_nDiffFrames = 0 ;
		}
		else
		{
			psbufData = &sbufPFrame ;
			pszRecID = "DiffeFrm" ;
		}
	}
	//
	// フレームを１つ進める
	//
	SByteBuffer	bufEncoded ;
	bufEncoded.SetLength( (size_t) psbufData->GetLength() ) ;
	psbufData->WriteToStream( bufEncoded ) ;
	//
	++ m_nFrameCount ;
	if ( m_eriInfoHeader.fdwTransformation == eriTransformationLossless )
	{
		//
		// 可逆圧縮の場合には、現在のフレームを直前フレームとして複製する
		//
		SGLImageBuffer		imgLastFrame ;
		SGLImageSmartBuffer	isbLastFrame
				( imgLastFrame, m_pLastImage.Ptr(), SGLImageObject::lockWrite ) ;
		sglCopyImageBuffer( imgLastFrame, imgCurFrame ) ;
	}
	else
	{
		//
		// 非可逆圧縮の場合には、実際にデータを展開して
		// 圧縮して劣化した画像を取得する
		//
		SGLImageBuffer		imgLastFrame ;
		SGLImageSmartBuffer	isbLastFrame
				( imgLastFrame, m_pLastImage.Ptr(), SGLImageObject::lockRead ) ;
		SGLImageBuffer		imgNextFrame ;
		SGLImageSmartBuffer	isbNextFrame( imgNextFrame, m_pNextImage.Ptr() ) ;
		//
		SGLDecodeBitStream	bstream( 0x10000 ) ;
		SGLImageDecoder		decoder ;
		bufEncoded.Seek( 0 ) ;
		bstream.AttachInputStream( &bufEncoded ) ;
		do
		{
			errResult = decoder.Initialize( m_eriInfoHeader ) ;
			if ( errResult )
			{
				break ;
			}
			uint32_t	flagDifDec = 0 ;
			if ( !fKeyFrame )
			{
				flagDifDec = SGLImageDecoder::flagDifferential ;
				decoder.SetRefPreviousFrame
						( &imgLastFrame, imgLastFrame.ptrBuffer ) ;
			}
			errResult =
				decoder.DecodeImage
					( imgNextFrame, imgNextFrame.ptrBuffer, bstream,
							flagDifDec | SGLImageDecoder::flagNoHalfFilter ) ;
		}
		while ( false ) ;
		//
		// B ピクチャがある場合には B ピクチャを圧縮する
		//
		SGLImageEncoder::BlendBlockHalfImage
			( imgNextFrame, imgNextFrame, imgCurFrame ) ;
		if ( !errResult )
		{
			errResult = WriteBirectionalFrames( ) ;
		}
		sglCopyImageBuffer( imgLastFrame, imgNextFrame ) ;
	}
	//
	// 音声データを書き出す
	//
	m_fKeyWaveBlock |= fKeyFrame ;
	if ( m_nWaveBufSamples > 0 )
	{
		WriteWaveBuffer( ) ;
	}
	//
	// データを書き出す
	//
	if ( DescendChunk( pszRecID ) )
	{
		return	errFailed ;
	}
	Write( bufEncoded.GetConstArray(), (size_t) bufEncoded.GetLength() ) ;
	AscendChunk( ) ;

	return	errResult ;
}

SSystem::SError SGLMediaFileWriter::WriteImageData
		( SakuraGL::SGLImageObject & image )
{
	return	WriteImageData( image, 0 ) ;
}

// ストリームを閉じる
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLMediaFileWriter::EndStream( uint32_t msecTotalTime )
{
	if ( m_wsStatus != wsWritingStream )
	{
		return	errFailed ;
	}
	//
	// B ピクチャが残っている場合には、
	// 最後のフレームを I ピクチャとしてエンコードしなおす
	//
	if ( m_arrFrameBuf.GetLength() > 0 )
	{
		size_t	nSaveKeyFrame = m_nKeyFrame ;
		SGLImageObject *	pLastFrame = m_arrFrameBuf.Pop() ;
		m_nKeyFrame = 1 ;
		//
		WriteImageData( *pLastFrame, 0 ) ;
		//
		delete	pLastFrame ;
		m_nKeyFrame = nSaveKeyFrame ;
	}
	//
	// 音声データが残っている場合にはエンコードして出力する
	//
	if ( m_nWaveBufSamples > 0 )
	{
		WriteWaveBuffer( ) ;
	}
	//
	// 圧縮オブジェクトの利用しているリソースを破棄
	//
	if ( m_eriFileHeader.dwContainedFlag & eriFileContainImage )
	{
		m_pencImage1 = NULL ;
		m_pencImage2 = NULL ;
	}
	if ( m_pencSound != NULL )
	{
		m_pencSound = NULL ;
	}
	//
	// バッファを削除
	//
	m_pLastImage = NULL ;
	m_pNextImage = NULL ;
	m_pCurImage = NULL ;
	m_arrFrameBuf.RemoveAll() ;
	m_arrEncFlags.RemoveAll() ;
	//
	// ストリームレコードを閉じる
	//
	AscendChunk() ;
	m_wsStatus = wsOpened ;
	//
	// ファイルヘッダを再出力
	//
	Seek( 0 ) ;
	//
	if ( DescendChunk( "Header  " )
		|| DescendChunk( "FileHdr " ) )
	{
		return	errFailed ;
	}
	if ( !m_fWithSeqTable )
	{
		m_eriFileHeader.dwFrameCount = (DWORD) m_nFrameCount ;
	}
	m_eriFileHeader.dwAllFrameTime = (DWORD) msecTotalTime ;
	Write( &m_eriFileHeader, sizeof(ERISA::ERI_FILE_HEADER) ) ;
	AscendChunk( ) ;
	//
	// 音声情報ヘッダを再出力
	//
	if ( m_eriFileHeader.dwContainedFlag & eriFileContainWave )
	{
		Seek( m_fposMioHeader ) ;
		DescendChunk( "SoundInf" ) ;
		m_mioInfoHeader.dwAllSampleCount = (DWORD) m_nOutputWaveSamples ;
		Write( &m_mioInfoHeader, sizeof(ERISA::MIO_INFO_HEADER) ) ;
		AscendChunk( ) ;
	}
	//
	Seek( m_fposEndOfHeader ) ;
	AscendChunk( ) ;

	return	errSuccess ;
}

// B ピクチャを圧縮して書き出す
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLMediaFileWriter::WriteBirectionalFrames( void )
{
	//
	// エンコーダをセットアップする
	//
	m_pencImage2->SetCompressionParameter( m_iencp_b ) ;
	//
	// 順次フレームを圧縮
	//
	SGLImageBuffer		imgLastFrame ;
	SGLImageBuffer		imgNextFrame ;
	SGLImageSmartBuffer	isbLastFrame
			( imgLastFrame, m_pLastImage.Ptr(), SGLImageObject::lockRead ) ;
	SGLImageSmartBuffer	isbNextFrame
			( imgNextFrame, m_pNextImage.Ptr(), SGLImageObject::lockRead ) ;
	//
	for ( size_t i = 0; i < m_arrFrameBuf.GetLength(); i ++ )
	{
		//
		// フレームを取得
		//
		SGLImageObject *	pImageBuf = m_arrFrameBuf.GetAt( i ) ;
		ESLAssert( pImageBuf != NULL ) ;
		if ( pImageBuf == NULL )
		{
			continue ;
		}
		uint32_t	flagsEnc =
			m_arrEncFlags.At(i) | SGLImageEncoder::flagDifferential ;
		do
		{
			SGLImageBuffer		imgCurFrame ;
			SGLImageSmartBuffer	isbCurFrame
				( imgCurFrame, pImageBuf, SGLImageObject::lockRead ) ;
			//
			// 差分画像を生成
			//
			int	nAbsMaxDiff = 0x7FFFFFFF ;
			if ( flagsEnc & SGLImageEncoder::flagNoMoveVector )
			{
				nAbsMaxDiff =
					SGLImageEncoder::MakeSubtractionBlock
									( imgCurFrame, imgLastFrame ) ;
			}
			else
			{
				m_pencImage2->ProcessMovingVector
					( imgCurFrame, imgLastFrame, nAbsMaxDiff, &imgNextFrame ) ;
			}
			//
			// 圧縮
			//
			if ( DescendChunk( "DiffeFrm" ) )
			{
				break ;
			}
			SGLEncodeBitStream	bstream( 0x10000 ) ;
			bstream.AttachOutputStream( this ) ;
			if ( m_pencImage2->EncodeImage
				( imgCurFrame, imgCurFrame.ptrBuffer, bstream, flagsEnc ) )
			{
				AscendChunk() ;
				break ;
			}
			AscendChunk() ;
		}
		while ( false ) ;
	}
	//
	m_arrFrameBuf.RemoveAll( ) ;
	m_arrEncFlags.RemoveAll( ) ;
	//
	return	errSuccess ;
}

// 音声データを圧縮して書き出す
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLMediaFileWriter::WriteWaveBuffer( void )
{
	if ( m_nWaveBufSamples == 0 )
	{
		return	errSuccess ;
	}
	if ( DescendChunk( "SoundStm" ) )
	{
		return	errFailed ;
	}
	SError	errResult = errFailed ;
	MIO_DATA_HEADER	miodh ;
	miodh.bytVersion = 1 ;
	miodh.bytFlags = 0 ;
	miodh.bytReserved1 = 0 ;
	miodh.bytReserved2 = 0 ;
	miodh.dwSampleCount = (DWORD) m_nWaveBufSamples ;
	if ( m_fKeyWaveBlock )
	{
		miodh.bytFlags = mioDataLeadBlock ;
		m_fKeyWaveBlock = false ;
	}
	if ( Write( &miodh, sizeof(MIO_DATA_HEADER) ) == sizeof(MIO_DATA_HEADER) )
	{
		SByteBuffer	buf ;
		buf.ReadFromFile( m_bufWaveBuffer ) ;
		//
		SGLEncodeBitStream	bstream( 0x10000 ) ;
		bstream.AttachOutputStream( this ) ;
		if ( !m_pencSound->EncodeSound( bstream, miodh, buf.GetConstArray() ) )
		{
			m_nOutputWaveSamples += m_nWaveBufSamples ;
			errResult = errSuccess ;
		}
		m_bufWaveBuffer.ClearAll() ;
	}
	m_nWaveBufSamples = 0 ;
	//
	AscendChunk( ) ;
	//
	return	errResult ;
}

// 画像バッファを生成
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLImageObject *
	SGLMediaFileWriter::CreateImageBuffer( const ERISA::ERI_INFO_HEADER & eih )
{
	uint32_t	width = (uint32_t) eih.nImageWidth ;
	uint32_t	height = (uint32_t) eih.nImageHeight ;
	if ( eih.nImageHeight < 0 )
	{
		height = (uint32_t) - eih.nImageHeight ;
	}
	uint32_t	format = (uint32_t) eih.fdwFormatType ;
	uint32_t	depth = (uint32_t) eih.dwBitsPerPixel ;
	//
	SGLImage *	pImage = new SGLImage ;
	pImage->CreateImage( width, height, format, depth ) ;
	return	pImage ;
}


