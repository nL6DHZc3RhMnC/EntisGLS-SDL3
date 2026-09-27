
import "xml_document.rs" ;

class	WebSkin
{
	public XMLDocument	m_xmlDoc = null ;
	public XMLDocument	m_xmlCache = null ;
	public XMLDocument	m_xmlStyles = null ;
	public XMLDocument	m_xmlForms = null ;

	public int			m_nErrors = 0 ;

	// 出力設定
	public String		m_sDstRsrcDir = "" ;
	public String		m_sDstImageMime = "image/png" ;
	public String		m_sDstImageExt = ".png" ;
	public int			m_nDstImageQuality = 0x100 ;

	// 入力設定
	public String		m_sSrcRsrcDir = "" ;
	public NoaFileArchiver	m_nfaNoaFile = null ;

	// リソース
	class	Resource
	{
		public String	m_strFile ;
	}
	class	ImageResource	extends Resource
	{
		public Point	m_ptOffset = new Point() ;
		public long		m_nDuration = 0 ;
		public String[]	m_aAnimeFrames = new String[] ;
	}
	public HashMap		m_mapRsrc = new HashMap( Resource ) ;

	// 画像参照
	class	ImageRef
	{
		public ImageResource	m_img = null ;
		public Rect				m_rect = null ;
	} ;

	// スタイル
	public HashMap		m_mapSrcStyle = new HashMap( XMLDocument ) ;

	// 初期化
	//////////////////////////////////////////////////////////////////////////
	public void initialize( void )
	{
		m_xmlDoc = new XMLDocument() ;
		m_xmlDoc.setTag( "skin" ) ;
		//
		m_xmlCache = m_xmlDoc.createElementTagAs( "cache" ) ;
		m_xmlStyles = m_xmlDoc.createElementTagAs( "styles" ) ;
		m_xmlForms = m_xmlDoc.createElementTagAs( "forms" ) ;
		//
		m_mapRsrc = new HashMap( Resource ) ;
		m_mapSrcStyle = new HashMap( XMLDocument ) ;
	}

	// 画像出力設定
	//////////////////////////////////////////////////////////////////////////
	public void setResourceDestination
		( String sDstDir, String sMime = null,
				String sExt = null, int nQuality = 0x100 )
	{
		m_sDstRsrcDir = sDstDir ;
		if ( sMime != null )
		{
			m_sDstImageMime = sMime ;
		}
		if ( sExt != null )
		{
			m_sDstImageExt = sExt ;
		}
		m_nDstImageQuality = nQuality ;
	}

	// 画像入力設定
	//////////////////////////////////////////////////////////////////////////
	public void setResourceSource( String sSrcDir, NoaFileArchiver noa )
	{
		m_sSrcRsrcDir = sSrcDir ;
		m_nfaNoaFile = noa ;
	}

	// 変換
	//////////////////////////////////////////////////////////////////////////
	public int convertSkin( XMLDocument xmlSkin )
	{
		if ( xmlSkin.getTag() != "skin" )
		{
			xmlSkin = xmlSkin.getElementTagAs( "skin" ) ;
			if ( xmlSkin == null )
			{
				m_nErrors ++ ;
				return	1 ;
			}
		}
		//
		// リソース解釈
		//
		XMLDocument	xmlRsrc = xmlSkin.getElementTagAs( "resource" ) ;
		if ( xmlRsrc != null )
		{
			parseSkinResource( xmlRsrc ) ;
		}
		//
		// スタイル取得
		//
		XMLDocument	xmlStyles = xmlSkin.getElementTagAs( "declare_style" ) ;
		if ( xmlStyles != null )
		{
			int	nCount = xmlStyles.getElementsCount() ;
			for ( int i = 0; i < nCount; i ++ )
			{
				XMLDocument	xmlStyle = xmlStyles.getElementAt( i ) ;
				if ( (xmlStyle == null)
					|| (xmlStyle.getTag() != "style") )
				{
					continue ;
				}
				String	strID = xmlStyle.getAttrStringAs( "id" ) ;
				if ( strID != "" )
				{
					m_mapSrcStyle[strID] = new XMLDocument( xmlStyle ) ;
				}
			}
		}
		//
		// ページフォーム順次解釈
		//
		for ( int i = 0; i < xmlSkin.getElementsCount(); i ++ )
		{
			XMLDocument	xmlPage = xmlSkin.getElementAt( i ) ;
			if ( (xmlPage == null)
				|| (xmlPage.getTag() != "page") )
			{
				continue ;
			}
			parseSkinPage( xmlPage ) ;
		}
		return	m_nErrors ;
	}

