
#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl_erisa_lib.h>
#include <sakuragl/sgl2d_image.h>
#include <sakuragl/sgl2d/sgl_image_encoder.h>

#if	defined(__PLATFORM_WINDOWS__)
#include <sakuragl/sgl_gdiplus_image_decoder.h>
#endif

using namespace SSystem ;
using namespace SakuraGL ;
using namespace ERISA ;


//////////////////////////////////////////////////////////////////////////////
// 画像エンコーダーインターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLImageEncoderInterface, SObject )

// MIME 判定
//////////////////////////////////////////////////////////////////////////////
bool SGLImageEncoderInterface::IsMatchableMIMEType( const wchar_t * pszMIME )
{
	return	false ;
}


//////////////////////////////////////////////////////////////////////////////
// 画像エンコーダー管理
//////////////////////////////////////////////////////////////////////////////

// 画像エンコーダー配列
//////////////////////////////////////////////////////////////////////////////
ESL_DLL_DECL( SSystem::SObjectArray<SGLImageEncoderInterface> *
				SGLImageEncoderManager::m_arrayImageEncoder = NULL ) ;

// 初期化
//////////////////////////////////////////////////////////////////////////////
void SGLImageEncoderManager::Initialzie( void )
{
	if ( m_arrayImageEncoder == NULL )
	{
		m_arrayImageEncoder =
			new SSystem::SObjectArray<SGLImageEncoderInterface> ;
		m_arrayImageEncoder->Add( new SGLERImageEncoder ) ;
		m_arrayImageEncoder->Add( new SGLWindowsBitmapEncoder ) ;

		#if	defined(__PLATFORM_WINDOWS__)
		SGLGdiplusImageEncoder *	pGdiplus = new SGLGdiplusImageEncoder ;
		if ( pGdiplus->IsGDIplusInstalled() )
		{
			m_arrayImageEncoder->Add( pGdiplus ) ;
		}
		else
		{
			delete	pGdiplus ;
		}
		#endif
	}
}

// 終了
//////////////////////////////////////////////////////////////////////////////
void SGLImageEncoderManager::Finalize( void )
{
	delete	m_arrayImageEncoder ;
	m_arrayImageEncoder = NULL ;
}

// エンコーダー追加登録
//////////////////////////////////////////////////////////////////////////////
void SGLImageEncoderManager::RegisterEncoder( SGLImageEncoderInterface * pDecoder )
{
	SSystem::QuickLock() ;
	if ( m_arrayImageEncoder == NULL )
	{
		m_arrayImageEncoder =
			new SSystem::SObjectArray<SGLImageEncoderInterface> ;
	}
	m_arrayImageEncoder->Add( pDecoder ) ;
	SSystem::QuickUnlock() ;
}

// 拡張子が合致するエンコーダー取得
//////////////////////////////////////////////////////////////////////////////
SGLImageEncoderInterface *
	SGLImageEncoderManager::FindEncoderAsExt
				( const wchar_t * pszExt, SString& strMIME )
{
	SSystem::QuickLock() ;
	if ( m_arrayImageEncoder == NULL )
	{
		SSystem::QuickUnlock() ;
		return	NULL ;
	}
	const size_t	nCount = m_arrayImageEncoder->GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		SGLImageEncoderInterface *
					pEncoder = m_arrayImageEncoder->GetAt( i ) ;
		if ( pEncoder != NULL )
		{
			if ( pEncoder->IsMatchableFileExtension( pszExt, strMIME ) )
			{
				SSystem::QuickUnlock() ;
				return	pEncoder ;
			}
		}
	}
	SSystem::QuickUnlock() ;
	return	NULL ;
}

