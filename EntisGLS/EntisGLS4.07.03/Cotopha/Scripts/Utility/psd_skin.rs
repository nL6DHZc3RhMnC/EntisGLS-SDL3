

//////////////////////////////////////////////////////////////////////////////
// 画像コンポーザー
//////////////////////////////////////////////////////////////////////////////

class	ImageComposor
{
	protected Image	m_image = null ;
	protected ImageComposition.Layer[]
				m_selLayers = new ImageComposition.Layer[] ;

	// 入力レイヤーを追加
	//////////////////////////////////////////////////////////////////////////
	public void addSelectLayer( ImageComposition.Layer layer )
	{
		if ( layer != null )
		{
			m_selLayers.add( layer ) ;
		}
	}
	public int selectLayers
			( ImageComposition imgcmp, UsageMatcher umLayerMatcher )
	{
		int	nAdded = 0 ;
		int	nCount = imgcmp.getLayerCount() ;
		for ( int i = 0; i < nCount; i ++ )
		{
			ImageComposition.Layer	layer = imgcmp.getLayerAt( i ) ;
			if ( (layer != null)
				&& (umLayerMatcher.parse( layer.getName() ) == null) )
			{
				m_selLayers.add( layer ) ;
				nAdded ++ ;
			}
		}
		return	nAdded ;
	}

	// 入力レイヤー数取得
	//////////////////////////////////////////////////////////////////////////
	public const int getSelectedLayers( void )
	{
		return	m_selLayers.length() ;
	}

	// 切り出しサイズ計算
	//////////////////////////////////////////////////////////////////////////
	public const Rect calcCutLayers( int nThreshold = 0, int nMargin = 0 )
	{
		int		xMin = 0x7FFFFFFF, xMax = 0 ;
		int		yMin = 0x7FFFFFFF, yMax = 0 ;
		boolean	flagCut = false ;
		int		nCount = m_selLayers.length() ;
		for ( int i = 0; i < nCount; i ++ )
		{
			Rect	rect = m_selLayers[i].layerSizeOf( nThreshold ) ;
			if ( rect != null )
			{
				if ( rect.x < xMin )
				{
					xMin = rect.x ;
				}
				if ( xMax < rect.x + rect.w )
				{
					xMax = rect.x + rect.w ;
				}
				if ( rect.y < yMin )
				{
					yMin = rect.y ;
				}
				if ( yMax < rect.y + rect.h )
				{
					yMax = rect.y + rect.h ;
				}
				flagCut = true ;
			}
		}
		if ( !flagCut )
		{
			return	null ;
		}
		return	new Rect( xMin, yMin, xMax - xMin, yMax - yMin ) ;
	}

	// 切り出し
	//////////////////////////////////////////////////////////////////////////
	public boolean cutLayers
			( int nThreshold = 0, int nMargin = 0, Rect rectCut = null )
	{
		int	nCount = m_selLayers.length() ;
		if ( nCount == 0 )
		{
			return	false ;
		}
		//
		// 切り出しサイズ計算
		//
		if ( rectCut == null )
		{
			rectCut = calcCutLayers( nThreshold, nMargin ) ;
			if ( rectCut == null )
			{
				return	false ;
			}
		}
		int	xCut = rectCut.x ;
		int	yCut = rectCut.y ;
		//
		// 切り出し
		//
		m_image = new Image() ;
		if ( !m_image.createImage
			( rectCut.w, rectCut.h,
				Image.formatARGB, 32,
				Image.bufferOnMemory, nCount ) )
		{
			return	false ;
		}
		for ( int i = 0; i < nCount; i ++ )
		{
			ImageComposition.Layer	layer = m_selLayers[i] ;
			if ( layer != null )
			{
				Point	ptLayer = layer.getPosition() ;
				m_image.selectFrame( i ) ;
				m_image.copyImage
					( layer, ptLayer.x - xCut, ptLayer.y - yCut ) ;
			}
		}
		m_image.selectFrame( 0 ) ;
		m_image.setImageOrigin( - xCut, - yCut ) ;
		return	true ;
	}

	// レイヤー配列（レイヤー位置を合わせてカット）
	//////////////////////////////////////////////////////////////////////////
	public Size[] collectLayers( int nThreshold = 0, int nMargin = 0 )
	{
		int	nCount = m_selLayers.length() ;
		if ( nCount == 0 )
		{
			return	null ;
		}
		//
		// 切り出しサイズ計算
		//
		Rect[]	rectLayer = new Rect[] ;
		Size[]	sizeLayer = new Size[] ;
		int	wMax = 0, hMax = 0 ;
		for ( int i = 0; i < nCount; i ++ )
		{
			Rect	rect = m_selLayers[i].layerSizeOf( nThreshold ) ;
			rectLayer[i] = rect ;
			if ( rect != null )
			{
				if ( wMax < rect.w )
				{
					wMax = rect.w ;
				}
				if ( hMax < rect.h )
				{
					hMax = rect.h ;
				}
				sizeLayer[i] = new Size( rect.w, rect.h ) ;
			}
			else
			{
				sizeLayer[i] = new Size( 0, 0 ) ;
			}
		}
		if ( (wMax == 0) || (hMax == 0) )
		{
			return	null ;
		}
		//
		// 切り出し
		//
		m_image = new Image() ;
		if ( !m_image.createImage
			( wMax + nMargin * 2, hMax + nMargin * 2,
				Image.formatARGB, 32,
				Image.bufferOnMemory, nCount ) )
		{
			return	null ;
		}
		for ( int i = 0; i < nCount; i ++ )
		{
			ImageComposition.Layer	layer = m_selLayers[i] ;
			if ( (layer != null) && (rectLayer[i] != null) )
			{
				Point	ptLayer = layer.getPosition() ;
				m_image.selectFrame( i ) ;
				m_image.copyImage
					( layer, ptLayer.x - rectLayer[i].x + nMargin,
								ptLayer.y - rectLayer[i].y + nMargin ) ;
			}
		}
		m_image.selectFrame( 0 ) ;
		return	sizeLayer ;
	}

	// 複数の画像を1枚の画像に整列
	//////////////////////////////////////////////////////////////////////////
	public Size arrangeImages
			( boolean flagHorizontal = false,
					int subCount = 1, int pixelGap = 0 )
	{
		Image	imgSrc = m_image ;
		if ( imgSrc == null )
		{
			return	null ;
		}
		int	wImages, hImages ;
		int	nImages = m_selLayers.length() ;
		if ( nImages == 0 )
		{
			return	null ;
		}
		//
		// 並べ方確定
		//
		if ( flagHorizontal )
		{
			wImages = 1 ;
			hImages = subCount ;
			if ( nImages < subCount )
			{
				hImages = nImages ;
			}
			else
			{
				wImages = (nImages + subCount - 1) / subCount ;
			}
		}
		else
		{
			hImages = 1 ;
			wImages = subCount ;
			if ( nImages < subCount )
			{
				wImages = nImages ;
			}
			else
			{
				hImages = (nImages + subCount - 1) / subCount ;
			}
		}
		//
		// 画像整列
		//
		Size	sizeImage = imgSrc.getImageSize() ;
		m_image = new Image() ;
		if ( !m_image.createImage
			( wImages * (sizeImage.w + pixelGap),
				hImages * (sizeImage.h + pixelGap),
				Image.formatARGB, 32, Image.bufferOnMemory, 1 ) )
		{
			return	null ;
		}
		for ( int i = 0; i < nImages; i ++ )
		{
			int	x, y ;
			if ( flagHorizontal )
			{
				y = i % subCount ;
				x = (i - y) / subCount ;
			}
			else
			{
				x = i % subCount ;
				y = (i - x) / subCount ;
			}
			imgSrc.selectFrame( i ) ;
			m_image.copyImage
				( imgSrc, x * (sizeImage.w + pixelGap),
							y * (sizeImage.h + pixelGap) ) ;
		}
		return	new Size( sizeImage.w + pixelGap, sizeImage.h + pixelGap ) ;
	}

	// アニメーション設定
	//////////////////////////////////////////////////////////////////////////
	public boolean setAnimationSequence( int[] seqFrames, int nDuration )
	{
		if ( m_image == null )
		{
			return	false ;
		}
		if ( seqFrames != null )
		{
			m_image.setSequenceTable( seqFrames ) ;
		}
		m_image.setAnimationDuration( nDuration ) ;
		return	true ;
	}

	// 出力画像サイズ取得
	//////////////////////////////////////////////////////////////////////////
	public Size getImageSize()
	{
		if ( m_image == null )
		{
			return	null ;
		}
		return	m_image.getImageSize() ;
	}

	// 中心座標取得
	//////////////////////////////////////////////////////////////////////////
	public Point getImageCenter()
	{
		if ( m_image == null )
		{
			return	null ;
		}
		Image.BufferInfo	bufinf = new Image.BufferInfo() ;
		if ( !m_image.getImageInfo( bufinf ) )
		{
			return	null ;
		}
		return	new Point( - bufinf.ptOrigin.x, - bufinf.ptOrigin.y ) ;
	}

	// 中心座標設定
	//////////////////////////////////////////////////////////////////////////
	public boolean setImageCenter( int x, int y )
	{
		if ( m_image == null )
		{
			return	false ;
		}
		m_image.setImageOrigin( x, y ) ;
		return	true ;
	}

	// 画像書き出し
	//////////////////////////////////////////////////////////////////////////
	public boolean saveImage
		( String path, String mime = null, int quality = 256 )
	{
		if ( m_image == null )
		{
			return	false ;
		}
		return	m_image.saveImage( path, mime, quality ) ;
	}

} ;



//////////////////////////////////////////////////////////////////////////////
// スキンコンポーザー
//////////////////////////////////////////////////////////////////////////////

class	SkinComposor
{
	// リソース情報
	//////////////////////////////////////////////////////////////////////////
	class	ResourceInfo
	{
		public String	m_sRsrcType ;		// "image", "sound"
		public String	m_sFileName ;		// リソースファイル名

		public XMLDocument format( String id )
		{
			XMLDocument	xmlDoc = new XMLDocument() ;
			xmlDoc.setTag( m_sRsrcType ) ;
			xmlDoc.setAttributeAs( "id", id ) ;
			xmlDoc.setAttributeAs( "src", m_sFileName ) ;
			return	xmlDoc ;
		}
	}

	// 画像情報
	//////////////////////////////////////////////////////////////////////////
	class	ImageInfo	extends ResourceInfo
	{
		public Point	m_ptOffset = new Point() ;		// 表示基準座標
		public Size		m_sizeImage = new Size() ;		// 画像サイズ
		public Point	m_ptImageOrg = new Point() ;	// 画像基準座標
	}

