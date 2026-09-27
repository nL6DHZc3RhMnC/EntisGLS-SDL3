
#include <sakuragl/sakuragl.h>
#include <esl/esl_java_object.h>
#include "UICustomDialog_java.h"

using namespace SSystem ;


/*
 * Class:     com_entis_android_entisgls4_UICustomDialog
 * Method:    callbackItem
 * Signature: (Lcom/entis/android/entisgls4/UICustomDialogInterface/Item;I)Z
 */
JNIEXPORT jboolean JNICALL Java_com_entis_android_entisgls4_UICustomDialog_callbackItem
  ( JNIEnv * env, jobject objThis, jobject objItem, jint nCode )
{
	JNI::JavaObject		jobjThis( objThis, false, env ) ;
	JNI::JSmartObject	jsobjBuf
		( jobjThis.GetObjectField
			( jobjThis.GetFieldID
				( "m_bufInstance", "L" JAVA_NIO_BYTEBUFFER ";" ) ), env ) ;
	JNI::JDirectBuffer	jdbBuf( jsobjBuf.GetObject(), env ) ;
	//
	if ( objItem == NULL )
	{
		return	((SCustomDialog*)jdbBuf.GetBuffer())->OnCallbackItem( NULL, nCode ) ;
	}
	//
	JNI::JavaObject		jobjItem( objItem, false, env ) ;
	JNI::JSmartObject	jsobjBufItem
		( jobjItem.GetObjectField
			( jobjItem.GetFieldID
				( "m_bufInstance", "L" JAVA_NIO_BYTEBUFFER ";" ) ), env ) ;
	//
	if ( (jsobjBuf.GetObject() == NULL)
		|| (jsobjBufItem.GetObject() == NULL) )
	{
		return	false ;
	}
	JNI::JDirectBuffer	jdbBufItem( jsobjBufItem.GetObject(), env ) ;
	return	((SCustomDialog*)jdbBuf.GetBuffer())->OnCallbackItem
				( (SCustomDialog::ElementData*) jdbBufItem.GetBuffer(), nCode ) ;
}


