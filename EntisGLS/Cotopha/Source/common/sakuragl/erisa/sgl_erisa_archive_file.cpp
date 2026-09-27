
/*****************************************************************************
                         E R I S A - L i b r a r y
 -----------------------------------------------------------------------------
    Copyright (C) 2002-2015 Leshade Entis, Entis-soft. All rights reserved.
 *****************************************************************************/


#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl_erisa_lib.h>

using namespace SSystem ;
using namespace SakuraGL ;
using namespace ERISA ;


//////////////////////////////////////////////////////////////////////////////
// NOA 書庫ディレクトリ・エントリ
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLArchiveFile::SDirectory::SDirectory( void )
{
	m_fposDirectoryBase = 0 ;
	m_sbufDescriptor.SetBlockSize( 0x1000 ) ;
}

SGLArchiveFile::SDirectory::SDirectory( const SDirectory & dirSrc )
{
	CopyDirectoryFrom( dirSrc ) ;
}

// 代入演算子
//////////////////////////////////////////////////////////////////////////////
const SGLArchiveFile::SDirectory &
	SGLArchiveFile::SDirectory::operator =
			( const SGLArchiveFile::SDirectory & dirSrc )
{
	CopyDirectoryFrom( dirSrc ) ;
	return	*this ;
}

// 複製
//////////////////////////////////////////////////////////////////////////////
void SGLArchiveFile::SDirectory::CopyDirectoryFrom
		( const SGLArchiveFile::SDirectory & dirSrc )
{
	RemoveAll() ;
	m_fposDirectoryBase = dirSrc.m_fposDirectoryBase ;
	//
	size_t	countFile = dirSrc.GetLength() ;
	for ( size_t i = 0; i < countFile; i ++ )
	{
		FileReferenceInfo *	pfriInfo = dirSrc.GetAt( i ) ;
		ESLAssert( pfriInfo != NULL ) ;
		if ( pfriInfo == NULL )
		{
			continue ;
		}
		AddFileEntry
			( pfriInfo->pszFilename,
				*(pfriInfo->pfeEntry), pfriInfo->pfxiExtra ) ;
	}
}

// 削除
//////////////////////////////////////////////////////////////////////////////
void SGLArchiveFile::SDirectory::RemoveAll( void )
{
	m_fposDirectoryBase = 0 ;
	//
	SObjectArray<FileReferenceInfo>::RemoveAll() ;
	m_sbufDescriptor.FreeAll() ;
	m_bufDescriptor.RemoveAll() ;
}

// ディレクトリ・ディスクリプタ追加読み込み
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLArchiveFile::SDirectory::ReadDescriptor
	( SSystem::SInputStream& stream, size_t nBytes )
{
	//
	// ディスクリプタ読み込み
	//
	uint8_t *	pbytDescBuf = m_sbufDescriptor.Allocate( nBytes ) ;
	nBytes = stream.Read( pbytDescBuf, nBytes ) ;
	//
	// 順次処理
	//
	uint32_t	countFiles = *((uint32_t*)pbytDescBuf) ;
	//
	SetLimit( GetLength() + countFiles ) ;
	//
	const uint8_t *	pszLastFilename = NULL ;
	size_t	iBuf = sizeof(uint32_t) ;
	for ( size_t i = 0; i < countFiles; i ++ )
	{
		//
		// ファイル・エントリ・アドレス取得
		//
		if ( iBuf + sizeof(FILE_ENTRY) >= nBytes )
		{
			return	errFailed ;
		}
		FILE_ENTRY_EX *		pfeEntry = (FILE_ENTRY_EX*) (pbytDescBuf + iBuf) ;
		FILE_EXTRA_INFO *	pfxiExtra = NULL ;
		if ( iBuf & 0x07 )
		{
			size_t	nEntryBytes = sizeof(FILE_ENTRY)
							+ sizeof(uint32_t) + pfeEntry->nExtraInfoBytes ;
			if ( iBuf + nEntryBytes > nBytes )
			{
				return	errFailed ;
			}
			uint8_t *	pbytBuf = m_sbufDescriptor.Allocate( nEntryBytes ) ;
			eslMoveMemory( pbytBuf, pfeEntry, nEntryBytes ) ;
			pfeEntry = (FILE_ENTRY_EX*) pbytBuf ;
			if ( pfeEntry->nExtraInfoBytes != 0 )
			{
				pfxiExtra =
					(FILE_EXTRA_INFO*)
						(pbytBuf + (sizeof(FILE_ENTRY) + sizeof(uint32_t))) ;
			}
			iBuf += nEntryBytes ;
		}
		else
		{
			iBuf += sizeof(FILE_ENTRY) + sizeof(uint32_t) ;
			if ( pfeEntry->nExtraInfoBytes != 0 )
			{
				pfxiExtra =  (FILE_EXTRA_INFO*) (pbytDescBuf + iBuf) ;
			}
			iBuf += pfeEntry->nExtraInfoBytes ;
		}
		if ( iBuf + sizeof(uint32_t) >= nBytes )
		{
			return	errFailed ;
		}
		uint32_t	lenFilename = *((uint32_t*)(pbytDescBuf + iBuf)) ;
		iBuf += sizeof(uint32_t) ;
		if ( iBuf + lenFilename > nBytes )
		{
			return	errFailed ;
		}
		uint8_t *	pszFilename = pbytDescBuf + iBuf ;
		iBuf += lenFilename ;
		if ( pszFilename[lenFilename - 1] != 0 )
		{
			return	errFailed ;
		}
		//
		// 追加登録
		//
		FileReferenceInfo *	pfriInfo = new FileReferenceInfo ;
		pfriInfo->pfeEntry = pfeEntry ;
		pfriInfo->pfxiExtra = pfxiExtra ;
		pfriInfo->lenFilename = lenFilename ;
		pfriInfo->pszFilename = pszFilename ;
		//
		if ( pszLastFilename != NULL )
		{
			if ( CompareFilename( pszFilename, pszLastFilename ) >= 0 )
			{
				Add( pfriInfo ) ;
				pszLastFilename = pszFilename ;
				continue ;
			}
		}
		size_t	iOrder = OrderIndex( pszFilename ) ;
		InsertAt( iOrder, pfriInfo ) ;
		if ( iOrder == GetLength() )
		{
			pszLastFilename = pszFilename ;
		}
	}
	return	errSuccess ;
}

