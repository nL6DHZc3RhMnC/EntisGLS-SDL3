
#include <psd_utilities.h>


// ImageContext 内のレイヤーを
// filename + conjunction + layer_name の名前のファイル名で書き出す
//////////////////////////////////////////////////////////////////////////////

void SaveImageEachLayer
	( ImageContext& imgctx, String dstdir,
			String mime, int quality,
			String conjunction, bool fKeepPosition,
			int alignCut, int marginCut, int thresholdCut )
{
	String	sFileTitle = imgctx.filename.ParseFileTitle() ;
	int		flagsCut = ImageContext::cutFlagAutoRemoveAlpha ;
	if ( fKeepPosition )
	{
		flagsCut |= ImageContext::cutFlagKeepHotspot ;
	}
	for ( rLayer : imgctx : iLayer )
	{
		ImageContext	ictxTemp ;
		ictxTemp.merge( imgctx, iLayer, 1 ) ;
		if ( ictxTemp.cut
			( alignCut, marginCut, thresholdCut, flagsCut ) == eslErrSuccess )
		{
			print( rLayer.name + "..." ) ;
			ictxTemp.save
				( dstdir.OffsetFilePath
					( sFileTitle + conjunction + rLayer.name ), mime, quality ) ;
			print( "\n" ) ;
		}
	}
}



// ImageContext 内のレイヤーをアニメーション画像として出力
//////////////////////////////////////////////////////////////////////////////

void SaveAnimationRichLayer
	( ImageContext& imgctx, String dstdir,
			String mime, int quality, bool fKeepPosition,
			int alignCut, int marginCut, int thresholdCut )
{
	int	flagsCut = ImageContext::cutFlagAutoRemoveAlpha ;
	if ( fKeepPosition )
	{
		flagsCut |= ImageContext::cutFlagKeepHotspot ;
	}
	for ( int iLayer = sizeof(imgctx) - 1; iLayer >= 0; iLayer -- )
	{
		Image&	rLayer = imgctx[iLayer] ;
		if ( rLayer.name.Left(1) == "#" )
		{
			ImageContext	ictxTemp ;
			String	sDstName = rLayer.name.Middle(1) ;
			String	sSeqList = "" ;
			ictxTemp.merge( imgctx, iLayer --, 1 ) ;
			while ( iLayer >= 0 )
			{
				if ( imgctx[iLayer].name.Left(1) == "#" )
				{
					break ;
				}
				ictxTemp.merge( imgctx, iLayer --, 1 ) ;
				if ( imgctx[iLayer].name.Left(5) == "@end:" )
				{
					ictxTemp.duration = int( imgctx[iLayer].name.Middle(5) ) ;
					break ;
				}
				if ( imgctx[iLayer].name.Left(5) == "@seq:" )
				{
					sSeqList = imgctx[iLayer].name.Middle(5) ;
				}
			}
			iLayer ++ ;
			//
			print( rLayer.name + "..." ) ;
			ictxTemp.animation( sSeqList ) ;
			ictxTemp.save
				( dstdir.OffsetFilePath( sDstName ), mime, quality ) ;
			print( "\n" ) ;
		}
		else
		{
			ImageContext	ictxTemp ;
			ictxTemp.merge( imgctx, iLayer, 1 ) ;
			if ( ictxTemp.cut
				( alignCut, marginCut, thresholdCut, flagsCut ) == eslErrSuccess )
			{
				print( rLayer.name + "..." ) ;
				ictxTemp.save
					( dstdir.OffsetFilePath( rLayer.name ), mime, quality ) ;
				print( "\n" ) ;
			}
		}
	}
}



// ImageContext 内のレイヤーをスキンデータとして出力
//////////////////////////////////////////////////////////////////////////////

int MakeSkinRichLayer
	( ImageContext& imgctx, String dstdir,
		String mime, bool fKeepPosition,
		int alignCut, int marginCut, int thresholdCut )
{
	SkinContext	skin ;
	return	skin.MakeSkin
		( imgctx, dstdir, mime,
			fKeepPosition, alignCut, marginCut, thresholdCut ) ;
}



//////////////////////////////////////////////////////////////////////////////
// スキンデータ
//////////////////////////////////////////////////////////////////////////////

// 初期設定
//////////////////////////////////////////////////////////////////////////////
void SkinPageData::InitializePage( String sID, int nWidth, int nHeight )
{
	if ( m_xmlPage === null )
	{
		m_xmlPage ::= XMLDocument ;
		m_xmlPage.SetTag( "page" ) ;
	}
	m_xmlPage.SetAttributeAs( "id", sID ) ;
	m_xmlPage.SetAttrIntegerAs( "width", nWidth ) ;
	m_xmlPage.SetAttrIntegerAs( "height", nHeight ) ;
}

// アイテム追加
//////////////////////////////////////////////////////////////////////////////
SkinItemStyle& SkinPageData::AddItem( void )
{
	SkinItemStyle&	item = m_items[sizeof(m_items)] ::= SkinItemStyle ;
	item.m_xmlStyle ::= XMLDocument ;
	m_xmlPage.AddElement( item.m_xmlStyle ) ;
	return	item ;
}

// ページの位置とサイズを最適化
//////////////////////////////////////////////////////////////////////////////
void SkinPageData::SmartPage( void )
{
	if ( m_iAlign <= 0 )
	{
		return ;
	}
	Rect	rectPage( 0x7FFF, 0x7FFF, -0x8000, -0x8000 ) ;
	for ( item : m_items )
	{
		if ( rectPage.left > item.m_ptStyle.x )
		{
			rectPage.left = item.m_ptStyle.x ;
		}
		if ( rectPage.top > item.m_ptStyle.y )
		{
			rectPage.top = item.m_ptStyle.y ;
		}
		if ( rectPage.right < item.m_ptStyle.x + item.m_sizeStyle.w )
		{
			rectPage.right = item.m_ptStyle.x + item.m_sizeStyle.w ;
		}
		if ( rectPage.bottom < item.m_ptStyle.y + item.m_sizeStyle.h )
		{
			rectPage.bottom = item.m_ptStyle.y + item.m_sizeStyle.h ;
		}
	}
	if ( (rectPage.right > rectPage.left)
		&& (rectPage.bottom > rectPage.top) )
	{
		rectPage.right += m_iAlign - 1 ;
		rectPage.bottom += m_iAlign - 1 ;
		rectPage.left -= (rectPage.left % m_iAlign) ;
		rectPage.top -= (rectPage.top % m_iAlign) ;
		rectPage.right -= (rectPage.right % m_iAlign) ;
		rectPage.bottom -= (rectPage.bottom % m_iAlign) ;
		//
		m_xmlPage.SetAttrIntegerAs
			( "width", rectPage.right - rectPage.left ) ;
		m_xmlPage.SetAttrIntegerAs
			( "height", rectPage.bottom - rectPage.top ) ;
		//
		OffsetItems( - rectPage.left, - rectPage.top ) ;
	}
}

// アイテム座標変更
//////////////////////////////////////////////////////////////////////////////
void SkinPageData::OffsetItems( int xOffset, int yOffset )
{
	m_xmlPage.SetAttrIntegerAs
		( "x", m_xmlPage.GetAttrIntegerAs( "x", 0 ) - xOffset ) ;
	m_xmlPage.SetAttrIntegerAs
		( "y", m_xmlPage.GetAttrIntegerAs( "y", 0 ) - yOffset ) ;
	//
	for ( item : m_items )
	{
		item.m_ptStyle.x += xOffset ;
		item.m_ptStyle.y += yOffset ;
		item.m_xmlStyle.SetAttrIntegerAs( "x", item.m_ptStyle.x ) ;
		item.m_xmlStyle.SetAttrIntegerAs( "y", item.m_ptStyle.y ) ;
	}
}


