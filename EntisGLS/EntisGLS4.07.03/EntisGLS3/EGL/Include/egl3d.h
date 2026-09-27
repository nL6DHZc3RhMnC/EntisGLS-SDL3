
/*****************************************************************************
                          Entis Graphic Library
 -----------------------------------------------------------------------------
    Copyright (c) 2002-2010 Leshade Entis, Entis-soft. Al rights reserved.
 *****************************************************************************/


#if	!defined(__EGL3D_H__)
#define	__EGL3D_H__

#if	!defined(COMPACT_NOA_DECODER)

//////////////////////////////////////////////////////////////////////////////
// ベジェ曲線
//////////////////////////////////////////////////////////////////////////////

template <class _T> class	EBezierCurves
{
protected:
	_T *			m_cp ;
	unsigned int	m_count ;

public:
	// 構築関数
	EBezierCurves( void ) : m_cp(NULL), m_count(0) { }
	EBezierCurves( int nCount )
		: m_cp(NULL), m_count(0) { SetCount( nCount ) ; }
	// 消滅関数
	~EBezierCurves( void )
		{
			if ( m_cp != NULL )
			{
				eslHeapFree( NULL, m_cp, 0 ) ;
			}
		}
	// 制御点数取得
	int GetCount( void ) const
		{
			return	m_count ;
		}
	// 制御点数設定
	void SetCount( int nCount )
		{
			m_cp = (_T*) eslHeapReallocate
				( NULL, m_cp, nCount * sizeof(_T), 0 ) ;
			m_count = nCount ;
		}
	// 制御点アクセス
	_T & operator [] ( int index )
		{
			if( (unsigned int) index >= m_count )
			{
				SetCount( index + 1 ) ;
			}
			return	m_cp[index] ;
		}
	_T operator [] ( int index ) const
		{
			ESLAssert( (unsigned int) index < m_count ) ;
			return	m_cp[index] ;
		}
	_T * GetArrayPtr( void ) const
		{
			return	m_cp ;
		}
	// ベジェ曲線分割位置取得
	int GetDividedPosition( double & t ) const
		{
			int		n = (m_count - 1) / 3 ;
			int		m ;
			for ( m = 0; m < (n - 1); m ++ )
			{
				if ( t <= (double) (m + 1) / n )
				{
					break ;
				}
			}
			if ( n > 0 )
			{
				t = (t - (double) m / n) * n ;
			}
			return	m * 3 ;
		}
	// 直線設定
	void SetLine( const _T & p0, const _T & p1, double v0 = 1, double v1 = 1 )
		{
			ESLAssert( m_count >= 4 ) ;
			_T	d = (p1 - p0) * (1.0 / 3.0) ;
			m_cp[0] = p0 ;
			m_cp[1] = p0 + d * v0 ;
			m_cp[2] = p1 - d * v1 ;
			m_cp[3] = p1 ;
		}
	// 曲線設定
	void SetCurveUnsmoothSpeed
		( const _T & p0, const _T & p1, const _T & p2,
			double v0, double v1, double v2, double v3 )
		{
			ESLAssert( m_count >= 7 ) ;
			const double	r = 2.0 / 3.0 ;
			_T	a = (p2 - p0) ;
			_T	b = p1 - a * (v1 * 0.25) ;
			_T	c = p1 + a * (v2 * 0.25) ;
			m_cp[0] = p0 ;
			m_cp[3] = p1 ;
			m_cp[6] = p2 ;
			m_cp[2] = p1 - a * (v1 * (r * 0.25)) ;
			m_cp[4] = p1 + a * (v2 * (r * 0.25)) ;
			m_cp[1] = p0 + (b - p0) * (v0 * r) ;
			m_cp[5] = p2 + (c - p2) * (v3 * r) ;
		}
	void SetCurve
		( const _T & p0, const _T & p1, const _T & p2,
			double v0 = 1, double v1 = 1, double v2 = 1 )
		{
			SetCurveUnsmoothSpeed( p0, p1, p2, v0, v1, v1, v2 ) ;
		}
	void AddCurveUnsmoothSpeed
		( const _T & px, double v0, double v1, int nDivision )
		{
			ESLAssert( nDivision >= 1 ) ;
			ESLAssert( (int) nDivision * 3 + 4 <= (int) m_count ) ;
			int	m = nDivision * 3 ;
			_T	d = m_cp[m] - m_cp[m - 1] ;
			_T	a = m_cp[m] + d * 1.5 ;
			m_cp[m + 1] = m_cp[m] + d * v0 ;
			m_cp[m + 2] = px + (a - px) * (v1 * (2.0 / 3.0)) ;
			m_cp[m + 3] = px ;
		}
	void AddCurve( const _T & px, double vx, int nDivision = 1 )
		{
			AddCurveUnsmoothSpeed( px, 1, vx, nDivision ) ;
		}
	// 制御点計算
	_T & pt( _T & p, double t, int n ) const
		{
			unsigned int	m = n * 3 ;
			if ( m_count >= m + 4 )
			{
				double	ct = 1.0 - t ;
				p = m_cp[m] * (ct * ct * ct) ;
				p += m_cp[m + 1] * (3.0 * t * ct * ct) ;
				p += m_cp[m + 2] * (3.0 * t * t * ct) ;
				p += m_cp[m + 3] * (t * t * t) ;
			}
			return	p ;
		}
	_T & pt( _T & p, double t ) const
		{
			int	m = GetDividedPosition( t ) / 3 ;
			return	pt( p, t, m ) ;
		}
	_T pt( double t, int n ) const
		{
			_T	p ;
			unsigned int	m = n * 3 ;
			if ( m_count >= m + 4 )
			{
				double	ct = 1.0 - t ;
				p = m_cp[m] * (ct * ct * ct) ;
				p += m_cp[m + 1] * (3.0 * t * ct * ct) ;
				p += m_cp[m + 2] * (3.0 * t * t * ct) ;
				p += m_cp[m + 3] * (t * t * t) ;
			}
			return	p ;
		}
	_T pt( double t ) const
		{
			_T	p ;
			if ( m_count >= 4 )
			{
				int		m = GetDividedPosition( t ) ;
				double	ct = 1.0 - t ;
				p = m_cp[m] * (ct * ct * ct) ;
				p += m_cp[m + 1] * (3.0 * t * ct * ct) ;
				p += m_cp[m + 2] * (3.0 * t * t * ct) ;
				p += m_cp[m + 3] * (t * t * t) ;
			}
			return	p ;
		}
	// ベジェ曲線分割
	void DivideBezier
		( double t, EBezierCurves<_T> & bzFirst, EBezierCurves<_T> & bzLast )
		{
			if ( m_count >= 4 )
			{
				unsigned int	m = GetDividedPosition( t ) ;
				//
				bzFirst.SetCount( m + 4 ) ;
				bzLast.SetCount( m_count - m ) ;
				//
				unsigned int	i ;
				for ( i = 0; i <= m; i ++ )
				{
					bzFirst.m_cp[i] = m_cp[i] ;
				}
				for ( i = m + 3; i < m_count; i ++ )
				{
					bzLast.m_cp[i - m] = m_cp[i] ;
				}
				double	u = 1.0 - t ;
				_T	P = m_cp[m] * u + m_cp[m + 1] * t ;
				_T	Q = m_cp[m + 1] * u + m_cp[m + 2] * t ;
				_T	R = m_cp[m + 2] * u + m_cp[m + 3] * t ;
				_T	S = P * u + Q * t ;
				_T	T = Q * u + R * t ;
				_T	U = S * u + T * t ;
				bzFirst.m_cp[m + 1] = P ;
				bzFirst.m_cp[m + 2] = S ;
				bzFirst.m_cp[m + 3] = U ;
				bzLast.m_cp[0] = U ;
				bzLast.m_cp[1] = T ;
				bzLast.m_cp[2] = R ;
			}
		}
	// 代入
	const EBezierCurves<_T> & operator = ( const EBezierCurves<_T> & bz )
		{
			int	i, nCount = bz.GetCount() ;
			SetCount( nCount ) ;
			for ( i = 0; i < nCount; i ++ )
			{
				m_cp[i] = bz.m_cp[i] ;
			}
			return	*this ;
		}

} ;


//////////////////////////////////////////////////////////////////////////////
// 画像バッファインターフェース
//////////////////////////////////////////////////////////////////////////////

