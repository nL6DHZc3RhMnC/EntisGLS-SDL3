
#include <sakuraglx/sakuraglx.h>
#include <sakura/ssys_smart_buffer.h>
#include <sakuragl/sgl2d/sgl_image_conversion.h>
#include <sakuraglx/sglx3d_render.h>
#include <sakuraglx/render/sglx_model_loader.h>
#include <sakuraglx/extra/sglx_model_gltf_importer.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// モデルファイル・抽象ローダー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DModelLoaderInterface, ESLObject )

// ファイル拡張子判定
//////////////////////////////////////////////////////////////////////////////
bool S3DModelLoaderInterface::IsMatchableFileExtension( const wchar_t * pszExt )
{
	return	false ;
}

// MIME 判定
//////////////////////////////////////////////////////////////////////////////
bool S3DModelLoaderInterface::IsMatchableMIMEType( const wchar_t * pszMIME )
{
	return	false ;
}

// モデルローダー生成
//////////////////////////////////////////////////////////////////////////////

const S3DModelLoaderInterface::FileExtensionMIMEPair
	S3DModelLoaderInterface::s_fileExeMimePairs[supportModelFileTypeCount] =
{
	// standardModelBinary (EntisGLS4 標準)
	{	standardModelBinary, { L"mdfx", nullptr }, L"application/x-mdfx"	},
	// standardModelXml  (EntisGLS4 標準)
	{	standardModelXml, { L"xmlmdf", nullptr }, L"application/x-xmlmdf"	},
	// legacyModelBinary (EntisGLS3 互換)
	{	legacyModelBinary, { L"mdf", nullptr }, L"application/x-mdf"	},
	// glTransmissionFormat (glTF バイナリ)
	{	glTransmissionFormat, { L"glb", L"vrm", nullptr }, L"modle/gltf+binary"	},
} ;

S3DModelLoaderInterface *
	S3DModelLoaderInterface::NewModelLoaderTypeAs( S3DModelLoaderInterface::SupportModelFileType type )
{
	switch ( type )
	{
	case	standardModelBinary:
		return	new S3DStdModelLoader ;

	case	standardModelXml:
		return	new S3DStdXMLModelLoader ;

	case	legacyModelBinary:
		return	new S3DModelMDFLoader ;

	case	glTransmissionFormat:
		return	new S3DModelGLTFImporter ;

	default:
		break ;
	}
	return	nullptr ;
}

S3DModelLoaderInterface *
	S3DModelLoaderInterface::NewModelLoaderFileExtensionAs( const wchar_t * pszExt )
{
	for ( int i = 0; i < supportModelFileTypeCount; i ++ )
	{
		for ( int j = 0; s_fileExeMimePairs[i].pszExts[j] != nullptr; j ++ )
		{
			if ( SString::CompareNoCase( s_fileExeMimePairs[i].pszExts[j], pszExt ) == 0 )
			{
				return	NewModelLoaderTypeAs( s_fileExeMimePairs[i].type ) ;
			}
		}
	}
	return	nullptr ;
}

S3DModelLoaderInterface *
	S3DModelLoaderInterface::NewModelLoaderMIMETypeAs( const wchar_t * pszMIME )
{
	for ( int i = 0; i < supportModelFileTypeCount; i ++ )
	{
		if ( SString::CompareNoCase( s_fileExeMimePairs[i].pszMIME, pszMIME ) == 0 )
		{
			return	NewModelLoaderTypeAs( s_fileExeMimePairs[i].type ) ;
		}
	}
	return	nullptr ;
}



//////////////////////////////////////////////////////////////////////////////
// モデルファイル・抽象セーバー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DModelSaverInterface, ESLObject )

// MIME 判定
//////////////////////////////////////////////////////////////////////////////
bool S3DModelSaverInterface::IsMatchableMIMEType( const wchar_t * pszMIME )
{
	return	false ;
}

// テクスチャ画像書き出しフォーマット指定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelSaverInterface::SetImageFormat
	( const wchar_t * pwszMIME, const wchar_t * pwszExt,
		const SGLImageEncoderInterface::Options * pOpt )
{
	return	sglErrNotSupported ;
}


//////////////////////////////////////////////////////////////////////////////
// EntisGLS3 互換モデルファイル・ローダー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DModelMDFLoader, S3DModelLoaderInterface )

// ファイル拡張子判定
//////////////////////////////////////////////////////////////////////////////
bool S3DModelMDFLoader::IsMatchableFileExtension( const wchar_t * pszExt )
{
	return	(SString::CompareNoCase( pszExt, L"mdf" ) == 0) ;
}

// MIME 判定
//////////////////////////////////////////////////////////////////////////////
bool S3DModelMDFLoader::IsMatchableMIMEType( const wchar_t * pszMIME )
{
	return	(SString::CompareNoCase( pszMIME, L"application/x-mdf" ) == 0) ;
}

// モデルデータ読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelMDFLoader::ReadModel
	( S3DModelBuffer & model, SSystem::SFileInterface & file )
{
	SChunkFile	cf ;
	if ( cf.OpenChunkFile( &file ) )
	{
		return	sglErrFailed ;
	}
	return	ReadModelChunkFile( model, cf ) ;
}

SGLError S3DModelMDFLoader::ReadModelChunkFile
	( S3DModelBuffer & model, SSystem::SChunkFile & file )
{
	for ( ; ; )
	{
		if ( file.DescendChunk() )
		{
			break ;
		}
		SGLError	err ;
		if ( file.IsEqualCurrentChunkID( "texture " ) )
		{
			err = ReadTextureRecord( model, file ) ;
		}
		else if ( file.IsEqualCurrentChunkID( "surface " ) )
		{
			err = ReadSurfaceRecord( model, file ) ;
		}
		else if ( file.IsEqualCurrentChunkID( "model   " ) )
		{
			err = ReadModelRecord( model, file ) ;
		}
		else
		{
			err = ReadUserRecord( model, file ) ;
		}
		if ( err )
		{
			return	err ;
		}
		file.AscendChunk() ;
	}
	model.Flush() ;
	return	sglErrSuccess ;
}

// テクスチャレコード読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelMDFLoader::ReadTextureRecord
	( S3DModelBuffer & model, SSystem::SChunkFile& file )
{
	//
	// テクスチャ名テーブルを読み込む
	//
	SByteBuffer	bufTxtIDs ;
	if ( file.DescendChunk( "txt_name" ) )
	{
		return	sglErrFailed ;
	}
	bufTxtIDs.ReadFromFile( file ) ;
	file.AscendChunk( ) ;
	//
	uint32_t	nCount ;
	if ( bufTxtIDs.Read( &nCount, sizeof(uint32_t) ) < sizeof(uint32_t) )
	{
		return	sglErrFailed ;
	}
	//
	// テクスチャ名リストを作成する
	//
	SObjectArray<SString>	arrTxtID ;
	size_t	i ;
	for ( i = 0; i < nCount; i ++ )
	{
		SString *	pstrID = new SString ;
		if ( bufTxtIDs.ReadString( *pstrID ) )
		{
			delete	pstrID ;
			return	sglErrFailed ;
		}
		arrTxtID.Add( pstrID ) ;
	}
	//
	// テクスチャ画像を順次読み込む
	//
	S3DTextureLibrary&	textures = model.GetTextureLibrary() ;
	for ( i = 0; i < nCount; i ++ )
	{
		if ( file.DescendChunk( "txtimage" ) )
		{
			return	sglErrFailed ;
		}
		SGLImage *	pImage = new SGLImage ;
		#if	defined(__COTOPHA__)
		SByteBuffer	buf ;
		buf.ReadFromFile( file ) ;
		if ( pImage->ReadImage( &buf ) )
		#else
		if ( pImage->ReadImage( &file ) )
		#endif
		{
			delete	pImage ;
			file.AscendChunk( ) ;
			return	sglErrFailed ;
		}
		file.AscendChunk( ) ;
		//
		pImage->NormalizeToTexture() ;
//		pImage->NormalizeToMipmapTexture() ;
		textures.AddSmartTextureAs( arrTxtID.At(i), pImage ) ;
	}
	return	sglErrSuccess ;
}

// 表面属性レコード読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelMDFLoader::ReadSurfaceRecord
	( S3DModelBuffer & model, SSystem::SChunkFile& file )
{
	//
	// 表面属性名テーブルを読み込む
	//
	SByteBuffer	bufAttrIDs ;
	if ( file.DescendChunk( "surfname" ) )
	{
		return	sglErrFailed ;
	}
	bufAttrIDs.ReadFromFile( file ) ;
	file.AscendChunk( ) ;
	//
	uint32_t	nCount ;
	if ( bufAttrIDs.Read( &nCount, sizeof(uint32_t) ) < sizeof(uint32_t) )
	{
		return	sglErrFailed ;
	}
	//
	// 表面属性名リストを作成する
	//
	SObjectArray<SString>	arrAttrID ;
	size_t	i ;
	for ( i = 0; i < nCount; i ++ )
	{
		SString *	pstrID = new SString ;
		if ( bufAttrIDs.ReadString( *pstrID ) )
		{
			delete	pstrID ;
			return	sglErrFailed ;
		}
		arrAttrID.Add( pstrID ) ;
	}
	//
	// 表面属性データを順次読み込む
	//
	S3DTextureLibrary&	textures = model.GetTextureLibrary() ;
	S3DMaterialLibrary&	materials = model.GetMaterialLibrary() ;
	for ( i = 0; i < nCount; i ++ )
	{
		//
		// 表面属性データを読み込む
		//
		if ( file.DescendChunk( "surfattr" ) )
		{
			return	sglErrFailed ;
		}
		SByteBuffer		bufSurfAttr ;
		bufSurfAttr.ReadFromFile( file ) ;
		file.AscendChunk( ) ;
		//
		size_t	nLimitSize = (size_t) bufSurfAttr.GetLength() ;
		bufSurfAttr.SetLength( nLimitSize + sizeof(uint16_t) ) ;
		//
		// 表面属性データを複製する
		//
		S3DMaterial *		pMaterial = new S3DMaterial ;
		const SURFACE_ATTRIBUTE *	pSufAttr =
				(const SURFACE_ATTRIBUTE*) bufSurfAttr.GetConstArray() ;
		//
		pMaterial->m_attrSurface.flagsShading = pSufAttr->dwShadingFlags ;
		pMaterial->m_attrSurface.colorBase = pSufAttr->rgbaColor ;
		pMaterial->m_attrSurface.colorShade = pSufAttr->rgbaShade ;
		pMaterial->m_attrSurface.nAmbient = pSufAttr->nAmbient ;
		pMaterial->m_attrSurface.nDiffusion = pSufAttr->nDiffusion ;
		pMaterial->m_attrSurface.nSpecular = pSufAttr->nSpecular ;
		pMaterial->m_attrSurface.nSpecularSize = pSufAttr->nSpecularSize ;
		pMaterial->m_attrSurface.nTransparency = pSufAttr->nTransparency ;
		pMaterial->m_attrSurface.nDeepness = pSufAttr->nDeepness ;
		pMaterial->m_attrSurface.nDeepnessPower = 0x100 ;
		pMaterial->m_attrSurface.nReflection = pSufAttr->nReflection ;
		pMaterial->m_attrSurface.fpRefraction = pSufAttr->nRefraction ;
		//
		bool	fBackSuface = false ;
		#if	!defined(__COTOPHA__) && !defined(__ENTIS_GLS__)
		if ( !(pMaterial->m_attrSurface.flagsShading & shadingSingleSidePlane) )
		{
			if ( pMaterial->m_attrSurface.nTransparency > 0 )
			{
				pMaterial->m_attrSurface.flagsShading |= shadingSingleSidePlane ;
				pMaterial->EnableBackSurfaceAttribute( true ) ;
				pMaterial->SetBackSurfaceAttribute( pMaterial->m_attrSurface ) ;
				//
				fBackSuface = true ;
			}
		}
		#endif
		//
		// テクスチャ画像を設定する
		//
		if ( pSufAttr->dwShadingFlags & shadingEnvironmentMapping )
		{
			if ( (pSufAttr->pTextureImage != 0)
				&& (pSufAttr->pTextureImage < nLimitSize) )
			{
				SString	strID
					( (const uint16_t*)
						bufSurfAttr.GetAt(pSufAttr->pTextureImage) ) ;
				pMaterial->SetTexture
					( textures.GetTextureAs( strID ),
						0, S3DMaterial::textureEnvironment, 1.0f, 0.0f, strID ) ;
				if ( fBackSuface )
				{
					pMaterial->SetBackTexture
						( textures.GetTextureAs( strID ),
							0, S3DMaterial::textureEnvironment, 1.0f, 0.0f, strID ) ;
				}
			}
		}
		else if ( pSufAttr->dwShadingFlags & shadingTextureMapping )
		{
			if ( (pSufAttr->pTextureImage != 0)
				&& (pSufAttr->pTextureImage < nLimitSize) )
			{
				SString	strID
					( (const uint16_t*)
						bufSurfAttr.GetAt(pSufAttr->pTextureImage) ) ;
				SGLImageObject *	pTexture = textures.GetTextureAs( strID ) ;
				pMaterial->SetTexture
					( pTexture, 0,
						S3DMaterial::textureDiffusion, 1.0f, 0.0f, strID ) ;
				if ( fBackSuface )
				{
					pMaterial->SetBackTexture
						( pTexture, 0,
							S3DMaterial::textureDiffusion, 1.0f, 0.0f, strID ) ;
				}
				if ( (pMaterial->m_attrSurface.flagsShading
								& shadingHintMask) == shadingHintOfUnknown )
				{
					pMaterial->m_attrSurface.flagsShading =
						(pMaterial->m_attrSurface.flagsShading
												& ~shadingHintMask)
							| TextureHintOfAlpha( pTexture ) ;
				}
			}
			/*
			if ( (pSufAttr->pSmallImage != 0)
				&& (pSufAttr->pSmallImage < nLimitSize) )
			{
				pMaterial->SetSubTextureZ
					( pSufAttr->rThresholdZ
						* pow( 0.5, (double) pSufAttr->nSmallScale - 1 ) ) ;
				//
				SString	strID
					( (const uint16_t*)
						bufSurfAttr.GetAt(pSufAttr->pSmallImage) ) ;
				pMaterial->SetTexture
					( textures.GetTextureAs( strID ),
						pSufAttr->nSmallScale, S3DMaterial::textureSub ) ;
				if ( fBackSuface )
				{
					pMaterial->SetBackTexture
						( textures.GetTextureAs( strID ),
							pSufAttr->nSmallScale, S3DMaterial::textureSub ) ;
				}
			}
			*/
			if ( (pSufAttr->pLuminousImage != 0)
				&& (pSufAttr->pLuminousImage < nLimitSize) )
			{
				SString	strID
					( (const uint16_t*)
						bufSurfAttr.GetAt(pSufAttr->pLuminousImage) ) ;
				int	iLuminous = 1 ;
				/*
				if ( (pSufAttr->pSmallImage != 0)
						&& (pSufAttr->nSmallScale != 0) )
				{
					iLuminous = pSufAttr->nSmallScale ;
				}
				*/
				pMaterial->m_attrSurface.flagsShading |= shadingLuminousTexture ;
				pMaterial->SetTexture
					( textures.GetTextureAs( strID ), iLuminous,
						S3DMaterial::textureLuminous,
						(float32_t) ((0x100 - pSufAttr->nLuminousApply) / 256.0), 0.0f, strID ) ;
				if ( fBackSuface )
				{
					pMaterial->m_attrBack.flagsShading |= shadingLuminousTexture ;
					pMaterial->SetBackTexture
						( textures.GetTextureAs( strID ), iLuminous,
							S3DMaterial::textureLuminous,
							(float32_t) ((0x100 - pSufAttr->nLuminousApply) / 256.0), 0.0f, strID ) ;
				}
			}
		}
		pMaterial->SetUpdateMaterialBuffer() ;
		//
		// 表面属性を追加
		//
		materials.AddSmartMaterialAs( arrAttrID.At(i), pMaterial ) ;
	}
	return	sglErrSuccess ;
}

// モデルレコード読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelMDFLoader::ReadModelRecord
	( S3DModelBuffer & model, SSystem::SChunkFile& file )
{
	//
	// 頂点テーブルレコードを読み込む
	//
	if ( file.DescendChunk( "vertexes" ) )
	{
		return	sglErrFailed ;
	}
	size_t	nLength ;
	nLength = (size_t) file.GetLength() / sizeof(S3DVector4) ;
	model.SetVertexBufferLength( nLength ) ;
	file.Read( model.GetVertexBufferAt(), nLength * sizeof(S3DVector4) ) ;
	file.AscendChunk( ) ;
	//
	model.CommitVertexBuffer( 0, nLength ) ;
	//
	// 法線テーブルレコードを読み込む
	//
	if ( file.DescendChunk( "normals " ) )
	{
		return	sglErrFailed ;
	}
	nLength = (size_t) file.GetLength() / sizeof(S3DVector4) ;
	model.SetNormalBufferLength( nLength ) ;
	file.Read( model.GetNormalBufferAt(), nLength * sizeof(S3DVector4) ) ;
	file.AscendChunk( ) ;
	//
	model.CommitNormalBuffer( 0, nLength ) ;
	//
	// プリミティブレコードを読み込む
	//
	if ( file.DescendChunk( "primitiv" ) )
	{
		return	sglErrFailed ;
	}
	uint32_t	nCount ;
	if ( file.Read( &nCount, sizeof(uint32_t) ) < sizeof(uint32_t) )
	{
		file.AscendChunk( ) ;
		return	sglErrFailed ;
	}
	S3DMaterialLibrary&	materials = model.GetMaterialLibrary() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		uint32_t	nDataBytes ;
		if ( file.Read( &nDataBytes, sizeof(uint32_t) ) < sizeof(uint32_t) )
		{
			file.AscendChunk( ) ;
			return	sglErrFailed ;
		}
		SByteBuffer	buf ;
		buf.SetLength( nDataBytes + sizeof(wchar_t) ) ;
		file.Read( buf.GetArray(), nDataBytes ) ;
		buf.FinishArray() ;
		//
		// プリミティブ・ヘッダ
		//
		PRIMITIVE_HEADER	phHeader ;
		buf.Read( &phHeader, sizeof(PRIMITIVE_HEADER) ) ;
		//
		if ( (phHeader.dwTypeFlag == typeImagePrimitive)
			|| (phHeader.dwTypeFlag == typeInfinitePlane) )
		{
			// 画像プリミティブ・無限平面は無視
			continue ;
		}
		//
		// 表面属性を設定する
		//
		S3DMaterial *	pMaterial =
			materials.GetMaterialAs
				( SString( (const uint16_t*) buf.GetAt( phHeader.pSurfaceAttr ) ) ) ;
		if ( pMaterial == nullptr )
		{
			return	sglErrFailed ;
		}
		//
		// プリミティブ追加
		//
		if ( phHeader.dwTypeFlag & flagMeshPolygon )
		{
			//
			// インデックス化されたメッシュ
			//
			uint32_t			iVertex, iNormal ;
			const S2DVector *	pvUVMap = nullptr ;
			const S3DColor *	pColor = nullptr ;
			buf.Read( &iVertex, sizeof(uint32_t) ) ;
			buf.Read( &iNormal, sizeof(uint32_t) ) ;
			if ( phHeader.dwTypeFlag & flagTexturePolygon )
			{
				pvUVMap = (const S2DVector *)
							buf.GetAt( (size_t) buf.GetPosition() ) ;
				buf.Seek
					( phHeader.dwVertexCount * sizeof(S2DVector),
										SFileInterface::FromCurrent ) ;
			}
			if ( phHeader.dwTypeFlag & flagVertexColorPolygon )
			{
				pColor = (const S3DColor *)
							buf.GetAt( (size_t) buf.GetPosition() ) ;
				buf.Seek
					( phHeader.dwVertexCount * sizeof(S3DColor),
										SFileInterface::FromCurrent ) ;
			}
			if ( !(phHeader.dwTypeFlag
					& (flagTexturePolygon | flagVertexColorPolygon)) )
			{
				buf.Seek
					( phHeader.dwVertexCount * sizeof(S2DVector),
										SFileInterface::FromCurrent ) ;
			}
			uint32_t	nMeshBytes = 0, nPolyCount = 0 ;
			buf.Read( &nMeshBytes, sizeof(uint32_t) ) ;
			buf.Read( &nPolyCount, sizeof(uint32_t) ) ;
			//
			const uint32_t *	pSrcIndex =
				(const uint32_t *) buf.GetAt( (size_t) buf.GetPosition() ) ;
			//
			SArray<uint32_t>	lstIndexed ;
			size_t				countTriangles = 0 ;
			lstIndexed.SetLimit( nMeshBytes / sizeof(uint32_t) ) ;
			for ( size_t j = 0; j < nPolyCount; j ++ )
			{
				uint32_t	nVertexCount = *(pSrcIndex ++) ;
				for ( size_t k = 2; k < nVertexCount; k ++ )
				{
					lstIndexed.Add( pSrcIndex[0] ) ;
					lstIndexed.Add( pSrcIndex[k - 1] ) ;
					lstIndexed.Add( pSrcIndex[k] ) ;
					countTriangles ++ ;
				}
				pSrcIndex += nVertexCount ;
			}
			//
			model.AddIndexedTriangleList
				( pMaterial, 0, countTriangles, phHeader.dwVertexCount,
					model.GetVertexBufferAt(iVertex),
					model.GetNormalBufferAt(iNormal),
					pvUVMap, pColor, lstIndexed.GetConstArray() ) ;
		}
		else
		{
			//
			// 単一のポリゴン
			//
		}
	}
	file.AscendChunk( ) ;
	//
	model.NormalizeAllMeshsFace() ;
	//
	return	sglErrSuccess ;
}

// 拡張レコード読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelMDFLoader::ReadUserRecord
	( S3DModelBuffer & model, SSystem::SChunkFile& file )
{
	if ( file.IsEqualCurrentChunkID( "portion " ) )
	{
		uint32_t	nPartCount ;
		if ( file.Read( &nPartCount, sizeof(uint32_t) ) < sizeof(uint32_t) )
		{
			return	sglErrFailed ;
		}
		SStrSortArray<S3DModelBuffer::MeshGroup>&
				ssaGroupList = model.GetMeshGroupList() ;
		ssaGroupList.RemoveAll() ;
		//
		for ( size_t i = 0; i < nPartCount; i ++ )
		{
			SString	strName ;
			file.ReadString( strName ) ;
			//
			MESH_PORTION	mp ;
			file.Read( &mp, sizeof(MESH_PORTION) ) ;
			//
			S3DModelBuffer::MeshGroup	mg ;
			mg.m_iFirstMesh = mp.iMesh ;
			mg.m_nMeshCount = mp.nCount ;
			mg.m_vCenter = S3DDVector( 0, 0, 0 ) ;
			//
			ssaGroupList.Add( strName, mg ) ;
		}
	}
	return	sglErrSuccess ;
}

// テクスチャのフォーマットやα値からシェーディングヒントフラグを評価する
//////////////////////////////////////////////////////////////////////////////
uint64_t S3DModelMDFLoader::TextureHintOfAlpha( SGLImageObject * pTexture )
{
	SGLImageInfo	imginf ;
	pTexture->GetImageInfo( imginf ) ;
	if ( !(imginf.format & formatImageFlagAlpha) )
	{
		return	shadingHintOfFullAlpha ;
	}
	if ( imginf.depth != 32 )
	{
		return	shadingHintOfUnknown ;
	}
	uint8_t *	pbytPixels =
		pTexture->LockBuffer( imginf, SGLImageObject::lockRead ) ;
	size_t	nOpaque = 0 ;
	size_t	nTransparent = 0 ;
	for ( uint32_t y = 0; y < imginf.height; y ++ )
	{
		uint32_t *	pdwLine = (uint32_t*) pbytPixels ;
		for ( uint32_t x = 0; x < imginf.width; x ++ )
		{
			uint32_t	rgba = pdwLine[x] ;
			if ( rgba == 0 )
			{
				nTransparent ++ ;
			}
			else if ( (rgba >> 24) == 0xFF )
			{
				nOpaque ++ ;
			}
		}
		pbytPixels += imginf.pitchLine ;
	}
	size_t	nAllPixels = imginf.width * imginf.height ;
	if ( nOpaque == nAllPixels )
	{
		return	shadingHintOfFullAlpha ;
	}
	else if ( (nOpaque + nTransparent) * 100 / nAllPixels >= 95 )
	{
		return	shadingHintOfAlpha ;
	}
	return	shadingHintOfHalfAlpha ;
}



//////////////////////////////////////////////////////////////////////////////
// EntisGLS4 標準モデルファイル・ローダー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DStdModelLoader, S3DModelLoaderInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DStdModelLoader::S3DStdModelLoader( void )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DStdModelLoader::~S3DStdModelLoader( void )
{
}

// ファイル拡張子判定
//////////////////////////////////////////////////////////////////////////////
bool S3DStdModelLoader::IsMatchableFileExtension( const wchar_t * pszExt )
{
	return	(SString::CompareNoCase( pszExt, L"mdfx" ) == 0) ;
}

// MIME 判定
//////////////////////////////////////////////////////////////////////////////
bool S3DStdModelLoader::IsMatchableMIMEType( const wchar_t * pszMIME )
{
	return	(SString::CompareNoCase( pszMIME, L"application/x-mdfx" ) == 0) ;
}

// ファイルを読み込む
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdModelLoader::ReadModel
	( S3DModelBuffer & model, SSystem::SFileInterface & file )
{
	SChunkFile	cf ;
	if ( cf.OpenChunkFile( &file ) )
	{
		return	sglErrFailed ;
	}
	if ( cf.GetFileHeader().dwFileID == SChunkFile::fidEGL3DModel )
	{
		S3DModelMDFLoader	gls3Loader ;
		return	gls3Loader.ReadModelChunkFile( model, cf ) ;
	}
	for ( ; ; )
	{
		if ( cf.DescendChunk() )
		{
			break ;
		}
		SGLError	err ;
		if ( cf.IsEqualCurrentChunkID( "texture " ) )
		{
			err = ReadTextureChunk( model, cf ) ;
		}
		else if ( cf.IsEqualCurrentChunkID( "material" ) )
		{
			err = ReadMaterialChunk( model, cf ) ;
		}
		else if ( cf.IsEqualCurrentChunkID( "mesh    " ) )
		{
			err = ReadMeshChunk( model, cf ) ;
		}
		else if ( cf.IsEqualCurrentChunkID( "mesh_grp" ) )
		{
			err = ReadMeshGroupChunk( model, cf ) ;
		}
		else if ( cf.IsEqualCurrentChunkID( "morphmsh" ) )
		{
			err = ReadMeshMorphTargetChunk( model, cf ) ;
		}
		else if ( cf.IsEqualCurrentChunkID( "mesh_div" ) )
		{
			err = ReadMeshDivisionInfoChunk( model, cf ) ;
		}
		else if ( cf.IsEqualCurrentChunkID( "bones   " ) )
		{
			err = ReadBoneChunk( model, cf ) ;
		}
		else if ( cf.IsEqualCurrentChunkID( "markinfo" ) )
		{
			err = ReadMarkerInfoChunk( model, cf ) ;
		}
		else if ( cf.IsEqualCurrentChunkID( "clothinf" ) )
		{
			err = ReadClothMeshInfoChunk( model, cf ) ;
		}
		else if ( cf.IsEqualCurrentChunkID( "poses   " ) )
		{
			err = ReadPosesChunk( model, cf ) ;
		}
		else if ( cf.IsEqualCurrentChunkID( "metainfo" ) )
		{
			err = ReadMetaInfoChunk( model, cf ) ;
		}
		else if ( cf.IsEqualCurrentChunkID( "scene   " ) )
		{
			err = ReadSceneChunk( model, cf ) ;
		}
		else
		{
			err = ReadUserChunk( model, cf ) ;
		}
		if ( err )
		{
			return	err ;
		}
		cf.AscendChunk() ;
	}
	model.Flush() ;
	return	sglErrSuccess ;
}

// テクスチャ読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdModelLoader::ReadTextureChunk
	( S3DModelBuffer & model, SSystem::SChunkFile& file )
{
	for ( ; ; )
	{
		if ( file.DescendChunk() )
		{
			break ;
		}
		if ( !file.IsEqualCurrentChunkID( "image   " ) )
		{
			continue ;
		}
		do
		{
			//
			// 定義文字列読み込み
			//
			if ( file.DescendChunk( "strings " ) )
			{
				break ;
			}
			SObjectArray<SString>	aStrings ;
			ReadStrings( aStrings, file ) ;
			file.AscendChunk() ;
			//
			SString *	pstrID = aStrings.GetAt( 0 ) ;
			SString *	pstrMIME = aStrings.GetAt( 1 ) ;
			if ( pstrID == nullptr )
			{
				break ;
			}
			const wchar_t *	pszMIME = nullptr ;
			if ( pstrMIME != nullptr )
			{
				pszMIME = *pstrMIME ;
			}
			//
			// 情報読み込み
			//
			TextureInfo	txinf ;
			eslFillMemory( &txinf, 0, sizeof(TextureInfo) ) ;
			//
			if ( !file.DescendChunk( "txt_info" ) )
			{
				file.Read( &txinf, sizeof(TextureInfo) ) ;
				file.AscendChunk() ;
			}
			//
			// 画像データ読み込み
			//
			if ( file.DescendChunk( "img_data" ) )
			{
				break ;
			}
			SGLImage *	pImage = new SGLImage ;
			if ( !pImage->ReadImage( &file, pszMIME ) )
			{
				uint32_t	nFlags = 0 ;
				if ( txinf.nFlags & textureCompressed )
				{
					nFlags |= SGLImageObject::bufferCompressedTexture ;
				}
				pImage->NormalizeToTexture( nFlags ) ;
				//
				if ( txinf.nFlags & textureMipmap )
				{
					pImage->NormalizeToMipmapTexture() ;
				}
				pImage->SetImageIdentity( *pstrID ) ;
				//
				model.GetTextureLibrary().
						AddSmartTextureAs( *pstrID, pImage ) ;
			}
			else
			{
				ESLTrace( "failed to read texture image \'%s\'\n",
									pstrID->ToCharArray().GetConstArray() ) ;
				delete	pImage ;
			}
			file.AscendChunk() ;
		}
		while ( false ) ;
		//
		file.AscendChunk() ;
	}
	return	sglErrSuccess ;
}

