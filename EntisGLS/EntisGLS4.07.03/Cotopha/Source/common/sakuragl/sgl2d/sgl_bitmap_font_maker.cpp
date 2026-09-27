
#include <sakuragl/sakuragl.h>
#include <sakura/ssys_smart_buffer.h>
#include <sakuragl/sgl2d_image.h>
#include <sakuragl/sgl2d/sgl_image_filter.h>
#include <sakuragl/sgl2d/sgl_bitmap_font_maker.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// 画像フォントファイル作成
//////////////////////////////////////////////////////////////////////////////

const wchar_t *	SGLBitmapFontMaker::m_pwszDefNoRotate4V =
					L"「」『』（）〔〕｛｝＜＞≪≫《》【】；：｜―ー－" ;
const wchar_t *	SGLBitmapFontMaker::m_pwszDefGeminate =
					L"ぁぃぅぇぉっゃゅょァィゥェォッャュョ" ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLBitmapFontMaker, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLBitmapFontMaker::SGLBitmapFontMaker( void )
{
	m_flagForVertical = false ;
}

// フォントファイルを作成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLBitmapFontMaker::MakeFontFile
	( const wchar_t * pwszFilePath, Listener * pListener )
{
	//
	// ファイルを開く
	//
	SFileInterface *	pFile =
		SFileOpener::DefaultNewOpenFile
				( pwszFilePath, SFileOpener::modeCreate ) ;
	if ( pFile == NULL )
	{
		return	sglErrFailed ;
	}
	//
	// ファイルヘッダ書き出し
	//
	SChunkFile::FILE_HEADER	fhHeader ;
	fhHeader.SetHeaderInfo
		( SChunkFile::fidBitmapFont, "EntisGLS4 bitmap font file" ) ;
	//
	SChunkFile	cf ;
	if ( cf.OpenChunkFile
		( pFile, true, SFileOpener::modeCreate, &fhHeader ) )
	{
		return	sglErrFailed ;
	}
	//
	// 順次フォントをラスタライズ
	//
	SSmartBuffer	sbufGrph ;
	SByteBuffer		bufRasterized ;
	SByteBuffer		bufOversampling ;
	SByteBuffer		bufRotation ;
	for ( size_t iFont = 0; iFont < m_listFontSet.GetLength(); iFont ++ )
	{
		FontInfo *	pFontInfo = m_listFontSet.GetAt( iFont ) ;
		if ( pFontInfo == NULL )
		{
			continue ;
		}
		if ( pFontInfo->m_strFont.IsEmpty() )
		{
			pFontInfo->m_strFont = m_strOrgFont ;
		}
		SGLFontStyle	style ;
		SGLFont			font ;
		int				nOversamplingScale = 0 ;
		style.nStyles = pFontInfo->m_nStyle ;
		style.nSize = pFontInfo->m_nSize ;
		style.pszFace = pFontInfo->m_strFont ;
		if ( pFontInfo->m_nOversampling >= 2 )
		{
			style.nSize *= 2 ;
			nOversamplingScale ++ ;
			//
			if ( pFontInfo->m_nOversampling >= 4 )
			{
				style.nSize *= 2 ;
				nOversamplingScale ++ ;
			}
		}
		font.SetStyle( style ) ;
		//
		SGLBitmapFontLoader::FontSet	fontset ;
		SGLFontMetrics					metrics ;
		fontset.m_style.sizeFont = pFontInfo->m_nAsSize ;
		fontset.m_style.stylesFont = style.nStyles ;
		fontset.m_style.formatBitmap = formatImageGray ;
		fontset.m_style.depthBitmap = 8 ;
		fontset.m_style.countCharacters = 0 ;
		//
		for ( size_t chr = 0; chr < 0x10000; chr ++ )
		{
			if ( pFontInfo->m_flagJISChar && (chr >= 0x80) )
			{
				DWORD	dwJIS = ESLCharset::JISCodeFromUnicode( (WORD) chr ) ;
				if ( dwJIS == ESLCharset::JiscodeError )
				{
					continue ;
				}
			}
			if ( pFontInfo->m_flag7bitChar && (chr >= 0x80) )
			{
				if ( pFontInfo->m_strEx7bitChar.Find( (wchar_t) chr ) < 0 )
				{
					continue ;
				}
			}
			const wchar_t	wch = (wchar_t) chr ;
			const wchar_t *	pwszExceptFont = GetExceptionFont( wch ) ;
			if ( pwszExceptFont != NULL )
			{
				SGLFontStyle	styleExcept ;
				styleExcept.nStyles = style.nStyles ;
				styleExcept.nSize = style.nSize ;
				styleExcept.pszFace = pwszExceptFont ;
				font.SetStyle( styleExcept ) ;
			}
			if ( font.GetMetrics( NULL, 0, metrics, wch ) )
			{
				continue ;
			}
			//
			// ラスタライズ
			//
			size_t	byteRasterized =
						metrics.rctExterior.w * metrics.rctExterior.h ;
			byteRasterized = (byteRasterized + 0x03) & ~0x03 ;
			bufRasterized.SetLength( byteRasterized ) ;
			font.GetMetrics
				( bufRasterized.GetArray(), byteRasterized, metrics, wch ) ;
			bufRasterized.FinishArray() ;
			//
			if ( nOversamplingScale > 0 )
			{
				const int	maskOdd = (1 << nOversamplingScale) - 1 ;
				if ( (metrics.rctExterior.x & maskOdd)
					|| (metrics.rctExterior.y & maskOdd) )
				{
					//
					// 端数位置の正規化
					//
					int	xOdd = (metrics.rctExterior.x & maskOdd) ;
					int	yOdd = (metrics.rctExterior.y & maskOdd) ;
					int	xRight =
							(metrics.rctExterior.x
								+ metrics.rctExterior.w
											+ maskOdd) & ~maskOdd ;
					int	yBottom =
							(metrics.rctExterior.y
								+ metrics.rctExterior.h
											+ maskOdd) & ~maskOdd ;
					int	nWidth = xRight - (metrics.rctExterior.x - xOdd) ;
					int	nHeight = yBottom - (metrics.rctExterior.y - yOdd) ;
					//
					bufOversampling = bufRasterized ;
					//
					byteRasterized = (size_t) (nWidth * nHeight) ;
					bufRasterized.SetLength( byteRasterized ) ;
					//
					SGLImageBuffer	imgDst, imgSrc ;
					imgSrc.format = formatImageGray ;
					imgSrc.depth = 8 ;
					imgSrc.width = metrics.rctExterior.w ;
					imgSrc.height = metrics.rctExterior.h ;
					imgSrc.pitchPixel = 1 ;
					imgSrc.pitchLine = metrics.rctExterior.w ;
					imgSrc.ptrBuffer = bufOversampling.GetArray() ;
					//
					imgDst.format = formatImageGray ;
					imgDst.depth = 8 ;
					imgDst.width = (uint32_t) nWidth ;
					imgDst.height = (uint32_t) nHeight ;
					imgDst.pitchPixel = 1 ;
					imgDst.pitchLine = (int32_t) nWidth ;
					imgDst.ptrBuffer = bufRasterized.GetArray() ;
					//
					eslFillMemory( imgDst.ptrBuffer, 0, byteRasterized ) ;
					sglCopyImageBuffer( imgDst, imgSrc, xOdd, yOdd ) ;
					//
					bufOversampling.FinishArray() ;
					bufRasterized.FinishArray() ;
					//
					metrics.rctExterior.x &= ~maskOdd ;
					metrics.rctExterior.y &= ~maskOdd ;
					metrics.rctExterior.w = (int32_t) nWidth ;
					metrics.rctExterior.h = (int32_t) nHeight ;
				}
				for ( int iOS = 0; iOS < nOversamplingScale; iOS ++ )
				{
					//
					// オーバーサンプリング
					//
					SGLImageBuffer	imgDst, imgSrc ;
					imgSrc.format = formatImageGray ;
					imgSrc.depth = 8 ;
					imgSrc.width = metrics.rctExterior.w ;
					imgSrc.height = metrics.rctExterior.h ;
					imgSrc.pitchPixel = 1 ;
					imgSrc.pitchLine = metrics.rctExterior.w ;
					imgSrc.ptrBuffer = bufRasterized.GetArray() ;
					//
					imgDst.format = formatImageGray ;
					imgDst.depth = 8 ;
					imgDst.width = (metrics.rctExterior.w + 1) >> 1 ;
					imgDst.height = (metrics.rctExterior.h + 1) >> 1 ;
					imgDst.pitchPixel = 1 ;
					imgDst.pitchLine = imgDst.width ;
					imgDst.ptrBuffer =
						bufOversampling.GetArray( imgDst.width * imgDst.height ) ;
					//
					sglEnlargeHalfImageBuffer( imgDst, imgSrc ) ;
					//
					bufRasterized.FinishArray() ;
					bufOversampling.FinishArray() ;
					//
					metrics.rctExterior.x /= 2 ;
					metrics.rctExterior.y /= 2 ;
					metrics.rctExterior.w = imgDst.width ;
					metrics.rctExterior.h = imgDst.height ;
					//
					bufRasterized = bufOversampling ;
					byteRasterized = imgDst.width * imgDst.height ;
				}
				const int	nOdd = (1 << nOversamplingScale) >> 1 ;
				metrics.nAscent = (metrics.nAscent + nOdd) >> nOversamplingScale ;
				metrics.nDescent = (metrics.nDescent + nOdd) >> nOversamplingScale ;
				metrics.nLeading = (metrics.nLeading + nOdd) >> nOversamplingScale ;
				metrics.nWidth = (metrics.nWidth + nOdd) >> nOversamplingScale ;
				metrics.nHeight = (metrics.nHeight + nOdd) >> nOversamplingScale ;
			}
			if ( pFontInfo->m_nGamma != 0x100 )
			{
				//
				// ガンマ補正
				//
				uint8_t	tone[0x100] ;
				sglMakeGammaToneFilter( &tone[0], pFontInfo->m_nGamma ) ;
				//
				uint8_t *	pbytRasterized = bufRasterized.GetArray() ;
				for ( size_t i = 0; i < byteRasterized; i ++ )
				{
					pbytRasterized[i] = tone[pbytRasterized[i]] ;
				}
				bufRasterized.FinishArray() ;
			}
			if ( m_flagForVertical
				&& (wch >= 0x80) && (m_strNoRotate4V.Find( wch ) < 0) )
			{
				//
				// 縦書き用に回転
				//
				SGLImageBuffer	imgDst, imgSrc ;
				imgSrc.format = formatImageGray ;
				imgSrc.depth = 8 ;
				imgSrc.width = metrics.rctExterior.w ;
				imgSrc.height = metrics.rctExterior.h ;
				imgSrc.pitchPixel = 1 ;
				imgSrc.pitchLine = metrics.rctExterior.w ;
				imgSrc.ptrBuffer = bufRasterized.GetArray() ;
				//
				imgDst.format = formatImageGray ;
				imgDst.depth = 8 ;
				imgDst.width = metrics.rctExterior.h ;
				imgDst.height = metrics.rctExterior.w ;
				imgDst.pitchPixel = 1 ;
				imgDst.pitchLine = imgDst.width ;
				imgDst.ptrBuffer =
					bufRotation.GetArray( imgDst.width * imgDst.height ) ;
				//
				sglOrthogonalRotateImageBuffer( imgDst, imgSrc, 90 ) ;
				//
				bufRasterized.FinishArray() ;
				bufRotation.FinishArray() ;
				//
				metrics.rctExterior.x = metrics.rctExterior.y ;
				metrics.rctExterior.y =
							((int32_t) pFontInfo->m_nSize
										- (int32_t) imgDst.height) / 2 ;
				if ( m_strGeminate.Find( wch ) >= 0 )
				{
					metrics.rctExterior.x =
								((int32_t) pFontInfo->m_nSize
											- (int32_t) imgDst.width) / 2 ;
					metrics.rctExterior.y =
								((int32_t) pFontInfo->m_nSize * 2 / 3
											- (int32_t) imgDst.height) / 2 ;
					if ( metrics.rctExterior.y < 0 )
					{
						metrics.rctExterior.y = 0 ;
					}
				}
				metrics.rctExterior.w = imgDst.width ;
				metrics.rctExterior.h = imgDst.height ;
				metrics.nAscent = pFontInfo->m_nSize ;
				metrics.nDescent = 0 ;
				metrics.nLeading = 0 ;
				metrics.nWidth = pFontInfo->m_nSize ;
				//
				bufRasterized = bufRotation ;
				byteRasterized = imgDst.width * imgDst.height ;
			}
			if ( pwszExceptFont == NULL )
			{
				fontset.m_style.metricsFont = metrics ;
			}
			else
			{
				font.SetStyle( style ) ;
			}
			//
			// エントリ情報
			//
			SGLBitmapFontLoader::CharacterEntry		chrEntry ;
			SGLBitmapFontLoader::CharacterHeader	chrHeader ;
			chrEntry.codeChar = wch ;
			chrEntry.offsetAddr = (uint32_t) sbufGrph.GetPosition() ;
			chrHeader.pitchChar = metrics.nWidth ;
			chrHeader.rctExterior = metrics.rctExterior ;
			//
			// アライメント調整
			//
			size_t	offsetLowAddr =
						chrEntry.offsetAddr
							& SGLBitmapFontLoader::bufferPageOffsetMask ;
			if ( offsetLowAddr
					+ sizeof(SGLBitmapFontLoader::CharacterHeader)
								> SGLBitmapFontLoader::bufferPageSize )
			{
				SByteBuffer	buf ;
				size_t	oddBytes =
					SGLBitmapFontLoader::bufferPageSize - offsetLowAddr ;
				buf.SetLength( oddBytes ) ;
				sbufGrph.Write( buf.GetConstArray(), oddBytes ) ;
				//
				chrEntry.offsetAddr = (uint32_t) sbufGrph.GetPosition() ;
			}
			//
			// エントリ追加
			//
			fontset.m_style.countCharacters ++ ;
			fontset.m_entries.Add( chrEntry ) ;
			sbufGrph.Write
				( &chrHeader,
					sizeof(SGLBitmapFontLoader::CharacterHeader) ) ;
			//
			// ラスタライズデータ追加
			//
			sbufGrph.Write( bufRasterized.GetConstArray(), byteRasterized ) ;
			//
			// 進行状況
			//
			if ( pListener != NULL )
			{
				SGLError	err = pListener->OnRender
					( this, wch, iFont, m_listFontSet.GetLength() ) ;
				if ( err )
				{
					cf.Close() ;
					return	err ;
				}
			}
		}
		//
		// フォントセット情報書き出し
		//
		cf.DescendChunk( "fontentr" ) ;
		cf.Write( &(fontset.m_style),
						sizeof(SGLBitmapFontLoader::FontEntry) ) ;
		cf.Write( fontset.m_entries.GetConstArray(),
					fontset.m_style.countCharacters
						* sizeof(SGLBitmapFontLoader::CharacterEntry) ) ;
		cf.AscendChunk() ;
	}
	if ( pListener != NULL )
	{
		SGLError	err = pListener->OnRender
			( this, 0, m_listFontSet.GetLength(),
							m_listFontSet.GetLength() ) ;
		if ( err )
		{
			cf.Close() ;
			return	err ;
		}
	}
	//
	// ラスタライズデータ書き出し
	//
	cf.DescendChunk( "fontgrph" ) ;
	sbufGrph.WriteToStream( cf ) ;
	cf.AscendChunk() ;
	cf.Close() ;
	//
	return	sglErrSuccess ;
}

