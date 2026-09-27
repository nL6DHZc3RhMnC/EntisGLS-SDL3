
#include <sakuragl/sakuragl.h>
#include <sakuragl/sgl_window.h>
#include <sakuragl/sgl_generic_window.h>
#include "VirtualWindow_java.h"

using namespace	SSystem ;
using namespace	SakuraGL ;


static SGLGenericWindow * GetWindowOf( JNIEnv * env, jobject objThis )
{
	JNI::JavaObject		jobjThis( objThis, false, env ) ;
	JNI::JSmartObject	jsobjBuf
		( jobjThis.GetObjectField
			( jobjThis.GetFieldID
				( "m_buf", "L" JAVA_NIO_BYTEBUFFER ";" ) ), env ) ;
	if ( jsobjBuf.GetObject() == NULL )
	{
		return	NULL ;
	}
	JNI::JDirectBuffer	jdbBuf( jsobjBuf.GetObject(), env ) ;
	return	(SGLGenericWindow*) jdbBuf.GetBuffer() ;
}


/*
 * Class:     com_entis_android_entisgls4_VirtualWindow
 * Method:    onSurfaceChanged
 * Signature: (II)V
 */
JNIEXPORT void JNICALL Java_com_entis_android_entisgls4_VirtualWindow_onSurfaceChanged
	( JNIEnv * env, jobject objThis, jint width, jint height )
{
	SGLGenericWindow *	pGenWnd = GetWindowOf( env, objThis ) ;
	if ( pGenWnd != NULL )
	{
		pGenWnd->OnSurfaceChanged( width, height ) ;
	}
}

/*
 * Class:     com_entis_android_entisgls4_VirtualWindow
 * Method:    draw
 * Signature: (Ljavax/microedition/khronos/opengles/GL10;)V
 */
JNIEXPORT void JNICALL Java_com_entis_android_entisgls4_VirtualWindow_draw
	( JNIEnv * env, jobject objThis, jobject gl )
{
	SGLGenericWindow *	pGenWnd = GetWindowOf( env, objThis ) ;
	if ( pGenWnd != NULL )
	{
		pGenWnd->OnDraw() ;
	}
}

/*
 * Class:     com_entis_android_entisgls4_VirtualWindow
 * Method:    onTouchedDown
 * Signature: (DDI)Z
 */
JNIEXPORT jboolean JNICALL Java_com_entis_android_entisgls4_VirtualWindow_onTouchedDown
	( JNIEnv * env, jobject objThis, jdouble x, jdouble y, jint id )
{
	SGLGenericWindow *	pGenWnd = GetWindowOf( env, objThis ) ;
	if ( pGenWnd != NULL )
	{
		return	pGenWnd->OnTouchedDown( x, y, id ) ;
	}
	return	false ;
}

/*
 * Class:     com_entis_android_entisgls4_VirtualWindow
 * Method:    onTouchedUp
 * Signature: (DDI)Z
 */
JNIEXPORT jboolean JNICALL Java_com_entis_android_entisgls4_VirtualWindow_onTouchedUp
	( JNIEnv * env, jobject objThis, jdouble x, jdouble y, jint id )
{
	SGLGenericWindow *	pGenWnd = GetWindowOf( env, objThis ) ;
	if ( pGenWnd != NULL )
	{
		return	pGenWnd->OnTouchedUp( x, y, id ) ;
	}
	return	false ;
}

/*
 * Class:     com_entis_android_entisgls4_VirtualWindow
 * Method:    onTouchMoved
 * Signature: (DDI)Z
 */
JNIEXPORT jboolean JNICALL Java_com_entis_android_entisgls4_VirtualWindow_onTouchMoved
	( JNIEnv * env, jobject objThis, jdouble x, jdouble y, jint id )
{
	SGLGenericWindow *	pGenWnd = GetWindowOf( env, objThis ) ;
	if ( pGenWnd != NULL )
	{
		return	pGenWnd->OnTouchedMoved( x, y, id ) ;
	}
	return	false ;
}