// MIME が合致するエンコーダー取得
//////////////////////////////////////////////////////////////////////////////
SGLImageEncoderInterface *
	SGLImageEncoderManager::FindEncoderAsMIME( const wchar_t * pszMIME )
{
	SSystem::QuickLock() ;
	if ( m_arrayImageEncoder == NULL )
	{
		SSystem::QuickUnlock() ;
		return	NULL ;
	}
	const size_t	nCount = m_arrayImageEncoder->GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		SGLImageEncoderInterface *
					pEncoder = m_arrayImageEncoder->GetAt( i ) ;
		if ( pEncoder != NULL )
		{
			if ( pEncoder->IsMatchableMIMEType( pszMIME ) )
			{
				SSystem::QuickUnlock() ;
				return	pEncoder ;
			}
		}
	}
	SSystem::QuickUnlock() ;
	return	NULL ;
}

// 画像書き出し
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageEncoderManager::WriteImage
	( SSystem::SFileInterface & file,
		SGLImageObject & image, const wchar_t * pszMIME,
		const SGLImageEncoderInterface::Options * pOpt )
{
	if ( pszMIME == NULL )
	{
		pszMIME = L"image/x-eri" ;
	}
	SGLImageEncoderInterface *	pEncoder = FindEncoderAsMIME( pszMIME ) ;
	if ( pEncoder == NULL )
	{
		return	sglErrFailed ;
	}
	SFileInterface *	pFile = &file ;
	SSmartBuffer		bufFile ;
	if ( !file.IsSeekable() )
	{
		pFile = &bufFile ;
	}
	SGLError	err =
		pEncoder->WriteImage( *pFile, image, pszMIME, pOpt ) ;
	if ( err )
	{
		return	err ;
	}
	if ( !file.IsSeekable() )
	{
		bufFile.WriteToStream( file ) ;
	}
	return	err ;
}


//////////////////////////////////////////////////////////////////////////////
// ERI 画像エンコーダー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SGLERImageEncoder, SGLImageEncoderInterface )

// ファイル拡張子判定
//////////////////////////////////////////////////////////////////////////////
bool SGLERImageEncoder::IsMatchableFileExtension( const wchar_t * pszExt, SString& strMIME )
{
	if ( SString::CompareNoCase( pszExt, L"eri" ) == 0 )
	{
		strMIME = L"image/x-eri" ;
		return	true ;
	}
	return	false ;
}

// MIME 判定
//////////////////////////////////////////////////////////////////////////////
bool SGLERImageEncoder::IsMatchableMIMEType( const wchar_t * pszMIME )
{
	return	(SString::CompareNoCase( pszMIME, L"image/x-eri" ) == 0)
			|| (SString::CompareNoCase( pszMIME, L"image/x-erina" ) == 0)
			|| (SString::CompareNoCase( pszMIME, L"image/x-erisa" ) == 0) ;
}

