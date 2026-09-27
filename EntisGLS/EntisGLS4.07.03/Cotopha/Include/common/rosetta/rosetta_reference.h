
#if	!defined(__ROSETTA_REFERENCE_H__)
#define	__ROSETTA_REFERENCE_H__

namespace	Rosetta
{
	//////////////////////////////////////////////////////////////////////////
	// オブジェクト参照
	//////////////////////////////////////////////////////////////////////////

	class	RSReference	: public RSObject
	{
	public:
		RSObject *	m_pRef ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSReference, RSObject )
		// 構築関数
		RSReference
			( RSObject * pRef,
				RSClass * pClass = NULL, BasicType type = typeReference )
			: RSObject(NULL,type), m_pRef(pRef) {}
		// 消滅関数
		virtual ~RSReference( void ) ;
		// 参照設定
		void SetReference( RSObject * pRef ) ;
		// 参照分離
		RSObject * DetachReference( void ) ;

	public:	// 型情報
		// 型名
		virtual const wchar_t * GetTypeName( void ) const ;
		// 型テスト
		virtual RSObject * InstanceOf( const wchar_t * pwszType ) ;
		virtual RSObject * InstanceOf( RSClass * pClass ) ;
		// 整数型か？
		virtual bool IsIntegerType( void ) const ;
		// 浮動小数点型か？
		virtual bool IsFloatType( void ) const ;
		// 文字列型か？
		virtual bool IsStringType( void ) const ;
		// 即値型か？
		virtual bool IsObjectType( void ) const ;
		// 整数値取得
		virtual bool AsInteger( int64_t& number ) const ;
		// 実数値取得
		virtual bool AsRealNumber( double& number ) const ;
		// ブール判定
		virtual bool AsBoolean( void ) const ;
		// 文字列変換
		virtual bool AsString( SSystem::SString& strValue ) const ;
		// 同定判定
		virtual bool IsEqualObject( RSObject * pObj ) const ;
		// デバッグ用ダンプ文字列
		virtual void ToDebugDump
			( SSystem::SFileInterface& dump,
					size_t nPtrNest = 10,
					const wchar_t * pwszIndent = NULL ) ;
		// 値設定
		virtual SSystem::SError SetIntegerAs( int64_t nValue ) ;
		virtual SSystem::SError SetNumberAs( double nValue ) ;
		virtual SSystem::SError SetStringAs( const wchar_t * pwszValue ) ;

	public:	// オブジェクト
		// 内部リソース解放
		virtual void DisposeObject( RSContext& context ) ;
		// 複製（参照の複製を含む）
		virtual RSObject * DuplicateObject( RSContext& context ) const ;
		// 複製（実体も可能な限り複製）
		virtual RSObject * CloneObject( RSContext& context ) const ;
		// 実体
		virtual RSObject * GetEntityObject( void ) const ;

	public:	// メンバ・プロパティ
		// 要素取得
		virtual RSObject * GetElementAt
			( RSContext& context, int nIndex ) const ;
		// 要素名取得
		virtual const wchar_t * GetElementNameAt( int nIndex ) const ;
		// 要素取得
		virtual RSObject * SetElementAt
			( RSContext& context, int nIndex, RSObject * pObj ) ;
		// 要素数取得
		virtual size_t GetElementCount( void ) const ;
		// 要素最大数取得
		virtual size_t GetElementLimit( void ) const ;
		// メンバ取得
		virtual RSObject * GetMemberAs
			( RSContext& context, const wchar_t * pwszName ) const ;
		// メンバ設定
		virtual RSObject * SetMemberAs
			( RSContext& context, const wchar_t * pwszName, RSObject * pObj ) ;
		// メンバ新規作成
		virtual RSObject * CreateMemberAs
			( RSContext& context, const wchar_t * pwszName, RSObject * pObj ) ;

	public:	// 関数
		// メソッド取得
		virtual METHOD_ENTRY * GetMethodAs
				( RSContext& context,
					const wchar_t * pwszName, METHOD_ENTRY& method ) const ;