// レイヤー解釈
//////////////////////////////////////////////////////////////////////////////
int SkinContext::MakeSkin
	( ImageContext& imgctx, String dstdir,
		String mime, bool fKeepPosition,
		int alignCut, int marginCut, int thresholdCut )
{
	m_dstdir = dstdir ;
	m_mimeImage = mime ;
	m_flagsCut = ImageContext::cutFlagAutoRemoveAlpha ;
	if ( fKeepPosition )
	{
		m_flagsCut |= ImageContext::cutFlagKeepHotspot ;
	}
	m_alignCut = alignCut ;
	m_marginCut = marginCut ;
	m_thresholdCut = thresholdCut ;
	//
	for ( int iLayer = sizeof(imgctx) - 1; iLayer >= 0; iLayer -- )
	{
		//
		// タグレイヤー名解釈
		//
		String	sTagLayer = imgctx[iLayer].name ;
		String	sTagType = sTagLayer.NextNeedChar( "$#" ) ;
		if ( (sTagType != "#") && (sTagType != "$") )
		{
			continue ;
		}
		Hash<SkinItemLayerProperty>	mapProperty ;
		String	strTagID = sTagLayer.NextToken() ;
		if ( sTagLayer.NextNeedChar( ":" ) == ":" )
		{
			for ( ; ; )
			{
				String	sName = sTagLayer.NextToken() ;
				if ( sName == "" )
				{
					break ;
				}
				if ( sTagLayer.NextNeedChar( "=" ) == "=" )
				{
					String	sValue = sTagLayer.NextEnclosedString( ";" ) ;
					sValue.TrimLeft() ;
					sValue.TrimRight() ;
					//
					SkinItemLayerProperty&	prop = mapProperty[sName] ;
					prop.sName = sName ;
					prop.iLayer = -1 ;
					prop.sProperty = sValue ;
				}
				else
				{
					OutputError( "タグレイヤーの書式エラー\n" + sTagLayer ) ;
					break ;
				}
			}
		}
		//
		// # 区間抽出
		//
		ImageContext	ictxTemp ;
		ictxTemp.merge( imgctx, iLayer --, 1 ) ;
		while ( iLayer >= 0 )
		{
			String	sLayer = imgctx[iLayer].name ;
			String	sSelChar = sLayer.NextNeedChar( "$#@" ) ;
			if ( (sSelChar == "#") || (sSelChar == "$") )
			{
				iLayer ++ ;
				break ;
			}
			if ( sSelChar == "@" )
			{
				for ( ; ; )
				{
					String	sName = sLayer.NextToken() ;
					if ( sName == "" )
					{
						break ;
					}
					if ( !mapProperty.IsEmpty( sName ) )
					{
						String	sBaseName = sName ;
						for ( int i = 1; i < 0x10000; i ++ )
						{
							sName = sBaseName + "@" + String(i) ;
							if ( mapProperty.IsEmpty( sName ) )
							{
								break ;
							}
						}
					}
					SkinItemLayerProperty&
							prop = mapProperty[sName] ;
					prop.sName = sName ;
					prop.iLayer = sizeof(ictxTemp) ;
					//
					String	sEqu = sLayer.NextNeedChar( ":=" ) ;
					if ( (sEqu == ":") || (sEqu == "=") )
					{
						sLayer.SeekNext() ;
						prop.sProperty = sLayer.NextEnclosedString( ";" ) ;
						prop.sProperty.TrimRight() ;
					}
					else
					{
						sLayer.NextEnclosedString( ";" ) ;
					}
					ictxTemp.merge( imgctx, iLayer, 1 ) ;
				}
			}
			iLayer -- ;
		}
		//
		if ( sTagType == "$" )
		{
			//
			// ページ
			//
			if ( m_curPage !== null )
			{
				m_curPage.SmartPage() ;
			}
			if ( strTagID != "" )
			{
				m_curPage ::= m_pages[strTagID] ;
				m_curPage.InitializePage
					( strTagID, imgctx.size.w, imgctx.size.h ) ;
				m_curPage.m_iAlign = m_alignCut ;
				if ( !mapProperty.IsEmpty( "align" ) )
				{
					m_curPage.m_iAlign = int( mapProperty["align"].sProperty ) ;
				}
				print( "page \'", strTagID, "\'\n" ) ;
			}
			else
			{
				print( "page <null>\n" ) ;
				m_curPage ::= null ;
			}
			continue ;
		}
		//
		// アイテムタイプとスタイルID正規化
		//
		String	sItemType = m_sDefItemType ;
		if ( !mapProperty.IsEmpty( "type" ) )
		{
			sItemType = mapProperty["type"].sProperty ;
		}
		m_sDefItemType = sItemType ;
		//
		String	sStyleID = strTagID ;
		if ( !mapProperty.IsEmpty( "style" ) )
		{
			sStyleID = mapProperty["style"].sProperty ;
			if ( !m_styles.IsEmpty( sStyleID ) )
			{
				OutputError
					( "@style:" + sStyleID
						+ " は既に定義されているので @ref_style へ置き換えます" ) ;
				//
				SkinItemLayerProperty&	prop = mapProperty["ref_style"] ;
				prop.sName = "ref_style" ;
				prop.iLayer = -1 ;
				prop.sProperty = sStyleID ;
			}
		}
		else
		{
			if ( !m_styles.IsEmpty( sStyleID ) )
			{
				String	sStyleBaseID = sStyleID ;
				if ( sStyleBaseID.Left(3) == "ID_" )
				{
					sStyleBaseID = "ID_" + sItemType + sStyleBaseID.Middle(2) ;
					sStyleBaseID.MakeUpper() ;
				}
				else
				{
					sStyleBaseID = sItemType + "_" + sStyleBaseID ;
					sStyleBaseID.MakeUpper() ;
				}
				for ( int i = 0; i < 0x10000; i ++ )
				{
					sStyleID = sStyleBaseID ;
					if ( i > 0 )
					{
						sStyleID += String(i) ;
					}
					if ( m_styles.IsEmpty( sStyleID ) )
					{
						break ;
					}
				}
			}
		}
		//
		// スタイル登録
		//
		if ( mapProperty.IsEmpty( "ref_style" ) )
		{
			if ( sItemType == "image" )
			{
				sStyleID = AddImageResource
					( ictxTemp, sItemType, sStyleID, mapProperty ) ;
			}
			else if ( sItemType == "anime" )
			{
				sStyleID = AddAnimationResource
					( ictxTemp, sItemType, sStyleID, mapProperty ) ;
			}
			else if ( (sItemType == "button")
				|| (sItemType == "radio") || (sItemType == "check") )
			{
				sStyleID = AddButtonStyle
					( ictxTemp, sItemType, sStyleID, mapProperty ) ;
			}
			else if ( sItemType == "scroll" )
			{
				sStyleID = AddScrollBarStyle
					( ictxTemp, sItemType, sStyleID, mapProperty ) ;
			}
			else if ( sItemType == "progress" )
			{
				sStyleID = AddProgressBarStyle
					( ictxTemp, sItemType, sStyleID, mapProperty ) ;
			}
			else if ( sItemType == "frame" )
			{
				sStyleID = AddFrameStyle
					( ictxTemp, sItemType, sStyleID, mapProperty ) ;
			}
			else if ( sItemType == "text" )
			{
				sStyleID = AddTextStyle
					( ictxTemp, sItemType, sStyleID, mapProperty ) ;
			}
			SkinItemLayerProperty&	prop = mapProperty["ref_style"] ;
			prop.sName = "ref_style" ;
			prop.iLayer = -1 ;
			prop.sProperty = sStyleID ;
		}
		//
		// アイテム追加
		//
		if ( !mapProperty.IsEmpty( "ref_style" ) )
		{
			if ( m_curPage === null )
			{
				OutputError
					( "\'" + sTagLayer
						+ "\' はページ外に記述されたレイヤーです" ) ;
				continue ;
			}
			String	sRefStyle = mapProperty["ref_style"].sProperty ;
			Point	ptItem ;
			Size	sizeItem ;
			ictxTemp.cut
				( 1, 0, 0,
					ImageContext::cutFlagKeepHotspot
						| ImageContext::cutFlagAutoRemoveAlpha,
					ptItem, sizeItem ) ;
			//
			SkinItemStyle&	item ;
			if ( (sItemType == "image") || (sItemType == "anime") )
			{
				// 画像アイテム
				print( "  image item \'", strTagID, "\'\n" ) ;
				item ::= AddImageItem
					( sItemType, strTagID, sRefStyle,
							mapProperty, ptItem, sizeItem ) ;
			}
			else if ( (sItemType == "button")
				|| (sItemType == "radio") || (sItemType == "check") )
			{
				// ボタンアイテム
				print( "  button item \'", strTagID, "\'\n" ) ;
				item ::= AddButtonItem
					( sItemType, strTagID, sRefStyle,
							mapProperty, ptItem, sizeItem ) ;
			}
			else if ( sItemType == "scroll" )
			{
				// スクロール・バー
				print( "  scroll item \'", strTagID, "\'\n" ) ;
				item ::= AddScrollBarItem
					( sItemType, strTagID, sRefStyle,
							mapProperty, ptItem, sizeItem ) ;
			}
			else if ( sItemType == "progress" )
			{
				// 進捗バー
				print( "  progress item \'", strTagID, "\'\n" ) ;
				item ::= AddProgressBarItem
					( sItemType, strTagID, sRefStyle,
							mapProperty, ptItem, sizeItem ) ;
			}
			else if ( sItemType == "frame" )
			{
				// フレーム
				print( "  frame item \'", strTagID, "\'\n" ) ;
				item ::= AddFrameItem
					( sItemType, strTagID, sRefStyle,
							mapProperty, ptItem, sizeItem ) ;
			}
			else if ( sItemType == "text" )
			{
				// スタティック・テキスト
				print( "  text item \'", strTagID, "\'\n" ) ;
				item ::= AddTextItem
					( sItemType, strTagID, sRefStyle,
							mapProperty, ptItem, sizeItem ) ;
			}
			else if ( sItemType == "rectangle" )
			{
				// 矩形
				print( "  rectangle item \'", strTagID, "\'\n" ) ;
				item ::= AddRectangleItem
					( sItemType, strTagID,
							mapProperty, ptItem, sizeItem ) ;
			}
			if ( item !== null )
			{
				if ( !mapProperty.IsEmpty( "group" ) )
				{
					item.m_xmlStyle.SetAttributeAs
						( "group", mapProperty["group"].sProperty ) ;
				}
				if ( !mapProperty.IsEmpty( "focusable" ) )
				{
					item.m_xmlStyle.SetAttributeAs
						( "tab_stop", mapProperty["focusable"].sProperty ) ;
				}
				if ( !mapProperty.IsEmpty( "key_input" ) )
				{
					item.m_xmlStyle.
						CreateElementTagAs("command").
						CreateElementTagAs("input").
						SetAttributeAs( "key",
								mapProperty["key_input"].sProperty ) ;
				}
				if ( !mapProperty.IsEmpty( "mouse_wheel" ) )
				{
					item.m_xmlStyle.
						CreateElementTagAs("command").
						CreateElementTagAs("input").
						SetAttributeAs( "wheel",
								mapProperty["mouse_wheel"].sProperty ) ;
				}
				if ( !mapProperty.IsEmpty( "unclickable" ) )
				{
					item.m_xmlStyle.
						CreateElementTagAs("command").
						CreateElementTagAs("basic_flag").
						SetAttributeAs( "hit_transparency",
								mapProperty["unclickable"].sProperty ) ;
				}
			}
		}
	}
	if ( m_curPage !== null )
	{
		m_curPage.SmartPage() ;
	}
	return	m_countError ;
}

