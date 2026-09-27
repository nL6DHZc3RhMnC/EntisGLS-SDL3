
/*****************************************************************************
             Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
	Copyright (c) 2002-2012 Leshade Entis, Entis-soft. All rights reserved.
 ****************************************************************************/


#include <gls.h>


//////////////////////////////////////////////////////////////////////////////
// スクリプト初期化
//////////////////////////////////////////////////////////////////////////////

static long int	g_nCotophaRefCount = 0 ;
static DWORD	g_dwCotophaTlsIndex = -1 ;
CRITICAL_SECTION	ECotophaScript::g_csCotopha ;
static CRITICAL_SECTION	g_csReference ;
static long int	g_fMultithreadReference = 0 ;
static HESLHEAP	g_hHeapCotopha = NULL ;
static HESLHEAP	g_hImportHeapCotopha = NULL ;
static EWaveMixingServer *	g_pCotophaWaveOutDev = NULL ;
static EGLDrawImage *		g_pCotophaDrawImage = NULL ;
static ECSContext *			g_pPrimaryContext = NULL ;

struct	COTOPHA_THREAD_DATA
{
	ECSThread *	pThread ;

	COTOPHA_THREAD_DATA( void )
	{
		pThread = NULL ;
	}
} ;

static const struct
{
	ECSStrTagArray **	pstaList ;
	const wchar_t **	ppwszName ;
}		g_fnlObjectFuncs[] =
{
	{
		&(ECSContext::m_staFuncName),
		&(ECSContext::m_pwszFuncName[0])
	},
	{
		&(ECSReference::m_staFuncName),
		&(ECSReference::m_pwszFuncName[0])
	},
	{
		&(ECSInteger::m_staFuncName),
		&(ECSInteger::m_pwszFuncName[0])
	},
	{
		&(ECSReal::m_staFuncName),
		&(ECSReal::m_pwszFuncName[0])
	},
	{
		&(ECSString::m_staFuncName),
		&(ECSString::m_pwszFuncName[0])
	},
	{
		&(ECSArray::m_staFuncName),
		&(ECSArray::m_pwszFuncName[0])
	},
	{
		&(ECSHash::m_staFuncName),
		&(ECSHash::m_pwszFuncName[0])
	},
	{
		&(ECSBuffer::m_staFuncName),
		&(ECSBuffer::m_pwszFuncName[0])
	},
	{
		&(ECSResource::m_staFuncName),
		&(ECSResource::m_pwszFuncName[0])
	},
	{
		&(ECSResourceManager::m_staFuncName),
		&(ECSResourceManager::m_pwszFuncName[0])
	},
	{
		&(ECSToneFilter::m_staFuncName),
		&(ECSToneFilter::m_pwszFuncName[0])
	},
	{
		&(ECSSprite::m_staFuncName),
		&(ECSSprite::m_pwszFuncName[0])
	},
	{
		&(ECSWindow::m_staFuncName),
		&(ECSWindow::m_pwszFuncName[0])
	},
	{
		&(ECSMessageSprite::m_staFuncName),
		&(ECSMessageSprite::m_pwszFuncName[0])
	},
	{
		&(ECSMovieSprite::m_staFuncName),
		&(ECSMovieSprite::m_pwszFuncName[0])
	},
	{
		&(ECSSuperSprite::m_staFuncName),
		&(ECSSuperSprite::m_pwszFuncName[0])
	},
	{
		&(ECSParticleSprite::m_staFuncName),
		&(ECSParticleSprite::m_pwszFuncName[0])
	},
	{
		&(ECSInputFilter::m_staFuncName),
		&(ECSInputFilter::m_pwszFuncName[0])
	},
	{
		&(ECSFile::m_staFuncName),
		&(ECSFile::m_pwszFuncName[0])
	},
	{
		&(ECSPolygonModel::m_staFuncName),
		&(ECSPolygonModel::m_pwszFuncName[0])
	},
	{
		&(ECSModelJoint::m_staFuncName),
		&(ECSModelJoint::m_pwszFuncName[0])
	},
	{
		&(ECSParticleModel::m_staFuncName),
		&(ECSParticleModel::m_pwszFuncName[0])
	},
	{
		&(ECSRenderSprite::m_staFuncName),
		&(ECSRenderSprite::m_pwszFuncName[0])
	},
	{
		&(ECSThread::m_staFuncName),
		&(ECSThread::m_pwszFuncName[0])
	},
	{
		&(ECSThreadEvent::m_staFuncName),
		&(ECSThreadEvent::m_pwszFuncName[0])
	},
	{
		&(ECSThreadMutex::m_staFuncName),
		&(ECSThreadMutex::m_pwszFuncName[0])
	},
	{
		&(ECSSetup::m_staFuncName),
		&(ECSSetup::m_pwszFuncName[0])
	},
	{
		NULL, NULL
	}
} ;