// 表面属性読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdModelLoader::ReadMaterialChunk
	( S3DModelBuffer & model, SSystem::SChunkFile& file )
{
	for ( ; ; )
	{
		if ( file.DescendChunk() )
		{
			break ;
		}
		if ( !file.IsEqualCurrentChunkID( "surface " ) )
		{
			continue ;
		}
		S3DMaterial *	pMaterial = new S3DMaterial ;
		do
		{
			//
			// 定義文字列読み込み
			//
			if ( file.DescendChunk( "strings " ) )
			{
				break ;
			}
			SObjectArray<SString>	aStrings ;
			ReadStrings( aStrings, file ) ;
			file.AscendChunk() ;
			//
			SString *	pstrID = aStrings.GetAt( 0 ) ;
			if ( pstrID == nullptr )
			{
				break ;
			}
			//
			// 表面
			//
			if ( file.DescendChunk( "faceattr" ) )
			{
				break ;
			}
			S3DSurfaceAttribute	sufattr ;
			file.Read( &sufattr, sizeof(S3DSurfaceAttribute) ) ;
			file.AscendChunk() ;
			//
			pMaterial->SetSurfaceAttribute( sufattr ) ;
			//
			// 表面テクスチャ
			//
			if ( !file.DescendChunk( "face_txt" ) )
			{
				SurfaceTextures	suftxt ;
				file.Read( &suftxt, sizeof(SurfaceTextures) ) ;
				file.AscendChunk() ;
				//
				for ( int i = 0; (i < S3DMaterial::textureMaxCount)
									&& (i < (int) suftxt.nCount); i ++ )
				{
					SString *	pstrTxtID =
						aStrings.GetAt( suftxt.txtEntries[i].iTexture ) ;
					if ( pstrTxtID == nullptr )
					{
						continue ;
					}
					SGLImageObject *	pTexture =
						model.GetTextureLibrary().GetTextureAs( *pstrTxtID ) ;
					if ( pTexture != nullptr )
					{
						pMaterial->SetTexture
							( pTexture, i, suftxt.txtEntries[i].nFlags,
										suftxt.txtEntries[i].fpApply,
										suftxt.txtEntries[i].fpParam1,
										*pstrTxtID ) ;
					}
				}
			}
			//
			// 裏面
			//
			if ( !file.DescendChunk( "backattr" ) )
			{
				S3DSurfaceAttribute	sufattr ;
				file.Read( &sufattr, sizeof(S3DSurfaceAttribute) ) ;
				file.AscendChunk() ;
				//
				pMaterial->SetBackSurfaceAttribute( sufattr ) ;
				pMaterial->EnableBackSurfaceAttribute( true ) ;
			}
			//
			// 裏面テクスチャ
			//
			if ( !file.DescendChunk( "back_txt" ) )
			{
				SurfaceTextures	suftxt ;
				file.Read( &suftxt, sizeof(SurfaceTextures) ) ;
				file.AscendChunk() ;
				//
				for ( int i = 0; (i < S3DMaterial::textureMaxCount)
									&& (i < (int) suftxt.nCount); i ++ )
				{
					SString *	pstrTxtID =
						aStrings.GetAt( suftxt.txtEntries[i].iTexture ) ;
					if ( pstrTxtID == nullptr )
					{
						continue ;
					}
					SGLImageObject *	pTexture =
						model.GetTextureLibrary().GetTextureAs( *pstrTxtID ) ;
					if ( pTexture != nullptr )
					{
						pMaterial->SetBackTexture
							( pTexture, i, suftxt.txtEntries[i].nFlags,
											suftxt.txtEntries[i].fpApply,
											suftxt.txtEntries[i].fpParam1,
											*pstrTxtID ) ;
					}
				}
			}
			//
			model.GetMaterialLibrary().
					AddSmartMaterialAs( *pstrID, pMaterial ) ;
			pMaterial = nullptr ;
		}
		while ( false ) ;
		//
		delete	pMaterial ;
		//
		file.AscendChunk() ;
	}
	return	sglErrSuccess ;
}

// メッシュ読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdModelLoader::ReadMeshChunk
	( S3DModelBuffer & model, SSystem::SChunkFile& file )
{
	for ( ; ; )
	{
		if ( file.DescendChunk() )
		{
			break ;
		}
		if ( file.IsEqualCurrentChunkID( "tristrip" )
			|| file.IsEqualCurrentChunkID( "itrilist" )
			|| file.IsEqualCurrentChunkID( "primitiv" ) )
		do
		{
			//
			// 定義文字列読み込み
			//
			if ( file.DescendChunk( "strings " ) )
			{
				break ;
			}
			SObjectArray<SString>	aStrings ;
			ReadStrings( aStrings, file ) ;
			file.AscendChunk() ;
			//
			// メッシュ情報
			//
			if ( file.DescendChunk( "meshinfo" ) )
			{
				break ;
			}
			TriangleMeshInfo	meshinf ;
			eslFillMemory
				( &meshinf, 0, sizeof(S3DStdModelLoader::TriangleMeshInfo) ) ;
			file.Read( &meshinf, sizeof(TriangleMeshInfo) ) ;
			file.AscendChunk() ;
			//
			// 表面属性取得
			//
			SString *	pstrMaterialID =
							aStrings.GetAt( meshinf.iMaterial ) ;
			if ( pstrMaterialID == nullptr )
			{
				break ;
			}
			S3DMaterial *	pMaterial =
				model.GetMaterialLibrary().GetMaterialAs( *pstrMaterialID ) ;
			if ( pMaterial == nullptr )
			{
				break ;
			}
			//
			// 頂点バッファ
			//
			SArray<S3DVector4>	bufVertex ;
			SArray<S3DVector4>	bufNormal ;
			SArray<S2DVector>	bufUVMap ;
			SArray<S3DColor>	bufColor ;
			SArray<float32_t>	bufExAttr ;
			SArray<uint32_t>	bufIndexed ;
			SArray<uint32_t>	bufSubIndexed[VertexBuffer::countSubMesh] ;
			//
			if ( file.DescendChunk( "vertex  " ) )
			{
				break ;
			}
			file.Read
				( bufVertex.GetArray( meshinf.countVertex ),
							meshinf.countVertex * sizeof(S3DVector4) ) ;
			bufVertex.FinishArray() ;
			file.AscendChunk() ;
			//
			for ( ; ; )
			{
				if ( file.DescendChunk() )
				{
					break ;
				}
				if ( file.IsEqualCurrentChunkID( "normal  " ) )
				{
					file.Read
						( bufNormal.GetArray( meshinf.countVertex ),
									meshinf.countVertex * sizeof(S3DVector4) ) ;
					bufNormal.FinishArray() ;
				}
				else if ( file.IsEqualCurrentChunkID( "uv_map  " ) )
				{
					file.Read
						( bufUVMap.GetArray( meshinf.countVertex ),
									meshinf.countVertex * sizeof(S2DVector) ) ;
					bufUVMap.FinishArray() ;
				}
				else if ( file.IsEqualCurrentChunkID( "color   " ) )
				{
					file.Read
						( bufColor.GetArray( meshinf.countVertex ),
									meshinf.countVertex * sizeof(S3DColor) ) ;
					bufColor.FinishArray() ;
				}
				else if ( file.IsEqualCurrentChunkID( "index   " ) )
				{
					file.Read
						( bufIndexed.GetArray( meshinf.countPolygon * 3 ),
							meshinf.countPolygon * (sizeof(uint32_t) * 3) ) ;
					bufIndexed.FinishArray() ;
				}
				else if ( file.IsEqualCurrentChunkID( "ex_attr " ) )
				{
					const size_t	nElCount = meshinf.countVertex
												* meshinf.nExAttrElements ;
					file.Read
						( bufExAttr.GetArray( nElCount ),
									nElCount * sizeof(S3DColor) ) ;
					bufExAttr.FinishArray() ;
				}
				else if ( file.IsEqualCurrentChunkID( "index1  " ) )
				{
					size_t	nLength =
						(size_t) file.GetLength() / sizeof(uint32_t) ;
					file.Read
						( bufSubIndexed[0].GetArray( nLength ),
									nLength * sizeof(uint32_t) ) ;
					bufSubIndexed[0].FinishArray() ;
				}
				else if ( file.IsEqualCurrentChunkID( "index2  " ) )
				{
					size_t	nLength =
						(size_t) file.GetLength() / sizeof(uint32_t) ;
					file.Read
						( bufSubIndexed[1].GetArray( nLength ),
									nLength * sizeof(uint32_t) ) ;
					bufSubIndexed[1].FinishArray() ;
				}
				else if ( file.IsEqualCurrentChunkID( "index3  " ) )
				{
					size_t	nLength =
						(size_t) file.GetLength() / sizeof(uint32_t) ;
					file.Read
						( bufSubIndexed[2].GetArray( nLength ),
									nLength * sizeof(uint32_t) ) ;
					bufSubIndexed[2].FinishArray() ;
				}
				else if ( file.IsEqualCurrentChunkID( "index4  " ) )
				{
					size_t	nLength =
						(size_t) file.GetLength() / sizeof(uint32_t) ;
					file.Read
						( bufSubIndexed[3].GetArray( nLength ),
									nLength * sizeof(uint32_t) ) ;
					bufSubIndexed[3].FinishArray() ;
				}
				file.AscendChunk() ;
			}
			//
			// メッシュ追加
			//
			const size_t	iMesh = model.GetMeshCount() ;
			if ( file.IsEqualCurrentChunkID( "tristrip" ) )
			{
				model.AddTriangleStrip
					( pMaterial, 0, meshinf.countPolygon,
						bufVertex.GetConstArray(), bufNormal.GetConstArray(),
						bufUVMap.GetConstArray(), bufColor.GetConstArray() ) ;
			}
			else
			{
				if ( file.IsEqualCurrentChunkID( "itrilist" ) )
				{
					model.AddIndexedTriangleList
						( pMaterial, 0,
							meshinf.countPolygon, meshinf.countVertex,
							bufVertex.GetConstArray(), bufNormal.GetConstArray(),
							bufUVMap.GetConstArray(), bufColor.GetConstArray(),
							bufIndexed.GetConstArray() ) ;
				}
				else
				{
					S3DPrimitiveType	type = (S3DPrimitiveType) meshinf.typePrimitive ;
					model.AddIndexedPrimitiveList
						( pMaterial, 0, type,
							meshinf.countPolygon * GetPrimitiveVertexCount(type),
							meshinf.countVertex,
							bufVertex.GetConstArray(), bufNormal.GetConstArray(),
							bufUVMap.GetConstArray(), bufColor.GetConstArray(),
							bufIndexed.GetConstArray() ) ;
				}
				//
				for ( size_t iSub = 0; iSub < VertexBuffer::countSubMesh; iSub ++ )
				{
					if ( bufSubIndexed[iSub].GetLength() == 0 )
					{
						continue ;
					}
					model.UpdateSubIndexedTriangleList
						( iMesh, iSub, 0,
							bufSubIndexed[iSub].GetLength() / 3,
							bufSubIndexed[iSub].GetConstArray() ) ;
				}
				if ( meshinf.nFlags & meshFlagSubMeshDensity )
				{
					model.SetSubMeshDensity( iMesh, meshinf.fpSubMeshDensity ) ;
				}
			}
			if ( (meshinf.nExAttrElements > 0)
				&& (bufExAttr.GetLength() > 0) )
			{
				model.SetExtendVertexAttribute
					( iMesh, meshinf.nExAttrElements,
						meshinf.countVertex, bufExAttr.GetConstArray() ) ;
			}
		}
		while ( false ) ;
		//
		file.AscendChunk() ;
	}
	return	sglErrSuccess ;
}

// メッシュグループ情報読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdModelLoader::ReadMeshGroupChunk
	( S3DModelBuffer & model, SSystem::SChunkFile& file )
{
	//
	// 定義文字列読み込み
	//
	if ( file.DescendChunk( "strings " ) )
	{
		return	sglErrFailed ;
	}
	SObjectArray<SString>	aStrings ;
	ReadStrings( aStrings, file ) ;
	file.AscendChunk() ;
	//
	size_t	iGroup = 0 ;
	for ( ; ; )
	{
		if ( file.DescendChunk() )
		{
			break ;
		}
		if ( file.IsEqualCurrentChunkID( "group   " ) )
		{
			//
			// メッシュグループ追加
			//
			S3DModelBuffer::MeshGroup	mgroup ;
			file.Read( &mgroup, sizeof(S3DModelBuffer::MeshGroup) ) ;
			//
			SString *	pstrID = aStrings.GetAt( iGroup ++ ) ;
			if ( pstrID != nullptr )
			{
				model.GetMeshGroupList().Add( *pstrID, mgroup ) ;
			}
		}
		file.AscendChunk() ;
	}
	return	sglErrSuccess ;
}

// モーフターゲット読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdModelLoader::ReadMeshMorphTargetChunk
	( S3DModelBuffer & model, SSystem::SChunkFile& file )
{
	//
	// 定義文字列読み込み
	//
	if ( file.DescendChunk( "strings " ) )
	{
		return	sglErrFailed ;
	}
	SObjectArray<SString>	aStrings ;
	ReadStrings( aStrings, file ) ;
	file.AscendChunk() ;
	//
	for ( ; ; )
	{
		if ( file.DescendChunk() )
		{
			break ;
		}
		if ( file.IsEqualCurrentChunkID( "mesh    " ) )
		{
			//
			// メッシュ情報
			//
			if ( file.DescendChunk( "meshinfo" ) )
			{
				break ;
			}
			SArray<uint32_t>	aRelMesh ;
			MorphingMeshInfo	meshinf ;
			if ( file.Read
				( &meshinf, sizeof(MorphingMeshInfo) )
								< sizeof(MorphingMeshInfo) )
			{
				file.AscendChunk() ;
				file.AscendChunk() ;
				return	sglErrFailed ;
			}
			file.Read
				( aRelMesh.GetArray(meshinf.countRelMesh),
							meshinf.countRelMesh * sizeof(uint32_t) ) ;
			aRelMesh.FinishArray() ;
			file.AscendChunk() ;
			//
			SString *	pstrMeshID = aStrings.GetAt( meshinf.iMeshID ) ;
			if ( pstrMeshID == nullptr )
			{
				return	sglErrFailed ;
			}
			for ( size_t i = 0; i < meshinf.countRelMesh; i ++ )
			{
				S3DModelBuffer::MeshObject *
						pMesh = model.GetMeshObjectAt( aRelMesh.At(i) ) ;
				if ( pMesh != nullptr )
				{
					pMesh->m_arrMorphTarget.Add( new SString( *pstrMeshID ) ) ;
				}
			}
			//
			// 頂点バッファ
			//
			S3DModelBuffer::MorphTargetMesh *
						pmtm = new S3DModelBuffer::MorphTargetMesh ;
			pmtm->m_countVertex = meshinf.countVertex ;
			model.AddMorhTargetAs( *pstrMeshID, pmtm ) ;
			//
			size_t	countVertex = meshinf.countVertex ;
			for ( ; ; )
			{
				if ( file.DescendChunk() )
				{
					break ;
				}
				if ( file.IsEqualCurrentChunkID( "vertex  " ) )
				{
					file.Read
						( pmtm->m_bufVertex.GetArray( countVertex ),
									countVertex * sizeof(S3DVector4) ) ;
					pmtm->m_bufVertex.FinishArray() ;
				}
				else if ( file.IsEqualCurrentChunkID( "normal  " ) )
				{
					file.Read
						( pmtm->m_bufNormal.GetArray( countVertex ),
									countVertex * sizeof(S3DVector4) ) ;
					pmtm->m_bufNormal.FinishArray() ;
				}
				else if ( file.IsEqualCurrentChunkID( "uv_map  " ) )
				{
					file.Read
						( pmtm->m_bufUVMap.GetArray( countVertex ),
									countVertex * sizeof(S2DVector) ) ;
					pmtm->m_bufUVMap.FinishArray() ;
				}
				else if ( file.IsEqualCurrentChunkID( "color   " ) )
				{
					file.Read
						( pmtm->m_bufColor.GetArray( countVertex ),
									countVertex * sizeof(S3DColor) ) ;
					pmtm->m_bufColor.FinishArray() ;
				}
				else if ( file.IsEqualCurrentChunkID( "weight  " ) )
				{
					file.Read
						( pmtm->m_bufWeight.GetArray( countVertex ),
									countVertex * sizeof(float32_t) ) ;
					pmtm->m_bufWeight.FinishArray() ;
				}
				file.AscendChunk() ;
			}
		}
		file.AscendChunk() ;
	}
	model.BuildupMeshMorphingTarget() ;
	return	sglErrSuccess ;
}

// 分割メッシュ情報読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdModelLoader::ReadMeshDivisionInfoChunk
	( S3DModelBuffer & model, SSystem::SChunkFile& file )
{
	//
	// 定義文字列読み込み
	//
	if ( file.DescendChunk( "strings " ) )
	{
		return	sglErrFailed ;
	}
	SObjectArray<SString>	aStrings ;
	ReadStrings( aStrings, file ) ;
	file.AscendChunk() ;
	//
	for ( ; ; )
	{
		if ( file.DescendChunk() )
		{
			break ;
		}
		if ( file.IsEqualCurrentChunkID( "mesh    " ) )
		{
			//
			// 分割メッシュ情報
			//
			if ( file.DescendChunk( "div_info" ) )
			{
				break ;
			}
			SArray<uint32_t>	aMorphEntry ;
			MeshDivisionInfo	divinf ;
			if ( file.Read
				( &divinf, sizeof(MeshDivisionInfo) )
								< sizeof(MeshDivisionInfo) )
			{
				file.AscendChunk() ;
				file.AscendChunk() ;
				return	sglErrFailed ;
			}
			file.Read
				( aMorphEntry.GetArray(divinf.countMorphList),
							divinf.countMorphList * sizeof(uint32_t) ) ;
			aMorphEntry.FinishArray() ;
			file.AscendChunk() ;
			//
			SString *	pstrMeshID = aStrings.GetAt( divinf.iMeshID ) ;
			if ( pstrMeshID == nullptr )
			{
				file.AscendChunk() ;
				return	sglErrFailed ;
			}
			//
			// 各分割情報
			//
			S3DModelBuffer::MeshDivision *	pMeshDiv = new S3DModelBuffer::MeshDivision ;
			model.GetMeshDivisionList().Add( *pstrMeshID, pMeshDiv ) ;
			//
			for ( size_t i = 0; i < divinf.countMorphList; i ++ )
			{
				SString *	pstrMorphID = aStrings.GetAt( aMorphEntry.At(i) ) ;
				if ( pstrMorphID == nullptr )
				{
					file.AscendChunk() ;
					return	sglErrFailed ;
				}
				pMeshDiv->m_aMorphEntries.Add( new SString(*pstrMorphID) ) ;
			}
			for ( size_t iDiv = 0; iDiv < divinf.countDivision; iDiv ++ )
			{
				if ( file.DescendChunk( "div_mesh" ) )
				{
					file.AscendChunk() ;
					return	sglErrFailed ;
				}
				uint32_t	iDivMeshID = 0 ;
				if ( file.Read( &iDivMeshID, sizeof(uint32_t) ) < sizeof(uint32_t) )
				{
					file.AscendChunk() ;
					return	sglErrFailed ;
				}
				file.Read
					( aMorphEntry.GetArray(divinf.countMorphList),
								divinf.countMorphList * sizeof(uint32_t) ) ;
				aMorphEntry.FinishArray() ;
				file.AscendChunk() ;
				//
				SString *	pstrDivMeshID = aStrings.GetAt( iDivMeshID ) ;
				if ( pstrDivMeshID == nullptr )
				{
					file.AscendChunk() ;
					return	sglErrFailed ;
				}
				S3DModelBuffer::MeshDivision::SplittedEntry *
					pSplitted = new S3DModelBuffer::MeshDivision::SplittedEntry ;
				pSplitted->m_strSplittedMesh = *pstrDivMeshID ;
				for ( size_t i = 0; i < divinf.countMorphList; i ++ )
				{
					SString *	pstrMorphID = aStrings.GetAt( aMorphEntry.At(i) ) ;
					if ( pstrMorphID == nullptr )
					{
						file.AscendChunk() ;
						return	sglErrFailed ;
					}
					pSplitted->m_aSplittedMorph.Add( new SString(*pstrMorphID) ) ;
				}
				pMeshDiv->m_aSplittedEntries.Add( pSplitted ) ;
			}
		}
		file.AscendChunk() ;
	}
	return	sglErrSuccess ;
}

// ボーン読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdModelLoader::ReadBoneChunk
	( S3DModelBuffer & model, SSystem::SChunkFile& file )
{
	//
	// 定義文字列読み込み
	//
	if ( file.DescendChunk( "strings " ) )
	{
		return	sglErrFailed ;
	}
	SObjectArray<SString>	aStrings ;
	ReadStrings( aStrings, file ) ;
	file.AscendChunk() ;
	//
	SObjectArray<BoneInfo>	aBoneEntries ;
	for ( ; ; )
	{
		if ( file.DescendChunk() )
		{
			break ;
		}
		if ( file.IsEqualCurrentChunkID( "bone    " ) )
		do
		{
			//
			// ボーン基本情報
			//
			if ( file.DescendChunk( "boneinfo" ) )
			{
				break ;
			}
			BoneInfo *	pBoneInf = new BoneInfo ;
			aBoneEntries.Add( pBoneInf ) ;
			//
			file.Read( pBoneInf, sizeof(BoneInfo) ) ;
			file.AscendChunk() ;
			//
			SString *	pstrID = aStrings.GetAt( pBoneInf->iBoneID ) ;
			if ( pstrID == nullptr )
			{
				break ;
			}
			S3DModelBoneSpace *	pBone = new S3DModelBoneSpace ;
			S3DDVector	vBoneHandle = pBoneInf->vBoneHandle ;
			pBone->m_vCenter = pBoneInf->vBoneBase ;
			pBone->SetBoneHandle( vBoneHandle ) ;
			pBone->SetBoneFlags( pBoneInf->flagsBone ) ;
			pBone->SetBonePhysicalMaterial( pBoneInf->physMaterial ) ;
			if ( pBoneInf->flagsBone & S3DModelBoneSpace::flagHaveOrgMatrix )
			{
				pBone->SetOriginalBoneMatrix( pBoneInf->mat4OrgBone ) ;
			}
			//
			SString *	pstrMaterialRefID =
							aStrings.GetAt( pBoneInf->iMaterialRefID ) ;
			if ( pstrMaterialRefID != nullptr )
			{
				pBone->SetBonePhysicalMaterialID( *pstrMaterialRefID ) ;
			}
			//
			model.AddBonePropertyAs( *pstrID, pBone ) ;
			//
			// ウェイトマップ／物理演算・被影響ボーン
			//
			SObjectArray<S3DModelBuilder::WeightMap>	aWeightMaps ;
			for ( ; ; )
			{
				if ( file.DescendChunk() )
				{
					break ;
				}
				if ( file.IsEqualCurrentChunkID( "ik_param" ) )
				{
					//
					// IK 用パラメータ
					//
					S3DModelBoneSpace::IKParameter	param ;
					file.Read( &param, sizeof(S3DModelBoneSpace::IKParameter) ) ;
					pBone->SetIKParameter( param ) ;
				}
				else if ( file.IsEqualCurrentChunkID( "efphybon" ) )
				{
					// 物理演算・被影響ボーン
					for ( ; ; )
					{
						S3DStdModelLoader::PhysEffectiveBoneInfo	pebi ;
						const size_t	nInfoBytes =
								sizeof(S3DStdModelLoader::PhysEffectiveBoneInfo) ;
						if ( file.Read( &pebi, nInfoBytes ) < nInfoBytes )
						{
							break ;
						}
						S3DModelBoneSpace::EffectiveBoneEntry *
							pebe = new S3DModelBoneSpace::EffectiveBoneEntry ;
						SString *	pstrBoneID = aStrings.GetAt( pebi.iBoneID ) ;
						if ( pstrBoneID != nullptr )
						{
							pebe->m_strBoneID = *pstrBoneID ;
						}
						pebe->m_fpWeight = pebi.fpWeight ;
						pBone->AddEffectivePhysBone( pebe ) ;
						//
						if ( pebi.nBytes > nInfoBytes )
						{
							file.Seek
								( pebi.nBytes - nInfoBytes,
									SFileInterface::FromCurrent ) ;
						}
					}
				}
				else if ( file.IsEqualCurrentChunkID( "weightmp" ) )
				{
					// ウェイトマップ
					WeightMapInfo	wmi ;
					file.Read( &wmi, sizeof(WeightMapInfo) ) ;
					//
					S3DModelBuffer::MeshObject *
						pmo = model.GetMeshObjectAt( wmi.iMesh ) ;
					if ( pmo == nullptr )
					{
						file.AscendChunk() ;
						continue ;
					}
					S3DModelBuilder::WeightMap *
							pwm = new S3DModelBuilder::WeightMap ;
					pwm->m_iTargetMesh = wmi.iMesh ;
					pwm->m_matIMesh = wmi.matIMesh ;
					pwm->m_matRelMesh = wmi.matRelMesh ;
					//
					if ( wmi.nFlags & flagIndexedWeightMap )
					{
						IndexedWeightMap	ixwmp ;
						file.Read
							( ixwmp.bufWeight.GetArray(wmi.nCount),
										wmi.nCount * sizeof(float32_t) ) ;
						ixwmp.bufWeight.FinishArray() ;
						//
						file.Read
							( ixwmp.bufIndex.GetArray(wmi.nCount),
										wmi.nCount * sizeof(uint32_t) ) ;
						ixwmp.bufIndex.FinishArray() ;
						//
						ExpandIndexedWeightMap( pwm->m_bufWeight, ixwmp ) ;
					}
					else
					{
						file.Read
							( pwm->m_bufWeight.GetArray(wmi.nCount),
											wmi.nCount * sizeof(float32_t) ) ;
						pwm->m_bufWeight.FinishArray() ;
					}
					aWeightMaps.Add( pwm ) ;
				}
				file.AscendChunk() ;
			}
			//
			S3DModelBuilder::FlatWeightMap	fwm ;
			S3DModelBuilder::MergeBoneWeightMap
				( fwm, model, aWeightMaps.GetConstArray(), aWeightMaps.GetLength() ) ;
			if ( fwm.m_bufWeight.GetLength() > 0 )
			{
				pBone->SetBoneWeight
					( &model, fwm.m_iVertex, fwm.m_iNormal,
						fwm.m_bufWeight.GetLength(), fwm.m_bufWeight.GetConstArray() ) ;
				//
				for ( size_t i = 0; i < aWeightMaps.GetLength(); i ++ )
				{
					S3DModelBuilder::WeightMap *	pwm = aWeightMaps.GetAt( i ) ;
					pBone->AddEffectiveMeshIndex
						( pwm->m_iTargetMesh, pwm->m_matIMesh, pwm->m_matRelMesh ) ;
				}
			}
			else
			{
				pBone->AttachModel( &model ) ;
			}
		}
		while ( false ) ;
		else if ( file.IsEqualCurrentChunkID( "physpalt" ) )
		{
			//
			// 物理演算パラメータパレット
			//
			SStrSortArray<S3DModelBoneSpace::PhysMaterial>&
							physPalette = model.GetPhysMaterialList() ;
			for ( ; ; )
			{
				if ( file.DescendChunk() )
				{
					break ;
				}
				if ( file.IsEqualCurrentChunkID( "material" ) )
				{
					S3DModelBoneSpace::PhysMaterial	physMaterial ;
					uint32_t	iRefID ;
					file.Read( &iRefID, sizeof(uint32_t) ) ;
					file.Read( &physMaterial, sizeof(S3DModelBoneSpace::PhysMaterial) ) ;
					//
					SString *	pstrMaterialID = aStrings.GetAt( iRefID ) ;
					if ( pstrMaterialID != nullptr )
					{
						physPalette.SetAs( *pstrMaterialID, physMaterial ) ;
					}
				}
				file.AscendChunk() ;
			}
		}
		file.AscendChunk() ;
	}
	//
	// ボーン・ツリー構造構築
	//
	for ( size_t i = 0; i < aBoneEntries.GetLength(); i ++ )
	{
		BoneInfo *	pBoneInf = aBoneEntries.GetAt( i ) ;
		ESLAssert( pBoneInf != nullptr ) ;
		//
		SString *	pstrID = aStrings.GetAt( pBoneInf->iBoneID ) ;
		if ( pstrID == nullptr )
		{
			continue ;
		}
		S3DModelBoneSpace *	pBone = model.GetBonePropertyAs( *pstrID ) ;
		ESLAssert( pBone != nullptr ) ;
		//
		if ( pBoneInf->iParentID == (uint32_t) -1 )
		{
			model.GetBoneRoot().AddChild( pBone ) ;
		}
		else
		{
			SString *	pstrParentID = aStrings.GetAt( pBoneInf->iParentID ) ;
			if ( pstrParentID != nullptr )
			{
				S3DModelBoneSpace *	pParentBone =
						model.GetBonePropertyAs( *pstrParentID ) ;
				if ( pParentBone != nullptr )
				{
					pParentBone->AddChild( pBone ) ;
				}
			}
		}
	}
	model.BuildupBoneRelation() ;
	//
	return	sglErrSuccess ;
}

// マーカー情報読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdModelLoader::ReadMarkerInfoChunk
	( S3DModelBuffer & model, SSystem::SChunkFile& file )
{
	SXMLDocument	xmlDoc ;
	if ( xmlDoc.ReadDocument( file, xmlDoc ) )
	{
		return	sglErrFailed ;
	}
	SXMLDocument *	pxmlMarkers = xmlDoc.GetElementTagAs( L"markers" ) ;
	if ( pxmlMarkers != nullptr )
	{
		model.ImportMarkerXML( *pxmlMarkers, true ) ;
	}
	return	sglErrSuccess ;
}

// クロスシミュレーターメッシュ情報読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdModelLoader::ReadClothMeshInfoChunk
	( S3DModelBuffer & model, SSystem::SChunkFile& file )
{
	SXMLDocument	xmlDoc ;
	if ( xmlDoc.ReadDocument( file, xmlDoc ) )
	{
		return	sglErrFailed ;
	}
	SXMLDocument *	pxmlCloth = xmlDoc.GetElementTagAs( L"cloth_entries" ) ;
	if ( pxmlCloth != nullptr )
	{
//		model.ImportClothMeshXML( *pxmlCloth, true ) ;
	}
	return	sglErrSuccess ;
}

// ポーズライブラリ読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdModelLoader::ReadPosesChunk
	( S3DModelBuffer & model, SSystem::SChunkFile& file )
{
	return	model.GetPoseLibrary().ReadLibraryChunk( file ) ;
}

