
/*****************************************************************************
                          Entis Graphic Library
 -----------------------------------------------------------------------------
       Copyright (c) 2003-2012 Leshade Entis, Entis-soft. Al rights reserved.
 *****************************************************************************/


#include <egl.h>
#include <math.h>

static const double	pi_rad = 3.141592653589 / 180.0 ;


//////////////////////////////////////////////////////////////////////////////
// テクスチャ画像ライブラリ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( E3DTextureLibrary, EPtrArray )

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
E3DTextureLibrary::~E3DTextureLibrary( void )
{
}

// テクスチャ画像を取得
//////////////////////////////////////////////////////////////////////////////
#if	defined(__PLATFORM_ANDROID__)
PEGL_IMAGE_INFO E3DTextureLibrary::GetTextureAs
	( const WORD * pwszName, bool fSearchParent ) const
#else
PEGL_IMAGE_INFO E3DTextureLibrary::GetTextureAs
	( const wchar_t * pwszName, bool fSearchParent ) const
#endif
{
	EGLImage *	pImage = EWStrTagArray<EGLImage>::GetAs( pwszName ) ;
	if ( (pImage != NULL) && (pImage->GetInfo() != NULL) )
	{
		return	pImage->GetInfo( ) ;
	}
	if ( fSearchParent && (m_pParent != NULL) )
	{
		return	m_pParent->GetTextureAs( pwszName, fSearchParent ) ;
	}
	return	NULL ;
}

// 画像ポインタからテクスチャ名を取得
//////////////////////////////////////////////////////////////////////////////
ESLError E3DTextureLibrary::GetTextureName
	( EWideString & wstrName,
		PEGL_IMAGE_INFO pTexture, bool fSearchParent ) const
{
	for ( int i = 0; i < (int) GetSize(); i ++ )
	{
		ETaggedElement<EWideString,EGLImage> *	pElement ;
		pElement = GetAt( i ) ;
		if ( pElement == NULL )
			continue ;
		EGLImage *	pImage = pElement->GetObject( ) ;
		if ( pImage == NULL )
			continue ;
		//
		if ( pImage->GetInfo() == pTexture )
		{
			wstrName = pElement->Tag( ) ;
			return	eslErrSuccess ;
		}
	}
	if ( fSearchParent && (m_pParent != NULL) )
	{
		return	m_pParent->GetTextureName( wstrName, pTexture, fSearchParent ) ;
	}
	return	eslErrGeneral ;
}


//////////////////////////////////////////////////////////////////////////////
// 表面属性ライブラリ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( E3DSurfaceLibrary, EPtrArray )

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
E3DSurfaceLibrary::~E3DSurfaceLibrary( void )
{
}

// 属性を取得する
//////////////////////////////////////////////////////////////////////////////
#if	defined(__PLATFORM_ANDROID__)
E3D_SURFACE_ATTRIBUTE * E3DSurfaceLibrary::GetAttributeAs
			( const WORD * pwszName, bool fSearchParent ) const
#else
E3D_SURFACE_ATTRIBUTE * E3DSurfaceLibrary::GetAttributeAs
			( const wchar_t * pwszName, bool fSearchParent ) const
#endif
{
	E3D_SURFACE_ATTRIBUTE *	pSufAttr =
		EWStrTagArray<E3D_SURFACE_ATTRIBUTE>::GetAs( pwszName ) ;
	if ( pSufAttr != NULL )
	{
		return	pSufAttr ;
	}
	if ( fSearchParent && (m_pParent != NULL) )
	{
		return	m_pParent->GetAttributeAs( pwszName, fSearchParent ) ;
	}
	return	NULL ;
}

// 属性ポインタから属性名を取得
//////////////////////////////////////////////////////////////////////////////
ESLError E3DSurfaceLibrary::GetAttributeName
	( EWideString & wstrName,
		E3D_SURFACE_ATTRIBUTE * pAttr, bool fSearchParent ) const
{
	for ( int i = 0; i < (int) GetSize(); i ++ )
	{
		ETaggedElement<EWideString,E3D_SURFACE_ATTRIBUTE> *	pElement ;
		pElement = GetAt( i ) ;
		if ( pElement == NULL )
			continue ;
		//
		if ( pElement->GetObject() == pAttr )
		{
			wstrName = pElement->Tag( ) ;
			return	eslErrSuccess ;
		}
	}
	if ( fSearchParent && (m_pParent != NULL) )
	{
		return	m_pParent->GetAttributeName( wstrName, pAttr, fSearchParent ) ;
	}
	return	eslErrGeneral ;
}


//////////////////////////////////////////////////////////////////////////////
// モデルオブジェクト
//////////////////////////////////////////////////////////////////////////////

// テクスチャ画像へのポインタを設定する
//////////////////////////////////////////////////////////////////////////////
inline ESLError eglNormalizeTextureAddress
	( PEGL_IMAGE_INFO & pTexture,
		E3DTextureLibrary & txlib, const EPtrBuffer & ptrbuf )
{
	if ( pTexture == NULL )
		return	eslErrSuccess ;
	//
	const BYTE *	pAttrData = (const BYTE *) ptrbuf.GetBuffer( ) ;
	DWORD		dwLength = ptrbuf.GetLength( ) ;
	ULONG_PTR	nAddress = (ULONG_PTR) pTexture ;
	if ( dwLength <= nAddress )
	{
		pTexture = NULL ;
		return	eslErrGeneral ;
	}
#if	defined(__PLATFORM_ANDROID__)
	pTexture =
		txlib.GetTextureAs( (const WORD *) (pAttrData + nAddress) ) ;
#else
	pTexture =
		txlib.GetTextureAs( (const wchar_t *) (pAttrData + nAddress) ) ;
#endif
	return	eslErrSuccess ;
}

