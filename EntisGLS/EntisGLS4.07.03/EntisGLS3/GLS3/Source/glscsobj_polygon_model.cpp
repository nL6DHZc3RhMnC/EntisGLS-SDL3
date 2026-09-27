
/*****************************************************************************
                          Entis Graphic Library
 -----------------------------------------------------------------------------
    Copyright (C) 2004-2012 Leshade Entis, Entis-soft. Al rights reserved.
 *****************************************************************************/


#include <gls.h>
#include <math.h>


//////////////////////////////////////////////////////////////////////////////
// モデルデータ・スクリプト・インターフェース
//////////////////////////////////////////////////////////////////////////////

// メンバ関数ポインタ
//////////////////////////////////////////////////////////////////////////////
ECSStrTagArray *	ECSPolygonModel::m_staFuncName = NULL ;
const wchar_t *		ECSPolygonModel::m_pwszFuncName[16] =
{
	L"LoadModel", L"DeleteModel",
	L"CreateImagePrimitive", L"CreateImagePolygon",
	L"AttachImagePolygon", L"TransformAccordingAsBone",
	L"FindBoneAs",
	L"RegisterSurfaceAttribute", L"RegisterTextureAttribute",
	L"AddGridMeshPrimitive", L"ModifyGridMeshPrimitive",
	L"AddTriangleStripPrimitive", L"ModifyTriangleStripPrimitive",
	L"AddTriangleListPrimitive", L"ModifyTriangleListPrimitive",
	NULL
} ;
const ECSModelJoint::PFUNC_CALL	ECSPolygonModel::m_pfnCallFunc[15] =
{
	&ECSPolygonModel::Call_LoadModel,
	&ECSPolygonModel::Call_DeleteModel,
	&ECSPolygonModel::Call_CreateImagePrimitive,
	&ECSPolygonModel::Call_CreateImagePolygon,
	&ECSPolygonModel::Call_AttachImagePolygon,
	&ECSPolygonModel::Call_TransformAccordingAsBone,
	&ECSPolygonModel::Call_FindBoneAs,
	&ECSPolygonModel::Call_RegisterSurfaceAttribute,
	&ECSPolygonModel::Call_RegisterTextureAttribute,
	&ECSPolygonModel::Call_AddGridMeshPrimitive,
	&ECSPolygonModel::Call_ModifyGridMeshPrimitive,
	&ECSPolygonModel::Call_AddTriangleStripPrimitive,
	&ECSPolygonModel::Call_ModifyTriangleStripPrimitive,
	&ECSPolygonModel::Call_AddTriangleListPrimitive,
	&ECSPolygonModel::Call_ModifyTriangleListPrimitive,
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO2( ECSPolygonModel, ECSObject, E3DBonePolygonModel )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ECSPolygonModel::ECSPolygonModel( void )
{
	m_dwPrimitiveType = 0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
ECSPolygonModel::~ECSPolygonModel( void )
{
}

// モデルデータを読み込む
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPolygonModel::ReadModel( ESLFileObject & file )
{
	ESLError	err = E3DBonePolygonModel::ReadModel( file ) ;
	if ( err )
	{
		return	err ;
	}
	m_staJoint.RemoveAll( ) ;
	for ( int i = 0; i < (int) m_lstBones.GetSize(); i ++ )
	{
		E3DBoneJoint *	pBone = m_lstBones.GetAt( i ) ;
		if ( pBone == NULL )
		{
			continue ;
		}
		RegisterBoneJoint( pBone ) ;
	}
	return	err ;
}

// ボーンジョイントを列挙し、登録する
//////////////////////////////////////////////////////////////////////////////
void ECSPolygonModel::RegisterBoneJoint( E3DBoneJoint * pBone )
{
	ECSModelJoint *	pJoint = new ECSModelJoint ;
	pJoint->m_pRefJoint = pBone ;
	m_staJoint.Add( ECSWideString(pBone->m_wstrName), pJoint ) ;
	//
	for ( int i = 0; i < (int) pBone->JointList().GetSize(); i ++ )
	{
		E3DBoneJoint *	pSubBone =
			ESLTypeCast<E3DBoneJoint>( pBone->JointList().GetAt( i ) ) ;
		if ( pSubBone == NULL )
		{
			continue ;
		}
		RegisterBoneJoint( pSubBone ) ;
	}
}

// モデルデータを削除する
//////////////////////////////////////////////////////////////////////////////
void ECSPolygonModel::DeleteContents( void )
{
	E3DBonePolygonModel::DeleteContents( ) ;
	m_dwPrimitiveType = 0 ;
	m_staJoint.RemoveAll( ) ;
	m_wstrFileName.FreeString( ) ;
	m_refImage.SetReference( NULL ) ;
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
ESLFileObject * ECSPolygonModel::OpenResourceFile
	( const wchar_t * pwszFilePath, ECSContext * pContext )
{
	if ( pContext != NULL )
	{
		return	pContext->OpenFileOnScript( pwszFilePath ) ;
	}
	ERawFile *	pfile = new ERawFile ;
	if ( pfile->Open
		( EString(pwszFilePath),
			ESLFileObject::modeRead | ESLFileObject::shareRead ) )
	{
		delete	pfile ;
		return	NULL ;
	}
	return	pfile ;
}

// オブジェクトの型名を取得する
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSPolygonModel::GetTypeName( void ) const
{
	return	L"PolygonModel" ;
}

// オブジェクトを複製
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSPolygonModel::Duplicate( void )
{
	return	new ECSPolygonModel ;
}

// オブジェクトを代入
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPolygonModel::Move( ECSContext & context, ECSObject * obj )
{
	return	ESLErrorMsg( "PolygonModel への定義されていない代入です。" ) ;
}

// 単項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPolygonModel::UnaryOperate
	( ECSContext & context, CSUnaryOperatorType csuopType )
{
	return	ESLErrorMsg( "PolygonModel の定義されていない演算子です。" ) ;
}

// 二項演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPolygonModel::Operate
	( ECSContext & context, CSOperatorType csopType, ECSObject * obj )
{
	return	ESLErrorMsg( "PolygonModel の定義されていない演算子です。" ) ;
}

// 比較演算子
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPolygonModel::Compare
	( ECSContext & context, int & nResult,
		CSCompareType cscpType, ECSObject & obj )
{
	return	ESLErrorMsg( "PolygonModel の定義されていない比較です。" ) ;
}

// メンバ変数取得
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSPolygonModel::GetVariableAt( int nIndex )
{
	return	NULL ;
}

// メンバ変数設定
//////////////////////////////////////////////////////////////////////////////
ECSObject * ECSPolygonModel::SetVariableAt( int nIndex, ECSObject * obj )
{
	return	NULL ;
}

// メンバ関数インデックス取得
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPolygonModel::GetFunction
	( ECSContext & context, int & nIndex, const wchar_t * pwszName )
{
	nIndex = m_staFuncName->FindIndex( pwszName ) ;
	if ( nIndex < 0 )
	{
		return	ESLErrorMsg
			( "定義されていない PolygonModel の"
				"メンバ関数を呼び出そうとしました。" ) ;
	}
	return	eslErrSuccess ;
}

// メンバ関数呼び出し
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPolygonModel::CallFunction
	( ECSContext & context,
		int nIndex, ECSObjArray<ECSObject> & lstArg )
{
	if ( (unsigned int) nIndex >= m_staFuncName->GetSize() )
	{
		return	ESLErrorMsg
			( "定義されていない PolygonModel の"
				"メンバ関数を呼び出そうとしています。" ) ;
	}
	return	(this->*m_pfnCallFunc[nIndex])( context, lstArg ) ;
}

// 全てのメンバ変数にインデックスを振る
//////////////////////////////////////////////////////////////////////////////
void ECSPolygonModel::IndexAllMember( void )
{
	m_refImage.IndexAllMember( ) ;
}

// 全てのメンバ変数の参照を解決する
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPolygonModel::CommitAllReference( ECSContext & context )
{
	ESLError	err ;
	err = m_refImage.CommitAllReference( context ) ;
	if ( err )
	{
//		return	err ;
	}
	ECSResource *	pRsrc = ESLTypeCast<ECSResource>( m_refImage.m_pRef ) ;
	if ( (m_dwPrimitiveType != 0) && (pRsrc != NULL) )
	{
		PEGL_IMAGE_INFO	pImage = NULL ;
		EGLAnimation *	pAnime = pRsrc->GetImage( ) ;
		if ( pAnime != NULL )
		{
			pImage = pAnime->GetFrameAt( m_nFrameNum ) ;
		}
		else
		{
			ECSSprite *	pSprite =
				ESLTypeCast<ECSSprite>( pRsrc ) ;
			if ( pSprite != NULL )
			{
				pImage = pSprite->GetInfo( ) ;
			}
		}
		if ( pImage != NULL )
		{
			int				nFrameNum = m_nFrameNum ;
			EGL_RECT		rectView = m_rectView ;
			E3D_VECTOR_2D	vCenter = m_vCenter ;
			E3D_VECTOR_2D	vEnlarge = m_vEnlarge ;
			E3D_SURFACE_ATTRIBUTE	sfAttribute = m_sfAttribute ;
			if ( m_dwPrimitiveType == E3D_IMAGE_PRIMITIVE )
			{
				CreateImagePrimitive
					( pImage, &m_rectView, &m_vCenter, &m_vEnlarge ) ;
				m_dwPrimitiveType = E3D_IMAGE_PRIMITIVE ;
			}
			else
			{
				m_sfAttribute.txmap.pTextureImage = pImage ;
				m_sfAttribute.txmap.pSmallImage = pImage ;
				CreateImagePolygon
					( pImage, &m_rectView, &m_vCenter, &m_sfAttribute ) ;
				m_dwPrimitiveType = E3D_TEXTURE_POLYGON ;
			}
			m_refImage.SetReference( pRsrc, &context ) ;
			m_nFrameNum = nFrameNum ;
			m_rectView = rectView ;
			m_vCenter = vCenter ;
			m_vEnlarge = vEnlarge ;
			m_sfAttribute = sfAttribute ;
		}
	}
	return	eslErrSuccess ;
}

// データを保存
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPolygonModel::Save( ESLFileObject & file, ECSContext & context )
{
	DWORD	dwLength ;
	dwLength = m_wstrFileName.GetLength( ) ;
	file.Write( &dwLength, sizeof(DWORD) ) ;
	if ( dwLength > 0 )
	{
		file.Write( m_wstrFileName.CharPtr(), dwLength * sizeof(wchar_t) ) ;
	}
	ESLError	err ;
	file.Write( &m_dwPrimitiveType, sizeof(m_dwPrimitiveType) ) ;
	err = m_refImage.Save( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	file.Write( &m_nFrameNum, sizeof(m_nFrameNum) ) ;
	file.Write( &m_rectView, sizeof(m_rectView) ) ;
	file.Write( &m_vCenter, sizeof(m_vCenter) ) ;
	file.Write( &m_vEnlarge, sizeof(m_vEnlarge) ) ;
	file.Write( &m_sfAttribute, sizeof(m_sfAttribute) ) ;
	return	eslErrSuccess ;
}

// データを復元
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPolygonModel::Load( ESLFileObject & file, ECSContext & context )
{
	DWORD	dwLength = 0 ;
	if ( file.Read( &dwLength, sizeof(DWORD) ) < sizeof(DWORD) )
	{
		return	eslErrGeneral ;
	}
	ECSWideString	wstrFileName ;
	file.Read
		( wstrFileName.GetBuffer(dwLength), dwLength * sizeof(wchar_t) ) ;
	wstrFileName.ReleaseBuffer( ) ;
	m_wstrFileName.FreeString( ) ;
	if ( !wstrFileName.IsEmpty() )
	{
		ESLFileObject *	pfile = OpenResourceFile( wstrFileName, &context ) ;
		if ( pfile != NULL )
		{
			if ( ReadModel( *pfile ) )
			{
				m_wstrFileName = wstrFileName ;
			}
			delete	pfile ;
		}
	}
	//
	ESLError	err ;
	file.Read( &m_dwPrimitiveType, sizeof(m_dwPrimitiveType) ) ;
	err = m_refImage.Load( file, context ) ;
	if ( err )
	{
		return	err ;
	}
	file.Read( &m_nFrameNum, sizeof(m_nFrameNum) ) ;
	file.Read( &m_rectView, sizeof(m_rectView) ) ;
	file.Read( &m_vCenter, sizeof(m_vCenter) ) ;
	file.Read( &m_vEnlarge, sizeof(m_vEnlarge) ) ;
	file.Read( &m_sfAttribute, sizeof(m_sfAttribute) ) ;
	return	eslErrSuccess ;
}

// データをダンプ
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPolygonModel::DumpObject
	( EStreamBuffer & buf, int nIndent, ECSContext & context )
{
	EString	strDump ;
	if ( !m_wstrFileName.IsEmpty() )
	{
		strDump = "モデルファイル \"" + EString(m_wstrFileName) + "\"" ;
		buf.Write( strDump.CharPtr(), strDump.GetLength() ) ;
	}
	else if ( m_refImage.m_pRef->IsValidObject() )
	{
		strDump = "画像モデル : " ;
		buf.Write( strDump.CharPtr(), strDump.GetLength() ) ;
		return	m_refImage.DumpObject( buf, nIndent, context ) ;
	}
	return	eslErrSuccess ;
}

// メンバ関数 : Integer LoadModel( String strFileName )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPolygonModel::Call_LoadModel
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrFileName ;
	err = context.GetArgumentAsStr( wstrFileName, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	//
	ESLFileObject *	pfile = OpenResourceFile( wstrFileName, &context ) ;
	if ( pfile == NULL )
	{
		return	context.PushObject( new ECSInteger( eslErrGeneral ) ) ;
	}
	err = ReadModel( *pfile ) ;
	if ( !err )
	{
		m_wstrFileName = wstrFileName ;
	}
	delete	pfile ;
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : DeleteModel()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPolygonModel::Call_DeleteModel
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	DeleteContents( ) ;
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : CreateImagePrimitive
//	( Reference rImage, Integer iFrame := 0
//		[, Rect rectView[, Vector2D vCenter[, Vector2D vEnlarge]]] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPolygonModel::Call_CreateImagePrimitive
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2, 6 ) ;
	if ( err )
		return	err ;
	//
	//
	// 画像取得
	//
	PEGL_IMAGE_INFO	pImage = NULL ;
	int	iFrame = 0 ;
	ECSResource *	pRsrc =
		ESLTypeCast<ECSResource>
			( context.GetArgumentObjectAs( lstArg, 1, L"Resource" ) ) ;
	if ( pRsrc != NULL )
	{
		EGLAnimation *	pAnime = pRsrc->GetImage( ) ;
		if ( pAnime != NULL )
		{
			err = context.GetArgumentAsInt( iFrame, lstArg, 2, 0 ) ;
			if ( err )
			{
				return	err ;
			}
			pImage = pAnime->GetFrameAt( iFrame ) ;
		}
		else
		{
			ECSSprite *	pSprite =
				ESLTypeCast<ECSSprite>( pRsrc ) ;
			if ( pSprite != NULL )
			{
				pImage = pSprite->GetInfo( ) ;
			}
		}
	}
	if ( pImage == NULL )
	{
		return	ESLErrorMsg( "画像が指定されていません。" ) ;
	}
	//
	// 矩形取得
	//
	ECSStructure *	pStruct ;
	EGL_RECT	rectView ;
	rectView.left = 0 ;
	rectView.top = 0 ;
	rectView.right = pImage->dwImageWidth - 1 ;
	rectView.bottom = pImage->dwImageHeight - 1 ;
	pStruct =
		ESLTypeCast<ECSStructure>
			( context.GetArgumentObjectAs( lstArg, 3, L"Rect" ) ) ;
	if ( pStruct != NULL )
	{
		rectView.left =
			pStruct->GetMemberAsInt( L"left", rectView.left ) ;
		rectView.top =
			pStruct->GetMemberAsInt( L"top", rectView.top ) ;
		rectView.right =
			pStruct->GetMemberAsInt( L"right", rectView.right ) ;
		rectView.bottom =
			pStruct->GetMemberAsInt( L"bottom", rectView.bottom ) ;
	}
	//
	// 中心座標取得
	//
	E3D_VECTOR_2D	vCenter = { 0.0F, 0.0F } ;
	vCenter.x = (REAL32) ((rectView.left + rectView.right) * 0.5) ;
	vCenter.y = (REAL32) ((rectView.top + rectView.bottom) * 0.5) ;
	pStruct =
		ESLTypeCast<ECSStructure>
			( context.GetArgumentObjectAs( lstArg, 4, L"Vector2D" ) ) ;
	if ( pStruct != NULL )
	{
		vCenter.x = (REAL32) pStruct->GetMemberAsReal( L"x", vCenter.x ) ;
		vCenter.y = (REAL32) pStruct->GetMemberAsReal( L"y", vCenter.y ) ;
	}
	//
	// 拡大率取得
	//
	E3D_VECTOR_2D	vEnlarge = { 1.0F, 1.0F } ;
	pStruct =
		ESLTypeCast<ECSStructure>
			( context.GetArgumentObjectAs( lstArg, 5, L"Vector2D" ) ) ;
	if ( pStruct != NULL )
	{
		vEnlarge.x = (REAL32) pStruct->GetMemberAsReal( L"x", vEnlarge.x ) ;
		vEnlarge.y = (REAL32) pStruct->GetMemberAsReal( L"y", vEnlarge.y ) ;
	}
	//
	// 画像プリミティブ生成
	//
	CreateImagePrimitive( pImage, &rectView, &vCenter, &vEnlarge ) ;
	//
	m_dwPrimitiveType = E3D_IMAGE_PRIMITIVE ;
	m_refImage.SetReference( pRsrc ) ;
	m_nFrameNum = iFrame ;
	m_rectView = rectView ;
	m_vCenter = vCenter ;
	m_vEnlarge = vEnlarge ;
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : CreateImagePolygon
//	( Reference rImage, Integer iFrame := 0
//		[, Rect rectView[, Vector2D vCenter[, SurfaceAttribute sfAttr
//						[, Real rFogDeepness[, Integer rgbFogColor]]]]] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPolygonModel::Call_CreateImagePolygon
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2, 8 ) ;
	if ( err )
		return	err ;
	//
	// 画像取得
	//
	PEGL_IMAGE_INFO	pImage = NULL ;
	int	iFrame = 0 ;
	ECSResource *	pRsrc =
		ESLTypeCast<ECSResource>
			( context.GetArgumentObjectAs( lstArg, 1, L"Resource" ) ) ;
	if ( pRsrc != NULL )
	{
		EGLAnimation *	pAnime = pRsrc->GetImage( ) ;
		if ( pAnime != NULL )
		{
			err = context.GetArgumentAsInt( iFrame, lstArg, 2, 0 ) ;
			if ( err )
			{
				return	err ;
			}
			pImage = pAnime->GetFrameAt( iFrame ) ;
		}
		else
		{
			ECSSprite *	pSprite =
				ESLTypeCast<ECSSprite>( pRsrc ) ;
			if ( pSprite != NULL )
			{
				pImage = pSprite->GetInfo( ) ;
			}
		}
	}
	if ( pImage == NULL )
	{
		return	ESLErrorMsg( "画像が指定されていません。" ) ;
	}
	//
	// 矩形取得
	//
	ECSStructure *	pStruct ;
	EGL_RECT	rectView ;
	rectView.left = 0 ;
	rectView.top = 0 ;
	rectView.right = pImage->dwImageWidth - 1 ;
	rectView.bottom = pImage->dwImageHeight - 1 ;
	pStruct =
		ESLTypeCast<ECSStructure>
			( context.GetArgumentObjectAs( lstArg, 3, L"Rect" ) ) ;
	if ( pStruct != NULL )
	{
		rectView.left =
			pStruct->GetMemberAsInt( L"left", rectView.left ) ;
		rectView.top =
			pStruct->GetMemberAsInt( L"top", rectView.top ) ;
		rectView.right =
			pStruct->GetMemberAsInt( L"right", rectView.right ) ;
		rectView.bottom =
			pStruct->GetMemberAsInt( L"bottom", rectView.bottom ) ;
	}
	//
	// 中心座標取得
	//
	E3D_VECTOR_2D	vCenter = { 0.0F, 0.0F } ;
	if ( (rectView.left != 0x80000000)
		|| (rectView.top != 0x80000000)
		|| (rectView.right != 0x7FFFFFFF)
		|| (rectView.bottom != 0x7FFFFFFF) )
	{
		vCenter.x = (REAL32) ((rectView.left + rectView.right) * 0.5) ;
		vCenter.y = (REAL32) ((rectView.top + rectView.bottom) * 0.5) ;
	}
	pStruct =
		ESLTypeCast<ECSStructure>
			( context.GetArgumentObjectAs( lstArg, 4, L"Vector2D" ) ) ;
	if ( pStruct != NULL )
	{
		vCenter.x = (REAL32) pStruct->GetMemberAsReal( L"x", vCenter.x ) ;
		vCenter.y = (REAL32) pStruct->GetMemberAsReal( L"y", vCenter.y ) ;
	}
	//
	// 表面属性取得
	//
	E3D_SURFACE_ATTRIBUTE	sfAttr ;
	::eslFillMemory( &sfAttr, 0, sizeof(sfAttr) ) ;
	sfAttr.dwShadingFlags =
		E3DSAF_NO_SHADING | E3DSAF_TEXTURE_SMOOTH | E3DSAF_TEXTURE_MAPPING ;
	sfAttr.rgbaColor.rgbMul.dwPixelCode = 0x00FFFFFF ;
	sfAttr.txmap.pTextureImage = pImage ;
	sfAttr.txmap.pSmallImage = pImage ;
	pStruct =
		ESLTypeCast<ECSStructure>
			( context.GetArgumentObjectAs( lstArg, 5, L"SurfaceAttribute" ) ) ;
	if ( pStruct != NULL )
	{
		sfAttr.dwShadingFlags |=
			pStruct->GetMemberAsInt
				( L"nShadingFlags", sfAttr.dwShadingFlags ) ;
		sfAttr.rgbaColor.rgbMul.dwPixelCode =
			pStruct->GetMemberAsInt( L"rgbColorMul", 0x00FFFFFF ) ;
		sfAttr.rgbaColor.rgbAdd.dwPixelCode =
			pStruct->GetMemberAsInt( L"rgbColorAdd", 0 ) ;
		sfAttr.nAmbient = pStruct->GetMemberAsInt( L"nAmbient", 0 ) ;
		sfAttr.nDiffusion = pStruct->GetMemberAsInt( L"nDiffusion", 0x100 ) ;
		sfAttr.nSpecular = pStruct->GetMemberAsInt( L"nSpecular", 0x80 ) ;
		sfAttr.nTransparency = pStruct->GetMemberAsInt( L"nTransparency", 0 ) ;
		sfAttr.nDeepness = pStruct->GetMemberAsInt( L"nDeepness", 0 ) ;
		sfAttr.rgbaShade.rgbMul.dwPixelCode =
			pStruct->GetMemberAsInt( L"rgbShadeMul", 0 ) ;
		sfAttr.rgbaShade.rgbAdd.dwPixelCode =
			pStruct->GetMemberAsInt( L"rgbShadeAdd", 0 ) ;
		sfAttr.nReflection = pStruct->GetMemberAsInt( L"nReflection", 0 ) ;
		sfAttr.nRefraction =
			(REAL32) pStruct->GetMemberAsReal( L"nRefraction", 0 ) ;
	}
	//
	// 擬似フォッグ用パラメータ取得
	//
	int		nFogColor ;
	EGL_PALETTE	rgbFogColor ;
	double	rFogDeepness ;
	err = context.GetArgumentAsReal( rFogDeepness, lstArg, 6, 0.0 ) ;
	if ( err )
		return	err ;
	err = context.GetArgumentAsInt( nFogColor, lstArg, 7, 0 ) ;
	if ( err )
		return	err ;
	rgbFogColor.dwPixelCode = nFogColor ;
	//
	// 画像モデル生成
	//
	CreateImagePolygon
		( pImage, &rectView, &vCenter, &sfAttr,
			(REAL32) rFogDeepness, &rgbFogColor ) ;
	//
	m_dwPrimitiveType = E3D_TEXTURE_POLYGON ;
	m_refImage.SetReference( pRsrc ) ;
	m_nFrameNum = iFrame ;
	m_rectView = rectView ;
	m_vCenter = vCenter ;
	m_sfAttribute = sfAttr ;
	//
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : AttachImagePolygon
//		( Reference rImage, Integer iFrame := 0
//			[, Rect rectView[, Vector2D vCenter]] )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPolygonModel::Call_AttachImagePolygon
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2, 5 ) ;
	if ( err )
		return	err ;
	//
	// 画像取得
	//
	PEGL_IMAGE_INFO	pImage = NULL ;
	int	iFrame = 0 ;
	ECSResource *	pRsrc =
		ESLTypeCast<ECSResource>
			( context.GetArgumentObjectAs( lstArg, 1, L"Resource" ) ) ;
	if ( pRsrc != NULL )
	{
		EGLAnimation *	pAnime = pRsrc->GetImage( ) ;
		if ( pAnime != NULL )
		{
			err = context.GetArgumentAsInt( iFrame, lstArg, 2, 0 ) ;
			if ( err )
			{
				return	err ;
			}
			pImage = pAnime->GetFrameAt( iFrame ) ;
		}
		else
		{
			ECSSprite *	pSprite =
				ESLTypeCast<ECSSprite>( pRsrc ) ;
			if ( pSprite != NULL )
			{
				pImage = pSprite->GetInfo( ) ;
			}
		}
	}
	if ( pImage == NULL )
	{
		return	ESLErrorMsg( "画像が指定されていません。" ) ;
	}
	//
	// 矩形取得
	//
	ECSStructure *	pStruct ;
	EGL_RECT	rectView ;
	rectView.left = 0 ;
	rectView.top = 0 ;
	rectView.right = pImage->dwImageWidth - 1 ;
	rectView.bottom = pImage->dwImageHeight - 1 ;
	pStruct =
		ESLTypeCast<ECSStructure>
			( context.GetArgumentObjectAs( lstArg, 3, L"Rect" ) ) ;
	if ( pStruct != NULL )
	{
		rectView.left =
			pStruct->GetMemberAsInt( L"left", rectView.left ) ;
		rectView.top =
			pStruct->GetMemberAsInt( L"top", rectView.top ) ;
		rectView.right =
			pStruct->GetMemberAsInt( L"right", rectView.right ) ;
		rectView.bottom =
			pStruct->GetMemberAsInt( L"bottom", rectView.bottom ) ;
	}
	//
	// 中心座標取得
	//
	E3D_VECTOR_2D	vCenter = { 0.0F, 0.0F } ;
	vCenter.x = (REAL32) ((rectView.left + rectView.right) * 0.5) ;
	vCenter.y = (REAL32) ((rectView.top + rectView.bottom) * 0.5) ;
	pStruct =
		ESLTypeCast<ECSStructure>
			( context.GetArgumentObjectAs( lstArg, 4, L"Vector2D" ) ) ;
	if ( pStruct != NULL )
	{
		vCenter.x = (REAL32) pStruct->GetMemberAsReal( L"x", vCenter.x ) ;
		vCenter.y = (REAL32) pStruct->GetMemberAsReal( L"y", vCenter.y ) ;
	}
	//
	// 画像関連付け
	//
	err = AttachImagePolygon( pImage, &rectView, &vCenter ) ;
	if ( !err )
	{
		m_refImage.SetReference( pRsrc ) ;
		m_nFrameNum = iFrame ;
		m_rectView = rectView ;
		m_vCenter = vCenter ;
	}
	return	context.PushObject( new ECSInteger( err ) ) ;
}

// メンバ関数 : TransformAccordingAsBone()
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPolygonModel::Call_TransformAccordingAsBone
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 1 ) ;
	if ( err )
		return	err ;
	//
	for ( unsigned int i = 0; i < m_staJoint.GetSize(); i ++ )
	{
		ECSModelJoint *	pJoint = m_staJoint.GetObjectAt( i ) ;
		if ( pJoint == NULL )
		{
			continue ;
		}
		if ( pJoint->m_pRefJoint == NULL )
		{
			continue ;
		}
		pJoint->m_pRefJoint->Position().x =
				(REAL32) pJoint->m_pcsrPosition[0]->m_varReal ;
		pJoint->m_pRefJoint->Position().y =
				(REAL32) pJoint->m_pcsrPosition[1]->m_varReal ;
		pJoint->m_pRefJoint->Position().z =
				(REAL32) pJoint->m_pcsrPosition[2]->m_varReal ;
	}
	TransformAccordingAsBone( ) ;
	return	context.PushObject( new ECSInteger ) ;
}

