
#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl2d_image.h>
#include <sakuragl/sgl2d/sgl_image_conversion.h>
#include <sakuragl/sgl2d/sgl_image_buf_object.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// レタリング
//////////////////////////////////////////////////////////////////////////////

const wchar_t *	SGLLetteringContext::pwszDefProhibition =
						L",.!?;:)]，．、。！？；：」】）〕｝〉》』"
						L"ぁぃぅぇぉっゃゅょァィゥェォッャュョー～" ;

// SGLLetteringContext シリアライズ（ポインタを除く）
//////////////////////////////////////////////////////////////////////////////
SSystem::SError SGLLetteringContext::SaveWithoutPointer( SSystem::SFileInterface& file ) const
{
	uint32_t	ptrNull = 0 ;
	file.Write( &ptStartWriting, sizeof(SGLPoint) ) ;
	file.Write( &rectWritable, sizeof(SGLRect) ) ;
	file.Write( &typeAlignment, sizeof(uint16_t) ) ;
	file.Write( &flagVertical, sizeof(uint16_t) ) ;
	file.Write( &pitchChar, sizeof(int32_t) ) ;
	file.Write( &offsetChar, sizeof(int32_t) ) ;
	file.Write( &scalePitch, sizeof(int32_t) ) ;
	file.Write( &offsetInHalf, sizeof(int32_t) ) ;
	file.Write( &offsetOutHalf, sizeof(int32_t) ) ;
	file.Write( &pitchTab, sizeof(int32_t) ) ;
	file.Write( &pitchLine, sizeof(int32_t) ) ;
	file.Write( &widthIndent, sizeof(int32_t) ) ;
	file.Write( &minHyphening, sizeof(uint32_t) ) ;
	file.Write( &maxProhibition, sizeof(uint32_t) ) ;
	file.Write( &ptrNull, sizeof(uint32_t) ) ;
	return	errSuccess ;
}

SSystem::SError SGLLetteringContext::LoadWithoutPointer( SSystem::SFileInterface& file )
{
	uint32_t	ptrNull = 0 ;
	file.Read( &ptStartWriting, sizeof(SGLPoint) ) ;
	file.Read( &rectWritable, sizeof(SGLRect) ) ;
	file.Read( &typeAlignment, sizeof(uint16_t) ) ;
	file.Read( &flagVertical, sizeof(uint16_t) ) ;
	file.Read( &pitchChar, sizeof(int32_t) ) ;
	file.Read( &offsetChar, sizeof(int32_t) ) ;
	file.Read( &scalePitch, sizeof(int32_t) ) ;
	file.Read( &offsetInHalf, sizeof(int32_t) ) ;
	file.Read( &offsetOutHalf, sizeof(int32_t) ) ;
	file.Read( &pitchTab, sizeof(int32_t) ) ;
	file.Read( &pitchLine, sizeof(int32_t) ) ;
	file.Read( &widthIndent, sizeof(int32_t) ) ;
	file.Read( &minHyphening, sizeof(uint32_t) ) ;
	file.Read( &maxProhibition, sizeof(uint32_t) ) ;
	file.Read( &ptrNull, sizeof(uint32_t) ) ;
	return	errSuccess ;
}

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLLetterer, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLLetterer::SGLLetterer( void )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLLetterer::~SGLLetterer( void )
{
	SGLLetterer::ClearLetter() ;
}

