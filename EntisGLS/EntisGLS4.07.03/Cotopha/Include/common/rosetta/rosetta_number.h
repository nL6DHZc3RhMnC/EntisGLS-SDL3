
#if	!defined(__ROSETTA_NUMBER_H__)
#define	__ROSETTA_NUMBER_H__

namespace	Rosetta
{
	//////////////////////////////////////////////////////////////////////////
	// 数値
	//////////////////////////////////////////////////////////////////////////

	class	RSNumber	: public RSObject
	{
	public:
		bool	m_flagInt32 ;
		int32_t	m_intValue ;
		double	m_fpValue ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSNumber, RSObject )
		// 構築関数
		RSNumber( RSClass * pClass, int32_t num = 0, BasicType type = typeNumber )
			: RSObject(pClass,type), m_flagInt32(true), m_intValue(num) {}
		RSNumber( RSClass * pClass, double num, BasicType type = typeNumber )
			: RSObject(pClass,type), m_flagInt32(false), m_fpValue(num) {}
		// 整数値設定
		void SetInteger( int32_t num ) ;
		// 実数値設定
		void SetNumber( double num ) ;

	public:	// 型情報
		// 型名
		virtual const wchar_t * GetTypeName( void ) const ;
		// 型テスト
		virtual RSObject * InstanceOf( const wchar_t * pwszType ) ;
		// 整数型か？
		virtual bool IsIntegerType( void ) const ;
		// 浮動小数点型か？
		virtual bool IsFloatType( void ) const ;
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
		// 値設定
		virtual SSystem::SError SetIntegerAs( int64_t nValue ) ;
		virtual SSystem::SError SetNumberAs( double nValue ) ;
		virtual SSystem::SError SetStringAs( const wchar_t * pwszValue ) ;

	public:	// オブジェクト
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

	public:
		// シリアライズ
		virtual RSObject * SerializeObject( RSContext& context ) ;
		virtual SSystem::SError SerializeBinary
				( RSContext& context, SSystem::SFileInterface& file ) ;
		virtual SSystem::SError MakeXMLDocument
				( RSContext& context, SSystem::SXMLDocument& xmlDoc ) ;
		// 復元
		virtual SSystem::SError RestoreObject( RSContext& context, RSObject * pObj ) ;
		virtual SSystem::SError RestoreBinary
				( RSContext& context, SSystem::SFileInterface& file ) ;
		virtual SSystem::SError RestoreXMLDocument
				( RSContext& context, const SSystem::SXMLDocument& xmlDoc ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 数値型
	//////////////////////////////////////////////////////////////////////////

	class	RSNumberClass	: public RSClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSNumberClass, RSClass )
		// 構築関数
		RSNumberClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"Number" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;

	public:
		// インスタンス生成
		virtual RSObject * NewInstance( RSContext& context, RSObject * pArg ) ;
		// 変数インスタンス生成
		virtual RSObject * NewVariable( RSContext& context ) ;
		// キャスト処理
		virtual bool TestCastInstance
			( RSObject * pObj, CastMethod castMethod = castNatural ) ;
		virtual RSObject * CastInstance
			( RSContext& context, RSObject * pObj, CastMethod castMethod ) ;

	protected:	// Number method
		// static long doubleToLongBits( double value )
		static RSObject * method_doubleToLongBits
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static int floatToIntBits( float value )
		static RSObject * method_floatToIntBits
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static double longBitsToDouble( long bits )
		static RSObject * method_longBitsToDouble
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static float intBitsToFloat( int bits )
		static RSObject * method_intBitsToFloat
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 整数値
	//////////////////////////////////////////////////////////////////////////

	class	RSInteger	: public RSObject
	{
	public:
		int64_t	m_intValue ;

		enum	IntegerType
		{
			typeBoolean,
			typeInt8,
			typeUint8,
			typeInt16,
			typeUint16,
			typeInt32,
			typeUint32,
			typeInt64,
			typeCount,
		} ;
		IntegerType	m_intType ;

		struct	TypeParam
		{
			int64_t	maskBits ;
			int64_t	maskSign ;
			size_t	sizeBits ;
		} ;
		static const TypeParam	m_typeParam[typeCount] ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSInteger, RSObject )
		// 構築関数
		RSInteger
			( RSClass * pClass,
				int64_t numInt, IntegerType intType = typeInt64 ) ;
		// 数値設定
		void SetInteger( int64_t num ) ;
		// 数値型設定
		void SetIntegerType( IntegerType intType ) ;
		// 型の符号有無
		bool IsSign( void ) const ;
		// 型のビットサイズ
		size_t BitSizeOf( void ) const ;

