
/*****************************************************************************
                         E R I S A - L i b r a r y
 -----------------------------------------------------------------------------
    Copyright (C) 2013-2016 Leshade Entis,  Entis-soft. All rights reserved.
 *****************************************************************************/


#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl_erisa_lib.h>

using namespace SSystem ;
using namespace SakuraGL ;
using namespace SakuraCL ;
using namespace ERISA ;


//////////////////////////////////////////////////////////////////////////////
// 乱数
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_NV_CLASS_INFO( SakuraCL::SCLRandomizer )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SCLRandomizer::SCLRandomizer( void )
{
	m_numRandom = 1 ;
	m_countSalt = 0 ;
}

SCLRandomizer::SCLRandomizer( uint32_t numInit )
{
	InitializeSeedBy( numInit ) ;
}

// 乱数の種初期化
//////////////////////////////////////////////////////////////////////////////
void SCLRandomizer::InitializeSeed( void )
{
	DATE_TIME	dtCurrent ;
	uint64_t	msecCurrent = CurrentMilliSec() ;
	CurrentLocalDate( dtCurrent ) ;
	//
	m_countSalt = 0 ;
	m_numRandom = (uint32_t) msecCurrent ^ (uint32_t) (msecCurrent >> 32) ;
	m_numRandom += dtCurrent.nYear + dtCurrent.nMonth + dtCurrent.nDay
					+ ((dtCurrent.nHour + dtCurrent.nMinute) << 8)
					+ ((dtCurrent.nSecond + dtCurrent.nMilliSec) << 16) ;
	//
#if	defined(__PLATFORM_WINDOWS__)
	LARGE_INTEGER	liCounter ;
	if ( ::QueryPerformanceCounter( &liCounter ) )
	{
		m_numRandom ^= liCounter.LowPart ^ liCounter.HighPart ;
	}
#endif
	//
	QuickRandomize() ;
	QuickRandomize() ;
	QuickRandomize() ;
	//
	m_numRandom ^= (uint32_t) msecCurrent ;
}

void SCLRandomizer::InitializeSeedBy( uint32_t numInit )
{
	m_countSalt = 0 ;
	m_numRandom = numInit ;
	//
	QuickRandomize() ;
	QuickRandomize() ;
	QuickRandomize() ;
	//
	m_numRandom ^= numInit ;
}

// 乱数の種取得
//////////////////////////////////////////////////////////////////////////////
uint32_t SCLRandomizer::SaveRandomSeed( void ) const
{
	return	m_numRandom ;
}

// 乱数の種復元
//////////////////////////////////////////////////////////////////////////////
void SCLRandomizer::RestoreRandomSeed( uint32_t numSeed )
{
	m_numRandom = numSeed ;
}

// 乱数生成 [0,x) or [0,0xFFFFFFFF]
//////////////////////////////////////////////////////////////////////////////
uint32_t SCLRandomizer::Randomize( uint32_t numLimit )
{
#if	defined(__PLATFORM_WINDOWS__)
	LARGE_INTEGER	liCounter ;
	if ( ::QueryPerformanceCounter( &liCounter ) )
	{
		m_numRandom ^= liCounter.LowPart ;
	}
	else
	{
#endif
	uint64_t	msecCurrent = CurrentMilliSec() ;
	if ( ++ m_countSalt >= (msecCurrent & 0xFF) )
	{
		m_numRandom += (uint32_t) msecCurrent ;
		m_countSalt = 0 ;
	}
#if	defined(__PLATFORM_WINDOWS__)
	}
#endif
	return	QuickRandomize( numLimit ) ;
}

uint32_t SCLRandomizer::QuickRandomize( uint32_t numLimit )
{
	m_numRandom = m_numRandom * 0xFB + 0x0093C98B ;
	if ( numLimit == 0 )
	{
		return	m_numRandom ;
	}
	if ( numLimit >= 0x1000000 )
	{
		return	m_numRandom % numLimit ;
	}
	return	(m_numRandom >> 8) % numLimit ;
}

// 乱数生成 [0.0,x)
//////////////////////////////////////////////////////////////////////////////
float32_t SCLRandomizer::QuickRandomFloat( float32_t fpRange )
{
	uint32_t	r = QuickRandomize() >> 8 ;
	return	(float32_t) (r & 0x00FFFFFF) * fpRange * (1.0f / 0x01000000) ;
}

// 乱数生成 [-x,x)
//////////////////////////////////////////////////////////////////////////////
double SCLRandomizer::QuickRandomDouble( double fpRange )
{
	uint32_t	r = QuickRandomize() ;
	return	((int32_t) r) * fpRange * (1.0 / 0x80000000UL) ;
}


