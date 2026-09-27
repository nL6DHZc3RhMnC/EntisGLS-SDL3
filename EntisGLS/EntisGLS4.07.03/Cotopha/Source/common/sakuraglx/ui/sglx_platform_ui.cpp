
#include <sakuraglx/sakuraglx.h>
#include <sakuraglx/sglx_platform_ui.h>

#if	defined(__PLATFORM_WINDOWS__)
#if	_MSC_VER >= 1700
#include <SensorsApi.h>
#include <Sensors.h>
#endif
#endif

using namespace SSystem ;
using namespace SakuraGL ;


//////////////////////////////////////////////////////////////////////////////
// プラットフォーム属性
//////////////////////////////////////////////////////////////////////////////

// タブレット判定
bool SakuraGL::UI::IsPlatformTablet( void )
{
	return	(g_infoPlatform.runtimeOS == platformOS_LinuxAndroid) ;
}



//////////////////////////////////////////////////////////////////////////////
// クリップボード
//////////////////////////////////////////////////////////////////////////////

#if	!defined(__COTOPHA__)

#if	defined(__PLATFORM_ANDROID__)
// ClipboardManager 取得
//////////////////////////////////////////////////////////////////////////////
bool SakuraGL::UI::Clipboard::GetClipboardManager( JNI::JavaObject & jobjClipboard )
{
	JNI::JSmartClass	jclsEntisGLS
		( JNI::FindJavaClass( ENTIS_GLS4_JAVA_PACKAGE "/EntisGLS" ) ) ;
	JNI::JSmartClass	jclsContext
		( JNI::FindJavaClass( "android/content/Context" ) ) ;
	//
	// Context	ctx = EntisGLS.getContext() ;
	//
	jmethodID	jmidGetContext =
		jclsEntisGLS.GetStaticMethodID
				( "getContext", "()Landroid/content/Context;" ) ;
	JNI::JavaObject	jobjContext
		( jclsEntisGLS.CallStaticObjectMethod( jmidGetContext ), true ) ;
	//
	// String sClipboard = Context.CLIPBOARD_SERVICE ;
	//
	JNI::JavaObject	jobjClipboardService
		( jclsContext.GetStaticObjectField
			( jclsContext.GetStaticFieldID
				( "CLIPBOARD_SERVICE", "L" JAVA_LANG_STRING ";" ) ), true ) ;
	//
	// Object cbm = ctx.getSystemService( sClipboard ) ;
	//
	jmethodID	jmidGetSystemService =
		jobjContext.GetMethodID
			( "getSystemService",
				"(L" JAVA_LANG_STRING ";)L" JAVA_LANG_OBJECT ";" ) ;
	jobjClipboard.AttachJavaObject
		( jobjContext.CallObjectMethod
			( jmidGetSystemService,
				jobjClipboardService.GetObject() ), true ) ;
	//
	return	(jobjClipboard.GetObject() != NULL) ;
}
#endif

// テキストデータの有無
//////////////////////////////////////////////////////////////////////////////
bool SakuraGL::UI::Clipboard::HasPlaneText( void )
{
#if	defined(__PLATFORM_WINDOWS__)
	if ( !::OpenClipboard( NULL ) )
	{
		return	false ;
	}
	HGLOBAL	hGlobal ;
	hGlobal = ::GetClipboardData( CF_UNICODETEXT ) ;
	if ( hGlobal == NULL )
	{
		hGlobal = ::GetClipboardData( CF_TEXT ) ;
		if ( hGlobal == NULL )
		{
			::CloseClipboard( ) ;
			return	false ;
		}
	}
	::CloseClipboard( ) ;
	return	true ;

#elif	defined(__PLATFORM_ANDROID__)
	JNI::JavaObject	jobjClipboard ;
	if ( !GetClipboardManager( jobjClipboard ) )
	{
		return	false ;
	}
	jmethodID	jmidHasText = jobjClipboard.GetMethodID( "hasText", "()Z" ) ;
	if ( jmidHasText == NULL )
	{
		return	false ;
	}
	return	(bool) jobjClipboard.CallBooleanMethod( jmidHasText ) ;

#else
	return	false ;
#endif
}

// テキストデータの取得
//////////////////////////////////////////////////////////////////////////////
bool SakuraGL::UI::Clipboard::GetPlaneText( SSystem::SString& strText )
{
#if	defined(__PLATFORM_WINDOWS__)
	bool	fClipboardText = false ;
	if ( !::OpenClipboard( NULL ) )
	{
		return	false ;
	}
	HGLOBAL	hGlobal ;
	hGlobal = ::GetClipboardData( CF_UNICODETEXT ) ;
	if ( hGlobal != NULL )
	{
		LPVOID	lpBuf = ::GlobalLock( hGlobal ) ;
		if ( lpBuf != NULL )
		{
			strText = (const wchar_t *) lpBuf ;
			::GlobalUnlock( hGlobal ) ;
			fClipboardText = true ;
		}
	}
	else
	{
		hGlobal = ::GetClipboardData( CF_TEXT ) ;
		if ( hGlobal != NULL )
		{
			LPVOID	lpBuf = ::GlobalLock( hGlobal ) ;
			if ( lpBuf != NULL )
			{
				strText = (const char *) lpBuf ;
				::GlobalUnlock( hGlobal ) ;
				fClipboardText = true ;
			}
		}
	}
	::CloseClipboard( ) ;
	return	fClipboardText ;

#elif	defined(__PLATFORM_ANDROID__)
	JNI::JavaObject	jobjClipboard ;
	if ( !GetClipboardManager( jobjClipboard ) )
	{
		return	false ;
	}
	jmethodID	jmidGetText =
		jobjClipboard.GetMethodID( "getText", "()Ljava/lang/CharSequence;" ) ;
	if ( jmidGetText == NULL )
	{
		return	false ;
	}
	JNI::JavaObject	jobjText ;
	jobjText.AttachJavaObject
		( jobjClipboard.CallObjectMethod( jmidGetText ), true ) ;
	if ( jobjText.GetObject() == NULL )
	{
		return	false ;
	}
	jmethodID	jmidToString =
		jobjText.GetMethodID( "toString", "()L" JAVA_LANG_STRING ";" ) ;
	if ( jmidToString == NULL )
	{
		return	false ;
	}
	JNI::JSmartObject
		jsobjText( jobjText.CallObjectMethod( jmidToString ) ) ;
	if ( jsobjText.GetObject() == NULL )
	{
		return	false ;
	}
	JNI::JString
		jstrText( (jstring) jsobjText.GetObject() ) ;
	jstrText.ToString( strText ) ;
	return	true ;

#else
	return	false ;
#endif
}

