
/*****************************************************************************
             Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
	Copyright (c) 2002-2014 Leshade Entis, Entis-soft. All rights reserved.
 ****************************************************************************/


#include <gls.h>
#include <glscs/glscs_sakura2_jit_x86_compiler.h>
#include <glscs/glscs_sakura2_jit_sse2_compiler.h>


//////////////////////////////////////////////////////////////////////////////
// クラス情報を保持する擬似オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ECSExecutionImage::ECSClassInfoObject, ECSStructure )

// オブジェクトを複製
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSExecutionImage::ECSClassInfoObject::Duplicate( void )
{
	ECSExecutionImage::ECSClassInfoObject *
		pObj = new ECSExecutionImage::ECSClassInfoObject ;
	pObj->CopyFrom( *this ) ;
	//
	pObj->m_pwszTag = pObj->m_wstrClassName = m_pwszTag ;
	pObj->m_pClassInf = m_pClassInf ;
	//
	if ( m_pClassInf == NULL )
	{
		pObj->m_staMember = m_staMember ;
	}
	return	pObj ;
}


//////////////////////////////////////////////////////////////////////////////
// オブジェクト・ヒープを参照するオブジェクトを保存するためのダミークラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ECSExecutionImage::ECSHeapDummy, ECSArray )

// 全てのメンバ変数にインデックスを振る
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionImage::ECSHeapDummy::IndexAllMember( void )
{
	for ( size_t i = 0; i < m_pHeap->GetLength(); i ++ )
	{
		ECSObject *	pElement =
			ESLTypeCast<ECSObject>( m_pHeap->GetAt( i ) ) ;
		if ( pElement != NULL )
		{
			pElement->m_pParent = this ;
			pElement->m_nIndex = i ;
			pElement->IndexAllMember( ) ;
		}
	}
}


