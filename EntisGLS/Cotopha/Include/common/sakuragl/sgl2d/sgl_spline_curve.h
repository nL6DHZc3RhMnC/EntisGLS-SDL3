
#if	!defined(__SAKURAGL_SGL_SPLINE_CURVE_H__)
#define	__SAKURAGL_SGL_SPLINE_CURVE_H__	1

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// NxN 一般行列式
	//////////////////////////////////////////////////////////////////////////

	class GenericMatrix : public SSystem::SArray<double>
	{
	public:
		int	m_N ;
	public:
		// 構築
		GenericMatrix( void )
		{
			m_N = 0 ;
		}
		GenericMatrix( int N )
		{
			m_N = N ;
			SetLength( (size_t) (N * N) ) ;
		}
		GenericMatrix( const GenericMatrix& mat )
			: SArray<double>( mat )
		{
			m_N = mat.m_N ;
		}
		// 代入
		const GenericMatrix& operator = ( const GenericMatrix& mat )
		{
			SArray<double>::operator = ( mat ) ;
			m_N = mat.m_N ;
			return	*this ;
		}
		// 行列式
		double Determinant( void ) const ;
		// 逆行列
		const GenericMatrix& InverseOf( const GenericMatrix& mat ) ;
		// ベクトル積
		void ProductVector( double * pDst, const double * pSrc ) const ;
		// 要素
		double GetAt( int iLine, int jCol ) const
		{
			ESLAssert( (iLine >= 0) && (iLine < m_N) ) ;
			ESLAssert( (jCol >= 0) && (jCol < m_N) ) ;
			return	SArray<double>::m_ptrArray[iLine * m_N + jCol] ;
		}
		void SetAt( int iLine, int jCol, double m )
		{
			ESLAssert( (iLine >= 0) && (iLine < m_N) ) ;
			ESLAssert( (jCol >= 0) && (jCol < m_N) ) ;
			SArray<double>::m_ptrArray[iLine * m_N + jCol] = m ;
		}

	} ;


	//////////////////////////////////////////////////////////////////////////
	// NxN 参照行列
	//////////////////////////////////////////////////////////////////////////

	class ReferenceMatrix
	{
	public:
		double *	m_mat ;
		size_t		m_refN ;
		size_t		m_N ;
		size_t *	m_lines ;
		size_t *	m_cols ;

	public:
		// 構築関数
		ReferenceMatrix( void )
			: m_mat( NULL ), m_refN( 0 ),
				m_N( 0 ), m_lines( NULL ), m_cols( NULL ) { }
		ReferenceMatrix( GenericMatrix& mat ) ;
		// 消滅関数
		~ReferenceMatrix( void )
		{
			delete []	m_lines ;
		}
		// 行と列を除去（余因子計算用）
		const ReferenceMatrix& CofactorOf
				( const GenericMatrix& mat, int iLine, int jCol ) ;
		const ReferenceMatrix& CofactorOf
				( const ReferenceMatrix& mat, int iLine, int jCol ) ;
		// 行列式
		double Determinant( void ) const ;
		// 要素
		double GetAt( int iLine, int jCol ) const
		{
			ESLAssert( (size_t) iLine < m_N ) ;
			ESLAssert( (size_t) jCol < m_N ) ;
			return	m_mat[m_lines[iLine] + m_cols[jCol]] ;
		}
		void SetAt( int iLine, int jCol, double m )
		{
			ESLAssert( (size_t) iLine < m_N ) ;
			ESLAssert( (size_t) jCol < m_N ) ;
			m_mat[m_lines[iLine] + m_cols[jCol]] = m ;
		}
	} ;


	//////////////////////////////////////////////////////////////////////////
	// スプライン補完
	//////////////////////////////////////////////////////////////////////////

	class	SGLSplineCurves
	{
	public:
		struct	Coefficient
		{
			double	a, b, c, d, t ;
		} ;

	protected:
		SSystem::SArray<Coefficient>	m_params ;

	public:
		// 3次スプライン補完係数計算
		void CreateSpline
			( const double * p, const double * t, size_t nCount ) ;
		// 係数取得
		const Coefficient& CoefficientAt( size_t i ) const ;
		// 補完値計算
		double InterpolateAt( size_t i, double t ) const ;
		double Interpolate( double t ) const ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// スプライン補完（4点3区間限定）
	//////////////////////////////////////////////////////////////////////////

	class	SGLSplineCurves3
	{
	public:
		typedef	SGLSplineCurves::Coefficient	Coefficient ;

	protected:
		Coefficient	m_params[4] ;

	public:
		// 3次スプライン補完係数計算
		void CreateSpline( const double * p, const double * t ) ;
		// 係数取得
		const Coefficient& CoefficientAt( size_t i ) const ;
		// 補完値計算
		double InterpolateAt( size_t i, double t ) const ;
		double Interpolate( double t ) const ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// スプライン補完（6点5区間限定）
	//////////////////////////////////////////////////////////////////////////

	class	SGLSplineCurves5
	{
	public:
		typedef	SGLSplineCurves::Coefficient	Coefficient ;

	protected:
		Coefficient	m_params[6] ;

	public:
		// 3次スプライン補完係数計算
		void CreateSpline( const double * p, const double * t ) ;
		// 係数取得
		const Coefficient& CoefficientAt( size_t i ) const ;
		// 補完値計算
		double InterpolateAt( size_t i, double t ) const ;
		double Interpolate( double t ) const ;
	} ;

	class	SGLSplineCurvesN	: public SGLSplineCurves
	{
	public:
		// 3次スプライン補完係数計算
		void CreateSpline
			( const double * p, const double * t, size_t nCount ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// ２次元スプライン補完
	//////////////////////////////////////////////////////////////////////////

	class	S2DSplineCurves
	{
	protected:
		SGLSplineCurvesN	m_splineX ;
		SGLSplineCurvesN	m_splineY ;

	public:
		// 3次スプライン補完係数計算
		void CreateSpline
			( const S2DDVector * p, const double * t, size_t nCount ) ;
		// 補完値計算
		S2DDVector InterpolateAt( size_t i, double t ) const ;
		S2DDVector Interpolate( double t ) const ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// ３次元スプライン補完
	//////////////////////////////////////////////////////////////////////////

	class	S3DSplineCurves
	{
	protected:
		SGLSplineCurvesN	m_splineX ;
		SGLSplineCurvesN	m_splineY ;
		SGLSplineCurvesN	m_splineZ ;

	public:
		// 3次スプライン補完係数計算
		void CreateSpline
			( const S3DDVector * p, const double * t, size_t nCount ) ;
		// 補完値計算
		S3DDVector InterpolateAt( size_t i, double t ) const ;
		S3DDVector Interpolate( double t ) const ;

	} ;


}

#endif

