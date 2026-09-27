
#if	!defined(__SAKURAGL_ANDROID_IMAGE_DECODER_H__)
#define	__SAKURAGL_ANDROID_IMAGE_DECODER_H__	1

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// Android BitmapFactory デコーダー
	//////////////////////////////////////////////////////////////////////////

	class	SGLAndroidImageDecoder	: public SGLImageDecoderInterface
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SGLAndroidImageDecoder, SGLImageDecoderInterface )
		// 構築関数
		SGLAndroidImageDecoder( void ) ;
		// 消滅関数
		virtual ~SGLAndroidImageDecoder( void ) ;
		// ファイル拡張子判定
		virtual bool IsMatchableFileExtension( const wchar_t * pszExt ) ;
		// MIME 判定
		virtual bool IsMatchableMIMEType( const wchar_t * pszMIME ) ;
		// 画像読み込み
		virtual SGLError ReadImage
			( SGLImageObject & image,
				SSystem::SFileInterface & file, size_t nLimitFrames = 0 ) ;
	} ;

}

#endif