// メンバ関数 : Reference FindBoneAs( String strBoneName )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPolygonModel::Call_FindBoneAs
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSWideString	wstrName ;
	err = context.GetArgumentAsStr( wstrName, lstArg, 1, NULL ) ;
	if ( err )
		return	err ;
	//
	return	context.PushObject
		( new ECSReference( m_staJoint.GetAs( wstrName ) ) ) ;
}

// メンバ関数 : String RegisterSurfaceAttribute( const SurfaceAttribute& sufattr )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPolygonModel::Call_RegisterSurfaceAttribute
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 2 ) ;
	if ( err )
		return	err ;
	//
	ECSStructureInterface *	pSufAttr =
		ESLTypeCast<ECSStructureInterface>
			( context.GetArgumentObjectAs( lstArg, 1, L"SurfaceAttribute" ) ) ;
	if ( pSufAttr == NULL )
	{
		return	ESLErrorMsg
			( "引数に SurfaceAttribute 構造体が指定されていません" ) ;
	}
	E3D_SURFACE_ATTRIBUTE	sufattr ;
	GetScriptSurfaceAttribute( sufattr, pSufAttr ) ;
	//
	E3D_SURFACE_ATTRIBUTE *	pAttr =
		RegisterSurfaceAttribute
			( sufattr.rgbaColor, sufattr.dwShadingFlags,
				sufattr.nAmbient, sufattr.nDiffusion,
				sufattr.nSpecular, sufattr.nSpecularSize,
				sufattr.nTransparency, sufattr.nDeepness,
				sufattr.nReflection, sufattr.nRefraction ) ;
	if ( pAttr == NULL )
	{
		return	context.PushObject( context.new_CSString() ) ;
	}
	pAttr->rgbaShade = sufattr.rgbaShade ;
	//
	EWideString	wstrName ;
	if ( SurfaceLibrary().GetAttributeName( wstrName, pAttr ) )
	{
		return	context.PushObject( context.new_CSString() ) ;
	}
	return	context.PushObject( context.new_CSString( wstrName ) ) ;
}

