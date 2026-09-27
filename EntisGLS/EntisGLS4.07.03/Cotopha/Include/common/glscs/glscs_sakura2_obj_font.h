
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/

#if	!defined(__GLSCS_SAKURA2_OBJECT_FONT_H__)
#define	__GLSCS_SAKURA2_OBJECT_FONT_H__

namespace	ECSSakura2
{
	//////////////////////////////////////////////////////////////////////////
	// Font オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	FontObject : public ECSSakura2::Object, public SakuraGL::SGLFont
	{
	public:
		struct	FONT_STYLE
		{
			uint32_t	nStyles ;
			uint32_t	nSize ;
			uint64_t	pszFace ;
		} ;
	protected:
		bool					m_fFontStyle ;
		SakuraGL::SGLFontStyle	m_styleFont ;
		SSystem::SString		m_strFontFace ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2( FontObject, ECSSakura2::Object, SGLFont )
		// 構築関数
		FontObject( void ) ;
		// 消滅関数
		virtual ~FontObject( void ) ;
		// スタイル設定
		virtual SakuraGL::SGLError
			SetStyle( const SakuraGL::SGLFontStyle& style ) ;

	public:
		// 実行時型名
		virtual const wchar_t * GetTypeName( void ) const ;
		// 保存処理
		virtual SSystem::SError SaveStatic
			( SSystem::SFileInterface * file,
				VirtualMachine * vm, Context * context ) ;
		// 復元処理
		virtual SSystem::SError LoadStatic
			( SSystem::SFileInterface * file,
				VirtualMachine * vm, Context * context ) ;

	} ;

}


//////////////////////////////////////////////////////////////////////////////
// Font スタブ
//////////////////////////////////////////////////////////////////////////////

// new SakuraGL::Font
ECS_LIB_DECLARE_EXPORT_NEW_OBJECT(SakuraGL_Font) ;

// SGLError Font::EnumerateFonts
//	( SSystem::SObjectArray<SSystem::SString>& listFonts ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Font_EnumerateFonts) ;

// SGLError SetStyle( const SGLFontStyle& style ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Font_SetStyle) ;

// SGLError GetMetrics
//	( uint8_t* pbytRasterized, size_t nBufBytes,
//				SGLFontMetrics& metrics, wchar_t wch ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SakuraGL_Font_GetMetrics) ;


#endif