// 設定XMLファイル読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError SGLBitmapFontMaker::LoadConfiguration( const wchar_t * pwszFilePath )
{
	SXMLDocument	xmlDoc ;
	if ( xmlDoc.LoadDocument( pwszFilePath, xmlDoc ) )
	{
		return	sglErrFailed ;
	}
	return	ParseConfiguration( xmlDoc ) ;
}

SGLError SGLBitmapFontMaker::ParseConfiguration( const SSystem::SXMLDocument& xmlDoc )
{
	SXMLDocument *	pxmlFontsets = xmlDoc.GetElementTagAs( L"fontsets" ) ;
	if ( pxmlFontsets == NULL )
	{
		return	sglErrFailed ;
	}
	m_strOrgFont = pxmlFontsets->GetAttrStringAs( L"def_face" ) ;
	//
	SXMLDocument *	pxmlForVertical =
						pxmlFontsets->GetElementTagAs( L"for_vertical" ) ;
	if ( pxmlForVertical != NULL )
	{
		if ( pxmlForVertical->GetAttrStringAs( L"rotation" ) == L"true" )
		{
			m_flagForVertical = true ;
			m_strNoRotate4V = m_pwszDefNoRotate4V ;
			m_strGeminate = m_pwszDefGeminate ;
		}
		for ( size_t iTag = 0; iTag < pxmlForVertical->GetElementsCount(); iTag ++ )
		{
			SXMLDocument *	pxmlTag = pxmlFontsets->GetElementAt( iTag ) ;
			if ( pxmlTag->GetType() != SXMLDocument::typeTag )
			{
				continue ;
			}
			if ( pxmlTag->GetTag() == L"no_rotation" )
			{
				SString	strAdd = pxmlTag->GetAttrStringAs( L"add" ) ;
				SString	strRemove = pxmlTag->GetAttrStringAs( L"remove" ) ;
				size_t	i ;
				for ( i = 0; i < strAdd.GetLength(); i ++ )
				{
					wchar_t	wch = strAdd.GetAt( i ) ;
					if ( m_strNoRotate4V.Find( wch ) < 0 )
					{
						m_strNoRotate4V += wch ;
					}
				}
				for ( i = 0; i < strRemove.GetLength(); i ++ )
				{
					wchar_t	wch = strRemove.GetAt( i ) ;
					ssize_t	k = m_strNoRotate4V.Find( wch ) ;
					if ( k >= 0 )
					{
						m_strNoRotate4V.RemoveAt( (size_t) k ) ;
					}
				}
			}
			else if ( pxmlTag->GetTag() == L"geminated_char" )
			{
				SString	strAdd = pxmlTag->GetAttrStringAs( L"add" ) ;
				SString	strRemove = pxmlTag->GetAttrStringAs( L"remove" ) ;
				size_t	i ;
				for ( i = 0; i < strAdd.GetLength(); i ++ )
				{
					wchar_t	wch = strAdd.GetAt( i ) ;
					if ( m_strGeminate.Find( wch ) < 0 )
					{
						m_strGeminate += wch ;
					}
				}
				for ( i = 0; i < strRemove.GetLength(); i ++ )
				{
					wchar_t	wch = strRemove.GetAt( i ) ;
					ssize_t	k = m_strGeminate.Find( wch ) ;
					if ( k >= 0 )
					{
						m_strGeminate.RemoveAt( (size_t) k ) ;
					}
				}
			}
		}
	}
	//
	m_listFontSet.RemoveAll() ;
	m_listException.RemoveAll() ;
	//
	for ( size_t iTag = 0; iTag < pxmlFontsets->GetElementsCount(); iTag ++ )
	{
		SXMLDocument *	pxmlTag = pxmlFontsets->GetElementAt( iTag ) ;
		if ( pxmlTag->GetType() != SXMLDocument::typeTag )
		{
			continue ;
		}
		if ( pxmlTag->GetTag() == L"fontset" )
		{
			FontInfo *	pFontInfo = new FontInfo ;
			pFontInfo->m_nSize =
				(uint32_t) pxmlTag->GetAttrIntegerAs( L"size", 16 ) ;
			pFontInfo->m_nAsSize =
				(uint32_t) pxmlTag->GetAttrIntegerAs
							( L"as_size", pFontInfo->m_nSize ) ;
			pFontInfo->m_nStyle = 0 ;
			if ( pxmlTag->GetAttrStringAs( L"bold" ) == L"true" )
			{
				pFontInfo->m_nStyle |= SGLFontStyle::styleBold ;
			}
			if ( pxmlTag->GetAttrStringAs( L"italic" ) == L"true" )
			{
				pFontInfo->m_nStyle |= SGLFontStyle::styleItalic ;
			}
			pFontInfo->m_nOversampling =
					(int) pxmlTag->GetAttrIntegerAs( L"oversampling", 1 ) ;
			pFontInfo->m_nGamma =
					(int) eslRoundR64ToLInt
						( pxmlTag->GetAttrRealAs( L"gamma", 1.0 ) * 0x100 ) ;
			pFontInfo->m_flagJISChar =
					(pxmlTag->GetAttrStringAs( L"jis_char", L"true" ) == L"true") ;
			pFontInfo->m_flag7bitChar =
					(pxmlTag->GetAttrStringAs( L"7bit_char" ) == L"true") ;
			pFontInfo->m_strEx7bitChar =
					pxmlTag->GetAttrStringAs( L"ex_7bit_chars" ) ;
			pFontInfo->m_strFont =
					pxmlTag->GetAttrStringAs( L"face", m_strOrgFont ) ;
			//
			m_listFontSet.Add( pFontInfo ) ;
		}
		else if ( pxmlTag->GetTag() == L"exception" )
		{
			ExceptionCharacters *	pException = new ExceptionCharacters ;
			pException->m_strFont =
				pxmlTag->GetAttrStringAs( L"face", m_strOrgFont ) ;
			pException->m_strCharacters =
				pxmlTag->GetAttrStringAs( L"chars", L"" ) ;
			//
			m_listException.Add( pException ) ;
		}
	}
	return	sglErrSuccess ;
}