// テクスチャ画像へのポインタを文字列へ変換する
//////////////////////////////////////////////////////////////////////////////
inline ESLError eglConvertTextureAddress
	( PEGL_IMAGE_INFO & pTexture, void * pBase,
		EStreamBuffer & buf, E3DTextureLibrary & txlib )
{
	EWideString	wstrName ;
	PEGL_IMAGE_INFO	pNameAddr = NULL ;
	if ( !txlib.GetTextureName( wstrName, pTexture ) )
	{
		pNameAddr = (PEGL_IMAGE_INFO) buf.GetLength( ) ;
		buf.Write
			( wstrName.CharPtr(),
				(wstrName.GetLength() + 1) * sizeof(WORD) ) ;
	}
	PEGL_IMAGE_INFO *	ppAddr =
		(PEGL_IMAGE_INFO*) buf.ModifyBuffer
			( ((DWORD) &pTexture - (DWORD) pBase), sizeof(PEGL_IMAGE_INFO) ) ;
	*(ppAddr) = pNameAddr ;
	return	(pTexture ? eslErrSuccess : eslErrGeneral) ;
}

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( E3DPolygonModel, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
E3DPolygonModel::E3DPolygonModel( void )
{
	m_nVertexCount = 0 ;
	m_pVertexes = NULL ;
	m_pVertexesBuf = NULL ;
	m_nNormalCount = 0 ;
	m_pNormals = NULL ;
	m_pNormalsBuf = NULL ;
	m_pConstantBuf = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
E3DPolygonModel::~E3DPolygonModel( void )
{
	DeleteContents( ) ;
}

// モデルデータを読み込む
//////////////////////////////////////////////////////////////////////////////
ESLError E3DPolygonModel::ReadModel( ESLFileObject & file )
{
	//
	// ファイルを開く
	//
	EMCFile	emcfile ;
	if ( emcfile.Open( &file ) )
	{
		return	ESLErrorMsg( "有効なモデルファイルではありません。" ) ;
	}
	DeleteContents( ) ;
	//
	for ( ; ; )
	{
		if ( emcfile.DescendRecord( ) )
		{
			break ;
		}
		ESLError	err ;
		UINT64	idRec = emcfile.GetRecordID( ) ;
		if ( idRec == *((UINT64*)"texture ") )
		{
			err = ReadTextureRecord( emcfile ) ;
		}
		else if ( idRec == *((UINT64*)"surface ") )
		{
			err = ReadSurfaceRecord( emcfile ) ;
		}
		else if ( idRec == *((UINT64*)"model   ") )
		{
			err = ReadModelRecord( emcfile ) ;
		}
		else
		{
			err = ReadUserRecord( emcfile, idRec ) ;
		}
		if ( err )
		{
			return	err ;
		}
		emcfile.AscendRecord( ) ;
	}
	//
	return	eslErrSuccess ;
}

// テクスチャデータを読み込む
//////////////////////////////////////////////////////////////////////////////
ESLError E3DPolygonModel::ReadTextureRecord( EMCFile & file )
{
	//
	// テクスチャ名テーブルを読み込む
	//
	EStreamBuffer	bufTxtName ;
	if ( file.DescendRecord( (UINT64*) "txt_name" ) )
	{
		#if	defined(__PLATFORM_ANDROID__)
			return	ESLErrorMsg( "not found texture name record" ) ;
		#else
			return	ESLErrorMsg( "テクスチャ名レコードが見つかりません。" ) ;
		#endif
	}
	//
	DWORD	dwLength = file.GetLength( ) ;
	bufTxtName.Flush
		( file.Read( bufTxtName.PutBuffer(dwLength), dwLength ) ) ;
	file.AscendRecord( ) ;
	//
	#if	defined(__PLATFORM_ANDROID__)
		static const char	szErrMsg[] = "invalid texture name record" ;
	#else
		static const char	szErrMsg[] = "テクスチャ名レコードが不正です。" ;
	#endif
	DWORD	i, dwEntryCount ;
	if ( bufTxtName.Read( &dwEntryCount, sizeof(DWORD) ) < sizeof(DWORD) )
	{
		return	ESLErrorMsg( szErrMsg ) ;
	}
	//
	// テクスチャ名リストを作成する
	//
	EObjArray<EWideString>	lstTxtName ;
	for ( i = 0; i < dwEntryCount; i ++ )
	{
		if ( bufTxtName.Read( &dwLength, sizeof(DWORD) ) < sizeof(DWORD) )
		{
			return	ESLErrorMsg( szErrMsg ) ;
		}
		//
		EPtrBuffer	ptrbuf =
			bufTxtName.GetBuffer( dwLength * sizeof(WORD) ) ;
		if ( ptrbuf.GetLength() < dwLength * sizeof(WORD) )
		{
			return	ESLErrorMsg( szErrMsg ) ;
		}
		//
		#if	defined(__PLATFORM_ANDROID__)
			lstTxtName.Add( new EWideString
				( (const WORD *) ptrbuf.GetBuffer(), dwLength ) ) ;
			bufTxtName.Release( dwLength * sizeof(WORD) ) ;
		#else
			lstTxtName.Add( new EWideString
				( (const wchar_t *) ptrbuf.GetBuffer(), dwLength ) ) ;
			bufTxtName.Release( dwLength * sizeof(wchar_t) ) ;
		#endif
	}
	//
	// テクスチャ画像を順次読み込む
	//
	for ( i = 0; i < dwEntryCount; i ++ )
	{
		if ( file.DescendRecord( (UINT64*) "txtimage" ) )
		{
			#if	defined(__PLATFORM_ANDROID__)
				return	ESLErrorMsg( "not found texture image record" ) ;
			#else
				return	ESLErrorMsg( "テクスチャ画像レコードが見つかりません。" ) ;
			#endif
		}
		//
		EGLImage *	pImage = new EGLImage ;
		if ( pImage->ReadImageFile( file ) )
		{
			delete	pImage ;
			#if	defined(__PLATFORM_ANDROID__)
				return	ESLErrorMsg( "failed to read texture image" ) ;
			#else
				return	ESLErrorMsg( "テクスチャ画像の読み込みに失敗しました。" ) ;
			#endif
		}
		file.AscendRecord( ) ;
		//
		m_txlib.Add( lstTxtName[i], pImage ) ;
	}
	//
	return	eslErrSuccess ;
}

// 表面属性データを読み込む
//////////////////////////////////////////////////////////////////////////////
ESLError E3DPolygonModel::ReadSurfaceRecord( EMCFile & file )
{
	//
	// 表面属性名テーブルを読み込む
	//
	EStreamBuffer	bufAttrName ;
	if ( file.DescendRecord( (UINT64*) "surfname" ) )
	{
		#if	defined(__PLATFORM_ANDROID__)
			return	ESLErrorMsg( "not found surface attribute name record" ) ;
		#else
			return	ESLErrorMsg( "表面属性名レコードが見つかりません。" ) ;
		#endif
	}
	//
	DWORD	dwLength = file.GetLength( ) ;
	bufAttrName.Flush
		( file.Read( bufAttrName.PutBuffer(dwLength), dwLength ) ) ;
	file.AscendRecord( ) ;
	//
	#if	defined(__PLATFORM_ANDROID__)
		static const char	szErrMsg[] = "invalid surface attribute name record" ;
	#else
		static const char	szErrMsg[] = "表面属性名レコードが不正です。" ;
	#endif
	DWORD	i, dwEntryCount ;
	if ( bufAttrName.Read( &dwEntryCount, sizeof(DWORD) ) < sizeof(DWORD) )
	{
		return	ESLErrorMsg( szErrMsg ) ;
	}
	//
	// 表面属性名リストを作成する
	//
	EObjArray<EWideString>	lstAttrName ;
	for ( i = 0; i < dwEntryCount; i ++ )
	{
		if ( bufAttrName.Read( &dwLength, sizeof(DWORD) ) < sizeof(DWORD) )
		{
			return	ESLErrorMsg( szErrMsg ) ;
		}
		//
		EPtrBuffer	ptrbuf =
			bufAttrName.GetBuffer( dwLength * sizeof(WORD) ) ;
		if ( ptrbuf.GetLength() < dwLength * sizeof(WORD) )
		{
			return	ESLErrorMsg( szErrMsg ) ;
		}
		//
		#if	defined(__PLATFORM_ANDROID__)
			lstAttrName.Add( new EWideString
				( (const WORD *) ptrbuf.GetBuffer(), dwLength ) ) ;
			bufAttrName.Release( dwLength * sizeof(WORD) ) ;
		#else
			lstAttrName.Add( new EWideString
				( (const wchar_t *) ptrbuf.GetBuffer(), dwLength ) ) ;
			bufAttrName.Release( dwLength * sizeof(wchar_t) ) ;
		#endif
	}
	//
	// 表面属性データを順次読み込む
	//
	for ( i = 0; i < dwEntryCount; i ++ )
	{
		//
		// 表面属性データを読み込む
		//
		if ( file.DescendRecord( (UINT64*) "surfattr" ) )
		{
			#if	defined(__PLATFORM_ANDROID__)
				return	ESLErrorMsg( "not found surface attribute data record" ) ;
			#else
				return	ESLErrorMsg( "表面属性データレコードが見つかりません。" ) ;
			#endif
		}
		//
		EStreamBuffer	bufSufAttr ;
		dwLength = file.GetLength( ) ;
		bufSufAttr.Flush
			( file.Read( bufSufAttr.PutBuffer(dwLength), dwLength ) ) ;
		file.AscendRecord( ) ;
		//
		#if	defined(__PLATFORM_ANDROID__)
			static const WORD	wchNull = L'\0' ;
			bufSufAttr.Write( &wchNull, sizeof(WORD) ) ;
		#else
			static const wchar_t	wchNull = L'\0' ;
			bufSufAttr.Write( &wchNull, sizeof(wchar_t) ) ;
		#endif
		EPtrBuffer	ptrbuf = bufSufAttr.GetBuffer( ) ;
		//
		// 表面属性データを複製する
		//
		E3D_SURFACE_ATTRIBUTE *	pSufAttr = new E3D_SURFACE_ATTRIBUTE ;
		DWORD	dwAddrLimit = __min((DWORD)dwLength,(DWORD)sizeof(E3D_SURFACE_ATTRIBUTE)) ;
		::eslFillMemory( pSufAttr, 0, sizeof(E3D_SURFACE_ATTRIBUTE) ) ;
		::eslMoveMemory( pSufAttr, ptrbuf, dwAddrLimit ) ;
		//
		// テクスチャ画像を設定する
		//
		if ( pSufAttr->dwShadingFlags & E3DSAF_ENVIRONMENT_MAP )
		{
			if ( pSufAttr->envmap.pUpperImage )
			{
				dwAddrLimit =
					__min( dwAddrLimit,
						(ULONG_PTR) pSufAttr->envmap.pUpperImage ) ;
				::eglNormalizeTextureAddress
					( pSufAttr->envmap.pUpperImage, m_txlib, ptrbuf ) ;
			}
			if ( pSufAttr->envmap.pUnderImage )
			{
				dwAddrLimit =
					__min( dwAddrLimit,
						(ULONG_PTR) pSufAttr->envmap.pUnderImage ) ;
				::eglNormalizeTextureAddress
					( pSufAttr->envmap.pUnderImage, m_txlib, ptrbuf ) ;
			}
		}
		else if ( pSufAttr->dwShadingFlags & E3DSAF_TEXTURE_MAPPING )
		{
			if ( pSufAttr->txmap.pTextureImage )
			{
				dwAddrLimit =
					__min( dwAddrLimit,
						(ULONG_PTR) pSufAttr->txmap.pTextureImage ) ;
				::eglNormalizeTextureAddress
					( pSufAttr->txmap.pTextureImage, m_txlib, ptrbuf ) ;
			}
			if ( pSufAttr->txmap.pLuminousImage )
			{
				dwAddrLimit =
					__min( dwAddrLimit,
						(ULONG_PTR) pSufAttr->txmap.pLuminousImage ) ;
				::eglNormalizeTextureAddress
					( pSufAttr->txmap.pLuminousImage, m_txlib, ptrbuf ) ;
			}
			if ( pSufAttr->txmap.pSmallImage )
			{
				dwAddrLimit =
					__min( dwAddrLimit,
						(ULONG_PTR) pSufAttr->txmap.pSmallImage ) ;
				::eglNormalizeTextureAddress
					( pSufAttr->txmap.pSmallImage, m_txlib, ptrbuf ) ;
			}
			if ( pSufAttr->txmap.pSmallLuminous )
			{
				dwAddrLimit =
					__min( dwAddrLimit,
						(ULONG_PTR) pSufAttr->txmap.pSmallLuminous ) ;
				::eglNormalizeTextureAddress
					( pSufAttr->txmap.pSmallLuminous, m_txlib, ptrbuf ) ;
			}
		}
		if ( (dwAddrLimit > 0)
			&& (dwAddrLimit < sizeof(E3D_SURFACE_ATTRIBUTE)) )
		{
			::eslFillMemory
				( ((BYTE*)pSufAttr) + dwAddrLimit,
					0, sizeof(E3D_SURFACE_ATTRIBUTE) - dwAddrLimit ) ;
		}
		//
		// 表面属性を追加
		//
		m_sflib.Add( lstAttrName[i], pSufAttr ) ;
	}
	//
	return	eslErrSuccess ;
}

// モデルデータを読み込む
//////////////////////////////////////////////////////////////////////////////
ESLError E3DPolygonModel::ReadModelRecord( EMCFile & file )
{
	//
	// 頂点テーブルレコードを読み込む
	//
	if ( file.DescendRecord( (UINT64*) "vertexes" ) )
	{
		#if	defined(__PLATFORM_ANDROID__)
			return	ESLErrorMsg( "not found vertices record" ) ;
		#else
			return	ESLErrorMsg( "頂点テーブルレコードが見つかりません。" ) ;
		#endif
	}
	DWORD	dwLength ;
	dwLength = file.GetLength() / sizeof(E3D_VECTOR4) ;
	AllocateVertexBuffer( dwLength ) ;
	file.Read( m_pVertexesBuf, dwLength * sizeof(E3D_VECTOR4) ) ;
	file.AscendRecord( ) ;
	//
	CommitVertexBuffer( 0, dwLength ) ;
	//
	// 法線テーブルレコードを読み込む
	//
	if ( file.DescendRecord( (UINT64*) "normals " ) )
	{
		#if	defined(__PLATFORM_ANDROID__)
			return	ESLErrorMsg( "not found normal record" ) ;
		#else
			return	ESLErrorMsg( "法線テーブルレコードが見つかりません。" ) ;
		#endif
	}
	dwLength = file.GetLength() / sizeof(E3D_VECTOR4) ;
	AllocateNormalBuffer( dwLength ) ;
	file.Read( m_pNormalsBuf, dwLength * sizeof(E3D_VECTOR4) ) ;
	file.AscendRecord( ) ;
	//
	CommitNormalBuffer( 0, dwLength ) ;
	//
	// プリミティブレコードを読み込む
	//
	if ( file.DescendRecord( (UINT64*) "primitiv" ) )
	{
		#if	defined(__PLATFORM_ANDROID__)
			return	ESLErrorMsg( "not found primitive data record" ) ;
		#else
			return	ESLErrorMsg( "プリミティブデータレコードが見つかりません。" ) ;
		#endif
	}
	#if	defined(__PLATFORM_ANDROID__)
	static const char	szErrMsg[] = "invalid primitive data record" ;
	#else
	static const char	szErrMsg[] = "プリミティブデータレコードが不正です。" ;
	#endif
	DWORD	i, dwEntryCount ;
	if ( file.Read( &dwEntryCount, sizeof(DWORD) ) < sizeof(DWORD) )
	{
		return	ESLErrorMsg( szErrMsg ) ;
	}
	for ( i = 0; i < dwEntryCount; i ++ )
	{
		if ( file.Read( &dwLength, sizeof(DWORD) ) < sizeof(DWORD) )
		{
			return	ESLErrorMsg( szErrMsg ) ;
		}
		//
		EStreamBuffer	buf ;
		BYTE *	ptrBuf = (BYTE*) buf.PutBuffer( dwLength ) ;
		file.Read( ptrBuf, dwLength ) ;
		//
		// データを複製
		//
		DWORD	dwBytes ;
		E3D_PRIMITIVE_POLYGON *
			pPrimitive = (E3D_PRIMITIVE_POLYGON*) ptrBuf ;
		dwBytes = CalcPrimitiveDataSize( pPrimitive ) ;
		if ( dwBytes > dwLength )
		{
			return	ESLErrorMsg( szErrMsg ) ;
		}
		pPrimitive =
			(E3D_PRIMITIVE_POLYGON*)
				::eslHeapAllocate( NULL, dwBytes, 0 ) ;
		::eslMoveMemory( pPrimitive, ptrBuf, dwBytes ) ;
		//
		if ( ConvertPrimitiveIndexToAddress( pPrimitive ) )
		{
			return	ESLErrorMsg( szErrMsg ) ;
		}
		if ( pPrimitive->dwTypeFlag == E3D_IMAGE_PRIMITIVE )
		{
			//
			// 画像プリミティブの画像設定
			//
			::eglNormalizeTextureAddress
				( pPrimitive->image.pImageInf,
					m_txlib, EPtrBuffer(ptrBuf,dwLength) ) ;
		}
		//
		// 表面属性を設定する
		//
		DWORD	dwAddress = (DWORD) pPrimitive->pSurfaceAttr ;
		if ( dwAddress > dwLength )
		{
			return	ESLErrorMsg( szErrMsg ) ;
		}
		buf.Flush( dwLength ) ;
		static const WORD	wchNull = L'\0' ;
		buf.Write( &wchNull, sizeof(WORD) ) ;
		EPtrBuffer	ptrbuf = buf.GetBuffer( ) ;
		ptrBuf = (BYTE*) ptrbuf.GetBuffer( ) ;
		//
		#if	defined(__PLATFORM_ANDROID__)
			pPrimitive->pSurfaceAttr =
				m_sflib.GetAttributeAs( (const WORD *) (ptrBuf + dwAddress) ) ;
		#else
			pPrimitive->pSurfaceAttr =
				m_sflib.GetAttributeAs( (const wchar_t *) (ptrBuf + dwAddress) ) ;
		#endif
		if ( pPrimitive->pSurfaceAttr == NULL )
		{
			return	ESLErrorMsg( szErrMsg ) ;
		}
		//
		// 表面属性の追加
		//
		#if	defined(__PLATFORM_ANDROID__)
			NormalizePrimitiveFace( pPrimitive ) ;
		#endif
		//
		m_lstPrimitives.Add( pPrimitive ) ;
	}
	file.AscendRecord( ) ;
	//
	return	eslErrSuccess ;
}

// ユーザー定義のレコードを読み込む
//////////////////////////////////////////////////////////////////////////////
ESLError E3DPolygonModel::ReadUserRecord( EMCFile & file, UINT64 idRec )
{
	return	eslErrSuccess ;
}

#if	defined(__PLATFORM_ANDROID__)

// ポリゴンの表裏正規化
//////////////////////////////////////////////////////////////////////////////
void E3DPolygonModel::NormalizePrimitiveFace( E3D_PRIMITIVE_POLYGON * pPrimitive )
{
	const DWORD	dwTypeFlag = pPrimitive->dwTypeFlag ;
	if ( dwTypeFlag & E3D_MESH_POLYGON )
	{
		NormalizeMeshFace( pPrimitive ) ;
	}
	else if ( !(dwTypeFlag & ~E3D_POLYGON_PRIMITIVE_MASK) )
	{
		NormalizePolygonFace( pPrimitive ) ;
	}
	else if ( dwTypeFlag == E3D_INFINITE_PLANE )
	{
	}
	else if ( dwTypeFlag == E3D_IMAGE_PRIMITIVE )
	{
	}
}

void E3DPolygonModel::NormalizeMeshFace( E3D_PRIMITIVE_POLYGON * pMesh )
{
	const long int	nVertexCount = pMesh->dwVertexCount ;
	E3D_PRIMITIVE_MESH_LIST *
			ppmlMesh = (E3D_PRIMITIVE_MESH_LIST*)
							&(pMesh->mesh.uv_map[nVertexCount]) ;
	const long int	nPolyCount = ppmlMesh->dwPolyCount ;
	PCE3D_VECTOR4	vertices = pMesh->mesh.vertices ;
	PCE3D_VECTOR4	normals = pMesh->mesh.normals ;
	//
	if ( !(pMesh->dwTypeFlag & E3D_SMOOTH_POLYGON) || (normals == NULL) )
	{
		return ;
	}
	E3D_PRIMITIVE_MESH_POLY *	ppmpMesh = ppmlMesh->mpEntries ;
	for ( int i = 0; i < nPolyCount; i ++ )
	{
		const long int	nPolyVertexCount = ppmpMesh->dwVertexCount ;
		if ( nPolyVertexCount >= 3 )
		{
			//
			// 法線計算
			//
			int	j ;
			E3DVector	vNormal = normals[ppmpMesh->dwIndex[0]] ;
			for ( j = 1; j < nPolyVertexCount; j ++ )
			{
				vNormal += E3DVector( normals[ppmpMesh->dwIndex[j]] ) ;
			}
			E3DVector	v0 = vertices[ppmpMesh->dwIndex[0]] ;
			E3DVector	v1 = vertices[ppmpMesh->dwIndex[1]] ;
			E3DVector	v2 = vertices[ppmpMesh->dwIndex[2]] ;
			E3DVector	vPlane = ((v1 - v0) * (v2 - v0)) ;
			//
			if ( (vPlane | vNormal) < 0.0f )
			{
				//
				// 反転
				//
				int	k ;
				for ( j = 1, k = nPolyVertexCount - 1; j < k; j ++, k -- )
				{
					DWORD	dwIndex = ppmpMesh->dwIndex[j] ;
					ppmpMesh->dwIndex[j] = ppmpMesh->dwIndex[k] ;
					ppmpMesh->dwIndex[k] = dwIndex ;
				}
			}
		}
		//
		// 次のポリゴン
		//
		ppmpMesh = (E3D_PRIMITIVE_MESH_POLY*)
						&(ppmpMesh->dwIndex[nPolyVertexCount]) ;
	}
}

void E3DPolygonModel::NormalizePolygonFace( E3D_PRIMITIVE_POLYGON * pPoly )
{
	const long int	nVertexCount = pPoly->dwVertexCount ;
	if ( nVertexCount < 3 )
	{
		return ;
	}
	//
	// 法線を計算
	//
	if ( !(pPoly->dwTypeFlag & E3D_SMOOTH_POLYGON) )
	{
		return ;
	}
	E3DVector	vNormal( 0, 0, 0 ) ;
	E3DVector	vVertex[3] ;
	int	i ;
	for ( i = 0; i < nVertexCount; i ++ )
	{
		if ( i < 3 )
		{
			if ( pPoly->polygon[i].vertex == NULL )
			{
				return ;
			}
			vVertex[i] = *(pPoly->polygon[i].vertex) ;
		}
		if ( pPoly->polygon[i].normal == NULL )
		{
			return ;
		}
		vNormal += E3DVector( *(pPoly->polygon[i].normal) ) ;
	}
	E3DVector	vPlane =
		((vVertex[1] - vVertex[0]) * (vVertex[2] - vVertex[0])) ;
	//
	if ( (vPlane | vNormal) < 0.0f )
	{
		//
		// 反転
		//
		int	j ;
		for ( i = 1, j = nVertexCount - 1; i < j; i ++, j -- )
		{
			E3D_PRIMITIVE_VERTEX	pvtx = pPoly->polygon[i] ;
			pPoly->polygon[i] = pPoly->polygon[j] ;
			pPoly->polygon[j] = pvtx ;
		}
	}
}

#else


// モデルデータを書き出す
//////////////////////////////////////////////////////////////////////////////
ESLError E3DPolygonModel::WriteModel( ESLFileObject & file )
{
	//
	// EMC ファイルヘッダを書き出す
	//
	EMCFile					emcfile ;
	EMCFile::FILE_HEADER	fhdr ;
	emcfile.SetFileHeader( fhdr, emcfile.fidEGL3DModel, NULL ) ;
	if ( emcfile.Open( &file, &fhdr ) )
	{
		return	ESLErrorMsg( "EMC ファイルヘッダの書き出しに失敗しました。" ) ;
	}
	//
	// テクスチャ画像を書き出す
	//
	if ( emcfile.DescendRecord( (UINT64*) "texture " ) )
	{
		return	ESLErrorMsg
			( "テクスチャレコードヘッダの書き出しに失敗しました。" ) ;
	}
	ESLError	err ;
	err = WriteTextureRecord( emcfile ) ;
	if ( err )
	{
		return	err ;
	}
	emcfile.AscendRecord( ) ;
	//
	// 表面属性を書き出す
	//
	if ( emcfile.DescendRecord( (UINT64*) "surface " ) )
	{
		return	ESLErrorMsg
			( "表面属性レコードヘッダの書き出しに失敗しました。" ) ;
	}
	err = WriteSurfaceRecord( emcfile ) ;
	if ( err )
	{
		return	err ;
	}
	emcfile.AscendRecord( ) ;
	//
	// モデルデータを書き出す
	//
	if ( emcfile.DescendRecord( (UINT64*) "model   " ) )
	{
		return	ESLErrorMsg
			( "モデルデータレコードヘッダの書き出しに失敗しました。" ) ;
	}
	err = WriteModelRecord( emcfile ) ;
	if ( err )
	{
		return	err ;
	}
	emcfile.AscendRecord( ) ;
	//
	// ユーザー定義のレコードを書き出す
	//
	err = WriteUserRecord( emcfile ) ;
	emcfile.Close( ) ;
	//
	return	err ;
}

// テクスチャデータを書き出す
//////////////////////////////////////////////////////////////////////////////
ESLError E3DPolygonModel::WriteTextureRecord( EMCFile & file )
{
	//
	// テクスチャ名テーブルレコードを書き出す
	//
	if ( file.DescendRecord( (UINT64*) "txt_name" ) )
	{
		return	ESLErrorMsg( "テクスチャ名レコードの書き出しに失敗しました。" ) ;
	}
	//
	DWORD	i, dwEntryCount = m_txlib.GetSize( ) ;
	file.Write( &dwEntryCount, sizeof(DWORD) ) ;
	//
	for ( i = 0; i < dwEntryCount; i ++ )
	{
		ETaggedElement<EWideString,EGLImage> *	pElement ;
		pElement = m_txlib.GetAt( i ) ;
		ESLAssert( pElement != NULL ) ;
		//
		DWORD	dwLength = pElement->Tag().GetLength( ) ;
		file.Write( &dwLength, sizeof(DWORD) ) ;
		file.Write( pElement->Tag().CharPtr(), dwLength * sizeof(wchar_t) ) ;
	}
	//
	file.AscendRecord( ) ;
	//
	// テクスチャ画像を順次書き出す
	//
	for ( i = 0; i < dwEntryCount; i ++ )
	{
		ETaggedElement<EWideString,EGLImage> *	pElement ;
		pElement = m_txlib.GetAt( i ) ;
		ESLAssert( pElement != NULL ) ;
		EGLImage *	pImage = pElement->GetObject( ) ;
		ESLAssert( pImage != NULL ) ;
		//
		if ( file.DescendRecord( (UINT64*) "txtimage" ) )
		{
			return	ESLErrorMsg
				( "テクスチャ画像レコードの書き出しに失敗しました。" ) ;
		}
		if ( pImage->WriteImageFile
			( file, EGLImage::ctfCompatibleFormat,
					ERISAEncoder::efBestCmpr, NULL ) )
		{
			return	ESLErrorMsg
				( "テクスチャ画像データの書き出しに失敗しました。" ) ;
		}
		file.AscendRecord( ) ;
	}
	//
	return	eslErrSuccess ;
}

// 表面属性データを書き出す
//////////////////////////////////////////////////////////////////////////////
ESLError E3DPolygonModel::WriteSurfaceRecord( EMCFile & file )
{
	//
	// 表面属性名テーブルを書き出す
	//
	if ( file.DescendRecord( (UINT64*) "surfname" ) )
	{
		return	ESLErrorMsg( "表面属性名レコードの書き出しに失敗しました。" ) ;
	}
	//
	DWORD	i, dwEntryCount ;
	ETaggedElement<EWideString,E3D_SURFACE_ATTRIBUTE> *	pElement ;
	dwEntryCount = m_sflib.GetSize( ) ;
	file.Write( &dwEntryCount, sizeof(DWORD) ) ;
	//
	for ( i = 0; i < dwEntryCount; i ++ )
	{
		pElement = m_sflib.GetAt( i ) ;
		ESLAssert( pElement != NULL ) ;
		//
		DWORD	dwLength = pElement->Tag().GetLength( ) ;
		file.Write( &dwLength, sizeof(DWORD) ) ;
		file.Write( pElement->Tag().CharPtr(), dwLength * sizeof(wchar_t) ) ;
	}
	//
	file.AscendRecord( ) ;
	//
	// 表面属性データを順次書き出す
	//
	for ( i = 0; i < dwEntryCount; i ++ )
	{
		pElement = m_sflib.GetAt( i ) ;
		ESLAssert( pElement != NULL ) ;
		E3D_SURFACE_ATTRIBUTE *	pAttr = pElement->GetObject( ) ;
		ESLAssert( pAttr != NULL ) ;
		//
		EStreamBuffer	buf ;
		buf.Write( pAttr, sizeof(E3D_SURFACE_ATTRIBUTE) ) ;
		//
		if ( pAttr->dwShadingFlags & E3DSAF_ENVIRONMENT_MAP )
		{
			::eglConvertTextureAddress
				( pAttr->envmap.pUpperImage, pAttr, buf, m_txlib ) ;
			::eglConvertTextureAddress
				( pAttr->envmap.pUnderImage, pAttr, buf, m_txlib ) ;
		}
		else if ( pAttr->dwShadingFlags & E3DSAF_TEXTURE_MAPPING )
		{
			::eglConvertTextureAddress
				( pAttr->txmap.pTextureImage, pAttr, buf, m_txlib ) ;
			::eglConvertTextureAddress
				( pAttr->txmap.pLuminousImage, pAttr, buf, m_txlib ) ;
			::eglConvertTextureAddress
				( pAttr->txmap.pSmallImage, pAttr, buf, m_txlib ) ;
			::eglConvertTextureAddress
				( pAttr->txmap.pSmallLuminous, pAttr, buf, m_txlib ) ;
		}
		//
		if ( file.DescendRecord( (UINT64*) "surfattr" ) )
		{
			return	ESLErrorMsg( "表面属性データレコードの書き出しに失敗しました。" ) ;
		}
		//
		EPtrBuffer	ptrbuf = buf.GetBuffer( ) ;
		if ( file.Write
			( ptrbuf, ptrbuf.GetLength() ) < ptrbuf.GetLength() )
		{
			return	ESLErrorMsg( "表面属性データの書き出しに失敗しました。" ) ;
		}
		//
		file.AscendRecord( ) ;
	}
	//
	return	eslErrSuccess ;
}

// モデルデータを書き出す
//////////////////////////////////////////////////////////////////////////////
ESLError E3DPolygonModel::WriteModelRecord( EMCFile & file )
{
	//
	// 頂点テーブルレコードを書き出す
	//
	if ( file.DescendRecord( (UINT64*) "vertexes" ) )
	{
		return	ESLErrorMsg
			( "頂点テーブルレコードの書き出しに失敗しました。" ) ;
	}
	file.Write( m_pVertexes, m_nVertexCount * sizeof(E3D_VECTOR4) ) ;
	file.AscendRecord( ) ;
	//
	// 法線テーブルレコードを書き出す
	//
	if ( file.DescendRecord( (UINT64*) "normals " ) )
	{
		return	ESLErrorMsg
			( "法線テーブルレコードの書き出しに失敗しました。" ) ;
	}
	file.Write( m_pNormals, m_nNormalCount * sizeof(E3D_VECTOR4) ) ;
	file.AscendRecord( ) ;
	//
	// プリミティブレコードを書き出す
	//
	if ( file.DescendRecord( (UINT64*) "primitiv" ) )
	{
		return	ESLErrorMsg
			( "プリミティブレコードの書き出しに失敗しました。" ) ;
	}
	//
	DWORD	i, dwEntryCount = m_lstPrimitives.GetSize( ) ;
	file.Write( &dwEntryCount, sizeof(DWORD) ) ;
	//
	for ( i = 0; i < dwEntryCount; i ++ )
	{
		E3D_PRIMITIVE_POLYGON *	pPrimitive = m_lstPrimitives.GetAt( i ) ;
		ESLAssert( pPrimitive != NULL ) ;
		//
		EStreamBuffer	buf ;
		DWORD			dwBytes = CalcPrimitiveDataSize( pPrimitive ) ;
		E3D_PRIMITIVE_POLYGON *	pBuf =
			(E3D_PRIMITIVE_POLYGON*) buf.PutBuffer( dwBytes ) ;
		::eslMoveMemory( pBuf, pPrimitive, dwBytes ) ;
		//
		if ( ConvertPrimitiveAddressToIndex( pBuf ) )
		{
			return	ESLErrorMsg( "定義されていないプリミティブです。" ) ;
		}
		buf.Flush( dwBytes ) ;
		//
		DWORD	dwTypeFlag = pPrimitive->dwTypeFlag ;
		if ( dwTypeFlag == E3D_IMAGE_PRIMITIVE )
		{
			//
			// 画像プリミティブ
			//
			if ( pPrimitive->image.pImageInf != NULL )
			{
				::eglConvertTextureAddress
					( pPrimitive->image.pImageInf, pPrimitive, buf, m_txlib ) ;
			}
		}
		//
		// 表面属性を設定する
		//
		EWideString	wstrName ;
		if ( m_sflib.GetAttributeName( wstrName, pPrimitive->pSurfaceAttr ) )
		{
			return	ESLErrorMsg( "表面属性が見つかりません。" ) ;
		}
		PE3D_SURFACE_ATTRIBUTE	pSufAttr =
			(PE3D_SURFACE_ATTRIBUTE) buf.GetLength( ) ;
		buf.Write
			( wstrName.CharPtr(),
				(wstrName.GetLength() + 1) * sizeof(wchar_t) ) ;
		((E3D_PRIMITIVE_POLYGON*)
			buf.ModifyBuffer(0,dwBytes))->pSurfaceAttr = pSufAttr ;
		//
		// 書き出す
		//
		EPtrBuffer	ptrbuf = buf.GetBuffer( ) ;
		DWORD	dwLength = ptrbuf.GetLength( ) ;
		file.Write( &dwLength, sizeof(DWORD) ) ;
		if ( file.Write( ptrbuf, dwLength ) < dwLength )
		{
			return	ESLErrorMsg
				( "プリミティブデータの書き出しに失敗しました。" ) ;
		}
	}
	//
	file.AscendRecord( ) ;
	//
	return	eslErrSuccess ;
}

// ユーザー定義のレコードを書き出す
//////////////////////////////////////////////////////////////////////////////
ESLError E3DPolygonModel::WriteUserRecord( EMCFile & file )
{
	return	eslErrSuccess ;
}

#endif

// モデルデータを削除する
//////////////////////////////////////////////////////////////////////////////
void E3DPolygonModel::DeleteContents( void )
{
	RemoveAllPrimitive( ) ;
	m_txlib.RemoveAll( ) ;
	m_sflib.RemoveAll( ) ;
	//
	if ( m_pVertexes != NULL )
	{
		::eslHeapFree( NULL, m_pVertexes ) ;
		m_pVertexes = NULL ;
	}
	if ( m_pVertexesBuf != NULL )
	{
		::eslHeapFree( NULL, m_pVertexesBuf ) ;
		m_pVertexesBuf = NULL ;
	}
	if ( m_pNormals != NULL )
	{
		::eslHeapFree( NULL, m_pNormals ) ;
		m_pNormals = NULL ;
	}
	if ( m_pNormalsBuf != NULL )
	{
		::eslHeapFree( NULL, m_pNormalsBuf ) ;
		m_pNormalsBuf = NULL ;
	}
	if ( m_pConstantBuf != NULL )
	{
		::eslHeapFree( NULL, m_pConstantBuf ) ;
		m_pConstantBuf = NULL ;
	}
	m_nVertexCount = 0 ;
	m_nNormalCount = 0 ;
}

// 頂点バッファを確保する
//////////////////////////////////////////////////////////////////////////////
void E3DPolygonModel::AllocateVertexBuffer( unsigned int nCount )
{
	DWORD	dwBytes = nCount * sizeof(E3D_VECTOR4) ;
	m_nVertexCount = nCount ;
	//
	m_pVertexes =
		(PE3D_VECTOR4) ::eslHeapReallocate
			( NULL, m_pVertexes, dwBytes, ESL_HEAP_ZERO_INIT ) ;
	m_pVertexesBuf =
		(PE3D_VECTOR4) ::eslHeapReallocate
			( NULL, m_pVertexesBuf, dwBytes, ESL_HEAP_ZERO_INIT ) ;
}

// 法線バッファを確保する
//////////////////////////////////////////////////////////////////////////////
void E3DPolygonModel::AllocateNormalBuffer( unsigned int nCount )
{
	DWORD	dwBytes = nCount * sizeof(E3D_VECTOR4) ;
	m_nNormalCount = nCount ;
	//
	m_pNormals =
		(PE3D_VECTOR4) ::eslHeapReallocate
			( NULL, m_pNormals, dwBytes, ESL_HEAP_ZERO_INIT ) ;
	m_pNormalsBuf =
		(PE3D_VECTOR4) ::eslHeapReallocate
			( NULL, m_pNormalsBuf, dwBytes, ESL_HEAP_ZERO_INIT ) ;
}

// 定数バッファを確保する
//////////////////////////////////////////////////////////////////////////////
void E3DPolygonModel::AllocateConstantBuffer( unsigned int nCount )
{
	DWORD	dwBytes = nCount * sizeof(E3D_VECTOR4) ;
	m_pConstantBuf =
		(PE3D_VECTOR4) ::eslHeapReallocate
			( NULL, m_pConstantBuf, dwBytes, ESL_HEAP_ZERO_INIT ) ;
}

// プリミティブデータ用バッファの再アロケート
//////////////////////////////////////////////////////////////////////////////
E3D_PRIMITIVE_POLYGON *
	E3DPolygonModel::ReallocatePrimitiveAt
		( unsigned int nIndex, unsigned int nBufSize )
{
	E3D_PRIMITIVE_POLYGON *
		pPrimitive = m_lstPrimitives.GetAt( nIndex ) ;
	if ( pPrimitive == NULL )
	{
		pPrimitive = (E3D_PRIMITIVE_POLYGON*)
				::eslHeapAllocate( NULL, nBufSize, 0 ) ;
	}
	else
	{
		pPrimitive = (E3D_PRIMITIVE_POLYGON*)
			::eslHeapReallocate( NULL, pPrimitive, nBufSize, 0 ) ;
	}
	m_lstPrimitives.SetAt( nIndex, pPrimitive ) ;
	return	pPrimitive ;
}

// ポリゴンプリミティブを追加
//////////////////////////////////////////////////////////////////////////////
void E3DPolygonModel::AddPolygon
	( DWORD dwTypeFlag, PE3D_SURFACE_ATTRIBUTE pSurfAttr,
		DWORD dwVertexCount, const E3D_PRIMITIVE_VERTEX * pVertexes )
{
	DWORD	dwBytes = 0x10 + dwVertexCount * sizeof(E3D_PRIMITIVE_VERTEX) ;
	E3D_PRIMITIVE_POLYGON *	pPrimitive =
		(E3D_PRIMITIVE_POLYGON*) ::eslHeapAllocate( NULL, dwBytes, 0 ) ;
	//
	pPrimitive->dwTypeFlag = dwTypeFlag ;
	pPrimitive->pSurfaceAttr = pSurfAttr ;
	pPrimitive->dwVertexCount = dwVertexCount ;
	pPrimitive->dwReserved = 0 ;
	//
	::eslMoveMemory
		( pPrimitive->polygon, pVertexes,
			dwVertexCount * sizeof(E3D_PRIMITIVE_VERTEX) ) ;
	//
	m_lstPrimitives.Add( pPrimitive ) ;
}

// プリミティブを追加
//////////////////////////////////////////////////////////////////////////////
void E3DPolygonModel::AddPrimitives
	( DWORD dwPrimitiveCount, const PE3D_PRIMITIVE_POLYGON * ppPrimitives )
{
	for ( DWORD i = 0; i < dwPrimitiveCount; i ++ )
	{
		DWORD	dwBytes = CalcPrimitiveDataSize( ppPrimitives[i] ) ;
		PE3D_PRIMITIVE_POLYGON	pNewPrim
			= (PE3D_PRIMITIVE_POLYGON) ::eslHeapAllocate( NULL, dwBytes, 0 ) ;
		::eslMoveMemory( pNewPrim, ppPrimitives[i], dwBytes ) ;
		m_lstPrimitives.Add( pNewPrim ) ;
	}
}

// プリミティブを削除
//////////////////////////////////////////////////////////////////////////////
void E3DPolygonModel::RemovePrimitives
	( unsigned int nFirst, unsigned int nCount )
{
	for ( unsigned int i = 0; i < nCount; i ++ )
	{
		if ( i + nFirst >= m_lstPrimitives.GetSize() )
			break ;
		//
		E3D_PRIMITIVE_POLYGON *	pPrimitive = m_lstPrimitives.GetAt( i + nFirst ) ;
		if ( pPrimitive != NULL )
		{
			::eslHeapFree( NULL, pPrimitive ) ;
		}
	}
	m_lstPrimitives.RemoveBetween( nFirst, nCount ) ;
}

// プリミティブを全て削除
//////////////////////////////////////////////////////////////////////////////
void E3DPolygonModel::RemoveAllPrimitive( void )
{
	for ( int i = 0; i < (int) m_lstPrimitives.GetSize(); i ++ )
	{
		E3D_PRIMITIVE_POLYGON *	pPrimitive = m_lstPrimitives.GetAt( i ) ;
		if ( pPrimitive != NULL )
		{
			::eslHeapFree( NULL, pPrimitive ) ;
		}
	}
	m_lstPrimitives.RemoveAll( ) ;
}

// 全プリミティブの頂点インデックスをアドレスに変換する
//////////////////////////////////////////////////////////////////////////////
ESLError E3DPolygonModel::ConvertAllPrimitiveIndexToAddress( void )
{
	unsigned int	nCount = m_lstPrimitives.GetSize() ;
	for ( unsigned int i = 0; i < nCount; i ++ )
	{
		E3D_PRIMITIVE_POLYGON *	pPrimitive = m_lstPrimitives.GetAt( i ) ;
		if ( pPrimitive != NULL )
		{
			ConvertPrimitiveIndexToAddress( pPrimitive ) ;
		}
	}
	return	eslErrSuccess ;
}

// プリミティブの頂点インデックスをアドレスに変換する
//////////////////////////////////////////////////////////////////////////////
ESLError E3DPolygonModel::ConvertPrimitiveIndexToAddress
		( E3D_PRIMITIVE_POLYGON * pPrimitive )
{
	const DWORD	dwTypeFlag = pPrimitive->dwTypeFlag ;
	if ( dwTypeFlag & E3D_MESH_POLYGON )
	{
		//
		// メッシュ
		//
		pPrimitive->mesh.vertices =
			m_pVertexesBuf + pPrimitive->mesh.i_vertices ;
		if ( dwTypeFlag & E3D_SMOOTH_POLYGON )
		{
			pPrimitive->mesh.normals =
				m_pNormalsBuf + pPrimitive->mesh.i_normals ;
		}
		else
		{
			pPrimitive->mesh.normals = NULL ;
		}
	}
	else if ( !(dwTypeFlag & ~E3D_POLYGON_PRIMITIVE_MASK) )
	{
		//
		// ポリゴンプリミティブ
		//
		const DWORD	dwVertexCount = pPrimitive->dwVertexCount ;
		for ( DWORD j = 0; j < dwVertexCount; j ++ )
		{
			pPrimitive->polygon[j].vertex =
				m_pVertexesBuf + pPrimitive->polygon[j].i_vertex ;
			if ( dwTypeFlag & E3D_SMOOTH_POLYGON )
			{
				pPrimitive->polygon[j].normal =
					m_pNormalsBuf + pPrimitive->polygon[j].i_normal ;
			}
		}
	}
	else if ( dwTypeFlag == E3D_INFINITE_PLANE )
	{
		//
		// 無限平面プリミティブ
		//
		pPrimitive->infinite_plane.vertex =
			m_pVertexesBuf + pPrimitive->infinite_plane.i_vertex ;
		pPrimitive->infinite_plane.xAxis =
			m_pNormalsBuf + pPrimitive->infinite_plane.i_xAxis ;
		pPrimitive->infinite_plane.yAxis =
			m_pNormalsBuf + pPrimitive->infinite_plane.i_yAxis ;
	}
	else if ( dwTypeFlag == E3D_IMAGE_PRIMITIVE )
	{
		//
		// 画像プリミティブ
		//
		pPrimitive->image.vCenter =
			m_pVertexesBuf + pPrimitive->image.i_vCenter ;
		pPrimitive->image.vEnlarge =
			m_pNormalsBuf + pPrimitive->image.i_vEnlarge ;
	}
	else
	{
		return	eslErrFailed ;
	}
	return	eslErrSuccess ;
}

// 全プリミティブの頂点アドレスをインデックスに変換する
//////////////////////////////////////////////////////////////////////////////
ESLError E3DPolygonModel::ConvertAllPrimitiveAddressToIndex( void )
{
	unsigned int	nCount = m_lstPrimitives.GetSize() ;
	for ( unsigned int i = 0; i < nCount; i ++ )
	{
		E3D_PRIMITIVE_POLYGON *	pPrimitive = m_lstPrimitives.GetAt( i ) ;
		if ( pPrimitive != NULL )
		{
			ConvertPrimitiveAddressToIndex( pPrimitive ) ;
		}
	}
	return	eslErrSuccess ;
}

// プリミティブの頂点アドレスをインデックスに変換する
//////////////////////////////////////////////////////////////////////////////
ESLError E3DPolygonModel::ConvertPrimitiveAddressToIndex
		( E3D_PRIMITIVE_POLYGON * pPrimitive )
{
	DWORD	dwTypeFlag = pPrimitive->dwTypeFlag ;
	if ( dwTypeFlag & E3D_MESH_POLYGON )
	{
		//
		// メッシュ
		//
		pPrimitive->mesh.vertices = (PE3D_VECTOR4)
			((pPrimitive->mesh.i_vertices
				- (INT_PTR) m_pVertexesBuf) / sizeof(E3D_VECTOR4)) ;
		if ( dwTypeFlag & E3D_SMOOTH_POLYGON )
		{
			pPrimitive->mesh.normals = (PE3D_VECTOR4)
				((pPrimitive->mesh.i_normals
					- (INT_PTR) m_pNormalsBuf) / sizeof(E3D_VECTOR4)) ;
		}
	}
	else if ( !(dwTypeFlag & ~E3D_POLYGON_PRIMITIVE_MASK) )
	{
		//
		// ポリゴンプリミティブ
		//
		DWORD	dwVertexCount = pPrimitive->dwVertexCount ;
		for ( DWORD j = 0; j < dwVertexCount; j ++ )
		{
			pPrimitive->polygon[j].vertex = (PE3D_VECTOR4)
				((pPrimitive->polygon[j].i_vertex
					- (INT_PTR) m_pVertexesBuf) / sizeof(E3D_VECTOR4)) ;
			if ( dwTypeFlag & E3D_SMOOTH_POLYGON )
			{
				pPrimitive->polygon[j].normal = (PE3D_VECTOR4)
					((pPrimitive->polygon[j].i_normal
						- (INT_PTR) m_pNormalsBuf) / sizeof(E3D_VECTOR4)) ;
			}
		}
	}
	else if ( dwTypeFlag == E3D_INFINITE_PLANE )
	{
		//
		// 無限平面プリミティブ
		//
		pPrimitive->infinite_plane.vertex = (PE3D_VECTOR4)
			((pPrimitive->infinite_plane.i_vertex
				- (INT_PTR) m_pVertexesBuf) / sizeof(E3D_VECTOR4)) ;
		pPrimitive->infinite_plane.xAxis = (PE3D_VECTOR4)
			((pPrimitive->infinite_plane.i_xAxis
				- (INT_PTR) m_pNormalsBuf) / sizeof(E3D_VECTOR4)) ;
		pPrimitive->infinite_plane.yAxis = (PE3D_VECTOR4)
			((pPrimitive->infinite_plane.i_yAxis
				- (INT_PTR) m_pNormalsBuf) / sizeof(E3D_VECTOR4)) ;
	}
	else if ( dwTypeFlag == E3D_IMAGE_PRIMITIVE )
	{
		//
		// 画像プリミティブ
		//
		pPrimitive->image.vCenter = (PE3D_VECTOR4)
			((pPrimitive->image.i_vCenter
				- (INT_PTR) m_pVertexesBuf) / sizeof(E3D_VECTOR4)) ;
		pPrimitive->image.vEnlarge = (PE3D_VECTOR4)
			((pPrimitive->image.i_vEnlarge
				- (INT_PTR) m_pNormalsBuf) / sizeof(E3D_VECTOR4)) ;
	}
	else
	{
		return	eslErrFailed ;
	}
	return	eslErrSuccess ;
}

// プリミティブの頂点インデックスに加算する
//////////////////////////////////////////////////////////////////////////////
void E3DPolygonModel::AddPrimitiveVertexIndex
		( E3D_PRIMITIVE_POLYGON * pPrimitive,
			int nAddVertex, int nVertexMin, int nVertexMax )
{
	if ( nVertexMax < 0 )
	{
		nVertexMax = 0x7FFFFFFF ;
	}
	const DWORD	dwTypeFlag = pPrimitive->dwTypeFlag ;
	if ( dwTypeFlag & E3D_MESH_POLYGON )
	{
		//
		// メッシュ
		//
		INT_PTR	vertex = pPrimitive->mesh.i_vertices ;
		if ( (vertex >= nVertexMin) && (vertex < nVertexMax) )
		{
			pPrimitive->mesh.i_vertices = (vertex + nAddVertex) ;
		}
	}
	else if ( !(dwTypeFlag & ~E3D_POLYGON_PRIMITIVE_MASK) )
	{
		//
		// ポリゴンプリミティブ
		//
		const DWORD	dwVertexCount = pPrimitive->dwVertexCount ;
		for ( DWORD j = 0; j < dwVertexCount; j ++ )
		{
			INT_PTR	vertex = pPrimitive->polygon[j].i_vertex ;
			if ( (vertex >= nVertexMin) && (vertex < nVertexMax) )
			{
				pPrimitive->polygon[j].i_vertex = (vertex + nAddVertex) ;
			}
		}
	}
	else if ( dwTypeFlag == E3D_INFINITE_PLANE )
	{
		//
		// 無限平面プリミティブ
		//
		INT_PTR	vertex = pPrimitive->infinite_plane.i_vertex ;
		if ( (vertex >= nVertexMin) && (vertex < nVertexMax) )
		{
			pPrimitive->infinite_plane.i_vertex = (vertex + nAddVertex) ;
		}
	}
	else if ( dwTypeFlag == E3D_IMAGE_PRIMITIVE )
	{
		//
		// 画像プリミティブ
		//
		INT_PTR	vertex = pPrimitive->image.i_vCenter ;
		if ( (vertex >= nVertexMin) && (vertex < nVertexMax) )
		{
			pPrimitive->image.i_vCenter = (vertex + nAddVertex) ;
		}
	}
}

// プリミティブの法線インデックスに加算する
//////////////////////////////////////////////////////////////////////////////
void E3DPolygonModel::AddPrimitiveNormalIndex
		( E3D_PRIMITIVE_POLYGON * pPrimitive,
			int nAddNormal, int nNormalMin, int nNormalMax )
{
	if ( nNormalMax < 0 )
	{
		nNormalMax = 0x7FFFFFFF ;
	}
	const DWORD	dwTypeFlag = pPrimitive->dwTypeFlag ;
	if ( dwTypeFlag & E3D_MESH_POLYGON )
	{
		//
		// メッシュ
		//
		if ( dwTypeFlag & E3D_SMOOTH_POLYGON )
		{
			INT_PTR	normal = pPrimitive->mesh.i_normals ;
			if ( (normal >= nNormalMin) && (normal < nNormalMax) )
			{
				pPrimitive->mesh.i_normals = (normal + nAddNormal) ;
			}
		}
	}
	else if ( !(dwTypeFlag & ~E3D_POLYGON_PRIMITIVE_MASK) )
	{
		//
		// ポリゴンプリミティブ
		//
		const DWORD	dwVertexCount = pPrimitive->dwVertexCount ;
		for ( DWORD j = 0; j < dwVertexCount; j ++ )
		{
			if ( dwTypeFlag & E3D_SMOOTH_POLYGON )
			{
				INT_PTR	normal = pPrimitive->polygon[j].i_normal ;
				if ( (normal >= nNormalMin) && (normal < nNormalMax) )
				{
					pPrimitive->polygon[j].i_normal = (normal + nAddNormal) ;
				}
			}
		}
	}
	else if ( dwTypeFlag == E3D_INFINITE_PLANE )
	{
		//
		// 無限平面プリミティブ
		//
		INT_PTR	normal = pPrimitive->infinite_plane.i_xAxis ;
		if ( (normal >= nNormalMin) && (normal < nNormalMax) )
		{
			pPrimitive->infinite_plane.i_xAxis = (normal + nAddNormal) ;
		}
		normal = pPrimitive->infinite_plane.i_yAxis ;
		if ( (normal >= nNormalMin) && (normal < nNormalMax) )
		{
			pPrimitive->infinite_plane.i_yAxis = (normal + nAddNormal) ;
		}
	}
	else if ( dwTypeFlag == E3D_IMAGE_PRIMITIVE )
	{
		//
		// 画像プリミティブ
		//
		INT_PTR	normal = pPrimitive->image.i_vEnlarge ;
		if ( (normal >= nNormalMin) && (normal < nNormalMax) )
		{
			pPrimitive->image.i_vEnlarge = (normal + nAddNormal) ;
		}
	}
}

// プリミティブのUV座標をスケーリングする
//////////////////////////////////////////////////////////////////////////////
void E3DPolygonModel::ScalePrimitiveUVMap
		( E3D_PRIMITIVE_POLYGON * pPrimitive,
			double xScale, double yScale )
{
	const DWORD	dwTypeFlag = pPrimitive->dwTypeFlag ;
	if ( dwTypeFlag & E3D_MESH_POLYGON )
	{
		//
		// メッシュ
		//
		if ( dwTypeFlag & E3D_TEXTURE_POLYGON )
		{
			const DWORD	dwVertexCount = pPrimitive->dwVertexCount ;
			E3D_VECTOR_2D *	pUV = pPrimitive->mesh.uv_map ;
			for ( DWORD j = 0; j < dwVertexCount; j ++ )
			{
				pUV[j].x = (REAL32) (pUV[j].x * xScale) ;
				pUV[j].y = (REAL32) (pUV[j].y * yScale) ;
			}
		}
	}
	else if ( !(dwTypeFlag & ~E3D_POLYGON_PRIMITIVE_MASK) )
	{
		//
		// ポリゴンプリミティブ
		//
		if ( dwTypeFlag & E3D_TEXTURE_POLYGON )
		{
			const DWORD	dwVertexCount = pPrimitive->dwVertexCount ;
			for ( DWORD j = 0; j < dwVertexCount; j ++ )
			{
				pPrimitive->polygon[j].uv_map.x =
					(REAL32) (pPrimitive->polygon[j].uv_map.x * xScale) ;
				pPrimitive->polygon[j].uv_map.y =
					(REAL32) (pPrimitive->polygon[j].uv_map.y * yScale) ;
			}
		}
	}
}

// プリミティブデータのバイト数を計算する
//////////////////////////////////////////////////////////////////////////////
DWORD E3DPolygonModel::CalcPrimitiveDataSize
	( const E3D_PRIMITIVE_POLYGON * pPrimitive )
{
	if ( pPrimitive->dwDataSize != 0 )
	{
		return	pPrimitive->dwDataSize + 0x10 ;
	}
	DWORD	dwTypeFlag = pPrimitive->dwTypeFlag ;
	if ( !(dwTypeFlag & ~E3D_POLYGON_PRIMITIVE_MASK) )
	{
		return	(0x10 + pPrimitive->dwVertexCount
							* sizeof(E3D_PRIMITIVE_VERTEX)) ;
	}
	else if ( dwTypeFlag & E3D_MESH_POLYGON )
	{
		DWORD	dwVertexCount = pPrimitive->dwVertexCount ;
		DWORD	dwSize =
			(0x10 + sizeof(E3D_PRIMITIVE_MESH))
				+ (dwVertexCount - 1) * sizeof(E3D_VECTOR_2D) ;
		const E3D_PRIMITIVE_MESH_LIST *	pMesh =
				(const E3D_PRIMITIVE_MESH_LIST *)
					&(pPrimitive->mesh.uv_map[dwVertexCount]) ;
		dwSize += pMesh->dwMeshBytes ;
		return	dwSize ;
	}
	else if ( dwTypeFlag == E3D_INFINITE_PLANE )
	{
		return	(0x10 + sizeof(E3D_PRIMITIVE_INFINITE_PLANE)) ;
	}
	else if ( dwTypeFlag == E3D_IMAGE_PRIMITIVE )
	{
		return	(0x10 + sizeof(E3D_PRIMITIVE_IMAGE)) ;
	}
	return	0 ;
}

// プリミティブの頂点・法線座標をコミットする
//////////////////////////////////////////////////////////////////////////////
void E3DPolygonModel::CommitPrimitiveVertices
	( E3D_PRIMITIVE_POLYGON * pPrimitive )
{
	const DWORD	dwTypeFlag = pPrimitive->dwTypeFlag ;
	if ( dwTypeFlag & E3D_MESH_POLYGON )
	{
		//
		// メッシュ
		//
		CommitVertexBuffer
			( VertexBufferPointerToIndex
				( pPrimitive->mesh.vertices ), pPrimitive->dwVertexCount ) ;
		//
		if ( (dwTypeFlag & E3D_SMOOTH_POLYGON)
						&& (pPrimitive->mesh.normals != NULL) )
		{
			CommitNormalBuffer
				( NormalBufferPointerToIndex
					( pPrimitive->mesh.normals ), pPrimitive->dwVertexCount ) ;
		}
	}
	else if ( !(dwTypeFlag & ~E3D_POLYGON_PRIMITIVE_MASK) )
	{
		//
		// ポリゴンプリミティブ
		//
		const DWORD	dwVertexCount = pPrimitive->dwVertexCount ;
		for ( DWORD j = 0; j < dwVertexCount; j ++ )
		{
			CommitVertexBuffer
				( VertexBufferPointerToIndex
					( pPrimitive->polygon[j].vertex ), 1 ) ;
			//
			if ( (dwTypeFlag & E3D_SMOOTH_POLYGON)
					&& (pPrimitive->polygon[j].normal != NULL) )
			{
				CommitNormalBuffer
					( NormalBufferPointerToIndex
						( pPrimitive->polygon[j].normal ), 1 ) ;
			}
		}
	}
	else if ( dwTypeFlag == E3D_INFINITE_PLANE )
	{
		//
		// 無限平面プリミティブ
		//
		CommitVertexBuffer
			( VertexBufferPointerToIndex
				( pPrimitive->infinite_plane.vertex ), 1 ) ;
		//
		CommitNormalBuffer
			( NormalBufferPointerToIndex
				( pPrimitive->infinite_plane.xAxis ), 1 ) ;
		CommitNormalBuffer
			( NormalBufferPointerToIndex
				( pPrimitive->infinite_plane.yAxis ), 1 ) ;
	}
	else if ( dwTypeFlag == E3D_IMAGE_PRIMITIVE )
	{
		//
		// 画像プリミティブ
		//
		CommitVertexBuffer
			( VertexBufferPointerToIndex
				( pPrimitive->image.vCenter ), 1 ) ;
		//
		CommitNormalBuffer
			( NormalBufferPointerToIndex
				( pPrimitive->image.vEnlarge ), 1 ) ;
	}
}

// 頂点リストの座標をコミットする
//////////////////////////////////////////////////////////////////////////////
void E3DPolygonModel::CommitVertexBuffer( int iVertex, int nCount )
{
	if ( ((unsigned int) iVertex < m_nVertexCount)
		&& ((unsigned int) (iVertex + nCount) <= m_nVertexCount) )
	{
		::eslMoveMemory
			( m_pVertexes + iVertex,
				m_pVertexesBuf + iVertex, nCount * sizeof(E3D_VECTOR4) ) ;
	}
}

// 法線リストの座標をコミットする
//////////////////////////////////////////////////////////////////////////////
void E3DPolygonModel::CommitNormalBuffer( int iNormal, int nCount )
{
	if ( ((unsigned int) iNormal < m_nNormalCount)
		&& ((unsigned int) (iNormal + nCount) <= m_nNormalCount) )
	{
		::eslMoveMemory
			( m_pNormals + iNormal,
				m_pNormalsBuf + iNormal, nCount * sizeof(E3D_VECTOR4) ) ;
	}
}


#if	!defined(__PLATFORM_ANDROID__)

// モデルデータのサイズを取得する
//////////////////////////////////////////////////////////////////////////////
ESLError E3DPolygonModel::GetUntransformedModelRange
	( E3D_VECTOR4 & vMax, E3D_VECTOR4 & vMin )
{
	if ( m_pVertexes == NULL )
	{
		return	eslErrGeneral ;
	}
	eglGetMaxVector( &vMax, m_pVertexes, m_nVertexCount ) ;
	eglGetMinVector( &vMin, m_pVertexes, m_nVertexCount ) ;
	return	eslErrSuccess ;
}

ESLError E3DPolygonModel::GetTransformedModelRange
	( E3D_VECTOR4 & vMax, E3D_VECTOR4 & vMin )
{
	if ( m_pVertexesBuf == NULL )
	{
		return	eslErrGeneral ;
	}
	eglGetMaxVector( &vMax, m_pVertexesBuf, m_nVertexCount ) ;
	eglGetMinVector( &vMin, m_pVertexesBuf, m_nVertexCount ) ;
	return	eslErrSuccess ;
}

#endif


// 画像プリミティブを作成する
//////////////////////////////////////////////////////////////////////////////
void E3DPolygonModel::CreateImagePrimitive
	( PEGL_IMAGE_INFO pImage, PCEGL_RECT pView,
		const E3D_VECTOR_2D * pCenter, const E3D_VECTOR_2D * pEnlarge )
{
	//
	// 現在のデータを削除
	//
	DeleteContents( ) ;
	//
	// テクスチャ登録
	//
	if ( pView != NULL )
	{
		EGLImage *	pTexture = new EGLImage ;
		pTexture->SetImageView( pImage, pView ) ;
		m_txlib.SetAs( L"image1", pTexture ) ;
		pImage = pTexture->GetInfo( ) ;
	}
	//
	// 頂点バッファを確保
	//
	AllocateVertexBuffer( 1 ) ;
	AllocateConstantBuffer( 1 ) ;
	//
	// 頂点座標設定
	//
	E3DVector2D	vCenter, vEnlarge( 1, 1 ) ;
	vCenter.x = (REAL32) (pImage->dwImageWidth * 0.5) ;
	vCenter.y = (REAL32) (pImage->dwImageHeight * 0.5) ;
	if ( pCenter != NULL )
	{
		vCenter = *pCenter ;
	}
	if ( pEnlarge != NULL )
	{
		vEnlarge = *pEnlarge ;
	}
	m_pVertexes[0] = E3DVector4( 0, 0, 0 ) ;
	m_pConstantBuf[0] = E3DVector4( vEnlarge.x, vEnlarge.y, 0.0F ) ;
	//
	// プリミティブ設定
	//
	E3D_PRIMITIVE_POLYGON	prmtv ;
	::eslFillMemory( &prmtv, 0, sizeof(prmtv) ) ;
	prmtv.dwTypeFlag = E3D_IMAGE_PRIMITIVE ;
	prmtv.image.vCenter = m_pVertexesBuf ;
	prmtv.image.vEnlarge = m_pConstantBuf ;
	prmtv.image.vImageBase = vCenter ;
	prmtv.image.pImageInf = pImage ;
	//
	E3D_PRIMITIVE_POLYGON *	pprmtv = &prmtv ;
	AddPrimitives( 1, &pprmtv ) ;
	//
	CommitVertexBuffer( 0, 1 ) ;
	CommitNormalBuffer( 0, 1 ) ;
}

// 画像モデルを作成する
//////////////////////////////////////////////////////////////////////////////
void E3DPolygonModel::CreateImagePolygon
	( PEGL_IMAGE_INFO pImage, PCEGL_RECT pView,
		const E3D_VECTOR_2D * pCenter,
		PCE3D_SURFACE_ATTRIBUTE pSurfAttr,
		REAL32 rFogDeepness, const EGL_PALETTE * pFogColor )
{
	//
	// 現在のデータを削除
	//////////////////////////////////////////////////////////////////////////
	DeleteContents( ) ;
	//
	// 表面属性を登録
	//////////////////////////////////////////////////////////////////////////
	PE3D_SURFACE_ATTRIBUTE	pAttr = new E3D_SURFACE_ATTRIBUTE ;
	if ( pSurfAttr != NULL )
	{
		*pAttr = *pSurfAttr ;
	}
	else
	{
		::eslFillMemory( pAttr, 0, sizeof(E3D_SURFACE_ATTRIBUTE) ) ;
		pAttr->dwShadingFlags = E3DSAF_NO_SHADING
			| E3DSAF_TEXTURE_SMOOTH | E3DSAF_TEXTURE_MAPPING ;
		pAttr->rgbaColor.rgbMul.dwPixelCode = 0x00FFFFFF ;
	}
	pAttr->txmap.pTextureImage = pImage ;
	pAttr->txmap.pSmallImage = pImage ;
	m_sflib.Add( L"image1", pAttr ) ;
	//
	// パラメータを取得
	//////////////////////////////////////////////////////////////////////////
	EGL_RECT		rectView ;
	E3D_VECTOR_2D	vCenter ;
	bool	fInfinitePlane = false ;
	if ( pView != NULL )
	{
		rectView = *pView ;
		fInfinitePlane = (rectView.left == 0x80000000)
						&& (rectView.top == 0x80000000)
						&& (rectView.right == 0x7FFFFFFF)
						&& (rectView.bottom == 0x7FFFFFFF) ;
	}
	else
	{
		rectView.left = 0 ;
		rectView.top = 0 ;
		rectView.right = pImage->dwImageWidth - 1 ;
		rectView.bottom = pImage->dwImageHeight - 1 ;
	}
	if ( pCenter != NULL )
	{
		vCenter = *pCenter ;
	}
	else
	{
		if ( fInfinitePlane )
		{
			vCenter.x = 0.0F ;
			vCenter.y = 0.0F ;
		}
		else
		{
			vCenter.x = (REAL32) (rectView.left + rectView.right) * 0.5F ;
			vCenter.y = (REAL32) (rectView.top + rectView.bottom) * 0.5F ;
		}
	}
	if ( !fInfinitePlane )
	{
		//
		//	通常のポリゴン
		//////////////////////////////////////////////////////////////////////
		//
		// 頂点バッファを確保
		//
		AllocateVertexBuffer( 4 ) ;
		AllocateNormalBuffer( 1 ) ;
		//
		// 頂点座標設定
		//
		m_pVertexesBuf[0] =
			E3DVector4( (REAL32) rectView.left - vCenter.x,
						(REAL32) rectView.top - vCenter.y, 0.0F ) ;
		m_pVertexesBuf[2] =
			E3DVector4( (REAL32) rectView.right - vCenter.x,
						(REAL32) rectView.bottom - vCenter.y, 0.0F ) ;
		m_pVertexesBuf[1] =
			E3DVector4( m_pVertexesBuf[2].x, m_pVertexesBuf[0].y, 0.0F ) ;
		m_pVertexesBuf[3] =
			E3DVector4( m_pVertexesBuf[0].x, m_pVertexesBuf[2].y, 0.0F ) ;
		//
		m_pNormalsBuf[0].x = 0.0F ;
		m_pNormalsBuf[0].y = 0.0F ;
		m_pNormalsBuf[0].z = -1.0F ;
		//
		// プリミティブ設定
		//
		E3D_PRIMITIVE_VERTEX	vertex[4] ;
		for ( int i = 0; i < 4; i ++ )
		{
			vertex[i].vertex = m_pVertexesBuf + i ;
			vertex[i].normal = m_pNormalsBuf ;
			vertex[i].uv_map.x = m_pVertexesBuf[i].x + vCenter.x ;
			vertex[i].uv_map.y = m_pVertexesBuf[i].y + vCenter.y ;
		}
		AddPolygon
			( E3D_SMOOTH_POLYGON | E3D_TEXTURE_POLYGON, pAttr, 4, vertex ) ;
		//
		CommitVertexBuffer( 0, 4 ) ;
		CommitNormalBuffer( 0, 1 ) ;
	}
	else
	{
		//
		//	無限平面
		//////////////////////////////////////////////////////////////////////
		//
		// 頂点バッファを確保
		//
		AllocateVertexBuffer( 1 ) ;
		AllocateNormalBuffer( 2 ) ;
		//
		// 頂点座標設定
		//
		m_pVertexesBuf[0] = E3DVector4( - vCenter.x, - vCenter.y, 0.0F ) ;
		//
		m_pNormalsBuf[0] = E3DVector4( 1.0F, 0.0F, 0.0F ) ;
		m_pNormalsBuf[1] = E3DVector4( 0.0F, 1.0F, 0.0F ) ;
		//
		// プリミティブ設定
		//
		E3D_PRIMITIVE_POLYGON	prmtv ;
		::eslFillMemory( &prmtv, 0, sizeof(prmtv) ) ;
		prmtv.dwTypeFlag = E3D_INFINITE_PLANE ;
		prmtv.pSurfaceAttr = pAttr ;
		prmtv.infinite_plane.vertex = m_pVertexesBuf ;
		prmtv.infinite_plane.xAxis = m_pNormalsBuf ;
		prmtv.infinite_plane.yAxis = m_pNormalsBuf + 1 ;
		prmtv.infinite_plane.rFogDeepness = rFogDeepness ;
		if ( pFogColor != NULL )
		{
			prmtv.infinite_plane.rgbFogColor = *pFogColor ;
		}
		E3D_PRIMITIVE_POLYGON *	pprmtv = &prmtv ;
		AddPrimitives( 1, &pprmtv ) ;
		//
		CommitVertexBuffer( 0, 1 ) ;
		CommitNormalBuffer( 0, 2 ) ;
	}
}

// 画像モデルの画像を切り替える
//////////////////////////////////////////////////////////////////////////////
ESLError E3DPolygonModel::AttachImagePolygon
	( PEGL_IMAGE_INFO pImage,
		PCEGL_RECT pView, const E3D_VECTOR_2D * pCenter )
{
	if ( m_lstPrimitives.GetSize() != 1 )
	{
		return	eslErrGeneral ;
	}
	E3D_PRIMITIVE_POLYGON *	pPrimitive = m_lstPrimitives.GetAt( 0 ) ;
	if ( pPrimitive == NULL )
	{
		return	eslErrGeneral ;
	}
	if ( pPrimitive->dwTypeFlag == E3D_IMAGE_PRIMITIVE )
	{
		//
		// 画像プリミティブ
		//
		if ( pView != NULL )
		{
			EGLImage *	pTexture = m_txlib.GetAs( L"image1" ) ;
			if ( pTexture == NULL )
			{
				pTexture = new EGLImage ;
				m_txlib.SetAs( L"image1", pTexture ) ;
			}
			pTexture->SetImageView( pImage, pView ) ;
			pImage = pTexture->GetInfo( ) ;
		}
		pPrimitive->image.pImageInf = pImage ;
		if ( pCenter != NULL )
		{
			pPrimitive->image.vImageBase = *pCenter ;
		}
	}
	else if ( pPrimitive->pSurfaceAttr != NULL )
	{
		pPrimitive->pSurfaceAttr->txmap.pTextureImage = pImage ;
		pPrimitive->pSurfaceAttr->txmap.pSmallImage = pImage ;
		//
		if ( pPrimitive->dwTypeFlag == E3D_INFINITE_PLANE )
		{
			//
			// 無限平面
			//
			ESLAssert( m_pVertexes != NULL ) ;
			if ( m_pVertexes != NULL )
			{
				m_pVertexes[0].x = - pCenter->x ;
				m_pVertexes[0].y = - pCenter->y ;
			}
		}
		else
		{
			//
			// ポリゴン
			//
			EGL_RECT		rectView ;
			E3D_VECTOR_2D	vCenter ;
			if ( pView != NULL )
			{
				rectView = *pView ;
			}
			else
			{
				rectView.left = 0 ;
				rectView.top = 0 ;
				rectView.right = pImage->dwImageWidth - 1 ;
				rectView.bottom = pImage->dwImageHeight - 1 ;
			}
			if ( pCenter != NULL )
			{
				vCenter = *pCenter ;
			}
			else
			{
				vCenter.x = (REAL32) (rectView.left + rectView.right) * 0.5F ;
				vCenter.y = (REAL32) (rectView.top + rectView.bottom) * 0.5F ;
			}
			//
			// 頂点座標設定
			//
			ESLAssert( m_pVertexes != NULL ) ;
			if ( m_pVertexes != NULL )
			{
				m_pVertexes[0] =
					E3DVector4( (REAL32) rectView.left - vCenter.x,
								(REAL32) rectView.top - vCenter.y, 0.0F ) ;
				m_pVertexes[2] =
					E3DVector4( (REAL32) rectView.right - vCenter.x,
								(REAL32) rectView.bottom - vCenter.y, 0.0F ) ;
				m_pVertexes[1] =
					E3DVector4( m_pVertexes[2].x, m_pVertexes[0].y, 0.0F ) ;
				m_pVertexes[3] =
					E3DVector4( m_pVertexes[0].x, m_pVertexes[2].y, 0.0F ) ;
			}
		}
	}
	return	eslErrSuccess ;
}

// 表面属性生成して登録する
//////////////////////////////////////////////////////////////////////////////
E3D_SURFACE_ATTRIBUTE * E3DPolygonModel::RegisterSurfaceAttribute
	( E3D_COLOR rgbaColor,
		DWORD dwShadingFlags,
		SDWORD nAmbient, SDWORD nDiffusion,
		SDWORD nSpecular, SDWORD nSpecularSize,
		SDWORD nTransparency, SDWORD nDeepness,
		DWORD nReflection, REAL32 nRefraction )
{
	PE3D_SURFACE_ATTRIBUTE	pAttr = new E3D_SURFACE_ATTRIBUTE ;
	::eslFillMemory( pAttr, 0, sizeof(E3D_SURFACE_ATTRIBUTE) ) ;
	//
	pAttr->dwShadingFlags = dwShadingFlags & ~E3DSAF_TEXTURE_MAPPING ;
	pAttr->rgbaColor = rgbaColor ;
	pAttr->nAmbient = nAmbient ;
	pAttr->nDiffusion = nDiffusion ;
	pAttr->nSpecular = nSpecular ;
	pAttr->nSpecularSize = nSpecularSize ;
	pAttr->nTransparency = nTransparency ;
	pAttr->nDeepness = nDeepness ;
	pAttr->nReflection = nReflection ;
	pAttr->nRefraction = nRefraction ;
	//
	EWideString	wstrSurfName ;
	int	nIndex = m_sflib.GetSize() ;
	for ( ; ; )
	{
		wstrSurfName = L"attr" ;
		wstrSurfName += EWideString( nIndex ) ;
		if ( m_sflib.GetAs( wstrSurfName ) == NULL )
		{
			break ;
		}
		nIndex ++ ;
	}
	m_sflib.Add( wstrSurfName, pAttr ) ;
	return	pAttr ;
}

// テクスチャ表面属性生成して登録する
//////////////////////////////////////////////////////////////////////////////
E3D_SURFACE_ATTRIBUTE * E3DPolygonModel::RegisterTextureAttribute
	( PEGL_IMAGE_INFO pTextureInf,
		DWORD dwShadingFlags,
		SDWORD nAmbient, SDWORD nDiffusion,
		SDWORD nSpecular, SDWORD nSpecularSize,
		SDWORD nTransparency, SDWORD nDeepness,
		DWORD nReflection, REAL32 nRefraction )
{
	PE3D_SURFACE_ATTRIBUTE	pAttr = new E3D_SURFACE_ATTRIBUTE ;
	::eslFillMemory( pAttr, 0, sizeof(E3D_SURFACE_ATTRIBUTE) ) ;
	//
	pAttr->dwShadingFlags = dwShadingFlags | E3DSAF_TEXTURE_MAPPING ;
	pAttr->rgbaColor.rgbMul.dwPixelCode = 0xFFFFFF ;
	pAttr->rgbaColor.rgbAdd.dwPixelCode = 0 ;
	pAttr->txmap.pTextureImage = pTextureInf ;
	pAttr->txmap.pSmallImage = pTextureInf ;
	pAttr->nAmbient = nAmbient ;
	pAttr->nDiffusion = nDiffusion ;
	pAttr->nSpecular = nSpecular ;
	pAttr->nSpecularSize = nSpecularSize ;
	pAttr->nTransparency = nTransparency ;
	pAttr->nDeepness = nDeepness ;
	pAttr->nReflection = nReflection ;
	pAttr->nRefraction = nRefraction ;
	//
	EWideString	wstrSurfName ;
	int	nIndex = m_sflib.GetSize() ;
	for ( ; ; )
	{
		wstrSurfName = L"attr" ;
		wstrSurfName += EWideString( nIndex ) ;
		if ( m_sflib.GetAs( wstrSurfName ) == NULL )
		{
			break ;
		}
		nIndex ++ ;
	}
	m_sflib.Add( wstrSurfName, pAttr ) ;
	return	pAttr ;
}

// 格子状メッシュの頂点数を計算する
//////////////////////////////////////////////////////////////////////////////
void E3DPolygonModel::CalcGridMeshVertexCount
	( GRID_MESH_INFO& gmi, int nMeshWidth, int nMeshHeight, int nFlags )
{
	gmi.nVertexCount = (nMeshWidth + 1) * (nMeshHeight + 1) ;
	gmi.nPolygonCount = nMeshWidth * nMeshHeight * 2 ;
	gmi.nHorzPolygonCount = nMeshWidth * 2 ;
	gmi.nMeshPolyWidth = nMeshWidth ;
	gmi.nMeshPolyHeight = nMeshHeight ;
	if ( nFlags & gridHorzLoop )
	{
		gmi.nHorzPolygonCount += 2 ;
		gmi.nMeshPolyWidth ++ ;
		gmi.nPolygonCount = gmi.nHorzPolygonCount * nMeshHeight ;
	}
	if ( nFlags & gridVertLoop )
	{
		gmi.nPolygonCount = gmi.nHorzPolygonCount * (nMeshHeight + 1 ) ;
		gmi.nMeshPolyHeight ++ ;
	}
	if ( nFlags & gridTopTip )
	{
		gmi.nVertexCount -= nMeshWidth ;
		gmi.nPolygonCount -= (gmi.nHorzPolygonCount >> 1) ;
	}
	if ( nFlags & gridBottomTip )
	{
		gmi.nVertexCount -= nMeshWidth ;
		gmi.nPolygonCount -= (gmi.nHorzPolygonCount >> 1) ;
	}
}

// 格子状メッシュを追加する
//////////////////////////////////////////////////////////////////////////////
E3D_PRIMITIVE_POLYGON *
	E3DPolygonModel::AddGridMeshPrimitive
		( E3D_SURFACE_ATTRIBUTE * pSurfAttr,
			int nMeshWidth, int nMeshHeight, int nFlags,
			PCE3D_VECTOR4 pvVertex, PCE3D_VECTOR4 pvNormal,
			PCE3D_VECTOR_2D pvUVMap, PCE3D_COLOR pvColor )
{
	//
	// バッファ確保
	//
	const int	iBaseVertex = m_nVertexCount ;
	const int	iBaseNormal = m_nNormalCount ;
	//
	GRID_MESH_INFO	gmi ;
	CalcGridMeshVertexCount( gmi, nMeshWidth, nMeshHeight, nFlags ) ;
	//
	const int	nVertexCount = gmi.nVertexCount ;
	const int	nPolygonCount = gmi.nPolygonCount ;
	const int	nHorzPolygonCount = gmi.nHorzPolygonCount ;
	const int	nMeshPolyWidth = gmi.nMeshPolyWidth ;
	const int	nMeshPolyHeight = gmi.nMeshPolyHeight ;
	//
	if ( (nVertexCount <= 0) || (nPolygonCount <= 0) )
	{
		return	NULL ;
	}
	ConvertAllPrimitiveAddressToIndex() ;
	AllocateVertexBuffer( iBaseVertex + nVertexCount ) ;
	AllocateNormalBuffer( iBaseNormal + nVertexCount ) ;
	ConvertAllPrimitiveIndexToAddress() ;
	//
	// プリミティブバッファ確保
	//
	const DWORD	dwHeaderSize =
		(0x10 + sizeof(E3D_PRIMITIVE_MESH))
			+ (nVertexCount - 1) * sizeof(E3D_VECTOR_2D) ;
	const DWORD	dwMeshListSize =
		sizeof(E3D_PRIMITIVE_MESH_LIST)
			+ (nPolygonCount - 1) * sizeof(E3D_PRIMITIVE_MESH_POLY) ;
	//
	E3D_PRIMITIVE_POLYGON *	pPrimitive =
		(E3D_PRIMITIVE_POLYGON*)
			::eslHeapAllocate
				( NULL, dwHeaderSize + dwMeshListSize, ESL_HEAP_ZERO_INIT ) ;
	m_lstPrimitives.Add( pPrimitive ) ;
	//
	// ヘッダ設定
	//
	pPrimitive->dwTypeFlag = E3D_MESH_POLYGON ;
	if ( (pvNormal != NULL) || (nFlags & meshAutoSmooth) )
	{
		pPrimitive->dwTypeFlag |= E3D_SMOOTH_POLYGON ;
	}
	if ( pvUVMap != NULL )
	{
		pPrimitive->dwTypeFlag |= E3D_TEXTURE_POLYGON ;
	}
	else if ( pvColor != NULL )
	{
		pPrimitive->dwTypeFlag |= E3D_VERTEX_COLOR_POLYGON ;
	}
	pPrimitive->pSurfaceAttr = pSurfAttr ;
	pPrimitive->dwVertexCount = nVertexCount ;
	pPrimitive->dwDataSize = dwHeaderSize + dwMeshListSize - 0x10 ;
	//
	pPrimitive->mesh.vertices = m_pVertexesBuf + iBaseVertex ;
	pPrimitive->mesh.normals = m_pNormalsBuf + iBaseNormal ;
	//
	// ポリゴンリスト設定
	//
	E3D_PRIMITIVE_MESH_LIST *	ppmlMesh =
		(E3D_PRIMITIVE_MESH_LIST*)
			&(pPrimitive->mesh.uv_map[nVertexCount]) ;
	//
	ppmlMesh->dwMeshBytes = dwMeshListSize ;
	ppmlMesh->dwPolyCount = nPolygonCount ;
	//
	E3D_PRIMITIVE_MESH_POLY *	ppmpMesh ;
	int	i, j ;
	int	iPoly = 0 ;
	int	nBodyHeight = nMeshPolyHeight ;
	//
	// 上部先端
	//
	int	iBodyBase = 0 ;
	if ( nFlags & gridTopTip )
	{
		nBodyHeight -- ;
		iBodyBase = 1 ;
		ESLAssert( nBodyHeight >= 0 ) ;
		//
		for ( i = 0; i < nMeshWidth; i ++ )
		{
			ppmpMesh = &(ppmlMesh->mpEntries[iPoly ++]) ;
			ppmpMesh->dwVertexCount = 3 ;
			ppmpMesh->dwIndex[0] = 0 ;
			ppmpMesh->dwIndex[1] = iBodyBase + i ;
			ppmpMesh->dwIndex[2] = iBodyBase + i + 1 ;
		}
		if ( nFlags & gridHorzLoop )
		{
			ppmpMesh = &(ppmlMesh->mpEntries[iPoly ++]) ;
			ppmpMesh->dwVertexCount = 3 ;
			ppmpMesh->dwIndex[0] = 0 ;
			ppmpMesh->dwIndex[1] = iBodyBase + nMeshWidth ;
			ppmpMesh->dwIndex[2] = iBodyBase ;
		}
	}
	//
	// 中央部分
	//
	if ( nFlags & gridBottomTip )
	{
		nBodyHeight -- ;
		ESLAssert( nBodyHeight >= 0 ) ;
	}
	for ( i = 0; i < nBodyHeight; i ++ )
	{
		const int	iLine0 = iBodyBase + i * (nMeshWidth + 1) ;
		const int	iLine1 = iLine0 + (nMeshWidth + 1) ;
		for ( j = 0; j < nMeshWidth; j ++ )
		{
			ppmpMesh = &(ppmlMesh->mpEntries[iPoly ++]) ;
			ppmpMesh->dwVertexCount = 3 ;
			ppmpMesh->dwIndex[0] = iLine0 + j + 1 ;
			ppmpMesh->dwIndex[1] = iLine0 + j ;
			ppmpMesh->dwIndex[2] = iLine1 + j ;
			//
			ppmpMesh = &(ppmlMesh->mpEntries[iPoly ++]) ;
			ppmpMesh->dwVertexCount = 3 ;
			ppmpMesh->dwIndex[0] = iLine0 + j + 1 ;
			ppmpMesh->dwIndex[1] = iLine1 + j ;
			ppmpMesh->dwIndex[2] = iLine1 + j + 1 ;
		}
		if ( nFlags & gridHorzLoop )
		{
			ppmpMesh = &(ppmlMesh->mpEntries[iPoly ++]) ;
			ppmpMesh->dwVertexCount = 3 ;
			ppmpMesh->dwIndex[0] = iLine0 ;
			ppmpMesh->dwIndex[1] = iLine0 + nMeshWidth ;
			ppmpMesh->dwIndex[2] = iLine1 + nMeshWidth ;
			//
			ppmpMesh = &(ppmlMesh->mpEntries[iPoly ++]) ;
			ppmpMesh->dwVertexCount = 3 ;
			ppmpMesh->dwIndex[0] = iLine0 ;
			ppmpMesh->dwIndex[1] = iLine1 + nMeshWidth ;
			ppmpMesh->dwIndex[2] = iLine1 ;
		}
	}
	//
	// 下部先端
	//
	if ( nFlags & gridBottomTip )
	{
		const int	iLine = iBodyBase + nBodyHeight * (nMeshWidth + 1) ;
		const int	iBottom = (nFlags & gridVertLoop) ? 0 : (iLine + nMeshWidth + 1) ;
		for ( j = 0; j < nMeshWidth; j ++ )
		{
			ppmpMesh = &(ppmlMesh->mpEntries[iPoly ++]) ;
			ppmpMesh->dwVertexCount = 3 ;
			ppmpMesh->dwIndex[0] = iLine + j + 1 ;
			ppmpMesh->dwIndex[1] = iLine + j ;
			ppmpMesh->dwIndex[2] = iBottom ;
		}
		if ( nFlags & gridHorzLoop )
		{
			ppmpMesh = &(ppmlMesh->mpEntries[iPoly ++]) ;
			ppmpMesh->dwVertexCount = 3 ;
			ppmpMesh->dwIndex[0] = iLine ;
			ppmpMesh->dwIndex[1] = iLine + nMeshWidth ;
			ppmpMesh->dwIndex[2] = iBottom ;
		}
	}
	ESLAssert( iPoly == nPolygonCount ) ;
	//
	// 情報設定
	//
	ModifyGridMeshPrimitive
		( pPrimitive, nMeshWidth, nMeshHeight,
			nFlags, pvVertex, pvNormal, pvUVMap, pvColor ) ;
	//
	return	pPrimitive ;
}

// 格子状メッシュを変更する
//////////////////////////////////////////////////////////////////////////////
ESLError E3DPolygonModel::ModifyGridMeshPrimitive
		( E3D_PRIMITIVE_POLYGON * pGridMesh,
			int nMeshWidth, int nMeshHeight, int nFlags,
			PCE3D_VECTOR4 pvVertex, PCE3D_VECTOR4 pvNormal,
			PCE3D_VECTOR_2D pvUVMap, PCE3D_COLOR pvColor )
{
	//
	// 頂点数・ポリゴン数計算
	//
	GRID_MESH_INFO	gmi ;
	CalcGridMeshVertexCount( gmi, nMeshWidth, nMeshHeight, nFlags ) ;
	//
	if ( (gmi.nVertexCount <= 0) || (gmi.nPolygonCount <= 0) )
	{
		return	eslErrFailed ;
	}
	//
	// 頂点複製
	//
	ESLError	err =
		ModifyMeshPrimitive
			( pGridMesh, gmi.nVertexCount,
					pvVertex, pvNormal, pvUVMap, pvColor ) ;
	//
	if ( (pvNormal == NULL) && (nFlags & meshAutoSmooth) )
	{
		SmoothTriangleMesh( pGridMesh ) ;
	}
	CommitPrimitiveVertices( pGridMesh ) ;
	//
	return	err ;
}

// トライアングルストリップを追加する
//////////////////////////////////////////////////////////////////////////////
E3D_PRIMITIVE_POLYGON *
	E3DPolygonModel::AddTriangleStripPrimitive
		( E3D_SURFACE_ATTRIBUTE * pSurfAttr,
			int nTriangleStripCount, int nFlags,
			PCE3D_VECTOR4 pvVertex, PCE3D_VECTOR4 pvNormal,
			PCE3D_VECTOR_2D pvUVMap, PCE3D_COLOR pvColor )
{
	//
	// バッファ確保
	//
	const int	iBaseVertex = m_nVertexCount ;
	const int	iBaseNormal = m_nNormalCount ;
	const int	nVertexCount = nTriangleStripCount + 2 ;
	const int	nPolygonCount = nTriangleStripCount ;
	if ( (nVertexCount <= 0) || (nPolygonCount <= 0) )
	{
		return	NULL ;
	}
	ConvertAllPrimitiveAddressToIndex() ;
	AllocateVertexBuffer( iBaseVertex + nVertexCount ) ;
	AllocateNormalBuffer( iBaseNormal + nVertexCount ) ;
	ConvertAllPrimitiveIndexToAddress() ;
	//
	// プリミティブバッファ確保
	//
	const DWORD	dwHeaderSize =
		(0x10 + sizeof(E3D_PRIMITIVE_MESH))
			+ (nVertexCount - 1) * sizeof(E3D_VECTOR_2D) ;
	const DWORD	dwMeshListSize =
		sizeof(E3D_PRIMITIVE_MESH_LIST)
			+ (nPolygonCount - 1) * sizeof(E3D_PRIMITIVE_MESH_POLY) ;
	//
	E3D_PRIMITIVE_POLYGON *	pPrimitive =
		(E3D_PRIMITIVE_POLYGON*)
			::eslHeapAllocate
				( NULL, dwHeaderSize + dwMeshListSize, ESL_HEAP_ZERO_INIT ) ;
	m_lstPrimitives.Add( pPrimitive ) ;
	//
	// ヘッダ設定
	//
	pPrimitive->dwTypeFlag = E3D_MESH_POLYGON ;
	if ( (pvNormal != NULL) || (nFlags & meshAutoSmooth) )
	{
		pPrimitive->dwTypeFlag |= E3D_SMOOTH_POLYGON ;
	}
	if ( pvUVMap != NULL )
	{
		pPrimitive->dwTypeFlag |= E3D_TEXTURE_POLYGON ;
	}
	else if ( pvColor != NULL )
	{
		pPrimitive->dwTypeFlag |= E3D_VERTEX_COLOR_POLYGON ;
	}
	pPrimitive->pSurfaceAttr = pSurfAttr ;
	pPrimitive->dwVertexCount = nVertexCount ;
	pPrimitive->dwDataSize = dwHeaderSize + dwMeshListSize - 0x10 ;
	//
	pPrimitive->mesh.vertices = m_pVertexesBuf + iBaseVertex ;
	pPrimitive->mesh.normals = m_pNormalsBuf + iBaseNormal ;
	//
	// ポリゴンリスト設定
	//
	E3D_PRIMITIVE_MESH_LIST *	ppmlMesh =
		(E3D_PRIMITIVE_MESH_LIST*)
			&(pPrimitive->mesh.uv_map[nVertexCount]) ;
	//
	ppmlMesh->dwMeshBytes = dwMeshListSize ;
	ppmlMesh->dwPolyCount = nPolygonCount ;
	//
	for ( int i = 0; i < nPolygonCount; i ++ )
	{
		E3D_PRIMITIVE_MESH_POLY *
			ppmpMesh = &(ppmlMesh->mpEntries[i]) ;
		ppmpMesh->dwVertexCount = 3 ;
		if ( !(i & 0x01) )
		{
			ppmpMesh->dwIndex[0] = i ;
			ppmpMesh->dwIndex[1] = i + 1 ;
			ppmpMesh->dwIndex[2] = i + 2 ;
		}
		else
		{
			ppmpMesh->dwIndex[0] = i ;
			ppmpMesh->dwIndex[1] = i + 2 ;
			ppmpMesh->dwIndex[2] = i + 1 ;
		}
	}
	//
	// 情報設定
	//
	ModifyTriangleStripPrimitive
		( pPrimitive, nTriangleStripCount,
			nFlags, pvVertex, pvNormal, pvUVMap, pvColor ) ;
	//
	return	pPrimitive ;
}

// トライアングルストリップを変更する
//////////////////////////////////////////////////////////////////////////////
ESLError E3DPolygonModel::ModifyTriangleStripPrimitive
		( E3D_PRIMITIVE_POLYGON * pMesh,
			int nTriangleStripCount, int nFlags,
			PCE3D_VECTOR4 pvVertex, PCE3D_VECTOR4 pvNormal,
			PCE3D_VECTOR_2D pvUVMap, PCE3D_COLOR pvColor )
{
	//
	// 頂点数・ポリゴン数計算
	//
	const int	iBaseVertex = m_nVertexCount ;
	const int	iBaseNormal = m_nNormalCount ;
	const int	nVertexCount = nTriangleStripCount + 2 ;
	const int	nPolygonCount = nTriangleStripCount ;
	if ( (nVertexCount <= 0) || (nPolygonCount <= 0) )
	{
		return	eslErrFailed ;
	}
	//
	// 頂点複製
	//
	ESLError	err =
		ModifyMeshPrimitive
			( pMesh, nVertexCount,
					pvVertex, pvNormal, pvUVMap, pvColor ) ;
	//
	if ( (pvNormal == NULL) && (nFlags & meshAutoSmooth) )
	{
		SmoothTriangleMesh( pMesh ) ;
	}
	CommitPrimitiveVertices( pMesh ) ;
	//
	return	err ;
}

// トライアングルリストを追加する
//////////////////////////////////////////////////////////////////////////////
E3D_PRIMITIVE_POLYGON *
	E3DPolygonModel::AddTriangleListPrimitive
		( E3D_SURFACE_ATTRIBUTE * pSurfAttr,
			int nTriangleCount, int nFlags,
			PCE3D_VECTOR4 pvVertex, PCE3D_VECTOR4 pvNormal,
			PCE3D_VECTOR_2D pvUVMap, PCE3D_COLOR pvColor )
{
	//
	// 頂点数・ポリゴン数計算
	//
	const int	iBaseVertex = m_nVertexCount ;
	const int	iBaseNormal = m_nNormalCount ;
	const int	nVertexCount = nTriangleCount * 3 ;
	const int	nPolygonCount = nTriangleCount ;
	if ( (nVertexCount <= 0) || (nPolygonCount <= 0) )
	{
		return	NULL ;
	}
	ConvertAllPrimitiveAddressToIndex() ;
	AllocateVertexBuffer( iBaseVertex + nVertexCount ) ;
	AllocateNormalBuffer( iBaseNormal + nVertexCount ) ;
	ConvertAllPrimitiveIndexToAddress() ;
	//
	// プリミティブバッファ確保
	//
	const DWORD	dwHeaderSize =
		(0x10 + sizeof(E3D_PRIMITIVE_MESH))
			+ (nVertexCount - 1) * sizeof(E3D_VECTOR_2D) ;
	const DWORD	dwMeshListSize =
		sizeof(E3D_PRIMITIVE_MESH_LIST)
			+ (nPolygonCount - 1) * sizeof(E3D_PRIMITIVE_MESH_POLY) ;
	//
	E3D_PRIMITIVE_POLYGON *	pPrimitive =
		(E3D_PRIMITIVE_POLYGON*)
			::eslHeapAllocate
				( NULL, dwHeaderSize + dwMeshListSize, ESL_HEAP_ZERO_INIT ) ;
	m_lstPrimitives.Add( pPrimitive ) ;
	//
	// ヘッダ設定
	//
	pPrimitive->dwTypeFlag = E3D_MESH_POLYGON ;
	if ( (pvNormal != NULL) || (nFlags & meshAutoSmooth) )
	{
		pPrimitive->dwTypeFlag |= E3D_SMOOTH_POLYGON ;
	}
	if ( pvUVMap != NULL )
	{
		pPrimitive->dwTypeFlag |= E3D_TEXTURE_POLYGON ;
	}
	else if ( pvColor != NULL )
	{
		pPrimitive->dwTypeFlag |= E3D_VERTEX_COLOR_POLYGON ;
	}
	pPrimitive->pSurfaceAttr = pSurfAttr ;
	pPrimitive->dwVertexCount = nVertexCount ;
	pPrimitive->dwDataSize = dwHeaderSize + dwMeshListSize - 0x10 ;
	//
	pPrimitive->mesh.vertices = m_pVertexesBuf + iBaseVertex ;
	pPrimitive->mesh.normals = m_pNormalsBuf + iBaseNormal ;
	//
	// ポリゴンリスト設定
	//
	E3D_PRIMITIVE_MESH_LIST *	ppmlMesh =
		(E3D_PRIMITIVE_MESH_LIST*)
			&(pPrimitive->mesh.uv_map[nVertexCount]) ;
	//
	ppmlMesh->dwMeshBytes = dwMeshListSize ;
	ppmlMesh->dwPolyCount = nPolygonCount ;
	//
	for ( int i = 0, j = 0; i < nPolygonCount; i ++, j += 3 )
	{
		E3D_PRIMITIVE_MESH_POLY *
			ppmpMesh = &(ppmlMesh->mpEntries[i]) ;
		ppmpMesh->dwVertexCount = 3 ;
		ppmpMesh->dwIndex[0] = j ;
		ppmpMesh->dwIndex[1] = j + 1 ;
		ppmpMesh->dwIndex[2] = j + 2 ;
	}
	//
	// 情報設定
	//
	ModifyTriangleListPrimitive
		( pPrimitive, nTriangleCount,
			nFlags, pvVertex, pvNormal, pvUVMap, pvColor ) ;
	//
	return	pPrimitive ;
}

// トライアングルリストを変更する
//////////////////////////////////////////////////////////////////////////////
ESLError E3DPolygonModel::ModifyTriangleListPrimitive
		( E3D_PRIMITIVE_POLYGON * pMesh,
			int nTriangleCount, int nFlags,
			PCE3D_VECTOR4 pvVertex, PCE3D_VECTOR4 pvNormal,
			PCE3D_VECTOR_2D pvUVMap, PCE3D_COLOR pvColor )
{
	//
	// 頂点数・ポリゴン数計算
	//
	const int	iBaseVertex = m_nVertexCount ;
	const int	iBaseNormal = m_nNormalCount ;
	const int	nVertexCount = nTriangleCount * 3 ;
	const int	nPolygonCount = nTriangleCount ;
	if ( (nVertexCount <= 0) || (nPolygonCount <= 0) )
	{
		return	eslErrFailed ;
	}
	//
	// 頂点複製
	//
	ESLError	err =
		ModifyMeshPrimitive
			( pMesh, nVertexCount,
					pvVertex, pvNormal, pvUVMap, pvColor ) ;
	//
	if ( (pvNormal == NULL) && (nFlags & meshAutoSmooth) )
	{
		SmoothTriangleMesh( pMesh ) ;
	}
	CommitPrimitiveVertices( pMesh ) ;
	//
	return	err ;
}

// 頂点情報を変更する
//////////////////////////////////////////////////////////////////////////////
ESLError E3DPolygonModel::ModifyMeshPrimitive
		( E3D_PRIMITIVE_POLYGON * pMesh,
			int nVertexCount,
			PCE3D_VECTOR4 pvVertex, PCE3D_VECTOR4 pvNormal,
			PCE3D_VECTOR_2D pvUVMap, PCE3D_COLOR pvColor )
{
	if ( (pMesh == NULL)
		|| !(pMesh->dwTypeFlag & E3D_MESH_POLYGON)
		|| (pMesh->dwVertexCount != (DWORD) nVertexCount) )
	{
		return	eslErrFailed ;
	}
	//
	// 頂点複製
	//
	if ( (pvVertex != NULL) && (pMesh->mesh.vertices != NULL) )
	{
		::eslMoveMemory
			( pMesh->mesh.vertices,
				pvVertex, nVertexCount * sizeof(E3D_VECTOR4) ) ;
	}
	if ( (pvNormal != NULL)
		&& (pMesh->dwTypeFlag & E3D_SMOOTH_POLYGON)
		&& (pMesh->mesh.normals != NULL) )
	{
		::eslMoveMemory
			( pMesh->mesh.normals,
				pvNormal, nVertexCount * sizeof(E3D_VECTOR4) ) ;
	}
	if ( (pvUVMap != NULL)
		&& !(pMesh->dwTypeFlag & E3D_VERTEX_COLOR_POLYGON) )
	{
		::eslMoveMemory
			( pMesh->mesh.uv_map,
				pvUVMap, nVertexCount * sizeof(E3D_VECTOR_2D) ) ;
	}
	if ( (pvColor != NULL)
		&& (pMesh->dwTypeFlag & E3D_VERTEX_COLOR_POLYGON) )
	{
		::eslMoveMemory
			( pMesh->mesh.color,
				pvColor, nVertexCount * sizeof(E3D_COLOR) ) ;
	}
	return	eslErrSuccess ;
}

// 三角ポリゴンメッシュの法線を自動生成する
//////////////////////////////////////////////////////////////////////////////
ESLError E3DPolygonModel::SmoothTriangleMesh( E3D_PRIMITIVE_POLYGON * pMesh )
{
	if ( (pMesh == NULL)
		|| !(pMesh->dwTypeFlag & E3D_MESH_POLYGON) )
	{
		return	eslErrFailed ;
	}
	PCE3D_VECTOR4	pvVertices = pMesh->mesh.vertices ;
	PE3D_VECTOR4	pvNormals = pMesh->mesh.normals ;
	const DWORD		dwVertexCount = pMesh->dwVertexCount ;
	//
	E3D_PRIMITIVE_MESH_LIST *
					ppmlMesh = (E3D_PRIMITIVE_MESH_LIST*)
								&(pMesh->mesh.uv_map[dwVertexCount]) ;
	//
	const DWORD		dwPolyCount = ppmlMesh->dwPolyCount ;
	E3D_PRIMITIVE_MESH_POLY *
					ppmpNext = ppmlMesh->mpEntries ;
	//
	::eslFillMemory
		( pvNormals, 0, dwVertexCount * sizeof(E3D_VECTOR4) ) ;
	//
	DWORD	i ;
	for ( i = 0; i < dwPolyCount; i ++ )
	{
		E3DVector	v0 = pvVertices[ppmpNext->dwIndex[0]] ;
		E3DVector	v1 = pvVertices[ppmpNext->dwIndex[1]] ;
		E3DVector	v2 = pvVertices[ppmpNext->dwIndex[2]] ;
		v1 -= v0 ;
		v2 -= v0 ;
		v1.ExteriorProduct( v2 ) ;
		v1.Normalize() ;
		//
		const DWORD	dwPolyVertices = ppmpNext->dwVertexCount ;
		E3DVector4	vNormal = v1 ;
		for ( DWORD j = 0; j < dwPolyVertices; j ++ )
		{
			pvNormals[ppmpNext->dwIndex[j]] += vNormal ;
		}
		//
		ppmpNext =
			(E3D_PRIMITIVE_MESH_POLY*)
				&(ppmpNext->dwIndex[dwPolyVertices]) ;
	}
	for ( i = 0; i < dwVertexCount; i ++ )
	{
		pvNormals[i].Normalize() ;
	}
	return	eslErrSuccess ;
}
