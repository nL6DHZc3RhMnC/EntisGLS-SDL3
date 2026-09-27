
#include <sakuragl/sakuragl.h>
#include <sakuragl/sgl2d/sgl_paint_buffer.h>
#include <sakuraglx/extra/sglx_image_composition.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// 画像コンポジション・レイヤー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLImageComposition::Layer, SGLImage )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLImageComposition::Layer::Layer( void )
{
	m_parent = NULL ;
	m_type = PSD::LayerRecord::layerNormal ;
	m_position = SGLPoint( 0, 0 ) ;
	m_blend = PSD::blendNormal ;
	m_visible = true ;
	m_transparency = 0 ;
}

SGLImageComposition::Layer::Layer( const SGLImageComposition::Layer& layer )
{
	DuplicateOf( layer.GetImageObject() ) ;
	//
	m_parent = NULL ;
	m_name = layer.m_name ;
	m_type = layer.m_type ;
	m_position = layer.m_position ;
	m_blend = layer.m_blend ;
	m_visible = layer.m_visible ;
	m_transparency = layer.m_transparency ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLImageComposition::Layer::~Layer( void )
{
}

// 子レイヤーの総数（孫レイヤー以下を含む）
//////////////////////////////////////////////////////////////////////////////
size_t SGLImageComposition::Layer::GetTotalLayerCount( void ) const
{
	size_t	nCount = m_layers.GetLength() ;
	for ( size_t i = 0; i < m_layers.GetLength(); i ++ )
	{
		Layer *	pChild = m_layers.GetAt(i) ;
		if ( pChild != nullptr )
		{
			nCount += pChild->GetTotalLayerCount() ;
		}
	}
	return	nCount ;
}



//////////////////////////////////////////////////////////////////////////////
// 画像コンポジション
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLImageComposition, SObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLImageComposition::SGLImageComposition( void )
	: m_mode( PSD::modeRGB ), m_sizeCanvas( 0, 0 ), m_nChannels( 0 )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLImageComposition::~SGLImageComposition( void )
{
}

// 読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageComposition::LoadPSDFile( const wchar_t * pwszFilePath )
{
	SSmartPointer<SFileInterface>
		pFile = SFileOpener::DefaultNewOpenFile
					( pwszFilePath, SFileOpener::shareRead ) ;
	if ( pFile == NULL )
	{
		return	sglErrFailed ;
	}
	return	ReadPSDFile( *pFile ) ;
}

SGLError SGLImageComposition::ReadPSDFile( SSystem::SFileInterface& file )
{
	PSD::FileReader	psdfr ;
	if ( psdfr.Open( &file, false ) )
	{
		return	sglErrFailed ;
	}
	PSD::FILE_HEADER	fhdr ;
	psdfr.GetFileHeader( fhdr ) ;
	//
	m_mode = (PSD::ImageMode) fhdr.wMode.GetUInt16() ;
	m_sizeCanvas.w = (int32_t) fhdr.dwWidth.GetUInt32() ;
	m_sizeCanvas.h = (int32_t) fhdr.dwHeight.GetUInt32() ;
	m_nChannels = fhdr.wChannels.GetUInt16() ;
	psdfr.GetColorModeData( m_bufColorMode ) ;
	//
	Layer *	pParent = NULL ;
	m_layers.RemoveAll() ;
	m_grouped.RemoveAll() ;
	//
	size_t	nLayerCount = psdfr.GetLayerCount() ;
	for ( size_t i = 0; i < nLayerCount; i ++ )
	{
		const size_t	iLayer = nLayerCount - i - 1 ;
		//
		PSD::FileReader::LayerInfo	liLayer ;
		if ( psdfr.GetLayerInfo( liLayer, iLayer ) )
		{
			return	sglErrFailed ;
		}
		Layer *	pLayer = new Layer ;
		if ( psdfr.LoadLayerImage( *pLayer, iLayer ) )
		{
			return	sglErrFailed ;
		}
		pLayer->m_name = liLayer.strLayerName ;
		pLayer->m_type = liLayer.typeLayer ;
		pLayer->m_position.x = liLayer.rectImage.x ;
		pLayer->m_position.y = liLayer.rectImage.y ;
		pLayer->m_blend = liLayer.dwBlendMode ;
		pLayer->m_visible = ((liLayer.dwFlags & PSD::flagInvisible) == 0) ;
		pLayer->m_transparency = liLayer.nTransparency ;
		//
		m_layers.Add( pLayer ) ;
		pLayer->m_parent = pParent ;
		//
		if ( liLayer.typeLayer == PSD::LayerRecord::layerEndOfGroup
			/*|| ((liLayer.typeLayer == PSD::LayerRecord::layerGroup)
				&& (pLayer->m_name == L"</Layer set>"))*/ )
		{
			pLayer->m_type = PSD::LayerRecord::layerEndOfGroup ;
			//
			if ( pParent != NULL )
			{
				pParent = pParent->m_parent ;
			}
		}
		else
		{
			if ( pParent == NULL )
			{
				m_grouped.Add( pLayer ) ;
			}
			else
			{
				pParent->m_layers.Add( pLayer ) ;
			}
			if ( liLayer.typeLayer == PSD::LayerRecord::layerGroup )
			{
				pParent = pLayer ;
			}
		}
	}
	return	sglErrSuccess ;
}

// 書き出し
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageComposition::SavePSDFile( const wchar_t * pwszFilePath )
{
	SSmartPointer<SFileInterface>
		pFile = SFileOpener::DefaultNewOpenFile
					( pwszFilePath, SFileOpener::modeCreate ) ;
	if ( pFile == NULL )
	{
		return	sglErrFailed ;
	}
	return	WritePSDFile( *pFile ) ;
}

SGLError SGLImageComposition::WritePSDFile( SSystem::SFileInterface& file )
{
	PSD::FileWriter::CanvasInfo	ci ;
	ci.mode = PSD::modeRGB ;
	ci.nWidth = m_sizeCanvas.w ;
	ci.nHeight = m_sizeCanvas.h ;
	ci.nChannels = (uint32_t) m_nChannels ;
	ci.nDepth = 8 ;
	ci.nColorDataByes = m_bufColorMode.GetLength() ;
	ci.pColorModeData = m_bufColorMode.GetConstArray() ;
	//
	PSD::FileWriter	psdfw ;
	if ( psdfw.Open( ci, &file, false ) )
	{
		return	sglErrFailed ;
	}
	for ( size_t i = 0; i < m_layers.GetLength(); i ++ )
	{
		Layer *	pLayer = m_layers.GetLastAt( i ) ;
		if ( pLayer == nullptr )
		{
			continue ;
		}
		if ( pLayer->m_type == PSD::LayerRecord::layerEndOfGroup )
		{
			psdfw.AppendEndOfGroup( pLayer->m_name ) ;
		}
		else if ( pLayer->m_type == PSD::LayerRecord::layerGroup )
		{
			psdfw.AppendGroupLayer( pLayer->m_name ) ;
		}
		else
		{
			psdfw.AppendLayerInfo
				( pLayer->m_name,
					pLayer, pLayer->m_position.x, pLayer->m_position.y,
					(PSD::LayerBlendMode) pLayer->m_blend,
					(0x100 - pLayer->m_transparency) * 0xFF / 0x100,
					pLayer->m_visible ? 0 : PSD::flagInvisible ) ;
		}
	}
	psdfw.WriteAllLayerImages() ;
	//
	SGLImage	imgBlended ;
	CreateBlendedImage( imgBlended ) ;
	psdfw.WriteBaseImage( imgBlended ) ;
	psdfw.Close() ;
	//
	return	sglErrSuccess ;
}

// キャンバス情報
//////////////////////////////////////////////////////////////////////////////
const SGLSize& SGLImageComposition::GetCanvasSize( void ) const
{
	return	m_sizeCanvas ;
}

PSD::ImageMode SGLImageComposition::GetImageMode( void ) const
{
	return	m_mode ;
}

size_t SGLImageComposition::GetChannelCount( void ) const
{
	return	m_nChannels ;
}

void SGLImageComposition::SetCanvasInfo
	( const SGLSize& sizeCanvas, size_t nChannels, PSD::ImageMode mode )
{
	m_sizeCanvas = sizeCanvas ;
	m_mode = mode ;
	m_nChannels = nChannels ;
}

// 合成イメージ生成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageComposition::CreateBlendedImage( SGLImageObject& imgBlended ) const
{
	if ( m_sizeCanvas.IsEmpty() || (m_nChannels == 0) )
	{
		return	sglErrFailed ;
	}
	uint32_t	format = formatImageRGB ;
	uint32_t	depth = (uint32_t) m_nChannels * 8 ;
	if ( m_nChannels >= 32 )
	{
		format = formatImageARGB ;
		depth = 32 ;
	}
	imgBlended.CreateImage
		( m_sizeCanvas.w, m_sizeCanvas.h, format, depth ) ;
	//
	SGLPaintBuffer	paint ;
	paint.AttachTargetImage( &imgBlended, nullptr ) ;
	paint.FillClearTarget( 0 ) ;
	//
	for ( size_t i = 0; i < m_layers.GetLength(); i ++ )
	{
		Layer *	pLayer = m_layers.GetLastAt( i ) ;
		if ( pLayer == nullptr )
		{
			continue ;
		}
		if ( pLayer->m_type == PSD::LayerRecord::layerNormal )
		{
			DrawLayer( paint, *pLayer ) ;
		}
	}
	paint.DetachTargetImage() ;
	return	sglErrSuccess ;
}

// レイヤー数
//////////////////////////////////////////////////////////////////////////////
size_t SGLImageComposition::GetLayerCount( void ) const
{
	return	m_layers.GetLength() ;
}

// レイヤー取得
//////////////////////////////////////////////////////////////////////////////
SGLImageComposition::Layer * SGLImageComposition::GetLayerAt( size_t iLayer ) const
{
	return	m_layers.GetAt( iLayer ) ;
}

// 階層状のレイヤー数
//////////////////////////////////////////////////////////////////////////////
size_t SGLImageComposition::GetTreeLayerCount( void ) const
{
	return	m_grouped.GetLength() ;
}

// 階層状のレイヤー取得
//////////////////////////////////////////////////////////////////////////////
SGLImageComposition::Layer * SGLImageComposition::GetTreeLayerAt( size_t iLayer ) const
{
	return	m_grouped.GetAt( iLayer ) ;
}

// レイヤー追加
//////////////////////////////////////////////////////////////////////////////
void SGLImageComposition::InsertLayerAt
	( size_t iLayer, SGLImageComposition::Layer * pLayer,
							SGLImageComposition::Layer * pParent )
{
	if ( pParent != nullptr )
	{
		ssize_t	iParent = m_layers.FindPtr( pParent ) ;
		ESLAssert( iParent >= 0 ) ;
		if ( iParent >= 0 )
		{
			ssize_t	iGlobalLayer = LocalIndexToGlobal( pParent, iLayer ) ;
			ESLAssert( iGlobalLayer >= 0 ) ;
			size_t	iSubLayer = pParent->m_layers.InsertAt( iLayer, pLayer ) ;
			if ( iGlobalLayer >= 0 )
			{
				m_layers.InsertAt( iGlobalLayer, pLayer ) ;
			}
			else
			{
				m_layers.InsertAt( (size_t) iParent + iSubLayer + 1, pLayer ) ;
			}
			pLayer->m_parent = pParent ;
		}
		else
		{
			m_layers.InsertAt( iLayer, pLayer ) ;
		}
	}
	else
	{
		m_layers.InsertAt( iLayer, pLayer ) ;
	}
}

void SGLImageComposition::AppendLayer
	( SGLImageComposition::Layer * pLayer, SGLImageComposition::Layer * pParent )
{
	if ( pParent != nullptr )
	{
		InsertLayerAt( pParent->m_layers.GetLength(), pLayer, pParent ) ;
	}
	else
	{
		m_layers.Add( pLayer ) ;
	}
}

SGLImageComposition::Layer *
	SGLImageComposition::AppendGroupLayer
		( const wchar_t * pwszLayerName, SGLImageComposition::Layer * pParent )
{
	Layer *	pGroupLayer = new Layer ;
	pGroupLayer->m_name = pwszLayerName ;
	pGroupLayer->m_type = PSD::LayerRecord::layerGroup ;
	AppendLayer( pGroupLayer, pParent ) ;

	Layer *	pEndOfGroup = new Layer ;
	pEndOfGroup->m_name = L"</Layer set>" ;
	pEndOfGroup->m_type = PSD::LayerRecord::layerEndOfGroup ;
	AppendLayer( pEndOfGroup, pParent ) ;

	return	pGroupLayer ;
}

// 階層内のレイヤー番号をフラットなレイヤー番号へ変換
//////////////////////////////////////////////////////////////////////////////
ssize_t SGLImageComposition::LocalIndexToGlobal
	( SGLImageComposition::Layer * pParent, size_t iLayer ) const
{
	if ( pParent == nullptr )
	{
		return	(ssize_t) iLayer ;
	}
	if ( iLayer < pParent->m_layers.GetLength() )
	{
		return	m_layers.FindPtr( pParent->m_layers.GetAt( iLayer ) ) ;
	}
	else
	{
		if ( pParent->m_parent != nullptr )
		{
			ssize_t	iParent = pParent->m_parent->m_layers.FindPtr( pParent ) ;
			ESLAssert( iParent >= 0 ) ;
			if ( iParent < 0 )
			{
				return	-1 ;
			}
			return	LocalIndexToGlobal( pParent->m_parent, (size_t) iParent + 1 ) ;
		}
		else
		{
			ssize_t	iParent = m_layers.FindPtr( pParent ) ;
			ESLAssert( iParent >= 0 ) ;
			if ( iParent < 0 )
			{
				return	-1 ;
			}
			return	iParent + (ssize_t) pParent->GetTotalLayerCount() + 1 ;
		}
	}
}

// レイヤー削除
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageComposition::RemoveLayer( SGLImageComposition::Layer * pLayer )
{
	SGLError	err = DetachLayer( pLayer ) ;
	if ( !err )
	{
		delete	pLayer ;
	}
	return	err ;
}

SGLError SGLImageComposition::DetachLayer( SGLImageComposition::Layer * pLayer )
{
	if ( pLayer == nullptr )
	{
		return	sglErrFailed ;
	}
	Layer *	pParent = pLayer->m_parent ;
	if ( pParent != nullptr )
	{
		ssize_t	iSubLayer = pParent->m_layers.FindPtr( pLayer ) ;
		ESLAssert( iSubLayer >= 0 ) ;
		if ( iSubLayer >= 0 )
		{
			pParent->m_layers.RemoveAt( (size_t) iSubLayer ) ;
		}
	}
	else
	{
		ssize_t	iSubLayer = m_grouped.FindPtr( pLayer ) ;
		ESLAssert( iSubLayer >= 0 ) ;
		if ( iSubLayer >= 0 )
		{
			m_grouped.RemoveAt( (size_t) iSubLayer ) ;
		}
	}
	ssize_t	iLayer = m_layers.FindPtr( pLayer ) ;
	if ( iLayer < 0 )
	{
		return	sglErrFailed ;
	}
	m_layers.DetachAt( (size_t) iLayer ) ;
	pLayer->m_parent = nullptr ;
	return	sglErrSuccess ;
}

// レイヤーサイズ計算
//////////////////////////////////////////////////////////////////////////////
bool SGLImageComposition::LayerSizeOf
	( SGLRect& rect, SGLImageComposition::Layer& layer, int nThreshold )
{
	SGLImageInfo	imginf ;
	const uint8_t *	pbytBuf =
			layer.LockBuffer( imginf, SGLImageObject::lockRead ) ;
	uint32_t	channels = imginf.depth >> 3 ;
	if ( !(imginf.format & formatImageFlagAlpha) )
	{
		if ( channels == 4 )
		{
			channels = 3 ;
		}
	}
	if ( (channels != 1) && (channels != 3) && (channels != 4) )
	{
		layer.UnlockBuffer( SGLImageObject::lockRead ) ;
		rect.left = layer.m_position.x ;
		rect.top = layer.m_position.y ;
		rect.SetWidth( imginf.width ) ;
		rect.SetHeight( imginf.height ) ;
		return	true ;
	}
	SGLRect	rectBounds( 0x7FFFFFFF, 0x7FFFFFFF, -0x7FFFFFFF, -0x7FFFFFFF ) ;
	const uint8_t *	pbytLine = pbytBuf ;
	for ( uint32_t y = 0; y < imginf.height; y ++ )
	{
		const uint8_t *	pbytPixel = pbytLine ;
		bool			fEmptyLine = true ;
		if ( channels == 4 )
		{
			for ( uint32_t x = 0; x < imginf.width; x ++ )
			{
				int	v = pbytPixel[3] ;
				if ( v > nThreshold )
				{
					if ( (int32_t) x < rectBounds.left )
					{
						rectBounds.left = (int32_t) x ;
					}
					if ( (int32_t) x > rectBounds.right )
					{
						rectBounds.right = (int32_t) x ;
					}
					fEmptyLine = false ;
				}
				pbytPixel += imginf.pitchPixel ;
			}
		}
		else if ( channels == 3 )
		{
			for ( uint32_t x = 0; x < imginf.width; x ++ )
			{
				int	v = (int) pbytPixel[0] + pbytPixel[1] + pbytPixel[2] ;
				if ( v > nThreshold )
				{
					if ( (int32_t) x < rectBounds.left )
					{
						rectBounds.left = (int32_t) x ;
					}
					if ( (int32_t) x > rectBounds.right )
					{
						rectBounds.right = (int32_t) x ;
					}
					fEmptyLine = false ;
				}
				pbytPixel += imginf.pitchPixel ;
			}
		}
		else if ( channels == 1 )
		{
			for ( uint32_t x = 0; x < imginf.width; x ++ )
			{
				int	v = pbytPixel[0] ;
				if ( v > nThreshold )
				{
					if ( (int32_t) x < rectBounds.left )
					{
						rectBounds.left = (int32_t) x ;
					}
					if ( (int32_t) x > rectBounds.right )
					{
						rectBounds.right = (int32_t) x ;
					}
					fEmptyLine = false ;
				}
				pbytPixel += imginf.pitchPixel ;
			}
		}
		if ( !fEmptyLine )
		{
			if ( (int32_t) y < rectBounds.top )
			{
				rectBounds.top = (int32_t) y ;
			}
			if ( (int32_t) y > rectBounds.bottom )
			{
				rectBounds.bottom = (int32_t) y ;
			}
		}
		pbytLine += imginf.pitchLine ;
	}
	layer.UnlockBuffer( SGLImageObject::lockRead ) ;
	//
	if ( rectBounds.IsEmpty() )
	{
		return	false ;
	}
	rect = rectBounds ;
	rect.left += layer.m_position.x ;
	rect.top += layer.m_position.y ;
	rect.right += layer.m_position.x ;
	rect.bottom += layer.m_position.y ;
	return	true ;
}

// 切り出し画像生成
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageComposition::LayerCutBack
	( SGLImageObject& imgCutted,
			SGLImageComposition::Layer& layer, const SGLRect& rectCut )
{
	SGLImageInfo	imginf ;
	if ( layer.GetImageInfo( imginf ) )
	{
		return	sglErrFailed ;
	}
	//
	imginf.width = (uint32_t) rectCut.GetWidth() ;
	imginf.height = (uint32_t) rectCut.GetHeight() ;
	imginf.ptOrigin.x = - rectCut.left ;
	imginf.ptOrigin.y = - rectCut.top ;
	//
	if ( imgCutted.CreateBuffer( imginf ) )
	{
		return	sglErrFailed ;
	}
	//
	SGLImageBuffer	bufCutted ;
	bufCutted.ptrBuffer = imgCutted.LockBuffer( bufCutted ) ;
	sglFillImageBuffer( bufCutted, SGLPalette(0) ) ;
	//
	SGLImageBuffer	bufLayer ;
	bufLayer.ptrBuffer =
			layer.LockBuffer( bufLayer, SGLImageObject::lockRead ) ;
	//
	sglCopyImageBuffer
		( bufCutted, bufLayer,
			layer.m_position.x - rectCut.left,
			layer.m_position.y - rectCut.top ) ;
	//
	layer.UnlockBuffer() ;
	imgCutted.UnlockBuffer() ;
	//
	return	sglErrSuccess ;
}

// レイヤー描画
//////////////////////////////////////////////////////////////////////////////
SGLError SGLImageComposition::DrawLayer
	( SGLPaintContextInterface& paint, Layer& layer )
{
	SGLPaintParam	pp ;
	pp.ptPaint = layer.m_position ;
	switch ( layer.m_blend )
	{
	case	PSD::blendNormal:
	default:
		pp.nFlags = 0 ;
		break ;
	case	PSD::blendMultiply:
		pp.nFlags = paintFunctionMul ;
		break ;
	case	PSD::blendDivision:
		pp.nFlags = paintFunctionDiv ;
		break ;
	case	PSD::blendAddition:
		pp.nFlags = paintFunctionAdd ;
		break ;
	case	PSD::blendSubtraction:
		pp.nFlags = paintFunctionSub ;
		break ;
	}
	paint.DrawImage( pp, &layer, NULL ) ;
	return	sglErrSuccess ;
}

