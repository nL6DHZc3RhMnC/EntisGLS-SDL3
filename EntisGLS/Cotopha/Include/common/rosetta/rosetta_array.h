
#if	!defined(__ROSETTA_ARRAY_H__)
#define	__ROSETTA_ARRAY_H__

namespace	Rosetta
{
	//////////////////////////////////////////////////////////////////////////
	// 配列基底オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	RSArray	: public RSObject
	{
	public:
		SSystem::SPointerArray<RSObject>	m_elements ;
		size_t								m_limitLength ;
		RSClass *							m_pElementClass ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSArray, RSObject )
		// 構築関数
		RSArray( RSClass * pClass,
				size_t nLimit = 0x7FFFFFFF, RSClass * pElementClass = NULL )
			: RSObject(pClass,typeArray),
				m_limitLength(nLimit), m_pElementClass(pElementClass) {}
		// 消滅関数
		virtual ~RSArray( void ) ;
		// 配列型設定
		void SetArrayPrototype( size_t nLimit, RSClass * pElementClass ) ;

	public:	// 型情報
		// 型名
		virtual const wchar_t * GetTypeName( void ) const ;
		// 型テスト
		virtual RSObject * InstanceOf( const wchar_t * pwszType ) ;
		virtual RSObject * InstanceOf( RSClass * pClass ) ;
		// オブジェクト型か？
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
		void DuplicateAllElements
			( RSContext& context,
				const SSystem::SPointerArray<RSObject>& elements ) ;
		// 複製（実体も可能な限り複製）
		virtual RSObject * CloneObject( RSContext& context ) const ;
		void CloneAllElements
			( RSContext& context,
				const SSystem::SPointerArray<RSObject>& elements ) ;

	public:	// メンバ・プロパティ
		// 要素取得
		virtual RSObject * GetElementAt
			( RSContext& context, int nIndex ) const ;
		// 要素設定
		virtual RSObject * SetElementAt
			( RSContext& context, int nIndex, RSObject * pObj ) ;
		// 要素数取得
		virtual size_t GetElementCount( void ) const ;
		// 要素最大数取得
		virtual size_t GetElementLimit( void ) const ;

	public:	// 配列操作
		// 要素全削除
		void RemoveAllElements( void ) ;
		void RemoveAllElements( RSContext& context ) ;
		// 配列長さ変更（切り詰め／拡張）
		void SetArrayLength( size_t nLength, RSContext& context ) ;
		// 要素追加（特定型）
		void AddIntegerElement( RSContext& context, int64_t num ) ;
		void AddNumberElement( RSContext& context, double num ) ;
		void AddStringElement( RSContext& context, const wchar_t * pwszString ) ;
		void AddObjectElement( RSObject * pObj ) ;
		// 配列取得
		RSObject*const* GetConstArrayPointer( void ) const ;

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
	// 配列ジェネリック型
	//////////////////////////////////////////////////////////////////////////

	class	RSGenericArrayClass	: public RSClass
	{
	public:
		RSClass *	m_pElementClass ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSGenericArrayClass, RSClass )
		// 構築関数
		RSGenericArrayClass
			( RSClass * pClass,
				const wchar_t * pwszClassName,
				RSClass * pElementClass = NULL,
				RSObject * pRefNamespace = NULL ) ;
		// 消滅関数
		virtual ~RSGenericArrayClass( void ) ;

	public:
		// クラス型テスト
		bool IsInstanceOf( const wchar_t * pwszClass ) const ;
		bool IsInstanceOf( RSClass * pClass ) const ;
		// インスタンス生成
		virtual RSObject * NewInstance( RSContext& context, RSObject * pArg ) ;
		// キャスト処理
		virtual bool TestCastInstance
			( RSObject * pObj, CastMethod castMethod = castNatural ) ;
		virtual RSObject * CastInstance
			( RSContext& context, RSObject * pObj, CastMethod castMethod ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// Array 型クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSArrayClass	: public RSGenericArrayClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSArrayClass, RSGenericArrayClass )
		// 構築関数
		RSArrayClass
			( RSClass * pClass,
				const wchar_t * pwszClassName,
				RSClass * pElementClass = NULL,
				RSObject * pRefNamespace = NULL ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		virtual void OverrideVirtualsGenerics( RSContext& context ) ;
		void RemoveGenericFunctions( RSContext& context ) ;

	public:	// Array method
		// int length()
		static RSObject * method_length
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int indexOf( obj, int from = 0 )
		static RSObject * method_indexOf
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean add( obj )
		static RSObject * method_add1
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void add( int index, obj )
		static RSObject * method_add2
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Object remove( int index )
		static RSObject * method_remove
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void clear()
		static RSObject * method_clear
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int push( obj )
		static RSObject * method_push
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Object pop()
		static RSObject * method_pop
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// Object shift()
		static RSObject * method_shift
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void unshift( obj )
		static RSObject * method_unshift
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;


}

#endif
