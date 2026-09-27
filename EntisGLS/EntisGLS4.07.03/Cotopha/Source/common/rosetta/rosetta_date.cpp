
#include <rosetta/rosetta.h>
#include <rosetta/rosetta_date.h>

using namespace	SSystem ;
using namespace	Rosetta ;


//////////////////////////////////////////////////////////////////////////////
// Date 型オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSDate, RSObject )

// ミリ秒日時設定 (1970年1月1日～)
//////////////////////////////////////////////////////////////////////////////
void RSDate::SetMilliSecTime( int64_t msec )
{
	msec -= DifferenceInLocalTime() * 1000 ;
	//
	int64_t	msecTime = msec % (24 * 60 * 60 * 1000) ;
	int64_t	countDay = (msec - msecTime) / (24 * 60 * 60 * 1000) ;
	m_date.SetAccumulatedDayCount
		( countDay + DATE_TIME::GetAccumulatedDayCount( 1970, 1, 1 ) ) ;
	m_date.nMilliSec = (uint16_t) (msecTime % 1000) ;
	//
	int	secTime = (int) (msecTime - m_date.nMilliSec) / 1000 ;
	m_date.nSecond = (uint16_t) (secTime % 60) ;
	//
	int	minTime = (secTime - m_date.nSecond) / 60 ;
	m_date.nMinute = (uint16_t) (minTime % 60) ;
	m_date.nHour = (uint16_t) ((minTime - m_date.nMinute) / 60) ;
}

// 整数型か？
//////////////////////////////////////////////////////////////////////////////
bool RSDate::IsIntegerType( void ) const
{
	return	true ;
}

// オブジェクト型か？
//////////////////////////////////////////////////////////////////////////////
bool RSDate::IsObjectType( void ) const
{
	return	true ;
}

// 整数値取得
//////////////////////////////////////////////////////////////////////////////
bool RSDate::AsInteger( int64_t& number ) const
{
	int64_t	days = m_date.GetAccumulatedDayCount()
					- DATE_TIME::GetAccumulatedDayCount( 1970, 1, 1 ) ;
	number = days * (24 * 60 * 60 * 1000)
			+ m_date.nHour * (60 * 60 * 1000)
			+ m_date.nMinute * (60 * 1000)
			+ m_date.nSecond * 1000 + m_date.nMilliSec
			+ DifferenceInLocalTime() * 1000 ;
	return	true ;
}

// 文字列変換
//////////////////////////////////////////////////////////////////////////////
bool RSDate::AsString( SSystem::SString& strValue ) const
{
	strValue.Format
		( L"%04d/%02d/%02d %02d:%02d:%02d.%03d",
			m_date.nYear, m_date.nMonth, m_date.nDay,
			m_date.nHour, m_date.nMinute, m_date.nSecond, m_date.nMilliSec ) ;
	return	true ;
}

// 同定判定
//////////////////////////////////////////////////////////////////////////////
bool RSDate::IsEqualObject( RSObject * pObj ) const
{
	if ( pObj == NULL )
	{
		return	false ;
	}
	RSDate *	pDate = ESLTypeCast<RSDate>( pObj->GetEntityObject() ) ;
	if ( pDate == NULL )
	{
		return	false ;
	}
	return	(m_date == pDate->m_date) ;
}

// 複製（実体も可能な限り複製）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDate::CloneObject( RSContext& context ) const
{
	return	new RSDate( GetRSClass(), m_date ) ;
}

// 二項演算子
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDate::OperatorCompareEQ( RSContext& context, RSObject * pObj ) const
{
	RSDate *	pDate = ESLTypeCast<RSDate>( pObj->GetEntityObject() ) ;
	if ( pDate != NULL )
	{
		return	context.new_Boolean( m_date == pDate->m_date ) ;
	}
	return	context.new_Boolean( false ) ;
}

RSObject * RSDate::OperatorCompareNE( RSContext& context, RSObject * pObj ) const
{
	RSDate *	pDate = ESLTypeCast<RSDate>( pObj->GetEntityObject() ) ;
	if ( pDate != NULL )
	{
		return	context.new_Boolean( m_date != pDate->m_date ) ;
	}
	return	context.new_Boolean( false ) ;
}

RSObject * RSDate::OperatorCompareGE( RSContext& context, RSObject * pObj ) const
{
	RSDate *	pDate = ESLTypeCast<RSDate>( pObj->GetEntityObject() ) ;
	if ( pDate != NULL )
	{
		return	context.new_Boolean( m_date >= pDate->m_date ) ;
	}
	return	context.new_Boolean( false ) ;
}

RSObject * RSDate::OperatorCompareGT( RSContext& context, RSObject * pObj ) const
{
	RSDate *	pDate = ESLTypeCast<RSDate>( pObj->GetEntityObject() ) ;
	if ( pDate != NULL )
	{
		return	context.new_Boolean( m_date > pDate->m_date ) ;
	}
	return	context.new_Boolean( false ) ;
}

