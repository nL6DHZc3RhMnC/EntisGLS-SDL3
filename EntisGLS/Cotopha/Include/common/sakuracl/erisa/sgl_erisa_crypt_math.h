
/*****************************************************************************
                         E R I S A - L i b r a r y
 -----------------------------------------------------------------------------
    Copyright (C) 2013-2016 Leshade Entis, Entis-soft. All rights reserved.
 *****************************************************************************/


#if	!defined(__SAKURA_ERISA_CRYPT_MATH_H__)
#define	__SAKURA_ERISA_CRYPT_MATH_H__

namespace	SakuraCL
{
	//////////////////////////////////////////////////////////////////////////
	// 算術関数
	//////////////////////////////////////////////////////////////////////////

	// 最大公約数を求める（ユークリッド互除法）
	template <class T> T ComputeGCD( const T a, const T b )
	{
		T	p = a ;
		T	q = b ;
		T	w ;
		while ( q > 0 )
		{
			w = p % q ;
			p = q ;
			q = w ;
		}
		return	p ;
	}

	// 最大公約数を求める（拡張ユークリッド互除法）
	template <class T> T ComputeExGCD( T& x, T& y, const T a, const T b )
	{
		T	x1 = 0, x2 ;
		T	y1 = 1, y2 ;
		T	r, s ;
		T	p = a ;
		T	q = b ;
		x = 1 ;
		y = 0 ;
		while ( q > 0 )
		{
			r = p / q ;
			s = p % q ;
			x2 = x - r * x1 ;
			y2 = y - r * y1 ;
			p = q ;
			q = s ;
			x = x1 ;
			x1 = x2 ;
			y = y1 ;
			y1 = y2 ;
		}
		return	p ;
	}

	// 最小公倍数を求める
	template <class T> T ComputeLCM( const T a, const T b )
	{
		return	a * b / ComputeGCD<T>( a, b ) ;
	}

	// n を法とする逆元を求める
	template <class T> T ComputeInverseElement( const T a, const T n )
	{
		T	x, y ;
		T	gcd = ComputeExGCD<T>( x, y, a, n ) ;
		if ( gcd != 1 )
		{
			return	0 ;	// 逆元無し
		}
		return	x ;
	}

	// n を法する累乗関数（引数の積が T の有効範囲内にある事）
	template <class T> T ComputePower( const T x, const T y, const T n )
	{
		T	a = x % n ;
		T	b = y % n ;
		T	r = 1 ;
		while ( b != 0 )
		{
			if ( b & 0x01 )
			{
				r = (r * a) % n ;
			}
			a = (a * a) % n ;
			b >>= 1 ;
		}
		return	r ;
	}


	//////////////////////////////////////////////////////////////////////////
	// 乱数
	//////////////////////////////////////////////////////////////////////////

	class	SCLRandomizer
	{
	protected:
		uint32_t	m_numRandom ;
		uint32_t	m_countSalt ;

	public:
		// クラス情報
		ESL_DECLARE_NV_CLASS_INFO( SCLRandomizer )
		// 構築関数
		SCLRandomizer( void ) ;
		SCLRandomizer( uint32_t numInit ) ;
		// 乱数の種初期化
		void InitializeSeed( void ) ;
		void InitializeSeedBy( uint32_t numInit ) ;
		// 乱数の種取得
		uint32_t SaveRandomSeed( void ) const ;
		// 乱数の種復元
		void RestoreRandomSeed( uint32_t numSeed ) ;
		// 乱数生成 [0,x) or [0,0xFFFFFFFF]
		uint32_t Randomize( uint32_t numLimit = 0 ) ;
		uint32_t QuickRandomize( uint32_t numLimit = 0 ) ;
		// 乱数生成 [0.0,x)
		float32_t QuickRandomFloat( float32_t fpRange ) ;
		// 乱数生成 [-x,x)
		double QuickRandomDouble( double fpRange ) ;

	} ;


	// 素数判定 Miller-Rabin 法
	//////////////////////////////////////////////////////////////////////////
	template <class T> bool IsPrimality( const T n )
	{
		ESLAssert( n > 0 ) ;
		if ( n == 2 )
		{
			return	true ;
		}
		if ( (n == 1) || ((n & 0x01) == 0) )
		{
			return	false ;
		}
		T	d = n - 1 ;
		while ( (d & 0x01) == 0 )
		{
			d >>= 1 ;
		}
		static const int	a[] =
		{
			 2,  3,  5,  7, 11, 13, 17, 19, 23, 29,
			31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 0
		} ;
		for ( size_t i = 0; a[i] && (n > a[i]); i ++ )
		{
			T	t = d ;
			T	y = ComputePower<T>( a[i], t, n ) ;
			while ( (t != n - 1) && (y != 1) && (y != n - 1) )
			{
				y = (y * y) % n ;
				t <<= 1 ;
			}
			if ( (y != n - 1) && ((t & 0x01) == 0) )
			{
				return	false ;
			}
		}
		return	true ;
	}

}

#endif
