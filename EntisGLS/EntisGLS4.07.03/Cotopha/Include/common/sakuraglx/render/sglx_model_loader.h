
#if	!defined(__SAKURAGLX_MODEL_LOADER_H__)
#define	__SAKURAGLX_MODEL_LOADER_H__	1

#include <sakura/ssys_queue_buffer.h>
#include <sakuragl/sgl3d/sglh3d_stddef.h>
#include <sakuraglx/render/sglx_model_buffer.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// EntisGLS3 互換モデルファイル・ローダー
	//////////////////////////////////////////////////////////////////////////

	class	S3DModelMDFLoader	: public S3DModelLoaderInterface
	{
	public:
		// 表面属性
		struct	SURFACE_ATTRIBUTE
		{
			uint32_t	dwShadingFlags ;	// シェーディングフラグ
			uint32_t	dwReserved ;
			S3DColor	rgbaColor ;			// 基本色
			uint32_t	pTextureImage ;		// テクスチャ画像
			uint32_t	pLuminousImage ;	// 発光テクスチャ
			float32_t	rThresholdZ ;		// 縮小画像に切り替えるための閾値
			uint32_t	nSmallScale ;		// 縮小画像のスケール（1/2^n）
			uint32_t	pSmallImage ;		// 縮小テクスチャ画像
			uint32_t	pSmallLuminous ;	// 縮小発光テクスチャ
			uint32_t	nTextureApply ;		// テクスチャ適用度（未使用）
			uint32_t	nLuminousApply ;	// 発光テクスチャ適用度（0 の時 100%）
			int32_t		nAmbient ;			// 環境光 (x256)
			int32_t		nDiffusion ;		// 拡散光 (x256)
			int32_t		nSpecular ;			// 反射光 (x256)
			int32_t		nSpecularSize ;		// 反射光の鋭さ（0～256）
			int32_t		nTransparency ;		// 透明度 (x256)
			int32_t		nDeepness ;			// 透明深度係数 (x256)
			S3DColor	rgbaShade ;			// 影色 (ver.3.09 以降)
			uint32_t	nReflection ;		// 反射率 (x256)
			float32_t	nRefraction ;		// 屈折率 (-1.0)（0.0 の時屈折率 1.0）
		} ;
		// プリミティブ・ヘッダ
		enum	PrimitiveTypeFlag
		{
			flagFlatPolygon				= 0x00000000,
			flagSmoothPolygon			= 0x00000001,
			flagTexturePolygon			= 0x00000002,
			flagVertexColorPolygon		= 0x00000004,
			maskPrimitiveFlag			= 0x00000007,
			typeInfinitePlane			= 0x0000000A,
			flagMeshPolygon				= 0x00000010,
			typeImagePrimitive			= 0x00000100,
		} ;
		struct	PRIMITIVE_HEADER
		{
			uint32_t	dwTypeFlag ;
			uint32_t	pSurfaceAttr ;
			uint32_t	dwVertexCount ;
			uint32_t	dwDataSize ;
		} ;
		// ポーション
		struct	MESH_PORTION
		{
			uint32_t	iMesh ;
			uint32_t	nCount ;
		} ;
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DModelMDFLoader, S3DModelLoaderInterface )
		// ファイル拡張子判定
		virtual bool IsMatchableFileExtension( const wchar_t * pszExt ) ;
		// MIME 判定
		virtual bool IsMatchableMIMEType( const wchar_t * pszMIME ) ;
		// モデルデータ読み込み
		virtual SGLError ReadModel
			( S3DModelBuffer & model, SSystem::SFileInterface & file ) ;
		SGLError ReadModelChunkFile
			( S3DModelBuffer & model, SSystem::SChunkFile & file ) ;

	protected:
		// テクスチャレコード読み込み
		virtual SGLError ReadTextureRecord
			( S3DModelBuffer & model, SSystem::SChunkFile& file ) ;
		// 表面属性レコード読み込み
		virtual SGLError ReadSurfaceRecord
			( S3DModelBuffer & model, SSystem::SChunkFile& file ) ;
		// モデルレコード読み込み
		virtual SGLError ReadModelRecord
			( S3DModelBuffer & model, SSystem::SChunkFile& file ) ;
		// 拡張レコード読み込み
		virtual SGLError ReadUserRecord
			( S3DModelBuffer & model, SSystem::SChunkFile& file ) ;

	public:
		// テクスチャのフォーマットやα値からシェーディングヒントフラグを評価する
		static uint64_t TextureHintOfAlpha( SGLImageObject * pTexture ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// EntisGLS4 標準モデルファイル・ローダー
	//////////////////////////////////////////////////////////////////////////

	class	S3DStdModelLoader	: public S3DModelLoaderInterface
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DStdModelLoader, S3DModelLoaderInterface )
		// 構築関数
		S3DStdModelLoader( void ) ;
		// 消滅関数
		virtual ~S3DStdModelLoader( void ) ;

	public:
		// ファイル拡張子判定
		virtual bool IsMatchableFileExtension( const wchar_t * pszExt ) ;
		// MIME 判定
		virtual bool IsMatchableMIMEType( const wchar_t * pszMIME ) ;
		// モデルデータ読み込み
		virtual SGLError ReadModel
			( S3DModelBuffer & model, SSystem::SFileInterface & file ) ;

	public:
		// テクスチャ情報フラグ
		enum	TextureFlag
		{
			textureMipmap		= 0x00000001,
			textureCompressed	= 0x00000002,
		} ;
		struct	TextureInfo
		{
			uint32_t	nFlags ;
			uint32_t	nReserved ;
		} ;
		// 表面属性テクスチャ情報
		struct	SurfaceTextureEntry
		{
			uint32_t	iTexture ;
			uint32_t	nFlags ;
			float32_t	fpApply ;
			float32_t	fpParam1 ;
			uint32_t	nReserved[4] ;
		} ;
		struct	SurfaceTextures
		{
			uint32_t			nCount ;
			uint32_t			nReserved[3] ;
			SurfaceTextureEntry	txtEntries[15] ;
		} ;
		// メッシュ情報
		struct	TriangleMeshInfo
		{
			uint32_t	iMaterial ;
			uint32_t	countPolygon ;
			uint32_t	countVertex ;
			uint32_t	nFlags ;
			float32_t	fpSubMeshDensity ;
			uint32_t	typePrimitive ;
			uint32_t	nExAttrElements ;
			uint32_t	nReserved ;
		} ;
		enum	TriangleMeshExtendFlag
		{
			meshFlagSubMeshDensity	= 0x0001,
			meshFlagPrimitiveType	= 0x0002,
		} ;
		// モーフターゲットメッシュ情報
		struct	MorphingMeshInfo
		{
			uint32_t	iMeshID ;
			uint32_t	countVertex ;
			uint32_t	countRelMesh ;
			uint32_t	nReserved[5] ;
		} ;
		// 分割メッシュ情報エントリヘッダ
		struct	MeshDivisionInfo
		{
			uint32_t	iMeshID ;
			uint32_t	countDivision ;
			uint32_t	countMorphList ;
			uint32_t	nReserved[5] ;
		} ;
		// ボーン情報
		struct	BoneInfo
		{
			uint32_t		iBoneID ;
			uint32_t		iParentID ;
			S3DVector		vBoneBase ;			// ボーン付け根位置（ローカル座標）
			S3DVector		vBoneHandle ;		// ボーンハンドル（相対座標）
			uint32_t		flagsBone ;			// complex of enum S3DModelBoneSpace::BoneFlag
			uint32_t		iMaterialRefID ;
			S3DModelBoneSpace::PhysMaterial
							physMaterial ;		// 物理属性
			S4DMatrix		mat4OrgBone ;		// 元モデルの変換行列
		} ;
		// 物理演算・被影響ボーン
		struct	PhysEffectiveBoneInfo
		{
			uint32_t	nBytes ;
			uint32_t	nType ;
			uint32_t	nReserved ;
			uint32_t	iBoneID ;
			double		fpWeight ;
		} ;
		// ウェイトマップ情報
		enum	WeightMapInfoFlag
		{
			flagIndexedWeightMap	= 0x0001,
		} ;
		struct	WeightMapInfo
		{
			uint32_t	iMesh ;
			uint32_t	nCount ;
			uint32_t	nFlags ;		//  enum WeightMapInfoFlag
			uint32_t	nReserved ;
			S4DDMatrix	matIMesh ;
			S4DDMatrix	matRelMesh ;
		} ;
		class	IndexedWeightMap
		{
		public:
			SSystem::SArray<float32_t>	bufWeight ;
			SSystem::SArray<uint32_t>	bufIndex ;
		} ;

	public:
		// テクスチャ読み込み
		virtual SGLError ReadTextureChunk
			( S3DModelBuffer & model, SSystem::SChunkFile& file ) ;
		// 表面属性読み込み
		virtual SGLError ReadMaterialChunk
			( S3DModelBuffer & model, SSystem::SChunkFile& file ) ;
		// メッシュ読み込み
		virtual SGLError ReadMeshChunk
			( S3DModelBuffer & model, SSystem::SChunkFile& file ) ;
		// メッシュグループ情報読み込み
		virtual SGLError ReadMeshGroupChunk
			( S3DModelBuffer & model, SSystem::SChunkFile& file ) ;
		// モーフターゲット読み込み
		virtual SGLError ReadMeshMorphTargetChunk
			( S3DModelBuffer & model, SSystem::SChunkFile& file ) ;
		// 分割メッシュ情報読み込み
		virtual SGLError ReadMeshDivisionInfoChunk
			( S3DModelBuffer & model, SSystem::SChunkFile& file ) ;
		// ボーン読み込み
		virtual SGLError ReadBoneChunk
			( S3DModelBuffer & model, SSystem::SChunkFile& file ) ;
		// マーカー情報読み込み
		virtual SGLError ReadMarkerInfoChunk
			( S3DModelBuffer & model, SSystem::SChunkFile& file ) ;
		// クロスシミュレーターメッシュ情報読み込み
		virtual SGLError ReadClothMeshInfoChunk
			( S3DModelBuffer & model, SSystem::SChunkFile& file ) ;
		// ポーズライブラリ読み込み
		virtual SGLError ReadPosesChunk
			( S3DModelBuffer & model, SSystem::SChunkFile& file ) ;
		// メタ情報読み込み
		virtual SGLError ReadMetaInfoChunk
			( S3DModelBuffer & model, SSystem::SChunkFile& file ) ;
		// コンポジション読み込み
		virtual SGLError ReadSceneChunk
			( S3DModelBuffer & model, SSystem::SChunkFile& file ) ;
		// 拡張データ読み込み
		virtual SGLError ReadUserChunk
			( S3DModelBuffer & model, SSystem::SChunkFile& file ) ;

	public:
		// 文字列配列読み込み
		static void ReadStrings
			( SSystem::SObjectArray<SSystem::SString>& aStrings,
										SSystem::SFileInterface& file ) ;
		// 指標付きウェイトマップ展開
		static void ExpandIndexedWeightMap
			( SSystem::SArray<float32_t>& bufWeight,
						const IndexedWeightMap& iwmSrcMap ) ;
		// 指標付きウェイトマップ精製
		static void MakeIndexedWeightMap
			( IndexedWeightMap& iwmDstMap,
					const float32_t * pfpWeight, size_t nCount ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// EntisGLS4 標準 XML 形式モデル・ローダー
	//////////////////////////////////////////////////////////////////////////

	class	S3DStdXMLModelLoader	: public S3DModelLoaderInterface
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DStdXMLModelLoader, S3DModelLoaderInterface )
		// 構築関数
		S3DStdXMLModelLoader( void ) ;
		// 消滅関数
		virtual ~S3DStdXMLModelLoader( void ) ;

	public:
		// ファイル拡張子判定
		virtual bool IsMatchableFileExtension( const wchar_t * pszExt ) ;
		// MIME 判定
		virtual bool IsMatchableMIMEType( const wchar_t * pszMIME ) ;
		// モデルデータ読み込み
		virtual SGLError ReadModel
			( S3DModelBuffer & model, SSystem::SFileInterface & file ) ;
		// モデルデータ解釈
		virtual SGLError ParseModel
			( S3DModelBuffer & model,
				SSystem::SXMLDocument & xmlModel,
				SSystem::SFileOpener & opener ) ;

	protected:
		class	BoneRelInfo
		{
		public:
			S3DModelBoneSpace *	m_pBone ;
			SSystem::SString	m_strParentID ;
		public:
			BoneRelInfo( void ) : m_pBone(NULL) {}
			BoneRelInfo( const BoneRelInfo& bri )
				: m_pBone(bri.m_pBone), m_strParentID(bri.m_strParentID) {}
		} ;

	public:
		// テクスチャ読み込み
		virtual SGLError ParseTextureTag
			( S3DModelBuffer & model,
				SSystem::SXMLDocument & xmlTexture,
				SSystem::SFileOpener & opener ) ;
		// 表面属性読み込み
		virtual SGLError ParseMaterialTag
			( S3DModelBuffer & model, SSystem::SXMLDocument & xmlMaterial ) ;
		// メッシュ読み込み
		virtual SGLError ParseMeshTag
			( S3DModelBuffer & model, SSystem::SXMLDocument & xmlMesh ) ;
		// メッシュグループ情報読み込み
		virtual SGLError ParseMeshGroupTag
			( S3DModelBuffer & model, SSystem::SXMLDocument & xmlGroup ) ;
		// モーフターゲット読み込み
		virtual SGLError ParseMorphMeshTag
			( S3DModelBuffer & model, SSystem::SXMLDocument & xmlMorph ) ;
		// 分割メッシュ情報読み込み
		virtual SGLError ParseMeshDivTag
			( S3DModelBuffer & model, SSystem::SXMLDocument & xmlMeshDiv ) ;
		// ボーン読み込み
		virtual SGLError ParseBoneTag
			( S3DModelBuffer & model, SSystem::SXMLDocument & xmlBone ) ;
		virtual SGLError ParseBoneTagPhysics
			( S3DModelBuffer & model, SSystem::SXMLDocument & xmlBone ) ;
		static void ParseBonePhysParameters
			( S3DModelBoneSpace* pBone, const SSystem::SXMLDocument& xmlTag ) ;
		static void ParseBonePhysMaterial
			( S3DModelBoneSpace::PhysMaterial& physMaterial,
							const SSystem::SXMLDocument& xmlTag ) ;
		// マーカー情報読み込み
		virtual SGLError ParseMarkerInfoTag
			( S3DModelBuffer & model, SSystem::SXMLDocument & xmlMarker ) ;
		// クロスシミュレーターメッシュ情報読み込み
		virtual SGLError ParseClothMeshInfoTag
			( S3DModelBuffer & model, SSystem::SXMLDocument & xmlCloth ) ;
		// ポーズライブラリ読み込み
		virtual SGLError ParsePosesTag
			( S3DModelBuffer & model, SSystem::SXMLDocument & xmlPose ) ;
		// 拡張データ読み込み
		virtual SGLError ParseUserTag
			( S3DModelBuffer & model, SSystem::SXMLDocument & xmlTag ) ;

	public:
		// 4x4 行列解釈
		static void ParseMatrix4x4
			( S4DMatrix& mat4, const wchar_t * pwsz4x4 ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// モデル・ビルダ
	//////////////////////////////////////////////////////////////////////////

	class	S3DModelBuilder	: public S3DModelLoaderInterface
	{
	public:
		// メッシュ情報
		class	MeshEntry
		{
		public:
			enum	MeshType
			{
				meshIndexedTriangleList,
				meshTriangleStrip,
			} ;
			MeshType					m_typeMesh ;
			S3DMaterial *				m_pMaterial ;
			size_t						m_countPolygon ;
			size_t						m_countVertex ;
			SSystem::SArray<S3DVector4>	m_bufVertex ;
			SSystem::SArray<S3DVector4>	m_bufNormal ;
			SSystem::SArray<S2DVector>	m_bufUVMap ;
			SSystem::SArray<S3DColor>	m_bufColor ;
			SSystem::SArray<uint32_t>	m_bufIndex ;

		public:
			// 構築関数
			MeshEntry( void ) ;
			MeshEntry( const MeshEntry& mesh ) ;
			// 消滅関数
			~MeshEntry( void ) ;

		public:
			// 法線の生成
			void MakeNormal( double cosLimit ) ;
		} ;
		// モーフターゲット
		class	MorphMeshEntry	: public S3DModelBuffer::MorphTargetMesh
		{
		public:
			SSystem::SString		m_strID ;
			SSystem::SArray<size_t>	m_arrRelMesh ;
		public:
			MorphMeshEntry( void ) {}
			MorphMeshEntry( const MorphMeshEntry& mme )
				: MorphTargetMesh( mme ),
					m_strID( mme.m_strID ), m_arrRelMesh( mme.m_arrRelMesh ) {}
		} ;
		// ボーン・ウェイトマップ
		class	WeightMap
		{
		public:
			size_t						m_iTargetMesh ;
			S4DMatrix					m_matIMesh ;
			S4DMatrix					m_matRelMesh ;
			SSystem::SArray<float32_t>	m_bufWeight ;
		public:
			WeightMap( void ) : m_iTargetMesh( 0 ) {}
			WeightMap( const WeightMap& wm )
				: m_iTargetMesh( wm.m_iTargetMesh ),
					m_matIMesh( wm.m_matIMesh ),
					m_matRelMesh( wm.m_matRelMesh ),
					m_bufWeight( wm.m_bufWeight ) {}
		} ;
		// 結合ウェイトマップ
		class	FlatWeightMap
		{
		public:
			size_t						m_iVertex ;
			size_t						m_iNormal ;
			SSystem::SArray<float32_t>	m_bufWeight ;
		public:
			FlatWeightMap( void ) : m_iVertex( 0 ), m_iNormal( 0 ) {}
			FlatWeightMap( const FlatWeightMap& fwm )
				: m_iVertex( fwm.m_iVertex ),
					m_iNormal( fwm.m_iNormal ),
					m_bufWeight( fwm.m_bufWeight ) {}
		} ;
		// ボーン情報
		class	BoneInfo
		{
		public:
			SSystem::SString	m_strParentID ;		// 親ボーン
			S3DDVector			m_vBonePos ;		// 基準座標（ローカル座標）
			S3DDVector			m_vHandle ;			// ハンドル（相対座標）
			uint32_t			m_flagsBone ;		// complex of enum S3DModelBoneSpace::BoneFlag
			float32_t			m_fpPhysHardness ;	// ボーンの硬さ（物理演算）
			S3DModelBoneSpace::PhysMaterial
								m_physMaterial ;	// 物理属性
			SSystem::SObjectArray<WeightMap>
								m_arrWeightMaps ;	// ウェイトマップ
		public:
			BoneInfo( void )
				: m_flagsBone( 0 ), m_fpPhysHardness( 0.5 ) { }
		} ;

	public:
		// メッシュリスト
		SSystem::SObjectArray<MeshEntry>	m_meshs ;

		// ライブラリ
		S3DTextureLibrary	m_textures ;
		S3DMaterialLibrary	m_materials ;

		// メッシュ・グループ
		SSystem::SStrSortArray<S3DModelBuffer::MeshGroup>	m_groups ;

		// モーフターゲット
		SSystem::SObjectArray<MorphMeshEntry>	m_morphings ;

		// ボーン
		SSystem::SStrSortObjectArray<BoneInfo>	m_bones ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DModelBuilder, S3DModelLoaderInterface )
		// 構築関数
		S3DModelBuilder( void ) ;
		S3DModelBuilder( const S3DModelBuilder & builder ) ;
		// 消滅関数
		virtual ~S3DModelBuilder( void ) ;

	public:
		// モデルデータ読み込み＆構築 (S3DModelLoaderInterface)
		virtual SGLError ReadModel
			( S3DModelBuffer & model, SSystem::SFileInterface & file ) ;
		// モデルデータ読み込み
		virtual SGLError ReadModel
			( SSystem::SFileInterface& file,
				SSystem::SParserErrorInterface& perr ) = 0 ;
		// モデル構築
		virtual SGLError BuildModel( S3DModelBuffer& model ) ;

	public:
		// ポリゴンリストをレンダリングバッファに追加
		virtual MeshEntry * AddIndexedTriangleList
			( S3DMaterial * pMaterial, uint32_t nFlags,
				size_t countPolygon, size_t countVertex,
				const S3DVector4 * pvVertex,
				const S3DVector4 * pvNormal,
				const S2DVector * pvUVMap,
				const S3DColor * pColor,
				const uint32_t * pIndexedList ) ;
		// トライアングルストリップをレンダリングバッファに追加
		virtual MeshEntry * AddTriangleStrip
			( S3DMaterial * pMaterial, uint32_t nFlags,
				size_t countTriangleStrip,
				const S3DVector4 * pvVertex,
				const S3DVector4 * pvNormal,
				const S2DVector * pvUVMap, const S3DColor * pColor ) ;

	public:
		// メッシュ・グループ名正規化（未使用名であることを保証する）
		const SSystem::SString&
				NormalizeGroupName( SSystem::SString& strName ) const ;
		// 範囲の一致するメッシュグループを検索する
		ssize_t FindMeshGroup
			( size_t iFirstMesh, size_t nMeshCount ) const ;

	public:
		// 数値配列解釈
		static size_t ParseFloatArray
			( SSystem::SArray<float>& aFloat,
				SSystem::SStringParser& sparsList, size_t nCount ) ;
		static size_t ParseIntArray
			( SSystem::SArray<int>& aInt,
				SSystem::SStringParser& sparsList, size_t nCount ) ;
		// 16進数配列デコード
		static size_t ParseHexBinaryArray
			( SSystem::SQueueBuffer& bufBin, SSystem::SStringParser& sparsHex ) ;
		// ボーン・ウェイトマップを統合
		static void MergeBoneWeightMap
			( FlatWeightMap& fwmDst, const S3DModelBuffer& model,
				const WeightMap*const* ppWeightMaps, size_t nWeightMaps ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// Sahde xml モデルデータ・ビルダ
	//////////////////////////////////////////////////////////////////////////

	class	S3DShadeXMLLoader	: public S3DModelBuilder
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DShadeXMLLoader, S3DModelBuilder )
		// 構築関数
		S3DShadeXMLLoader( void ) ;
		// 消滅関数
		virtual ~S3DShadeXMLLoader( void ) ;

	public:
		// ファイル拡張子判定
		virtual bool IsMatchableFileExtension( const wchar_t * pszExt ) ;
		// MIME 判定
		virtual bool IsMatchableMIMEType( const wchar_t * pszMIME ) ;
		// ファイルを読み込む
		virtual SGLError LoadModel
			( const wchar_t * pwszFilePath,
				SSystem::SParserErrorInterface& perr ) ;
		virtual SGLError ReadModel
			( SSystem::SFileInterface& file,
				SSystem::SParserErrorInterface& perr ) ;
		// 解釈
		SGLError ParseModel
			( SSystem::SXMLDocument& xmlDoc,
				SSystem::SFileOpener& opener,
				SSystem::SParserErrorInterface& perr ) ;

	protected:
		struct	MappingInfo
		{
			bool	flagFlipX ;
			bool	flagFlipY ;
			bool	flagSwapXY ;
			int		nRepeatX ;
			int		nRepeatY ;
			int		iMappingBy ;

			MappingInfo( void )
				: flagFlipX(false), flagFlipY(false), flagSwapXY(false),
						nRepeatX(1), nRepeatY(1), iMappingBy(0) {}
		} ;
		SSystem::SObjectArray<SSystem::SString>		m_aMasterSurface ;
		SSystem::SStrSortArray<MappingInfo>			m_mapMappingInfo ;
		SSystem::SStrSortArray<SSystem::SString>	m_mapImageId ;
		SSystem::SStrSortArray<SSystem::SString>	m_mapImageObjPath ;

	protected:
		// 一般パート処理
		SGLError ParsePart
			( const S4DMatrix& matPart,
				S3DMaterial * pMaterial,
				SSystem::SXMLDocument& xmlPart,
				SSystem::SParserErrorInterface& perr ) ;
		// パート表面属性処理
		S3DMaterial * ParsePartSurface
			( S3DMaterial * pMaterial,
				SSystem::SXMLDocument& xmlPart ) ;
		// <polygon_mesh> ポリゴンメッシュ処理
		SGLError ParsePolygonMesh
			( const S4DMatrix& matPart,
				S3DMaterial * pMaterial,
				SSystem::SXMLDocument& xmlPolygonMesh,
				SSystem::SParserErrorInterface& perr ) ;
		// <master_image> パート処理
		SGLError ParseAllMasterImage
			( SSystem::SXMLDocument& xmlPart,
				SSystem::SFileOpener& opener,
				SSystem::SParserErrorInterface& perr ) ;
		SGLError ParseMasterImage
			( SSystem::SXMLDocument& xmlMasterImage,
				SSystem::SFileOpener& opener,
				SSystem::SParserErrorInterface& perr ) ;
		// <master_surface> パート処理
		SGLError ParseAllMasterSurface
			( SSystem::SXMLDocument& xmlPart ) ;
		SGLError ParseMasterSurface
			( SSystem::SXMLDocument& xmlMasterSurface ) ;

	protected:
		// <part><surface> 処理
		S3DMaterial * ParseSurfaceAttribute
			( SSystem::SString& strSurfaceID,
				SSystem::SXMLDocument& xmlSurface ) ;
		// <polygon_mesh><vertices> 処理
		SGLError ParsePolygonVertices
			( const S4DMatrix& matPart,
				SSystem::SXMLDocument& xmlVertices,
				SSystem::SArray<S3DVector4>& bufVertex ) ;
		// <polygon_mesh><faces> 処理
		SGLError ParsePolygonFaces
			( SSystem::SXMLDocument& xmlFaces,
				const MappingInfo * pMappingInf,
				SSystem::SArray<S3DVector4>& bufVertex,
				SSystem::SArray<S2DVector>& bufUVMap,
				SSystem::SArray<uint32_t>& bufIndex ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// EntisGLS4 標準モデルファイル・セーバー
	//////////////////////////////////////////////////////////////////////////

	class	S3DStdModelSaver	: public S3DModelSaverInterface
	{
	protected:
		SSystem::SString	m_strImageMIME ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DStdModelSaver, S3DModelSaverInterface )
		// 構築関数
		S3DStdModelSaver( void ) ;
		// 消滅関数
		virtual ~S3DStdModelSaver( void ) ;
		// MIME 判定
		virtual bool IsMatchableMIMEType( const wchar_t * pszMIME ) ;
		// テクスチャ画像保存形式設定
		virtual SGLError SetImageFormat
			( const wchar_t * pwszMIME,
				const wchar_t * pwszExt = NULL,
				const SGLImageEncoderInterface::Options * pOpt = NULL ) ;
		// モデルデータ読み込み
		virtual SGLError WriteModel
			( SSystem::SFileInterface & file, S3DModelBuffer & model ) ;

	public:
		// テクスチャ書き出し
		virtual SGLError WriteTextureChunk
			( SSystem::SChunkFile & file, S3DModelBuffer & model ) ;
		// 表面属性書き出し
		virtual SGLError WriteMaterialChunk
			( SSystem::SChunkFile & file, S3DModelBuffer & model ) ;
		// メッシュ書き出し
		virtual SGLError WriteMeshChunk
			( SSystem::SChunkFile & file, S3DModelBuffer & model ) ;
		// メッシュグループ情報書き出し
		virtual SGLError WriteMeshGroupChunk
			( SSystem::SChunkFile & file, S3DModelBuffer & model ) ;
		// モーフターゲット書き出し
		virtual SGLError WriteMeshMorphTargetChunk
			( SSystem::SChunkFile& file, S3DModelBuffer & model ) ;
		// 分割メッシュ情報書き出し
		virtual SGLError WriteMeshDivisionInfoChunk
			( SSystem::SChunkFile& file, S3DModelBuffer & model ) ;
		// ボーン書き出し
		virtual SGLError WriteBoneChunk
			( SSystem::SChunkFile & file, S3DModelBuffer & model ) ;
		// マーカー情報書き出し
		virtual SGLError WriteMarkerInfoChunk
			( SSystem::SChunkFile & file, S3DModelBuffer & model ) ;
		// クロスシミュレーターメッシュ情報書き出し
		virtual SGLError WriteClothMeshInfoChunk
			( SSystem::SChunkFile & file, S3DModelBuffer & model ) ;
		// ポーズ書き出し
		virtual SGLError WritePosesChunk
			( SSystem::SChunkFile & file, S3DModelBuffer & model ) ;
		// メタ情報書き出し
		virtual SGLError WriteMetaInfoChunk
			( SSystem::SChunkFile & file, S3DModelBuffer & model ) ;
		// コンポジション書き出し
		virtual SGLError WriteSceneChunk
			( SSystem::SChunkFile & file, S3DModelBuffer & model ) ;
		// 拡張データ書き出し
		virtual SGLError WriteUserChunk
			( SSystem::SChunkFile & file, S3DModelBuffer & model ) ;

	public:
		// 文字列配列書き出し
		static void WriteStrings
			( SSystem::SFileInterface& file,
				const SSystem::SObjectArray<SSystem::SString>& aStrings ) ;
		// 一致文字列検索
		static ssize_t FindString
			( const SSystem::SObjectArray<SSystem::SString>& aStrings, const wchar_t * pwszStr ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// EntisGLS4 標準 XML 形式モデルファイル・セーバー
	//////////////////////////////////////////////////////////////////////////

	class	S3DStdXMLModelSaver	: public S3DModelSaverInterface
	{
	protected:
		SSystem::SString					m_strImageMIME ;
		SSystem::SString					m_strImageExt ;
		SGLImageEncoderInterface::Options	m_optImage ;	

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DStdXMLModelSaver, S3DModelSaverInterface )
		// 構築関数
		S3DStdXMLModelSaver( void ) ;
		// 消滅関数
		virtual ~S3DStdXMLModelSaver( void ) ;
		// MIME 判定
		virtual bool IsMatchableMIMEType( const wchar_t * pszMIME ) ;
		// テクスチャ画像保存形式設定
		virtual SGLError SetImageFormat
			( const wchar_t * pwszMIME,
				const wchar_t * pwszExt,
				const SGLImageEncoderInterface::Options * pOpt = NULL ) ;
		// モデルデータ読み込み
		virtual SGLError WriteModel
			( SSystem::SFileInterface & file, S3DModelBuffer & model ) ;

	public:
		// テクスチャ書き出し
		virtual SGLError FormatTextureTag
			( SSystem::SXMLDocument & xmlTexture,
				S3DModelBuffer & model, SSystem::SFileOpener & opener ) ;
		// 表面属性書き出し
		virtual SGLError FormatMaterialTag
			( SSystem::SXMLDocument & xmlMaterial, S3DModelBuffer & model ) ;
		// メッシュ書き出し
		virtual SGLError FormatMeshTag
			( SSystem::SXMLDocument & xmlMesh, S3DModelBuffer & model ) ;
		// メッシュグループ情報書き出し
		virtual SGLError FormatMeshGroupTag
			( SSystem::SXMLDocument & xmlGroup, S3DModelBuffer & model ) ;
		// モーフターゲット書き出し
		virtual SGLError FormatMeshMorphTargetTag
			( SSystem::SXMLDocument & xmlMorph, S3DModelBuffer & model ) ;
		// 分割メッシュ情報書き出し
		virtual SGLError FormatMeshDivisionTag
			( SSystem::SXMLDocument & xmlMorph, S3DModelBuffer & model ) ;
		// ボーン書き出し
		enum	ExportBoneFlag
		{
			boneOnlyPhysics	= 0x0001,
		} ;
		virtual SGLError FormatBoneTag
			( SSystem::SXMLDocument & xmlBone,
					S3DModelBuffer & model, uint32_t nFlags = 0 ) ;
		static void FormatBonePhysMaterial
			( SSystem::SXMLDocument& xmlMaterial,
				const S3DModelBoneSpace::PhysMaterial& physMaterial ) ;
		// マーカー情報書き出し
		virtual SGLError FormatMarkerInfoTag
			( SSystem::SXMLDocument & xmlMarker, S3DModelBuffer & model ) ;
		// クロスシミュレーターメッシュ情報書き出し
		virtual SGLError FormatClothMeshInfoTag
			( SSystem::SXMLDocument & xmlMarker, S3DModelBuffer & model ) ;
		// ポーズ書き出し
		virtual SGLError FormatPosesTag
			( SSystem::SXMLDocument & xmlPose, S3DModelBuffer & model ) ;
		// 拡張データ書き出し
		virtual SGLError FormatUserTags
			( SSystem::SXMLDocument & xmlTag, S3DModelBuffer & model ) ;

	public:
		// 数値列文字列化
		void FormatFloatList
			( SSystem::SString& strList,
				const float32_t * pFloatList,
				size_t nCount, size_t nCountInLine ) ;

	} ;

}

#endif
