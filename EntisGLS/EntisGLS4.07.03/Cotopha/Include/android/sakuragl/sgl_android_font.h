
#if	!defined(__SAKURAGL_ANDROID_FONT_H__)
#define	__SAKURAGL_ANDROID_FONT_H__

#include <esl/esl_java_object.h>

namespace	SakuraGL
{
	//////////////////////////////////////////////////////////////////////////
	// Android フォント実装
	//////////////////////////////////////////////////////////////////////////

	class	SGLAndroidFont	: public SGLFontObject, public JNI::JavaObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2( SGLAndroidFont, SGLFontObject, JavaObject )
		// 構築関数
		SGLAndroidFont( void ) ;
		// 消滅関数
		virtual ~SGLAndroidFont( void ) ;

	protected:
		jmethodID	m_jmidSetStyle ;
		jmethodID	m_jmidGetMetrics ;

	public:
		// フォントオブジェクト生成
		virtual SGLFontObject * NewFont( const SGLFontStyle& style ) ;
		// スタイル設定
		virtual SGLError SetStyle( const SGLFontStyle& style ) ;
		// フォント情報取得・ラスタライズ
		virtual SGLError GetMetrics
			( uint8_t* pbytRasterized, size_t nBufBytes,
						SGLFontMetrics& metrics, uint32_t wch ) ;

	public:
		// フォント読み込み
		static void LoadFontFromAsset
			( const wchar_t * pwszFontName, const wchar_t * pwszFilePath ) ;

	} ;

}

#endif