// ディレクトリ・ディスクリプタ書き出し
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLArchiveFile::SDirectory::WriteDescriptor
	( SSystem::SOutputStream& stream ) const
{
	uint32_t	countFiles = (uint32_t) GetLength() ;
	stream.Write( &countFiles, sizeof(uint32_t) ) ;
	//
	for ( size_t i = 0; i < countFiles; i ++ )
	{
		FileReferenceInfo *	pfriInfo = GetAt( i ) ;
		ESLAssert( pfriInfo != NULL ) ;
		if ( pfriInfo == NULL )
		{
			return	errFailed ;
		}
		stream.Write
			( pfriInfo->pfeEntry,
				sizeof(FILE_ENTRY) + sizeof(uint32_t) ) ;
		if ( pfriInfo->pfeEntry->nExtraInfoBytes != 0 )
		{
			ESLAssert( pfriInfo->pfxiExtra != NULL ) ;
			if ( pfriInfo->pfxiExtra == NULL )
			{
				return	errFailed ;
			}
			stream.Write
				( pfriInfo->pfxiExtra,
					pfriInfo->pfeEntry->nExtraInfoBytes ) ;
		}
		stream.Write( &(pfriInfo->lenFilename), sizeof(uint32_t) ) ;
		stream.Write( pfriInfo->pszFilename, pfriInfo->lenFilename ) ;
	}
	return	errSuccess ;
}

// ディレクトリ・ディスクリプタサイズ算出
//////////////////////////////////////////////////////////////////////////////
size_t SGLArchiveFile::SDirectory::GetDescriptorSize( void ) const
{
	size_t	nBytes = 0 ;
	size_t	countFiles = GetLength() ;
	for ( size_t i = 0; i < countFiles; i ++ )
	{
		FileReferenceInfo *	pfriInfo = GetAt( i ) ;
		ESLAssert( pfriInfo != NULL ) ;
		if ( pfriInfo == NULL )
		{
			continue ;
		}
		nBytes += sizeof(FILE_ENTRY) + sizeof(uint32_t)
				+ pfriInfo->pfeEntry->nExtraInfoBytes
				+ sizeof(uint32_t) + pfriInfo->lenFilename ;
	}
	return	errSuccess ;
}

// ファイル・エントリを検索
//////////////////////////////////////////////////////////////////////////////
SGLArchiveFile::FileReferenceInfo *
	SGLArchiveFile::SDirectory::GetFileInfoAs( const uint8_t * pszFilenameUTF8 )
{
	size_t	iOrder = OrderIndex( pszFilenameUTF8 ) ;
	FileReferenceInfo *	pfriInfo = GetAt( iOrder ) ;
	if ( (pfriInfo != NULL)
		&& (CompareFilename
			( pfriInfo->pszFilename, pszFilenameUTF8 ) == 0) )
	{
		return	pfriInfo ;
	}
	return	NULL ;
}

SGLArchiveFile::FileReferenceInfo *
	SGLArchiveFile::SDirectory::GetFileInfoAs( const wchar_t * pwszFilename )
{
	SArray<uint8_t>	bufFilename ;
	Charset::Encode( bufFilename, Charset::encodingUTF8, pwszFilename ) ;
	bufFilename.Add( 0 ) ;
	//
	return	GetFileInfoAs( bufFilename.GetConstArray() ) ;
}

// ファイル・エントリを追加
//////////////////////////////////////////////////////////////////////////////
size_t SGLArchiveFile::SDirectory::AddFileEntry
	( const uint8_t * pszFilenameUTF8,
		const SGLArchiveFile::FILE_ENTRY_EX& feEntry,
		const SGLArchiveFile::FILE_EXTRA_INFO * pfxiExtra )
{
	//
	// ファイル名の長さ計測
	//
	uint32_t	lenFilename = 0 ;
	while ( pszFilenameUTF8[lenFilename] != 0 )
	{
		lenFilename ++ ;
	}
	lenFilename ++ ;
	//
	// バッファ確保
	//
	FileReferenceInfo *	pfriInfo = new FileReferenceInfo ;
	SByteBuffer *	pBufDesc = new SByteBuffer ;
	uint32_t	nExtraInfoBytes = feEntry.nExtraInfoBytes ;
	if ( nExtraInfoBytes < sizeof(FILE_EXTRA_INFO) )
	{
		nExtraInfoBytes = sizeof(FILE_EXTRA_INFO) ;
	}
	m_bufDescriptor.Add( pBufDesc ) ;
	pBufDesc->SetLength
		( sizeof(FILE_ENTRY) + sizeof(uint32_t)
			+ nExtraInfoBytes
			+ sizeof(uint32_t) + lenFilename ) ;
	//
	// FILE_ENTRY 複製
	//
	uint8_t *	pbytBuf = pBufDesc->GetArray() ;
	pfriInfo->pfeEntry = (FILE_ENTRY_EX*) pbytBuf ;
	eslCopyMemory
		( pbytBuf, &feEntry,
			sizeof(FILE_ENTRY) + sizeof(uint32_t) ) ;
	//
	pfriInfo->pfeEntry->nExtraInfoBytes = nExtraInfoBytes ;
	pbytBuf += sizeof(FILE_ENTRY) + sizeof(uint32_t) ;
	//
	// FILE_EXTRA_INFO 複製
	//
	pfriInfo->pfxiExtra = (FILE_EXTRA_INFO*) pbytBuf ;
	if ( feEntry.nExtraInfoBytes != 0 )
	{
		eslCopyMemory( pbytBuf, pfxiExtra, feEntry.nExtraInfoBytes ) ;
	}
	pbytBuf += nExtraInfoBytes ;
	//
	// ファイル名複製
	//
	pfriInfo->lenFilename = lenFilename ;
	*((uint32_t*)pbytBuf) = lenFilename ;
	pbytBuf += sizeof(uint32_t) ;
	//
	pfriInfo->pszFilename = pbytBuf ;
	eslCopyMemory( pbytBuf, pszFilenameUTF8, lenFilename ) ;
	//
	pBufDesc->FinishArray() ;
	//
	// 登録
	//
	size_t	iEntry = OrderIndex( pszFilenameUTF8 ) ;
	InsertAt( iEntry, pfriInfo ) ;
	return	iEntry ;
}

size_t SGLArchiveFile::SDirectory::AddFileEntry
	( const wchar_t * pwszFilename,
		const SGLArchiveFile::FILE_ENTRY_EX& feEntry,
		const SGLArchiveFile::FILE_EXTRA_INFO * pfxiExtra )
{
	SArray<uint8_t>	bufFilename ;
	Charset::Encode( bufFilename, Charset::encodingUTF8, pwszFilename ) ;
	bufFilename.Add( 0 ) ;
	//
	return	AddFileEntry( bufFilename.GetConstArray(), feEntry, pfxiExtra ) ;
}

