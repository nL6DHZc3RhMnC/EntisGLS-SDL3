
#include <sakuragl/sakuragl.h>
#include <sakuragl/sgl_erisa_lib.h>
#include <rosetta/rosetta.h>
#include <rosetta/rosetta_reference.h>
#include <rosetta/rosetta_crypt.h>

using namespace	SSystem ;
using namespace	Rosetta ;


//////////////////////////////////////////////////////////////////////////////
// CRC32Context 型オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSCRC32Context, RSObject )

// 複製（実体も可能な限り複製）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSCRC32Context::CloneObject( RSContext& context ) const
{
	return	new RSCRC32Context( GetRSClass(), m_context ) ;
}



//////////////////////////////////////////////////////////////////////////////
// CRC32Context クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSCRC32ContextClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSCRC32ContextClass::RSCRC32ContextClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSCRC32ContextClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSCRC32Context( this ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"",
			NULL, &RSCRC32ContextClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"stream",
			NULL, L"Uint8Pointer ptrData, int nBytes",
			NULL, &RSCRC32ContextClass::method_stream, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getCRC32", L"int", L"",
			NULL, &RSCRC32ContextClass::method_getCRC32,
			NULL, RSFunctionPrototype::flagConstant ) ;
}

// this オブジェクトのファイルを取得
//////////////////////////////////////////////////////////////////////////////
RSCRC32Context *
	RSCRC32ContextClass::GetThisCRC32( RSContext& context, RSObject* pThis )
{
	RSCRC32Context *	pCRC32 = ESLTypeCast<RSCRC32Context>( pThis ) ;
	if ( pCRC32 == NULL )
	{
		context.ThrowExceptionError( L"this が CRC32Context ではありません" ) ;
	}
	return	pCRC32 ;
}

// void <init>( void )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSCRC32ContextClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSCRC32Context *	pCRC32 = GetThisCRC32( context, pThis ) ;
	if ( pCRC32 == NULL )
	{
		return	NULL ;
	}
	pCRC32->m_context.Initialize() ;
	return	NULL ;
}

// void stream( Uint8Pointer ptrData, int nBytes )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSCRC32ContextClass::method_stream
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSCRC32Context *	pCRC32 = GetThisCRC32( context, pThis ) ;
	if ( pCRC32 == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	size_t		nBytes = (size_t) arg.IntAt( 1 ) ;
	uint8_t *	pbytData = arg.PointerAt( 0, nBytes ) ;
	if ( pbytData == NULL )
	{
		context.ThrowExceptionError
			( L"stream 引数が無効なポインタ、又は範囲です" ) ;
		return	NULL ;
	}
	pCRC32->m_context.Stream( pbytData, nBytes ) ;
	return	NULL ;
}

// int getCRC32()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSCRC32ContextClass::method_getCRC32
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSCRC32Context *	pCRC32 = GetThisCRC32( context, pThis ) ;
	if ( pCRC32 == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( pCRC32->m_context.GetCRC32() ) ;
}



//////////////////////////////////////////////////////////////////////////////
// MD5DigestContext 型オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSMD5DigestContext, RSObject )

// 複製（実体も可能な限り複製）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMD5DigestContext::CloneObject( RSContext& context ) const
{
	return	new RSMD5DigestContext( GetRSClass() ) ;
}



//////////////////////////////////////////////////////////////////////////////
// MD5DigestContext クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSMD5DigestContextClass, RSClass )
// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSMD5DigestContextClass::RSMD5DigestContextClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSMD5DigestContextClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSMD5DigestContext( this ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"",
			NULL, &RSMD5DigestContextClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"stream",
			NULL, L"Uint8Pointer ptrData, int nBytes",
			NULL, &RSMD5DigestContextClass::method_stream, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"flush", NULL, L"",
			NULL, &RSMD5DigestContextClass::method_flush, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getDigest", NULL, L"Uint8Pointer digest",
			NULL, &RSMD5DigestContextClass::method_getDigest,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getDigestHex", L"String", L"",
			NULL, &RSMD5DigestContextClass::method_getDigestHex,
			NULL, RSFunctionPrototype::flagConstant ) ;
}

// this オブジェクトのファイルを取得
//////////////////////////////////////////////////////////////////////////////
RSMD5DigestContext *
	RSMD5DigestContextClass::GetThisMD5( RSContext& context, RSObject* pThis )
{
	RSMD5DigestContext *	pMD5 = ESLTypeCast<RSMD5DigestContext>( pThis ) ;
	if ( pMD5 == NULL )
	{
		context.ThrowExceptionError( L"this が MD5DigestContext ではありません" ) ;
	}
	return	pMD5 ;
}