// 初期化
//////////////////////////////////////////////////////////////////////////////
void ECotophaScript::Initialize( DWORD dwFlags, HESLHEAP hImportHeap )
{
	if ( ::InterlockedIncrement( &g_nCotophaRefCount ) == 1 )
	{
		EGLMediaLoader::Initialize( ) ;
		ECSSakura2Processor::Initialize() ;
		//
		::InitializeCriticalSection( &g_csCotopha ) ;
		::InitializeCriticalSection( &g_csReference ) ;
		if ( (dwFlags != 0) && (hImportHeap == NULL) )
		{
			g_hHeapCotopha = ::eslHeapCreate( 0, 0, dwFlags ) ;
		}
		else
		{
			g_hHeapCotopha = hImportHeap ;
		}
		g_hImportHeapCotopha = hImportHeap ;
		//
		g_dwCotophaTlsIndex = ::TlsAlloc( ) ;
		//
		ECSResource::m_plstPlayRsrc = new EPtrObjArray<ECSResource> ;
		//
		for ( int i = 0; g_fnlObjectFuncs[i].pstaList != NULL; i ++ )
		{
			ECSStrTagArray *	pstrList = new ECSStrTagArray ;
			const wchar_t **	ppwszName = g_fnlObjectFuncs[i].ppwszName ;
			*(g_fnlObjectFuncs[i].pstaList) = pstrList ;
			pstrList->RemoveAll( ) ;
			for ( int j = 0; ppwszName[j] != NULL; j ++ )
			{
				pstrList->Add( ppwszName[j] ) ;
			}
		}
	}
}

// 終了
//////////////////////////////////////////////////////////////////////////////
void ECotophaScript::Release( void )
{
	if ( ::InterlockedDecrement( &g_nCotophaRefCount ) == 0 )
	{
		for ( int i = 0; g_fnlObjectFuncs[i].pstaList != NULL; i ++ )
		{
			delete	*(g_fnlObjectFuncs[i].pstaList) ;
			*(g_fnlObjectFuncs[i].pstaList) = NULL ;
		}
		delete	ECSResource::m_plstPlayRsrc ;
		ECSResource::m_plstPlayRsrc = NULL ;
		//
		::DeleteCriticalSection( &g_csReference ) ;
		::DeleteCriticalSection( &g_csCotopha ) ;
		::TlsFree( g_dwCotophaTlsIndex ) ;
		g_dwCotophaTlsIndex = (DWORD) -1 ;
		//
		__try
		{
			if ( g_pCotophaWaveOutDev != NULL )
			{
				ECSResource::SetWaveOutDevice( NULL ) ;
				g_pCotophaWaveOutDev->Close( ) ;
				delete	g_pCotophaWaveOutDev ;
			}
			if ( g_pCotophaDrawImage != NULL )
			{
				ECSMovieSprite::SetDrawImageObject( NULL ) ;
				g_pCotophaDrawImage->Release( ) ;
				delete	g_pCotophaDrawImage ;
			}
			if ( (g_hHeapCotopha != g_hImportHeapCotopha) )
			{
				::eslHeapDump( g_hHeapCotopha, 0x100 ) ;
				::eslHeapDestroy( g_hHeapCotopha ) ;
			}
		}
		__except ( EXCEPTION_EXECUTE_HANDLER )
		{
			::OutputDebugString
				( "ECotophaScript::Release 内で例外エラーが発生しました。\n" ) ;
		}
		g_pCotophaWaveOutDev = NULL ;
		g_pCotophaDrawImage = NULL ;
		g_pPrimaryContext = NULL ;
		g_hHeapCotopha = NULL ;
		//
		EGLMediaLoader::Close( ) ;
		ECSSakura2Processor::Close() ;
	}
}

