
/*****************************************************************************
				Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
    Copyright (c) 2009-2015 Leshade Entis, Entis-soft. All rights reserved.
 ****************************************************************************/


#include <gls.h>
#include <shlobj.h>

#if	_MSC_VER >= 1800
#include <VersionHelpers.h>
#endif

using namespace SSystem ;


//////////////////////////////////////////////////////////////////////////////
// ファイルリスト管理クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ECSSetup::EFileList, EDescription ) ;

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ECSSetup::EFileList::EFileList( void )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ECSSetup::EFileList::~EFileList( void )
{
}

// ディレクトリを追加する
//////////////////////////////////////////////////////////////////////////////
EDescription * ECSSetup::EFileList::AddDirectory
	( const wchar_t * pwszBasePath, int nCreated )
{
	//
	// 既存のディレクトリエントリ検索
	//
	EDescription *	pdscSetup = CreateContentTagAs( 0, L"setup" ) ;
	if ( pdscSetup == NULL )
	{
		return	NULL ;
	}
	EWideString	wstrBasePath = pwszBasePath ;
	EDescription *	pdscDir = pdscSetup ;
	int	i = 0 ;
	while ( i < pdscDir->GetContentTagCount() )
	{
		EDescription *	pdscTag = pdscDir->GetContentTagAt( i ++ ) ;
		ESLAssert( pdscTag != NULL ) ;
		if ( (pdscTag == NULL)
			|| (pdscTag->Tag() != L"directory") )
		{
			continue ;
		}
		EWideString	wstrDir = pdscTag->GetAttrString( L"path", NULL ) ;
		if ( wstrDir.CompareNoCase
			( wstrBasePath.Left( wstrDir.GetLength() ) ) )
		{
			continue ;
		}
		EWideString	wstrTempBasePath =
			wstrBasePath.Middle( wstrDir.GetLength() ) ;
		if ( (wstrTempBasePath.GetAt(0) == L'\\')
			|| (wstrTempBasePath.GetAt(0) == L'/') )
		{
			wstrBasePath = wstrTempBasePath.Middle( 1 ) ;
			if ( wstrBasePath.IsEmpty() )
			{
				if ( nCreated > 0 )
				{
					pdscTag->SetAttrInteger( L"created", nCreated ) ;
				}
				return	pdscTag ;		// 指定ディレクトリは既に存在している
			}
		}
		else if ( wstrTempBasePath.IsEmpty() )
		{
			if ( nCreated > 0 )
			{
				pdscTag->SetAttrInteger( L"created", nCreated ) ;
			}
			return	pdscTag ;		// 指定ディレクトリは既に存在している
		}
		else
		{
			continue ;
		}
		pdscDir = pdscTag ;
		i = 0 ;
	}
	//
	// ディレクトリエントリ追加
	//
	if ( wstrBasePath.Right(1) == L"\\" )
	{
		wstrBasePath =
			wstrBasePath.Left( wstrBasePath.GetLength() - 1 ) ;
	}
	EDescription *	pdscNewDir = new EDescription ;
	pdscDir->AddContentTag( pdscNewDir ) ;
	pdscNewDir->SetTag( L"directory" ) ;
	pdscNewDir->SetAttrString( L"path", wstrBasePath ) ;
	pdscNewDir->SetAttrInteger( L"created", nCreated ) ;
	return	pdscNewDir ;
}

// アーカイブディレクトリを追加する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::EFileList::AddArchiveDirectory
	( const wchar_t * pwszBasePath,
		const char * pszPassword, int nType )
{
	EDescription *	pdscDir = AddDirectory( pwszBasePath, 0 ) ;
	if ( pdscDir == NULL )
	{
		return	eslErrGeneral ;
	}
	if ( pszPassword != NULL )
	{
		pdscDir->SetAttrString( L"password", EWideString( pszPassword ) ) ;
	}
	pdscDir->SetAttrInteger( L"archive", 1 ) ;
	pdscDir->SetAttrInteger( L"encoding_type", nType ) ;
	return	eslErrSuccess ;
}

// ファイルを追加する
//////////////////////////////////////////////////////////////////////////////
EDescription * ECSSetup::EFileList::AddFile
	( const wchar_t * pwszBasePath, const wchar_t * pwszFileName )
{
	EDescription *	pdscDir = AddDirectory( pwszBasePath, 0 ) ;
	if ( pdscDir == NULL )
	{
		return	NULL ;
	}
	EWideString	wstrFilePath = pwszFileName ;
	EWideString	wstrAddFileName = GetFileNamePart( wstrFilePath ) ;
	for ( int i = 0; i < pdscDir->GetContentTagCount(); i ++ )
	{
		EDescription *	pdscTag = pdscDir->GetContentTagAt( i ) ;
		ESLAssert( pdscTag != NULL ) ;
		if ( (pdscTag == NULL) || (pdscTag->Tag() != L"file") )
		{
			continue ;
		}
		wstrFilePath = pdscTag->GetAttrString( L"path", NULL ) ;
		if ( !wstrAddFileName.CompareNoCase( GetFileNamePart( wstrFilePath ) ) )
		{
			pdscTag->SetAttrString( L"path", pwszFileName ) ;
			return	pdscTag ;			// 一致するファイルが見つかった
		}
	}
	EDescription *	pdscFile = new EDescription ;
	pdscDir->AddContentTag( pdscFile ) ;
	pdscFile->SetTag( L"file" ) ;
	pdscFile->SetAttrString( L"path", pwszFileName ) ;
	return	pdscFile ;
}

// ファイル名部分を取得する
//////////////////////////////////////////////////////////////////////////////
const wchar_t *
	ECSSetup::EFileList::GetFileNamePart( const wchar_t * pwszFilePath )
{
	unsigned int	l = 0 ;
	if ( pwszFilePath == NULL )
	{
		return	NULL ;
	}
	for ( unsigned int i = 0; pwszFilePath[i]; i ++ )
	{
		wchar_t	c = pwszFilePath[i] ;
		if ( (c == L'\\') || (c == L'/') || (c == L'#') )
		{
			l = i + 1 ;
		}
	}
	return	pwszFilePath + l ;
}

// ディレクトリ部分を取得する
//////////////////////////////////////////////////////////////////////////////
EWideString ECSSetup::EFileList::GetFileDirectoryPart
	( const wchar_t * pwszFilePath )
{
	unsigned int	l = 0 ;
	if ( pwszFilePath == NULL )
	{
		return	NULL ;
	}
	for ( unsigned int i = 0; pwszFilePath[i]; i ++ )
	{
		wchar_t	c = pwszFilePath[i] ;
		if ( (c == L'\\') || (c == L'/') || (c == L'#') )
		{
			l = i + 1 ;
		}
	}
	return	EWideString( pwszFilePath, l ) ;
}

// アーカイブファイルを追加する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::EFileList::AddArchiveTree
	( const wchar_t * pwszBasePath,
		const wchar_t * pwszSrcArchive, const char * pszPassword )
{
	EDescription *	pdscDir = AddDirectory( pwszBasePath, 0 ) ;
	if ( pdscDir == NULL )
	{
		return	eslErrGeneral ;
	}
	ERawFile	file ;
	if ( file.Open
		( EString( pwszSrcArchive ), file.modeRead | file.shareRead ) )
	{
		return	eslErrGeneral ;
	}
	ERISAArchive	arcfile ;
	if ( arcfile.Open( &file ) )
	{
		return	eslErrGeneral ;
	}
	//
	AddArchiveSubTree
		( pdscDir, pwszBasePath, arcfile,
			EWideString( pwszSrcArchive ) + L"#", pszPassword ) ;
	//
	arcfile.Close( ) ;
	file.Close( ) ;
	return	eslErrSuccess ;
}

ESLError ECSSetup::EFileList::AddArchiveSubTree
	( EDescription * pdscDir, const wchar_t * pwszBasePath,
		ERISAArchive & arcfile,
		const wchar_t * pwszSrcFileBase, const char * pszPassword )
{
	EWideString	wstrSrcFileBase = pwszSrcFileBase ;
	if ( !wstrSrcFileBase.IsEmpty() )
	{
		wchar_t	wchLastSrcFileBase =
			wstrSrcFileBase.GetAt( wstrSrcFileBase.GetLength() - 1 ) ;
		if ( (wchLastSrcFileBase != L'\\')
			&& (wchLastSrcFileBase != L'/')
			&& (wchLastSrcFileBase != L'#') )
		{
			wstrSrcFileBase += L'\\' ;
		}
	}
	for ( unsigned int i = 0; i < arcfile.ReferFileEntries().GetSize(); i ++ )
	{
		ERISAArchive::FILE_INFO *
			pfiEntry = arcfile.ReferFileEntries().GetObjectAt( i ) ;
		if ( pfiEntry == NULL )
		{
			continue ;
		}
		if ( pfiEntry->dwAttribute & ERISAArchive::attrDirectory )
		{
			EWideString	wstrSubDir = pwszBasePath ;
			if ( wstrSubDir.Right(1) != L"\\" )
			{
				wstrSubDir += L'\\' ;
			}
			EWideString	wstrFileName =
				wstrSubDir.OffsetFilePath
					( EWideString( pfiEntry->ptrFileName ) ) ;
			//
			EDescription *	pdscSubDir = AddDirectory( wstrSubDir, 0 ) ;
			if ( pdscSubDir != NULL )
			{
				if ( !arcfile.DescendDirectory( pfiEntry->ptrFileName ) )
				{
					EWideString	wstrSrcFileBase = 
					AddArchiveSubTree
						( pdscSubDir, wstrSubDir, arcfile,
							wstrSrcFileBase + wstrFileName, pszPassword ) ;
					arcfile.AscendDirectory( ) ;
				}
			}
		}
		else
		{
			EDescription *	pdscFile =
				AddFile( pwszBasePath,
					wstrSrcFileBase + EWideString( pfiEntry->ptrFileName ) ) ;
			if ( (pdscFile != NULL) && (pszPassword != NULL) )
			{
				pdscFile->SetAttrString
					( L"password", EWideString( pszPassword ) ) ;
			}
		}
	}
	return	eslErrSuccess ;
}

