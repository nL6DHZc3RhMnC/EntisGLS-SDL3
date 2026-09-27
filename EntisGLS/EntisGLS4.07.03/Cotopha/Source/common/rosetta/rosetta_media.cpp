
#include <rosetta/rosetta.h>
#include <sakuragl/sakuragl.h>
#include <rosetta/rosetta_reference.h>
#include <rosetta/rosetta_array.h>
#include <rosetta/rosetta_image.h>
#include <rosetta/rosetta_media.h>

using namespace	SSystem ;
using namespace	SakuraGL ;
using namespace	Rosetta ;


//////////////////////////////////////////////////////////////////////////////
// SoundPlayer.Format クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSSoundPlayerClass::FormatClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSSoundPlayerClass::FormatClass::FormatClass
	( RSClass * pClass, const wchar_t * pwszClassName )
: RSClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSSoundPlayerClass::FormatClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSGenericObject( this ) ;
	//
	m_pPrototype->CreateMemberIntegerAs( context, L"format", 0 ) ;
	m_pPrototype->CreateMemberIntegerAs( context, L"frequency", 0 ) ;
	m_pPrototype->CreateMemberIntegerAs( context, L"channels", 0 ) ;
	m_pPrototype->CreateMemberIntegerAs( context, L"bitsPerSample", 0 ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"samplesToBytes", L"long", L"long samples",
			NULL, &RSSoundPlayerClass::FormatClass::method_samplesToBytes, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"bytesToSamples", L"long", L"long bytes",
			NULL, &RSSoundPlayerClass::FormatClass::method_bytesToSamples, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"samplesToMilliSec", L"long", L"long samples",
			NULL, &RSSoundPlayerClass::FormatClass::method_samplesToMilliSec, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"milliSecToSamples", L"long", L"long millisec",
			NULL, &RSSoundPlayerClass::FormatClass::method_milliSecToSamples, NULL ) ;
}

// long samplesToBytes( long samples )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSoundPlayerClass::FormatClass::method_samplesToBytes
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	int64_t	nBitsPerSamples = pThis->GetMemberIntegerAs( context, L"bisPerSample" ) ;
	int64_t	nChannels = pThis->GetMemberIntegerAs( context, L"channels" ) ;
	return	context.new_Integer
				( (arg.LongAt(0) * nBitsPerSamples * nChannels) >> 3 ) ;
}

// long bytesToSamples( long bytes )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSoundPlayerClass::FormatClass::method_bytesToSamples
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	int64_t	nBitsPerSamples = pThis->GetMemberIntegerAs( context, L"bisPerSample" ) ;
	int64_t	nChannels = pThis->GetMemberIntegerAs( context, L"channels" ) ;
	if ( nBitsPerSamples * nChannels != 0 )
	{
		return	context.new_Integer
					( (arg.LongAt(0) << 3) / (nBitsPerSamples * nChannels) ) ;
	}
	return	context.new_Integer( 0 ) ;
}

// long samplesToMilliSec( long samples )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSoundPlayerClass::FormatClass::method_samplesToMilliSec
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	int64_t	nFrequency = pThis->GetMemberIntegerAs( context, L"frequency" ) ;
	if ( nFrequency != 0 )
	{
		return	context.new_Integer( arg.LongAt(0) * 1000 / nFrequency ) ;
	}
	return	context.new_Integer( 0 ) ;
}

// long milliSecToSamples( long millisec )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSoundPlayerClass::FormatClass::method_milliSecToSamples
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	int64_t	nFrequency = pThis->GetMemberIntegerAs( context, L"frequency" ) ;
	return	context.new_Integer( arg.LongAt(0) * nFrequency / 1000 ) ;
}



//////////////////////////////////////////////////////////////////////////////
// SoundPlayer.Listener オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
	( Rosetta::RSSoundPlayerClass::Listener, RSGenericObject, SGLSoundPlayerListener )

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
RSSoundPlayerClass::Listener::~Listener( void )
{
}

// 複製（参照の複製を含む）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSoundPlayerClass::Listener::DuplicateObject( RSContext& context ) const
{
	ESLAssert( m_pClass != NULL ) ;
	Listener *	pObj = new Listener( context.GetVM(), m_pClass ) ;
	pObj->m_gcmMembers.DuplicateAllMembers
				( context, m_gcmMembers.m_members ) ;
	return	pObj ;
}

// 複製（実体も可能な限り複製）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSoundPlayerClass::Listener::CloneObject( RSContext& context ) const
{
	ESLAssert( m_pClass != NULL ) ;
	Listener *	pObj = new Listener( context.GetVM(), m_pClass ) ;
	pObj->m_gcmMembers.CloneAllMembers
				( context, m_gcmMembers.m_members ) ;
	return	pObj ;
}

// バッファへの出力タイミング
//////////////////////////////////////////////////////////////////////////////
void RSSoundPlayerClass::Listener::OnStreaming( SoundPlayer * player )
{
	ESLAssert( m_vm != NULL ) ;
	RSContext	context( m_vm ) ;
	RSFunctionObject *	pFunc =
			m_pClass->GetVirtualMemberAs( context, L"onStream" ) ;
	if ( pFunc != NULL )
	{
		RSObject *	pArgs[1] ;
		pArgs[0] = new RSNativeObject
					( player, context.GetClassAs( L"SoundPlayer" ) ) ;
		//
		RSObject::ReleaseRef
			( context.CallFunction( *pFunc, this, &pArgs[0], 1, false ) ) ;
		//
		RSObject::ReleaseRef( pArgs[0] ) ;
		RSObject::ReleaseRef( pFunc ) ;
	}
}



//////////////////////////////////////////////////////////////////////////////
// SoundPlayer.Listener クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSSoundPlayerClass::ListenerClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSSoundPlayerClass::ListenerClass::ListenerClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSSoundPlayerClass::ListenerClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSSoundPlayerClass::Listener( context.GetVM(), this ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"onStream", NULL, L"SoundPlayer player",
			NULL, NULL, NULL ) ;
}