	public:	// 型情報
		// 型名
		virtual const wchar_t * GetTypeName( void ) const ;
		// 型テスト
		virtual RSObject * InstanceOf( const wchar_t * pwszType ) ;
		// 整数型か？
		virtual bool IsIntegerType( void ) const ;
		// 浮動小数点型か？
		virtual bool IsFloatType( void ) const ;
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
		// 値設定
		virtual SSystem::SError SetIntegerAs( int64_t nValue ) ;
		virtual SSystem::SError SetNumberAs( double nValue ) ;
		virtual SSystem::SError SetStringAs( const wchar_t * pwszValue ) ;

	public:	// オブジェクト
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

	public:
		// シリアライズ
		virtual RSObject * SerializeObject( RSContext& context ) ;
		virtual SSystem::SError SerializeBinary
				( RSContext& context, SSystem::SFileInterface& file ) ;
		virtual SSystem::SError MakeXMLDocument
				( RSContext& context, SSystem::SXMLDocument& xmlDoc ) ;
		// 復元
		virtual SSystem::SError RestoreObject( RSContext& context, RSObject * pObj ) ;
		virtual SSystem::SError RestoreBinary
				( RSContext& context, SSystem::SFileInterface& file ) ;
		virtual SSystem::SError RestoreXMLDocument
				( RSContext& context, const SSystem::SXMLDocument& xmlDoc ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 整数型
	//////////////////////////////////////////////////////////////////////////

	class	RSIntegerClass	: public RSClass
	{
	public:
		RSInteger::IntegerType	m_intType ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSIntegerClass, RSClass )
		// 構築関数
		RSIntegerClass
			( RSClass * pClass,
				const wchar_t * pwszClassName,
				RSInteger::IntegerType intType ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;

	public:
		// インスタンス生成
		virtual RSObject * NewInstance( RSContext& context, RSObject * pArg ) ;
		// 変数インスタンス生成
		virtual RSObject * NewVariable( RSContext& context ) ;
		// キャスト処理
		virtual bool TestCastInstance
			( RSObject * pObj, CastMethod castMethod = castNatural ) ;
		virtual RSObject * CastInstance
			( RSContext& context, RSObject * pObj, CastMethod castMethod ) ;

	protected:	// Integer method
		// static double longBitsToDouble( long bits )
		static RSObject * method_longBitsToDouble
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// static float intBitsToFloat( int bits )
		static RSObject * method_intBitsToFloat
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// ブール値
	//////////////////////////////////////////////////////////////////////////

	class	RSBoolean	: public RSNumber
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSBoolean, RSNumber )
		// 構築関数
		RSBoolean( RSClass * pClass, bool boolValue )
			: RSNumber( pClass, (int32_t) boolValue, typeBoolean ) {}

	public:
		// 型名
		virtual const wchar_t * GetTypeName( void ) const ;
		// 型テスト
		virtual RSObject * InstanceOf( const wchar_t * pwszType ) ;
		// 整数値取得
		virtual bool AsInteger( int64_t& number ) const ;
		// 実数値取得
		virtual bool AsRealNumber( double& number ) const ;
		// 文字列変換
		virtual bool AsString( SSystem::SString& strValue ) const ;
		// 同定判定
		virtual bool IsEqualObject( RSObject * pObj ) const ;
		// 複製（実体も可能な限り複製）
		virtual RSObject * CloneObject( RSContext& context ) const ;

	public:
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
	// ブール型
	//////////////////////////////////////////////////////////////////////////

	class	RSBooleanClass	: public RSClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSBooleanClass, RSClass )
		// 構築関数
		RSBooleanClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"boolean" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;

	public:
		// インスタンス生成
		virtual RSObject * NewInstance( RSContext& context, RSObject * pArg ) ;
		// 変数インスタンス生成
		virtual RSObject * NewVariable( RSContext& context ) ;
		// キャスト処理
		virtual bool TestCastInstance
			( RSObject * pObj, CastMethod castMethod = castNatural ) ;
		virtual RSObject * CastInstance
			( RSContext& context, RSObject * pObj, CastMethod castMethod ) ;
	} ;


}

#endif