// レタリング
//////////////////////////////////////////////////////////////////////////////
size_t SGLLetterer::WriteLetter
	( SGLFont& font, SGLLetteringContext& context, const wchar_t * pwszLetter )
{
	//
	// 文字列長
	//
	size_t	lenLetter = 0 ;
	if ( pwszLetter != NULL )
	{
		while ( pwszLetter[lenLetter] != 0 )
		{
			lenLetter ++ ;
		}
	}
	const size_t	iStartChar = m_characters.GetLength() ;
	m_characters.SetLength( iStartChar + lenLetter ) ;
	//
	// 文字ラスタライズ
	//
	size_t	i ;
	SArray<uint8_t>	bufGray ;
	SGLSize			sizeMaxChar( 0, 0 ) ;
	/*
	for ( i = 0; i < lenLetter; i ++ )
	{
		Character *	pChar = m_characters.GetAt( iStartChar + i ) ;
		ESLAssert( pChar != NULL ) ;
		RasterizeCharacter( font, *pChar, bufGray, pwszLetter[i] ) ;
		//
		if ( sizeMaxChar.w < pChar->sizeChar.w )
		{
			sizeMaxChar.w = pChar->sizeChar.w ;
		}
		if ( sizeMaxChar.h < pChar->sizeChar.h )
		{
			sizeMaxChar.h = pChar->sizeChar.h ;
		}
	}
	*/
	//
	// 文字配置
	//
	const bool	flagVertical =
					((context.flagVertical
						& SGLLetteringContext::writingVertical) != 0) ;
	int32_t		posWriting = context.ptStartWriting.x ;
	int32_t		limitLine = context.rectWritable.right ;
	size_t		iLineStart = iStartChar ;
	enum	CharacterWidthType
	{
		charWidthNothing,
		charWidthHalf,
		charWidthFull,
	}			cwtLast = charWidthNothing ;
	if ( flagVertical )
	{
		posWriting = context.ptStartWriting.y ;
		limitLine = context.rectWritable.bottom ;
	}
	i = 0 ;
	while ( i < lenLetter )
	{
		//
		// 文字ラスタライズ
		//
		Character *	pChar = m_characters.GetAt( iStartChar + i ) ;
		ESLAssert( pChar != NULL ) ;
		RasterizeCharacter( font, *pChar, bufGray, pwszLetter[i] ) ;
		//
		CharacterWidthType	cwtCurrent = charWidthNothing ;
		if ( pwszLetter[i] >= 0x100 )
		{
			cwtCurrent = charWidthFull ;
		}
		else if ( pwszLetter[i] > L' ' )
		{
			cwtCurrent = charWidthHalf ;
		}
		//
		if ( sizeMaxChar.w < pChar->sizeChar.w )
		{
			sizeMaxChar.w = pChar->sizeChar.w ;
		}
		if ( sizeMaxChar.h < pChar->sizeChar.h )
		{
			sizeMaxChar.h = pChar->sizeChar.h ;
		}
		//
		// 改行判定
		//
		bool	flagWordWrap = false ;
		bool	flagNextLine = false ;
		size_t	iLineEnd = i ;
		if ( pChar->wchCode == L'\n' )
		{
			//
			// 改行コード
			//
			pChar->sizeChar.w = 0 ;
			pChar->sizeChar.h = 0 ;
			flagNextLine = true ;
			iLineEnd = ++ i ;
		}
		else if ( pChar->wchCode == L'\t' )
		{
			//
			// タブ
			//
			int32_t	posNext = posWriting - context.rectWritable.left ;
			if ( flagVertical )
			{
				posNext = posWriting - context.rectWritable.top ;
			}
			int32_t	pitchTab = context.pitchTab ;
			if ( pitchTab <= 0 )
			{
				if ( context.pitchChar > 0 )
				{
					pitchTab = (context.pitchChar + context.offsetChar) * 4 ;
				}
				else
				{
					pitchTab = 64 ;
				}
			}
			int32_t	posCur = posNext ;
			posNext = ((posNext / pitchTab) + 1) * pitchTab ;
			if ( !flagVertical )
			{
				posWriting = posNext + context.rectWritable.left ;
				pChar->sizeChar.w = posWriting - posCur ;
			}
			else
			{
				posWriting = posNext + context.rectWritable.top ;
				pChar->sizeChar.h = posWriting - posCur ;
			}
			i ++ ;
			cwtLast = charWidthNothing ;
		}
		else
		{
			//
			// 文字幅判定
			//
			int32_t	pitchChar = GetCharacterPitch( context, *pChar ) ;
			if ( (cwtCurrent == charWidthFull)
						&& (cwtLast == charWidthHalf) )
			{
				Character *	pLastChar =
							m_characters.GetAt( iStartChar + i - 1 ) ;
				if ( pLastChar != NULL )
				{
					if ( !flagVertical )
					{
						pLastChar->sizeChar.w += context.offsetOutHalf ;
					}
					else
					{
						pLastChar->sizeChar.h += context.offsetOutHalf ;
					}
				}
				posWriting += context.offsetOutHalf ;
			}
			else if ( (cwtCurrent == charWidthHalf)
						&& (cwtLast == charWidthFull) )
			{
				Character *	pLastChar =
							m_characters.GetAt( iStartChar + i - 1 ) ;
				if ( pLastChar != NULL )
				{
					if ( !flagVertical )
					{
						pLastChar->sizeChar.w += context.offsetInHalf ;
					}
					else
					{
						pLastChar->sizeChar.h += context.offsetInHalf ;
					}
				}
				posWriting += context.offsetInHalf ;
			}
			cwtLast = cwtCurrent ;
			//
			// 行溢れ判定
			//
			if ( posWriting + pitchChar > limitLine )
			{
				flagNextLine = true ;
				flagWordWrap = true ;
				//
				// 禁則文字判定
				//
				if ( IsProhibitionChar
					( context.pwszProhibition, (wchar_t) pChar->wchCode ) )
				{
					size_t	j ;
					for ( j = 1; j < context.maxProhibition; j ++ )
					{
						if ( i - j <= iLineStart + 1 )
						{
							break ;
						}
						pChar = m_characters.GetAt( iLineStart + i - j ) ;
						if ( pChar == NULL )
						{
							break ;
						}
						if ( !IsProhibitionChar
							( context.pwszProhibition, (wchar_t) pChar->wchCode ) )
						{
							break ;
						}
					}
					if ( i >= j )
					{
						i -= j ;
						iLineEnd = i ;
					}
				}
				else if ( (context.minHyphening != 0)
						&& IsAlphabetChar( (wchar_t) pChar->wchCode ) )
				{
					//
					// ハイフニング／ワードラップ判定
					//
					size_t	j ;
					for ( j = 1; j < context.minHyphening; j ++ )
					{
						if ( (ssize_t) i - (ssize_t) j <= (ssize_t) iLineStart + 1 )
						{
							break ;
						}
						pChar = m_characters.GetAt( iStartChar + i - j ) ;
						ESLAssert( pChar != NULL ) ;
						if ( !IsAlphabetChar( (wchar_t) pChar->wchCode ) )
						{
							break ;
						}
					}
					if ( (j < context.minHyphening) | (j < 3) )
					{
						if ( i >= j )
						{
							i -= j ;
							iLineEnd = i ;
						}
					}
					else
					{
						Character	chrHyphen ;
						eslFillMemory( &chrHyphen, 0, sizeof(Character) ) ;
						//
						RasterizeCharacter
							( font, chrHyphen, bufGray, L'-' ) ;
						if ( GetCharacterPitch( context, chrHyphen )
												+ posWriting > limitLine )
						{
							i -- ;
						}
						m_characters.InsertAt( i ++, chrHyphen ) ;
						iLineEnd = i ;
						lenLetter ++ ;
					}
				}
			}
			else
			{
				posWriting += pitchChar ;
				i ++ ;
			}
		}
		if ( flagNextLine )
		{
			//
			// 行の整列処理
			//
			SGLLetteringContext::AlignmentType	typeAlign =
				(SGLLetteringContext::AlignmentType) context.typeAlignment ;
			if ( flagWordWrap )
			{
				typeAlign = SGLLetteringContext::alignLong ;
			}
			if ( iLineEnd > iLineStart )
			{
				DoLineAlignment
					( context, typeAlign,
						iLineStart, iLineEnd - iLineStart ) ;
				iLineStart = iLineEnd ;
			}
			//
			// 改行処理
			//
			if ( !flagVertical )
			{
				context.ptStartWriting.x = context.rectWritable.left ;
				if ( context.pitchLine != 0 )
				{
					context.ptStartWriting.y += context.pitchLine ;
				}
				else
				{
					context.ptStartWriting.y += sizeMaxChar.h ;
				}
				if ( flagWordWrap )
				{
					context.ptStartWriting.x += context.widthIndent ;
				}
				posWriting = context.ptStartWriting.x ;
				//
				if ( context.ptStartWriting.y + context.pitchLine - 1
											> context.rectWritable.bottom )
				{
					ClearLetter( iLineStart ) ;
					return	iLineStart ;
				}
			}
			else
			{
				if ( context.pitchLine != 0 )
				{
					context.ptStartWriting.x -= context.pitchLine ;
				}
				else
				{
					context.ptStartWriting.x -= sizeMaxChar.w ;
				}
				context.ptStartWriting.y = context.rectWritable.top ;
				if ( flagWordWrap )
				{
					context.ptStartWriting.y += context.widthIndent ;
				}
				posWriting = context.ptStartWriting.y ;
				//
				if ( context.ptStartWriting.x + context.pitchLine
										< context.rectWritable.left )
				{
					ClearLetter( iLineStart ) ;
					return	iLineStart ;
				}
			}
			cwtLast = charWidthNothing ;
		}
	}
	if ( iLineStart < m_characters.GetLength() )
	{
		DoLineAlignment
			( context, context.typeAlignment,
					iLineStart, m_characters.GetLength() - iLineStart ) ;
	}
	return	i ;
}

