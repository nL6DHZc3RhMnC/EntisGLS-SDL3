
#if	!defined(__SAKURAGL_SGLH3D_STDDEF_H__)
#define	__SAKURAGL_SGLH3D_STDDEF_H__	1

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// 同次座標ベクトル
	//////////////////////////////////////////////////////////////////////////

	template <class T> struct SGL4DVector	: public SGL3DVector<T>
	{
		T	w ;

		SGL4DVector( void ) : w(1.0) {}
		SGL4DVector( T xInit, T yInit, T zInit, T wInit = 1.0 )
			: SGL3DVector<T>( xInit, yInit, zInit )
		{
			w = (T) wInit ;
		}
		SGL4DVector( const SGL3DVector<float32_t> & v, double wInit = 1.0 )
			: SGL3DVector<T>( (T) v.x, (T) v.y, (T) v.z )
		{
			w = (T) wInit ;
		}
		SGL4DVector( const SGL3DVector<double> & v, double wInit = 1.0 )
			: SGL3DVector<T>( (T) v.x, (T) v.y, (T) v.z )
		{
			w = (T) wInit ;
		}
		SGL4DVector( const SGL4DVector<T> & v )
			: SGL3DVector<T>( v )
		{
			w = v.w ;
		}
		SGL3DVector<T> & xyz( void )
		{
			return	*this ;
		}
		const SGL4DVector<T> & operator = ( const SGL4DVector<T> & v )
		{
			SGL3DVector<T>::x = v.x ;
			SGL3DVector<T>::y = v.y ;
			SGL3DVector<T>::z = v.z ;
			w = v.w ;
			return	*this ;
		}
		const SGL4DVector<T> & operator = ( const SGL3DVector<float32_t> & v )
		{
			SGL3DVector<T>::x = v.x ;
			SGL3DVector<T>::y = v.y ;
			SGL3DVector<T>::z = v.z ;
			w = (T) 1.0 ;
			return	*this ;
		}
		const SGL4DVector<T> & operator = ( const SGL3DVector<double> & v )
		{
			SGL3DVector<T>::x = (T) v.x ;
			SGL3DVector<T>::y = (T) v.y ;
			SGL3DVector<T>::z = (T) v.z ;
			w = (T) 1.0 ;
			return	*this ;
		}
		bool operator == ( const SGL4DVector<T> & v ) const
		{
			return	(SGL3DVector<T>::x == v.x)
						& (SGL3DVector<T>::y == v.y)
						& (SGL3DVector<T>::z == v.z) & (w == v.w) ;
		}
		bool operator != ( const SGL4DVector<T> & v ) const
		{
			return	(SGL3DVector<T>::x != v.x)
						| (SGL3DVector<T>::y != v.y)
						| (SGL3DVector<T>::z != v.z) | (w != v.w) ;
		}
		bool IsNaN( void ) const
		{
			return	SGL3DVector<T>::IsNaN() || (w != w) ;
		}
		const SGL4DVector<T> & operator += ( const SGL4DVector<T> & v )
		{
			SGL3DVector<T>::x += v.x ;
			SGL3DVector<T>::y += v.y ;
			SGL3DVector<T>::z += v.z ;
			w += v.w ;
			return	*this ;
		}
		const SGL4DVector<T> & operator -= ( const SGL4DVector<T> & v )
		{
			SGL3DVector<T>::x -= v.x ;
			SGL3DVector<T>::y -= v.y ;
			SGL3DVector<T>::z -= v.z ;
			w -= v.w ;
			return	*this ;
		}
		const SGL4DVector<T> & operator *= ( T s )
		{
			SGL3DVector<T>::x *= s ;
			SGL3DVector<T>::y *= s ;
			SGL3DVector<T>::z *= s ;
			w *= s ;
			return	*this ;
		}
		const SGL4DVector<T> & operator /= ( T s )
		{
			SGL3DVector<T>::x /= s ;
			SGL3DVector<T>::y /= s ;
			SGL3DVector<T>::z /= s ;
			w /= s ;
			return	*this ;
		}
		SGL4DVector<T> operator + ( const SGL4DVector<T> & v ) const
		{
			return	SGL4DVector<T>
				( SGL3DVector<T>::x + v.x,
					SGL3DVector<T>::y + v.y,
					SGL3DVector<T>::z + v.z, w + v.w ) ;
		}
		SGL4DVector<T> operator - ( const SGL4DVector<T> & v ) const
		{
			return	SGL4DVector<T>
				( SGL3DVector<T>::x - v.x,
					SGL3DVector<T>::y - v.y,
					SGL3DVector<T>::z - v.z, w - v.w ) ;
		}
		SGL4DVector<T> operator - ( void ) const
		{
			return	SGL4DVector<T>
				( - SGL3DVector<T>::x,
					- SGL3DVector<T>::y,
					- SGL3DVector<T>::z, - w ) ;
		}
		SGL4DVector<T> operator * ( T s ) const
		{
			return	SGL4DVector<T>
				( SGL3DVector<T>::x * s,
					SGL3DVector<T>::y * s,
					SGL3DVector<T>::z * s, w * s ) ;
		}
		SGL4DVector<T> operator / ( T s ) const
		{
			return	SGL4DVector<T>
				( SGL3DVector<T>::x / s,
					SGL3DVector<T>::y / s,
					SGL3DVector<T>::z / s, w / s ) ;
		}
	} ;

	struct	S4DVector	: public SGL4DVector<float32_t>
	{
		S4DVector( void ) {}
		S4DVector( double xInit, double yInit, double zInit, double wInit = 1.0 )
			: SGL4DVector<float32_t>
				( (float32_t) xInit, (float32_t) yInit,
							(float32_t) zInit, (float32_t) wInit ) {}
		S4DVector( const SGL3DVector<float32_t> & v, double wInit = 1.0 )
			: SGL4DVector<float32_t>( v, wInit ) {}
		S4DVector( const SGL3DVector<double> & v, double wInit = 1.0 )
			: SGL4DVector<float32_t>( v, wInit ) {}
		S4DVector( const S4DVector & v )
			: SGL4DVector<float32_t>( v.x, v.y, v.z, v.w ) {}
		S4DVector( const SGLPalette & argb ) { FromColor( argb ) ; }
		const S4DVector & operator = ( const SGL4DVector<float32_t> & v )
		{
			SGL3DVector<float32_t>::x = v.x ;
			SGL3DVector<float32_t>::y = v.y ;
			SGL3DVector<float32_t>::z = v.z ;
			w = v.w ;
			return	*this ;
		}
		const S4DVector & operator = ( const SGL3DVector<float32_t> & v )
		{
			SGL3DVector<float32_t>::x = v.x ;
			SGL3DVector<float32_t>::y = v.y ;
			SGL3DVector<float32_t>::z = v.z ;
			w = 1.0f ;
			return	*this ;
		}
		const S4DVector & operator = ( const SGL3DVector<double> & v )
		{
			SGL3DVector<float32_t>::x = (float32_t) v.x ;
			SGL3DVector<float32_t>::y = (float32_t) v.y ;
			SGL3DVector<float32_t>::z = (float32_t) v.z ;
			w = 1.0f ;
			return	*this ;
		}
		const S4DVector & operator = ( const SGLPalette & argb )
		{
			return	FromColor( argb ) ;
		}
		const S4DVector & FromColor( const SGLPalette & argb )
		{
			SGL3DVector<float32_t>::x = argb.argb.Red * (1.0f / 255.0f) ;
			SGL3DVector<float32_t>::y = argb.argb.Green * (1.0f / 255.0f) ;
			SGL3DVector<float32_t>::z = argb.argb.Blue * (1.0f / 255.0f) ;
			w = argb.argb.Alpha * (1.0f / 255.0f) ;
			return	*this ;
		}
		SGLPalette ToColor( void ) const
		{
			SGLPalette	argb ;
			argb.argb.Red =
				(uint8_t) esl_clampi( esl_roundfi
						( SGL3DVector<float32_t>::x * 255.0f ), 0, 0xFF ) ;
			argb.argb.Green =
				(uint8_t) esl_clampi( esl_roundfi
						( SGL3DVector<float32_t>::y * 255.0f ), 0, 0xFF ) ;
			argb.argb.Blue =
				(uint8_t) esl_clampi( esl_roundfi
						( SGL3DVector<float32_t>::z * 255.0f ), 0, 0xFF ) ;
			argb.argb.Alpha =
				(uint8_t) esl_clampi( esl_roundfi( w * 255.0f ), 0, 0xFF ) ;
			return	argb ;
		}
	} ;


	struct	S4DDVector	: public SGL4DVector<double>
	{
		S4DDVector( void ) {}
		S4DDVector( double xInit, double yInit, double zInit, double wInit = 1.0 )
			: SGL4DVector<double>( xInit, yInit, zInit, wInit ) {}
		S4DDVector( const SGL3DVector<double> & v, double wInit = 1.0 )
			: SGL4DVector<double>( v, wInit ) {}
		S4DDVector( const SGL3DVector<float32_t> & v, double wInit = 1.0 )
			: SGL4DVector<double>( v, wInit ) {}
		S4DDVector( const S4DVector & v )
			: SGL4DVector<double>( v ) {}
		const S4DDVector & operator = ( const SGL4DVector<double> & v )
		{
			SGL3DVector<double>::x = v.x ;
			SGL3DVector<double>::y = v.y ;
			SGL3DVector<double>::z = v.z ;
			w = v.w ;
			return	*this ;
		}
		const S4DDVector & operator = ( const SGL3DVector<float32_t> & v )
		{
			SGL3DVector<double>::x = v.x ;
			SGL3DVector<double>::y = v.y ;
			SGL3DVector<double>::z = v.z ;
			w = 1.0 ;
			return	*this ;
		}
		const S4DDVector & operator = ( const SGL3DVector<double> & v )
		{
			SGL3DVector<double>::x = v.x ;
			SGL3DVector<double>::y = v.y ;
			SGL3DVector<double>::z = v.z ;
			w = 1.0 ;
			return	*this ;
		}
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 4x4 行列
	//////////////////////////////////////////////////////////////////////////

	template <class T> struct	SGL4DMatrix
	{
		T	m[4][4] ;

		// 構築
		SGL4DMatrix( void )
		{
			eslFillMemory( &m[0][0], 0, sizeof(T) * 4 * 4 ) ;
		}
		SGL4DMatrix( T m11, T m12, T m13, T m14,
						T m21, T m22, T m23, T m24,
						T m31, T m32, T m33, T m34,
						T m41, T m42, T m43, T m44 )
		{
			m[0][0] = m11 ;  m[0][1] = m12 ;  m[0][2] = m13 ;  m[0][3] = m14 ;
			m[1][0] = m21 ;  m[1][1] = m22 ;  m[1][2] = m23 ;  m[1][3] = m24 ;
			m[2][0] = m31 ;  m[2][1] = m32 ;  m[2][2] = m33 ;  m[2][3] = m34 ;
			m[3][0] = m41 ;  m[3][1] = m42 ;  m[3][2] = m43 ;  m[3][3] = m44 ;
		}
		SGL4DMatrix( T x, T y, T z, T w )
		{
			InitializeMatrix( x, y, z, w ) ;
		}
		SGL4DMatrix( const SGL4DMatrix<T> & mat )
		{
			eslMoveMemory( &m[0][0], &mat.m[0][0], sizeof(T) * 4 * 4 ) ;
		}
		// 対角行列初期化
		void InitializeMatrix( T x, T y, T z, T w )
		{
			eslFillMemory( &m[0][0], 0, sizeof(T) * 4 * 4 ) ;
			m[0][0] = x ;
			m[1][1] = y ;
			m[2][2] = z ;
			m[3][3] = w ;
		}
		// 比較
		bool operator == ( const SGL4DMatrix<T> & mat ) const
		{
			return	(m[0][0] == mat.m[0][0])
						& (m[0][1] == mat.m[0][1])
						& (m[0][2] == mat.m[0][2])
						& (m[0][3] == mat.m[0][3])
					& (m[1][0] == mat.m[1][0])
						& (m[1][1] == mat.m[1][1])
						& (m[1][2] == mat.m[1][2])
						& (m[1][3] == mat.m[1][3])
					& (m[2][0] == mat.m[2][0])
						& (m[2][1] == mat.m[2][1])
						& (m[2][2] == mat.m[2][2])
						& (m[2][3] == mat.m[2][3])
					& (m[3][0] == mat.m[3][0])
						& (m[3][1] == mat.m[3][1])
						& (m[3][2] == mat.m[3][2])
						& (m[3][3] == mat.m[3][3]) ;
		}
		bool operator != ( const SGL4DMatrix<T> & mat ) const
		{
			return	(m[0][0] != mat.m[0][0])
						| (m[0][1] != mat.m[0][1])
						| (m[0][2] != mat.m[0][2])
						| (m[0][3] != mat.m[0][3])
					| (m[1][0] != mat.m[1][0])
						| (m[1][1] != mat.m[1][1])
						| (m[1][2] != mat.m[1][2])
						| (m[1][3] != mat.m[1][3])
					| (m[2][0] != mat.m[2][0])
						| (m[2][1] != mat.m[2][1])
						| (m[2][2] != mat.m[2][2])
						| (m[2][3] != mat.m[2][3])
					| (m[3][0] != mat.m[3][0])
						| (m[3][1] != mat.m[3][1])
						| (m[3][2] != mat.m[3][2])
						| (m[3][3] != mat.m[3][3]) ;
		}
		// 加算
		const SGL4DMatrix<T> & operator += ( const SGL4DMatrix<T> & mat )
		{
			for ( int i = 0; i < 4; i ++ )
			{
				m[i][0] += mat.m[i][0] ;
				m[i][1] += mat.m[i][1] ;
				m[i][2] += mat.m[i][2] ;
				m[i][3] += mat.m[i][3] ;
			}
			return	*this ;
		}
		const SGL4DMatrix<T> & operator += ( const SGL3DVector<T> & vTranslate )
		{
			Translate( vTranslate.x, vTranslate.y, vTranslate.z ) ;
			return	*this ;
		}
		// 減算
		const SGL4DMatrix<T> & operator -= ( const SGL4DMatrix<T> & mat )
		{
			for ( int i = 0; i < 4; i ++ )
			{
				m[i][0] -= mat.m[i][0] ;
				m[i][1] -= mat.m[i][1] ;
				m[i][2] -= mat.m[i][2] ;
				m[i][3] -= mat.m[i][3] ;
			}
			return	*this ;
		}
		const SGL4DMatrix<T> & operator -= ( const SGL3DVector<T> & vTranslate )
		{
			Translate( -vTranslate.x, -vTranslate.y, -vTranslate.z ) ;
			return	*this ;
		}
		// 積
		const SGL4DMatrix<T> & operator *= ( const SGL4DMatrix<T> & mat )
		{
			return	RevolveByMatrix( mat ) ;
		}
		const SGL4DMatrix<T> & operator *= ( T s )
		{
			for ( int i = 0; i < 4; i ++ )
			{
				m[i][0] *= s ;
				m[i][1] *= s ;
				m[i][2] *= s ;
				m[i][3] *= s ;
			}
			return	*this ;
		}
		const SGL4DMatrix<T> & operator /= ( T s )
		{
			for ( int i = 0; i < 4; i ++ )
			{
				m[i][0] /= s ;
				m[i][1] /= s ;
				m[i][2] /= s ;
				m[i][3] /= s ;
			}
			return	*this ;
		}
		// 加算
		SGL4DMatrix<T> operator + ( const SGL4DMatrix<T> & mat ) const
		{
			return	SGL4DMatrix<T>
				( m[0][0] + mat.m[0][0],
						m[0][1] + mat.m[0][1],
						m[0][2] + mat.m[0][2],
						m[0][3] + mat.m[0][3],
					m[1][0] + mat.m[1][0],
						m[1][1] + mat.m[1][1],
						m[1][2] + mat.m[1][2],
						m[1][3] + mat.m[1][3],
					m[2][0] + mat.m[2][0],
						m[2][1] + mat.m[2][1],
						m[2][2] + mat.m[2][2],
						m[2][3] + mat.m[2][3],
					m[3][0] + mat.m[3][0],
						m[3][1] + mat.m[3][1],
						m[3][2] + mat.m[3][2],
						m[3][3] + mat.m[3][3] ) ;
		}
		// 減算
		SGL4DMatrix<T> operator - ( const SGL4DMatrix<T> & mat ) const
		{
			return	SGL4DMatrix<T>
				( m[0][0] - mat.m[0][0],
						m[0][1] - mat.m[0][1],
						m[0][2] - mat.m[0][2],
						m[0][3] - mat.m[0][3],
					m[1][0] - mat.m[1][0],
						m[1][1] - mat.m[1][1],
						m[1][2] - mat.m[1][2],
						m[1][3] - mat.m[1][3],
					m[2][0] - mat.m[2][0],
						m[2][1] - mat.m[2][1],
						m[2][2] - mat.m[2][2],
						m[2][3] - mat.m[2][3],
					m[3][0] - mat.m[3][0],
						m[3][1] - mat.m[3][1],
						m[3][2] - mat.m[3][2],
						m[3][3] - mat.m[3][3] ) ;
		}
		SGL4DMatrix<T> operator - ( void ) const
		{
			return	SGL4DMatrix<T>
				( - m[0][0], - m[0][1], - m[0][2], - m[0][3],
					- m[1][0], - m[1][1], - m[1][2], - m[1][3],
					- m[2][0], - m[2][1], - m[2][2],  - m[2][3],
					- m[3][0], - m[3][1], - m[3][2],  - m[3][3] ) ;
		}
		// 積
		SGL4DMatrix<T> operator * ( const SGL4DMatrix<T> & mat ) const
		{
			SGL4DMatrix<T>	t = *this ;
			return	t.RevolveByMatrix( mat ) ;
		}
		SGL4DVector<T> operator * ( const SGL4DVector<T> & v )
		{
			const T	x = v.x, y = v.y, z = v.z, w = v.w ;
			return	SGL4DVector<T>
				( m[0][0] * x + m[0][1] * y + m[0][2] * z + m[0][3] * w,
					m[1][0] * x + m[1][1] * y + m[1][2] * z + m[1][3] * w,
					m[2][0] * x + m[2][1] * y + m[2][2] * z + m[2][3] * w,
					m[3][0] * x + m[3][1] * y + m[3][2] * z + m[3][3] * w ) ;
		}
		SGL4DMatrix<T> operator * ( T s ) const
		{
			return	SGL4DMatrix<T>
				( m[0][0] * s, m[0][1] * s, m[0][2] * s, m[0][3] * s,
					m[1][0] * s, m[1][1] * s, m[1][2] * s, m[1][3] * s,
					m[2][0] * s, m[2][1] * s, m[2][2] * s, m[2][3] * s,
					m[3][0] * s, m[3][1] * s, m[3][2] * s, m[3][3] * s ) ;
		}
		SGL4DMatrix<T> operator / ( T s ) const
		{
			return	SGL4DMatrix<T>
				( m[0][0] / s, m[0][1] / s, m[0][2] / s, m[0][3] / s,
					m[1][0] / s, m[1][1] / s, m[1][2] / s, m[1][3] / s,
					m[2][0] / s, m[2][1] / s, m[2][2] / s, m[2][3] / s,
					m[3][0] / s, m[3][1] / s, m[3][2] / s, m[3][3] / s ) ;
		}
		const SGL4DMatrix<T> & RevolveByMatrix( const SGL4DMatrix<T> & mat )
		{
			for ( int i = 0; i < 4; i ++ )
			{
				T	r1, r2, r3, r4 ;
				r1 = m[i][0] ;
				r2 = m[i][1] ;
				r3 = m[i][2] ;
				r4 = m[i][3] ;
				m[i][0] = (r1 * mat.m[0][0] + r2 * mat.m[1][0]
								+ r3 * mat.m[2][0] + r4 * mat.m[3][0]) ;
				m[i][1] = (r1 * mat.m[0][1] + r2 * mat.m[1][1]
								+ r3 * mat.m[2][1] + r4 * mat.m[3][1]) ;
				m[i][2] = (r1 * mat.m[0][2] + r2 * mat.m[1][2]
								+ r3 * mat.m[2][2] + r4 * mat.m[3][2]) ;
				m[i][3] = (r1 * mat.m[0][3] + r2 * mat.m[1][3]
								+ r3 * mat.m[2][3] + r4 * mat.m[3][3]) ;
			}
			return	*this ;
		}
		// スケール
		const SGL4DMatrix<T> & Scale( double x, double y, double z )
		{
			m[0][0] *= (T) x ;	m[0][1] *= (T) y ;	m[0][2] *= (T) z ;
			m[1][0] *= (T) x ;	m[1][1] *= (T) y ;	m[1][2] *= (T) z ;
			m[2][0] *= (T) x ;	m[2][1] *= (T) y ;	m[2][2] *= (T) z ;
			m[3][0] *= (T) x ;	m[3][1] *= (T) y ;	m[3][2] *= (T) z ;
			return	*this ;
		}
		// 平行移動
		const SGL4DMatrix<T> & Translate( double x, double y, double z )
		{
			m[0][3] += (T) (m[0][0] * x + m[0][1] * y + m[0][2] * z) ;
			m[1][3] += (T) (m[1][0] * x + m[1][1] * y + m[1][2] * z) ;
			m[2][3] += (T) (m[2][0] * x + m[2][1] * y + m[2][2] * z) ;
			m[3][3] += (T) (m[3][0] * x + m[3][1] * y + m[3][2] * z) ;
			return	*this ;
		}
		// 積
		void RevolveVector( SGL4DVector<T> & v ) const
		{
			const T	x = v.x, y = v.y, z = v.z, w = v.w ;
			v.x = m[0][0] * x + m[0][1] * y + m[0][2] * z + m[0][3] * w ;
			v.y = m[1][0] * x + m[1][1] * y + m[1][2] * z + m[1][3] * w ;
			v.z = m[2][0] * x + m[2][1] * y + m[2][2] * z + m[2][3] * w ;
			v.w = m[3][0] * x + m[3][1] * y + m[3][2] * z + m[3][3] * w ;
		}
		// 転置行列
		const SGL4DMatrix<T> & TransposeOf( const SGL4DMatrix<T> & mat )
		{
			for ( int i = 0; i < 4; i ++ )
			{
				m[i][0] = mat.m[0][i] ;
				m[i][1] = mat.m[1][i] ;
				m[i][2] = mat.m[2][i] ;
				m[i][3] = mat.m[3][i] ;
			}
			return	*this ;
		}
		// 行列式
		T Determinant( void ) const
		{
			T	d = 0.0 ;
			for ( int i = 0; i < 4; i ++ )
			{
				int	j = (int) (i == 0) ;
				int	k = 1 + (int) (i <= 1) ;
				int	l = 2 + (int) (i <= 2) ;
				T	a = m[1][j] * (m[2][k] * m[3][l] - m[3][k] * m[2][l])
						+ m[1][k] * (m[2][l] * m[3][j] - m[3][l] * m[2][j])
						+ m[1][l] * (m[2][j] * m[3][k] - m[3][j] * m[2][k]) ;
				if ( i & 0x01 )
				{
					d -= m[0][i] * a ;
				}
				else
				{
					d += m[0][i] * a ;
				}
			}
			return	d ;
		}
		// 逆行列
		const SGL4DMatrix<T> & InverseOf( const SGL4DMatrix<T> & mat )
		{
			T	d = (T) (1.0 / mat.Determinant()) ;
			if ( isinf(d) )	d = 0.0 ;
			for ( int i = 0; i < 4; i ++ )
			{
				int	j0 = (int) (i == 0) ;
				int	j1 = 1 + (int) (i <= 1) ;
				int	j2 = 2 + (int) (i <= 2) ;
				//
				for ( int j = 0; j < 4; j ++ )
				{
					int	i0 = (int) (j == 0) ;
					int	i1 = 1 + (int) (j <= 1) ;
					int	i2 = 2 + (int) (j <= 2) ;
					//
					T	a = mat.m[i0][j0]
								* (mat.m[i1][j1] * mat.m[i2][j2]
										- mat.m[i2][j1] * mat.m[i1][j2])
							+ mat.m[i0][j1]
								* (mat.m[i1][j2] * mat.m[i2][j0]
										- mat.m[i2][j2] * mat.m[i1][j0])
							+ mat.m[i0][j2]
								* (mat.m[i1][j0] * mat.m[i2][j1]
										- mat.m[i2][j0] * mat.m[i1][j1]) ;
					if ( (i ^ j) & 0x01 )
					{
						a = - a ;
					}
					m[i][j] = a * d ;
				}
			}
			return	*this ;
		}
		// 平行投影
		void OrthogonalProjection
			( double xViewLeft, double xViewRight,
				double yViewTop, double yViewBottom,
				double zNear, double zFar, bool yReverse = false,
				double xOffset = 0.0, double yOffset = 0.0,
				const SGLAffine* pAffineRotation = NULL )
		{
			T	tx = (T) (- (xViewRight + xViewLeft)
								/ (xViewRight - xViewLeft)) ;
			T	ty = (T) (- (yViewBottom + yViewTop)
								/ (yViewBottom - yViewTop)) ;
			T	tz = (T) ((zFar + zNear) / (zNear - zFar)) ;
			T	xs = (T) (2.0 / (xViewRight - xViewLeft)) ;
			T	ys = (T) (2.0 / (yViewBottom - yViewTop)) ;
			T	zs = (T) (2.0 / (zFar - zNear)) ;
			//
			if ( yReverse )
			{
				ty = - ty ;
				ys = - ys ;
			}
			//
			m[0][0] = xs ;	m[0][1] = 0.0 ;	m[0][2] = 0.0 ;	m[0][3] = tx ;
			m[1][0] = 0.0 ;	m[1][1] = ys ;	m[1][2] = 0.0 ;	m[1][3] = ty ;
			m[2][0] = 0.0 ;	m[2][1] = 0.0 ;	m[2][2] = zs ;	m[2][3] = tz ;
			m[3][0] = 0.0 ;	m[3][1] = 0.0 ;	m[3][2] = 0.0 ;	m[3][3] = 1.0 ;
			//
			if ( pAffineRotation != NULL )
			{
				SGL4DMatrix<T>	mat
					( pAffineRotation->a11, pAffineRotation->a12,
											0.0, pAffineRotation->a13,
						pAffineRotation->a21, pAffineRotation->a22,
											0.0, pAffineRotation->a23,
						0.0, 0.0, 1.0, 0.0,
						0.0, 0.0, 0.0, 1.0 ) ;
				RevolveByMatrix( mat ) ;
			}
			Translate( xOffset, yOffset, 0.0 ) ;
		}
		// 透視変換
		enum	PerspectiveProjectionType
		{
			persNoZBounds_YUp	= 0,
			persZBoundsN1_1_YUp	= 1,		// z=[-1,1], y-up
			persZBounds0_1_YUp	= 2,		// z=[0,1], y-up
		} ;
		void PerspectiveProjection
			( double xScreen, double yScreen, double zScreen,
				double widthView, double heightView,
				double zNear, double zFar,
				PerspectiveProjectionType fZBounds = persZBoundsN1_1_YUp,
				double xViewLeft = 0.0, double yViewBottom = 0.0,
				double xOffset = 0.0, double yOffset = 0.0,
				const SGLAffine* pAffineRotation = NULL )
		{
			T	ws, hs, xs, ys, xo, yo ;
			xScreen += xOffset ;
			yScreen += yOffset ;
			if ( pAffineRotation != NULL )
			{
				double	xd = pAffineRotation->a11 * xScreen
								+ pAffineRotation->a12 * yScreen
								+ pAffineRotation->a13 - xViewLeft ;
				double	yd = pAffineRotation->a21 * xScreen
								+ pAffineRotation->a22 * yScreen
								+ pAffineRotation->a23 - yViewBottom ;
				ws = (T) (heightView * 0.5) ;
				hs = (T) (widthView * 0.5) ;
				xs = (T)   zScreen / ws ;
				ys = (T) - zScreen / hs ;
				xo = (T)   (xd - ws) / ws ;
				yo = (T) - (yd - hs) / hs ;
			}
			else
			{
				xScreen -= xViewLeft ;
				yScreen -= yViewBottom ;
				ws = (T) (widthView * 0.5) ;
				hs = (T) (heightView * 0.5) ;
				xs = (T)   zScreen / ws ;
				ys = (T) - zScreen / hs ;
				xo = (T)   (xScreen - ws) / ws ;
				yo = (T) - (yScreen - hs) / hs ;
			}
			if ( fZBounds == persZBoundsN1_1_YUp )
			{
				T	zn = (T) zNear ;		// > 0
				T	zf = (T) zFar ;			// > 0
				//
				m[0][0] = xs ;	m[0][1] = 0.0 ;	m[0][2] = xo ;	m[0][3] = 0.0 ;
				m[1][0] = 0.0 ;	m[1][1] = ys ;	m[1][2] = yo ;	m[1][3] = 0.0 ;
				m[2][0] = 0.0 ;	m[2][1] = 0.0 ;
					m[2][2] = (zf+zn) / (zf-zn) ;
					m[2][3] = (T) -2.0*zf*zn / (zf-zn) ;
				m[3][0] = 0.0 ;	m[3][1] = 0.0 ;	m[3][2] = 1.0 ;	m[3][3] = 0.0 ;
			}
			else if ( fZBounds == persZBounds0_1_YUp )
			{
				T	zn = (T) zNear ;		// > 0
				T	zf = (T) zFar ;			// > 0
				//
				m[0][0] = xs ;	m[0][1] = 0.0 ;	m[0][2] = xo ;	m[0][3] = 0.0 ;
				m[1][0] = 0.0 ;	m[1][1] = ys ;	m[1][2] = yo ;	m[1][3] = 0.0 ;
				m[2][0] = 0.0 ;	m[2][1] = 0.0 ;
					m[2][2] = zf / (zf-zn) ;
					m[2][3] = -zf*zn / (zf-zn) ;
				m[3][0] = 0.0 ;	m[3][1] = 0.0 ;	m[3][2] = 1.0 ;	m[3][3] = 0.0 ;
			}
			else
			{
				T	zr = (T) zScreen ;
				//
				m[0][0] = xs ;	m[0][1] = 0.0 ;	m[0][2] = xo ;	m[0][3] = 0.0 ;
				m[1][0] = 0.0 ;	m[1][1] = ys ;	m[1][2] = yo ;	m[1][3] = 0.0 ;
				m[2][0] = 0.0 ;	m[2][1] = 0.0 ;	m[2][2] = 1.0 ;	m[2][3] = -zr ;
				m[3][0] = 0.0 ;	m[3][1] = 0.0 ;	m[3][2] = 1.0 ;	m[3][3] = 0.0 ;
			}
			if ( pAffineRotation != NULL )
			{
				SGL4DMatrix<T>	mat
					( pAffineRotation->a11, pAffineRotation->a12, 0.0, 0.0f,
						pAffineRotation->a21, pAffineRotation->a22, 0.0, 0.0f,
						0.0, 0.0, 1.0, 0.0,
						0.0, 0.0, 0.0, 1.0 ) ;
				RevolveByMatrix( mat ) ;
			}
		}
		// ｚ値 - depth 値変換
		void ZValueFromDepthOnOrthogonal
			( T* pzDst, const T* pzSrc, size_t nCount )
		{
			T	m22 = m[2][2] ;
			T	m23 = m[2][3] ;
			T	im22 = (T) 1.0 / m22 ;
			for ( size_t i = 0; i < nCount; i ++ )
			{
				pzDst[i] = (m23 - pzSrc[i]) * im22 ;
			}
		}
		void ZValueFromDepthOnPerspective
			( T* pzDst, const T* pzSrc, size_t nCount )
		{
			T	m22 = m[2][2] ;
			T	m23 = m[2][3] ;
			for ( size_t i = 0; i < nCount; i ++ )
			{
				pzDst[i] = m23 / (pzSrc[i] - m22) ;
			}
		}
		void DepthFromZValueOnOrthogonal
			( T* pzDst, const T* pzSrc, size_t nCount )
		{
			T	m22 = m[2][2] ;
			T	m23 = m[2][3] ;
			for ( size_t i = 0; i < nCount; i ++ )
			{
				pzDst[i] = m22 * pzSrc[i] + m23 ;
			}
		}
		void DepthFromZValueOnPerspective
			( T* pzDst, const T* pzSrc, size_t nCount )
		{
			T	m22 = m[2][2] ;
			T	m23 = m[2][3] ;
			for ( size_t i = 0; i < nCount; i ++ )
			{
				T	z = pzSrc[i] ;
				pzDst[i] = (m22 * z + m23) / z ;
			}
		}
	} ;

	struct	S4DMatrix	: public SGL4DMatrix<float32_t>
	{
		S4DMatrix( void )
		{
			eslFillMemory( &m[0][0], 0, sizeof(float32_t) * 4 * 4 ) ;
		}
		S4DMatrix( double m11, double m12, double m13, double m14,
					double m21, double m22, double m23, double m24,
					double m31, double m32, double m33, double m34,
					double m41, double m42, double m43, double m44 )
		{
			m[0][0] = (float32_t) m11 ;  m[0][1] = (float32_t) m12 ;
				m[0][2] = (float32_t) m13 ;  m[0][3] = (float32_t) m14 ;
			m[1][0] = (float32_t) m21 ;  m[1][1] = (float32_t) m22 ;
				m[1][2] = (float32_t) m23 ;  m[1][3] = (float32_t) m24 ;
			m[2][0] = (float32_t) m31 ;  m[2][1] = (float32_t) m32 ;
				m[2][2] = (float32_t) m33 ;  m[2][3] = (float32_t) m34 ;
			m[3][0] = (float32_t) m41 ;  m[3][1] = (float32_t) m42 ;
				m[3][2] = (float32_t) m43 ;  m[3][3] = (float32_t) m44 ;
		}
		S4DMatrix( double x, double y, double z, double w )
			: SGL4DMatrix<float32_t>
				( (float32_t) x, (float32_t) y, (float32_t) z, (float32_t) w ) { }
		S4DMatrix( const S3DMatrix & mat3, const S3DVector& vec3 )
		{
			m[0][0] = mat3.m[0][0] ;  m[0][1] = mat3.m[0][1] ;
				m[0][2] = mat3.m[0][2] ;  m[0][3] = vec3.x ;
			m[1][0] = mat3.m[1][0] ;  m[1][1] = mat3.m[1][1] ;
				m[1][2] = mat3.m[1][2] ;  m[1][3] = vec3.y ;
			m[2][0] = mat3.m[2][0] ;  m[2][1] = mat3.m[2][1] ;
				m[2][2] = mat3.m[2][2] ;  m[2][3] = vec3.z ;
			m[3][0] = 0.0f ;  m[3][1] = 0.0f ;
				m[3][2] = 0.0f ;  m[3][3] = 1.0f ;
		}
		S4DMatrix( const SGL4DMatrix<float32_t> & mat )
		{
			eslMoveMemory( &m[0][0], &mat.m[0][0], sizeof(float32_t) * 4 * 4 ) ;
		}
		S4DMatrix( const SGL4DMatrix<double> & mat )
		{
			for ( int i = 0; i < 4; i ++ )
			{
				m[i][0] = (float32_t) mat.m[i][0] ;
				m[i][1] = (float32_t) mat.m[i][1] ;
				m[i][2] = (float32_t) mat.m[i][2] ;
				m[i][3] = (float32_t) mat.m[i][3] ;
			}
		}
		S4DMatrix Inverse( void ) const
		{
			S4DMatrix	t ;
			t.InverseOf( *this ) ;
			return	t ;
		}
		S3DMatrix GetMatrix3( void ) const
		{
			return	S3DMatrix( m[0][0], m[0][1], m[0][2],
								m[1][0], m[1][1], m[1][2],
								m[2][0], m[2][1], m[2][2] ) ;
		}
		void SetMatrix3( const S3DMatrix& m3 )
		{
			for ( int i = 0; i < 3; i ++ )
			{
				m[i][0] = m3.m[i][0] ;
				m[i][1] = m3.m[i][1] ;
				m[i][2] = m3.m[i][2] ;
			}
		}
		S3DVector GetTranslation( void ) const
		{
			return	S3DVector( m[0][3], m[1][3], m[2][3] ) ;
		}
		void SetTranslation( const S3DVector& v )
		{
			m[0][3] = v.x ;
			m[1][3] = v.y ;
			m[2][3] = v.z ;
		}
	} ;


	struct	S4DDMatrix	: public SGL4DMatrix<double>
	{
		S4DDMatrix( void )
		{
			eslFillMemory( &m[0][0], 0, sizeof(double) * 4 * 4 ) ;
		}
		S4DDMatrix( double m11, double m12, double m13, double m14,
					double m21, double m22, double m23, double m24,
					double m31, double m32, double m33, double m34,
					double m41, double m42, double m43, double m44 )
			: SGL4DMatrix<double>
					( m11, m12, m13, m14,
						m21, m22, m23, m24,
						m31, m32, m33, m34,
						m41, m42, m43, m44 ) { }
		S4DDMatrix( double x, double y, double z, double w )
			: SGL4DMatrix<double>( x, y, z, w ) { }
		S4DDMatrix( const S3DDMatrix & mat3, const S3DDVector& vec3 )
		{
			m[0][0] = mat3.m[0][0] ;  m[0][1] = mat3.m[0][1] ;
				m[0][2] = mat3.m[0][2] ;  m[0][3] = vec3.x ;
			m[1][0] = mat3.m[1][0] ;  m[1][1] = mat3.m[1][1] ;
				m[1][2] = mat3.m[1][2] ;  m[1][3] = vec3.y ;
			m[2][0] = mat3.m[2][0] ;  m[2][1] = mat3.m[2][1] ;
				m[2][2] = mat3.m[2][2] ;  m[2][3] = vec3.z ;
			m[3][0] = 0.0 ;  m[3][1] = 0.0 ;
				m[3][2] = 0.0 ;  m[3][3] = 1.0 ;
		}
		S4DDMatrix( const SGL4DMatrix<double> & mat )
		{
			eslMoveMemory( &m[0][0], &mat.m[0][0], sizeof(double) * 4 * 4 ) ;
		}
		S4DDMatrix( const SGL4DMatrix<float32_t> & mat )
		{
			for ( int i = 0; i < 4; i ++ )
			{
				m[i][0] = mat.m[i][0] ;
				m[i][1] = mat.m[i][1] ;
				m[i][2] = mat.m[i][2] ;
				m[i][3] = mat.m[i][3] ;
			}
		}
		S4DDMatrix Inverse( void ) const
		{
			S4DDMatrix	t ;
			t.InverseOf( *this ) ;
			return	t ;
		}
		S3DDMatrix GetMatrix3( void ) const
		{
			return	S3DDMatrix( m[0][0], m[0][1], m[0][2],
								m[1][0], m[1][1], m[1][2],
								m[2][0], m[2][1], m[2][2] ) ;
		}
		void SetMatrix3( const S3DDMatrix& m3 )
		{
			for ( int i = 0; i < 3; i ++ )
			{
				m[i][0] = m3.m[i][0] ;
				m[i][1] = m3.m[i][1] ;
				m[i][2] = m3.m[i][2] ;
			}
		}
		S3DDVector GetTranslation( void ) const
		{
			return	S3DDVector( m[0][3], m[1][3], m[2][3] ) ;
		}
		void SetTranslation( const S3DDVector& v )
		{
			m[0][3] = v.x ;
			m[1][3] = v.y ;
			m[2][3] = v.z ;
		}
	} ;

	template < class T, class M3, class V3 >
		void Matrix4x4From3x3
				( SGL4DMatrix<T>& mDst, const M3& mSrc, const V3& vSrc )
		{
			mDst.m[0][0] = (T) mSrc.m[0][0] ;
			mDst.m[0][1] = (T) mSrc.m[0][1] ;
			mDst.m[0][2] = (T) mSrc.m[0][2] ;
			mDst.m[0][3] = (T) vSrc.x ;
			mDst.m[1][0] = (T) mSrc.m[1][0] ;
			mDst.m[1][1] = (T) mSrc.m[1][1] ;
			mDst.m[1][2] = (T) mSrc.m[1][2] ;
			mDst.m[1][3] = (T) vSrc.y ;
			mDst.m[2][0] = (T) mSrc.m[2][0] ;
			mDst.m[2][1] = (T) mSrc.m[2][1] ;
			mDst.m[2][2] = (T) mSrc.m[2][2] ;
			mDst.m[2][3] = (T) vSrc.z ;
			mDst.m[3][0] = 0.0 ;
			mDst.m[3][1] = 0.0 ;
			mDst.m[3][2] = 0.0 ;
			mDst.m[3][3] = 1.0 ;
		}

	template < class M3, class V3, class T >
		void Matrix3x3From4x4
				( M3& mDst, V3& vDst, const SGL4DMatrix<T>& mSrc )
		{
			T	d = (T) 1.0 / mSrc.m[3][3] ;
			mDst.m[0][0] = mSrc.m[0][0] * d ;
			mDst.m[0][1] = mSrc.m[0][1] * d ;
			mDst.m[0][2] = mSrc.m[0][2] * d ;
			vDst.x = mSrc.m[0][3] * d ;
			mDst.m[1][0] = mSrc.m[1][0] * d ;
			mDst.m[1][1] = mSrc.m[1][1] * d ;
			mDst.m[1][2] = mSrc.m[1][2] * d ;
			vDst.y = mSrc.m[1][3] * d ;
			mDst.m[2][0] = mSrc.m[2][0] * d ;
			mDst.m[2][1] = mSrc.m[2][1] * d ;
			mDst.m[2][2] = mSrc.m[2][2] * d ;
			vDst.z = mSrc.m[2][3] * d ;
		}

}

#endif
