
#if	!defined(__ROSETTA_ARRAY_BUFFER_H__)
#define	__ROSETTA_ARRAY_BUFFER_H__

namespace	Rosetta
{
	//////////////////////////////////////////////////////////////////////////
	// 型
	//////////////////////////////////////////////////////////////////////////

	typedef	uint8_t		rsBoolean ;
	typedef	int8_t		rsByte ;
	typedef	int16_t		rsShort ;
	typedef	uint16_t	rsChar ;
	typedef	int32_t		rsInt ;
	typedef	int64_t		rsLong ;
	typedef	float32_t	rsFloat ;
	typedef	float64_t	rsDouble ;



	//////////////////////////////////////////////////////////////////////////
	// バッファ
	//////////////////////////////////////////////////////////////////////////

	class	RSArrayBuffer	: public RSObject
	{
	public:
		SSystem::SArray<uint8_t>	m_buffer ;
		uint8_t *					m_ptrBuf ;
		size_t						m_lenBuf ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSArrayBuffer, RSObject )
		// 構築関数
		RSArrayBuffer( RSClass * pClass = NULL )
			: RSObject(pClass,typeOther), m_ptrBuf(NULL) { }
		// 消滅関数
		virtual ~RSArrayBuffer( void ) ;

	public:
		// バッファ確保
		void AllocateBuffer( size_t nBytes ) ;
		// バッファ関連付け
		void AttachBuffer( uint8_t * ptrBuf, size_t nBytes ) ;

	public:
		// 型名
		virtual const wchar_t * GetTypeName( void ) const ;
		// 文字列変換
		virtual bool AsString( SSystem::SString& strValue ) const ;
		// 内部リソース解放
		virtual void DisposeObject( RSContext& context ) ;
		// 複製（実体も可能な限り複製）
		virtual RSObject * CloneObject( RSContext& context ) const ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// バッファ参照数値
	//////////////////////////////////////////////////////////////////////////

	class	RSReferenceNumber	: public RSObject
	{
	public:
		enum	NumberType
		{
			typeUint8,
			typeInt8,
			typeUint16,
			typeInt16,
			typeUint32,
			typeInt32,
			typeInt64,
			typeFloat32,
			typeFloat64,
			typeCountOfNumber,
			typeObject	 = typeCountOfNumber,
		} ;

		NumberType	m_type ;
		void *		m_ptrBuffer ;
		RSObject *	m_pRef ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSReferenceNumber, RSObject )
		// 構築関数
		RSReferenceNumber
			( RSClass * pClass,
				void * ptrBuf, NumberType type, RSObject * pRef )
			: RSObject(pClass,typeReferenceNumber),
				m_type(type), m_ptrBuffer(ptrBuf), m_pRef(pRef) {}
		// 消滅関数
		virtual ~RSReferenceNumber( void ) ;

	public:
		// 要素サイズ計算
		static size_t GetNumberSizeOf( NumberType type ) ;
		// 要素型名取得
		static const wchar_t * GetNumberTypeName( NumberType type ) ;
		// プリミティブデータ型変換
		static NumberType FromPrimitiveType( RSPrimitiveNumberType type ) ;
		// 参照設定
		void SetReference
			( void * ptrBuf, NumberType type, RSObject * pRef ) ;
		// 整数値取得
		int64_t LoadInteger( void ) const ;
		// 整数値設定
		void StoreInteger( int64_t num ) const ;
		// 実数値取得
		double LoadRealNumber( void ) const ;
		// 実数値設定
		void StoreRealNumber( double num ) const ;

	public:
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

	public:	// オブジェクト
		// 内部リソース解放
		virtual void DisposeObject( RSContext& context ) ;
		// 複製（参照の複製を含む）
		virtual RSObject * DuplicateObject( RSContext& context ) const ;
		// 複製（実体も可能な限り複製）
		virtual RSObject * CloneObject( RSContext& context ) const ;

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
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 型付配列（ポインタ）
	//////////////////////////////////////////////////////////////////////////