// テキストデータの設定
//////////////////////////////////////////////////////////////////////////////
bool SakuraGL::UI::Clipboard::SetPlaneText( const wchar_t * pwszText )
{
#if	defined(__PLATFORM_WINDOWS__)
	if ( ::OpenClipboard( NULL ) )
	{
		SString			strText = pwszText ;
		SArray<char>	aText ;
		strText.EncodeDefaultTo( aText ) ;
		//
		HGLOBAL	hGlobalText = ::GlobalAlloc
			( GMEM_MOVEABLE, (aText.GetLength() + 1) * sizeof(char) ) ;
		LPVOID	lpBuf = ::GlobalLock( hGlobalText ) ;
		if ( aText.GetLength() > 0 )
		{
			::eslMoveMemory
				( lpBuf, aText.GetConstArray(),
					(aText.GetLength() + 1) * sizeof(char) ) ;
		}
		else
		{
			((char*)lpBuf)[0] = 0 ;
		}
		::GlobalUnlock( hGlobalText ) ;
		//
		HGLOBAL	hGlobalUnicode = ::GlobalAlloc
			( GMEM_MOVEABLE, (strText.GetLength() + 1) * sizeof(wchar_t) ) ;
		lpBuf = ::GlobalLock( hGlobalUnicode ) ;
		if ( strText.GetLength() )
		{
			::eslMoveMemory
				( lpBuf, (const wchar_t *) strText,
					(strText.GetLength() + 1) * sizeof(wchar_t) ) ;
		}
		else
		{
			((wchar_t*)lpBuf)[0] = 0 ;
		}
		::GlobalUnlock( hGlobalUnicode ) ;
		//
		::EmptyClipboard( ) ;
		::SetClipboardData( CF_TEXT, hGlobalText ) ;
		::SetClipboardData( CF_UNICODETEXT, hGlobalUnicode ) ; 
		::CloseClipboard( ) ;
		return	true ;
	}
	return	false ;

#elif	defined(__PLATFORM_ANDROID__)
	JNI::JavaObject	jobjText ;
	jstring	jstrText = jobjText.CreateWideString( pwszText ) ;
	if ( jstrText == NULL )
	{
		return	false ;
	}
	JNI::JString	jstrTextBuf( jstrText ) ;
	const jsize		nTextLen = jstrTextBuf.GetLength() ;
	jstrTextBuf.ReleaseBuffer() ;
	//
	jmethodID	jmidSubSequence =
		jobjText.GetMethodID( "subSequence", "(II)Ljava/lang/CharSequence;" ) ;
	if ( jmidSubSequence == NULL )
	{
		return	false ;
	}
	JNI::JavaObject	jobjCharSeq ;
	jobjCharSeq.AttachJavaObject
		( jobjText.CallObjectMethod( jmidSubSequence, 0, nTextLen ), true ) ;
	if ( jobjCharSeq.GetObject() == NULL )
	{
		return	false ;
	}
	//
	JNI::JavaObject	jobjClipboard ;
	if ( !GetClipboardManager( jobjClipboard ) )
	{
		return	false ;
	}
	jmethodID	jmidSetText =
		jobjClipboard.GetMethodID( "setText", "(Ljava/lang/CharSequence;)V" ) ;
	if ( jmidSetText == NULL )
	{
		return	false ;
	}
	jobjClipboard.CallVoidMethod( jmidSetText, jobjCharSeq.GetObject() ) ;
	return	true ;

#else
	return	false ;
#endif
}

#endif