// system.inf 書き出し
//////////////////////////////////////////////////////////////////////////////
Error SkinContext::SaveSkinDescription( String filename )
{
	XMLDocument	xmlSkin ;
	xmlSkin.SetTag( "skin" ) ;
	//
	XMLDocument&	xmlResource = XMLDocument ;
	xmlResource.SetTag( "resource" ) ;
	xmlSkin.AddElement( xmlResource ) ;
	for ( rsrc : m_rsrc )
	{
		xmlResource.AddElement( XMLDocument(rsrc.m_xmlStyle) ) ;
	}
	//
	XMLDocument&	xmlStyles = XMLDocument ;
	xmlStyles.SetTag( "declare_style" ) ;
	xmlSkin.AddElement( xmlStyles ) ;
	for ( style : m_styles )
	{
		xmlStyles.AddElement( XMLDocument(style.m_xmlStyle) ) ;
	}
	//
	for ( page : m_pages )
	{
		xmlSkin.AddElement( XMLDocument(page.m_xmlPage) ) ;
	}
	return	xmlSkin.SaveDocument( filename ) ;
}

// エラー出力
//////////////////////////////////////////////////////////////////////////////
void SkinContext::OutputError( String sErrMsg )
{
	print( sErrMsg, "\n" ) ;
	m_countError ++ ;
}

// 画像リソースを保存して登録
//////////////////////////////////////////////////////////////////////////////
String SkinContext::SaveImageResource
	( ImageContext& imgctx, String sRsrcID,
		const Point& ptImage, const Size& sizeImage )
{
	if ( !m_rsrc.IsEmpty( sRsrcID ) )
	{
		OutputError( "画像ID \'" + sRsrcID + "\' は重複しています" ) ;
	}
	SkinResource&	rsrc = m_rsrc[sRsrcID] ;
	//
	rsrc.m_ptStyle = ptImage ;
	rsrc.m_sizeStyle = sizeImage ;
	rsrc.m_strFile = sRsrcID + ".eri" ;
	rsrc.m_ptHotSpot = imgctx.hotspot ;
	//
	rsrc.m_xmlStyle ::= XMLDocument ;
	rsrc.m_xmlStyle.SetTag( "image" ) ;
	rsrc.m_xmlStyle.SetAttributeAs( "id", sRsrcID ) ;
	rsrc.m_xmlStyle.SetAttributeAs( "src", rsrc.m_strFile ) ;
	//
	if ( imgctx.save
		( m_dstdir.OffsetFilePath(rsrc.m_strFile), m_mimeImage ) != eslErrSuccess )
	{
		OutputError( "\'" + rsrc.m_strFile + "\' の書き出しに失敗しました" ) ;
	}
	return	sRsrcID ;
}