// プライマリコンテキスト設定
//////////////////////////////////////////////////////////////////////////////
void ECotophaScript::SetPrimaryContext( ECSContext * context )
{
	g_pPrimaryContext = context ;
}

// プライマリコンテキスト設定
//////////////////////////////////////////////////////////////////////////////
ECSContext * ECotophaScript::GetPrimaryContext( void )
{
	return	g_pPrimaryContext ;
}

// カレントスレッド設定
//////////////////////////////////////////////////////////////////////////////
void ECotophaScript::SetCurrentThread( ECSThread * pThread )
{
	COTOPHA_THREAD_DATA *	pctd =
		(COTOPHA_THREAD_DATA*) ::TlsGetValue( g_dwCotophaTlsIndex ) ;
	if ( pctd == NULL )
	{
		pctd = new COTOPHA_THREAD_DATA ;
		::TlsSetValue( g_dwCotophaTlsIndex, pctd ) ;
	}
	pctd->pThread = pThread ;
}

// カレントスレッド取得
//////////////////////////////////////////////////////////////////////////////
ECSThread * ECotophaScript::GetCurrentThread( void )
{
	COTOPHA_THREAD_DATA *	pctd =
		(COTOPHA_THREAD_DATA*) ::TlsGetValue( g_dwCotophaTlsIndex ) ;
	if ( pctd != NULL )
	{
		return	pctd->pThread ;
	}
	return	NULL ;
}

// 音声出力オブジェクト生成
//////////////////////////////////////////////////////////////////////////////
EWaveMixingServer * ECotophaScript::OpenWaveDevice
	( unsigned int nBufferingTime, unsigned int nQuantumTime,
			unsigned int nFrequency,
			unsigned int nChannels, unsigned int nBitsPerSample )
{
	if ( g_pCotophaWaveOutDev == NULL )
	{
		g_pCotophaWaveOutDev = new EWaveMixingServer ;
		if ( g_pCotophaWaveOutDev->Open
			( nBufferingTime, nQuantumTime,
				nFrequency, nChannels, nBitsPerSample ) )
		{
			g_pCotophaWaveOutDev->OpenVirtualSync
				( nBufferingTime, nQuantumTime,
					nFrequency, nChannels, nBitsPerSample ) ;
		}
		ECSResource::SetWaveOutDevice( g_pCotophaWaveOutDev ) ;
	}
	return	g_pCotophaWaveOutDev ;
}

EWaveMixingServer * ECotophaScript::OpenDirectSound
	( unsigned int nBufferingTime, unsigned int nQuantumTime,
			unsigned int nFrequency,
			unsigned int nChannels, unsigned int nBitsPerSample )
{
	if ( g_pCotophaWaveOutDev == NULL )
	{
		g_pCotophaWaveOutDev = new EWaveMixingServer ;
		if ( !g_pCotophaWaveOutDev->OpenDirectSound
			( nBufferingTime, nQuantumTime,
				nFrequency, nChannels, nBitsPerSample ) )
		{
			ECSResource::SetWaveOutDevice( g_pCotophaWaveOutDev ) ;
		}
		else
		{
			delete	g_pCotophaWaveOutDev ;
			g_pCotophaWaveOutDev = NULL ;
		}
	}
	return	g_pCotophaWaveOutDev ;
}

