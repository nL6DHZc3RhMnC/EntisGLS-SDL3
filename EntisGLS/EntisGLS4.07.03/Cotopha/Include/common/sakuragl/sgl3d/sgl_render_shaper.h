
#if	!defined(__SAKURAGL_RENDER_SHAPER_H__)
#define	__SAKURAGL_RENDER_SHAPER_H__

#include <sakuragl/sgl3d/sglh3d_stddef.h>
#include <sakuracl/erisa/sgl_erisa_crypt_math.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// 形状生成オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	S3DMeshShaper	: public ESLObject
	{
	protected:
		SSystem::SArray<S3DVector4>		m_bufVertex ;
		SSystem::SArray<S3DVector4>		m_bufNormal ;
		SSystem::SArray<S2DVector>		m_bufUVMap ;
		SSystem::SArray<S3DColor>		m_bufColor ;
		SSystem::SArray<uint32_t>		m_bufIndex ;
		SSystem::SArray<float32_t>		m_bufThickness ;
		SSystem::SArray<SGLImageRect>	m_bufRect ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DMeshShaper, ESLObject )
		// 構築関数
		S3DMeshShaper( void ) ;
		// 消滅関数
		virtual ~S3DMeshShaper( void ) ;

	public:	// 格子状メッシュ
		// 格子状メッシュフラグ
		enum	GridMeshFlag
		{
			gridHorzLoop	= 0x00000001,
			gridVertLoop	= 0x00000002,
			gridTopTip		= 0x00000010,
			gridBottomTip	= 0x00000020,
			gridBackface	= 0x00000040,
		} ;
		// 格子状メッシュを生成
		SGLError GridMesh
			( S3DRenderBufferInterface & render,
				S3DMaterial * pMaterial, uint32_t nFlags,
				size_t widthMesh, size_t heightMesh,
				const S3DVector4 * pvVertex,
				const S3DVector4 * pvNormal,
				const S2DVector * pvUVMap, const S3DColor * pColor ) ;

		struct	MeshPrimitiveCount
		{
			size_t	countPolygon ;
			size_t	countVertex ;
		} ;
		struct	GridMeshParam
		{
			uint32_t	nFlags ;
			size_t		widthMesh ;
			size_t		heightMesh ;
		} ;
		struct	GridMeshGenParams	: public MeshPrimitiveCount
		{
			size_t	iGridBodyFirst ;
			size_t	iGridBodyEnd ;
			size_t	countBodyLines ;
		} ;
		static SGLError AddGridMesh
			( S3DVertexBufferInterface & vbuf,
				const GridMeshGenParams& gmgp,
				const GridMeshParam& gmp,
				const S3DVertexBufferInterface::PrimitiveBuffer& prmbuf ) ;
		static size_t CalcGridMeshPolygonCount
			( GridMeshGenParams& gmgp,
				uint32_t nFlags, size_t widthMesh, size_t heightMesh ) ;
		static void MakeGridMeshIndex
			( uint32_t nFlags,
				size_t widthMesh, size_t heightMesh,
				const GridMeshGenParams& gmgp, uint32_t * pIndexedBuf ) ;

	public:	// 凸でない頂点を含む多角形の分割
		// 凸でない頂点を含む多角形を三角形リストに分割したインデックスリスト
		static size_t MakeTriangleIndexedList
			( SSystem::SArray<uint32_t>& aIndexedList,
				SSystem::SArray<uint32_t>& aWorkIndex,
				const S3DVector4 * pvVertex, size_t nVertexCount ) ;
	protected:
		static bool IsValidTriangleFace
			( uint32_t iTriangleTop, const uint32_t * pVertexChain,
				const S3DVector& vValidNormal, const S3DVector4 * pvVertex ) ;
		static uint32_t FindValidTriangle
			( uint32_t iv0, uint32_t iv1, uint32_t iv2,
				const uint32_t * pVertexChain,
				const S3DVector& vValidNormal,
				const S3DVector4 * pvVertex, float32_t fpErrorGap ) ;
		static bool IsTriangleIncludeOtherPoint
			( uint32_t iTriangleTop,
				const uint32_t * pVertexChain,
				const S3DVector& vNormal,
				const S3DVector4 * pvVertex, float32_t fpErrorGap ) ;

	public:	// 太さを付加した連続線分
		// 太さを付加した線分フラグ
		enum	ThickLinesFlag
		{
			thickLineLoop	= 0x00000001,	// 視点と終端が接続している
		} ;
		// 太さを付加した線分パラメータ
		struct	ThickLinesParam
		{
			uint32_t			nFlags ;	// フラグ組み合わせ enum ThickLinesFlag
			float32_t			uWidth ;	// テクスチャ幅（線の厚み方向）
			float32_t			vRatio ;	// テクスチャＶ・空間距離比
			float32_t			vOffset ;	// テクスチャＶオフセット
			float32_t			zBais ;		// 視線方向へのｚバイアス
			const S3DColor *	pColors ;	// 線の中心から外側の順で色指定（省略可）
			size_t				nColorDiv ;	// 線の色指定数
			const float32_t *	pThickDiv ;	// 太さの分割位置 [0,1]

			ThickLinesParam( void )
				: nFlags(0), uWidth(1.0f), vRatio(1.0f),
					vOffset(0.0f), zBais(0.0f),
					pColors(NULL), nColorDiv(2), pThickDiv(NULL) {}
		} ;
		// 太さを付加した線分を生成
		SGLError ThickLines
			( S3DRenderBufferInterface & render,
				const S3DVector & vCameraRay,
				S3DMaterial * pMaterial,
				const ThickLinesParam & tlpParam,
				size_t nPoints,
				const S3DVector4 * pvPoints,
				const float32_t * pThickness,
				const uint32_t * pAlphas = NULL ) ;
		static SGLError AddThickLines
			( S3DVertexBufferInterface & vbuf,
				const S3DVector & vCameraRay,
				const ThickLinesParam & tlpParam,
				size_t nPoints,
				const S3DVector4 * pvPoints,
				const float32_t * pThickness,
				const uint32_t * pAlphas = NULL ) ;
		// （見かけ上の）太さを付加した線分を生成
		SGLError RegularThickLines
			( S3DRenderBufferInterface & render,
				const S3DVector & vCameraRay,
				const S3DVector & vCameraPos,
				S3DMaterial * pMaterial,
				const ThickLinesParam & tlpParam,
				size_t nPoints, float32_t fpThicknessByZ,
				const S3DVector4 * pvPoints,
				const uint32_t * pAlphas = NULL ) ;

		static size_t CalcThickLinesPolygonCount
			( GridMeshGenParams& gmgp,
				GridMeshParam& gmp,
				const ThickLinesParam & tlpParam, size_t nPoints ) ;
		static void MakeThickLinesGridMesh
			( const S3DVertexBufferInterface::PrimitiveBuffer& prmbuf,
				const S3DVector & vCameraRay,
				const ThickLinesParam & tlpParam,
				size_t nPoints,
				const S3DVector4 * pvPoints,
				const float32_t * pThickness,
				const uint32_t * pAlphas = NULL ) ;

	public:	// ビルボード
		// ビルボードパラメータ
		struct	BillboardParam
		{
			SGLImageObject *	pImage ;	// ビルボード画像
			S2DVector			vCenter ;	// 画像中心座標
			S2DVector			vZoom ;		// 拡大率
			float32_t			zAngle ;	// 回転角 [deg]
			float32_t			zBias ;		// 視線方向へのｚバイアス

			BillboardParam( void )
				: pImage(NULL), vZoom(1,1), zAngle(0.0f), zBias(0.0f) {}
			void CalcVertex( S3DVector4 * pvVertex ) const ;
			void CalcVertex
				( S3DVector4 * pvVertex, const S3DMatrix & matICamera ) const ;
		} ;
		// ビルボードパーティクルを生成
		SGLError BillboardParticle
			( S3DRenderBufferInterface & render,
				const S3DMatrix & matICamera,
				const S3DVector & vCameraPos,
				uint64_t nDrawShadingFlag,
				const BillboardParam & bpParam,
				size_t nPoints,
				const S3DVector4 * pvPoints,
				const S3DColor * pColors = NULL,
				const float32_t * pZooms = NULL,
				const S4DVector * pFaceDirs = NULL,
				const float32_t * pxAspect = NULL ) ;
		// アニメーション画像ビルボードパーティクルを生成
		SGLError AnimationBillboardParticle
			( S3DRenderBufferInterface & render,
				const S3DMatrix & matICamera,
				const S3DVector & vCameraPos,
				uint64_t nDrawShadingFlag,
				const BillboardParam & bpParam,
				size_t nPoints,
				const S3DVector4 * pvPoints,
				const size_t * pFrames,
				const S3DColor * pColors = NULL,
				const float32_t * pZooms = NULL,
				const S4DVector * pFaceDirs = NULL,
				const float32_t * pxAspect = NULL ) ;

		SGLImageRect * GetImageRectBuffer( size_t nFrameCount ) ;
		static void MakeAnimationBillboardParticle
			( const S3DVertexBufferInterface::PrimitiveBuffer& prmbuf,
				const S3DMatrix & matICamera,
				const S3DVector & vCameraPos,
				const BillboardParam & bpParam,
				const SGLImageRect * pFrameRect,
				size_t nFrameCount,
				size_t nPoints,
				const S3DVector4 * pvPoints,
				const size_t * pFrames,
				const S3DColor * pColors = NULL,
				const float32_t * pZooms = NULL,
				const S4DVector * pFaceDirs = NULL,
				const float32_t * pxAspect = NULL ) ;

		// 汎用ビルボード
		struct	BillboardEntry
		{
			SGLImageObject *	pImage ;	// 画像
			ssize_t				iFrame ;	// フレーム番号
			SGLImageRect		rectImage ;	// 画像表示領域
			S2DVector			vOffset ;	// 画像中心位置
			S3DVector			vPosition ;	// 位置
			S3DColor			clrEffect ;	// 色効果とα
			S2DVector			vZoom ;		// 拡大率
			S4DVector			vFaceDir ;	// 法線(x,y,z)とz回転(w)[rad]
		} ;
		enum	BillboardRenderFlag
		{
			billboardWithFaceDir	= 0x0001,	// ビルボードごとの法線とz回転有効
		} ;
		struct	BillboardRenderParam
		{
			S3DMatrix	matICamera ;		// カメラ逆変換
			S3DVector	vCameraPos ;		// カメラ座標
			S2DVector	vZoom ;				// 拡大率
			float32_t	zAngle ;			// 回転角 [deg]
			float32_t	zBias ;				// 視線方向へのｚバイアス（＋は奥方向）
			uint32_t	flagsBillboard ;	// enum BillboardRenderFlag 組み合わせ
			uint64_t	flagsShadingOpt ;	// shadingTextureSmoothing, shadingVertexAlpha,
											// shadingZBufferNoWrite, shadingNoZBuffer 等
		} ;
		SGLError RenderMultiBillboards
			( S3DRenderBufferInterface & render,
				const BillboardRenderParam& brp,
				size_t nBillboardCount,
				const BillboardEntry * pBillboards ) ;
		SGLError RenderBillboardsOfAtlasImage
			( S3DRenderBufferInterface & render,
				const BillboardRenderParam& brp,
				SGLImageObject * pAtlasImage,
				size_t nBillboardCount,
				const SGLImageRect * pAtlasRects,
				const BillboardEntry * pBillboards ) ;

	public:	// 立方体
		// 立方体を生成
		static SGLError Cube
			( S3DRenderBufferInterface & render,
				S3DMaterial * pMaterial,
				const S3DVector & vPosition,
				const S3DVector & vSize ) ;

	public:	// 太さのあるリング
		// 太さのあるリングパラメータ
		struct	ThickRingParam
		{
			S3DVector			vCenter ;		// 中心
			S3DVector			vNormal ;		// 法線ベクトル
			float32_t			fpRadius ;		// 半径（内側）
			float32_t			fpThickness ;	// 太さ
			S2DVector			vZoom ;			// リングの縦横拡大率
			S2DVector			uvScale ;		// テクスチャスケール
												//（円周方向がｘ、厚み方向がｙ）
												//（ループしない場合は画像サイズ）
			size_t				nDivision ;		// 円周方向分割数
			const S3DColor *	pColors ;		// リングの外から内側の順で色指定（省略可）
			size_t				nColorDiv ;		// 色指定数

			ThickRingParam( void )
				: vNormal(0,0,1), fpRadius(0.0f), fpThickness(1.0f),
					vZoom(1,1), uvScale(1,1),
					nDivision(32), pColors(NULL), nColorDiv(2) {}
		} ;
		// 太さのあるリングを生成
		SGLError ThickRing
			( S3DRenderBufferInterface & render,
				S3DMaterial * pMaterial,
				const ThickRingParam & trpParam ) ;
		static SGLError AddThickRing
			( S3DVertexBufferInterface & vbuf,
				const ThickRingParam & trpParam ) ;

		static size_t CalcThickRingPolygonCount
			( GridMeshGenParams& gmgp,
				GridMeshParam& gmp,
				const ThickRingParam & trpParam ) ;
		static void MakeThickRingGridMesh
			( const S3DVertexBufferInterface::PrimitiveBuffer& prmbuf,
				const ThickRingParam & trpParam ) ;

	public:	// 円柱
		// 円柱パラメータ
		struct	CylinderParam
		{
			S3DVector	vCenter ;		// 底の中心
			S3DVector	vDirection ;	// 柱の立ち上げ方向
			float32_t	fpRadius ;		// 底の半径
			float32_t	fpHeight ;		// 柱の高さ
			size_t		hDivision ;		// 底の分割数
			size_t		vDivision ;		// 柱の分割数
			S2DVector	uvScale ;		// テクスチャスケール（ループしない場合は画像サイズ）
			S2DVector	uvOffset ;		// テクスチャオフセット
			S3DColor	colorBase ;		// 頂点共通色

			CylinderParam( void )
				: vDirection(0,-1,0),
					fpRadius(1.0f), fpHeight(1.0f),
					hDivision(24), vDivision(1),
					uvScale(1,1), uvOffset(0,0), colorBase(0xFFFFFFFF,0) {}
		} ;
		// 円柱を生成
		SGLError Cylinder
			( S3DRenderBufferInterface & render,
				S3DMaterial * pMaterial,
				const CylinderParam & cpParam, bool fBackface = false ) ;
		static SGLError AddCylinder
			( S3DVertexBufferInterface & vbuf,
				const CylinderParam & cpParam, bool fBackface = false ) ;

		static size_t CalcCylinderPolygonCount
			( GridMeshGenParams& gmgp,
				GridMeshParam& gmp,
				const CylinderParam & cpParam, bool fBackface = false ) ;
		static void MakeCylinderGridMesh
			( const S3DVertexBufferInterface::PrimitiveBuffer& prmbuf,
				const CylinderParam & cpParam, bool fBackface = false ) ;

	public:	// 球
		// 球パラメータ
		struct SphereParam ;
		typedef void (*PFUNC_SPHERE_COLOR_MAP)
			( S3DColor& clrVertex,
				void * pInstance,
				const SphereParam& spParam,
				double radX, double radY ) ;
		struct	SphereParam
		{
			float32_t	fpRadius ;		// 半径
			S3DVector	vSizeScale ;
			float32_t	degStartLatitude ;	// 開始緯度 [deg]
			float32_t	degEndLatitude ;	// 終了緯度 [deg]
			size_t		hDivision ;		// 赤道方向分割数
			size_t		vDivision ;		// 垂直方向分割数
			S2DVector	uvScale ;		// テクスチャスケール（ループしない場合は画像サイズ）
			S2DVector	uvOffset ;		// テクスチャオフセット
			S3DColor	colorBase ;		// 頂点共通色
			PFUNC_SPHERE_COLOR_MAP
						pfnColorMap ;	// 頂点色を求めたい場合の関数
			void *		pColorMapInstance ;

			SphereParam( void )
				: fpRadius(1.0f),
					vSizeScale(1.0f,1.0f,1.0f),
					degStartLatitude(90.0f),
					degEndLatitude(-90.0f),
					hDivision(24), vDivision(12),
					uvScale(1,1), uvOffset(0,0),
					colorBase(0xFFFFFFFF,0),
					pfnColorMap(NULL), pColorMapInstance(NULL) {}
		} ;
		// 球を生成
		SGLError Sphere
			( S3DRenderBufferInterface & render,
				S3DMaterial * pMaterial,
				const SphereParam & spParam, bool fBackface = false ) ;
		static SGLError AddSphere
			( S3DVertexBufferInterface & vbuf,
				const SphereParam & spParam, bool fBackface = false ) ;

		static size_t CalcSpherePolygonCount
			( GridMeshGenParams& gmgp,
				GridMeshParam& gmp,
				const SphereParam & spParam, bool fBackface = false ) ;
		static void MakeSphereGridMesh
			( const S3DVertexBufferInterface::PrimitiveBuffer& prmbuf,
				const SphereParam & spParam, bool fBackface = false ) ;