// 画像リソースを登録
//////////////////////////////////////////////////////////////////////////////
String SkinContext::AddImageResource
	( ImageContext& imgctx,
		String sType, String sRsrcID,
		Hash<SkinItemLayerProperty>& mapProperty )
{
	ImageContext	ictxTemp ;
	String[]		aPostfix ;
	if ( !mapProperty.IsEmpty( "id" ) )
	{
		sRsrcID = mapProperty["id"].sProperty ;
	}
	int	flagsCut = m_flagsCut ;
	if ( !mapProperty.IsEmpty( "hotspot" ) )
	{
		if ( mapProperty["hotspot"].sProperty == "keep" )
		{
			flagsCut |= ImageContext::cutFlagKeepHotspot ;
		}
		else
		{
			flagsCut &= ~ImageContext::cutFlagKeepHotspot ;
		}
	}
	if ( !mapProperty.IsEmpty( "image" ) )
	{
		ictxTemp.merge( imgctx, mapProperty["image"].iLayer, 1 ) ;
		aPostfix += "" ;
	}
	for ( int i = 0; i < 0x10000; i ++ )
	{
		String	sProp = "image" + String(i) ;
		if ( !mapProperty.IsEmpty( sProp ) )
		{
			ictxTemp.merge( imgctx, mapProperty[sProp].iLayer, 1 ) ;
			aPostfix += String(i) ;
		}
		else if ( i >= 0x100 )
		{
			break ;
		}
	}
	if ( sizeof(ictxTemp) == 0 )
	{
		ictxTemp.merge( imgctx, 0, 1 ) ;
		aPostfix += "" ;
	}
	Point	ptImage ;
	Size	sizeImage ;
	ictxTemp.cut
		( m_alignCut, m_marginCut, m_thresholdCut,
			flagsCut, ptImage, sizeImage ) ;
	//
	for ( int i = 0; i < sizeof(ictxTemp); i ++ )
	{
		ImageContext	ictxSave ;
		ictxSave.merge( ictxTemp, i, 1 ) ;
		ictxSave.hotspot = ictxTemp.hotspot ;
		//
		SaveImageResource
			( ictxSave, sRsrcID + aPostfix[i], ptImage, sizeImage ) ;
	}
	return	sRsrcID + aPostfix[0] ;
}


String SkinContext::AddAnimationResource
	( ImageContext& imgctx,
		String sType, String sRsrcID,
		Hash<SkinItemLayerProperty>& mapProperty )
{
	ImageContext	ictxTemp ;
	if ( !mapProperty.IsEmpty( "id" ) )
	{
		sRsrcID = mapProperty["id"].sProperty ;
	}
	if ( !mapProperty.IsEmpty( "image" ) )
	{
		if ( !mapProperty.IsEmpty( "image" ) )
		{
			ictxTemp.merge( imgctx, mapProperty["image"].iLayer, 1 ) ;
		}
		for ( int i = 1; i < 0x10000; i ++ )
		{
			String	sProp = "image" + String(i) ;
			if ( !mapProperty.IsEmpty( sProp ) )
			{
				ictxTemp.merge( imgctx, mapProperty[sProp].iLayer, 1 ) ;
			}
			else if ( i >= 0x100 )
			{
				break ;
			}
		}
	}
	else
	{
		ictxTemp.merge( imgctx, 0, 1 ) ;
	}
	Point	ptImage ;
	Size	sizeImage ;
	ictxTemp.cut
		( m_alignCut, m_marginCut, m_thresholdCut,
			m_flagsCut, ptImage, sizeImage ) ;
	//
	int	duration = 0 ;
	if ( !mapProperty.IsEmpty( "duration" ) )
	{
		duration = int( mapProperty["duration"].sProperty ) ;
	}
	ictxTemp.animation() ;
	ictxTemp.duration = duration ;
	//
	SaveImageResource
		( ictxTemp, sRsrcID, ptImage, sizeImage ) ;
	//
	return	sRsrcID ;
}

// テキストスタイルを登録
//////////////////////////////////////////////////////////////////////////////
String SkinContext::AddTextStyle
	( ImageContext& imgctx, String sType, String sStyleID,
		Hash<SkinItemLayerProperty>& mapProperty )
{
	if ( !mapProperty.IsEmpty( "style" ) )
	{
		sStyleID = mapProperty["style"].sProperty ;
	}
	if ( !m_styles.IsEmpty( sStyleID ) )
	{
		OutputError( "スタイルID \'" + sStyleID + "\' は重複しています" ) ;
	}
	SkinItemStyle&	style = m_styles[sStyleID] ;
	//
	ImageContext	ictxTemp ;
	ictxTemp.merge( imgctx, 0, 1 ) ;
	ictxTemp.cut
		( m_alignCut, m_marginCut, m_thresholdCut,
			m_flagsCut, style.m_ptStyle, style.m_sizeStyle ) ;
	//
	style.m_xmlStyle ::= XMLDocument ;
	style.m_xmlStyle.SetTag( "style" ) ;
	style.m_xmlStyle.SetAttributeAs( "type", "static_text" ) ;
	//
	String[][]	aPropNames =
	{
		// prop / sub-tag / attr-name / def-value
		{ "align", "arrange", "align", "left" },
		{ "size", "font", "size", "16" },
		{ "line_height", "arrange", "line_height", "" },
		{ "pitch", "arrange", "pitch", "" },
		{ "font", "font", "face", "Default" },
		{ "bold", "font", "bold", "false" },
		{ "italic", "font", "italic", "false" },
		{ "indent", "arrange", "indent", "0" },
		{ "color", "text", "<color>", "FFFFFFFF" },
		{ "shadow_color", "shadow", "<color>", "00000000" },
		{ "shadow_x", "shadow", "x", "1" },
		{ "shadow_y", "shadow", "y", "1" },
		{ "border_color", "border", "<color>", "00000000" },
	} ;
	for ( prop : aPropNames )
	{
		XMLDocument&	xmlTag =
				style.m_xmlStyle.CreateElementTagAs( prop[1] ) ;
		String	sValue = prop[3] ;
		if ( !mapProperty.IsEmpty( prop[0] ) )
		{
			sValue = mapProperty[prop[0]].sProperty ;
		}
		if ( sValue != "" )
		{
			if ( prop[2] == "<color>" )
			{
				sValue = "0" + sValue + "H" ;
				sValue.SeekIndex( 0 ) ;
				int	color = sValue.NextInteger() ;
				int	alpha = (color >> 24) & 0xFF ;
				xmlTag.SetAttributeAs
					( "color", "0" + color.Format(16,6) + "H" ) ;
				xmlTag.SetAttrIntegerAs
					( "transparency", 0x100 - (alpha * 0x100 / 0xFF) ) ;
			}
			else
			{
				xmlTag.SetAttributeAs( prop[2], sValue ) ;
			}
		}
	}
	return	sStyleID ;
}

