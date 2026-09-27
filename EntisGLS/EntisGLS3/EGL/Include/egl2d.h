
/*****************************************************************************
                          Entis Graphic Library
 -----------------------------------------------------------------------------
     Copyright (c) 2002-2012 Leshade Entis, Entis-soft. Al rights reserved.
 *****************************************************************************/


#if	!defined(__EGL2D_H__)
#define	__EGL2D_H__

//#include <mmintrin.h>		// MMX
//#include <xmmintrin.h>	// SSE
//#include <emmintrin.h>	// SSE2
#include <math.h>

//
// パレット共用体
//
union	EGL_PALETTE
{
	DWORD	dwPixelCode ;			// パレット番号／ピクセルコード
	REAL32	rZOrder32 ;				// Z 値
	struct
	{
		BYTE	Blue ;
		BYTE	Green ;
		BYTE	Red ;
		BYTE	Reserved ;
	}		rgb ;					// RGB24 フォーマット
	struct
	{
		BYTE	Blue ;
		BYTE	Green ;
		BYTE	Red ;
		BYTE	Alpha ;
	}		rgba ;					// RGBA32 フォーマット
	struct
	{
		BYTE	Y ;
		BYTE	U ;
		BYTE	V ;
		BYTE	Reserved ;
	}		yuv ;					// YUV24 フォーマット
	struct
	{
		BYTE	Brightness ;
		BYTE	Saturation ;
		BYTE	Hue ;
		BYTE	Reserved ;
	}		hsb ;					// HSB24 フォーマット

	bool operator == ( const EGL_PALETTE & rgbColor )
		{
			return	dwPixelCode == rgbColor.dwPixelCode ;
		}
	bool operator != ( const EGL_PALETTE & rgbColor )
		{
			return	dwPixelCode != rgbColor.dwPixelCode ;
		}
	const EGL_PALETTE & operator += ( const EGL_PALETTE & rgbColor )
		{
			DWORD	aria = (dwPixelCode & 0x00FF00FF)
							+ (rgbColor.dwPixelCode & 0x00FF00FF) ;
			DWORD	maria = ((dwPixelCode >> 8) & 0x00FF00FF)
							+ ((rgbColor.dwPixelCode >> 8) & 0x00FF00FF) ;
			DWORD	arisa = (((0x00FF00FF - aria) & 0xFF00FF00) >> 8)
								| ((0x00FF00FF - maria) & 0xFF00FF00) ;
			dwPixelCode =
				(aria & 0x00FF00FF) | ((maria & 0x00FF00FF) << 8) | arisa ;
			return	*this ;
		}
	const EGL_PALETTE & operator *= ( int x )
		{
			ESLAssert( x <= 0x100 ) ;
			DWORD	aria = (dwPixelCode & 0x00FF00FF) * x ;
			DWORD	maria = ((dwPixelCode >> 8) & 0x00FF00FF) * x ;
			dwPixelCode =
				((aria & 0xFF00FF00) >> 8) | (maria & 0xFF00FF00) ;
			return	*this ;
		}
	const EGL_PALETTE & operator *= ( unsigned int x )
		{
			ESLAssert( x <= 0x100 ) ;
			DWORD	aria = (dwPixelCode & 0x00FF00FF) * x ;
			DWORD	maria = ((dwPixelCode >> 8) & 0x00FF00FF) * x ;
			dwPixelCode =
				((aria & 0xFF00FF00) >> 8) | (maria & 0xFF00FF00) ;
			return	*this ;
		}
	const EGL_PALETTE & operator *= ( double y )
		{
			unsigned int	x = (unsigned int) (int) (y * 256.0) ;
			ESLAssert( x <= 0x100 ) ;
			DWORD	aria = (dwPixelCode & 0x00FF00FF) * x ;
			DWORD	maria = ((dwPixelCode >> 8) & 0x00FF00FF) * x ;
			dwPixelCode =
				((aria & 0xFF00FF00) >> 8) | (maria & 0xFF00FF00) ;
			return	*this ;
		}
	EGL_PALETTE operator + ( const EGL_PALETTE & rgbColor ) const
		{
			EGL_PALETTE	t = *this ;
			t += rgbColor ;
			return	t ;
		}
	EGL_PALETTE operator * ( int x ) const
		{
			EGL_PALETTE	t = *this ;
			t *= x ;
			return	t ;
		}
	EGL_PALETTE operator * ( unsigned int x ) const
		{
			EGL_PALETTE	t = *this ;
			t *= x ;
			return	t ;
		}
	EGL_PALETTE operator * ( double x ) const
		{
			EGL_PALETTE	t = *this ;
			t *= x ;
			return	t ;
		}
} ;

class	EGLPalette
{
public:
	EGL_PALETTE	m_palette ;
public:
	EGLPalette( void ) { }
	EGLPalette( const EGL_PALETTE & palette )
		{ m_palette = palette ; }
	EGLPalette( DWORD dwRGBA )
		{ m_palette.dwPixelCode = dwRGBA ; }
	EGLPalette( int nBlue, int nGreen, int nRed, int nAlpha = 0 )
		{
			m_palette.rgba.Blue  = (BYTE) nBlue ;
			m_palette.rgba.Green = (BYTE) nGreen ;
			m_palette.rgba.Red   = (BYTE) nRed ;
			m_palette.rgba.Alpha = (BYTE) nAlpha ;
		}
	operator EGL_PALETTE ( void ) const
		{	return	m_palette ;	}
	operator EGL_PALETTE & ( void )
		{	return	m_palette ;	}
	operator const EGL_PALETTE * ( void ) const
		{	return	&m_palette ;	}
	operator EGL_PALETTE * ( void )
		{	return	&m_palette ;	}
	const EGLPalette & operator = ( const EGL_PALETTE & palette )
		{
			m_palette = palette ;
			return	*this ;
		}
	const EGLPalette & operator += ( const EGL_PALETTE & rgbColor )
		{
			DWORD	aria = (m_palette.dwPixelCode & 0x00FF00FF)
							+ (rgbColor.dwPixelCode & 0x00FF00FF) ;
			DWORD	maria = ((m_palette.dwPixelCode >> 8) & 0x00FF00FF)
							+ ((rgbColor.dwPixelCode >> 8) & 0x00FF00FF) ;
			DWORD	arisa = (((0x00FF00FF - aria) & 0xFF00FF00) >> 8)
								| ((0x00FF00FF - maria) & 0xFF00FF00) ;
			m_palette.dwPixelCode =
				(aria & 0x00FF00FF) | ((maria & 0x00FF00FF) << 8) | arisa ;
			return	*this ;
		}
	const EGLPalette & operator *= ( unsigned int x )
		{
			ESLAssert( x <= 0x100 ) ;
			DWORD	aria = (m_palette.dwPixelCode & 0x00FF00FF) * x ;
			DWORD	maria = ((m_palette.dwPixelCode >> 8) & 0x00FF00FF) * x ;
			m_palette.dwPixelCode =
				((aria & 0xFF00FF00) >> 8) | (maria & 0xFF00FF00) ;
			return	*this ;
		}
	EGLPalette operator + ( const EGL_PALETTE & rgbColor ) const
		{
			EGLPalette	t = *this ;
			t += rgbColor ;
			return	t ;
		}
	EGLPalette operator * ( unsigned int x ) const
		{
			EGLPalette	t = *this ;
			t *= x ;
			return	t ;
		}
} ;

typedef	EGL_PALETTE *	PEGL_PALETTE ;
typedef	const EGL_PALETTE *	PCEGL_PALETTE ;

//
// 画像情報構造体
//
struct	EGL_IMAGE_INFO
{
	DWORD				dwInfoSize ;
	DWORD				fdwFormatType ;
	DWORD				ptrOffsetPixel ;
	void *				ptrImageArray ;
	PEGL_PALETTE		pPaletteEntries ;
	DWORD				dwPaletteCount ;
	DWORD				dwImageWidth ;
	DWORD				dwImageHeight ;
	DWORD				dwBitsPerPixel ;
	SDWORD				dwBytesPerLine ;
	SDWORD				dwSizeOfImage ;
	DWORD				dwClippedPixel ;
} ;
typedef	EGL_IMAGE_INFO *	PEGL_IMAGE_INFO ;
typedef	const EGL_IMAGE_INFO *	PCEGL_IMAGE_INFO ;

#define	EIF_RGB_BITMAP		0x00000001
#define	EIF_RGBA_BITMAP		0x04000001
#define	EIF_GRAY_BITMAP		0x00000002
#define	EIF_YUV_BITMAP		0x00000004
#define	EIF_HSB_BITMAP		0x00000006
#define	EIF_Z_BUFFER_R4		0x00002005
#define	EIF_TYPE_MASK		0x00FFFFFF
#define	EIF_WITH_PALETTE	0x01000000
#define	EIF_WITH_CLIPPING	0x02000000
#define	EIF_WITH_ALPHA		0x04000000
#define	EIF_SIDE_BY_SIDE	0x10000000		// ステレオ画像形式
											// バッファ情報では右画像のみを参照する

//
// 座標・サイズ・矩形
//
struct	EGL_POINT
{
	SDWORD	x ;
	SDWORD	y ;
} ;
typedef	EGL_POINT *	PEGL_POINT ;
typedef	const EGL_POINT *	PCEGL_POINT ;

struct	EGL_SIZE
{
	SDWORD	w ;
	SDWORD	h ;
} ;
typedef	EGL_SIZE *	PEGL_SIZE ;
typedef	const EGL_SIZE *	PCEGL_SIZE ;

class	EGLPoint	: public	EGL_POINT
{
public:
	EGLPoint( void ) { }
	EGLPoint( SDWORD xPos, SDWORD yPos ) { x = xPos ;  y = yPos ; }
	EGLPoint( const EGL_POINT & point ) { x = point.x ;  y = point.y ; }
	EGLPoint( const EGL_SIZE & size ) { x = size.w ;  y = size.h ; }
	const EGLPoint & operator = ( const EGL_POINT & point )
		{
			x = point.x ;
			y = point.y ;
			return	*this ;
		}
	bool operator == ( const EGL_POINT & point ) const
		{
			return	(x == point.x) && (y == point.y) ;
		}
	bool operator != ( const EGL_POINT & point ) const
		{
			return	(x != point.x) || (y != point.y) ;
		}
	const EGLPoint & operator += ( const EGL_POINT & point )
		{
			x += point.x ;
			y += point.y ;
			return	*this ;
		}
	const EGLPoint & operator -= ( const EGL_POINT & point )
		{
			x -= point.x ;
			y -= point.y ;
			return	*this ;
		}
	EGLPoint operator + ( const EGL_POINT & point ) const
		{
			EGLPoint	ptResult = *this ;
			ptResult += point ;
			return	ptResult ;
		}
	EGLPoint operator - ( const EGL_POINT & point ) const
		{
			EGLPoint	ptResult = *this ;
			ptResult -= point ;
			return	ptResult ;
		}
	EGLPoint operator - ( void ) const
		{
			return	EGLPoint( - x, - y ) ;
		}
} ;

class	EGLSize	: public	EGL_SIZE
{
public:
	EGLSize( void ) { }
	EGLSize( SDWORD width, SDWORD height ) { w = width ;  h = height ; }
	EGLSize( const EGL_SIZE & size ) { w = size.w ;  h = size.h ; }
	EGLSize( const EGL_POINT & point ) { w = point.x ;  h = point.y ; }
	const EGLSize & operator = ( const EGL_SIZE & size )
		{
			w = size.w ;
			h = size.h ;
			return	*this ;
		}
	bool operator == ( const EGL_SIZE & size ) const
		{
			return	(w == size.w) && (h == size.h) ;
		}
	bool operator != ( const EGL_SIZE & size ) const
		{
			return	(w != size.w) || (h != size.h) ;
		}
	const EGLSize & operator += ( const EGL_SIZE & size )
		{
			w += size.w ;
			h += size.h ;
			return	*this ;
		}
	const EGLSize & operator -= ( const EGL_SIZE & size )
		{
			w -= size.w ;
			h -= size.h ;
			return	*this ;
		}
	EGLSize operator + ( const EGL_SIZE & size ) const
		{
			EGLSize	sizeResult = *this ;
			sizeResult += size ;
			return	sizeResult ;
		}
	EGLSize operator - ( const EGL_SIZE & size ) const
		{
			EGLSize	sizeResult = *this ;
			sizeResult -= size ;
			return	sizeResult ;
		}
} ;

struct	EGL_RECT
{
	SDWORD	left ;
	SDWORD	top ;
	SDWORD	right ;
	SDWORD	bottom ;
} ;
typedef	EGL_RECT *	PEGL_RECT ;
typedef	const EGL_RECT *	PCEGL_RECT ;

struct	EGL_IMAGE_RECT
{
	SDWORD	x ;
	SDWORD	y ;
	SDWORD	w ;
	SDWORD	h ;
} ;
typedef	EGL_IMAGE_RECT *	PEGL_IMAGE_RECT ;
typedef	const EGL_IMAGE_RECT *	PCEGL_IMAGE_RECT ;

class	EGLRect	: public	EGL_RECT
{
public:
	EGLRect( void ) { }
	EGLRect( SDWORD x1, SDWORD y1, SDWORD x2, SDWORD y2 )
		{
			left = x1 ;
			top = y1 ;
			right = x2 ;
			bottom = y2 ;
		}
	EGLRect( const EGL_RECT & rect )
		{
			left = rect.left ;
			top = rect.top ;
			right = rect.right ;
			bottom = rect.bottom ;
		}
	EGLRect( const struct EGL_IMAGE_RECT & rect )
		{
			left = rect.x ;
			top = rect.y ;
			right = rect.x + rect.w - 1 ;
			bottom = rect.y + rect.h - 1 ;
		}
	const EGLRect & operator = ( const EGL_RECT & rect )
		{
			left = rect.left ;
			top = rect.top ;
			right = rect.right ;
			bottom = rect.bottom ;
			return	*this ;
		}
	const EGLRect & operator = ( const struct EGL_IMAGE_RECT & rect )
		{
			left = rect.x ;
			top = rect.y ;
			right = rect.x + rect.w - 1 ;
			bottom = rect.y + rect.h - 1 ;
			return	*this ;
		}
	bool operator &= ( const EGL_RECT & rect )
		{
			if ( (left > rect.right) || (top > rect.bottom)
					|| (right < rect.left) || (bottom < rect.top) )
			{
				left = top = 0 ;
				right = bottom = -1 ;
				return	false ;
			}
			else
			{
				if ( left < rect.left )
					left = rect.left ;
				if ( top < rect.top )
					top = rect.top ;
				if ( right > rect.right )
					right = rect.right ;
				if ( bottom > rect.bottom )
					bottom = rect.bottom ;
				return	true ;
			}
		}
	void operator |= ( const EGL_RECT & rect )
		{
			if ( left > rect.left )
			{
				left = rect.left ;
			}
			if ( top > rect.top )
			{
				top = rect.top ;
			}
			if ( right < rect.right )
			{
				right = rect.right ;
			}
			if ( bottom < rect.bottom )
			{
				bottom = rect.bottom ;
			}
		}
	int operator == (const EGL_RECT & rect ) const
		{
			return	(left == rect.left) & (top == rect.top)
				& (right == rect.right) & (bottom == rect.bottom) ;
		}
	int operator != (const EGL_RECT & rect ) const
		{
			return	(left != rect.left) | (top != rect.top)
				| (right != rect.right) | (bottom != rect.bottom) ;
		}
	bool IsEmpty( void ) const
		{
			return	(left > right) || (top > bottom) ;
		}
	void Clear( void )
		{
			left = 0 ;
			top = 0 ;
			right = -1 ;
			bottom = -1 ;
		}
} ;

class	EGLImageRect	: public	EGL_IMAGE_RECT
{
public:
	EGLImageRect( void ) { }
	EGLImageRect( SDWORD xPos, SDWORD yPos, SDWORD nWidth, SDWORD nHeight )
		{
			x = xPos ;
			y = yPos ;
			w = nWidth ;
			h = nHeight ;
		}
	EGLImageRect( const EGL_IMAGE_RECT & rect )
		{
			x = rect.x ;
			y = rect.y ;
			w = rect.w ;
			h = rect.h ;
		}
	EGLImageRect( const EGL_RECT & rect )
		{
			x = rect.left ;
			y = rect.top ;
			w = rect.right - rect.left + 1 ;
			h = rect.bottom - rect.top + 1 ;
		}
	const EGLImageRect & operator = ( const EGL_IMAGE_RECT & rect )
		{
			x = rect.x ;
			y = rect.y ;
			w = rect.w ;
			h = rect.h ;
			return	*this ;
		}
	const EGLImageRect & operator = ( const EGL_RECT & rect )
		{
			x = rect.left ;
			y = rect.top ;
			w = rect.right - rect.left + 1 ;
			h = rect.bottom - rect.top + 1 ;
			return	*this ;
		}
} ;

struct	EGL_IMAGE_AXES
{
	struct
	{
		REAL32	x ;
		REAL32	y ;
	}			xAxis ;
	struct
	{
		REAL32	x ;
		REAL32	y ;
	}			yAxis ;
} ;
typedef	EGL_IMAGE_AXES *	PEGL_IMAGE_AXES ;
typedef	const EGL_IMAGE_AXES *	PCEGL_IMAGE_AXES ;