RSObject * RSDate::OperatorCompareLE( RSContext& context, RSObject * pObj ) const
{
	RSDate *	pDate = ESLTypeCast<RSDate>( pObj->GetEntityObject() ) ;
	if ( pDate != NULL )
	{
		return	context.new_Boolean( m_date <= pDate->m_date ) ;
	}
	return	context.new_Boolean( false ) ;
}

RSObject * RSDate::OperatorCompareLT( RSContext& context, RSObject * pObj ) const
{
	RSDate *	pDate = ESLTypeCast<RSDate>( pObj->GetEntityObject() ) ;
	if ( pDate != NULL )
	{
		return	context.new_Boolean( m_date < pDate->m_date ) ;
	}
	return	context.new_Boolean( false ) ;
}

// 代入演算子
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDate::OperatorMove( RSContext& context, RSObject * pObj )
{
	RSDate *	pDate = ESLTypeCast<RSDate>( pObj->GetEntityObject() ) ;
	if ( pDate != NULL )
	{
		m_date = pDate->m_date ;
		AddRef() ;
		return	this ;
	}
	else
	{
		int64_t	num ;
		if ( pObj->AsInteger( num ) )
		{
			SetMilliSecTime( num ) ;
			AddRef() ;
			return	this ;
		}
	}
	context.ThrowExceptionError( L"Date へ代入できません" ) ;
	return	NULL ;
}


//////////////////////////////////////////////////////////////////////////////
// Date 型クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSDateClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSDateClass::RSDateClass( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSDateClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSDate( this ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"",
					NULL, &RSDateClass::method_init0, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"long dateVal",
					NULL, &RSDateClass::method_init1, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"compareTo", L"int", L"Date date",
					NULL, &RSDateClass::method_compareTo,
					NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getDate", L"int", L"",
					NULL, &RSDateClass::method_getDate,
					NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getDay", L"int", L"",
					NULL, &RSDateClass::method_getDay,
					NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getHours", L"int", L"",
					NULL, &RSDateClass::method_getHours,
					NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getMinutes", L"int", L"",
					NULL, &RSDateClass::method_getMinutes,
					NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getMonth", L"int", L"",
					NULL, &RSDateClass::method_getMonth,
					NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getSeconds", L"int", L"",
					NULL, &RSDateClass::method_getSeconds,
					NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getYear", L"int", L"",
					NULL, &RSDateClass::method_getYear,
					NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getFullYear", L"int", L"",
					NULL, &RSDateClass::method_getFullYear,
					NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getTime", L"long", L"",
					NULL, &RSDateClass::method_getTime,
					NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getTimezoneOffset", L"int", L"",
					NULL, &RSDateClass::method_getTimezoneOffset,
					NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setDate", NULL, L"int date",
					NULL, &RSDateClass::method_setDate, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setHours", NULL, L"int hours",
					NULL, &RSDateClass::method_setHours, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setMinutes", NULL, L"int minutes",
					NULL, &RSDateClass::method_setMinutes, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setMonth", NULL, L"int month",
					NULL, &RSDateClass::method_setMonth, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setSeconds", NULL, L"int seconds",
					NULL, &RSDateClass::method_setSeconds, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setYear", NULL, L"int year",
					NULL, &RSDateClass::method_setYear, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setFullYear", NULL, L"int year",
					NULL, &RSDateClass::method_setFullYear, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setTime", NULL, L"long time",
					NULL, &RSDateClass::method_setTime, NULL ) ;
}

// this オブジェクトのファイルを取得
//////////////////////////////////////////////////////////////////////////////
RSDate * RSDateClass::GetThisDate( RSContext& context, RSObject* pThis )
{
	RSDate *	pDate = ESLTypeCast<RSDate>( pThis ) ;
	if ( pDate == NULL )
	{
		context.ThrowExceptionError( L"this が Date ではありません" ) ;
	}
	return	pDate ;
}

// void <init>( void )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDateClass::method_init0
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSDate *	pDate = GetThisDate( context, pThis ) ;
	if ( pDate == NULL )
	{
		return	NULL ;
	}
	CurrentLocalDate( pDate->m_date ) ;
	return	NULL ;
}

// void <init>( long dateVal )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDateClass::method_init1
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSDate *	pDate = GetThisDate( context, pThis ) ;
	if ( pDate == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pDate->SetMilliSecTime( arg.LongAt( 0 ) ) ;
	return	NULL ;
}

// int compareTo( Date date )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDateClass::method_compareTo
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSDate *	pDate = GetThisDate( context, pThis ) ;
	if ( pDate == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSDate *	pSrcDate = ESLTypeCast<RSDate>( arg.ObjectAt( 0 ) ) ;
	if ( pSrcDate == NULL )
	{
		context.ThrowExceptionError
			( L"Date.compareTo の引数が Date ではありません" ) ;
		return	NULL ;
	}
	return	context.new_Integer
				( pDate->m_date.Compare( pSrcDate->m_date ) ) ;
}