EWaveMixingServer * ECotophaScript::GetWaveDevice( void )
{
	return	g_pCotophaWaveOutDev ;
}

// 画像描画オブジェクト生成
//////////////////////////////////////////////////////////////////////////////
EGLDrawImage * ECotophaScript::CreateDrawImage( void )
{
	if ( g_pCotophaDrawImage == NULL )
	{
		g_pCotophaDrawImage = new EGLDrawImage ;
		g_pCotophaDrawImage->Initialize( ) ;
		ECSMovieSprite::SetDrawImageObject( g_pCotophaDrawImage ) ;
	}
	return	g_pCotophaDrawImage ;
}

EGLDrawImage * ECotophaScript::GetDrawImage( void )
{
	return	g_pCotophaDrawImage ;
}

// ヒープメモリ取得
//////////////////////////////////////////////////////////////////////////////
/*HESLHEAP ECotophaScript::GetHeap( void )
{
	return	g_hHeapCotopha ;
}*/

// Reference 排他アクセス用
//////////////////////////////////////////////////////////////////////////////
void ECotophaScript::MultithreadReference( bool fMultithread )
{
	g_fMultithreadReference = fMultithread ;
}

void ECotophaScript::LockReference( void )
{
	if ( g_fMultithreadReference )
	{
		::EnterCriticalSection( &g_csReference ) ;
	}
}

void ECotophaScript::UnlockReference( void )
{
	if ( g_fMultithreadReference )
	{
		::LeaveCriticalSection( &g_csReference ) ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// 検索用インデックス付き文字列配列（追加のみ：非同期ヒープ）
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ECSStrTagArray, EPtrArray )

// 要素追加
//////////////////////////////////////////////////////////////////////////////
int ECSStrTagArray::Add( const wchar_t * pwszStr )
{
	int		nIndex = EPtrObjArray<const wchar_t>::Add( pwszStr ) ;
	//
	if ( pwszStr != NULL )
	{
		const wchar_t *	pwszElement ;
		int		iFirst = 0 ;
		int		iMiddle = 0 ;
		int		iEnd = m_idxSorted.GetSize() - 1 ;
		//
		while ( iFirst <= iEnd )
		{
			iMiddle = ((iFirst + iEnd) >> 1) ;
			pwszElement = GetAt( m_idxSorted.GetAt(iMiddle) ) ;
			ESLAssert( pwszElement != NULL ) ;
			//
			int	nCompare = EWideString::Compare( pwszStr, pwszElement ) ;
			if ( nCompare < 0 )
			{
				iEnd = iMiddle - 1 ;
			}
			else if ( nCompare > 0 )
			{
				iFirst = iMiddle + 1 ;
			}
			else
			{
				iFirst = iMiddle ;
				break ;
			}
		}
		m_idxSorted.InsertAt( iFirst, nIndex ) ;
	}
	//
	return	nIndex ;
}

// インデックスを検索する
//////////////////////////////////////////////////////////////////////////////
int ECSStrTagArray::FindIndex( const wchar_t * pwszStr ) const
{
	const wchar_t *	pwszElement ;
	int		iFirst, iEnd, iMiddle ;
	iFirst = 0 ;
	iEnd = m_idxSorted.GetSize() - 1 ;
	//
	while ( iFirst <= iEnd )
	{
		iMiddle = ((iFirst + iEnd) >> 1) ;
		pwszElement = GetAt( m_idxSorted.GetAt(iMiddle) ) ;
		ESLAssert( pwszElement != NULL ) ;
		//
		int	nCompare = EWideString::Compare( pwszElement, pwszStr ) ;
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
			return	m_idxSorted.GetAt(iMiddle) ;
		}
	}
	//
	return	-1 ;
}

// 全要素削除
//////////////////////////////////////////////////////////////////////////////
void ECSStrTagArray::RemoveAll( void )
{
	EPtrObjArray<const wchar_t>::RemoveAll( ) ;
	m_idxSorted.RemoveAll( ) ;
}