class	EGLImageBufferInterface
			: public ESLObject, public EGL_IMAGE_BUFFER_INTERFACE
{
protected:
	PEGL_IMAGE_INFO	m_pAttached ;

public:
	// クラス情報
	DECLARE_CLASS_INFO( EGLImageBufferInterface, ESLObject )
	// 構築関数
	EGLImageBufferInterface( void ) ;
	// 消滅関数
	virtual ~EGLImageBufferInterface( void ) ;
	// バッファ更新通知
	virtual ESLError UpdateBuffer
		( PEGL_IMAGE_INFO pImageInf, const EGL_RECT * pRect ) = 0 ;
	// バッファ更新確定
	virtual ESLError CommitBuffer( PEGL_IMAGE_INFO pImageInf ) = 0 ;
	// バッファに反映
	virtual ESLError ReflectBuffer
		( PEGL_IMAGE_INFO pImageInf, const EGL_RECT * pRect ) = 0 ;

private:
	// コールバック関数
	static void ReleaseProc
		( PEGL_IMAGE_INFO pImageInf,
			EGL_IMAGE_BUFFER_INTERFACE * pInterface ) ;
	static ESLError UpdateImageBufferProc
		( PEGL_IMAGE_INFO pImageInf,
			EGL_IMAGE_BUFFER_INTERFACE * pInterface,
			const struct EGL_RECT * pRect ) ;
	static ESLError CommitBufferProc
		( PEGL_IMAGE_INFO pImageInf,
			EGL_IMAGE_BUFFER_INTERFACE * pInterface ) ;
	static ESLError ReflectBufferProc
		( PEGL_IMAGE_INFO pImageInf,
			EGL_IMAGE_BUFFER_INTERFACE * pInterface,
			const struct EGL_RECT * pRect ) ;
	// ポインタキャスト
	static EGLImageBufferInterface *
			FromPtr( EGL_IMAGE_BUFFER_INTERFACE * pInterface )
		{
			ESLAssert( pInterface != NULL ) ;
			return	(EGLImageBufferInterface*) pInterface ;
		}

public:
	// 画像バッファにインターフェース設定
	ESLError SetBufferInterface( PEGL_IMAGE_INFO pImageInf ) ;
	// 画像バッファのインターフェース解除
	void DetachBufferInterface( void ) ;
	// 画像バッファ取得
	PEGL_IMAGE_INFO GetAttachedImageBuffer( void ) const
		{
			return	m_pAttached ;
		}

} ;


//////////////////////////////////////////////////////////////////////////////
// 画像オブジェクト
//////////////////////////////////////////////////////////////////////////////

class	EGLImage	: public	ESLObject
{
protected:
	enum	ImageOwnerFlag
	{
		iofRefInfo		= 0,
		iofRefBuffer,
		iofOwnBuffer
	} ;
	PEGL_IMAGE_INFO	m_pImage ;
	ImageOwnerFlag	m_iofOwnerFlag ;

public:
	// 画像展開コールバック
	typedef ESLError
		(__stdcall *PFUNC_CALLBACK_DECODE)
			( EGLImage * ptrImage, void * ptrData,
				DWORD dwDecoded, DWORD dwTotal ) ;
public:
	// 画像展開オブジェクト
	class	EGLImageDecoder	: public	ERISADecoder
	{
	public:
		EGLImage *				m_ptrImage ;
		PFUNC_CALLBACK_DECODE	m_pfnCallback ;
		void *					m_ptrCallbackData ;
	public:
		EGLImageDecoder
			( EGLImage * ptrImage,
				PFUNC_CALLBACK_DECODE pfnCallback, void * ptrData )
			: m_ptrImage( ptrImage ),
				m_pfnCallback( pfnCallback ), m_ptrCallbackData( ptrData ) { }
		virtual ESLError OnDecodedBlock
			( LONG line, LONG column, const EGL_IMAGE_RECT & rect ) ;
	} ;

public:
	// 構築関数
	EGLImage( void ) : m_pImage(NULL), m_iofOwnerFlag(iofRefInfo) { }
	// 消滅関数
	virtual ~EGLImage( void )
		{
			if ( m_pImage && m_iofOwnerFlag )
				DeleteImage( ) ;
		}
	// クラス情報
	DECLARE_CLASS_INFO( EGLImage, ESLObject )

public:
	// 画像バッファ関連付け
	virtual void AttachImage( PEGL_IMAGE_INFO pImage ) ;
	// 画像バッファの参照を設定
	virtual void SetImageView
		( PEGL_IMAGE_INFO pImage, PCEGL_RECT pViewRect = NULL ) ;
	// 画像バッファ作成
	virtual PEGL_IMAGE_INFO CreateImage
		( DWORD fdwFormat, DWORD dwWidth, DWORD dwHeight,
				DWORD dwBitsPerPixel, DWORD dwFlags = 0 ) ;
	// 画像バッファ複製
	virtual PEGL_IMAGE_INFO DuplicateImage
		( PCEGL_IMAGE_INFO pImage, DWORD dwFlags = 0 ) ;

	// 画像ファイルを読み込む
	virtual ESLError ReadImageFile
		( ESLFileObject & file,
			PFUNC_CALLBACK_DECODE pfnCallback = NULL,
			void * ptrCallbackData = NULL ) ;
	// 画像バッファ消去
	virtual void DeleteImage( void ) ;

	// プレビュー画像展開フラグ
	enum	PreviewFlags
	{
		pfForcePreview		= 0x0001,
		pfResizePreview		= 0x0002,
		pfNoResizeDecoding	= 0x0004
	} ;
	// プレビュー画像を読み込む
	ESLError ReadPreviewImage
		( ESLFileObject & file,
			DWORD dwMaxWidth, DWORD dwMaxHeight,
			DWORD dwPreviewFlags = pfForcePreview,
			EGL_SIZE * pOriginalSize = NULL,
			PFUNC_CALLBACK_DECODE pfnCallback = NULL,
			void * ptrCallbackData = NULL ) ;

