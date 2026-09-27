
#if	!defined(__SAKURAGL_PAINT_BUFFER_H__)
#define	__SAKURAGL_PAINT_BUFFER_H__

#include <sakuragl/sgl2d/sgl_image_conversion.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// 描画リージョン
	//////////////////////////////////////////////////////////////////////////

	struct	SGLRegionLine
	{
		int32_t			fxLeft ;		// 左座標固定小数点 (x10000H)
		int32_t			fxRight ;		// 右座標固定小数点 (x10000H)
		S3DColor		rgbaLeft ;		// 左頂点色
		S3DColor		rgbaRight ;		// 右頂点色
		/*
		S3DWVector4		vLeft ;			// 左法線
		S3DWVector4		vRight ;		// 右法線
		*/
	} ;
	struct	SGLRegion
	{
		int32_t			yTop ;
		int32_t			yBottom ;
		uint32_t		areaPixels ;
		uint32_t		nReserved ;
		SGLRegionLine	rgLine[1] ;
	} ;

	// 多角形リージョン生成
	bool sglCreatePolygonRegion
		( SGLRegion * pRegion,
			const SGLRect & rctClip,
			const S2DVector * pVertices, size_t nCount,
			const S3DColor * pColors = NULL,
			const S3DVector4 * pNormals = NULL ) ;
	// 矩形リージョン生成
	bool sglCreateRectangleRegion
		( SGLRegion * pRegion,
			const SGLRect & rctClip, const SGLRect & rctRegion ) ;
	// 直線リージョン生成
	enum	ThinLineRegionFlag
	{
		thinLineExcludeStart	= 0x01,
		thinLineExcludeEnd		= 0x02,
	} ;
	bool sglCreateThinLineRegion
		( SGLRegion * pRegion,
			const SGLRect & rctClip,
			const S2DVector& vStart, const S2DVector& vEnd, int nFlags = 0 ) ;
	// ベジェ曲線閉鎖領域（交差領域は除外）ビットマスク生成
	bool sglHatchBezierBitmask
		( SGLImageBuffer& imgbuf,
			const S2DVector * pVertices, size_t nCount ) ;
	// 多角形（交差領域は除外）ビットマスク生成
	bool sglFillPolygonBitmask
		( SGLImageBuffer& imgbuf,
			const S2DVector * pVertices, size_t nCount ) ;
	// 三角形ビットマスク反転描画
	bool sglFillXorTriangleBitmask
		( SGLImageBuffer& imgbuf, const S2DVector * pVertices ) ;


	//////////////////////////////////////////////////////////////////////////
	// CPU 画像描画コンテキスト
	//////////////////////////////////////////////////////////////////////////

	class	SGLPaintBuffer : public SGLPaintParameterContext,
								public SGLDrawContextInterface
	{
	public:
		// サンプリング関数
		typedef void (SGLPaintBuffer::*PROC_SAMPLING_PIXELS)
			( SGLPalette * pargbDst, int xDst, int yDst,
							size_t nPixels, uint8_t * pbytBufUnformat ) ;
		// フィルタ処理関数
		typedef void (SGLPaintBuffer::*PROC_FILTER_PIXELS)
			( SGLPalette * pargbPixels, size_t nPixels ) ;
		// 描画関数
		typedef void (SGLPaintBuffer::*PROC_PAINT_PIXELS)
			( SGLPalette * pargbDst, float32_t * pzBuf,
					const SGLPalette * pargbSrc, size_t nPixels ) ;
		typedef void (SGLPaintBuffer::*PROC_PERFORM_PAINT)( void ) ;

	protected:
		// 描画先バッファ情報
		SGLImageInfo	m_infTarget ;
		SGLImageInfo	m_infZBuffer ;
		uint8_t *		m_pbytTarget ;
		uint8_t *		m_pbytZBuffer ;
		bool			m_flagUpdateTarget ;
		bool			m_flagUpdateZBuf ;

		// 入力画像バッファ情報
		SGLImageInfo	m_infSource ;
		uint8_t *		m_pbytSource ;
		SGLPalette		m_tableSrcPalette[0x100] ;

		// 変形なし描画のバッファ情報
		uint8_t *		m_pbytRectDst ;
		uint8_t *		m_pbytRectZBuf ;

		// 描画透明度による乗算テーブル
		uint8_t			m_mulSrcTable[0x100] ;
		uint8_t			m_mulDstTable[0x100] ;

		// フィルタテーブル
		uint8_t			m_filterTable[4][0x100] ;

		// 多角形頂点バッファ
		SSystem::SArray<S2DVector>	m_bufPolyVertices ;

		// リージョンバッファ
		SGLRegion *		m_pbufRegion ;
		SSystem::SArray<uint8_t>	m_bufRegion ;

		// 塗りつぶしタイプ
		enum	FillOperation
		{
			fillFlat,
			fillLinearGradation,
			fillRingedGradation,
			fillTypeCount,
		} ;
		FillOperation	m_opFillType ;
		SGLPalette		m_argbFill ;			// 塗りつぶし色
		SSystem::SArray<SGLPalette>
						m_aGradation ;			// グラデーション色
		S2DVector		m_vGradationCenter ;	// グラデーション基準点
		S2DVector		m_vGradationDelta ;		// グラデーション基底（*色数/長さ）
		float32_t		m_radGradation ;

		// 中間描画バッファ
		class	InternalBuffer
		{
		public:
			SSystem::SArray<uint8_t>	m_bufSrcUnformat ;	// ソースが ARGB32 でない場合、元フォーマットのまま格納
			SSystem::SArray<SGLPalette>	m_bufSrcSampled ;	// ARGB32 形式でのソース
			SSystem::SArray<SGLPalette>	m_bufDstSampled ;	// 出力が ARGB32 でない場合、ここに一旦描画する
			SSystem::SArray<float32_t>	m_bufZSampled ;
			SGLPalette *	m_pbufSrcLine ;
			SGLPalette *	m_pbufDstLine ;
			float32_t *		m_pbufZBufLine ;
			uint8_t *		m_pbytBufUnformat ;
			//
			uint32_t		m_xLeft, m_xRight ;
			int32_t			m_yLine ;
			uint8_t *		m_pbytDst ;
			uint8_t *		m_pbytZBuf ;
			uint8_t *		m_pbytSrc ;
		public:
			// バッファ準備
			void PrepareBuffer( uint32_t width ) ;
		} ;
		enum	ConstantValue
		{
			MAX_THREADS	= 4,
		} ;
		size_t			m_countThreads ;
		InternalBuffer	m_bufInternal[MAX_THREADS] ;

		// サンプリング座標変換行列
		//（出力座標系→入力画像座標系：16ビット固定小数点）
		int32_t			m_fxSampling00, m_fxSampling10 ;
		int32_t			m_fxSampling01, m_fxSampling11 ;
		int32_t			m_fxSampling02, m_fxSampling12 ;

		// 描画属性パラメータ
		uint32_t		m_nTransparency ;	// [0,0x100]
		float32_t		m_zOrder ;			// ｚ値
		SGLPalette		m_argbColorParam1 ;	// 色パラメータ

		// サンプリング関数
		PROC_SAMPLING_PIXELS			m_pfnSampleSrc ;
		PROC_CONVERT_COLOR_FORMAT		m_pfnNormalizeSrc ;	// ソースが ARGB32 でない場合
		PROC_DECODE_PIXEL_COMPOSITION	m_pfnConvertSrc ;	// ソースが 32 ビットでない場合
		PROC_DECODE_PIXEL_COMPOSITION	m_pfnSampleDst ;	// 出力が ARGB32 形式でない場合
		PROC_DECODE_PIXEL_COMPOSITION	m_pfnSampleZBuf ;	// ｚバッファへ書き出さない場合

		// フィルタ処理関数
		PROC_FILTER_PIXELS				m_pfnFilterSrc ;
		PROC_FILTER_PIXELS				m_pfnFilterPost ;

		// フィルタ関数テーブル
		static const PROC_FILTER_PIXELS	m_tableFilterProc[0x10] ;

		// ストア関数
		PROC_ENCODE_PIXEL_COMPOSITION	m_pfnStoreDst ;		// 出力が ARGB32 形式でない場合

		// 描画処理関数
		PROC_PAINT_PIXELS				m_pfnPaintLine ;	// １ライン単位
		PROC_PAINT_PIXELS				m_pfnPaintPost ;

		// 通常描画関数テーブル [alpha?][transparency?]
		struct	PAINT_PROC_ENTRY
		{
			PROC_PAINT_PIXELS	pfnPaintProc ;		// 描画関数
			bool				fNeedsMulTable ;	// 透明度による乗算テーブルを必要とするか？
		} ;
		static const PAINT_PROC_ENTRY	m_tableNormalPaintProc[2][2] ;
		// 特殊描画関数テーブル [transparency?][func]
		static const PROC_PAINT_PIXELS	m_tablePaintLineProc[2][0x10] ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
			( SakuraGL::SGLPaintBuffer,
				SGLPaintParameterContext, SGLDrawContextInterface )
		// 構築関数
		SGLPaintBuffer( void ) ;
		// 消滅関数
		virtual ~SGLPaintBuffer( void ) ;

	public:	// SGLPaintContextInterface オーバーライド
		// 描画先設定
		virtual SGLError AttachTargetImage
			( SGLImageObject * pImage,
					SGLImageObject * pZBuffer,
					const SGLImageRect * pView = NULL ) ;
		// 描画先解除
		virtual SGLError DetachTargetImage( void ) ;
		// 描画先クリア
		virtual SGLError FillClearTarget
				( uint32_t argb = 0xff000000, int64_t flags = 0 ) ;
		// 形状描画
		virtual SGLError FillRectangle
			( int x, int y, int width, int height,
				uint32_t argb, double z = 0.0, uint32_t flags = 0 ) ;
		virtual SGLError FillPolygon
			( const S2DVector * vertices, size_t count,
				uint32_t argb, double z = 0.0, uint32_t flags = 0 ) ;
		// 画像描画
		virtual SGLError DrawImage
			( const SGLPaintParam & ppPaint,
				SGLImageObject * pSrcImage,
				const SGLImageRect * pSrcClip = NULL ) ;
		// 描画の確定
		virtual SGLError Flush( void ) ;
		virtual SGLError Finish( void ) ;

	public:
		// 点描画
		virtual SGLError DrawPoints
			( const S2DVector * pPoints, size_t nPoints,
				uint32_t argb, double z = 0.0, uint32_t flags = 0 ) ;
		// 直線描画
		virtual SGLError DrawThinLines
			( const S2DVector * pLines, size_t nLines,
				uint32_t argb, double z = 0.0, uint32_t flags = 0 ) ;
		// グラデーション解除
		virtual void FreeGradation( void ) ;
		// 線形グラデーション設定
		virtual SGLError SetLinearGradation
			( float32_t x0, float32_t y0, float32_t x1, float32_t y1,
					const SGLPalette * pGradation, size_t nCount ) ;
		// 環状グラデーション設定
		virtual SGLError SetRingedGradation
			( float32_t xCenter, float32_t yCenter, float32_t radAngle,
				const SGLPalette * pGradation, size_t nCount ) ;

	protected:
		// フィルタ関数・ストア関数・描画関数設定
		bool PrepareFilterPaintProc
			( uint32_t nFlags,
				const SGLImageInfo& infSrc, SGLImageObject * pSrcImage,
				uint32_t nTransparency = 0,
				float32_t zOrder = 0.0f, uint32_t argbColorParam = 0 ) ;

	protected:	// サンプリング関数
		// 塗りつぶし関数用関数テーブル [FillOperation]
		static const PROC_SAMPLING_PIXELS	m_tableFillSamplingProc[fillTypeCount] ;

		// サンプリング関数テーブル [depth/8-1][pitch==depth/8?][smoothing?]
		static const PROC_SAMPLING_PIXELS	m_tableSamplingProc[4][2][2] ;

		// fill color
		void SamplingFillColorProc
			( SGLPalette * pargbDst,
				int xDst, int yDst, size_t nPixels, uint8_t * pbytBufUnformat ) ;
		// linear gradation color
		void SamplingLinearGradationProc
			( SGLPalette * pargbDst,
				int xDst, int yDst, size_t nPixels, uint8_t * pbytBufUnformat ) ;
		// ringed gradation color
		void SamplingRingedGradationProc
			( SGLPalette * pargbDst,
				int xDst, int yDst, size_t nPixels, uint8_t * pbytBufUnformat ) ;
		// source 32bit 4pitch
		void Sampling32bitsProc
			( SGLPalette * pargbDst,
				int xDst, int yDst, size_t nPixels, uint8_t * pbytBufUnformat ) ;
		void Sampling32bitsNpitchProc
			( SGLPalette * pargbDst,
				int xDst, int yDst, size_t nPixels, uint8_t * pbytBufUnformat ) ;
		void SamplingSmooth32bitsProc
			( SGLPalette * pargbDst,
				int xDst, int yDst, size_t nPixels, uint8_t * pbytBufUnformat ) ;
		// source 24bit
		void Sampling24bitsNpitchProc
			( SGLPalette * pargbDst,
				int xDst, int yDst, size_t nPixels, uint8_t * pbytBufUnformat ) ;
		// source 16bit
		void Sampling16bitsNpitchProc
			( SGLPalette * pargbDst,
				int xDst, int yDst, size_t nPixels, uint8_t * pbytBufUnformat ) ;
		// source 8bit
		void Sampling8bitsNpitchProc
			( SGLPalette * pargbDst,
				int xDst, int yDst, size_t nPixels, uint8_t * pbytBufUnformat ) ;
		// source S3TC DXT1 RGBA
		void SamplingRGBA_S3TC_DXT1
			( SGLPalette * pargbDst,
				int xDst, int yDst, size_t nPixels, uint8_t * pbytBufUnformat ) ;

	protected:	// フィルタ処理関数
		// 何もしない
		void FilterNothingProc
			( SGLPalette * pargbPixels, size_t nPixels ) ;
		// 透明度
		void FilterTransparencyProc
			( SGLPalette * pargbPixels, size_t nPixels ) ;
		// パレットテーブル展開
		void FilterIndexedPaletteProc
			( SGLPalette * pargbPixels, size_t nPixels ) ;
		// paintApplyColorAdd
		void FilterColorAddProc
			( SGLPalette * pargbPixels, size_t nPixels ) ;
		// paintApplyColorMul
		void FilterColorMulProc
			( SGLPalette * pargbPixels, size_t nPixels ) ;
		// paintApplyAlphaMul
		void FilterAlphaMulProc
			( SGLPalette * pargbPixels, size_t nPixels ) ;
		// paintApplyColorMask
		void FilterColorMaskProc
			( SGLPalette * pargbPixels, size_t nPixels ) ;

	public:	// 描画処理関数
		// 除算描画用逆数テーブル
		static const uint32_t	m_tableDivBlend[0x100] ;

	protected:
		// 通常描画
		void PaintNormalBlendProc
			( SGLPalette * pargbDst, float32_t * pzBuf,
					const SGLPalette * pargbSrc, size_t nPixels ) ;
		// 通常描画（RGB ソース）
		void PaintNormalProc
			( SGLPalette * pargbDst, float32_t * pzBuf,
					const SGLPalette * pargbSrc, size_t nPixels ) ;
		// 透明度付き描画
		void PaintTransparencyBlendProc
			( SGLPalette * pargbDst, float32_t * pzBuf,
					const SGLPalette * pargbSrc, size_t nPixels ) ;
		// 透明度付き描画（RGB ソース）
		void PaintTransparencyProc
			( SGLPalette * pargbDst, float32_t * pzBuf,
					const SGLPalette * pargbSrc, size_t nPixels ) ;
		// ｚ比較付き描画
		void PaintBlendWithZProc
			( SGLPalette * pargbDst, float32_t * pzBuf,
					const SGLPalette * pargbSrc, size_t nPixels ) ;
		// 上書き描画
		void PaintNoBlendAlphaProc
			( SGLPalette * pargbDst, float32_t * pzBuf,
					const SGLPalette * pargbSrc, size_t nPixels ) ;
		// 透明度付き上書き描画
		void PaintTransparencyNoBlendAlphaProc
			( SGLPalette * pargbDst, float32_t * pzBuf,
					const SGLPalette * pargbSrc, size_t nPixels ) ;
		// 加算描画
		void PaintAddBlendProc
			( SGLPalette * pargbDst, float32_t * pzBuf,
					const SGLPalette * pargbSrc, size_t nPixels ) ;
		// 減算描画
		void PaintSubBlendProc
			( SGLPalette * pargbDst, float32_t * pzBuf,
					const SGLPalette * pargbSrc, size_t nPixels ) ;
		// 積算描画
		void PaintMulBlendProc
			( SGLPalette * pargbDst, float32_t * pzBuf,
					const SGLPalette * pargbSrc, size_t nPixels ) ;
		// 除算描画（覆い焼き）
		void PaintDivBlendProc
			( SGLPalette * pargbDst, float32_t * pzBuf,
					const SGLPalette * pargbSrc, size_t nPixels ) ;
		// 最大値選択
		void PaintMaxBlendProc
			( SGLPalette * pargbDst, float32_t * pzBuf,
					const SGLPalette * pargbSrc, size_t nPixels ) ;
		// 最小値選択
		void PaintMinBlendProc
			( SGLPalette * pargbDst, float32_t * pzBuf,
					const SGLPalette * pargbSrc, size_t nPixels ) ;
		// 反転乗算描画（スクリーン）
		void PaintNegMulBlendProc
			( SGLPalette * pargbDst, float32_t * pzBuf,
					const SGLPalette * pargbSrc, size_t nPixels ) ;
		// αチャネルで ARGB 乗算して、入力 RGB を加算
		void PaintMulAlphaBlendProc
			( SGLPalette * pargbDst, float32_t * pzBuf,
					const SGLPalette * pargbSrc, size_t nPixels ) ;
		// 出力先αチャネルで入力 ARGB 乗算して描画、出力先αチャネルは不変
		void PaintDstAlphaMaskBlendProc
			( SGLPalette * pargbDst, float32_t * pzBuf,
					const SGLPalette * pargbSrc, size_t nPixels ) ;
		// ｚ比較出力
		void PaintWriteWithZProc
			( SGLPalette * pargbDst, float32_t * pzBuf,
					const SGLPalette * pargbSrc, size_t nPixels ) ;

	protected:	// 変形なし描画処理フレームワーク関数
		// ARGB -> ARGB 描画
		void PerformPaintRectSimple( void ) ;

		// ARGB -> ARGB 描画・並列処理用
		class	PaintRectSimpleProc	: public SSystem::SParallelProcedure
		{
		protected:
			SGLPaintBuffer *	m_paint ;

			SGLPaintBuffer::PROC_PAINT_PIXELS	m_pfnPaintLine ;	// １ライン単位

			uint8_t *		m_pbytRectDst ;
			uint8_t *		m_pbytRectZBuf ;
			uint8_t *		m_pbytRectSrc ;
			uint32_t		m_widthRect ;
			uint32_t		m_heightRect ;
			int32_t			m_pitchDst ;
			int32_t			m_pitchZBuf ;
			int32_t			m_pitchSrc ;

			uint32_t		m_yNext ;

		public:
			// 構築関数
			PaintRectSimpleProc( SGLPaintBuffer * paint ) ;
			// ループ処理／終了判定関数
			virtual bool Continue( void * pInstance ) ;
			// 並列処理関数
			virtual void RunParallel( void * pInstance ) ;
		} ;

	protected:
		// 変形なし汎用描画
		void PerformPaintRectGeneric( void ) ;

		// 変形なし汎用描画・並列処理用
		class	PaintRectGenericProc : public SSystem::SParallelProcedure
		{
		protected:
			SGLPaintBuffer *	m_paint ;

			// サンプリング関数
			PROC_CONVERT_COLOR_FORMAT		m_pfnNormalizeSrc ;	// ソースが ARGB32 でない場合
			PROC_DECODE_PIXEL_COMPOSITION	m_pfnConvertSrc ;	// ソースが 32 ビットでない場合
			PROC_DECODE_PIXEL_COMPOSITION	m_pfnSampleDst ;	// 出力が ARGB32 形式でない場合
			PROC_DECODE_PIXEL_COMPOSITION	m_pfnSampleZBuf ;	// ｚバッファへ書き出さない場合

			// フィルタ処理関数
			SGLPaintBuffer::PROC_FILTER_PIXELS	m_pfnFilterSrc ;
			SGLPaintBuffer::PROC_FILTER_PIXELS	m_pfnFilterPost ;

			// ストア関数
			PROC_ENCODE_PIXEL_COMPOSITION	m_pfnStoreDst ;		// 出力が ARGB32 形式でない場合

			// 描画処理関数
			SGLPaintBuffer::PROC_PAINT_PIXELS	m_pfnPaintLine ;	// １ライン単位
			SGLPaintBuffer::PROC_PAINT_PIXELS	m_pfnPaintPost ;

			uint8_t *		m_pbytRectDst ;
			uint8_t *		m_pbytRectZBuf ;
			uint8_t *		m_pbytRectSrc ;
			uint32_t		m_widthRect ;
			uint32_t		m_heightRect ;
			int32_t			m_pitchDst ;
			int32_t			m_pitchZBuf ;
			int32_t			m_pitchSrc ;

			uint32_t		m_yNext ;

		public:
			// 構築関数
			PaintRectGenericProc( SGLPaintBuffer * paint ) ;
			// ループ処理／終了判定関数
			virtual bool Continue( void * pInstance ) ;
			// 並列処理関数
			virtual void RunParallel( void * pInstance ) ;
		} ;

	protected:	// 変形あり描画処理フレームワーク関数
		// 変形あり汎用描画
		void PerformPaintTransformedGeneric( void ) ;

		// 変形あり汎用描画・並列処理用
		class	PaintTransformedGenericProc : public SSystem::SParallelProcedure
		{
		protected:
			SGLPaintBuffer *	m_paint ;
			//
			// サンプリング関数
			SGLPaintBuffer::PROC_SAMPLING_PIXELS	m_pfnSampleSrc ;
			PROC_CONVERT_COLOR_FORMAT		m_pfnNormalizeSrc ;	// ソースが ARGB32 でない場合／出力がソースと一致しない場合
			PROC_DECODE_PIXEL_COMPOSITION	m_pfnSampleDst ;	// 出力が ARGB32 形式でない場合
			PROC_DECODE_PIXEL_COMPOSITION	m_pfnSampleZBuf ;	// ｚバッファへ書き出さない場合

			// フィルタ処理関数
			SGLPaintBuffer::PROC_FILTER_PIXELS	m_pfnFilterSrc ;
			SGLPaintBuffer::PROC_FILTER_PIXELS	m_pfnFilterPost ;

			// ストア関数
			PROC_ENCODE_PIXEL_COMPOSITION	m_pfnStoreDst ;		// 出力が ARGB32 形式でない場合

			// 描画処理関数
			SGLPaintBuffer::PROC_PAINT_PIXELS	m_pfnPaintLine ;	// １ライン単位
			SGLPaintBuffer::PROC_PAINT_PIXELS	m_pfnPaintPost ;

			//
			uint8_t *			m_pbytDst ;
			uint8_t *			m_pbytZBuf ;
			int32_t				m_pitchDst ;
			int32_t				m_pitchZBuf ;
			//
			int32_t				m_yRegionTop ;
			int32_t				m_yRegionBottom ;
			SGLRegionLine *		m_prgLine ;
			//
			int32_t				m_yNext ;

		public:
			// 構築関数
			PaintTransformedGenericProc( SGLPaintBuffer * paint ) ;
			// ループ処理／終了判定関数
			virtual bool Continue( void * pInstance ) ;
			// 並列処理関数
			virtual void RunParallel( void * pInstance ) ;
		} ;

		friend class PaintRectSimpleProc ;
		friend class PaintRectGenericProc ;
		friend class PaintTransformedGenericProc ;

	} ;

}

#endif
