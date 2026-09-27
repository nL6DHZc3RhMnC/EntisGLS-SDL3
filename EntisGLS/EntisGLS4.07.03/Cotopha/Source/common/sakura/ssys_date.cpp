
/*****************************************************************************
                          Sakura2 Library
 ****************************************************************************/

#include <sakura/sakura.h>

using namespace SSystem ;


//////////////////////////////////////////////////////////////////////////
// ライブラリバージョン
//////////////////////////////////////////////////////////////////////////

ESL_DLL_DECL( const uint32_t	SSystem::entisgls4_version = (entisglsVersionMajor << 16) | entisglsVersionMinor ) ;
ESL_DLL_DECL( const char *		SSystem::entisgls4_version_name = "EntisGLS 4.07" ) ;



//////////////////////////////////////////////////////////////////////////
// システム（タイマ・時刻）
//////////////////////////////////////////////////////////////////////////

// 比較
//////////////////////////////////////////////////////////////////////////
int DATE_TIME::Compare( const DATE_TIME& date ) const
{
	int	c = (int) nYear - (int) date.nYear ;
	if ( c == 0 )
	{
		c = (int) nMonth - (int) date.nMonth ;
		if ( c == 0 )
		{
			c = (int) nDay - (int) date.nDay ;
			if ( c == 0 )
			{
				c = (int) nHour - (int) date.nHour ;
				if ( c == 0 )
				{
					c = (int) nMinute - (int) date.nMinute ;
					if ( c == 0 )
					{
						c = (int) nSecond - (int) date.nSecond ;
						if ( c == 0 )
						{
							c = (int) nMilliSec - (int) date.nMilliSec ;
						}
					}
				}
			}
		}
	}
	return	c ;
}

// 累積日数変換（紀元元年～）
//////////////////////////////////////////////////////////////////////////
uint64_t DATE_TIME::GetAccumulatedDayCount
				( int nYear, int nMonth, int nDay )
{
	uint64_t	countDay = (nYear - 1) * 365 ;
	countDay += ((nYear - 1) / 4)
					- ((nYear - 1) / 100) + ((nYear - 1) / 400) ;
	for ( int m = 1; m < nMonth; m ++ )
	{
		countDay += GetDayOfMonth( nYear, m ) ;
	}
	return	countDay + nDay - 1 ;
}

void DATE_TIME::SetAccumulatedDayCount( uint64_t countDay )
{
	const uint32_t	dayOf400Years = 365 * 400 + 97 ;
	const uint32_t	dayOf100Years = 365 * 100 + 24 ;
	const uint32_t	dayOf4Years = 365 * 4 + 1 ;
	const uint32_t	dayOf1Year = 365 ;
	const uint64_t	nTotalDay = countDay ;
	uint32_t	year400 = (uint32_t) (countDay / dayOf400Years) ;
	countDay %= dayOf400Years ;
	uint32_t	year100 = (uint32_t) (countDay / dayOf100Years) ;
	countDay %= dayOf100Years ;
	uint32_t	year4 = (uint32_t) (countDay / dayOf4Years) ;
	countDay %= dayOf4Years ;
	uint32_t	year1 = (uint32_t) (countDay / dayOf1Year) ;
	countDay %= dayOf1Year ;

	nYear = (uint16_t)
				(year400 * 400 + year100 * 100
							+ year4 * 4 + year1 + 1) ;
	if ( (countDay == 0) && (nTotalDay + 1 == GetAccumulatedDayCount( nYear, 1, 1 )) )
	{
		nYear -- ;
		countDay += dayOf1Year ;
	}
	nMonth = 0 ;
	for ( int m = 1; m <= 12; m ++ )
	{
		unsigned int	nDayOfMonth = GetDayOfMonth( nYear, m ) ;
		if ( countDay < nDayOfMonth )
		{
			nMonth = (uint16_t) m ;
			break ;
		}
		countDay -= nDayOfMonth ;
	}
	nDay = (uint16_t) (countDay + 1) ;
	nWeek = (uint16_t) ComputeDayOfWeek() ;
}

// 累積ミリ秒数（紀元元年～）
//////////////////////////////////////////////////////////////////////////
uint64_t DATE_TIME::GetAccumulatedMilliSec( void ) const
{
	uint64_t	nDays = GetAccumulatedDayCount() ;
	return	nDays * (24 * 60 * 60 * 1000)
				+ nHour * (60 * 60 * 1000)
				+ nMinute * (60 * 1000)
				+ nSecond * 1000 + nMilliSec ;
}

void DATE_TIME::SetAccumulatedMilliSec( uint64_t nAccMilliSec )
{
	uint64_t	countDay = nAccMilliSec / (24 * 60 * 60 * 1000) ;
	uint32_t	nTemp = (uint32_t) (nAccMilliSec % (24 * 60 * 60 * 1000)) ;
	nMilliSec = (uint16_t) (nTemp % 1000) ;
	nTemp /= 1000 ;
	nSecond = nTemp % 60 ;
	nTemp /= 60 ;
	nMinute = nTemp % 60 ;
	nHour = nTemp / 60 ;
	//
	SetAccumulatedDayCount( countDay ) ;
}

