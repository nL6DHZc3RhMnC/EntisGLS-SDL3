
#include <sakura/sakura.h>
#include <esl/esl_java_object.h>

using namespace SSystem ;
using namespace JNI ;


JavaVM *	JNI::g_JavaVM = NULL ;

static jobject		g_clsClassLoader = NULL ;
static jmethodID	g_jmidFindClass = NULL ;


//////////////////////////////////////////////////////////////////////////////
// JNI エントリポイント
//////////////////////////////////////////////////////////////////////////////

JNIEXPORT jint JNI_OnLoad( JavaVM * vm, void * reserved )
{
	Trace( "JNI_OnLoad\n" ) ;
	JNI::g_JavaVM = vm ;

	JNIEnv *	env = JNI::GetJNIEnv() ;
	jclass		clsEntisGLS =
		env->FindClass( ENTIS_GLS4_JAVA_PACKAGE "/EntisGLS" ) ;
	jclass		clsClass = env->FindClass( "java/lang/Class" ) ;
	jclass		clsClssLoader = env->FindClass( "java/lang/ClassLoader" ) ;
	jmethodID	jmidGetClassLoader =
		env->GetMethodID
			( clsClass, "getClassLoader", "()Ljava/lang/ClassLoader;" ) ;
	g_clsClassLoader =
		env->NewGlobalRef
			( env->CallObjectMethod( clsEntisGLS, jmidGetClassLoader ) ) ;
	g_jmidFindClass =
		env->GetMethodID
			( clsClssLoader, "findClass",
						"(L" JAVA_LANG_STRING ";)Ljava/lang/Class;" ) ;

	return	JNI_VERSION_1_6 ;
}


// JNIEnv 取得
//////////////////////////////////////////////////////////////////////////////
JNIEnv * JNI::GetJNIEnv( void )
{
	JNIEnv *	env = NULL ;
	if ( g_JavaVM->GetEnv( (void**) &env, JNI_VERSION_1_6 ) != JNI_OK )
	{
		if ( g_JavaVM->AttachCurrentThread( &env, NULL ) != JNI_OK )
		{
			return	NULL ;
		}
	}
	return	env ;
}


// クラスオブジェクト検索
//////////////////////////////////////////////////////////////////////////////
jclass JNI::FindJavaClass( const char * pszClassSig )
{
	JNIEnv * env = JNI::GetJNIEnv() ;
	if ( env == NULL )
	{
		return	NULL ;
	}
	jclass	cls = env->FindClass( pszClassSig ) ;
	if ( cls != NULL )
	{
		return	cls ;
	}
	env->ExceptionClear() ;
	//
	JNI::JavaObject	jobjClassSig ;
	jclass	jcls =
		(jclass) (env->CallObjectMethod
				( g_clsClassLoader, g_jmidFindClass,
					jobjClassSig.CreateUTFString( pszClassSig, env ) ) ) ;
	if ( env->ExceptionOccurred() != NULL )
	{
		env->ExceptionClear() ;
		return	NULL ;
	}
	return	jcls ;
}



//////////////////////////////////////////////////////////////////////////
// Java クラスオブジェクト・ローカル参照解放クラス
//////////////////////////////////////////////////////////////////////////

// static メンバ取得
//////////////////////////////////////////////////////////////////////////
jobject JSmartClass::GetStaticObjectField( jfieldID jfid ) const
{
	JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
	return	env->GetStaticObjectField( m_obj, jfid ) ;
}

jboolean JSmartClass::GetStaticBooleanField( jfieldID jfid ) const
{
	JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
	return	env->GetStaticBooleanField( m_obj, jfid ) ;
}

jbyte JSmartClass::GetStaticByteField( jfieldID jfid ) const
{
	JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
	return	env->GetStaticByteField( m_obj, jfid ) ;
}

jchar JSmartClass::GetStaticCharField( jfieldID jfid ) const
{
	JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
	return	env->GetStaticCharField( m_obj, jfid ) ;
}

jshort JSmartClass::GetStaticShortField( jfieldID jfid ) const
{
	JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
	return	env->GetStaticShortField( m_obj, jfid ) ;
}

jint JSmartClass::GetStaticIntField( jfieldID jfid ) const
{
	JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
	return	env->GetStaticIntField( m_obj, jfid ) ;
}

