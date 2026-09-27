
import "xml_document.rs" ;

Console	con = System.console() ;


// PSD ファイル・コンバーター
//////////////////////////////////////////////////////////////////////////////
//
// PSD ファイルから EntisGLS4 BasicForm を生成
// レイヤー名書式;
// @<type>:<id>[,[<opt_id>][,<name1>=<param1>[,<name2>=<param2>[,...]]]]
//
// @ 記号から始まる名前のレイヤーを処理
// id は + から始まる場合、親レイヤーの id を継承し後ろに追加
//
// ex.
//   @form:page1
//      @image:+_item1
//
// "@form:page1" レイヤーがフォルダで、"@image:+_item1" がその中にある場合、
// "@image:+_item1" は "@image:page1_item1" と同義
//
// opt_id は参照する画像IDや画像セットIDを指定する。
// opt_id 以降は、追加的なパラメータを記述できる。
//
// レイヤータイプ；
//
// @form:<form-id>[,[,sdir={vert|horz}[,margin=<inner-margin>]][,sbar=<scroll-bar-id>]]
//	このレイヤーのサブレイヤーにフォームを記述
//	またフォーム内の場合にはサブフォームアイテムを追加する
//	パラメータにはスクロール方向の設定(sdir)や、関連付けるスクロール用の
//	トラックバーを指定できる
//	スクロールできるのはフォーム内に @alpha_mask レイヤーで表示範囲を指定した場合
//
// @image:<id>[,[<image-id>][,st=<frame-id>]]
//	このレイヤー画像を画像識別子 id として保存
//	image-id を指定した場合には画像識別子 image-id として保存
//	画像セット（@images）子レイヤーの場合には画像セットに追加する
//	フォーム（@form）配下レイヤーの場合には画像アイテムとして追加する
//	その場合の画像アイテム識別子は id
//	ボタン画像セット（@button|@check）子レイヤーの場合、
//	パラメータ st=<frame-id> で対応するボタンステータスを指定する
//		st={normal|focus|pushed|pushed_focus|pushing|disable|pushed_disable}
//	トラックバー（@vtrack|@htrack）子レイヤーの場合、
//	パラメータ st=<frame-id> で対応するボタンステータスを指定する
//		st={normal|focus|disable}
//
// @images:<id>[,,track=<track-item-id>]
//	このフォルダ内の @image レイヤーを要素とする画像セットを作成する
//	フォーム（@form）配下レイヤーの場合には画像選択アイテムとして追加する
//
// @anime:<id>,[<image-id>][,{dr=<total-time>|fps=<frame-per-sec>}]
//	このフォルダ内の全レイヤーを要素とするアニメーション画像を作成する
//	画像識別子 image-id として保存。省略した場合は id
//	フォーム（@form）配下レイヤーの場合には画像アイテムとして追加する
//
// @iref:<id>,<ref-image-id>
//	ref-image-id 画像を参照する、アイテム識別子 ID の画像アイテムを追加する
//
// @imgsel:<id>,<ref-imageset-id>[,track=<track-item-id>]
//	ref-imageset-id 画像セットを参照する、アイテム識別子 ID の画像選択
//	アイテムを追加する
//	track-item-id を指定する場合、このアイテムをトラックバーアイテムの
//	背景アイテムとして動作させる
//	トラックバーの背景アイテムは、トラックバーのステータスと連動される
//
// @button:<id>[,<ref-imageset-id>]
//	このフォルダの子レイヤー @image を画像要素に持つボタンアイテムを追加する
//	ref-imageset-id を指定する場合、このレイヤー画像の位置に指定画像セットの
//	要素を持つボタンアイテムを追加する
//
// @check:<id>[,<ref-imageset-id>]
//	@button と同じだが、トグルボタンとなる。
//
// @mask
//	このレイヤー自体は何も生成しないが、画像セットなどで切り出し矩形を
//	明示したい場合に配置する
//
// @track
//	このレイヤー自体は何も生成しないが、@vtrack, @htrack のトラック領域を
//	指定するために配置する
//
// @alpha_mask:<id>
//	このレイヤー自体はアイテムを生成しないが、サブフォームアイテムの場合
//	αマスクを設定する
//	id には保存するαマスク画像の識別子を指定する
//
// @id:<id>
//	このレイヤー自体は何も生成しないが、このレイヤー配下のベースIDを
//	設定したい場合に @id を使用する
//
// @rect:<id>
//	このレイヤーの矩形位置とサイズを矩形識別子 id として登録する
//
// @hbar:<id>[,<ref-image-id>]
//	このレイヤーの位置に、このレイヤー画像、又は ref-image-id の画像の
//	水平バーアイテムを追加する
//	以下のパラメータを指定できる
//		inv={0|1}     (反転（水平なら右から、垂直なら下から伸びる）バーか？)
//
// @vbar:<id>[,<ref-image-id>]
//	このレイヤーの位置に、このレイヤー画像、又は ref-image-id の画像の
//	垂直バーアイテムを追加する
//
// @htrack:<id>[,<ref-image-id>]
//	このレイヤーの位置に、この子レイヤー画像、又は ref-image-id の画像の
//	水平トラックバーアイテムを追加する
//	このフォルダの子レイヤー @image を画像要素がバーの表示用状態画像となり
//	このレイヤー（又はレイヤーグループ内の @track）がトラック領域となる
//
// @vtrack:<id>[,<ref-image-id>]
//	このレイヤーの位置に、この子レイヤー画像、又は ref-image-id の画像の
//	垂直トラックバーアイテムを追加する
//	@htrack と同じ
//
// @text:<id>[,[<ref-text-style][,...]]
//	テキストアイテムをこのレイヤーの位置に追加する
//	ref-text-style は既に作成されたテキストアイテムのIDを指定して同じスタイルを
//	そうでない場合には以下のパラメータを指定できます
//		size=<font-size>
//		line=<line-height>
//		font=<font-face>
//		color=<text-color-hex>
//		border=<border-color-hex>
//	色指定は6桁の16進数です。border は無指定の場合にはフチなし文字です
//
// @msg:<id>[,[<ref-text-style][,...]]
//	メッセージアイテムをこのレイヤーの位置に追加する
//	パラメータは @text と同じ
//	以下の追加パラメータを指定できる
//		buf={0|1}     (フレームバッファを保持するか？)
//
// @rtxt:<id>[,[<ref-text-style][,...]]
//	リッチテキストアイテムをこのレイヤーの位置に追加する
//	パラメータは @message と同じ
//	以下の追加パラメータも指定できる
//		scroll=<track-bar-id>
//
// @sframe:<id>[,<def-style-id>]
//	フレームスタイルを定義する
//	子レイヤーにはフレームパーツ画像と切り出し矩形を含む
//	def-style-id を指定した場合にはその識別子でスタイルを定義し、
//	フレーム画像識別子は id からの差分を指定できる。
//
//		@frame_image
//			フレーム画像。このレイヤー画像を切り出す。
//		@back_frame:<id>[,<ref-image-id>]
//			背景フレーム画像。
//			ref-image-id を指定した場合、既に存在する画像を使用する。
//		@client
//			クライアント領域。
//		@caption:<ref-text-style>
//			キャプション領域。
//			ref-text-style にはキャプション文字を表示するためのスタイルを指定する。
//		@upper_left:<id>[,<ref-image-id>]
//		@upper_center_l:<id>[,<ref-image-id>]
//		@upper_center:<id>[,<ref-image-id>]
//		@upper_center_r:<id>[,<ref-image-id>]
//		@upper_right:<id>[,<ref-image-id>]
//		@left_upper:<id>[,<ref-image-id>]
//		@left:<id>[,<ref-image-id>]
//		@left_under:<id>[,<ref-image-id>]
//		@right_upper:<id>[,<ref-image-id>]
//		@right:<id>[,<ref-image-id>]
//		@right_under:<id>[,<ref-image-id>]
//		@under_left:<id>[,<ref-image-id>]
//		@under_center_l:<id>[,<ref-image-id>]
//		@under_center:<id>[,<ref-image-id>]
//		@under_center_r:<id>[,<ref-image-id>]
//		@under_right:<id>[,<ref-image-id>]
//			@frame_image レイヤーを切り出す矩形を指定する。
//			id は切り出した画像の識別子を指定する。
//			ref-image-id を指定した場合、既に存在する画像を使用する。
//
// @xframe:<id>,<style-id>
//	フレームアイテムをこのレイヤーの位置（外接）に追加する。
//
// @iframe:<id>,<style-id>
//	フレームアイテムをこのレイヤーの位置（内接）に追加する。
//
//////////////////////////////////////////////////////////////////////////////

class	PSDConverter
{
	protected ImageComposition	m_psd = new ImageComposition() ;
	protected String			m_sPsdFile = null ;
	protected String			m_sDstDir = "" ;
	protected String			m_sRsrcDir = "" ;
	protected String			m_sFormXml = null ;
	protected String			m_sCompProj = null ;
	protected int				m_nClipMargin = 0 ;
	protected int				m_nClipThreshold = 0 ;
	protected String			m_sStyleFile = null ;