// 指標検索
//////////////////////////////////////////////////////////////////////////////
size_t SGLArchiveFile::SDirectory::OrderIndex( const uint8_t * pszFilenameUTF8 ) const
{
	FileReferenceInfo *		pfriInfo ;
	ssize_t	iFirst, iEnd, iMiddle = 0 ;
	iFirst = 0 ;
	iEnd = (ssize_t) SArray<FileReferenceInfo*>::m_nLength - 1 ;
	//
	while ( iFirst <= iEnd )
	{
		iMiddle = ((iFirst + iEnd) >> 1) ;
		pfriInfo = SArray<FileReferenceInfo*>::m_ptrArray[iMiddle] ;
		ESLAssert( pfriInfo != NULL ) ;
		//
		int	nCompare =
			CompareFilename( pfriInfo->pszFilename, pszFilenameUTF8 ) ;
		if ( nCompare > 0 )
		{
			iEnd = iMiddle - 1 ;
		}
		else if ( nCompare < 0 )
		{
			iFirst = iMiddle + 1 ;
		}
		else
		{
			return	iMiddle ;
		}
	}
	return	iFirst ;
}

// ファイル名比較
//////////////////////////////////////////////////////////////////////////////
int SGLArchiveFile::SDirectory::CompareFilename
	( const uint8_t * pszFile1, const uint8_t * pszFile2 )
{
	size_t	i = 0 ;
	while ( pszFile2[i] != 0 )
	{
		int	c1 = pszFile1[i] ;
		int	c2 = pszFile2[i ++] ;
		if ( (c1 >= L'A') && (c1 <= L'Z') )
		{
			c1 += L'a' - L'A' ;
		}
		if ( (c2 >= L'A') && (c2 <= L'Z') )
		{
			c2 += L'a' - L'A' ;
		}
		int	cmp = c1 - c2 ;
		if ( cmp != 0 )
		{
			return	cmp ;
		}
	}
	return	(int) pszFile1[i] ;
}


//////////////////////////////////////////////////////////////////////////////
// ファイル参照
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ERISA::SGLArchiveFile::RefFile, SSmartFile )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLArchiveFile::RefFile::RefFile( SGLArchiveFile * pArcFile )
	: SSmartFile( pArcFile, pArcFile, false ), m_errResult( errPending )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLArchiveFile::RefFile::~RefFile( void )
{
	if ( m_pFile != NULL )
	{
		SGLArchiveFile *	pArcFile = ESLTypeCast<SGLArchiveFile>( m_pFile ) ;
		if ( pArcFile != NULL )
		{
			m_errResult = pArcFile->AscendFile() ;
		}
	}
}

// ファイルの参照を解除する
//////////////////////////////////////////////////////////////////////////////
void SGLArchiveFile::RefFile::Close( void )
{
	if ( m_pFile != NULL )
	{
		SGLArchiveFile *	pArcFile = ESLTypeCast<SGLArchiveFile>( m_pFile ) ;
		if ( pArcFile != NULL )
		{
			m_errResult = pArcFile->AscendFile() ;
		}
	}
	SSmartFile::Close() ;
}

// エラーコード取得
//////////////////////////////////////////////////////////////////////////////
SError SGLArchiveFile::RefFile::GetError( void ) const
{
	return	m_errResult ;
}


