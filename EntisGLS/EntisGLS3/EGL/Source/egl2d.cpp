
/*****************************************************************************
                          Entis Graphic Library
 -----------------------------------------------------------------------------
     Copyright (c) 2002-2013 Leshade Entis, Entis-soft. Al rights reserved.
 *****************************************************************************/


#include <egl.h>
#include <math.h>


//////////////////////////////////////////////////////////////////////////////
// 3x3 変換行列
//////////////////////////////////////////////////////////////////////////////

void E3D_REV_MATRIX::InitializeMatrix( const E3D_VECTOR & vector )
{
	//
	// 対角行列初期化
	//
	matrix[0][0] = vector.x ;
	matrix[1][1] = vector.y ;
	matrix[2][2] = vector.z ;
	matrix[0][1] = matrix[0][2] = matrix[0][3]
		= matrix[1][0] = matrix[1][2] = matrix[1][3]
		= matrix[2][0] = matrix[2][1] = matrix[2][3] = 0 ;
}

void E3DDF_REV_MATRIX::InitializeMatrix( const E3DDF_VECTOR & vector )
{
	//
	// 対角行列初期化
	//
	matrix[0][0] = vector.x ;
	matrix[1][1] = vector.y ;
	matrix[2][2] = vector.z ;
	matrix[0][1] = matrix[0][2]
		= matrix[1][0] = matrix[1][2]
		= matrix[2][0] = matrix[2][1] = 0 ;
}

const E3DDF_REV_MATRIX &
	E3DDF_REV_MATRIX::operator += ( const E3DDF_REV_MATRIX & m )
{
	for ( int i = 0; i < 3; i ++ )
	{
		matrix[i][0] += m.matrix[i][0] ;
		matrix[i][1] += m.matrix[i][1] ;
		matrix[i][2] += m.matrix[i][2] ;
	}
	return	*this ;
}

const E3DDF_REV_MATRIX &
	E3DDF_REV_MATRIX::operator -= ( const E3DDF_REV_MATRIX & m )
{
	for ( int i = 0; i < 3; i ++ )
	{
		matrix[i][0] -= m.matrix[i][0] ;
		matrix[i][1] -= m.matrix[i][1] ;
		matrix[i][2] -= m.matrix[i][2] ;
	}
	return	*this ;
}

const E3DDF_REV_MATRIX & E3DDF_REV_MATRIX::operator *= ( double number )
{
	for ( int i = 0; i < 3; i ++ )
	{
		matrix[i][0] *= number ;
		matrix[i][1] *= number ;
		matrix[i][2] *= number ;
	}
	return	*this ;
}

const E3DDF_REV_MATRIX & E3DDF_REV_MATRIX::operator /= ( double number )
{
	for ( int i = 0; i < 3; i ++ )
	{
		matrix[i][0] /= number ;
		matrix[i][1] /= number ;
		matrix[i][2] /= number ;
	}
	return	*this ;
}

E3DDF_REV_MATRIX E3DDF_REV_MATRIX::operator - ( void ) const
{
	E3DDF_REV_MATRIX	result ;
	for ( int i = 0; i < 3; i ++ )
	{
		result.matrix[i][0] = - matrix[i][0] ;
		result.matrix[i][1] = - matrix[i][1] ;
		result.matrix[i][2] = - matrix[i][2] ;
	}
	return	result ;
}

double E3DDF_REV_MATRIX::Determinant( void ) const
{
	return	matrix[0][0]
				* (matrix[1][1] * matrix[2][2]
					- matrix[2][1] * matrix[1][2])
			+ matrix[0][1]
				* (matrix[1][2] * matrix[2][0]
					- matrix[2][2] * matrix[1][0])
			+ matrix[0][2]
				* (matrix[1][0] * matrix[2][1]
					- matrix[2][0] * matrix[1][1]) ;
}

E3DDF_REV_MATRIX & E3DDF_REV_MATRIX::InverseOf( const E3DDF_REV_MATRIX & m )
{
	double	d = 1.0 / m.Determinant() ;
	//
	static const int	nMod3[6] = { 0, 1, 2, 0, 1, 2 } ;
	for ( int i = 0; i < 3; i ++ )
	{
		int	i1 = nMod3[i + 1] ;
		int	i2 = nMod3[i + 2] ;
		//
		for ( int j = 0; j < 3; j ++ )
		{
			int	j1 = nMod3[j + 1] ;
			int	j2 = nMod3[j + 2] ;
			//
			matrix[j][i] =
				d * (m.matrix[i1][j1] * m.matrix[i2][j2]
						- m.matrix[i1][j2] * m.matrix[i2][j1]) ;
		}
	}
	return	*this ;
}

void E3DDF_REV_MATRIX::RevolveOnX( double rSin, double rCos )
{
	//
	// ( a11 a12 a13 )   ( 1    0       0    )
	// ( a21 a22 a23 ) X ( 0  cos(x)  sin(x) )
	// ( a31 a32 a33 )   ( 0 -sin(x)  cos(x) )
	//
	for ( int i = 0; i < 3; i ++ )
	{
		double	r1, r2 ;
		r1 = matrix[i][1] ;
		r2 = matrix[i][2] ;
		matrix[i][1] = (rCos * r1 - rSin * r2) ;
		matrix[i][2] = (rCos * r2 + rSin * r1) ;
	}
}

void E3DDF_REV_MATRIX::RevolveOnY( double rSin, double rCos )
{
	//
	// ( a11 a12 a13 )   (  cos(y)  0  sin(y) )
	// ( a21 a22 a23 ) X (    0     1    0    )
	// ( a31 a32 a33 )   ( -sin(y)  0  cos(y) )
	//
	for ( int i = 0; i < 3; i ++ )
	{
		double	r1, r2 ;
		r1 = matrix[i][0] ;
		r2 = matrix[i][2] ;
		matrix[i][0] = (rCos * r1 - rSin * r2) ;
		matrix[i][2] = (rSin * r1 + rCos * r2) ;
	}
}

void E3DDF_REV_MATRIX::RevolveOnZ( double rSin, double rCos )
{
	//
	// ( a11 a12 a13 )   ( cos(x) -sin(z)  0 )
	// ( a21 a22 a23 ) X ( sin(x)  cos(z)  0 )
	// ( a31 a32 a33 )   (   0       0     1 )
	//
	for ( int i = 0; i < 3; i ++ )
	{
		double	r1, r2 ;
		r1 = matrix[i][0] ;
		r2 = matrix[i][1] ;
		matrix[i][0] = (rCos * r1 + rSin * r2) ;
		matrix[i][1] = (rCos * r2 - rSin * r1) ;
	}
}

void E3DDF_REV_MATRIX::RevolveByAngleOn( const E3DDF_VECTOR & angle )
{
	double	rXxZz = angle.x * angle.x + angle.z * angle.z ;
	double	rSqrtXY = sqrt( rXxZz ) ;
	double	rSqrtXYZ = sqrt( rXxZz + angle.y * angle.y ) ;
	if ( rSqrtXYZ > 1.0e-32 )
	{
		RevolveOnX( (- angle.y / rSqrtXYZ), (rSqrtXY / rSqrtXYZ) ) ;
	}
	if ( rSqrtXY > 1.0e-32 )
	{
		RevolveOnY( (- angle.x / rSqrtXY), (angle.z / rSqrtXY) ) ;
	}
}

void E3DDF_REV_MATRIX::RevolveForAngle( const E3DDF_VECTOR & angle )
{
	double	rXxZz = angle.x * angle.x + angle.z * angle.z ;
	double	rSqrtXY = sqrt( rXxZz ) ;
	double	rSqrtXYZ = sqrt( rXxZz + angle.y * angle.y ) ;
	if ( rSqrtXY > 1.0e-32 )
	{
		RevolveOnY( (angle.x / rSqrtXY), (angle.z / rSqrtXY) ) ;
	}
	if ( rSqrtXYZ > 1.0e-32 )
	{
		RevolveOnX( (angle.y / rSqrtXYZ), (rSqrtXY / rSqrtXYZ) ) ;
	}
}

void E3DDF_REV_MATRIX::MagnifyByVector( const E3DDF_VECTOR & vector )
{
	for ( int i = 0; i < 3; i ++ )
	{
		matrix[i][0] = matrix[i][0] * vector.x ;
		matrix[i][1] = matrix[i][1] * vector.y ;
		matrix[i][2] = matrix[i][2] * vector.z ;
	}
}

void E3DDF_REV_MATRIX::RevolveMatrix( E3DDF_REV_MATRIX & matDst ) const
{
	//
	// matDst <- this * matDst
	//
	E3DDF_REV_MATRIX	matTemp = matDst ;
	matDst = *this ;
	matDst.RevolveByMatrix( matTemp ) ;
}

void E3DDF_REV_MATRIX::RevolveByMatrix( const E3DDF_REV_MATRIX & matSrc )
{
	//
	// this <- this * matSrc
	//
	for ( int i = 0; i < 3; i ++ )
	{
		double	r1, r2, r3 ;
		r1 = matrix[i][0] ;
		r2 = matrix[i][1] ;
		r3 = matrix[i][2] ;
		matrix[i][0] =
			(r1 * matSrc.matrix[0][0]
					+ r2 * matSrc.matrix[1][0] + r3 * matSrc.matrix[2][0]) ;
		matrix[i][1] =
			(r1 * matSrc.matrix[0][1]
					+ r2 * matSrc.matrix[1][1] + r3 * matSrc.matrix[2][1]) ;
		matrix[i][2] =
			(r1 * matSrc.matrix[0][2]
					+ r2 * matSrc.matrix[1][2] + r3 * matSrc.matrix[2][2]) ;
	}
}

void E3DDF_REV_MATRIX::RevolveVector( E3DDF_VECTOR & vector ) const
{
	E3DDF_VECTOR	v = vector ;
	vector.x = matrix[0][0] * v.x + matrix[0][1] * v.y + matrix[0][2] * v.z ;
	vector.y = matrix[1][0] * v.x + matrix[1][1] * v.y + matrix[1][2] * v.z ;
	vector.z = matrix[2][0] * v.x + matrix[2][1] * v.y + matrix[2][2] * v.z ;
}

E3DRevMatrix::E3DRevMatrix( void )
{
	InitializeMatrix( E3DVector( 1, 1, 1 ) ) ;
}

E3DRevMatrix::E3DRevMatrix( const E3D_VECTOR & vUnit )
{
	InitializeMatrix( vUnit ) ;
}

E3DRevMatrix::E3DRevMatrix( const E3D_REV_MATRIX & m )
{
	*this = m ;
}

E3DRevMatrix::E3DRevMatrix( const E3DDF_REV_MATRIX & m )
{
	*this = m ;
}

const E3DRevMatrix & E3DRevMatrix::operator = ( const E3D_REV_MATRIX & m )
{
	for ( int i = 0; i < 3; i ++ )
	{
		matrix[i][0] = m.matrix[i][0] ;
		matrix[i][1] = m.matrix[i][1] ;
		matrix[i][2] = m.matrix[i][2] ;
	}
	return	*this ;
}

const E3DRevMatrix & E3DRevMatrix::operator = ( const E3DDF_REV_MATRIX & m )
{
	for ( int i = 0; i < 3; i ++ )
	{
		matrix[i][0] = (REAL32) m.matrix[i][0] ;
		matrix[i][1] = (REAL32) m.matrix[i][1] ;
		matrix[i][2] = (REAL32) m.matrix[i][2] ;
	}
	return	*this ;
}

E3DDFRevMatrix::E3DDFRevMatrix( void )
{
	InitializeMatrix( E3DDFVector( 1, 1, 1 ) ) ;
}

E3DDFRevMatrix::E3DDFRevMatrix( const E3DDF_VECTOR & vUnit )
{
	InitializeMatrix( vUnit ) ;
}

E3DDFRevMatrix::E3DDFRevMatrix( const E3DDF_REV_MATRIX & m )
{
	*this = m ;
}

E3DDFRevMatrix::E3DDFRevMatrix( const E3D_REV_MATRIX & m )
{
	*this = m ;
}

const E3DDFRevMatrix & E3DDFRevMatrix::operator = ( const E3DDF_REV_MATRIX & m )
{
	for ( int i = 0; i < 3; i ++ )
	{
		matrix[i][0] = m.matrix[i][0] ;
		matrix[i][1] = m.matrix[i][1] ;
		matrix[i][2] = m.matrix[i][2] ;
	}
	return	*this ;
}

const E3DDFRevMatrix & E3DDFRevMatrix::operator = ( const E3D_REV_MATRIX & m )
{
	for ( int i = 0; i < 3; i ++ )
	{
		matrix[i][0] = m.matrix[i][0] ;
		matrix[i][1] = m.matrix[i][1] ;
		matrix[i][2] = m.matrix[i][2] ;
	}
	return	*this ;
}


