
/*****************************************************************************
             Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
	Copyright (c) 2002-2012 Leshade Entis, Entis-soft. All rights reserved.
 ****************************************************************************/


#include <gls.h>


//////////////////////////////////////////////////////////////////////////////
// Entis Cotopha Script 実行イメージ・リンカ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( ECSExecutionImageLinker, ECSExecutionImage )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ECSExecutionImageLinker::ECSExecutionImageLinker( void )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ECSExecutionImageLinker::~ECSExecutionImageLinker( void )
{
}

// エラー出力
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionImageLinker::OutputError( const char * pszErrMsg )
{
	ESLTrace( "%s\n", pszErrMsg ) ;
}

// 実行イメージを結合
//////////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImageLinker::MergeImage
	( const ECSExecutionImage & image, DWORD dwFlags )
{
	//
	// 重複するインライン関数をあらかじめ削除しておく
	//
	unsigned int	i, nCount ;
	/*
	nCount = image.m_wstaFunc.GetSize( ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		ETaggedElement<ECSWideString,FUNC_ENTRY> *	pElement ;
		pElement = image.m_wstaFunc.GetAt( i ) ;
		ESLAssert( pElement != NULL ) ;
		if ( pElement == NULL )
		{
			continue ;
		}
		FUNC_ENTRY *	pfe = m_wstaFunc.GetAs( pElement->Tag() ) ;
		if ( (pfe != NULL)
			&& (pfe->dwFlags & ECSTypeInfo::flagInline)
			&& (pfe->dwBytes != (DWORD) -1) )
		{
			DWORD	dwFuncAddr = pfe->dwAddress ;
			DWORD	dwFuncBytes = pfe->dwBytes ;
			m_wstaFunc.RemoveAs( pElement->Tag() ) ;
			RemoveCodeImage( dwFuncAddr, dwFuncBytes ) ;
		}
	}
	m_dwImageSize = m_bufImage.GetLength( ) ;
	m_pImage = (BYTE*) m_bufImage.ModifyBuffer( 0, m_dwImageSize ) ;
	*/
	//
	// naked メモリ結合準備
	//
	AlignCodeBuffer( 8 ) ;
	//
	const DWORD	dwImageBias = m_dwImageSize ;
	const DWORD	dwGlobalBias = m_csgGlobalType.m_varArray.GetSize( ) ;
	const DWORD	dwDataBias = m_csgDataType.m_varArray.GetSize( ) ;
	//
	if ( m_bufNakedGlobal.GetLength() & 0x0F )
	{
		m_bufNakedGlobal.ResizeBuffer
			( (m_bufNakedGlobal.GetLength() + 0x0F) & ~0x0F ) ;
	}
	if ( m_bufNakedConst.GetLength() & 0x0F )
	{
		m_bufNakedConst.ResizeBuffer
			( (m_bufNakedConst.GetLength() + 0x0F) & ~0x0F ) ;
	}
	if ( m_bufNakedShared.GetLength() & 0x0F )
	{
		m_bufNakedShared.ResizeBuffer
			( (m_bufNakedShared.GetLength() + 0x0F) & ~0x0F ) ;
	}
	const DWORD	dwNakedGlobalBias = m_bufNakedGlobal.GetLength() ;
	const DWORD	dwNakedConstBias = m_bufNakedConst.GetLength() ;
	const DWORD	dwNakedSharedBias = m_bufNakedShared.GetLength() ;
	//
	// クラス情報の置き換えテーブルを生成
	//
	ENumArray<int>	lstClassIndex ;
	if ( !(dwFlags & mfMirrorClassInf) )
	{
		nCount = image.m_lstClassInfo.GetSize() ;
		for ( i = 0; i < nCount; i ++ )
		{
			ECSClassInfo *	pSrcClassInf = image.m_lstClassInfo.GetAt( i ) ;
			ESLAssert( pSrcClassInf != NULL ) ;
			int	iClass =
//				m_staClassName.FindIndex( pSrcClassInf->GetGlobalName() ) ;
				m_vectorNewObject.FindEntry( pSrcClassInf->GetGlobalName() ) ;
			if ( iClass >= 0 )
			{
				ECSClassInfo *	pClassInf = m_lstClassInfo.GetAt( iClass ) ;
				ESLAssert( pClassInf != NULL ) ;
				if ( pClassInf->IsNamespace() )
				{
					pClassInf->CleanupClassInfo() ;
				}
				else if ( !pClassInf->IsClassEqual( *pSrcClassInf ) )
				{
					m_strErrMsg = "クラス情報 \'"
						+ EString( pSrcClassInf->GetGlobalName() )
						+ "\' が一致しません。" ;
					return	ESLErrorMsg( m_strErrMsg ) ;
				}
			}
			else
			{
				iClass = ImportClassInfo( *pSrcClassInf ) ;
			}
			lstClassIndex.SetAt( i, iClass ) ;
		}
	}
	else
	{
		nCount = m_lstClassInfo.GetSize() ;
		for ( i = 0; i < nCount; i ++ )
		{
			ECSClassInfo *	pDstClassInf = m_lstClassInfo.GetAt( i ) ;
			ECSClassInfo *	pSrcClassInf = image.m_lstClassInfo.GetAt( i ) ;
			ESLAssert( pDstClassInf != NULL ) ;
			if ( pSrcClassInf != NULL )
			{
				if ( !pDstClassInf->IsClassEqual( *pSrcClassInf ) )
				{
					m_strErrMsg = "クラス情報 \'"
						+ EString( pSrcClassInf->GetGlobalName() )
						+ "\' が一致しません。" ;
					return	ESLErrorMsg( m_strErrMsg ) ;
				}
			}
			lstClassIndex.SetAt( i, i ) ;
		}
	}
	//
	// 大域変数を結合
	//
	ESLError	err ;
	err = MergeGlobalVariable
		( m_csgGlobalType, image.m_csgGlobalType, dwFlags ) ;
	if ( err )
		return	err ;
	//
	err = MergeGlobalVariable
		( m_csgDataType, image.m_csgDataType, dwFlags ) ;
	if ( err )
		return	err ;
	//
	// naked データ領域を結合
	//
	err = MergeNakedGlobalVariable
		( m_bufNakedGlobal, image.m_bufNakedGlobal, dwFlags ) ;
	if ( err )
		return	err ;
	//
	err = MergeNakedGlobalVariable
		( m_bufNakedConst, image.m_bufNakedConst, dwFlags ) ;
	if ( err )
		return	err ;
	//
	err = MergeNakedGlobalVariable
		( m_bufNakedShared, image.m_bufNakedShared, dwFlags ) ;
	if ( err )
		return	err ;
	//
	// naked シンボル情報を結合
	//
	if ( dwFlags & mfOverwriteVariable )
	{
		m_wstaSymbols.RemoveAll() ;
	}
	nCount = image.m_wstaSymbols.GetSize() ;
	for ( i = 0; i < nCount; i ++ )
	{
		ECSWideString *
			pwstrSymbol = image.m_wstaSymbols.GetTagAt( i ) ;
		NAKED_SYMBOL_INFO *
			pSrcSymInf = image.m_wstaSymbols.GetObjectAt( i ) ;
		if ( (pwstrSymbol == NULL) || (pSrcSymInf == NULL) )
		{
			continue ;
		}
		NAKED_SYMBOL_INFO *
			pDstSymInf = m_wstaSymbols.GetAs( *pwstrSymbol ) ;
		if ( pDstSymInf != NULL )
		{
			m_strErrMsg = "シンボル \'"
				+ EString( *pwstrSymbol ) + "\' が重複しています。" ;
			return	ESLErrorMsg( m_strErrMsg ) ;
		}
		NAKED_SYMBOL_INFO *
			pSymInf = new NAKED_SYMBOL_INFO( *pSrcSymInf ) ;
		switch ( (int) (pSymInf->nAddress >> 56) & 0xFF )
		{
		case	roasCode:
			pSymInf->nAddress += dwImageBias ;
			break ;
		case	roasNakedGlobal:
			pSymInf->nAddress += dwNakedGlobalBias ;
			break ;
		case	roasNakedConst:
			pSymInf->nAddress += dwNakedConstBias ;
			break ;
		case	roasNakedShared:
			pSymInf->nAddress += dwNakedSharedBias ;
			break ;
		default:
			m_strErrMsg = "シンボル \'"
				+ EString( *pwstrSymbol )
				+ "\' をリアロケーションできません" ;
			return	ESLErrorMsg( m_strErrMsg ) ;
		}
		m_wstaSymbols.Add( *pwstrSymbol, pSymInf ) ;
	}
	//
	// 実行イメージを結合
	//
	m_bufImage.MergeBuffer( image.m_pImage, image.m_dwImageSize ) ;
	m_dwImageSize = m_bufImage.GetLength( ) ;
	m_pImage = (BYTE*) m_bufImage.ModifyBuffer( 0, m_dwImageSize ) ;
	if ( (m_pImage == NULL) && (m_dwImageSize > 0) )
	{
		return	ESLErrorMsg( "内部エラーが発生しました。" ) ;
	}
	//
	// 初期化関数アドレスを結合
	//
	if ( dwFlags & mfOverwriteVariable )
	{
		m_pifPrologue.RemoveAll( ) ;
		m_pifNakedPrologue.RemoveAll( ) ;
	}
	nCount = image.m_pifPrologue.GetSize( ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		m_pifPrologue.Add( image.m_pifPrologue.GetAt(i) + dwImageBias ) ;
	}
	nCount = image.m_pifNakedPrologue.GetSize( ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		m_pifNakedPrologue.Add( image.m_pifNakedPrologue.GetAt(i) + dwImageBias ) ;
	}
	//
	// 終了関数アドレスを結合
	//
	if ( dwFlags & mfOverwriteVariable )
	{
		m_pifEpilogue.RemoveAll( ) ;
		m_pifNakedEpilogue.RemoveAll( ) ;
	}
	nCount = image.m_pifEpilogue.GetSize( ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		m_pifEpilogue.Add( image.m_pifEpilogue.GetAt(i) + dwImageBias ) ;
	}
	nCount = image.m_pifNakedEpilogue.GetSize( ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		m_pifNakedEpilogue.Add( image.m_pifNakedEpilogue.GetAt(i) + dwImageBias ) ;
	}
	//
	// 関数名連想アドレス配列を結合
	//
	nCount = image.m_wstaFunc.GetSize( ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		ETaggedElement<ECSWideString,FUNC_ENTRY> *	pElement ;
		pElement = image.m_wstaFunc.GetAt( i ) ;
		ESLAssert( pElement != NULL ) ;
		if ( pElement == NULL )
			continue ;
		//
		if ( !(dwFlags & mfOverwriteFunction) )
		{
			FUNC_ENTRY *	pfe = m_wstaFunc.GetAs( pElement->Tag() ) ;
			if ( pfe != NULL )
			{
				if ( pfe->dwFlags & ECSTypeInfo::flagInline )
				{
					continue ;
				}
				m_strErrMsg = "関数 \'" + EString(pElement->Tag())
					+ "\' が異なるモジュールで二重に定義されています。" ;
				return	ESLErrorMsg( m_strErrMsg ) ;
			}
		}
		FUNC_ENTRY *	pfeSrc = pElement->GetObject() ;
		FUNC_ENTRY *	pfeMerge = new FUNC_ENTRY( *pfeSrc ) ;
		pfeMerge->dwAddress += dwImageBias ;
		if ( ((pfeMerge->dwBytes == (DWORD) -1)
					&& (pfeMerge->dwAddress >= m_dwImageSize))
			|| ((pfeMerge->dwBytes != (DWORD) -1)
					&& (pfeMerge->dwAddress + pfeMerge->dwBytes > m_dwImageSize)) )
		{
			m_strErrMsg = EString(pElement->Tag())
				+ " : 不正な関数アドレスが指定されています。" ;
			delete	pfeMerge ;
			return	ESLErrorMsg( m_strErrMsg ) ;
		}
		m_wstaFunc.SetAs( pElement->Tag(), pfeMerge ) ;
	}
	//
	// コード参照リストを結合
	//
	err = MergeCodeRef( m_extCodeRef, image.m_extCodeRef, dwImageBias ) ;
	if ( err )
	{
		return	err ;
	}
	err = MergeCodeRef
		( m_extNakedFuncRef, image.m_extNakedFuncRef, dwImageBias ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// 大域変数参照リストを結合
	//
	err = MergeGlobalRef
		( m_extGlobalRef, m_csgGlobalType,
			image.m_extGlobalRef, image.m_csgGlobalType, 
					dwImageBias, dwGlobalBias, dwFlags ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// 大域定数参照リストを結合
	//
	err = MergeGlobalRef
		( m_extDataRef, m_csgDataType,
			image.m_extDataRef, image.m_csgDataType,
					dwImageBias, dwDataBias, dwFlags ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// naked 大域データ参照リストを結合
	//
	err = MergeNakedGlobalRef
		( m_extNakedGlobalRef, image.m_extNakedGlobalRef, 
					dwImageBias, dwNakedGlobalBias, dwFlags ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// naked 不変データ参照リストを結合
	//
	err = MergeNakedGlobalRef
		( m_extNakedConstRef, image.m_extNakedConstRef, 
					dwImageBias, dwNakedConstBias, dwFlags ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// naked 共有データ参照リストを結合
	//
	err = MergeNakedGlobalRef
		( m_extNakedSharedRef, image.m_extNakedSharedRef, 
					dwImageBias, dwNakedSharedBias, dwFlags ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// クラス参照リストを結合
	//
	nCount = image.m_extClassIndexRef.GetSize() ;
	for ( i = 0; i < nCount; i ++ )
	{
		DWORD	dwRefAddr = image.m_extClassIndexRef[i] + dwImageBias ;
		DWORD	dwClassIndex = *((DWORD*)(m_pImage + dwRefAddr)) ;
		if ( dwClassIndex >= lstClassIndex.GetSize() )
		{
			return	ESLErrorMsg( "不正なクラス情報参照があります" ) ;
		}
		else
		{
			m_extClassIndexRef.Add( dwRefAddr ) ;
			*((DWORD*)(m_pImage + dwRefAddr)) = lstClassIndex[dwClassIndex] ;
		}
	}
	//
	// ネイティブ関数参照リストを結合
	//
	err = MergeImportNameRef
		( m_staNativeFuncName, m_impNativeFunc,
			image.m_staNativeFuncName, image.m_impNativeFunc, dwImageBias ) ;
	if ( err )
	{
		return	err ;
	}
	err = MergeImportNameRef
//		( m_staNakedNativeFuncName, m_impNakedNativeFunc,
//			image.m_staNakedNativeFuncName, image.m_impNakedNativeFunc, dwImageBias ) ;
		( m_vectorSysCall.GetEntryIndex(), m_impNakedNativeFunc,
			image.m_vectorSysCall.GetEntryIndex(),
				image.m_impNakedNativeFunc, dwImageBias ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// 外部参照（未解決変数）リストを結合
	//
	err = MergeGlobalRefList
		( m_impGlobalRef, m_csgGlobalType,
			image.m_impGlobalRef, image.m_csgGlobalType, dwImageBias, 0, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// 外部参照（未解決定数）リストを結合
	//
	err = MergeGlobalRefList
		( m_impDataRef, m_csgDataType,
			image.m_impDataRef, m_csgDataType, dwImageBias, 0, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// naked 外部参照（未解決変数）リストを結合
	//
	err = MergeNakedGlobalRefList
		( m_impNakedGlobalRef,
			image.m_impNakedGlobalRef, dwImageBias, 0, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	err = MergeNakedGlobalRefList
		( m_impNakedConstRef,
			image.m_impNakedConstRef, dwImageBias, 0, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	err = MergeNakedGlobalRefList
		( m_impNakedSharedRef,
			image.m_impNakedSharedRef, dwImageBias, 0, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// 外部参照（未解決関数）リストを結合
	//
	err = MergeNakedGlobalRefList
		( m_impFuncRef,
			image.m_impFuncRef, dwImageBias, 0, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	/*
	nCount = image.m_impFuncRef.GetSize() ;
	for ( i = 0; i < nCount; i ++ )
	{
		ETaggedElement< ECSWideString, ENumArray<DWORD> > *	pElement ;
		pElement = image.m_impFuncRef.GetAt( i ) ;
		ESLAssert( pElement != NULL ) ;
		if ( pElement == NULL )
		{
			continue ;
		}
		ENumArray<DWORD> *	pAddList = pElement->GetObject( ) ;
		if ( pAddList == NULL )
		{
			continue ;
		}
		ENumArray<DWORD> *	pList = m_impFuncRef.GetAs( pElement->Tag() ) ;
		if ( pList == NULL )
		{
			pList = new ENumArray<DWORD> ;
			m_impFuncRef.SetAs( pElement->Tag(), pList ) ;
		}
		for ( int j = 0; j < (int) pAddList->GetSize(); j ++ )
		{
			pList->Add( pAddList->GetAt(j) + dwImageBias ) ;
		}
	}
	*/
	err = MergeNakedGlobalRefList
		( m_impNakedFuncRef,
			image.m_impNakedFuncRef, dwImageBias, 0, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// 固定文字列参照リストを結合
	//
	nCount = image.m_extConstStr.GetSize( ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		ETaggedElement< ECSWideString, ENumArray<DWORD> > *	pElement ;
		pElement = image.m_extConstStr.GetAt( i ) ;
		ESLAssert( pElement != NULL ) ;
		if ( pElement == NULL )
		{
			continue ;
		}
		ENumArray<DWORD> *	pAddList = pElement->GetObject( ) ;
		if ( pAddList == NULL )
		{
			continue ;
		}
		unsigned int	iStr ;
		ENumArray<DWORD> *
			pList = m_extConstStr.GetAs( pElement->Tag(), &iStr ) ;
		if ( pList == NULL )
		{
			pList = new ENumArray<DWORD> ;
			unsigned int	nIndex =
				m_extConstStr.SetAs( pElement->Tag(), pList ) ;
			m_lstConstStr.InsertAt
				( nIndex, new ECSString( pElement->Tag() ) ) ;
		}
		for ( int j = 0; j < (int) pAddList->GetSize(); j ++ )
		{
			pList->Add( pAddList->GetAt(j) + dwImageBias ) ;
		}
	}
	//
	return	eslErrSuccess ;
}

// コードをアライメント
//////////////////////////////////////////////////////////////////////////////
DWORD ECSExecutionImageLinker::AlignCodeBuffer( int nAlign )
{
	if ( m_bufImage.GetLength() % nAlign )
	{
		int	nCodeAlign = nAlign - (m_bufImage.GetLength() % nAlign) ;
		::eslFillMemory( m_bufImage.PutBuffer( nCodeAlign ), 0, nCodeAlign ) ;
		m_bufImage.Flush( nCodeAlign ) ;
		//
		m_dwImageSize = m_bufImage.GetLength( ) ;
		m_pImage = (BYTE*) m_bufImage.ModifyBuffer( 0, m_dwImageSize ) ;
	}
	return	m_bufImage.GetLength() ;
}

// 大域変数を結合
//////////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImageLinker::MergeGlobalVariable
	( ECSGlobal & csgDst, const ECSGlobal & csgSrc, DWORD dwFlags )
{
	unsigned int	i, nCount ;
	nCount = csgSrc.m_varArray.GetSize( ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		const wchar_t *	pwszName = csgSrc.m_staObjName.GetAt( i ) ;
		ECSObject *	pObj = csgSrc.m_varArray.GetAt( i ) ;
		ESLAssert( pwszName != NULL ) ;
		ESLAssert( pObj != NULL ) ;
		if ( (pwszName == NULL) || (pObj == NULL) )
		{
			return	ESLErrorMsg( "不正な大域変数を発見しました。" ) ;
		}
		int	nVarIndex = csgDst.m_staObjName.FindIndex( pwszName ) ;
		if ( nVarIndex >= 0 )
		{
			if ( !(dwFlags & mfOverwriteVariable) )
			{
				m_strErrMsg = "変数 \'" + EString(pwszName)
					+ "\' が異なるモジュールで二重に定義されています。" ;
				return	ESLErrorMsg( m_strErrMsg ) ;
			}
			csgDst.m_varArray.SetAt( nVarIndex, ECSTypeInfo::DuplicateType( pObj ) ) ;
			continue ;
		}
		csgDst.AddVariable( pwszName, ECSTypeInfo::DuplicateType( pObj ) ) ;
	}
	return	eslErrSuccess ;
}

// 変数参照リストを結合
//////////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImageLinker::MergeCodeRef
	( ENumArray<DWORD> & lstDst,
		const ENumArray<DWORD> & lstSrc, DWORD dwImageBias )
{
	DWORD	i, dwCount ;
	dwCount = lstSrc.GetSize( ) ;
	for ( i = 0; i < dwCount; i ++ )
	{
		DWORD	dwRefAddr = lstSrc.GetAt(i) + dwImageBias ;
		if ( dwRefAddr >= m_dwImageSize )
		{
			return	ESLErrorMsg( "不正な関数ポインタがあります" ) ;
		}
		*((DWORD*)(m_pImage + dwRefAddr)) += dwImageBias ;
		lstDst.Add( dwRefAddr ) ;
	}
	return	eslErrSuccess ;
}

ESLError ECSExecutionImageLinker::MergeGlobalRef
	( ENumArray<DWORD> & lstDst, const ECSGlobal & varDst,
		const ENumArray<DWORD> & lstSrc, const ECSGlobal & varSrc,
		DWORD dwImageBias, DWORD dwGlobalBias, DWORD dwFlags )
{
	DWORD	i, dwCount ;
	dwCount = lstSrc.GetSize( ) ;
	for ( i = 0; i < dwCount; i ++ )
	{
		DWORD	dwRefAddr = lstSrc.GetAt(i) + dwImageBias ;
		if ( dwRefAddr >= m_dwImageSize )
		{
			return	ESLErrorMsg( "不正な変数ポインタがあります" ) ;
		}
		if ( !(dwFlags & mfOverwriteVariable) )
		{
			*((DWORD*)(m_pImage + dwRefAddr)) += dwGlobalBias ;
		}
		else
		{
			DWORD	dwSrcRefIndex = *((DWORD*)(m_pImage + dwRefAddr)) ;
			const wchar_t *	pwszVarName =
						varSrc.m_staObjName.GetAt( dwSrcRefIndex ) ;
			if ( pwszVarName == NULL )
			{
				return	ESLErrorMsg( "不正な変数参照があります" ) ;
			}
			int		nDstRefIndex =
						varDst.m_staObjName.FindIndex( pwszVarName ) ;
			if ( nDstRefIndex < 0 )
			{
				m_strErrMsg = "変数 \"" + EString( pwszVarName )
											+ "\" が見つかりません。" ;
				return	ESLErrorMsg( m_strErrMsg ) ;
			}
			*((DWORD*)(m_pImage + dwRefAddr)) = nDstRefIndex ;
		}
		lstDst.Add( dwRefAddr ) ;
	}
	return	eslErrSuccess ;
}

ESLError ECSExecutionImageLinker::MergeGlobalRefList
	( TaggedRefAddresList & impDst, const ECSGlobal & varDst,
		const TaggedRefAddresList & impSrc, const ECSGlobal & varSrc,
		DWORD dwImageBias, DWORD dwGlobalBias, DWORD dwFlags )
{
	ESLError		err ;
	unsigned int	i, nCount ;
	nCount = impSrc.GetSize( ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		ETaggedElement< ECSWideString, ENumArray<DWORD> > *	pElement ;
		pElement = impSrc.GetAt( i ) ;
		ESLAssert( pElement != NULL ) ;
		if ( pElement == NULL )
			continue ;
		//
		ENumArray<DWORD> *
			pList = impDst.GetAs( pElement->Tag() ) ;
		if ( pList == NULL )
		{
			pList = new ENumArray<DWORD> ;
			impDst.SetAs( pElement->Tag(), pList ) ;
		}
		//
		err = MergeGlobalRef
			( *pList, varDst, *(pElement->GetObject()),
					varSrc, dwImageBias, dwGlobalBias, dwFlags ) ;
		if ( err )
		{
			return	err ;
		}
	}
	return	eslErrSuccess ;
}

// naked データ領域を結合
//////////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImageLinker::MergeNakedGlobalVariable
	( ECSBuffer & bufDst, const ECSBuffer & bufSrc, DWORD dwFlags )
{
	if ( dwFlags & mfOverwriteVariable )
	{
		if ( bufDst.GetLength()
			!= ((bufSrc.GetLength() + 0x0F) & ~0x0F) )
		{
			return	ESLErrorMsg
				( "naked データ領域のサイズが"
					"一致しないのでオーバーライドできません" ) ;
		}
		eslMoveMemory
			( bufDst.GetBuffer(),
				bufSrc.GetBuffer(), bufSrc.GetLength() ) ;
	}
	else
	{
		bufDst.MergeBuffer
			( bufSrc.GetBuffer(), bufSrc.GetLength() ) ;
	}
	return	eslErrSuccess ;
}

// naked データ参照リストを結合
//////////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImageLinker::MergeNakedGlobalRef
	( ENumArray<DWORD> & lstDst, const ENumArray<DWORD> & lstSrc,
		DWORD dwImageBias, DWORD dwGlobalBias, DWORD dwFlags )
{
	DWORD	i, dwCount ;
	dwCount = lstSrc.GetSize( ) ;
	for ( i = 0; i < dwCount; i ++ )
	{
		DWORD	dwRefAddr = lstSrc.GetAt(i) + dwImageBias ;
		if ( dwRefAddr >= m_dwImageSize )
		{
			return	ESLErrorMsg( "不正な変数ポインタがあります" ) ;
		}
		if ( !(dwFlags & mfOverwriteVariable) )
		{
			*((DWORD*)(m_pImage + dwRefAddr)) += dwGlobalBias ;
		}
		lstDst.Add( dwRefAddr ) ;
	}
	return	eslErrSuccess ;
}

ESLError ECSExecutionImageLinker::MergeNakedGlobalRefList
	( TaggedRefAddresList & impDst,
		const TaggedRefAddresList & impSrc,
		DWORD dwImageBias, DWORD dwGlobalBias, DWORD dwFlags )
{
	ESLError		err ;
	unsigned int	i, nCount ;
	nCount = impSrc.GetSize( ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		ETaggedElement< ECSWideString, ENumArray<DWORD> > *	pElement ;
		pElement = impSrc.GetAt( i ) ;
		ESLAssert( pElement != NULL ) ;
		if ( pElement == NULL )
			continue ;
		//
		ENumArray<DWORD> *
			pList = impDst.GetAs( pElement->Tag() ) ;
		if ( pList == NULL )
		{
			pList = new ENumArray<DWORD> ;
			impDst.SetAs( pElement->Tag(), pList ) ;
		}
		//
		err = MergeNakedGlobalRef
			( *pList, *(pElement->GetObject()),
					dwImageBias, dwGlobalBias, dwFlags ) ;
		if ( err )
		{
			return	err ;
		}
	}
	return	eslErrSuccess ;
}

// インポート名関連リストを結合
//////////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImageLinker::MergeImportNameRef
	( ECSStrBufTagArray & staDstName,
		ENumArray<DWORD> & lstDst, 
		const ECSStrBufTagArray & staSrcName,
		const ENumArray<DWORD> & lstSrc, DWORD dwImageBias )
{
	//
	// 名前リストを結合し、インデックス置き換えリストを生成
	//
	ENumArray<DWORD>	lstNameRemap ;
	int	i, nCount ;
	nCount = staSrcName.GetSize() ;
	for ( i = 0; i < nCount; i ++ )
	{
		int	iDst = staDstName.FindIndex( staSrcName.GetAt( i ) ) ;
		if ( iDst < 0 )
		{
			iDst = staDstName.Add( staSrcName.GetAt( i ) ) ;
		}
		lstNameRemap.SetAt( i, iDst ) ;
	}
	//
	// 参照インデックスをリマップ
	//
	nCount = lstSrc.GetSize() ;
	for ( i = 0; i < nCount; i ++ )
	{
		DWORD	dwRefAddr = lstSrc.GetAt(i) + dwImageBias ;
		if ( dwRefAddr >= m_dwImageSize )
		{
			return	ESLErrorMsg( "不正な名前参照ポインタがあります" ) ;
		}
		DWORD	dwIndex = *((DWORD*)(m_pImage + dwRefAddr)) ;
		if ( dwIndex >= lstNameRemap.GetSize() )
		{
			return	ESLErrorMsg( "不正な名前参照ポインタがあります" ) ;
		}
		ESLAssert( lstDst.Find( dwRefAddr ) < 0 ) ;
		lstDst.Add( dwRefAddr ) ;
		*((DWORD*)(m_pImage + dwRefAddr)) = lstNameRemap[dwIndex] ;
	}
	return	eslErrSuccess ;
}

ESLError ECSExecutionImageLinker::MergeImportNameRef
	( SSystem::SIndexedArray
			<SSystem::SString,const wchar_t*> & staDstName,
		ENumArray<DWORD> & lstDst, 
		const SSystem::SIndexedArray
			<SSystem::SString,const wchar_t*> & staSrcName,
		const ENumArray<DWORD> & lstSrc, DWORD dwImageBias )
{
	//
	// 名前リストを結合し、インデックス置き換えリストを生成
	//
	ENumArray<DWORD>	lstNameRemap ;
	int	i, nCount ;
	nCount = staSrcName.GetLength() ;
	for ( i = 0; i < nCount; i ++ )
	{
		SSystem::SString *	pSrcName = staSrcName.GetAt( i ) ;
		int	iDst = staDstName.FindIndex( *pSrcName ) ;
		if ( iDst < 0 )
		{
			iDst = staDstName.Add( new SSystem::SString( *pSrcName ) ) ;
		}
		lstNameRemap.SetAt( i, iDst ) ;
	}
	//
	// 参照インデックスをリマップ
	//
	nCount = lstSrc.GetSize() ;
	for ( i = 0; i < nCount; i ++ )
	{
		DWORD	dwRefAddr = lstSrc.GetAt(i) + dwImageBias ;
		if ( dwRefAddr >= m_dwImageSize )
		{
			return	ESLErrorMsg( "不正な名前参照ポインタがあります" ) ;
		}
		DWORD	dwIndex = *((DWORD*)(m_pImage + dwRefAddr)) ;
		if ( dwIndex >= lstNameRemap.GetSize() )
		{
			return	ESLErrorMsg( "不正な名前参照ポインタがあります" ) ;
		}
		ESLAssert( lstDst.Find( dwRefAddr ) < 0 ) ;
		lstDst.Add( dwRefAddr ) ;
		*((DWORD*)(m_pImage + dwRefAddr)) = lstNameRemap[dwIndex] ;
	}
	return	eslErrSuccess ;
}

// 使用していないクラス情報を削除する（naked only mode 用）
//////////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImageLinker::TrimUnsedClassInfo( void )
{
	//
	// コードからクラスIDの参照を列挙
	//
	ENumArray<DWORD>	lstClassUsed ;
	lstClassUsed.SetSize( m_lstClassInfo.GetSize() ) ;
	//
	const int	nRefClassCount = m_extClassIndexRef.GetSize() ;
	int	i, j ;
	for ( i = 0; i < nRefClassCount; i ++ )
	{
		DWORD	dwRefAddr = m_extClassIndexRef.GetAt(i) ;
		if ( dwRefAddr + sizeof(DWORD) <= m_dwImageSize )
		{
			DWORD	dwClassID = *((DWORD*)(m_pImage + dwRefAddr)) ;
			ECSClassInfo *	pClassInf = m_lstClassInfo.GetAt(dwClassID) ;
			if ( pClassInf != NULL )
			{
				pClassInf->EnumerateReferenceClasses( lstClassUsed, this ) ;
			}
			else
			{
				lstClassUsed.SetAt
					( dwClassID, lstClassUsed.GetAt(dwClassID) + 1 ) ;
			}
		}
	}
	//
	// 参照されないクラスを列挙し削除
	//
	ENumArray<DWORD>	lstClassRemap ;
	lstClassRemap.SetSize( lstClassUsed.GetSize() ) ;
	for ( i = 0, j = 0; i < (int) m_lstClassInfo.GetSize(); i ++ )
	{
		if ( lstClassUsed.GetAt(i) == 0 )
		{
			ECSClassInfo *	pClassInf = m_lstClassInfo.GetAt(i) ;
			if ( pClassInf != NULL )
			{
				ESLTrace
					( "remove class info \"%s\"\n",
						EString(pClassInf->GetGlobalName()).CharPtr() ) ;
			}
			m_lstClassInfo.SetAt( i, NULL ) ;
		}
		else
		{
			ECSClassInfo *	pClassInf = m_lstClassInfo.GetAt(i) ;
			if ( pClassInf != NULL )
			{
				ESLTrace
					( "no remove class info \"%s\"\n",
						EString(pClassInf->GetGlobalName()).CharPtr() ) ;
			}
			lstClassRemap.SetAt( i, j ++ ) ;
		}
	}
	m_lstClassInfo.TrimEmpty() ;
	//
	// コードから参照されるクラスIDのリマップ
	//
	for ( i = 0; i < nRefClassCount; i ++ )
	{
		DWORD	dwRefAddr = m_extClassIndexRef.GetAt(i) ;
		if ( dwRefAddr + sizeof(DWORD) <= m_dwImageSize )
		{
			DWORD	dwClassID = *((DWORD*)(m_pImage + dwRefAddr)) ;
			*((DWORD*)(m_pImage + dwRefAddr)) = lstClassRemap.GetAt( dwClassID ) ;
		}
	}
	//
	m_vectorNewObject.RemoveAll() ;
	for ( i = 0, j = 0; i < (int) m_lstClassInfo.GetSize(); i ++ )
	{
		ECSClassInfo *	pClassInf = m_lstClassInfo.GetAt(i) ;
		ESLAssert( pClassInf != NULL ) ;
		if ( pClassInf != NULL )
		{
			m_vectorNewObject.AddEntry( pClassInf->GetGlobalName() ) ;
		}
		else
		{
			m_vectorNewObject.AddEntry( L"" ) ;
		}
	}
	return	eslErrSuccess ;
}

// 未解決リンクを解決する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImageLinker::SolveLinkInfo( bool fNoImport )
{
	ESLError	err ;
	int			i, j ;
	//
	// naked クラスの初期値イメージ・仮想関数ベクタがなければ追加する
	//
	err = CommitAllClassInitImage() ;
	if ( err )
	{
		return	err ;
	}
	//
	// クラス情報のメンバ関数アドレスを確定する
	//
	err = CommitAllClassMethod() ;
	if ( err )
	{
		return	err ;
	}
	//
	// 外部参照（未解決変数）を解決する
	//
	err = SolveImportObjectSymbol
		( m_extGlobalRef, m_impGlobalRef, m_csgGlobalType ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// 外部参照（未解決定数）を解決する
	//
	err = SolveImportObjectSymbol
		( m_extDataRef, m_impDataRef, m_csgDataType ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// naked 外部参照（未解決定数）を解決する
	//
	err = SolveImportNakedSymbol
			( m_extNakedGlobalRef, m_impNakedGlobalRef ) ;
	if ( err )
	{
		return	err ;
	}
	err = SolveImportNakedSymbol
			( m_extNakedConstRef, m_impNakedConstRef ) ;
	if ( err )
	{
		return	err ;
	}
	err = SolveImportNakedSymbol
			( m_extNakedSharedRef, m_impNakedSharedRef ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// 外部参照（未解決関数）
	//
	if ( !fNoImport )
	{
		err = SolveImportFunctionPointer( m_extCodeRef, m_impFuncRef, false ) ;
		if ( err )
		{
			return	err ;
		}
		err = SolveImportFunctionPointer
					( m_extNakedFuncRef, m_impNakedFuncRef, true ) ;
		if ( err )
		{
			return	err ;
		}
	}
	//
	// 固定文字列参照を解決する
	//
	for ( i = 0; i < (int) m_extConstStr.GetSize(); i ++ )
	{
		ETaggedElement< ECSWideString, ENumArray<DWORD> > *
								pElement = m_extConstStr.GetAt( i ) ;
		if ( pElement == NULL )
		{
			continue ;
		}
		int	iStr = i ;
		if ( (m_lstConstStr.GetAt(i) == NULL)
			|| (m_lstConstStr.GetAt(i)->m_varStr != pElement->Tag()) )
		{
			for ( j = 0; j < (int) m_lstConstStr.GetSize(); j ++ )
			{
				ECSString *	pStr = m_lstConstStr.GetAt( j ) ;
				if ( (pStr != NULL) && (pStr->m_varStr == pElement->Tag()) )
				{
					iStr = j ;
					break ;
				}
			}
		}
		ENumArray<DWORD> *	pList = pElement->GetObject() ;
		if ( pList == NULL )
		{
			continue ;
		}
		for ( j = 0; j < (int) pList->GetSize(); j ++ )
		{
			DWORD	dwRefAddr = pList->GetAt( j ) ;
			if ( dwRefAddr >= m_dwImageSize )
			{
				return	ESLErrorMsg( "不正な固定文字列参照があります。" ) ;
			}
			*((DWORD*)(m_pImage + dwRefAddr)) = iStr ;
		}
	}
	//
	// naked 変数イメージ複製
	//
	m_bufNakedGlobalInit.ResizeBuffer( m_bufNakedGlobal.GetLength(), 0 ) ;
	//
	::eslMoveMemory
		( m_bufNakedGlobalInit.GetBuffer(),
			m_bufNakedGlobal.GetBuffer(), m_bufNakedGlobal.GetLength() ) ;
	//
	return	eslErrSuccess ;
}

// リンク用データを削除
//////////////////////////////////////////////////////////////////////////////
void ECSExecutionImageLinker::DeleteLinkerInfo( void )
{
	m_extCodeRef.RemoveAll( ) ;
	m_extGlobalRef.RemoveAll( ) ;
	m_extDataRef.RemoveAll( ) ;
	m_extClassIndexRef.RemoveAll( ) ;
	//
	// ※m_extConstStr は固定文字列として
	// 　ファイルに書き出す際に使用するので削除しない
	//
//	m_impGlobalRef.RemoveAll() ;
//	m_impDataRef.RemoveAll() ;
//	m_impFuncRef.RemoveAll() ;
}

// 未解決のリンクの総数を取得
//////////////////////////////////////////////////////////////////////////////
unsigned int ECSExecutionImageLinker::GetUnsolvedLinkCount( void ) const
{
	return	m_impGlobalRef.GetSize()
			+ m_impDataRef.GetSize()
			+ m_impNakedGlobalRef.GetSize()
			+ m_impNakedConstRef.GetSize()
			+ m_impNakedSharedRef.GetSize()
			+ m_impNakedFuncRef.GetSize()
			+ m_impFuncRef.GetSize() ;
}

// 未解決のリンク名を取得
//////////////////////////////////////////////////////////////////////////////
ECSWideString
	ECSExecutionImageLinker::GetUnsolveLinkName( unsigned int nIndex ) const
{
	if ( nIndex < m_impGlobalRef.GetSize() )
	{
		ESLAssert( m_impGlobalRef.GetAt(nIndex) != NULL ) ;
		return	m_impGlobalRef.GetAt(nIndex)->Tag() ;
	}
	nIndex -= m_impGlobalRef.GetSize() ;
	//
	if ( nIndex < m_impDataRef.GetSize() )
	{
		ESLAssert( m_impDataRef.GetAt(nIndex) != NULL ) ;
		return	m_impDataRef.GetAt(nIndex)->Tag() ;
	}
	nIndex -= m_impDataRef.GetSize() ;
	//
	if ( nIndex < m_impFuncRef.GetSize() )
	{
		ESLAssert( m_impFuncRef.GetAt(nIndex) != NULL ) ;
		return	m_impFuncRef.GetAt(nIndex)->Tag() ;
	}
	nIndex -= m_impFuncRef.GetSize() ;
	//
	if ( nIndex < m_impNakedGlobalRef.GetSize() )
	{
		ESLAssert( m_impNakedGlobalRef.GetAt(nIndex) != NULL ) ;
		return	m_impNakedGlobalRef.GetAt(nIndex)->Tag() ;
	}
	nIndex -= m_impNakedGlobalRef.GetSize() ;
	//
	if ( nIndex < m_impNakedConstRef.GetSize() )
	{
		ESLAssert( m_impNakedConstRef.GetAt(nIndex) != NULL ) ;
		return	m_impNakedConstRef.GetAt(nIndex)->Tag() ;
	}
	nIndex -= m_impNakedConstRef.GetSize() ;
	//
	if ( nIndex < m_impNakedSharedRef.GetSize() )
	{
		ESLAssert( m_impNakedSharedRef.GetAt(nIndex) != NULL ) ;
		return	m_impNakedSharedRef.GetAt(nIndex)->Tag() ;
	}
	nIndex -= m_impNakedSharedRef.GetSize() ;
	return	ECSWideString() ;
}

// naked クラスの初期値イメージ・仮想関数ベクタがなければ追加する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImageLinker::CommitAllClassInitImage( void )
{
	ECSCompiler					compiler ;
	ECSExecutionImageCompiler	csxiTemp ;
	compiler.Initialize( &csxiTemp ) ;
	//
	ESLError	err ;
	int			i, nCount ;
	nCount = m_lstClassInfo.GetSize() ;
	for ( i = 0; i < nCount; i ++ )
	{
		ECSClassInfo *	pClassInf = m_lstClassInfo.GetAt( i ) ;
		ESLAssert( pClassInf != NULL ) ;
		if ( (pClassInf->GetAttribute() & ECSTypeInfo::flagNativeObject)
			|| !pClassInf->IsNakedMemoryClass() )
		{
			continue ;
		}
		if ( GetFunctionAddress
			( pClassInf->GetGlobalName() + L"::<image>" ) == NULL )
		{
			err = compiler.CompileCodeNakedClassInitImage( pClassInf ) ;
			if ( err )
			{
				return	err ;
			}
		}
		if ( GetFunctionAddress
			( pClassInf->GetGlobalName() + L"::<vfvector>" ) == NULL )
		{
			err = compiler.CompileCodeNakedVirtualFuncVector( pClassInf ) ;
			if ( err )
			{
				return	err ;
			}
		}
	}
	err = compiler.FinishCompile
		( ECSCompiler::flagForceImplementAll ) ;
	if ( err )
	{
		return	err ;
	}
	return	MergeImage
		( csxiTemp, ECSExecutionImageLinker::mfMirrorClassInf ) ;
}

// クラス情報のメンバ関数アドレスを確定する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImageLinker::CommitAllClassMethod( void )
{
	int	i, j, nCount ;
	nCount = m_lstClassInfo.GetSize() ;
	for ( i = 0; i < nCount; i ++ )
	{
		ECSClassInfo *	pClassInf = m_lstClassInfo.GetAt( i ) ;
		ESLAssert( pClassInf != NULL ) ;
		if ( pClassInf->GetAttribute() & ECSTypeInfo::flagNativeObject )
		{
			continue ;
		}
		for ( j = 0; j < (int) pClassInf->GetFunctionCount(); j ++ )
		{
			ECSClassInfo::MemberFunction *
					pFunc = pClassInf->GetFunctionAt( j ) ;
			ESLAssert( pFunc != NULL ) ;
			if ( pFunc->GetAttribute() & ECSTypeInfo::flagNativeObject )
			{
				continue ;
			}
			if ( pFunc->GetAttribute() & ECSTypeInfo::flagAbstract )
			{
				pFunc->m_fpFuncPointer.m_ftType =
						ECS_FUNCTION_POINTER::funcScriptCall ;
				pFunc->m_fpFuncPointer.m_varFunc.addrScript = (DWORD) -1 ;
				continue ;
			}
			DWORD *	pdwFuncAddr =
				GetFunctionAddress( pFunc->GetGlobalName() ) ;
			if ( pdwFuncAddr != NULL )
			{
				pFunc->m_fpFuncPointer.m_ftType =
						ECS_FUNCTION_POINTER::funcScriptCall ;
				pFunc->m_fpFuncPointer.m_varFunc.addrScript = *pdwFuncAddr ;
			}
			else if ( !pClassInf->IsNakedMemoryClass() )
			{
				m_strErrMsg = "関数 \'"
						+ EString( pFunc->GetGlobalName() )
						+ "\' が見つかりません。" ;
				return	ESLErrorMsg( m_strErrMsg ) ;
			}
		}
		if ( pClassInf->TestMemberVariable( *pClassInf ) )
		{
			m_strErrMsg =
					EString( pClassInf->GetGlobalName() )
					+ " クラスは無限入れ子になっています。"
					"クラス自体のメンバ変数は参照型にしなければなりません。" ;
			return	ESLErrorMsg( m_strErrMsg ) ;
		}
	}
	return	eslErrSuccess ;
}

// object シンボル参照解決
//////////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImageLinker::SolveImportObjectSymbol
	( ENumArray<DWORD> & expList,
		TaggedRefAddresList & impRef,
		const ECSGlobal & csgGlobalType )
{
	ETaggedElement< ECSWideString, ENumArray<DWORD> > *	pElement ;
	for ( int i = impRef.GetSize() - 1; i >= 0; i -- )
	{
		pElement = impRef.GetAt( i ) ;
		ESLAssert( pElement != NULL ) ;
		if ( pElement == NULL )
			continue ;
		//
		int	iVar = csgGlobalType.m_staObjName.FindIndex( pElement->Tag() ) ;
		if ( iVar >= 0 )
		{
			ENumArray<DWORD> *	pList = pElement->GetObject( ) ;
			for ( int j = 0; j < (int) pList->GetSize(); j ++ )
			{
				DWORD	dwRefAddr = pList->GetAt( j ) ;
				if ( dwRefAddr >= m_dwImageSize )
				{
					return	ESLErrorMsg( "不正な変数ポインタがあります" ) ;
				}
				expList.Add( dwRefAddr ) ;
				*((DWORD*)(m_pImage + dwRefAddr)) = iVar ;
			}
			impRef.RemoveAt( i ) ;
		}
	}
	return	eslErrSuccess ;
}

// naked シンボル参照解決
//////////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImageLinker::SolveImportNakedSymbol
	( ENumArray<DWORD> & expList, TaggedRefAddresList & impRef )
{
	ETaggedElement< ECSWideString, ENumArray<DWORD> > *	pElement ;
	for ( int i = impRef.GetSize() - 1; i >= 0; i -- )
	{
		pElement = impRef.GetAt( i ) ;
		ESLAssert( pElement != NULL ) ;
		if ( pElement == NULL )
			continue ;
		//
		NAKED_SYMBOL_INFO *	pSymbol =
				m_wstaSymbols.GetAs( pElement->Tag() ) ;
		if ( pSymbol != NULL )
		{
			ENumArray<DWORD> *	pList = pElement->GetObject( ) ;
			for ( int j = 0; j < (int) pList->GetSize(); j ++ )
			{
				DWORD	dwRefAddr = pList->GetAt( j ) ;
				if ( dwRefAddr >= m_dwImageSize )
				{
					return	ESLErrorMsg
						( "不正な naked 変数ポインタがあります" ) ;
				}
				expList.Add( dwRefAddr ) ;
				*((INT64*)(m_pImage + dwRefAddr)) = pSymbol->nAddress ;
			}
			impRef.RemoveAt( i ) ;
		}
	}
	return	eslErrSuccess ;
}

// 関数参照解決
//////////////////////////////////////////////////////////////////////////
ESLError ECSExecutionImageLinker::SolveImportFunctionPointer
	( ENumArray<DWORD> & expList,
		TaggedRefAddresList & impRef, bool fRef64 )
{
	ETaggedElement< ECSWideString, ENumArray<DWORD> > *	pElement ;
	for ( int i = impRef.GetSize() - 1; i >= 0; i -- )
	{
		pElement = impRef.GetAt( i ) ;
		ESLAssert( pElement != NULL ) ;
		if ( pElement == NULL )
		{
			continue ;
		}
		DWORD *	pdwFuncAddr = GetFunctionAddress( pElement->Tag() ) ;
		if ( pdwFuncAddr == NULL )
		{
			m_strErrMsg = "関数 \'"
					+ EString( pElement->Tag() ) + "\' が見つかりません。" ;
			return	ESLErrorMsg( m_strErrMsg ) ;
		}
		ENumArray<DWORD> *	pList = pElement->GetObject() ;
		if ( pList == NULL )
		{
			continue ;
		}
		for ( int j = 0; j < (int) pList->GetSize(); j ++ )
		{
			DWORD	dwRefAddr = pList->GetAt( j ) ;
			if ( dwRefAddr >= m_dwImageSize )
			{
				m_strErrMsg = "不正な関数参照 \'"
						+ EString( pElement->Tag() ) + "\' があります。" ;
				return	ESLErrorMsg( m_strErrMsg ) ;
			}
			expList.Add( dwRefAddr ) ;
			*((DWORD*)(m_pImage + dwRefAddr)) = *pdwFuncAddr ;
			if ( fRef64 )
			{
				*((DWORD*)(m_pImage + dwRefAddr + 4)) = (roasCode << 24) ;
			}
		}
		impRef.RemoveAt( i ) ;
	}
	return	eslErrSuccess ;
}

// コードの参照情報削除とコードのシフト
//////////////////////////////////////////////////////////////////////////
void ECSExecutionImageLinker::RemoveCodeImage( DWORD dwAddress, int nRange )
{
	RemoveCodeReferenceInfo( dwAddress, nRange ) ;
	ShiftCodeImage( dwAddress + nRange, - nRange ) ;
}

// コードの参照情報削除
//////////////////////////////////////////////////////////////////////////
void ECSExecutionImageLinker::RemoveCodeReferenceInfo( DWORD dwAddress, int nRange )
{
	RemoveCodeReferenceAddress( m_pifPrologue, dwAddress, nRange ) ;
	RemoveCodeReferenceAddress( m_pifEpilogue, dwAddress, nRange ) ;
	RemoveCodeReferenceAddress( m_pifNakedPrologue, dwAddress, nRange ) ;
	RemoveCodeReferenceAddress( m_pifNakedEpilogue, dwAddress, nRange ) ;
	//
	RemoveCodeReferenceAddress( m_extCodeRef, dwAddress, nRange ) ;
	RemoveCodeReferenceAddress( m_extGlobalRef, dwAddress, nRange ) ;
	RemoveCodeReferenceAddress( m_extDataRef, dwAddress, nRange ) ;
	RemoveCodeReferenceAddress( m_extClassIndexRef, dwAddress, nRange ) ;
	//
	RemoveCodeReferenceAddress( m_extNakedFuncRef, dwAddress, nRange ) ;
	RemoveCodeReferenceAddress( m_extNakedGlobalRef, dwAddress, nRange ) ;
	RemoveCodeReferenceAddress( m_extNakedConstRef, dwAddress, nRange ) ;
	RemoveCodeReferenceAddress( m_extNakedSharedRef, dwAddress, nRange ) ;
	//
	RemoveCodeReferenceAddress( m_impNativeFunc, dwAddress, nRange ) ;
	RemoveCodeReferenceAddress( m_impNakedNativeFunc, dwAddress, nRange ) ;
	//
	RemoveCodeReferenceAddressList( m_impGlobalRef, dwAddress, nRange ) ;
	RemoveCodeReferenceAddressList( m_impDataRef, dwAddress, nRange ) ;
	//
	RemoveCodeReferenceAddressList( m_impNakedGlobalRef, dwAddress, nRange ) ;
	RemoveCodeReferenceAddressList( m_impNakedConstRef, dwAddress, nRange ) ;
	RemoveCodeReferenceAddressList( m_impNakedSharedRef, dwAddress, nRange ) ;
	RemoveCodeReferenceAddressList( m_impNakedFuncRef, dwAddress, nRange ) ;
	//
	RemoveCodeReferenceAddressList( m_impFuncRef, dwAddress, nRange ) ;
	RemoveCodeReferenceAddressList( m_extConstStr, dwAddress, nRange ) ;
}

// コードのシフト操作
//（特定アドレス以降への参照リンク情報をオフセットとコード自体のムーブ）
//////////////////////////////////////////////////////////////////////////
void ECSExecutionImageLinker::ShiftCodeImage
	( DWORD dwAddress, int nShiftOffset )
{
	ShiftCodeReferenceAddress( m_pifPrologue, dwAddress, nShiftOffset ) ;
	ShiftCodeReferenceAddress( m_pifEpilogue, dwAddress, nShiftOffset ) ;
	ShiftCodeReferenceAddress( m_pifNakedPrologue, dwAddress, nShiftOffset ) ;
	ShiftCodeReferenceAddress( m_pifNakedEpilogue, dwAddress, nShiftOffset ) ;
	//
	ShiftCodeReferenceAddress( m_extCodeRef, dwAddress, nShiftOffset ) ;
	ShiftCodeReferenceAddress( m_extGlobalRef, dwAddress, nShiftOffset ) ;
	ShiftCodeReferenceAddress( m_extDataRef, dwAddress, nShiftOffset ) ;
	ShiftCodeReferenceAddress( m_extClassIndexRef, dwAddress, nShiftOffset ) ;
	//
	ShiftCodeReferenceAddress( m_extNakedFuncRef, dwAddress, nShiftOffset ) ;
	ShiftCodeReferenceAddress( m_extNakedGlobalRef, dwAddress, nShiftOffset ) ;
	ShiftCodeReferenceAddress( m_extNakedConstRef, dwAddress, nShiftOffset ) ;
	ShiftCodeReferenceAddress( m_extNakedSharedRef, dwAddress, nShiftOffset ) ;
	//
	ShiftCodeReferenceAddress( m_impNativeFunc, dwAddress, nShiftOffset ) ;
	ShiftCodeReferenceAddress( m_impNakedNativeFunc, dwAddress, nShiftOffset ) ;
	//
	ShiftCodeReferenceAddressList( m_impGlobalRef, dwAddress, nShiftOffset ) ;
	ShiftCodeReferenceAddressList( m_impDataRef, dwAddress, nShiftOffset ) ;
	//
	ShiftCodeReferenceAddressList( m_impNakedGlobalRef, dwAddress, nShiftOffset ) ;
	ShiftCodeReferenceAddressList( m_impNakedConstRef, dwAddress, nShiftOffset ) ;
	ShiftCodeReferenceAddressList( m_impNakedSharedRef, dwAddress, nShiftOffset ) ;
	ShiftCodeReferenceAddressList( m_impNakedFuncRef, dwAddress, nShiftOffset ) ;
	//
	ShiftCodeReferenceAddressList( m_impFuncRef, dwAddress, nShiftOffset ) ;
	ShiftCodeReferenceAddressList( m_extConstStr, dwAddress, nShiftOffset ) ;
	//
	unsigned int	nFuncCount = m_wstaFunc.GetSize() ;
	for ( unsigned int i = 0; i < nFuncCount; i ++ )
	{
		FUNC_ENTRY *	pfe = m_wstaFunc.GetObjectAt( i ) ;
		if ( (pfe != NULL) && (pfe->dwAddress >= dwAddress) )
		{
			pfe->dwAddress += nShiftOffset ;
		}
	}
	//
	unsigned int	nBufLength = m_bufImage.GetLength() ;
	ESLAssert( dwAddress <= nBufLength ) ;
	if ( nShiftOffset > 0 )
	{
		m_bufImage.ResizeBuffer( nBufLength + nShiftOffset ) ;
		//
		BYTE *	ptrBuf =
			(BYTE*) m_bufImage.ModifyBuffer
				( dwAddress, nBufLength + nShiftOffset - dwAddress ) ;
		::eslMoveMemory
			( ptrBuf + nShiftOffset, ptrBuf, nBufLength - dwAddress ) ;
	}
	else
	{
		ESLAssert( (SDWORD) (dwAddress + nShiftOffset) >= 0 ) ;
		BYTE *	ptrBuf =
			(BYTE*) m_bufImage.ModifyBuffer
				( dwAddress + nShiftOffset,
						nBufLength - (dwAddress + nShiftOffset) ) ;
		::eslMoveMemory
			( ptrBuf, ptrBuf - nShiftOffset, nBufLength - dwAddress ) ;
		//
		m_bufImage.ResizeBuffer( nBufLength + nShiftOffset ) ;
	}
}

void ECSExecutionImageLinker::RemoveCodeReferenceAddressList
	( TaggedRefAddresList & listCodeRef, DWORD dwAddress, int nRange )
{
	const int	nCount = listCodeRef.GetSize() ;
	for ( int i = 0; i < nCount; i ++ )
	{
		ENumArray<DWORD> *	pList = listCodeRef.GetObjectAt( i ) ;
		ESLAssert( pList != NULL ) ;
		if ( pList != NULL )
		{
			RemoveCodeReferenceAddress( *pList, dwAddress, nRange ) ;
		}
	}
}

void ECSExecutionImageLinker::RemoveCodeReferenceAddress
	( ENumArray<DWORD> & extCodeRef, DWORD dwAddress, int nRange )
{
	int	nCount = extCodeRef.GetSize() ;
	for ( int i = 0; i < nCount; i ++ )
	{
		DWORD	dwRefAddr = extCodeRef.GetAt( i ) ;
		if ( (dwAddress <= dwRefAddr) && (dwRefAddr < dwAddress + nRange) )
		{
			extCodeRef.RemoveAt( i ) ;
			i -- ;
			nCount = extCodeRef.GetSize() ;
		}
	}
}

void ECSExecutionImageLinker::ShiftCodeReferenceAddressList
	( ECSExecutionImage::TaggedRefAddresList & listCodeRef,
								DWORD dwAddress, int nShiftOffset )
{
	const int	nCount = listCodeRef.GetSize() ;
	for ( int i = 0; i < nCount; i ++ )
	{
		ENumArray<DWORD> *	pList = listCodeRef.GetObjectAt( i ) ;
		ESLAssert( pList != NULL ) ;
		if ( pList != NULL )
		{
			ShiftCodeReferenceAddress( *pList, dwAddress, nShiftOffset ) ;
		}
	}
}

void ECSExecutionImageLinker::ShiftCodeReferenceAddress
	( ENumArray<DWORD> & extCodeRef, DWORD dwAddress, int nShiftOffset )
{
	const int	nCount = extCodeRef.GetSize() ;
	for ( int i = 0; i < nCount; i ++ )
	{
		DWORD	dwRefAddr = extCodeRef.GetAt( i ) ;
		if ( dwRefAddr >= dwAddress )
		{
			ESLAssert( (long int) dwRefAddr + nShiftOffset >= 0 ) ;
			extCodeRef.SetAt( i, dwRefAddr + nShiftOffset ) ;
		}
	}
}