//////////////////////////////////////////////////////////////////////////////
// NOA 書庫ファイル
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ERISA::SGLArchiveFile, SChunkFile )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLArchiveFile::SGLArchiveFile( void )
{
	m_pdirCurrent = NULL ;
	m_pfriFile = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLArchiveFile::~SGLArchiveFile( void )
{
	SGLArchiveFile::CloseArchive() ;
}

// 書庫ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLArchiveFile::OpenArchive
	( SFileInterface * pFile, bool flagOwner,
		long int nFlags, SDirectory * pRootDir )
{
	SSmartLock<SCriticalSection>	smartLock( &m_csSync ) ;
	//
	// ファイルヘッダ読み込み／書き出し
	//
	FILE_HEADER	fhHeader ;
	if ( nFlags & SFileOpener::modeCreateFlag )
	{
		const char *	pszFormatDesc = "ERISA archive file" ;
		eslFillMemory( &fhHeader, 0, sizeof(FILE_HEADER) ) ;
		eslMoveMemory
			( &fhHeader.bytSignature[0],
				&m_bytDefaultSignature[0], 8 ) ;
		fhHeader.dwFileID = 0x02000400 ;
		for ( int i = 0; (i < 0x30) & (pszFormatDesc[i] != 0); i ++ )
		{
			fhHeader.bytFormatDesc[i] = (BYTE) pszFormatDesc[i] ;
		}
	}
	SError	err = OpenChunkFile( pFile, flagOwner, nFlags, &fhHeader ) ;
	if ( err )
	{
		if ( flagOwner )
		{
			delete	pFile ;
		}
		return	err ;
	}
	if ( IsFileCreatingMode() )
	{
		//
		// ルートディレクトリ書き出し
		//
		if ( pRootDir == NULL )
		{
			return	errFailed ;
		}
		m_strCurDirectory = L"" ;
		m_pdirCurrent = new SDirectory( *pRootDir ) ;
		m_pdirCurrent->SetBaseFilePosition( m_pFile->GetPosition() ) ;
		m_cacheDir.SetAs( m_strCurDirectory, m_pdirCurrent ) ;
		//
		err = WriteDirectoryDescription( *m_pdirCurrent ) ;
		if ( err )
		{
			return	err ;
		}
	}
	else
	{
		//
		// ルートディレクトリ読み込み
		//
		m_strCurDirectory = L"" ;
		m_pdirCurrent = new SDirectory() ;
		m_pdirCurrent->SetBaseFilePosition( m_pFile->GetPosition() ) ;
		m_cacheDir.SetAs( m_strCurDirectory, m_pdirCurrent ) ;
		//
		err = ReadDirectoryDescription( *m_pdirCurrent ) ;
		if ( err )
		{
			return	err ;
		}
	}
	return	errSuccess ;
}

// 書庫ファイルを閉じる
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLArchiveFile::CloseArchive( void )
{
	SSmartLock<SCriticalSection>	smartLock( &m_csSync ) ;
	//
	if ( m_pfriFile != NULL )
	{
		AscendFile() ;
	}
	if ( m_pdirCurrent != NULL )
	{
		if ( IsFileWritingMode() )
		{
			// ディレクトリ・ディスクリプタ書き出し
			Seek( 0 ) ;
			WriteDirectoryDescription( *m_pdirCurrent ) ;
		}
	}
	m_cacheDir.RemoveAll() ;
	//
	m_pfriFile = NULL ;
	m_strCurDirectory.FreeArray() ;
	m_pdirCurrent = NULL ;
	//
	SChunkFile::Close() ;
	//
	return	errSuccess ;
}

// 書庫ファイルを閉じる
//////////////////////////////////////////////////////////////////////////////
void SGLArchiveFile::Close( void )
{
	CloseArchive() ;
}

// ディレクトリを書き出す
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLArchiveFile::WriteDirectoryDescription
						( SGLArchiveFile::SDirectory& dir )
{
	ESLAssert( m_pFile != NULL ) ;
	dir.SetBaseFilePosition( m_pFile->GetPosition() ) ;
	//
	SError	err = DescendChunk( "DirEntry" ) ;
	if ( err )
	{
		return	err ;
	}
	dir.WriteDescriptor( *this ) ;
	//
	return	AscendChunk() ;
}

// ディレクトリを読み込む
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLArchiveFile::ReadDirectoryDescription
						( SGLArchiveFile::SDirectory& dir )
{
	ESLAssert( m_pFile != NULL ) ;
	dir.SetBaseFilePosition( m_pFile->GetPosition() ) ;
	//
	SError	err = DescendChunk() ;
	if ( err )
	{
		return	err ;
	}
	if ( !IsEqualCurrentChunkID( "DirEntry" ) )
	{
		AscendChunk() ;
		return	errFailed ;
	}
	dir.ReadDescriptor( *m_pFile, (size_t) GetCurrentChunkLength() ) ;
	//
	return	AscendChunk() ;
}

// エンコーダーを設定する
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLArchiveFile::PrepareEncoder
	( SGLArchiveFile::FileReferenceInfo * pfriInfo,
		SOutputStream * pStream, const wchar_t * pwszPassword )
{
	FILE_ENTRY_EX *		pfeEntry = pfriInfo->pfeEntry ;
	FILE_EXTRA_INFO *	pfxiExtra = NULL ;
	if ( pfeEntry->nExtraInfoBytes >= sizeof(FILE_EXTRA_INFO) )
	{
		pfxiExtra = pfriInfo->pfxiExtra ;
		pfxiExtra->nCRC32 = 0 ;
		pfxiExtra->nDecrypeKey[0] = 0 ;
	}
	if ( pfeEntry->nEncodeType == encodeRaw )
	{
		m_pOutCRC32 = NULL ;
	}
	else if ( pfeEntry->nEncodeType == encodeERISA )
	{
		m_pEncBitStream = new ERISA::SGLEncodeBitStream( 0x4000 ) ;
		m_pEncBitStream->AttachOutputStream( pStream ) ;
		//
		m_pEncERISAN = new ERISA::SGLERISANEncodeContext( m_pEncBitStream ) ;
		m_pEncERISAN->PrepareToEncodeERISANCode() ;
		//
		m_pOutCRC32 = new SakuraCL::CRC32OutputStream
							( (SGLERISANEncodeContext*) m_pEncERISAN ) ;
	}
	else if ( pfeEntry->nEncodeType == encodeCrypt32 )
	{
		m_pEncrypt32 = new ERISA::SGLEncrypt32OutputStream( pStream ) ;
		m_pEncrypt32->Initialize( pwszPassword ) ;
		ESLAssert( pfxiExtra != NULL ) ;
		if ( pfxiExtra != NULL )
		{
			pfxiExtra->nDecrypeKey[0] = m_pEncrypt32->GenerateKey() ;
		}
		else
		{
			ESLTrace( "Needs extra info for Crypt32\n" ) ;
			return	errFailed ;
		}
		m_pOutCRC32 = new SakuraCL::CRC32OutputStream
							( (SGLEncrypt32OutputStream*) m_pEncrypt32 ) ;
	}
	else if ( pfeEntry->nEncodeType == encodeERISACrypt32 )
	{
		m_pEncrypt32 = new ERISA::SGLEncrypt32OutputStream( pStream ) ;
		m_pEncrypt32->Initialize( pwszPassword ) ;
		if ( pfxiExtra != NULL )
		{
			pfxiExtra->nDecrypeKey[0] = m_pEncrypt32->GenerateKey() ;
		}
		else
		{
			ESLTrace( "Needs extra info for ERISA-Crypt32\n" ) ;
			return	errFailed ;
		}
		//
		m_pEncBitStream = new ERISA::SGLEncodeBitStream( 0x4000 ) ;
		m_pEncBitStream->AttachOutputStream
							( (SGLEncrypt32OutputStream*) m_pEncrypt32 ) ;
		//
		m_pEncERISAN = new ERISA::SGLERISANEncodeContext( m_pEncBitStream ) ;
		m_pEncERISAN->PrepareToEncodeERISANCode() ;
		//
		m_pOutCRC32 = new SakuraCL::CRC32OutputStream
							( (SGLERISANEncodeContext*) m_pEncERISAN ) ;
	}
	else
	{
		ESLTrace( "Invalid encoding type\n" ) ;
		return	errFailed ;
	}
	return	errSuccess ;
}

// デコーダーを設定する
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLArchiveFile::PrepareDecoder
	( SGLArchiveFile::FileReferenceInfo * pfriInfo,
		SInputStream * pStream, const wchar_t * pwszPassword )
{
	FILE_ENTRY_EX *		pfeEntry = pfriInfo->pfeEntry ;
	FILE_EXTRA_INFO *	pfxiExtra = NULL ;
	if ( pfeEntry->nExtraInfoBytes >= sizeof(FILE_EXTRA_INFO) )
	{
		pfxiExtra = pfriInfo->pfxiExtra ;
	}
	if ( pfeEntry->nEncodeType == encodeERISA )
	{
		m_pDecBitStream = new ERISA::SGLDecodeBitStream( 0x10000 ) ;
		m_pDecBitStream->AttachInputStream( pStream ) ;
		//
		m_pDecERISAN =
			new ERISA::SGLERISANDecodeContext( m_pDecBitStream ) ;
		m_pDecERISAN->PrepareToDecodeERISANCode() ;
		//
		m_pInCRC32 = new SakuraCL::CRC32InputStream
							( (SGLERISANDecodeContext*) m_pDecERISAN ) ;
	}
	else if ( pfeEntry->nEncodeType == encodeCrypt32 )
	{
		m_pDecrypt32 =
			new ERISA::SGLDecrypt32InputStream( pStream ) ;
		m_pDecrypt32->Initialize( pwszPassword ) ;
		if ( pfxiExtra != NULL )
		{
			m_pDecrypt32->SetDecryptKey( pfxiExtra->nDecrypeKey[0] ) ;
		}
		else
		{
			ESLTrace( "Needs extra info for ERISA-Crypt32\n" ) ;
			return	errFailed ;
		}
		m_pInCRC32 = new SakuraCL::CRC32InputStream
							( (SGLDecrypt32InputStream*) m_pDecrypt32 ) ;
	}
	else if ( pfeEntry->nEncodeType == encodeERISACrypt32 )
	{
		m_pDecrypt32 =
			new ERISA::SGLDecrypt32InputStream( pStream ) ;
		m_pDecrypt32->Initialize( pwszPassword ) ;
		if ( pfxiExtra != NULL )
		{
			m_pDecrypt32->SetDecryptKey( pfxiExtra->nDecrypeKey[0] ) ;
		}
		else
		{
			ESLTrace( "Needs extra info for ERISA-Crypt32\n" ) ;
			return	errFailed ;
		}
		m_pDecBitStream = new ERISA::SGLDecodeBitStream( 0x10000 ) ;
		m_pDecBitStream->AttachInputStream
							( (SGLDecrypt32InputStream*) m_pDecrypt32 ) ;
		//
		m_pDecERISAN =
			new ERISA::SGLERISANDecodeContext( m_pDecBitStream ) ;
		m_pDecERISAN->PrepareToDecodeERISANCode() ;
		//
		m_pInCRC32 = new SakuraCL::CRC32InputStream
							( (SGLERISANDecodeContext*) m_pDecERISAN ) ;
	}
	else
	{
		ESLTrace( "Invalid encoding type\n" ) ;
		return	errFailed ;
	}
	return	errSuccess ;
}

// デコード済みのバッファを生成する
//////////////////////////////////////////////////////////////////////////////
SSmartBuffer * SGLArchiveFile::CreateDecodedFile
		( SGLArchiveFile::FileReferenceInfo * pfriInfo )
{
	if ( m_pInCRC32 == NULL )
	{
		return	NULL ;
	}
	SSmartBuffer *	pFileBuffer = new SSmartBuffer ;
	FILE_ENTRY_EX *	pfeEntry = pfriInfo->pfeEntry ;
	pFileBuffer->ReadFromStream
		( *m_pInCRC32, (ssize_t) pfeEntry->nBytes ) ;
	//
	uint32_t	nCRC32 = m_pInCRC32->GetCRC32() ;
	m_pDecrypt32 = NULL ;
	m_pDecBitStream = NULL ;
	m_pDecERISAN = NULL ;
	m_pInCRC32 = NULL ;
	//
	if ( (pfeEntry->nExtraInfoBytes >= sizeof(FILE_EXTRA_INFO))
		&& (pfriInfo->pfxiExtra != NULL)
		&& (pfriInfo->pfxiExtra->nCRC32 != nCRC32) )
	{
		ESLTrace( "CRC is not match\n" ) ;
		delete	pFileBuffer ;
		return	NULL ;
	}
	return	pFileBuffer ;
}

// サブディレクトリを開く
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLArchiveFile::DescendDirectory
	( const wchar_t * pwszDirName, SDirectory * pWriteDir )
{
	SSmartLock<SCriticalSection>	smartLock( &m_csSync ) ;
	//
	if ( m_pfriFile != NULL )
	{
		AscendFile() ;
	}
	if ( m_pdirCurrent == NULL )
	{
		return	errFailed ;
	}
	//
	// ディレクトリ・エントリ検索
	//
	FileReferenceInfo *	pfriDir = m_pdirCurrent->GetFileInfoAs( pwszDirName ) ;
	if ( pfriDir == NULL )
	{
		return	errFailed ;
	}
	if ( pWriteDir != NULL )
	{
		//
		// ディレクトリ書き出し
		//
		if ( !IsFileWritingMode() )
		{
			return	errFailed ;
		}
		pfriDir->pfeEntry->nOffsetPos = GetPosition() ;
		//
		SError	err = DescendChunk( "filedata" ) ;
		if ( err )
		{
			return	err ;
		}
		//
		m_strCurDirectory = m_strCurDirectory.OffsetFilePath( pwszDirName ) ;
		m_strCurDirectory.MakeLower() ;
		//
		m_pdirCurrent = new SDirectory( *pWriteDir ) ;
		m_cacheDir.SetAs( m_strCurDirectory, m_pdirCurrent ) ;
		//
		return	WriteDirectoryDescription( *m_pdirCurrent ) ;
	}
	else
	{
		//
		// ディレクトリ読み込み
		//
		Seek( pfriDir->pfeEntry->nOffsetPos ) ;
		//
		SError	err = DescendChunk() ;
		if ( err )
		{
			return	err ;
		}
		if ( !IsEqualCurrentChunkID( "filedata" ) )
		{
			return	errFailed ;
		}
		//
		m_strCurDirectory = m_strCurDirectory.OffsetFilePath( pwszDirName ) ;
		m_strCurDirectory.MakeLower() ;
		//
		SDirectory *	pSubDir = m_cacheDir.GetAs( m_strCurDirectory ) ;
		if ( pSubDir == NULL )
		{
			pSubDir = new SDirectory ;
			err = ReadDirectoryDescription( *pSubDir ) ;
			if ( err )
			{
				delete	pSubDir ;
				return	err ;
			}
			m_cacheDir.SetAs( m_strCurDirectory, pSubDir ) ;
		}
		m_pdirCurrent = pSubDir ;
	}
	return	errSuccess ;
}

// ディレクトリを一つ上に移動
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLArchiveFile::AscendDirectory( void )
{
	SSmartLock<SCriticalSection>	smartLock( &m_csSync ) ;
	//
	if ( m_pfriFile != NULL )
	{
		AscendFile() ;
	}
	if ( m_strCurDirectory.IsEmpty() )
	{
		return	errFailed ;
	}
	if ( m_pdirCurrent != NULL )
	{
		if ( IsFileWritingMode() )
		{
			// ディレクトリ・ディスクリプタ書き出し
			Seek( 0 ) ;
			WriteDirectoryDescription( *m_pdirCurrent ) ;
		}
		AscendChunk() ;
	}
	//
	// 親ディレクトリ情報取得
	//
	SString	strParentDirPath = m_strCurDirectory.GetFileDirectoryPart() ;
	wchar_t	wchLast = strParentDirPath.GetLastAt( 0 ) ;
	if ( wchLast == L'\\' )
	{
		m_strCurDirectory =
			strParentDirPath.Left( strParentDirPath.GetLength() - 1 ) ;
	}
	else
	{
		m_strCurDirectory = strParentDirPath ;
	}
	m_pdirCurrent = m_cacheDir.GetAs( m_strCurDirectory ) ;
	if ( m_pdirCurrent == NULL )
	{
		return	errFailed ;
	}
	return	errSuccess ;
}

// 書庫内のファイルを開く
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLArchiveFile::DescendFile
	( const wchar_t * pwszFilename,
		const wchar_t * pwszPassword,
		SGLArchiveFile::FileOpenMethod openMethod )
{
	SSmartLock<SCriticalSection>	smartLock( &m_csSync ) ;
	//
	if ( m_pfriFile != NULL )
	{
		AscendFile() ;
	}
	if ( m_pdirCurrent == NULL )
	{
		return	errFailed ;
	}
	//
	// ファイル・エントリ検索
	//
	m_pfriFile = m_pdirCurrent->GetFileInfoAs( pwszFilename ) ;
	if ( m_pfriFile == NULL )
	{
		SString			strFileName = pwszFilename ;
		SArray<char>	bufFileName ;
		ESLTrace( "not found \'%s\' file entry.\n",
						strFileName.EncodeDefaultTo( bufFileName )  ) ;
		return	errFailed ;
	}
	if ( IsFileWritingMode() )
	{
		//
		// 書き出しモードで開く
		//
		FILE_ENTRY *	pfeEntry = m_pfriFile->pfeEntry ;
		pfeEntry->nOffsetPos = GetPosition() ;
		pfeEntry->nBytes = 0 ;
		//
		SError	err = DescendChunk( "filedata" ) ;
		if ( err )
		{
			m_pfriFile = NULL ;
			return	err ;
		}
		//
		// エンコーディング設定
		//
		err = PrepareEncoder( m_pfriFile, m_pFile, pwszPassword ) ;
		if ( err )
		{
			return	err ;
		}
	}
	else
	{
		//
		// 読み込みモードで開く
		//
		FILE_ENTRY *	pfeEntry = m_pfriFile->pfeEntry ;
		Seek( pfeEntry->nOffsetPos ) ;
		//
		SError	err = DescendChunk() ;
		if ( err )
		{
			m_pfriFile = NULL ;
			return	err ;
		}
		if ( !IsEqualCurrentChunkID( "filedata" ) )
		{
			return	errFailed ;
		}
		if ( pfeEntry->nEncodeType == encodeRaw )
		{
			return	errSuccess ;
		}
		//
		// デコーダ設定
		//
		err = PrepareDecoder( m_pfriFile, m_pFile, pwszPassword ) ;
		if ( err )
		{
			return	err ;
		}
		if ( (openMethod == openAsNormal)
				&& (pfeEntry->nBytes <= 0x40000000 /*1GB*/) )
		{
			//
			// バッファに展開
			//
			m_pFileBuffer = CreateDecodedFile( m_pfriFile ) ;
			if ( m_pFileBuffer == NULL )
			{
				return	errFailed ;
			}
		}
	}
	return	errSuccess ;
}

// 書庫内のファイルを閉じる
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLArchiveFile::AscendFile( void )
{
	SSmartLock<SCriticalSection>	smartLock( &m_csSync ) ;
	//
	SError	errResult = errSuccess ;
	if ( m_pfriFile != nullptr )
	{
		if ( IsFileWritingMode() )
		{
			//
			// 書き出し完了
			//
			FILE_EXTRA_INFO *	pfxiExtra = nullptr ;
			FILE_ENTRY_EX *	pfeEntry = m_pfriFile->pfeEntry ;
			if ( pfeEntry->nExtraInfoBytes >= sizeof(FILE_EXTRA_INFO) )
			{
				pfxiExtra = m_pfriFile->pfxiExtra ;
			}
			if ( m_pEncERISAN != nullptr )
			{
				m_pEncERISAN->FinishERISACode() ;
			}
			if ( m_pEncBitStream != nullptr )
			{
				m_pEncBitStream->Flushout() ;
			}
			if ( m_pEncrypt32 != nullptr )
			{
				m_pEncrypt32->FlushData() ;
			}
			if ( m_pOutCRC32 != nullptr )
			{
				if ( pfxiExtra != nullptr )
				{
					pfxiExtra->nCRC32 = m_pOutCRC32->GetCRC32() ;
				}
				if ( m_pEncrypt32 == nullptr )
				{
					// ※古いローダーで読めるよう互換性の為
					uint32_t	xor32 = m_pOutCRC32->GetXOR32() ;
					m_pFile->Write( &xor32, sizeof(uint32_t) ) ;
				}
			}
			m_pOutCRC32 = nullptr ;
			m_pEncERISAN = nullptr ;
			m_pEncBitStream = nullptr ;
			m_pEncrypt32 = nullptr ;
			//
			UpdateFilePointer() ;
			//
			if ( pfeEntry->nEncodeType == encodeRaw )
			{
				pfeEntry->nBytes = GetCurrentChunkLength() ;
			}
		}
		else
		{
			//
			// 読み込み完了
			//
			FILE_EXTRA_INFO *	pfxiExtra = nullptr ;
			FILE_ENTRY_EX *	pfeEntry = m_pfriFile->pfeEntry ;
			if ( pfeEntry->nExtraInfoBytes >= sizeof(FILE_EXTRA_INFO) )
			{
				pfxiExtra = m_pfriFile->pfxiExtra ;
			}
			if ( m_pInCRC32 != nullptr )
			{
				uint32_t	nCRC32 = m_pInCRC32->GetCRC32() ;
				if ( pfxiExtra != nullptr )
				{
					if ( pfxiExtra->nCRC32 != nCRC32 )
					{
						ESLTrace( "CRC is not match\n" ) ;
						errResult = errFailed ;
					}
				}
			}
			m_pDecrypt32 = nullptr ;
			m_pDecBitStream = nullptr ;
			m_pDecERISAN = nullptr ;
			m_pInCRC32 = nullptr ;
		}
		AscendChunk() ;
		m_pfriFile = nullptr ;
	}
	m_pFileBuffer = nullptr ;
	return	errResult ;
}

// ディレクトリ情報をロードし取得
//////////////////////////////////////////////////////////////////////////////
SGLArchiveFile::SDirectory *
	SGLArchiveFile::LoadDirectoryDescriptorAs( const wchar_t * pwszDirPath )
{
	SSmartLock<SCriticalSection>	smartLock( &m_csSync ) ;
	//
	// 既にキャッシュされているディレクトリ・ディスクリプタを検索
	//
	if ( (pwszDirPath == NULL) || (pwszDirPath[0] == 0) )
	{
		return	m_cacheDir.GetAs( L"" ) ;
	}
	SString	strDirPath = pwszDirPath ;
	NormalizeDirectoryPath( strDirPath ) ;
	//
	SGLArchiveFile::SDirectory *	pDir = m_cacheDir.GetAs( strDirPath ) ;
	if ( pDir != NULL )
	{
		return	pDir ;
	}
	//
	// ルートディレクトリに移動
	//
	if ( m_pfriFile != NULL )
	{
		AscendFile() ;
	}
	while ( (m_pdirCurrent != NULL) && !m_strCurDirectory.IsEmpty() )
	{
		if ( AscendDirectory() )
		{
			return	NULL ;
		}
	}
	if ( m_pdirCurrent == NULL )
	{
		return	NULL ;
	}
	//
	// 順次ディレクトリを検索
	//
	size_t	iLastDir = 0 ;
	for ( ; ; )
	{
		SString	strDirName ;
		ssize_t	iNextDir = strDirPath.Find( L'\\', iLastDir ) ;
		if ( iNextDir >= 0 )
		{
			ESLAssert( (size_t) iNextDir >= iLastDir ) ;
			strDirName = strDirPath.Middle( iLastDir, (ssize_t) (iNextDir - iLastDir) ) ;
		}
		else
		{
			strDirName = strDirPath.Middle( iLastDir ) ;
		}
		if ( DescendDirectory( strDirName ) )
		{
			return	NULL ;
		}
		if ( iNextDir < 0 )
		{
			break ;
		}
		iLastDir = iNextDir + 1 ;
	}
	return	m_pdirCurrent ;
}

// ディレクトリパスを正規化
//////////////////////////////////////////////////////////////////////////////
void SGLArchiveFile::NormalizeDirectoryPath( SSystem::SString& strDirPath )
{
	size_t		nLength = strDirPath.GetLength() ;
	size_t		nNormalLen = nLength ;
	uint16_t *	pszDirPath = strDirPath.LockBuffer( nLength ) ;
	for ( size_t i = 0; i < nLength; i ++ )
	{
		uint16_t	wch = pszDirPath[i] ;
		nNormalLen = i + 1 ;
		if ( (wch >= L'A') && (wch <= L'Z') )
		{
			pszDirPath[i] = (uint16_t) (wch + (L'a' - L'A')) ;
		}
		else if ( wch == L'/' )
		{
			pszDirPath[i] = L'\\' ;
			nNormalLen -- ;
		}
		else if ( wch == L'\\' )
		{
			nNormalLen -- ;
		}
	}
	strDirPath.UnlockBuffer( (ssize_t) nNormalLen ) ;
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SFileInterface * SGLArchiveFile::NewOpenFile
	( const wchar_t * pszFilePath, long int nOpenFlags )
{
	SSmartLock<SCriticalSection>	smartLock( &m_csSync ) ;
	if ( nOpenFlags & modeStreaming )
	{
		SError	err = DescendFile( pszFilePath, NULL, openAsStream ) ;
		if ( err )
		{
			return	NULL ;
		}
		return	new RefFile( this ) ;
	}
	//
	// ファイル・エントリを検索する
	//
	SString	strFilePath = pszFilePath ;
	SDirectory *	pDir =
		LoadDirectoryDescriptorAs( strFilePath.GetFileDirectoryPart() ) ;
	if ( pDir == NULL )
	{
		return	NULL ;
	}
	SString	strFileName = strFilePath.GetFileNamePart() ;
	FileReferenceInfo *	pfriInfo = pDir->GetFileInfoAs( strFileName ) ;
	if ( pfriInfo == NULL )
	{
		return	NULL ;
	}
	FILE_ENTRY *	pfeEntry = pfriInfo->pfeEntry ;
	int64_t	posFileBase =
				pDir->GetBaseFilePosition() + pfeEntry->nOffsetPos ;
	//
	SChunkFile::CHUNK_HEADER	header ;
	m_pFile->Seek( posFileBase ) ;
	if ( m_pFile->Read
		( &header, sizeof(SChunkFile::CHUNK_HEADER) )
							< sizeof(SChunkFile::CHUNK_HEADER) )
	{
		return	NULL ;
	}
	posFileBase += sizeof(SChunkFile::CHUNK_HEADER) ;
	//
	if ( pfeEntry->nEncodeType == encodeRaw )
	{
		//
		// 無圧縮ファイルは参照元ファイルを複製して部分参照を生成する
		//
		SFileInterface *	pFile = m_pFile->Duplicate() ;
		if ( pFile == NULL )
		{
			return	NULL ;
		}
		pFile->Seek( posFileBase ) ;
		//
		SFileDomainInterface *	pDomainFile =
			new SFileDomainInterface
				( pFile, true,
					(m_nFlags & SFileOpener::modeReadWrite),
					posFileBase, pfeEntry->nBytes ) ;
		//
		return	new SSmartFile( this, pDomainFile ) ;
	}
	//
	// 圧縮／暗号化されたファイルは展開する
	//
	SError	err ;
	SSmartPointer<SFileInterface>	pFile = m_pFile->Duplicate() ;
	if ( pFile != NULL )
	{
		pFile->Seek( posFileBase ) ;
		err = PrepareDecoder( pfriInfo, pFile.Ptr(), m_strDefPassword ) ;
	}
	else
	{
		ESLTrace( "Failed file Duplicate() at SGLArchiveFile::NewOpenFile()\n" ) ;
		m_pFile->Seek( posFileBase ) ;
		err = PrepareDecoder( pfriInfo, m_pFile, m_strDefPassword ) ;
	}
	if ( err )
	{
		return	NULL ;
	}
	SSmartBuffer *	pFileBuffer = CreateDecodedFile( pfriInfo ) ;
	if ( pFileBuffer == NULL )
	{
		return	NULL ;
	}
	return	new SSmartFile( this, pFileBuffer ) ;
}

// ファイルの存在
//////////////////////////////////////////////////////////////////////////////
bool SGLArchiveFile::IsExisting( const wchar_t * pszFilePath )
{
	SSmartLock<SCriticalSection>	smartLock( &m_csSync ) ;
	//
	SString	strFilePath = pszFilePath ;
	SDirectory *	pDir =
		LoadDirectoryDescriptorAs( strFilePath.GetFileDirectoryPart() ) ;
	if ( pDir != NULL )
	{
		SString	strFileName = strFilePath.GetFileNamePart() ;
		return	(pDir->GetFileInfoAs( strFileName ) != NULL) ;
	}
	return	false ;
}

// ファイル状態
//////////////////////////////////////////////////////////////////////////////
SError SGLArchiveFile::QueryState
	( const wchar_t * pszFilePath, SFileOpener::State& state )
{
	SSmartLock<SCriticalSection>	smartLock( &m_csSync ) ;
	//
	SString	strFilePath = pszFilePath ;
	SDirectory *	pDir =
		LoadDirectoryDescriptorAs( strFilePath.GetFileDirectoryPart() ) ;
	if ( pDir == NULL )
	{
		return	errFailed ;
	}
	SString	strFileName = strFilePath.GetFileNamePart() ;
	FileReferenceInfo *	pfri = pDir->GetFileInfoAs( strFileName ) ;
	if ( pfri == NULL )
	{
		return	errFailed ;
	}
	FILE_ENTRY_EX *	pfex = pfri->pfeEntry ;
	ESLAssert( pfex != NULL ) ;
	if ( pfex == NULL )
	{
		return	errFailed ;
	}
	state.bitFields = fieldFileSize | fieldModifiedTime ;
	state.bitAttributes = 0 ;
	state.nFileSize = pfex->nBytes ;
	state.dtModified.nYear = pfex->ftFileTime.nYear ;
	state.dtModified.nMonth = pfex->ftFileTime.nMonth ;
	state.dtModified.nDay = pfex->ftFileTime.nDay ;
	state.dtModified.nWeek = pfex->ftFileTime.nWeek ;
	state.dtModified.nHour = pfex->ftFileTime.nHour ;
	state.dtModified.nMinute = pfex->ftFileTime.nMinute ;
	state.dtModified.nSecond = pfex->ftFileTime.nSecond ;
	state.dtModified.nMilliSec = 0 ;
	return	errSuccess ;
}

// ファイルの一覧取得
//////////////////////////////////////////////////////////////////////////////
void SGLArchiveFile::ListSubFiles
	( SObjectArray<SString>& listFiles, const wchar_t * pszDirPath )
{
	SSmartLock<SCriticalSection>	smartLock( &m_csSync ) ;
	//
	listFiles.RemoveAll() ;
	//
	SDirectory *	pDir = LoadDirectoryDescriptorAs( pszDirPath ) ;
	if ( pDir != NULL )
	{
		const size_t	nCount = pDir->GetLength() ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			FileReferenceInfo *	pfriInfo = pDir->GetAt( i ) ;
			if ( pfriInfo != NULL )
			{
				FILE_ENTRY *	pfeEntry = pfriInfo->pfeEntry ;
				if ( !(pfeEntry->nAttribute & attrDirectory) )
				{
					SString *	pFilename = new SString ;
					Charset::Decode
						( *pFilename, Charset::encodingUTF8,
											pfriInfo->pszFilename ) ;
					listFiles.Add( pFilename ) ;
				}
			}
		}
	}
}

// ディレクトリの一覧取得
//////////////////////////////////////////////////////////////////////////////
void SGLArchiveFile::ListSubDirectories
	( SObjectArray<SString>& listDirs, const wchar_t * pszDirPath )
{
	SSmartLock<SCriticalSection>	smartLock( &m_csSync ) ;
	//
	listDirs.RemoveAll() ;
	//
	SDirectory *	pDir = LoadDirectoryDescriptorAs( pszDirPath ) ;
	if ( pDir != NULL )
	{
		const size_t	nCount = pDir->GetLength() ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			FileReferenceInfo *	pfriInfo = pDir->GetAt( i ) ;
			if ( pfriInfo != NULL )
			{
				FILE_ENTRY *	pfeEntry = pfriInfo->pfeEntry ;
				if ( pfeEntry->nAttribute & attrDirectory )
				{
					SString *	pFilename = new SString ;
					Charset::Decode
						( *pFilename, Charset::encodingUTF8,
											pfriInfo->pszFilename ) ;
					listDirs.Add( pFilename ) ;
				}
			}
		}
	}
}

