
#include <sakuragl/sakuragl.h>
#include <esl/esl_java_object.h>
#include <sakuraglx/sglx_std_app.h>
#include "EntisServiceBootReceiver_java.h"

using namespace SSystem ;
using namespace SakuraGL ;


/*
 * Class:     com_entis_android_entisgls4_EntisServiceBootReceiver
 * Method:    onNativeDeviceBoot
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_entis_android_entisgls4_EntisServiceBootReceiver_onNativeDeviceBoot
  ( JNIEnv * env, jobject objThis )
{
	SakuraGL::Initialize() ;
	sglStaticInitialize() ;
}