// int getDate()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDateClass::method_getDate
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSDate *	pDate = GetThisDate( context, pThis ) ;
	if ( pDate == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( pDate->m_date.nDay ) ;
}

// int getDay()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDateClass::method_getDay
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSDate *	pDate = GetThisDate( context, pThis ) ;
	if ( pDate == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( pDate->m_date.nWeek ) ;
}

// int getHours()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDateClass::method_getHours
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSDate *	pDate = GetThisDate( context, pThis ) ;
	if ( pDate == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( pDate->m_date.nHour ) ;
}

// int getMinutes()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDateClass::method_getMinutes
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSDate *	pDate = GetThisDate( context, pThis ) ;
	if ( pDate == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( pDate->m_date.nMinute ) ;
}

// int getMonth()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDateClass::method_getMonth
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSDate *	pDate = GetThisDate( context, pThis ) ;
	if ( pDate == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( pDate->m_date.nMonth - 1 ) ;
}

// int getSeconds()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDateClass::method_getSeconds
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSDate *	pDate = GetThisDate( context, pThis ) ;
	if ( pDate == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( pDate->m_date.nSecond ) ;
}

// int getYear()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDateClass::method_getYear
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSDate *	pDate = GetThisDate( context, pThis ) ;
	if ( pDate == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( pDate->m_date.nYear - 1900 ) ;
}

// int getFullYear()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDateClass::method_getFullYear
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSDate *	pDate = GetThisDate( context, pThis ) ;
	if ( pDate == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( pDate->m_date.nYear ) ;
}

// long getTime()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDateClass::method_getTime
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSDate *	pDate = GetThisDate( context, pThis ) ;
	if ( pDate == NULL )
	{
		return	NULL ;
	}
	int64_t	num ;
	pDate->AsInteger( num ) ;
	return	context.new_Integer( num ) ;
}

// int getTimezoneOffset()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDateClass::method_getTimezoneOffset
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	return	context.new_Integer( SSystem::DifferenceInLocalTime() / 60 ) ;
}

// void setDate( int date )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDateClass::method_setDate
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSDate *	pDate = GetThisDate( context, pThis ) ;
	if ( pDate == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pDate->m_date.SetAccumulatedDayCount
		( DATE_TIME::GetAccumulatedDayCount
			( pDate->m_date.nYear, pDate->m_date.nMonth, arg.IntAt( 0 ) ) ) ;
	return	NULL ;
}

// void setHours( int hours )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDateClass::method_setHours
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSDate *	pDate = GetThisDate( context, pThis ) ;
	if ( pDate == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pDate->m_date.nHour = (uint16_t) arg.IntAt( 0 ) ;
	return	NULL ;
}

// void setMinutes( int minutes )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDateClass::method_setMinutes
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSDate *	pDate = GetThisDate( context, pThis ) ;
	if ( pDate == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pDate->m_date.nMinute = (uint16_t) arg.IntAt( 0 ) ;
	return	NULL ;
}

// void setMonth( int month )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDateClass::method_setMonth
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSDate *	pDate = GetThisDate( context, pThis ) ;
	if ( pDate == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pDate->m_date.SetAccumulatedDayCount
		( DATE_TIME::GetAccumulatedDayCount
			( pDate->m_date.nYear, arg.IntAt( 0 ), pDate->m_date.nDay ) ) ;
	return	NULL ;
}

// void setSeconds( int seconds )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDateClass::method_setSeconds
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSDate *	pDate = GetThisDate( context, pThis ) ;
	if ( pDate == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pDate->m_date.nSecond = (uint16_t) arg.IntAt( 0 ) ;
	return	NULL ;
}

// void setYear( int year )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDateClass::method_setYear
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSDate *	pDate = GetThisDate( context, pThis ) ;
	if ( pDate == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pDate->m_date.SetAccumulatedDayCount
		( DATE_TIME::GetAccumulatedDayCount
			( arg.IntAt( 0 ) + 1900, pDate->m_date.nMonth, pDate->m_date.nDay ) ) ;
	return	NULL ;
}

// void setFullYear( int year )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDateClass::method_setFullYear
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSDate *	pDate = GetThisDate( context, pThis ) ;
	if ( pDate == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pDate->m_date.SetAccumulatedDayCount
		( DATE_TIME::GetAccumulatedDayCount
			( arg.IntAt( 0 ), pDate->m_date.nMonth, pDate->m_date.nDay ) ) ;
	return	NULL ;
}

// void setTime( long time )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDateClass::method_setTime
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSDate *	pDate = GetThisDate( context, pThis ) ;
	if ( pDate == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pDate->SetMilliSecTime( arg.LongAt( 0 ) ) ;
	return	NULL ;
}


