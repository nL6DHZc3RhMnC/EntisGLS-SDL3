
#include <sakuragl/sakuragl.h>
#include <sakuragl/sgl_media.h>
#include <esl/esl_java_object.h>
#include "NativeSoundPlayerListener_java.h"

using namespace SSystem ;
using namespace SakuraGL ;


/*
 * Class:     com_entis_android_entisgls4_NativeSoundPlayerListener
 * Method:    nativeOnStream
 * Signature: (Ljava/nio/ByteBuffer;Ljava/nio/ByteBuffer;)V
 */
JNIEXPORT void JNICALL Java_com_entis_android_entisgls4_NativeSoundPlayerListener_nativeOnStream
	( JNIEnv * env, jclass clsThis, jobject objListener, jobject objPlayer )
{
	JNI::JDirectBuffer	jdbListener( objListener, env ) ;
	JNI::JDirectBuffer	jdbPlayer( objPlayer, env ) ;
	SGLSoundPlayerListener *
		pListener = (SGLSoundPlayerListener*) jdbListener.GetBuffer() ;
	SGLSoundPlayerInterface *
		pPlayer = (SGLSoundPlayerInterface*) jdbPlayer.GetBuffer() ;
	if ( pListener && pPlayer )
	{
		pListener->OnStreaming( pPlayer ) ;
	}
}