// ディレクトリを追加する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::EFileList::AddDirectoryTree
	( const wchar_t * pwszBasePath,
		const wchar_t * pwszSrcDirectory )
{
	EDescription *	pdscDir = AddDirectory( pwszBasePath, 0 ) ;
	if ( pdscDir == NULL )
	{
		return	eslErrGeneral ;
	}
	EWideString	wstrPath = pwszSrcDirectory ;
	//
	WIN32_FIND_DATA	wfd ;
	HANDLE	hFind =
		::FindFirstFile
			( EString( wstrPath.OffsetFilePath( L"*.*" ) ), &wfd ) ;
	if ( hFind != INVALID_HANDLE_VALUE )
	{
		do
		{
			if ( wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
			{
				EWideString	wstrDirName = wfd.cFileName ;
				if ( (wstrDirName != L".") && (wstrDirName != L"..") )
				{
					EWideString	wstrSubDir =
						EWideString( pwszBasePath ).
							OffsetFilePath( wstrDirName ) ;
					//
					AddDirectoryTree
						( wstrSubDir, wstrPath.OffsetFilePath( wstrDirName ) ) ;
				}
			}
			else
			{
				AddFile
					( pwszBasePath,
						wstrPath.OffsetFilePath
							( EWideString( wfd.cFileName ) ) ) ;
			}
		}
		while ( ::FindNextFile( hFind, &wfd ) ) ;
		::FindClose( hFind ) ;
	}
	return	eslErrSuccess ;
}

// 容量を計算する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::EFileList::MeasureSize( UINT64 & nBytes )
{
	EDescription *	pdscSetup = GetContentTagAs( 0, L"setup" ) ;
	if ( pdscSetup == NULL )
	{
		return	eslErrGeneral ;
	}
	return	MeasureSizeDirectory( pdscSetup, nBytes ) ;
}

ESLError ECSSetup::EFileList::MeasureSizeDirectory
	( EDescription * pdscDir, UINT64 & nBytes )
{
	nBytes = 0 ;
	for ( int i = 0; i < pdscDir->GetContentTagCount(); i ++ )
	{
		EDescription *	pdscTag = pdscDir->GetContentTagAt( i ) ;
		if ( pdscTag == NULL )
		{
			continue ;
		}
		if ( pdscTag->Tag() == L"file" )
		{
			EWideString	wstrFilePath =
				pdscTag->GetAttrString( L"path", NULL ) ;
			int	iFindArcSep = wstrFilePath.Find( L'#' ) ;
			if ( iFindArcSep < 0 )
			{
				//
				// ファイルサイズを取得する
				//
				WIN32_FIND_DATA	wfd ;
				HANDLE	hFind =
					::FindFirstFile( EString( wstrFilePath ), &wfd ) ;
				if ( hFind != INVALID_HANDLE_VALUE )
				{
					nBytes +=
						(((UINT64) wfd.nFileSizeHigh) << 32)
											| wfd.nFileSizeLow ;
					::FindClose( hFind ) ;
				}
			}
			else
			{
				//
				// アーカイブ内のファイルサイズを取得する
				//
				EString	strArcFile = wstrFilePath.Left( iFindArcSep ) ;
				EString	strFilePath = wstrFilePath.Middle( iFindArcSep + 1 ) ;
				ERawFile	file ;
				if ( !file.Open( strArcFile, file.modeRead | file.shareRead ) )
				{
					ERISAArchive	arcfile ;
					if ( !arcfile.Open( &file ) )
					{
						if ( !arcfile.OpenDirectory
							( strFilePath.GetFileDirectoryPart() ) )
						{
							ERISAArchive::FILE_INFO *	pfi =
								arcfile.ReferFileEntries().GetFileInfo
										( strFilePath.GetFileNamePart() ) ;
							if ( pfi != NULL )
							{
								nBytes += pfi->nBytes ;
							}
						}
						arcfile.Close( ) ;
					}
				}
			}
		}
		else if ( pdscTag->Tag() == L"directory" )
		{
			//
			// サブディレクトリ
			//
			UINT64	nSubBytes = 0 ;
			MeasureSizeDirectory( pdscTag, nSubBytes ) ;
			nBytes += nSubBytes ;
		}
	}
	return	eslErrSuccess ;
}


//////////////////////////////////////////////////////////////////////////////
// CRC32 計算用クラス
//////////////////////////////////////////////////////////////////////////////

const DWORD ECSSetup::CRC32::CRC32Table[256] =
{
	0x00000000,0x77073096,0xEE0E612C,0x990951BA,0x076DC419,0x706AF48F,
	0xE963A535,0x9E6495A3,0x0EDB8832,0x79DCB8A4,0xE0D5E91E,0x97D2D988,
	0x09B64C2B,0x7EB17CBD,0xE7B82D07,0x90BF1D91,0x1DB71064,0x6AB020F2,
	0xF3B97148,0x84BE41DE,0x1ADAD47D,0x6DDDE4EB,0xF4D4B551,0x83D385C7,
	0x136C9856,0x646BA8C0,0xFD62F97A,0x8A65C9EC,0x14015C4F,0x63066CD9,
	0xFA0F3D63,0x8D080DF5,0x3B6E20C8,0x4C69105E,0xD56041E4,0xA2677172,
	0x3C03E4D1,0x4B04D447,0xD20D85FD,0xA50AB56B,0x35B5A8FA,0x42B2986C,
	0xDBBBC9D6,0xACBCF940,0x32D86CE3,0x45DF5C75,0xDCD60DCF,0xABD13D59,
	0x26D930AC,0x51DE003A,0xC8D75180,0xBFD06116,0x21B4F4B5,0x56B3C423,
	0xCFBA9599,0xB8BDA50F,0x2802B89E,0x5F058808,0xC60CD9B2,0xB10BE924,
	0x2F6F7C87,0x58684C11,0xC1611DAB,0xB6662D3D,0x76DC4190,0x01DB7106,
	0x98D220BC,0xEFD5102A,0x71B18589,0x06B6B51F,0x9FBFE4A5,0xE8B8D433,
	0x7807C9A2,0x0F00F934,0x9609A88E,0xE10E9818,0x7F6A0DBB,0x086D3D2D,
	0x91646C97,0xE6635C01,0x6B6B51F4,0x1C6C6162,0x856530D8,0xF262004E,
	0x6C0695ED,0x1B01A57B,0x8208F4C1,0xF50FC457,0x65B0D9C6,0x12B7E950,
	0x8BBEB8EA,0xFCB9887C,0x62DD1DDF,0x15DA2D49,0x8CD37CF3,0xFBD44C65,
	0x4DB26158,0x3AB551CE,0xA3BC0074,0xD4BB30E2,0x4ADFA541,0x3DD895D7,
	0xA4D1C46D,0xD3D6F4FB,0x4369E96A,0x346ED9FC,0xAD678846,0xDA60B8D0,
	0x44042D73,0x33031DE5,0xAA0A4C5F,0xDD0D7CC9,0x5005713C,0x270241AA,
	0xBE0B1010,0xC90C2086,0x5768B525,0x206F85B3,0xB966D409,0xCE61E49F,
	0x5EDEF90E,0x29D9C998,0xB0D09822,0xC7D7A8B4,0x59B33D17,0x2EB40D81,
	0xB7BD5C3B,0xC0BA6CAD,0xEDB88320,0x9ABFB3B6,0x03B6E20C,0x74B1D29A,
	0xEAD54739,0x9DD277AF,0x04DB2615,0x73DC1683,0xE3630B12,0x94643B84,
	0x0D6D6A3E,0x7A6A5AA8,0xE40ECF0B,0x9309FF9D,0x0A00AE27,0x7D079EB1,
	0xF00F9344,0x8708A3D2,0x1E01F268,0x6906C2FE,0xF762575D,0x806567CB,
	0x196C3671,0x6E6B06E7,0xFED41B76,0x89D32BE0,0x10DA7A5A,0x67DD4ACC,
	0xF9B9DF6F,0x8EBEEFF9,0x17B7BE43,0x60B08ED5,0xD6D6A3E8,0xA1D1937E,
	0x38D8C2C4,0x4FDFF252,0xD1BB67F1,0xA6BC5767,0x3FB506DD,0x48B2364B,
	0xD80D2BDA,0xAF0A1B4C,0x36034AF6,0x41047A60,0xDF60EFC3,0xA867DF55,
	0x316E8EEF,0x4669BE79,0xCB61B38C,0xBC66831A,0x256FD2A0,0x5268E236,
	0xCC0C7795,0xBB0B4703,0x220216B9,0x5505262F,0xC5BA3BBE,0xB2BD0B28,
	0x2BB45A92,0x5CB36A04,0xC2D7FFA7,0xB5D0CF31,0x2CD99E8B,0x5BDEAE1D,
	0x9B64C2B0,0xEC63F226,0x756AA39C,0x026D930A,0x9C0906A9,0xEB0E363F,
	0x72076785,0x05005713,0x95BF4A82,0xE2B87A14,0x7BB12BAE,0x0CB61B38,
	0x92D28E9B,0xE5D5BE0D,0x7CDCEFB7,0x0BDBDF21,0x86D3D2D4,0xF1D4E242,
	0x68DDB3F8,0x1FDA836E,0x81BE16CD,0xF6B9265B,0x6FB077E1,0x18B74777,
	0x88085AE6,0xFF0F6A70,0x66063BCA,0x11010B5C,0x8F659EFF,0xF862AE69,
	0x616BFFD3,0x166CCF45,0xA00AE278,0xD70DD2EE,0x4E048354,0x3903B3C2,
	0xA7672661,0xD06016F7,0x4969474D,0x3E6E77DB,0xAED16A4A,0xD9D65ADC,
	0x40DF0B66,0x37D83BF0,0xA9BCAE53,0xDEBB9EC5,0x47B2CF7F,0x30B5FFE9,
	0xBDBDF21C,0xCABAC28A,0x53B39330,0x24B4A3A6,0xBAD03605,0xCDD70693,
	0x54DE5729,0x23D967BF,0xB3667A2E,0xC4614AB8,0x5D681B02,0x2A6F2B94,
	0xB40BBE37,0xC30C8EA1,0x5A05DF1B,0x2D02EF8D
} ;

void ECSSetup::CRC32::Stream( const BYTE * pbytBuf, int nBytes )
{
	for( int i = 0; i < nBytes; i ++ )
	{
		crc = CRC32Table[((pbytBuf[i] ^ crc) & 0xFF)] ^ (crc >> 8) ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// MD5 計算用クラス
//////////////////////////////////////////////////////////////////////////////

const DWORD ECSSetup::MD5::T[64] =
{
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,  //0
    0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,  //4
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,  //8
    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,  //12
    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,  //16
    0xd62f105d,  0x2441453, 0xd8a1e681, 0xe7d3fbc8,  //20
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,  //24
    0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,  //28
    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,  //32
    0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,  //36
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085,  0x4881d05,  //40
    0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,  //44
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,  //48
    0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,  //52
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,  //56
    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391   //60
} ;

void ECSSetup::MD5::Round_Calculate
	( const unsigned char * block,
		DWORD &A, DWORD &B, DWORD &C, DWORD &D )
{
	//
	// 計算用バッファ
	//
	DWORD	dwBuf[16] ;  // 512bit 64byte
	pX = dwBuf ;
	//
	// Copy block(padding_message) i into X
	//
	for ( int j = 0, k = 0; j < 64; j += 4, k ++ )
	{
		dwBuf[k] = ((DWORD) block[j])
					| (((DWORD) block[j+1]) << 8)
					| (((DWORD) block[j+2]) << 16)
					| (((DWORD) block[j+3]) << 24) ;
	}
	//
	// Save A as AA, B as BB, C as CC, and D as DD (A,B,C,Dの保存)
	//
	DWORD AA = A, BB = B, CC = C, DD = D ;
	//
	//Round 1
	//
	Round1(A,B,C,D,  0,  7,  0);  Round1(D,A,B,C,  1, 12,  1);  Round1(C,D,A,B,  2, 17,  2);  Round1(B,C,D,A,  3, 22,  3);
	Round1(A,B,C,D,  4,  7,  4);  Round1(D,A,B,C,  5, 12,  5);  Round1(C,D,A,B,  6, 17,  6);  Round1(B,C,D,A,  7, 22,  7);
	Round1(A,B,C,D,  8,  7,  8);  Round1(D,A,B,C,  9, 12,  9);  Round1(C,D,A,B, 10, 17, 10);  Round1(B,C,D,A, 11, 22, 11);
	Round1(A,B,C,D, 12,  7, 12);  Round1(D,A,B,C, 13, 12, 13);  Round1(C,D,A,B, 14, 17, 14);  Round1(B,C,D,A, 15, 22, 15);
	//
	//Round 2
	//
	Round2(A,B,C,D,  1,  5, 16);  Round2(D,A,B,C,  6,  9, 17);  Round2(C,D,A,B, 11, 14, 18);  Round2(B,C,D,A,  0, 20, 19);
	Round2(A,B,C,D,  5,  5, 20);  Round2(D,A,B,C, 10,  9, 21);  Round2(C,D,A,B, 15, 14, 22);  Round2(B,C,D,A,  4, 20, 23);
	Round2(A,B,C,D,  9,  5, 24);  Round2(D,A,B,C, 14,  9, 25);  Round2(C,D,A,B,  3, 14, 26);  Round2(B,C,D,A,  8, 20, 27);
	Round2(A,B,C,D, 13,  5, 28);  Round2(D,A,B,C,  2,  9, 29);  Round2(C,D,A,B,  7, 14, 30);  Round2(B,C,D,A, 12, 20, 31);
	//
	//Round 3
	//
	Round3(A,B,C,D,  5,  4, 32);  Round3(D,A,B,C,  8, 11, 33);  Round3(C,D,A,B, 11, 16, 34);  Round3(B,C,D,A, 14, 23, 35);
	Round3(A,B,C,D,  1,  4, 36);  Round3(D,A,B,C,  4, 11, 37);  Round3(C,D,A,B,  7, 16, 38);  Round3(B,C,D,A, 10, 23, 39);
	Round3(A,B,C,D, 13,  4, 40);  Round3(D,A,B,C,  0, 11, 41);  Round3(C,D,A,B,  3, 16, 42);  Round3(B,C,D,A,  6, 23, 43);
	Round3(A,B,C,D,  9,  4, 44);  Round3(D,A,B,C, 12, 11, 45);  Round3(C,D,A,B, 15, 16, 46);  Round3(B,C,D,A,  2, 23, 47);
	//
	//Round 4
	//
	Round4(A,B,C,D,  0,  6, 48);  Round4(D,A,B,C,  7, 10, 49);  Round4(C,D,A,B, 14, 15, 50);  Round4(B,C,D,A,  5, 21, 51);
	Round4(A,B,C,D, 12,  6, 52);  Round4(D,A,B,C,  3, 10, 53);  Round4(C,D,A,B, 10, 15, 54);  Round4(B,C,D,A,  1, 21, 55);
	Round4(A,B,C,D,  8,  6, 56);  Round4(D,A,B,C, 15, 10, 57);  Round4(C,D,A,B,  6, 15, 58);  Round4(B,C,D,A, 13, 21, 59);
	Round4(A,B,C,D,  4,  6, 60);  Round4(D,A,B,C, 11, 10, 61);  Round4(C,D,A,B,  2, 15, 62);  Round4(B,C,D,A,  9, 21, 63);
	//
	// Then perform the following additions.
	//
	A = A + AA ;
	B = B + BB ;
	C = C + CC ;
	D = D + DD ;
	//
	//機密情報のクリア
	//
	memset( dwBuf, 0, sizeof(dwBuf) ) ;
}

void ECSSetup::MD5::String
	( char * output, const char * string, int nLength )
{
#if	!defined(UINT_MAX)
	const DWORD		UINT_MAX = 0xffffffffUL ;
#endif
	BYTE	padding_message[64] ;	// 拡張メッセージ 512bit 64byte
	BYTE *	pstring ;				// 現在走査注中のstringの位置を保持
	BYTE	digest[16] ;
	DWORD	string_byte_len ;		// stringのバイト長を保持
	DWORD	string_bit_len ;		// stringのビット長を保持
	DWORD	copy_len ;				// 1-3で使う残ったバイト数
	DWORD	msg_digest[4] ;			// メッセージダイジェスト 128bit 4byte
	DWORD &	A = msg_digest[0] ;		// RFCに則ったメッセージダイジェスト
	DWORD & B = msg_digest[1] ;
	DWORD &	C = msg_digest[2] ;
	DWORD &	D = msg_digest[3] ;

	//Step 3. Initialize MD Buffer
	A = 0x67452301 ;
	B = 0xefcdab89 ;
	C = 0x98badcfe ;
	D = 0x10325476 ;

	//Step 1. Append Padding Bits
	//1-1
	if ( nLength < 0 )
	{
		string_byte_len = strlen( string ) ;
	}
	else
	{
		string_byte_len = nLength ;
	}
	pstring         = (BYTE*) string ;

	//1-2  長さが６４バイト未満になるまで計算を繰り返す
	int	i ;
	for ( i = string_byte_len; 64 <= i; i -= 64, pstring += 64 )
	{
		Round_Calculate( pstring, A,B,C,D ) ;
	}

	//1-3
	copy_len = string_byte_len % 64 ;
#if	_MSC_VER >= 1400
	strncpy_s( (char*) padding_message, sizeof(padding_message), (char*) pstring, copy_len ) ;
#else
	strncpy( (char*) padding_message, (char*) pstring, copy_len ) ;
#endif
	memset( padding_message + copy_len, 0, 64 - copy_len ) ;
	padding_message[copy_len] |= 0x80 ;

	//1-4 
	//残りが56バイト以上（６４バイト未満）ならば６４バイトに拡張して計算
	if ( 56 <= copy_len )
	{
		Round_Calculate( padding_message, A,B,C,D ) ;
		memset( padding_message, 0, 56 ) ;
	}

	//Step 2. Append Length (長さの情報を追加)
	string_bit_len = string_byte_len * 8 ;
	memcpy( &padding_message[56], &string_bit_len, 4 ) ;

	//下位３２バイトだけではビット長を表現できないときは上位に桁上げ
	if ( UINT_MAX / 8 < string_byte_len )
	{
		unsigned int high = (string_byte_len - UINT_MAX / 8) * 8 ;
		memcpy( &padding_message[60], &high, 4 ) ;
	}
	else
	{
		memset( &padding_message[60], 0, 4) ;
	}

	//Step 4. Process Message in 16-Word Blocks (MD5の計算)
	Round_Calculate( padding_message, A,B,C,D ) ;

	//Step 5. Output (出力)
	memcpy( digest, msg_digest, 16 ) ;
	int	j ;
	for ( i = 0, j = 0; i < 16; i ++, j += 2 )
	{
		BYTE	bytHigh = (digest[i] >> 4) & 0x0F ;
		BYTE	bytLow = digest[i] & 0x0F ;
		if ( bytHigh < 10 )
		{
			output[j] = '0' + bytHigh ;
		}
		else
		{
			output[j] = ('A' - 10) + bytHigh ;
		}
		if ( bytLow < 10 )
		{
			output[j + 1] = '0' + bytLow ;
		}
		else
		{
			output[j + 1] = ('A' - 10) + bytLow ;
		}
	}
	output[j] = 0 ;
}


//////////////////////////////////////////////////////////////////////////////
// セットアップコンポーネント
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO2( ECSSetup, ECSObject, EGLSThread )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ECSSetup::ECSSetup( void )
{
	m_fRebootToDelete = false ;
	m_pdscCurrentDir = NULL ;
	m_iCurrent = 0 ;
	m_nTotalInstallationBytes = 0 ;
	m_nTotalInstalledBytes = 0 ;
	m_nCurrentFileBytes = 0 ;
	m_nCurrentCopiedBytes = 0 ;
	m_pdstfile = NULL ;
	m_dstfile = NULL ;
	m_fDstArchive = false ;
	m_psrcfile = NULL ;
	m_srcfile = NULL ;
	m_fSrcArchive = false ;
	m_hFinishedCopy = NULL ;
	m_hThreadReady = NULL ;
	m_hBootMutex = NULL ;
	m_pInstDlgParam = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ECSSetup::~ECSSetup( void )
{
	EndInstall( ) ;
	CloseInstallationDialog( ) ;
	ReleaseBootCheck( ) ;
}

// セットアップするファイルリストを読み込む
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::ReadSetupList( ESLFileObject & file )
{
	EStreamBuffer	buf ;
	buf.ReadFromFile( file ) ;
	return	m_flstSetup.ReadDescription( buf ) ;
}

// セットアップするファイルリストを書き出す
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::WriteSetupList( ESLFileObject & file )
{
	return	m_flstSetup.WriteDescription
		( file, 0, EDescription::dftAuto, EDescription::ceUTF8 ) ;
}

// インストールログファイルを読み込む
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::ReadInstalledLog( ESLFileObject & file )
{
	EStreamBuffer	buf ;
	buf.ReadFromFile( file ) ;
	return	m_flstLog.ReadDescription( buf ) ;
}

// インストールログファイルを書き出す
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::WriteInstalledLog( ESLFileObject & file )
{
	return	m_flstLog.WriteDescription
		( file, 0, EDescription::dftAuto, EDescription::ceUTF8 ) ;
}

// インストール先ディレクトリを追加する
//////////////////////////////////////////////////////////////////////////////
EDescription * ECSSetup::AddInstallDirectory
	( const wchar_t * pwszBasePath )
{
	return	m_flstSetup.AddDirectory( pwszBasePath, 0 ) ;
}

// インストール先アーカイブディレクトリを追加する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::AddInstallArchiveDirectory
	( const wchar_t * pwszBasePath,
		const char * pszPassword, int nType )
{
	DWORD	dwEncodeType = 0 ;
	if ( nType & 0x01 )
	{
		dwEncodeType |= ERISAArchive::etERISACode ;
	}
	if ( nType & 0x02 )
	{
		dwEncodeType |= ERISAArchive::etSimpleCrypt32 ;
	}
	return	m_flstSetup.AddArchiveDirectory
				( pwszBasePath, pszPassword, dwEncodeType ) ;
}

// インストール先ファイルを追加する
//////////////////////////////////////////////////////////////////////////////
EDescription * ECSSetup::AddInstallFile
	( const wchar_t * pwszBasePath, const wchar_t * pwszFileName )
{
	return	m_flstSetup.AddFile( pwszBasePath, pwszFileName ) ;
}

// インストール先アーカイブファイルを追加する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::AddInstallArchiveTree
	( const wchar_t * pwszBasePath,
		const wchar_t * pwszSrcArchive, const char * pszPassword )
{
	return	m_flstSetup.AddArchiveTree
				( pwszBasePath, pwszSrcArchive, pszPassword ) ;
}

// インストール先ディレクトリを追加する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::AddInstallDirectoryTree
	( const wchar_t * pwszBasePath,
		const wchar_t * pwszSrcDirectory )
{
	return	m_flstSetup.AddDirectoryTree
				( pwszBasePath, pwszSrcDirectory ) ;
}

// 起動チェック
//////////////////////////////////////////////////////////////////////////////
bool ECSSetup::BootCheck( const char * pszCheckName, bool fDisableBoot )
{
	EString	strMutexName = "COTOMI_" ;
	strMutexName += pszCheckName ;
	if ( m_hBootMutex != NULL )
	{
		::CloseHandle( m_hBootMutex ) ;
	}
	m_hBootMutex = ::CreateMutex( NULL, TRUE, strMutexName ) ;
	bool	fBoot = false ;
	if ( (m_hBootMutex == NULL)
		|| (GetLastError() == ERROR_ALREADY_EXISTS) )
	{
		fBoot = true ;
	}
	if ( !fDisableBoot && (m_hBootMutex != NULL) )
	{
		::CloseHandle( m_hBootMutex ) ;
		m_hBootMutex = NULL ;
	}
	return	fBoot ;
}

// 起動チェック解放
//////////////////////////////////////////////////////////////////////////////
void ECSSetup::ReleaseBootCheck( void )
{
	if ( m_hBootMutex != NULL )
	{
		::CloseHandle( m_hBootMutex ) ;
		m_hBootMutex = NULL ;
	}
}

// ProductID を取得する
//////////////////////////////////////////////////////////////////////////////
typedef BOOL (WINAPI * API_IsWow64Process)( HANDLE, PBOOL ) ;
EString ECSSetup::GetWindowsProductID( void )
{
	SString	strProductID ;
	SString	strWindowsDir ;
	if ( !SFile::GetDefaultDirectory
		( strWindowsDir, SFile::DefaultDirectory::WindowsDirectory ) )
	{
		SString	strDrv = strWindowsDir.GetFileDrivePart() ;
		strDrv.Replace( L'/', L'\\' ) ;
		if ( strDrv.GetLastAt(0) != L'\\' )
		{
			strDrv += L'\\' ;
		}
		UINT	nErrorMode = ::SetErrorMode( SEM_FAILCRITICALERRORS ) ;
		SArray<char>	bufDrv ;
		DWORD			dwSerialNum ;
		if ( GetVolumeInformation
			( strDrv.EncodeDefaultTo(bufDrv),
				NULL, 0, &dwSerialNum, NULL, NULL, NULL, 0 ) )
		{
			SString	strHex( dwSerialNum, 8, 16 ) ;
			strProductID += strHex ;
		}
		::SetErrorMode( nErrorMode ) ;
	}
	SArray<char>	bufProductID ;
	return	EString( strProductID.EncodeDefaultTo( bufProductID ) ) ;
/*
	//
	// Wow64 判定
	//
	DWORD	dwWow64Key = 0 ;
	bool	fWindowsNT = false ;
	OSVERSIONINFO	osvi ;
	osvi.dwOSVersionInfoSize = sizeof( OSVERSIONINFO ) ;
	if ( ::GetVersionEx( &osvi ) )
	{
		if ( osvi.dwPlatformId == VER_PLATFORM_WIN32_NT )
		{
			fWindowsNT = true ;
			//
			HMODULE	hModule = ::GetModuleHandle( "Kernel32.dll" );
			if ( hModule != NULL )
			{
				API_IsWow64Process	apiIsWow64Process =
					(API_IsWow64Process)
						::GetProcAddress( hModule, "IsWow64Process" ) ;
				if ( apiIsWow64Process != NULL )
				{
					BOOL	fWow64 = false ;
					if ( apiIsWow64Process( ::GetCurrentProcess(), &fWow64 ) )
					{
						if ( fWow64 )
						{
							dwWow64Key = KEY_WOW64_64KEY ;
						}
					}
				}
			}
		}
	}
	//
	// レジストリを読み込む
	//
	ERegistryKey	key ;
	const char *	pszKeyPath =
		"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion" ;
	if ( !fWindowsNT )
	{
		pszKeyPath =
			"SOFTWARE\\Microsoft\\Windows\\CurrentVersion" ;
	}
	EString	strProductID = "" ;
	if ( !key.OpenKey
		( HKEY_LOCAL_MACHINE, pszKeyPath, KEY_ALL_ACCESS | dwWow64Key ) )
	{
		strProductID = key.GetString( "ProductId", "" ) ;
		key.CloseKey( ) ;
	}
	return	strProductID ;
*/
}

// MD5 ハッシュを生成する
//////////////////////////////////////////////////////////////////////////////
EString ECSSetup::MakeMD5Digest( const char * pszBuf, int nLength )
{
	EString	strMD5Digest ;
	MD5		md5 ;
	md5.String( strMD5Digest.GetBuffer( 0x40 ), pszBuf, nLength ) ;
	strMD5Digest.ReleaseBuffer( ) ;
	return	strMD5Digest ;
}

// CRC32 を生成する
//////////////////////////////////////////////////////////////////////////////
DWORD ECSSetup::CalcCRC32( const char * pszBuf, int nLength )
{
	int		i ;
	if ( nLength < 0 )
	{
		if ( pszBuf != NULL )
		{
			for ( i = 0; pszBuf[i]; i ++ )
			{
			}
			nLength = i ;
		}
		else
		{
			nLength = 0 ;
		}
	}
	DWORD	crc = 0xFFFFFFFF ;
	for( i = 0; i < nLength; i ++ )
	{
		crc = CRC32::CRC32Table[((pszBuf[i] ^ crc) & 0xFF)] ^ (crc >> 8) ;
	}
	return	(crc ^ 0xFFFFFFFF) ;
}

// 32bitチェックサムを生成する
//////////////////////////////////////////////////////////////////////////////
DWORD ECSSetup::CheckSum32( const char * pszBuf, int nLength )
{
	int		i ;
	if ( nLength < 0 )
	{
		if ( pszBuf != NULL )
		{
			for ( i = 0; pszBuf[i]; i ++ )
			{
			}
			nLength = i ;
		}
		else
		{
			nLength = 0 ;
		}
	}
	DWORD	sum = 0 ;
	for( i = 0; i < nLength; i ++ )
	{
		DWORD	d = ((DWORD) pszBuf[i]) << ((i & 0x03) * 8) ;
		if ( d > ~sum )
		{
			sum += d + 1 ;
		}
		else
		{
			sum += d ;
		}
	}
	return	sum ;
}

// デスクトップパスを取得する
//////////////////////////////////////////////////////////////////////////////
EString ECSSetup::GetDesktopDirectory( void )
{
	const char *	pszKeyPath =
		"Software\\Microsoft\\Windows\\"
		"CurrentVersion\\Explorer\\Shell Folders" ;
	ERegistryKey	key ;
	EString			strPath ;
	if ( !key.OpenKey( HKEY_CURRENT_USER, pszKeyPath ) )
	{
		strPath = key.GetString( "Desktop", "" ) ;
		key.CloseKey( ) ;
	}
	return	strPath ;
}

// スタートメニューパスを取得する
//////////////////////////////////////////////////////////////////////////////
EString ECSSetup::GetStartMenuDirectory( void )
{
	const char *	pszKeyPath =
		"Software\\Microsoft\\Windows\\"
		"CurrentVersion\\Explorer\\Shell Folders" ;
	ERegistryKey	key ;
	EString			strPath ;
	if ( !key.OpenKey( HKEY_CURRENT_USER, pszKeyPath ) )
	{
		strPath = key.GetString( "Programs", "" ) ;
		key.CloseKey( ) ;
	}
	return	strPath ;
}

EString ECSSetup::GetCommonStartMenuDirectory( void )
{
	const char *	pszKeyPath =
		"SOFTWARE\\Microsoft\\Windows\\"
		"CurrentVersion\\Explorer\\Shell Folders" ;
	ERegistryKey	key ;
	EString			strPath ;
	if ( !key.OpenKey( HKEY_LOCAL_MACHINE, pszKeyPath ) )
	{
		strPath = key.GetString( "Common Programs", "" ) ;
		key.CloseKey( ) ;
	}
	return	strPath ;
}

// アプリケーションデータパスを取得する
//////////////////////////////////////////////////////////////////////////////
EString ECSSetup::GetAppDataDirectory( void )
{
	const char *	pszKeyPath =
		"Software\\Microsoft\\Windows\\"
		"CurrentVersion\\Explorer\\Shell Folders" ;
	ERegistryKey	key ;
	EString			strPath ;
	if ( !key.OpenKey( HKEY_CURRENT_USER, pszKeyPath ) )
	{
		strPath = key.GetString( "AppData", "" ) ;
		key.CloseKey( ) ;
	}
	return	strPath ;
}

// Windows ディレクトリを取得する
//////////////////////////////////////////////////////////////////////////////
EString ECSSetup::GetWindowsDirectory( void )
{
	EString	strPath ;
	::GetWindowsDirectory( strPath.GetBuffer( 0x400 ), 0x400 ) ;
	strPath.ReleaseBuffer( ) ;
	return	strPath ;
}

// モジュール（インストーラー基底）パスを取得する
//////////////////////////////////////////////////////////////////////////////
EString ECSSetup::GetCurrentModulePath( void )
{
	EString	strPath ;
	::GetModuleFileName
		( ::GetModuleHandle(NULL),
			strPath.GetBuffer(0x400), 0x400 ) ;
	strPath.ReleaseBuffer( ) ;
	return	strPath ;
}

// ディスクボリュームを取得する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::GetDiskVolumeName
	( const char * pszDrv, EString & strVolumeName )
{
	EWideString	wstrDrv = pszDrv ;
	if ( !wstrDrv.IsEmpty() && (wstrDrv.Right(1) != L"\\") )
	{
		wstrDrv += L'\\' ;
	}
	UINT	nErrorMode = ::SetErrorMode( SEM_FAILCRITICALERRORS ) ;
	BOOL	fResult =
		::GetVolumeInformation
			( EString( wstrDrv ), strVolumeName.GetBuffer(0x400),
							0x400, NULL, NULL, NULL, NULL, 0 ) ;
	strVolumeName.ReleaseBuffer() ;
	::SetErrorMode( nErrorMode ) ;
	if ( fResult )
	{
		return	eslErrSuccess ;
	}
	return	eslErrGeneral ;
}

// ディスクのシリアル番号を取得する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::GetDiskSerialNumber
	( const char * pszDrv, DWORD & dwSerialNumber )
{
	EWideString	wstrDrv = pszDrv ;
	if ( !wstrDrv.IsEmpty() && (wstrDrv.Right(1) != L"\\") )
	{
		wstrDrv += L'\\' ;
	}
	UINT	nErrorMode = ::SetErrorMode( SEM_FAILCRITICALERRORS ) ;
	BOOL	fResult =
		::GetVolumeInformation
			( EString( wstrDrv ),
				NULL, 0, &dwSerialNumber, NULL, NULL, NULL, 0 ) ;
	::SetErrorMode( nErrorMode ) ;
	if ( fResult )
	{
		return	eslErrSuccess ;
	}
	return	eslErrGeneral ;
}

// ディスクの空き容量を取得する
//////////////////////////////////////////////////////////////////////////////
typedef BOOL (WINAPI * API_GetDiskFreeSpaceEx)
	( IN LPCSTR lpDirectoryName,
		OUT PULARGE_INTEGER lpFreeBytesAvailableToCaller,
		OUT PULARGE_INTEGER lpTotalNumberOfBytes,
		OUT PULARGE_INTEGER lpTotalNumberOfFreeBytes ) ;
ESLError ECSSetup::GetDiskFreeSpace
	( const char * pszDrv, UINT64 & nFreeAvailable,
				UINT64 & nTotalBytes, UINT64 & nFreeSpace )
{
	EWideString	wstrDrv = pszDrv ;
	if ( !wstrDrv.IsEmpty() && (wstrDrv.Right(1) != L"\\") )
	{
		wstrDrv += L'\\' ;
	}
	API_GetDiskFreeSpaceEx	apiGetDiskFreeSpaceEx =
		(API_GetDiskFreeSpaceEx) ::GetProcAddress
			( GetModuleHandle("kernel32.dll"), "GetDiskFreeSpaceExA" ) ;
	if ( apiGetDiskFreeSpaceEx != NULL )
	{
		if ( apiGetDiskFreeSpaceEx
			( EString( wstrDrv ),
				(PULARGE_INTEGER) &nFreeAvailable,
				(PULARGE_INTEGER) &nTotalBytes,
				(PULARGE_INTEGER) &nFreeSpace ) )
		{
			return	eslErrSuccess ;
		}
	}
	else
	{
		DWORD	dwSectPerClust, dwBytesPerSect,
				dwFreeClusters, dwTotalClusters ;
		if ( ::GetDiskFreeSpace
			( EString( wstrDrv ),
				&dwSectPerClust, &dwBytesPerSect,
				&dwFreeClusters, &dwTotalClusters ) )
		{
			UINT64	nBytesPerClust =
				(UINT64) dwBytesPerSect * dwSectPerClust ;
			nTotalBytes = dwTotalClusters * nBytesPerClust ;
			nFreeSpace = dwFreeClusters * nBytesPerClust ;
			nFreeAvailable = nFreeSpace ;
			return	eslErrSuccess ;
		}
	}
	return	eslErrGeneral ;
}

// インストール情報を取得する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::GetUninstallInfo
	( const wchar_t * pwszRegName, UNINSTALL_INFO & uninst_info )
{
	EString	strRegPath =
		"SOFTWARE\\Microsoft\\Windows\\"
		"CurrentVersion\\Uninstall\\" + EString( pwszRegName ) ;
	ERegistryKey	key ;
	EString			strInstDir ;
	if ( key.OpenKey( HKEY_LOCAL_MACHINE, strRegPath, KEY_ALL_ACCESS ) )
	{
		return	eslErrGeneral ;
	}
	uninst_info.wstrDisplayName =
		key.GetString( "DisplayName", NULL ) ;
	uninst_info.wstrDisplayIcon =
		key.GetString( "DisplayIcon", NULL ) ;
	uninst_info.wstrUninstallCmdLine =
		key.GetString( "UninstallString", NULL ) ;
	uninst_info.wstrUninstallPath =
		key.GetString( "UninstallPath", NULL ) ;
	uninst_info.wstrInstallLocation =
		key.GetString( "InstallLocation", NULL ) ;
	uninst_info.wstrPublisher =
		key.GetString( "Publisher", NULL ) ;
	uninst_info.wstrVersionMajor =
		key.GetString( "VersionMajor", NULL ) ;
	uninst_info.wstrVersionMinor =
		key.GetString( "VersionMinor", NULL ) ;
	key.CloseKey( ) ;
	return	eslErrSuccess ;
}

// ショートカットファイルを作成する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::CreateShortcutFile
	( const wchar_t * pwszShortcutFile,
		const char * pszLinkFile, const char * pszArg )
{
	//
	// ショートカットファイルオブジェクト作成
	//
	ESLError	err = eslErrGeneral ;
	IShellLink *	psl ;
	if( SUCCEEDED( ::CoCreateInstance
		( CLSID_ShellLink, NULL, 
			CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&psl ) ) )
	{
		//
		// ファイル名設定
		//
		EWideString	wstrShortcutFile = pwszShortcutFile ;
		EString		strName = wstrShortcutFile.GetFileTitlePart() ;
		EString		strLinkFile = pszLinkFile ;
		psl->SetPath( strLinkFile ) ;
		psl->SetDescription( strName ) ;
		psl->SetWorkingDirectory( strLinkFile.GetFileDirectoryPart() ) ;
		if ( (pszArg != NULL) && (pszArg[0] != '\0') )
		{
			psl->SetArguments( pszArg ) ;
		}
		//
		// 書き出しのためのインターフェース取得
		//
		IPersistFile *	ppf ;
		if( SUCCEEDED( psl->QueryInterface
			( IID_IPersistFile, (void**)&ppf ) ) )
		{
			if ( ppf->Save( wstrShortcutFile, TRUE ) == S_OK )
			{
				err = eslErrSuccess ;
			}
			ppf->Release( ) ;
		}
		psl->Release( ) ;
	}
	return	err ;
}

// シェルでファイルを開く
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::ShellExecute
	( const wchar_t * pwszVerb,
		const wchar_t * pwszFile, const wchar_t * pwszParameter )
{
	EString	strVerb = pwszVerb ;
	EString	strFile = pwszFile ;
	EString	strParameter = pwszParameter ;
	const char *	pszParam = strParameter ;
	if ( strParameter.IsEmpty() )
	{
		pszParam = NULL ;
	}
	if ( (ULONG_PTR) ::ShellExecute
		( NULL, strVerb, strFile, pszParam, NULL, SW_SHOW ) > 32 )
	{
		return	eslErrSuccess ;
	}
	return	eslErrGeneral ;
}

// Win32 実行可能ファイルを起動する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::ExecuteProcess
	( const wchar_t * pwszFile, const wchar_t * pwszParameter,
		DWORD dwFlags, const wchar_t * pwszEnvironment,
		const wchar_t * pwszCurrentDirectory )
{
	return	m_processSub.CreateProcess
		( EString( pwszFile ), EString( pwszParameter ), dwFlags,
			EString( pwszEnvironment ), EString( pwszCurrentDirectory ) ) ;
}

// Execute で起動したプロセスの完了を待つ
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::GetExitCodeExecute( DWORD dwTimeout, DWORD * pExitCode )
{
	return	m_processSub.GetProcessExitCode( dwTimeout, pExitCode ) ;
}

// ディレクトリを選択する
//////////////////////////////////////////////////////////////////////////////
static LRESULT __stdcall BrowseForlderWindowProcedureCallback( void * pInstance ) ;
static int CALLBACK BrowseCallbackProc
	( HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData ) ;
struct	BROWSE_FOLDER
{
	BOOL	fResult ;
	HWND	hwndParent ;
	HANDLE	hEventDone ;
	EString	strCaption ;
	EString	strDirPath ;
} ;

ESLError ECSSetup::BrowseForFolder
	( EWideString & wstrDir,
		const wchar_t * pwszCaption,
		ECSWindow * pWindow, HWND hwndParent )
{
	if ( (hwndParent == NULL) && (pWindow != NULL) )
	{
		EWindow *	pWnd = pWindow->GetWindow( ) ;
		if ( pWnd != NULL )
		{
			hwndParent = *pWnd ;
		}
	}
	BROWSE_FOLDER	bf ;
	bf.fResult = FALSE ;
	bf.hwndParent = hwndParent ;
	bf.hEventDone = ::CreateEvent( NULL, TRUE, FALSE, NULL ) ;
	bf.strCaption = pwszCaption ;
	bf.strDirPath = wstrDir ;
	//
	if ( pWindow != NULL )
	{
		LRESULT	lResult ;
		if ( pWindow->ProcedureOnWindowThread
			( BrowseForlderWindowProcedureCallback, &bf, &lResult, true ) )
		{
			return	eslErrGeneral ;
		}
		::WaitForSingleObject( bf.hEventDone, INFINITE ) ;
	}
	else
	{
		BrowseForlderWindowProcedureCallback( &bf ) ;
	}
	//
	::CloseHandle( bf.hEventDone ) ;
	//
	if ( bf.fResult )
	{
		wstrDir = bf.strDirPath ;
		return	eslErrSuccess ;
	}
	return	eslErrAbort ;
}

static LRESULT __stdcall
	BrowseForlderWindowProcedureCallback( void * pInstance )
{
	BROWSE_FOLDER *	pbf = (BROWSE_FOLDER*) pInstance ;
	BROWSEINFO		bi ;
	EString			strDispName ;
	::memset( &bi, 0, sizeof(bi) ) ;
	bi.hwndOwner = pbf->hwndParent ;
	bi.pszDisplayName = strDispName.GetBuffer(MAX_PATH) ;
	bi.lpszTitle = pbf->strCaption ;
	bi.lpfn = &BrowseCallbackProc ;
	bi.lParam = (LPARAM) pbf->strDirPath.CharPtr( ) ;
	//
	LPITEMIDLIST	piidl = ::SHBrowseForFolder( &bi ) ;
	if ( piidl != NULL )
	{
		::SHGetPathFromIDList( piidl, bi.pszDisplayName ) ;
		::CoTaskMemFree( piidl ) ;
		pbf->strDirPath = bi.pszDisplayName ;
		pbf->fResult = true ;
	}
	else
	{
		pbf->fResult = false ;
	}
	if ( bi.hwndOwner != NULL )
	{
		::EnableWindow( bi.hwndOwner, TRUE ) ;
	}
	::SetEvent( pbf->hEventDone ) ;
	//
	return	0 ;
}

static int CALLBACK BrowseCallbackProc
	( HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData )
{
	if ( uMsg == BFFM_INITIALIZED )
	{
		SendMessage( hwnd, BFFM_SETSELECTION, (WPARAM) TRUE, lpData ) ;
	}
	return	0 ;
}

// ファイルを選択する
//////////////////////////////////////////////////////////////////////////////
static LRESULT __stdcall BrowseFileDialogWindowProcedureCallback( void * pInstance ) ;
struct	BROWSE_FILE
{
	BOOL	fResult ;
	BOOL	fSaveFile ;
	HWND	hwndParent ;
	HANDLE	hEventDone ;
	EString	strCaption ;
	EString	strFilter ;
	EString	strFilePath ;
} ;

ESLError ECSSetup::BrowseFileDialog
	( EWideString & wstrFile,
		bool fSaveFileDialog,
		const wchar_t * pwszCaption,
		const wchar_t * pwszFilters,
		ECSWindow * pWindow, HWND hwndParent )
{
	BROWSE_FILE	bf ;
	bf.fResult = FALSE ;
	bf.fSaveFile = fSaveFileDialog ;
	bf.hwndParent = hwndParent ;
	bf.hEventDone = ::CreateEvent( NULL, TRUE, FALSE, NULL ) ;
	bf.strCaption = pwszCaption ;
	bf.strFilter = pwszFilters ;
	bf.strFilePath = wstrFile ;
	//
	for ( size_t i = 0; i < bf.strFilter.GetLength(); i ++ )
	{
		if ( bf.strFilter.GetAt(i) == '|' )
		{
			bf.strFilter.SetAt( i, '\0' ) ;
		}
	}
	//
	if ( pWindow != NULL )
	{
		LRESULT	lResult ;
		if ( pWindow->ProcedureOnWindowThread
			( BrowseFileDialogWindowProcedureCallback, &bf, &lResult, true ) )
		{
			return	eslErrGeneral ;
		}
		::WaitForSingleObject( bf.hEventDone, INFINITE ) ;
	}
	else
	{
		BrowseFileDialogWindowProcedureCallback( &bf ) ;
	}
	::CloseHandle( bf.hEventDone ) ;
	//
	if ( bf.fResult )
	{
		wstrFile = bf.strFilePath ;
		return	eslErrSuccess ;
	}
	return	eslErrAbort ;
}

static LRESULT __stdcall
	BrowseFileDialogWindowProcedureCallback( void * pInstance )
{
	BROWSE_FILE *	pbf = (BROWSE_FILE*) pInstance ;
	//
	OPENFILENAME	ofn ;
	EString			strFilePath ;
	EString			strInitDir = pbf->strFilePath.GetFileDirectoryPart() ;
	if ( strInitDir.Right(1) == "\\" )
	{
		strInitDir = strInitDir.Left( strInitDir.GetLength() - 1 ) ;
	}
	memset( &ofn, 0, sizeof(OPENFILENAME) ) ;
	ofn.lStructSize = sizeof(OPENFILENAME) ;
	ofn.hwndOwner = pbf->hwndParent ;
	ofn.lpstrFilter = pbf->strFilter ;
	ofn.lpstrFile = strFilePath.GetBuffer( 0x400 ) ;
	ofn.nMaxFile = 0x400 ;
	ofn.lpstrInitialDir = strInitDir ;
	ofn.lpstrTitle = pbf->strCaption ;
	//
	if ( pbf->fSaveFile )
	{
		ofn.Flags = OFN_OVERWRITEPROMPT ;
		pbf->fResult = ::GetSaveFileName( &ofn ) ;
	}
	else
	{
		ofn.Flags = OFN_HIDEREADONLY ;
		pbf->fResult = ::GetOpenFileName( &ofn ) ;
	}
	pbf->strFilePath = ofn.lpstrFile ;
	//
	if ( ofn.hwndOwner != NULL )
	{
		::EnableWindow( ofn.hwndOwner, TRUE ) ;
	}
	::SetEvent( pbf->hEventDone ) ;
	//
	return	0 ;
}

// システムを再起動させる
//////////////////////////////////////////////////////////////////////////////
void ECSSetup::RebootWindows( void )
{
	//
	// Get OS type (Windows or NT)
	//
#if	_MSC_VER < 1800
	OSVERSIONINFO	osvi;
	osvi.dwOSVersionInfoSize = sizeof(osvi);
	::GetVersionEx( &osvi );
#endif
	//
	// Whether it can use NT APIs?
	//
#if	_MSC_VER < 1800
	if( osvi.dwPlatformId == VER_PLATFORM_WIN32_NT )
#endif
	{
		HANDLE hToken ;
		TOKEN_PRIVILEGES tkp ;
		//
		// Get a token for this process. 
		//
 		if ( !OpenProcessToken
			( GetCurrentProcess(), 
				TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken ) )
		{
			ESLTrace( "Failed to OpenProcessToken\n" ) ;
			return ;
		}
		//
		// Get the LUID for the shutdown privilege. 
		//
		LookupPrivilegeValue
			( NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid ) ;
		//
		tkp.PrivilegeCount = 1;  // one privilege to set
		tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED ;
		//
		// Get the shutdown privilege for this process. 
		//
		AdjustTokenPrivileges
			( hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0 ) ;
		//
		// Cannot test the return value of AdjustTokenPrivileges. 
		//
		if ( GetLastError() != ERROR_SUCCESS )
		{
			ESLTrace( "Failed to AdjustTokenPrivileges\n" ) ;
			return ;
		}
	}
	//
	// Shut down the system and force all applications to close. 
	//
	if ( !ExitWindowsEx( EWX_REBOOT | EWX_FORCE, 0 ) ) 
	{
		ESLTrace( "Failed to ExitWindowsEx\n" ) ;
	}
}

// 最後のエラーメッセージを取得する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::GetLastErrorMsg( EWideString & wstrErrMsg )
{
	ESLError	err ;
	m_csStatus.Lock() ;
	err = m_errCopyResult ;
	wstrErrMsg = m_wstrErrMsg ;
	m_csStatus.Unlock() ;
	return	err ;
}

// 容量を計算する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::MeasureInstallSize( UINT64 & nBytes )
{
	ESLError	err =
		m_flstSetup.MeasureSize( m_nTotalInstallationBytes ) ;
	if ( !err )
	{
		nBytes = m_nTotalInstallationBytes ;
	}
	else
	{
		nBytes = 0 ;
	}
	return	err ;
}

// スレッド関数
//////////////////////////////////////////////////////////////////////////////
DWORD ECSSetup::ThreadProc( void )
{
	MSG	msg ;
	::PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) ;
	::SetEvent( m_hThreadReady ) ;
	//
	while ( ::GetMessage( &msg, NULL, 0, 0 ) )
	{
		if ( msg.message == tmQuitThread )
		{
			break ;
		}
		switch ( msg.message )
		{
		case	tmBeginCopy:
			OnFileCopy( ) ;
			break ;

		default:
			::TranslateMessage( &msg ) ;
			::DispatchMessage( &msg ) ;
			break ;
		}
	}
	//
	return	0 ;
}

// ファイルコピー
//////////////////////////////////////////////////////////////////////////////
void ECSSetup::OnFileCopy( void )
{
	//
	// ファイルコピーの開始
	//
	ESLError	errResult = eslErrSuccess ;
	m_csStatus.Lock() ;
	m_nCurrentFileBytes = m_psrcfile->GetLargeLength( ) ;
	m_nCurrentCopiedBytes = 0 ;
	m_wstrErrMsg = L"ファイルをコピーしています" ;
	m_errCopyResult = eslErrPending ;
	m_csStatus.Unlock() ;
	::SetEvent( m_hThreadReady ) ;
	//
	// ファイルのコピー
	//
	EStreamBuffer	buf ;
	const DWORD		dwBufSize = 0x10000 ;
	void *			ptrBuf = buf.PutBuffer( dwBufSize ) ;
	//
	for ( ; ; )
	{
		MSG	msg ;
		if ( ::PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
		{
			if ( msg.message == tmQuitThread )
			{
				m_csStatus.Lock() ;
				m_errCopyResult = eslErrAbort ;
				m_wstrErrMsg = L"キャンセルしました。" ;
				break ;
			}
		}
		//
		UINT64	nLastBytes = m_nCurrentFileBytes - m_nCurrentCopiedBytes ;
		DWORD	dwNextBytes = dwBufSize ;
		if ( nLastBytes < dwNextBytes )
		{
			dwNextBytes = (DWORD) nLastBytes ;
		}
		DWORD	dwReadBytes = m_psrcfile->Read( ptrBuf, dwNextBytes ) ;
		DWORD	dwWrittenBytes = m_pdstfile->Write( ptrBuf, dwReadBytes ) ;
		//
		m_csStatus.Lock() ;
		m_nCurrentCopiedBytes += dwWrittenBytes ;
		m_nTotalInstalledBytes += dwWrittenBytes ;
		//
		if ( m_nCurrentCopiedBytes >= m_nCurrentFileBytes )
		{
			m_wstrErrMsg = L"" ;
			m_errCopyResult = eslErrSuccess ;
			break ;
		}
		else if ( dwReadBytes < dwNextBytes )
		{
			m_errCopyResult = eslErrAbort ;
			m_wstrErrMsg = L"ファイルの読み込みに失敗しました。" ;
			break ;
		}
		else if ( dwWrittenBytes < dwNextBytes )
		{
			m_errCopyResult = eslErrAbort ;
			m_wstrErrMsg = L"ファイルの書き出しに失敗しました。" ;
			break ;
		}
		m_csStatus.Unlock() ;
	}
	//
	// ファイルのコピー完了
	//
	if ( m_fDstArchive )
	{
		m_dstarcf.AscendFile( ) ;
	}
	else
	{
		delete	m_dstfile ;
		m_dstfile = NULL ;
		m_pdstfile = NULL ;
	}
	if ( m_fSrcArchive )
	{
		if ( m_srcarcf.AscendFile( ) )
		{
			if ( m_errCopyResult == eslErrSuccess )
			{
				m_errCopyResult = eslErrAbort ;
				m_wstrErrMsg = L"ファイルのチェックサムが一致しません。" ;
			}
		}
	}
	::SetEvent( m_hFinishedCopy ) ;
	m_csStatus.Unlock() ;
}

// ファイルのインストールを開始する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::BeginInstall( const wchar_t * pwszInstallDir )
{
	if ( m_pdscCurrentDir != NULL )
	{
		return	eslErrGeneral ;
	}
	//
	// ステータス初期化
	//
	m_wstrInstDir = pwszInstallDir ;
//	m_flstLog.RemoveAllContentTag() ;
	if ( m_wstrInstDir.Right(1) != L"\\" )
	{
		m_wstrInstDir += L'\\' ;
	}
	m_wstrDstCurrentDir = m_wstrInstDir ;
	m_pdscCurrentDir = m_flstSetup.GetContentTagAs( 0, L"setup" ) ;
	m_iCurrent = 0 ;
	m_fDstArchive = false ;
	m_fSrcArchive = false ;
	if ( m_nTotalInstallationBytes == 0 )
	{
		MeasureInstallSize( m_nTotalInstallationBytes ) ;
	}
	//
	// スレッド開始
	//
	m_hThreadReady = ::CreateEvent( NULL, TRUE, FALSE, NULL ) ;
	m_hFinishedCopy = ::CreateEvent( NULL, TRUE, TRUE, NULL ) ;
	//
	ESLError	err = BeginThread( ) ;
	if ( !err )
	{
		::WaitForSingleObject( m_hThreadReady, INFINITE ) ;
	}
	return	err ;
}

// ファイルのインストール完了か？
//////////////////////////////////////////////////////////////////////////////
bool ECSSetup::IsFinishedInstall( void ) const
{
	if ( m_pdscCurrentDir == NULL )
	{
		return	true ;
	}
	if ( ::WaitForSingleObject( m_hFinishedCopy, 0 ) == WAIT_TIMEOUT )
	{
		return	false ;
	}
	if ( m_iCurrent >= m_pdscCurrentDir->GetContentTagCount() )
	{
		const EDescription *	pdscParent = m_pdscCurrentDir->GetParent( ) ;
		if ( (pdscParent == NULL)
			|| (pdscParent == &m_flstSetup) )
		{
			return	true ;
		}
	}
	return	false ;
}

// ファイルのコピーを開始する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::InstallNextFile
	( EWideString & wstrDstPath,
		EWideString & wstrSrcPath, ECSContext * context )
{
	if ( m_pdscCurrentDir == NULL )
	{
		return	eslErrGeneral ;
	}
	if ( ::WaitForSingleObject( m_hFinishedCopy, 0 ) == WAIT_TIMEOUT )
	{
		return	eslErrGeneral ;
	}
	for ( ; ; )
	{
		while ( m_iCurrent >= m_pdscCurrentDir->GetContentTagCount() )
		{
			//
			// 現在インストールしているディレクトリの終了処理
			//
			if ( FinishInstallDirectory() )
			{
				return	eslErrGeneral ;
			}
		}
		//
		// 次のエントリ
		//
		while ( m_iCurrent < m_pdscCurrentDir->GetContentTagCount() )
		{
			EDescription *	pdscNext =
				m_pdscCurrentDir->GetContentTagAt( m_iCurrent ++ ) ;
			if ( pdscNext == NULL )
			{
				continue ;
			}
			if ( pdscNext->Tag() == L"file" )
			{
				if ( BeginNextCopyFile( pdscNext, context ) )
				{
					return	eslErrGeneral ;
				}
				wstrDstPath = m_wstrDstFile ;
				wstrSrcPath = m_wstrSrcFile ;
				return	eslErrSuccess ;
			}
			else if ( pdscNext->Tag() == L"directory" )
			{
				if ( BeginNextInstallDirectory( pdscNext, context ) )
				{
					return	eslErrGeneral ;
				}
				m_pdscCurrentDir = pdscNext ;
				m_iCurrent = 0 ;
				break ;
			}
		}
	}

	return	eslErrGeneral ;
}

// 現在インストールしているディレクトリの終了処理
//////////////////////////////////////////////////////////////////////////////
bool ECSSetup::FinishInstallDirectory( void )
{
	if ( m_pdscCurrentDir == NULL )
	{
		return	true ;
	}
	if ( m_pdscCurrentDir->GetAttrInteger( L"archive", 0 ) )
	{
		ESLAssert( m_pdstfile == &m_dstarcf ) ;
		if ( m_fDstArchive )
		{
			m_dstarcf.Close( ) ;
		}
		delete	m_dstfile ;
		m_dstfile = NULL ;
		m_pdstfile = NULL ;
		m_fDstArchive = false ;
	}
	else
	{
		if ( m_fDstArchive )
		{
			m_dstarcf.AscendDirectory( ) ;
		}
		else
		{
			delete	m_dstfile ;
			m_dstfile = NULL ;
			m_pdstfile = NULL ;
		}
	}
	unsigned long int	nDstCurDirLen = m_wstrDstCurrentDir.GetLength() ;
	if ( nDstCurDirLen > 0 )
	{
		wchar_t	wchLast =
			m_wstrDstCurrentDir.GetAt( nDstCurDirLen - 1 ) ;
		if ( (wchLast == L'\\') || (wchLast == L'#') )
		{
			m_wstrDstCurrentDir =
				m_wstrDstCurrentDir.Left( nDstCurDirLen - 1 ) ;
		}
	}
	m_wstrDstCurrentDir =
		EFileList::GetFileDirectoryPart( m_wstrDstCurrentDir ) ;
	//
	EDescription *	pdscChild = m_pdscCurrentDir ;
	EDescription *	pdscParent = m_pdscCurrentDir->GetParent( ) ;
	m_iCurrent = 0 ;
	if ( (pdscParent == &m_flstSetup) || (pdscParent == NULL) )
	{
		m_pdscCurrentDir = NULL ;
		m_iCurrent = 0 ;
		return	true ;
	}
	m_pdscCurrentDir = pdscParent ;
	for ( ; m_iCurrent < pdscParent->GetContentTagCount(); m_iCurrent ++ )
	{
		if ( pdscParent->GetContentTagAt( m_iCurrent ) == pdscChild )
		{
			m_iCurrent ++ ;
			break ;
		}
	}
	EWideString	wstrPath = m_pdscCurrentDir->GetAttrString( L"path", NULL ) ;
	if ( (wstrPath.Find( L':' ) < 0)
		&& wstrPath.CompareLeft( L"\\\\" ) )
	{
		m_wstrDstCurrentDir = m_wstrInstDir + wstrPath ;
	}
	else
	{
		m_wstrDstCurrentDir = wstrPath ;
	}
	return	false ;
}

// 次のインストールファイルのコピーを開始する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::BeginNextCopyFile
	( EDescription * pdscNext, ECSContext * context )
{
	//
	// ソースファイルを開く
	//
	m_wstrSrcFile = pdscNext->GetAttrString( L"path", NULL ) ;
	int	iFindArcSep = m_wstrSrcFile.Find( L'#' ) ;
	if ( iFindArcSep >= 0 )
	{
		EWideString	wstrArcFile = m_wstrSrcFile.Left( iFindArcSep ) ;
		EWideString	wstrFilePath = m_wstrSrcFile.Middle( iFindArcSep + 1 ) ;
		if ( !m_fSrcArchive
			|| m_wstrSrcArchivePath.CompareNoCase( wstrArcFile ) )
		{
			m_srcarcf.Close() ;
			m_psrcfile = NULL ;
			delete	m_srcfile ;
			if ( context != NULL )
			{
				m_srcfile = context->OpenFileOnScript( wstrArcFile ) ;
			}
			else
			{
				m_srcfile = new ERawFile ;
				if ( ((ERawFile*)m_srcfile)->Open
					( EString( wstrArcFile ),
						ESLFileObject::modeRead | ESLFileObject::shareRead ) )
				{
					delete	m_srcfile ;
					m_srcfile = NULL ;
				}
			}
			if ( (m_srcfile == NULL) || m_srcarcf.Open( m_srcfile ) )
			{
				delete	m_srcfile ;
				m_srcfile = NULL ;
				m_csStatus.Lock() ;
				m_wstrErrMsg = wstrArcFile + L" を開けませんでした。" ;
				m_csStatus.Unlock() ;
				return	eslErrGeneral ;
			}
			m_wstrSrcArchivePath = wstrArcFile ;
		}
		EString	strPassword =
			pdscNext->GetAttrString( L"password", NULL ) ;
		if ( m_srcarcf.OpenFile
			( EString( wstrFilePath ),
				strPassword, ERISAArchive::otStream ) )
		{
			m_csStatus.Lock() ;
			m_wstrErrMsg = m_wstrSrcFile + L" を開けませんでした。" ;
			m_csStatus.Unlock() ;
			return	eslErrGeneral ;
		}
		m_psrcfile = &m_srcarcf ;
		m_fSrcArchive = true ;
	}
	else
	{
		m_srcarcf.Close( ) ;
		delete	m_srcfile ;
		m_srcfile = NULL ;
		m_psrcfile = NULL ;
		m_fSrcArchive = false ;
		if ( context != NULL )
		{
			m_srcfile = context->OpenFileOnScript( m_wstrSrcFile ) ;
		}
		else
		{
			m_srcfile = new ERawFile ;
			if ( ((ERawFile*)m_srcfile)->Open
				( EString( m_wstrSrcFile ),
					ESLFileObject::modeRead | ESLFileObject::shareRead ) )
			{
				delete	m_srcfile ;
				m_srcfile = NULL ;
			}
		}
		if ( m_srcfile == NULL )
		{
			m_csStatus.Lock() ;
			m_wstrErrMsg = m_wstrSrcFile + L" を開けませんでした。" ;
			m_csStatus.Unlock() ;
			return	eslErrGeneral ;
		}
		m_psrcfile = m_srcfile ;
	}
	//
	// ディスティネーションファイルを開く
	//
	EWideString	wstrFileName = EFileList::GetFileNamePart( m_wstrSrcFile ) ;
	if ( m_fDstArchive )
	{
		if ( m_dstarcf.DescendFile
			( EString( wstrFileName ),
				m_strDstPassword, ERISAArchive::otStream ) )
		{
			m_csStatus.Lock() ;
			m_wstrErrMsg = wstrFileName
							+ L" のアーカイブ化を開始できませんでした。" ;
			m_csStatus.Unlock() ;
			return	eslErrGeneral ;
		}
		m_pdstfile = &m_dstarcf ;
		m_wstrDstFile = m_wstrDstCurrentDir + L"#" + wstrFileName ;
	}
	else
	{
		EWideString	wstrDstPath =
			m_wstrDstCurrentDir.OffsetFilePath( wstrFileName ) ;
		m_dstarcf.Close( ) ;
		delete	m_dstfile ;
		if ( context != NULL )
		{
			m_dstfile =
				context->OpenFileOnScript
					( wstrDstPath, ESLFileObject::modeCreate ) ;
		}
		else
		{
			m_dstfile = new ERawFile ;
			if ( ((ERawFile*)m_dstfile)->Open
				( EString( wstrDstPath ), ESLFileObject::modeCreate ) )
			{
				delete	m_dstfile ;
				m_dstfile = NULL ;
			}
		}
		if ( m_dstfile == NULL )
		{
			m_csStatus.Lock() ;
			m_wstrErrMsg = wstrDstPath + L" を開けませんでした。" ;
			m_csStatus.Unlock() ;
			return	eslErrGeneral ;
		}
		m_flstLog.AddFile
			( wstrDstPath.GetFileDirectoryPart(),
						wstrDstPath.GetFileNamePart() ) ;
		m_pdstfile = m_dstfile ;
		m_wstrDstFile = wstrDstPath ;
	}
	//
	// スレッドの処理を開始する
	//
	::ResetEvent( m_hThreadReady ) ;
	::ResetEvent( m_hFinishedCopy ) ;
	PostThreadMessage( tmBeginCopy, 0, 0 ) ;
	::WaitForSingleObject( m_hThreadReady, INFINITE ) ;
	return	eslErrSuccess ;
}

// 次のインストールディレクトリの準備をする
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::BeginNextInstallDirectory
	( EDescription * pdscNext, ECSContext * context )
{
	EWideString	wstrPath = pdscNext->GetAttrString( L"path", NULL ) ;
	m_wstrDstFile = m_wstrDstCurrentDir.OffsetFilePath( wstrPath ) ;
	m_wstrDstCurrentDir = m_wstrDstFile ;
	if ( pdscNext->GetAttrInteger( L"archive", 0 ) )
	{
		//
		// アーカイブ化の開始処理
		//
		m_strDstPassword = pdscNext->GetAttrString( L"password", NULL ) ;
		m_dwDstEncodeType = pdscNext->GetAttrInteger( L"encoding_type", 0 ) ;
		m_pdstfile = NULL ;
		m_dstarcf.Close( ) ;
		delete	m_dstfile ;
		if ( context != NULL )
		{
			m_dstfile =
				context->OpenFileOnScript
					( m_wstrDstFile, ESLFileObject::modeCreate ) ;
		}
		else
		{
			m_dstfile = new ERawFile ;
			if ( ((ERawFile*)m_dstfile)->Open
				( EString( m_wstrDstFile ), ESLFileObject::modeCreate ) )
			{
				delete	m_dstfile ;
				m_dstfile = NULL ;
			}
		}
		if ( m_dstfile == NULL )
		{
			m_csStatus.Lock() ;
			m_wstrErrMsg = m_wstrDstFile + L" を開けませんでした。" ;
			m_csStatus.Unlock() ;
			return	eslErrGeneral ;
		}
		ERISAArchive::EDirectory	dirList ;
		if ( CreateArchiveDirectory( dirList, pdscNext ) )
		{
			return	eslErrGeneral ;
		}
		if ( m_dstarcf.Open( m_dstfile, &dirList ) )
		{
			delete	m_dstfile ;
			m_dstfile = NULL ;
			m_csStatus.Lock() ;
			m_wstrErrMsg =
				m_wstrDstFile + L" アーカイブを開始できませんでした。" ;
			m_csStatus.Unlock() ;
			return	eslErrGeneral ;
		}
		if ( m_wstrDstCurrentDir.Right(1) != L"#" )
		{
			m_wstrDstCurrentDir += L'#' ;
		}
		//
		m_flstLog.AddFile
			( m_wstrDstFile.GetFileDirectoryPart(),
						m_wstrDstFile.GetFileNamePart() ) ;
		m_pdstfile = &m_dstarcf ;
		m_fDstArchive = true ;
	}
	else if ( m_fDstArchive )
	{
		//
		// アーカイブのディレクトリ作成
		//
		ESLAssert( m_pdstfile == &m_dstarcf ) ;
		ERISAArchive::EDirectory	dirList ;
		if ( CreateArchiveDirectory( dirList, pdscNext ) )
		{
			return	eslErrGeneral ;
		}
		EWideString	wstrDirName =
			EFileList::GetFileNamePart( m_wstrDstFile ) ;
		if ( m_dstarcf.DescendDirectory( EString( wstrDirName ), &dirList ) )
		{
			m_csStatus.Lock() ;
			m_wstrErrMsg =
				L" アーカイブにディレクトリ "
					+ wstrDirName +  L" を作成できませんでした。" ;
			m_csStatus.Unlock() ;
			return	eslErrGeneral ;
		}
	}
	else
	{
		//
		// Windows ファイルシステム上のディレクトリ作成
		//
		if ( m_wstrDstCurrentDir.Right(1) != L"\\" )
		{
			m_wstrDstCurrentDir += L'\\' ;
		}
//		int	nCreated = CreateDirectoryAsFilePath( m_wstrDstCurrentDir ) ;
		InstallCreateDirectory( m_wstrDstCurrentDir, 0 ) ;
		//
		if ( !wstrPath.IsEmpty() )
		{
			EWideString	wstrInstDirOffset = m_wstrDstCurrentDir ;
			if ( !wstrInstDirOffset.CompareLeft( m_wstrInstDir ) )
			{
				wstrInstDirOffset =
					wstrInstDirOffset.Middle( m_wstrInstDir.GetLength() ) ;
			}
			InstallCreateDirectory
				( wstrInstDirOffset.Left
					( wstrInstDirOffset.GetLength() - 1 ), 0 ) ;
		}
		m_fDstArchive = false ;
	}
	return	eslErrSuccess ;
}

// アーカイブ用ディレクトリテーブルを生成する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::CreateArchiveDirectory
	( ERISAArchive::EDirectory & dirList, EDescription * pdscDir )
{
	for ( int i = 0; i < pdscDir->GetContentTagCount(); i ++ )
	{
		EDescription *	pdscTag = pdscDir->GetContentTagAt( i ) ;
		if ( pdscTag == NULL )
		{
			continue ;
		}
		if ( pdscTag->Tag() == L"file" )
		{
			EString	strFileName =
				EFileList::GetFileNamePart
					( pdscTag->GetAttrString( L"path", NULL ) ) ;
			dirList.AddFileEntry
				( strFileName, ERISAArchive::attrNormal, m_dwDstEncodeType ) ;
		}
		else if ( pdscTag->Tag() == L"directory" )
		{
			EString	strFileName =
				EFileList::GetFileNamePart
					( pdscTag->GetAttrString( L"path", NULL ) ) ;
			dirList.AddFileEntry
				( strFileName, ERISAArchive::attrDirectory ) ;
		}
	}
	return	eslErrSuccess ;
}

// 指定パスのディレクトリを作成する
//////////////////////////////////////////////////////////////////////////////
int ECSSetup::CreateDirectoryAsFilePath( const wchar_t * pwszFilePath )
{
	int	i, j = 2 ;
	if ( pwszFilePath == NULL )
	{
		return	0 ;
	}
	for ( i = 0; pwszFilePath[i]; i ++ )
	{
		if ( pwszFilePath[i] == L':' )
		{
			j = i + 3 ;
		}
	}
	if ( j >= i )
	{
		return	0 ;
	}
	int	nCreated = 0 ;
	for ( i = j; pwszFilePath[i]; i ++ )
	{
		if ( (pwszFilePath[i] == L'\\')
			|| (pwszFilePath[i] == L'/') )
		{
			EString	strDirPath( pwszFilePath, i ) ;
			WIN32_FIND_DATA	wfd ;
			HANDLE	hFind = ::FindFirstFile( strDirPath, &wfd ) ;
			if ( hFind == INVALID_HANDLE_VALUE )
			{
				::CreateDirectory( strDirPath, NULL ) ;
				nCreated ++ ;
			}
			else
			{
				::FindClose( hFind ) ;
			}
		}
	}
	return	nCreated ;
}

// ファイルコピーの進行状況を取得する
//////////////////////////////////////////////////////////////////////////////
UINT64 ECSSetup::GetCurrentCopiedBytes( UINT64 & nFileSize )
{
	UINT64	nCurrentBytes ;
	m_csStatus.Lock() ;
	nCurrentBytes = m_nCurrentCopiedBytes ;
	nFileSize = m_nCurrentFileBytes ;
	m_csStatus.Unlock() ;
	return	nCurrentBytes ;
}

// 全体の進行状況を取得する
//////////////////////////////////////////////////////////////////////////////
UINT64 ECSSetup::GetTotalCopiedBytes( UINT64 & nTotalSize )
{
	UINT64	nCurrentBytes ;
	m_csStatus.Lock() ;
	nCurrentBytes = m_nTotalInstalledBytes ;
	nTotalSize = m_nTotalInstallationBytes ;
	m_csStatus.Unlock() ;
	return	nCurrentBytes ;
}

// ファイルコピーの完了を待つ
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::WaitForCurrentCopy
	( DWORD dwTimeout, ECSContext * pContext )
{
	if ( pContext == NULL )
	{
		pContext = ECotophaScript::GetPrimaryContext( ) ;
		if ( pContext == NULL )
		{
			DWORD	dwWaitResult =
				::WaitForSingleObject( m_hFinishedCopy, dwTimeout ) ;
			if ( dwWaitResult == WAIT_TIMEOUT )
			{
				return	eslErrTimeout ;
			}
			if ( dwWaitResult != WAIT_OBJECT_0 )
			{
				return	eslErrGeneral ;
			}
			return	eslErrSuccess ;
		}
	}
	return	pContext->WaitUntilEvent( m_hFinishedCopy, dwTimeout ) ;
}

// ファイルのインストールを終了する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::EndInstall( void )
{
	if ( m_hFinishedCopy != NULL )
	{
		if ( Handle() != NULL )
		{
			PostThreadMessage( tmQuitThread, 0, 0 ) ;
		}
		WaitForCurrentCopy( INFINITE ) ;
		FinishInstallDirectory( ) ;
		//
		if ( Handle() != NULL )
		{
			PostThreadMessage( tmQuitThread, 0, 0 ) ;
			::WaitForSingleObject( Handle(), INFINITE ) ;
			CloseThread( ) ;
		}
		//
		::CloseHandle( m_hFinishedCopy ) ;
		m_hFinishedCopy = NULL ;
	}
	if ( m_hThreadReady != NULL )
	{
		::CloseHandle( m_hThreadReady ) ;
		m_hThreadReady = NULL ;
	}
	if ( m_dstfile != NULL )
	{
		m_dstarcf.Close( ) ;
		delete	m_dstfile ;
		m_dstfile = NULL ;
		m_fDstArchive = false ;
	}
	if ( m_srcfile != NULL )
	{
		m_srcarcf.Close( ) ;
		delete	m_srcfile ;
		m_srcfile = NULL ;
		m_fSrcArchive = false ;
	}
	m_pdstfile = NULL ;
	m_psrcfile = NULL ;
	return	eslErrSuccess ;
}

// ディレクトリを生成する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::InstallCreateDirectory
			( const wchar_t * pwszDirPath, int nCreated )
{
	EWideString	wstrDirPath = pwszDirPath ;
	if ( (wstrDirPath.Find( L':' ) < 0)
		&& wstrDirPath.CompareLeft( L"\\\\" ) )
	{
		wstrDirPath = m_wstrInstDir + pwszDirPath ;
	}
	nCreated += CreateDirectoryAsFilePath( wstrDirPath ) ;
	if ( wstrDirPath.Right(1) != L"\\" )
	{
		::CreateDirectory( EString( wstrDirPath ), NULL ) ;
		nCreated ++ ;
	}
	m_flstLog.AddDirectory( wstrDirPath, nCreated ) ;
	return	eslErrSuccess ;
}

// ショートカットファイルを作成する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::InstallCreateShortcutFile
	( const wchar_t * pwszBaseDir, const wchar_t * pwszName,
		const char * pszLinkFile, const char * pszArg )
{
	EWideString	wstrShortcutFile = pwszBaseDir ;
	if ( wstrShortcutFile.Right(1) != L"\\" )
	{
		wstrShortcutFile += L"\\" ;
	}
	wstrShortcutFile += pwszName ;
	wstrShortcutFile += L".lnk" ;
	//
	ESLError	err =
		CreateShortcutFile( wstrShortcutFile, pszLinkFile, pszArg ) ;
	//
	m_flstLog.AddFile( pwszBaseDir, wstrShortcutFile ) ;
	//
	return	err ;
}

// アンインストール情報を登録する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::RegisterUninstall
	( const wchar_t * pwszRegName,
		const UNINSTALL_INFO & uninst_info )
{
	EString	strRegPath =
		"SOFTWARE\\Microsoft\\Windows\\"
		"CurrentVersion\\Uninstall\\" + EString( pwszRegName ) ;
	ERegistryKey	key ;
	if ( key.CreateKey( HKEY_LOCAL_MACHINE, strRegPath ) )
	{
		return	eslErrGeneral ;
	}
	if ( uninst_info.wstrDisplayName != L"" )
	{
		key.SetString
			( "DisplayName",
				EString( uninst_info.wstrDisplayName ) ) ;
	}
	if ( uninst_info.wstrDisplayIcon != L"" )
	{
		key.SetString
			( "DisplayIcon",
				EString( uninst_info.wstrDisplayIcon ) ) ;
	}
	if ( uninst_info.wstrUninstallCmdLine != L"" )
	{
		key.SetString
			( "UninstallString",
				EString( uninst_info.wstrUninstallCmdLine ) ) ;
	}
	if ( uninst_info.wstrUninstallPath != L"" )
	{
		key.SetString
			( "UninstallPath",
				EString( uninst_info.wstrUninstallPath ) ) ;
	}
	if ( uninst_info.wstrInstallLocation != L"" )
	{
		key.SetString
			( "InstallLocation",
				EString( uninst_info.wstrInstallLocation ) ) ;
	}
	if ( uninst_info.wstrPublisher != L"" )
	{
		key.SetString
			( "Publisher",
				EString( uninst_info.wstrPublisher ) ) ;
	}
	if ( uninst_info.wstrVersionMajor != L"" )
	{
		key.SetString
			( "VersionMajor",
				EString( uninst_info.wstrVersionMajor ) ) ;
	}
	if ( uninst_info.wstrVersionMinor != L"" )
	{
		key.SetString
			( "VersionMinor",
				EString( uninst_info.wstrVersionMinor ) ) ;
	}
	key.CloseKey( ) ;
	return	eslErrSuccess ;
}

// アンインストール情報を取得する
//////////////////////////////////////////////////////////////////////////////
DWORD ECSSetup::GetRegUninstallInteger32
	( const wchar_t * pwszRegName,
		const wchar_t * pwszValueName, DWORD nDefValue )
{
	EString	strRegPath =
		"SOFTWARE\\Microsoft\\Windows\\"
		"CurrentVersion\\Uninstall\\" + EString( pwszRegName ) ;
	ERegistryKey	key ;
	if ( key.OpenKey( HKEY_LOCAL_MACHINE, strRegPath ) )
	{
		return	nDefValue ;
	}
	DWORD	dwValue =
		key.GetInteger( EString( pwszValueName ), nDefValue ) ;
	key.CloseKey( ) ;
	return	dwValue ;
}

INT64 ECSSetup::GetRegUninstallInteger64
	( const wchar_t * pwszRegName,
		const wchar_t * pwszValueName, INT64 nDefValue )
{
	EString	strRegPath =
		"SOFTWARE\\Microsoft\\Windows\\"
		"CurrentVersion\\Uninstall\\" + EString( pwszRegName ) ;
	ERegistryKey	key ;
	if ( key.OpenKey( HKEY_LOCAL_MACHINE, strRegPath ) )
	{
		return	nDefValue ;
	}
	INT64	nValue =
		key.GetInteger64( EString( pwszValueName ), nDefValue ) ;
	key.CloseKey( ) ;
	return	nValue ;
}

EWideString ECSSetup::GetRegUninstallString
	( const wchar_t * pwszRegName,
		const wchar_t * pwszValueName, const wchar_t * pwszDefValue )
{
	EString	strRegPath =
		"SOFTWARE\\Microsoft\\Windows\\"
		"CurrentVersion\\Uninstall\\" + EString( pwszRegName ) ;
	ERegistryKey	key ;
	if ( key.OpenKey( HKEY_LOCAL_MACHINE, strRegPath ) )
	{
		return	pwszDefValue ;
	}
	EWideString	wstrValue =
		key.GetString( EString( pwszValueName ), EString( pwszDefValue ) ) ;
	key.CloseKey( ) ;
	return	wstrValue ;
}

// アンインストール情報に任意に記録する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::SetRegUninstallInteger32
	( const wchar_t * pwszRegName,
		const wchar_t * pwszValueName, DWORD nValue )
{
	EString	strRegPath =
		"SOFTWARE\\Microsoft\\Windows\\"
		"CurrentVersion\\Uninstall\\" + EString( pwszRegName ) ;
	ERegistryKey	key ;
	ESLError	err ;
	err = key.OpenKey( HKEY_LOCAL_MACHINE, strRegPath ) ;
	if ( !err )
	{
		err = key.SetInteger( EString( pwszValueName ), nValue ) ;
		key.CloseKey( ) ;
	}
	return	err ;
}

ESLError ECSSetup::SetRegUninstallInteger64
	( const wchar_t * pwszRegName,
		const wchar_t * pwszValueName, INT64 nValue )
{
	EString	strRegPath =
		"SOFTWARE\\Microsoft\\Windows\\"
		"CurrentVersion\\Uninstall\\" + EString( pwszRegName ) ;
	ERegistryKey	key ;
	ESLError	err ;
	err = key.OpenKey( HKEY_LOCAL_MACHINE, strRegPath ) ;
	if ( !err )
	{
		err = key.SetInteger64( EString( pwszValueName ), nValue ) ;
		key.CloseKey( ) ;
	}
	return	err ;
}

ESLError ECSSetup::SetRegUninstallString
	( const wchar_t * pwszRegName,
		const wchar_t * pwszValueName, const wchar_t * pwszValue )
{
	EString	strRegPath =
		"SOFTWARE\\Microsoft\\Windows\\"
		"CurrentVersion\\Uninstall\\" + EString( pwszRegName ) ;
	ERegistryKey	key ;
	ESLError	err ;
	err = key.OpenKey( HKEY_LOCAL_MACHINE, strRegPath ) ;
	if ( !err )
	{
		err = key.SetString
			( EString( pwszValueName ), EString( pwszValue ) ) ;
		key.CloseKey( ) ;
	}
	return	err ;
}

// ダイアログデータ
//////////////////////////////////////////////////////////////////////////////

#define IDD_INST_DLG                    103
#define IDC_EDIT_INSTR_DIR              1000
#define IDC_BUTTON_BROWSE               1001
#define IDC_CHECK_DESKTOP               1002
#define IDC_CHECK_PROGRAMS              1003
#define IDC_PROGRESS_TOTAL              1004
#define IDC_PROGRESS_FILE               1005
#define IDC_STATIC_FILE                 1006

/*
IDD_INST_DLG DIALOG DISCARDABLE  0, 0, 194, 205
STYLE DS_MODALFRAME | WS_POPUP | WS_CAPTION | WS_SYSMENU
CAPTION "インストール"
FONT 9, "ＭＳ Ｐゴシック"
BEGIN
    EDITTEXT        IDC_EDIT_INSTR_DIR,20,30,130,15,ES_AUTOHSCROLL
    PUSHBUTTON      "参照",IDC_BUTTON_BROWSE,155,30,25,15
    CONTROL         "デスクトップにショートカットを作成する",
                    IDC_CHECK_DESKTOP,"Button",BS_AUTOCHECKBOX | WS_TABSTOP,
                    20,55,140,10
    CONTROL         "スタートメニューに登録する",IDC_CHECK_PROGRAMS,"Button",
                    BS_AUTOCHECKBOX | WS_TABSTOP,20,70,140,10
    DEFPUSHBUTTON   "インストール",IDOK,75,175,50,14
    PUSHBUTTON      "キャンセル",IDCANCEL,130,175,50,14
    GROUPBOX        "インストール先",IDC_STATIC,10,10,175,80
    LTEXT           "全体の進行状況",IDC_STATIC,15,110,55,10
    CONTROL         "Progress1",IDC_PROGRESS_TOTAL,"msctls_progress32",
                    WS_BORDER,20,120,150,10
    LTEXT           "現在のファイル",IDC_STATIC_FILE,15,135,55,10
    CONTROL         "Progress1",IDC_PROGRESS_FILE,"msctls_progress32",
                    WS_BORDER,20,145,150,10
    GROUPBOX        "進行状況",IDC_STATIC,10,95,170,70
END
*/
static const BYTE	bytInstDlgData[614] =
{
	0xC0, 0x00, 0xC8, 0x80, 0x00, 0x00, 0x00, 0x00, 
	0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC2, 0x00, 
	0xCD, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA4, 0x30, 
	0xF3, 0x30, 0xB9, 0x30, 0xC8, 0x30, 0xFC, 0x30, 
	0xEB, 0x30, 0x00, 0x00, 0x09, 0x00, 0x2D, 0xFF, 
	0x33, 0xFF, 0x20, 0x00, 0x30, 0xFF, 0xB4, 0x30, 
	0xB7, 0x30, 0xC3, 0x30, 0xAF, 0x30, 0x00, 0x00, 
	0x80, 0x00, 0x81, 0x50, 0x00, 0x00, 0x00, 0x00, 
	0x14, 0x00, 0x1E, 0x00, 0x82, 0x00, 0x0F, 0x00, 
	0xE8, 0x03, 0xFF, 0xFF, 0x81, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x50, 
	0x00, 0x00, 0x00, 0x00, 0x9B, 0x00, 0x1E, 0x00, 
	0x19, 0x00, 0x0F, 0x00, 0xE9, 0x03, 0xFF, 0xFF, 
	0x80, 0x00, 0xC2, 0x53, 0x67, 0x71, 0x00, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x01, 0x50, 
	0x00, 0x00, 0x00, 0x00, 0x14, 0x00, 0x37, 0x00, 
	0x8C, 0x00, 0x0A, 0x00, 0xEA, 0x03, 0xFF, 0xFF, 
	0x80, 0x00, 0xC7, 0x30, 0xB9, 0x30, 0xAF, 0x30, 
	0xC8, 0x30, 0xC3, 0x30, 0xD7, 0x30, 0x6B, 0x30, 
	0xB7, 0x30, 0xE7, 0x30, 0xFC, 0x30, 0xC8, 0x30, 
	0xAB, 0x30, 0xC3, 0x30, 0xC8, 0x30, 0x92, 0x30, 
	0x5C, 0x4F, 0x10, 0x62, 0x59, 0x30, 0x8B, 0x30, 
	0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x01, 0x50, 
	0x00, 0x00, 0x00, 0x00, 0x14, 0x00, 0x46, 0x00, 
	0x8C, 0x00, 0x0A, 0x00, 0xEB, 0x03, 0xFF, 0xFF, 
	0x80, 0x00, 0xB9, 0x30, 0xBF, 0x30, 0xFC, 0x30, 
	0xC8, 0x30, 0xE1, 0x30, 0xCB, 0x30, 0xE5, 0x30, 
	0xFC, 0x30, 0x6B, 0x30, 0x7B, 0x76, 0x32, 0x93, 
	0x59, 0x30, 0x8B, 0x30, 0x00, 0x00, 0x00, 0x00, 
	0x01, 0x00, 0x01, 0x50, 0x00, 0x00, 0x00, 0x00, 
	0x4B, 0x00, 0xAF, 0x00, 0x32, 0x00, 0x0E, 0x00, 
	0x01, 0x00, 0xFF, 0xFF, 0x80, 0x00, 0xA4, 0x30, 
	0xF3, 0x30, 0xB9, 0x30, 0xC8, 0x30, 0xFC, 0x30, 
	0xEB, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x01, 0x50, 0x00, 0x00, 0x00, 0x00, 
	0x82, 0x00, 0xAF, 0x00, 0x32, 0x00, 0x0E, 0x00, 
	0x02, 0x00, 0xFF, 0xFF, 0x80, 0x00, 0xAD, 0x30, 
	0xE3, 0x30, 0xF3, 0x30, 0xBB, 0x30, 0xEB, 0x30, 
	0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x50, 
	0x00, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x0A, 0x00, 
	0xAF, 0x00, 0x50, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 
	0x80, 0x00, 0xA4, 0x30, 0xF3, 0x30, 0xB9, 0x30, 
	0xC8, 0x30, 0xFC, 0x30, 0xEB, 0x30, 0x48, 0x51, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x50, 
	0x00, 0x00, 0x00, 0x00, 0x0F, 0x00, 0x6E, 0x00, 
	0x37, 0x00, 0x0A, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 
	0x82, 0x00, 0x68, 0x51, 0x53, 0x4F, 0x6E, 0x30, 
	0x32, 0x90, 0x4C, 0x88, 0xB6, 0x72, 0xC1, 0x6C, 
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x50, 
	0x00, 0x00, 0x00, 0x00, 0x14, 0x00, 0x78, 0x00, 
	0x96, 0x00, 0x0A, 0x00, 0xEC, 0x03, 0x6D, 0x00, 
	0x73, 0x00, 0x63, 0x00, 0x74, 0x00, 0x6C, 0x00, 
	0x73, 0x00, 0x5F, 0x00, 0x70, 0x00, 0x72, 0x00, 
	0x6F, 0x00, 0x67, 0x00, 0x72, 0x00, 0x65, 0x00, 
	0x73, 0x00, 0x73, 0x00, 0x33, 0x00, 0x32, 0x00, 
	0x00, 0x00, 0x50, 0x00, 0x72, 0x00, 0x6F, 0x00, 
	0x67, 0x00, 0x72, 0x00, 0x65, 0x00, 0x73, 0x00, 
	0x73, 0x00, 0x31, 0x00, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x02, 0x50, 0x00, 0x00, 0x00, 0x00, 
	0x0F, 0x00, 0x87, 0x00, 0x9B, 0x00, 0x08, 0x00, 
	0xEE, 0x03, 0xFF, 0xFF, 0x82, 0x00, 0xFE, 0x73, 
	0x28, 0x57, 0x6E, 0x30, 0xD5, 0x30, 0xA1, 0x30, 
	0xA4, 0x30, 0xEB, 0x30, 0x00, 0x00, 0x00, 0x00, 
	0x00, 0x00, 0x80, 0x50, 0x00, 0x00, 0x00, 0x00, 
	0x14, 0x00, 0x91, 0x00, 0x96, 0x00, 0x0A, 0x00, 
	0xED, 0x03, 0x6D, 0x00, 0x73, 0x00, 0x63, 0x00, 
	0x74, 0x00, 0x6C, 0x00, 0x73, 0x00, 0x5F, 0x00, 
	0x70, 0x00, 0x72, 0x00, 0x6F, 0x00, 0x67, 0x00, 
	0x72, 0x00, 0x65, 0x00, 0x73, 0x00, 0x73, 0x00, 
	0x33, 0x00, 0x32, 0x00, 0x00, 0x00, 0x50, 0x00, 
	0x72, 0x00, 0x6F, 0x00, 0x67, 0x00, 0x72, 0x00, 
	0x65, 0x00, 0x73, 0x00, 0x73, 0x00, 0x31, 0x00, 
	0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x00, 0x50, 
	0x00, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x5F, 0x00, 
	0xAA, 0x00, 0x46, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 
	0x80, 0x00, 0x32, 0x90, 0x4C, 0x88, 0xB6, 0x72, 
	0xC1, 0x6C, 0x00, 0x00, 0x00, 0x00
} ;

static INT_PTR CALLBACK InstallationDialogProc
	( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	ECSSetup::INST_DLG_PARAM *
		param = (ECSSetup::INST_DLG_PARAM*)
			::GetWindowLong( hwndDlg, DWL_USER ) ;
	switch ( uMsg )
	{
	case	WM_COMMAND:
		switch ( LOWORD( wParam ) )
		{
		case	IDC_BUTTON_BROWSE:
			if ( param != NULL )
			{
				EString	strBuf ;
				::GetDlgItemText
					( hwndDlg, IDC_EDIT_INSTR_DIR,
						strBuf.GetBuffer(0x400), 0x400 ) ;
				strBuf.ReleaseBuffer( ) ;
				param->wstrInstDir = strBuf ;
				//
				ECSSetup::BrowseForFolder
					( param->wstrInstDir,
						L"インストール先", NULL, hwndDlg ) ;
				//
				if ( !param->wstrDefSubDir.IsEmpty() )
				{
					param->wstrInstDir =
						EWideString(param->wstrInstDir).
								OffsetFilePath( param->wstrDefSubDir ) ;
				}
				::SetDlgItemText
					( hwndDlg, IDC_EDIT_INSTR_DIR,
						EString( param->wstrInstDir ) ) ;
			}
			break ;

		case	IDOK:
			if ( param != NULL )
			{
				if ( IsDlgButtonChecked( hwndDlg, IDC_CHECK_DESKTOP ) )
				{
					param->dwOptionFlags |=
						ECSSetup::instoptShortCutDesktop ;
				}
				else
				{
					param->dwOptionFlags &=
						~ECSSetup::instoptShortCutDesktop ;
				}
				if ( IsDlgButtonChecked( hwndDlg, IDC_CHECK_PROGRAMS ) )
				{
					param->dwOptionFlags |=
						ECSSetup::instoptShortCutPrograms ;
				}
				else
				{
					param->dwOptionFlags &=
						~ECSSetup::instoptShortCutPrograms ;
				}
				EString	strBuf ;
				::GetDlgItemText
					( hwndDlg, IDC_EDIT_INSTR_DIR,
						strBuf.GetBuffer(0x400), 0x400 ) ;
				strBuf.ReleaseBuffer( ) ;
				param->wstrInstDir = strBuf ;
				//
				::EnableWindow
					( ::GetDlgItem( hwndDlg, IDC_BUTTON_BROWSE ), FALSE ) ;
				::EnableWindow
					( ::GetDlgItem( hwndDlg, IDC_EDIT_INSTR_DIR ), FALSE ) ;
				::EnableWindow
					( ::GetDlgItem( hwndDlg, IDC_CHECK_DESKTOP ), FALSE ) ;
				::EnableWindow
					( ::GetDlgItem( hwndDlg, IDC_CHECK_PROGRAMS ), FALSE ) ;
				::EnableWindow
					( ::GetDlgItem( hwndDlg, IDOK ), FALSE ) ;
				//
				param->nEventResult = 0 ;
				::SetEvent( param->hEventOkCancel ) ;
			}
			break ;

		case	IDCANCEL:
			if ( param != NULL )
			{
				param->nEventResult = 1 ;
				::SetEvent( param->hEventOkCancel ) ;
			}
			break ;
		}
		break ;

	case	WM_INITDIALOG:
		::SetWindowLong( hwndDlg, DWL_USER, lParam ) ;
		param = (ECSSetup::INST_DLG_PARAM*) lParam ;
		if ( param != NULL )
		{
			RECT	rctParent ;
			if ( (param->pWindow != NULL)
				&& (param->pWindow->GetWindow() != NULL) )
			{
				EGameWindow *	pWnd = param->pWindow->GetWindow( ) ;
				pWnd->GetWindowRect( &rctParent ) ;
			}
			else
			{
				rctParent.left = 0 ;
				rctParent.top = 0 ;
				rctParent.right = ::GetSystemMetrics( SM_CXSCREEN ) ;
				rctParent.bottom = ::GetSystemMetrics( SM_CYSCREEN ) ;
			}
			RECT	rctDlg ;
			::GetWindowRect( hwndDlg, &rctDlg ) ;
			::SetWindowPos
				( hwndDlg, NULL,
					((rctParent.right - rctParent.left)
						- (rctDlg.right - rctDlg.left)) / 2 + rctParent.left,
					((rctParent.bottom - rctParent.top)
						- (rctDlg.bottom - rctDlg.top)) / 2 + rctParent.top,
					0, 0, SWP_NOSIZE | SWP_NOZORDER ) ;
			//
			if ( !param->wstrCaption.IsEmpty() )
			{
				::SetWindowText
					( hwndDlg, EString( param->wstrCaption ) ) ;
			}
			//
			::SetDlgItemText
				( hwndDlg, IDC_EDIT_INSTR_DIR,
					EString( param->wstrInstDir ) ) ;
			::CheckDlgButton
				( hwndDlg, IDC_CHECK_DESKTOP,
					(param->dwOptionFlags
						& ECSSetup::instoptShortCutDesktop) ?
									BST_CHECKED : BST_UNCHECKED ) ;
			::CheckDlgButton
				( hwndDlg, IDC_CHECK_PROGRAMS,
					(param->dwOptionFlags
						& ECSSetup::instoptShortCutPrograms) ?
									BST_CHECKED : BST_UNCHECKED ) ;
			//
			if ( param->dwOptionFlags & ECSSetup::instoptDisableDesktop )
			{
				::EnableWindow
					( ::GetDlgItem( hwndDlg, IDC_CHECK_DESKTOP ), FALSE ) ;
			}
			//
			if ( param->dwOptionFlags & ECSSetup::instoptDisablePrograms )
			{
				::EnableWindow
					( ::GetDlgItem( hwndDlg, IDC_CHECK_PROGRAMS ), FALSE ) ;
			}
			//
			if ( param->dwOptionFlags & ECSSetup::instoptDisableInstDir )
			{
				::EnableWindow
					( ::GetDlgItem( hwndDlg, IDC_EDIT_INSTR_DIR ), FALSE ) ;
				::EnableWindow
					( ::GetDlgItem( hwndDlg, IDC_BUTTON_BROWSE ), FALSE ) ;
			}
			//
			::SendMessage(
				::GetDlgItem( hwndDlg, IDC_PROGRESS_TOTAL ),
						PBM_SETRANGE, 0, MAKELPARAM(0,1000) ) ;
			::SendMessage(
				::GetDlgItem( hwndDlg, IDC_PROGRESS_FILE ),
						PBM_SETRANGE, 0, MAKELPARAM(0,1000) ) ;
		}
		break ;
	}
	return	0 ;
}

static LRESULT __stdcall WindowProcedureCreateInstDlg( void * pInstance )
{
	ECSSetup::INST_DLG_PARAM *
		param = (ECSSetup::INST_DLG_PARAM*) pInstance ;
	HWND	hParentWnd = NULL ;
	if ( param->pWindow != NULL )
	{
		EGameWindow *	pWindow = param->pWindow->GetWindow( ) ;
		if ( pWindow != NULL )
		{
			hParentWnd = *pWindow ;
		}
	}
	param->hDlg = ::CreateDialogIndirectParam
		( ::GetModuleHandle( NULL ),
			(LPCDLGTEMPLATE) bytInstDlgData,
			hParentWnd, InstallationDialogProc, (LPARAM) param ) ;
	if ( param->hDlg != NULL )
	{
		if ( hParentWnd != NULL )
		{
			::EnableWindow( hParentWnd, FALSE ) ;
		}
		::ShowWindow( param->hDlg, SW_SHOW ) ;
		::SetEvent( param->hEventReady ) ;
	}
	else
	{
		param->nEventResult = -1 ;
		::SetEvent( param->hEventOkCancel ) ;
		::SetEvent( param->hEventReady ) ;
	}
	return	0 ;
}

static LRESULT __stdcall WindowProcedureDestroyInstDlg( void * pInstance )
{
	ECSSetup::INST_DLG_PARAM *
		param = (ECSSetup::INST_DLG_PARAM*) pInstance ;
	if ( param->hDlg != NULL )
	{
		::DestroyWindow( param->hDlg ) ;
	}
	::SetEvent( param->hEventReady ) ;
	return	0 ;
}

struct	INST_MSGBOX_INFO
{
	HWND	hwndParent ;
	EString	strText ;
	EString	strCaption ;
	int		nMBType ;
	int		nResult ;
	HANDLE	hDone ;
} ;

static LRESULT __stdcall WindowProcedureMessageBox( void * pInstance )
{
	INST_MSGBOX_INFO *	pmbi = (INST_MSGBOX_INFO*) pInstance ;
	pmbi->nResult =
		::MessageBox
			( pmbi->hwndParent, pmbi->strText,
					pmbi->strCaption, pmbi->nMBType ) ;
	::SetEvent( pmbi->hDone ) ;
	return	0 ;
}

static void HandleInstDlgMessageLoop( void )
{
	MSG		msg ;
	DWORD	dwTimeoutTime = ::GetCurrentTime() + 33 ;
	while ( ::PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) )
	{
		if ( ::GetMessage( &msg, NULL, 0, 0 ) )
		{
			::TranslateMessage( &msg ) ;
			::DispatchMessage( &msg ) ;
		}
		else
		{
			return ;
		}
		if ( (signed int)(msg.time - dwTimeoutTime) >= 0 )
		{
			return ;
		}
	}
}


// インストールダイアログインターフェース
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::CreateInstallationDialog
	( EWideString & wstrInitDir, DWORD & dwOptionFlags,
		const wchar_t * pwszCaption, ECSWindow * pWindow,
		const wchar_t * pwszDefSubDir )
{
	if ( m_pInstDlgParam != NULL )
	{
		return	eslErrGeneral ;
	}
	m_pInstDlgParam = new INST_DLG_PARAM ;
	m_pInstDlgParam->pSetup = this ;
	m_pInstDlgParam->pWindow = pWindow ;
	m_pInstDlgParam->hEventOkCancel =
			::CreateEvent( NULL, TRUE, FALSE, NULL ) ;
	m_pInstDlgParam->hEventReady =
			::CreateEvent( NULL, TRUE, FALSE, NULL ) ;
	m_pInstDlgParam->nEventResult = 0 ;
	m_pInstDlgParam->hDlg = NULL ;
	m_pInstDlgParam->wstrInstDir = wstrInitDir ;
	if ( pwszDefSubDir != NULL )
	{
		m_pInstDlgParam->wstrDefSubDir = pwszDefSubDir ;
	}
	m_pInstDlgParam->wstrCaption = pwszCaption ;
	m_pInstDlgParam->dwOptionFlags = dwOptionFlags ;
	//
	if ( pWindow != NULL )
	{
		if ( pWindow->ProcedureOnWindowThread
			( WindowProcedureCreateInstDlg, m_pInstDlgParam, NULL, true ) )
		{
			return	eslErrGeneral ;
		}
		WaitForSingleObject( m_pInstDlgParam->hEventOkCancel, INFINITE ) ;
	}
	else
	{
		WindowProcedureCreateInstDlg( m_pInstDlgParam ) ;
		//
		for ( ; ; )
		{
			MSG	msg ;
			if ( ::PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
			{
				::TranslateMessage( &msg ) ;
				::DispatchMessage( &msg ) ;
			}
			else
			{
				if ( WaitForSingleObject
					( m_pInstDlgParam->hEventOkCancel, 33 ) == WAIT_OBJECT_0 )
				{
					break ;
				}
			}
		}
	}
	if ( m_pInstDlgParam->nEventResult == 0 )
	{
		wstrInitDir = m_pInstDlgParam->wstrInstDir ;
		dwOptionFlags = m_pInstDlgParam->dwOptionFlags ;
		return	eslErrSuccess ;
	}
	return	eslErrGeneral ;
}

// ダイアログを閉じる
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::CloseInstallationDialog( void )
{
	if ( m_pInstDlgParam != NULL )
	{
		if ( ::IsWindow( m_pInstDlgParam->hDlg ) )
		{
			if ( m_pInstDlgParam->pWindow != NULL )
			{
				::ResetEvent( m_pInstDlgParam->hEventReady ) ;
				if ( !m_pInstDlgParam->pWindow->ProcedureOnWindowThread
					( WindowProcedureDestroyInstDlg,
						m_pInstDlgParam, NULL, true ) )
				{
					::WaitForSingleObject
						( m_pInstDlgParam->hEventReady, INFINITE ) ;
				}
				EGameWindow *	pWindow =
					m_pInstDlgParam->pWindow->GetWindow( ) ;
				if ( pWindow != NULL )
				{
					::EnableWindow( *pWindow, TRUE ) ;
				}
			}
			else
			{
				DestroyWindow( m_pInstDlgParam->hDlg ) ;
			}
		}
		::CloseHandle( m_pInstDlgParam->hEventOkCancel ) ;
		::CloseHandle( m_pInstDlgParam->hEventReady ) ;
		delete	m_pInstDlgParam ;
		m_pInstDlgParam = NULL ;
	}
	return	eslErrSuccess ;
}

// インストールダイアログのキャンセル・閉じるボタンが押されたか調べる
//////////////////////////////////////////////////////////////////////////////
bool ECSSetup::IsInstallationDialogCanceled( void ) const
{
	if ( m_pInstDlgParam != NULL )
	{
		return	(m_pInstDlgParam->nEventResult == 1) ;
	}
	return	false ;
}

// インストール中のファイル名を設定する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::SetInstallationDialogFileText
			( const wchar_t * pwszFileText )
{
	if ( m_pInstDlgParam != NULL )
	{
		if ( m_pInstDlgParam->pWindow == NULL )
		{
			HandleInstDlgMessageLoop( ) ;
		}
		::SetDlgItemText
			( m_pInstDlgParam->hDlg, IDC_STATIC_FILE, EString( pwszFileText ) ) ;
	}
	return	eslErrSuccess ;
}

// 進行状況を更新する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::SetInstallationDialogProgress( int nFile, int nTotal )
{
	if ( m_pInstDlgParam != NULL )
	{
		if ( m_pInstDlgParam->pWindow == NULL )
		{
			HandleInstDlgMessageLoop( ) ;
		}
		::SendMessage(
			::GetDlgItem( m_pInstDlgParam->hDlg,
					IDC_PROGRESS_TOTAL ), PBM_SETPOS, nTotal, 0 ) ;
		::SendMessage(
			::GetDlgItem( m_pInstDlgParam->hDlg,
					IDC_PROGRESS_FILE ), PBM_SETPOS, nFile, 0 ) ;
	}
	return	eslErrSuccess ;
}

// メッセージボックスを表示する
//////////////////////////////////////////////////////////////////////////////
int ECSSetup::InstallationMessageBox
	( const wchar_t * pwszText, const wchar_t * pwszCaption,
							int nMBType, ECSWindow * pParentWnd )
{
	HWND	hwndParent = NULL ;
	if ( (m_pInstDlgParam != NULL) && (pParentWnd == NULL) )
	{
		pParentWnd = m_pInstDlgParam->pWindow ;
	}
	if ( (pParentWnd != NULL) && (pParentWnd->GetWindow() != NULL) )
	{
		hwndParent = *(pParentWnd->GetWindow()) ;
	}
	else
	{
		pParentWnd = NULL ;
	}
	if ( m_pInstDlgParam != NULL )
	{
		if ( m_pInstDlgParam->pWindow == pParentWnd )
		{
			hwndParent = m_pInstDlgParam->hDlg ;
		}
	}
	if ( pParentWnd != NULL )
	{
		INST_MSGBOX_INFO	imbi ;
		imbi.hwndParent = hwndParent ;
		imbi.strText = pwszText ;
		imbi.strCaption = pwszCaption ;
		imbi.nMBType = nMBType ;
		imbi.nResult = IDOK ;
		imbi.hDone = ::CreateEvent( NULL, TRUE, FALSE, NULL ) ;
		//
		if ( !pParentWnd->ProcedureOnWindowThread
			( WindowProcedureMessageBox, &imbi, NULL, true ) )
		{
			::WaitForSingleObject( imbi.hDone, INFINITE ) ;
		}
		//
		::CloseHandle( imbi.hDone ) ;
		return	imbi.nResult ;
	}
	else
	{
		return	::MessageBox
			( hwndParent, EString( pwszText ),
					EString( pwszCaption ), nMBType ) ;
	}
}

// インストールしたファイルをアンインストールする
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Uninstall( const wchar_t * pwszInstallDir )
{
	EDescription *	pdscLog = m_flstLog.GetContentTagAs( 0, L"setup" ) ;
	m_wstrInstDir = pwszInstallDir ;
	if ( m_wstrInstDir.Right(1) != L"\\" )
	{
		m_wstrInstDir += L'\\' ;
	}
	m_fRebootToDelete = false ;
	return	DeleteLogFiles( m_wstrInstDir, pdscLog ) ;
}

// ログのファイルを削除する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::DeleteLogFiles
	( const wchar_t * pwszBaseDir, EDescription * pdscDir )
{
	if ( pdscDir == NULL )
	{
		return	eslErrSuccess ;
	}
	EWideString	wstrBaseDir = pwszBaseDir ;
	wstrBaseDir = wstrBaseDir.OffsetFilePath
				( pdscDir->GetAttrString( L"path", NULL ) ) ;
	//
	for ( int i = 0; i < pdscDir->GetContentTagCount(); i ++ )
	{
		EDescription *	pdscTag = pdscDir->GetContentTagAt( i ) ;
		if ( pdscTag == NULL )
		{
			continue ;
		}
		EWideString	wstrFilePath =
				wstrBaseDir.OffsetFilePath
					( pdscTag->GetAttrString( L"path", NULL ) ) ;
		if ( pdscTag->Tag() == L"file" )
		{
			DeleteInstalledFile( EString( wstrFilePath ) ) ;
		}
		else if ( pdscTag->Tag() == L"directory" )
		{
			DeleteLogFiles( wstrBaseDir, pdscTag ) ;
			//
			if ( wstrFilePath.Right(1) == L"\\" )
			{
				wstrFilePath =
					wstrFilePath.Left( wstrFilePath.GetLength() - 1 ) ;
			}
			if ( pdscTag->GetAttrInteger( L"created", 1 ) )
			{
				EString	strDirPath = wstrFilePath ;
				::RemoveDirectory( strDirPath ) ;
				//
				EString	strStartMenuDir = GetStartMenuDirectory() ;
				if ( strDirPath.Left
					( strStartMenuDir.GetLength() ).
						CompareNoCase( strStartMenuDir ) == 0 )
				{
					EString	strCommonStartMenuDir
									= GetCommonStartMenuDirectory() ;
					if ( !strCommonStartMenuDir.IsEmpty() )
					{
						::RemoveDirectory
							( strCommonStartMenuDir
								+ strDirPath.Middle
									( strStartMenuDir.GetLength() ) ) ;
					}
				}
			}
		}
	}
	return	eslErrSuccess ;
}

// ファイルを削除する
//////////////////////////////////////////////////////////////////////////////
void ECSSetup::DeleteInstalledFile( const char * pszFilePath )
{
	if ( !::DeleteFile( pszFilePath ) )
	{
		m_fRebootToDelete = true ;
		//
		bool	fWindowsNT = false ;
		#if	_MSC_VER >= 1800
			fWindowsNT = true ;
		#else
			OSVERSIONINFO	osvi ;
			osvi.dwOSVersionInfoSize = sizeof( OSVERSIONINFO ) ;
			if ( ::GetVersionEx( &osvi ) )
			{
				if ( osvi.dwPlatformId == VER_PLATFORM_WIN32_NT )
				{
					fWindowsNT = true ;
				}
			}
		#endif
		if ( fWindowsNT )
		{
			::MoveFileEx
				( pszFilePath,
					NULL, MOVEFILE_DELAY_UNTIL_REBOOT ) ;
		}
		else
		{
			EString	strWinIni, strShortPath ;
			::GetWindowsDirectory
				( strWinIni.GetBuffer(0x400), 0x400 ) ;
			strWinIni.ReleaseBuffer( ) ;
			if ( strWinIni.Right(1) != "\\" )
			{
				strWinIni += "\\" ;
			}
			strWinIni += "WININIT.INI" ;
			//
			::GetShortPathName
				( pszFilePath, strShortPath.GetBuffer(0x400), 0x400 ) ;
			strShortPath.ReleaseBuffer( ) ;
			//
			::WritePrivateProfileString
				( "Rename", "NUL", strShortPath, strWinIni ) ;
		}
	}
}

// アンインストール情報を削除する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::UnRegisterUninstall
	( const wchar_t * pwszRegName )
{
	EString	strRegPath =
		"SOFTWARE\\Microsoft\\Windows\\"
		"CurrentVersion\\Uninstall\\" + EString( pwszRegName ) ;
	return	ERegistryKey::DeleteKey( HKEY_LOCAL_MACHINE, strRegPath ) ;
}


// オブジェクトの型名を取得する
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSSetup::GetTypeName( void ) const
{
	return	L"Setup" ;
}

ECSObject * ECSSetup::GetTypeOf( const wchar_t * pwszTypeName )
{
	if ( EWideString::Compare( GetTypeName(), pwszTypeName ) == 0 )
	{
		return	this ;
	}
	return	ECSObject::GetTypeOf( pwszTypeName ) ;
}

// オブジェクトを複製
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSSetup::Duplicate( void )
{
	return	new ECSSetup ;
}

// オブジェクトを代入
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Move( ECSContext & context, ECSObject * obj )
{
	return	ESLErrorMsg( "定義されていない Setup 型への代入です" ) ;
}

// 単項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::UnaryOperate
	( ECSContext & context, CSUnaryOperatorType csuopType )
{
	return	ESLErrorMsg( "Setup 型の定義されていない単項演算子です" ) ;
}

//////////////////////////////////////////////////////////////////////////////
// 二項演算子
ESLError ECSSetup::Operate
	( ECSContext & context, CSOperatorType csopType, ECSObject * obj )
{
	return	ESLErrorMsg( "Setup 型の定義されていない演算子です" ) ;
}

// 比較演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Compare
	( ECSContext & context, int & nResult,
		CSCompareType cscpType, ECSObject & obj )
{
	return	ESLErrorMsg( "Setup 型の定義されていない比較演算子です" ) ;
}

// メンバ関数インデックス取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::GetFunction
	( ECSContext & context, int & nIndex, const wchar_t * pwszName )
{
	nIndex = m_staFuncName->FindIndex( pwszName ) ;
	if ( nIndex < 0 )
	{
		return	ESLErrorMsg( "定義されていない関数を呼び出しています。" ) ;
	}
	return	eslErrSuccess ;
}

// メンバ関数呼び出し
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::CallFunction
	( ECSContext & context,
		int nIndex, ECSObjArray<ECSObject> & lstArg )
{
	if ( (unsigned int) nIndex >= m_staFuncName->GetSize() )
	{
		return	ESLErrorMsg( "不正な関数を呼び出そうとしました。" ) ;
	}
	return	(this->*m_pfnCallFunc[nIndex])( context, lstArg ) ;
}

// データを保存
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Save( ESLFileObject & file, ECSContext & context )
{
	return	eslErrSuccess ;
}

// データを復元
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Load( ESLFileObject & file, ECSContext & context )
{
	return	eslErrSuccess ;
}

// データをダンプ
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::DumpObject
	( EStreamBuffer & buf, int nIndent, ECSContext & context )
{
	return	eslErrSuccess ;
}

// メンバ関数リスト
//////////////////////////////////////////////////////////////////////////////
ECSStrTagArray *	ECSSetup::m_staFuncName = NULL ;
const wchar_t *	ECSSetup::m_pwszFuncName[61] =
{
	// ユーザーインターフェース関数
	L"CreateInstallationDialog",
	L"CloseInstallationDialog",
	L"IsInstallationDialogCanceled",
	L"SetInstallationDialogFileText",
	L"SetInstallationDialogProgress",
	L"InstallationMessageBox",
	// インストール支援関数
	L"GetFontList",
	L"GetWindowsProductID",	L"MakeMD5Digest",
	L"CalcCRC32",	L"CheckSum32",
	L"GetDesktopDirectory",	L"GetStartMenuDirectory",
	L"GetAppDataDirectory",	L"GetWindowsDirectory",
	L"GetCurrentModulePath",	L"GetEnvironmentVariable",
	L"FilterEnvironmentPath",
	L"GetDiskVolumeName",	L"GetDiskSerialNumber",
	L"GetDiskFreeSpace",	L"ShellExecute",
	L"ExecuteProcess",	L"GetExecuteExitCode",
	L"BrowseForFolder", L"BrowseFileDialog",
	// インストールファイルリスト関数
	L"ReadInstalledLog",	L"WriteInstalledLog",
	L"MeasureInstallSize",	L"AddInstallDirectory",
	L"AddInstallArchiveDirectory",	L"AddInstallFile",
	L"AddInstallArchiveTree",	L"AddInstallDirectoryTree",
	// インストール処理関数
	L"BootCheck",	L"ReleaseBootCheck",
	L"GetLastErrorMsg",	L"BeginInstall",
	L"IsFinishedInstall",	L"InstallNextFile",
	L"GetCurrentCopiedBytes",	L"GetTotalCopiedBytes",
	L"WaitForCurrentCopy",	L"EndInstall",
	L"AddInstallFileLog",
	L"InstallCreateDirectory",	L"InstallCreateShortcutFile",
	// レジストリ関数
	L"GetUninstallInfo",	L"RegisterUninstall",
	L"GetRegUninstallInteger32",	L"GetRegUninstallInteger64",
	L"GetRegUninstallString",	L"SetRegUninstallInteger32",
	L"SetRegUninstallInteger64",	L"SetRegUninstallString",
	// アンインストール関数
	L"Uninstall",	L"DeleteInstalledFile",
	L"IsNecessaryRebootToDelete", L"UnRegisterUninstall",
	L"RebootWindows",
	NULL
} ;
const ECSSetup::PFUNC_CALL	ECSSetup::m_pfnCallFunc[60] =
{
	// ユーザーインターフェース関数
	&ECSSetup::Call_CreateInstallationDialog,
	&ECSSetup::Call_CloseInstallationDialog,
	&ECSSetup::Call_IsInstallationDialogCanceled,
	&ECSSetup::Call_SetInstallationDialogFileText,
	&ECSSetup::Call_SetInstallationDialogProgress,
	&ECSSetup::Call_InstallationMessageBox,
	// インストール支援関数
	&ECSSetup::Call_GetFontList,
	&ECSSetup::Call_GetWindowsProductID,
	&ECSSetup::Call_MakeMD5Digest,
	&ECSSetup::Call_CalcCRC32,
	&ECSSetup::Call_CheckSum32,
	&ECSSetup::Call_GetDesktopDirectory,
	&ECSSetup::Call_GetStartMenuDirectory,
	&ECSSetup::Call_GetAppDataDirectory,
	&ECSSetup::Call_GetWindowsDirectory,
	&ECSSetup::Call_GetCurrentModulePath,
	&ECSSetup::Call_GetEnvironmentVariable,
	&ECSSetup::Call_FilterEnvironmentPath,
	&ECSSetup::Call_GetDiskVolumeName,
	&ECSSetup::Call_GetDiskSerialNumber,
	&ECSSetup::Call_GetDiskFreeSpace,
	&ECSSetup::Call_ShellExecute,
	&ECSSetup::Call_ExecuteProcess,
	&ECSSetup::Call_GetExecuteExitCode,
	&ECSSetup::Call_BrowseForFolder,
	&ECSSetup::Call_BrowseFileDialog,
	// インストールファイルリスト関数
	&ECSSetup::Call_ReadInstalledLog,
	&ECSSetup::Call_WriteInstalledLog,
	&ECSSetup::Call_MeasureInstallSize,
	&ECSSetup::Call_AddInstallDirectory,
	&ECSSetup::Call_AddInstallArchiveDirectory,
	&ECSSetup::Call_AddInstallFile,
	&ECSSetup::Call_AddInstallArchiveTree,
	&ECSSetup::Call_AddInstallDirectoryTree,
	// インストール処理関数
	&ECSSetup::Call_BootCheck,
	&ECSSetup::Call_ReleaseBootCheck,
	&ECSSetup::Call_GetLastErrorMsg,
	&ECSSetup::Call_BeginInstall,
	&ECSSetup::Call_IsFinishedInstall,
	&ECSSetup::Call_InstallNextFile,
	&ECSSetup::Call_GetCurrentCopiedBytes,
	&ECSSetup::Call_GetTotalCopiedBytes,
	&ECSSetup::Call_WaitForCurrentCopy,
	&ECSSetup::Call_EndInstall,
	&ECSSetup::Call_AddInstallFileLog,
	&ECSSetup::Call_InstallCreateDirectory,
	&ECSSetup::Call_InstallCreateShortcutFile,
	// レジストリ関数
	&ECSSetup::Call_GetUninstallInfo,
	&ECSSetup::Call_RegisterUninstall,
	&ECSSetup::Call_GetRegUninstallInteger32,
	&ECSSetup::Call_GetRegUninstallInteger64,
	&ECSSetup::Call_GetRegUninstallString,
	&ECSSetup::Call_SetRegUninstallInteger32,
	&ECSSetup::Call_SetRegUninstallInteger64,
	&ECSSetup::Call_SetRegUninstallString,
	// アンインストール関数
	&ECSSetup::Call_Uninstall,
	&ECSSetup::Call_DeleteInstalledFile,
	&ECSSetup::Call_IsNecessaryRebootToDelete,
	&ECSSetup::Call_UnRegisterUninstall,
	&ECSSetup::Call_RebootWindows,
} ;

// インストール支援関数
//////////////////////////////////////////////////////////////////////////////

// メンバ関数 : Integer CreateInstallationDialog
//		( String sInstDir [, Integer nOptionFlags
//			[, String sCaption[, Window window [, String sDefSubDir]]]] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_CreateInstallationDialog
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2, 6 ) ;
	if ( err )
	{
		return	err ;
	}
	ECSString *	pstrInstDir =
		ESLTypeCast<ECSString>
			( context.GetArgumentObjectAs( lstArg, 1, L"String" ) ) ;
	if ( pstrInstDir == NULL )
	{
		return	ESLErrorMsg
			( "CreateInstallationDialog : "
				"1つ目の引数が String オブジェクトではありません。" ) ;
	}
	DWORD	dwOptions = 0 ;
	ECSInteger *	pintOptions =
		ESLTypeCast<ECSInteger>
			( context.GetArgumentObjectAs( lstArg, 2, L"Integer" ) ) ;
	if ( pintOptions == NULL )
	{
		int	nOptions ;
		err = context.GetArgumentAsInt( nOptions, lstArg, 2, 0 ) ;
		if ( err )
		{
			return	err ;
		}
		dwOptions = nOptions ;
	}
	else
	{
		dwOptions = pintOptions->GetInt( ) ;
	}
	EWideString	wstrCaption ;
	err = context.GetArgumentAsStr( wstrCaption, lstArg, 3, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	ECSWindow *	pWindow =
		ESLTypeCast<ECSWindow>
			( context.GetArgumentObjectAs( lstArg, 4, L"Window" ) ) ;
	//
	EWideString	wstrDefSubDir ;
	err = context.GetArgumentAsStr( wstrDefSubDir, lstArg, 5, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	//
	err = CreateInstallationDialog
		( pstrInstDir->m_varStr, dwOptions,
				wstrCaption, pWindow, wstrDefSubDir ) ;
	if ( !err )
	{
		if ( pintOptions != NULL )
		{
			pintOptions->SetValue( dwOptions ) ;
		}
	}
	return	context.PushObject( context.new_CSInteger( err ) ) ;
}

// メンバ関数 : Integer CloseInstallationDialog()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_CloseInstallationDialog
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
	{
		return	err ;
	}
	return	context.PushObject
		( context.new_CSInteger( CloseInstallationDialog() ) ) ;
}

// メンバ関数 : Integer IsInstallationDialogCanceled()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_IsInstallationDialogCanceled
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
	{
		return	err ;
	}
	return	context.PushObject
		( context.new_CSInteger( IsInstallationDialogCanceled() ) ) ;
}

// メンバ関数 : Integer SetInstallationDialogFileText( String sText )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_SetInstallationDialogFileText
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
	{
		return	err ;
	}
	EWideString	wstrText ;
	err = context.GetArgumentAsStr( wstrText, lstArg, 1, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	err = SetInstallationDialogFileText( wstrText ) ;
	return	context.PushObject( context.new_CSInteger( err ) ) ;
}

// メンバ関数 : Integer SetInstallationDialogProgress
//						( Integer nFile, Integer nTotal )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_SetInstallationDialogProgress
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 3 ) ;
	if ( err )
	{
		return	err ;
	}
	int	nFile, nTotal ;
	err = context.GetArgumentAsInt( nFile, lstArg, 1, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsInt( nTotal, lstArg, 2, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	err = SetInstallationDialogProgress( nFile, nTotal ) ;
	return	context.PushObject( context.new_CSInteger( err ) ) ;
}

// メンバ関数 : Integer InstallationMessageBox
//			( String sText, String sCaption, Integer nType [, Window window] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_InstallationMessageBox
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 3, 5 ) ;
	if ( err )
	{
		return	err ;
	}
	EWideString	wstrText, wstrCaption ;
	int			nMBType ;
	err = context.GetArgumentAsStr( wstrText, lstArg, 1, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsStr( wstrCaption, lstArg, 2, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsInt( nMBType, lstArg, 3, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	ECSWindow *	pWindow =
		ESLTypeCast<ECSWindow>
			( context.GetArgumentObjectAs( lstArg, 4, L"Window" ) ) ;
	int	nResult =
		InstallationMessageBox( wstrText, wstrCaption, nMBType, pWindow ) ;
	return	context.PushObject( context.new_CSInteger( nResult ) ) ;
}

// メンバ関数 : Integer GetFontList( String[]& aFontList, Integer nFlags )
//////////////////////////////////////////////////////////////////////////////
struct	EnumFontParam
{
	ECSArray *	paFonts ;
	DWORD		nFlags ;
} ;
static int CALLBACK EnumFontFamiliessProc
	( ENUMLOGFONTEX *lpelfe,
		NEWTEXTMETRICEX *lpntme, int FontType, LPARAM lParam )
{
	EnumFontParam *	efp = (EnumFontParam*) lParam ;
	if ( efp->nFlags != 0 )
	{
		bool	fMatchCharset = false ;
		if ( (efp->nFlags & 0x01)
			&& (lpelfe->elfLogFont.lfCharSet == DEFAULT_CHARSET) )
		{
			fMatchCharset = true ;
		}
		if ( (efp->nFlags & 0x02)
			&& (lpelfe->elfLogFont.lfCharSet == ANSI_CHARSET) )
		{
			fMatchCharset = true ;
		}
		if ( (efp->nFlags & 0x04)
			&& (lpelfe->elfLogFont.lfCharSet == SHIFTJIS_CHARSET) )
		{
			fMatchCharset = true ;
		}
		if ( (efp->nFlags & 0x08)
			&& (lpelfe->elfLogFont.lfCharSet == SYMBOL_CHARSET) )
		{
			fMatchCharset = true ;
		}
		if ( !fMatchCharset )
		{
			return	TRUE ;
		}
	}
	efp->paFonts->m_varArray.Add
		( new ECSString( EWideString( lpelfe->elfLogFont.lfFaceName ) ) ) ;
	return	TRUE ;
}

ESLError ECSSetup::Call_GetFontList
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err = context.VerifyArgumentCount( lstArg, 2, 3 ) ;
	if ( err )
		return	err ;
	//
	ECSArray *	paFonts =
		ESLTypeCast<ECSArray>
			( context.GetArgumentObjectAs( lstArg, 1, L"Array" ) ) ;
	if ( paFonts == NULL )
	{
		return	ESLErrorMsg( "Array オブジェクトが指定されていません。" ) ;
	}
	int	nFlags ;
	err = context.GetArgumentAsInt( nFlags, lstArg, 2, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	HDC	hdc = ::CreateCompatibleDC( NULL ) ;
	LOGFONT			lfEnum ;
	::eslFillMemory( &lfEnum, 0, sizeof(lfEnum) ) ;
	lfEnum.lfCharSet = DEFAULT_CHARSET ;
	//
	EnumFontParam	efpParam ;
	efpParam.paFonts = paFonts ;
	efpParam.nFlags = nFlags ;
	//
	::EnumFontFamiliesEx
		( hdc, &lfEnum,
			(FONTENUMPROCA) &EnumFontFamiliessProc, (LPARAM) &efpParam, 0 ) ;
	::DeleteDC( hdc ) ;
	//
	return	context.PushObject
		( context.new_CSInteger( paFonts->m_varArray.GetSize() ) ) ;
}

// メンバ関数 : String GetWindowsProductID()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_GetWindowsProductID
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
	{
		return	err ;
	}
	return	context.PushObject
		( new ECSString( EWideString( GetWindowsProductID() ) ) ) ;
}

// メンバ関数 : String MakeMD5Digest( Reference rObj )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_MakeMD5Digest
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
	{
		return	err ;
	}
	EString	strMD5 ;
	ECSFile *	pfile =
		ESLTypeCast<ECSFile>
			( context.GetArgumentObjectAs( lstArg, 1, L"File" ) ) ;
	if ( pfile == NULL )
	{
		EWideString	wstrBuf ;
		err = context.GetArgumentAsStr( wstrBuf, lstArg, 1, NULL ) ;
		if ( !err )
		{
			EString	strBuf = wstrBuf ;
			strMD5 = MakeMD5Digest( strBuf, strBuf.GetLength() ) ;
		}
		else
		{
			return	err ;
		}
	}
	else
	{
		if ( pfile->GetFileInterface() == NULL )
		{
			return	err ;
		}
		EStreamBuffer	buf ;
		buf.ReadFromFile( *(pfile->GetFileInterface()) ) ;
		EPtrBuffer	ptrbuf = buf.GetBuffer( ) ;
		strMD5 = MakeMD5Digest
			( (const char *) ptrbuf.GetBuffer(), ptrbuf.GetLength() ) ;
	}
	return	context.PushObject
		( new ECSString( EWideString( strMD5 ) ) ) ;
}

// メンバ関数 : Integer CalcCRC32( Reference rObj )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_CalcCRC32
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
	{
		return	err ;
	}
	DWORD	dwCRC ;
	ECSFile *	pfile =
		ESLTypeCast<ECSFile>
			( context.GetArgumentObjectAs( lstArg, 1, L"File" ) ) ;
	if ( pfile == NULL )
	{
		EWideString	wstrBuf ;
		err = context.GetArgumentAsStr( wstrBuf, lstArg, 1, NULL ) ;
		if ( !err )
		{
			EString	strBuf = wstrBuf ;
			dwCRC = CalcCRC32( strBuf, strBuf.GetLength() ) ;
		}
		else
		{
			return	err ;
		}
	}
	else
	{
		if ( pfile->GetFileInterface() == NULL )
		{
			return	err ;
		}
		EStreamBuffer	buf ;
		buf.ReadFromFile( *(pfile->GetFileInterface()) ) ;
		EPtrBuffer	ptrbuf = buf.GetBuffer( ) ;
		dwCRC = CalcCRC32
			( (const char *) ptrbuf.GetBuffer(), ptrbuf.GetLength() ) ;
	}
	return	context.PushObject( new ECSInteger( dwCRC ) ) ;
}

