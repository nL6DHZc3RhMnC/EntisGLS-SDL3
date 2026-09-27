
#if	!defined(__SAKURAGLX_SPRITE_TEXT_H__)
#define	__SAKURAGLX_SPRITE_TEXT_H__	1

#include <sakuragl/sgl2d_image.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// テキスト表示スプライト
	//////////////////////////////////////////////////////////////////////////

	class	SGLSpriteText	: public SGLSprite
	{
	public:
		// ボックス・アライメント
		enum	BoxAlignment
		{
			alignBoxLeft		= 0x00,
			alignBoxRight		= 0x01,
			alignBoxCenter		= 0x02,
			alignBoxTop			= 0x00,
			alignBoxBottom		= 0x10,
			alignBoxVCenter		= 0x20,
			alignBoxHorzMask	= 0x0F,
			alignBoxVertMask	= 0xF0,
		} ;
		// テキストスタイル
		struct	TextStyle
		{
			uint32_t				boxAlign ;
			SGLFontStyle			font ;
			SGLLetteringContext		context ;
			SGLLetterer::Decoration	decoration ;

			// 構築関数（デフォルト値）
			TextStyle( void ) ;
			// 構築関数（複製）
			TextStyle( const TextStyle& style ) ;
			// 代入
			const TextStyle& operator = ( const TextStyle& style ) ;
			// シリアライズ（ポインタを除く）
			SGLError SaveWithoutPointer( SSystem::SFileInterface& file ) const ;
			SGLError LoadWithoutPointer( SSystem::SFileInterface& file ) ;
		} ;

	protected:
		SSystem::SSmartPointer<SGLImageObject>	m_pTextImage ;
		SGLPoint			m_ptTextUpperLeft ;

		SSystem::SString	m_strFontFace ;
		TextStyle			m_styleText ;

		SSystem::SString	m_strText ;

	public:
		// クラス情報
		SGL_DECLARE_CLASS_INFO( SGLSpriteText, SGLSprite )
		// 構築関数
		SGLSpriteText( void ) ;
		SGLSpriteText( const SGLSpriteText& src ) ;
		// 消滅関数
		virtual ~SGLSpriteText( void ) ;

	public:
		// テキストスタイル取得
		const TextStyle& GetTextStyle( void ) const
		{
			return	m_styleText ;
		}
		// テキストスタイル設定
		void SetTextStyle( const TextStyle& style ) ;

	public:
		// 外接矩形取得
		virtual bool GetRectangle( SGLRect& rectExt ) const ;
		// 文書表示領域取得
		bool GetTextRectangle( SGLRect& rectExt ) const ;

	public:
		// 文字列属性
		virtual SSystem::SString GetText( void ) const ;
		virtual void SetText( const wchar_t * pwszText ) ;
		// 文字フォント属性
		virtual void SetTextFont
			( const wchar_t * pwszFont, int nSize = 0 ) ;

	protected:
		// 文字画像を更新する
		void UpdateTextImage( void ) ;

	public:
		// 画像化したテキストを生成する
		static SGLImageObject * CreateTextImage
			( SGLPoint& ptUpperLeft,
				const TextStyle& style, const wchar_t * pwszText ) ;
		// テキストスタイルを解釈する
		static void ParseTextStyle
			( TextStyle& style, SSystem::SString& strFontFace,
							const SSystem::SXMLDocument& xmlStyle ) ;
		// フォントスタイルを解釈する
		static void ParseFontStyle
			( SGLFontStyle& style, SSystem::SString& strFontFace,
							const SSystem::SXMLDocument& xmlFont ) ;
		// 文字装飾スタイルを解釈する
		static void ParseTextDecoration
			( SGLLetterer::Decoration& deco,
				const SSystem::SXMLDocument& xmlStyle ) ;
		// 文字色を解釈する
		static void ParseTextColor
			( SGLPalette& rgbaText, const SSystem::SXMLDocument& xmlText ) ;
		// セミコロンで区切られた複数のフォントから有効なフォントを選ぶ
		static void SelectValidFont( SSystem::SString& strFontList ) ;

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
