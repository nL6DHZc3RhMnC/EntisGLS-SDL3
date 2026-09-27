
#include <sakuragl/sakuragl.h>
#include <sakura/ssys_smart_buffer.h>
#include <sakuragl/sgl2d_image.h>
#include <sakuragl/sgl_android_image_decoder.h>

using namespace SSystem ;
using namespace SakuraGL ;

#define	JAVA_ANDROID_BITMAP			"android/graphics/Bitmap"
#define	JAVA_ANDROID_BITMAPFACTORY	"android/graphics/BitmapFactory"



//////////////////////////////////////////////////////////////////////////////
// Android BitmapFactory デコーダー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SGLAndroidImageDecoder, SGLImageDecoderInterface )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLAndroidImageDecoder::SGLAndroidImageDecoder( void )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLAndroidImageDecoder::~SGLAndroidImageDecoder( void )
{
}

// ファイル拡張子判定
//////////////////////////////////////////////////////////////////////////////
bool SGLAndroidImageDecoder::IsMatchableFileExtension( const wchar_t * pszExt )
{
	return	(SString::CompareNoCase(pszExt,L"png") == 0)
				|| (SString::CompareNoCase(pszExt,L"jpg") == 0)
				|| (SString::CompareNoCase(pszExt,L"jpeg") == 0) ;
}

// MIME 判定
//////////////////////////////////////////////////////////////////////////////
bool SGLAndroidImageDecoder::IsMatchableMIMEType( const wchar_t * pszMIME )
{
	return	(SString::CompareNoCase(pszMIME,L"image/png") == 0)
				|| (SString::CompareNoCase(pszMIME,L"image/jpeg") == 0) ;
}

// 画像読み込み
//////////////////////////////////////////////////////////////////////////////
SGLError SGLAndroidImageDecoder::ReadImage
	( SGLImageObject & image,
			SSystem::SFileInterface & file, size_t nLimitFrames )
{
	JNIEnv *	env = JNI::GetJNIEnv() ;
	//
	// ファイルを読み込む
	//
	JNI::JavaObject	jobjBufFile ;
	SSystem::SFileInterface *
				pImageFile = &file ;
	int64_t		nBytes = file.GetLength() ;
	SByteBuffer	sbuf ;
	if ( nBytes < 0 )
	{
		sbuf.ReadFromFile( file ) ;
		nBytes = sbuf.GetLength() ;
		pImageFile = &sbuf ;
	}
	{
		JNI::JByteArray	jbarrBufFile
			( jobjBufFile.CreateByteArray( (jsize) nBytes, env ) ) ;
		jbyte *	pbytBuf = jbarrBufFile.GetBuffer() ;
		file.Read( pbytBuf, (size_t) nBytes ) ;
		jbarrBufFile.ReleaseBuffer() ;
	}
	//
	// android.graphics.BitmapFactory.decodeByteArray 呼び出し
	//
	JNI::JSmartClass	jsclsBitmapFactory
		( JNI::FindJavaClass( JAVA_ANDROID_BITMAPFACTORY ) ) ;
	jmethodID	jmidDecodeByteArray =
			jsclsBitmapFactory.GetStaticMethodID
				( "decodeByteArray", "([BII)L" JAVA_ANDROID_BITMAP ";" ) ;
	//
	jobject	jobjDecoded =
		jsclsBitmapFactory.CallStaticObjectMethod
				( jmidDecodeByteArray,
					jobjBufFile.GetObject(), 0, (jint) nBytes ) ;
	if ( env->ExceptionOccurred() != NULL )
	{
		env->ExceptionClear() ;
		return	sglErrFailed ;
	}
	JNI::JavaObject	jobjBitmap ;
	jobjBitmap.AttachJavaObject( jobjDecoded, true, env ) ;
	if ( jobjBitmap.GetObject() == NULL )
	{
		return	sglErrFailed ;
	}
	jobjBufFile.DetachJavaObject() ;
	//
	// android.graphics.Bitmap.getWidth 呼び出し
	//
	jmethodID	jmidGetWidth =
					jobjBitmap.GetMethodID( "getWidth", "()I" ) ;
	jint	nWidth = jobjBitmap.CallIntMethod( jmidGetWidth ) ;
	//
	// android.graphics.Bitmap.getHeight 呼び出し
	//
	jmethodID	jmidGetHeight =
					jobjBitmap.GetMethodID( "getHeight", "()I" ) ;
	jint	nHeight = jobjBitmap.CallIntMethod( jmidGetHeight ) ;
	//
	// android.graphics.Bitmap.getPixels 呼び出し
	//
	JNI::JavaObject	jobjPixels ;
	jmethodID	jmidGetPixels =
					jobjBitmap.GetMethodID( "getPixels", "([IIIIIII)V" ) ;
	if ( env->ExceptionOccurred() != NULL )
	{
		env->ExceptionClear() ;
		return	sglErrFailed ;
	}
	jobjBitmap.CallVoidMethod
		( jmidGetPixels,
			jobjPixels.CreateIntArray
				( (jsize) (nWidth * nHeight), env ),
					0, nWidth, 0, 0, nWidth, nHeight ) ;
	env->ExceptionClear() ;
	//
	// ピクセルデータ取得
	//
	JNI::JIntArray	jiarrPixels( (jintArray) jobjPixels.GetObject(), env ) ;
	jint *	pPixels = jiarrPixels.GetBuffer() ;
	//
	SGLImageBuffer	imgSrc ;
	imgSrc.format = formatImageRGB ;
	imgSrc.depth = 32 ;
	imgSrc.width = (uint32_t) nWidth ;
	imgSrc.height = (uint32_t) nHeight ;
	imgSrc.pitchPixel = 4 ;
	imgSrc.pitchLine = (int32_t) (nWidth * 4) ;
	imgSrc.ptrBuffer = (uint8_t*) pPixels ;
	//
	jsize	countPixels = nWidth * nHeight ;
	for ( jsize i = 0; i < countPixels; i ++ )
	{
		if ( (pPixels[i] & 0xFF000000) != 0xFF000000 )
		{
			imgSrc.format =
				formatImageARGB | formatImageFlagNoProductOfAlpha ;
		}
	}
	//
	SGLImageBuffer	imgDst ;
	image.CreateImage( nWidth, nHeight, imgSrc.format ) ;
	//
	imgDst.ptrBuffer = image.LockBuffer( imgDst ) ;
	//
	sglCopyImageBuffer( imgDst, imgSrc ) ;
	//
	image.UnlockBuffer() ;
	jiarrPixels.ReleaseBuffer() ;
	//
	return	sglErrSuccess ;
}




