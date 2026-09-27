
#include <sakuragl/sakuragl.h>
#include <sakuragl/sgl2d/sgl_spline_curve.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// NxN 一般行列式
//////////////////////////////////////////////////////////////////////////////

// 行列式
//////////////////////////////////////////////////////////////////////////////
double GenericMatrix::Determinant( void ) const
{
	const double *	m = SArray<double>::m_ptrArray ;
	if ( m_N <= 3 )
	{
		if ( m_N == 3 )
		{
			return	m[0] * (m[4] * m[8] - m[7] * m[5])
					+ m[1] * (m[5] * m[6] - m[8] * m[3])
					+ m[2] * (m[3] * m[7] - m[6] * m[4]) ;
		}
		else if ( m_N == 2 )
		{
			return	m[0] * m[3] - m[1] * m[2] ;
		}
		else
		{
			return	m[0] ;
		}
	}
	double			d = 0.0 ;
	ReferenceMatrix	mTemp ;
	for ( int i = 0; i < m_N; i ++ )
	{
		if ( m[i] != 0.0 )
		{
			mTemp.CofactorOf( *this, 0, i ) ;
			if ( i & 0x01 )
			{
				d -= m[i] * mTemp.Determinant() ;
			}
			else
			{
				d += m[i] * mTemp.Determinant() ;
			}
		}
	}
	return	d ;
}

// 逆行列
//////////////////////////////////////////////////////////////////////////////
const GenericMatrix& GenericMatrix::InverseOf( const GenericMatrix& mat )
{
	m_N = mat.m_N ;
	SArray<double>::SetLength( (size_t) (m_N * m_N) ) ;
	//
	if ( m_N == 1 )
	{
		SetAt( 0, 0, 1.0 / mat.GetAt( 0, 0 ) ) ;
	}
	else
	{
		ReferenceMatrix	mTemp ;
		double			d = 1.0 / mat.Determinant() ;
		double *		m = SArray<double>::m_ptrArray ;
		for ( int i = 0; i < m_N; i ++ )
		{
			for ( int j = 0; j < m_N; j ++ )
			{
				double	a ;
				mTemp.CofactorOf( mat, j, i ) ;
				a = mTemp.Determinant() * d ;
				if ( (i ^ j) & 0x01 )
				{
					a = -a ;
				}
				m[i * m_N + j] = a ;
			}
		}
	}
	return	*this ;
}