// デフォルトのパスワード設定
//////////////////////////////////////////////////////////////////////////////
void SGLArchiveFile::SetDefaultPassword( const wchar_t * pwszPassword )
{
	m_strDefPassword = pwszPassword ;
}

// ファイルインターフェースの複製
//////////////////////////////////////////////////////////////////////////////
SFileInterface * SGLArchiveFile::Duplicate( void ) const
{
	SSmartLock<SCriticalSection>	smartLock( (SCriticalSection*) &m_csSync ) ;
	//
	if ( IsFileWritingMode() )
	{
		return	new RefFile( (SGLArchiveFile*) this ) ;
	}
	if ( m_pFileBuffer != NULL )
	{
		return	new SSmartFile
					( (SFileOpener*) this, m_pFileBuffer->Duplicate() ) ;
	}
	else if ( (m_pInCRC32 != NULL) | (m_pOutCRC32 != NULL) )
	{
		return	new RefFile( (SGLArchiveFile*) this ) ;
	}
	return	new SSmartFile
				( (SFileOpener*) this, SChunkFile::Duplicate() ) ;
}

// ファイルから読み込み
//////////////////////////////////////////////////////////////////////////////
size_t SGLArchiveFile::Read( void * ptrBuf, size_t nBytes )
{
	SSmartLock<SCriticalSection>	smartLock( &m_csSync ) ;
	//
	if ( m_pFileBuffer != NULL )
	{
		return	m_pFileBuffer->Read( ptrBuf, nBytes ) ;
	}
	else if ( m_pInCRC32 != NULL )
	{
		return	m_pInCRC32->Read( ptrBuf, nBytes ) ;
	}
	return	SChunkFile::Read( ptrBuf, nBytes ) ;
}

