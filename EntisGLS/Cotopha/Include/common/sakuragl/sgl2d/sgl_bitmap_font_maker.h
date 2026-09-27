
#if	!defined(__SAKURAGL_BITMAP_FONT_MAKER_H__)
#define	__SAKURAGL_BITMAP_FONT_MAKER_H__

#include <sakuragl/sgl2d/sgl_bitmap_font.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// 画像フォントファイル作成
	//////////////////////////////////////////////////////////////////////////

	class	SGLBitmapFontMaker	: public ESLObject
	{
	public:
		// リスナ
		class	Listener
		{
		public:
			virtual SGLError OnRender
				( SGLBitmapFontMaker * pMaker,
					wchar_t wch, size_t nCurrent, size_t nTotal ) = 0 ;
		} ;
		// フォントセット情報
		class	FontInfo
		{
		public:
			SSystem::SString	m_strFont ;			// フォントフェース名
			uint32_t			m_nStyle ;			// スタイル
			uint32_t			m_nSize ;			// サイズ
			uint32_t			m_nAsSize ;			// サイズ
			int					m_nOversampling ;	// オーバーサンプリング
			int					m_nGamma ;			// ガンマ補正
			bool				m_flagJISChar ;		// 8ビット以上は JIS 文字に限定
			bool				m_flag7bitChar ;	// 7ビット文字に限定
			SSystem::SString	m_strEx7bitChar ;	// 7ビット文字以外の追加文字
		public:
			FontInfo( void )
				: m_nStyle(0), m_nSize(16), m_nAsSize(16),
					m_nOversampling(1), m_nGamma(0x100),
					m_flagJISChar(true), m_flag7bitChar(false) {}
		} ;
		// 変換元例外フォント情報
		class	ExceptionCharacters
		{
		public:
			SSystem::SString	m_strFont ;
			SSystem::SString	m_strCharacters ;
		} ;

	protected:
		// 変換元フォント
		SSystem::SString		m_strOrgFont ;

		// 縦書き用への変換
		bool					m_flagForVertical ;	// 横書きを縦書き用に回転
		SSystem::SString		m_strNoRotate4V ;	// 縦書き用に回転しない文字
		SSystem::SString		m_strGeminate ;		// 促音や小さな文字（縦書きの時右の寄せる）

		static const wchar_t *	m_pwszDefNoRotate4V ;
		static const wchar_t *	m_pwszDefGeminate ;

		// フォントセット情報
		SSystem::SObjectArray<FontInfo>
								m_listFontSet ;

		// 例外フォント情報
		SSystem::SObjectArray<ExceptionCharacters>
								m_listException ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLBitmapFontMaker, ESLObject )
		// 構築関数
		SGLBitmapFontMaker( void ) ;

	public:
		// フォントファイルを作成
		SGLError MakeFontFile
			( const wchar_t * pwszFilePath, Listener * pListener ) ;

	public:
		// 設定XMLファイル読み込み
		SGLError LoadConfiguration( const wchar_t * pwszFilePath ) ;
		SGLError ParseConfiguration( const SSystem::SXMLDocument& xmlDoc ) ;
		// 設定XMLファイル保存
		SGLError SaveConfiguration( const wchar_t * pwszFilePath ) ;
		SGLError GetConfiguration( SSystem::SXMLDocument& xmlDoc ) ;

	public:
		// 変換元フォント設定
		void SetOriginalFont( const wchar_t * pwszFont ) ;
		// 変換元フォント取得
		const wchar_t * GetOriginalFont( void ) const ;
		// フォントセット追加
		void AddFontSet( const SGLFontStyle& style ) ;
		// フォントセットリスト取得
		const SSystem::SObjectArray<FontInfo> & GetFontSetArray( void ) const ;
		// フォントセットリスト全削除
		void RemoveAllFontSet( void ) ;
		// 例外フォント情報追加
		void AddException
			( const wchar_t * pwszFont, const wchar_t * pwszChar ) ;
		// 例外フォント取得
		const wchar_t * GetExceptionFont( wchar_t wch ) const ;
		// 例外フォント情報全削除
		void RemoveAllException( void ) ;
		// 例外フォント情報配列取得
		SSystem::SObjectArray<ExceptionCharacters> & GetExceptionArray( void ) ;

	} ;

}

#endif
