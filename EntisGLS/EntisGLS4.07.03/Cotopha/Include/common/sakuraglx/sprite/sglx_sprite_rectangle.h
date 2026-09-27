
#if	!defined(__SAKURAGLX_SPRITE_RECTANGLE_H__)
#define	__SAKURAGLX_SPRITE_RECTANGLE_H__	1

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// 矩形表示スプライト
	//////////////////////////////////////////////////////////////////////////

	class	SGLSpriteRectangle	: public SGLSprite
	{
	public:
		// 矩形スタイル
		struct	RectStyle
		{
			SGLSize		size ;
			SGLPalette	color ;

			// 構築関数（デフォルト値）
			RectStyle( void ) ;
			// 構築関数（複製）
			RectStyle( const RectStyle& style ) ;
			// 代入
			const RectStyle& operator = ( const RectStyle& style ) ;
		} ;

	public:
		// 矩形描画オブジェクト
		class	SGLRectDrawer	: public SGLSpriteDrawer
		{
		public:
			SGLPalette	m_argbFill ;
			SGLSize		m_sizeRect ;
		public:
			// クラス情報
			SGL_DECLARE_CLASS_INFO( SGLRectDrawer, SGLSpriteDrawer )
			// 描画
			virtual void Draw
				( S3DRenderContextInterface& render,
					const SGLPaintParam& pp, SGLImageObject* image ) ;
			// 描画域取得
			virtual bool GetRectangle
				( SGLImageRect& rectDraw, SGLImageObject* image ) const ;
			// 当たり判定
			virtual bool IsHitPointAt
				( SGLImageObject* image, double x, double y ) const ;
		} ;
	protected:
		RectStyle	m_style ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( SGLSpriteRectangle, SGLSprite )
		// 構築関数
		SGLSpriteRectangle( void ) ;
		SGLSpriteRectangle( const SGLSpriteRectangle& src ) ;
		// 消滅関数
		virtual ~SGLSpriteRectangle( void ) ;

	public:
		// 矩形スタイル設定
		void SetRectangleStyle( const RectStyle& style ) ;
		// 矩形サイズ設定
		void SetRectangleSize( ssize_t w, ssize_t h ) ;
		// 矩形塗りつぶし色設定
		void SetRectangleColor( SGLPalette color ) ;
		// 矩形サイズスタイル取得
		const RectStyle& GetRectStyle( void ) const
		{
			return	m_style ;
		}

	public:
		// 矩形スタイルを解釈する
		static void ParseRectStyle
			( RectStyle& style, const SSystem::SXMLDocument& xmlStyle ) ;

	public:
		// Loquaty 用クラス
		virtual const wchar_t * GetLQClassName( void ) const ;

	public:
		// 複製
		virtual SGLObject * DuplicateObject( void ) ;
		// シリアライズ
		virtual SGLError OnSave( SSystem::SFileInterface& file ) ;
		// 復元
		virtual SGLError OnRestore( SSystem::SFileInterface& file ) ;

	} ;

}

#endif