//////////////////////////////////////////////////////////////////////////////
// 簡易暗号化（難読化）コンテキスト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_NV_CLASS_INFO( ERISA::SGLSimpleCrypt32Context )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLSimpleCrypt32Context::SGLSimpleCrypt32Context( void )
{
	m_nBuffered = 0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLSimpleCrypt32Context::~SGLSimpleCrypt32Context( void )
{
}

// 初期化
//////////////////////////////////////////////////////////////////////////////
void SGLSimpleCrypt32Context::Initialize( const wchar_t * pszPassword )
{
	//
	// パスワードを UTF-8 へ変換
	//
	SArray<uint8_t>	bufUTF8 ;
	Charset::Encode( bufUTF8, Charset::encodingUTF8, pszPassword ) ;
	//
	// パスワードの CRC32 を取得
	//
	const size_t	nLenUTF8 = bufUTF8.GetLength() ;
	const uint8_t *	pBufUTF8 = bufUTF8.GetConstArray() ;
	CRC32Context	crc32 ;
	crc32.Stream( pBufUTF8, nLenUTF8 ) ;
	m_crcPassword = crc32.GetCRC32() ;
	//
	// 難読化マスクを生成（パスワードのビット反転を設定）
	//
	uint8_t *	pBufSalt = (uint8_t*) &(m_bufSalt[0]) ;
	eslFillMemory( pBufSalt, 0, bufferSizeInBytes ) ;
	//
	size_t	iSrc = 0, iDst = 0 ;
	while ( iSrc < nLenUTF8 )
	{
		pBufSalt[iDst] ^= ~pBufUTF8[iSrc ++] ;
		iDst = (iDst + 1) & bufferSizeModMask ;
	}
	if ( nLenUTF8 > 0 )
	{
		uint8_t	nSaltMul = 7 ;
		iSrc = 0 ;
		while ( iDst < bufferSizeInBytes )
		{
			pBufSalt[iDst ++] ^= ~pBufUTF8[iSrc ++] * nSaltMul ;
			if ( iSrc >= nLenUTF8 )
			{
				iSrc = 0 ;
				nSaltMul = nSaltMul * 7 + 5 ;
			}
		}
	}
	//
	// バッファの蓄積カウンタ初期化
	//
	m_nBuffered = 0 ;
}

// 暗号化鍵生成・復号鍵取得
//////////////////////////////////////////////////////////////////////////////
uint32_t SGLSimpleCrypt32Context::GenerateKey( void )
{
	//
	// 適当に擬似乱数生成
	//
	SCLRandomizer	rand ;
	uint32_t		nRandom ;
	rand.InitializeSeed() ;
	nRandom = rand.QuickRandomize() ;
	//
	// 逆元の存在する数を検索
	// ※特に素数に限定しない
	//
	uint64_t	N = ((uint64_t) 1) << 32 ;
	nRandom |= 0x01 ;
	for ( ; ; )
	{
		uint64_t	nInv = ComputeInverseElement<uint64_t>( nRandom, N ) ;
		if ( nInv != 0 )
		{
			// 念のため検算
			m_keyCrypt = (uint32_t) nInv ;
			ESLAssert( ((uint32_t) (m_keyCrypt * nRandom) & 0xFFFFFFFFUL) == 1 ) ;
			if ( ((uint32_t) (m_keyCrypt * nRandom) & 0xFFFFFFFFUL) == 1 )
			{
				return	nRandom ^ m_crcPassword ;
			}
		}
		nRandom += 2 ;
	}
	return	nRandom ^ m_crcPassword ;
}

// 復号鍵設定
//////////////////////////////////////////////////////////////////////////////
void SGLSimpleCrypt32Context::SetDecryptKey( uint32_t keyDecrypt )
{
	m_keyCrypt = keyDecrypt ^ m_crcPassword ;
}

// パスワード・鍵情報複製
//////////////////////////////////////////////////////////////////////////////
void SGLSimpleCrypt32Context::DuplicateKey( const SGLSimpleCrypt32Context& ctx )
{
	for ( size_t i = 0; i < bufferSizeInDWords; i ++ )
	{
		m_bufSalt[i] = ctx.m_bufSalt[i] ;
	}
	m_crcPassword = ctx.m_crcPassword ;
	m_keyCrypt = ctx.m_keyCrypt ;
}

// 暗号化／復号バッファにデータを追加
//////////////////////////////////////////////////////////////////////////////
size_t SGLSimpleCrypt32Context::WriteData( const void * ptrSrc, size_t nBytes )
{
	if ( m_nBuffered >= bufferSizeInBytes )
	{
		return	0 ;
	}
	const size_t	nLeftBytes = bufferSizeInBytes - m_nBuffered ;
	if ( nBytes > nLeftBytes )
	{
		nBytes = nLeftBytes ;
	}
	uint8_t *	pBufCrypt = (uint8_t*) &(m_bufCrypt[0]) ;
	eslMoveMemory( pBufCrypt + m_nBuffered, ptrSrc, nBytes ) ;
	m_nBuffered += nBytes ;
	return	nBytes ;
}

// データの終端処理
//////////////////////////////////////////////////////////////////////////////
void SGLSimpleCrypt32Context::FlushData( void )
{
	if ( m_nBuffered < bufferSizeInBytes )
	{
		const size_t	nLeftBytes = bufferSizeInBytes - m_nBuffered ;
		uint8_t *		pBufCrypt = (uint8_t*) &(m_bufCrypt[0]) ;
		eslFillMemory( pBufCrypt + m_nBuffered, 0, nLeftBytes ) ;
	}
}

// 暗号化処理実行
//////////////////////////////////////////////////////////////////////////////
size_t SGLSimpleCrypt32Context::EncryptBuffer( void )
{
	size_t	nCount = (m_nBuffered + 0x03) >> 2 ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		m_bufCrypt[i] = (m_bufCrypt[i] ^ m_bufSalt[i]) * m_keyCrypt ;
	}
	m_nBuffered = 0 ;
	return	nCount << 2 ;
}

// 復号化処理実行
//////////////////////////////////////////////////////////////////////////////
size_t SGLSimpleCrypt32Context::DecryptBuffer( void )
{
	size_t	nCount = (m_nBuffered + 0x03) >> 2 ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		m_bufCrypt[i] = (m_bufCrypt[i] * m_keyCrypt) ^ m_bufSalt[i] ;
	}
	m_nBuffered = 0 ;
	return	nCount << 2 ;
}