// 設定XMLファイル保存
//////////////////////////////////////////////////////////////////////////////
SGLError SGLBitmapFontMaker::SaveConfiguration( const wchar_t * pwszFilePath )
{
	SXMLDocument	xmlDoc ;
	if ( GetConfiguration( xmlDoc ) )
	{
		return	sglErrFailed ;
	}
	SFileInterface *	pFile =
		SFileOpener::DefaultNewOpenFile
			( pwszFilePath, SFileOpener::modeCreate ) ;
	if ( pFile == NULL )
	{
		return	sglErrFailed ;
	}
	xmlDoc.WriteDocument( *pFile ) ;
	delete	pFile ;
	return	sglErrSuccess ;
}

SGLError SGLBitmapFontMaker::GetConfiguration( SSystem::SXMLDocument& xmlDoc )
{
	xmlDoc.SetTag( L"fontsets" ) ;
	xmlDoc.SetAttributeAs( L"def_face", m_strOrgFont ) ;
	//
	if ( m_flagForVertical )
	{
		SXMLDocument *	pxmlForVertical =
							xmlDoc.CreateElementTagAs( L"for_vertical" ) ;
		pxmlForVertical->SetAttributeAs( L"rotation", L"true" ) ;
		//
		SXMLDocument *	pxmlNoRotation =
							pxmlForVertical->CreateElementTagAs( L"no_rotation" ) ;
		SString	strRemove ;
		size_t	i ;
		for ( i = 0; m_pwszDefNoRotate4V[i]; i ++ )
		{
			if ( m_strNoRotate4V.Find( m_pwszDefNoRotate4V[i] ) < 0 )
			{
				strRemove.Add( m_pwszDefNoRotate4V[i] ) ;
			}
		}
		pxmlNoRotation->SetAttributeAs( L"add", m_strNoRotate4V ) ;
		pxmlNoRotation->SetAttributeAs( L"remove", strRemove ) ;
		//
		SXMLDocument *	pxmlGeminated =
							pxmlForVertical->CreateElementTagAs( L"geminated_char" ) ;
		strRemove = L"" ;
		for ( i = 0; m_pwszDefGeminate[i]; i ++ )
		{
			if ( m_strGeminate.Find( m_pwszDefGeminate[i] ) < 0 )
			{
				strRemove.Add( m_pwszDefGeminate[i] ) ;
			}
		}
		pxmlGeminated->SetAttributeAs( L"add", m_strGeminate ) ;
		pxmlGeminated->SetAttributeAs( L"remove", strRemove ) ;
	}
	for ( size_t i = 0; i < m_listFontSet.GetLength(); i ++ )
	{
		FontInfo *	pFontInfo = m_listFontSet.GetAt( i ) ;
		if ( pFontInfo != NULL )
		{
			SXMLDocument *	pxmlTag = new SXMLDocument ;
			xmlDoc.AddElement( pxmlTag ) ;
			pxmlTag->SetTag( L"fontset" ) ;
			pxmlTag->SetAttrIntegerAs( L"size", pFontInfo->m_nSize ) ;
			pxmlTag->SetAttrIntegerAs( L"as_size", pFontInfo->m_nAsSize ) ;
			if ( pFontInfo->m_nStyle & SGLFontStyle::styleBold )
			{
				pxmlTag->SetAttributeAs( L"bold", L"true" ) ;
			}
			if ( pFontInfo->m_nStyle & SGLFontStyle::styleItalic )
			{
				pxmlTag->SetAttributeAs( L"italic", L"true" ) ;
			}
			if ( pFontInfo->m_nOversampling > 1 )
			{
				pxmlTag->SetAttrIntegerAs
						( L"oversampling", pFontInfo->m_nOversampling ) ;
			}
			if ( pFontInfo->m_flag7bitChar )
			{
				pxmlTag->SetAttributeAs( L"7bit_char", L"true" ) ;
				pxmlTag->SetAttributeAs( L"ex_7bit_chars", pFontInfo->m_strEx7bitChar ) ;
			}
			pxmlTag->SetAttributeAs( L"face", pFontInfo->m_strFont ) ;
		}
	}
	for ( size_t i = 0; i < m_listException.GetLength(); i ++ )
	{
		ExceptionCharacters *	pException = m_listException.GetAt( i ) ;
		if ( pException != NULL )
		{
			SXMLDocument *	pxmlTag = new SXMLDocument ;
			xmlDoc.AddElement( pxmlTag ) ;
			pxmlTag->SetTag( L"exception" ) ;
			pxmlTag->SetAttributeAs( L"face", pException->m_strFont ) ;
			pxmlTag->SetAttributeAs( L"chars", pException->m_strCharacters ) ;
		}
	}
	return	sglErrSuccess ;
}

