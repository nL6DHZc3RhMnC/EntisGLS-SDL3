
/*****************************************************************************
                          Entis Graphic Library
 -----------------------------------------------------------------------------
     Copyright (c) 2008-2012 Leshade Entis, Entis-soft. Al rights reserved.
 *****************************************************************************/


//////////////////////////////////////////////////////////////////////////////
// 布シミュレーション実装抽象オブジェクト
//////////////////////////////////////////////////////////////////////////////

class	EGLClothModelMorphObject	: public EGL_CLOTH_MODEL_MORPH
{
public:
	// 構築関数
	EGLClothModelMorphObject( void ) ;
	// 消滅関数
	~EGLClothModelMorphObject( void ) ;

public:
	// 頂点連結データ
	struct	VertexRelation
	{
		unsigned int	iRelation ;			// 頂点指標
		REAL32			rLength ;			// 距離
	} ;
	// １つの頂点制御用基底
	struct	Vertex
	{
		// 物理演算用パラメータ（直前の状態）
		E3DVector4		vMomentum ;			// 運動量
		E3DVector4		vPosition ;			// 位置ベクトル
		E3DVector4		vNormal ;			// 法線
		// 基準点パラメータ
		E3DVector4		vDistance ;			// 基準点からの距離
		Vertex *		pAboveVertex ;		// 基準点
		PCE3D_PRIMITIVE_POLYGON
						pMesh ;				// メッシュプリミティブ
		unsigned int	iVertex ;			// この頂点指標
		// 適用度
		REAL32			rMorphWeight ;		// 適用度（0:物理演算効果なし）
		// この頂点を含むポリゴン配列
		unsigned int *	pPolygons ;			// 法線を生成するためのポリゴン
		unsigned int	nPolygons ;			// １ポリゴンにつき２つの頂点指標
											// iVertex が指す頂点を p,
											// pPolygons[i], pPolygons[i + 1] を
											// p1, p2 としたときの法線は
											// (p1 - p) * (p2 - p) の外積
		// この頂点から連結される頂点
		VertexRelation *pRelations ;		// 頂点連結
		unsigned int	nRelations ;
	} ;
	// メッシュ構築一時データ用
	struct	NewVertexInfo
	{
		Vertex *		pAboveVertex ;
		unsigned int	iVertex ;
		unsigned int	nRefCount ;
	} ;
	// １つのメッシュ
	class	Mesh
	{
	public:
		const EGL_CLOTH_ATTRIBUTE *	m_pClothAttr ;
		PCE3D_PRIMITIVE_POLYGON		m_pMesh ;
		Vertex *					m_pVertices ;
		unsigned int				m_nVertices ;
		EStreamBuffer				m_bufPolygonList ;
	public:
		// 構築関数
		Mesh( void ) ;
		// 消滅関数
		~Mesh( void ) ;
		// メッシュ設定
		ESLError AttachMesh
			( const EGL_CLOTH_ATTRIBUTE * pAttribute,
						PCE3D_PRIMITIVE_POLYGON pMesh ) ;
		// メッシュ状態初期化
		ESLError InitializeMesh( void ) ;
		// メッシュ構築
		ESLError CreateMesh
			( const REAL32 * pVertexApply,
				unsigned int iApplyFirst, unsigned int nApplyCount ) ;
		// 演算実行
		ESLError MorphCloth
			( const EGL_CLOTH_MORPH_PARAMETER * pcmp,
						const EGLClothModelMorphObject & cmm ) ;
		// 座標変換
		ESLError MorphMesh
			( const EGL_CLOTH_MORPH_PARAMETER * pcmp,
						const EGLClothModelMorphObject & cmm ) ;
		// 法線計算
		ESLError MakeNormal
			( const EGL_CLOTH_MORPH_PARAMETER * pcmp,
						const EGLClothModelMorphObject & cmm ) ;
	public:
		// 頂点追加
		Vertex * AddVertex
			( Vertex * pAboveVertex,
				unsigned int iVertex, REAL32 rWeight ) ;
		// 既にエントリされている頂点の中で指定距離以下の最近頂点を検索する
		Vertex * FindRedundantVertex
			( const E3D_VECTOR & vPos, REAL32 & rRedundantRadius ) ;
		// 既にエントリされている頂点の中で
		// iVertex 指定頂点を含むポリゴンに属する最近頂点を検索する
		Vertex * FindNearestVertex
			( Vertex * pReference,
				unsigned int iVertex, unsigned int nScanLimit ) ;
		// すでにエントリされている指定の頂点を検索する
		int FindVertex( unsigned int iVertex ) ;
	private:
		// 追加する頂点情報をリストに追加する
		void AddListNewVertexInfo
			( EObjArray<NewVertexInfo> & list,
				Vertex * pAboveVertex, unsigned int iVertex ) ;
		// 指定頂点を含むポリゴンリストを作成
		void CreatePolygonList( Vertex * pNewVertex ) ;
	} ;

protected:
	// 当たり判定オブジェクト
	const HINDER_MODEL_INFO *	m_phmiHinder ;
	unsigned int				m_nHinderCount ;

