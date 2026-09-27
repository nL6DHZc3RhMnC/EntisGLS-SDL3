
#include <sakura/sakura.h>
#include <sakura/ssys_win_resource_file.h>

using namespace SSystem ;


//////////////////////////////////////////////////////////////////////////////
// Win32 PE バイナリリソース・ファイルインターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SWin32PEBinResourceFile, SMemoryReferenceFile )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SWin32PEBinResourceFile::SWin32PEBinResourceFile( void )
{
	m_hRsrc = NULL ;
	m_hGlobal = NULL ;
	m_lpData = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SWin32PEBinResourceFile::~SWin32PEBinResourceFile( void )
{
	CloseResource() ;
}

// バイナリリソースを開く
//////////////////////////////////////////////////////////////////////////////
SError SWin32PEBinResourceFile::OpenResource
				( HMODULE hModule, const wchar_t * pwszRsrcID )
{
	if ( m_hRsrc != NULL )
	{
		CloseResource() ;
	}
	SArray<char>	bufName ;
	LPCTSTR			lpName ;
	SString	strRsrcID = pwszRsrcID ;
	lpName = strRsrcID.EncodeDefaultTo( bufName ) ;
	//
	m_hRsrc = ::FindResource( hModule, lpName, RT_RCDATA ) ;
	if ( m_hRsrc == NULL )
	{
		m_hRsrc = ::FindResource( hModule, lpName, "RT_RCDATA" ) ;
		if ( m_hRsrc == NULL )
		{
			return	errFailed ;
		}
	}
	m_hGlobal = ::LoadResource( hModule, m_hRsrc ) ;
	if ( m_hGlobal == NULL )
	{
		return	errFailed ;
	}
	DWORD	dwSize = ::SizeofResource( hModule, m_hRsrc ) ;
	m_lpData = ::LockResource( m_hGlobal ) ;
	if ( m_lpData == NULL )
	{
		return	errFailed ;
	}
	AttachMemory( m_lpData, (size_t) dwSize ) ;
	return	errSuccess ;
}

// リソースを解放する
//////////////////////////////////////////////////////////////////////////////
SError SWin32PEBinResourceFile::CloseResource( void )
{
	AttachMemory( NULL, 0 ) ;
	//
	m_lpData = NULL ;
	m_hGlobal = NULL ;
	m_hRsrc = NULL ;
	//
	return	errSuccess ;
}

// ファイルインターフェースの複製
//////////////////////////////////////////////////////////////////////////////
SFileInterface * SWin32PEBinResourceFile::Duplicate( void ) const
{
	SWin32PEBinResourceFile *	pfile = new SWin32PEBinResourceFile ;
	pfile->AttachMemory( m_pbytMemory, m_nLength ) ;
	return	pfile ;
}

// ファイルへ書き込み
//////////////////////////////////////////////////////////////////////////////
size_t SWin32PEBinResourceFile::Write( const void * ptrBuf, size_t nBytes )
{
	return	0 ;
}



//////////////////////////////////////////////////////////////////////////////
// Win32 PE バイナリリソース・オープナー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SWin32PEBinResourceOpener, SFileOpener )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SWin32PEBinResourceOpener::SWin32PEBinResourceOpener( HMODULE hModule )
{
	m_hModule = hModule ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SWin32PEBinResourceOpener::~SWin32PEBinResourceOpener( void )
{
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SFileInterface * SWin32PEBinResourceOpener::NewOpenFile
	( const wchar_t * pszFilePath, long int nOpenFlags )
{
	SWin32PEBinResourceFile *	pfile = new SWin32PEBinResourceFile ;
	if ( pfile->OpenResource( m_hModule, pszFilePath ) )
	{
		delete	pfile ;
		return	NULL ;
	}
	return	pfile ;
}

// ファイルの存在
//////////////////////////////////////////////////////////////////////////////
bool SWin32PEBinResourceOpener::IsExisting( const wchar_t * pszFilePath )
{
	SWin32PEBinResourceFile	file ;
	if ( file.OpenResource( m_hModule, pszFilePath ) )
	{
		return	false ;
	}
	return	true ;
}

// ファイル状態
//////////////////////////////////////////////////////////////////////////////
SError SWin32PEBinResourceOpener::QueryState
	( const wchar_t * pszFilePath, SFileOpener::State& state )
{
	return	errFailed ;
}

// ファイルの一覧取得
//////////////////////////////////////////////////////////////////////////////
void SWin32PEBinResourceOpener::ListSubFiles
	( SObjectArray<SString>& listFiles, const wchar_t * pszDirPath )
{
}

// ディレクトリの一覧取得
//////////////////////////////////////////////////////////////////////////////
void SWin32PEBinResourceOpener::ListSubDirectories
	( SObjectArray<SString>& listDirs, const wchar_t * pszDirPath )
{
}