	// 圧縮方式フラグ
	enum	CompressTypeFlag
	{
		ctfCompatibleFormat,
		ctfExtendedFormat,
		ctfSuperiorArchitecure
	} ;
	// 画像ファイルへ書き出す
	ESLError WriteImageFile
		( ESLFileObject & file,
			CompressTypeFlag ctfType = ctfCompatibleFormat,
			DWORD dwFlags = ERISAEncoder::efNormalCmpr,
			const ERISAEncoder::PARAMETER * periep = NULL ) ;

public:
	// 画像情報取得
	PEGL_IMAGE_INFO GetInfo( void ) const
		{
			return	m_pImage ;
		}
	operator PCEGL_IMAGE_INFO ( void ) const
		{
			return	m_pImage ;
		}
	operator PEGL_IMAGE_INFO ( void )
		{
			return	m_pImage ;
		}
	operator const EGL_IMAGE_INFO & ( void ) const
		{
			ESLAssert( m_pImage != NULL ) ;
			return	*m_pImage ;
		}
	// 画像フォーマット取得
	DWORD GetFormatType( void ) const
		{
			return	m_pImage ? m_pImage->fdwFormatType : 0 ;
		}
	// 画像サイズ取得
	EGLSize GetSize( void ) const
		{
			return	EGLSize( GetWidth(), GetHeight() ) ;
		}
	DWORD GetWidth( void ) const
		{
			return	m_pImage ? m_pImage->dwImageWidth : 0 ;
		}
	DWORD GetHeight( void ) const
		{
			return	m_pImage ? m_pImage->dwImageHeight : 0 ;
		}
	// ビット深度取得
	DWORD GetBitsPerPixel( void ) const
		{
			return	m_pImage ? m_pImage->dwBitsPerPixel : 0 ;
		}

public:
	// 画像の上下反転
	ESLError ReverseVertically( void )
		{
			ESLAssert( m_pImage != NULL ) ;
			if ( m_pImage == NULL )
				return	eslErrGeneral ;
			return	::eglReverseVertically( m_pImage ) ;
		}
	// 画像をデバイスコンテキストに描画
	ESLError DrawToDC( HDC hDstDC, int nPosX, int nPosY,
					PCEGL_SIZE pSizeToDraw, PCEGL_RECT pViewRect ) const
		{
			ESLAssert( m_pImage != NULL ) ;
			if ( m_pImage == NULL )
				return	eslErrGeneral ;
			return	::eglDrawToDC
				( hDstDC, m_pImage,
					nPosX, nPosY, pSizeToDraw, pViewRect ) ;
		}
	// 画像を指定の値で塗りつぶす
	ESLError FillImage( EGL_PALETTE colorFill )
		{
			ESLAssert( m_pImage != NULL ) ;
			if ( m_pImage == NULL )
				return	eslErrGeneral ;
			return	::eglFillImage( m_pImage, colorFill ) ;
		}
	// 指定ピクセルを取得
	EGL_PALETTE GetPixel( int nPosX, int nPosY ) const
		{
			ESLAssert( m_pImage != NULL ) ;
			if ( m_pImage == NULL )
				return	(EGL_PALETTE) EGLPalette( 0UL ) ;
			return	::eglGetPixel( m_pImage, nPosX, nPosY ) ;
		}
	// 指定ピクセルに設定
	ESLError SetPixel( int nPosX, int nPosY, EGL_PALETTE colorPixel )
		{
			ESLAssert( m_pImage != NULL ) ;
			if ( m_pImage == NULL )
				return	eslErrGeneral ;
			return	::eglSetPixel( m_pImage, nPosX, nPosY, colorPixel ) ;
		}

public:
	// フォーマット変換
	ESLError ConvertFrom( PCEGL_IMAGE_INFO pSrcImage )
		{
			ESLAssert( m_pImage != NULL ) ;
			if ( m_pImage == NULL )
				return	eslErrGeneral ;
			return	::eglConvertFormat( m_pImage, pSrcImage ) ;
		}
	// トーンフィルタ適用
	ESLError ApplyToneTable
		( PCEGL_IMAGE_INFO pSrcImage,
			const void * pBlueTone, const void * pGreenTone,
			const void * pRedTone, const void * pAlphaTone )
		{
			ESLAssert( m_pImage != NULL ) ;
			if ( m_pImage == NULL )
				return	eslErrGeneral ;
			return	::eglApplyToneTable
				( m_pImage, pSrcImage,
					pBlueTone, pGreenTone, pRedTone, pAlphaTone ) ;
		}
	// 輝度フィルタ適用
	ESLError SetColorTone
		( PCEGL_IMAGE_INFO pSrcImage,
			int nBlueTone, int nGreenTone, int nRedTone, int nAlphaTone )
		{
			ESLAssert( m_pImage != NULL ) ;
			if ( m_pImage == NULL )
				return	eslErrGeneral ;
			return	::eglSetColorTone
				( m_pImage, pSrcImage,
					nBlueTone, nGreenTone, nRedTone, nAlphaTone ) ;
		}
	// 2倍に拡大処理
	ESLError EnlargeDouble( PCEGL_IMAGE_INFO pSrcImage, DWORD dwFlags = 0 )
		{
			ESLAssert( m_pImage != NULL ) ;
			if ( m_pImage == NULL )
				return	eslErrGeneral ;
			return	::eglEnlargeDouble( m_pImage, pSrcImage, dwFlags ) ;
		}
	// αチャネル合成
	ESLError BlendAlphaChannel
		( PCEGL_IMAGE_INFO pSrcRGB, PCEGL_IMAGE_INFO pSrcAlpha,
			DWORD dwFlags = 0, SDWORD nAlphaBase = 0, DWORD nCoefficient = 0x1000 )
		{
			ESLAssert( m_pImage != NULL ) ;
			if ( m_pImage == NULL )
				return	eslErrGeneral ;
			return	::eglBlendAlphaChannel
				( m_pImage, pSrcRGB, pSrcAlpha,
					dwFlags, nAlphaBase, nCoefficient ) ;
		}
	// αチャネル分離
	ESLError UnpackAlphaChannel
			( PEGL_IMAGE_INFO pDstRGB,
				PEGL_IMAGE_INFO pDstAlpha, DWORD dwFlags = 0 ) const
		{
			ESLAssert( m_pImage != NULL ) ;
			if ( m_pImage == NULL )
				return	eslErrGeneral ;
			return	::eglUnpackAlphaChannel
						( pDstRGB, pDstAlpha, m_pImage, dwFlags ) ;
		}

} ;


//////////////////////////////////////////////////////////////////////////////
// アニメーション画像オブジェクト
//////////////////////////////////////////////////////////////////////////////