// ファイルへ書き込み
//////////////////////////////////////////////////////////////////////////////
size_t SGLArchiveFile::Write( const void * ptrBuf, size_t nBytes )
{
	SSmartLock<SCriticalSection>	smartLock( &m_csSync ) ;
	//
	if ( m_pOutCRC32 != NULL )
	{
		ESLAssert( m_pfriFile != NULL ) ;
		size_t	nWrittenBytes = m_pOutCRC32->Write( ptrBuf, nBytes ) ;
		m_pfriFile->pfeEntry->nBytes += nWrittenBytes ;
		return	nWrittenBytes ;
	}
	return	SChunkFile::Write( ptrBuf, nBytes ) ;
}

// シーク可能か否か？
//////////////////////////////////////////////////////////////////////////////
bool SGLArchiveFile::IsSeekable( void ) const
{
	return	(m_pInCRC32 == NULL) & (m_pOutCRC32 == NULL) ;
}

// ファイル長の取得
//////////////////////////////////////////////////////////////////////////////
int64_t SGLArchiveFile::GetLength( void ) const
{
	SSmartLock<SCriticalSection>	smartLock( (SCriticalSection*) &m_csSync ) ;
	//
	if ( m_pfriFile != NULL )
	{
		return	m_pfriFile->pfeEntry->nBytes ;
	}
	return	SChunkFile::GetLength() ;
}