	// 保存
	//////////////////////////////////////////////////////////////////////////
	public boolean writeDocument( String sDstFile )
	{
		return	m_xmlDoc.saveDocument( sDstFile ) ;
	}

	// リソース解釈
	//////////////////////////////////////////////////////////////////////////
	public void parseSkinResource( XMLDocument xmlRsrc )
	{
		int	nCount = xmlRsrc.getElementsCount() ;
		for ( int i = 0; i < nCount; i ++ )
		{
			XMLDocument	xmlEntry = xmlRsrc.getElementAt( i ) ;
			if ( xmlEntry == null )
			{
				continue ;
			}
			if ( xmlEntry.getTag() == "image" )
			{
				String	strID = xmlEntry.getAttrStringAs( "id" ) ;
				String	strSrc = xmlEntry.getAttrStringAs( "src" ) ;
				//
				outputLog( strID + "：" + strSrc ) ;
				//
				Image	img = new Image() ;
				if ( m_nfaNoaFile != null )
				{
					RandomAccessFile	raf = m_nfaNoaFile.openFile( strSrc ) ;
					if ( (raf == null) || !img.readImage( raf ) )
					{
						outputError( "読み込みに失敗しました" ) ;
						continue ;
					}
				}
				else
				{
					if ( !img.loadImage( m_sSrcRsrcDir.offsetFilePath( strSrc ) ) )
					{
						outputError( "読み込みに失敗しました" ) ;
						continue ;
					}
				}
				String	strDstFile =
							strSrc.getFileTitlePart() + m_sDstImageExt ;
				Image.BufferInfo	imginf = new Image.BufferInfo() ;
				img.getImageInfo( imginf ) ;
				//
				WebSkin.ImageResource	rsrc = new WebSkin.ImageResource() ;
				rsrc.m_strFile = strDstFile ;
				rsrc.m_ptOffset.x = - imginf.ptOrigin.x ;
				rsrc.m_ptOffset.y = - imginf.ptOrigin.y ;
				rsrc.m_nDuration = img.getTotalTime() ;
				m_mapRsrc[strID] = rsrc ;
				//
				if ( img.getFrameCount() == 1 )
				{
					outputLog( "   --> " + strDstFile ) ;
					if ( !img.saveImage
						( m_sDstRsrcDir.offsetFilePath( strDstFile ),
								m_sDstImageMime, m_nDstImageQuality ) )
					{
						outputError( "書き出しに失敗しました" ) ;
						continue ;
					}
					rsrc.m_aAnimeFrames[0] = strDstFile ;
					//
					XMLDocument	xmlRsrc = new XMLDocument() ;
					xmlRsrc.setTag( "image" ) ;
					xmlRsrc.setAttributeAs( "id", strID ) ;
					xmlRsrc.setAttributeAs( "src", strDstFile ) ;
					m_xmlCache.addElement( xmlRsrc ) ;
				}
				else
				{
					for ( int j = 0; j < img.getFrameCount(); j ++ )
					{
						String	sDstFileI =
									strSrc.getFileTitlePart()
											+ "_" + j + m_sDstImageExt ;
						outputLog( "   --> " + sDstFileI ) ;
						if ( !img.newReference( null, j ).saveImage
							( m_sDstRsrcDir.offsetFilePath( sDstFileI ),
									m_sDstImageMime, m_nDstImageQuality ) )
						{
							outputError( "書き出しに失敗しました" ) ;
							continue ;
						}
						rsrc.m_aAnimeFrames[j] = sDstFileI ;
						//
						XMLDocument	xmlRsrc = new XMLDocument() ;
						xmlRsrc.setTag( "image" ) ;
						if ( j == 0 )
						{
							rsrc.m_strFile = sDstFileI ;
							xmlRsrc.setAttributeAs( "id", strID ) ;
						}
						xmlRsrc.setAttributeAs( "src", sDstFileI ) ;
						m_xmlCache.addElement( xmlRsrc ) ;
					}
				}
			}
		}
	}