// メタ情報読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdModelLoader::ReadMetaInfoChunk
	( S3DModelBuffer & model, SSystem::SChunkFile& file )
{
	uint32_t	nFlags ;
	file.Read( &nFlags, sizeof(uint32_t) ) ;
	//
	SParserErrorTracer	perrTrace ;
	if ( model.EditMetaInfo().ReadDocument( file, perrTrace ) )
	{
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

// コンポジション読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdModelLoader::ReadSceneChunk
	( S3DModelBuffer & model, SSystem::SChunkFile& file )
{
	uint32_t	nFlags ;
	file.Read( &nFlags, sizeof(uint32_t) ) ;
	//
	SParserErrorTracer	perrTrace ;
	if ( model.EditSceneComposition().ReadDocument( file, perrTrace ) )
	{
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
}

// 拡張データ読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdModelLoader::ReadUserChunk
	( S3DModelBuffer & model, SSystem::SChunkFile& file )
{
	return	sglErrSuccess ;
}

// 文字列配列読み込み
//////////////////////////////////////////////////////////////////////////////
void S3DStdModelLoader::ReadStrings
	( SSystem::SObjectArray<SSystem::SString>& aStrings,
								SSystem::SFileInterface& file )
{
	SArray<uint16_t>	bufStrings ;
	size_t		nStrLen = (size_t) file.GetLength() / sizeof(uint16_t) ;
	uint16_t *	pwStrBuf = bufStrings.GetArray( nStrLen ) ;
	file.Read( pwStrBuf, nStrLen * sizeof(uint16_t) ) ;
	bufStrings.FinishArray() ;
	//
	size_t	iLastStr = 0 ;
	for ( size_t i = 0; i < nStrLen; i ++ )
	{
		if ( pwStrBuf[i] == 0 )
		{
			aStrings.Add( new SString( pwStrBuf + iLastStr ) ) ;
			iLastStr = i + 1 ;
		}
	}
	if ( iLastStr < nStrLen )
	{
		aStrings.Add
			( new SString
				( pwStrBuf + iLastStr, (ssize_t) (nStrLen - iLastStr) ) ) ;
	}
}

// 指標付きウェイトマップ展開
//////////////////////////////////////////////////////////////////////////////
void S3DStdModelLoader::ExpandIndexedWeightMap
	( SSystem::SArray<float32_t>& bufWeight,
			const S3DStdModelLoader::IndexedWeightMap& iwmSrcMap )
{
	size_t	nCount = iwmSrcMap.bufWeight.GetLength() ;
	ESLAssert( nCount == iwmSrcMap.bufIndex.GetLength() ) ;
	const float32_t *	pfpSrc = iwmSrcMap.bufWeight.GetConstArray() ;
	const uint32_t *	pIndex = iwmSrcMap.bufIndex.GetConstArray() ;
	uint32_t			nMaxIndex = 0 ;
	bool				flagEmpty = true ;
	size_t				i ;
	for ( i = 0; i < nCount; i ++ )
	{
		if ( nMaxIndex < pIndex[i] )
		{
			nMaxIndex = pIndex[i] ;
			flagEmpty = false ;
		}
	}
	if ( !flagEmpty )
	{
		float32_t *	pfpDst = bufWeight.GetArray( nMaxIndex + 1 ) ;
		for ( i = 0; i < nCount; i ++ )
		{
			pfpDst[pIndex[i]] = pfpSrc[i] ;
		}
		bufWeight.FinishArray() ;
	}
}

// 指標付きウェイトマップ精製
//////////////////////////////////////////////////////////////////////////////
void S3DStdModelLoader::MakeIndexedWeightMap
	( S3DStdModelLoader::IndexedWeightMap& iwmDstMap,
				const float32_t * pfpWeight, size_t nCount )
{
	float32_t *	pfpDst = iwmDstMap.bufWeight.GetArray( nCount ) ;
	uint32_t *	pIndex = iwmDstMap.bufIndex.GetArray( nCount ) ;
	size_t		iDst = 0 ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		if ( fabs( pfpWeight[i] ) > 0.0000001f )
		{
			pfpDst[iDst] = pfpWeight[i] ;
			pIndex[iDst] = (uint32_t) i ;
			iDst ++ ;
		}
	}
	iwmDstMap.bufWeight.FinishArray() ;
	iwmDstMap.bufIndex.FinishArray() ;
	ESLAssert( iDst <= nCount ) ;
	iwmDstMap.bufWeight.SetLength( iDst ) ;
	iwmDstMap.bufIndex.SetLength( iDst ) ;
}



//////////////////////////////////////////////////////////////////////////////
// EntisGLS4 標準 XML 形式モデル・ローダー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DStdXMLModelLoader, S3DModelLoaderInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DStdXMLModelLoader::S3DStdXMLModelLoader( void )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DStdXMLModelLoader::~S3DStdXMLModelLoader( void )
{
}

// ファイル拡張子判定
//////////////////////////////////////////////////////////////////////////////
bool S3DStdXMLModelLoader::IsMatchableFileExtension( const wchar_t * pszExt )
{
	return	(SString::CompareNoCase( pszExt, L"xmlmdf" ) == 0) ;
}

// MIME 判定
//////////////////////////////////////////////////////////////////////////////
bool S3DStdXMLModelLoader::IsMatchableMIMEType( const wchar_t * pszMIME )
{
	return	(SString::CompareNoCase( pszMIME, L"application/x-xmlmdf" ) == 0) ;
}

// モデルデータ読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdXMLModelLoader::ReadModel
	( S3DModelBuffer & model, SSystem::SFileInterface & file )
{
	SXMLDocument	xmlDoc ;
	SError	err = xmlDoc.ReadDocument( file, xmlDoc ) ;
	if ( err )
	{
		return	sglErrFailed ;
	}
	SXMLDocument *	pxmlModel = xmlDoc.GetElementTagAs( L"model" ) ;
	if ( pxmlModel == nullptr )
	{
		return	sglErrFailed ;
	}
	return	ParseModel( model, *pxmlModel, file ) ;
}

// モデルデータ解釈
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdXMLModelLoader::ParseModel
	( S3DModelBuffer & model,
		SSystem::SXMLDocument & xmlModel, SSystem::SFileOpener & opener )
{
	SGLError		err ;
	SXMLDocument *	pxmlTextures = xmlModel.GetElementTagAs( L"textures" ) ;
	if ( pxmlTextures != nullptr )
	{
		err = ParseTextureTag( model, *pxmlTextures, opener ) ;
		if ( err )
		{
			return	err ;
		}
	}
	SXMLDocument *	pxmlMaterials = xmlModel.GetElementTagAs( L"materials" ) ;
	if ( pxmlMaterials != nullptr )
	{
		err = ParseMaterialTag( model, *pxmlMaterials ) ;
		if ( err )
		{
			return	err ;
		}
	}
	SXMLDocument *	pxmlMeshs = xmlModel.GetElementTagAs( L"meshs" ) ;
	if ( pxmlMeshs != nullptr )
	{
		err = ParseMeshTag( model, *pxmlMeshs ) ;
		if ( err )
		{
			return	err ;
		}
	}
	SXMLDocument *	pxmlMeshGroup = xmlModel.GetElementTagAs( L"mesh_group" ) ;
	if ( pxmlMeshGroup != nullptr )
	{
		err = ParseMeshGroupTag( model, *pxmlMeshGroup ) ;
		if ( err )
		{
			return	err ;
		}
	}
	SXMLDocument *	pxmlMorphMesh = xmlModel.GetElementTagAs( L"morph_meshs" ) ;
	if ( pxmlMorphMesh != nullptr )
	{
		err = ParseMorphMeshTag( model, *pxmlMorphMesh ) ;
		if ( err )
		{
			return	err ;
		}
	}
	SXMLDocument *	pxmlMeshDiv = xmlModel.GetElementTagAs( L"mesh_div" ) ;
	if ( pxmlMeshDiv != nullptr )
	{
		err = ParseMeshDivTag( model, *pxmlMeshDiv ) ;
		if ( err )
		{
			return	err ;
		}
	}
	SXMLDocument *	pxmlBones = xmlModel.GetElementTagAs( L"bones" ) ;
	if ( pxmlBones != nullptr )
	{
		err = ParseBoneTag( model, *pxmlBones ) ;
		if ( err )
		{
			return	err ;
		}
	}
	SXMLDocument *	pxmlMarkers = xmlModel.GetElementTagAs( L"markers" ) ;
	if ( pxmlMarkers != nullptr )
	{
		err = ParseMarkerInfoTag( model, *pxmlMarkers ) ;
		if ( err )
		{
			return	err ;
		}
	}
	SXMLDocument *	pxmlCloth = xmlModel.GetElementTagAs( L"cloth_entries" ) ;
	if ( pxmlCloth != nullptr )
	{
		err = ParseClothMeshInfoTag( model, *pxmlCloth ) ;
		if ( err )
		{
			return	err ;
		}
	}
	SXMLDocument *	pxmlPoses = xmlModel.GetElementTagAs( L"poses" ) ;
	if ( pxmlPoses != nullptr )
	{
		err = ParsePosesTag( model, *pxmlPoses ) ;
		if ( err )
		{
			return	err ;
		}
	}
	SXMLDocument *	pxmlMetaInfo = xmlModel.GetElementTagAs( L"meta_info" ) ;
	if ( pxmlMetaInfo != nullptr )
	{
		model.EditMetaInfo() = *pxmlMetaInfo ;
	}
	SXMLDocument *	pxmlScene = xmlModel.GetElementTagAs( L"scene" ) ;
	if ( pxmlScene != nullptr )
	{
		model.EditSceneComposition() = *pxmlScene ;
	}
	SXMLDocument *	pxmlExtensions = xmlModel.GetElementTagAs( L"extensions" ) ;
	if ( pxmlExtensions != nullptr )
	{
		for ( size_t i = 0; i < pxmlExtensions->GetElementsCount(); i ++ )
		{
			SXMLDocument *	pxmlTag = pxmlExtensions->GetElementAt( i ) ;
			ESLAssert( pxmlTag != nullptr ) ;
			err = ParseUserTag( model, *pxmlTag ) ;
			if ( err )
			{
				return	err ;
			}
		}
	}
	return	sglErrSuccess ;
}

// テクスチャ読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdXMLModelLoader::ParseTextureTag
	( S3DModelBuffer & model,
		SSystem::SXMLDocument & xmlTexture, SSystem::SFileOpener & opener )
{
	S3DTextureLibrary&	textures = model.GetTextureLibrary() ;
	//
	for ( size_t i = 0; i < xmlTexture.GetElementsCount(); i ++ )
	{
		SXMLDocument *	pxmlTag = xmlTexture.GetElementAt( i ) ;
		ESLAssert( pxmlTag != nullptr ) ;
		if ( pxmlTag->GetTag() != L"image" )
		{
			continue ;
		}
		SString	strID = pxmlTag->GetAttrStringAs( L"id" ) ;
		SString	strPath = pxmlTag->GetAttrStringAs( L"src" ) ;
		SSmartPointer<SFileInterface>
			pFile = opener.NewOpenFile( strPath, SFileOpener::shareRead ) ;
		if ( pFile == nullptr )
		{
			pFile = SFileOpener::DefaultNewOpenFile
							( strPath, SFileOpener::shareRead ) ;
			if ( pFile == nullptr )
			{
				Trace( "falied to open \'%s\' for texture.\n",
								strPath.ToCharArray().GetConstArray() ) ;
				return	sglErrFailed ;
			}
		}
		SString			strMIME = pxmlTag->GetAttrStringAs( L"mime" ) ;
		const wchar_t *	pwszMIME = strMIME ;
		if ( strMIME.IsEmpty() )
		{
			pwszMIME = nullptr ;
		}
		SGLImage *	pImage = new SGLImage ;
		if ( pImage->ReadImage( pFile, pwszMIME ) )
		{
			Trace( "falied to read \'%s\' for texture.\n",
							strPath.ToCharArray().GetConstArray() ) ;
			delete	pImage ;
			return	sglErrFailed ;
		}
		pImage->NormalizeToTexture() ;
//		pImage->NormalizeToMipmapTexture() ;
		textures.AddSmartTextureAs( strID, pImage ) ;
	}
	return	sglErrSuccess ;
}

// 表面属性読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdXMLModelLoader::ParseMaterialTag
	( S3DModelBuffer & model, SSystem::SXMLDocument & xmlMaterial )
{
	S3DTextureLibrary&	textures = model.GetTextureLibrary() ;
	S3DMaterialLibrary&	materials = model.GetMaterialLibrary() ;
	return	materials.ParseXML( xmlMaterial, textures ) ;
}

// メッシュ読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdXMLModelLoader::ParseMeshTag
	( S3DModelBuffer & model, SSystem::SXMLDocument & xmlMesh )
{
	for ( size_t iTag = 0; iTag < xmlMesh.GetElementsCount(); iTag ++ )
	{
		SXMLDocument *	pxmlTag = xmlMesh.GetElementAt( iTag ) ;
		ESLAssert( pxmlTag != nullptr ) ;
		if ( (pxmlTag->GetTag() != L"triangle_strip")
			&& (pxmlTag->GetTag() != L"indexed_triangle_list")
			&& (pxmlTag->GetTag() != L"primitive"))
		{
			continue ;
		}
		//
		// メッシュ情報
		//
		SString			strMaterialID = pxmlTag->GetAttrStringAs( L"material" ) ;
		S3DMaterial *	pMaterial =
							model.GetMaterialLibrary().
										GetMaterialAs( strMaterialID ) ;
		if ( pMaterial == nullptr )
		{
			Trace( "not found \'%s\' material.\n",
						strMaterialID.ToCharArray().GetConstArray() ) ;
			return	sglErrFailed ;
		}
		size_t	i, j ;
		size_t	countPolygon = (size_t) pxmlTag->GetAttrIntegerAs( L"polygons" ) ;
		size_t	countVertex = (size_t) pxmlTag->GetAttrIntegerAs( L"vertics" ) ;
		size_t	countExAttr = (size_t) pxmlTag->GetAttrIntegerAs( L"ex_attr_count" ) ;
		//
		SXMLDocument::AttrInteger	aiPrimitiveTypes[] =
		{
			{ L"point", primitivePoint },
			{ L"line", primitiveLine },
			{ L"triangle", primitiveTriangle },
			{ nullptr, 0 },
		} ;
		S3DPrimitiveType
			typePrimitive =
				(S3DPrimitiveType) pxmlTag->GetAttrSymbolizedIntegerAs
					( L"primitive_type", aiPrimitiveTypes, primitiveTriangle ) ;
		//
		// 頂点バッファ
		//
		SArray<double>		bufNumber ;
		SArray<int64_t>		bufInteger ;
		SArray<S3DVector4>	bufVertex ;
		SArray<S3DVector4>	bufNormal ;
		SArray<S2DVector>	bufUVMap ;
		SArray<S3DColor>	bufColor ;
		SArray<float32_t>	bufExAttr ;
		SArray<uint32_t>	bufIndexed ;
		//
		SStringParser	sparsList ;
		double *		pBufNum = bufNumber.GetArray( countVertex * 3 ) ;
		int64_t *		pBufInt =
			bufInteger.GetArray
				( (size_t) esl_max( (int) countPolygon * 3,
										(int) countVertex * 2 ) ) ;
		//
		S3DVector4 *	pvVertex = bufVertex.GetArray( countVertex ) ;
		sparsList = pxmlTag->GetContentsAsString( L"vertex" ) ;
		sparsList.ParseNumberArray( pBufNum, countVertex * 3 ) ;
		bufNumber.FinishArray() ;
		//
		for ( i = 0, j = 0; i < countVertex; i ++, j += 3 )
		{
			pvVertex[i].x = (float32_t) pBufNum[j] ;
			pvVertex[i].y = (float32_t) pBufNum[j + 1] ;
			pvVertex[i].z = (float32_t) pBufNum[j + 2] ;
		}
		bufVertex.FinishArray() ;
		//
		// 法線
		//
		sparsList = pxmlTag->GetContentsAsString( L"normal" ) ;
		if ( !sparsList.IsEmpty() )
		{
			S3DVector4 *	pvNormal = bufNormal.GetArray( countVertex ) ;
			sparsList.ParseNumberArray( pBufNum, countVertex * 3 ) ;
			for ( i = 0, j = 0; i < countVertex; i ++, j += 3 )
			{
				pvNormal[i].x = (float32_t) pBufNum[j] ;
				pvNormal[i].y = (float32_t) pBufNum[j + 1] ;
				pvNormal[i].z = (float32_t) pBufNum[j + 2] ;
			}
			bufNormal.FinishArray() ;
		}
		//
		// UV マップ
		//
		sparsList = pxmlTag->GetContentsAsString( L"uv_map" ) ;
		if ( !sparsList.IsEmpty() )
		{
			S2DVector *	pvUVMap = bufUVMap.GetArray( countVertex ) ;
			sparsList.ParseNumberArray( pBufNum, countVertex * 2 ) ;
			for ( i = 0, j = 0; i < countVertex; i ++, j += 2 )
			{
				pvUVMap[i].x = (float32_t) pBufNum[j] ;
				pvUVMap[i].y = (float32_t) pBufNum[j + 1] ;
			}
			bufUVMap.FinishArray() ;
		}
		//
		// 頂点色
		//
		sparsList = pxmlTag->GetContentsAsString( L"color_map" ) ;
		if ( !sparsList.IsEmpty() )
		{
			S3DColor *	pColor = bufColor.GetArray( countVertex ) ;
			sparsList.ParseHexIntegerArray( pBufInt, countVertex * 2 ) ;
			for ( i = 0, j = 0; i < countVertex; i ++, j += 2 )
			{
				pColor[i].rgbMul.ui32 = (uint32_t) pBufInt[j] ;
				pColor[i].rgbAdd.ui32 = (uint32_t) pBufInt[j + 1] ;
			}
			bufColor.FinishArray() ;
		}
		//
		// 拡張属性
		//
		sparsList = pxmlTag->GetContentsAsString( L"ex_attr_elements" ) ;
		if ( !sparsList.IsEmpty() )
		{
			SArray<double>	bufNumExAttr ;
			const size_t	nElCount = countExAttr * countVertex ;
			float32_t *		pfpExAttr = bufExAttr.GetArray( nElCount ) ;
			sparsList.ParseNumberArray( pBufNum, nElCount ) ;
			for ( i = 0; i < nElCount; i ++ )
			{
				pfpExAttr[i] = (float32_t) pBufNum[i] ;
			}
			bufExAttr.FinishArray() ;
			bufNumExAttr.FinishArray() ;
		}
		//
		// インデックス
		//
		sparsList = pxmlTag->GetContentsAsString( L"indexed_list" ) ;
		if ( !sparsList.IsEmpty() )
		{
			size_t		nIndexCount = countPolygon * 3 ;
			uint32_t *	pIndexedList = bufIndexed.GetArray( nIndexCount ) ;
			sparsList.ParseIntegerArray( pBufInt, nIndexCount ) ;
			for ( i = 0; i < nIndexCount; i ++ )
			{
				pIndexedList[i] = (uint32_t) pBufInt[i] ;
			}
			bufIndexed.FinishArray() ;
		}
		//
		// メッシュ追加
		//
		size_t	iMesh = model.GetMeshCount() ;
		if ( pxmlTag->GetTag() == L"triangle_strip" )
		{
			if ( countVertex < countPolygon + 2 )
			{
				return	sglErrFailed ;
			}
			model.AddTriangleStrip
				( pMaterial, 0, countPolygon,
					bufVertex.GetConstArray(), bufNormal.GetConstArray(),
					bufUVMap.GetConstArray(), bufColor.GetConstArray() ) ;
		}
		else
		{
			if ( bufIndexed.GetConstArray() == nullptr )
			{
				return	sglErrFailed ;
			}
			if ( pxmlTag->GetTag() == L"indexed_triangle_list" )
			{
				model.AddIndexedTriangleList
					( pMaterial, 0,
						countPolygon, countVertex,
						bufVertex.GetConstArray(), bufNormal.GetConstArray(),
						bufUVMap.GetConstArray(), bufColor.GetConstArray(),
						bufIndexed.GetConstArray() ) ;
			}
			else
			{
				model.AddIndexedPrimitiveList
					( pMaterial, 0, typePrimitive,
						countPolygon * GetPrimitiveVertexCount(typePrimitive),
						countVertex,
						bufVertex.GetConstArray(), bufNormal.GetConstArray(),
						bufUVMap.GetConstArray(), bufColor.GetConstArray(),
						bufIndexed.GetConstArray() ) ;
			}
			for ( size_t iSub = 0; iSub < VertexBuffer::countSubMesh; iSub ++ )
			{
				SString	strTag = L"indexed_list" ;
				strTag += SString( iSub + 1 ) ;
				sparsList = pxmlTag->GetContentsAsString( strTag ) ;
				if ( sparsList.IsEmpty() )
				{
					continue ;
				}
				size_t		nIndexCount = countPolygon * 3 ;
				nIndexCount =
					sparsList.ParseIntegerArray( pBufInt, nIndexCount ) ;
				size_t		nPolygonCount = nIndexCount / 3 ;
				nIndexCount = nPolygonCount * 3 ;
				//
				uint32_t *	pIndexedList = bufIndexed.GetArray( nIndexCount ) ;
				for ( i = 0; i < nIndexCount; i ++ )
				{
					pIndexedList[i] = (uint32_t) pBufInt[i] ;
				}
				model.UpdateSubIndexedTriangleList
					( iMesh, iSub, 0, nPolygonCount, pIndexedList ) ;
				bufIndexed.FinishArray() ;
			}
			//
			model.SetSubMeshDensity
				( iMesh, (float32_t) pxmlTag->GetAttrRealAs
									( L"sub_mesh_density", 4.0 ) ) ;
		}
		if ( (countExAttr > 0)
			&& (bufExAttr.GetLength() >= countExAttr * countVertex) )
		{
			model.SetExtendVertexAttribute
				( iMesh, countExAttr, countVertex, bufExAttr.GetConstArray() ) ;
		}
		bufInteger.FinishArray() ;
	}
	return	sglErrSuccess ;
}

// メッシュグループ情報読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdXMLModelLoader::ParseMeshGroupTag
	( S3DModelBuffer & model, SSystem::SXMLDocument & xmlGroup )
{
	for ( size_t i = 0; i < xmlGroup.GetElementsCount(); i ++ )
	{
		SXMLDocument *	pxmlTag = xmlGroup.GetElementAt( i ) ;
		ESLAssert( pxmlTag != nullptr ) ;
		if ( pxmlTag->GetTag() != L"group_item" )
		{
			continue ;
		}
		S3DModelBuffer::MeshGroup	mgroup ;
		SString	strID = pxmlTag->GetAttrStringAs( L"id" ) ;
		if ( !strID.IsEmpty() )
		{
			mgroup.m_iFirstMesh =
					(uint32_t) pxmlTag->GetAttrIntegerAs( L"first_mesh" ) ;
			mgroup.m_nMeshCount =
					(uint32_t) pxmlTag->GetAttrIntegerAs( L"mesh_count" ) ;
			mgroup.m_vCenter.x = pxmlTag->GetAttrRealAs( L"center_x" ) ;
			mgroup.m_vCenter.y = pxmlTag->GetAttrRealAs( L"center_y" ) ;
			mgroup.m_vCenter.z = pxmlTag->GetAttrRealAs( L"center_z" ) ;
			//
			model.GetMeshGroupList().Add( strID, mgroup ) ;
		}
	}
	return	sglErrSuccess ;
}

// モーフターゲット読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdXMLModelLoader::ParseMorphMeshTag
	( S3DModelBuffer & model, SSystem::SXMLDocument & xmlMorph )
{
	for ( size_t iTag = 0; iTag < xmlMorph.GetElementsCount(); iTag ++ )
	{
		SXMLDocument *	pxmlTag = xmlMorph.GetElementAt( iTag ) ;
		ESLAssert( pxmlTag != nullptr ) ;
		if ( pxmlTag->GetTag() != L"mesh" )
		{
			continue ;
		}
		SString	strMeshID = pxmlTag->GetAttrStringAs( L"id" ) ;
		if ( strMeshID.IsEmpty() )
		{
			continue ;
		}
		S3DModelBuffer::MorphTargetMesh *
				pmtm = new S3DModelBuffer::MorphTargetMesh ;
		size_t	countVertex =
				(size_t) pxmlTag->GetAttrIntegerAs( L"vertics" ) ;
		pmtm->m_countVertex = countVertex ;
		model.AddMorhTargetAs( strMeshID, pmtm ) ;
		//
		// 頂点バッファ
		//
		size_t			i, j ;
		SArray<double>	bufNumber ;
		SArray<int64_t>	bufInteger ;
		SStringParser	sparsList ;
		double *		pBufNum = bufNumber.GetArray( countVertex * 3 ) ;
		int64_t *		pBufInt = bufInteger.GetArray( countVertex * 2 ) ;
		//
		S3DVector4 *	pvVertex = pmtm->m_bufVertex.GetArray( countVertex ) ;
		sparsList = pxmlTag->GetContentsAsString( L"vertex" ) ;
		sparsList.ParseNumberArray( pBufNum, countVertex * 3 ) ;
		for ( i = 0, j = 0; i < countVertex; i ++, j += 3 )
		{
			pvVertex[i].x = (float32_t) pBufNum[j] ;
			pvVertex[i].y = (float32_t) pBufNum[j + 1] ;
			pvVertex[i].z = (float32_t) pBufNum[j + 2] ;
		}
		pmtm->m_bufVertex.FinishArray() ;
		//
		// 法線
		//
		sparsList = pxmlTag->GetContentsAsString( L"normal" ) ;
		if ( !sparsList.IsEmpty() )
		{
			S3DVector4 *	pvNormal = pmtm->m_bufNormal.GetArray( countVertex ) ;
			sparsList.ParseNumberArray( pBufNum, countVertex * 3 ) ;
			for ( i = 0, j = 0; i < countVertex; i ++, j += 3 )
			{
				pvNormal[i].x = (float32_t) pBufNum[j] ;
				pvNormal[i].y = (float32_t) pBufNum[j + 1] ;
				pvNormal[i].z = (float32_t) pBufNum[j + 2] ;
			}
			pmtm->m_bufNormal.FinishArray() ;
		}
		//
		// UV マップ
		//
		sparsList = pxmlTag->GetContentsAsString( L"uv_map" ) ;
		if ( !sparsList.IsEmpty() )
		{
			S2DVector *	pvUVMap = pmtm->m_bufUVMap.GetArray( countVertex ) ;
			sparsList.ParseNumberArray( pBufNum, countVertex * 2 ) ;
			for ( i = 0, j = 0; i < countVertex; i ++, j += 2 )
			{
				pvUVMap[i].x = (float32_t) pBufNum[j] ;
				pvUVMap[i].y = (float32_t) pBufNum[j + 1] ;
			}
			pmtm->m_bufUVMap.FinishArray() ;
		}
		//
		// 頂点色
		//
		sparsList = pxmlTag->GetContentsAsString( L"color_map" ) ;
		if ( !sparsList.IsEmpty() )
		{
			S3DColor *	pColor = pmtm->m_bufColor.GetArray( countVertex ) ;
			sparsList.ParseHexIntegerArray( pBufInt, countVertex * 2 ) ;
			for ( i = 0, j = 0; i < countVertex; i ++, j += 2 )
			{
				pColor[i].rgbMul.ui32 = (uint32_t) pBufInt[j] ;
				pColor[i].rgbAdd.ui32 = (uint32_t) pBufInt[j + 1] ;
			}
			pmtm->m_bufColor.FinishArray() ;
		}
		bufInteger.FinishArray() ;
		//
		// ウェイト
		//
		sparsList = pxmlTag->GetContentsAsString( L"weight_map" ) ;
		if ( !sparsList.IsEmpty() )
		{
			float32_t *	pfpWeightMap = pmtm->m_bufWeight.GetArray( countVertex ) ;
			sparsList.ParseNumberArray( pBufNum, countVertex ) ;
			for ( i = 0; i < countVertex; i ++ )
			{
				pfpWeightMap[i] = (float32_t) pBufNum[i] ;
			}
			pmtm->m_bufWeight.FinishArray() ;
		}
		bufNumber.FinishArray() ;
		//
		// 関連メッシュ
		//
		sparsList = pxmlTag->GetContentsAsString( L"rel_mesh" ) ;
		while ( sparsList.PassSpace() )
		{
			int	typeNum = sparsList.IsNextNumber() ;
			if ( typeNum == SStringParser::numberInvalid )
			{
				break ;
			}
			size_t	iRelMesh = (size_t) sparsList.NextInteger( typeNum ) ;
			//
			S3DModelBuffer::MeshObject *
					pMesh = model.GetMeshObjectAt( iRelMesh ) ;
			if ( pMesh != nullptr )
			{
				pMesh->m_arrMorphTarget.Add( new SString( strMeshID ) ) ;
			}
		}
	}
	model.BuildupMeshMorphingTarget() ;
	return	sglErrSuccess ;
}

// 分割メッシュ情報読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdXMLModelLoader::ParseMeshDivTag
	( S3DModelBuffer & model, SSystem::SXMLDocument & xmlMeshDiv )
{
	for ( size_t i = 0; i < xmlMeshDiv.GetElementsCount(); i ++ )
	{
		SXMLDocument *	pxmlMesh = xmlMeshDiv.GetElementAt( i ) ;
		ESLAssert( pxmlMesh != nullptr ) ;
		if ( pxmlMesh->GetTag() != L"mesh" )
		{
			continue ;
		}
		SString	strMeshID = pxmlMesh->GetAttrStringAs( L"id" ) ;
		//
		S3DModelData::MeshDivision *	pMeshDiv = new S3DModelData::MeshDivision ;
		model.GetMeshDivisionList().Add( strMeshID, pMeshDiv ) ;
		//
		SXMLDocument *	pxmlMorphEntries =
							pxmlMesh->GetElementTagAs( L"morph_entries" ) ;
		if ( pxmlMorphEntries != nullptr )
		{
			for ( size_t j = 0; j < pxmlMorphEntries->GetElementsCount(); j ++ )
			{
				SXMLDocument *	pxmlEntry = pxmlMorphEntries->GetElementAt( j ) ;
				ESLAssert( pxmlEntry != nullptr ) ;
				if ( pxmlEntry->GetTag() != L"entry" )
				{
					continue ;
				}
				const SString *	pstrTargetID =
									pxmlEntry->GetAttributeAs( L"morph_target" ) ;
				if ( pstrTargetID != nullptr )
				{
					pMeshDiv->m_aMorphEntries.Add( new SString( *pstrTargetID ) ) ;
				}
			}
		}
		for ( size_t j = 0; j < pxmlMesh->GetElementsCount(); j ++ )
		{
			SXMLDocument *	pxmlSplitted = pxmlMesh->GetElementAt( j ) ;
			ESLAssert( pxmlSplitted != nullptr ) ;
			if ( pxmlSplitted->GetTag() != L"splitted_mesh" )
			{
				continue ;
			}
			const SString *	pstrSplittedMeshID = pxmlSplitted->GetAttributeAs( L"id" ) ;
			if ( pstrSplittedMeshID == nullptr )
			{
				continue ;
			}
			S3DModelData::MeshDivision::SplittedEntry *
				pSplitted = new S3DModelData::MeshDivision::SplittedEntry ;
			pSplitted->m_strSplittedMesh = *pstrSplittedMeshID ;
			pMeshDiv->m_aSplittedEntries.Add( pSplitted ) ;
			//
			for ( size_t k = 0; k < pxmlSplitted->GetElementsCount(); k ++ )
			{
				SXMLDocument *	pxmlEntry = pxmlSplitted->GetElementAt( k ) ;
				ESLAssert( pxmlEntry != nullptr ) ;
				if ( pxmlEntry->GetTag() != L"entry" )
				{
					continue ;
				}
				const SString *	pstrTargetID =
									pxmlEntry->GetAttributeAs( L"morph_target" ) ;
				if ( pstrTargetID != nullptr )
				{
					pSplitted->m_aSplittedMorph.Add( new SString( *pstrTargetID ) ) ;
				}
			}
		}
	}
	return	sglErrSuccess ;
}

// ボーン読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdXMLModelLoader::ParseBoneTag
	( S3DModelBuffer & model, SSystem::SXMLDocument & xmlBone )
{
	SObjectArray<BoneRelInfo>	aBoneRel ;
	for ( size_t i = 0; i < xmlBone.GetElementsCount(); i ++ )
	{
		SXMLDocument *	pxmlTag = xmlBone.GetElementAt( i ) ;
		ESLAssert( pxmlTag != nullptr ) ;
		if ( pxmlTag->GetTag() != L"bone" )
		{
			continue ;
		}
		SString	strBoneID = pxmlTag->GetAttrStringAs( L"id" ) ;
		if ( strBoneID.IsEmpty() )
		{
			continue ;
		}
		//
		// ボーンパラメータ
		//
		S3DModelBoneSpace *	pBone = new S3DModelBoneSpace ;
		model.AddBonePropertyAs( strBoneID, pBone ) ;
		//
		pBone->m_vCenter.x = pxmlTag->GetAttrRealAs( L"x" ) ;
		pBone->m_vCenter.y = pxmlTag->GetAttrRealAs( L"y" ) ;
		pBone->m_vCenter.z = pxmlTag->GetAttrRealAs( L"z" ) ;
		//
		S3DDVector	vHandle ;
		vHandle.x = pxmlTag->GetAttrRealAs( L"handle_x" ) ;
		vHandle.y = pxmlTag->GetAttrRealAs( L"handle_y" ) ;
		vHandle.z = pxmlTag->GetAttrRealAs( L"handle_z" ) ;
		pBone->SetBoneHandle( vHandle ) ;
		//
		ParseBonePhysParameters( pBone, *pxmlTag ) ;
		//
		// 親子関係
		//
		BoneRelInfo *	pbri = new BoneRelInfo ;
		pbri->m_pBone = pBone ;
		pbri->m_strParentID = pxmlTag->GetAttrStringAs( L"parent" ) ;
		aBoneRel.Add( pbri ) ;
		//
		// ウェイトマップ
		//
		SObjectArray<S3DModelBuilder::WeightMap>	aWeightMaps ;
		SXMLDocument *	pxmlWeightMaps =
							pxmlTag->GetElementTagAs( L"weight_maps" ) ;
		if ( pxmlWeightMaps != nullptr )
		{
			for ( size_t j = 0; j < pxmlWeightMaps->GetElementsCount(); j ++ )
			{
				SXMLDocument *	pxmlMap = pxmlWeightMaps->GetElementAt( j ) ;
				ESLAssert( pxmlMap != nullptr ) ;
				if ( pxmlMap->GetTag() != L"weight_map" )
				{
					continue ;
				}
				SString *	pstrMap = pxmlMap->GetTextElement() ;
				if ( pstrMap == nullptr )
				{
					continue ;
				}
				S3DModelBuilder::WeightMap *
							pwm = new S3DModelBuilder::WeightMap ;
				pwm->m_iTargetMesh =
					(size_t) pxmlMap->GetAttrIntegerAs( L"target_mesh" ) ;
				ParseMatrix4x4
					( pwm->m_matIMesh,
						pxmlMap->GetAttrStringAs( L"inv_mesh_matrix" ) ) ;
				ParseMatrix4x4
					( pwm->m_matRelMesh,
						pxmlMap->GetAttrStringAs( L"rel_mesh_matrix" ) ) ;
				aWeightMaps.Add( pwm ) ;
				//
				SStringParser	sparsMap ;
				SArray<double>	aWeightMap ;
				size_t	nCount = (size_t) pxmlMap->GetAttrIntegerAs( L"count" ) ;
				//
				sparsMap.AttachString( *pstrMap ) ;
				nCount = sparsMap.ParseNumberArray
							( aWeightMap.GetArray( nCount ), nCount ) ;
				aWeightMap.FinishArray() ;
				//
				float32_t *		pfpWeightMap = pwm->m_bufWeight.GetArray( nCount ) ;
				const double *	pdbWeightMap = aWeightMap.GetConstArray() ;
				for ( size_t k = 0; k < nCount; k ++ )
				{
					pfpWeightMap[k] = (float32_t) pdbWeightMap[k] ;
				}
				pwm->m_bufWeight.FinishArray() ;
			}
		}
		S3DModelBuilder::FlatWeightMap	fwm ;
		S3DModelBuilder::MergeBoneWeightMap
			( fwm, model, aWeightMaps.GetConstArray(), aWeightMaps.GetLength() ) ;
		if ( fwm.m_bufWeight.GetLength() > 0 )
		{
			pBone->SetBoneWeight
				( &model, fwm.m_iVertex, fwm.m_iNormal,
					fwm.m_bufWeight.GetLength(), fwm.m_bufWeight.GetConstArray() ) ;
				//
				for ( size_t i = 0; i < aWeightMaps.GetLength(); i ++ )
				{
					S3DModelBuilder::WeightMap *	pwm = aWeightMaps.GetAt( i ) ;
					pBone->AddEffectiveMeshIndex
						( pwm->m_iTargetMesh, pwm->m_matIMesh, pwm->m_matRelMesh ) ;
				}
		}
		else
		{
			pBone->AttachModel( &model ) ;
		}
	}
	//
	// ボーン・ツリー構造構築
	//
	for ( size_t i = 0; i < aBoneRel.GetLength(); i ++ )
	{
		BoneRelInfo *	pbri = aBoneRel.GetAt( i ) ;
		ESLAssert( pbri != nullptr ) ;
		//
		if ( pbri->m_strParentID.IsEmpty() )
		{
			model.GetBoneRoot().AddChild( pbri->m_pBone ) ;
		}
		else
		{
			S3DModelBoneSpace *	pParentBone =
					model.GetBonePropertyAs( pbri->m_strParentID ) ;
			if ( pParentBone != nullptr )
			{
				pParentBone->AddChild( pbri->m_pBone ) ;
			}
		}
	}
	model.BuildupBoneRelation() ;
	//
	SXMLDocument *	pxmlMatPalette =
					xmlBone.GetElementTagAs( L"phys_material_palette" ) ;
	if ( pxmlMatPalette != nullptr )
	{
		SStrSortArray<S3DModelBoneSpace::PhysMaterial>&
						physPalette = model.GetPhysMaterialList() ;
		for ( size_t i = 0; i < pxmlMatPalette->GetElementsCount(); i ++ )
		{
			SXMLDocument *	pxmlTag = pxmlMatPalette->GetElementAt( i ) ;
			ESLAssert( pxmlTag != nullptr ) ;
			if ( pxmlTag->GetTag() != L"phys_material" )
			{
				continue ;
			}
			SString	strID = pxmlTag->GetAttrStringAs( L"id" ) ;
			//
			S3DModelBoneSpace::PhysMaterial	physMaterial ;
			ParseBonePhysMaterial( physMaterial, *pxmlTag ) ;
			physPalette.SetAs( strID, physMaterial ) ;
		}
	}
	return	sglErrSuccess ;
}

SGLError S3DStdXMLModelLoader::ParseBoneTagPhysics
	( S3DModelBuffer & model, SSystem::SXMLDocument & xmlBone )
{
	SObjectArray<BoneRelInfo>	aBoneRel ;
	for ( size_t i = 0; i < xmlBone.GetElementsCount(); i ++ )
	{
		SXMLDocument *	pxmlTag = xmlBone.GetElementAt( i ) ;
		ESLAssert( pxmlTag != nullptr ) ;
		if ( pxmlTag->GetTag() != L"bone" )
		{
			continue ;
		}
		SString	strBoneID = pxmlTag->GetAttrStringAs( L"id" ) ;
		if ( strBoneID.IsEmpty() )
		{
			continue ;
		}
		//
		// ボーンパラメータ
		//
		S3DModelBoneSpace *	pBone = model.GetBonePropertyAs( strBoneID ) ;
		if ( pBone == nullptr )
		{
			continue ;
		}
		ParseBonePhysParameters( pBone, *pxmlTag ) ;
	}
	SXMLDocument *	pxmlMatPalette =
					xmlBone.GetElementTagAs( L"phys_material_palette" ) ;
	if ( pxmlMatPalette != nullptr )
	{
		SStrSortArray<S3DModelBoneSpace::PhysMaterial>&
						physPalette = model.GetPhysMaterialList() ;
		for ( size_t i = 0; i < pxmlMatPalette->GetElementsCount(); i ++ )
		{
			SXMLDocument *	pxmlTag = pxmlMatPalette->GetElementAt( i ) ;
			ESLAssert( pxmlTag != nullptr ) ;
			if ( pxmlTag->GetTag() != L"phys_material" )
			{
				continue ;
			}
			SString	strID = pxmlTag->GetAttrStringAs( L"id" ) ;
			//
			S3DModelBoneSpace::PhysMaterial	physMaterial ;
			ParseBonePhysMaterial( physMaterial, *pxmlTag ) ;
			physPalette.SetAs( strID, physMaterial ) ;
		}
	}
	return	sglErrSuccess ;
}

void S3DStdXMLModelLoader::ParseBonePhysParameters
	( S3DModelBoneSpace* pBone, const SSystem::SXMLDocument& xmlTag )
{
	SXMLDocument::AttrInteger	aiBoneFlags[] =
	{
		{ L"physics", S3DModelBoneSpace::flagBonePhysics },
		{ L"no_collision", S3DModelBoneSpace::flagNoCollision },
		{ L"track_pos", S3DModelBoneSpace::flagTrackingPos },
		{ nullptr, 0 },
	} ;
	uint32_t	nFlags ;
	nFlags = (uint32_t) xmlTag.GetAttrComplexIntegerAs
									( L"flags", aiBoneFlags ) ;
	pBone->SetBoneFlags( nFlags ) ;
	//
	SString *	pstrOrgMatrix =
					xmlTag.GetTextElementAs( L"original_matrix" ) ;
	if ( pstrOrgMatrix != nullptr )
	{
		S4DMatrix	mat4Org ;
		ParseMatrix4x4( mat4Org, *pstrOrgMatrix ) ;
		//
		pBone->SetBoneFlags
			( nFlags | S3DModelBoneSpace::flagHaveOrgMatrix ) ;
		pBone->SetOriginalBoneMatrix( mat4Org ) ;
	}
	//
	SXMLDocument *	pxmlIKParam =
						xmlTag.GetElementTagAs( L"ik_parameter" ) ;
	if ( pxmlIKParam != nullptr )
	{
		SXMLDocument::AttrInteger	aiIKFlags[] =
		{
			{ L"min_bent", S3DModelBoneSpace::flagIKMinBent },
			{ L"max_bent", S3DModelBoneSpace::flagIKMaxBent },
			{ L"bend_direction", S3DModelBoneSpace::flagIKBendDirection },
			{ L"parent_axis", S3DModelBoneSpace::flagIKParentAxis },
			{ L"ik_terminate", S3DModelBoneSpace::flagIKTerminate },
			{ nullptr, 0 },
		} ;
		S3DModelBoneSpace::IKParameter	param ;
		param.nIKFlags =
			(uint32_t) pxmlIKParam->GetAttrComplexIntegerAs
									( L"flags", aiIKFlags ) ;
		param.fpWeight =
			(float32_t) pxmlIKParam->GetAttrRealAs( L"weight", 1.0 ) ;
		param.degMinBent =
			(float32_t) pxmlIKParam->GetAttrRealAs( L"min_bent", 0.0 ) ;
		param.degMaxBent =
			(float32_t) pxmlIKParam->GetAttrRealAs( L"max_bent", 180.0 ) ;
		param.vBendDirection.x =
			(float32_t) pxmlIKParam->GetAttrRealAs( L"bend_dir_x", 0.0 ) ;
		param.vBendDirection.y =
			(float32_t) pxmlIKParam->GetAttrRealAs( L"bend_dir_y", 0.0 ) ;
		param.vBendDirection.z =
			(float32_t) pxmlIKParam->GetAttrRealAs( L"bend_dir_z", 0.0 ) ;
	}
	//
	SXMLDocument *	pxmlMaterial =
						xmlTag.GetElementTagAs( L"phys_material" ) ;
	if ( pxmlMaterial != nullptr )
	{
		S3DModelBoneSpace::PhysMaterial	physMaterial ;
		ParseBonePhysMaterial( physMaterial, *pxmlMaterial ) ;
		pBone->SetBonePhysicalMaterial( physMaterial ) ;
		//
		pBone->SetBonePhysicalMaterialID
			( pxmlMaterial->GetAttrStringAs( L"ref_id" ) ) ;
	}
	//
	SXMLDocument *	pxmlPhysBones =
				xmlTag.GetElementTagAs( L"phys_effecive_bones" ) ;
	pBone->RemoveAllEffectivePhysBones() ;
	if ( pxmlPhysBones != nullptr )
	{
		for ( size_t j = 0; j < pxmlPhysBones->GetElementsCount(); j ++ )
		{
			SXMLDocument *	pxmlEffBone = pxmlPhysBones->GetElementAt( j ) ;
			ESLAssert( pxmlEffBone != nullptr ) ;
			if ( pxmlEffBone->GetTag() != L"effecive_bone" )
			{
				continue ;
			}
			S3DModelBoneSpace::EffectiveBoneEntry *
				pebe = new S3DModelBoneSpace::EffectiveBoneEntry ;
			pebe->m_strBoneID = pxmlEffBone->GetAttrStringAs( L"bone" ) ;
			pebe->m_fpWeight = pxmlEffBone->GetAttrRealAs( L"weight" ) ;
			pBone->AddEffectivePhysBone( pebe ) ;
		}
	}
}

void S3DStdXMLModelLoader::ParseBonePhysMaterial
	( S3DModelBoneSpace::PhysMaterial& physMaterial, const SXMLDocument& xmlTag )
{
	physMaterial.ParseXML( xmlTag ) ;
}

// マーカー情報読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdXMLModelLoader::ParseMarkerInfoTag
	( S3DModelBuffer & model, SSystem::SXMLDocument & xmlMarker )
{
	return	model.ImportMarkerXML( xmlMarker, true ) ;
}

// クロスシミュレーターメッシュ情報読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdXMLModelLoader::ParseClothMeshInfoTag
	( S3DModelBuffer & model, SSystem::SXMLDocument & xmlCloth )
{
//	return	model.ImportClothMeshXML( xmlCloth, true ) ;
	return	sglErrSuccess ;
}

// ポーズライブラリ読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdXMLModelLoader::ParsePosesTag
	( S3DModelBuffer & model, SSystem::SXMLDocument & xmlPose )
{
	return	model.GetPoseLibrary().ParseLibraryXML( xmlPose ) ;
}

// 拡張データ読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdXMLModelLoader::ParseUserTag
	( S3DModelBuffer & model, SSystem::SXMLDocument & xmlTag )
{
	return	sglErrSuccess ;
}

// 4x4 行列解釈
//////////////////////////////////////////////////////////////////////////////
void S3DStdXMLModelLoader::ParseMatrix4x4
	( S4DMatrix& mat4, const wchar_t * pwsz4x4 )
{
	SStringParser	sparsMatrix = pwsz4x4 ;
	double			bufMatrix[4][4] ;
	//
	size_t	nCount = sparsMatrix.ParseNumberArray( &bufMatrix[0][0], 16 ) ;
	if ( nCount < 16 )
	{
		mat4.InitializeMatrix( 1, 1, 1, 1 ) ;
		return ;
	}
	for ( int i = 0; i < 4; i ++ )
	{
		for ( int j = 0; j < 4; j ++ )
		{
			mat4.m[i][j] = (float32_t) bufMatrix[i][j] ;
		}
	}
}



//////////////////////////////////////////////////////////////////////////////
// モデル・ビルダ・メッシュ情報
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DModelBuilder::MeshEntry::MeshEntry( void )
{
	m_typeMesh = meshIndexedTriangleList ;
	m_pMaterial = nullptr ;
	m_countPolygon = 0 ;
	m_countVertex = 0 ;
}

S3DModelBuilder::MeshEntry::MeshEntry
		( const S3DModelBuilder::MeshEntry& mesh )
	: m_typeMesh( mesh.m_typeMesh ),
		m_pMaterial( mesh.m_pMaterial ),
		m_countPolygon( mesh.m_countPolygon ),
		m_countVertex( mesh.m_countVertex ),
		m_bufVertex( mesh.m_bufVertex ),
		m_bufNormal( mesh.m_bufNormal ),
		m_bufUVMap( mesh.m_bufUVMap ),
		m_bufColor( mesh.m_bufColor ), m_bufIndex( mesh.m_bufIndex )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DModelBuilder::MeshEntry::~MeshEntry( void )
{
}

// 法線の生成
//////////////////////////////////////////////////////////////////////////////
void S3DModelBuilder::MeshEntry::MakeNormal( double cosLimit )
{
	//
	// ポリゴン毎の法線計算
	//
	const S3DVector4 *	pvVertex = m_bufVertex.GetConstArray() ;
	ESLAssert( m_countVertex <= m_bufVertex.GetLength() ) ;
	//
	S3DTemporaryIndexTriangleStrip	tits ;
	uint32_t *	pIndexedList ;
	if ( m_typeMesh == meshIndexedTriangleList )
	{
		ESLAssert( m_countPolygon * 3 <= m_bufIndex.GetLength() ) ;
		pIndexedList = m_bufIndex.GetArray() ;
	}
	else if ( m_typeMesh == meshTriangleStrip )
	{
		 tits.MakeIndexList( m_countPolygon ) ;
		 pIndexedList = tits.GetArray() ;
	}
	else
	{
		return ;
	}
	SArray<S3DVector>	aPolyNormals ;
	S3DVector *	pvPolyNormals = aPolyNormals.GetArray( m_countPolygon ) ;
	size_t	i, j ;
	for ( i = 0, j = 0; i < m_countPolygon; i ++, j += 3 )
	{
		uint32_t	vi0 = pIndexedList[j] ;
		uint32_t	vi1 = pIndexedList[j + 1] ;
		uint32_t	vi2 = pIndexedList[j + 2] ;
		ESLAssert( vi0 < m_countVertex ) ;
		ESLAssert( vi1 < m_countVertex ) ;
		ESLAssert( vi2 < m_countVertex ) ;
		S3DVector	v0 = pvVertex[vi0] ;
		S3DVector	v1 = pvVertex[vi1] ;
		S3DVector	v2 = pvVertex[vi2] ;
		//
		S3DVector	vNormal = (v1 - v0) * (v2 - v0) ;
		vNormal.Normalize() ;
		pvPolyNormals[i] = vNormal ;
	}
	aPolyNormals.FinishArray() ;
	//
	// 法線を計算する
	//
	SArray<S3DVector>	vVertNormals ;
	S3DVector *			pvVertNormals =
							vVertNormals.GetArray( m_countPolygon * 3 ) ;
	for ( i = 0; i < m_countPolygon; i ++ )
	{
		const size_t		ii = i * 3 ;
		const size_t		vi0 = pIndexedList[ii] ;
		const size_t		vi1 = pIndexedList[ii + 1] ;
		const size_t		vi2 = pIndexedList[ii + 2] ;
		const S3DVector&	vvi0 = pvVertex[vi0] ;
		const S3DVector&	vvi1 = pvVertex[vi1] ;
		const S3DVector&	vvi2 = pvVertex[vi2] ;
		const S3DVector	vPolyNormal( pvPolyNormals[i] ) ;
		pvVertNormals[ii] =
			pvVertNormals[ii + 1] =
			pvVertNormals[ii + 2] = vPolyNormal ;
		//
		for ( j = 0; j < i; j ++ )
		{
			//
			// 共有頂点判定
			//
			const size_t	ij = j * 3 ;
			for ( size_t k = 0; k < 3; k ++ )
			{
				size_t	vjk = pIndexedList[ij + k] ;
				const S3DVector&	vvjk = pvVertex[vjk] ;
				if ( (vvi0 == vvjk) || (vvi1 == vvjk) || (vvi2 == vvjk) )
				{
					const size_t	ii0 =
							((vvi0 == vvjk) ? 0 : ((vvi1 == vvjk) ? 1 : 2)) ;
					if ( (vPolyNormal | pvPolyNormals[j]) >= cosLimit )
					{
						//
						// スムージング処理
						//
						ESLAssert( ij + k < m_countPolygon * 3 ) ;
						ESLAssert( ii + ii0 < m_countPolygon * 3 ) ;
						pvVertNormals[ij + k] += vPolyNormal ;
						pvVertNormals[ii + ii0] += pvPolyNormals[j] ;
					}
				}
			}
		}
	}
	for ( i = 0, j = 0; i < m_countPolygon; i ++, j += 3 )
	{
		pvVertNormals[j].Normalize() ;
		pvVertNormals[j + 1].Normalize() ;
		pvVertNormals[j + 2].Normalize() ;
	}
	vVertNormals.FinishArray() ;
	//
	// 法線の重複判定／頂点を増やす
	//
	S3DVector4 *	pvNormal = m_bufNormal.GetArray( m_countVertex ) ;
	for ( i = 0, j = 0; i < m_countPolygon; i ++, j += 3 )
	{
		const size_t	vi0 = pIndexedList[j] ;
		const size_t	vi1 = pIndexedList[j + 1] ;
		const size_t	vi2 = pIndexedList[j + 2] ;
		ESLAssert( vi0 < m_countVertex ) ;
		ESLAssert( vi1 < m_countVertex ) ;
		ESLAssert( vi2 < m_countVertex ) ;
		pvNormal[vi0] = pvVertNormals[j] ;
		pvNormal[vi1] = pvVertNormals[j + 1] ;
		pvNormal[vi2] = pvVertNormals[j + 2] ;
	}
	m_bufNormal.FinishArray() ;
	//
	SArray<S3DVector4>	vAddVertex ;
	SArray<S3DVector4>	vAddNormal ;
	SArray<S2DVector>	vAddUVMap ;
	SArray<S3DColor>	vAddColor ;
	const S2DVector *	pvUVMap = m_bufUVMap.GetConstArray() ;
	const S3DColor *	pColor = m_bufColor.GetConstArray() ;
	for ( i = 0, j = 0; i < m_countPolygon; i ++, j += 3 )
	{
		for ( size_t k = 0; k < 3; k ++ )
		{
			const size_t	vik = pIndexedList[j + k] ;
			if ( pvNormal[vik] != pvVertNormals[j + k] )
			{
				const S3DVector&	vVertex = pvVertex[vik] ;
				const S3DVector&	vNormal = pvVertNormals[j + k] ;
				const S3DVector *	pvAddVertex = vAddVertex.GetConstArray() ;
				const S3DVector *	pvAddNormal = vAddNormal.GetConstArray() ;
				const size_t		nAddCount = vAddVertex.GetLength() ;
				ssize_t				iVertex = -1 ;
				ESLAssert( nAddCount == vAddNormal.GetLength() ) ;
				for ( size_t iAdd = 0; iAdd < nAddCount; iAdd ++ )
				{
					if ( (pvAddVertex[iAdd] == vVertex)
						&& (pvAddNormal[iAdd] == vNormal) )
					{
						if ( (pvUVMap != nullptr)
							&& (vAddUVMap.At(iAdd) != pvUVMap[vik]) )
						{
							continue ;
						}
						if ( (pColor != nullptr)
							&& (vAddColor.At(iAdd) != pColor[vik]) )
						{
							continue ;
						}
						iVertex = (ssize_t) (m_countVertex + iAdd) ;
						break ;
					}
				}
				if ( iVertex < 0 )
				{
					iVertex = (ssize_t) (m_countVertex + nAddCount) ;
					vAddVertex.Add( S3DVector4( vVertex ) ) ;
					vAddNormal.Add( S3DVector4( vNormal ) ) ;
					if ( pvUVMap != nullptr )
					{
						vAddUVMap.Add( pvUVMap[vik] ) ;
					}
					if ( pColor != nullptr )
					{
						vAddColor.Add( pColor[vik] ) ;
					}
				}
				pIndexedList[j + k] = (uint32_t) iVertex ;
			}
		}
	}
	m_bufIndex.FinishArray() ;
	tits.FinishArray() ;
	//
	m_bufVertex.AddArray
		( vAddVertex.GetConstArray(), vAddVertex.GetLength() ) ;
	m_bufNormal.AddArray
		( vAddNormal.GetConstArray(), vAddNormal.GetLength() ) ;
	if ( pvUVMap != nullptr )
	{
		ESLAssert( m_bufUVMap.GetLength() == m_countVertex ) ;
		m_bufUVMap.AddArray
			( vAddUVMap.GetConstArray(), vAddUVMap.GetLength() ) ;
	}
	if ( pColor != nullptr )
	{
		ESLAssert( m_bufColor.GetLength() == m_countVertex ) ;
		m_bufColor.AddArray
			( vAddColor.GetConstArray(), vAddColor.GetLength() ) ;
	}
	m_countVertex = m_bufVertex.GetLength() ;
	//
	if ( (m_typeMesh == meshTriangleStrip)
				&& (vAddVertex.GetLength() > 0) )
	{
		m_typeMesh = meshIndexedTriangleList ;
		eslMoveMemory
			( m_bufIndex.GetArray( m_countVertex ),
						pIndexedList, m_countPolygon * 3 ) ;
		m_bufIndex.FinishArray() ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// モデル・ビルダ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DModelBuilder, S3DModelLoaderInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DModelBuilder::S3DModelBuilder( void )
{
}

S3DModelBuilder::S3DModelBuilder( const S3DModelBuilder & builder )
	: m_meshs( builder.m_meshs ),
		m_textures( builder.m_textures ),
		m_materials( builder.m_materials ),
		m_groups( builder.m_groups )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DModelBuilder::~S3DModelBuilder( void )
{
}

// モデルデータ読み込み＆構築 (S3DModelLoaderInterface)
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelBuilder::ReadModel
	( S3DModelBuffer & model, SSystem::SFileInterface & file )
{
	SParserErrorTracer	perr ;
	SGLError	err = ReadModel( file, perr ) ;
	if ( err )
	{
		return	err ;
	}
	return	BuildModel( model ) ;
}

// モデル構築
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelBuilder::BuildModel( S3DModelBuffer& model )
{
	//
	// テクスチャ
	//
	model.GetTextureLibrary() = m_textures ;
	//
	// 属性
	//
	model.GetMaterialLibrary() = m_materials ;
	//
	// メッシュ
	//
	const size_t	nMeshs = m_meshs.GetLength() ;
	size_t	i ;
	for ( i = 0; i < nMeshs; i ++ )
	{
		MeshEntry *	pMesh = m_meshs.GetAt( i ) ;
		ESLAssert( pMesh != nullptr ) ;
		if ( pMesh->m_typeMesh == MeshEntry::meshIndexedTriangleList )
		{
			model.AddIndexedTriangleList
				( pMesh->m_pMaterial, 0,
					pMesh->m_countPolygon, pMesh->m_countVertex,
					pMesh->m_bufVertex.GetConstArray(),
					pMesh->m_bufNormal.GetConstArray(),
					pMesh->m_bufUVMap.GetConstArray(),
					pMesh->m_bufColor.GetConstArray(),
					pMesh->m_bufIndex.GetConstArray() ) ;
		}
		else if ( pMesh->m_typeMesh == MeshEntry::meshTriangleStrip )
		{
			model.AddTriangleStrip
				( pMesh->m_pMaterial, 0,
					pMesh->m_countPolygon,
					pMesh->m_bufVertex.GetConstArray(),
					pMesh->m_bufNormal.GetConstArray(),
					pMesh->m_bufUVMap.GetConstArray(),
					pMesh->m_bufColor.GetConstArray() ) ;
		}
	}
	model.Flush() ;
	//
	// メッシュ・グループ
	//
	SStrSortArray<S3DModelBuffer::MeshGroup>&
					groups = model.GetMeshGroupList() ;
	const size_t	nMeshGroups = m_groups.GetLength() ;
	for ( i = 0; i < nMeshGroups; i ++ )
	{
		S3DModelBuffer::MeshGroup *	pmgSrc = m_groups.GetAt( i ) ;
		const SString *	pGroupID = m_groups.GetTagAt( i ) ;
		ESLAssert( pmgSrc != nullptr ) ;
		ESLAssert( pGroupID != nullptr ) ;
		//
		groups.Add( *pGroupID, *pmgSrc ) ;
	}
	//
	// モーフターゲット
	//
	const size_t	nMorphings = m_morphings.GetLength() ;
	for ( i = 0; i < nMorphings; i ++ )
	{
		MorphMeshEntry *	pmme = m_morphings.GetAt( i ) ;
		ESLAssert( pmme != nullptr ) ;
		//
		S3DModelBuffer::MorphTargetMesh *
				pmtm = new S3DModelBuffer::MorphTargetMesh( *pmme ) ;
		model.AddMorhTargetAs( pmme->m_strID, pmtm ) ;
		//
		size_t			nRelMeshs = pmme->m_arrRelMesh.GetLength() ;
		const size_t *	pRelMeshs = pmme->m_arrRelMesh.GetConstArray() ;
		for ( size_t j = 0; j < nRelMeshs; j ++ )
		{
			S3DModelBuffer::MeshObject *
					pMesh = model.GetMeshObjectAt( pRelMeshs[j] ) ;
			if ( pMesh != nullptr )
			{
				pMesh->m_arrMorphTarget.Add( new SString(pmme->m_strID) ) ;
			}
		}
	}
	model.BuildupMeshMorphingTarget() ;
	//
	// ボーン登録
	//
	const size_t	nBones = m_bones.GetLength() ;
	for ( size_t i = 0; i < nBones; i ++ )
	{
		BoneInfo *		pBoneInfo = m_bones.GetAt( i ) ;
		const SString *	pBoneID = m_bones.GetTagAt( i ) ;
		ESLAssert( pBoneInfo != nullptr ) ;
		ESLAssert( pBoneID != nullptr ) ;
		//
		S3DModelBoneSpace *	pBone = new S3DModelBoneSpace ;
		pBone->m_vCenter = pBoneInfo->m_vBonePos ;
		pBone->SetBoneHandle( pBoneInfo->m_vHandle ) ;
		pBone->SetBoneFlags( pBoneInfo->m_flagsBone ) ;
		pBone->SetBonePhysicalMaterial( pBoneInfo->m_physMaterial ) ;
		//
		if ( pBoneInfo->m_arrWeightMaps.GetLength() == 0 )
		{
			pBone->AttachModel( &model ) ;
		}
		else
		{
			FlatWeightMap	fwmDst ;
			MergeBoneWeightMap
				( fwmDst, model,
					pBoneInfo->m_arrWeightMaps.GetConstArray(),
					pBoneInfo->m_arrWeightMaps.GetLength() ) ;
			pBone->SetBoneWeight
				( &model, fwmDst.m_iVertex, fwmDst.m_iNormal,
					fwmDst.m_bufWeight.GetLength(),
					fwmDst.m_bufWeight.GetConstArray() ) ;
			for ( size_t j = 0; j < pBoneInfo->m_arrWeightMaps.GetLength(); j ++ )
			{
				S3DModelBuilder::WeightMap *
							pwm = pBoneInfo->m_arrWeightMaps.GetAt( j ) ;
				pBone->AddEffectiveMeshIndex
					( pwm->m_iTargetMesh, pwm->m_matIMesh, pwm->m_matRelMesh ) ;
			}
		}
		model.AddBonePropertyAs( *pBoneID, pBone ) ;
	}
	//
	// ボーン・ツリー構造構築
	//
	for ( size_t i = 0; i < nBones; i ++ )
	{
		BoneInfo *		pBoneInfo = m_bones.GetAt( i ) ;
		const SString *	pBoneID = m_bones.GetTagAt( i ) ;
		ESLAssert( pBoneInfo != nullptr ) ;
		ESLAssert( pBoneID != nullptr ) ;
		//
		S3DModelBoneSpace *	pBone = model.GetBonePropertyAs( *pBoneID ) ;
		ESLAssert( pBone != nullptr ) ;
		//
		if ( !pBoneInfo->m_strParentID.IsEmpty() )
		{
			S3DModelBoneSpace *	pParentBone =
					model.GetBonePropertyAs( pBoneInfo->m_strParentID ) ;
			if ( pParentBone != nullptr )
			{
				pParentBone->AddChild( pBone ) ;
			}
		}
		else
		{
			model.GetBoneRoot().AddChild( pBone ) ;
		}
	}
	model.BuildupBoneRelation() ;
	//
	return	sglErrFailed ;
}

// ポリゴンリストをレンダリングバッファに追加
//////////////////////////////////////////////////////////////////////////////
S3DModelBuilder::MeshEntry *
	S3DModelBuilder::AddIndexedTriangleList
		( S3DMaterial * pMaterial, uint32_t nFlags,
			size_t countPolygon, size_t countVertex,
			const S3DVector4 * pvVertex,
			const S3DVector4 * pvNormal,
			const S2DVector * pvUVMap,
			const S3DColor * pColor,
			const uint32_t * pIndexedList )
{
	 MeshEntry *	pMesh = new MeshEntry ;
	 pMesh->m_typeMesh = MeshEntry::meshIndexedTriangleList ;
	 pMesh->m_pMaterial = pMaterial ;
	 pMesh->m_countPolygon = countPolygon ;
	 pMesh->m_countVertex = countVertex ;
	 ESLAssert( pvVertex != nullptr ) ;
	 pMesh->m_bufVertex.AddArray( pvVertex, countVertex ) ;
	 if ( pvNormal != nullptr )
	 {
		 pMesh->m_bufNormal.AddArray( pvNormal, countVertex ) ;
	 }
	 if ( pvUVMap != nullptr )
	 {
		 pMesh->m_bufUVMap.AddArray( pvUVMap, countVertex ) ;
	 }
	 if ( pColor != nullptr )
	 {
		 pMesh->m_bufColor.AddArray( pColor, countVertex ) ;
	 }
	 ESLAssert( pIndexedList != nullptr ) ;
	 pMesh->m_bufIndex.AddArray( pIndexedList, countPolygon * 3 ) ;
	m_meshs.Add( pMesh ) ;
	return	pMesh ;
}

// トライアングルストリップをレンダリングバッファに追加
//////////////////////////////////////////////////////////////////////////////
S3DModelBuilder::MeshEntry *
	S3DModelBuilder::AddTriangleStrip
		( S3DMaterial * pMaterial, uint32_t nFlags,
			size_t countTriangleStrip,
			const S3DVector4 * pvVertex,
			const S3DVector4 * pvNormal,
			const S2DVector * pvUVMap, const S3DColor * pColor )
{
	 MeshEntry *	pMesh = new MeshEntry ;
	 size_t	countVertex = countTriangleStrip + 2 ;
	 pMesh->m_typeMesh = MeshEntry::meshTriangleStrip ;
	 pMesh->m_pMaterial = pMaterial ;
	 pMesh->m_countPolygon = countTriangleStrip ;
	 pMesh->m_countVertex = countVertex ;
	 ESLAssert( pvVertex != nullptr ) ;
	 pMesh->m_bufVertex.AddArray( pvVertex, countVertex ) ;
	 if ( pvNormal != nullptr )
	 {
		 pMesh->m_bufNormal.AddArray( pvNormal, countVertex ) ;
	 }
	 if ( pvUVMap != nullptr )
	 {
		 pMesh->m_bufUVMap.AddArray( pvUVMap, countVertex ) ;
	 }
	 if ( pColor != nullptr )
	 {
		 pMesh->m_bufColor.AddArray( pColor, countVertex ) ;
	 }
	m_meshs.Add( pMesh ) ;
	return	pMesh ;
}

// メッシュ・グループ名正規化（未使用名であることを保証する）
//////////////////////////////////////////////////////////////////////////////
const SString& S3DModelBuilder::NormalizeGroupName( SString& strName ) const
{
	if ( m_groups.GetAs( strName ) == nullptr )
	{
		return	strName ;
	}
	SString	strBase = strName ;
	for ( int i = 1; i < 0x10000; i ++ )
	{
		strName = strBase + SString( i ) ;
		if ( m_groups.GetAs( strName ) == nullptr )
		{
			break ;
		}
	}
	return	strName ;
}

// 範囲の一致するメッシュグループを検索する
//////////////////////////////////////////////////////////////////////////////
ssize_t S3DModelBuilder::FindMeshGroup
	( size_t iFirstMesh, size_t nMeshCount ) const
{
	size_t	nCount = m_groups.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		S3DModelBuffer::MeshGroup *	pmg = m_groups.GetAt( i ) ;
		ESLAssert( pmg != nullptr ) ;
		if ( (pmg->m_iFirstMesh == iFirstMesh)
			&& (pmg->m_nMeshCount == nMeshCount) )
		{
			return	(ssize_t) i ;
		}
	}
	return	-1 ;
}

// 数値配列解釈
//////////////////////////////////////////////////////////////////////////////
size_t S3DModelBuilder::ParseFloatArray
	( SArray<float>& aFloat,
		SStringParser& sparsList, size_t nCount )
{
	for ( size_t i = 0; i < nCount; i ++ )
	{
		int	type = sparsList.IsNextNumber() ;
		if ( type == SStringParser::numberInvalid )
		{
			return	i ;
		}
		aFloat.Add( (float) sparsList.NextRealNumber( type ) ) ;
	}
	return	nCount ;
}

size_t S3DModelBuilder::ParseIntArray
	( SArray<int>& aInt,
		SStringParser& sparsList, size_t nCount )
{
	for ( size_t i = 0; i < nCount; i ++ )
	{
		int	type = sparsList.IsNextNumber() ;
		if ( type == SStringParser::numberInvalid )
		{
			return	i ;
		}
		aInt.Add( (int) sparsList.NextInteger( type ) ) ;
	}
	return	nCount ;
}

// 16進数配列デコード
//////////////////////////////////////////////////////////////////////////////
size_t S3DModelBuilder::ParseHexBinaryArray
	( SQueueBuffer& bufBin, SStringParser& sparsHex )
{
	size_t	nBytes = 0 ;
	SString	strHex ;
	while ( sparsHex.PassSpace() )
	{
		SArray<uint8_t>	buf ;
		sparsHex.NextString( strHex ) ;
		SStringParser::DecodeHexString
			( buf, strHex, (ssize_t) strHex.GetLength() ) ;
		bufBin.Write( buf.GetConstArray(), buf.GetLength() ) ;
		nBytes += buf.GetLength() ;
	}
	return	nBytes ;
}

// ボーン・ウェイトマップを統合
//////////////////////////////////////////////////////////////////////////////
void S3DModelBuilder::MergeBoneWeightMap
	( S3DModelBuilder::FlatWeightMap& fwmDst,
		const S3DModelBuffer& model,
		const S3DModelBuilder::WeightMap*const* ppWeightMaps, size_t nWeightMaps )
{
	for ( size_t i = 0; i < nWeightMaps; i ++ )
	{
		const WeightMap *	pwm = ppWeightMaps[i] ;
		ESLAssert( pwm != nullptr ) ;
		//
		S3DModelBuffer::MeshObject *
			pMesh = model.GetMeshObjectAt( pwm->m_iTargetMesh ) ;
		if ( pMesh == nullptr )
		{
			continue ;
		}
		if ( fwmDst.m_bufWeight.GetLength() == 0 )
		{
			fwmDst.m_iVertex = pMesh->m_iVertex ;
			fwmDst.m_iNormal = pMesh->m_iNormal ;
			fwmDst.m_bufWeight = pwm->m_bufWeight ;
		}
		else
		{
			if ( (size_t) pMesh->m_iVertex < fwmDst.m_iVertex )
			{
				size_t	nOffset = fwmDst.m_iVertex - pMesh->m_iVertex ;
				size_t	nLastSize = fwmDst.m_bufWeight.GetLength() ;
				fwmDst.m_bufWeight.SetLength( nLastSize + nOffset ) ;
				//
				eslMoveMemory
					( fwmDst.m_bufWeight.GetArray() + nOffset,
						fwmDst.m_bufWeight.GetConstArray(),
						nLastSize * sizeof(float32_t) ) ;
				fwmDst.m_bufWeight.FinishArray() ;
				//
				eslFillMemory
					( fwmDst.m_bufWeight.GetArray(),
									0, nOffset * sizeof(float32_t) ) ;
				fwmDst.m_bufWeight.FinishArray() ;
				//
				fwmDst.m_iVertex -= nOffset ;
				fwmDst.m_iNormal -= nOffset ;
			}
			size_t	nWeightLength = pwm->m_bufWeight.GetLength() ;
			size_t	iEndVertex = pMesh->m_iVertex + nWeightLength ;
			if ( iEndVertex > fwmDst.m_iVertex + fwmDst.m_bufWeight.GetLength() )
			{
				fwmDst.m_bufWeight.SetLength( iEndVertex - fwmDst.m_iVertex ) ;
			}
			eslMoveMemory
				( fwmDst.m_bufWeight.GetArray()
						+ (pMesh->m_iVertex - fwmDst.m_iVertex),
					pwm->m_bufWeight.GetConstArray(),
					nWeightLength * sizeof(float32_t) ) ;
			fwmDst.m_bufWeight.FinishArray() ;
		}
	}
}


//////////////////////////////////////////////////////////////////////////////
// Sahde xml モデルデータ・ビルダ
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DShadeXMLLoader, S3DModelBuilder )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DShadeXMLLoader::S3DShadeXMLLoader( void )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DShadeXMLLoader::~S3DShadeXMLLoader( void )
{
}

// ファイル拡張子判定
//////////////////////////////////////////////////////////////////////////////
bool S3DShadeXMLLoader::IsMatchableFileExtension( const wchar_t * pszExt )
{
	return	(SString::CompareNoCase( pszExt, L"xmlshd" ) == 0) ;
}

// MIME 判定
//////////////////////////////////////////////////////////////////////////////
bool S3DShadeXMLLoader::IsMatchableMIMEType( const wchar_t * pszMIME )
{
	return	(SString::CompareNoCase( pszMIME, L"application/x-xmlshd" ) == 0) ;
}

// ファイルを読み込む
//////////////////////////////////////////////////////////////////////////////
SGLError S3DShadeXMLLoader::LoadModel
	( const wchar_t * pwszFilePath, SParserErrorInterface& perr )
{
	SSmartPointer<SFileInterface>	pfile =
		SFileOpener::DefaultNewOpenFile
			( pwszFilePath, SFileOpener::shareRead ) ;
	if ( pfile == nullptr )
	{
		return	sglErrFailed ;
	}
	return	ReadModel( *pfile, perr ) ;
}

SGLError S3DShadeXMLLoader::ReadModel
	( SSystem::SFileInterface& file, SParserErrorInterface& perr )
{
	SXMLDocument	xmlDoc ;
	if ( xmlDoc.ReadDocument( file, perr ) )
	{
		return	sglErrFailed ;
	}
	return	ParseModel( xmlDoc, file, perr ) ;
}

// 解釈
//////////////////////////////////////////////////////////////////////////////
SGLError S3DShadeXMLLoader::ParseModel
	( SSystem::SXMLDocument& xmlDoc,
		SSystem::SFileOpener& opener, SParserErrorInterface& perr )
{
	SXMLDocument *	pxmlShade = xmlDoc.GetElementTagAs( L"shade" ) ;
	if ( pxmlShade == nullptr )
	{
		SStringParser	ssDummy ;
		perr.OutputError( ssDummy, L"<shade> タグが見つかりません" ) ;
		return	sglErrFailed ;
	}
	//
	// マスター画像処理
	//
	SGLError	err ;
	err = ParseAllMasterImage( *pxmlShade, opener, perr ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// マスター表面属性処理
	//
	err = ParseAllMasterSurface( *pxmlShade ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// パート順次処理
	//
	S4DMatrix	matPart( 1, 0, 0, 0,  0, -1, 0, 0,  0, 0, -1, 0,  0, 0, 0, 1 ) ;
	return	ParsePart( matPart, nullptr, *pxmlShade, perr ) ;
}

// 一般パート処理
//////////////////////////////////////////////////////////////////////////////
SGLError S3DShadeXMLLoader::ParsePart
	( const S4DMatrix& matPart, S3DMaterial * pMaterial,
		SSystem::SXMLDocument& xmlPart, SParserErrorInterface& perr )
{
	//
	// パート名
	//
	SString	strPartName = xmlPart.GetContentsAsString( L"string", L"part" ) ;
	NormalizeGroupName( strPartName ) ;
	//
	// ローカル空間
	//
	S4DMatrix	matLocal = matPart ;
	SStringParser
		sparsTransformation =
			xmlPart.GetContentsAsString( L"transformation", nullptr ) ;
	SArray<float>	aTransformation ;
	if ( ParseFloatArray
		( aTransformation, sparsTransformation, 16 ) >= 16 )
	{
		S4DMatrix		mat ;
		const float *	pft = aTransformation.GetConstArray() ;
		mat.m[0][0] = pft[0] ;
		mat.m[1][0] = pft[1] ;
		mat.m[2][0] = pft[2] ;
		mat.m[3][0] = pft[3] ;
		mat.m[0][1] = pft[4] ;
		mat.m[1][1] = pft[5] ;
		mat.m[2][1] = pft[6] ;
		mat.m[3][1] = pft[7] ;
		mat.m[0][2] = pft[8] ;
		mat.m[1][2] = pft[9] ;
		mat.m[2][2] = pft[10] ;
		mat.m[3][2] = pft[11] ;
		mat.m[0][3] = pft[12] ;
		mat.m[1][3] = pft[13] ;
		mat.m[2][3] = pft[14] ;
		mat.m[3][3] = pft[15] ;
		matLocal *= mat ;
	}
	//
	// 表面属性
	//
	pMaterial = ParsePartSurface( pMaterial, xmlPart ) ;
	//
	// サブパート処理
	//
	size_t	iFirstMesh = m_meshs.GetLength() ;
	size_t	nCount = xmlPart.GetElementsCount() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		SXMLDocument *	pxmlTag = xmlPart.GetElementAt( i ) ;
		if ( pxmlTag == nullptr )
		{
			continue ;
		}
		const SString& strTag = pxmlTag->GetTag() ;
		SGLError	err ;
		if ( strTag == L"polygon_mesh" )
		{
			err = ParsePolygonMesh( matLocal, pMaterial, *pxmlTag, perr ) ;
			if ( err )
			{
				return	err ;
			}
		}
		else if ( strTag == L"part" )
		{
			err = ParsePart( matLocal, pMaterial, *pxmlTag, perr ) ;
			if ( err )
			{
				return	err ;
			}
		}
	}
	//
	// メッシュグループ登録
	//
	size_t	iEndMesh = m_meshs.GetLength() ;
	if ( (iFirstMesh < iEndMesh)
		&& (FindMeshGroup( iFirstMesh, iEndMesh - iFirstMesh ) >= 0) )
	{
		S3DModelBuffer::MeshGroup	mgroup ;
		mgroup.m_iFirstMesh = (uint32_t) iFirstMesh ;
		mgroup.m_nMeshCount = (uint32_t) (iEndMesh - iFirstMesh) ;
		//
		S4DVector	vPos( 0, 0, 0, 1 ) ;
		matLocal.RevolveVector( vPos ) ;
		mgroup.m_vCenter.x = vPos.x / vPos.w ;
		mgroup.m_vCenter.y = vPos.y / vPos.w ;
		mgroup.m_vCenter.z = vPos.z / vPos.w ;
		//
		m_groups.Add( strPartName, mgroup ) ;
	}
	return	sglErrSuccess ;
}

// パート表面属性処理
//////////////////////////////////////////////////////////////////////////////
S3DMaterial * S3DShadeXMLLoader::ParsePartSurface
	( S3DMaterial * pMaterial, SSystem::SXMLDocument& xmlPart )
{
	//
	// パート名
	//
	SString	strMaterialName =
				xmlPart.GetContentsAsString( L"string", L"material" ) ;
	//
	// 表面属性解釈
	//
	SXMLDocument *	pxmlInt = xmlPart.GetElementTagAs( L"int" ) ;
	if ( pxmlInt != nullptr )
	{
		SString *	pstrInt = pxmlInt->GetTextElement() ;
		if ( pstrInt != nullptr )
		{
			SString *	pstrSurfaceID =
				m_aMasterSurface.GetAt( (size_t) pstrInt->AsInteger() ) ;
			if ( pstrSurfaceID != nullptr )
			{
				pMaterial = m_materials.GetMaterialAs( *pstrSurfaceID ) ;
				ESLAssert( pMaterial != nullptr ) ;
			}
		}
	}
	SXMLDocument *	pxmlSurface = xmlPart.GetElementTagAs( L"surface" ) ;
	if ( pxmlSurface != nullptr )
	{
		pMaterial = ParseSurfaceAttribute( strMaterialName, *pxmlSurface ) ;
	}
	return	pMaterial ;
}

// <polygon_mesh> ポリゴンメッシュ処理
//////////////////////////////////////////////////////////////////////////////
SGLError S3DShadeXMLLoader::ParsePolygonMesh
	( const S4DMatrix& matPart,
		S3DMaterial * pMaterial,
		SSystem::SXMLDocument& xmlPolygonMesh,
		SSystem::SParserErrorInterface& perr )
{
	size_t	iMesh = m_meshs.GetLength() ;
	//
	// メッシュ名
	//
	SString	strMeshName =
				xmlPolygonMesh.GetContentsAsString( L"string", L"mesh" ) ;
	NormalizeGroupName( strMeshName ) ;
	//
	// 表面属性解釈
	//
	pMaterial = ParsePartSurface( pMaterial, xmlPolygonMesh ) ;
	if ( pMaterial == nullptr )
	{
		SStringParser	ssTemp ;
		perr.OutputError( ssTemp, L"表面属性が設定されていません" ) ;
		return	sglErrFailed ;
	}
	//
	// ポリゴン情報解釈
	//
	SArray<S3DVector4>	bufVertex ;
	SArray<S2DVector>	bufUVMap ;
	SArray<uint32_t>	bufIndex ;
	SXMLDocument *	pxmlVertices =
						xmlPolygonMesh.GetElementTagAs( L"vertices" ) ;
	if ( pxmlVertices == nullptr )
	{
		SStringParser	ssTemp ;
		perr.OutputError
			( ssTemp, L"<polygon_mesh> に <vertices> が見つかりません" ) ;
		return	sglErrFailed ;
	}
	SXMLDocument *	pxmlFaces = xmlPolygonMesh.GetElementTagAs( L"faces" ) ;
	if ( pxmlFaces == nullptr )
	{
		SStringParser	ssTemp ;
		perr.OutputError
			( ssTemp, L"<polygon_mesh> に <faces> が見つかりません" ) ;
		return	sglErrFailed ;
	}
	ssize_t	iMaterial = m_materials.FindMaterialPtr( pMaterial ) ;
	const MappingInfo *
			pMappingInf =
				m_mapMappingInfo.GetAs
					( m_materials.GetMaterialIdentityAt( iMaterial ) ) ;
	if ( ParsePolygonVertices( matPart, *pxmlVertices, bufVertex )
		|| ParsePolygonFaces
			( *pxmlFaces, pMappingInf, bufVertex, bufUVMap, bufIndex ) )
	{
		return	sglErrFailed ;
	}
	if ( bufUVMap.GetLength() > 0 )
	{
		SGLImageObject *	pImage = pMaterial->GetTexture() ;
		if ( pImage != nullptr )
		{
			SGLSize		sizeImage = pImage->GetImageSize() ;
			size_t		nUVMap = bufUVMap.GetLength() ;
			S2DVector *	pvUVMap = bufUVMap.GetArray() ;
			for ( size_t i = 0; i < nUVMap; i ++ )
			{
				pvUVMap[i].x *= (float) sizeImage.w ;
				pvUVMap[i].y *= (float) sizeImage.h ;
			}
			bufUVMap.FinishArray() ;
		}
	}
	MeshEntry *	pMesh =
		AddIndexedTriangleList
		( pMaterial, 0,
			bufIndex.GetLength() / 3,
			bufVertex.GetLength(),
			bufVertex.GetConstArray(), nullptr,
			bufUVMap.GetConstArray(), nullptr, bufIndex.GetConstArray() ) ;
	if ( pMesh != nullptr )
	{
		double	degSmooth = 0.0 ;
		if ( xmlPolygonMesh.GetContentsAsInteger( L"flags\\smooth_edges", 1 ) )
		{
			degSmooth = xmlPolygonMesh.GetContentsAsReal( L"threshold", 30.0 ) ;
		}
		pMesh->MakeNormal( cos( degSmooth * PI / 180 ) ) ;
	}
	//
	// メッシュグループ登録
	//
	size_t	iEndMesh = m_meshs.GetLength() ;
	if ( iMesh < iEndMesh )
	{
		S3DModelBuffer::MeshGroup	mgroup ;
		mgroup.m_iFirstMesh = (uint32_t) iMesh ;
		mgroup.m_nMeshCount = (uint32_t) (iEndMesh - iMesh) ;
		//
		S4DMatrix	matLocal = matPart ;
		S4DVector	vPos( 0, 0, 0, 1 ) ;
		matLocal.RevolveVector( vPos ) ;
		mgroup.m_vCenter.x = vPos.x / vPos.w ;
		mgroup.m_vCenter.y = vPos.y / vPos.w ;
		mgroup.m_vCenter.z = vPos.z / vPos.w ;
		//
		m_groups.Add( strMeshName, mgroup ) ;
	}
	return	sglErrSuccess ;
}

// <master_image> 画像パート処理
//////////////////////////////////////////////////////////////////////////////
SGLError S3DShadeXMLLoader::ParseAllMasterImage
	( SSystem::SXMLDocument& xmlPart,
		SSystem::SFileOpener& opener,
		SSystem::SParserErrorInterface& perr )
{
	size_t	nCount = xmlPart.GetElementsCount() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		SXMLDocument *	pxmlTag = xmlPart.GetElementAt( i ) ;
		if ( pxmlTag == nullptr )
		{
			continue ;
		}
		SGLError	err ;
		if ( pxmlTag->GetTag() == L"part" )
		{
			err = ParseAllMasterImage( *pxmlTag, opener, perr ) ;
			if ( err )
			{
				return	err ;
			}
		}
		else if ( pxmlTag->GetTag() == L"master_image" )
		{
			err = ParseMasterImage( *pxmlTag, opener, perr ) ;
			if ( err )
			{
				return	err ;
			}
		}
		else if ( pxmlTag->GetTag() == L"master_surface" )
		{
			SXMLDocument *
				pxmlSurface = pxmlTag->GetElementTagAs( L"surface" ) ;
			if ( pxmlSurface == nullptr )
			{
				continue ;
			}
			ssize_t	iLastTag = 0 ;
			for ( ; ; )
			{
				ssize_t	iTag =
					pxmlSurface->FindElementTag
								( L"mapping_layer", iLastTag ) ;
				if ( iTag <= 0 )
				{
					break ;
				}
				SXMLDocument *	pxmlMapLayer =
						pxmlSurface->GetElementAt( (size_t) iTag ) ;
				ESLAssert( pxmlMapLayer != nullptr ) ;
				SString *	pstrObjectId =
								pxmlMapLayer->GetContentsValue
										( L"pixels\\object_id" ) ;
				SString *	pstrWinPath =
								pxmlMapLayer->GetContentsValue
									( L"pixels\\file_alias\\win_path" ) ;
				if ( pstrObjectId && pstrWinPath )
				{
					m_mapImageObjPath.SetAs( *pstrObjectId, *pstrWinPath ) ;
				}
				iLastTag = iTag + 1 ;
			}
		}
	}
	return	sglErrSuccess ;
}

SGLError S3DShadeXMLLoader::ParseMasterImage
	( SSystem::SXMLDocument& xmlMasterImage,
		SSystem::SFileOpener& opener,
		SSystem::SParserErrorInterface& perr )
{
	//
	// 画像名
	//
	SString	strImageName =
			xmlMasterImage.GetContentsAsString( L"string", nullptr ) ;
	//
	// 画像情報
	//
	SXMLDocument *	pxmlPixels =
			xmlMasterImage.GetElementTagAs( L"pixels" ) ;
	if ( pxmlPixels == nullptr )
	{
		SStringParser	ssTemp ;
		perr.OutputError
			( ssTemp, L"<master_image> に <pixels> が見つかりません" ) ;
		return	sglErrFailed ;
	}
	SString	strObjectId =
			pxmlPixels->GetContentsAsString( L"object_id", L"image" ) ;
	//
	// 外部ファイル名取得
	//
	SStringParser	sparsWinPath ;
	SXMLDocument *	pxmlFileAlias =
			pxmlPixels->GetElementTagAs( L"file_alias" ) ;
	if ( pxmlFileAlias != nullptr )
	{
		sparsWinPath =
			pxmlFileAlias->GetContentsAsString( L"win_path", nullptr ) ;
	}
	else
	{
		SString	strLinkId =
				pxmlPixels->GetContentsAsString( L"link_id", nullptr ) ;
		SString *	pstrFileAlias = m_mapImageObjPath.GetAs( strLinkId ) ;
		if ( strLinkId.IsEmpty() || (pstrFileAlias == nullptr) )
		{
			SStringParser	ssTemp ;
			perr.OutputError
				( ssTemp, L"<master_image><pixels> に"
							L" <file_alias> が見つかりません" ) ;
			return	sglErrFailed ;
		}
		strObjectId = strLinkId ;
		sparsWinPath = *pstrFileAlias ;
	}
	SQueueBuffer	qbufWinPath ;
	ParseHexBinaryArray( qbufWinPath, sparsWinPath ) ;
	//
	SString			strWinPath ;
	size_t			lenWinPath ;
	const uint8_t *	pWinPath = qbufWinPath.GetBuffer( lenWinPath ) ;
	Charset::Decode
		( strWinPath,
			Charset::encodingUTF8, pWinPath, (ssize_t) lenWinPath ) ;
	//
	SSmartPointer<SFileInterface>
		pFile = opener.NewOpenFile( strWinPath, SFileOpener::shareRead ) ;
	if ( pFile == nullptr )
	{
		SStringParser	ssTemp ;
		perr.OutputError
			( ssTemp, strWinPath + L" を開けませんでした" ) ;
		return	sglErrFailed ;
	}
	SGLImage *	pImage = new SGLImage ;
	if ( pImage->ReadImage( pFile ) )
	{
		delete	pImage ;
		//
		SStringParser	ssTemp ;
		perr.OutputError
			( ssTemp, strWinPath + L" の読み込みに失敗しました" ) ;
		return	sglErrFailed ;
	}
	if ( strImageName.IsEmpty() )
	{
		strImageName = strWinPath.GetFileTitlePart() ;
	}
	m_textures.NormalizeIdentity( strImageName ) ;
	m_textures.AddSmartTextureAs( strImageName, pImage ) ;
	m_mapImageId.SetAs( strObjectId, strImageName ) ;
	return	sglErrSuccess ;
}

// <master_surface> パート処理
//////////////////////////////////////////////////////////////////////////////
SGLError S3DShadeXMLLoader::ParseAllMasterSurface
	( SSystem::SXMLDocument& xmlPart )
{
	size_t	nCount = xmlPart.GetElementsCount() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		SXMLDocument *	pxmlTag = xmlPart.GetElementAt( i ) ;
		if ( pxmlTag == nullptr )
		{
			continue ;
		}
		SGLError	err ;
		if ( pxmlTag->GetTag() == L"part" )
		{
			err = ParseAllMasterSurface( *pxmlTag ) ;
			if ( err )
			{
				return	err ;
			}
		}
		if ( pxmlTag->GetTag() == L"master_surface" )
		{
			err = ParseMasterSurface( *pxmlTag ) ;
			if ( err )
			{
				return	err ;
			}
		}
	}
	return	sglErrSuccess ;
}

SGLError S3DShadeXMLLoader::ParseMasterSurface
	( SSystem::SXMLDocument& xmlMasterSurface )
{
	//
	// 属性名
	//
	SString	strSurfaceName =
			xmlMasterSurface.GetContentsAsString( L"string", L"image" ) ;
	//
	// <surface> 処理
	//
	SXMLDocument *	pxmlSurface =
			xmlMasterSurface.GetElementTagAs( L"surface" ) ;
	if ( pxmlSurface != nullptr )
	{
		if ( ParseSurfaceAttribute( strSurfaceName, *pxmlSurface ) != nullptr )
		{
			m_aMasterSurface.InsertAt( 0, new SString(strSurfaceName) ) ;
		}
	}
	return	sglErrSuccess ;
}

// <part><surface> 処理
//////////////////////////////////////////////////////////////////////////////
S3DMaterial * S3DShadeXMLLoader::ParseSurfaceAttribute
	( SSystem::SString& strSurfaceID, SSystem::SXMLDocument& xmlSurface )
{
	S3DMaterial *	pMaterial = new S3DMaterial ;
	m_materials.NormalizeIdentity( strSurfaceID ) ;
	m_materials.AddMaterialAs( strSurfaceID, pMaterial ) ;
	//
	S3DSurfaceAttribute	sufattr ;
	SXMLDocument *	pxmlFlags = xmlSurface.GetElementTagAs( L"flags" ) ;
	if ( pxmlFlags && pxmlFlags->GetAttrIntegerAs( L"has_diffuse", 1 ) )
	{
		SArray<float>	aFloat ;
		SStringParser	sparsColor =
				xmlSurface.GetContentsAsString( L"diffuse_color" ) ;
		if ( ParseFloatArray( aFloat, sparsColor, 3 ) >= 3 )
		{
			sufattr.colorBase.rgbMul.argb.Red =
				(uint8_t) esl_clampi
					( eslRoundR32ToInt
						( aFloat.At(0) * 255.0f ), 0, 255 ) ;
			sufattr.colorBase.rgbMul.argb.Green =
				(uint8_t) esl_clampi
					( eslRoundR32ToInt
						( aFloat.At(1) * 255.0f ), 0, 255 ) ;
			sufattr.colorBase.rgbMul.argb.Blue =
				(uint8_t) esl_clampi
					( eslRoundR32ToInt
						( aFloat.At(2) * 255.0f ), 0, 255 ) ;
			sufattr.colorBase.rgbAdd = 0 ;
		}
		sufattr.nDiffusion =
			(int) eslRoundR64ToLInt
					( xmlSurface.GetContentsAsReal
						( L"diffuse", sufattr.nDiffusion / 256.0 ) * 256.0 ) ;
	}
	else
	{
		sufattr.nDiffusion = 0x100 ;
	}
	if ( pxmlFlags && pxmlFlags->GetAttrIntegerAs( L"has_specular", 1 ) )
	{
		sufattr.nSpecular =
			(int) eslRoundR64ToLInt
					( xmlSurface.GetContentsAsReal
						( L"highlight", sufattr.nSpecular / 256.0 ) * 256.0 ) ;
		sufattr.nSpecularSize =
			(int) eslRoundR64ToLInt
					( xmlSurface.GetContentsAsReal
						( L"highlight_size", sufattr.nSpecularSize / 256.0 ) * 256.0 ) ;
	}
	else
	{
		sufattr.nSpecular = 0x80 ;
		sufattr.nSpecularSize = 0x20 ;
	}
	if ( pxmlFlags && pxmlFlags->GetAttrIntegerAs( L"has_transparency", 1 ) )
	{
		sufattr.nTransparency =
			(int) eslRoundR64ToLInt
					( xmlSurface.GetContentsAsReal
						( L"transparency", sufattr.nTransparency / 256.0 ) * 256.0 ) ;
	}
	if ( pxmlFlags && pxmlFlags->GetAttrIntegerAs( L"has_glow", 1 ) )
	{
		sufattr.nAmbient =
			(int) eslRoundR64ToLInt
					( xmlSurface.GetContentsAsReal
						( L"glow", sufattr.nAmbient / 256.0 ) * 256.0 ) ;
		sufattr.nDeepness =
			(int) eslRoundR64ToLInt
					( xmlSurface.GetContentsAsReal
						( L"soft_glow", sufattr.nDeepness / 256.0 ) * 256.0 ) ;
	}
	if ( pxmlFlags && pxmlFlags->GetAttrIntegerAs( L"has_reflection", 1 ) )
	{
		sufattr.nReflection =
			(int) eslRoundR64ToLInt
					( xmlSurface.GetContentsAsReal
						( L"reflection", sufattr.nReflection / 256.0 ) * 256.0 ) ;
	}
	if ( pxmlFlags && pxmlFlags->GetAttrIntegerAs( L"has_refraction", 1 ) )
	{
		sufattr.fpRefraction =
				(float32_t) xmlSurface.GetContentsAsReal
								( L"refraction", sufattr.fpRefraction ) ;
	}
	size_t	iLastMappingLayer = 0 ;
	for ( ; ; )
	{
		ssize_t	iMappingLayerTag =
			xmlSurface.FindElementTag( L"mapping_layer", iLastMappingLayer ) ;
		if ( iMappingLayerTag < 0 )
		{
			break ;
		}
		iLastMappingLayer = (size_t) iMappingLayerTag + 1 ;
		//
		SXMLDocument *	pxmlMappingLayer =
				xmlSurface.GetElementAt( (size_t) iMappingLayerTag ) ;
		//
		// テクスチャ画像取得
		//
		SGLImageObject *	pImage = nullptr ;
		SString *	pstrPixelsLinkId =
						pxmlMappingLayer->GetContentsValue( L"pixels\\link_id" ) ;
		if ( pstrPixelsLinkId == nullptr )
		{
			pstrPixelsLinkId =
					pxmlMappingLayer->GetContentsValue( L"pixels\\object_id" ) ;
		}
		if ( pstrPixelsLinkId != nullptr )
		{
			pstrPixelsLinkId = m_mapImageId.GetAs( *pstrPixelsLinkId ) ;
			if ( pstrPixelsLinkId != nullptr )
			{
				pImage = m_textures.GetTextureAs( *pstrPixelsLinkId ) ;
			}
		}
		if ( pImage == nullptr )
		{
			continue ;
		}
		//
		// 補完処理
		//
		if ( pxmlMappingLayer->GetContentsAsInteger( L"softness" ) )
		{
			sufattr.flagsShading |= shadingTextureSmoothing ;
		}
		//
		// マッピング情報取得
		//
		MappingInfo	mapinf ;
		mapinf.flagFlipX =
			(pxmlMappingLayer->GetContentsAsInteger
								( L"flags\\horizontal_flip" ) != 0) ;
		mapinf.flagFlipY =
			(pxmlMappingLayer->GetContentsAsInteger
								( L"flags\\vertical_flip" ) != 0) ;
		mapinf.flagSwapXY =
			(pxmlMappingLayer->GetContentsAsInteger
								( L"flags\\swap_axis" ) != 0) ;
		mapinf.nRepeatX =
			(int) pxmlMappingLayer->GetContentsAsInteger( L"repetition_x" ) ;
		mapinf.nRepeatY =
			(int) pxmlMappingLayer->GetContentsAsInteger( L"repetition_y" ) ;
		mapinf.iMappingBy =
			(int) pxmlMappingLayer->GetContentsAsInteger( L"mapping_by" ) ;
		//
		if ( (mapinf.nRepeatX >= 2) || (mapinf.nRepeatY >= 2) )
		{
			sufattr.flagsShading |= shadingTextureTiling ;
		}
		//
		// テクスチャ種類
		//
		int	iMappingType =
			(int) pxmlMappingLayer->GetContentsAsInteger( L"mapping_type" ) ;
		float32_t	fpWeight =
			(float32_t) pxmlMappingLayer->GetContentsAsReal( L"weight", 1.0 ) ;
		if ( iMappingType == 0 )
		{
			sufattr.flagsShading |= shadingTextureMapping | shadingTextureTiling ;
			//
			pMaterial->SetTexture
				( pImage, 0, S3DMaterial::textureMain,
						fpWeight, 0.0f, *pstrPixelsLinkId ) ;
			//
			m_mapMappingInfo.SetAs( strSurfaceID, mapinf ) ;
		}
		else if ( iMappingType == 8 )
		{
			sufattr.nAmbient = 0 ;
			sufattr.flagsShading |= shadingLuminousTexture ;
			//
			pMaterial->SetTexture
				( pImage, 1, S3DMaterial::textureLuminous,
						fpWeight, 0.0f, *pstrPixelsLinkId ) ;
		}
	}
	if ( !(sufattr.flagsShading & shadingTextureMapping) )
	{
		sufattr.colorBase.rgbAdd = sufattr.colorBase.rgbMul ;
		sufattr.colorBase.rgbMul = 0 ;
	}
	sufattr.flagsShading |= shadingMethodGouraud | shadingSingleSidePlane ;
	pMaterial->SetSurfaceAttribute( sufattr ) ;
	return	pMaterial ;
}

// <polygon_mesh><vertices> 処理
//////////////////////////////////////////////////////////////////////////////
SGLError S3DShadeXMLLoader::ParsePolygonVertices
	( const S4DMatrix& matPart,
		SSystem::SXMLDocument& xmlVertices,
		SSystem::SArray<S3DVector4>& bufVertex )
{
	SStringParser	sparsPositions =
						xmlVertices.GetContentsAsString( L"positions" ) ;
	size_t	nCount = (size_t) sparsPositions.NextInteger() ;
	//
	SArray<float>	aFloat ;
	if ( ParseFloatArray( aFloat, sparsPositions, nCount * 3 ) < nCount * 3 )
	{
		return	sglErrFailed ;
	}
	S3DVector4 *	pvVertex = bufVertex.GetArray( nCount ) ;
	const float *	pFloat = aFloat.GetConstArray() ;
	for ( size_t i = 0, j = 0; i < nCount; i ++, j += 3 )
	{
		S4DVector	v4 ;
		v4.x = pFloat[j] ;
		v4.y = pFloat[j + 1] ;
		v4.z = pFloat[j + 2] ;
		v4.w = 1.0f ;
		matPart.RevolveVector( v4 ) ;
		//
		pvVertex[i].x = v4.x / v4.w ;
		pvVertex[i].y = v4.y / v4.w ;
		pvVertex[i].z = v4.z / v4.w ;
		pvVertex[i].d = 0.0f ;
	}
	bufVertex.FinishArray() ;
	return	sglErrSuccess ;
}

// <polygon_mesh><faces> 処理
//////////////////////////////////////////////////////////////////////////////
SGLError S3DShadeXMLLoader::ParsePolygonFaces
	( SSystem::SXMLDocument& xmlFaces,
		const S3DShadeXMLLoader::MappingInfo * pMappingInf,
		SSystem::SArray<S3DVector4>& bufVertex,
		SSystem::SArray<S2DVector>& bufUVMap,
		SSystem::SArray<uint32_t>& bufIndex )
{
	//
	// ポリゴンリスト
	// <全要素数> <全面数> <面1頂点数> <頂点1> <頂点2> ...
	//                     <面2頂点数> <頂点1> <頂点2> ...
	//
	SStringParser	sparsFaceVertices =
						xmlFaces.GetContentsAsString( L"face_vertices" ) ;
	size_t	nCount = (size_t) sparsFaceVertices.NextInteger() ;
	//
	SArray<int>	aIndex ;
	if ( ParseIntArray
		( aIndex, sparsFaceVertices, nCount ) < nCount )
	{
		return	sglErrFailed ;
	}
	if ( nCount < 1 )
	{
		return	sglErrSuccess ;
	}
	SArray<size_t>	aFaceVertexIndex ;
	SArray<size_t>	aFaceIndexIndex ;
	const int *		pSrcIndex = aIndex.GetConstArray() ;
	const size_t	nFaceCount = (size_t) pSrcIndex[0] ;
	size_t			iFaceVertex = 0 ;
	for ( size_t i = 0, j = 1; (i < nFaceCount) && (j < nCount); i ++ )
	{
		size_t	nFaceVertics = (size_t) pSrcIndex[j ++] ;
		for ( size_t k = 2; (k < nFaceVertics) && (j + k < nCount); k ++ )
		{
			bufIndex.Add( pSrcIndex[j] ) ;
			bufIndex.Add( pSrcIndex[j + k] ) ;
			bufIndex.Add( pSrcIndex[j + k - 1] ) ;
			aFaceIndexIndex.Add( iFaceVertex ) ;
			aFaceIndexIndex.Add( iFaceVertex + k ) ;
			aFaceIndexIndex.Add( iFaceVertex + k - 1 ) ;
		}
		for ( size_t k = 0; k < nFaceVertics; k ++ )
		{
			aFaceVertexIndex.Add( pSrcIndex[j + k] ) ;
		}
		j += nFaceVertics ;
		iFaceVertex += nFaceVertics ;
	}
	//
	// UV マップ
	// <全要素数> <面1u0> <面1v0> <面1u1> <面1v1> ...
	//            <面2u0> <面2v0> <面2u1> <面2v1> ...
	//
	SPointerArray<SXMLDocument>	aUVLayer ;
	ssize_t	iLastUVLayer = 0 ;
	for ( ; ; )
	{
		ssize_t	iUVLayer =
					xmlFaces.FindElementTag( L"uv_layer", iLastUVLayer ) ;
		if ( iUVLayer < 0 )
		{
			break ;
		}
		aUVLayer.Add( xmlFaces.GetElementAt( (size_t) iUVLayer ) ) ;
		iLastUVLayer = (ssize_t) iUVLayer + 1 ;
	}
	SXMLDocument *	pxmlUVLayer = aUVLayer.GetAt( 0 ) ;
	if ( pMappingInf != nullptr )
	{
		pxmlUVLayer = aUVLayer.GetAt( pMappingInf->iMappingBy ) ;
		if ( pxmlUVLayer == nullptr )
		{
			pxmlUVLayer = aUVLayer.GetAt( 0 ) ;
		}
	}
	if ( pxmlUVLayer == nullptr )
	{
		return	sglErrSuccess ;
	}
	SString *	pstrUVLayer = pxmlUVLayer->GetTextElement() ;
	if ( pstrUVLayer == nullptr )
	{
		return	sglErrSuccess ;
	}
	SStringParser	sparsUVLayer ;
	sparsUVLayer.AttachString( *pstrUVLayer ) ;
	nCount = (size_t) sparsUVLayer.NextInteger() ;
	//
	SArray<float>	aFloat ;
	if ( ParseFloatArray( aFloat, sparsUVLayer, nCount ) < nCount )
	{
		return	sglErrSuccess ;
	}
	nCount /= 2 ;
	//
	const float *	pFloat = aFloat.GetConstArray() ;
	const size_t *	pFaceIndexIndex = aFaceIndexIndex.GetConstArray() ;
	size_t			nFaceIndexIndex = aFaceIndexIndex.GetLength() ;
	size_t *		pFaceVertexIndex = aFaceVertexIndex.GetArray() ;
	size_t			nFaceVertexIndex = aFaceVertexIndex.GetLength() ;
	bufUVMap.SetLength( bufVertex.GetLength() ) ;
	for ( size_t i = 0, j = 0; i < nCount; i ++, j += 2 )
	{
		if ( i < nFaceVertexIndex )
		{
			size_t	iVertex = pFaceVertexIndex[i] ;
			//
			S2DVector	vUVMap ;
			vUVMap.x = pFloat[j] ;
			vUVMap.y = pFloat[j + 1] ;
			//
			bool	fDoubleVertex = false ;
			for ( size_t k = 0; k < i; k ++ )
			{
				// 重複頂点判定
				size_t	kVertex = pFaceVertexIndex[k] ;
				if ( kVertex == iVertex )
				{
					// 重複頂点で異なる UV 座標判定
					fDoubleVertex = true ;
					if ( bufUVMap.At(kVertex) == vUVMap )
					{
						iVertex = kVertex ;
						fDoubleVertex = false ;
						break ;
					}
				}
			}
			if ( fDoubleVertex )
			{
				// UV の異なる頂点を増やす
				size_t		k = bufVertex.GetLength() ;
				S3DVector4	vVertex = bufVertex.At( iVertex ) ;
				bufVertex.SetAt( k, vVertex ) ;
				iVertex = k ;
				pFaceVertexIndex[i] = iVertex ;
				//
				for ( k = 0; k < nFaceIndexIndex; k ++ )
				{
					if ( pFaceIndexIndex[k] == i )
					{
						bufIndex.SetAt( k, (uint32_t) iVertex ) ;
					}
				}
			}
			bufUVMap.SetAt( iVertex, vUVMap ) ;
		}
	}
	aFaceVertexIndex.FinishArray() ;
	//
	if ( pMappingInf != nullptr )
	{
		S2DVector *	pvUVMap = bufUVMap.GetArray() ;
		nCount = bufUVMap.GetLength() ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			if ( pMappingInf->flagFlipX )
			{
				pvUVMap[i].x = 1.0f - pvUVMap[i].x ;
			}
			if ( pMappingInf->flagFlipY )
			{
				pvUVMap[i].y = 1.0f - pvUVMap[i].y ;
			}
			pvUVMap[i].x *= (float) pMappingInf->nRepeatX ;
			pvUVMap[i].y *= (float) pMappingInf->nRepeatY ;
			if ( pMappingInf->flagSwapXY )
			{
				float	x = pvUVMap[i].x ;
				pvUVMap[i].x = pvUVMap[i].y ;
				pvUVMap[i].y = x ;
			}
		}
	}
	return	sglErrSuccess ;
}


