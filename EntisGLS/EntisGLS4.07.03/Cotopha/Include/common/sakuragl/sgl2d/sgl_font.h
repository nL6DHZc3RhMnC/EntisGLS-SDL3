
#if	!defined(__SAKURAGL_FONT_H__)
#define	__SAKURAGL_FONT_H__

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// フォントスタイル
	//////////////////////////////////////////////////////////////////////////

	struct	SGLFontStyle
	{
		enum	FontStyle
		{
			styleItalic			= 0x00000001,	// 斜体
			styleBold			= 0x00000002,	// 太字
			styleNoSmooth		= 0x00010000,	// アンチエイリアス無効化
			styleHighDefinition	= 0x00020000,	// 高品位なアンチエイリアス
		} ;
		uint32_t		nStyles ;
		uint32_t		nSize ;
		const wchar_t *	pszFace ;

		// デフォルト・フォント名
		static const wchar_t *	StandardFont ;
		static const wchar_t *	FixedPitchFont ;

		// 構築関数
		#if	!defined(__COTOPHA__)
		SGLFontStyle( void ) : nStyles(0), nSize(0), pszFace(NULL) {}
		#endif
		SGLFontStyle( const SGLFontStyle& fs )
			: nStyles( fs.nStyles ),
				nSize( fs.nSize ), pszFace( fs.pszFace ) {}
		// 構造体変換
		#if	defined(__PLATFORM_WINDOWS__)
		void ToLogFont( LOGFONT& lf ) const ;
		void FromLogFont( const LOGFONT& lf, SSystem::SString& strFace ) ;
		#endif
		// 代入
		const SGLFontStyle& operator = ( const SGLFontStyle& fs )
		{
			nStyles = fs.nStyles ;
			nSize = fs.nSize ;
			pszFace = fs.pszFace ;
			return	*this ;
		}
		// シリアライズ（ポインタを除く）
		SSystem::SError SaveWithoutPointer( SSystem::SFileInterface& file ) const ;
		SSystem::SError LoadWithoutPointer( SSystem::SFileInterface& file ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// フォント表示情報
	//////////////////////////////////////////////////////////////////////////

	struct	SGLFontMetrics
	{
		uint32_t		nFlags ;
		int32_t			nAscent ;
		int32_t			nDescent ;
		int32_t			nLeading ;
		int32_t			nWidth ;
		int32_t			nHeight ;
		SGLImageRect	rctExterior ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// フォント抽象オブジェクト
	//////////////////////////////////////////////////////////////////////////

	#if	defined(__COTOPHA__)
	class	native Font
	{
	public:
		// フォントを列挙
		static native void EnumerateFonts
				( SSystem::SObjectArray<SSystem::SString>& listFonts ) ;
		// スタイル設定
		native SGLError SetStyle( const SGLFontStyle& style ) ;
		// フォント情報取得・ラスタライズ
		native SGLError GetMetrics
			( uint8_t* pbytRasterized, size_t nBufBytes,
						SGLFontMetrics& metrics, wchar_t wch ) ;

	} ;
	#endif

	class	SGLFontObject	: public SSystem::SObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLFontObject, SObject )
		// 構築関数
		SGLFontObject( void ) {}
		// 構築関数（ダミー）
		SGLFontObject( const SGLFontObject& font ) {}

	public:
		// フォントを列挙
		static void EnumerateFonts
				( SSystem::SObjectArray<SSystem::SString>& listFonts ) ;
		// フォントオブジェクト生成
		virtual SGLFontObject * NewFont( const SGLFontStyle& style ) ;
		// スタイル設定
		virtual SGLError SetStyle( const SGLFontStyle& style ) = 0 ;
		// フォント情報取得・ラスタライズ
		virtual SGLError GetMetrics
			( uint8_t* pbytRasterized, size_t nBufBytes,
						SGLFontMetrics& metrics, uint32_t wch ) = 0 ;
		// 文字画像取得
		SGLError GetFontImage
			( SGLImageObject * pCharImage,
				SGLFontMetrics& metrics,
				uint32_t wch, uint32_t rgbaColor = 0xFFFFFFFF ) ;
		// 文字列描画
		SGLError DrawText
			( SGLImageObject * pDstImage,
				int xPos, int yPos,
				const wchar_t * pwszText,
				uint32_t rgbaColor = 0xFFFFFFFF ) ;
	} ;

	#if	!defined(__COTOPHA__)
	typedef	SGLFontObject	Font ;
	#endif


	//////////////////////////////////////////////////////////////////////////
	// フォントオブジェクトラッパー
	//////////////////////////////////////////////////////////////////////////

	class	SGLFont	: public SGLFontObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLFont, SGLFontObject )
		// 構築関数
		SGLFont( void ) ;
		SGLFont( Font * pFont, bool flagOwner = false ) ;
		// 消滅関数
		virtual ~SGLFont( void ) ;

	protected:
		Font *	m_pFont ;
		bool	m_flagOwner ;

	public:
		// フォントを列挙
		static void EnumerateFonts
				( SSystem::SObjectArray<SSystem::SString>& listFonts ) ;
		// スタイル設定
		virtual SGLError SetStyle( const SGLFontStyle& style ) ;
		// フォント情報取得・ラスタライズ
		virtual SGLError GetMetrics
			( uint8_t* pbytRasterized, size_t nBufBytes,
						SGLFontMetrics& metrics, uint32_t wch ) ;

	protected:
		// フォント・リマップ・テーブル
		static ESL_DLL_EXPORT SSystem::SStrSortObjectArray<SSystem::SString> *	m_pFontRemap ;
		static ESL_DLL_EXPORT SSystem::SStrSortObjectArray<SGLFontObject> *	m_pFontStock ;

	public:
		// フォント・リマップ初期化
		static void InitializeRemapFontTable( void ) ;
		// フォント・リマップ全削除
		static void FinalizeRemapFontTable( void ) ;
		// フォント・リマップ登録
		static void RegisterRemapFont
			( const wchar_t * pwszName, const wchar_t * pwszRemapped ) ;
		// リマップ・フォント取得
		static const wchar_t * RemappedFontOf( const wchar_t * pwszName ) ;
		// 特殊フォント登録
		static void RegisterStockFont
			( const wchar_t * pwszName, SGLFontObject * pFontGenerator ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// Windows API フォント実装
	//////////////////////////////////////////////////////////////////////////

	#if	defined(__PLATFORM_WINDOWS__)
	class	SGLWindowsFont	: public SGLFontObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLWindowsFont, SGLFontObject )
		// 構築関数
		SGLWindowsFont( void ) ;
		// 消滅関数
		virtual ~SGLWindowsFont( void ) ;

	protected:
		HDC							m_hDC ;
		HFONT						m_hFont ;
		HFONT						m_hDefFont ;
		TEXTMETRIC					m_tmMetrics ;
		SSystem::SArray<uint8_t>	m_bufGlyphOutline ;
		bool						m_flagAntialias ;
		bool						m_flagMultiSampling ;

		static int CALLBACK EnumFontFamExProc
			( const LOGFONT *lplfe,
				const TEXTMETRIC *lptme,
				DWORD FontType, LPARAM lParam ) ;
		static void NormalizeGlyphOutlineGray16
			( uint8_t * pbytDst,
				size_t nWidth, size_t nHeight,
				const uint8_t * pbytSrc, const SGLImageRect& rectSrcBuf ) ;
		static void NormalizeGlyphOutlineGray8
			( uint8_t * pbytDst, const uint8_t * pbytSrc,
							size_t nWidth, size_t nHeight ) ;
		static void NormalizeGlyphOutlineGray1
			( uint8_t * pbytDst, const uint8_t * pbytSrc,
							size_t nWidth, size_t nHeight ) ;

	public:
		// フォントを列挙
		static void EnumerateFonts
				( SSystem::SObjectArray<SSystem::SString>& listFonts ) ;
		// スタイル設定
		virtual SGLError SetStyle( const SGLFontStyle& style ) ;
		// フォント情報取得・ラスタライズ
		virtual SGLError GetMetrics
			( uint8_t* pbytRasterized, size_t nBufBytes,
						SGLFontMetrics& metrics, uint32_t wch ) ;
	} ;
	#endif

}

#endif