// メンバ関数 : Integer CheckSum32( Reference rObj )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_CheckSum32
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
	{
		return	err ;
	}
	DWORD	dwCheckSum ;
	ECSFile *	pfile =
		ESLTypeCast<ECSFile>
			( context.GetArgumentObjectAs( lstArg, 1, L"File" ) ) ;
	if ( pfile == NULL )
	{
		EWideString	wstrBuf ;
		err = context.GetArgumentAsStr( wstrBuf, lstArg, 1, NULL ) ;
		if ( !err )
		{
			EString	strBuf = wstrBuf ;
			dwCheckSum = CheckSum32( strBuf, strBuf.GetLength() ) ;
		}
		else
		{
			return	err ;
		}
	}
	else
	{
		if ( pfile->GetFileInterface() == NULL )
		{
			return	err ;
		}
		EStreamBuffer	buf ;
		buf.ReadFromFile( *(pfile->GetFileInterface()) ) ;
		EPtrBuffer	ptrbuf = buf.GetBuffer( ) ;
		dwCheckSum = CheckSum32
			( (const char *) ptrbuf.GetBuffer(), ptrbuf.GetLength() ) ;
	}
	return	context.PushObject( new ECSInteger( dwCheckSum ) ) ;
}

// メンバ関数 : String GetDesktopDirectory()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_GetDesktopDirectory
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
	{
		return	err ;
	}
	return	context.PushObject
		( new ECSString( EWideString( GetDesktopDirectory() ) ) ) ;
}

