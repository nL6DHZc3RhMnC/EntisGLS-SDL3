
#include <sakuraglx/sakuraglx.h>
#include <sakuragl/sgl2d_image.h>
#include <sakuragl/sgl_android_font.h>

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// Android フォント実装
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2( SakuraGL::SGLAndroidFont, SGLFontObject, JavaObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLAndroidFont::SGLAndroidFont( void )
{
	CreateJavaObject( ENTIS_GLS4_JAVA_PACKAGE "/FontRasterizer" ) ;
	MakeGlobalRef() ;
	//
	m_jmidSetStyle = GetMethodID( "setStyle", "(L" JAVA_LANG_STRING ";II)Z" ) ;
	m_jmidGetMetrics =
		GetMethodID( "getMetrics",
			"(C[B)L" ENTIS_GLS4_JAVA_PACKAGE "/FontRasterizer$Metrics;" ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLAndroidFont::~SGLAndroidFont( void )
{
}

// フォントオブジェクト生成
//////////////////////////////////////////////////////////////////////////////
SGLFontObject * SGLAndroidFont::NewFont( const SGLFontStyle& style )
{
	SGLAndroidFont *	pFont = new SGLAndroidFont ;
	pFont->SetStyle( style ) ;
	return	pFont ;
}

// スタイル設定
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAndroidFont::SetStyle( const SGLFontStyle& style )
{
	JNI::JavaObject	jobjStrFace ;
	if ( CallBooleanMethod
		( m_jmidSetStyle,
			jobjStrFace.CreateWideString( style.pszFace ),
									style.nSize, style.nStyles ) )
	{
		return	sglErrSuccess ;
	}
	return	sglErrFailed ;
}

// フォント情報取得・ラスタライズ
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAndroidFont::GetMetrics
	( uint8_t* pbytRasterized, size_t nBufBytes,
				SGLFontMetrics& metrics, uint32_t wch )
{
	JNI::JavaObject	jobjByteArray ;
	jbyteArray		jbarrBuf = NULL ;
	if ( pbytRasterized != NULL )
	{
		jbarrBuf = jobjByteArray.CreateByteArray( nBufBytes ) ;
	}
	JNI::JavaObject	jobjMetrics
		( CallObjectMethod( m_jmidGetMetrics, (jchar) wch, jbarrBuf ), true ) ;
	//
	metrics.nFlags = jobjMetrics.GetIntField( "nFlags" ) ;
	metrics.nAscent = jobjMetrics.GetIntField( "nAscent" ) ;
	metrics.nDescent = jobjMetrics.GetIntField( "nDescent" ) ;
	metrics.nLeading = jobjMetrics.GetIntField( "nLeading" ) ;
	metrics.nWidth = jobjMetrics.GetIntField( "nWidth" ) ;
	metrics.nHeight = jobjMetrics.GetIntField( "nHeight" ) ;
	metrics.rctExterior.x = jobjMetrics.GetIntField( "xExterior" ) ;
	metrics.rctExterior.y = jobjMetrics.GetIntField( "yExterior" ) ;
	metrics.rctExterior.w = jobjMetrics.GetIntField( "wExterior" ) ;
	metrics.rctExterior.h = jobjMetrics.GetIntField( "hExterior" ) ;
	//
	if ( pbytRasterized != NULL )
	{
		JNI::JByteArray	jbarrBufObj( jbarrBuf ) ;
		jbyte *	buf = jbarrBufObj.GetBuffer() ;
		for ( size_t i = 0; i < nBufBytes; i ++ )
		{
			pbytRasterized[i] = (uint8_t) buf[i] ;
		}
	}
	return	sglErrSuccess ;
}

// フォント読み込み
//////////////////////////////////////////////////////////////////////////////
void SGLAndroidFont::LoadFontFromAsset
	( const wchar_t * pwszFontName, const wchar_t * pwszFilePath )
{
	JNI::JSmartClass	jsclsFontManager
		( JNI::FindJavaClass( ENTIS_GLS4_JAVA_PACKAGE "/FontManager" ) ) ;
	jmethodID	jmidAddFontFromAsset =
		jsclsFontManager.GetStaticMethodID
			( "addFontFromAsset",
				"(L" JAVA_LANG_STRING ";L" JAVA_LANG_STRING ";)Z" ) ;
	JNI::JavaObject	jobjFontName, jobjFilePath ;
	if ( jsclsFontManager.CallStaticBooleanMethod
		( jmidAddFontFromAsset,
			jobjFontName.CreateWideString( pwszFontName ),
			jobjFilePath.CreateWideString( pwszFilePath ) ) )
	{
		SGLAndroidFont *	pFont = new SGLAndroidFont ;
		pFont->DetachJavaObject() ;
		//
		SGLFont::RegisterStockFont( pwszFontName, pFont ) ;
	}
}