//////////////////////////////////////////////////////////////////////////////
// サウンド出力オブジェクトクラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSSoundPlayerClass, RGenericNativeObjectClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSSoundPlayerClass::RSSoundPlayerClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RGenericNativeObjectClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSSoundPlayerClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( NULL, this ) ;
	//
	CreateMemberIntegerAs
		( context, L"formatSoundLinearPCM", formatSoundLinearPCM, modifierConst ) ;
	//
	FormatClass *	pFormatClass = new FormatClass( context.GetClassClass() ) ;
	pFormatClass->Initialize( context ) ;
	context.ReleaseObjectRef
		( CreateMemberAs( context, L"Format", pFormatClass ) ) ;
	//
	ListenerClass *	pListenerClass = new ListenerClass( context.GetClassClass() ) ;
	pListenerClass->Initialize( context ) ;
	context.ReleaseObjectRef
		( CreateMemberAs( context, L"Listener", pListenerClass ) ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"",
				NULL, &RSSoundPlayerClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"open",
				L"boolean", L"SoundPlayer.Format fmt",
				NULL, &RSSoundPlayerClass::method_open, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"close",
				L"boolean", L"",
				NULL, &RSSoundPlayerClass::method_close, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"writeStatic",
				L"boolean", L"Uint8Pointer ptrSound, int nBytes",
				NULL, &RSSoundPlayerClass::method_writeStatic, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"prepareStream",
				L"boolean", L"int nBytes = 0",
				NULL, &RSSoundPlayerClass::method_prepareStream, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"write",
				L"int", L"Uint8Pointer ptrSound, int nBytes",
				NULL, &RSSoundPlayerClass::method_write, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"play",
				L"boolean", L"long nFlags = 0",
				NULL, &RSSoundPlayerClass::method_play, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"stop",
				L"boolean", L"",
				NULL, &RSSoundPlayerClass::method_stop, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"pause",
				L"boolean", L"",
				NULL, &RSSoundPlayerClass::method_pause, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"restart",
				L"boolean", L"",
				NULL, &RSSoundPlayerClass::method_restart, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getVolume",
				L"boolean", L"float[] volumes, int nChannels",
				NULL, &RSSoundPlayerClass::method_getVolume,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setVolume",
				L"boolean", L"float[] volumes, int nChannels",
				NULL, &RSSoundPlayerClass::method_setVolume, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"isPlaying",
				L"boolean", L"",
				NULL, &RSSoundPlayerClass::method_isPlaying,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"isPaused",
				L"boolean", L"",
				NULL, &RSSoundPlayerClass::method_isPaused,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getPlayingPosition",
				L"long", L"",
				NULL, &RSSoundPlayerClass::method_getPlayingPosition,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"seekPosition",
				L"boolean", L"long nPos",
				NULL, &RSSoundPlayerClass::method_seekPosition, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setListener",
				NULL, L"SoundPlayer.Listener listener",
				NULL, &RSSoundPlayerClass::method_setListener, NULL ) ;
}

// ネイティブ型テスト
//////////////////////////////////////////////////////////////////////////////
bool RSSoundPlayerClass::IsNativeObjectOf( ESLObject * pObj ) const
{
	return	(ESLTypeCast<SGLSoundPlayerInterface>(pObj) != nullptr) ;
}

// this オブジェクトのファイルを取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLSoundPlayerInterface *
	RSSoundPlayerClass::GetThisSoundPlayer( RSContext& context, RSObject* pThis )
{
	SGLSoundPlayerInterface *	pSoundPlayer = NULL ;
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj != NULL )
	{
		pSoundPlayer = ESLTypeCast<SGLSoundPlayerInterface>( pNativeObj->GetObject() ) ;
	}
	if ( pSoundPlayer == NULL )
	{
		context.ThrowExceptionError( L"this が SoundPlayer ではありません" ) ;
	}
	return	pSoundPlayer ;
}

// void <init>()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSoundPlayerClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj == NULL )
	{
		context.ThrowExceptionError
			( L"SoundPlayer.<init> の this が SoundPlayer ではありません" ) ;
		return	NULL ;
	}
	pNativeObj->SetObject( new SGLSoundPlayer ) ;
	return	NULL ;
}

// boolean open( SoundPlayer.Format fmt )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSoundPlayerClass::method_open
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSoundPlayerInterface *
			pPlayer = GetThisSoundPlayer( context, pThis ) ;
	if ( pPlayer == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSObject *	pObjFmt = arg.ObjectAt( 0 ) ;
	if ( pObjFmt == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	SGLSoundFormat	fmt ;
	fmt.format = (uint32_t) pObjFmt->GetMemberIntegerAs( context, L"format" ) ;
	fmt.frequency = (uint32_t) pObjFmt->GetMemberIntegerAs( context, L"frequency" ) ;
	fmt.channels = (uint32_t) pObjFmt->GetMemberIntegerAs( context, L"channels" ) ;
	fmt.bitsPerSample = (uint32_t) pObjFmt->GetMemberIntegerAs( context, L"bitsPerSample" ) ;
	if ( pPlayer->Open( fmt ) )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean( true ) ;
}

// boolean close()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSoundPlayerClass::method_close
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSoundPlayerInterface *
			pPlayer = GetThisSoundPlayer( context, pThis ) ;
	if ( pPlayer == NULL )
	{
		return	NULL ;
	}
	if ( pPlayer->Close() )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean( true ) ;
}

// boolean writeStatic( Uint8Pointer ptrSound, int nBytes )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSoundPlayerClass::method_writeStatic
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSoundPlayerInterface *
			pPlayer = GetThisSoundPlayer( context, pThis ) ;
	if ( pPlayer == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSTypedArrayPointer *	pPtrSound =
			ESLTypeCast<RSTypedArrayPointer>( arg.ObjectAt( 0 ) ) ;
	if ( (pPtrSound == NULL) || (pPtrSound->GetPointer() == NULL) )
	{
		return	context.new_Boolean( false ) ;
	}
	if ( pPlayer->WriteStatic
		( pPtrSound->GetPointer(), (size_t) arg.IntAt( 1 ) ) )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean( true ) ;
}

// boolean prepareStream( int nBytes = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSoundPlayerClass::method_prepareStream
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSoundPlayerInterface *
			pPlayer = GetThisSoundPlayer( context, pThis ) ;
	if ( pPlayer == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	if ( pPlayer->PrepareStream( arg.IntAt( 0 ) ) )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean( true ) ;
}

// int write( Uint8Pointer ptrSound, int nBytes )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSoundPlayerClass::method_write
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSoundPlayerInterface *
			pPlayer = GetThisSoundPlayer( context, pThis ) ;
	if ( pPlayer == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSTypedArrayPointer *	pPtrSound =
			ESLTypeCast<RSTypedArrayPointer>( arg.ObjectAt( 0 ) ) ;
	if ( (pPtrSound == NULL) || (pPtrSound->GetPointer() == NULL) )
	{
		return	context.new_Integer( 0 ) ;
	}
	return	context.new_Integer
			( pPlayer->Write
				( pPtrSound->GetPointer(), (size_t) arg.IntAt( 1 ) ) ) ;
}

// boolean play( long nFlags = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSoundPlayerClass::method_play
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSoundPlayerInterface *
			pPlayer = GetThisSoundPlayer( context, pThis ) ;
	if ( pPlayer == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	if ( pPlayer->Play( arg.LongAt( 0 ) ) )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean( true ) ;
}

// boolean stop()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSoundPlayerClass::method_stop
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSoundPlayerInterface *
			pPlayer = GetThisSoundPlayer( context, pThis ) ;
	if ( pPlayer == NULL )
	{
		return	NULL ;
	}
	if ( pPlayer->Stop() )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean( true ) ;
}

// boolean pause()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSoundPlayerClass::method_pause
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSoundPlayerInterface *
			pPlayer = GetThisSoundPlayer( context, pThis ) ;
	if ( pPlayer == NULL )
	{
		return	NULL ;
	}
	if ( pPlayer->Pause() )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean( true ) ;
}

// boolean restart()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSoundPlayerClass::method_restart
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSoundPlayerInterface *
			pPlayer = GetThisSoundPlayer( context, pThis ) ;
	if ( pPlayer == NULL )
	{
		return	NULL ;
	}
	if ( pPlayer->Restart() )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean( true ) ;
}