// メンバ関数 : String GetStartMenuDirectory()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_GetStartMenuDirectory
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
	{
		return	err ;
	}
	return	context.PushObject
		( new ECSString( EWideString( GetStartMenuDirectory() ) ) ) ;
}

// メンバ関数 : String GetAppDataDirectory()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_GetAppDataDirectory
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
	{
		return	err ;
	}
	return	context.PushObject
		( new ECSString( EWideString( GetAppDataDirectory() ) ) ) ;
}

// メンバ関数 : String GetWindowsDirectory()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_GetWindowsDirectory
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
	{
		return	err ;
	}
	return	context.PushObject
		( new ECSString( EWideString( GetWindowsDirectory() ) ) ) ;
}

// メンバ関数 : String GetCurrentModulePath()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_GetCurrentModulePath
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
	{
		return	err ;
	}
	return	context.PushObject
		( new ECSString( EWideString( GetCurrentModulePath() ) ) ) ;
}

// メンバ関数 : Integer GetEnvironmentVariable( Hash<String>& mapEnv )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_GetEnvironmentVariable
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
	{
		return	err ;
	}
	ECSHash *	pEnv =
		ESLTypeCast<ECSHash>
			( context.GetArgumentObjectAs( lstArg, 1, L"Hash" ) ) ;
	if ( pEnv == NULL )
	{
		return	ESLErrorMsg( "引数に Hash オブジェクトが指定されていません" ) ;
	}
	const char *	pszEnvs = ::GetEnvironmentStrings() ;
	while ( pszEnvs[0] )
	{
		EString	strEnv = pszEnvs ;
		pszEnvs += strEnv.GetLength() + 1 ;
		//
		int	iEqu = strEnv.Find( '=' ) ;
		if ( iEqu >= 0 )
		{
			ECSWideString	wstrName = strEnv.Left( iEqu ) ;
			ECSWideString	wstrValue = strEnv.Middle( iEqu + 1 ) ;
			wstrName.MakeUpper() ;
			pEnv->m_varArray.SetAs( wstrName, new ECSString( wstrValue ) ) ;
		}
	}
	return	context.PushObject( new ECSInteger( eslErrSuccess ) ) ;
}