// メンバ関数 : String RegisterTextureAttribute
//		( Resource& rsTexture, const SurfaceAttribute& sufattr )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPolygonModel::Call_RegisterTextureAttribute
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 3 ) ;
	if ( err )
		return	err ;
	//
	ECSResource *	prsTexture =
		ESLTypeCast<ECSResource>
			( context.GetArgumentObjectAs( lstArg, 1, L"Resource" ) ) ;
	if ( prsTexture == NULL )
	{
		return	ESLErrorMsg
			( "引数に Resource が指定されていません" ) ;
	}
	EGLAnimation *	pImage = prsTexture->GetImage() ;
	PEGL_IMAGE_INFO	pSrcImage = NULL ;
	if ( pImage != NULL )
	{
		pSrcImage = *pImage ;
	}
	else
	{
		ECSSprite *	prsSprite = ESLTypeCast<ECSSprite>( prsTexture ) ;
		if ( prsSprite != NULL )
		{
			pSrcImage = prsSprite->GetInfo() ;
		}
	}
	if ( pSrcImage == NULL )
	{
		return	context.PushObject( context.new_CSString() ) ;
	}
	ECSStructureInterface *	pSufAttr =
		ESLTypeCast<ECSStructureInterface>
			( context.GetArgumentObjectAs( lstArg, 2, L"SurfaceAttribute" ) ) ;
	if ( pSufAttr == NULL )
	{
		return	ESLErrorMsg
			( "引数に SurfaceAttribute 構造体が指定されていません" ) ;
	}
	E3D_SURFACE_ATTRIBUTE	sufattr ;
	GetScriptSurfaceAttribute( sufattr, pSufAttr ) ;
	//
	EGLImage *	pTextureImage = new EGLImage ;
	pTextureImage->SetImageView( pSrcImage, NULL ) ;
	//
	E3D_SURFACE_ATTRIBUTE *	pAttr =
		RegisterTextureAttribute
			( *pTextureImage, sufattr.dwShadingFlags,
				sufattr.nAmbient, sufattr.nDiffusion,
				sufattr.nSpecular, sufattr.nSpecularSize,
				sufattr.nTransparency, sufattr.nDeepness,
				sufattr.nReflection, sufattr.nRefraction ) ;
	if ( pAttr == NULL )
	{
		delete	pTextureImage ;
		return	context.PushObject( context.new_CSString() ) ;
	}
	pAttr->rgbaShade = sufattr.rgbaShade ;
	//
	EWideString	wstrName ;
	if ( SurfaceLibrary().GetAttributeName( wstrName, pAttr ) )
	{
		delete	pTextureImage ;
		return	context.PushObject( context.new_CSString() ) ;
	}
	TextureLibrary().Add( wstrName, pTextureImage ) ;
	//
	return	context.PushObject( context.new_CSString( wstrName ) ) ;
}