// boolean getVolume( float[] volumes, int nChannels )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSoundPlayerClass::method_getVolume
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSoundPlayerInterface *
			pPlayer = GetThisSoundPlayer( context, pThis ) ;
	if ( pPlayer == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSArray *	pObjVols = ESLTypeCast<RSArray>( arg.ObjectAt( 0 ) ) ;
	if ( pObjVols == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	SArray<float32_t>	bufVolumes ;
	size_t		nChannels = (size_t) arg.IntAt( 1 ) ;
	float32_t *	pVolumes = bufVolumes.GetArray( nChannels ) ;
	if ( pPlayer->GetVolume( pVolumes, nChannels ) )
	{
		bufVolumes.FinishArray() ;
		return	context.new_Boolean( false ) ;
	}
	for ( size_t i = 0; i < nChannels; i ++ )
	{
		pObjVols->SetElementNumberAt( context, (int) i, pVolumes[i] ) ;
	}
	bufVolumes.FinishArray() ;
	return	context.new_Boolean( true ) ;
}

// boolean setVolume( float[] volumes, int nChannels )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSoundPlayerClass::method_setVolume
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSoundPlayerInterface *
			pPlayer = GetThisSoundPlayer( context, pThis ) ;
	if ( pPlayer == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSArray *	pObjVols = ESLTypeCast<RSArray>( arg.ObjectAt( 0 ) ) ;
	if ( pObjVols == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	SArray<float32_t>	bufVolumes ;
	size_t		nChannels = (size_t) arg.IntAt( 1 ) ;
	float32_t *	pVolumes = bufVolumes.GetArray( nChannels ) ;
	for ( size_t i = 0; i < nChannels; i ++ )
	{
		pVolumes[i] =
			(float32_t) pObjVols->GetElementNumberAt( context, (int) i ) ;
	}
	if ( pPlayer->SetVolume( pVolumes, nChannels ) )
	{
		bufVolumes.FinishArray() ;
		return	context.new_Boolean( false ) ;
	}
	bufVolumes.FinishArray() ;
	return	context.new_Boolean( true ) ;
}

// boolean isPlaying()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSoundPlayerClass::method_isPlaying
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSoundPlayerInterface *
			pPlayer = GetThisSoundPlayer( context, pThis ) ;
	if ( pPlayer == NULL )
	{
		return	NULL ;
	}
	return	context.new_Boolean( pPlayer->IsPlaying() ) ;
}

// boolean isPaused()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSoundPlayerClass::method_isPaused
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSoundPlayerInterface *
			pPlayer = GetThisSoundPlayer( context, pThis ) ;
	if ( pPlayer == NULL )
	{
		return	NULL ;
	}
	return	context.new_Boolean( pPlayer->IsPaused() ) ;
}

// long getPlayingPosition()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSoundPlayerClass::method_getPlayingPosition
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSoundPlayerInterface *
			pPlayer = GetThisSoundPlayer( context, pThis ) ;
	if ( pPlayer == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( pPlayer->GetPlayingPosition() ) ;
}

// boolean seekPosition( long nPos )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSoundPlayerClass::method_seekPosition
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSoundPlayerInterface *
			pPlayer = GetThisSoundPlayer( context, pThis ) ;
	if ( pPlayer == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	if ( pPlayer->SeekPosition( (uint64_t) arg.LongAt( 0 ) ) )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean( true ) ;
}

// void setListener( SoundPlayer.Listener listener )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSoundPlayerClass::method_setListener
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSoundPlayerInterface *
			pPlayer = GetThisSoundPlayer( context, pThis ) ;
	if ( pPlayer == NULL )
	{
		return	NULL ;
	}
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	ESLAssert( pNativeObj != NULL ) ;
	//
	RSObject *	pLastListener =
		pNativeObj->FindOwnObject( ESL_RUNTIME_CLASS(Listener) ) ;
	//
	RSContext::SArgList	arg( ppArg, count ) ;
	Listener *	pListener = ESLTypeCast<Listener>( arg.ObjectAt( 0 ) ) ;
	if ( pListener != NULL )
	{
		pPlayer->SetListener( pListener ) ;
		pNativeObj->AddOwnObject( pListener ) ;
	}
	else
	{
		pPlayer->SetListener( NULL ) ;
	}
	if ( pLastListener != NULL )
	{
		pNativeObj->ReleaseOwnObject( pLastListener ) ;
	}
	return	NULL ;
}



//////////////////////////////////////////////////////////////////////////////
// オーディオファイル再生クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSAudioPlayerClass, RGenericNativeObjectClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSAudioPlayerClass::RSAudioPlayerClass
	( RSClass * pClass, const wchar_t * pwszClassName )
	: RGenericNativeObjectClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSAudioPlayerClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( NULL, this ) ;
	//
	CreateMemberIntegerAs
		( context, L"modeOpenAuto",
			SGLAudioPlayerInterface::modeOpenAuto, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"modeOpenStatic",
			SGLAudioPlayerInterface::modeOpenStatic, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"modeOpenAutoStatic",
			SGLAudioPlayerInterface::modeOpenAutoStatic, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"modeOpenDynamicOnMemory",
			SGLAudioPlayerInterface::modeOpenDynamicOnMemory, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"modeOpenDynamicRead",
			SGLAudioPlayerInterface::modeOpenDynamicRead, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"modeOpenMask",
			SGLAudioPlayerInterface::modeOpenMask, modifierConst ) ;
	//
	CreateMemberIntegerAs
		( context, L"lineTotal",
			SGLAudioPlayer::lineTotal, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"lineTotal2nd",
			SGLAudioPlayer::lineTotal2nd, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"lineComposition",
			SGLAudioPlayer::lineComposition, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"lineSystem",
			SGLAudioPlayer::lineSystem, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"lineMusic",
			SGLAudioPlayer::lineMusic, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"lineSound",
			SGLAudioPlayer::lineSound, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"lineVoice",
			SGLAudioPlayer::lineVoice, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"lineUserFirst",
			SGLAudioPlayer::lineUserFirst, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"lineCount",
			SGLAudioPlayer::lineCount, modifierConst ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"",
				NULL, &RSAudioPlayerClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"open",
				L"boolean", L"String path, long nFlags = 0",
				NULL, &RSAudioPlayerClass::method_open, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"create",
				L"boolean", L"RandomAccessFile file, long nFlags = 0",
				NULL, &RSAudioPlayerClass::method_create, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"clonePlayer",
				L"AudioPlayer", L"",
				NULL, &RSAudioPlayerClass::method_clonePlayer, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"close",
				L"boolean", L"",
				NULL, &RSAudioPlayerClass::method_close, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"play",
				L"boolean", L"long nFlags = 0",
				NULL, &RSAudioPlayerClass::method_play, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"stop",
				L"boolean", L"",
				NULL, &RSAudioPlayerClass::method_stop, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setLoop",
				L"boolean", L"boolean flagLoop = true,"
							L" long nStart = -1, long nEnd = -1",
				NULL, &RSAudioPlayerClass::method_setLoop, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"pause",
				L"boolean", L"",
				NULL, &RSAudioPlayerClass::method_pause, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"restart",
				L"boolean", L"",
				NULL, &RSAudioPlayerClass::method_restart, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getVolume",
				L"boolean", L"float[] pVolumes, int nChannels",
				NULL, &RSAudioPlayerClass::method_getVolume, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setVolume",
				L"boolean", L"float[] pVolumes, int nChannels",
				NULL, &RSAudioPlayerClass::method_setVolume, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"isPlaying",
				L"boolean", L"",
				NULL, &RSAudioPlayerClass::method_isPlaying, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"isPaused",
				L"boolean", L"",
				NULL, &RSAudioPlayerClass::method_isPaused, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getSampleFrequency",
				L"int", L"",
				NULL, &RSAudioPlayerClass::method_getSampleFrequency, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getTotalLength",
				L"long", L"",
				NULL, &RSAudioPlayerClass::method_getTotalLength, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getPosition",
				L"long", L"",
				NULL, &RSAudioPlayerClass::method_getPosition, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"seekPosition",
				NULL, L"long nPos",
				NULL, &RSAudioPlayerClass::method_seekPosition, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"beginFadeVolume",
				NULL, L"float[] pVolumes, int nChannels, int msecDuration",
				NULL, &RSAudioPlayerClass::method_beginFadeVolume, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"isVolumeFading",
				L"boolean", L"",
				NULL, &RSAudioPlayerClass::method_isVolumeFading,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"cancelFadeVolume",
				NULL, L"",
				NULL, &RSAudioPlayerClass::method_cancelFadeVolume, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"flushFadeVolume",
				NULL, L"",
				NULL, &RSAudioPlayerClass::method_flushFadeVolume, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getVolumeLineMask",
				L"long", L"",
				NULL, &RSAudioPlayerClass::method_getVolumeLineMask,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setVolumeLineMask",
				NULL, L"long maskLines",
				NULL, &RSAudioPlayerClass::method_setVolumeLineMask, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setVolumeLine",
				NULL, L"int iLine",
				NULL, &RSAudioPlayerClass::method_setVolumeLine, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"resetVolumeLine",
				NULL, L"int iLine",
				NULL, &RSAudioPlayerClass::method_resetVolumeLine, NULL ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"setLineVolume",
				NULL, L"int iLine, double volume",
				NULL, &RSAudioPlayerClass::method_setLineVolume, NULL ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"getLineVolume",
				L"double", L"int iLine",
				NULL, &RSAudioPlayerClass::method_getLineVolume, NULL ) ;
}