// メンバ関数 : String FilterEnvironmentPath( String sSrcPath )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_FilterEnvironmentPath
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
	{
		return	err ;
	}
	EWideString	wstrSrc ;
	err = context.GetArgumentAsStr( wstrSrc, lstArg, 1, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	ECSEnvironment *	pEnv = context.GetEnvironment() ;
	if ( pEnv != NULL )
	{
		wstrSrc = pEnv->FilterFilePath( wstrSrc ) ;
	}
	return	context.PushObject( new ECSString( wstrSrc ) ) ;
}

// メンバ関数 : Integer GetDiskVolumeName( String sDrv, Reference sVolName )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_GetDiskVolumeName
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 3 ) ;
	if ( err )
	{
		return	err ;
	}
	EWideString	wstrDrv ;
	err = context.GetArgumentAsStr( wstrDrv, lstArg, 1, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	ECSString *	pStrVolName =
		ESLTypeCast<ECSString>
			( context.GetArgumentObjectAs( lstArg, 2, L"String" ) ) ;
	if ( pStrVolName == NULL )
	{
		return	ESLErrorMsg
			( "引数に String 型オブジェクトが指定されていません" ) ;
	}
	EString	strVolName ;
	err = GetDiskVolumeName( EString( wstrDrv ), strVolName ) ;
	if ( !err )
	{
		pStrVolName->m_varStr = strVolName ;
	}
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer GetDiskSerialNumber( String sDrv, Reference nSerialNum )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_GetDiskSerialNumber
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 3 ) ;
	if ( err )
	{
		return	err ;
	}
	EWideString	wstrDrv ;
	err = context.GetArgumentAsStr( wstrDrv, lstArg, 1, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	ECSInteger *	pSerialNum =
		ESLTypeCast<ECSInteger>
			( context.GetArgumentObjectAs( lstArg, 2, L"Integer" ) ) ;
	if ( pSerialNum == NULL )
	{
		return	ESLErrorMsg
			( "引数に Integer 型オブジェクトが指定されていません" ) ;
	}
	DWORD	dwSerialNumber ;
	err = GetDiskSerialNumber( EString( wstrDrv ), dwSerialNumber ) ;
	if ( !err )
	{
		pSerialNum->SetValue( dwSerialNumber ) ;
	}
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer GetDiskFreeSpace
//		( String sDrv, Reference nFreeAvailable,
//			Reference nTotalBytes, Reference nFreeSpace )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_GetDiskFreeSpace
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 5 ) ;
	if ( err )
	{
		return	err ;
	}
	EWideString	wstrDrv ;
	err = context.GetArgumentAsStr( wstrDrv, lstArg, 1, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	ECSInteger *	pFreeAvailable =
		ESLTypeCast<ECSInteger>
			( context.GetArgumentObjectAs( lstArg, 2, L"Integer" ) ) ;
	if ( pFreeAvailable == NULL )
	{
		return	ESLErrorMsg
			( "引数に Integer 型オブジェクトが指定されていません" ) ;
	}
	ECSInteger *	pTotalBytes =
		ESLTypeCast<ECSInteger>
			( context.GetArgumentObjectAs( lstArg, 3, L"Integer" ) ) ;
	if ( pTotalBytes == NULL )
	{
		return	ESLErrorMsg
			( "引数に Integer 型オブジェクトが指定されていません" ) ;
	}
	ECSInteger *	pFreeSpace =
		ESLTypeCast<ECSInteger>
			( context.GetArgumentObjectAs( lstArg, 4, L"Integer" ) ) ;
	if ( pFreeSpace == NULL )
	{
		return	ESLErrorMsg
			( "引数に Integer 型オブジェクトが指定されていません" ) ;
	}
	UINT64	nFreeAvailable, nTotalBytes, nFreeSpace ;
	err = GetDiskFreeSpace
		( EString( wstrDrv ), nFreeAvailable, nTotalBytes, nFreeSpace ) ;
	if ( !err )
	{
		pFreeAvailable->SetValue( nFreeAvailable ) ;
		pTotalBytes->SetValue( nTotalBytes ) ;
		pFreeSpace->SetValue( nFreeSpace ) ;
	}
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer ShellExecute( String sVerb, String sFile )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_ShellExecute
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 3, 4 ) ;
	if ( err )
	{
		return	err ;
	}
	EWideString	wstrVerb, wstrFile, wstrParameter ;
	err = context.GetArgumentAsStr( wstrVerb, lstArg, 1, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsStr( wstrFile, lstArg, 2, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsStr( wstrParameter, lstArg, 3, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	return	context.PushObject
		( new ECSInteger( ShellExecute( wstrVerb, wstrFile, wstrParameter ) ) ) ;
}

// メンバ関数 : Integer ExecuteProcess
//		( String sAppFile, String sCmdLine[, Integer nFlags
//				[, String sEnvironment[, String sCurrentDirectory]]] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_ExecuteProcess
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 3, 6 ) ;
	if ( err )
	{
		return	err ;
	}
	ECSWideString	wstrAppFile, wstrCmdLine ;
	ECSWideString	wstrEnvironment, wstrCurrentDir ;
	int				nFlags ;
	err = context.GetArgumentAsStr( wstrAppFile, lstArg, 1, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsStr( wstrCmdLine, lstArg, 2, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsInt( nFlags, lstArg, 3, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsStr( wstrEnvironment, lstArg, 4, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsStr( wstrCurrentDir, lstArg, 5, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	return	context.PushObject
		( new ECSInteger( ExecuteProcess
				( wstrAppFile, wstrCmdLine,
					nFlags, wstrEnvironment, wstrCurrentDir ) ) ) ;
}

// メンバ関数 : Integer GetExecuteExitCode
//					( Integer nTimeout[, Integer & nExitCode] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_GetExecuteExitCode
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1, 3 ) ;
	if ( err )
	{
		return	err ;
	}
	int	nTimeout ;
	err = context.GetArgumentAsInt( nTimeout, lstArg, 1, INFINITE ) ;
	if ( err )
	{
		return	err ;
	}
	ECSInteger *	pExitCode =
		ESLTypeCast<ECSInteger>
			( context.GetArgumentObjectAs( lstArg, 2, L"Integer" ) ) ;
	DWORD	dwExitCode ;
	err = GetExitCodeExecute( nTimeout, &dwExitCode ) ;
	if ( pExitCode != NULL )
	{
		pExitCode->SetValue( dwExitCode ) ;
	}
	return	context.PushObject( context.new_CSInteger( err ) ) ;
}

// メンバ関数 : Integer BrowseForFolder
//		( Reference sDir, String sCaption
//				[, Reference rMainWindow [, Integer nParentWnd]] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_BrowseForFolder
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 3, 5 ) ;
	if ( err )
	{
		return	err ;
	}
	ECSString *	pStrDir =
		ESLTypeCast<ECSString>
			( context.GetArgumentObjectAs( lstArg, 1, L"String" ) ) ;
	if ( pStrDir == NULL )
	{
		return	ESLErrorMsg
			( "引数に String 型オブジェクトが指定されていません。" ) ;
	}
	EWideString	wstrCaption ;
	err = context.GetArgumentAsStr( wstrCaption, lstArg, 2, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	HWND	hParentWnd = NULL ;
	ECSWindow *	pWindow = NULL ;
	pWindow = ESLTypeCast<ECSWindow>
			( context.GetArgumentObjectAs( lstArg, 3, L"Window" ) ) ;
	if ( pWindow != NULL )
	{
		if ( pWindow->GetWindow() != NULL )
		{
			hParentWnd = *(pWindow->GetWindow()) ;
		}
	}
	INT64	nParentWnd ;
	err = context.GetArgumentAsInt64( nParentWnd, lstArg, 4, 0 ) ;
	if ( !err )
	{
		hParentWnd = (HWND) nParentWnd ;
		if ( !IsWindow( hParentWnd ) )
		{
			hParentWnd = NULL ;
		}
	}
	err = BrowseForFolder
		( pStrDir->m_varStr, wstrCaption, pWindow, hParentWnd ) ;
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer BrowseFileDialog
//			( String& sPath, Boolean fSaveFile, String sCaption,
//				String aFilters, Window& rMainWindow := null )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_BrowseFileDialog
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 5, 6 ) ;
	if ( err )
	{
		return	err ;
	}
	ECSString *	pStrDir =
		ESLTypeCast<ECSString>
			( context.GetArgumentObjectAs( lstArg, 1, L"String" ) ) ;
	if ( pStrDir == NULL )
	{
		return	ESLErrorMsg
			( "引数に String 型オブジェクトが指定されていません。" ) ;
	}
	int	fSaveFile ;
	err = context.GetArgumentAsInt( fSaveFile, lstArg, 2, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	EWideString	wstrCaption ;
	err = context.GetArgumentAsStr( wstrCaption, lstArg, 3, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	EWideString	wstrFilter ;
	err = context.GetArgumentAsStr( wstrFilter, lstArg, 4, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	HWND	hParentWnd = NULL ;
	ECSWindow *	pWindow = NULL ;
	pWindow = ESLTypeCast<ECSWindow>
			( context.GetArgumentObjectAs( lstArg, 5, L"Window" ) ) ;
	if ( pWindow != NULL )
	{
		if ( pWindow->GetWindow() != NULL )
		{
			hParentWnd = *(pWindow->GetWindow()) ;
		}
	}
	err = BrowseFileDialog
		( pStrDir->m_varStr, (fSaveFile != 0),
			wstrCaption, wstrFilter, pWindow, hParentWnd ) ;
	return	context.PushObject( new ECSInteger( err ) ) ;
}


// インストールファイルリスト関数
//////////////////////////////////////////////////////////////////////////////

// メンバ関数 : Integer ReadInstalledLog( {File file | String filename} )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_ReadInstalledLog
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
	{
		return	err ;
	}
	ECSFile *	pFile =
		ESLTypeCast<ECSFile>
			( context.GetArgumentObjectAs( lstArg, 1, L"File" ) ) ;
	if ( pFile != NULL )
	{
		if ( pFile->GetFileInterface() != NULL )
		{
			err = ReadInstalledLog( *(pFile->GetFileInterface()) ) ;
		}
		else
		{
			err = eslErrGeneral ;
		}
	}
	else
	{
		EWideString	wstrFileName ;
		err = context.GetArgumentAsStr( wstrFileName, lstArg, 1, NULL ) ;
		if ( err )
		{
			return	err ;
		}
		ESLFileObject *	pfile = context.OpenFileOnScript( wstrFileName ) ;
		if ( pfile != NULL )
		{
			err = ReadInstalledLog( *pfile ) ;
			delete	pfile ;
		}
		else
		{
			err = eslErrGeneral ;
		}
	}
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer WriteInstalledLog( {File file | String filename} )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_WriteInstalledLog
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
	{
		return	err ;
	}
	ECSFile *	pFile =
		ESLTypeCast<ECSFile>
			( context.GetArgumentObjectAs( lstArg, 1, L"File" ) ) ;
	if ( pFile != NULL )
	{
		if ( pFile->GetFileInterface() != NULL )
		{
			err = WriteInstalledLog( *(pFile->GetFileInterface()) ) ;
		}
		else
		{
			err = eslErrGeneral ;
		}
	}
	else
	{
		EWideString	wstrFileName ;
		err = context.GetArgumentAsStr( wstrFileName, lstArg, 1, NULL ) ;
		if ( err )
		{
			return	err ;
		}
		ESLFileObject *	pfile =
			context.OpenFileOnScript
				( wstrFileName, ESLFileObject::modeCreate ) ;
		if ( pfile != NULL )
		{
			err = WriteInstalledLog( *pfile ) ;
			delete	pfile ;
		}
		else
		{
			err = eslErrGeneral ;
		}
	}
	return	context.PushObject( new ECSInteger( err ) ) ;
}


// メンバ関数 : Integer MeasureInstallSize( Reference nSize )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_MeasureInstallSize
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
	{
		return	err ;
	}
	ECSInteger *	pGetSize =
		ESLTypeCast<ECSInteger>
			( context.GetArgumentObjectAs( lstArg, 1, L"Integer" ) ) ;
	if ( pGetSize == NULL )
	{
		return	ESLErrorMsg
			( "引数に Integer オブジェクトが指定されていません。" ) ;
	}
	UINT64	nBytes ;
	err = MeasureInstallSize( nBytes ) ;
	pGetSize->SetValue( nBytes ) ;
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer AddInstallDirectory( String sDstPath )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_AddInstallDirectory
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
	{
		return	err ;
	}
	EWideString	wstrDstPath ;
	err = context.GetArgumentAsStr( wstrDstPath, lstArg, 1, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	if ( AddInstallDirectory( wstrDstPath ) == NULL )
	{
		err = eslErrGeneral ;
	}
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer AddInstallArchiveDirectory
//		( String sDstPath, String sPassword := "", Integer nType := 0 )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_AddInstallArchiveDirectory
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2, 4 ) ;
	if ( err )
	{
		return	err ;
	}
	EWideString	wstrDstPath, wstrPassword ;
	int			nType ;
	err = context.GetArgumentAsStr( wstrDstPath, lstArg, 1, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsStr( wstrPassword, lstArg, 2, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsInt( nType, lstArg, 3, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	err = AddInstallArchiveDirectory
		( wstrDstPath, EString( wstrPassword ), nType ) ;
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer AddInstallFile
//		( String sDstPath, String sSrcPath )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_AddInstallFile
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 3 ) ;
	if ( err )
	{
		return	err ;
	}
	EWideString	wstrDstPath, wstrSrcPath ;
	err = context.GetArgumentAsStr( wstrDstPath, lstArg, 1, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsStr( wstrSrcPath, lstArg, 2, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	if ( AddInstallFile( wstrDstPath, wstrSrcPath ) == NULL )
	{
		err = eslErrGeneral ;
	}
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer AddInstallArchiveTree
//		( String sDstPath, String sSrcArchiveFile, String sPassword := "" )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_AddInstallArchiveTree
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 3, 4 ) ;
	if ( err )
	{
		return	err ;
	}
	EWideString	wstrDstPath, wstrSrcArchiveFile, wstrPassword ;
	err = context.GetArgumentAsStr( wstrDstPath, lstArg, 1, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsStr( wstrSrcArchiveFile, lstArg, 2, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsStr( wstrPassword, lstArg, 3, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	ECSEnvironment *	pEnv = context.GetEnvironment() ;
	if ( pEnv != NULL )
	{
		wstrSrcArchiveFile = pEnv->FilterFilePath( wstrSrcArchiveFile ) ;
	}
	err = AddInstallArchiveTree
		( wstrDstPath, wstrSrcArchiveFile, EString( wstrPassword ) ) ;
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer AddInstallDirectoryTree
//		( String sDstPath, String sSrcPath )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_AddInstallDirectoryTree
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 3 ) ;
	if ( err )
	{
		return	err ;
	}
	EWideString	wstrDstPath, wstrSrcPath ;
	err = context.GetArgumentAsStr( wstrDstPath, lstArg, 1, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsStr( wstrSrcPath, lstArg, 2, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	err = AddInstallDirectoryTree( wstrDstPath, wstrSrcPath ) ;
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// インストール処理関数
//////////////////////////////////////////////////////////////////////////////

// メンバ関数 : Integer BootCheck
//		( Stirng sCheckName, Integer fDisableBoot := false )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_BootCheck
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2, 3 ) ;
	if ( err )
	{
		return	err ;
	}
	EWideString	wstrCheckName ;
	int			fDisableBoot ;
	err = context.GetArgumentAsStr( wstrCheckName, lstArg, 1, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsInt( fDisableBoot, lstArg, 2, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	bool	fBoot =
		BootCheck( EString( wstrCheckName ), (fDisableBoot != 0) ) ;
	return	context.PushObject( new ECSInteger( - (int) fBoot ) ) ;
}

// メンバ関数 : ReleaseBootCheck()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_ReleaseBootCheck
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
	{
		return	err ;
	}
	ReleaseBootCheck( ) ;
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : Integer GetLastErrorMsg( Reference rErrMsg )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_GetLastErrorMsg
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
	{
		return	err ;
	}
	ECSString *	pStrErrMsg =
		ESLTypeCast<ECSString>
			( context.GetArgumentObjectAs( lstArg, 1, L"String" ) ) ;
	if ( pStrErrMsg == NULL )
	{
		return	ESLErrorMsg
			( "引数に String オブジェクトが指定されていません" ) ;
	}
	err = GetLastErrorMsg( pStrErrMsg->m_varStr ) ;
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer BeginInstall( String sInstDir )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_BeginInstall
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
	{
		return	err ;
	}
	EWideString	wstrInstDir ;
	err = context.GetArgumentAsStr( wstrInstDir, lstArg, 1, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	err = BeginInstall( wstrInstDir ) ;
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer IsFinishedInstall( )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_IsFinishedInstall
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
	{
		return	err ;
	}
	bool	fFinished = IsFinishedInstall() ;
	return	context.PushObject( new ECSInteger( - (int) fFinished ) ) ;
}

// メンバ関数 : Integer InstallNextFile
//			( Reference rDstFilePath, Reference rSrcFilePath )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_InstallNextFile
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 3 ) ;
	if ( err )
	{
		return	err ;
	}
	ECSString *	pDstFilePath =
		ESLTypeCast<ECSString>
			( context.GetArgumentObjectAs( lstArg, 1, L"String" ) ) ;
	if ( pDstFilePath == NULL )
	{
		return	ESLErrorMsg
			( "引数に String オブジェクトが指定されていません" ) ;
	}
	ECSString *	pSrcFilePath =
		ESLTypeCast<ECSString>
			( context.GetArgumentObjectAs( lstArg, 2, L"String" ) ) ;
	if ( pSrcFilePath == NULL )
	{
		return	ESLErrorMsg
			( "引数に String オブジェクトが指定されていません" ) ;
	}
	err = InstallNextFile
		( pDstFilePath->m_varStr, pSrcFilePath->m_varStr, &context ) ;
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer GetCurrentCopiedBytes( [Reference nFileSize] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_GetCurrentCopiedBytes
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
	{
		return	err ;
	}
	ECSInteger *	pGetFileSize =
		ESLTypeCast<ECSInteger>
			( context.GetArgumentObjectAs( lstArg, 1, L"Integer" ) ) ;
	UINT64	nCurrent, nTotal ;
	nCurrent = GetCurrentCopiedBytes( nTotal ) ;
	if ( pGetFileSize != NULL )
	{
		pGetFileSize->SetValue( nTotal ) ;
	}
	return	context.PushObject( new ECSInteger( nCurrent ) ) ;
}

// メンバ関数 : Integer GetTotalCopiedBytes( [Reference nTotalSize] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_GetTotalCopiedBytes
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1, 2 ) ;
	if ( err )
	{
		return	err ;
	}
	ECSInteger *	pGetTotalSize =
		ESLTypeCast<ECSInteger>
			( context.GetArgumentObjectAs( lstArg, 1, L"Integer" ) ) ;
	UINT64	nCurrent, nTotal ;
	nCurrent = GetTotalCopiedBytes( nTotal ) ;
	if ( pGetTotalSize != NULL )
	{
		pGetTotalSize->SetValue( nTotal ) ;
	}
	return	context.PushObject( new ECSInteger( nCurrent ) ) ;
}

// メンバ関数 : Integer WaitForCurrentCopy( Integer nTimeout )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_WaitForCurrentCopy
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
	{
		return	err ;
	}
	int	nTimeout ;
	err = context.GetArgumentAsInt( nTimeout, lstArg, 1, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	err = WaitForCurrentCopy( nTimeout, &context ) ;
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer EndInstall()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_EndInstall
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
	{
		return	err ;
	}
	err = EndInstall( ) ;
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer AddInstallFileLog( String sDstFilePath )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_AddInstallFileLog
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
	{
		return	err ;
	}
	EWideString	wstrDstPath ;
	err = context.GetArgumentAsStr( wstrDstPath, lstArg, 1, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	m_flstLog.AddFile
		( wstrDstPath.GetFileDirectoryPart(),
					wstrDstPath.GetFileNamePart() ) ;
	return	context.PushObject( new ECSInteger( 0 ) ) ;
}

// メンバ関数 : Integer InstallCreateDirectory( String sDirPath )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_InstallCreateDirectory
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
	{
		return	err ;
	}
	EWideString	wstrDirPath ;
	err = context.GetArgumentAsStr( wstrDirPath, lstArg, 1, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	err = InstallCreateDirectory( wstrDirPath, 0 ) ;
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer InstallCreateShortcutFile
//		( String sDstDir, String sName, String sLinkTarget, String sArg := "" )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_InstallCreateShortcutFile
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 4, 5 ) ;
	if ( err )
	{
		return	err ;
	}
	EWideString	wstrDirPath, wstrName, wstrLinkTarget, wstrArg ;
	err = context.GetArgumentAsStr( wstrDirPath, lstArg, 1, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsStr( wstrName, lstArg, 2, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsStr( wstrLinkTarget, lstArg, 3, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsStr( wstrArg, lstArg, 4, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	err = InstallCreateShortcutFile
		( wstrDirPath, wstrName,
			EString( wstrLinkTarget ), EString( wstrArg ) ) ;
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// レジストリ関数
//////////////////////////////////////////////////////////////////////////////

// メンバ関数 : Integer GetUninstallInfo
//					( String sRegName, UninstallInfo info )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_GetUninstallInfo
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 3 ) ;
	if ( err )
	{
		return	err ;
	}
	EWideString	wstrRegName ;
	err = context.GetArgumentAsStr( wstrRegName, lstArg, 1, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	ECSStructure *	pStructInfo =
		ESLTypeCast<ECSStructure>
			( context.GetArgumentObjectAs( lstArg, 2, L"UninstallInfo" ) ) ;
	if ( pStructInfo == NULL )
	{
		return	ESLErrorMsg
			( "引数に UninstallInfo オブジェクトが指定されていません" ) ;
	}
	UNINSTALL_INFO	uninst ;
	err = GetUninstallInfo( wstrRegName, uninst ) ;
	if ( !err )
	{
		pStructInfo->SetMemberAsStr
			( L"strDisplayName", uninst.wstrDisplayName ) ;
		pStructInfo->SetMemberAsStr
			( L"strDisplayIcon", uninst.wstrDisplayIcon ) ;
		pStructInfo->SetMemberAsStr
			( L"strUninstallCmdLine", uninst.wstrUninstallCmdLine ) ;
		pStructInfo->SetMemberAsStr
			( L"strUninstallPath", uninst.wstrUninstallPath ) ;
		pStructInfo->SetMemberAsStr
			( L"strInstallLocation", uninst.wstrInstallLocation ) ;
		pStructInfo->SetMemberAsStr
			( L"strPublisher", uninst.wstrPublisher ) ;
		pStructInfo->SetMemberAsStr
			( L"strVersionMajor", uninst.wstrVersionMajor ) ;
		pStructInfo->SetMemberAsStr
			( L"strVersionMinor", uninst.wstrVersionMinor ) ;
	}
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer RegisterUninstall
//					( String sRegName, UninstallInfo info )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_RegisterUninstall
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 3 ) ;
	if ( err )
	{
		return	err ;
	}
	EWideString	wstrRegName ;
	err = context.GetArgumentAsStr( wstrRegName, lstArg, 1, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	ECSStructure *	pStructInfo =
		ESLTypeCast<ECSStructure>
			( context.GetArgumentObjectAs( lstArg, 2, L"UninstallInfo" ) ) ;
	if ( pStructInfo == NULL )
	{
		return	ESLErrorMsg
			( "引数に UninstallInfo オブジェクトが指定されていません" ) ;
	}
	UNINSTALL_INFO	uninst ;
	uninst.wstrDisplayName =
		pStructInfo->GetMemberAsStr
			( L"strDisplayName", uninst.wstrDisplayName ) ;
	uninst.wstrDisplayIcon =
		pStructInfo->GetMemberAsStr
			( L"strDisplayIcon", uninst.wstrDisplayIcon ) ;
	uninst.wstrUninstallCmdLine =
		pStructInfo->GetMemberAsStr
			( L"strUninstallCmdLine", uninst.wstrUninstallCmdLine ) ;
	uninst.wstrUninstallPath =
		pStructInfo->GetMemberAsStr
			( L"strUninstallPath", uninst.wstrUninstallPath ) ;
	uninst.wstrInstallLocation =
		pStructInfo->GetMemberAsStr
			( L"strInstallLocation", uninst.wstrInstallLocation ) ;
	uninst.wstrPublisher =
		pStructInfo->GetMemberAsStr
			( L"strPublisher", uninst.wstrPublisher ) ;
	uninst.wstrVersionMajor =
		pStructInfo->GetMemberAsStr
			( L"strVersionMajor", uninst.wstrVersionMajor ) ;
	uninst.wstrVersionMinor =
		pStructInfo->GetMemberAsStr
			( L"strVersionMinor", uninst.wstrVersionMinor ) ;
	err = RegisterUninstall( wstrRegName, uninst ) ;
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer GetRegUninstallInteger32
//		( String sRegName, String sValueName, Integer nDefValue := 0 )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_GetRegUninstallInteger32
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 3, 4 ) ;
	if ( err )
	{
		return	err ;
	}
	EWideString	wstrRegName, wstrValueName ;
	int			nDefValue ;
	err = context.GetArgumentAsStr( wstrRegName, lstArg, 1, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsStr( wstrValueName, lstArg, 2, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsInt( nDefValue, lstArg, 3, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	DWORD	dwValue =
		GetRegUninstallInteger32( wstrRegName, wstrValueName, nDefValue ) ;
	return	context.PushObject( new ECSInteger( dwValue ) ) ;
}

// メンバ関数 : Integer GetRegUninstallInteger64
//		( String sRegName, String sValueName, Integer nDefValue := 0 )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_GetRegUninstallInteger64
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 3, 4 ) ;
	if ( err )
	{
		return	err ;
	}
	EWideString	wstrRegName, wstrValueName ;
	INT64		nDefValue ;
	err = context.GetArgumentAsStr( wstrRegName, lstArg, 1, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsStr( wstrValueName, lstArg, 2, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsInt64( nDefValue, lstArg, 3, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	INT64	nValue =
		GetRegUninstallInteger64( wstrRegName, wstrValueName, nDefValue ) ;
	return	context.PushObject( new ECSInteger( nValue ) ) ;
}

// メンバ関数 : String GetRegUninstallString
//		( String sRegName, String sValueName, String sDefValue := "" )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_GetRegUninstallString
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 3, 4 ) ;
	if ( err )
	{
		return	err ;
	}
	EWideString	wstrRegName, wstrValueName ;
	EWideString	wstrDefValue ;
	err = context.GetArgumentAsStr( wstrRegName, lstArg, 1, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsStr( wstrValueName, lstArg, 2, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsStr( wstrDefValue, lstArg, 3, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	EWideString	wstrValue =
		GetRegUninstallString( wstrRegName, wstrValueName, wstrDefValue ) ;
	return	context.PushObject( new ECSString( wstrValue ) ) ;
}

// メンバ関数 : Integer SetRegUninstallInteger32
//		( String sRegName, String sValueName, Integer nValue )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_SetRegUninstallInteger32
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 4 ) ;
	if ( err )
	{
		return	err ;
	}
	EWideString	wstrRegName, wstrValueName ;
	int			nValue ;
	err = context.GetArgumentAsStr( wstrRegName, lstArg, 1, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsStr( wstrValueName, lstArg, 2, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsInt( nValue, lstArg, 3, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	err = SetRegUninstallInteger32
		( wstrRegName, wstrValueName, nValue ) ;
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer SetRegUninstallInteger64
//		( String sRegName, String sValueName, Integer nValue )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_SetRegUninstallInteger64
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 4 ) ;
	if ( err )
	{
		return	err ;
	}
	EWideString	wstrRegName, wstrValueName ;
	INT64		nValue ;
	err = context.GetArgumentAsStr( wstrRegName, lstArg, 1, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsStr( wstrValueName, lstArg, 2, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsInt64( nValue, lstArg, 3, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	err = SetRegUninstallInteger64
		( wstrRegName, wstrValueName, nValue ) ;
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : Integer SetRegUninstallString
//		( String sRegName, String sValueName, String strValue )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_SetRegUninstallString
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 4 ) ;
	if ( err )
	{
		return	err ;
	}
	EWideString	wstrRegName, wstrValueName ;
	EWideString	wstrValue ;
	err = context.GetArgumentAsStr( wstrRegName, lstArg, 1, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsStr( wstrValueName, lstArg, 2, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsStr( wstrValue, lstArg, 3, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	err = SetRegUninstallString
		( wstrRegName, wstrValueName, wstrValue ) ;
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// アンインストール関数
//////////////////////////////////////////////////////////////////////////////

// メンバ関数 : Integer Uninstall( String sInstDir )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_Uninstall
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
	{
		return	err ;
	}
	EWideString	wstrInstDir ;
	err = context.GetArgumentAsStr( wstrInstDir, lstArg, 1, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	err = Uninstall( wstrInstDir ) ;
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : DeleteInstalledFile( String sFilePath )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_DeleteInstalledFile
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
	{
		return	err ;
	}
	EWideString	wstrFilePath ;
	err = context.GetArgumentAsStr( wstrFilePath, lstArg, 1, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	DeleteInstalledFile( EString( wstrFilePath ) ) ;
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : Integer IsNecessaryRebootToDelete()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_IsNecessaryRebootToDelete
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
	{
		return	err ;
	}
	bool	fReboot = IsNecessaryRebootToDelete( ) ;
	return	context.PushObject( new ECSInteger( - (int) fReboot ) ) ;
}

// メンバ関数 : UnRegisterUninstall( String sRegName )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_UnRegisterUninstall
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
	{
		return	err ;
	}
	EWideString	wstrRegName ;
	err = context.GetArgumentAsStr( wstrRegName, lstArg, 1, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	err = UnRegisterUninstall( wstrRegName ) ;
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : RebootWindows()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSSetup::Call_RebootWindows
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
	{
		return	err ;
	}
	if ( IsNecessaryRebootToDelete() )
	{
		RebootWindows( ) ;
	}
	return	context.PushObject( new ECSInteger( 0 ) ) ;
}