public:	// チューブ
		// チューブフラグ
		enum	TubeFlag
		{
			tubeCapHead		= 0x0001,	// 始点に半球を付加する
			tubeCapTail		= 0x0002,	// 終点に半球を付加する
		} ;
		// チューブパラメータ
		struct	TubeParam
		{
			uint32_t	nFlags ;		// フラグ集合
			float32_t	fpRadius ;		// 半径
			size_t		hDivision ;		// 周囲分割数
			size_t		vDivision ;		// 終端半球分割数
			S2DVector	uvScale ;		// テクスチャスケール
										// （ｘは周囲方向、ｙは空間距離比）
			S2DVector	uvOffset ;		// テクスチャオフセット
			float32_t	vTerminal ;		// テクスチャ終端半球部区間ｙ座標
			S3DColor	colorBase ;		// 頂点共通色

			TubeParam( void )
				: nFlags(0), fpRadius(1.0f), hDivision(16), vDivision(8),
					uvScale(1,1), uvOffset(0,0), vTerminal(1.0f), colorBase(0xFFFFFFFF,0) {}
		} ;
		// チューブを生成
		SGLError Tube
			( S3DRenderBufferInterface & render,
				S3DMaterial * pMaterial,
				const TubeParam & tpParam,
				const S3DVector& vHandle,
				size_t nPoints, const S3DVector4 * pvPoints,
				const float32_t * pThickness = NULL,
				const uint32_t * pAlphas = NULL, bool fBackface = false ) ;
		static SGLError AddTube
			( S3DVertexBufferInterface & vbuf,
				const TubeParam & tpParam,
				const S3DVector& vHandle,
				size_t nPoints, const S3DVector4 * pvPoints,
				const float32_t * pThickness = NULL,
				const uint32_t * pAlphas = NULL, bool fBackface = false ) ;

		static size_t CalcTubePolygonCount
			( GridMeshGenParams& gmgp,
				GridMeshParam& gmp,
				const TubeParam & tpParam,
				size_t nPoints,  bool fBackface = false ) ;
		static void MakeTubeGridMesh
			( const S3DVertexBufferInterface::PrimitiveBuffer& prmbuf,
				const TubeParam & tpParam, const S3DVector& vHandle,
				size_t nPoints, const S3DVector4 * pvPoints,
				const float32_t * pThickness = NULL,
				const uint32_t * pAlphas = NULL, bool fBackface = false ) ;
		static void CalcDefaultTubeHandle
			( S3DVector& vHandle,
				size_t nPoints, const S3DVector4 * pvPoints ) ;

	public:	// 稲妻生成用
		// 稲妻パラメータ
		struct	ThunderParam
		{
			double	fpJointEffect ;		// 稲妻ジグザグ振幅
			double	fpJointLen ;		// 稲妻ジグザグ波長
			double	fpWaveEffect ;		// 稲妻効果のより長い振幅
			double	fpWaveLen ;			// 稲妻効果のより長い波長
		} ;
		// 稲妻生成器（連続線分→稲妻）
		class	ThunderContext
		{
		protected:
			SSystem::SArray<S3DVector4>	m_aPoint ;		// 稲妻頂点座標
			SSystem::SArray<double>		m_aIndex ;		// 各頂点に対応する元線分指標
														// （少数は線分内位置に対応）
			size_t						m_index ;
			S3DVector					m_vLastPos ;
			double						m_fpNextAmpX ;
			double						m_fpNextAmpY ;
			double						m_fpLastAmpX ;
			double						m_fpLastAmpY ;
			double						m_fpPhase ;
		public:
			// 構築関数
			ThunderContext( void ) ;
			// 稲妻生成
			void Create
				( SakuraCL::SCLRandomizer& randomizer,
					const ThunderParam& param,
					size_t nPointCount, const S3DDVector * pvPoints ) ;
			// 初期設定
			void InitContext
				( SakuraCL::SCLRandomizer& randomizer,
					const S3DDVector& vTrack0, const ThunderParam& param ) ;
			// 次の点（元線分）追加
			void NextPoint
				( SakuraCL::SCLRandomizer& randomizer,
					const S3DDVector& vTrackX, const ThunderParam& param ) ;
			// 次の指標（元線分）
			size_t GetNextIndex( void ) const ;
			// 指標削除
			void DecreaseIndex( size_t nDec ) ;
			// 生成された稲妻
			size_t GetPointCount( void ) const ;
			const S3DVector4 * GetPointArray( void ) const ;
			const double * GetIndexArray( void ) const ;
		} ;

	} ;



	//////////////////////////////////////////////////////////////////////////
	// 多面体（正20面体 / 20*4^n 面体 / 切頂多面体）生成オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	S3DPolyhedronMesh	: public ESLObject
	{
	public:
		// 定数
		enum	ConstantValue
		{
			vertexMaxEdge	= 6,
		} ;

		// 三角ポリゴン
		struct	Triangle
		{
			size_t	iVertex[3] ;			// 三角ポリゴンの頂点指標
		} ;

		// 頂点からの稜線結合
		struct	VertexEdge
		{
			size_t	nEdge ;					// 接続頂点数
			size_t	iEdge[vertexMaxEdge] ;	// 接続先頂点指標

			// 稜線を検索
			ssize_t Find( size_t iVertex ) const ;
			// 稜線を追加
			void AddEdge( size_t iVertex ) ;
		} ;

		// 切頂面
		struct	TruncatedFace
		{
			size_t		nVertex ;
			S3DVector	vCenter ;
			S3DVector	vVertex[vertexMaxEdge] ;
		} ;

		// 変型インターフェース
		class	Modifier
		{
		public:
			// 三角ポリゴンリスト変型
			virtual void ModifyTriangles
				( S3DVector4 * pvVertex,
					S3DVector4 * pvNormal,
					S2DVector * pvUVMap,
					S3DColor * pColor,
					size_t countVertex ) = 0 ;
			// 多角形ポリゴン変型
			virtual void ModifyFace
				( S3DVector4 * pvVertex,
					S3DVector4 * pvNormal,
					S2DVector * pvUVMap,
					S3DColor * pColor,
					size_t countVertex,
					const TruncatedFace& face ) = 0 ;
		} ;

		// 三角多面体
		class	TrianglePolyhedron
		{
		public:
			SSystem::SArray<S3DVector>	m_aVertex ;		// 頂点
			SSystem::SArray<Triangle>	m_aFace ;		// 三角面
			SSystem::SArray<VertexEdge>	m_aEdge ;		// 頂点の稜線結合
			SSystem::SArray<VertexEdge>	m_aEdgeTemp ;
			SSystem::SArray<VertexEdge>	m_aDivEdge ;	// 稜線の中間で分割した頂点指標
		public:
			// 正20面体生成
			void CreateIcosahedron( void ) ;
			// 分割（三角を4分割）多面体生成
			void DividedTriangle( void ) ;
		protected:
			// 稜線情報追加
			void AddEdge( size_t iVertex0, size_t iVertex1 ) ;
		public:
			// メッシュ出力
			void RenderMesh( S3DVertexBufferInterface& vb, Modifier * pModifier ) ;
		} ;

		// 切頂多面体
		class	TruncatedPolyhedron
		{
		public:
			SSystem::SArray<TruncatedFace>	m_aFace ;
		public:
			// 切頂多面体生成
			void CreateTruncatedFace( const TrianglePolyhedron& tp ) ;
			// メッシュ出力
			void RenderMesh( S3DVertexBufferInterface& vb, Modifier * pModifier ) ;
		} ;

	protected:
		bool				m_flagTruncated ;
		TrianglePolyhedron	m_triangles ;
		TruncatedPolyhedron	m_truncated ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( S3DPolyhedronMesh, ESLObject )
		// 構築関数
		S3DPolyhedronMesh( void ) ;
		// 形状生成
		void CreatePolyhedron( size_t nDivCount, bool flagTruncate ) ;
		// メッシュ出力
		void RenderMesh( S3DVertexBufferInterface& vb, Modifier * pModifier ) ;

	} ;

}

#endif
