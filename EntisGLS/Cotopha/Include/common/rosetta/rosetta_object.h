
#if	!defined(__ROSETTA_OBJECT_H__)
#define	__ROSETTA_OBJECT_H__

namespace	Rosetta
{
	class	RSVirtualMachine ;
	class	RSContext ;
	class	RSCodeComment ;
	class	RSClass ;
	class	RSStructuredPointerClass ;
	class	RSFunctionObject ;
	class	RSArray ;


	//////////////////////////////////////////////////////////////////////////
	// 基本プリミティブ型
	//////////////////////////////////////////////////////////////////////////

	enum	RSPrimitiveNumberType
	{
		typeNumberUint8,
		typeNumberInt8,
		typeNumberUint16,
		typeNumberInt16,
		typeNumberUint32,
		typeNumberInt32,
		typeNumberInt64,
		typeNumberFloat32,
		typeNumberFloat64,
		typeCountOfNumber,
		typePrimitiveObject	 = typeCountOfNumber,
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 基底オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	RSObject	: public SSystem::SObject
	{
	public:
		// 基本型
		enum	BasicType
		{
			typeInvalid	= -1,
			typeNumber,				// JavaScript 相当 Number
			typeInteger,			// 64ビット整数型
			typeBoolean,			// ブール型
			typeString,				// 文字列型
			typeArray,				// 配列型
			typeReference,			// 参照型
			typePointer,			// ポインタ型
			typeReferenceNumber,	// 数値参照型（型付配列要素）
			typePointerNumber,		// 型付配列型（ポインタ相当）
			typeStrucuredPointer,	// 構造体ポインタ型
			typeNamespace,			// 名前空間
			typeFunction,			// 関数型
			typeClass,				// クラス型
			typeObject,				// JavaScript 相当 Object
			typeGenericObject,		// 型付きクラスオブジェクト
			typeOther,
		} ;
		// アクセス修飾子
		enum	AccessModifier
		{
			modifierPublic,
			modifierProtected,
			modifierPrivate,
			modifierPrivateInvisible,
			accessMask				= 0x0003,
			modifierStatic			= 0x0100,
			modifierAbstract		= 0x0200,
			modifierConst			= 0x0400,
			modifierNative			= 0x0800,
			modifierSynchronized	= 0x1000,
			modifierExtern			= 0x2000,
		} ;
		// 排他同期
		struct	Synchronized
		{
			RSContext *		pContext ;
			atomic_int_t	countSync ;
		} ;
		// オブジェクトモニタ
		class	Monitor	: public SSystem::SSignalEvent
		{
		public:
			// モニタ種類
			enum	Type
			{
				typeSynchronized,
				typeNotification,
			} ;
			Type		m_type ;
			Monitor *	m_pNextChain ;
		public:
			// クラス情報
			ESL_DECLARE_CLASS_INFO( Monitor, SSignalEvent )
			// 構築関数
			Monitor( Type type ) : m_type(type), m_pNextChain(NULL) {}
		} ;

	protected:
		Synchronized *		m_pSynchronized ;
		Monitor *			m_pFirstMonitor ;
		RSClass *			m_pClass ;
		BasicType			m_typeObj ;
		uint32_t			m_accModifier ;
		atomic_int_t		m_countRef ;
		RSCodeComment *		m_pComment ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSObject, SObject )
		// 構築関数
		RSObject
			( RSClass * pClass, BasicType type,
				uint32_t accMod = modifierPublic )
			: m_pSynchronized(NULL), m_pFirstMonitor(NULL),
				m_pClass(pClass), m_typeObj(type),
				m_accModifier(accMod),
				m_countRef(1), m_pComment(NULL) {}
		RSObject( const RSObject& obj )
			: m_pSynchronized(NULL), m_pFirstMonitor(NULL),
				m_pClass(obj.m_pClass), m_typeObj(obj.m_typeObj),
				m_accModifier(obj.m_accModifier),
				m_countRef(1), m_pComment(obj.m_pComment) {}
		// 消滅関数
		virtual ~RSObject( void ) ;