	public:	// オペレーター
		// 単項演算子
		virtual RSObject * OperatorPlus( RSContext& context ) const ;
		virtual RSObject * OperatorNegate( RSContext& context ) const ;
		virtual RSObject * OperatorBitNot( RSContext& context ) const ;
		virtual RSObject * OperatorIncrement( RSContext& context ) ;
		virtual RSObject * OperatorDecrement( RSContext& context ) ;
		// 二項演算子
		virtual RSObject * OperatorMul( RSContext& context, RSObject * pObj ) const ;
		virtual RSObject * OperatorDiv( RSContext& context, RSObject * pObj ) const ;
		virtual RSObject * OperatorMod( RSContext& context, RSObject * pObj ) const ;
		virtual RSObject * OperatorAdd( RSContext& context, RSObject * pObj ) const ;
		virtual RSObject * OperatorSub( RSContext& context, RSObject * pObj ) const ;
		virtual RSObject * OperatorShiftLeft( RSContext& context, RSObject * pObj ) const ;
		virtual RSObject * OperatorShiftRight( RSContext& context, RSObject * pObj ) const ;
		virtual RSObject * OperatorBitShiftRight( RSContext& context, RSObject * pObj ) const ;
		virtual RSObject * OperatorBitAnd( RSContext& context, RSObject * pObj ) const ;
		virtual RSObject * OperatorBitOr( RSContext& context, RSObject * pObj ) const ;
		virtual RSObject * OperatorBitXor( RSContext& context, RSObject * pObj ) const ;
		virtual RSObject * OperatorCompareEQ( RSContext& context, RSObject * pObj ) const ;
		virtual RSObject * OperatorCompareNE( RSContext& context, RSObject * pObj ) const ;
		virtual RSObject * OperatorCompareGE( RSContext& context, RSObject * pObj ) const ;
		virtual RSObject * OperatorCompareGT( RSContext& context, RSObject * pObj ) const ;
		virtual RSObject * OperatorCompareLE( RSContext& context, RSObject * pObj ) const ;
		virtual RSObject * OperatorCompareLT( RSContext& context, RSObject * pObj ) const ;
		// 代入演算子
		virtual RSObject * OperatorMove( RSContext& context, RSObject * pObj ) ;