//////////////////////////////////////////////////////////////////////////////
// 画像バッファインターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( EGLImageBufferInterface, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
EGLImageBufferInterface::EGLImageBufferInterface( void )
{
	dwType = 0 ;
	ptrObject = NULL ;
	pNextInterface = NULL ;
	pfnRelease = ReleaseProc ;
	pfnUpdateBuffer = UpdateImageBufferProc ;
	pfnCommitBuffer = CommitBufferProc ;
	pfnReflectBuffer = ReflectBufferProc ;
	m_pAttached = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
EGLImageBufferInterface::~EGLImageBufferInterface( void )
{
	DetachBufferInterface() ;
}

// コールバック関数
//////////////////////////////////////////////////////////////////////////////
void EGLImageBufferInterface::ReleaseProc
	( PEGL_IMAGE_INFO pImageInf,
		EGL_IMAGE_BUFFER_INTERFACE * pInterface )
{
	delete	FromPtr(pInterface) ;
}

ESLError EGLImageBufferInterface::UpdateImageBufferProc
	( PEGL_IMAGE_INFO pImageInf,
		EGL_IMAGE_BUFFER_INTERFACE * pInterface, const EGL_RECT * pRect )
{
	return	FromPtr(pInterface)->UpdateBuffer( pImageInf, pRect ) ;
}

ESLError EGLImageBufferInterface::CommitBufferProc
	( PEGL_IMAGE_INFO pImageInf,
		EGL_IMAGE_BUFFER_INTERFACE * pInterface )
{
	return	FromPtr(pInterface)->CommitBuffer( pImageInf ) ;
}

ESLError EGLImageBufferInterface::ReflectBufferProc
	( PEGL_IMAGE_INFO pImageInf,
		EGL_IMAGE_BUFFER_INTERFACE * pInterface, const EGL_RECT * pRect )
{
	return	FromPtr(pInterface)->ReflectBuffer( pImageInf, pRect ) ;
}

// 画像バッファにインターフェース設定
//////////////////////////////////////////////////////////////////////////////
ESLError EGLImageBufferInterface::SetBufferInterface( PEGL_IMAGE_INFO pImageInf )
{
	if ( m_pAttached != pImageInf )
	{
		if ( m_pAttached != NULL )
		{
			DetachBufferInterface() ;
		}
		eglAddBufferUpdateInterface( pImageInf, this ) ;
		m_pAttached = pImageInf ;
	}
	return	eslErrSuccess ;
}

// 画像バッファのインターフェース解除
//////////////////////////////////////////////////////////////////////////////
void EGLImageBufferInterface::DetachBufferInterface( void )
{
	if ( m_pAttached != NULL )
	{
		eglDetachBufferUpdateInterface( m_pAttached, this ) ;
		m_pAttached = NULL ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// 画像オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( EGLImage, ESLObject )

// 画像展開コールバック関数
//////////////////////////////////////////////////////////////////////////////
ESLError EGLImage::EGLImageDecoder::OnDecodedBlock
	( LONG line, LONG column, const EGL_IMAGE_RECT & rect )
{
	if ( m_pfnCallback != NULL )
	{
		DWORD	dwTotal = m_nWidthBlocks * m_nHeightBlocks ;
		DWORD	dwDecoded = line * m_nWidthBlocks + column + 1 ;
		return	m_pfnCallback
			( m_ptrImage, m_ptrCallbackData, dwDecoded, dwTotal ) ;
	}
	return	eslErrSuccess ;
}

// 画像バッファ関連付け
//////////////////////////////////////////////////////////////////////////////
void EGLImage::AttachImage( PEGL_IMAGE_INFO pImage )
{
	EGLImage::DeleteImage( ) ;
	m_pImage = pImage ;
	m_iofOwnerFlag = iofRefInfo ;
}

// 画像バッファの参照を設定
//////////////////////////////////////////////////////////////////////////////
void EGLImage::SetImageView
	( PEGL_IMAGE_INFO pImage, PCEGL_RECT pViewRect )
{
	EGLImage::DeleteImage( ) ;
	//
	if ( pImage != NULL )
	{
		m_iofOwnerFlag = iofRefBuffer ;
		m_pImage = ::eglCreateTextureInfo
						( pImage, pViewRect, EGL_IMAGE_NO_DUP ) ;
	}
}

// 画像バッファ作成
//////////////////////////////////////////////////////////////////////////////
PEGL_IMAGE_INFO EGLImage::CreateImage
	( DWORD fdwFormat, DWORD dwWidth, DWORD dwHeight,
			DWORD dwBitsPerPixel, DWORD dwFlags )
{
	EGLImage::DeleteImage( ) ;
	//
	m_iofOwnerFlag = iofOwnBuffer ;
	m_pImage = ::eglCreateImageBuffer
		( fdwFormat, dwWidth, dwHeight, dwBitsPerPixel, dwFlags ) ;
	//
	return	m_pImage ;
}

// 画像バッファ複製
//////////////////////////////////////////////////////////////////////////////
PEGL_IMAGE_INFO EGLImage::DuplicateImage
	( PCEGL_IMAGE_INFO pImage, DWORD dwFlags )
{
	EGLImage::DeleteImage( ) ;
	//
	m_iofOwnerFlag = iofOwnBuffer ;
	m_pImage = ::eglDuplicateImageBuffer( pImage, dwFlags ) ;
	//
	return	m_pImage ;
}

// 画像ファイルを読み込む
//////////////////////////////////////////////////////////////////////////////
ESLError EGLImage::ReadImageFile
	( ESLFileObject & file,
		PFUNC_CALLBACK_DECODE pfnCallback, void * ptrCallbackData )
{
	EGLImage::DeleteImage( ) ;
	//
	// ファイルヘッダを読み込む
	//
	ERIFile			erif ;
	if ( erif.Open( &file ) )
	{
		return	ESLErrorMsg( "画像ファイルを開けませんでした。" ) ;
	}
	//
	// 画像バッファを生成する
	//
	ERI_INFO_HEADER &	eih = erif.m_InfoHeader ;
	SDWORD	nWidth = eih.nImageWidth ;
	SDWORD	nHeight = eih.nImageHeight ;
	if ( nHeight < 0 )
		nHeight = - nHeight ;
	DWORD	dwBitsPerPixel = eih.dwBitsPerPixel ;
	if ( dwBitsPerPixel == 24 )
		dwBitsPerPixel = 32 ;
	//
	m_iofOwnerFlag = iofOwnBuffer ;
	m_pImage = ::eglCreateImageBuffer
		( eih.fdwFormatType, nWidth, nHeight, dwBitsPerPixel, 0 ) ;
	if ( m_pImage == NULL )
	{
		return	ESLErrorMsg
			( "展開する画像バッファを確保できませんでした。" ) ;
	}
	if ( (m_pImage->fdwFormatType & EIF_WITH_PALETTE)
		&& (m_pImage->pPaletteEntries != NULL)
		&& (erif.m_fdwReadMask & ERIFile::rmPaletteTable) )
	{
		::eslMoveMemory
			( m_pImage->pPaletteEntries,
				erif.m_PaletteTable,
				sizeof(EGL_PALETTE) * m_pImage->dwPaletteCount ) ;
	}
	//
	// 画像を展開する
	//
	EGLImageDecoder		decoder( this, pfnCallback, ptrCallbackData ) ;
	ERISADecodeContext	context( 0x10000 ) ;
	context.AttachInputFile( &erif ) ;
	if ( decoder.Initialize( eih ) )
	{
		return	ESLErrorMsg( "未対応の画像フォーマットです。" ) ;
	}
	if ( pfnCallback != NULL )
	{
		pfnCallback( this, ptrCallbackData, 0, 0 ) ;
	}
	if ( decoder.DecodeImage( *m_pImage, context, 0 ) )
	{
		return	ESLErrorMsg( "画像の展開に失敗しました。" ) ;
	}
	ReverseVertically( ) ;
	//
	return	eslErrSuccess ;
}

// 画像バッファ消去
//////////////////////////////////////////////////////////////////////////////
void EGLImage::DeleteImage( void )
{
	if ( m_iofOwnerFlag != iofRefInfo )
	{
		::eglDeleteImageBuffer( m_pImage ) ;
	}
	m_iofOwnerFlag = iofRefInfo ;
	m_pImage = NULL ;
}

// プレビュー画像を読み込む
//////////////////////////////////////////////////////////////////////////////
ESLError EGLImage::ReadPreviewImage
	( ESLFileObject & file,
		DWORD dwMaxWidth, DWORD dwMaxHeight,
		DWORD dwPreviewFlags, EGL_SIZE * pOriginalSize,
		EGLImage::PFUNC_CALLBACK_DECODE pfnCallback, void * ptrCallbackData )
{
	EGLImage::DeleteImage( ) ;
	//
	// ファイルヘッダを読み込む
	//
	ERIFile		erif ;
	if ( erif.Open( &file, erif.otOpenStream ) )
	{
		return	ESLErrorMsg( "ファイルヘッダを読み込めませんでした。" ) ;
	}
	if ( !(erif.m_fdwReadMask & erif.rmImageInfo) )
	{
		return	ESLErrorMsg( "画像データがありません。" ) ;
	}
	//
	// プレビュー画像を使うか？
	//
	const ERI_INFO_HEADER *	peih = &(erif.m_InfoHeader) ;
	bool					fPreview = false ;
	EGL_SIZE				sizeImage ;
	sizeImage.w = peih->nImageWidth ;
	sizeImage.h = peih->nImageHeight ;
	if ( sizeImage.h < 0 )
	{
		sizeImage.h = - sizeImage.h ;
	}
	if ( pOriginalSize != NULL )
	{
		*pOriginalSize = sizeImage ;
	}
	if ( ((DWORD) sizeImage.w > dwMaxWidth)
		|| ((DWORD) sizeImage.h > dwMaxHeight) )
	{
		if ( erif.m_fdwReadMask & erif.rmPreviewInfo )
		{
			EGL_SIZE	sizePreview ;
			sizePreview.w = erif.m_PreviewInfo.nImageWidth ;
			sizePreview.h = erif.m_PreviewInfo.nImageHeight ;
			if ( sizePreview.h < 0 )
				sizePreview.h = - sizePreview.h ;
			//
			if ( (sizePreview.w < sizeImage.w)
					&& (sizePreview.h < sizeImage.h) )
			{
				peih = &(erif.m_PreviewInfo) ;
				sizeImage = sizePreview ;
				fPreview = true ;
			}
		}
		if ( !fPreview && !(dwPreviewFlags & pfForcePreview) )
		{
			return	ESLErrorMsg( "プレビュー画像がありません" ) ;
		}
	}
	//
	// 画像レコードを開く
	//
	UINT64	uiRecID ;
	if ( fPreview )
	{
		uiRecID = *((UINT64*)"Preview ") ;
	}
	else
	{
		uiRecID = *((UINT64*)"ImageFrm") ;
	}
	for ( ; ; )
	{
		if ( erif.DescendRecord( ) )
		{
			return	ESLErrorMsg( "画像データが見つかりません" ) ;
		}
		if ( erif.GetRecordID() == uiRecID )
		{
			break ;
		}
		if ( erif.GetRecordID() == *((UINT64*)"Palette ") )
		{
			::eslFillMemory
				( erif.m_PaletteTable, 0, sizeof(erif.m_PaletteTable) ) ;
			erif.Read( erif.m_PaletteTable, sizeof(erif.m_PaletteTable) ) ;
		}
		erif.AscendRecord( ) ;
	}
	//
	// 画像バッファを生成する
	//
	DWORD	fdwDecFlags =
		ERISADecoder::dfQuickDecode | ERISADecoder::dfNoLoopFilter ;
	DWORD	dwBitsPerPixel = peih->dwBitsPerPixel ;
	if ( dwBitsPerPixel == 24 )
	{
		dwBitsPerPixel = 32 ;
	}
	if ( !(dwPreviewFlags & pfNoResizeDecoding) )
	{
		if ( peih->fdwTransformation == CVTYPE_LOSSLESS_ERI )
		{
			int	d = peih->dwBlockingDegree ;
			if ( ((DWORD) (sizeImage.w >> d) >= (dwMaxWidth >> 1))
				|| ((DWORD) (sizeImage.h >> d) >= (dwMaxHeight >> 1)) )
			{
				sizeImage.w >>= d ;
				sizeImage.h >>= d ;
				fdwDecFlags = ERISADecoder::dfPreviewDecode ;
			}
		}
	}
	m_iofOwnerFlag = iofOwnBuffer ;
	m_pImage = ::eglCreateImageBuffer
		( peih->fdwFormatType, sizeImage.w, sizeImage.h, dwBitsPerPixel, 0 ) ;
	if ( m_pImage == NULL )
	{
		return	ESLErrorMsg
			( "展開する画像バッファを確保できませんでした。" ) ;
	}
	if ( (m_pImage->fdwFormatType & EIF_WITH_PALETTE)
		&& (m_pImage->pPaletteEntries != NULL) )
	{
		::eslMoveMemory
			( m_pImage->pPaletteEntries,
				erif.m_PaletteTable,
				sizeof(EGL_PALETTE) * m_pImage->dwPaletteCount ) ;
	}
	//
	// 画像を展開する
	//
	EGLImageDecoder		decoder( this, pfnCallback, ptrCallbackData ) ;
	ERISADecodeContext	context( 0x10000 ) ;
	context.AttachInputFile( &erif ) ;
	if ( decoder.Initialize( *peih ) )
	{
		return	ESLErrorMsg( "未対応の画像フォーマットです。" ) ;
	}
	if ( pfnCallback != NULL )
	{
		pfnCallback( this, ptrCallbackData, 0, 0 ) ;
	}
	if ( decoder.DecodeImage( *m_pImage, context, fdwDecFlags ) )
	{
		return	ESLErrorMsg( "画像の展開に失敗しました。" ) ;
	}
	//
	// 画像サイズを補正する
	//
	if ( (dwPreviewFlags & pfResizePreview)
		&& ((dwMaxWidth < m_pImage->dwImageWidth)
			|| (dwMaxHeight < m_pImage->dwImageHeight)) )
	{
		//
		// 補正した画像バッファを生成する
		//
		DWORD	fdwFormat = m_pImage->fdwFormatType ;
		if ( fdwFormat & EIF_WITH_CLIPPING )
		{
			fdwFormat = (fdwFormat & EIF_TYPE_MASK) | EIF_WITH_ALPHA ;
		}
		else
		{
			fdwFormat &= EIF_TYPE_MASK ;
		}
		EGL_SIZE	sizeResize ;
		double		rWidthRate, rHeightRate, rRate ;
		rWidthRate = (int) dwMaxWidth / (double) sizeImage.w ;
		rHeightRate = (int) dwMaxHeight / (double) sizeImage.h ;
		if ( rWidthRate <= rHeightRate )
		{
			rRate = rWidthRate ;
		}
		else
		{
			rRate = rHeightRate ;
		}
		sizeResize.w = (int) ::eriRoundR64ToLInt( sizeImage.w * rRate ) ;
		sizeResize.h = (int) ::eriRoundR64ToLInt( sizeImage.h * rRate ) ;
		//
		PEGL_IMAGE_INFO	pResizeImage =
			::eglCreateImageBuffer
				( fdwFormat, sizeResize.w, sizeResize.h, 32 ) ;
		if ( pResizeImage != NULL )
		{
			//
			// 縮小描画する
			//
			EGL_DRAW_PARAM	dp ;
			EGL_IMAGE_AXES	iax ;
			::eslFillMemory( &dp, 0, sizeof(dp) ) ;
			iax.xAxis.x = (REAL32) rRate ;
			iax.xAxis.y = 0 ;
			iax.yAxis.x = 0 ;
			iax.yAxis.y = (REAL32) rRate ;
			dp.pSrcImage = m_pImage ;
			dp.pImageAxes = &iax ;
			//
			HEGL_DRAW_IMAGE	hDraw = ::eglCreateDrawImage( ) ;
			hDraw->Initialize( pResizeImage, NULL, NULL ) ;
			if ( !hDraw->PrepareDraw( &dp ) )
			{
				hDraw->DrawImage( ) ;
				//
				::eglDeleteImageBuffer( m_pImage ) ;
				m_pImage = pResizeImage ;
			}
			else
			{
				::eglDeleteImageBuffer( pResizeImage ) ;
			}
			hDraw->Release( ) ;
		}
	}
	//
	ReverseVertically( ) ;
	//
	return	eslErrSuccess ;
}

// 画像ファイルへ書き出す
//////////////////////////////////////////////////////////////////////////////
ESLError EGLImage::WriteImageFile
	( ESLFileObject & file,
		EGLImage::CompressTypeFlag ctfType,
		DWORD dwFlags, const ERISAEncoder::PARAMETER * periep )
{
	if ( m_pImage == NULL )
	{
		return	eslErrGeneral ;
	}
	//
	// ファイルを開く
	//
	ESLError	err ;
	ERIAnimationWriter	eriaw ;
	err = eriaw.Open( &file, eriaw.fidImage ) ;
	if ( err )
		return	err ;
	//
	// パラメータを設定する
	//
	ERI_INFO_HEADER	eih ;
	::eslFillMemory( &eih, 0, sizeof(eih) ) ;
	eih.dwVersion =
		(ctfType == ctfCompatibleFormat) ? 0x00020100 : 0x00020200 ;
	eih.fdwTransformation = CVTYPE_LOSSLESS_ERI ;
	if ( ctfType == ctfCompatibleFormat )
		eih.dwArchitecture = ERI_RUNLENGTH_GAMMA ;
	else if ( ctfType == ctfExtendedFormat )
		eih.dwArchitecture = ERI_RUNLENGTH_HUFFMAN ;
	else
		eih.dwArchitecture = ERISA_NEMESIS_CODE ;
	//
	eih.fdwFormatType = m_pImage->fdwFormatType ;
	eih.nImageWidth = (SDWORD) m_pImage->dwImageWidth ;
	eih.nImageHeight = (SDWORD) m_pImage->dwImageHeight ;
	if ( dwFlags & ERISAEncoder::efTopDown )
		eih.nImageHeight = - eih.nImageHeight ;
	eih.dwBitsPerPixel = m_pImage->dwBitsPerPixel ;
	eih.dwClippedPixel = m_pImage->dwClippedPixel ;
	eih.dwBlockingDegree = 3 ;
	//
	if ( periep != NULL )
	{
		eih.fdwTransformation = CVTYPE_LOT_ERI ;
		eih.dwArchitecture = ERI_RUNLENGTH_GAMMA ;
		eriaw.SetImageCompressionParameter( *periep ) ;
	}
	//
	// ファイルヘッダを書き出す
	//
	err = eriaw.BeginFileHeader( 1, 0 ) ;
	if ( err )
		return	err ;
	err = eriaw.WriteEriInfoHeader( eih ) ;
	if ( err )
		return	err ;
	eriaw.EndFileHeader( ) ;
	//
	// 画像データを書き出す
	//
	err = eriaw.BeginStream( ) ;
	if ( err )
		return	err ;
	if ( (m_pImage->fdwFormatType & EIF_WITH_PALETTE)
		&& (m_pImage->pPaletteEntries != NULL) )
	{
		err = eriaw.WritePaletteTable
			( m_pImage->pPaletteEntries, m_pImage->dwPaletteCount ) ;
		if ( err )
			return	err ;
	}
	if ( !(dwFlags & ERISAEncoder::efTopDown) )
		ReverseVertically( ) ;
	err = eriaw.WriteImageData( *m_pImage, dwFlags ) ;
	if ( !(dwFlags & ERISAEncoder::efTopDown) )
		ReverseVertically( ) ;
	if ( err )
		return	err ;
	err = eriaw.EndStream( 0 ) ;
	if ( err )
		return	err ;
	//
	// 完了
	//
	eriaw.Close( ) ;
	return	eslErrSuccess ;
}


//////////////////////////////////////////////////////////////////////////////
// アニメーション画像オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( EGLAnimation, EGLImage )

// ERI アニメーション展開オブジェクト
//////////////////////////////////////////////////////////////////////////////
class	EGL_ERIAnimation	: public	ERIAnimation
{
public:
	EGLAnimation *					m_ptrImage ;
	EGLImage::PFUNC_CALLBACK_DECODE	m_pfnCallback ;
	void *							m_ptrCallbackData ;
public:
	EGL_ERIAnimation( EGLAnimation * ptrImage,
			EGLImage::PFUNC_CALLBACK_DECODE pfnCallback, void * ptrData )
		: m_ptrImage( ptrImage ),
			m_pfnCallback( pfnCallback ), m_ptrCallbackData( ptrData ) { }
	virtual ERISADecoder * CreateERIDecoder( void ) ;
} ;

ERISADecoder * EGL_ERIAnimation::CreateERIDecoder( void )
{
	return	new EGLImage::EGLImageDecoder
		( m_ptrImage, m_pfnCallback, m_ptrCallbackData ) ;
}

// 構築関数
//////////////////////////////////////////////////////////////////////////////
EGLAnimation::EGLAnimation( void )
{
	m_ptHotSpot.x = 0 ;
	m_ptHotSpot.y = 0 ;
	m_nResolution = 0 ;
	m_dwTotalTime = 0 ;
	m_dwCurrent = 0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
EGLAnimation::~EGLAnimation( void )
{
	DeleteImage( ) ;
}

// 画像バッファの参照を設定
//////////////////////////////////////////////////////////////////////////////
void EGLAnimation::SetImageView
	( PEGL_IMAGE_INFO pImage, PCEGL_RECT pViewRect )
{
	if ( m_lstFrames.GetSize() == 0 )
	{
		EGLAnimation::DeleteImage( ) ;
		//
		m_iofOwnerFlag = iofRefInfo ;
		m_pImage = ::eglCreateTextureInfo( pImage, pViewRect, EGL_IMAGE_NO_DUP ) ;
		m_lstFrames.Add( m_pImage ) ;
		m_lstSequence.Add( 0 ) ;
	}
	else
	{
		EGLImage::SetImageView( pImage, pViewRect ) ;
	}
}

// 画像バッファ作成
//////////////////////////////////////////////////////////////////////////////
PEGL_IMAGE_INFO EGLAnimation::CreateImage
	( DWORD fdwFormat, DWORD dwWidth, DWORD dwHeight,
				DWORD dwBitsPerPixel, DWORD dwFlags )
{
	EGLAnimation::DeleteImage( ) ;
	//
	m_pImage = ::eglCreateImageBuffer
		( fdwFormat, dwWidth, dwHeight, dwBitsPerPixel, dwFlags ) ;
	if ( m_pImage != NULL )
	{
		m_iofOwnerFlag = iofRefInfo ;
		m_lstFrames.Add( m_pImage ) ;
		m_lstSequence.Add( 0 ) ;
	}
	return	m_pImage ;
}

// 画像バッファ複製
//////////////////////////////////////////////////////////////////////////////
PEGL_IMAGE_INFO EGLAnimation::DuplicateImage
	( PCEGL_IMAGE_INFO pImage, DWORD dwFlags )
{
	EGLAnimation::DeleteImage( ) ;
	//
	m_pImage = ::eglDuplicateImageBuffer( pImage, dwFlags ) ;
	if ( m_pImage != NULL )
	{
		m_iofOwnerFlag = iofRefInfo ;
		m_lstFrames.Add( m_pImage ) ;
		m_lstSequence.Add( 0 ) ;
	}
	return	m_pImage ;
}

// 画像ファイルを読み込む
//////////////////////////////////////////////////////////////////////////////
ESLError EGLAnimation::ReadImageFile
	( ESLFileObject & file,
		PFUNC_CALLBACK_DECODE pfnCallback, void * ptrCallbackData )
{
	EGLAnimation::DeleteImage( ) ;
	//
	// ファイルを開く
	//
	EGL_ERIAnimation	eria( this, pfnCallback, ptrCallbackData ) ;
	if ( eria.Open( &file ) )
	{
		return	ESLErrorMsg( "画像ファイルを開けませんでした。" ) ;
	}
	//
	// アニメーション画像を取得
	//
	int		nFrameCount = eria.GetAllFrameCount( ) ;
	int		i, nLastFrame = -1 ;
	for ( i = 0; i < nFrameCount; i ++ )
	{
		if ( (int) eria.CurrentIndex() <= nLastFrame )
		{
			break ;
		}
		const EGL_IMAGE_INFO *	pFrame = eria.GetImageInfo( ) ;
		if ( pFrame != NULL )
		{
			m_lstFrames.Add( ::eglDuplicateImageBuffer( pFrame ) ) ;
		}
		eria.SeekToNextFrame( ) ;
	}
	for ( i = 0; i < (int) m_lstFrames.GetSize(); i ++ )
	{
		PEGL_IMAGE_INFO	pImage = m_lstFrames.GetAt( i ) ;
		if ( pImage != NULL )
		{
			::eglReverseVertically( pImage ) ;
		}
	}
	//
	// ホットスポット・解像度・参照画像ファイル取得
	//
	const ERIFile &	erif = eria.GetERIFile( ) ;
	ERIFile::ETagInfo	taginf ;
	const wchar_t *	pwszRefFile = NULL ;
	if ( erif.m_fdwReadMask & ERIFile::rmDescription )
	{
		taginf.CreateTagInfo( erif.m_wstrDescription ) ;
		m_ptHotSpot = taginf.GetHotSpot( ) ;
		m_nResolution = taginf.GetResolution( ) ;
		//
		pwszRefFile = taginf.GetTagContents( erif.tagReferenceFile ) ;
	}
	else
	{
		m_ptHotSpot.x = 0 ;
		m_ptHotSpot.y = 0 ;
		m_nResolution = 0 ;
	}
	//
	// シーケンステーブル設定
	//
	m_dwTotalTime = erif.m_FileHeader.dwAllFrameTime ;
	if ( erif.m_fdwReadMask & ERIFile::rmSequenceTable )
	{
		const ERIFile::SEQUENCE_DELTA *	pSequence ;
		DWORD	dwSeqLength ;
		pSequence = erif.GetSequenceTable( &dwSeqLength ) ;
		if ( pSequence != NULL )
		{
			for ( i = 0; i < (int) dwSeqLength; i ++ )
			{
				UINT	nFrame = pSequence[i].dwFrameIndex ;
				if ( nFrame >= m_lstFrames.GetSize() )
				{
					nFrame = 0 ;
				}
				for ( DWORD j = 0; j < pSequence[i].dwDuration; j ++ )
				{
					m_lstSequence.Add( nFrame ) ;
				}
			}
		}
	}
	else
	{
		m_lstSequence.SetLimit( m_lstFrames.GetSize() ) ;
		for ( i = 0; i < (int) m_lstFrames.GetSize(); i ++ )
		{
			m_lstSequence.Add( i ) ;
		}
	}
	//
	// 参照画像処理
	//
	if ( pwszRefFile != NULL )
	{
		ESLFileObject *	pRefFile =
			file.OpenFileObject
				( pwszRefFile,
					ESLFileObject::modeRead | ESLFileObject::shareRead ) ;
		if ( pRefFile != NULL )
		{
			EGLImage	imgRef ;
			if ( !imgRef.ReadImageFile( *pRefFile ) )
			{
				for ( i = 0; i < (int) m_lstFrames.GetSize(); i ++ )
				{
					PEGL_IMAGE_INFO	pImage = m_lstFrames.GetAt( i ) ;
					if ( pImage != NULL )
					{
						::eriLLAdditionOfFrame( *pImage, imgRef ) ;
					}
				}
			}
			delete	pRefFile ;
		}
	}
	//
	// 初期フレームを設定
	//
	return	SetCurrentSequence( 0 ) ;
}

// 画像バッファ消去
//////////////////////////////////////////////////////////////////////////////
void EGLAnimation::DeleteImage( void )
{
	for ( unsigned int i = 0; i < m_lstFrames.GetSize(); i ++ )
	{
		::eglDeleteImageBuffer( m_lstFrames.GetAt( i ) ) ;
	}
	m_lstSequence.RemoveAll( ) ;
	m_lstFrames.RemoveAll( ) ;
	//
	m_dwTotalTime = 0 ;
	m_dwCurrent = 0 ;
	m_ptHotSpot.x = 0 ;
	m_ptHotSpot.y = 0 ;
	m_nResolution = 0 ;
	//
	m_wstrReferenceFile.FreeString( ) ;
	//
	EGLImage::DeleteImage( ) ;
}

// 画像ファイルへ書き出す
//////////////////////////////////////////////////////////////////////////////
ESLError EGLAnimation::WriteImageFile
	( ESLFileObject & file,
		EGLImage::CompressTypeFlag ctfType,
		DWORD dwFlags, const ERISAEncoder::PARAMETER * periep )
{
	unsigned int	i, nCount ;
	if ( m_lstFrames.GetSize() == 0 )
	{
		return	eslErrGeneral ;
	}
	//
	// ファイルを開く
	//
	ESLError	err ;
	ERIAnimationWriter	eriaw ;
	err = eriaw.Open( &file, eriaw.fidImage ) ;
	if ( err )
		return	err ;
	//
	// パラメータを設定する
	//
	ERI_INFO_HEADER	eih ;
	::eslFillMemory( &eih, 0, sizeof(eih) ) ;
	eih.dwVersion =
		(ctfType == ctfCompatibleFormat) ? 0x00020100 : 0x00020200 ;
	eih.fdwTransformation = CVTYPE_LOSSLESS_ERI ;
	if ( ctfType == ctfCompatibleFormat )
		eih.dwArchitecture = ERI_RUNLENGTH_GAMMA ;
	else if ( ctfType == ctfExtendedFormat )
		eih.dwArchitecture = ERI_RUNLENGTH_HUFFMAN ;
	else
		eih.dwArchitecture = ERISA_NEMESIS_CODE ;
	//
	eih.fdwFormatType = m_pImage->fdwFormatType ;
	eih.nImageWidth = (SDWORD) m_pImage->dwImageWidth ;
	eih.nImageHeight = (SDWORD) m_pImage->dwImageHeight ;
	if ( dwFlags & ERISAEncoder::efTopDown )
		eih.nImageHeight = - eih.nImageHeight ;
	eih.dwBitsPerPixel = m_pImage->dwBitsPerPixel ;
	eih.dwClippedPixel = m_pImage->dwClippedPixel ;
	eih.dwBlockingDegree = 3 ;
	//
	if ( periep != NULL )
	{
		eih.fdwTransformation = CVTYPE_DCT_ERI ;
		eih.dwArchitecture = ERI_RUNLENGTH_GAMMA ;
		eih.dwSamplingFlags = ERISF_YUV_4_1_1 ;
		eriaw.SetImageCompressionParameter( *periep ) ;
	}
	//
	// ファイルヘッダを書き出す
	//
	err = eriaw.BeginFileHeader( 0, 0 ) ;
	if ( err )
		return	err ;
	err = eriaw.WriteEriInfoHeader( eih ) ;
	if ( err )
		return	err ;
	//
	ERIFile::ETagInfo	taginf ;
	taginf.AddTag
		( ERIFile::tagHotSpot,
			EWideString((int)m_ptHotSpot.x)
				+ L"," + EWideString((int)m_ptHotSpot.y) ) ;
	taginf.AddTag
		( ERIFile::tagResolution, EWideString( (double) m_nResolution / 100.0 ) ) ;
	if ( !m_wstrReferenceFile.IsEmpty() )
	{
		taginf.AddTag
			( ERIFile::tagReferenceFile, m_wstrReferenceFile ) ;
	}
	EWideString	wstrDesc ;
	taginf.FormatDescription( wstrDesc ) ;
	err = eriaw.WriteDescription
		( wstrDesc.CharPtr(), wstrDesc.GetLength() * sizeof(wchar_t) ) ;
	if ( err )
		return	err ;
	//
	ERIFile::SEQUENCE_DELTA *	pSeqTable ;
	DWORD	dwLength = (DWORD) -1 ;
	UINT	nLastFrame = (UINT) -1 ;
	nCount = m_lstSequence.GetSize( ) ;
	pSeqTable =
		(ERIFile::SEQUENCE_DELTA*) ::eslHeapAllocate
			( NULL, nCount * sizeof(ERIFile::SEQUENCE_DELTA), 0 ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		if ( (dwLength == (DWORD) -1) || (m_lstSequence[i] != nLastFrame) )
		{
			nLastFrame = m_lstSequence[i] ;
			dwLength ++ ;
			pSeqTable[dwLength].dwFrameIndex = nLastFrame ;
			pSeqTable[dwLength].dwDuration = 1 ;
		}
		else
		{
			pSeqTable[dwLength].dwDuration ++ ;
		}
	}
	err = eriaw.WriteSequenceTable( pSeqTable, dwLength + 1 ) ;
	::eslHeapFree( NULL, pSeqTable ) ;
	if ( err )
		return	err ;
	//
	eriaw.EndFileHeader( ) ;
	//
	// 画像データを書き出す
	//
	err = eriaw.BeginStream( ) ;
	if ( err )
		return	err ;
	//
	nCount = m_lstFrames.GetSize( ) ;
	for ( i = 0; i < nCount; i ++ )
	{
		PEGL_IMAGE_INFO	pImage = m_lstFrames.GetAt( i ) ;
		ESLAssert( pImage != NULL ) ;
		if ( pImage == NULL )
			return	eslErrGeneral ;
		//
		if ( (pImage->fdwFormatType & EIF_WITH_PALETTE)
			&& (pImage->pPaletteEntries != NULL) )
		{
			err = eriaw.WritePaletteTable
				( pImage->pPaletteEntries, pImage->dwPaletteCount ) ;
			if ( err )
				return	err ;
		}
		if ( !(dwFlags & ERISAEncoder::efTopDown) )
			::eglReverseVertically( pImage ) ;
		err = eriaw.WriteImageData( *pImage, dwFlags ) ;
		if ( !(dwFlags & ERISAEncoder::efTopDown) )
			::eglReverseVertically( pImage ) ;
		if ( err )
			return	err ;
	}
	err = eriaw.EndStream( m_dwTotalTime ) ;
	if ( err )
		return	err ;
	//
	// 完了
	//
	eriaw.Close( ) ;
	return	eslErrSuccess ;
}

// フレームを削除
//////////////////////////////////////////////////////////////////////////////
void EGLAnimation::RemoveFrameAt( int iFrame )
{
	::eglDeleteImageBuffer( m_lstFrames.GetAt( iFrame ) ) ;
	m_lstFrames.RemoveAt( iFrame ) ;
}

// フレームを追加
//////////////////////////////////////////////////////////////////////////////
void EGLAnimation::AddFrame( PEGL_IMAGE_INFO pImage )
{
	m_lstFrames.Add( pImage ) ;
}

// フレームを挿入
//////////////////////////////////////////////////////////////////////////////
void EGLAnimation::InsertFrame( int iFrame, PEGL_IMAGE_INFO pImage )
{
	if ( iFrame < 0 )
		iFrame = 0 ;
	if ( iFrame > (int) m_lstFrames.GetSize() )
		iFrame = m_lstFrames.GetSize() - 1 ;
	//
	m_lstFrames.InsertAt( iFrame, pImage ) ;
}

// 時間からシーケンス番号に変換
//////////////////////////////////////////////////////////////////////////////
DWORD EGLAnimation::TimeToSequence( DWORD dwMilliSec ) const
{
	if ( m_dwTotalTime == 0 )
		return	0 ;
	//
	return	(DWORD) ((UINT64)
				dwMilliSec * GetSequenceLength() / m_dwTotalTime) ;
}

// シーケンス番号から時間に変換
//////////////////////////////////////////////////////////////////////////////
DWORD EGLAnimation::SequenceToTime( DWORD dwSequence ) const
{
	if ( GetSequenceLength() == 0 )
	{
		return	0 ;
	}
	return	(DWORD) ((UINT64)
				dwSequence * m_dwTotalTime / GetSequenceLength()) ;
}

// シーケンス番号からフレーム番号に変換
//////////////////////////////////////////////////////////////////////////////
DWORD EGLAnimation::SequenceToFrame( DWORD dwSequence ) const
{
	DWORD	dwSeqLength = GetSequenceLength( ) ;
	if ( dwSeqLength == 0 )
		return	0 ;
	//
	if ( dwSequence >= dwSeqLength )
		dwSequence %= dwSeqLength ;
	//
	return	m_lstSequence.GetAt( dwSequence ) ;
}

// 指定フレームの画像を取得
//////////////////////////////////////////////////////////////////////////////
PEGL_IMAGE_INFO EGLAnimation::GetFrameAt( DWORD dwFrame )
{
	return	m_lstFrames.GetAt( dwFrame ) ;
}

// 現在のシーケンス画像を設定する
//////////////////////////////////////////////////////////////////////////////
ESLError EGLAnimation::SetCurrentSequence( DWORD dwSequence )
{
	m_dwCurrent = dwSequence ;
	PEGL_IMAGE_INFO	pFrame =
		m_lstFrames.GetAt( SequenceToFrame( dwSequence ) ) ;
	if ( pFrame == NULL )
		return	eslErrGeneral ;
	AttachImage( pFrame ) ;
	return	eslErrSuccess ;
}


//////////////////////////////////////////////////////////////////////////////
// 画像メディア読み込みクラス
//////////////////////////////////////////////////////////////////////////////

#include <vfw.h>
#include <objbase.h>
#include <gdiplus.h>
#include <psdfile.h>

//GetEncoderParameterList
// GDI+ 関数インターフェース
//////////////////////////////////////////////////////////////////////////////
typedef	Gdiplus::Status 
	(WINAPI *PGDIP_GdiplusStartup)
		( OUT ULONG_PTR * token,
			const Gdiplus::GdiplusStartupInput * input,
			OUT Gdiplus::GdiplusStartupOutput * output ) ;
typedef	VOID (WINAPI *PGDIP_GdiplusShutdown)( ULONG_PTR token ) ;
typedef	Gdiplus::GpStatus
	(WINGDIPAPI *PGDIP_GdipCreateBitmapFromStream)
		( IStream * stream, Gdiplus::GpBitmap ** bitmap ) ;
typedef	Gdiplus::GpStatus
	(WINGDIPAPI *PGDIP_GdipCreateBitmapFromScan0)
		( INT width, INT height, INT stride,
			Gdiplus::PixelFormat format,
			BYTE * scan0, Gdiplus::GpBitmap** bitmap ) ;
typedef	Gdiplus::GpStatus
	(WINGDIPAPI *PGDIP_GdipCreateBitmapFromFile)
		( GDIPCONST WCHAR * filename, Gdiplus::GpBitmap ** bitmap) ;
typedef	Gdiplus::GpStatus
	(WINGDIPAPI *PGDIP_GdipDisposeImage)( Gdiplus::GpImage * image ) ;
typedef	Gdiplus::GpStatus
	(WINGDIPAPI *PGDIP_GdipGetImageFlags)
		( Gdiplus::GpImage * image, UINT * flags ) ;
typedef	Gdiplus::GpStatus
	(WINGDIPAPI *PGDIP_GdipImageGetFrameCount)
		( Gdiplus::GpImage * image,
			GDIPCONST GUID * dimensionID, UINT * count ) ;
typedef	Gdiplus::GpStatus
	(WINGDIPAPI *PGDIP_GdipImageSelectActiveFrame)
		( Gdiplus::GpImage * image,
			GDIPCONST GUID * dimensionID, UINT frameIndex ) ;
typedef	Gdiplus::GpStatus
	(WINGDIPAPI *PGDIP_GdipGetImagePixelFormat)
		( Gdiplus::GpImage * image, Gdiplus::PixelFormat * format ) ;
typedef Gdiplus::GpStatus
	(WINGDIPAPI *PGDIP_GdipGetImageThumbnail)
		( Gdiplus::GpImage * image, UINT thumbWidth, UINT thumbHeight,
			Gdiplus::GpImage **thumbImage,
			Gdiplus::GetThumbnailImageAbort callback, VOID * callbackData ) ;
typedef	Gdiplus::GpStatus
	(WINGDIPAPI *PGDIP_GdipGetImagePaletteSize)
		( Gdiplus::GpImage * image, INT * size ) ;
typedef	Gdiplus::GpStatus
	(WINGDIPAPI *PGDIP_GdipGetImagePalette)
		( Gdiplus::GpImage * image, Gdiplus::ColorPalette * palette, INT size ) ;
typedef	Gdiplus::GpStatus
	(WINGDIPAPI *PGDIP_GdipSetImagePalette)
		( Gdiplus::GpImage * image, GDIPCONST Gdiplus::ColorPalette * palette) ;
typedef	Gdiplus::GpStatus
	(WINGDIPAPI *PGDIP_GdipGetImageWidth)
		( Gdiplus::GpImage * image, UINT * width ) ;
typedef	Gdiplus::GpStatus
	(WINGDIPAPI *PGDIP_GdipGetImageHeight)
		( Gdiplus::GpImage * image, UINT * height ) ;
typedef	Gdiplus::GpStatus
	(WINGDIPAPI *PGDIP_GdipBitmapLockBits)
		( Gdiplus::GpBitmap * bitmap,
			GDIPCONST Gdiplus::GpRect * rect,
			UINT flags, Gdiplus::PixelFormat format,
			Gdiplus::BitmapData * lockedBitmapData ) ;
typedef	Gdiplus::GpStatus
	(WINGDIPAPI *PGDIP_GdipBitmapUnlockBits)
		( Gdiplus::GpBitmap * bitmap, Gdiplus::BitmapData * lockedBitmapData ) ;
typedef	Gdiplus::GpStatus
	(WINGDIPAPI *PDGIP_GdipSaveImageToFile)
		( Gdiplus::GpImage * image,
			GDIPCONST WCHAR * filename,
			GDIPCONST CLSID * clsidEncoder,
			GDIPCONST Gdiplus::EncoderParameters * encoderParams ) ;
typedef	Gdiplus::GpStatus
	(WINGDIPAPI *PDGIP_GdipSaveImageToStream)
		( Gdiplus::GpImage * image, IStream * stream,
			GDIPCONST CLSID * clsidEncoder, 
			GDIPCONST Gdiplus::EncoderParameters * encoderParams ) ;
typedef	Gdiplus::GpStatus
	(WINGDIPAPI *PGDIP_GdipGetImageDecodersSize)
		( UINT * numDecoders, UINT * size ) ;
typedef	Gdiplus::GpStatus
	(WINGDIPAPI *PGDIP_GdipGetImageDecoders)
		( UINT numDecoders, UINT size, Gdiplus::ImageCodecInfo * decoders ) ;
typedef	Gdiplus::GpStatus
	(WINGDIPAPI *PGDIP_GdipGetImageEncodersSize)
		( UINT * numEncoders, UINT * size) ;
typedef	Gdiplus::GpStatus
	(WINGDIPAPI *PGDIP_GdipGetImageEncoders)
		( UINT numEncoders, UINT size, Gdiplus::ImageCodecInfo * encoders ) ;

// 共用リソース
//////////////////////////////////////////////////////////////////////////////
static long int	g_nRefMediaLoader = 0 ;		// 参照回数
static HMODULE	g_hGDIPlus = NULL ;			// GDI+ モジュール

// GDI+ 用パラメータ
static ULONG_PTR						g_gdiplusToken ;

// GDI+ インターフェース
static PGDIP_GdiplusStartup				g_pfnGdiplusStartup = NULL ;
static PGDIP_GdiplusShutdown			g_pfnGdiplusShutdown = NULL ;
static PGDIP_GdipCreateBitmapFromStream	g_pfnCreateBitmapFromStream = NULL ;
static PGDIP_GdipCreateBitmapFromFile	g_pfnCreateBitmapFromFile = NULL ;
static PGDIP_GdipCreateBitmapFromScan0	g_pfnCreateBitmapFromScan0 = NULL ;
static PGDIP_GdipDisposeImage			g_pfnDisposeImage = NULL ;
static PGDIP_GdipGetImageFlags			g_pfnGetImageFlags = NULL ;
static PGDIP_GdipImageGetFrameCount		g_pfnImageGetFrameCount = NULL ;
static PGDIP_GdipImageSelectActiveFrame	g_pfnImageSelectActiveFrame = NULL ;
static PGDIP_GdipGetImagePixelFormat	g_pfnGetImagePixelFormat = NULL ;
static PGDIP_GdipGetImageThumbnail		g_pfnGetImageThumbnail = NULL ;
static PGDIP_GdipGetImagePaletteSize	g_pfnGetImagePaletteSize = NULL ;
static PGDIP_GdipGetImagePalette		g_pfnGetImagePalette = NULL ;
static PGDIP_GdipSetImagePalette		g_pfnSetImagePalette = NULL ;
static PGDIP_GdipGetImageWidth			g_pfnGetImageWidth = NULL ;
static PGDIP_GdipGetImageHeight			g_pfnGetImageHeight = NULL ;
static PGDIP_GdipBitmapLockBits			g_pfnBitmapLockBits = NULL ;
static PGDIP_GdipBitmapUnlockBits		g_pfnBitmapUnlockBits = NULL ;
static PDGIP_GdipSaveImageToFile		g_pfnSaveImageToFile = NULL ;
static PDGIP_GdipSaveImageToStream		g_pfnSaveImageToStream = NULL ;
static PGDIP_GdipGetImageDecodersSize	g_pfnGetImageDecodersSize = NULL ;
static PGDIP_GdipGetImageDecoders		g_pfnGetImageDecoders = NULL ;
static PGDIP_GdipGetImageEncodersSize	g_pfnGetImageEncodersSize = NULL ;
static PGDIP_GdipGetImageEncoders		g_pfnGetImageEncoders = NULL ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( EGLMediaLoader, EGLAnimation )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
EGLMediaLoader::EGLMediaLoader( void )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
EGLMediaLoader::~EGLMediaLoader( void )
{
}

// 画像ファイルを読み込む
//////////////////////////////////////////////////////////////////////////////
ESLError EGLMediaLoader::LoadMediaFile
	( const char * pszFileName,
		DWORD fdwFormat, DWORD dwBitsPerPixel,
		DWORD dwLimitFrames, DWORD dwLimitSize )
{
	//
	// 現在のデータを削除する
	//
	DeleteImage( ) ;
	//
	// 拡張子から判定
	//
	ESLError	err ;
	ERawFile	file ;
	EString	strFileName = pszFileName ;
	EString	strFileExt = strFileName.GetFileExtensionPart( ) ;
	if ( !strFileExt.CompareNoCase( "bmp" ) )
	{
		//
		// Windows Bitmap ファイルか？
		//
		err = file.Open( strFileName, file.modeRead | file.shareRead ) ;
		if ( err )
		{
			return	err ;
		}
		err = ReadBitmapFile( file, fdwFormat, dwBitsPerPixel ) ;
		file.Close( ) ;
		if ( err == eslErrSuccess )
		{
			return	eslErrSuccess ;
		}
	}
	else if ( !strFileExt.CompareNoCase( "avi" ) )
	{
		//
		// Windows AVI ファイルか？
		//
		if ( !LoadAviFile
				( strFileName, fdwFormat, dwBitsPerPixel,
							dwLimitFrames, dwLimitSize ) )
		{
			return	eslErrSuccess ;
		}
	}
	else if ( !strFileExt.CompareNoCase( "psd" ) )
	{
		//
		// Photoshop PSD ファイルか？
		//
		err = file.Open( strFileName, file.modeRead | file.shareRead ) ;
		if ( err )
		{
			return	err ;
		}
		err = ReadPhotoshopPSDFile
				( file, fdwFormat, dwBitsPerPixel,
							dwLimitFrames, dwLimitSize ) ;
		file.Close( ) ;
		if ( err == eslErrSuccess )
		{
			return	eslErrSuccess ;
		}
	}
	//
	// ERI ファイルか判定
	//
	err = file.Open( strFileName, file.modeRead | file.shareRead ) ;
	if ( err )
	{
		return	err ;
	}
	err = EGLAnimation::ReadImageFile( file ) ;
	file.Close( ) ;
	if ( err == eslErrSuccess )
	{
		if ( fdwFormat == EIF_WITH_ALPHA )
		{
			fdwFormat = 0 ;
			dwBitsPerPixel = 0 ;
		}
		return	ConvertFormatTo( fdwFormat, dwBitsPerPixel ) ;
	}
	//
	// GDI+ を利用して読み込む
	//
	return	LoadWithGDIplus( strFileName, fdwFormat, dwBitsPerPixel ) ;
}

// 画像ファイルを読み込む
//////////////////////////////////////////////////////////////////////////////
ESLError EGLMediaLoader::ReadMediaFile
	( ESLFileObject & file,
		DWORD fdwFormat, DWORD dwBitsPerPixel )
{
	ESLError	err ;
	DWORD	dwFilePos = file.GetPosition( ) ;
	err = EGLAnimation::ReadImageFile( file ) ;
	if ( err == eslErrSuccess )
	{
		if ( fdwFormat == EIF_WITH_ALPHA )
		{
			fdwFormat = 0 ;
			dwBitsPerPixel = 0 ;
		}
		return	ConvertFormatTo( fdwFormat, dwBitsPerPixel ) ;
	}
	//
	file.Seek( dwFilePos, file.FromBegin ) ;
	err = ReadBitmapFile( file, fdwFormat, dwBitsPerPixel ) ;
	if ( err == eslErrSuccess )
	{
		return	eslErrSuccess ;
	}
	//
	file.Seek( dwFilePos, file.FromBegin ) ;
	err = ReadPhotoshopPSDFile( file, fdwFormat, dwBitsPerPixel ) ;
	if ( err == eslErrSuccess )
	{
		return	eslErrSuccess ;
	}
	//
	file.Seek( dwFilePos, file.FromBegin ) ;
	return	ReadWithGDIplus( file, fdwFormat, dwBitsPerPixel ) ;
}

// GDI+ サムネイルコールバック関数
//////////////////////////////////////////////////////////////////////////////
struct	GDIP_THUMBNAIL_DATA
{
	EGLImage *								pImage ;
	EGLMediaLoader::PFUNC_CALLBACK_DECODE	pfnCallback ;
	void *									ptrData ;
} ;

static BOOL CALLBACK gdipThumbnailCallbackFunc( void * pInstance )
{
	GDIP_THUMBNAIL_DATA *
		pThumbData = (GDIP_THUMBNAIL_DATA*) pInstance ;
	if ( pThumbData->pfnCallback
		( pThumbData->pImage, pThumbData->ptrData, 0, 1 ) )
	{
		return	TRUE ;
	}
	return	FALSE ;
}

// プレビュー画像を読み込む
//////////////////////////////////////////////////////////////////////////////
ESLError EGLMediaLoader::LoadPreviewImage
	( const char * pszFileName,
		DWORD dwWidth, DWORD dwHeight,
		DWORD dwPreviewFlags, EGL_SIZE * pOriginalSize,
		EGLMediaLoader::PFUNC_CALLBACK_DECODE pfnCallback,
		void * ptrCallbackData )
{
	//
	// 現在のデータを削除する
	//
	DeleteImage( ) ;
	//
	// ERI 形式の判定
	//
	ERawFile	file ;
	if ( !file.Open( pszFileName, file.modeRead | file.shareRead ) )
	{
		ERIFile	erif ;
		if ( !erif.Open( &file ) )
		{
			erif.Close( ) ;
			file.Seek( 0, file.FromBegin ) ;
			//
			return	EGLImage::ReadPreviewImage
				( file, dwWidth, dwHeight, dwPreviewFlags,
					pOriginalSize, pfnCallback, ptrCallbackData ) ;
		}
	}
	//
	// GDI+ を使った読み込み
	//
	ESLError			err = eslErrSuccess ;
	Gdiplus::GpBitmap * pbitmap = NULL ;
	Gdiplus::GpBitmap * pthumbnail = NULL ;
	//
	Initialize( ) ;
	do
	{
		//
		// ファイルを開く
		//
		EWideString	wstrFileName = pszFileName ;
		if ( (g_pfnCreateBitmapFromFile == NULL)
			|| g_pfnCreateBitmapFromFile( wstrFileName, &pbitmap ) )
		{
			err = eslErrGeneral ;
			break ;
		}
		//
		// 画像サイズ取得
		//
		UINT	nWidth, nHeight ;
		if ( (g_pfnGetImageWidth == NULL)
			|| g_pfnGetImageWidth( pbitmap, &nWidth ) )
		{
			break ;
		}
		if ( (g_pfnGetImageHeight == NULL)
			|| g_pfnGetImageHeight( pbitmap, &nHeight ) )
		{
			break ;
		}
		if ( pOriginalSize != NULL )
		{
			pOriginalSize->w = nWidth ;
			pOriginalSize->h = nHeight ;
		}
		double		rWidthRate, rHeightRate, rRate ;
		EGL_SIZE	sizeThumbnail ;
		rWidthRate = (double) (int) dwWidth / (int) nWidth ;
		rHeightRate = (double) (int) dwHeight / (int) nHeight ;
		if ( rWidthRate <= rHeightRate )
		{
			rRate = rWidthRate ;
		}
		else
		{
			rRate = rHeightRate ;
		}
		sizeThumbnail.w = (int) ::eriRoundR64ToLInt( rRate * (int) nWidth ) ;
		sizeThumbnail.h = (int) ::eriRoundR64ToLInt( rRate * (int) nHeight ) ;
		//
		// サムネイルを作成
		//
		GDIP_THUMBNAIL_DATA				gtd ;
		Gdiplus::GetThumbnailImageAbort	callbackThumb = NULL ;
		void *							callbackData = NULL ;
		gtd.pImage = this ;
		gtd.pfnCallback = pfnCallback ;
		gtd.ptrData = ptrCallbackData ;
		if ( pfnCallback != NULL )
		{
			callbackThumb = &gdipThumbnailCallbackFunc ;
			callbackData = &gtd ;
		}
		if ( (g_pfnGetImageThumbnail == NULL)
			|| g_pfnGetImageThumbnail
				( pbitmap, sizeThumbnail.w, sizeThumbnail.h,
					(Gdiplus::GpImage**) &pthumbnail,
					callbackThumb, callbackData ) )
		{
			err = eslErrGeneral ;
			break ;
		}
		err = ConvertFromGDIplus( pthumbnail ) ;
	}
	while ( false ) ;
	//
	if ( pthumbnail != NULL )
	{
		ESLAssert( g_pfnDisposeImage != NULL ) ;
		if ( g_pfnDisposeImage != NULL )
		{
			g_pfnDisposeImage( pbitmap ) ;
		}
	}
	if ( pbitmap != NULL )
	{
		ESLAssert( g_pfnDisposeImage != NULL ) ;
		if ( g_pfnDisposeImage != NULL )
		{
			g_pfnDisposeImage( pbitmap ) ;
		}
	}
	//
	Close( ) ;
	//
	return	err ;
}

// Window Bitmap ファイルを読み込む
//////////////////////////////////////////////////////////////////////////////
ESLError EGLMediaLoader::ReadBitmapFile
	( ESLFileObject & file,
		DWORD fdwFormat, DWORD dwBitsPerPixel )
{
	//
	// 現在のデータを削除する
	//
	DeleteImage( ) ;
	//
	// ファイルヘッダ読み込み
	//
	BITMAPFILEHEADER	bmfh ;
	BITMAPINFOHEADER	bmih ;
	if ( file.Read( &bmfh, sizeof(bmfh) ) < sizeof(bmfh) )
	{
		return	eslErrGeneral ;
	}
	if ( (bmfh.bfType != 'MB') || (bmfh.bfReserved1 != 0)
		|| (bmfh.bfReserved2 != 0)
		|| (bmfh.bfOffBits < sizeof(bmfh) + sizeof(bmih)) )
	{
		return	eslErrGeneral ;
	}
	if ( file.Read( &bmih, sizeof(bmih) ) < sizeof(bmih) )
	{
		return	eslErrGeneral ;
	}
	if ( (bmih.biSize != sizeof(bmih)) || (bmih.biCompression != BI_RGB) )
	{
		return	eslErrGeneral ;
	}
	if ( (bmih.biBitCount != 1) && (bmih.biBitCount != 4)
		&& (bmih.biBitCount != 8) && (bmih.biBitCount != 16)
		&& (bmih.biBitCount != 24) && (bmih.biBitCount != 32) )
	{
		return	eslErrGeneral ;
	}
	//
	// パレットテーブル読み込み
	//
	RGBQUAD	ptbl[0x100] ;
	int	nPalLength = 0 ;
	if ( bmih.biBitCount <= 8 )
	{
		nPalLength = 1 << bmih.biBitCount ;
		if ( bmih.biClrUsed != 0 )
		{
			if ( (DWORD) nPalLength < bmih.biClrUsed )
			{
				return	eslErrGeneral ;
			}
			nPalLength = bmih.biClrUsed ;
		}
		if ( file.Read
			( ptbl, sizeof(RGBQUAD) * nPalLength )
						< sizeof(RGBQUAD) * nPalLength )
		{
			return	eslErrGeneral ;
		}
	}
	//
	// 画像バッファ作成
	//
	DWORD	dwLineBytes =
		((bmih.biWidth * bmih.biBitCount + 0x1F) & ~0x1F) >> 3 ;
	PEGL_IMAGE_INFO	pImage ;
	pImage = ::eglCreateImageBuffer
		( EIF_RGB_BITMAP | (nPalLength ? EIF_WITH_PALETTE : 0),
					bmih.biWidth, bmih.biHeight, bmih.biBitCount ) ;
	if ( pImage == NULL )
	{
		return	eslErrGeneral ;
	}
	if ( pImage->pPaletteEntries != NULL )
	{
		if ( nPalLength > (int) pImage->dwPaletteCount )
		{
			nPalLength = (int) pImage->dwPaletteCount ;
		}
		::eslMoveMemory
			( pImage->pPaletteEntries,
				ptbl, nPalLength * sizeof(EGL_PALETTE) ) ;
		if ( nPalLength == 0x100 )
		{
			int		i ;
			for ( i = 0; i < 0x100; i ++ )
			{
				if ( (ptbl[i].rgbBlue != i)
					|| (ptbl[i].rgbGreen != i)
					|| (ptbl[i].rgbRed != i) )
				{
					break ;
				}
			}
			if ( i >= 0x100 )
			{
				pImage->fdwFormatType = EIF_GRAY_BITMAP ;
			}
		}
	}
	m_lstFrames.Add( pImage ) ;
	m_lstSequence.Add( 0 ) ;
	AttachImage( pImage ) ;
	//
	file.Seek( bmfh.bfOffBits, ESLFileObject::FromBegin ) ;
	PBYTE	pbytBuf = (PBYTE) pImage->ptrImageArray ;
	for ( DWORD y = 0; y < pImage->dwImageHeight; y ++ )
	{
		if ( file.Read( pbytBuf, dwLineBytes ) < dwLineBytes )
			break ;
		pbytBuf += pImage->dwBytesPerLine ;
	}
	::eglReverseVertically( pImage ) ;
	//
	return	ConvertFormatTo( fdwFormat, dwBitsPerPixel ) ;
}

// Photoshop PSD ファイルを読み込む
//////////////////////////////////////////////////////////////////////////////
ESLError EGLMediaLoader::ReadPhotoshopPSDFile
	( ESLFileObject & file,
		DWORD fdwFormat, DWORD dwBitsPerPixel,
		DWORD dwLimitFrames, DWORD dwLimitSize )
{
	ESLError		err = eslErrSuccess ;
	PSD::File		psdf ;
	err = psdf.Open( file ) ;
	if ( !err )
	{
		DeleteImage( ) ;
		//
		EGLImage		imgLayer, imgBase ;
		PEGL_IMAGE_INFO	pImage ;
		unsigned int	i, nCount ;
		HEGL_DRAW_IMAGE	hDraw = NULL ;
		nCount = psdf.GetLayerCount( ) ;
		if ( (dwLimitFrames == (DWORD) -1) || (nCount == 0) )
		{
			//
			// ベース画像を読み込む
			//
			err = psdf.LoadBaseImage( imgLayer ) ;
			if ( !err )
			{
				pImage = imgLayer ;
				::eglAddImageBufferRef( pImage ) ;
				AddFrame( pImage ) ;
			}
		}
		else
		{
			//
			// 指定された数だけレイヤーを読み込む
			// 或いは、レイヤーを合成する
			//
			EGL_DRAW_PARAM	dp ;
			::eslFillMemory( &dp, 0, sizeof(dp) ) ;
			hDraw = ::eglCreateDrawImage( ) ;
			//
			PSD::FILE_HEADER	filehdr ;
			psdf.GetFileHeader( filehdr ) ;
			for ( i = 0; i < nCount; i ++ )
			{
				if ( (dwLimitFrames != 0) && (i >= dwLimitFrames) )
				{
					break ;
				}
				PSD::File::LayerInfo	liLayer ;
				err = psdf.GetLayerInfo( liLayer, i ) ;
				if ( err )
				{
					break ;
				}
				err = psdf.LoadLayerImage( imgLayer, i ) ;
				if ( err )
				{
					break ;
				}
				dp.dwFlags = EGL_DRAW_BLEND_ALPHA | EGL_APPLY_A_MUL ;
				dp.ptBasePos.x = liLayer.irRect.x ;
				dp.ptBasePos.y = liLayer.irRect.y ;
				dp.pSrcImage = imgLayer ;
				//
				if ( dwLimitFrames != 0 )
				{
					//
					// レイヤー画像を追加
					//
					DWORD	dwSizeOfImage ;
					imgBase.CreateImage
						( imgLayer.GetFormatType(),
							filehdr.dwWidth, filehdr.dwHeight, 32 ) ;
					pImage = imgBase ;
					dwSizeOfImage = pImage->dwSizeOfImage ;
					imgBase.ReverseVertically( ) ;
					//
					hDraw->Initialize( imgBase, NULL, NULL ) ;
					if ( !hDraw->PrepareDraw( &dp ) )
					{
						hDraw->DrawImage( ) ;
					}
					pImage = imgBase ;
					::eglAddImageBufferRef( pImage ) ;
					AddFrame( pImage ) ;
					//
					if ( (dwLimitSize != 0) && (dwLimitSize <= dwSizeOfImage) )
					{
						break ;
					}
				}
				else
				{
					//
					// レイヤー画像を合成
					//
					if ( liLayer.dwBlendMode
							== PSD::dwswap(PSD::blendMultiply) )
					{
						dp.dwFlags |= EGL_DRAW_F_MUL ;
					}
					else if ( liLayer.dwBlendMode
								== PSD::dwswap(PSD::blendScreen) )
					{
						dp.dwFlags |= EGL_DRAW_F_ADD ;
					}
					else if ( liLayer.dwBlendMode
								== PSD::dwswap(PSD::blendDivision) )
					{
						dp.dwFlags |= EGL_DRAW_F_DIV ;
					}
					else if ( liLayer.dwBlendMode
								== PSD::dwswap(PSD::blendHardLight) )
					{
						dp.dwFlags |= EGL_DRAW_F_SUB ;
					}
					dp.nTransparency = liLayer.nTransparency ;
					//
					if ( imgBase.GetInfo() == NULL )
					{
/*						if ( nCount == 1 )
						{
							imgLayer.BlendAlphaChannel
								( NULL, NULL, EGL_BAC_MULTIPLY ) ;
							//
							pImage = imgLayer ;
							::eglAddImageBufferRef( pImage ) ;
							AddFrame( pImage ) ;
							//
							SetHotSpot
								( EGLPoint( - dp.ptBasePos.x, - dp.ptBasePos.y ) ) ;
							break ;
						}
						else
*/						{
							imgBase.CreateImage
								( imgLayer.GetFormatType(),
									filehdr.dwWidth, filehdr.dwHeight, 32 ) ;
							imgBase.ReverseVertically( ) ;
							pImage = imgBase ;
							::eglAddImageBufferRef( pImage ) ;
							AddFrame( pImage ) ;
						}
					}
					hDraw->Initialize( imgBase, NULL, NULL ) ;
					if ( !hDraw->PrepareDraw( &dp ) )
					{
						hDraw->DrawImage( ) ;
					}
				}
			}
		}
		if ( hDraw != NULL )
		{
			hDraw->Release( ) ;
		}
		for ( i = 0; i < GetTotalFrameCount(); i ++ )
		{
			m_lstSequence.Add( i ) ;
		}
		SetCurrentSequence( 0 ) ;
		if ( GetTotalFrameCount() == 0 )
		{
			err = eslErrGeneral ;
		}
		psdf.Close( ) ;
	}
	if ( err )
	{
		return	err ;
	}
	return	ConvertFormatTo( fdwFormat, dwBitsPerPixel ) ;
}

// AVI ファイルを読み込む
//////////////////////////////////////////////////////////////////////////////
ESLError EGLMediaLoader::LoadAviFile
	( const char * pszFileName,
		DWORD fdwFormat, DWORD dwBitsPerPixel,
		DWORD dwLimitFrames, DWORD dwLimitSize )
{
	ESLError		err = eslErrSuccess ;
	PAVIFILE		pAviFile = NULL ;
	PAVISTREAM		pAviStream = NULL ;
	PGETFRAME		pGetFrame = NULL ;
	AVIFILEINFO		afiFileInfo ;
	AVISTREAMINFO	asiStreamInfo ;
	EGL_IMAGE_INFO	eiiInfo ;
	//
	::eslFillMemory( &eiiInfo, 0, sizeof(eiiInfo) ) ;
//	::CoInitializeEx( NULL, COINIT_APARTMENTTHREADED ) ;
	::CoInitialize( NULL ) ;
	::AVIFileInit( ) ;
	do
	{
		//
		// 現在のデータを削除する
		//
		DeleteImage( ) ;
		//
		// AVI ファイルを開く
		//
		if ( ::AVIFileOpen( &pAviFile, pszFileName, OF_READ, NULL ) )
		{
			err = eslErrGeneral ;
			break ;
		}
		if ( ::AVIFileInfo( pAviFile, &afiFileInfo, sizeof(afiFileInfo) ) )
		{
			err = eslErrGeneral ;
			break ;
		}
		//
		// 映像ストリームを取得する
		//
		long int	i ;
		for ( i = 0; i < (long int) afiFileInfo.dwStreams; i ++ )
		{
			if ( ::AVIFileGetStream( pAviFile, &pAviStream, 0, i ) )
			{
				pAviStream = NULL ;
				continue ;
			}
			if ( !::AVIStreamInfo
				( pAviStream, &asiStreamInfo, sizeof(asiStreamInfo) ) )
			{
				if ( asiStreamInfo.fccType == streamtypeVIDEO )
				{
					break ;
				}
			}
			::AVIStreamRelease( pAviStream ) ;
			pAviStream = NULL ;
		}
		if ( pAviStream == NULL )
		{
			err = eslErrGeneral ;
			break ;
		}
		//
		// 映像展開の準備を行う
		//
		pGetFrame = ::AVIStreamGetFrameOpen( pAviStream, NULL ) ;
		if ( pGetFrame == NULL )
		{
			pGetFrame = ::AVIStreamGetFrameOpen
				( pAviStream, (LPBITMAPINFOHEADER) AVIGETFRAMEF_BESTDISPLAYFMT ) ;
			if ( pGetFrame == NULL )
			{
				err = eslErrGeneral ;
				break ;
			}
		}
		//
		// ストリームの情報を取得
		//
		long int	nLength = ::AVIStreamLength( pAviStream ) ;
		if ( dwLimitFrames != 0 )
		{
			if ( nLength > (long int) dwLimitFrames )
			{
				nLength = dwLimitFrames ;
			}
		}
		m_dwTotalTime = ::AVIStreamLengthTime( pAviStream ) ;
		//
		m_lstSequence.SetLimit( nLength ) ;
		m_lstFrames.SetLimit( nLength ) ;
		//
		for ( i = 0; i < nLength; i ++ )
		{
			//
			// フレーム情報取得
			//
			BITMAPINFOHEADER *	pbmih =
				(BITMAPINFOHEADER*) ::AVIStreamGetFrame( pGetFrame, i ) ;
			if ( pbmih == NULL )
			{
				err = eslErrGeneral ;
				break ;
			}
			//
			// EGL_IMAGE_INFO 構造体へ変換
			//
			int		nPalLength = 0 ;
			if ( pbmih->biBitCount <= 8 )
			{
				nPalLength = 1 << pbmih->biBitCount ;
				if ( (pbmih->biClrUsed != 0)
					&& ((DWORD) nPalLength >= pbmih->biClrUsed) )
				{
					nPalLength = pbmih->biClrUsed ;
				}
			}
			eiiInfo.dwInfoSize = sizeof(eiiInfo) ;
			eiiInfo.fdwFormatType = EIF_RGB_BITMAP ;
			if ( nPalLength != 0 )
			{
				eiiInfo.fdwFormatType |= EIF_WITH_PALETTE ;
				eiiInfo.pPaletteEntries =
					(PEGL_PALETTE) (((PBYTE)pbmih) + pbmih->biSize) ;
				eiiInfo.ptrImageArray = eiiInfo.pPaletteEntries + nPalLength ;
			}
			else
			{
				eiiInfo.ptrImageArray = ((PBYTE)pbmih) + pbmih->biSize ;
				eiiInfo.pPaletteEntries = NULL ;
			}
			eiiInfo.dwPaletteCount = nPalLength ;
			eiiInfo.dwImageWidth = pbmih->biWidth ;
			eiiInfo.dwImageHeight = pbmih->biHeight ;
			eiiInfo.dwBitsPerPixel = pbmih->biBitCount ;
			eiiInfo.dwBytesPerLine =
				((eiiInfo.dwImageWidth
					* eiiInfo.dwBitsPerPixel + 0x1F) & ~0x1F) >> 3 ;
			eiiInfo.dwSizeOfImage =
				eiiInfo.dwBytesPerLine * (SDWORD) eiiInfo.dwImageHeight ;
			//
			PEGL_IMAGE_INFO	pImage = ::eglDuplicateImageBuffer( &eiiInfo ) ;
			if ( pImage == NULL )
			{
				err = eslErrGeneral ;
				break ;
			}
			::eglReverseVertically( pImage ) ;
			//
			// 画像追加
			//
			m_lstFrames.Add( pImage ) ;
			m_lstSequence.Add( i ) ;
			//
			if ( dwLimitSize != 0 )
			{
				if ( eiiInfo.dwSizeOfImage >= (SDWORD) dwLimitSize )
				{
					break ;
				}
				dwLimitSize -= eiiInfo.dwSizeOfImage ;
			}
		}
	}
	while ( false ) ;
	//
	// 終了
	//
	if ( pGetFrame != NULL )
		::AVIStreamGetFrameClose( pGetFrame ) ;
	if ( pAviStream != NULL )
		::AVIStreamRelease( pAviStream ) ;
	if ( pAviFile != NULL )
		::AVIFileRelease( pAviFile ) ;
	//
	::AVIFileExit( ) ;
//	::CoUninitialize( ) ;
	if ( err )
	{
		return	err ;
	}
	SetCurrentSequence( 0 ) ;
	return	ConvertFormatTo( fdwFormat, dwBitsPerPixel ) ;
}

// GDI+ を利用して読み込む
//////////////////////////////////////////////////////////////////////////////
ESLError EGLMediaLoader::LoadWithGDIplus
	( const char * pszFileName,
		DWORD fdwFormat, DWORD dwBitsPerPixel )
{
	ESLError	err = eslErrSuccess ;
	Gdiplus::GpBitmap *	pbitmap = NULL ;
	//
	Initialize( ) ;
	do
	{
		//
		// 現在のデータを削除する
		//
		DeleteImage( ) ;
		//
		// ファイルを開く
		//
		EWideString	wstrFileName = pszFileName ;
		if ( (g_pfnCreateBitmapFromFile == NULL)
			|| g_pfnCreateBitmapFromFile( wstrFileName, &pbitmap ) )
		{
			err = eslErrGeneral ;
			break ;
		}
		//
		// EGL 画像オブジェクトへ変換
		//
		err = ConvertFromGDIplus( pbitmap, fdwFormat, dwBitsPerPixel ) ;
	}
	while ( false ) ;
	//
	// 終了
	//
	if ( pbitmap != NULL )
	{
		ESLAssert( g_pfnDisposeImage != NULL ) ;
		if ( g_pfnDisposeImage != NULL )
		{
			g_pfnDisposeImage( pbitmap ) ;
		}
	}
	Close( ) ;
	if ( err )
	{
		return	err ;
	}
	return	ConvertFormatTo( fdwFormat, dwBitsPerPixel ) ;
}

ESLError EGLMediaLoader::ReadWithGDIplus
	( ESLFileObject & file,
		DWORD fdwFormat, DWORD dwBitsPerPixel )
{
	ESLError	err = eslErrSuccess ;
	Gdiplus::GpBitmap *	pbitmap = NULL ;
	//
	Initialize( ) ;
	do
	{
		//
		// 現在のデータを削除する
		//
		DeleteImage( ) ;
		//
		// ファイルをメモリ上に読み込む
		//
		DWORD	dwFileSize = file.GetLength( ) ;
		HGLOBAL	hGlobal = ::GlobalAlloc( GMEM_MOVEABLE, dwFileSize ) ;
		if ( hGlobal == NULL )
		{
			err = eslErrGeneral ;
			break ;
		}
		LPVOID	lpBuffer = ::GlobalLock( hGlobal ) ;
		if ( lpBuffer == NULL )
		{
			::GlobalFree( hGlobal ) ;
			err = eslErrGeneral ;
			break ;
		}
		file.Read( lpBuffer, dwFileSize ) ;
		::GlobalUnlock( hGlobal ) ;
		//
		// IStream オブジェクトを生成する
		//
		LPSTREAM	stream = NULL;
		if ( ::CreateStreamOnHGlobal( hGlobal, TRUE, &stream ) != S_OK )
		{
			err = eslErrGeneral ;
			break ;
		}
		if ( stream == NULL )
		{
			err = eslErrGeneral ;
			break ;
		}
		//
		// 画像を読み込む
		//
		if ( (g_pfnCreateBitmapFromStream == NULL)
			|| g_pfnCreateBitmapFromStream( stream, &pbitmap ) )
		{
			stream->Release( ) ;
			err = eslErrGeneral ;
			break ;
		}
		stream->Release( ) ;
		//
		// EGL 画像オブジェクトへ変換
		//
		err = ConvertFromGDIplus( pbitmap, fdwFormat, dwBitsPerPixel ) ;
	}
	while ( false ) ;
	//
	// 終了
	//
	if ( pbitmap != NULL )
	{
		ESLAssert( g_pfnDisposeImage != NULL ) ;
		if ( g_pfnDisposeImage != NULL )
		{
			g_pfnDisposeImage( pbitmap ) ;
		}
	}
	Close( ) ;
	if ( err )
	{
		return	err ;
	}
	return	ConvertFormatTo( fdwFormat, dwBitsPerPixel ) ;
}

// GDI+ オブジェクトから EGL 画像オブジェクトへ変換
//////////////////////////////////////////////////////////////////////////////
static PEGL_IMAGE_INFO eglConvertFromGDIplus
	( Gdiplus::GpBitmap * pbitmap, DWORD fdwFormat, DWORD dwBitsPerPixel )
{
	//
	// ピクセルフォーマットを取得
	//
	Gdiplus::PixelFormat	pxfmt ;
	INT						nPaletteSize ;
	Gdiplus::ColorPalette *	pPalette = NULL ;
	if ( (g_pfnGetImagePixelFormat == NULL)
		|| g_pfnGetImagePixelFormat( pbitmap, &pxfmt ) )
	{
		return	NULL ;
	}
	//
	EGL_IMAGE_INFO	eiiInfo ;
	::eslFillMemory( &eiiInfo, 0, sizeof(eiiInfo) ) ;
	eiiInfo.dwInfoSize = sizeof(eiiInfo) ;
	if ( pxfmt & PixelFormatAlpha )
	{
		//
		// RGBA フォーマット
		//
		pxfmt = PixelFormat32bppPARGB ;
		eiiInfo.fdwFormatType = EIF_RGBA_BITMAP ;
		eiiInfo.dwBitsPerPixel = 32 ;
	}
	else if ( pxfmt & PixelFormatIndexed )
	{
		//
		// 256色 フォーマット
		//
		pxfmt = PixelFormat8bppIndexed ;
		eiiInfo.fdwFormatType = EIF_RGB_BITMAP | EIF_WITH_PALETTE ;
		eiiInfo.dwBitsPerPixel = 8 ;
		//
		// パレットテーブルを取得する
		//
		if ( (g_pfnGetImagePaletteSize == NULL)
			|| g_pfnGetImagePaletteSize( pbitmap, &nPaletteSize ) )
		{
			return	NULL ;
		}
		pPalette = (Gdiplus::ColorPalette*)
			::eslHeapAllocate( NULL, nPaletteSize, 0 ) ;
		if ( (g_pfnGetImagePalette == NULL)
			|| g_pfnGetImagePalette( pbitmap, pPalette, nPaletteSize ) )
		{
			::eslHeapFree( NULL, pPalette ) ;
			return	NULL ;
		}
		eiiInfo.dwPaletteCount = pPalette->Count ;
		eiiInfo.pPaletteEntries = (PEGL_PALETTE) pPalette->Entries ;
		//
		if ( pPalette->Flags & Gdiplus::PaletteFlagsGrayScale )
		{
			eiiInfo.fdwFormatType = EIF_GRAY_BITMAP ;
		}
		else if ( pPalette->Flags & Gdiplus::PaletteFlagsHasAlpha )
		{
			for ( int i = 0; i < (int) pPalette->Count; i ++ )
			{
				if ( pPalette->Entries[i] & 0xFF000000 )
				{
					eiiInfo.fdwFormatType |= EIF_WITH_CLIPPING ;
					eiiInfo.dwClippedPixel = i ;
					break ;
				}
			}
		}
	}
	else
	{
		//
		// RGB フォーマット
		//
		if ( fdwFormat & EIF_WITH_ALPHA )
		{
			pxfmt = PixelFormat32bppARGB ;
			eiiInfo.fdwFormatType = EIF_RGBA_BITMAP ;
		}
		else
		{
			pxfmt = PixelFormat32bppRGB ;
			eiiInfo.fdwFormatType = EIF_RGB_BITMAP ;
		}
		eiiInfo.dwBitsPerPixel = 32 ;
	}
	//
	PEGL_IMAGE_INFO	pImage = NULL ;
	do
	{
		//
		// 画像サイズを取得
		//
		UINT	nWidth, nHeight ;
		if ( (g_pfnGetImageWidth == NULL)
			|| g_pfnGetImageWidth( pbitmap, &nWidth ) )
		{
			break ;
		}
		if ( (g_pfnGetImageHeight == NULL)
			|| g_pfnGetImageHeight( pbitmap, &nHeight ) )
		{
			break ;
		}
		//
		// 画像データ配列を取得
		//
		Gdiplus::Rect	rect ;
		Gdiplus::BitmapData	bmdata ;
		rect.X = 0 ;
		rect.Y = 0 ;
		rect.Width = nWidth ;
		rect.Height = nHeight ;
		if ( (g_pfnBitmapLockBits == NULL)
			|| g_pfnBitmapLockBits( pbitmap, &rect,
				Gdiplus::ImageLockModeRead, pxfmt, &bmdata ) )
		{
			break ;
		}
		//
		// EGL 画像オブジェクトに変換
		//
		eiiInfo.ptrImageArray = bmdata.Scan0 ;
		eiiInfo.dwImageWidth = bmdata.Width ;
		eiiInfo.dwImageHeight = bmdata.Height ;
		eiiInfo.dwBytesPerLine = bmdata.Stride ;
		eiiInfo.dwSizeOfImage =
			eiiInfo.dwBytesPerLine * (SDWORD) eiiInfo.dwImageHeight ;
		//
		::eglReverseVertically( &eiiInfo ) ;
		pImage = ::eglCreateImageBuffer
			( eiiInfo.fdwFormatType, eiiInfo.dwImageWidth,
				eiiInfo.dwImageHeight, eiiInfo.dwBitsPerPixel, 0 ) ;
		if ( pImage != NULL )
		{
			if ( pImage->pPaletteEntries && eiiInfo.pPaletteEntries )
			{
				::eslMoveMemory
					( pImage->pPaletteEntries,
						eiiInfo.pPaletteEntries,
						eiiInfo.dwPaletteCount * sizeof(EGL_PALETTE) ) ;
				pImage->dwClippedPixel = eiiInfo.dwClippedPixel ;
			}
			//
			EGL_DRAW_PARAM	edp ;
			HEGL_DRAW_IMAGE	hDraw = ::eglCreateDrawImage( ) ;
			hDraw->Initialize( pImage, NULL, NULL ) ;
			::eslFillMemory( &edp, 0, sizeof(edp) ) ;
			edp.pSrcImage = &eiiInfo ;
			if ( !hDraw->PrepareDraw( &edp ) )
				hDraw->DrawImage( ) ;
			hDraw->Release( ) ;
			::eglReverseVertically( pImage ) ;
		}
		//
		// 画像データ取得完了
		//
		if ( g_pfnBitmapUnlockBits != NULL )
		{
			g_pfnBitmapUnlockBits( pbitmap, &bmdata ) ;
		}
	}
	while ( false ) ;
	//
	// 終了
	//
	if ( pPalette != NULL )
	{
		::eslHeapFree( NULL, pPalette ) ;
	}
	return	pImage ;
}

static const GUID	GLS_FrameDimensionTime =
	{ 0x6aedbd6d,0x3fb5,0x418a, { 0x83,0xa6,0x7f,0x45,0x22,0x9d,0xc8,0x72 } } ;
static const GUID	GLS_FrameDimensionResolution =
	{ 0x84236f7b,0x3bd3,0x428f, { 0x8d,0xab,0x4e,0xa1,0x43,0x9c,0xa3,0x15 } } ;
static const GUID	GLS_FrameDimensionPage =
	{ 0x7462dc86,0x6180,0x4c7e, { 0x8e,0x3f,0xee,0x73,0x33,0xa7,0xa4,0x83 } } ;
static const GUID	GLS_EncoderQuality =
	{ 0x1d5be4b5,0xfa4a,0x452d, { 0x9c,0xdd,0x5d,0xb3,0x51,0x05,0xe7,0xeb } } ;


// GDI+ オブジェクトから画像オブジェクトへ変換する
//////////////////////////////////////////////////////////////////////////////
ESLError EGLMediaLoader::ConvertFromGDIplus
	( Gdiplus::GpBitmap * pbitmap, DWORD fdwFormat, DWORD dwBitsPerPixel )
{
	unsigned int	nFrameCount = 1 ;
	if ( g_pfnImageGetFrameCount != NULL )
	{
		m_dwTotalTime = 0 ;
		g_pfnImageGetFrameCount
			( pbitmap, &GLS_FrameDimensionTime, &nFrameCount ) ;
		m_dwTotalTime = nFrameCount * 66 ;
	}
	for ( unsigned int i = 0; i < nFrameCount; i ++ )
	{
		if ( g_pfnImageSelectActiveFrame != NULL )
		{
			g_pfnImageSelectActiveFrame( pbitmap, &GLS_FrameDimensionTime, i )  ;
		}
		PEGL_IMAGE_INFO	pImage =
			eglConvertFromGDIplus( pbitmap, fdwFormat, dwBitsPerPixel ) ;
		if ( pImage == NULL )
		{
			return	eslErrGeneral ;
		}
		m_lstFrames.Add( pImage ) ;
		m_lstSequence.Add( i ) ;
	}
	SetCurrentSequence( 0 ) ;
	return	ConvertFormatTo( fdwFormat, dwBitsPerPixel ) ;
}

// 画像フォーマットを変換する
//////////////////////////////////////////////////////////////////////////////
ESLError EGLMediaLoader::ConvertFormatTo
	( DWORD fdwFormat, DWORD dwBitsPerPixel )
{
	for ( unsigned int i = 0; i < m_lstFrames.GetSize(); i ++ )
	{
		PEGL_IMAGE_INFO	pImage = m_lstFrames.GetAt( i ) ;
		if ( pImage == NULL )
			continue ;
		//
		if ( fdwFormat == EIF_WITH_ALPHA )
		{
			if ( (pImage->dwBitsPerPixel == 32)
				&& (pImage->fdwFormatType == EIF_RGB_BITMAP) )
			{
				pImage->fdwFormatType = EIF_RGBA_BITMAP ;
			}
		}
		else if ( ((fdwFormat != 0)
					&& (fdwFormat != pImage->fdwFormatType))
			|| ((dwBitsPerPixel != 0)
					&& (dwBitsPerPixel != pImage->dwBitsPerPixel)) )
		{
			if ( fdwFormat == 0 )
			{
				fdwFormat = pImage->fdwFormatType ;
			}
			PEGL_IMAGE_INFO	pNew =
				::eglCreateImageBuffer
					( fdwFormat, pImage->dwImageWidth,
						pImage->dwImageHeight, dwBitsPerPixel ) ;
			if ( pNew == NULL )
			{
				continue ;
			}
			if ( pImage->dwBytesPerLine < 0 )
			{
				::eglReverseVertically( pNew ) ;
			}
			::eglConvertFormat( pNew, pImage ) ;
			//
			if ( m_pImage == pImage )
			{
				AttachImage( pNew ) ;
			}
			::eglDeleteImageBuffer( pImage ) ;
			m_lstFrames.SetAt( i, pNew ) ;
		}
	}
	return	eslErrSuccess ;
}

// ビットマップデータから画像オブジェクトを作成する
//////////////////////////////////////////////////////////////////////////////
ESLError EGLMediaLoader::CreateFromBitmap
	( const BITMAPINFO * pbmi, void * ptrBitmap, DWORD dwFlags )
{
	DeleteImage( ) ;
	//
	// パックド DIB のビットマップ配列のアドレスを取得
	//
	if ( ptrBitmap == NULL )
	{
		unsigned int	nPalCount = pbmi->bmiHeader.biClrUsed ;
		if ( (nPalCount == 0) && (pbmi->bmiHeader.biBitCount <= 8) )
		{
			nPalCount = (1 << pbmi->bmiHeader.biBitCount) ;
		}
		ptrBitmap = ((BYTE*)pbmi)
			+ sizeof(BITMAPINFOHEADER) + (sizeof(RGBQUAD) * nPalCount) ;
	}
	//
	// EGL_IMAGE_INFO 構造体に変換
	//
	EGL_IMAGE_INFO	eiiInfo ;
	::eslFillMemory( &eiiInfo, 0, sizeof(eiiInfo) ) ;
	eiiInfo.dwInfoSize = sizeof(eiiInfo) ;
	eiiInfo.fdwFormatType = EIF_RGB_BITMAP ;
	eiiInfo.ptrImageArray = ptrBitmap ;
	eiiInfo.dwImageWidth = pbmi->bmiHeader.biWidth ;
	eiiInfo.dwImageHeight = pbmi->bmiHeader.biHeight ;
	eiiInfo.dwBitsPerPixel = pbmi->bmiHeader.biBitCount ;
	if ( eiiInfo.dwBitsPerPixel <= 8 )
	{
		eiiInfo.fdwFormatType |= EIF_WITH_PALETTE ;
		eiiInfo.pPaletteEntries = (PEGL_PALETTE) &(pbmi->bmiColors[0]) ;
		eiiInfo.dwPaletteCount = pbmi->bmiHeader.biClrUsed ;
		if ( eiiInfo.dwPaletteCount == 0 )
		{
			eiiInfo.dwPaletteCount = (1 << eiiInfo.dwBitsPerPixel) ;
		}
	}
	eiiInfo.dwBytesPerLine =
		((eiiInfo.dwImageWidth
			* eiiInfo.dwBitsPerPixel + 0x1F) & (~0x1F)) >> 3 ;
	eiiInfo.dwSizeOfImage = eiiInfo.dwBytesPerLine * eiiInfo.dwImageHeight ;
	//
	// 複製
	//
	DuplicateImage( &eiiInfo, dwFlags ) ;
	ReverseVertically( ) ;
	//
	return	eslErrSuccess ;
}

// 画像オブジェクトからパックドDIBを作成する
//////////////////////////////////////////////////////////////////////////////
BITMAPINFO * EGLMediaLoader::CreatePackedDIB( void ** ppBitmap ) const
{
	if ( m_pImage == NULL )
	{
		if ( ppBitmap != NULL )
			*ppBitmap = NULL ;
		return	NULL ;
	}
	//
	// 画像サイズ計算
	//
	int		nLineBytes, nImageSize, nPalCount = 0 ;
	if ( m_pImage->fdwFormatType & EIF_WITH_PALETTE )
	{
		nPalCount = (int) m_pImage->dwPaletteCount ;
	}
	else if ( m_pImage->fdwFormatType == EIF_GRAY_BITMAP )
	{
		nPalCount = 256 ;
	}
	nLineBytes = ((m_pImage->dwImageWidth
				* m_pImage->dwBitsPerPixel + 0x1F) & (~0x1F)) >> 3 ;
	nImageSize = nLineBytes * m_pImage->dwImageHeight ;
	//
	// メモリ確保
	//
	int		nHeahderSize =
		sizeof(BITMAPINFOHEADER) + nPalCount * sizeof(RGBQUAD) ;
	BITMAPINFO *	pbmi =
		(BITMAPINFO*) ::eslHeapAllocate
			( NULL, nHeahderSize + nImageSize, ESL_HEAP_ZERO_INIT ) ;
	BYTE *	pImageLine = ((BYTE*) pbmi) + nHeahderSize ;
	if ( ppBitmap != NULL )
	{
		*ppBitmap = pImageLine ;
	}
	//
	// ヘッダ情報設定
	//
	pbmi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER) ;
	pbmi->bmiHeader.biWidth = m_pImage->dwImageWidth ;
	pbmi->bmiHeader.biHeight = m_pImage->dwImageHeight ;
	pbmi->bmiHeader.biPlanes = 1 ;
	pbmi->bmiHeader.biBitCount = (WORD) m_pImage->dwBitsPerPixel ;
	pbmi->bmiHeader.biSizeImage = nImageSize ;
	//
	// パレットテーブルを複製
	//
	if ( nPalCount > 0 )
	{
		if ( m_pImage->fdwFormatType & EIF_WITH_PALETTE )
		{
			::eslMoveMemory
				( &(pbmi->bmiColors[0]),
					m_pImage->pPaletteEntries,
					nPalCount * sizeof(RGBQUAD) ) ;
		}
		else
		{
			for ( int i = 0; i < nPalCount; i ++ )
			{
				pbmi->bmiColors[i].rgbBlue = (BYTE) i ;
				pbmi->bmiColors[i].rgbGreen = (BYTE) i ;
				pbmi->bmiColors[i].rgbRed = (BYTE) i ;
				pbmi->bmiColors[i].rgbReserved = 0 ;
			}
		}
	}
	//
	// ビットマップ配列を複製
	//
	BYTE *	pbytSrcLine = (BYTE*) m_pImage->ptrImageArray ;
	SDWORD	dwSrcLineBytes = m_pImage->dwBytesPerLine ;
	if ( dwSrcLineBytes < 0 )
	{
		pbytSrcLine += dwSrcLineBytes * (m_pImage->dwImageHeight - 1) ;
		dwSrcLineBytes = - dwSrcLineBytes ;
	}
	for ( DWORD y = 0; y < m_pImage->dwImageHeight; y ++ )
	{
		::eslMoveMemory( pImageLine, pbytSrcLine, nLineBytes ) ;
		pImageLine += nLineBytes ;
		pbytSrcLine += dwSrcLineBytes ;
	}
	//
	return	pbmi ;
}

// 画像オブジェクトから GDI+ オブジェクトへ変換する
//////////////////////////////////////////////////////////////////////////////
ESLError EGLMediaLoader::CreateGDIplusBitmap( Gdiplus::GpBitmap *& pbitmap )
{
	PEGL_IMAGE_INFO	pImage = GetInfo( ) ;
	pbitmap = NULL ;
	if ( pImage == NULL )
	{
		return	eslErrGeneral ;
	}
	Gdiplus::PixelFormat	pxfmt ;
	switch ( pImage->fdwFormatType & EIF_TYPE_MASK )
	{
	case	EIF_RGB_BITMAP:
	case	EIF_YUV_BITMAP:
	case	EIF_HSB_BITMAP:
	case	EIF_GRAY_BITMAP:
		switch ( pImage->dwBitsPerPixel )
		{
		case	24:
			pxfmt = PixelFormat24bppRGB ;
			break ;
		case	32:
			pxfmt = PixelFormat32bppRGB ;
			if ( pImage->fdwFormatType & EIF_WITH_ALPHA )
			{
				pxfmt = PixelFormat32bppARGB ;
			}
			break ;
		case	8:
			pxfmt = PixelFormat8bppIndexed ;
			break ;
		case	16:
			pxfmt = PixelFormat16bppRGB555 ;
			break ;
		case	1:
			pxfmt = PixelFormat1bppIndexed ;
			break ;
		case	4:
			pxfmt = PixelFormat4bppIndexed ;
			break ;
		}
		break ;
	case	EIF_Z_BUFFER_R4:
		pxfmt = PixelFormat32bppARGB ;
		break ;
	}
	if ( g_pfnCreateBitmapFromScan0 == NULL )
	{
		return	eslErrGeneral ;
	}
	if ( g_pfnCreateBitmapFromScan0
		( pImage->dwImageWidth, pImage->dwImageHeight,
			pImage->dwBytesPerLine, pxfmt,
			(BYTE*) pImage->ptrImageArray, &pbitmap ) != Gdiplus::Ok )
	{
		return	eslErrGeneral ;
	}
	if ( (pImage->dwPaletteCount != 0) && (pImage->pPaletteEntries != NULL) )
	{
		if ( g_pfnSetImagePalette != NULL )
		{
			EStreamBuffer	bufPalette ;
			Gdiplus::ColorPalette *	pPltTable =
				(Gdiplus::ColorPalette*)
				bufPalette.PutBuffer( sizeof(Gdiplus::ColorPalette)
					+ pImage->dwPaletteCount * sizeof(Gdiplus::ARGB) ) ;
			pPltTable->Flags = 0 ;
			pPltTable->Count = pImage->dwPaletteCount ;
			if ( pImage->fdwFormatType & EIF_WITH_CLIPPING )
			{
				pPltTable->Flags |= Gdiplus::PaletteFlagsHasAlpha ;
				for ( UINT i = 0; i < pPltTable->Count; i ++ )
				{
					if ( i != pImage->dwClippedPixel )
					{
						pPltTable->Entries[i] =
							pImage->pPaletteEntries[i].dwPixelCode | 0xFF000000 ;
					}
					else
					{
						pPltTable->Entries[i] = 0 ;
					}
				}
			}
			else if ( pImage->fdwFormatType & EIF_WITH_ALPHA )
			{
				pPltTable->Flags |= Gdiplus::PaletteFlagsHasAlpha ;
				for ( UINT i = 0; i < pPltTable->Count; i ++ )
				{
					pPltTable->Entries[i] =
						pImage->pPaletteEntries[i].dwPixelCode ;
				}
			}
			else
			{
				if ( (pImage->fdwFormatType & EIF_TYPE_MASK) == EIF_GRAY_BITMAP )
				{
					pPltTable->Flags |= Gdiplus::PaletteFlagsGrayScale ;
				}
				for ( UINT i = 0; i < pPltTable->Count; i ++ )
				{
					pPltTable->Entries[i] =
						pImage->pPaletteEntries[i].dwPixelCode | 0xFF000000 ;
				}
			}
			g_pfnSetImagePalette( pbitmap, pPltTable ) ;
		}
	}
	return	eslErrSuccess ;
}

// Windows Bitmap ファイルへ書き出す
//////////////////////////////////////////////////////////////////////////////
ESLError EGLMediaLoader::WriteBitmapFile( ESLFileObject & file )
{
	PEGL_IMAGE_INFO	pImage = GetInfo( ) ;
	if ( pImage == NULL )
	{
		return	eslErrGeneral ;
	}
	//
	// ファイルヘッダ準備
	//
	BITMAPFILEHEADER	bmfh ;
	BITMAPINFOHEADER	bmih ;
	DWORD	dwLineBytes, dwPaletteBytes = 0 ;
	::memset( &bmfh, 0, sizeof(bmfh) ) ;
	::memset( &bmih, 0, sizeof(bmih) ) ;
	//
	dwLineBytes =
		((pImage->dwImageWidth
			* pImage->dwBitsPerPixel + 0x1F) & ~0x1F) >> 3 ;
	bmih.biSize = sizeof(bmih) ;
	bmih.biWidth = pImage->dwImageWidth ;
	bmih.biHeight = pImage->dwImageHeight ;
	bmih.biPlanes = 1 ;
	bmih.biBitCount = (WORD) pImage->dwBitsPerPixel ;
	bmih.biSizeImage = dwLineBytes * pImage->dwImageHeight ;
	bmih.biXPelsPerMeter = m_nResolution * 3937 / 10000 ;
	bmih.biYPelsPerMeter = bmih.biXPelsPerMeter ;
	if ( pImage->pPaletteEntries != NULL )
	{
		bmih.biClrUsed = pImage->dwPaletteCount ;
		if ( pImage->dwPaletteCount > 0 )
		{
			dwPaletteBytes = pImage->dwPaletteCount * sizeof(RGBQUAD) ;
		}
	}
	bmfh.bfType = 'MB' ;
	bmfh.bfOffBits = sizeof(bmfh) + sizeof(bmih) + dwPaletteBytes ;
	bmfh.bfSize = bmfh.bfOffBits + bmih.biSizeImage ;
	//
	// ヘッダ書き出し
	//
	if ( file.Write( &bmfh, sizeof(bmfh) ) < sizeof(bmfh) )
	{
		return	eslErrGeneral ;
	}
	if ( file.Write( &bmih, sizeof(bmih) ) < sizeof(bmih) )
	{
		return	eslErrGeneral ;
	}
	if ( dwPaletteBytes != 0 )
	{
		if ( file.Write
			( pImage->pPaletteEntries, dwPaletteBytes ) < dwPaletteBytes )
		{
			return	eslErrGeneral ;
		}
	}
	//
	// 画像配列書き出し
	//
	const BYTE *	pbytLine = (const BYTE *) pImage->ptrImageArray ;
	SDWORD	dwBytesPerLine = pImage->dwBytesPerLine ;
	pbytLine += (pImage->dwImageHeight - 1) * dwBytesPerLine ;
	dwBytesPerLine = - dwBytesPerLine ;
	//
	for ( DWORD y = 0; y < pImage->dwImageHeight; y ++ )
	{
		if ( file.Write( pbytLine, dwLineBytes ) < dwLineBytes )
		{
			return	eslErrGeneral ;
		}
		pbytLine += dwBytesPerLine ;
	}
	return	eslErrSuccess ;
}

// Photoshop PSD ファイルを書き出す
//////////////////////////////////////////////////////////////////////////////
ESLError EGLMediaLoader::WritePhotoshopPSDFile( ESLFileObject & file )
{
	PEGL_IMAGE_INFO	pImage = GetInfo( ) ;
	if ( pImage == NULL )
	{
		return	eslErrGeneral ;
	}
	PSD::File	psdf ;
	if ( psdf.PrepareToWrite( file, pImage ) )
	{
		return	eslErrGeneral ;
	}
	if ( (pImage->fdwFormatType & EIF_WITH_ALPHA)
						|| (GetTotalFrameCount() > 1) )
	{
		for ( int i = 0; i < (int) GetTotalFrameCount(); i ++ )
		{
			pImage = GetFrameAt( i ) ;
			if ( pImage != NULL )
			{
				psdf.AddLayerImage( pImage ) ;
			}
		}
	}
	return	psdf.FinishToWrite( ) ;
}

// AVI ファイルを書き出す
//////////////////////////////////////////////////////////////////////////////
ESLError EGLMediaLoader::SaveAviFile( const char * pszFileName )
{
	ESLError	errResult = eslErrGeneral ;
	Initialize( ) ;
	//
	PAVIFILE	pAviFile = NULL ;
	PAVISTREAM	pAviStream = NULL ;
	do
	{
		//
		// AVI ファイルを開く
		//
		if ( ::AVIFileOpen( &pAviFile, pszFileName, OF_CREATE, NULL ) )
		{
			ESLTrace( "Failed to AVIFileOpen.\n" ) ;
			break ;
		}
		//
		// ビデオストリームを作成
		//
		AVISTREAMINFO	sinf ;
		PEGL_IMAGE_INFO	pImage = GetInfo( ) ;
		if ( pImage == NULL )
		{
			break ;
		}
		int	nFrameCount = GetSequenceLength() ;
		::memset( &sinf, 0, sizeof(sinf) ) ;
		sinf.fccType = streamtypeVIDEO ;
		sinf.dwScale = GetTotalTime() ;
		sinf.dwRate = nFrameCount * 1000 ;
		sinf.dwLength = nFrameCount ;
		sinf.rcFrame.right = pImage->dwImageWidth ;
		sinf.rcFrame.bottom = pImage->dwImageHeight ;
		//
		if ( ::AVIFileCreateStream( pAviFile, &pAviStream, &sinf ) )
		{
			ESLTrace( "Failed to AVIFileCreateStream.\n" ) ;
			break ;
		}
		//
		// ビットマップヘッダを出力
		//
		BITMAPINFOHEADER	bmih ;
		::memset( &bmih, 0, sizeof(bmih) ) ;
		const int	nLineBytes =
			((pImage->dwImageWidth
				* pImage->dwBitsPerPixel + 0x1F) & ~0x1F) >> 3 ;
		bmih.biSize = sizeof(bmih) ;
		bmih.biWidth = pImage->dwImageWidth ;
		bmih.biHeight = pImage->dwImageHeight ;
		bmih.biPlanes = 1 ;
		bmih.biBitCount = (WORD) pImage->dwBitsPerPixel ;
		bmih.biSizeImage = nLineBytes * pImage->dwImageHeight ;
		//
		if ( ::AVIStreamSetFormat( pAviStream, 0, &bmih, sizeof(bmih) ) )
		{
			ESLTrace( "Faild to AVIStreamSetFormat.\n" ) ;
			break ;
		}
		//
		// 順次画像フレームを書き出す
		//
		EStreamBuffer	bufImage ;
		const DWORD		dwImageBytes = bmih.biSizeImage ;
		BYTE *			pbytDstBuf =
							(BYTE*) bufImage.PutBuffer( dwImageBytes ) ;
		//
		for ( int i = 0; i < nFrameCount; i ++ )
		{
			//
			PEGL_IMAGE_INFO	pInfo = GetFrameAt( SequenceToFrame( i ) ) ;
			if ( (pInfo == NULL)
				|| (pInfo->dwImageWidth != pImage->dwImageWidth)
				|| (pInfo->dwImageHeight != pImage->dwImageHeight)
				|| (pInfo->dwBitsPerPixel != pImage->dwBitsPerPixel) )
			{
				continue ;
			}
			//
			BYTE *	pbytDstLine = pbytDstBuf ;
			const BYTE *	pbytSrcLine = (const BYTE*) pInfo->ptrImageArray ;
			SDWORD			dwBytesPerLine = pInfo->dwBytesPerLine ;
			pbytSrcLine += (pInfo->dwImageHeight - 1) * dwBytesPerLine ;
			dwBytesPerLine = - dwBytesPerLine ;
			//
			for ( DWORD y = 0; y < pInfo->dwImageHeight; y ++ )
			{
				eslMoveMemory( pbytDstLine, pbytSrcLine, nLineBytes ) ;
				pbytDstLine += nLineBytes ;
				pbytSrcLine += dwBytesPerLine ;
			}
			//
			if ( ::AVIStreamWrite
				( pAviStream, i, 1,
					pbytDstBuf, dwImageBytes,
						AVIIF_KEYFRAME, NULL, NULL ) )
			{
				ESLTrace( "Failed to AVIStreamWrite.(%dth frame)\n", i ) ;
			}
		}
		//
		errResult = eslErrSuccess ;
	}
	while ( false ) ;
	//
	if ( pAviStream != NULL )
	{
		::AVIStreamRelease( pAviStream ) ;
	}
	if ( pAviFile != NULL )
	{
		::AVIFileRelease( pAviFile ) ;
	}
	//
	Close( ) ;
	return	errResult ;
}

// GDI+ を利用して書き出す
//////////////////////////////////////////////////////////////////////////////
ESLError EGLMediaLoader::SaveWithGDIplus
	( const char * pszFileName, const wchar_t * pwszMimeType, int nQuality )
{
	ESLError	errResult = eslErrGeneral ;
	Gdiplus::GpBitmap *	pbitmap = NULL ;
	Initialize( ) ;
	do
	{
		//
		// GDI+ オブジェクトに変換する
		//
		if ( g_pfnSaveImageToFile == NULL )
		{
			return	eslErrGeneral ;
		}
		CLSID	clsidEncoder ;
		if ( GetEncoderClsid( pwszMimeType, clsidEncoder ) )
		{
			break ;
		}
		if ( CreateGDIplusBitmap( pbitmap ) )
		{
			break ;
		}
		//
		// エンコード・パラメータ
		//
		Gdiplus::EncoderParameters * pEncParams = NULL ;
		Gdiplus::EncoderParameters	encParams ;
		ULONG						ulQualityParam ;
		if ( nQuality >= 0 )
		{
			ulQualityParam = nQuality ;
			if ( nQuality > 100 )
			{
				ulQualityParam = 100 ;
			}
			encParams.Count = 1 ;
			encParams.Parameter[0].Guid = GLS_EncoderQuality ;
			encParams.Parameter[0].Type = Gdiplus::EncoderParameterValueTypeLong ;
			encParams.Parameter[0].NumberOfValues = 1 ;
			encParams.Parameter[0].Value = &ulQualityParam ;
			//
			pEncParams = &encParams ;
		}
		//
		// ファイルへ保存する
		//
		ESLAssert( g_pfnSaveImageToFile != NULL ) ;
		if ( Gdiplus::Ok != g_pfnSaveImageToFile
			( pbitmap, EWideString(pszFileName), &clsidEncoder, pEncParams ) )
		{
			break ;
		}
		errResult = eslErrSuccess ;
	}
	while ( false ) ;
	//
	ESLAssert( g_pfnDisposeImage != NULL ) ;
	if ( g_pfnDisposeImage != NULL )
	{
		g_pfnDisposeImage( pbitmap ) ;
	}
	Close( ) ;
	return	errResult ;
}

// GDI+ を利用して書き出す
//////////////////////////////////////////////////////////////////////////////
ESLError EGLMediaLoader::WriteWithGDIplus
	( ESLFileObject & file, const wchar_t * pwszMimeType, int nQuality )
{
	Gdiplus::GpBitmap *	pbitmap = NULL ;
	HGLOBAL				hGlobal = NULL ;
	LPSTREAM			stream = NULL ;
	ESLError			errResult = eslErrGeneral ;
	Initialize( ) ;
	do
	{
		//
		// GDI+ オブジェクトへ変換する
		//
		if ( g_pfnSaveImageToStream == NULL )
		{
			break ;
		}
		CLSID	clsidEncoder ;
		if ( GetEncoderClsid( pwszMimeType, clsidEncoder ) )
		{
			break ;
		}
		if ( CreateGDIplusBitmap( pbitmap ) )
		{
			break ;
		}
		//
		// IStream オブジェクトを生成する
		//
		hGlobal = ::GlobalAlloc( GMEM_MOVEABLE, 0 ) ;
		if ( hGlobal == NULL )
		{
			break ;
		}
		if ( ::CreateStreamOnHGlobal( hGlobal, TRUE, &stream ) != S_OK )
		{
			break ;
		}
		if ( stream == NULL )
		{
			break ;
		}
		//
		// エンコード・パラメータ
		//
		Gdiplus::EncoderParameters * pEncParams = NULL ;
		Gdiplus::EncoderParameters	encParams ;
		ULONG						ulQualityParam ;
		if ( nQuality >= 0 )
		{
			ulQualityParam = nQuality ;
			if ( nQuality > 100 )
			{
				ulQualityParam = 100 ;
			}
			encParams.Count = 1 ;
			encParams.Parameter[0].Guid = GLS_EncoderQuality ;
			encParams.Parameter[0].Type = Gdiplus::EncoderParameterValueTypeLong ;
			encParams.Parameter[0].NumberOfValues = 1 ;
			encParams.Parameter[0].Value = &ulQualityParam ;
			//
			pEncParams = &encParams ;
		}
		//
		// IStream へ書き出す
		//
		ESLAssert( g_pfnSaveImageToStream != NULL ) ;
		if ( g_pfnSaveImageToStream
			( pbitmap, stream, &clsidEncoder, pEncParams ) != Gdiplus::Ok )
		{
			break ;
		}
		//
		// ESLFileObject へ書き出す
		//
		STATSTG	stat ;
		if ( stream->Stat( &stat, STATFLAG_NONAME ) != S_OK )
		{
			break ;
		}
		LPVOID	ptrGlobal = ::GlobalLock( hGlobal ) ;
		if ( ptrGlobal == NULL )
		{
			break ;
		}
		if ( file.Write
			( ptrGlobal, stat.cbSize.LowPart ) < stat.cbSize.LowPart )
		{
			::GlobalUnlock( hGlobal ) ;
			break ;
		}
		::GlobalUnlock( hGlobal ) ;
		errResult = eslErrSuccess ;
	}
	while ( false ) ;
	//
	// 終了
	//
	ESLAssert( g_pfnDisposeImage != NULL ) ;
	if ( g_pfnDisposeImage != NULL )
	{
		g_pfnDisposeImage( pbitmap ) ;
	}
	if ( stream != NULL )
	{
		stream->Release( ) ;
	}
	if ( hGlobal != NULL )
	{
		GlobalFree( hGlobal ) ;
	}
	Close( ) ;
	return	errResult ;
}

// MIME タイプから CLSID を取得
//////////////////////////////////////////////////////////////////////////////
ESLError EGLMediaLoader::GetEncoderClsid
	( const wchar_t * pwszMimeType, CLSID & clsidEncoder )
{
	if ( (g_pfnGetImageEncodersSize == NULL)
			|| (g_pfnGetImageEncoders == NULL) )
	{
		return	eslErrGeneral ;
	}
	UINT	nEncoderCount, nEncoderSize ;
	if ( g_pfnGetImageEncodersSize
		( &nEncoderCount, &nEncoderSize ) != Gdiplus::Ok )
	{
		return	eslErrGeneral ;
	}
	EStreamBuffer	bufEncoders ;
	Gdiplus::ImageCodecInfo *	piciEncoders =
		(Gdiplus::ImageCodecInfo*) bufEncoders.PutBuffer( nEncoderSize ) ;
	if ( g_pfnGetImageEncoders
		( nEncoderCount, nEncoderSize, piciEncoders ) != Gdiplus::Ok )
	{
		return	eslErrGeneral ;
	}
	EWideString	wstrMimeType = pwszMimeType ;
	for ( UINT i = 0; i < nEncoderCount; i ++ )
	{
		if ( !wstrMimeType.CompareNoCase( piciEncoders[i].MimeType ) )
		{
			clsidEncoder = piciEncoders[i].Clsid ;
			return	eslErrSuccess ;
		}
	}
	return	eslErrGeneral ;
}

// GDI+ で利用可能なデコーダーを列挙する
//////////////////////////////////////////////////////////////////////////////
ESLError EGLMediaLoader::EnumGDIplusDecoderList
	( EObjArray<EGLMediaLoader::GDIP_CODEC> & lstCodec )
{
	if ( (g_pfnGetImageDecodersSize == NULL)
			|| (g_pfnGetImageDecoders == NULL) )
	{
		return	eslErrGeneral ;
	}
	UINT	nDecoderCount, nDecoderSize ;
	if ( g_pfnGetImageDecodersSize
		( &nDecoderCount, &nDecoderSize ) != Gdiplus::Ok )
	{
		return	eslErrGeneral ;
	}
	EStreamBuffer	bufDecoders ;
	Gdiplus::ImageCodecInfo *	piciDecoders =
		(Gdiplus::ImageCodecInfo*) bufDecoders.PutBuffer( nDecoderSize ) ;
	if ( g_pfnGetImageDecoders
		( nDecoderCount, nDecoderSize, piciDecoders ) != Gdiplus::Ok )
	{
		return	eslErrGeneral ;
	}
	lstCodec.RemoveAll( ) ;
	for ( UINT i = 0; i < nDecoderCount; i ++ )
	{
		GDIP_CODEC *	pCodec = new GDIP_CODEC ;
		pCodec->wstrCodecName = piciDecoders[i].CodecName ;
		pCodec->wstrDllName = piciDecoders[i].DllName ;
		pCodec->wstrDescription = piciDecoders[i].FormatDescription ;
		pCodec->wstrFileExtension = piciDecoders[i].FilenameExtension ;
		pCodec->wstrMimeType = piciDecoders[i].MimeType ;
		lstCodec.Add( pCodec ) ;
	}
	return	eslErrSuccess ;
}

// GDI+ で利用可能なエンコーダーを列挙する
//////////////////////////////////////////////////////////////////////////////
ESLError EGLMediaLoader::EnumGDIplusEncoderList
	( EObjArray<EGLMediaLoader::GDIP_CODEC> & lstCodec )
{
	if ( (g_pfnGetImageEncodersSize == NULL)
			|| (g_pfnGetImageEncoders == NULL) )
	{
		return	eslErrGeneral ;
	}
	UINT	nEncoderCount, nEncoderSize ;
	if ( g_pfnGetImageEncodersSize
		( &nEncoderCount, &nEncoderSize ) != Gdiplus::Ok )
	{
		return	eslErrGeneral ;
	}
	EStreamBuffer	bufEncoders ;
	Gdiplus::ImageCodecInfo *	piciEncoders =
		(Gdiplus::ImageCodecInfo*) bufEncoders.PutBuffer( nEncoderSize ) ;
	if ( g_pfnGetImageEncoders
		( nEncoderCount, nEncoderSize, piciEncoders ) != Gdiplus::Ok )
	{
		return	eslErrGeneral ;
	}
	lstCodec.RemoveAll( ) ;
	for ( UINT i = 0; i < nEncoderCount; i ++ )
	{
		GDIP_CODEC *	pCodec = new GDIP_CODEC ;
		pCodec->wstrCodecName = piciEncoders[i].CodecName ;
		pCodec->wstrDllName = piciEncoders[i].DllName ;
		pCodec->wstrDescription = piciEncoders[i].FormatDescription ;
		pCodec->wstrFileExtension = piciEncoders[i].FilenameExtension ;
		pCodec->wstrMimeType = piciEncoders[i].MimeType ;
		lstCodec.Add( pCodec ) ;
	}
	return	eslErrSuccess ;
}

// 初期化
//////////////////////////////////////////////////////////////////////////////
void EGLMediaLoader::Initialize( bool fComMTA )
{
	if ( ++ g_nRefMediaLoader == 1 )
	{
		::CoInitialize( NULL ) ;
//		::CoInitializeEx
//			( NULL, fComMTA ?
//				COINIT_MULTITHREADED : COINIT_APARTMENTTHREADED ) ;
		::AVIFileInit( ) ;
		//
		if ( g_hGDIPlus == NULL )
		{
			g_hGDIPlus = (HMODULE) ::LoadLibrary( "gdiplus.dll" ) ;
			if ( g_hGDIPlus != NULL )
			{
				g_pfnGdiplusStartup = (PGDIP_GdiplusStartup)
					::GetProcAddress( g_hGDIPlus, "GdiplusStartup" ) ;
				g_pfnGdiplusShutdown = (PGDIP_GdiplusShutdown)
					::GetProcAddress( g_hGDIPlus, "GdiplusShutdown" ) ;
				g_pfnCreateBitmapFromStream = (PGDIP_GdipCreateBitmapFromStream)
					::GetProcAddress( g_hGDIPlus, "GdipCreateBitmapFromStream" ) ;
				g_pfnCreateBitmapFromFile = (PGDIP_GdipCreateBitmapFromFile)
					::GetProcAddress( g_hGDIPlus, "GdipCreateBitmapFromFile" ) ;
				g_pfnCreateBitmapFromScan0 = (PGDIP_GdipCreateBitmapFromScan0)
					::GetProcAddress( g_hGDIPlus, "GdipCreateBitmapFromScan0" ) ;
				g_pfnDisposeImage = (PGDIP_GdipDisposeImage)
					::GetProcAddress( g_hGDIPlus, "GdipDisposeImage" ) ;
				g_pfnGetImageFlags = (PGDIP_GdipGetImageFlags)
					::GetProcAddress( g_hGDIPlus, "GdipGetImageFlags" ) ;
				g_pfnImageGetFrameCount = (PGDIP_GdipImageGetFrameCount)
					::GetProcAddress( g_hGDIPlus, "GdipImageGetFrameCount" ) ;
				g_pfnImageSelectActiveFrame = (PGDIP_GdipImageSelectActiveFrame)
					::GetProcAddress( g_hGDIPlus, "GdipImageSelectActiveFrame" ) ;
				g_pfnGetImagePixelFormat = (PGDIP_GdipGetImagePixelFormat)
					::GetProcAddress( g_hGDIPlus, "GdipGetImagePixelFormat" ) ;
				g_pfnGetImageThumbnail = (PGDIP_GdipGetImageThumbnail)
					::GetProcAddress( g_hGDIPlus, "GdipGetImageThumbnail" ) ;
				g_pfnGetImagePaletteSize = (PGDIP_GdipGetImagePaletteSize)
					::GetProcAddress( g_hGDIPlus, "GdipGetImagePaletteSize" ) ;
				g_pfnGetImagePalette = (PGDIP_GdipGetImagePalette)
					::GetProcAddress( g_hGDIPlus, "GdipGetImagePalette" ) ;
				g_pfnSetImagePalette = (PGDIP_GdipSetImagePalette)
					::GetProcAddress( g_hGDIPlus, "GdipSetImagePalette" ) ;
				g_pfnGetImageWidth = (PGDIP_GdipGetImageWidth)
					::GetProcAddress( g_hGDIPlus, "GdipGetImageWidth" ) ;
				g_pfnGetImageHeight = (PGDIP_GdipGetImageHeight)
					::GetProcAddress( g_hGDIPlus, "GdipGetImageHeight" ) ;
				g_pfnBitmapLockBits = (PGDIP_GdipBitmapLockBits)
					::GetProcAddress( g_hGDIPlus, "GdipBitmapLockBits" ) ;
				g_pfnBitmapUnlockBits = (PGDIP_GdipBitmapUnlockBits)
					::GetProcAddress( g_hGDIPlus, "GdipBitmapUnlockBits" ) ;
				g_pfnSaveImageToFile = (PDGIP_GdipSaveImageToFile)
					::GetProcAddress( g_hGDIPlus, "GdipSaveImageToFile" ) ;
				g_pfnSaveImageToStream = (PDGIP_GdipSaveImageToStream)
					::GetProcAddress( g_hGDIPlus, "GdipSaveImageToStream" ) ;
				g_pfnGetImageDecodersSize = (PGDIP_GdipGetImageDecodersSize)
					::GetProcAddress( g_hGDIPlus, "GdipGetImageDecodersSize" ) ;
				g_pfnGetImageDecoders = (PGDIP_GdipGetImageDecoders)
					::GetProcAddress( g_hGDIPlus, "GdipGetImageDecoders" ) ;
				g_pfnGetImageEncodersSize = (PGDIP_GdipGetImageEncodersSize)
					::GetProcAddress( g_hGDIPlus, "GdipGetImageEncodersSize" ) ;
				g_pfnGetImageEncoders = (PGDIP_GdipGetImageEncoders)
					::GetProcAddress( g_hGDIPlus, "GdipGetImageEncoders" ) ;
				//
				if ( g_pfnGdiplusStartup != NULL )
				{
					Gdiplus::GdiplusStartupInput	gdiplusStartupInput ;
					g_pfnGdiplusStartup
						( &g_gdiplusToken, &gdiplusStartupInput, NULL ) ;
				}
			}
		}
	}
}

// 終了
//////////////////////////////////////////////////////////////////////////////
void EGLMediaLoader::Close( void )
{
	long int	nRef = -- g_nRefMediaLoader ;
	ESLAssert( nRef >= 0 ) ;
	if ( nRef == 0 )
	{
		if ( g_pfnGdiplusShutdown != NULL )
		{
			g_pfnGdiplusShutdown( g_gdiplusToken ) ;
		}
		if ( g_hGDIPlus != NULL )
		{
			::FreeLibrary( g_hGDIPlus ) ;
			g_hGDIPlus = NULL ;
		}
		//
		::AVIFileExit( ) ;
//		::CoUninitialize( ) ;
	}
}

// GDI+ は有効か？
//////////////////////////////////////////////////////////////////////////////
bool EGLMediaLoader::IsInstalledGDIplus( void )
{
	return	(g_hGDIPlus != NULL) && (g_pfnGdiplusShutdown != NULL) ;
}


//////////////////////////////////////////////////////////////////////////////
// 減色処理クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
IMPLEMENT_CLASS_INFO( EGLColorDecreaser, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
EGLColorDecreaser::EGLColorDecreaser( void )
{
	m_pDstImage = NULL ;
	m_pSrcImage = NULL ;
	m_pcsTable = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
EGLColorDecreaser::~EGLColorDecreaser( void )
{
	delete [] m_pcsTable ;
}

// 画像を変換する
//////////////////////////////////////////////////////////////////////////////
ESLError EGLColorDecreaser::ConvertImage
	( PEGL_IMAGE_INFO pDstImage,
		PEGL_IMAGE_INFO pSrcImage, DWORD fdwFlags,
		unsigned int nColors, double rColorWeight,
		unsigned int nAlphaThreshold )
{
	if ( (pDstImage == NULL) || (pSrcImage == NULL) )
	{
		return	eslErrGeneral ;
	}
	if ( pDstImage->pPaletteEntries == NULL )
	{
		return	eslErrGeneral ;
	}
	if ( pDstImage->dwBitsPerPixel > 8 )
	{
		return	eslErrGeneral ;
	}
	AttachImageBuffer( pDstImage, pSrcImage ) ;
	//
	if ( !(m_pSrcImage->fdwFormatType & EIF_WITH_ALPHA) )
	{
		fdwFlags &= ~cvfTransparencyPalette ;
	}
	//
	ESLError	err ;
	int			nPaletteCount = (1 << pDstImage->dwBitsPerPixel) ;
	if ( nColors >= 8 )
	{
		if ( nColors < (unsigned int) nPaletteCount )
		{
			nPaletteCount = nColors ;
		}
	}
	if ( fdwFlags & cvfTransparencyPalette )
	{
		nPaletteCount -- ;
		pDstImage->dwClippedPixel = nPaletteCount ;
		pDstImage->fdwFormatType |= EIF_WITH_CLIPPING ;
	}
	else if ( m_pSrcImage->fdwFormatType & EIF_WITH_ALPHA )
	{
		fdwFlags |= cvfPaletteHasAlpha ;
		pDstImage->fdwFormatType |= EIF_WITH_ALPHA ;
	}
	err = MakePalette
		( pDstImage->pPaletteEntries,
			nPaletteCount, fdwFlags, rColorWeight ) ;
	if ( err )
	{
		return	err ;
	}
	if ( (fdwFlags & cvfTransparencyPalette)
		&& (nPaletteCount < (int) pDstImage->dwPaletteCount) )
	{
		pDstImage->dwClippedPixel = nPaletteCount ;
		pDstImage->dwPaletteCount = nPaletteCount + 1 ;
	}
	else
	{
		pDstImage->dwPaletteCount = nPaletteCount ;
	}
	return	DecreaseColorImage( fdwFlags, nAlphaThreshold ) ;
}

// 処理画像バッファを関連付ける
//////////////////////////////////////////////////////////////////////////////
void EGLColorDecreaser::AttachImageBuffer
	( PEGL_IMAGE_INFO pDstImage, PEGL_IMAGE_INFO pSrcImage )
{
	m_pDstImage = pDstImage ;
	m_pSrcImage = pSrcImage ;
}

// 減色パレットを決定
//////////////////////////////////////////////////////////////////////////////
ESLError EGLColorDecreaser::MakePalette
	( EGL_PALETTE rgbPalette[], int & nPaletteCount,
					DWORD fdwFlags, double rColorWeight )
{
	if ( m_pSrcImage == NULL )
	{
		return	eslErrGeneral ;
	}
	if ( m_pSrcImage->dwBitsPerPixel < 24 )
	{
		return	eslErrGeneral ;
	}
	//
	// 色統計情報
	//
	PrepareMakePalette() ;
	//
	ESLError	err =
		CompileColorStatistics( m_pSrcImage, fdwFlags ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// パレット生成
	//
	return	BuildPalette
		( rgbPalette, nPaletteCount, fdwFlags, rColorWeight ) ;
}

// パレット生成準備
//////////////////////////////////////////////////////////////////////////////
void EGLColorDecreaser::PrepareMakePalette( void )
{
	if ( m_pcsTable == NULL )
	{
		m_pcsTable = new COLOR_STATISTICS[0x2000] ;
		m_nTableSize = 0x2000 ;
	}
	::memset( m_pcsTable, 0, m_nTableSize * sizeof(COLOR_STATISTICS) ) ;
}

// 画像情報統計処理
//////////////////////////////////////////////////////////////////////////////
ESLError EGLColorDecreaser::CompileColorStatistics
		( PEGL_IMAGE_INFO pImageInf, DWORD fdwFlags )
{
	ESLError	err ;
	err = OnProgress
		( ptCompileColorStatistics, 0, pImageInf->dwImageHeight ) ;
	if ( err )
	{
		return	err ;
	}
	bool	fSrcAlpha = (pImageInf->dwBitsPerPixel == 32)
						&& (pImageInf->fdwFormatType & EIF_WITH_ALPHA) ;
	if ( fdwFlags & cvfTransparencyPalette )
	{
		if ( !fSrcAlpha )
		{
			fdwFlags &= ~cvfTransparencyPalette ;
		}
	}
	int	y ;
	for ( y = 0; y < (int) pImageInf->dwImageHeight; y ++ )
	{
		int	nWidth = pImageInf->dwImageWidth ;
		for ( int x = 0; x < nWidth; x ++ )
		{
			EGL_PALETTE	rgbColor =::eglGetPixel( pImageInf, x, y ) ;
			int	nRed = rgbColor.rgb.Red ;
			int	nGreen = rgbColor.rgb.Green ;
			int	nBlue = rgbColor.rgb.Blue ;
			int	nAlpha = rgbColor.rgba.Alpha ;
			if ( !fSrcAlpha )
			{
				nAlpha = 0xFF ;
			}
			if ( fdwFlags & cvfTransparencyPalette )
			{
				if ( nAlpha < ALPHA_BIAS )
				{
					nRed = 0 ;
					nGreen = 0 ;
					nBlue = 0 ;
				}
				else
				{
					nBlue = nBlue * 0x100 / nAlpha ;
					nGreen = nGreen * 0x100 / nAlpha ;
					nRed = nRed * 0x100 / nAlpha ;
					if ( nBlue >= 0x100 )
					{
						nBlue = 0xFF ;
					}
					if ( nGreen >= 0x100 )
					{
						nGreen = 0xFF ;
					}
					if ( nRed >= 0x100 )
					{
						nRed = 0xFF ;
					}
				}
			}
			int	c = ((nRed >> 4) << 9) | ((nGreen >> 3) << 4) | (nBlue >> 4) ;
			ESLAssert( c < m_nTableSize ) ;
			m_pcsTable[c].count ++ ;
			m_pcsTable[c].rgba[0] += nBlue ;
			m_pcsTable[c].rgba[1] += nGreen ;
			m_pcsTable[c].rgba[2] += nRed ;
			m_pcsTable[c].rgba[3] += nAlpha ;
		}
		err = OnProgress
			( ptCompileColorStatistics,
				y + 1, pImageInf->dwImageHeight ) ;
		if ( err )
		{
			return	err ;
		}
	}
	return	err ;
}

// パレット生成
//////////////////////////////////////////////////////////////////////////////
ESLError EGLColorDecreaser::BuildPalette
	( EGL_PALETTE rgbPalette[], int & nPaletteCount,
				DWORD fdwFlags, double rColorWeight )
{
	//
	// デジタル色設定
	//
	int	i ;
	for ( i = 0; i < 8; i ++ )
	{
		if ( i >= nPaletteCount )
		{
			break ;
		}
		rgbPalette[i].rgba.Blue = (i & 0x01) ? 0xFF : 0 ;
		rgbPalette[i].rgba.Red = (i & 0x02) ? 0xFF : 0 ;
		rgbPalette[i].rgba.Green = (i & 0x04) ? 0xFF : 0 ;
		rgbPalette[i].rgba.Alpha = 0xFF ;
	}
	ESLError	err ;
	err = OnProgress
		( ptBuildPalette, 0, nPaletteCount - i ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// その他の色追加
	//
	switch ( fdwFlags & cvtPaletteAlgorithmMask )
	{
	default:
	case	cvfPaletteComposition:
		if ( nPaletteCount > 8 )
		{
			int	nBasePaletteCount = 8 ;
			if ( fdwFlags & cvfPaletteHasAlpha )
			{
				rgbPalette[nBasePaletteCount].rgba.Blue = 0 ;
				rgbPalette[nBasePaletteCount].rgba.Red = 0 ;
				rgbPalette[nBasePaletteCount].rgba.Green = 0 ;
				rgbPalette[nBasePaletteCount].rgba.Alpha = 0 ;
				nBasePaletteCount ++ ;
			}
			nPaletteCount -= nBasePaletteCount ;
			//
			// 出現頻度に応じてソート
			//
			SortColorStatisticsTable( 0, m_nTableSize - 1 ) ;
			//
#if	defined(_DEBUG)
			for ( i = 1; i < m_nTableSize; i ++ )
			{
				ESLAssert( m_pcsTable[i - 1].count >= m_pcsTable[i].count ) ;
			}
#endif
			int	nGenPalettes = 0 ;
			for ( ; nGenPalettes < m_nTableSize; nGenPalettes ++ )
			{
				if ( m_pcsTable[nGenPalettes].count == 0 )
				{
					break ;
				}
			}
			int	nInitGenPalettes = nGenPalettes ;
			//
			// 近い色のパレットを結合
			//
			int	nThreshold[4] = { 8, 16, 16, 16 } ;	// green, blue, red
			int	t = 0 ;
			while ( nGenPalettes > nPaletteCount )
			{
				nThreshold[t] += 8 ;
				t = (t + 1) % 4 ;
				//
				int	thGreen = nThreshold[0] * nThreshold[0] ;
				int	thBlue = nThreshold[1] * nThreshold[1] ;
				int	thRed = nThreshold[2] * nThreshold[2] ;
				int	thAlpha = nThreshold[3] * nThreshold[3] ;
				//
				for ( i = 0; i < nGenPalettes; i ++ )
				{
					m_pcsTable[i].color = m_pcsTable[i].GetAverageColor() ;
				}
				int	j ;
				int	nPalLimit = nPaletteCount ;
				for ( i = nGenPalettes - 1; i >= nPalLimit; i -- )
				{
					const int	b = (int) m_pcsTable[i].color.rgba.Blue ;
					const int	g = (int) m_pcsTable[i].color.rgba.Green ;
					const int	r = (int) m_pcsTable[i].color.rgba.Red ;
					const int	a = (int) m_pcsTable[i].color.rgba.Alpha ;
					const int	sc = (i < nPaletteCount / 2) ?
										16 : ((i < nPaletteCount) ? 9 : 1) ;
					int			k = -1 ;
					int			nMin = 0x7FFFFFFF ;
					//
					j = nPaletteCount - 1 ;
					if ( j >= i )
					{
						j = i - 1 ;
					}
					for ( ; j >= 0; j -- )
					{
						int	db = b - (int) m_pcsTable[j].color.rgba.Blue ;
						int	dg = g - (int) m_pcsTable[j].color.rgba.Green ;
						int	dr = r - (int) m_pcsTable[j].color.rgba.Red ;
						int	da = a - (int) m_pcsTable[j].color.rgba.Alpha ;
						db *= db ;
						dg *= dg ;
						dr *= dr ;
						da *= da ;
						int	dc = db + dg * 9 + dr * 4 + da * 2 ;
						//
						if ( (sc * db <= thBlue) && (sc * dg <= thGreen)
							&& (sc * dr <= thRed) && (sc * da <= thAlpha) && (dc < nMin) )
						{
							nMin = dc ;
							k = j ;
						}
					}
					if ( k >= 0 )
					{
						if ( k > nBasePaletteCount )
						{
							m_pcsTable[k].rgba[0] += m_pcsTable[i].rgba[0] ;
							m_pcsTable[k].rgba[1] += m_pcsTable[i].rgba[1] ;
							m_pcsTable[k].rgba[2] += m_pcsTable[i].rgba[2] ;
							m_pcsTable[k].rgba[3] += m_pcsTable[i].rgba[3] ;
							m_pcsTable[k].count += m_pcsTable[i].count ;
						}
						m_pcsTable[i].count = 0 ;
						//
						if ( ++ nPalLimit >= nPaletteCount )
						{
							nPalLimit = nPaletteCount ;
						}
					}
					else
					{
						if ( -- nPalLimit <= nPaletteCount / 2 )
						{
							nPalLimit = nPaletteCount / 2 ;
						}
					}
				}
				for ( i = j = 0; i < nGenPalettes; i ++ )
				{
					if ( m_pcsTable[i].count != 0 )
					{
						m_pcsTable[j ++] = m_pcsTable[i] ;
					}
				}
				nGenPalettes = j ;
				//
				err = OnProgress
					( ptBuildPalette,
						nInitGenPalettes - nGenPalettes,
								nInitGenPalettes - nPaletteCount ) ;
				if ( err )
				{
					return	err ;
				}
			}
			//
			nPaletteCount = nGenPalettes + nBasePaletteCount ;
			//
			for ( i = 0; i < nGenPalettes; i ++ )
			{
				rgbPalette[i + nBasePaletteCount] = m_pcsTable[i].GetAverageColor( ) ;
			}
		}
		break ;

	case	cvfPaletteSelection:
		{
			//
			// 最多出現数で出現テーブルを準備
			//
			int	iMaxOccuredColor = FindMaxOccuredColor( ) ;
			if ( iMaxOccuredColor < 0 )
			{
				return	eslErrGeneral ;
			}
			int		nMaxOccuredCount = m_pcsTable[iMaxOccuredColor].count ;
			double	rCoffOccuedCount =
				128.0 / pow( (double) nMaxOccuredCount, rColorWeight * 0.5) ;
			for ( i = 0; i < m_nTableSize; i ++ )
			{
				int	count = m_pcsTable[i].count ;
				int	half = count / 2 ;
				if ( count > 0 )
				{
					m_pcsTable[i].rgba[0] = (m_pcsTable[i].rgba[0] + half) / count ;
					m_pcsTable[i].rgba[1] = (m_pcsTable[i].rgba[1] + half) / count ;
					m_pcsTable[i].rgba[2] = (m_pcsTable[i].rgba[2] + half) / count ;
					m_pcsTable[i].rgba[3] = (m_pcsTable[i].rgba[3] + half) / count ;
					ESLAssert( m_pcsTable[i].rgba[0] < 0x100 ) ;
					ESLAssert( m_pcsTable[i].rgba[1] < 0x100 ) ;
					ESLAssert( m_pcsTable[i].rgba[2] < 0x100 ) ;
					ESLAssert( m_pcsTable[i].rgba[3] < 0x100 ) ;
					if ( m_pcsTable[i].rgba[0] >= 0x100 )
					{
						m_pcsTable[i].rgba[0] = 0xFF ;
					}
					if ( m_pcsTable[i].rgba[1] >= 0x100 )
					{
						m_pcsTable[i].rgba[1] = 0xFF ;
					}
					if ( m_pcsTable[i].rgba[2] >= 0x100 )
					{
						m_pcsTable[i].rgba[2] = 0xFF ;
					}
					if ( m_pcsTable[i].rgba[3] >= 0x100 )
					{
						m_pcsTable[i].rgba[3] = 0xFF ;
					}
					m_pcsTable[i].rate =
						(float) (pow( (double) m_pcsTable[i].count,
											rColorWeight * 0.5) * rCoffOccuedCount) ;
				}
			}
			//
			// 重みでパレットを選択
			//
			for ( i = 8; i < nPaletteCount; i ++ )
			{
				int	c = GetMaxWeightColorStatistics( rgbPalette, i ) ;
				if ( m_pcsTable[c].count == 0 )
				{
					break ;
				}
				rgbPalette[i].rgba.Blue = (BYTE) m_pcsTable[c].rgba[0] ;
				rgbPalette[i].rgba.Green = (BYTE) m_pcsTable[c].rgba[1] ;
				rgbPalette[i].rgba.Red = (BYTE) m_pcsTable[c].rgba[2] ;
				rgbPalette[i].rgba.Alpha = (BYTE) m_pcsTable[c].rgba[3] ;
				m_pcsTable[c].count = 0 ;
				m_pcsTable[c].rate = 0 ;
				//
				err = OnProgress
					( ptBuildPalette, i - 8, nPaletteCount - 8 ) ;
				if ( err )
				{
					return	err ;
				}
			}
			nPaletteCount = i ;
		}
		break ;
	}
	return	eslErrSuccess ;
}

// 減色処理
//////////////////////////////////////////////////////////////////////////////
ESLError EGLColorDecreaser::DecreaseColorImage
	( DWORD fdwFlags, unsigned int nAlphaThreshold )
{
	if ( (m_pSrcImage == NULL) || (m_pDstImage == NULL) )
	{
		return	eslErrGeneral ;
	}
	if ( m_pSrcImage->dwBitsPerPixel < 24 )
	{
		return	eslErrGeneral ;
	}
	if ( m_pDstImage->pPaletteEntries == NULL )
	{
		return	eslErrGeneral ;
	}
	ESLError	err ;
	err = OnProgress
		( ptConvertImage, 0, m_pSrcImage->dwImageHeight ) ;
	if ( err )
	{
		return	err ;
	}
	//
	const int			nWidth = m_pSrcImage->dwImageWidth ;
	const int			iClipped =
							(m_pDstImage->fdwFormatType & EIF_WITH_CLIPPING) ?
										(int) m_pDstImage->dwClippedPixel : -1 ;
	const bool			fSrcAlpha =
							(m_pSrcImage->dwBitsPerPixel == 32)
								&& (m_pSrcImage->fdwFormatType & EIF_WITH_ALPHA) ;
	const bool			fDstAlpha =
							((m_pDstImage->fdwFormatType & EIF_WITH_ALPHA) != 0) ;
	unsigned long int	nAlphaSlat = ::GetTickCount() ^ 0x8AB761C5 ;
	COLOR_INT_RGBA *	prgbLineBuf = new COLOR_INT_RGBA[nWidth] ;
	COLOR_INT_RGBA *	prgbNextBuf = new COLOR_INT_RGBA[nWidth] ;
	memset( prgbLineBuf, 0,
			m_pSrcImage->dwImageWidth * sizeof(COLOR_INT_RGBA) ) ;
	//
	for ( int y = 0; y < (int) m_pSrcImage->dwImageHeight; y ++ )
	{
		COLOR_INT_RGBA	rgbLastDiff ;
		rgbLastDiff.b = 0 ;
		rgbLastDiff.g = 0 ;
		rgbLastDiff.r = 0 ;
		rgbLastDiff.a = 0 ;
		//
		memset( prgbNextBuf, 0, nWidth * sizeof(COLOR_INT_RGBA) ) ;
		//
		for ( int x = 0; x < nWidth; x ++ )
		{
			EGL_PALETTE		rgbColor =::eglGetPixel( m_pSrcImage, x, y ) ;
			PEGL_PALETTE	pPalette = m_pDstImage->pPaletteEntries ;
			int				nPaletteCount = m_pDstImage->dwPaletteCount ;
			int				iNearest = 0 ;
			int				nNearestDistance = 0xFFFFF ;
			int				nBlue = rgbColor.rgba.Blue ;
			int				nGreen = rgbColor.rgba.Green ;
			int				nRed = rgbColor.rgba.Red ;
			int				nAlpha = fSrcAlpha ? rgbColor.rgba.Alpha : 0xFF ;
			COLOR_INT_RGBA	rgbCurDiff ;
			bool			fTransparent = false ;
			//
			rgbCurDiff.b = 0 ;
			rgbCurDiff.g = 0 ;
			rgbCurDiff.r = 0 ;
			rgbCurDiff.a = 0 ;
			//
			if ( fdwFlags & cvfTransparencyPalette )
			{
				const int	nAlphaBias = ALPHA_BIAS ;
				if ( nAlpha < nAlphaBias )
				{
					fTransparent = true ;
					nBlue = 0 ;
					nGreen = 0 ;
					nRed = 0 ;
					nAlpha = 0 ;
				}
				else if ( nAlpha < 0xC0 )
				{
					if ( ((::eglGetPixel
							( m_pSrcImage, x - 1, y ).dwPixelCode == 0)
						|| (::eglGetPixel
							( m_pSrcImage, x + 1, y ).dwPixelCode == 0)
						|| (::eglGetPixel
							( m_pSrcImage, x, y - 1 ).dwPixelCode == 0)
						|| (::eglGetPixel
							( m_pSrcImage, x, y + 1 ).dwPixelCode == 0)) )
					{
						fTransparent = true ;
					}
				}
				if ( !fTransparent )
				{
					if ( !(fdwFlags & cvfNoDitherTransparency) )
					{
						int	nAlphaThreashold = 0 ;
						if ( (x ^ y) & 0x01 )
						{
							nAlphaThreashold = 0x40 ;
						}
						int	nDitherAlpha ;
						nAlphaSlat = nAlphaSlat * 9 + 0x4B1976A1 ;
						if ( nAlpha < 0x80 )
						{
							nDitherAlpha =
								nAlpha - (rgbLastDiff.a + prgbLineBuf[x].a) / 2
														- nAlphaThreashold * 4 ;
							if ( nDitherAlpha < (int) ((nAlphaSlat >> 16) & 0xFF) )
							{
								fTransparent = true ;
							}
						}
						else
						{
							nDitherAlpha =
								nAlpha - (rgbLastDiff.a + prgbLineBuf[x].a) / 2
										- (int) ((nAlphaSlat >> 16) & 0x1F) ;
							if ( nDitherAlpha < nAlphaThreashold + 0x90 )
							{
								fTransparent = true ;
							}
						}
					}
					else
					{
/*						EGL_PALETTE	rgba[4] ;
						rgba[0] = ::eglGetPixel( m_pSrcImage, x - 1, y ) ;
						rgba[1] = ::eglGetPixel( m_pSrcImage, x + 1, y ) ;
						rgba[2] = ::eglGetPixel( m_pSrcImage, x, y - 1 ) ;
						rgba[3] = ::eglGetPixel( m_pSrcImage, x, y + 1 ) ;
						int	a = (int) rgba[0].rgba.Alpha
								+ (int) rgba[1].rgba.Alpha
								+ (int) rgba[2].rgba.Alpha
								+ (int) rgba[3].rgba.Alpha ;
						if ( (((a >> 2) + nAlpha) >> 1) < nAlphaThreshold )*/
						if ( nAlpha < (int) nAlphaThreshold )
						{
							fTransparent = true ;
						}
					}
				}
				if ( fTransparent )
				{
					iNearest = iClipped ;
					//
					if ( nAlpha > nAlphaBias )
					{
						nAlpha = (nAlpha - nAlphaBias)
								* 0x100 / (0x100 - nAlphaBias) ;
					}
					else
					{
						nAlpha = 0 ;
					}
					nBlue = 0 ;
					nGreen = 0 ;
					nRed = 0 ;
				}
				else
				{
					ESLAssert( nAlpha > 1 ) ;
					nBlue = nBlue * 0x100 / nAlpha ;
					nGreen = nGreen * 0x100 / nAlpha ;
					nRed = nRed * 0x100 / nAlpha ;
					nAlpha = 0xFF ;
				}
			}
			if ( fDstAlpha )
			{
				iNearest = -1 ;
				nNearestDistance = 0xFFFFFFF ;
				if ( nAlpha < ALPHA_BIAS )
				{
					nBlue = 0 ;
					nGreen = 0 ;
					nRed = 0 ;
					nAlpha = 0 ;
					iNearest = iClipped ;
				}
				if ( iNearest < 0 )
				{
					int	db = nBlue ;
					int	dg = nGreen ;
					int	dr = nRed ;
					int	da = nAlpha ;
					if ( fdwFlags & cvfDithering )
					{
						db -= (rgbLastDiff.b + prgbLineBuf[x].b) / 2 ;
						dg -= (rgbLastDiff.g + prgbLineBuf[x].g) / 2 ;
						dr -= (rgbLastDiff.r + prgbLineBuf[x].r) / 2 ;
						da -= (rgbLastDiff.a + prgbLineBuf[x].a) / 2 ;
					}
					for ( int i = 0; i < nPaletteCount; i ++ )
					{
						if ( i != iClipped )
						{
							int	b = (int) pPalette[i].rgba.Blue - db ;
							int	g = ((int) pPalette[i].rgba.Green - dg) * 4 ;
							int	r = ((int) pPalette[i].rgba.Red - dr) * 2 ;
							int	a = ((int) pPalette[i].rgba.Alpha - da) * 2 ;
							int	gb = (int) pPalette[i].rgba.Green
									- (int) pPalette[i].rgba.Blue - (dg - db) ;
							int	rb = (int) pPalette[i].rgba.Red
									- (int) pPalette[i].rgba.Blue - (dr - db) ;
							int	ab = (int) pPalette[i].rgba.Alpha
									- (int) pPalette[i].rgba.Blue - (da - db) ;
							int	d = b * b + g * g + r * r
									+ gb * gb + rb * rb + ab * ab ;
							if ( nNearestDistance >= d )
							{
								iNearest = i ;
								nNearestDistance = d ;
							}
						}
					}
				}
				if ( iNearest != iClipped )
				{
					rgbCurDiff.b += (int) pPalette[iNearest].rgba.Blue ;
					rgbCurDiff.g += (int) pPalette[iNearest].rgba.Green ;
					rgbCurDiff.r += (int) pPalette[iNearest].rgba.Red ;
					rgbCurDiff.a += (int) pPalette[iNearest].rgba.Alpha ;
				}
			}
			else if ( !fTransparent )
			{
				int	db = nBlue ;
				int	dg = nGreen ;
				int	dr = nRed ;
				if ( fdwFlags & cvfDithering )
				{
					db -= (rgbLastDiff.b + prgbLineBuf[x].b) / 2 ;
					dg -= (rgbLastDiff.g + prgbLineBuf[x].g) / 2 ;
					dr -= (rgbLastDiff.r + prgbLineBuf[x].r) / 2 ;
				}
				for ( int i = 0; i < nPaletteCount; i ++ )
				{
					if ( i != iClipped )
					{
						int	b = (int) pPalette[i].rgb.Blue - db ;
						int	g = ((int) pPalette[i].rgb.Green - dg) * 4 ;
						int	r = ((int) pPalette[i].rgb.Red - dr) * 2 ;
						int	gb = (int) pPalette[i].rgb.Green
								- (int) pPalette[i].rgb.Blue - (dg - db) ;
						int	rb = (int) pPalette[i].rgb.Red
								- (int) pPalette[i].rgb.Blue - (dr - db) ;
						int	d = b * b + g * g + r * r + gb * gb + rb * rb ;
						if ( nNearestDistance >= d )
						{
							iNearest = i ;
							nNearestDistance = d ;
						}
					}
				}
				rgbCurDiff.b += (int) pPalette[iNearest].rgb.Blue ;
				rgbCurDiff.g += (int) pPalette[iNearest].rgb.Green ;
				rgbCurDiff.r += (int) pPalette[iNearest].rgb.Red ;
				rgbCurDiff.a += 0xFF ;
			}
			rgbCurDiff.b -= nBlue ;
			rgbCurDiff.g -= nGreen ;
			rgbCurDiff.r -= nRed ;
			rgbCurDiff.a -= nAlpha ;
			//
			rgbLastDiff += prgbLineBuf[x] ;
			rgbLastDiff *= 2 ;
			rgbLastDiff /= 3 ;
			rgbCurDiff += rgbLastDiff ;
			rgbCurDiff /= 3 ;
			//
			rgbLastDiff = rgbCurDiff ;
			prgbNextBuf[x] += rgbCurDiff ;
			//
			rgbCurDiff /= 2 ;
			//
			if ( x > 0 )
			{
				prgbNextBuf[x - 1] += rgbCurDiff ;
			}
			if ( x + 1 < nWidth )
			{
				prgbNextBuf[x + 1] += rgbCurDiff ;
			}
			//
			::eglSetPixel( m_pDstImage, x, y, EGLPalette( iNearest ) ) ;
		}
		err = OnProgress
			( ptConvertImage, y + 1, m_pSrcImage->dwImageHeight ) ;
		if ( err )
		{
			break ;
		}
		COLOR_INT_RGBA *	prgbTemp = prgbNextBuf ;
		prgbNextBuf = prgbLineBuf ;
		prgbLineBuf = prgbTemp ;
	}
	delete	[] prgbLineBuf ;
	delete	[] prgbNextBuf ;
	return	err ;
}

// パレットテーブルをソートする
//////////////////////////////////////////////////////////////////////////////
void EGLColorDecreaser::SortColorStatisticsTable( int nFirst, int nEnd )
{
	if ( nFirst >= nEnd )
	{
		return ;
	}
	COLOR_STATISTICS	csTemp ;
	if ( nFirst + 1 == nEnd )
	{
		if ( m_pcsTable[nFirst].count < m_pcsTable[nEnd].count )
		{
			csTemp = m_pcsTable[nFirst] ;
			m_pcsTable[nFirst] = m_pcsTable[nEnd] ;
			m_pcsTable[nEnd] = csTemp ;
			return ;
		}
	}
	//
	// ２つの区間に分割
	//
	int	iLeft = nFirst ;
	int	iRight = nEnd ;
	//
	csTemp = m_pcsTable[nEnd] ;
	//
	do
	{
		for ( ; ; )
		{
			if ( m_pcsTable[iLeft].count < csTemp.count )
			{
				m_pcsTable[iRight --] = m_pcsTable[iLeft] ;
				break ;
			}
			if ( ++ iLeft >= iRight )
			{
				break ;
			}
		}
		if ( iLeft >= iRight )
		{
			break ;
		}
		for ( ; ; )
		{
			if ( csTemp.count < m_pcsTable[iRight].count )
			{
				m_pcsTable[iLeft ++] = m_pcsTable[iRight] ;
				break ;
			}
			if ( iLeft >= -- iRight )
			{
				break ;
			}
		}
	}
	while ( iLeft < iRight ) ;
	//
	ESLAssert( iLeft == iRight ) ;
	m_pcsTable[iLeft] = csTemp ;
	//
	// 前半区間をソート
	//
	if ( iLeft - nFirst >= 2 )
	{
		SortColorStatisticsTable( nFirst, iLeft - 1 ) ;
	}
	//
	// 後半区間をソート
	//
	if ( nEnd - iLeft >= 2 )
	{
		SortColorStatisticsTable( iLeft + 1, nEnd ) ;
	}
}

// 最も出現数の大きいテーブル要素を検索する
//////////////////////////////////////////////////////////////////////////////
int EGLColorDecreaser::FindMaxOccuredColor( void ) const
{
	if ( m_pcsTable == NULL )
	{
		return	-1 ;
	}
	int	iFind = 0 ;
	int	nMax = m_pcsTable[0].count ;
	for ( int i = 1; i < m_nTableSize; i ++ )
	{
		if ( nMax < m_pcsTable[i].count )
		{
			iFind = i ;
			nMax = m_pcsTable[i].count ;
		}
	}
	return	iFind ;
}

// 最も重みのあるテーブル要素を検索する
//////////////////////////////////////////////////////////////////////////////
int EGLColorDecreaser::GetMaxWeightColorStatistics
	( const EGL_PALETTE rgbPalette[], int nPaletteCount ) const
{
	int		iColor = 0 ;
	double	rMaxWeight = 0 ;
	for ( int i = 0; i < m_nTableSize; i ++ )
	{
		if ( m_pcsTable[i].count != 0 )
		{
			double	rWeight =
				CalcColorStatisticsWeight
					( m_pcsTable[i], rgbPalette, nPaletteCount ) ;
			if ( rMaxWeight < rWeight )
			{
				rMaxWeight = rWeight ;
				iColor = i ;
			}
		}
	}
	return	iColor ;
}

// 色の統計情報の重みを計算する
//////////////////////////////////////////////////////////////////////////////
double EGLColorDecreaser::CalcColorStatisticsWeight
	( const COLOR_STATISTICS & csColor,
		const EGL_PALETTE rgbPalette[], int nPaletteCount )
{
	double	d = 0 ;
	for ( int i = 0; i < nPaletteCount; i ++ )
	{
		int	c[4] ;
		c[0] = (int) csColor.rgba[0] - (int) rgbPalette[i].rgba.Blue ;
		c[1] = (int) csColor.rgba[1] - (int) rgbPalette[i].rgba.Green ;
		c[2] = (int) csColor.rgba[2] - (int) rgbPalette[i].rgba.Red ;
		c[3] = (int) csColor.rgba[3] - (int) rgbPalette[i].rgba.Alpha ;
		c[1] *= 3 ;
		c[2] *= 2 ;
		d += sqrt( (double) (c[0] * c[0] + c[1] * c[1]
								+ c[2] * c[2] + c[3] * c[3]) ) ;
	}
	return	d / nPaletteCount * csColor.rate ;
}

// 進行状況
//////////////////////////////////////////////////////////////////////////////
ESLError EGLColorDecreaser::OnProgress
	( int nProcess, int nCurrent, int nTotal )
{
#if	defined(_DEBUG)
	switch ( nProcess )
	{
	case	ptCompileColorStatistics:
		ESLTrace( "Compile color statistics %d / %d\n", nCurrent, nTotal ) ;
		break ;
	case	ptBuildPalette:
		ESLTrace( "Build palette %d / %d\n", nCurrent, nTotal ) ;
		break ;
	case	ptConvertImage:
		ESLTrace( "Convert image %d / %d\n", nCurrent, nTotal ) ;
		break ;
	}
#endif
	return	eslErrSuccess ;
}