	// ページ解釈
	//////////////////////////////////////////////////////////////////////////
	public void parseSkinPage( XMLDocument xmlPage )
	{
		String	strPageID = xmlPage.getAttrStringAs( "id" ) ;
		outputLog( "page：" + strPageID ) ;
		//
		XMLDocument	xmlForm = new XMLDocument() ;
		xmlForm.setTag( "form" ) ;
		xmlForm.setAttributeAs( "id", strPageID ) ;
		m_xmlForms.addElement( xmlForm ) ;
		//
		xmlForm.setAttrIntegerAs
			( "x", xmlPage.getAttrIntegerAs( "x", 0 ) ) ;
		xmlForm.setAttrIntegerAs
			( "y", xmlPage.getAttrIntegerAs( "y", 0 ) ) ;
		xmlForm.setAttrIntegerAs
			( "width", xmlPage.getAttrIntegerAs( "width", 0 ) ) ;
		xmlForm.setAttrIntegerAs
			( "height", xmlPage.getAttrIntegerAs( "height", 0 ) ) ;
		xmlForm.setAttrIntegerAs( "buffered", 1 ) ;
		//
		for ( int i = 0; i < xmlPage.getElementsCount(); i ++ )
		{
			XMLDocument	xmlTag = xmlPage.getElementAt( i ) ;
			if ( xmlTag == null )
			{
				continue ;
			}
			XMLDocument	xmlItem = null ;
			String	strType = xmlTag.getTag() ;
			if ( strType == "image" )
			{
				xmlItem = parseImageItem( xmlTag ) ;
			}
			else if ( strType == "button" )
			{
				xmlItem = parseButtonItem( xmlTag ) ;
			}
			else if ( strType == "static_text" )
			{
				xmlItem = parseStaticTextItem( xmlTag ) ;
			}
			else if ( strType == "progress_bar" )
			{
				xmlItem = parseProgressBarItem( xmlTag ) ;
			}
			else if ( strType == "scroll_bar" )
			{
			}
			else if ( strType == "rectangle" )
			{
			}
			else
			{
				outputError( "<" + strType + "> は処理されません" ) ;
			}
			if ( xmlItem != null )
			{
				String	strID = xmlTag.getAttrStringAs( "id" ) ;
				if ( strID != "" )
				{
					outputLog( "  + " + strID ) ;
					xmlItem.setAttributeAs( "id", strID ) ;
				}
				xmlForm.addElement( xmlItem ) ;
			}
		}
	}

	// image アイテム解釈
	//////////////////////////////////////////////////////////////////////////
	public XMLDocument parseImageItem( XMLDocument xmlTag )
	{
		int			xPos = xmlTag.getAttrIntegerAs( "x" ) ;
		int			yPos = xmlTag.getAttrIntegerAs( "y" ) ;
		String		strRsrc = xmlTag.getAttrStringAs( "rsrc" ) ;
		WebSkin.ImageRef
					imgRef = parseImageRef( strRsrc ) ;
		if ( imgRef.m_img == null )
		{
			outputError( strRsrc + "：解釈できません" ) ;
			return	null ;
		}
		XMLDocument	xmlItem = new XMLDocument() ;
		xmlItem.setTag( "image" ) ;
		//
		xmlItem.setAttrIntegerAs( "x", xPos ) ;
		xmlItem.setAttrIntegerAs( "y", yPos ) ;
		xmlItem.setAttrIntegerAs( "center_x", - imgRef.m_img.m_ptOffset.x ) ;
		xmlItem.setAttrIntegerAs( "center_y", - imgRef.m_img.m_ptOffset.y ) ;
		//
		formatImageRef( xmlItem, imgRef ) ;
		//
		return	xmlItem ;
	}

