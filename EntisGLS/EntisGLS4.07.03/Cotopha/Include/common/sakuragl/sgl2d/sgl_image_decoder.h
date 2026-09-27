
#if	!defined(__SAKURAGL_IMAGE_DECODER_H__)
#define	__SAKURAGL_IMAGE_DECODER_H__	1

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// 画像デコーダーインターフェース
	//////////////////////////////////////////////////////////////////////////

	class	SGLImageDecoderInterface	: public SSystem::SObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLImageDecoderInterface, SObject )
		// 構築関数（デフォルト）
		SGLImageDecoderInterface( void ) {}
		SGLImageDecoderInterface( const SGLImageDecoderInterface& decoder ) {}
		// ファイル拡張子判定
		virtual bool IsMatchableFileExtension( const wchar_t * pszExt ) ;
		// MIME 判定
		virtual bool IsMatchableMIMEType( const wchar_t * pszMIME ) ;
		// 画像読み込み
		virtual SGLError ReadImage
			( SGLImageObject & image,
				SSystem::SFileInterface & file, size_t nLimitFrames = 0 ) = 0 ;
		// 画像読み込み（縮小）
		virtual SGLError ReadPreviewImage
			( SGLImageObject & image, SSystem::SFileInterface & file ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 画像デコーダー管理
	//////////////////////////////////////////////////////////////////////////

	class	SGLImageDecoderManager
	{
	public:
		// 画像デコーダー配列
		static ESL_DLL_EXPORT SSystem::SObjectArray<SGLImageDecoderInterface> *	m_arrayImageDecoder ;

	public:
		// 初期化
		static void Initialzie( void ) ;
		// 終了
		static void Finalize( void ) ;
		// デコーダー追加登録
		static void RegisterDecoder( SGLImageDecoderInterface * pDecoder ) ;
		// 拡張子が合致するデコーダー取得
		static SGLImageDecoderInterface * FindDecoder( const wchar_t * pszExt ) ;
		// MIME が合致するデコーダー取得
		static SGLImageDecoderInterface * FindDecoderAsMIME( const wchar_t * pszMIME ) ;
		// 画像読み込み
		static SGLError ReadImage
			( SGLImageObject & image,
				SSystem::SFileInterface & file, size_t nLimitFrames = 0 ) ;
		// 画像読み込み（縮小）
		static SGLError ReadPreviewImage
			( SGLImageObject & image, SSystem::SFileInterface & file ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// Sakura2VM ネイティブ実装デコーダー
	//////////////////////////////////////////////////////////////////////////

	#if	defined(__COTOPHA__)
	class	SGLDefaultImageDecoder	: public SGLImageDecoderInterface
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLDefaultImageDecoder, SGLImageDecoderInterface )
		// ファイル拡張子判定
		virtual bool IsMatchableFileExtension( const wchar_t * pszExt ) ;
		// MIME 判定
		virtual bool IsMatchableMIMEType( const wchar_t * pszMIME ) ;
		// 画像読み込み
		virtual SGLError ReadImage
			( SGLImageObject & image,
				SSystem::SFileInterface & file, size_t nLimitFrames ) ;
	} ;
	#endif


	//////////////////////////////////////////////////////////////////////////
	// ERI 画像デコーダー
	//////////////////////////////////////////////////////////////////////////

	class	SGLERImageDecoder	: public SGLImageDecoderInterface
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLERImageDecoder, SGLImageDecoderInterface )
		// ファイル拡張子判定
		virtual bool IsMatchableFileExtension( const wchar_t * pszExt ) ;
		// MIME 判定
		virtual bool IsMatchableMIMEType( const wchar_t * pszMIME ) ;
		// 画像読み込み
		virtual SGLError ReadImage
			( SGLImageObject & image,
				SSystem::SFileInterface & file, size_t nLimitFrames ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// Windows Bitmap 画像デコーダー
	//////////////////////////////////////////////////////////////////////////

	class	SGLWindowsBitmapDecoder	: public SGLImageDecoderInterface
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
		ESL_DECLARE_CLASS_INFO( SGLWindowsBitmapDecoder, SGLImageDecoderInterface )
		// ファイル拡張子判定
		virtual bool IsMatchableFileExtension( const wchar_t * pszExt ) ;
		// MIME 判定
		virtual bool IsMatchableMIMEType( const wchar_t * pszMIME ) ;
		// 画像読み込み
		virtual SGLError ReadImage
			( SGLImageObject & image,	
				SSystem::SFileInterface & file, size_t nLimitFrames ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// TGA 画像デコーダー
	//////////////////////////////////////////////////////////////////////////

	class	SGLTGAImageDecoder	: public SGLImageDecoderInterface
	{
	public:
		struct	Word
		{
			uint8_t	l ;
			uint8_t	h ;

			uint16_t GetWord( void ) const
			{
				return	l + (((uint16_t)h) << 8) ;
			}
		} ;
		enum	TGAFormat
		{
			tgaIndexed		= 1,
			tgaRGB			= 2,
			tgaRLE_Flag		= 8,
			tgaIndexed_RLE	= tgaIndexed | tgaRLE_Flag,
			tgaRGB_RLE		= tgaRGB | tgaRLE_Flag,
		} ;
		struct	FILEHEADER
		{
			uint8_t	lenIdFidld ;
			uint8_t	typeColormap ;
			uint8_t	typeFormat ;
			Word	orgColormap ;
			Word	lenColormap ;
			uint8_t	depthColormap ;
			Word	xOrigin ;
			Word	yOrigin ;
			Word	width ;
			Word	height ;
			uint8_t	depth ;
			uint8_t	descriptor ;
		} ;
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLTGAImageDecoder, SGLImageDecoderInterface )
		// ファイル拡張子判定
		virtual bool IsMatchableFileExtension( const wchar_t * pszExt ) ;
		// MIME 判定
		virtual bool IsMatchableMIMEType( const wchar_t * pszMIME ) ;
		// 画像読み込み
		virtual SGLError ReadImage
			( SGLImageObject & image,
				SSystem::SFileInterface & file, size_t nLimitFrames ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// PSD ファイルデコーダー
	//////////////////////////////////////////////////////////////////////////

	class	SGLPSDImageDecoder	: public SGLImageDecoderInterface
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLPSDImageDecoder, SGLImageDecoderInterface )
		// ファイル拡張子判定
		virtual bool IsMatchableFileExtension( const wchar_t * pszExt ) ;
		// MIME 判定
		virtual bool IsMatchableMIMEType( const wchar_t * pszMIME ) ;
		// 画像読み込み
		virtual SGLError ReadImage
			( SGLImageObject & image,
				SSystem::SFileInterface & file, size_t nLimitFrames ) ;
	} ;


	//////////////////////////////////////////////////////////////////////////
	// AVI ファイルデコーダー
	//////////////////////////////////////////////////////////////////////////

#if	defined(__PLATFORM_WINDOWS__)
	class	SGLAVIImageDecoder	: public SGLImageDecoderInterface
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLAVIImageDecoder, SGLImageDecoderInterface )
		// ファイル拡張子判定
		virtual bool IsMatchableFileExtension( const wchar_t * pszExt ) ;
		// MIME 判定
		virtual bool IsMatchableMIMEType( const wchar_t * pszMIME ) ;
		// 画像読み込み
		virtual SGLError ReadImage
			( SGLImageObject & image,
				SSystem::SFileInterface & file, size_t nLimitFrames ) ;
	} ;
#endif

}

#endif