//
// 2次元ベクトル
//
struct	E3D_VECTOR_2D
{
public:
	REAL32	x ;
	REAL32	y ;
public:
	bool operator == ( const E3D_VECTOR_2D & vector ) const
		{
			return	(x == vector.x) && (y == vector.y) ;
		}
	bool operator != ( const E3D_VECTOR_2D & vector ) const
		{
			return	(x != vector.x) && (y != vector.y) ;
		}
	const E3D_VECTOR_2D & operator += ( const E3D_VECTOR_2D & vector )
		{
			x += vector.x ;
			y += vector.y ;
			return	*this ;
		}
	const E3D_VECTOR_2D & operator -= ( const E3D_VECTOR_2D & vector )
		{
			x -= vector.x ;
			y -= vector.y ;
			return	*this ;
		}
	const E3D_VECTOR_2D & operator *= ( double num )
		{
			x *= (REAL32) num ;
			y *= (REAL32) num ;
			return	*this ;
		}
	E3D_VECTOR_2D operator + ( const E3D_VECTOR_2D & vector ) const
		{
			E3D_VECTOR_2D	result = *this ;
			result += vector ;
			return	result ;
		}
	E3D_VECTOR_2D operator - ( void ) const
		{
			E3D_VECTOR_2D	result = { - x, - y } ;
			return	result ;
		}
	E3D_VECTOR_2D operator - ( const E3D_VECTOR_2D & vector ) const
		{
			E3D_VECTOR_2D	result = *this ;
			result -= vector ;
			return	result ;
		}
	E3D_VECTOR_2D operator * ( double num ) const
		{
			E3D_VECTOR_2D	result ;
			result.x = (REAL32) (x * num) ;
			result.y = (REAL32) (y * num) ;
			return	result ;
		}
	E3D_VECTOR_2D operator * ( const E3D_VECTOR_2D & vector ) const
		{
			E3D_VECTOR_2D	result ;
			result.x = x * vector.x - y * vector.y ;
			result.y = x * vector.y + y * vector.x ;
			return	result ;
		}
	E3D_VECTOR_2D operator / ( const E3D_VECTOR_2D & vector ) const
		{
			E3D_VECTOR_2D	result ;
			REAL32	temp_num
				= vector.x * vector.x + vector.y * vector.y ;
			result.x = (x * vector.x + y * vector.y) / temp_num ;
			result.y = (y * vector.x - y * vector.x) / temp_num ;
			return	result ;
		}
	const E3D_VECTOR_2D & operator *= ( const E3D_VECTOR_2D & vector )
		{
			return	(*this = *this * vector) ;
		}
	const E3D_VECTOR_2D & operator /= ( const E3D_VECTOR_2D & vector )
		{
			return	(*this = *this / vector) ;
		}
} ;

class	E3DVector2D	: public	E3D_VECTOR_2D
{
public:
	E3DVector2D( void ) { }
	E3DVector2D( double vx, double vy ) { x = (REAL32) vx;  y = (REAL32) vy ; }
	E3DVector2D( const E3D_VECTOR_2D & v ) { x = v.x ;  y = v.y ; }
	E3DVector2D( const EGL_POINT & point )
		{ x = (REAL32) point.x ;  y = (REAL32) point.y ; }
	const E3DVector2D & operator = ( const E3D_VECTOR_2D & vector )
		{
			x = vector.x ;
			y = vector.y ;
			return	*this ;
		}

} ;

typedef	E3D_VECTOR_2D *			PE3D_VECTOR_2D ;
typedef	const E3D_VECTOR_2D *	PCE3D_VECTOR_2D ;

//
// シェーディング色情報
//
struct	E3D_COLOR
{
	EGL_PALETTE	rgbMul ;
	EGL_PALETTE	rgbAdd ;
} ;
typedef	E3D_COLOR *	PE3D_COLOR ;
typedef	const E3D_COLOR *	PCE3D_COLOR ;

//
// 16ビット要素ベクトル
//
struct	E3D_VECTOR4_SW
{
	SWORD	x ;
	SWORD	y ;
	SWORD	z ;
	SWORD	d ;
} ;

//
// リージョン
//
struct	E3D_POLY_LINE_REGION
{
	SDWORD			nLeft ;
	DWORD			dwReserved1 ;
	SDWORD			nRight ;
	DWORD			dwReserved2 ;
	E3D_COLOR		rgbaLeft ;
	E3D_COLOR		rgbaRight ;
	E3D_VECTOR4_SW	vLeft ;
	E3D_VECTOR4_SW	vRight ;
} ;

struct	E3D_POLYGON_REGION
{
	SDWORD					nTopLine ;
	SDWORD					nBottomLine ;
	E3D_POLY_LINE_REGION	plrLineRgn[1] ;
} ;
typedef	E3D_POLYGON_REGION *	PE3D_POLYGON_REGION ;

//
// 基礎算術関数
//
struct	E3D_VECTOR ;
struct	E3D_VECTOR4 ;
struct	E3D_REV_MATRIX ;

extern	"C"
{
	void eglInitializeMathFunctions( void ) ;
	void eglSineCosine( REAL32 * pCosSin, REAL32 rRadian ) ;
	void eglSineCosineSSE( void ) ;
	ESLError eglBaseVectorFromMapping2D
		( PEGL_IMAGE_AXES pBaseVector,
			PE3D_VECTOR_2D pvMappedOrigin,
			PCE3D_VECTOR_2D pvMapped, PCE3D_VECTOR_2D pvMapping ) ;
	ESLError eglMapping2DVectors
		( PCEGL_IMAGE_AXES pBaseVector,
			PCE3D_VECTOR_2D pvMappedOrigin,
			PE3D_VECTOR_2D pvMapped,
			PCE3D_VECTOR_2D pvMapping, unsigned int nCount ) ;
} ;

typedef void (*pfn_eglNegateVector)( E3D_VECTOR * pv ) ;
typedef void (*pfn_eglAddVector)( E3D_VECTOR * pv1, const E3D_VECTOR * pv2 ) ;
typedef void (*pfn_eglSubVector)( E3D_VECTOR * pv1, const E3D_VECTOR * pv2 ) ;
typedef void (*pfn_eglMultipleVector)( E3D_VECTOR * pv, REAL32 r ) ;
typedef void (*pfn_eglDivideVector)( E3D_VECTOR * pv, REAL32 r ) ;
typedef void (*pfn_eglAbsoluteVector)( REAL32 * pabs, const E3D_VECTOR * pv ) ;
typedef void (*pfn_eglVectorExteriorProduct)
	( E3D_VECTOR * pv1, const E3D_VECTOR * pv2 ) ;
typedef void (*pfn_eglVectorInnerProduct)
	( REAL32 * prs,
		const E3D_VECTOR * pv1, const E3D_VECTOR * pv2 ) ;
typedef void (*pfn_eglVectorRoundTo1)( E3D_VECTOR * pv ) ;
typedef void (*pfn_eglMatrixNegate)
	( E3D_REV_MATRIX * matrix ) ;
typedef void (*pfn_eglMatrixAdd)
	( E3D_REV_MATRIX * matDst, const E3D_REV_MATRIX * matSrc ) ;
typedef void (*pfn_eglMatrixSub)
	( E3D_REV_MATRIX * matDst, const E3D_REV_MATRIX * matSrc ) ;
typedef void (*pfn_eglMatrixMultiple)
	( E3D_REV_MATRIX * matrix, REAL32 r ) ;
typedef void (*pfn_eglMatrixDeterminant)
	( REAL32 * pDet, const E3D_REV_MATRIX * matrix ) ;
typedef void (*pfn_eglMatrixInverse)
	( E3D_REV_MATRIX * matDst, const E3D_REV_MATRIX * matSrc ) ;
typedef void (*pfn_eglMatrixRevolveOnX)
	( E3D_REV_MATRIX * matrix, REAL32 rSin, REAL32 rCos ) ;
typedef void (*pfn_eglMatrixRevolveOnY)
	( E3D_REV_MATRIX * matrix, REAL32 rSin, REAL32 rCos ) ;
typedef void (*pfn_eglMatrixRevolveOnZ)
	( E3D_REV_MATRIX * matrix, REAL32 rSin, REAL32 rCos ) ;
typedef void (*pfn_eglMatrixRevolveByAngleOn)
	( E3D_REV_MATRIX * matrix, const E3D_VECTOR * pAngle ) ;
typedef void (*pfn_eglMatrixRevolveForAngle)
	( E3D_REV_MATRIX * matrix, const E3D_VECTOR * pAngle ) ;
typedef void (*pfn_eglMatrixMagnifyByVector)
	( E3D_REV_MATRIX * matrix, const E3D_VECTOR * pv ) ;
typedef void (*pfn_eglMatrixRevolve)
	( const E3D_REV_MATRIX * matrix,
			E3D_REV_MATRIX * matDst ) ;
typedef void (*pfn_eglMatrixRevolveBy)
	( E3D_REV_MATRIX * matDst,
		const E3D_REV_MATRIX * matSrc ) ;
typedef void (*pfn_eglMatrixRevolveVector)
	( const E3D_REV_MATRIX * matrix, E3D_VECTOR * pv ) ;
typedef void (*pfn_eglMatrixRevolveVectors)
	( const E3D_REV_MATRIX * matrix,
		 E3D_VECTOR4 * pvDst, const E3D_VECTOR4 * pvSrc,
				const E3D_VECTOR * pvOrigin, int nCount ) ;
typedef	void (*pfn_eglGetMinVector)
	( E3D_VECTOR4 * pvMin, const E3D_VECTOR4 * pvList, int nCount ) ;
typedef	void (*pfn_eglGetMaxVector)
	( E3D_VECTOR4 * pvMax, const E3D_VECTOR4 * pvList, int nCount ) ;

extern	"C" pfn_eglNegateVector				eglNegateVector ;
extern	"C" pfn_eglAddVector				eglAddVector ;
extern	"C" pfn_eglSubVector				eglSubVector ;
extern	"C" pfn_eglMultipleVector			eglMultipleVector ;
extern	"C" pfn_eglDivideVector				eglDivideVector ;
extern	"C" pfn_eglAbsoluteVector			eglAbsoluteVector ;
extern	"C" pfn_eglVectorExteriorProduct	eglVectorExteriorProduct ;
extern	"C" pfn_eglVectorInnerProduct		eglVectorInnerProduct ;
extern	"C" pfn_eglVectorRoundTo1			eglVectorRoundTo1 ;
extern	"C" pfn_eglMatrixNegate				eglMatrixNegate ;
extern	"C" pfn_eglMatrixAdd				eglMatrixAdd ;
extern	"C" pfn_eglMatrixSub				eglMatrixSub ;
extern	"C" pfn_eglMatrixMultiple			eglMatrixMultiple ;
extern	"C" pfn_eglMatrixDeterminant		eglMatrixDeterminant ;
extern	"C" pfn_eglMatrixInverse			eglMatrixInverse ;
extern	"C" pfn_eglMatrixRevolveOnX			eglMatrixRevolveOnX ;
extern	"C" pfn_eglMatrixRevolveOnY			eglMatrixRevolveOnY ;
extern	"C" pfn_eglMatrixRevolveOnZ			eglMatrixRevolveOnZ ;
extern	"C" pfn_eglMatrixRevolveByAngleOn	eglMatrixRevolveByAngleOn ;
extern	"C" pfn_eglMatrixRevolveForAngle	eglMatrixRevolveForAngle ;
extern	"C" pfn_eglMatrixMagnifyByVector	eglMatrixMagnifyByVector ;
extern	"C" pfn_eglMatrixRevolve			eglMatrixRevolve ;
extern	"C" pfn_eglMatrixRevolveBy			eglMatrixRevolveBy ;
extern	"C" pfn_eglMatrixRevolveVector		eglMatrixRevolveVector ;
extern	"C" pfn_eglMatrixRevolveVectors		eglMatrixRevolveVectors ;
extern	"C" pfn_eglGetMinVector				eglGetMinVector ;
extern	"C" pfn_eglGetMaxVector				eglGetMaxVector ;

//
// 画像バッファ管理
//

struct	EGL_IMAGE_BUFFER_INTERFACE
{
	DWORD							dwType ;	// enum EGLImageBufferObjectType
	void *							ptrObject ;
	EGL_IMAGE_BUFFER_INTERFACE *	pNextInterface ;
	void (*pfnRelease)
		( PEGL_IMAGE_INFO pImageInf,
			EGL_IMAGE_BUFFER_INTERFACE * pInterface ) ;
	ESLError (*pfnUpdateBuffer)
		( PEGL_IMAGE_INFO pImageInf,
			EGL_IMAGE_BUFFER_INTERFACE * pInterface,
			const struct EGL_RECT * pRect ) ;
	ESLError (*pfnCommitBuffer)
		( PEGL_IMAGE_INFO pImageInf,
			EGL_IMAGE_BUFFER_INTERFACE * pInterface ) ;
	ESLError (*pfnReflectBuffer)
		( PEGL_IMAGE_INFO pImageInf,
			EGL_IMAGE_BUFFER_INTERFACE * pInterface,
			const struct EGL_RECT * pRect ) ;
} ;
typedef ESLError (*PFUNC_UPDATE_IMAGE_BUFFER)
	( PEGL_IMAGE_INFO pImageInf, int nAction, const struct EGL_RECT * pRect ) ;

enum	EGLImageBufferObjectType
{
	EGL_BUFOBJ_GL_TEXTURE	= 0x00000000,
	EGL_BUFOBJ_CUDA_MEMORY	= 0x01000000,
	EGL_BUFOBJ_DD_SURFACE1	= 0x02000001,
	EGL_BUFOBJ_DD_SURFACE7	= 0x02000007,
	EGL_BUFOBJ_D3D_SURFACE9	= 0x02000009,
	EGL_BUFOBJ_D3D_TEXTURE9	= 0x03000009,
} ;
struct	EGL_IMAGE_INFO2	: public EGL_IMAGE_INFO
{
	// dwInfoSize が sizeof(EGL_IMAGE_INFO2) 以上の時に使用できるメンバ
	// ※このメンバを直接操作する場合には、将来の互換性が保証されないかもしれない
	EGL_IMAGE_BUFFER_INTERFACE *	pInterface ;
} ;
typedef	EGL_IMAGE_INFO2 *	PEGL_IMAGE_INFO2 ;
typedef	const EGL_IMAGE_INFO2 *	PCEGL_IMAGE_INFO2 ;

extern	"C"
{
	PEGL_IMAGE_INFO eglCreateImageBuffer
		( DWORD fdwFormat,
			DWORD dwWidth, DWORD dwHeight,
			DWORD dwBitsPerPixel, DWORD dwFlags = 0 ) ;
	PEGL_IMAGE_INFO eglCreateTextureInfo
		( PCEGL_IMAGE_INFO pImageInf,
			PCEGL_RECT pClipRect = NULL, DWORD dwFlags = 0 ) ;
	PEGL_IMAGE_INFO eglDuplicateImageBuffer
		( PCEGL_IMAGE_INFO pImageInf, DWORD dwFlags = 0 ) ;
	ESLError eglGetStereoLeftImageBuffer
		( PCEGL_IMAGE_INFO pImageInf, PEGL_IMAGE_INFO pLeftImage ) ;
	ESLError eglAddImageBufferRef( PEGL_IMAGE_INFO pImageInf ) ;
	ESLError eglDeleteImageBuffer( PEGL_IMAGE_INFO pImageInf ) ;
	ESLError eglDrawToDC
		( HDC hDstDC, PCEGL_IMAGE_INFO pImageInf,
			int nPosX, int nPosY,
			PCEGL_SIZE pSizeToDraw, PCEGL_RECT pViewRect ) ;
	HDC eglGetDC( PCEGL_IMAGE_INFO pImageInf ) ;
	ESLError eglFillImage
		( PEGL_IMAGE_INFO pImageInf, EGL_PALETTE colorFill ) ;
	ESLError eglGetClippedImageInfo
		( PEGL_IMAGE_INFO pClippedImage,
			PCEGL_IMAGE_INFO pOriginalImage,
			PCEGL_IMAGE_RECT pClippingRect ) ;
	PEGL_IMAGE_RECT eglGetOverlappedRectangle
		( PEGL_IMAGE_RECT pImageRect,
			PCEGL_RECT pDstViewRect,
			PCEGL_RECT pSrcViewRect,
			PCEGL_POINT pSrcViewOffset,
			PCEGL_SIZE pSrcViewMaxSize = NULL ) ;
	ESLError eglGetRevolvedAxes
		( PEGL_IMAGE_AXES pImageAxes,
			PEGL_POINT pBasePosition,
			PCEGL_POINT pCenterOffset,
			REAL32 rHorizontalRate = 1,
			REAL32 rVerticalRate = 1,
			REAL32 rRevolutionAngle = 0,
			REAL32 rCrossingAngle = 90,
			unsigned int nFlag = 0 ) ;
	ESLError eglReverseVertically( PEGL_IMAGE_INFO pImageInf ) ;
	EGL_PALETTE eglGetPixel
		( PCEGL_IMAGE_INFO pImageInf, int nPosX, int nPosY ) ;
	ESLError eglSetPixel
		( PEGL_IMAGE_INFO pImageInf,
			int nPosX, int nPosY, EGL_PALETTE colorPixel ) ;
} ;

inline ESLError eglUpdateImageBuffer
	( PEGL_IMAGE_INFO pImageInf, const EGL_RECT * pRect )
{
	if ( pImageInf->dwInfoSize < sizeof(EGL_IMAGE_INFO2) )
	{
		return	eslErrGeneral ;
	}
	EGL_IMAGE_BUFFER_INTERFACE *	pInterface =
			((EGL_IMAGE_INFO2*)pImageInf)->pInterface ;
	ESLError	errResult = eslErrSuccess ;
	while ( pInterface != NULL )
	{
		ESLError	err =
			pInterface->pfnUpdateBuffer( pImageInf, pInterface, pRect ) ;
		if ( err )
		{
			errResult = err ;
		}
		pInterface = pInterface->pNextInterface ;
	}
	return	errResult ;
}

inline ESLError eglCommitImageBuffer( PEGL_IMAGE_INFO pImageInf )
{
	if ( pImageInf->dwInfoSize < sizeof(EGL_IMAGE_INFO2) )
	{
		return	eslErrGeneral ;
	}
	EGL_IMAGE_BUFFER_INTERFACE *	pInterface =
			((EGL_IMAGE_INFO2*)pImageInf)->pInterface ;
	ESLError	errResult = eslErrSuccess ;
	while ( pInterface != NULL )
	{
		ESLError	err =
			pInterface->pfnCommitBuffer( pImageInf, pInterface ) ;
		if ( err )
		{
			errResult = err ;
		}
		pInterface = pInterface->pNextInterface ;
	}
	return	errResult ;
}