//////////////////////////////////////////////////////////////////////////////
// EntisGLS4 標準モデルファイル・セーバー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DStdModelSaver, S3DModelSaverInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DStdModelSaver::S3DStdModelSaver( void )
{
	m_strImageMIME = L"image/x-eri" ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DStdModelSaver::~S3DStdModelSaver( void )
{
}

// MIME 判定
//////////////////////////////////////////////////////////////////////////////
bool S3DStdModelSaver::IsMatchableMIMEType( const wchar_t * pszMIME )
{
	return	(SString::CompareNoCase( pszMIME, L"application/x-mdfx" ) == 0) ;
}

// テクスチャ画像保存形式設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdModelSaver::SetImageFormat
	( const wchar_t * pwszMIME, const wchar_t * pwszExt,
		const SGLImageEncoderInterface::Options * pOpt )
{
	m_strImageMIME = pwszMIME ;
	return	sglErrSuccess ;
}

// モデルデータ読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdModelSaver::WriteModel
	( SSystem::SFileInterface & file, S3DModelBuffer & model )
{
	SChunkFile::FILE_HEADER	fhdr ;
	fhdr.SetHeaderInfo
		( SChunkFile::fidEGL3DModel2, "EntisGLS4 standard model file" ) ;
	//
	SChunkFile	cf ;
	if ( cf.OpenChunkFile
		( &file, false, SFileOpener::modeCreate, &fhdr ) )
	{
		return	sglErrFailed ;
	}
	//
	// テクスチャ
	//
	if ( cf.DescendChunk( "texture " ) )
	{
		return	sglErrFailed ;
	}
	WriteTextureChunk( cf, model ) ;
	cf.AscendChunk() ;
	//
	// 表面属性
	//
	if ( cf.DescendChunk( "material" ) )
	{
		return	sglErrFailed ;
	}
	WriteMaterialChunk( cf, model ) ;
	cf.AscendChunk() ;
	//
	// メッシュ書き出し
	//
	if ( cf.DescendChunk( "mesh    " ) )
	{
		return	sglErrFailed ;
	}
	WriteMeshChunk( cf, model ) ;
	cf.AscendChunk() ;
	//
	// メッシュグループ情報書き出し
	//
	if ( cf.DescendChunk( "mesh_grp" ) )
	{
		return	sglErrFailed ;
	}
	WriteMeshGroupChunk( cf, model ) ;
	cf.AscendChunk() ;
	//
	// モーフターゲット書き出し
	//
	if ( cf.DescendChunk( "morphmsh" ) )
	{
		return	sglErrFailed ;
	}
	WriteMeshMorphTargetChunk( cf, model ) ;
	cf.AscendChunk() ;
	//
	// 分割メッシュ情報書き出し
	//
	if ( cf.DescendChunk( "mesh_div" ) )
	{
		return	sglErrFailed ;
	}
	WriteMeshDivisionInfoChunk( cf, model ) ;
	cf.AscendChunk() ;
	//
	// ボーン書き出し
	//
	if ( cf.DescendChunk( "bones   " ) )
	{
		return	sglErrFailed ;
	}
	WriteBoneChunk( cf, model ) ;
	cf.AscendChunk() ;
	//
	// マーカー情報書き出し
	//
	if ( cf.DescendChunk( "markinfo" ) )
	{
		return	sglErrFailed ;
	}
	WriteMarkerInfoChunk( cf, model ) ;
	cf.AscendChunk() ;
	//
	// クロスシミュレーターメッシュ情報書き出し
	//
/*	if ( cf.DescendChunk( "clothinf" ) )
	{
		return	sglErrFailed ;
	}
	WriteClothMeshInfoChunk( cf, model ) ;
	cf.AscendChunk() ;
*/	//
	// ポーズ書き出し
	//
	if ( cf.DescendChunk( "poses   " ) )
	{
		return	sglErrFailed ;
	}
	WritePosesChunk( cf, model ) ;
	cf.AscendChunk() ;
	//
	// メタ情報書き出し
	//
	if ( cf.DescendChunk( "metainfo" ) )
	{
		return	sglErrFailed ;
	}
	WriteMetaInfoChunk( cf, model ) ;
	cf.AscendChunk() ;
	//
	// シーン書き出し
	//
	if ( cf.DescendChunk( "scene   " ) )
	{
		return	sglErrFailed ;
	}
	WriteSceneChunk( cf, model ) ;
	cf.AscendChunk() ;
	//
	// 拡張データ書き出し
	//
	WriteUserChunk( cf, model ) ;
	//
	// 書き出し完了
	//
	cf.Close() ;
	//
	return	sglErrSuccess ;
}

// テクスチャ書き出し
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdModelSaver::WriteTextureChunk
		( SChunkFile & file, S3DModelBuffer & model )
{
	S3DTextureLibrary&	txlib = model.GetTextureLibrary() ;
	S3DMaterialLibrary&	matlib = model.GetMaterialLibrary() ;
	for ( size_t i = 0; i < txlib.GetTextureCount(); i ++ )
	{
		SGLImageObject *	pTexture = txlib.GetTextureAt( i ) ;
		ESLAssert( pTexture != nullptr ) ;
		if ( pTexture == nullptr )
		{
			continue ;
		}
		SObjectArray<SString>	aStrings ;
		aStrings.Add( new SString( txlib.GetTextureIdentityAt( i ) ) ) ;
		aStrings.Add( new SString( m_strImageMIME ) ) ;
		//
		file.DescendChunk( "image   " ) ;
		//
		// 定義文字列書き出し
		//
		file.DescendChunk( "strings " ) ;
		WriteStrings( file, aStrings ) ;
		file.AscendChunk() ;
		//
		// 情報書き出し
		//
		S3DStdModelLoader::TextureInfo	txinf ;
		eslFillMemory( &txinf, 0, sizeof(S3DStdModelLoader::TextureInfo) ) ;
		//
		if ( pTexture->GetBufferFlags()
				& SGLImageObject::bufferForMipmapTexture )
		{
			txinf.nFlags |= S3DStdModelLoader::textureMipmap ;
		}
		if ( pTexture->GetBufferFlags()
				& SGLImageObject::bufferCompressedTexture )
		{
			txinf.nFlags |= S3DStdModelLoader::textureCompressed ;
		}
		file.DescendChunk( "txt_info" ) ;
		file.Write( &txinf, sizeof(S3DStdModelLoader::TextureInfo) ) ;
		file.AscendChunk() ;
		//
		// 画像データ書き出し
		//
		SSmartBuffer	sbuf ;
		pTexture->WriteImage( &sbuf, m_strImageMIME ) ;
		//
		file.DescendChunk( "img_data" ) ;
		sbuf.WriteToStream( file ) ;
		file.AscendChunk() ;
		//
		file.AscendChunk() ;
	}
	return	sglErrSuccess ;
}

// 表面属性書き出し
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdModelSaver::WriteMaterialChunk
		( SChunkFile & file, S3DModelBuffer & model )
{
	S3DTextureLibrary&	txlib = model.GetTextureLibrary() ;
	S3DMaterialLibrary&	matlib = model.GetMaterialLibrary() ;
	for ( size_t i = 0; i < matlib.GetMaterialCount(); i ++ )
	{
		S3DMaterial *	pMaterial = matlib.GetMaterialAt( i ) ;
		if ( pMaterial == nullptr )
		{
			continue ;
		}
		//
		// 文字列配列準備
		//
		SObjectArray<SString>	aStrings ;
		aStrings.Add( new SString( matlib.GetMaterialIdentityAt( i ) ) ) ;
		//
		for ( int j = 0; j < S3DMaterial::textureMaxCount; j ++ )
		{
			SGLImageObject *	pImage = pMaterial->GetTexture( j ) ;
			if ( pImage != nullptr )
			{
				ssize_t	iTexture = txlib.FindTexturePtr( pImage ) ;
				if ( iTexture >= 0 )
				{
					const wchar_t *	pwszID =
						txlib.GetTextureIdentityAt( (size_t) iTexture ) ;
					if ( FindString( aStrings, pwszID ) < 0 )
					{
						aStrings.Add( new SString( pwszID ) ) ;
					}
				}
			}
			pImage = pMaterial->GetBackTexture( j ) ;
			if ( pImage != nullptr )
			{
				ssize_t	iTexture = txlib.FindTexturePtr( pImage ) ;
				if ( iTexture >= 0 )
				{
					const wchar_t *	pwszID =
						txlib.GetTextureIdentityAt( (size_t) iTexture ) ;
					if ( FindString( aStrings, pwszID ) < 0 )
					{
						aStrings.Add( new SString( pwszID ) ) ;
					}
				}
			}
		}
		//
		file.DescendChunk( "surface " ) ;
		//
		// 定義文字列書き出し
		//
		file.DescendChunk( "strings " ) ;
		WriteStrings( file, aStrings ) ;
		file.AscendChunk() ;
		//
		// 表面属性
		//
		S3DSurfaceAttribute	sufattr ;
		pMaterial->GetSurfaceAttribute( sufattr ) ;
		//
		file.DescendChunk( "faceattr" ) ;
		file.Write( &sufattr, sizeof(S3DSurfaceAttribute) ) ;
		file.AscendChunk() ;
		//
		// 表面テクスチャ
		//
		S3DStdModelLoader::SurfaceTextures	suftxt ;
		eslFillMemory
			( &suftxt, 0, sizeof(S3DStdModelLoader::SurfaceTextures) ) ;
		//
		for ( int j = 0; j < S3DMaterial::textureMaxCount; j ++ )
		{
			SGLImageObject *	pImage = pMaterial->GetTexture( j ) ;
			suftxt.txtEntries[j].iTexture = (uint32_t) -1 ;
			if ( pImage != nullptr )
			{
				ssize_t	iTexture = txlib.FindTexturePtr( pImage ) ;
				if ( iTexture >= 0 )
				{
					const wchar_t *	pwszID =
						txlib.GetTextureIdentityAt( (size_t) iTexture ) ;
					ssize_t	k = FindString( aStrings, pwszID ) ;
					ESLAssert( k >= 0 ) ;
					suftxt.txtEntries[j].iTexture = (uint32_t) k ;
					suftxt.txtEntries[j].nFlags =
								pMaterial->GetTextureFlags( j ) ;
					suftxt.txtEntries[j].fpApply =
								pMaterial->GetTextureApplication( j ) ;
					suftxt.txtEntries[j].fpParam1 =
								pMaterial->GetTextureParameter( j ) ;
					suftxt.nCount = (uint32_t) j + 1 ;
				}
			}
		}
		file.DescendChunk( "face_txt" ) ;
		file.Write( &suftxt, sizeof(S3DStdModelLoader::SurfaceTextures) ) ;
		file.AscendChunk() ;
		//
		if ( pMaterial->IsEnabledBackSurfaceAttribute() )
		{
			//
			// 裏面属性
			//
			pMaterial->GetBackSurfaceAttribute( sufattr ) ;
			//
			file.DescendChunk( "backattr" ) ;
			file.Write( &sufattr, sizeof(S3DSurfaceAttribute) ) ;
			file.AscendChunk() ;
			//
			// 裏面テクスチャ
			//
			eslFillMemory
				( &suftxt, 0, sizeof(S3DStdModelLoader::SurfaceTextures) ) ;
			//
			for ( int j = 0; j < S3DMaterial::textureMaxCount; j ++ )
			{
				SGLImageObject *	pImage = pMaterial->GetBackTexture( j ) ;
				suftxt.txtEntries[j].iTexture = (uint32_t) -1 ;
				if ( pImage != nullptr )
				{
					ssize_t	iTexture = txlib.FindTexturePtr( pImage ) ;
					if ( iTexture >= 0 )
					{
						const wchar_t *	pwszID =
							txlib.GetTextureIdentityAt( (size_t) iTexture ) ;
						ssize_t	k = FindString( aStrings, pwszID ) ;
						ESLAssert( k ) ;
						suftxt.txtEntries[j].iTexture = (uint32_t) k ;
						suftxt.txtEntries[j].nFlags =
								pMaterial->GetBackTextureFlags( j ) ;
						suftxt.txtEntries[j].fpApply =
								pMaterial->GetBackTextureApplication( j ) ;
						suftxt.txtEntries[j].fpParam1 =
								pMaterial->GetBackTextureParameter( j ) ;
						suftxt.nCount = (uint32_t) j + 1 ;
					}
				}
			}
			file.DescendChunk( "back_txt" ) ;
			file.Write( &suftxt, sizeof(S3DStdModelLoader::SurfaceTextures) ) ;
			file.AscendChunk() ;
		}
		file.AscendChunk() ;
	}
	return	sglErrSuccess ;
}

// メッシュ書き出し
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdModelSaver::WriteMeshChunk
		( SChunkFile & file, S3DModelBuffer & model )
{
	S3DMaterialLibrary&	matlib = model.GetMaterialLibrary() ;
	size_t	nCount = model.GetMeshCount() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		S3DModelBuffer::MeshObject *	pMesh = model.GetMeshObjectAt( i ) ;
		if ( pMesh == nullptr )
		{
			continue ;
		}
		ssize_t	iMaterial = matlib.FindMaterialPtr( pMesh->m_pMaterial ) ;
		if ( iMaterial < 0 )
		{
			continue ;
		}
		if ( pMesh->m_typeMesh == primitiveTriangle )
		{
			file.DescendChunk( "itrilist" ) ;
		}
		else if ( pMesh->m_typeMesh == primitiveTriangleStrip )
		{
			file.DescendChunk( "tristrip" ) ;
		}
		else
		{
			file.DescendChunk( "primitiv" ) ;
		}
		//
		// 定義文字列
		//
		SObjectArray<SString>	aStrings ;
		aStrings.Add( new SString
				( matlib.GetMaterialIdentityAt( (size_t) iMaterial ) ) ) ;
		//
		file.DescendChunk( "strings " ) ;
		WriteStrings( file, aStrings ) ;
		file.AscendChunk() ;
		//
		// メッシュ情報
		//
		S3DStdModelLoader::TriangleMeshInfo	meshinf ;
		eslFillMemory
			( &meshinf, 0, sizeof(S3DStdModelLoader::TriangleMeshInfo) ) ;
		meshinf.iMaterial = 0 ;
		meshinf.countPolygon = (uint32_t) pMesh->m_countPolygon ;
		meshinf.countVertex = (uint32_t) pMesh->m_countVertex ;
		meshinf.nFlags = S3DStdModelLoader::meshFlagSubMeshDensity
						| S3DStdModelLoader::meshFlagPrimitiveType ;
		meshinf.fpSubMeshDensity = pMesh->m_fpSubMeshDensity ;
		meshinf.typePrimitive = pMesh->m_typeMesh ;
		meshinf.nExAttrElements = (uint32_t) pMesh->m_nExAttrElements ;
		//
		file.DescendChunk( "meshinfo" ) ;
		file.Write( &meshinf, sizeof(S3DStdModelLoader::TriangleMeshInfo) ) ;
		file.AscendChunk() ;
		//
		// 頂点バッファ
		//
		file.DescendChunk( "vertex  " ) ;
		file.Write
			( model.GetVertexBufferAt( pMesh->m_iVertex ),
				pMesh->m_countVertex * sizeof(S3DVector4) ) ;
		file.AscendChunk() ;
		//
		file.DescendChunk( "normal  " ) ;
		file.Write
			( model.GetNormalBufferAt( pMesh->m_iNormal ),
				pMesh->m_countVertex * sizeof(S3DVector4) ) ;
		file.AscendChunk() ;
		//
		if ( pMesh->m_bufUVMap.GetLength() >= pMesh->m_countVertex )
		{
			file.DescendChunk( "uv_map  " ) ;
			file.Write
				( pMesh->m_bufUVMap.GetConstArray(),
					pMesh->m_countVertex * sizeof(S2DVector) ) ;
			file.AscendChunk() ;
		}
		//
		if ( pMesh->m_bufColor.GetLength() >= pMesh->m_countVertex )
		{
			file.DescendChunk( "color   " ) ;
			file.Write
				( pMesh->m_bufColor.GetConstArray(),
					pMesh->m_countVertex * sizeof(S3DColor) ) ;
			file.AscendChunk() ;
		}
		//
		if ( pMesh->m_bufExAttrElements.GetLength()
					>= pMesh->m_countVertex * pMesh->m_nExAttrElements )
		{
			file.DescendChunk( "ex_attr " ) ;
			file.Write
				( pMesh->m_bufExAttrElements.GetConstArray(),
					pMesh->m_countVertex
						* pMesh->m_nExAttrElements * sizeof(float32_t) ) ;
			file.AscendChunk() ;
		}
		//
		if ( pMesh->m_bufIndex.GetLength() >= pMesh->m_countPolygon * 3 )
		{
			file.DescendChunk( "index   " ) ;
			file.Write
				( pMesh->m_bufIndex.GetConstArray(),
					pMesh->m_countPolygon * sizeof(uint32_t) * 3 ) ;
			file.AscendChunk() ;
		}
		//
		for ( size_t iSub = 0; iSub < VertexBuffer::countSubMesh; iSub ++ )
		{
			if ( (pMesh->m_countSubPoly[iSub] == 0)
				|| (pMesh->m_bufSubIndex[iSub].GetLength() == 0) )
			{
				continue ;
			}
			char	szChunkID[0x10] = "index1  " ;
			szChunkID[5] = '1' + (char) iSub ;
			file.DescendChunk( szChunkID ) ;
			file.Write
				( pMesh->m_bufSubIndex[iSub].GetConstArray(),
					pMesh->m_countSubPoly[iSub] * sizeof(uint32_t) * 3 ) ;
			file.AscendChunk() ;
		}
		//
		file.AscendChunk() ;
	}
	return	sglErrSuccess ;
}