jlong JSmartClass::GetStaticLongField( jfieldID jfid ) const
{
	JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
	return	env->GetStaticLongField( m_obj, jfid ) ;
}

jfloat JSmartClass::GetStaticFloatField( jfieldID jfid ) const
{
	JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
	return	env->GetStaticFloatField( m_obj, jfid ) ;
}

jdouble JSmartClass::GetStaticDoubleField( jfieldID jfid ) const
{
	JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
	return	env->GetStaticDoubleField( m_obj, jfid ) ;
}

// static メソッド呼び出し
//////////////////////////////////////////////////////////////////////////
jobject JSmartClass::CallStaticObjectMethod( jmethodID jmid, ... ) const
{
	va_list	vl ;
	va_start( vl, jmid ) ;
	JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
	ESLAssert( env != NULL ) ;
	ESLAssert( env->ExceptionOccurred() == NULL ) ;
	env->ExceptionClear() ;
	jobject	obj = env->CallStaticObjectMethodV( m_obj, jmid, vl ) ;
	ESLAssert( env->ExceptionOccurred() == NULL ) ;
	return	obj ;
}

jboolean JSmartClass::CallStaticBooleanMethod( jmethodID jmid, ... ) const
{
	va_list	vl ;
	va_start( vl, jmid ) ;
	JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
	ESLAssert( env != NULL ) ;
	ESLAssert( env->ExceptionOccurred() == NULL ) ;
	env->ExceptionClear() ;
	jboolean	b = env->CallStaticBooleanMethodV( m_obj, jmid, vl ) ;
	ESLAssert( env->ExceptionOccurred() == NULL ) ;
	return	b ;
}

jbyte JSmartClass::CallStaticByteMethod( jmethodID jmid, ... ) const
{
	va_list	vl ;
	va_start( vl, jmid ) ;
	JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
	ESLAssert( env != NULL ) ;
	ESLAssert( env->ExceptionOccurred() == NULL ) ;
	env->ExceptionClear() ;
	jbyte	b = env->CallStaticByteMethodV( m_obj, jmid, vl ) ;
	ESLAssert( env->ExceptionOccurred() == NULL ) ;
	return	b ;
}

jchar JSmartClass::CallStaticCharMethod( jmethodID jmid, ... ) const
{
	va_list	vl ;
	va_start( vl, jmid ) ;
	JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
	ESLAssert( env != NULL ) ;
	ESLAssert( env->ExceptionOccurred() == NULL ) ;
	env->ExceptionClear() ;
	jchar	c = env->CallStaticCharMethodV( m_obj, jmid, vl ) ;
	ESLAssert( env->ExceptionOccurred() == NULL ) ;
	return	c ;
}

jshort JSmartClass::CallStaticShortMethod( jmethodID jmid, ... ) const
{
	va_list	vl ;
	va_start( vl, jmid ) ;
	JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
	ESLAssert( env != NULL ) ;
	ESLAssert( env->ExceptionOccurred() == NULL ) ;
	env->ExceptionClear() ;
	jshort	s = env->CallStaticShortMethodV( m_obj, jmid, vl ) ;
	ESLAssert( env->ExceptionOccurred() == NULL ) ;
	return	s ;
}

jint JSmartClass::CallStaticIntMethod( jmethodID jmid, ... ) const
{
	va_list	vl ;
	va_start( vl, jmid ) ;
	JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
	ESLAssert( env != NULL ) ;
	ESLAssert( env->ExceptionOccurred() == NULL ) ;
	env->ExceptionClear() ;
	jint	i = env->CallStaticIntMethodV( m_obj, jmid, vl ) ;
	ESLAssert( env->ExceptionOccurred() == NULL ) ;
	return	i ;
}

jlong JSmartClass::CallStaticLongMethod( jmethodID jmid, ... ) const
{
	va_list	vl ;
	va_start( vl, jmid ) ;
	JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
	ESLAssert( env != NULL ) ;
	ESLAssert( env->ExceptionOccurred() == NULL ) ;
	env->ExceptionClear() ;
	jlong	l = env->CallStaticLongMethodV( m_obj, jmid, vl ) ;
	ESLAssert( env->ExceptionOccurred() == NULL ) ;
	return	l ;
}