inline ESLError eglReflectImageBuffer
	( PEGL_IMAGE_INFO pImageInf, const EGL_RECT * pRect )
{
	if ( pImageInf->dwInfoSize < sizeof(EGL_IMAGE_INFO2) )
	{
		return	eslErrGeneral ;
	}
	EGL_IMAGE_BUFFER_INTERFACE *	pInterface =
			((EGL_IMAGE_INFO2*)pImageInf)->pInterface ;
	ESLError	errResult = eslErrSuccess ;
	while ( pInterface != NULL )
	{
		ESLError	err =
			pInterface->pfnReflectBuffer( pImageInf, pInterface, pRect ) ;
		if ( err )
		{
			errResult = err ;
		}
		pInterface = pInterface->pNextInterface ;
	}
	return	errResult ;
}

inline ESLError eglAddBufferUpdateInterface
	( PEGL_IMAGE_INFO pImageInf,
			EGL_IMAGE_BUFFER_INTERFACE * pInterface )
{
	if ( pImageInf->dwInfoSize < sizeof(EGL_IMAGE_INFO2) )
	{
		return	eslErrGeneral ;
	}
	EGL_IMAGE_INFO2 *	pImageInf2 = (EGL_IMAGE_INFO2*) pImageInf ;
	pInterface->pNextInterface = pImageInf2->pInterface ;
	pImageInf2->pInterface = pInterface ;
	return	eslErrSuccess ;
}

inline ESLError eglDetachBufferUpdateInterface
	( PEGL_IMAGE_INFO pImageInf,
			EGL_IMAGE_BUFFER_INTERFACE * pInterface )
{
	if ( pImageInf->dwInfoSize < sizeof(EGL_IMAGE_INFO2) )
	{
		return	eslErrGeneral ;
	}
	EGL_IMAGE_INFO2 *	pImageInf2 = (EGL_IMAGE_INFO2*) pImageInf ;
	EGL_IMAGE_BUFFER_INTERFACE *	pLast = NULL ;
	EGL_IMAGE_BUFFER_INTERFACE *	pNext = pImageInf2->pInterface ;
	while ( pNext != NULL )
	{
		if ( pNext == pInterface )
		{
			if ( pLast != NULL )
			{
				ESLAssert( pLast->pNextInterface == pInterface ) ;
				pLast->pNextInterface = pInterface->pNextInterface ;
			}
			else
			{
				ESLAssert( pImageInf2->pInterface == pInterface ) ;
				pImageInf2->pInterface = pInterface->pNextInterface ;
			}
			pInterface->pNextInterface = NULL ;
			return	eslErrSuccess ; ;
		}
		pLast = pNext ;
		pNext = pNext->pNextInterface ;
	}
	return	eslErrGeneral ;
}

inline EGL_IMAGE_BUFFER_INTERFACE *
	eglGetBufferUpdateInterface
			( PEGL_IMAGE_INFO pImageInf, DWORD dwType )
{
	if ( pImageInf->dwInfoSize < sizeof(EGL_IMAGE_INFO2) )
	{
		return	NULL ;
	}
	EGL_IMAGE_BUFFER_INTERFACE *	pInterface =
			((EGL_IMAGE_INFO2*)pImageInf)->pInterface ;
	while ( pInterface != NULL )
	{
		if ( pInterface->dwType == dwType )
		{
			return	pInterface ;
		}
		pInterface = pInterface->pNextInterface ;
	}
	return	NULL ;
}

inline void * eglGetBufferObject( PEGL_IMAGE_INFO pImageInf, DWORD dwType )
{
	if ( pImageInf->dwInfoSize < sizeof(EGL_IMAGE_INFO2) )
	{
		return	NULL ;
	}
	EGL_IMAGE_BUFFER_INTERFACE *	pInterface =
			((EGL_IMAGE_INFO2*)pImageInf)->pInterface ;
	while ( pInterface != NULL )
	{
		if ( pInterface->dwType == dwType )
		{
			return	pInterface->ptrObject ;
		}
		pInterface = pInterface->pNextInterface ;
	}
	return	NULL ;
}

#define	EGL_DRAW_RADIAN		0x0001

#define	EGL_IMAGE_HAS_DC	0x0001
#define	EGL_IMAGE_NO_DUP	0x0010


//
// カラーコード変換
//
extern	"C"
{
	void eglRGBtoYUV( EGL_PALETTE * pColor ) ;
	void eglYUVtoRGB( EGL_PALETTE * pColor ) ;
	void eglRGBtoHSB( EGL_PALETTE * pColor ) ;
	void eglHSBtoRGB( EGL_PALETTE * pColor ) ;
	DWORD eglBlendRGBA( EGL_PALETTE rgba1, EGL_PALETTE rgba2 ) ;
} ;

//
// フィルタリング関数
//
extern	"C"
{
	ESLError eglConvertFormat
		( PEGL_IMAGE_INFO pDstImage,
			PCEGL_IMAGE_INFO pSrcImage, DWORD dwFlags = 0 ) ;
	ESLError eglCalculateToneTable
		( void * pToneBuf, signed int nTone, int nFlag = 0 ) ;
	ESLError eglApplyToneTable
		( PEGL_IMAGE_INFO pDstImage, PCEGL_IMAGE_INFO pSrcImage,
			const void * pBlueTone, const void * pGreenTone,
			const void * pRedTone, const void * pAlphaTone ) ;
	ESLError eglSetColorTone
		( PEGL_IMAGE_INFO pDstImage, PCEGL_IMAGE_INFO pSrcImage,
			int nBlueTone, int nGreenTone, int nRedTone, int nAlphaTone ) ;
	ESLError eglMakeGrayTone
		( PEGL_IMAGE_INFO pDstImage, PCEGL_IMAGE_INFO pSrcImage ) ;
	ESLError eglEnlargeDouble
		( PEGL_IMAGE_INFO pDstImage,
			PCEGL_IMAGE_INFO pSrcImage, DWORD dwFlags = 0 ) ;
	ESLError eglReduceHalf
		( PEGL_IMAGE_INFO pDstImage,
			PCEGL_IMAGE_INFO pSrcImage, DWORD dwFlags = 0 ) ;
	ESLError eglBlendAlphaChannel
		( PEGL_IMAGE_INFO pRGBA32,
			PCEGL_IMAGE_INFO pSrcRGB,
			PCEGL_IMAGE_INFO pSrcAlpha,
			DWORD dwFlags = 0,
			SDWORD nAlphaBase = 0,
			DWORD nCoefficient = 0x10 ) ;
	ESLError eglUnpackAlphaChannel
		( PEGL_IMAGE_INFO pDstRGB, PEGL_IMAGE_INFO pDstAlpha,
			PCEGL_IMAGE_INFO pSrcRGBA32, DWORD dwFlags = 0 ) ;
} ;

#define	EGL_TONE_BRIGHTNESS		0x0000
#define	EGL_TONE_INVERSION		0x0001
#define	EGL_TONE_LIGHT			0x0002

#define	EGL_BAC_MULTIPLY		0x0001
#define	EGL_BAC_ADD_ALPHA		0x0002
#define	EGL_BAC_MULTIPLY_ALPHA	0x0004

//
// 画像描画
//
struct	EGL_DRAW_DEST
{
	PEGL_IMAGE_INFO	pDstImage ;
	EGL_RECT		rectDstClip ;
	PEGL_IMAGE_INFO	pZBuffer ;
} ;

struct	EGL_DRAW_PARAM
{
	DWORD				dwFlags ;
	EGL_POINT			ptBasePos ;
	PEGL_IMAGE_INFO		pSrcImage ;
	PCEGL_RECT			pViewRect ;
	EGL_PALETTE			rgbDimColor ;
	EGL_PALETTE			rgbLightColor ;
	unsigned int		nTransparency ;
	REAL32				rZOrder ;
	PCEGL_IMAGE_AXES	pImageAxes ;
	EGL_PALETTE			rgbColorParam1 ;
	unsigned int		nVertexCount ;
	PCE3D_VECTOR_2D		pVertexPos ;
} ;

#define	EGL_DRAW_BLEND_ALPHA	0x0001
#define	EGL_DRAW_GLOW_LIGHT		0x0002
#define	EGL_WITH_Z_ORDER		0x0004
#define	EGL_SMOOTH_STRETCH		0x0010
#define	EGL_UNSMOOTH_STRETCH	0x0020
#define	EGL_FIXED_POSITION		0x0040
#define	EGL_POLYGON_SHAPED		0x0080

inline DWORD EGL_DRAW_APPLY( int x )
{
	return	(x & 0x7F) << 16 ;
}
inline DWORD EGL_DRAW_FUNC( int x )
{
	return	(x & 0x7F) << 24 ;
}

#define	EGL_APPLY_C_ADD		0x00800000
#define	EGL_APPLY_C_MUL		0x00820000
#define	EGL_APPLY_A_MUL		0x00880000
#define	EGL_APPLY_C_MASK	0x00890000
#define	EGL_DRAW_F_ADD		0x80000000
#define	EGL_DRAW_F_SUB		0x81000000
#define	EGL_DRAW_F_MUL		0x82000000
#define	EGL_DRAW_F_DIV		0x83000000
#define	EGL_DRAW_F_MAX		0x84000000
#define	EGL_DRAW_F_MIN		0x85000000
#define	EGL_DRAW_F_SCREEN	0x86000000
#define	EGL_DRAW_A_MOVE		0x88000000
#define	EGL_DRAW_A_MUL		0x89000000
#define	EGL_DRAW_D_MASK		0x8A000000

#define	EGL_FILL_INVERSION	0x00000100

typedef	struct EGL_DRAW_IMAGE *	HEGL_DRAW_IMAGE ;

struct	EGL_DRAW_IMAGE
{
	ESLError (*pfnRelease)( HEGL_DRAW_IMAGE hDrawImage ) ;
	ESLError (*pfnInitialize)
		( HEGL_DRAW_IMAGE hDrawImage, PEGL_IMAGE_INFO pDstImage,
					PCEGL_RECT pClipRect, PEGL_IMAGE_INFO pZBuffer ) ;
	ESLError (*pfnGetDestination)
		( HEGL_DRAW_IMAGE hDrawImage, EGL_DRAW_DEST * pDrawDest ) ;
	DWORD (*pfnGetFunctionFlags)( HEGL_DRAW_IMAGE hDrawImage ) ;
	DWORD (*pfnSetFunctionFlags)
		( HEGL_DRAW_IMAGE hDrawImage, DWORD dwFlags ) ;
	void (*pfnGetDrawingOffset)
		( HEGL_DRAW_IMAGE hDrawImage, EGL_POINT * pOffset ) ;
	void (*pfnSetDrawingOffset)
		( HEGL_DRAW_IMAGE hDrawImage, const EGL_POINT * pOffset ) ;
	void *	ptrReserved1 ;
	ESLError (*pfnPrepareDraw)
		( HEGL_DRAW_IMAGE hDrawImage, const EGL_DRAW_PARAM * pDrawParam ) ;
	ESLError (*pfnDrawImage)( HEGL_DRAW_IMAGE hDrawImage ) ;
	void *	ptrReserved2 ;
	ESLError (*pfnPrepareLine)
		( HEGL_DRAW_IMAGE hDrawImage,
			int x1, int y1, int x2, int y2,
			EGL_PALETTE colorDraw,
			unsigned int nTranparency, DWORD dwFlags ) ;
	ESLError (*pfnPrepareFillRect)
		( HEGL_DRAW_IMAGE hDrawImage,
			PCEGL_RECT pDrawRect, EGL_PALETTE colorDraw,
			unsigned int nTranparency, DWORD dwFlags ) ;
	ESLError (*pfnPrepareFillEllipse)
		( HEGL_DRAW_IMAGE hDrawImage,
			PCEGL_POINT pCenter, PCEGL_SIZE pRadius,
			EGL_PALETTE colorDraw,
			unsigned int nTranparency, DWORD dwFlags ) ;
	ESLError (*pfnPrepareFillPolygon)
		( HEGL_DRAW_IMAGE hDrawImage,
			PCEGL_POINT pVertexes, unsigned int nCount,
			EGL_PALETTE colorDraw,
			unsigned int nTranparency, DWORD dwFlags ) ;
	ESLError (*pfnFillRegion)( HEGL_DRAW_IMAGE hDrawImage ) ;
	ESLError (*pfnDrawRegion)( HEGL_DRAW_IMAGE hDrawImage ) ;

	// 画像描画オブジェクト解放
	inline ESLError Release( void )
		{
			return	(*pfnRelease)( this ) ;
		}
	// 画像描画オブジェクト初期化
	inline ESLError Initialize
		( PEGL_IMAGE_INFO pDstImage,
			PCEGL_RECT pClipRect, PEGL_IMAGE_INFO pZBuffer )
		{
			return	(*pfnInitialize)( this, pDstImage, pClipRect, pZBuffer ) ;
		}
	// 画像描画先取得
	inline ESLError GetDestination( EGL_DRAW_DEST * pDrawDest )
		{
			return	(*pfnGetDestination)( this, pDrawDest ) ;
		}
	// 描画機能フラグ取得
	inline DWORD GetFunctionFlags( void )
		{
			return	(*pfnGetFunctionFlags)( this ) ;
		}
	// 描画機能フラグ設定
	inline DWORD SetFunctionFlags( DWORD dwFlags )
		{
			return	(*pfnSetFunctionFlags)( this, dwFlags ) ;
		}
	// 描画オフセット座標取得
	inline EGL_POINT GetDrawingOffset( void )
		{
			EGL_POINT	ptOffset ;
			(*pfnGetDrawingOffset)( this, &ptOffset ) ;
			return	ptOffset ;
		}
	// 描画オフセット座標設定
	inline void SetDrawingOffset( const EGL_POINT & ptOffset )
		{
			(*pfnSetDrawingOffset)( this, &ptOffset ) ;
		}
	// 画像描画準備
	inline ESLError PrepareDraw( const EGL_DRAW_PARAM * pDrawParam )
		{
			return	(*pfnPrepareDraw)( this, pDrawParam ) ;
		}
	// 画像描画
	inline ESLError DrawImage( void )
		{
			return	(*pfnDrawImage)( this ) ;
		}
	// 直線描画準備
	inline ESLError PrepareLine
		( int x1, int y1, int x2, int y2,
			EGL_PALETTE colorDraw,
			unsigned int nTransparency, DWORD dwFlags = 0 )
		{
			return	(*pfnPrepareLine)
				( this, x1, y1, x2, y2, colorDraw, nTransparency, dwFlags ) ;
		}
	// 矩形描画準備
	inline ESLError PrepareFillRect
		( PCEGL_RECT pDrawRect, EGL_PALETTE colorDraw,
			unsigned int nTransparency, DWORD dwFlags = 0 )
		{
			return	(*pfnPrepareFillRect)
				( this, pDrawRect, colorDraw, nTransparency, dwFlags ) ;
		}
	// 楕円描画準備
	inline ESLError PrepareFillEllipse
		( PCEGL_POINT pCenter, PCEGL_SIZE pRadius,
			EGL_PALETTE colorDraw,
			unsigned int nTransparency, DWORD dwFlags = 0 )
		{
			return	(*pfnPrepareFillEllipse)
				( this, pCenter, pRadius, colorDraw, nTransparency, dwFlags ) ;
		}
	// 多角形描画準備
	inline ESLError PrepareFillPolygon
		( PCEGL_POINT pVertexes, unsigned int nCount,
			EGL_PALETTE colorDraw,
			unsigned int nTransparency, DWORD dwFlags = 0 )
		{
			return	(*pfnPrepareFillPolygon)
				( this, pVertexes, nCount, colorDraw, nTransparency, dwFlags ) ;
		}
	// 図形描画
	inline ESLError FillRegion( void )
		{
			return	(*pfnFillRegion)( this ) ;
		}
	// 図形輪郭描画
	inline ESLError DrawRegion( void )
		{
			return	(*pfnDrawRegion)( this ) ;
		}

} ;

extern	"C"
{
	HEGL_DRAW_IMAGE eglCreateDrawImage( void ) ;
} ;