// ボタンスタイルを登録
//////////////////////////////////////////////////////////////////////////////
String SkinContext::AddButtonStyle
	( ImageContext& imgctx, String sType, String sStyleID,
		Hash<SkinItemLayerProperty>& mapProperty )
{
	if ( !mapProperty.IsEmpty( "style" ) )
	{
		sStyleID = mapProperty["style"].sProperty ;
	}
	if ( !m_styles.IsEmpty( sStyleID ) )
	{
		OutputError( "スタイルID \'" + sStyleID + "\' は重複しています" ) ;
	}
	SkinItemStyle&	style = m_styles[sStyleID] ;
	//
	style.m_xmlStyle ::= XMLDocument ;
	style.m_xmlStyle.SetTag( "style" ) ;
	style.m_xmlStyle.SetAttributeAs( "id", sStyleID ) ;
	style.m_xmlStyle.SetAttributeAs( "type", sType ) ;
	//
	String[][]	aPropNames =
	{
		// prop / sub-tag / attr-name / def-value
		{ "type", "arrange", "type", "button" },
		{ "mask", "mask", "<mask>", "" },
		{ "normal", "normal", "<image>", "" },
		{ "focus", "focus", "<image>", "" },
		{ "pushed", "pushed", "<image>", "" },
		{ "pushed_focus", "pushed_focus", "<image>", "" },
		{ "active", "active", "<image>", "" },
		{ "active_pushed", "active_pushed", "<image>", "" },
		{ "disabled", "disabled", "<image>", "" },
		{ "push_disabled", "push_disabled", "<image>", "" },
	} ;
	//
	// ボタン画像収集
	//
	ImageContext	ictxImages ;
	Hash<int>		mapImages ;
	for ( prop : aPropNames )
	{
		if ( !mapProperty.IsEmpty( prop[0] ) )
		{
			if ( (prop[2] == "<mask>") || (prop[2] == "<image>") )
			{
				mapImages[prop[0]] = sizeof(ictxImages) ;
				ictxImages.merge( imgctx, mapProperty[prop[0]].iLayer, 1 ) ;
			}
		}
	}
	Point	ptImage, ptText ;
	Size	sizeImage, sizeText ;
	ictxImages.cut
		( m_alignCut, m_marginCut, m_thresholdCut,
			m_flagsCut, ptImage, sizeImage ) ;
	//
	// テキスト領域取得
	//
	ImageContext	ictxText ;
	String			sFontStyle ;
	if ( !mapProperty.IsEmpty( "font_style" ) )
	{
		sFontStyle = mapProperty["font_style"].sProperty ;
		if ( !m_styles.IsEmpty( sFontStyle ) )
		{
			ictxText.merge( imgctx, mapProperty["font_style"].iLayer, 1 ) ;
			if ( ictxText.cut
				( m_alignCut, m_marginCut, m_thresholdCut,
					m_flagsCut, ptText, sizeText ) == eslErrSuccess )
			{
				ptText -= ptImage ;
			}
			else
			{
				ptText = m_styles[sFontStyle].m_ptStyle - ptImage ;
				sizeText = m_styles[sFontStyle].m_sizeStyle ;
			}
		}
		else
		{
			OutputError
				( "フォントスタイル \'"
					+ sFontStyle + "\' は未定義です" ) ;
			sFontStyle = "" ;
		}
	}
	//
	// 属性設定
	//
	for ( prop : aPropNames )
	{
		if ( !mapProperty.IsEmpty( prop[0] ) )
		{
			XMLDocument&	xmlTag =
					style.m_xmlStyle.CreateElementTagAs( prop[1] ) ;
			String	sValue = mapProperty[prop[0]].sProperty ;
			if ( prop[2] == "<mask>" )
			{
				if ( sValue == "rect" )
				{
					xmlTag.SetAttributeAs( "rect", "true" ) ;
				}
				ImageContext	ictxMask ;
				ictxMask.merge( ictxImages, mapImages[prop[0]], 1 ) ;
				xmlTag.SetAttributeAs
					( "image", SaveImageResource
						( ictxMask, sStyleID + "_MASK", ptImage, sizeImage ) ) ;
			}
			else if ( prop[2] == "<image>" )
			{
				ImageContext	ictxImage ;
				ictxImage.merge( ictxImages, mapImages[prop[0]], 1 ) ;
				//
				String	sImageID = sStyleID + "_" + prop[0] ;
				sImageID.MakeUpper() ;
				xmlTag.SetAttributeAs
					( "image", SaveImageResource
						( ictxImage, sImageID, ptImage, sizeImage ) ) ;
				//
				if ( sFontStyle != "" )
				{
					XMLDocument&	xmlFontStyle = m_styles[sFontStyle].m_xmlStyle ;
					for ( int i = 0; i < xmlFontStyle.GetElementsCount(); i ++ )
					{
						xmlTag.AddElement
							( XMLDocument(xmlFontStyle.GetElementAt(i)) ) ;
					}
					XMLDocument&
						xmlArrange = xmlTag.CreateElementTagAs( "arrange" ) ;
					xmlArrange.SetAttrIntegerAs( "left", ptText.x ) ;
					xmlArrange.SetAttrIntegerAs( "top", ptText.y ) ;
					xmlArrange.SetAttrIntegerAs( "width", sizeText.w ) ;
					xmlArrange.SetAttrIntegerAs( "height", sizeText.h ) ;
				}
			}
			else
			{
				xmlTag.SetAttributeAs( prop[2], sValue ) ;
			}
		}
	}
	return	sStyleID ;
}

// フレームスタイルを登録
//////////////////////////////////////////////////////////////////////////////
String SkinContext::AddFrameStyle
	( ImageContext& imgctx, String sType, String sStyleID,
		Hash<SkinItemLayerProperty>& mapProperty )
{
	if ( !mapProperty.IsEmpty( "style" ) )
	{
		sStyleID = mapProperty["style"].sProperty ;
	}
	if ( !m_styles.IsEmpty( sStyleID ) )
	{
		OutputError( "スタイルID \'" + sStyleID + "\' は重複しています" ) ;
	}
	SkinItemStyle&	style = m_styles[sStyleID] ;
	//
	style.m_xmlStyle ::= XMLDocument ;
	style.m_xmlStyle.SetTag( "style" ) ;
	style.m_xmlStyle.SetAttributeAs( "id", sStyleID ) ;
	style.m_xmlStyle.SetAttributeAs( "type", "static_frame" ) ;
	//
	String[]	aPropNames =
	{
		"upper_left", "upper", "upper_right",
		"left", "pane", "right",
		"under_left", "under", "under_right"
	} ;
	XMLDocument&	xmlImage =
			style.m_xmlStyle.CreateElementTagAs( "image" ) ;
	for ( prop : aPropNames )
	{
		if ( !mapProperty.IsEmpty( prop ) )
		{
			ImageContext	ictxFrame ;
			ictxFrame.merge( imgctx, mapProperty[prop].iLayer, 1 ) ;
			//
			Point	ptImage ;
			Size	sizeImage ;
			ictxFrame.cut
				( m_alignCut, m_marginCut, m_thresholdCut,
					m_flagsCut, ptImage, sizeImage ) ;
			//
			String	sImageID = sStyleID + "_" + prop ;
			sImageID.MakeUpper() ;
			xmlImage.SetAttributeAs
				( prop, SaveImageResource
					( ictxFrame, sImageID, ptImage, sizeImage ) ) ;
		}
	}
	return	sStyleID ;
}