jfloat JSmartClass::CallStaticFloatMethod( jmethodID jmid, ... ) const
{
	va_list	vl ;
	va_start( vl, jmid ) ;
	JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
	ESLAssert( env != NULL ) ;
	ESLAssert( env->ExceptionOccurred() == NULL ) ;
	env->ExceptionClear() ;
	jfloat	f = env->CallStaticFloatMethodV( m_obj, jmid, vl ) ;
	ESLAssert( env->ExceptionOccurred() == NULL ) ;
	return	f ;
}

jdouble JSmartClass::CallStaticDoubleMethod( jmethodID jmid, ... ) const
{
	va_list	vl ;
	va_start( vl, jmid ) ;
	JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
	ESLAssert( env != NULL ) ;
	ESLAssert( env->ExceptionOccurred() == NULL ) ;
	env->ExceptionClear() ;
	jdouble	d = env->CallStaticDoubleMethodV( m_obj, jmid, vl ) ;
	ESLAssert( env->ExceptionOccurred() == NULL ) ;
	return	d ;
}

void JSmartClass::CallStaticVoidMethod( jmethodID jmid, ... ) const
{
	va_list	vl ;
	va_start( vl, jmid ) ;
	JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
	ESLAssert( env != NULL ) ;
	ESLAssert( env->ExceptionOccurred() == NULL ) ;
	env->ExceptionClear() ;
	env->CallStaticVoidMethodV( m_obj, jmid, vl ) ;
	ESLAssert( env->ExceptionOccurred() == NULL ) ;
}



//////////////////////////////////////////////////////////////////////////////
// Java Object
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( JNI::JavaObject, ESLObject )

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
JavaObject::~JavaObject( void )
{
	JavaObject::DetachJavaObject() ;
}

// 関連付け
//////////////////////////////////////////////////////////////////////////////
void JavaObject::AttachJavaObject
	( jobject obj, bool refLocal, JNIEnv * env )
{
	DetachJavaObject() ;
	//
	m_env = env ;
	m_obj = obj ;
	m_refLocal = refLocal ;
	m_refGlobal = false ;
	//
	if ( obj && !env )
	{
		m_env = JNI::GetJNIEnv() ;
	}
}

// 参照解放
//////////////////////////////////////////////////////////////////////////////
void JavaObject::DetachJavaObject( void )
{
	if ( m_obj )
	{
		if ( m_refLocal )
		{
			ESLAssert( m_env != NULL ) ;
			m_env->DeleteLocalRef( m_obj ) ;
		}
		else if ( m_refGlobal )
		{
			JNI::GetJNIEnv()->DeleteGlobalRef( m_obj ) ;
		}
	}
	m_class.DetachObject() ;
	//
	m_env = NULL ;
	m_obj = NULL ;
	m_refLocal = false ;
	m_refGlobal = false ;
}

// Java クラス取得
//////////////////////////////////////////////////////////////////////////////
JSmartClass& JavaObject::GetClass( void )
{
	if ( m_class.GetObject() == NULL )
	{
		if ( m_obj != NULL )
		{
			JNIEnv *	env = JNI::GetJNIEnv() ;
			jclass	jcls = env->GetObjectClass( m_obj ) ;
			m_class.AttachObject( jcls, env ) ;
			if ( m_refGlobal )
			{
				m_class.MakeGlobalRef() ;
			}
		}
	}
	return	m_class ;
}

// グローバル参照へ変換する
//////////////////////////////////////////////////////////////////////////////
jobject JavaObject::MakeGlobalRef( void )
{
	if ( !m_refGlobal && m_obj )
	{
		jobject	gobj = NewGlobalRef() ;
		if ( m_refLocal )
		{
			ESLAssert( m_env != NULL ) ;
			m_env->DeleteLocalRef( m_obj ) ;
		}
		m_env = NULL ;
		m_obj = gobj ;
		m_refLocal = false ;
		m_refGlobal = true ;
		//
		if ( m_class.GetObject() )
		{
			m_class.MakeGlobalRef() ;
		}
	}
	return	m_obj ;
}