// 月の日数取得
//////////////////////////////////////////////////////////////////////////
unsigned int DATE_TIME::GetDayOfMonth( int nYear, int nMonth )
{
	static const unsigned int	countDayOfMonth[12] =
	{
		31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31,
	} ;
	if ( (nMonth < 1) | (nMonth > 12) )
	{
		return	0 ;
	}
	if ( nMonth != 2 )
	{
		return	countDayOfMonth[nMonth - 1] ;
	}
	if ( IsLeapYear( nYear ) )
	{
		return	29 ;
	}
	return	28 ;
}

// 曜日計算
//////////////////////////////////////////////////////////////////////////
DayOfWeek DATE_TIME::ComputeDayOfWeek( int nYear, int nMonth, int nDay )
{
	return	(DayOfWeek)
		((GetAccumulatedDayCount( nYear, nMonth, nDay ) + 1) % 7) ;
}



//////////////////////////////////////////////////////////////////////////
// パフォーマンスカウンタ
//////////////////////////////////////////////////////////////////////////

bool			STimeCounter::m_flagVirtual = false ;
atomic_int_t	STimeCounter::m_timeVirtual = 0 ;

// 構築関数
//////////////////////////////////////////////////////////////////////////
STimeCounter::STimeCounter( void )
{
	m_timeFreq = GetPerformanceFrequency() ;
	m_flagPerformance = (m_timeFreq != 0) ;
	//
	Reset() ;
}

// 現在の経過時間
//////////////////////////////////////////////////////////////////////////
int64_t STimeCounter::GetTime( void ) const
{
	if ( m_flagFreeze )
	{
		return	eslRoundR64ToLInt( m_timeFreeze ) ;
	}
	else if ( m_flagVirtual )
	{
		return	m_timeVirtual - m_nVirtStart ;
	}
	else if ( m_flagPerformance )
	{
		int64_t	timeCurrent = GetPerformanceCounter() ;
		return	(timeCurrent - m_timeStart) * 1000 / m_timeFreq ;
	}
	else
	{
		return	CurrentMilliSec() - m_timeStart ;
	}
}

// 現在の経過時間（実数）
//////////////////////////////////////////////////////////////////////////
double STimeCounter::GetRealTime( void ) const
{
	if ( m_flagFreeze )
	{
		return	m_timeFreeze ;
	}
	else if ( m_flagVirtual )
	{
		return	(double) (m_timeVirtual - m_nVirtStart) ;
	}
	else if ( m_flagPerformance )
	{
		int64_t	timeCurrent = GetPerformanceCounter() ;
		return	(double) (timeCurrent - m_timeStart) * 1000.0 / m_timeFreq ;
	}
	else
	{
		return	(double) (CurrentMilliSec() - m_timeStart) ;
	}
}

// 基準時間リセット
//////////////////////////////////////////////////////////////////////////
void STimeCounter::Reset( int64_t timeInit )
{
	m_timeStart = GetPerformanceCounter() ;
	m_timeFreeze = (double) timeInit ;
	if ( !m_flagPerformance )
	{
		m_timeStart = CurrentMilliSec() - timeInit ;
	}
	else if ( timeInit != 0 )
	{
		m_timeStart -= m_timeFreq * timeInit / 1000 ;
	}
	m_flagFreeze = false ;
	m_nVirtStart = m_timeVirtual - (atomic_int_t) timeInit ;
}

// 一時停止
//////////////////////////////////////////////////////////////////////////
void STimeCounter::Freeze( void )
{
	if ( !m_flagFreeze )
	{
		m_timeFreeze = GetRealTime() ;
		m_flagFreeze = true ;
	}
}

// 再開
//////////////////////////////////////////////////////////////////////////
void STimeCounter::Restart( void )
{
	if ( m_flagFreeze )
	{
		m_timeStart = GetPerformanceCounter() ;
		if ( !m_flagPerformance )
		{
			m_timeStart =
				CurrentMilliSec() - eslRoundR64ToLInt( m_timeFreeze ) ;
		}
		else
		{
			m_timeStart -=
				eslRoundR64ToLInt
					( (double) m_timeFreq * m_timeFreeze / 1000 ) ;
		}
		m_flagFreeze = false ;
		m_nVirtStart =
			m_timeVirtual
				- (atomic_int_t) eslRoundR64ToLInt( m_timeFreeze ) ;
	}
}

// 仮想タイマーモード設定
//////////////////////////////////////////////////////////////////////////
void STimeCounter::SetVirtualTimerMode( bool flagVirtual )
{
	m_flagVirtual = flagVirtual ;
}

// 仮想タイマーを進める
//////////////////////////////////////////////////////////////////////////
void STimeCounter::AdvanceVirtualTimer( atomic_int_t msecAdv )
{
	AtomicAdd( &m_timeVirtual, msecAdv ) ;
}