// 禁則文字判定
//////////////////////////////////////////////////////////////////////////////
bool SGLLetterer::IsProhibitionChar
	( const wchar_t * pwszProhibition, wchar_t wchCode )
{
	if ( pwszProhibition != NULL )
	{
		while ( *pwszProhibition != 0 )
		{
			if ( *(pwszProhibition ++) == wchCode )
			{
				return	true ;
			}
		}
	}
	return	false ;
}

// アルファベット判定
//////////////////////////////////////////////////////////////////////////////
bool SGLLetterer::IsAlphabetChar( wchar_t wchCode )
{
	return	((L'A' <= wchCode) & (wchCode <= L'Z'))
				|| ((L'a' <= wchCode) & (wchCode <= L'z')) ;
}

// 文字ピッチ取得
//////////////////////////////////////////////////////////////////////////////
int32_t SGLLetterer::GetCharacterPitch
	( const SGLLetteringContext& context, const Character& chChar )
{
	if ( context.pitchChar != 0 )
	{
		return	(int32_t) (((int64_t) context.pitchChar
							* context.scalePitch) >> 16) + context.offsetChar ;
	}
	else if ( !(context.flagVertical & SGLLetteringContext::writingVertical) )
	{
		return	(int32_t) (((int64_t) chChar.sizeChar.w
							* context.scalePitch) >> 16) + context.offsetChar ;
	}
	else
	{
		return	(int32_t) (((int64_t) chChar.sizeChar.h
							* context.scalePitch) >> 16) + context.offsetChar ;
	}
}