// バッファ取得
//////////////////////////////////////////////////////////////////////////////
size_t SGLSimpleCrypt32Context::ReadData( void * ptrDst, size_t nBytes )
{
	if ( nBytes == 0 )
	{
		return	0 ;
	}
	if ( nBytes > bufferSizeInBytes )
	{
		nBytes = bufferSizeInBytes ;
	}
	eslMoveMemory( ptrDst, &m_bufCrypt[0], nBytes ) ;
	return	nBytes ;
}


//////////////////////////////////////////////////////////////////////////////
// 簡易暗号化入力ストリーム
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
	( ERISA::SGLDecrypt32InputStream, SInputStream, SGLSimpleCrypt32Context )

// ファイルから読み込み
//////////////////////////////////////////////////////////////////////////////
size_t SGLDecrypt32InputStream::Read( void * ptrBuf, size_t nBytes )
{
	ESLAssert( m_pStream != NULL ) ;
	uint8_t *	pbytDst = (uint8_t*) ptrBuf ;
	size_t		nTotalBytes = 0 ;
	while ( nBytes != 0 )
	{
		if ( m_iNextOutput < m_nDecrypted )
		{
			//
			// 復号済みバッファからコピー
			//
			const size_t	nLeftDecrypt = m_nDecrypted - m_iNextOutput ;
			size_t	nCopyBytes = nLeftDecrypt ;
			if ( nCopyBytes > nBytes )
			{
				nCopyBytes = nBytes ;
			}
			eslMoveMemory
				( pbytDst, &m_bufDecrypt[m_iNextOutput], nCopyBytes ) ;
			//
			pbytDst += nCopyBytes ;
			nBytes -= nCopyBytes ;
			m_iNextOutput += nCopyBytes ;
			nTotalBytes += nCopyBytes ;
		}
		else if ( (m_pStream != NULL) && (nBytes != 0) && !m_flagEOF )
		{
			//
			// 一括してストリームから読み込み中間バッファへ溜めておく
			//
			uint8_t	bufTemp[bufferSizeInBytes * 0x10] ;
			size_t	nReadBytes =
				m_pStream->Read( &bufTemp[0], bufferSizeInBytes * 0x10 ) ;
			m_flagEOF = (nReadBytes == 0) ;
			//
			m_nDecrypted = 0 ;
			m_iNextOutput = 0 ;
			//
			size_t	iTemp = 0 ;
			while ( iTemp < nReadBytes )
			{
				iTemp += WriteData( &bufTemp[iTemp], nReadBytes - iTemp ) ;
				if ( IsBufferFull() )
				{
					size_t	nDecryptBytes = DecryptBuffer() ;
					eslMoveMemory
						( &m_bufDecrypt[m_nDecrypted],
							GetCryptBuffer(), nDecryptBytes ) ;
					m_nDecrypted += nDecryptBytes ;
				}
			}
			if ( m_flagEOF )
			{
				FlushData() ;
				//
				size_t	nDecryptBytes = DecryptBuffer() ;
				eslMoveMemory
					( &m_bufDecrypt[m_nDecrypted],
						GetCryptBuffer(), nDecryptBytes ) ;
				m_nDecrypted += nDecryptBytes ;
			}
			ESLAssert( m_nDecrypted <= bufferSizeInBytes * 0x11 ) ;
		}
		else
		{
			break ;
		}
	}
	return	nTotalBytes ;
}


//////////////////////////////////////////////////////////////////////////////
// 簡易暗号化出力ストリーム
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
	( ERISA::SGLEncrypt32OutputStream, SOutputStream, SGLSimpleCrypt32Context )

// データの終端処理
//////////////////////////////////////////////////////////////////////////////
void SGLEncrypt32OutputStream::FlushData( void )
{
	SGLSimpleCrypt32Context::FlushData() ;
	//
	ESLAssert( m_pStream != NULL ) ;
	if ( m_pStream != NULL )
	{
		size_t	nEncryptBytes = EncryptBuffer() ;
		if ( nEncryptBytes != 0 )
		{
			m_pStream->Write( GetCryptBuffer(), nEncryptBytes ) ;
		}
	}
}

// ファイルへ書き込み
//////////////////////////////////////////////////////////////////////////////
size_t SGLEncrypt32OutputStream::Write( const void * ptrBuf, size_t nBytes )
{
	ESLAssert( m_pStream != NULL ) ;
	size_t	nTotalBytes = 0 ;
	if ( (m_pStream != NULL) && (nBytes != 0) )
	{
		const uint8_t *	pbytSrc = (const uint8_t*) ptrBuf ;
		while ( nBytes != 0 )
		{
			size_t	nNextBytes = WriteData( pbytSrc, nBytes ) ;
			pbytSrc += nNextBytes ;
			nBytes -= nNextBytes ;
			nTotalBytes += nNextBytes ;
			//
			if ( IsBufferFull() )
			{
				size_t	nEncryptBytes = EncryptBuffer() ;
				if ( nEncryptBytes != 0 )
				{
					m_pStream->Write( GetCryptBuffer(), nEncryptBytes ) ;
				}
			}
		}
	}
	return	nTotalBytes ;
}


