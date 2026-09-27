
#if	!defined(__ROSETTA_DATE_H__)
#define	__ROSETTA_DATE_H__

namespace	Rosetta
{
	//////////////////////////////////////////////////////////////////////////
	// Date 型オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	RSDate	: public RSObject
	{
	public:
		SSystem::DATE_TIME	m_date ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSDate, RSObject )
		// 構築関数
		RSDate( RSClass * pClass )
			: RSObject( pClass, typeOther )
		{
			SSystem::CurrentLocalDate( m_date ) ;
		}
		RSDate( RSClass * pClass, const SSystem::DATE_TIME& date )
			: RSObject( pClass, typeOther )
		{
			m_date = date ;
		}
		// ミリ秒日時設定 (1970年1月1日～)
		void SetMilliSecTime( int64_t msec ) ;

	public:	// 型情報
		// 整数型か？
		virtual bool IsIntegerType( void ) const ;
		// オブジェクト型か？
		virtual bool IsObjectType( void ) const ;
		// 整数値取得
		virtual bool AsInteger( int64_t& number ) const ;
		// 文字列変換
		virtual bool AsString( SSystem::SString& strValue ) const ;
		// 同定判定
		virtual bool IsEqualObject( RSObject * pObj ) const ;

	public:	// オブジェクト
		// 複製（実体も可能な限り複製）
		virtual RSObject * CloneObject( RSContext& context ) const ;

	public:
		// 二項演算子
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
	// Date 型クラス
	//////////////////////////////////////////////////////////////////////////

	class	RSDateClass	: public RSClass
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSDateClass, RSClass )
		// 構築関数
		RSDateClass
			( RSClass * pClass, const wchar_t * pwszClassName = L"Date" ) ;
		// クラス固有仮想関数オーバーライドと
		// クラス static 変数のオーバーロード
		virtual void OverrideVirtuals( RSContext& context ) ;

	protected:
		// this オブジェクトのファイルを取得
		static RSDate * GetThisDate( RSContext& context, RSObject* pThis ) ;

	public:	// RandomAccessFile method
		// void <init>( void )
		static RSObject * method_init0
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void <init>( long dateVal )
		static RSObject * method_init1
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int compareTo( Date date )
		static RSObject * method_compareTo
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int getDate()
		static RSObject * method_getDate
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int getDay()
		static RSObject * method_getDay
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int getHours()
		static RSObject * method_getHours
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int getMinutes()
		static RSObject * method_getMinutes
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int getMonth()
		static RSObject * method_getMonth
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int getSeconds()
		static RSObject * method_getSeconds
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int getYear()
		static RSObject * method_getYear
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int getFullYear()
		static RSObject * method_getFullYear
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// long getTime()
		static RSObject * method_getTime
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// int getTimezoneOffset()
		static RSObject * method_getTimezoneOffset
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setDate( int date )
		static RSObject * method_setDate
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setHours( int hours )
		static RSObject * method_setHours
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setMinutes( int minutes )
		static RSObject * method_setMinutes
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setMonth( int month )
		static RSObject * method_setMonth
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setSeconds( int seconds )
		static RSObject * method_setSeconds
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setYear( int year )
		static RSObject * method_setYear
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setFullYear( int year )
		static RSObject * method_setFullYear
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;
		// void setTime( long time )
		static RSObject * method_setTime
			( RSContext& context, void * pInstace,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;

	} ;


}

#endif
