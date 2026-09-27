
#if	!defined(__ROSETTA_GENERIC_OBJECT_H__)
#define	__ROSETTA_GENERIC_OBJECT_H__

namespace	Rosetta
{
	class	RSParenthesis ;
	class	RSScript ;
	class	RSFunctionPrototype ;

	//////////////////////////////////////////////////////////////////////////
	// RSScript コンテナ
	//////////////////////////////////////////////////////////////////////////

	class	RSScriptOwner	: public ESLObject
	{
	protected:
		RSScript *	m_prsScript ;

	public:
		ESL_DECLARE_CLASS_INFO( RSScriptOwner, ESLObject )
		RSScriptOwner( void ) : m_prsScript(nullptr) {}
		virtual ~RSScriptOwner( void ) ;
		void SetOwnScript( RSScript * pScript ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 汎用クラス実装
	//////////////////////////////////////////////////////////////////////////

	class	RSGenericClassMembers
	{
	public:
		SSystem::SStrSortArray<RSObject*>	m_members ;

	public:
		// 消滅関数
		~RSGenericClassMembers( void ) ;

	public:	// メンバ
		// デバッグ用ダンプ文字列
		void ToDebugDump
			( SSystem::SFileInterface& dump,
					size_t nPtrNest = 10,
					const wchar_t * pwszIndent = NULL ) ;
		// 複製（参照の複製を含む）
		void DuplicateAllMembers
			( RSContext& context,
				const SSystem::SStrSortArray<RSObject*>& members ) ;
		// 複製（実体も可能な限り複製）
		void CloneAllMembers
			( RSContext& context,
				const SSystem::SStrSortArray<RSObject*>& members ) ;
		// 全メンバに DisposeObject を呼び出す
		void DisposeAllMembers( RSContext& context ) ;
		// 全メンバ削除
		void RemoveAllMembers( void ) ;
		// メンバ削除
		void RemoveMemberAs( const wchar_t * pwszName ) ;
		RSObject * DetachMemberAs( const wchar_t * pwszName ) ;
		// メンバ数取得
		size_t GetMemberCount( void ) const ;
		// メンバ取得
		RSObject * GetMemberAt( size_t nIndex ) const ;
		// メンバ数取得
		const wchar_t * GetMemberNameAt( size_t nIndex ) const ;
		// メンバ取得
		RSObject * GetMemberAs( const wchar_t * pwszName ) const ;
		// メンバ設定
		RSObject * SetMemberAs
			( RSContext& context, const wchar_t * pwszName, RSObject * pObj ) ;

	public:
		// シリアライズ
		SSystem::SError SerializeBinary
				( RSContext& context, SSystem::SFileInterface& file ) ;
		SSystem::SError MakeXMLDocument
				( RSContext& context, SSystem::SXMLDocument& xmlDoc ) ;
		// 復元
		SSystem::SError RestoreBinary
				( RSContext& context, SSystem::SFileInterface& file ) ;
		SSystem::SError RestoreXMLDocument
				( RSContext& context, const SSystem::SXMLDocument& xmlDoc ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 汎用クラス実装（動的メンバ型／スレッド同期）
	//////////////////////////////////////////////////////////////////////////

	class	RSDynamicTypeClassMembers	: public RSGenericClassMembers
	{
	public:
		// 消滅関数
		~RSDynamicTypeClassMembers( void ) ;

	public:
		// 複製（参照の複製を含む）
		void DuplicateAllMembers
			( RSContext& context,
				const SSystem::SStrSortArray<RSObject*>& members ) ;
		// 複製（実体も可能な限り複製）
		void CloneAllMembers
			( RSContext& context,
				const SSystem::SStrSortArray<RSObject*>& members ) ;
		// 全メンバ削除
		void RemoveAllMembers( void ) ;
		// メンバ削除
		void RemoveMemberAs( const wchar_t * pwszName ) ;
		// メンバ数取得
		size_t GetMemberCount( void ) const ;
		// メンバ取得
		RSObject * GetMemberAt( size_t nIndex ) const ;
		// メンバ数取得
		const wchar_t * GetMemberNameAt( size_t nIndex ) const ;
		// メンバ取得
		RSObject * GetMemberAs( const wchar_t * pwszName ) const ;
		// メンバ設定
		RSObject * SetMemberAs
			( RSContext& context, const wchar_t * pwszName, RSObject * pObj ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 汎用オブジェクト（JavaScript Object 相当）
	//////////////////////////////////////////////////////////////////////////

	class	RSGenericObject ;
	class	RSDynamicObject	: public RSObject, public RSScriptOwner
	{
	public:
		enum	PropertyMode
		{
			modePropAllAccess,
			modePropReadWrite,
			modePropReadOnly,
		} ;

	protected:
		RSDynamicTypeClassMembers	m_dtcmProperties ;

		PropertyMode	m_modeProp ;
		RSClass *		m_pPropClass ;
		RSObject *		m_pDefElement ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2( RSDynamicObject, RSObject, RSScriptOwner )
		// 構築関数
		RSDynamicObject
			( RSClass * pClass,
				BasicType type = typeObject,
				PropertyMode modeProp = modePropAllAccess )
			: RSObject(pClass, type), m_modeProp(modeProp),
				m_pPropClass(nullptr), m_pDefElement(nullptr) {}
		RSDynamicObject
			( RSContext& context, const RSDynamicObject& obj ) ;
		// 消滅関数
		virtual ~RSDynamicObject( void ) ;

	public:
		// メンバ型情報
		RSClass * GetMemberClass( void ) const
		{
			return	m_pPropClass ;
		}
		void SetMemberClass( RSClass * pPropClass ) ;
		// デフォルトエレメント設定
		void SetDefaultElement( RSObject * pDefElement ) ;
		// アクセスモード
		PropertyMode GetPropertyMode( void ) const
		{
			return	m_modeProp ;
		}
		void SetPropertyMode( PropertyMode mode )
		{
			m_modeProp = mode ;
		}
		// プロパティ
		RSDynamicTypeClassMembers& GetProperties( void )
		{
			return	m_dtcmProperties ;
		}

	public:	// 型情報
		// 即値型か？
		virtual bool IsObjectType( void ) const ;
		// 文字列変換
		virtual bool AsString( SSystem::SString& strValue ) const ;
		// デバッグ用ダンプ文字列
		virtual void ToDebugDump
			( SSystem::SFileInterface& dump,
					size_t nPtrNest = 10,
					const wchar_t * pwszIndent = NULL ) ;

	public:	// オブジェクト
		// 内部リソース解放
		virtual void DisposeObject( RSContext& context ) ;
		// 複製（参照の複製を含む）
		virtual RSObject * DuplicateObject( RSContext& context ) const ;
		// 複製（実体も可能な限り複製）
		virtual RSObject * CloneObject( RSContext& context ) const ;

	public:	// メンバ・プロパティ
		// 要素取得
		virtual RSObject * GetElementAt( RSContext& context, int nIndex ) const ;
		// 要素名取得
		virtual const wchar_t * GetElementNameAt( int nIndex ) const ;
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
		// メンバ削除
		virtual RSObject * RemoveMemberAs
			( RSContext& context, const wchar_t * pwszName ) ;

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

		friend class RSGenericObject ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 汎用オブジェクト（静的メンバ型）
	//////////////////////////////////////////////////////////////////////////

	class	RSGenericObject	: public RSObject, public RSScriptOwner
	{
	protected:
		RSGenericClassMembers	m_gcmMembers ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2( RSGenericObject, RSObject, RSScriptOwner )
		// 構築関数
		RSGenericObject
			( RSClass * pClass,
				BasicType type = typeGenericObject,
				uint32_t accMode = modifierPublic )
			: RSObject(pClass, type, accMode) {}
		// 消滅関数
		virtual ~RSGenericObject( void ) ;

	public:
		// static 関数設定
		void SetFunctionMemberAs
			( RSContext& context,
				const wchar_t * pwszName,
				RSFunctionPrototype * pProto, bool fOverride = true ) ;
		RSFunctionPrototype * AddFunctionDescriptiveAs
			( RSContext& context,
				SSystem::SParserErrorInterface& perr,
				const wchar_t * pwszName,
				const wchar_t * pwszType,
				const wchar_t * pwszArgList,
				RSParenthesis * pParenthesis,
				RSObject::METHOD_PROC pfnMethod = NULL,
				void * pMethodInstance = NULL,
				RSCodeComment * pComment = NULL ) ;

	public:	// 型情報
		// 即値型か？
		virtual bool IsObjectType( void ) const ;
		// 文字列変換
		virtual bool AsString( SSystem::SString& strValue ) const ;
		// デバッグ用ダンプ文字列
		virtual void ToDebugDump
			( SSystem::SFileInterface& dump,
					size_t nPtrNest = 10,
					const wchar_t * pwszIndent = NULL ) ;

	public:	// オブジェクト
		// 内部リソース解放
		virtual void DisposeObject( RSContext& context ) ;
		// 複製（参照の複製を含む）
		virtual RSObject * DuplicateObject( RSContext& context ) const ;
		// 複製（実体も可能な限り複製）
		virtual RSObject * CloneObject( RSContext& context ) const ;

	public:	// メンバ・プロパティ
		// 要素取得
		virtual RSObject * GetElementAt( RSContext& context, int nIndex ) const ;
		// 要素名取得
		virtual const wchar_t * GetElementNameAt( int nIndex ) const ;
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
		// メンバ削除
		virtual RSObject * RemoveMemberAs
			( RSContext& context, const wchar_t * pwszName ) ;

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


}

#endif