	// メッシュ
	EObjArray<Mesh>	m_lstMesh ;

	// 風速揺らぎ
	E3DVector4		m_vStream ;
	double			m_rStreamAmplitude[3] ;
	double			m_rStreamPhase[3] ;
	double			m_rStreamFrequency[3] ;

	// 乱数
	DWORD			m_dwRandomSeed ;

	friend	Mesh ;

public:
	// 乱数発生
	int Random( int nLimit ) ;
	// ストリームパラメータ更新
	void UpdateStream( const EGL_CLOTH_MORPH_PARAMETER * pcmp ) ;
	// 障害物当たり判定
	virtual bool TestCollisionSphere
		( const E3D_VECTOR & vPos,
			E3D_VECTOR & vHitPos, E3D_VECTOR & vHitNormal ) const ;
	virtual bool TestCollisionSegment
		( const E3D_VECTOR & vPos0, const E3D_VECTOR & vPos1,
			E3D_VECTOR & vHitPos, E3D_VECTOR & vHitNormal ) const ;

public:		// 関数オーバーロード
	// モデル情報削除
	virtual ESLError DeleteCloth( void ) ;
	// モデル状態初期化
	virtual ESLError InitializeCloth( void ) ;
	// 当たり判定オブジェクト設定
	virtual ESLError SetHinderModel
		( const HINDER_MODEL_INFO * phmiHinder, unsigned int nCount ) ;
	// 布メッシュ追加
	virtual ESLError WeaveCloth
		( const EGL_CLOTH_ATTRIBUTE * pAttribute,
			PCE3D_PRIMITIVE_POLYGON pMeshPrimitive,
			const REAL32 * pVertexApply,
			unsigned int iApplyFirst, unsigned int nApplyCount ) ;
	// 布メッシュ追加
	virtual ESLError PatchCloth
		( const EGL_CLOTH_ATTRIBUTE * pAttribute,
			PCE3D_PRIMITIVE_POLYGON pMeshPrimitive, REAL32 rPathRadius,
			const REAL32 * pVertexApply,
			unsigned int iApplyFirst, unsigned int nApplyCount ) ;
	// 演算実行
	virtual ESLError MorphCloth( const EGL_CLOTH_MORPH_PARAMETER * pcmp ) ;
	virtual ESLError MorphMesh( const EGL_CLOTH_MORPH_PARAMETER * pcmp ) ;

private:	// 関数呼び出しインターフェース
	static EGLClothModelMorphObject *
				PtrFromHandle( HEGL_CLOTH_MODEL_MORPH hCloth )
		{
			ESLAssert( hCloth != NULL ) ;
			return	(EGLClothModelMorphObject*) hCloth ;
		}
	static ESLError Call_Release( HEGL_CLOTH_MODEL_MORPH hCloth ) ;
	static ESLError Call_DeleteCloth( HEGL_CLOTH_MODEL_MORPH hCloth ) ;
	static ESLError Call_InitializeCloth( HEGL_CLOTH_MODEL_MORPH hCloth ) ;
	static ESLError Call_SetHinderModel
		( HEGL_CLOTH_MODEL_MORPH hCloth,
			const HINDER_MODEL_INFO * phmiHinder, unsigned int nCount ) ;
	static ESLError Call_WeaveCloth
		( HEGL_CLOTH_MODEL_MORPH hCloth,
			const EGL_CLOTH_ATTRIBUTE * pAttribute,
			PCE3D_PRIMITIVE_POLYGON pMesh,
			const REAL32 * pVertexApply,
			unsigned int iApplyFirst, unsigned int nApplyCount ) ;
	static ESLError Call_PatchCloth
		( HEGL_CLOTH_MODEL_MORPH hCloth,
			const EGL_CLOTH_ATTRIBUTE * pAttribute,
			PCE3D_PRIMITIVE_POLYGON pMesh, REAL32 rPathRadius,
			const REAL32 * pVertexApply,
			unsigned int iApplyFirst, unsigned int nApplyCount ) ;
	static ESLError Call_MorphCloth
		( HEGL_CLOTH_MODEL_MORPH hCloth,
			const EGL_CLOTH_MORPH_PARAMETER * pcmp ) ;
	static ESLError Call_MorphMesh
		( HEGL_CLOTH_MODEL_MORPH hCloth,
			const EGL_CLOTH_MORPH_PARAMETER * pcmp ) ;

} ;


