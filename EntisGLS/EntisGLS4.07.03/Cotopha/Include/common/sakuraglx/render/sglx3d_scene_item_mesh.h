
#if	!defined(__SAKURAGLX3D_SCENE_ITEM_MESH_H__)
#define	__SAKURAGLX3D_SCENE_ITEM_MESH_H__	1

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// メッシュバッファ・シリアライザ（要素）
	//////////////////////////////////////////////////////////////////////////

	class	S3DMeshBufferPropertySerializer
	{
	public:
		struct	MeshBuffer
		{
			SSystem::SString			strName ;
			S3DPrimitiveType			typeMesh ;
			uint32_t					countPrimitive ;
			uint32_t					countVertex ;
			uint32_t					countIndex ;
			SSystem::SArray<S3DVector4>	bufVertex ;
			SSystem::SArray<S3DVector4>	bufNormal ;
			SSystem::SArray<S2DVector>	bufUVMap ;
			SSystem::SArray<S3DColor>	bufColor ;
			SSystem::SArray<uint32_t>	bufIndex ;
			SSystem::SStrSortObjectArray
				< SSystem::SArray<uint8_t> >
										m_ssaExBuf ;

			bool						flagUpdate ;
			size_t						nTotalBytes ;
			SSystem::SArray<uint8_t>	bufHeadData ;	// BinaryPrimitiveData

			MeshBuffer( void )
				: typeMesh(primitivePoint), flagUpdate(true),
					countPrimitive(0), countVertex(0), countIndex(0) { }
		} ;

	protected:
		SSystem::SString					m_strBase64 ;
		S3DSceneComposer::BinaryHeader		m_binHeader ;
		SSystem::SObjectArray<MeshBuffer>	m_aMeshs ;
		bool								m_flagUpdateMesh ;

	public:
		// 構築関数
		S3DMeshBufferPropertySerializer( void ) ;

	public:
		// メッシュバッファ編集
		size_t GetMeshCount( void ) const ;
		MeshBuffer * GetMeshAt( size_t i ) const ;
		size_t AddMesh( MeshBuffer * pMesh ) ;
		ssize_t FindMesh( MeshBuffer * pMesh ) const ;
		void SwapMesh( size_t i0, size_t i1 ) ;
		void RemoveMeshAt( size_t i ) ;
		void UpdateMesh( MeshBuffer * pMesh ) ;

	public:
		// アイテムから取得
		bool GetMeshBufferParameterFrom
			( const S3DSceneComposer::Parameter& item, size_t iParam ) ;
		// アイテムへ設定
		void PutMeshBufferParameterTo
			( S3DSceneComposer::Parameter& item, size_t iParam ) ;

	public:
		// ヘッダ取得
		const S3DSceneComposer::BinaryHeader& GetBinaryHeader( void ) const ;
		// ヘッダ設定
		void SetBinaryType( const S3DSceneComposer::BinaryHeader& hdr ) ;
	public:
		// シリアライズ
		const SSystem::SString& Serialize( void ) ;
		void SerializeBinary
			( uint8_t * pbytBinary, size_t nBufBytes, size_t nHeaderBytes ) ;
		size_t SerializeBuffer( size_t& nHeaderBytes ) ;
	public:
		// デシリアライズ
		void Deserialize( const wchar_t * pwszBase64 ) ;
		bool DeserializeBinary( const void * pbytBinary, size_t nBytes ) ;

	public:
		// レンダリング
		void RenderBuffer( S3DVertexBufferInterface& vb ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// メッシュバッファ
	//////////////////////////////////////////////////////////////////////////

	class	S3DMeshBufferItemSerializer
				: public S3DSceneComposer::ItemBasicSerializer,
					public S3DParticleSerializer::RenderTarget,
					public S3DInstancingItemInterface
	{
	public:
		// メッシュ・コントローラー
		class	MeshInterface	: public ESLObject
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( MeshInterface, ESLObject )
			// メッシュ追加処理（全視点・ビュー共通処理）
			virtual void AddMesh
				( S3DScene& scene,
					S3DSceneComposer::ItemSerializer * pItem,
					S3DVertexBufferInterface ** ppVBs, size_t nVBCount ) = 0 ;
			// フレーム描画前処理（全視点・ビュー共通処理）
			virtual void UpdateMesh
				( S3DScene& scene,
					S3DSceneComposer::ItemSerializer * pItem,
					S3DVertexBufferInterface ** ppVBs, size_t nVBCount ) = 0 ;
		} ;
		class	MeshController	: public S3DSceneComposer::Controller,
									public MeshInterface
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO2( MeshController, Controller, MeshInterface )
			// 構築関数
			MeshController( const wchar_t * pwszClassID ) ;
		} ;

	public:
		enum	MeshDynamicsMode
		{
			meshStatic,
			meshUpdatable,
			meshDynamic,
			meshModeCount,
		} ;
		enum	ParameterIndex
		{
			paramDynamicMesh		= ItemBasicSerializer::paramItemTotalCount,
			paramMaterialCount,
			paramMaterial0,
			paramMeshData0,
			paramMaterial1,
			paramMeshData1,
			paramMaterial2,
			paramMeshData2,
			paramMaterial3,
			paramMeshData3,
			paramCollision,
			paramColliderFlags,
			paramCollisionAlpha,
			paramVisibleMask,
			paramCollisionMask,
			paramInstancedDraw,
			paramInstancing,
			paramInstanceRotate,
			paramInstanceZoom,
			paramSortingMethod,
			paramCullingMethod,
			paramCullingNearZ,
			paramCullingFarZ,
			paramCullingOffset,
			paramCullingAngleGap,
			paramMeshTotalCount,
			paramMeshCount		= paramMeshTotalCount - paramDynamicMesh,
			paramMaterialMaxCount	= 4,
		} ;

		static const wchar_t *	m_pwszMeshDynamicsMode[meshModeCount] ;

	protected:
		static const S3DSceneComposer::ParamEntry		m_paramEntries[paramMeshCount] ;
		static const S3DSceneComposer::ParamSetClass	m_pscClass ;

	protected:
		MeshDynamicsMode				m_dynamicsMesh ;
		bool							m_flagUpdateMesh ;
		bool							m_flagCollision ;
		bool							m_flagCollisionUpdate ;
		bool							m_flagInstancedDraw ;
		uint32_t						m_nMaterialCount ;
		uint32_t						m_maskCollisionFlags ;
		uint32_t						m_maskVisible ;
		uint32_t						m_maskCollision ;

		SSystem::SObjectArray<S3DMeshBufferPropertySerializer>
										m_aMeshData ;
		S3DMaterial *					m_pMaterials[paramMaterialMaxCount] ;
		SSystem::SString				m_strMaterialIDs[paramMaterialMaxCount] ;
		SSystem::SObjectArray<S3DVertexBuffer>
										m_aVBMesh ;
		SSystem::SPointerArray<S3DVertexBufferInterface>
										m_aVBArray ;

		S3DCollision					m_collision ;
		S3DItemInstancingSerializer		m_instancing ;
		int32_t							m_nCollisionAlpha ;
		double							m_fpCullingOffset ;
		double							m_fpCullingAngle ;
		SSystem::SArray<S4DMatrix>		m_aTempMatrixs ;
		SSystem::SArray<S3DColor>		m_aTempColors ;

		SSystem::SObjectArray<SSystem::SString>
										m_aAdvPropIDs ;
		SSystem::SObjectArray<SSystem::SString>
										m_aAdvPropNames ;

		SSystem::SCriticalSection		m_csLock ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO3
			( S3DMeshBufferItemSerializer,
				ItemBasicSerializer, RenderTarget, S3DInstancingItemInterface )
		S3D_DECLARE_COMPOSER_ITEM( S3DMeshBufferItemSerializer, mesh_buffer )
		// 構築関数
		S3DMeshBufferItemSerializer( void ) ;
		// 消滅関数
		virtual ~S3DMeshBufferItemSerializer( void ) ;

	public:
		// 出力先メッシュバッファ取得
		S3DVertexBufferInterface ** GetTargetVertexBuffers( size_t& nTargetCount ) ;
		// 出力先メッシュバッファ取得（動的メッシュバッファの場合のみ）
		// （動的メッシュの場合 S3DScene::classPreRender で追加可能）
		S3DVertexBufferInterface * GetVertexBuffer( void ) ;
		// 同期
		const SSystem::SCriticalSection * GetInstanceLocker( void ) const
		{
			return	&m_csLock ;
		}
		// 静的メッシュの更新フラグ設定
		void PostUpdateMesh( void ) ;

	public:
		// マテリアル最大数設定
		void SetMaxMaterialCount( size_t nCount ) ;
		size_t GetMaxMaterialCount( void ) const ;
		// マテリアル関連付け
		void AttachMaterial
			( S3DMaterial * pMaterial, const wchar_t * pwszMaterialID, size_t iNum = 0 ) ;
		S3DMaterial * GetMaterialAt( size_t iMaterial ) const ;
		const wchar_t * GetMaterialIDAt( size_t iMaterial ) const ;
		// メッシュデータ
		S3DMeshBufferPropertySerializer * GetStaticMeshDataAt( size_t iMaterial ) const ;
		// メッシュデータ・レンダリング
		void RenderToVertexBuffers
			( S3DScene& scene, S3DVertexBufferInterface** ppVBs, size_t nVBCount ) ;
		static void RenderControllersToVertexBuffers
			( S3DScene& scene, S3DSceneComposer::ItemSerializer& itemOwner,
						S3DVertexBufferInterface** ppVBs, size_t nVBCount ) ;
		// インスタンシングモード
		bool IsInstancingDraw( void ) const ;

	public:
		// マテリアル参照更新
		void UpdateMaterialRef( void ) ;
		// モデルバッファ更新（静的メッシュ）
		void UpdateStaticMesh( void ) ;
		// モデルバッファに MeshController メッシュ追加処理
		void AddControllerMeshs( S3DScene& scene ) ;
		// マテリアルごとのメッシュを統合する
		void MergeMeshEachMaterials( void ) ;
		// モデルバッファに MeshController による更新処理
		void UpdateControllerMeshs( S3DScene& scene ) ;
		// 当たり判定の構築
		void UpdateCollisionBuffer( void ) ;

	public:	// ParameterProperty
		// パラメータ値取得
		virtual S3DDMatrix GetMatrixParameter( size_t i ) const ;
		virtual S3DDVector GetVectorParameter( size_t i ) const ;
		virtual double GetScalarParameter( size_t i ) const ;
		virtual int32_t GetIntegerParameter( size_t i ) const ;
		virtual bool GetBooleanParameter( size_t i ) const ;
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		virtual size_t GetBinaryParameter
			( void * pDst, size_t nBufBytes, size_t i ) const ;
		// パラメータ値設定
		virtual void SetMatrixParameter( size_t i, const S3DDMatrix& mat ) ;
		virtual void SetVectorParameter( size_t i, const S3DDVector& vec ) ;
		virtual void SetScalarParameter( size_t i, double s ) ;
		virtual void SetIntegerParameter( size_t i, int32_t n ) ;
		virtual void SetBooleanParameter( size_t i, bool b ) ;
		virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		virtual size_t SetBinaryParameter
			( size_t i, const void * pSrc, size_t nBufBytes ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;
	public:
		// Loquaty クラス名
		virtual const wchar_t * GetLQClassName( void ) const ;

	public:	// ParameterProperty
		// パラメーター有効性
		virtual bool IsParameterValidation( size_t i ) const ;
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;

	public:	// S3DScene::Item
		// アイテム作用の追加処理
		virtual void OnUpdateBehavior( S3DScene& scene ) ;

	protected:	// ItemSerializer
		// コントローラー通知
		virtual void OnAddController
				( S3DSceneComposer::Controller * pController ) ;
		virtual void BeforeDetachController
				( S3DSceneComposer::Controller * pController ) ;
		// アイテムのプライマリモデル取得
		virtual S3DVertexBufferInterface * GetItemPrimaryModel( void ) ;
		// アイテムのコリジョンバッファ取得
		virtual S3DCollider * GetItemPrimaryCollider( void ) ;
		// レンダリングの為のデバイスリソース準備
		virtual void OnPrepareToRender
			( S3DRenderDevice * pDevice, uint32_t nFlags = 0 ) ;

	public:	// ItemBasicSerializer
		// フレームを適用
		virtual void SetFrameParameters
				( double fpFrame, S3DSceneComposer::SeekMethod seek ) ;
		// レンダリング前後処理（全視点共通）
		virtual void OnItemRenderEvent
			( S3DScene& scene, S3DScene::ItemClass clsItem ) ;
		// 当たり判定追加
		virtual void OnItemRenderCollision
			( const S3DScene& scene, S3DCollision& render ) ;
		// 表示モデル追加
		virtual void OnItemRenderModel
			( const S3DScene& scene,
				S3DRenderContextInterface& render,
				uint64_t flagsExclusion = 0 ) ;
		// アイテムプロパティのリソース等の参照を更新する
		virtual uint32_t UpdatePropertyReference
			( S3DSceneComposer::Composition& comp, uint32_t nFlags ) ;

	public:	// S3DParticleSerializer::RenderTarget
		// アニメーション長取得
		virtual bool GetTargetAnimationLength( double& secLength ) const ;
		// 全フレーム数取得
		virtual size_t GetTargetAnimationFrames( void ) const ;
		// ターゲット空間（逆変換用）
		virtual void GetTargetSpaceTransformation
				( S3DDMatrix& matITarget, S3DDVector& vITarget ) ;
		// パーティクル追加
		virtual void AddParticles
			( size_t nCount,
				const S3DVector4 * pvPoints,
				const size_t * pFrames = NULL,
				const S3DColor * pColors = NULL,
				const float32_t * pZooms = NULL,
				const S4DVector * pFaceDirs = NULL,
				const float32_t * pxAspect = NULL ) ;
		// AddIndexedParticles を使うか？
		virtual bool IsUsingIndexedParticles( void ) ;
		// パーティクル追加（高機能）（classPreRender で呼び出す）
		virtual void AddIndexedParticles
			( size_t nCount,
				const S3DVector4 * pvPoints,
				const S3DParticleSerializer::ParticleIndex * pIndexes,
				const size_t * pFrames = NULL,
				const S3DColor * pColors = NULL,
				const S3DMatrix * pFaceDirs = NULL ) ;

	public:	// S3DInstancingItemInterface
		// インスタンス処理（classPreRender で呼び出す）
		virtual void AddDynamicInstancingEntries
			( const S4DMatrix * pMatrixs,
				const S3DColor * pColors,
				size_t nCount, ESLObject * pSrcItem ) ;
		// S3DItemInstancingSerializer 取得
		virtual S3DItemInstancingSerializer * GetInstancing( void ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// メッシュ出力中継アイテム
	//////////////////////////////////////////////////////////////////////////

	class	S3DIndirectMeshBuilderSerializer
				: public S3DSceneComposer::ItemBasicSerializer
	{
	public:
		enum	ParameterIndex
		{
			paramMeshTarget	= ItemBasicSerializer::paramItemTotalCount,
			paramBuilderTotalCount,
			paramBuilderCount	= paramBuilderTotalCount - paramMeshTarget,
		} ;

	protected:
		static const S3DSceneComposer::ParamEntry		m_paramEntries[paramBuilderCount] ;
		static const S3DSceneComposer::ParamSetClass	m_pscClass ;

	protected:
		SSystem::SString		m_strMeshTarget ;
		SSystem::SSyncReference	m_refMeshTarget ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO
			( S3DIndirectMeshBuilderSerializer, ItemBasicSerializer )
		S3D_DECLARE_COMPOSER_ITEM( S3DMeshBufferItemSerializer, mesh_indirect )
		// 構築関数
		S3DIndirectMeshBuilderSerializer( void ) ;
		// 消滅関数
		virtual ~S3DIndirectMeshBuilderSerializer( void ) ;

	public:
		// 出力先設定
		void AttachMeshTarget
			( S3DMeshBufferItemSerializer * pMesh, const wchar_t * pwszMeshID ) ;
		bool UpdateMeshTarget( void ) ;
		// 出力先取得
		S3DMeshBufferItemSerializer * GetMeshTarget( void ) const ;

	public:	// S3DSceneComposer::Parameter
		// パラメータ値取得
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		// パラメータ値設定
		virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;
		// パラメーター有効性
		virtual bool IsParameterValidation( size_t i ) const ;
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;

	public:	// ItemBasicSerializer
		// レンダリング前後処理（全視点共通）
		virtual void OnItemRenderEvent
			( S3DScene& scene, S3DScene::ItemClass clsItem ) ;
		// アイテムプロパティのリソース等の参照を更新する
		virtual uint32_t UpdatePropertyReference
			( S3DSceneComposer::Composition& comp, uint32_t nFlags ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 多角形・多面体メッシュ生成
	//////////////////////////////////////////////////////////////////////////

	class	S3DPolyhedronMeshController
				: public S3DMeshBufferItemSerializer::MeshController
	{
	public:
		enum	ParameterIndex
		{
			paramShapeType,
			paramUVProjection,
			paramBackFace,
			paramPosition,
			paramRotation,
			paramSize,
			paramUVAtlas,
			paramUVPosition,
			paramUVScale,
			paramBevelType,
			paramBevelSize,
			paramBevelDepth,
			paramSwelling,
			paramSwellDiv,
			paramCount,
		} ;
		enum	ShapeType
		{
			shapeTriangle,
			shapeSqueare,
			shapePentagon,
			shapeHexagon,
			shapeStar,
			shapeTetrahedron,
			shapeHexahedron,
			shapeOctahedron,
			shapeDodecahedron,
			shapeIcosahedron,
			shapeTruncatedIcosahedron,
			shapeIcosahedronX4,
			shapeTruncatedIcosahedronX4,
			shapeTypeCount,
		} ;
		enum	UVProjectionType
		{
			uvOrthogonal,
			uvCube,
			uvTypeCount,
		} ;
		enum	BevelType
		{
			bevelNo,
			bevelFlat,
			bevelCurved,
			bevelCutOff,
			bevelTypeCount,
		} ;

		static const wchar_t *	m_pwszShapeType[shapeTypeCount] ;
		static const wchar_t *	m_pwszUVProjection[uvTypeCount] ;
		static const wchar_t *	m_pwszBevelType[bevelTypeCount] ;

		typedef	void (S3DPolyhedronMeshController::*PFUNC_ADD_MESH)( S3DVertexBufferInterface& vb ) ;
		static const PFUNC_ADD_MESH	m_pfnAddMesh[shapeTypeCount] ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DPolyhedronMeshController, MeshController )
		S3D_DECLARE_COMPOSER_ITEM( S3DPolyhedronMeshController, polyhedron )
		// 構築関数
		S3DPolyhedronMeshController( void ) ;
		// 消滅関数
		virtual ~S3DPolyhedronMeshController( void ) ;

	protected:
		ShapeType			m_shape ;
		UVProjectionType	m_uvPorj ;
		BevelType			m_bevel ;
		bool				m_flagBackFace ;
		S3DDVector			m_vPosition ;
		S3DDVector			m_vSize ;
		S3DDMatrix			m_matRotation ;
		SSystem::SString	m_strUVAtlas ;
		SGLImageRect		m_rectAtlasRef ;
		S2DDVector			m_vUVPosition ;
		S2DDVector			m_vUVScale ;
		double				m_fpBevel ;
		double				m_fpBevelDepth ;
		double				m_fpSwelling ;
		int32_t				m_nSwellDiv ;

		size_t				m_nIcosahedronDiv ;
		bool				m_flagTrianglePolyhedron ;
		bool				m_flagPolyhedronTruncated ;
		S3DPolyhedronMesh::TrianglePolyhedron	m_triangles ;
		S3DPolyhedronMesh::TruncatedPolyhedron	m_truncated ;

		S3DMatrix			m_matMeshSpace ;
		S3DMatrix			m_matUVSpace ;
		S3DVector			m_vUVSpace ;

	public:
		// アトラス画像参照領域更新
		void UpdateRefAtlasRect( void ) ;

	public:	// S3DSceneComposer::Parameter
		// パラメータ値取得
		virtual S3DDMatrix GetMatrixParameter( size_t i ) const ;
		virtual S3DDVector GetVectorParameter( size_t i ) const ;
		virtual double GetScalarParameter( size_t i ) const ;
		virtual int32_t GetIntegerParameter( size_t i ) const ;
		virtual bool GetBooleanParameter( size_t i ) const ;
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		virtual size_t GetBinaryParameter
			( void * pDst, size_t nBufBytes, size_t i ) const ;
		// パラメータ値設定
		virtual void SetMatrixParameter( size_t i, const S3DDMatrix& mat ) ;
		virtual void SetVectorParameter( size_t i, const S3DDVector& vec ) ;
		virtual void SetScalarParameter( size_t i, double s ) ;
		virtual void SetIntegerParameter( size_t i, int32_t n ) ;
		virtual void SetBooleanParameter( size_t i, bool b ) ;
		virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		virtual size_t SetBinaryParameter
			( size_t i, const void * pSrc, size_t nBufBytes ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;
		// アイテムプロパティのリソース等の参照を更新する
		virtual uint32_t UpdatePropertyReference
			( S3DSceneComposer::Composition& comp,
				S3DSceneComposer::ItemSerializer * pItem, uint32_t nFlags ) ;

	public:	// S3DMeshBufferItemSerializer::MeshController
		// メッシュ追加処理（全視点・ビュー共通処理）
		virtual void AddMesh
			( S3DScene& scene,
				S3DSceneComposer::ItemSerializer * pItem,
				S3DVertexBufferInterface ** ppVBs, size_t nVBCount ) ;
		// フレーム描画前処理（全視点・ビュー共通処理）
		virtual void UpdateMesh
			( S3DScene& scene,
				S3DSceneComposer::ItemSerializer * pItem,
				S3DVertexBufferInterface ** ppVBs, size_t nVBCount ) ;

	protected:
		// 正三角形
		void AddMeshTriangle( S3DVertexBufferInterface& vb ) ;
		// 正四角形
		void AddMeshSqueare( S3DVertexBufferInterface& vb ) ;
		// 正五角形
		void AddMeshPentagon( S3DVertexBufferInterface& vb ) ;
		// 正六角形
		void AddMeshHexagon( S3DVertexBufferInterface& vb ) ;
		// 星型
		void AddMeshStar( S3DVertexBufferInterface& vb ) ;
		// 正四面体
		void AddMeshTetrahedron( S3DVertexBufferInterface& vb ) ;
		// 立方体
		void AddMeshHexahedron( S3DVertexBufferInterface& vb ) ;
		// 正八面体
		void AddMeshOctahedron( S3DVertexBufferInterface& vb ) ;
		// 正12面体
		void AddMeshDodecahedron( S3DVertexBufferInterface& vb ) ;
		// 正20面体
		void AddMeshIcosahedron( S3DVertexBufferInterface& vb ) ;
		// 切頂20面体（36面体）
		void AddMeshTruncatedIcosahedron( S3DVertexBufferInterface& vb ) ;
		// 80面体
		void AddMeshIcosahedronX4( S3DVertexBufferInterface& vb ) ;
		// 切頂80面体
		void AddMeshTruncatedIcosahedronX4( S3DVertexBufferInterface& vb ) ;
		// 正20面体分割体・切頂多面体更新
		void UpdateIcosahedronMesh( size_t nDiv, bool flagTruncate ) ;
		// 正20面体分割体
		void AddTrianglePolyhedron( S3DVertexBufferInterface& vb, size_t nDiv ) ;
		// 正20面体分割切頂多面体
		void AddTruncatedPolyhedron( S3DVertexBufferInterface& vb, size_t nDiv ) ;
		// 一つの面を追加
		void AddMeshPlane
			( S3DVertexBufferInterface& vb,
				size_t nCount,
				S3DVector * pvVertex,
				const S3DVector * pvNormal,
				const S3DVector & vPlaneNormal,
				float32_t cosBevelAngle ) ;
		// 面のベベル処理
		void AddMeshBevel
			( S3DVertexBufferInterface& vb,
				size_t nCount,
				S3DVector * pvVertex,
				const S3DVector * pvNormal,
				float32_t cosBevelAngle,
				const S3DVector & vPlaneCenter,
				const S3DVector & vPlaneNormal ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 平面／円柱／球／トーラス／放物錐メッシュ生成
	//////////////////////////////////////////////////////////////////////////

	class	S3DStandardMeshController
				: public S3DMeshBufferItemSerializer::MeshController
	{
	public:
		enum	ParameterIndex
		{
			paramShapeType,
			paramBackFace,
			paramPosition,
			paramRotation,
			paramSize,
			paramRadius,
			paramHorzDiv,
			paramVertDiv,
			paramStartLatitude,
			paramRangeLatitude,
			paramStartLongitude,
			paramRangeLongitude,
			paramUVAtlas,
			paramUVScale,
			paramUVOffset,
			paramColorMul,
			paramColorAdd,
			paramColorAlphaMaster,
			paramColorAlphaCenter,
			paramColorAlphaTop,
			paramColorAlphaBottom,
			paramColorAlphaLeft,
			paramColorAlphaRight,
			paramCount,
		} ;
		enum	ShapeType
		{
			shapePlane,
			shapeCylinder,
			shapeSphere,
			shapeTorus,
			shapeParabola,
			shapeCount,
		} ;

	protected:
		static const wchar_t *	m_pwszShapeType[shapeCount] ;

		typedef	void (S3DStandardMeshController::*PFUNC_ADD_MESH)
				( S3DVector4 * pvVertex, S3DVector4 * pvNormal ) ;
		static const PFUNC_ADD_MESH	m_pfnAddMesh[shapeCount] ;

	protected:
		ShapeType			m_shape ;
		bool				m_flagBackFace ;
		S3DDVector			m_vPosition ;
		S3DDMatrix			m_matRotation ;
		S3DDVector			m_vSize ;
		double				m_fpRadius ;
		int32_t				m_nHorzDiv ;
		int32_t				m_nVertDiv ;
		double				m_degStartLat ;
		double				m_degRangeLat ;
		double				m_degStartLong ;
		double				m_degRangeLong ;
		SSystem::SString	m_strUVAtlas ;
		SGLImageRect		m_rectAtlasRef ;
		S2DDVector			m_vUVPosition ;
		S2DDVector			m_vUVScale ;
		S3DColor			m_colorBase ;
		double				m_alphaMaster ;
		double				m_alphaCenter ;
		double				m_alphaTop ;
		double				m_alphaBottom ;
		double				m_alphaLeft ;
		double				m_alphaRight ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DStandardMeshController, MeshController )
		S3D_DECLARE_COMPOSER_ITEM( S3DStandardMeshController, simple_mesh_builder )
		// 構築関数
		S3DStandardMeshController( void ) ;
		// 消滅関数
		virtual ~S3DStandardMeshController( void ) ;

	public:
		// アトラス画像参照領域更新
		void UpdateRefAtlasRect( void ) ;

	public:	// S3DSceneComposer::Parameter
		// パラメータ値取得
		virtual S3DDMatrix GetMatrixParameter( size_t i ) const ;
		virtual S3DDVector GetVectorParameter( size_t i ) const ;
		virtual double GetScalarParameter( size_t i ) const ;
		virtual int32_t GetIntegerParameter( size_t i ) const ;
		virtual bool GetBooleanParameter( size_t i ) const ;
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		virtual size_t GetBinaryParameter
			( void * pDst, size_t nBufBytes, size_t i ) const ;
		// パラメータ値設定
		virtual void SetMatrixParameter( size_t i, const S3DDMatrix& mat ) ;
		virtual void SetVectorParameter( size_t i, const S3DDVector& vec ) ;
		virtual void SetScalarParameter( size_t i, double s ) ;
		virtual void SetIntegerParameter( size_t i, int32_t n ) ;
		virtual void SetBooleanParameter( size_t i, bool b ) ;
		virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		virtual size_t SetBinaryParameter
			( size_t i, const void * pSrc, size_t nBufBytes ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;

	public:	// S3DMeshBufferItemSerializer::MeshController
		// メッシュ追加処理（全視点・ビュー共通処理）
		virtual void AddMesh
			( S3DScene& scene,
				S3DSceneComposer::ItemSerializer * pItem,
				S3DVertexBufferInterface ** ppVBs, size_t nVBCount ) ;
		// フレーム描画前処理（全視点・ビュー共通処理）
		virtual void UpdateMesh
			( S3DScene& scene,
				S3DSceneComposer::ItemSerializer * pItem,
				S3DVertexBufferInterface ** ppVBs, size_t nVBCount ) ;

	protected:
		// 平面
		void AddMeshPlane( S3DVector4 * pvVertex, S3DVector4 * pvNormal ) ;
		// 円柱
		void AddMeshCylinder( S3DVector4 * pvVertex, S3DVector4 * pvNormal ) ;
		// 球
		void AddMeshSphere( S3DVector4 * pvVertex, S3DVector4 * pvNormal ) ;
		// トーラス
		void AddMeshTorus( S3DVector4 * pvVertex, S3DVector4 * pvNormal ) ;
		// 放物線回転体
		void AddMeshParabola( S3DVector4 * pvVertex, S3DVector4 * pvNormal ) ;

	} ;



	//////////////////////////////////////////////////////////////////////////
	// モデル参照メッシュ
	//////////////////////////////////////////////////////////////////////////

	class	S3DModelRefMeshController
				: public S3DMeshBufferItemSerializer::MeshController
	{
	public:
		enum	ParameterIndex
		{
			paramReferenceModel,
			paramReferenceMesh,
			paramOutBuffer,
			paramRotate,
			paramZoom,
			paramOffset,
			paramCount,
		} ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DModelRefMeshController, MeshController )
		S3D_DECLARE_COMPOSER_ITEM( S3DModelRefMeshController, ref_model_mesh )
		// 構築関数
		S3DModelRefMeshController( void ) ;
		// 消滅関数
		virtual ~S3DModelRefMeshController( void ) ;

	protected:
		S3DModelBuffer *			m_pRefModel ;
		S3DModelData::MeshGroup *	m_pmgRefMesh ;
		SSystem::SString			m_strRefModel ;
		SSystem::SString			m_strRefMesh ;
		S3DMatrix					m_matTransform ;
		S3DMatrix					m_matRotate ;
		S3DVector					m_vZoom ;
		S3DVector					m_vOffset ;
		int32_t						m_iOutBuffer ;

	public:
		// 参照モデル更新
		void UpdateRefModel( void ) ;
		// 行列更新
		void UpdateTransformation( void ) ;

	public:	// S3DSceneComposer::Parameter
		// パラメータ値取得
		virtual S3DDMatrix GetMatrixParameter( size_t i ) const ;
		virtual S3DDVector GetVectorParameter( size_t i ) const ;
		virtual int32_t GetIntegerParameter( size_t i ) const ;
		virtual const wchar_t * GetCommandParameter( size_t i ) const ;
		// パラメータ値設定
		virtual void SetMatrixParameter( size_t i, const S3DDMatrix& mat ) ;
		virtual void SetVectorParameter( size_t i, const S3DDVector& vec ) ;
		virtual void SetIntegerParameter( size_t i, int32_t n ) ;
		virtual void SetCommandParameter( size_t i, const wchar_t * pwszCmd ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;

	public:	// S3DSceneComposer::Controller
		// アイテムプロパティのリソース等の参照を更新する
		virtual uint32_t UpdatePropertyReference
			( S3DSceneComposer::Composition& comp,
				S3DSceneComposer::ItemSerializer * pItem, uint32_t nFlags ) ;

	public:	// S3DMeshBufferItemSerializer::MeshController
		// メッシュ追加処理（全視点・ビュー共通処理）
		virtual void AddMesh
			( S3DScene& scene,
				S3DSceneComposer::ItemSerializer * pItem,
				S3DVertexBufferInterface ** ppVBs, size_t nVBCount ) ;
		// フレーム描画前処理（全視点・ビュー共通処理）
		virtual void UpdateMesh
			( S3DScene& scene,
				S3DSceneComposer::ItemSerializer * pItem,
				S3DVertexBufferInterface ** ppVBs, size_t nVBCount ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 段階状密度平面メッシュ
	//////////////////////////////////////////////////////////////////////////

	class	S3DCascadePlaneMeshController
				: public S3DMeshBufferItemSerializer::MeshController
	{
	public:
		enum	ParameterIndex
		{
			paramCenter,
			paramUnit,
			paramDivision,
			paramCascade,
			paramCount,
		} ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DCascadePlaneMeshController, MeshController )
		S3D_DECLARE_COMPOSER_ITEM( S3DCascadePlaneMeshController, cascade_plane_mesh )
		// 構築関数
		S3DCascadePlaneMeshController( void ) ;
		// 消滅関数
		virtual ~S3DCascadePlaneMeshController( void ) ;

	protected:
		S3DDVector	m_vCenter ;
		double		m_fpUnit ;
		size_t		m_nDivision ;
		size_t		m_nCascade ;

	public:	// S3DSceneComposer::Parameter
		// パラメータ値取得
		virtual S3DDVector GetVectorParameter( size_t iParam ) const ;
		virtual double GetScalarParameter( size_t iParam ) const ;
		virtual int32_t GetIntegerParameter( size_t iParam ) const ;
		// パラメータ値設定
		virtual void SetVectorParameter( size_t iParam, const S3DDVector& vec ) ;
		virtual void SetScalarParameter( size_t iParam, double s ) ;
		virtual void SetIntegerParameter( size_t iParam, int32_t n ) ;

	public:	// S3DMeshBufferItemSerializer::MeshController
		// メッシュ追加処理（全視点・ビュー共通処理）
		virtual void AddMesh
			( S3DScene& scene,
				S3DSceneComposer::ItemSerializer * pItem,
				S3DVertexBufferInterface ** ppVBs, size_t nVBCount ) ;
		// フレーム描画前処理（全視点・ビュー共通処理）
		virtual void UpdateMesh
			( S3DScene& scene,
				S3DSceneComposer::ItemSerializer * pItem,
				S3DVertexBufferInterface ** ppVBs, size_t nVBCount ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 木メッシュ生成
	//////////////////////////////////////////////////////////////////////////

	class	S3DTreeMeshBuilderController
				: public S3DMeshBufferItemSerializer::MeshController
	{
	public:
		enum	ParameterIndex
		{
			paramRandomSeed,
			paramNoBranchMesh,
			paramRootThickness,
			paramLeafThickness,
			paramSegmentLength,
			paramLeafWidth,
			paramLeafHeight,
			paramLeafSizeRandom,
			paramLeafBaseColor,
			paramTrunkVScale,
			paramTrunkShrink,
			paramBranchShrink,
			paramRootTrunkLength,
			paramTrunkDivCount,
			paramBranchCount,
			paramBranchRandom,
			paramTrunkAngle,
			paramBranchAngle,
			paramBranchRndAngle,
			paramLeafShape,
			paramTopLeafShape,
			paramLeafSwell,
			paramTopLeafAnglel,
			paramTipLeafCount,
			paramTopLeafCount,
			paramLeafHangAngle,
			paramLeafRndAngle,
			paramLeafFaceAngle,
			paramCount,
		} ;
		enum	LeafShape
		{
			leafPlane,
			leafPyramid,
			leafBothPyramid,
			leafShapeCount,
		} ;
		static const wchar_t *	s_pwszLeafShape[leafShapeCount] ;
		static const SSystem::SXMLDocument::AttrInteger	s_aiLeafShape[leafShapeCount+1] ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( S3DTreeMeshBuilderController, MeshController )
		S3D_DECLARE_COMPOSER_ITEM( S3DTreeMeshBuilderController, treee_mesh_builder )
		// 構築関数
		S3DTreeMeshBuilderController( void ) ;
		// 消滅関数
		virtual ~S3DTreeMeshBuilderController( void ) ;

	protected:
		int32_t		m_nRandomSeed ;				// 乱数の種
		bool		m_flagNoBranchMesh ;		// 幹・枝メッシュは生成しない
		double		m_fpRootThickness ;			// 根元の太さ
		double		m_fpLeafThickness ;			// 枝が葉になる太さ
		double		m_fpSegmentLength ;			// 根本の節の長さ
		double		m_fpLeafWidth ;				// 葉のサイズ（幅）
		double		m_fpLeafHeight ;			// 葉のサイズ（高さ）
		double		m_fpLeafSizeRandom ;		// 葉のサイズの揺らぎ率
		SGLPalette	m_rgbLeafBaseColor ;		// 葉の付け根乗算色
		double		m_fpTrunkVScale ;			// 幹 UV の高さ比率
		double		m_fpTrunkShrink ;			// 幹太さ減衰率（拡大率）
		double		m_fpBranchShrink ;			// 枝太さ減衰率
		int32_t		m_nRootTrunkLength ;		// 根本幹（枝のない幹）の長さ
		int32_t		m_nTrunkDivCount ;			// 幹（枝）の分割数
		int32_t		m_nBranchCount ;			// 1つの節から分岐する枝の本数
		int32_t		m_nBranchRandom ;			// 同、追加の乱数
		double		m_degTrunkAngle ;			// 幹の節の曲がり角（0～乱数） [deg]
		double		m_degBranchAngle ;			// 枝の節の曲がり角 [deg]
		double		m_degBranchRndAngle ;		// 枝の節の曲がり角乱数幅 [deg]
		LeafShape	m_shapeLeaf ;				// 葉形状
		LeafShape	m_shapeTopLeaf ;			// 天頂葉形状
		double		m_fpLeafSwell ;				// 葉の厚み比
		double		m_degTopLeafAngle ;			// 葉の天頂方向角 [deg]
		int32_t		m_nTipLeafCount ;			// 枝先端の葉の枚数
		int32_t		m_nTopLeafCount ;			// 天頂先端の葉の枚数
		double		m_degLeafHangAngle ;		// 葉垂れ角度 [deg]
		double		m_degLeafRndAngle ;			// 先端の追加的な葉の角度幅 [deg]
		double		m_degLeafFaceAngle ;		// 葉の天頂方向からの遊び角 [deg]

		SakuraCL::SCLRandomizer	m_random ;
		size_t					m_nSegCount ;

	public:	// S3DSceneComposer::Parameter
		// パラメータ値取得
		virtual S3DDVector GetVectorParameter( size_t iParam ) const ;
		virtual double GetScalarParameter( size_t iParam ) const ;
		virtual int32_t GetIntegerParameter( size_t iParam ) const ;
		virtual bool GetBooleanParameter( size_t iParam ) const ;
		virtual const wchar_t * GetCommandParameter( size_t iParam ) const ;
		// パラメータ値設定
		virtual void SetVectorParameter( size_t iParam, const S3DDVector& vec ) ;
		virtual void SetScalarParameter( size_t iParam, double s ) ;
		virtual void SetIntegerParameter( size_t iParam, int32_t n ) ;
		virtual void SetBooleanParameter( size_t iParam, bool b ) ;
		virtual void SetCommandParameter( size_t iParam, const wchar_t * pwszCmd ) ;
		// パラメータ値域列挙
		virtual bool EnumerateStringSet
			( size_t iParam, SSystem::SStringArray& aStrSet ) ;
		// パラメータカテゴリ名取得
		virtual const wchar_t * GetParameterCategoryName( size_t iCategory ) const ;

	public:	// S3DMeshBufferItemSerializer::MeshController
		// メッシュ追加処理（全視点・ビュー共通処理）
		virtual void AddMesh
			( S3DScene& scene,
				S3DSceneComposer::ItemSerializer * pItem,
				S3DVertexBufferInterface ** ppVBs, size_t nVBCount ) ;
		// フレーム描画前処理（全視点・ビュー共通処理）
		virtual void UpdateMesh
			( S3DScene& scene,
				S3DSceneComposer::ItemSerializer * pItem,
				S3DVertexBufferInterface ** ppVBs, size_t nVBCount ) ;

	protected:
		// 枝発生
		void BuildBranch
			( S3DVertexBufferInterface ** ppVBs, size_t nVBCount,
				const S3DMatrix& matBuild0, const S3DMatrix& matBuild1,
				S3DVector& vPos0, float32_t vCoord0,
				float32_t fpThickness0, float32_t fpLength,
				int nBranch, int nNestCount ) ;
		// 葉発生
		void BuildLeaf
			( S3DVertexBufferInterface ** ppVBs, size_t nVBCount,
				const S3DMatrix& matBuild, S3DVector& vPos, size_t nAddCount ) ;
		void BuildThickLeaf
			( S3DVertexBufferInterface ** ppVBs, size_t nVBCount,
				const S3DMatrix& matBuild, S3DVector& vPos,
				size_t nAddCount, bool flagTopLeaf ) ;
		void BuildBothThickLeaf
			( S3DVertexBufferInterface ** ppVBs, size_t nVBCount,
				const S3DMatrix& matBuild, S3DVector& vPos,
				size_t nAddCount, bool flagTopLeaf ) ;

	} ;

}

#endif
