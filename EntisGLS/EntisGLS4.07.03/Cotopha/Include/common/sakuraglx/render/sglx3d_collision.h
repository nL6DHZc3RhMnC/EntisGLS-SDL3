
#if	!defined(__SAKURAGLX3D_COLLISION_H__)
#define	__SAKURAGLX3D_COLLISION_H__	1

#include <sakura/ssys_bit_array.h>
#include <sakuragl/sgl3d/sgl_render_buffer.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// 3D 当たり判定
	//////////////////////////////////////////////////////////////////////////

	struct	S3DCollisionResult ;
	class	S3DCollider	: public SSystem::SObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DCollider, SObject )
		// 範囲取得
		virtual bool GetCollisionRange( S3DDVector& vCenter, double& fpRadius ) const = 0 ;
		// 凸形状の内側判定
		virtual bool IsSphereInclusive
			( const S3DDVector& vPos, float fpRadius, S3DCollisionResult& rsIncluded ) const = 0 ;
		// 球との交差判定
		virtual bool IsHitAgainstSphere
			( const S3DDVector& vPos, float fpRadius, S3DCollisionResult& rsHit ) const = 0 ;
		// 線分との交差判定
		//（※rsCross.fpDistance にはあらかじめ |vPos1-vPos0| を設定しておくこと）
		virtual bool IsSegmentCrossing
			( const S3DDVector& vPos0, const S3DDVector& vPos1,
							float fpErrorGap, S3DCollisionResult& rsCross ) const = 0 ;
	} ;

	class	S3DCollision : public S3DRenderBuffer, public S3DCollider
	{
	public:
		// 当たり判定用構造体 △ABC （4 Polygons）
		struct	QuadPolygonsCollision
		{
			float32_t	xA[4] ;			// 頂点A座標
			float32_t	yA[4] ;
			float32_t	zA[4] ;
			float32_t	xB[4] ;			// 頂点B座標
			float32_t	yB[4] ;
			float32_t	zB[4] ;
			float32_t	xC[4] ;			// 頂点C座標
			float32_t	yC[4] ;
			float32_t	zC[4] ;
			float32_t	xNormal[4] ;	// 法線
			float32_t	yNormal[4] ;
			float32_t	zNormal[4] ;
			float32_t	absAB[4] ;		// |A - B|
			float32_t	absAC[4] ;		// |A - C|
			float32_t	absBC[4] ;		// |B - C|
			float32_t	cosB[4] ;		// cos∠ABC
			float32_t	sinB[4] ;		// sin∠ABC
			float32_t	cosC[4] ;		// cos∠ACB
			float32_t	sinC[4] ;		// sin∠ACB
			uint32_t	iShiftA[4] ;	// 頂点Aの順序
						// ※元Polygonの頂点をD,E,Fとしたとき、
						// 　△ABCの辺BCが最長となるように
						// 　△ABC <= { 0:△DEF, 1:△EFD, 2:△FDE }
		} ;
		// 当たり判定用（範囲球）構造体 (4x4x2^n Polygons)
		struct	QuadSphereCollision
		{
			float32_t	xCenter[4] ;
			float32_t	yCenter[4] ;
			float32_t	zCenter[4] ;
			float32_t	fpRadius[4] ;
		} ;
		// 座標変換（逆変換用）
		struct	ITransformation
		{
			S3DDVector	vPos ;
			S3DMatrix	matRev ;
			S3DMatrix	matIRev ;		// x' = matIRev * (x - vPos)
			float32_t	fpScale ;		// abs(det matIRev)^(1/3)
			bool		flagOrthMat ;	// matRev は直行行列か直行行列のスカラ倍
		} ;
		// メッシュツリーノード
		struct	MeshCollision ;
		struct	MeshTreeNode
		{
			enum	NodeDefinition
			{
				countChild	= 2,
			} ;
			S3DDVector		vCenter ;			// ※グローバル座標
			double			fpRadius ;
			MeshCollision *	pMeshCol ;
			MeshTreeNode *	pParent ;
			MeshTreeNode *	pChild[countChild] ;
		} ;
		// メッシュ
		enum	MeshCollisionType
		{
			typeTriangleList,
			typeCollider,
			typeSolidSphere,
			typeSolidCube,
			typeLineList,
			typeLineStrip,
			typePoints,
		} ;
		enum	UserColliderClass
		{
			colliderClassShape	= 0,
			colliderClassBarrier,
			colliderClassHit,
			colliderClassAttack,
			colliderClassEvent,
			colliderClassBone,
			colliderClassPhysics,
			colliderClassReserved,
			colliderClassUser0,
			colliderShape		= 0x00000001,	// 形状・移動障壁
			colliderBarrier		= 0x00000002,	// 移動障壁
			colliderHit			= 0x00000004,	// 被当たり判定領域（敵）
			colliderAttack		= 0x00000008,	// 攻撃当たり判定（敵）
			colliderEvent		= 0x00000010,	// イベント発生
			colliderBone		= 0x00000020,	// ボーン物理演算用当たり判定
			colliderPhysics		= 0x00000040,	// 物理演算（障壁）当たり判定
			colliderReserved	= 0x00000080,
			colliderUser0		= 0x00000100,
			colliderBone0		= 0x00010000,	// ボーン当たり判定用
			colliderPhysItem	= colliderShape | colliderPhysics,
			colliderPhysTarget	= colliderPhysItem | colliderEvent,
			colliderBoneAllMask	= 0xFFFF0020,
			colliderBone0Shift	= 16,
		} ;
		// maskClasses から UserColliderClass 成分の分離
		static uint32_t GetUserColliderMask( uint64_t maskClasses ) ;
		// maskClasses へ UserColliderClass 成分の合成用
		static uint64_t MakeUserColliderMask( uint32_t maskColliders ) ;

		struct	MeshCollision
		{
			MeshCollisionType		typeCol ;
			ITransformation			itrans ;
			S3DVector				vCenter ;		// ※ローカル座標
			float32_t				fpRadius ;
			float32_t				fpThickness ;
			MeshTreeNode *			pParentNode ;
			size_t					nQSPackScale ;	// pSphereCol の1エントリの 4Polygons セット数
			QuadSphereCollision *	pSphereCol ;
			QuadPolygonsCollision *	pCollision ;
			S3DCollider *			pCollider ;
			size_t					iInstance ;
			size_t					iMesh ;
			RENDER_ENTRY *			preMesh ;
			size_t					nPolygons ;
			ESLObject *				pUserData ;
			uint64_t				maskClasses ;
		} ;

		// 当たり判定結果構造体
		typedef	S3DCollisionResult	Result ;

		// コールバック関数
		enum	HitColliderCallback
		{
			hitColliderReturn,			// 当たり判定有効
			hitColliderNext,			// 当たり判定せず次のプリミティブへ
			hitColliderNextMesh,		// 当たり判定せず次のメッシュへ
		} ;
		typedef HitColliderCallback
			(*Callback_OnHitCollider)
				( const S3DCollisionResult& rsHit,
					const S3DVector& vHitPos, const S3DVector& vHitNormal,
					const MeshCollision * pMesh, size_t iPolygon ) ;

		// コールバック関数でのローカル空間から
		// グローバル空間へ変換した情報を受け取る構造体
		struct	HitColliderGlobalInfo
		{
			S3DMatrix				matToGlobal ;	// ローカル→グローバル変換
			S3DVector				vToGlobal ;
			ESLObject *				pUserData ;		// グローバルなアイテム
			size_t					iInstance ;
			const MeshCollision *	pGlobalMesh ;
		} ;

		// 同一メッシュ内球当たり判定コンテキスト
		struct	NextHitContext
		{
			S3DMatrix				matMesh ;
			S3DVector				vMesh ;
			const MeshCollision *	pmcMeshCol ;
			S3DVector				vTestPos ;
			float32_t				fpRadius ;
			size_t					iNextPoly ;

			NextHitContext( void )
				: matMesh(1,1,1), vMesh(0,0,0), pmcMeshCol(NULL),
					vTestPos(0,0,0), fpRadius(0), iNextPoly(0) { }
			void PrepareNextHitSphere
				( const Result& rsHit, const S3DDVector& vPos, float fpRadius )
			{
				S3DCollision::PrepareNextHitSphere
					( *this, rsHit, vPos, fpRadius ) ;
			}
			bool NextHitAgainstSphere( Result& rsHit )
			{
				return	S3DCollision::NextHitAgainstSphere( *this, rsHit ) ;
			}
		} ;

		// 複数の当たり判定を受け取る際の１エントリ構造体
		struct	HitEntryInfo
		{
			const MeshCollision *	pmcHitPrimitive ;
			const MeshCollision *	pmcHitGlobal ;
			S3DVector				vHitGlobalPos ;
			S3DVector				vHitGlobalNormal ;
		} ;

		// 当たり判定情報
		enum	ColliderType
		{
			coliderTypeBuffer,
			coliderTypeSolidSphere,
			coliderTypeSolidCube,
		} ;
		struct	ColliderDescription
		{
			ColliderType	type ;
			S3DVector		vPos ;			// type == {coliderTypeSolidSphere | coliderTypeSolidCube}
			float32_t		fpRadius ;		// type == coliderTypeSolidSphere
			S3DVector		vCubeSize ;		// type == coliderTypeSolidCube
			S3DCollider *	pColider ;		// type == coliderTypeBuffer
		} ;

	protected:
		MeshTreeNode *							m_pRootNode ;
		SSystem::SPointerArray<MeshCollision>	m_arrCollision ;
		SSystem::SArray<size_t>					m_arrCollisionIndex ;
		S3DTemporaryIndexTriangleStrip			m_indexTriangleStrip ;
		SSystem::SPointerArray<MeshTreeNode>	m_arrMeshNode ;
		atomic_int_t							m_countBatchBuild ;

		// 追加オブジェクト付加情報
		ESLObject *						m_pUserData ;
		uint64_t						m_maskClasses ;
		float32_t						m_fpAddThickness ;

		struct	CollisionContext
		{
			ESLObject *	pUserData ;
			float32_t	fpAddThickness ;
			uint64_t	maskClasses ;
		} ;
		SSystem::SArray<CollisionContext>	m_arrCtxStack ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
			( S3DCollision, S3DRenderBufferInterface, S3DCollider )
		// 構築関数
		S3DCollision( void ) ;
		// 消滅関数
		virtual ~S3DCollision( void ) ;

	public:
		// 範囲取得
		virtual bool GetCollisionRange( S3DDVector& vCenter, double& fpRadius ) const ;
		// 凸形状の内側判定
		virtual bool IsSphereInclusive
			( const S3DDVector& vPos, float fpRadius, S3DCollisionResult& rsIncluded ) const ;
		// 球との交差判定
		virtual bool IsHitAgainstSphere
			( const S3DDVector& vPos, float fpRadius, S3DCollisionResult& rsHit ) const ;
		// 線分との交差判定
		virtual bool IsSegmentCrossing
			( const S3DDVector& vPos0, const S3DDVector& vPos1,
							float fpErrorGap, S3DCollisionResult& rsCross ) const ;

	public:
		// 複数の当たり判定取得
		enum	MultiHitEntriesFlag
		{
			multiHitAny				= 0,
			multiHitUniqueMesh		= 0x0001,	// 同一 MeshCollision は一つだけ
			multiHitUniqueUserData	= 0x0002,	// 同一 pUserData は一つだけ
		} ;
		bool AddHitEntriesAgainstSphere
			( SSystem::SArray<HitEntryInfo>& aHitEntries,
				const S3DDVector& vPos, float fpRadius,
				MultiHitEntriesFlag flagsMultiHit,
				uint64_t maskCollider, uint64_t maskException = 0 ) const ;

	protected:
		struct	AddHitEntriesAgainstSphereParam
		{
			SSystem::SArray<HitEntryInfo> *	pHitEntries ;
			const S3DCollision *			pCollision ;
			MultiHitEntriesFlag				flagMultiHit ;
		} ;
		static HitColliderCallback
			Callback_OnAddHitEntriesAgainstSphere
				( const S3DCollisionResult& rsHit,
					const S3DVector& vHitPos, const S3DVector& vHitNormal,
					const MeshCollision * pMesh, size_t iPolygon ) ;

	public:
		// 特定メッシュに対して凸形状の内側判定
		static bool IsSphereInclusiveForMesh
			( const MeshCollision * pMeshCol,
				const S3DDVector& vPos, float fpRadius, Result& rsIncluded ) ;
		// 特定メッシュに対して球との交差判定
		static bool IsHitAgainstSphereForMesh
			( const MeshCollision * pMeshCol,
				const S3DDVector& vPos, float fpRadius, Result& rsHit ) ;
		// 特定メッシュに対して線分との交差判定
		static bool IsSegmentCrossingForMesh
			( const MeshCollision * pMeshCol,
				const S3DDVector& vPos0, const S3DDVector& vPos1,
							float fpErrorGap, Result& rsCross ) ;
	protected:
		static bool IsSegmentCrossingForMesh
			( const MeshCollision * pMeshCol,
				const S3DDVector& vPos0, const S3DDVector& vPos1,
				const S3DDVector& vGlobalSegRay,
				double fpGlobalRayScale, double fpGlobalRcpRayScale,
							float fpErrorGap, Result& rsCross ) ;

	protected:
		// 凸形状の内側判定（ツリー探索）
		static bool IsSphereIncludedNode
			( const MeshTreeNode * pNode,
				const S3DDVector& vPos, float fpRadius, Result& rsIncluded ) ;
		// 球との交差判定（ツリー探索）
		static bool DoesNodeHitAgainstSphere
			( const MeshTreeNode * pNode,
				const S3DDVector& vPos, float fpRadius, Result& rsHit ) ;
		// 線分との交差判定（ツリー探索）
		static bool IsSegmentCrossingNode
			( const MeshTreeNode * pMeshCol,
				const S3DDVector& vPos0, const S3DDVector& vPos1,
				const S3DDVector& vGlobalSegRay,
				double fpGlobalRayScale, double fpGlobalRcpRayScale,
								float fpErrorGap, Result& rsCross ) ;

	public:
		// 変換行列を
		// △ABC の辺 AB, AC を基底とした交差座標を計算
		static SGLError ComputeHitLocalCoord( Result& rsCross ) ;
		// 法線補完計算
		static SGLError ComplementeLocalNormal( Result& rsCross ) ;
		// UV座標計算
		static SGLError ComplementeTextureCoord( Result& rsCross ) ;
		// 頂点色計算
		static SGLError ComplementeVertexColor( Result& rsCross ) ;
		// テクスチャサンプリング
		static SGLError SampleDiffusionTexture
				( const Result& rsCross, SGLPalette& rgbaTexture ) ;
		static SGLError SampleLuminousTexture
				( const Result& rsCross, SGLPalette& rgbaTexture ) ;
		// 法線・UV・頂点色計算／各種テクスチャサンプリング
		static SGLError ComplementeAndSampleAttributes( Result& rsCross ) ;
		// 当たり判定コールバック関数に渡される座標空間からグローバルへの変換情報
		static void ComputeHitColliderGlobalInfo
			( HitColliderGlobalInfo& hcgi,
				const Result& rs, const MeshCollision * pMeshCol ) ;
		// 当たり判定コールバックに渡されるパラメータを反映
		static void SetResultParamOnHitCollider
			( Result& rs, const S3DCollision::HitColliderGlobalInfo& hcgi,
				const S3DVector& vHitPos, const S3DVector& vHitNormal,
				const S3DCollision::MeshCollision * pMesh, size_t iPolygon ) ;
		// 法線のローカル空間からグローバル空間への変換（一般）
		static void ComputeLocalNormalToGlobal
			( S3DVector& vNormal, const S3DMatrix& matToGlobal ) ;

	public:
		// 球との交差判定、次候補準備
		static void PrepareNextHitSphere
			( NextHitContext& nhc, const Result& rsHit,
					const S3DDVector& vPos, float fpRadius ) ;
		// 次の当たり判定
		static bool NextHitAgainstSphere
			( NextHitContext& nhc, Result& rsHit ) ;

	public:
		// ローカル座標へ変換
		static void TransformToMeshLocal
			( const MeshCollision * pMeshCol,
				S3DVector& vLocalPos, float32_t& fpLocalRadius,
				const S3DDVector& vGlobalPos, float fpGlobalRadius ) ;
		// 凸形状の内側判定（メッシュ内）
		static bool IsSphereIncludedMesh
			( const MeshCollision * pMeshCol,
				const S3DVector& vPos, float fpRadius, Result& rsIncluded ) ;
		static bool IsSphereIncludedLine
			( const S3DVector& vLine0, const S3DVector& vLine1,
				const S3DVector& vPos, float fpRadius ) ;
		// 球との交差判定（メッシュ内）
		static bool DoesMeshHitAgainstSphere
			( const MeshCollision * pMeshCol,
				const S3DVector& vPos,
				float fpRadius, Result& rsHit, size_t iFirstPoly = 0 ) ;
		static bool DoesLineHitAgainstSphere
			( const S3DVector& vLine0, const S3DVector& vLine1,
				const S3DVector& vPos, float fpRadius, Result& rsHit ) ;
		static bool DoesTriangleMeshHitAgainstSphere
			( const MeshCollision * pMeshCol,
				const S3DVector& vPos,
				float fpRadius, Result& rsHit, size_t iFirstPoly ) ;
		#if	defined(__PROCESSOR_INTEL_X86__) || defined(__PROCESSOR_INTEL_X86_64__)
		static bool DoesTriangleMeshHitAgainstSphere_SSE
			( const MeshCollision * pMeshCol,
				const S3DVector& vPos,
				float fpRadius, Result& rsHit, size_t iFirstPoly ) ;
		#endif
		#if	defined(__PROCESSOR_ARM__) && (__PROCESSOR_ARM__ >= 7)
		static bool DoesTriangleMeshHitAgainstSphere_NEON
			( const MeshCollision * pMeshCol,
				const S3DVector& vPos,
				float fpRadius, Result& rsHit, size_t iFirstPoly ) ;
		#endif
		// 球との交差判定の結果を大域空間に変換
		static void ComputeGlobalOfHitAgainstSphere
			( Result& rsHit, const MeshCollision& mcMeshCol ) ;
		// 線分との交差判定（メッシュ内）
		static bool IsSegmentCrossingMesh
			( const MeshCollision * pMeshCol,
				const S3DVector& vPos0, const S3DVector& vPos1,
								float fpErrorGap, Result& rsCross ) ;
		static bool IsSegmentCrossingSolidSphere
			( const MeshCollision * pMeshCol,
				const S3DVector& vPos0,
				const S3DVector& vSegRay, double fpSegLength,
				const S3DVector& vCenter, float fpRadius, Result& rsCross ) ;
		static bool IsSegmentCrossingLine
			( const MeshCollision * pMeshCol,
				const S3DVector& vPos0,
				const S3DVector& vSegRay, double fpSegLength,
				const S3DVector& vLine0,
				const S3DVector& vLine1, float fpThickness, Result& rsCross ) ;
		static bool IsSegmentCrossingTriangleMesh
			( const MeshCollision * pMeshCol,
				const S3DVector& vPos0, const S3DVector& vPos1,
								float fpErrorGap, Result& rsCross ) ;
		#if	defined(__PROCESSOR_INTEL_X86__) || defined(__PROCESSOR_INTEL_X86_64__)
		static bool IsSegmentCrossingTriangleMesh_SSE
			( const MeshCollision * pMeshCol,
				const S3DVector& vPos0, const S3DVector& vPos1,
								float fpErrorGap, Result& rsCross ) ;
		#endif
		#if	defined(__PROCESSOR_ARM__) && (__PROCESSOR_ARM__ >= 7)
		static bool IsSegmentCrossingTriangleMesh_NEON
			( const MeshCollision * pMeshCol,
				const S3DVector& vPos0, const S3DVector& vPos1,
								float fpErrorGap, Result& rsCross ) ;
		#endif

	public:
		// 追加判定幅
		float32_t GetCurrentThickness( void ) const ;
		void SetCurrentThickness( float32_t fpThickness ) ;
		// 以降に追加するメッシュに関連付けるユーザーデータを設定する
		void AttachMeshUserData( ESLObject * pUserData ) ;
		ESLObject * GetMeshUserData( void ) const ;
		// シーンクラスマスク（システム規定）
		// （※S3DScene::ItemClass に対応するビットマスク）
		void SetSceneClassesMask( uint32_t maskClasses ) ;
		uint32_t GetSceneClassesMask( void ) const ;
		// ユーザー拡張クラスマスク
		void SetUserClassesMask( uint32_t maskClasses ) ;
		uint32_t GetUserClassesMask( void ) const ;

	public:
		// コンテキストの階層化
		virtual SGLError PushTransformation( void ) ;
		virtual SGLError PopTransformation( void ) ;
		virtual SGLError ResetTransformation( void ) ;
		// ソリッド球を追加
		virtual SGLError AddSolidSphere
			( const S3DVector& vPos, float32_t fpRadius, size_t iInstanceNum ) ;
		// ソリッド直方体を追加
		virtual SGLError AddSolidCube
			( const S3DVector& vPos, const S3DVector& vCubeSize, size_t iInstanceNum ) ;
		// ソリッド円柱を追加
		virtual SGLError AddSolidTubeList
			( const S3DVector* pPoints, size_t nPointCount, float32_t fpRadius ) ;
		// プリミティブリストをレンダリングバッファに追加
		virtual SGLError AddIndexedPrimitiveList
			( S3DMaterial * pMaterial, uint32_t nFlags,
				S3DPrimitiveType typePrimitive,
				size_t countIndex, size_t countVertex,
				const S3DVector4 * pvVertex,
				const S3DVector4 * pvNormal,
				const S2DVector * pvUVMap,
				const S3DColor * pColor,
				const uint32_t * pIndexedList ) ;
		// 頂点バッファの内容を描画
		virtual SGLError AddVertexBuffer
			( S3DMaterial * pMaterial, uint32_t nFlags,
				S3DVertexBufferInterface * pBuffer,
				size_t iFirst = 0, ssize_t iEnd = -1,
				size_t nInstancing = 0,
				const S4DMatrix * pmatInstancing = NULL,
				const S3DColor * pColorInstancing = NULL ) ;
		SGLError AddColliderObject
			( S3DCollider * pCollider,
				const S4DMatrix * pmatInstancing, size_t iInstanceNum ) ;
		// 当たり判定追加
		SGLError AddColliderDescription
			( const ColliderDescription& desc, size_t iInstanceNum ) ;
		// プリミティブを追加するためのバッファを確保する
		virtual SGLError AllocatePrimitiveBuffer
			( PrimitiveBuffer& prmbuf,
				S3DPrimitiveType typePrimitive,
				size_t countIndex, size_t countVertex ) ;
		// プリミティブを追加する（バッファの管理は S3DVertexBufferInterface に移る）
		virtual SGLError AddPrimitiveBuffer
			( S3DMaterial * pMaterial, uint32_t nFlags,
				S3DPrimitiveType typePrimitive,
				const PrimitiveBuffer& prmbuf,
				size_t countIndex, size_t countVertex ) ;
		// プリミティブを追加せずにバッファを開放する
		virtual SGLError FreePrimitiveBuffer
			( const PrimitiveBuffer& prmbuf ) ;
		// プリミティブリストを更新
		virtual SGLError UpdateIndexedPrimitiveList
			( size_t iMesh, uint32_t nFlags,
				size_t countIndex, size_t countVertex,
				const S3DVector4 * pvVertex,
				const S3DVector4 * pvNormal,
				const S2DVector * pvUVMap,
				const S3DColor * pColor,
				const uint32_t * pIndexedList ) ;
		// バッファを消去
		virtual void ClearBuffer( void ) ;
		// デバイスリソースを解放
		virtual void ReleaseAllDeviceResources( void ) ;

	protected:
		// 追加時処理 (デフォルトは頂点バッファ複製)
		virtual bool OnAddRenderBuffer
			( RENDER_ENTRY& entry,
				const S3DVector4 * pvVertex, const S3DVector4 * pvNormal,
				const S2DVector * pvUVMap, const S3DColor * pColor ) ;
		// 逆変換行列取得
		static void GetITransformation
			( ITransformation& itrans, Transformation * pTrans ) ;
		static void GetITransformationInstancing
			( ITransformation& itrans,
				Transformation * pTrans, const S4DMatrix * pmatInstancing ) ;
	public:
		// メッシュ階層スケール計算
		static size_t CalculateScaleForQuadSpherePolygons( size_t nPolygons ) ;
		// 当たり判定情報生成
		static void BuildTriangleListCollision
			( QuadSphereCollision * pqsc,
				QuadPolygonsCollision * pqpc,
				size_t nQSPackScale, size_t countPolygon,
				const S3DVector4 * pvVertex, const uint32_t * pIndexedList ) ;

	public:
		// メッシュツリー・一括構築開始宣言（複数回可）
		void BeginBatchBuild( void ) ;
		// メッシュツリー・一括構築終了（BeginBatchBuild と同じ回数呼び出す）
		void EndBatchBuild( void ) ;
		// メッシュツリー最適化（現在のツリーを再構築）
		void OptimizeMeshNodeTree( void ) ;

	protected:
		// MeshTreeNode* 配列からノードを構築
		MeshTreeNode * BuildNodeTreeFromArray
			( SSystem::SPointerArray<MeshTreeNode>& arrNodes ) ;
		// メッシュノード列挙
		static void EnumerateMeshNode
			( SSystem::SPointerArray<MeshTreeNode>& arrNodes, MeshTreeNode * pParentNode ) ;
		// メッシュノードサイズで昇順ソート
		static void SortNodeArray( MeshTreeNode** ppNodes, size_t nNodeCount ) ;
		// ノードを結合する
		MeshTreeNode * BindNodeTree( MeshTreeNode * pNode0, MeshTreeNode * pNode1 ) ;

	protected:
		// メッシュツリーにノードを追加
		void AddMeshNodeToTree( MeshCollision * pMeshCol ) ;
		void AddNodeToTree( MeshTreeNode * pAddNode ) ;
		// 子ノードとしてノードを挿入する
		void InsertNodeForTree
			( MeshTreeNode * pNode, size_t iChild, MeshTreeNode * pAddNode ) ;
		// pNode0 ノード位置に新規ノードを作成して pNode0, pNode1 の子を持つノードにする
		MeshTreeNode * BridgeNodeForTree( MeshTreeNode * pNode0, MeshTreeNode * pNode1 ) ;
		// メッシュのサイズ更新に伴うノード情報の更新
		void UpdateNodeForTree( MeshTreeNode * pNode ) const ;
		void UpdateMeshNodeForTree( MeshCollision * pMeshCol ) const ;
		// ノード中心座標・半径の計算
		static void CalcNodeSizeAndPosition( MeshTreeNode * pNode ) ;

	public:
		// ノード取得（MeshCollision.iMesh の番号）
		MeshCollision * GetMeshCollisionAt( size_t iMesh ) const ;
		// 全ノードを列挙
		void EnumerateAllMeshCollision
			( SSystem::SPointerArray<MeshCollision>& aMeshColls,
				uint64_t maskExcClasses = 0,
				uint64_t maskIncClasses = (uint64_t) -1 ) const ;
	protected:
		void EnumerateMeshCollisionOfNode
			( SSystem::SPointerArray<MeshCollision>& aMeshColls,
				const S3DCollision::MeshTreeNode * pNode,
				uint64_t maskExcClasses = 0,
				uint64_t maskIncClasses = (uint64_t) -1 ) const ;

	public:
		// ノードの正常性テスト
		bool VerifyMeshTree( void ) const ;
		bool VerifyMeshNode( S3DCollision::MeshTreeNode * pNode ) const ;

	public:
		// 単独メッシュ当たり判定オブジェクト
		class	MeshObject	: public MeshCollision, public S3DCollider
		{
		protected:
			void *	m_pBuffer ;

		public:
			// 構築関数
			MeshObject( void ) ;
			// 消滅関数
			~MeshObject( void ) ;
			// 当たり判定メッシュ構築
			SGLError CreateMesh
				( size_t countPolygon, size_t countVertex,
					const S3DVector4 * pvVertex,
					const uint32_t * pIndexedList ) ;
			// 当たり判定メッシュ更新
			SGLError UpdateMesh
				( size_t countPolygon, size_t countVertex,
					const S3DVector4 * pvVertex,
					const uint32_t * pIndexedList ) ;
			// 当たり判定メッシュ用バッファ解放
			void Release( void ) ;

		public:
			// 範囲取得
			virtual bool GetCollisionRange( S3DDVector& vCenter, double& fpRadius ) const ;
			// 凸形状の内側判定
			virtual bool IsSphereInclusive
				( const S3DDVector& vPos,
						float fpRadius, S3DCollisionResult& rsIncluded ) const ;
			// 球との交差判定（メッシュ内）
			virtual bool IsHitAgainstSphere
				( const S3DDVector& vPos,
						float fpRadius, S3DCollisionResult& rsHit ) const ;
			// 線分との交差判定（メッシュ内）
			virtual bool IsSegmentCrossing
				( const S3DDVector& vPos0, const S3DDVector& vPos1,
						float fpErrorGap, S3DCollisionResult& rsCross ) const ;
		} ;

		// メッシュ頂点の最大値・最小値取得
		static void MinMaxRangeOfMesh
			( S3DVector& vMin, S3DVector& vMax,
				const S3DVector4 * pvVertex, size_t countVertex ) ;

	} ;

	struct	S3DCollisionResult 
	{
		S3DVector		vHitLocal ;		// 交差点（ローカル座標系）
		S3DVector		vHitGlobal ;	// 交差点（グローバル座標系）
		S3DVector		vNormalLocal ;	// 法線（ローカル座標系）
		S3DVector		vNormal ;		// 法線（グローバル座標系）
		S3DVector		vReflection ;	// 反射ベクトル
		S2DVector		vLocalCoord ;	// △ABC の辺 AB, AC を基底とした交差座標
		S2DVector		vTexCoord ;		// テクスチャ座標
		S3DColor		vtxColor ;		// 頂点色
		S3DMaterial *	pMaterial ;		// マテリアル
		S3DMaterial::ColorAttribute
						attrTexture ;	// テクスチャサンプリング
		S3DMatrix		matLocal ;		// ローカル座標からグローバル座標へ変換する行列（全ての積）
		S3DVector		vLocalBase ;	// ローカル座標からグローバル座標へ変換する移動（全ての合成）
		float32_t		fpDistance ;	// 交差点までの距離
		size_t			iInstance ;		// 交差インスタンス指標
		size_t			iMesh ;			// 交差メッシュ指標
		size_t			iPolygon ;		// 交差ポリゴン指標
		const S3DCollision::MeshCollision *
						pMesh ;
		const S3DCollision::MeshCollision *
						pPrimitiveMesh ;
		const S3DCollision::QuadPolygonsCollision *
						pqpcHit ;
		uint64_t		maskExcClasses ;	// 当たり判定除外対象
		uint64_t		maskIncClasses ;	// 当たり判定対象
		ESLObject *		pExcUserData ;		// 当たり判定除外対象
		const S3DCollision::MeshCollision * const*
						ppExceptions ;	// 当たり判定除外
		size_t			nException ;
		S3DCollision::Callback_OnHitCollider
						pfnOnHitCollider ;
		void *			ptrOnHitInstance ;

		struct	ColliderChain
		{
			const ColliderChain *				pccParent ;
			const S3DCollision::MeshCollision *	pMeshCol ;

			ColliderChain( const S3DCollision::MeshCollision * pMesh = NULL )
				: pccParent(NULL), pMeshCol(pMesh) { }
		} ;
		ColliderChain *	pccChain ;

		class	ChainIterator
		{
		protected:
			const ColliderChain *	m_pChain ;
		public:
			ChainIterator( const S3DCollisionResult& rs ) : m_pChain( rs.pccChain ) { }
			bool HasNext( void ) const { return (m_pChain != nullptr) ; }
			const S3DCollision::MeshCollision * Next( void )
			{
				const S3DCollision::MeshCollision *	pMesh = nullptr ;
				if ( m_pChain != nullptr )
				{
					pMesh = m_pChain->pMeshCol ;
					m_pChain = m_pChain->pccParent ;
				}
				return	pMesh ;
			}
		} ;

		// 構築関数
		S3DCollisionResult( void ) ;
		S3DCollisionResult( const S3DCollisionResult& rs ) ;
		// 代入
		const S3DCollisionResult& operator = ( const S3DCollisionResult& rs ) ;
		// 除外クラス設定
		void ModifyExceptionSceneFlags( uint32_t nAddClasses, uint32_t nRemoveFlags ) ;
		void ModifyExceptionUserFlags( uint32_t nAddClasses, uint32_t nRemoveFlags ) ;
		void SetExceptionSceneFlags( uint32_t nClasses )
		{
			ModifyExceptionSceneFlags( nClasses, 0xFFFFFFFFUL ) ;
		}
		void SetExceptionUserFlags( uint32_t nClasses )
		{
			ModifyExceptionUserFlags( nClasses, 0xFFFFFFFFUL ) ;
		}
		uint32_t GetExceptionSceneFlags( void ) const ;
		uint32_t GetExceptionUserFlags( void ) const ;
		// 対象クラス設定
		void ModifyInclusionSceneFlags( uint32_t nAddClasses, uint32_t nRemoveFlags ) ;
		void ModifyInclusionUserFlags( uint32_t nAddClasses, uint32_t nRemoveFlags ) ;
		void SetInclusionSceneFlags( uint32_t nClasses )
		{
			ModifyInclusionSceneFlags( nClasses, 0xFFFFFFFFUL ) ;
		}
		void SetInclusionUserFlags( uint32_t nClasses )
		{
			ModifyInclusionUserFlags( nClasses, 0xFFFFFFFFUL ) ;
		}
		uint32_t GetInclusionSceneFlags( void ) const ;
		uint32_t GetInclusionUserFlags( void ) const ;
		// 除外判定
		bool IsException( S3DCollision::MeshCollision * pMeshCol ) const ;
		// チェイン追加・削除（STACK順序）
		void PushColliderChain( ColliderChain * pChain ) ;
		void PopColliderChain( ColliderChain * pChain ) ;
		// 当たり判定コールバック
		S3DCollision::HitColliderCallback OnHitCollider
				( const S3DVector& vHitPos, const S3DVector& vHitNormal,
					const S3DCollision::MeshCollision * pMesh, size_t iPolygon ) const
		{
			ESLVerify( !vHitPos.IsNaN() ) ;
			ESLVerify( !vHitNormal.IsNaN() ) ;
			return	(pfnOnHitCollider == NULL)
						? S3DCollision::hitColliderReturn
						: pfnOnHitCollider( *this, vHitPos, vHitNormal, pMesh, iPolygon ) ;
		}
		// 当たり判定コールバック関数内でコライダーマスクの判定を行う
		uint64_t TestColliderMask( const S3DCollision::MeshCollision * pMesh, uint64_t mask ) const ;
		uint32_t TestUserColliderMask( const S3DCollision::MeshCollision * pMesh, uint32_t maskUser ) const
		{
			return	S3DCollision::GetUserColliderMask
					( TestColliderMask
						( pMesh, S3DCollision::MakeUserColliderMask( maskUser ) ) ) ;
		}
		// 当たり判定コールバック関数内でユーザーデータを取得する
		ESLObject * GetUserDataOnHitCollider
			( const S3DCollision::MeshCollision * pMesh, const ESLRuntimeClass& rtClass ) const ;
		template <class T> T * GetUserDataOnHitCollider( const S3DCollision::MeshCollision * pMesh ) const
		{
			return	ESLTypeCast<T>( GetUserDataOnHitCollider( pMesh, ESL_RUNTIME_CLASS(T) ) ) ;
		}
		// 当たり判定コールバックに渡される座標空間からグローバルへの変換情報取得
		void GetHitColliderGlobalInfo
			( S3DCollision::HitColliderGlobalInfo& hcgi,
					const S3DCollision::MeshCollision * pMeshCol ) const
		{
			S3DCollision::ComputeHitColliderGlobalInfo( hcgi, *this, pMeshCol ) ;
		}
		// 当たり判定コールバックに渡されるパラメータを反映
		void SetParamOnHitCollider
			( const S3DCollision::HitColliderGlobalInfo& hcgi,
				const S3DVector& vHitPos, const S3DVector& vHitNormal,
				const S3DCollision::MeshCollision * pMeshCol, size_t iHitPolygon )
		{
			S3DCollision::SetResultParamOnHitCollider
				( *this, hcgi, vHitPos, vHitNormal, pMeshCol, iHitPolygon ) ;
		}
		// プリミティブメッシュ S3DCollision::MeshCollision を取得
		const S3DCollision::MeshCollision * GetPrimitiveMesh( void ) const
		{
			return	(pPrimitiveMesh != NULL) ? pPrimitiveMesh : pMesh ;
		}
		// △ABC の辺 AB, AC を基底とした交差座標を計算 -> vLocalCoord
		SGLError ComputeHitLocalCoord( void )
		{
			return	S3DCollision::ComputeHitLocalCoord( *this ) ;
		}
		// 法線補完計算 (ComputeHitLocalCoord 後) -> vNormalLocal
		SGLError ComplementeLocalNormal( void )
		{
			return	S3DCollision::ComplementeLocalNormal( *this ) ;
		}
		// UV座標計算 (ComputeHitLocalCoord 後) -> vTexCoord
		SGLError ComplementeTextureCoord( void )
		{
			return	S3DCollision::ComplementeTextureCoord( *this ) ;
		}
		// 頂点色計算 (ComputeHitLocalCoord 後) -> vtxColor
		SGLError ComplementeVertexColor( void )
		{
			return	S3DCollision::ComplementeVertexColor( *this ) ;
		}
		// テクスチャサンプリング (ComplementeTextureCoord 後)
		SGLError SampleDiffusionTexture( SGLPalette& rgbaTexture ) const
		{
			return	S3DCollision::SampleDiffusionTexture( *this, rgbaTexture ) ;
		}
		SGLError SampleLuminousTexture( SGLPalette& rgbaTexture ) const
		{
			return	S3DCollision::SampleLuminousTexture( *this, rgbaTexture ) ;
		}
		// 法線・UV・頂点色計算／各種テクスチャサンプリング (ComplementeTextureCoord 後)
		SGLError ComplementeAndSampleAttributes( void )
		{
			return	S3DCollision::ComplementeAndSampleAttributes( *this ) ;
		}

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 物理演算
	//////////////////////////////////////////////////////////////////////////

	class	S3DPhysicsScene	: public ESLObject
	{
	public:
		class	Actor ;

		// 環境
		class	Environment
		{
		public:
			S3DVector	m_vGravity ;		// 重力加速度 [/sec^2]
			S3DVector	m_vStream ;			// 空間流速 [/sec]
			float32_t	m_fpDensity ;		// 空間密度（空間抵抗計算用）
			float32_t	m_fpResistance ;	// 空間抵抗（減速率 [/sec]）

			Environment( void )
				: m_vGravity( 0, 9.8f, 0 ),
					m_vStream( 0, 0, 0 ),
					m_fpDensity( 0.01f ), m_fpResistance( 0.5f ) { }
			Environment( const Environment& env )
				: m_vGravity( env.m_vGravity ),
					m_vStream( env.m_vStream ),
					m_fpDensity( env.m_fpDensity ),
					m_fpResistance( env.m_fpResistance ) { }
			const Environment& operator = ( const Environment& env )
			{
				m_vGravity = env.m_vGravity ;
				m_vStream = env.m_vStream ;
				m_fpDensity = env.m_fpDensity ;
				m_fpResistance = env.m_fpResistance ;
				return	*this ;
			}
		} ;

		// オブジェクト
		class	Object	: public SGLObject
		{
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( Object, SGLObject )
			// 物理演算オブジェクト取得
			virtual S3DPhysicsScene::Actor * GetPhysicalActor( void ) const = 0 ;
			// オーナーオブジェクト取得
			virtual ESLObject * GetOwnerObject( void ) const = 0 ;
			// 座標取得
			virtual const S3DDVector&
						GetPhysicsPosition( S3DDVector& vPos ) const = 0 ;
			// 座標設定
			virtual void SetPhysicsPosition( const S3DDVector& vPos ) = 0 ;
			// 姿勢取得
			virtual const S3DQuaternion&
						GetPhysicsPosture( S3DQuaternion& qPosture ) const = 0 ;
			// 姿勢設定
			virtual void SetPhysicsPosture( const S3DQuaternion& qPosture ) = 0 ;
		} ;

		// 動的オブジェクト動作フラグ
		enum	ActorFlag
		{
			actorNoRotation			= 0x0001,
			actorNoMove				= 0x0002,
		} ;

		// 物性
		struct	Material
		{
			uint32_t			nFlags ;			// enum ActorFlag 集合
			float32_t			fpWeight ;			// 質量
			float32_t			fpRotWeight ;		// 回転質量（比）
			float32_t			fpVolume ;			// 体積（空間流影響度）
			float32_t			fpStillFriction ;	// 静止摩擦力（この力以上で接地先から外れる）
			float32_t			fpResistance ;		// 摩擦抵抗 F = R*W
			float32_t			fpElasticity  ;		// 弾性率   V'= V*E
		} ;
		static const Material	m_materialDefault ;

		// 動的オブジェクト
		class	Actor	: public Object
		{
		public:
			ESLObject *			m_pOwnerObj ;
			const Material *	m_pMaterial ;		// 物性
			S3DDVector			m_vPos ;			// 位置
			S3DDVector			m_vLastPos ;		// 位置（前フレーム）
			S3DVector			m_vSpeed ;			// 速度 [/sec]
			S3DVector			m_vLastSpeed ;		// 速度 [/sec]（前フレーム）
			S3DQuaternion		m_qPosture ;		// 姿勢（回転行列）
			S3DQuaternion		m_qLastPosture ;	// 姿勢（前フレーム）
			S3DVector			m_vRotate ;			// 回転軸
			float32_t			m_fpRotSpeed ;		// 回転速度 [rad/sec]
			S3DVector			m_vZoom ;			// 表示用拡大率

			const S3DVector4 *	m_pHitPoints ;		// 当たり判定座標（d は半径）
			size_t				m_nHitPointCount ;	// 指標 0 は接地判定用

		protected:
			SSystem::SPointerArray<ESLObject>
								m_arrCurrentHit ;	// 現フレームで衝突処理済み対象

		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( Actor, Object )
			// 構築関数
			Actor( ESLObject * pOwner = nullptr ) ;
			Actor( const Actor& act ) ;

		public:
			// （物理演算以外の）イベント当たり判定ハンドラ (false の場合衝突処理はしない）
			virtual bool OnHitEventCollider
				( const S3DCollisionResult& rsHit, Object * pHit,
					const S3DVector& vHitPos, const S3DVector& vHitNormal,
					const S3DCollision::MeshCollision * pMesh, size_t iPolygon ) ;

		public:
			// 衝突時ハンドラ
			virtual bool OnCollidedWith
				( Object * pObj,
					const S3DDVector& vImpactPos,
					const S3DVector& vActNormal, float32_t secAdv ) ;
			// 動的環境
			virtual void DynamicEnvironment( Environment& env ) ;

		public:
			// 慣性運動／接地先への追従
			virtual void KineticMove( const Environment& env, float32_t secAdv ) ;
			// 衝突計算
			void Collide
				( const S3DDVector& vImpactPos,
					const S3DVector& vActNormal, float32_t secAdv ) ;
			void CollideWith
				( const S3DDVector& vImpactPos,
					const S3DVector& vActNormal, float32_t secAdv,
					const S3DVector& vActSpeed, float32_t fpActElasticity ) ;
			void CollideWithObject
				( S3DPhysicsScene& scene, Object& obj,
					const S3DDVector& vImpactPos,
					const S3DVector& vActNormal, float32_t secAdv ) ;
			void CollideWithActor
				( S3DPhysicsScene& scene, Actor& act,
					const S3DDVector& vImpactPos,
					const S3DVector& vActNormal, float32_t secAdv ) ;
			// 回転速度を合成
			void AddRotate
				( const S3DVector& vRotate, float32_t fpRotSpeed ) ;
			// 回転速度を計算
			static void CalcRotation
				( S3DVector& vRotate, float32_t& fpRotSpeed,
					const S3DVector& vOffsetPos,
					const S3DVector& vSpeed, float32_t fpRadius ) ;

		public:
			// ソート処理用
			bool operator < ( const Actor& act ) const
			{
				return	((ulong_ptr_t) this) < ((ulong_ptr_t) &act) ;
			}
			bool operator > ( const Actor& act ) const
			{
				return	((ulong_ptr_t) this) > ((ulong_ptr_t) &act) ;
			}
			bool operator == ( const Actor& act ) const
			{
				return	((ulong_ptr_t) this) == ((ulong_ptr_t) &act) ;
			}
			bool operator != ( const Actor& act ) const
			{
				return	((ulong_ptr_t) this) != ((ulong_ptr_t) &act) ;
			}

		public:	// S3DPhysicsScene::Object
			// 物理演算オブジェクト取得
			virtual S3DPhysicsScene::Actor * GetPhysicalActor( void ) const ;
			// オーナーオブジェクト取得
			virtual ESLObject * GetOwnerObject( void ) const ;
			// 座標取得
			virtual const S3DDVector&
						GetPhysicsPosition( S3DDVector& vPos ) const ;
			// 座標設定
			virtual void SetPhysicsPosition( const S3DDVector& vPos ) ;
			// 姿勢取得
			virtual const S3DQuaternion&
						GetPhysicsPosture( S3DQuaternion& qPosture ) const ;
			// 姿勢設定
			virtual void SetPhysicsPosture( const S3DQuaternion& qPosture ) ;

			friend class S3DPhysicsScene ;
		} ;

	protected:
		// 環境
		Environment		m_env ;

		// アクター当たり判定
		S3DCollision	m_collision ;

		// アクター配列
		SSystem::SPointerArray<Actor>	m_actors ;

		// アクター並列処理時排他処理用
		SSystem::SCriticalSection	m_csActor ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DPhysicsScene, ESLObject )
		// 構築関数
		S3DPhysicsScene( void ) ;
		// 消滅関数
		virtual ~S3DPhysicsScene( void ) ;

	public:
		// 環境設定
		void SetEnvironment( const Environment& env ) ;
		// 環境取得
		const Environment& GetEnvironment( void ) const
		{
			return	m_env ;
		}

	public:	// アクター当たり判定
		// アクター用当たり判定バッファ
		S3DCollision& Collision( void )
		{
			return	m_collision ;
		}
		const S3DCollision& GetCollision( void ) const
		{
			return	m_collision ;
		}
		// ソリッド球を追加
		SGLError AddSolidSphere
			( Actor * pAct, const S3DVector& vPos,
				float32_t fpRadius, size_t iInstanceNum ) ;
		// ソリッド直方体を追加
		SGLError AddSolidCube
			( Actor * pAct, const S3DVector& vPos,
				const S3DVector& vCubeSize, size_t iInstanceNum ) ;
		// コライダーバッファを追加
		SGLError AddColliderObject
			( Actor * pAct, S3DCollider * pCollider,
				const S4DMatrix * pmatInstancing, size_t iInstanceNum ) ;

	public:
		// スレッド排他処理
		void Lock( void ) ;
		void Unlock( void ) ;

	public:
		// アクター追加
		void AddActor( Actor * pAct ) ;
		void AddActors( Actor *const* ppAct, size_t nCount ) ;
		// アクター総数取得
		size_t GetActorsCount( void ) const ;
		// アクター取得
		Actor * GetActorAt( size_t nIndex ) const ;
		// アクター指標検索
		ssize_t FindActor( Actor * pAct ) const ;
		// アクター削除
		void DetachAllActor( void ) ;

	public:
		// 時間進行処理
		struct	ErrorGap
		{
			float32_t	fpHitGap ;
			float32_t	fpGroundGap ;
		} ;
		void AdvanceTime
			( S3DCollider& collider, const ErrorGap& eg, float32_t secAdv ) ;

	public:
		// 並列処理
		enum	ConstantValue
		{
			MAX_THREADS	= 32,
		} ;
		struct	CollisionProcInstance
		{
			Actor *	pAct ;
		} ;
		class	CollisionProc : public SSystem::SParallelProcedure
		{
		protected:
			S3DPhysicsScene *	m_pScene ;
			S3DCollider&		m_colider ;
			ErrorGap			m_errGap ;
			float32_t			m_secAdv ;
			size_t				m_iNextAct ;
			size_t				m_nActCount ;
			Actor*const*		m_ppActors ;

		public:
			// 構築関数
			CollisionProc
				( S3DPhysicsScene * pScene,
					S3DCollider& collider, const ErrorGap& eg, float32_t secAdv,
					size_t nActCount, Actor*const* ppActors ) ;
			// ループ処理／終了判定関数
			virtual bool Continue( void * pInstance ) ;
			// 並列処理関数
			virtual void RunParallel( void * pInstance ) ;
		} ;

	public:
		// アクター当たり判定スレッド排他処理
		void LockActor( void ) ;
		void UnlockActor( void ) ;
		// アクターの衝突処理済みか？
		bool HasCollidedWith( Actor * pAct1, Actor * pAct2 ) const ;

	protected:
		struct	ActorHitDescEntry
		{
			bool		flagMove ;
			float32_t	fpDistance ;
			size_t		iHitPoint ;
			Object *	pHitObject ;
			S3DVector	vHitPos ;
			S3DVector	vHitDelta ;
			S3DVector	vHitNormal ;
		} ;
		enum	ActorHitDescTableConstant
		{
			ActorHitDescTableLength	= 3,
		} ;
		struct	ActorHitDescTable
		{
			S3DPhysicsScene *	pScene ;
			Actor *				pThisAct ;
			S3DVector			vMoveDelta ;
			size_t				nEntryCount ;
			ActorHitDescEntry	ahdeTable[ActorHitDescTableLength] ;
		} ;
		// 当たり判定テーブルに追加
		static bool AddActorHitDescriptor
			( ActorHitDescTable& ahdt,
				const S3DCollisionResult& rsHit,
				bool flagMove, float32_t fpDistance, size_t iHitPoint,
				const S3DVector& vHitPos,
				const S3DVector& vHitDelta,
				const S3DVector& vHitNormal ) ;
		// 当たり判定関数
		static S3DCollision::HitColliderCallback
			Callback_MoveTestOnHitCollider
				( const S3DCollisionResult& rsHit,
					const S3DVector& vHitPos, const S3DVector& vHitNormal,
					const S3DCollision::MeshCollision * pMesh, size_t iPolygon ) ;
		static S3DCollision::HitColliderCallback
			Callback_SphereTestOnHitCollider
				( const S3DCollisionResult& rsHit,
					const S3DVector& vHitPos, const S3DVector& vHitNormal,
					const S3DCollision::MeshCollision * pMesh, size_t iPolygon ) ;
	} ;

}

#endif