// ネイティブ型テスト
//////////////////////////////////////////////////////////////////////////////
bool RSAudioPlayerClass::IsNativeObjectOf( ESLObject * pObj ) const
{
	return	(ESLTypeCast<SGLAudioPlayer>(pObj) != nullptr) ;
}

// this オブジェクトのファイルを取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLAudioPlayer *
	RSAudioPlayerClass::GetThisAudioPlayer( RSContext& context, RSObject* pThis )
{
	SGLAudioPlayer *	pAudioPlayer = NULL ;
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj != NULL )
	{
		pAudioPlayer = ESLTypeCast<SGLAudioPlayer>( pNativeObj->GetObject() ) ;
	}
	if ( pAudioPlayer == NULL )
	{
		context.ThrowExceptionError( L"this が AudioPlayer ではありません" ) ;
	}
	return	pAudioPlayer ;
}

// void <init>()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSAudioPlayerClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj == NULL )
	{
		context.ThrowExceptionError
			( L"AudioPlayer.<init> の this が AudioPlayer ではありません" ) ;
		return	NULL ;
	}
	pNativeObj->SetObject( new SGLAudioPlayer ) ;
	return	NULL ;
}

// boolean open( String path, long nFlags = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSAudioPlayerClass::method_open
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLAudioPlayer *
			pPlayer = GetThisAudioPlayer( context, pThis ) ;
	if ( pPlayer == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Boolean
				( pPlayer->Open( arg.StringAt(0), arg.LongAt(1) ) == sglErrSuccess ) ;
}

// boolean create( RandomAccessFile file, long nFlags = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSAudioPlayerClass::method_create
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLAudioPlayer *	pPlayer = GetThisAudioPlayer( context, pThis ) ;
	if ( pPlayer == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SFileInterface *
		pfile = ESLTypeCast<SFileInterface>( arg.NativeObjectAt( 0 ) ) ;
	if ( pfile == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	SGLError	err = pPlayer->Create( pfile, false, arg.LongAt( 1 ) ) ;
	if ( !err )
	{
		RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
		if ( pNativeObj != NULL )
		{
			pNativeObj->AddOwnObject( arg.ObjectAt(0) ) ;
		}
	}
	return	context.new_Boolean( err == sglErrSuccess ) ;
}

// AudioPlayer clonePlayer()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSAudioPlayerClass::method_clonePlayer
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLAudioPlayer *	pPlayer = GetThisAudioPlayer( context, pThis ) ;
	if ( pPlayer == NULL )
	{
		return	NULL ;
	}
	SGLAudioPlayerInterface *	pClone = pPlayer->ClonePlayer() ;
	if ( pClone == NULL )
	{
		return	NULL ;
	}
	RSObject *	pObj = context.new_Object( L"AudioPlayer" ) ;
	if ( pObj == NULL )
	{
		delete	pClone ;
		return	NULL ;
	}
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pObj ) ;
	if ( pNativeObj != NULL )
	{
		delete	pClone ;
		return	pObj ;
	}
	pNativeObj->SetObject( pClone ) ;
	return	pObj ;
}

// boolean close()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSAudioPlayerClass::method_close
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLAudioPlayer *	pPlayer = GetThisAudioPlayer( context, pThis ) ;
	if ( pPlayer == NULL )
	{
		return	NULL ;
	}
	return	context.new_Boolean( pPlayer->Close() == sglErrSuccess ) ;
}

// boolean play( long nFlags = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSAudioPlayerClass::method_play
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLAudioPlayer *	pPlayer = GetThisAudioPlayer( context, pThis ) ;
	if ( pPlayer == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Boolean
				( pPlayer->Play( arg.LongAt(0) ) == sglErrSuccess ) ;
}

// boolean stop()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSAudioPlayerClass::method_stop
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLAudioPlayer *	pPlayer = GetThisAudioPlayer( context, pThis ) ;
	if ( pPlayer == NULL )
	{
		return	NULL ;
	}
	return	context.new_Boolean( pPlayer->Stop() == sglErrSuccess ) ;
}

// boolean setLoop( boolean flagLoop = true, long nStart = -1, long nEnd = -1 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSAudioPlayerClass::method_setLoop
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLAudioPlayer *	pPlayer = GetThisAudioPlayer( context, pThis ) ;
	if ( pPlayer == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Boolean
		( pPlayer->SetLoop
			( arg.BooleanAt(0),
				arg.LongAt(1,-1), arg.LongAt(2,-1) ) == sglErrSuccess ) ;
}

// boolean pause()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSAudioPlayerClass::method_pause
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLAudioPlayer *	pPlayer = GetThisAudioPlayer( context, pThis ) ;
	if ( pPlayer == NULL )
	{
		return	NULL ;
	}
	return	context.new_Boolean( pPlayer->Pause() == sglErrSuccess ) ;
}

