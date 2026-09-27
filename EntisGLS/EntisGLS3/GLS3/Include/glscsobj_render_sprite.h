
//////////////////////////////////////////////////////////////////////////////
// レンダリングスプライト・スクリプト・インターフェース
//////////////////////////////////////////////////////////////////////////////

class	ECSRenderSprite	: public ECSObject, public E3DRenderSprite
{
public:
	// 構築関数
	ECSRenderSprite( void ) ;
	// 消滅関数
	virtual ~ECSRenderSprite( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO2( ECSRenderSprite, ECSObject, E3DRenderSprite )

protected:
	// 保存用パラメータ
	struct	SAVE_PARAM
	{
		int			nInitPriority ;
		int			nInitHeapSize ;
		int			nInitPolyLimit ;
		int			nInitFlags ;
		EGL_RECT	rctInitClip ;
		REAL32		rClipMinZ, rClipMaxZ ;
		E3D_VECTOR	vViewPoint[2], vViewTarget[2] ;
		double		rViewAngleZ[2] ;
		DWORD		dwSortingFlags ;
		DWORD		dwCameraFlags, dwDurationTime, dwOffsetTime ;
	} ;
	ECSReference		m_refInitParent ;		// 初期化パラメータ
	SAVE_PARAM			m_spParam ;

	E3D_LIGHT_ENTRY *	m_pleLightEntries ;		// 光源
	unsigned int		m_nLightEntryCount ;

	ECSReference		m_refRootJoint ;		// ルートジョイント

	E3DViewAngleCurve	m_vacCameraCurve ;		// カメラ移動曲線
	DWORD				m_dwCameraFlags ;
	SDWORD				m_dwDurationTime ;		// アニメーション継続時間
	DWORD				m_dwAnimationTime ;		// アニメーション時間

	DWORD				m_dwLastUpdateTime ;

public:
	// カメラ設定
	void SetViewPoint
		( const E3D_VECTOR & vViewPoint,
			const E3D_VECTOR & vTarget, double rDegAngle ) ;
	// カメラ曲線設定
	void SetEndOfViewCurve
		( const E3D_VECTOR & vViewPoint,
			const E3D_VECTOR & vTarget,
			double rDegAngle, DWORD dwFlags = 0 ) ;
	// カメラ設定
	void SetViewOnCurve( double t ) ;
	// カメラアニメーション設定
	void BeginViewAnimation( DWORD dwDuration ) ;
	// カメラアニメーション終了
	void FlushViewAnimation( void ) ;
	// カメラアニメーション中か？
	bool IsViewAnimation( void ) const
		{
			return	(m_dwDurationTime != 0) ;
		}
	// アニメーション終了
	void FlushActivation( void ) ;
	// アニメーション中か？
	bool IsActivation( void ) const ;

public:
	// スプライト描画
	virtual void MTDraw( HEGL_RENDER_POLYGON hRenderPoly ) ;
	// リソース開放
	virtual ESLError Release( void ) ;
	// ルートジョイント関連付け
	virtual ECSModelJoint * AttachRootJoint( ECSModelJoint * pJoint ) ;
	// 表示更新
	virtual ESLError UpdateRendering( void ) ;
	// アニメーション処理
	virtual ESLError OnAdvanceAnimation( unsigned int nTime ) ;

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
	// 全てのメンバ変数の参照を解消する
	virtual void CleanupAllReference( ECSContext & context ) ;
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
	typedef	ESLError (ECSRenderSprite::*PFUNC_CALL)
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	// メンバ関数ポインタ
	static ECSStrTagArray *	m_staFuncName ;
	static const wchar_t *	m_pwszFuncName[20] ;
	static const PFUNC_CALL	m_pfnCallFunc[19] ;
	// メンバ関数
	ESLError Call_Initialize
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Release
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetViewPoint
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetEndOfViewCurve
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetViewOnCurve
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_BeginViewAnimation
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_FlushViewAnimation
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_IsViewAnimation
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_FlushActivation
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_IsActivation
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetZClipRange
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetLightEntries
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetSortingFlags
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetSortingFlags
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_AddModel
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_PrepareRendering
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_FlushAllPolygon
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_AttachRootJoint
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_UpdateRendering
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;

} ;
