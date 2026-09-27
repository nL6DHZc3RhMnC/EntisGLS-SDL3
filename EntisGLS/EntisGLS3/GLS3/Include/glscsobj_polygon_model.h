
//////////////////////////////////////////////////////////////////////////////
// モデルデータ・スクリプト・インターフェース
//////////////////////////////////////////////////////////////////////////////

class	ECSPolygonModel	: public ECSObject, public E3DBonePolygonModel
{
public:
	// 構築関数
	ECSPolygonModel( void ) ;
	// 消滅関数
	virtual ~ECSPolygonModel( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO2( ECSPolygonModel, ECSObject, E3DBonePolygonModel )

public:
	ECSWStrTagArray<ECSModelJoint>	m_staJoint ;

protected:
	// セーブ用
	ECSWideString			m_wstrFileName ;	// モデルファイル名

	DWORD					m_dwPrimitiveType ;	// プリミティブタイプ
	ECSReference			m_refImage ;		// 参照画像
	int						m_nFrameNum ;
	EGL_RECT				m_rectView ;
	E3D_VECTOR_2D			m_vCenter ;
	E3D_VECTOR_2D			m_vEnlarge ;
	E3D_SURFACE_ATTRIBUTE	m_sfAttribute ;

public:
	// モデルデータを読み込む
	virtual ESLError ReadModel( ESLFileObject & file ) ;
protected:
	// ボーンジョイントを列挙し、登録する
	void RegisterBoneJoint( E3DBoneJoint * pBone ) ;

public:
	// モデルデータを削除する
	virtual void DeleteContents( void ) ;

public:
	// ファイルを開く
	virtual ESLFileObject * OpenResourceFile
		( const wchar_t * pwszFilePath, ECSContext * pContext ) ;

public:		// 通常のオブジェクト処理
	// オブジェクトの型名を取得する
	virtual const wchar_t * GetTypeName( void ) const ;
	// オブジェクトを複製
	virtual ECSObject * Duplicate( void ) ;
	// オブジェクトを代入
	virtual ESLError Move( ECSContext & context, ECSObject * obj ) ;
	// 単項演算子
	virtual ESLError UnaryOperate
		( ECSContext & context, CSUnaryOperatorType csuopType ) ;
	// 二項演算子
	virtual ESLError Operate
		( ECSContext & context, CSOperatorType csopType, ECSObject * obj ) ;
	// 比較演算子
	virtual ESLError Compare
		( ECSContext & context, int & nResult,
			CSCompareType cscpType, ECSObject & obj ) ;
	// メンバ変数取得
	virtual ECSObject * GetVariableAt( int nIndex ) ;
	// メンバ変数設定
	virtual ECSObject * SetVariableAt( int nIndex, ECSObject * obj ) ;
	// メンバ関数インデックス取得
	virtual ESLError GetFunction
		( ECSContext & context, int & nIndex, const wchar_t * pwszName ) ;
	// メンバ関数呼び出し
	virtual ESLError CallFunction
		( ECSContext & context,
			int nIndex, ECSObjArray<ECSObject> & lstArg ) ;

public:		// シリアル化のための関数（システムによって必要）
	// 全てのメンバ変数にインデックスを振る
	virtual void IndexAllMember( void ) ;
	// 全てのメンバ変数の参照を解決する
	virtual ESLError CommitAllReference( ECSContext & context ) ;
	// データを保存
	virtual ESLError Save( ESLFileObject & file, ECSContext & context ) ;
	// データを復元
	virtual ESLError Load( ESLFileObject & file, ECSContext & context ) ;
	// データをダンプ
	virtual ESLError DumpObject
		( EStreamBuffer & buf, int nIndent, ECSContext & context ) ;

public:
	// メンバ関数プロトタイプ
	typedef	ESLError (ECSPolygonModel::*PFUNC_CALL)
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	// メンバ関数ポインタ
	static ECSStrTagArray *	m_staFuncName ;
	static const wchar_t *	m_pwszFuncName[16] ;
	static const PFUNC_CALL	m_pfnCallFunc[15] ;
	// メンバ関数
	ESLError Call_LoadModel
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_DeleteModel
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_CreateImagePrimitive
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_CreateImagePolygon
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_AttachImagePolygon
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_TransformAccordingAsBone
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_FindBoneAs
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	//
	ESLError Call_RegisterSurfaceAttribute
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_RegisterTextureAttribute
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_AddGridMeshPrimitive
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_ModifyGridMeshPrimitive
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_AddTriangleStripPrimitive
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_ModifyTriangleStripPrimitive
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_AddTriangleListPrimitive
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_ModifyTriangleListPrimitive
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	//
	void GetScriptSurfaceAttribute
		( E3D_SURFACE_ATTRIBUTE & sufattr,
			ECSStructureInterface * pSufAttr ) ;
	//
	struct	MeshVertexBuffer
	{
		EStreamBuffer	bufVertex ;
		EStreamBuffer	bufNormal ;
		PE3D_VECTOR4	pvVertex ;
		PE3D_VECTOR4	pvNormal ;
		PE3D_VECTOR_2D	pvUVMap ;
		PE3D_COLOR		pvColor ;
	} ;
	void GetScriptMeshVertexBuffer
		( MeshVertexBuffer& mvbuf,
			ECSObject * pObjVertex, ECSObject * pObjNormal,
			ECSObject * pObjUVMap, ECSObject * pObjColor, int nVertexCount ) ;
	void ReleaseScriptMeshVertexBuffer
		( MeshVertexBuffer& mvbuf,
			ECSObject * pObjVertex, ECSObject * pObjNormal,
			ECSObject * pObjUVMap, ECSObject * pObjColor, int nVertexCount ) ;

} ;