// 文字ラスタライズ
//////////////////////////////////////////////////////////////////////////////
void SGLLetterer::RasterizeCharacter
		( SGLFont& font, SGLLetterer::Character& chChar,
			SSystem::SArray<uint8_t>& bufGray, wchar_t wchCode )
{
	if ( chChar.pImage != NULL )
	{
		sglReleaseImageBuffer( chChar.pImage ) ;
		chChar.pImage = NULL ;
	}
	chChar.wchCode = wchCode ;
	chChar.sizeChar.w = 0 ;
	chChar.sizeChar.h = 0 ;
	//
	SGLFontMetrics	fmChar ;
	if ( font.GetMetrics( NULL, 0, fmChar, wchCode ) )
	{
		return ;
	}
	chChar.ptOffset.x = fmChar.rctExterior.x ;
	chChar.ptOffset.y = fmChar.rctExterior.y ;
	chChar.sizeChar.w = fmChar.nWidth ;
	chChar.sizeChar.h = fmChar.nHeight ;
	//
	size_t	areaPixels = fmChar.rctExterior.w * fmChar.rctExterior.h ;
	if ( (areaPixels != 0) & (wchCode > L' ') & (wchCode != L'　') )
	{
		bufGray.SetLength( areaPixels ) ;
		if ( font.GetMetrics
			( bufGray.GetArray(),
				bufGray.GetLength(), fmChar, wchCode ) )
		{
			bufGray.FinishArray() ;
			return ;
		}
		bufGray.FinishArray() ;
		//
		SGLImageBuffer		infChar ;
		infChar.format = formatImageGray ;
		infChar.depth = 8 ;
		infChar.width = fmChar.rctExterior.w ;
		infChar.height = fmChar.rctExterior.h ;
		infChar.pitchPixel = 1 ;
		infChar.pitchLine = fmChar.rctExterior.w ;
		infChar.ptrBuffer = bufGray.GetArray() ;
		//
		SGLImageBuffer *
			pImage = sglCreateImageBuffer( infChar ) ;
		chChar.pImage = pImage ;
		//
		sglCopyImageBuffer( *pImage, infChar ) ;
		bufGray.FinishArray() ;
	}
}

// 行アライメント
//////////////////////////////////////////////////////////////////////////////
void SGLLetterer::DoLineAlignment
	( SGLLetteringContext& context,
		int typeAlign, size_t iFirst, size_t nCount )
{
	const bool	flagVertical =
					((context.flagVertical
						& SGLLetteringContext::writingVertical) != 0) ;
	if ( nCount == 0 )
	{
		return ;
	}
	//
	// 行の幅と高さを計算
	//
	int32_t	widthChars = 0 ;
	int32_t	pitchLine = 0 ;
	size_t	i ;
	if ( context.pitchChar != 0 )
	{
		widthChars = (int32_t) (context.pitchChar * nCount) ;
	}
	else if ( !flagVertical )
	{
		for ( i = 0; i < nCount; i ++ )
		{
			Character *	pChar = m_characters.GetAt( iFirst + i ) ;
			ESLAssert( pChar != NULL ) ;
			widthChars += pChar->sizeChar.w ;
			//
			if ( pitchLine < pChar->sizeChar.h )
			{
				pitchLine = pChar->sizeChar.h ;
			}
		}
	}
	else
	{
		for ( i = 0; i < nCount; i ++ )
		{
			Character *	pChar = m_characters.GetAt( iFirst + i ) ;
			ESLAssert( pChar != NULL ) ;
			widthChars += pChar->sizeChar.h ;
			//
			if ( pitchLine < pChar->sizeChar.w )
			{
				pitchLine = pChar->sizeChar.w ;
			}
		}
	}
	//
	// 描画位置の補正
	//
	int32_t	offsetChars = 0 ;
	int32_t	offsetLine = 0 ;
	if ( !flagVertical )
	{
		if ( pitchLine < context.pitchLine )
		{
			offsetLine = context.pitchLine - pitchLine ;
			offsetLine -= (offsetLine >> 2) ;
		}
	}
	if ( typeAlign == SGLLetteringContext::alignRight )
	{
		if ( !flagVertical )
		{
			context.ptStartWriting.x =
					context.rectWritable.right - widthChars ;
		}
		else
		{
			context.ptStartWriting.y =
					context.rectWritable.bottom - widthChars ;
		}
	}
	else if ( typeAlign == SGLLetteringContext::alignCenter )
	{
		if ( !flagVertical )
		{
			context.ptStartWriting.x +=
				((context.rectWritable.right
						- context.ptStartWriting.x + 1) - widthChars) / 2 ;
		}
		else
		{
			context.ptStartWriting.y +=
				((context.rectWritable.bottom
						- context.ptStartWriting.y + 1) - widthChars) / 2 ;
		}
	}
	else if ( (context.pitchChar == 0)
				& (typeAlign == SGLLetteringContext::alignLong) )
	{
		if ( !flagVertical )
		{
			offsetChars =
				((context.rectWritable.right
						- context.ptStartWriting.x + 1) - widthChars) ;
		}
		else
		{
			offsetChars =
				((context.rectWritable.bottom
						- context.ptStartWriting.y + 1) - widthChars) ;
		}
	}
	//
	// 描画位置決定
	//
	for ( i = 0; i < nCount; i ++ )
	{
		Character *	pChar = m_characters.GetAt( iFirst + i ) ;
		ESLAssert( pChar != NULL ) ;
		pChar->ptWriting = context.ptStartWriting ;
//		pChar->ptWriting.y += offsetLine ;
		pChar->ptOffset.y += offsetLine ;
		//
		int32_t	offsetPitch = 0 ;
		if ( (i < nCount - 1) & (offsetChars != 0) )
		{
			offsetPitch =
				(((offsetChars * (int32_t) (i + 1)) / (int32_t) (nCount - 1))
					- ((int32_t) (offsetChars * i) / (int32_t) (nCount - 1))) ;
		}
		if ( context.pitchChar != 0 )
		{
			if ( !flagVertical )
			{
				context.ptStartWriting.x += context.pitchChar ;
			}
			else
			{
				context.ptStartWriting.y += context.pitchChar ;
			}
		}
		else if ( !flagVertical )
		{
			context.ptStartWriting.x += pChar->sizeChar.w + offsetPitch ;
		}
		else
		{
			context.ptStartWriting.y += pChar->sizeChar.h + offsetPitch ;
		}
	}
}

