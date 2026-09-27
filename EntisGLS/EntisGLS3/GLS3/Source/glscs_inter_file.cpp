
/*****************************************************************************
             Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
	Copyright (c) 2002-2012 Leshade Entis, Entis-soft. All rights reserved.
 ****************************************************************************/


#include <gls.h>


//////////////////////////////////////////////////////////////////////////////
// SSystem::SFileInterface -> ESLFileObject の変換
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ESLSFileInterface, ESLFileObject )

// ESLSFileInterface 消滅
//////////////////////////////////////////////////////////////////////////////
ESLSFileInterface::~ESLSFileInterface( void )
{
	if ( m_fOwner )
	{
		delete	m_pfile ;
	}
}

// ファイルオブジェクトを複製する
//////////////////////////////////////////////////////////////////////////////
ESLFileObject * ESLSFileInterface::Duplicate( void ) const
{
	return	new ESLSFileInterface( m_pfile->Duplicate(), true ) ;
}

// ファイルから読み込む
//////////////////////////////////////////////////////////////////////////////
unsigned long int ESLSFileInterface::Read
	( void * ptrBuffer, unsigned long int nBytes )
{
	return	m_pfile->Read( ptrBuffer, nBytes ) ;
}

// ファイルへ書き出す
//////////////////////////////////////////////////////////////////////////////
unsigned long int ESLSFileInterface::Write
	( const void * ptrBuffer, unsigned long int nBytes )
{
	return	m_pfile->Write( ptrBuffer, nBytes ) ;
}

// ファイルの長さを取得
//////////////////////////////////////////////////////////////////////////////
unsigned long int ESLSFileInterface::GetLength( void ) const
{
	return	(unsigned long int) m_pfile->GetLength() ;
}

// ファイルポインタを移動
//////////////////////////////////////////////////////////////////////////////
unsigned long int ESLSFileInterface::Seek
	( long int nOffsetPos, ESLFileObject::SeekOrigin fSeekFrom )
{
	return	(unsigned long int) m_pfile->Seek
		( nOffsetPos, (SSystem::SFileInterface::SeekOrigin) fSeekFrom ) ;
}

// ファイルポインタを取得
//////////////////////////////////////////////////////////////////////////////
unsigned long int ESLSFileInterface::GetPosition( void ) const
{
	return	(unsigned long int) m_pfile->GetPosition() ;
}

// ファイルの終端を現在の位置に設定する
//////////////////////////////////////////////////////////////////////////////
ESLError ESLSFileInterface::SetEndOfFile( void )
{
	return	(ESLError) m_pfile->SetEndOfFile() ;
}

// ファイルの長さを取得
//////////////////////////////////////////////////////////////////////////////
UINT64 ESLSFileInterface::GetLargeLength( void ) const
{
	return	m_pfile->GetLength() ;
}

// ファイルポインタを移動
//////////////////////////////////////////////////////////////////////////////
UINT64 ESLSFileInterface::SeekLarge
	( INT64 nOffsetPos, ESLFileObject::SeekOrigin fSeekFrom )
{
	return	m_pfile->Seek
		( nOffsetPos, (SSystem::SFileInterface::SeekOrigin) fSeekFrom ) ;
}

// ファイルポインタを取得
//////////////////////////////////////////////////////////////////////////////
UINT64 ESLSFileInterface::GetLargePosition( void ) const
{
	return	m_pfile->GetPosition() ;
}


//////////////////////////////////////////////////////////////////////////////
// ESLFileObject -> SSystem::SFileInterface の変換
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SESLFileInterface, SFileInterface )

// SESLFileInterface 消滅
//////////////////////////////////////////////////////////////////////////////
SESLFileInterface::~SESLFileInterface( void )
{
	if ( m_fOwner )
	{
		delete	m_pfile ;
	}
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SSystem::SFileInterface * SESLFileInterface::OpenFile
	( const wchar_t * pwszFilePath, long int nOpenFlags )
{
	return	new SESLFileInterface
		( m_pfile->OpenFileObject( pwszFilePath, nOpenFlags ), true ) ;
}

// ファイルの存在
//////////////////////////////////////////////////////////////////////////////
bool SESLFileInterface::IsExisting( const wchar_t * pwszFilePath )
{
	return	false ;
}

// ファイルインターフェースの複製
//////////////////////////////////////////////////////////////////////////////
SSystem::SFileInterface * SESLFileInterface::Duplicate( void ) const
{
	return	new SESLFileInterface( m_pfile->Duplicate(), true ) ;
}

// ファイルから読み込み
//////////////////////////////////////////////////////////////////////////////
size_t SESLFileInterface::Read( void * ptrBuf, size_t nBytes )
{
	return	m_pfile->Read( ptrBuf, nBytes ) ;
}

// ファイルへ書き込み
//////////////////////////////////////////////////////////////////////////////
size_t SESLFileInterface::Write( const void * ptrBuf, size_t nBytes )
{
	return	m_pfile->Write( ptrBuf, nBytes ) ;
}

// シーク可能か否か？
//////////////////////////////////////////////////////////////////////////////
bool SESLFileInterface::IsSeekable( void ) const
{
	return	m_pfile->GetLargeLength() >= 0 ;
}

// ファイル長の取得
//////////////////////////////////////////////////////////////////////////////
int64_t SESLFileInterface::GetLength( void ) const
{
	return	m_pfile->GetLargeLength() ;
}

// ファイルポインタを移動
//////////////////////////////////////////////////////////////////////////////
int64_t SESLFileInterface::Seek
	( int64_t posFile, SSystem::SFileInterface::SeekOrigin seekFrom )
{
	return	m_pfile->SeekLarge
				( posFile, (ESLFileObject::SeekOrigin) seekFrom ) ;
}

// ファイルポインタを取得
//////////////////////////////////////////////////////////////////////////////
int64_t SESLFileInterface::GetPosition( void ) const
{
	return	m_pfile->GetLargePosition() ;
}

// ファイルの終端を現在の位置に設定する
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SESLFileInterface::SetEndOfFile( void )
{
	return	(SSystem::SError) m_pfile->SetEndOfFile() ;
}