//
// 3次元ベクトル
//
struct	E3D_VECTOR
{
public:
	REAL32	x ;
	REAL32	y ;
	REAL32	z ;
public:
	bool operator == ( const E3D_VECTOR & vector ) const
		{
			return	(x == vector.x) && (y == vector.y) && (z == vector.z) ;
		}
	bool operator != ( const E3D_VECTOR & vector ) const
		{
			return	(x != vector.x) || (y != vector.y) || (z != vector.z) ;
		}
	const E3D_VECTOR & operator += ( const E3D_VECTOR & vector )
		{
			eglAddVector( this, &vector ) ;
			return	*this ;
		}
	const E3D_VECTOR & operator -= ( const E3D_VECTOR & vector )
		{
			eglSubVector( this, &vector ) ;
			return	*this ;
		}
	const E3D_VECTOR & operator *= ( double number )
		{
			eglMultipleVector( this, (REAL32) number ) ;
			return	*this ;
		}
	const E3D_VECTOR & operator *= ( const E3D_VECTOR & vector )
		{
			eglVectorExteriorProduct( this, &vector ) ;
			return	*this ;
		}
	const E3D_VECTOR & operator /= ( double number )
		{
			eglDivideVector( this, (REAL32) number ) ;
			return	*this ;
		}
	E3D_VECTOR operator + ( const E3D_VECTOR & vector ) const
		{
			E3D_VECTOR	result = *this ;
			eglAddVector( &result, &vector ) ;
			return	result ;
		}
	E3D_VECTOR operator - ( void ) const
		{
			E3D_VECTOR	result = { x, y, z } ;
			*((DWORD*)&(result.x)) ^= 0x80000000 ;
			*((DWORD*)&(result.y)) ^= 0x80000000 ;
			*((DWORD*)&(result.z)) ^= 0x80000000 ;
			return	result ;
		}
	E3D_VECTOR operator - ( const E3D_VECTOR & vector ) const
		{
			E3D_VECTOR	result = *this ;
			eglSubVector( &result, &vector ) ;
			return	result ;
		}
	E3D_VECTOR operator * ( const E3D_VECTOR & vector ) const
		{
			E3D_VECTOR	result = *this ;
			eglVectorExteriorProduct( &result, &vector ) ;
			return	result ;
		}
	REAL32 operator | ( const E3D_VECTOR & vector ) const
		{
			REAL32	r ;
			eglVectorInnerProduct( &r, this, &vector ) ;
			return	r ;
		}
	E3D_VECTOR operator * ( double number ) const
		{
			E3D_VECTOR	result = *this ;
			eglMultipleVector( &result, (REAL32) number ) ;
			return	result ;
		}
	E3D_VECTOR operator / ( double number ) const
		{
			E3D_VECTOR	result = *this ;
			eglDivideVector( &result, (REAL32) number ) ;
			return	result ;
		}
	REAL32 Absolute( void ) const
		{
			REAL32	r ;
			eglAbsoluteVector( &r, this ) ;
			return	r ;
		}
	void ExteriorProduct( const E3D_VECTOR & vector )
		{
			eglVectorExteriorProduct( this, &vector ) ;
		}
	REAL32 InnerProduct( const E3D_VECTOR & vector ) const
		{
			REAL32	r ;
			eglVectorInnerProduct( &r, this, &vector ) ;
			return	r ;
		}
	void Normalize( void )
		{
			eglVectorRoundTo1( this ) ;
		}
	void RoundTo1( void )
		{
			eglVectorRoundTo1( this ) ;
		}
} ;

struct	E3DDF_VECTOR
{
public:
	double	x ;
	double	y ;
	double	z ;
public:
	bool operator == ( const E3DDF_VECTOR & vector ) const
		{
			return	(x == vector.x) && (y == vector.y) && (z == vector.z) ;
		}
	bool operator != ( const E3DDF_VECTOR & vector ) const
		{
			return	(x != vector.x) || (y != vector.y) || (z != vector.z) ;
		}
	const E3DDF_VECTOR & operator += ( const E3DDF_VECTOR & vector )
		{
			x += vector.x ;
			y += vector.y ;
			z += vector.z ;
			return	*this ;
		}
	const E3DDF_VECTOR & operator -= ( const E3DDF_VECTOR & vector )
		{
			x -= vector.x ;
			y -= vector.y ;
			z -= vector.z ;
			return	*this ;
		}
	const E3DDF_VECTOR & operator *= ( double number )
		{
			x *= number ;
			y *= number ;
			z *= number ;
			return	*this ;
		}
	const E3DDF_VECTOR & operator *= ( const E3DDF_VECTOR & vector )
		{
			E3DDF_VECTOR	result ;
			result.x = y * vector.z - z * vector.y ;
			result.y = z * vector.x - x * vector.z ;
			result.z = x * vector.y - y * vector.x ;
			*this = result ;
			return	*this ;
		}
	const E3DDF_VECTOR & operator /= ( double number )
		{
			x /= number ;
			y /= number ;
			z /= number ;
			return	*this ;
		}
	E3DDF_VECTOR operator + ( const E3DDF_VECTOR & vector ) const
		{
			E3DDF_VECTOR	result = *this ;
			result += vector ;
			return	result ;
		}
	E3DDF_VECTOR operator - ( void ) const
		{
			E3DDF_VECTOR	result = { - x, - y, - z } ;
			return	result ;
		}
	E3DDF_VECTOR operator - ( const E3DDF_VECTOR & vector ) const
		{
			E3DDF_VECTOR	result = *this ;
			result -= vector ;
			return	result ;
		}
	double operator | ( const E3DDF_VECTOR & vector ) const
		{
			return	x * vector.x + y * vector.y + z * vector.z ;
		}
	E3DDF_VECTOR operator * ( double number ) const
		{
			E3DDF_VECTOR	result = { x * number, y * number, z * number } ;
			return	result ;
		}
	E3DDF_VECTOR operator * ( const E3DDF_VECTOR & vector ) const
		{
			E3DDF_VECTOR	result ;
			result.x = y * vector.z - z * vector.y ;
			result.y = z * vector.x - x * vector.z ;
			result.z = x * vector.y - y * vector.x ;
			return	result ;
		}
	E3DDF_VECTOR operator / ( double number ) const
		{
			E3DDF_VECTOR	result = *this ;
			result /= number ;
			return	result ;
		}
	double Absolute( void ) const
		{
			return	sqrt( x * x + y * y + z * z ) ;
		}
	void ExteriorProduct( const E3DDF_VECTOR & vector )
		{
			*this *= vector ;
		}
	double InnerProduct( const E3DDF_VECTOR & vector ) const
		{
			return	x * vector.x + y * vector.y + z * vector.z ;
		}
	void Normalize( void )
		{
			double	d = sqrt( x * x + y * y + z * z ) ;
			if ( d > 0.0 )
			{
				*this *= (1.0 / d) ;
			}
		}
	void RoundTo1( void )
		{
			double	d = sqrt( x * x + y * y + z * z ) ;
			if ( d > 0.0 )
			{
				*this *= (1.0 / d) ;
			}
		}
} ;

class	E3DVector	: public	E3D_VECTOR
{
public:
	E3DVector( void ) { }
	E3DVector( double vx, double vy, double vz )
		{ x = (REAL32) vx ;  y = (REAL32) vy ;  z = (REAL32) vz ; }
	template <class T> E3DVector( const T & v )
		{ x = (REAL32) v.x ;  y = (REAL32) v.y ;  z = (REAL32) v.z ; }
	template <class T> const E3DVector & operator = ( const T & vector )
		{
			x = (REAL32) vector.x ;
			y = (REAL32) vector.y ;
			z = (REAL32) vector.z ;
			return	*this ;
		}

} ;

class	E3DDFVector	: public	E3DDF_VECTOR
{
public:
	E3DDFVector( void ) { }
	E3DDFVector( double vx, double vy, double vz )
		{ x = (REAL32) vx ;  y = (REAL32) vy ;  z = (REAL32) vz ; }
	E3DDFVector( const E3D_VECTOR & v )
		{ x = v.x ;  y = v.y ;  z = v.z ; }
	E3DDFVector( const E3DDF_VECTOR & v )
		{ x = v.x ;  y = v.y ;  z = v.z ; }
	template <class T> const E3DDFVector & operator = ( const T & vector )
		{
			x = vector.x ;
			y = vector.y ;
			z = vector.z ;
			return	*this ;
		}

} ;

typedef	E3D_VECTOR *		PE3D_VECTOR ;
typedef	const E3D_VECTOR *	PCE3D_VECTOR ;
typedef	E3DDF_VECTOR *			PE3DDF_VECTOR ;
typedef	const E3DDF_VECTOR *	PCE3DDF_VECTOR ;

struct	E3D_VECTOR4	: public	E3D_VECTOR
{
	REAL32	d ;
} ;

class	E3DVector4	: public	E3D_VECTOR4
{
public:
	E3DVector4( void ) { }
	E3DVector4( double vx, double vy, double vz, double vd = 0 )
		{ x = (REAL32) vx ;  y = (REAL32) vy ;
			z = (REAL32) vz ;  d = (REAL32) vd ; }
	E3DVector4( const E3D_VECTOR4 & v )
		{ x = v.x ;  y = v.y ;  z = v.z ;  d = v.d ; }
	E3DVector4( const E3D_VECTOR & v )
		{ x = v.x ;  y = v.y ;  z = v.z ;  d = 0 ; }
	E3DVector4( const E3DDF_VECTOR & v )
		{ x = (REAL32) v.x ;  y = (REAL32) v.y ;  z = (REAL32) v.z ;  d = 0 ; }
	const E3DVector4 & operator = ( const E3D_VECTOR & vector )
		{
			x = vector.x ;
			y = vector.y ;
			z = vector.z ;
			d = 0 ;
			return	*this ;
		}
	const E3DVector4 & operator = ( const E3D_VECTOR4 & vector )
		{
			x = vector.x ;
			y = vector.y ;
			z = vector.z ;
			d = vector.d ;
			return	*this ;
		}

} ;

typedef	E3D_VECTOR4 *		PE3D_VECTOR4 ;
typedef	const E3D_VECTOR4 *	PCE3D_VECTOR4 ;


//
// 3x3 変換行列
// （アラインのため 3x4 ）
//
struct	E3D_REV_MATRIX
{
public:
	REAL32	matrix[3][4] ;
public:
	void InitializeMatrix( const E3D_VECTOR & vector ) ;
	int operator == ( const E3D_REV_MATRIX & m ) const
		{
			return	(matrix[0][0] == m.matrix[0][0])
						& (matrix[0][1] == m.matrix[0][1])
						& (matrix[0][2] == m.matrix[0][2])
					& (matrix[1][0] == m.matrix[1][0])
						& (matrix[1][1] == m.matrix[1][1])
						& (matrix[1][2] == m.matrix[1][2])
					& (matrix[2][0] == m.matrix[2][0])
						& (matrix[2][1] == m.matrix[2][1])
						& (matrix[2][2] == m.matrix[2][2]) ;
		}
	int operator != ( const E3D_REV_MATRIX & m ) const
		{
			return	(matrix[0][0] != m.matrix[0][0])
						| (matrix[0][1] != m.matrix[0][1])
						| (matrix[0][2] != m.matrix[0][2])
					| (matrix[1][0] != m.matrix[1][0])
						| (matrix[1][1] != m.matrix[1][1])
						| (matrix[1][2] != m.matrix[1][2])
					| (matrix[2][0] != m.matrix[2][0])
						| (matrix[2][1] != m.matrix[2][1])
						| (matrix[2][2] != m.matrix[2][2]) ;
		}
	const E3D_REV_MATRIX & operator += ( const E3D_REV_MATRIX & matrix )
		{
			eglMatrixAdd( this, &matrix ) ;
			return	*this ;
		}
	const E3D_REV_MATRIX & operator -= ( const E3D_REV_MATRIX & matrix )
		{
			eglMatrixSub( this, &matrix ) ;
			return	*this ;
		}
	const E3D_REV_MATRIX & operator *= ( const E3D_REV_MATRIX & matrix )
		{
			eglMatrixRevolveBy( this, &matrix ) ;
			return	*this ;
		}
	const E3D_REV_MATRIX & operator *= ( double number )
		{
			eglMatrixMultiple( this, (REAL32) number ) ;
			return	*this ;
		}
	const E3D_REV_MATRIX & operator /= ( double number )
		{
			eglMatrixMultiple( this, (REAL32) (1.0 / number) ) ;
			return	*this ;
		}
	E3D_REV_MATRIX operator + ( const E3D_REV_MATRIX & matrix ) const
		{
			E3D_REV_MATRIX	result = *this ;
			eglMatrixAdd( &result, &matrix ) ;
			return	result ;
		}
	E3D_REV_MATRIX operator - ( void ) const
		{
			E3D_REV_MATRIX	result = *this ;
			eglMatrixNegate( &result ) ;
			return	result ;
		}
	E3D_REV_MATRIX operator - ( const E3D_REV_MATRIX & matrix ) const
		{
			E3D_REV_MATRIX	result = *this ;
			eglMatrixSub( &result, &matrix ) ;
			return	result ;
		}
	E3D_REV_MATRIX operator * ( const E3D_REV_MATRIX & matrix )
		{
			E3D_REV_MATRIX	result = *this ;
			eglMatrixRevolveBy( &result, &matrix ) ;
			return	result ;
		}
	E3D_VECTOR operator * ( const E3D_VECTOR & vector )
		{
			E3D_VECTOR	result = vector ;
			eglMatrixRevolveVector( this, &result ) ;
			return	result ;
		}
	E3D_REV_MATRIX operator * ( double number ) const
		{
			E3D_REV_MATRIX	result = *this ;
			eglMatrixMultiple( &result, (REAL32) number ) ;
			return	result ;
		}
	E3D_REV_MATRIX operator / ( double number ) const
		{
			E3D_REV_MATRIX	result = *this ;
			eglMatrixMultiple( &result, (REAL32) (1.0 / number) ) ;
			return	result ;
		}
	REAL32 Determinant( void ) const
		{
			REAL32	r ;
			eglMatrixDeterminant( &r, this ) ;
			return	r ;
		}
	E3D_REV_MATRIX & InverseOf( const E3D_REV_MATRIX & matrix )
		{
			eglMatrixInverse( this, &matrix ) ;
			return	*this ;
		}
	void RevolveOnX( double rSin, double rCos )
		{
			eglMatrixRevolveOnX( this, (REAL32) rSin, (REAL32) rCos ) ;
		}
	void RevolveOnY( double rSin, double rCos )
		{
			eglMatrixRevolveOnY( this, (REAL32) rSin, (REAL32) rCos ) ;
		}
	void RevolveOnZ( double rSin, double rCos )
		{
			eglMatrixRevolveOnZ( this, (REAL32) rSin, (REAL32) rCos ) ;
		}
	void RevolveByAngleOn( const E3D_VECTOR & angle )
		{
			eglMatrixRevolveByAngleOn( this, &angle ) ;
		}
	void RevolveForAngle( const E3D_VECTOR & angle )
		{
			eglMatrixRevolveForAngle( this, &angle ) ;
		}
	void MagnifyByVector( const E3D_VECTOR & vector )
		{
			eglMatrixMagnifyByVector( this, &vector ) ;
		}
	void RevolveMatrix( E3D_REV_MATRIX & matDst ) const
		{
			eglMatrixRevolve( this, &matDst ) ;
		}
	void RevolveByMatrix( const E3D_REV_MATRIX & matSrc )
		{
			eglMatrixRevolveBy( this, &matSrc ) ;
		}
	void RevolveVector( E3D_VECTOR & vector ) const
		{
			eglMatrixRevolveVector( this, &vector ) ;
		}
	void RevolveVectors
			( E3D_VECTOR4 * pDst, const E3D_VECTOR4 * pSrc,
					const E3D_VECTOR * pvMove, int nCount ) const
		{
			eglMatrixRevolveVectors( this, pDst, pSrc, pvMove, nCount ) ;
		}
} ;

struct	E3DDF_REV_MATRIX
{
public:
	double	matrix[3][3] ;
public:
	void InitializeMatrix( const E3DDF_VECTOR & vector ) ;
	int operator == ( const E3DDF_REV_MATRIX & m ) const
		{
			return	(matrix[0][0] == m.matrix[0][0])
						& (matrix[0][1] == m.matrix[0][1])
						& (matrix[0][2] == m.matrix[0][2])
					& (matrix[1][0] == m.matrix[1][0])
						& (matrix[1][1] == m.matrix[1][1])
						& (matrix[1][2] == m.matrix[1][2])
					& (matrix[2][0] == m.matrix[2][0])
						& (matrix[2][1] == m.matrix[2][1])
						& (matrix[2][2] == m.matrix[2][2]) ;
		}
	int operator != ( const E3DDF_REV_MATRIX & m ) const
		{
			return	(matrix[0][0] != m.matrix[0][0])
						| (matrix[0][1] != m.matrix[0][1])
						| (matrix[0][2] != m.matrix[0][2])
					| (matrix[1][0] != m.matrix[1][0])
						| (matrix[1][1] != m.matrix[1][1])
						| (matrix[1][2] != m.matrix[1][2])
					| (matrix[2][0] != m.matrix[2][0])
						| (matrix[2][1] != m.matrix[2][1])
						| (matrix[2][2] != m.matrix[2][2]) ;
		}
	const E3DDF_REV_MATRIX & operator += ( const E3DDF_REV_MATRIX & m ) ;
	const E3DDF_REV_MATRIX & operator -= ( const E3DDF_REV_MATRIX & m ) ;
	const E3DDF_REV_MATRIX & operator *= ( const E3DDF_REV_MATRIX & m )
		{
			RevolveByMatrix( m ) ;
			return	*this ;
		}
	const E3DDF_REV_MATRIX & operator *= ( double number ) ;
	const E3DDF_REV_MATRIX & operator /= ( double number ) ;
	E3DDF_REV_MATRIX operator + ( const E3DDF_REV_MATRIX & m ) const
		{
			E3DDF_REV_MATRIX	result = *this ;
			result += m ;
			return	result ;
		}
	E3DDF_REV_MATRIX operator - ( void ) const ;
	E3DDF_REV_MATRIX operator - ( const E3DDF_REV_MATRIX & m ) const
		{
			E3DDF_REV_MATRIX	result = *this ;
			result -= m ;
			return	result ;
		}
	E3DDF_REV_MATRIX operator * ( const E3DDF_REV_MATRIX & m ) const
		{
			E3DDF_REV_MATRIX	result = *this ;
			result *= m ;
			return	result ;
		}
	E3DDF_VECTOR operator * ( const E3DDF_VECTOR & vector ) const
		{
			E3DDF_VECTOR	result = vector ;
			RevolveVector( result ) ;
			return	result ;
		}
	E3DDF_REV_MATRIX operator * ( double number ) const
		{
			E3DDF_REV_MATRIX	result = *this ;
			result *= number ;
			return	result ;
		}
	E3DDF_REV_MATRIX operator / ( double number ) const
		{
			E3DDF_REV_MATRIX	result = *this ;
			result /= number ;
			return	result ;
		}
	double Determinant( void ) const ;
	E3DDF_REV_MATRIX & InverseOf( const E3DDF_REV_MATRIX & m ) ;
	void RevolveOnX( double rSin, double rCos ) ;
	void RevolveOnY( double rSin, double rCos ) ;
	void RevolveOnZ( double rSin, double rCos ) ;
	void RevolveByAngleOn( const E3DDF_VECTOR & angle ) ;
	void RevolveForAngle( const E3DDF_VECTOR & angle ) ;
	void MagnifyByVector( const E3DDF_VECTOR & vector ) ;
	void RevolveMatrix( E3DDF_REV_MATRIX & matDst ) const ;
	void RevolveByMatrix( const E3DDF_REV_MATRIX & matSrc ) ;
	void RevolveVector( E3DDF_VECTOR & vector ) const ;
} ;