//////////////////////////////////////////////////////////////////////////////
// Entis Cotopha Script 実行イメージ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ECSExecutionImage, StandardVM )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ECSExecutionImage::ECSExecutionImage( void )
	: m_csaHeap( &m_heapGlobal ), m_csaHeapShared( &m_heapShared )
{
	m_exiHeader.nVersion = 1 ;
	m_exiHeader.nIntBase = 64 ;
	m_exiHeader.nContainerFlags =
			flagContainerExtRefClass | flagContainerImpRefFunc ;
	m_exiHeader.nReserved = 0 ;
	m_exiHeader.nStackSize = 0x1000 ;
	m_exiHeader.nHeapSize = 0x1000 ;
	m_exiHeader.fnEntryPoint = -1 ;
	m_exiHeader.fnStaticInitialize = -1 ;
	m_exiHeader.fnResumePrepare = -1 ;
	//
	m_pEnv = NULL ;
	//
	m_pImage = NULL ;
	m_dwImageSize = 0 ;
	//
	for ( int i = 0; i < 0x100; i ++ )
	{
		m_pAddressRootDirectory[i] = NULL ;
	}
	//
	m_pSystemContent = NULL ;
	//
	m_bufNativeCodes = NULL ;
	m_bufNativeGates = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ECSExecutionImage::~ECSExecutionImage( void )
{
	DeleteImage() ;
}

// メモリブロック確保
//////////////////////////////////////////////////////////////////////////////
INT64 ECSExecutionImage::AllocateHeapMemory
	( DWORD dwBytes, SSystem::MemoryAllocationMode mode )
{
	ECSBuffer *	pBuf = new ECSBuffer ;
	pBuf->CreateBuffer( dwBytes ) ;
	//
	INT64	addrAlloc ;
	ECSSakura2Processor::AssertLock() ;
	addrAlloc = AllocateHeapObjectAddress( pBuf ) ;
	ECSSakura2Processor::AssertUnlock() ;
	return	addrAlloc ;
}

// メモリブロック再確保
//////////////////////////////////////////////////////////////////////////////
INT64 ECSExecutionImage::ReallocateHeapMemory( INT64 addrBlock, DWORD dwBytes )
{
	if ( addrBlock == 0 )
	{
		return	AllocateHeapMemory( dwBytes ) ;
	}
	INT64		addrAlloc = 0 ;
	ECSBuffer *	pBuf =
		ESLTypeCast<ECSBuffer>
			( AtomicObjectFromAddress( (DWORD) (addrBlock >> 32) ) ) ;
	if ( pBuf != NULL )
	{
		ECSSakura2Processor::AssertLock() ;
		pBuf->ResizeBuffer( dwBytes ) ;
		addrAlloc = addrBlock ;
		ECSSakura2Processor::AssertUnlock() ;
	}
	return	addrAlloc ;
}

// メモリブロック解放
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionImage::FreeHeapMemory
	( INT64 addrBlock, ECSSakura2Processor::Context * context )
{
//	ECSSakura2Processor::AssertLock() ;
	FreeHeapObjectAddress( addrBlock, context ) ;
//	ECSSakura2Processor::AssertUnlock() ;
}

// 実行イメージを読み込む
//////////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImage::ReadExecution( ESLFileObject & file )
{
	//
	// 以前のデータを削除する
	//
	DeleteImage( ) ;
	//
	// EMC ファイルを開く
	//
	EMCFile	emcfile ;
	if ( emcfile.Open( &file ) )
	{
		return	eslErrGeneral ;
	}
	eslFillMemory( &m_exiHeader, 0, sizeof(m_exiHeader) ) ;
	//
	// 各レコードを読み込む
	//
	for ( ; ; )
	{
		if ( emcfile.DescendRecord( ) )
		{
			break ;
		}
		UINT64	idRec = emcfile.GetRecordID( ) ;
		if ( idRec == *((UINT64*)"header  ") )
		{
			//
			// ヘッダ
			//
			eslFillMemory( &m_exiHeader, 0, sizeof(m_exiHeader) ) ;
			m_exiHeader.nStackSize = 0x1000 ;
			m_exiHeader.nHeapSize = 0x1000 ;
			m_exiHeader.fnEntryPoint = -1 ;
			m_exiHeader.fnStaticInitialize = -1 ;
			m_exiHeader.fnResumePrepare = -1 ;
			//
			emcfile.Read( &m_exiHeader, sizeof(m_exiHeader) ) ;
		}
		else if ( idRec == *((UINT64*)"image   ") )
		{
			//
			// 実行イメージ
			//
			DWORD	dwLength = emcfile.GetLength( ) ;
			dwLength =
				emcfile.Read( m_bufImage.PutBuffer(dwLength), dwLength ) ;
			m_bufImage.Flush( dwLength ) ;
			m_pImage = (BYTE*) m_bufImage.ModifyBuffer( 0, dwLength ) ;
			m_dwImageSize = dwLength ;
		}
		else if ( idRec == *((UINT64*)"imgglobl") )
		{
			//
			// naked グローバル領域
			//
			DWORD	dwLength = emcfile.GetLength( ) ;
			void *	pbytInit = m_bufNakedGlobalInit.PutBuffer(dwLength) ;
			dwLength = emcfile.Read( pbytInit, dwLength ) ;
			::eslMoveMemory
				( m_bufNakedGlobal.PutBuffer(dwLength),
										pbytInit, dwLength ) ;
			m_bufNakedGlobalInit.Flush( dwLength ) ;
			m_bufNakedGlobal.Flush( dwLength ) ;
		}
		else if ( idRec == *((UINT64*)"imgconst") )
		{
			//
			// naked 不変グローバル領域
			//
			DWORD	dwLength = emcfile.GetLength( ) ;
			dwLength = emcfile.Read
				( m_bufNakedConst.PutBuffer(dwLength), dwLength ) ;
			m_bufNakedConst.Flush( dwLength ) ;
		}
		else if ( idRec == *((UINT64*)"imgshare") )
		{
			//
			// naked 共有グローバル領域
			//
			DWORD	dwLength = emcfile.GetLength( ) ;
			dwLength = emcfile.Read
				( m_bufNakedShared.PutBuffer(dwLength), dwLength ) ;
			m_bufNakedShared.Flush( dwLength ) ;
		}
		else if ( idRec == *((UINT64*)"classinf") )
		{
			//
			// クラス情報
			//
			DWORD	dwClassCount ;
			if ( emcfile.Read
				( &dwClassCount, sizeof(dwClassCount) )
										< sizeof(dwClassCount) )
			{
				return	eslErrGeneral ;
			}
			DWORD	i ;
			ESLError	err ;
			for ( i = 0; i < dwClassCount; i ++ )
			{
				ECSWideString	wstrGlobalName ;
				err = ReadWideString( emcfile, wstrGlobalName ) ;
				if ( err )
				{
					return	err ;
				}
				ECSClassInfo *	pClassInf = new ECSClassInfo ;
				pClassInf->SetGlobalName( wstrGlobalName ) ;
				int	iClassInf = AddClassInfo( pClassInf ) ;
				ESLAssert( (DWORD) iClassInf == i ) ;
			}
			//
			for ( i = 0; i < dwClassCount; i ++ )
			{
				ECSClassInfo *	pClassInf = GetClassInfoAt( i ) ;
				ESLAssert( pClassInf != NULL ) ;
				err = ReadClassInfo( emcfile, *pClassInf ) ;
				if ( err )
				{
					return	err ;
				}
			}
			for ( i = 0; i < m_lstDelayClassInfo.GetSize(); i ++ )
			{
				ECSDelayClassInfo *	pDelayClass = m_lstDelayClassInfo.GetAt(i) ;
				if ( pDelayClass != NULL )
				{
					RestoreDelayClassInfo( pDelayClass ) ;
				}
			}
			m_lstDelayClassInfo.RemoveAll() ;
		}
		else if ( idRec == *((UINT64*)"function") )
		{
			//
			// 初期化・終了関数リスト
			//
			ESLError	err ;
			err = ReadDWordArray( emcfile, m_pifPrologue ) ;
			if ( err )
				return	err ;
			//
			err = ReadDWordArray( emcfile, m_pifEpilogue ) ;
			if ( err )
				return	err ;
			//
			// 関数名リスト（互換）
			//
			DWORD	i, dwLength ;
			if ( emcfile.Read
				( &dwLength, sizeof(dwLength) ) < sizeof(dwLength) )
			{
				return	eslErrGeneral ;
			}
			for ( i = 0; i < dwLength; i ++ )
			{
				DWORD	dwFuncAddr ;
				if ( emcfile.Read
					( &dwFuncAddr, sizeof(DWORD) ) < sizeof(DWORD) )
				{
					return	eslErrGeneral ;
				}
				ECSWideString	wstrFuncName ;
				err = ReadWideString( emcfile, wstrFuncName ) ;
				if ( err )
					return	err ;
				//
				FUNC_ENTRY *	pfe = new FUNC_ENTRY ;
				pfe->dwAddress = dwFuncAddr ;
				m_wstaFunc.SetAs( wstrFuncName, pfe ) ;
			}
		}
		else if ( idRec == *((UINT64*)"initnfnc") )
		{
			//
			// naked 初期化関数リスト
			//
			ESLError	err ;
			err = ReadDWordArray( emcfile, m_pifNakedPrologue ) ;
			if ( err )
				return	err ;
			//
			err = ReadDWordArray( emcfile, m_pifNakedEpilogue ) ;
			if ( err )
				return	err ;
		}
		else if ( idRec == *((UINT64*)"funcinfo") )
		{
			//
			// 関数情報リスト（拡張）
			//
			DWORD	i, dwLength ;
			if ( emcfile.Read
				( &dwLength, sizeof(dwLength) ) < sizeof(dwLength) )
			{
				return	eslErrGeneral ;
			}
			for ( i = 0; i < dwLength; i ++ )
			{
				FUNC_ENTRY *	pfe = new FUNC_ENTRY ;
				if ( emcfile.Read
					( (FUNC_ENTRY_HEADER*) pfe,
						sizeof(FUNC_ENTRY_HEADER) ) < sizeof(FUNC_ENTRY_HEADER) )
				{
					delete	pfe ;
					return	eslErrGeneral ;
				}
				ECSWideString	wstrFuncName ;
				ESLError	err = ReadWideString( emcfile, wstrFuncName ) ;
				if ( err )
				{
					delete	pfe ;
					return	err ;
				}
				if ( pfe->dwReserved != 0 )
				{
					EStreamBuffer	buf ;
					pfe->dwReserved =
						emcfile.Read( buf.PutBuffer( pfe->dwReserved ), pfe->dwReserved ) ;
					FUNC_EXTENDED *	pfxLast = NULL ;
					for ( ; ; )
					{
						FUNC_EXTENDED *	pfxNext = new FUNC_EXTENDED ;
						if ( (buf.Read( &(pfxNext->dwID), sizeof(DWORD) ) < sizeof(DWORD))
							|| (buf.Read( &(pfxNext->dwBytes), sizeof(DWORD) ) < sizeof(DWORD)) )
						{
							delete	pfxNext ;
							break ;
						}
						pfxNext->ptrData = eslHeapAllocate( NULL, pfxNext->dwBytes, 0 ) ;
						buf.Read( pfxNext->ptrData, pfxNext->dwBytes ) ;
						if ( pfxLast == NULL )
						{
							pfe->ptrExtended = pfxNext ;
						}
						else
						{
							pfxLast->ptrNext = pfxNext ;
						}
						pfxLast = pfxNext ;
					}
				}
				m_wstaFunc.SetAs( wstrFuncName, pfe ) ;
			}
		}
		else if ( idRec == *((UINT64*)"symblinf") )
		{
			//
			// naked シンボル情報リスト
			//
			DWORD	i, dwLength ;
			if ( emcfile.Read
				( &dwLength, sizeof(dwLength) ) < sizeof(dwLength) )
			{
				return	eslErrGeneral ;
			}
			for ( i = 0; i < dwLength; i ++ )
			{
				NAKED_SYMBOL_INFO *	psi = new NAKED_SYMBOL_INFO ;
				if ( emcfile.Read
					( psi, sizeof(NAKED_SYMBOL_INFO) )
							< sizeof(NAKED_SYMBOL_INFO) )
				{
					delete	psi ;
					return	eslErrGeneral ;
				}
				ECSWideString	wstrSymName ;
				ESLError	err = ReadWideString( emcfile, wstrSymName ) ;
				if ( err )
				{
					delete	psi ;
					return	err ;
				}
				m_wstaSymbols.SetAs( wstrSymName, psi ) ;
			}
		}
		else if ( idRec == *((UINT64*)"global  ") )
		{
			//
			// グローバル変数
			//
			DWORD	i, dwCount ;
			if ( emcfile.Read
				( &dwCount, sizeof(dwCount) ) < sizeof(dwCount) )
			{
				return	eslErrGeneral ;
			}
			for ( i = 0; i < dwCount; i ++ )
			{
				ESLError	err ;
				ECSWideString	wstrName ;
				err = ReadWideString( emcfile, wstrName ) ;
				if ( err )
					return	err ;
				//
				ECSObject *	pObj ;
				err = ReadObject( emcfile, pObj ) ;
				if ( err )
					return	err ;
				//
				m_csgGlobalType.AddVariable( wstrName, pObj ) ;
			}
		}
		else if ( idRec == *((UINT64*)"data    ") )
		{
			//
			// 定数リスト
			//
			DWORD	i, dwCount ;
			if ( emcfile.Read
				( &dwCount, sizeof(dwCount) ) < sizeof(dwCount) )
			{
				return	eslErrGeneral ;
			}
			for ( i = 0; i < dwCount; i ++ )
			{
				ESLError	err ;
				ECSWideString	wstrName ;
				err = ReadWideString( emcfile, wstrName ) ;
				if ( err )
					return	err ;
				//
				DWORD	j, dwLength ;
				if ( emcfile.Read
					( &dwLength, sizeof(dwLength) ) < sizeof(dwLength) )
				{
					return	eslErrGeneral ;
				}
				if ( !(dwLength & 0x80000000) )
				{
					ECSGlobal *	pData = new ECSGlobal ;
					for ( j = 0; j < dwLength; j ++ )
					{
						ECSWideString	wstrTagName ;
						err = ReadWideString( emcfile, wstrTagName ) ;
						if ( err )
						{
							delete	pData ;
							return	err ;
						}
						//
						ECSObject *	pObj ;
						err = ReadObject( emcfile, pObj ) ;
						if ( err )
						{
							delete	pData ;
							return	err ;
						}
						//
						pData->AddVariable( wstrTagName, pObj ) ;
					}
					m_csgDataType.AddVariable( wstrName, pData ) ;
				}
				else
				{
					ECSObject *	pObj ;
					err = ReadObject( emcfile, pObj ) ;
					if ( err )
					{
						return	err ;
					}
					m_csgDataType.AddVariable( wstrName, pObj ) ;
				}
			}
		}
		else if ( idRec == *((UINT64*)"conststr") )
		{
			//
			// 固定文字列
			//
			DWORD	i, dwCount ;
			if ( emcfile.Read
				( &dwCount, sizeof(dwCount) ) < sizeof(dwCount) )
			{
				return	eslErrGeneral ;
			}
			for ( i = 0; i < dwCount; i ++ )
			{
				ECSWideString	wstrName ;
				ESLError	err = ReadWideString( emcfile, wstrName ) ;
				if ( err )
					return	err ;
				//
				ENumArray<DWORD> *	pList = new ENumArray<DWORD> ;
				err = ReadDWordArray( emcfile, *pList ) ;
				if ( err )
				{
					delete	pList ;
					return	err ;
				}
				m_lstConstStr.Add( new ECSString( wstrName ) ) ;
				m_extConstStr.SetAs( wstrName, pList ) ;
			}
		}
		else if ( idRec == *((UINT64*)"linkinf ") )
		{
			//
			// 大域変数・定数参照リスト
			//
			ESLError	err ;
			err = ReadDWordArray( emcfile, m_extGlobalRef ) ;
			if ( err )
				return	err ;
			//
			err = ReadDWordArray( emcfile, m_extDataRef ) ;
			if ( err )
				return	err ;
			//
			// 外部参照（未解決変数）
			//
			err = ReadTagedDWordArray( emcfile, m_impGlobalRef ) ;
			if ( err )
			{
				return	err ;
			}
			//
			// 外部参照（未解決定数）
			//
			err = ReadTagedDWordArray( emcfile, m_impDataRef ) ;
			if ( err )
			{
				return	err ;
			}
		}
		else if ( idRec == *((UINT64*)"linkex64") )
		{
			//
			// naked 大域変数・定数参照リスト
			//
			ESLError	err ;
			DWORD	dwFlags = 0 ;
			emcfile.Read( &dwFlags, sizeof(DWORD) ) ;
			//
			err = ReadDWordArray( emcfile, m_extNakedGlobalRef ) ;
			if ( err )
				return	err ;
			//
			err = ReadDWordArray( emcfile, m_extNakedConstRef ) ;
			if ( err )
				return	err ;
			//
			err = ReadDWordArray( emcfile, m_extNakedSharedRef ) ;
			if ( err )
				return	err ;
			//
			if ( dwFlags & 0x08 )
			{
				err = ReadDWordArray( emcfile, m_extNakedFuncRef ) ;
				if ( err )
					return	err ;
			}
			//
			// 外部参照（未解決変数）
			//
			err = ReadTagedDWordArray( emcfile, m_impNakedGlobalRef ) ;
			if ( err )
			{
				return	err ;
			}
			//
			// 外部参照（未解決定数）
			//
			err = ReadTagedDWordArray( emcfile, m_impNakedConstRef ) ;
			if ( err )
			{
				return	err ;
			}
			//
			// 外部参照（未解決共有変数）
			//
			err = ReadTagedDWordArray( emcfile, m_impNakedSharedRef ) ;
			if ( err )
			{
				return	err ;
			}
			//
			// 外部参照（未解決関数（64ビットアドレス））
			//
			if ( dwFlags & 0x080000 )
			{
				err = ReadTagedDWordArray( emcfile, m_impNakedFuncRef ) ;
				if ( err )
				{
					return	err ;
				}
			}
		}
		else if ( idRec == *((UINT64*)"reffunc ") )
		{
			//
			// 外部参照（未解決関数）
			//
			ESLError	err ;
			err = ReadTagedDWordArray( emcfile, m_impFuncRef ) ;
			if ( err )
			{
				return	err ;
			}
		}
		else if ( idRec == *((UINT64*)"refcode ") )
		{
			//
			// コード参照リスト
			//
			ESLError	err ;
			err = ReadDWordArray( emcfile, m_extCodeRef ) ;
			if ( err )
			{
				return	err ;
			}
		}
		else if ( idRec == *((UINT64*)"refclass") )
		{
			//
			// クラス情報参照リスト
			//
			ESLError	err ;
			err = ReadDWordArray( emcfile, m_extClassIndexRef ) ;
			if ( err )
			{
				return	err ;
			}
		}
		else if ( idRec == *((UINT64*)"impnativ") )
		{
			for ( ; ; )
			{
				ESLError	err ;
				if ( emcfile.DescendRecord( ) )
				{
					break ;
				}
				UINT64	idRec = emcfile.GetRecordID( ) ;
				if ( idRec == *((UINT64*)"nativfnc") )
				{
					//
					// ネイティブ関数参照
					//
					err = ReadWideStringArray
								( emcfile, m_staNativeFuncName ) ;
					if ( err )
					{
						break ;
					}
					err = ReadDWordArray( emcfile, m_impNativeFunc ) ;
					if ( err )
					{
						break ;
					}
				}
				else if ( idRec == *((UINT64*)"nakedfnc") )
				{
					//
					// naked ネイティブ関数参照
					//
					err = ReadWideStringArray
							( emcfile, m_vectorSysCall.GetEntryIndex() ) ;
					if ( err )
					{
						break ;
					}
					err = ReadDWordArray( emcfile, m_impNakedNativeFunc ) ;
					if ( err )
					{
						break ;
					}
				}
				emcfile.AscendRecord( ) ;
			}
		}
		emcfile.AscendRecord( ) ;
	}
	//
	emcfile.Close( ) ;
	return	eslErrSuccess ;
}

// 実行イメージを書き出す
//////////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImage::WriteExecution( ESLFileObject & file )
{
	//
	// EMC ファイルを開く
	//
	EMCFile					emcfile ;
	ESLError				err ;
	EMCFile::FILE_HEADER	fhHdr ;
	emcfile.SetFileHeader
		( fhHdr, EMCFile::fidUndefinedEMC, "Cotopha Image file" ) ;
	err = emcfile.Open( &file, &fhHdr ) ;
	if ( err )
	{
		return	err ;
	}
	do
	{
		//
		// ヘッダ
		//
		err = emcfile.DescendRecord( (UINT64*)"header  " ) ;
		if ( err )
			break ;
		//
		if ( emcfile.Write
			( &m_exiHeader, sizeof(m_exiHeader) ) < sizeof(m_exiHeader) )
		{
			err = eslErrGeneral ;
			break ;
		}
		emcfile.AscendRecord( ) ;
		//
		// 実行イメージ
		//
		err = emcfile.DescendRecord( (UINT64*)"image   " ) ;
		if ( err )
			break ;
		//
		if ( emcfile.Write( m_pImage, m_dwImageSize ) < m_dwImageSize )
		{
			err = eslErrGeneral ;
			break ;
		}
		emcfile.AscendRecord( ) ;
		//
		// naked グローバル領域
		//
		if ( m_bufNakedGlobal.GetLength() > 0 )
		{
			err = emcfile.DescendRecord( (UINT64*)"imgglobl" ) ;
			if ( err )
				break ;
			//
			if ( emcfile.Write
				( m_bufNakedGlobal.GetBuffer(),
					m_bufNakedGlobal.GetLength() )
							< m_bufNakedGlobal.GetLength() )
			{
				err = eslErrGeneral ;
				break ;
			}
			emcfile.AscendRecord( ) ;
		}
		//
		// naked 不変グローバル領域
		//
		if ( m_bufNakedConst.GetLength() > 0 )
		{
			err = emcfile.DescendRecord( (UINT64*)"imgconst" ) ;
			if ( err )
				break ;
			//
			if ( emcfile.Write
				( m_bufNakedConst.GetBuffer(),
					m_bufNakedConst.GetLength() )
							< m_bufNakedConst.GetLength() )
			{
				err = eslErrGeneral ;
				break ;
			}
			emcfile.AscendRecord( ) ;
		}
		//
		// naked 共有グローバル領域
		//
		if ( m_bufNakedShared.GetLength() > 0 )
		{
			err = emcfile.DescendRecord( (UINT64*)"imgshare" ) ;
			if ( err )
				break ;
			//
			if ( emcfile.Write
				( m_bufNakedShared.GetBuffer(),
					m_bufNakedShared.GetLength() )
							< m_bufNakedShared.GetLength() )
			{
				err = eslErrGeneral ;
				break ;
			}
			emcfile.AscendRecord( ) ;
		}
		//
		// クラス情報
		//
		err = emcfile.DescendRecord( (UINT64*)"classinf" ) ;
		if ( err )
			break ;
		//
		DWORD	i, dwLength ;
		dwLength = m_lstClassInfo.GetSize() ;
		if ( emcfile.Write
			( &dwLength, sizeof(dwLength) ) < sizeof(dwLength) )
		{
			err = eslErrGeneral ;
			break ;
		}
		for ( i = 0; i < dwLength; i ++ )
		{
			ECSClassInfo *	pClassInf = m_lstClassInfo.GetAt( i ) ;
			ESLAssert( pClassInf != NULL ) ;
			err = WriteWideString( emcfile, pClassInf->GetGlobalName() ) ;
			if ( err )
			{
				break ;
			}
		}
		if ( err )
		{
			break ;
		}
		for ( i = 0; i < dwLength; i ++ )
		{
			ECSClassInfo *	pClassInf = m_lstClassInfo.GetAt( i ) ;
			ESLAssert( pClassInf != NULL ) ;
			err = WriteClassInfo( emcfile, *pClassInf ) ;
			if ( err )
			{
				break ;
			}
		}
		if ( err )
		{
			break ;
		}
		emcfile.AscendRecord( ) ;
		//
		// 初期化・終了関数リスト
		//
		err = emcfile.DescendRecord( (UINT64*)"function" ) ;
		if ( err )
			return	err ;
		//
		err = WriteDWordArray( emcfile, m_pifPrologue ) ;
		if ( err )
			return	err ;
		//
		err = WriteDWordArray( emcfile, m_pifEpilogue ) ;
		if ( err )
			return	err ;
		//
		dwLength = 0 ;
		emcfile.Write( &dwLength, sizeof(dwLength) ) ;
		//
		emcfile.AscendRecord( ) ;
		//
		// naked 初期化関数リスト
		//
		err = emcfile.DescendRecord( (UINT64*)"initnfnc" ) ;
		if ( err )
			return	err ;
		//
		err = WriteDWordArray( emcfile, m_pifNakedPrologue ) ;
		if ( err )
			return	err ;
		//
		err = WriteDWordArray( emcfile, m_pifNakedEpilogue ) ;
		if ( err )
			return	err ;
		//
		emcfile.AscendRecord( ) ;
		//
		// 関数名リスト
		//
		err = emcfile.DescendRecord( (UINT64*)"funcinfo" ) ;
		if ( err )
			return	err ;
		//
		dwLength = m_wstaFunc.GetSize( ) ;
		emcfile.Write( &dwLength, sizeof(dwLength) ) ;
		//
		for ( i = 0; i < dwLength; i ++ )
		{
			ETaggedElement<ECSWideString,FUNC_ENTRY> *	pElement ;
			pElement = m_wstaFunc.GetAt( i ) ;
			ESLAssert( pElement != NULL ) ;
			FUNC_ENTRY *	pfe = pElement->GetObject() ;
			emcfile.Write( (FUNC_ENTRY_HEADER*) pfe, sizeof(FUNC_ENTRY_HEADER) ) ;
			err = WriteWideString( emcfile, pElement->Tag() ) ;
			if ( err )
			{
				break ;
			}
			if ( pfe->dwReserved != 0 )
			{
				EStreamBuffer	buf ;
				FUNC_EXTENDED *	pfxNext = pfe->ptrExtended ;
				while ( pfxNext != NULL )
				{
					buf.Write( &(pfxNext->dwID), sizeof(DWORD) ) ;
					buf.Write( &(pfxNext->dwBytes), sizeof(DWORD) ) ;
					buf.Write( pfxNext->ptrData, pfxNext->dwBytes ) ;
					pfxNext = pfxNext->ptrNext ;
				}
				while ( buf.GetLength() < pfe->dwReserved )
				{
					BYTE	bytZero = 0 ;
					buf.Write( &bytZero, sizeof(BYTE) ) ;
				}
				EPtrBuffer	ptrbuf = buf.GetBuffer() ;
				ESLAssert( ptrbuf.GetLength() >= pfe->dwReserved ) ;
				emcfile.Write( ptrbuf.GetBuffer(), pfe->dwReserved ) ;
			}
		}
		if ( err )
			break ;
		//
		emcfile.AscendRecord( ) ;
		//
		// naked シンボル情報リスト
		//
		err = emcfile.DescendRecord( (UINT64*)"symblinf" ) ;
		if ( err )
			return	err ;
		//
		dwLength = m_wstaSymbols.GetSize( ) ;
		emcfile.Write( &dwLength, sizeof(dwLength) ) ;
		//
		for ( i = 0; i < dwLength; i ++ )
		{
			ETaggedElement<ECSWideString,NAKED_SYMBOL_INFO> *	pElement ;
			pElement = m_wstaSymbols.GetAt( i ) ;
			ESLAssert( pElement != NULL ) ;
			emcfile.Write( pElement->GetObject(), sizeof(NAKED_SYMBOL_INFO) ) ;
			err = WriteWideString( emcfile, pElement->Tag() ) ;
			if ( err )
				break ;
		}
		if ( err )
			break ;
		//
		emcfile.AscendRecord( ) ;
		//
		// グローバル変数
		//
		err = emcfile.DescendRecord( (UINT64*)"global  " ) ;
		if ( err )
			break ;
		//
		dwLength = m_csgGlobalType.m_varArray.GetSize( ) ;
		emcfile.Write( &dwLength, sizeof(dwLength) ) ;
		//
		for ( i = 0; i < dwLength; i ++ )
		{
			const wchar_t *	pwszName =
					m_csgGlobalType.m_staObjName.GetAt( i ) ;
			ECSWideString	wstrName ;
			if ( pwszName != NULL )
				wstrName = pwszName ;
			//
			err = WriteWideString( emcfile, wstrName ) ;
			if ( err )
				break ;
			//
			ECSObject *	pObj = m_csgGlobalType.m_varArray.GetAt( i ) ;
			err = WriteObject( emcfile, pObj ) ;
			if ( err )
				break ;
		}
		if ( err )
			break ;
		//
		emcfile.AscendRecord( ) ;
		//
		// 定数リスト
		//
		err = emcfile.DescendRecord( (UINT64*)"data    " ) ;
		if ( err )
			break ;
		//
		dwLength = m_csgDataType.m_varArray.GetSize( ) ;
		emcfile.Write( &dwLength, sizeof(dwLength) ) ;
		//
		for ( i = 0; i < dwLength; i ++ )
		{
			const wchar_t *	pwszName = m_csgDataType.m_staObjName.GetAt( i ) ;
			ECSWideString	wstrName ;
			if ( pwszName != NULL )
				wstrName = pwszName ;
			//
			err = WriteWideString( emcfile, wstrName ) ;
			if ( err )
				break ;
			//
			ECSObject *	pObj = m_csgDataType.m_varArray.GetAt( i ) ;
			ECSGlobal *	pData = ESLTypeCast<ECSGlobal>( pObj ) ;
			ESLAssert( pObj != NULL ) ;
			//
			if ( pData != NULL )
			{
				DWORD	j, dwCount ;
				dwCount = pData->m_varArray.GetSize( ) ;
				emcfile.Write( &dwCount, sizeof(dwCount) ) ;
				//
				for ( j = 0; j < dwCount; j ++ )
				{
					const wchar_t *	pwszNameTag = pData->m_staObjName.GetAt( j ) ;
					ECSWideString	wstrNameTag ;
					if ( pwszNameTag != NULL )
						wstrNameTag = pwszNameTag ;
					//
					err = WriteWideString( emcfile, wstrNameTag ) ;
					if ( err )
						break ;
					//
					ECSObject *	pObjData = pData->m_varArray.GetAt( j ) ;
					err = WriteObject( emcfile, pObjData ) ;
					if ( err )
						break ;
				}
			}
			else
			{
				DWORD	dwDummy = 0x80000000 ;
				emcfile.Write( &dwDummy, sizeof(dwDummy) ) ;
				err = WriteObject( emcfile, pObj ) ;
			}
			if ( err )
				break ;
		}
		if ( err )
			break ;
		//
		emcfile.AscendRecord( ) ;
		//
		// 固定文字列リスト
		//
		err = emcfile.DescendRecord( (UINT64*)"conststr" ) ;
		if ( err )
			break ;
		//
		err = WriteTagedDWordArray( emcfile, m_extConstStr ) ;
		if ( err )
		{
			return	err ;
		}
		//
		emcfile.AscendRecord( ) ;
		//
		if ( err )
			break ;
		//
		// 大域変数・定数参照リスト
		//
		err = emcfile.DescendRecord( (UINT64*)"linkinf " ) ;
		if ( err )
			break ;
		//
		err = WriteDWordArray( emcfile, m_extGlobalRef ) ;
		if ( err )
			break ;
		//
		err = WriteDWordArray( emcfile, m_extDataRef ) ;
		if ( err )
			break ;
		//
		// 外部参照（未解決変数）
		//
		err = WriteTagedDWordArray( emcfile, m_impGlobalRef ) ;
		if ( err )
		{
			break ;
		}
		//
		// 外部参照（未解決定数）
		//
		err = WriteTagedDWordArray( emcfile, m_impDataRef ) ;
		if ( err )
		{
			break ;
		}
		//
		emcfile.AscendRecord( ) ;
		//
		// naked 大域変数・定数参照リスト
		//
		err = emcfile.DescendRecord( (UINT64*)"linkex64" ) ;
		if ( err )
			break ;
		//
		DWORD	dwNakedLinkFlags = 0x000F000F ;
		emcfile.Write( &dwNakedLinkFlags, sizeof(DWORD) ) ;
		//
		err = WriteDWordArray( emcfile, m_extNakedGlobalRef ) ;
		if ( err )
			break ;
		//
		err = WriteDWordArray( emcfile, m_extNakedConstRef ) ;
		if ( err )
			break ;
		//
		err = WriteDWordArray( emcfile, m_extNakedSharedRef ) ;
		if ( err )
			break ;
		//
		err = WriteDWordArray( emcfile, m_extNakedFuncRef ) ;
		if ( err )
			break ;
		//
		// 外部参照（未解決変数）
		//
		err = WriteTagedDWordArray( emcfile, m_impNakedGlobalRef ) ;
		if ( err )
		{
			break ;
		}
		//
		// 外部参照（未解決定数）
		//
		err = WriteTagedDWordArray( emcfile, m_impNakedConstRef ) ;
		if ( err )
		{
			break ;
		}
		//
		// 外部参照（未解決共有変数）
		//
		err = WriteTagedDWordArray( emcfile, m_impNakedSharedRef ) ;
		if ( err )
		{
			break ;
		}
		//
		// 外部参照（未解決関数（64ビットアドレス））
		//
		err = WriteTagedDWordArray( emcfile, m_impNakedFuncRef ) ;
		if ( err )
		{
			break ;
		}
		//
		emcfile.AscendRecord( ) ;
		//
		// 外部参照（未解決関数）
		//
		err = emcfile.DescendRecord( (UINT64*)"reffunc " ) ;
		if ( err )
		{
			break ;
		}
		err = WriteTagedDWordArray( emcfile, m_impFuncRef ) ;
		if ( err )
		{
			break ;
		}
		emcfile.AscendRecord( ) ;
		//
		// コード参照リスト
		//
		err = emcfile.DescendRecord( (UINT64*)"refcode " ) ;
		if ( err )
		{
			break ;
		}
		err = WriteDWordArray( emcfile, m_extCodeRef ) ;
		if ( err )
		{
			break ;
		}
		emcfile.AscendRecord( ) ;
		//
		// クラス情報参照リスト
		//
		err = emcfile.DescendRecord( (UINT64*)"refclass" ) ;
		if ( err )
		{
			break ;
		}
		err = WriteDWordArray( emcfile, m_extClassIndexRef ) ;
		if ( err )
		{
			break ;
		}
		emcfile.AscendRecord( ) ;
		//
		// ネイティブ関数参照
		//
		err = emcfile.DescendRecord( (UINT64*)"impnativ" ) ;
		if ( err )
		{
			break ;
		}
		err = emcfile.DescendRecord( (UINT64*)"nativfnc" ) ;
		if ( err )
		{
			break ;
		}
		err = WriteWideStringArray( emcfile, m_staNativeFuncName ) ;
		if ( err )
		{
			break ;
		}
		err = WriteDWordArray( emcfile, m_impNativeFunc ) ;
		if ( err )
		{
			break ;
		}
		emcfile.AscendRecord( ) ;
		//
		// naked ネイティブ関数参照
		//
		err = emcfile.DescendRecord( (UINT64*)"nakedfnc" ) ;
		if ( err )
		{
			break ;
		}
		err = WriteWideStringArray
				( emcfile, m_vectorSysCall.GetEntryIndex() ) ;
		if ( err )
		{
			break ;
		}
		err = WriteDWordArray( emcfile, m_impNakedNativeFunc ) ;
		if ( err )
		{
			break ;
		}
		emcfile.AscendRecord( ) ;
		//
		emcfile.AscendRecord( ) ;
	}
	while ( false ) ;
	//
	emcfile.Close( ) ;
	return	err ;
}

// 実行イメージを消去
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionImage::DeleteImage( void )
{
	m_exiHeader.nVersion = 1 ;
	m_exiHeader.nIntBase = 64 ;
	m_exiHeader.nContainerFlags =
			flagContainerExtRefClass | flagContainerImpRefFunc ;
	m_pImage = NULL ;
	m_dwImageSize = 0 ;
	//
	m_csgGlobal.RemoveAllVariable( ) ;
	m_csgData.RemoveAllVariable( ) ;
	m_lstConstStr.RemoveAll( ) ;
	//
	m_bufNakedGlobalInit.FreeBuffer() ;
	m_bufNakedGlobal.FreeBuffer() ;
	m_bufNakedConst.FreeBuffer() ;
	m_bufNakedShared.FreeBuffer() ;
	m_ptblNakedGlobals.RemoveAll() ;
	m_ptblNakedConsts.RemoveAll() ;
	m_ptblNakedShareds.RemoveAll() ;
	//
	m_bufImage.FreeBuffer( ) ;
	m_pifPrologue.RemoveAll( ) ;
	m_pifEpilogue.RemoveAll( ) ;
	m_pifNakedPrologue.RemoveAll( ) ;
	m_pifNakedEpilogue.RemoveAll( ) ;
	//
	m_csgGlobalType.RemoveAllVariable( ) ;
	m_csgDataType.RemoveAllVariable( ) ;
	m_wstaFunc.RemoveAll( ) ;
	m_lstClassInfo.RemoveAll( ) ;
	m_vectorNewObject.RemoveAll() ;
	//
	m_extCodeRef.RemoveAll() ;
	m_extGlobalRef.RemoveAll( ) ;
	m_extDataRef.RemoveAll( ) ;
	m_extClassIndexRef.RemoveAll( ) ;
	//
	m_staNativeFuncName.RemoveAll() ;
	m_vectorSysCall.RemoveAll() ;
	m_impNativeFunc.RemoveAll() ;
	m_impNakedNativeFunc.RemoveAll() ;
	//
	m_impNakedGlobalRef.RemoveAll() ;
	m_impNakedConstRef.RemoveAll() ;
	m_impNakedSharedRef.RemoveAll() ;
	m_impNakedFuncRef.RemoveAll() ;
	//
	m_impGlobalRef.RemoveAll( ) ;
	m_impDataRef.RemoveAll( ) ;
	m_impFuncRef.RemoveAll( ) ;
	m_extConstStr.RemoveAll( ) ;
	//
	delete	m_pSystemContent ;
	m_pSystemContent = NULL ;
	//
	delete	m_bufNativeCodes ;
	delete	m_bufNativeGates ;
	m_bufNativeCodes = NULL ;
	m_bufNativeGates = NULL ;
}

// 実行イメージ初期化
//////////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImage::InitializeExecution( ECSContext & context )
{
	int	i, nCount ;
	//
	// プラグインインスタンスの初期化
	//
	ECSEnvironment *	pEnv = GetCSEnvironment() ;
	if ( pEnv != NULL )
	{
		for ( i = 0; i < (int) pEnv->m_lstModule.GetSize(); i ++ )
		{
			ECSEnvironment::EPlugin *
				ppi = pEnv->m_lstModule.GetAt( i ) ;
			if ( (ppi != NULL) && (ppi->m_ppiet != NULL) && !ppi->m_fStartup )
			{
				ppi->m_ppiet->pfnStartup( context.GetContextInterface() ) ;
				ppi->m_fStartup = true ;
			}
		}
	}
	//
	// オブジェクト・ページテーブル初期化
	//
	m_ptblNull.RemoveAll() ;
	m_ptblCodeImages.RemoveAll() ;
	m_ptblCodeImages.SetAt( 0, &m_bufImage ) ;
	//
	m_ptblNakedGlobals.RemoveAll() ;
	m_ptblNakedGlobals.SetAt( 0, &m_bufNakedGlobal ) ;
	//
	m_ptblNakedConsts.RemoveAll() ;
	m_ptblNakedConsts.SetAt( 0, &m_bufNakedConst ) ;
	//
	m_ptblNakedShareds.RemoveAll() ;
	m_ptblNakedShareds.SetAt( 0, &m_bufNakedShared ) ;
	//
	InitializeDirectoryTable() ;
	//
	m_pAddressRootDirectory[roasObjectGlobal] =
		(SSystem::SPointerArray<ECSSakura2::Object>*) &(m_csgGlobal.m_varArray) ;
	m_pAddressRootDirectory[roasObjectData] =
		(SSystem::SPointerArray<ECSSakura2::Object>*) &(m_csgData.m_varArray) ;
	//
	// プライマリ・モジュール情報設定
	//
	::eslMoveMemory
		( &(m_module.m_exmHeader), &m_exiHeader,
			__min(sizeof(HEADER),
				sizeof(ECSSakura2::ExecutableModule::HEADER)) ) ;
	//
	AttachModuleAt( 0, &m_module ) ;
	//
	// クラス情報の初期化
	//
	ESLError	err ;
	err = InitializeClassInfo() ;
	if ( err )
	{
		return	err ;
	}
	err = InitializeNativeClass( context ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// （大域）共有データ領域の初期化
	//
	nCount = m_csgDataType.m_varArray.GetSize( ) ;
	m_csgData.m_varArray.RemoveAll( ) ;
	m_csgData.m_staObjName.RemoveAll( ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		ECSWideString	wstrName ;
		const wchar_t *	pwszName = m_csgDataType.m_staObjName.GetAt( i ) ;
		if ( pwszName != NULL )
		{
			wstrName = pwszName ;
		}
		//
		ECSObject *	pType = m_csgDataType.m_varArray.GetAt( i ) ;
		ESLAssert( pType != NULL ) ;
		if ( ESLTypeCast<ECSGlobal>( pType ) == NULL )
		{
			ECSObject *	pObj ;
			err = context.CreateInstanceFromTypeInfo( pObj, pType ) ;
			if ( err )
			{
				return	err ;
			}
			m_csgData.AddVariable( wstrName, pObj ) ;
			//
			if ( pObj->m_vtType == csvtReference )
			{
				((ECSReference*)pObj)->m_fNontemp = true ;
			}
		}
		else
		{
			m_csgData.AddVariable( wstrName, pType->Duplicate() ) ;
		}
	}
	//
	// 大域データ領域の初期化
	//
	nCount = m_csgGlobalType.m_varArray.GetSize( ) ;
	m_csgGlobal.m_varArray.RemoveAll( ) ;
	m_csgGlobal.m_staObjName.RemoveAll( ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		ECSWideString	wstrName ;
		const wchar_t *	pwszName = m_csgGlobalType.m_staObjName.GetAt( i ) ;
		if ( pwszName != NULL )
		{
			wstrName = pwszName ;
		}
		//
		ECSObject *	pType =
			m_csgGlobalType.m_varArray.GetAt( i ) ;
		ECSObject *	pObj ;
		err = context.CreateInstanceFromTypeInfo( pObj, pType ) ;
		if ( err )
		{
			return	err ;
		}
		m_csgGlobal.AddVariable( wstrName, pObj ) ;
		//
		if ( pObj->m_vtType == csvtReference )
		{
			((ECSReference*)pObj)->m_fNontemp = true ;
		}
	}
	//
	// naked 大域データ領域の初期化
	//
	::eslMoveMemory
		( m_bufNakedGlobal.GetBuffer(),
			m_bufNakedGlobalInit.GetBuffer(),
			m_bufNakedGlobal.GetLength() ) ;
	//
	return	eslErrSuccess ;
}

// 実行リソースの解放
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionImage::ReleaseExecution( ECSContext & context )
{
	//
	// グローバル変数の消去
	//
	m_csgGlobal.CleanupAllReference( context ) ;
	m_csgGlobal.RemoveAllVariable( ) ;
	m_csgData.CleanupAllReference( context ) ;
	m_csgData.RemoveAllVariable( ) ;
	//
	// プラグインインスタンスの解放
	//
	ECSEnvironment *	pEnv = GetCSEnvironment() ;
	if ( pEnv != NULL )
	{
		for ( int i = 0; i < (int) pEnv->m_lstModule.GetSize(); i ++ )
		{
			ECSEnvironment::EPlugin *
				ppi = pEnv->m_lstModule.GetAt( i ) ;
			if ( (ppi != NULL) && (ppi->m_ppiet != NULL) && ppi->m_fStartup )
			{
				ppi->m_ppiet->pfnShutdown( context.GetContextInterface() ) ;
				ppi->m_fStartup = false ;
			}
		}
	}
}

// ネイティブクラス情報初期化
//////////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImage::InitializeNativeClass( ECSContext & context )
{
	int	i, nCount ;
	nCount = GetClassInfoCount() ;
	for ( i = 0; i < nCount; i ++ )
	{
		ECSClassInfo *	pClassInf = GetClassInfoAt( i ) ;
		ECSObject *	pClassObj = NULL ;
		if ( pClassInf == NULL )
		{
			continue ;
		}
		if ( pClassInf->IsNakedMemoryClass() )
		{
			continue ;
		}
		if ( pClassInf->GetAttribute() & ECSTypeInfo::flagNativeObject )
		{
			pClassObj = context.CreateObject
							( csvtObject, pClassInf->GetGlobalName() ) ;
			if ( pClassObj == NULL )
			{
				m_strErrMsg =
					EString( pClassInf->GetGlobalName() )
							+ " クラス情報を初期化できませんでした。" ;
				ESLTrace( "%s\n", m_strErrMsg.CharPtr() ) ;
//				return	ESLErrorMsg( m_strErrMsg ) ;
			}
		}
		EWStrTagArray<ECSObject>	wstaClassObject ;
		//
		unsigned int	j, m ;
		m = pClassInf->GetFunctionCount() ;
		for ( j = 0; j < m; j ++ )
		{
			ECSClassInfo::MemberFunction *
					pFunc = pClassInf->GetFunctionAt( j ) ;
			if ( pFunc == NULL )
			{
				m_strErrMsg =
					EString( pFunc->GetGlobalName() )
							+ " 関数を初期化できませんでした。" ;
				delete	pClassObj ;
				return	ESLErrorMsg( m_strErrMsg ) ;
			}
			if ( !(pFunc->GetAttribute() & ECSTypeInfo::flagNativeObject) )
			{
				continue ;
			}
			if ( (pClassInf->GetAttribute() & ECSTypeInfo::flagNativeObject)
				&& (pFunc->GetAttribute() & ECSTypeInfo::flagNakedCall) )
			{
				continue ;
			}
			ECSObject *	pFuncClass = NULL ;
			if ( pClassObj != NULL )
			{
				pFuncClass = pClassObj ;
				//
				pFunc->m_fpFuncPointer.m_ftType =
								ECS_FUNCTION_POINTER::funcIndexCall ;
				pFunc->m_fpFuncPointer.m_castThis =
											ECS_CAST_INTERFACE( NULL ) ;
			}
			else
			{
				pFuncClass = wstaClassObject.GetAs( pFunc->m_wstrClass ) ;
				if ( pFuncClass == NULL )
				{
					pFuncClass = context.CreateObject
									( csvtObject, pFunc->m_wstrClass ) ;
					if ( pFuncClass == NULL )
					{
						m_strErrMsg =
							EString( pFunc->m_wstrClass )
									+ " クラス情報を初期化できませんでした。" ;
						return	ESLErrorMsg( m_strErrMsg ) ;
					}
					wstaClassObject.SetAs( pFunc->m_wstrClass, pFuncClass ) ;
				}
				pFunc->m_fpFuncPointer.m_ftType =
								ECS_FUNCTION_POINTER::funcIndexCall ;
			}
			ESLError	err =
				pFuncClass->GetFunction
					( context, pFunc->m_fpFuncPointer.m_varFunc.nIndex,
														pFunc->GetName() ) ;
			if ( err )
			{
				m_strErrMsg =
					EString( pFunc->GetGlobalName() )
							+ " 関数が見つかりませんでした。" ;
				delete	pClassObj ;
				return	ESLErrorMsg( m_strErrMsg ) ;
			}
		}
		delete	pClassObj ;
	}
	return	eslErrSuccess ;
}

// クラス情報初期化
//////////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImage::InitializeClassInfo( void )
{
	int	i, nCount ;
	nCount = GetClassInfoCount() ;
	for ( i = 0; i < nCount; i ++ )
	{
		ECSClassInfo *	pClassInf = GetClassInfoAt( i ) ;
		if ( pClassInf == NULL )
		{
			continue ;
		}
		pClassInf->UpdateDestructorList() ;
		//
		if ( pClassInf->IsNakedMemoryClass() )
		{
			ECSExecutionImage::FUNC_ENTRY *	pInitEntry =
				GetFunctionEntry
					( pClassInf->GetGlobalName() + L"::<image>" ) ;
			if ( pInitEntry != NULL )
			{
				pClassInf->m_dwNakedInitAddr = pInitEntry->dwAddress ;
			}
			else
			{
				m_strErrMsg =
					EString( pClassInf->GetGlobalName() )
							+ " クラスの初期値情報が見つかりません。" ;
				return	ESLErrorMsg( m_strErrMsg ) ;
			}
		}
	}
	return	eslErrSuccess ;
}

// クラス名からクラス ID を取得
//////////////////////////////////////////////////////////////////////////////
int ECSExecutionImage::GetClassIdentity( const wchar_t * pwszClassName ) const
{
	return	GetClassInfoIndex( pwszClassName ) ;
}

// クラス ID を追加
//////////////////////////////////////////////////////////////////////////////
int ECSExecutionImage::AddClassIdentity( const wchar_t * pwszClassName )
{
	int	idClass = GetClassInfoIndex( pwszClassName ) ;
	if ( idClass >= 0 )
	{
		return	idClass ;
	}
	return	m_vectorNewObject.AddEntry( pwszClassName ) ;
}

// クラス ID からオブジェクトを生成
//////////////////////////////////////////////////////////////////////////////
ECSSakura2::Object *
	ECSExecutionImage::NewObjectByIdentity
		( ECSSakura2Processor::Context * context, int cls_id )
{
	ECSSakura2::Object *
		pObj = StandardVM::NewObjectByIdentity( context, cls_id ) ;
	if ( pObj != NULL )
	{
		return	pObj ;
	}
	ECSClassInfo *	pClassInf = GetClassInfoAt( cls_id ) ;
	if ( pClassInf != NULL )
	{
		return	((ECSContext*)context)->CreateClassObject( *pClassInf ) ;
	}
	return	NULL ;
}

// エクスポート関数取得
//////////////////////////////////////////////////////////////////////////////
void * ECSExecutionImage::GetModuleExportFunction( const char * pszFuncName )
{
	ESLAssert( m_pEnv != NULL ) ;
	return	m_pEnv->FindPluginedFunction( pszFuncName ) ;
}

// オブジェクト仮想アドレスディレクトリ・アロケーション
//////////////////////////////////////////////////////////////////////////////
INT64 ECSExecutionImage::AllocateVirtualAddressDirectory
		( SSystem::SPointerArray<ECSObject> * plstDirectory )
{
	for ( int i = roasFree; i < 0x100; i ++ )
	{
		if ( m_pAddressRootDirectory[i] == &m_ptblNull )
		{
			m_pAddressRootDirectory[i] =
				(SSystem::SPointerArray<ECSSakura2::Object>*) plstDirectory ;
			return	((INT64) i) << 56 ;
		}
		ESLAssert( (void*) m_pAddressRootDirectory[i] != (void*) plstDirectory ) ;
	}
	ESLTrace( "overflow cotopha virtual allocation" ) ;
	return	0 ;
}

INT64 ECSExecutionImage::AllocateVirtualAddressDirectory
		( INT64 nAddr, SSystem::SPointerArray<ECSObject> * plstDirectory )
{
	int	i = (int) (nAddr >> 56) ;
	if ( (void*) m_pAddressRootDirectory[i] == (void*) plstDirectory )
	{
		return	((INT64) i) << 56 ;
	}
	if ( m_pAddressRootDirectory[i] == &m_ptblNull )
	{
		m_pAddressRootDirectory[i] =
			(SSystem::SPointerArray<ECSSakura2::Object>*) plstDirectory ;
		return	((INT64) i) << 56 ;
	}
	ESLTrace( "virtual address root #%d is already allocated.\n", i ) ;
	return	AllocateVirtualAddressDirectory( plstDirectory ) ;
}

// オブジェクト仮想アドレスディレクトリ解放
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionImage::FreeVirtualAddressDirectory
		( INT64 nAddress, SSystem::SPointerArray<ECSObject> * plstDirectory )
{
	const int	nSel = (int) (nAddress >> 56) & 0xFF ;
	ESLAssert( (void*) m_pAddressRootDirectory[nSel] == (void*) plstDirectory ) ;
	if ( (void*) m_pAddressRootDirectory[nSel] == (void*) plstDirectory )
	{
		m_pAddressRootDirectory[nSel] = &m_ptblNull ;
	}
}

// システムコンテキストを取得
//////////////////////////////////////////////////////////////////////////////
ECSContext * ECSExecutionImage::GetSystemContext( void )
{
	if ( m_pSystemContent == NULL )
	{
		m_pSystemContent = new ECSContext ;
		m_pSystemContent->InitializeContext( this, false ) ;
	}
	return	m_pSystemContent ;
}

// クラス名ベクタを保存する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImage::SaveClassVector( ESLFileObject & file )
{
	SESLFileInterface	sfile( &file ) ;
	return	(ESLError) SaveNewObjectVector( &sfile ) ;
}

// 自由領域を保存する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImage::SaveHeapMemory( ECSContext& context, ESLFileObject & file )
{
	SESLFileInterface	sfile( &file ) ;
	//
	int	nErr = 0 ;
	nErr += m_heapGlobal.PrepareSave( this, &context ) ;
	//
	nErr += m_heapGlobal.SaveHeapStatic( &sfile, this, &context ) ;
	//
	if ( nErr > 0 )
	{
		return	eslErrFailed ;
	}
	return	eslErrSuccess ;
}

// クラス名ベクタを復元する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImage::LoadClassVector( ESLFileObject & file )
{
	SESLFileInterface	sfile( &file ) ;
	return	(ESLError) LoadNewObjectVector( &sfile ) ;
}

// 自由領域を復元する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImage::LoadHeapMemory( ECSContext& context, ESLFileObject & file )
{
	SESLFileInterface	sfile( &file ) ;
	//
	int	nErr = 0 ;
	nErr += m_heapGlobal.LoadHeapStatic( &sfile, this, &context ) ;
	if ( nErr > 0 )
	{
		return	eslErrFailed ;
	}
	//
	nErr += m_heapGlobal.CommitAfterLoad( this, &context ) ;
	//
	nErr += m_heapGlobal.OnLoadedDynamic( this, &context ) ;
	//
	if ( nErr > 0 )
	{
		return	eslErrFailed ;
	}
	return	eslErrSuccess ;
}

// 数値配列を読み込む
//////////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImage::ReadDWordArray
	( ESLFileObject & file, ENumArray<DWORD> & array )
{
	DWORD	dwLength ;
	if ( file.Read( &dwLength, sizeof(dwLength) ) < sizeof(dwLength) )
	{
		return	eslErrGeneral ;
	}
	if ( dwLength != 0 )
	{
		EStreamBuffer	buf ;
		DWORD *	pdwArray = (DWORD*) buf.PutBuffer( dwLength * sizeof(DWORD) ) ;
		if ( file.Read( pdwArray,
				dwLength * sizeof(DWORD) ) < dwLength * sizeof(DWORD) )
		{
			return	eslErrGeneral ;
		}
		array.SetSize( dwLength ) ;
		for ( DWORD i = 0; i < dwLength; i ++ )
		{
			array.SetAt( i, pdwArray[i] ) ;
		}
	}
	return	eslErrSuccess ;
}

// 文字列を読み込む
//////////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImage::ReadWideString
	( ESLFileObject & file, ECSWideString & wstr )
{
	DWORD	dwLength ;
	if ( file.Read( &dwLength, sizeof(dwLength) ) < sizeof(dwLength) )
	{
		return	eslErrGeneral ;
	}
	if ( file.Read( wstr.GetBuffer(dwLength),
			dwLength * sizeof(wchar_t) ) < dwLength * sizeof(wchar_t) )
	{
		return	eslErrGeneral ;
	}
	wstr.ReleaseBuffer( dwLength ) ;
	return	eslErrSuccess ;
}

// 文字列配列を読み込む
//////////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImage::ReadWideStringArray
	( ESLFileObject & file, ECSStrBufTagArray & array )
{
	DWORD	i, dwCount ;
	if ( file.Read( &dwCount, sizeof(dwCount) ) < sizeof(dwCount) )
	{
		return	eslErrGeneral ;
	}
	for ( i = 0; i < dwCount; i ++ )
	{
		ECSWideString	wstrName ;
		ESLError	err ;
		err = ReadWideString( file, wstrName ) ;
		if ( err )
		{
			return	err ;
		}
		array.Add( wstrName ) ;
	}
	return	eslErrSuccess ;
}

ESLError ECSExecutionImage::ReadWideStringArray
	( ESLFileObject & file,
		SSystem::SIndexedArray<SSystem::SString,const wchar_t*> & array )
{
	DWORD	i, dwCount ;
	if ( file.Read( &dwCount, sizeof(dwCount) ) < sizeof(dwCount) )
	{
		return	eslErrGeneral ;
	}
	for ( i = 0; i < dwCount; i ++ )
	{
		ECSWideString	wstrName ;
		ESLError	err ;
		err = ReadWideString( file, wstrName ) ;
		if ( err )
		{
			return	err ;
		}
		array.Add( new SSystem::SString( wstrName ) ) ;
	}
	return	eslErrSuccess ;
}

// タグ付き数値配列を読み込む
//////////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImage::ReadTagedDWordArray
	( ESLFileObject & file, TaggedRefAddresList & array )
{
	DWORD	i, dwCount ;
	if ( file.Read( &dwCount, sizeof(dwCount) ) < sizeof(dwCount) )
	{
		return	eslErrGeneral ;
	}
	for ( i = 0; i < dwCount; i ++ )
	{
		ECSWideString	wstrName ;
		ESLError	err ;
		err = ReadWideString( file, wstrName ) ;
		if ( err )
		{
			return	err ;
		}
		ENumArray<DWORD> *	pList = new ENumArray<DWORD> ;
		err = ReadDWordArray( file, *pList ) ;
		if ( err )
		{
			delete	pList ;
			return	err ;
		}
		array.SetAs( wstrName, pList ) ;
	}
	return	eslErrSuccess ;
}

// オブジェクトを読み込む
//////////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImage::ReadObject
	( ESLFileObject & file, ECSObject *& pObj )
{
	if ( m_exiHeader.nIntBase == 64 )
	{
		return	ReadTypeObject( file, pObj ) ;
	}
	CSVariableType	csvtType ;
	if ( file.Read( &csvtType, sizeof(csvtType) ) < sizeof(csvtType) )
	{
		return	eslErrGeneral ;
	}
	switch ( csvtType )
	{
	case	csvtInteger:
		{
			ECSInteger *	pInt = new ECSInteger ;
			int	varInt ;
			if ( file.Read
				( &varInt, sizeof(int) ) < sizeof(int) )
			{
				delete	pInt ;
				return	eslErrGeneral ;
			}
			pInt->SetValue( varInt ) ;
			pObj = pInt ;
		}
		break ;
	case	csvtReal:
		{
			ECSReal *	pReal = new ECSReal ;
			if ( file.Read( &(pReal->m_varReal),
					sizeof(pReal->m_varReal) ) < sizeof(pReal->m_varReal) )
			{
				delete	pReal ;
				return	eslErrGeneral ;
			}
			pObj = pReal ;
		}
		break ;
	case	csvtString:
		{
			ECSString *	pStr = new ECSString ;
			if ( ReadWideString( file, pStr->m_varStr ) )
			{
				delete	pStr ;
				return	eslErrGeneral ;
			}
			pObj = pStr ;
		}
		break ;
	case	csvtReference :
		pObj = new ECSReference ;
		break ;
	case	csvtArray:
		pObj = new ECSArray ;
		{
			int	nLength ;
			if ( file.Read( &nLength, sizeof(int) ) < sizeof(int) )
			{
				delete	pObj ;
				return	eslErrGeneral ;
			}
			for ( int i = 0; i < nLength; i ++ )
			{
				ECSObject *	pElement ;
				if ( ReadObject( file, pElement ) )
				{
					delete	pObj ;
					return	eslErrGeneral ;
				}
				((ECSArray*)pObj)->m_varArray.Add( pElement ) ;
			}
		}
		break ;
	case	csvtHash:
		pObj = new ECSHash ;
		break ;
	case	csvtObject:
		{
			ECSClassInfoObject *	pClassObj = new ECSClassInfoObject ;
			if ( ReadWideString( file, pClassObj->m_wstrClassName ) )
			{
				delete	pClassObj ;
				return	eslErrGeneral ;
			}
			pClassObj->m_pwszTag = pClassObj->m_wstrClassName ;
			pClassObj->m_pClassInf =
					GetClassInfoAs( pClassObj->m_wstrClassName ) ;
			pObj = pClassObj ;
		}
		break ;
	default:
		return	eslErrGeneral ;
	}
	return	eslErrSuccess ;
}

// 型情報を読み込む
//////////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImage::ReadTypeObject
	( ESLFileObject & file, ECSObject *& pObj )
{
	CSVariableType	csvtType ;
	if ( file.Read( &csvtType, sizeof(csvtType) ) < sizeof(csvtType) )
	{
		return	eslErrGeneral ;
	}
	ESLError		err ;
	INT64			nValue, nMask ;
	REAL64			rValue ;
	ECSWideString	wstrValue ;
	switch ( csvtType )
	{
	case	csvtObject:
		{
			ECSClassInfoObject *	pClassObj = new ECSClassInfoObject ;
			if ( ReadWideString( file, pClassObj->m_wstrClassName ) )
			{
				delete	pClassObj ;
				return	eslErrGeneral ;
			}
			pClassObj->m_pwszTag = pClassObj->m_wstrClassName ;
			pClassObj->m_pClassInf =
					GetClassInfoAs( pClassObj->m_wstrClassName ) ;
			pObj = pClassObj ;
		}
		break ;
	case	csvtReference:
		{
			ECSReference *	pRef = new ECSReference ;
			ECSObject *		pRefType = NULL ;
			err = ReadTypeObject( file, pRefType ) ;
			if ( err )
			{
				delete	pRef ;
				return	err ;
			}
			pRef->SetOwnObject( pRefType ) ;
			pObj = pRef ;
		}
		break ;
	case	csvtArray:
		pObj = new ECSArray ;
		{
			int	nLength ;
			if ( file.Read( &nLength, sizeof(int) ) < sizeof(int) )
			{
				delete	pObj ;
				return	eslErrGeneral ;
			}
			for ( int i = 0; i < nLength; i ++ )
			{
				ECSObject *	pElement ;
				if ( ReadTypeObject( file, pElement ) )
				{
					delete	pObj ;
					return	eslErrGeneral ;
				}
				((ECSArray*)pObj)->m_varArray.Add( pElement ) ;
			}
		}
		break ;
	case	csvtHash:
		pObj = new ECSHash ;
		break ;
	case	csvtInteger64:
		file.Read( &nMask, sizeof(INT64) ) ;
		file.Read( &nValue, sizeof(INT64) ) ;
		pObj = new ECSInteger( nValue, nMask ) ;
		break ;
	case	csvtInteger:
		file.Read( &nValue, sizeof(INT64) ) ;
		pObj = new ECSInteger( nValue ) ;
		break ;
	case	csvtReal:
	case	csvtReal64:
		file.Read( &rValue, sizeof(REAL64) ) ;
		pObj = new ECSReal( rValue ) ;
		break ;
	case	csvtString:
		err = ReadWideString( file, wstrValue ) ;
		if ( err )
		{
			return	err ;
		}
		pObj = new ECSString( wstrValue ) ;
		break ;
	case	csvtPointer:
		{
			ECSPointer *	pPtr = new ECSPointer ;
			//
			file.Read( &(pPtr->m_csvtRefType), sizeof(CSVariableType) ) ;
			file.Read( &(pPtr->m_fReadOnly), sizeof(bool) ) ;
			//
			ECSObject *		pRefType = NULL ;
			err = ReadTypeObject( file, pRefType ) ;
			if ( err )
			{
				delete	pPtr ;
				return	err ;
			}
			pPtr->SetOwnObject( pRefType ) ;
			pObj = pPtr ;
		}
		break ;
	case	csvtFunction:
		{
			ECSFunction *	pFunc = new ECSFunction ;
			pObj = pFunc ;
			file.Read( &nValue, sizeof(INT64) ) ;
			pFunc->m_fpAddress = nValue ;
			//
			ECSWideString	wstrThisClass ;
			err = ReadWideString( file, wstrThisClass ) ;
			if ( err )
			{
				delete	pFunc ;
				return	err ;
			}
			if ( !wstrThisClass.IsEmpty() )
			{
				pFunc->m_pThisCall = GetClassInfoAs( wstrThisClass ) ;
			}
			//
			err = ReadPrototypeInfo( file, pFunc->m_prototype ) ;
			if ( err )
			{
				delete	pFunc ;
				return	err ;
			}
		}
		break ;
	case	csvtArrayDimension:
		{
			ECSObject *	pElementType = NULL ;
			err = ReadTypeObject( file, pElementType ) ;
			if ( err )
			{
				delete	pElementType ;
				return	err ;
			}
			int	nDim ;
			if ( file.Read( &nDim, sizeof(int) ) < sizeof(int) )
			{
				delete	pElementType ;
				return	eslErrGeneral ;
			}
			unsigned int *	pBounds = new unsigned int [nDim] ;
			if ( file.Read( pBounds, nDim * sizeof(unsigned int) )
										< nDim * sizeof(unsigned int) )
			{
				delete []	pBounds ;
				delete	pElementType ;
				return	eslErrGeneral ;
			}
			ECSArray *	pArray = new ECSArray ;
			pArray->MakeDimension( pBounds, nDim, pElementType ) ;
			delete []	pBounds ;
			pObj = pArray ;
			//
			int	nLength ;
			if ( file.Read( &nLength, sizeof(int) ) < sizeof(int) )
			{
				delete	pObj ;
				return	eslErrGeneral ;
			}
			for ( int i = 0; i < nLength; i ++ )
			{
				ECSObject *	pElement ;
				if ( ReadTypeObject( file, pElement ) )
				{
					delete	pObj ;
					return	eslErrGeneral ;
				}
				((ECSArray*)pObj)->m_varArray.Add( pElement ) ;
			}
		}
		break ;
	case	csvtHashContainer:
		{
			ECSObject *	pElementType = NULL ;
			err = ReadTypeObject( file, pElementType ) ;
			if ( err )
			{
				delete	pElementType ;
				return	err ;
			}
			ECSHash *	pHash = new ECSHash ;
			pHash->SetDefaultElement( pElementType ) ;
			pObj = pHash ;
		}
		break ;
	case	csvtBoolean:
		file.Read( &nValue, sizeof(INT64) ) ;
		pObj = new ECSInteger( nValue, ECSInteger::m_maskBoolean ) ;
		break ;
	case	csvtInt8:
		file.Read( &nValue, sizeof(INT64) ) ;
		pObj = new ECSInteger( nValue, ECSInteger::m_maskInt8 ) ;
		break ;
	case	csvtUint8:
		file.Read( &nValue, sizeof(INT64) ) ;
		pObj = new ECSInteger( nValue, ECSInteger::m_maskUint8 ) ;
		break ;
	case	csvtInt16:
		file.Read( &nValue, sizeof(INT64) ) ;
		pObj = new ECSInteger( nValue, ECSInteger::m_maskInt16 ) ;
		break ;
	case	csvtUint16:
		file.Read( &nValue, sizeof(INT64) ) ;
		pObj = new ECSInteger( nValue, ECSInteger::m_maskUint16 ) ;
		break ;
	case	csvtInt32:
		file.Read( &nValue, sizeof(INT64) ) ;
		pObj = new ECSInteger( nValue, ECSInteger::m_maskInt32 ) ;
		break ;
	case	csvtUint32:
		file.Read( &nValue, sizeof(INT64) ) ;
		pObj = new ECSInteger( nValue, ECSInteger::m_maskUint32 ) ;
		break ;
	case	csvtReal32:
		file.Read( &rValue, sizeof(REAL64) ) ;
		pObj = new ECSReal( rValue ) ;
		((ECSReal*)pObj)->m_vtRealType = csvtReal32 ;
		break ;
	case	csvtInvalid:
		pObj = NULL ;
		break ;
	default:
		return	eslErrGeneral ;
	}
	return	eslErrSuccess ;
}

ESLError ECSExecutionImage::ReadTypeInfo
	( ESLFileObject & file, ECSTypeInfo & typeinf )
{
	typeinf = ECSTypeInfo() ;
	file.Read( &(typeinf.m_dwFlags), sizeof(DWORD) ) ;
	return	ReadTypeObject( file, typeinf.m_pValue ) ;
}

// プロトタイプ情報を読み込む
//////////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImage::ReadPrototypeInfo
	( ESLFileObject & file, ECSPrototypeInfo & protoinf )
{
	//
	// フラグ
	//
	DWORD	dwFlags ;
	if ( file.Read( &dwFlags, sizeof(dwFlags) ) < sizeof(dwFlags) )
	{
		return	eslErrGeneral ;
	}
	protoinf.SetAttribute( dwFlags ) ;
	//
	// 名前
	//
	ESLError	err ;
	ECSWideString	wstrName, wstrGlobalName ;
	err = ReadWideString( file, wstrName ) ;
	if ( err )
	{
		return	err ;
	}
	err = ReadWideString( file, wstrGlobalName ) ;
	if ( err )
	{
		return	err ;
	}
	protoinf.SetName( wstrName ) ;
	protoinf.SetGlobalName( wstrGlobalName ) ;
	//
	// 返り値
	//
	ECSTypeInfo	typeReturn ;
	err = ReadTypeInfo( file, typeReturn ) ;
	if ( err )
	{
		return	err ;
	}
	protoinf.SetReturnType( typeReturn ) ;
	//
	// 引数
	//
	DWORD	dwArgCount ;
	if ( file.Read( &dwArgCount, sizeof(dwArgCount) ) < sizeof(dwArgCount) )
	{
		return	eslErrGeneral ;
	}
	for ( DWORD i = 0; i < dwArgCount; i ++ )
	{
		ECSTypeInfo *	pTypeArg = new ECSTypeInfo ;
		err = ReadTypeInfo( file, *pTypeArg ) ;
		if ( err )
		{
			delete	pTypeArg ;
			return	err ;
		}
		protoinf.SetArgumentAt( i, pTypeArg ) ;
	}
	return	eslErrSuccess ;
}
#include <stdio.h>
// クラス情報を読み込む
//////////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImage::ReadClassInfo
	( ESLFileObject & file, ECSClassInfo & clsinf )
{
	//
	// フラグ
	//
	DWORD	dwFlags ;
	if ( file.Read( &dwFlags, sizeof(dwFlags) ) < sizeof(dwFlags) )
	{
		return	eslErrGeneral ;
	}
	clsinf.SetAttribute( dwFlags ) ;
	//
	// クラス名
	//
	ECSWideString	wstrName ;
	ESLError	err ;
	err = ReadWideString( file, wstrName ) ;
	if ( err )
	{
		return	err ;
	}
	clsinf.SetName( wstrName ) ;
	//
	ECSWideString	wstrGlobalName ;
	err = ReadWideString( file, wstrGlobalName ) ;
	if ( err )
	{
		return	err ;
	}
	clsinf.SetGlobalName( wstrGlobalName ) ;
	//
	// 親クラス
	//
	DWORD	dwParentCount ;
	if ( file.Read( &dwParentCount, sizeof(dwParentCount) )
									< sizeof(dwParentCount) )
	{
		return	eslErrGeneral ;
	}
	DWORD	i ;
	for ( i = 0; i < dwParentCount; i ++ )
	{
		DWORD	dwFlags ;
		if ( file.Read( &dwFlags, sizeof(DWORD) ) < sizeof(DWORD) )
		{
			return	eslErrGeneral ;
		}
		ECSWideString	wstrClassName ;
		err = ReadWideString( file, wstrClassName ) ;
		if ( err )
		{
			return	err ;
		}
		ECSClassInfo *	pClassInf = GetClassInfoAs( wstrClassName ) ;
		if ( pClassInf == NULL )
		{
			return	eslErrGeneral ;
		}
		ECSClassInfo::ParentClass *
			pParentClass = new ECSClassInfo::ParentClass ;
		pParentClass->dwFlags = dwFlags ;
		pParentClass->pClassInf = pClassInf ;
		clsinf.AddParentClassInfo( pParentClass ) ;
	}
	//
	// 親クラスキャスト情報
	//
	DWORD	dwCastCount ;
	if ( file.Read( &dwCastCount, sizeof(dwCastCount) )
									< sizeof(dwCastCount) )
	{
		return	eslErrGeneral ;
	}
	for ( i = 0; i < dwCastCount; i ++ )
	{
		ECSWideString	wstrClassName ;
		err = ReadWideString( file, wstrClassName ) ;
		if ( err )
		{
			return	err ;
		}
		ECSClassInfo *	pClassInf = GetClassInfoAs( wstrClassName ) ;
		if ( pClassInf == NULL )
		{
			return	eslErrGeneral ;
		}
		ECSClassInfo::CastInfo *
					pciCastInf = new ECSClassInfo::CastInfo ;
		ECS_CAST_INTERFACE *	pci = pciCastInf ;
		if ( file.Read( pci, sizeof(ECS_CAST_INTERFACE) )
								< sizeof(ECS_CAST_INTERFACE) )
		{
			delete	pciCastInf ;
			return	eslErrGeneral ;
		}
		if ( file.Read( &(pciCastInf->dwFlags), sizeof(DWORD) ) < sizeof(DWORD) )
		{
			delete	pciCastInf ;
			return	eslErrGeneral ;
		}
		pciCastInf->pClassInf = pClassInf ;
		//
		clsinf.AddCastClassInfo( wstrClassName, pciCastInf ) ;
	}
	ECSDelayClassInfo *	pDelayClass = new ECSDelayClassInfo( &clsinf ) ;
	m_lstDelayClassInfo.Add( pDelayClass ) ;
	//
	// メンバ変数
	//
	DWORD	dwVarCount ;
	if ( file.Read( &dwVarCount, sizeof(dwVarCount) ) < sizeof(dwVarCount) )
	{
		return	eslErrGeneral ;
	}
	for ( i = 0; i < dwVarCount; i ++ )
	{
		ECSWideString *	pwstrVarName = new ECSWideString ;
		err = ReadWideString( file, *pwstrVarName ) ;
		if ( err )
		{
			delete	pwstrVarName ;
			return	err ;
		}
		ECSTypeInfo *	pVarType = new ECSTypeInfo ;
		err = ReadTypeInfo( file, *pVarType ) ;
		if ( err )
		{
			delete	pwstrVarName ;
			delete	pVarType ;
			return	err ;
		}
		pDelayClass->m_lstVarName.Add( pwstrVarName ) ;
		pDelayClass->m_lstVarType.Add( pVarType ) ;
	}
	//
	// メンバ関数
	//
	DWORD	dwFuncCount ;
	if ( file.Read( &dwFuncCount, sizeof(dwFuncCount) ) < sizeof(dwFuncCount) )
	{
		return	eslErrGeneral ;
	}
	for ( i = 0; i < dwFuncCount; i ++ )
	{
		ECSClassInfo::MemberFunction *
				pPrototype = new ECSClassInfo::MemberFunction ;
		err = ReadPrototypeInfo( file, *pPrototype ) ;
		if ( err )
		{
			delete	pPrototype ;
			return	err ;
		}
		//
		ECSWideString	wstrFuncClass ;
		err = ReadWideString( file, wstrFuncClass ) ;
		if ( err )
		{
			delete	pPrototype ;
			return	err ;
		}
		pPrototype->m_wstrClass = wstrFuncClass ;
		pPrototype->m_pClassCast =
			clsinf.GetCastClassInfoAs( wstrFuncClass ) ;
		//
		if ( file.Read
			( &(pPrototype->m_fpFuncPointer),
				sizeof(ECS_FUNCTION_POINTER) )
					< sizeof(ECS_FUNCTION_POINTER) )
		{
			delete	pPrototype ;
			return	eslErrGeneral ;
		}
		pDelayClass->m_lstPrototype.Add( pPrototype ) ;
	}
	//
	// 拡張情報用
	//
	DWORD	dwExDataSize ;
	file.Read( &dwExDataSize, sizeof(DWORD) ) ;
	if ( dwExDataSize > 0 )
	{
		file.Read( pDelayClass->m_bufExData.PutBuffer( dwExDataSize ), dwExDataSize ) ;
		pDelayClass->m_bufExData.Flush( dwExDataSize ) ;
	}
	//
	return	eslErrSuccess ;
}

// クラス情報を復元する（2パス）
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionImage::RestoreDelayClassInfo( ECSDelayClassInfo * pDelayClass )
{
	ECSClassInfo &	clsinf = *(pDelayClass->m_pClassInfo) ;
	DWORD	i ;
	DWORD	dwVarCount = pDelayClass->m_lstVarType.GetSize() ;
	for ( i = 0; i < dwVarCount; i ++ )
	{
		ECSWideString *	pwstrVarName = pDelayClass->m_lstVarName.GetAt(i) ;
		ECSTypeInfo *	pVarType = pDelayClass->m_lstVarType.GetAt(i) ;
		if ( pwstrVarName && pVarType )
		{
			clsinf.AddVariable( *pwstrVarName, pVarType ) ;
		}
	}
	DWORD	dwFuncCount = pDelayClass->m_lstPrototype.GetSize() ;
	for ( i = 0; i < dwFuncCount; i ++ )
	{
		clsinf.AddFunction( pDelayClass->m_lstPrototype.GetAt(i) ) ;
	}
	EStreamBuffer&	bufExData = pDelayClass->m_bufExData ;
	if ( bufExData.GetLength() != 0 )
	{
		DWORD	dwExFlags ;
		bufExData.Read( &dwExFlags, sizeof(DWORD) ) ;
		//
		if ( dwExFlags & cxfNakedAddress )
		{
			bufExData.Read
				( &(clsinf.m_nNakedSize), sizeof(clsinf.m_nNakedSize) ) ;
			bufExData.Read
				( &(clsinf.m_nUnnakedObjects), sizeof(clsinf.m_nUnnakedObjects) ) ;
			//
			int *	pVarOffset = new int[dwVarCount] ;
			bufExData.Read( pVarOffset, dwVarCount * sizeof(int) ) ;
			//
			for ( i = 0; i < dwVarCount; i ++ )
			{
				clsinf.m_lstVarNakedOffset.SetAt( i, pVarOffset[i] ) ;
			}
			delete []	pVarOffset ;
		}
	}
}

// 数値配列を書き出す
//////////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImage::WriteDWordArray
	( ESLFileObject & file, const ENumArray<DWORD> & array )
{
	DWORD	dwLength = array.GetSize( ) ;
	file.Write( &dwLength, sizeof(dwLength) ) ;
	//
	for ( DWORD i = 0; i < dwLength; i ++ )
	{
		DWORD	dwValue = array.GetAt( i ) ;
		if ( file.Write( &dwValue, sizeof(dwValue) ) < sizeof(dwValue) )
		{
			return	eslErrGeneral ;
		}
	}
	return	eslErrSuccess ;
}

// 文字列を書き出す
//////////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImage::WriteWideString
	( ESLFileObject & file, const ECSWideString & wstr )
{
	DWORD	dwLength = wstr.GetLength( ) ;
	file.Write( &dwLength, sizeof(dwLength) ) ;
	//
	if ( file.Write( wstr.CharPtr(),
			dwLength * sizeof(wchar_t) ) < dwLength * sizeof(wchar_t) )
	{
		return	eslErrGeneral ;
	}
	return	eslErrSuccess ;
}

// 文字列配列を書き出す
//////////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImage::WriteWideStringArray
	( ESLFileObject & file, const ECSStrBufTagArray & array )
{
	DWORD		i, dwLength ;
	dwLength = array.GetSize( ) ;
	file.Write( &dwLength, sizeof(dwLength) ) ;
	//
	for ( i = 0; i < dwLength; i ++ )
	{
		ECSWideString	wstr = array.GetAt( i ) ;
		ESLError	err = WriteWideString( file, wstr ) ;
		if ( err )
		{
			return	err ;
		}
	}
	return	eslErrSuccess ;
}

ESLError ECSExecutionImage::WriteWideStringArray
	( ESLFileObject & file,
		const SSystem::SIndexedArray
				<SSystem::SString,const wchar_t*> & array )
{
	DWORD	i, dwLength ;
	dwLength = array.GetLength( ) ;
	file.Write( &dwLength, sizeof(dwLength) ) ;
	//
	for ( i = 0; i < dwLength; i ++ )
	{
		SSystem::SString *	pStr = array.GetAt( i ) ;
		ESLError	err =
			WriteWideString( file, ECSWideString( *pStr ) ) ;
		if ( err )
		{
			return	err ;
		}
	}
	return	eslErrSuccess ;
}

// タグ付き数値配列を書き出す
//////////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImage::WriteTagedDWordArray
	( ESLFileObject & file, const TaggedRefAddresList & array )
{
	ESLError	err = eslErrSuccess ;
	DWORD		i, dwLength ;
	dwLength = array.GetSize( ) ;
	file.Write( &dwLength, sizeof(dwLength) ) ;
	//
	for ( i = 0; i < dwLength; i ++ )
	{
		ETaggedElement
			< ECSWideString, ENumArray<DWORD> > *
								pElement = array.GetAt( i ) ;
		ESLAssert( pElement != NULL ) ;
		//
		err = WriteWideString( file, pElement->Tag() ) ;
		if ( err )
		{
			break ;
		}
		err = WriteDWordArray( file, *(pElement->GetObject()) ) ;
		if ( err )
		{
			break ;
		}
	}
	return	err ;
}

// オブジェクトを書き出す
//////////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImage::WriteObject
	( ESLFileObject & file, ECSObject * pObj )
{
	if ( m_exiHeader.nIntBase == 64 )
	{
		return	WriteTypeObject( file, pObj ) ;
	}
	CSVariableType	csvtType = pObj->m_vtType ;
	file.Write( &csvtType, sizeof(csvtType) ) ;
	//
	switch ( csvtType )
	{
	case	csvtInteger:
		{
			ECSInteger *	pInt = (ECSInteger*) pObj ;
			if ( m_exiHeader.nIntBase == 64 )
			{
				INT64	varInt = pInt->GetValue( ) ;
				if ( file.Write
					( &varInt, sizeof(INT64) ) < sizeof(INT64) )
				{
					return	eslErrGeneral ;
				}
			}
			else
			{
				INT64	varInt = pInt->GetValue( ) ;
				if ( file.Write
					( &varInt, sizeof(int) ) < sizeof(int) )
				{
					return	eslErrGeneral ;
				}
			}
		}
		break ;
	case	csvtReal:
		{
			ECSReal *	pReal = (ECSReal*) pObj ;
			if ( file.Write( &(pReal->m_varReal),
					sizeof(pReal->m_varReal) ) < sizeof(pReal->m_varReal) )
			{
				return	eslErrGeneral ;
			}
		}
		break ;
	case	csvtString:
		if ( WriteWideString( file, ((ECSString*)pObj)->m_varStr ) )
		{
			return	eslErrGeneral ;
		}
		break ;
	case	csvtArray:
		{
			int	nLength = ((ECSArray*)pObj)->m_varArray.GetSize() ;
			if ( file.Write( &nLength, sizeof(int) ) < sizeof(int) )
			{
				return	eslErrGeneral ;
			}
			for ( int i = 0; i < nLength; i ++ )
			{
				ECSObject *	pElement =
					((ECSArray*)pObj)->m_varArray.GetAt( i ) ;
				if ( WriteObject( file, pElement ) )
				{
					return	eslErrGeneral ;
				}
			}
		}
		break ;
	case	csvtObject:
		{
			ECSWideString	wstrTypeName = pObj->GetTypeName( ) ;
			if ( WriteWideString( file, wstrTypeName ) )
			{
				return	eslErrGeneral ;
			}
		}
		break ;
	}
	return	eslErrSuccess ;
}

// 型情報を書き出す
//////////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImage::WriteTypeObject
	( ESLFileObject & file, ECSObject * pObj )
{
	CSVariableType	csvtType = csvtInvalid ;
	if ( pObj != NULL )
	{
		csvtType = pObj->m_vtType ;
		if ( csvtType == csvtInteger )
		{
			csvtType = ((ECSInteger*)pObj)->GetIntegerType() ;
			if ( (csvtType == csvtInteger)
				&& (((ECSInteger*)pObj)->SizeOf() == 64)
				&& !((ECSInteger*)pObj)->IsSign() )
			{
				csvtType = csvtInteger64 ;
			}
		}
		else if ( csvtType == csvtReal )
		{
			csvtType = ((ECSReal*)pObj)->m_vtRealType ;
		}
		else if ( csvtType == csvtArray )
		{
			if ( ((ECSArray*)pObj)->GetEndDefaultElement() != NULL )
			{
				csvtType = csvtArrayDimension ;
			}
		}
		else if ( csvtType == csvtHash )
		{
			if ( ((ECSHash*)pObj)->m_pDefObj != NULL )
			{
				csvtType = csvtHashContainer ;
			}
		}
	}
	if ( file.Write( &csvtType, sizeof(csvtType) ) < sizeof(csvtType) )
	{
		return	eslErrGeneral ;
	}
	ESLError		err ;
	INT64			nValue ;
	REAL64			rValue ;
	ECSWideString	wstrValue ;
	switch ( csvtType )
	{
	case	csvtObject:
		{
			ECSWideString	wstrType = pObj->GetTypeName() ;
			return	WriteWideString( file, wstrType ) ;
		}
	case	csvtReference:
		return	WriteTypeObject( file, ((ECSReference*)pObj)->m_pRef ) ;

	case	csvtPointer:
		{
			ECSPointer *	pPtr = (ECSPointer*) pObj ;
			file.Write( &(pPtr->m_csvtRefType), sizeof(CSVariableType) ) ;
			file.Write( &(pPtr->m_fReadOnly), sizeof(bool) ) ;
			return	WriteTypeObject( file, pPtr->m_pRef ) ;
		}

	case	csvtInteger64:
		nValue = ((ECSInteger*)pObj)->GetValueMask() ;
		file.Write( &nValue, sizeof(INT64) ) ;

	case	csvtInteger:
	case	csvtInt32:
	case	csvtUint32:
	case	csvtInt16:
	case	csvtUint16:
	case	csvtInt8:
	case	csvtUint8:
	case	csvtBoolean:
		nValue = ((ECSInteger*)pObj)->GetValue() ;
		file.Write( &nValue, sizeof(INT64) ) ;
		break ;

	case	csvtReal:
	case	csvtReal32:
	case	csvtReal64:
		rValue = ((ECSReal*)pObj)->m_varReal ;
		file.Write( &rValue, sizeof(REAL64) ) ;
		break ;

	case	csvtString:
		wstrValue = ((ECSString*)pObj)->m_varStr ;
		err = WriteWideString( file, wstrValue ) ;
		if ( err )
		{
			return	err ;
		}
		break ;

	case	csvtArrayDimension:
		{
			ESLAssert( pObj->m_vtType == csvtArray ) ;
			ECSArray *	pArray = (ECSArray*) pObj ;
			ECSObject *	pElementType = pArray->GetEndDefaultElement() ;
			ESLError	err = WriteTypeObject( file, pElementType ) ;
			if ( err )
			{
				return	err ;
			}
			int	nDim = pArray->GetDimension() ;
			unsigned int *	pBounds = new unsigned int [nDim] ;
			nDim = pArray->GetDimensionSize( pBounds, nDim ) ;
			if ( file.Write( &nDim, sizeof(int) ) < sizeof(int) )
			{
				delete []	pBounds ;
				return	eslErrGeneral ;
			}
			if ( file.Write( pBounds, nDim * sizeof(unsigned int) )
										< nDim * sizeof(unsigned int) )
			{
				delete []	pBounds ;
				return	eslErrGeneral ;
			}
			delete []	pBounds ;
		}
	case	csvtArray:
		{
			int	nLength = ((ECSArray*)pObj)->m_varArray.GetSize() ;
			if ( file.Write( &nLength, sizeof(int) ) < sizeof(int) )
			{
				return	eslErrGeneral ;
			}
			for ( int i = 0; i < nLength; i ++ )
			{
				ECSObject *	pElement =
					((ECSArray*)pObj)->m_varArray.GetAt( i ) ;
				if ( WriteTypeObject( file, pElement ) )
				{
					return	eslErrGeneral ;
				}
			}
		}
		break ;

	case	csvtHashContainer:
		{
			ESLAssert( pObj->m_vtType == csvtHash ) ;
			ECSHash *	pHash = (ECSHash*) pObj ;
			ESLError	err = WriteTypeObject( file, pHash->m_pDefObj ) ;
			if ( err )
			{
				return	err ;
			}
		}
		break ;

	case	csvtFunction:
		{
			ESLAssert( pObj->m_vtType == csvtFunction ) ;
			ECSFunction *	pFunc = (ECSFunction*) pObj ;
			file.Write( &(pFunc->m_fpAddress), sizeof(INT64) ) ;
			//
			ECSWideString	wstrThisClass ;
			if ( pFunc->m_pThisCall != NULL )
			{
				wstrThisClass = pFunc->m_pThisCall->GetGlobalName() ;
			}
			err = WriteWideString( file, wstrThisClass ) ;
			if ( err )
			{
				return	err ;
			}
			//
			err = WritePrototypeInfo( file, pFunc->m_prototype ) ;
			if ( err )
			{
				return	err ;
			}
		}
		break ;
	}
	return	eslErrSuccess ;
}

ESLError ECSExecutionImage::WriteTypeInfo
	( ESLFileObject & file, const ECSTypeInfo & typeinf )
{
	file.Write( &(typeinf.m_dwFlags), sizeof(DWORD) ) ;
	return	WriteTypeObject( file, typeinf.m_pValue ) ;
}

// プロトタイプ情報を書き出す
//////////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImage::WritePrototypeInfo
	( ESLFileObject & file, const ECSPrototypeInfo & protoinf )
{
	//
	// フラグ
	//
	DWORD	dwFlags = protoinf.GetAttribute() ;
	if ( file.Write( &dwFlags, sizeof(dwFlags) ) < sizeof(dwFlags) )
	{
		return	eslErrGeneral ;
	}
	//
	// 名前
	//
	ESLError	err ;
	ECSWideString	wstrName = protoinf.GetName() ;
	ECSWideString	wstrGlobalName = protoinf.GetGlobalName() ;
	err = WriteWideString( file, wstrName ) ;
	if ( err )
	{
		return	err ;
	}
	err = WriteWideString( file, wstrGlobalName ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// 返り値
	//
	err = WriteTypeInfo( file, protoinf.GetReturnType() ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// 引数
	//
	DWORD	dwArgCount = protoinf.GetArgumentCount() ;
	if ( file.Write( &dwArgCount, sizeof(dwArgCount) ) < sizeof(dwArgCount) )
	{
		return	eslErrGeneral ;
	}
	for ( DWORD i = 0; i < dwArgCount; i ++ )
	{
		ECSTypeInfo *	pTypeArg = protoinf.GetArgumentAt( i ) ;
		ESLAssert( pTypeArg != NULL ) ;
		err = WriteTypeInfo( file, *pTypeArg ) ;
		if ( err )
		{
			return	err ;
		}
	}
	return	eslErrSuccess ;
}

// クラス情報を書き出す
//////////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImage::WriteClassInfo
	( ESLFileObject & file, const ECSClassInfo & clsinf )
{
	//
	// フラグ
	//
	DWORD	dwFlags = clsinf.GetAttribute() ;
	if ( file.Write( &dwFlags, sizeof(dwFlags) ) < sizeof(dwFlags) )
	{
		return	eslErrGeneral ;
	}
	//
	// クラス名
	//
	ECSWideString	wstrName = clsinf.GetName() ;
	ESLError	err ;
	err = WriteWideString( file, wstrName ) ;
	if ( err )
	{
		return	err ;
	}
	//
	ECSWideString	wstrGlobalName = clsinf.GetGlobalName() ;
	err = WriteWideString( file, wstrGlobalName ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// 親クラス
	//
	DWORD	dwParentCount = clsinf.GetParentClassCount() ;
	if ( file.Write( &dwParentCount, sizeof(dwParentCount) )
									< sizeof(dwParentCount) )
	{
		return	eslErrGeneral ;
	}
	DWORD	i ;
	for ( i = 0; i < dwParentCount; i ++ )
	{
		ECSClassInfo::ParentClass *
			pParentClass = clsinf.GetParentClassAt( i ) ;
		ESLAssert( pParentClass != NULL ) ;
		DWORD	dwFlags = pParentClass->dwFlags ;
		if ( file.Write( &dwFlags, sizeof(DWORD) ) < sizeof(DWORD) )
		{
			return	eslErrGeneral ;
		}
		ESLAssert( pParentClass->pClassInf != NULL ) ;
		ECSWideString	wstrClassName =
							pParentClass->pClassInf->GetGlobalName() ;
		err = WriteWideString( file, wstrClassName ) ;
		if ( err )
		{
			return	err ;
		}
	}
	//
	// 親クラスキャスト情報
	//
	const EWStrTagArray<ECSClassInfo::CastInfo> &
							wstaCast = clsinf.GetCastClassArray() ;
	DWORD	dwCastCount = wstaCast.GetSize() ;
	if ( file.Write( &dwCastCount, sizeof(dwCastCount) )
									< sizeof(dwCastCount) )
	{
		return	eslErrGeneral ;
	}
	for ( i = 0; i < dwCastCount; i ++ )
	{
		ECSClassInfo::CastInfo *	pciCastInf = wstaCast.GetObjectAt( i ) ;
		ESLAssert( pciCastInf != NULL ) ;
		ESLAssert( wstaCast.GetTagAt(i) != NULL ) ;
		//
		ECSWideString	wstrClassName = *(wstaCast.GetTagAt(i)) ;
		err = WriteWideString( file, wstrClassName ) ;
		if ( err )
		{
			return	err ;
		}
		ECS_CAST_INTERFACE *	pci = pciCastInf ;
		if ( file.Write( pci, sizeof(ECS_CAST_INTERFACE) )
								< sizeof(ECS_CAST_INTERFACE) )
		{
			return	eslErrGeneral ;
		}
		if ( file.Write( &(pciCastInf->dwFlags), sizeof(DWORD) ) < sizeof(DWORD) )
		{
			return	eslErrGeneral ;
		}
	}
	//
	// メンバ変数
	//
	DWORD	dwVarCount = clsinf.GetVariableCount() ;
	if ( file.Write( &dwVarCount, sizeof(dwVarCount) ) < sizeof(dwVarCount) )
	{
		return	eslErrGeneral ;
	}
	for ( i = 0; i < dwVarCount; i ++ )
	{
		ECSWideString	wstrVarName = clsinf.GetVariableNameAt( i ) ;
		err = WriteWideString( file, wstrVarName ) ;
		if ( err )
		{
			return	err ;
		}
		ECSTypeInfo *	pVarType = clsinf.GetVariableAt( i ) ;
		ESLAssert( pVarType != NULL ) ;
		err = WriteTypeInfo( file, *pVarType ) ;
		if ( err )
		{
			return	err ;
		}
	}
	//
	// メンバ関数
	//
	DWORD	dwFuncCount = clsinf.GetFunctionCount() ;
	if ( file.Write
		( &dwFuncCount, sizeof(dwFuncCount) ) < sizeof(dwFuncCount) )
	{
		return	eslErrGeneral ;
	}
	for ( i = 0; i < dwFuncCount; i ++ )
	{
		const ECSClassInfo::MemberFunction *
						pPrototype = clsinf.GetFunctionAt( i ) ;
		err = WritePrototypeInfo( file, *pPrototype ) ;
		if ( err )
		{
			return	err ;
		}
		ECSWideString	wstrFuncClass = pPrototype->m_wstrClass ;
		err = WriteWideString( file, wstrFuncClass ) ;
		if ( err )
		{
			return	err ;
		}
		if ( file.Write
			( &(pPrototype->m_fpFuncPointer),
				sizeof(ECS_FUNCTION_POINTER) )
					< sizeof(ECS_FUNCTION_POINTER) )
		{
			return	eslErrGeneral ;
		}
	}
	//
	// 拡張情報用
	//
	DWORD			dwExFlags = 0 ;
	EStreamBuffer	bufExData ;
	//
	if ( clsinf.GetAttribute() & ECSTypeInfo::flagNakedBuffer )
	{
		dwExFlags |= cxfNakedAddress ;
	}
	bufExData.Write( &dwExFlags, sizeof(DWORD) ) ;
	//
	if ( dwExFlags & cxfNakedAddress )
	{
		//
		// naked オフセットアドレス
		//
		bufExData.Write
			( &(clsinf.m_nNakedSize), sizeof(clsinf.m_nNakedSize) ) ;
		bufExData.Write
			( &(clsinf.m_nUnnakedObjects), sizeof(clsinf.m_nUnnakedObjects) ) ;
		//
		int *	pVarOffset = new int[dwVarCount] ;
		for ( i = 0; i < dwVarCount; i ++ )
		{
			pVarOffset[i] = clsinf.m_lstVarNakedOffset[i] ;
		}
		bufExData.Write( pVarOffset, dwVarCount * sizeof(int) ) ;
		delete []	pVarOffset ;
	}
	//
	EPtrBuffer	pbfExData = bufExData.GetBuffer() ;
	DWORD		dwExDataSize = pbfExData.GetLength() ;
	file.Write( &dwExDataSize, sizeof(DWORD) ) ;
	if ( dwExDataSize > 0 )
	{
		file.Write( pbfExData.GetBuffer(), dwExDataSize ) ;
	}
	return	eslErrSuccess ; 
}

// ネイティブコード化
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionImage::CompileToNativeCode( bool fNoBoundary )
{
	BYTE *	pTrickBuf ;
	m_bufImage.CreateShadowBuffer() ;
	pTrickBuf = m_bufImage.GetSegmentShadowBuffer() ;
	//
	delete	m_bufNativeCodes ;
	delete	m_bufNativeGates ;
	m_bufNativeCodes = new ECSSakura2JIT::X86CodeBuffer ;
	m_bufNativeGates = new ECSSakura2JIT::X86CodeBuffer ;
	//
	ECSSakura2JIT::X86GenericAssembler *	pAsmCodes = NULL ;
	ECSSakura2JIT::X86GenericAssembler *	pAsmGates = NULL ;
	if ( SSystem::GetCPUFeatures() & SSystem::cpuX86_Feature_SSE2 )
	{
		pAsmCodes = new ECSSakura2JIT::X86SSE2Assembler ;
		pAsmGates = new ECSSakura2JIT::X86SSE2Assembler ;
	}
	else
	{
		pAsmCodes = new ECSSakura2JIT::X86GenericAssembler ;
		pAsmGates = new ECSSakura2JIT::X86GenericAssembler ;
	}
	pAsmCodes->SetCallingABI
		( ECSSakura2JIT::X86GenericAssembler::fastcallMSstyle ) ;
	pAsmGates->SetCallingABI
		( ECSSakura2JIT::X86GenericAssembler::fastcallMSstyle ) ;
	pAsmCodes->AttachCodeBuffer( m_bufNativeCodes, m_bufNativeGates ) ;
	pAsmGates->AttachCodeBuffer( m_bufNativeGates ) ;
	pAsmCodes->SetNoBoundaryWithAddressTranslation( fNoBoundary ) ;
	pAsmGates->SetNoBoundaryWithAddressTranslation( fNoBoundary ) ;
	//
	ECSSakura2JIT::NativeCompiler	ncompiler ;
	ncompiler.AttachCodeAssembler( pAsmCodes, pAsmGates ) ;
	//
	for ( int i = 0; i < (int) m_wstaFunc.GetSize(); i ++ )
	{
		FUNC_ENTRY *	pFunc = m_wstaFunc.GetObjectAt( i ) ;
		if ( (pFunc == NULL)
			|| (pFunc->dwBytes == (DWORD) -1)
			|| !(pFunc->dwFlags & ECSTypeInfo::flagNakedCall) )
		{
			continue ;
		}
		ncompiler.AttachFunction
			( m_pImage + pFunc->dwAddress,
				pTrickBuf + pFunc->dwAddress,
				pFunc->dwAddress, pFunc->dwBytes ) ;
		if ( ncompiler.PreprocessFunction() )
		{
			ncompiler.CompileFunction() ;
		}
	}
	//
	delete	pAsmCodes ;
	delete	pAsmGates ;
}

// 関数のアドレス取得
//////////////////////////////////////////////////////////////////////////////
DWORD * ECSExecutionImage::GetFunctionAddress( const wchar_t * pwszFuncName ) const
{
	FUNC_ENTRY *	pfe = m_wstaFunc.GetAs( pwszFuncName ) ;
	if ( pfe != NULL )
	{
		return	&(pfe->dwAddress) ;
	}
	return	NULL ;
}

// 関数情報エントリを取得する
//////////////////////////////////////////////////////////////////////////////
ECSExecutionImage::FUNC_ENTRY *
	ECSExecutionImage::GetFunctionEntry( const wchar_t * pwszFuncName ) const
{
	return	m_wstaFunc.GetAs( pwszFuncName ) ;
}

// 関数エントリを追加
//////////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImage::AddFunctionEntry
	( const wchar_t * pwszFuncName, DWORD dwPosition, DWORD dwFlags )
{
	if ( m_wstaFunc.GetAs( pwszFuncName ) != NULL )
	{
		return	ESLErrorMsg
			( "既に定義されている関数を追加しようとしました。" ) ;
	}
	//
	FUNC_ENTRY *	pfe = new FUNC_ENTRY ;
	pfe->dwAddress = dwPosition ;
	pfe->dwFlags = dwFlags ;
	//
	m_wstaFunc.SetAs( pwszFuncName, pfe ) ;
	return	eslErrSuccess ;
}

// 関数エントリの終了アドレス設定
//////////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImage::SetEndOfFunctionAddress
	( const wchar_t * pwszFuncName, DWORD dwPosition )
{
	FUNC_ENTRY *	pfe = m_wstaFunc.GetAs( pwszFuncName ) ;
	if ( pfe == NULL )
	{
		return	ESLErrorMsg
			( "未定義関数の終了アドレスを設定しようとしています。" ) ;
	}
	pfe->dwBytes = dwPosition - pfe->dwAddress ;
	return	eslErrSuccess ;
}

// 関数参照コードアドレス登録
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionImage::AddCodeRefFunctionAddress
	( const wchar_t * pwszGlobalFuncName, DWORD dwCodeAddr )
{
	ENumArray<DWORD> *	pList = m_impFuncRef.GetAs( pwszGlobalFuncName ) ;
	if ( pList == NULL )
	{
		pList = new ENumArray<DWORD> ;
		m_impFuncRef.SetAs( pwszGlobalFuncName, pList ) ;
	}
	pList->Add( dwCodeAddr ) ;
}

void ECSExecutionImage::AddCodeRefFunctionAddress64
	( const wchar_t * pwszGlobalFuncName, DWORD dwCodeAddr )
{
	ENumArray<DWORD> *	pList = m_impNakedFuncRef.GetAs( pwszGlobalFuncName ) ;
	if ( pList == NULL )
	{
		pList = new ENumArray<DWORD> ;
		m_impNakedFuncRef.SetAs( pwszGlobalFuncName, pList ) ;
	}
	pList->Add( dwCodeAddr ) ;
}

// グローバルオブジェクトを取得
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSExecutionImage::GetGlobalObject( const wchar_t * pwszObjName )
{
	ECSObject *	pObj = NULL ;
	ESLError	err ;
	int			nIndex ;
	err = m_csgGlobal.GetVariableIndex( nIndex, pwszObjName ) ;
	if ( !err )
	{
		pObj = m_csgGlobal.GetVariableAt( nIndex ) ;
	}
	if ( pObj == NULL )
	{
		err = m_csgData.GetVariableIndex( nIndex, pwszObjName ) ;
		if ( !err )
		{
			pObj = m_csgData.GetVariableAt( nIndex ) ;
		}
	}
	return	pObj ;
}

// クラス数を取得
//////////////////////////////////////////////////////////////////////////////
int ECSExecutionImage::GetClassInfoCount( void ) const
{
	return	m_lstClassInfo.GetSize() ;
}

// クラスを検索
//////////////////////////////////////////////////////////////////////////////
int ECSExecutionImage::GetClassInfoIndex
				( const wchar_t * pwszClassName ) const
{
	return	m_vectorNewObject.FindEntry( pwszClassName ) ;
}

// クラス情報を取得
//////////////////////////////////////////////////////////////////////////////
ECSClassInfo * ECSExecutionImage::GetClassInfoAt( int nIndex ) const
{
	return	m_lstClassInfo.GetAt( nIndex ) ;
}

ECSClassInfo * ECSExecutionImage::GetClassInfoAs
					( const wchar_t * pwszClassName ) const
{
	int	iClass = m_vectorNewObject.FindEntry( pwszClassName ) ;
	if ( iClass >= 0 )
	{
		ECSClassInfo *	pClassInf = m_lstClassInfo.GetAt( iClass ) ;
		ESLAssert( pClassInf != NULL ) ;
		ESLAssert( pClassInf->GetGlobalName() == pwszClassName ) ;
		return	pClassInf ;
	}
	return	NULL ;
}

// クラスを追加
//////////////////////////////////////////////////////////////////////////////
int ECSExecutionImage::AddClassInfo( ECSClassInfo * pClassInf )
{
	int	iClass = m_vectorNewObject.FindEntry( pClassInf->GetGlobalName() ) ;
	if ( iClass >= 0 )
	{
		m_lstClassInfo.SetAt( iClass, pClassInf ) ;
	}
	else
	{
		iClass = m_lstClassInfo.GetSize() ;
		ESLAssert( (int) m_vectorNewObject.GetEntryIndex().GetLength() == iClass ) ;
		m_lstClassInfo.Add( pClassInf ) ;
		m_vectorNewObject.AddEntry( pClassInf->GetGlobalName() ) ;
	}
	return	iClass ;
}

// マージ元のクラス情報から正規のインポートクラス情報を取得
//////////////////////////////////////////////////////////////////////////////
ECSClassInfo *
	ECSExecutionImage::GetImportedClassInfo
			( const ECSClassInfo * pImportClassInf )
{
	if ( pImportClassInf != NULL )
	{
		ECSClassInfo *	pClassInf =
			GetClassInfoAs( pImportClassInf->GetGlobalName() ) ;
		if ( pClassInf == NULL )
		{
			int	iClassInf = ImportClassInfo( *pImportClassInf ) ;
			pClassInf = GetClassInfoAt( iClassInf ) ;
			ESLAssert( pClassInf != NULL ) ;
		}
		return	pClassInf ;
	}
	return	NULL ;
}

// クラス情報を複製して追加
//////////////////////////////////////////////////////////////////////////////
int ECSExecutionImage::ImportClassInfo( const ECSClassInfo & clsinfImport )
{
	ECSClassInfo *const	pClassInf = new ECSClassInfo( clsinfImport ) ;
	const int			iClassInf = AddClassInfo( pClassInf ) ;
	//
	// 親クラス情報
	//
	unsigned int	i, nCount ;
	nCount = pClassInf->GetParentClassCount() ;
	for ( i = 0; i < nCount; i ++ )
	{
		ECSClassInfo::ParentClass *
			pParentClass = pClassInf->GetParentClassAt( i ) ;
		ESLAssert( pParentClass != NULL ) ;
		if ( pParentClass->pClassInf != NULL )
		{
			pParentClass->pClassInf =
				GetImportedClassInfo( pParentClass->pClassInf ) ;
		}
	}
	//
	// キャスト情報
	//
	const EWStrTagArray<ECSClassInfo::CastInfo> &
					wstaCast = pClassInf->GetCastClassArray() ;
	nCount = wstaCast.GetSize() ;
	for ( i = 0; i < nCount; i ++ )
	{
		ECSClassInfo::CastInfo *	pCastInf = wstaCast.GetObjectAt( i ) ;
		ESLAssert( pCastInf != NULL ) ;
		if ( pCastInf->pClassInf != NULL )
		{
			ECSClassInfo *	pCastClassInf =
					GetClassInfoAs( pCastInf->pClassInf->GetGlobalName() ) ;
			if ( (pCastClassInf == NULL)
				&& (pCastInf->pClassInf->GetGlobalName()
									!= clsinfImport.GetGlobalName()) )
			{
				int	iCastClass = ImportClassInfo( *(pCastInf->pClassInf) ) ;
				pCastClassInf = GetClassInfoAt( iCastClass ) ;
				ESLAssert( pCastClassInf != NULL ) ;
			}
			pCastInf->pClassInf = pCastClassInf ;
		}
	}
	//
	// メンバ変数型
	//
	nCount = pClassInf->GetVariableCount() ;
	for ( i = 0; i < nCount; i ++ )
	{
		ECSTypeInfo *	pVarType = pClassInf->GetVariableAt( i ) ;
		if ( (pVarType != NULL) && (pVarType->m_pValue != NULL) )
		{
			NormalizeVariableTypeClassInfo( pVarType->m_pValue ) ;
		}
	}
	//
	// メンバ関数プロトタイプ
	//
	nCount = pClassInf->GetFunctionCount() ;
	for ( i = 0; i < nCount; i ++ )
	{
		ECSClassInfo::MemberFunction *
				pMemberFunc = pClassInf->GetFunctionAt( i ) ;
		if ( pMemberFunc == NULL )
		{
			continue ;
		}
		if ( pMemberFunc->m_pClassCast != NULL )
		{
			pMemberFunc->m_pClassCast->pClassInf =
				GetImportedClassInfo( pMemberFunc->m_pClassCast->pClassInf ) ;
		}
		NormalizePrototypeClassInfo( pMemberFunc ) ;
	}
	//
	return	iClassInf ;
}

// クラス情報からメンバ情報を削除
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionImage::CleanupClassMemberInfo( void )
{
	unsigned int	nCount = m_lstClassInfo.GetSize() ;
	for ( unsigned int i = 0; i < nCount; i ++ )
	{
		ECSClassInfo *	pClassInf = m_lstClassInfo.GetAt( i ) ;
		if ( pClassInf != NULL )
		{
			pClassInf->CleanupClassMember() ;
		}
	}
}

// 関数プロトタイプのクラス情報を正規化
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionImage::NormalizePrototypeClassInfo( ECSPrototypeInfo * pPrototype )
{
	ECSTypeInfo	typeReturn = pPrototype->GetReturnType() ;
	if ( typeReturn.m_pValue != NULL )
	{
		NormalizeVariableTypeClassInfo( typeReturn.m_pValue ) ;
		pPrototype->SetReturnType( typeReturn ) ;
	}
	int	j, m ;
	m = pPrototype->GetArgumentCount() ;
	for ( j = 0; j < m; j ++ )
	{
		ECSTypeInfo *	pArgType = pPrototype->GetArgumentAt( j ) ;
		if ( pArgType != NULL )
		{
			NormalizeVariableTypeClassInfo( pArgType->m_pValue ) ;
		}
	}
}

// 変数型のクラス情報を正規化
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionImage::NormalizeVariableTypeClassInfo( ECSObject * pVarType )
{
	if ( pVarType != NULL )
	{
		pVarType->m_pClassInf =
			GetImportedClassInfo( pVarType->m_pClassInf ) ;
		switch ( pVarType->m_vtType )
		{
		case	csvtArray:
			NormalizeVariableTypeClassInfo
					( ((ECSArray*)pVarType)->m_pDefObj ) ;
			break ;
		case	csvtHash:
			NormalizeVariableTypeClassInfo
					( ((ECSHash*)pVarType)->m_pDefObj ) ;
			break ;
		case	csvtReference:
			NormalizeVariableTypeClassInfo
					( ((ECSReference*)pVarType)->m_pRef ) ;
			break ;
		case	csvtPointer:
			NormalizeVariableTypeClassInfo
					( ((ECSPointer*)pVarType)->m_pRef ) ;
			break ;
		case	csvtFunction:
			{
				ECSFunction*	pFunc = (ECSFunction*) pVarType ;
				pFunc->m_pThisCall =
					GetImportedClassInfo( pFunc->m_pThisCall ) ;
				NormalizePrototypeClassInfo( &(pFunc->m_prototype) ) ;
			}
			break ;
		}
	}
}

