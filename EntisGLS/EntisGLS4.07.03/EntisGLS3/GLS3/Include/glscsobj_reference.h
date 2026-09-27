
//////////////////////////////////////////////////////////////////////////////
// 参照オブジェクト
//////////////////////////////////////////////////////////////////////////////

class	ECSReference	: public	ECSObject
{
public:
	ECSObject *			m_pRef ;				// 参照オブジェクト
	ECSObject *			m_pOwnObj ;				// 所有オブジェクト
	bool				m_fNontemp ;			// 非一時参照オブジェクト

	int					m_iVarOffset ;
	int					m_nVarBounds ;
	int					m_iFuncOffset ;

	// m_pRef がヌルの場合、m_pRefParent と m_iParentRef が用いられる
	ECSObject *			m_pRefParent ;			// 参照先の親
	int					m_iParentRef ;			// 参照先の親に対する指標

	// 同一オブジェクトを参照している ECSReference 連鎖
	ECSReference *		m_pNextBackRef ;
	ECSReference *		m_pPrevBackRef ;

protected:		// 大域的参照解決のためのデータ
	CSObjectMode		m_omBaseMode ;			// ベースオブジェクト
	ECSNumArray<int>	m_dimIndex ;			// 指標配列

public:
	struct	PLUGIN_OBJECT_HEADER
	{
		ECSReference *	pBackLink ;
	} ;
	struct	PLUGIN_REFERENCE
		: public PLUGIN_OBJECT_HEADER, public ECS_REFERENCE_INTERFACE { } ;
	PLUGIN_REFERENCE	m_pir ;		// プラグイン用インターフェース

public:
	// 構築関数
	ECSReference( ECSObject * pRef = NULL )
		{
			m_vtType = csvtReference ;
			m_pRef = NULL ;
			m_pOwnObj = NULL ;
			//
			m_iVarOffset = 0 ;
			m_nVarBounds = -1 ;
			m_iFuncOffset = 0 ;
			m_fNontemp = false ;
			//
			m_pRefParent = NULL ;
			//
			m_pNextBackRef = NULL ;
			m_pPrevBackRef = NULL ;
			m_omBaseMode = csomImmediate ;
			//
			if ( pRef != NULL )
			{
				SetReference( pRef, NULL ) ;
			}
		}
	// 消滅関数
	virtual ~ECSReference( void )
		{
			if ( m_pBackRef != NULL )
			{
				ECotophaScript::LockReference( ) ;
				CleanupAllBackReference( ) ;
				ECotophaScript::UnlockReference( ) ;
			}
			if ( m_pRef != NULL )
			{
				SetReference( NULL, NULL ) ;
			}
			ESLAssert( m_pOwnObj == NULL ) ;
			ESLAssert( !m_pNextBackRef && !m_pPrevBackRef ) ;
		}
	// 参照バックリンクを解消
	void ReleaseBackLink( void )
		{
			if ( m_pBackRef != NULL )
			{
				ECotophaScript::LockReference( ) ;
				CleanupAllBackReference( ) ;
				ECotophaScript::UnlockReference( ) ;
			}
		}
	// クラス情報
	DECLARE_CLASS_INFO( ECSReference, ECSObject )

public:
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
	virtual ESLError GetVariableIndex( int & nIndex, int iMember ) ;
	virtual ESLError GetVariableIndex
					( int & nIndex, const wchar_t * pwszMember ) ;
	// メンバ変数取得
	virtual ECSObject * GetVariableAt( int nIndex ) ;
	// メンバ変数設定
	virtual ECSObject * SetVariableAt( int nIndex, ECSObject * obj ) ;
	// メンバ関数インデックス取得
	virtual ESLError GetFunction
		( ECSContext & context, int & nIndex, const wchar_t * pwszName ) ;
	// メンバ関数ポインタ取得
	virtual ESLError GetFunctionPointer
		( ECSContext & context,
			ECS_FUNCTION_POINTER & fptr, const wchar_t * pwszName ) ;
	virtual ESLError GetFunctionPointer
		( ECSContext & context,
			ECS_FUNCTION_POINTER & fptr, int nIndex ) ;
	// メンバ関数呼び出し
	virtual ESLError CallFunction
		( ECSContext & context,
			int nIndex, ECSObjArray<ECSObject> & lstArg ) ;
	// 特殊演算子 : boolean 判定
	virtual ESLError OperateBoolean( int & nBoolean ) ;
	// 特殊演算子 : sizeof
	virtual ESLError OperateSizeOf( INT64 & nSize ) ;
	// 特殊演算子 : typeof
	virtual const wchar_t * OperateTypeOf( void ) const ;
	// 特殊演算子 : interface 型変換
	virtual ESLError OperateCastInterface
			( ECS_CAST_INTERFACE & ci, const wchar_t * pwszTypeName ) ;
	// 整数値取得
	virtual ESLError OperateInteger( INT64 & nValue ) ;
	// 実数取得
	virtual ESLError OperateReal( REAL64 & nValue ) ;
	// 文字列取得
	virtual ESLError OperateString( EWideString & wstrValue ) ;
	// 内部バッファインターフェース
	virtual void * GetBuffer( int iOffset, int nSize, bool fWritable ) ;
	virtual void FlushBuffer
		( int iOffset, int nSize, void * ptrBuf, bool fModified ) ;
	virtual ECSSakura2Processor::LinearAddressCache *
			GetSegmentBuffer( ECSSakura2Processor::LinearAddressCache & seg ) ;

public:
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
	// 参照オブジェクトインデックス表記を取得する
	EString FormatReferenceObjectIndex( ECSContext & context ) ;
	// 大域的参照解決
	ESLError CommitGlobalReference( ECSContext & context ) ;
	// スクリプトのデストラクタ
	virtual void OnDestruction( ECSContext & context ) ;

public:
	// メンバ関数プロトタイプ
	typedef	ESLError (ECSReference::*PFUNC_CALL)
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	// メンバ関数ポインタ
	static ECSStrTagArray *	m_staFuncName ;
	static const wchar_t *	m_pwszFuncName[6] ;
	static const PFUNC_CALL	m_pfnCallFunc[5] ;
	// メンバ関数
	ESLError Call_GetType
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetReference
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_DetachReference
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_IsIdentity
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Duplicate
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;

public:
	// 参照を設定する
	void SetReference
			( ECSObject * pRef, ECSContext * context = NULL,
				int iVarOffset = 0, int nVarBounds = -1, int iFuncOffset = 0,
				ECSObject * pRefParent = NULL, int iParentRef = 0 )
		{
			if ( m_pOwnObj == NULL )
			{
				ECotophaScript::LockReference( ) ;
				DetachBackLinkChain( ) ;
			}
			else
			{
				ReleaseOwnObject( context ) ;
				ECotophaScript::LockReference( ) ;
			}
			ESLAssert( this != pRef ) ;
			m_pRef = pRef ;
			m_iVarOffset = iVarOffset ;
			m_nVarBounds = nVarBounds ;
			m_iFuncOffset = iFuncOffset ;
			m_pRefParent = pRefParent ;
			m_iParentRef = iParentRef ;
			//
			if ( pRef->IsValidObject() )
			{
				UpdateReferenceBackLink() ;
			}
			ECotophaScript::UnlockReference( ) ;
		}
	void SetReferenceCastInterface
			( ECSObject * pRef,
				ECSContext * context, const ECS_CAST_INTERFACE & ci )
		{
			SetReference
				( pRef, context,
					ci.iVarOffset, ci.nVarBounds, ci.iFuncOffset ) ;
		}
	void SetReferenceCastInterface
			( ECSObject * pRef,
				ECSContext * context, const ECSReference & ref )
		{
			SetReference
				( pRef, context,
					ref.m_iVarOffset, ref.m_nVarBounds, ref.m_iFuncOffset ) ;
		}
	void SetOwnObject
			( ECSObject * pRef, ECSContext * context = NULL,
				int iVarOffset = 0, int nVarBounds = -1, int iFuncOffset = 0 )
		{
			if ( m_pOwnObj == NULL )
			{
				ECotophaScript::LockReference( ) ;
				DetachBackLinkChain( ) ;
			}
			else
			{
				ReleaseOwnObject( context ) ;
				ECotophaScript::LockReference( ) ;
			}
			ESLAssert( this != pRef ) ;
			m_pOwnObj = m_pRef = pRef ;
			m_iVarOffset = iVarOffset ;
			m_nVarBounds = nVarBounds ;
			m_iFuncOffset = iFuncOffset ;
			m_pRefParent = NULL ;
			m_iParentRef = 0 ;
			//
			if ( pRef->IsValidObject() )
			{
//				ESLAssert( pRef->m_vtType != csvtReference ) ;
				AddReferenceBackLinkChain( ) ;
			}
			ECotophaScript::UnlockReference( ) ;
		}
	void SetOwnObjectCastInterface
			( ECSObject * pRef,
				ECSContext * context, const ECS_CAST_INTERFACE & ci )
		{
			SetOwnObject
				( pRef, context,
					ci.iVarOffset, ci.nVarBounds, ci.iFuncOffset ) ;
		}
	// 参照や所有オブジェクトを分離する
	ECSObject * DetachObject( ECSContext & context ) ;
	// Reference オブジェクトへの参照を、参照先への参照へ正規化する
	void CleanupAllBackReference( void ) ;

protected:
	// 参照先からのバックリンクを更新する
	void UpdateReferenceBackLink( void ) ;
	// 参照連鎖から分離する
	void DetachBackLinkChain( void )
		{
			ESLAssert( m_pOwnObj == NULL ) ;
			if ( m_pRef != NULL )
			{
				if ( m_pPrevBackRef != NULL )
				{
					ESLAssert( m_pPrevBackRef->IsValidObject() ) ;
					ESLAssert( !m_pNextBackRef || m_pNextBackRef->IsValidObject() ) ;
					m_pPrevBackRef->m_pNextBackRef = m_pNextBackRef ;
				}
				else
				{
					ESLAssert( m_pRef->IsValidObject() ) ;
					ESLAssert( m_pRef->m_pBackRef == this ) ;
					ESLAssert( !m_pNextBackRef || m_pNextBackRef->IsValidObject() ) ;
					m_pRef->m_pBackRef = m_pNextBackRef ;
				}
				if ( m_pNextBackRef != NULL )
				{
					ESLAssert( m_pNextBackRef->IsValidObject() ) ;
					m_pNextBackRef->m_pPrevBackRef = m_pPrevBackRef ;
				}
				m_pNextBackRef = NULL ;
				m_pPrevBackRef = NULL ;
			}
			ESLAssert( !m_pNextBackRef && !m_pPrevBackRef ) ;
			m_pRef = NULL ;
		}
	// 参照連鎖に m_pRef の参照先を追加する
	void AddReferenceBackLinkChain( void )
		{
			ESLAssert( m_pRef != NULL ) ;
			m_pNextBackRef = m_pRef->m_pBackRef ;
			m_pPrevBackRef = NULL ;
			if ( m_pNextBackRef != NULL )
			{
				ESLAssert( m_pNextBackRef->IsValidObject() ) ;
				ESLAssert( m_pNextBackRef->m_pPrevBackRef == NULL ) ;
				m_pNextBackRef->m_pPrevBackRef = this ;
			}
			m_pRef->m_pBackRef = this ;
		}
	// 所有オブジェクト参照解放
	void ReleaseOwnObject( ECSContext * context )
		{
			ECotophaScript::LockReference( ) ;
			ESLAssert( m_pOwnObj != NULL ) ;
			ECSObject *	pOwnObj = m_pOwnObj ;
			m_pOwnObj = NULL ;
			ESLAssert( m_pRef != NULL ) ;
			DetachBackLinkChain( ) ;
			//
			ECSReference *	pNewOwner = pOwnObj->m_pBackRef ;
			if ( pNewOwner != NULL )
			{
				ESLAssert( pNewOwner != this ) ;
				ESLAssert( pNewOwner->m_pOwnObj == NULL ) ;
				ESLAssert( GetEntity(pNewOwner->m_pRefParent) == NULL ) ;
				ESLAssert( pNewOwner->m_pRef == pOwnObj ) ;
				pNewOwner->m_pOwnObj = pOwnObj ;
				pNewOwner->m_pRefParent = NULL ;
				ECotophaScript::UnlockReference( ) ;
			}
			else
			{
				ECotophaScript::UnlockReference( ) ;
				if ( context != NULL )
				{
					pOwnObj->OnDestruction( *context ) ;
				}
				delete	pOwnObj ;
			}
			ESLAssert( m_pRef == NULL ) ;
		}

public:
	// オブジェクトを所有する ECSReference を検索する
	static ECSReference * FindReferenceOwner( ECSObject * pObj ) ;
	// 参照先オブジェクトから破棄通知
	virtual void OnObjectDestory( ECSObject * pObj ) ;

public:
	// プラグインインターフェースを取得する
	virtual void * GetObjectInterface( const wchar_t * pwszType ) ;

protected:
	static ECS_OBJECT * __stdcall PIC_GetObjectEntity
		( ECS_REFERENCE_INTERFACE * instance ) ;
	static void __stdcall PIC_SetReference
		( ECS_REFERENCE_INTERFACE * instance,
			ECS_OBJECT * pRef, ECS_CONTEXT * context ) ;

} ;