class	E3DRevMatrix	: public E3D_REV_MATRIX
{
public:
	E3DRevMatrix( void ) ;
	E3DRevMatrix( const E3D_VECTOR & vUnit ) ;
	E3DRevMatrix( const E3D_REV_MATRIX & m ) ;
	E3DRevMatrix( const E3DDF_REV_MATRIX & m ) ;
	const E3DRevMatrix & operator = ( const E3D_REV_MATRIX & m ) ;
	const E3DRevMatrix & operator = ( const E3DDF_REV_MATRIX & m ) ;
} ;

class	E3DDFRevMatrix	: public E3DDF_REV_MATRIX
{
public:
	E3DDFRevMatrix( void ) ;
	E3DDFRevMatrix( const E3DDF_VECTOR & vUnit ) ;
	E3DDFRevMatrix( const E3DDF_REV_MATRIX & m ) ;
	E3DDFRevMatrix( const E3D_REV_MATRIX & m ) ;
	const E3DDFRevMatrix & operator = ( const E3DDF_REV_MATRIX & m ) ;
	const E3DDFRevMatrix & operator = ( const E3D_REV_MATRIX & m ) ;
} ;

typedef	E3D_REV_MATRIX *		PE3D_REV_MATRIX ;
typedef	const E3D_REV_MATRIX *	PCE3D_REV_MATRIX ;
typedef	E3DDF_REV_MATRIX *			PE3DDF_REV_MATRIX ;
typedef	const E3DDF_REV_MATRIX *	PCE3DDF_REV_MATRIX ;

//
// クォータニオン
//
template <class T, class M> struct	E3DT_QUATERNION
{
	T	q[4] ;

	// 行列へ変換
	void ToMatrix( M & m )
		{
			m.matrix[0][0] =
				q[0]*q[0] + q[1]*q[1] - q[2]*q[2] - q[3]*q[3] ;
			m.matrix[0][1] =
				2 * (q[1] * q[2] - q[0] * q[3]) ;
			m.matrix[0][2] =
				2 * (q[1] * q[3] + q[0] * q[2]) ;
			//
			m.matrix[1][0] =
				2 * (q[1] * q[2] + q[0] * q[3]) ;
			m.matrix[1][1] =
				q[0]*q[0] - q[1]*q[1] + q[2]*q[2] - q[3]*q[3] ;
			m.matrix[1][2] =
				2 * (q[2] * q[3] - q[0] * q[1]) ;
			//
			m.matrix[2][0] =
				2 * (q[1] * q[3] - q[0] * q[2]) ;
			m.matrix[2][1] =
				2 * (q[2] * q[3] + q[0] * q[1]) ;
			m.matrix[2][2] =
				q[0]*q[0] - q[1]*q[1] - q[2]*q[2] + q[3]*q[3] ;
		}
	// 行列から変換
	void FromMatrix( const M & m )
		{
			q[0] = (T) (( m.matrix[0][0] + m.matrix[1][1]
											+ m.matrix[2][2] + 1) * 0.25) ;
			q[1] = (T) (( m.matrix[0][0] - m.matrix[1][1]
											- m.matrix[2][2] + 1) * 0.25) ;
			q[2] = (T) ((-m.matrix[0][0] + m.matrix[1][1]
											- m.matrix[2][2] + 1) * 0.25) ;
			q[3] = (T) ((-m.matrix[0][0] - m.matrix[1][1]
											+ m.matrix[2][2] + 1) * 0.25) ;
			//
			if ( q[0] < 0 )	q[0] = 0 ;
			if ( q[1] < 0 )	q[1] = 0 ;
			if ( q[2] < 0 )	q[2] = 0 ;
			if ( q[3] < 0 )	q[3] = 0 ;
			q[0] = (T) sqrt( q[0] ) ;
			q[1] = (T) sqrt( q[1] ) ;
			q[2] = (T) sqrt( q[2] ) ;
			q[3] = (T) sqrt( q[3] ) ;
			//
			if ( (q[0] < 1.0e-10) && (q[1] < 1.0e-10) )
			{
				q[0] = 0 ;
				q[1] = 0 ;
				if ( m.matrix[2][1] + m.matrix[1][2] < 0 )
				{
					q[3] = - q[3] ;
				}
			}
			else if ( q[0] < 1.0e-10 )
			{
				q[0] = 0 ;
				if ( m.matrix[1][0] + m.matrix[0][2] < 0 )
				{
					q[2] = - q[2] ;
				}
				if ( m.matrix[0][2] + m.matrix[2][0] < 0 )
				{
					q[3] = - q[3] ;
				}
			}
			else
			{
				if ( m.matrix[2][1] - m.matrix[1][2] < 0 )
				{
					q[1] = - q[1] ;
				}
				if ( m.matrix[0][2] - m.matrix[2][0] < 0 )
				{
					q[2] = - q[2] ;
				}
				if ( m.matrix[1][0] - m.matrix[0][1] < 0 )
				{
					q[3] = - q[3] ;
				}
			}
			Normalize( ) ;
		}
	// 加算
	const E3DT_QUATERNION<T,M> & operator += ( const E3DT_QUATERNION<T,M> & r )
		{
			q[0] += r.q[0] ;
			q[1] += r.q[1] ;
			q[2] += r.q[2] ;
			q[3] += r.q[3] ;
			return	*this ;
		}
	E3DT_QUATERNION<T,M> operator + ( const E3DT_QUATERNION<T,M> & r ) const
		{
			E3DT_QUATERNION<T,M>	a = *this ;
			a += r ;
			return	a ;
		}
	// 減算
	const E3DT_QUATERNION<T,M> & operator -= ( const E3DT_QUATERNION<T,M> & r )
		{
			q[0] -= r.q[0] ;
			q[1] -= r.q[1] ;
			q[2] -= r.q[2] ;
			q[3] -= r.q[3] ;
			return	*this ;
		}
	E3DT_QUATERNION<T,M> operator - ( const E3DT_QUATERNION<T,M> & r ) const
		{
			E3DT_QUATERNION<T,M>	a = *this ;
			a -= r ;
			return	a ;
		}
	// 積
	const E3DT_QUATERNION<T,M> & operator *= ( double r )
		{
			q[0] *= (T) r ;
			q[1] *= (T) r ;
			q[2] *= (T) r ;
			q[3] *= (T) r ;
			return	*this ;
		}
	const E3DT_QUATERNION<T,M> & operator *= ( const E3DT_QUATERNION<T,M> & s )
		{
			*this = (*this * s) ;
			return	*this ;
		}
	E3DT_QUATERNION<T,M> operator * ( double r ) const
		{
			E3DT_QUATERNION<T,M>	a = *this ;
			a *= r ;
			return	a ;
		}
	E3DT_QUATERNION<T,M> operator * ( const E3DT_QUATERNION<T,M> & s )
		{
			E3DT_QUATERNION<T,M>	d ;
			d.q[0] = q[0] * s.q[0] - q[1] * s.q[1]
							- q[2] * s.q[2] - q[3] * s.q[3] ;
			d.q[1] = q[0] * s.q[1] + q[1] * s.q[0]
							+ q[2] * s.q[3] - q[3] * s.q[2] ;
			d.q[2] = q[0] * s.q[2] - q[1] * s.q[3]
							+ q[2] * s.q[0] + q[3] * s.q[1] ;
			d.q[3] = q[0] * s.q[3] + q[1] * s.q[2]
							- q[2] * s.q[1] + q[3] * s.q[0] ;
			return	d ;
		}
	// 曲面補間
	void Slerp( const E3DT_QUATERNION<T,M> & q1,
					const E3DT_QUATERNION<T,M> & q2, double t )
		{
			double	qr = q1.q[0] * q2.q[0] + q1.q[1] * q2.q[1]
							+ q1.q[2] * q2.q[2] + q1.q[3] * q2.q[3] ;
			if ( qr * qr >= 1.0 - 1.0e-10 )		// 1.0 - qr * qr != 0.0
			{
				q[0] = q1.q[0] ;
				q[1] = q1.q[1] ;
				q[2] = q1.q[2] ;
				q[3] = q1.q[3] ;
			}
			else
			{
				double	rad = acos( qr ) ;
				double	sr = 1.0 / sin( rad ) ;
				double	st1 = sin( rad * (1 - t) ) * sr ;
				double	st2 = sin( rad * t ) * sr ;
				//
				if ( (qr < 0) /*&& (rad >= 3.14159265 / 2)*/ )
				{
					q[0] = (T) (q1.q[0] * st1 - q2.q[0] * st2) ;
					q[1] = (T) (q1.q[1] * st1 - q2.q[1] * st2) ;
					q[2] = (T) (q1.q[2] * st1 - q2.q[2] * st2) ;
					q[3] = (T) (q1.q[3] * st1 - q2.q[3] * st2) ;
				}
				else
				{
					q[0] = (T) (q1.q[0] * st1 + q2.q[0] * st2) ;
					q[1] = (T) (q1.q[1] * st1 + q2.q[1] * st2) ;
					q[2] = (T) (q1.q[2] * st1 + q2.q[2] * st2) ;
					q[3] = (T) (q1.q[3] * st1 + q2.q[3] * st2) ;
				}
				Normalize( ) ;
			}
		}
	// 正規化
	void Normalize( void )
		{
			double	r = sqrt( q[0] * q[0] + q[1] * q[1]
								+ q[2] * q[2] + q[3] * q[3] ) ;
			if ( r >= 1.0e-32 )
			{
				r = 1.0 / r ;
				q[0] *= (T) r ;
				q[1] *= (T) r ;
				q[2] *= (T) r ;
				q[3] *= (T) r ;
			}
		}
} ;

typedef	E3DT_QUATERNION<REAL32,E3D_REV_MATRIX>	E3D_QUATERNION ;
typedef	E3DT_QUATERNION<double,E3DDF_REV_MATRIX>	E3DDF_QUATERNION ;

class	E3DQuaternion	: public E3D_QUATERNION
{
public:
	E3DQuaternion( void ) {} ;
	E3DQuaternion( const E3D_QUATERNION & qt )
		{
			q[0] = qt.q[0] ;
			q[1] = qt.q[1] ;
			q[2] = qt.q[2] ;
			q[3] = qt.q[3] ;
		}
	E3DQuaternion( const E3DDF_QUATERNION & qt )
		{
			q[0] = (REAL32) qt.q[0] ;
			q[1] = (REAL32) qt.q[1] ;
			q[2] = (REAL32) qt.q[2] ;
			q[3] = (REAL32) qt.q[3] ;
		}
	E3DQuaternion( const E3D_REV_MATRIX & mat )
		{
			FromMatrix( mat ) ;
		}
	const E3DQuaternion & operator = ( const E3D_QUATERNION & qt )
		{
			q[0] = qt.q[0] ;
			q[1] = qt.q[1] ;
			q[2] = qt.q[2] ;
			q[3] = qt.q[3] ;
			return	*this ;
		}
	const E3DQuaternion & operator = ( const E3D_REV_MATRIX & mat )
		{
			FromMatrix( mat ) ;
			return	*this ;
		}
} ;

class	E3DDFQuaternion	: public E3DDF_QUATERNION
{
public:
	E3DDFQuaternion( void ) {} ;
	E3DDFQuaternion( const E3DDF_QUATERNION & qt )
		{
			q[0] = qt.q[0] ;
			q[1] = qt.q[1] ;
			q[2] = qt.q[2] ;
			q[3] = qt.q[3] ;
		}
	E3DDFQuaternion( const E3D_QUATERNION & qt )
		{
			q[0] = qt.q[0] ;
			q[1] = qt.q[1] ;
			q[2] = qt.q[2] ;
			q[3] = qt.q[3] ;
		}
	E3DDFQuaternion( const E3DDF_REV_MATRIX & mat )
		{
			FromMatrix( mat ) ;
		}
	const E3DDFQuaternion & operator = ( const E3DDF_QUATERNION & qt )
		{
			q[0] = qt.q[0] ;
			q[1] = qt.q[1] ;
			q[2] = qt.q[2] ;
			q[3] = qt.q[3] ;
			return	*this ;
		}
	const E3DDFQuaternion & operator = ( const E3DDF_REV_MATRIX & mat )
		{
			FromMatrix( mat ) ;
			return	*this ;
		}
} ;

//
// 環境マッピング
//
struct	E3D_ENVIRONMENT_MAPPING
{
	PEGL_IMAGE_INFO	pUpperImage ;		// 上半球
	PEGL_IMAGE_INFO	pUnderImage ;		// 下半球
	DWORD			dwFlags ;
	DWORD			dwReserved[3] ;
} ;

//
// テクスチャーマッピング
//
struct	E3D_TEXTURE_MAPPING
{
	PEGL_IMAGE_INFO	pTextureImage ;		// テクスチャ画像
	PEGL_IMAGE_INFO	pLuminousImage ;	// 発光テクスチャ
	REAL32			rThresholdZ ;		// 縮小画像に切り替えるための閾値
	DWORD			nSmallScale ;		// 縮小画像のスケール（1/2^n）
	PEGL_IMAGE_INFO	pSmallImage ;		// 縮小テクスチャ画像
	PEGL_IMAGE_INFO	pSmallLuminous ;	// 縮小発光テクスチャ
} ;

//
// ポリゴンの表面属性
//
struct	E3D_SURFACE_ATTRIBUTE
{
	DWORD			dwShadingFlags ;	// シェーディングフラグ
	DWORD			dwReserved ;
	E3D_COLOR		rgbaColor ;			// 基本色
	union
	{
		E3D_TEXTURE_MAPPING		txmap ;
		E3D_ENVIRONMENT_MAPPING	envmap ;
	} ;
	DWORD			nTextureApply ;		// テクスチャ適用度（未使用）
	DWORD			nLuminousApply ;	// 発光テクスチャ適用度
	SDWORD			nAmbient ;			// 環境光 (x256)
	SDWORD			nDiffusion ;		// 拡散光 (x256)
	SDWORD			nSpecular ;			// 反射光 (x256)
	SDWORD			nSpecularSize ;		// 反射光の鋭さ（0～256）
	SDWORD			nTransparency ;		// 透明度 (x256)
	SDWORD			nDeepness ;			// 透明深度係数 (x256)
	E3D_COLOR		rgbaShade ;			// 影色 (ver.3.09 以降)
	DWORD			nReflection ;		// 反射率 (x256)
	REAL32			nRefraction ;		// 屈折率 (-1.0)（0.0 の時屈折率 1.0）
} ;
typedef	E3D_SURFACE_ATTRIBUTE *			PE3D_SURFACE_ATTRIBUTE ;
typedef	const E3D_SURFACE_ATTRIBUTE *	PCE3D_SURFACE_ATTRIBUTE ;

#define	E3DSAF_NO_SHADING			0x00000000		// シェーディング無し
#define	E3DSAF_FLAT_SHADE			0x00000001		// フラットシェーディング（未使用）
#define	E3DSAF_GOURAUD_SHADE		0x00000002		// グーローシェーディング
#define	E3DSAF_PHONG_SHADE			0x00000004		// フォンシェーディング (ver.3.09 以降)
#define	E3DSAF_PHONG_SHADE_BEFORE_TEXTURE	0x00000006
#define	E3DSAF_RAY_SHADOWING		0x00000010		// 陰を落とす (ver.3.09 以降)
#define	E3DSAF_RAY_REFLECTING		0x00000020		// 反射を有効にする (ver.3.09 以降)
#define	E3DSAF_RAY_REFRACTING		0x00000040		// 屈折を有効にする (ver.3.09 以降)
#define	E3DSAF_RAY_TRACING			0x00000074		// レイトレーシング (ver.3.09 以降)
#define	E3DSAF_RAY_TRACING_BEFORE_TEXTURE	0x00000076
#define	E3DSAF_SHADING_MASK			0x000000FF
#define	E3DSAF_TEXTURE_TILING		0x00000100		// テクスチャをタイリング
#define	E3DSAF_TEXTURE_TRIM			0x00000200		// トリミングテクスチャ
#define	E3DSAF_TEXTURE_SMOOTH		0x00000400		// テクスチャ補完拡大
#define	E3DSAF_TEXTURE_MAPPING		0x00001000		// テクスチャマッピング
#define	E3DSAF_ENVIRONMENT_MAP		0x00002000		// 環境マッピング
#define	E3DSAF_GENVIRONMENT_MAP		0x00004000		// グローバル環境マッピング
#define	E3DSAF_SINGLE_SIDE_PLANE	0x00010000		// 片面ポリゴン
#define	E3DSAF_NO_ZBUFFER			0x00020000		// ｚ比較を行わないで描画
#define	E3DSAF_ZBUF_ONLY_COMPARE	0x00040000		// ｚ比較のみ（ｚバッファへ書き込まない）
#define	E3DSAF_NO_SHADOW_OBJECT		0x01000000		// 他のオブジェクトに陰を落とさない（レイトレーシングモードのみ）
#define	E3DSAF_NO_SALF_SHADOW		0x02000000		// 自分自身のメッシュに陰を落とさない（レイトレーシングモードのみ）
#define	E3DSAF_NO_REFLECT_OBJECT	0x04000000		// 他のオブジェクトに映りこまない（レイトレーシングモードのみ）
#define	E3DSAF_GLOBAL_REFLECT_OBJECT	0x08000000	// 距離に関係なく他のオブジェクトに映りこむ（レイトレーシングモードのみ）