	protected int				m_nErrors = 0 ;

	class	ImageSet
	{
		public String[]			m_idRefs = new String[] ;
		public String[]			m_idFrames = new String[] ;
	}

	class	FormStyle
	{
		public XMLDocument	m_tags = new XMLDocument() ;

		public const FormStyle clone( void )
		{
			PSDConverter.FormStyle	style = new PSDConverter.FormStyle() ;
			style.m_tags = new XMLDocument( m_tags ) ;
			return	style ;
		}
	}

	class	FormItem
	{
		public String			m_id ;
		public String			m_type ;
		public Rect				m_rect = null ;
		public HashMap<String>	m_params = null ;
		public XMLDocument		m_tags = null ;
	}

	class	FormData
	{
		public String		m_id ;
		public Rect			m_rect = null ;
		public String		m_idAlpha = null ;
		public Rect			m_rctAlpha = null ;
		public FormItem[]	m_items = new FormItem[] ;
	}

	protected HashMap<Rect>			m_mapRects = new HashMap<Rect>() ;
	protected HashMap<String>		m_mapImages = new HashMap<String>() ;
	protected HashMap<ImageSet>		m_mapImageSets = new HashMap<ImageSet>() ;
	protected HashMap<FormStyle>	m_mapStyles = new HashMap<FormStyle>() ;
	protected HashMap<FormData>		m_mapForms = new HashMap<FormData>() ;


	static public const int	CONTEXT_FLAG_IMAGES	= 0x00000001 ;

	class	Context
	{
		public String	m_id = "" ;
		public Rect		m_clip = null ;
		public ImageSet	m_imgset = null ;
		public FormData	m_form = null ;
		public int		m_flags = 0 ;
		public String	m_imgFirst = null ;

		public Context()
		{
		}
		public Context( Context context )
		{
			m_id = context.m_id ;
			m_clip = context.m_clip ;
			m_imgset = context.m_imgset ;
			m_form = context.m_form ;
			m_flags = context.m_flags ;
		}
	}

	class	LayerParam
	{
		public String			m_cmd = null ;
		public String			m_id = null ;
		public String			m_opt = null ;
		public HashMap<String>	m_params = null ;
	}

	static public const int		ARG_SRC_PSD			= 0 ;
	static public const int		ARG_DST_DIR			= 1 ;
	static public const int		ARG_DST_FORM		= 2 ;
	static public const int		ARG_DST_COMP		= 3 ;
	static public const int		ARG_CLIP_MARGIN		= 4 ;
	static public const int		ARG_CLIP_THRESHOLD	= 5 ;
	static public const int		ARG_STYLE_FILE		= 6 ;

	// 引数解釈
	public boolean parseCmdLine( String[] arg )
	{
		int	nArgOpt = ARG_SRC_PSD ;
		for ( int i = 0; i < arg.length(); i ++ )
		{
			if ( arg[i] == "/dst_dir" )
			{
				nArgOpt = ARG_DST_DIR ;
			}
			else if ( arg[i] == "/src" )
			{
				nArgOpt = ARG_SRC_PSD ;
			}
			else if ( arg[i] == "/form" )
			{
				nArgOpt = ARG_DST_FORM ;
			}
			else if ( arg[i] == "/comp" )
			{
				nArgOpt = ARG_DST_COMP ;
			}
			else if ( arg[i] == "/margin" )
			{
				nArgOpt = ARG_CLIP_MARGIN ;
			}
			else if ( arg[i] == "/threshold" )
			{
				nArgOpt = ARG_CLIP_THRESHOLD ;
			}
			else if ( arg[i] == "/style" )
			{
				nArgOpt = ARG_STYLE_FILE ;
			}
			else
			{
				try
				{
					switch ( nArgOpt )
					{
					case	ARG_SRC_PSD:
						m_sPsdFile = arg[i] ;
						break ;
					case	ARG_DST_DIR:
						m_sDstDir = arg[i] ;
						break ;
					case	ARG_DST_FORM:
						m_sFormXml = arg[i] ;
						if ( m_sFormXml.getFileExtensionPart() == "" )
						{
							m_sFormXml += ".xmlfrm" ;
						}
						break ;
					case	ARG_DST_COMP:
						m_sCompProj = arg[i] ;
						if ( m_sCompProj.getFileExtensionPart() == "" )
						{
							m_sCompProj += ".xmlprs" ;
						}
						break ;
					case	ARG_CLIP_MARGIN:
						m_nClipMargin = arg[i].asInteger() ;
						break ;
					case	ARG_CLIP_THRESHOLD:
						m_nClipThreshold = arg[i].asInteger() ;
						break ;
					case	ARG_STYLE_FILE:
						m_sStyleFile = arg[i] ;
						break ;
					}
				}
				catch ( Exception e )
				{
				}
				nArgOpt = ARG_SRC_PSD ;
			}
		}
		if ( m_sPsdFile == null )
		{
			con.printf( "PSD ファイルが指定されていません。\n" ) ;
			return	false ;
		}
		if ( m_sStyleFile != null )
		{
			if ( !loadStyleDef( m_sStyleFile ) )
			{
				con.printf( "\'%s\' を読み込めませんでした。\n", m_sStyleFile ) ;
				return	false ;
			}
		}
		con.printf( "loading \'%s\'...\n", m_sPsdFile ) ;
		if ( !m_psd.loadPsdFile( m_sPsdFile ) )
		{
			con.printf( "\'%s\' を読み込めませんでした。\n", m_sPsdFile ) ;
			return	1 ;
		}
		m_sRsrcDir = m_sDstDir ;
		if ( m_sCompProj != null )
		{
			String	sProjDir = m_sCompProj.getFileDirectoryPart() ;
			m_sRsrcDir = sProjDir.relativeFilePath( m_sDstDir ) ;
		}
		return	true ;
	}

	// スタイル定義ファイル読み込み
	public boolean loadStyleDef( String sFilePath )
	{
		XMLDocument	xmlStyleDef = new XMLDocument() ;
		if ( !xmlStyleDef.loadDocument( sFilePath, new ParserErrorTracer() ) )
		{
			con.printf( "\'%s\' を読み込めませんでした。\n", sFilePath ) ;
			return	false ;
		}
		XMLDocument	xmlStyles = xmlStyleDef.getElementTagAs( "styles" ) ;
		if ( xmlStyles == null )
		{
			return	true ;
		}
		for ( int i = 0; i < xmlStyles.getElementsCount(); i ++ )
		{
			XMLDocument	xmlTag = xmlStyles.getElementAt( i ) ;
			if ( (xmlTag == null) || (xmlTag.getTag() != "style") )
			{
				continue ;
			}
			String	id = xmlTag.getAttributeAs( "id" ) ;
			if ( id == null )
			{
				continue ;
			}
			PSDConverter.FormStyle	style = new PSDConverter.FormStyle() ;
			style.m_tags = new XMLDocument( xmlTag ) ;
			m_mapStyles.put( id, style ) ;
		}
		return	true ;
	}

	// 実行
	public int run()
	{
		const int	nLayerCount = m_psd.getLayerTreeCount() ;
		for ( int i = 0; i < nLayerCount; i ++ )
		{
			ImageComposition.Layer	layer = m_psd.getLayerTreeAt( i ) ;
			prosessLayer( layer, new PSDConverter.Context() ) ;
		}
		if ( m_sFormXml != null )
		{
			saveFormData( m_sFormXml ) ;
		}
		if ( m_sCompProj != null )
		{
			saveComposition( m_sCompProj ) ;
		}
		return	m_nErrors ;
	}

