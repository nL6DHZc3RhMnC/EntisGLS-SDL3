
/*****************************************************************************
             Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
   Copyright (c) 2003-2007 Leshade Entis, Entis-soft. All rights reserved.
 *****************************************************************************/


#include <gls.h>


//////////////////////////////////////////////////////////////////////////////
// Win32 PE 形式ファイル解析クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( EXEImageFileObject, ESLFileObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
EXEImageFileObject::EXEImageFileObject( void )
{
	m_pImageFile = NULL ;
	m_hFileMapping = NULL ;
	m_pPEImage = NULL ;
	m_dwBytes = 0 ;
	//
	m_fildehdr = NULL ;
	m_opthdr = NULL ;
	m_sechdr = NULL ;
	m_prsrcsec = NULL ;
	//
	m_dwRsrcBasePos = 0 ;
	m_dwRsrcDataLen = 0 ;
	//
	m_pDstFile = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
EXEImageFileObject::~EXEImageFileObject( void )
{
	ClosePEImageFile( ) ;
}

// PE イメージファイルを読み込む
//////////////////////////////////////////////////////////////////////////////
ESLError EXEImageFileObject::ReadPEImageFile
	( ESLFileObject & file, bool fFileMapping )
{
	//
	// ファイルを読み込む
	//
	ClosePEImageFile( ) ;
	//
	ERawFile *	pRawFile = ESLTypeCast<ERawFile>( &file ) ;
	if ( pRawFile && fFileMapping )
	{
		m_dwBytes = file.GetLength( ) ;
		m_hFileMapping =
			::CreateFileMapping( *pRawFile, NULL, PAGE_READONLY, 0, 0, NULL ) ;
		if ( m_hFileMapping == NULL )
		{
			return	ESLErrorMsg
				( "ファイルマッピングオブジェクトの生成に失敗しました。" ) ;
		}
		m_pPEImage =
			(PBYTE) ::MapViewOfFile( m_hFileMapping, FILE_MAP_READ, 0, 0, 0 ) ;
		if ( m_pPEImage == NULL )
		{
			return	ESLErrorMsg( "ファイルマッピングに失敗しました。" ) ;
		}
	}
	else
	{
		m_dwBytes = file.GetLength( ) ;
		m_pPEImage = (PBYTE) ::eslHeapAllocate( NULL, m_dwBytes, 0 ) ;
		file.Read( m_pPEImage, m_dwBytes ) ;
	}
	//
	ESLError	err =
		ESLErrorMsg( "有効な PE イメージファイルではありません。" ) ;
	do
	{
		//
		// COFF ファイルヘッダを取得する
		//
		if ( m_dwBytes < 0x40 )
		{
			break ;
		}
		DWORD	dwPEAddress = *((DWORD*)(m_pPEImage + 0x3C)) ;
		if ( m_dwBytes < dwPEAddress + 4 + sizeof(IMAGE_OPTIONAL_HEADER32) )
		{
			break ;
		}
		if ( *((DWORD*)(m_pPEImage + dwPEAddress)) != *((DWORD*)"PE\0\0") )
		{
			break ;
		}
		m_fildehdr = (IMAGE_FILE_HEADER*) (m_pPEImage + dwPEAddress + 4) ;
		//
		// オプションヘッダを取得する
		//
		m_opthdr =
			(IMAGE_OPTIONAL_HEADER32*)
				(((PBYTE) m_fildehdr) + sizeof(IMAGE_FILE_HEADER)) ;
		//
		// セクションヘッダを取得する
		//
		DWORD	dwSecHdrAddr = dwPEAddress + 4 + sizeof(IMAGE_FILE_HEADER) ;
		dwSecHdrAddr += m_fildehdr->SizeOfOptionalHeader ;
		if ( m_dwBytes < dwSecHdrAddr
				+ sizeof(IMAGE_SECTION_HEADER) * m_fildehdr->NumberOfSections )
		{
			break ;
		}
		m_sechdr = (IMAGE_SECTION_HEADER*) (m_pPEImage + dwSecHdrAddr) ;
		m_prsrcsec = NULL ;
		//
		err = eslErrSuccess ;
		//
		// 各セクションをサーチして、".rsrc" セクションを取得する
		//
		for ( unsigned int i = 0; i < m_fildehdr->NumberOfSections; i ++ )
		{
			if ( m_dwBytes < m_sechdr[i].PointerToRawData
								+ m_sechdr[i].SizeOfRawData )
			{
				err = ESLErrorMsg( "セクションヘッダが不正です。" ) ;
				break ;
			}
			if ( *((UINT64*) m_sechdr[i].Name) == *((UINT64*) ".rsrc\0\0\0") )
			{
				m_prsrcsec = &(m_sechdr[i]) ;
			}
		}
	}
	while ( false ) ;
	//
	if ( err )
	{
		ClosePEImageFile( ) ;
	}
	//
	return	err ;
}

// PE イメージファイルを開く
//////////////////////////////////////////////////////////////////////////////
ESLError EXEImageFileObject::OpenPEImageFile( ESLFileObject & file )
{
	//
	// ファイルヘッダを読み込む
	//
	ClosePEImageFile( ) ;
	//
	ESLError	err =
		ESLErrorMsg( "有効な PE イメージファイルではありません。" ) ;
	m_pImageFile = &file ;
	SetAttribute( modeRead ) ;
	m_dwBytes = file.GetLength( ) ;
	do
	{
		//
		// COFF ファイルヘッダを取得する
		//
		EStreamBuffer	buf ;
		BYTE *	pbytBuf = (BYTE*) buf.PutBuffer( 0x40 ) ;
		if ( file.Read( pbytBuf, 0x40 ) < 0x40 )
		{
			break ;
		}
		DWORD	dwPEAddress = *((DWORD*)(pbytBuf + 0x3C)) ;
		if ( m_dwBytes < dwPEAddress + 4 + sizeof(IMAGE_OPTIONAL_HEADER32) )
		{
			break ;
		}
		//
		// ファイルヘッダを読み込む
		//
		IMAGE_FILE_HEADER	filehdr ;
		file.Seek( dwPEAddress, file.FromBegin ) ;
		file.Read( pbytBuf, 4 ) ;
		if ( *((DWORD*)pbytBuf) != *((DWORD*)"PE\0\0") )
		{
			break ;
		}
		if ( file.Read( &filehdr, sizeof(filehdr) ) < sizeof(filehdr) )
		{
			break ;
		}
		//
		// セクションヘッダをサーチして ".rsrc" セクションを取得する
		//
		unsigned int			i, j, k ;
		DWORD					dwSecHdrAddr, dwRsrcSecAddr = 0 ;
		IMAGE_SECTION_HEADER	sechdr ;
		dwSecHdrAddr =
			file.Seek( filehdr.SizeOfOptionalHeader, file.FromCurrent ) ;
		//
		for ( i = 0; i < filehdr.NumberOfSections; i ++ )
		{
			DWORD	dwFilePos = file.GetPosition( ) ;
			if ( file.Read( &sechdr, sizeof(sechdr) ) < sizeof(sechdr) )
			{
				break ;
			}
			if ( *((UINT64*) sechdr.Name) == *((UINT64*) ".rsrc\0\0\0") )
			{
				dwRsrcSecAddr = dwFilePos ;
				break ;
			}
		}
		if ( dwRsrcSecAddr == 0 )
		{
			break ;
		}
		//
		// リソースヘッダのサイズを取得する
		//		＞リソースタイプ列挙
		//
		DWORD	dwRsrcDataAddr = 0x7FFFFFFF ;
		RSRC_DIRECTORY_TABLE	rtdir ;
		file.Seek( sechdr.PointerToRawData, file.FromBegin ) ;
		if ( file.Read( &rtdir, sizeof(rtdir) ) < sizeof(rtdir) )
		{
			break ;
		}
		EStreamBuffer	bufRTEntry ;
		DWORD			dwTblBytes ;
		unsigned int	nTypeCount =
			rtdir.wNumberOfNameEntries + rtdir.wNumberOfIDEntries ;
		dwTblBytes = nTypeCount * sizeof(RSRC_DIRECTORY_ENTRY) ;
		RSRC_DIRECTORY_ENTRY *	prtentry =
			(RSRC_DIRECTORY_ENTRY*) bufRTEntry.PutBuffer( dwTblBytes ) ;
		if ( file.Read( prtentry, dwTblBytes ) < dwTblBytes )
		{
			break ;
		}
		for ( i = 0; i < nTypeCount; i ++ )
		{
			//
			// リソース識別子を列挙
			//
			RSRC_DIRECTORY_TABLE	riddir ;
			file.Seek
				( sechdr.PointerToRawData
					+ (prtentry[i].data.dwSubdirectoryRVA
										& 0x7FFFFFFF), file.FromBegin ) ;
			if ( file.Read( &riddir, sizeof(riddir) ) < sizeof(riddir) )
			{
				continue ;
			}
			EStreamBuffer	bufRIDEntry ;
			unsigned int	nIDCount =
				riddir.wNumberOfNameEntries + riddir.wNumberOfIDEntries ;
			dwTblBytes = nIDCount * sizeof(RSRC_DIRECTORY_ENTRY) ;
			RSRC_DIRECTORY_ENTRY *	pridentry =
				(RSRC_DIRECTORY_ENTRY*) bufRIDEntry.PutBuffer( dwTblBytes ) ;
			if ( file.Read( pridentry, dwTblBytes ) < dwTblBytes )
			{
				continue ;
			}
			for ( j = 0; j < nIDCount; j ++ )
			{
				//
				// リソース言語を列挙
				//
				RSRC_DIRECTORY_TABLE	rlngdir ;
				file.Seek
					( sechdr.PointerToRawData
						+ (pridentry[i].data.dwSubdirectoryRVA
											& 0x7FFFFFFF), file.FromBegin ) ;
				if ( file.Read( &rlngdir, sizeof(rlngdir) ) < sizeof(rlngdir) )
				{
					continue ;
				}
				EStreamBuffer	bufRLngEntry ;
				unsigned int	nLngCount =
					rlngdir.wNumberOfNameEntries + rlngdir.wNumberOfIDEntries ;
				dwTblBytes = nLngCount * sizeof(RSRC_DIRECTORY_ENTRY) ;
				RSRC_DIRECTORY_ENTRY *	prlngentry =
					(RSRC_DIRECTORY_ENTRY*) bufRLngEntry.PutBuffer( dwTblBytes ) ;
				if ( file.Read( prlngentry, dwTblBytes ) < dwTblBytes )
				{
					continue ;
				}
				for ( k = 0; k < nLngCount; k ++ )
				{
					//
					// リソースデータの格納位置を取得
					//
					RSRC_DATA_ENTRY	rdata ;
					file.Seek
						( sechdr.PointerToRawData
							+ (prlngentry[i].data.dwSubdirectoryRVA
											& 0x7FFFFFFF), file.FromBegin ) ;
					if ( file.Read( &rdata, sizeof(rdata) ) < sizeof(rdata) )
					{
						continue ;
					}
					DWORD	dwDataAddress =
						sechdr.PointerToRawData
							+ rdata.dwDataRVA - sechdr.VirtualAddress ;
					if ( dwDataAddress < dwRsrcDataAddr )
					{
						dwRsrcDataAddr = dwDataAddress ;
					}
				}
			}
		}
		if ( dwRsrcDataAddr == 0x7FFFFFFF )
		{
			break ;
		}
		//
		// ファイルの先頭部分だけ読み込んで各ヘッダ情報へのポインタを確定する
		//
		m_pPEImage = (PBYTE) ::eslHeapAllocate( NULL, dwRsrcDataAddr, 0 ) ;
		file.Seek( 0, file.FromBegin ) ;
		file.Read( m_pPEImage, dwRsrcDataAddr ) ;
		//
		m_fildehdr = (IMAGE_FILE_HEADER*) (m_pPEImage + dwPEAddress + 4) ;
		m_opthdr =
			(IMAGE_OPTIONAL_HEADER32*)
				(((PBYTE) m_fildehdr) + sizeof(IMAGE_FILE_HEADER)) ;
		m_sechdr = (IMAGE_SECTION_HEADER*) (m_pPEImage + dwSecHdrAddr) ;
		m_prsrcsec = (IMAGE_SECTION_HEADER*) (m_pPEImage + dwRsrcSecAddr) ;
		//
		err = eslErrSuccess ;
		break ;
	}
	while ( false ) ;
	//
	return	err ;
}

// PE イメージファイルを閉じる
//////////////////////////////////////////////////////////////////////////////
void EXEImageFileObject::ClosePEImageFile( void )
{
	EndWritingPEImage( ) ;
	//
	if ( m_hFileMapping != NULL )
	{
		if ( m_pPEImage != NULL )
		{
			::UnmapViewOfFile( m_pPEImage ) ;
			m_pPEImage = NULL ;
			m_dwBytes = 0 ;
		}
		::CloseHandle( m_hFileMapping ) ;
		m_hFileMapping = NULL ;
	}
	else
	{
		if ( m_pPEImage != NULL )
		{
			::eslHeapFree( NULL, m_pPEImage ) ;
			m_pPEImage = NULL ;
			m_dwBytes = 0 ;
		}
	}
	m_pImageFile = NULL ;
	//
	m_fildehdr = NULL ;
	m_opthdr = NULL ;
	m_sechdr = NULL ;
	m_prsrcsec = NULL ;
	//
	m_dwRsrcBasePos = 0 ;
	m_dwRsrcDataLen = 0 ;
}

// 指定のリソースデータを検索する
//////////////////////////////////////////////////////////////////////////////
ESLError EXEImageFileObject::FindResourceData
	( EPtrBuffer & ptrbuf,
		LPCTSTR lpTypeID, LPCSTR lpResID, WORD wLanguage )
{
	//
	// リソースセクション取得
	//
	if ( m_prsrcsec == NULL )
	{
		return	ESLErrorMsg( "リソースセクションがありません。" ) ;
	}
	PBYTE	pbytRsrc = m_pPEImage + m_prsrcsec->PointerToRawData ;
	//
	try
	{
		RSRC_DIRECTORY_TABLE *	pdir ;
		RSRC_DIRECTORY_ENTRY *	pentry ;
		DWORD	i, dwCount ;
		EWideString	wstrTypeID ;
		EWideString	wstrResID ;
		//
		// リソースタイプ検索
		//
		pdir = (RSRC_DIRECTORY_TABLE*) pbytRsrc ;
		pentry = (RSRC_DIRECTORY_ENTRY*) (pdir + 1) ;
		dwCount = pdir->wNumberOfNameEntries + pdir->wNumberOfIDEntries ;
		if ( (DWORD) lpTypeID & 0xFFFF0000 )
		{
			wstrTypeID = lpTypeID ;
		}
		for ( i = 0; i < dwCount; i ++ )
		{
			if ( pentry[i].type.dwNameRVA & 0x80000000 )
			{
				DWORD		dwRVA = pentry[i].type.dwNameRVA & 0x7FFFFFFF ;
				WORD		wLength = *((WORD*)(pbytRsrc + dwRVA)) ;
				EWideString	wstrID
					( (const wchar_t *) (pbytRsrc + dwRVA + 2), wLength ) ;
				if ( wstrTypeID == wstrID )
				{
					break ;
				}
			}
			else
			{
				if ( pentry[i].type.dwIntegerID == (DWORD) lpTypeID )
				{
					break ;
				}
			}
		}
		if ( i >= dwCount )
		{
			return	ESLErrorMsg( "リソースタイプが見つかりません。" ) ;
		}
		//
		// リソース ID 検索
		//
		pdir = (RSRC_DIRECTORY_TABLE*)
				(pbytRsrc + (pentry[i].data.dwSubdirectoryRVA & 0x7FFFFFFF)) ;
		pentry = (RSRC_DIRECTORY_ENTRY*) (pdir + 1) ;
		dwCount = pdir->wNumberOfNameEntries + pdir->wNumberOfIDEntries ;
		if ( (DWORD) lpResID & 0xFFFF0000 )
		{
			wstrResID = lpResID ;
		}
		for ( i = 0; i < dwCount; i ++ )
		{
			if ( pentry[i].type.dwNameRVA & 0x80000000 )
			{
				DWORD		dwRVA = pentry[i].type.dwNameRVA & 0x7FFFFFFF ;
				WORD		wLength = *((WORD*)(pbytRsrc + dwRVA)) ;
				EWideString	wstrID
					( (const wchar_t *) (pbytRsrc + dwRVA + 2), wLength ) ;
				if ( wstrResID == wstrID )
				{
					break ;
				}
			}
			else
			{
				if ( pentry[i].type.dwIntegerID == (DWORD) lpResID )
				{
					break ;
				}
			}
		}
		if ( i >= dwCount )
		{
			return	ESLErrorMsg( "リソース識別子が見つかりません。" ) ;
		}
		//
		// 言語 ID 検索
		//
		pdir = (RSRC_DIRECTORY_TABLE*)
				(pbytRsrc + (pentry[i].data.dwSubdirectoryRVA & 0x7FFFFFFF)) ;
		pentry = (RSRC_DIRECTORY_ENTRY*) (pdir + 1) ;
		dwCount = pdir->wNumberOfNameEntries + pdir->wNumberOfIDEntries ;
		for ( i = 0; i < dwCount; i ++ )
		{
			if ( (wLanguage == 0) || (pentry->type.dwIntegerID == wLanguage) )
			{
				break ;
			}
		}
		if ( i >= dwCount )
		{
			return	ESLErrorMsg( "言語識別子が見つかりません。" ) ;
		}
		//
		// データエントリ取得
		//
		RSRC_DATA_ENTRY *	pdata =
			(RSRC_DATA_ENTRY*) (pbytRsrc + pentry[i].data.dwDataEntryRVA) ;
		void *	ptrData =
			pbytRsrc + (pdata->dwDataRVA - m_prsrcsec->VirtualAddress) ;
		if ( m_prsrcsec->SizeOfRawData
			< (pdata->dwDataRVA - m_prsrcsec->VirtualAddress) + pdata->dwSize )
		{
			return	ESLErrorMsg( "データエントリが不正です。" ) ;
		}
		ptrbuf = EPtrBuffer( ptrData, pdata->dwSize ) ;
	}
	catch ( ... )
	{
		return	ESLErrorMsg( "リソースデータが見つかりませんでした。" ) ;
	}
	//
	return	eslErrSuccess ;
}

// 指定のリソースデータを開く
//////////////////////////////////////////////////////////////////////////////
ESLError EXEImageFileObject::DescendResourceData
	( LPCTSTR lpTypeID, LPCSTR lpResID, WORD wLanguage )
{
	if ( m_pImageFile == NULL )
	{
		return	ESLErrorMsg
			( "OpenPEImageFile 関数でイメージファイルが開かれていません。" ) ;
	}
	EPtrBuffer	ptrbuf( NULL, 0 ) ;
	ESLError	err =
		FindResourceData( ptrbuf, lpTypeID, lpResID, wLanguage ) ;
	if ( err )
	{
		return	err ;
	}
	m_dwRsrcBasePos = (DWORD) ptrbuf.GetBuffer() - (DWORD) m_pPEImage ;
	m_dwRsrcDataLen = ptrbuf.GetLength( ) ;
	m_pImageFile->Seek( m_dwRsrcBasePos, ESLFileObject::FromBegin ) ;
	return	eslErrSuccess ;
}

// 現在開いているリソースデータを閉じる
//////////////////////////////////////////////////////////////////////////////
ESLError EXEImageFileObject::AscendResourceData( void )
{
	m_dwRsrcBasePos = 0 ;
	m_dwRsrcDataLen = 0 ;
	return	eslErrSuccess ;
}

// リソースを列挙する
//////////////////////////////////////////////////////////////////////////////
ESLError EXEImageFileObject::EnumResourceEntries
	( LPCTSTR lpTypeID, LPCSTR lpResID, WORD wLanguage )
{
	//
	// リソースセクション取得
	//
	if ( m_prsrcsec == NULL )
	{
		return	ESLErrorMsg( "リソースセクションがありません。" ) ;
	}
	PBYTE		pbytRsrc = m_pPEImage + m_prsrcsec->PointerToRawData ;
	ESLError	errResult = eslErrSuccess ;
	//
	try
	{
		RSRC_DIRECTORY_TABLE *	pdir ;
		RSRC_DIRECTORY_ENTRY *	pentry ;
		DWORD	i, dwCount ;
		EString	strTypeID ;
		//
		// リソースタイプを列挙
		//
		pdir = (RSRC_DIRECTORY_TABLE*) pbytRsrc ;
		pentry = (RSRC_DIRECTORY_ENTRY*) (pdir + 1) ;
		dwCount = pdir->wNumberOfNameEntries + pdir->wNumberOfIDEntries ;
		if ( (DWORD) lpTypeID & 0xFFFF0000 )
		{
			strTypeID = lpTypeID ;
		}
		for ( i = 0; i < dwCount; i ++ )
		{
			//
			// リソースタイプを比較
			//
			EString	strEnumID ;
			DWORD	dwEnumID ;
			LPCTSTR	lpEnumTypeID ;
			if ( pentry[i].type.dwNameRVA & 0x80000000 )
			{
				DWORD		dwRVA = pentry[i].type.dwNameRVA & 0x7FFFFFFF ;
				WORD		wLength = *((WORD*)(pbytRsrc + dwRVA)) ;
				strEnumID = EString
					( (const wchar_t *) (pbytRsrc + dwRVA + 2), wLength ) ;
				if ( (lpTypeID != NULL) && (strTypeID != strEnumID) )
				{
					continue ;
				}
				lpEnumTypeID = strEnumID ;
			}
			else
			{
				dwEnumID = pentry[i].type.dwIntegerID ;
				if ( (lpTypeID != NULL) && (dwEnumID != (DWORD) lpTypeID) )
				{
					continue ;
				}
				lpEnumTypeID = MAKEINTRESOURCE(dwEnumID) ;
			}
			//
			// リソース識別子を列挙
			//
			RSRC_DIRECTORY_TABLE *	pdirResIDs =
				(RSRC_DIRECTORY_TABLE*) (pbytRsrc
						+ (pentry[i].data.dwSubdirectoryRVA & 0x7FFFFFFF)) ;
			errResult = EnumResourceIDsEntries
				( pdirResIDs, lpEnumTypeID, lpResID, wLanguage ) ;
			if ( errResult )
			{
				break ;
			}
		}
	}
	catch ( ... )
	{
		return	ESLErrorMsg( "リソースデータが見つかりませんでした。" ) ;
	}
	return	errResult ;
}

// リソースを列挙する（リソースID、言語を列挙）
//////////////////////////////////////////////////////////////////////////////
ESLError EXEImageFileObject::EnumResourceIDsEntries
	( EXEImageFileObject::RSRC_DIRECTORY_TABLE * pdirResID,
			LPCTSTR lpTypeID, LPCSTR lpResID, WORD wLanguage )
{
	//
	// リソースセクション取得
	//
	if ( m_prsrcsec == NULL )
	{
		return	ESLErrorMsg( "リソースセクションがありません。" ) ;
	}
	PBYTE		pbytRsrc = m_pPEImage + m_prsrcsec->PointerToRawData ;
	ESLError	errResult = eslErrSuccess ;
	//
	try
	{
		RSRC_DIRECTORY_ENTRY *	pentry ;
		DWORD	i, dwCount ;
		EString	strResID ;
		//
		// リソース識別子を列挙
		//
		pentry = (RSRC_DIRECTORY_ENTRY*) (pdirResID + 1) ;
		dwCount = pdirResID->wNumberOfNameEntries
					+ pdirResID->wNumberOfIDEntries ;
		if ( (DWORD) lpResID & 0xFFFF0000 )
		{
			strResID = lpResID ;
		}
		for ( i = 0; i < dwCount; i ++ )
		{
			//
			// リソース識別子を比較
			//
			EString	strEnumID ;
			DWORD	dwEnumID ;
			LPCTSTR	lpEnumTypeID ;
			if ( pentry[i].type.dwNameRVA & 0x80000000 )
			{
				DWORD		dwRVA = pentry[i].type.dwNameRVA & 0x7FFFFFFF ;
				WORD		wLength = *((WORD*)(pbytRsrc + dwRVA)) ;
				strEnumID = EString
					( (const wchar_t *) (pbytRsrc + dwRVA + 2), wLength ) ;
				if ( (lpResID != NULL) && (strResID != strEnumID) )
				{
					continue ;
				}
				lpEnumTypeID = strEnumID ;
			}
			else
			{
				dwEnumID = pentry[i].type.dwIntegerID ;
				if ( (lpResID != NULL) && (dwEnumID != (DWORD) lpResID) )
				{
					continue ;
				}
				lpEnumTypeID = MAKEINTRESOURCE(dwEnumID) ;
			}
			//
			// リソース言語を列挙
			//
			RSRC_DIRECTORY_TABLE *	pdirLanguage =
				(RSRC_DIRECTORY_TABLE*) (pbytRsrc
						+ (pentry[i].data.dwSubdirectoryRVA & 0x7FFFFFFF)) ;
			errResult = EnumResourceLangEntries
				( pdirLanguage, lpTypeID, lpEnumTypeID, wLanguage ) ;
			if ( errResult )
			{
				break ;
			}
		}
	}
	catch ( ... )
	{
		return	ESLErrorMsg( "リソースデータが見つかりませんでした。" ) ;
	}
	return	errResult ;
}

// リソースを列挙する（言語を列挙）
//////////////////////////////////////////////////////////////////////////////
ESLError EXEImageFileObject::EnumResourceLangEntries
	( EXEImageFileObject::RSRC_DIRECTORY_TABLE * pdirLanguage,
			LPCTSTR lpTypeID, LPCSTR lpResID, WORD wLanguage )
{
	//
	// リソースセクション取得
	//
	if ( m_prsrcsec == NULL )
	{
		return	ESLErrorMsg( "リソースセクションがありません。" ) ;
	}
	PBYTE		pbytRsrc = m_pPEImage + m_prsrcsec->PointerToRawData ;
	ESLError	errResult = eslErrSuccess ;
	//
	try
	{
		RSRC_DIRECTORY_ENTRY *	pentry ;
		DWORD	i, dwCount ;
		//
		// リソース識別子を列挙
		//
		pentry = (RSRC_DIRECTORY_ENTRY*) (pdirLanguage + 1) ;
		dwCount = pdirLanguage->wNumberOfNameEntries
					+ pdirLanguage->wNumberOfIDEntries ;
		for ( i = 0; i < dwCount; i ++ )
		{
			//
			// リソース言語を比較
			//
			if ( (wLanguage != 0)
				&& (pentry[i].type.dwIntegerID != wLanguage) )
			{
				continue ;
			}
			//
			// リソースデータを取得
			//
			RSRC_DATA_ENTRY *	pdata =
				(RSRC_DATA_ENTRY*) (pbytRsrc + pentry[i].data.dwDataEntryRVA) ;
			DWORD	dwDataAddress =
				pdata->dwDataRVA - m_prsrcsec->VirtualAddress ;
			void *	ptrData = pbytRsrc + dwDataAddress ;
			if ( m_prsrcsec->SizeOfRawData < dwDataAddress + pdata->dwSize )
			{
				return	ESLErrorMsg( "データエントリが不正です。" ) ;
			}
			errResult = OnFindResourceEntry
				( lpTypeID, lpResID,
					(WORD) pentry[i].type.dwIntegerID,
						ptrData, pdata->dwSize ) ;
			if ( errResult )
			{
				break ;
			}
		}
	}
	catch ( ... )
	{
		return	ESLErrorMsg( "リソースデータが見つかりませんでした。" ) ;
	}
	return	errResult ;
}

// リソース列挙関数
//////////////////////////////////////////////////////////////////////////////
ESLError EXEImageFileObject::OnFindResourceEntry
	( LPCTSTR lpTypeID, LPCSTR lpResID,
		WORD wLanguage, void * ptrDataEntry, DWORD dwDataLength )
{
	return	eslErrSuccess ;
}

// リソースデータエントリを追加設定する
//////////////////////////////////////////////////////////////////////////////
ESLError EXEImageFileObject::AddResourceDataEntry
	( LPCTSTR lpTypeID, LPCSTR lpResID, WORD wLanguage, DWORD dwSize )
{
	//
	// データエントリ取得
	//
	RSRC_DATA_ENTRY *	prsde =
		FindResourceToWrite( lpTypeID, lpResID, wLanguage, true ) ;
	if ( prsde == NULL )
	{
		return	eslErrGeneral ;
	}
	//
	// データエントリ設定
	//
	prsde->dwDataRVA = 0 ;
	prsde->dwSize = dwSize ;
	prsde->dwCodepage = 0 ;
	prsde->dwReserved = 0 ;
	//
	return	eslErrSuccess ;
}

// PE イメージファイルの書き出しを開始する
//////////////////////////////////////////////////////////////////////////////
ESLError EXEImageFileObject::BeginWritingPEImage( ESLFileObject & file )
{
	//
	// 現在読み込んでいる PE 形式の有効性を検証する
	//
	if ( m_prsrcsec == NULL )
	{
		return	ESLErrorMsg( "リソースセクションがありません。" ) ;
	}
	DWORD	i, j, k ;
	for ( i = 0; i < m_fildehdr->NumberOfSections; i ++ )
	{
		if ( m_sechdr[i].PointerToRawData > m_prsrcsec->PointerToRawData )
		{
			return	ESLErrorMsg
				( "リソースセクションの位置がファイルの末端にありません。" ) ;
		}
	}
	//
	// リソースデータ以外のイメージを書き出す
	//
	if ( file.Write( m_pPEImage, m_prsrcsec->PointerToRawData )
									< m_prsrcsec->PointerToRawData )
	{
		return	ESLErrorMsg( "PE イメージの書き出し中に失敗しました。" ) ;
	}
	//
	// リソースディレクトリテーブルのサイズを計算する
	//
	static const BYTE	bytDummy[0x10] =
	{
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
	} ;
	DWORD			dwDirTableSize = 0 ;
	DWORD			dwDataEntrySize = 0 ;
	DWORD			dwDataSize = 0 ;
	EStreamBuffer	bufNames ;
	RSRC_DIRECTORY_TABLE	rsdtRoot ;
	//
	::eslFillMemory( &rsdtRoot, 0, sizeof(rsdtRoot) ) ;
	dwDirTableSize +=
		sizeof(RSRC_DIRECTORY_TABLE)
			+ m_lstRsrcDir.GetSize() * sizeof(RSRC_DIRECTORY_ENTRY) ;
	//
	for ( i = 0; i < m_lstRsrcDir.GetSize(); i ++ )
	{
		ERsrcNameList *	pNameList = m_lstRsrcDir.GetAt( i ) ;
		ESLAssert( pNameList != NULL ) ;
		if ( pNameList == NULL )
			continue ;
		//
		::eslFillMemory
			( &(pNameList->m_rsdt), 0, sizeof(RSRC_DIRECTORY_TABLE) ) ;
		//
		pNameList->m_rsde.data.
			dwSubdirectoryRVA = dwDirTableSize | 0x80000000 ;
		dwDirTableSize +=
			sizeof(RSRC_DIRECTORY_TABLE)
				+ pNameList->m_lstEntries.GetSize()
						* sizeof(RSRC_DIRECTORY_ENTRY) ;
		//
		if ( pNameList->m_wstrTypeID.GetAt(0) != L'#' )
		{
			rsdtRoot.wNumberOfNameEntries ++ ;
			pNameList->m_rsde.type.dwNameRVA =
					bufNames.GetLength() | 0x80000000 ;
			*((WORD*) bufNames.PutBuffer(2)) =
					pNameList->m_wstrTypeID.GetLength( ) ;
			bufNames.Flush( 2 ) ;
			bufNames.Write
				( pNameList->m_wstrTypeID.CharPtr(),
					pNameList->m_wstrTypeID.GetLength() * sizeof(wchar_t) ) ;
		}
		else
		{
			rsdtRoot.wNumberOfIDEntries ++ ;
			EStreamWideString	sws( pNameList->m_wstrTypeID.Middle(1) ) ;
			pNameList->m_rsde.type.dwIntegerID = sws.GetInteger( ) ;
		}
		//
		for ( j = 0; j < pNameList->m_lstEntries.GetSize(); j ++ )
		{
			ERsrcDataList *	pDataList = pNameList->m_lstEntries.GetAt( j ) ;
			ESLAssert( pDataList != NULL ) ;
			if ( pDataList == NULL )
				continue ;
			//
			::eslFillMemory
				( &(pDataList->m_rsdt), 0, sizeof(RSRC_DIRECTORY_TABLE) ) ;
			//
			pDataList->m_rsde.data.
				dwSubdirectoryRVA = dwDirTableSize | 0x80000000 ;
			dwDirTableSize +=
				sizeof(RSRC_DIRECTORY_TABLE)
					+ pDataList->m_lstEntries.GetSize()
							* sizeof(RSRC_DIRECTORY_ENTRY) ;
			//
			if ( pDataList->m_wstrResID.GetAt(0) != L'#' )
			{
				pNameList->m_rsdt.wNumberOfNameEntries ++ ;
				pDataList->m_rsde.type.dwNameRVA =
						bufNames.GetLength() | 0x80000000 ;
				*((WORD*) bufNames.PutBuffer(2)) =
						pDataList->m_wstrResID.GetLength( ) ;
				bufNames.Flush( 2 ) ;
				bufNames.Write
					( pDataList->m_wstrResID.CharPtr(),
						pDataList->m_wstrResID.GetLength() * sizeof(wchar_t) ) ;
			}
			else
			{
				pNameList->m_rsdt.wNumberOfIDEntries ++ ;
				EStreamWideString	sws( pDataList->m_wstrResID.Middle(1) ) ;
				pDataList->m_rsde.type.dwIntegerID = sws.GetInteger( ) ;
			}
			//
			for ( k = 0; k < pDataList->m_lstEntries.GetSize(); k ++ )
			{
				ERsrcDataEntry *	pDataEntry =
						pDataList->m_lstEntries.GetAt( k ) ;
				ESLAssert( pDataEntry != NULL ) ;
				if ( pDataEntry == NULL )
					continue ;
				//
				pDataList->m_rsdt.wNumberOfIDEntries ++ ;
				//
				pDataEntry->m_dwRVA = dwDataEntrySize ;
				dwDataEntrySize += sizeof(RSRC_DATA_ENTRY) ;
				//
				pDataEntry->m_rsde.dwDataRVA = dwDataSize ;
				dwDataSize += (pDataEntry->m_rsde.dwSize + 0x07) & ~0x07 ;
			}
		}
	}
	//
	// オフセットアドレスを計算する
	//
	DWORD	dwDataEntryOffset = (dwDirTableSize + 0x07) & ~0x07 ;
	DWORD	dwNameListOffset =
		dwDataEntryOffset + ((dwDataEntrySize + 0x0F) & ~0x0F) ;
	DWORD	dwDataOffset =
		dwNameListOffset + ((bufNames.GetLength() + 0x0F) & ~0x0F) ;
	//
	// ディレクトリテーブルを書き出す
	//
	file.Write( &rsdtRoot, sizeof(rsdtRoot) ) ;
	m_lstRsrcDir.TrimEmpty( ) ;
	for ( i = 1; i < m_lstRsrcDir.GetSize(); i ++ )
	{
		int		p = i - 1, q = i - 1 ;
		SDWORD	dwID1 = m_lstRsrcDir.GetAt(p)->m_rsde.type.dwIntegerID ;
		dwID1 ^= (dwID1 >> 31) ;
		for ( j = i; j < m_lstRsrcDir.GetSize(); j ++ )
		{
			SDWORD	dwID2 = m_lstRsrcDir.GetAt(j)->m_rsde.type.dwIntegerID ;
			dwID2 ^= (dwID2 >> 31) ;
			if ( dwID1 > dwID2 )
				q = j ;
		}
		m_lstRsrcDir.Swap( p, q ) ;
	}
	for ( i = 0; i < m_lstRsrcDir.GetSize(); i ++ )
	{
		ERsrcNameList *	pNameList = m_lstRsrcDir.GetAt( i ) ;
		file.Write( &(pNameList->m_rsde), sizeof(RSRC_DIRECTORY_ENTRY) ) ;
	}
	//
	for ( i = 0; i < m_lstRsrcDir.GetSize(); i ++ )
	{
		ERsrcNameList *	pNameList = m_lstRsrcDir.GetAt( i ) ;
		ESLAssert( pNameList != NULL ) ;
		if ( pNameList == NULL )
			continue ;
		//
		if ( pNameList->m_rsde.type.dwNameRVA & 0x80000000 )
		{
			pNameList->m_rsde.type.dwNameRVA += dwNameListOffset ;
		}
		file.Write( &(pNameList->m_rsdt), sizeof(RSRC_DIRECTORY_TABLE) ) ;
		//
		pNameList->m_lstEntries.TrimEmpty( ) ;
		for ( j = 1; j < pNameList->m_lstEntries.GetSize(); j ++ )
		{
			int		p = j - 1, q = j - 1 ;
			SDWORD	dwID1 =
				pNameList->m_lstEntries.GetAt(p)->m_rsde.type.dwIntegerID ;
			dwID1 ^= (dwID1 >> 31) ;
			for ( k = j; k < pNameList->m_lstEntries.GetSize(); k ++ )
			{
				SDWORD	dwID2 =
					pNameList->m_lstEntries.GetAt(k)->m_rsde.type.dwIntegerID ;
				dwID2 ^= (dwID2 >> 31) ;
				if ( dwID1 > dwID2 )
					q = k ;
			}
			pNameList->m_lstEntries.Swap( p, q ) ;
		}
		for ( j = 0; j < pNameList->m_lstEntries.GetSize(); j ++ )
		{
			ERsrcDataList *	pDataList = pNameList->m_lstEntries.GetAt( j ) ;
			ESLAssert( pDataList != NULL ) ;
			if ( pDataList == NULL )
				continue ;
			//
			if ( pDataList->m_rsde.type.dwNameRVA & 0x80000000 )
			{
				pDataList->m_rsde.type.dwNameRVA += dwNameListOffset ;
			}
			file.Write( &(pDataList->m_rsde), sizeof(RSRC_DIRECTORY_ENTRY) ) ;
		}
		//
		for ( j = 0; j < pNameList->m_lstEntries.GetSize(); j ++ )
		{
			ERsrcDataList *	pDataList = pNameList->m_lstEntries.GetAt( j ) ;
			ESLAssert( pDataList != NULL ) ;
			if ( pDataList == NULL )
				continue ;
			//
			file.Write( &(pDataList->m_rsdt), sizeof(RSRC_DIRECTORY_TABLE) ) ;
			for ( k = 0; k < pDataList->m_lstEntries.GetSize(); k ++ )
			{
				ERsrcDataEntry *	pDataEntry =
						pDataList->m_lstEntries.GetAt( k ) ;
				ESLAssert( pDataEntry != NULL ) ;
				if ( pDataEntry == NULL )
					continue ;
				//
				RSRC_DIRECTORY_ENTRY	rsde ;
				rsde.type.dwIntegerID = pDataEntry->m_dwLanguage ;
				rsde.data.dwDataEntryRVA =
					dwDataEntryOffset + pDataEntry->m_dwRVA ;
				file.Write( &rsde, sizeof(RSRC_DIRECTORY_ENTRY) ) ;
			}
		}
	}
	if ( dwDataEntryOffset != dwDirTableSize )
	{
		file.Write( bytDummy, dwDataEntryOffset - dwDirTableSize ) ;
	}
	//
	// データエントリテーブルを書き出す
	//
	for ( i = 0; i < m_lstRsrcDir.GetSize(); i ++ )
	{
		ERsrcNameList *	pNameList = m_lstRsrcDir.GetAt( i ) ;
		ESLAssert( pNameList != NULL ) ;
		if ( pNameList == NULL )
			continue ;
		//
		for ( j = 0; j < pNameList->m_lstEntries.GetSize(); j ++ )
		{
			ERsrcDataList *	pDataList = pNameList->m_lstEntries.GetAt( j ) ;
			ESLAssert( pDataList != NULL ) ;
			if ( pDataList == NULL )
				continue ;
			//
			for ( k = 0; k < pDataList->m_lstEntries.GetSize(); k ++ )
			{
				ERsrcDataEntry *	pDataEntry =
						pDataList->m_lstEntries.GetAt( k ) ;
				ESLAssert( pDataEntry != NULL ) ;
				if ( pDataEntry == NULL )
					continue ;
				//
				pDataEntry->m_rsde.dwDataRVA +=
							dwDataOffset + m_prsrcsec->VirtualAddress ;
				file.Write( &(pDataEntry->m_rsde), sizeof(RSRC_DATA_ENTRY) ) ;
			}
		}
	}
	if ( dwNameListOffset != dwDataEntryOffset + dwDataEntrySize )
	{
		file.Write( bytDummy, dwNameListOffset
								- (dwDataEntryOffset + dwDataEntrySize) ) ;
	}
	//
	// 名前テーブルを書き出す
	//
	EPtrBuffer	ptrbuf = bufNames.GetBuffer( ) ;
	if ( file.Write( ptrbuf, ptrbuf.GetLength() ) < ptrbuf.GetLength() )
	{
		return	ESLErrorMsg( "リソースデータの書き出しに失敗しました。" ) ;
	}
	bufNames.Release( 0 ) ;
	//
	if ( dwDataOffset != dwNameListOffset + bufNames.GetLength() )
	{
		file.Write( bytDummy, dwDataOffset
							- (dwNameListOffset + bufNames.GetLength()) ) ;
	}
	//
	// リソースセクションのデータサイズを更新する
	//
	IMAGE_OPTIONAL_HEADER32	opthdr = *m_opthdr ;
	IMAGE_SECTION_HEADER	rsrcsec = *m_prsrcsec ;
	rsrcsec.Misc.VirtualSize = dwDataOffset + dwDataSize ;
	rsrcsec.SizeOfRawData = (rsrcsec.Misc.VirtualSize + 0x1FF) & ~0x1FF ;
	opthdr.SizeOfImage += (rsrcsec.SizeOfRawData - m_prsrcsec->SizeOfRawData) ;
	file.Seek( rsrcsec.PointerToRawData
				+ rsrcsec.SizeOfRawData, file.FromBegin ) ;
	file.SetEndOfFile( ) ;
	//
	file.Seek( (DWORD) m_opthdr - (DWORD) m_pPEImage, file.FromBegin ) ;
	file.Write( &opthdr, m_fildehdr->SizeOfOptionalHeader ) ;
	file.Seek( (DWORD) m_prsrcsec - (DWORD) m_pPEImage, file.FromBegin ) ;
	file.Write( &rsrcsec, sizeof(rsrcsec) ) ;
	//
	m_pDstFile = &file ;
	//
	return	eslErrSuccess ;
}

// リソースデータを書き出す
//////////////////////////////////////////////////////////////////////////////
ESLError EXEImageFileObject::AddWriteResourceData
	( LPCTSTR lpTypeID, LPCSTR lpResID, WORD wLanguage,
		const void * ptrBuf, unsigned long int nBytes )
{
	if ( m_pDstFile == NULL )
	{
		return	ESLErrorMsg
			( "BeginWritingPEImage 関数が呼び出されていません。" ) ;
	}
	RSRC_DATA_ENTRY *	prsde =
		FindResourceToWrite( lpTypeID, lpResID, wLanguage, false ) ;
	if ( prsde == NULL )
	{
		return	ESLErrorMsg( "リソースデータエントリが見つかりません。" ) ;
	}
	m_pDstFile->Seek
		( prsde->dwDataRVA - m_prsrcsec->VirtualAddress
			+ m_prsrcsec->PointerToRawData, ESLFileObject::FromBegin ) ;
	if ( nBytes > prsde->dwSize )
	{
		nBytes = prsde->dwSize ;
	}
	if ( m_pDstFile->Write( ptrBuf, nBytes ) < nBytes )
	{
		return	ESLErrorMsg( "ファイルへの書き出しに失敗しました。" ) ;
	}
	return	eslErrSuccess ;
}

// リソースデータを書き出す
//////////////////////////////////////////////////////////////////////////////
ESLError EXEImageFileObject::AddWriteResourceFile
	( LPCTSTR lpTypeID, LPCSTR lpResID, WORD wLanguage, ESLFileObject & file )
{
	if ( m_pDstFile == NULL )
	{
		return	ESLErrorMsg
			( "BeginWritingPEImage 関数が呼び出されていません。" ) ;
	}
	RSRC_DATA_ENTRY *	prsde =
		FindResourceToWrite( lpTypeID, lpResID, wLanguage, false ) ;
	if ( prsde == NULL )
	{
		return	ESLErrorMsg( "リソースデータエントリが見つかりません。" ) ;
	}
	m_pDstFile->Seek
		( prsde->dwDataRVA - m_prsrcsec->VirtualAddress
			+ m_prsrcsec->PointerToRawData, ESLFileObject::FromBegin ) ;
	//
	EStreamBuffer	buf ;
	DWORD	dwDeltaBytes = 0x1000 ;
	void *	ptrBuf = buf.PutBuffer( dwDeltaBytes ) ;
	DWORD	dwWrittenBytes = 0 ;
	while ( dwWrittenBytes < prsde->dwSize )
	{
		DWORD	dwReadBytes = file.Read( ptrBuf, dwDeltaBytes ) ;
		DWORD	dwWriteBytes = dwReadBytes ;
		if ( dwWriteBytes > prsde->dwSize - dwWrittenBytes )
		{
			dwWriteBytes = prsde->dwSize - dwWrittenBytes ;
		}
		if ( m_pDstFile->Write( ptrBuf, dwWriteBytes ) < dwWriteBytes )
		{
			return	ESLErrorMsg( "ファイルへの書き出しに失敗しました。" ) ;
		}
		dwWrittenBytes += dwWriteBytes ;
		if ( dwReadBytes < dwDeltaBytes )
		{
			break ;
		}
	}
	return	eslErrSuccess ;
}

// PE イメージファイルの書き出しを完了する
//////////////////////////////////////////////////////////////////////////////
ESLError EXEImageFileObject::EndWritingPEImage( void )
{
	m_lstRsrcDir.RemoveAll( ) ;
	m_pDstFile = NULL ;
	return	eslErrSuccess ;
}

// リソースデータ書き出しのためのリソースの情報を取得する
//////////////////////////////////////////////////////////////////////////////
EXEImageFileObject::RSRC_DATA_ENTRY *
	EXEImageFileObject::FindResourceToWrite
		( LPCTSTR lpTypeID, LPCSTR lpResID, WORD wLanguage, bool fCreate )
{
	//
	// リソース識別子を文字列化する
	//
	EWideString	wstrTypeID, wstrResID ;
	if ( (DWORD) lpTypeID & 0xFFFF0000 )
	{
		wstrTypeID = lpTypeID ;
	}
	else
	{
		wstrTypeID = L"#" + EWideString( (int) lpTypeID ) ;
	}
	if ( (DWORD) lpResID & 0xFFFF0000 )
	{
		wstrResID = lpResID ;
	}
	else
	{
		wstrResID = L"#" + EWideString( (int) lpResID ) ;
	}
	//
	// リソースの種別検索
	//
	ERsrcNameList *		pNameList = NULL ;
	ERsrcDataList *		pDataList = NULL ;
	ERsrcDataEntry *	pDataEntry = NULL ;
	unsigned int		i, nCount ;
	//
	nCount = m_lstRsrcDir.GetSize( ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		ERsrcNameList *	pList = m_lstRsrcDir.GetAt( i ) ;
		ESLAssert( pList != NULL ) ;
		if ( pList == NULL )
			continue ;
		//
		if ( pList->m_wstrTypeID == wstrTypeID )
		{
			pNameList = pList ;
			break ;
		}
	}
	if ( pNameList == NULL )
	{
		if ( fCreate )
		{
			pNameList = new ERsrcNameList ;
			pNameList->m_dwTypeID = (DWORD) lpTypeID ;
			pNameList->m_wstrTypeID = wstrTypeID ;
			//
			for ( i = 0; i < nCount; i ++ )
			{
				ERsrcNameList *	pList = m_lstRsrcDir.GetAt( i ) ;
				ESLAssert( pList != NULL ) ;
				if ( pList == NULL )
					continue ;
				if ( (DWORD) lpTypeID & 0xFFFF0000 )
				{
					if ( pList->m_wstrTypeID.CompareNoCase( wstrTypeID ) < 0 )
						break ;
				}
				else
				{
					if ( pList->m_dwTypeID > (DWORD) lpTypeID )
						break ;
				}
			}
			m_lstRsrcDir.InsertAt( i, pNameList ) ;
		}
		else
		{
			return	NULL ;
		}
	}
	//
	// リソースの識別子検索
	//
	nCount = pNameList->m_lstEntries.GetSize( ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		ERsrcDataList *	pList = pNameList->m_lstEntries.GetAt( i ) ;
		ESLAssert( pList != NULL ) ;
		if ( pList == NULL )
			continue ;
		//
		if ( pList->m_wstrResID == wstrResID )
		{
			pDataList = pList ;
			break ;
		}
	}
	if ( pDataList == NULL )
	{
		if ( fCreate )
		{
			pDataList = new ERsrcDataList ;
			pDataList->m_dwResID = (DWORD) lpResID ;
			pDataList->m_wstrResID = wstrResID ;
			for ( i = 0; i < nCount; i ++ )
			{
				ERsrcDataList *	pList = pNameList->m_lstEntries.GetAt( i ) ;
				ESLAssert( pList != NULL ) ;
				if ( pList == NULL )
					continue ;
				if ( (DWORD) lpResID & 0xFFFF0000 )
				{
					if ( pList->m_wstrResID.CompareNoCase( wstrResID ) < 0 )
						break ;
				}
				else
				{
					if ( pList->m_dwResID > (DWORD) lpResID )
						break ;
				}
			}
			pNameList->m_lstEntries.InsertAt( i, pDataList ) ;
		}
		else
		{
			return	NULL ;
		}
	}
	//
	// 言語識別子の検索
	//
	nCount = pDataList->m_lstEntries.GetSize( ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		ERsrcDataEntry *	pEntry = pDataList->m_lstEntries.GetAt( i ) ;
		ESLAssert( pEntry != NULL ) ;
		if ( pEntry == NULL )
			continue ;
		//
		if ( pEntry->m_dwLanguage == wLanguage )
		{
			pDataEntry = pEntry ;
			break ;
		}
	}
	if ( pDataEntry == NULL )
	{
		if ( fCreate )
		{
			pDataEntry = new ERsrcDataEntry ;
			pDataEntry->m_dwLanguage = wLanguage ;
			for ( i = 0; i < nCount; i ++ )
			{
				ERsrcDataEntry *	pEntry = pDataList->m_lstEntries.GetAt( i ) ;
				ESLAssert( pEntry != NULL ) ;
				if ( pEntry == NULL )
					continue ;
				if ( pEntry->m_dwLanguage > wLanguage )
					break ;
			}
			pDataList->m_lstEntries.InsertAt( i, pDataEntry ) ;
		}
		else
		{
			return	NULL ;
		}
	}
	//
	return	&(pDataEntry->m_rsde) ;
}

// ファイルオブジェクトを複製する
//////////////////////////////////////////////////////////////////////////////
ESLFileObject * EXEImageFileObject::Duplicate( void ) const
{
	return	NULL ;
}

// ファイルから読み込む
//////////////////////////////////////////////////////////////////////////////
unsigned long int EXEImageFileObject::Read
	( void * ptrBuffer, unsigned long int nBytes )
{
	if ( (m_pImageFile == NULL)
		|| ((m_dwRsrcBasePos == 0) && (m_dwRsrcDataLen == 0)) )
	{
		return	0 ;
	}
	DWORD	dwPos = m_pImageFile->GetPosition( ) ;
	if ( (dwPos - m_dwRsrcBasePos) + nBytes > m_dwRsrcDataLen )
	{
		nBytes = m_dwRsrcDataLen - (dwPos - m_dwRsrcBasePos) ;
	}
	return	m_pImageFile->Read( ptrBuffer, nBytes ) ;
}

// ファイルへ書き出す
//////////////////////////////////////////////////////////////////////////////
unsigned long int EXEImageFileObject::Write
	( const void * ptrBuffer, unsigned long int nBytes )
{
	return	0 ;
}

// ファイルの長さを取得
//////////////////////////////////////////////////////////////////////////////
unsigned long int EXEImageFileObject::GetLength( void ) const
{
	return	m_dwRsrcDataLen ;
}

// ファイルポインタを移動
//////////////////////////////////////////////////////////////////////////////
unsigned long int EXEImageFileObject::Seek
	( long int nOffsetPos, ESLFileObject::SeekOrigin fSeekFrom )
{
	if ( (m_pImageFile == NULL)
		|| ((m_dwRsrcBasePos == 0) && (m_dwRsrcDataLen == 0)) )
	{
		return	0 ;
	}
	switch ( fSeekFrom )
	{
	case	FromCurrent:
		nOffsetPos += GetPosition( ) ;
		break ;
	case	FromEnd:
		nOffsetPos += m_dwRsrcDataLen ;
		break ;
	}
	if ( nOffsetPos < 0 )
	{
		nOffsetPos = 0 ;
	}
	else if ( (DWORD) nOffsetPos > m_dwRsrcDataLen )
	{
		nOffsetPos = m_dwRsrcDataLen ;
	}
	return	m_pImageFile->Seek
			( m_dwRsrcBasePos + nOffsetPos, FromBegin ) - m_dwRsrcBasePos ;
}

// ファイルポインタを取得
//////////////////////////////////////////////////////////////////////////////
unsigned long int EXEImageFileObject::GetPosition( void ) const
{
	if ( (m_pImageFile == NULL)
		|| ((m_dwRsrcBasePos == 0) && (m_dwRsrcDataLen == 0)) )
	{
		return	0 ;
	}
	return	m_pImageFile->GetPosition() - m_dwRsrcBasePos ;
}


//////////////////////////////////////////////////////////////////////////////
// アイコン・カーソルファイル解析クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( EWin32IconFile, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
EWin32IconFile::EWin32IconFile( void )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
EWin32IconFile::~EWin32IconFile( void )
{
}

// アイコンファイルを読み込む
//////////////////////////////////////////////////////////////////////////////
ESLError EWin32IconFile::ReadIconFile( ESLFileObject & file )
{
	//
	// 現在の内容を消去する
	//
	DeleteContents( ) ;
	//
	// ファイルヘッダを読み込む
	//
	UINT64	nPos = file.GetLargePosition( ) ;
	if ( file.Read( &m_filehdr, sizeof(m_filehdr) ) < sizeof(m_filehdr) )
	{
		return	ESLErrorMsg( "ファイルヘッダを読み込めませんでした。" ) ;
	}
	if ( m_filehdr.wSignature != 0 )
	{
		if ( m_filehdr.wSignature == 'MB' )
		{
			file.SeekLarge( nPos, file.FromBegin ) ;
			return	ReadBitmapFile( file ) ;
		}
		return	ESLErrorMsg( "アイコンファイルではありません。" ) ;
	}
	//
	// アイコンエントリを読み込む
	//
	EStreamBuffer	bufEntryTable ;
	DWORD	dwEntryBytes = m_filehdr.wImageCount * sizeof(ICON_ENTRY) ;
	ICON_ENTRY *	pIconEntries =
		(ICON_ENTRY*) bufEntryTable.PutBuffer( dwEntryBytes ) ;
	if ( file.Read( pIconEntries, dwEntryBytes ) < dwEntryBytes )
	{
		return	ESLErrorMsg
			( "アイコンエントリテーブルを読み込めませんでした。" ) ;
	}
	//
	// 画像データを読み込む
	//
	for ( unsigned int i = 0; i < m_filehdr.wImageCount; i ++ )
	{
		EStreamBuffer	buf ;
		BITMAPINFOHEADER *	pbmih =
			(BITMAPINFOHEADER*) buf.PutBuffer( pIconEntries[i].dwSize ) ;
		file.Seek( pIconEntries[i].dwAddress, file.FromBegin ) ;
		if ( file.Read( pbmih,
				pIconEntries[i].dwSize ) < pIconEntries[i].dwSize )
		{
			return	ESLErrorMsg( "画像データの読み込みに失敗しました。" ) ;
		}
		ESLError	err =
			AddImage( pbmih, pIconEntries[i].dwSize, &pIconEntries[i] ) ;
		if ( err )
		{
			return	err ;
		}
	}
	//
	return	eslErrSuccess ;
}

ESLError EWin32IconFile::ReadBitmapFile( ESLFileObject & file )
{
	//
	// 現在の内容を消去する
	//
	DeleteContents( ) ;
	//
	// ファイルヘッダを読み込む
	//
	BITMAPFILEHEADER	bmfh ;
	BITMAPINFOHEADER	bmih ;
	if ( file.Read( &bmfh, sizeof(bmfh) ) < sizeof(bmfh) )
	{
		return	ESLErrorMsg( "ファイルヘッダの読み込みに失敗しました。" ) ;
	}
	if ( (bmfh.bfType != 'MB') || (bmfh.bfReserved1 != 0)
		|| (bmfh.bfReserved2 != 0)
		|| (bmfh.bfOffBits < sizeof(bmfh) + sizeof(bmih)) )
	{
		return	ESLErrorMsg( "ビットマップファイルではありません。" ) ;
	}
	//
	// ビットマップヘッダを読み込む
	//
	EStreamBuffer	buf ;
	if ( file.Read( &bmih, sizeof(bmih) ) < sizeof(bmih) )
	{
		return	ESLErrorMsg( "ビットマップヘッダの読み込みに失敗しました。" ) ;
	}
	if ( (bmih.biSize != sizeof(BITMAPINFOHEADER))
					|| (bmih.biCompression != BI_RGB) )
	{
		return	ESLErrorMsg( "有効なビットマップではありません。" ) ;
	}
	bmih.biHeight <<= 1 ;
	buf.Write( &bmih, sizeof(bmih) ) ;
	bmih.biHeight >>= 1 ;
	//
	// パレットテーブル読み込み
	//
	int	nPalLength = 0 ;
	if ( bmih.biBitCount <= 8 )
	{
		nPalLength = 1 << bmih.biBitCount ;
		if ( bmih.biClrUsed != 0 )
		{
			nPalLength = bmih.biClrUsed ;
		}
		DWORD	dwPalSize = sizeof(RGBQUAD) * nPalLength ;
		if ( file.Read( buf.PutBuffer(dwPalSize), dwPalSize ) < dwPalSize )
		{
			return	ESLErrorMsg( "パレットテーブルの読み込みに失敗しました。" ) ;
		}
		buf.Flush( dwPalSize ) ;
	}
	//
	// ビットマップ読み込み
	//
	DWORD	dwBitmapSize = bmih.biSizeImage ;
	if ( dwBitmapSize == 0 )
	{
		DWORD	dwLineBytes =
			((bmih.biWidth * bmih.biBitCount + 0x1F) & ~0x1F) >> 3 ;
		dwBitmapSize = dwLineBytes * bmih.biHeight ;
	}
	if ( file.Read
		( buf.PutBuffer( dwBitmapSize ), dwBitmapSize ) < dwBitmapSize )
	{
		return	ESLErrorMsg( "ビットマップの読み込みに失敗しました。" ) ;
	}
	buf.Flush( dwBitmapSize ) ;
	//
	// マスク追加
	//
	DWORD	dwMaskSize =
		(((bmih.biWidth + 0x1F) & ~0x1F) >> 3) * bmih.biHeight ;
	eslFillMemory( buf.PutBuffer( dwMaskSize ), 0, dwMaskSize ) ;
	buf.Flush( dwMaskSize ) ;
	//
	// アイコンエントリに追加
	//
	m_filehdr.wSignature = 0 ;
	m_filehdr.wFileType = 1 ;
	m_filehdr.wImageCount = 1 ;
	//
	EPtrBuffer	ptrbuf = buf.GetBuffer() ;
	return	AddImage
		( (BITMAPINFOHEADER*) ptrbuf.GetBuffer(), ptrbuf.GetLength() ) ;
}

// アイコンファイルを書き出す
//////////////////////////////////////////////////////////////////////////////
ESLError EWin32IconFile::WriteIconFile( ESLFileObject & file, WORD wType )
{
	//
	// ファイルヘッダを書き出す
	//
	if ( m_lstImages.GetSize() == 0 )
	{
		return	ESLErrorMsg( "アイコンデータがありません。" ) ;
	}
	FILE_HEADER	filehdr ;
	filehdr.wSignature = 0 ;
	filehdr.wFileType = wType ;
	filehdr.wImageCount = (WORD) m_lstImages.GetSize() ;
	if ( file.Write( &filehdr, sizeof(filehdr) ) < sizeof(filehdr) )
	{
		return	ESLErrorMsg( "ファイルヘッダの書き出しに失敗しました。" ) ;
	}
	//
	// ファイルエントリの書き出し
	//
	EImageEntry *	pImage ;
	DWORD	dwEntryBytes = filehdr.wImageCount * sizeof(ICON_ENTRY) ;
	DWORD	dwImageBase = sizeof(filehdr) + dwEntryBytes ;
	//
	unsigned int	i ;
	for ( i = 0; i < filehdr.wImageCount; i ++ )
	{
		pImage = m_lstImages.GetAt( i ) ;
		ESLAssert( pImage != NULL ) ;
		if ( pImage == NULL )
			continue ;
		//
		ICON_ENTRY *	pEntry = m_lstEntries.GetAt( i ) ;
		ESLAssert( pEntry != NULL ) ;
		if ( pEntry == NULL )
			continue ;
		//
		ICON_ENTRY	ieEntry = *pEntry ;
		ieEntry.dwSize = pImage->m_dwSize ;
		ieEntry.dwAddress = dwImageBase ;
		dwImageBase += ieEntry.dwSize ;
		//
		if ( file.Write( &ieEntry, sizeof(ieEntry) ) < sizeof(ieEntry) )
		{
			return	ESLErrorMsg
				( "アイコンエントリの書き出しに失敗しました。" ) ;
		}
	}
	//
	// 画像データの書き出し
	//
	for ( i = 0; i < filehdr.wImageCount; i ++ )
	{
		pImage = m_lstImages.GetAt( i ) ;
		ESLAssert( pImage != NULL ) ;
		if ( pImage == NULL )
			continue ;
		//
		if ( file.Write( pImage->m_pbmih,
							pImage->m_dwSize ) < pImage->m_dwSize )
		{
			return	ESLErrorMsg( "画像データの書き出しに失敗しました。" ) ;
		}
	}
	//
	return	eslErrSuccess ;
}

// データを初期化する
//////////////////////////////////////////////////////////////////////////////
void EWin32IconFile::DeleteContents( void )
{
	m_lstEntries.RemoveAll( ) ;
	m_lstImages.RemoveAll( ) ;
}

// アイコンエントリを取得する
//////////////////////////////////////////////////////////////////////////////
EWin32IconFile::ICON_ENTRY *
	EWin32IconFile::GetIconEntryAt( int nIndex ) const
{
	return	m_lstEntries.GetAt( nIndex ) ;
}

// 画像データを取得する
//////////////////////////////////////////////////////////////////////////////
BITMAPINFOHEADER * EWin32IconFile::GetImageAt( int nIndex ) const
{
	EImageEntry *	pImage = m_lstImages.GetAt( nIndex ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	return	pImage->m_pbmih ;
}

// 画像データサイズを取得する
//////////////////////////////////////////////////////////////////////////////
DWORD EWin32IconFile::GetImageSizeAt( int nIndex ) const
{
	EImageEntry *	pImage = m_lstImages.GetAt( nIndex ) ;
	if ( pImage == NULL )
	{
		return	0 ;
	}
	return	pImage->m_dwSize ;
}

// 画像データを追加する
//////////////////////////////////////////////////////////////////////////////
ESLError EWin32IconFile::AddImage
	( BITMAPINFOHEADER * pbmih, DWORD dwSize, const ICON_ENTRY * pIconEntry )
{
/*	DWORD	dwPaletteLength = 0 ;
	if ( pbmih->biBitCount <= 8 )
	{
		dwPaletteLength = 1 << pbmih->biBitCount ;
		if ( dwPaletteLength > pbmih->biClrUsed )
			dwPaletteLength = pbmih->biClrUsed ;
	}
	DWORD	dwImageBytes =
		(((pbmih->biWidth
			* pbmih->biBitCount + 0x1F) & ~0x1F) >> 3) * pbmih->biHeight ;
	DWORD	dwPackedBytes =
		sizeof(BITMAPINFOHEADER)
			+ dwPaletteLength * sizeof(RGBQUAD) + dwImageBytes ;
	//
	if ( dwSize < dwPackedBytes )
	{
		return	eslErrGeneral ;
	}
*/	//
	EImageEntry *	pImage = new EImageEntry ;
	pImage->Write( pbmih, dwSize ) ;
	EPtrBuffer	ptrbuf = pImage->GetBuffer( ) ;
	pImage->m_pbmih = (BITMAPINFOHEADER*) ptrbuf.GetBuffer( ) ;
	pImage->m_dwSize = ptrbuf.GetLength( ) ;
	m_lstImages.Add( pImage ) ;
	//
	ICON_ENTRY *	pEntry = new ICON_ENTRY ;
	if ( pIconEntry != NULL )
	{
		*pEntry = *pIconEntry ;
	}
	else
	{
		pEntry->bytWidth = (BYTE) pImage->m_pbmih->biWidth ;
		pEntry->bytHeight = (BYTE) pImage->m_pbmih->biHeight ;
		pEntry->bytColors = (BYTE) (1 << pImage->m_pbmih->biBitCount) ;
		pEntry->bytReserved = 0 ;
		pEntry->xHotSpot = 0 ;
		pEntry->yHotSpot = 0 ;
		pEntry->dwSize = pImage->m_dwSize ;
	}
	m_lstEntries.Add( pEntry ) ;
	//
	return	eslErrSuccess ;
}
