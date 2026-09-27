
#include <sakuragl/sakuragl.h>
#include <esl/esl_java_object.h>
#include <sakuraglx/sglx_std_app.h>
#include "EntisService_java.h"
#include "app/sakura2vm.h"

using namespace SSystem ;
using namespace SakuraGL ;


/*
 * Class:     com_entis_android_entisgls4_EntisService
 * Method:    onNativeServiceCreate
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_entis_android_entisgls4_EntisService_onNativeServiceCreate
  ( JNIEnv * env, jobject objThis )
{
	SakuraGL::Initialize() ;
	//
	SGLService *	pService = SGLService::GetService() ;
	if ( pService != NULL )
	{
		// 初回の startupService でのみ呼び出される
		pService->OnInitializeService() ;
		pService->OnStartService() ;
	}
}

/*
 * Class:     com_entis_android_entisgls4_EntisService
 * Method:    onNativeServiceDestroy
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_entis_android_entisgls4_EntisService_onNativeServiceDestroy
  ( JNIEnv * env, jobject objThis )
{
	SGLService *	pService = SGLService::GetService() ;
	if ( pService != NULL )
	{
		// サービス終了時に呼び出される
		pService->OnFinishService() ;
	}
	SakuraGL::Finalize() ;
}

/*
 * Class:     com_entis_android_entisgls4_EntisService
 * Method:    onNativeServiceStart
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_com_entis_android_entisgls4_EntisService_onNativeServiceStart
  ( JNIEnv * env, jobject objThis )
{
	SGLService *	pService = SGLService::GetService() ;
	if ( pService != NULL )
	{
		// startupService の度に複数回呼び出される
//		pService->OnStartService() ;
	}
}

/*
 * Class:     com_entis_android_entisgls4_EntisService
 * Method:    onNativeServiceAction
 * Signature: (Ljava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_com_entis_android_entisgls4_EntisService_onNativeServiceAction
  ( JNIEnv * env, jobject objThis, jstring strAction )
{
	SGLService *	pService = SGLService::GetService() ;
	if ( pService != NULL )
	{
		// アラームサービスで呼び出される
		JNI::JString	jstrAction( strAction ) ;
		SString			strBufAction ;
		jstrAction.ToString( strBufAction ) ;
		//
		pService->OnServiceTask( strBufAction ) ;
	}
}