	// button アイテム解釈
	//////////////////////////////////////////////////////////////////////////
	public XMLDocument parseButtonItem( XMLDocument xmlTag )
	{
		int		xPos = xmlTag.getAttrIntegerAs( "x" ) ;
		int		yPos = xmlTag.getAttrIntegerAs( "y" ) ;
		String	strStyle = xmlTag.getAttrStringAs( "style" ) ;
		//
		XMLDocument	xmlStyle = m_mapSrcStyle[strStyle] ;
		if ( xmlStyle == null )
		{
			outputError( strStyle + "：解釈できません" ) ;
			return	null ;
		}
		XMLDocument	xmlItem = new XMLDocument() ;
		xmlItem.setTag( "button" ) ;
		//
		xmlItem.setAttrIntegerAs( "x", xPos ) ;
		xmlItem.setAttrIntegerAs( "y", yPos ) ;
		//
		// ボタンタイプ
		//
		XMLDocument	xmlArrange = xmlStyle.getElementTagAs( "arrange" ) ;
		String	strType = "button" ;
		if ( xmlArrange != null )
		{
			strType = xmlArrange.getAttrStringAs( "type" ) ;
			if ( strType == "radio" )
			{
				strType = "check" ;
			}
		}
		xmlItem.setAttributeAs( "type", strType ) ;
		//
		// 各状態画像
		//
		String[]	aSrcStatus =
		[
			"normal", "focus", "pushed", "pushed_focus",
			"active", "active_pushed", "disabled", "push_disabled"
		] ;
		String[]	aDstStatus =
		[
			"normal", "focus", "pushed", "pushed_focus",
			"active", "pushed_active", "disable", "pushed_disable"
		] ;
		for ( int i = 0; i < aSrcStatus.length(); i ++ )
		{
			XMLDocument	xmlSrc = xmlStyle.getElementTagAs( aSrcStatus[i] ) ;
			if ( xmlSrc == null )
			{
				continue ;
			}
			String		strImage = xmlSrc.getAttrStringAs( "image" ) ;
			ImageRef	imgRef = parseImageRef( strImage ) ;
			if ( imgRef.m_img == null )
			{
				outputError( strImage + "：解釈できません" ) ;
				continue ;
			}
			XMLDocument	xmlDst = new XMLDocument() ;
			xmlDst.setTag( "image" ) ;
			xmlDst.setAttributeAs( "id", aDstStatus[i] ) ;
			xmlDst.setAttributeAs( "src", imgRef.m_img.m_strFile ) ;
			if ( imgRef.m_rect != null )
			{
				xmlDst.setAttributeAs
					( "rect", String.format
							( "%d,%d,%d,%d", imgRef.m_rect.x, imgRef.m_rect.y,
											imgRef.m_rect.w, imgRef.m_rect.h ) ) ;
			}
			xmlItem.addElement( xmlDst ) ;
		}
		return	xmlItem ;
	}

	// static_text アイテム解釈
	//////////////////////////////////////////////////////////////////////////
	public XMLDocument parseStaticTextItem( XMLDocument xmlTag )
	{
		int		xPos = xmlTag.getAttrIntegerAs( "x" ) ;
		int		yPos = xmlTag.getAttrIntegerAs( "y" ) ;
		int		nWidth = xmlTag.getAttrIntegerAs( "width" ) ;
		int		nHeight = xmlTag.getAttrIntegerAs( "height" ) ;
		String	strStyle = xmlTag.getAttrStringAs( "style" ) ;
		//
		XMLDocument	xmlStyle = m_mapSrcStyle[strStyle] ;
		if ( xmlStyle == null )
		{
			outputError( strStyle + "：解釈できません" ) ;
			return	null ;
		}
		XMLDocument	xmlItem = new XMLDocument() ;
		xmlItem.setTag( "static_text" ) ;
		//
		xmlItem.setAttrIntegerAs( "x", xPos ) ;
		xmlItem.setAttrIntegerAs( "y", yPos ) ;
		xmlItem.setAttrIntegerAs( "width", nWidth ) ;
		xmlItem.setAttrIntegerAs( "height", nHeight ) ;
		//
		XMLDocument	xmlArrange = xmlStyle.getElementTagAs( "arrange" ) ;
		if ( xmlArrange != null )
		{
			xmlItem.setAttrIntegerAs
				( "line_height",
					xmlArrange.getAttrIntegerAs( "line_height", 16 ) ) ;
			xmlItem.setAttrIntegerAs
				( "pitch",
					xmlArrange.getAttrIntegerAs( "pitch", 0 ) ) ;
		}
		XMLDocument	xmlFont = xmlStyle.getElementTagAs( "font" ) ;
		if ( xmlFont != null )
		{
			xmlItem.setAttrIntegerAs
				( "size", xmlFont.getAttrIntegerAs( "size", 16 ) ) ;
			xmlItem.setAttributeAs
				( "font", xmlFont.getAttrStringAs( "face" ) ) ;
		}
		XMLDocument	xmlText = xmlStyle.getElementTagAs( "text" ) ;
		if ( xmlText != null )
		{
			int	rgbText = xmlText.getAttrIntegerAs( "color", 0xFFFFFF ) ;
			xmlItem.setAttributeAs( "color", String.format( "%06X", rgbText ) ) ;
		}
		XMLDocument	xmlBorder = xmlStyle.getElementTagAs( "border" ) ;
		if ( xmlBorder != null )
		{
			if ( xmlBorder.getAttrIntegerAs( "transparency" ) < 0x80 )
			{
				xmlItem.setAttrIntegerAs( "bodering", 1 ) ;
			}
		}
		return	xmlItem ;
	}