// Java オブジェクト生成
//////////////////////////////////////////////////////////////////////////////
jobject JavaObject::CreateJavaObject
	( jclass clsObj, const char * pszType, va_list vl, JNIEnv * env )
{
	if ( env == NULL )
	{
		env = JNI::GetJNIEnv() ;
	}
	if ( pszType == NULL )
	{
		pszType = "()V" ;
	}
	jmethodID	midConstructor =
					env->GetMethodID( clsObj, "<init>", pszType ) ;
	if ( midConstructor == NULL )
	{
		ESLTrace( "failed to get constructor" ) ;
		return	NULL ;
	}
	jobject	obj = env->NewObjectV( clsObj, midConstructor, vl ) ;
	//
	AttachJavaObject( obj, true, env ) ;
	//
	return	obj ;
}

jobject JavaObject::CreateJavaObject
	( const char * pszClassSig, const char * pszType, ... )
{
	va_list	vl ;
	va_start( vl, pszType ) ;
	//
	jclass	clsObj = JNI::FindJavaClass( pszClassSig ) ;
	if ( clsObj == NULL )
	{
		ESLTrace( "failed to FindClass %s", pszClassSig ) ;
		return	NULL ;
	}
	JSmartClass	jsClass( clsObj ) ;
	return	CreateJavaObject( clsObj, pszType, vl ) ;
}

// ローカル参照の自動解放をキャンセルし Java オブジェクトを取得する
//////////////////////////////////////////////////////////////////////////////
jobject JavaObject::DetachLocalRef( void )
{
	ESLAssert( !m_refGlobal ) ;
	m_refLocal = false ;
	m_refGlobal = false ;
	return	m_obj ;
}

// Java グローバル参照を生成する
//////////////////////////////////////////////////////////////////////////////
jobject JavaObject::NewGlobalRef( void )
{
	if ( (m_env != NULL) && (m_obj != NULL) )
	{
		return	m_env->NewGlobalRef( m_obj ) ;
	}
	return	NULL ;
}

// String 生成
//////////////////////////////////////////////////////////////////////////////
jstring JavaObject::CreateString
	( const jchar * pszText, jsize nLength, JNIEnv * env )
{
	ESLAssert( pszText != NULL ) ;
	if ( nLength == (jsize) -1 )
	{
		nLength = 0 ;
		if ( pszText != NULL )
		for ( int i = 0; pszText[i] != 0; i ++ )
		{
			nLength = i ;
		}
	}
	if ( env == NULL )
	{
		env = JNI::GetJNIEnv() ;
	}
	jstring	obj = env->NewString( pszText, nLength ) ;
	AttachJavaObject( obj, true, env ) ;
	return	obj ;
}

jstring JavaObject::CreateUTFString
	( const char * pszText, JNIEnv * env )
{
	ESLAssert( pszText != NULL ) ;
	if ( env == NULL )
	{
		env = JNI::GetJNIEnv() ;
	}
	jstring	obj = env->NewStringUTF( pszText ) ;
	AttachJavaObject( obj, true, env ) ;
	return	obj ;
}

jstring JavaObject::CreateWideString
	( const wchar_t * pwszText, jsize nLength, JNIEnv * env )
{
	ESLAssert( pwszText != NULL ) ;
	if ( nLength == (jsize) -1 )
	{
		nLength = 0 ;
		if ( pwszText != NULL )
		for ( int i = 0; pwszText[i] != 0; i ++ )
		{
			nLength = i + 1 ;
		}
	}
	SArray<jchar>	bufText ;
	jchar *			pszText ;
	bufText.SetLength( nLength ) ;
	pszText = bufText.GetArray() ;
	for ( jsize i = 0; i < nLength; i ++ )
	{
		pszText[i] = (jchar) (pwszText[i]) ;
	}
	return	CreateString( pszText, nLength, env ) ;
}


// boolean[] 生成
//////////////////////////////////////////////////////////////////////////////
jbooleanArray JavaObject::CreateBooleanArray( jsize nLength, JNIEnv * env )
{
	if ( env == NULL )
	{
		env = JNI::GetJNIEnv() ;
	}
	jbooleanArray	obj = env->NewBooleanArray( nLength ) ;
	AttachJavaObject( obj, true, env ) ;
	return	obj ;
}

// byte[] 生成
//////////////////////////////////////////////////////////////////////////////
jbyteArray JavaObject::CreateByteArray( jsize nLength, JNIEnv * env )
{
	if ( env == NULL )
	{
		env = JNI::GetJNIEnv() ;
	}
	jbyteArray	obj = env->NewByteArray( nLength ) ;
	AttachJavaObject( obj, true, env ) ;
	return	obj ;
}