class	EGLAnimation	: public	EGLImage
{
protected:
	DWORD				m_dwTotalTime ;
	DWORD				m_dwCurrent ;
	ENumArray<UINT>		m_lstSequence ;
	EPtrObjArray<EGL_IMAGE_INFO>
						m_lstFrames ;
	EGL_POINT			m_ptHotSpot ;
	long int			m_nResolution ;		// x100 [pixel/inch]
	EWideString			m_wstrReferenceFile ;

public:
	// 構築関数
	EGLAnimation( void ) ;
	// 消滅関数
	virtual ~EGLAnimation( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( EGLAnimation, EGLImage )

public:
	// 画像バッファの参照を設定
	virtual void SetImageView
		( PEGL_IMAGE_INFO pImage, PCEGL_RECT pViewRect = NULL ) ;
	// 画像バッファ作成
	virtual PEGL_IMAGE_INFO CreateImage
		( DWORD fdwFormat, DWORD dwWidth, DWORD dwHeight,
				DWORD dwBitsPerPixel, DWORD dwFlags = 0 ) ;
	// 画像バッファ複製
	virtual PEGL_IMAGE_INFO DuplicateImage
		( PCEGL_IMAGE_INFO pImage, DWORD dwFlags = 0 ) ;
	// 画像ファイルを読み込む
	virtual ESLError ReadImageFile
		( ESLFileObject & file,
			PFUNC_CALLBACK_DECODE pfnCallback = NULL,
			void * ptrCallbackData = NULL ) ;
	// 画像バッファ消去
	virtual void DeleteImage( void ) ;
	// 画像ファイルへ書き出す
	ESLError WriteImageFile
		( ESLFileObject & file,
			CompressTypeFlag ctfType = ctfCompatibleFormat,
			DWORD dwFlags = ERISAEncoder::efNormalCmpr,
			const ERISAEncoder::PARAMETER * periep = NULL ) ;

public:
	// シーケンステーブルを取得
	ENumArray<UINT> & GetSequenceTable( void )
		{
			return	m_lstSequence ;
		}
	// フレームを削除
	void RemoveFrameAt( int iFrame ) ;
	// フレームを追加
	void AddFrame( PEGL_IMAGE_INFO pImage ) ;
	// フレームを挿入
	void InsertFrame( int iFrame, PEGL_IMAGE_INFO pImage ) ;

public:
	// 全時間を取得
	DWORD GetTotalTime( void ) const
		{
			return	m_dwTotalTime ;
		}
	// 全フレーム数を設定
	void SetTotalTime( DWORD dwTotalTime )
		{
			m_dwTotalTime = dwTotalTime ;
		}
	// 全フレーム数を取得
	DWORD GetTotalFrameCount( void ) const
		{
			return	m_lstFrames.GetSize( ) ;
		}
	// 全シーケンス長を取得
	DWORD GetSequenceLength( void ) const
		{
			return	m_lstSequence.GetSize( ) ;
		}
	// 現在のシーケンス番号を取得
	DWORD GetCurrentSequence( void ) const
		{
			return	m_dwCurrent ;
		}
	// ホットスポットを取得
	EGL_POINT GetHotSpot( void ) const
		{
			return	m_ptHotSpot ;
		}
	// ホットスポットを設定
	void SetHotSpot( EGL_POINT ptHotSpot )
		{
			m_ptHotSpot = ptHotSpot ;
		}
	// 解像度取得
	long int GetResolution( void ) const
		{
			return	m_nResolution ;
		}
	// 解像度設定
	void SetResolution( long int nResolution )
		{
			m_nResolution = nResolution ;
		}
	// リファレンスファイル名取得
	const EWideString & GetReferenceFile( void ) const
		{
			return	m_wstrReferenceFile ;
		}
	// リファレンスファイル名設定
	void SetReferenceFile( const wchar_t * pwszFileName )
		{
			m_wstrReferenceFile = pwszFileName ;
		}
	// 時間からシーケンス番号に変換
	DWORD TimeToSequence( DWORD dwMilliSec ) const ;
	// シーケンス番号から時間に変換
	DWORD SequenceToTime( DWORD dwSequence ) const ;
	// シーケンス番号からフレーム番号に変換
	DWORD SequenceToFrame( DWORD dwSequence ) const ;
	// 指定フレームの画像を取得
	PEGL_IMAGE_INFO GetFrameAt( DWORD dwFrame ) ;
	// 現在のシーケンス画像を設定する
	ESLError SetCurrentSequence( DWORD dwSequence ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
// 減色処理クラス
//////////////////////////////////////////////////////////////////////////////

class	EGLColorDecreaser	: public	ESLObject
{
public:
	// 構築関数
	EGLColorDecreaser( void ) ;
	// 消滅関数
	virtual ~EGLColorDecreaser( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( EGLColorDecreaser, ESLObject )

protected:
	PEGL_IMAGE_INFO	m_pDstImage ;
	PEGL_IMAGE_INFO	m_pSrcImage ;

	struct	COLOR_INT_RGBA
	{
		int	b, g, r, a ;

		void operator *= ( int nMul )
			{
				b *= nMul ;
				g *= nMul ;
				r *= nMul ;
				a *= nMul ;
			}
		void operator /= ( int nDiv )
			{
				b = (b - (b >> 31)) / nDiv ;
				g = (g - (g >> 31)) / nDiv ;
				r = (r - (r >> 31)) / nDiv ;
				a = (a - (a >> 31)) / nDiv ;
			}
		void operator += ( const COLOR_INT_RGBA & rgba )
			{
				b += rgba.b ;
				g += rgba.g ;
				r += rgba.r ;
				a += rgba.a ;
			}
	} ;
	struct	COLOR_STATISTICS
	{
		INT64	rgba[4] ;
		int		count ;
		union
		{
			float		rate ;
			EGL_PALETTE	color ;
		} ;
		EGL_PALETTE GetAverageColor( void ) const
			{
				int	c = count ;
				ESLAssert( c > 0 ) ;
				int	b = (int) ((rgba[0] + (c / 2)) / c) ;
				int	g = (int) ((rgba[1] + (c / 2)) / c) ;
				int	r = (int) ((rgba[2] + (c / 2)) / c) ;
				int	a = (int) ((rgba[3] + (c / 2)) / c) ;
				ESLAssert( b >= 0 ) ;
				ESLAssert( g >= 0 ) ;
				ESLAssert( r >= 0 ) ;
				if ( b >= 0x100 )
				{
					b = 0xFF ;
				}
				if ( g >= 0x100 )
				{
					g = 0xFF ;
				}
				if ( r >= 0x100 )
				{
					r = 0xFF ;
				}
				if ( a >= 0x100 )
				{
					a = 0xFF ;
				}
				EGL_PALETTE	avgc ;
				avgc.rgba.Blue = (BYTE) b ;
				avgc.rgba.Green = (BYTE) g ;
				avgc.rgba.Red = (BYTE) r ;
				avgc.rgba.Alpha = (BYTE) a ;
				return	avgc ;
			}
	} ;
	enum
	{
		ALPHA_BIAS	= 0x10
	} ;
	COLOR_STATISTICS *	m_pcsTable ;
	int					m_nTableSize ;

public:
	enum	ConvertFlag
	{
		cvfDithering				= 0x01,
		cvfTransparencyPalette		= 0x02,
		cvfNoDitherTransparency		= 0x04,
		cvfPaletteHasAlpha			= 0x08,
		cvfPaletteComposition		= 0x00000,
		cvfPaletteSelection			= 0x10000,
		cvtPaletteAlgorithmMask		= 0xF0000,
		cvtPaletteAlgorithmShifter	= 16,
	} ;
	// 画像を変換する
	ESLError ConvertImage
		( PEGL_IMAGE_INFO pDstImage,
			PEGL_IMAGE_INFO pSrcImage, DWORD fdwFlags,
			unsigned int nColors, double rColorWeight = 1.0,
			unsigned int nAlphaThreshold = 0x7F ) ;
	// 処理画像バッファを関連付ける
	void AttachImageBuffer
		( PEGL_IMAGE_INFO pDstImage, PEGL_IMAGE_INFO pSrcImage ) ;
	// 減色パレットを決定
	ESLError MakePalette
		( EGL_PALETTE rgbPalette[], int & nPaletteCount,
					DWORD fdwFlags, double rColorWeight = 1.0 ) ;
	// 減色処理
	ESLError DecreaseColorImage
		( DWORD fdwFlags, unsigned int nAlphaThreshold = 0x7F ) ;

public:
	// パレット生成準備
	void PrepareMakePalette( void ) ;
	// 画像情報統計処理
	ESLError CompileColorStatistics
			( PEGL_IMAGE_INFO pImageInf, DWORD fdwFlags ) ;
	// パレット生成
	ESLError BuildPalette
		( EGL_PALETTE rgbPalette[], int & nPaletteCount,
					DWORD fdwFlags, double rColorWeight = 1.0 ) ;

protected:
	// パレットテーブルをソートする
	void SortColorStatisticsTable( int nFirst, int nEnd ) ;
	// 最も出現数の大きいテーブル要素を検索する
	int FindMaxOccuredColor( void ) const ;
	// 最も重みのあるテーブル要素を検索する
	int GetMaxWeightColorStatistics
		( const EGL_PALETTE rgbPalette[], int nPaletteCount ) const ;
	// 色の統計情報の重みを計算する
	static double CalcColorStatisticsWeight
		( const COLOR_STATISTICS & csColor,
			const EGL_PALETTE rgbPalette[], int nPaletteCount ) ;

public:
	enum	ProcessType
	{
		ptCompileColorStatistics,		// 色情報の統計処理
		ptBuildPalette,					// パレット生成
		ptConvertImage,					// 画像減色処理
	} ;
	// 進行状況
	virtual ESLError OnProgress
		( int nProcess, int nCurrent, int nTotal ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
// 画像メディア読み込みクラス
//////////////////////////////////////////////////////////////////////////////

namespace	Gdiplus
{
	class	GpBitmap ;
} ;
class	EGLMediaLoader	: public	EGLAnimation
{
public:
	// 構築関数
	EGLMediaLoader( void ) ;
	// 消滅関数
	virtual ~EGLMediaLoader( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( EGLMediaLoader, EGLAnimation )

public:
	// 画像ファイルを読み込む
	ESLError LoadMediaFile
		( const char * pszFileName,
			DWORD fdwFormat = 0, DWORD dwBitsPerPixel = 0,
			DWORD dwLimitFrames = 0, DWORD dwLimitSize = 0 ) ;
	ESLError ReadMediaFile
		( ESLFileObject & file,
			DWORD fdwFormat = 0, DWORD dwBitsPerPixel = 0 ) ;
	// プレビュー画像を読み込む
	ESLError LoadPreviewImage
		( const char * pszFileName,
			DWORD dwWidth, DWORD dwHeight,
			DWORD dwPreviewFlags = pfForcePreview,
			EGL_SIZE * pOriginalSize = NULL,
			PFUNC_CALLBACK_DECODE pfnCallback = NULL,
			void * ptrCallbackData = NULL ) ;

public:
	// Window Bitmap ファイルを読み込む
	ESLError ReadBitmapFile
		( ESLFileObject & file,
			DWORD fdwFormat = 0, DWORD dwBitsPerPixel = 0 ) ;
	// Photoshop PSD ファイルを読み込む
	ESLError ReadPhotoshopPSDFile
		( ESLFileObject & file,
			DWORD fdwFormat = 0, DWORD dwBitsPerPixel = 0,
			DWORD dwLimitFrames = 0, DWORD dwLimitSize = 0 ) ;
	// AVI ファイルを読み込む
	ESLError LoadAviFile
		( const char * pszFileName,
			DWORD fdwFormat = 0, DWORD dwBitsPerPixel = 0,
			DWORD dwLimitFrames = 0, DWORD dwLimitSize = 0 ) ;
	// GDI+ を利用して読み込む
	ESLError LoadWithGDIplus
		( const char * pszFileName,
			DWORD fdwFormat = 0, DWORD dwBitsPerPixel = 0 ) ;
	ESLError ReadWithGDIplus
		( ESLFileObject & file,
			DWORD fdwFormat = 0, DWORD dwBitsPerPixel = 0 ) ;
	// 画像フォーマットを変換する
	ESLError ConvertFormatTo
		( DWORD fdwFormat, DWORD dwBitsPerPixel ) ;

public:
	// ビットマップデータから画像オブジェクトを作成する
	ESLError CreateFromBitmap
		( const BITMAPINFO * pbmi,
			void * ptrBitmap = NULL, DWORD dwFlags = 0 ) ;
	// 画像オブジェクトからパックドDIBを作成する
	BITMAPINFO * CreatePackedDIB( void ** ppBitmap = NULL ) const ;
	// GDI+ オブジェクトから画像オブジェクトへ変換する
	ESLError ConvertFromGDIplus
		( Gdiplus::GpBitmap * pbitmap,
			DWORD fdwFormat = 0, DWORD dwBitsPerPixel = 0 ) ;
	// 画像オブジェクトから GDI+ オブジェクトへ変換する
	ESLError CreateGDIplusBitmap( Gdiplus::GpBitmap *& pbitmap ) ;

public:
	// Windows Bitmap ファイルを書き出す
	ESLError WriteBitmapFile( ESLFileObject & file ) ;
	// Photoshop PSD ファイルを書き出す
	ESLError WritePhotoshopPSDFile( ESLFileObject & file ) ;
	// AVI ファイルを書き出す
	ESLError SaveAviFile( const char * pszFileName ) ;
	// GDI+ を利用して書き出す
	ESLError SaveWithGDIplus
		( const char * pszFileName,
			const wchar_t * pwszMimeType, int nQuality = -1 ) ;
	ESLError WriteWithGDIplus
		( ESLFileObject & file,
			const wchar_t * pwszMimeType, int nQuality = -1 ) ;

protected:
	// MIME タイプから CLSID を取得
	static ESLError GetEncoderClsid
		( const wchar_t * pwszMimeType, CLSID & clsidEncoder ) ;

public:
	struct	GDIP_CODEC
	{
		EWideString	wstrCodecName ;
		EWideString	wstrDllName ;
		EWideString	wstrDescription ;
		EWideString	wstrFileExtension ;
		EWideString	wstrMimeType ;
	} ;
	// GDI+ で利用可能なデコーダーを列挙する
	static ESLError EnumGDIplusDecoderList
		( EObjArray<GDIP_CODEC> & lstCodec ) ;
	// GDI+ で利用可能なエンコーダーを列挙する
	static ESLError EnumGDIplusEncoderList
		( EObjArray<GDIP_CODEC> & lstCodec ) ;

public:
	// 初期化
	static void Initialize( bool fComMTA = false ) ;
	// 終了
	static void Close( void ) ;
	// GDI+ は有効か？
	static bool IsInstalledGDIplus( void ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
// モデルオブジェクト
//////////////////////////////////////////////////////////////////////////////

#include <egl3d_polygon_model.h>



//////////////////////////////////////////////////////////////////////////////
// モデルジョイント
//////////////////////////////////////////////////////////////////////////////

#include <egl3d_model_joint.h>



//////////////////////////////////////////////////////////////////////////////
// 骨組み込みモデルオブジェクト
//////////////////////////////////////////////////////////////////////////////

class	E3DBonePolygonModel	: public	E3DPolygonModel
{
public:
	// 構築関数
	E3DBonePolygonModel( void ) ;
	// 消滅関数
	virtual ~E3DBonePolygonModel( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( E3DBonePolygonModel, E3DPolygonModel )

public:
	// ボーンジョイント
	class	E3DBoneJoint	: public	E3DModelJoint
	{
	public:
		// 構築関数
		E3DBoneJoint( void ) ;
		// 消滅関数
		virtual ~E3DBoneJoint( void ) ;
		// クラス情報
		DECLARE_CLASS_INFO( E3DBoneJoint, E3DModelJoint )
	public:
		EWideString		m_wstrName ;		// ボーンの名前
		E3DVector		m_vCenter ;			// ボーンの基準点
		bool			m_fVertexApplyAll ;
		bool			m_fNormalApplyAll ;
		unsigned int	m_iRefVertex ;		// 頂点参照
		unsigned int	m_nRefVertexCount ;
		REAL32 *		m_pVertexApply ;
		PE3D_VECTOR4	m_pVertexBuf ;
		unsigned int	m_iRefNormal ;		// 法線参照
		unsigned int	m_nRefNormalCount ;
		REAL32 *		m_pNormalApply ;
		PE3D_VECTOR4	m_pNormalBuf ;
	public:
		// ジョイント生成
		virtual E3DModelJoint * CreateJoint( void ) const ;
		// パラメータ反映
		virtual void RefreshJoint( void ) ;
		// ジョイント回転処理
		virtual void TransformJoint( const E3DModelJoint & mjParent ) ;
		// モデル回転処理
		virtual void TransformModel( E3DPolygonModel & model ) const ;
	protected:
		void TransformVertex
			( E3DPolygonModel & model,
				unsigned int iFirst, unsigned int nCount ) const ;
		void TransformNormal
			( E3DPolygonModel & model,
				unsigned int iFirst, unsigned int nCount ) const ;
	public:
		// 全てのジョイントを削除
		void DeleteContents( void ) ;
		// ボーンを読み込む
		ESLError ReadBoneData( ESLFileObject & file ) ;
		// ボーンを書き出す
		ESLError WriteBoneData( ESLFileObject & file ) ;
		// 頂点用バッファ確保
		void AllocateVertexBuffer
			( unsigned int iFirst, unsigned int nCount ) ;
		// 法線用バッファ確保
		void AllocateNormalBuffer
			( unsigned int iFirst, unsigned int nCount ) ;
		// ボーンを名前で検索
		E3DBoneJoint * FindBoneAs( const wchar_t * pwszName ) ;
	} ;
	// クロスモーフィングメッシュエントリ
	struct	CLOTH_MESH_ENTRY
	{
		DWORD					dwType ;		// ClothMeshType
		DWORD					dwMeshIndex ;
		PE3D_PRIMITIVE_POLYGON	pMesh ;
		HINDER_MODEL_INFO		hmiHinderInfo ;
		EGL_CLOTH_ATTRIBUTE		caClothAttr ;
	} ;
	struct	CLOTH_MESH_ENTRY_DATA
	{
		DWORD					dwType ;		// ClothMeshType
		DWORD					dwMeshIndex ;
		DWORD					dwReserved[2] ;
		HINDER_MODEL_INFO		hmiHinderInfo ;
		EGL_CLOTH_ATTRIBUTE		caClothAttr ;
	} ;
	enum	ClothMeshType
	{
		cmtHinder		= 0,
		cmtWeaveCloth	= 1,
		cmtPatchCloth	= 2,
	} ;
	// クロスモーフィング
	class	E3DClothMorph	: public ESLObject
	{
	public:
		E3DBoneJoint *				m_pJoint ;
		HEGL_CLOTH_MODEL_MORPH		m_hClothModel ;
		EWideString					m_wstrName ;
		EObjArray<CLOTH_MESH_ENTRY>	m_lstMeshEntry ;
		EStreamBuffer				m_bufHinderList ;
		EPtrBuffer					m_ptrHinderList ;
	public:
		// 構築関数
		E3DClothMorph( void ) ;
		// 消滅関数
		virtual ~E3DClothMorph( void ) ;
		// クラス情報
		DECLARE_CLASS_INFO( E3DClothMorph, ESLObject )
	public:
		// 所有データを開放する
		ESLError ReleaseData( void ) ;
		// モーフィングオブジェクトを生成する
		ESLError CreateClothMorph( E3DBonePolygonModel & model ) ;
		// 当たり判定オブジェクトを設定する
		ESLError SetHinderModel
			( const HINDER_MODEL_INFO * phmiHinder, unsigned int nCount ) ;
		// 当たり判定座標を更新する
		ESLError UpdateHinderModel( void ) ;
		// モーフィングオブジェクトを削除する
		ESLError DeleteClothMorph( void ) ;
		// データを読み込む
		ESLError ReadClothData( ESLFileObject & file ) ;
		// データを書き出す
		ESLError WriteClothData( ESLFileObject & file ) ;
		// モーフィング実行
		ESLError MorphCloth( const EGL_CLOTH_MORPH_PARAMETER & cmp ) ;
		ESLError MorphMesh( const EGL_CLOTH_MORPH_PARAMETER & cmp ) ;
	} ;
	class	E3DClothMorphModelSet	: public	EObjArray<E3DClothMorph>
	{
	public:
		// 構築関数
		E3DClothMorphModelSet( void ) {}
		E3DClothMorphModelSet( const E3DClothMorphModelSet & cmms ) ;
		// 消滅関数
		virtual ~E3DClothMorphModelSet( void ) ;
		// クラス情報
		DECLARE_CLASS_INFO( E3DClothMorphModelSet, EPtrArray )
	public:
		// 代入（データ複製）
		const E3DClothMorphModelSet & operator =
					( const E3DClothMorphModelSet & cmms ) ;
		// データを読み込む
		ESLError ReadClothData( ESLFileObject & file ) ;
		// データを書き出す
		ESLError WriteClothData( ESLFileObject & file ) ;
		// 所有データを開放する
		ESLError ReleaseData( void ) ;
		// モーフィングオブジェクトを削除する
		ESLError DeleteClothMorph( void ) ;
	public:
		// 布シミュレータオブジェクトを生成する
		ESLError CreateClothMorph( E3DBonePolygonModel & model ) ;
		// 外部当たり判定オブジェクトを設定する
		ESLError SetHinderModel
			( const HINDER_MODEL_INFO * phmiHinder, unsigned int nCount ) ;
		// 当たり判定座標を更新する
		ESLError UpdateHinderModel( void ) ;
		// 布メッシュ処理
		ESLError MorphCloth( const EGL_CLOTH_MORPH_PARAMETER & cmp ) ;
		ESLError MorphMesh( const EGL_CLOTH_MORPH_PARAMETER & cmp ) ;
		// 布メッシュを検索する
		E3DClothMorph * FindClothAs
			( const wchar_t * pwszName, unsigned int * pIndex = NULL ) ;
	} ;
	// ポーズデータ
	struct	POSE_ENTRY
	{
		DWORD			dwFlags ;			// フラグ PoseEntryFlags
		DWORD			dwReserved[3] ;
		E3D_QUATERNION	qtRev ;				// 回転行列（クォータニオン）
		E3D_VECTOR4		vOffset ;			// 移動座標
		E3D_VECTOR4		vRevParam1 ;		// 回転オイラー角表現
		E3D_VECTOR4		vRevParam2 ;
	} ;
	enum	PoseEntryFlags
	{
		psfNotUsedEntry	= 0x01,
	} ;
	class	E3DPose	: public EWStrTagArray<POSE_ENTRY>
	{
	public:
		// 構築関数
		E3DPose( void ) {}
		E3DPose( const E3DPose & pose ) ;
		// データ複製
		const E3DPose & operator = ( const E3DPose & pose ) ;
		// データを読み込む
		ESLError ReadPose( ESLFileObject & file ) ;
		// データを書き出す
		ESLError WritePose( ESLFileObject & file ) ;
	} ;
	struct	POSE_ANIMATION_ENTRY
	{
		double			rVelocity ;
		unsigned int	nDuration ;
		unsigned int	nReserved ;
	} ;
	class	E3DPoseAnimationEntry
	{
	public:
		EWideString		m_wstrPose ;
		double			m_rVelocity ;
		unsigned int	m_nDuration ;
	public:
		// 構築関数
		E3DPoseAnimationEntry( void )
			: m_rVelocity( 0 ), m_nDuration( 0 ) { }
		E3DPoseAnimationEntry( const E3DPoseAnimationEntry & pose )
			{
				m_wstrPose = pose.m_wstrPose ;
				m_rVelocity = pose.m_rVelocity ;
				m_nDuration = pose.m_nDuration ;
			}
		// データ複製
		const E3DPoseAnimationEntry & operator =
				( const E3DPoseAnimationEntry & pose )
			{
				m_wstrPose = pose.m_wstrPose ;
				m_rVelocity = pose.m_rVelocity ;
				m_nDuration = pose.m_nDuration ;
				return	*this ;
			}
	} ;
	class	E3DBoneAnimation
	{
	public:
		E3DBoneJoint *					m_pJoint ;
		ENumArray<unsigned int>			m_nDurations ;
		EBezierCurves<E3D_QUATERNION>	m_bzRev ;
		EBezierCurves<E3D_VECTOR>		m_bzMove ;
		double							m_rLastVelocity ;
	public:
		// 構築関数
		E3DBoneAnimation( void ) :
		  m_pJoint( NULL ), m_rLastVelocity( 0 ) { }
		E3DBoneAnimation( const E3DBoneAnimation & anime )
			{
				m_pJoint = anime.m_pJoint ;
				m_nDurations = anime.m_nDurations ;
				m_bzRev = anime.m_bzRev ;
				m_bzMove = anime.m_bzMove ;
				m_rLastVelocity = anime.m_rLastVelocity ;
			}
		// データ複製
		const E3DBoneAnimation & operator = ( const E3DBoneAnimation & anime )
			{
				m_pJoint = anime.m_pJoint ;
				m_nDurations = anime.m_nDurations ;
				m_bzRev = anime.m_bzRev ;
				m_bzMove = anime.m_bzMove ;
				m_rLastVelocity = anime.m_rLastVelocity ;
				return	*this ;
			}
		// 長さ取得
		unsigned int GetDuration( void ) const
			{
				unsigned int	i, n, m = 0 ;
				n = m_nDurations.GetSize( ) ;
				for ( i = 0 ; i < n; i ++ )
				{
					m += m_nDurations[i] ;
				}
				return	m ;
			}
	} ;
	class	E3DPoseAnimation	: public ESLObject
	{
	public:
		EWideString							m_wstrName ;
		EObjArray<E3DPoseAnimationEntry>	m_lstPose ;
		EWStrTagArray<E3DBoneAnimation>		m_lstBone ;
	private:
		unsigned int						m_nDuration ;
	public:
		// 構築関数
		E3DPoseAnimation( void ) ;
		// 消滅関数
		virtual ~E3DPoseAnimation( void ) ;
		// クラス情報
		DECLARE_CLASS_INFO( E3DPoseAnimation, ESLObject )
	public:
		// データを読み込む
		ESLError ReadPoseAnimation( ESLFileObject & file ) ;
		// データを書き出す
		ESLError WritePoseAnimation( ESLFileObject & file ) ;
		// データを解放する
		void ReleasePoseAnimation( void ) ;
		// データ複製
		const E3DPoseAnimation & operator =
			( const E3DPoseAnimation & poseanime ) ;
	public:
		// モーフィングオブジェクトを生成する
		ESLError InitializePoseAnimation( E3DBonePolygonModel & model ) ;
		// アニメーションデータを開放する
		ESLError ClosePoseAnimation( void ) ;
		// トータル時間取得
		unsigned int GetTotalDuration( void ) const
			{
				return	m_nDuration ;
			}
		// アニメーション設定
		ESLError UpdateAnimation
			( E3DBonePolygonModel & model, unsigned int nTime ) ;
	} ;

public:
	EObjArray<E3DBoneJoint>
					m_lstBones ;		// ボーン配列（ルートに直接属するもの）
	E3DClothMorphModelSet
					m_cmmsClothes ;		// クロスシミュレータ配列
	EWStrTagArray<E3DPose>
					m_wstaPoses ;		// ポーズ
	EWStrTagArray<E3DPoseAnimation>
					m_wstaPoseAnimations ;

	PE3D_VECTOR4	m_pOrgVertexes ;	// 元頂点
	PE3D_VECTOR4	m_pOrgNormals ;

public:
	// モデルデータを削除する
	virtual void DeleteContents( void ) ;
	// 頂点バッファを確保する
	virtual void AllocateVertexBuffer( unsigned int nCount ) ;
	// 法線バッファを確保する
	virtual void AllocateNormalBuffer( unsigned int nCount ) ;
	// 頂点リストの座標をコミットする
	virtual void CommitVertexBuffer( int iVertex, int nCount ) ;
	// 法線リストの座標をコミットする
	virtual void CommitNormalBuffer( int iNormal, int nCount ) ;

public:
	// モデルデータを読み込む
	virtual ESLError ReadModel( ESLFileObject & file ) ;
protected:
	// ユーザー定義のレコードを読み込む
	virtual ESLError ReadUserRecord( EMCFile & file, UINT64 idRec ) ;
public:
	// モデルデータを書き出す
	ESLError WriteModel( ESLFileObject & file ) ;
protected:
	// ユーザー定義のレコードを書き出す
	virtual ESLError WriteUserRecord( EMCFile & file ) ;

public:
	// ボーンの現在のパラメータをモデルに反映する
	virtual void TransformAccordingAsBone( void ) ;
	// ボーンを検索する
	E3DBoneJoint * FindBoneAs( const wchar_t * pwszName ) ;

public:
	// クロスシミュレータオブジェクト
	E3DClothMorphModelSet & ClothMorph( void )
		{
			return	m_cmmsClothes ;
		}
	// 布シミュレータオブジェクトを生成する
	ESLError CreateClothMorph( void ) ;
	// 外部当たり判定オブジェクトを設定する
	ESLError SetHinderModel
		( const HINDER_MODEL_INFO * phmiHinder, unsigned int nCount ) ;
	// 布メッシュ処理
	ESLError MorphCloth( const EGL_CLOTH_MORPH_PARAMETER & cmp ) ;
	// 布メッシュを検索する
	E3DClothMorph * FindClothAs
		( const wchar_t * pwszName, unsigned int * pIndex = NULL ) ;

public:
	// ポーズを取得する
	E3DPose * FindPoseAs
		( const wchar_t * pwszName, unsigned int * pIndex = NULL ) ;
	// ポーズを取得する
	E3DPoseAnimation * FindPoseAnimationAs
		( const wchar_t * pwszName, unsigned int * pIndex = NULL ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
// レンダリングオブジェクト
//////////////////////////////////////////////////////////////////////////////

class	E3DRenderPolygon	: public	ESLObject
{
public:
	// 構築関数
	E3DRenderPolygon( void ) ;
	// 消滅関数
	virtual ~E3DRenderPolygon( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( E3DRenderPolygon, ESLObject )

protected:
	// スレッドメッセージ
	enum	RenderThreadMessage
	{
		rtmQuit		= WM_QUIT,
		rtmRender	= WM_USER + 1,
		rtmRenderDynamically,
		rtmAfterRenderingDynamically,
		rtmRayTracing,
		rtmBeginBuildModel,
	} ;
	// レンダリング用スレッドエントリ
	struct	RENDER_THREAD
	{
		E3DRenderPolygon *	pRenderPoly ;
		HEGL_RENDER_POLYGON	hRenderPoly ;
		HSTACKHEAP			hStackHeap ;		// スタック式ヒープ
		HANDLE				hThread ;			// スレッドハンドル
		DWORD				dwThreadID ;		// スレッドID
		HANDLE				hRenderEvent ;		// コマンド完了
		HANDLE				hRendered ;			// 3D レンダリング完了
		RENDER_THREAD( DWORD dwStackSize ) ;
		~RENDER_THREAD( void ) ;
	} ;
	friend	RENDER_THREAD ;

public:
	// 列挙型
	enum	MultiRenderingFlag
	{
		mrfAuto,
		mrfSingle,
		mrfDual,
		mrfMulti,
	} ;
	enum	ViewIndex
	{
		viewRight	= 0,
		viewLeft	= 1,
		viewCount	= 2,
	} ;
	// ポリゴンエントリーリスト
	struct	POLYGON_LIST
	{
		DWORD					dwCount ;	// ポリゴンエントリ数
		DWORD					dwLimit ;	// ポリゴンエントリ限界数
		PE3D_POLYGON_ENTRY *	pEntries ;	// ポリゴンエントリ

		POLYGON_LIST( void ) ;
		void Release( void ) ;
		void Allocate( HESLHEAP hHeap, DWORD dwBufSize ) ;
		void AddEntry( HESLHEAP hHeap, PE3D_POLYGON_ENTRY pEntry ) ;
	} ;

protected:
	HEGL_RENDER_POLYGON		m_hRenderPoly ;		// 主レンダリングオブジェクト
	DWORD					m_dwSortingFlags ;	// ソートフラグ
	DWORD					m_dwRayTracingFlags ;	// レイトレーシングフラグ
	EGL_RENDER_RAY_TRACE_PARAM
							m_rrtpParam ;

	HESLHEAP				m_hLocalHeap ;		// ローカルヒープ
	HSTACKHEAP				m_hStackHeap ;		// スタック式ヒープ
	DWORD					m_dwStackSize ;

	int						m_iCurrentView ;			// 現在のビュー指標
	POLYGON_LIST			m_plObject[viewCount] ;		// 描画するオブジェクト
	POLYGON_LIST			m_plShadow[viewCount] ;		// 陰を落とすオブジェクト
	POLYGON_LIST			m_plReflect[viewCount] ;	// 映りこむオブジェクト
	POLYGON_LIST			m_plGlobalRef[viewCount] ;	// 距離に関係なく映りこむオブジェクト

	E3DViewPointJoint		m_vpjView ;			// カメラオブジェクト

	PEGL_IMAGE_INFO			m_pDstImage ;		// レンダリング情報
	PEGL_IMAGE_INFO			m_pZBuffer ;
	EGL_RECT				m_rectDst ;

	MultiRenderingFlag		m_mrfRenderingFlag ;
	DWORD					m_dwUsingThreads ;	// 現在レンダリングに使用しているスレッド数
	EObjArray<RENDER_THREAD>
							m_lstThreads ;

	E3D_LIGHT_ENTRY *			m_pLightEntries ;
	E3D_SHADOW_MAP_INFO *		m_pShadowMapEntries ;
	unsigned int				m_nLightCount ;

	bool						m_fGlobalEnvironment ;
	E3D_ENVIRONMENT_MAPPING		m_envmapGlobal ;

	PE3D_GPU_PLUGIN_INTERFACE	m_pGPUInterface ;
	HE3D_GPU_PLUGIN_BUFFER		m_hGPUBuffer ;

	struct	ADD_MODEL_ENTRY
	{
		E3D_REV_MATRIX		rvmat ;
		E3D_VECTOR			vmove ;
		E3DPolygonModel *	pModel ;
		E3D_COLOR *			pColor ;
		unsigned int		nTransparency ;
		DWORD				dwExceptionShadeFlags ;
		E3D_COLOR			bufColor ;
	} ;
	bool						m_fBeginBuildModel ;
	bool						m_fMultiThreadBuildModel ;
	DWORD						m_dwExceptionShadeFlags ;
	EObjArray<ADD_MODEL_ENTRY>	m_queAddModel ;
	ESLEventObject				m_eventAddModel ;
	ESLEventObject				m_eventEndBuildModel ;

	struct	RENDERING_CONTEXT
	{
		HEGL_RENDER_POLYGON	hRender ;
		EGL_DRAW_DEST		ddst ;
		int					yNextLine ;
		int					nThreadLines ;
	} ;
	ESLCriticalSection		m_csSyncContext ;
	RENDERING_CONTEXT		m_rcContext ;

public:
	// 初期化
	virtual ESLError Initialize
		( PEGL_IMAGE_INFO pDstImage, PCEGL_RECT pClipRect,
			PEGL_IMAGE_INFO pZBuffer, PCE3D_VECTOR pScreenPos,
			DWORD dwHeapSize = 0x100000, DWORD dwPolyLimit = 0x8000,
			MultiRenderingFlag mrfFlag = mrfAuto ) ;
	// リソース開放
	virtual ESLError Release( void ) ;
	// レンダリングオブジェクトを生成する
	HEGL_RENDER_POLYGON CreateRenderingObject( void ) ;
	// レンダリングターゲットを設定する
	ESLError SetRenderTarget
		( PEGL_IMAGE_INFO pDstImage, PCEGL_RECT pClipRect,
			PEGL_IMAGE_INFO pZBuffer, PCE3D_VECTOR pScreenPos ) ;
	// ヒープを設定する
	void SetRenderingHeapSize
		( DWORD dwHeapSize = 0x100000, DWORD dwPolyLimit = 0x8000 ) ;
	// ヒープを解放する
	void ReleaseRenderingHeap( void ) ;
	// レンダリングプロセッサ設定
	void SetRenderingProcessorCount( MultiRenderingFlag mrfFlag = mrfAuto ) ;
	// マルチプロセッサ対応取得
	MultiRenderingFlag GetMultiRenderingFlag( void ) const
		{
			return	m_mrfRenderingFlag ;
		}

public:
	// 現在のビュー取得
	int GetCurrentView( void ) const
		{
			return	m_iCurrentView ;
		}
	// ビュー選択
	virtual void SetCurrentView( int iView ) ;
	// モデルエントリ追加開始
	virtual ESLError BeginBuildModel
		( bool fMultiThread = true, DWORD dwExceptionShadeFlags = 0 ) ;
	// モデルエントリ追加終了
	virtual ESLError EndBuildModel( void ) ;
	// モデル追加
	virtual ESLError AddModel
		( E3DModelJoint & model,
			const E3D_COLOR * pColor = NULL,
			unsigned int nTransparency = 0,
			bool fAddSubJoint = true,
			DWORD dwExceptionShadeFlags = 0 ) ;
	// プリミティブ追加
	virtual ESLError AddPrimitive
		( const E3D_PRIMITIVE_POLYGON * pPrimitive,
			const E3D_COLOR * pColor = NULL,
			unsigned int nTransparency = 0,
			DWORD dwExceptionShadeFlags = 0,
			const E3D_COLOR * pvVertexColors = NULL ) ;
	// 同期
	void Lock( void ) const ;
	void Unlock( void ) const ;

protected:
	// モデル構築スレッド
	void BuildModelThreadProc( RENDER_THREAD * prt ) ;
	// モデル追加処理をキューに追加
	void QueueAddModelEntry
		( E3DModelJoint & model,
			E3DPolygonModel * pModel,
			const E3D_COLOR * pColor,
			unsigned int nTransparency,
			DWORD dwExceptionShadeFlags ) ;
	// モデル追加処理
	void AddModelEntry
		( HSTACKHEAP hStackHeap,
			E3DPolygonModel * pModel,
			const E3D_COLOR * pColor,
			unsigned int nTransparency,
			DWORD dwExceptionShadeFlags ) ;
	// モデルエントリ待ち行列を処理する
	void BuildModelQueue( HSTACKHEAP hStackHeap ) ;
	// 同期処理用関数
	typedef	void (E3DRenderPolygon::*PFUNC_LOCK_FUNC)( void ) const ;
	// 非同期処理用関数
	void LockAsync( void ) const ;
	void UnlockAsync( void ) const ;

public:
	// レンダリング準備
	virtual ESLError PrepareRendering( void ) ;
	// レンダリング実行
	virtual ESLError RenderAllPolygon
		( HEGL_RENDER_POLYGON hRenderPoly = NULL,
				bool fRayTracing = false, int nThreadLines = 0 ) ;
	// ポリゴンを消去
	virtual ESLError FlushAllPolygon( void ) ;

public:
	// レンダリングスレッド数取得
	int GetRenderingThreadCount( void ) const
		{
			return	m_lstThreads.GetSize() ;
		}
	// 現在のレンダリングスレッド取得
	int GetCurrentRenderingThread( void ) const ;
	// 現在のレンダリングスレッドの描画用オブジェクト取得
	HEGL_RENDER_POLYGON GetCurrentThreadRenderer( void ) const ;
	// 現在のレンダリングスレッド数取得
	DWORD GetCurrentRenderingThreadCount( void ) const
		{
			return	m_dwUsingThreads ;
		}
	// 全スレッドの3Dレンダリング完了を待つ
	ESLError WaitUntilAllThread3DRendering( DWORD dwTimeout = INFINITE ) const ;
protected:
	// レンダリングスレッド
	static DWORD WINAPI RenderThreadProc( LPVOID param ) ;
	DWORD RenderingThread( RENDER_THREAD * prt ) ;
	// レンダリング実行関数
	virtual void RenderAllPolygonProc
		( HEGL_RENDER_POLYGON hRenderPoly, int iThread ) ;
	// レンダリング後処理関数
	virtual void OnAfterAllRenderPolygon
		( HEGL_RENDER_POLYGON hRenderPoly, int iThread ) ;

protected:
	// 次のレンダリング領域をセットアップ
	bool PrepareNextRenderingRect( RENDER_THREAD * prt ) ;
	// GPU でのレイトレーシングスレッド処理
	void RenderRayTracingByGPUThread
		( HEGL_RENDER_POLYGON hRenderPoly,
			RENDER_THREAD * prtPrim, bool fMultiThread ) ;
	// GPU レイトレーシング準備
	void PrepareGPURayTracing( int nMaxPixelCount ) ;
	// GPU レイトレーシング開始
	ESLError RenderByGPURayTracing
		( HEGL_RENDER_POLYGON hRenderPoly, const EGL_RECT & rctRender ) ;
	// GPU レイトレーシング結果取得
	ESLError GetResultOfGPURayTracing
		( HEGL_RENDER_POLYGON hRenderPoly ) ;
	// 結果を画像バッファに書き込む
	static void WriteFromGPUResultBuffer
		( PEGL_IMAGE_INFO pImageInf,
			const EGL_RECT & rctRender, const DWORD * pdwBuffer ) ;

public:
	// カメラオブジェクト取得
	E3DViewPointJoint & ViewPoint( void )
		{
			return	m_vpjView ;
		}
	// カメラ設定
	template <class T1, class T2> void SetViewPoint
		( const T1 & vViewPoint,
			const T2 & vTarget, double rDegAngle )
		{
			m_vpjView.SetViewPoint( vViewPoint ) ;
			m_vpjView.SetTarget( vTarget ) ;
			m_vpjView.SetRevolveZ( rDegAngle ) ;
			m_vpjView.RefreshJoint( ) ;
		}
	// スクリーン座標取得
	const E3D_VECTOR & GetScreenPos( void ) const
		{
			return	m_hRenderPoly->GetScreenPos() ;
		}
	// レンダリングオブジェクト取得
	operator HEGL_RENDER_POLYGON ( void ) const
		{
			return	m_hRenderPoly ;
		}
	// ヒープ取得
	HSTACKHEAP GetStackHeap( void ) const
		{
			return	m_hStackHeap ;
		}
	// 描画先矩形設定
	void SetRenderingRect( const EGL_RECT & rctDst )
		{
			m_rectDst = rctDst ;
		}
	// 描画先矩形取得
	const EGL_RECT & GetRenderingRect( void ) const
		{
			return	m_rectDst ;
		}
	// 描画オブジェクト取得
	HEGL_DRAW_IMAGE GetDrawImage( void )
		{
			ESLAssert( m_hRenderPoly != NULL ) ;
			return	m_hRenderPoly->GetDrawImage( ) ;
		}
	// 機能フラグを変更
	void ModifyFunctionFlags( DWORD dwAddFlags, DWORD dwRemoveFlags )
		{
			ESLAssert( m_hRenderPoly != NULL ) ;
			m_hRenderPoly->SetFunctionFlags
				( (m_hRenderPoly->GetFunctionFlags()
							& ~dwRemoveFlags) | dwAddFlags ) ;
		}
	// 機能フラグを取得
	DWORD GetFunctionFlags( void ) const
		{
			ESLAssert( m_hRenderPoly != NULL ) ;
			return	m_hRenderPoly->GetFunctionFlags( ) ;
		}
	// 機能フラグを設定
	DWORD SetFunctionFlags( DWORD dwFlags )
		{
			ESLAssert( m_hRenderPoly != NULL ) ;
			return	m_hRenderPoly->SetFunctionFlags( dwFlags ) ;
		}
	// Z クリップ値を設定
	ESLError SetZClipRange( REAL32 rMin, REAL32 rMax )
		{
			ESLAssert( m_hRenderPoly != NULL ) ;
			return	m_hRenderPoly->SetZClipRange( rMin, rMax ) ;
		}
	// 大域環境マッピングを設定
	ESLError SetGlobalEnvironmentMapping
			( const E3D_ENVIRONMENT_MAPPING * envmap ) ;
	// ライトを設定
	ESLError SetLightEntries
			( unsigned int nLightCount, PCE3D_LIGHT_ENTRY pLightEntries ) ;
	// 最小外接矩形を取得
	ESLError GetExternalRect( EGL_RECT * pExtRect ) const
		{
			ESLAssert( m_hRenderPoly != NULL ) ;
			return	m_hRenderPoly->GetExternalRect
						( pExtRect, (PCE3D_POLYGON_ENTRY*)
								m_plObject[m_iCurrentView].pEntries,
								m_plObject[m_iCurrentView].dwCount ) ;
		}
	// ソートフラグを取得する
	DWORD GetSortingFlags( void ) const
		{
			return	m_dwSortingFlags ;
		}
	// ソートフラグを設定する
	void SetSortingFlags( DWORD dwSortingFlags )
		{
			m_dwSortingFlags = dwSortingFlags ;
		}
	// レイトレーシングが有効か？
	bool IsRayTracing( void ) const
		{
			return	(m_dwRayTracingFlags != 0)
						&& (GetFunctionFlags() &
							(E3D_FLAG_RAY_SHADOWING
								| E3D_FLAG_RAY_REFLECTING
								| E3D_FLAG_RAY_REFRACTING)) ;
		}
	// レイトレーシングパラメータを設定する
	ESLError SetRayTracingParameter
			( const EGL_RENDER_RAY_TRACE_PARAM & rrtp )
		{
			ESLAssert( m_hRenderPoly != NULL ) ;
			ESLError	err =
				m_hRenderPoly->SetRayTracingParameter( &rrtp ) ;
			if ( !err )
			{
				m_dwRayTracingFlags = rrtp.dwFlags ;
				m_rrtpParam = rrtp ;
			}
			return	err ;
		}
	// レイトレーシング中断
	ESLError AbortRenderRayTracing( void ) ;
	// GPU プラグインを関連付け
	ESLError AttachGPUInterface
			( PE3D_GPU_PLUGIN_INTERFACE pGPUInterface ) ;

} ;


#endif	//	!defined(COMPACT_NOA_DECODER)

#endif