// boolean restart()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSAudioPlayerClass::method_restart
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLAudioPlayer *	pPlayer = GetThisAudioPlayer( context, pThis ) ;
	if ( pPlayer == NULL )
	{
		return	NULL ;
	}
	return	context.new_Boolean( pPlayer->Restart() == sglErrSuccess ) ;
}

// boolean getVolume( float[] pVolumes, int nChannels )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSAudioPlayerClass::method_getVolume
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLAudioPlayer *	pPlayer = GetThisAudioPlayer( context, pThis ) ;
	if ( pPlayer == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSObject *	pObjVols = arg.ObjectAt( 0 ) ;
	if ( pObjVols == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	SArray<float32_t>	aVolumes ;
	size_t		nChannels = (size_t) arg.LongAt( 1 ) ;
	float32_t *	pVolumes = aVolumes.GetArray( nChannels ) ;
	//
	SGLError	err = pPlayer->GetVolume( pVolumes, nChannels ) ;
	if ( !err )
	{
		for ( size_t i = 0; i < nChannels; i ++ )
		{
			pObjVols->SetElementNumberAt( context, (int) i, pVolumes[i] ) ;
		}
	}
	aVolumes.FinishArray() ;
	return	context.new_Boolean( err == sglErrSuccess ) ;
}

// boolean setVolume( float[] pVolumes, int nChannels )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSAudioPlayerClass::method_setVolume
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLAudioPlayer *	pPlayer = GetThisAudioPlayer( context, pThis ) ;
	if ( pPlayer == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSObject *	pObjVols = arg.ObjectAt( 0 ) ;
	if ( pObjVols == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	SArray<float32_t>	aVolumes ;
	size_t		nChannels = (size_t) arg.LongAt( 1 ) ;
	float32_t *	pVolumes = aVolumes.GetArray( nChannels ) ;
	for ( size_t i = 0; i < nChannels; i ++ )
	{
		pVolumes[i] =
			(float32_t) pObjVols->GetElementNumberAt( context, (int) i ) ;
	}
	SGLError	err = pPlayer->SetVolume( pVolumes, nChannels ) ;
	aVolumes.FinishArray() ;
	return	context.new_Boolean( err == sglErrSuccess ) ;
}

// boolean isPlaying()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSAudioPlayerClass::method_isPlaying
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLAudioPlayer *	pPlayer = GetThisAudioPlayer( context, pThis ) ;
	if ( pPlayer == NULL )
	{
		return	NULL ;
	}
	return	context.new_Boolean( pPlayer->IsPlaying() ) ;
}

// boolean isPaused()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSAudioPlayerClass::method_isPaused
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLAudioPlayer *	pPlayer = GetThisAudioPlayer( context, pThis ) ;
	if ( pPlayer == NULL )
	{
		return	NULL ;
	}
	return	context.new_Boolean( pPlayer->IsPaused() ) ;
}

// int getSampleFrequency()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSAudioPlayerClass::method_getSampleFrequency
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLAudioPlayer *	pPlayer = GetThisAudioPlayer( context, pThis ) ;
	if ( pPlayer == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( pPlayer->GetSampleFrequency() ) ;
}

// long getTotalLength()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSAudioPlayerClass::method_getTotalLength
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLAudioPlayer *	pPlayer = GetThisAudioPlayer( context, pThis ) ;
	if ( pPlayer == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( (int64_t) pPlayer->GetTotalLength() ) ;
}

// long getPosition()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSAudioPlayerClass::method_getPosition
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLAudioPlayer *	pPlayer = GetThisAudioPlayer( context, pThis ) ;
	if ( pPlayer == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( (int64_t) pPlayer->GetPosition() ) ;
}

// void seekPosition( long nPos )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSAudioPlayerClass::method_seekPosition
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLAudioPlayer *	pPlayer = GetThisAudioPlayer( context, pThis ) ;
	if ( pPlayer == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pPlayer->SeekPosition( (uint64_t) arg.LongAt(0) ) ;
	return	NULL ;
}

// void beginFadeVolume( float[] pVolumes, int nChannels, int msecDuration )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSAudioPlayerClass::method_beginFadeVolume
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLAudioPlayer *	pPlayer = GetThisAudioPlayer( context, pThis ) ;
	if ( pPlayer == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSObject *	pObjVols = arg.ObjectAt( 0 ) ;
	if ( pObjVols == NULL )
	{
		return	NULL ;
	}
	SArray<float32_t>	aVolumes ;
	size_t		nChannels = (size_t) arg.LongAt( 1 ) ;
	float32_t *	pVolumes = aVolumes.GetArray( nChannels ) ;
	for ( size_t i = 0; i < nChannels; i ++ )
	{
		pVolumes[i] =
			(float32_t) pObjVols->GetElementNumberAt( context, (int) i ) ;
	}
	pPlayer->BeginFadeVolume
		( pVolumes, nChannels, (uint32_t) arg.IntAt(2) ) ;
	aVolumes.FinishArray() ;
	return	NULL ;
}

// const boolean isVolumeFading()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSAudioPlayerClass::method_isVolumeFading
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLAudioPlayer *	pPlayer = GetThisAudioPlayer( context, pThis ) ;
	if ( pPlayer == NULL )
	{
		return	NULL ;
	}
	return	context.new_Boolean( pPlayer->IsVolumeFading() ) ;
}

// void cancelFadeVolume()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSAudioPlayerClass::method_cancelFadeVolume
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLAudioPlayer *	pPlayer = GetThisAudioPlayer( context, pThis ) ;
	if ( pPlayer == NULL )
	{
		return	NULL ;
	}
	pPlayer->CancelFadeVolume() ;
	return	NULL ;
}

// void flushFadeVolume()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSAudioPlayerClass::method_flushFadeVolume
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLAudioPlayer *	pPlayer = GetThisAudioPlayer( context, pThis ) ;
	if ( pPlayer == NULL )
	{
		return	NULL ;
	}
	pPlayer->FlushFadeVolume() ;
	return	NULL ;
}

// const long getVolumeLineMask()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSAudioPlayerClass::method_getVolumeLineMask
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLAudioPlayer *	pPlayer = GetThisAudioPlayer( context, pThis ) ;
	if ( pPlayer == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( pPlayer->GetVolumeLineMask() ) ;
}

// void setVolumeLineMask( long maskLines )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSAudioPlayerClass::method_setVolumeLineMask
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLAudioPlayer *	pPlayer = GetThisAudioPlayer( context, pThis ) ;
	if ( pPlayer == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pPlayer->SetVolumeLineMask( (uint32_t) arg.LongAt( 0 ) ) ;
	return	NULL ;
}

// void setVolumeLine( int iLine )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSAudioPlayerClass::method_setVolumeLine
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLAudioPlayer *	pPlayer = GetThisAudioPlayer( context, pThis ) ;
	if ( pPlayer == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pPlayer->SetVolumeLine( (size_t) arg.LongAt( 0 ) ) ;
	return	NULL ;
}