// char[] 生成
//////////////////////////////////////////////////////////////////////////////
jcharArray JavaObject::CreateCharArray( jsize nLength, JNIEnv * env )
{
	if ( env == NULL )
	{
		env = JNI::GetJNIEnv() ;
	}
	jcharArray	obj = env->NewCharArray( nLength ) ;
	AttachJavaObject( obj, true, env ) ;
	return	obj ;
}

// short[] 生成
//////////////////////////////////////////////////////////////////////////////
jshortArray JavaObject::CreateShortArray( jsize nLength, JNIEnv * env )
{
	if ( env == NULL )
	{
		env = JNI::GetJNIEnv() ;
	}
	jshortArray	obj = env->NewShortArray( nLength ) ;
	AttachJavaObject( obj, true, env ) ;
	return	obj ;
}

// int[] 生成
//////////////////////////////////////////////////////////////////////////////
jintArray JavaObject::CreateIntArray( jsize nLength, JNIEnv * env )
{
	if ( env == NULL )
	{
		env = JNI::GetJNIEnv() ;
	}
	jintArray	obj = env->NewIntArray( nLength ) ;
	AttachJavaObject( obj, true, env ) ;
	return	obj ;
}

// long[] 生成
//////////////////////////////////////////////////////////////////////////////
jlongArray JavaObject::CreateLongArray( jsize nLength, JNIEnv * env )
{
	if ( env == NULL )
	{
		env = JNI::GetJNIEnv() ;
	}
	jlongArray	obj = env->NewLongArray( nLength ) ;
	AttachJavaObject( obj, true, env ) ;
	return	obj ;
}

// float[] 生成
//////////////////////////////////////////////////////////////////////////////
jfloatArray JavaObject::CreateFloatArray( jsize nLength, JNIEnv * env )
{
	if ( env == NULL )
	{
		env = JNI::GetJNIEnv() ;
	}
	jfloatArray	obj = env->NewFloatArray( nLength ) ;
	AttachJavaObject( obj, true, env ) ;
	return	obj ;
}

// double[] 生成
//////////////////////////////////////////////////////////////////////////////
jdoubleArray JavaObject::CreateDoubleArray( jsize nLength, JNIEnv * env )
{
	if ( env == NULL )
	{
		env = JNI::GetJNIEnv() ;
	}
	jdoubleArray	obj = env->NewDoubleArray( nLength ) ;
	AttachJavaObject( obj, true, env ) ;
	return	obj ;
}

// Object[] 生成
//////////////////////////////////////////////////////////////////////////////
jobjectArray JavaObject::CreateObjectArray
	( jsize nLength, jclass clsType, jobject objInitElement, JNIEnv * env )
{
	if ( env == NULL )
	{
		env = JNI::GetJNIEnv() ;
	}
	jobjectArray	obj =
		env->NewObjectArray( nLength, clsType, objInitElement ) ;
	AttachJavaObject( obj, true, env ) ;
	return	obj ;
}

// ByteBuffer 生成
//////////////////////////////////////////////////////////////////////////////
jobject JavaObject::CreateByteBuffer
	( void * ptrBuf, jlong nBytes, JNIEnv * env )
{
	ESLAssert( ptrBuf != NULL ) ;
	if ( env == NULL )
	{
		env = JNI::GetJNIEnv() ;
	}
	jobject	obj = env->NewDirectByteBuffer( ptrBuf, nBytes ) ;
	AttachJavaObject( obj, true, env ) ;
	return	obj ;
}

// メソッド呼び出し
//////////////////////////////////////////////////////////////////////////////
jobject JavaObject::CallObjectMethod( jmethodID jmid, ... ) const
{
	va_list	vl ;
	va_start( vl, jmid ) ;
	JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
	ESLAssert( env != NULL ) ;
	ESLAssert( env->ExceptionOccurred() == NULL ) ;
	env->ExceptionClear() ;
	jobject	obj = env->CallObjectMethodV( m_obj, jmid, vl ) ;
	ESLAssert( env->ExceptionOccurred() == NULL ) ;
	return	obj ;
}