	// スタイル情報
	//////////////////////////////////////////////////////////////////////////
	class	ItemStyle	extends XMLDocument
	{
		public String	m_sID ;
		public String	m_sStyle ;
		public Point	m_ptStyle = new Point() ;
		public Size		m_szStyle = new Size() ;

		public ItemStyle()
		{
			setTag( "style" ) ;
		}
		public ItemStyle( String id, String type )
		{
			setTag( "style" ) ;
			setAttributeAs( "id", id ) ;
			setAttributeAs( "type", type ) ;
			//
			m_sID = id ;
			m_sStyle = type ;
		}
		public XMLDocument format()
		{
			return	new XMLDocument( this ) ;
		}
	}

	// テキストスタイル
	//////////////////////////////////////////////////////////////////////////
	class	TextStyle	extends ItemStyle
	{
		public TextStyle( String id )
		{
			super( id, "static_text" ) ;
		}
	}
	public static const String	textAlignLeft		= "left" ;
	public static const String	textAlignTop		= "top" ;
	public static const String	textAlignRight		= "right" ;
	public static const String	textAlignCenter		= "center" ;
	public static const String	textAlignAccordance	= "accordance" ;
	public static const String	textAlignBottom		= "bottom" ;
	public static const String	textAlignVCenter	= "vcenter" ;

	// ボタンスタイル
	//////////////////////////////////////////////////////////////////////////
	class	ButtonStyle	extends ItemStyle
	{
		public ButtonStyle( String id )
		{
			super( id, "button" ) ;
		}
	}
	public static const String	btButton		= "button" ;
	public static const String	btCheck			= "check" ;
	public static const String	btRadio			= "radio" ;
	public static const int	btnHideNormal		= 0x0001 ;
	public static const int	btnHitRect			= 0x0002 ;
	public static const int	btnHitMask			= 0x0004 ;
	public static const int	btnVisFocus			= 0x0008 ;
	public static const int	btnVisPushed		= 0x0010 ;
	public static const int	btnVisPushedFocus	= 0x0020 ;
	public static const int	btnVisActive		= 0x0040 ;
	public static const int	btnVisActivePushed	= 0x0080 ;
	public static const int	btnVisDisabled		= 0x0100 ;
	public static const int	btnVisDisablePushed	= 0x0200 ;
	public static const int	btnHiddenRect		=
				(btnHideNormal | btnHitRect
					| btnVisFocus | btnVisPushed | btnVisPushedFocus) ;
	public static const int	btnHiddenMask		=
				(btnHideNormal | btnHitMask
					| btnVisFocus | btnVisPushed | btnVisPushedFocus) ;

	// 進捗バースタイル
	//////////////////////////////////////////////////////////////////////////
	class	ProgressBarStyle	extends ItemStyle
	{
		public String	m_sType ;

		public ProgressBarStyle( String id )
		{
			super( id, "progress_bar" ) ;
		}
	}
	public static const String	pbtVertical		= "vert" ;
	public static const String	pbtHorizontal	= "horz" ;

	// スクロールバースタイル
	//////////////////////////////////////////////////////////////////////////
	class	ScrollBarStyle	extends ItemStyle
	{
		public String	m_sType ;

		public ScrollBarStyle( String id )
		{
			super( id, "scroll_bar" ) ;
		}
	}
	public static const String	sbtVertical		= "vert" ;
	public static const String	sbtHorizontal	= "horz" ;

	// 編集テキストスタイル
	//////////////////////////////////////////////////////////////////////////
	class	EditTextStyleCompatibleGLS3	extends ItemStyle
	{
		public EditTextStyleCompatibleGLS3( String id )
		{
			super( id, "edit_text" ) ;
		}
	}
	public static const String	edtSingleLine	= "single" ;
	public static const String	edtMultiLine	= "multiline" ;

	// 編集テキストスタイル（EntisGLS4）
	//////////////////////////////////////////////////////////////////////////
	class	EditTextStyle	extends ItemStyle
	{
		public EditTextStyle( String id )
		{
			super( id, "edit_text2" ) ;
		}
	}
	public static const int		edtFlagsSingleLine		= 0x00000000 ;
	public static const int		edtFlagsMultiLine		= 0x00000001 ;
	public static const int		edtFlagsLineWrap		= 0x00000002 ;
	public static const int		edtFlagAcceptReturn		= 0x00000004 ;
	public static const int		edtFlagAcceptTab		= 0x00000008 ;
	public static const int		edtFlagAutoIndent		= 0x00000010 ;
	public static const int		edtFlagMutiLineTab		= 0x00000020 ;
	public static const int		edtFlagReadOnly			= 0x00000040 ;
	public static const int		edtFlagDenyAlphabet		= 0x00000100 ;
	public static const int		edtFlagDenyNumber		= 0x00000200 ;
	public static const int		edtFlagDeny8bitChar		= 0x00000400 ;
	public static const int		edtFlagDenyMBChar		= 0x00000800 ;
	public static const int		edtFlagStyleNumber		= 0x00001000 ;
	public static const int		edtFlagStylePassword	= 0x00002000 ;
	public static const int		edtFlagColmunCaret		= 0x00000000 ;
	public static const int		edtFlagUnderbarCaret	= 0x00004000 ;
	public static const int		edtFlagDefault
									= edtFlagsMultiLine | edtFlagsLineWrap
										| edtFlagAcceptReturn | edtFlagAcceptTab ;

	// フレームスタイル
	//////////////////////////////////////////////////////////////////////////
	class	FrameStyle	extends ItemStyle
	{
		public FrameStyle( String id )
		{
			super( id, "static_frame" ) ;
		}
	}

	// ページアイテム
	//////////////////////////////////////////////////////////////////////////
	class	GenericItem		extends XMLDocument
	{
		public String	m_idItem = null ;
		public String	m_idStyle = null ;
		public Point	m_ptItem = new Point() ;	// 指定座標
		public boolean	m_flagGroup = true ;
		public boolean	m_flagTabStop = false ;
		public Rect		m_rectItem = new Rect() ;	// 表示有効矩形

		public GenericItem( String type )
		{
			setTag( type ) ;
		}
		public XMLDocument format()
		{
			if ( m_idItem != null )
			{
				setAttributeAs( "id", m_idItem ) ;
			}
			if ( m_idStyle != null )
			{
				setAttributeAs( "style", m_idStyle ) ;
			}
			setAttrIntegerAs( "x", m_ptItem.x ) ;
			setAttrIntegerAs( "y", m_ptItem.y ) ;
			setAttributeAs( "group", m_flagGroup ? "true" : "false" ) ;
			setAttributeAs( "tab_stop", m_flagTabStop ? "true" : "false" ) ;
			//
			return	new XMLDocument( this ) ;
		} ;
	} ;

	class	TextItem	extends GenericItem
	{
		public TextItem( void )
		{
			super( "static_text" ) ;
		}
	}

	class	RectangleItem	extends GenericItem
	{
		public RectangleItem( void )
		{
			super( "rectangle" ) ;
		}
	}

	class	ImageItem	extends GenericItem
	{
		public ImageItem( void )
		{
			super( "image" ) ;
		}
	}

	class	ButtonItem	extends GenericItem
	{
		public ButtonItem( void )
		{
			super( "button" ) ;
		}
	}
	public static const int	itemNoTabStop	 			= 0x0001 ;
	public static const int	itemNoGroup		 			= 0x0002 ;
	public static const int	buttonStatusNotification	= 0x0010 ;
	public static const int	buttonAcceptRightClick		= 0x0020 ;

	class	ProgressBarItem	extends GenericItem
	{
		public ProgressBarItem( void )
		{
			super( "progress_bar" ) ;
		}
	}

	class	ScrollBarItem	extends GenericItem
	{
		public ScrollBarItem( void )
		{
			super( "scroll_bar" ) ;
		}
	}

	class	EditTextItem	extends GenericItem
	{
		public EditTextItem( void )
		{
			super( "edit_text" ) ;
		}
	}
	public static const int	acceptEditReturn	= 0x0001 ;
	public static const int	acceptEditTab		= 0x0002 ;

	// ページ
	//////////////////////////////////////////////////////////////////////////
	class	FormPage	extends XMLDocument
	{
		protected String		m_id ;
		public Point			m_ptPage = new Point() ;
		public Size				m_szPage = new Size() ;
		protected GenericItem[]	m_items = new GenericItem[] ;

		// 構築関数
		public FormPage( String type, String id )
		{
			setTag( type ) ;
			m_id = id ;
		}
		// アイテム追加
		public void addItem( GenericItem item )
		{
			m_items.add( item ) ;
		}
		// ページサイズ計算
		public void calcPageSize()
		{
			int	nItems = m_items.length() ;
			int	xMin = 0x7FFFFFFF, xMax = 0 ;
			int	yMin = 0x7FFFFFFF, yMax = 0 ;
			for ( int i = 0; i < nItems; i ++ )
			{
				SkinComposor.GenericItem	item = m_items[i] ;
				if ( item != null )
				{
					if ( item.m_rectItem.x < xMin )
					{
						xMin = item.m_rectItem.x ;
					}
					if ( xMax < item.m_rectItem.x + item.m_rectItem.w )
					{
						xMax = item.m_rectItem.x + item.m_rectItem.w ;
					}
					if ( item.m_rectItem.y < yMin )
					{
						yMin = item.m_rectItem.y ;
					}
					if ( yMax < item.m_rectItem.y + item.m_rectItem.h )
					{
						yMax = item.m_rectItem.y + item.m_rectItem.h ;
					}
				}
			}
			m_ptPage.x = xMin ;
			m_ptPage.y = yMin ;
			m_szPage.w = xMax - xMin ;
			m_szPage.h = yMax - yMin ;
		}
		// アイテム座標シフト
		public void offsetPosition( int x, int y )
		{
			int	nItems = m_items.length() ;
			for ( int i = 0; i < nItems; i ++ )
			{
				SkinComposor.GenericItem	item = m_items[i] ;
				if ( item != null )
				{
					item.m_ptItem.x += x ;
					item.m_ptItem.y += y ;
				}
			}
		}
		// 出力
		public XMLDocument format()
		{
			XMLDocument	xmlPage = new XMLDocument( this ) ;
			xmlPage.setAttributeAs( "id", m_id ) ;
			xmlPage.setAttrIntegerAs( "x", m_ptPage.x ) ;
			xmlPage.setAttrIntegerAs( "y", m_ptPage.y ) ;
			xmlPage.setAttrIntegerAs( "width", m_szPage.w ) ;
			xmlPage.setAttrIntegerAs( "height", m_szPage.h ) ;
			//
			int	nItems = m_items.length() ;
			for ( int i = 0; i < nItems; i ++ )
			{
				SkinComposor.GenericItem	item = m_items[i] ;
				if ( item != null )
				{
					xmlPage.addElement( item.format() ) ;
				}
			}
			//
			return	xmlPage ;
		}
	} ;