//
// 光源
//
struct	E3D_SHADOW_MAP_INFO
{
	E3D_VECTOR4		vLightPos ;		// 光源座標
	E3D_VECTOR4		vLightRay ;		// 光線ベクトル
	E3D_VECTOR4		vOriginPos ;	// シャドウマップの原点
	E3D_VECTOR4		vAxisX ;		// シャドウマップのｘ基底ベクトル
	E3D_VECTOR4		vAxisY ;		// シャドウマップのｙ基底ベクトル
	PEGL_IMAGE_INFO	pShadowMap ;	// シャドウマップ画像
	REAL32			rFixErrorGap ;	// z 演算誤差固定比率（1/4096 等）
	REAL32			rVarErrorGap ;	// 角度に応じた z 演算誤差比率（1/512 等）
	DWORD			dwReserved ;
} ;
typedef	E3D_SHADOW_MAP_INFO *	PE3D_SHADOW_MAP_INFO ;

struct	E3D_LIGHT_ENTRY
{
	DWORD		dwLightType ;				// 光源タイプ
	EGL_PALETTE	rgbColor ;					// 光源色
	union
	{
		REAL32	rBrightness ;				// 光源輝度
		REAL32	rFogDeepness ;				// 擬似フォグ濃度（100％までの距離）
	} ;
	union
	{
		DWORD	dwReserved ;
		REAL32	rFogDistance ;				// 擬似フォッグ開始距離 (ver.3.09 以降)
	} ;
	union
	{
		E3D_VECTOR4	vecLight ;				// 光源ベクトル
		struct
		{
			E3D_VECTOR				vecLight ;
			PE3D_SHADOW_MAP_INFO	pMapInfo ;
		}			ShadowMap ;
	} ;
} ;
typedef	E3D_LIGHT_ENTRY *	PE3D_LIGHT_ENTRY ;
typedef	const E3D_LIGHT_ENTRY *	PCE3D_LIGHT_ENTRY ;

#define	E3D_FOG_LIGHT			0x00000000		// 擬似フォッグ
#define	E3D_AMBIENT_LIGHT		0x00000001		// 環境光
#define	E3D_VECTOR_LIGHT		0x00000002		// 無限遠光源
#define	E3D_SHADOW_LIGHT		0x00000003		// 影色用環境光源
#define	E3D_POINT_LIGHT			0x00000004		// 点光源
#define	E3D_LIGHT_TYPE_MASK		0x0000000F
#define	E3D_LIGHT_SHADOW_MAP	0x80000000		// シャドウマップ有効

//
// テクスチャマッピング用パラメータ
//
struct	E3D_TEXTURE_MAPINFO
{
	PEGL_IMAGE_INFO	pTextureImage ;		// 通常テクスチャ
	PEGL_IMAGE_INFO	pLuminousImage ;	// 発光テクスチャ
	DWORD			nTextureApply ;		// テクスチャ適用度
	DWORD			nLuminousApply ;	// 発光テクスチャ適用度
	E3D_VECTOR4		vOriginPos ;		// マッピングパラメータ
	E3D_VECTOR4		vAxisX ;
	E3D_VECTOR4		vAxisY ;
	REAL32			rFogDeepness ;		// 擬似フォッグ用
	EGL_PALETTE		rgbFogColor ;
} ;

//
// ポリゴンプリミティブ情報
//
struct	E3D_PRIMITIVE_VERTEX
{
	union
	{
		PE3D_VECTOR4	vertex ;	// 頂点座標
		INT_PTR			i_vertex ;
	} ;
	union
	{
		PE3D_VECTOR4	normal ;	// 法線（スムースポリゴンのみ）
		INT_PTR			i_normal ;
	} ;
	union
	{
		E3D_VECTOR_2D	uv_map ;	// UV 座標（テクスチャポリゴンのみ）
		E3D_COLOR		color ;		// 頂点色（頂点色ポリゴンのみ）
	} ;
} ;

struct	E3D_PRIMITIVE_INFINITE_PLANE
{
	union
	{
		PE3D_VECTOR4	vertex ;	// 平面基準点
		INT_PTR			i_vertex ;
	} ;
	union
	{
		PE3D_VECTOR4	xAxis ;		// テクスチャｘ軸
		INT_PTR			i_xAxis ;
	} ;
	union
	{
		PE3D_VECTOR4	yAxis ;		// テクスチャｙ軸
		INT_PTR			i_yAxis ;
	} ;
	REAL32			rFogDeepness ;	// 擬似フォッグ用
	EGL_PALETTE		rgbFogColor ;
} ;

struct	E3D_PRIMITIVE_IMAGE
{
	union
	{
		PE3D_VECTOR4	vCenter ;		// 中心座標
		INT_PTR			i_vCenter ;
	} ;
	union
	{
		PE3D_VECTOR4	vEnlarge ;		// 拡大率
		INT_PTR			i_vEnlarge ;
	} ;
	E3D_VECTOR_2D	vImageBase ;		// 画像の基準座標
	DWORD			dwTransparency ;	// 透明度
	REAL32			rRevolveAngle ;		// 回転角 [rad]
	PEGL_IMAGE_INFO	pImageInf ;			// 画像バッファ（必要であれば）
} ;

struct	E3D_PRIMITIVE_MESH_POLY
{
	DWORD			dwVertexCount ;	// メッシュエントリの頂点数
	DWORD			dwIndex[3] ;	// 頂点の指標
} ;

struct	E3D_PRIMITIVE_MESH
{
	union
	{
		PE3D_VECTOR4	vertices ;		// 頂点座標配列
		INT_PTR			i_vertices ;
	} ;
	union
	{
		PE3D_VECTOR4	normals ;		// 法線配列
		INT_PTR			i_normals ;
	} ;
	union
	{
		E3D_VECTOR_2D	uv_map[1] ;		// UV 座標配列（テクスチャ・非テクスチャ共）
		E3D_COLOR		color[1] ;		// 頂点色
	} ;
} ;

struct	E3D_PRIMITIVE_MESH_LIST
{
	DWORD					dwMeshBytes ;	// メッシュのデータサイズ（dwMeshBytes を含む）
	DWORD					dwPolyCount ;	// メッシュに含まれるポリゴン数
	E3D_PRIMITIVE_MESH_POLY	mpEntries[1] ;
} ;
typedef E3D_PRIMITIVE_MESH_LIST * PE3D_PRIMITIVE_MESH_LIST ;

struct	E3D_PRIMITIVE_POLYGON
{
	DWORD					dwTypeFlag ;
	PE3D_SURFACE_ATTRIBUTE	pSurfaceAttr ;
	DWORD					dwVertexCount ;
	union
	{
		DWORD				dwReserved ;
		DWORD				dwDataSize ;	// データサイズ（dwDataSize の次のアドレスから）
	} ;
	union
	{
		// ポリゴン
		E3D_PRIMITIVE_VERTEX			polygon[1] ;
		// 無限平面（UV付）
		E3D_PRIMITIVE_INFINITE_PLANE	infinite_plane ;
		// 画像
		E3D_PRIMITIVE_IMAGE				image ;
		// ポリゴンメッシュ
		E3D_PRIMITIVE_MESH				mesh ;
	} ;
} ;
typedef	E3D_PRIMITIVE_POLYGON *	PE3D_PRIMITIVE_POLYGON ;
typedef	const E3D_PRIMITIVE_POLYGON *	PCE3D_PRIMITIVE_POLYGON ;

enum	EGLPrimitiveTypeFlag
{
	E3D_FLAT_POLYGON			= 0x00000000,
	E3D_SMOOTH_POLYGON			= 0x00000001,
	E3D_TEXTURE_POLYGON			= 0x00000002,
	E3D_VERTEX_COLOR_POLYGON	= 0x00000004,
	E3D_POLYGON_PRIMITIVE_MASK	= 0x00000007,
	E3D_INFINITE_PLANE			= 0x0000000A,
	E3D_MESH_POLYGON			= 0x00000010,
	E3D_IMAGE_PRIMITIVE			= 0x00000100,
} ;

//
// 平面パラメータ
//
typedef	E3D_VECTOR4	E3D_PLANE_PARAMETER ;
typedef	E3D_PLANE_PARAMETER *		PE3D_PLANE_PARAMETER ;
typedef	const E3D_PLANE_PARAMETER *	PCE3D_PLANE_PARAMETER ;

//
// レンダリングポリゴン情報
//
struct	E3D_POLYGON_ENTRY
{
	DWORD					dwTypeFlag ;
	DWORD					dwTransparency ;
	PE3D_SURFACE_ATTRIBUTE	pAttr ;
	DWORD					dwShadingFlags ;
	union
	{
		struct
		{
			E3D_TEXTURE_MAPINFO		txmap ;
		}					poly ;
		struct
		{
			PEGL_IMAGE_INFO			pInfo ;
			REAL32					rRevolveAngle ;
			E3D_VECTOR_2D			vCenter ;
			E3D_VECTOR_2D			vEnlarge ;
			E3D_VECTOR_2D			vImageBase ;
		}					image ;
		struct
		{
			E3D_VECTOR4					vMinMesh ;
			E3D_VECTOR4					vMaxMesh ;
			PE3D_PRIMITIVE_MESH_LIST	pMesh ;
			PE3D_VECTOR_2D				pUVMap ;
			REAL32						rMeshRadius ;
			void *						pMeshReserved ; // *EGL_RENDER_POLY_MATRIX_PCK4
			void *						pMeshReserved2 ;// *EGL_RENDER_POLY_SPHERE_PCK4
		}					mesh ;
	}					surface ;
	E3D_PLANE_PARAMETER	plane ;
	E3D_VECTOR4			vCenter ;
	DWORD				dwVertexCount ;
	PE3D_VECTOR4		pVertexes ;
	PE3D_VECTOR4		pNormals ;
	DWORD				dwProjectedCount ;
	PE3D_VECTOR_2D		pProjVertexes ;
	PE3D_COLOR			pVertexColors ;
} ;
typedef	E3D_POLYGON_ENTRY *			PE3D_POLYGON_ENTRY ;
typedef	const E3D_POLYGON_ENTRY *	PCE3D_POLYGON_ENTRY ;

//
// 3D 描画パラメータ
//
struct	E3D_RENDER_PARAM
{
	DWORD				dwFlags ;
	DWORD				dwTransparency ;
	E3D_COLOR			rgbaColor ;
	PEGL_IMAGE_INFO		pSrcImage ;
	PEGL_IMAGE_INFO		pLuminousImage ;
	DWORD				nTextureApply ;		// テクスチャ適用度（未使用）
	DWORD				nLuminousApply ;	// 発光テクスチャ適用度
	union
	{
		struct
		{
			E3D_VECTOR			vRenderPos ;
			E3D_VECTOR_2D		vRevCenter ;
			int					nViewVertexes ;
			PCE3D_VECTOR_2D		pViewVertexes ;
			PCE3D_COLOR			pVertexColors ;
			PCE3D_REV_MATRIX	pRevMatrix ;
		}		rev ;
		struct
		{
			int					nViewVertexes ;
			PCE3D_VECTOR4		pVertexes ;
			PCE3D_VECTOR_2D		pViewVertexes ;
			PCE3D_COLOR			pVertexColors ;
		}		poly ;
	} ;
} ;
typedef	E3D_RENDER_PARAM *			PE3D_RENDER_PARAM ;
typedef	const E3D_RENDER_PARAM *	PCE3D_RENDER_PARAM ;

/*	EGL_RENDER_PARAM::dwFlags フラグの取りうる組み合わせ
#define	EGL_WITH_Z_ORDER			0x0004
#define	E3DSAF_TEXTURE_TILING		0x00000100		// テクスチャをタイリング
#define	E3DSAF_TEXTURE_TRIM			0x00000200		// トリミングテクスチャ
#define	E3DSAF_TEXTURE_SMOOTH		0x00000400		// テクスチャ補完拡大
*/
#define	E3DRP_RENDER_REV_IMAGE		0x0010
#define	E3DRP_RENDER_POLYGON		0x0000
#define	E3DRP_Z_ORDER_SCREEN		0x0020
#define	E3DRP_NO_SCREEN_ORIGIN		0x0040


//
// レイトレーシング内部演算用パラメータ
//
struct	EGL_RENDER_POLY_MATRIX_PCK4
{
	REAL32	xVertexA[4] ;					// 基準頂点 A
	REAL32	yVertexA[4] ;
	REAL32	zVertexA[4] ;
	REAL32	xNormal[4] ;					// (A - B) * (A - C) / (|A - B| * |A - C|)
	REAL32	yNormal[4] ;
	REAL32	zNormal[4] ;
	REAL32	xVertexB[4] ;					// 頂点 B
	REAL32	yVertexB[4] ;
	REAL32	zVertexB[4] ;
	REAL32	xVertexC[4] ;					// 頂点 C
	REAL32	yVertexC[4] ;
	REAL32	zVertexC[4] ;
	REAL32	xAB[4] ;						// (A - B) / |A - B|
	REAL32	yAB[4] ;
	REAL32	zAB[4] ;
	REAL32	xAC[4] ;						// (A - C) / |A - C|
	REAL32	yAC[4] ;
	REAL32	zAC[4] ;
	REAL32	xBC[4] ;						// (B - C) / |B - C|
	REAL32	yBC[4] ;
	REAL32	zBC[4] ;
	REAL32	max_ab_ac_sqr[4] ;				// max(|A - B|,|A - C|)^2
	REAL32	error_gap[4] ;					// |A| * error_scale
	REAL32	cos_abc[4] ;					// cos (∠ABC)
	REAL32	cos_acb[4] ;					// cos (∠ACB)
	DWORD	dwReserved[4] ;
} ;

//
// レイトレーシング GPGPU 処理用インターフェース
//

struct	E3D_GPU_PLUGIN_PROPERTY
{
	int	maxThreadsPerBlock ;
	int	maxThreadsDim[3] ;
	int	maxGridSize[3] ;
	int	maxReflection ;
} ;

typedef	struct E3D_GPU_PLUGIN_BUFFER *	HE3D_GPU_PLUGIN_BUFFER ;

#define	E3D_PLUGIN_GPU_RAY_TRACING	0x00010000

struct	E3D_GPU_PLUGIN_RENDER_RESULT
{
	EGL_IMAGE_RECT	rctRender ;
	DWORD *			prgbaImage ;
	REAL32 *		pzBuffer ;
} ;

struct	E3D_GPU_PLUGIN_INTERFACE
{
	DWORD	dwPluginType ;			// = E3D_PLUGIN_GPU_RAY_TRACING
	ESLError (*pfnStartup)( void ) ;
	ESLError (*pfnShutdown)( void ) ;
	//
	ESLError (*pfnInitialize)( int iDev ) ;
	ESLError (*pfnClose)( void ) ;
	ESLError (*pfnAttachThread)( void ) ;
	ESLError (*pfnDetachThread)( void ) ;
	int (*pfnGetDeviceCount)( void ) ;
	ESLError (*pfnGetDeviceName)
		( char * pszBuf, int nBufLen, int iDev ) ;
	ESLError (*pfnGetDeviceProperty)
		( E3D_GPU_PLUGIN_PROPERTY * pProp, int iDev ) ;
	//
	ESLError (*pfnReleaseBuffer)( HE3D_GPU_PLUGIN_BUFFER hBuffer ) ;
	HE3D_GPU_PLUGIN_BUFFER (*pfnCreateRayTracingBuffer)( void ) ;
	ESLError (*pfnSetWorkBufferSize)
		( HE3D_GPU_PLUGIN_BUFFER hBuffer, unsigned int nBytes ) ;
	ESLError (*pfnSetRayTracingTarget)
		( HE3D_GPU_PLUGIN_BUFFER hBuffer,
			PE3D_POLYGON_ENTRY * ppShadowMeshs,
						unsigned int nShadowCount,
			PE3D_POLYGON_ENTRY * ppReflectionMeshs,
						unsigned int nReflectionCount,
			PE3D_POLYGON_ENTRY * ppGlobalRefMeshs,
						unsigned int nGlobalRefCount ) ;
	ESLError (*pfnSetLightEntries)
		( HE3D_GPU_PLUGIN_BUFFER hBuffer,
			unsigned int nLightCount, PCE3D_LIGHT_ENTRY pLightEntries ) ;
	ESLError (*pfnPrepareToTraceRays)
		( HE3D_GPU_PLUGIN_BUFFER hBuffer,
			REAL32 rShadowDistance, REAL32 rReflectionDistance,
			unsigned int nBlockPixelCount, unsigned int nReflectionCount ) ;
	//
	ESLError (*pfnGetResultToTraceRays)
		( HE3D_GPU_PLUGIN_BUFFER hBuffer,
			E3D_GPU_PLUGIN_RENDER_RESULT * pResult ) ;
	ESLError (*pfnBeginTraceRaysRect)
		( HE3D_GPU_PLUGIN_BUFFER hBuffer,
			const EGL_IMAGE_RECT * pRect, const E3D_VECTOR * pScreen,
			unsigned int nReflectionCount ) ;