	public:
		// 参照確保
		static RSObject * AddRef( RSObject * pObj )
		{
			if ( pObj != NULL )
			{
				SSystem::AtomicAdd( &(pObj->m_countRef), 1 ) ;
				return	pObj ;
			}
			return	NULL ;
		}
		void AddRef( void )
		{
			ESLAssert( this != NULL ) ;
			SSystem::AtomicAdd( &m_countRef, 1 ) ;
		}
		// 解放
		static void ReleaseRef( RSObject * pObj )
		{
			if ( pObj != NULL )
			{
				if ( SSystem::AtomicSub( &(pObj->m_countRef), 1 ) <= 0 )
				{
					delete	pObj ;
				}
			}
		}
		void ReleaseRef( void )
		{
			ESLAssert( this != NULL ) ;
			if ( SSystem::AtomicSub( &m_countRef, 1 ) <= 0 )
			{
				delete	this ;
			}
		}
		// 排他同期取得
		SSystem::SError LockSynchronized
				( Synchronized * sync, RSContext * context ) ;
		// 排他同期解放
		bool UnlockSynchronized( RSContext * context ) ;
		// モニタ待機
		SSystem::SError WaitNotification
					( RSContext& context, int64_t timeout = 0 ) ;
		// モニタ追加
		void AddMonitor( Monitor * monitor ) ;
		// モニタ分離
		void DetachMonitor( Monitor * monitor ) ;
		// モニタ通知と分離
		void NotifyMonitor( Monitor::Type typeNotify ) ;
		void NotifyAllMonitor( Monitor::Type typeNotify ) ;

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
		// オブジェクト型か？
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
		// 動的型変数
		bool IsDynamicType( void ) const
		{
			return	(m_typeObj == typeObject) ;
		}
		// 静的型変数
		bool IsStaticType( void ) const
		{
			return	(m_typeObj != typeObject) ;
		}
		// 型取得
		BasicType GetBasicType( void ) const
		{
			return	m_typeObj ;
		}
		// クラス取得
		RSClass * GetRSClass( void ) const
		{
			return	m_pClass ;
		}
		// クラス設定
		void SetRSClass( RSClass * pClass )
		{
			m_pClass = pClass ;
		}
		// アクセス可能判定
		bool IsEnableAccessModifier( uint32_t accMod ) const
		{
			return	((m_accModifier & accessMask) <= (accMod & accessMask)) ;
		}
		// 修飾子取得
		uint32_t GetModifiers( void ) const
		{
			return	m_accModifier ;
		}
		// 修飾子設定
		void SetModifiers( uint32_t accModifiers )
		{
			m_accModifier = accModifiers ;
		}
		// アクセス修飾子
		AccessModifier GetAccessModifier( void ) const
		{
			return	(AccessModifier) (m_accModifier & accessMask) ;
		}
		void SetAccessModifier( AccessModifier accMod )
		{
			m_accModifier = (m_accModifier & ~accessMask) | accMod ;
		}
		// static 修飾
		bool IsStaticModifier( void ) const
		{
			return	(m_accModifier & modifierStatic) != 0 ;
		}
		void SetStaticModifier( bool fStatic )
		{
			m_accModifier = (m_accModifier & ~modifierStatic)
								| (fStatic ? modifierStatic : 0) ;
		}
		// abstract 修飾
		bool IsAbstractModifier( void ) const
		{
			return	(m_accModifier & modifierAbstract) != 0 ;
		}
		void SetAbstractModifier( bool fAbs )
		{
			m_accModifier = (m_accModifier & ~modifierAbstract)
								| (fAbs ? modifierAbstract : 0) ;
		}
		// const 修飾
		bool IsConstModifier( void ) const
		{
			return	(m_accModifier & modifierConst) != 0 ;
		}
		void SetConstModifier( bool fConst )
		{
			m_accModifier = (m_accModifier & ~modifierConst)
								| (fConst ? modifierConst : 0) ;
		}
		// synchronized 修飾
		bool IsSynchronizedModifier( void ) const
		{
			return	(m_accModifier & modifierSynchronized) != 0 ;
		}
		// 定義コメント
		RSCodeComment * GetDefinitionComment( void ) const
		{
			return	m_pComment ;
		}
		void SetDefinitionComment( RSCodeComment * pComment )
		{
			m_pComment = pComment ;
		}
		// 値設定
		virtual SSystem::SError SetIntegerAs( int64_t nValue ) ;
		virtual SSystem::SError SetNumberAs( double nValue ) ;
		virtual SSystem::SError SetStringAs( const wchar_t * pwszValue ) ;
		// 配列要素取得
		int64_t GetElementIntegerAt
			( RSContext& context, int nIndex,
				int64_t nDefault = 0, bool * pError = NULL ) ;
		double GetElementNumberAt
			( RSContext& context, int nIndex,
				double nDefault = 0, bool * pError = NULL ) ;
		SSystem::SString GetElementStringAt
			( RSContext& context, int nIndex,
				const wchar_t * pwszDefault = NULL, bool * pError = NULL ) ;
		// 配列要素設定
		SSystem::SError SetElementIntegerAt
			( RSContext& context, int nIndex, int64_t nValue ) ;
		SSystem::SError SetElementNumberAt
			( RSContext& context, int nIndex, double nValue ) ;
		SSystem::SError SetElementStringAt
			( RSContext& context, int nIndex, const wchar_t * pwszValue ) ;
		// メンバ要素取得
		int64_t GetMemberIntegerAs
			( RSContext& context, const wchar_t * pwszMember,
				int64_t nDefault = 0, bool * pError = NULL ) ;
		double GetMemberNumberAs
			( RSContext& context, const wchar_t * pwszMember,
				double nDefault = 0, bool * pError = NULL ) ;
		SSystem::SString GetMemberStringAs
			( RSContext& context, const wchar_t * pwszMember,
				const wchar_t * pwszDefault = NULL, bool * pError = NULL ) ;
		SSystem::SObject * GetMemberNativeObjAs
			( RSContext& context, const wchar_t * pwszMember, bool * pError = NULL ) ;
		uint8_t * GetMemberNativePtrAs
			( RSContext& context, const wchar_t * pwszMember, size_t nReqBytes ) ;
		// メンバ要素設定
		SSystem::SError SetMemberIntegerAs
			( RSContext& context, const wchar_t * pwszMember, int64_t nValue ) ;
		SSystem::SError SetMemberNumberAs
			( RSContext& context, const wchar_t * pwszMember, double nValue ) ;
		SSystem::SError SetMemberStringAs
			( RSContext& context,
				const wchar_t * pwszMember, const wchar_t * pwszValue ) ;
		SSystem::SError SetMemberNativeObjRefAs
			( RSContext& context, const wchar_t * pwszMember,
						SSystem::SObject * pRef, RSClass * pClass ) ;
		SSystem::SError SetMemberNativePtrRefAs
			( RSContext& context,
				const wchar_t * pwszMember,
				uint8_t * ptrBuf, size_t nBytes,
				RSPrimitiveNumberType type ) ;
		SSystem::SError SetMemberStructPtrRefAs
			( RSContext& context,
				const wchar_t * pwszMember,
				uint8_t * ptrBuf, size_t nBytes,
				RSStructuredPointerClass * pType ) ;
		// メンバ要素作成
		SSystem::SError CreateMemberIntegerAs
			( RSContext& context,
				const wchar_t * pwszMember,
				int64_t nValue, uint32_t accMod = 0 ) ;
		SSystem::SError CreateMemberNumberAs
			( RSContext& context,
				const wchar_t * pwszMember,
				double nValue, uint32_t accMod = 0 ) ;
		SSystem::SError CreateMemberStringAs
			( RSContext& context,
				const wchar_t * pwszMember,
				const wchar_t * pwszValue, uint32_t accMod = 0 ) ;
		// メンバ全削除
		void RemoveAllMembers( RSContext& context ) ;