	class	RSTypedArrayPointer	: public RSObject
	{
	public:
		RSReferenceNumber::NumberType	m_typeElement ;
		size_t							m_nElementBytes ;
		RSArrayBuffer *					m_pRefBuffer ;
		size_t							m_iOffset ;
		size_t							m_nLimit ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSTypedArrayPointer, RSObject )
		// 構築関数
		RSTypedArrayPointer
			( RSClass * pClass,
				RSArrayBuffer * pBuf,
				RSReferenceNumber::NumberType type,
				size_t iOffset = 0, ssize_t nLimit = -1 ) ;
		// 消滅関数
		virtual ~RSTypedArrayPointer( void ) ;
		// ポインタ設定
		void SetPointer
			( RSArrayBuffer * pBuf,
				RSReferenceNumber::NumberType type,
				size_t iOffset = 0, ssize_t nLimit = -1 ) ;
		// ポインタ取得
		uint8_t * GetPointer( void ) const ;
		uint8_t * GetPointer( size_t nReqBytes ) const ;
		template <class T> T * GetPtr( void ) const
		{
			return	(T*) GetPointer( sizeof(T) ) ;
		}

	public:
		// 型名
		virtual const wchar_t * GetTypeName( void ) const ;
		// 型テスト
		virtual RSObject * InstanceOf( const wchar_t * pwszType ) ;
		virtual RSObject * InstanceOf( RSClass * pClass ) ;
		// オブジェクト型か？
		virtual bool IsObjectType( void ) const ;
		// ブール判定
		virtual bool AsBoolean( void ) const ;
		// 文字列変換
		virtual bool AsString( SSystem::SString& strValue ) const ;
		// 同定判定
		virtual bool IsEqualObject( RSObject * pObj ) const ;

	public:
		// 要素取得
		virtual RSObject * GetElementAt
			( RSContext& context, int nIndex ) const ;
		// 要素取得
		virtual RSObject * SetElementAt
			( RSContext& context, int nIndex, RSObject * pObj ) ;
		// 要素数取得
		virtual size_t GetElementCount( void ) const ;
		// 要素最大数取得
		virtual size_t GetElementLimit( void ) const ;

	public:	// オブジェクト
		// 内部リソース解放
		virtual void DisposeObject( RSContext& context ) ;
		// 複製（参照の複製を含む）
		virtual RSObject * DuplicateObject( RSContext& context ) const ;
		// 複製（実体も可能な限り複製）
		virtual RSObject * CloneObject( RSContext& context ) const ;

	public:	// オペレーター
		// 単項演算子
		virtual RSObject * OperatorIncrement( RSContext& context ) ;
		virtual RSObject * OperatorDecrement( RSContext& context ) ;
		// 二項演算子
		virtual RSObject * OperatorAdd( RSContext& context, RSObject * pObj ) const ;
		virtual RSObject * OperatorSub( RSContext& context, RSObject * pObj ) const ;
		virtual RSObject * OperatorCompareEQ( RSContext& context, RSObject * pObj ) const ;
		virtual RSObject * OperatorCompareNE( RSContext& context, RSObject * pObj ) const ;
		// 代入演算子
		virtual RSObject * OperatorMove( RSContext& context, RSObject * pObj ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 構造体ポインタ
	//////////////////////////////////////////////////////////////////////////

	class	RSStructuredPointerClass ;
	class	RSStructuredPointer	: public RSTypedArrayPointer
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSStructuredPointer, RSTypedArrayPointer )
		// 構築関数
		RSStructuredPointer
			( RSStructuredPointerClass * pType,
				RSArrayBuffer * pBuf,
				size_t iOffset = 0, ssize_t nLimit = -1 ) ;
		// 消滅関数
		virtual ~RSStructuredPointer( void ) ;

	public:
		// 同定判定
		virtual bool IsEqualObject( RSObject * pObj ) const ;

	public:
		// ポインタ設定
		void SetPointer
			( RSArrayBuffer * pBuf,
				RSStructuredPointerClass * pType,
				size_t iOffset = 0, ssize_t nLimit = -1 ) ;
	public:
		// 要素取得
		virtual RSObject * GetElementAt
			( RSContext& context, int nIndex ) const ;
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
		// メンバ名取得
		const wchar_t * GetMemberNameAt( int nIndex ) const ;
		// メンバ数取得
		size_t GetMemberCount( void ) const ;

	public:	// オブジェクト
		// 内部リソース解放
		virtual void DisposeObject( RSContext& context ) ;
		// 複製（参照の複製を含む）
		virtual RSObject * DuplicateObject( RSContext& context ) const ;
		// 複製（実体も可能な限り複製）
		virtual RSObject * CloneObject( RSContext& context ) const ;