// メッシュグループ情報書き出し
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdModelSaver::WriteMeshGroupChunk
		( SChunkFile & file, S3DModelBuffer & model )
{
	//
	// 定義文字列準備
	//
	SStrSortArray<S3DModelBuffer::MeshGroup>&
							ssaMeshGroup = model.GetMeshGroupList() ;
	SObjectArray<SString>	aStrings ;
	for ( size_t i = 0; i < ssaMeshGroup.GetLength(); i ++ )
	{
		const SString *	pstrID = ssaMeshGroup.GetTagAt( i ) ;
		ESLAssert( pstrID != nullptr ) ;
		if ( pstrID == nullptr )
		{
			continue ;
		}
		aStrings.Add( new SString( *pstrID ) ) ;
	}
	file.DescendChunk( "strings " ) ;
	WriteStrings( file, aStrings ) ;
	file.AscendChunk() ;
	//
	for ( size_t i = 0; i < ssaMeshGroup.GetLength(); i ++ )
	{
		//
		// メッシュグループ情報
		//
		S3DModelBuffer::MeshGroup *	pMeshGroup = ssaMeshGroup.GetAt( i ) ;
		ESLAssert( pMeshGroup != nullptr ) ;
		if ( pMeshGroup == nullptr )
		{
			continue ;
		}
		file.DescendChunk( "group   " ) ;
		file.Write( pMeshGroup, sizeof(S3DModelBuffer::MeshGroup) ) ;
		file.AscendChunk() ;
	}
	return	sglErrSuccess ;
}