//////////////////////////////////////////////////////////////////////////////
// バイブレーション
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::UI::SGLVibrator, SGLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
UI::SGLVibrator::SGLVibrator( void )
{
#if	defined(__COTOPHA__)
	try
	{
		m_pVibrator = new UI::Vibrator ;
	}
	catch ( ... )
	{
		m_pVibrator = NULL ;
	}

#else
	m_flagLoop = true ;
	m_iLoopStart = 0 ;

#if	defined(__PLATFORM_ANDROID__)
	JNI::JSmartClass	jclsEntisGLS
		( JNI::FindJavaClass( ENTIS_GLS4_JAVA_PACKAGE "/EntisGLS" ) ) ;
	JNI::JSmartClass	jclsContext
		( JNI::FindJavaClass( "android/content/Context" ) ) ;
	//
	// Context	ctx = EntisGLS.getContext() ;
	//
	jmethodID	jmidGetContext =
		jclsEntisGLS.GetStaticMethodID
				( "getContext", "()Landroid/content/Context;" ) ;
	JNI::JavaObject	jobjContext
		( jclsEntisGLS.CallStaticObjectMethod( jmidGetContext ), true ) ;
	//
	// String sVibrator = Context.VIBRATOR_SERVICE ;
	//
	JNI::JavaObject	jobjVibratorService
		( jclsContext.GetStaticObjectField
			( jclsContext.GetStaticFieldID
				( "VIBRATOR_SERVICE", "L" JAVA_LANG_STRING ";" ) ), true ) ;
	//
	// Object vib = ctx.getSystemService( sVibrator ) ;
	//
	jmethodID	jmidGetSystemService =
		jobjContext.GetMethodID
			( "getSystemService",
				"(L" JAVA_LANG_STRING ";)L" JAVA_LANG_OBJECT ";" ) ;
	m_jobjVibrator.AttachJavaObject
		( jobjContext.CallObjectMethod
			( jmidGetSystemService,
				jobjVibratorService.GetObject() ), true ) ;
	//
	m_jmidCancel = NULL ;
	m_jmidVibrate = NULL ;
	m_jmidHasVibrator = NULL ;
	//
	if ( m_jobjVibrator.GetObject() != NULL )
	{
		m_jobjVibrator.MakeGlobalRef() ;
		//
		m_jmidCancel = m_jobjVibrator.GetMethodID( "cancel", "()V" ) ;
		m_jmidVibrate = m_jobjVibrator.GetMethodID( "vibrate", "([JI)V" ) ;
		m_jmidHasVibrator = m_jobjVibrator.GetMethodID( "hasVibrator", "()Z" ) ;
	}
#endif
#endif
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
UI::SGLVibrator::~SGLVibrator( void )
{
#if	defined(__COTOPHA__)
	if ( m_pVibrator != NULL )
	{
		delete	m_pVibrator ;
		m_pVibrator = NULL ;
	}
#endif
}

// パターン設定
//////////////////////////////////////////////////////////////////////////////
void UI::SGLVibrator::SetPattern
	( const uint32_t* pPattern, size_t nCount, bool fLoop, size_t iLoopStart )
{
#if	defined(__COTOPHA__)
	if ( m_pVibrator != NULL )
	{
		m_pVibrator->SetPattern( pPattern, nCount, fLoop, iLoopStart ) ;
	}
#else
	m_pattern.RemoveAll() ;
	m_pattern.AddArray( pPattern, nCount ) ;
	m_flagLoop = fLoop ;
	m_iLoopStart = iLoopStart ;
#endif
}

// 振動
//////////////////////////////////////////////////////////////////////////////
SGLError UI::SGLVibrator::Start( void )
{
#if	defined(__COTOPHA__)
	if ( m_pVibrator != NULL )
	{
		return	m_pVibrator->Start() ;
	}
	return	sglErrFailed ;

#elif	defined(__PLATFORM_ANDROID__)
	if ( m_jobjVibrator.GetObject() == NULL )
	{
		return	sglErrFailed ;
	}
	JNI::JavaObject	jobjPat ;
	int	nRepeat = -1 ;
	if ( m_pattern.GetLength() > 0 )
	{
		JNI::JLongArray	jlaPat
			(  jobjPat.CreateLongArray( (jsize) m_pattern.GetLength() ) ) ;
		jlong *	pPat = jlaPat.GetBuffer() ;
		jsize	nLen = jlaPat.GetLength() ;
		for ( jsize i = 0; i < nLen; i ++ )
		{
			pPat[i] = (jlong) m_pattern.At( i ) ;
		}
		nRepeat = (int) m_iLoopStart ;
		if ( !m_flagLoop )
		{
			nRepeat = -1 ;
		}
	}
	else
	{
		JNI::JLongArray	jlaPat(  jobjPat.CreateLongArray( 1 ) ) ;
		jlong *	pPat = jlaPat.GetBuffer() ;
		pPat[0] = 1000 ;
	}
	m_jobjVibrator.CallVoidMethod
		( m_jmidVibrate, jobjPat.GetObject(), nRepeat ) ;
	return	sglErrSuccess ;

#else
	return	sglErrFailed ;
#endif
}

// 停止
//////////////////////////////////////////////////////////////////////////////
void UI::SGLVibrator::Stop( void )
{
#if	defined(__COTOPHA__)
	if ( m_pVibrator != NULL )
	{
		m_pVibrator->Stop() ;
	}

#elif	defined(__PLATFORM_ANDROID__)
	if ( m_jobjVibrator.GetObject() == NULL )
	{
		return ;
	}
	m_jobjVibrator.CallVoidMethod( m_jmidCancel ) ;
#endif
}

// バイブレーション機能が使用可能か？
//////////////////////////////////////////////////////////////////////////////
bool UI::SGLVibrator::IsInstalled( void )
{
#if	defined(__COTOPHA__)
	if ( m_pVibrator != NULL )
	{
		return	m_pVibrator->IsInstalled() ;
	}
	return	false ;

#elif	defined(__PLATFORM_ANDROID__)
	if ( m_jobjVibrator.GetObject() == NULL )
	{
		return	false ;
	}
	if ( m_jmidHasVibrator == NULL )
	{
		return	false ;
	}
	return	(bool) m_jobjVibrator.CallBooleanMethod( m_jmidHasVibrator ) ;

#else
	return	false ;
#endif
}



//////////////////////////////////////////////////////////////////////////
// 加速度・ジャイロセンサー
//////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::UI::SGLPostureSensor, SGLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////
UI::SGLPostureSensor::SGLPostureSensor( void )
	: m_matPosture( 1, 0, 0,  0, 1, 0,  0, 0, 1 ),
		m_matAccPosture( 1, 0, 0,  0, 1, 0,  0, 0, 1 ),
		m_matBasePosture( 1, 0, 0,  0, 1, 0,  0, 0, 1 ),
		m_vLastAccel( 0, 0, 0 ),
		m_vLastGyro( 0, 0, 0 ), m_vLastCompass( 0, 0, 0 )
{
	m_fpGravity = 9.80665 ;
	m_fpGDeviation = 0.0 ;
	m_nPollCounter = 0 ;
	m_nFeatures = 0 ;

#if	defined(__PLATFORM_WINDOWS__)
#if	_MSC_VER >= 1700
	m_tidSensor = SThread::InvalidId ;
	m_pSensorManager = NULL ;
	m_pAccelCollection = NULL ;
	m_pGyroCollection = NULL ;
	m_pCompassCollection = NULL ;
	m_pAccelerometer = NULL ;
	m_pGyrometer = NULL ;
	m_pCompass = NULL ;
#endif

#elif	defined(__PLATFORM_ANDROID__)
	m_jmidPrepareSensor = NULL ;
	m_jmidGetSensorFeatures = NULL ;
	m_jmidGetAccelerometer = NULL ;
	m_jmidGetGyroscope = NULL ;
	m_jmidGetCompass = NULL ;
#endif
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////
UI::SGLPostureSensor::~SGLPostureSensor( void )
{
	Release() ;
}

// センサ準備
//////////////////////////////////////////////////////////////////////////////
SGLError UI::SGLPostureSensor::PrepareSensor( uint32_t nFlags )
{
	if ( nFlags == 0 )
	{
		nFlags = featureAccelerometer
				| featureGyroscope
				| featureCompass
				| featureRotateLandScape ;
	}
#if	defined(__PLATFORM_WINDOWS__)
#if	_MSC_VER >= 1700
	if ( m_pAccelerometer != NULL )
	{
		m_pAccelerometer->Release() ;
		m_pAccelerometer = NULL ;
	}
	if ( m_pGyrometer != NULL )
	{
		m_pGyrometer->Release() ;
		m_pGyrometer = NULL ;
	}
	if ( m_pCompass != NULL )
	{
		m_pCompass->Release() ;
		m_pCompass = NULL ;
	}
	if ( m_pSensorManager == NULL )
	{
		if ( ::CoCreateInstance
			( CLSID_SensorManager, NULL,
				CLSCTX_INPROC_SERVER,
				IID_PPV_ARGS(&m_pSensorManager) ) != S_OK )
		{
			return	sglErrFailed ;
		}
	}
	if ( nFlags & featureAccelerometer )
	{
		if ( m_pAccelCollection == NULL )
		{
			m_pSensorManager->GetSensorsByCategory
				( SENSOR_TYPE_ACCELEROMETER_3D, &m_pAccelCollection ) ;
		}
		if ( m_pAccelCollection != NULL )
		{
			m_pAccelCollection->GetAt( 0, &m_pAccelerometer ) ;
		}
	}
	if ( nFlags & featureGyroscope )
	{
		if ( m_pGyroCollection == NULL )
		{
			m_pSensorManager->GetSensorsByCategory
				( SENSOR_TYPE_GYROMETER_3D, &m_pGyroCollection ) ;
		}
		if ( m_pGyroCollection != NULL )
		{
			m_pGyroCollection->GetAt( 0, &m_pGyrometer ) ;
		}
	}
	if ( nFlags & featureCompass )
	{
		if ( m_pCompassCollection == NULL )
		{
			m_pSensorManager->GetSensorsByCategory
				( SENSOR_TYPE_COMPASS_3D, &m_pCompassCollection ) ;
		}
		if ( m_pCompassCollection != NULL )
		{
			m_pCompassCollection->GetAt( 0, &m_pCompass ) ;
		}
	}
	m_nFeatures = GetSensorFeatures() | (nFlags & featureRotateLandScape) ;
	//
	m_tidSensor = SThread::GetCurrentId() ;
	ResetPosture() ;
	PollSensor() ;
	return	(m_pAccelerometer || m_pGyrometer) ? sglErrSuccess : sglErrFailed ;
#else
	return	sglErrNotSupported ;
#endif

#elif	defined(__PLATFORM_ANDROID__)
	if ( m_jobjSensor.GetObject() != NULL )
	{
		return	sglErrFailed ;
	}
	m_jobjSensor.CreateJavaObject
		( ENTIS_GLS4_JAVA_PACKAGE "/PostureSensor" ) ;
	m_jobjSensor.MakeGlobalRef() ;
	//
	m_jobjValues.CreateFloatArray( 3 ) ;
	m_jobjValues.MakeGlobalRef() ;
	//
	m_jmidPrepareSensor =
		m_jobjSensor.GetMethodID( "prepareSensor", "(I)Z" ) ;
	m_jmidGetSensorFeatures =
		m_jobjSensor.GetMethodID( "getSensorFeatures", "()I" ) ;
	m_jmidGetAccelerometer =
		m_jobjSensor.GetMethodID( "getAccelerometer", "([F)V" ) ;
	m_jmidGetGyroscope =
		m_jobjSensor.GetMethodID( "getGyroscope", "([F)V" ) ;
	m_jmidGetCompass =
		m_jobjSensor.GetMethodID( "getCompass", "([F)V" ) ;
	m_jmidWaitSensor =
		m_jobjSensor.GetMethodID( "waitSensor", "(II)Z" ) ;
	//
	if ( !m_jobjSensor.CallBooleanMethod( m_jmidPrepareSensor, nFlags ) )
	{
		return	sglErrFailed ;
	}
	m_nFeatures = GetSensorFeatures() | (nFlags & featureRotateLandScape) ;
	//
	ResetPosture() ;
	PollSensor() ;
	return	sglErrSuccess ;
#else
	return	sglErrNotSupported ;
#endif
}

// センサ機能取得
//////////////////////////////////////////////////////////////////////////////
uint32_t UI::SGLPostureSensor::GetSensorFeatures( void ) const
{
#if	defined(__PLATFORM_WINDOWS__)
	uint32_t	nFlags = 0 ;
	#if	_MSC_VER >= 1700
	if ( m_pAccelerometer != NULL )
	{
		nFlags |= featureAccelerometer ;
	}
	if ( m_pGyrometer != NULL )
	{
		nFlags |= featureGyroscope ;
	}
	if ( m_pCompass != NULL )
	{
		nFlags |= featureCompass ;
	}
	#endif
	return	nFlags ;

#elif	defined(__PLATFORM_ANDROID__)
	if ( m_jobjSensor.GetObject() == NULL )
	{
		return	0 ;
	}
	return	(uint32_t) m_jobjSensor.CallIntMethod( m_jmidGetSensorFeatures ) ;

#else
	return	0 ;
#endif
}

// 所有リソース解放
//////////////////////////////////////////////////////////////////////////////
void UI::SGLPostureSensor::Release( void )
{
#if	defined(__PLATFORM_WINDOWS__)
	#if	_MSC_VER >= 1700
	if ( m_pAccelerometer != NULL )
	{
		m_pAccelerometer->Release() ;
		m_pAccelerometer = NULL ;
	}
	if ( m_pGyrometer != NULL )
	{
		m_pGyrometer->Release() ;
		m_pGyrometer = NULL ;
	}
	if ( m_pCompass != NULL )
	{
		m_pCompass->Release() ;
		m_pCompass = NULL ;
	}
	if ( m_pAccelCollection != NULL )
	{
		m_pAccelCollection->Release() ;
		m_pAccelCollection = NULL ;
	}
	if ( m_pGyroCollection != NULL )
	{
		m_pGyroCollection->Release() ;
		m_pGyroCollection = NULL ;
	}
	if ( m_pCompassCollection != NULL )
	{
		m_pCompassCollection->Release() ;
		m_pCompassCollection = NULL ;
	}
	if ( m_pSensorManager != NULL )
	{
		m_pSensorManager->Release() ;
		m_pSensorManager = NULL ;
	}
	#endif

#elif	defined(__PLATFORM_ANDROID__)
	m_jobjSensor.DetachJavaObject() ;
	m_jobjValues.DetachJavaObject() ;
#endif
}

// 加速度センサ取得 [m/sec^2]
//////////////////////////////////////////////////////////////////////////////
SGLError UI::SGLPostureSensor::GetAccelerometer( S3DVector& vAccel ) const
{
#if	defined(__PLATFORM_WINDOWS__)
	#if	_MSC_VER >= 1700
	if ( m_pAccelerometer == NULL )
	{
		return	sglErrFailed ;
	}
	if ( m_tidSensor != SThread::GetCurrentId() )
	{
		vAccel = m_vLastAccel ;
		return	sglErrSuccess ;
	}
	ISensorDataReport *	pData = NULL ;
	if ( m_pAccelerometer->GetData( &pData ) != S_OK )
	{
		return	sglErrFailed ;
	}
	vAccel.x = 0.0f ;
	vAccel.y = 0.0f ;
	vAccel.z = 0.0f ;
	//
	// ENU 座標系から変換含む
	PROPVARIANT x = {} ;
	PROPVARIANT y = {} ;
	PROPVARIANT z = {} ;
	if ( pData->GetSensorValue
		( SENSOR_DATA_TYPE_ACCELERATION_X_G, &x ) == S_OK )
	{
		if ( !(m_nFeatures & featureRotateLandScape) )
		{
			vAccel.x = (float32_t) x.dblVal ;
		}
		else
		{
			vAccel.z = (float32_t) x.dblVal ;
		}
	}
	if ( pData->GetSensorValue
		( SENSOR_DATA_TYPE_ACCELERATION_Y_G, &y ) == S_OK )
	{
		if ( !(m_nFeatures & featureRotateLandScape) )
		{
			vAccel.z = (float32_t) y.dblVal ;
		}
		else
		{
			vAccel.x = - (float32_t) y.dblVal ;
		}
	}
	if ( pData->GetSensorValue
		( SENSOR_DATA_TYPE_ACCELERATION_Z_G, &z ) == S_OK )
	{
		vAccel.y = - (float32_t) z.dblVal ;
	}
	pData->Release() ;
	return	sglErrSuccess ;
	#else
	return	sglErrNotSupported ;
	#endif

#elif	defined(__PLATFORM_ANDROID__)
	if ( (m_jobjSensor.GetObject() == NULL)
		|| (m_jobjValues.GetObject() == NULL) )
	{
		return	sglErrFailed ;
	}
	m_jobjSensor.CallVoidMethod
		( m_jmidGetAccelerometer, m_jobjValues.GetObject() ) ;
	//
	JNI::JFloatArray
		jfaValues( (jfloatArray) m_jobjValues.GetObject() ) ;
	jfloat *	pfArray = jfaValues.GetBuffer() ;
	if ( !(m_nFeatures & featureRotateLandScape) )
	{
		vAccel.x = pfArray[0] ;
		vAccel.y = - pfArray[1] ;
		vAccel.z = - pfArray[2] ;
	}
	else
	{
		vAccel.x = - pfArray[1] ;
		vAccel.y = - pfArray[0] ;
		vAccel.z = - pfArray[2] ;
	}
	//
	return	sglErrSuccess ;
#else
	return	sglErrNotSupported ;
#endif
}

// ジャイロセンサ取得 [rad/sec]
//////////////////////////////////////////////////////////////////////////////
SGLError UI::SGLPostureSensor::GetGyroscope( S3DVector& vRot ) const
{
#if	defined(__PLATFORM_WINDOWS__)
	#if	_MSC_VER >= 1700
	if ( m_pGyrometer == NULL )
	{
		return	sglErrFailed ;
	}
	if ( m_tidSensor != SThread::GetCurrentId() )
	{
		vRot = m_vLastGyro ;
		return	sglErrSuccess ;
	}
	ISensorDataReport *	pData = NULL ;
	if ( m_pGyrometer->GetData( &pData ) != S_OK )
	{
		return	sglErrFailed ;
	}
	vRot.x = 0.0f ;
	vRot.y = 0.0f ;
	vRot.z = 0.0f ;
	//
	// ENU 座標系から変換含む
	PROPVARIANT x = {} ;
	PROPVARIANT y = {} ;
	PROPVARIANT z = {} ;
	if ( pData->GetSensorValue
		( SENSOR_DATA_TYPE_ANGULAR_ACCELERATION_X_DEGREES_PER_SECOND_SQUARED, &x ) == S_OK )
	{
		if ( !(m_nFeatures & featureRotateLandScape) )
		{
			vRot.x = - (float32_t) (x.dblVal * PI / 180.0) ;
		}
		else
		{
			vRot.z = - (float32_t) (x.dblVal * PI / 180.0) ;
		}
	}
	if ( pData->GetSensorValue
		( SENSOR_DATA_TYPE_ANGULAR_ACCELERATION_Y_DEGREES_PER_SECOND_SQUARED, &y ) == S_OK )
	{
		if ( !(m_nFeatures & featureRotateLandScape) )
		{
			vRot.z = - (float32_t) (y.dblVal * PI / 180.0) ;
		}
		else
		{
			vRot.x = (float32_t) (y.dblVal * PI / 180.0) ;
		}
	}
	if ( pData->GetSensorValue
		( SENSOR_DATA_TYPE_ANGULAR_ACCELERATION_Z_DEGREES_PER_SECOND_SQUARED, &z ) == S_OK )
	{
		vRot.y = (float32_t) (z.dblVal * PI / 180.0) ;
	}
	pData->Release() ;
	return	sglErrSuccess ;
	#else
	return	sglErrNotSupported ;
	#endif

#elif	defined(__PLATFORM_ANDROID__)
	if ( (m_jobjSensor.GetObject() == NULL)
		|| (m_jobjValues.GetObject() == NULL) )
	{
		return	sglErrFailed ;
	}
	m_jobjSensor.CallVoidMethod
		( m_jmidGetGyroscope, m_jobjValues.GetObject() ) ;
	//
	JNI::JFloatArray
		jfaValues( (jfloatArray) m_jobjValues.GetObject() ) ;
	jfloat *	pfArray = jfaValues.GetBuffer() ;
	if ( !(m_nFeatures & featureRotateLandScape) )
	{
		vRot.x = pfArray[0] ;
		vRot.z = - pfArray[1] ;
		vRot.y = - pfArray[2] ;
	}
	else
	{
//		vRot.x = pfArray[1] ;		// *original : 要テスト
//		vRot.z = pfArray[0] ;
//		vRot.y = - pfArray[2] ;

		vRot.x = pfArray[1] ;
		vRot.y = - pfArray[0] ;
		vRot.z = - pfArray[2] ;
	}
	//
	return	sglErrSuccess ;
#else
	return	sglErrNotSupported ;
#endif
}

// コンパス（北方位ベクトル）取得
//////////////////////////////////////////////////////////////////////////////
SGLError UI::SGLPostureSensor::GetCompass( S3DVector& vCompass ) const
{
#if	defined(__PLATFORM_WINDOWS__)
	#if	_MSC_VER >= 1700
	if ( m_pCompass == NULL )
	{
		return	sglErrFailed ;
	}
	if ( m_tidSensor != SThread::GetCurrentId() )
	{
		vCompass = m_vLastCompass ;
		return	sglErrSuccess ;
	}
	ISensorDataReport *	pData = NULL ;
	if ( m_pCompass->GetData( &pData ) != S_OK )
	{
		return	sglErrFailed ;
	}
	//
	// ENU 座標系から変換含む
	PROPVARIANT x = {} ;
	PROPVARIANT y = {} ;
	PROPVARIANT z = {} ;
	S3DDVector	vRot( 0, 0, 0 ) ;
	if ( pData->GetSensorValue
		( SENSOR_DATA_TYPE_MAGNETIC_HEADING_X_DEGREES, &x ) == S_OK )
	{
		vRot.y = - (x.dblVal * PI / 180.0) ;
	}
	if ( pData->GetSensorValue
		( SENSOR_DATA_TYPE_MAGNETIC_HEADING_Y_DEGREES, &y ) == S_OK )
	{
		vRot.z = - (y.dblVal * PI / 180.0) ;
	}
	if ( pData->GetSensorValue
		( SENSOR_DATA_TYPE_MAGNETIC_HEADING_Z_DEGREES, &z ) == S_OK )
	{
		vRot.y = (z.dblVal * PI / 180.0) ;
	}
	S3DMatrix	matRot( 1, 0, 0,  0, 1, 0,  0, 0, 1 ) ;
	matRot.RevolveOnZ( sin(vRot.z), cos(vRot.z) ) ;
	matRot.RevolveOnY( sin(vRot.y), cos(vRot.y) ) ;
	matRot.RevolveOnZ( sin(vRot.x), cos(vRot.x) ) ;
	vCompass.x = 0.0f ;
	vCompass.y = 0.0f ;
	vCompass.z = 1.0f ;
	matRot.RevolveVector( vCompass ) ;
	//
	pData->Release() ;
	return	sglErrSuccess ;
	#else
	return	sglErrNotSupported ;
	#endif

#elif	defined(__PLATFORM_ANDROID__)
	if ( (m_jobjSensor.GetObject() == NULL)
		|| (m_jobjValues.GetObject() == NULL) )
	{
		return	sglErrFailed ;
	}
	m_jobjSensor.CallVoidMethod
		( m_jmidGetCompass, m_jobjValues.GetObject() ) ;
	//
	JNI::JFloatArray
		jfaValues( (jfloatArray) m_jobjValues.GetObject() ) ;
	jfloat *	pfArray = jfaValues.GetBuffer() ;
	//
	S3DVector	vMagnetic ;
	if ( !(m_nFeatures & featureRotateLandScape) )
	{
		vMagnetic.x = pfArray[0] ;
		vMagnetic.z = pfArray[1] ;
	}
	else
	{
		vMagnetic.z = pfArray[0] ;
		vMagnetic.x = - pfArray[1] ;
	}
	vMagnetic.y = - pfArray[2] ;
	//
	/*
	vCompass.x = 0.0f ;
	vCompass.y = 0.0f ;
	vCompass.z = 0.0f ;
	//
	double	g = m_vLastAccel.Absolute() ;
	if ( g > 0.01 )
	{
		// 以下の行列 R, I を計算
		// (0,-g,0) = R * gravity
		// (0,0,m) = I * R * geomagnetic
		S3DMatrix	matR ;
		S3DVector	vG( 0, -1, 0 ) ;
		matR.VectorRotationOf( m_vLastAccel, vG ) ;
		ESLAssert( fabs(((matR * m_vLastAccel).InnerProduct(vG) / g) - 1.0) < 1.0e-5 ) ;
		//
		S3DVector	vM( 0, 0, 1 ) ;
		S3DMatrix	matI ;
		matI.VectorRotationOf( matR * vMagnetic, vM ) ;
		ESLAssert( fabs(((matI * matR * vMagnetic).InnerProduct(vM) / g) - 1.0) < 1.0e-5 ) ;
	}
	*/
	vCompass = vMagnetic ;
	//
	return	sglErrSuccess ;
#else
	return	sglErrNotSupported ;
#endif
}

// センサ値の更新タイミング同期
//////////////////////////////////////////////////////////////////////////////
SGLError UI::SGLPostureSensor::WaitSensor( uint32_t nFeatures, int64_t msecTimeout )
{
#if	defined(__PLATFORM_ANDROID__)
	if ( m_jobjSensor.GetObject() == NULL )
	{
		return	sglErrFailed ;
	}
	jint	jTimeout = (jint) msecTimeout ;
	if ( msecTimeout == Synchronism::Infinite )
	{
		jTimeout = 0x7FFFFFFF ;
	}
	if ( !m_jobjSensor.CallBooleanMethod
		( m_jmidWaitSensor, (jint) nFeatures, jTimeout ) )
	{
		return	sglErrTimeout ;
	}
	return	sglErrSuccess ;

#else
	return	sglErrNotSupported ;
#endif
}

// センサ値ポーリング
//////////////////////////////////////////////////////////////////////////////
SGLError UI::SGLPostureSensor::PollSensor( uint32_t nFlags )
{
	double	secDelta = m_timerPoll.GetRealTime() * 0.001 ;
	m_timerPoll.Reset() ;
	//
	S3DVector	vLinearAccel( 0, 0, 0 ) ;
	S3DVector	vRot( 0, 0, 0 ) ;
	uint32_t	nFeatures = m_nFeatures ;
	if ( nFeatures & featureAccelerometer )
	{
		S3DVector	vAccel ;
		if ( GetAccelerometer( vAccel ) )
		{
			return	sglErrFailed ;
		}
		S3DVector	vGlobalAccel =
						m_matAccPosture * S3DDVector( vAccel ) ;
		S3DVector	vClampAccel = vGlobalAccel ;
		double	fpAccel = vClampAccel.Absolute() ;
		vClampAccel.Normalize() ;
		vClampAccel *=
			(float32_t) esl_fclamp( fpAccel, 9.8 - 0.3, 9.8 + 0.3 ) ;
		//
		float32_t	alpha = (float32_t) pow( 0.5, secDelta ) ;
		float32_t	nalpha = 1.0f - alpha ;
		m_vGravityLowPass = m_vGravityLowPass * alpha + vClampAccel * nalpha ;
		vLinearAccel = vGlobalAccel - m_vGravityLowPass ;
	}
	if ( nFeatures & featureGyroscope )
	{
		if ( GetGyroscope( vRot ) )
		{
			return	sglErrFailed ;
		}
		m_vLastGyro = vRot ;
	}
	if ( nFeatures & featureCompass )
	{
		if ( GetCompass( m_vLastCompass ) )
		{
			return	sglErrFailed ;
		}
	}
	if ( nFeatures & featureGyroscope )
	{
		//
		// ジャイロセンサ差分
		//
		double	r = vRot.Absolute() ;
		if ( r > 1.0e-8 )
		{
			vRot *= (float32_t) (1.0 / r) ;
		}
		double	radTheta = r * secDelta / 2.0 ;
		double	sinTheta = sin( radTheta ) ;
		double	cosTheta = cos( radTheta ) ;
		//
		S3DDQuaternion	qRot ;
		qRot.q[0] = cosTheta ;
		qRot.q[1] = sinTheta * - vRot.x ;
		qRot.q[2] = sinTheta * vRot.y ;
		qRot.q[3] = sinTheta * vRot.z ;
		//
		S3DDMatrix	matRot ;
		qRot.ToMatrix( matRot ) ;
		//
		m_matAccPosture *= matRot ;
		m_matPosture = m_matAccPosture * m_matBasePosture ;
		//
		// 加速度から下方向補正
		//
		if ( nFeatures & featureAccelerometer )
		{
			S3DDVector	vGBase( 0, -1, 0 ) ;
			S3DDVector	vGLP = m_vGravityLowPass ;
			float32_t	alpha = (float32_t) pow( 0.1, secDelta ) ;
			float32_t	nalpha = 1.0f - alpha ;
			vGLP.Normalize() ;
			vGBase = vGBase * nalpha + vGLP * alpha ;
			//
			S3DDMatrix	matG ;
			matG.VectorRotationOf( vGLP, vGBase ) ;
			m_matAccPosture = matG * m_matAccPosture ;
			m_matPosture = m_matAccPosture * m_matBasePosture ;
		}
		//
		// コンパスから向きを補正
		/*
		if ( nFeatures & featureCompass )
		{
			S3DVector	vCompass = matIPosture * m_vLastCompass ;
			vCompass.Normalize() ;
			//
			S3DDVector	vTarget = vCompass * 0.00125 + m_vBaseCompass * 0.99875 ;
			S3DDVector	vCurrent = vCompass ;
			S3DDMatrix	matOrientation ;
			vTarget.y = 0 ;
			vCurrent.y = 0 ;
			matOrientation.VectorRotationOf( vCurrent, vTarget ) ;
			m_matPosture = matOrientation * m_matPosture ;
		}
		*/
	}
	//
	// 加速度から座標を計算
	//
	m_vSpeed += S3DDVector( vLinearAccel ) * secDelta ;
	m_vPosition += m_vSpeed * secDelta ;
	m_vSpeed *= pow( 0.5, secDelta ) ;
	//
	if ( ++ m_nPollCounter >= 1000 )
	{
		S3DDQuaternion	qPosture ;
		qPosture.FromMatrix( m_matAccPosture ) ;
		qPosture.Normalize() ;
		qPosture.ToMatrix( m_matAccPosture ) ;
		m_nPollCounter = 0 ;
	}
	return	sglErrSuccess ;
}

// 姿勢と位置をリセット
//////////////////////////////////////////////////////////////////////////////
SGLError UI::SGLPostureSensor::ResetPosture( void )
{
	m_matPosture = S3DDMatrix( 1, 0, 0,  0, 1, 0,  0, 0, 1 ) ;
	m_matAccPosture = S3DDMatrix( 1, 0, 0,  0, 1, 0,  0, 0, 1 ) ;
	m_matBasePosture = S3DDMatrix( 1, 0, 0,  0, 1, 0,  0, 0, 1 ) ;
	m_vPosition = S3DDVector( 0, 0, 0 ) ;
	m_vSpeed = S3DDVector( 0, 0, 0 ) ;
	m_timerPoll.Reset() ;
	//
	S3DVector	vAccel ;
	if ( GetAccelerometer( vAccel ) )
	{
		return	sglErrFailed ;
	}
	S3DDVector	vdAccel = vAccel ;
	S3DDVector	vdBase( 0, -1, 0 ) ;
	m_matAccPosture.VectorRotationOf( vdAccel, vdBase ) ;
	m_vGravityLowPass = m_matAccPosture * S3DDVector( vAccel ) ;
	//
	double	g = m_vGravityLowPass.Absolute() ;
	m_vGravityLowPass.Normalize() ;
	m_vGravityLowPass *=
			(float32_t) esl_fclamp( g, 9.8 - 0.3, 9.8 + 0.3 ) ;
	//
	/*
	if ( m_nFeatures & featureCompass )
	{
		S3DMatrix	matIPosture ;
		matIPosture.InverseOf( S3DMatrix(m_matPosture) ) ;
		GetCompass( m_vBaseCompass ) ;
		matIPosture.RevolveVector( m_vBaseCompass ) ;
		m_vBaseCompass.Normalize() ;
	}
	*/
	return	sglErrSuccess ;
}

// 位置をリセット
//////////////////////////////////////////////////////////////////////////////
SGLError UI::SGLPostureSensor::ResetPosition( void )
{
	m_vPosition = S3DDVector( 0, 0, 0 ) ;
	m_vSpeed = S3DDVector( 0, 0, 0 ) ;
	m_timerPoll.Reset() ;
	return	sglErrSuccess ;
}

// 姿勢取得
//////////////////////////////////////////////////////////////////////////////
const S3DDMatrix & UI::SGLPostureSensor::GetPosture( void ) const
{
	return	m_matPosture ;
}

// 位置取得
//////////////////////////////////////////////////////////////////////////////
const S3DDVector & UI::SGLPostureSensor::GetPosition( void ) const
{
	return	m_vPosition ;
}



//////////////////////////////////////////////////////////////////////////////
// アプリ内購入アイテム
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
UI::SGLBillingInApp::Purchase::Purchase( void )
{
	m_type = typeInAppItem ;
}

UI::SGLBillingInApp::Purchase::Purchase
			( const UI::SGLBillingInApp::Purchase& purchase )
	: m_type( purchase.m_type ),
		m_aProductIds( purchase.m_aProductIds ),
		#if	!defined(__ANDROID_BILLING_LIBRARY__)
		m_strDevPayload( purchase.m_strDevPayload ),
		#endif
		m_strOrderID( purchase.m_strOrderID )
{
#if	defined(__PLATFORM_ANDROID__)
	JNIEnv *	env = JNI::GetJNIEnv() ;
	jobject		jobj = env->NewLocalRef( purchase.m_jobjPurchase ) ;
	m_jobjPurchase.AttachJavaObject( jobj, true, env ) ;
#endif
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
UI::SGLBillingInApp::Purchase::~Purchase( void )
{
}


//////////////////////////////////////////////////////////////////////////////
// アプリ内購入
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::UI::SGLBillingInApp, SGLObject )

// テスト用定義済みプロダクトID
//////////////////////////////////////////////////////////////////////////////
#if	defined(__PLATFORM_ANDROID__)
const wchar_t *	UI::SGLBillingInApp::TestProductPurchased = L"android.test.purchased" ;		// 購入可能なテストアイテム
const wchar_t *	UI::SGLBillingInApp::TestProductCanceled = L"android.test.canceled" ;		// 購入がキャンセルされるテストアイテム
const wchar_t *	UI::SGLBillingInApp::TestProductRefunded = L"android.test.refunded" ;		// 払い戻しレスポンスされるテストアイテム
const wchar_t *	UI::SGLBillingInApp::TestProductUnavailable = L"android.test.item_unavailable" ;	// 存在しないシミュレーションアイテム
#endif

// 構築関数
//////////////////////////////////////////////////////////////////////////////
UI::SGLBillingInApp::SGLBillingInApp( void )
{
	m_flagStartup = false ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
UI::SGLBillingInApp::~SGLBillingInApp( void )
{
}

#if	defined(__PLATFORM_ANDROID__)
// EntisGLS.getActivity
jobject UI::SGLBillingInApp::GetEntisGLActivity( JNI::JavaObject& jobjActivity )
{
	JNI::JSmartClass	jsclsEntisGLS
		( JNI::FindJavaClass( ENTIS_GLS4_JAVA_PACKAGE "/EntisGLS" ) ) ;
	jmethodID	jmidGetActivity =
		jsclsEntisGLS.GetStaticMethodID
			( "getActivity", "()L" ENTIS_GLS4_JAVA_PACKAGE "/EntisGLActivity;" ) ;
	jobjActivity.AttachJavaObject
		( jsclsEntisGLS.CallStaticObjectMethod( jmidGetActivity ), true ) ;
	return	jobjActivity.GetObject() ;
}

// Purchase 変換
SGLError UI::SGLBillingInApp::ConvertFromJava
			( UI::SGLBillingInApp::Purchase& purchase, jobject jobj )
{
	purchase.m_jobjPurchase.AttachJavaObject( jobj, true ) ;

#if	__ANDROID_BILLING_LIBRARY__ >= 3
	purchase.m_type =
		(ItemType) purchase.m_jobjPurchase.GetIntField( "m_nType" ) ;
	//
#if	__ANDROID_BILLING_LIBRARY__ >= 4
	JNI::JavaObject	jsoStrSku
		( purchase.m_jobjPurchase.GetObjectField
			( "m_listSkus", "Ljava/util/List;" ), true ) ;
	//
	// int count = m_listSkus.size() ;
	//
	jmethodID	jmidSize = jsoStrSku.GetMethodID( "size", "()I" ) ;
	if ( jmidSize == NULL )
	{
		return	sglErrFailed ;
	}
	jint	nCount = jsoStrSku.CallIntMethod( jmidSize ) ;
	// 
	// String sku = m_listSkus.get( i ) ;
	//
	jmethodID	jmidGet =
		jsoStrSku.GetMethodID( "get", "(I)L" JAVA_LANG_OBJECT ";" ) ;
	if ( jmidGet == NULL )
	{
		return	sglErrFailed ;
	}
	for ( jint i = 0; i < nCount; i ++ )
	{
		JNI::JSmartObject	jobjSku( jsoStrSku.CallObjectMethod( jmidGet, i ) ) ;
		if ( jobjSku.GetObject() != NULL )
		{
			JNI::JString	jstrSku( (jstring) jobjSku.GetObject() ) ;
			SString *	pstrSku = new SString ;
			purchase.m_aProductIds.Add( pstrSku ) ;
			jstrSku.ToString( *pstrSku ) ;
		}
	}
#else
	JNI::JSmartObject	jsoStrSku
		( purchase.m_jobjPurchase.GetObjectField
			( "m_strSku", "L" JAVA_LANG_STRING ";" ) ) ;
	JNI::JString	jstrSku( (jstring) jsoStrSku.GetObject() ) ;
	//
	SString *	pstrSku = new SString ;
	purchase.m_aProductIds.Add( pstrSku ) ;
	jstrSku.ToString( *pstrSku ) ;
#endif
	//
	JNI::JSmartObject	jsoStrOrderID
		( purchase.m_jobjPurchase.GetObjectField
			( "m_strOrderId", "L" JAVA_LANG_STRING ";" ) ) ;
	JNI::JString	jstrOrderID( (jstring) jsoStrOrderID.GetObject() ) ;
	jstrOrderID.ToString( purchase.m_strOrderID ) ;

#else
	//
	// purchase.getItemType() == IabHelper.ITEM_TYPE_SUBS ?
	//
	JNI::JSmartClass	jclsIabHelper
		( JNI::FindJavaClass( ENTIS_GLS4_JAVA_PACKAGE "/util/IabHelper" ) ) ;
	if ( jclsIabHelper.GetObject() == NULL )
	{
		return	sglErrFailed ;
	}
	JNIEnv *	env = JNI::GetJNIEnv() ;
	jfieldID	jfidTypeSubs =
		jclsIabHelper.GetStaticFieldID
			( "ITEM_TYPE_SUBS", "L" JAVA_LANG_STRING ";" ) ;
	if ( jfidTypeSubs == NULL )
	{
		return	sglErrFailed ;
	}
	JNI::JSmartObject
		jsoTypeSubs
			( jclsIabHelper.GetStaticObjectField( jfidTypeSubs ), env ) ;
	JNI::JString
		jstrTypeSubs
			( (jstring) jsoTypeSubs.GetObject(), env ) ;
	SString	strTypeSubs ;
	jstrTypeSubs.ToString( strTypeSubs ) ;
	jstrTypeSubs.ReleaseBuffer() ;
	jsoTypeSubs.DetachObject() ;
	//
	jmethodID	jmidGetItemType =
		purchase.m_jobjPurchase.GetMethodID
			( "getItemType", "()L" JAVA_LANG_STRING ";" ) ;
	if ( jmidGetItemType == NULL )
	{
		return	sglErrFailed ;
	}
	JNI::JSmartObject
		jsoItemType
			( purchase.m_jobjPurchase.CallObjectMethod( jmidGetItemType ), env ) ;
	JNI::JString
		jstrItemType
			( (jstring) jsoItemType.GetObject(), env ) ;
	SString	strItemType ;
	jstrItemType.ToString( strItemType ) ;
	jstrItemType.ReleaseBuffer() ;
	jsoItemType.DetachObject() ;
	//
	if ( strTypeSubs == strItemType )
	{
		purchase.m_type = typeSubscription ;
	}
	else
	{
		purchase.m_type = typeInAppItem ;
	}
	//
	// purchase.getSku()
	//
	jmethodID	jmidGetSku =
		purchase.m_jobjPurchase.GetMethodID
			( "getSku", "()L" JAVA_LANG_STRING ";" ) ;
	if ( jmidGetSku == NULL )
	{
		return	sglErrFailed ;
	}
	JNI::JSmartObject
		jsoSku
			( purchase.m_jobjPurchase.CallObjectMethod( jmidGetSku ), env ) ;
	JNI::JString
		jstrSku
			( (jstring) jsoSku.GetObject(), env ) ;
	jstrSku.ToString( purchase.m_strProductId ) ;
	//
	// purchase.getDeveloperPayload()
	//
	jmethodID	jmidGetDeveloperPayload =
		purchase.m_jobjPurchase.GetMethodID
			( "getDeveloperPayload", "()L" JAVA_LANG_STRING ";" ) ;
	if ( jmidGetDeveloperPayload == NULL )
	{
		return	sglErrFailed ;
	}
	JNI::JSmartObject
		jsoDeveloperPayload
			( purchase.m_jobjPurchase.CallObjectMethod( jmidGetDeveloperPayload ), env ) ;
	JNI::JString
		jstrDeveloperPayload
			( (jstring) jsoDeveloperPayload.GetObject(), env ) ;
	jstrDeveloperPayload.ToString( purchase.m_strDevPayload ) ;
	jstrDeveloperPayload.ReleaseBuffer() ;
	jsoDeveloperPayload.DetachObject() ;
#endif
	//
	return	sglErrSuccess ;
}

SGLError UI::SGLBillingInApp::ConvertListFromJava
	( SSystem::SObjectArray<Purchase>& listPurchase, jobject jobj )
{
	JNI::JavaObject	jobjListPurchase( jobj, true ) ;
	if ( jobjListPurchase.GetObject() == NULL )
	{
		return	sglErrFailed ;
	}
	//
	// int count = listPurchase.size() ;
	//
	jmethodID	jmidSize = jobjListPurchase.GetMethodID( "size", "()I" ) ;
	if ( jmidSize == NULL )
	{
		return	sglErrFailed ;
	}
	jint	nCount = jobjListPurchase.CallIntMethod( jmidSize ) ;
	// 
	// Purchase purchase = listPurchase.get( i ) ;
	//
	jmethodID	jmidGet =
		jobjListPurchase.GetMethodID( "get", "(I)L" JAVA_LANG_OBJECT ";" ) ;
	if ( jmidGet == NULL )
	{
		return	sglErrFailed ;
	}
	for ( jint i = 0; i < nCount; i ++ )
	{
		jobject	jobjPurchase = jobjListPurchase.CallObjectMethod( jmidGet, i ) ;
		if ( jobjPurchase != NULL )
		{
			Purchase *	pPurchase = new Purchase ;
			if ( ConvertFromJava( *pPurchase, jobjPurchase ) )
			{
				delete	pPurchase ;
			}
			else
			{
				listPurchase.Add( pPurchase ) ;
			}
		}
	}
	return	sglErrSuccess ;
}

#endif

// 開始
//////////////////////////////////////////////////////////////////////////////
SGLError UI::SGLBillingInApp::Startup( const wchar_t * pwszBase64PublicKey )
{
#if	defined(__PLATFORM_ANDROID__)
	//
	// EntisGLActivity activity = EntisGLS.getActivity() ;
	//
	JNI::JavaObject	jobjActivity ;
	if ( GetEntisGLActivity( jobjActivity ) == NULL )
	{
		return	sglErrFailed ;
	}
	//
	// activity.startupBilling( base64EncodedPublicKey ) ;
	//
	jmethodID	jmidStartupBilling =
		jobjActivity.GetMethodID
			( "startupBilling", "(L" JAVA_LANG_STRING ";)Z" ) ;
	if ( jmidStartupBilling == NULL )
	{
		return	sglErrFailed ;
	}
	JNI::JavaObject	jobjStrPublicKey ;
	if ( !jobjActivity.CallBooleanMethod
		( jmidStartupBilling,
			jobjStrPublicKey.CreateWideString( pwszBase64PublicKey ) ) )
	{
		return	sglErrFailed ;
	}
	m_flagStartup = true ;
	return	sglErrSuccess ;
#else
	return	sglErrFailed ;
#endif
}

// 購入済みアイテムリスト
//////////////////////////////////////////////////////////////////////////////
SGLError UI::SGLBillingInApp::QueryInventory
	( SSystem::SObjectArray<UI::SGLBillingInApp::Purchase>& listPurchase )
{
	listPurchase.RemoveAll() ;
	//
	if ( !m_flagStartup )
	{
		return	sglErrFailed ;
	}
#if	defined(__PLATFORM_ANDROID__)
	//
	// EntisGLActivity activity = EntisGLS.getActivity() ;
	//
	JNI::JavaObject	jobjActivity ;
	if ( GetEntisGLActivity( jobjActivity ) == NULL )
	{
		return	sglErrFailed ;
	}
	//
	// List<Purchase> listPurchase = activity.queryInventory() ;
	//
	jmethodID	jmidQueryInventory =
		jobjActivity.GetMethodID
			( "queryInventory", "()Ljava/util/List;" ) ;
	if ( jmidQueryInventory == NULL )
	{
		return	sglErrFailed ;
	}
	return	ConvertListFromJava
				( listPurchase,
					jobjActivity.CallObjectMethod( jmidQueryInventory ) ) ;
#else
	return	sglErrFailed ;
#endif
}

// 購入フロー実行
//////////////////////////////////////////////////////////////////////////////
SGLError UI::SGLBillingInApp::DoPurchaseItem
	( SObjectArray<UI::SGLBillingInApp::Purchase>& listPurchase,
		const wchar_t * pwszProductId,
		const wchar_t * pwszDevPayload )
{
	listPurchase.RemoveAll() ;
	//
	if ( !m_flagStartup )
	{
		return	sglErrFailed ;
	}
#if	defined(__PLATFORM_ANDROID__)
	//
	// EntisGLActivity activity = EntisGLS.getActivity() ;
	//
	JNI::JavaObject	jobjActivity ;
	if ( GetEntisGLActivity( jobjActivity ) == NULL )
	{
		return	sglErrFailed ;
	}
	//
	// boolean f = activity.doPurchaseItem( sku, payload ) ;
	//
	jmethodID	jmidDoPurchaseItem =
		jobjActivity.GetMethodID
			( "doPurchaseItem",
				"(L" JAVA_LANG_STRING ";L" JAVA_LANG_STRING ";)Z" ) ;
	if ( jmidDoPurchaseItem == NULL )
	{
		return	sglErrFailed ;
	}
	JNI::JavaObject	jobjSku ;
	JNI::JavaObject	jobjPayload ;
	jboolean	bResult =
		jobjActivity.CallBooleanMethod
			( jmidDoPurchaseItem,
				jobjSku.CreateWideString(pwszProductId),
				jobjPayload.CreateWideString(pwszDevPayload) ) ;
	if ( !bResult )
	{
		return	sglErrFailed ;
	}
	//
	// List<Purchase> listPurchase = activity.getPurchaseItemList() ;
	//
	jmethodID	jmidGetPurchaseItemList =
		jobjActivity.GetMethodID
			( "getPurchaseItemList", "()Ljava/util/List;" ) ;
	if ( jmidGetPurchaseItemList == NULL )
	{
		return	sglErrFailed ;
	}
	return	ConvertListFromJava
				( listPurchase,
					jobjActivity.CallObjectMethod( jmidGetPurchaseItemList ) ) ;
#else
	return	sglErrFailed ;
#endif
}

SGLError UI::SGLBillingInApp::DoPurchaseSubscription
	( SObjectArray<UI::SGLBillingInApp::Purchase>& listPurchase,
		const wchar_t * pwszProductId,
		const wchar_t * pwszDevPayload )
{
	listPurchase.RemoveAll() ;
	//
	if ( !m_flagStartup )
	{
		return	sglErrFailed ;
	}
#if	defined(__PLATFORM_ANDROID__)
	//
	// EntisGLActivity activity = EntisGLS.getActivity() ;
	//
	JNI::JavaObject	jobjActivity ;
	if ( GetEntisGLActivity( jobjActivity ) == NULL )
	{
		return	sglErrFailed ;
	}
	//
	// boolean f = activity.doPurchaseSubscription( sku, payload ) ;
	//
	jmethodID	jmidDoPurchaseSubscription =
		jobjActivity.GetMethodID
			( "doPurchaseSubscription",
				"(L" JAVA_LANG_STRING ";L" JAVA_LANG_STRING ";)Z" ) ;
	if ( jmidDoPurchaseSubscription == NULL )
	{
		return	sglErrFailed ;
	}
	JNI::JavaObject	jobjSku ;
	JNI::JavaObject	jobjPayload ;
	jboolean bResult =
		jobjActivity.CallBooleanMethod
			( jmidDoPurchaseSubscription,
				jobjSku.CreateWideString(pwszProductId),
				jobjPayload.CreateWideString(pwszDevPayload) ) ;
	if ( !bResult )
	{
		return	sglErrFailed ;
	}
	//
	// List<Purchase> listPurchase = activity.getPurchaseItemList() ;
	//
	jmethodID	jmidGetPurchaseItemList =
		jobjActivity.GetMethodID
			( "getPurchaseItemList", "()Ljava/util/List;" ) ;
	if ( jmidGetPurchaseItemList == NULL )
	{
		return	sglErrFailed ;
	}
	return	ConvertListFromJava
				( listPurchase,
					jobjActivity.CallObjectMethod( jmidGetPurchaseItemList ) ) ;
#else
	return	sglErrFailed ;
#endif
}

// アイテム消費
//////////////////////////////////////////////////////////////////////////////
SGLError UI::SGLBillingInApp::DoConsumeItem
	( const UI::SGLBillingInApp::Purchase& purchase )
{
	if ( !m_flagStartup )
	{
		return	sglErrFailed ;
	}
#if	defined(__PLATFORM_ANDROID__)
	//
	// EntisGLActivity activity = EntisGLS.getActivity() ;
	//
	JNI::JavaObject	jobjActivity ;
	if ( GetEntisGLActivity( jobjActivity ) == NULL )
	{
		return	sglErrFailed ;
	}
	//
	// activity.doConsumeItem( purchase ) ;
	//
#if	__ANDROID_BILLING_LIBRARY__ >= 3
	jmethodID	jmidDoConsumeItem =
		jobjActivity.GetMethodID
			( "doConsumeItem",
				"(L" ENTIS_GLS4_JAVA_PACKAGE "/BillingHandler$PurchaseInfo;)Z" ) ;
#else
	jmethodID	jmidDoConsumeItem =
		jobjActivity.GetMethodID
			( "doConsumeItem",
				"(L" ENTIS_GLS4_JAVA_PACKAGE "/util/Purchase;)Z" ) ;
#endif
	if ( jmidDoConsumeItem == NULL )
	{
		return	sglErrFailed ;
	}
	if ( !jobjActivity.CallBooleanMethod
		( jmidDoConsumeItem, purchase.m_jobjPurchase.GetObject() ) )
	{
		return	sglErrFailed ;
	}
	return	sglErrSuccess ;
#else
	return	sglErrFailed ;
#endif
}

