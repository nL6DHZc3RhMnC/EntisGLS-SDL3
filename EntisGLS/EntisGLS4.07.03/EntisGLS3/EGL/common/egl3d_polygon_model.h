
//////////////////////////////////////////////////////////////////////////////
// テクスチャ画像ライブラリ
//////////////////////////////////////////////////////////////////////////////

class	E3DTextureLibrary	: public	EWStrTagArray<EGLImage>
{
protected:
	E3DTextureLibrary *	m_pParent ;

public:
	// 構築関数
	E3DTextureLibrary( void ) : m_pParent( NULL ) { }
	// 消滅関数
	virtual ~E3DTextureLibrary( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( E3DTextureLibrary, EPtrArray )

public:
	// 関連親ライブラリを取得
	E3DTextureLibrary * GetParent( void ) const
		{
			return	m_pParent ;
		}
	// 関連親ライブラリを設定
	void SetParent( E3DTextureLibrary * pParent )
		{
			m_pParent = pParent ;
		}
	// テクスチャ画像を取得
	#if	defined(__PLATFORM_ANDROID__)
	PEGL_IMAGE_INFO GetTextureAs
		( const WORD * pwszName, bool fSearchParent = true ) const ;
	#else
	PEGL_IMAGE_INFO GetTextureAs
		( const wchar_t * pwszName, bool fSearchParent = true ) const ;
	#endif
	// 画像ポインタからテクスチャ名を取得
	ESLError GetTextureName
		( EWideString & wstrName,
			PEGL_IMAGE_INFO pTexture, bool fSearchParent = true ) const ;

} ;


//////////////////////////////////////////////////////////////////////////////
// 表面属性ライブラリ
//////////////////////////////////////////////////////////////////////////////

class	E3DSurfaceLibrary	: public	EWStrTagArray<E3D_SURFACE_ATTRIBUTE>
{
protected:
	E3DSurfaceLibrary *	m_pParent ;

public:
	// 構築関数
	E3DSurfaceLibrary( void ) : m_pParent( NULL ) { }
	// 消滅関数
	virtual ~E3DSurfaceLibrary( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( E3DSurfaceLibrary, EPtrArray )

public:
	// 関連親ライブラリを取得
	E3DSurfaceLibrary * GetParent( void ) const
		{
			return	m_pParent ;
		}
	// 関連親ライブラリを設定
	void SetParent( E3DSurfaceLibrary * pParent )
		{
			m_pParent = pParent ;
		}
	// 属性を取得する
	#if	defined(__PLATFORM_ANDROID__)
	E3D_SURFACE_ATTRIBUTE * GetAttributeAs
		( const WORD * pszName, bool fSearchParent = true ) const ;
	#else
	E3D_SURFACE_ATTRIBUTE * GetAttributeAs
		( const wchar_t * pszName, bool fSearchParent = true ) const ;
	#endif
	// 属性ポインタから属性名を取得
	ESLError GetAttributeName
		( EWideString & wstrName,
			E3D_SURFACE_ATTRIBUTE * pAttr, bool fSearchParent = true ) const ;

} ;


//////////////////////////////////////////////////////////////////////////////
// モデルオブジェクト
//////////////////////////////////////////////////////////////////////////////

class	E3DPolygonModel	: public	ESLObject
{
public:
	// 構築関数
	E3DPolygonModel( void ) ;
	// 消滅関数
	virtual ~E3DPolygonModel( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( E3DPolygonModel, ESLObject )

protected:
	E3DTextureLibrary			m_txlib ;
	E3DSurfaceLibrary			m_sflib ;
	#if	!defined(__PLATFORM_ANDROID__)
	ESLCriticalSection			m_csSync ;
	#endif

	unsigned int				m_nVertexCount ;
	PE3D_VECTOR4				m_pVertexes ;		// 頂点バッファ
	PE3D_VECTOR4				m_pVertexesBuf ;	// 一時頂点バッファ
	unsigned int				m_nNormalCount ;
	PE3D_VECTOR4				m_pNormals ;		// 法線バッファ
	PE3D_VECTOR4				m_pNormalsBuf ;		// 法線一時バッファ
	PE3D_VECTOR4				m_pConstantBuf ;
	EPtrObjArray<E3D_PRIMITIVE_POLYGON>
								m_lstPrimitives ;

public:
	// モデルデータを読み込む
	virtual ESLError ReadModel( ESLFileObject & file ) ;
protected:
	// テクスチャデータを読み込む
	virtual ESLError ReadTextureRecord( EMCFile & file ) ;
	// 表面属性データを読み込む
	virtual ESLError ReadSurfaceRecord( EMCFile & file ) ;
	// モデルデータを読み込む
	virtual ESLError ReadModelRecord( EMCFile & file ) ;
	// ユーザー定義のレコードを読み込む
	virtual ESLError ReadUserRecord( EMCFile & file, UINT64 idRec ) ;

#if	defined(__PLATFORM_ANDROID__)
protected:
	// ポリゴンの表裏正規化
	void NormalizePrimitiveFace( E3D_PRIMITIVE_POLYGON * pPrimitive ) ;
	void NormalizeMeshFace( E3D_PRIMITIVE_POLYGON * pMesh ) ;
	void NormalizePolygonFace( E3D_PRIMITIVE_POLYGON * pPoly ) ;
#else

public:
	// モデルデータを書き出す
	ESLError WriteModel( ESLFileObject & file ) ;
protected:
	// テクスチャデータを書き出す
	virtual ESLError WriteTextureRecord( EMCFile & file ) ;
	// 表面属性データを書き出す
	virtual ESLError WriteSurfaceRecord( EMCFile & file ) ;
	// モデルデータを書き出す
	virtual ESLError WriteModelRecord( EMCFile & file ) ;
	// ユーザー定義のレコードを書き出す
	virtual ESLError WriteUserRecord( EMCFile & file ) ;
#endif

public:
	// モデルデータを削除する
	virtual void DeleteContents( void ) ;
	// テクスチャ画像ライブラリを参照する
	E3DTextureLibrary & TextureLibrary( void )
		{
			return	m_txlib ;
		}
	// 表面属性ライブラリを参照する
	E3DSurfaceLibrary & SurfaceLibrary( void )
		{
			return	m_sflib ;
		}

#if	!defined(__PLATFORM_ANDROID__)
public:
	// 同期処理
	void Lock( void ) const
		{
			m_csSync.Lock() ;
		}
	void Unlock( void ) const
		{
			m_csSync.Unlock() ;
		}
#endif

public:
	// 頂点の数を取得する
	unsigned int GetVertexCount( void ) const
		{
			return	m_nVertexCount ;
		}
	// 頂点バッファのアドレスを取得する
	PE3D_VECTOR4 GetVertexList( unsigned int nIndex = 0 )
		{
			ESLAssert( m_pVertexes != NULL ) ;
			ESLAssert( nIndex < m_nVertexCount ) ;
			return	m_pVertexes + nIndex ;
		}
	PE3D_VECTOR4 GetVertexBuffer( void )
		{
			return	m_pVertexesBuf ;
		}
	// 頂点アドレスからインデックスに変換する
	int VertexBufferPointerToIndex( PE3D_VECTOR4 pVertex ) const
		{
			ESLAssert( pVertex != NULL ) ;
			LONG_PTR	i = ((LONG_PTR) pVertex - (LONG_PTR) m_pVertexesBuf)
													/ sizeof(E3D_VECTOR4) ;
			ESLAssert( (i >= 0) && (i < (LONG_PTR) m_nVertexCount) ) ;
			return	(int) i ;
		}
	// 法線の数を取得する
	unsigned int GetNormalCount( void ) const
		{
			return	m_nNormalCount ;
		}
	// 法線バッファのアドレスを取得する
	PE3D_VECTOR4 GetNormalList( unsigned int nIndex = 0 )
		{
			ESLAssert( m_pNormals != NULL ) ;
			ESLAssert( nIndex < m_nNormalCount ) ;
			return	m_pNormals + nIndex ;
		}
	PE3D_VECTOR4 GetNormalBuffer( void )
		{
			return	m_pNormalsBuf ;
		}
	// 法線アドレスからインデックスに変換する
	int NormalBufferPointerToIndex( PE3D_VECTOR4 pNormal ) const
		{
			ESLAssert( pNormal != NULL ) ;
			LONG_PTR	i = ((LONG_PTR) pNormal - (LONG_PTR) m_pNormalsBuf)
													/ sizeof(E3D_VECTOR4) ;
			ESLAssert( (i >= 0) && (i < (LONG_PTR) m_nNormalCount) ) ;
			return	(int) i ;
		}
	// 定数バッファのアドレスを取得する
	PE3D_VECTOR4 GetConstantBuffer( unsigned int nIndex = 0 )
		{
			ESLAssert( m_pConstantBuf != NULL ) ;
			return	m_pConstantBuf + nIndex ;
		}
	// 頂点バッファを確保する
	virtual void AllocateVertexBuffer( unsigned int nCount ) ;
	// 法線バッファを確保する
	virtual void AllocateNormalBuffer( unsigned int nCount ) ;
	// 定数バッファを確保する
	virtual void AllocateConstantBuffer( unsigned int nCount ) ;

public:
	// プリミティブ数を取得
	unsigned int GetPrimitiveCount( void ) const
		{
			return	m_lstPrimitives.GetSize( ) ;
		}
	// プリミティブを取得
	E3D_PRIMITIVE_POLYGON * GetPrimitiveAt( unsigned int nIndex )
		{
			return	m_lstPrimitives.GetAt( nIndex ) ;
		}
	// プリミティブ配列を取得
	PE3D_PRIMITIVE_POLYGON * const GetPrimitiveList( void ) const
		{
			return	m_lstPrimitives.GetData( ) ;
		}
	// プリミティブを検索
	int FindPrimitivePtr( E3D_PRIMITIVE_POLYGON * pPrimitive ) const
		{
			return	m_lstPrimitives.FindPtr( pPrimitive ) ;
		}
	// プリミティブデータ用バッファの再アロケート
	E3D_PRIMITIVE_POLYGON *
		ReallocatePrimitiveAt
			( unsigned int nIndex, unsigned int nBufSize ) ;
	// ポリゴンプリミティブを追加
	void AddPolygon
		( DWORD dwTypeFlag, PE3D_SURFACE_ATTRIBUTE pSurfAttr,
			DWORD dwVertexCount, const E3D_PRIMITIVE_VERTEX * pVertexes ) ;
	// プリミティブを追加
	void AddPrimitives
		( DWORD dwPrimitiveCount,
			const PE3D_PRIMITIVE_POLYGON * ppPrimitives ) ;
	// プリミティブを削除
	void RemovePrimitives( unsigned int nFirst, unsigned int nCount ) ;
	// プリミティブを全て削除
	void RemoveAllPrimitive( void ) ;

public:
	// 全プリミティブの頂点インデックスをアドレスに変換する
	ESLError ConvertAllPrimitiveIndexToAddress( void ) ;
	// プリミティブの頂点インデックスをアドレスに変換する
	ESLError ConvertPrimitiveIndexToAddress
			( E3D_PRIMITIVE_POLYGON * pPrimitive ) ;
	// 全プリミティブの頂点アドレスをインデックスに変換する
	ESLError ConvertAllPrimitiveAddressToIndex( void ) ;
	// プリミティブの頂点アドレスをインデックスに変換する
	ESLError ConvertPrimitiveAddressToIndex
			( E3D_PRIMITIVE_POLYGON * pPrimitive ) ;
	// プリミティブの頂点インデックスに加算する
	void AddPrimitiveVertexIndex
			( E3D_PRIMITIVE_POLYGON * pPrimitive,
				int nAddVertex,
				int nVertexMin = 0, int nVertexMax = -1 ) ;
	// プリミティブの法線インデックスに加算する
	void AddPrimitiveNormalIndex
			( E3D_PRIMITIVE_POLYGON * pPrimitive,
				int nAddNormal,
				int nNormalMin = 0, int nNormalMax = -1 ) ;
	// プリミティブのUV座標をスケーリングする
	void ScalePrimitiveUVMap
			( E3D_PRIMITIVE_POLYGON * pPrimitive,
				double xScale, double yScale ) ;
	// プリミティブデータのバイト数を計算する
	static DWORD CalcPrimitiveDataSize
		( const E3D_PRIMITIVE_POLYGON * pPrimitive ) ;

public:
	// プリミティブの頂点・法線座標をコミットする
	void CommitPrimitiveVertices
		( E3D_PRIMITIVE_POLYGON * pPrimitive ) ;
	// 頂点リストの座標をコミットする
	virtual void CommitVertexBuffer( int iVertex, int nCount ) ;
	// 法線リストの座標をコミットする
	virtual void CommitNormalBuffer( int iNormal, int nCount ) ;

#if	!defined(__PLATFORM_ANDROID__)
public:
	// モデルデータのサイズを取得する
	ESLError GetUntransformedModelRange
		( E3D_VECTOR4 & vMax, E3D_VECTOR4 & vMin ) ;
	ESLError GetTransformedModelRange
		( E3D_VECTOR4 & vMax, E3D_VECTOR4 & vMin ) ;
#endif

public:
	// 画像プリミティブを作成する
	void CreateImagePrimitive
		( PEGL_IMAGE_INFO pImage,
			PCEGL_RECT pView = NULL,
			const E3D_VECTOR_2D * pCenter = NULL,
			const E3D_VECTOR_2D * pEnlarge = NULL ) ;
	// 画像モデルを作成する
	void CreateImagePolygon
		( PEGL_IMAGE_INFO pImage,
			PCEGL_RECT pView = NULL,
			const E3D_VECTOR_2D * pCenter = NULL,
			PCE3D_SURFACE_ATTRIBUTE pSurfAttr = NULL,
			REAL32 rFogDeepness = 0.0F,
			const EGL_PALETTE * pFogColor = NULL ) ;
	// 画像モデルの画像を切り替える
	ESLError AttachImagePolygon
		( PEGL_IMAGE_INFO pImage,
			PCEGL_RECT pView = NULL, const E3D_VECTOR_2D * pCenter = NULL ) ;

public:
	// 表面属性生成して登録する
	E3D_SURFACE_ATTRIBUTE * RegisterSurfaceAttribute
		( E3D_COLOR rgbaColor,
			DWORD dwShadingFlags =
				E3DSAF_GOURAUD_SHADE | E3DSAF_SINGLE_SIDE_PLANE,
			SDWORD nAmbient = 0, SDWORD nDiffusion = 0x100,
			SDWORD nSpecular = 0, SDWORD nSpecularSize = 0x40,
			SDWORD nTransparency = 0, SDWORD nDeepness = 0,
			DWORD nReflection = 0, REAL32 nRefraction = 0.0f ) ;
	// テクスチャ表面属性生成して登録する
	E3D_SURFACE_ATTRIBUTE * RegisterTextureAttribute
		( PEGL_IMAGE_INFO pTextureInf,
			DWORD dwShadingFlags =
				E3DSAF_GOURAUD_SHADE | E3DSAF_SINGLE_SIDE_PLANE
					| E3DSAF_TEXTURE_MAPPING | E3DSAF_TEXTURE_SMOOTH,
			SDWORD nAmbient = 0, SDWORD nDiffusion = 0x100,
			SDWORD nSpecular = 0, SDWORD nSpecularSize = 0x40,
			SDWORD nTransparency = 0, SDWORD nDeepness = 0,
			DWORD nReflection = 0, REAL32 nRefraction = 0.0f ) ;

public:
	// メッシュタイプ
	enum	MeshType
	{
		gridHorzLoop	= 0x0001,
		gridVertLoop	= 0x0002,
		gridTopTip		= 0x0010,
		gridBottomTip	= 0x0020,
		meshAutoSmooth	= 0x8000,
	} ;
	// 格子状メッシュ頂点情報
	struct	GRID_MESH_INFO
	{
		int	nVertexCount ;		// 頂点数
		int	nPolygonCount ;		// ポリゴン数
		int	nHorzPolygonCount ;	// 行三角ポリゴン数
		int	nMeshPolyWidth ;	// 行の幅（三角ポリゴン数単位）
		int	nMeshPolyHeight ;	// 桁の幅（三角ポリゴン数単位）
	} ;
	// 格子状メッシュの頂点数を計算する
	static void CalcGridMeshVertexCount
		( GRID_MESH_INFO& gmi,
			int nMeshWidth, int nMeshHeight, int nFlags ) ;
	// 格子状メッシュを追加する
	E3D_PRIMITIVE_POLYGON *
		AddGridMeshPrimitive
			( E3D_SURFACE_ATTRIBUTE * pSurfAttr,
				int nMeshWidth, int nMeshHeight, int nFlags,
				PCE3D_VECTOR4 pvVertex, PCE3D_VECTOR4 pvNormal,
				PCE3D_VECTOR_2D pvUVMap, PCE3D_COLOR pvColor ) ;
	// 格子状メッシュを変更する
	ESLError ModifyGridMeshPrimitive
			( E3D_PRIMITIVE_POLYGON * pGridMesh,
				int nMeshWidth, int nMeshHeight, int nFlags,
				PCE3D_VECTOR4 pvVertex, PCE3D_VECTOR4 pvNormal,
				PCE3D_VECTOR_2D pvUVMap, PCE3D_COLOR pvColor ) ;
	// トライアングルストリップを追加する
	E3D_PRIMITIVE_POLYGON *
		AddTriangleStripPrimitive
			( E3D_SURFACE_ATTRIBUTE * pSurfAttr,
				int nTriangleStripCount, int nFlags,
				PCE3D_VECTOR4 pvVertex, PCE3D_VECTOR4 pvNormal,
				PCE3D_VECTOR_2D pvUVMap, PCE3D_COLOR pvColor ) ;
	// トライアングルストリップを変更する
	ESLError ModifyTriangleStripPrimitive
			( E3D_PRIMITIVE_POLYGON * pMesh,
				int nTriangleStripCount, int nFlags,
				PCE3D_VECTOR4 pvVertex, PCE3D_VECTOR4 pvNormal,
				PCE3D_VECTOR_2D pvUVMap, PCE3D_COLOR pvColor ) ;
	// トライアングルリストを追加する
	E3D_PRIMITIVE_POLYGON *
		AddTriangleListPrimitive
			( E3D_SURFACE_ATTRIBUTE * pSurfAttr,
				int nTriangleCount, int nFlags,
				PCE3D_VECTOR4 pvVertex, PCE3D_VECTOR4 pvNormal,
				PCE3D_VECTOR_2D pvUVMap, PCE3D_COLOR pvColor ) ;
	// トライアングルリストを変更する
	ESLError ModifyTriangleListPrimitive
			( E3D_PRIMITIVE_POLYGON * pMesh,
				int nTriangleCount, int nFlags,
				PCE3D_VECTOR4 pvVertex, PCE3D_VECTOR4 pvNormal,
				PCE3D_VECTOR_2D pvUVMap, PCE3D_COLOR pvColor ) ;
	// 頂点情報を変更する
	ESLError ModifyMeshPrimitive
			( E3D_PRIMITIVE_POLYGON * pMesh,
				int nVertexCount,
				PCE3D_VECTOR4 pvVertex, PCE3D_VECTOR4 pvNormal,
				PCE3D_VECTOR_2D pvUVMap, PCE3D_COLOR pvColor ) ;
	// 三角ポリゴンメッシュの法線を自動生成する
	ESLError SmoothTriangleMesh( E3D_PRIMITIVE_POLYGON * pMesh ) ;

	friend	class E3DModelJoint ;
} ;