	public void prosessLayer
		( ImageComposition.Layer layer, Context context )
	{
		PSDConverter.LayerParam	lp = parseLayerParam( layer.getName(), context ) ;
		if ( lp.m_cmd != null )
		{
			if ( lp.m_cmd == "image" )
			{
				prosessImageLayer( layer, lp, context ) ;
			}
			else if ( lp.m_cmd == "images" )
			{
				prosessImageSetLayer( layer, lp, context ) ;
			}
			else if ( lp.m_cmd == "anime" )
			{
				prosessAnimeLayer( layer, lp, context ) ;
			}
			else if ( lp.m_cmd == "iref" )
			{
				if ( lp.m_opt != null )
				{
					addImageItem
						( lp.m_id, lp.m_opt, getLayerRect( layer ), context ) ;
				}
			}
			else if ( lp.m_cmd == "imgsel" )
			{
				if ( lp.m_opt != null )
				{
					addImageSelItem
						( lp.m_id, lp.m_opt, lp.m_params,
							getLayerRect( layer ), context ) ;
				}
			}
			else if ( lp.m_cmd == "button" )
			{
				if ( lp.m_opt != null )
				{
					addButtonItem
						( lp.m_id, lp.m_opt, false,
							getLayerRect( layer ), context ) ;
				}
				else
				{
					prosessImageSetLayer( layer, lp, context ) ;
				}
			}
			else if ( lp.m_cmd == "check" )
			{
				if ( lp.m_opt != null )
				{
					addButtonItem
						( lp.m_id, lp.m_opt, true,
							getLayerRect( layer ), context ) ;
				}
				else
				{
					prosessImageSetLayer( layer, lp, context ) ;
				}
			}
			else if ( lp.m_cmd == "id" )
			{
				PSDConverter.Context 
					ctxSub = new PSDConverter.Context( context ) ;
				if ( lp.m_id != null )
				{
					ctxSub.m_id = lp.m_id ;
				}
				prosessLayerGroup( layer, ctxSub ) ;
			}
			else if ( lp.m_cmd == "rect" )
			{
				if ( lp.m_id != null )
				{
					registerRect( lp.m_id, layer.layerSizeOf() ) ;
				}
			}
			else if ( (lp.m_cmd == "hbar")
					|| (lp.m_cmd == "vbar") )
			{
				prosessGaugeBarLayer( layer, lp, context ) ;
			}
			else if ( (lp.m_cmd == "htrack")
					|| (lp.m_cmd == "vtrack") )
			{
				prosessTrackBarLayer( layer, lp, context ) ;
			}
			else if ( (lp.m_cmd == "text")
					|| (lp.m_cmd == "msg")
					|| (lp.m_cmd == "rtxt") )
			{
				prosessTextLayer( layer, lp, context ) ;
			}
			else if ( lp.m_cmd == "form" )
			{
				prosessFormLayer( layer, lp, context ) ;
			}
			else if ( lp.m_cmd == "sframe" )
			{
				prosessFrameStyle( layer, lp, context ) ;
			}
			else if ( lp.m_cmd == "xframe" )
			{
				prosessFrameItem( layer, lp, context ) ;
			}
			else if ( lp.m_cmd == "iframe" )
			{
				prosessInnerFrameItem( layer, lp, context ) ;
			}
			else if ( lp.m_cmd == "alpha" )
			{
				prosessAlphaLayer( layer, lp, context ) ;
			}
			else if ( lp.m_cmd == "mask" )
			{
			}
			else if ( lp.m_cmd == "track" )
			{
			}
			else
			{
				outputError( "\'" + layer.getName() + "\' レイヤーは処理されません" ) ;
			}
		}
	}

	public void prosessFormLayer
		( ImageComposition.Layer layer,
			PSDConverter.LayerParam	lp, Context context )
	{
		PSDConverter.Context 
			ctxSub = new PSDConverter.Context( context ) ;
		if ( lp.m_id != null )
		{
			if ( m_mapForms.get(lp.m_id) != null )
			{
				outputError( "form のＩＤが重複しています \'" + lp.m_id + "\'" ) ;
			}
			ctxSub.m_id = lp.m_id ;
			ctxSub.m_form = new PSDConverter.FormData() ;
			ctxSub.m_form.m_id = lp.m_id ;
			//
			prosessLayerGroup( layer, ctxSub ) ;
			//
			normalizeFormRect( ctxSub.m_form ) ;
			m_mapForms.put( lp.m_id, ctxSub.m_form ) ;
			//
			addFormItem
				( lp.m_id, lp.m_id, lp.m_params, ctxSub.m_form.m_rect, context ) ;
		}
		else
		{
			outputError( "form にＩＤの指定がありません\n" ) ;
		}
	}

	// @image
	public void prosessImageLayer
		( ImageComposition.Layer layer,
			PSDConverter.LayerParam	lp, Context context )
	{
		String	idImage = lp.m_id ;
		if ( lp.m_opt != null )
		{
			idImage = lp.m_opt ;
		}
		Rect	rectImage = saveLayerImage( layer, idImage, context ) ;
		if ( rectImage != null )
		{
			registerRect( lp.m_id, rectImage ) ;
			//
			if ( context.m_imgset != null )
			{
				context.m_imgset.m_idRefs.add( idImage ) ;
				//
				if ( (lp.m_params != null)
					&& (lp.m_params.get("st") != null) )
				{
					while ( context.m_imgset.m_idFrames.length() + 1
								< context.m_imgset.m_idRefs.length() )
					{
						context.m_imgset.m_idFrames.add( null ) ;
					}
					context.m_imgset.m_idFrames.add( lp.m_params.get("st") ) ;
				}
			}
			if ( (context.m_flags & CONTEXT_FLAG_IMAGES) == 0 )
			{
				addImageItem( lp.m_id, idImage, rectImage, context ) ;
			}
			else if ( context.m_imgFirst == null )
			{
				context.m_imgFirst = idImage ;
			}
		}
	}

	public String prosessImageSet
		( Rect rectImageSet,
			ImageComposition.Layer layer,
			PSDConverter.LayerParam	lp, Context context )
	{
		PSDConverter.Context 
			ctxSub = new PSDConverter.Context( context ) ;
		if ( lp.m_id != null )
		{
			ctxSub.m_id = lp.m_id ;
		}
		ctxSub.m_flags |= CONTEXT_FLAG_IMAGES ;
		ctxSub.m_clip = getLayerGroupRect( layer, context ) ;
		ctxSub.m_imgset = new PSDConverter.ImageSet() ;
		//
		prosessLayerGroup( layer, ctxSub ) ;
		//
		if ( lp.m_id != null )
		{
			String	idImgSet = lp.m_opt ;
			if ( idImgSet == null )
			{
				idImgSet = lp.m_id ;
			}
			m_mapImageSets.put( idImgSet, ctxSub.m_imgset ) ;
			rectImageSet.copy( ctxSub.m_clip ) ;
			return	idImgSet ;
		}
		return	null ;
	}

	// @images
	public void prosessImageSetLayer
		( ImageComposition.Layer layer,
			PSDConverter.LayerParam	lp, Context context )
	{
		Rect	rectImageSet = new Rect() ;
		String	idImgSet =
			prosessImageSet( rectImageSet, layer, lp, context ) ;
		//
		if ( (lp.m_id != null) && (idImgSet != null) )
		{
			if ( lp.m_cmd == "images" )
			{
				addImageSelItem
					( lp.m_id, idImgSet, lp.m_params,
								rectImageSet, context ) ;
			}
			else if ( lp.m_cmd == "button" )
			{
				addButtonItem
					( lp.m_id, idImgSet, false, rectImageSet, context ) ;
			}
			else if ( lp.m_cmd == "check" )
			{
				addButtonItem
					( lp.m_id, idImgSet, true, rectImageSet, context ) ;
			}
		}
	}

	// @anime
	public void prosessAnimeLayer
		( ImageComposition.Layer layer,
			PSDConverter.LayerParam	lp, Context context )
	{
		String	idImage = lp.m_id ;
		if ( lp.m_opt != null )
		{
			idImage = lp.m_opt ;
		}
		Rect	rectImage = saveLayerGroupAnimeImage( layer, idImage, lp ) ;
		if ( rectImage != null )
		{
			registerRect( lp.m_id, rectImage ) ;
			//
			if ( (context.m_flags & CONTEXT_FLAG_IMAGES) == 0 )
			{
				addImageItem( lp.m_id, idImage, rectImage, context ) ;
			}
			else if ( context.m_imgFirst == null )
			{
				context.m_imgFirst = idImage ;
			}
		}
	}