	ImageComposition	m_imgComp = null ;
	String				m_sDstDir = "" ;
	String				m_sImageMIME = null ;
	String				m_sImageExt = ".eri" ;
	int					m_cutMargin = 0 ;
	int					m_cutThreshold = 0 ;

	HashMap				m_resources = new HashMap( ResourceInfo ) ;	// リソース配列
	HashMap				m_styles = new HashMap( ItemStyle ) ;		// スタイル配列
	FormPage[]			m_pages = new FormPage[] ;			// ページ配列
	FormPage			m_pageCur = null ;

	// 関連付け
	//////////////////////////////////////////////////////////////////////////
	public void attachImageComposition( ImageComposition imgcmp )
	{
		m_imgComp = imgcmp ;
	}

	// 出力ディレクトリ設定
	//////////////////////////////////////////////////////////////////////////
	public void setDestinationPath( String sDstDir )
	{
		m_sDstDir = sDstDir ;
	}

	// 画像切り出しパラメータ
	//////////////////////////////////////////////////////////////////////////
	public void setCutParameter( int nMargin, int nThreshold )
	{
		m_cutMargin = nMargin ;
		m_cutThreshold = nThreshold ;
	}

	// ページ開始
	//////////////////////////////////////////////////////////////////////////
	public boolean beginPage( ImageComposition imgcmp, String id )
	{
		m_imgComp = imgcmp ;
		m_pageCur = new SkinComposor.FormPage( "page", id ) ;
		m_pages.add( m_pageCur ) ;
		//
		m_pageCur.m_ptPage.x = 0 ;
		m_pageCur.m_ptPage.y = 0 ;
		m_pageCur.m_szPage = imgcmp.getCanvasSize() ;
		//
		outputMessage( "page: " + id ) ;
	}

	// ページ終了
	//////////////////////////////////////////////////////////////////////////
	public boolean endPage( int nPageMargin = 0 /* -1 is canvas size */ )
	{
		if ( m_pageCur == null )
		{
			return	false ;
		}
		if ( nPageMargin >= 0 )
		{
			m_pageCur.calcPageSize() ;
			m_pageCur.m_ptPage.x -= nPageMargin ;
			m_pageCur.m_ptPage.y -= nPageMargin ;
			m_pageCur.m_szPage.w += nPageMargin * 2 ;
			m_pageCur.m_szPage.h += nPageMargin * 2 ;
			m_pageCur.offsetPosition
				( - m_pageCur.m_ptPage.x, - m_pageCur.m_ptPage.y ) ;
		}
		m_pageCur = null ;
		return	true ;
	}

	// スキンを書式化して出力
	//////////////////////////////////////////////////////////////////////////
	public boolean saveSkin( String sDstPath = "system.inf" )
	{
		XMLDocument	xmlDoc = formatSkin() ;
		return	xmlDoc.saveDocument( m_sDstDir.offsetFilePath( sDstPath ) )  ;
	}

	public XMLDocument formatSkin()
	{
		XMLDocument	xmlDoc = new XMLDocument() ;
		xmlDoc.setTag( "skin" ) ;
		//
		// リソース
		//
		XMLDocument	xmlRsrc = new XMLDocument() ;
		xmlRsrc.setTag( "resource" ) ;
		xmlDoc.addElement( xmlRsrc ) ;
		//
		for ( id in m_resources )
		{
			xmlRsrc.addElement( m_resources[id].format( id ) ) ;
		}
		//
		// スタイル
		//
		XMLDocument	xmlStyles = new XMLDocument() ;
		xmlStyles.setTag( "declare_style" ) ;
		xmlDoc.addElement( xmlStyles ) ;
		//
		for ( id in m_styles )
		{
			xmlStyles.addElement( m_styles[id].format() ) ;
		}
		//
		// ページ
		//
		for ( int i = 0; i < m_pages.length(); i ++ )
		{
			xmlDoc.addElement( m_pages[i].format() ) ;
		}
		return	xmlDoc ;
	}

	// レイヤー検索
	//////////////////////////////////////////////////////////////////////////
	public int findLayer( String sLayerUsage, int iFirst = 0 )
	{
		if ( m_imgComp == null )
		{
			return	-1 ;
		}
		UsageMatcher	um = new UsageMatcher( sLayerUsage ) ;
		int	nCount = m_imgComp.getLayerCount() ;
		for ( int i = 0; i < nCount; i ++ )
		{
			ImageComposition.Layer	layer = m_imgComp.getLayerAt( i ) ;
			if ( (layer != null)
				&& (um.parse( layer.getName() ) == null) )
			{
				return	i ;
			}
		}
		return	-1 ;
	}

	// 画像リソース保存
	//////////////////////////////////////////////////////////////////////////
	public boolean saveAnimation
			( String sID, String sLayerUsage,
				int nDuration, boolean flagKeepPos, int[] seq = null )
	{
		if ( m_imgComp == null )
		{
			return	false ;
		}
		if ( m_resources[sID] != null )
		{
			outputError
				( "\'" + sID + "\' リソースの二重定義です" ) ;
			return	false ;
		}
		ImageComposor	imgcmp = new ImageComposor() ;
		if ( imgcmp.selectLayers
				( m_imgComp, new UsageMatcher( sLayerUsage ) ) == 0 )
		{
			outputError
				( "\'" + sLayerUsage + "\' に一致するレイヤーが見つかりません" ) ;
			return	false ;
		}
		if ( !imgcmp.cutLayers( m_cutThreshold, m_cutMargin ) )
		{
			outputError( "\'" + sLayerUsage + "\' に画像が含まれていません" ) ;
			return	false ;
		}
		SkinComposor.ImageInfo	rsi = new SkinComposor.ImageInfo() ;
		rsi.m_sRsrcType = "image" ;
		rsi.m_sFileName = sID + m_sImageExt ;
		rsi.m_ptOffset = imgcmp.getImageCenter() ;
		rsi.m_ptImageOrg = new Point( - rsi.m_ptOffset.x, - rsi.m_ptOffset.y ) ;
		rsi.m_sizeImage = imgcmp.getImageSize() ;
		//
		imgcmp.setAnimationSequence( seq, nDuration ) ;
		if ( !flagKeepPos )
		{
			imgcmp.setImageCenter( 0, 0 ) ;
			rsi.m_ptImageOrg = new Point( 0, 0 ) ;
		}
		else
		{
			rsi.m_ptOffset = new Point( 0, 0 ) ;
		}
		String	sFilePath = m_sDstDir.offsetFilePath( sID + m_sImageExt ) ;
		if ( !imgcmp.saveImage( sFilePath, m_sImageMIME ) )
		{
			outputError( "\'" + sFilePath + "\' への書き出しに失敗しました" ) ;
			return	false ;
		}
		m_resources[sID] = rsi ;
		outputMessage( "image: " + sID ) ;
		return	true ;
	}
	public boolean saveImage
		( String sID, String sLayerUsage, boolean flagKeepPos )
	{
		return	saveAnimation( sID, sLayerUsage, 1000, flagKeepPos, null ) ;
	}
	public ImageInfo registerImage( String sID, String sPath )
	{
		if ( m_resources[sID] != null )
		{
			outputError
				( "\'" + sID + "\' リソースの二重定義です" ) ;
			return	null ;
		}
		SkinComposor.ImageInfo	rsi = new SkinComposor.ImageInfo() ;
		rsi.m_sRsrcType = "image" ;
		rsi.m_sFileName = sPath ;
		m_resources[sID] = rsi ;
		return	rsi ;
	}
	public ResourceInfo registerSound( String sID, String sPath )
	{
		if ( m_resources[sID] != null )
		{
			outputError
				( "\'" + sID + "\' リソースの二重定義です" ) ;
			return	null ;
		}
		SkinComposor.ResourceInfo	rsi = new SkinComposor.ResourceInfo() ;
		rsi.m_sRsrcType = "sound" ;
		rsi.m_sFileName = sPath ;
		m_resources[sID] = rsi ;
		return	rsi ;
	}

	// テキストスタイル追加
	//////////////////////////////////////////////////////////////////////////
	public TextStyle addTextStyle
		( String id, String align, String boxAlign,
			String font, int size, int pitchLine, int pitchChar,
			boolean bold, boolean italic, int rgbText, int transText,
			Point ptShadowOffset = null,
			int rgbShadow = 0, int transShadow = 0x100,
			boolean border = false, int rgbBorder = 0, int transBorder )
	{
		if ( m_styles[id] != null )
		{
			outputError( "\'" + id + "\' スタイルの二重定義です" ) ;
			return	null ;
		}
		SkinComposor.TextStyle	style = new SkinComposor.TextStyle( id ) ;
		//
		XMLDocument	xmlArrange = new XMLDocument() ;
		xmlArrange.setTag( "arrange" ) ;
		xmlArrange.setAttributeAs( "align", align ) ;
		xmlArrange.setAttributeAs( "box_align", boxAlign ) ;
		xmlArrange.setAttrIntegerAs( "line_height", pitchLine ) ;
		xmlArrange.setAttrIntegerAs( "indent", 0 ) ;
		xmlArrange.setAttrIntegerAs( "pitch", pitchChar ) ;
		style.addElement( xmlArrange ) ;
		//
		XMLDocument	xmlFont = new XMLDocument() ;
		xmlFont.setTag( "font" ) ;
		xmlFont.setAttributeAs( "face", font ) ;
		xmlFont.setAttrIntegerAs( "size", size ) ;
		xmlFont.setAttributeAs( "bold", bold ? "true" : "false" ) ;
		xmlFont.setAttributeAs( "italic", italic ? "true" : "false" ) ;
		style.addElement( xmlFont ) ;
		//
		XMLDocument	xmlText = new XMLDocument() ;
		xmlText.setTag( "text" ) ;
		xmlText.setAttributeAs
			( "color", String.format( "%08XH", rgbText ) ) ;
		xmlText.setAttrIntegerAs( "transparency", transText ) ;
		style.addElement( xmlText ) ;
		//
		XMLDocument	xmlShadow = new XMLDocument() ;
		xmlShadow.setTag( "shadow" ) ;
		xmlShadow.setAttributeAs
			( "color", String.format( "%08XH", rgbShadow ) ) ;
		xmlShadow.setAttrIntegerAs( "transparency", transShadow ) ;
		if ( ptShadowOffset != null )
		{
			xmlShadow.setAttrIntegerAs( "x", ptShadowOffset.x ) ;
			xmlShadow.setAttrIntegerAs( "y", ptShadowOffset.y ) ;
		}
		style.addElement( xmlShadow ) ;
		//
		if ( border )
		{
			XMLDocument	xmlBorder = new XMLDocument() ;
			xmlBorder.setTag( "border" ) ;
			xmlBorder.setAttributeAs
				( "color", String.format( "%08XH", rgbBorder ) ) ;
			xmlBorder.setAttrIntegerAs( "transparency", transBorder ) ;
			style.addElement( xmlBorder ) ;
		}
		//
		m_styles[id] = style ;
		outputMessage( "text style: " + id ) ;
		return	style ;
	}