// void resetVolumeLine( int iLine )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSAudioPlayerClass::method_resetVolumeLine
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLAudioPlayer *	pPlayer = GetThisAudioPlayer( context, pThis ) ;
	if ( pPlayer == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pPlayer->ResetVolumeLine( (size_t) arg.LongAt( 0 ) ) ;
	return	NULL ;
}

// static void setLineVolume( int iLine, double volume )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSAudioPlayerClass::method_setLineVolume
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLAudioPlayer::SetLineVolume( (size_t) arg.LongAt(0), arg.DoubleAt(1) ) ;
	return	NULL ;
}

// static double getLineVolume( int iLine )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSAudioPlayerClass::method_getLineVolume
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Number
				( SGLAudioPlayer::GetLineVolume( (size_t) arg.LongAt(0) ) ) ;
}


//////////////////////////////////////////////////////////////////////////////
// MediaOptionalInfo クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSMediaOptionalInfoClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSMediaOptionalInfoClass::RSMediaOptionalInfoClass
	( RSClass * pClass, const wchar_t * pwszClassName )
: RSClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSMediaOptionalInfoClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSGenericObject( this ) ;
	//
	m_pPrototype->CreateMemberIntegerAs( context, L"m_nFlags", 0 ) ;
	m_pPrototype->CreateMemberIntegerAs( context, L"m_nLoopStart", 0 ) ;
	m_pPrototype->CreateMemberIntegerAs( context, L"m_nLoopEnd", 0 ) ;
	m_pPrototype->CreateMemberStringAs( context, L"m_strTitle", NULL ) ;
	m_pPrototype->CreateMemberStringAs( context, L"m_strPlayer", NULL ) ;
	m_pPrototype->CreateMemberStringAs( context, L"m_strComposer", NULL ) ;
	m_pPrototype->CreateMemberStringAs( context, L"m_strArranger", NULL ) ;
	//
	CreateMemberIntegerAs
		( context, L"flagLoopStart",
			SGLMediaOptionalInfo::flagLoopStart, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"flagLoopEnd",
			SGLMediaOptionalInfo::flagLoopEnd, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"flagTitle",
			SGLMediaOptionalInfo::flagTitle, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"flagVocalPlayer",
			SGLMediaOptionalInfo::flagVocalPlayer, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"flagComposer",
			SGLMediaOptionalInfo::flagComposer, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"flagArranger",
			SGLMediaOptionalInfo::flagArranger, modifierConst ) ;
}

// Object -> SGLMediaOptionalInfo 変換
//////////////////////////////////////////////////////////////////////////////
void RSMediaOptionalInfoClass::FromObject
	( RSContext& context,
		SGLMediaOptionalInfo& optinf, RSObject * pObj )
{
	optinf.m_nFlags =
		pObj->GetMemberIntegerAs
			( context, L"m_nFlags", optinf.m_nFlags ) ;
	optinf.m_nLoopStart =
		pObj->GetMemberIntegerAs
			( context, L"m_nLoopStart", optinf.m_nLoopStart ) ;
	optinf.m_nLoopEnd =
		pObj->GetMemberIntegerAs
			( context, L"m_nLoopEnd", optinf.m_nLoopEnd ) ;
	optinf.m_strTitle =
		pObj->GetMemberStringAs
			( context, L"m_strTitle", optinf.m_strTitle ) ;
	optinf.m_strPlayer =
		pObj->GetMemberStringAs
			( context, L"m_strPlayer", optinf.m_strPlayer ) ;
	optinf.m_strComposer =
		pObj->GetMemberStringAs
			( context, L"m_strComposer", optinf.m_strComposer ) ;
	optinf.m_strArranger =
		pObj->GetMemberStringAs
			( context, L"m_strArranger", optinf.m_strArranger ) ;
}

// Object <- SGLMediaOptionalInfo 変換
//////////////////////////////////////////////////////////////////////////////
void RSMediaOptionalInfoClass::ToObject
	( RSContext& context,
		RSObject * pObj, const SGLMediaOptionalInfo& optinf )
{
	pObj->SetMemberIntegerAs
		( context, L"m_nFlags", optinf.m_nFlags ) ;
	pObj->SetMemberIntegerAs
		( context, L"m_nLoopStart", optinf.m_nLoopStart ) ;
	pObj->SetMemberIntegerAs
		( context, L"m_nLoopEnd", optinf.m_nLoopEnd ) ;
	pObj->SetMemberStringAs
		( context, L"m_strTitle", optinf.m_strTitle ) ;
	pObj->SetMemberStringAs
		( context, L"m_strPlayer", optinf.m_strPlayer ) ;
	pObj->SetMemberStringAs
		( context, L"m_strComposer", optinf.m_strComposer ) ;
	pObj->SetMemberStringAs
		( context, L"m_strArranger", optinf.m_strArranger ) ;
}



//////////////////////////////////////////////////////////////////////////////
// AudioInputStream クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSAudioInputStreamClass, RGenericNativeObjectClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSAudioInputStreamClass::RSAudioInputStreamClass
	( RSClass * pClass, const wchar_t * pwszClassName )
	: RGenericNativeObjectClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSAudioInputStreamClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( NULL, this ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"getAudioFormat",
				L"boolean", L"SoundPlayer.Format fmt",
				NULL, &RSAudioInputStreamClass::method_getAudioFormat, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getAudioOptinalInfo",
				L"boolean", L"MediaOptionalInfo optinf",
				NULL, &RSAudioInputStreamClass::method_getAudioOptinalInfo, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getAudioLength", L"long", L"",
				NULL, &RSAudioInputStreamClass::method_getAudioLength,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"readAudio",
				L"int", L"Uint8Pointer buf, int nSamples",
				NULL, &RSAudioInputStreamClass::method_readAudio, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"seekAudio",
				L"boolean", L"long nSamples",
				NULL, &RSAudioInputStreamClass::method_seekAudio, NULL ) ;
}

// ネイティブ型テスト
//////////////////////////////////////////////////////////////////////////////
bool RSAudioInputStreamClass::IsNativeObjectOf( ESLObject * pObj ) const
{
	return	(ESLTypeCast<SGLAudioInputStream>(pObj) != nullptr) ;
}

// this オブジェクトのファイルを取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLAudioInputStream *
	RSAudioInputStreamClass::GetThisAudioInputStream
					( RSContext& context, RSObject* pThis )
{
	SGLAudioInputStream *	pAudio = NULL ;
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj != NULL )
	{
		pAudio = ESLTypeCast<SGLAudioInputStream>( pNativeObj->GetObject() ) ;
	}
	if ( pAudio == NULL )
	{
		context.ThrowExceptionError( L"this が AudioInputStream ではありません" ) ;
	}
	return	pAudio ;
}

