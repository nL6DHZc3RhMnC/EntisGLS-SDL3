
#if	!defined(__SAKURAGL_IMAGE_ENCODER_H__)
#define	__SAKURAGL_IMAGE_ENCODER_H__	1

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// 画像エンコーダーインターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SGLImageEncoderInterface	: public SSystem::SObject
	{
	public:
		enum	OptionFlag
		{
			optionQuality	= 0x0001,
		} ;
		struct	Options
		{
			uint32_t	nFlags ;
			uint32_t	nQuality ;		// [0,256] 0:lowest, 256:best quality
		} ;
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLImageEncoderInterface, SObject )
		// 構築関数（デフォルト）
		SGLImageEncoderInterface( void ) {}
		SGLImageEncoderInterface( const SGLImageEncoderInterface& encoder ) {}
		// ファイル拡張子判定
		virtual bool IsMatchableFileExtension
				( const wchar_t * pszExt, SSystem::SString& strMIME ) = 0 ;
		// MIME 判定
		virtual bool IsMatchableMIMEType( const wchar_t * pszMIME ) ;
		// 画像書き出し
		virtual SGLError WriteImage
			( SSystem::SFileInterface & file,
				SGLImageObject & image,
				const wchar_t * pszMIME,
				const SGLImageEncoderInterface::Options * pOpt = NULL ) = 0 ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 画像エンコーダー管理
	//////////////////////////////////////////////////////////////////////////

	class	SGLImageEncoderManager
	{
	public:
		// 画像エンコーダー配列
		static ESL_DLL_EXPORT SSystem::SObjectArray<SGLImageEncoderInterface> *	m_arrayImageEncoder ;

	public:
		// 初期化
		static void Initialzie( void ) ;
		// 終了
		static void Finalize( void ) ;
		// エンコーダー追加登録
		static void RegisterEncoder( SGLImageEncoderInterface * pEncoder ) ;
		// 拡張子が合致するエンコーダー取得
		static SGLImageEncoderInterface *
			FindEncoderAsExt( const wchar_t * pszExt, SSystem::SString& strMIME ) ;
		// MIME が合致するエンコーダー取得
		static SGLImageEncoderInterface * FindEncoderAsMIME( const wchar_t * pszMIME ) ;
		// 画像書き出し
		static SGLError WriteImage
			( SSystem::SFileInterface & file,
				SGLImageObject & image,
				const wchar_t * pszMIME,
				const SGLImageEncoderInterface::Options * pOpt = NULL ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// ERI 画像エンコーダー
	//////////////////////////////////////////////////////////////////////////

	class	SGLERImageEncoder	: public SGLImageEncoderInterface
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLERImageEncoder, SGLImageEncoderInterface )
		// ファイル拡張子判定
		virtual bool IsMatchableFileExtension
			( const wchar_t * pszExt, SSystem::SString& strMIME ) ;
		// MIME 判定
		virtual bool IsMatchableMIMEType( const wchar_t * pszMIME ) ;
		// 画像書き出し
		virtual SGLError WriteImage
			( SSystem::SFileInterface & file,
				SGLImageObject & image,
				const wchar_t * pszMIME,
				const SGLImageEncoderInterface::Options * pOpt = NULL ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// Windows Bitmap 画像エンコーダー
	//////////////////////////////////////////////////////////////////////////

	class	SGLWindowsBitmapEncoder	: public SGLImageEncoderInterface
	{
	public:
		struct	BITMAPFILEHEADER
		{
			DWORD	bfSize ;
			WORD	bfReserved1 ;
			WORD	bfReserved2 ;
			DWORD	bfOffBits ;
		} ;
		struct	BITMAPINFOHEADER
		{
			DWORD	biSize ;
			DWORD	biWidth ;
			DWORD	biHeight ;
			WORD	biPlanes ;
			WORD	biBitCount ;
			DWORD	biCompression ;
			DWORD	biSizeImage ;
			DWORD	biXPelsPerMeter ;
			DWORD	biYPelsPerMeter ;
			DWORD	biClrUsed ;
			DWORD	biClrImportant ;
		} ;
		struct	RGBQUAD
		{
			BYTE	rgbBlue ;
			BYTE	rgbGreen ;
			BYTE	rgbRed ;
			BYTE	rgbReserved ;
		} ;
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLWindowsBitmapEncoder, SGLImageEncoderInterface )
		// ファイル拡張子判定
		virtual bool IsMatchableFileExtension
			( const wchar_t * pszExt, SSystem::SString& strMIME ) ;
		// MIME 判定
		virtual bool IsMatchableMIMEType( const wchar_t * pszMIME ) ;
		// 画像書き出し
		virtual SGLError WriteImage
			( SSystem::SFileInterface & file,
				SGLImageObject & image,
				const wchar_t * pszMIME,
				const SGLImageEncoderInterface::Options * pOpt = NULL ) ;

	} ;


}

#endif

