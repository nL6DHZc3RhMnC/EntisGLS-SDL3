
/*****************************************************************************
             Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
   Copyright (c) 2003-2007 Leshade Entis, Entis-soft. All rights reserved.
 *****************************************************************************/


#if	!defined(__GLSPEIMAGEFILE_H__)
#define	__GLSPEIMAGEFILE_H__

#pragma	pack( push, __GLSPEIMAGEFILE_ALIGN__, 1 )

//////////////////////////////////////////////////////////////////////////////
// Win32 PE 形式ファイル解析クラス
//////////////////////////////////////////////////////////////////////////////

class	EXEImageFileObject	: public	ESLFileObject
{
public:
	// 構築関数
	EXEImageFileObject( void ) ;
	// 消滅関数
	virtual ~EXEImageFileObject( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( EXEImageFileObject, ESLFileObject )

protected:	// 読み込んだデータ
	ESLFileObject *				m_pImageFile ;		// イメージファイル
	HANDLE						m_hFileMapping ;	// ファイルオブジェクト
	PBYTE						m_pPEImage ;		// イメージデータ
	DWORD						m_dwBytes ;			// イメージサイズ

	IMAGE_FILE_HEADER *			m_fildehdr ;		// COFF ファイルヘッダ
	IMAGE_OPTIONAL_HEADER32 *	m_opthdr ;			// オプションヘッダ
	IMAGE_SECTION_HEADER *		m_sechdr ;			// セクションヘッダ
	IMAGE_SECTION_HEADER *		m_prsrcsec ;		// ".rsrc" セクションヘッダ

	DWORD						m_dwRsrcBasePos ;	// リソースの基準アドレス
	DWORD						m_dwRsrcDataLen ;	// リソースのバイト数

protected:	// 書き出すためのデータ
	// リソースディレクトリテーブル
	struct	RSRC_DIRECTORY_TABLE
	{
		DWORD	dwCharacteristics ;
		DWORD	dwTimeStamp ;
		WORD	wMajorVersion ;
		WORD	wMinorVersion ;
		WORD	wNumberOfNameEntries ;
		WORD	wNumberOfIDEntries ;
	} ;
	// リソースディレクトリエントリ
	struct	RSRC_DIRECTORY_ENTRY
	{
		union
		{
			DWORD	dwNameRVA ;		// 文字列のアドレス（最上位ビットは1）
			DWORD	dwIntegerID ;	// 整数識別子
		}			type ;
		union
		{
			DWORD	dwDataEntryRVA ;	// データ（最上位ビットが0）
			DWORD	dwSubdirectoryRVA ;	// サブディレクトリ（最上位ビットが1）
		}			data ;
	} ;
	// リソースデータエントリ
	struct	RSRC_DATA_ENTRY
	{
		DWORD	dwDataRVA ;			// データのオフセットアドレス
		DWORD	dwSize ;			// データのバイト数
		DWORD	dwCodepage ;		// コードページ
		DWORD	dwReserved ;
	} ;
	// リソースデータエントリ配列
	class	ERsrcDataEntry
	{
	public:
		DWORD			m_dwLanguage ;	// 言語識別子
		DWORD			m_dwRVA ;		// アドレス
		RSRC_DATA_ENTRY	m_rsde ;		// リソースデータエントリ
	} ;
	// リソースデータ言語別リスト
	class	ERsrcDataList
	{
	public:
		DWORD						m_dwResID ;
		EWideString					m_wstrResID ;
		RSRC_DIRECTORY_TABLE		m_rsdt ;
		RSRC_DIRECTORY_ENTRY		m_rsde ;
		EObjArray<ERsrcDataEntry>	m_lstEntries ;
	} ;
	// リソースデータ識別子別リスト
	class	ERsrcNameList
	{
	public:
		DWORD						m_dwTypeID ;
		EWideString					m_wstrTypeID ;
		RSRC_DIRECTORY_TABLE		m_rsdt ;
		RSRC_DIRECTORY_ENTRY		m_rsde ;
		EObjArray<ERsrcDataList>	m_lstEntries ;
	} ;

	EObjArray<ERsrcNameList>	m_lstRsrcDir ;	// データエントリ
	ESLFileObject *				m_pDstFile ;	// 出力ファイル

public:
	// PE イメージファイルを読み込む
	ESLError ReadPEImageFile
		( ESLFileObject & file, bool fFileMapping = true ) ;
	// PE イメージファイルを開く
	ESLError OpenPEImageFile( ESLFileObject & file ) ;
	// PE イメージファイルを閉じる
	void ClosePEImageFile( void ) ;
	// 指定のリソースデータを検索する
	ESLError FindResourceData
		( EPtrBuffer & ptrbuf,
			LPCTSTR lpTypeID, LPCSTR lpResID, WORD wLanguage = 0 ) ;
	// 指定のリソースデータを開く
	ESLError DescendResourceData
		( LPCTSTR lpTypeID, LPCSTR lpResID, WORD wLanguage = 0 ) ;
	// 現在開いているリソースデータを閉じる
	ESLError AscendResourceData( void ) ;
	// リソースを列挙する
	ESLError EnumResourceEntries
		( LPCTSTR lpTypeID = NULL,
			LPCSTR lpResID = NULL, WORD wLanguage = 0 ) ;
protected:
	// リソースを列挙する（リソースID、言語を列挙）
	ESLError EnumResourceIDsEntries
		( RSRC_DIRECTORY_TABLE * pdirResID,
			LPCTSTR lpTypeID, LPCSTR lpResID = NULL, WORD wLanguage = 0 ) ;
	// リソースを列挙する（言語を列挙）
	ESLError EnumResourceLangEntries
		( RSRC_DIRECTORY_TABLE * pdirLanguage,
			LPCTSTR lpTypeID, LPCSTR lpResID, WORD wLanguage = 0 ) ;
	// リソース列挙関数
	virtual ESLError OnFindResourceEntry
		( LPCTSTR lpTypeID, LPCSTR lpResID,
			WORD wLanguage, void * ptrDataEntry, DWORD dwDataLength ) ;

public:
	// リソースデータエントリを追加設定する
	ESLError AddResourceDataEntry
		( LPCTSTR lpTypeID, LPCSTR lpResID, WORD wLanguage, DWORD dwSize ) ;
	// PE イメージファイルの書き出しを開始する
	ESLError BeginWritingPEImage( ESLFileObject & file ) ;
	// リソースデータを書き出す
	ESLError AddWriteResourceData
		( LPCTSTR lpTypeID, LPCSTR lpResID, WORD wLanguage,
			const void * ptrBuf, unsigned long int nBytes ) ;
	// リソースデータを書き出す
	ESLError AddWriteResourceFile
		( LPCTSTR lpTypeID, LPCSTR lpResID,
				WORD wLanguage, ESLFileObject & file ) ;
	// PE イメージファイルの書き出しを完了する
	ESLError EndWritingPEImage( void ) ;

protected:
	// リソースデータ書き出しのためのリソースの情報を取得する
	RSRC_DATA_ENTRY * FindResourceToWrite
		( LPCTSTR lpTypeID, LPCSTR lpResID, WORD wLanguage, bool fCreate ) ;

public:
	// ファイルオブジェクトを複製する
	virtual ESLFileObject * Duplicate( void ) const ;
	// ファイルから読み込む
	virtual unsigned long int Read
		( void * ptrBuffer, unsigned long int nBytes ) ;
	// ファイルへ書き出す
	virtual unsigned long int Write
		( const void * ptrBuffer, unsigned long int nBytes ) ;
	// ファイルの長さを取得
	virtual unsigned long int GetLength( void ) const ;
	// ファイルポインタを移動
	virtual unsigned long int Seek
		( long int nOffsetPos, SeekOrigin fSeekFrom ) ;
	// ファイルポインタを取得
	virtual unsigned long int GetPosition( void ) const ;

} ;


//////////////////////////////////////////////////////////////////////////////
// アイコン・カーソルファイル解析クラス
//////////////////////////////////////////////////////////////////////////////

class	EWin32IconFile	: public	ESLObject
{
public:
	// 構築関数
	EWin32IconFile( void ) ;
	// 消滅関数
	virtual ~EWin32IconFile( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( EWin32IconFile, ESLObject )

public:
	// ファイルヘッダ
	struct	FILE_HEADER
	{
		WORD	wSignature ;
		WORD	wFileType ;
		WORD	wImageCount ;
	} ;
	// アイコンエントリ
	struct	ICON_ENTRY
	{
		BYTE	bytWidth ;
		BYTE	bytHeight ;
		BYTE	bytColors ;
		BYTE	bytReserved ;
		WORD	xHotSpot ;
		WORD	yHotSpot ;
		DWORD	dwSize ;
		DWORD	dwAddress ;
	} ;
	// RT_GROUP_CURSOR データ構造体
	struct	GROUP_CURSOR_ENTRY
	{
		WORD		wWidth ;
		WORD		wHeight ;
		WORD		wPlanes ;
		WORD		wBitCount ;
		DWORD		dwDataSize ;
		WORD		wResourceID ;
	} ;
	struct	GROUP_CURSOR
	{
		FILE_HEADER			gcFileHdr ;
		GROUP_CURSOR_ENTRY	gcEntries[1] ;
	} ;
	// RT_GROUP_ICON データ構造体
	struct	GROUP_ICON_ENTRY
	{
		BYTE		bytWidth ;
		BYTE		bytHeight ;
		BYTE		bytColors ;
		BYTE		bytReserved ;
		WORD		wPlanes ;
		WORD		wBitCount ;
		DWORD		dwDataSize ;
		WORD		wResourceID ;
	} ;
	struct	GROUP_ICON
	{
		FILE_HEADER			giFileHdr ;
		GROUP_ICON_ENTRY	giEntries[1] ;
	} ;

protected:
	FILE_HEADER				m_filehdr ;		// ファイルヘッダ

	class	EImageEntry	: public	EStreamBuffer
	{
	public:
		BITMAPINFOHEADER *	m_pbmih ;
		DWORD				m_dwSize ;
	} ;
	EObjArray<ICON_ENTRY>	m_lstEntries ;	// エントリリスト
	EObjArray<EImageEntry>	m_lstImages ;	// 画像リスト

public:
	// アイコンファイルを読み込む
	ESLError ReadIconFile( ESLFileObject & file ) ;
	ESLError ReadBitmapFile( ESLFileObject & file ) ;
	// アイコンファイルを書き出す
	ESLError WriteIconFile( ESLFileObject & file, WORD wType = 1 ) ;
	// データを初期化する
	void DeleteContents( void ) ;

public:
	// ファイルヘッダを取得する
	const FILE_HEADER & GetFileHeader( void ) const
		{
			return	m_filehdr ;
		}
	// 画像枚数を取得する
	int GetImageCount( void ) const
		{
			return	m_lstImages.GetSize( ) ;
		}
	// アイコンエントリを取得する
	ICON_ENTRY * GetIconEntryAt( int nIndex ) const ;
	// 画像データを取得する
	BITMAPINFOHEADER * GetImageAt( int nIndex ) const ;
	// 画像データサイズを取得する
	DWORD GetImageSizeAt( int nIndex ) const ;
	// 画像データを追加する
	ESLError AddImage
		( BITMAPINFOHEADER * pbmih,
			DWORD dwSize, const ICON_ENTRY * pIconEntry = NULL ) ;

} ;


#pragma	pack( pop, __GLSPEIMAGEFILE_ALIGN__ )

#endif