// void <init>( void )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMD5DigestContextClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSMD5DigestContext *	pMD5 = GetThisMD5( context, pThis ) ;
	if ( pMD5 == NULL )
	{
		return	NULL ;
	}
	pMD5->m_context.Initialize() ;
	return	NULL ;
}

// void stream( Uint8Pointer ptrData, int nBytes )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMD5DigestContextClass::method_stream
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSMD5DigestContext *	pMD5 = GetThisMD5( context, pThis ) ;
	if ( pMD5 == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	size_t		nBytes = (size_t) arg.IntAt( 1 ) ;
	uint8_t *	pbytData = arg.PointerAt( 0, nBytes ) ;
	if ( pbytData == NULL )
	{
		context.ThrowExceptionError
			( L"stream 引数が無効なポインタ、又は範囲です" ) ;
		return	NULL ;
	}
	pMD5->m_context.Stream( pbytData, nBytes ) ;
	return	NULL ;
}

// void flush()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMD5DigestContextClass::method_flush
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSMD5DigestContext *	pMD5 = GetThisMD5( context, pThis ) ;
	if ( pMD5 == NULL )
	{
		return	NULL ;
	}
	pMD5->m_context.Flush() ;
	return	NULL ;
}

// void getDigest( Uint8Pointer digest )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMD5DigestContextClass::method_getDigest
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSMD5DigestContext *	pMD5 = GetThisMD5( context, pThis ) ;
	if ( pMD5 == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	uint8_t *	pbytDigest = arg.PointerAt( 0, 16 ) ;
	if ( pbytDigest == NULL )
	{
		context.ThrowExceptionError
			( L"getDigest 引数が無効なポインタ、又は範囲です" ) ;
		return	NULL ;
	}
	pMD5->m_context.GetMD5Digest( (uint32_t*) pbytDigest ) ;
	return	NULL ;
}

// String getDigestHex()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMD5DigestContextClass::method_getDigestHex
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSMD5DigestContext *	pMD5 = GetThisMD5( context, pThis ) ;
	if ( pMD5 == NULL )
	{
		return	NULL ;
	}
	SString	strDigest ;
	pMD5->m_context.GetMD5DigestHex( strDigest ) ;
	return	context.new_String( strDigest ) ;
}


//////////////////////////////////////////////////////////////////////////////
// Randomizer 型オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSRondomizer, RSObject )

// 複製（実体も可能な限り複製）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRondomizer::CloneObject( RSContext& context ) const
{
	return	new RSRondomizer( GetRSClass() ) ;
}


//////////////////////////////////////////////////////////////////////////////
// Randomizer クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSRandomizerClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSRandomizerClass::RSRandomizerClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSRandomizerClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSRondomizer( this ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"",
			NULL, &RSRandomizerClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"initSeed",
			NULL, L"int seed",
			NULL, &RSRandomizerClass::method_initSeed, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"randomize",
			L"int", L"int nRange",
			NULL, &RSRandomizerClass::method_randomize, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"quickInt",
			L"int", L"int nRange",
			NULL, &RSRandomizerClass::method_quickInt, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"quickFloat",
			L"float", L"float nRange",
			NULL, &RSRandomizerClass::method_quickFloat, NULL ) ;
}

// this オブジェクトのファイルを取得
//////////////////////////////////////////////////////////////////////////////
RSRondomizer *
	RSRandomizerClass::GetThisRandomizer( RSContext& context, RSObject* pThis )
{
	RSRondomizer *	pRand = ESLTypeCast<RSRondomizer>( pThis ) ;
	if ( pRand == NULL )
	{
		context.ThrowExceptionError( L"this が Randomizer ではありません" ) ;
	}
	return	pRand ;
}

// void <init>( void )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRandomizerClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSRondomizer *	pRand = GetThisRandomizer( context, pThis ) ;
	if ( pRand == NULL )
	{
		return	NULL ;
	}
	pRand->m_random.InitializeSeed() ;
	return	NULL ;
}

// void initSeed( int seed )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRandomizerClass::method_initSeed
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSRondomizer *	pRand = GetThisRandomizer( context, pThis ) ;
	if ( pRand == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pRand->m_random.InitializeSeedBy( (uint32_t) arg.IntAt( 0 ) ) ;
	return	NULL ;
}

// int randomize( int nRange )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRandomizerClass::method_randomize
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSRondomizer *	pRand = GetThisRandomizer( context, pThis ) ;
	if ( pRand == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Integer
		( pRand->m_random.Randomize( (uint32_t) arg.IntAt( 0 ) ) ) ;
}

