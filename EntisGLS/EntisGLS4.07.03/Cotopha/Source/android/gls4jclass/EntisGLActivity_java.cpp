
#include <sakuragl/sakuragl.h>
#include <esl/esl_java_object.h>
#include "EntisGLActivity_java.h"
#include "app/sakura2vm.h"


/*
 * Class:     com_entis_android_entisgls4_EntisGLActivity
 * Method:    nativeMain
 * Signature: (Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_com_entis_android_entisgls4_EntisGLActivity_nativeMain
  ( JNIEnv * env, jobject objThis, jstring jstrObjArg )
{
	int	codeExit = 0 ;
	SakuraGL::Initialize() ;
	{
		SSystem::SetMemoryAllocationMode( SSystem::mallocModeShared ) ;
		if ( sglStaticInitialize() == SakuraGL::sglErrSuccess )
		{
			SSystem::SetMemoryAllocationMode( SSystem::mallocModeGlobal ) ;
			//
			SSystem::SString	strArg ;
			if ( jstrObjArg != NULL )
			{
				JNI::JString	jstrArg( jstrObjArg, env ) ;
				jstrArg.ToString( strArg ) ;
			}
			codeExit = sglMain( strArg ) ;
			SSystem::Trace( "exit code = %d", codeExit ) ;
			//
			sglStaticFinalize() ;
		}
	}
	SakuraGL::Finalize() ;
	return	codeExit ;
}


/*
 * Class:     com_entis_android_entisgls4_EntisGLActivity
 * Method:    nativeAbort
 * Signature: ()I
 */
JNIEXPORT jint JNICALL Java_com_entis_android_entisgls4_EntisGLActivity_nativeAbort
  ( JNIEnv * env, jobject objThis )
{
	sglAbortVM() ;
	return	0 ;
}