// 進捗バースタイルを登録
//////////////////////////////////////////////////////////////////////////////
String SkinContext::AddProgressBarStyle
	( ImageContext& imgctx, String sType, String sStyleID,
		Hash<SkinItemLayerProperty>& mapProperty )
{
	if ( !mapProperty.IsEmpty( "style" ) )
	{
		sStyleID = mapProperty["style"].sProperty ;
	}
	if ( !m_styles.IsEmpty( sStyleID ) )
	{
		OutputError( "スタイルID \'" + sStyleID + "\' は重複しています" ) ;
	}
	SkinItemStyle&	style = m_styles[sStyleID] ;
	//
	style.m_xmlStyle ::= XMLDocument ;
	style.m_xmlStyle.SetTag( "style" ) ;
	style.m_xmlStyle.SetAttributeAs( "id", sStyleID ) ;
	style.m_xmlStyle.SetAttributeAs( "type", "progress_bar" ) ;
	//
	String[][]	aPropNames =
	{
		// prop / sub-tag / attr-name / def-value
		{ "arrange", "arrange", "type", "horz" },
		{ "frame_left", "frame", "left", "<image>" },
		{ "frame", "frame", "way", "<image>" },
		{ "frame_right", "frame", "right", "<image>" },
		{ "bar_left", "bar", "left", "<image>" },
		{ "bar", "bar", "way", "<image>" },
		{ "bar_right", "bar", "right", "<image>" },
	} ;
	for ( prop : aPropNames )
	{
		if ( !mapProperty.IsEmpty( prop[0] ) )
		{
			XMLDocument&	xmlTag =
					style.m_xmlStyle.CreateElementTagAs( prop[1] ) ;
			String	sValue = prop[3] ;
			if ( prop[3] == "<image>" )
			{
				ImageContext	ictxBar ;
				ictxBar.merge( imgctx, mapProperty[prop[0]].iLayer, 1 ) ;
				//
				Point	ptImage ;
				Size	sizeImage ;
				ictxBar.cut
					( m_alignCut, m_marginCut, m_thresholdCut,
						m_flagsCut, ptImage, sizeImage ) ;
				//
				String	sImageID = sStyleID + "_" + prop[0] ;
				sImageID.MakeUpper() ;
				xmlTag.SetAttributeAs
					( prop[2], SaveImageResource
						( ictxBar, sImageID, ptImage, sizeImage ) ) ;
			}
			else
			{
				sValue = mapProperty[prop[0]].sProperty ;
				xmlTag.SetAttributeAs( prop[2], sValue ) ;
			}
		}
	}
	String	sFrameLeft = style.m_xmlStyle.GetContentsAsString( "frame\\left" ) ;
	String	sBarLeft = style.m_xmlStyle.GetContentsAsString( "bar\\left" ) ;
	if ( (sFrameLeft != "") && (sBarLeft != "")
		&& !m_rsrc.IsEmpty(sFrameLeft) && !m_rsrc.IsEmpty(sBarLeft) )
	{
		Point	ptBarOffset = m_rsrc[sBarLeft].m_ptStyle ;
		ptBarOffset -= m_rsrc[sFrameLeft].m_ptStyle ;
		//
		XMLDocument&	xmlArrange =
				style.m_xmlStyle.CreateElementTagAs( "arrange" ) ;
		xmlArrange.SetAttrIntegerAs( "bar_x", ptBarOffset.x ) ;
		xmlArrange.SetAttrIntegerAs( "bar_y", ptBarOffset.y ) ;
	}
	return	sStyleID ;
}

// スクロールバースタイルを登録
//////////////////////////////////////////////////////////////////////////////
String SkinContext::AddScrollBarStyle
	( ImageContext& imgctx, String sType, String sStyleID,
		Hash<SkinItemLayerProperty>& mapProperty )
{
	if ( !mapProperty.IsEmpty( "style" ) )
	{
		sStyleID = mapProperty["style"].sProperty ;
	}
	if ( !m_styles.IsEmpty( sStyleID ) )
	{
		OutputError( "スタイルID \'" + sStyleID + "\' は重複しています" ) ;
	}
	SkinItemStyle&	style = m_styles[sStyleID] ;
	//
	style.m_xmlStyle ::= XMLDocument ;
	style.m_xmlStyle.SetTag( "style" ) ;
	style.m_xmlStyle.SetAttributeAs( "id", sStyleID ) ;
	style.m_xmlStyle.SetAttributeAs( "type", "scroll_bar" ) ;
	//
	String[][]	aPropNames =
	{
		// prop / sub-tag / attr-name / def-value
		{ "arrange", "arrange", "type", "horz" },
		{ "track", "track", "", "<margin>" },
		{ "stretchable", "stretchable", "", "<rect>" },
		{ "normal_bar", "normal", "bar", "<bar>" },
		{ "normal_column", "normal", "column", "<column>" },
		{ "normal_progress", "normal", "progress", "<progress>" },
		{ "focus_bar", "focus", "bar", "<bar>" },
		{ "focus_column", "focus", "column", "<column>" },
		{ "focus_progress", "focus", "progress", "<progress>" },
		{ "tracking_bar", "tracking", "bar", "<bar>" },
		{ "tracking_column", "tracking", "column", "<column>" },
		{ "tracking_progress", "tracking", "progress", "<progress>" },
		{ "disabled_bar", "disabled", "bar", "<bar>" },
		{ "disabled_column", "disabled", "column", "<column>" },
		{ "disabled_progress", "disabled", "progress", "<progress>" },
	} ;
	//
	// スクロール画像収集
	//
	Hash<int>			mapLayer ;
	Hash<ImageContext>	mapScroll ;
	for ( prop : aPropNames )
	{
		if ( !mapProperty.IsEmpty( prop[0] ) )
		{
			if ( (prop[3] == "<margin>")
				|| (prop[3] == "<rect>")
				|| (prop[3] == "<bar>")
				|| (prop[3] == "<column>")
				|| (prop[3] == "<progress>") )
			{
				ImageContext&	ictxScroll = mapScroll[prop[3]] ;
				mapLayer[prop[0]] = sizeof(ictxScroll) ;
				ictxScroll.merge( imgctx, mapProperty[prop[0]].iLayer, 1 ) ;
			}
			if ( prop[3] == "<bar>" )
			{
				mapScroll["<column>"].merge
					( imgctx, mapProperty[prop[0]].iLayer, 1 ) ;
				mapScroll["<progress>"].merge
					( imgctx, mapProperty[prop[0]].iLayer, 1 ) ;
			}
			if ( prop[3] == "<margin>" )
			{
				mapScroll["<column>"].merge
					( imgctx, mapProperty[prop[0]].iLayer, 1 ) ;
			}
		}
	}
	Hash<Point>	mapPoint ;
	Hash<Size>	mapSize ;
	for ( ictx : mapScroll : i )
	{
		String	sType = mapScroll.GetTagName(i) ;
		if ( ictx.cut
			( m_alignCut, m_marginCut, m_thresholdCut,
				m_flagsCut, mapPoint[sType], mapSize[sType] ) != eslErrSuccess )
		{
			mapPoint.Remove( sType ) ;
			mapSize.Remove( sType ) ;
		}
	}
	//
	// 属性設定
	//
	if ( mapPoint.IsEmpty( "<column>" ) )
	{
		OutputError
			( "\'" + sStyleID + "\' のスクロール領域が指定されていません" ) ;
	}
	for ( prop : aPropNames )
	{
		if ( !mapProperty.IsEmpty( prop[0] ) )
		{
			XMLDocument&	xmlTag =
					style.m_xmlStyle.CreateElementTagAs( prop[1] ) ;
			String	sValue = prop[3] ;
			if ( sValue == "<margin>" )
			{
				if ( !mapPoint.IsEmpty( "<rect>" ) )
				{
					xmlTag.SetAttrIntegerAs
						( "left", mapPoint["<rect>"].x - mapPoint["<column>"].x ) ;
					xmlTag.SetAttrIntegerAs
						( "top", mapPoint["<rect>"].y - mapPoint["<column>"].y ) ;
					xmlTag.SetAttrIntegerAs
						( "right", mapPoint["<column>"].x + mapSize["<column>"].w
									- (mapPoint["<rect>"].x + mapSize["<rect>"].w) ) ;
					xmlTag.SetAttrIntegerAs
						( "bottom", mapPoint["<column>"].y + mapSize["<column>"].h
									- (mapPoint["<rect>"].y + mapSize["<rect>"].h) ) ;
				}
			}
			else if ( sValue == "<rect>" )
			{
				if ( !mapPoint.IsEmpty( "<rect>" ) )
				{
					xmlTag.SetAttrIntegerAs
						( "x", mapPoint["<rect>"].x - mapPoint["<column>"].x ) ;
					xmlTag.SetAttrIntegerAs
						( "y", mapPoint["<rect>"].y - mapPoint["<column>"].y ) ;
					xmlTag.SetAttrIntegerAs( "width", mapSize["<rect>"].w ) ;
					xmlTag.SetAttrIntegerAs( "height", mapSize["<rect>"].h ) ;
				}
			}
			else if ( (sValue == "<bar>")
					|| (sValue == "<column>")
					|| (sValue == "<progress>") )
			{
				ImageContext	ictxImage ;
				ictxImage.merge( mapScroll[sValue], mapLayer[prop[0]], 1 ) ;
				//
				String	sImageID = sStyleID + "_" + prop[0] ;
				sImageID.MakeUpper() ;
				xmlTag.SetAttributeAs
					( prop[2], SaveImageResource
						( ictxImage, sImageID,
							mapPoint[sValue], mapSize[sValue] ) ) ;
			}
			else
			{
				sValue = mapProperty[prop[0]].sProperty ;
				xmlTag.SetAttributeAs( prop[2], sValue ) ;
			}
		}
	}
	return	sStyleID ;
}