// int quickInt( int nRange )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRandomizerClass::method_quickInt
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSRondomizer *	pRand = GetThisRandomizer( context, pThis ) ;
	if ( pRand == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Integer
		( pRand->m_random.QuickRandomize( (uint32_t) arg.IntAt( 0 ) ) ) ;
}

// float quickFloat( float nRange )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRandomizerClass::method_quickFloat
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSRondomizer *	pRand = GetThisRandomizer( context, pThis ) ;
	if ( pRand == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Number
		( (double) pRand->m_random.QuickRandomize( 0x1000000 )
									* arg.DoubleAt(0) / 0x1000000 ) ;
}


//////////////////////////////////////////////////////////////////////////////
// Encrypt32OutputStream クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSEncrypt32OutputStreamClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSEncrypt32OutputStreamClass::RSEncrypt32OutputStreamClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// メンバ初期設定
//////////////////////////////////////////////////////////////////////////////
void RSEncrypt32OutputStreamClass::Initialize( RSContext& context )
{
	AddSuperClass( context, context.GetClassAs( L"OutputStream" ) ) ;
	OverrideVirtuals( context ) ;
	m_flagInitialized = true ;
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSEncrypt32OutputStreamClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( NULL, this ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"OutputStream stream",
				NULL, &RSEncrypt32OutputStreamClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"initialize", NULL, L"String password",
				NULL, &RSEncrypt32OutputStreamClass::method_initialize, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"generateKey", L"int", L"",
				NULL, &RSEncrypt32OutputStreamClass::method_generateKey, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"flushData", NULL, L"",
				NULL, &RSEncrypt32OutputStreamClass::method_flushData, NULL ) ;
}

// this オブジェクトのファイルを取得
//////////////////////////////////////////////////////////////////////////////
ERISA::SGLEncrypt32OutputStream *
	RSEncrypt32OutputStreamClass::GetThisOutputStream( RSContext& context, RSObject* pThis )
{
	ERISA::SGLEncrypt32OutputStream *	pStream = NULL ;
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj != NULL )
	{
		pStream = ESLTypeCast<ERISA::SGLEncrypt32OutputStream>( pNativeObj->GetObject() ) ;
	}
	if ( pStream == NULL )
	{
		context.ThrowExceptionError( L"this が Encrypt32OutputStream ではありません" ) ;
	}
	return	pStream ;
}

// void <init>( OutputStream stream )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSEncrypt32OutputStreamClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj == NULL )
	{
		context.ThrowExceptionError
			( L"Encrypt32OutputStream.<init> の this が Encrypt32OutputStream ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SOutputStream *	pOutStream =
		ESLTypeCast<SOutputStream>( arg.NativeObjectAt( 0 ) ) ;
	pNativeObj->SetObject
		( (SOutputStream*) new ERISA::SGLEncrypt32OutputStream( pOutStream ) ) ;
	return	NULL ;
}

// void initialize( String password )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSEncrypt32OutputStreamClass::method_initialize
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	ERISA::SGLEncrypt32OutputStream *
			pEncrypt = GetThisOutputStream( context, pThis ) ;
	if ( pEncrypt == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pEncrypt->Initialize( arg.StringAt( 0 ) ) ;
	return	NULL ;
}

// int generateKey()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSEncrypt32OutputStreamClass::method_generateKey
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	ERISA::SGLEncrypt32OutputStream *
			pEncrypt = GetThisOutputStream( context, pThis ) ;
	if ( pEncrypt == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( pEncrypt->GenerateKey() ) ;
}

// void flushData()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSEncrypt32OutputStreamClass::method_flushData
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	ERISA::SGLEncrypt32OutputStream *
			pEncrypt = GetThisOutputStream( context, pThis ) ;
	if ( pEncrypt == NULL )
	{
		return	NULL ;
	}
	pEncrypt->FlushData() ;
	return	NULL ;
}


//////////////////////////////////////////////////////////////////////////////
// Decrypt32InputStream クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSDecrypt32InputStreamClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSDecrypt32InputStreamClass::RSDecrypt32InputStreamClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// メンバ初期設定
//////////////////////////////////////////////////////////////////////////////
void RSDecrypt32InputStreamClass::Initialize( RSContext& context )
{
	AddSuperClass( context, context.GetClassAs( L"InputStream" ) ) ;
	OverrideVirtuals( context ) ;
	m_flagInitialized = true ;
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSDecrypt32InputStreamClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( NULL, this ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"InputStream stream",
				NULL, &RSDecrypt32InputStreamClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"initialize", NULL, L"String password",
				NULL, &RSDecrypt32InputStreamClass::method_initialize, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setDecryptKey", NULL, L"int key",
				NULL, &RSDecrypt32InputStreamClass::method_setDecryptKey, NULL ) ;
}