// 文字装飾
//////////////////////////////////////////////////////////////////////////////
SGLError SGLLetterer::DecorateLetter
	( const SGLLetterer::Decoration& deco, size_t iFirst, ssize_t nCount )
{
	if ( nCount < 0 )
	{
		nCount = (ssize_t) m_characters.GetLength() - (ssize_t) iFirst ;
	}
	for ( size_t i = 0; (ssize_t) i < nCount; i ++ )
	{
		Character *	pChar = m_characters.GetAt( iFirst + i ) ;
		if ( pChar && (pChar->pImage != NULL) )
		{
			DoDecoration( deco, *pChar ) ;
		}
	}
	return	sglErrSuccess ;
}

// 文字装飾
//////////////////////////////////////////////////////////////////////////////
void SGLLetterer::DoDecoration
	( const SGLLetterer::Decoration& deco, SGLLetterer::Character& chLetter )
{
	//
	// 文字画像を ARGB 形式に変換
	//
	if ( chLetter.pImage == NULL )
	{
		return ;
	}
	if ( chLetter.pImage->format == formatImageGray )
	{
		NormalizeCharacterGrayToARGB( chLetter ) ;
	}
	SGLImageBuffer *	pCharShape = chLetter.pImage ;
	if ( pCharShape == NULL )
	{
		return ;
	}
	//
	// 文字色適用
	//
	if ( (deco.nFlags == 0) & (deco.rgbaBody.ui32 == 0xFFFFFFFF) )
	{
		return ;
	}
	SGLImageBuffer *	pCharBody = sglCreateImageBuffer( *pCharShape ) ;
	if ( pCharBody == NULL )
	{
		return ;
	}
	SGLSmartImage	imgCharBody( pCharBody ) ;
	FillLetterColor
		( deco, *pCharBody, chLetter.ptOffset, chLetter.sizeChar ) ;
	if ( deco.nFlags == 0 )
	{
		sglMultiplierBlendImageBuffer( *pCharShape, *pCharBody ) ;
		return ;
	}
	sglMultiplierBlendImageBuffer( *pCharBody, *pCharShape ) ;
	//
	// 外接矩形取得
	//
	SGLSize		sizeLetter = pCharShape->GetImageSize() ;
	SGLPoint	ptOffset( 0, 0 ) ;
	SGLRect		rectExLetter = SGLImageRect( ptOffset, sizeLetter ) ;
	if ( deco.nFlags & flagShadow )
	{
		rectExLetter |= SGLRect( SGLImageRect( deco.ptShadow, sizeLetter ) ) ;
	}
	if ( deco.nFlags & flagBorder )
	{
		rectExLetter |=
			SGLRect( - (int32_t) deco.widthBorder,
						- (int32_t) deco.widthBorder,
						sizeLetter.w + deco.widthBorder - 1,
						sizeLetter.h + deco.widthBorder - 1 ) ;
		//
		if ( deco.nFlags & flagBorder2 )
		{
			int32_t	nBorder = (int32_t) (deco.widthBorder + deco.widthBorder2) ;
			rectExLetter |=
				SGLRect( - nBorder, - nBorder,
							sizeLetter.w + nBorder - 1,
							sizeLetter.h + nBorder - 1 ) ;
		}
	}
	//
	// 合成画像の出力バッファを生成する
	//
	SGLImageInfo	infLetter ;
	infLetter.format = formatImageARGB ;
	infLetter.depth = 32 ;
	infLetter.width = rectExLetter.GetWidth() ;
	infLetter.height = rectExLetter.GetHeight() ;
	infLetter.pitchPixel = 4 ;
	infLetter.pitchLine = infLetter.width * 4 ;
	//
	SGLImageBuffer *	pBufLetter = sglCreateImageBuffer( infLetter ) ;
	if ( pBufLetter == NULL )
	{
		return ;
	}
	SGLSmartImage	imgCharShape( pCharShape ) ;
	chLetter.pImage = pBufLetter ;
	//
	// 縁取りを描画
	//
	if ( deco.nFlags & flagBorder )
	{
		SGLSmartImage	imgBorder[2] ;
		imgBorder[0].CreateBuffer( infLetter ) ;
		imgBorder[1].CreateBuffer( infLetter ) ;
		//
		sglAdditionalBlendImageBuffer
			( *(imgBorder[1].GetImage()), *pCharShape,
				- rectExLetter.left, - rectExLetter.top ) ;
		//
		for ( size_t i = 0; i < deco.widthBorder; i ++ )
		{
			sglAdditionalBlendImageBuffer
				( *(imgBorder[0].GetImage()),
						*(imgBorder[1].GetImage()), -1, 0 ) ;
			sglAdditionalBlendImageBuffer
				( *(imgBorder[0].GetImage()),
						*(imgBorder[1].GetImage()), 1, 0 ) ;
			sglAdditionalBlendImageBuffer
				( *(imgBorder[0].GetImage()),
						*(imgBorder[1].GetImage()), 0, -1 ) ;
			sglAdditionalBlendImageBuffer
				( *(imgBorder[0].GetImage()),
						*(imgBorder[1].GetImage()), 0, 1 ) ;
			sglCopyImageBuffer
				( *(imgBorder[1].GetImage()),
						*(imgBorder[0].GetImage()) ) ;
		}
		FillBorderColor
			( deco, imgBorder[0], chLetter.ptOffset, chLetter.sizeChar ) ;
		sglMultiplierBlendImageBuffer
			( *(imgBorder[0].GetImage()), *(imgBorder[1].GetImage()) ) ;
		sglBlendBackImageBuffer( *pBufLetter, *(imgBorder[0].GetImage()) ) ;
		//
		if ( deco.nFlags & flagBorder2 )
		{
			sglFillImageBuffer( *(imgBorder[0].GetImage()), SGLPalette(0) ) ;
			//
			for ( size_t i = 0; i < deco.widthBorder2; i ++ )
			{
				sglAdditionalBlendImageBuffer
					( *(imgBorder[0].GetImage()),
							*(imgBorder[1].GetImage()), -1, 0 ) ;
				sglAdditionalBlendImageBuffer
					( *(imgBorder[0].GetImage()),
							*(imgBorder[1].GetImage()), 1, 0 ) ;
				sglAdditionalBlendImageBuffer
					( *(imgBorder[0].GetImage()),
							*(imgBorder[1].GetImage()), 0, -1 ) ;
				sglAdditionalBlendImageBuffer
					( *(imgBorder[0].GetImage()),
							*(imgBorder[1].GetImage()), 0, 1 ) ;
				sglCopyImageBuffer
					( *(imgBorder[1].GetImage()),
							*(imgBorder[0].GetImage()) ) ;
			}
			FillBorder2Color
				( deco, imgBorder[0], chLetter.ptOffset, chLetter.sizeChar ) ;
			sglMultiplierBlendImageBuffer
				( *(imgBorder[0].GetImage()), *(imgBorder[1].GetImage()) ) ;
			sglBlendBackImageBuffer( *pBufLetter, *(imgBorder[0].GetImage()) ) ;
		}
	}
	//
	// 影を描画
	//
	if ( deco.nFlags & flagShadow )
	{
		SGLSmartImage	imgShadow ;
		imgShadow.CreateBuffer( *pBufLetter ) ;
		FillShadowColor
			( deco, imgShadow, chLetter.ptOffset, chLetter.sizeChar ) ;
		sglMultiplierBlendImageBuffer( *(imgShadow.GetImage()), *pBufLetter ) ;
		sglBlendBackImageBuffer
			( *pBufLetter, *(imgShadow.GetImage()),
				deco.ptShadow.x, deco.ptShadow.y ) ;
	}
	//
	// 文字を描画
	//
	sglBlendImageBuffer
		( *pBufLetter, *pCharBody,
			- rectExLetter.left, - rectExLetter.top ) ;
}