jboolean JavaObject::CallBooleanMethod( jmethodID jmid, ... ) const
{
	va_list	vl ;
	va_start( vl, jmid ) ;
	JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
	ESLAssert( env != NULL ) ;
	ESLAssert( env->ExceptionOccurred() == NULL ) ;
	env->ExceptionClear() ;
	jboolean	b = env->CallBooleanMethodV( m_obj, jmid, vl ) ;
	ESLAssert( env->ExceptionOccurred() == NULL ) ;
	return	b ;
}

jbyte JavaObject::CallByteMethod( jmethodID jmid, ... ) const
{
	va_list	vl ;
	va_start( vl, jmid ) ;
	JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
	ESLAssert( env != NULL ) ;
	ESLAssert( env->ExceptionOccurred() == NULL ) ;
	env->ExceptionClear() ;
	jbyte	b = env->CallByteMethodV( m_obj, jmid, vl ) ;
	ESLAssert( env->ExceptionOccurred() == NULL ) ;
	return	b ;
}

jchar JavaObject::CallCharMethod( jmethodID jmid, ... ) const
{
	va_list	vl ;
	va_start( vl, jmid ) ;
	JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
	ESLAssert( env != NULL ) ;
	ESLAssert( env->ExceptionOccurred() == NULL ) ;
	env->ExceptionClear() ;
	jchar	c = env->CallCharMethodV( m_obj, jmid, vl ) ;
	ESLAssert( env->ExceptionOccurred() == NULL ) ;
	return	c ;
}

jshort JavaObject::CallShortMethod( jmethodID jmid, ... ) const
{
	va_list	vl ;
	va_start( vl, jmid ) ;
	JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
	ESLAssert( env != NULL ) ;
	ESLAssert( env->ExceptionOccurred() == NULL ) ;
	env->ExceptionClear() ;
	jshort	s = env->CallShortMethodV( m_obj, jmid, vl ) ;
	ESLAssert( env->ExceptionOccurred() == NULL ) ;
	return	s ;
}

jint JavaObject::CallIntMethod( jmethodID jmid, ... ) const
{
	va_list	vl ;
	va_start( vl, jmid ) ;
	JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
	ESLAssert( env != NULL ) ;
	ESLAssert( env->ExceptionOccurred() == NULL ) ;
	env->ExceptionClear() ;
	jint	i = env->CallIntMethodV( m_obj, jmid, vl ) ;
	ESLAssert( env->ExceptionOccurred() == NULL ) ;
	return	i ;
}

jlong JavaObject::CallLongMethod( jmethodID jmid, ... ) const
{
	va_list	vl ;
	va_start( vl, jmid ) ;
	JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
	ESLAssert( env != NULL ) ;
	ESLAssert( env->ExceptionOccurred() == NULL ) ;
	env->ExceptionClear() ;
	jlong	l = env->CallLongMethodV( m_obj, jmid, vl ) ;
	ESLAssert( env->ExceptionOccurred() == NULL ) ;
	return	l ;
}

jfloat JavaObject::CallFloatMethod( jmethodID jmid, ... ) const
{
	va_list	vl ;
	va_start( vl, jmid ) ;
	JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
	ESLAssert( env != NULL ) ;
	ESLAssert( env->ExceptionOccurred() == NULL ) ;
	env->ExceptionClear() ;
	jfloat	f = env->CallFloatMethodV( m_obj, jmid, vl ) ;
	ESLAssert( env->ExceptionOccurred() == NULL ) ;
	return	f ;
}

jdouble JavaObject::CallDoubleMethod( jmethodID jmid, ... ) const
{
	va_list	vl ;
	va_start( vl, jmid ) ;
	JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
	ESLAssert( env != NULL ) ;
	ESLAssert( env->ExceptionOccurred() == NULL ) ;
	env->ExceptionClear() ;
	jdouble	d = env->CallDoubleMethodV( m_obj, jmid, vl ) ;
	ESLAssert( env->ExceptionOccurred() == NULL ) ;
	return	d ;
}

void JavaObject::CallVoidMethod( jmethodID jmid, ... ) const
{
	va_list	vl ;
	va_start( vl, jmid ) ;
	JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
	ESLAssert( env != NULL ) ;
	ESLAssert( env->ExceptionOccurred() == NULL ) ;
	env->ExceptionClear() ;
	env->CallVoidMethodV( m_obj, jmid, vl ) ;
	ESLAssert( env->ExceptionOccurred() == NULL ) ;
}