// 全配列をデバッグ出力する
//////////////////////////////////////////////////////////////////////////////
void ECSStrTagArray::TraceDebugOutput( void )
{
	for ( int i = 0; i < (int) m_idxSorted.GetSize(); i ++ )
	{
		const wchar_t *	pwszStr = GetAt( m_idxSorted.GetAt( i ) ) ;
		if ( pwszStr != NULL )
		{
			ESLTrace( "[%d] ; \"%s\"\n", i, EString(pwszStr).CharPtr() ) ;
		}
	}
}

// 配列保存
//////////////////////////////////////////////////////////////////////////////
ESLError ECSStrTagArray::SaveArray( ESLFileObject & file )
{
	DWORD	dwLength = GetSize( ) ;
	file.Write( &dwLength, sizeof(DWORD) ) ;
	for ( DWORD i = 0; i < dwLength; i ++ )
	{
		const wchar_t *	pwszText = GetAt( i ) ;
		DWORD	dwStrLen = 0 ;
		if ( pwszText != NULL )
		{
			while ( pwszText[dwStrLen] != L'\0' )
			{
				dwStrLen ++ ;
			}
		}
		file.Write( &dwStrLen, sizeof(DWORD) ) ;
		if ( dwStrLen > 0 )
		{
			file.Write( pwszText, dwStrLen * sizeof(wchar_t) ) ;
		}
	}
	return	eslErrSuccess ;
}

// 配列復元
//////////////////////////////////////////////////////////////////////////////
ESLError ECSStrTagArray::LoadArray
		( ESLFileObject & file, ECSContext & context )
{
	DWORD	dwLength ;
	if ( file.Read( &dwLength, sizeof(DWORD) ) < sizeof(DWORD) )
	{
		return	eslErrSuccess ;
	}
	for ( DWORD i = 0; i < dwLength; i ++ )
	{
		DWORD	dwStrLen ;
		if ( file.Read( &dwStrLen, sizeof(DWORD) ) < sizeof(DWORD) )
		{
			return	eslErrSuccess ;
		}
		const wchar_t *	pwszText = NULL ;
		if ( dwStrLen > 0 )
		{
			ECSWideString	wstrTemp ;
			file.Read
				( wstrTemp.GetBuffer( dwStrLen ),
						dwStrLen * sizeof(wchar_t) ) ;
			wstrTemp.ReleaseBuffer( dwStrLen ) ;
			//
			pwszText =
				context.GetConstantString( wstrTemp )->CharPtr() ;
		}
		Add( pwszText ) ;
	}
	return	eslErrSuccess ;
}

// メモリ確保
//////////////////////////////////////////////////////////////////////////////
void * ECSStrTagArray::operator new ( size_t stObj )
{
	return	::eslHeapAllocate( g_hHeapCotopha, stObj, 0 ) ;
}

void * ECSStrTagArray::operator new
	( size_t stObj, const char * pszFileName, int nLine )
{
	return	::eslHeapAllocate( g_hHeapCotopha, stObj, 0 ) ;
}

// メモリ解放
//////////////////////////////////////////////////////////////////////////////
void ECSStrTagArray::operator delete( void * ptrObj )
{
	::eslHeapFree( g_hHeapCotopha, ptrObj ) ;
}


//////////////////////////////////////////////////////////////////////////////
// Entis Cotopha Script ソース解析オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ECSSourceStream, EStreamWideString )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ECSSourceStream::ECSSourceStream( void )
{
}

ECSSourceStream::ECSSourceStream( const ECSSourceStream & cssSrc )
{
	operator = ( cssSrc ) ;
}

ECSSourceStream::ECSSourceStream( const wchar_t * pwszString )
{
	operator = ( pwszString ) ;
}