// モーフターゲット書き出し
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdModelSaver::WriteMeshMorphTargetChunk
	( SSystem::SChunkFile& file, S3DModelBuffer & model )
{
	SStrSortObjectArray<S3DModelBuffer::MorphTargetMesh>&
							ssoaMorph = model.GetMorphTargetList() ;
	//
	// 定義文字列準備
	//
	SObjectArray<SString>	aStrings ;
	for ( size_t i = 0; i < ssoaMorph.GetLength(); i ++ )
	{
		const SString *	pstrID = ssoaMorph.GetTagAt( i ) ;
		ESLAssert( pstrID != nullptr ) ;
		if ( pstrID == nullptr )
		{
			continue ;
		}
		aStrings.Add( new SString( *pstrID ) ) ;
	}
	file.DescendChunk( "strings " ) ;
	WriteStrings( file, aStrings ) ;
	file.AscendChunk() ;
	//
	// モーフターゲットメッシュ順次書き出し
	//
	const size_t	nMeshs = model.GetMeshCount() ;
	for ( size_t i = 0; i < ssoaMorph.GetLength(); i ++ )
	{
		S3DModelBuffer::MorphTargetMesh *	pmtm = ssoaMorph.GetAt( i ) ;
		const SString *	pstrID = ssoaMorph.GetTagAt( i ) ;
		ESLAssert( pmtm != nullptr ) ;
		ESLAssert( pstrID != nullptr ) ;
		//
		file.DescendChunk( "mesh    " ) ;
		//
		// モーフメッシュ情報
		//
		S3DStdModelLoader::MorphingMeshInfo	meshinf ;
		SArray<uint32_t>	aRelMesh ;
		eslFillMemory
			( &meshinf, 0, sizeof(S3DStdModelLoader::MorphingMeshInfo) ) ;
		//
		for ( size_t j = 0; j < nMeshs; j ++ )
		{
			S3DModelBuffer::MeshObject *
						pMesh = model.GetMeshObjectAt( j ) ;
			if ( pMesh == nullptr )
			{
				continue ;
			}
			for ( size_t k = 0; k < pMesh->m_arrMorphTarget.GetLength(); k ++ )
			{
				SString *	pstrMeshID = pMesh->m_arrMorphTarget.GetAt( k ) ;
				ESLAssert( pstrMeshID != nullptr ) ;
				if ( (pstrMeshID != nullptr) && (*pstrMeshID == *pstrID) )
				{
					aRelMesh.Add( (uint32_t) j ) ;
					break ;
				}
			}
		}
		meshinf.iMeshID = (uint32_t) i ;
		meshinf.countVertex = (uint32_t) pmtm->m_countVertex ;
		meshinf.countRelMesh = (uint32_t) aRelMesh.GetLength() ;
		//
		file.DescendChunk( "meshinfo" ) ;
		file.Write( &meshinf, sizeof(S3DStdModelLoader::MorphingMeshInfo) ) ;
		file.Write
			( aRelMesh.GetConstArray(),
				meshinf.countRelMesh * sizeof(uint32_t) ) ;
		file.AscendChunk() ;
		//
		// 頂点バッファ
		//
		if ( pmtm->m_bufVertex.GetLength() >= meshinf.countVertex )
		{
			file.DescendChunk( "vertex  " ) ;
			file.Write
				( pmtm->m_bufVertex.GetConstArray(),
					meshinf.countVertex * sizeof(S3DVector4) ) ;
			file.AscendChunk() ;
		}
		if ( pmtm->m_bufNormal.GetLength() >= meshinf.countVertex )
		{
			file.DescendChunk( "normal  " ) ;
			file.Write
				( pmtm->m_bufNormal.GetConstArray(),
					meshinf.countVertex * sizeof(S3DVector4) ) ;
			file.AscendChunk() ;
		}
		if ( pmtm->m_bufUVMap.GetLength() >= meshinf.countVertex )
		{
			file.DescendChunk( "uv_map  " ) ;
			file.Write
				( pmtm->m_bufUVMap.GetConstArray(),
					meshinf.countVertex * sizeof(S2DVector) ) ;
			file.AscendChunk() ;
		}
		if ( pmtm->m_bufColor.GetLength() >= meshinf.countVertex )
		{
			file.DescendChunk( "color   " ) ;
			file.Write
				( pmtm->m_bufColor.GetConstArray(),
					meshinf.countVertex * sizeof(S3DColor) ) ;
			file.AscendChunk() ;
		}
		if ( pmtm->m_bufWeight.GetLength() >= meshinf.countVertex )
		{
			file.DescendChunk( "weight  " ) ;
			file.Write
				( pmtm->m_bufWeight.GetConstArray(),
					meshinf.countVertex * sizeof(float32_t) ) ;
			file.AscendChunk() ;
		}
		//
		file.AscendChunk() ;
	}
	return	sglErrSuccess ;
}