	public:	// オブジェクト
		// オブジェクト解放処理
		virtual void Finalize( RSContext& context ) ;
		// 内部リソース解放
		virtual void DisposeObject( RSContext& context ) ;
		// 複製（参照の複製を含む）
		virtual RSObject * DuplicateObject( RSContext& context ) const ;
		// 複製（実体も可能な限り複製）
		virtual RSObject * CloneObject( RSContext& context ) const = 0 ;
		// 実体
		virtual RSObject * GetEntityObject( void ) const ;
		// 実体型
		virtual RSClass * GetEntityClass( void ) const ;

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
		// メンバ削除
		virtual RSObject * RemoveMemberAs
			( RSContext& context, const wchar_t * pwszName ) ;

	public:	// 関数
		typedef	RSObject * (*METHOD_PROC)
			( RSContext& context, void * pInstance,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		struct	METHOD_ENTRY
		{
			METHOD_PROC	pfnMethod ;
			void *		pInstance ;
		} ;
		// メソッド取得
		virtual METHOD_ENTRY * GetMethodAs
			( RSContext& context,
				const wchar_t * pwszName, METHOD_ENTRY& method ) const ;
		// メソッド呼び出し
		RSObject * CallMethodAs
			( RSContext& context,
				const wchar_t * pwszFuncName,
				RSObject*const* ppArgs, size_t countArg,
				bool fStructCast = true, bool* pArgMatchResult = NULL ) ;

	public:
		// スクリプト上のメソッド呼び出し関数
		static RSObject * methodRuntimeFunction
			( RSContext& context, void * pInstance,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;

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
		virtual RSObject * OperatorMoveMul( RSContext& context, RSObject * pObj ) ;
		virtual RSObject * OperatorMoveDiv( RSContext& context, RSObject * pObj ) ;
		virtual RSObject * OperatorMoveMod( RSContext& context, RSObject * pObj ) ;
		virtual RSObject * OperatorMoveAdd( RSContext& context, RSObject * pObj ) ;
		virtual RSObject * OperatorMoveSub( RSContext& context, RSObject * pObj ) ;
		virtual RSObject * OperatorMoveShiftLeft( RSContext& context, RSObject * pObj ) ;
		virtual RSObject * OperatorMoveShiftRight( RSContext& context, RSObject * pObj ) ;
		virtual RSObject * OperatorMoveBitShiftRight( RSContext& context, RSObject * pObj ) ;
		virtual RSObject * OperatorMoveBitAnd( RSContext& context, RSObject * pObj ) ;
		virtual RSObject * OperatorMoveBitOr( RSContext& context, RSObject * pObj ) ;
		virtual RSObject * OperatorMoveBitXor( RSContext& context, RSObject * pObj ) ;

	public:
		// シリアライズ
		virtual RSObject * SerializeObject( RSContext& context ) ;
		virtual SSystem::SError SerializeBinary
				( RSContext& context, SSystem::SFileInterface& file ) ;
		virtual SSystem::SError MakeXMLDocument
				( RSContext& context, SSystem::SXMLDocument& xmlDoc ) ;
		static SSystem::SError SaveObjectBinary
				( RSObject * pObj, RSContext& context, SSystem::SFileInterface& file ) ;
		static SSystem::SError SaveObjectDirectBinary
				( RSObject * pObj, RSContext& context, SSystem::SFileInterface& file ) ;
		static SSystem::SError MakeXMLDocumentOfObject
				( RSObject * pObj, RSContext& context, SSystem::SXMLDocument& xmlDoc ) ;
		// 復元
		virtual SSystem::SError RestoreObject
				( RSContext& context, RSObject * pObj ) ;
		virtual SSystem::SError RestoreBinary
				( RSContext& context, SSystem::SFileInterface& file ) ;
		virtual SSystem::SError RestoreXMLDocument
				( RSContext& context, const SSystem::SXMLDocument& xmlDoc ) ;
		static RSObject * LoadObjectBinary
				( RSContext& context, SSystem::SFileInterface& file ) ;
		static RSObject * LoadObjectDirectBinary
				( RSContext& context, SSystem::SFileInterface& file ) ;
		static RSObject * RestoreObjectOfXMLDocument
				( RSContext& context, const SSystem::SXMLDocument& xmlDoc ) ;

		friend class RSContext ;
	} ;

}

#endif