	// ボタンスタイル追加
	//////////////////////////////////////////////////////////////////////////
	public ButtonStyle addButtonStyle
		( String id, String type, int flags,
			String sLayer, int iHitMask, int iFocus, int iPushed = 0,
			int iPushedFocus = 0, int iActive = 0, int iActivePushed = 0,
			int iDisabled = 0, int iDisablePushed = 0, Rect rectCut = null )
	{
		int	iLayer = findLayer( sLayer ) ;
		if ( iLayer < 0 )
		{
			outputError( "\'" + sLayer + "\' レイヤーが見つかりません" ) ;
			return	null ;
		}
		return	addButtonStyle
			( id, type, flags, iLayer, iHitMask,
				iFocus, iPushed, iPushedFocus,
				iActive, iActivePushed, iDisabled, iDisablePushed, rectCut ) ;
	}

	public ButtonStyle addButtonStyle
		( String id, String type, int flags,
			int iLayer, int iHitMask, int iFocus, int iPushed = 0,
			int iPushedFocus = 0, int iActive = 0, int iActivePushed = 0,
			int iDisabled = 0, int iDisablePushed = 0, Rect rectCut = null )
	{
		if ( m_styles[id] != null )
		{
			outputError( "\'" + id + "\' スタイルの二重定義です" ) ;
			return	null ;
		}
		//
		// 画像切り出し
		//
		ImageComposor	imgcmp = new ImageComposor() ;
		int[]	iImage = (int[]) [ -1, -1, -1, -1, -1, -1, -1, -1, -1 ] ;
		//
		imgcmp.addSelectLayer( m_imgComp.getLayerAt( iLayer ) ) ;
		//
		if ( !(flags & SkinComposor.btnHideNormal) )
		{
			iImage[0] = 0 ;
		}
		if ( flags & SkinComposor.btnHitMask )
		{
			if ( iHitMask != 0 )
			{
				iImage[1] = imgcmp.getSelectedLayers() ;
				imgcmp.addSelectLayer
					( m_imgComp.getLayerAt( iLayer + iHitMask ) ) ;
			}
			else
			{
				iImage[1] = 0 ;
			}
		}
		int[]	iOffsetLayer =
			(int[]) [ iFocus, iPushed, iPushedFocus,
						iActive, iActivePushed, iDisabled, iDisablePushed ] ;
		for ( int i = 0; i < iOffsetLayer.length(); i ++ )
		{
			if ( iOffsetLayer[i] != 0 )
			{
				int	iCommonLayer = -1 ;
				for ( int j = 0; j < i; j ++ )
				{
					if ( iOffsetLayer[j] == iOffsetLayer[i] )
					{
						iCommonLayer = iImage[2 + j] ;
						break ;
					}
				}
				if ( iCommonLayer >= 0 )
				{
					iImage[2 + i] = iCommonLayer ;
				}
				else
				{
					iImage[2 + i] = imgcmp.getSelectedLayers() ;
					imgcmp.addSelectLayer
						( m_imgComp.getLayerAt( iLayer + iOffsetLayer[i] ) ) ;
				}
			}
			else if ( flags & (SkinComposor.btnVisFocus << i) )
			{
				iImage[2 + i] = 0 ;
			}
		}
		if ( !imgcmp.cutLayers( m_cutThreshold, m_cutMargin, rectCut ) )
		{
			outputError( "\'" + id + "\' 画像の切り出しに失敗しました" ) ;
			return	null ;
		}
		//
		// 画像保存
		//
		SkinComposor.ButtonStyle	style = new SkinComposor.ButtonStyle( id ) ;
		style.m_ptStyle = imgcmp.getImageCenter() ;
		style.m_szStyle = imgcmp.getImageSize() ;
		//
		Size	sizeParts = imgcmp.arrangeImages( false, 1, 0 ) ;
		if ( sizeParts == null )
		{
			outputError( "\'" + id + "\' 画像の構成に失敗しました" ) ;
			return	null ;
		}
		if ( !imgcmp.saveImage
			( m_sDstDir.offsetFilePath( id + m_sImageExt ), m_sImageMIME ) )
		{
			outputError( "\'" + id + "\' 画像の書き出しに失敗しました" ) ;
			return	null ;
		}
		if ( !registerImage( id, id + m_sImageExt ) )
		{
			return	null ;
		}
		//
		// ボタンスタイル設定
		//
		XMLDocument	xmlArrange = new XMLDocument() ;
		xmlArrange.setTag( "arrange" ) ;
		xmlArrange.setAttributeAs( "type", type ) ;
		style.addElement( xmlArrange ) ;
		//
		String[]	aStatus =
			[ "normal", "mask", "focus", "pushed", "pushed_focus",
				"active", "active_pushed", "disabled", "push_disabled" ] ;
		for ( int i = 0; i < aStatus.length(); i ++ )
		{
			if ( aStatus[i] == "mask" )
			{
				XMLDocument	xmlMask = new XMLDocument() ;
				xmlMask.setTag( "mask" ) ;
				if ( flags & SkinComposor.btnHitRect )
				{
					xmlMask.setAttributeAs( "rect", "true" ) ;
				}
				else if ( (iImage[i] >= 0)
						&& (flags & SkinComposor.btnHitMask) )
				{
					xmlMask.setAttributeAs( "rect", "false" ) ;
					xmlMask.setAttributeAs
						( "image", id
							+ ":RECT(0," + (iImage[i] * sizeParts.h)
							+ "," + sizeParts.w + "," + sizeParts.h + ")" ) ;
				}
				style.addElement( xmlMask ) ;
			}
			else if ( iImage[i] >= 0 )
			{
				XMLDocument	xmlImage = new XMLDocument() ;
				xmlImage.setTag( aStatus[i] ) ;
				xmlImage.setAttributeAs
					( "image", id
						+ ":RECT(0," + (iImage[i] * sizeParts.h)
						+ "," + sizeParts.w + "," + sizeParts.h + ")" ) ;
				style.addElement( xmlImage ) ;
			}
		}
		//
		m_styles[id] = style ;
		outputMessage( "button style: " + id ) ;
		return	style ;
	}

	// ボタンスタイルにテキストスタイル適用
	//////////////////////////////////////////////////////////////////////////
	public boolean setButtonTextStyle
		( String sStyleID, int xText, int yText, int wText, int hText,
			Point ptFocusDelta, Point ptPushDelta, Point ptActiveDelta,
			String sNormal, String sFocus, String sPushed, String sPushedFocus,
			String sActive, String sActivePushed, String sDisabled, String sDisabledPush )
	{
		SkinComposor.ItemStyle	style = m_styles[sStyleID] ;
		if ( !(style instanceof SkinComposor.ButtonStyle) )
		{
			outputError( "\'" + sStyleID + "\' スタイルは定義されていません" ) ;
			return	false ;
		}
		xText -= style.m_ptStyle.x ;
		yText -= style.m_ptStyle.y ;
		//
		HashMap	mapTextStyles =
			{ normal : sNormal, focus : sFocus,
				pushed : sPushed, pushed_focus : sPushedFocus,
				active : sActive, active_pushed : sActivePushed,
				disabled : sDisabled, push_disabled : sDisablePushed } ;
		HashMap	mapTextDelta =
			{ focus : ptFocusDelta, pushed : ptPushDelta,
				pushed_focus : (ptPushDelta != null) ?
										ptPushDelta : ptFocusDelta,
				active : ptActiveDelta, active_pushed : ptActiveDelta } ;
		for ( int i = 0; i < style.getElementsCount(); i ++ )
		{
			XMLDocument	xmlTag = style.getElementAt( i ) ;
			String	sTextStyleID = (String) mapTextStyles[xmlTag.getTag()] ;
			if ( sTextStyleID == null )
			{
				continue ;
			}
			ItemStyle	styleText = m_styles[sTextStyleID] ;
			if ( styleText == null )
			{
				continue ;
			}
			Point	ptDelta = (Point) mapTextDelta[xmlTag.getTag()] ;
			//
			xmlTag.removeAllElements() ;
			for ( int j = 0; j < styleText.getElementsCount(); j ++ )
			{
				XMLDocument	xmlTextElement =
								new XMLDocument( styleText.getElementAt(j) ) ;
				if ( (ptDelta != null)
					&& (xmlTextElement.getTag() == "arrange") )
				{
					int	xDelta = 0 ;
					int	yDelta = 0 ;
					if ( ptDelta != null )
					{
						xDelta = ptDelta.x ;
						yDelta = ptDelta.y ;
					}
					xmlTextElement.setAttrIntegerAs( "left", xText + xDelta ) ;
					xmlTextElement.setAttrIntegerAs( "top", yText + yDelta ) ;
					xmlTextElement.setAttrIntegerAs( "width", wText ) ;
					xmlTextElement.setAttrIntegerAs( "height", hText ) ;
				}
				xmlTag.addElement( xmlTextElement ) ;
			}
		}
		return	true ;
	}

	// ボタンスタイルの位置とサイズを取得する
	//////////////////////////////////////////////////////////////////////////
	public const Rect getButtonStyleRect( String id )
	{
		SkinComposor.ItemStyle	style = m_styles[id] ;
		if ( style == null )
		{
			return	null ;
		}
		return	new Rect( style.m_ptStyle.x, style.m_ptStyle.y,
							style.m_szStyle.w, style.m_szStyle.h ) ;
	}