// 分割メッシュ情報書き出し
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdModelSaver::WriteMeshDivisionInfoChunk
	( SSystem::SChunkFile& file, S3DModelBuffer & model )
{
	SStrSortObjectArray<S3DModelBuffer::MeshDivision>&
							ssoaMeshDiv = model.GetMeshDivisionList() ;
	//
	// 定義文字列準備
	//
	SIndexedArray<SString,const wchar_t*>	aStrings ;
	for ( size_t i = 0; i < ssoaMeshDiv.GetLength(); i ++ )
	{
		const SString *	pstrID = ssoaMeshDiv.GetTagAt( i ) ;
		ESLAssert( pstrID != nullptr ) ;
		if ( pstrID == nullptr )
		{
			continue ;
		}
		aStrings.Add( new SString( *pstrID ) ) ;
		//
		S3DModelBuffer::MeshDivision *	pMeshDiv = ssoaMeshDiv.GetAt( i ) ;
		ESLAssert( pMeshDiv != nullptr ) ;
		if ( pstrID == nullptr )
		{
			continue ;
		}
		for ( size_t j = 0; j < pMeshDiv->m_aMorphEntries.GetLength(); j ++ )
		{
			ESLAssert( pMeshDiv->m_aMorphEntries.GetAt(j) != nullptr ) ;
			if ( pMeshDiv->m_aMorphEntries.GetAt(j) != nullptr )
			{
				aStrings.Add( new SString( pMeshDiv->m_aMorphEntries.At(j) ) ) ;
			}
		}
		for ( size_t j = 0; j < pMeshDiv->m_aSplittedEntries.GetLength(); j ++ )
		{
			S3DModelBuffer::MeshDivision::SplittedEntry *
				pSplitted = pMeshDiv->m_aSplittedEntries.GetAt( j ) ;
			ESLAssert( pSplitted != nullptr ) ;
			if ( pSplitted == nullptr )
			{
				continue ;
			}
			aStrings.Add( new SString( pSplitted->m_strSplittedMesh ) ) ;
			for ( size_t k = 0; k < pSplitted->m_aSplittedMorph.GetLength(); k ++ )
			{
				ESLAssert( pSplitted->m_aSplittedMorph.GetAt(k) != nullptr ) ;
				if ( pSplitted->m_aSplittedMorph.GetAt(k) != nullptr )
				{
					aStrings.Add( new SString( pSplitted->m_aSplittedMorph.At(k) ) ) ;
				}
			}
		}
	}
	file.DescendChunk( "strings " ) ;
	WriteStrings( file, aStrings ) ;
	file.AscendChunk() ;
	//
	// 分割メッシュ情報順次書き出し
	//
	for ( size_t i = 0; i < ssoaMeshDiv.GetLength(); i ++ )
	{
		S3DModelBuffer::MeshDivision *	pMeshDiv = ssoaMeshDiv.GetAt( i ) ;
		const SString *					pstrID = ssoaMeshDiv.GetTagAt( i ) ;
		if ( (pstrID == nullptr) || (pMeshDiv == nullptr) )
		{
			continue ;
		}
		file.DescendChunk( "mesh    " ) ;
		//
		// 分割情報
		//
		S3DStdModelLoader::MeshDivisionInfo	divinf ;
		SArray<uint32_t>					aMorphEntry ;
		eslFillMemory
			( &divinf, 0, sizeof(S3DStdModelLoader::MeshDivisionInfo) ) ;
		ESLAssert( aStrings.FindIndex( *pstrID ) >= 0 ) ;
		divinf.iMeshID = (uint32_t) aStrings.FindIndex( *pstrID ) ;
		divinf.countMorphList = (uint32_t) pMeshDiv->m_aMorphEntries.GetLength() ;
		divinf.countDivision = (uint32_t) pMeshDiv->m_aSplittedEntries.GetLength() ;
		//
		aMorphEntry.SetLimit( pMeshDiv->m_aMorphEntries.GetLength() ) ;
		for ( size_t j = 0; j < pMeshDiv->m_aMorphEntries.GetLength(); j ++ )
		{
			ESLAssert( pMeshDiv->m_aMorphEntries.GetAt(j) != nullptr ) ;
			ESLAssert( aStrings.FindIndex( pMeshDiv->m_aMorphEntries.At(j) ) >= 0 ) ;
			aMorphEntry.SetAt
				( j, (uint32_t) aStrings.FindIndex( pMeshDiv->m_aMorphEntries.At(j) ) ) ;
		}
		//
		file.DescendChunk( "div_info" ) ;
		file.Write( &divinf, sizeof(S3DStdModelLoader::MeshDivisionInfo) ) ;
		file.Write
			( aMorphEntry.GetConstArray(),
				divinf.countMorphList * sizeof(uint32_t) ) ;
		file.AscendChunk() ;
		//
		// 各分割情報
		//
		for ( size_t iDiv = 0; iDiv < divinf.countDivision; iDiv ++ )
		{
			S3DModelBuffer::MeshDivision::SplittedEntry *
					pSplitted = pMeshDiv->m_aSplittedEntries.GetAt( iDiv ) ;
			ESLAssert( pSplitted != nullptr ) ;
			if ( pSplitted == nullptr )
			{
				continue ;
			}
			if ( file.DescendChunk( "div_mesh" ) )
			{
				file.AscendChunk() ;
				return	sglErrFailed ;
			}
			ESLAssert( aStrings.FindIndex( pSplitted->m_strSplittedMesh ) >= 0 ) ;
			uint32_t	iDivMeshID =
				(uint32_t) aStrings.FindIndex( pSplitted->m_strSplittedMesh ) ;
			file.Write( &iDivMeshID, sizeof(uint32_t) ) ;
			//
			for ( size_t j = 0; j < divinf.countMorphList; j ++ )
			{
				ESLAssert( pSplitted->m_aSplittedMorph.GetAt(j) != nullptr ) ;
				ESLAssert( aStrings.FindIndex( pSplitted->m_aSplittedMorph.At(j) ) >= 0 ) ;
				aMorphEntry.SetAt
					( j, (uint32_t) aStrings.FindIndex( pSplitted->m_aSplittedMorph.At(j) ) ) ;
			}
			file.Write
				( aMorphEntry.GetConstArray(),
					divinf.countMorphList * sizeof(uint32_t) ) ;
			file.AscendChunk() ;
		}
		file.AscendChunk() ;
	}
	return	sglErrSuccess ;
}

// ボーン書き出し
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdModelSaver::WriteBoneChunk
		( SChunkFile & file, S3DModelBuffer & model )
{
	SStrSortObjectArray<S3DModelBoneSpace>&
					bones = model.GetBonePropertyList() ;
	SStrSortArray<S3DModelBoneSpace::PhysMaterial>&
					physPalette = model.GetPhysMaterialList() ;
	//
	// 定義文字列準備
	//
	SObjectArray<SString>	aStrings ;
	for ( size_t i = 0; i < bones.GetLength(); i ++ )
	{
		const SString *	pstrID = bones.GetTagAt( i ) ;
		ESLAssert( pstrID != nullptr ) ;
		if ( pstrID != nullptr )
		{
			aStrings.Add( new SString( *pstrID ) ) ;
		}
		S3DModelBoneSpace *	pBone = bones.GetAt( i ) ;
		if ( pBone != nullptr )
		{
			if ( !pBone->GetBonePhysicalMaterialID().IsEmpty() )
			{
				aStrings.Add
					( new SString( pBone->GetBonePhysicalMaterialID() ) ) ;
			}
			for ( size_t j = 0; j < pBone->GetEffectivePhysBoneCount(); j ++ )
			{
				S3DModelBoneSpace::EffectiveBoneEntry *
						pebe = pBone->GettEffectivePhysBoneAt( i ) ;
				if ( (pebe != nullptr)
					&& !pebe->m_strBoneID.IsEmpty()
					&& (FindString(aStrings,pebe->m_strBoneID) < 0) )
				{
					aStrings.Add( new SString( pebe->m_strBoneID ) ) ;
				}
			}
		}
	}
	for ( size_t i = 0; i < physPalette.GetLength(); i ++ )
	{
		const SString *	pstrID = physPalette.GetTagAt( i ) ;
		if ( (pstrID != nullptr) && (FindString( aStrings, *pstrID ) < 0) )
		{
			aStrings.Add( new SString( *pstrID ) ) ;
		}
	}
	file.DescendChunk( "strings " ) ;
	WriteStrings( file, aStrings ) ;
	file.AscendChunk() ;
	//
	for ( size_t i = 0; i < bones.GetLength(); i ++ )
	{
		const SString *	pstrID = bones.GetTagAt( i ) ;
		S3DModelBoneSpace *	pBone = bones.GetAt( i ) ;
		ESLAssert( pBone != nullptr ) ;
		if ( (pstrID == nullptr) || (pBone == nullptr) )
		{
			continue ;
		}
		file.DescendChunk( "bone    " ) ;
		//
		// ボーン基本情報
		//
		S3DStdModelLoader::BoneInfo	boneinf ;
		eslFillMemory( &boneinf, 0, sizeof(S3DStdModelLoader::BoneInfo) ) ;
		boneinf.iBoneID = (uint32_t) FindString( aStrings, *pstrID ) ;
		boneinf.iParentID = (uint32_t) -1 ;
		boneinf.vBoneBase = pBone->m_vCenter ;
		boneinf.vBoneHandle = pBone->GetBoneHandle() ;
		boneinf.flagsBone = pBone->GetBoneFlags()
							& ~S3DModelBoneSpace::flagFreezePhysics ;
		boneinf.iMaterialRefID = (uint32_t) -1 ;
		boneinf.physMaterial = pBone->GetBonePhysicalMaterial() ;
		boneinf.mat4OrgBone = pBone->GetOriginalBoneMatrix() ;
		//
		S3DModelBoneSpace *	pParentBone =
			ESLTypeCast<S3DModelBoneSpace>( pBone->GetParentSpace() ) ;
		if ( (pParentBone != nullptr)
			&& (&(model.GetBoneRoot()) != pParentBone) )
		{
			ssize_t	iParent = bones.FindPtr( pParentBone ) ;
			if ( iParent >= 0 )
			{
				const SString *	pstrParentID =
							bones.GetTagAt( (size_t) iParent ) ;
				ESLAssert( pstrParentID != nullptr ) ;
				iParent = FindString( aStrings, *pstrParentID ) ;
				if ( iParent >= 0 )
				{
					boneinf.iParentID = (uint32_t) iParent ;
				}
			}
		}
		if ( !pBone->GetBonePhysicalMaterialID().IsEmpty() )
		{
			boneinf.iMaterialRefID =
				(uint32_t) FindString
						( aStrings, pBone->GetBonePhysicalMaterialID() ) ;
		}
		file.DescendChunk( "boneinfo" ) ;
		file.Write( &boneinf, sizeof(S3DStdModelLoader::BoneInfo) ) ;
		file.AscendChunk() ;
		//
		// IK 用パラメータ
		//
		file.DescendChunk( "ik_param" ) ;
		file.Write( &(pBone->GetIKParameter()),
						sizeof(S3DModelBoneSpace::IKParameter) ) ;
		file.AscendChunk() ;
		//
		// 物理演算・被影響ボーン
		//
		file.DescendChunk( "efphybon" ) ;
		for ( size_t j = 0; j < pBone->GetEffectivePhysBoneCount(); j ++ )
		{
			S3DModelBoneSpace::EffectiveBoneEntry *
				pebe = pBone->GettEffectivePhysBoneAt( j ) ;
			if ( pebe != nullptr )
			{
				S3DStdModelLoader::PhysEffectiveBoneInfo	pebi ;
				pebi.nBytes = sizeof(S3DStdModelLoader::PhysEffectiveBoneInfo) ;
				pebi.nType = 0 ;
				pebi.nReserved = 0 ;
				pebi.iBoneID =
					(uint32_t) FindString( aStrings, pebe->m_strBoneID ) ;
				pebi.fpWeight = pebe->m_fpWeight ;
				file.Write( &pebi, sizeof(S3DStdModelLoader::PhysEffectiveBoneInfo) ) ;
			}
		}
		file.AscendChunk() ;
		//
		// ウェイトマップ
		//
		const size_t		iWeightVertex =
									pBone->VertexIndexOfBoneWeight() ;
		const size_t		iWeightNormal =
									pBone->NormalIndexOfBoneWeight() ;
		const size_t		nWeightMapLength =
									pBone->VertexCountOfBoneWeight() ;
//		const float32_t *	pWeightMap = pBone->GetBoneWeightMap() ;
		//
		size_t			nRefMeshs ;
		const S3DModelBoneSpace::REF_MESH_INFO *
						pRefMeshInfo =
							pBone->GetEffectiveMeshIndexArray( nRefMeshs ) ;
		for ( size_t j = 0; j < nRefMeshs; j ++ )
		{
			S3DModelBuffer::MeshObject *
				pMesh = model.GetMeshObjectAt( pRefMeshInfo[j].iMesh ) ;
			if ( pMesh == nullptr )
			{
				continue ;
			}
			if ( ((size_t) pMesh->m_iVertex < iWeightVertex)
				|| ((size_t) pMesh->m_iNormal < iWeightNormal)
				|| ((size_t) pMesh->m_iVertex + pMesh->m_countVertex
									> iWeightVertex + nWeightMapLength)
				|| ((size_t) pMesh->m_iNormal + pMesh->m_countVertex
									> iWeightNormal + nWeightMapLength) )
			{
				continue ;
			}
			S3DStdModelLoader::WeightMapInfo	wmi ;
			eslFillMemory
				( &wmi, 0, sizeof(S3DStdModelLoader::WeightMapInfo) ) ;
			wmi.iMesh = (uint32_t) pRefMeshInfo[j].iMesh ;
			wmi.nCount = (uint32_t) pMesh->m_countVertex ;
			wmi.matIMesh = pRefMeshInfo[j].matIMesh ;
			wmi.matRelMesh = pRefMeshInfo[j].matRelMesh ;
			//
			bool	fRefMesh = false ;
//			const float32_t *
//					pMeshWeight =
//						pWeightMap + (pMesh->m_iVertex - iWeightVertex) ;
			size_t	iLockFirst = pMesh->m_iVertex ;
			size_t	nLockCount = pMesh->m_countVertex ;
			const float32_t *	pMeshWeight =
					pBone->LockBoneWeightMap( iLockFirst, nLockCount ) ;
			ESLAssert( iLockFirst == pMesh->m_iVertex ) ;
			ESLAssert( nLockCount >= pMesh->m_countVertex ) ;
			for ( size_t k = 0; k < wmi.nCount; k ++ )
			{
				if ( pMeshWeight[k] != 0.0f )
				{
					fRefMesh = true ;
					break ;
				}
			}
			if ( fRefMesh )
			{
				S3DStdModelLoader::IndexedWeightMap	ixwmp ;
				S3DStdModelLoader::MakeIndexedWeightMap
					( ixwmp, pMeshWeight, pMesh->m_countVertex ) ;
				if ( ixwmp.bufWeight.GetLength() < pMesh->m_countVertex / 2 )
				{
					wmi.nCount = (uint32_t) ixwmp.bufWeight.GetLength() ;
					wmi.nFlags |= S3DStdModelLoader::flagIndexedWeightMap ;
				}
				//
				file.DescendChunk( "weightmp" ) ;
				file.Write
					( &wmi, sizeof(S3DStdModelLoader::WeightMapInfo) ) ;
				if ( wmi.nFlags & S3DStdModelLoader::flagIndexedWeightMap )
				{
					file.Write
						( ixwmp.bufWeight.GetConstArray(),
							ixwmp.bufWeight.GetLength() * sizeof(float32_t) ) ;
					file.Write
						( ixwmp.bufIndex.GetConstArray(),
							ixwmp.bufIndex.GetLength() * sizeof(uint32_t) ) ;
				}
				else
				{
					file.Write
						( pMeshWeight,
							pMesh->m_countVertex * sizeof(float32_t) ) ;
				}
				file.AscendChunk() ;
			}
			pBone->UnlockBoneWeightMap( false ) ;
		}
		//
		file.AscendChunk() ;
	}
	//
	// 物理演算パラメータパレット
	//
	file.DescendChunk( "physpalt" ) ;
	for ( size_t i = 0; i < physPalette.GetLength(); i ++ )
	{
		const SString *	pstrID = physPalette.GetTagAt( i ) ;
		S3DModelBoneSpace::PhysMaterial *
						pMaterial = physPalette.GetAt( i ) ;
		ESLAssert( pMaterial != nullptr ) ;
		if ( (pstrID == nullptr) || (pMaterial == nullptr) )
		{
			continue ;
		}
		file.DescendChunk( "material" ) ;
		//
		uint32_t	iRefID = (uint32_t) FindString( aStrings, *pstrID ) ;
		file.Write( &iRefID, sizeof(uint32_t) ) ;
		file.Write( pMaterial, sizeof(S3DModelBoneSpace::PhysMaterial) ) ;
		//
		file.AscendChunk() ;
	}
	file.AscendChunk() ;
	//
	return	sglErrSuccess ;
}

// マーカー情報書き出し
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdModelSaver::WriteMarkerInfoChunk
	( SSystem::SChunkFile & file, S3DModelBuffer & model )
{
	SXMLDocument	xmlDoc ;
	model.FormatMarkerXML( xmlDoc ) ;
	xmlDoc.FormatDocument( file ) ;
	//
	return	sglErrSuccess ;
}

// クロスシミュレーターメッシュ情報書き出し
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdModelSaver::WriteClothMeshInfoChunk
	( SSystem::SChunkFile & file, S3DModelBuffer & model )
{
//	SXMLDocument	xmlDoc ;
//	model.FormatClothMeshXML( xmlDoc ) ;
//	xmlDoc.FormatDocument( file ) ;
	//
	return	sglErrSuccess ;
}

// ポーズ書き出し
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdModelSaver::WritePosesChunk
	( SSystem::SChunkFile & file, S3DModelBuffer & model )
{
	return	model.GetPoseLibrary().WriteLibraryChunk( file ) ;
}

// メタ情報書き出し
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdModelSaver::WriteMetaInfoChunk
	( SSystem::SChunkFile & file, S3DModelBuffer & model )
{
	uint32_t	nFlags = 0 ;
	file.Write( &nFlags, sizeof(uint32_t) ) ;
	//
	model.GetMetaInfo().FormatDocument( file ) ;
	//
	return	sglErrSuccess ;
}

// コンポジション書き出し
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdModelSaver::WriteSceneChunk
	( SSystem::SChunkFile & file, S3DModelBuffer & model )
{
	uint32_t	nFlags = 0 ;
	file.Write( &nFlags, sizeof(uint32_t) ) ;
	//
	model.GetSceneComposition().FormatDocument( file ) ;
	//
	return	sglErrSuccess ;
}

// 拡張データ書き出し
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdModelSaver::WriteUserChunk
		( SChunkFile & file, S3DModelBuffer & model )
{
	return	sglErrSuccess ;
}

// 文字列配列書き出し
//////////////////////////////////////////////////////////////////////////////
void S3DStdModelSaver::WriteStrings
	( SSystem::SFileInterface& file,
		const SSystem::SObjectArray<SSystem::SString>& aStrings )
{
	size_t	nTotalLen = 0 ;
	size_t	nCount = aStrings.GetLength() ;
	size_t	i ;
	for ( i = 0; i < nCount; i ++ )
	{
		SString *	pStr = aStrings.GetAt( i ) ;
		ESLAssert( pStr != nullptr ) ;
		if ( pStr != nullptr )
		{
			nTotalLen += pStr->GetLength() + 1 ;
		}
	}
	SArray<uint16_t>	bufStrings ;
	uint16_t *	pwStrBuf = bufStrings.GetArray( nTotalLen ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		SString *	pStr = aStrings.GetAt( i ) ;
		ESLAssert( pStr != nullptr ) ;
		if ( pStr != nullptr )
		{
			const wchar_t *	pwszStr = *pStr ;
			size_t	nLength = pStr->GetLength() ;
			for ( size_t j = 0; j < nLength; j ++ )
			{
				*(pwStrBuf ++) = (uint16_t) pwszStr[j] ;
			}
			*(pwStrBuf ++) = 0 ;
		}
	}
	bufStrings.FinishArray() ;
	//
	file.Write( bufStrings.GetConstArray(), nTotalLen * sizeof(uint16_t) ) ;
}

// 一致文字列検索
//////////////////////////////////////////////////////////////////////////////
ssize_t S3DStdModelSaver::FindString
	( const SSystem::SObjectArray<SSystem::SString>& aStrings, const wchar_t * pwszStr )
{
	size_t	nCount = aStrings.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		SString *	pStr = aStrings.GetAt( i ) ;
		ESLAssert( pStr != nullptr ) ;
		if ( pStr != nullptr )
		{
			if ( *pStr == pwszStr )
			{
				return	(ssize_t) i ;
			}
		}
	}
	return	-1 ;
}