	// @sframe
	public void prosessFrameStyle
		( ImageComposition.Layer layer,
			PSDConverter.LayerParam	lp, Context context )
	{
		String	idFrameStyle = lp.m_id ;
		if ( lp.m_opt != null )
		{
			idFrameStyle = lp.m_opt ;
		}
		PSDConverter.Context 
			ctxSub = new PSDConverter.Context( context ) ;
		if ( lp.m_id != null )
		{
			ctxSub.m_id = lp.m_id ;
		}
		// 画像レイヤー情報収集
		HashMap	mapParts =
		{
			upper_left : 0,
			upper_center_l : 1,
			upper_center : 2,
			upper_center_r : 3,
			upper_right : 4,
			left_upper : 5,
			left : 6,
			left_under : 7,
			right_upper : 8,
			right : 9,
			right_under : 10,
			under_left : 11,
			under_center_l : 12,
			under_center : 13,
			under_center_r : 14,
			under_right : 15,
		} ;
		const int					nCount = layer.getChildrenCount() ;
		ImageComposition.Layer		lyImage = null ;
		ImageComposition.Layer		lyBackImage = null ;
		ImageComposition.Layer[]	aLyParts = new ImageComposition.Layer[] ;
		String[]					aPartsIDs = new String[] ;
		String[]					aPartsImageIDs = new String[] ;
		String[]					aPartsRefImageIDs = new String[] ;
		Rect[]						aRctParts = new Rect[] ;
		Rect						rectParts = null ;
		Rect						rectInner = null ;
		String						sCaptionStyle = null ;
		Rect						rectCaption = null ;
		for ( int i = 0; i < nCount; i ++ )
		{
			ImageComposition.Layer	child = layer.getChildAt( i ) ;
			PSDConverter.LayerParam	lpChild =
						parseLayerParam( child.getName(), ctxSub ) ;
			if ( lpChild.m_cmd != null )
			{
				if ( lpChild.m_cmd == "frame_image" )
				{
					lyImage = child ;
				}
				else if ( lpChild.m_cmd == "back_frame" )
				{
					lyBackImage = child ;
				}
				else if ( lpChild.m_cmd == "client" )
				{
					rectInner = child.layerSizeOf() ;
				}
				else if ( lpChild.m_cmd == "caption" )
				{
					rectCaption = child.layerSizeOf() ;
					sCaptionStyle = lpChild.m_id ;
				}
				else if ( !mapParts.isEmpty( lpChild.m_cmd ) )
				{
					const int	iParts = mapParts[lpChild.m_cmd] ;
					aLyParts[iParts] = child ;
					aPartsIDs[iParts] = lpChild.m_cmd ;
					aPartsImageIDs[iParts] = lpChild.m_id ;
					if ( lpChild.m_opt != null )
					{
						aPartsRefImageIDs[iParts] = lpChild.m_opt ;
					}
					aRctParts[iParts] = child.layerSizeOf() ;
					if ( aRctParts[iParts] != null )
					{
						if ( rectParts == null )
						{
							rectParts = aRctParts[iParts].clone() ;
						}
						else
						{
							rectParts = rectParts.or( aRctParts[iParts] ) ;
						}
					}
				}
			}
		}
		if ( lyImage == null )
		{
			outputError( "フレーム画像が指定されていません" ) ;
			return ;
		}
		if ( rectParts == null )
		{
			outputError( "フレーム画像パーツが指定されていません" ) ;
			return ;
		}
		// 画像集合作成
		PSDConverter.FormStyle	style = new PSDConverter.FormStyle() ;
		PSDConverter.ImageSet	imgset = new PSDConverter.ImageSet() ;
		if ( lyBackImage != null )
		{
			PSDConverter.LayerParam	lpBackFrame =
						parseLayerParam( lyBackImage.getName(), ctxSub ) ;
			String	idImage = lpBackFrame.m_id ;
			String	idRefImage = lpBackFrame.m_opt ;
			Rect	rectImage = null ;
			if ( idRefImage == null )
			{
				rectImage = saveLayerImage( lyBackImage, idImage, ctxSub ) ;
				if ( rectImage != null )
				{
					registerRect( idImage, rectImage ) ;
				}
			}
			else
			{
				rectImage = m_mapRects.get( idRefImage ) ;
				if ( rectImage == null )
				{
					outputError( "\'" + idRefImage + "\' は未定義の画像ＩＤです" ) ;
				}
				idImage = idRefImage ;
			}
			if ( rectImage != null )
			{
				style.m_tags.setAttrIntegerAs
					( "back_margin_left", rectImage.x - rectParts.x ) ;
				style.m_tags.setAttrIntegerAs
					( "back_margin_top", rectImage.y - rectParts.y ) ;
				style.m_tags.setAttrIntegerAs
					( "back_margin_right", (rectParts.x + rectParts.w)
												- (rectImage.x + rectImage.w) ) ;
				style.m_tags.setAttrIntegerAs
					( "back_margin_bottom", (rectParts.y + rectParts.h)
												- (rectImage.y + rectImage.h) ) ;
			}
			imgset.m_idRefs.add( idImage ) ;
			imgset.m_idFrames.add( "back_frame") ;
		}
		for ( int iParts = 0; iParts < aLyParts.length(); iParts ++ )
		{
			if ( aPartsImageIDs[iParts] != null )
			{
				String	sRefImageID = aPartsRefImageIDs[iParts] ;
				if ( aPartsRefImageIDs[iParts] == null )
				{
					ctxSub.m_clip = aRctParts[iParts] ;
					sRefImageID = aPartsImageIDs[iParts] ;
					saveLayerImage( lyImage, sRefImageID, ctxSub ) ;
				}
				imgset.m_idRefs.add( sRefImageID ) ;
				imgset.m_idFrames.add( aPartsIDs[iParts] ) ;
			}
		}
		m_mapImageSets.put( idFrameStyle, imgset ) ;
		//
		if ( rectInner != null )
		{
			style.m_tags.setAttrIntegerAs
				( "inner_margin_left", rectInner.x - rectParts.x ) ;
			style.m_tags.setAttrIntegerAs
				( "inner_margin_top", rectInner.y - rectParts.y ) ;
			style.m_tags.setAttrIntegerAs
				( "inner_margin_right", (rectParts.x + rectParts.w)
											- (rectInner.x + rectInner.w) ) ;
			style.m_tags.setAttrIntegerAs
				( "inner_margin_bottom", (rectParts.y + rectParts.h)
											- (rectInner.y + rectInner.h) ) ;
		}
		if ( rectCaption != null )
		{
			style.m_tags.setAttrIntegerAs
				( "caption_left", rectCaption.x - rectParts.x ) ;
			style.m_tags.setAttrIntegerAs
				( "caption_top", rectCaption.y - rectParts.y ) ;
			style.m_tags.setAttrIntegerAs
				( "caption_right", (rectParts.x + rectParts.w)
									- (rectCaption.x + rectCaption.w) ) ;
			//
			if ( sCaptionStyle != null )
			{
				PSDConverter.FormStyle	styleText = m_mapStyles.get( sCaptionStyle ) ;
				if ( styleText == null )
				{
					outputError( "\'" + sCaptionStyle + "\' は未定義のスタイルです" ) ;
				}
				else
				{
					for ( int i = 0; i < styleText.m_tags.getElementsCount(); i ++ )
					{
						style.m_tags.addElement
							( new XMLDocument( styleText.m_tags.getElementAt(i) ) ) ;
					}
				}
			}
		}
		style.m_tags.setAttributeAs( "imgset", idFrameStyle ) ;
		m_mapStyles.put( idFrameStyle, style ) ;
	}

	// @xframe
	public void prosessFrameItem
		( ImageComposition.Layer layer,
			PSDConverter.LayerParam	lp, Context context )
	{
		if ( lp.m_opt == null )
		{
			outputError( "フレームスタイルが指定されていません" ) ;
			return ;
		}
		PSDConverter.FormStyle
				style = m_mapStyles.get( lp.m_opt ) ;
		if ( style == null )
		{
			outputError( "フレームスタイル \'" + lp.m_opt + "\' は未定義です" ) ;
			return ;
		}
		Rect	rect = layer.layerSizeOf() ;
		addFrameItem( lp.m_id, style, rect, context ) ;
	}

	// @iframe
	public void prosessInnerFrameItem
		( ImageComposition.Layer layer,
			PSDConverter.LayerParam	lp, Context context )
	{
		if ( lp.m_opt == null )
		{
			outputError( "フレームスタイルが指定されていません" ) ;
			return ;
		}
		PSDConverter.FormStyle
				style = m_mapStyles.get( lp.m_opt ) ;
		if ( style == null )
		{
			outputError( "フレームスタイル \'" + lp.m_opt + "\' は未定義です" ) ;
			return ;
		}
		Rect	rect = layer.layerSizeOf() ;
		if ( rect == null )
		{
			outputError( "レイヤー領域を取得できません" ) ;
			return ;
		}
		int	xMarginLeft = style.m_tags.getAttrIntegerAs( "inner_margin_left", 0 ) ;
		int	yMarginTop = style.m_tags.getAttrIntegerAs( "inner_margin_top", 0 ) ;
		int	xMarginRight = style.m_tags.getAttrIntegerAs( "inner_margin_right", 0 ) ;
		int	yMarginBottom = style.m_tags.getAttrIntegerAs( "inner_margin_bottom", 0 ) ;
		rect.x -= xMarginLeft ;
		rect.y -= yMarginTop ;
		rect.w += xMarginLeft + xMarginRight ;
		rect.h += yMarginTop + yMarginBottom ;
		//
		addFrameItem( lp.m_id, style, rect, context ) ;
	}

	// @alpha
	public void prosessAlphaLayer
		( ImageComposition.Layer layer,
			PSDConverter.LayerParam	lp, Context context )
	{
		String	idImage = lp.m_id ;
		if ( lp.m_opt != null )
		{
			idImage = lp.m_opt ;
		}
		Rect	rectImage = saveLayerImage( layer, idImage, context ) ;
		if ( rectImage != null )
		{
			registerRect( lp.m_id, rectImage ) ;
			//
			if ( context.m_form != null )
			{
				context.m_form.m_idAlpha = lp.m_id ;
				context.m_form.m_rctAlpha = rectImage ;
			}
		}
	}

	// @hbar | @vbar
	public void prosessGaugeBarLayer
		( ImageComposition.Layer layer,
			PSDConverter.LayerParam	lp, Context context )
	{
		Rect	rectImage = null ;
		if ( lp.m_opt != null )
		{
			rectImage = getLayerRect( layer ) ;
		}
		else
		{
			rectImage = saveLayerImage( layer, lp.m_id, context ) ;
			lp.m_opt = lp.m_id ;
		}
		boolean	fInverse = false ;
		if ( (lp.m_params != null)
			&& (lp.m_params.get("inv") != null) )
		{
			fInverse = ((int) lp.m_params.get("inv")) != 0 ;
		}
		if ( rectImage != null )
		{
			registerRect( lp.m_id, rectImage ) ;
			addGaugeBarItem
				( lp.m_id, lp.m_opt, rectImage,
					(lp.m_cmd == "vbar"), fInverse, context ) ;
		}
	}