// 画像結合
//////////////////////////////////////////////////////////////////////////////
SGLError SGLLetterer::CombineLetter( void )
{
	//
	// 矩形合成
	//
	const size_t	nCount = m_characters.GetLength() ;
	SGLRect			rectLetter( 0, 0, -1, -1 ) ;
	SGLRect			rectExtLetter( 0, 0, -1, -1 ) ;
	size_t			i ;
	for ( i = 0; i < nCount; i ++ )
	{
		Character *	pChar = m_characters.GetAt( i ) ;
		ESLAssert( pChar != NULL ) ;
		if ( pChar->pImage != NULL )
		{
			SGLRect	rectChar =
						pChar->pImage->GetImageRect()
							+ pChar->ptWriting + pChar->ptOffset ;
			SGLRect	rectExt =
						SGLImageRect( pChar->ptWriting, pChar->sizeChar ) ;
			if ( rectLetter.IsEmpty() )
			{
				rectLetter = rectChar ;
				rectExtLetter = rectExt ;
			}
			else
			{
				rectLetter |= rectChar ;
				rectExtLetter |= rectExt ;
			}
		}
	}
	if ( rectLetter.IsEmpty() )
	{
		ClearLetter() ;
		return	sglErrFailed ;
	}
	//
	// 画像バッファ生成
	//
	SGLImageInfo	infLetter ;
	infLetter.format = formatImageARGB ;
	infLetter.depth = 32 ;
	infLetter.width = rectLetter.GetWidth() ;
	infLetter.height = rectLetter.GetHeight() ;
	infLetter.pitchPixel = 4 ;
	infLetter.pitchLine = infLetter.width * infLetter.pitchPixel ;
	//
	SGLImageBuffer *	pImageLetter = sglCreateImageBuffer( infLetter ) ;
	if ( pImageLetter == NULL )
	{
		return	sglErrFailed ;
	}
	//
	// 画像合成描画
	//
	for ( i = 0; i < nCount; i ++ )
	{
		Character *	pChar = m_characters.GetAt( i ) ;
		ESLAssert( pChar != NULL ) ;
		if ( pChar->pImage != NULL )
		{
			NormalizeCharacterGrayToARGB( *pChar ) ;
			sglBlendImageBuffer
				( *pImageLetter, *(pChar->pImage),
					pChar->ptWriting.x + pChar->ptOffset.x - rectLetter.left,
					pChar->ptWriting.y + pChar->ptOffset.y - rectLetter.top ) ;
		}
	}
	//
	// 合成画像を文字画像として設定
	//
	ClearLetter() ;
	//
	Character	chLetter ;
	chLetter.pImage = pImageLetter ;
	chLetter.ptWriting.x = rectExtLetter.left ;
	chLetter.ptWriting.y = rectExtLetter.top ;
	chLetter.ptOffset.x = rectLetter.left - rectExtLetter.left ;
	chLetter.ptOffset.y = rectLetter.top - rectExtLetter.top ;
	chLetter.sizeChar.w = rectExtLetter.GetWidth() ;
	chLetter.sizeChar.h = rectExtLetter.GetHeight() ;
	//
	m_characters.Add( chLetter ) ;
	//
	return	sglErrSuccess ;
}