	// 進捗バースタイルを登録する
	//////////////////////////////////////////////////////////////////////////
	public ProgressBarStyle addProgressBarStyle
		( String id, String type,
			String sBarLayer, String sFrameLayer, Rect rectFrameCut = null )
	{
		if ( m_styles[id] != null )
		{
			outputError( "\'" + id + "\' スタイルの二重定義です" ) ;
			return	null ;
		}
		SkinComposor.ProgressBarStyle
				style = new SkinComposor.ProgressBarStyle( id ) ;
		//
		int	iBarLayer = findLayer( sBarLayer ) ;
		int	iFrameLayer = findLayer( sFrameLayer ) ;
		if ( iBarLayer < 0 )
		{
			outputError( "\'" + sBarLayer + "\' レイヤーが見つかりません" ) ;
			return	null ;
		}
		if ( iFrameLayer < 0 )
		{
			outputError( "\'" + sFrameLayer + "\' レイヤーが見つかりません" ) ;
			return	null ;
		}
		//
		// フレーム画像切り出し
		//
		ImageComposor	imgcmp = new ImageComposor() ;
		imgcmp.addSelectLayer( m_imgComp.getLayerAt( iFrameLayer ) ) ;
		if ( !imgcmp.cutLayers( m_cutThreshold, m_cutMargin, rectFrameCut ) )
		{
			outputError( "\'" + sFrameLayer + "\' レイヤーを切り出せません" ) ;
			return	null ;
		}
		style.m_ptStyle = imgcmp.getImageCenter() ;
		style.m_szStyle = imgcmp.getImageSize() ;
		style.m_sType = type ;
		//
		if ( !imgcmp.saveImage
			( m_sDstDir.offsetFilePath
				( id + "_FRAME" + m_sImageExt ), m_sImageMIME ) )
		{
			outputError( "\'" + id + "_FRAME\' 画像の書き出しに失敗しました" ) ;
			return	null ;
		}
		if ( !registerImage( id + "_FRAME", id + "_FRAME" + m_sImageExt ) )
		{
			return	null ;
		}
		//
		// バー画像切り出し
		//
		imgcmp = new ImageComposor() ;
		imgcmp.addSelectLayer( m_imgComp.getLayerAt( iBarLayer ) ) ;
		if ( !imgcmp.cutLayers( m_cutThreshold, m_cutMargin, rectFrameCut ) )
		{
			outputError( "\'" + sBarLayer + "\' レイヤーを切り出せません" ) ;
			return	null ;
		}
		Point	ptBar = imgcmp.getImageCenter() ;
		//
		if ( !imgcmp.saveImage
			( m_sDstDir.offsetFilePath
				( id + "_BAR" + m_sImageExt ), m_sImageMIME ) )
		{
			outputError( "\'" + id + "_BAR\' 画像の書き出しに失敗しました" ) ;
			return	null ;
		}
		if ( !registerImage( id + "_BAR", id + "_BAR" + m_sImageExt ) )
		{
			return	null ;
		}
		//
		// スタイル設定
		//
		XMLDocument	xmlArrange = new XMLDocument() ;
		xmlArrange.setTag( "arrange" ) ;
		xmlArrange.setAttributeAs( "type", type ) ;
		xmlArrange.setAttrIntegerAs( "bar_x", ptBar.x - style.m_ptStyle.x ) ;
		xmlArrange.setAttrIntegerAs( "bar_y", ptBar.y - style.m_ptStyle.y ) ;
		style.addElement( xmlArrange ) ;
		//
		XMLDocument	xmlFrame = new XMLDocument() ;
		xmlFrame.setTag( "frame" ) ;
		xmlFrame.setAttributeAs( "way", id + "_FRAME" ) ;
		style.addElement( xmlFrame ) ;
		//
		XMLDocument	xmlBar = new XMLDocument() ;
		xmlBar.setTag( "frame" ) ;
		xmlBar.setAttributeAs( "way", id + "_BAR" ) ;
		style.addElement( xmlBar ) ;
		//
		m_styles[id] = style ;
		outputMessage( "progress style: " + id ) ;
		return	style ;
	}

	// スクロールバースタイルを登録する
	//////////////////////////////////////////////////////////////////////////
	public ScrollBarStyle addScrollBarStyle
		( String id, String type,
			String sBarLayer, int iBarNormal,
			int iBarFocus, int iBarTracking, int iBarDisabled,
			String sColLayer, int iColNormal,
			int iColFocus, int iColTracking, int iColDisabled,
			String sProgressLayer = null, Rect rectCutCol = null,
			int leftBarMargin = 0, int topBarMargin = 0,
			int rightBarMargin = 0, int bottomBarMargin = 0 )
	{
		if ( m_styles[id] != null )
		{
			outputError( "\'" + id + "\' スタイルの二重定義です" ) ;
			return	null ;
		}
		int	iBarLayer = findLayer( sBarLayer ) ;
		if ( iBarLayer < 0 )
		{
			outputError( "\'" + sBarLayer + "\' レイヤーが見つかりません" ) ;
			return	null ;
		}
		int	iColLayer = findLayer( sColLayer ) ;
		if ( iColLayer < 0 )
		{
			outputError( "\'" + sColLayer + "\' レイヤーが見つかりません" ) ;
			return	null ;
		}
		int	iProgLayer = -1 ;
		if ( sProgressLayer != null )
		{
			iProgLayer = findLayer( sProgressLayer ) ;
			if ( iProgLayer < 0 )
			{
				outputError
					( "\'" + sProgressLayer + "\' レイヤーが見つかりません" ) ;
				return	null ;
			}
		}
		boolean	flagHorz = (type == SkinComposor.sbtHorizontal) ;
		//
		// 切り出しサイズ計算
		//
		ImageComposor	icBar = new ImageComposor() ;
		icBar.addSelectLayer
			( m_imgComp.getLayerAt( iBarLayer + iBarNormal ) ) ;
		icBar.addSelectLayer
			( m_imgComp.getLayerAt( iBarLayer + iBarFocus ) ) ;
		icBar.addSelectLayer
			( m_imgComp.getLayerAt( iBarLayer + iBarTracking ) ) ;
		icBar.addSelectLayer
			( m_imgComp.getLayerAt( iBarLayer + iBarDisabled ) ) ;
		//
		ImageComposor	icCol = new ImageComposor() ;
		icCol.addSelectLayer
			( m_imgComp.getLayerAt( iColLayer + iColNormal ) ) ;
		icCol.addSelectLayer
			( m_imgComp.getLayerAt( iColLayer + iColFocus ) ) ;
		icCol.addSelectLayer
			( m_imgComp.getLayerAt( iColLayer + iColTracking ) ) ;
		icCol.addSelectLayer
			( m_imgComp.getLayerAt( iColLayer + iColDisabled ) ) ;
		//
		Rect	rectBar = icBar.calcCutLayers( m_cutThreshold, m_cutMargin ) ;
		if ( rectBar == null )
		{
			outputError
				( "\'" + sBarLayer + "\' 切り出し領域を確定できません" ) ;
			return	null ;
		}
		if ( rectCutCol == null )
		{
			Rect	rectCol = icCol.calcCutLayers( m_cutThreshold, m_cutMargin ) ;
			if ( rectCol == null )
			{
				outputError
					( "\'" + sColLayer + "\' 切り出し領域を確定できません" ) ;
				return	null ;
			}
			if ( rectBar.x < rectCol.x )
			{
				rectCol.w += rectCol.x - rectBar.x ;
				rectCol.x = rectBar.x ;
			}
			if ( rectBar.y < rectCol.y )
			{
				rectCol.h += rectCol.y - rectBar.y ;
				rectCol.y = rectBar.y ;
			}
			if ( rectBar.x + rectBar.w > rectCol.x + rectCol.w )
			{
				rectCol.w = rectBar.x + rectBar.w - rectCol.x ;
			}
			if ( rectBar.y + rectBar.h > rectCol.y + rectCol.h )
			{
				rectCol.h = rectBar.y + rectBar.h - rectCol.y ;
			}
			rectCutCol = rectCol ;
		}
		//
		// バー画像切り出し
		//
		if ( !icBar.cutLayers( m_cutThreshold, m_cutMargin, rectBar ) )
		{
			outputError( "\'" + sBarLayer + "\' レイヤーを切り出せません" ) ;
			return	null ;
		}
		Size	sizeBar = icBar.arrangeImages( !flagHorz, 1, 0 ) ;
		if ( (sizeBar == null)
			|| !icBar.saveImage
				( m_sDstDir.offsetFilePath
					( id + "_BAR" + m_sImageExt ), m_sImageMIME ) )
		{
			outputError( "\'" + id + "_BAR\' 画像の書き出しに失敗しました" ) ;
			return	null ;
		}
		if ( !registerImage( id + "_BAR", id + "_BAR" + m_sImageExt ) )
		{
			return	null ;
		}
		//
		// フレーム画像切り出し
		//
		if ( !icCol.cutLayers( m_cutThreshold, m_cutMargin, rectCutCol ) )
		{
			outputError( "\'" + sBarLayer + "\' レイヤーを切り出せません" ) ;
			return	null ;
		}
		Size	sizeCol = icCol.arrangeImages( !flagHorz, 1, 0 ) ;
		if ( (sizeCol == null)
			|| !icCol.saveImage
				( m_sDstDir.offsetFilePath
					( id + "_COL" + m_sImageExt ), m_sImageMIME ) )
		{
			outputError( "\'" + id + "_COL\' 画像の書き出しに失敗しました" ) ;
			return	null ;
		}
		if ( !registerImage( id + "_COL", id + "_COL" + m_sImageExt ) )
		{
			return	null ;
		}
		//
		// 進捗バー画像切り出し
		//
		if ( iProgLayer >= 0 )
		{
			ImageComposor	icProg = new ImageComposor() ;
			icProg.addSelectLayer( m_imgComp.getLayerAt( iProgLayer ) ) ;
			if ( !icProg.cutLayers( m_cutThreshold, m_cutMargin, rectCutCol ) )
			{
				outputError( "\'" + sProgressLayer + "\' レイヤーを切り出せません" ) ;
				return	null ;
			}
			if ( !icProg.saveImage
					( m_sDstDir.offsetFilePath
						( id + "_PRGBAR" + m_sImageExt ), m_sImageMIME ) )
			{
				outputError( "\'" + id + "_PRGBAR\' 画像の書き出しに失敗しました" ) ;
				return	null ;
			}
			if ( !registerImage( id + "_PRGBAR", id + "_PRGBAR" + m_sImageExt ) )
			{
				return	null ;
			}
		}
		//
		// スタイル設定
		//
		SkinComposor.ScrollBarStyle
					style = new SkinComposor.ScrollBarStyle( id ) ;
		style.m_ptStyle.x = rectCutCol.x ;
		style.m_ptStyle.y = rectCutCol.y ;
		style.m_szStyle.w = rectCutCol.w ;
		style.m_szStyle.h = rectCutCol.h ;
		style.m_sType = type ;
		//
		XMLDocument	xmlArrange = new XMLDocument() ;
		xmlArrange.setTag( "arrange" ) ;
		xmlArrange.setAttributeAs( "type", type ) ;
		if ( flagHorz )
		{
			xmlArrange.setAttrIntegerAs
				( "bar_offset", sizeBar.w - sizeCol.w ) ;
		}
		else
		{
			xmlArrange.setAttrIntegerAs
				( "bar_offset", sizeBar.h - sizeCol.h ) ;
		}
		style.addElement( xmlArrange ) ;
		//
		XMLDocument	xmlTrack = new XMLDocument() ;
		xmlTrack.setTag( "track" ) ;
		xmlTrack.setAttrIntegerAs( "left", leftBarMargin ) ;
		xmlTrack.setAttrIntegerAs( "top", topBarMargin ) ;
		xmlTrack.setAttrIntegerAs( "right", rightBarMargin ) ;
		xmlTrack.setAttrIntegerAs( "bottom", bottomBarMargin ) ;
		style.addElement( xmlTrack ) ;
		//
		Point	ptBarStep = new Point( sizeBar.w, sizeBar.h ) ;
		Point	ptColStep = new Point( sizeCol.w, sizeCol.h ) ;
		if ( flagHorz )
		{
			ptBarStep.x = 0 ;
			ptColStep.x = 0 ;
		}
		else
		{
			ptBarStep.y = 0 ;
			ptColStep.y = 0 ;
		}
		//
		String[]	aStatus = [ "normal", "focus", "tracking", "disabled" ] ;
		for ( int i = 0; i < aStatus.length(); i ++ )
		{
			XMLDocument	xmlImage = new XMLDocument() ;
			xmlImage.setTag( aStatus[i] ) ;
			xmlImage.setAttributeAs
				( "bar", id + "_BAR:RECT("
						+ (ptBarStep.x * i) + "," + (ptBarStep.y * i)
						+ "," + sizeBar.w + "," + sizeBar.h + ")" ) ;
			xmlImage.setAttributeAs
				( "column", id + "_COL:RECT("
						+ (ptColStep.x * i) + "," + (ptColStep.y * i)
						+ "," + sizeCol.w + "," + sizeCol.h + ")" ) ;
			if ( iProgLayer >= 0 )
			{
				xmlImage.setAttributeAs( "progress", id + "_PRGBAR" ) ;
			}
			style.addElement( xmlImage ) ;
		}
		//
		m_styles[id] = style ;
		outputMessage( "scroll style: " + id ) ;
		return	style ;
	}