// ベクトル積
//////////////////////////////////////////////////////////////////////////////
void GenericMatrix::ProductVector( double * pDst, const double * pSrc ) const
{
	const double *	m = SArray<double>::m_ptrArray ;
	for ( int i = 0; i < m_N; i ++ )
	{
		double	x = 0.0 ;
		for ( int j = 0; j < m_N; j ++ )
		{
			x += m[j] * pSrc[j] ;
		}
		m += m_N ;
		pDst[i] = x ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// NxN 参照行列
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
ReferenceMatrix::ReferenceMatrix( GenericMatrix& mat )
{
	m_mat = mat.GetArray() ;
	m_refN = (size_t) mat.m_N ;
	m_N = m_refN ;
	m_lines = new size_t[m_N * 2] ;
	m_cols = m_lines + m_N ;
	for ( size_t i = 0; i < m_N; i ++ )
	{
		m_lines[i] = i * m_refN ;
		m_cols[i] = i ;
	}
}

// 行と列を除去（余因子計算用）
//////////////////////////////////////////////////////////////////////////////
const ReferenceMatrix& ReferenceMatrix::CofactorOf
		( const GenericMatrix& mat, int iLine, int jCol )
{
	m_mat = mat.GetArray() ;
	m_refN = mat.m_N ;
	if ( m_N != mat.m_N - 1 )
	{
		m_N = mat.m_N - 1 ;
		delete []	m_lines ;
		m_lines = new size_t[m_N * 2] ;
		m_cols = m_lines + m_N ;
	}
	//
	for ( int i = 0, iDst = 0; i < mat.m_N; i ++ )
	{
		if ( i != iLine )
		{
			m_lines[iDst ++] = i * m_refN ;
		}
	}
	for ( int i = 0, iDst = 0; i < mat.m_N; i ++ )
	{
		if ( i != jCol )
		{
			m_cols[iDst ++] = i ;
		}
	}
	return	*this ;
}

const ReferenceMatrix& ReferenceMatrix::CofactorOf
		( const ReferenceMatrix& mat, int iLine, int jCol )
{
	m_mat = mat.m_mat ;
	m_refN = mat.m_refN ;
	if ( m_N != mat.m_N - 1 )
	{
		m_N = mat.m_N - 1 ;
		delete []	m_lines ;
		m_lines = new size_t[m_N * 2] ;
		m_cols = m_lines + m_N ;
	}
	//
	for ( size_t i = 0, iDst = 0; i < mat.m_N; i ++ )
	{
		if ( (int) i != iLine )
		{
			m_lines[iDst ++] = mat.m_lines[i] ;
		}
	}
	for ( size_t i = 0, iDst = 0; i < mat.m_N; i ++ )
	{
		if ( (int) i != jCol )
		{
			m_cols[iDst ++] = mat.m_cols[i] ;
		}
	}
	return	*this ;
}

// 行列式
//////////////////////////////////////////////////////////////////////////////
double ReferenceMatrix::Determinant( void ) const
{
	if ( m_N <= 3 )
	{
		if ( m_N == 3 )
		{
			return	GetAt(0,0) * (GetAt(1,1) * GetAt(2,2)
									- GetAt(2,1) * GetAt(1,2))
					+ GetAt(0,1) * (GetAt(1,2) * GetAt(2,0)
									- GetAt(2,2) * GetAt(1,0))
					+ GetAt(0,2) * (GetAt(1,0) * GetAt(2,1)
									- GetAt(2,0) * GetAt(1,1)) ;
		}
		else if ( m_N == 2 )
		{
			return	GetAt(0,0) * GetAt(1,1)
							- GetAt(0,1) * GetAt(1,0) ;
		}
		else
		{
			return	GetAt(0,0) ;
		}
	}
	double			d = 0.0 ;
	ReferenceMatrix	mTemp ;
	for ( size_t i = 0; i < m_N; i ++ )
	{
		double	m = GetAt( 0, (int) i ) ;
		if ( m != 0.0 )
		{
			mTemp.CofactorOf( *this, 0, (int) i ) ;
			if ( i & 0x01 )
			{
				d -= m * mTemp.Determinant() ;
			}
			else
			{
				d += m * mTemp.Determinant() ;
			}
		}
	}
	return	d ;
}


//////////////////////////////////////////////////////////////////////////////
// スプライン補完
//////////////////////////////////////////////////////////////////////////////

// 3次スプライン補完係数計算
// S(t) = a*(t-t0)^3 + b*(t-t0)^2 + c*(t-t0) + d
//////////////////////////////////////////////////////////////////////////////
void SGLSplineCurves::CreateSpline
	( const double * p, const double * t, size_t nCount )
{
	ESLAssert( nCount >= 1 ) ;
	if ( nCount < 3 )
	{
		Coefficient	coef ;
		coef.a = 0.0 ;
		coef.b = 0.0 ;
		coef.d = p[0] ;
		if ( nCount == 2 )
		{
			coef.c = (p[1] - p[0]) / (t[1] - t[0]) ;
		}
		else
		{
			coef.c = 0.0 ;
		}
		coef.t = t[0] ;
		m_params.RemoveAll() ;
		m_params.Add( coef ) ;
		return ;
	}
	const int	N = (int) nCount - 1 ;		// 区間数
	//
	// 二次導関数値 u(i) を求める
	//
	GenericMatrix	Q( N - 1 ) ;
	for ( int i = 0; i < N - 1; i ++ )
	{
		if ( i >= 1 )
		{
			Q.SetAt( i, i - 1, t[i + 1] - t[i] ) ;
		}
		Q.SetAt( i, i, (t[i + 2] - t[i]) * 2.0 ) ;
		if ( i + 1 < N - 1 )
		{
			Q.SetAt( i, i + 1, t[i + 2] - t[i + 1] ) ;
		}
	}
	GenericMatrix	R ;
	R.InverseOf( Q ) ;
	//
	SSystem::SArray<double>	vBuf ;
	double *	v = vBuf.GetArray( nCount ) ;
	for ( int i = 1; i < N; i ++ )
	{
		v[i] = ((p[i + 1] - p[i]) / (t[i + 1] - t[i])
				- (p[i] - p[i - 1]) / (t[i] - t[i - 1])) * 6.0 ;
	}
	//
	SSystem::SArray<double>	uBuf ;
	double *	u = uBuf.GetArray( nCount ) ;
	R.ProductVector( u + 1, v + 1 ) ;
	u[0] = 0.0 ;
	u[N] = 0.0 ;
	//
	vBuf.FinishArray() ;
	//
	// 区間毎の係数を計算
	//
	Coefficient	coef ;
	m_params.SetLength( (size_t) N + 1 ) ;
	for ( int i = 0; i < N; i ++ )
	{
		coef.a = (u[i + 1] - u[i]) / ((t[i + 1] - t[i]) * 6.0) ;
		coef.b = u[i] / 2.0 ;
		coef.c = (p[i + 1] - p[i]) / (t[i + 1] - t[i])
				- (t[i + 1] - t[i]) * (2.0 * u[i] + u[i + 1]) / 6.0 ;
		coef.d = p[i] ;
		coef.t = t[i] ;
		m_params.SetAt( (size_t) i, coef ) ;
	}
	uBuf.FinishArray() ;
	//
	coef.a = 0.0 ;
	coef.b = 0.0 ;
	coef.c = 0.0 ;
	coef.d = p[N] ;
	coef.t = t[N] ;
	m_params.SetAt( (size_t) N, coef ) ;
}

// 係数取得
//////////////////////////////////////////////////////////////////////////////
const SGLSplineCurves::Coefficient& SGLSplineCurves::CoefficientAt( size_t i ) const
{
	return	m_params.At(i) ;
}

// 補完値計算
//////////////////////////////////////////////////////////////////////////////
double SGLSplineCurves::InterpolateAt( size_t i, double t ) const
{
	Coefficient *	p = m_params.GetAt( i ) ;
	if ( p != NULL )
	{
		t -= p->t ;
		return	((p->a * t + p->b) * t + p->c) * t + p->d ;
	}
	p = m_params.GetLastAt() ;
	if ( p != NULL )
	{
		return	p->d ;
	}
	return	0.0 ;
}

double SGLSplineCurves::Interpolate( double t ) const
{
	const Coefficient *	p = m_params.GetConstArray() ;
	ssize_t	iFirst = 0 ;
	ssize_t	iEnd = (ssize_t) m_params.GetLength() ;
	while ( iFirst + 1 < iEnd )
	{
		ssize_t	i = (iFirst + iEnd) >> 1 ;
		if ( t < p[i].t )
		{
			ESLAssert( iEnd != i ) ;
			iEnd = i ;
		}
		else
		{
			ESLAssert( iFirst != i ) ;
			iFirst = i ;
		}
	}
	return	InterpolateAt( iFirst, t ) ;
}




//////////////////////////////////////////////////////////////////////////////
// スプライン補完（4点3区間限定）
//////////////////////////////////////////////////////////////////////////////

// 3次スプライン補完係数計算
//////////////////////////////////////////////////////////////////////////////
void SGLSplineCurves3::CreateSpline( const double * p, const double * t )
{
	const int	N = 3 ;		// 区間数
	//
	// 二次導関数値 u(i) を求める
	//
	SGL2DMatrix<double>
		Q( (t[2] - t[0]) * 2.0,  t[2] - t[1],
			t[2] - t[1],         (t[3] - t[1]) * 2.0 ) ;
	//
	SGL2DMatrix<double>	R ;
	R.InverseOf( Q ) ;
	//
	double	v[4] ;
	v[0] = 0 ;
	v[3] = 0 ;
	for ( int i = 1; i < 3; i ++ )
	{
		v[i] = ((p[i + 1] - p[i]) / (t[i + 1] - t[i])
				- (p[i] - p[i - 1]) / (t[i] - t[i - 1])) * 6.0 ;
	}
	SGL2DVector<double,double>
			u1 = R * SGL2DVector<double,double>( v[1], v[2] ) ;
	//
	double	u[4] ;
	u[0] = 0.0 ;
	u[1] = u1.x ;
	u[2] = u1.y ;
	u[3] = 0.0 ;
	//
	// 区間毎の係数を計算
	//
	Coefficient	coef ;
	for ( int i = 0; i < 3; i ++ )
	{
		coef.a = (u[i + 1] - u[i]) / ((t[i + 1] - t[i]) * 6.0) ;
		coef.b = u[i] / 2.0 ;
		coef.c = (p[i + 1] - p[i]) / (t[i + 1] - t[i])
				- (t[i + 1] - t[i]) * (2.0 * u[i] + u[i + 1]) / 6.0 ;
		coef.d = p[i] ;
		coef.t = t[i] ;
		m_params[i] = coef ;
	}
	coef.a = 0.0 ;
	coef.b = 0.0 ;
	coef.c = 0.0 ;
	coef.d = p[3] ;
	coef.t = t[3] ;
	m_params[3] = coef ;
}

// 係数取得
//////////////////////////////////////////////////////////////////////////////
const SGLSplineCurves3::Coefficient&
		SGLSplineCurves3::CoefficientAt( size_t i ) const
{
	ESLAssert( i <= 3 ) ;
	return	m_params[i] ;
}

// 補完値計算
//////////////////////////////////////////////////////////////////////////////
double SGLSplineCurves3::InterpolateAt( size_t i, double t ) const
{
	if ( i <= 3 )
	{
		const Coefficient&	p = m_params[i] ;
		t -= p.t ;
		return	((p.a * t + p.b) * t + p.c) * t + p.d ;
	}
	return	m_params[3].d ;
}

double SGLSplineCurves3::Interpolate( double t ) const
{
	const Coefficient *	p = m_params ;
	ssize_t	iFirst = 0 ;
	ssize_t	iEnd = 4 ;
	while ( iFirst + 1 < iEnd )
	{
		ssize_t	i = (iFirst + iEnd) >> 1 ;
		if ( t < p[i].t )
		{
			ESLAssert( iEnd != i ) ;
			iEnd = i ;
		}
		else
		{
			ESLAssert( iFirst != i ) ;
			iFirst = i ;
		}
	}
	return	InterpolateAt( iFirst, t ) ;
}



//////////////////////////////////////////////////////////////////////////////
// スプライン補完（6点5区間限定）
//////////////////////////////////////////////////////////////////////////////

// 5次スプライン補完係数計算
//////////////////////////////////////////////////////////////////////////////
void SGLSplineCurves5::CreateSpline( const double * p, const double * t )
{
	const int	N = 5 ;		// 区間数
	//
	// 二次導関数値 u(i) を求める
	//
	SGL4DMatrix<double>
		Q( (t[2]-t[0])*2.0,  t[2] - t[1],      0.0,              0.0,
			t[2] - t[1],     (t[3]-t[1])*2.0,  t[3] - t[2],      0.0,
			0.0,             t[3] - t[2],      (t[4]-t[2])*2.0,  t[4] - t[3],
			0.0,             0.0,              t[4] - t[3],      (t[5]-t[3])*2.0 ) ;
	//
	SGL4DMatrix<double>	R ;
	R.InverseOf( Q ) ;
	//
	double	v[6] ;
	v[0] = 0 ;
	v[3] = 0 ;
	for ( int i = 1; i < N; i ++ )
	{
		v[i] = ((p[i + 1] - p[i]) / (t[i + 1] - t[i])
				- (p[i] - p[i - 1]) / (t[i] - t[i - 1])) * 6.0 ;
	}
	SGL4DVector<double>
			u1 = R * SGL4DVector<double>( v[1], v[2], v[3], v[4] ) ;
	//
	double	u[6] ;
	u[0] = 0.0 ;
	u[1] = u1.x ;
	u[2] = u1.y ;
	u[3] = u1.z ;
	u[4] = u1.w ;
	u[5] = 0.0 ;
	//
	// 区間毎の係数を計算
	//
	Coefficient	coef ;
	for ( int i = 0; i < N; i ++ )
	{
		coef.a = (u[i + 1] - u[i]) / ((t[i + 1] - t[i]) * 6.0) ;
		coef.b = u[i] / 2.0 ;
		coef.c = (p[i + 1] - p[i]) / (t[i + 1] - t[i])
				- (t[i + 1] - t[i]) * (2.0 * u[i] + u[i + 1]) / 6.0 ;
		coef.d = p[i] ;
		coef.t = t[i] ;
		m_params[i] = coef ;
	}
	coef.a = 0.0 ;
	coef.b = 0.0 ;
	coef.c = 0.0 ;
	coef.d = p[N] ;
	coef.t = t[N] ;
	m_params[N] = coef ;
}

// 係数取得
//////////////////////////////////////////////////////////////////////////////
const SGLSplineCurves5::Coefficient& SGLSplineCurves5::CoefficientAt( size_t i ) const
{
	ESLAssert( i <= 5 ) ;
	return	m_params[i] ;
}

// 補完値計算
//////////////////////////////////////////////////////////////////////////////
double SGLSplineCurves5::InterpolateAt( size_t i, double t ) const
{
	if ( i <= 5 )
	{
		const Coefficient&	p = m_params[i] ;
		t -= p.t ;
		return	((p.a * t + p.b) * t + p.c) * t + p.d ;
	}
	return	m_params[5].d ;
}

double SGLSplineCurves5::Interpolate( double t ) const
{
	const Coefficient *	p = m_params ;
	ssize_t	iFirst = 0 ;
	ssize_t	iEnd = 6 ;
	while ( iFirst + 1 < iEnd )
	{
		ssize_t	i = (iFirst + iEnd) >> 1 ;
		if ( t < p[i].t )
		{
			ESLAssert( iEnd != i ) ;
			iEnd = i ;
		}
		else
		{
			ESLAssert( iFirst != i ) ;
			iFirst = i ;
		}
	}
	return	InterpolateAt( iFirst, t ) ;
}


void SGLSplineCurvesN::CreateSpline
	( const double * p, const double * t, size_t nCount )
{
	if ( nCount < 6 )
	{
		SGLSplineCurves::CreateSpline( p, t, nCount ) ;
		return ;
	}
	SGLSplineCurves5	spline5 ;
	m_params.SetLength( nCount ) ;
	for ( size_t i = 0; i + 5 < nCount; i ++ )
	{
		spline5.CreateSpline( p + i, t + i ) ;
		//
		if ( i == 0 )
		{
			m_params.At(0) = spline5.CoefficientAt(0) ;
			m_params.At(1) = spline5.CoefficientAt(1) ;
		}
		m_params.At(i + 2) = spline5.CoefficientAt(2) ;
		m_params.At(i + 3) = spline5.CoefficientAt(3) ;
		m_params.At(i + 4) = spline5.CoefficientAt(4) ;
		m_params.At(i + 5) = spline5.CoefficientAt(5) ;
	}
}



//////////////////////////////////////////////////////////////////////////////
// ２次元スプライン補完
//////////////////////////////////////////////////////////////////////////////

// 3次スプライン補完係数計算
//////////////////////////////////////////////////////////////////////////////
void S2DSplineCurves::CreateSpline
	( const S2DDVector * p, const double * t, size_t nCount )
{
	SArray<double>	aTemp ;
	double *	pX = aTemp.GetArray( nCount ) ;
	//
	for ( size_t i = 0; i < nCount; i ++ )
	{
		pX[i] = p[i].x ;
	}
	m_splineX.CreateSpline( pX, t, nCount ) ;
	//
	for ( size_t i = 0; i < nCount; i ++ )
	{
		pX[i] = p[i].y ;
	}
	m_splineY.CreateSpline( pX, t, nCount ) ;
	//
	aTemp.FinishArray() ;
}

// 補完値計算
//////////////////////////////////////////////////////////////////////////////
S2DDVector S2DSplineCurves::InterpolateAt( size_t i, double t ) const
{
	double	x = m_splineX.InterpolateAt( i, t ) ;
	double	y = m_splineY.InterpolateAt( i, t ) ;
	return	S2DDVector( x, y ) ;
}

S2DDVector S2DSplineCurves::Interpolate( double t ) const
{
	double	x = m_splineX.Interpolate( t ) ;
	double	y = m_splineY.Interpolate( t ) ;
	return	S2DDVector( x, y ) ;
}


//////////////////////////////////////////////////////////////////////////////
// ３次元スプライン補完
//////////////////////////////////////////////////////////////////////////////

// 3次スプライン補完係数計算
//////////////////////////////////////////////////////////////////////////////
void S3DSplineCurves::CreateSpline
	( const S3DDVector * p, const double * t, size_t nCount )
{
	SArray<double>	aTemp ;
	double *	pX = aTemp.GetArray( nCount ) ;
	//
	for ( size_t i = 0; i < nCount; i ++ )
	{
		pX[i] = p[i].x ;
	}
	m_splineX.CreateSpline( pX, t, nCount ) ;
	//
	for ( size_t i = 0; i < nCount; i ++ )
	{
		pX[i] = p[i].y ;
	}
	m_splineY.CreateSpline( pX, t, nCount ) ;
	//
	for ( size_t i = 0; i < nCount; i ++ )
	{
		pX[i] = p[i].z ;
	}
	m_splineZ.CreateSpline( pX, t, nCount ) ;
	//
	aTemp.FinishArray() ;
}

// 補完値計算
//////////////////////////////////////////////////////////////////////////////
S3DDVector S3DSplineCurves::InterpolateAt( size_t i, double t ) const
{
	double	x = m_splineX.InterpolateAt( i, t ) ;
	double	y = m_splineY.InterpolateAt( i, t ) ;
	double	z = m_splineZ.InterpolateAt( i, t ) ;
	return	S3DDVector( x, y, z ) ;
}

S3DDVector S3DSplineCurves::Interpolate( double t ) const
{
	double	x = m_splineX.Interpolate( t ) ;
	double	y = m_splineY.Interpolate( t ) ;
	double	z = m_splineZ.Interpolate( t ) ;
	return	S3DDVector( x, y, z ) ;
}