// 画像書き出し
//////////////////////////////////////////////////////////////////////////////
SGLError SGLERImageEncoder::WriteImage
	( SSystem::SFileInterface & file,
		SGLImageObject & image,
		const wchar_t * pszMIME,
		const SGLImageEncoderInterface::Options * pOpt )
{
	ERISA::SGLMediaFileWriter	mfw ;
	//
	// 圧縮パラメータを生成する
	//
	ERISA::ERI_INFO_HEADER	eih ;
	eslFillMemory( &eih, 0, sizeof(ERISA::ERI_INFO_HEADER) ) ;
	if ( SString::CompareNoCase( pszMIME, L"image/x-eri" ) == 0 )
	{
		eih.dwVersion = ERISA::eriFileStandardVersinon ;
		eih.fdwTransformation = ERISA::eriTransformationLossless ;
		eih.dwArchitecture = ERISA::eriRunlengthGamma ;
	}
	else
	{
		eih.dwVersion = ERISA::eriFileEnhancedVersinon ;
		eih.fdwTransformation = ERISA::eriTransformationLossless ;
		if ( SString::CompareNoCase( pszMIME, L"image/x-erina" ) == 0 )
		{
			eih.dwArchitecture = ERISA::eriRunlengthHuffman ;
		}
		else
		{
			eih.dwArchitecture = ERISA::erisaNemesisCode ;
		}
	}
	SGLImageInfo	imginf ;
	image.GetImageInfo( imginf ) ;
	//
	eih.fdwFormatType = (DWORD) imginf.format ;
	eih.nImageWidth = (SDWORD) imginf.width ;
	eih.nImageHeight = - (SDWORD) imginf.height ;
	eih.dwBitsPerPixel = (DWORD) imginf.depth ;
	eih.dwClippedPixel = (DWORD) imginf.colorClip ;
	eih.dwBlockingDegree = 3 ;
	//
	SGLImageEncoder::Parameter	prmEnc ;
	if ( (pOpt != NULL)
		&& (pOpt->nFlags & SGLImageEncoderInterface::optionQuality)
		&& (pOpt->nQuality < 0x100) )
	{
		int	ipp = (int) (pOpt->nQuality
						* SGLImageEncoder::ppBelowQuality / 0x100) ;
		int	nDelta = pOpt->nQuality
						- ipp * 0x100 / SGLImageEncoder::ppBelowQuality ;
		nDelta *= SGLImageEncoder::ppBelowQuality ;
		if ( nDelta > 0x100 )
		{
			nDelta = 0x100 ;
		}
		nDelta = 0x100 - nDelta ;
		//
		SGLImageEncoder::PresetParameter	pp =
			(SGLImageEncoder::PresetParameter)
				(SGLImageEncoder::ppBelowQuality - ipp - 1) ;
		ESLAssert( ((int) pp >= 0) && (pp < SGLImageEncoder::ppBelowQuality) ) ;
		prmEnc.LoadPresetParam( pp, eih ) ;
		//
		if ( nDelta > 0 )
		{
			SGLImageEncoder::Parameter	prmEnc2 ;
			ERISA::ERI_INFO_HEADER		eih2 ;
			prmEnc2.LoadPresetParam
				( (SGLImageEncoder::PresetParameter) (pp + 1), eih2 ) ;
			//
			float32_t	t = (float32_t) nDelta / 0x100 ;
			prmEnc.m_fpYScaleDC = prmEnc.m_fpYScaleDC * (1.0F - t)
										+ prmEnc2.m_fpYScaleDC * t ;
			prmEnc.m_fpCScaleDC = prmEnc.m_fpCScaleDC * (1.0F - t)
										+ prmEnc2.m_fpCScaleDC * t ;
			prmEnc.m_fpYScaleLow = prmEnc.m_fpYScaleLow * (1.0F - t)
										+ prmEnc2.m_fpYScaleLow * t ;
			prmEnc.m_fpCScaleLow = prmEnc.m_fpCScaleLow * (1.0F - t)
										+ prmEnc2.m_fpCScaleLow * t ;
			prmEnc.m_fpYScaleHigh = prmEnc.m_fpYScaleHigh * (1.0F - t)
										+ prmEnc2.m_fpYScaleHigh * t ;
			prmEnc.m_fpCScaleHigh = prmEnc.m_fpCScaleHigh * (1.0F - t)
										+ prmEnc2.m_fpCScaleHigh * t ;
		}
//		prmEnc.m_nFlags &= ~SGLImageEncoder::pfUseLoopFilter ;
		mfw.SetImageCompressionParameter( prmEnc ) ;
	}
	//
	// ファイルを開く
	//
	SError	err =
		mfw.OpenMediaFile
			( &file, SGLMediaFileWriter::fidImage, false ) ;
	if ( err )
	{
		return	(SGLError) err ;
	}
	do
	{
		//
		// ファイルヘッダ
		//
		err = mfw.BeginFileHeader( 0, 0 ) ;
		if ( err )
		{
			break ;
		}
		err = mfw.WriteEriInfoHeader( eih ) ;
		if ( err )
		{
			break ;
		}
		SGLMediaFile::STagInfo	taginf ;
		taginf.AddTag
			( SGLMediaFile::tagHotSpot,
				SString( imginf.ptOrigin.x )
					+ L"," + SString( imginf.ptOrigin.y ) ) ;
		SString	strDesc ;
		taginf.FormatTagInfo( strDesc ) ;
		err = mfw.WriteDescription( strDesc ) ;
		if ( err )
		{
			break ;
		}
		size_t	nSeqLen = image.GetSequenceLength() ;
		if ( nSeqLen > 1 )
		{
			SArray<uint32_t>	aSeqIndex ;
			nSeqLen = image.GetSequenceTable
						( aSeqIndex.GetArray( nSeqLen ), nSeqLen ) ;
			aSeqIndex.FinishArray() ;
			//
			SArray<SGLMediaFile::SEQUENCE_DELTA>	aSeqDelta ;
			SGLMediaFile::SEQUENCE_DELTA *
						pSeqDelta = aSeqDelta.GetArray( nSeqLen ) ;
			int64_t		nTotalTime = image.GetTotalTime() ;
			uint32_t	nLastTime = 0 ;
			for ( size_t i = 0; i < nSeqLen; i ++ )
			{
				pSeqDelta[i].nFrame = aSeqIndex.At(i) ;
				pSeqDelta[i].nDuration =
					(uint32_t) (nTotalTime * (i + 1) / nSeqLen) - nLastTime ;
				nLastTime += pSeqDelta[i].nDuration ;
			}
			err = mfw.WriteSequenceTable( pSeqDelta, nSeqLen ) ;
			aSeqDelta.FinishArray() ;
			if ( err )
			{
				break ;
			}
		}
		mfw.EndFileHeader() ;
		//
		// 画像データを書き出す
		//
		err = mfw.BeginStream() ;
		if ( err )
		{
			break ;
		}
		int		iSaveSide ;
		size_t	iSaveFrame = image.GetSelectedFrame( &iSaveSide ) ;
		//
		SGLPalette	palTable[0x100] ;
		size_t	nFrames = image.GetFrameCount() ;
		for ( size_t i = 0; i < nFrames; i ++ )
		{
			image.SelectFrame( i ) ;
			//
			size_t	nPalCount =
						image.GetPaletteTable( &palTable[0], 0x100 ) ;
			if ( nPalCount > 0 )
			{
				mfw.WritePaletteTable( &palTable[0], nPalCount ) ;
			}
			mfw.WriteImageData( image ) ;
		}
		image.SelectFrame( iSaveFrame, iSaveSide ) ;
		//
		err = mfw.EndStream( (uint32_t) image.GetTotalTime() ) ;
	}
	while ( false ) ;
	//
	mfw.Close() ;
	return	(SGLError) err ;
}