	// 編集テキストスタイルを登録する
	//////////////////////////////////////////////////////////////////////////
	public EditTextStyle addEditTextStyle
		( String id, String type,
			int nFontSize, String sFontFace,
			boolean flagBold, boolean flagItalic,
			int nLineHeight,
			int rgbTextColor, int rgbSelColor,
			int nCaretWidth, int nCaretHeight,
			int nCaretInterval, int rgbaCaretColor,
			int nIMEFontSize = 0, String sIMEFontFace = null )
	{
		if ( m_styles[id] != null )
		{
			outputError( "\'" + id + "\' スタイルの二重定義です" ) ;
			return	null ;
		}
		SkinComposor.EditTextStyleCompatibleGLS3
			style = new SkinComposor.EditTextStyleCompatibleGLS3( id ) ;
		//
		XMLDocument	xmlArrange = new XMLDocument() ;
		xmlArrange.setTag( "arrange" ) ;
		xmlArrange.setAttributeAs( "type", type ) ;
		style.addElement( xmlArrange ) ;
		//
		XMLDocument	xmlFont = new XMLDocument() ;
		xmlFont.setTag( "font" ) ;
		xmlFont.setAttrIntegerAs( "size", nFontSize ) ;
		xmlFont.setAttributeAs( "face", sFontFace ) ;
		xmlFont.setAttributeAs( "bold", flagBold ? "true" : "false" ) ;
		xmlFont.setAttributeAs( "italic", flagItalic ? "true" : "false" ) ;
		style.addElement( xmlFont ) ;
		//
		XMLDocument	xmlEdit = new XMLDocument() ;
		xmlEdit.setTag( "edit" ) ;
		xmlEdit.setAttrIntegerAs( "top", 0 ) ;
		xmlEdit.setAttrIntegerAs( "bottom", nLineHeight - 1 ) ;
		xmlEdit.setAttributeAs
			( "color", String.format( "00%06XH", rgbTextColor ) ) ;
		xmlEdit.setAttributeAs
			( "sel_color", String.format( "00%06XH", rgbSelColor ) ) ;
		style.addElement( xmlEdit ) ;
		//
		XMLDocument	xmlCaret = new XMLDocument() ;
		xmlCaret.setTag( "caret" ) ;
		xmlCaret.setAttrIntegerAs( "width", nCaretWidth ) ;
		xmlCaret.setAttrIntegerAs( "height", nCaretHeight ) ;
		xmlCaret.setAttrIntegerAs( "interval", nCaretInterval ) ;
		xmlCaret.setAttributeAs
			( "color", String.format( "0%08XH", rgbaCaretColor ) ) ;
		style.addElement( xmlCaret ) ;
		//
		if ( nIMEFontSize > 0 )
		{
			if ( sIMEFontFace == null )
			{
				sIMEFontFace = sFontFace ;
			}
			XMLDocument	xmlImeFont = new XMLDocument() ;
			xmlImeFont.setTag( "ime_font" ) ;
			xmlImeFont.setAttrIntegerAs( "size", nIMEFontSize ) ;
			xmlImeFont.setAttributeAs( "face", sIMEFontFace ) ;
			style.addElement( xmlImeFont ) ;
		}
		//
		m_styles[id] = style ;
		outputMessage( "edit style: " + id ) ;
		return	style ;
	}

	// 編集テキストスタイルを登録する（EntisGLS4）
	//////////////////////////////////////////////////////////////////////////
	public EditTextStyle addEditTextStyle2
		( String id, String sAlign,
			int nFontSize, String sFontFace,
			boolean flagBold, boolean flagItalic,
			int nLineHeight, int nFontPitch,
			int rgbTextColor, int rgbSelColor,
			boolean fBordering, int rgbaBorderColor,
			int nCaretWidth, int nCaretInterval,
			int rgbaCaretColor, int rgbaSelBack,
			int nEditFlags = edtFlagDefault,
			int nIMEFontSize = 0, String sIMEFontFace = null )
	{
		if ( m_styles[id] != null )
		{
			outputError( "\'" + id + "\' スタイルの二重定義です" ) ;
			return	null ;
		}
		SkinComposor.EditTextStyle	style = new SkinComposor.EditTextStyle( id ) ;
		//
		XMLDocument	xmlArrange = new XMLDocument() ;
		xmlArrange.setTag( "arrange" ) ;
		xmlArrange.setAttributeAs( "align", sAlign ) ;
		xmlArrange.setAttrIntegerAs( "line_height", nLineHeight ) ;
		xmlArrange.setAttrIntegerAs( "indent", 0 ) ;
		xmlArrange.setAttrIntegerAs( "pitch", nFontPitch ) ;
		style.addElement( xmlArrange ) ;
		//
		XMLDocument	xmlFont = new XMLDocument() ;
		xmlFont.setTag( "font" ) ;
		xmlFont.setAttrIntegerAs( "size", nFontSize ) ;
		xmlFont.setAttributeAs( "face", sFontFace ) ;
		xmlFont.setAttributeAs( "bold", flagBold ? "true" : "false" ) ;
		xmlFont.setAttributeAs( "italic", flagItalic ? "true" : "false" ) ;
		style.addElement( xmlFont ) ;
		//
		XMLDocument	xmlText = new XMLDocument() ;
		xmlText.setTag( "text" ) ;
		xmlText.setAttributeAs
			( "color", String.format( "00%06XH", rgbTextColor ) ) ;
		style.addElement( xmlText ) ;
		//
		XMLDocument	xmlSelText = new XMLDocument() ;
		xmlSelText.setTag( "sel_text" ) ;
		xmlSelText.setAttributeAs
			( "color", String.format( "00%06XH", rgbSelColor ) ) ;
		style.addElement( xmlSelText ) ;
		//
		if ( fBordering )
		{
			XMLDocument	xmlBorder = new XMLDocument() ;
			xmlBorder.setTag( "border" ) ;
			xmlBorder.setAttributeAs
				( "color", String.format( "00%06XH", rgbaBorderColor ) ) ;
			style.addElement( xmlBorder ) ;
		}
		//
		HashMap	mapEditFlags =
		{
			single_line:	SkinComposor.edtFlagsSingleLine,
			multi_line:		SkinComposor.edtFlagsMultiLine,
			line_wrap:		SkinComposor.edtFlagsLineWrap,
			accept_return:	SkinComposor.edtFlagAcceptReturn,
			accept_tab:		SkinComposor.edtFlagAcceptTab,
			auto_indent:	SkinComposor.edtFlagAutoIndent,
			multi_line_tab:	SkinComposor.edtFlagMutiLineTab,
			read_only:		SkinComposor.edtFlagReadOnly,
			deny_alphabet:	SkinComposor.edtFlagDenyAlphabet,
			deny_number:	SkinComposor.edtFlagDenyNumber,
			deny_8bit_char:	SkinComposor.edtFlagDeny8bitChar,
			deny_mb_char:	SkinComposor.edtFlagDenyMBChar,
			style_number:	SkinComposor.edtFlagStyleNumber,
			style_password:	SkinComposor.edtFlagStylePassword,
			underbar_caret:	SkinComposor.edtFlagUnderbarCaret,
		} ;
		//
		XMLDocument	xmlEdit = new XMLDocument() ;
		xmlEdit.setTag( "edit" ) ;
		xmlEdit.setAttrComplexIntegerAs( "flags", mapEditFlags, nEditFlags ) ;
		xmlEdit.setAttrIntegerAs( "caret_width", nCaretWidth ) ;
		xmlEdit.setAttrIntegerAs( "caret_interval", nCaretInterval ) ;
		xmlEdit.setAttributeAs
			( "caret_color", String.format( "0%08XH", rgbaCaretColor ) ) ;
		xmlEdit.setAttributeAs
			( "sel_back_color", String.format( "0%08XH", rgbaSelBack ) ) ;
		style.addElement( xmlEdit ) ;
		//
		if ( nIMEFontSize > 0 )
		{
			if ( sIMEFontFace == null )
			{
				sIMEFontFace = sFontFace ;
			}
			XMLDocument	xmlImeFont = new XMLDocument() ;
			xmlImeFont.setTag( "ime_font" ) ;
			xmlImeFont.setAttrIntegerAs( "size", nIMEFontSize ) ;
			xmlImeFont.setAttributeAs( "face", sIMEFontFace ) ;
			style.addElement( xmlImeFont ) ;
		}
		//
		m_styles[id] = style ;
		outputMessage( "edit2 style: " + id ) ;
		return	style ;
	}