//////////////////////////////////////////////////////////////////////////////
// 簡易暗号化ファイル読み込みフィルタ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ERISA::SGLDecrypt32File, SFileInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLDecrypt32File::SGLDecrypt32File( void )
{
	m_pFile = NULL ;
	m_flagOwnFile = false ;
	m_fpPos = 0 ;
	m_fpDataCache = 0 ;
	m_nDataCacheBytes = 0 ;
	m_nDecryptBytes = 0 ;
	m_fpDecryptCache = 0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLDecrypt32File::~SGLDecrypt32File( void )
{
	Close() ;
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLDecrypt32File::Open
	( SFileInterface * pFile,
		bool flagOwnFile, const wchar_t * pwszPassword )
{
	Close() ;
	//
	// ファイルヘッダ読み込み
	//
	m_pFile = pFile ;
	m_flagOwnFile = flagOwnFile ;
	//
	if ( m_pFile->Read
		( &m_fhHeader, sizeof(FILE_HEADER) ) < sizeof(FILE_HEADER) )
	{
		Close() ;
		return	errFailed ;
	}
	m_fpBasePos = m_pFile->GetPosition() ;
	//
	// 復号準備
	//
	m_context.Initialize( pwszPassword ) ;
	m_context.SetDecryptKey( m_fhHeader.keyDecrypt ) ;
	//
	return	errSuccess ;
}

// ファイルを閉じる
//////////////////////////////////////////////////////////////////////////////
void SGLDecrypt32File::Close( void )
{
	if ( m_flagOwnFile )
	{
		delete	m_pFile ;
	}
	m_pFile = NULL ;
	m_flagOwnFile = false ;
	m_fpPos = 0 ;
	m_bufDataCache.RemoveAll() ;
	m_fpDataCache = 0 ;
	m_nDataCacheBytes = 0 ;
	m_nDecryptBytes = 0 ;
	m_fpDecryptCache = 0 ;
}

// ファイルインターフェースの複製
//////////////////////////////////////////////////////////////////////////////
SFileInterface * SGLDecrypt32File::Duplicate( void ) const
{
	SGLDecrypt32File *	pdf = new SGLDecrypt32File ;
	if ( m_pFile != NULL )
	{
		pdf->m_pFile = m_pFile->Duplicate() ;
		pdf->m_flagOwnFile = true ;
		pdf->m_fhHeader = m_fhHeader ;
		pdf->m_fpBasePos = m_fpBasePos ;
		pdf->m_context.DuplicateKey( m_context ) ;
	}
	return	pdf ;
}

// ファイルから読み込み
//////////////////////////////////////////////////////////////////////////////
size_t SGLDecrypt32File::Read( void * ptrBuf, size_t nBytes )
{
	size_t	nReadBytes = 0 ;
	while ( nBytes != 0 )
	{
		if ( (m_fpPos < m_fpDecryptCache)
			|| (m_fpPos - m_fpDecryptCache >= (int64_t) m_nDecryptBytes) )
		{
			if ( (m_fpPos < m_fpDataCache)
				|| (m_fpPos - m_fpDataCache >= (int64_t) m_nDataCacheBytes) )
			{
				//
				// 暗号データ読み込み
				//
				if ( m_pFile == NULL )
				{
					break ;
				}
				m_fpDataCache = m_fpPos & ~0xFF ;
				m_pFile->Seek( m_fpDataCache + m_fpBasePos ) ;
				m_nDataCacheBytes =
					m_pFile->Read
						( m_bufDataCache.GetArray( 0x1000 ), 0x1000 ) ;
				m_bufDataCache.FinishArray() ;
			}
			//
			// 暗号データ復号
			//
			int64_t	nOffset = m_fpPos - m_fpDataCache ;
			if ( (nOffset < 0)
				|| (nOffset > (int64_t) m_nDataCacheBytes) )
			{
				break ;
			}
			size_t	nBlockPos =
				(size_t) nOffset
					& ~SGLSimpleCrypt32Context::bufferSizeModMask ;
			m_fpDecryptCache = m_fpDataCache + nBlockPos ;
			m_nDecryptBytes = SGLSimpleCrypt32Context::bufferSizeInBytes ;
			//
			if ( m_fpDecryptCache + (int64_t) m_nDecryptBytes
									> (int64_t) m_fhHeader.nBytes )
			{
				if ( m_fpDecryptCache >= (int64_t) m_fhHeader.nBytes )
				{
					break ;
				}
				m_nDecryptBytes =
					(size_t) (m_fhHeader.nBytes - m_fpDecryptCache) ;
			}
			//
			m_context.WriteData
				( m_bufDataCache.GetConstArray() + nBlockPos,
					SGLSimpleCrypt32Context::bufferSizeInBytes ) ;
			m_context.DecryptBuffer() ;
			m_context.ReadData
				( &m_bufDecrypt[0],
					SGLSimpleCrypt32Context::bufferSizeInBytes ) ;
		}
		//
		// 復号済みデータを複製
		//
		size_t	nOffset = (size_t) (m_fpPos - m_fpDecryptCache) ;
		if ( nOffset >= m_nDecryptBytes )
		{
			break ;
		}
		size_t	nBlockBytes = m_nDecryptBytes - nOffset ;
		if ( nBlockBytes > nBytes )
		{
			nBlockBytes = nBytes ;
		}
		if ( (uint64_t) m_fpPos + nBlockBytes >= m_fhHeader.nBytes )
		{
			if ( (uint64_t) m_fpPos >= m_fhHeader.nBytes )
			{
				break ;
			}
			nBlockBytes = (size_t) (m_fhHeader.nBytes - m_fpPos) ;
			eslMoveMemory( ptrBuf, &m_bufDecrypt[nOffset], nBlockBytes ) ;
			nReadBytes += nBlockBytes ;
			m_fpPos += nBlockBytes ;
			break ;
		}
		else
		{
			eslMoveMemory( ptrBuf, &m_bufDecrypt[nOffset], nBlockBytes ) ;
			ptrBuf = ((uint8_t*)ptrBuf) + nBlockBytes ;
			nBytes -= nBlockBytes ;
			nReadBytes += nBlockBytes ;
			m_fpPos += nBlockBytes ;
		}
	}
	return	nReadBytes ;
}

// ファイルへ書き込み
//////////////////////////////////////////////////////////////////////////////
size_t SGLDecrypt32File::Write( const void * ptrBuf, size_t nBytes )
{
	return	0 ;
}

// シーク可能か否か？
//////////////////////////////////////////////////////////////////////////////
bool SGLDecrypt32File::IsSeekable( void ) const
{
	return	true ;
}

// ファイル長の取得
//////////////////////////////////////////////////////////////////////////////
int64_t SGLDecrypt32File::GetLength( void ) const
{
	if ( m_pFile == NULL )
	{
		return	0 ;
	}
	return	(int64_t) m_fhHeader.nBytes ;
}

// ファイルポインタを移動
//////////////////////////////////////////////////////////////////////////////
int64_t SGLDecrypt32File::Seek
	( int64_t posFile, SFileInterface::SeekOrigin seekFrom )
{
	switch ( seekFrom )
	{
	case	SFileInterface::FromBegin:
		m_fpPos = posFile ;
		break ;
	case	SFileInterface::FromCurrent:
		m_fpPos += posFile ;
		break ;
	case	SFileInterface::FromEnd:
		m_fpPos = (int64_t) m_fhHeader.nBytes + posFile ;
		break ;
	}
	if ( m_fpPos < 0 )
	{
		m_fpPos = 0 ;
	}
	else if ( m_fpPos > (int64_t) m_fhHeader.nBytes )
	{
		m_fpPos = (int64_t) m_fhHeader.nBytes ;
	}
	return	m_fpPos ;
}

// ファイルポインタを取得
//////////////////////////////////////////////////////////////////////////////
int64_t SGLDecrypt32File::GetPosition( void ) const
{
	return	m_fpPos ;
}

// ファイルの終端を現在の位置に設定する
//////////////////////////////////////////////////////////////////////////////
SError SGLDecrypt32File::SetEndOfFile( void )
{
	return	errFailed ;
}


//////////////////////////////////////////////////////////////////////////////
// 簡易暗号化ファイル書き出し
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ERISA::SGLEncrypt32FileWriter, SFileInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLEncrypt32FileWriter::SGLEncrypt32FileWriter( void )
{
	m_pFile = NULL ;
	m_flagOwnFile = false ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLEncrypt32FileWriter::~SGLEncrypt32FileWriter( void )
{
	Close() ;
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLEncrypt32FileWriter::Open
	( SFileInterface * pFile, bool flagOwnFile, const wchar_t * pwszPassword )
{
	m_pFile = pFile ;
	m_flagOwnFile = flagOwnFile ;
	m_fpHeaderPos = m_pFile->GetPosition() ;
	//
	m_context.Initialize( pwszPassword ) ;
	//
	m_fhHeader.chSign[0] = 's' ;
	m_fhHeader.chSign[1] = 'c' ;
	m_fhHeader.chSign[2] = 'p' ;
	m_fhHeader.chSign[3] = 'd' ;
	m_fhHeader.keyDecrypt = m_context.GenerateKey() ;
	m_fhHeader.nBytes = 0 ;
	m_fhHeader.nReserved = 0 ;
	//
	if ( m_pFile->Write
		( &m_fhHeader, sizeof(SGLDecrypt32File::FILE_HEADER) )
							< sizeof(SGLDecrypt32File::FILE_HEADER) )
	{
		m_pFile = NULL ;
		return	errFailed ;
	}
	return	errSuccess ;
}

// ファイルを閉じる
//////////////////////////////////////////////////////////////////////////////
void SGLEncrypt32FileWriter::Close( void )
{
	if ( m_pFile != NULL )
	{
		m_context.FlushData() ;
		size_t	nEncryptBytes = m_context.EncryptBuffer() ;
		if ( nEncryptBytes != 0 )
		{
			m_qbufTemp.Write( m_context.GetCryptBuffer(), nEncryptBytes ) ;
		}
		size_t			nBufBytes = (size_t) m_qbufTemp.GetLength() ;
		const uint8_t *	pbytBuf = m_qbufTemp.GetBuffer( nBufBytes ) ;
		if ( nBufBytes > 0 )
		{
			m_pFile->Write( pbytBuf, nBufBytes ) ;
		}
		int64_t	fpEnd = m_pFile->GetPosition() ;
		m_pFile->Seek( m_fpHeaderPos ) ;
		m_pFile->Write
			( &m_fhHeader, sizeof(SGLDecrypt32File::FILE_HEADER) ) ;
		m_pFile->Seek( fpEnd ) ;
		//
		if ( m_flagOwnFile )
		{
			delete	m_pFile ;
		}
	}
	m_pFile = NULL ;
	m_flagOwnFile = false ;
	m_qbufTemp.ClearAll() ;
}

// ファイルインターフェースの複製
//////////////////////////////////////////////////////////////////////////////
SFileInterface * SGLEncrypt32FileWriter::Duplicate( void ) const
{
	return	new SGLEncrypt32FileWriter ;
}

// ファイルから読み込み
//////////////////////////////////////////////////////////////////////////////
size_t SGLEncrypt32FileWriter::Read( void * ptrBuf, size_t nBytes )
{
	return	0 ;
}

// ファイルへ書き込み
//////////////////////////////////////////////////////////////////////////////
size_t SGLEncrypt32FileWriter::Write( const void * ptrBuf, size_t nBytes )
{
	ESLAssert( m_pFile != NULL ) ;
	size_t	nTotalBytes = 0 ;
	if ( (m_pFile != NULL) && (nBytes != 0) )
	{
		const uint8_t *	pbytSrc = (const uint8_t*) ptrBuf ;
		while ( nBytes != 0 )
		{
			size_t	nNextBytes = m_context.WriteData( pbytSrc, nBytes ) ;
			pbytSrc += nNextBytes ;
			nBytes -= nNextBytes ;
			//
			if ( m_context.IsBufferFull() )
			{
				size_t	nEncryptBytes = m_context.EncryptBuffer() ;
				if ( nEncryptBytes != 0 )
				{
					m_qbufTemp.Write
						( m_context.GetCryptBuffer(), nEncryptBytes ) ;
					if ( m_qbufTemp.GetLength() >= 0x10000 )
					{
						size_t	nBufBytes = 0x10000 ;
						const uint8_t *
								pbytBuf = m_qbufTemp.GetBuffer( nBufBytes ) ;
						if ( nBufBytes > 0 )
						{
							if ( m_pFile->Write
									( pbytBuf, nBufBytes ) < nBufBytes )
							{
								break ;
							}
						}
						m_qbufTemp.ReleaseBuffer( (ssize_t) nBufBytes ) ;
					}
				}
			}
			nTotalBytes += nNextBytes ;
			m_fhHeader.nBytes += nNextBytes ;
		}
	}
	return	nTotalBytes ;
}

// シーク可能か否か？
//////////////////////////////////////////////////////////////////////////////
bool SGLEncrypt32FileWriter::IsSeekable( void ) const
{
	return	false ;
}

// ファイル長の取得
//////////////////////////////////////////////////////////////////////////////
int64_t SGLEncrypt32FileWriter::GetLength( void ) const
{
	return	m_fhHeader.nBytes ;
}

// ファイルポインタを移動
//////////////////////////////////////////////////////////////////////////////
int64_t SGLEncrypt32FileWriter::Seek
	( int64_t posFile, SFileInterface::SeekOrigin seekFrom )
{
	return	(int64_t) m_fhHeader.nBytes ;
}

// ファイルポインタを取得
//////////////////////////////////////////////////////////////////////////////
int64_t SGLEncrypt32FileWriter::GetPosition( void ) const
{
	return	(int64_t) m_fhHeader.nBytes ;
}

// ファイルの終端を現在の位置に設定する
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLEncrypt32FileWriter::SetEndOfFile( void )
{
	return	errFailed ;
}


//////////////////////////////////////////////////////////////////////////////
// 簡易暗号化ファイル・オープナー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ERISA::SGLDecrypt32FileOpener, SOffsetFileOpener )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLDecrypt32FileOpener::SGLDecrypt32FileOpener
	( const wchar_t * pszBasePath,
		wchar_t wchSeparator,
		SFileOpener * pOpener, bool flagOwner, const wchar_t * pszPassword )
	: SOffsetFileOpener( pszBasePath, wchSeparator, pOpener, flagOwner ),
		m_strPassword( pszPassword )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLDecrypt32FileOpener::~SGLDecrypt32FileOpener( void )
{
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SFileInterface * SGLDecrypt32FileOpener::NewOpenFile
	( const wchar_t * pszFilePath, long int nOpenFlags )
{
	SFileInterface *	pFile =
		SOffsetFileOpener::NewOpenFile( pszFilePath, nOpenFlags ) ;
	if ( pFile != NULL )
	{
		SGLDecrypt32File *	pDecrypt = new SGLDecrypt32File ;
		if ( pDecrypt->Open( pFile, true, m_strPassword ) )
		{
			delete	pDecrypt ;
			return	NULL ;
		}
		return	pDecrypt ;
	}
	return	NULL ;
}



#if	!defined(__COTOPHA__)

//////////////////////////////////////////////////////////////////////////
// 簡易 RSA 暗号化コンテキスト (96bit)
//////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_NV_CLASS_INFO( ERISA::RSA96CryptContext )

// 構築関数
//////////////////////////////////////////////////////////////////////////
RSA96CryptContext::RSA96CryptContext( void )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////
RSA96CryptContext::~RSA96CryptContext( void )
{
}

// 初期化
//////////////////////////////////////////////////////////////////////////
void RSA96CryptContext::Initialize( const wchar_t * pszPassword )
{
	//
	// パスワードから各種パラメータ生成
	//
	uint32_t	saltForData[3] = { 0, 0, 0 } ;
	uint32_t	saltForKey[3] = { 0, 0, 0 } ;
	uint32_t	keyEncrypt = 0 ;
	//
	if ( (pszPassword != NULL) && (pszPassword[0] != 0) )
	{
		for ( size_t i = 0; pszPassword[i]; i ++ )
		{
			keyEncrypt ^= pszPassword[i] ;
		}
		//
		SArray<uint8_t>	bufPassword ;
		Charset::Encode
			( bufPassword, Charset::encodingUTF8, pszPassword ) ;
		//
		CRC32Context	crc32 ;
		crc32.Stream( bufPassword.GetConstArray(), bufPassword.GetLength() ) ;
		//
		saltForKey[0] = crc32.GetCRC32() ;
		saltForKey[1] = ~saltForKey[0] * 5 ;
		saltForKey[2] = ~saltForKey[1] * 5 ;
		//
		saltForData[0] = ~saltForKey[0] * 7 ;
		saltForData[1] = ~saltForData[0] * 7 ;
		saltForData[2] = ~saltForData[1] * 7 ;
	}
	keyEncrypt |= 0x10001 ;
	//
	while ( !SakuraCL::IsPrimality<int64_t>( keyEncrypt ) )
	{
		keyEncrypt += 2 ;
	}
	//
	m_bnSaltForData = BigNumber<6>( &saltForData[0], 3 ) ;
	m_bnSaltForKey = BigNumber<6>( &saltForKey[0], 3 ) ;
	m_keyEncrypt = BigNumber<6>( &keyEncrypt, 1 ) ;
}

// 暗号化鍵生成・復号鍵取得
//////////////////////////////////////////////////////////////////////////
BigNumber<3> RSA96CryptContext::GenerateKey( void )
{
	SakuraCL::SCLRandomizer	rand ;
	rand.InitializeSeed() ;
	//
	for ( ; ; )
	{
		//
		// 素数を２つ生成する
		//
		BigNumber<6>	p = GeneratePrime( rand ) ;
		BigNumber<6>	q = GeneratePrime( rand ) ;
		while ( p == q )
		{
			q = GeneratePrime( rand ) ;
		}
		//
		// 暗号（公開）鍵／復号（秘密）鍵生成
		//
		m_keyMod = p * q ;
		p -= 1 ;
		q -= 1 ;
		//
		SignedBigNumber<6>	r = p * q ;
		m_keyDecrypt =
			SakuraCL::ComputeInverseElement< SignedBigNumber<6> >
							( SignedBigNumber<6>( m_keyEncrypt ), r ) ;
		if ( ((m_keyEncrypt * m_keyDecrypt) % r) == 1 )
		{
			// 暗号化に適した素数が生成できた
			break ;
		}
	}
	//
	BigNumber<6>	keyPublic = m_keyMod ;
	keyPublic ^= m_bnSaltForKey ;
	return	BigNumber<3>( &(keyPublic.m_num[0]), 3 ) ;
}

// 復号鍵設定
//////////////////////////////////////////////////////////////////////////
void RSA96CryptContext::SetPublicKey( const BigNumber<3>& keyPublic )
{
	m_keyMod = BigNumber<6>( &(keyPublic.m_num[0]), 3 ) ;
	m_keyMod ^= m_bnSaltForKey ;
}

// 暗号鍵用素数を生成
//////////////////////////////////////////////////////////////////////////
BigNumber<6> RSA96CryptContext::GeneratePrime( SCLRandomizer& rand )
{
	uint32_t	n[2] ;
	n[0] = rand.Randomize() ;
	n[1] = rand.Randomize() ;
	n[0] ^= n[1] >> 16 ;
	n[1] >>= 16 ;
	n[0] |= 0x00000001 ;
	n[1] |= 0x00008000 ;
	//
	BigNumber<6>	bn( &n[0], 2 ) ;
	while ( !SakuraCL::IsPrimality< BigNumber<6> >( bn ) )
	{
		bn += 2 ;
	}
	return	bn ;
}

// 暗号化処理実行
//////////////////////////////////////////////////////////////////////////
void RSA96CryptContext::Encrypt
	( BigNumber<3>& bnCrypt, const uint8_t * pbytSrc )
{
	uint32_t	bufSrc[3] ;
	bufSrc[0] = pbytSrc[0]
			| ((uint32_t) pbytSrc[1] << 8)
			| ((uint32_t) pbytSrc[2] << 16)
			| ((uint32_t) pbytSrc[3] << 24) ;
	bufSrc[1] = pbytSrc[4]
			| ((uint32_t) pbytSrc[5] << 8)
			| ((uint32_t) pbytSrc[6] << 16)
			| ((uint32_t) pbytSrc[7] << 24) ;
	bufSrc[2] = pbytSrc[8]
			| ((uint32_t) pbytSrc[9] << 8)
			| ((uint32_t) pbytSrc[10] << 16) ;
	//
	BigNumber<6>	bnSrc( &bufSrc[0], 3 ) ;
	BigNumber<6>	bnDst =
						SakuraCL::ComputePower< BigNumber<6> >
								( bnSrc, m_keyEncrypt, m_keyMod ) ;
	//
	bnDst ^= m_bnSaltForData ;
	bnCrypt = BigNumber<3>( &bnDst.m_num[0], 3 ) ;
}

// 復号化処理実行
//////////////////////////////////////////////////////////////////////////
void RSA96CryptContext::Decrypt
	( uint8_t * pbytDst, const BigNumber<3>& bnCrypt )
{
	BigNumber<6>	bnSrc( &bnCrypt.m_num[0], 3 ) ;
	bnSrc ^= m_bnSaltForData ;
	//
	BigNumber<6>	bnDst =
						SakuraCL::ComputePower< BigNumber<6> >
								( bnSrc, m_keyDecrypt, m_keyMod ) ;
	//
	pbytDst[0] = (uint8_t) bnDst.m_num[0] ;
	pbytDst[1] = (uint8_t) (bnDst.m_num[0] >> 8) ;
	pbytDst[2] = (uint8_t) (bnDst.m_num[0] >> 16) ;
	pbytDst[3] = (uint8_t) (bnDst.m_num[0] >> 24) ;
	pbytDst[4] = (uint8_t) bnDst.m_num[1] ;
	pbytDst[5] = (uint8_t) (bnDst.m_num[1] >> 8) ;
	pbytDst[6] = (uint8_t) (bnDst.m_num[1] >> 16) ;
	pbytDst[7] = (uint8_t) (bnDst.m_num[1] >> 24) ;
	pbytDst[8] = (uint8_t) bnDst.m_num[2] ;
	pbytDst[9] = (uint8_t) (bnDst.m_num[2] >> 8) ;
	pbytDst[10] = (uint8_t) (bnDst.m_num[2] >> 16) ;
}


//////////////////////////////////////////////////////////////////////////////
// 簡易 RSA 暗号化出力ストリーム
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
	( ERISA::SGLEncryptRSA96OutputStream, SOutputStream, RSA96CryptContext )

// データの終端処理
//////////////////////////////////////////////////////////////////////////////
void SGLEncryptRSA96OutputStream::FlushData( void )
{
	size_t	nOdd = m_nBuffered % plainInBytes ;
	size_t	nBlocks = (m_nBuffered - nOdd) / plainInBytes ;
	if ( nOdd != 0 )
	{
		for ( size_t i = nOdd; i < plainInBytes; i ++ )
		{
			m_bufSrc[m_nBuffered - nOdd + i] = 0xFF ;
		}
	}
	BigNumber<3>	bnCrypt ;
	for ( size_t i = 0; i < nBlocks; i ++ )
	{
		Encrypt( bnCrypt, &(m_bufSrc[i * plainInBytes]) ) ;
		uint32_t *	pDst = (uint32_t*) &(m_bufDst[i * cryptInBytes]) ;
		pDst[0] = bnCrypt.m_num[0] ;
		pDst[1] = bnCrypt.m_num[1] ;
		pDst[2] = bnCrypt.m_num[2] ;
	}
	ESLAssert( m_pStream != NULL ) ;
	m_pStream->Write( &m_bufDst[0], nBlocks * cryptInBytes ) ;
	m_nBuffered = 0 ;
}

// ファイルへ書き込み
//////////////////////////////////////////////////////////////////////////////
size_t SGLEncryptRSA96OutputStream::Write( const void * ptrBuf, size_t nBytes )
{
	size_t	nWrittenBytes = 0 ;
	while ( nBytes != 0 )
	{
		size_t	nBlockBytes = nBytes ;
		if ( m_nBuffered + nBlockBytes > plainInBytes * cryptInBytes )
		{
			nBlockBytes = plainInBytes * cryptInBytes - m_nBuffered ;
		}
		eslMoveMemory( &(m_bufSrc[m_nBuffered]), ptrBuf, m_nBuffered ) ;
		m_nBuffered += nBlockBytes ;
		ptrBuf = ((uint8_t*) ptrBuf) + nBlockBytes ;
		nBytes -= nBlockBytes ;
		nWrittenBytes += nBlockBytes ;
		//
		if ( m_nBuffered >= plainInBytes * cryptInBytes )
		{
			FlushData() ;
		}
	}
	return	nWrittenBytes ;
}


//////////////////////////////////////////////////////////////////////////////
// 簡易 RSA 復号入力ストリーム
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
	( ERISA::SGLDecryptRSA96InputStream, SInputStream, RSA96CryptContext )

// ファイルから読み込み
//////////////////////////////////////////////////////////////////////////////
size_t SGLDecryptRSA96InputStream::Read( void * ptrBuf, size_t nBytes )
{
	size_t	nTotalReadBytes = 0 ;
	while ( nBytes != 0 )
	{
		if ( m_nDstBuffered > 0 )
		{
			if ( m_nDstBuffered >= nBytes )
			{
				eslMoveMemory
					( ptrBuf, &(m_bufDst[m_iDstOffset]), nBytes ) ;
				m_nDstBuffered -= nBytes ;
				m_iDstOffset += nBytes ;
				nTotalReadBytes += nBytes ;
				break ;
			}
			eslMoveMemory
				( ptrBuf, &(m_bufDst[m_iDstOffset]), m_nDstBuffered ) ;
			ptrBuf = ((uint8_t*) ptrBuf) + m_nDstBuffered ;
			nBytes -= m_nDstBuffered ;
			nTotalReadBytes += m_nDstBuffered ;
			m_nDstBuffered = 0 ;
			m_iDstOffset = 0 ;
		}
		else
		{
			if ( m_flagEOF )
			{
				break ;
			}
			size_t	nReadBytes =
						cryptInBytes * cryptInBytes - m_nSrcBuffered ;
			nReadBytes =
				m_pStream->Read( &(m_bufSrc[m_nSrcBuffered]), nReadBytes ) ;
			m_nSrcBuffered += nReadBytes ;
			if ( nReadBytes == 0 )
			{
				m_flagEOF = true ;
				break ;
			}
			size_t	nBlocks = m_nSrcBuffered / cryptInBytes ;
			for ( size_t i = 0; i < nBlocks; i ++ )
			{
				BigNumber<3>
					bnCrypt( (uint32_t*) &(m_bufSrc[i * cryptInBytes]), 3 ) ;
				Decrypt( &(m_bufDst[i * plainInBytes]), bnCrypt ) ;
			}
			eslMoveMemory
				( &(m_bufSrc[0]),
					&(m_bufSrc[nBlocks * cryptInBytes]),
					m_nSrcBuffered - nBlocks * cryptInBytes ) ;
			m_nSrcBuffered -= nBlocks * cryptInBytes ;
			m_nDstBuffered = nBlocks * plainInBytes ;
			m_iDstOffset = 0 ;
		}
	}
	return	nTotalReadBytes ;
}

#endif