	ESLError Startup( void )
		{	return	pfnStartup() ;	}
	ESLError Shutdown( void )
		{	return	pfnShutdown() ;	}
	ESLError Initialize( int iDev )
		{	return	pfnInitialize( iDev ) ;	}
	ESLError Close( void )
		{	return	pfnClose() ;	}
	ESLError AttachThread( void )
		{	return	pfnAttachThread( ) ;	}
	ESLError DetachThread( void )
		{	return	pfnDetachThread() ;	}
	int GetDeviceCount( void )
		{	return	pfnGetDeviceCount() ;	}
	ESLError GetDeviceName( char * pszBuf, int nBufLen, int iDev )
		{	return	pfnGetDeviceName( pszBuf, nBufLen, iDev ) ;	}
	ESLError GetDeviceProperty( E3D_GPU_PLUGIN_PROPERTY * pProp, int iDev )
		{	return	pfnGetDeviceProperty( pProp, iDev ) ;	}
	ESLError ReleaseBuffer( HE3D_GPU_PLUGIN_BUFFER hBuffer )
		{	return	pfnReleaseBuffer( hBuffer ) ;	}
	HE3D_GPU_PLUGIN_BUFFER CreateRayTracingBuffer( void )
		{	return	pfnCreateRayTracingBuffer() ;	}
	ESLError SetWorkBufferSize
		( HE3D_GPU_PLUGIN_BUFFER hBuffer, unsigned int nBytes )
		{
			return	pfnSetWorkBufferSize( hBuffer, nBytes ) ;
		}
	ESLError SetRayTracingTarget
		( HE3D_GPU_PLUGIN_BUFFER hBuffer,
			PE3D_POLYGON_ENTRY * ppShadowMeshs,
						unsigned int nShadowCount,
			PE3D_POLYGON_ENTRY * ppReflectionMeshs,
						unsigned int nReflectionCount,
			PE3D_POLYGON_ENTRY * ppGlobalRefMeshs,
						unsigned int nGlobalRefCount )
		{
			return	pfnSetRayTracingTarget
				( hBuffer, ppShadowMeshs, nShadowCount,
						ppReflectionMeshs, nReflectionCount,
						ppGlobalRefMeshs, nGlobalRefCount ) ;
		}
	ESLError SetLightEntries
		( HE3D_GPU_PLUGIN_BUFFER hBuffer,
			unsigned int nLightCount, PCE3D_LIGHT_ENTRY pLightEntries )
		{
			return	pfnSetLightEntries
				( hBuffer, nLightCount, pLightEntries ) ;
		}
	ESLError PrepareToTraceRays
		( HE3D_GPU_PLUGIN_BUFFER hBuffer,
			REAL32 rShadowDistance, REAL32 rReflectionDistance,
				unsigned int nBlockPixelCount, unsigned int nReflectionCount )
		{
			return	pfnPrepareToTraceRays
				( hBuffer, rShadowDistance, rReflectionDistance,
								nBlockPixelCount, nReflectionCount ) ;
		}
	ESLError GetResultToTraceRays
		( HE3D_GPU_PLUGIN_BUFFER hBuffer,
			E3D_GPU_PLUGIN_RENDER_RESULT & result )
		{
			return	pfnGetResultToTraceRays( hBuffer, &result ) ;
		}
	ESLError BeginTraceRaysRect
		( HE3D_GPU_PLUGIN_BUFFER hBuffer,
			const EGL_IMAGE_RECT & rect, const E3D_VECTOR & vScreen,
			unsigned int nReflectionCount )
		{
			return	pfnBeginTraceRaysRect
				( hBuffer, &rect, &vScreen, nReflectionCount ) ;
		}
} ;

typedef	E3D_GPU_PLUGIN_INTERFACE *	PE3D_GPU_PLUGIN_INTERFACE ;

typedef	E3D_GPU_PLUGIN_INTERFACE *
		(*GPUPluginInterfaceProc)( DWORD_PTR dwType, int nReserved ) ;

class	E3DGPUPlugin
{
protected:
	HMODULE						m_hModule ;
	E3D_GPU_PLUGIN_INTERFACE *	m_pgpupi ;

public:
	E3DGPUPlugin( void ) : m_hModule(NULL), m_pgpupi(NULL) {}
	~E3DGPUPlugin( void )
		{	Release( ) ;	}
	ESLError LoadModule( const char * pszFileName )
		{
			Release( ) ;
			m_hModule = ::LoadLibrary( pszFileName ) ;
			if ( m_hModule == NULL )
			{
				return	eslErrGeneral ;
			}
			GPUPluginInterfaceProc	pfnEntryPoint =
				(GPUPluginInterfaceProc) ::GetProcAddress
						( m_hModule, "EntryPointProc" ) ;
			if ( pfnEntryPoint == NULL )
			{
				return	eslErrGeneral ;
			}
			m_pgpupi = pfnEntryPoint( E3D_PLUGIN_GPU_RAY_TRACING, 0 ) ;
			if ( m_pgpupi == NULL )
			{
				return	eslErrGeneral ;
			}
			if ( m_pgpupi->dwPluginType != E3D_PLUGIN_GPU_RAY_TRACING )
			{
				m_pgpupi = NULL ;
				return	eslErrGeneral ;
			}
			return	m_pgpupi->Startup( ) ;
		}
	void Release( void )
		{
			if ( m_pgpupi != NULL )
			{
				m_pgpupi->Shutdown( ) ;
				m_pgpupi = NULL ;
			}
			if ( m_hModule != NULL )
			{
				::FreeLibrary( m_hModule ) ;
				m_hModule = NULL ;
			}
		}

public:
	E3D_GPU_PLUGIN_INTERFACE * Interface( void ) const
		{
			return	m_pgpupi ;
		}
	ESLError Initialize( int iDev )
		{
			ESLAssert( m_pgpupi != NULL ) ;
			return	m_pgpupi->pfnInitialize( iDev ) ;
		}
	ESLError Close( void )
		{
			ESLAssert( m_pgpupi != NULL ) ;
			return	m_pgpupi->pfnClose() ;
		}
	ESLError GetDeviceName( char * pszBuf, int nBufLen, int iDev )
		{
			ESLAssert( m_pgpupi != NULL ) ;
			return	m_pgpupi->pfnGetDeviceName( pszBuf, nBufLen, iDev ) ;
		}
	ESLError GetDeviceProperty( E3D_GPU_PLUGIN_PROPERTY * pProp, int iDev )
		{
			ESLAssert( m_pgpupi != NULL ) ;
			return	m_pgpupi->pfnGetDeviceProperty( pProp, iDev ) ;
		}
	ESLError pfnReleaseBuffer( HE3D_GPU_PLUGIN_BUFFER hBuffer )
		{
			ESLAssert( m_pgpupi != NULL ) ;
			return	m_pgpupi->pfnReleaseBuffer( hBuffer ) ;
		}
	HE3D_GPU_PLUGIN_BUFFER CreateRayTracingBuffer( void )
		{
			ESLAssert( m_pgpupi != NULL ) ;
			return	m_pgpupi->pfnCreateRayTracingBuffer() ;
		}
	ESLError SetRayTracingTarget
		( HE3D_GPU_PLUGIN_BUFFER hBuffer,
			PE3D_POLYGON_ENTRY * ppShadowMeshs,
						unsigned int nShadowCount,
			PE3D_POLYGON_ENTRY * ppReflectionMeshs,
						unsigned int nReflectionCount,
			PE3D_POLYGON_ENTRY * ppGlobalRefMeshs,
						unsigned int nGlobalRefCount )
		{
			ESLAssert( m_pgpupi != NULL ) ;
			return	m_pgpupi->pfnSetRayTracingTarget
				( hBuffer, ppShadowMeshs, nShadowCount,
						ppReflectionMeshs, nReflectionCount,
						ppGlobalRefMeshs, nGlobalRefCount ) ;
		}
	ESLError SetLightEntries
		( HE3D_GPU_PLUGIN_BUFFER hBuffer,
			unsigned int nLightCount, PCE3D_LIGHT_ENTRY pLightEntries )
		{
			ESLAssert( m_pgpupi != NULL ) ;
			return	m_pgpupi->pfnSetLightEntries
				( hBuffer, nLightCount, pLightEntries ) ;
		}
} ;


//
// ポリゴン描画
//
typedef	struct EGL_RENDER_POLYGON *	HEGL_RENDER_POLYGON ;

struct	EGL_RENDER_RAY_TRACE_PARAM
{
	DWORD	dwFlags ;					// フラグ (EGLRenderRayTraceFlag)
	REAL32	rShadowingDistance ;		// 影を落とす有効距離
	REAL32	rRayTracingDistance ;		// 光線追跡有効距離
	DWORD	dwRayReflectCount ;			// 光線追跡回数
	DWORD	dwAppendShadowingCount ;	// 陰光線追跡回数（追加）
} ;

enum	EGLRenderRayTraceFlag
{
	E3D_RAYTRACE_SHADOWING			= 0x0001,		// 影を落とす
	E3D_RAYTRACE_SHADOW_ALPHA		= 0x0002,		// 影に透明度を考慮する
	E3D_RAYTRACE_SHADOWING_APPEND	= 0x0004,		// dwAppendShadowingCount を有効にする
	E3D_RAYTRACE_REFLECTION			= 0x0010,		// 反射を有効にする
	E3D_RAYTRACE_REFRACTION			= 0x0020,		// 屈折を有効にする
	E3D_RAYTRACE_ONLY_GLOBAL_REF	= 0x0040,		// 大域反射・屈折のみ有効にする
} ;

struct	EGL_RENDER_POLYGON
{
	ESLError (*pfnRelease)( HEGL_RENDER_POLYGON hRenderPoly ) ;
	ESLError (*pfnInitialize)
		( HEGL_RENDER_POLYGON hRenderPoly,
			PEGL_IMAGE_INFO pDstImage,
			PCEGL_RECT pClipRect,
			PEGL_IMAGE_INFO pZBuffer, PCE3D_VECTOR pScreenPos ) ;
	ESLError (*pfnInitializeToReference)
		( HEGL_RENDER_POLYGON hRenderPoly,
			HEGL_RENDER_POLYGON hRefRenderPoly,
			PCEGL_RECT pClipRect, DWORD dwFlags ) ;
	PCE3D_VECTOR (*pfnGetScreenPos)( HEGL_RENDER_POLYGON hRenderPoly ) ;
	HEGL_DRAW_IMAGE (*pfnGetDrawImage)( HEGL_RENDER_POLYGON hRenderPoly ) ;
	DWORD (*pfnGetFunctionFlags)( HEGL_RENDER_POLYGON hRenderPoly ) ;
	DWORD (*pfnSetFunctionFlags)
		( HEGL_RENDER_POLYGON hRenderPoly, DWORD dwFlags ) ;
	ESLError (*pfnPrepareMatrix)
		( HEGL_RENDER_POLYGON hRenderPoly,
			const E3D_REV_MATRIX * matrix,
			PCE3D_VECTOR pOrigin, PCE3D_VECTOR pEnlarge ) ;
	ESLError (*pfnRevolveMatrix)
		( HEGL_RENDER_POLYGON hRenderPoly,
			PE3D_VECTOR4 pDst, PCE3D_VECTOR4 pSrc, unsigned int nVectorCount ) ;
	ESLError (*pfnSetZClipRange)
		( HEGL_RENDER_POLYGON hRenderPoly, REAL32 rMin, REAL32 rMax ) ;
	ESLError (*pfnSetEnvironmentMapping)
		( HEGL_RENDER_POLYGON hRenderPoly,
			const E3D_REV_MATRIX * matrix,
			const E3D_ENVIRONMENT_MAPPING * envmap ) ;
	ESLError (*pfnPrepareLight)
		( HEGL_RENDER_POLYGON hRenderPoly,
			HSTACKHEAP hStackHeap, unsigned int nLightCount,
								PCE3D_LIGHT_ENTRY pLightEntries ) ;
	E3D_POLYGON_ENTRY * (*pfnCreatePolygonEntry)
		( HEGL_RENDER_POLYGON hRenderPoly,
			HSTACKHEAP hStackHeap, PCE3D_PRIMITIVE_POLYGON pPrimitive ) ;
	E3D_POLYGON_ENTRY * (*pfnMakeUpPolygon)
		( HEGL_RENDER_POLYGON hRenderPoly,
			HSTACKHEAP hStackHeap, E3D_POLYGON_ENTRY * pPolyEntry ) ;
	ESLError (*pfnApplyAttribute)
		( HEGL_RENDER_POLYGON hRenderPoly,
			E3D_POLYGON_ENTRY * pPolyEntry,
				const E3D_COLOR * pColor, unsigned int nTransparency ) ;
	ESLError (*pfnGetExternalRect)
		( HEGL_RENDER_POLYGON hRenderPoly, EGL_RECT * pExtRect,
			PCE3D_POLYGON_ENTRY * pPolyEntry, unsigned int nCount ) ;
	ESLError (*pfnShadeReflectLights)
		( HEGL_RENDER_POLYGON hRenderPoly,
			PCE3D_SURFACE_ATTRIBUTE pSurfaceAttribute,
			PCE3D_VECTOR4 pNormals, PCE3D_VECTOR4 pFocusPoints,
			E3D_COLOR * pColorLooks, unsigned int nCount ) ;
	ESLError (*pfnProjectScreen)
		( HEGL_RENDER_POLYGON hRenderPoly, PE3D_VECTOR_2D pDst,
				PCE3D_VECTOR4 pSrc, unsigned int nVertexCount ) ;
	ESLError (*pfnSortPolygonEntry)
		( HEGL_RENDER_POLYGON hRenderPoly,
			PE3D_POLYGON_ENTRY * ppPolygons,
			unsigned int nPolygonCount, DWORD dwSortingFlags ) ;
	ESLError (*pfnPrepareRender)
		( HEGL_RENDER_POLYGON hRenderPoly, PCE3D_POLYGON_ENTRY pPolyEntry ) ;
	ESLError (*pfnPrepareRenderParam)
		( HEGL_RENDER_POLYGON hRenderPoly, PCE3D_RENDER_PARAM pRenderParam ) ;
	ESLError (*pfnRenderPolygon)( HEGL_RENDER_POLYGON hRenderPoly ) ;
	// (ver.3.09 以降)
	ESLError (*pfnSetRayTracingParameter)
		( HEGL_RENDER_POLYGON hRenderPoly,
			const EGL_RENDER_RAY_TRACE_PARAM * prrtp ) ;
	ESLError (*pfnAttachRayTracingTarget)
		( HEGL_RENDER_POLYGON hRenderPoly, HSTACKHEAP hStackHeap,
			E3D_POLYGON_ENTRY ** ppShadowingPoly, unsigned int nShadowingCount,
			E3D_POLYGON_ENTRY ** ppRayTracingPoly, unsigned int nRayTracingCount,
			E3D_POLYGON_ENTRY ** ppGlobalReflection, unsigned int nGlobalReflections ) ;
	E3D_POLYGON_ENTRY * (*pfnCreatePolygonEntryRT)
		( HEGL_RENDER_POLYGON hRenderPoly, HSTACKHEAP hStackHeap,
			PCE3D_PRIMITIVE_POLYGON pPrimitive, DWORD * pdwResult ) ;
	ESLError (*pfnPrepareRenderRayTracing)
		( HEGL_RENDER_POLYGON hRenderPoly, PCEGL_RECT pRenderRect ) ;
	ESLError (*pfnRenderRayTracing)
		( HEGL_RENDER_POLYGON hRenderPoly, HEGL_RENDER_POLYGON hSyncRender ) ;
	ESLError (*pfnAbortRenderRayTracing)
		( HEGL_RENDER_POLYGON hRenderPoly ) ;
	ESLError (*pfnGetProgressRayTracing)
		( HEGL_RENDER_POLYGON hRenderPoly, DWORD * pdwPixelCount ) ;
	DWORD (*pfnAllocateRayTraceRect)
		( HEGL_RENDER_POLYGON hRenderPoly,
			PEGL_RECT pRenderRect, int nLineCount, int nOffset ) ;