//////////////////////////////////////////////////////////////////////////////
// EntisGLS4 標準 XML 形式モデルファイル・セーバー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DStdXMLModelSaver, S3DModelSaverInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DStdXMLModelSaver::S3DStdXMLModelSaver( void )
	: m_strImageMIME( L"image/x-eri" ), m_strImageExt( L".eri" )
{
	m_optImage.nFlags = 0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DStdXMLModelSaver::~S3DStdXMLModelSaver( void )
{
}

// MIME 判定
//////////////////////////////////////////////////////////////////////////////
bool S3DStdXMLModelSaver::IsMatchableMIMEType( const wchar_t * pszMIME )
{
	return	(SString::CompareNoCase( pszMIME, L"application/x-xmlmdf" ) == 0) ;
}

// テクスチャ画像保存形式設定
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdXMLModelSaver::SetImageFormat
	( const wchar_t * pwszMIME, const wchar_t * pwszExt,
		const SGLImageEncoderInterface::Options * pOpt )
{
	m_strImageMIME = pwszMIME ;
	m_strImageExt = pwszExt ;
	m_optImage.nFlags = 0 ;
	if ( !m_strImageExt.IsEmpty()
		&& (m_strImageExt.GetAt(0) != L'.') )
	{
		m_strImageExt = L"." ;
		m_strImageExt += pwszExt ;
	}
	if ( pOpt != nullptr )
	{
		m_optImage = *pOpt ;
	}
	return	sglErrSuccess ;
}

// モデルデータ読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdXMLModelSaver::WriteModel
	( SSystem::SFileInterface & file, S3DModelBuffer & model )
{
	SXMLDocument	xmlDoc ;
	xmlDoc.SetTag( L"model" ) ;
	//
	// テクスチャ
	//
	SXMLDocument *	pxmlTextures = xmlDoc.CreateElementTagAs( L"textures" ) ;
	SGLError	err = FormatTextureTag( *pxmlTextures, model, file ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// 表面属性
	//
	SXMLDocument *	pxmlMaterials = xmlDoc.CreateElementTagAs( L"materials" ) ;
	FormatMaterialTag( *pxmlMaterials, model ) ;
	//
	// メッシュ
	//
	SXMLDocument *	pxmlMeshs = xmlDoc.CreateElementTagAs( L"meshs" ) ;
	FormatMeshTag( *pxmlMeshs, model ) ;
	//
	// メッシュグループ
	//
	SXMLDocument *	pxmlMeshGroup = xmlDoc.CreateElementTagAs( L"mesh_group" ) ;
	FormatMeshGroupTag( *pxmlMeshGroup, model ) ;
	//
	// モーフターゲット
	//
	SXMLDocument *	pxmlMorphMesh = xmlDoc.CreateElementTagAs( L"morph_meshs" ) ;
	FormatMeshMorphTargetTag( *pxmlMorphMesh, model ) ;
	//
	// 分割メッシュ情報
	//
	SXMLDocument *	pxmlMeshDiv = xmlDoc.CreateElementTagAs( L"mesh_div" ) ;
	FormatMeshDivisionTag( *pxmlMeshDiv, model ) ;
	//
	// ボーン
	//
	SXMLDocument *	pxmlBones = xmlDoc.CreateElementTagAs( L"bones" ) ;
	FormatBoneTag( *pxmlBones, model ) ;
	//
	// マーカー情報書き出し
	//
	SXMLDocument *	pxmlMarkers = xmlDoc.CreateElementTagAs( L"markers" ) ;
	FormatMarkerInfoTag( *pxmlMarkers, model ) ;
	//
	// クロスシミュレーターメッシュ情報書き出し
	//
//	SXMLDocument *	pxmlCloth = xmlDoc.CreateElementTagAs( L"cloth_entries" ) ;
//	FormatClothMeshInfoTag( *pxmlCloth, model ) ;
	//
	// ポーズ
	//
	SXMLDocument *	pxmlPoses = xmlDoc.CreateElementTagAs( L"poses" ) ;
	FormatPosesTag( *pxmlPoses, model ) ;
	//
	// メタ情報
	//
	const SXMLDocument &	xmlSrcMetaInfo = model.GetMetaInfo() ;
	const SXMLDocument *	pxmlSrcMetaInfo = nullptr ;
	if ( xmlSrcMetaInfo.GetTag() == L"meta_info" )
	{
		pxmlSrcMetaInfo = &xmlSrcMetaInfo ;
	}
	else if ( xmlSrcMetaInfo.GetType() == SXMLDocument::typeRoot )
	{
		pxmlSrcMetaInfo = xmlSrcMetaInfo.GetElementTagAs( L"meta_info" ) ;
	}
	if ( pxmlSrcMetaInfo != nullptr )
	{
		xmlDoc.AddElement( new SXMLDocument( *pxmlSrcMetaInfo ) ) ;
	}
	//
	// コンポジション
	//
	const SXMLDocument &	xmlSrcScene = model.GetSceneComposition() ;
	const SXMLDocument *	pxmlSrcScene = nullptr ;
	if ( xmlSrcScene.GetTag() == L"scene" )
	{
		pxmlSrcScene = &xmlSrcScene ;
	}
	else if ( xmlSrcScene.GetType() == SXMLDocument::typeRoot )
	{
		pxmlSrcScene = xmlSrcScene.GetElementTagAs( L"scene" ) ;
	}
	if ( pxmlSrcScene != nullptr )
	{
		xmlDoc.AddElement( new SXMLDocument( *pxmlSrcScene ) ) ;
	}
	//
	// 拡張
	//
	SXMLDocument *	pxmlExtensions = xmlDoc.CreateElementTagAs( L"extensions" ) ;
	FormatUserTags( *pxmlExtensions, model ) ;
	//
	// ファイルへ書き出し
	//
	xmlDoc.WriteDocument( file ) ;
	//
	return	sglErrSuccess ;
}

// テクスチャ書き出し
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdXMLModelSaver::FormatTextureTag
	( SSystem::SXMLDocument & xmlTexture,
		S3DModelBuffer & model, SSystem::SFileOpener & opener )
{
	S3DTextureLibrary&	textures = model.GetTextureLibrary() ;
	//
	size_t	nCount = textures.GetTextureCount() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		const wchar_t *		pwszID = textures.GetTextureIdentityAt( i ) ;
		SGLImageObject *	pImage = textures.GetTextureAt( i ) ;
		ESLAssert( pwszID != nullptr ) ;
		ESLAssert( pImage != nullptr ) ;
		if ( pwszID && pImage )
		{
			SString	strPath = pwszID ;
			strPath.MakeLower() ;
			strPath += m_strImageExt ;
			//
			if ( opener.IsExisting( strPath ) )
			{
				for ( int j = 1; j < 100000; j ++ )
				{
					strPath = pwszID ;
					strPath.MakeLower() ;
					strPath += SString( j ) ;
					strPath += m_strImageExt ;
					if ( !opener.IsExisting( strPath ) )
					{
						break ;
					}
				}
			}
			SSmartPointer<SFileInterface>
				pFile = opener.NewOpenFile
							( strPath, SFileOpener::modeCreate ) ;
			if ( pFile == nullptr )
			{
				return	sglErrFailed ;
			}
			SGLImageEncoderInterface::Options *	pOpt = nullptr ;
			if ( m_optImage.nFlags != 0 )
			{
				pOpt = &m_optImage ;
			}
			pImage->WriteImage( pFile, m_strImageMIME, pOpt ) ;
			//
			SXMLDocument *	pxmlTag = new SXMLDocument ;
			pxmlTag->SetTag( L"image" ) ;
			pxmlTag->SetAttributeAs( L"id", pwszID ) ;
			pxmlTag->SetAttributeAs( L"src", strPath ) ;
			pxmlTag->SetAttributeAs( L"mime", m_strImageMIME ) ;
			xmlTexture.AddElement( pxmlTag ) ;
		}
	}
	return	sglErrSuccess ;
}

// 表面属性書き出し
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdXMLModelSaver::FormatMaterialTag
	( SSystem::SXMLDocument & xmlMaterial, S3DModelBuffer & model )
{
	S3DTextureLibrary&	textures = model.GetTextureLibrary() ;
	S3DMaterialLibrary&	materials = model.GetMaterialLibrary() ;
	return	materials.FormatXML( xmlMaterial, textures ) ;
}

// メッシュ書き出し
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdXMLModelSaver::FormatMeshTag
	( SSystem::SXMLDocument & xmlMesh, S3DModelBuffer & model )
{
	S3DMaterialLibrary&	materials = model.GetMaterialLibrary() ;
	//
	for ( size_t iMesh = 0; iMesh < model.GetMeshCount(); iMesh ++ )
	{
		S3DModelBuffer::MeshObject *	pMesh = model.GetMeshObjectAt( iMesh ) ;
		ESLAssert( pMesh != nullptr ) ;
		//
		SXMLDocument *	pxmlTag = new SXMLDocument ;
		if ( pMesh->m_typeMesh == primitiveTriangle )
		{
			pxmlTag->SetTag( L"indexed_triangle_list" ) ;
		}
		else if ( pMesh->m_typeMesh == primitiveTriangleStrip )
		{
			pxmlTag->SetTag( L"triangle_strip" ) ;
		}
		else
		{
			pxmlTag->SetTag( L"primitive" ) ;
		}
		xmlMesh.AddElement( pxmlTag ) ;
		//
		ssize_t	iMaterial =
					materials.FindMaterialPtr( pMesh->m_pMaterial ) ;
		if ( iMaterial >= 0 )
		{
			pxmlTag->SetAttributeAs
				( L"material", materials.GetMaterialIdentityAt( (size_t) iMaterial ) ) ;
		}
		const size_t	countPolygon = pMesh->m_countPolygon ;
		const size_t	countVertex = pMesh->m_countVertex ;
		const size_t	countExAttr = pMesh->m_nExAttrElements ;
		pxmlTag->SetAttrIntegerAs( L"polygons", countPolygon ) ;
		pxmlTag->SetAttrIntegerAs( L"vertics", countVertex ) ;
		pxmlTag->SetAttrIntegerAs( L"ex_attr_count", countExAttr ) ;
		pxmlTag->SetAttrRealAs
			( L"sub_mesh_density", pMesh->m_fpSubMeshDensity ) ;
		//
		SXMLDocument::AttrInteger	aiPrimitiveTypes[] =
		{
			{ L"point", primitivePoint },
			{ L"line", primitiveLine },
			{ L"triangle", primitiveTriangle },
			{ nullptr, 0 },
		} ;
		pxmlTag->SetAttrSymbolizedIntegerAs
			( L"primitive_type", aiPrimitiveTypes, pMesh->m_typeMesh ) ;
		//
		// 頂点配列
		//
		SArray<float32_t>	bufNumber ;
		float32_t *	pBufNum = bufNumber.GetArray( countVertex * 3 ) ;
		size_t		i, j ;
		//
		const S3DVector4 *	pvVertex = model.GetVertexBufferAt( pMesh->m_iVertex ) ;
		for ( i = 0, j = 0; i < countVertex; i ++, j += 3 )
		{
			pBufNum[j]     = pvVertex[i].x ;
			pBufNum[j + 1] = pvVertex[i].y ;
			pBufNum[j + 2] = pvVertex[i].z ;
		}
		SString	strVertexList ;
		FormatFloatList( strVertexList, pBufNum, countVertex * 3, 3 ) ;
		pxmlTag->CreateElementTagAs( L"vertex" )->SetTextElement( strVertexList ) ;
		//
		// 法線配列
		//
		const S3DVector4 *	pvNormal = model.GetNormalBufferAt( pMesh->m_iNormal ) ;
		for ( i = 0, j = 0; i < countVertex; i ++, j += 3 )
		{
			pBufNum[j]     = pvNormal[i].x ;
			pBufNum[j + 1] = pvNormal[i].y ;
			pBufNum[j + 2] = pvNormal[i].z ;
		}
		SString	strNormalList ;
		FormatFloatList( strNormalList, pBufNum, countVertex * 3, 3 ) ;
		pxmlTag->CreateElementTagAs( L"normal" )->SetTextElement( strNormalList ) ;
		//
		// UV マップ
		//
		if ( pMesh->m_bufUVMap.GetLength() >= countVertex )
		{
			const S2DVector *	pvUVMap = pMesh->m_bufUVMap.GetConstArray() ;
			for ( i = 0, j = 0; i < countVertex; i ++, j += 2 )
			{
				pBufNum[j]     = pvUVMap[i].x ;
				pBufNum[j + 1] = pvUVMap[i].y ;
			}
			SString	strUVList ;
			FormatFloatList( strUVList, pBufNum, countVertex * 2, 4 ) ;
			pxmlTag->CreateElementTagAs( L"uv_map" )->SetTextElement( strUVList ) ;
		}
		bufNumber.FinishArray() ;
		//
		// 頂点色
		//
		if ( pMesh->m_bufColor.GetLength() >= countVertex )
		{
			SString				strColorList ;
			const S3DColor *	pColor = pMesh->m_bufColor.GetConstArray() ;
			for ( i = 0; i < countVertex; i ++ )
			{
				strColorList += SString( pColor[i].rgbMul.ui32, 6, 16 ) ;
				strColorList += L" " ;
				strColorList += SString( pColor[i].rgbAdd.ui32, 6, 16 ) ;
				if ( ((i + 1) % 4) == 0 )
				{
					strColorList += L"\n" ;
				}
				else
				{
					strColorList += L" " ;
				}
			}
			pxmlTag->CreateElementTagAs( L"color_map" )->SetTextElement( strColorList ) ;
		}
		//
		// 拡張属性
		//
		if ( (countExAttr > 0)
			&& (pMesh->m_bufExAttrElements.GetLength() >= countExAttr * countVertex) )
		{
			const size_t	nElCount = countExAttr * countVertex ;
			SString			strExAttrList ;
			FormatFloatList
				( strExAttrList,
					pMesh->m_bufExAttrElements.GetConstArray(), nElCount, countExAttr ) ;
			pxmlTag->CreateElementTagAs( L"ex_attr_elements" )->
											SetTextElement( strExAttrList ) ;
		}
		//
		// インデックス
		//
		if ( pMesh->m_bufIndex.GetLength() >= countPolygon * 3 )
		{
			SString				strIndexList ;
			size_t				nIndexCount = countPolygon * 3 ;
			const uint32_t *	pIndexedList = pMesh->m_bufIndex.GetConstArray() ;
			for ( i = 0; i < nIndexCount; i ++ )
			{
				strIndexList += SString( pIndexedList[i] ) ;
				if ( ((i + 1) % 9) == 0 )
				{
					strIndexList += L"\n" ;
				}
				else
				{
					strIndexList += L" " ;
				}
			}
			pxmlTag->CreateElementTagAs( L"indexed_list" )->SetTextElement( strIndexList ) ;
		}
		//
		for ( size_t iSub = 0; iSub < VertexBuffer::countSubMesh; iSub ++ )
		{
			if ( (pMesh->m_countSubPoly[iSub] == 0)
				|| (pMesh->m_bufSubIndex[iSub].GetLength() == 0) )
			{
				continue ;
			}
			SString				strIndexList ;
			size_t				nIndexCount = pMesh->m_countSubPoly[iSub] * 3 ;
			const uint32_t *	pIndexedList = pMesh->m_bufSubIndex[iSub].GetConstArray() ;
			for ( i = 0; i < nIndexCount; i ++ )
			{
				strIndexList += SString( pIndexedList[i] ) ;
				if ( ((i + 1) % 9) == 0 )
				{
					strIndexList += L"\n" ;
				}
				else
				{
					strIndexList += L" " ;
				}
			}
			SString	strTag = L"indexed_list" ;
			strTag += SString( iSub + 1 ) ;
			pxmlTag->CreateElementTagAs( strTag )->SetTextElement( strIndexList ) ;
		}
	}
	return	sglErrSuccess ;
}

// メッシュグループ情報書き出し
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdXMLModelSaver::FormatMeshGroupTag
	( SSystem::SXMLDocument & xmlGroup, S3DModelBuffer & model )
{
	SStrSortArray<S3DModelBuffer::MeshGroup>&
					ssaGroups = model.GetMeshGroupList() ;
	for ( size_t i = 0; i < ssaGroups.GetLength(); i ++ )
	{
		const SString *				pstrID = ssaGroups.GetTagAt( i ) ;
		S3DModelBuffer::MeshGroup *	pGroup = ssaGroups.GetAt( i ) ;
		ESLAssert( pstrID != nullptr ) ;
		ESLAssert( pGroup != nullptr ) ;
		//
		SXMLDocument *	pxmlTag = new SXMLDocument ;
		pxmlTag->SetTag( L"group_item" ) ;
		xmlGroup.AddElement( pxmlTag ) ;
		//
		pxmlTag->SetAttributeAs( L"id", *pstrID ) ;
		pxmlTag->SetAttrIntegerAs( L"first_mesh", pGroup->m_iFirstMesh ) ;
		pxmlTag->SetAttrIntegerAs( L"mesh_count", pGroup->m_nMeshCount ) ;
		pxmlTag->SetAttrRealAs( L"center_x", pGroup->m_vCenter.x ) ;
		pxmlTag->SetAttrRealAs( L"center_y", pGroup->m_vCenter.y ) ;
		pxmlTag->SetAttrRealAs( L"center_z", pGroup->m_vCenter.z ) ;
	}
	return	sglErrSuccess ;
}

// モーフターゲット書き出し
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdXMLModelSaver::FormatMeshMorphTargetTag
	( SSystem::SXMLDocument & xmlMorph, S3DModelBuffer & model )
{
	SStrSortObjectArray<S3DModelBuffer::MorphTargetMesh>&
							ssoaMorph = model.GetMorphTargetList() ;
	for ( size_t iMesh = 0; iMesh < ssoaMorph.GetLength(); iMesh ++ )
	{
		const SString *	pstrID = ssoaMorph.GetTagAt( iMesh ) ;
		S3DModelBuffer::MorphTargetMesh *
						pmtm = ssoaMorph.GetAt( iMesh ) ;
		ESLAssert( pstrID != nullptr ) ;
		ESLAssert( pmtm != nullptr ) ;
		//
		SXMLDocument *	pxmlTag = new SXMLDocument ;
		pxmlTag->SetTag( L"mesh" ) ;
		xmlMorph.AddElement( pxmlTag ) ;
		//
		pxmlTag->SetAttributeAs( L"id", *pstrID ) ;
		pxmlTag->SetAttrIntegerAs( L"vertics", pmtm->m_countVertex ) ;
		//
		// 頂点配列
		//
		SArray<float32_t>	bufNumber ;
		const size_t		countVertex = pmtm->m_countVertex ;
		float32_t *	pBufNum = bufNumber.GetArray( countVertex * 3 ) ;
		size_t		i, j ;
		//
		if ( pmtm->m_bufVertex.GetLength() >= countVertex )
		{
			const S3DVector4 *	pvVertex = pmtm->m_bufVertex.GetConstArray() ;
			for ( i = 0, j = 0; i < countVertex; i ++, j += 3 )
			{
				pBufNum[j]     = pvVertex[i].x ;
				pBufNum[j + 1] = pvVertex[i].y ;
				pBufNum[j + 2] = pvVertex[i].z ;
			}
			SString	strVertexList ;
			FormatFloatList( strVertexList, pBufNum, countVertex * 3, 3 ) ;
			pxmlTag->CreateElementTagAs( L"vertex" )->SetTextElement( strVertexList ) ;
		}
		//
		// 法線配列
		//
		if ( pmtm->m_bufNormal.GetLength() >= countVertex )
		{
			const S3DVector4 *	pvNormal = pmtm->m_bufNormal.GetConstArray() ;
			for ( i = 0, j = 0; i < countVertex; i ++, j += 3 )
			{
				pBufNum[j]     = pvNormal[i].x ;
				pBufNum[j + 1] = pvNormal[i].y ;
				pBufNum[j + 2] = pvNormal[i].z ;
			}
			SString	strNormalList ;
			FormatFloatList( strNormalList, pBufNum, countVertex * 3, 3 ) ;
			pxmlTag->CreateElementTagAs( L"normal" )->SetTextElement( strNormalList ) ;
		}
		//
		// UV マップ
		//
		if ( pmtm->m_bufUVMap.GetLength() >= countVertex )
		{
			const S2DVector *	pvUVMap = pmtm->m_bufUVMap.GetConstArray() ;
			for ( i = 0, j = 0; i < countVertex; i ++, j += 2 )
			{
				pBufNum[j]     = pvUVMap[i].x ;
				pBufNum[j + 1] = pvUVMap[i].y ;
			}
			SString	strUVList ;
			FormatFloatList( strUVList, pBufNum, countVertex * 2, 4 ) ;
			pxmlTag->CreateElementTagAs( L"uv_map" )->SetTextElement( strUVList ) ;
		}
		bufNumber.FinishArray() ;
		//
		// 頂点色
		//
		if ( pmtm->m_bufColor.GetLength() >= countVertex )
		{
			SString				strColorList ;
			const S3DColor *	pColor = pmtm->m_bufColor.GetConstArray() ;
			for ( i = 0; i < countVertex; i ++ )
			{
				strColorList += SString( pColor[i].rgbMul.ui32, 6, 16 ) ;
				strColorList += L" " ;
				strColorList += SString( pColor[i].rgbAdd.ui32, 6, 16 ) ;
				if ( ((i + 1) % 4) == 0 )
				{
					strColorList += L"\n" ;
				}
				else
				{
					strColorList += L" " ;
				}
			}
			pxmlTag->CreateElementTagAs( L"color_map" )->SetTextElement( strColorList ) ;
		}
		//
		// ウェイト
		//
		if ( pmtm->m_bufWeight.GetLength() >= countVertex )
		{
			SString	strWeightList ;
			FormatFloatList
				( strWeightList,
					pmtm->m_bufWeight.GetConstArray(), countVertex, 4 ) ;
			pxmlTag->CreateElementTagAs( L"weight_map" )->SetTextElement( strWeightList ) ;
		}
		//
		// 関連
		//
		SString	strRelMesh ;
		size_t	nMeshs = model.GetMeshCount() ;
		for ( size_t j = 0; j < nMeshs; j ++ )
		{
			S3DModelBuffer::MeshObject *
						pMesh = model.GetMeshObjectAt( j ) ;
			if ( pMesh == nullptr )
			{
				continue ;
			}
			for ( size_t k = 0; k < pMesh->m_arrMorphTarget.GetLength(); k ++ )
			{
				SString *	pstrMeshID = pMesh->m_arrMorphTarget.GetAt( k ) ;
				ESLAssert( pstrMeshID != nullptr ) ;
				if ( (pstrMeshID != nullptr) && (*pstrMeshID == *pstrID) )
				{
					strRelMesh += SString( j ) ;
					strRelMesh += L" " ;
					break ;
				}
			}
		}
		pxmlTag->CreateElementTagAs( L"rel_mesh" )->SetTextElement( strRelMesh ) ;
	}
	return	sglErrSuccess ;
}

// 分割メッシュ情報書き出し
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdXMLModelSaver::FormatMeshDivisionTag
	( SSystem::SXMLDocument & xmlMorph, S3DModelBuffer & model )
{
	SStrSortObjectArray<S3DModelData::MeshDivision>&
						ssoaMeshDiv = model.GetMeshDivisionList() ;
	for ( size_t i = 0; i < ssoaMeshDiv.GetLength(); i ++ )
	{
		const SString *	pstrTag = ssoaMeshDiv.GetTagAt( i ) ;
		S3DModelData::MeshDivision *
						pMeshDiv = ssoaMeshDiv.GetAt( i ) ;
		ESLAssert( pstrTag != nullptr ) ;
		ESLAssert( pMeshDiv != nullptr ) ;
		if ( (pstrTag == nullptr) || (pMeshDiv == nullptr) )
		{
			continue ;
		}
		SXMLDocument *	pxmlMesh = new SXMLDocument ;
		pxmlMesh->SetTag( L"mesh" ) ;
		pxmlMesh->SetAttributeAs( L"id", *pstrTag ) ;
		xmlMorph.AddElement( pxmlMesh ) ;
		//
		SXMLDocument *	pxmlMorphEntries =
							pxmlMesh->CreateElementTagAs( L"morph_entries" ) ;
		for ( size_t j = 0; j < pMeshDiv->m_aMorphEntries.GetLength(); j ++ )
		{
			const SString *	pstrTargetID = pMeshDiv->m_aMorphEntries.GetAt( j ) ;
			if ( pstrTargetID != nullptr )
			{
				SXMLDocument *	pxmlEntry = new SXMLDocument ;
				pxmlEntry->SetTag( L"entry" ) ;
				pxmlEntry->SetAttributeAs( L"morph_target", *pstrTargetID ) ;
				pxmlMorphEntries->AddElement( pxmlEntry ) ;
			}
		}
		for ( size_t j = 0; j < pMeshDiv->m_aSplittedEntries.GetLength(); j ++ )
		{
			S3DModelData::MeshDivision::SplittedEntry *
					pSplitted = pMeshDiv->m_aSplittedEntries.GetAt( j ) ;
			ESLAssert( pSplitted != nullptr ) ;
			if ( pSplitted == nullptr )
			{
				continue ;
			}
			SXMLDocument *	pxmlSplitted = new SXMLDocument ;
			pxmlSplitted->SetTag( L"splitted_mesh" ) ;
			pxmlSplitted->SetAttributeAs( L"id", pSplitted->m_strSplittedMesh ) ;
			pxmlMesh->AddElement( pxmlSplitted ) ;
			//
			for ( size_t k = 0; k < pSplitted->m_aSplittedMorph.GetLength(); k ++ )
			{
				const SString *	pstrTargetID =
									pSplitted->m_aSplittedMorph.GetAt( k ) ;
				ESLAssert( pstrTargetID != nullptr ) ;
				if ( pstrTargetID != nullptr )
				{
					SXMLDocument *	pxmlEntry = new SXMLDocument ;
					pxmlEntry->SetTag( L"entry" ) ;
					pxmlEntry->SetAttributeAs( L"morph_target", *pstrTargetID ) ;
					pxmlSplitted->AddElement( pxmlEntry ) ;
				}
			}
		}
	}
	return	sglErrSuccess ;
}

// ボーン書き出し
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdXMLModelSaver::FormatBoneTag
	( SSystem::SXMLDocument & xmlBone, S3DModelBuffer & model, uint32_t nFlags )
{
	SStrSortObjectArray<S3DModelBoneSpace>&
								ssoaBone = model.GetBonePropertyList() ;
	for ( size_t i = 0; i < ssoaBone.GetLength(); i ++ )
	{
		const SString *		pstrID = ssoaBone.GetTagAt( i ) ;
		S3DModelBoneSpace *	pBone = ssoaBone.GetAt( i ) ;
		ESLAssert( pstrID != nullptr ) ;
		ESLAssert( pBone != nullptr ) ;
		//
		if ( nFlags & boneOnlyPhysics )
		{
			if ( !(pBone->GetBoneFlags()
						& S3DModelBoneSpace::flagBonePhysics) )
			{
				continue ;
			}
		}
		//
		SXMLDocument *	pxmlTag = new SXMLDocument ;
		pxmlTag->SetTag( L"bone" ) ;
		xmlBone.AddElement( pxmlTag ) ;
		//
		pxmlTag->SetAttributeAs( L"id", *pstrID ) ;
		//
		// ボーンパラメータ
		//
		pxmlTag->SetAttrRealAs( L"x", pBone->m_vCenter.x ) ;
		pxmlTag->SetAttrRealAs( L"y", pBone->m_vCenter.y ) ;
		pxmlTag->SetAttrRealAs( L"z", pBone->m_vCenter.z ) ;
		//
		S3DDVector	vHandle = pBone->GetBoneHandle() ;
		pxmlTag->SetAttrRealAs( L"handle_x", vHandle.x ) ;
		pxmlTag->SetAttrRealAs( L"handle_y", vHandle.y ) ;
		pxmlTag->SetAttrRealAs( L"handle_z", vHandle.z ) ;
		//
		SXMLDocument::AttrInteger	aiBoneFlags[] =
		{
			{ L"physics", S3DModelBoneSpace::flagBonePhysics },
			{ L"no_collision", S3DModelBoneSpace::flagNoCollision },
			{ L"track_pos", S3DModelBoneSpace::flagTrackingPos },
			{ nullptr, 0 },
		} ;
		pxmlTag->SetAttrComplexIntegerAs
			( L"flags", aiBoneFlags, pBone->GetBoneFlags() ) ;
		//
		if ( pBone->GetBoneFlags() & S3DModelBoneSpace::flagHaveOrgMatrix )
		{
			SString	strOrgMatrix ;
			FormatFloatList
				( strOrgMatrix,
					&(pBone->GetOriginalBoneMatrix().m[0][0]), 16, 16 ) ;
			SXMLDocument *	pxmlOrgMatrix =
						pxmlTag->CreateElementTagAs( L"original_matrix" ) ;
			pxmlOrgMatrix->AddTextElement( strOrgMatrix ) ;
		}
		//
		SXMLDocument::AttrInteger	aiIKFlags[] =
		{
			{ L"min_bent", S3DModelBoneSpace::flagIKMinBent },
			{ L"max_bent", S3DModelBoneSpace::flagIKMaxBent },
			{ L"bend_direction", S3DModelBoneSpace::flagIKBendDirection },
			{ L"parent_axis", S3DModelBoneSpace::flagIKParentAxis },
			{ L"ik_terminate", S3DModelBoneSpace::flagIKTerminate },
			{ nullptr, 0 },
		} ;
		SXMLDocument *	pxmlIKParam =
							pxmlTag->CreateElementTagAs( L"ik_parameter" ) ;
		const S3DModelBoneSpace::IKParameter&
							paramIK = pBone->GetIKParameter() ;
		pxmlIKParam->SetAttrComplexIntegerAs
					( L"flags", aiIKFlags, paramIK.nIKFlags ) ;
		pxmlIKParam->SetAttrRealAs( L"weight", paramIK.fpWeight ) ;
		pxmlIKParam->SetAttrRealAs( L"min_bent", paramIK.degMinBent ) ;
		pxmlIKParam->SetAttrRealAs( L"max_bent", paramIK.degMaxBent ) ;
		pxmlIKParam->SetAttrRealAs( L"bend_dir_x", paramIK.vBendDirection.x ) ;
		pxmlIKParam->SetAttrRealAs( L"bend_dir_y", paramIK.vBendDirection.y ) ;
		pxmlIKParam->SetAttrRealAs( L"bend_dir_z", paramIK.vBendDirection.z ) ;
		//
		SXMLDocument *	pxmlMaterial =
							pxmlTag->CreateElementTagAs( L"phys_material" ) ;
		S3DModelBoneSpace::PhysMaterial
					physMaterial = pBone->GetBonePhysicalMaterial() ;
		FormatBonePhysMaterial( *pxmlMaterial, physMaterial ) ;
		pxmlMaterial->SetAttributeAs
				( L"ref_id", pBone->GetBonePhysicalMaterialID() ) ;
		//
		// 親子関係
		//
		S3DModelBoneSpace *	pParent =
			ESLTypeCast<S3DModelBoneSpace>( pBone->GetParentSpace() ) ;
		if ( pParent != nullptr )
		{
			ssize_t	iParent = ssoaBone.FindPtr( pParent ) ;
			if ( iParent >= 0 )
			{
				const SString *	pstrParentID =
							ssoaBone.GetTagAt( (size_t) iParent ) ;
				if ( pstrParentID != nullptr )
				{
					pxmlTag->SetAttributeAs( L"parent", *pstrParentID ) ;
				}
			}
		}
		//
		// ウェイトマップ
		//
		if ( !(nFlags & boneOnlyPhysics) )
		{
			const size_t		iWeightVertex =
										pBone->VertexIndexOfBoneWeight() ;
			const size_t		iWeightNormal =
										pBone->NormalIndexOfBoneWeight() ;
			const size_t		nWeightMapLength =
										pBone->VertexCountOfBoneWeight() ;
//			const float32_t *	pWeightMap = pBone->GetBoneWeightMap() ;
			//
			size_t			nRefMeshs ;
			const S3DModelBoneSpace::REF_MESH_INFO *
							pRefMeshInfo =
								pBone->GetEffectiveMeshIndexArray( nRefMeshs ) ;
			//
			SXMLDocument *	pxmlWeightMaps =
								pxmlTag->CreateElementTagAs( L"weight_maps" ) ;
			for ( size_t j = 0; j < nRefMeshs; j ++ )
			{
				S3DModelBuffer::MeshObject *
					pMesh = model.GetMeshObjectAt( pRefMeshInfo[j].iMesh ) ;
				if ( pMesh == nullptr )
				{
					continue ;
				}
				if ( ((size_t) pMesh->m_iVertex < iWeightVertex)
					|| ((size_t) pMesh->m_iNormal < iWeightNormal)
					|| ((size_t) pMesh->m_iVertex + pMesh->m_countVertex
										> iWeightVertex + nWeightMapLength)
					|| ((size_t) pMesh->m_iNormal + pMesh->m_countVertex
										> iWeightNormal + nWeightMapLength) )
				{
					continue ;
				}
				SXMLDocument *	pxmlMap = new SXMLDocument ;
				pxmlMap->SetTag( L"weight_map" ) ;
				//
				size_t	countVertex = pMesh->m_countVertex ;
				pxmlMap->SetAttrIntegerAs( L"target_mesh", pRefMeshInfo[j].iMesh ) ;
				pxmlMap->SetAttrIntegerAs( L"count", countVertex ) ;
				//
				SString	strIMeshMatrix ;
				FormatFloatList
					( strIMeshMatrix,
						&(pRefMeshInfo[j].matIMesh.m[0][0]), 16, 16 ) ;
				pxmlMap->SetAttributeAs( L"inv_mesh_matrix", strIMeshMatrix ) ;
				//
				SString	strRelMeshMatrix ;
				FormatFloatList
					( strRelMeshMatrix,
						&(pRefMeshInfo[j].matRelMesh.m[0][0]), 16, 16 ) ;
				pxmlMap->SetAttributeAs( L"rel_mesh_matrix", strRelMeshMatrix ) ;
				//
				bool	fRefMesh = false ;
//				const float32_t *
//						pMeshWeight =
//							pWeightMap + (pMesh->m_iVertex - iWeightVertex) ;
				size_t	iLockFirst = pMesh->m_iVertex ;
				size_t	nLockCount = countVertex ;
				const float32_t *	pMeshWeight =
					pBone->LockBoneWeightMap( iLockFirst, nLockCount ) ;
				ESLAssert( iLockFirst == pMesh->m_iVertex ) ;
				ESLAssert( nLockCount >= countVertex ) ;
				for ( size_t k = 0; k < countVertex; k ++ )
				{
					if ( pMeshWeight[k] != 0.0f )
					{
						fRefMesh = true ;
						break ;
					}
				}
				if ( fRefMesh )
				{
					pxmlWeightMaps->AddElement( pxmlMap ) ;
					//
					SString	strList ;
					FormatFloatList( strList, pMeshWeight, countVertex, 5 ) ;
					pxmlMap->AddTextElement( strList ) ;
				}
				else
				{
					delete	pxmlMap ;
				}
				pBone->UnlockBoneWeightMap( false ) ;
			}
		}
		//
		// 物理演算・被影響ボーン
		//
		size_t	nEffPhysBones = pBone->GetEffectivePhysBoneCount() ;
		if ( nEffPhysBones > 0 )
		{
			SXMLDocument *	pxmlPhysBones =
				pxmlTag->CreateElementTagAs( L"phys_effecive_bones" ) ;
			for ( size_t j = 0; j < nEffPhysBones; j ++ )
			{
				S3DModelBoneSpace::EffectiveBoneEntry *
					pebe = pBone->GettEffectivePhysBoneAt( j ) ;
				if ( pebe != nullptr )
				{
					SXMLDocument *	pxmlEffBone = new SXMLDocument ;
					pxmlEffBone->SetTag( L"effecive_bone" ) ;
					pxmlPhysBones->AddElement( pxmlEffBone ) ;
					//
					pxmlEffBone->SetAttributeAs( L"bone", pebe->m_strBoneID ) ;
					pxmlEffBone->SetAttrRealAs( L"weight", pebe->m_fpWeight ) ;
				}
			}
		}
	}
	SStrSortArray<S3DModelBoneSpace::PhysMaterial>&
					physPalette = model.GetPhysMaterialList() ;
	SXMLDocument *	pxmlMatPalette =
					xmlBone.CreateElementTagAs( L"phys_material_palette" ) ;
	for ( size_t i = 0; i < physPalette.GetLength(); i ++ )
	{
		const SString *	pstrID = physPalette.GetTagAt( i ) ;
		S3DModelBoneSpace::PhysMaterial *
						pMaterial = physPalette.GetAt( i ) ;
		ESLAssert( pMaterial != nullptr ) ;
		if ( (pstrID == nullptr) || (pMaterial == nullptr) )
		{
			continue ;
		}
		SXMLDocument *	pxmlMaterial = new SXMLDocument ;
		pxmlMaterial->SetTag( L"phys_material" ) ;
		pxmlMatPalette->AddElement( pxmlMaterial ) ;
		//
		pxmlMaterial->SetAttributeAs( L"id", *pstrID ) ;
		FormatBonePhysMaterial( *pxmlMaterial, *pMaterial ) ;
	}
	return	sglErrSuccess ;
}

void S3DStdXMLModelSaver::FormatBonePhysMaterial
	( SXMLDocument& xmlMaterial,
		const S3DModelBoneSpace::PhysMaterial& physMaterial )
{
	physMaterial.FormatXML( xmlMaterial ) ;
}

// マーカー情報書き出し
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdXMLModelSaver::FormatMarkerInfoTag
	( SSystem::SXMLDocument & xmlMarker, S3DModelBuffer & model )
{
	return	model.FormatMarkerXML( xmlMarker ) ;
}

// クロスシミュレーターメッシュ情報書き出し
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdXMLModelSaver::FormatClothMeshInfoTag
	( SSystem::SXMLDocument & xmlMarker, S3DModelBuffer & model )
{
//	return	model.FormatClothMeshXML( xmlMarker ) ;
	return	sglErrSuccess ;
}

// ポーズ書き出し
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdXMLModelSaver::FormatPosesTag
	( SSystem::SXMLDocument & xmlPose, S3DModelBuffer & model )
{
	return	model.GetPoseLibrary().FormatLibraryXML( xmlPose ) ;
}

// 拡張データ書き出し
//////////////////////////////////////////////////////////////////////////////
SGLError S3DStdXMLModelSaver::FormatUserTags
	( SSystem::SXMLDocument & xmlTag, S3DModelBuffer & model )
{
	return	sglErrSuccess ;
}

// 数値列文字列化
//////////////////////////////////////////////////////////////////////////////
void S3DStdXMLModelSaver::FormatFloatList
	( SSystem::SString& strList,
		const float32_t * pFloatList, size_t nCount, size_t nCountInLine )
{
	for ( size_t i = 0, j = 0; i < nCount; i ++ )
	{
		//
		// 正規化
		//
		double	n = pFloatList[i] ;
		double	s = 1.0 ;
		int		e = 0 ;
		if ( n < 0 )
		{
			s = -1.0 ;
			n = - n ;
		}
		if ( n >= 10 )
		{
			double	v = 10 ;
			e = 1 ;
			while ( n >= v * 10 )
			{
				v *= 10 ;
				if ( ++ e >= 99 )
				{
					break ;
				}
			}
			n /= v ;
		}
		else if ( (n > 0) && (n < 1) )
		{
			double	v = 1 ;
			e = 0 ;
			while ( n * v < 1 )
			{
				v *= 10 ;
				if ( -- e <= -99 )
				{
					break ;
				}
			}
			n *= v ;
		}
		//
		// 文字列化
		//
		if ( j > 0 )
		{
			strList += L" " ;
		}
		SString	strNum ;
		strNum.FromReal( n * s, 8 ) ;
		strList += strNum ;
		if ( e > 0 )
		{
			strList += L"e+" ;
			strList += SString( e ) ;
		}
		else if ( e < 0 )
		{
			strList += L"e-" ;
			strList += SString( -e ) ;
		}
		if ( ++ j >= nCountInLine )
		{
			strList += L"\n" ;
			j = 0 ;
		}
	}
}