	// @htrack | @vtrack
	public void prosessTrackBarLayer
		( ImageComposition.Layer layer,
			PSDConverter.LayerParam	lp, Context context )
	{
		Rect	rectTrack = null ;
		String	idImgSet = lp.m_opt ;
		if ( lp.m_opt != null )
		{
			rectTrack = getLayerRect( layer ) ;
		}
		else
		{
			rectTrack = getChildLayerRect( layer, "@track" ) ;
			if ( rectTrack == null )
			{
				outputError( lp.m_cmd + " の子レイヤーに @track が見つかりません" ) ;
				return ;
			}
			idImgSet = prosessImageSet( new Rect(), layer, lp, context ) ;
		}
		if ( rectTrack != null )
		{
			registerRect( lp.m_id, rectTrack ) ;
			addTrackBarItem
				( lp.m_id, idImgSet, (lp.m_cmd == "vtrack"), rectTrack, context ) ;
		}
	}

	// @text | @msg | @rtxt
	public void prosessTextLayer
		( ImageComposition.Layer layer,
			PSDConverter.LayerParam	lp, Context context )
	{
		Rect	rectLayer = getLayerRect( layer ) ;
		boolean	fBuffered = false ;
		PSDConverter.FormStyle
				style = null ;
		if ( lp.m_opt != null )
		{
			style = m_mapStyles.get( lp.m_opt ) ;
			if ( style == null )
			{
				outputError( "スタイルは定義されていません \'" + lp.m_opt + "\'" ) ;
				return ;
			}
			style = parseTextStyleRect( style.clone(), rectLayer ) ;
		}
		else if ( lp.m_cmd == "text" )
		{
			style = parseTextStyle( lp, rectLayer ) ;
		}
		else
		{
			style = parseMessageStyle( lp, rectLayer ) ;
		}
		if ( lp.m_cmd == "text" )
		{
			addTextItem( lp.m_id, style, rectLayer, context ) ;
		}
		else if ( lp.m_cmd == "msg" )
		{
			addMessageItem
				( lp.m_id, style, lp.m_params, rectLayer, context ) ;
		}
		else
		{
			addRichTextItem
				( lp.m_id, style, lp.m_params, rectLayer, context ) ;
		}
	}

	// @id
	public void prosessLayerGroup
		( ImageComposition.Layer layer, Context context )
	{
		const int	nCount = layer.getChildrenCount() ;
		for ( int i = 0; i < nCount; i ++ )
		{
			ImageComposition.Layer	child = layer.getChildAt( i ) ;
			prosessLayer( child, context ) ;
		}
	}

	// レイヤー名解釈
	public LayerParam parseLayerParam( String sLayerName, Context context )
	{
		PSDConverter.LayerParam	lp = new PSDConverter.LayerParam() ;

		StringParser	sparsLayer = new StringParser() ;
		sparsLayer.attachString( sLayerName ) ;
		if ( sparsLayer.hasToComeChar( "@" ) == '@' )
		{
			int[]	pType = new int[1] ;
			lp.m_cmd = sparsLayer.getToken( pType ) ;
			if ( pType[0] == StringParser.tokenNormal )
			{
				if ( sparsLayer.hasToComeChar( ":" ) == ':' )
				{
					if ( sparsLayer.hasToComeChar( "+" ) == '+' )
					{
						lp.m_id = context.m_id + sparsLayer.getToken() ;
					}
					else
					{
						sparsLayer.markIndex() ;
						lp.m_id = sparsLayer.getToken() ;
						if ( lp.m_id == "," )
						{
							lp.m_id = null ;
							sparsLayer.seekToMark() ;
						}
					}
					boolean	flagHasParams = false ;
					if ( sparsLayer.hasToComeChar( "," ) == ',' )
					{
						lp.m_opt = sparsLayer.getToken() ;
						if ( lp.m_opt == "," )
						{
							lp.m_opt = null ;
							flagHasParams = true ;
						}
						else
						{
							flagHasParams =
								(sparsLayer.hasToComeChar( "," ) == ',') ;
						}
					}
					if ( flagHasParams )
					{
						lp.m_params = new HashMap<String>() ;
						//
						while ( sparsLayer.passSpace() )
						{
							String	strName = sparsLayer.getToken( pType ) ;
							if ( pType[0] != StringParser.tokenNormal )
							{
								outputError( "レイヤー \'" + sLayerName 
									+ "\' : 処理できないレイヤーパラメータ名 : \'" + strName + "\'" ) ;
								break ;
							}
							if ( sparsLayer.hasToComeChar( "=" ) != '=' )
							{
								outputError( "レイヤー \'" + sLayerName 
									+ "\' : レイヤーパラメータが = で結ばれていません" ) ;
								break ;
							}
							String	strParam = sparsLayer.getEnclosedString( ',' ) ;
							//
							lp.m_params.put( strName, strParam ) ;
						}
						if ( sparsLayer.passSpace() )
						{
							outputError( "レイヤー \'" + sLayerName 
									+ "\' : レイヤーパラメータが , で区切られていません" ) ;
						}
					}
				}
			}
			else
			{
				lp.m_cmd = null ;
			}
		}
		return	lp ;
	}

	// テキストスタイル解釈
	public FormStyle parseTextStyleRect( FormStyle style, Rect rect )
	{
		XMLDocument	xmlArrange = style.m_tags.createElementTagAs( "arrange" ) ;
		xmlArrange.setAttrIntegerAs( "left", 0 ) ;
		xmlArrange.setAttrIntegerAs( "top", 0 ) ;
		xmlArrange.setAttrIntegerAs( "width", rect.w ) ;
		xmlArrange.setAttrIntegerAs( "height", rect.h ) ;
		return	style ;
	}

	public FormStyle parseTextStyle( LayerParam lp, Rect rect )
	{
		PSDConverter.FormStyle
					style = new PSDConverter.FormStyle() ;
		int			nFontSize = 24 ;
		int			nLineHeight = 32 ;
		String		strFace = "Default" ;
		int			rgbText = 0xFFFFFF ;
		int			rgbBorder = 0, nBorder = 0 ;
		if ( lp.m_params != null )
		{
			if ( lp.m_params.get("size") != null )
			{
				nFontSize = (int) lp.m_params.get("size") ;
				nLineHeight = nFontSize * 4 / 3 ;
			}
			if ( lp.m_params.get("line") != null )
			{
				nLineHeight = (int) lp.m_params.get("line") ;
			}
			if ( lp.m_params.get("font") != null )
			{
				strFace = lp.m_params.get("font") ;
			}
			if ( lp.m_params.get("color") != null )
			{
				rgbText = lp.m_params["color"].parseInt(16) ;
			}
			if ( lp.m_params.get("border") != null )
			{
				rgbBorder = lp.m_params["border"].parseInt(16) ;
				nBorder = 1 ;
			}
		}
		XMLDocument	xmlArrange = style.m_tags.createElementTagAs( "arrange" ) ;
		XMLDocument	xmlFont = style.m_tags.createElementTagAs( "font" ) ;
		XMLDocument	xmlText = style.m_tags.createElementTagAs( "text" ) ;
		xmlArrange.setAttrIntegerAs( "line_height", nLineHeight ) ;
		xmlFont.setAttributeAs( "face", strFace ) ;
		xmlFont.setAttrIntegerAs( "size", nFontSize ) ;
		xmlText.setAttributeAs( "color", String.format( "0x%06x", rgbText ) ) ;
		if ( nBorder != 0 )
		{
			XMLDocument	xmlBorder = style.m_tags.createElementTagAs( "border" ) ;
			xmlBorder.setAttributeAs( "color", String.format( "0x%06x", rgbBorder ) ) ;
			xmlBorder.setAttributeAs( "transparency", "0" ) ;
		}
		return	parseTextStyleRect( style, rect ) ;
	}

	public FormStyle parseMessageStyle( LayerParam lp, Rect rect )
	{
		PSDConverter.FormStyle	style = parseTextStyle( lp, rect ) ;
		//
		XMLDocument	xmlFont = style.m_tags.getElementTagAs( "font" ) ;
		if ( xmlFont != null )
		{
			String	strFont = xmlFont.setAttributeAs( "face", "Default" ) ;
			int		nFontSize = xmlFont.getAttrIntegerAs( "size", 24 ) ;
			//
			XMLDocument	xmlRuby = style.m_tags.createElementTagAs( "ruby" ) ;
			xmlRuby.setAttributeAs( "face", strFont ) ;
			xmlRuby.setAttrIntegerAs( "size", nFontSize / 3 ) ;
		}
		return	style;
	}