// boolean getAudioFormat( SoundPlayer.Format fmt )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSAudioInputStreamClass::method_getAudioFormat
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLAudioInputStream *
			pAudio = GetThisAudioInputStream( context, pThis ) ;
	if ( pAudio == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSObject *	pObjFmt = arg.ObjectAt( 0 ) ;
	if ( pObjFmt == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	SGLSoundFormat	fmt ;
	if ( pAudio->GetAudioFormat( fmt ) )
	{
		return	context.new_Boolean( false ) ;
	}
	pObjFmt->SetMemberIntegerAs( context, L"format", fmt.format ) ;
	pObjFmt->SetMemberIntegerAs( context, L"frequency", fmt.frequency ) ;
	pObjFmt->SetMemberIntegerAs( context, L"channels", fmt.channels ) ;
	pObjFmt->SetMemberIntegerAs( context, L"bitsPerSample", fmt.bitsPerSample ) ;
	return	context.new_Boolean( true ) ;
}

// boolean getAudioOptinalInfo( MediaOptionalInfo optinf )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSAudioInputStreamClass::method_getAudioOptinalInfo
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLAudioInputStream *
			pAudio = GetThisAudioInputStream( context, pThis ) ;
	if ( pAudio == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSObject *	pObjOpt = arg.ObjectAt( 0 ) ;
	if ( pObjOpt == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	SGLMediaOptionalInfo	optinf ;
	if ( pAudio->GetAudioOptinalInfo( optinf ) )
	{
		return	context.new_Boolean( false ) ;
	}
	RSMediaOptionalInfoClass::ToObject( context, pObjOpt, optinf ) ;
	return	context.new_Boolean( true ) ;
}

// const long getAudioLength()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSAudioInputStreamClass::method_getAudioLength
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLAudioInputStream *
			pAudio = GetThisAudioInputStream( context, pThis ) ;
	if ( pAudio == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( pAudio->GetAudioLength() ) ;
}

// int readAudio( Uint8Pointer buf, int nSamples )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSAudioInputStreamClass::method_readAudio
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLAudioInputStream *
			pAudio = GetThisAudioInputStream( context, pThis ) ;
	if ( pAudio == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	size_t		nSamples = (size_t) arg.IntAt( 1 ) ;
	uint8_t *	ptrBuf = arg.PointerAt( 0 ) ;
	return	context.new_Integer( pAudio->ReadAudio( ptrBuf, nSamples ) ) ;
}

// boolean seekAudio( long nSamples )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSAudioInputStreamClass::method_seekAudio
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLAudioInputStream *
			pAudio = GetThisAudioInputStream( context, pThis ) ;
	if ( pAudio == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Boolean
				( pAudio->SeekAudio( arg.LongAt(0) ) == sglErrSuccess ) ;
}



//////////////////////////////////////////////////////////////////////////////
// AudioOutputStream クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSAudioOutputStreamClass, RGenericNativeObjectClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSAudioOutputStreamClass::RSAudioOutputStreamClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RGenericNativeObjectClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSAudioOutputStreamClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( NULL, this ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"prepareAudio",
				L"boolean",
				L"SoundPlayer.Format fmt, long nSamples = -1, "
				L"MediaOptionalInfo optinf = null",
				NULL, &RSAudioOutputStreamClass::method_prepareAudio, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"writeAudio",
				L"int", L"Uint8Pointer buf, int nSamples",
				NULL, &RSAudioOutputStreamClass::method_writeAudio, NULL ) ;
}

// ネイティブ型テスト
//////////////////////////////////////////////////////////////////////////////
bool RSAudioOutputStreamClass::IsNativeObjectOf( ESLObject * pObj ) const
{
	return	(ESLTypeCast<SGLAudioOutputStream>(pObj) != nullptr) ;
}

// this オブジェクトのファイルを取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLAudioOutputStream *
	RSAudioOutputStreamClass::GetThisAudioOutputStream
						( RSContext& context, RSObject* pThis )
{
	SGLAudioOutputStream *	pAudio = NULL ;
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj != NULL )
	{
		pAudio = ESLTypeCast<SGLAudioOutputStream>( pNativeObj->GetObject() ) ;
	}
	if ( pAudio == NULL )
	{
		context.ThrowExceptionError( L"this が AudioOutputStream ではありません" ) ;
	}
	return	pAudio ;
}

// boolean prepareAudio
//	( SoundPlayer.Format fmt,
//		long nSamples = -1, MediaOptionalInfo optinf = null )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSAudioOutputStreamClass::method_prepareAudio
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLAudioOutputStream *
			pAudio = GetThisAudioOutputStream( context, pThis ) ;
	if ( pAudio == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSObject *	pObjFmt = arg.ObjectAt( 0 ) ;
	if ( pObjFmt == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	SGLSoundFormat	fmt ;
	fmt.format =
		(uint32_t) pObjFmt->GetMemberIntegerAs( context, L"format" ) ;
	fmt.frequency =
		(uint32_t) pObjFmt->GetMemberIntegerAs( context, L"frequency" ) ;
	fmt.channels =
		(uint32_t) pObjFmt->GetMemberIntegerAs( context, L"channels" ) ;
	fmt.bitsPerSample =
		(uint32_t) pObjFmt->GetMemberIntegerAs( context, L"bitsPerSample" ) ;
	//
	SGLMediaOptionalInfo	optTemp ;
	SGLMediaOptionalInfo *	pOptInf = NULL ;
	RSObject *	pObjOpt = arg.ObjectAt( 2 ) ;
	if ( pObjOpt != NULL )
	{
		RSMediaOptionalInfoClass::FromObject( context, optTemp, pObjOpt ) ;
		pOptInf = &optTemp ;
	}
	//
	return	context.new_Boolean
				( pAudio->PrepareAudio
					( fmt, arg.LongAt(1,-1), pOptInf ) == sglErrSuccess ) ;
}

// int writeAudio( Uint8Pointer buf, int nSamples )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSAudioOutputStreamClass::method_writeAudio
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLAudioOutputStream *
			pAudio = GetThisAudioOutputStream( context, pThis ) ;
	if ( pAudio == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	size_t		nSamples = (size_t) arg.IntAt( 1 ) ;
	uint8_t *	ptrBuf = arg.PointerAt( 0 ) ;
	return	context.new_Integer( pAudio->WriteAudio( ptrBuf, nSamples ) ) ;
}



//////////////////////////////////////////////////////////////////////////////
// VideoInputStream クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSVideoInputStreamClass, RGenericNativeObjectClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSVideoInputStreamClass::RSVideoInputStreamClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RGenericNativeObjectClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSVideoInputStreamClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( NULL, this ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"getImageFormat",
				L"boolean", L"Image.BufferInfo fmt",
				NULL, &RSVideoInputStreamClass::method_getImageFormat, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getVideoOptinalInfo",
				L"boolean", L"MediaOptionalInfo optinf",
				NULL, &RSVideoInputStreamClass::method_getVideoOptinalInfo, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getVideoLength", L"long", L"",
				NULL, &RSVideoInputStreamClass::method_getVideoLength,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getVideoDuration", L"long", L"",
				NULL, &RSVideoInputStreamClass::method_getVideoDuration,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"readFrame",
				L"boolean", L"Image img",
				NULL, &RSVideoInputStreamClass::method_readFrame, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"seekFrame",
				L"boolean", L"long nFrames",
				NULL, &RSVideoInputStreamClass::method_seekFrame, NULL ) ;
}