// 全ての文字データを削除
//////////////////////////////////////////////////////////////////////////////
void SGLLetterer::ClearLetter( size_t iFirst )
{
	const size_t	nCount = m_characters.GetLength() ;
	for ( size_t i = iFirst; i < nCount; i ++ )
	{
		Character *	pChar = m_characters.GetAt( i ) ;
		ESLAssert( pChar != NULL ) ;
		if ( pChar->pImage != NULL )
		{
			sglReleaseImageBuffer( pChar->pImage ) ;
		}
	}
	if ( iFirst == 0 )
	{
		m_characters.RemoveAll() ;
	}
	else
	{
		m_characters.SetLength( iFirst ) ;
	}
}

// 1文字ラスタライズ
//////////////////////////////////////////////////////////////////////////////
SGLError SGLLetterer::RasterizeCharacter
	( SGLImageObject& imgChar,
		SGLPoint& ptOffset, int32_t& nCharWidth,
		SGLFont& font, const SGLLetterer::Decoration& deco, wchar_t wch )
{
	Character		chChar ;
	SArray<uint8_t>	bufGray ;
	eslFillMemory( &chChar, 0, sizeof(Character) ) ;
	//
	RasterizeCharacter( font, chChar, bufGray, wch ) ;
	DoDecoration( deco, chChar ) ;
	//
	nCharWidth = chChar.sizeChar.w ;
	ptOffset = chChar.ptOffset ;
	//
	if ( chChar.pImage != NULL )
	{
		imgChar.CreateBuffer( *chChar.pImage ) ;
		//
		SGLImageBuffer	imgbuf ;
		imgbuf.ptrBuffer = imgChar.LockBuffer( imgbuf ) ;
		sglCopyImageBuffer( imgbuf, *chChar.pImage ) ;
		imgChar.UnlockBuffer() ;
	}
	return	sglErrSuccess ;
}

// 文字装飾画像生成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLLetterer::CreateDecoratedCharacterAt
	( SGLLetterer::Character& chChar,
		const SGLLetterer::Decoration& deco, size_t iChar )
{
	chChar.pImage = NULL ;
	//
	Character *	pChar = m_characters.GetAt( iChar ) ;
	if ( pChar == NULL )
	{
		return	sglErrFailed ;
	}
	chChar = *pChar ;
	//
	if ( chChar.pImage != NULL )
	{
		SGLImageBuffer *
			pTempBuf = sglCreateImageBuffer( *(chChar.pImage) ) ;
		sglCopyImageBuffer( *pTempBuf, *(chChar.pImage) ) ;
		chChar.pImage = pTempBuf ;
		DoDecoration( deco, chChar ) ;
	}
	return	sglErrSuccess ;
}

// Gray 画像を白色 ARGB 形式に変換
//////////////////////////////////////////////////////////////////////////////
void SGLLetterer::NormalizeCharacterGrayToARGB
					( SGLLetterer::Character& chLetter )
{
	if ( chLetter.pImage == NULL )
	{
		return ;
	}
	if ( chLetter.pImage->format != formatImageGray )
	{
		return ;
	}
	SGLImageInfo	infLetter ;
	infLetter.format = formatImageARGB ;
	infLetter.depth = 32 ;
	infLetter.width = chLetter.pImage->width ;
	infLetter.height = chLetter.pImage->height ;
	infLetter.pitchPixel = 4 ;
	infLetter.pitchLine = infLetter.width * 4 ;
	//
	SGLImageBuffer *	pLetter = sglCreateImageBuffer( infLetter ) ;
	if ( pLetter == NULL )
	{
		return ;
	}
	sglConvertImageBuffer( *pLetter, *(chLetter.pImage) ) ;
	sglReleaseImageBuffer( chLetter.pImage ) ;
	chLetter.pImage = pLetter ;
}