	// 画像保存
	public Rect saveLayerImage
		( ImageComposition.Layer layer, String id, Context context )
	{
		if ( id == null )
		{
			return	null ;
		}
		if ( m_mapImages.get( id ) != null )
		{
			outputError( "画像ＩＤが重複しています \'" + id + "\'" ) ;
			return	null ;
		}
		Rect	rectClip = context.m_clip ;
		if ( rectClip == null )
		{
			rectClip = getLayerRect( layer ) ;
		}
		Point	ptLayer = layer.getPosition() ;
		Image	img = new Image() ;
		img.createImage( rectClip.w, rectClip.h, Image.formatARGB, 32 ) ;
		img.convertImage( layer, ptLayer.x - rectClip.x, ptLayer.y - rectClip.y ) ;
		//
		String	sDstImageFile = m_sDstDir.offsetFilePath( id + ".eri" ) ;
		if ( !img.saveImage( sDstImageFile ) )
		{
			outputError( "画像の書き出しに失敗しました \'" + sDstImageFile + "\'" ) ;
			return	null ;
		}
		con.printf( "image resource:%s\n", id ) ;
		m_mapImages.put( id, m_sRsrcDir.offsetFilePath( id + ".eri" ) ) ;
		//
		return	rectClip ;
	}

	// アニメーション画像保存
	public Rect saveLayerGroupAnimeImage
		( ImageComposition.Layer layer, String id, LayerParam lp )
	{
		if ( id == null )
		{
			return	null ;
		}
		if ( m_mapImages.get( id ) != null )
		{
			outputError( "画像ＩＤが重複しています \'" + id + "\'" ) ;
			return	null ;
		}
		// 矩形
		Rect		rectGroup = null ;
		const int	nFrames = layer.getChildrenCount() ;
		for ( int i = 0; i < nFrames; i ++ )
		{
			ImageComposition.Layer	child = layer.getChildAt( i ) ;
			Rect	rectLayer = getLayerRect( child ) ;
			if ( (rectLayer == null) || rectLayer.isEmpty() )
			{
				continue ;
			}
			if ( rectGroup != null )
			{
				rectGroup = rectGroup.or( rectLayer ) ;
			}
			else
			{
				rectGroup = rectLayer ;
			}
		}
		// アニメーション作成
		long	msecLong = nFrames * 33 ;
		if ( lp.m_params != null )
		{
			if ( lp.m_params.get("dr") != null )
			{
				msecLong = (int) lp.m_params["dr"] ;
			}
			else if ( lp.m_params.get("fps") != null )
			{
				msecLong = nFrames * (int) lp.m_params["fps"] ;
			}
		}
		Image	img = new Image() ;
		img.createImage
			( rectGroup.w, rectGroup.h,
				Image.formatARGB, 32, 0, nFrames, msecLong ) ;
		for ( int i = 0; i < nFrames; i ++ )
		{
			ImageComposition.Layer	child = layer.getChildAt( i ) ;
			Rect	rectLayer = getLayerRect( child ) ;
			if ( (rectLayer == null) || rectLayer.isEmpty() )
			{
				continue ;
			}
			Point	ptLayer = child.getPosition() ;
			img.selectFrame( i ) ;
			img.convertImage
				( child, ptLayer.x - rectGroup.x, ptLayer.y - rectGroup.y ) ;
		}
		img.selectFrame( 0 ) ;
		//
		// 保存
		String	sDstImageFile = m_sDstDir.offsetFilePath( id + ".eri" ) ;
		if ( !img.saveImage( sDstImageFile ) )
		{
			outputError( "画像の書き出しに失敗しました \'" + sDstImageFile + "\'" ) ;
			return	null ;
		}
		con.printf( "image resource:%s\n", id ) ;
		m_mapImages.put( id, m_sRsrcDir.offsetFilePath( id + ".eri" ) ) ;
		//
		return	rectGroup ;
	}

	// レイヤー矩形
	public const Rect getLayerRect( ImageComposition.Layer layer )
	{
		Rect	rectLayer = layer.layerSizeOf() ;
		if ( (rectLayer != null) && !rectLayer.isEmpty() )
		{
			rectLayer.x -= m_nClipMargin ;
			rectLayer.y -= m_nClipMargin ;
			rectLayer.w += m_nClipMargin * 2 ;
			rectLayer.h += m_nClipMargin * 2 ;
		}
		return	rectLayer ;
	}

	public Rect getChildLayerRect
				( ImageComposition.Layer layer, String name )
	{
		const int	nCount = layer.getChildrenCount() ;
		for ( int i = 0; i < nCount; i ++ )
		{
			ImageComposition.Layer	child = layer.getChildAt( i ) ;
			if ( child.getName() == name )
			{
				return	getLayerRect( child ) ;
			}
		}
		return	null ;
	}

	public Rect getLayerGroupRect
				( ImageComposition.Layer layer, Context context )
	{
		Rect		rectGroup = null ;
		const int	nCount = layer.getChildrenCount() ;
		for ( int i = 0; i < nCount; i ++ )
		{
			ImageComposition.Layer	child = layer.getChildAt( i ) ;
			PSDConverter.LayerParam
					lp = parseLayerParam( child.getName(), context ) ;
			if ( lp.m_cmd == "track" )
			{
				continue ;
			}
			Rect	rectLayer = getLayerRect( child ) ;
			if ( (lp.m_cmd != null) && (lp.m_cmd == "mask") )
			{
				return	rectLayer ;
			}
			if ( (rectLayer == null) || rectLayer.isEmpty() )
			{
				continue ;
			}
			if ( rectGroup != null )
			{
				rectGroup = rectGroup.or( rectLayer ) ;
			}
			else
			{
				rectGroup = rectLayer ;
			}
		}
		return	rectGroup ;
	}

	// 矩形登録
	public void registerRect( String id, Rect rect )
	{
		if ( id == null )
		{
			return ;
		}
		if ( m_mapRects.get(id) != null )
		{
			outputError( "矩形ＩＤが重複しています \'" + id + "\'" ) ;
			return ;
		}
		m_mapRects[id] = rect ;
	}

	// 画像アイテム追加
	public void addImageItem( String id, String img, Rect rect, Context context )
	{
		if ( context.m_form != null )
		{
			if ( m_mapImages.get(img) == null )
			{
				outputError( "未定義の画像ＩＤです \'" + img + "\'" ) ;
			}
			PSDConverter.FormItem	item = new PSDConverter.FormItem() ;
			item.m_id = id ;
			item.m_type = "image" ;
			item.m_rect = rect.clone() ;
			item.m_params = new HashMap<String>() ;
			item.m_params.put( "image", img ) ;
			//
			context.m_form.m_items.add( item ) ;
			//
			con.printf( "image item:%s=%s\n", id, img ) ;
		}
	}

	// 画像セレクタアイテム追加
	public void addImageSelItem
		( String id, String imgset,
			HashMap<String> params, Rect rect, Context context )
	{
		if ( context.m_form != null )
		{
			if ( m_mapImageSets.get(imgset) == null )
			{
				outputError( "未定義の画像セットＩＤです \'" + imgset + "\'" ) ;
			}
			PSDConverter.FormItem	item = new PSDConverter.FormItem() ;
			item.m_id = id ;
			item.m_type = "imgsel" ;
			item.m_rect = rect.clone() ;
			item.m_params = new HashMap<String>() ;
			item.m_params.put( "imgset", imgset ) ;
			//
			if ( (params != null)
				&& (params.get( "track" ) != null) )
			{
				item.m_params.put( "track_bar", params.get( "track" )  ) ;
			}
			//
			context.m_form.m_items.add( item ) ;
			//
			con.printf( "imgsel item:%s=%s\n", id, imgset ) ;
		}
	}

	// ボタンアイテム追加
	public void addButtonItem
		( String id, String imgset, boolean toggle, Rect rect, Context context )
	{
		if ( context.m_form != null )
		{
			if ( m_mapImageSets.get(imgset) == null )
			{
				outputError( "未定義の画像セットＩＤです \'" + imgset + "\'" ) ;
			}
			PSDConverter.FormItem	item = new PSDConverter.FormItem() ;
			item.m_id = id ;
			item.m_type = toggle ? "check" : "button" ;
			item.m_rect = rect.clone() ;
			item.m_params = new HashMap<String>() ;
			item.m_params.put( "imgset", imgset ) ;
			//
			context.m_form.m_items.add( item ) ;
			//
			con.printf( "imgsel item:%s=%s\n", id, imgset ) ;
		}
	}

	// 水平・垂直バーアイテム追加
	public void addGaugeBarItem
		( String id, String img, Rect rect,
			boolean fVert, boolean fInverse, Context context )
	{
		if ( context.m_form != null )
		{
			if ( m_mapImages.get(img) == null )
			{
				outputError( "未定義の画像ＩＤです \'" + img + "\'" ) ;
			}
			PSDConverter.FormItem	item = new PSDConverter.FormItem() ;
			item.m_id = id ;
			item.m_type = "gauge_bar" ;
			item.m_rect = rect.clone() ;
			item.m_params = new HashMap<String>() ;
			item.m_params.put( "image", img ) ;
			item.m_params.put( "vertical", fVert ? "1" : "0" ) ;
			item.m_params.put( "inverse", fInverse ? "1" : "0" ) ;
			//
			context.m_form.m_items.add( item ) ;
			//
			con.printf( "gauge_bar item:%s=%s\n", id, img ) ;
		}
	}