	// progress_bar アイテム解釈
	//////////////////////////////////////////////////////////////////////////
	public XMLDocument parseProgressBarItem( XMLDocument xmlTag )
	{
		int		xPos = xmlTag.getAttrIntegerAs( "x" ) ;
		int		yPos = xmlTag.getAttrIntegerAs( "y" ) ;
		int		nWidth = xmlTag.getAttrIntegerAs( "width" ) ;
		String	strStyle = xmlTag.getAttrStringAs( "style" ) ;
		//
		XMLDocument	xmlStyle = m_mapSrcStyle[strStyle] ;
		if ( xmlStyle == null )
		{
			outputError( strStyle + "：解釈できません" ) ;
			return	null ;
		}
		XMLDocument	xmlItem = new XMLDocument() ;
		xmlItem.setTag( "progress_bar" ) ;
		//
		xmlItem.setAttrIntegerAs( "x", xPos ) ;
		xmlItem.setAttrIntegerAs( "y", yPos ) ;
		xmlItem.setAttrIntegerAs( "width", nWidth ) ;
		//
		XMLDocument	xmlArrange = xmlStyle.getElementTagAs( "arrange" ) ;
		if ( xmlArrange != null )
		{
			xmlItem.addElement( new XMLDocument( xmlArrange ) ) ;
		}
		XMLDocument	xmlFrame = xmlStyle.getElementTagAs( "frame" ) ;
		if ( xmlFrame != null )
		{
			xmlItem.addElement( new XMLDocument( xmlFrame ) ) ;
		}
		XMLDocument	xmlBar = xmlStyle.getElementTagAs( "bar" ) ;
		if ( xmlBar != null )
		{
			xmlItem.addElement( new XMLDocument( xmlBar ) ) ;
		}
		return	xmlItem ;
	}

	// 画像参照解釈
	//////////////////////////////////////////////////////////////////////////
	public ImageRef parseImageRef( String sImageID )
	{
		WebSkin.ImageRef	imgRef = new WebSkin.ImageRef() ;
		String		sRectUsage = ":RECT(" ;
		const int	iSep = sImageID.indexOf( sRectUsage ) ;
		if ( iSep >= 0 )
		{
			imgRef.m_img =
				(WebSkin.ImageResource) m_mapRsrc[sImageID.substring(0,iSep)] ;
			//
			String[]		aParams = new String[] ;
			UsageMatcher	usage = new UsageMatcher
										( "(%n) , (%n) , (%n) , (%n)" ) ;
			StringParser	sparsRect = new StringParser() ;
			sparsRect.attachString( sImageID ) ;
			sparsRect.seekIndex( iSep + sRectUsage.length() ) ;
			if ( usage.parseNext( sparsRect, aParams ) == null )
			{
				imgRef.m_rect =
					new Rect( (int) aParams[0], (int) aParams[1],
								(int) aParams[2], (int) aParams[3] ) ;
			}
		}
		else
		{
			imgRef.m_img = (WebSkin.ImageResource) m_mapRsrc[sImageID] ;
		}
		return	imgRef ;
	}