// 画像アイテムを追加
//////////////////////////////////////////////////////////////////////////////
SkinItemStyle& SkinContext::AddImageItem
	( String sType, String sItemID, String sImageID,
		Hash<SkinItemLayerProperty>& mapProperty,
		const Point& ptItem, const Size& sizeItem )
{
	if ( m_rsrc.IsEmpty( sImageID ) )
	{
		OutputError
			( "\'" + sImageID + "\' は定義されていない画像IDです" ) ;
	}
	SkinResource&	rsrc = m_rsrc[sImageID] ;
	SkinItemStyle&	item = m_curPage.AddItem() ;
	item.m_ptStyle = ptItem + rsrc.m_ptHotSpot ;
	item.m_sizeStyle = rsrc.m_sizeStyle ;
	//
	item.m_xmlStyle.SetTag( "image" ) ;
	item.m_xmlStyle.SetAttributeAs( "id", sItemID ) ;
	item.m_xmlStyle.SetAttrIntegerAs( "x", item.m_ptStyle.x ) ;
	item.m_xmlStyle.SetAttrIntegerAs( "y", item.m_ptStyle.y ) ;
	item.m_xmlStyle.SetAttributeAs( "rsrc", sImageID ) ;
	//
	return	item ;
}

// 矩形アイテムを追加
//////////////////////////////////////////////////////////////////////////////
SkinItemStyle& SkinContext::AddRectangleItem
	( String sType, String sItemID,
		Hash<SkinItemLayerProperty>& mapProperty,
		const Point& ptItem, const Size& sizeItem )
{
	SkinItemStyle&	item = m_curPage.AddItem() ;
	item.m_ptStyle = ptItem ;
	item.m_sizeStyle = sizeItem ;
	//
	item.m_xmlStyle.SetTag( "rectangle" ) ;
	item.m_xmlStyle.SetAttributeAs( "id", sItemID ) ;
	item.m_xmlStyle.SetAttrIntegerAs( "x", item.m_ptStyle.x ) ;
	item.m_xmlStyle.SetAttrIntegerAs( "y", item.m_ptStyle.y ) ;
	item.m_xmlStyle.SetAttrIntegerAs( "width", item.m_sizeStyle.w ) ;
	item.m_xmlStyle.SetAttrIntegerAs( "height", item.m_sizeStyle.h ) ;
	//
	if ( !mapProperty.IsEmpty( "color" ) )
	{
		item.m_xmlStyle.SetAttributeAs
			( "color", "0" + mapProperty["color"].sProperty + "H" ) ;
	}
	return	item ;
}

// テキストアイテムを追加
//////////////////////////////////////////////////////////////////////////////
SkinItemStyle& SkinContext::AddTextItem
	( String sType, String sItemID, String sStyleID,
		Hash<SkinItemLayerProperty>& mapProperty,
		const Point& ptItem, const Size& sizeItem )
{
	SkinItemStyle&	item = m_curPage.AddItem() ;
	item.m_ptStyle = ptItem ;
	item.m_sizeStyle = sizeItem ;
	//
	item.m_xmlStyle.SetTag( "static_text" ) ;
	item.m_xmlStyle.SetAttributeAs( "id", sItemID ) ;
	item.m_xmlStyle.SetAttrIntegerAs( "x", item.m_ptStyle.x ) ;
	item.m_xmlStyle.SetAttrIntegerAs( "y", item.m_ptStyle.y ) ;
	item.m_xmlStyle.SetAttributeAs( "style", sStyleID ) ;
	item.m_xmlStyle.SetAttrIntegerAs( "width", item.m_sizeStyle.h ) ;
	item.m_xmlStyle.SetAttrIntegerAs( "height", item.m_sizeStyle.h ) ;
	//
	if ( !mapProperty.IsEmpty( "text" ) )
	{
		item.m_xmlStyle.SetAttributeAs
				( "text", mapProperty["text"].sProperty ) ;
	}
	return	item ;
}

// ボタンアイテムを追加
//////////////////////////////////////////////////////////////////////////////
SkinItemStyle& SkinContext::AddButtonItem
	( String sType, String sItemID, String sStyleID,
		Hash<SkinItemLayerProperty>& mapProperty,
		const Point& ptItem, const Size& sizeItem )
{
	if ( m_styles.IsEmpty( sStyleID ) )
	{
		OutputError
			( "\'" + sStyleID
				+ "\' は定義されていないボタン・スタイルです" ) ;
	}
	SkinItemStyle&	style = m_styles[sStyleID] ;
	SkinItemStyle&	item = m_curPage.AddItem() ;
	item.m_ptStyle = ptItem ;
	item.m_sizeStyle = style.m_sizeStyle ;
	//
	Size	sizeButton = sizeItem ;
	if ( sizeButton.w < item.m_sizeStyle.w )
	{
		sizeButton.w = item.m_sizeStyle.w ;
	}
	if ( sizeButton.h < item.m_sizeStyle.h )
	{
		sizeButton.h = item.m_sizeStyle.h ;
	}
	//
	item.m_xmlStyle.SetTag( "button" ) ;
	item.m_xmlStyle.SetAttributeAs( "id", sItemID ) ;
	item.m_xmlStyle.SetAttributeAs( "style", sStyleID ) ;
	item.m_xmlStyle.SetAttrIntegerAs( "x", item.m_ptStyle.x ) ;
	item.m_xmlStyle.SetAttrIntegerAs( "y", item.m_ptStyle.y ) ;
	item.m_xmlStyle.SetAttrIntegerAs( "width", sizeButton.w ) ;
	item.m_xmlStyle.SetAttrIntegerAs( "height", sizeButton.h ) ;
	//
	if ( mapProperty.IsEmpty( "focus_se" ) )
	{
		m_sDefFocusSE = mapProperty["focus_se"].sProperty ;
	}
	if ( mapProperty.IsEmpty( "pushed_se" ) )
	{
		m_sDefPushedSE = mapProperty["pushed_se"].sProperty ;
	}
	if ( m_sDefFocusSE != "" )
	{
		item.m_xmlStyle.SetAttributeAs( "focus_se", m_sDefFocusSE ) ;
	}
	if ( m_sDefPushedSE != "" )
	{
		item.m_xmlStyle.SetAttributeAs( "pushed_se", m_sDefPushedSE ) ;
	}
	if ( !mapProperty.IsEmpty( "text" ) )
	{
		item.m_xmlStyle.SetAttributeAs
			( "text", mapProperty["text"].sProperty ) ;
	}
	if ( !mapProperty.IsEmpty( "before_repeat" ) )
	{
		item.m_xmlStyle.
			CreateElementTagAs("command").
			CreateElementTagAs("push_repeat").
			SetAttributeAs( "before_repeat",
					mapProperty["before_repeat"].sProperty ) ;
	}
	if ( !mapProperty.IsEmpty( "repeat_interval" ) )
	{
		item.m_xmlStyle.
			CreateElementTagAs("command").
			CreateElementTagAs("push_repeat").
			SetAttributeAs( "interval",
					mapProperty["repeat_interval"].sProperty ) ;
	}
	if ( !mapProperty.IsEmpty( "scroll_target" ) )
	{
		item.m_xmlStyle.
			CreateElementTagAs("command").
			CreateElementTagAs("scroll").
			SetAttributeAs( "target",
					mapProperty["scroll_target"].sProperty ) ;
	}
	if ( !mapProperty.IsEmpty( "scroll_offset" ) )
	{
		item.m_xmlStyle.
			CreateElementTagAs("command").
			CreateElementTagAs("scroll").
			SetAttributeAs( "offset",
					mapProperty["scroll_offset"].sProperty ) ;
	}
	if ( !mapProperty.IsEmpty( "scroll_type" ) )
	{
		item.m_xmlStyle.
			CreateElementTagAs("command").
			CreateElementTagAs("scroll").
			SetAttributeAs( "type",
					mapProperty["scroll_type"].sProperty ) ;
	}
	return	item ;
}

