
#if	!defined(__SAKURAGLX_SPRITE_PROGRESS_BAR_H__)
#define	__SAKURAGLX_SPRITE_PROGRESS_BAR_H__	1

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// 進捗状況バー
	//////////////////////////////////////////////////////////////////////////

	class	SGLSpriteProgressBar	: public SGLSprite
	{
	public:
		// バースタイル
		enum	BarDirection
		{
			barHorz,
			barVert,
		} ;
		enum	BarType
		{
			frameLeft,
			frameWay,
			frameRight,
			barLeft,
			barWay,
			barRight,
			typeCount,
		} ;
		struct	BarStyle
		{
			BarDirection						typeBar ;
			SGLPoint							ptBarOffset ;
			SGLSkinManager::ImageDescription	imgdscParts[typeCount] ;

			// 構築関数（デフォルト値）
			BarStyle( void ) ;
			// 構築関数（複製）
			BarStyle( const BarStyle& style ) ;
			// 代入
			const BarStyle& operator = ( const BarStyle& style ) ;
		} ;

	protected:
		SSystem::SSmartPointer<SGLImageObject>		m_pFrameImage ;
		SSystem::SSmartPointer<SGLImageObject>		m_pBarImage ;
		SSystem::SSmartReference<SGLImageObject>	m_refFrame[typeCount] ;

		BarStyle	m_styleBar ;
		SGLSize		m_sizeBar ;
		int			m_rangeBar ;
		int			m_posBar ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( SGLSpriteProgressBar, SGLSprite )
		// 構築関数
		SGLSpriteProgressBar( void ) ;
		SGLSpriteProgressBar( const SGLSpriteProgressBar& src ) ;
		// 消滅関数
		virtual ~SGLSpriteProgressBar( void ) ;

	protected:
		// スプライト画像の描画処理
		virtual void DrawSprite
			( S3DRenderContextInterface& render,
				const SGLPaintParam& pp, SGLImageObject* image ) const ;

	public:
		// 外接矩形取得
		virtual bool GetRectangle( SGLRect& rectExt ) const ;

	public:
		// スクロール・トラック位置属性
		virtual int GetScrollPos
			( ScrollDirection scrlDir = scrollDefault ) ;
		virtual void SetScrollPos
			( int nPos, ScrollDirection scrlDir = scrollDefault ) ;
		// スクロール・トラック位置範囲属性
		virtual int GetScrollRange
			( ScrollDirection scrlDir = scrollDefault ) ;
		virtual void SetScrollRange
			( int nRange, ScrollDirection scrlDir = scrollDefault ) ;
		// コマンド処理
		virtual SGLError InvokeCommand
			( const SSystem::SXMLDocument& xmlCmd,
				SSystem::SXMLDocument * pxmlResult = NULL ) ;

	public:
		// フレームスタイル取得
		const BarStyle& GetBarStyle( void ) const
		{
			return	m_styleBar ;
		}
		// フレームスタイル設定
		void SetBarStyle( const BarStyle& style ) ;
		// フレーム矩形取得
		const SGLSize& GetBarSize( void ) const
		{
			return	m_sizeBar ;
		}
		// フレーム矩形設定
		void SetBarSize( const SGLSize& size ) ;

	protected:
		// 文字画像を更新する
		void UpdateBarImage( void ) ;

	public:
		// フレーム画像を生成する
		static SGLImageObject * CreateFrameImage
			( const BarStyle& style, const SGLSize& size ) ;
		// バー画像を生成する
		static SGLImageObject * CreateBarImage
			( const BarStyle& style, const SGLSize& size ) ;
		// バースタイルを解釈する
		static void ParseBarStyle
			( SGLSkinManager& skin,
				BarStyle& style, const SSystem::SXMLDocument& xmlStyle ) ;

	public:
		// 複製
		virtual SGLObject * DuplicateObject( void ) ;

	} ;

}

#endif