// 変換元フォント設定
//////////////////////////////////////////////////////////////////////////////
void SGLBitmapFontMaker::SetOriginalFont( const wchar_t * pwszFont )
{
	m_strOrgFont = pwszFont ;
}

// 変換元フォント取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * SGLBitmapFontMaker::GetOriginalFont( void ) const
{
	return	m_strOrgFont ;
}

// フォントセット追加
//////////////////////////////////////////////////////////////////////////////
void SGLBitmapFontMaker::AddFontSet( const SGLFontStyle& style )
{
	FontInfo *	pFontInfo = new FontInfo ;
	pFontInfo->m_nStyle = style.nStyles ;
	pFontInfo->m_nSize = style.nSize ;
	pFontInfo->m_nAsSize = style.nSize ;
	pFontInfo->m_strFont = style.pszFace ;
	m_listFontSet.Add( pFontInfo ) ;
}

// フォントセットリスト取得
//////////////////////////////////////////////////////////////////////////////
const SSystem::SObjectArray<SGLBitmapFontMaker::FontInfo> &
			SGLBitmapFontMaker::GetFontSetArray( void ) const
{
	return	m_listFontSet ;
}

// フォントセットリスト全削除
//////////////////////////////////////////////////////////////////////////////
void SGLBitmapFontMaker::RemoveAllFontSet( void )
{
	m_listFontSet.RemoveAll() ;
}