//////////////////////////////////////////////////////////////////////////////
// Windows Bitmap 画像エンコーダー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLWindowsBitmapEncoder, SGLImageEncoderInterface )

// ファイル拡張子判定
//////////////////////////////////////////////////////////////////////////////
bool SGLWindowsBitmapEncoder::IsMatchableFileExtension
		( const wchar_t * pszExt, SString& strMIME )
{
	if ( SString::CompareNoCase( pszExt, L"bmp" ) == 0 )
	{
		strMIME = L"image/bmp" ;
		return	true ;
	}
	return	false ;
}

// MIME 判定
//////////////////////////////////////////////////////////////////////////////
bool SGLWindowsBitmapEncoder::IsMatchableMIMEType( const wchar_t * pszMIME )
{
	return	(SString::CompareNoCase( pszMIME, L"image/bmp" ) == 0) ;
}

// 画像書き出し
//////////////////////////////////////////////////////////////////////////////
SGLError SGLWindowsBitmapEncoder::WriteImage
	( SSystem::SFileInterface & file,
		SGLImageObject & image, const wchar_t * pszMIME,
		const SGLImageEncoderInterface::Options * pOpt )
{
	int64_t	fposStart = file.GetPosition() ;
	//
	// 画像情報取得
	//
	SGLImageInfo	imginf ;
	if ( image.GetImageInfo( imginf ) )
	{
		return	sglErrFailed ;
	}
	RGBQUAD	tblPalette[0x100] ;
	size_t	nPalLength =
		image.GetPaletteTable
			( (SGLPalette*) &tblPalette[0], 0x100 ) ;
	size_t	nLineBytes =
				(imginf.width * imginf.pitchPixel + 0x03) & ~0x03 ;
	//
	// ファイルヘッダ書き出し
	//
	BYTE	bytSig[2] ;
	bytSig[0] = (BYTE) 'B' ;
	bytSig[1] = (BYTE) 'M' ;
	if ( file.Write( &bytSig[0], 2 ) < 2 )
	{
		return	sglErrFailed ;
	}
	BITMAPFILEHEADER	bmFileHeader ;
	bmFileHeader.bfReserved1 = 0 ;
	bmFileHeader.bfReserved2 = 0 ;
	bmFileHeader.bfOffBits =
		(DWORD) fposStart + 2
			+ sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) ;
	bmFileHeader.bfSize =
		bmFileHeader.bfOffBits + (DWORD) (nLineBytes * imginf.height) ;
	//
	if ( file.Write
		( &bmFileHeader, sizeof(BITMAPFILEHEADER) )
								< sizeof(BITMAPFILEHEADER) )
	{
		return	sglErrFailed ;
	}
	//
	// 画像情報ヘッダ書き出し
	//
	BITMAPINFOHEADER	bmInfoHeader ;
	memset( &bmInfoHeader, 0, sizeof(BITMAPINFOHEADER) ) ;
	bmInfoHeader.biSize = sizeof(BITMAPINFOHEADER) ;
	bmInfoHeader.biWidth = (DWORD) imginf.width ;
	bmInfoHeader.biHeight = (DWORD) imginf.height ;
	bmInfoHeader.biPlanes = 1 ;
	bmInfoHeader.biBitCount = (WORD) imginf.depth ;
	bmInfoHeader.biSizeImage = (DWORD) (nLineBytes * imginf.height) ;
	bmInfoHeader.biClrUsed = (DWORD) nPalLength ;
	//
	if ( file.Write
		( &bmInfoHeader, sizeof(BITMAPINFOHEADER) )
								< sizeof(BITMAPINFOHEADER) )
	{
		return	sglErrFailed ;
	}
	//
	// パレットテーブル書き出し
	//
	if ( nPalLength != 0 )
	{
		if ( file.Write
			( &tblPalette[0],
				nPalLength * sizeof(RGBQUAD) )
						< nPalLength * sizeof(RGBQUAD) )
		{
			return	sglErrFailed ;
		}
	}
	//
	// 画像データ書き出し
	//
	SGLImageInfo	infFrame ;
	uint8_t *		ptrFrame =
		image.LockBuffer( infFrame, SGLImageObject::lockRead ) ;
	if ( ptrFrame == NULL )
	{
		return	sglErrFailed ;
	}
	DWORD	dwPadZero = 0 ;
	size_t	nLineSize = imginf.width * imginf.pitchPixel ;
	size_t	nPadSize = nLineBytes - nLineSize ;
	ptrFrame += infFrame.pitchLine * (infFrame.height - 1) ;
	for ( size_t y = 0; y < infFrame.height; y ++ )
	{
		if ( file.Write( ptrFrame, nLineSize ) < nLineSize )
		{
			break ;
		}
		if ( (nPadSize != 0)
			&& (file.Write( &dwPadZero, nPadSize ) < nPadSize) )
		{
			break ;
		}
		ptrFrame -= infFrame.pitchLine ;
	}
	image.UnlockBuffer( SGLImageObject::lockRead ) ;
	return	sglErrSuccess ;
}