/*
 * Class:     com_entis_android_entisgls4_VirtualWindow
 * Method:    onKeyDown
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_com_entis_android_entisgls4_VirtualWindow_onKeyDown
	( JNIEnv * env, jobject objThis, jint key )
{
	SGLGenericWindow *	pGenWnd = GetWindowOf( env, objThis ) ;
	if ( pGenWnd != NULL )
	{
		return	pGenWnd->OnKeyDown( key ) ;
	}
	return	false ;
}

/*
 * Class:     com_entis_android_entisgls4_VirtualWindow
 * Method:    onKeyUp
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_com_entis_android_entisgls4_VirtualWindow_onKeyUp
	( JNIEnv * env, jobject objThis, jint key )
{
	SGLGenericWindow *	pGenWnd = GetWindowOf( env, objThis ) ;
	if ( pGenWnd != NULL )
	{
		return	pGenWnd->OnKeyUp( key ) ;
	}
	return	false ;
}

/*
 * Class:     com_entis_android_entisgls4_VirtualWindow
 * Method:    onChar
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_com_entis_android_entisgls4_VirtualWindow_onChar
	( JNIEnv * env, jobject objThis, jint code )
{
	SGLGenericWindow *	pGenWnd = GetWindowOf( env, objThis ) ;
	if ( pGenWnd != NULL )
	{
		return	pGenWnd->OnChar( code ) ;
	}
	return	false ;
}

/*
 * Class:     com_entis_android_entisgls4_VirtualWindow
 * Method:    onJoystickAxis
 * Signature: (FFFF)Z
 */
JNIEXPORT jboolean JNICALL Java_com_entis_android_entisgls4_VirtualWindow_onJoystickAxis
  ( JNIEnv * env, jobject objThis, jfloat xAxis, jfloat yAxis, jfloat zAxis, jfloat rzAxis )
{
	SGLGenericWindow *	pGenWnd = GetWindowOf( env, objThis ) ;
	if ( pGenWnd != NULL )
	{
		return	pGenWnd->OnJoystickAxis( xAxis, yAxis, zAxis, rzAxis ) ;
	}
	return	false ;
}

/*
 * Class:     com_entis_android_entisgls4_VirtualWindow
 * Method:    onTimer
 * Signature: ()Z
 */
JNIEXPORT jboolean JNICALL Java_com_entis_android_entisgls4_VirtualWindow_onTimer
	( JNIEnv * env, jobject objThis )
{
	SGLGenericWindow *	pGenWnd = GetWindowOf( env, objThis ) ;
	if ( pGenWnd != NULL )
	{
		pGenWnd->OnTimer() ;
		return	true ;
	}
	return	false ;
}

/*
 * Class:     com_entis_android_entisgls4_VirtualWindow
 * Method:    onSystemEvent
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_com_entis_android_entisgls4_VirtualWindow_onSystemEvent
	( JNIEnv * env, jobject objThis, jint event )
{
	SGLGenericWindow *	pGenWnd = GetWindowOf( env, objThis ) ;
	if ( pGenWnd != NULL )
	{
		switch ( event )
		{
		case	com_entis_android_entisgls4_VirtualWindow_SYS_EVENT_PAUSE:
			pGenWnd->OnSystemEvent( SysCommandId::AppSuspend ) ;
			return	true ;

		case	com_entis_android_entisgls4_VirtualWindow_SYS_EVENT_RESUME:
			pGenWnd->OnSystemEvent( SysCommandId::AppResume ) ;
			return	true ;

		case	com_entis_android_entisgls4_VirtualWindow_SYS_EVENT_DESTROY:
			pGenWnd->OnSystemEvent( SysCommandId::AppDestroy ) ;
			return	true ;
		}
	}
	return	false ;
}

/*
 * Class:     com_entis_android_entisgls4_VirtualWindow
 * Method:    onMenuCommand
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_com_entis_android_entisgls4_VirtualWindow_onMenuCommand
	( JNIEnv * env, jobject objThis, jint idMenu )
{
	SGLGenericWindow *	pGenWnd = GetWindowOf( env, objThis ) ;
	if ( pGenWnd != NULL )
	{
		return	pGenWnd->OnMenuCommand( idMenu ) ;
	}
	return	false ;
}