// this オブジェクトのファイルを取得
//////////////////////////////////////////////////////////////////////////////
ERISA::SGLDecrypt32InputStream *
	RSDecrypt32InputStreamClass::GetThisInputStream( RSContext& context, RSObject* pThis )
{
	ERISA::SGLDecrypt32InputStream *	pStream = NULL ;
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj != NULL )
	{
		pStream = ESLTypeCast<ERISA::SGLDecrypt32InputStream>( pNativeObj->GetObject() ) ;
	}
	if ( pStream == NULL )
	{
		context.ThrowExceptionError( L"this が Decrypt32InputStream ではありません" ) ;
	}
	return	pStream ;
}

// void <init>( InputStream stream )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDecrypt32InputStreamClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj == NULL )
	{
		context.ThrowExceptionError
			( L"Decrypt32InputStream.<init> の this が Decrypt32InputStream ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SInputStream *	pInStream =
		ESLTypeCast<SInputStream>( arg.NativeObjectAt( 0 ) ) ;
	pNativeObj->SetObject
		( (SInputStream*) new ERISA::SGLDecrypt32InputStream( pInStream ) ) ;
	return	NULL ;
}

// void initialize( String password )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDecrypt32InputStreamClass::method_initialize
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	ERISA::SGLDecrypt32InputStream *
			pDecrypt = GetThisInputStream( context, pThis ) ;
	if ( pDecrypt == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pDecrypt->Initialize( arg.StringAt( 0 ) ) ;
	return	NULL ;
}

// void setDecryptKey( int key )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDecrypt32InputStreamClass::method_setDecryptKey
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	ERISA::SGLDecrypt32InputStream *
			pDecrypt = GetThisInputStream( context, pThis ) ;
	if ( pDecrypt == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pDecrypt->SetDecryptKey( (uint32_t) arg.IntAt( 0 ) ) ;
	return	NULL ;
}


//////////////////////////////////////////////////////////////////////////////
// EncryptRSA96OutputStream クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSEncryptRSA96OutputStreamClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSEncryptRSA96OutputStreamClass::RSEncryptRSA96OutputStreamClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName, NULL, RSObject::modifierPrivate )
{
}

// メンバ初期設定
//////////////////////////////////////////////////////////////////////////////
void RSEncryptRSA96OutputStreamClass::Initialize( RSContext& context )
{
	AddSuperClass( context, context.GetClassAs( L"OutputStream" ) ) ;
	OverrideVirtuals( context ) ;
	m_flagInitialized = true ;
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSEncryptRSA96OutputStreamClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( NULL, this ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"OutputStream stream",
				NULL, &RSEncryptRSA96OutputStreamClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"initialize", NULL, L"String password",
				NULL, &RSEncryptRSA96OutputStreamClass::method_initialize, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setPublicKey", NULL, L"Uint8Pointer key",
				NULL, &RSEncryptRSA96OutputStreamClass::method_setPublicKey, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"flushData", NULL, L"",
				NULL, &RSEncryptRSA96OutputStreamClass::method_flushData, NULL ) ;
}

// this オブジェクトのファイルを取得
//////////////////////////////////////////////////////////////////////////////
ERISA::SGLEncryptRSA96OutputStream *
	RSEncryptRSA96OutputStreamClass::GetThisOutputStream( RSContext& context, RSObject* pThis )
{
	ERISA::SGLEncryptRSA96OutputStream *	pStream = NULL ;
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj != NULL )
	{
		pStream = ESLTypeCast<ERISA::SGLEncryptRSA96OutputStream>( pNativeObj->GetObject() ) ;
	}
	if ( pStream == NULL )
	{
		context.ThrowExceptionError( L"this が EncryptRSA96OutputStream ではありません" ) ;
	}
	return	pStream ;
}