	public:
		// シリアライズ
		virtual RSObject * SerializeObject( RSContext& context ) ;
		virtual SSystem::SError SerializeBinary
				( RSContext& context, SSystem::SFileInterface& file ) ;
		virtual SSystem::SError MakeXMLDocument
				( RSContext& context, SSystem::SXMLDocument& xmlDoc ) ;
		// 復元
		virtual SSystem::SError RestoreObject
				( RSContext& context, RSObject * pObj ) ;
		virtual SSystem::SError RestoreBinary
				( RSContext& context, SSystem::SFileInterface& file ) ;
		virtual SSystem::SError RestoreXMLDocument
				( RSContext& context, const SSystem::SXMLDocument& xmlDoc ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// オブジェクト参照（ポインタ）
	//////////////////////////////////////////////////////////////////////////

	class	RSPointer	: public RSReference
	{
	public:
		RSClass *	m_pPtrClass ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSPointer, RSReference )
		// 構築関数
		RSPointer( RSObject * pRef, RSClass * pClass = NULL )
			: RSReference(pRef, pClass, typePointer), m_pPtrClass(pClass) {}

	public:
		// ポインタ型設定
		void SetPointerType( RSClass * pClass )
		{
			m_pPtrClass = pClass ;
		}

	public:
		// ブール判定
		virtual bool AsBoolean( void ) const ;
		// 文字列変換
		virtual bool AsString( SSystem::SString& strValue ) const ;
		// 複製（参照の複製を含む）
		virtual RSObject * DuplicateObject( RSContext& context ) const ;
		// 複製（実体も可能な限り複製）
		virtual RSObject * CloneObject( RSContext& context ) const ;
		// 実体型
		virtual RSClass * GetEntityClass( void ) const ;
		// 二項演算子
		virtual RSObject * OperatorCompareEQ( RSContext& context, RSObject * pObj ) const ;
		virtual RSObject * OperatorCompareNE( RSContext& context, RSObject * pObj ) const ;
		// 代入演算子
		virtual RSObject * OperatorMove( RSContext& context, RSObject * pObj ) ;

	public:
		// シリアライズ
		virtual SSystem::SError MakeXMLDocument
				( RSContext& context, SSystem::SXMLDocument& xmlDoc ) ;
		// 復元
		virtual SSystem::SError RestoreXMLDocument
				( RSContext& context, const SSystem::SXMLDocument& xmlDoc ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 配列要素参照
	//////////////////////////////////////////////////////////////////////////

	class	RSReferenceElement	: public RSObject
	{
	public:
		RSObject *	m_pRefParent ;
		int			m_nRefIndex ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSReferenceElement, RSObject )
		// 構築関数
		RSReferenceElement
			( RSObject * pRef, int nIndex, RSClass * pClass = NULL )
				: RSObject(pClass, typeOther),
					m_pRefParent(pRef), m_nRefIndex(nIndex) {}
		// 消滅関数
		virtual ~RSReferenceElement( void ) ;

	public:
		// ブール判定
		virtual bool AsBoolean( void ) const ;
		// 文字列変換
		virtual bool AsString( SSystem::SString& strValue ) const ;
		// 複製（参照の複製を含む）
		virtual RSObject * DuplicateObject( RSContext& context ) const ;
		// 複製（実体も可能な限り複製）
		virtual RSObject * CloneObject( RSContext& context ) const ;
		// 実体
		virtual RSObject * GetEntityObject( void ) const ;
		// 代入演算子
		virtual RSObject * OperatorMove( RSContext& context, RSObject * pObj ) ;
		// 二項演算子
		virtual RSObject * OperatorCompareEQ( RSContext& context, RSObject * pObj ) const ;
		virtual RSObject * OperatorCompareNE( RSContext& context, RSObject * pObj ) const ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// メンバ参照
	//////////////////////////////////////////////////////////////////////////

	class	RSReferenceMember	: public RSObject
	{
	public:
		RSObject *			m_pRefParent ;
		SSystem::SString	m_strRefMember ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSReferenceMember, RSObject )
		// 構築関数
		RSReferenceMember
			( RSObject * pRef,
					const wchar_t * pwszMember, RSClass * pClass = NULL )
				: RSObject(pClass, typeOther),
					m_pRefParent(pRef), m_strRefMember(pwszMember) {}
		// 消滅関数
		virtual ~RSReferenceMember( void ) ;

	public:
		// ブール判定
		virtual bool AsBoolean( void ) const ;
		// 文字列変換
		virtual bool AsString( SSystem::SString& strValue ) const ;
		// 複製（参照の複製を含む）
		virtual RSObject * DuplicateObject( RSContext& context ) const ;
		// 複製（実体も可能な限り複製）
		virtual RSObject * CloneObject( RSContext& context ) const ;
		// 実体
		virtual RSObject * GetEntityObject( void ) const ;
		// 代入演算子
		virtual RSObject * OperatorMove( RSContext& context, RSObject * pObj ) ;
		// 二項演算子
		virtual RSObject * OperatorCompareEQ( RSContext& context, RSObject * pObj ) const ;
		virtual RSObject * OperatorCompareNE( RSContext& context, RSObject * pObj ) const ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// ネイティブオブジェクトコンテナ
	//////////////////////////////////////////////////////////////////////////

	class	RSNativeObject	: public RSGenericObject
	{
	public:
		class	ObjectListener	: public ESLObject
		{
		public:
			ESL_DECLARE_CLASS_INFO( ObjectListener, ESLObject )
			virtual void OnDetach( RSNativeObject * pNObj ) = 0 ;
			virtual void OnRelease( RSNativeObject * pNObj ) = 0 ;
		} ;

	protected:
		SSystem::SSyncReference				m_refObject ;
		SSystem::SObjectArray<RSSmartPtr>	m_ownObjects ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSNativeObject, RSGenericObject )
		// 構築関数
		RSNativeObject
			( SSystem::SObject * pObj, RSClass * pClass = NULL )
				: RSGenericObject(pClass, typeOther), m_refObject(pObj) {}
		RSNativeObject( const RSNativeObject& nobj )
				: RSGenericObject(nobj.GetRSClass(), typeOther),
								m_refObject(nobj.m_refObject) {}
		// 消滅関数
		virtual ~RSNativeObject( void ) ;

	public:
		// オブジェクト取得
		SSystem::SObject * GetObject( void ) const ;
		static SSystem::SObject * GetNativeOf( RSObject * pObj ) ;
		template <class T> static T * GetNative( RSObject * pObj )
		{
			return	ESLTypeCast<T>( GetNativeOf( pObj ) ) ;
		}
		// オブジェクト設定
		void AttachObject( SSystem::SObject * pObj ) ;
		void SetObject( ESLObject * pObj ) ;
		static bool AttachNative( RSObject * pObj, SSystem::SObject * pNObj ) ;
		static bool SetNative( RSObject * pObj, SSystem::SObject * pNObj ) ;
		// オブジェクト分離
		SSystem::SObject * DetachObject( void ) ;
		void ReleaseNativeRef( void ) ;
		// オブジェクト所有判定
		bool IsObjectOwner( void ) const ;

	public:
		// オブジェクト所持
		void AddOwnObject( RSObject * pObj ) ;
		// オブジェクト解放
		void ReleaseOwnObject( RSObject * pObj ) ;
		void ReleaseOwnNativeObject( SSystem::SObject * pObj ) ;
		void ReleaseAllOwnObjects( void ) ;
		// 所持オブジェクト検索
		RSObject * FindOwnObject( const ESLRuntimeClass& rtClass ) const ;
		RSNativeObject * FindOwnNativeObject( SSystem::SObject * pObj ) const ;

	public:
		// 同定判定
		virtual bool IsEqualObject( RSObject * pObj ) const ;
		// 内部リソース解放
		virtual void DisposeObject( RSContext& context ) ;
		// 複製（参照の複製を含む）
		virtual RSObject * DuplicateObject( RSContext& context ) const ;
		// 複製（実体も可能な限り複製）
		virtual RSObject * CloneObject( RSContext& context ) const ;

	} ;

}

#endif