	// 画像指定変換
	//////////////////////////////////////////////////////////////////////////
	public void formatImageRef( XMLDocument xmlTag, ImageRef imgRef )
	{
		String	strRect = null ;
		if ( imgRef.m_rect != null )
		{
			strRect = String.format
						( "%d,%d,%d,%d", imgRef.m_rect.x, imgRef.m_rect.y,
										imgRef.m_rect.w, imgRef.m_rect.h ) ;
		}
		if ( (imgRef.m_img.m_aAnimeFrames == null)
			|| (imgRef.m_img.m_aAnimeFrames.length() <= 1) )
		{
			xmlTag.setAttributeAs( "src", imgRef.m_img.m_strFile ) ;
			if ( strRect != null )
			{
				xmlTag.setAttributeAs( "rect", strRect ) ;
			}
		}
		else
		{
			xmlTag.setAttrIntegerAs
				( "duration", imgRef.m_img.m_nDuration ) ;
			//
			for ( int i = 0; i < imgRef.m_img.m_aAnimeFrames.length(); i ++ )
			{
				XMLDocument	xmlImage = new XMLDocument() ;
				xmlImage.setTag( "image" ) ;
				xmlImage.setAttributeAs
					( "src", imgRef.m_img.m_aAnimeFrames[i] ) ;
				if ( strRect != null )
				{
					xmlImage.setAttributeAs( "rect", strRect ) ;
				}
				xmlTag.addElement( xmlImage ) ;
			}
		}
	}

	// ログ出力
	//////////////////////////////////////////////////////////////////////////
	void outputLog( String sMsg )
	{
		System.console().printf( "%s\n", sMsg ) ;
	}

	// エラー出力
	//////////////////////////////////////////////////////////////////////////
	void outputError( String sMsg )
	{
		System.console().printf( "error:%s\n", sMsg ) ;
		m_nErrors ++ ;
	}

}


// エントリポイント
//////////////////////////////////////////////////////////////////////////////
int main( String[] arg )
{
	//
	// 入出力ファイル取得
	//
	Console	con = System.console() ;
	String	sSrcNoa ;
	if ( arg.length() >= 1 )
	{
		sSrcNoa = arg[0] ;
	}
	else
	{
		con.printf( "入力スキンファイル：" ) ;
		sSrcNoa = con.readLine().trim() ;
	}
	String	sDstFile ;
	if ( arg.length() >= 2 )
	{
		sDstFile = arg[1] ;
	}
	else
	{
		con.printf( "出力ファイル：" ) ;
		sDstFile = con.readLine().trim() ;
	}
	String	sDstRsrcDir =
			sDstFile.getFileDirectoryPart().offsetFilePath( "image" ) ;
	File	fileDstRsrcDir = new File( sDstRsrcDir ) ;
	if ( !fileDstRsrcDir.exists() )
	{
		fileDstRsrcDir.mkdirs() ;
	}
	//
	// 入力ファイルを開く
	//
	NoaFileArchiver	noaSkin = new NoaFileArchiver() ;
	try
	{
		RandomAccessFile	rafSrc = new RandomAccessFile( sSrcNoa, "r" ) ;
		if ( !noaSkin.openArchive( rafSrc ) )
		{
			con.printf
				( "%s は有効な noa 書庫ファイルではありません\n", sSrcNoa ) ;
			return	1 ;
		}
	}
	catch ( Exception e )
	{
		con.printf( "%s を開けませんでした\n", sSrcNoa ) ;
		return	1 ;
	}
	//
	// xml 読み込み
	//
	RandomAccessFile	rafInf = noaSkin.openFile( "system.inf" ) ;
	if ( rafInf == null )
	{
		con.printf
			( "%s は有効なスキンファイルではありません\n", sSrcNoa ) ;
		return	1 ;
	}
	XMLDocument	xmlSkin = new XMLDocument() ;
	if ( !xmlSkin.readDocument
		( rafInf.getInputStream(), new ParserErrorTracer() ) )
	{
		con.printf( "system.inf の読み込みに失敗しました\n" ) ;
		return	1 ;
	}
	//
	// 処理開始
	//
	WebSkin	ws = new WebSkin() ;
	ws.initialize() ;
	ws.setResourceDestination( sDstRsrcDir ) ;
	ws.setResourceSource( null, noaSkin ) ;
	//
	int	nErrors = ws.convertSkin( xmlSkin ) ;
	if ( nErrors == 0 )
	{
		if ( ws.writeDocument( sDstFile ) )
		{
			con.printf( "正常に終了しました\n" ) ;
		}
		else
		{
			con.printf( "%s への書き出しに失敗しました\n", sDstFile ) ;
			nErrors ++ ;
		}
	}
	else
	{
		con.printf( "%d のエラーが発生しました\n", nErrors ) ;
	}
	return	nErrors ;
}