// void <init>( OutputStream stream )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSEncryptRSA96OutputStreamClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj == NULL )
	{
		context.ThrowExceptionError
			( L"EncryptRSA96OutputStream.<init> の this が EncryptRSA96OutputStream ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SOutputStream *	pOutStream =
		ESLTypeCast<SOutputStream>( arg.NativeObjectAt( 0 ) ) ;
	pNativeObj->SetObject
		( (SOutputStream*) new ERISA::SGLEncryptRSA96OutputStream( pOutStream ) ) ;
	return	NULL ;
}

// void initialize( String password )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSEncryptRSA96OutputStreamClass::method_initialize
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	ERISA::SGLEncryptRSA96OutputStream *
			pEncrypt = GetThisOutputStream( context, pThis ) ;
	if ( pEncrypt == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pEncrypt->Initialize( arg.StringAt( 0 ) ) ;
	return	NULL ;
}

// void setPublicKey( Uint8Pointer key )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSEncryptRSA96OutputStreamClass::method_setPublicKey
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	ERISA::SGLEncryptRSA96OutputStream *
			pEncrypt = GetThisOutputStream( context, pThis ) ;
	if ( pEncrypt == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	uint8_t *	pbytKey = arg.PointerAt( 0, 12 ) ;
	if ( pbytKey == NULL )
	{
		context.ThrowExceptionError
			( L"EncryptRSA96OutputStream.setPublicKey の引数が不正です" ) ;
		return	NULL ;
	}
	ERISA::BigNumber<3>	keyPublic( (uint32_t*) pbytKey, 3 ) ;
	pEncrypt->SetPublicKey( keyPublic ) ;
	return	NULL ;
}

// void flushData()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSEncryptRSA96OutputStreamClass::method_flushData
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	ERISA::SGLEncryptRSA96OutputStream *
			pEncrypt = GetThisOutputStream( context, pThis ) ;
	if ( pEncrypt == NULL )
	{
		return	NULL ;
	}
	pEncrypt->FlushData() ;
	return	NULL ;
}


//////////////////////////////////////////////////////////////////////////////
// DecryptRSA96InputStream クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSDecryptRSA96InputStreamClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSDecryptRSA96InputStreamClass::RSDecryptRSA96InputStreamClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName, NULL, RSObject::modifierPrivate )
{
}

// メンバ初期設定
//////////////////////////////////////////////////////////////////////////////
void RSDecryptRSA96InputStreamClass::Initialize( RSContext& context )
{
	AddSuperClass( context, context.GetClassAs( L"InputStream" ) ) ;
	OverrideVirtuals( context ) ;
	m_flagInitialized = true ;
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSDecryptRSA96InputStreamClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( NULL, this ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"InputStream stream",
				NULL, &RSDecryptRSA96InputStreamClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"initialize", NULL, L"String password",
				NULL, &RSDecryptRSA96InputStreamClass::method_initialize, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"generateKey", L"Uint8Pointer", L"",
				NULL, &RSDecryptRSA96InputStreamClass::method_generateKey, NULL ) ;
}

// this オブジェクトのファイルを取得
//////////////////////////////////////////////////////////////////////////////
ERISA::SGLDecryptRSA96InputStream *
	RSDecryptRSA96InputStreamClass::GetThisInputStream( RSContext& context, RSObject* pThis )
{
	ERISA::SGLDecryptRSA96InputStream *	pStream = NULL ;
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj != NULL )
	{
		pStream = ESLTypeCast<ERISA::SGLDecryptRSA96InputStream>( pNativeObj->GetObject() ) ;
	}
	if ( pStream == NULL )
	{
		context.ThrowExceptionError( L"this が DecryptRSA96InputStream ではありません" ) ;
	}
	return	pStream ;
}

// void <init>( InputStream stream )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDecryptRSA96InputStreamClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj == NULL )
	{
		context.ThrowExceptionError
			( L"Decrypt32InputStream.<init> の this が Decrypt32InputStream ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SInputStream *	pInStream =
		ESLTypeCast<SInputStream>( arg.NativeObjectAt( 0 ) ) ;
	pNativeObj->SetObject
		( (SInputStream*) new ERISA::SGLDecryptRSA96InputStream( pInStream ) ) ;
	return	NULL ;
}

// void initialize( String password )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDecryptRSA96InputStreamClass::method_initialize
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	ERISA::SGLDecryptRSA96InputStream *
			pDecrypt = GetThisInputStream( context, pThis ) ;
	if ( pDecrypt == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pDecrypt->Initialize( arg.StringAt( 0 ) ) ;
	return	NULL ;
}

// Uint8Pointer generateKey()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSDecryptRSA96InputStreamClass::method_generateKey
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	ERISA::SGLDecryptRSA96InputStream *
			pDecrypt = GetThisInputStream( context, pThis ) ;
	if ( pDecrypt == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	ERISA::BigNumber<3>	bnPublicKey = pDecrypt->GenerateKey() ;
	//
	RSArrayBuffer *	pBuffer =
		new RSArrayBuffer( context.GetArrayBufferClass() ) ;
	pBuffer->AllocateBuffer( 12 ) ;
	eslMoveMemory
		( pBuffer->m_ptrBuf, &(bnPublicKey.m_num[0]), 12 ) ;
	//
	return	context.new_PointerNumber
				( pBuffer, RSReferenceNumber::typeUint8 ) ;
}


