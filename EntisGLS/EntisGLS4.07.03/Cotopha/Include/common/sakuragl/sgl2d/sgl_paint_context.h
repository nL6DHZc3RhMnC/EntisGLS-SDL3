
#if	!defined(__SAKURAGL_PAINT_CCONTEXT_H__)
#define	__SAKURAGL_PAINT_CCONTEXT_H__

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// 画像描画コンテキスト
	//////////////////////////////////////////////////////////////////////////

	// 描画コンテキスト種類
	enum	SGLPaintContextType
	{
		typePaintDefault,
		typePaintEntisGLS,
		typePaintEntisGLS4,
		typePaintOpenGL,
		typePaintDirect3D,
	} ;
	// 描画フラグ
	enum	SGLPaintFlag
	{
		// 入力画像←入力色ファンクション
		paintApplyColorAdd		= 0x00800000,
		paintApplyColorMul		= 0x00820000,
		paintApplyAlphaMul		= 0x00880000,
		paintApplyColorMask		= 0x00890000,
		paintMaskApply			= 0x00FF0000,
		paintApplyShifter		= 16,
		// 出力画像←入力画像描画ファンクション
		paintFunctionAdd		= 0x80000000,
		paintFunctionSub		= 0x81000000,
		paintFunctionMul		= 0x82000000,
		paintFunctionDiv		= 0x83000000,
		paintFunctionMax		= 0x84000000,
		paintFunctionMin		= 0x85000000,
		paintFunctionScreen		= 0x86000000,
		paintFunctionMove		= 0x88000000,
		paintMultipleByAlpha	= 0x89000000,
		paintDstAlphaMask		= 0x8A000000,
		paintMaskFunction		= 0xFF000000,
		paintFunctionShifter	= 24,
		// 描画フラグ
		paintNoBlendAlpha		= paintFunctionMove,	// 入力画像のαを無視して出力先に上書きする
		paintNoProductOfAlpha	= paintApplyAlphaMul,	// 入力画像のαチャネルで RGB チャネルを積算する
		paintWithZOrder			= 0x00000002,		// ｚ値比較
		paintWithZOrderNoWrite	= 0x00000004,		// ｚ値比較（ｚバッファへ書き出さない）
		paintSmoothStretch		= 0x00000010,		// ピクセル補完
		paintUnsmoothStretch	= 0x00000020,		// ピクセル補完無効化
		paintFixedPosition		= 0x00000040,		// 入力座標は固定小数点（x10000H）
		paintPolygonShaped		= 0x00000080,		// 出力先描画形状指定
		paintDelayable			= 0x00000100,		// 遅延描画されても良い
		paintOrderNoCare		= 0x00000200,		// 遅延描画で描画順は問わない
	} ;
	// 描画パラメータ
	struct	SGLPaintParam
	{
		uint32_t			nFlags ;
		uint32_t			nReserved1 ;
		SGLPoint			ptPaint ;
		uint32_t			nTransparency ;
		float32_t			zOrder ;
		SGLPalette			rgbColorParam ;
		uint32_t			nReserved2 ;
		const SGLAffine *	pAffine ;
		const S2DVector *	pVertices ;
		uint32_t			countVertex ;

		// 構築関数
		#if	!defined(__COTOPHA__)
		SGLPaintParam( void )
			: nFlags(0), nReserved1(0), nTransparency(0),
				zOrder(0.0f), nReserved2(0), pAffine(NULL),
				pVertices(NULL), countVertex(0) {}
		#endif
		// アフィン設定
		void SetAffine
			( SGLAffine& affine, double xDst, double yDst,
				double xSrcOrg, double ySrcOrg,
				double xZoom = 1.0, double yZoom = 1.0,
				double zAngle = 0.0, double xyCross = 90.0 ) ;
		// アフィン取得
		void GetAffine( SGLAffine& affine ) const ;
	} ;

	#if	defined(__COTOPHA__)
	class	native PaintContext : public SSystem::VolatileObject
	{
	public:
		// 描画オブジェクト生成
		native static PaintContext *
			NewContext( SGLPaintContextType type = typePaintDefault ) ;
		// 描画先取得
		native Image * GetTargetImage( void ) ;
		native Image * GetTargetZBuffer( void ) ;
		// ビューポート取得
		native SGLError GetViewPort( SGLImageRect & rctView ) const ;
		// 描画先設定
		native SGLError AttachTargetImage
			( Image * pImage, Image * pZBuffer,
					const SGLImageRect * pView = NULL ) ;
		// 描画先解除
		native SGLError DetachTargetImage( void ) ;
		// 描画座標空間設定
		native SGLError AppendTransformation
			( const SGLAffine & af, unsigned int nTransparency ) ;
		native SGLError SetTransformation
			( const SGLAffine & af, unsigned int nTransparency ) ;
		native SGLError CurrentAffine( SGLAffine & af ) ;
		native unsigned int CurrentTransparency( void ) ;
		native SGLError PushTransformation( void ) ;
		native SGLError PopTransformation( void ) ;
		native SGLError ResetTransformation( void ) ;
		// 描画デフォルトフラグ
		native void SetPaintFlags( int64_t nFlags ) ;
		native int64_t GetPaintFlags( void ) ;
		// クリアターゲット
		enum	ClearTargetFlag
		{
			clearTargetColor	= 0x0001,
			clearTargetZBuffer	= 0x0002,
		} ;
		// 描画先クリア
		native SGLError FillClearTarget( uint32_t argb, int64_t flags ) ;
		// 形状描画
		native SGLError FillRectangle
			( int x, int y, int width, int height,
						uint32_t argb, double z, uint32_t flags ) ;
		native SGLError FillPolygon
			( const S2DVector * vertices, size_t count,
						uint32_t argb, double z, uint32_t flags ) ;
		// 画像描画
		native SGLError DrawImage
			( const SGLPaintParam & ppPaint,
				Image * pSrcImage,
				const SGLImageRect * pSrcClip = NULL ) ;
		// ２Ｄメッシュ描画
		native SGLError DrawMesh
			( const S2DVector * pDstMesh,
				const S2DVector * pSrcMesh,
				size_t widthMesh, size_t heightMesh,
				const SGLPaintParam & ppPaint,
				Image * pSrcImage,
				const SGLImageRect * pSrcClip = NULL ) ;
		// 描画の確定
		native SGLError Flush( void ) ;
		// 描画の確定（描画ターゲットのメモリへの反映）
		native SGLError Finish( void ) ;
	} ;
	#endif

	class	SGLPaintContextInterface	: public SSystem::SObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SakuraGL::SGLPaintContextInterface, SObject )
		// 描画先取得
		virtual SGLImageObject * GetTargetImage( void ) = 0 ;
		virtual SGLImageObject * GetTargetZBuffer( void ) = 0 ;
		// ビューポート取得
		virtual SGLError GetViewPort( SGLImageRect & rctView ) const = 0 ;
		// 描画先設定
		virtual SGLError AttachTargetImage
			( SGLImageObject * pImage,
					SGLImageObject * pZBuffer,
					const SGLImageRect * pView = NULL ) = 0 ;
		// 描画先解除
		virtual SGLError DetachTargetImage( void ) = 0 ;
		// 描画座標空間設定
		virtual SGLError AppendTransformation
			( const SGLAffine & af, unsigned int nTransparency ) = 0 ;
		SGLError AddTransformation
			( const SGLAffine & af, unsigned int nTransparency )
		{
			return	AppendTransformation( af, nTransparency ) ;
		}
		virtual SGLError SetTransformation
			( const SGLAffine & af, unsigned int nTransparency ) = 0 ;
		virtual SGLError CurrentAffine( SGLAffine & af ) = 0 ;
		virtual unsigned int CurrentTransparency( void ) = 0 ;
		virtual SGLError PushTransformation( void ) = 0 ;
		virtual SGLError PopTransformation( void ) = 0 ;
		virtual SGLError ResetTransformation( void ) = 0 ;
		// 描画デフォルトフラグ
		virtual void SetPaintFlags( int64_t nFlags ) = 0 ;
		virtual int64_t GetPaintFlags( void ) = 0 ;
		// クリアターゲット
		enum	ClearTargetFlag
		{
			clearTargetColor	= 0x0001,
			clearTargetZBuffer	= 0x0002,
		} ;
		// 描画先クリア
		virtual SGLError FillClearTarget
				( uint32_t argb = 0xff000000, int64_t flags = 0 ) = 0 ;
		// 形状描画
		virtual SGLError FillRectangle
			( int x, int y, int width, int height,
				uint32_t argb, double z = 0.0, uint32_t flags = 0 ) = 0 ;
		virtual SGLError FillPolygon
			( const S2DVector * vertices, size_t count,
				uint32_t argb, double z = 0.0, uint32_t flags = 0 ) = 0 ;
		// 画像描画
		virtual SGLError DrawImage
			( const SGLPaintParam & ppPaint,
				SGLImageObject * pSrcImage,
				const SGLImageRect * pSrcClip = NULL ) = 0 ;
		// ２Ｄメッシュ描画
		virtual SGLError DrawMesh
			( const S2DVector * pDstMesh,
				const S2DVector * pSrcMesh,
				size_t widthMesh, size_t heightMesh,
				const SGLPaintParam & ppPaint,
				SGLImageObject * pSrcImage,
				const SGLImageRect * pSrcClip = NULL ) = 0 ;
		// 複数画像描画
		virtual SGLError DrawMultiImages
			( size_t nCount,
				const SGLPaintParam * pParams,
				SGLImageObject *const* ppSrcImages,
				const SGLImageRect * pSrcClips = NULL ) = 0 ;
		// 描画の確定
		virtual SGLError Flush( void ) = 0 ;
		// 描画の確定（描画ターゲットのメモリへの反映）
		virtual SGLError Finish( void ) = 0 ;

	public:
		#if	defined(__COTOPHA__)
		// PaintContext 取得
		virtual PaintContext * GetPaintContextObject( void ) const ;
		#endif
	} ;

	#if	!defined(__COTOPHA__)
	typedef	SGLPaintContextInterface	PaintContext ;
	#endif


	//////////////////////////////////////////////////////////////////////////
	// 線描画コンテキスト
	//////////////////////////////////////////////////////////////////////////

	class	SGLDrawContextInterface	: public ESLObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SakuraGL::SGLDrawContextInterface, ESLObject )
		// 点描画
		virtual SGLError DrawPoints
			( const S2DVector * pPoints, size_t nPoints,
				uint32_t argb, double z = 0.0, uint32_t flags = 0 ) = 0 ;
		// 直線描画
		virtual SGLError DrawThinLine
			( int x0, int y0, int x1, int y1,
				uint32_t argb, double z = 0.0, uint32_t flags = 0 ) ;
		virtual SGLError DrawThinLines
			( const S2DVector * pLines, size_t nLines,
				uint32_t argb, double z = 0.0, uint32_t flags = 0 ) = 0 ;
		// 楕円輪郭描画
		virtual SGLError DrawEllipse
			( float32_t xCenter, float32_t yCenter,
				float32_t rWidth, float32_t rHeight,
				uint32_t argb, double z = 0.0, uint32_t flags = 0 ) ;
		// 円弧輪郭描画
		virtual SGLError DrawArc
			( float32_t xCenter, float32_t yCenter,
				float32_t rWidth, float32_t rHeight,
				float32_t radFirst, float32_t radEnd,
				uint32_t argb, double z = 0.0, uint32_t flags = 0 ) ;
		// ベジェ曲線描画
		virtual SGLError DrawBezier
			( const S2DVector * pPoints, size_t nPoints,
				uint32_t argb, double z = 0.0, uint32_t flags = 0 ) ;
		// 矩形塗りつぶし
		virtual SGLError FillRectangle
			( int x, int y, int width, int height,
						uint32_t argb, double z = 0.0, uint32_t flags = 0 ) ;
		// （凸な）多角形塗りつぶし
		virtual SGLError FillPolygon
			( const S2DVector * vertices, size_t count,
						uint32_t argb, double z = 0.0, uint32_t flags = 0 ) = 0 ;
		// 楕円塗りつぶし
		virtual SGLError FillEllipse
			( float32_t xCenter, float32_t yCenter,
				float32_t rWidth, float32_t rHeight,
				uint32_t argb, double z = 0.0, uint32_t flags = 0 ) ;
		// 円弧塗りつぶし
		virtual SGLError FillArc
			( float32_t xCenter, float32_t yCenter,
				float32_t rWidth, float32_t rHeight,
				float32_t radFirst, float32_t radEnd,
				uint32_t argb, double z = 0.0, uint32_t flags = 0 ) ;
		// グラデーション解除
		virtual void FreeGradation( void ) = 0 ;
		// 線形グラデーション設定
		virtual SGLError SetLinearGradation
			( float32_t x0, float32_t y0, float32_t x1, float32_t y1,
					const SGLPalette * pGradation, size_t nCount ) = 0 ;
		// 環状グラデーション設定
		virtual SGLError SetRingedGradation
			( float32_t xCenter, float32_t yCenter, float32_t radAngle,
				const SGLPalette * pGradation, size_t nCount ) = 0 ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// PaintContext - SGLPaintContextInterface インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SGLPaintContext	: public SGLPaintContextInterface
	{
	protected:
		// 接続オブジェクト
		PaintContext *	m_paint ;
		bool			m_flagOwner ;

		// 描画先情報
		SGLImage			m_imgTarget ;
		SGLImage			m_imgZBuffer ;
		SGLImageObject *	m_pTarget ;
		SGLImageObject *	m_pZBuffer ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLPaintContext, SGLPaintContextInterface )
		// 構築関数
		SGLPaintContext( void ) ;
		SGLPaintContext( PaintContext * paint, bool flagOwner = false ) ;
		// 消滅関数
		virtual ~SGLPaintContext( void ) ;
		// 関連付け
		void AttachPaintContext( PaintContext * paint, bool flagOwner = false ) ;
		// PaintContext 取得
		PaintContext * GetPaintContext( void ) const
		{
			return	m_paint ;
		}
		operator PaintContext * ( void ) const
		{
			return	m_paint ;
		}
		PaintContext * operator -> ( void ) const
		{
			return	m_paint ;
		}

	public:
		// 描画先取得
		virtual SGLImageObject * GetTargetImage( void ) ;
		virtual SGLImageObject * GetTargetZBuffer( void ) ;
		// ビューポート取得
		virtual SGLError GetViewPort( SGLImageRect & rctView ) const ;
		// 描画先設定
		virtual SGLError AttachTargetImage
			( SGLImageObject * pImage,
					SGLImageObject * pZBuffer,
					const SGLImageRect * pView = NULL ) ;
		// 描画先解除
		virtual SGLError DetachTargetImage( void ) ;
		// 描画座標空間設定
		virtual SGLError AppendTransformation
			( const SGLAffine & af, unsigned int nTransparency ) ;
		virtual SGLError SetTransformation
			( const SGLAffine & af, unsigned int nTransparency ) ;
		virtual SGLError CurrentAffine( SGLAffine & af ) ;
		virtual unsigned int CurrentTransparency( void ) ;
		virtual SGLError PushTransformation( void ) ;
		virtual SGLError PopTransformation( void ) ;
		virtual SGLError ResetTransformation( void ) ;
		// 描画デフォルトフラグ
		virtual void SetPaintFlags( int64_t nFlags ) ;
		virtual int64_t GetPaintFlags( void ) ;
		// 描画先クリア
		virtual SGLError FillClearTarget
				( uint32_t argb = 0xff000000, int64_t flags = 0 ) ;
		// 形状描画
		virtual SGLError FillRectangle
			( int x, int y, int width, int height,
						uint32_t argb, double z, uint32_t flags ) ;
		virtual SGLError FillPolygon
			( const S2DVector * vertices, size_t count,
						uint32_t argb, double z, uint32_t flags ) ;
		// 画像描画
		virtual SGLError DrawImage
			( const SGLPaintParam & ppPaint,
				SGLImageObject * pSrcImage,
				const SGLImageRect * pSrcClip = NULL ) ;
		// ２Ｄメッシュ描画
		virtual SGLError DrawMesh
			( const S2DVector * pDstMesh,
				const S2DVector * pSrcMesh,
				size_t widthMesh, size_t heightMesh,
				const SGLPaintParam & ppPaint,
				SGLImageObject * pSrcImage,
				const SGLImageRect * pSrcClip = NULL ) ;
		// 複数画像描画
		virtual SGLError DrawMultiImages
			( size_t nCount,
				const SGLPaintParam * pParams,
				SGLImageObject *const* ppSrcImages,
				const SGLImageRect * pSrcClips = NULL ) ;
		// 描画の確定
		virtual SGLError Flush( void ) ;
		virtual SGLError Finish( void ) ;

	public:
		#if	defined(__COTOPHA__)
		// PaintContext 取得
		virtual PaintContext * GetPaintContextObject( void ) const ;
		#endif
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 2D 描画コンテキスト基底
	//////////////////////////////////////////////////////////////////////////

	class	SGLPaintParameterContext	: public SGLPaintContextInterface
	{
	protected:
		// 描画先情報
		SGLImageObject *	m_pTarget ;
		SGLImageObject *	m_pZBuffer ;
		SGLImageRect		m_rctView ;

		// 座標変換
		struct	TransformationList
		{
			TransformationList *	pPrev ;
			SGLAffine				afTransform ;
			unsigned int			nTransparency ;

			TransformationList( void )
				: pPrev( NULL ), nTransparency( 0 ) {}
		} ;
		TransformationList *	m_pTransformation ;

		// デフォルト描画フラグ
		int64_t		m_flagsDefPaint ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO
			( SakuraGL::SGLPaintParameterContext, SGLPaintContextInterface )
		// 構築関数
		SGLPaintParameterContext( void ) ;
		// 消滅関数
		virtual ~SGLPaintParameterContext( void ) ;

	public:	// SGLPaintContextInterface オーバーライド
		// 描画先取得
		virtual SGLImageObject * GetTargetImage( void ) ;
		virtual SGLImageObject * GetTargetZBuffer( void ) ;
		// ビューポート取得
		virtual SGLError GetViewPort( SGLImageRect & rctView ) const ;
		// 描画先設定
		virtual SGLError AttachTargetImage
			( SGLImageObject * pImage,
					SGLImageObject * pZBuffer,
					const SGLImageRect * pView = NULL ) ;
		// 描画先解除
		virtual SGLError DetachTargetImage( void ) ;
		// 描画座標空間設定
		virtual SGLError AppendTransformation
			( const SGLAffine & af, unsigned int nTransparency ) ;
		virtual SGLError SetTransformation
			( const SGLAffine & af, unsigned int nTransparency ) ;
		virtual SGLError CurrentAffine( SGLAffine & af ) ;
		virtual unsigned int CurrentTransparency( void ) ;
		virtual SGLError PushTransformation( void ) ;
		virtual SGLError PopTransformation( void ) ;
		virtual SGLError ResetTransformation( void ) ;
		// 描画デフォルトフラグ
		virtual void SetPaintFlags( int64_t nFlags ) ;
		virtual int64_t GetPaintFlags( void ) ;
		// ２Ｄメッシュ描画（DrawImage 呼び出し）
		virtual SGLError DrawMesh
			( const S2DVector * pDstMesh,
				const S2DVector * pSrcMesh,
				size_t widthMesh, size_t heightMesh,
				const SGLPaintParam & ppPaint,
				SGLImageObject * pSrcImage,
				const SGLImageRect * pSrcClip = NULL ) ;
		// 複数画像描画
		virtual SGLError DrawMultiImages
			( size_t nCount,
				const SGLPaintParam * pParams,
				SGLImageObject *const* ppSrcImages,
				const SGLImageRect * pSrcClips = NULL ) ;
	public:
		// 画像変形描画
		virtual SGLError DrawTrianglePolygon
			( const S2DVector * pDstVertices,
				const S2DVector * pSrcVertices,
				const SGLPaintParam & ppPaint,
				SGLImageObject * pSrcImage ) ;

	public:
		// 座標空間取得
		bool GetTransformation( SGLAffine & af ) const
		{
			if ( m_pTransformation != NULL )
			{
				af = m_pTransformation->afTransform ;
				return	true ;
			}
			return	false ;
		}
		void GetTransformationOf( SGLAffine & af ) const
		{
			if ( m_pTransformation != NULL )
			{
				af = (m_pTransformation->afTransform * af) ;
			}
		}
		// 透明度取得
		unsigned int GetTransparencyOf( unsigned int nTransparency ) const
		{
			if ( m_pTransformation != NULL )
			{
				return	0x100 - (0x100 - nTransparency)
								* (0x100 - m_pTransformation->nTransparency) ;
			}
			return	nTransparency ;
		}

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 画像描画リスト
	//////////////////////////////////////////////////////////////////////////

	class	S3DRenderBufferInterface ;
	class	SGLDrawImageParamList	: public ESLObject
	{
	protected:
		SGLAffine								m_affine ;
		uint32_t								m_nTransparency ;
		SSystem::SArray<SGLPaintParam>			m_aParams ;
		SSystem::SPointerArray<SGLImageObject>	m_pImages ;
		SSystem::SArray<SGLImageRect>			m_pRects ;
		SSystem::SArray<SGLAffine>				m_aAffines ;
		SSystem::SArray<S2DVector>				m_aVertices ;

		SSystem::SArray<S3DVector4>				m_bufVertex ;
		SSystem::SArray<S2DVector>				m_bufUVMap ;
		SSystem::SArray<S3DColor>				m_bufColor ;
		SSystem::SArray<uint32_t>				m_bufIndexedList ;
		SSystem::SArray<SGLImageRect>			m_bufImageRect ;

	public:
		ESL_DECLARE_CLASS_INFO( SGLDrawImageParamList, ESLObject )
		// 構築関数
		SGLDrawImageParamList( void ) ;
		// 変形
		const SGLAffine& GetAffine( void ) const ;
		void SetAffine( const SGLAffine& affine ) ;
		void AppendAffine( const SGLAffine& affine ) ;
		// 透明度
		uint32_t GetTransparency( void ) const ;
		void SetTransparency( uint32_t nTransparency ) ;
		void AppendTransparency( uint32_t nTransparency ) ;
		// 追加
		void AddDrawParam
			( const SGLPaintParam& param,
				SGLImageObject * pImage,
				const SGLImageRect * pSrcRect = NULL ) ;
	protected:
		SGLAffine * AddAffine( const SGLAffine& affine ) ;
		S2DVector * AddVertices( const S2DVector * pVertices, size_t nCount ) ;

	public:
		// クリア
		void ClearList( void ) ;
		// リストが空か？
		bool IsEmptyList( void ) const ;
		// 描画実行
		void Draw( SGLPaintContextInterface& paint ) const ;
		void DrawToList( SGLDrawImageParamList& dpiList ) const ;
		size_t CountDraw( void ) const ;
		// 描画実行（ピクセルスケールで３次元空間上へ）
		enum	StereoViewIndex
		{
			stereoViewAuto	= -1,
			stereoViewRight,
			stereoViewLeft,
		} ;
		void RenderAs3D
			( S3DRenderBufferInterface& render,
				uint64_t flagsExclusion,
				StereoViewIndex sviView = stereoViewAuto ) ;
	protected:
		void RenderMultiImages
			( S3DRenderBufferInterface& render,
				uint64_t flagsExclusion,
				StereoViewIndex sviView,
				size_t nCount,
				const SGLPaintParam * pParams,
				SGLImageObject *const* ppSrcImages,
				const SGLImageRect * pSrcClips ) ;
	} ;

}

#endif

