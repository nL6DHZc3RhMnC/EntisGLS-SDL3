

;	ネイティブクラス
; -----------------------------------------------------------------------------

Class	Native Image	: Public Resource
Public
	Integer	index
	String	name
	String	filename
	Integer	blendmode
	Integer	transparency
	Point	position
	Size	size
Public
	Prototype	void Image( const Image& image )
	Prototype	const Image& operator := ( const Image& image )
	Prototype	Error set_position( integer x, integer y )
EndClass


Class	Native ImageContext	: Public Array
Public
	Integer	duration
	Size	size
	Point	hotspot
	String	filename
Public
	Constant	blendmodeNoBlend				:= 0,
				blendmodeBlendLayer				:= 1
	Constant	layermaskNoMultipleAlpha		:= 01H,
				layermaskMultipleAlphaChannel	:= 02H
	Constant	maskFlagReverse					:= 01H
	Constant	cutFlagKeepHotspot				:= 01H,
				cutFlagRemoveAlpha				:= 02H,
				cutFlagAutoRemoveAlpha			:= 04H
	Constant	resizeFlagOverSampling			:= 02H
	Constant	indexDithering					:= 01H,
				indexTransparency				:= 02H,
				indexNoTransparencyDithering	:= 04H
	Constant	mosaicAverage					:= 0,
				mosaicUpperLeft					:= 1,
				mosaicCenter					:= 2,
				mosaicRandom					:= 3,
				mosaicRandom4					:= 4,
				mosaicFigure					:= 100H
Public
	Prototype	void ImageContext( const ImageContext& ictx )
	Prototype	const ImageContext& operator := ( const ImageContext& ictx )
	Prototype	Image& operator [] ( Integer nIndex ) const
	Prototype	Error load(
					String filename,
					Integer blend_mode := 0, Integer laer_mask := 0,
					Integer first_layer := 0, Integer end_layer := -1 )
	Prototype	Error load_info(
					String filename,
					Integer& layer_count, Size& canvas_size )
	Prototype	Error save( String filename )
	Prototype	Error save(
					String filename, String mime_type, integer quality := -1 )
	Prototype	Error mask()
	Prototype	Error mask( ImageContext& icxMask )
	Prototype	Error mask( ImageContext& icxMask, Integer flags )
	Prototype	Error cut(
					Integer align := 0, Integer margin := 0,
					Integer threshold := 0, Integer op_flags := 5,
					Point& cut_offset := null, Size& cut_size := null )
	Prototype	Error arrange(
					Integer way_vert := 0,
					Integer put_right := 0, Integer align := 1 )
	Prototype	Error animation( String seq_list := "" )
	Prototype	Error merge(
					ImageContext& imgctx, Integer first := 0,
					Integer count := -1, String name := "" )
	Prototype	Error compose()
	Prototype	Error resize( Integer width, Integer height )
	Prototype	Error resize( Integer width, Integer height, Integer flags )
	Prototype	Error gray_scale()
	Prototype	Error unmultiplied_alpha()
	Prototype	Error index_color(
					Integer pixel_bits := 8,
					Integer coloes := 256,
					Integer flags := 3, Integer alpha_threshold := 127 )
	Prototype	Error tone_filter( const ToneFilter& filter )
	Prototype	Error make_mosaic_mask( Integer tile_size )
	Prototype	Error filter_mosaic(
					ImageContext& imgctx,
					Integer tile_size, Integer flags := 3 )
EndClass

Prototype	Error print( ... ) native
Prototype	Error output( ... ) native


