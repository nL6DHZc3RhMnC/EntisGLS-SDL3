
#if	!defined(__SAKURAGL_IMAGE_BUF_OBJECT_H__)
#define	__SAKURAGL_IMAGE_BUF_OBJECT_H__

#include <sakuragl/sgl2d/sgl_image.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// 画像オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	SGLSmartImage	: public SGLImageObject
	{
	protected:
		SGLImageBuffer *	m_pImage ;
		bool				m_flagOwnBuffer ;
		atomic_int_t		m_countLocked ;
		SGLImageRect		m_rectLocked ;
		uint8_t *			m_pbytLocked ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLSmartImage, SGLImageObject )
		// 構築関数
		#if	!defined(__COTOPHA__)
		SGLSmartImage( void )
			: m_pImage(NULL), m_flagOwnBuffer(false),
				m_countLocked(0), m_pbytLocked(NULL) {}
		#endif
		SGLSmartImage( SGLImageBuffer * pImage, bool flagOwner = true )
			: m_pImage(pImage), m_flagOwnBuffer(flagOwner),
				m_countLocked(0), m_pbytLocked(NULL) {}
		// 消滅関数
		virtual ~SGLSmartImage( void ) ;
		// 代入
		SGLImageBuffer * operator = ( SGLImageBuffer * pImage )
		{
			return	SetImageBuffer( pImage ) ;
		}
		SGLImageBuffer * SetImageBuffer( SGLImageBuffer * pImage ) ;
		// 関連付け
		void AttachImageBuffer( SGLImageBuffer * pImage ) ;
		// ポインタ変換
		SGLImageBuffer * operator -> ( void ) const
		{
			return	m_pImage ;
		}
		SGLImageBuffer * GetImage( void ) const
		{
			return	m_pImage ;
		}
		operator SGLImageBuffer * ( void ) const
		{
			return	m_pImage ;
		}
		operator SGLImageBuffer & ( void ) const
		{
			return	*m_pImage ;
		}

	public:
		// フレーム選択
		virtual SGLError SelectFrame( size_t iFrame, int iSide = stereoImageRight ) ;
		// 画像情報取得
		virtual SGLError GetImageInfo( SGLImageInfo & imginf ) const ;
		// パレット・テーブル取得
		virtual size_t GetPaletteTable( SGLPalette * pPalette, size_t nCount ) ;
		// 画像バッファ取得
		virtual uint8_t * LockBuffer
			( SGLImageInfo & imginf,
				int flags = lockReadWrite,
				const SGLImageRect * pRect = NULL ) ;
		virtual SGLError FlushBuffer( int flags = lockReadWrite ) ;
		virtual SGLError UnlockBuffer( int flags = lockReadWrite ) ;
		// 画像バッファ読み込み
		virtual SGLError ReadFrameBuffer
			( const SGLImageInfo & imginf, uint8_t * ptrBuffer,
					size_t iFrame = 0, int iSide = stereoImageRight ) ;

	public:
		// 画像バッファへの参照生成
		virtual SGLImageObject * NewReference
			( const SGLImageRect * pClip = NULL,
					ssize_t iFrame = -1, int iSide = stereoImageRight ) ;
		// テクスチャのための正規化
		virtual SGLError NormalizeToTexture( uint32_t nFlags = 0 ) ;
		virtual SGLError NormalizeToMipmapTexture( uint32_t nFlags = 0 ) ;
		// レンダリング・ターゲットのための正規化
		virtual SGLError NormalizeToRenderTarget( uint32_t nFlags = 0 ) ;
		// テクスチャのための正規化フラグを除去する
		virtual SGLError DenormalizeForTexture( uint32_t nFlags ) ;
		// 画像バッファ生成
		virtual SGLError CreateBuffer
			( const SGLImageInfo& imginf,
				uint64_t nFlags = bufferOnMemory,
				size_t countFrame = 1, uint64_t msecLong = 0 ) ;
		// 保有リソース解放
		virtual void ReleaseBuffer( void ) ;
		// バッファフラグ取得
		virtual uint64_t GetBufferFlags( void ) ;
		// パレット・テーブル設定
		virtual size_t SetPaletteTable
			( const SGLPalette * pPalette, size_t nCount ) ;
		// 中心座標情報設定
		virtual void SetImageOrigin( int x, int y ) ;
		// アニメーション全長時間設定
		virtual void SetAnimationDuration( uint64_t nDuration ) ;

	public:
		// SGLImageObject::BufferTypeFlag -> SGLBufferCreationFlag 変換
		static uint64_t ConvertFlagsToBufferFlags( uint64_t nTypeFlags ) ;
		// SGLBufferCreationFlag -> SGLImageObject::BufferTypeFlag 変換
		static uint64_t ConvertFlagsFromBufferFlags( uint64_t nBufFlags ) ;

	public:
		// 画像フォーマット正規化
		virtual SGLError NormalizeFormat
			( uint32_t format = 0, uint32_t depth = 0,
				uint32_t nFlags = 0,
				uint32_t width = 0, uint32_t height = 0 ) ;
	protected:
		SGLImageBuffer * NormalizeBufferFormat
			( SGLImageBuffer * pImageBuf,
				uint32_t format, uint32_t depth,
				uint32_t nFlags, uint32_t width, uint32_t height ) ;
	public:
		// NormalizeFormat で結合されたアニメーション画像への参照を生成
		virtual SGLImageObject * NewAnimationReference
			( SGLImageRect* pFrameRects, size_t nRectsCount ) ;

	public:
		// アトラス化された画像の参照矩形を取得
		virtual SGLError GetReferenceRectOfAtlas
			( SGLImageRect& rect, ssize_t iFrame = -1 ) const ;
		// 参照元画像情報取得
		virtual SGLImageObject * GetImageReference
					( SGLImageRect& rectRef, ssize_t iFrame = -1 ) ;
	protected:
		SGLImageObject * GetImageReferenceOf
					( SGLImageRect& rectRef, SGLImageBuffer * pImage ) ;
		static SGLImageObject *
					CommitImageReferenceOf( SGLImageBuffer * pImage ) ;
	public:
		// 参照先画像変更
		virtual SGLError MakeImageReference
			( size_t iFrame,
				SGLImageObject * pAtlasImage, const SGLImageRect& rectRef ) ;

	public:
		// 画像オブジェクト取得
		virtual SGLImageBufferInterface *
			CommitImageObject
				( uint32_t idType, SGLImageRect& rectRef, bool fNoRefImage ) ;
		// 画像オブジェクト追加登録
		virtual bool AddImageObject
			( SGLImageBufferInterface * pObject, bool fOriginalImage ) ;
		// 画像オブジェクト分離
		virtual bool DetachImageObject( SGLImageBufferInterface * pObject ) ;
		// 画像オブジェクト更新通知
		virtual SGLError UpdateImageObject( const SGLImageRect * pUpdateRect = NULL ) ;
		// 画像オブジェクト更新確定
		virtual void FlushImageObject( void ) ;
		// 画像オブジェクト更新反映
		virtual SGLError ReflectImageObject( uint32_t idType ) ;
		// 画像オブジェクト削除
		virtual SGLError DeleteImageObjectTypeOf( uint32_t idType ) ;
		virtual SGLError DeleteAllImageObjects( void ) ;
		// 画像オブジェクトクリア通知
		virtual SGLError NotifyClearImageObject( SGLPalette pxClear ) ;
		// 内部バッファ取得
		virtual SGLImageBuffer * GetImageBuffer( void ) const ;
	#if	defined(__COTOPHA__)
		// 画像オブジェクト取得
		virtual Image * GetImageObject( void ) const ;
	#endif

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 複数画像オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	SGLMultiImage	: public SGLSmartImage
	{
	protected:
		SSystem::SPointerArray<SGLImageBuffer>	m_paImages ;
		SSystem::SArray<uint32_t>				m_tblSequence ;
		uint64_t								m_msecLong ;
		size_t									m_iSelFrame ;
		int										m_iSelFrameSide ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLMultiImage, SGLSmartImage )
		// 構築関数
		SGLMultiImage( void ) ;
		SGLMultiImage
			( const SGLMultiImage& img,
				const SGLImageRect * pClip = NULL,
				int iSide = stereoImageBoth ) ;
		// 消滅関数
		virtual ~SGLMultiImage( void ) ;
		// 参照複製
		void CreateReferenceFrom
			( const SGLMultiImage& img,
				const SGLImageRect * pClip = NULL,
				int iSide = stereoImageBoth ) ;

	public:
		// フレーム数取得
		virtual size_t GetFrameCount( void ) const ;
		// アニメーション・シーケンス全長取得
		virtual size_t GetSequenceLength( void ) const ;
		// アニメーション・シーケンス取得
		virtual size_t GetSequenceTable( uint32_t * pSeq, size_t nCount ) const ;
		// アニメーション全長（時間）取得 [msec]
		virtual uint64_t GetTotalTime( void ) const ;
		// 時間からフレーム番号へ変換
		virtual size_t FrameFromMilliSec( uint64_t msec ) ;
		// フレーム選択
		virtual SGLError SelectFrame( size_t iFrame, int iSide = stereoImageRight ) ;
		// 選択中フレーム取得
		virtual size_t GetSelectedFrame( int * pSide = NULL ) const ;
		// 画像バッファ読み込み
		virtual SGLError ReadFrameBuffer
			( const SGLImageInfo & imginf, uint8_t * ptrBuffer,
					size_t iFrame = 0, int iSide = stereoImageRight ) ;

	public:
		// 画像バッファへの参照生成
		virtual SGLImageObject * NewReference
			( const SGLImageRect * pClip = NULL,
					ssize_t iFrame = -1, int iSide = stereoImageRight ) ;
		// テクスチャのための正規化
		virtual SGLError NormalizeToTexture( uint32_t nFlags = 0 ) ;
		virtual SGLError NormalizeToMipmapTexture( uint32_t nFlags = 0 ) ;
		// レンダリング・ターゲットのための正規化
		virtual SGLError NormalizeToRenderTarget( uint32_t nFlags = 0 ) ;
		// テクスチャのための正規化フラグを除去する
		virtual SGLError DenormalizeForTexture( uint32_t nFlags ) ;
		// 画像バッファ生成
		virtual SGLError CreateBuffer
			( const SGLImageInfo& imginf,
				uint64_t nFlags = bufferOnMemory,
				size_t countFrame = 1, uint64_t msecLong = 0 ) ;
		// 保有リソース解放
		virtual void ReleaseBuffer( void ) ;
		// アニメーション・シーケンス・テーブルの設定
		virtual void SetSequenceTable( const uint32_t * pSeq, size_t nCount ) ;
		// アニメーション全長時間設定
		virtual void SetAnimationDuration( uint64_t nDuration ) ;

	public:
		// 画像フォーマット正規化
		virtual SGLError NormalizeFormat
			( uint32_t format = 0, uint32_t depth = 0,
				uint32_t nFlags = 0,
				uint32_t width = 0, uint32_t height = 0 ) ;
	protected:
		struct	ImageRefUnit
		{
			SGLImageRect	rectRef ;
			size_t			iFrame ;
		} ;
		class	ImageRefArray	: public SSystem::SArray<ImageRefUnit>
		{
		public:
			SGLSize			m_sizeMerge ;
			SGLImageRect	m_rectCurrent ;
		} ;
		void FlushMergeAnimationBy
			( ImageRefArray& ira,
				uint32_t format, uint32_t depth, int nBufFlags ) ;
	public:
		// NormalizeFormat で結合されたアニメーション画像への参照を生成
		virtual SGLImageObject * NewAnimationReference
			( SGLImageRect* pFrameRects, size_t nRectsCount ) ;

	public:
		// アトラス化された画像の参照矩形を取得
		virtual SGLError GetReferenceRectOfAtlas
			( SGLImageRect& rect, ssize_t iFrame = -1 ) const ;
		// 参照元画像情報取得
		virtual SGLImageObject * GetImageReference
					( SGLImageRect& rectRef, ssize_t iFrame = -1 ) ;
		// 参照先画像変更
		virtual SGLError MakeImageReference
			( size_t iFrame,
				SGLImageObject * pAtlasImage, const SGLImageRect& rectRef ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// 画像オブジェクト・スマート参照バッファ
	//////////////////////////////////////////////////////////////////////////

	class	SGLImageSmartBuffer
	{
	protected:
		SGLImageObject *	m_pImage ;
		int					m_flags ;

	public:
		// 構築関数
		SGLImageSmartBuffer( void ) ;
		SGLImageSmartBuffer
			( SGLImageBuffer& imgbuf,
				SGLImageObject * pImage,
				int flags = SGLImageObject::lockReadWrite,
				const SGLImageRect * pRect = NULL ) ;
		// 消滅関数
		~SGLImageSmartBuffer( void ) ;
		// バッファ参照
		const SGLImageBuffer& Lock
			( SGLImageBuffer& imgbuf,
				SGLImageObject * pImage,
				int flags = SGLImageObject::lockReadWrite,
				const SGLImageRect * pRect = NULL ) ;
		// バッファ参照解放
		void Unlock( void ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 画像サンプラー
	//////////////////////////////////////////////////////////////////////////

	class	SGLImageSampler
	{
	public:
		struct	SampleCoord
		{
			uint32_t	x0 ;
			uint32_t	y0 ;
			uint32_t	x1 ;
			uint32_t	y1 ;
			uint32_t	dx ;
			uint32_t	dy ;
		} ;

	protected:
		SGLImageInfo		m_infImage ;
		const uint8_t *		m_pbytImage ;
		uint32_t			m_xClamp ;
		uint32_t			m_yClamp ;
		uint32_t			m_xMask ;
		uint32_t			m_yMask ;

	public:
		// 構築
		SGLImageSampler( void )
			: m_pbytImage( NULL ),
				m_xClamp( 0 ), m_yClamp( 0 ), m_xMask( 0 ), m_yMask( 0 ) { }
		SGLImageSampler( const SGLImageBuffer& imgbuf )
		{
			Prepare( imgbuf, imgbuf.ptrBuffer ) ;
		}
		SGLImageSampler( const SGLImageInfo& imginf, const uint8_t * pbytImage )
		{
			Prepare( imginf, pbytImage ) ;
		}
		// 準備
		SGLError Prepare( const SGLImageInfo& imginf, const uint8_t * pbytImage )
		{
			m_infImage = imginf ;
			m_pbytImage = pbytImage ;
			m_xClamp = imginf.width - 1 ;
			m_yClamp = imginf.height - 1 ;
			m_xMask = sglNormalizeScalePowerBy2( imginf.width ) - 1 ;
			m_yMask = sglNormalizeScalePowerBy2( imginf.height ) - 1 ;
			if ( m_xMask > m_xClamp )
			{
				m_xMask >>= 1 ;
			}
			if ( m_yMask > m_yClamp )
			{
				m_yMask >>= 1 ;
			}
			ESLAssert( imginf.depth == 32 ) ;
			return	(imginf.depth == 32) ? sglErrSuccess : sglErrFailed ;
		}

	public:
		// 座標クリップ
		inline void ClampCoord( SampleCoord& sc, float32_t x, float32_t y ) const
		{
			int	fxx = esl_roundfi( x * 256.0f ) ;
			int	fxy = esl_roundfi( y * 256.0f ) ;
			int	ix = fxx >> 8 ;
			int	iy = fxy >> 8 ;
			sc.dx = (uint32_t) fxx & 0xFF ;
			sc.dy = (uint32_t) fxy & 0xFF ;
			sc.x0 = (uint32_t) ix ;
			sc.y0 = (uint32_t) iy ;
			if ( (uint32_t) ix > m_xClamp )
			{
				sc.x0 = m_xClamp & ~(ix >> (sizeof(int) * 8 - 1)) ;
			}
			if ( (uint32_t) iy > m_yClamp )
			{
				sc.y0 = m_yClamp & ~(iy >> (sizeof(int) * 8 - 1)) ;
			}
			++ ix ;
			++ iy ;
			sc.x1 = (uint32_t) ix ;
			sc.y1 = (uint32_t) iy ;
			if ( (uint32_t) ix > m_xClamp )
			{
				sc.x1 = m_xClamp & ~(ix >> (sizeof(int) * 8 - 1)) ;
			}
			if ( (uint32_t) iy > m_yClamp )
			{
				sc.y1 = m_yClamp & ~(iy >> (sizeof(int) * 8 - 1)) ;
			}
		}
		// 座標ループ
		inline void WrapCoord( SampleCoord& sc, float32_t x, float32_t y ) const
		{
			int	fxx = esl_roundfi( x * 256.0f ) ;
			int	fxy = esl_roundfi( y * 256.0f ) ;
			int	ix = fxx >> 8 ;
			int	iy = fxy >> 8 ;
			sc.dx = (uint32_t) fxx & 0xFF ;
			sc.dy = (uint32_t) fxy & 0xFF ;
			sc.x0 = (uint32_t) ix & m_xMask ;
			sc.y0 = (uint32_t) iy & m_yMask ;
			sc.x1 = (uint32_t) (ix + 1) & m_xMask ;
			sc.y1 = (uint32_t) (iy + 1) & m_yMask ;
		}

	public:
		// 単純サンプリング
		inline SGLPalette PointSampling( const SampleCoord& sc ) const
		{
			return	((SGLPalette*)(m_pbytImage + sc.y0 * m_infImage.pitchLine))[sc.x0] ;
		}
		// バイリニア補完サンプリング
		inline SGLPalette BilinearSampling( const SampleCoord& sc ) const
		{
			const unsigned int	dx = (unsigned int) sc.dx ;
			const unsigned int	ndx = 0x100U - dx ;
			const unsigned int	dy = (unsigned int) sc.dy ;
			const unsigned int	ndy = 0x100U - dy ;
			const SGLPalette *	ppxLine0 =
					(SGLPalette*) (m_pbytImage + sc.y0 * m_infImage.pitchLine) ;
			const SGLPalette *	ppxLine1 =
					(SGLPalette*) (m_pbytImage + sc.y1 * m_infImage.pitchLine) ;
			//
			SGLPalette	px0 = ppxLine0[sc.x0].imul(ndx)
							+ ppxLine0[sc.x1].imul(dx) ;
			SGLPalette	px1 = ppxLine1[sc.x0].imul(ndx)
							+ ppxLine1[sc.x1].imul(dx) ;
			return	px0.imul(ndy) + px1.imul(dy) ;
		}

	public:
		// 座標クリップ・単純サンプリング
		inline SGLPalette ClampedPoint( float32_t x, float32_t y ) const
		{
			SampleCoord	sc ;
			ClampCoord( sc, x, y ) ;
			return	PointSampling( sc ) ;
		}
		// 座標ループ・単純サンプリング
		inline SGLPalette WrappedPoint( float32_t x, float32_t y ) const
		{
			SampleCoord	sc ;
			WrapCoord( sc, x, y ) ;
			return	PointSampling( sc ) ;
		}
		// 座標クリップ・バイリニア補完サンプリング
		inline SGLPalette ClampedBilinear( float32_t x, float32_t y ) const
		{
			SampleCoord	sc ;
			ClampCoord( sc, x, y ) ;
			return	BilinearSampling( sc ) ;
		}
		// 座標ループ・バイリニア補完サンプリング
		inline SGLPalette WrappedBilinear( float32_t x, float32_t y ) const
		{
			SampleCoord	sc ;
			WrapCoord( sc, x, y ) ;
			return	BilinearSampling( sc ) ;
		}

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 二次元領域アロケーター
	//////////////////////////////////////////////////////////////////////////

	class	SGLAreaAllocator	: public ESLObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLAreaAllocator, ESLObject )
		// 構築関数
		SGLAreaAllocator( void ) ;
		// 消滅関数
		virtual ~SGLAreaAllocator( void ) ;

	public:
		enum	Flags
		{
			flagSizePoweredBy2	= 0x0001,	// 全体領域を２の累乗へ正規化する
		} ;

	protected:
		struct	Local	: public SGLImageRect
		{
			SGLImageRect	m_rectExt ;		// 外接領域
			Local *			m_pParent ;		// 親領域
			Local *			m_pChild ;		// 左上領域
			Local *			m_pRight ;		// 隣接（右）領域
			Local *			m_pUnder ;		// 隣接（下）領域
		} ;
		uint32_t	m_nFlags ;
		SGLSize		m_sizeInit ;
		SGLSize		m_sizeLimit ;
		Local *		m_pRoot ;
		Local *		m_pLast ;

	public:
		// 領域確保
		SGLImageRect * Allocate( int w, int h ) ;
		// 領域一括確保
		size_t BatchAllocate
			( SGLImageRect** ppRects,
				const SGLSize * pSizes, size_t nCount,
				size_t * pAllocaedIndexes = nullptr ) ;
		// 全領域解放
		void FreeAll( void ) ;
		// 全体サイズ取得
		SGLSize GetTotalSize( void ) const ;
		// フラグ取得
		uint32_t GetFlags( void ) const ;
		// フラグ設定
		void SetFlags( uint32_t nFlags ) ;
		// 全体初期サイズ設定
		void SetInitialSize( int w, int h ) ;
		// 全体限界サイズ設定
		void SetLimitSize( int w, int h ) ;

	protected:
		// 指定サイズローカル領域生成
		Local * CreateLocalArea( int x, int y, int w, int h, Local * pParent ) ;
		// サブ領域確保
		void AllocateLocalArea( Local * pLocal, int w, int h ) ;
		// サブ領域確保（反復）
		Local * AllocateSubLocal( Local * pLocal, int w, int h ) ;
		// 領域解放
		void FreeLocal( Local * pLocal ) ;

	} ;

}

#endif