	// 水平・垂直トラックバーアイテム追加
	public void addTrackBarItem
		( String id, String imgset, boolean fVert, Rect rect, Context context )
	{
		if ( context.m_form != null )
		{
			if ( m_mapImageSets.get(imgset) == null )
			{
				outputError( "未定義の画像セットＩＤです \'" + imgset + "\'" ) ;
			}
			PSDConverter.FormItem	item = new PSDConverter.FormItem() ;
			item.m_id = id ;
			item.m_type = "track_bar" ;
			item.m_rect = rect.clone() ;
			item.m_params = new HashMap<String>() ;
			item.m_params.put( "imgset", imgset ) ;
			item.m_params.put( "vertical", fVert ? "1" : "0" ) ;
			//
			context.m_form.m_items.add( item ) ;
			//
			con.printf( "track_bar item:%s=%s\n", id, imgset ) ;
		}
	}

	// テキストアイテム追加
	public void addTextItem
		( String id, FormStyle style, Rect rect, Context context )
	{
		if ( context.m_form != null )
		{
			PSDConverter.FormItem	item = new PSDConverter.FormItem() ;
			item.m_id = id ;
			item.m_type = "text" ;
			item.m_rect = rect.clone() ;
			item.m_tags = style.m_tags ;
			//
			context.m_form.m_items.add( item ) ;
			//
			con.printf( "text item:%s\n", id ) ;
		}
		m_mapStyles.put( id, style ) ;
	}

	// リッチアイテム（汎用）追加
	public void addGenRichTextItem
		( String id, String type, HashMap<String> params,
			FormStyle style, Rect rect, Context context )
	{
		if ( context.m_form != null )
		{
			PSDConverter.FormItem	item = new PSDConverter.FormItem() ;
			item.m_id = id ;
			item.m_type = type ;
			item.m_rect = rect.clone() ;
			item.m_tags = style.m_tags ;
			item.m_params = new HashMap<String>() ;
			//
			if ( (params != null) && (params.get("buf") != null) )
			{
				item.m_params.put( "buffered", params.get("buf") ) ;
			}
			if ( (params != null) && (params.get("scroll") != null) )
			{
				item.m_params.put( "scroll_bar", params.get("scroll") ) ;
			}
			//
			context.m_form.m_items.add( item ) ;
			//
			con.printf( "%s item:%s\n", type, id ) ;
		}
		m_mapStyles.put( id, style ) ;
	}

	// メッセージアイテム追加
	public void addMessageItem
		( String id, FormStyle style,
			HashMap<String> params, Rect rect, Context context )
	{
		addGenRichTextItem
			( id, "message", params, style, rect, context ) ;
	}

	// リッチアイテム追加
	public void addRichTextItem
		( String id, FormStyle style,
			HashMap<String> params, Rect rect, Context context )
	{
		addGenRichTextItem
			( id, "rich_text", params, style, rect, context ) ;
	}

	// フレームアイテム追加
	public void addFrameItem
		( String id, FormStyle style, Rect rect, Context context )
	{
		String[]	aParamNames =
		[
			"inner_margin_left", "inner_margin_top",
			"inner_margin_right", "inner_margin_bottom",
			"back_margin_left", "back_margin_top",
			"back_margin_right", "back_margin_bottom",
		] ;
		PSDConverter.FormItem	item = new PSDConverter.FormItem() ;
		item.m_id = id ;
		item.m_type = "stretch_frame" ;
		item.m_rect = rect.clone() ;
		item.m_params = new HashMap<String>() ;
		item.m_params.put
			( "imgset", style.m_tags.getAttrStringAs( "imgset" ) ) ;
		for ( int i = 0; i < aParamNames.length(); i ++ )
		{
			item.m_params.put
				( aParamNames[i],
					style.m_tags.getAttrStringAs( aParamNames[i], "0" ) ) ;
		}
		item.m_tags = style.m_tags ;
		//
		context.m_form.m_items.add( item ) ;
		//
		con.printf( "frame item:%s\n", id ) ;
	}

	// フォームアイテム追加
	public void addFormItem
		( String id, String form,
			HashMap<String> params, Rect rect, Context context )
	{
		if ( context.m_form != null )
		{
			if ( m_mapForms.get(form) == null )
			{
				outputError( "未定義のフォームＩＤです \'" + form + "\'" ) ;
			}
			PSDConverter.FormData	formData = m_mapForms.get(form) ;
			PSDConverter.FormItem	item = new PSDConverter.FormItem() ;
			item.m_id = id ;
			item.m_type = "form" ;
			item.m_rect = rect.clone() ;
			item.m_params = new HashMap<String>() ;
			item.m_params.put( "form", form ) ;
			//
			if ( formData.m_idAlpha != null )
			{
				item.m_params.put( "alpha", formData.m_idAlpha ) ;
				//
				if ( params != null )
				{
					if ( params.get("sdir") != null )
					{
						boolean	fHorz = (params.get("sdir") == "horz") ;
						item.m_params.put( "scrollable", params.get("sdir") ) ;
						//
						int	nFormMargin = 0 ;
						if ( params.get("margin") != null )
						{
							nFormMargin = (int) params.get("margin") ;
						}
						if ( fHorz )
						{
							if ( nFormMargin == 0 )
							{
								nFormMargin = rect.x - formData.m_rctAlpha.x ;
							}
							item.m_params.put
								( "margin_top",
									(String) (rect.y - formData.m_rctAlpha.y) ) ;
							item.m_params.put
								( "margin_left", (String) nFormMargin ) ;
							item.m_params.put
								( "margin_right", (String) nFormMargin ) ;
						}
						else
						{
							if ( nFormMargin == 0 )
							{
								nFormMargin = rect.y - formData.m_rctAlpha.y ;
							}
							item.m_params.put
								( "margin_left",
									(String) (rect.x - formData.m_rctAlpha.x) ) ;
							item.m_params.put
								( "margin_top", (String) nFormMargin ) ;
							item.m_params.put
								( "margin_bottom", (String) nFormMargin ) ;
						}
					}
					if ( params.get("sbar") != null )
					{
						item.m_params.put( "scroll_bar", params.get("sbar") ) ;
					}
				}
			}
			//
			context.m_form.m_items.add( item ) ;
			//
			con.printf( "form item:%s=%s\n", id, form ) ;
		}
	}

	// フォーム矩形を取得しアイテム座標を修正
	public void normalizeFormRect( FormData form )
	{
		form.m_rect = null ;
		for ( int i = 0; i < form.m_items.length(); i ++ )
		{
			PSDConverter.FormItem	item = form.m_items[i] ;
			if ( (item != null) && (item.m_rect != null) )
			{
				if ( form.m_rect != null )
				{
					form.m_rect = form.m_rect.or( item.m_rect ) ;
				}
				else
				{
					form.m_rect = item.m_rect.clone() ;
				}
			}
		}
		if ( form.m_rect != null )
		{
			for ( int i = 0; i < form.m_items.length(); i ++ )
			{
				PSDConverter.FormItem	item = form.m_items[i] ;
				if ( (item != null) && (item.m_rect != null) )
				{
					item.m_rect.x -= form.m_rect.x ;
					item.m_rect.y -= form.m_rect.y ;
				}
			}
		}
		else
		{
			form.m_rect = new Rect() ;
		}
	}

	// フォームデータを出力
	public void saveFormData( String sFilePath )
	{
		XMLDocument	xmlDoc = formatFormData() ;
		if ( !xmlDoc.saveDocument( sFilePath ) )
		{
			outputError( "\'" + sFilePath + "\' への書き出しに失敗しました" ) ;
		}
	}