// ファイルポインタを移動
//////////////////////////////////////////////////////////////////////////////
int64_t SGLArchiveFile::Seek
	( int64_t posFile, SFileInterface::SeekOrigin seekFrom )
{
	SSmartLock<SCriticalSection>	smartLock( &m_csSync ) ;
	//
	if ( m_pFileBuffer != NULL )
	{
		return	m_pFileBuffer->Seek( posFile, seekFrom ) ;
	}
	else if ( (m_pInCRC32 != NULL) | (m_pOutCRC32 != NULL) )
	{
		return	-1 ;
	}
	return	SChunkFile::Seek( posFile, seekFrom ) ;
}

// ファイルポインタを取得
//////////////////////////////////////////////////////////////////////////////
int64_t SGLArchiveFile::GetPosition( void ) const
{
	SSmartLock<SCriticalSection>	smartLock( (SCriticalSection*) &m_csSync ) ;
	//
	if ( m_pFileBuffer != NULL )
	{
		return	m_pFileBuffer->GetPosition() ;
	}
	else if ( (m_pInCRC32 != NULL) | (m_pOutCRC32 != NULL) )
	{
		return	-1 ;
	}
	return	SChunkFile::GetPosition() ;
}

// ファイルの終端を現在の位置に設定する
//////////////////////////////////////////////////////////////////////////////
SError SGLArchiveFile::SetEndOfFile( void )
{
	return	errSuccess ;
}