	// ポリゴンレンダリングオブジェクト解放
	inline ESLError Release( void )
		{
			return	(*pfnRelease)( this ) ;
		}
	// ポリゴンレンダリングオブジェクト初期化
	inline ESLError Initialize
		( PEGL_IMAGE_INFO pDstImage, PCEGL_RECT pClipRect,
			PEGL_IMAGE_INFO pZBuffer, PCE3D_VECTOR pScreenPos )
		{
			return	(*pfnInitialize)
				( this, pDstImage, pClipRect, pZBuffer, pScreenPos ) ;
		}
	inline ESLError InitializeToReference
		( HEGL_RENDER_POLYGON hRefRenderPoly,
			PCEGL_RECT pClipRect, DWORD dwFlags = 0 )
		{
			return	(*pfnInitializeToReference)
				( this, hRefRenderPoly, pClipRect, dwFlags ) ;
		}
	// 投影スクリーン座標を取得
	inline const E3D_VECTOR & GetScreenPos( void )
		{
			return	*((*pfnGetScreenPos)( this )) ;
		}
	// 画像描画オブジェクト取得
	inline HEGL_DRAW_IMAGE GetDrawImage( void )
		{
			return	(*pfnGetDrawImage)( this ) ;
		}
	// 機能フラグを取得する
	inline DWORD GetFunctionFlags( void )
		{
			return	(*pfnGetFunctionFlags)( this ) ;
		}
	// 機能フラグを設定する
	inline DWORD SetFunctionFlags( DWORD dwFlags )
		{
			return	(*pfnSetFunctionFlags)( this, dwFlags ) ;
		}
	// 座標変換準備
	inline ESLError PrepareMatrix
		( const E3D_REV_MATRIX * matrix,
			PCE3D_VECTOR pOrigin, PCE3D_VECTOR pEnlarge )
		{
			return	(*pfnPrepareMatrix)( this, matrix, pOrigin, pEnlarge ) ;
		}
	// 座標変換
	inline ESLError RevolveMatrix
		( PE3D_VECTOR4 pDst, PCE3D_VECTOR4 pSrc, unsigned int nVectorCount )
		{
			return	(*pfnRevolveMatrix)( this, pDst, pSrc, nVectorCount ) ;
		}
	// Z クリッピング設定
	inline ESLError SetZClipRange( REAL32 rMin, REAL32 rMax )
		{
			return	(*pfnSetZClipRange)( this, rMin, rMax ) ;
		}
	// 環境マッピング設定
	inline ESLError SetEnvironmentMapping
		( const E3D_REV_MATRIX * matrix,
			const E3D_ENVIRONMENT_MAPPING * envmap )
		{
			return	(*pfnSetEnvironmentMapping)( this, matrix, envmap ) ;
		}
	// ライト設定
	inline ESLError PrepareLight
			( HSTACKHEAP hStackHeap,
				unsigned int nLightCount, PCE3D_LIGHT_ENTRY pLightEntries )
		{
			return	(*pfnPrepareLight)
				( this, hStackHeap, nLightCount, pLightEntries ) ;
		}
	// レンダリング用ポリゴン情報作成
	inline E3D_POLYGON_ENTRY * CreatePolygonEntry
			( HSTACKHEAP hStackHeap, PCE3D_PRIMITIVE_POLYGON pPrimitive )
		{
			return	(*pfnCreatePolygonEntry)( this, hStackHeap, pPrimitive ) ;
		}
	// ポリゴンにシェーディング・透視変換を施す
	inline E3D_POLYGON_ENTRY * MakeUpPolygon
			( HSTACKHEAP hStackHeap, E3D_POLYGON_ENTRY * pPolyEntry )
		{
			return	(*pfnMakeUpPolygon)( this, hStackHeap, pPolyEntry ) ;
		}
	// ポリゴンに属性を設定する
	inline ESLError ApplyAttribute
			( E3D_POLYGON_ENTRY * pPolyEntry,
				const E3D_COLOR * pColor, unsigned int nTransparency )
		{
			return	(*pfnApplyAttribute)
				( this, pPolyEntry, pColor, nTransparency ) ;
		}
	// 最小外接矩形を取得する
	inline ESLError GetExternalRect
			( EGL_RECT * pExtRect,
				PCE3D_POLYGON_ENTRY * pPolyEntry, unsigned int nCount )
		{
			return	(*pfnGetExternalRect)
				( this, pExtRect, pPolyEntry, nCount ) ;
		}
	// シェーディング
	inline ESLError ShadeReflectLights
		( PCE3D_SURFACE_ATTRIBUTE pSurfaceAttribute,
			PCE3D_VECTOR4 pNormals, PCE3D_VECTOR4 pFocusPoints,
				E3D_COLOR * pColorLooks, unsigned int nCount = 1 )
		{
			return	(*pfnShadeReflectLights)
				( this, pSurfaceAttribute,
					pNormals, pFocusPoints, pColorLooks, nCount ) ;
		}
	// 透視変換
	inline ESLError ProjectScreen
			( PE3D_VECTOR_2D pDst,
				PCE3D_VECTOR4 pSrc, unsigned int nVertexCount )
		{
			return	(*pfnProjectScreen)( this, pDst, pSrc, nVertexCount ) ;
		}
	// ポリゴンをソート
	inline ESLError SortPolygonEntry
		( PE3D_POLYGON_ENTRY * ppPolygons,
			unsigned int nPolygonCount, DWORD dwSortingFlags )
		{
			return	(*pfnSortPolygonEntry)
				( this, ppPolygons, nPolygonCount, dwSortingFlags ) ;
		}
	// レンダリング準備
	inline ESLError PrepareRender( PCE3D_POLYGON_ENTRY pPolyEntry )
		{
			return	(*pfnPrepareRender)( this, pPolyEntry ) ;
		}
	// レンダリング準備
	inline ESLError PrepareRenderParam( PCE3D_RENDER_PARAM pRenderParam )
		{
			return	(*pfnPrepareRenderParam)( this, pRenderParam ) ;
		}
	// レンダリング
	inline ESLError RenderPolygon( void )
		{
			return	(*pfnRenderPolygon)( this ) ;
		}
	// (ver.3.09 以降)
	// レイトレーシングパラメータを設定する
	inline ESLError SetRayTracingParameter
			( const EGL_RENDER_RAY_TRACE_PARAM * prrtp )
		{
			return	(*pfnSetRayTracingParameter)( this, prrtp ) ;
		}
	// レイトレーシングターゲットを設定する
	inline ESLError AttachRayTracingTarget
		( HSTACKHEAP hStackHeap,
			E3D_POLYGON_ENTRY ** ppShadowingPoly, unsigned int nShadowingCount,
			E3D_POLYGON_ENTRY ** ppRayTracingPoly, unsigned int nRayTracingCount,
			E3D_POLYGON_ENTRY ** ppGlobalReflection, unsigned int nGlobalReflections )
		{
			return	(*pfnAttachRayTracingTarget)
				( this, hStackHeap, ppShadowingPoly, nShadowingCount,
						ppRayTracingPoly, nRayTracingCount,
						ppGlobalReflection, nGlobalReflections ) ;
		}
	// レンダリング用ポリゴン情報作成（レイトレーシング用）
	inline E3D_POLYGON_ENTRY * CreatePolygonEntryRT
		( HSTACKHEAP hStackHeap,
			PCE3D_PRIMITIVE_POLYGON pPrimitive, DWORD * pdwResult )
		{
			return	(*pfnCreatePolygonEntryRT)
					( this, hStackHeap, pPrimitive, pdwResult ) ;
		}
	// レイトレーシング準備
	inline ESLError PrepareRenderRayTracing( PCEGL_RECT pRenderRect = NULL )
		{
			return	(*pfnPrepareRenderRayTracing)( this, pRenderRect ) ;
		}
	// レイトレーシング実行
	inline ESLError RenderRayTracing( HEGL_RENDER_POLYGON hSyncRender = NULL )
		{
			return	(*pfnRenderRayTracing)( this, hSyncRender ) ;
		}
	// レイトレーシング中断
	ESLError AbortRenderRayTracing( void )
		{
			return	(*pfnAbortRenderRayTracing)( this ) ;
		}
	// レイトレーシング進行状況取得
	ESLError GetProgressRayTracing( DWORD * pdwPixelCount )
		{
			return	(*pfnGetProgressRayTracing)( this, pdwPixelCount ) ;
		}
	// マルチスレッドレイトレーシングでレンダリング領域を確保する
	DWORD AllocateRayTraceRect
			( PEGL_RECT pRenderRect, int nLineCount, int nOffset )
		{
			return	(*pfnAllocateRayTraceRect)
				( this, pRenderRect, nLineCount, nOffset ) ;
		}

} ;

enum	EGLRenderPolygonInitializeToReferenceFlag
{
	E3D_FLAG_INIT_REF_GPGPU_BUF		= 0x0100,
} ;

enum	EGLRenderPolygonFuncFlag
{
	E3D_FLAG_ANTIALIAS_SIDE_EDGE	= 0x0001,
	E3D_FLAG_TEXTURE_SMOOTHING		= 0x0002,
	E3D_FLAG_PHONG_SHADING			= 0x0004,
	E3D_FLAG_RAY_SHADOWING			= 0x0010,
	E3D_FLAG_RAY_REFLECTING			= 0x0020,
	E3D_FLAG_RAY_REFRACTING			= 0x0040,
	E3D_FLAG_RAY_TRACING			= 0x0074,
	E3D_FLAG_ENABLE_SSE2			= 0x0100,
} ;

enum	EGLRenderPolygonSortFlag
{
	E3D_SORT_TRANSPARENT	= 0x0001,
	E3D_SORT_OPAQUE			= 0x0002,
} ;

enum	EGLRenderCreatePolygonEntryResult
{
	E3D_RTCPE_RESULT_NO_RENDER		= 0,		// 視界外
	E3D_RTCPE_RESULT_SHOULD_RENDER	= 1,		// 視界内
	E3D_RTCPE_RESULT_NO_SHADOWING	= 2,		// 陰オブジェクト対象外
	E3D_RTCPE_RESULT_NO_REFLECTING	= 4,		// 反射オブジェクト対象外
} ;

//
// モデル当たり判定
//
typedef	struct EGL_MODEL_MATRIX *	HEGL_MODEL_MATRIX ;

struct	EGL_MODEL_MATRIX
{
	ESLError (*pfnRelease)( HEGL_MODEL_MATRIX hMatrix ) ;
	ESLError (*pfnInitialize)
		( HEGL_MODEL_MATRIX hMatrix,
			PCE3D_PRIMITIVE_POLYGON * pModel, unsigned int nPolyCount ) ;
	ESLError (*pfnIsHitInclusiveSphere)
		( HEGL_MODEL_MATRIX hMatrix,
			const E3D_VECTOR * pSphere, REAL32 rRadius, int * pHitResult ) ;
	ESLError (*pfnIsHitAgainstSphere)
		( HEGL_MODEL_MATRIX hMatrix,
			const E3D_VECTOR * pSphere, REAL32 rRadius,
				int * pHitResult, E3D_VECTOR * pHitPos,
				E3D_VECTOR * pHitNormal, E3D_VECTOR * pReflection ) ;
	ESLError (*pfnIsCrossingSegment)
		( HEGL_MODEL_MATRIX hMatrix,
			const E3D_VECTOR * pPos0, const E3D_VECTOR * pPos1,
				REAL32 rErrorGap, int * pHitResult, E3D_VECTOR * pHitPos,
				E3D_VECTOR * pHitNormal, E3D_VECTOR * pReflection ) ;

	// 当たり判定オブジェクト解放
	inline ESLError Release( void )
		{
			return	(*pfnRelease)( this ) ;
		}
	// 当たり判定オブジェクト初期化
	inline ESLError Initialize
			( PCE3D_PRIMITIVE_POLYGON * pModel, unsigned int nPolyCount )
		{
			return	(*pfnInitialize)( this, pModel, nPolyCount ) ;
		}
	// 内包空間と球体との交差判定
	inline ESLError IsHitInclusiveSphere
			( const E3D_VECTOR * pSphere, double rRadius, int * pHitResult )
		{
			return	(*pfnIsHitInclusiveSphere)
				( this, pSphere, (REAL32) rRadius, pHitResult ) ;
		}
	// ポリゴンと球体との交差判定
	inline ESLError IsHitAgainstSphere
		( const E3D_VECTOR * pSphere, double rRadius,
				int * pHitResult, E3D_VECTOR * pHitPos,
				E3D_VECTOR * pHitNormal, E3D_VECTOR * pReflection )
		{
			return	(*pfnIsHitAgainstSphere)
				( this, pSphere, (REAL32) rRadius,
					pHitResult, pHitPos, pHitNormal, pReflection ) ;
		}
	// ポリゴンと線分との交差判定
	inline ESLError IsCrossingSegment
		( const E3D_VECTOR * pPos0, const E3D_VECTOR * pPos1,
			double rErrorGap, int * pHitResult, E3D_VECTOR * pHitPos,
			E3D_VECTOR * pHitNormal, E3D_VECTOR * pReflection )
		{
			return	(*pfnIsCrossingSegment)
				( this, pPos0, pPos1, (REAL32) rErrorGap,
					pHitResult, pHitPos, pHitNormal, pReflection ) ;
		}
} ;


//
// クロス（布）シミュレーションモデル
//
typedef	struct EGL_CLOTH_MODEL_MORPH *	HEGL_CLOTH_MODEL_MORPH ;

struct	HINDER_MODEL_INFO
{
	HEGL_MODEL_MATRIX	hMatrix ;
	DWORD				dwFlags ;			// complex of HinderModelFlags
	REAL32				rGapRadius ;
	DWORD				dwReserved ;
} ;

enum	HinderModelFlags
{
	HMI_FLAG_INCLUSIVE	= 0,		// not supported, assigned to HMI_FLAG_SURFACE
	HMI_FLAG_SURFACE	= 1,
	HMI_FLAG_CROSSING	= 2,		// not supported
} ;

struct	EGL_CLOTH_ATTRIBUTE
{
	DWORD	dwFlags ;				// フラグ（＝０）
	REAL32	rSoftness ;				// 柔らかさ（曲がりに対する）
	REAL32	rElastically ;			// 伸縮性（伸びに対する）
	REAL32	rDamping ;				// 速度の減衰係数（1.0で無減衰）
	REAL32	rWeight ;				// 重さ
	DWORD	nEffectLayers ;			// 適用階層数
	DWORD	dwReserved[2] ;
} ;

struct	EGL_CLOTH_MORPH_PARAMETER
{
	E3D_VECTOR4		vGravity ;			// 重力加速度 [/sec^2]
	E3D_VECTOR4		vStream ;			// 風速 [/sec]
	E3D_VECTOR4		vMoveDelta ;		// デルタ移動量（加速度） [/frame^2]
	PE3D_REV_MATRIX	pBaseMatrix ;		// 親空間変換行列
	DWORD			dwFlags ;			// フラグ
	REAL32			rDeltaTime ;		// デルタ時間 [sec]
	REAL32			rDensity ;			// 空気密度（風比重）
	REAL32			rStreamFluctuation ;// 風速揺らぎ率
	REAL32			rStreamFrequency ;	// 風速揺らぎ周期 [sec]
	DWORD			dwReserved[2] ;
	E3D_VECTOR4		vAirBallCenter ;	// 空気パーティクル発生エリア中心
	DWORD			nAirBallLimit ;		// 空気パーティクル最大数
	DWORD			nAirBallCount ;		// 空気パーティクル発生数 [256/sec]
	REAL32			rAirBallDuration ;	// 空気パーティクル最大寿命 [sec]
	REAL32			rAirBallPressure ;	// 空気パーティクル圧力
	REAL32			rAirBallRadius ;	// 空気パーティクル半径
	REAL32			rAirBallDensity ;	// 空気パーティクル比重
	REAL32			rAirBallAreaRadius ;// 空気パーティクル発生エリア半径
} ;

enum	ClothMorphFlags
{
	CMP_FLAG_NO_STREAM_EFFECT	= 0x01,
	CMP_FLAG_AIR_PARTICLE		= 0x02,
} ;

struct	EGL_CLOTH_MODEL_MORPH
{
	ESLError (*pfnRelease)( HEGL_CLOTH_MODEL_MORPH hCloth ) ;
	ESLError (*pfnDeleteCloth)( HEGL_CLOTH_MODEL_MORPH hCloth ) ;
	ESLError (*pfnInitializeCloth)( HEGL_CLOTH_MODEL_MORPH hCloth ) ;
	ESLError (*pfnSetHinderModel)
		( HEGL_CLOTH_MODEL_MORPH hCloth,
			const HINDER_MODEL_INFO * phmiHinder, unsigned int nCount ) ;
	ESLError (*pfnWeaveCloth)
		( HEGL_CLOTH_MODEL_MORPH hCloth,
			const EGL_CLOTH_ATTRIBUTE * pAttribute,
			PCE3D_PRIMITIVE_POLYGON pMesh,
			const REAL32 * pVertexApply,
			unsigned int iApplyFirst, unsigned int nApplyCount ) ;
	ESLError (*pfnPatchCloth)
		( HEGL_CLOTH_MODEL_MORPH hCloth,
			const EGL_CLOTH_ATTRIBUTE * pAttribute,
			PCE3D_PRIMITIVE_POLYGON pMesh, REAL32 rPathRadius,
			const REAL32 * pVertexApply,
			unsigned int iApplyFirst, unsigned int nApplyCount ) ;
	ESLError (*pfnMorphCloth)
		( HEGL_CLOTH_MODEL_MORPH hCloth,
			const EGL_CLOTH_MORPH_PARAMETER * pcmp ) ;
	ESLError (*pfnMorphMesh)
		( HEGL_CLOTH_MODEL_MORPH hCloth,
			const EGL_CLOTH_MORPH_PARAMETER * pcmp ) ;

	inline ESLError Release( void )
		{
			return	pfnRelease( this ) ;
		}
	inline ESLError DeleteCloth( void )
		{
			return	pfnDeleteCloth( this ) ;
		}
	inline ESLError InitializeCloth( void )
		{
			return	pfnInitializeCloth( this ) ;
		}
	inline ESLError SetHinderModel
			( const HINDER_MODEL_INFO * phmiHinder, unsigned int nCount )
		{
			return	pfnSetHinderModel( this, phmiHinder, nCount ) ;
		}
	inline ESLError WeaveCloth
			( const EGL_CLOTH_ATTRIBUTE * pAttribute,
				PCE3D_PRIMITIVE_POLYGON pMesh,
				const REAL32 * pVertexApply,
				unsigned int iApplyFirst, unsigned int nApplyCount )
		{
			return	pfnWeaveCloth
				( this, pAttribute, pMesh,
					pVertexApply, iApplyFirst, nApplyCount ) ;
		}
	inline ESLError PatchCloth
			( const EGL_CLOTH_ATTRIBUTE * pAttribute,
				PCE3D_PRIMITIVE_POLYGON pMesh, REAL32 rPathRadius,
				const REAL32 * pVertexApply,
				unsigned int iApplyFirst, unsigned int nApplyCount )
		{
			return	pfnPatchCloth
				( this, pAttribute,pMesh, rPathRadius,
					pVertexApply, iApplyFirst, nApplyCount ) ;
		}
	inline ESLError MorphCloth
			( const EGL_CLOTH_MORPH_PARAMETER * pcmp )
		{
			return	pfnMorphCloth( this, pcmp ) ;
		}
	inline ESLError MorphMesh
			( const EGL_CLOTH_MORPH_PARAMETER * pcmp )
		{
			return	pfnMorphMesh( this, pcmp ) ;
		}

} ;

//
// EGL 関数
//
extern	"C"
{
	PE3D_POLYGON_REGION eglNormalizePolygonRegion
		( PE3D_POLYGON_REGION pPolyRegion, PCEGL_RECT pClipRect,
			unsigned int nVertexCount, PCE3D_VECTOR_2D pPolyVertexes,
			PCE3D_COLOR pVertexColors = NULL, PCE3D_VECTOR4 pNormals = NULL ) ;
	HEGL_RENDER_POLYGON eglCreateRenderPolygon( void ) ;
	HEGL_RENDER_POLYGON eglCurrentRenderPolygon( void ) ;
	HEGL_MODEL_MATRIX eglCreateModelMatrix( void ) ;
	HEGL_CLOTH_MODEL_MORPH eglCreateClothModelMorph( void ) ;
} ;


#endif
