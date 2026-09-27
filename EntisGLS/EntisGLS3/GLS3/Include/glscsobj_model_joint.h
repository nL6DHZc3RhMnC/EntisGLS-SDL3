
//////////////////////////////////////////////////////////////////////////////
// モデルジョイント・スクリプト・インターフェース
//////////////////////////////////////////////////////////////////////////////

class	ECSModelJoint	: public ECSObject, public E3DModelJoint
{
public:
	// 構築関数
	ECSModelJoint( void ) ;
	// 消滅関数
	virtual ~ECSModelJoint( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO2( ECSModelJoint, ECSObject, E3DModelJoint )

public:
	E3DModelJoint *	m_pRefJoint ;			// 参照行列
	ECSStructure *	m_strcPosition ;		// Vector 構造体
	ECSReal *		m_pcsrPosition[3] ;

	E3D_COLOR		m_clrModelColor ;		// 適用色
	unsigned int	m_nTransparency ;		// 透明度

	enum	ActionType
	{
		actNormal,	actLoop, actTurnLoop,
	} ;

protected:
	ECSObjArray<ECSReference>	m_lstRefModel ;

	int				m_nActionType ;			// ActionType
	SDWORD			m_dwDurationTime ;		// 全体時間
	SDWORD			m_dwAnimationTime ;		// 経過時間
	bool			m_fEnableFading ;
	bool			m_fEnableMoving ;
	bool			m_fEnableRotation ;

	E3D_COLOR		m_clrStartColor ;		// 色合い変化
	E3D_COLOR		m_clrEndColor ;
	unsigned int	m_nStartTransparency ;
	unsigned int	m_nEndTransparency ;

	EBezierCurves<E3D_VECTOR>
					m_bzPosition ;			// 移動ベジェ曲線
	EBezierCurves<E3D_VECTOR>
					m_bzRevolution ;		// 回転ベジェ曲線
	EBezierCurves<E3D_VECTOR>
					m_bzMagnification ;		// 拡大ベジェ曲線

	EBezierCurves<E3D_QUATERNION>
					m_bzRotation ;			// 回転ベジェ曲線

	// セーブ用構造体
	struct	MODEL_JOINT_SAVE_DATA
	{
		E3D_VECTOR		vPos ;
		E3D_REV_MATRIX	matrix ;
		E3D_COLOR		clrModelColor ;
		unsigned int	nTransparency ;
		int				nActionType ;
		DWORD			dwOffsetTime ;
		SDWORD			dwDurationTime ;
		int				fEnableFading ;
		int				fEnableMoving ;
		E3D_COLOR		clrStartColor ;
		E3D_COLOR		clrEndColor ;
		unsigned int	nStartTransparency ;
		unsigned int	nEndTransparency ;
//		E3D_VECTOR		bzPosition[4] ;
//		E3D_VECTOR		bzRevolution[4] ;
//		E3D_VECTOR		bzMagnification[4] ;
	} ;

public:
	// ジョイント生成
	virtual E3DModelJoint * CreateJoint( void ) const ;
	// パラメータ反映
	virtual void RefreshJoint( void ) ;
	void RefreshJointPosition( void ) ;
	// ジョイント複製
	void CopyModelJoint( const E3DModelJoint & model ) ;

public:
	// ジョイントアニメーション
	virtual bool OnAdvanceAnimation( unsigned int nTime ) ;

public:
	// モデルをレンダリングバッファに追加する
	virtual ESLError AddModelToRender( E3DRenderPolygon & render ) ;
	// 適用色取得
	const E3D_COLOR & GetColorAttribute( void ) const
		{	return	m_clrModelColor ;	}
	// 透明度取得
	unsigned int GetTransparency( void ) const
		{	return	m_nTransparency ;	}
	// 適用色設定
	void SetColorAttribute( const E3D_COLOR * pColor = NULL ) ;
	// 透明度設定
	void SetTransparency( unsigned int nTransparency ) ;
	// 適用色フェード設定
	void SetColorMorphing
		( const E3D_COLOR * pColor = NULL, unsigned int nTransparency = 0 ) ;
	// 移動アニメーション設定
	void SetBezierCurve
		( const EBezierCurves<E3D_VECTOR> * pbzCurve,
			const EBezierCurves<E3D_VECTOR> * pbzRevolution,
			const EBezierCurves<E3D_VECTOR> * pbzMagnification ) ;
	// 回転アニメーション設定
	void SetRotationBezier
		( const EBezierCurves<E3D_QUATERNION> & bzRotaion ) ;
	// アニメーション開始
	void BeginActivation
		( unsigned int nDuration, int nActionType = actNormal ) ;
	// アニメーション終了
	void FlushActivation( void ) ;
	// アニメーション中か？
	bool IsActivation( void ) const ;

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
	// メンバ変数インデックス取得
	virtual ESLError GetVariableIndex( int & nIndex, int iElement ) ;
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
	typedef	ESLError (ECSModelJoint::*PFUNC_CALL)
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	// メンバ関数ポインタ
	static ECSStrTagArray *	m_staFuncName ;
	static const wchar_t *	m_pwszFuncName[31] ;
	static const PFUNC_CALL	m_pfnCallFunc[30] ;
	// メンバ関数
	ESLError Call_Position
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_InitializeMatrix
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_RevolveOnX
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_RevolveOnY
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_RevolveOnZ
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_RevolveByAngleOn
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_MagnifyByVector
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetQuaternion
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetQuaternion
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_AddModelRef
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_ClearModelRef
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_AddSubJoint
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_CreateSubJoint
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetLength
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_RemoveSubJoint
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_RemoveAllSubJoint
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetColorAttribute
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetTransparency
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetColorAttribute
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetTransparency
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetColorMorphing
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetBezierCurve
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetRotationBezier
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_BeginActivation
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_FlushActivation
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_IsActivation
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;

} ;