// メンバ関数 : Integer AddGridMeshPrimitive
//	( String strAttrID,
//		Integer nMeshWidth, Integer nMeshHeight, Integer nFlags,
//		float[3]* pvVertex, float[3]* pvNormal := null,
//		float[2]* pvUVMap := null, uint32[2]* pvColor := null )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPolygonModel::Call_AddGridMeshPrimitive
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 6, 9 ) ;
	if ( err )
	{
		return	err ;
	}
	ECSWideString	wstrAttrID ;
	int				nMeshWidth, nMeshHeight, nFlags ;
	err = context.GetArgumentAsStr( wstrAttrID, lstArg, 1, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	E3D_SURFACE_ATTRIBUTE *	pSurfAttr =
			SurfaceLibrary().GetAttributeAs( wstrAttrID ) ;
	if ( pSurfAttr == NULL )
	{
		return	context.PushObject( context.new_CSInteger( -1 ) ) ;
	}
	err = context.GetArgumentAsInt( nMeshWidth, lstArg, 2, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsInt( nMeshHeight, lstArg, 3, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsInt( nFlags, lstArg, 4, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	GRID_MESH_INFO	gmi ;
	CalcGridMeshVertexCount( gmi, nMeshWidth, nMeshHeight, nFlags ) ;
	if ( (gmi.nVertexCount <= 0) || (gmi.nPolygonCount <= 0) )
	{
		return	context.PushObject( context.new_CSInteger( -1 ) ) ;
	}
	MeshVertexBuffer	mvbuf ;
	ECSObject *		pObjVertex = ECSObject::GetEntity( lstArg.GetAt(5) ) ;
	ECSObject *		pObjNormal = ECSObject::GetEntity( lstArg.GetAt(6) ) ;
	ECSObject *		pObjUVMap = ECSObject::GetEntity( lstArg.GetAt(7) ) ;
	ECSObject *		pObjColor = ECSObject::GetEntity( lstArg.GetAt(8) ) ;
	//
	GetScriptMeshVertexBuffer
		( mvbuf, pObjVertex, pObjNormal,
			pObjUVMap, pObjColor, gmi.nVertexCount ) ;
	//
	E3D_PRIMITIVE_POLYGON *	pMesh =
		AddGridMeshPrimitive
			( pSurfAttr, nMeshWidth, nMeshHeight, nFlags,
				mvbuf.pvVertex, mvbuf.pvNormal,
				mvbuf.pvUVMap, mvbuf.pvColor ) ;
	//
	ReleaseScriptMeshVertexBuffer
		( mvbuf, pObjVertex, pObjNormal,
			pObjUVMap, pObjColor, gmi.nVertexCount ) ;
	//
	return	context.PushObject
				( context.new_CSInteger( FindPrimitivePtr( pMesh ) ) ) ;
}

// メンバ関数 : Error ModifyGridMeshPrimitive
//	( Integer nPrimitiveID,
//		Integer nMeshWidth, Integer nMeshHeight, Integer nFlags,
//		float[3]* pvVertex, float[3]* pvNormal := null,
//		float[2]* pvUVMap := null, uint32[2]* pvColor := null )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPolygonModel::Call_ModifyGridMeshPrimitive
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 6, 9 ) ;
	if ( err )
	{
		return	err ;
	}
	int	nMesh, nMeshWidth, nMeshHeight, nFlags ;
	err = context.GetArgumentAsInt( nMesh, lstArg, 1, -1 ) ;
	if ( err )
	{
		return	err ;
	}
	E3D_PRIMITIVE_POLYGON * pGridMesh = GetPrimitiveAt( nMesh ) ;
	if ( pGridMesh == NULL )
	{
		return	context.PushObject
					( context.new_CSInteger( eslErrFailed ) ) ;
	}
	err = context.GetArgumentAsInt( nMeshWidth, lstArg, 2, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsInt( nMeshHeight, lstArg, 3, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsInt( nFlags, lstArg, 4, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	GRID_MESH_INFO	gmi ;
	CalcGridMeshVertexCount( gmi, nMeshWidth, nMeshHeight, nFlags ) ;
	if ( (gmi.nVertexCount <= 0) || (gmi.nPolygonCount <= 0) )
	{
		return	context.PushObject( context.new_CSInteger( -1 ) ) ;
	}
	MeshVertexBuffer	mvbuf ;
	ECSObject *		pObjVertex = ECSObject::GetEntity( lstArg.GetAt(5) ) ;
	ECSObject *		pObjNormal = ECSObject::GetEntity( lstArg.GetAt(6) ) ;
	ECSObject *		pObjUVMap = ECSObject::GetEntity( lstArg.GetAt(7) ) ;
	ECSObject *		pObjColor = ECSObject::GetEntity( lstArg.GetAt(8) ) ;
	//
	GetScriptMeshVertexBuffer
		( mvbuf, pObjVertex, pObjNormal,
			pObjUVMap, pObjColor, gmi.nVertexCount ) ;
	//
	err = ModifyGridMeshPrimitive
			( pGridMesh, nMeshWidth, nMeshHeight, nFlags,
				mvbuf.pvVertex, mvbuf.pvNormal,
				mvbuf.pvUVMap, mvbuf.pvColor ) ;
	//
	ReleaseScriptMeshVertexBuffer
		( mvbuf, pObjVertex, pObjNormal,
			pObjUVMap, pObjColor, gmi.nVertexCount ) ;
	//
	return	context.PushObject( context.new_CSInteger( err ) ) ;
}

// メンバ関数 : Integer AddTriangleStripPrimitive
//	( String strAttrID,
//		Integer nTriangleStripCount, Integer nFlags,
//		float[3]* pvVertex, float[3]* pvNormal := null,
//		float[2]* pvUVMap := null, uint32[2]* pvColor := null )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPolygonModel::Call_AddTriangleStripPrimitive
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 5, 8 ) ;
	if ( err )
	{
		return	err ;
	}
	ECSWideString	wstrAttrID ;
	int				nTriangleStripCount, nFlags ;
	err = context.GetArgumentAsStr( wstrAttrID, lstArg, 1, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	E3D_SURFACE_ATTRIBUTE *	pSurfAttr =
			SurfaceLibrary().GetAttributeAs( wstrAttrID ) ;
	if ( pSurfAttr == NULL )
	{
		return	context.PushObject( context.new_CSInteger( -1 ) ) ;
	}
	err = context.GetArgumentAsInt( nTriangleStripCount, lstArg, 2, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsInt( nFlags, lstArg, 3, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	MeshVertexBuffer	mvbuf ;
	ECSObject *		pObjVertex = ECSObject::GetEntity( lstArg.GetAt(4) ) ;
	ECSObject *		pObjNormal = ECSObject::GetEntity( lstArg.GetAt(5) ) ;
	ECSObject *		pObjUVMap = ECSObject::GetEntity( lstArg.GetAt(6) ) ;
	ECSObject *		pObjColor = ECSObject::GetEntity( lstArg.GetAt(7) ) ;
	//
	const int	nVertexCount = nTriangleStripCount + 2 ;
	GetScriptMeshVertexBuffer
		( mvbuf, pObjVertex, pObjNormal,
			pObjUVMap, pObjColor, nVertexCount ) ;
	//
	E3D_PRIMITIVE_POLYGON *	pMesh =
		AddTriangleStripPrimitive
			( pSurfAttr, nTriangleStripCount, nFlags,
				mvbuf.pvVertex, mvbuf.pvNormal,
				mvbuf.pvUVMap, mvbuf.pvColor ) ;
	//
	ReleaseScriptMeshVertexBuffer
		( mvbuf, pObjVertex, pObjNormal,
			pObjUVMap, pObjColor, nVertexCount ) ;
	//
	return	context.PushObject
				( context.new_CSInteger( FindPrimitivePtr( pMesh ) ) ) ;
}

// メンバ関数 : Error ModifyTriangleStripPrimitive
//	( Integer nPrimitiveID,
//		Integer nTriangleStripCount, Integer nFlags,
//		float[3]* pvVertex, float[3]* pvNormal := null,
//		float[2]* pvUVMap := null, uint32[2]* pvColor := null )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPolygonModel::Call_ModifyTriangleStripPrimitive
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 5, 8 ) ;
	if ( err )
	{
		return	err ;
	}
	int	nMesh, nTriangleStripCount, nFlags ;
	err = context.GetArgumentAsInt( nMesh, lstArg, 1, -1 ) ;
	if ( err )
	{
		return	err ;
	}
	E3D_PRIMITIVE_POLYGON * pGridMesh = GetPrimitiveAt( nMesh ) ;
	if ( pGridMesh == NULL )
	{
		return	context.PushObject
					( context.new_CSInteger( eslErrFailed ) ) ;
	}
	err = context.GetArgumentAsInt( nTriangleStripCount, lstArg, 2, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsInt( nFlags, lstArg, 3, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	MeshVertexBuffer	mvbuf ;
	ECSObject *		pObjVertex = ECSObject::GetEntity( lstArg.GetAt(4) ) ;
	ECSObject *		pObjNormal = ECSObject::GetEntity( lstArg.GetAt(5) ) ;
	ECSObject *		pObjUVMap = ECSObject::GetEntity( lstArg.GetAt(6) ) ;
	ECSObject *		pObjColor = ECSObject::GetEntity( lstArg.GetAt(7) ) ;
	//
	const int	nVertexCount = nTriangleStripCount + 2 ;
	GetScriptMeshVertexBuffer
		( mvbuf, pObjVertex, pObjNormal,
			pObjUVMap, pObjColor, nVertexCount ) ;
	//
	err = ModifyTriangleStripPrimitive
			( pGridMesh, nTriangleStripCount, nFlags,
				mvbuf.pvVertex, mvbuf.pvNormal,
				mvbuf.pvUVMap, mvbuf.pvColor ) ;
	//
	ReleaseScriptMeshVertexBuffer
		( mvbuf, pObjVertex, pObjNormal,
			pObjUVMap, pObjColor, nVertexCount ) ;
	//
	return	context.PushObject( context.new_CSInteger( err ) ) ;
}

// メンバ関数 : Integer AddTriangleListPrimitive
//	( String strAttrID,
//		Integer nTriangleCount, Integer nFlags,
//		float[3]* pvVertex, float[3]* pvNormal := null,
//		float[2]* pvUVMap := null, uint32[2]* pvColor := null )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPolygonModel::Call_AddTriangleListPrimitive
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 5, 8 ) ;
	if ( err )
	{
		return	err ;
	}
	ECSWideString	wstrAttrID ;
	int				nTriangleCount, nFlags ;
	err = context.GetArgumentAsStr( wstrAttrID, lstArg, 1, NULL ) ;
	if ( err )
	{
		return	err ;
	}
	E3D_SURFACE_ATTRIBUTE *	pSurfAttr =
			SurfaceLibrary().GetAttributeAs( wstrAttrID ) ;
	if ( pSurfAttr == NULL )
	{
		return	context.PushObject( context.new_CSInteger( -1 ) ) ;
	}
	err = context.GetArgumentAsInt( nTriangleCount, lstArg, 2, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsInt( nFlags, lstArg, 3, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	MeshVertexBuffer	mvbuf ;
	ECSObject *		pObjVertex = ECSObject::GetEntity( lstArg.GetAt(4) ) ;
	ECSObject *		pObjNormal = ECSObject::GetEntity( lstArg.GetAt(5) ) ;
	ECSObject *		pObjUVMap = ECSObject::GetEntity( lstArg.GetAt(6) ) ;
	ECSObject *		pObjColor = ECSObject::GetEntity( lstArg.GetAt(7) ) ;
	//
	const int	nVertexCount = nTriangleCount * 3 ;
	GetScriptMeshVertexBuffer
		( mvbuf, pObjVertex, pObjNormal,
			pObjUVMap, pObjColor, nVertexCount ) ;
	//
	E3D_PRIMITIVE_POLYGON *	pMesh =
		AddTriangleListPrimitive
			( pSurfAttr, nTriangleCount, nFlags,
				mvbuf.pvVertex, mvbuf.pvNormal,
				mvbuf.pvUVMap, mvbuf.pvColor ) ;
	//
	ReleaseScriptMeshVertexBuffer
		( mvbuf, pObjVertex, pObjNormal,
			pObjUVMap, pObjColor, nVertexCount ) ;
	//
	return	context.PushObject
				( context.new_CSInteger( FindPrimitivePtr( pMesh ) ) ) ;
}

// メンバ関数 : Error ModifyTriangleListPrimitive
//	( Integer nPrimitiveID,
//		Integer nTriangleCount, Integer nFlags,
//		float[3]* pvVertex, float[3]* pvNormal := null,
//		float[2]* pvUVMap := null, uint32[2]* pvColor := null )
//////////////////////////////////////////////////////////////////////////////
ESLError ECSPolygonModel::Call_ModifyTriangleListPrimitive
	( ECSContext & context, ECSObjArray<ECSObject> & lstArg )
{
	ESLError	err ;
	err = context.VerifyArgumentCount( lstArg, 5, 8 ) ;
	if ( err )
	{
		return	err ;
	}
	int	nMesh, nTriangleCount, nFlags ;
	err = context.GetArgumentAsInt( nMesh, lstArg, 1, -1 ) ;
	if ( err )
	{
		return	err ;
	}
	E3D_PRIMITIVE_POLYGON * pMesh = GetPrimitiveAt( nMesh ) ;
	if ( pMesh == NULL )
	{
		return	context.PushObject
					( context.new_CSInteger( eslErrFailed ) ) ;
	}
	err = context.GetArgumentAsInt( nTriangleCount, lstArg, 2, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	err = context.GetArgumentAsInt( nFlags, lstArg, 3, 0 ) ;
	if ( err )
	{
		return	err ;
	}
	MeshVertexBuffer	mvbuf ;
	ECSObject *		pObjVertex = ECSObject::GetEntity( lstArg.GetAt(4) ) ;
	ECSObject *		pObjNormal = ECSObject::GetEntity( lstArg.GetAt(5) ) ;
	ECSObject *		pObjUVMap = ECSObject::GetEntity( lstArg.GetAt(6) ) ;
	ECSObject *		pObjColor = ECSObject::GetEntity( lstArg.GetAt(7) ) ;
	//
	const int	nVertexCount = nTriangleCount * 3 ;
	GetScriptMeshVertexBuffer
		( mvbuf, pObjVertex, pObjNormal,
			pObjUVMap, pObjColor, nVertexCount ) ;
	//
	err = ModifyTriangleListPrimitive
			( pMesh, nTriangleCount, nFlags,
				mvbuf.pvVertex, mvbuf.pvNormal,
				mvbuf.pvUVMap, mvbuf.pvColor ) ;
	//
	ReleaseScriptMeshVertexBuffer
		( mvbuf, pObjVertex, pObjNormal,
			pObjUVMap, pObjColor, nVertexCount ) ;
	//
	return	context.PushObject( context.new_CSInteger( err ) ) ;
}

// スクリプト上 SurfaceAttribute 構造体を E3D_SURFACE_ATTRIBUTE に取得
//////////////////////////////////////////////////////////////////////////////
void ECSPolygonModel::GetScriptSurfaceAttribute
	( E3D_SURFACE_ATTRIBUTE & sufattr,
		ECSStructureInterface * pSufAttr )
{
	sufattr.dwShadingFlags =
		pSufAttr->GetMemberAsInt( L"nShadingFlags", 0 ) ;
	sufattr.rgbaColor.rgbMul.dwPixelCode =
		pSufAttr->GetMemberAsInt( L"rgbColorMul", 0 ) ;
	sufattr.rgbaColor.rgbAdd.dwPixelCode =
		pSufAttr->GetMemberAsInt( L"rgbColorAdd", 0 ) ;
	sufattr.nAmbient =
		pSufAttr->GetMemberAsInt( L"nAmbient", 0 ) ;
	sufattr.nDiffusion =
		pSufAttr->GetMemberAsInt( L"nDiffusion", 0x100 ) ;
	sufattr.nSpecular =
		pSufAttr->GetMemberAsInt( L"nSpecular", 0 ) ;
	sufattr.nSpecularSize =
		pSufAttr->GetMemberAsInt( L"nSpecularSize", 0x40 ) ;
	sufattr.nTransparency =
		pSufAttr->GetMemberAsInt( L"nTransparency", 0 ) ;
	sufattr.nDeepness =
		pSufAttr->GetMemberAsInt( L"nDeepness", 0 ) ;
	sufattr.rgbaShade.rgbMul.dwPixelCode =
		pSufAttr->GetMemberAsInt( L"rgbShadeMul", 0 ) ;
	sufattr.rgbaShade.rgbAdd.dwPixelCode =
		pSufAttr->GetMemberAsInt( L"rgbShadeAdd", 0 ) ;
	sufattr.nReflection =
		pSufAttr->GetMemberAsInt( L"nReflection", 0 ) ;
	sufattr.nRefraction =
		(REAL32) pSufAttr->GetMemberAsReal( L"nRefraction", 0 ) ;
}

void ECSPolygonModel::GetScriptMeshVertexBuffer
	( MeshVertexBuffer& mvbuf,
		ECSObject * pObjVertex, ECSObject * pObjNormal,
		ECSObject * pObjUVMap, ECSObject * pObjColor, int nVertexCount )
{
	mvbuf.pvVertex = NULL ;
	mvbuf.pvNormal = NULL ;
	mvbuf.pvUVMap = NULL ;
	mvbuf.pvColor = NULL ;
	//
	if ( pObjVertex != NULL )
	{
		const int	nBufSize = nVertexCount * (sizeof(REAL32) * 3) ;
		REAL32 *	pvSrcVertex =
			(REAL32 *) pObjVertex->GetBuffer( 0, nBufSize, false ) ;
		if ( pvSrcVertex != NULL )
		{
			PE3D_VECTOR4	pvDstVertex = mvbuf.pvVertex =
				(PE3D_VECTOR4) mvbuf.bufVertex.PutBuffer
								( nVertexCount * sizeof(E3D_VECTOR4) ) ;
			for ( int i = 0, j = 0; i < nVertexCount; i ++, j += 3 )
			{
				pvDstVertex[i].x = pvSrcVertex[j] ;
				pvDstVertex[i].y = pvSrcVertex[j + 1] ;
				pvDstVertex[i].z = pvSrcVertex[j + 2] ;
				pvDstVertex[i].d = 0 ;
			}
			pObjVertex->FlushBuffer( 0, nBufSize, pvSrcVertex, false ) ;
		}
	}
	if ( pObjNormal != NULL )
	{
		const int	nBufSize = nVertexCount * (sizeof(REAL32) * 3) ;
		REAL32 *	pvSrcNormal =
			(REAL32 *) pObjNormal->GetBuffer( 0, nBufSize, false ) ;
		if ( pvSrcNormal != NULL )
		{
			PE3D_VECTOR4	pvDstNormal = mvbuf.pvNormal =
				(PE3D_VECTOR4) mvbuf.bufNormal.PutBuffer
								( nVertexCount * sizeof(E3D_VECTOR4) ) ;
			for ( int i = 0, j = 0; i < nVertexCount; i ++, j += 3 )
			{
				pvDstNormal[i].x = pvSrcNormal[j] ;
				pvDstNormal[i].y = pvSrcNormal[j + 1] ;
				pvDstNormal[i].z = pvSrcNormal[j + 2] ;
				pvDstNormal[i].d = 0 ;
			}
			pObjNormal->FlushBuffer( 0, nBufSize, pvSrcNormal, false ) ;
		}
	}
	if ( pObjUVMap != NULL )
	{
		mvbuf.pvUVMap =
			(PE3D_VECTOR_2D)
				pObjUVMap->GetBuffer
					( 0, nVertexCount * sizeof(E3D_VECTOR_2D), false ) ;
	}
	if ( pObjColor != NULL )
	{
		mvbuf.pvColor =
			(PE3D_COLOR)
				pObjColor->GetBuffer
					( 0, nVertexCount * sizeof(E3D_COLOR), false ) ;
	}
}

void ECSPolygonModel::ReleaseScriptMeshVertexBuffer
	( MeshVertexBuffer& mvbuf,
		ECSObject * pObjVertex, ECSObject * pObjNormal,
		ECSObject * pObjUVMap, ECSObject * pObjColor, int nVertexCount )
{
	if ( (pObjUVMap != NULL) && (mvbuf.pvUVMap != NULL) )
	{
		pObjUVMap->FlushBuffer
			( 0, nVertexCount * sizeof(E3D_VECTOR_2D), mvbuf.pvUVMap, false ) ;
	}
	if ( (pObjColor != NULL) && (mvbuf.pvColor != NULL) )
	{
		pObjColor->FlushBuffer
			( 0, nVertexCount * sizeof(E3D_COLOR), mvbuf.pvColor, false ) ;
	}
}
