
#if	!defined(__ESL_JAVA_OBJECT_H__)
#define	__ESL_JAVA_OBJECT_H__	1

#include <jni.h>

#define	ENTIS_GLS_JAVA_PACKAGE	"com/entis/android/entisgls"
#define	ENTIS_GLS4_JAVA_PACKAGE	"com/entis/android/entisgls4"
#define	JAVA_LANG_OBJECT		"java/lang/Object"
#define	JAVA_LANG_STRING		"java/lang/String"
#define	JAVA_LANG_RUNNABLE		"java/lang/Runnable"
#define	JAVA_NIO_BYTEBUFFER		"java/nio/ByteBuffer"

namespace	JNI
{
	extern	JavaVM *	g_JavaVM ;

	// 現在のスレッドの JNIEnv を取得
	JNIEnv * GetJNIEnv( void ) ;
	// Java クラス検索
	jclass FindJavaClass( const char * pszClassSig ) ;


	//////////////////////////////////////////////////////////////////////////
	// Java クラスオブジェクト・ローカル（グローバル）参照解放クラス
	//////////////////////////////////////////////////////////////////////////

	template <class T> class	JavaSmartLocalRef
	{
	protected:
		JNIEnv *	m_env ;
		T			m_obj ;
		bool		m_refGlobal ;

	public:
		// 構築関数
		JavaSmartLocalRef( T obj = NULL, JNIEnv * env = NULL )
		{
			m_env = env ;
			m_obj = obj ;
			m_refGlobal = false ;
			if ( obj && !env )
			{
				m_env = JNI::GetJNIEnv() ;
			}
		}
		// 消滅関数
		~JavaSmartLocalRef( void )
		{
			DetachObject() ;
		}
		// 関連付け
		void AttachObject( T obj, JNIEnv * env = NULL )
		{
			DetachObject() ;
			m_env = env ;
			m_obj = obj ;
			m_refGlobal = false ;
			if ( obj && !env )
			{
				m_env = JNI::GetJNIEnv() ;
			}
		}
		void AttachGlobalRef( T obj )
		{
			DetachObject() ;
			m_env = NULL ;
			m_obj = obj ;
			m_refGlobal = true ;
		}
		const JavaSmartLocalRef<T>& operator = ( T obj )
		{
			AttachObject( obj, NULL ) ;
			return	*this ;
		}
		// 参照解放
		void DetachObject( void )
		{
			if ( m_obj )
			{
				JNIEnv *	env = m_env ;
				if ( m_refGlobal )
				{
					env = JNI::GetJNIEnv() ;
					ESLAssert( env != NULL ) ;
					env->DeleteGlobalRef( m_obj ) ;
				}
				else
				{
					ESLAssert( env != NULL ) ;
					if ( env != NULL )
					{
						env->DeleteLocalRef( m_obj ) ;
					}
				}
			}
			m_obj = NULL ;
		}
		// オブジェクト取得
		T GetObject( void ) const
		{
			return	m_obj ;
		}
		operator T ( void ) const
		{
			return	m_obj ;
		}
		// グローバル参照へ変換
		void MakeGlobalRef( void )
		{
			if ( !m_refGlobal && m_obj && m_env )
			{
				T	gobj = (T) m_env->NewGlobalRef( m_obj ) ;
				AttachGlobalRef( gobj ) ;
			}
		}
	} ;

	class	JSmartObject	: public JavaSmartLocalRef<jobject>
	{
	public:
		// 構築関数
		JSmartObject( jobject obj = NULL, JNIEnv * env = NULL )
			: JavaSmartLocalRef<jobject>( obj, env )
		{
		}
		const JSmartObject& operator = ( jobject obj )
		{
			AttachObject( obj, NULL ) ;
			return	*this ;
		}
	} ;