	public:	// オペレーター
		// 単項演算子
		virtual RSObject * OperatorIncrement( RSContext& context ) ;
		virtual RSObject * OperatorDecrement( RSContext& context ) ;
		// 二項演算子
		virtual RSObject * OperatorAdd( RSContext& context, RSObject * pObj ) const ;
		virtual RSObject * OperatorSub( RSContext& context, RSObject * pObj ) const ;
		// 代入演算子
		virtual RSObject * OperatorMove( RSContext& context, RSObject * pObj ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// ArrayBuffer 型
	//////////////////////////////////////////////////////////////////////////

	class	RSArrayBufferClass	: public RSClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSArrayBufferClass, RSClass )
		// 構築関数
		RSArrayBufferClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"ArrayBuffer" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;

	public:	// String method
		// void <init>( int length )
		static RSObject * method_init
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int length()
		static RSObject * method_length
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static void memset( ptrDst, byte fill, int nBytes )
		static RSObject * method_memset
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static void memmove( ptrDst, ptrSrc, int nBytes )
		static RSObject * method_memmove
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 型付配列（ポインタ）型
	//////////////////////////////////////////////////////////////////////////

	class	RSTypedArrayPointerClass	: public RSClass
	{
	public:
		RSReferenceNumber::NumberType	m_typeElement ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSTypedArrayPointerClass, RSClass )
		// 構築関数
		RSTypedArrayPointerClass
			( RSClass * pClass,
				const wchar_t * pwszClassName,
				RSReferenceNumber::NumberType type ) ;
		// メンバ初期設定
		virtual void Initialize( RSContext& context ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// 変数インスタンス生成
		virtual RSObject * NewVariable( RSContext& context ) ;

	public:
		// ポインタ要素サイズ取得
		virtual size_t GetElementBytes( void ) const ;
		// アライメントサイズ取得
		virtual size_t GetAlignment( void ) const ;
		// エレメント型名取得
		virtual const wchar_t * GetElementTypeName( void ) const ;

	public:
		// void <init>( int length )
		static RSObject * method_init1
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void <init>( Uint8Pointer ptr )
		static RSObject * method_init2
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void <init>( ArrayBuffer buf, int iOffset = 0, int nLength = -1 )
		static RSObject * method_init3
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// ArrayBuffer getBuffer()
		static RSObject * method_getBuffer
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int getOffset()
		static RSObject * method_getOffset
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int getBytes()
		static RSObject * method_getBytes
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int length()
		static RSObject * method_length
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 構造体型
	//////////////////////////////////////////////////////////////////////////

	class	RSStructuredPointerClass	: public RSTypedArrayPointerClass
	{
	public:
		class	ElementInfo
		{
		public:
			size_t							m_iOffset ;
			size_t							m_nBytes ;
			uint32_t						m_accMod ;
			RSReferenceNumber::NumberType	m_type ;
			RSClass *						m_pClass ;
			RSObject *						m_pInitObj ;
			RSCodeComment *					m_pComment ;

		public:
			// 構築関数
			ElementInfo( void )
				: m_iOffset(0), m_nBytes(0), m_accMod(0),
					m_type(RSReferenceNumber::typeUint8),
					m_pClass(NULL), m_pInitObj(NULL), m_pComment(NULL) {}
			ElementInfo( const ElementInfo& ei )
				: m_iOffset(ei.m_iOffset), m_nBytes(ei.m_nBytes),
					m_accMod(ei.m_accMod), m_type(ei.m_type),
					m_pClass(ei.m_pClass),
					m_pInitObj(ei.m_pInitObj), m_pComment(ei.m_pComment)
			{
				RSObject::AddRef( m_pInitObj ) ;
			}
			// 消滅関数
			~ElementInfo( void )
			{
				RSObject::ReleaseRef( m_pInitObj ) ;
				m_pInitObj = NULL ;
			}
		} ;

	protected:
		SSystem::SArray<uint8_t>			m_bufInit ;		// 初期値
		SSystem::SStrSortArray<ElementInfo>	m_ssaElements ;
		SSystem::SArray<size_t>				m_aOrderedIndex ;
		size_t								m_iNextOffset ;	// 次に割り当てるオフセット
		size_t								m_nBaseAlign ;	// 構造体の基本アライメント
		size_t								m_nMaxAlign ;	// 要素の最大アライメント

		struct	SuperStructCast
		{
			RSStructuredPointerClass *	pStruct ;
			size_t						nOffset ;

			SuperStructCast( void ) : pStruct(NULL), nOffset(0) {}
			SuperStructCast( const SuperStructCast& ssc )
				: pStruct(ssc.pStruct), nOffset(ssc.nOffset) {}
		} ;
		SSystem::SArray<SuperStructCast>	m_lstStructCast ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSStructuredPointerClass, RSTypedArrayPointerClass )
		// 構築関数
		RSStructuredPointerClass
			( RSClass * pClass, const wchar_t * pwszClassName ) ;
		// ポインタ要素サイズ取得
		virtual size_t GetElementBytes( void ) const ;
		// アライメントサイズ取得
		virtual size_t GetAlignment( void ) const ;
		// エレメント型名取得
		virtual const wchar_t * GetElementTypeName( void ) const ;

	public:
		// メンバ初期設定
		virtual void Initialize( RSContext& context ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;
		// クラス定義の完了
		virtual void FinishClass( RSContext& context ) ;
		// 派生元クラス追加
		void AddSuperClass( RSContext& context, RSClass * pSuperClass ) ;
		void AddImplementClass( RSContext& context, RSClass * pSuperClass ) ;
		// 変数インスタンス生成
		virtual RSObject * NewVariable( RSContext& context ) ;
		// キャスト処理
		virtual RSObject * CastInstance
			( RSContext& context, RSObject * pObj,
							CastMethod castMethod = castNatural ) ;
		// 親構造体キャストオフセット計算
		ssize_t OffsetSuperStruct
			( const RSStructuredPointerClass * pStruct ) const ;
		// クラスメンバダンプ
		virtual void DumpClassPrototypeMembers
			( RSContext& context, SSystem::SString& strDecl ) ;
		static void DumpElementDeclaration
			( RSContext& context, SSystem::SString& strDecl,
				const wchar_t * pwszName, const ElementInfo * peiMember ) ;
		// オフセット順のメンバ指標取得
		const size_t * GetOrderedMemberIndex( void ) ;
		void GetOrderedMemberIndex( SSystem::SArray<size_t>& bufIndex ) const ;

	public:
		// メンバ変数情報追加
		void AddArrayMemberAs
			( RSContext& context,
				const wchar_t * pwszName, RSClass * pType,
				uint32_t accMod = 0, size_t nArray = 0,
				RSObject * pInitObj = NULL, RSCodeComment * pComment = NULL ) ;
		// 構造体初期値設定
		void SetStructureInitValue
			( RSContext& context, size_t iOffset, size_t nBytes,
				RSReferenceNumber::NumberType type,
				RSClass * pClass, RSObject * pInitObj ) ;
		// メンバ変数情報取得
		ElementInfo * GetArrayMemberAs( const wchar_t * pwszName ) const ;
		ElementInfo * GetArrayMemberAt( size_t nIndex ) const ;
		//メンバ変数名取得
		const wchar_t * GetArrayMemberNameAt( size_t nIndex ) const ;
		// メンバ変数数取得
		size_t GetArrayMemberCount( void ) const ;
		// バッファ生成
		RSArrayBuffer * NewBuffer( RSContext& context, size_t nLength = 1 ) ;
		// 構造体初期値
		const uint8_t * GetStructureInit( void ) const
		{
			return	m_bufInit.GetConstArray() ;
		}
		// 構造体サイズ取得
		size_t GetStructureBytes( void ) const
		{
			return	m_iNextOffset ;
		}
		// アライメント取得
		size_t GetStructureAlign( void ) const
		{
			return	m_nMaxAlign ;
		}

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 構造体基底クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSStructureClass	: public RSStructuredPointerClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSStructureClass, RSStructuredPointerClass )
		// 構築関数
		RSStructureClass
			( RSClass * pClass, const wchar_t * pwszClassName ) ;

	public:
		// メンバ初期設定
		virtual void Initialize( RSContext& context ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;

	public:
		// void <init>( int length = 1 )
		static RSObject * method_init1
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void <init>( Uint8Pointer ptr )
		static RSObject * method_init2
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void <init>( ArrayBuffer buf, int iOffset = 0, int nLength = -1 )
		static RSObject * method_init3
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// boolean equals( Object obj )
		static RSObject * method_equals
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static int sizeof( Class cls )
		static RSObject * method_sizeof1
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static int sizeof( Structure cls )
		static RSObject * method_sizeof2
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;

}

#endif