// 文字色塗りつぶし
//////////////////////////////////////////////////////////////////////////////
void SGLLetterer::FillLetterColor
	( const SGLLetterer::Decoration& deco, SGLImageBuffer& imgBuf,
			const SGLPoint& ptOffset, const SGLSize& sizeChar )
{
	if ( (deco.nFlags & flagGradation)
		&& (deco.pGradation != NULL)
		&& (deco.nGradationCount >= 2)
		&& (imgBuf.depth == 32) )
	{
		for ( uint32_t y = 0; y < imgBuf.height; y ++ )
		{
			uint32_t	g = 0, t = 0 ;
			if ( deco.nGradationHeight == 0 )
			{
				uint32_t	v = y * (deco.nGradationCount - 1)
										* 0x100 / imgBuf.height ;
				g = (v >> 8) ;
				t = (v & 0xFF) ;
				ESLAssert( g < deco.nGradationCount - 1 ) ;
			}
			else
			{
				size_t	v = (ptOffset.y + y)
								* (deco.nGradationCount - 1)
										* 0x100 / deco.nGradationHeight ;
				g = (v >> 8) % (deco.nGradationCount - 1) ;
				t = (v & 0xFF) ;
			}
			SGLPalette	argb = deco.pGradation[g].imul(0x100 - t)
								+ deco.pGradation[g + 1].imul(t) ;
			//
			SGLPalette *	ppxLine =
				(SGLPalette*) (imgBuf.ptrBuffer + imgBuf.pitchLine * (int32_t) y) ;
			for ( uint32_t x = 0; x < imgBuf.width; x ++ )
			{
				ppxLine[x] = argb ;
			}
		}
	}
	else
	{
		if ( (deco.nFlags & flagGradation)
			&& (deco.pGradation != NULL)
			&& (deco.nGradationCount >= 1) )
		{
			sglFillImageBuffer( imgBuf, deco.pGradation[0] ) ;
		}
		else
		{
			sglFillImageBuffer( imgBuf, deco.rgbaBody ) ;
		}
	}
}

// 縁取り色塗りつぶし
//////////////////////////////////////////////////////////////////////////////
void SGLLetterer::FillBorderColor
	( const SGLLetterer::Decoration& deco, SGLImageBuffer& imgBuf,
			const SGLPoint& ptOffset, const SGLSize& sizeChar )
{
	sglFillImageBuffer( imgBuf, deco.rgbaBorder ) ;
}

void SGLLetterer::FillBorder2Color
	( const SGLLetterer::Decoration& deco, SGLImageBuffer& imgBuf,
		const SGLPoint& ptOffset, const SGLSize& sizeChar )
{
	sglFillImageBuffer( imgBuf, deco.rgbaBorder2 ) ;
}

// 影色塗りつぶし
//////////////////////////////////////////////////////////////////////////////
void SGLLetterer::FillShadowColor
	( const SGLLetterer::Decoration& deco, SGLImageBuffer& imgBuf,
			const SGLPoint& ptOffset, const SGLSize& sizeChar )
{
	sglFillImageBuffer( imgBuf, deco.rgbaShadow ) ;
}

// 文字列描画
//////////////////////////////////////////////////////////////////////////////
SGLError SGLLetterer::DrawLetterTo
	( SGLPaintContextInterface & paint, int xPos, int yPos )
{
	const size_t	nCount = m_characters.GetLength() ;
	SGLPaintParam	ppPaint ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		Character *	pChar = m_characters.GetAt( i ) ;
		ESLAssert( pChar != NULL ) ;
		SGLImageBuffer *	pImage = pChar->pImage ;
		if ( pImage != NULL )
		{
			sglAddReferenceImageBuffer( pImage ) ;
			//
			SGLSmartImage *	pTempImage =
				ESLTypeCast<SGLSmartImage>( m_aTempImageToDraw.GetAt( i ) ) ;
			if ( pTempImage == nullptr )
			{
				pTempImage = new SGLSmartImage ;
				m_aTempImageToDraw.SetAt( i, pTempImage ) ;
			}
			pTempImage->SetImageBuffer( pImage ) ;
			ppPaint.ptPaint = pChar->ptWriting ;
			ppPaint.ptPaint += pChar->ptOffset ;
			ppPaint.ptPaint.x += xPos ;
			ppPaint.ptPaint.y += yPos ;
			paint.DrawImage( ppPaint, pTempImage ) ;
		}
	}
	return	sglErrSuccess ;
}

// 外接矩形取得
//////////////////////////////////////////////////////////////////////////////
const SGLImageRect& SGLLetterer::GetLetterRect( SGLImageRect& rect ) const
{
	const size_t	nCount = m_characters.GetLength() ;
	SGLRect			rectLetter ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		Character *	pChar = m_characters.GetAt( i ) ;
		ESLAssert( pChar != NULL ) ;
		if ( pChar != NULL )
		{
			SGLRect	rectChar ;
			rectChar.left = pChar->ptWriting.x + pChar->ptOffset.x ;
			rectChar.top = pChar->ptWriting.y + pChar->ptOffset.y ;
			if ( pChar->pImage != NULL )
			{
				rectChar.SetWidth( pChar->pImage->width ) ;
				rectChar.SetHeight( pChar->pImage->height ) ;
			}
			else
			{
				rectChar.SetWidth( pChar->sizeChar.w ) ;
				rectChar.SetHeight( pChar->sizeChar.h ) ;
			}
			if ( rectLetter.IsEmpty() )
			{
				rectLetter = rectChar ;
			}
			else
			{
				rectLetter |= rectChar ;
			}
		}
	}
	rect = rectLetter ;
	return	rect ;
}

