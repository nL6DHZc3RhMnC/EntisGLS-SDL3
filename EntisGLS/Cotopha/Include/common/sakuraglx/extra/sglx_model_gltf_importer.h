
#pragma once

#include <sakuragl/sgl2d/sgl_spline_curve.h>


// needs tinygltf
#if	defined(__PLATFORM_ANDROID__)

// Android ではファイルシステムなしとして tinygltf ヘッダを通す
#define	TINYGLTF_NO_FS

// APP_CPPFLAGS := -std=c++14 以下ならコンパイルできるがデフォルトではないので……
#define	JSON_HAS_CPP_14

// ※ 内部でインクルードされる <filesystem> が
// Android NDK r25 でコンパイルエラーになるので回避
//#define	_LIBCPP_FILESYSTEM
//#define	_LIBCPP_HAS_NO_FILESYSTEM_LIBRARY

// ※Android NDK r27 では ofstream の定義がないとエラーが出るので
// NO_FILESYSTEM にせずにインクルードする
//#define	_LIBCPP_HAS_NO_FILESYSTEM			// NDK r27

#include <iostream>
#include <fstream>

#else
#define	STBI_MSC_SECURE_CRT

#endif

#undef	max
#include <tiny_gltf.h>


namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////////
	// glTF インポーター
	//////////////////////////////////////////////////////////////////////////////

	class	S3DModelGLTFImporter	: public S3DModelLoaderInterface
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DModelGLTFImporter, S3DModelLoaderInterface )
		// 構築関数
		S3DModelGLTFImporter( void ) ;
		// 消滅関数
		virtual ~S3DModelGLTFImporter( void ) ;

	public:	// S3DModelLoaderInterface
		// ファイル拡張子判定
		virtual bool IsMatchableFileExtension( const wchar_t * pszExt ) ;
		// MIME 判定
		virtual bool IsMatchableMIMEType( const wchar_t * pszMIME ) ;
		// モデルデータ読み込み
		virtual SGLError ReadModel
			( S3DModelBuffer & model, SSystem::SFileInterface & file ) ;

	protected:
		bool				m_flagModelLoaded ;
		tinygltf::Model		m_model ;
		S3DModelBuffer *	m_pDstModel ;
		S3DDMatrix			m_matCvtAxis ;
		S4DMatrix			m_mat4CvtAxis ;
		S4DMatrix			m_mat4ICvtAxis ;
		double				m_fpsFrameRatio ;

		struct	ImageEntry
		{
			const wchar_t *		pwszID ;
			SGLImageObject *	pDefault ;
		} ;
		struct	TextureEntry
		{
			ImageEntry *		pImageEntry ;
			uint64_t			nShadingFlags ;
			SGLSize				sizeTexture ;
		} ;
		struct	MaterialEntry
		{
			S3DMaterial *		pMaterial ;
			S2DVector			vUVScale ;
		} ;
		struct	NodeEntry
		{
			S4DDMatrix					mat4Node ;
			S4DDMatrix					mat4Local ;
			const tinygltf::Node *		pNode ;
			S3DModelBoneSpace *			pBone ;
			ssize_t						iMesh ;
			ssize_t						nMeshCount ;
			SSystem::SObjectArray
				<SSystem::SString>		aMeshIDs ;
			S3DDVector					vTranslation ;	// T
			S3DDQuaternion				qRotation ;		// R
			S3DDVector					vScale ;		// S
			SSystem::SArray<float32_t>	aWeights ;

			NodeEntry( void )
				: pNode(NULL), pBone(NULL), iMesh(-1), nMeshCount(0),
					vTranslation(0,0,0), qRotation(1,0,0,0), vScale(1,1,1) { }
		} ;
		class	MeshSkinData
		{
		public:
			SSystem::SArray<S4DVector>	m_bufWeights0 ;
			SSystem::SArray<uint32_t>	m_bufJoints0 ;
		} ;

		enum	TargetPath
		{
			pathInvalid,
			pathTranslation,
			pathRotation,
			pathScale,
			pathWeights,
		} ;
		enum	InterpolationMethod
		{
			interpolationLinear,
			interpolationStep,
			interpolationCatmullromSpline,
			interpolationCubicSpline,
		} ;
		class	AnimationTrack
		{
		public:
			NodeEntry *				m_pTargetNode ;
			TargetPath				m_pathTarget ;
			InterpolationMethod		m_interplation ;
			size_t					m_nElements ;
			SSystem::SArray<double>	m_bufKeyValues ;
			SSystem::SArray<double>	m_aKeyTimes ;			// [sec]
			double					m_secFirst ;
			double					m_secEnd ;
			SSystem::SObjectArray<SGLSplineCurvesN>
									m_splines ;
		public:
			AnimationTrack( void )
				: m_pTargetNode(NULL), m_pathTarget(pathInvalid),
					m_interplation(interpolationLinear),
					m_nElements(1), m_secFirst(0.0), m_secEnd(0.0) { }
		} ;
		class	AnimationSet
		{
		public:
			SSystem::SObjectArray<AnimationTrack>	m_aTracks ;
		} ;

		SSystem::SArray<ImageEntry>					m_aImages ;
		SSystem::SArray<TextureEntry>				m_aTextures ;
		SSystem::SArray<MaterialEntry>				m_aMaterials ;
		SSystem::SObjectArray<NodeEntry>				m_aNodes ;
		SSystem::SPointerArray<NodeEntry>			m_aMeshs ;
		SSystem::SArray<size_t>						m_aRefMeshNode ;
		SSystem::SObjectArray<MeshSkinData>			m_aMeshSkins ;
		SSystem::SStrSortObjectArray<AnimationSet>	m_ssoaAnimations ;

	public:
		// 読み込み処理 .gltf
		SGLError LoadGLTFText( const wchar_t * pwszFilePath ) ;
		// 読み込み処理 .glb
		SGLError LoadGLTFBinary( const wchar_t * pwszFilePath ) ;
		SGLError LoadGLTFBinaryOnMemory
			( const unsigned char *bytes, const unsigned int length ) ;
		// 変換
		SGLError ConvertTo( S3DModelBuffer * pModel ) ;
		// 画像変換
		SGLError ConvertImage
			( size_t iImage, const tinygltf::Image& image ) ;
		// テクスチャ変換
		SGLError ConvertTexture
			( size_t iTexture, const tinygltf::Texture& texture ) ;
		// マテリアル変換
		SGLError ConvertMaterial
			( size_t iMaterial, const tinygltf::Material& material ) ;
		// ノード内のメッシュ変換
		SGLError ConvertNode
			( int iNode, const tinygltf::Node& node,
				const S4DDMatrix& mat4Base, S3DModelBoneSpace& boneParent ) ;
		SGLError ConvertMeshOfNode
			( int iNode, const tinygltf::Node& node, const S4DDMatrix& mat4Node ) ;
		SGLError ConvertMorphTargets
			( int iDstMesh, const wchar_t * pwszMeshID,
					const S3DMatrix& matNode,
					const tinygltf::Primitive& primitive ) ;
		static void AddVectorArray
			( SSystem::SArray<S3DVector4>& bufDst,
					const S3DVector4 * pvSrc, size_t nVertexCount ) ;
		// ボーン関連付け構築
		SGLError BuildAllBoneRelations( void ) ;
		SGLError BuildBoneRelations( S3DModelBoneSpace& bone ) ;
		SGLError MakeSkinsOfBone
			( S3DModelBoneSpace& bone,
					NodeEntry * pneNode, size_t iBoneNode ) ;
		// ボーンのノードを取得
		NodeEntry * GetNodeOfBone( S3DModelBoneSpace * pBone, size_t& iBone ) const ;
		// メッシュのノードを取得
		NodeEntry * GetNodeOfMesh( const wchar_t * pwszMeshID, size_t& iMesh ) const ;
		// VRM 拡張変換
		SGLError ConvertVRMExtensions( void ) ;
		// VRM 拡張 blendShapeMaster.blendShapeGroups
		SGLError ConvertVRMBlendShape( const tinygltf::Value& valVRM ) ;
		SGLError ConvertVRMBlendShapeBinds
					( S3DModelPose& pose, const tinygltf::Value& valBinds ) ;
		// VRM 拡張 secondaryAnimation.boneGroups
		SGLError ConvertVRMSecondaryAnimation( const tinygltf::Value& valVRM ) ;
		SGLError ConvertVRMSecondaryAnimationBones( const tinygltf::Value& valBoneGroups ) ;
		static void SetBonePhysMaterials
				( S3DModelBoneSpace * pBone,
					const S3DModelBoneSpace::PhysMaterial& physMaterial ) ;
		SGLError ConvertVRMSecondaryAnimationColliders( const tinygltf::Value& valColliderGroups ) ;
		// json 数値要素取得
		static double GetJsonPropNumberAs
			( const tinygltf::Value& val, const char * pszName, double numDef ) ;
		static int GetJsonPropIntAs
			( const tinygltf::Value& val, const char * pszName, int nDef ) ;
		// アニメーション変換
		SGLError ConvertAnimation
			( size_t iAnimation, const tinygltf::Animation& animations ) ;
		void SampleAnimationFrame( const AnimationTrack& aniTrack, double t ) ;
		// ノードの変換行列取得
		void GetNodeTransformation
			( S4DDMatrix& mat4, const tinygltf::Node& node ) const ;
		// 行列配列サンプリング
		bool Sample4DMatrixsFromAccessor
			( SSystem::SArray<S4DMatrix>& bufDst,
					const tinygltf::Accessor& accessor ) const ;
		// 座標配列サンプリング
		bool Sample4DVectorsFromAccessor
			( SSystem::SArray<S4DVector>& bufDst,
					const tinygltf::Accessor& accessor ) const ;
		bool Sample4DIVectorsFromAccessor
			( SSystem::SArray<uint32_t>& bufDst,
					const tinygltf::Accessor& accessor ) const ;
		bool Sample3DVectorsFromAccessor
			( SSystem::SArray<S3DVector4>& bufDst,
					const tinygltf::Accessor& accessor ) const ;
		bool Sample2DVectorsFromAccessor
			( SSystem::SArray<S2DVector>& bufDst,
					const tinygltf::Accessor& accessor ) const ;
		bool SampleIndicesFromAccessor
			( SSystem::SArray<uint32_t>& bufDst,
					const tinygltf::Accessor& accessor ) const ;
		bool SampleFloatArrayFromAccessor
			( SSystem::SArray<double>& bufDst,
					const tinygltf::Accessor& accessor ) const ;
		// 色ファクター取得
		static SGLPalette ConvertColorFactor
			( const tinygltf::Parameter& param, uint32_t argbDefault = 0 ) ;
		// 文字列変換
		static SSystem::SString ConvertString( const std::string& str ) ;

	protected:
		// 進捗状況出力
		virtual void OnProgress
			( const wchar_t * pwszMsg, int nCurrent, int nTotal ) ;
		// エラー出力
		virtual void OutputError( const wchar_t * pwszMsg ) ;
		// 警告出力
		virtual void OutputWarning( const wchar_t * pwszMsg ) ;

	} ;

}