	public XMLDocument formatFormData()
	{
		XMLDocument	xmlDoc = new XMLDocument() ;
		xmlDoc.setTag( "gls4_basic_form" ) ;
		//
		// 矩形情報
		//
		XMLDocument	xmlRects = new XMLDocument() ;
		xmlRects.setTag( "rectangles" ) ;
		xmlDoc.addElement( xmlRects ) ;
		//
		for ( int i = 0; i < m_mapRects.size(); i ++ )
		{
			String	id = m_mapRects.keyAt(i) ;
			Rect	rect = m_mapRects[i] ;
			//
			XMLDocument	xmlRect = new XMLDocument() ;
			xmlRect.setTag( "rect" ) ;
			xmlRect.setAttributeAs( "id", id ) ;
			xmlRect.setAttrIntegerAs( "x", rect.x ) ;
			xmlRect.setAttrIntegerAs( "y", rect.y ) ;
			xmlRect.setAttrIntegerAs( "width", rect.w ) ;
			xmlRect.setAttrIntegerAs( "height", rect.h ) ;
			//
			xmlRects.addElement( xmlRect ) ;
		}
		//
		// リソース情報
		//
		XMLDocument	xmlResources = new XMLDocument() ;
		xmlResources.setTag( "resources" ) ;
		xmlDoc.addElement( xmlResources ) ;
		//
		for ( int i = 0; i < m_mapImages.size(); i ++ )
		{
			String	id = m_mapImages.keyAt(i) ;
			String	file = m_mapImages.get(id) ;
			//
			XMLDocument	xmlRsrc = new XMLDocument() ;
			xmlRsrc.setTag( "image" ) ;
			xmlRsrc.setAttributeAs( "id", id ) ;
			xmlRsrc.setAttributeAs( "src", file ) ;
			//
			xmlResources.addElement( xmlRsrc ) ;
		}
		//
		// 画像セット情報
		//
		XMLDocument	xmlImageSets = new XMLDocument() ;
		xmlImageSets.setTag( "image_sets" ) ;
		xmlDoc.addElement( xmlImageSets ) ;
		//
		for ( int i = 0; i < m_mapImageSets.size(); i ++ )
		{
			String	id = m_mapImageSets.keyAt(i) ;
			PSDConverter.ImageSet
					imgset = m_mapImageSets.get(id) ;
			//
			XMLDocument	xmlImageSet = new XMLDocument() ;
			xmlImageSet.setTag( "image_set" ) ;
			xmlImageSet.setAttributeAs( "id", id ) ;
			//
			for ( int j = 0; j < imgset.m_idRefs.length(); j ++ )
			{
				XMLDocument	xmlImage = new XMLDocument() ;
				xmlImage.setTag( "image" ) ;
				xmlImage.setAttributeAs( "ref_id", imgset.m_idRefs[j] ) ;
				//
				if ( (j < imgset.m_idFrames.length())
					&& (imgset.m_idFrames[j] != null) )
				{
					xmlImage.setAttributeAs( "frame_id", imgset.m_idFrames[j] ) ;
				}
				xmlImageSet.addElement( xmlImage ) ;
			}
			//
			xmlImageSets.addElement( xmlImageSet ) ;
		}
		//
		// スタイル情報
		//
		XMLDocument	xmlStyles = new XMLDocument() ;
		xmlStyles.setTag( "styles" ) ;
		xmlDoc.addElement( xmlStyles ) ;
		//
		for ( int i = 0; i < m_mapStyles.size(); i ++ )
		{
			String	id = m_mapStyles.keyAt(i) ;
			PSDConverter.FormStyle
					style = m_mapStyles.get(id) ;
			XMLDocument	xmlTag = new XMLDocument( style.m_tags ) ;
			if ( (xmlTag.getTag() == null)
				|| (xmlTag.getTag() == "") )
			{
				xmlTag.setTag( "style" ) ;
			}
			if ( xmlTag.getAttributeAs( "id" ) == null )
			{
				xmlTag.setAttributeAs( "id", id ) ;
			}
			xmlStyles.addElement( xmlTag) ;
		}
		//
		// フォーム情報
		//
		XMLDocument	xmlForms = new XMLDocument() ;
		xmlForms.setTag( "forms" ) ;
		xmlDoc.addElement( xmlForms ) ;
		//
		for ( int i = 0; i < m_mapForms.size(); i ++ )
		{
			String	id = m_mapForms.keyAt(i) ;
			PSDConverter.FormData
					form = m_mapForms.get(id) ;
			//
			xmlForms.addElement( formatForm( id, form ) ) ;
		}
		return	xmlDoc ;
	}

	public XMLDocument formatForm( String id, FormData form )
	{
		XMLDocument	xmlForm = new XMLDocument() ;
		xmlForm.setTag( "form" ) ;
		xmlForm.setAttributeAs( "id", id ) ;
		//
		if ( form.m_rect != null )
		{
			xmlForm.setAttrIntegerAs( "x", form.m_rect.x ) ;
			xmlForm.setAttrIntegerAs( "y", form.m_rect.y ) ;
			xmlForm.setAttrIntegerAs( "width", form.m_rect.w ) ;
			xmlForm.setAttrIntegerAs( "height", form.m_rect.h ) ;
		}
		//
		for ( int i = 0; i < form.m_items.length(); i ++ )
		{
			PSDConverter.FormItem	item = form.m_items[i] ;
			//
			XMLDocument	xmlItem = new XMLDocument() ;
			xmlItem.setTag( item.m_type ) ;
			//
			if ( item.m_id != null )
			{
				xmlItem.setAttributeAs( "id", item.m_id ) ;
			}
			if ( item.m_rect != null )
			{
				xmlItem.setAttrIntegerAs( "x", item.m_rect.x ) ;
				xmlItem.setAttrIntegerAs( "y", item.m_rect.y ) ;
				xmlItem.setAttrIntegerAs( "width", item.m_rect.w ) ;
				xmlItem.setAttrIntegerAs( "height", item.m_rect.h ) ;
			}
			if ( item.m_params != null )
			{
				for ( int j = 0; j < item.m_params.size(); j ++ )
				{
					xmlItem.setAttributeAs
						( item.m_params.keyAt(j), item.m_params[j] ) ;
				}
			}
			if ( item.m_tags != null )
			{
				for ( int j = 0; j < item.m_tags.getElementsCount(); j ++ )
				{
					xmlItem.addElement
						( new XMLDocument( item.m_tags.getElementAt(j) ) ) ;
				}
			}
			//
			xmlForm.addElement( xmlItem ) ;
		}
		//
		return	xmlForm ;
	}

	// コンポジションファイルを書き出す
	public boolean saveComposition( String sFilePath )
	{
		XMLDocument	xmlDoc = formatComposition() ;
		if ( !xmlDoc.saveDocument( sFilePath ) )
		{
			outputError( "\'" + sFilePath + "\' への書き出しに失敗しました" ) ;
		}
	}

	public XMLDocument formatComposition()
	{
		XMLDocument	xmlDoc = new XMLDocument() ;
		xmlDoc.setTag( "scene" ) ;
		//
		XMLDocument	xmlAssets = new XMLDocument() ;
		xmlAssets.setTag( "assets" ) ;
		xmlDoc.addElement( xmlAssets ) ;
		//
		for ( int i = 0; i < m_mapImages.size(); i ++ )
		{
			String	id = m_mapImages.keyAt(i) ;
			String	file = m_mapImages.get(id) ;
			//
			XMLDocument	xmlRsrc = new XMLDocument() ;
			xmlRsrc.setTag( "image" ) ;
			xmlRsrc.setAttributeAs( "id", id ) ;
			xmlRsrc.setAttributeAs( "src", file ) ;
			//
			xmlAssets.addElement( xmlRsrc ) ;
		}
		//
		XMLDocument	xmlAtlas = new XMLDocument() ;
		xmlAtlas.setTag( "image" ) ;
		xmlAtlas.setAttributeAs
			( "id", "atlas_image_" + m_sFormXml.getFileTitlePart() ) ;
		xmlAssets.addElement( xmlAtlas ) ;
		//
		XMLDocument	xmlOptions = new XMLDocument() ;
		xmlOptions.setTag( "options" ) ;
		xmlOptions.setAttributeAs( "proc_id", "atlas_texture" ) ;
		xmlAtlas.addElement( xmlOptions ) ;
		//
		xmlOptions.createElementTagAs("init_width").getAttrIntegerAs( "value", 128 ) ;
		xmlOptions.createElementTagAs("init_height").getAttrIntegerAs( "value", 128 ) ;
		xmlOptions.createElementTagAs("size_pot").setAttributeAs( "value", "false" ) ;
		xmlOptions.createElementTagAs("clamp_border").setAttributeAs( "value", "true" ) ;
		xmlOptions.createElementTagAs("ref_count").
							setAttrIntegerAs( "value", m_mapImages.size() ) ;
		for ( int i = 0; i < m_mapImages.size(); i ++ )
		{
			xmlOptions.createElementTagAs( "ref_image" + i ).
					setAttributeAs( "value", m_mapImages.keyAt(i) ) ;
		}
		//
		if ( m_sFormXml != null )
		{
			String	sProjDir = m_sCompProj.getFileDirectoryPart() ;
			XMLDocument	xmlRsrc = new XMLDocument() ;
			xmlRsrc.setTag( "basic_form" ) ;
			xmlRsrc.setAttributeAs( "id", m_sFormXml.getFileTitlePart() ) ;
			xmlRsrc.setAttributeAs( "src", sProjDir.relativeFilePath( m_sFormXml ) ) ;
			//
			xmlAssets.addElement( xmlRsrc ) ;
		}
		return	xmlDoc ;
	}

	// エラー
	public void outputError( String sErrMsg )
	{
		con.printf( "error:%s\n", sErrMsg ) ;
		m_nErrors ++ ;
	}

}




// エントリポイント
//////////////////////////////////////////////////////////////////////////////

int main( String[] arg )
{
	PSDConverter	converter = new PSDConverter() ;
	if ( !converter.parseCmdLine( arg ) )
	{
		return	1 ;
	}
	int	nErrors = converter.run() ;
	con.printf( "%d errors\n", nErrors ) ;
	return	nErrors ;
}

