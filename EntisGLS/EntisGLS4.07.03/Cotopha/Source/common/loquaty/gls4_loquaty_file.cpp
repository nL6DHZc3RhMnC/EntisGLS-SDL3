
#include <loquaty/gls4_loquaty.h>


//////////////////////////////////////////////////////////////////////////////
// Loquaty::LPureFile -> SSystem::SFileInterface
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Loquaty::SLoquatyFile, SFileInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SLoquatyFile::SLoquatyFile( LFilePtr pFile )
	: m_pFile( pFile )
{
	assert( pFile != nullptr ) ;
}

// ファイルから読み込み
//////////////////////////////////////////////////////////////////////////////
size_t SLoquatyFile::Read( void * ptrBuf, size_t nBytes )
{
	return	m_pFile->Read( ptrBuf, nBytes ) ;
}

// ファイルへ書き込み
//////////////////////////////////////////////////////////////////////////////
size_t SLoquatyFile::Write( const void * ptrBuf, size_t nBytes )
{
	return	m_pFile->Write( ptrBuf, nBytes ) ;
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SSystem::SFileInterface * SLoquatyFile::NewOpenFile
	( const wchar_t * pszFilePath, long int nOpenFlags )
{
	LDirectory *	pDir = m_pFile->GetDirectory() ;
	if ( pDir != nullptr )
	{
		LFilePtr	pFile =
			pDir->OpenFile
				( pszFilePath, OpenFlagsGLS4ToLoquaty(nOpenFlags) ) ;
		if ( pFile != nullptr )
		{
			return	new SLoquatyFile( pFile ) ;
		}
	}
	return	nullptr ;
}

long SLoquatyFile::OpenFlagsGLS4ToLoquaty( long int nOpenFlags )
{
	static const long	s_flagsLoquaty[][2] =
	{
		{ SFileOpener::modeCreateFlag,		LDirectory::modeCreateFlag },
		{ SFileOpener::modeReadFlag,		LDirectory::modeReadFlag },
		{ SFileOpener::modeWriteFlag,		LDirectory::modeWriteFlag },
		{ SFileOpener::modeCreateDirFlag,	LDirectory::modeCreateDirsFlag },
		{ SFileOpener::modeStreaming,		LDirectory::modeStreamingFlag },
		{ 0, 0 },
	} ;
	long	nFlags = (nOpenFlags & SFileOpener::maskPermission)
									>> SFileOpener::shifterPermission ;
	for ( int i = 0; s_flagsLoquaty[i][0] != 0; i ++ )
	{
		if ( nOpenFlags & s_flagsLoquaty[i][0] )
		{
			nFlags |= s_flagsLoquaty[i][1] ;
		}
	}
	return	nFlags ;
}

// ファイルの存在
//////////////////////////////////////////////////////////////////////////////
bool SLoquatyFile::IsExisting( const wchar_t * pszFilePath )
{
	LDirectory *	pDir = m_pFile->GetDirectory() ;
	if ( pDir != nullptr )
	{
		return	pDir->IsExisting( pszFilePath ) ;
	}
	return	false ;
}

// ファイル状態
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SLoquatyFile::QueryState
	( const wchar_t * pszFilePath, SFileOpener::State& state )
{
	LDirectory *	pDir = m_pFile->GetDirectory() ;
	if ( pDir == nullptr )
	{
		return	errFailed ;
	}
	LDirectory::State	lstate ;
	if ( !pDir->QueryFileState( lstate, pszFilePath ) )
	{
		return	errFailed ;
	}
	StateLoquatyToGLS4( lstate, state ) ;
	return	errSuccess ;
}

void SLoquatyFile::StateLoquatyToGLS4
	( const LDirectory::State& lstate, SFileOpener::State& state )
{
	state.bitFields = 0 ;
	if ( lstate.fields & LDirectory::stateHasAttribute )
	{
		state.bitFields |= SFileOpener::fieldAttributes ;
		state.bitAttributes = (lstate.attributes & LDirectory::permissionMask) ;
		if ( lstate.attributes & LDirectory::attrDirectory )
		{
			state.bitAttributes |= SFileOpener::attrDirectory ;
		}
		if ( lstate.attributes & LDirectory::attrHidden )
		{
			state.bitAttributes |= SFileOpener::attrHidden ;
		}
	}
	if ( lstate.fields & LDirectory::stateHasFileSize )
	{
		state.bitFields |= SFileOpener::fieldFileSize ;
		state.nFileSize = lstate.fileSizeInBytes ;
	}
	if ( lstate.fields & LDirectory::stateHasModifiedDate )
	{
		state.bitFields |= SFileOpener::fieldModifiedTime ;
		DateTimeLoquatyToGLS4( lstate.dateModified, state.dtModified ) ;
	}
	if ( lstate.fields & LDirectory::stateHasCreatedDate )
	{
		state.bitFields |= SFileOpener::fieldCreatedTime ;
		DateTimeLoquatyToGLS4( lstate.dateCreated, state.dtCreated ) ;
	}
	if ( lstate.fields & LDirectory::stateHasAccessedDate )
	{
		state.bitFields |= SFileOpener::fieldAccessedTime ;
		DateTimeLoquatyToGLS4( lstate.dateAccessed, state.dtAccessed ) ;
	}
}

void SLoquatyFile::DateTimeLoquatyToGLS4
	( const LDateTime& ldate, SSystem::DATE_TIME& date )
{
	date.nYear = ldate.year ;
	date.nMonth = ldate.month ;
	date.nDay = ldate.day ;
	date.nWeek = ldate.week ;
	date.nHour = ldate.hour ;
	date.nMinute = ldate.minute ;
	date.nSecond = ldate.second ;
	date.nMilliSec = ldate.msec ;
}

// ファイルの一覧取得
//////////////////////////////////////////////////////////////////////////////
void SLoquatyFile::ListSubFiles
	( SSystem::SObjectArray<SSystem::SString>& listFiles, const wchar_t * pszDirPath )
{
	listFiles.RemoveAll() ;

	LDirectory *	pDir = m_pFile->GetDirectory() ;
	if ( pDir != nullptr )
	{
		std::vector<LString>	files ;
		pDir->ListFiles( files, pszDirPath ) ;

		for ( size_t i = 0; i < files.size(); i ++ )
		{
			LString				path = LURLSchemer::SubPath
											( pszDirPath, files.at(i).c_str() ) ;
			LDirectory::State	state ;
			if ( pDir->QueryFileState( state, path.c_str() ) )
			{
				if ( (state.fields & LDirectory::stateHasAttribute)
					&& !(state.attributes & LDirectory::attrDirectory) )
				{
					listFiles.Add( new SString( files.at(i).c_str() ) ) ;
				}
			}
		}
	}
}

// ディレクトリの一覧取得
//////////////////////////////////////////////////////////////////////////////
void SLoquatyFile::ListSubDirectories
	( SSystem::SObjectArray<SSystem::SString>& listDirs, const wchar_t * pszDirPath )
{
	listDirs.RemoveAll() ;

	LDirectory *	pDir = m_pFile->GetDirectory() ;
	if ( pDir != nullptr )
	{
		std::vector<LString>	files ;
		pDir->ListFiles( files, pszDirPath ) ;

		for ( size_t i = 0; i < files.size(); i ++ )
		{
			LString				path = LURLSchemer::SubPath
											( pszDirPath, files.at(i).c_str() ) ;
			LDirectory::State	state ;
			if ( pDir->QueryFileState( state, path.c_str() ) )
			{
				if ( (state.fields & LDirectory::stateHasAttribute)
					&& (state.attributes & LDirectory::attrDirectory) )
				{
					listDirs.Add( new SString( files.at(i).c_str() ) ) ;
				}
			}
		}
	}
}

// ファイルインターフェースの複製
//////////////////////////////////////////////////////////////////////////////
SSystem::SFileInterface * SLoquatyFile::Duplicate( void ) const
{
	return	new SLoquatyFile( m_pFile ) ;
}

// シーク可能か否か？
//////////////////////////////////////////////////////////////////////////////
bool SLoquatyFile::IsSeekable( void ) const
{
	return	m_pFile->IsSeekable() ;
}

// ファイル長の取得
//////////////////////////////////////////////////////////////////////////////
int64_t SLoquatyFile::GetLength( void ) const
{
	return	m_pFile->GetLength() ;
}

// ファイルポインタを移動
//////////////////////////////////////////////////////////////////////////////
int64_t SLoquatyFile::Seek( int64_t posFile, SeekOrigin seekFrom )
{
	switch ( seekFrom )
	{
	case	FromBegin:
		m_pFile->Seek( posFile ) ;
		break ;
	case	FromCurrent:
		m_pFile->Skip( posFile ) ;
		break ;
	case	FromEnd:
		m_pFile->Seek( m_pFile->GetLength() + posFile ) ;
		break ;
	}
	return	m_pFile->GetPosition() ;
}

// ファイルポインタを取得
//////////////////////////////////////////////////////////////////////////////
int64_t SLoquatyFile::GetPosition( void ) const
{
	return	m_pFile->GetPosition() ;
}

// ファイルの終端を現在の位置に設定する
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SLoquatyFile::SetEndOfFile( void )
{
	m_pFile->Truncate() ;
	return	errSuccess ;
}



//////////////////////////////////////////////////////////////////////////////
// SSystem::SFileInterface -> Loquaty::LPureFile
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
LGLS4File::LGLS4File( SSystem::SFileInterface * pFile )
	: m_pFile( pFile )
{
}

// ファイルから読み込む
//////////////////////////////////////////////////////////////////////////////
size_t LGLS4File::Read( void * buf, size_t bytes )
{
	return	m_pFile->Read( buf, bytes ) ;
}

// ファイルへ読み込む
//////////////////////////////////////////////////////////////////////////////
size_t LGLS4File::Write( const void * buf, size_t bytes )
{
	return	m_pFile->Write( buf, bytes ) ;
}

// 書き出しを確定する
//////////////////////////////////////////////////////////////////////////////
void LGLS4File::Flush( void )
{
}

// シークする
//////////////////////////////////////////////////////////////////////////////
void LGLS4File::Seek( std::int64_t pos )
{
	m_pFile->Seek( pos, SFileInterface::FromBegin ) ;
}

void LGLS4File::Skip( std::int64_t bytes )
{
	m_pFile->Seek( bytes, SFileInterface::FromCurrent ) ;
}

// シーク可能か？
//////////////////////////////////////////////////////////////////////////////
bool LGLS4File::IsSeekable( void ) const
{
	return	m_pFile->IsSeekable() ;
}

// 現在の位置を取得する (ストリームの場合は -1)
//////////////////////////////////////////////////////////////////////////////
std::int64_t LGLS4File::GetPosition( void ) const
{
	return	m_pFile->GetPosition() ;
}

// ファイル全長 (ストリームの場合は -1)
//////////////////////////////////////////////////////////////////////////////
std::int64_t LGLS4File::GetLength( void ) const
{
	return	m_pFile->GetLength() ;
}

// 現在の位置にファイルを切り詰める
//////////////////////////////////////////////////////////////////////////////
void LGLS4File::Truncate( void )
{
	m_pFile->SetEndOfFile() ;
}

// ファイルパスを取得する（大抵は開いたときのファイルパスで絶対パスではない）
//////////////////////////////////////////////////////////////////////////////
LString LGLS4File::GetFilePath( void ) const
{
	SFile *	pFile = ESLTypeCast<SFile>( m_pFile.Ptr() ) ;
	if ( pFile != nullptr )
	{
		return	LString( pFile->GetFilePath() ) ;
	}
	return	LString() ;
}

// ディレクトリを取得する
//（可能なら GetFilePath() で取得したパスで同じファイルが開けるようにする）
//////////////////////////////////////////////////////////////////////////////
LDirectory * LGLS4File::GetDirectory( void )
{
	return	this ;
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
LFilePtr LGLS4File::OpenFile
	( const wchar_t * pwszPath, long nOpenFlags )
{
	SFileInterface *	pFile =
		m_pFile->NewOpenFile
			( pwszPath, OpenFlagsLoquatyToGLS4( nOpenFlags ) ) ;
	if ( pFile != nullptr )
	{
		return	std::make_shared<LGLS4File>( pFile ) ;
	}
	return	nullptr ;
}

long LGLS4File::OpenFlagsLoquatyToGLS4( long nOpenFlags )
{
	static const long	s_flagsLoquaty[][2] =
	{
		{ SFileOpener::modeCreateFile,		LDirectory::modeCreateFlag },
		{ SFileOpener::shareRead,			LDirectory::modeReadFlag },
		{ SFileOpener::shareWrite,			LDirectory::modeWriteFlag },
		{ SFileOpener::modeCreateDirFlag,	LDirectory::modeCreateDirsFlag },
		{ SFileOpener::modeStreaming,		LDirectory::modeStreamingFlag },
		{ 0, 0 },
	} ;
	long	nFlags = (nOpenFlags & LDirectory::modePermissionMask)
									<< SFileOpener::shifterPermission ;
	for ( int i = 0; s_flagsLoquaty[i][0] != 0; i ++ )
	{
		if ( nOpenFlags & s_flagsLoquaty[i][1] )
		{
			nFlags |= s_flagsLoquaty[i][0] ;
		}
	}
	return	nFlags ;
}

// 可能なら同等のディレクトリを複製する
//////////////////////////////////////////////////////////////////////////////
std::shared_ptr<LDirectory> LGLS4File::Duplicate( void )
{
	SFileInterface *	pDup = m_pFile->Duplicate() ;
	if ( pDup != nullptr )
	{
		return	std::make_shared<LGLS4File>( pDup ) ;
	}
	return	nullptr ;
}

// ファイル情報取得
//////////////////////////////////////////////////////////////////////////////
bool LGLS4File::QueryFileState( LDirectory::State& state, const wchar_t * pwszPath )
{
	SFileOpener::State	fstate ;
	if ( m_pFile->QueryState( pwszPath, fstate ) == errSuccess )
	{
		state.fields = 0 ;
		if ( fstate.bitFields & SFileOpener::fieldAttributes )
		{
			state.fields |= stateHasAttribute ;
			state.attributes = fstate.bitAttributes & permissionMask ;
			if ( fstate.bitAttributes & SFileOpener::attrDirectory )
			{
				state.attributes |= attrDirectory ;
			}
			if ( fstate.bitAttributes & SFileOpener::attrHidden )
			{
				state.attributes |= attrHidden ;
			}
		}
		if ( fstate.bitFields & SFileOpener::fieldFileSize )
		{
			state.fields |= stateHasFileSize ;
			state.fileSizeInBytes = fstate.nFileSize ;
		}
		if ( fstate.bitFields & SFileOpener::fieldAccessedTime )
		{
			state.fields |= stateHasAccessedDate ;
			DateTimeGLS4ToLoquaty( fstate.dtAccessed, state.dateAccessed ) ;
		}
		if ( fstate.bitFields & SFileOpener::fieldModifiedTime )
		{
			state.fields |= stateHasModifiedDate ;
			DateTimeGLS4ToLoquaty( fstate.dtModified, state.dateModified ) ;
		}
		if ( fstate.bitFields & SFileOpener::fieldCreatedTime )
		{
			state.fields |= stateHasCreatedDate ;
			DateTimeGLS4ToLoquaty( fstate.dtCreated, state.dateCreated ) ;
		}
		return	true ;
	}
	return	false ;
}

void LGLS4File::DateTimeGLS4ToLoquaty
	( const SSystem::DATE_TIME& date, LDateTime& ldate )
{
	ldate.year = date.nYear ;
	ldate.month = (LUint8) date.nMonth ;
	ldate.day = (LUint8) date.nDay ;
	ldate.week = (LUint8) date.nWeek ;
	ldate.hour = (LUint8) date.nHour ;
	ldate.minute = (LUint8) date.nMinute ;
	ldate.second = (LUint8) date.nSecond ;
	ldate.msec = date.nMilliSec ;
}

// ファイル名（サブディレクトリ含む）列挙
// ※ files へは以前のデータを削除せずに追加
// ※ ファイル名にはディレクトリパスを含まない
//////////////////////////////////////////////////////////////////////////////
void LGLS4File::ListFiles
	( std::vector<LString>& files, const wchar_t * pwszSubDirPath )
{
	SObjectArray<SString>	listDirs ;
	m_pFile->ListSubDirectories( listDirs, pwszSubDirPath ) ;

	SObjectArray<SString>	listFiles ;
	m_pFile->ListSubFiles( listFiles, pwszSubDirPath ) ;

	for ( size_t i = 0; i < listDirs.GetLength(); i ++ )
	{
		files.push_back( LString( listDirs.At(i) ) ) ;
	}
	for ( size_t i = 0; i < listFiles.GetLength(); i ++ )
	{
		files.push_back( LString( listFiles.At(i) ) ) ;
	}
}

// ファイル削除
//////////////////////////////////////////////////////////////////////////////
bool LGLS4File::DeleteFile( const wchar_t * pwszPath )
{
	return	(m_pFile->RemoveSubFile( pwszPath ) == errSuccess) ;
}

// ファイル名変更
//////////////////////////////////////////////////////////////////////////////
bool LGLS4File::RenameFile
	( const wchar_t * pwszOldPath, const wchar_t * pwszNewPath )
{
	return	(m_pFile->RenameSubFile( pwszOldPath, pwszNewPath ) == errSuccess) ;
}

// サブディレクトリ作成
//////////////////////////////////////////////////////////////////////////////
bool LGLS4File::CreateDirectory( const wchar_t * pwszPath )
{
	return	(m_pFile->CreateSubDirectory( pwszPath ) == errSuccess) ;
}

// サブディレクトリ削除
//////////////////////////////////////////////////////////////////////////////
bool LGLS4File::DeleteDirectory( const wchar_t * pwszPath )
{
	return	(m_pFile->RemoveSubDirectory( pwszPath ) == errSuccess) ;
}