	// フレームスタイル追加
	//////////////////////////////////////////////////////////////////////////
	public FrameStyle addFrameStyle
		( String id, String sBaseLayer,
			int iUpperLeft, int iUpper, int iUpperRight,
			int iLeft, int iCenter, int iRight,
			int iUnderLeft, int iUnder, int iUnderRight )
	{
		if ( m_styles[id] != null )
		{
			outputError( "\'" + id + "\' スタイルの二重定義です" ) ;
			return	null ;
		}
		int	iBaseLayer = findLayer( sBaseLayer ) ;
		if ( iBaseLayer < 0 )
		{
			outputError( "\'" + sBaseLayer + "\' レイヤーが見つかりません" ) ;
			return	null ;
		}
		ImageComposor	imgcmp = new ImageComposor() ;
		imgcmp.addSelectLayer
			( m_imgComp.getLayerAt( iBaseLayer + iUpperLeft ) ) ;
		imgcmp.addSelectLayer
			( m_imgComp.getLayerAt( iBaseLayer + iUpper ) ) ;
		imgcmp.addSelectLayer
			( m_imgComp.getLayerAt( iBaseLayer + iUpperRight ) ) ;
		imgcmp.addSelectLayer
			( m_imgComp.getLayerAt( iBaseLayer + iLeft ) ) ;
		imgcmp.addSelectLayer
			( m_imgComp.getLayerAt( iBaseLayer + iCenter ) ) ;
		imgcmp.addSelectLayer
			( m_imgComp.getLayerAt( iBaseLayer + iRight ) ) ;
		imgcmp.addSelectLayer
			( m_imgComp.getLayerAt( iBaseLayer + iUnderLeft ) ) ;
		imgcmp.addSelectLayer
			( m_imgComp.getLayerAt( iBaseLayer + iUnder ) ) ;
		imgcmp.addSelectLayer
			( m_imgComp.getLayerAt( iBaseLayer + iUnderRight ) ) ;
		//
		Size[]	sizeLayers =
			imgcmp.collectLayers( m_cutThreshold, m_cutMargin ) ;
		Size	sizePart = imgcmp.arrangeImages( false, 3, 0 ) ;
		//
		if ( !imgcmp.saveImage
				( m_sDstDir.offsetFilePath
					( id + m_sImageExt ), m_sImageMIME ) )
		{
			outputError( "\'" + id + "\' 画像の書き出しに失敗しました" ) ;
			return	null ;
		}
		if ( !registerImage( id, id + m_sImageExt ) )
		{
			return	null ;
		}
		//
		SkinComposor.FrameStyle	style = new SkinComposor.FrameStyle( id ) ;
		//
		XMLDocument	xmlImage = new XMLDocument() ;
		xmlImage.setTag( "image" ) ;
		style.addElement( xmlImage ) ;
		//
		String[]	aParts =
			[ "upper_left", "upper", "upper_right",
				"left", "pane", "right",
				"under_left", "under", "under_right" ] ;
		for ( int i = 0; i < aParts.length(); i ++ )
		{
			int	x = i % 3 ;
			int	y = (i - x) / 3 ;
			xmlImage.setAttributeAs
				( aParts[i], id + ":RECT("
					+ (x * sizePart.w) + "," + (y * sizePart.h) + ","
					+ sizeLayers[i].w + "," + sizeLayers[i].h + ")" ) ;
		}
		//
		m_styles[id] = style ;
		outputMessage( "frame style: " + id ) ;
		return	style ;
	}

	// 静テキストアイテム追加
	//////////////////////////////////////////////////////////////////////////
	public TextItem addTextItem
		( String idItem, String idStyle,
			int x, int y, int w, int h, String sText = "" )
	{
		SkinComposor.TextItem	item = new SkinComposor.TextItem() ;
		m_pageCur.addItem( item ) ;
		//
		item.m_idItem = idItem ;
		item.m_idStyle = idStyle ;
		item.m_ptItem.x = x ;
		item.m_ptItem.y = y ;
		item.m_rectItem.x = x ;
		item.m_rectItem.y = y ;
		item.m_rectItem.w = w ;
		item.m_rectItem.h = h ;
		//
		item.setAttrIntegerAs( "width", w ) ;
		item.setAttrIntegerAs( "height", h ) ;
		item.setAttributeAs( "text", sText ) ;
		//
		outputMessage( "text item: " + idItem ) ;
		return	item ;
	}

	public TextItem addTextItem
		( String idItem, String idStyle,
			String sGuideLayer, String sText = "" )
	{
		int	iLayer = findLayer( sGuideLayer ) ;
		if ( iLayer < 0 )
		{
			outputError( "\'" + sGuideLayer + "\' レイヤーが見つかりません" ) ;
			return	null ;
		}
		ImageComposor	imgcmp = new ImageComposor() ;
		imgcmp.addSelectLayer( m_imgComp.getLayerAt( iLayer ) ) ;
		//
		Rect	rect = imgcmp.calcCutLayers( m_cutThreshold, 0 ) ;
		if ( rect == null )
		{
			outputError( "\'" + sGuideLayer
							+ "\' レイヤーに有効な領域がありません" ) ;
			return	null ;
		}
		return	addTextItem
			( idItem, idStyle, rect.x, rect.y, rect.w, rect.h, sText ) ;
	}

	// 矩形アイテム追加
	//////////////////////////////////////////////////////////////////////////
	public RectangleItem addRectangleItem
		( String idItem, int x, int y, int w, int h, int argbColor = 0 )
	{
		SkinComposor.RectangleItem	item = new SkinComposor.RectangleItem() ;
		m_pageCur.addItem( item ) ;
		//
		item.m_idItem = idItem ;
		item.m_ptItem.x = x ;
		item.m_ptItem.y = y ;
		item.m_rectItem.x = x ;
		item.m_rectItem.y = y ;
		item.m_rectItem.w = w ;
		item.m_rectItem.h = h ;
		//
		item.setAttrIntegerAs( "width", w ) ;
		item.setAttrIntegerAs( "height", h ) ;
		item.setAttributeAs( "color", String.format( "0%08XH", argbColor ) ) ;
		//
		outputMessage( "rectangle item: " + idItem ) ;
		return	item ;
	}

	public RectangleItem addRectangleItem
		( String idItem, String sLayerUsage, int argbColor = 0 )
	{
		int	iLayer = findLayer( sLayerUsage ) ;
		if ( iLayer < 0 )
		{
			outputError( "\'" + sLayerUsage + "\' レイヤーが見つかりません" ) ;
			return	null ;
		}
		ImageComposor	imgcmp = new ImageComposor() ;
		imgcmp.addSelectLayer( m_imgComp.getLayerAt( iLayer ) ) ;
		//
		Rect	rect = imgcmp.calcCutLayers( m_cutThreshold, 0 ) ;
		if ( rect == null )
		{
			outputError( "\'" + sLayerUsage
							+ "\' レイヤーに有効な領域がありません" ) ;
			return	null ;
		}
		return	addRectangleItem
					( idItem, rect.x, rect.y, rect.w, rect.h, argbColor ) ;
	}

	// 画像アイテム追加
	//////////////////////////////////////////////////////////////////////////
	public ImageItem addImageItem
		( String idItem, String idImage,
			boolean fUnclickable = false,
			int xOffset = 0, int yOffset = 0, Point ptItem = null )
	{
		String		idPureImage = idImage ;
		Size		sizeImage = null ;
		Point		ptImageOrg = null ;
		//
		String[]	aParam = new String[] ;
		if ( (new UsageMatcher
				("(%s) [\\: &!RECT& \\( (%n) ," +
				" (%n) , (%n), (%n) \\)]\\")).parse( idImage, aParam ) == null )
		{
			idPureImage = aParam[0] ;
			if ( aParam[3] != "" )
			{
				sizeImage = new Size() ;
				sizeImage.w = (int) aParam[3] ;
				sizeImage.h = (int) aParam[4] ;
			}
		}
		if ( sizeImage == null )
		{
			SkinComposor.ResourceInfo	rsi = m_resources[idPureImage] ;
			if ( (rsi == null)
				|| !(rsi instanceof SkinComposor.ImageInfo) )
			{
				outputError( idPureImage + " は未定義の識別子です" ) ;
				return	null ;
			}
			SkinComposor.ImageInfo	imginf = (SkinComposor.ImageInfo) rsi ;
			sizeImage = imginf.m_sizeImage ;
			ptImageOrg = imginf.m_ptImageOrg ;
			if ( ptItem == null )
			{
				xOffset += imginf.m_ptOffset.x ;
				yOffset += imginf.m_ptOffset.y ;
			}
		}
		if ( ptItem != null )
		{
			xOffset += ptItem.x ;
			yOffset += ptItem.y ;
		}
		//
		SkinComposor.ImageItem	item = new SkinComposor.ImageItem() ;
		m_pageCur.addItem( item ) ;
		//
		item.m_idItem = idItem ;
		item.m_ptItem.x = xOffset ;
		item.m_ptItem.y = yOffset ;
		item.m_rectItem.x = xOffset ;
		item.m_rectItem.y = yOffset ;
		item.m_rectItem.w = sizeImage.w ;
		item.m_rectItem.h = sizeImage.h ;
		if ( ptImageOrg != null )
		{
			item.m_rectItem.x -= ptImageOrg.x ;
			item.m_rectItem.y -= ptImageOrg.y ;
		}
		item.setAttributeAs( "rsrc", idImage ) ;
		//
		if ( fUnclickable )
		{
			XMLDocument	xmlCmd = item.createElementTagAs( "command" ) ;
			XMLDocument	xmlFlag = new XMLDocument() ;
			xmlFlag.setTag( "basic_flag" ) ;
			xmlFlag.setAttributeAs( "hit_transparency", "true" ) ;
			xmlCmd.addElement( xmlFlag ) ;
		}
		outputMessage( "image item: " + idItem ) ;
		return	item ;
	}

