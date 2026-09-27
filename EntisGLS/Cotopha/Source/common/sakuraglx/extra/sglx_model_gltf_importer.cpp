

#include <sakuraglx/sakuraglx.h>
#include <sakuraglx/render/sglx_model_loader.h>
#include <sakuragl/sgl2d/sgl_image_conversion.h>

#define	TINYGLTF_IMPLEMENTATION
#define	STB_IMAGE_IMPLEMENTATION
#define	STB_IMAGE_WRITE_IMPLEMENTATION

#include <sakuraglx/extra/sglx_model_gltf_importer.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// glTF インポーター
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DModelGLTFImporter, S3DModelLoaderInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DModelGLTFImporter::S3DModelGLTFImporter( void )
	: m_matCvtAxis( 1, -1, -1 ), m_mat4CvtAxis( 1, -1, -1, 1 )
{
	m_mat4ICvtAxis.InverseOf( m_mat4CvtAxis ) ;
	m_fpsFrameRatio = 60.0 ;
	//
	m_flagModelLoaded = false ;
	m_pDstModel = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DModelGLTFImporter::~S3DModelGLTFImporter( void )
{
}

// ファイル拡張子判定
//////////////////////////////////////////////////////////////////////////////
bool S3DModelGLTFImporter::IsMatchableFileExtension( const wchar_t * pszExt )
{
	return	((SString::CompareNoCase( pszExt, L"glb" ) == 0)
			|| (SString::CompareNoCase( pszExt, L"vrm" ) == 0)) ;
}

// MIME 判定
//////////////////////////////////////////////////////////////////////////////
bool S3DModelGLTFImporter::IsMatchableMIMEType( const wchar_t * pszMIME )
{
	return	(SString::CompareNoCase( pszMIME, L"modle/gltf+binary" ) == 0) ;
}

// モデルデータ読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelGLTFImporter::ReadModel
	( S3DModelBuffer & model, SSystem::SFileInterface & file )
{
	SByteBuffer	bufFile ;
	bufFile.ReadFromFile( file ) ;
	//
	SGLError	err =
		LoadGLTFBinaryOnMemory
			( bufFile.GetConstArray(), (unsigned int) bufFile.GetLength() ) ;
	if ( err )
	{
		return	err ;
	}
	return	ConvertTo( &model ) ;
}

// 読み込み処理 .gltf
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelGLTFImporter::LoadGLTFText( const wchar_t * pwszFilePath )
{
	tinygltf::TinyGLTF	gltfLoader ;
	std::string			strError ;
	std::string			strWarning ;
	//
	SArray<char>	bufFilePath ;
	std::string		strFilePath =
						SString(pwszFilePath).
							EncodeDefaultTo( bufFilePath ) ;
	m_flagModelLoaded =
		gltfLoader.LoadASCIIFromFile
			( &m_model, &strError, &strWarning, strFilePath ) ;
	//
	if ( !strError.empty() )
	{
		OutputError( SString( strError.c_str() ) ) ;
	}
	if ( !strWarning.empty() )
	{
		OutputWarning( SString( strWarning.c_str() ) ) ;
	}
	//
	return	m_flagModelLoaded ? sglErrSuccess : sglErrFailed ;
}

// 読み込み処理 .glb
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelGLTFImporter::LoadGLTFBinary( const wchar_t * pwszFilePath )
{
	tinygltf::TinyGLTF	gltfLoader ;
	std::string			strError ;
	std::string			strWarning ;
	//
	SArray<char>		bufFilePath ;
	std::string			strFilePath =
							SString(pwszFilePath).
								EncodeDefaultTo( bufFilePath ) ;
#if	!defined(__DISABLED_EXCEPTION__)
	try
#endif
	{
		m_flagModelLoaded =
			gltfLoader.LoadBinaryFromFile
				( &m_model, &strError, &strWarning, strFilePath ) ;
	}
#if	!defined(__DISABLED_EXCEPTION__)
	catch ( std::exception& e )
	{
		strError = e.what() ;
	}
#endif
	if ( !strError.empty() )
	{
		OutputError( SString( strError.c_str() ) ) ;
	}
	if ( !strWarning.empty() )
	{
		OutputWarning( SString( strWarning.c_str() ) ) ;
	}
	//
	return	m_flagModelLoaded ? sglErrSuccess : sglErrFailed ;
}

SGLError S3DModelGLTFImporter::LoadGLTFBinaryOnMemory
	( const unsigned char *bytes, const unsigned int length )
{
	tinygltf::TinyGLTF	gltfLoader ;
	std::string			strError ;
	std::string			strWarning ;
#if	!defined(__DISABLED_EXCEPTION__)
	try
#endif
	{
		m_flagModelLoaded =
			gltfLoader.LoadBinaryFromMemory
				( &m_model, &strError, &strWarning, bytes, length ) ;
	}
#if	!defined(__DISABLED_EXCEPTION__)
	catch ( std::exception& e )
	{
		strError = e.what() ;
	}
#endif
	if ( !strError.empty() )
	{
		OutputError( SString( strError.c_str() ) ) ;
	}
	if ( !strWarning.empty() )
	{
		OutputWarning( SString( strWarning.c_str() ) ) ;
	}
	//
	return	m_flagModelLoaded ? sglErrSuccess : sglErrFailed ;
}

// 変換
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelGLTFImporter::ConvertTo( S3DModelBuffer * pModel )
{
	if ( !m_flagModelLoaded )
	{
		return	sglErrFailed ;
	}
	m_pDstModel = pModel ;
	//
	// 画像
	//
	for ( size_t i = 0; i < m_model.images.size(); i ++ )
	{
		ConvertImage( i, m_model.images[i] ) ;
	}
	//
	// テクスチャ
	//
	for ( size_t i = 0; i < m_model.textures.size(); i ++ )
	{
		ConvertTexture( i, m_model.textures[i] ) ;
	}
	//
	// マテリアル
	//
	for ( size_t i = 0; i < m_model.materials.size(); i ++ )
	{
		ConvertMaterial( i, m_model.materials[i] ) ;
	}
	//
	// メッシュ
	//
	const int	iDefScene = (m_model.defaultScene < 0) ? 0 : m_model.defaultScene ;
	const tinygltf::Scene&	scene = m_model.scenes[iDefScene] ;
	for ( size_t i = 0; i < scene.nodes.size(); i ++ )
	{
		S4DDMatrix	matI( 1, 1, 1, 1 ) ;
		ConvertNode
			( scene.nodes[i],
				m_model.nodes[scene.nodes[i]],
				matI, m_pDstModel->GetBoneRoot() ) ;
	}
	//
	// ボーン関連付け構築
	//
	BuildAllBoneRelations() ;
	//
	// VRM 拡張
	//
	ConvertVRMExtensions() ;
	//
	// アニメーション
	//
	for ( size_t i = 0; i < m_model.animations.size(); i ++ )
	{
		ConvertAnimation( i, m_model.animations[i] ) ;
	}
	//
	// データ完成
	//
	m_pDstModel->Flush() ;
	//
	return	sglErrSuccess ;
}

// 画像変換
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelGLTFImporter::ConvertImage
		( size_t iImage, const tinygltf::Image& image )
{
	ImageEntry	imgEntry ;
	eslFillMemory( &imgEntry, 0, sizeof(ImageEntry) ) ;
	//
	S3DTextureLibrary&	libTexture = m_pDstModel->GetTextureLibrary() ;
	SString				strImageID = ConvertString( image.name ) ;
	if ( strImageID.IsEmpty() )
	{
		strImageID = L"texture" ;
	}
	libTexture.NormalizeIdentity( strImageID ) ;
	//
	uint32_t	format = formatImageARGB ;
	uint32_t	formatSrc = formatImageABGR ;
	uint32_t	depth = (uint32_t) image.component * 8 ;
	switch ( image.component )
	{
	case	1:
		format = formatImageGray ;
		formatSrc = formatImageGray ;
		break ;
	case	3:
		format = formatImageRGB ;
		formatSrc = formatImageBGR ;
		break ;
	case	4:
		break ;
	default:
		OutputError
			( strImageID + L"：未対応の画像形式のためインポートできません" ) ;
		return	sglErrFailed ;
	}
	SGLImageBuffer	imgbufSrc ;
	imgbufSrc.format = formatSrc ;
	imgbufSrc.depth = depth ;
	imgbufSrc.width = (uint32_t) image.width ;
	imgbufSrc.height = (uint32_t) image.height ;
	imgbufSrc.pitchPixel = depth / 8 ;
	imgbufSrc.pitchLine = imgbufSrc.width * imgbufSrc.pitchPixel ;
	imgbufSrc.ptrBuffer = (uint8_t*) &(image.image.at(0)) ;
	//
	SGLImage *	pImage = new SGLImage ;
	pImage->CreateImage
		( (uint32_t) image.width,
			(uint32_t) image.height, format, depth ) ;
	libTexture.AddSmartTextureAs( strImageID, pImage ) ;
	//
	SGLImageBuffer	imgbuf ;
	imgbuf.ptrBuffer = pImage->LockBuffer( imgbuf ) ;
	sglConvertImageBuffer( imgbuf, imgbufSrc ) ;
	pImage->UnlockBuffer() ;
	//
	if ( format & formatImageFlagAlpha )
	{
		ESLTrace( "texture %s\n", strImageID.ToCharArray().GetArray() ) ;
		pImage->MultiplyImageRGBAlpha() ;
	}
	//
	imgEntry.pDefault = pImage ;
	imgEntry.pwszID = libTexture.GetTextureIdentityAt
						( (size_t) libTexture.FindTexturePtr( pImage ) ) ;
	m_aImages.SetAt( iImage, imgEntry ) ;
	return	sglErrSuccess ;
}

// テクスチャ変換
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelGLTFImporter::ConvertTexture
		( size_t iTexture, const tinygltf::Texture& texture )
{
	ImageEntry *	pieSrc = m_aImages.GetAt( (size_t) texture.source ) ;
	if ( pieSrc == NULL )
	{
		OutputError
			( ConvertString(texture.name)
					+ L"：テクスチャソースが見つかりません" ) ;
		return	sglErrFailed ;
	}
	const tinygltf::Sampler&	sampler = m_model.samplers[texture.sampler] ;
	//
	TextureEntry	txtEntry ;
	eslFillMemory( &txtEntry, 0, sizeof(TextureEntry) ) ;
	txtEntry.pImageEntry = pieSrc ;
	//
	if ( (sampler.minFilter == TINYGLTF_TEXTURE_FILTER_LINEAR)
		|| (sampler.minFilter == TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST)
		|| (sampler.minFilter == TINYGLTF_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR) )
	{
		txtEntry.nShadingFlags |= shadingTextureSmoothing ;
	}
	if ( sampler.magFilter == TINYGLTF_TEXTURE_FILTER_LINEAR )
	{
		txtEntry.nShadingFlags |= shadingTextureSmoothing ;
	}
	if ( (sampler.wrapS != TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE)
		&& (sampler.wrapT != TINYGLTF_TEXTURE_WRAP_CLAMP_TO_EDGE) )
	{
		txtEntry.nShadingFlags |= shadingTextureTiling ;
	}
	//
	if ( pieSrc && pieSrc->pDefault )
	{
		txtEntry.sizeTexture = pieSrc->pDefault->GetImageSize() ;
	}
	else
	{
		txtEntry.sizeTexture.w = 1 ;
		txtEntry.sizeTexture.h = 1 ;
	}
	//
	m_aTextures.SetAt( iTexture, txtEntry ) ;
	return	sglErrSuccess ;
}

// マテリアル変換
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelGLTFImporter::ConvertMaterial
		( size_t iMaterial, const tinygltf::Material& material )
{
	S3DTextureLibrary&	libTexture = m_pDstModel->GetTextureLibrary() ;
	S3DMaterialLibrary&	libMaterial = m_pDstModel->GetMaterialLibrary() ;
	SString	strMaterialName = ConvertString( material.name ) ;
	if ( strMaterialName.IsEmpty() )
	{
		strMaterialName = L"material" ;
	}
	libMaterial.NormalizeIdentity( strMaterialName ) ;
	//
	S3DMaterial *	pMaterial = new S3DMaterial ;
	libMaterial.AddSmartMaterialAs( strMaterialName, pMaterial ) ;
	//
	S3DSurfaceAttribute	attr ;
	S2DVector			vUVScale( 1, 1 ) ;
	pMaterial->GetSurfaceAttribute( attr ) ;
	attr.flagsShading |= shadingMethodGouraud | shadingSingleSidePlane ;
	attr.nDiffusion = 0x100 ;
	//
	int	iNextTexture = 0 ;
	//
	// 基本色
	//
	const char *	pszAttrBaseColor = "baseColorFactor" ;
	const char *	pszAttrColorTexture = "baseColorTexture" ;
	tinygltf::ParameterMap::const_iterator
			itColorTexture = material.values.find( pszAttrColorTexture ) ;
	if ( itColorTexture != material.values.end() )
	{
		const tinygltf::Parameter&
					prmBaseTexture = itColorTexture->second ;
		int	iTexture = prmBaseTexture.TextureIndex() ;
		if ( iTexture >= 0 )
		{
			TextureEntry *	pte = m_aTextures.GetAt( (size_t) iTexture ) ;
			if ( pte && pte->pImageEntry )
			{
				attr.flagsShading |= shadingTextureMapping ;
				pMaterial->SetTexture
					( pte->pImageEntry->pDefault,
						iNextTexture ++,
						S3DMaterial::textureDiffusion,
						1.0f, 0.0f, pte->pImageEntry->pwszID ) ;
				//
				vUVScale.x = (float32_t) pte->sizeTexture.w ;
				vUVScale.y = (float32_t) pte->sizeTexture.h ;
			}
		}
	}
	tinygltf::ParameterMap::const_iterator
			itBaseColor = material.values.find( pszAttrBaseColor ) ;
	if ( itBaseColor != material.values.end() )
	{
		SGLPalette	argbColor =
			ConvertColorFactor( itBaseColor->second, 0xFFFFFFFF ) ;
		if ( argbColor.argb.Alpha < 0xFF )
		{
			if ( attr.flagsShading & shadingTextureMapping )
			{
				attr.colorBase.rgbMul = argbColor.ui32 & 0x00FFFFFF ;
				attr.colorBase.rgbAdd = 0 ;
				attr.nTransparency = 0x100 - argbColor.argb.Alpha ;
			}
			else
			{
				attr.colorBase.rgbMul.argb.Red = ~argbColor.argb.Alpha ;
				attr.colorBase.rgbMul.argb.Green = ~argbColor.argb.Alpha ;
				attr.colorBase.rgbMul.argb.Blue = ~argbColor.argb.Alpha ;
				attr.colorBase.rgbAdd = argbColor.ui32 & 0x00FFFFFF ;
			}
		}
		else
		{
			if ( attr.flagsShading & shadingTextureMapping )
			{
				attr.colorBase.rgbMul = argbColor.ui32 & 0x00FFFFFF ;
				attr.colorBase.rgbAdd = 0 ;
			}
			else
			{
				attr.colorBase.rgbMul.argb.Red = ~argbColor.argb.Alpha ;
				attr.colorBase.rgbMul.argb.Green = ~argbColor.argb.Alpha ;
				attr.colorBase.rgbMul.argb.Blue = ~argbColor.argb.Alpha ;
				attr.colorBase.rgbAdd = argbColor.ui32 & 0x00FFFFFF ;
			}
		}
	}
	//
	// 表面物性
	//
	tinygltf::ParameterMap::const_iterator
			itMetallic = material.values.find( "metallicFactor" ) ;
	if ( itMetallic != material.values.end() )
	{
		double	m = esl_fclamp( itMetallic->second.number_value, 0.0, 1.0 ) ;
		attr.nSpecular = (int32_t) eslRoundR64ToLInt( m * 256.0 ) ;
		attr.nReflection = (uint32_t) eslRoundR64ToLInt( m * m * 256.0 ) ;
	}
	tinygltf::ParameterMap::const_iterator
			itRoughness = material.values.find( "roughnessFactor" ) ;
	if ( itRoughness != material.values.end() )
	{
		double	r = esl_fclamp( itRoughness->second.number_value, 0.0, 1.0 ) ;
		attr.nSpecularSize = (int32_t) eslRoundR64ToLInt( r * 256.0 ) ;
	}
	//
	// 発光成分
	//
	tinygltf::ParameterMap::const_iterator
			itEmissiveFactor = material.additionalValues.find( "emissiveFactor" ) ;
	SGLPalette	rgbEmissive( 0 ) ;
	if ( itEmissiveFactor != material.additionalValues.end() )
	{
		rgbEmissive = ConvertColorFactor( itEmissiveFactor->second, 0 ) ;
	}
	//
	tinygltf::ParameterMap::const_iterator
			itEmissiveTexture = material.additionalValues.find( "emissiveTexture" ) ;
	if ( itEmissiveTexture != material.additionalValues.end() )
	{
		float32_t	factor =
			(float32_t) (rgbEmissive.argb.Red
						+ rgbEmissive.argb.Green
						+ rgbEmissive.argb.Blue) / (3.0f * 255.0f) ;
		const tinygltf::Parameter&
					prmEmissiveTexture = itEmissiveTexture->second ;
		int			iTexture = prmEmissiveTexture.TextureIndex() ;
		if ( iTexture >= 0 )
		{
			TextureEntry *	pte = m_aTextures.GetAt( (size_t) iTexture ) ;
			if ( pte && pte->pImageEntry )
			{
				attr.flagsShading |= shadingLuminousTexture ;
				pMaterial->SetTexture
					( pte->pImageEntry->pDefault,
						iNextTexture ++,
						S3DMaterial::textureLuminous,
						factor, 0.0f, pte->pImageEntry->pwszID ) ;
			}
		}
	}
	//
	// 法線テクスチャ
	//
	tinygltf::ParameterMap::const_iterator
			itNormalTexture = material.additionalValues.find( "normalTexture" ) ;
	if ( itNormalTexture != material.additionalValues.end() )
	{
		const tinygltf::Parameter&
			prmNormalTexture = itNormalTexture->second ;
		int	iTexture = prmNormalTexture.TextureIndex() ;
		if ( iTexture >= 0 )
		{
			TextureEntry *	pte = m_aTextures.GetAt( (size_t) iTexture ) ;
			if ( pte && pte->pImageEntry )
			{
				attr.flagsShading |= shadingNormalTexture ;
				pMaterial->SetTexture
					( pte->pImageEntry->pDefault,
						iNextTexture ++,
						S3DMaterial::textureNormal,
						1.0f, 0.0f, pte->pImageEntry->pwszID ) ;
			}
		}
	}
	//
	pMaterial->SetSurfaceAttribute( attr ) ;
	//
	// エントリ情報
	//
	MaterialEntry	matEntry ;
	eslFillMemory( &matEntry, 0, sizeof(MaterialEntry) ) ;
	matEntry.pMaterial = pMaterial ;
	matEntry.vUVScale = vUVScale ;
	//
	m_aMaterials.SetAt( iMaterial, matEntry ) ;
	//
	return	sglErrSuccess ;
}

// ノード内のメッシュ変換
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelGLTFImporter::ConvertNode
	( int iNode, const tinygltf::Node& node,
		const S4DDMatrix& mat4Base, S3DModelBoneSpace& boneParent )
{
	//
	// ノードエントリ
	//
	NodeEntry *	pneNode = m_aNodes.GetAt( (size_t) iNode ) ;
	ESLAssert( pneNode == NULL ) ;
	if ( pneNode == NULL )
	{
		pneNode = new NodeEntry ;
		m_aNodes.SetAt( (size_t) iNode, pneNode ) ;
	}
	//
	S4DDMatrix	mat4Node ;
	GetNodeTransformation( mat4Node, node ) ;
	pneNode->mat4Local = mat4Node ;
	//
	mat4Node = mat4Base * mat4Node ;
	pneNode->mat4Node = mat4Node ;
	pneNode->pNode = &node ;
	//
	// 行列要素変換
	//
	S3DDMatrix	matNode ;
	S3DDVector	vNode ;
	Matrix3x3From4x4<S3DDMatrix,S3DDVector,double>
				( matNode, vNode, pneNode->mat4Local ) ;
	//
	S3DModelPose::MatrixElement	meNode ;
	meNode.FromMatrix( matNode ) ;
	//
	pneNode->vTranslation = vNode ;
	pneNode->qRotation = meNode.qRotation ;
	pneNode->vScale = meNode.vZoom ;
	//
	// ボーン構造生成
	//
	SGLError	errAny = sglErrSuccess ;
	if ( pneNode->pBone == NULL )
	{
		SString	strBoneName ;
		if ( node.skin >= 0 )
		{
			strBoneName =
				ConvertString( m_model.skins[node.skin].name ) ;
		}
		if ( strBoneName.IsEmpty() )
		{
			strBoneName = ConvertString( node.name ) ;
		}
		if ( strBoneName.IsEmpty() )
		{
			strBoneName = L"bone" ;
		}
		if ( m_pDstModel->GetBonePropertyAs( strBoneName ) != NULL )
		{
			SString	strBaseID = strBoneName ;
			for ( int i = 1; i < 10000; i ++ )
			{
				strBoneName.Format( L"%s%d", (const wchar_t*) strBaseID, i ) ;
				if ( m_pDstModel->GetBonePropertyAs( strBoneName ) == NULL )
				{
					break ;
				}
			}
		}
		S3DModelBoneSpace *	pBone = new S3DModelBoneSpace ;
		m_pDstModel->AddBonePropertyAs( strBoneName, pBone ) ;
		pBone->SetBoneFlags
			( pBone->GetBoneFlags() | S3DModelBoneSpace::flagHaveOrgMatrix ) ;
		pBone->SetOriginalBoneMatrix( pneNode->mat4Node ) ;
		boneParent.AddChild( pBone ) ;
		//
		S3DDMatrix	matdNode, matdBase ;
		S3DDVector	vdNode, vdBase ;
		Matrix3x3From4x4
			<S3DDMatrix,S3DDVector,double>( matdNode, vdNode, mat4Node ) ;
		Matrix3x3From4x4
			<S3DDMatrix,S3DDVector,double>( matdBase, vdBase, mat4Base ) ;
		//
		pBone->m_vCenter = m_matCvtAxis * (vdNode - vdBase) ;
		//
		pneNode->pBone = pBone ;
	}
	if ( node.mesh >= 0 )
	{
		//
		// メッシュ
		//
		pneNode->iMesh = (ssize_t) m_pDstModel->GetMeshCount() ;
		pneNode->nMeshCount = 0 ;
		//
		SGLError	err = ConvertMeshOfNode( iNode, node, mat4Node ) ;
		if ( !errAny )
		{
			errAny = err ;
		}
		pneNode->nMeshCount = (ssize_t) m_pDstModel->GetMeshCount() - pneNode->iMesh ;
		//
		m_aMeshs.SetAt( (size_t) node.mesh, pneNode ) ;
	}
	//
	// 子ノード
	//
	for ( size_t i = 0; i < node.children.size(); i ++ )
	{
		SGLError	err =
			ConvertNode
				( node.children[i],
					m_model.nodes[node.children[i]],
					mat4Node, *(pneNode->pBone) ) ;
		if ( !errAny )
		{
			errAny = err ;
		}
	}
	return	errAny ;
}

SGLError S3DModelGLTFImporter::ConvertMeshOfNode
	( int iNode, const tinygltf::Node& node, const S4DDMatrix& mat4Node )
{
	NodeEntry *	pneNode = m_aNodes.GetAt( iNode ) ;
	ESLAssert( pneNode != NULL ) ;
	ESLAssert( pneNode->pNode == &node ) ;
	//
	S3DDMatrix	matdNode ;
	S3DDVector	vdNode ;
	Matrix3x3From4x4
		<S3DDMatrix,S3DDVector,double>( matdNode, vdNode, mat4Node ) ;
	//
	const S3DMatrix	matNode = m_matCvtAxis * matdNode ;
	const S3DVector	vNode = vdNode ;
	//
	const tinygltf::Mesh&	mesh = m_model.meshes[node.mesh] ;
	SString	strMeshName = ConvertString( mesh.name ) ;
	if ( strMeshName.IsEmpty() )
	{
		strMeshName = L"mesh" ;
	}
	//
	SGLError	errAny = sglErrSuccess ;
	for ( size_t i = 0; i < mesh.primitives.size(); i ++ )
	{
		const tinygltf::Primitive&	primitive = mesh.primitives[i] ;
		if ( primitive.indices < 0 )
		{
			continue ;
		}
		MaterialEntry *	pmeMaterial =
						m_aMaterials.GetAt( (size_t) primitive.material ) ;
		if ( (pmeMaterial == NULL) || (pmeMaterial->pMaterial == NULL) )
		{
			OutputError( strMeshName + L"：マテリアル情報が見つかりません" ) ;
			errAny = sglErrInvalidParam ;
			continue ;
		}
		const size_t	iMesh = m_pDstModel->GetMeshCount() ;
		//
		// 頂点情報取得
		//
		MeshSkinData *		pmsdSkin = new MeshSkinData ;
		SArray<S3DVector4>	bufVertex ;
		SArray<S3DVector4>	bufNormal ;
		SArray<S2DVector>	bufUVMap ;
		SArray<S3DColor>	bufColorMap ;
		SArray<uint32_t>	bufIndex ;
		bool				flagError = false ;
		//
		m_aMeshSkins.SetAt( iMesh, pmsdSkin ) ;
		//
		std::map<std::string, int>::const_iterator
				it( primitive.attributes.begin() ) ;
		std::map<std::string, int>::const_iterator
				itEnd( primitive.attributes.end() ) ;
		for ( ; it != itEnd; it ++ )
		{
			const tinygltf::Accessor&
						accessor = m_model.accessors[it->second] ;
			if ( it->first.compare( "POSITION" ) == 0 )
			{
				if ( !Sample3DVectorsFromAccessor( bufVertex, accessor ) )
				{
					OutputError( strMeshName + L"：頂点座標形式が不正です" ) ;
					flagError = true ;
					continue ;
				}
				matNode.RevolveVectors
					( bufVertex.GetArray(),
						bufVertex.GetArray(),
						bufVertex.GetLength(), vNode ) ;
			}
			else if ( it->first.compare( "NORMAL" ) == 0 )
			{
				if ( !Sample3DVectorsFromAccessor( bufNormal, accessor ) )
				{
					OutputError( strMeshName + L"：法線形式が不正です" ) ;
					flagError = true ;
					continue ;
				}
				S3DVector4 *	pvNormal = bufNormal.GetArray() ;
				const size_t	nCount = bufNormal.GetLength() ;
				S3DVector		vZero( 0, 0, 0 ) ;
				matNode.RevolveVectors
					( pvNormal, pvNormal, nCount, vZero ) ;
				//
				for ( size_t j = 0; j < nCount; j ++ )
				{
					pvNormal[j].Normalize() ;
				}
			}
			else if ( it->first.compare( "TEXCOORD_0" ) == 0 )
			{
				if ( !Sample2DVectorsFromAccessor( bufUVMap, accessor ) )
				{
					OutputError( strMeshName + L"：UV座標形式が不正です" ) ;
					flagError = true ;
					continue ;
				}
				S2DVector *	pvUVs = bufUVMap.GetArray() ;
				const size_t	nCount = bufUVMap.GetLength() ;
				for ( size_t j = 0; j < nCount; j ++ )
				{
					pvUVs[j].x *= pmeMaterial->vUVScale.x ;
					pvUVs[j].y *= pmeMaterial->vUVScale.y ;
				}
			}
			else if ( it->first.compare( "WEIGHTS_0" ) == 0 )
			{
				if ( !Sample4DVectorsFromAccessor
						( pmsdSkin->m_bufWeights0, accessor ) )
				{
					OutputError( strMeshName + L"：ウェイトマップ形式が不正です" ) ;
					flagError = true ;
					continue ;
				}
			}
			else if ( it->first.compare( "JOINTS_0" ) == 0 )
			{
				if ( !Sample4DIVectorsFromAccessor
						( pmsdSkin->m_bufJoints0, accessor ) )
				{
					OutputError( strMeshName + L"：ジョイントマップ形式が不正です" ) ;
					flagError = true ;
					continue ;
				}
			}
		}
		if ( flagError )
		{
			errAny = sglErrInvalidParam ;
			continue ;
		}
		if ( primitive.indices >= 0 )
		{
			const tinygltf::Accessor&
						accIndex = m_model.accessors[primitive.indices] ;
			if ( !SampleIndicesFromAccessor( bufIndex, accIndex ) )
			{
				OutputError( strMeshName + L"：インデックス形式が不正です" ) ;
				errAny = sglErrInvalidParam ;
				continue ;
			}
		}
		//
		// プリミティブ追加
		//
		S3DPrimitiveType	typePrimitive ;
		switch ( primitive.mode )
		{
		case	TINYGLTF_MODE_POINTS:
			typePrimitive = primitivePoint ;
			break ;
		case	TINYGLTF_MODE_LINE:
			typePrimitive = primitiveLine ;
			break ;
		case	TINYGLTF_MODE_TRIANGLES:
			typePrimitive = primitiveTriangle ;
			break ;
		case	TINYGLTF_MODE_TRIANGLE_STRIP:
			typePrimitive = primitiveTriangleStrip ;
			break ;
		default:
			OutputError( strMeshName + L"：未対応のプリミティブモードです" ) ;
			flagError = true ;
			break ;
		}
		if ( flagError )
		{
			errAny = sglErrInvalidParam ;
			continue ;
		}
		const size_t	nVertexCount = bufVertex.GetLength() ;
		S3DVector4 *	pvVertex = bufVertex.GetArray() ;
		S3DVector4 *	pvNormal = bufNormal.GetArray() ;
		S2DVector *		pvUVMap = bufUVMap.GetArray() ;
		S3DColor *		pColorMap = bufColorMap.GetArray() ;
		if ( bufNormal.GetLength() < nVertexCount )
		{
			pvNormal = NULL ;
		}
		if ( bufUVMap.GetLength() < nVertexCount )
		{
			pvUVMap = NULL ;
		}
		if ( bufColorMap.GetLength() < nVertexCount )
		{
			pColorMap = NULL ;
		}
		m_pDstModel->AddIndexedPrimitiveList
			( pmeMaterial->pMaterial, 0, typePrimitive,
				bufIndex.GetLength(), nVertexCount,
				pvVertex, pvNormal, pvUVMap,
				pColorMap, bufIndex.GetArray() ) ;
		//
		m_aRefMeshNode.SetAt( iMesh, iNode ) ;
		//
		// メッシュ情報追加
		//
		S3DVector4	vVertexMin, vVertexMax ;
		MinMaxVector4DArray
			( vVertexMin, vVertexMax, pvVertex, nVertexCount ) ;
		//
		S3DModelBuffer::MeshGroup	mg ;
		mg.m_iFirstMesh = (uint32_t) iMesh ;
		mg.m_nMeshCount = 1 ;
		mg.m_vCenter = (S3DDVector(vVertexMin)
							+ S3DDVector(vVertexMax)) * 0.5 ;
		//
		SString	strMeshID = strMeshName ;
		if ( m_pDstModel->GetMeshGroupAs( strMeshID ) != NULL )
		{
			for ( int j = 1; j < 10000; j ++ )
			{
				strMeshID.Format( L"%s%d", (const wchar_t*) strMeshName, j ) ;
				if ( m_pDstModel->GetMeshGroupAs( strMeshID ) == NULL )
				{
					break ;
				}
			}
		}
		m_pDstModel->GetMeshGroupList().Add( strMeshID, mg ) ;
		//
		pneNode->aMeshIDs.Add( new SString(strMeshID) ) ;
		//
		// モーフターゲット追加
		//
		SGLError	err =
			ConvertMorphTargets( (int) iMesh, strMeshID, matNode, primitive ) ;
		if ( err && !errAny )
		{
			errAny = err ;
		}
	}
	return	errAny ;
}

SGLError S3DModelGLTFImporter::ConvertMorphTargets
	( int iDstMesh, const wchar_t * pwszMeshID,
		const S3DMatrix& matNode, const tinygltf::Primitive& primitive )
{
	S3DModelData::MeshObject *
			pmoMesh = m_pDstModel->GetMeshObjectAt( iDstMesh ) ;
	if ( pmoMesh == NULL )
	{
		return	sglErrFailed ;
	}
	const tinygltf::Value	valNull ;
	const tinygltf::Value&
		valTargetNames =
			!primitive.extras.IsObject() ? valNull
				: primitive.extras.Get( std::string( "targetNames" ) ) ;
	//
	S3DVector	vZero( 0, 0, 0 ) ;
	SGLError	errAny = sglErrSuccess ;
	for ( size_t i = 0; i < primitive.targets.size(); i ++ )
	{
		//
		// モーフターゲット名
		//
		SString	strMorphName ;
		strMorphName.Format( L"%s_morph%d", pwszMeshID, i + 1 ) ;
		//
		if ( valTargetNames.IsArray() )
		{
			const tinygltf::Value&
				valTargetName = valTargetNames.Get( (int) i ) ;
			if ( valTargetName.IsString() )
			{
				strMorphName =
					ConvertString( valTargetName.Get<std::string>() ) ;
			}
		}
		if ( m_pDstModel->GetMorhTargetAs( strMorphName ) != NULL )
		{
			SString	strBaseName = strMorphName ;
			for ( int j = 1; j < 10000; j ++ )
			{
				strMorphName = strBaseName + SString(j) ;
				if ( m_pDstModel->GetMorhTargetAs( strMorphName ) == NULL )
				{
					break ;
				}
			}
		}
		//
		// モーフィングデータ
		//
		S3DModelBuffer::MorphTargetMesh *
				pmtmMesh = new S3DModelBuffer::MorphTargetMesh ;
		pmtmMesh->m_countVertex = pmoMesh->m_countVertex ;
		//
		bool	flagError = false ;
		std::map<std::string, int>::const_iterator
				it( primitive.targets[i].begin() ) ;
		std::map<std::string, int>::const_iterator
				itEnd( primitive.targets[i].end() ) ;
		for ( ; it != itEnd; it ++ )
		{
			const tinygltf::Accessor&
						accessor = m_model.accessors[it->second] ;
			if ( it->first.compare( "POSITION" ) == 0 )
			{
				if ( !Sample3DVectorsFromAccessor( pmtmMesh->m_bufVertex, accessor ) )
				{
					OutputError( strMorphName + L"：頂点座標形式が不正です" ) ;
					flagError = true ;
					continue ;
				}
				S3DVector4 *	pvMorphDelta = pmtmMesh->m_bufVertex.GetArray() ;
				const size_t	nCount = pmtmMesh->m_bufVertex.GetLength() ;
				float32_t *		pfpWeight = pmtmMesh->m_bufWeight.GetArray( nCount ) ;
				for ( size_t j = 0; j < nCount; j ++ )
				{
					if ( (pvMorphDelta[j].x != 0.0f)
						|| (pvMorphDelta[j].y != 0.0f)
						|| (pvMorphDelta[j].z != 0.0f) )
					{
						pfpWeight[j] = 1.0f ;
					}
				}
				matNode.RevolveVectors
					( pmtmMesh->m_bufVertex.GetArray(),
						pmtmMesh->m_bufVertex.GetArray(),
						pmtmMesh->m_bufVertex.GetLength(), vZero ) ;
				AddVectorArray
					( pmtmMesh->m_bufVertex,
						m_pDstModel->GetVertexBufferAt(pmoMesh->m_iVertex),
						pmoMesh->m_countVertex ) ;
			}
			else if ( it->first.compare( "NORMAL" ) == 0 )
			{
				if ( !Sample3DVectorsFromAccessor( pmtmMesh->m_bufNormal, accessor ) )
				{
					OutputError( strMorphName + L"：法線形式が不正です" ) ;
					flagError = true ;
					continue ;
				}
				matNode.RevolveVectors
					( pmtmMesh->m_bufNormal.GetArray(),
						pmtmMesh->m_bufNormal.GetArray(),
						pmtmMesh->m_bufNormal.GetLength(), vZero ) ;
				AddVectorArray
					( pmtmMesh->m_bufNormal,
						m_pDstModel->GetNormalBufferAt(pmoMesh->m_iNormal),
						pmoMesh->m_countVertex ) ;
				//
				S3DVector4 *	pvNormal = pmtmMesh->m_bufNormal.GetArray() ;
				const size_t	nCount = pmtmMesh->m_bufNormal.GetLength() ;
				for ( size_t j = 0; j < nCount; j ++ )
				{
					pvNormal[j].Normalize() ;
				}
			}
		}
		if ( flagError )
		{
			errAny = sglErrInvalidParam ;
			delete	pmtmMesh ;
			continue ;
		}
		//
		// 追加
		//
		if ( pmtmMesh->m_bufVertex.GetLength() == 0 )
		{
			delete	pmtmMesh ;
			continue ;
		}
		m_pDstModel->AddMorhTargetAs( strMorphName, pmtmMesh ) ;
		//
		pmoMesh->m_arrMorphTarget.Add( new SString(strMorphName) ) ;
	}
	return	errAny ;
}

void S3DModelGLTFImporter::AddVectorArray
		( SArray<S3DVector4>& bufDst,
				const S3DVector4 * pvSrc, size_t nVertexCount )
{
	S3DVector4 *	pvDst ;
	bufDst.SetLength( nVertexCount ) ;
	pvDst = bufDst.GetArray() ;
	//
	for ( size_t i = 0; i < nVertexCount; i ++ )
	{
		pvDst[i] += pvSrc[i] ;
	}
}

// ボーン関連付け構築
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelGLTFImporter::BuildAllBoneRelations( void )
{
	SGLError	erAny = sglErrSuccess ;
	for ( size_t i = 0; i < m_pDstModel->GetBoneRoot().GetChildrenCount(); i ++ )
	{
		S3DModelBoneSpace *	pBone =
			ESLTypeCast<S3DModelBoneSpace>
				( m_pDstModel->GetBoneRoot().GetChildAt( i ) ) ;
		if ( pBone != NULL )
		{
			SGLError	err = BuildBoneRelations( *pBone ) ;
			if ( !erAny )
			{
				erAny = err ;
			}
		}
	}
	m_pDstModel->BuildupBoneRelation() ;
	m_pDstModel->UpdateBoneMatrix() ;
	return	erAny ;
}

SGLError S3DModelGLTFImporter::BuildBoneRelations( S3DModelBoneSpace& bone )
{
	size_t		iBoneNode ;
	NodeEntry *	pneNode = GetNodeOfBone( &bone, iBoneNode ) ;
	ESLAssert( pneNode != NULL ) ;
	if ( pneNode == NULL )
	{
		return	sglErrFailed ;
	}
	//
	// ボーンハンドル生成
	//
	S3DDVector	vSubBonePos( 0, 0, 0 ) ;
	size_t		nSubBones = 0 ;
	for ( size_t i = 0; i < bone.GetChildrenCount(); i ++ )
	{
		S3DModelBoneSpace *	pBone =
			ESLTypeCast<S3DModelBoneSpace>( bone.GetChildAt( i ) ) ;
		if ( pBone != NULL )
		{
			vSubBonePos += pBone->m_vCenter ;
			nSubBones ++ ;
		}
	}
	if ( nSubBones >= 1 )
	{
		bone.SetBoneHandle( vSubBonePos / (double) nSubBones ) ;
	}
	else
	{
		S3DModelBoneSpace *	pParent =
			ESLTypeCast<S3DModelBoneSpace>( bone.GetParentSpace() ) ;
		if ( pParent != NULL )
		{
			bone.SetBoneHandle( pParent->GetBoneHandle() * 0.5 ) ;
		}
	}
	//
	// スキン関連付け
	//
	SGLError	errAny = sglErrSuccess ;
	if ( (pneNode->pNode != NULL)
		&& (pneNode->pNode->skin >= 0) )
	{
		SGLError	err = MakeSkinsOfBone( bone, pneNode, iBoneNode ) ;
		if ( !errAny )
		{
			errAny = err ;
		}
	}
	//
	// 子ボーン
	//
	for ( size_t i = 0; i < bone.GetChildrenCount(); i ++ )
	{
		S3DModelBoneSpace *	pBone =
			ESLTypeCast<S3DModelBoneSpace>( bone.GetChildAt( i ) ) ;
		if ( pBone != NULL )
		{
			SGLError	err = BuildBoneRelations( *pBone ) ;
			if ( !errAny )
			{
				errAny = err ;
			}
		}
	}
	return	errAny ;
}

SGLError S3DModelGLTFImporter::MakeSkinsOfBone
	( S3DModelBoneSpace& bone,
		S3DModelGLTFImporter::NodeEntry * pneNode, size_t iBoneNode )
{
	const tinygltf::Node *	pNode = pneNode->pNode ;
	ESLAssert( pNode != NULL ) ;
	ESLAssert( pNode->skin >= 0 ) ;
	//
	// 逆行列取得
	//
	const tinygltf::Skin&	skin = m_model.skins[pNode->skin] ;
	SArray<S4DMatrix>	bufInvBinds ;
	const size_t		nJoints = skin.joints.size() ;
	if ( skin.inverseBindMatrices >= 0 )
	{
		const tinygltf::Accessor&
			accessor = m_model.accessors[skin.inverseBindMatrices] ;
		if ( !Sample4DMatrixsFromAccessor( bufInvBinds, accessor ) )
		{
			return	sglErrNotSupported ;
		}
		if ( bufInvBinds.GetLength() < nJoints )
		{
			return	sglErrInvalidParam ;
		}
	}
	else
	{
		S4DMatrix	matI( 1, 1, 1, 1 ) ;
		bufInvBinds.SetLength( nJoints ) ;
		for ( size_t i = 0; i < nJoints; i ++ )
		{
			bufInvBinds.SetAt( i, matI ) ;
		}
	}
	//
	// 関連メッシュ
	//
	for ( size_t iJoint = 0; iJoint < nJoints; iJoint ++ )
	{
		size_t		iSkeltonNode = (size_t) skin.joints.at( iJoint ) ;
		NodeEntry *	pneSkelton = m_aNodes.GetAt( iSkeltonNode ) ;
		if ( (pneSkelton == NULL) || (pneSkelton->pBone == NULL) )
		{
			continue ;
		}
		S3DDMatrix	matSkelton ;
		S3DDVector	vSkelton ;
		Matrix3x3From4x4<S3DDMatrix,S3DDVector,double>
				( matSkelton, vSkelton, pneSkelton->mat4Node ) ;
		//
		S4DMatrix	mat4Skelton = pneSkelton->mat4Node ;
		S4DMatrix	matSkeltonRot ;
		Matrix4x4From3x3<float32_t,S3DDMatrix,S3DDVector>
				( matSkeltonRot, matSkelton, S3DDVector( 0, 0, 0 ) ) ;
		//
		S3DModelBoneSpace *	pBone = pneSkelton->pBone ;
		for ( size_t iMesh = 0; iMesh < m_aRefMeshNode.GetLength(); iMesh ++ )
		{
			if ( m_aRefMeshNode.At(iMesh) != iBoneNode )
			{
				continue ;
			}
			S3DModelBuffer::MeshObject *
				pmoMesh = m_pDstModel->GetMeshObjectAt( iMesh ) ;
			if ( pmoMesh == NULL )
			{
				continue ;
			}
			const size_t	nVertices = pmoMesh->m_countVertex ;
			MeshSkinData *	pmsdSkin = m_aMeshSkins.GetAt( iMesh ) ;
			if ( (pmsdSkin == NULL)
				|| (pmsdSkin->m_bufWeights0.GetLength() < nVertices)
				|| (pmsdSkin->m_bufJoints0.GetLength() < nVertices * 4) )
			{
				continue ;
			}
			SArray<float32_t>	bufWeight ;
			float32_t *			pfpWeight = bufWeight.GetArray( nVertices ) ;
			S4DVector *			pWeights0 = pmsdSkin->m_bufWeights0.GetArray() ;
			uint32_t *			pJoints0 = pmsdSkin->m_bufJoints0.GetArray() ;
			bool				flagRelBone = false ;
			for ( size_t j = 0; j < nVertices; j ++ )
			{
				float32_t	w = 0.0f ;
				if ( pJoints0[0] == iJoint )
				{
					w += pWeights0->x ;
				}
				if ( pJoints0[1] == iJoint )
				{
					w += pWeights0->y ;
				}
				if ( pJoints0[2] == iJoint )
				{
					w += pWeights0->z ;
				}
				if ( pJoints0[3] == iJoint )
				{
					w += pWeights0->w ;
				}
				flagRelBone |= (fabs(w) >= 1.0e-8) ;
				pfpWeight[j] = w ;
				pWeights0 ++ ;
				pJoints0 += 4 ;
			}
			if ( flagRelBone )
			{
				S4DMatrix *	pmatInvBind = bufInvBinds.GetAt( iJoint ) ;
				ESLAssert( pmatInvBind != NULL ) ;
				S4DMatrix	matBind, matInvBind ;
				matBind.InverseOf( *pmatInvBind ) ;
				matBind = m_mat4ICvtAxis * matBind * m_mat4CvtAxis ;
				matInvBind.InverseOf( matBind ) ;
				//
				S4DMatrix	matRelMesh, matIMesh ;
				S4DMatrix	matBonePos( 1, 1, 1, 1 ) ;
				matBonePos.m[0][3] = (float32_t) -vSkelton.x ;
				matBonePos.m[1][3] = (float32_t) vSkelton.y ;
				matBonePos.m[2][3] = (float32_t) vSkelton.z ;
//				matRelMesh.InverseOf( S4DMatrix( pneNode->mat4Node ) ) ;
//				matIMesh = matSkeltonRot * *pmatInvBind ;
				matRelMesh.InverseOf( S4DMatrix( pneNode->mat4Node ) ) ;
				matRelMesh = m_mat4CvtAxis * matRelMesh * m_mat4ICvtAxis ;
				matIMesh = matBonePos * m_mat4CvtAxis
							* mat4Skelton * *pmatInvBind * m_mat4ICvtAxis ;
				//
				pBone->AddEffectiveMeshIndex
					( iMesh, matIMesh, matRelMesh ) ;
				pBone->ExpandBoneWeightBounds
					( pmoMesh->m_iVertex, nVertices ) ;
				pBone->UpdateBoneWeight
					( pmoMesh->m_iVertex, nVertices, pfpWeight ) ;
			}
		}
	}
	return	sglErrSuccess ;
}

// ボーンのノードを取得
//////////////////////////////////////////////////////////////////////////////
S3DModelGLTFImporter::NodeEntry *
	S3DModelGLTFImporter::GetNodeOfBone
		( S3DModelBoneSpace * pBone, size_t& iBone ) const
{
	for ( size_t i = 0; i < m_aNodes.GetLength(); i ++ )
	{
		NodeEntry *	pneNode = m_aNodes.GetAt( i ) ;
		if ( (pneNode != NULL) && (pneNode->pBone == pBone) )
		{
			iBone = i ;
			return	pneNode ;
		}
	}
	return	NULL ;
}

// メッシュのノードを取得
//////////////////////////////////////////////////////////////////////////////
S3DModelGLTFImporter::NodeEntry *
	S3DModelGLTFImporter::GetNodeOfMesh
		( const wchar_t * pwszMeshID, size_t& iMesh ) const
{
	for ( size_t i = 0; i < m_aNodes.GetLength(); i ++ )
	{
		NodeEntry *	pneNode = m_aNodes.GetAt( i ) ;
		if ( pneNode == NULL )
		{
			continue ;
		}
		for ( size_t j = 0; j < pneNode->aMeshIDs.GetLength(); j ++ )
		{
			SString *	pstrMeshID = pneNode->aMeshIDs.GetAt( j ) ;
			if ( (pstrMeshID != NULL) && (*pstrMeshID == pwszMeshID) )
			{
				iMesh = i ;
				return	pneNode ;
			}
		}
	}
	return	NULL ;
}

// VRM 拡張変換
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelGLTFImporter::ConvertVRMExtensions( void )
{
	//
	// VRM 拡張取得
	//
	const tinygltf::Value *	pValVRM = nullptr ;
	const tinygltf::ExtensionMap::const_iterator&
			itVRM = m_model.extensions.find( std::string( "VRM" ) ) ;
	if ( itVRM != m_model.extensions.end() )
	{
		pValVRM = &(itVRM->second) ;
	}
	else
	{
		const tinygltf::ExtensionMap::const_iterator&
			itvrm = m_model.extensions.find( std::string( "vrm" ) ) ;
		if ( itvrm == m_model.extensions.end() )
		{
			return	sglErrSuccess ;
		}
		pValVRM = &(itvrm->second) ;
	}
	const tinygltf::Value&	valVRM = *pValVRM ;
	if ( !valVRM.IsObject() )
	{
		return	sglErrInvalidParam ;
	}
	//
	// VRM.blendShapeMaster.blendShapeGroups
	//
	ConvertVRMBlendShape( valVRM ) ;
	//
	// VRM.secondaryAnimation.boneGroups
	//
	ConvertVRMSecondaryAnimation( valVRM ) ;
	//
	return	sglErrSuccess ;
}

// VRM 拡張 VRM.blendShapeMaster.blendShapeGroups
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelGLTFImporter::ConvertVRMBlendShape( const tinygltf::Value& valVRM )
{
	const tinygltf::Value&
		valBlendShapeMaster = valVRM.Get( std::string( "blendShapeMaster" ) ) ;
	if ( !valBlendShapeMaster.IsObject() )
	{
		return	sglErrInvalidParam ;
	}
	const tinygltf::Value&
		valBlendShapeGroups = valBlendShapeMaster.Get( std::string( "blendShapeGroups" ) ) ;
	if ( !valBlendShapeGroups.IsArray() )
	{
		return	sglErrInvalidParam ;
	}
	SGLError	errAny = sglErrSuccess ;
	for ( size_t i = 0; i < valBlendShapeGroups.Size(); i ++ )
	{
		const tinygltf::Value&
				valGroup = valBlendShapeGroups.Get( (int) i ) ;
		if ( !valGroup.IsObject() )
		{
			continue ;
		}
		//
		// 名前取得
		//
		SString	strName ;
		const tinygltf::Value&
				valPresetName = valGroup.Get( std::string( "presetName" ) ) ;
		if ( valPresetName.IsString() )
		{
			strName = ConvertString( valPresetName.Get<std::string>() ) ;
		}
		else
		{
			const tinygltf::Value&
					valName = valGroup.Get( std::string( "name" ) ) ;
			if ( !valName.IsString() )
			{
				OutputError( L"VRM ブレンドシェイプ名がありません" ) ;
				errAny = sglErrInvalidParam ;
				continue ;
			}
			strName = ConvertString( valName.Get<std::string>() ) ;
		}
		//
		// ポーズID
		//
		SString	strPoseID = L"vrm.blendshape." ;
		strPoseID += strName ;
		if ( m_pDstModel->GetPoseLibrary().GetPoseAs( strPoseID ) != NULL )
		{
			SString	strBaseID = strPoseID ;
			for ( int j = 1; j < 10000; j ++ )
			{
				strPoseID = strBaseID + SString( j ) ;
				if ( m_pDstModel->GetPoseLibrary().GetPoseAs( strPoseID ) == NULL )
				{
					break ;
				}
			}
		}
		S3DModelPose *	pPose = new S3DModelPose ;
		m_pDstModel->GetPoseLibrary().AddPoseAs( strPoseID, pPose ) ;
		//
		// ポーズ要素
		//
		const tinygltf::Value&
				valBinds = valGroup.Get( std::string( "binds" ) ) ;
		if ( !valBinds.IsArray() )
		{
			OutputError( L"VRM ブレンドシェイプのバインド指定が不正です" ) ;
			errAny = sglErrInvalidParam ;
			continue ;
		}
		SGLError	err = ConvertVRMBlendShapeBinds( *pPose, valBinds ) ;
		if ( err && !errAny )
		{
			errAny = err ;
		}
		for ( size_t j = 0; j < pPose->GetMorphingCount(); j ++ )
		{
			S3DModelPose::MorphInfo *	pmi = pPose->GetMorphingAt( j ) ;
			ESLAssert( pmi != NULL ) ;
			//
			float32_t	wTotal = 0.0 ;
			for ( size_t j = 0; j < pmi->m_aAnimation.GetLength(); j ++ )
			{
				wTotal += pmi->m_aAnimation.At( j ) ;
			}
			//
			pmi->m_aTargetIDs.Add( new SString() ) ;
			pmi->m_aAnimation.Add( 1.0f - wTotal ) ;
		}
	}
	return	errAny ;
}

SGLError S3DModelGLTFImporter::ConvertVRMBlendShapeBinds
			( S3DModelPose& pose, const tinygltf::Value& valBinds )
{
	for ( size_t i = 0; i < valBinds.Size(); i ++ )
	{
		const tinygltf::Value&	valBind = valBinds.Get( (int) i ) ;
		if ( !valBind.IsObject() )
		{
			continue ;
		}
		const tinygltf::Value&
					valMesh = valBind.Get( std::string( "mesh" ) ) ;
		if ( !valMesh.IsInt() )
		{
			continue ;
		}
		const tinygltf::Value&
					valIndex = valBind.Get( std::string( "index" ) ) ;
		if ( !valIndex.IsInt() )
		{
			continue ;
		}
		const tinygltf::Value&
					valWeight = valBind.Get( std::string( "weight" ) ) ;
		if ( !valWeight.IsNumber() && !valWeight.IsInt() )
		{
			continue ;
		}
		NodeEntry *	pneNode = m_aMeshs.GetAt( (size_t) valMesh.Get<int>() ) ;
		if ( pneNode == NULL )
		{
			continue ;
		}
		for ( size_t j = 0; j < pneNode->aMeshIDs.GetLength(); j ++ )
		{
			SString *	pstrMeshID = pneNode->aMeshIDs.GetAt( j ) ;
			ESLAssert( pstrMeshID != NULL ) ;
			//
			S3DModelData::MeshObject *
				pmoMesh = m_pDstModel->GetMeshObjectAt( pneNode->iMesh + j ) ;
			ESLAssert( pmoMesh != NULL ) ;
			//
			SString *	pstrMorphTarget =
							pmoMesh->m_arrMorphTarget.GetAt
								( (size_t) valIndex.Get<int>() ) ;
			if ( pstrMorphTarget == NULL )
			{
				continue ;
			}
			//
			S3DModelPose::MorphInfo *
						pmi = pose.GetMorphingAs( *pstrMeshID ) ;
			if ( pmi == NULL )
			{
				pmi = new S3DModelPose::MorphInfo ;
				pose.AddMorphingAs( *pstrMeshID, pmi ) ;
			}
			float32_t	w = 0.0f ;
			if ( valWeight.IsNumber() )
			{
				w = (float32_t) valWeight.Get<double>() / 100.0f ;
			}
			else if ( valWeight.IsInt() )
			{
				w = (float32_t) valWeight.Get<int>() / 100.0f ;
			}
			pmi->m_aTargetIDs.Add( new SString( *pstrMorphTarget ) ) ;
			pmi->m_aAnimation.Add( w ) ;
		}
	}
	return	sglErrSuccess ;
}

// VRM 拡張 secondaryAnimation.boneGroups
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelGLTFImporter::ConvertVRMSecondaryAnimation( const tinygltf::Value& valVRM )
{
	const tinygltf::Value&
		valSecondaryAnimation = valVRM.Get( std::string( "secondaryAnimation" ) ) ;
	if ( !valSecondaryAnimation.IsObject() )
	{
		return	sglErrInvalidParam ;
	}
	const tinygltf::Value&
		valBoneGroups = valSecondaryAnimation.Get( std::string( "boneGroups" ) ) ;
	if ( valBoneGroups.IsArray() )
	{
		ConvertVRMSecondaryAnimationBones( valBoneGroups ) ;
	}
	const tinygltf::Value&
		valColliderGroups = valSecondaryAnimation.Get( std::string( "colliderGroups" ) ) ;
	if ( valColliderGroups.IsArray() )
	{
		ConvertVRMSecondaryAnimationColliders( valColliderGroups ) ;
	}
	return	sglErrSuccess ;
}

SGLError S3DModelGLTFImporter::ConvertVRMSecondaryAnimationBones( const tinygltf::Value& valBoneGroups )
{
	for ( size_t i = 0; i < valBoneGroups.Size(); i ++ )
	{
		const tinygltf::Value&
				valSpring = valBoneGroups.Get( (int) i ) ;
		if ( !valSpring.IsObject() )
		{
			continue ;
		}
		//
		// パラメータ取得
		//
		double	fpStiffiness =
			GetJsonPropNumberAs( valSpring, "stiffiness", 0.5 ) ;
		double	fpHitRadius =
			GetJsonPropNumberAs( valSpring, "hitRadius", 0.0 ) ;
		//
		S3DModelBoneSpace::PhysMaterial	physMaterial ;
		physMaterial.fpShrinkable = fpStiffiness ;
		physMaterial.fpElasticity = fpStiffiness ;
		physMaterial.fpHardness = pow( fpStiffiness, 2.0 );
		physMaterial.fpCollisionRadius = fpHitRadius ;
		physMaterial.nPhysExFlags1 = S3DModelBoneSpace::flagPhysExColliderUseMask ;
		//
		// 当たり判定対象配列
		//
		const tinygltf::Value&
			valColliders = valSpring.Get( std::string( "colliderGroups" ) ) ;
		if ( valColliders.IsArray() )
		{
			for ( size_t j = 0; j < valColliders.Size(); j ++ )
			{
				const tinygltf::Value&
							valColIndex = valColliders.Get( (int) j ) ;
				if ( !valColIndex.IsInt() )
				{
					continue ;
				}
				int	nIndex = valColIndex.Get<int>() ;
				if ( (nIndex >= 0)
					&& (nIndex < S3DModelBoneSpace::flagPhysExColliderMaxCount) )
				{
					physMaterial.nPhysExFlags1 |=
						S3DModelBoneSpace::flagPhysExCollider0 << nIndex ;
				}
			}
		}
		//
		// 対象ボーン配列
		//
		const tinygltf::Value&
			valBones = valSpring.Get( std::string( "bones" ) ) ;
		if ( !valBones.IsArray() )
		{
			continue ;
		}
		for ( size_t j = 0; j < valBones.Size(); j ++ )
		{
			const tinygltf::Value&	valBone = valBones.Get( (int) j ) ;
			if ( !valBone.IsInt() )
			{
				continue ;
			}
			NodeEntry *	pneBone =
				m_aNodes.GetAt( (size_t) valBone.Get<int>() ) ;
			if ( (pneBone == NULL) || (pneBone->pBone == NULL) )
			{
				continue ;
			}
			SetBonePhysMaterials( pneBone->pBone, physMaterial ) ;
		}
	}
	return	sglErrSuccess ;
}

void S3DModelGLTFImporter::SetBonePhysMaterials
	( S3DModelBoneSpace * pBone,
		const S3DModelBoneSpace::PhysMaterial& physMaterial ) 
{
	pBone->SetBoneFlags
		( pBone->GetBoneFlags() | S3DModelBoneSpace::flagBonePhysics ) ;
	pBone->SetBonePhysicalMaterial( physMaterial ) ;
	//
	for ( size_t i = 0; i < pBone->GetChildrenCount(); i ++ )
	{
		S3DModelBoneSpace *	pChild =
			ESLTypeCast<S3DModelBoneSpace>( pBone->GetChildAt( i ) ) ;
		if ( pChild != NULL )
		{
			SetBonePhysMaterials( pChild, physMaterial ) ;
		}
	}
}

SGLError S3DModelGLTFImporter::ConvertVRMSecondaryAnimationColliders( const tinygltf::Value& valColliderGroups )
{
	const S3DMatrix	matCvtAxis = m_matCvtAxis ;
	int	iColliderNum = 0 ;
	//
	for ( size_t i = 0; i < valColliderGroups.Size(); i ++ )
	{
		const tinygltf::Value&
				valColliderGroup = valColliderGroups.Get( (int) i ) ;
		if ( !valColliderGroup.IsObject() )
		{
			continue ;
		}
		//
		// 設置ボーン
		//
		SString	strRefBone ;
		const tinygltf::Value&
			valNode = valColliderGroup.Get( std::string( "node" ) ) ;
		if ( valNode.IsInt() )
		{
			NodeEntry *	pneNode =
					m_aNodes.GetAt( (size_t) valNode.Get<int>() ) ;
			if ( pneNode != NULL )
			{
				const SString *	pstrBoneID =
						m_pDstModel->GetBoneIdentityOf( pneNode->pBone ) ;
				if ( pstrBoneID != NULL )
				{
					strRefBone = *pstrBoneID ;
				}
			}
		}
		//
		// コライダ配列
		//
		const tinygltf::Value&
			valColliders = valColliderGroup.Get( std::string( "colliders" ) ) ;
		if ( !valColliders.IsArray() )
		{
			continue ;
		}
		for ( size_t j = 0; j < valColliders.Size(); j ++ )
		{
			const tinygltf::Value&
					valCollider = valColliders.Get( (int) j ) ;
			if ( !valCollider.IsObject() )
			{
				continue ;
			}
			const tinygltf::Value&
					valOffset = valCollider.Get( std::string( "offset" ) ) ;
			if ( !valOffset.IsObject() )
			{
				continue ;
			}
			S3DModelData::MarkerInfo *	pmi = new S3DModelData::MarkerInfo ;
			pmi->m_type = S3DModelData::MarkerInfo::typeBoneColider ;
			pmi->m_shape = S3DModelData::MarkerInfo::shapeSphere ;
			pmi->m_iCollider = (int) i ;
			pmi->m_vPosition.x =
				(float32_t) GetJsonPropNumberAs( valOffset, "x", 0.0 ) ;
			pmi->m_vPosition.y =
				(float32_t) GetJsonPropNumberAs( valOffset, "y", 0.0 ) ;
			pmi->m_vPosition.z =
				(float32_t) GetJsonPropNumberAs( valOffset, "z", 0.0 ) ;
			pmi->m_fpRadius =
				(float32_t) GetJsonPropNumberAs
								( valCollider, "radius", 0.0 ) ;
			pmi->m_strRefBone = strRefBone ;
			//
			matCvtAxis.RevolveVector( pmi->m_vPosition ) ;
			pmi->CommitInfo() ;
			//
			SString	strID = L"vrm.bone_collider" ;
			strID += SString( iColliderNum ++ ) ;
			//
			m_pDstModel->GetMarkerInfoList().Add( strID, pmi ) ;
		}
	}
	return	sglErrSuccess ;
}

// json 数値要素取得
//////////////////////////////////////////////////////////////////////////////
double S3DModelGLTFImporter::GetJsonPropNumberAs
	( const tinygltf::Value& val, const char * pszName, double numDef )
{
	const tinygltf::Value&	valProp = val.Get( std::string( pszName ) ) ;
	if ( valProp.IsNumber() )
	{
		return	valProp.Get<double>() ;
	}
	if ( valProp.IsInt() )
	{
		return	valProp.Get<int>() ;
	}
	return	numDef ;
}

int S3DModelGLTFImporter::GetJsonPropIntAs
	( const tinygltf::Value& val, const char * pszName, int nDef )
{
	const tinygltf::Value&	valProp = val.Get( std::string( pszName ) ) ;
	if ( valProp.IsInt() )
	{
		return	valProp.Get<int>() ;
	}
	return	nDef ;
}

// アニメーション変換
//////////////////////////////////////////////////////////////////////////////
SGLError S3DModelGLTFImporter::ConvertAnimation
	( size_t iAnimation, const tinygltf::Animation& animations )
{
	SString	strAnimationName = ConvertString( animations.name ) ;
	AnimationSet *	panis = new AnimationSet ;
	m_ssoaAnimations.Add( strAnimationName, panis ) ;
	//
	SGLError	errAny = sglErrSuccess ;
	double		secTotalDuration = 0.0 ;
	for ( size_t i = 0; i < animations.channels.size(); i ++ )
	{
		//
		// チャネルとサンプラー取得
		//
		const tinygltf::AnimationChannel&
							aniChannel = animations.channels[i] ;
		if ( (aniChannel.sampler < 0)
			|| ((size_t) aniChannel.sampler >= animations.samplers.size()) )
		{
			OutputError( strAnimationName + L"：サンプラー指標が不正です" ) ;
			errAny = sglErrInvalidParam ;
			continue ;
		}
		const tinygltf::AnimationSampler&
				aniSampler = animations.samplers[aniChannel.sampler] ;
		//
		// ターゲット
		//
		NodeEntry *	pneNode =
				m_aNodes.GetAt( (size_t) aniChannel.target_node ) ;
		if ( pneNode == NULL )
		{
			OutputError( strAnimationName + L"：ターゲット指標が不正です" ) ;
			errAny = sglErrInvalidParam ;
			continue ;
		}
		AnimationTrack *	paniTrack = new AnimationTrack ;
		panis->m_aTracks.Add( paniTrack ) ;
		//
		paniTrack->m_pTargetNode = pneNode ;
		//
		bool	flagTargetShouldBone = false ;
		if ( aniChannel.target_path.compare( "translation" ) == 0 )
		{
			paniTrack->m_pathTarget = pathTranslation ;
			paniTrack->m_nElements = 3 ;
			flagTargetShouldBone = true ;
		}
		else if ( aniChannel.target_path.compare( "rotation" ) == 0 )
		{
			paniTrack->m_pathTarget = pathRotation ;
			paniTrack->m_nElements = 4 ;
			flagTargetShouldBone = true ;
		}
		else if ( aniChannel.target_path.compare( "scale" ) == 0 )
		{
			paniTrack->m_pathTarget = pathScale ;
			paniTrack->m_nElements = 3 ;
			flagTargetShouldBone = true ;
		}
		else if ( aniChannel.target_path.compare( "weights" ) == 0 )
		{
			paniTrack->m_pathTarget = pathWeights ;
			//
			S3DModelData::MeshObject *	pmoMesh =
					m_pDstModel->GetMeshObjectAt( pneNode->iMesh ) ;
			if ( pmoMesh == NULL )
			{
				OutputError
					( strAnimationName
						+ L"：モーフィング対象がメッシュではありません" ) ;
				errAny = sglErrInvalidParam ;
				continue ;
			}
			paniTrack->m_nElements = pmoMesh->m_arrMorphTarget.GetLength() ;
			if ( paniTrack->m_nElements == 0 )
			{
				OutputError
					( strAnimationName
						+ L"：メッシュにモーフィング対象がありません" ) ;
				errAny = sglErrInvalidParam ;
				continue ;
			}
			pneNode->aWeights.SetLength( paniTrack->m_nElements ) ;
		}
		else
		{
			OutputError
				( strAnimationName + L"：未対応のターゲットパス "
								+ ConvertString(aniChannel.target_path) ) ;
			errAny = sglErrInvalidParam ;
			continue ;
		}
		if ( flagTargetShouldBone && (pneNode->pBone == NULL) )
		{
			OutputError
				( strAnimationName + L"：ターゲットがボーンではありません" ) ;
			errAny = sglErrInvalidParam ;
			continue ;
		}
		//
		// 補完方法
		//
		if ( aniSampler.interpolation.compare( "LINEAR" ) == 0 )
		{
			paniTrack->m_interplation = interpolationLinear ;
		}
		else if ( aniSampler.interpolation.compare( "STEP" ) == 0 )
		{
			paniTrack->m_interplation = interpolationStep ;
		}
		else if ( aniSampler.interpolation.compare( "CATMULLROMSPLINE" ) == 0 )
		{
			paniTrack->m_interplation = interpolationCatmullromSpline ;
		}
		else if ( aniSampler.interpolation.compare( "CUBICSPLINE" ) == 0 )
		{
			paniTrack->m_interplation = interpolationCubicSpline ;
		}
		else if ( aniSampler.interpolation.compare( "" ) != 0 )
		{
			OutputWarning
				( strAnimationName + L"：不明な補完法です "
							+ ConvertString(aniSampler.interpolation) ) ;
		}
		//
		// キーフレーム時間
		//
		if ( !SampleFloatArrayFromAccessor
				( paniTrack->m_aKeyTimes,
						m_model.accessors[aniSampler.input] ) )
		{
			OutputError
				( strAnimationName + L"：キーフレーム時間を取得できません" ) ;
			errAny = sglErrInvalidParam ;
			continue ;
		}
		const size_t	nKeyTimeCount = paniTrack->m_aKeyTimes.GetLength() ;
		if ( nKeyTimeCount == 0 )
		{
			continue ;
		}
		double *	pfpKeyTimes = paniTrack->m_aKeyTimes.GetArray() ;
		paniTrack->m_secFirst = pfpKeyTimes[0] ;
		paniTrack->m_secEnd = pfpKeyTimes[nKeyTimeCount - 1] ;
		//
		if ( secTotalDuration < paniTrack->m_secEnd )
		{
			secTotalDuration = paniTrack->m_secEnd ;
		}
		//
		// キーフレーム値
		//
		if ( !SampleFloatArrayFromAccessor
				( paniTrack->m_bufKeyValues,
						m_model.accessors[aniSampler.output] ) )
		{
			OutputError
				( strAnimationName + L"：キーフレーム値を取得できません" ) ;
			errAny = sglErrInvalidParam ;
			continue ;
		}
		if ( paniTrack->m_bufKeyValues.GetLength()
					< nKeyTimeCount * paniTrack->m_nElements )
		{
			OutputError
				( strAnimationName
					+ L"：キーフレーム値の数がキーフレームと一致しません" ) ;
			errAny = sglErrInvalidParam ;
			continue ;
		}
		//
		// スプライン準備
		//
		if ( (paniTrack->m_interplation == interpolationCatmullromSpline)
			|| (paniTrack->m_interplation == interpolationCubicSpline) )
		{
			SArray<double>	aValues ;
			double *		pValues = aValues.GetArray( nKeyTimeCount ) ;
			const double *	pKeyValues = paniTrack->m_bufKeyValues.GetArray() ;
			for ( size_t j = 0; j < paniTrack->m_nElements; j ++ )
			{
				for ( size_t k = 0; k < nKeyTimeCount; k ++ )
				{
					pValues[k] = pKeyValues[k * paniTrack->m_nElements + j] ;
				}
				SGLSplineCurvesN *	pSpline = new SGLSplineCurvesN ;
				pSpline->CreateSpline( pValues, pfpKeyTimes, nKeyTimeCount ) ;
				paniTrack->m_splines.SetAt( j, pSpline ) ;
			}
		}
	}
	//
	// 出力ポーズ準備
	//
	S3DModelPose *	pPose = new S3DModelPose ;
	SString			strPoseID = strAnimationName ;
	if ( m_pDstModel->GetPoseLibrary().GetPoseAs( strPoseID ) != NULL )
	{
		OutputWarning
			( strAnimationName + L"：アニメーション名が重複しています" ) ;
		//
		for ( size_t i = 1; i < 10000; i ++ )
		{
			strPoseID = strAnimationName + SString(i) ;
			if ( m_pDstModel->GetPoseLibrary().GetPoseAs( strPoseID ) == NULL )
			{
				break ;
			}
		}
	}
	m_pDstModel->GetPoseLibrary().AddPoseAs( strPoseID, pPose ) ;
	//
	S3DModelPose::MetaInfo	metaInfo ;
	metaInfo.msecDuration =
			(uint32_t) eslRoundR64ToLInt( secTotalDuration * 1000.0 ) ;
	metaInfo.fxFrameRatio =
			(uint32_t) eslRoundR64ToLInt( m_fpsFrameRatio * 0x10000 ) ;
	pPose->SetMetaInfo( metaInfo ) ;
	//
	for ( size_t i = 0; i < panis->m_aTracks.GetLength(); i ++ )
	{
		AnimationTrack *	paniTrack = panis->m_aTracks.GetAt( i ) ;
		ESLAssert( paniTrack != NULL ) ;
		ESLAssert( paniTrack->m_pTargetNode != NULL ) ;
		NodeEntry *	pneNode = paniTrack->m_pTargetNode ;
		if ( paniTrack->m_pathTarget == pathWeights )
		{
			for ( size_t j = 0; j < pneNode->aMeshIDs.GetLength(); j ++ )
			{
				SString *	pstrMeshID = pneNode->aMeshIDs.GetAt( j ) ;
				ESLAssert( pstrMeshID != NULL ) ;
				//
				S3DModelData::MeshObject *	pmoMesh =
						m_pDstModel->GetMeshObjectAt( pneNode->iMesh + j ) ;
				ESLAssert( pmoMesh != NULL ) ;
				//
				S3DModelPose::MorphInfo *
						pmi = pPose->GetMorphingAs( *pstrMeshID ) ;
				if ( pmi == NULL )
				{
					pmi = new S3DModelPose::MorphInfo ;
					pPose->AddMorphingAs( *pstrMeshID, pmi ) ;
					//
					pmi->m_aTargetIDs.Add( new SString() ) ;
					for ( size_t k = 0; k < pmoMesh->m_arrMorphTarget.GetLength(); k ++ )
					{
						SString *	pstrMorph = pmoMesh->m_arrMorphTarget.GetAt( k ) ;
						ESLAssert( pstrMorph != NULL ) ;
						pmi->m_aTargetIDs.Add( new SString( *pstrMorph ) ) ;
					}
				}
			}
		}
		else if ( pneNode->pBone != NULL )
		{
			const SString *	pstrBoneID =
					m_pDstModel->GetBoneIdentityOf( pneNode->pBone ) ;
			if ( pstrBoneID != NULL )
			{
				S3DModelPose::JointAnimation *
						pja = pPose->GetJointAs( *pstrBoneID ) ;
				if ( pja == NULL )
				{
					pja = new S3DModelPose::JointAnimation ;
					pPose->AddJointAs( *pstrBoneID, pja ) ;
				}
			}
		}
	}
	//
	// フレームサンプリング
	//
	size_t	nTotalFrameCount =
				(size_t) floor( secTotalDuration * m_fpsFrameRatio ) ;
	//
	for ( size_t iFrame = 0; iFrame < nTotalFrameCount; iFrame ++ )
	{
		//
		// 値計算
		//
		double	t = iFrame / m_fpsFrameRatio ;
		for ( size_t i = 0; i < panis->m_aTracks.GetLength(); i ++ )
		{
			AnimationTrack *	paniTrack = panis->m_aTracks.GetAt( i ) ;
			ESLAssert( paniTrack != NULL ) ;
			//
			if ( (paniTrack->m_secFirst <= t) && (t <= paniTrack->m_secEnd) )
			{
				SampleAnimationFrame( *paniTrack, t ) ;
			}
		}
		//
		// ボーン座標追加
		//
		for ( size_t iJoint = 0; iJoint < pPose->GetJointCount(); iJoint ++ )
		{
			S3DModelPose::JointAnimation *	pja = pPose->GetJointAt( iJoint ) ;
			ESLAssert( pja != NULL ) ;
			//
			S3DModelBoneSpace *
				pBone = m_pDstModel->GetBonePropertyAs
								( pPose->GetJointNameAt( iJoint ) ) ;
			ESLAssert( pBone != NULL ) ;
			//
			size_t		iBone ;
			NodeEntry *	pneBone = GetNodeOfBone( pBone, iBone ) ;
			ESLAssert( pneBone != NULL ) ;
			if ( pneBone == NULL )
			{
				continue ;
			}
			//
			// 行列再構成
			//
			S3DDMatrix	matBone( 1, 1, 1 ) ;
			pneBone->qRotation.ToMatrix( matBone ) ;
			matBone.MagnifyByVector( pneBone->vScale ) ;
			//
			S3DDMatrix	matLocal ;
			S3DDVector	vLocal ;
			Matrix3x3From4x4<S3DDMatrix,S3DDVector,double>
					( matLocal, vLocal, pneBone->mat4Local ) ;
			//
			// 差分行列
			//
			S3DDMatrix	matDelta = matBone * matLocal.Inverse() ;
			S3DDVector	vDelta = pneBone->vTranslation - vLocal ;
			//
			// 座標空間変換
			//
			matDelta = m_matCvtAxis * matDelta * m_matCvtAxis.Inverse() ;
			vDelta = m_matCvtAxis * vDelta ;
			//
			S4DMatrix	mat4Delta ;
			Matrix4x4From3x3
				<float32_t,S3DDMatrix,S3DDVector>
						( mat4Delta, matDelta, vDelta ) ;
			//
			ESLAssert( pja->m_aMatrixs.GetLength() == iFrame ) ;
			pja->m_aMatrixs.Add( mat4Delta ) ;
		}
		//
		// モーフィング追加
		//
		for ( size_t iMorph = 0; iMorph < pPose->GetMorphingCount(); iMorph ++ )
		{
			S3DModelPose::MorphInfo *	pmi = pPose->GetMorphingAt( iMorph ) ;
			ESLAssert( pmi != NULL ) ;
			//
			size_t		iMeshNode ;
			NodeEntry *	pneMesh =
				GetNodeOfMesh
					( pPose->GetMorphingMeshNameAt( iMorph ), iMeshNode ) ;
			ESLAssert( pneMesh != NULL ) ;
			if ( pneMesh == NULL )
			{
				continue ;
			}
			size_t	nElements = pmi->m_aTargetIDs.GetLength() ;
			ESLAssert( nElements >= 1 ) ;
			ESLAssert( pneMesh->aWeights.GetLength() == nElements - 1 ) ;
			//
			float32_t *	pfpWeights = pneMesh->aWeights.GetArray( nElements - 1 ) ;
			float32_t	wTotal = 0.0 ;
			for ( size_t j = 0; j < nElements - 1; j ++ )
			{
				wTotal += pfpWeights[j] ;
			}
			//
			ESLAssert( pmi->m_aAnimation.GetLength() == iFrame * nElements ) ;
			pmi->m_aAnimation.Add( 1.0f - wTotal ) ;
			pmi->m_aAnimation.AddArray( pfpWeights, nElements - 1 ) ;
		}
	}
	return	errAny ;
}

void S3DModelGLTFImporter::SampleAnimationFrame
			( const AnimationTrack& aniTrack, double t )
{
	ESLAssert( aniTrack.m_pTargetNode != NULL ) ;
	NodeEntry *		pneNode = aniTrack.m_pTargetNode ;
	const size_t	nElements = aniTrack.m_nElements ;
	//
	// フレーム補完値計算
	//
	SArray<double>	bufValues ;
	double *	pValues = bufValues.GetArray( nElements ) ;
	//
	if ( (aniTrack.m_interplation == interpolationCatmullromSpline)
		|| (aniTrack.m_interplation == interpolationCubicSpline) )
	{
		for ( size_t i = 0; i < nElements; i ++ )
		{
			SGLSplineCurves *	pSpline = aniTrack.m_splines.GetAt( i ) ;
			ESLAssert( pSpline != NULL ) ;
			pValues[i] = pSpline->Interpolate( t ) ;
		}
	}
	else
	{
		size_t			nKeyCount = aniTrack.m_aKeyTimes.GetLength() ;
		const double *	pKeyTimes = aniTrack.m_aKeyTimes.GetArray() ;
		size_t			iKeyFrame = 0 ;
		double			fpInterpolate = 0.0 ;
		while ( iKeyFrame + 1 < nKeyCount )
		{
			if ( (pKeyTimes[iKeyFrame] <= t)
					&& (t <= pKeyTimes[iKeyFrame + 1]) )
			{
				if ( pKeyTimes[iKeyFrame + 1] != pKeyTimes[iKeyFrame] )
				{
					fpInterpolate = (t - pKeyTimes[iKeyFrame])
						/ (pKeyTimes[iKeyFrame + 1] - pKeyTimes[iKeyFrame]) ;
				}
				break ;
			}
			iKeyFrame ++ ;
		}
		if ( iKeyFrame + 1 >= nKeyCount )
		{
			ESLAssert( nKeyCount >= 1 ) ;
			iKeyFrame = nKeyCount - 1 ;
		}
		if ( (aniTrack.m_interplation == interpolationStep)
			|| (iKeyFrame + 1 >= nKeyCount) )
		{
			const double *	pKeyValues =
					aniTrack.m_bufKeyValues.GetAt( iKeyFrame * nElements ) ;
			ESLAssert( pKeyValues != NULL ) ;
			for ( size_t i = 0; i < nElements; i ++ )
			{
				pValues[i] = pKeyValues[i] ;
			}
		}
		else
		{
			const double *	pKeyValues =
					aniTrack.m_bufKeyValues.GetAt( iKeyFrame * nElements ) ;
			ESLAssert( pKeyValues != NULL ) ;
			for ( size_t i = 0; i < nElements; i ++ )
			{
				pValues[i] = pKeyValues[i] * (1.0 - fpInterpolate)
							+ pKeyValues[i + nElements] * fpInterpolate ;
			}
		}
	}
	//
	// ターゲットの値を更新する
	//
	if ( aniTrack.m_pathTarget == pathTranslation )
	{
		ESLAssert( nElements == 3 ) ;
		pneNode->vTranslation.x = pValues[0] ;
		pneNode->vTranslation.y = pValues[1] ;
		pneNode->vTranslation.z = pValues[2] ;
	}
	else if ( aniTrack.m_pathTarget == pathRotation )
	{
		ESLAssert( nElements == 4 ) ;
		pneNode->qRotation.q[0] = pValues[3] ;
		pneNode->qRotation.q[1] = pValues[0] ;
		pneNode->qRotation.q[2] = pValues[1] ;
		pneNode->qRotation.q[3] = pValues[2] ;
		pneNode->qRotation.Normalize() ;
	}
	else if ( aniTrack.m_pathTarget == pathScale )
	{
		ESLAssert( nElements == 3 ) ;
		pneNode->vScale.x = pValues[0] ;
		pneNode->vScale.y = pValues[1] ;
		pneNode->vScale.z = pValues[2] ;
	}
	else if ( aniTrack.m_pathTarget == pathWeights )
	{
		ESLAssert( pneNode->aWeights.GetLength() == nElements ) ;
		for ( size_t i = 0; i < nElements; i ++ )
		{
			pneNode->aWeights.SetAt( i, (float32_t) pValues[i] ) ;
		}
	}
}

// ノードの変換行列取得
//////////////////////////////////////////////////////////////////////////////
void S3DModelGLTFImporter::GetNodeTransformation
	( S4DDMatrix& mat4, const tinygltf::Node& node ) const
{
	if ( node.matrix.size() == 16 )
	{
		const double *	pNodeMatrix = node.matrix.data() ;
		for ( int i = 0; i < 4; i ++ )
		{
			mat4.m[0][i] = pNodeMatrix[i*4] ;
			mat4.m[1][i] = pNodeMatrix[i*4 + 1] ;
			mat4.m[2][i] = pNodeMatrix[i*4 + 2] ;
			mat4.m[3][i] = pNodeMatrix[i*4 + 3] ;
		}
	}
	else
	{
		S3DDMatrix	mat( 1, 1, 1 ) ;
		S3DDVector	vec( 0, 0, 0 ) ;
		//
		if ( node.translation.size() == 3 )
		{
			vec.x = node.translation[0] ;
			vec.y = node.translation[1] ;
			vec.z = node.translation[2] ;
		}
		if ( node.rotation.size() == 4 )
		{
			S3DDQuaternion	qRot( node.rotation[3],
									node.rotation[0],
									node.rotation[1],
									node.rotation[2] ) ;
			qRot.ToMatrix( mat ) ;
		}
		if ( node.scale.size() == 3 )
		{
			mat.MagnifyByVector
				( S3DDVector( node.scale[0], node.scale[1], node.scale[2] ) ) ;
		}
		//
		Matrix4x4From3x3<double,S3DDMatrix,S3DDVector>( mat4, mat, vec ) ;
	}
}

// 行列配列サンプリング
//////////////////////////////////////////////////////////////////////////////
bool S3DModelGLTFImporter::Sample4DMatrixsFromAccessor
	( SArray<S4DMatrix>& bufDst, const tinygltf::Accessor& accessor ) const
{
	const tinygltf::BufferView&
					bufView = m_model.bufferViews[accessor.bufferView] ;
	const tinygltf::Buffer&
					buffer = m_model.buffers[bufView.buffer] ;
	const uint8_t *	pbytData = ((const uint8_t *) &buffer.data.at(0))
						+ bufView.byteOffset + accessor.byteOffset ;
	const int		bytStride = accessor.ByteStride( bufView ) ;
	//
	if ( accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT )
	{
		return	false ;
	}
	if ( accessor.type == TINYGLTF_TYPE_MAT4 )
	{
		S4DMatrix *	pMatrix = bufDst.GetArray( accessor.count ) ;
		for ( size_t i = 0; i < accessor.count; i ++ )
		{
			S4DMatrix&	mDst = pMatrix[i] ;
			float32_t *	pSrc = (float32_t*) pbytData ;
			for ( int i = 0; i < 4; i ++ )
			{
				mDst.m[0][i] = pSrc[0] ;
				mDst.m[1][i] = pSrc[1] ;
				mDst.m[2][i] = pSrc[2] ;
				mDst.m[3][i] = pSrc[3] ;
				pSrc += 4 ;
			}
			pbytData += bytStride ;
		}
	}
	else
	{
		return	false ;
	}
	return	true ;
}

// 座標配列サンプリング
//////////////////////////////////////////////////////////////////////////////
bool S3DModelGLTFImporter::Sample4DVectorsFromAccessor
	( SArray<S4DVector>& bufDst, const tinygltf::Accessor& accessor ) const
{
	const tinygltf::BufferView&
					bufView = m_model.bufferViews[accessor.bufferView] ;
	const tinygltf::Buffer&
					buffer = m_model.buffers[bufView.buffer] ;
	const uint8_t *	pbytData = ((const uint8_t *) &buffer.data.at(0))
						+ bufView.byteOffset + accessor.byteOffset ;
	const int		bytStride = accessor.ByteStride( bufView ) ;
	//
	if ( accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT )
	{
		return	false ;
	}
	if ( accessor.type == TINYGLTF_TYPE_VEC4 )
	{
		S4DVector *	pvVertex = bufDst.GetArray( accessor.count ) ;
		for ( size_t i = 0; i < accessor.count; i ++ )
		{
			pvVertex[i] = *((S4DVector*) pbytData) ;
			pbytData += bytStride ;
		}
	}
	else
	{
		return	false ;
	}
	return	true ;
}

bool S3DModelGLTFImporter::Sample4DIVectorsFromAccessor
	( SArray<uint32_t>& bufDst, const tinygltf::Accessor& accessor ) const
{
	const tinygltf::BufferView&
					bufView = m_model.bufferViews[accessor.bufferView] ;
	const tinygltf::Buffer&
					buffer = m_model.buffers[bufView.buffer] ;
	const uint8_t *	pbytData = ((const uint8_t *) &buffer.data.at(0))
						+ bufView.byteOffset + accessor.byteOffset ;
	const int		bytStride = accessor.ByteStride( bufView ) ;
	//
	if ( accessor.type != TINYGLTF_TYPE_VEC4 )
	{
		return	false ;
	}
	if ( (accessor.componentType == TINYGLTF_COMPONENT_TYPE_INT)
		|| (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) )
	{
		uint32_t *	piVertex = bufDst.GetArray( accessor.count * 4 ) ;
		for ( size_t i = 0; i < accessor.count; i ++ )
		{
			uint32_t *	pSrc = (uint32_t*) pbytData ;
			piVertex[0] = pSrc[0] ;
			piVertex[1] = pSrc[1] ;
			piVertex[2] = pSrc[2] ;
			piVertex[3] = pSrc[3] ;
			piVertex += 4 ;
			pbytData += bytStride ;
		}
	}
	else if ( (accessor.componentType == TINYGLTF_COMPONENT_TYPE_SHORT)
			|| (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) )
	{
		uint32_t *	piVertex = bufDst.GetArray( accessor.count * 4 ) ;
		for ( size_t i = 0; i < accessor.count; i ++ )
		{
			uint16_t *	pSrc = (uint16_t*) pbytData ;
			piVertex[0] = pSrc[0] ;
			piVertex[1] = pSrc[1] ;
			piVertex[2] = pSrc[2] ;
			piVertex[3] = pSrc[3] ;
			piVertex += 4 ;
			pbytData += bytStride ;
		}
	}
	else if ( (accessor.componentType == TINYGLTF_COMPONENT_TYPE_BYTE)
			|| (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) )
	{
		uint32_t *	piVertex = bufDst.GetArray( accessor.count * 4 ) ;
		for ( size_t i = 0; i < accessor.count; i ++ )
		{
			uint8_t *	pSrc = (uint8_t*) pbytData ;
			piVertex[0] = pSrc[0] ;
			piVertex[1] = pSrc[1] ;
			piVertex[2] = pSrc[2] ;
			piVertex[3] = pSrc[3] ;
			piVertex += 4 ;
			pbytData += bytStride ;
		}
	}
	else
	{
		return	false ;
	}
	return	true ;
}

bool S3DModelGLTFImporter::Sample3DVectorsFromAccessor
	( SArray<S3DVector4>& bufDst,
			const tinygltf::Accessor& accessor ) const
{
	const tinygltf::BufferView&
					bufView = m_model.bufferViews[accessor.bufferView] ;
	const tinygltf::Buffer&
					buffer = m_model.buffers[bufView.buffer] ;
	const uint8_t *	pbytData = ((const uint8_t *) &buffer.data.at(0))
						+ bufView.byteOffset + accessor.byteOffset ;
	const int		bytStride = accessor.ByteStride( bufView ) ;
	//
	if ( accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT )
	{
		return	false ;
	}
	if ( accessor.type == TINYGLTF_TYPE_VEC3 )
	{
		S3DVector4 *	pvVertex = bufDst.GetArray( accessor.count ) ;
		for ( size_t i = 0; i < accessor.count; i ++ )
		{
			S3DVector *	pvSrc = (S3DVector*) pbytData ;
			S3DVector4	v = *pvSrc ;
			v.d = 0.0f ;
			pvVertex[i] = *pvSrc ;
			pbytData += bytStride ;
		}
	}
	else if ( accessor.type == TINYGLTF_TYPE_VEC4 )
	{
		S3DVector4 *	pvVertex = bufDst.GetArray( accessor.count ) ;
		for ( size_t i = 0; i < accessor.count; i ++ )
		{
			S3DVector4	v = *((S3DVector4*) pbytData) ;
			if ( v.d != 0.0f )
			{
				v *= 1.0f / v.d ;
				v.d = 0.0f ;
			}
			pvVertex[i] = v ;
			pbytData += bytStride ;
		}
	}
	else
	{
		return	false ;
	}
	return	true ;
}

bool S3DModelGLTFImporter::Sample2DVectorsFromAccessor
	( SArray<S2DVector>& bufDst,
			const tinygltf::Accessor& accessor ) const
{
	const tinygltf::BufferView&
					bufView = m_model.bufferViews[accessor.bufferView] ;
	const tinygltf::Buffer&
					buffer = m_model.buffers[bufView.buffer] ;
	const uint8_t *	pbytData = ((const uint8_t *) &buffer.data.at(0))
						+ bufView.byteOffset + accessor.byteOffset ;
	const int		bytStride = accessor.ByteStride( bufView ) ;
	//
	if ( accessor.componentType != TINYGLTF_COMPONENT_TYPE_FLOAT )
	{
		return	false ;
	}
	if ( accessor.type == TINYGLTF_TYPE_VEC2 )
	{
		S2DVector *	pvDst = bufDst.GetArray( accessor.count ) ;
		for ( size_t i = 0; i < accessor.count; i ++ )
		{
			S2DVector *	pvSrc = (S2DVector*) pbytData ;
			pvDst[i] = *pvSrc ;
			pbytData += bytStride ;
		}
	}
	else
	{
		return	false ;
	}
	return	true ;
}

bool S3DModelGLTFImporter::SampleIndicesFromAccessor
	( SArray<uint32_t>& bufDst,
			const tinygltf::Accessor& accessor ) const
{
	const tinygltf::BufferView&
					bufView = m_model.bufferViews[accessor.bufferView] ;
	const tinygltf::Buffer&
					buffer = m_model.buffers[bufView.buffer] ;
	const uint8_t *	pbytData = ((const uint8_t *) &buffer.data.at(0))
						+ bufView.byteOffset + accessor.byteOffset ;
	const int		bytStride = accessor.ByteStride( bufView ) ;
	//
	if ( (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT)
		|| (accessor.componentType == TINYGLTF_COMPONENT_TYPE_INT) )
	{
		uint32_t *	piDst = bufDst.GetArray( accessor.count ) ;
		for ( size_t i = 0; i < accessor.count; i ++ )
		{
			piDst[i] = *((uint32_t*)pbytData) ;
			pbytData += bytStride ;
		}
	}
	else if ( (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT)
			|| (accessor.componentType == TINYGLTF_COMPONENT_TYPE_SHORT) )
	{
		uint32_t *	piDst = bufDst.GetArray( accessor.count ) ;
		for ( size_t i = 0; i < accessor.count; i ++ )
		{
			piDst[i] = *((uint16_t*)pbytData) ;
			pbytData += bytStride ;
		}
	}
	else if ( (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE)
			|| (accessor.componentType == TINYGLTF_COMPONENT_TYPE_BYTE) )
	{
		uint32_t *	piDst = bufDst.GetArray( accessor.count ) ;
		for ( size_t i = 0; i < accessor.count; i ++ )
		{
			piDst[i] = *pbytData ;
			pbytData += bytStride ;
		}
	}
	else
	{
		return	false ;
	}
	return	true ;
}

bool S3DModelGLTFImporter::SampleFloatArrayFromAccessor
	( SArray<double>& bufDst,
			const tinygltf::Accessor& accessor ) const
{
	const tinygltf::BufferView&
					bufView = m_model.bufferViews[accessor.bufferView] ;
	const tinygltf::Buffer&
					buffer = m_model.buffers[bufView.buffer] ;
	const uint8_t *	pbytData = ((const uint8_t *) &buffer.data.at(0))
						+ bufView.byteOffset + accessor.byteOffset ;
	const int		bytStride = accessor.ByteStride( bufView ) ;
	//
	int	nElementCount = 1 ;
	switch ( accessor.type )
	{
	case	TINYGLTF_TYPE_SCALAR:
		nElementCount = 1 ;
		break ;
	case	TINYGLTF_TYPE_VEC2:
		nElementCount = 2 ;
		break ;
	case	TINYGLTF_TYPE_VEC3:
		nElementCount = 3 ;
		break ;
	case	TINYGLTF_TYPE_VEC4:
	case	TINYGLTF_TYPE_MAT2:
		nElementCount = 4 ;
		break ;
	case	TINYGLTF_TYPE_MAT3:
		nElementCount = 9 ;
		break ;
	case	TINYGLTF_TYPE_MAT4:
		nElementCount = 16 ;
		break ;
	default:
		return	false ;
	}
	double *	pfpDst = bufDst.GetArray( accessor.count * nElementCount ) ;
	//
	if ( accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT )
	{
		for ( size_t i = 0; i < accessor.count; i ++ )
		{
			float32_t *	pfpSrc = (float32_t*) pbytData ;
			for ( int j = 0; j < nElementCount; j ++ )
			{
				pfpDst[j] = pfpSrc[j] ;
			}
			pfpDst += nElementCount ;
			pbytData += bytStride ;
		}
	}
	else if ( accessor.componentType == TINYGLTF_COMPONENT_TYPE_DOUBLE )
	{
		for ( size_t i = 0; i < accessor.count; i ++ )
		{
			double *	pfpSrc = (double*) pbytData ;
			for ( int j = 0; j < nElementCount; j ++ )
			{
				pfpDst[j] = pfpSrc[j] ;
			}
			pfpDst += nElementCount ;
			pbytData += bytStride ;
		}
	}
	return	true ;
}

// 色ファクター取得
//////////////////////////////////////////////////////////////////////////////
SGLPalette S3DModelGLTFImporter::ConvertColorFactor
		( const tinygltf::Parameter& param, uint32_t argbDefault )
{
	SGLPalette	argbColor( argbDefault ) ;
	if ( param.number_array.size() >= 4 )
	{
		argbColor.argb.Red =
			(uint8_t) esl_clampi( (int) eslRoundR64ToLInt
							( param.number_array[0] * 255.0 ), 0, 0xFF ) ;
		argbColor.argb.Green =
			(uint8_t) esl_clampi( (int) eslRoundR64ToLInt
							( param.number_array[1] * 255.0 ), 0, 0xFF ) ;
		argbColor.argb.Blue =
			(uint8_t) esl_clampi( (int) eslRoundR64ToLInt
							( param.number_array[2] * 255.0 ), 0, 0xFF ) ;
		argbColor.argb.Alpha =
			(uint8_t) esl_clampi( (int) eslRoundR64ToLInt
							( param.number_array[3] * 255.0 ), 0, 0xFF ) ;
	}
	else if ( param.number_array.size() >= 3 )
	{
		argbColor.argb.Red =
			(uint8_t) esl_clampi( (int) eslRoundR64ToLInt
							( param.number_array[0] * 255.0 ), 0, 0xFF ) ;
		argbColor.argb.Green =
			(uint8_t) esl_clampi( (int) eslRoundR64ToLInt
							( param.number_array[1] * 255.0 ), 0, 0xFF ) ;
		argbColor.argb.Blue =
			(uint8_t) esl_clampi( (int) eslRoundR64ToLInt
							( param.number_array[2] * 255.0 ), 0, 0xFF ) ;
		argbColor.argb.Alpha = 0xFF ;
	}
	else if ( param.number_array.size() >= 1 )
	{
		argbColor.argb.Red =
			(uint8_t) esl_clampi( (int) eslRoundR64ToLInt
							( param.number_array[0] * 255.0 ), 0, 0xFF ) ;
		argbColor.argb.Green = argbColor.argb.Red ;
		argbColor.argb.Blue = argbColor.argb.Red ;
		argbColor.argb.Alpha = 0xFF ;
	}
	return	argbColor ;
}

// 文字列変換
//////////////////////////////////////////////////////////////////////////////
SString S3DModelGLTFImporter::ConvertString( const std::string& str )
{
	SString	strDst ;
	if ( sizeof(char) == sizeof(uint8_t) )
	{
		Charset::Decode
			( strDst, Charset::encodingUTF8,
					(const uint8_t*) str.c_str() ) ;
	}
	else
	{
		strDst = str.c_str() ;
	}
	return	strDst ;
}

// 進捗状況出力
//////////////////////////////////////////////////////////////////////////////
void S3DModelGLTFImporter::OnProgress
	( const wchar_t * pwszMsg, int nCurrent, int nTotal )
{
}

// エラー出力
//////////////////////////////////////////////////////////////////////////////
void S3DModelGLTFImporter::OutputError( const wchar_t * pwszMsg )
{
}

// 警告出力
//////////////////////////////////////////////////////////////////////////////
void S3DModelGLTFImporter::OutputWarning( const wchar_t * pwszMsg )
{
}