// 例外フォント情報追加
//////////////////////////////////////////////////////////////////////////////
void SGLBitmapFontMaker::AddException
	( const wchar_t * pwszFont, const wchar_t * pwszChar )
{
	ExceptionCharacters *	pException = new ExceptionCharacters ;
	pException->m_strFont = pwszFont ;
	pException->m_strCharacters = pwszChar ;
	m_listException.Add( pException ) ;
}

// 例外フォント取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * SGLBitmapFontMaker::GetExceptionFont( wchar_t wch ) const
{
	for ( size_t i = 0; i < m_listException.GetLength(); i ++ )
	{
		ExceptionCharacters *	pException = m_listException.GetAt( i ) ;
		if ( pException != NULL )
		{
			if ( pException->m_strCharacters.Find( wch ) >= 0 )
			{
				return	pException->m_strFont ;
			}
		}
	}
	return	NULL ;
}

// 例外フォント情報全削除
//////////////////////////////////////////////////////////////////////////////
void SGLBitmapFontMaker::RemoveAllException( void )
{
	m_listException.RemoveAll() ;
}

// 例外フォント情報配列取得
//////////////////////////////////////////////////////////////////////////////
SSystem::SObjectArray<SGLBitmapFontMaker::ExceptionCharacters> &
					SGLBitmapFontMaker::GetExceptionArray( void )
{
	return	m_listException ;
}