ECSSourceStream::ECSSourceStream( const char * pszString )
{
	operator = ( pszString ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ECSSourceStream::~ECSSourceStream( void )
{
	if ( m_pszString != NULL )
	{
		FreeString( ) ;
	}
}

// 代入操作
//////////////////////////////////////////////////////////////////////////////
const ECSSourceStream &
		ECSSourceStream::operator = ( const ECSSourceStream & cssSrc )
{
	EStreamWideString::operator = ( cssSrc ) ;
	return	*this ;
}

const ECSSourceStream &
		ECSSourceStream::operator = ( const wchar_t * pwszString )
{
	EStreamWideString::operator = ( pwszString ) ;
	return	*this ;
}

const ECSSourceStream &
		ECSSourceStream::operator = ( const char * pszString )
{
	EStreamWideString::operator = ( pszString ) ;
	return	*this ;
}

// 現在の文字列（区切り記号は無視）を通過する
//////////////////////////////////////////////////////////////////////////////
void ECSSourceStream::PassEnclosedString
		( wchar_t wchClose, int flagCtrlCode )
{
	while ( m_nIndex < m_nLength )
	{
		wchar_t	wch = m_pszString[m_nIndex] ;
		if ( wch == wchClose )
		{
			break ;
		}
		m_nIndex ++ ;
		//
		if ( flagCtrlCode & flagDisableExpression )
		{
			continue ;
		}
		if ( wchClose == L'\"' )
		{
			if ( (wch == L'\\') && (m_nIndex < m_nLength) )
			{
				m_nIndex ++ ;
			}
		}
		else if ( wchClose == L'\'' )
		{
			if ( !(flagCtrlCode & flagQuoteNakedString) )
			{
				if ( (wch == L'\\') && (m_nIndex < m_nLength) )
				{
					m_nIndex ++ ;
				}
			}
		}
		else if ( (wch == L'\"') || (wch == L'\'') )
		{
			PassEnclosedString( wch, flagCtrlCode ) ;
			if ( m_nIndex < m_nLength )
			{
				m_nIndex ++ ;
			}
		}
		else if ( wch == L'(' )
		{
			PassEnclosedString( L')', flagCtrlCode ) ;
			if ( m_nIndex < m_nLength )
			{
				m_nIndex ++ ;
			}
		}
		else if ( wch == L'[' )
		{
			PassEnclosedString( L']', flagCtrlCode ) ;
			if ( m_nIndex < m_nLength )
			{
				m_nIndex ++ ;
			}
		}
		else if ( wch == L'{' )
		{
			PassEnclosedString( L'}', flagCtrlCode ) ;
			if ( m_nIndex < m_nLength )
			{
				m_nIndex ++ ;
			}
		}
		/*
		else if ( wch == L'<' )
		{
			PassEnclosedString( L'>', flagCtrlCode ) ;
			if ( m_nIndex < m_nLength )
			{
				m_nIndex ++ ;
			}
		}
		*/
	}
}

// 現在のトークンを通過する
//////////////////////////////////////////////////////////////////////////////
void ECSSourceStream::PassAToken( int * pTokenType )
{
	wchar_t	wch = CurrentCharacter( ) ;
	if ( (wch == L'<') || (wch == L'>') )
	{
		wchar_t	wch1 = m_pszString[++ m_nIndex] ;
		if ( wch1 == L'=' )
		{
			m_nIndex ++ ;
		}
		else if ( wch1 == wch )
		{
			if ( m_pszString[++ m_nIndex] == L'=' )
			{
				m_nIndex ++ ;
			}
		}
		if( pTokenType != NULL )
		{
			*pTokenType = 2 ;
		}
		return ;
	}
	else if ( wch == L'-' )
	{
		wchar_t	wch1 = m_pszString[m_nIndex + 1] ;
		if ( wch1 == L'>' )
		{
			// ->
			m_nIndex += 2 ;
			//
			if ( m_pszString[m_nIndex] == L'*' )
			{
				// ->*
				m_nIndex ++ ;
			}
			if( pTokenType != NULL )
			{
				*pTokenType = 2 ;
			}
			return ;
		}
	}
	else if ( wch == L'.' )
	{
		if ( m_pszString[m_nIndex + 1] == L'*' )
		{
			// .*
			m_nIndex += 2 ;
			if( pTokenType != NULL )
			{
				*pTokenType = 2 ;
			}
			return ;
		}
	}
	else if ( wch == L':' )
	{
		wchar_t	wch1 = m_pszString[m_nIndex + 1] ;
		if ( wch1 == L':' )
		{
			wchar_t	wch2 = m_pszString[m_nIndex + 2] ;
			if ( wch2 == L'=' )
			{
				// ::=
				m_nIndex += 3 ;
				if( pTokenType != NULL )
				{
					*pTokenType = 2 ;
				}
				return ;
			}
			else
			{
				// ::
				m_nIndex += 2 ;
				if( pTokenType != NULL )
				{
					*pTokenType = 2 ;
				}
				return ;
			}
		}
	}
	EStreamWideString::PassAToken( pTokenType ) ;
}

void ECSSourceStream::PassAExpressionTerm( int flagCtrlCode )
{
	wchar_t	wch = CurrentCharacter( ) ;
	wchar_t	wchClose = 0 ;
//	wchar_t	wchNext ;
	switch ( wch )
	{
	case	L'\'':
	case	L'\"':
		wchClose = wch ;
		break ;
	case	L'(':
		wchClose = L')' ;
		break ;
	case	L'{':
		wchClose = L'}' ;
		break ;
	case	L'[':
		wchClose = L']' ;
		break ;
	/*
	case	L'<':
		wchNext = GetAt( GetIndex() + 1 ) ;
		if ( (wchNext != L'=') && (wchNext != L'<') )
		{
			wchClose = L'>' ;
		}
		break ;
	*/
	default:
		PassAToken( ) ;
		break ;
	}
	if ( wchClose )
	{
		GetCharacter( ) ;
		PassEnclosedString( wchClose, flagCtrlCode ) ;
		if ( CurrentCharacter() == wchClose )
		{
			GetCharacter( ) ;
		}
	}
}

// 文字列用バッファ確保
//////////////////////////////////////////////////////////////////////////////
void ECSSourceStream::AllocString( unsigned int nLength )
{
	if ( m_pszString != NULL )
	{
		m_pszString = (wchar_t*)
			::eslHeapReallocate
				( g_hHeapCotopha, m_pszString,
					((nLength + 1) * sizeof(wchar_t)), 0 ) ;
	}
	else
	{
		m_pszString = (wchar_t*)
			::eslHeapAllocate
				( g_hHeapCotopha, ((nLength + 1) * sizeof(wchar_t)), 0 ) ;
		if ( m_pszString != NULL )
		{
			m_pszString[0] = '\0' ;
		}
	}
	m_nBufLimit = ((m_pszString != NULL) ? nLength : 0) ;
}

void ECSSourceStream::FreeString( void )
{
	if ( m_pszString != NULL )
	{
		::eslHeapFree( g_hHeapCotopha, m_pszString ) ;
		m_pszString = NULL ;
		m_nLength = 0 ;
		m_nBufLimit = 0 ;
	}
}

// メモリ確保
//////////////////////////////////////////////////////////////////////////////
void * ECSSourceStream::operator new ( size_t stObj )
{
	return	::eslHeapAllocate( g_hHeapCotopha, stObj, 0 ) ;
}

void * ECSSourceStream::operator new
	( size_t stObj, const char * pszFileName, int nLine )
{
	return	::eslHeapAllocate( g_hHeapCotopha, stObj, 0 ) ;
}

// メモリ解放
//////////////////////////////////////////////////////////////////////////////
void ECSSourceStream::operator delete( void * ptrObj )
{
	::eslHeapFree( g_hHeapCotopha, ptrObj ) ;
}

// オブジェクトの複製
//////////////////////////////////////////////////////////////////////////////
ECSSourceStream * ECSSourceStream::Duplicate( void ) const
{
	ECSSourceStream *	pcss = new ECSSourceStream ;
	*pcss = *this ;
	return	pcss ;
}