// ネイティブ型テスト
//////////////////////////////////////////////////////////////////////////////
bool RSVideoInputStreamClass::IsNativeObjectOf( ESLObject * pObj ) const
{
	return	(ESLTypeCast<SGLVideoInputStream>(pObj) != nullptr) ;
}

// this オブジェクトのファイルを取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLVideoInputStream *
	RSVideoInputStreamClass::GetThisVideoInputStream( RSContext& context, RSObject* pThis )
{
	SGLVideoInputStream *	pVideo = NULL ;
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj != NULL )
	{
		pVideo = ESLTypeCast<SGLVideoInputStream>( pNativeObj->GetObject() ) ;
	}
	if ( pVideo == NULL )
	{
		context.ThrowExceptionError( L"this が VideoInputStream ではありません" ) ;
	}
	return	pVideo ;
}

// boolean getImageFormat( Image.BufferInfo fmt )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVideoInputStreamClass::method_getImageFormat
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLVideoInputStream *
			pVideo = GetThisVideoInputStream( context, pThis ) ;
	if ( pVideo == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLImageInfo *	pFmt =
		(SGLImageInfo*) arg.PointerAt( 0, sizeof(SGLImageInfo) ) ;
	if ( pFmt == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean
				( pVideo->GetImageFormat( *pFmt ) == sglErrSuccess ) ;
}

// boolean getVideoOptinalInfo( MediaOptionalInfo optinf )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVideoInputStreamClass::method_getVideoOptinalInfo
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLVideoInputStream *
			pVideo = GetThisVideoInputStream( context, pThis ) ;
	if ( pVideo == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSObject *	pObjOpt = arg.ObjectAt( 0 ) ;
	if ( pObjOpt == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	SGLMediaOptionalInfo	optinf ;
	if ( pVideo->GetVideoOptinalInfo( optinf ) )
	{
		return	context.new_Boolean( false ) ;
	}
	RSMediaOptionalInfoClass::ToObject( context, pObjOpt, optinf ) ;
	return	context.new_Boolean( true ) ;
}

// const long getVideoLength()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVideoInputStreamClass::method_getVideoLength
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLVideoInputStream *
			pVideo = GetThisVideoInputStream( context, pThis ) ;
	if ( pVideo == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( pVideo->GetVideoLength() ) ;
}

// const long getVideoDuration()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVideoInputStreamClass::method_getVideoDuration
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLVideoInputStream *
			pVideo = GetThisVideoInputStream( context, pThis ) ;
	if ( pVideo == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( pVideo->GetVideoDuration() ) ;
}

// boolean readFrame( Image img )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVideoInputStreamClass::method_readFrame
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLVideoInputStream *
			pVideo = GetThisVideoInputStream( context, pThis ) ;
	if ( pVideo == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSObject *	pObjImage = arg.ObjectAt( 0 ) ;
	SGLImageObject *
		pImage = RSImageClass::ImageFromObject( context, pObjImage ) ;
	if ( pImage == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	SGLImageInfo	imginf ;
	uint8_t *	pbytBuf =
		pImage->LockBuffer( imginf, SGLImageObject::lockWrite ) ;
	if ( pbytBuf == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	SGLError	err = pVideo->ReadFrame( imginf, pbytBuf ) ;
	pImage->UnlockBuffer( SGLImageObject::lockWrite ) ;
	//
	return	context.new_Boolean( err == sglErrSuccess ) ;
}

// boolean seekFrame( long nFrames )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVideoInputStreamClass::method_seekFrame
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLVideoInputStream *
			pVideo = GetThisVideoInputStream( context, pThis ) ;
	if ( pVideo == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Boolean
				( pVideo->SeekFrame( arg.LongAt(0) ) == sglErrSuccess ) ;
}


//////////////////////////////////////////////////////////////////////////////
// VideoOutputStream クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSVideoOutputStreamClass, RGenericNativeObjectClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSVideoOutputStreamClass::RSVideoOutputStreamClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RGenericNativeObjectClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSVideoOutputStreamClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( NULL, this ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"prepareVideo",
				L"boolean",
				L"Image.BufferInfo fmt, long nFrames, long nDuration, "
				L"MediaOptionalInfo optinf = null",
				NULL, &RSVideoOutputStreamClass::method_prepareVideo, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"writeFrame",
				L"boolean", L"Image img",
				NULL, &RSVideoOutputStreamClass::method_writeFrame, NULL ) ;
}

// ネイティブ型テスト
//////////////////////////////////////////////////////////////////////////////
bool RSVideoOutputStreamClass::IsNativeObjectOf( ESLObject * pObj ) const
{
	return	(ESLTypeCast<SGLVideoOutputStream>(pObj) != nullptr) ;
}

// this オブジェクトのファイルを取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLVideoOutputStream *
	RSVideoOutputStreamClass::GetThisVideoOutputStream( RSContext& context, RSObject* pThis )
{
	SGLVideoOutputStream *	pVideo = NULL ;
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj != NULL )
	{
		pVideo = ESLTypeCast<SGLVideoOutputStream>( pNativeObj->GetObject() ) ;
	}
	if ( pVideo == NULL )
	{
		context.ThrowExceptionError( L"this が VideoOutputStream ではありません" ) ;
	}
	return	pVideo ;
}

// boolean prepareVideo
//	( Image.BufferInfo fmt,
//		long nFrames, long nDuration,
//		MediaOptionalInfo optinf = null )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVideoOutputStreamClass::method_prepareVideo
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLVideoOutputStream *
			pVideo = GetThisVideoOutputStream( context, pThis ) ;
	if ( pVideo == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLImageInfo *	pFmt =
		(SGLImageInfo*) arg.PointerAt( 0, sizeof(SGLImageInfo) ) ;
	if ( pFmt == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	SGLMediaOptionalInfo	optTemp ;
	SGLMediaOptionalInfo *	pOptInf = NULL ;
	RSObject *	pObjOpt = arg.ObjectAt( 3 ) ;
	if ( pObjOpt != NULL )
	{
		RSMediaOptionalInfoClass::FromObject( context, optTemp, pObjOpt ) ;
		pOptInf = &optTemp ;
	}
	return	context.new_Boolean
				( pVideo->PrepareVideo
					( *pFmt, arg.LongAt(1,0),
						arg.LongAt(2,0), pOptInf ) == sglErrSuccess ) ;
}

// boolean writeFrame( Image img )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVideoOutputStreamClass::method_writeFrame
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLVideoOutputStream *
			pVideo = GetThisVideoOutputStream( context, pThis ) ;
	if ( pVideo == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSObject *	pObjImage = arg.ObjectAt( 0 ) ;
	SGLImageObject *
		pImage = RSImageClass::ImageFromObject( context, pObjImage ) ;
	if ( pImage == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	SGLImageInfo	imginf ;
	uint8_t *	pbytBuf =
		pImage->LockBuffer( imginf, SGLImageObject::lockRead ) ;
	if ( pbytBuf == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	SGLError	err = pVideo->WriteFrame( imginf, pbytBuf ) ;
	pImage->UnlockBuffer( SGLImageObject::lockRead ) ;
	//
	return	context.new_Boolean( err == sglErrSuccess ) ;
}


