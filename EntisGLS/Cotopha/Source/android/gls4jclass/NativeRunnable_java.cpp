
#include <sakuragl/sakuragl.h>
#include <esl/esl_java_object.h>
#include "NativeRunnable_java.h"

using namespace	SSystem ;


/*
 * Class:     com_entis_android_entisgls4_NativeRunnable
 * Method:    nativeRun
 * Signature: (Ljava/nio/ByteBuffer;)V
 */
JNIEXPORT void JNICALL Java_com_entis_android_entisgls4_NativeRunnable_nativeRun
	( JNIEnv * env, jclass clsThis, jobject objBuf )
{
	JNI::JDirectBuffer	jdbBuf( objBuf, env ) ;
	SProcedure *	pProc = (SProcedure*) jdbBuf.GetBuffer() ;
	if ( pProc != NULL )
	{
		pProc->Prepare() ;
		pProc->Run() ;
		pProc->Finalize() ;
	}
}