	class	JSmartClass	: public JavaSmartLocalRef<jclass>
	{
	public:
		// 構築関数
		JSmartClass( jclass obj = NULL, JNIEnv * env = NULL )
			: JavaSmartLocalRef<jclass>( obj, env )
		{
		}
		const JSmartClass& operator = ( jclass obj )
		{
			AttachObject( obj, NULL ) ;
			return	*this ;
		}
		// 静メンバID取得
		jfieldID GetStaticFieldID( const char * pszName, const char * pszType )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			ESLAssert( m_obj != NULL ) ;
			return	env->GetStaticFieldID( m_obj, pszName, pszType ) ;
		}
		// static メンバ取得
		jobject GetStaticObjectField( jfieldID jfid ) const ;
		jboolean GetStaticBooleanField( jfieldID jfid ) const ;
		jbyte GetStaticByteField( jfieldID jfid ) const ;
		jchar GetStaticCharField( jfieldID jfid ) const ;
		jshort GetStaticShortField( jfieldID jfid ) const ;
		jint GetStaticIntField( jfieldID jfid ) const ;
		jlong GetStaticLongField( jfieldID jfid ) const ;
		jfloat GetStaticFloatField( jfieldID jfid ) const ;
		jdouble GetStaticDoubleField( jfieldID jfid ) const ;
		// メソッドID取得
		jmethodID GetMethodID( const char * pszName, const char * pszType )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			ESLAssert( m_obj != NULL ) ;
			return	env->GetMethodID( m_obj, pszName, pszType ) ;
		}
		jmethodID GetStaticMethodID( const char * pszName, const char * pszType )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			ESLAssert( m_obj != NULL ) ;
			return	env->GetStaticMethodID( m_obj, pszName, pszType ) ;
		}
		// static メソッド呼び出し
		jobject CallStaticObjectMethod( jmethodID jmid, ... ) const ;
		jboolean CallStaticBooleanMethod( jmethodID jmid, ... ) const ;
		jbyte CallStaticByteMethod( jmethodID jmid, ... ) const ;
		jchar CallStaticCharMethod( jmethodID jmid, ... ) const ;
		jshort CallStaticShortMethod( jmethodID jmid, ... ) const ;
		jint CallStaticIntMethod( jmethodID jmid, ... ) const ;
		jlong CallStaticLongMethod( jmethodID jmid, ... ) const ;
		jfloat CallStaticFloatMethod( jmethodID jmid, ... ) const ;
		jdouble CallStaticDoubleMethod( jmethodID jmid, ... ) const ;
		void CallStaticVoidMethod( jmethodID jmid, ... ) const ;

	} ;



	//////////////////////////////////////////////////////////////////////////
	// Java Object
	//////////////////////////////////////////////////////////////////////////

	class	JavaObject	: public ESLObject
	{
	protected:
		JSmartClass	m_class ;
		JNIEnv *	m_env ;
		jobject		m_obj ;
		bool		m_refLocal ;
		bool		m_refGlobal ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( JavaObject, ESLObject )
		// 構築関数
		JavaObject( jobject obj = NULL, bool refLocal = false, JNIEnv * env = NULL )
		{
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
		// 消滅関数
		virtual ~JavaObject( void ) ;
		// 関連付け
		virtual void AttachJavaObject
			( jobject obj, bool refLocal = false, JNIEnv * env = NULL ) ;
		const JavaObject& operator = ( jobject obj )
		{
			AttachJavaObject( obj, false, NULL ) ;
			return	*this ;
		}
		// 参照解放
		virtual void DetachJavaObject( void ) ;
		// オブジェクト取得
		jobject GetObject( void ) const
		{
			return	m_obj ;
		}
		operator jobject ( void ) const
		{
			return	m_obj ;
		}
		// Java クラス取得
		JSmartClass& GetClass( void ) ;
		// グローバル参照へ変換する
		jobject MakeGlobalRef( void ) ;

	public:
		// Java オブジェクト生成
		jobject CreateJavaObject
			( jclass clsObj, const char * pszType,
							va_list vl, JNIEnv * env = NULL ) ;
		jobject CreateJavaObject
			( const char * pszClassSig,
					const char * pszType = NULL, ... ) ;
		// ローカル参照の自動解放をキャンセルし Java オブジェクトを取得する
		jobject DetachLocalRef( void ) ;
		// Java グローバル参照を生成する
		jobject NewGlobalRef( void ) ;

	public:
		// String 生成
		jstring CreateString
			( const jchar * pszText, jsize nLength = -1, JNIEnv * env = NULL ) ;
		jstring CreateUTFString
			( const char * pszText, JNIEnv * env = NULL ) ;
		jstring CreateWideString
			( const wchar_t * pwszText, jsize nLength = -1, JNIEnv * env = NULL ) ;
		// boolean[] 生成
		jbooleanArray CreateBooleanArray( jsize nLength, JNIEnv * env = NULL ) ;
		// byte[] 生成
		jbyteArray CreateByteArray( jsize nLength, JNIEnv * env = NULL ) ;
		// char[] 生成
		jcharArray CreateCharArray( jsize nLength, JNIEnv * env = NULL ) ;
		// short[] 生成
		jshortArray CreateShortArray( jsize nLength, JNIEnv * env = NULL ) ;
		// int[] 生成
		jintArray CreateIntArray( jsize nLength, JNIEnv * env = NULL ) ;
		// long[] 生成
		jlongArray CreateLongArray( jsize nLength, JNIEnv * env = NULL ) ;
		// float[] 生成
		jfloatArray CreateFloatArray( jsize nLength, JNIEnv * env = NULL ) ;
		// double[] 生成
		jdoubleArray CreateDoubleArray( jsize nLength, JNIEnv * env = NULL ) ;
		// Object[] 生成
		jobjectArray CreateObjectArray
			( jsize nLength, jclass clsType,
				jobject objInitElement = NULL, JNIEnv * env = NULL ) ;
		// ByteBuffer 生成
		jobject CreateByteBuffer
			( void * ptrBuf, jlong nBytes, JNIEnv * env = NULL ) ;

	public:
		// メンバ ID 取得
		jfieldID GetFieldID( const char * pszName, const char * pszType )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	env->GetFieldID( GetClass(), pszName, pszType ) ;
		}
		jfieldID GetBooleanFieldID( const char * pszName )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	env->GetFieldID( GetClass(), pszName, "Z" ) ;
		}
		jfieldID GetByteFieldID( const char * pszName )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	env->GetFieldID( GetClass(), pszName, "B" ) ;
		}
		jfieldID GetCharFieldID( const char * pszName )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	env->GetFieldID( GetClass(), pszName, "C" ) ;
		}
		jfieldID GetShortFieldID( const char * pszName )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	env->GetFieldID( GetClass(), pszName, "S" ) ;
		}
		jfieldID GetIntFieldID( const char * pszName )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	env->GetFieldID( GetClass(), pszName, "I" ) ;
		}
		jfieldID GetLongFieldID( const char * pszName )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	env->GetFieldID( GetClass(), pszName, "J" ) ;
		}
		jfieldID GetFloatFieldID( const char * pszName )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	env->GetFieldID( GetClass(), pszName, "F" ) ;
		}
		jfieldID GetDoubleFieldID( const char * pszName )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	env->GetFieldID( GetClass(), pszName, "D" ) ;
		}
		jfieldID GetByteArrayFieldID( const char * pszName )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	env->GetFieldID( GetClass(), pszName, "[B" ) ;
		}
		jfieldID GetShortArrayFieldID( const char * pszName )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	env->GetFieldID( GetClass(), pszName, "[S" ) ;
		}
		jfieldID GetIntArrayFieldID( const char * pszName )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	env->GetFieldID( GetClass(), pszName, "[I" ) ;
		}
		jfieldID GetFloatArrayFieldID( const char * pszName )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	env->GetFieldID( GetClass(), pszName, "[F" ) ;
		}
		// メンバ値取得
		jboolean GetBooleanField( jfieldID fidMember )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	env->GetBooleanField( m_obj, fidMember ) ;
		}
		jbyte GetByteField( jfieldID fidMember )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	env->GetByteField( m_obj, fidMember ) ;
		}
		jchar GetCharField( jfieldID fidMember )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	env->GetCharField( m_obj, fidMember ) ;
		}
		jshort GetShortField( jfieldID fidMember )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	env->GetShortField( m_obj, fidMember ) ;
		}
		jint GetIntField( jfieldID fidMember )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	env->GetIntField( m_obj, fidMember ) ;
		}
		jlong GetLongField( jfieldID fidMember )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	env->GetLongField( m_obj, fidMember ) ;
		}
		jfloat GetFloatField( jfieldID fidMember )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	env->GetFloatField( m_obj, fidMember ) ;
		}
		jdouble GetDoubleField( jfieldID fidMember )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	env->GetDoubleField( m_obj, fidMember ) ;
		}
		jobject GetObjectField( jfieldID fidMember )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	env->GetObjectField( m_obj, fidMember ) ;
		}
		// 静メンバ ID 取得
		jfieldID GetStaticFieldID( const char * pszName, const char * pszType )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	env->GetStaticFieldID( GetClass(), pszName, pszType ) ;
		}
		jfieldID GetStaticBooleanFieldID( const char * pszName )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	env->GetStaticFieldID( GetClass(), pszName, "Z" ) ;
		}
		jfieldID GetStaticByteFieldID( const char * pszName )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	env->GetStaticFieldID( GetClass(), pszName, "B" ) ;
		}
		jfieldID GetStaticCharFieldID( const char * pszName )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	env->GetStaticFieldID( GetClass(), pszName, "C" ) ;
		}
		jfieldID GetStaticShortFieldID( const char * pszName )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	env->GetStaticFieldID( GetClass(), pszName, "S" ) ;
		}
		jfieldID GetStaticIntFieldID( const char * pszName )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	env->GetStaticFieldID( GetClass(), pszName, "I" ) ;
		}
		jfieldID GetStaticLongFieldID( const char * pszName )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	env->GetStaticFieldID( GetClass(), pszName, "J" ) ;
		}
		jfieldID GetStaticFloatFieldID( const char * pszName )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	env->GetStaticFieldID( GetClass(), pszName, "F" ) ;
		}
		jfieldID GetStaticDoubleFieldID( const char * pszName )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	env->GetStaticFieldID( GetClass(), pszName, "D" ) ;
		}
		// 静メンバ値取得
		jboolean GetStaticBooleanField( jfieldID fidMember )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	env->GetStaticBooleanField( GetClass(), fidMember ) ;
		}
		jbyte GetStaticByteField( jfieldID fidMember )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	env->GetStaticByteField( GetClass(), fidMember ) ;
		}
		jchar GetStaticCharField( jfieldID fidMember )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	env->GetStaticCharField( GetClass(), fidMember ) ;
		}
		jshort GetStaticShortField( jfieldID fidMember )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	env->GetStaticShortField( GetClass(), fidMember ) ;
		}
		jint GetStaticIntField( jfieldID fidMember )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	env->GetStaticIntField( GetClass(), fidMember ) ;
		}
		jlong GetStaticLongField( jfieldID fidMember )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	env->GetStaticLongField( GetClass(), fidMember ) ;
		}
		jfloat GetStaticFloatField( jfieldID fidMember )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	env->GetStaticFloatField( GetClass(), fidMember ) ;
		}
		jdouble GetStaticDoubleField( jfieldID fidMember )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	env->GetStaticDoubleField( GetClass(), fidMember ) ;
		}
		jobject GetStaticObjectField( jfieldID fidMember )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	env->GetStaticObjectField( GetClass(), fidMember ) ;
		}
		// メンバ値設定
		void SetBooleanField( jfieldID fidMember, jboolean value )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			env->SetBooleanField( m_obj, fidMember, value ) ;
		}
		void SetByteField( jfieldID fidMember, jbyte value )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			env->SetByteField( m_obj, fidMember, value ) ;
		}
		void SetCharField( jfieldID fidMember, jchar value )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			env->SetCharField( m_obj, fidMember, value ) ;
		}
		void SetShortField( jfieldID fidMember, jshort value )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			env->SetShortField( m_obj, fidMember, value ) ;
		}
		void SetIntField( jfieldID fidMember, jint value )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			env->SetIntField( m_obj, fidMember, value ) ;
		}
		void SetLongField( jfieldID fidMember, jlong value )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			env->SetLongField( m_obj, fidMember, value ) ;
		}
		void SetFloatField( jfieldID fidMember, jfloat value )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			env->SetFloatField( m_obj, fidMember, value ) ;
		}
		void SetObjectField( jfieldID fidMember, jobject value )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			env->SetObjectField( m_obj, fidMember, value ) ;
		}
		// メンバ直接取得
		jbyte GetByteField( const char * pszName )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	env->GetByteField( m_obj, GetByteFieldID( pszName ) ) ;
		}
		jchar GetCharField( const char * pszName )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	env->GetCharField( m_obj, GetCharFieldID( pszName ) ) ;
		}
		jshort GetShortField( const char * pszName )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	env->GetShortField( m_obj, GetShortFieldID( pszName ) ) ;
		}
		jint GetIntField( const char * pszName )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	env->GetIntField( m_obj, GetIntFieldID( pszName ) ) ;
		}
		jlong GetLongField( const char * pszName )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	env->GetLongField( m_obj, GetLongFieldID( pszName ) ) ;
		}
		jfloat GetFloatField( const char * pszName )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	env->GetFloatField( m_obj, GetFloatFieldID( pszName ) ) ;
		}
		jobject GetObjectField( const char * pszName, const char * pszType )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	env->GetObjectField( m_obj, GetFieldID( pszName, pszType ) ) ;
		}
		jbyteArray GetByteArrayField( const char * pszName )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	(jbyteArray) env->GetObjectField
							( m_obj, GetByteArrayFieldID( pszName ) ) ;
		}
		jshortArray GetShortArrayField( const char * pszName )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	(jshortArray) env->GetObjectField
							( m_obj, GetShortArrayFieldID( pszName ) ) ;
		}
		jintArray GetIntArrayField( const char * pszName )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	(jintArray) env->GetObjectField
							( m_obj, GetIntArrayFieldID( pszName ) ) ;
		}
		jfloatArray GetFloatArrayField( const char * pszName )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	(jfloatArray) env->GetObjectField
							( m_obj, GetFloatArrayFieldID( pszName ) ) ;
		}
		// メソッドID取得
		jmethodID GetMethodID( const char * pszName, const char * pszType )
		{
			JNIEnv *	env = m_refGlobal ? JNI::GetJNIEnv() : m_env ;
			ESLAssert( env != NULL ) ;
			return	env->GetMethodID( GetClass(), pszName, pszType ) ;
		}
		// メソッド呼び出し
		jobject CallObjectMethod( jmethodID jmid, ... ) const ;
		jboolean CallBooleanMethod( jmethodID jmid, ... ) const ;
		jbyte CallByteMethod( jmethodID jmid, ... ) const ;
		jchar CallCharMethod( jmethodID jmid, ... ) const ;
		jshort CallShortMethod( jmethodID jmid, ... ) const ;
		jint CallIntMethod( jmethodID jmid, ... ) const ;
		jlong CallLongMethod( jmethodID jmid, ... ) const ;
		jfloat CallFloatMethod( jmethodID jmid, ... ) const ;
		jdouble CallDoubleMethod( jmethodID jmid, ... ) const ;
		void CallVoidMethod( jmethodID jmid, ... ) const ;
	} ;



	//////////////////////////////////////////////////////////////////////////
	// Java String 参照
	//////////////////////////////////////////////////////////////////////////

	class	JString
	{
	protected:
		JNIEnv *		m_env ;
		jstring			m_str ;
		const jchar *	m_buf ;

	public:
		JString( jstring str, JNIEnv * env = NULL )
			: m_env( NULL ), m_str( NULL ), m_buf( NULL )
		{
			GetBuffer( str, env ) ;
		}
		~JString( void )
		{
			ReleaseBuffer() ;
		}
		const jchar * GetBuffer( jstring str, JNIEnv * env = NULL )
		{
			ReleaseBuffer() ;
			if ( str != NULL )
			{
				if ( env == NULL )
				{
					env = JNI::GetJNIEnv() ;
				}
				jboolean	b ;
				m_env = env ;
				m_str = str ;
				m_buf = env->GetStringChars( m_str, &b ) ;
			}
			return	m_buf ;
		}
		jsize GetLength( void ) const
		{
			if ( m_str != NULL )
			{
				ESLAssert( m_env != NULL ) ;
				return	m_env->GetStringLength( m_str ) ;
			}
			return	0 ;
		}
		const jchar * GetBuffer( void ) const
		{
			return	m_buf ;
		}
		void ReleaseBuffer( void )
		{
			if ( m_buf != NULL )
			{
				ESLAssert( m_env != NULL ) ;
				m_env->ReleaseStringChars( m_str, m_buf ) ;
				m_buf = NULL ;
			}
		}
		void ToString( SSystem::SString& strBuf )
		{
			jsize			len = GetLength() ;
			const jchar *	pjstr = GetBuffer() ;
			uint16_t *		pszBuf = strBuf.LockBuffer( len ) ;
			for ( jsize i = 0; i < len; i ++ )
			{
				pszBuf[i] = (uint16_t) pjstr[i] ;
			}
			strBuf.UnlockBuffer( len ) ;
		}
	} ;



	//////////////////////////////////////////////////////////////////////////
	// Java 配列参照
	//////////////////////////////////////////////////////////////////////////

	class	JByteArray
	{
	protected:
		JNIEnv *	m_env ;
		jbyteArray	m_arr ;
		jbyte *		m_buf ;

	public:
		JByteArray( jbyteArray arr = NULL, JNIEnv * env = NULL )
			: m_env( NULL ), m_arr( NULL ), m_buf( NULL )
		{
			GetBuffer( arr, env ) ;
		}
		~JByteArray( void )
		{
			ReleaseBuffer() ;
		}
		jbyte * GetBuffer( jbyteArray arr, JNIEnv * env = NULL )
		{
			ReleaseBuffer() ;
			if ( arr != NULL )
			{
				if ( env == NULL )
				{
					env = JNI::GetJNIEnv() ;
				}
				jboolean	b ;
				m_env = env ;
				m_arr = arr ;
				m_buf = env->GetByteArrayElements( arr, &b ) ;
			}
			return	m_buf ;
		}
		jsize GetLength( void ) const
		{
			if ( m_arr != NULL )
			{
				ESLAssert( m_env != NULL ) ;
				return	m_env->GetArrayLength( m_arr ) ;
			}
			return	0 ;
		}
		jbyte * GetBuffer( void ) const
		{
			return	m_buf ;
		}
		void ReleaseBuffer( void )
		{
			if ( m_buf != NULL )
			{
				ESLAssert( m_env != NULL ) ;
				m_env->ReleaseByteArrayElements( m_arr, m_buf, 0 ) ;
				m_buf = NULL ;
			}
		}
	} ;


	class	JCharArray
	{
	protected:
		JNIEnv *	m_env ;
		jcharArray	m_arr ;
		jchar *		m_buf ;

	public:
		JCharArray( jcharArray arr = NULL, JNIEnv * env = NULL )
			: m_env( NULL ), m_arr( NULL ), m_buf( NULL )
		{
			GetBuffer( arr, env ) ;
		}
		~JCharArray( void )
		{
			ReleaseBuffer() ;
		}
		jchar * GetBuffer( jcharArray arr, JNIEnv * env = NULL )
		{
			ReleaseBuffer() ;
			if ( arr != NULL )
			{
				if ( env == NULL )
				{
					env = JNI::GetJNIEnv() ;
				}
				jboolean	b ;
				m_env = env ;
				m_arr = arr ;
				m_buf = env->GetCharArrayElements( arr, &b ) ;
			}
			return	m_buf ;
		}
		jsize GetLength( void ) const
		{
			if ( m_arr != NULL )
			{
				ESLAssert( m_env != NULL ) ;
				return	m_env->GetArrayLength( m_arr ) ;
			}
			return	0 ;
		}
		jchar * GetBuffer( void ) const
		{
			return	m_buf ;
		}
		void ReleaseBuffer( void )
		{
			if ( m_buf != NULL )
			{
				ESLAssert( m_env != NULL ) ;
				m_env->ReleaseCharArrayElements( m_arr, m_buf, 0 ) ;
				m_buf = NULL ;
			}
		}
	} ;

	class	JShortArray
	{
	protected:
		JNIEnv *	m_env ;
		jshortArray	m_arr ;
		jshort *	m_buf ;

	public:
		JShortArray( jshortArray arr = NULL, JNIEnv * env = NULL )
			: m_env( NULL ), m_arr( NULL ), m_buf( NULL )
		{
			GetBuffer( arr, env ) ;
		}
		~JShortArray( void )
		{
			ReleaseBuffer() ;
		}
		jshort * GetBuffer( jshortArray arr, JNIEnv * env = NULL )
		{
			ReleaseBuffer() ;
			if ( arr != NULL )
			{
				if ( env == NULL )
				{
					env = JNI::GetJNIEnv() ;
				}
				jboolean	b ;
				m_env = env ;
				m_arr = arr ;
				m_buf = env->GetShortArrayElements( arr, &b ) ;
			}
			return	m_buf ;
		}
		jsize GetLength( void ) const
		{
			if ( m_arr != NULL )
			{
				return	m_env->GetArrayLength( m_arr ) ;
			}
			return	0 ;
		}
		jshort * GetBuffer( void ) const
		{
			return	m_buf ;
		}
		void ReleaseBuffer( void )
		{
			if ( m_buf != NULL )
			{
				m_env->ReleaseShortArrayElements( m_arr, m_buf, 0 ) ;
				m_buf = NULL ;
			}
		}
	} ;

	class	JIntArray
	{
	protected:
		JNIEnv *	m_env ;
		jintArray	m_arr ;
		jint *		m_buf ;

	public:
		JIntArray( jintArray arr = NULL, JNIEnv * env = NULL )
			: m_env( NULL ), m_arr( NULL ), m_buf( NULL )
		{
			GetBuffer( arr, env ) ;
		}
		~JIntArray( void )
		{
			ReleaseBuffer() ;
		}
		jint * GetBuffer( jintArray arr, JNIEnv * env = NULL )
		{
			ReleaseBuffer() ;
			if ( arr != NULL )
			{
				if ( env == NULL )
				{
					env = JNI::GetJNIEnv() ;
				}
				jboolean	b ;
				m_env = env ;
				m_arr = arr ;
				m_buf = env->GetIntArrayElements( arr, &b ) ;
			}
			return	m_buf ;
		}
		jsize GetLength( void ) const
		{
			if ( m_arr != NULL )
			{
				ESLAssert( m_env != NULL ) ;
				return	m_env->GetArrayLength( m_arr ) ;
			}
			return	0 ;
		}
		jint * GetBuffer( void ) const
		{
			return	m_buf ;
		}
		void ReleaseBuffer( void )
		{
			if ( m_buf != NULL )
			{
				ESLAssert( m_env != NULL ) ;
				m_env->ReleaseIntArrayElements( m_arr, m_buf, 0 ) ;
				m_buf = NULL ;
			}
		}
	} ;

	class	JLongArray
	{
	protected:
		JNIEnv *	m_env ;
		jlongArray	m_arr ;
		jlong *		m_buf ;

	public:
		JLongArray( jlongArray arr = NULL, JNIEnv * env = NULL )
			: m_env( NULL ), m_arr( NULL ), m_buf( NULL )
		{
			GetBuffer( arr, env ) ;
		}
		~JLongArray( void )
		{
			ReleaseBuffer() ;
		}
		jlong * GetBuffer( jlongArray arr, JNIEnv * env = NULL )
		{
			ReleaseBuffer() ;
			if ( arr != NULL )
			{
				if ( env == NULL )
				{
					env = JNI::GetJNIEnv() ;
				}
				jboolean	b ;
				m_env = env ;
				m_arr = arr ;
				m_buf = env->GetLongArrayElements( arr, &b ) ;
			}
			return	m_buf ;
		}
		jsize GetLength( void ) const
		{
			if ( m_arr != NULL )
			{
				ESLAssert( m_env != NULL ) ;
				return	m_env->GetArrayLength( m_arr ) ;
			}
			return	0 ;
		}
		jlong * GetBuffer( void ) const
		{
			return	m_buf ;
		}
		void ReleaseBuffer( void )
		{
			if ( m_buf != NULL )
			{
				ESLAssert( m_env != NULL ) ;
				m_env->ReleaseLongArrayElements( m_arr, m_buf, 0 ) ;
				m_buf = NULL ;
			}
		}
	} ;

	class	JFloatArray
	{
	protected:
		JNIEnv *	m_env ;
		jfloatArray	m_arr ;
		jfloat *	m_buf ;

	public:
		JFloatArray( jfloatArray arr = NULL, JNIEnv * env = NULL )
			: m_env( NULL ), m_arr( NULL ), m_buf( NULL )
		{
			GetBuffer( arr, env ) ;
		}
		~JFloatArray( void )
		{
			ReleaseBuffer() ;
		}
		jfloat * GetBuffer( jfloatArray arr, JNIEnv * env = NULL )
		{
			ReleaseBuffer() ;
			if ( arr != NULL )
			{
				if ( env == NULL )
				{
					env = JNI::GetJNIEnv() ;
				}
				jboolean	b ;
				m_env = env ;
				m_arr = arr ;
				m_buf = env->GetFloatArrayElements( arr, &b ) ;
			}
			return	m_buf ;
		}
		jsize GetLength( void ) const
		{
			if ( m_arr != NULL )
			{
				ESLAssert( m_env != NULL ) ;
				return	m_env->GetArrayLength( m_arr ) ;
			}
			return	0 ;
		}
		jfloat * GetBuffer( void ) const
		{
			return	m_buf ;
		}
		void ReleaseBuffer( void )
		{
			if ( m_buf != NULL )
			{
				ESLAssert( m_env != NULL ) ;
				m_env->ReleaseFloatArrayElements( m_arr, m_buf, 0 ) ;
				m_buf = NULL ;
			}
		}
	} ;

	class	JDoubleArray
	{
	protected:
		JNIEnv *		m_env ;
		jdoubleArray	m_arr ;
		jdouble *		m_buf ;

	public:
		JDoubleArray( jdoubleArray arr = NULL, JNIEnv * env = NULL )
			: m_env( NULL ), m_arr( NULL ), m_buf( NULL )
		{
			GetBuffer( arr, env ) ;
		}
		~JDoubleArray( void )
		{
			ReleaseBuffer() ;
		}
		jdouble * GetBuffer( jdoubleArray arr, JNIEnv * env = NULL )
		{
			ReleaseBuffer() ;
			if ( arr != NULL )
			{
				if ( env == NULL )
				{
					env = JNI::GetJNIEnv() ;
				}
				jboolean	b ;
				m_env = env ;
				m_arr = arr ;
				m_buf = env->GetDoubleArrayElements( arr, &b ) ;
			}
			return	m_buf ;
		}
		jsize GetLength( void ) const
		{
			if ( m_arr != NULL )
			{
				ESLAssert( m_env != NULL ) ;
				return	m_env->GetArrayLength( m_arr ) ;
			}
			return	0 ;
		}
		jdouble * GetBuffer( void ) const
		{
			return	m_buf ;
		}
		void ReleaseBuffer( void )
		{
			if ( m_buf != NULL )
			{
				ESLAssert( m_env != NULL ) ;
				m_env->ReleaseDoubleArrayElements( m_arr, m_buf, 0 ) ;
				m_buf = NULL ;
			}
		}
	} ;

	class	JObjectArray
	{
	protected:
		JNIEnv *		m_env ;
		jobjectArray	m_arr ;

	public:
		JObjectArray( jobjectArray arr = NULL, JNIEnv * env = NULL )
			: m_env( NULL ), m_arr( NULL )
		{
			AttachArray( arr, env ) ;
		}
		void AttachArray( jobjectArray arr, JNIEnv * env = NULL )
		{
			m_env = env ;
			m_arr = arr ;
			if ( m_arr && !m_env )
			{
				m_env = JNI::GetJNIEnv() ;
			}
		}
		jsize GetLength( void ) const
		{
			if ( m_arr != NULL )
			{
				ESLAssert( m_env != NULL ) ;
				return	m_env->GetArrayLength( m_arr ) ;
			}
			return	0 ;
		}
		jobject GetAt( jsize index ) const
		{
			if ( m_arr != NULL )
			{
				ESLAssert( m_env != NULL ) ;
				return	m_env->GetObjectArrayElement( m_arr, index ) ;
			}
			return	NULL ;
		}
		void SetAt( jsize index, jobject obj )
		{
			if ( m_arr != NULL )
			{
				ESLAssert( m_env != NULL ) ;
				m_env->SetObjectArrayElement( m_arr, index, obj ) ;
			}
		}
	} ;



	//////////////////////////////////////////////////////////////////////////
	// ByteBuffer バッファ参照
	//////////////////////////////////////////////////////////////////////////

	class	JDirectBuffer
	{
	protected:
		jobject		m_obj ;
		void *		m_buf ;
		jlong		m_bytes ;

	public:
		JDirectBuffer( jobject obj = NULL, JNIEnv * env = NULL )
			: m_obj( NULL ), m_buf( NULL ), m_bytes( 0 )
		{
			GetBuffer( obj, env ) ;
		}
		jobject GetJavaObject( void )
		{
			return	m_obj ;
		}
		void * GetBuffer( jobject obj, JNIEnv * env = NULL )
		{
			m_obj = obj ;
			m_buf = NULL ;
			m_bytes = 0 ;
			if ( obj != NULL )
			{
				if ( env == NULL )
				{
					env = JNI::GetJNIEnv() ;
				}
				m_buf = env->GetDirectBufferAddress( obj ) ;
				m_bytes = env->GetDirectBufferCapacity( obj ) ;
			}
			return	m_buf ;
		}
		void * GetBuffer( void ) const
		{
			return	m_buf ;
		}
		jlong GetLength( void ) const
		{
			return	m_bytes ;
		}
	} ;

}


#endif