// フレームアイテムを追加
//////////////////////////////////////////////////////////////////////////////
SkinItemStyle& SkinContext::AddFrameItem
	( String sType, String sItemID, String sStyleID,
		Hash<SkinItemLayerProperty>& mapProperty,
		const Point& ptItem, const Size& sizeItem )
{
	if ( m_styles.IsEmpty( sStyleID ) )
	{
		OutputError
			( "\'" + sStyleID
				+ "\' は定義されていないフレーム・スタイルです" ) ;
	}
	SkinItemStyle&	style = m_styles[sStyleID] ;
	SkinItemStyle&	item = m_curPage.AddItem() ;
	item.m_ptStyle = ptItem ;
	item.m_sizeStyle = sizeItem ;
	//
	item.m_xmlStyle.SetTag( "static_frame" ) ;
	item.m_xmlStyle.SetAttributeAs( "id", sItemID ) ;
	item.m_xmlStyle.SetAttributeAs( "style", sStyleID ) ;
	item.m_xmlStyle.SetAttrIntegerAs( "x", ptItem.x ) ;
	item.m_xmlStyle.SetAttrIntegerAs( "y", ptItem.y ) ;
	item.m_xmlStyle.SetAttrIntegerAs( "width", sizeItem.w ) ;
	item.m_xmlStyle.SetAttrIntegerAs( "height", sizeItem.h ) ;
	//
	return	item ;
}

// 進捗バーアイテムを追加
//////////////////////////////////////////////////////////////////////////////
SkinItemStyle& SkinContext::AddProgressBarItem
	( String sType, String sItemID, String sStyleID,
		Hash<SkinItemLayerProperty>& mapProperty,
		const Point& ptItem, const Size& sizeItem )
{
	if ( m_styles.IsEmpty( sStyleID ) )
	{
		OutputError
			( "\'" + sStyleID
				+ "\' は定義されていない進捗バー・スタイルです" ) ;
	}
	SkinItemStyle&	style = m_styles[sStyleID] ;
	SkinItemStyle&	item = m_curPage.AddItem() ;
	item.m_ptStyle = ptItem ;
	item.m_sizeStyle = sizeItem ;
	//
	item.m_xmlStyle.SetTag( "progress_bar" ) ;
	item.m_xmlStyle.SetAttributeAs( "id", sItemID ) ;
	item.m_xmlStyle.SetAttributeAs( "style", sStyleID ) ;
	item.m_xmlStyle.SetAttrIntegerAs( "x", item.m_ptStyle.x ) ;
	item.m_xmlStyle.SetAttrIntegerAs( "y", item.m_ptStyle.y ) ;
	//
	const XMLDocument&	xmlArrange = style.m_xmlStyle.GetElementTagAs( "arrange" ) ;
	if ( (xmlArrange !== null)
		&& (xmlArrange.GetAttrStringAs("type","") == "vert") )
	{
		item.m_xmlStyle.SetAttrIntegerAs( "width", sizeItem.h ) ;
	}
	else
	{
		item.m_xmlStyle.SetAttrIntegerAs( "width", sizeItem.w ) ;
	}
	//
	if ( !mapProperty.IsEmpty( "range" ) )
	{
		item.m_xmlStyle.
			CreateElementTagAs("command").
			CreateElementTagAs("bar").
			SetAttributeAs( "range", mapProperty["range"].sProperty ) ;
	}
	if ( !mapProperty.IsEmpty( "pos" ) )
	{
		item.m_xmlStyle.
			CreateElementTagAs("command").
			CreateElementTagAs("bar").
			SetAttributeAs( "pos", mapProperty["pos"].sProperty ) ;
	}
	return	item ;
}

// スクロールバーアイテムを追加
//////////////////////////////////////////////////////////////////////////////
SkinItemStyle& SkinContext::AddScrollBarItem
	( String sType, String sItemID, String sStyleID,
		Hash<SkinItemLayerProperty>& mapProperty,
		const Point& ptItem, const Size& sizeItem )
{
	if ( m_styles.IsEmpty( sStyleID ) )
	{
		OutputError
			( "\'" + sStyleID
				+ "\' は定義されていないスクロールバー・スタイルです" ) ;
	}
	SkinItemStyle&	style = m_styles[sStyleID] ;
	SkinItemStyle&	item = m_curPage.AddItem() ;
	item.m_ptStyle = ptItem ;
	item.m_sizeStyle = sizeItem ;
	//
	item.m_xmlStyle.SetTag( "scroll_bar" ) ;
	item.m_xmlStyle.SetAttributeAs( "id", sItemID ) ;
	item.m_xmlStyle.SetAttributeAs( "style", sStyleID ) ;
	item.m_xmlStyle.SetAttrIntegerAs( "x", item.m_ptStyle.x ) ;
	item.m_xmlStyle.SetAttrIntegerAs( "y", item.m_ptStyle.y ) ;
	//
	const XMLDocument&	xmlArrange = style.m_xmlStyle.GetElementTagAs( "arrange" ) ;
	if ( (xmlArrange !== null)
		&& (xmlArrange.GetAttrStringAs("type","") == "vert") )
	{
		item.m_xmlStyle.SetAttrIntegerAs( "width", sizeItem.h ) ;
	}
	else
	{
		item.m_xmlStyle.SetAttrIntegerAs( "width", sizeItem.w ) ;
	}
	//
	if ( !mapProperty.IsEmpty( "range" ) )
	{
		item.m_xmlStyle.
			CreateElementTagAs("command").
			CreateElementTagAs("bar").
			SetAttributeAs( "range", mapProperty["range"].sProperty ) ;
	}
	if ( !mapProperty.IsEmpty( "pos" ) )
	{
		item.m_xmlStyle.
			CreateElementTagAs("command").
			CreateElementTagAs("bar").
			SetAttributeAs( "pos", mapProperty["pos"].sProperty ) ;
	}
	return	item ;
}