	// ボタンアイテム追加
	//////////////////////////////////////////////////////////////////////////
	public ButtonItem addButtonItem
		( String idItem, String idStyle, int nFlags = 0,
			int xOffset = 0, int yOffset = 0, Point ptAbsolute = null,
			String sPushedSE = null, String sFocusSE = null )
	{
		SkinComposor.ItemStyle	style = m_styles[idStyle] ;
		if ( (style == null)
			|| !(style instanceof SkinComposor.ButtonStyle) )
		{
			outputError( idStyle + " は未定義のスタイルです" ) ;
			return	null ;
		}
		if ( ptAbsolute != null )
		{
			xOffset += ptAbsolute.x ;
			yOffset += ptAbsolute.y ;
		}
		else
		{
			xOffset += style.m_ptStyle.x ;
			yOffset += style.m_ptStyle.y ;
		}
		//
		SkinComposor.ButtonItem	item = new SkinComposor.ButtonItem() ;
		m_pageCur.addItem( item ) ;
		//
		item.m_idItem = idItem ;
		item.m_idStyle = idStyle ;
		item.m_flagTabStop = ((nFlags & SkinComposor.itemNoTabStop) == 0) ;
		item.m_flagGroup = ((nFlags & SkinComposor.itemNoGroup) == 0) ;
		item.m_ptItem.x = xOffset ;
		item.m_ptItem.y = yOffset ;
		item.m_rectItem.x = xOffset ;
		item.m_rectItem.y = yOffset ;
		item.m_rectItem.w = style.m_szStyle.w ;
		item.m_rectItem.h = style.m_szStyle.h ;
		//
		item.setAttrIntegerAs( "width", style.m_szStyle.w ) ;
		item.setAttrIntegerAs( "height", style.m_szStyle.h ) ;
		//
		if ( sPushedSE != null )
		{
			item.setAttributeAs( "pushed_se", sPushedSE ) ;
		}
		if ( sFocusSE != null )
		{
			item.setAttributeAs( "focus_se", sFocusSE ) ;
		}
		if ( nFlags & SkinComposor.buttonStatusNotification )
		{
			XMLDocument	xmlCmd = item.createElementTagAs( "command" ) ;
			XMLDocument	xmlParam = new XMLDocument() ;
			xmlParam.setTag( "status_notification" ) ;
			xmlParam.setAttributeAs( "parameter", "1" ) ;
			xmlCmd.addElement( xmlParam ) ;
		}
		if ( nFlags & SkinComposor.buttonAcceptRightClick )
		{
			XMLDocument	xmlCmd = item.createElementTagAs( "command" ) ;
			XMLDocument	xmlParam = new XMLDocument() ;
			xmlParam.setTag( "right_click" ) ;
			xmlParam.setAttributeAs( "parameter", "2" ) ;
			xmlCmd.addElement( xmlParam ) ;
		}
		outputMessage( "button item: " + idItem ) ;
		return	item ;
	}

	// 進捗バーアイテム追加
	//////////////////////////////////////////////////////////////////////////
	public ProgressBarItem addProgressBarItem
		( String idItem, String idStyle, int nRange, int nFlags = 0,
			int xOffset = 0, int yOffset = 0, Point ptAbsolute = null )
	{
		SkinComposor.ItemStyle	style = m_styles[idStyle] ;
		if ( (style == null)
			|| !(style instanceof SkinComposor.ProgressBarStyle) )
		{
			outputError( idStyle + " は未定義のスタイルです" ) ;
			return	null ;
		}
		if ( ptAbsolute != null )
		{
			xOffset += ptAbsolute.x ;
			yOffset += ptAbsolute.y ;
		}
		else
		{
			xOffset += style.m_ptStyle.x ;
			yOffset += style.m_ptStyle.y ;
		}
		SkinComposor.ProgressBarStyle
			pbs = (SkinComposor.ProgressBarStyle) style ;
		//
		SkinComposor.ProgressBarItem
				item = new SkinComposor.ProgressBarItem() ;
		m_pageCur.addItem( item ) ;
		//
		item.m_idItem = idItem ;
		item.m_idStyle = idStyle ;
		item.m_ptItem.x = xOffset ;
		item.m_ptItem.y = yOffset ;
		item.m_rectItem.x = xOffset ;
		item.m_rectItem.y = yOffset ;
		item.m_rectItem.w = style.m_szStyle.w ;
		item.m_rectItem.h = style.m_szStyle.h ;
		//
		if ( pbs.m_sType == SkinComposor.pbtVertical )
		{
			item.setAttrIntegerAs( "width", style.m_szStyle.h ) ;
		}
		else
		{
			item.setAttrIntegerAs( "width", style.m_szStyle.w ) ;
		}
		//
		XMLDocument	xmlCmd = item.createElementTagAs( "command" ) ;
		XMLDocument	xmlParam = new XMLDocument() ;
		xmlParam.setTag( "bar" ) ;
		xmlParam.setAttrIntegerAs( "range", nRange ) ;
		xmlCmd.addElement( xmlParam ) ;
		//
		outputMessage( "progress item: " + idItem ) ;
		return	item ;
	}

	// スクロールバーアイテム追加
	//////////////////////////////////////////////////////////////////////////
	public ScrollBarItem addScrollBarItem
		( String idItem, String idStyle, int nRange, int nFlags = 0,
			int xOffset = 0, int yOffset = 0, Point ptAbsolute = null )
	{
		SkinComposor.ItemStyle	style = m_styles[idStyle] ;
		if ( (style == null)
			|| !(style instanceof SkinComposor.ScrollBarStyle) )
		{
			outputError( idStyle + " は未定義のスタイルです" ) ;
			return	null ;
		}
		if ( ptAbsolute != null )
		{
			xOffset += ptAbsolute.x ;
			yOffset += ptAbsolute.y ;
		}
		else
		{
			xOffset += style.m_ptStyle.x ;
			yOffset += style.m_ptStyle.y ;
		}
		SkinComposor.ScrollBarStyle
			sbs = (SkinComposor.ScrollBarStyle) style ;
		//
		SkinComposor.ScrollBarItem
				item = new SkinComposor.ScrollBarItem() ;
		m_pageCur.addItem( item ) ;
		//
		item.m_idItem = idItem ;
		item.m_idStyle = idStyle ;
		item.m_ptItem.x = xOffset ;
		item.m_ptItem.y = yOffset ;
		item.m_rectItem.x = xOffset ;
		item.m_rectItem.y = yOffset ;
		item.m_rectItem.w = style.m_szStyle.w ;
		item.m_rectItem.h = style.m_szStyle.h ;
		//
		if ( sbs.m_sType == SkinComposor.sbtVertical )
		{
			item.setAttrIntegerAs( "width", style.m_szStyle.h ) ;
		}
		else
		{
			item.setAttrIntegerAs( "width", style.m_szStyle.w ) ;
		}
		//
		XMLDocument	xmlCmd = item.createElementTagAs( "command" ) ;
		XMLDocument	xmlParam = new XMLDocument() ;
		xmlParam.setTag( "bar" ) ;
		xmlParam.setAttrIntegerAs( "range", nRange ) ;
		xmlCmd.addElement( xmlParam ) ;
		//
		outputMessage( "scroll item: " + idItem ) ;
		return	item ;
	}

	// 編集テキストアイテム追加
	//////////////////////////////////////////////////////////////////////////
	public EditTextItem addEditTextItem
		( String idItem, String idStyle,
			int x, int y, int w, int h, String sText = null,
			int nLimitChars = 0, int nOptFlags = 0 )
	{
		SkinComposor.ItemStyle	style = m_styles[idStyle] ;
		if ( (style == null)
			|| !(style instanceof SkinComposor.EditTextStyle) )
		{
			outputError( idStyle + " は未定義のスタイルです" ) ;
			return	null ;
		}
		//
		SkinComposor.EditTextItem
				item = new SkinComposor.EditTextItem() ;
		m_pageCur.addItem( item ) ;
		//
		item.m_idItem = idItem ;
		item.m_idStyle = idStyle ;
		item.m_ptItem.x = x ;
		item.m_ptItem.y = y ;
		item.m_rectItem.x = x ;
		item.m_rectItem.y = y ;
		item.m_rectItem.w = w ;
		item.m_rectItem.h = h ;
		//
		item.setAttrIntegerAs( "width", w ) ;
		item.setAttrIntegerAs( "height", h ) ;
		if ( sText != null )
		{
			item.setAttributeAs( "text", sText ) ;
		}
		//
		String	sAccept = "" ;
		if ( nOptFlags & SkinComposor.acceptEditReturn )
		{
			sAccept = "return" ;
		}
		if ( nOptFlags & SkinComposor.acceptEditTab )
		{
			if ( sAccept != "" )
			{
				sAccept += " " ;
			}
			sAccept = "tab" ;
		}
		//
		XMLDocument	xmlCmd = item.createElementTagAs( "command" ) ;
		XMLDocument	xmlOpt = new XMLDocument() ;
		xmlOpt.setTag( "option" ) ;
		xmlOpt.setAttrIntegerAs( "limit", nLimitChars ) ;
		xmlOpt.setAttributeAs( "accept", sAccept ) ;
		xmlCmd.addElement( xmlOpt ) ;
		//
		outputMessage( "edit item: " + idItem ) ;
		return	item ;
	}

	public EditTextItem addEditTextItem
		( String idItem, String idStyle,
			String sGuideLayer, String sText = null,
			int nLimitChars = 0, int nOptFlags = 0 )
	{
		int	iLayer = findLayer( sGuideLayer ) ;
		if ( iLayer < 0 )
		{
			outputError( "\'" + sGuideLayer + "\' レイヤーが見つかりません" ) ;
			return	null ;
		}
		ImageComposor	imgcmp = new ImageComposor() ;
		imgcmp.addSelectLayer( m_imgComp.getLayerAt( iLayer ) ) ;
		//
		Rect	rect = imgcmp.calcCutLayers( m_cutThreshold, 0 ) ;
		if ( rect == null )
		{
			outputError( "\'" + sGuideLayer
							+ "\' レイヤーに有効な領域がありません" ) ;
			return	null ;
		}
		return	addEditTextItem
			( idItem, idStyle, rect.x, rect.y,
				rect.w, rect.h, sText, nLimitChars, nOptFlags ) ;
	}

	// メッセージ出力
	//////////////////////////////////////////////////////////////////////////
	public void outputMessage( String msg )
	{
		System.console().printf( "%s\n", msg ) ;
	}

	// エラー出力
	//////////////////////////////////////////////////////////////////////////
	public void outputError( String err )
	{
		System.console().printf( "error: %s\n", err ) ;
	}

} ;




