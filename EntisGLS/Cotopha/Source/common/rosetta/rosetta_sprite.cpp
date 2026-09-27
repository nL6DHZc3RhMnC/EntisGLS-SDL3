
#include <rosetta/rosetta.h>
#include <sakuragl/sgl2d_image.h>
#include <sakuragl/sgl2d/sgl_image_filter.h>
#include <sakuragl/sgl2d/sgl_image_conversion.h>
#include <sakuragl/sgl2d/sgl_image_buf_object.h>
#include <sakuragl/sgl_erisa_lib.h>
#include <sakuraglx/sakuraglx.h>
#include <sakuraglx/sprite/sglx_sprite_formed.h>
#include <rosetta/rosetta_reference.h>
#include <rosetta/rosetta_array.h>
#include <rosetta/rosetta_thread.h>
#include <rosetta/rosetta_file.h>
#include <rosetta/rosetta_image.h>
#include <rosetta/rosetta_sprite.h>

using namespace	SSystem ;
using namespace	SakuraGL ;
using namespace	Rosetta ;


//////////////////////////////////////////////////////////////////////////////
// SkinManager クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSSkinManagerClass, RGenericNativeObjectClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSSkinManagerClass::RSSkinManagerClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RGenericNativeObjectClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSSkinManagerClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( NULL, this ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"",
				NULL, &RSSkinManagerClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"loadSkinFile", L"boolean",
			L"String file, boolean fStaticRsrc = false",
				NULL, &RSSkinManagerClass::method_loadSkinFile, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"readSkinFile", L"boolean",
			L"InputStream is, boolean fStaticRsrc = false",
				NULL, &RSSkinManagerClass::method_readSkinFile, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"readSkinFile", L"boolean",
			L"RandomAccessFile raf, boolean fStaticRsrc = false",
				NULL, &RSSkinManagerClass::method_readSkinFile, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"cleanupResource", NULL, L"",
				NULL, &RSSkinManagerClass::method_cleanupResource, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"removeSkinResource", NULL, L"",
				NULL, &RSSkinManagerClass::method_removeSkinResource, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getImageAs", L"Image", L"String id",
				NULL, &RSSkinManagerClass::method_getImageAs,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getTextStyleAs",
				L"TextSprite.TextStyle", L"String id",
				NULL, &RSSkinManagerClass::method_getTextStyleAs,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"createFormedPage", L"Sprite", L"String id",
				NULL, &RSSkinManagerClass::method_createFormedPage, NULL ) ;
}

// ネイティブ型テスト
//////////////////////////////////////////////////////////////////////////////
bool RSSkinManagerClass::IsNativeObjectOf( ESLObject * pObj ) const
{
	return	(ESLTypeCast<SGLSkinManager>(pObj) != nullptr) ;
}

// this オブジェクトを取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLSkinManager *
	RSSkinManagerClass::GetThisSkinManager( RSContext& context, RSObject* pThis )
{
	SGLSkinManager *	pSkin = NULL ;
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj != NULL )
	{
		pSkin = ESLTypeCast<SGLSkinManager>( pNativeObj->GetObject() ) ;
	}
	if ( pSkin == NULL )
	{
		context.ThrowExceptionError( L"this が SkinManager ではありません" ) ;
	}
	return	pSkin ;
}

// void <init>()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSkinManagerClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj == NULL )
	{
		context.ThrowExceptionError
			( L"SkinManager.<init> の this が SkinManager ではありません" ) ;
		return	NULL ;
	}
	pNativeObj->SetObject
		( SGLSkinManager::GetESLPointer( new SGLSkinManager ) ) ;
	return	NULL ;
}

// boolean loadSkinFile( String file, boolean fStaticRsrc = false ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSkinManagerClass::method_loadSkinFile
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSkinManager *	pSkin = GetThisSkinManager( context, pThis ) ;
	if ( pSkin == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLError	err =
		pSkin->LoadSkinFile( arg.StringAt(0), arg.BooleanAt(1,false) ) ;
	return	context.new_Boolean( err == sglErrSuccess ) ;
}

// boolean readSkinFile( InputStream is, boolean fStaticRsrc = false ) ;
// boolean readSkinFile( RandomAccessFile raf, boolean fStaticRsrc = false ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSkinManagerClass::method_readSkinFile
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSkinManager *	pSkin = GetThisSkinManager( context, pThis ) ;
	if ( pSkin == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SFileInterface *	pFile =
		ESLTypeCast<SFileInterface>( arg.NativeObjectAt(1) ) ;
	if ( pFile == NULL )
	{
		context.ThrowExceptionError
			( L"SkinManager.readSkinFile の引数が InputStream ではありません" ) ;
		return	NULL ;
	}
	SGLError	err =
		pSkin->ReadSkinFile( *pFile, arg.BooleanAt(1,false) ) ;
	return	context.new_Boolean( err == sglErrSuccess ) ;
}

// void cleanupResource()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSkinManagerClass::method_cleanupResource
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSkinManager *	pSkin = GetThisSkinManager( context, pThis ) ;
	if ( pSkin == NULL )
	{
		return	NULL ;
	}
	pSkin->CleanupResource() ;
	return	NULL ;
}

// void removeSkinResource()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSkinManagerClass::method_removeSkinResource
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSkinManager *	pSkin = GetThisSkinManager( context, pThis ) ;
	if ( pSkin == NULL )
	{
		return	NULL ;
	}
	pSkin->RemoveSkinResource() ;
	return	NULL ;
}

// Image getImageAs( String id )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSkinManagerClass::method_getImageAs
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSkinManager *	pSkin = GetThisSkinManager( context, pThis ) ;
	if ( pSkin == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLImageObject *	pImage = pSkin->GetImageAs( arg.StringAt(0) ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	return	new RSNativeObject( pImage, context.GetClassAs(L"Image") ) ;
}

// TextSprite.TextStyle getTextStyleAs( String id )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSkinManagerClass::method_getTextStyleAs
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSkinManager *	pSkin = GetThisSkinManager( context, pThis ) ;
	if ( pSkin == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SXMLDocument *	pxmlStyle = pSkin->GetStyleAs( arg.StringAt(0) ) ;
	if ( pxmlStyle == NULL )
	{
		return	NULL ;
	}
	SGLSpriteText::TextStyle	style ;
	SString	strFontFace ;
	SGLSpriteText::ParseTextStyle( style, strFontFace, *pxmlStyle ) ;
	//
	RSObject *	pObjStyle = context.new_Object( L"TextSprite.TextStyle" ) ;
	//
	RSTextSpriteClass::TextStyleClass::
			ConvertToObject( context, pObjStyle, style ) ;
	//
	return	pObjStyle ;
}

// Sprite createFormedPage( String id )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSkinManagerClass::method_createFormedPage
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSkinManager *	pSkin = GetThisSkinManager( context, pThis ) ;
	if ( pSkin == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLSprite *	pSprite = pSkin->CreateFormedSprite( arg.StringAt(0) ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	return	new RSNativeObject
		( new SSmartObject( pSprite ),
			context.GetClassAs( pSprite->GetRSClassName() ) ) ;
}



//////////////////////////////////////////////////////////////////////////////
// BasicFormParser クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( RSBasicFormParserClass, RGenericNativeObjectClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSBasicFormParserClass::RSBasicFormParserClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RGenericNativeObjectClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSBasicFormParserClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( nullptr, this ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", nullptr, L"",
				nullptr, &RSBasicFormParserClass::method_init, nullptr ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"openPackage", L"boolean", L"String file",
			nullptr, &RSBasicFormParserClass::method_openPackage, nullptr ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"closePackage", nullptr, L"",
			nullptr, &RSBasicFormParserClass::method_closePackage, nullptr ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"loadForm", L"boolean", L"String file",
			nullptr, &RSBasicFormParserClass::method_loadForm, nullptr ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getImageAs", L"Image", L"String id",
			nullptr, &RSBasicFormParserClass::method_getImageAs,
			nullptr, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"buildImageAtlas", L"int",
			L"int nMaxAtlasSize = 2048, int nReqBatchCount = 3",
			nullptr, &RSBasicFormParserClass::method_buildImageAtlas, nullptr ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"createFormedSprite", L"Sprite",
			L"String id, boolean fBuffered = false, long nBufFlags = 0",
			nullptr, &RSBasicFormParserClass::method_createFormedSprite,
			nullptr, RSFunctionPrototype::flagConstant ) ;
}

// ネイティブ型テスト
//////////////////////////////////////////////////////////////////////////////
bool RSBasicFormParserClass::IsNativeObjectOf( ESLObject * pObj ) const
{
	return	(ESLTypeCast<SGLBasicFormParser>(pObj) != nullptr) ;
}

// this オブジェクトを取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLBasicFormParser *
	RSBasicFormParserClass::GetThisFormParser( RSContext& context, RSObject* pThis )
{
	return	RSNativeObject::GetNative<SGLBasicFormParser>( pThis ) ;
}

// void <init>()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSBasicFormParserClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj == nullptr )
	{
		context.ThrowExceptionError
			( L"BasicFormParser.<init> の this が BasicFormParser ではありません" ) ;
		return	nullptr ;
	}
	pNativeObj->SetObject( new SGLBasicFormParser ) ;
	return	nullptr ;
}

// boolean openPackage( String file ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSBasicFormParserClass::method_openPackage
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLBasicFormParser *	pForm = GetThisFormParser( context, pThis ) ;
	if ( pForm == nullptr )
	{
		context.ThrowExceptionError
			( L"this ポインタが null です", L"NullPointerException" ) ;
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLError	err = pForm->OpenPackage( arg.StringAt(0) ) ;
	return	context.new_Boolean( err == sglErrSuccess ) ;
}

// void closePackage( void ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSBasicFormParserClass::method_closePackage
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLBasicFormParser *	pForm = GetThisFormParser( context, pThis ) ;
	if ( pForm == nullptr )
	{
		context.ThrowExceptionError
			( L"this ポインタが null です", L"NullPointerException" ) ;
		return	nullptr ;
	}
	pForm->ClosePackage() ;
	return	nullptr ;
}

// boolean loadForm( String file )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSBasicFormParserClass::method_loadForm
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLBasicFormParser *	pForm = GetThisFormParser( context, pThis ) ;
	if ( pForm == nullptr )
	{
		context.ThrowExceptionError
			( L"this ポインタが null です", L"NullPointerException" ) ;
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLError	err = pForm->LoadForm( arg.StringAt(0) ) ;
	return	context.new_Boolean( err == sglErrSuccess ) ;
}

// Image getImageAs( String id )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSBasicFormParserClass::method_getImageAs
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLBasicFormParser *	pForm = GetThisFormParser( context, pThis ) ;
	if ( pForm == nullptr )
	{
		context.ThrowExceptionError
			( L"this ポインタが null です", L"NullPointerException" ) ;
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLImageObject *	pImage = pForm->GetImageAs( arg.StringAt(0) ) ;
	if ( pImage == nullptr )
	{
		return	nullptr ;
	}
	return	new RSNativeObject( pImage, context.GetClassAs(L"Image") ) ;
}

// int buildImageAtlas( int nMaxAtlasSize = 2048, int nReqBatchCount = 3 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSBasicFormParserClass::method_buildImageAtlas
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLBasicFormParser *	pForm = GetThisFormParser( context, pThis ) ;
	if ( pForm == nullptr )
	{
		context.ThrowExceptionError
			( L"this ポインタが null です", L"NullPointerException" ) ;
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Integer
		( pForm->BuildImageAtlas
			( (size_t) arg.IntAt( 0, 2048 ), (size_t) arg.IntAt( 1, 3 ) ) ) ;
}

// Sprite createFormedSprite
//	( String id, boolean flagBuffered = false, long nBufFlags = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSBasicFormParserClass::method_createFormedSprite
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLBasicFormParser *	pForm = GetThisFormParser( context, pThis ) ;
	if ( pForm == nullptr )
	{
		context.ThrowExceptionError
			( L"this ポインタが null です", L"NullPointerException" ) ;
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLBasicForm *	pBasicForm = new SGLBasicForm ;
	SGLError	err = pForm->BuildFormAs( *pBasicForm, arg.StringAt(0) ) ;
	if ( err )
	{
		delete	pBasicForm ;
		return	nullptr ;
	}
	SGLSpriteFormed *	pSprite = new SGLSpriteFormed ;
	pSprite->AttachFormParser( pForm ) ;
	pSprite->SetBasicForm( pBasicForm ) ;
	//
	SGLImageRect	rectForm = pBasicForm->GetFormRect() ;
	pSprite->SetPosition( rectForm.x, rectForm.y ) ;
	//
	if ( arg.BooleanAt(1) )
	{
		pSprite->CreateBuffer
			( rectForm.w, rectForm.h, formatImageARGB, 32, arg.LongAt(2) ) ;
	}
	return	new RSNativeObject
		( new SSmartObject( pSprite ),
			context.GetClassAs( pSprite->GetRSClassName() ) ) ;
}



//////////////////////////////////////////////////////////////////////////////
// SpriteTimer オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
	( Rosetta::RSSpriteTimer, RSGenericObject, SGLSpriteTimer )

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
RSSpriteTimer::~RSSpriteTimer( void )
{
}

// 複製（参照の複製を含む）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteTimer::DuplicateObject( RSContext& context ) const
{
	ESLAssert( m_pClass != NULL ) ;
	RSSpriteTimer *	pObj = new RSSpriteTimer( context.GetVM(), m_pClass ) ;
	pObj->m_gcmMembers.DuplicateAllMembers
				( context, m_gcmMembers.m_members ) ;
	return	pObj ;
}

// 複製（実体も可能な限り複製）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteTimer::CloneObject( RSContext& context ) const
{
	ESLAssert( m_pClass != NULL ) ;
	RSSpriteTimer *	pObj = new RSSpriteTimer( context.GetVM(), m_pClass ) ;
	pObj->m_gcmMembers.CloneAllMembers
				( context, m_gcmMembers.m_members ) ;
	return	pObj ;
}

// タイマー処理（true で終了）
//////////////////////////////////////////////////////////////////////////////
bool RSSpriteTimer::OnTimer( SGLSprite& sprite, uint32_t msecPast )
{
	ESLAssert( m_vm != NULL ) ;
	RSContext	context( m_vm ) ;
	RSFunctionObject *	pFunc =
			m_pClass->GetVirtualMemberAs( context, L"onTimer" ) ;
	bool	fResult = false ;
	if ( pFunc != NULL )
	{
		RSObject *	pArgs[2] ;
		pArgs[0] = new RSNativeObject
					( &sprite, context.GetClassAs( sprite.GetRSClassName() ) ) ;
		pArgs[1] = context.new_Integer( msecPast ) ;
		//
		RSSmartPtr	pResult
			( context.CallFunction( *pFunc, this, &pArgs[0], 2, false ) ) ;
		if ( pResult != NULL )
		{
			fResult = pResult->AsBoolean() ;
		}
		//
		RSObject::ReleaseRef( pArgs[0] ) ;
		RSObject::ReleaseRef( pArgs[1] ) ;
		RSObject::ReleaseRef( pFunc ) ;
	}
	return	fResult ;
}


//////////////////////////////////////////////////////////////////////////////
// SpriteTimer クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSSpriteTimerClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSSpriteTimerClass::RSSpriteTimerClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSSpriteTimerClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSSpriteTimer( context.GetVM(), this ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"onTimer", L"boolean",
			L"Sprite sprite, int msecPast", NULL, NULL, NULL ) ;
}


//////////////////////////////////////////////////////////////////////////////
// マウス入力インターフェースオブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
	( Rosetta::RSSpriteMouseListener, RSGenericObject, SGLSpriteMouseStateListener )

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
RSSpriteMouseListener::~RSSpriteMouseListener( void )
{
}

// 複製（参照の複製を含む）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteMouseListener::DuplicateObject( RSContext& context ) const
{
	ESLAssert( m_pClass != NULL ) ;
	RSSpriteMouseListener *	pObj = new RSSpriteMouseListener( context.GetVM(), m_pClass ) ;
	pObj->m_gcmMembers.DuplicateAllMembers
				( context, m_gcmMembers.m_members ) ;
	return	pObj ;
}

// 複製（実体も可能な限り複製）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteMouseListener::CloneObject( RSContext& context ) const
{
	ESLAssert( m_pClass != NULL ) ;
	RSSpriteMouseListener *	pObj = new RSSpriteMouseListener( context.GetVM(), m_pClass ) ;
	pObj->m_gcmMembers.CloneAllMembers
				( context, m_gcmMembers.m_members ) ;
	return	pObj ;
}

// スクリプト呼び出し
//////////////////////////////////////////////////////////////////////////////
bool RSSpriteMouseListener::CallMouseListener
	( const wchar_t * pwszFunc,
		SGLSprite& sprite, double xPos, double yPos, int64_t nFlags )
{
	ESLAssert( m_vm != NULL ) ;
	RSContext	context( m_vm ) ;
	RSFunctionObject *	pFunc =
			m_pClass->GetVirtualMemberAs( context, pwszFunc ) ;
	bool	fResult = false ;
	if ( pFunc != NULL )
	{
		RSObject *	pArgs[4] ;
		pArgs[0] = new RSNativeObject
					( &sprite, context.GetClassAs( sprite.GetRSClassName() ) ) ;
		pArgs[1] = context.new_Number( xPos ) ;
		pArgs[2] = context.new_Number( yPos ) ;
		pArgs[3] = context.new_Integer( nFlags ) ;
		//
		RSSmartPtr	pResult
			( context.CallFunction( *pFunc, this, &pArgs[0], 4, false ) ) ;
		if ( pResult != NULL )
		{
			fResult = pResult->AsBoolean() ;
		}
		//
		RSObject::ReleaseRef( pArgs[0] ) ;
		RSObject::ReleaseRef( pArgs[1] ) ;
		RSObject::ReleaseRef( pArgs[2] ) ;
		RSObject::ReleaseRef( pArgs[3] ) ;
		RSObject::ReleaseRef( pFunc ) ;
	}
	return	fResult ;
}

// マウス移動
//////////////////////////////////////////////////////////////////////////////
bool RSSpriteMouseListener::OnMouseMove
	( SGLSprite& sprite, double xPos, double yPos, int64_t nFlags )
{
	SGLSpriteMouseStateListener::OnMouseMove( sprite, xPos, yPos, nFlags ) ;
	//
	return	CallMouseListener( L"onMouseMove", sprite, xPos, yPos, nFlags ) ;
}

void RSSpriteMouseListener::OnMouseLeave( SGLSprite& sprite, int64_t nFlags )
{
	SGLSpriteMouseStateListener::OnMouseLeave( sprite, nFlags ) ;
	//
	ESLAssert( m_vm != NULL ) ;
	RSContext	context( m_vm ) ;
	RSFunctionObject *	pFunc =
			m_pClass->GetVirtualMemberAs( context, L"onMouseLeave" ) ;
	if ( pFunc != NULL )
	{
		RSObject *	pArgs[2] ;
		pArgs[0] = new RSNativeObject
					( &sprite, context.GetClassAs( sprite.GetRSClassName() ) ) ;
		pArgs[1] = context.new_Integer( nFlags ) ;
		//
		RSSmartPtr	pResult
			( context.CallFunction( *pFunc, this, &pArgs[0], 2, false ) ) ;
		//
		RSObject::ReleaseRef( pArgs[0] ) ;
		RSObject::ReleaseRef( pArgs[1] ) ;
		RSObject::ReleaseRef( pFunc ) ;
	}
}

// ホイール回転
//////////////////////////////////////////////////////////////////////////////
bool RSSpriteMouseListener::OnMouseWheel
	( SGLSprite& sprite, int32_t zDelta,
		double xPos, double yPos, int64_t nFlags )
{
	ESLAssert( m_vm != NULL ) ;
	RSContext	context( m_vm ) ;
	RSFunctionObject *	pFunc =
			m_pClass->GetVirtualMemberAs( context, L"onMouseWheel" ) ;
	bool	fResult = false ;
	if ( pFunc != NULL )
	{
		RSObject *	pArgs[5] ;
		pArgs[0] = new RSNativeObject
					( &sprite, context.GetClassAs( sprite.GetRSClassName() ) ) ;
		pArgs[1] = context.new_Integer( zDelta ) ;
		pArgs[2] = context.new_Number( xPos ) ;
		pArgs[3] = context.new_Number( yPos ) ;
		pArgs[4] = context.new_Integer( nFlags ) ;
		//
		RSSmartPtr	pResult
			( context.CallFunction( *pFunc, this, &pArgs[0], 5, false ) ) ;
		if ( pResult != NULL )
		{
			fResult = pResult->AsBoolean() ;
		}
		//
		RSObject::ReleaseRef( pArgs[0] ) ;
		RSObject::ReleaseRef( pArgs[1] ) ;
		RSObject::ReleaseRef( pArgs[2] ) ;
		RSObject::ReleaseRef( pArgs[3] ) ;
		RSObject::ReleaseRef( pArgs[4] ) ;
		RSObject::ReleaseRef( pFunc ) ;
	}
	return	fResult ;
}

// 左ボタン
//////////////////////////////////////////////////////////////////////////////
bool RSSpriteMouseListener::OnLButtonDown
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	return	CallMouseListener( L"onLButtonDown", sprite, xPos, yPos, nFlags ) ;
}

bool RSSpriteMouseListener::OnLButtonUp
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	return	CallMouseListener( L"onLButtonUp", sprite, xPos, yPos, nFlags ) ;
}

bool RSSpriteMouseListener::OnLButtonDblClk
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	return	CallMouseListener( L"onLButtonDblClk", sprite, xPos, yPos, nFlags ) ;
}

// 右ボタン
//////////////////////////////////////////////////////////////////////////////
bool RSSpriteMouseListener::OnRButtonDown
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	return	CallMouseListener( L"onRButtonDown", sprite, xPos, yPos, nFlags ) ;
}

bool RSSpriteMouseListener::OnRButtonUp
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	return	CallMouseListener( L"onRButtonUp", sprite, xPos, yPos, nFlags ) ;
}

bool RSSpriteMouseListener::OnRButtonDblClk
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	return	CallMouseListener( L"onRButtonDblClk", sprite, xPos, yPos, nFlags ) ;
}

// 中央ボタン
//////////////////////////////////////////////////////////////////////////////
bool RSSpriteMouseListener::OnMButtonDown
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	return	CallMouseListener( L"onMButtonDown", sprite, xPos, yPos, nFlags ) ;
}

bool RSSpriteMouseListener::OnMButtonUp
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	return	CallMouseListener( L"onMButtonUp", sprite, xPos, yPos, nFlags ) ;
}

bool RSSpriteMouseListener::OnMButtonDblClk
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	return	CallMouseListener( L"onMButtonDblClk", sprite, xPos, yPos, nFlags ) ;
}

// 左ボタン（後処理）
//////////////////////////////////////////////////////////////////////////////
bool RSSpriteMouseListener::AfterLButtonDown
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	return	CallMouseListener( L"afterLButtonDown", sprite, xPos, yPos, nFlags ) ;
}

bool RSSpriteMouseListener::AfterLButtonUp
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	return	CallMouseListener( L"afterLButtonUp", sprite, xPos, yPos, nFlags ) ;
}

bool RSSpriteMouseListener::AfterLButtonDblClk
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	return	CallMouseListener( L"afterLButtonDblClk", sprite, xPos, yPos, nFlags ) ;
}

// 右ボタン（後処理）
//////////////////////////////////////////////////////////////////////////////
bool RSSpriteMouseListener::AfterRButtonDown
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	return	CallMouseListener( L"afterRButtonDown", sprite, xPos, yPos, nFlags ) ;
}

bool RSSpriteMouseListener::AfterRButtonUp
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	return	CallMouseListener( L"afterRButtonUp", sprite, xPos, yPos, nFlags ) ;
}

bool RSSpriteMouseListener::AfterRButtonDblClk
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	return	CallMouseListener( L"afterRButtonDblClk", sprite, xPos, yPos, nFlags ) ;
}

// 中央ボタン（後処理）
//////////////////////////////////////////////////////////////////////////////
bool RSSpriteMouseListener::AfterMButtonDown
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	return	CallMouseListener( L"afterMButtonDown", sprite, xPos, yPos, nFlags ) ;
}

bool RSSpriteMouseListener::AfterMButtonUp
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	return	CallMouseListener( L"afterMButtonUp", sprite, xPos, yPos, nFlags ) ;
}

bool RSSpriteMouseListener::AfterMButtonDblClk
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	return	CallMouseListener( L"afterMButtonDblClk", sprite, xPos, yPos, nFlags ) ;
}


//////////////////////////////////////////////////////////////////////////////
// マウス入力インターフェースクラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSSpriteMouseListenerClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSSpriteMouseListenerClass::RSSpriteMouseListenerClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSSpriteMouseListenerClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSSpriteMouseListener( context.GetVM(), this ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"getPointerCount", L"int", L"",
			NULL, &RSSpriteMouseListenerClass::method_getPointerCount,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"findMouseIndexById", L"int", L"int idMouse",
			NULL, &RSSpriteMouseListenerClass::method_findMouseIndexById,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getMousePointAt", L"Vector2D", L"int i",
			NULL, &RSSpriteMouseListenerClass::method_getMousePointAt,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"isLButtonDownAt", L"boolean", L"int i",
			NULL, &RSSpriteMouseListenerClass::method_isLButtonDownAt,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"isRButtonDownAt", L"boolean", L"int i",
			NULL, &RSSpriteMouseListenerClass::method_isRButtonDownAt,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"enumerateLDownPoints", L"Vector2D", L"",
			NULL, &RSSpriteMouseListenerClass::method_enumerateLDownPoints,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"onMouseMove", L"boolean",
			L"Sprite sprite, double xPos, double yPos, long nFlags",
			NULL, &RSSpriteMouseListenerClass::method_onMouseEvent, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"onMouseLeave", NULL,
			L"Sprite sprite, long nFlags",
			NULL, &RSSpriteMouseListenerClass::method_onMouseLeave, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"onMouseWheel", L"boolean",
			L"Sprite sprite, int zDelta, "
			L"double xPos, double yPos, long nFlags",
			NULL, &RSSpriteMouseListenerClass::method_onMouseWheel, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"onLButtonDown", L"boolean",
			L"Sprite sprite, double xPos, double yPos, long nFlags",
			NULL, &RSSpriteMouseListenerClass::method_onMouseEvent, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"onLButtonUp", L"boolean",
			L"Sprite sprite, double xPos, double yPos, long nFlags",
			NULL, &RSSpriteMouseListenerClass::method_onMouseEvent, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"onLButtonDblClk", L"boolean",
			L"Sprite sprite, double xPos, double yPos, long nFlags",
			NULL, &RSSpriteMouseListenerClass::method_onMouseEvent, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"onRButtonDown", L"boolean",
			L"Sprite sprite, double xPos, double yPos, long nFlags",
			NULL, &RSSpriteMouseListenerClass::method_onMouseEvent, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"onRButtonUp", L"boolean",
			L"Sprite sprite, double xPos, double yPos, long nFlags",
			NULL, &RSSpriteMouseListenerClass::method_onMouseEvent, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"onRButtonDblClk", L"boolean",
			L"Sprite sprite, double xPos, double yPos, long nFlags",
			NULL, &RSSpriteMouseListenerClass::method_onMouseEvent, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"onMButtonDown", L"boolean",
			L"Sprite sprite, double xPos, double yPos, long nFlags",
			NULL, &RSSpriteMouseListenerClass::method_onMouseEvent, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"onMButtonUp", L"boolean",
			L"Sprite sprite, double xPos, double yPos, long nFlags",
			NULL, &RSSpriteMouseListenerClass::method_onMouseEvent, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"onMButtonDblClk", L"boolean",
			L"Sprite sprite, double xPos, double yPos, long nFlags",
			NULL, &RSSpriteMouseListenerClass::method_onMouseEvent, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"afterLButtonDown", L"boolean",
			L"Sprite sprite, double xPos, double yPos, long nFlags",
			NULL, &RSSpriteMouseListenerClass::method_onMouseEvent, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"afterLButtonUp", L"boolean",
			L"Sprite sprite, double xPos, double yPos, long nFlags",
			NULL, &RSSpriteMouseListenerClass::method_onMouseEvent, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"afterLButtonDblClk", L"boolean",
			L"Sprite sprite, double xPos, double yPos, long nFlags",
			NULL, &RSSpriteMouseListenerClass::method_onMouseEvent, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"afterRButtonDown", L"boolean",
			L"Sprite sprite, double xPos, double yPos, long nFlags",
			NULL, &RSSpriteMouseListenerClass::method_onMouseEvent, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"afterRButtonUp", L"boolean",
			L"Sprite sprite, double xPos, double yPos, long nFlags",
			NULL, &RSSpriteMouseListenerClass::method_onMouseEvent, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"afterRButtonDblClk", L"boolean",
			L"Sprite sprite, double xPos, double yPos, long nFlags",
			NULL, &RSSpriteMouseListenerClass::method_onMouseEvent, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"afterMButtonDown", L"boolean",
			L"Sprite sprite, double xPos, double yPos, long nFlags",
			NULL, &RSSpriteMouseListenerClass::method_onMouseEvent, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"afterMButtonUp", L"boolean",
			L"Sprite sprite, double xPos, double yPos, long nFlags",
			NULL, &RSSpriteMouseListenerClass::method_onMouseEvent, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"afterMButtonDblClk", L"boolean",
			L"Sprite sprite, double xPos, double yPos, long nFlags",
			NULL, &RSSpriteMouseListenerClass::method_onMouseEvent, NULL ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"getMouseID", L"int", L"long nFlags",
			NULL, &RSSpriteMouseListenerClass::method_getMouseID, NULL ) ;
}

// this オブジェクトを取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLSpriteMouseStateListener *
	RSSpriteMouseListenerClass::GetThisMouseListener( RSContext& context, RSObject* pThis )
{
	SGLSpriteMouseStateListener *
		pListener = ESLTypeCast<RSSpriteMouseListener>( pThis ) ;
	if ( pListener == NULL )
	{
		context.ThrowExceptionError( L"this が SpriteMouseStateListener ではありません" ) ;
	}
	return	pListener ;
}

// int getPointerCount()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteMouseListenerClass::method_getPointerCount
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteMouseStateListener *
			pListener = GetThisMouseListener( context, pThis ) ;
	if ( pListener == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( pListener->GetPointerCount() ) ;
}

// int findMouseIndexById( int idMouse )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteMouseListenerClass::method_findMouseIndexById
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteMouseStateListener *
			pListener = GetThisMouseListener( context, pThis ) ;
	if ( pListener == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Integer
				( pListener->FindMouseIndexById( arg.IntAt( 0 ) ) ) ;
}

// Vector2D getMousePointAt( int i )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteMouseListenerClass::method_getMousePointAt
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteMouseStateListener *
			pListener = GetThisMouseListener( context, pThis ) ;
	if ( pListener == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S2DDVector	vPos ;
	if ( !pListener->GetMousePointAt( (size_t) arg.IntAt( 0 ), vPos ) )
	{
		return	NULL ;
	}
	RSObject *	pObjPos = context.new_Object( L"Vector2D" ) ;
	ESLAssert( pObjPos != NULL ) ;
	pObjPos->SetMemberNumberAs( context, L"x", vPos.x ) ;
	pObjPos->SetMemberNumberAs( context, L"y", vPos.y ) ;
	return	pObjPos ;
}

// boolean isLButtonDownAt( int i )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteMouseListenerClass::method_isLButtonDownAt
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteMouseStateListener *
			pListener = GetThisMouseListener( context, pThis ) ;
	if ( pListener == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Boolean
				( pListener->IsLButtonDownAt( (size_t) arg.IntAt( 0 ) ) ) ;
}

// boolean isRButtonDownAt( int i )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteMouseListenerClass::method_isRButtonDownAt
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteMouseStateListener *
			pListener = GetThisMouseListener( context, pThis ) ;
	if ( pListener == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Boolean
				( pListener->IsRButtonDownAt( (size_t) arg.IntAt( 0 ) ) ) ;
}

// Vector2D enumerateLDownPoints()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteMouseListenerClass::method_enumerateLDownPoints
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteMouseStateListener *
			pListener = GetThisMouseListener( context, pThis ) ;
	if ( pListener == NULL )
	{
		return	NULL ;
	}
	SArray<S2DDVector>	arrPoints ;
	pListener->EnumerateLDownPoints( arrPoints ) ;
	//
	RSStructuredPointer *	pPtrPoints =
		ESLTypeCast<RSStructuredPointer>
			( context.new_StructuredPointer
				( L"Vector2D", arrPoints.GetLength() ) ) ;
	S2DVector *	pvPoints = (S2DVector*) pPtrPoints->GetPointer() ;
	for ( size_t i = 0; i < arrPoints.GetLength(); i ++ )
	{
		S2DDVector *	pvPoint = arrPoints.GetAt( i ) ;
		ESLAssert( pvPoint != NULL ) ;
		pvPoints[i] = *pvPoint ;
	}
	return	pPtrPoints ;
}

// boolean onMouseEvent
//	( Sprite sprite, double xPos, double yPos, long nFlags )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteMouseListenerClass::method_onMouseEvent
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	return	context.new_Boolean( false ) ;
}

// void onMouseLeave( Sprite sprite, long nFlags )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteMouseListenerClass::method_onMouseLeave
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	return	NULL ;
}

// boolean onMouseWheel
//	( Sprite sprite, int zDelta, double xPos, double yPos, long nFlags )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteMouseListenerClass::method_onMouseWheel
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	return	context.new_Boolean( false ) ;
}

// int getMouseID( long nFlags )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteMouseListenerClass::method_getMouseID
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Integer
				( SGLSpriteMouseListener::GetMouseID( arg.LongAt( 0 ) ) ) ;
}



//////////////////////////////////////////////////////////////////////////////
// キー入力インターフェースオブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
	( Rosetta::RSSpriteKeyListener, RSGenericObject, SGLSpriteKeyListener )

	// 消滅関数
//////////////////////////////////////////////////////////////////////////////
RSSpriteKeyListener::~RSSpriteKeyListener( void )
{
}

// 複製（参照の複製を含む）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteKeyListener::DuplicateObject( RSContext& context ) const
{
	ESLAssert( m_pClass != NULL ) ;
	RSSpriteKeyListener *
			pObj = new RSSpriteKeyListener( context.GetVM(), m_pClass ) ;
	pObj->m_gcmMembers.DuplicateAllMembers
				( context, m_gcmMembers.m_members ) ;
	return	pObj ;
}

// 複製（実体も可能な限り複製）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteKeyListener::CloneObject( RSContext& context ) const
{
	ESLAssert( m_pClass != NULL ) ;
	RSSpriteKeyListener *
		pObj = new RSSpriteKeyListener( context.GetVM(), m_pClass ) ;
	pObj->m_gcmMembers.CloneAllMembers
				( context, m_gcmMembers.m_members ) ;
	return	pObj ;
}

// キー入力
//////////////////////////////////////////////////////////////////////////////
bool RSSpriteKeyListener::OnKeyDown
	( SGLSprite& sprite, int64_t nVirtKey, int64_t nFlags )
{
	ESLAssert( m_vm != NULL ) ;
	RSContext	context( m_vm ) ;
	RSFunctionObject *	pFunc =
			m_pClass->GetVirtualMemberAs( context, L"onKeyDown" ) ;
	bool	fResult = false ;
	if ( pFunc != NULL )
	{
		RSObject *	pArgs[3] ;
		pArgs[0] = new RSNativeObject
					( &sprite, context.GetClassAs( sprite.GetRSClassName() ) ) ;
		pArgs[1] = context.new_Integer( nVirtKey ) ;
		pArgs[2] = context.new_Integer( nFlags ) ;
		//
		RSSmartPtr	pResult
			( context.CallFunction( *pFunc, this, &pArgs[0], 3, false ) ) ;
		if ( pResult != NULL )
		{
			fResult = pResult->AsBoolean() ;
		}
		//
		RSObject::ReleaseRef( pArgs[0] ) ;
		RSObject::ReleaseRef( pArgs[1] ) ;
		RSObject::ReleaseRef( pArgs[2] ) ;
		RSObject::ReleaseRef( pFunc ) ;
	}
	return	fResult ;
}

bool RSSpriteKeyListener::OnKeyUp
	( SGLSprite& sprite, int64_t nVirtKey, int64_t nFlags )
{
	ESLAssert( m_vm != NULL ) ;
	RSContext	context( m_vm ) ;
	RSFunctionObject *	pFunc =
			m_pClass->GetVirtualMemberAs( context, L"onKeyUp" ) ;
	bool	fResult = false ;
	if ( pFunc != NULL )
	{
		RSObject *	pArgs[3] ;
		pArgs[0] = new RSNativeObject
					( &sprite, context.GetClassAs( sprite.GetRSClassName() ) ) ;
		pArgs[1] = context.new_Integer( nVirtKey ) ;
		pArgs[2] = context.new_Integer( nFlags ) ;
		//
		RSSmartPtr	pResult
			( context.CallFunction( *pFunc, this, &pArgs[0], 3, false ) ) ;
		if ( pResult != NULL )
		{
			fResult = pResult->AsBoolean() ;
		}
		//
		RSObject::ReleaseRef( pArgs[0] ) ;
		RSObject::ReleaseRef( pArgs[1] ) ;
		RSObject::ReleaseRef( pArgs[2] ) ;
		RSObject::ReleaseRef( pFunc ) ;
	}
	return	fResult ;
}

// 文字入力
//////////////////////////////////////////////////////////////////////////////
bool RSSpriteKeyListener::OnChar( SGLSprite& sprite, uint16_t codeChar )
{
	ESLAssert( m_vm != NULL ) ;
	RSContext	context( m_vm ) ;
	RSFunctionObject *	pFunc =
			m_pClass->GetVirtualMemberAs( context, L"onChar" ) ;
	bool	fResult = false ;
	if ( pFunc != NULL )
	{
		RSObject *	pArgs[2] ;
		pArgs[0] = new RSNativeObject
					( &sprite, context.GetClassAs( sprite.GetRSClassName() ) ) ;
		pArgs[1] = context.new_Integer( codeChar ) ;
		//
		RSSmartPtr	pResult
			( context.CallFunction( *pFunc, this, &pArgs[0], 2, false ) ) ;
		if ( pResult != NULL )
		{
			fResult = pResult->AsBoolean() ;
		}
		//
		RSObject::ReleaseRef( pArgs[0] ) ;
		RSObject::ReleaseRef( pArgs[1] ) ;
		RSObject::ReleaseRef( pFunc ) ;
	}
	return	fResult ;
}

// コンポジション開始
//////////////////////////////////////////////////////////////////////////////
bool RSSpriteKeyListener::OnStartComposition
	( SGLSprite& sprite, SGLInputStartComposition& iscForm )
{
	ESLAssert( m_vm != NULL ) ;
	RSContext	context( m_vm ) ;
	RSFunctionObject *	pFunc =
			m_pClass->GetVirtualMemberAs( context, L"onStartComposition" ) ;
	bool	fResult = false ;
	if ( pFunc != NULL )
	{
		RSObject *	pArgs[2] ;
		pArgs[0] = new RSNativeObject
					( &sprite, context.GetClassAs( sprite.GetRSClassName() ) ) ;
		pArgs[1] = context.new_Object( L"SpriteKeyListener.StartComposition" ) ;
		ESLAssert( pArgs[1] != NULL ) ;
		//
		RSSmartPtr	pResult
			( context.CallFunction( *pFunc, this, &pArgs[0], 2, false ) ) ;
		if ( pResult != NULL )
		{
			fResult = pResult->AsBoolean() ;
			//
			if ( pArgs[1] != NULL )
			{
				iscForm.nFlags =
					pArgs[1]->GetMemberIntegerAs( context, L"nFlags" ) ;
				//
				RSSmartPtr	pptStart =
								pArgs[1]->GetMemberAs( context, L"ptStart" ) ;
				if ( pptStart != NULL )
				{
					iscForm.ptStart.x =
						(int32_t) pptStart->GetMemberIntegerAs( context, L"x" ) ;
					iscForm.ptStart.y =
						(int32_t) pptStart->GetMemberIntegerAs( context, L"y" ) ;
				}
				//
				RSSmartPtr	prctArea =
								pArgs[1]->GetMemberAs( context, L"rctArea" ) ;
				if ( prctArea != NULL )
				{
					iscForm.rctArea.x =
						(int32_t) prctArea->GetMemberIntegerAs( context, L"x" ) ;
					iscForm.rctArea.y =
						(int32_t) prctArea->GetMemberIntegerAs( context, L"y" ) ;
					iscForm.rctArea.w =
						(int32_t) prctArea->GetMemberIntegerAs( context, L"w" ) ;
					iscForm.rctArea.h =
						(int32_t) prctArea->GetMemberIntegerAs( context, L"h" ) ;
				}
				//
				RSSmartPtr	pfsFontStyle =
								pArgs[1]->GetMemberAs( context, L"fsFontStyle" ) ;
				if ( pfsFontStyle != NULL )
				{
					RSFontStyleClass::ConvertFromObject
						( context, iscForm.fsFontStyle,
								m_strFontFaceTemp, pfsFontStyle ) ;
				}
			}
		}
		//
		RSObject::ReleaseRef( pArgs[0] ) ;
		RSObject::ReleaseRef( pArgs[1] ) ;
		RSObject::ReleaseRef( pFunc ) ;
	}
	return	fResult ;
}

// コンポジション終了
//////////////////////////////////////////////////////////////////////////////
bool RSSpriteKeyListener::OnEndComposition( SGLSprite& sprite )
{
	ESLAssert( m_vm != NULL ) ;
	RSContext	context( m_vm ) ;
	RSFunctionObject *	pFunc =
			m_pClass->GetVirtualMemberAs( context, L"onEndComposition" ) ;
	bool	fResult = false ;
	if ( pFunc != NULL )
	{
		RSObject *	pArgs[1] ;
		pArgs[0] = new RSNativeObject
					( &sprite, context.GetClassAs( sprite.GetRSClassName() ) ) ;
		//
		RSSmartPtr	pResult
			( context.CallFunction( *pFunc, this, &pArgs[0], 1, false ) ) ;
		if ( pResult != NULL )
		{
			fResult = pResult->AsBoolean() ;
		}
		//
		RSObject::ReleaseRef( pArgs[0] ) ;
		RSObject::ReleaseRef( pFunc ) ;
	}
	return	fResult ;
}

// コンポジション文字列
//////////////////////////////////////////////////////////////////////////////
bool RSSpriteKeyListener::OnCompositionString
	( SGLSprite& sprite, const SGLInputCompositionString& icsComp )
{
	ESLAssert( m_vm != NULL ) ;
	RSContext	context( m_vm ) ;
	RSFunctionObject *	pFunc =
			m_pClass->GetVirtualMemberAs( context, L"onCompositionString" ) ;
	bool	fResult = false ;
	if ( pFunc != NULL )
	{
		RSObject *	pArgs[2] ;
		pArgs[0] = new RSNativeObject
					( &sprite, context.GetClassAs( sprite.GetRSClassName() ) ) ;
		pArgs[1] = context.new_Object( L"SpriteKeyListener.CompositionString" ) ;
		ESLAssert( pArgs[1] != NULL ) ;
		if ( pArgs[1] != NULL )
		{
			pArgs[1]->SetMemberIntegerAs
					( context, L"nFlags", icsComp.nFlags ) ;
			pArgs[1]->SetMemberStringAs
					( context, L"strComposition", icsComp.pszComposition ) ;
			pArgs[1]->SetMemberIntegerAs
					( context, L"nStart", icsComp.nStart ) ;
			pArgs[1]->SetMemberIntegerAs
					( context, L"nCount", icsComp.nCount ) ;
		}
		//
		RSSmartPtr	pResult
			( context.CallFunction( *pFunc, this, &pArgs[0], 2, false ) ) ;
		if ( pResult != NULL )
		{
			fResult = pResult->AsBoolean() ;
		}
		//
		RSObject::ReleaseRef( pArgs[0] ) ;
		RSObject::ReleaseRef( pArgs[1] ) ;
		RSObject::ReleaseRef( pFunc ) ;
	}
	return	fResult ;
}

// コマンド
//////////////////////////////////////////////////////////////////////////////
bool RSSpriteKeyListener::OnCommand
	( SGLSprite& sprite, const wchar_t * pszCmd,
		int64_t nParam, int64_t nCode, int nPriority, bool fOverwritable )
{
	ESLAssert( m_vm != NULL ) ;
	RSContext	context( m_vm ) ;
	RSFunctionObject *	pFunc =
			m_pClass->GetVirtualMemberAs( context, L"onCommand" ) ;
	bool	fResult = false ;
	if ( pFunc != NULL )
	{
		RSObject *	pArgs[6] ;
		pArgs[0] = new RSNativeObject
					( &sprite, context.GetClassAs( sprite.GetRSClassName() ) ) ;
		pArgs[1] = context.new_String( pszCmd ) ;
		pArgs[2] = context.new_Integer( nParam ) ;
		pArgs[3] = context.new_Integer( nCode ) ;
		pArgs[4] = context.new_Integer( nPriority ) ;
		pArgs[5] = context.new_Boolean( fOverwritable ) ;
		//
		RSSmartPtr	pResult
			( context.CallFunction( *pFunc, this, &pArgs[0], 6, false ) ) ;
		if ( pResult != NULL )
		{
			fResult = pResult->AsBoolean() ;
		}
		//
		RSObject::ReleaseRef( pArgs[0] ) ;
		RSObject::ReleaseRef( pArgs[1] ) ;
		RSObject::ReleaseRef( pArgs[2] ) ;
		RSObject::ReleaseRef( pArgs[3] ) ;
		RSObject::ReleaseRef( pArgs[4] ) ;
		RSObject::ReleaseRef( pArgs[5] ) ;
		RSObject::ReleaseRef( pFunc ) ;
	}
	return	fResult ;
}



//////////////////////////////////////////////////////////////////////////////
// SpriteKeyListener.StartComposition
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSSpriteKeyListenerClass::StartCompositionClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSSpriteKeyListenerClass::StartCompositionClass::StartCompositionClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSSpriteKeyListenerClass::StartCompositionClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSGenericObject( this ) ;
	//
	context.ReleaseObjectRef
		( m_pPrototype->CreateMemberAs
			( context, L"nFlags", context.new_Integer(0) ) ) ;
	context.ReleaseObjectRef
		( m_pPrototype->CreateMemberAs
			( context, L"ptStart", context.new_ObjectPointer( L"Point" ) ) ) ;
	context.ReleaseObjectRef
		( m_pPrototype->CreateMemberAs
			( context, L"rctArea", context.new_ObjectPointer( L"Rect" ) ) ) ;
	context.ReleaseObjectRef
		( m_pPrototype->CreateMemberAs
			( context, L"fsFontStyle", context.new_ObjectPointer( L"FontStyle" ) ) ) ;
}


//////////////////////////////////////////////////////////////////////////////
// SpriteKeyListener.CompositionString
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( Rosetta::RSSpriteKeyListenerClass::CompositionStringClass, RSClass )
// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSSpriteKeyListenerClass::CompositionStringClass::CompositionStringClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSSpriteKeyListenerClass::CompositionStringClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSGenericObject( this ) ;
	//
	m_pPrototype->CreateMemberIntegerAs( context, L"nFlags", 0 ) ;
	m_pPrototype->CreateMemberStringAs( context, L"strComposition", L"" ) ;
	m_pPrototype->CreateMemberIntegerAs( context, L"nStart", 0 ) ;
	m_pPrototype->CreateMemberIntegerAs( context, L"nCount", 0 ) ;
}



//////////////////////////////////////////////////////////////////////////////
// キー入力インターフェースクラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSSpriteKeyListenerClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSSpriteKeyListenerClass::RSSpriteKeyListenerClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSSpriteKeyListenerClass::OverrideVirtuals( RSContext& context )
{
	StartCompositionClass *
		pStartCompClass =
			new StartCompositionClass( context.GetVM()->GetClassClass() ) ;
	pStartCompClass->Initialize( context ) ;
	pStartCompClass->FinishClass( context ) ;
	context.ReleaseObjectRef
		( CreateMemberAs( context, L"StartComposition", pStartCompClass ) ) ;
	//
	CompositionStringClass *
		pCompStringClass =
			new CompositionStringClass( context.GetVM()->GetClassClass() ) ;
	pCompStringClass->Initialize( context ) ;
	pCompStringClass->FinishClass( context ) ;
	context.ReleaseObjectRef
		( CreateMemberAs( context, L"CompositionString", pCompStringClass ) ) ;
	//
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSSpriteKeyListener( context.GetVM(), this ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"onKeyDown", L"boolean",
			L"Sprite sprite, long nVirtKey, long nFlags",
			NULL, &RSSpriteKeyListenerClass::method_onDummyKeyEvent, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"onKeyUp", L"boolean",
			L"Sprite sprite, long nVirtKey, long nFlags",
			NULL, &RSSpriteKeyListenerClass::method_onDummyKeyEvent, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"onChar", L"boolean",
			L"Sprite sprite, char codeChar",
			NULL, &RSSpriteKeyListenerClass::method_onDummyKeyEvent, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"onStartComposition", L"boolean",
			L"Sprite sprite, SpriteKeyListener.StartComposition iscForm",
			NULL, &RSSpriteKeyListenerClass::method_onDummyKeyEvent, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"onEndComposition", L"boolean", L"Sprite sprite",
			NULL, &RSSpriteKeyListenerClass::method_onDummyKeyEvent, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"onCompositionString", L"boolean",
			L"Sprite sprite, SpriteKeyListener.CompositionString icsComp",
			NULL, &RSSpriteKeyListenerClass::method_onDummyKeyEvent, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"onCommand", L"boolean",
			L"Sprite sprite, String strCmd, "
			L"long nParam, long nCode, int nPriority, boolean fOverwritable",
			NULL, &RSSpriteKeyListenerClass::method_onDummyKeyEvent, NULL ) ;
}

RSObject * RSSpriteKeyListenerClass::method_onDummyKeyEvent
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	return	context.new_Boolean( false ) ;
}


//////////////////////////////////////////////////////////////////////////////
// Sprite.Parameter クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSSpriteClass::ParameterClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSSpriteClass::ParameterClass::ParameterClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSSpriteClass::ParameterClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSGenericObject( this ) ;
	//
	m_pPrototype->CreateMemberIntegerAs
		( context, L"nFlags", paintSmoothStretch ) ;
	m_pPrototype->CreateMemberIntegerAs( context, L"nSpriteFlags", 0 ) ;
	RSObject::ReleaseRef
		( m_pPrototype->CreateMemberAs
			( context, L"vDst",
				context.new_ObjectPointer( L"Vector3D" ) ) ) ;
	RSObject::ReleaseRef
		( m_pPrototype->CreateMemberAs
			( context, L"vCenter",
				context.new_ObjectPointer( L"Vector2D" ) ) ) ;
	RSObject *	pObjZoom = context.new_ObjectPointer( L"Vector2D" ) ;
	pObjZoom->SetMemberNumberAs( context, L"x", 1.0 ) ;
	pObjZoom->SetMemberNumberAs( context, L"y", 1.0 ) ;
	RSObject::ReleaseRef
		( m_pPrototype->CreateMemberAs( context, L"vZoom", pObjZoom ) ) ;
	m_pPrototype->CreateMemberNumberAs( context, L"zAngle", 0 ) ;
	m_pPrototype->CreateMemberNumberAs( context, L"xyCross", 90 ) ;
	m_pPrototype->CreateMemberIntegerAs( context, L"nTransparency", 0 ) ;
	m_pPrototype->CreateMemberIntegerAs( context, L"paramFilter", 0 ) ;
	m_pPrototype->CreateMemberIntegerAs( context, L"paramFilter2", 0 ) ;
	m_pPrototype->CreateMemberIntegerAs( context, L"rgbColorParam", 0 ) ;
	RSObject::ReleaseRef
		( m_pPrototype->CreateMemberAs
			( context, L"pVertices",
				context.new_Pointer
					( NULL, context.GetClassAs( L"Vector2D" ) ) ) ) ;
}

// Object -> SGLSprite::Parameter 変換
//////////////////////////////////////////////////////////////////////////////
void RSSpriteClass::ParameterClass::ParameterFromObject
	( RSContext& context, SakuraGL::SGLSprite::Parameter& param,
			SSystem::SArray<S2DVector>& vertics, RSObject * obj )
{
	param.nFlags =
		(uint32_t) obj->GetMemberIntegerAs( context, L"nFlags" ) ;
	param.nSpriteFlags =
		(uint32_t) obj->GetMemberIntegerAs( context, L"nSpriteFlags" ) ;
	//
	RSSmartPtr	pObjDst( obj->GetMemberAs( context, L"vDst" ), &context ) ;
	if ( pObjDst != NULL )
	{
		param.vDst.x =
			pObjDst->GetMemberNumberAs( context, L"x", param.vDst.x ) ;
		param.vDst.y =
			pObjDst->GetMemberNumberAs( context, L"y", param.vDst.y ) ;
		param.vDst.z =
			pObjDst->GetMemberNumberAs( context, L"z", param.vDst.z ) ;
	}
	RSSmartPtr	pObjCenter( obj->GetMemberAs( context, L"vCenter" ), &context ) ;
	if ( pObjCenter != NULL )
	{
		param.vCenter.x =
			pObjCenter->GetMemberNumberAs
						( context, L"x", param.vCenter.x ) ;
		param.vCenter.y =
			pObjCenter->GetMemberNumberAs
						( context, L"y", param.vCenter.y ) ;
	}
	RSSmartPtr	pObjZoom( obj->GetMemberAs( context, L"vZoom" ), &context ) ;
	if ( pObjZoom != NULL )
	{
		param.vZoom.x =
			pObjZoom->GetMemberNumberAs
						( context, L"x", param.vZoom.x ) ;
		param.vZoom.y =
			pObjZoom->GetMemberNumberAs
						( context, L"y", param.vZoom.y ) ;
	}
	param.zAngle = obj->GetMemberNumberAs( context, L"zAngle" ) ;
	param.xyCross = obj->GetMemberNumberAs( context, L"xyCross" ) ;
	param.nTransparency =
		(uint32_t) obj->GetMemberIntegerAs( context, L"nTransparency" ) ;
	param.paramFilter =
		(int32_t) obj->GetMemberIntegerAs( context, L"paramFilter" ) ;
	param.paramFilter2 =
		(int32_t) obj->GetMemberIntegerAs( context, L"paramFilter2" ) ;
	param.rgbColorParam.ui32 =
		(uint32_t) obj->GetMemberIntegerAs( context, L"rgbColorParam" ) ;
	param.countVertex = 0 ;
	param.pVertices = NULL ;
	//
	RSSmartPtr	pObjVertices
		( obj->GetMemberAs( context, L"pVertices" ), &context ) ;
	RSStructuredPointer *
		pPtrVertices = ESLTypeCast<RSStructuredPointer>( pObjVertices.Ptr() ) ;
	if ( pPtrVertices != NULL )
	{
		param.countVertex = pPtrVertices->GetElementCount() ;
		param.pVertices = vertics.GetArray( param.countVertex ) ;
		eslMoveMemory
			( vertics.GetArray(),
				pPtrVertices->GetPointer(),
				param.countVertex * sizeof(S2DVector) ) ;
		vertics.FinishArray() ;
	}
}

// Object <- SGLSprite::Parameter 変換
//////////////////////////////////////////////////////////////////////////////
void RSSpriteClass::ParameterClass::ParameterToObject
	( RSContext& context, RSObject * obj,
			const SakuraGL::SGLSprite::Parameter& param )
{
	obj->SetMemberIntegerAs( context, L"nFlags", param.nFlags ) ;
	obj->SetMemberIntegerAs( context, L"nSpriteFlags", param.nSpriteFlags ) ;
	//
	RSSmartPtr	pObjDst( obj->GetMemberAs( context, L"vDst" ), &context ) ;
	if ( pObjDst != NULL )
	{
		pObjDst->SetMemberNumberAs( context, L"x", param.vDst.x ) ;
		pObjDst->SetMemberNumberAs( context, L"y", param.vDst.y ) ;
		pObjDst->SetMemberNumberAs( context, L"z", param.vDst.z ) ;
	}
	RSSmartPtr	pObjCenter( obj->GetMemberAs( context, L"vCenter" ), &context ) ;
	if ( pObjCenter != NULL )
	{
		pObjCenter->SetMemberNumberAs( context, L"x", param.vCenter.x ) ;
		pObjCenter->SetMemberNumberAs( context, L"y", param.vCenter.y ) ;
	}
	RSSmartPtr	pObjZoom( obj->GetMemberAs( context, L"vZoom" ), &context ) ;
	if ( pObjZoom != NULL )
	{
		pObjZoom->SetMemberNumberAs( context, L"x", param.vZoom.x ) ;
		pObjZoom->SetMemberNumberAs( context, L"y", param.vZoom.y ) ;
	}
	pObjZoom->SetMemberNumberAs( context, L"zAngle", param.zAngle ) ;
	pObjZoom->SetMemberNumberAs( context, L"xyCross", param.xyCross ) ;
	pObjZoom->SetMemberIntegerAs( context, L"nTransparency", param.nTransparency ) ;
	pObjZoom->SetMemberIntegerAs( context, L"paramFilter", param.paramFilter ) ;
	pObjZoom->SetMemberIntegerAs( context, L"paramFilter2", param.paramFilter2 ) ;
	pObjZoom->SetMemberIntegerAs( context, L"rgbColorParam", param.rgbColorParam.ui32 ) ;
	//
	if ( (param.countVertex > 0)
		&& (param.pVertices != NULL) )
	{
		RSStructuredPointer *	pPtrVertices =
			ESLSmartCast<RSStructuredPointer>
				( context.new_StructuredPointer
					( L"Vector2D", param.countVertex ) ) ;
		if ( pPtrVertices != NULL )
		{
			eslMoveMemory
				( pPtrVertices->GetPointer(),
					param.pVertices,
					param.countVertex * sizeof(S2DVector) ) ;
		}
		RSObject::ReleaseRef
			( pObjZoom->SetMemberAs
				( context, L"pVertices",
					context.new_Pointer(pPtrVertices) ) ) ;
	}
	else
	{
		RSObject::ReleaseRef
			( pObjZoom->SetMemberAs
				( context, L"pVertices", context.new_Pointer(NULL) ) ) ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// Sprite クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSSpriteClass, RGenericNativeObjectClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSSpriteClass::RSSpriteClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RGenericNativeObjectClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSSpriteClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( NULL, this ) ;
	//
	ParameterClass *	pParamClass =
		new ParameterClass( context.GetClassClass(), L"Parameter" ) ;
	pParamClass->Initialize( context ) ;
	pParamClass->FinishClass( context ) ;
	context.ReleaseObjectRef
		( CreateMemberAs( context, L"Parameter", pParamClass ) ) ;
	//
	CreateMemberIntegerAs
		( context, L"regCenter", SGLSprite::regCenter, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"regLeft", SGLSprite::regLeft, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"regRight", SGLSprite::regRight, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"regVCenter", SGLSprite::regVCenter, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"regTop", SGLSprite::regTop, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"regBottom", SGLSprite::regBottom, modifierConst ) ;
	//
	CreateMemberIntegerAs
		( context, L"flagZScale", SGLSprite::flagZScale, modifierConst ) ;
	//
	CreateMemberIntegerAs
		( context, L"scrollDefault", SGLSprite::scrollDefault, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"scrollHorz", SGLSprite::scrollHorz, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"scrollVert", SGLSprite::scrollVert, modifierConst ) ;
	//
	CreateMemberIntegerAs
		( context, L"commandLow", SGLSprite::commandLow, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"commandBelow", SGLSprite::commandBelow, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"commandNormal", SGLSprite::commandNormal, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"commandAbove", SGLSprite::commandAbove, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"commandHigh", SGLSprite::commandHigh, modifierConst ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"",
				NULL, &RSSpriteClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getUIFlag", L"long", L"",
				NULL, &RSSpriteClass::method_getUIFlag,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"modifyUIFlag",
				L"long", L"long nAddFlags, long nRemoveFlags = 0",
				NULL, &RSSpriteClass::method_modifyUIFlag ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getClickableRect", L"Rect", L"",
				NULL, &RSSpriteClass::method_getClickableRect,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setClickableRect", NULL, L"Rect rect",
				NULL, &RSSpriteClass::method_setClickableRect ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getParameter", L"Sprite.Parameter", L"",
				NULL, &RSSpriteClass::method_getParameter,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getPosition", L"Vector2D", L"",
				NULL, &RSSpriteClass::method_getPosition,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getCenterPosition", L"Vector2D", L"",
				NULL, &RSSpriteClass::method_getCenterPosition,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getZoom", L"Vector2D", L"",
				NULL, &RSSpriteClass::method_getZoom,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getRotation", L"double", L"",
				NULL, &RSSpriteClass::method_getRotation,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getTransparency", L"int", L"",
				NULL, &RSSpriteClass::method_getTransparency,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getFilterParameter", L"int", L"",
				NULL, &RSSpriteClass::method_getFilterParameter,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getFilter2Parameter", L"int", L"",
				NULL, &RSSpriteClass::method_getFilter2Parameter,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"isVisible", L"boolean", L"",
				NULL, &RSSpriteClass::method_isVisible,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getPriority", L"int", L"",
				NULL, &RSSpriteClass::method_getPriority,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getID", L"String", L"",
				NULL, &RSSpriteClass::method_getID,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getRectangle", L"Rect", L"",
				NULL, &RSSpriteClass::method_getRectangle,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setParameter", NULL, L"Sprite.Parameter param",
				NULL, &RSSpriteClass::method_setParameter, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setPosition", NULL, L"double x, double y",
				NULL, &RSSpriteClass::method_setPosition, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setCenterPosition", NULL, L"double x, double y",
				NULL, &RSSpriteClass::method_setCenterPosition, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setZoom", NULL, L"double x, double y",
				NULL, &RSSpriteClass::method_setZoom, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setRotation", NULL, L"double zAngle",
				NULL, &RSSpriteClass::method_setRotation, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setTransparency", NULL, L"int nTransparency",
				NULL, &RSSpriteClass::method_setTransparency, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setFilterParameter", NULL, L"int paramFilter",
				NULL, &RSSpriteClass::method_setFilterParameter, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setFilter2Parameter", NULL, L"int paramFilter",
				NULL, &RSSpriteClass::method_setFilter2Parameter, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"regulateCenter", L"boolean",
			L"int nFlags = 0, double xOffset = 0.0, double yOffset = 0.0",
			NULL, &RSSpriteClass::method_regulateCenter, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setVisible", NULL, L"boolean visible",
				NULL, &RSSpriteClass::method_setVisible, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"changePriority", NULL, L"int nPriority",
				NULL, &RSSpriteClass::method_changePriority, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setID", NULL, L"String id",
				NULL, &RSSpriteClass::method_setID, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"postUpdate", NULL, L"",
				NULL, &RSSpriteClass::method_postUpdate, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"freezeFrameUpdate", NULL, L"",
				NULL, &RSSpriteClass::method_freezeFrameUpdate, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"defrostFrameUpdate", NULL, L"",
				NULL, &RSSpriteClass::method_defrostFrameUpdate, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"addChild", NULL, L"Sprite sprite",
				NULL, &RSSpriteClass::method_addChild, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"detachChild", NULL, L"Sprite sprite",
				NULL, &RSSpriteClass::method_detachChild, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"detachAllChildren", NULL, L"",
				NULL, &RSSpriteClass::method_detachAllChildren, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getParent", L"Sprite", L"",
				NULL, &RSSpriteClass::method_getParent,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getItemAs", L"Sprite", L"String id",
				NULL, &RSSpriteClass::method_getItemAs,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getHitSpriteAt", L"Sprite", L"double x, double y",
				NULL, &RSSpriteClass::method_getHitSpriteAt,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"isHitSprite", L"boolean", L"double x, double y",
				NULL, &RSSpriteClass::method_isHitSprite,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getChildCount", L"int", L"",
				NULL, &RSSpriteClass::method_getChildCount,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getChildAt", L"Sprite", L"int i",
				NULL, &RSSpriteClass::method_getChildAt,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"loadImage", L"boolean", L"String file",
				NULL, &RSSpriteClass::method_loadImage, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"attachAnimation", NULL, L"Image image",
				NULL, &RSSpriteClass::method_attachAnimation, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"beginAnimation", NULL,
			L"int loop = -1, int iLoopStart = 0, int iLoopEnd = 0,"
			L"int iAnimeStart = 0, int msecDuration = 0",
			NULL, &RSSpriteClass::method_beginAnimation, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setLoopAnimation", NULL,
			L"int loop = -1, int iLoopStart = 0, int iLoopEnd = 0",
			NULL, &RSSpriteClass::method_setLoopAnimation, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getImageSize", L"Size", L"",
				NULL, &RSSpriteClass::method_getImageSize,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"createBuffer", L"boolean",
			L"int width, int height, int format = Image.formatDefaultRGBA,"
			L"int depth = 32, long nFlags = Image.bufferOnMemory,"
			L"boolean flagZBuffer = false, boolean flagStereo3D = false",
			NULL, &RSSpriteClass::method_createBuffer, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"releaseBuffer", NULL, L"",
				NULL, &RSSpriteClass::method_releaseBuffer, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"isBuffered", L"boolean", L"",
				NULL, &RSSpriteClass::method_isBuffered,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setFillBackColor",
				NULL, L"int argbFill, boolean flagFillBack = true",
				NULL, &RSSpriteClass::method_setFillBackColor, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getText", L"String", L"",
				NULL, &RSSpriteClass::method_getText,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setText", NULL, L"String text",
				NULL, &RSSpriteClass::method_setText, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setTextFont",
				NULL, L"String font, int nSize = 0",
				NULL, &RSSpriteClass::method_setTextFont, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"isEnabled", L"boolean", L"",
				NULL, &RSSpriteClass::method_isEnabled,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setEnable", NULL, L"boolean fEnable",
				NULL, &RSSpriteClass::method_setEnable, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getScrollPos",
				L"int", L"int scrlDir = Sprite.scrollDefault",
				NULL, &RSSpriteClass::method_getScrollPos,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setScrollPos",
				NULL, L"int nPos, int scrlDir = Sprite.scrollDefault",
				NULL, &RSSpriteClass::method_setScrollPos, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getScrollRange",
				L"int", L"int scrlDir = Sprite.scrollDefault",
				NULL, &RSSpriteClass::method_getScrollRange,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setScrollRange",
				NULL, L"int nRange, int scrlDir = Sprite.scrollDefault",
				NULL, &RSSpriteClass::method_setScrollRange, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getScrollPageSize",
				L"int", L"int scrlDir = Sprite.scrollDefault",
				NULL, &RSSpriteClass::method_getScrollPageSize,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setScrollPageSize",
				NULL, L"int nPageSize, int scrlDir = Sprite.scrollDefault",
				NULL, &RSSpriteClass::method_setScrollPageSize, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"isButtonChecked", L"boolean", L"",
				NULL, &RSSpriteClass::method_isButtonChecked,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"checkButton", NULL, L"boolean check",
				NULL, &RSSpriteClass::method_checkButton, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"invokeCommands", NULL, L"String xmlCommands",
				NULL, &RSSpriteClass::method_invokeCommands, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"isSpriteVisible", L"boolean", L"String id",
				NULL, &RSSpriteClass::method_isSpriteVisible,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setSpriteVisible",
				NULL, L"String id, boolean visible",
				NULL, &RSSpriteClass::method_setSpriteVisible, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getSpritePriority", L"int", L"String id",
				NULL, &RSSpriteClass::method_getSpritePriority,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"changeSpritePriority",
				NULL, L"String id, int priority",
				NULL, &RSSpriteClass::method_changeSpritePriority, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getSpriteTransparency", L"int", L"String id",
				NULL, &RSSpriteClass::method_getSpriteTransparency,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setSpriteTransparency",
				NULL, L"String id, int transparency",
				NULL, &RSSpriteClass::method_setSpriteTransparency, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getSpriteRectangle", L"Rect", L"String id",
				NULL, &RSSpriteClass::method_getSpriteRectangle,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getSpriteText", L"String", L"String id",
				NULL, &RSSpriteClass::method_getSpriteText,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setSpriteText",
				NULL, L"String id, String text",
				NULL, &RSSpriteClass::method_setSpriteText, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setSpriteTextFont",
				NULL, L"String id, String font, int nSize = 0",
				NULL, &RSSpriteClass::method_setSpriteTextFont, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setSpriteImage",
				NULL, L"String id, String image",
				NULL, &RSSpriteClass::method_setSpriteImage, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"isSpriteEnabled", L"boolean", L"String id",
				NULL, &RSSpriteClass::method_isSpriteEnabled,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setSpriteEnable",
				NULL, L"String id, boolean fEnable",
				NULL, &RSSpriteClass::method_setSpriteEnable, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getSpriteScrollPos",
				L"int", L"String id, int scrlDir = Sprite.scrollDefault",
				NULL, &RSSpriteClass::method_getSpriteScrollPos,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setSpriteScrollPos",
				NULL, L"String id, int nPos, int scrlDir = Sprite.scrollDefault",
				NULL, &RSSpriteClass::method_setSpriteScrollPos, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getSpriteScrollRange",
				L"int", L"String id, int scrlDir = Sprite.scrollDefault",
				NULL, &RSSpriteClass::method_getSpriteScrollRange,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setSpriteScrollRange",
				NULL, L"String id, int nRange, int scrlDir = Sprite.scrollDefault",
				NULL, &RSSpriteClass::method_setSpriteScrollRange, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getSpriteScrollPageSize",
				L"int", L"String id, int scrlDir = Sprite.scrollDefault",
				NULL, &RSSpriteClass::method_getSpriteScrollPageSize,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setSpriteScrollPageSize",
				NULL, L"String id, int nPageSize, int scrlDir = Sprite.scrollDefault",
				NULL, &RSSpriteClass::method_setSpriteScrollPageSize, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"isSpriteButtonChecked", L"boolean", L"String id",
				NULL, &RSSpriteClass::method_isSpriteButtonChecked,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"checkSpriteButton",
				NULL, L"String id, boolean check",
				NULL, &RSSpriteClass::method_checkSpriteButton, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setActionLinearTo", L"SpriteAction",
				L"int msecDuration, int nTransparency,"
				L"Vector2D vPos = null, Vector2D vZoom = null,"
				L"double a0 = 0.0, double a1 = 0.0",
				NULL, &RSSpriteClass::method_setActionLinearTo, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"addAction", NULL, L"SpriteAction act",
				NULL, &RSSpriteClass::method_addAction, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"flushAction", NULL, L"",
				NULL, &RSSpriteClass::method_flushAction, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"isAction", L"boolean", L"",
				NULL, &RSSpriteClass::method_isAction,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"pauseAllAction", NULL, L"",
				NULL, &RSSpriteClass::method_pauseAllAction, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"restartAllAction", NULL, L"",
				NULL, &RSSpriteClass::method_restartAllAction, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"addTimer", NULL, L"SpriteTimer timer",
				NULL, &RSSpriteClass::method_addTimer, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"removeTimer", NULL, L"SpriteTimer timer",
				NULL, &RSSpriteClass::method_removeTimer, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"attachMouseListener",
				NULL, L"SpriteMouseListener listener",
				NULL, &RSSpriteClass::method_attachMouseListener, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"detachMouseListener",
				NULL, L"SpriteMouseListener listener",
				NULL, &RSSpriteClass::method_detachMouseListener, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"attachKeyListener",
				NULL, L"SpriteKeyListener listener",
				NULL, &RSSpriteClass::method_attachKeyListener, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"detachKeyListener",
				NULL, L"SpriteKeyListener listener",
				NULL, &RSSpriteClass::method_detachKeyListener, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setMouseCapture", NULL, L"",
				NULL, &RSSpriteClass::method_setMouseCapture, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"releaseMouseCapture", NULL, L"",
				NULL, &RSSpriteClass::method_releaseMouseCapture, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setKeyFocus", NULL, L"",
				NULL, &RSSpriteClass::method_setKeyFocus, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"killKeyFocus", NULL, L"",
				NULL, &RSSpriteClass::method_killKeyFocus, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"hasKeyFocus", L"boolean", L"",
				NULL, &RSSpriteClass::method_hasKeyFocus,
				NULL, RSFunctionPrototype::flagConstant ) ;
}

// ネイティブ型テスト
//////////////////////////////////////////////////////////////////////////////
bool RSSpriteClass::IsNativeObjectOf( ESLObject * pObj ) const
{
	return	(ESLTypeCast<SGLSprite>(pObj) != nullptr) ;
}

// this オブジェクトのファイルを取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLSprite *
	RSSpriteClass::GetThisSprite( RSContext& context, RSObject* pThis )
{
	SGLSprite *			pSprite = nullptr ;
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj != NULL )
	{
		pSprite = ESLTypeCast<SGLSprite>( pNativeObj->GetObject() ) ;
	}
	if ( pSprite == nullptr )
	{
		if ( (pThis == nullptr) || (pThis->GetEntityObject() == nullptr) )
		{
			context.ThrowExceptionError
				( L"null ポインタから Sprite メソッドを呼び出しています",
												L"NullPointerException" ) ;
		}
		else
		{
			context.ThrowExceptionError( L"this が Sprite ではありません" ) ;
		}
	}
	return	pSprite ;
}

// SGLSprite 参照オブジェクト生成
//////////////////////////////////////////////////////////////////////////////
RSNativeObject * RSSpriteClass::CreateRefObject( SGLSprite * pSprite )
{
	return	new RSNativeObject( pSprite, this ) ;
}

// void <init>()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj == NULL )
	{
		context.ThrowExceptionError
			( L"Sprite.<init> の this が Sprite ではありません" ) ;
		return	NULL ;
	}
	pNativeObj->SetObject( new SGLSprite ) ;
	return	NULL ;
}

// const long getUIFlag()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_getUIFlag
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( pSprite->GetUIFlag() ) ;
}

// long modifyUIFlag( long nAddFlags, long nRemoveFlags = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_modifyUIFlag
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Integer
		( pSprite->ModifyUIFlag( arg.LongAt(0), arg.LongAt(1) ) ) ;
}

// const Rect getClickableRect()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_getClickableRect
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	SGLImageRect	rect = pSprite->GetClickableRect() ;
	RSObject *	pObjRect = context.new_Object( L"Rect" ) ;
	pObjRect->SetMemberIntegerAs( context, L"x", rect.x ) ;
	pObjRect->SetMemberIntegerAs( context, L"y", rect.y ) ;
	pObjRect->SetMemberIntegerAs( context, L"w", rect.w ) ;
	pObjRect->SetMemberIntegerAs( context, L"h", rect.h ) ;
	return	pObjRect ;
}

// void setClickableRect( Rect rect )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_setClickableRect
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLImageRect *	pRect =
			(SGLImageRect*) arg.PointerAt( 0, sizeof(SGLImageRect) ) ;
	pSprite->SetClickableRect( *pRect ) ;
	return	NULL ;
}

// Sprite.Parameter getParameter()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_getParameter
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	const SGLSprite::Parameter&	param = pSprite->GetParameter() ;
	RSObject *	pObjParam = context.new_Object( L"Sprite.Parameter" ) ;
	ParameterClass::ParameterToObject( context, pObjParam, param ) ;
	return	pObjParam ;
}

// Vector2D getPosition()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_getPosition
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	S2DDVector	vPos = pSprite->GetPosition2D() ;
	RSObject *	pPosObj = context.new_Object( L"Vector2D" ) ;
	pPosObj->SetMemberNumberAs( context, L"x", vPos.x ) ;
	pPosObj->SetMemberNumberAs( context, L"y", vPos.y ) ;
	return	pPosObj ;
}

// Vector2D getCenterPosition()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_getCenterPosition
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	S2DDVector	vPos = pSprite->GetCenterPosition() ;
	RSObject *	pPosObj = context.new_Object( L"Vector2D" ) ;
	pPosObj->SetMemberNumberAs( context, L"x", vPos.x ) ;
	pPosObj->SetMemberNumberAs( context, L"y", vPos.y ) ;
	return	pPosObj ;
}

// Vector2D getZoom()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_getZoom
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	S2DDVector	vZoom = pSprite->GetZoom() ;
	RSObject *	pZoomObj = context.new_Object( L"Vector2D" ) ;
	pZoomObj->SetMemberNumberAs( context, L"x", vZoom.x ) ;
	pZoomObj->SetMemberNumberAs( context, L"y", vZoom.y ) ;
	return	pZoomObj ;
}

// double getRotation()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_getRotation
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	return	context.new_Number( pSprite->GetRotation() ) ;
}

// int getTransparency()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_getTransparency
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( pSprite->GetTransparency() ) ;
}

// int getFilterParameter()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_getFilterParameter
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( pSprite->GetFilterParameter() ) ;
}

// int getFilter2Parameter()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_getFilter2Parameter
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( pSprite->GetFilter2Parameter() ) ;
}

// booleam isVisible()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_isVisible
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	return	context.new_Boolean( pSprite->IsVisible() ) ;
}

// int getPriority()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_getPriority
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( pSprite->GetPriority() ) ;
}

// String getID()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_getID
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	return	context.new_String( pSprite->GetID() ) ;
}

// Rect getRectangle()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_getRectangle
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	SGLRect	rect ;
	if ( !pSprite->GetRectangle( rect ) )
	{
		return	NULL ;
	}
	RSObject *	pObjRect = context.new_Object( L"Rect" ) ;
	pObjRect->SetMemberIntegerAs( context, L"x", rect.left ) ;
	pObjRect->SetMemberIntegerAs( context, L"y", rect.top ) ;
	pObjRect->SetMemberIntegerAs( context, L"w", rect.GetWidth() ) ;
	pObjRect->SetMemberIntegerAs( context, L"h", rect.GetHeight() ) ;
	return	pObjRect ;
}

// void setParameter( Sprite.Parameter param )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_setParameter
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSObject *	pObjParam = arg.ObjectAt(0) ;
	if ( pObjParam != NULL )
	{
		SGLSprite::Parameter	param ;
		SArray<S2DVector>		vertices ;
		ParameterClass::ParameterFromObject
			( context, param, vertices, pObjParam ) ;
		pSprite->SetParameter( param ) ;
	}
	return	NULL ;
}

// void setPosition( double x, double y )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_setPosition
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pSprite->SetPosition( arg.DoubleAt(0), arg.DoubleAt(1) ) ;
	return	NULL ;
}

// void setCenterPosition( double x, double y )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_setCenterPosition
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pSprite->SetCenterPosition( arg.DoubleAt(0), arg.DoubleAt(1) ) ;
	return	NULL ;
}

// void setZoom( double x, double y )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_setZoom
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pSprite->SetZoom( arg.DoubleAt(0), arg.DoubleAt(1) ) ;
	return	NULL ;
}

// void setRotation( double zAngle )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_setRotation
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pSprite->SetRotation( arg.DoubleAt(0) ) ;
	return	NULL ;
}

// void setTransparency( int nTransparency )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_setTransparency
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pSprite->SetTransparency( arg.IntAt(0) ) ;
	return	NULL ;
}

// void setFilterParameter( int paramFilter )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_setFilterParameter
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pSprite->SetFilterParameter( arg.IntAt(0) ) ;
	return	NULL ;
}

// void setFilter2Parameter( int paramFilter )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_setFilter2Parameter
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pSprite->SetFilter2Parameter( arg.IntAt(0) ) ;
	return	NULL ;
}

// boolean regulateCenter
//	( int nFlags = 0, double xOffset = 0.0, double yOffset = 0.0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_regulateCenter
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Boolean
		( pSprite->RegulateCenter
			( (uint32_t) arg.IntAt(0), arg.DoubleAt(1), arg.DoubleAt(2) ) ) ;
}

// void setVisible( boolean visible )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_setVisible
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pSprite->SetVisible( arg.BooleanAt(0) ) ;
	return	NULL ;
}

// void changePriority( int nPriority )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_changePriority
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pSprite->ChangePriority( arg.IntAt(0) ) ;
	return	NULL ;
}

// void setID( String id )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_setID
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pSprite->SetID( arg.StringAt(0) ) ;
	return	NULL ;
}

// void postUpdate()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_postUpdate
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	pSprite->PostUpdate() ;
	return	NULL ;
}

// void freezeFrameUpdate()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_freezeFrameUpdate
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	pSprite->FreezeFrameUpdate() ;
	return	NULL ;
}

// void defrostFrameUpdate()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_defrostFrameUpdate
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	pSprite->DefrostFrameUpdate() ;
	return	NULL ;
}

// void addChild( Sprite sprite )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_addChild
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLSprite *	pChild = ESLTypeCast<SGLSprite>( arg.NativeObjectAt(0) ) ;
	if ( pChild == NULL )
	{
		context.ThrowExceptionError( L"Sprite.addChild の引数が null です" ) ;
	}
	else
	{
		pSprite->AddChild( pChild ) ;
	}
	return	NULL ;
}

// void detachChild( Sprite sprite )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_detachChild
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLSprite *	pChild = ESLTypeCast<SGLSprite>( arg.NativeObjectAt(0) ) ;
	if ( pChild == NULL )
	{
		context.ThrowExceptionError( L"Sprite.detachChild の引数が null です" ) ;
	}
	else
	{
		pSprite->DetachChild( pChild ) ;
	}
	return	NULL ;
}

// void detachAllChildren()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_detachAllChildren
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	pSprite->DetachAllChildren() ;
	return	NULL ;
}

// Sprite getParent()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_getParent
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	SGLSprite *	pParent = pSprite->GetParent() ;
	if ( pParent == NULL )
	{
		return	NULL ;
	}
	return	new RSNativeObject
		( pParent, context.GetClassAs( pParent->GetRSClassName() ) ) ;
}

// Sprite getItemAs( String id )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_getItemAs
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLSprite *	pItem = pSprite->GetItemAs( arg.StringAt(0) ) ;
	if ( pItem == NULL )
	{
		return	NULL ;
	}
	return	new RSNativeObject
		( pItem, context.GetClassAs( pItem->GetRSClassName() ) ) ;
}

// Sprite getHitSpriteAt( double x, double y )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_getHitSpriteAt
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S2DDVector	vPos( arg.DoubleAt(0), arg.DoubleAt(1) ) ;
	SGLSprite *	pItem = pSprite->GetHitSpriteAt( vPos ) ;
	if ( pItem == NULL )
	{
		return	NULL ;
	}
	return	new RSNativeObject
		( pItem, context.GetClassAs( pItem->GetRSClassName() ) ) ;
}

// boolean isHitSprite( double x, double y )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_isHitSprite
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Boolean
			( pSprite->IsHitSprite( arg.DoubleAt(0), arg.DoubleAt(1) ) ) ;
}

// int getChildCount()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_getChildCount
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( pSprite->GetChildCount() ) ;
}

// Sprite getChildAt( int i )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_getChildAt
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLSprite *	pChild = pSprite->GetChildAt( (size_t) arg.IntAt(0) ) ;
	if ( pChild == NULL )
	{
		return	NULL ;
	}
	return	new RSNativeObject
		( pChild, context.GetClassAs( pChild->GetRSClassName() ) ) ;
}

// boolean loadImage( String file )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_loadImage
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Boolean
				( pSprite->LoadImage( arg.StringAt(0) ) == sglErrSuccess ) ;
}

// void attachAnimation( Image image )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_attachAnimation
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLImageObject *	pImage =
			ESLTypeCast<SGLImageObject>( arg.NativeObjectAt(0) ) ;
	pSprite->AttachAnimation( pImage ) ;
	return	NULL ;
}

// void beginAnimation
//	( int loop = -1, int iLoopStart = 0, int iLoopEnd = 0,
//		int iAnimeStart = 0, int msecDuration = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_beginAnimation
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	ssize_t	countLoop = (ssize_t) arg.IntAt( 0, -1 ) ;
	size_t	iLoopStart = (size_t) arg.IntAt( 1, 0 ) ;
	size_t	iLoopEnd = (size_t) arg.IntAt( 2, 0 ) ;
	size_t	iStartFrame = (size_t) arg.IntAt( 3, 0 ) ;
	size_t	msecDuration = (size_t) arg.IntAt( 4, 0 ) ;
	//
	pSprite->BeginAnimation
		( countLoop, iLoopStart, iLoopEnd, iStartFrame, msecDuration ) ;
	//
	return	NULL ;
}

// void setLoopAnimation
//	( int loop = -1, int iLoopStart = 0, int iLoopEnd = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_setLoopAnimation
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	ssize_t	countLoop = (ssize_t) arg.IntAt( 0, -1 ) ;
	size_t	iLoopStart = (size_t) arg.IntAt( 1, 0 ) ;
	size_t	iLoopEnd = (size_t) arg.IntAt( 2, 0 ) ;
	//
	pSprite->SetLoopAnimation( countLoop, iLoopStart, iLoopEnd ) ;
	//
	return	NULL ;
}

// Size getImageSize()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_getImageSize
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSObject *	pObjSize = context.new_Object( L"Size" ) ;
	if ( pObjSize == NULL )
	{
		return	NULL ;
	}
	SGLSize	sizeImage = pSprite->GetImageSize() ;
	pObjSize->SetMemberIntegerAs( context, L"w", sizeImage.w ) ;
	pObjSize->SetMemberIntegerAs( context, L"h", sizeImage.h ) ;
	return	pObjSize ;
}

// boolean createBuffer
//	( int width, int height, int format = Image.formatDefaultRGBA,
//		int depth = 32, long nFlags = Image.bufferOnMemory,
//		boolean flagZBuffer = false, boolean flagStereo3D = false )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_createBuffer
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	uint32_t	width = (uint32_t) arg.IntAt( 0 ) ;
	uint32_t	height = (uint32_t) arg.IntAt( 1 ) ;
	uint32_t	format = (uint32_t) arg.IntAt( 2, formatImageDefaultRGBA ) ;
	uint32_t	depth = (uint32_t) arg.IntAt( 3, 32 ) ;
	uint64_t	nBufFlags = arg.LongAt( 4, SGLImageObject::bufferOnMemory ) ;
	bool		flagZBuffer = arg.BooleanAt( 5, false ) ;
	bool		flagStereo3D = arg.BooleanAt( 6, false ) ;
	//
	SGLError	err =
		pSprite->CreateBuffer
			( width, height, format, depth,
				nBufFlags, flagZBuffer, flagStereo3D ) ;
	//
	return	context.new_Boolean( err == sglErrSuccess ) ;
}

// void releaseBuffer()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_releaseBuffer
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	pSprite->ReleaseBuffer() ;
	return	NULL ;
}

// boolean isBuffered()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_isBuffered
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	return	context.new_Boolean( pSprite->IsBuffered() ) ;
}

// void setFillBackColor( int argbFill, boolean flagFillBack = true )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_setFillBackColor
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pSprite->SetFillBackColor
		( (uint32_t) arg.IntAt(0), arg.BooleanAt(1,true) ) ;
	return	NULL ;
}

// String getText()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_getText
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	return	context.new_String( pSprite->GetText() ) ;
}

// void setText( String text )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_setText
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pSprite->SetText( arg.StringAt(0) ) ;
	return	NULL ;
}

// void setTextFont( String font, int nSize = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_setTextFont
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pSprite->SetTextFont( arg.StringAt(0), arg.IntAt(1, 0) ) ;
	return	NULL ;
}

// boolean isEnabled()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_isEnabled
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	return	context.new_Boolean( pSprite->IsEnabled() ) ;
}

// void setEnable( boolean fEnable )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_setEnable
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pSprite->SetEnable( arg.BooleanAt(0) ) ;
	return	NULL ;
}

// int getScrollPos( int scrlDir = Sprite.scrollDefault )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_getScrollPos
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Integer
		( pSprite->GetScrollPos
			( (SGLSprite::ScrollDirection)
				arg.IntAt( 0, SGLSprite::scrollDefault ) ) ) ;
}

// void setScrollPos( int nPos, int scrlDir = Sprite.scrollDefault )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_setScrollPos
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pSprite->SetScrollPos
		( arg.IntAt(0),
			(SGLSprite::ScrollDirection)
				arg.IntAt( 0, SGLSprite::scrollDefault ) ) ;
	return	NULL ;
}

// int getScrollRange( int scrlDir = Sprite.scrollDefault )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_getScrollRange
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Integer
		( pSprite->GetScrollRange
			( (SGLSprite::ScrollDirection)
				arg.IntAt( 0, SGLSprite::scrollDefault ) ) ) ;
}

// void setScrollRange( int nRange, int scrlDir = Sprite.scrollDefault )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_setScrollRange
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pSprite->SetScrollRange
		( arg.IntAt(0),
			(SGLSprite::ScrollDirection)
				arg.IntAt( 0, SGLSprite::scrollDefault ) ) ;
	return	NULL ;
}

// int getScrollPageSize( int scrlDir = Sprite.scrollDefault )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_getScrollPageSize
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Integer
		( pSprite->GetScrollPageSize
			( (SGLSprite::ScrollDirection)
				arg.IntAt( 0, SGLSprite::scrollDefault ) ) ) ;
}

// void setScrollPageSize( int nPageSize, int scrlDir = Sprite.scrollDefault )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_setScrollPageSize
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pSprite->SetScrollPageSize
		( arg.IntAt(0),
			(SGLSprite::ScrollDirection)
				arg.IntAt( 0, SGLSprite::scrollDefault ) ) ;
	return	NULL ;
}

// boolean isButtonChecked()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_isButtonChecked
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	return	context.new_Boolean( pSprite->IsButtonChecked() ) ;
}

// void checkButton( boolean check )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_checkButton
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pSprite->CheckButton( arg.BooleanAt( 0 ) ) ;
	return	NULL ;
}

// void invokeCommands( String xmlCommands )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_invokeCommands
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pSprite->InvokeCommands( arg.StringAt(0) ) ;
	return	NULL ;
}

// boolean isSpriteVisible( String id )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_isSpriteVisible
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Boolean
		( pSprite->IsSpriteVisible( arg.StringAt(0) ) ) ;
}

// void setSpriteVisible( String id, boolean visible )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_setSpriteVisible
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pSprite->SetSpriteVisible( arg.StringAt(0), arg.BooleanAt(1) ) ;
	return	NULL ;
}

// int getSpritePriority( String id )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_getSpritePriority
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Integer
		( pSprite->GetSpritePriority( arg.StringAt(0) ) ) ;
}

// void changeSpritePriority( String id, int priority )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_changeSpritePriority
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pSprite->ChangeSpritePriority( arg.StringAt(0), (int32_t) arg.IntAt(1) ) ;
	return	NULL ;
}

// int getSpriteTransparency( String id )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_getSpriteTransparency
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Integer
		( pSprite->GetSpriteTransparency( arg.StringAt(0) ) ) ;
}

// void setSpriteTransparency( String id, int transparency )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_setSpriteTransparency
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pSprite->SetSpriteTransparency( arg.StringAt(0), (uint32_t) arg.IntAt(1) ) ;
	return	NULL ;
}

// Rect getSpriteRectangle( String id )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_getSpriteRectangle
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLRect	rect ;
	if ( !pSprite->GetSpriteRectangle( arg.StringAt(0), rect ) )
	{
		return	NULL ;
	}
	RSObject *	pObjRect = context.new_Object( L"Rect" ) ;
	pObjRect->SetMemberIntegerAs( context, L"x", rect.left ) ;
	pObjRect->SetMemberIntegerAs( context, L"y", rect.top ) ;
	pObjRect->SetMemberIntegerAs( context, L"w", rect.GetWidth() ) ;
	pObjRect->SetMemberIntegerAs( context, L"h", rect.GetHeight() ) ;
	return	pObjRect ;
}

// String getSpriteText( String id )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_getSpriteText
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_String
		( pSprite->GetSpriteText( arg.StringAt(0) ) ) ;
}

// void setSpriteText( String id, String text )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_setSpriteText
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pSprite->SetSpriteText( arg.StringAt(0), arg.StringAt(1) ) ;
	return	NULL ;
}

// void setSpriteTextFont( String id, String font, int nSize = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_setSpriteTextFont
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pSprite->SetSpriteTextFont( arg.StringAt(0), arg.StringAt(1) ) ;
	return	NULL ;
}

// void setSpriteImage( String id, String image )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_setSpriteImage
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pSprite->SetSpriteImage( arg.StringAt(0), arg.StringAt(1) ) ;
	return	NULL ;
}

// boolean isSpriteEnabled( String id )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_isSpriteEnabled
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Boolean
		( pSprite->IsSpriteEnabled( arg.StringAt(0) ) ) ;
}

// void setSpriteEnable( String id, boolean fEnable )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_setSpriteEnable
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pSprite->SetSpriteEnable( arg.StringAt(0), arg.BooleanAt(1) ) ;
	return	NULL ;
}

// int getSpriteScrollPos( String id, int scrlDir = Sprite.scrollDefault )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_getSpriteScrollPos
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Integer
		( pSprite->GetSpriteScrollPos
			( arg.StringAt(0),
				(SGLSprite::ScrollDirection)
					arg.IntAt(1, SGLSprite::scrollDefault ) ) ) ;
}

// void setSpriteScrollPos( String id, int nPos, int scrlDir = Sprite.scrollDefault )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_setSpriteScrollPos
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pSprite->SetSpriteScrollPos
		( arg.StringAt(0), arg.IntAt(1),
			(SGLSprite::ScrollDirection)
				arg.IntAt(2, SGLSprite::scrollDefault) ) ;
	return	NULL ;
}

// int getSpriteScrollRange( String id, int scrlDir = Sprite.scrollDefault )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_getSpriteScrollRange
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Integer
		( pSprite->GetSpriteScrollRange
			( arg.StringAt(0),
				(SGLSprite::ScrollDirection)
					arg.IntAt(1, SGLSprite::scrollDefault ) ) ) ;
}

// void setSpriteScrollRange( String id, int nRange, int scrlDir = Sprite.scrollDefault )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_setSpriteScrollRange
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pSprite->SetSpriteScrollRange
		( arg.StringAt(0), arg.IntAt(1),
			(SGLSprite::ScrollDirection)
				arg.IntAt(2, SGLSprite::scrollDefault) ) ;
	return	NULL ;
}

// int getSpriteScrollPageSize( String id, int scrlDir = Sprite.scrollDefault )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_getSpriteScrollPageSize
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Integer
		( pSprite->GetSpriteScrollPageSize
			( arg.StringAt(0),
				(SGLSprite::ScrollDirection)
					arg.IntAt(1, SGLSprite::scrollDefault ) ) ) ;
}

// void setSpriteScrollPageSize( String id, int nPageSize, int scrlDir = Sprite.scrollDefault )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_setSpriteScrollPageSize
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pSprite->SetSpriteScrollPageSize
		( arg.StringAt(0), arg.IntAt(1),
			(SGLSprite::ScrollDirection)
				arg.IntAt(2, SGLSprite::scrollDefault) ) ;
	return	NULL ;
}

// boolean isSpriteButtonChecked( String id )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_isSpriteButtonChecked
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Boolean
		( pSprite->IsSpriteButtonChecked( arg.StringAt(0) ) ) ;
}

// void checkSpriteButton( String id, boolean check )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_checkSpriteButton
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pSprite->CheckSpriteButton( arg.StringAt(0), arg.BooleanAt(1) ) ;
	return	NULL ;
}

// SpriteAction setActionLinearTo
//		( int msecDuration, int nTransparency,
//			Vector2D vPos = null, Vector2D vZoom = null,
//			double a0 = 0.0, double a1 = 0.0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_setActionLinearTo
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S2DDVector		vPos, vZoom ;
	S2DDVector *	pPos = NULL ;
	S2DDVector *	pZoom = NULL ;
	uint32_t	msecDuration = (uint32_t) arg.IntAt( 0 ) ;
	uint32_t	nTransparency = (uint32_t) arg.IntAt( 1 ) ;
	RSObject *	pPosObj = arg.ObjectAt( 2 ) ;
	if ( pPosObj != NULL )
	{
		pPos = &vPos ;
		vPos.x = pPosObj->GetMemberNumberAs( context, L"x" ) ;
		vPos.y = pPosObj->GetMemberNumberAs( context, L"y" ) ;
	}
	RSObject *	pZoomObj = arg.ObjectAt( 3 ) ;
	if ( pZoomObj != NULL )
	{
		pZoom = &vZoom ;
		vZoom.x = pZoomObj->GetMemberNumberAs( context, L"x" ) ;
		vZoom.y = pZoomObj->GetMemberNumberAs( context, L"y" ) ;
	}
	double	a0 = arg.DoubleAt( 4, 0.0 ) ;
	double	a1 = arg.DoubleAt( 5, 0.0 ) ;
	//
	SGLSpriteAction *	pAct =
		pSprite->SetActionLinearTo
			( msecDuration, nTransparency, pPos, pZoom, a0, a1 ) ;
	if ( pAct == NULL )
	{
		return	NULL ;
	}
	return	new RSNativeObject( pAct, context.GetClassAs( L"SpriteAction" ) ) ;
}

// void addAction( SpriteAction act )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_addAction
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSNativeObject *
		pNObjAct = ESLTypeCast<RSNativeObject>( arg.ObjectAt(0) ) ;
	if ( pNObjAct != NULL )
	{
		SGLSprite::Action *	pAct =
			ESLTypeCast<SGLSprite::Action>( pNObjAct->GetObject() ) ;
		if ( pAct != NULL )
		{
			pAct = ESLSmartCast<SGLSprite::Action>
								( pNObjAct->DetachObject() ) ;
			if ( pAct != NULL )
			{
				pNObjAct->AttachObject( pAct ) ;
				pSprite->AddAction( pAct ) ;
			}
		}
	}
	return	NULL ;
}

// void flushAction()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_flushAction
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	pSprite->FlushAction() ;
	return	NULL ;
}

// boolean isAction()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_isAction
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	return	context.new_Boolean( pSprite->IsAction() ) ;
}

// void pauseAllAction()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_pauseAllAction
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	pSprite->PauseAllAction() ;
	return	NULL ;
}

// void restartAllAction()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_restartAllAction
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	pSprite->RestartAllAction() ;
	return	NULL ;
}

// void addTimer( SpriteTimer timer )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_addTimer
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSSpriteTimer *	pObjTimer = ESLTypeCast<RSSpriteTimer>( arg.ObjectAt(0) ) ;
	if ( pObjTimer != NULL )
	{
		pSprite->AddReferenceTimer( pObjTimer ) ;
	}
	return	NULL ;
}

// void removeTimer( SpriteTimer timer )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_removeTimer
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSSpriteTimer *	pObjTimer = ESLTypeCast<RSSpriteTimer>( arg.ObjectAt(0) ) ;
	if ( pObjTimer != NULL )
	{
		pSprite->RemoveTimer( pObjTimer ) ;
	}
	return	NULL ;
}

// void attachMouseListener( SpriteMouseListener listener )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_attachMouseListener
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSSpriteMouseListener *
		pObjListener = ESLTypeCast<RSSpriteMouseListener>( arg.ObjectAt(0) ) ;
	if ( pObjListener != NULL )
	{
		pSprite->AttachMouseListener( pObjListener ) ;
	}
	return	NULL ;
}

// void detachMouseListener( SpriteMouseListener listener )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_detachMouseListener
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSSpriteMouseListener *
		pObjListener = ESLTypeCast<RSSpriteMouseListener>( arg.ObjectAt(0) ) ;
	if ( pObjListener != NULL )
	{
		pSprite->DetachMouseListener( pObjListener ) ;
	}
	return	NULL ;
}

// void attachKeyListener( SpriteKeyListener listener )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_attachKeyListener
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSSpriteKeyListener *
		pObjListener = ESLTypeCast<RSSpriteKeyListener>( arg.ObjectAt(0) ) ;
	if ( pObjListener != NULL )
	{
		pSprite->AttachKeyListener( pObjListener ) ;
	}
	return	NULL ;
}

// void detachKeyListener( SpriteKeyListener listener )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_detachKeyListener
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSSpriteKeyListener *
		pObjListener = ESLTypeCast<RSSpriteKeyListener>( arg.ObjectAt(0) ) ;
	if ( pObjListener != NULL )
	{
		pSprite->DetachKeyListener( pObjListener ) ;
	}
	return	NULL ;
}

// void setMouseCapture()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_setMouseCapture
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	pSprite->SetMouseCapture() ;
	return	NULL ;
}

// void releaseMouseCapture()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_releaseMouseCapture
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	pSprite->ReleaseMouseCapture() ;
	return	NULL ;
}

// void setKeyFocus()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_setKeyFocus
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	pSprite->SetKeyFocus() ;
	return	NULL ;
}

// void killKeyFocus()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_killKeyFocus
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	pSprite->KillKeyFocus() ;
	return	NULL ;
}

// boolean hasKeyFocus()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteClass::method_hasKeyFocus
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite == NULL )
	{
		return	NULL ;
	}
	return	context.new_Boolean( pSprite->HasKeyFocus() ) ;
}


//////////////////////////////////////////////////////////////////////////////
// SpriteAction クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSSpriteActionClass, RGenericNativeObjectClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSSpriteActionClass::RSSpriteActionClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RGenericNativeObjectClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSSpriteActionClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( NULL, this ) ;
	//
	CreateMemberIntegerAs
		( context, L"actionOnce", SGLSprite::actionOnce, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"actionLoop", SGLSprite::actionLoop, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"actionTurn", SGLSprite::actionTurn, modifierConst ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"",
				NULL, &RSSpriteActionClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setActionType", NULL, L"int type",
				NULL, &RSSpriteActionClass::method_setActionType, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setDuration", NULL,
			L"int msecDuration, int msecDelay = 0",
				NULL, &RSSpriteActionClass::method_setDuration, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setMoveTo", NULL,
				L"Sprite sprite, double x, double y, "
				L"double a0 = 0.0, double a1 = 0.0",
				NULL, &RSSpriteActionClass::method_setMoveTo, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setZoomTo", NULL,
				L"Sprite sprite, double x, double y, "
				L"double a0 = 0.0, double a1 = 0.0",
				NULL, &RSSpriteActionClass::method_setZoomTo, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setRotationTo", NULL,
				L"Sprite sprite, double z, "
				L"double a0 = 0.0, double a1 = 0.0",
				NULL, &RSSpriteActionClass::method_setRotationTo, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setTransparencyTo", NULL,
				L"Sprite sprite, int nTransparency",
				NULL, &RSSpriteActionClass::method_setTransparencyTo, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setFilterTo", NULL,
				L"Sprite sprite, int paramFilter",
				NULL, &RSSpriteActionClass::method_setFilterTo, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setFilter2To", NULL,
				L"Sprite sprite, int paramFilter",
				NULL, &RSSpriteActionClass::method_setFilter2To, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setBezierCurve", NULL,
				L"Vector2D bzCurve, boolean fOffset = false",
				NULL, &RSSpriteActionClass::method_setBezierCurve, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setCenterCurve", NULL,
				L"Vector2D bzCurve, boolean fOffset = false",
				NULL, &RSSpriteActionClass::method_setCenterCurve, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setZoomCurve", NULL,
				L"Vector2D bzCurve, boolean fOffset = false",
				NULL, &RSSpriteActionClass::method_setZoomCurve, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setAngleCurve", NULL,
				L"Float32Pointer bzCurve, boolean fOffset = false",
				NULL, &RSSpriteActionClass::method_setAngleCurve, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setTransparencyCurve", NULL,
				L"Float32Pointer bzCurve, boolean fOffset = false",
				NULL, &RSSpriteActionClass::method_setTransparencyCurve, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setFilterParamCurve", NULL,
				L"Float32Pointer bzCurve, boolean fOffset = false",
				NULL, &RSSpriteActionClass::method_setFilterParamCurve, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setFilter2ParamCurve", NULL,
				L"Float32Pointer bzCurve, boolean fOffset = false",
				NULL, &RSSpriteActionClass::method_setFilter2ParamCurve, NULL ) ;
}

// ネイティブ型テスト
//////////////////////////////////////////////////////////////////////////////
bool RSSpriteActionClass::IsNativeObjectOf( ESLObject * pObj ) const
{
	return	(ESLTypeCast<SGLSpriteAction>(pObj) != nullptr) ;
}

// this オブジェクトを取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLSpriteAction *
	RSSpriteActionClass::GetThisSpriteAction
			( RSContext& context, RSObject* pThis )
{
	SGLSpriteAction *	pAct = NULL ;
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj != NULL )
	{
		pAct = ESLTypeCast<SGLSpriteAction>( pNativeObj->GetObject() ) ;
	}
	if ( pAct == NULL )
	{
		context.ThrowExceptionError( L"this が SpriteAction ではありません" ) ;
	}
	return	pAct ;
}

// void <init>()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteActionClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj == NULL )
	{
		context.ThrowExceptionError
			( L"Sprite.<init> の this が Sprite ではありません" ) ;
		return	NULL ;
	}
	pNativeObj->SetObject( new SGLSpriteAction ) ;
	return	NULL ;
}

// void setActionType( int type ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteActionClass::method_setActionType
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteAction *	pAct = GetThisSpriteAction( context, pThis ) ;
	if ( pAct == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pAct->SetActionType( arg.IntAt(0) ) ;
	return	nullptr ;
}

// void setDuration( int msecDuration, int msecDelay = 0 ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteActionClass::method_setDuration
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteAction *	pAct = GetThisSpriteAction( context, pThis ) ;
	if ( pAct == nullptr )
	{
		return	nullptr ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pAct->SetDuration( arg.IntAt(0), arg.IntAt(1) ) ;
	return	nullptr ;
}

// void setMoveTo
//	( Sprite sprite, double x, double y, double a0 = 0.0, double a1 = 0.0 ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteActionClass::method_setMoveTo
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteAction *	pAct = GetThisSpriteAction( context, pThis ) ;
	if ( pAct == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLSprite *	pSprite = ESLTypeCast<SGLSprite>( arg.NativeObjectAt(0) ) ;
	if ( pSprite == NULL )
	{
		context.ThrowExceptionError
			( L"SpriteAction.setMoveTo の引数が null です" ) ;
		return	NULL ;
	}
	pAct->SetMoveTo
		( *pSprite, arg.DoubleAt(1), arg.DoubleAt(2),
			arg.DoubleAt(3, 0.0), arg.DoubleAt(4, 0.0) ) ;
	return	NULL ;
}

// void setZoomTo
//	( Sprite sprite, double x, double y, double a0 = 0.0, double a1 = 0.0 ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteActionClass::method_setZoomTo
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteAction *	pAct = GetThisSpriteAction( context, pThis ) ;
	if ( pAct == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLSprite *	pSprite = ESLTypeCast<SGLSprite>( arg.NativeObjectAt(0) ) ;
	if ( pSprite == NULL )
	{
		context.ThrowExceptionError
			( L"SpriteAction.setZoomTo の引数が null です" ) ;
		return	NULL ;
	}
	pAct->SetZoomTo
		( *pSprite, arg.DoubleAt(1), arg.DoubleAt(2),
			arg.DoubleAt(3, 0.0), arg.DoubleAt(4, 0.0) ) ;
	return	NULL ;
}

// void setRotationTo
//	( Sprite sprite, double z, double a0 = 0.0, double a1 = 0.0 ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteActionClass::method_setRotationTo
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteAction *	pAct = GetThisSpriteAction( context, pThis ) ;
	if ( pAct == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLSprite *	pSprite = ESLTypeCast<SGLSprite>( arg.NativeObjectAt(0) ) ;
	if ( pSprite == NULL )
	{
		context.ThrowExceptionError
			( L"SpriteAction.setRotationTo の引数が null です" ) ;
		return	NULL ;
	}
	pAct->SetRotationTo
		( *pSprite, arg.DoubleAt(1),
			arg.DoubleAt(2, 0.0), arg.DoubleAt(3, 0.0) ) ;
	return	NULL ;
}

// void setTransparencyTo( Sprite sprite, int nTransparency ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteActionClass::method_setTransparencyTo
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteAction *	pAct = GetThisSpriteAction( context, pThis ) ;
	if ( pAct == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLSprite *	pSprite = ESLTypeCast<SGLSprite>( arg.NativeObjectAt(0) ) ;
	if ( pSprite == NULL )
	{
		context.ThrowExceptionError
			( L"SpriteAction.setTransparencyTo の引数が null です" ) ;
		return	NULL ;
	}
	pAct->SetTransparencyTo( *pSprite, (uint32_t) arg.IntAt(1) ) ;
	return	NULL ;
}

// void setFilterTo( Sprite sprite, int paramFilter ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteActionClass::method_setFilterTo
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteAction *	pAct = GetThisSpriteAction( context, pThis ) ;
	if ( pAct == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLSprite *	pSprite = ESLTypeCast<SGLSprite>( arg.NativeObjectAt(0) ) ;
	if ( pSprite == NULL )
	{
		context.ThrowExceptionError
			( L"SpriteAction.setFilterTo の引数が null です" ) ;
		return	NULL ;
	}
	pAct->SetFilterTo( *pSprite, (uint32_t) arg.IntAt(1) ) ;
	return	NULL ;
}

// void setFilter2To( Sprite sprite, int paramFilter ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteActionClass::method_setFilter2To
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteAction *	pAct = GetThisSpriteAction( context, pThis ) ;
	if ( pAct == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLSprite *	pSprite = ESLTypeCast<SGLSprite>( arg.NativeObjectAt(0) ) ;
	if ( pSprite == NULL )
	{
		context.ThrowExceptionError
			( L"SpriteAction.setFilterTo の引数が null です" ) ;
		return	NULL ;
	}
	pAct->SetFilter2To( *pSprite, (uint32_t) arg.IntAt(1) ) ;
	return	NULL ;
}

// void setBezierCurve( Vector2D bzCurve, boolean fOffset = false ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteActionClass::method_setBezierCurve
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteAction *	pAct = GetThisSpriteAction( context, pThis ) ;
	if ( pAct == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	size_t		nCount ;
	S2DVector *	pvCurve = (S2DVector*) arg.PointerAt( 0, &nCount ) ;
	nCount /= sizeof(S2DVector) ;
	//
	SArray<S3DDVector>	bzCurve ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		S3DDVector	v ;
		v.x = pvCurve[i].x ;
		v.y = pvCurve[i].y ;
		v.z = 0 ;
		bzCurve.Add( v ) ;
	}
	pAct->SetBezierCurve( bzCurve, arg.BooleanAt(1, false) ) ;
	return	NULL ;
}

// void setCenterCurve( Vector2D bzCurve, boolean fOffset = false ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteActionClass::method_setCenterCurve
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteAction *	pAct = GetThisSpriteAction( context, pThis ) ;
	if ( pAct == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	size_t		nCount ;
	S2DVector *	pvCurve = (S2DVector*) arg.PointerAt( 0, &nCount ) ;
	nCount /= sizeof(S2DVector) ;
	//
	SArray<S2DDVector>	bzCurve ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		S2DDVector	v = pvCurve[i] ;
		bzCurve.Add( v ) ;
	}
	pAct->SetCenterCurve( bzCurve, arg.BooleanAt(1, false) ) ;
	return	NULL ;
}

// void setZoomCurve( Vector2D bzCurve, boolean fOffset = false ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteActionClass::method_setZoomCurve
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteAction *	pAct = GetThisSpriteAction( context, pThis ) ;
	if ( pAct == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	size_t		nCount ;
	S2DVector *	pvCurve = (S2DVector*) arg.PointerAt( 0, &nCount ) ;
	nCount /= sizeof(S2DVector) ;
	//
	SArray<S2DDVector>	bzCurve ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		S2DDVector	v = pvCurve[i] ;
		bzCurve.Add( v ) ;
	}
	pAct->SetZoomCurve( bzCurve, arg.BooleanAt(1, false) ) ;
	return	NULL ;
}

// void setAngleCurve( Float32Pointer bzCurve, boolean fOffset = false ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteActionClass::method_setAngleCurve
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteAction *	pAct = GetThisSpriteAction( context, pThis ) ;
	if ( pAct == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	size_t		nCount ;
	float32_t *	pCurve = (float32_t*) arg.PointerAt( 0, &nCount ) ;
	nCount /= sizeof(float32_t) ;
	//
	SArray<double>	bzCurve ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		bzCurve.Add( pCurve[i] ) ;
	}
	pAct->SetAngleCurve( bzCurve, arg.BooleanAt(1, false) ) ;
	return	NULL ;
}

// void setTransparencyCurve( Float32Pointer bzCurve, boolean fOffset = false ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteActionClass::method_setTransparencyCurve
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteAction *	pAct = GetThisSpriteAction( context, pThis ) ;
	if ( pAct == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	size_t		nCount ;
	float32_t *	pCurve = (float32_t*) arg.PointerAt( 0, &nCount ) ;
	nCount /= sizeof(float32_t) ;
	//
	SArray<double>	bzCurve ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		bzCurve.Add( pCurve[i] ) ;
	}
	pAct->SetTransparencyCurve( bzCurve, arg.BooleanAt(1, false) ) ;
	return	NULL ;
}

// void setFilterParamCurve( Float32Pointer bzCurve, boolean fOffset = false ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteActionClass::method_setFilterParamCurve
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteAction *	pAct = GetThisSpriteAction( context, pThis ) ;
	if ( pAct == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	size_t		nCount ;
	float32_t *	pCurve = (float32_t*) arg.PointerAt( 0, &nCount ) ;
	nCount /= sizeof(float32_t) ;
	//
	SArray<double>	bzCurve ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		bzCurve.Add( pCurve[i] ) ;
	}
	pAct->SetFilterParamCurve( bzCurve, arg.BooleanAt(1, false) ) ;
	return	NULL ;
}

// void setFilter2ParamCurve( Float32Pointer bzCurve, boolean fOffset = false ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSSpriteActionClass::method_setFilter2ParamCurve
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteAction *	pAct = GetThisSpriteAction( context, pThis ) ;
	if ( pAct == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	size_t		nCount ;
	float32_t *	pCurve = (float32_t*) arg.PointerAt( 0, &nCount ) ;
	nCount /= sizeof(float32_t) ;
	//
	SArray<double>	bzCurve ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		bzCurve.Add( pCurve[i] ) ;
	}
	pAct->SetFilter2ParamCurve( bzCurve, arg.BooleanAt(1, false) ) ;
	return	NULL ;
}


//////////////////////////////////////////////////////////////////////////////
// WindowSprite.UpdateParameter クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( Rosetta::RSWindowSpriteClass::UpdateParameterClass, RSStructuredPointerClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSWindowSpriteClass::UpdateParameterClass::UpdateParameterClass
	( RSClass * pClass, const wchar_t * pwszClassName )
	: RSStructuredPointerClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSWindowSpriteClass::UpdateParameterClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSStructuredPointer( this, NULL ) ;
	//
	AddArrayMemberAs
		( context, L"flagsUpdate",
			context.GetBasicTypeClass(RSCodeControl::wiLong) ) ;
	AddArrayMemberAs
		( context, L"framesPerSec",
			context.GetBasicTypeClass(RSCodeControl::wiInt) ) ;
	AddArrayMemberAs
		( context, L"framesPast",
			context.GetBasicTypeClass(RSCodeControl::wiInt) ) ;
	AddArrayMemberAs
		( context, L"msecLastUpdate",
			context.GetBasicTypeClass(RSCodeControl::wiLong) ) ;
	AddArrayMemberAs
		( context, L"msecRendering",
			context.GetBasicTypeClass(RSCodeControl::wiInt) ) ;
	AddArrayMemberAs
		( context, L"msecFrameError",
			context.GetBasicTypeClass(RSCodeControl::wiInt) ) ;
}


//////////////////////////////////////////////////////////////////////////////
// WindowSprite.Stereo3D クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSWindowSpriteClass::Stereo3DClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSWindowSpriteClass::Stereo3DClass::Stereo3DClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSWindowSpriteClass::Stereo3DClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSGenericObject( this ) ;
	//
	CreateMemberStringAs
		( context, L"AnaglyphView", Window::Stereo3D::AnaglyphView, modifierConst ) ;
	CreateMemberStringAs
		( context, L"DDStereoscopic", Window::Stereo3D::DDStereoscopic, modifierConst ) ;
	CreateMemberStringAs
		( context, L"OpenGLQuadBuffer", Window::Stereo3D::OpenGLQuadBuffer, modifierConst ) ;
	CreateMemberStringAs
		( context, L"NVStereoBLT", Window::Stereo3D::NVStereoBLT, modifierConst ) ;
	CreateMemberStringAs
		( context, L"InterleavedView", Window::Stereo3D::InterleavedView, modifierConst ) ;
	CreateMemberStringAs
		( context, L"MonoView", Window::Stereo3D::MonoView, modifierConst ) ;
}



//////////////////////////////////////////////////////////////////////////////
// WindowSprite クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSWindowSpriteClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSWindowSpriteClass::RSWindowSpriteClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// メンバ初期設定
//////////////////////////////////////////////////////////////////////////////
void RSWindowSpriteClass::Initialize( RSContext& context )
{
	AddSuperClass( context, context.GetClassAs( L"Sprite" ) ) ;
	AddImplementClass( context, context.GetClassAs( L"Window" ) ) ;
	OverrideVirtuals( context ) ;
	m_flagInitialized = true ;
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSWindowSpriteClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( NULL, this ) ;
	//
	UpdateParameterClass *
		pUpdateClass = new UpdateParameterClass
							( context.GetClassClass(), L"UpdateParameter" ) ;
	pUpdateClass->Initialize( context ) ;
	pUpdateClass->FinishClass( context ) ;
	context.ReleaseObjectRef
		( CreateMemberAs( context, L"UpdateParameter", pUpdateClass ) ) ;
	//
	Stereo3DClass *	pStereo3DClass =
		new Stereo3DClass( context.GetClassClass(), L"Stereo3D" ) ;
	pStereo3DClass->Initialize( context ) ;
	pStereo3DClass->FinishClass( context ) ;
	context.ReleaseObjectRef
		( CreateMemberAs( context, L"Stereo3D", pStereo3DClass ) ) ;
	//
	CreateMemberIntegerAs
		( context, L"modeWindow", Window::modeWindow, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"modeNormal", Window::modeNormal, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"modeFullScreen", Window::modeFullScreen, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"modeExclusive", Window::modeExclusive, modifierConst ) ;
	//
	CreateMemberIntegerAs
		( context, L"flagUseDblClick", Window::flagUseDblClick, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"flagAllowClose", Window::flagAllowClose, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"flagBlackBack", Window::flagBlackBack, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"flagEnableIME", Window::flagEnableIME, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"flagAllowMinimize", Window::flagAllowMinimize, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"flagGrantScreenSave", Window::flagGrantScreenSave, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"flagGrantMonitorSave", Window::flagGrantMonitorSave, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"flagGrantPowerSuspend", Window::flagGrantPowerSuspend, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"flagVariableWindowSize", Window::flagVariableWindowSize, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"flagAllowMaximize", Window::flagAllowMaximize, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"flagChildWindow", Window::flagChildWindow, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"flagPopupWindow", Window::flagPopupWindow, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"flagInvisibleWindow", Window::flagInvisibleWindow, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"flagOpenIME", Window::flagOpenIME, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"flagDoMinimize", Window::flagDoMinimize, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"flagDoMaximize", Window::flagDoMaximize, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"flagDoNormalize", Window::flagDoNormalize, modifierConst ) ;
	//
	CreateMemberIntegerAs
		( context, L"flagLayeredWindow", Window::flagLayeredWindow, modifierConst ) ;
	//
	CreateMemberIntegerAs
		( context, L"layoutNothing", Window::layoutNothing, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"layoutOffsetClient", Window::layoutOffsetClient, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"layoutDockingLeft", Window::layoutDockingLeft, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"layoutDockingRight", Window::layoutDockingRight, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"layoutDockingUpper", Window::layoutDockingUpper, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"layoutDockingUnder", Window::layoutDockingUnder, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"layoutDockingMask", Window::layoutDockingMask, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"layoutAlignLeft", Window::layoutAlignLeft, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"layoutAlignTop", Window::layoutAlignTop, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"layoutAlignCenter", Window::layoutAlignCenter, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"layoutAlignRight", Window::layoutAlignRight, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"layoutAlignBottom", Window::layoutAlignBottom, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"layoutAlignAccording", Window::layoutAlignAccording, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"layoutAlignMask", Window::layoutAlignMask, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"layoutTypeClient", Window::layoutTypeClient, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"layoutTypeWindow", Window::layoutTypeWindow, modifierConst ) ;
	//
	CreateMemberIntegerAs
		( context, L"exteriorFillColor", Window::exteriorFillColor, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"exteriorStretch", Window::exteriorStretch, modifierConst ) ;
	//
	CreateMemberIntegerAs
		( context, L"updateVSync", Window::updateVSync, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"updateWaitFrames", Window::updateWaitFrames, modifierConst ) ;
	//
	CreateMemberIntegerAs
		( context, L"postNormal", Window::postNormal, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"postDelay", Window::postDelay, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"postAsyncNoRender", Window::postAsyncNoRender, modifierConst ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"",
				NULL, &RSWindowSpriteClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"createDisplay", L"boolean",
			L"String name, int mode, "
			L"int width, int height, int bpp = 0, int frequency = 0",
			NULL, &RSWindowSpriteClass::method_createDisplay, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"closeDisplay", L"boolean", L"",
			NULL, &RSWindowSpriteClass::method_closeDisplay, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getOptionalFlags", L"long", L"",
			NULL, &RSWindowSpriteClass::method_getOptionalFlags,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setOptionalFlags", NULL, L"long nFlags",
			NULL, &RSWindowSpriteClass::method_setOptionalFlags, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"changeCooperationLevel", L"boolean", L"int mode",
			NULL, &RSWindowSpriteClass::method_changeCooperationLevel, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"changeDisplaySize", L"boolean",
			L"int width, int height, int bpp = 0, int frequency = 0",
			NULL, &RSWindowSpriteClass::method_changeDisplaySize, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getDisplaySize", L"boolean", L"Size sizeDisplay",
			NULL, &RSWindowSpriteClass::method_getDisplaySize, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"enableChangePhysicalMode", L"boolean", L"boolean flagEnable",
			NULL, &RSWindowSpriteClass::method_enableChangePhysicalMode, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"enableZBuffer", L"boolean", L"boolean flagZBuffer",
			NULL, &RSWindowSpriteClass::method_enableZBuffer, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setStereoDisplayMode",
			L"boolean", L"String sMethodID, long nParam = 0",
			NULL, &RSWindowSpriteClass::method_setStereoDisplayMode, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"isSupportedStereoDisplayMode",
			L"boolean", L"String sMethodID",
			NULL, &RSWindowSpriteClass::method_isSupportedStereoDisplayMode, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"initWindowPosition",
			L"boolean", L"int xPos, int yPos, Size pInitExSize = null",
			NULL, &RSWindowSpriteClass::method_initWindowPosition, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getNormalWindowPosition",
			L"boolean", L"Point ptWindow, Size sizeWindow = null",
			NULL, &RSWindowSpriteClass::method_getNormalWindowPosition, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getInternalDisplayPosition",
			L"boolean", L"Rect rectRender, Rect rectDisplay",
			NULL, &RSWindowSpriteClass::method_getInternalDisplayPosition, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setExteriorBackgroundFrame",
			L"boolean", L"int nFlags, int rgbColor, Image pTile, "
						L"Image pLeft = null, Image pRight = null,"
						L"Image pUpper = null, Image pUnder = null",
			NULL, &RSWindowSpriteClass::method_setExteriorBackgroundFrame, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"createWindow",
			L"boolean", L"String name, int width, int height, "
						L"int flags, WindowSprite parent = null",
			NULL, &RSWindowSpriteClass::method_createWindow, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"closeWindow", L"boolean", L"",
			NULL, &RSWindowSpriteClass::method_closeWindow, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setWindowLayout",
			L"boolean", L"int nFlags, int xPos = 0, int yPos = 0",
			NULL, &RSWindowSpriteClass::method_setWindowLayout, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"changeWindowSize",
			L"boolean", L"int width, int height",
			NULL, &RSWindowSpriteClass::method_changeWindowSize, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getWindowClientRect",
			L"boolean", L"Rect rectClient",
			NULL, &RSWindowSpriteClass::method_getWindowClientRect, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"screenPositionFromClient",
			L"Vector2D", L"Vector2D vClient",
			NULL, &RSWindowSpriteClass::method_screenPositionFromClient, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"clientPositionFromScreen",
			L"Vector2D", L"Vector2D vScreen",
			NULL, &RSWindowSpriteClass::method_clientPositionFromScreen, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"postUpdate",
			NULL, L"Rect pUpdate = null",
			NULL, &RSWindowSpriteClass::method_postUpdate, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"updateWindow",
			L"boolean", L"WindowSprite.UpdateParameter pUpdate = null",
			NULL, &RSWindowSpriteClass::method_updateWindow, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"processUserInput",
			L"boolean", L"long msecTimeout = 1",
			NULL, &RSWindowSpriteClass::method_processUserInput, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"postRenderingThread",
			L"boolean", L"Runnable proc, int postType = 0",
			NULL, &RSWindowSpriteClass::method_postRenderingThread, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"postUIThread",
			L"boolean", L"Runnable proc",
			NULL, &RSWindowSpriteClass::method_postUIThread, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"isWindowActive", L"boolean", L"",
			NULL, &RSWindowSpriteClass::method_isWindowActive, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setWindowCaption", L"boolean", L"String name",
			NULL, &RSWindowSpriteClass::method_setWindowCaption, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"showCursor", L"boolean", L"boolean fShow",
			NULL, &RSWindowSpriteClass::method_showCursor, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"isShowCursor", L"boolean", L"",
			NULL, &RSWindowSpriteClass::method_isShowCursor, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setCursor", L"boolean", L"String sCursorID",
			NULL, &RSWindowSpriteClass::method_setCursor, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"moveCursorPosition",
			L"boolean", L"int xPos, int yPos, int idMouse = 0",
			NULL, &RSWindowSpriteClass::method_moveCursorPosition, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getCursorPosition",
			L"boolean", L"Point ptCursor, int idMouse = 0",
			NULL, &RSWindowSpriteClass::method_getCursorPosition, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getMonitorFrequency", L"int", L"",
			NULL, &RSWindowSpriteClass::method_getMonitorFrequency, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"lock", NULL, L"",
				NULL, &RSWindowSpriteClass::method_lock, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"unlock", NULL, L"",
				NULL, &RSWindowSpriteClass::method_unlock, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"testLocked", L"long", L"",
				NULL, &RSWindowSpriteClass::method_testLocked, NULL ) ;
}

// ネイティブ型テスト
//////////////////////////////////////////////////////////////////////////////
bool RSWindowSpriteClass::IsNativeObjectOf( ESLObject * pObj ) const
{
	return	(ESLTypeCast<SGLWindowSprite>(pObj) != nullptr) ;
}

// this オブジェクトを取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLWindowSprite *
	RSWindowSpriteClass::GetThisWindowSprite( RSContext& context, RSObject* pThis )
{
	SGLWindowSprite *	pSprite = NULL ;
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj != NULL )
	{
		pSprite = ESLTypeCast<SGLWindowSprite>( pNativeObj->GetObject() ) ;
	}
	if ( pSprite == NULL )
	{
		context.ThrowExceptionError( L"this が WindowSprite ではありません" ) ;
	}
	return	pSprite ;
}

// SGLWindowSprite 参照オブジェクト生成
//////////////////////////////////////////////////////////////////////////////
RSNativeObject * RSWindowSpriteClass::CreateRefObject( SGLWindowSprite * pWindow )
{
	return	new RSNativeObject( (SGLSprite*) pWindow, this ) ;
}

// void <init>()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSWindowSpriteClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj == NULL )
	{
		context.ThrowExceptionError
			( L"WindowSprite.<init> の this が WindowSprite ではありません" ) ;
		return	NULL ;
	}
	pNativeObj->SetObject
		( SGLWindowSprite::GetESLPointer( new SGLWindowSprite ) ) ;
	return	NULL ;
}

// boolean createDisplay
//	( String name, int mode,
//		int width, int height, int bpp = 0, int frequency = 0 ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSWindowSpriteClass::method_createDisplay
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLWindowSprite *	pWindow = GetThisWindowSprite( context, pThis ) ;
	if ( pWindow == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLError	err =
		pWindow->CreateDisplay
			( arg.StringAt(0), (Window::CooperationMode) arg.IntAt(1),
				(uint32_t) arg.IntAt(2), (uint32_t) arg.IntAt(3),
				(uint32_t) arg.IntAt(4,0), (uint32_t) arg.IntAt(5,0) ) ;
	return	context.new_Boolean( err == sglErrSuccess ) ;
}

// boolean closeDisplay()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSWindowSpriteClass::method_closeDisplay
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLWindowSprite *	pWindow = GetThisWindowSprite( context, pThis ) ;
	if ( pWindow == NULL )
	{
		return	NULL ;
	}
	SGLError	err = pWindow->CloseDisplay() ;
	return	context.new_Boolean( err == sglErrSuccess ) ;
}

// const long getOptionalFlags()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSWindowSpriteClass::method_getOptionalFlags
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLWindowSprite *	pWindow = GetThisWindowSprite( context, pThis ) ;
	if ( pWindow == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( pWindow->GetOptionalFlags() ) ;
}

// void setOptionalFlags( long nFlags )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSWindowSpriteClass::method_setOptionalFlags
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLWindowSprite *	pWindow = GetThisWindowSprite( context, pThis ) ;
	if ( pWindow == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pWindow->SetOptionalFlags( (uint64_t) arg.LongAt(0) ) ;
	return	NULL ;
}

// boolean changeCooperationLevel( int mode )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSWindowSpriteClass::method_changeCooperationLevel
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLWindowSprite *	pWindow = GetThisWindowSprite( context, pThis ) ;
	if ( pWindow == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLError	err =
		pWindow->ChangeCooperationLevel
			( (Window::CooperationMode) arg.IntAt(0) ) ;
	return	context.new_Boolean( err == sglErrSuccess ) ;
}

// boolean changeDisplaySize
//	( int width, int height, int bpp = 0, int frequency = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSWindowSpriteClass::method_changeDisplaySize
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLWindowSprite *	pWindow = GetThisWindowSprite( context, pThis ) ;
	if ( pWindow == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLError	err =
		pWindow->ChangeDisplaySize
			( (uint32_t) arg.IntAt(0), (uint32_t) arg.IntAt(1),
				(uint32_t) arg.IntAt(2,0), (uint32_t) arg.IntAt(3,0) ) ;
	return	context.new_Boolean( err == sglErrSuccess ) ;
}

// boolean getDisplaySize( Size sizeDisplay )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSWindowSpriteClass::method_getDisplaySize
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLWindowSprite *	pWindow = GetThisWindowSprite( context, pThis ) ;
	if ( pWindow == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLSize *	pSizeDisplay = (SGLSize*) arg.PointerAt( 0, sizeof(SGLSize) ) ;
	SGLError	err =
		pWindow->GetDisplaySize( *pSizeDisplay ) ;
	return	context.new_Boolean( err == sglErrSuccess ) ;
}

// boolean enableChangePhysicalMode( boolean flagEnable )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSWindowSpriteClass::method_enableChangePhysicalMode
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLWindowSprite *	pWindow = GetThisWindowSprite( context, pThis ) ;
	if ( pWindow == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLError	err =
		pWindow->EnableChangePhysicalMode( arg.BooleanAt( 0 ) ) ;
	return	context.new_Boolean( err == sglErrSuccess ) ;
}

// boolean enableZBuffer( boolean flagZBuffer )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSWindowSpriteClass::method_enableZBuffer
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLWindowSprite *	pWindow = GetThisWindowSprite( context, pThis ) ;
	if ( pWindow == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLError	err =
		pWindow->EnableZBuffer( arg.BooleanAt( 0 ) ) ;
	return	context.new_Boolean( err == sglErrSuccess ) ;
}

// boolean setStereoDisplayMode( String sMethodID, long nParam = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSWindowSpriteClass::method_setStereoDisplayMode
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLWindowSprite *	pWindow = GetThisWindowSprite( context, pThis ) ;
	if ( pWindow == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLError	err =
		pWindow->SetStereoDisplayMode
			( arg.StringAt(0), (uint64_t) arg.LongAt(1) ) ;
	return	context.new_Boolean( err == sglErrSuccess ) ;
}

// boolean isSupportedStereoDisplayMode( String sMethodID )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSWindowSpriteClass::method_isSupportedStereoDisplayMode
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLWindowSprite *	pWindow = GetThisWindowSprite( context, pThis ) ;
	if ( pWindow == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Boolean
		( pWindow->IsSupportedStereoDisplayMode( arg.StringAt(0) ) ) ;
}

// boolean initWindowPosition
//		( int xPos, int yPos, Size pInitExSize = null )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSWindowSpriteClass::method_initWindowPosition
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLWindowSprite *	pWindow = GetThisWindowSprite( context, pThis ) ;
	if ( pWindow == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	const SGLSize *	pExSize =
			(const SGLSize*) arg.PointerAt( 2, sizeof(SGLSize) ) ;
	SGLError	err =
		pWindow->InitWindowPosition
			( arg.IntAt(0), arg.IntAt(1), pExSize ) ;
	return	context.new_Boolean( err == sglErrSuccess ) ;
}

// boolean getNormalWindowPosition
//		( Point ptWindow, Size sizeWindow = null )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSWindowSpriteClass::method_getNormalWindowPosition
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLWindowSprite *	pWindow = GetThisWindowSprite( context, pThis ) ;
	if ( pWindow == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLPoint *	pWindowPos =
			(SGLPoint*) arg.PointerAt( 0, sizeof(SGLPoint) ) ;
	SGLSize *	pWindowSize =
			(SGLSize*) arg.PointerAt( 1, sizeof(SGLSize) ) ;
	if ( pWindowPos == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	SGLError	err =
		pWindow->GetNormalWindowPosition( *pWindowPos, pWindowSize ) ;
	return	context.new_Boolean( err == sglErrSuccess ) ;
}

// boolean getInternalDisplayPosition
//		( Rect rectRender, Rect rectDisplay )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSWindowSpriteClass::method_getInternalDisplayPosition
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLWindowSprite *	pWindow = GetThisWindowSprite( context, pThis ) ;
	if ( pWindow == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLImageRect *	pRenderRect =
			(SGLImageRect*) arg.PointerAt( 0, sizeof(SGLImageRect) ) ;
	SGLImageRect *	pDisplayRect =
			(SGLImageRect*) arg.PointerAt( 1, sizeof(SGLImageRect) ) ;
	if ( (pRenderRect == NULL) || (pDisplayRect == NULL) )
	{
		return	context.new_Boolean( false ) ;
	}
	SGLError	err =
		pWindow->GetInternalDisplayPosition( *pRenderRect, *pDisplayRect ) ;
	return	context.new_Boolean( err == sglErrSuccess ) ;
}

// boolean setExteriorBackgroundFrame
//		( int nFlags, int rgbColor, Image pTile,
//			Image pLeft = null, Image pRight = null,
//			Image pUpper = null, Image pUnder = null )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSWindowSpriteClass::method_setExteriorBackgroundFrame
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLWindowSprite *	pWindow = GetThisWindowSprite( context, pThis ) ;
	if ( pWindow == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLImageObject *	pTile =
		RSImageClass::ImageFromObject( context, arg.ObjectAt(2) ) ;
	SGLImageObject *	pLeft =
		RSImageClass::ImageFromObject( context, arg.ObjectAt(3) ) ;
	SGLImageObject *	pRight =
		RSImageClass::ImageFromObject( context, arg.ObjectAt(4) ) ;
	SGLImageObject *	pUpper =
		RSImageClass::ImageFromObject( context, arg.ObjectAt(5) ) ;
	SGLImageObject *	pUnder =
		RSImageClass::ImageFromObject( context, arg.ObjectAt(6) ) ;
	SGLError	err =
		pWindow->SetExteriorBackgroundFrame
			( (uint32_t) arg.IntAt(0),
				(uint32_t) arg.IntAt(1),
				pTile, pLeft, pRight, pUpper, pUnder ) ;
	return	context.new_Boolean( err == sglErrSuccess ) ;
}

// boolean createWindow
//		( String name, int width, int height,
//				int flags, WindowSprite parent = null )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSWindowSpriteClass::method_createWindow
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLWindowSprite *	pWindow = GetThisWindowSprite( context, pThis ) ;
	if ( pWindow == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLError	err =
		pWindow->CreateWindow
			( arg.StringAt(0),
				(uint32_t) arg.IntAt(1), (uint32_t) arg.IntAt(2),
				 (uint32_t) arg.IntAt(3,0),
				 ESLTypeCast<SGLWindowSprite>( arg.NativeObjectAt(4) ) ) ;
	return	context.new_Boolean( err == sglErrSuccess ) ;
}

// boolean closeWindow()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSWindowSpriteClass::method_closeWindow
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLWindowSprite *	pWindow = GetThisWindowSprite( context, pThis ) ;
	if ( pWindow == NULL )
	{
		return	NULL ;
	}
	SGLError	err = pWindow->CloseWindow() ;
	return	context.new_Boolean( err == sglErrSuccess ) ;
}

// boolean setWindowLayout( int nFlags, int xPos = 0, int yPos = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSWindowSpriteClass::method_setWindowLayout
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLWindowSprite *	pWindow = GetThisWindowSprite( context, pThis ) ;
	if ( pWindow == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLError	err =
		pWindow->SetWindowLayout
			( (uint32_t) arg.IntAt(0), arg.IntAt(1), arg.IntAt(2) ) ;
	return	context.new_Boolean( err == sglErrSuccess ) ;
}

// boolean changeWindowSize( int width, int height )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSWindowSpriteClass::method_changeWindowSize
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLWindowSprite *	pWindow = GetThisWindowSprite( context, pThis ) ;
	if ( pWindow == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLError	err =
		pWindow->ChangeWindowSize
			( (uint32_t) arg.IntAt(0), (uint32_t) arg.IntAt(1) ) ;
	return	context.new_Boolean( err == sglErrSuccess ) ;
}

// boolean getWindowClientRect( Rect rectClient )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSWindowSpriteClass::method_getWindowClientRect
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLWindowSprite *	pWindow = GetThisWindowSprite( context, pThis ) ;
	if ( pWindow == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLImageRect *	pRectClient =
		(SGLImageRect*) arg.PointerAt( 0, sizeof(SGLImageRect) ) ;
	if ( pRectClient == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	SGLError	err =
		pWindow->GetWindowClientRect( *pRectClient ) ;
	return	context.new_Boolean( err == sglErrSuccess ) ;
}

// Vector2D screenPositionFromClient( Vector2D vClient )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSWindowSpriteClass::method_screenPositionFromClient
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLWindowSprite *	pWindow = GetThisWindowSprite( context, pThis ) ;
	if ( pWindow == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S2DVector *	pClientPos =
		(S2DVector*) arg.PointerAt( 0, sizeof(S2DVector) ) ;
	if ( pClientPos == NULL )
	{
		return	NULL ;
	}
	S2DDVector	vClient = *pClientPos ;
	pWindow->ScreenPositionFromClient( vClient ) ;
	*pClientPos = vClient ;
	//
	RSObject *	pObj = arg.ObjectAt( 0 ) ;
	RSObject::AddRef( pObj ) ;
	return	pObj ;
}

// Vector2D clientPositionFromScreen( Vector2D vScreen )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSWindowSpriteClass::method_clientPositionFromScreen
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLWindowSprite *	pWindow = GetThisWindowSprite( context, pThis ) ;
	if ( pWindow == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	S2DVector *	pScreenPos =
		(S2DVector*) arg.PointerAt( 0, sizeof(S2DVector) ) ;
	if ( pScreenPos == NULL )
	{
		return	NULL ;
	}
	S2DDVector	vScreen = *pScreenPos ;
	pWindow->ClientPositionFromScreen( vScreen ) ;
	*pScreenPos = vScreen ;
	//
	RSObject *	pObj = arg.ObjectAt( 0 ) ;
	RSObject::AddRef( pObj ) ;
	return	pObj ;
}

// boolean postUpdate( Rect pUpdate = null )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSWindowSpriteClass::method_postUpdate
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLWindowSprite *	pWindow = GetThisWindowSprite( context, pThis ) ;
	if ( pWindow == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	const SGLImageRect *	pUpdateRect =
		(const SGLImageRect*) arg.PointerAt( 0, sizeof(SGLImageRect) ) ;
	SGLRect		rectUpdate ;
	SGLRect *	prectUpdate = NULL ;
	if ( pUpdateRect != NULL )
	{
		rectUpdate = *pUpdateRect ;
		prectUpdate = &rectUpdate ;
	}
	pWindow->PostUpdate( prectUpdate ) ;
	return	NULL ;
}

// boolean updateWindow( WindowSprite.UpdateParameter pUpdate = null )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSWindowSpriteClass::method_updateWindow
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLWindowSprite *	pWindow = GetThisWindowSprite( context, pThis ) ;
	if ( pWindow == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	Window::UpdateParameter *	pUpdate =
		(Window::UpdateParameter*)
			arg.PointerAt( 0, sizeof(Window::UpdateParameter) ) ;
	SGLError	err = pWindow->UpdateWindow( pUpdate ) ;
	return	context.new_Boolean( err == sglErrSuccess ) ;
}

// boolean processUserInput( long msecTimeout = 1 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSWindowSpriteClass::method_processUserInput
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLWindowSprite *	pWindow = GetThisWindowSprite( context, pThis ) ;
	if ( pWindow == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLError	err = pWindow->ProcessUserInput( arg.LongAt(0,1) ) ;
	return	context.new_Boolean( err == sglErrSuccess ) ;
}

// boolean postRenderingThread( Runnable proc, int postType )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSWindowSpriteClass::method_postRenderingThread
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLWindowSprite *	pWindow = GetThisWindowSprite( context, pThis ) ;
	if ( pWindow == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSObject *	pRunnable = arg.ObjectAt( 0 ) ;
	if ( pRunnable == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	RSThread::RunnableProcedure *	pProc =
		new RSThread::RunnableProcedure
				( context.GetVM(), NULL, pRunnable, true ) ;
	SGLError	err =
		pWindow->PostRenderingThread
			( pProc, (SGLAbstractWindow::PostThreadType) arg.IntAt(1) ) ;
	return	context.new_Boolean( err == sglErrSuccess ) ;
}

// boolean postUIThread( Runnable proc )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSWindowSpriteClass::method_postUIThread
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLWindowSprite *	pWindow = GetThisWindowSprite( context, pThis ) ;
	if ( pWindow == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSObject *	pRunnable = arg.ObjectAt( 0 ) ;
	if ( pRunnable == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	RSThread::RunnableProcedure *	pProc =
		new RSThread::RunnableProcedure
				( context.GetVM(), NULL, pRunnable, true ) ;
	SGLError	err = pWindow->PostUIThread( pProc ) ;
	return	context.new_Boolean( err == sglErrSuccess ) ;
}

// boolean isWindowActive()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSWindowSpriteClass::method_isWindowActive
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLWindowSprite *	pWindow = GetThisWindowSprite( context, pThis ) ;
	if ( pWindow == NULL )
	{
		return	NULL ;
	}
	return	context.new_Boolean( pWindow->IsWindowActive() ) ;
}

// boolean setWindowCaption( String name )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSWindowSpriteClass::method_setWindowCaption
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLWindowSprite *	pWindow = GetThisWindowSprite( context, pThis ) ;
	if ( pWindow == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLError	err = pWindow->SetWindowCaption( arg.StringAt(0) ) ;
	return	context.new_Boolean( err == sglErrSuccess ) ;
}

// boolean showCursor( boolean fShow )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSWindowSpriteClass::method_showCursor
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLWindowSprite *	pWindow = GetThisWindowSprite( context, pThis ) ;
	if ( pWindow == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLError	err = pWindow->ShowCursor( arg.BooleanAt(0) ) ;
	return	context.new_Boolean( err == sglErrSuccess ) ;
}

// boolean isShowCursor()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSWindowSpriteClass::method_isShowCursor
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLWindowSprite *	pWindow = GetThisWindowSprite( context, pThis ) ;
	if ( pWindow == NULL )
	{
		return	NULL ;
	}
	return	context.new_Boolean( pWindow->IsShowCursor() ) ;
}

// boolean setCursor( String sCursorID )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSWindowSpriteClass::method_setCursor
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLWindowSprite *	pWindow = GetThisWindowSprite( context, pThis ) ;
	if ( pWindow == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLError	err = pWindow->SetCursor( arg.StringAt(0) ) ;
	return	context.new_Boolean( err == sglErrSuccess ) ;
}

// boolean moveCursorPosition( int xPos, int yPos, int idMouse = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSWindowSpriteClass::method_moveCursorPosition
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLWindowSprite *	pWindow = GetThisWindowSprite( context, pThis ) ;
	if ( pWindow == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLError	err =
		pWindow->MoveCursorPosition
			( arg.IntAt(0), arg.IntAt(1), arg.IntAt(2) ) ;
	return	context.new_Boolean( err == sglErrSuccess ) ;
}

// boolean getCursorPosition( Point ptCursor, int idMouse = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSWindowSpriteClass::method_getCursorPosition
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLWindowSprite *	pWindow = GetThisWindowSprite( context, pThis ) ;
	if ( pWindow == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLPoint *	pCursorPos =
		(SGLPoint*) arg.PointerAt( 0, sizeof(SGLPoint) ) ;
	if ( pCursorPos == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	SGLError	err =
		pWindow->GetCursorPosition( *pCursorPos, arg.IntAt(1) ) ;
	return	context.new_Boolean( err == sglErrSuccess ) ;
}

// int getMonitorFrequency()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSWindowSpriteClass::method_getMonitorFrequency
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLWindowSprite *	pWindow = GetThisWindowSprite( context, pThis ) ;
	if ( pWindow == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( pWindow->GetMonitorFrequency() ) ;
}

// void lock()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSWindowSpriteClass::method_lock
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLWindowSprite *	pWindow = GetThisWindowSprite( context, pThis ) ;
	if ( pWindow == NULL )
	{
		SSystem::Lock() ;
		return	NULL ;
	}
	pWindow->Lock() ;
	return	NULL ;
}

// void unlock()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSWindowSpriteClass::method_unlock
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLWindowSprite *	pWindow = GetThisWindowSprite( context, pThis ) ;
	if ( pWindow == NULL )
	{
		SSystem::Unlock() ;
		return	NULL ;
	}
	pWindow->Unlock() ;
	return	NULL ;
}

// long testLocked()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSWindowSpriteClass::method_testLocked
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLWindowSprite *	pWindow = GetThisWindowSprite( context, pThis ) ;
	if ( pWindow == NULL )
	{
		return	context.new_Integer( SSystem::TestLocked() ) ;
	}
	return	context.new_Integer( pWindow->TestLocked() ) ;
}



//////////////////////////////////////////////////////////////////////////////
// VirtualInput.InputEvent クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSVirtualInputClass::InputEventClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSVirtualInputClass::InputEventClass::InputEventClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSVirtualInputClass::InputEventClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSGenericObject( this ) ;
	//
	m_pPrototype->CreateMemberIntegerAs( context, L"typeDevice", 0 ) ;
	m_pPrototype->CreateMemberIntegerAs( context, L"numDevice", 0 ) ;
	m_pPrototype->CreateMemberIntegerAs( context, L"codeKey", 0 ) ;
	m_pPrototype->CreateMemberStringAs( context, L"strCommand", L"" ) ;
}

// Object -> SGLVirtualInput::InputEvent 変換
//////////////////////////////////////////////////////////////////////////////
void RSVirtualInputClass::InputEventClass::InputEventFromObject
	( RSContext& context, SGLVirtualInput::InputEvent& ev, RSObject * obj )
{
	ev.typeDevice = (SGLVirtualInput::DeviceType)
						obj->GetMemberIntegerAs( context, L"typeDevice" ) ;
	ev.numDevice = obj->GetMemberIntegerAs( context, L"numDevice" ) ;
	ev.codeKey = obj->GetMemberIntegerAs( context, L"codeKey" ) ;
	ev.strCommand = obj->GetMemberStringAs( context, L"strCommand" ) ;
}

// Object <- SGLVirtualInput::InputEvent 変換
//////////////////////////////////////////////////////////////////////////////
void RSVirtualInputClass::InputEventClass::InputEventToObject
	( RSContext& context, RSObject * obj, const SGLVirtualInput::InputEvent& ev )
{
	obj->SetMemberIntegerAs( context, L"typeDevice", ev.typeDevice ) ;
	obj->SetMemberIntegerAs( context, L"numDevice", ev.numDevice ) ;
	obj->SetMemberIntegerAs( context, L"codeKey", ev.codeKey ) ;
	obj->SetMemberStringAs( context, L"strCommand", ev.strCommand ) ;
}


//////////////////////////////////////////////////////////////////////////////
// VirtualInput.Command クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSVirtualInputClass::CommandClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSVirtualInputClass::CommandClass::CommandClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSVirtualInputClass::CommandClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSGenericObject( this ) ;
	//
	m_pPrototype->CreateMemberStringAs( context, L"strFullID", L"" ) ;
	m_pPrototype->CreateMemberStringAs( context, L"strID", L"" ) ;
	m_pPrototype->CreateMemberIntegerAs( context, L"nParam", 0 ) ;
	m_pPrototype->CreateMemberIntegerAs( context, L"nCode", 0 ) ;
	m_pPrototype->CreateMemberIntegerAs( context, L"nPriority", 0 ) ;
	context.ReleaseObjectRef
		( m_pPrototype->CreateMemberAs
			( context, L"fOverwritable", context.new_Boolean(false) ) ) ;
	//
	CreateMemberStringAs
		( context, L"AppExit", SysCommandId::AppExit, modifierConst ) ;
	CreateMemberStringAs
		( context, L"AppBack", SysCommandId::AppBack, modifierConst ) ;
	CreateMemberStringAs
		( context, L"AppSuspend", SysCommandId::AppSuspend, modifierConst ) ;
	CreateMemberStringAs
		( context, L"AppResume", SysCommandId::AppResume, modifierConst ) ;
	CreateMemberStringAs
		( context, L"AppDestroy", SysCommandId::AppDestroy, modifierConst ) ;
	CreateMemberStringAs
		( context, L"WindowActive", SysCommandId::WindowActive, modifierConst ) ;
	CreateMemberStringAs
		( context, L"WindowInactive", SysCommandId::WindowInactive, modifierConst ) ;
	CreateMemberStringAs
		( context, L"WindowSizeChanged", SysCommandId::WindowSizeChanged, modifierConst ) ;
}

// Object <- SGLVirtualInput::Command 変換
//////////////////////////////////////////////////////////////////////////////
void RSVirtualInputClass::CommandClass::CommandToObject
	( RSContext& context, RSObject * obj, const SGLVirtualInput::Command& cmd )
{
	obj->SetMemberStringAs( context, L"strFullID", cmd.strFullID ) ;
	obj->SetMemberStringAs( context, L"strID", cmd.strID ) ;
	obj->SetMemberIntegerAs( context, L"nParam", cmd.nParam ) ;
	obj->SetMemberIntegerAs( context, L"nCode", cmd.nCode ) ;
	obj->SetMemberIntegerAs( context, L"nPriority", cmd.nPriority ) ;
	context.ReleaseObjectRef
		( obj->SetMemberAs
			( context, L"fOverwritable",
				context.new_Boolean( cmd.fOverwritable ) ) ) ;
}


//////////////////////////////////////////////////////////////////////////////
// VirtualInput クラス
//////////////////////////////////////////////////////////////////////////////

#define	VKEY_DESC(x)	{ L###x, x }
#define	VJOY_DESC(x)	{ L###x, SGLVirtualInput::x }

const SSystem::SXMLDocument::AttrInteger
	RSVirtualInputClass::m_aiKeyCode[] =
{
	VKEY_DESC(vkeyMouseLeft),
	VKEY_DESC(vkeyMouseRight),
	VKEY_DESC(vkeyMouseMiddle),
	VKEY_DESC(vkeyBack),
	VKEY_DESC(vkeyTab),
	VKEY_DESC(vkeyReturn),
	VKEY_DESC(vkeyShift),
	VKEY_DESC(vkeyControl),
	VKEY_DESC(vkeyMenu),
	VKEY_DESC(vkeyPause),
	VKEY_DESC(vkeyCapital),
	VKEY_DESC(vkeyEscape),
	VKEY_DESC(vkeySpace),
	VKEY_DESC(vkeyPageUp),
	VKEY_DESC(vkeyPageDown),
	VKEY_DESC(vkeyEnd),
	VKEY_DESC(vkeyHome),
	VKEY_DESC(vkeyLeft),
	VKEY_DESC(vkeyUp),
	VKEY_DESC(vkeyRight),
	VKEY_DESC(vkeyDown),
	VKEY_DESC(vkeyInsert),
	VKEY_DESC(vkeyDelete),
	VKEY_DESC(vkeyHelp),
	VKEY_DESC(vkeyNumPad0),
	VKEY_DESC(vkeyNumPad1),
	VKEY_DESC(vkeyNumPad2),
	VKEY_DESC(vkeyNumPad3),
	VKEY_DESC(vkeyNumPad4),
	VKEY_DESC(vkeyNumPad5),
	VKEY_DESC(vkeyNumPad6),
	VKEY_DESC(vkeyNumPad7),
	VKEY_DESC(vkeyNumPad8),
	VKEY_DESC(vkeyNumPad9),
	VKEY_DESC(vkeyNumPadMultiply),
	VKEY_DESC(vkeyNumPadAdd),
	VKEY_DESC(vkeyNumPadSeparator),
	VKEY_DESC(vkeyNumPadSubtract),
	VKEY_DESC(vkeyNumPadDecimal),
	VKEY_DESC(vkeyNumPadDivide),
	VKEY_DESC(vkeyFunction1),
	VKEY_DESC(vkeyFunction2),
	VKEY_DESC(vkeyFunction3),
	VKEY_DESC(vkeyFunction4),
	VKEY_DESC(vkeyFunction5),
	VKEY_DESC(vkeyFunction6),
	VKEY_DESC(vkeyFunction7),
	VKEY_DESC(vkeyFunction8),
	VKEY_DESC(vkeyFunction9),
	VKEY_DESC(vkeyFunction10),
	VKEY_DESC(vkeyFunction11),
	VKEY_DESC(vkeyFunction12),
	VKEY_DESC(vkeyNumLock),
	VKEY_DESC(vkeyScroll),
	VKEY_DESC(vkeyPlay),
	VKEY_DESC(vkeyCodeMask),
	VKEY_DESC(vkeyContextCapital),
	VKEY_DESC(vkeyContextShift),
	VKEY_DESC(vkeyContextControl),
	VKEY_DESC(vkeyContextMenu),
	VKEY_DESC(vkeyContextMask),
	VJOY_DESC(deviceKeyboard),
	VJOY_DESC(deviceMouse),
	VJOY_DESC(deviceJoyStick),
	VJOY_DESC(deviceCommand),
	VJOY_DESC(deviceSignal),
	VJOY_DESC(joyStickId1),
	VJOY_DESC(joyStickId2),
	VJOY_DESC(joyStickXInput1),
	VJOY_DESC(joyStickXInput2),
	VJOY_DESC(joyStickXInput3),
	VJOY_DESC(joyStickXInput4),
	VJOY_DESC(joyStickUser1),
	VJOY_DESC(joyStickUser2),
	VJOY_DESC(joyStickUser3),
	VJOY_DESC(joyStickUser4),
	VJOY_DESC(joyUp),
	VJOY_DESC(joyDown),
	VJOY_DESC(joyLeft),
	VJOY_DESC(joyRight),
	VJOY_DESC(joyButton1),
	VJOY_DESC(joyButton2),
	VJOY_DESC(joyButton3),
	VJOY_DESC(joyButton4),
	VJOY_DESC(joyButton5),
	VJOY_DESC(joyButton6),
	VJOY_DESC(joyButton7),
	VJOY_DESC(joyButton8),
	VJOY_DESC(xinputDPadUp),
	VJOY_DESC(xinputDPadDown),
	VJOY_DESC(xinputDPadLeft),
	VJOY_DESC(xinputDPadRight),
	VJOY_DESC(xinputStart),
	VJOY_DESC(xinputBack),
	VJOY_DESC(xinputLeftThumb),
	VJOY_DESC(xinputRightThumb),
	VJOY_DESC(xinputLeftShoulder),
	VJOY_DESC(xinputRightShoulder),
	VJOY_DESC(xinputButtonA),
	VJOY_DESC(xinputButtonB),
	VJOY_DESC(xinputButtonX),
	VJOY_DESC(xinputButtonY),
	VJOY_DESC(xinputLeftTrigger),
	VJOY_DESC(xinputRightTrigger),
	VJOY_DESC(androidPadUp),
	VJOY_DESC(androidPadDown),
	VJOY_DESC(androidPadLeft),
	VJOY_DESC(androidPadRight),
	VJOY_DESC(androidButtonA),
	VJOY_DESC(androidButtonB),
	VJOY_DESC(androidButtonC),
	VJOY_DESC(androidButtonX),
	VJOY_DESC(androidButtonY),
	VJOY_DESC(androidButtonZ),
	VJOY_DESC(androidButtonL1),
	VJOY_DESC(androidButtonR1),
	VJOY_DESC(androidButtonL2),
	VJOY_DESC(androidButtonR2),
	VJOY_DESC(androidButtonStart),
	VJOY_DESC(androidButtonSelect),
	{ NULL, 0 },
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSVirtualInputClass, RGenericNativeObjectClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSVirtualInputClass::RSVirtualInputClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RGenericNativeObjectClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSVirtualInputClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( NULL, this ) ;
	//
	InputEventClass *
		pInputEventClass =
			new InputEventClass( context.GetVM()->GetClassClass() ) ;
	pInputEventClass->Initialize( context ) ;
	pInputEventClass->FinishClass( context ) ;
	context.ReleaseObjectRef
		( CreateMemberAs( context, L"InputEvent", pInputEventClass ) ) ;
	//
	CommandClass *
		pCommandClass =
			new CommandClass( context.GetVM()->GetClassClass() ) ;
	pCommandClass->Initialize( context ) ;
	pCommandClass->FinishClass( context ) ;
	context.ReleaseObjectRef
		( CreateMemberAs( context, L"Command", pCommandClass ) ) ;
	//
	for ( int i = 0; m_aiKeyCode[i].pszSymbol != NULL; i ++ )
	{
		CreateMemberIntegerAs
			( context, m_aiKeyCode[i].pszSymbol,
					m_aiKeyCode[i].nValue, modifierConst ) ;
	}
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"",
			NULL, &RSVirtualInputClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"attachPostListenerToWindow",
			NULL, L"WindowSprite window",
			NULL, &RSVirtualInputClass::method_attachPostListenerToWindow, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"detachPostListenerToWindow",
			NULL, L"WindowSprite window",
			NULL, &RSVirtualInputClass::method_detachPostListenerToWindow, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"pollJoyStick",
			L"boolean", L"",
			NULL, &RSVirtualInputClass::method_pollJoyStick, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getAnalogJoyPosition",
			L"boolean", L"Vector3D4 vPos, int joyStick = 0",
			NULL, &RSVirtualInputClass::method_getAnalogJoyPosition, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"isJoyButtonPushing",
			L"boolean", L"int joyButton, int joyStick = 0",
			NULL, &RSVirtualInputClass::method_isJoyButtonPushing, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getJoyButtonPushed",
			L"int", L"int joyButton, int joyStick = 0",
			NULL, &RSVirtualInputClass::method_getJoyButtonPushed, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"resetJoyButtonPushed",
			NULL, L"int joyButton, int joyStick = 0",
			NULL, &RSVirtualInputClass::method_resetJoyButtonPushed, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"resetAllJoyButtonPushed",
			NULL, L"",
			NULL, &RSVirtualInputClass::method_resetAllJoyButtonPushed, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"pressInputEvent",
			NULL, L"VirtualInput.InputEvent evIn",
			NULL, &RSVirtualInputClass::method_pressInputEvent, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"releaseInputEvent",
			NULL, L"VirtualInput.InputEvent evIn",
			NULL, &RSVirtualInputClass::method_releaseInputEvent, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getInputEvent",
			L"boolean", L"VirtualInput.InputEvent ev",
			NULL, &RSVirtualInputClass::method_getInputEvent, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getInputEvent",
			L"boolean", L"VirtualInput.InputEvent ev",
			NULL, &RSVirtualInputClass::method_getInputEvent, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setInputQueueLimit",
			NULL, L"int nLimit",
			NULL, &RSVirtualInputClass::method_setInputQueueLimit, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"flushInputQueue",
			NULL, L"",
			NULL, &RSVirtualInputClass::method_flushInputQueue, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"addCommand",
			NULL, L"String strCmd, int nParam = 0, int nCode = 0, "
					L"int nPriority = Sprite.commandNormal, "
					L"boolean fOverwritable = false",
			NULL, &RSVirtualInputClass::method_addCommand, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getCommand",
			L"boolean", L"VirtualInput.Command cmd",
			NULL, &RSVirtualInputClass::method_getCommand, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"flushCommandQueue",
			NULL, L"",
			NULL, &RSVirtualInputClass::method_flushCommandQueue, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"addFilter",
			NULL, L"VirtualInput.InputEvent evIn, VirtualInput.InputEvent evOut",
			NULL, &RSVirtualInputClass::method_addFilter, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"removeFilter",
			NULL, L"VirtualInput.InputEvent evIn",
			NULL, &RSVirtualInputClass::method_removeFilter, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"removeAllFilter",
			NULL, L"",
			NULL, &RSVirtualInputClass::method_removeAllFilter, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getFilterAs",
			L"VirtualInput.InputEvent", L"VirtualInput.InputEvent evIn",
			NULL, &RSVirtualInputClass::method_getFilterAs,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"addInputMap",
			NULL, L"VirtualInput.InputEvent evIn, VirtualInput.InputEvent evOut",
			NULL, &RSVirtualInputClass::method_addInputMap, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"removeInputMap",
			NULL, L"VirtualInput.InputEvent evIn",
			NULL, &RSVirtualInputClass::method_removeInputMap, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"removeAllInputMap",
			NULL, L"",
			NULL, &RSVirtualInputClass::method_removeAllInputMap, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getInputMapAs",
			L"VirtualInput.InputEvent", L"VirtualInput.InputEvent evIn",
			NULL, &RSVirtualInputClass::method_getInputMapAs,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"loadPrefilter",
			L"boolean", L"String sFilterFile",
			NULL, &RSVirtualInputClass::method_loadPrefilter, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"readPrefilter",
			L"boolean", L"InputStream isFilter",
			NULL, &RSVirtualInputClass::method_readPrefilter, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"loadPostfilter",
			L"boolean", L"String sFilterFile",
			NULL, &RSVirtualInputClass::method_loadPostfilter, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"readPostfilter",
			L"boolean", L"InputStream isFilter",
			NULL, &RSVirtualInputClass::method_readPostfilter, NULL ) ;
}

// ネイティブ型テスト
//////////////////////////////////////////////////////////////////////////////
bool RSVirtualInputClass::IsNativeObjectOf( ESLObject * pObj ) const
{
	return	(ESLTypeCast<SGLVirtualInput>(pObj) != nullptr) ;
}

// this オブジェクトを取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLVirtualInput *
	RSVirtualInputClass::GetThisVirtualInput( RSContext& context, RSObject* pThis )
{
	SGLVirtualInput *	pInput = NULL ;
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj != NULL )
	{
		pInput = ESLTypeCast<SGLVirtualInput>( pNativeObj->GetObject() ) ;
	}
	if ( pInput == NULL )
	{
		context.ThrowExceptionError( L"this が VirtualInput ではありません" ) ;
	}
	return	pInput ;
}

// SGLVirtualInput 参照オブジェクト生成
//////////////////////////////////////////////////////////////////////////////
RSNativeObject * RSVirtualInputClass::CreateRefObject( SGLVirtualInput * pInput )
{
	return	new RSNativeObject( pInput, this ) ;
}

// void <init>()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVirtualInputClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj == NULL )
	{
		context.ThrowExceptionError
			( L"VirtualInput.<init> の this が VirtualInput ではありません" ) ;
		return	NULL ;
	}
	pNativeObj->SetObject( new SGLVirtualInput ) ;
	return	NULL ;
}

// void attachPostListenerToWindow( WindowSprite window )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVirtualInputClass::method_attachPostListenerToWindow
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLVirtualInput *	pInput = GetThisVirtualInput( context, pThis ) ;
	if ( pInput == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSNativeObject *
		pNObjWindow = ESLTypeCast<RSNativeObject>( arg.ObjectAt( 0 ) ) ;
	if ( pNObjWindow == NULL )
	{
		return	NULL ;
	}
	SGLWindowSprite *	pWindow =
		ESLTypeCast<SGLWindowSprite>( pNObjWindow->GetObject() ) ;
	if ( pWindow == NULL )
	{
		return	NULL ;
	}
	pInput->AttachPostListenerToWindow( pWindow ) ;
	return	NULL ;
}

// void detachPostListenerToWindow( WindowSprite window )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVirtualInputClass::method_detachPostListenerToWindow
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLVirtualInput *	pInput = GetThisVirtualInput( context, pThis ) ;
	if ( pInput == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSNativeObject *
		pNObjWindow = ESLTypeCast<RSNativeObject>( arg.ObjectAt( 0 ) ) ;
	if ( pNObjWindow == NULL )
	{
		return	NULL ;
	}
	SGLWindowSprite *	pWindow =
		ESLTypeCast<SGLWindowSprite>( pNObjWindow->GetObject() ) ;
	if ( pWindow == NULL )
	{
		return	NULL ;
	}
	pInput->DetachPostListenerToWindow( pWindow ) ;
	return	NULL ;
}

// boolean pollJoyStick( void ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVirtualInputClass::method_pollJoyStick
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLVirtualInput *	pInput = GetThisVirtualInput( context, pThis ) ;
	if ( pInput == NULL )
	{
		return	NULL ;
	}
	return	context.new_Boolean( pInput->PollJoyStick() == sglErrSuccess ) ;
}

// boolean getAnalogJoyPosition( Vector4D vPos, int joyStick = 0 ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVirtualInputClass::method_getAnalogJoyPosition
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLVirtualInput *	pInput = GetThisVirtualInput( context, pThis ) ;
	if ( pInput == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	size_t			nPosSize ;
	S3DVector4 *	pPos = (S3DVector4*) arg.PointerAt( 0, &nPosSize ) ;
	if ( nPosSize < sizeof(S3DVector4) )
	{
		context.ThrowExceptionError
			( L"Vector4D 引数の有効配列長が不足しています" ) ;
		return	NULL ;
	}
	return	context.new_Boolean
		( pInput->GetAnalogJoyPosition
			( *pPos, (size_t) arg.IntAt( 1 ) ) == sglErrSuccess ) ;
}

// boolean isJoyButtonPushing( int joyButton, int joyStick = 0 ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVirtualInputClass::method_isJoyButtonPushing
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLVirtualInput *	pInput = GetThisVirtualInput( context, pThis ) ;
	if ( pInput == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Boolean
		( pInput->IsJoyButtonPushing
			( (size_t) arg.IntAt( 0 ), (size_t) arg.IntAt( 1 ) ) ) ;
}

// int getJoyButtonPushed( int joyButton, int joyStick = 0 ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVirtualInputClass::method_getJoyButtonPushed
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLVirtualInput *	pInput = GetThisVirtualInput( context, pThis ) ;
	if ( pInput == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Integer
		( pInput->GetJoyButtonPushed
			( (size_t) arg.IntAt( 0 ), (size_t) arg.IntAt( 1 ) ) ) ;
}

// void resetJoyButtonPushed( int joyButton, int joyStick = 0 ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVirtualInputClass::method_resetJoyButtonPushed
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLVirtualInput *	pInput = GetThisVirtualInput( context, pThis ) ;
	if ( pInput == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pInput->ResetJoyButtonPushed
			( (size_t) arg.IntAt( 0 ), (size_t) arg.IntAt( 1 ) ) ;
	return	NULL ;
}

// void resetAllJoyButtonPushed( void ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVirtualInputClass::method_resetAllJoyButtonPushed
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLVirtualInput *	pInput = GetThisVirtualInput( context, pThis ) ;
	if ( pInput == NULL )
	{
		return	NULL ;
	}
	pInput->ResetAllJoyButtonPushed() ;
	return	NULL ;
}

// void pressInputEvent( InputEvent evIn ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVirtualInputClass::method_pressInputEvent
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLVirtualInput *	pInput = GetThisVirtualInput( context, pThis ) ;
	if ( pInput == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLVirtualInput::InputEvent	ev ;
	InputEventClass::InputEventFromObject( context, ev, arg.ObjectAt( 0 ) ) ;
	pInput->PressInputEvent( ev ) ;
	return	NULL ;
}

// void releaseInputEvent( InputEvent evIn ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVirtualInputClass::method_releaseInputEvent
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLVirtualInput *	pInput = GetThisVirtualInput( context, pThis ) ;
	if ( pInput == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLVirtualInput::InputEvent	ev ;
	InputEventClass::InputEventFromObject( context, ev, arg.ObjectAt( 0 ) ) ;
	pInput->ReleaseInputEvent( ev ) ;
	return	NULL ;
}

// boolean getInputEvent( InputEvent ev ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVirtualInputClass::method_getInputEvent
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLVirtualInput *	pInput = GetThisVirtualInput( context, pThis ) ;
	if ( pInput == NULL )
	{
		return	NULL ;
	}
	SGLVirtualInput::InputEvent	ev ;
	if ( pInput->GetInputEvent( ev ) != sglErrSuccess )
	{
		return	context.new_Boolean( false ) ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	InputEventClass::InputEventToObject( context, arg.ObjectAt(0), ev ) ;
	return	context.new_Boolean( true ) ;
}

// void setInputQueueLimit( int nLimit ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVirtualInputClass::method_setInputQueueLimit
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLVirtualInput *	pInput = GetThisVirtualInput( context, pThis ) ;
	if ( pInput == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pInput->SetInputQueueLimit( (size_t) arg.IntAt( 0 ) ) ;
	return	NULL ;
}

// void flushInputQueue( void ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVirtualInputClass::method_flushInputQueue
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLVirtualInput *	pInput = GetThisVirtualInput( context, pThis ) ;
	if ( pInput == NULL )
	{
		return	NULL ;
	}
	pInput->FlushInputQueue() ;
	return	NULL ;
}

// void addCommand
//	( String strCmd, int nParam = 0, int nCode = 0,
//		int nPriority = Sprite.commandNormal,
//		boolean fOverwritable = false ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVirtualInputClass::method_addCommand
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLVirtualInput *	pInput = GetThisVirtualInput( context, pThis ) ;
	if ( pInput == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pInput->AddCommand
		( arg.StringAt(0), arg.IntAt(1), arg.IntAt(2),
			arg.IntAt( 3, SGLSprite::commandNormal ),
			arg.BooleanAt( 4, false ) ) ;
	return	NULL ;
}

// boolean getCommand( Command cmd ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVirtualInputClass::method_getCommand
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLVirtualInput *	pInput = GetThisVirtualInput( context, pThis ) ;
	if ( pInput == NULL )
	{
		return	NULL ;
	}
	SGLVirtualInput::Command	cmd ;
	if ( pInput->GetCommand( cmd ) != sglErrSuccess )
	{
		return	context.new_Boolean( false ) ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	CommandClass::CommandToObject( context, arg.ObjectAt(0), cmd ) ;
	return	context.new_Boolean( true ) ;
}

// void flushCommandQueue( void ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVirtualInputClass::method_flushCommandQueue
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLVirtualInput *	pInput = GetThisVirtualInput( context, pThis ) ;
	if ( pInput == NULL )
	{
		return	NULL ;
	}
	pInput->FlushCommandQueue() ;
	return	NULL ;
}

// void addFilter( InputEvent evIn, InputEvent evOut ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVirtualInputClass::method_addFilter
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLVirtualInput *	pInput = GetThisVirtualInput( context, pThis ) ;
	if ( pInput == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLVirtualInput::InputEvent	evIn ;
	SGLVirtualInput::InputEvent	evOut ;
	InputEventClass::InputEventFromObject( context, evIn, arg.ObjectAt(0) ) ;
	InputEventClass::InputEventFromObject( context, evOut, arg.ObjectAt(1) ) ;
	pInput->AddFilter( evIn, evOut ) ;
	return	NULL ;
}

// void removeFilter( InputEvent evIn ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVirtualInputClass::method_removeFilter
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLVirtualInput *	pInput = GetThisVirtualInput( context, pThis ) ;
	if ( pInput == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLVirtualInput::InputEvent	evIn ;
	InputEventClass::InputEventFromObject( context, evIn, arg.ObjectAt(0) ) ;
	pInput->RemoveFilter( evIn ) ;
	return	NULL ;
}

// void removeAllFilter( void ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVirtualInputClass::method_removeAllFilter
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLVirtualInput *	pInput = GetThisVirtualInput( context, pThis ) ;
	if ( pInput == NULL )
	{
		return	NULL ;
	}
	pInput->RemoveAllFilter() ;
	return	NULL ;
}

// InputEvent getFilterAs( InputEvent evIn ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVirtualInputClass::method_getFilterAs
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLVirtualInput *	pInput = GetThisVirtualInput( context, pThis ) ;
	if ( pInput == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLVirtualInput::InputEvent	evIn ;
	InputEventClass::InputEventFromObject( context, evIn, arg.ObjectAt(0) ) ;
	const SGLVirtualInput::InputEvent *
			pevOut = pInput->GetFilterAs( evIn ) ;
	if ( pevOut == NULL )
	{
		return	NULL ;
	}
	RSObject *	pObj = context.new_Object( L"VirtualIntpu.InputEvent" ) ;
	ESLAssert( pObj != NULL ) ;
	InputEventClass::InputEventToObject( context, pObj, *pevOut ) ;
	return	pObj ;
}

// void addInputMap( InputEvent evIn, InputEvent evOut ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVirtualInputClass::method_addInputMap
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLVirtualInput *	pInput = GetThisVirtualInput( context, pThis ) ;
	if ( pInput == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLVirtualInput::InputEvent	evIn ;
	SGLVirtualInput::InputEvent	evOut ;
	InputEventClass::InputEventFromObject( context, evIn, arg.ObjectAt(0) ) ;
	InputEventClass::InputEventFromObject( context, evOut, arg.ObjectAt(1) ) ;
	pInput->AddInputMap( evIn, evOut ) ;
	return	NULL ;
}

// void removeInputMap( InputEvent evIn ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVirtualInputClass::method_removeInputMap
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLVirtualInput *	pInput = GetThisVirtualInput( context, pThis ) ;
	if ( pInput == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLVirtualInput::InputEvent	evIn ;
	InputEventClass::InputEventFromObject( context, evIn, arg.ObjectAt(0) ) ;
	pInput->RemoveInputMap( evIn ) ;
	return	NULL ;
}

// void removeAllInputMap( void ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVirtualInputClass::method_removeAllInputMap
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLVirtualInput *	pInput = GetThisVirtualInput( context, pThis ) ;
	if ( pInput == NULL )
	{
		return	NULL ;
	}
	pInput->RemoveAllInputMap() ;
	return	NULL ;
}

// InputEvent getInputMapAs( InputEvent evIn ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVirtualInputClass::method_getInputMapAs
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLVirtualInput *	pInput = GetThisVirtualInput( context, pThis ) ;
	if ( pInput == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLVirtualInput::InputEvent	evIn ;
	InputEventClass::InputEventFromObject( context, evIn, arg.ObjectAt(0) ) ;
	const SGLVirtualInput::InputEvent *
			pevOut = pInput->GetInputMapAs( evIn ) ;
	if ( pevOut == NULL )
	{
		return	NULL ;
	}
	RSObject *	pObj = context.new_Object( L"VirtualIntpu.InputEvent" ) ;
	ESLAssert( pObj != NULL ) ;
	InputEventClass::InputEventToObject( context, pObj, *pevOut ) ;
	return	pObj ;
}

// boolean loadPrefilter( String sFilterFile ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVirtualInputClass::method_loadPrefilter
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLVirtualInput *	pInput = GetThisVirtualInput( context, pThis ) ;
	if ( pInput == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Boolean
		( pInput->LoadPrefilter( arg.StringAt(0) ) == sglErrSuccess ) ;
}

// boolean readPrefilter( InputStream isFilter ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVirtualInputClass::method_readPrefilter
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLVirtualInput *	pInput = GetThisVirtualInput( context, pThis ) ;
	if ( pInput == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SFileInterface *	pFile =
		RSInputStreamClass::GetFileOf( context, arg.ObjectAt(0) ) ;
	if ( pFile == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	SXMLDocument	xmlDoc ;
	if ( xmlDoc.ReadDocument( *pFile, xmlDoc ) )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean
		( pInput->ParsePrefilter( xmlDoc ) == sglErrSuccess ) ;
}

// boolean loadPostfilter( String sFilterFile ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVirtualInputClass::method_loadPostfilter
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLVirtualInput *	pInput = GetThisVirtualInput( context, pThis ) ;
	if ( pInput == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Boolean
		( pInput->LoadPostfilter( arg.StringAt(0) ) == sglErrSuccess ) ;
}

// boolean readPostfilter( InputStream isFilter ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSVirtualInputClass::method_readPostfilter
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLVirtualInput *	pInput = GetThisVirtualInput( context, pThis ) ;
	if ( pInput == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SFileInterface *	pFile =
		RSInputStreamClass::GetFileOf( context, arg.ObjectAt(0) ) ;
	if ( pFile == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	SXMLDocument	xmlDoc ;
	if ( xmlDoc.ReadDocument( *pFile, xmlDoc ) )
	{
		return	context.new_Boolean( false ) ;
	}
	return	context.new_Boolean
		( pInput->ParsePostfilter( xmlDoc ) == sglErrSuccess ) ;
}



//////////////////////////////////////////////////////////////////////////////
// FontStyle クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSFontStyleClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSFontStyleClass::RSFontStyleClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSFontStyleClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSGenericObject( this ) ;
	//
	m_pPrototype->CreateMemberIntegerAs( context, L"nStyles", 0 ) ;
	m_pPrototype->CreateMemberIntegerAs( context, L"nSize", 0 ) ;
	m_pPrototype->CreateMemberStringAs( context, L"strFace", L"" ) ;
	//
	CreateMemberIntegerAs
		( context, L"styleItalic", SGLFontStyle::styleItalic, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"styleBold", SGLFontStyle::styleBold, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"styleNoSmooth", SGLFontStyle::styleNoSmooth, modifierConst ) ;
	//
	CreateMemberStringAs
		( context, L"StandardFont",
			SGLFontStyle::StandardFont, modifierConst ) ;
	CreateMemberStringAs
		( context, L"FixedPitchFont",
			SGLFontStyle::FixedPitchFont, modifierConst ) ;
}

// SGLFontStyle から変換
//////////////////////////////////////////////////////////////////////////////
void RSFontStyleClass::ConvertToObject
	( RSContext& context, RSObject * pObj,
		const SakuraGL::SGLFontStyle& style )
{
	pObj->SetMemberIntegerAs( context, L"nStyles", style.nStyles ) ;
	pObj->SetMemberIntegerAs( context, L"nSize", style.nSize ) ;
	pObj->SetMemberStringAs( context, L"strFace", style.pszFace ) ;
}

// SGLFontStyle へ変換
//////////////////////////////////////////////////////////////////////////////
void RSFontStyleClass::ConvertFromObject
	( RSContext& context,
		SakuraGL::SGLFontStyle& style,
		SSystem::SString& strFace, RSObject * pObj )
{
	style.nStyles = (uint32_t)
		pObj->GetMemberIntegerAs( context, L"nStyles", style.nStyles ) ;
	style.nSize = (uint32_t)
		pObj->GetMemberIntegerAs( context, L"nSize", style.nSize ) ;
	strFace = pObj->GetMemberStringAs( context, L"strFace", style.pszFace ) ;
	style.pszFace = strFace ;
}


//////////////////////////////////////////////////////////////////////////////
// LetteringContext クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSLetteringContextClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSLetteringContextClass::RSLetteringContextClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSLetteringContextClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSGenericObject( this ) ;
	//
	SGLLetteringContext	ltctx ;
	//
	context.ReleaseObjectRef
		( m_pPrototype->CreateMemberAs
			( context, L"rectWritable",
				context.new_ObjectPointer( L"Rect" ) ) ) ;
	m_pPrototype->CreateMemberIntegerAs
		( context, L"typeAlignment", ltctx.typeAlignment ) ;
	m_pPrototype->CreateMemberIntegerAs
		( context, L"flagVertical", ltctx.flagVertical ) ;
	m_pPrototype->CreateMemberIntegerAs
		( context, L"pitchChar", ltctx.pitchChar ) ;
	m_pPrototype->CreateMemberIntegerAs
		( context, L"offsetChar", ltctx.offsetChar ) ;
	m_pPrototype->CreateMemberIntegerAs
		( context, L"scalePitch", ltctx.scalePitch ) ;
	m_pPrototype->CreateMemberIntegerAs
		( context, L"pitchTab", ltctx.pitchTab ) ;
	m_pPrototype->CreateMemberIntegerAs
		( context, L"pitchLine", ltctx.pitchLine ) ;
	m_pPrototype->CreateMemberIntegerAs
		( context, L"widthIndent", ltctx.widthIndent ) ;
	m_pPrototype->CreateMemberIntegerAs
		( context, L"minHyphening", ltctx.minHyphening ) ;
	m_pPrototype->CreateMemberIntegerAs
		( context, L"maxProhibition", ltctx.maxProhibition ) ;
	m_pPrototype->CreateMemberStringAs
		( context, L"strProhibition", ltctx.pwszProhibition ) ;
}

// SGLLetteringContext から変換
//////////////////////////////////////////////////////////////////////////////
void RSLetteringContextClass::ConvertToObject
	( RSContext& context,
		RSObject * pObj, const SGLLetteringContext& ltctx )
{
	RSSmartPtr	pRect( pObj->GetMemberAs( context, L"rectWritable" ), &context ) ;
	if ( pRect != NULL )
	{
		pRect->SetMemberIntegerAs( context, L"x", ltctx.rectWritable.left ) ;
		pRect->SetMemberIntegerAs( context, L"y", ltctx.rectWritable.top ) ;
		pRect->SetMemberIntegerAs( context, L"w", ltctx.rectWritable.GetWidth() ) ;
		pRect->SetMemberIntegerAs( context, L"h", ltctx.rectWritable.GetHeight() ) ;
	}
	pObj->SetMemberIntegerAs
		( context, L"typeAlignment", ltctx.typeAlignment ) ;
	pObj->SetMemberIntegerAs
		( context, L"flagVertical", ltctx.flagVertical ) ;
	pObj->SetMemberIntegerAs
		( context, L"pitchChar", ltctx.pitchChar ) ;
	pObj->SetMemberIntegerAs
		( context, L"offsetChar", ltctx.offsetChar ) ;
	pObj->SetMemberIntegerAs
		( context, L"scalePitch", ltctx.scalePitch ) ;
	pObj->SetMemberIntegerAs
		( context, L"pitchTab", ltctx.pitchTab ) ;
	pObj->SetMemberIntegerAs
		( context, L"pitchLine", ltctx.pitchLine ) ;
	pObj->SetMemberIntegerAs
		( context, L"widthIndent", ltctx.widthIndent ) ;
	pObj->SetMemberIntegerAs
		( context, L"minHyphening", ltctx.minHyphening ) ;
	pObj->SetMemberIntegerAs
		( context, L"maxProhibition", ltctx.maxProhibition ) ;
	pObj->SetMemberStringAs
		( context, L"strProhibition", ltctx.pwszProhibition ) ;
}

// SGLLetteringContext へ変換
//////////////////////////////////////////////////////////////////////////////
void RSLetteringContextClass::ConvertFromObject
	( RSContext& context,
		SGLLetteringContext& ltctx,
		SString& strProhibition, RSObject * pObj )
{
	RSSmartPtr	pRect( pObj->GetMemberAs( context, L"rectWritable" ), &context ) ;
	if ( pRect != NULL )
	{
		ltctx.rectWritable.left =
			(int32_t) pRect->GetMemberIntegerAs
						( context, L"x", ltctx.rectWritable.left ) ;
		ltctx.rectWritable.top =
			(int32_t) pRect->GetMemberIntegerAs
						( context, L"y", ltctx.rectWritable.top ) ;
		ltctx.rectWritable.SetWidth
			( (int32_t) pRect->GetMemberIntegerAs
						( context, L"w", ltctx.rectWritable.GetWidth() ) ) ;
		ltctx.rectWritable.SetHeight
			( (int32_t) pRect->GetMemberIntegerAs
						( context, L"h", ltctx.rectWritable.GetHeight() ) ) ;
	}
	ltctx.typeAlignment = (uint16_t)
		pObj->GetMemberIntegerAs
			( context, L"typeAlignment", ltctx.typeAlignment ) ;
	ltctx.flagVertical = (uint16_t)
		pObj->GetMemberIntegerAs
			( context, L"flagVertical", ltctx.flagVertical ) ;
	ltctx.pitchChar = (int32_t)
		pObj->GetMemberIntegerAs
			( context, L"pitchChar", ltctx.pitchChar ) ;
	ltctx.offsetChar = (int32_t)
		pObj->GetMemberIntegerAs
			( context, L"offsetChar", ltctx.offsetChar ) ;
	ltctx.scalePitch = (int32_t)
		pObj->GetMemberIntegerAs
			( context, L"scalePitch", ltctx.scalePitch ) ;
	ltctx.pitchTab = (int32_t)
		pObj->GetMemberIntegerAs
			( context, L"pitchTab", ltctx.pitchTab ) ;
	ltctx.pitchLine = (int32_t)
		pObj->GetMemberIntegerAs
			( context, L"pitchLine", ltctx.pitchLine ) ;
	ltctx.widthIndent = (int32_t)
		pObj->GetMemberIntegerAs
			( context, L"widthIndent", ltctx.widthIndent ) ;
	ltctx.minHyphening = (uint32_t)
		pObj->GetMemberIntegerAs
			( context, L"minHyphening", ltctx.minHyphening ) ;
	ltctx.maxProhibition = (uint32_t)
		pObj->GetMemberIntegerAs
			( context, L"maxProhibition", ltctx.maxProhibition ) ;
	strProhibition =
		pObj->GetMemberStringAs
			( context, L"strProhibition", ltctx.pwszProhibition ) ;
	ltctx.pwszProhibition = strProhibition ;
}


//////////////////////////////////////////////////////////////////////////////
// LetteringDecoration クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSLetteringDecorationClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSLetteringDecorationClass::RSLetteringDecorationClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSLetteringDecorationClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSGenericObject( this ) ;
	//
	m_pPrototype->CreateMemberIntegerAs( context, L"nFlags", 0 ) ;
	m_pPrototype->CreateMemberIntegerAs( context, L"rgbaBody", 0 ) ;
	m_pPrototype->CreateMemberIntegerAs( context, L"widthBorder", 0 ) ;
	m_pPrototype->CreateMemberIntegerAs( context, L"rgbaBorder", 0 ) ;
	m_pPrototype->CreateMemberIntegerAs( context, L"widthBorder2", 0 ) ;
	m_pPrototype->CreateMemberIntegerAs( context, L"rgbaBorder2", 0 ) ;
	m_pPrototype->CreateMemberIntegerAs( context, L"rgbaShadow", 0 ) ;
	context.ReleaseObjectRef
		( m_pPrototype->CreateMemberAs
			( context, L"ptShadow", context.new_ObjectPointer( L"Point" ) ) ) ;
	//
	CreateMemberIntegerAs
		( context, L"flagShadow", SGLLetterer::flagShadow, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"flagBorder", SGLLetterer::flagBorder, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"flagBorder2", SGLLetterer::flagBorder2, modifierConst ) ;
}

// SGLLetterer::Decoration から変換
//////////////////////////////////////////////////////////////////////////////
void RSLetteringDecorationClass::ConvertToObject
	( RSContext& context, RSObject * pObj,
		const SakuraGL::SGLLetterer::Decoration& ltdec )
{
	pObj->SetMemberIntegerAs( context, L"nFlags", ltdec.nFlags ) ;
	pObj->SetMemberIntegerAs( context, L"rgbaBody", ltdec.rgbaBody ) ;
	pObj->SetMemberIntegerAs( context, L"widthBorder", ltdec.widthBorder ) ;
	pObj->SetMemberIntegerAs( context, L"rgbaBorder", ltdec.rgbaBorder ) ;
	pObj->SetMemberIntegerAs( context, L"widthBorder2", ltdec.widthBorder2 ) ;
	pObj->SetMemberIntegerAs( context, L"rgbaBorder2", ltdec.rgbaBorder2 ) ;
	pObj->SetMemberIntegerAs( context, L"rgbaShadow", ltdec.rgbaShadow ) ;
	//
	RSSmartPtr	pPoint( pObj->GetMemberAs( context, L"ptShadow" ), &context ) ;
	if ( pPoint != NULL )
	{
		pPoint->SetMemberIntegerAs( context, L"x", ltdec.ptShadow.x ) ;
		pPoint->SetMemberIntegerAs( context, L"y", ltdec.ptShadow.y ) ;
	}
}

// SGLLetterer::Decoration へ変換
//////////////////////////////////////////////////////////////////////////////
void RSLetteringDecorationClass::ConvertFromObject
	( RSContext& context,
		SakuraGL::SGLLetterer::Decoration& ltdec, RSObject * pObj )
{
	ltdec.nFlags = (uint32_t)
		pObj->GetMemberIntegerAs( context, L"nFlags", ltdec.nFlags ) ;
	ltdec.rgbaBody = (uint32_t)
		pObj->GetMemberIntegerAs( context, L"rgbaBody", ltdec.rgbaBody ) ;
	ltdec.widthBorder = (uint32_t)
		pObj->GetMemberIntegerAs( context, L"widthBorder", ltdec.widthBorder ) ;
	ltdec.rgbaBorder = (uint32_t)
		pObj->GetMemberIntegerAs( context, L"rgbaBorder", ltdec.rgbaBorder ) ;
	ltdec.widthBorder2 = (uint32_t)
		pObj->GetMemberIntegerAs( context, L"widthBorder2", ltdec.widthBorder2 ) ;
	ltdec.rgbaBorder2 = (uint32_t)
		pObj->GetMemberIntegerAs( context, L"rgbaBorder2", ltdec.rgbaBorder2 ) ;
	ltdec.rgbaShadow = (uint32_t)
		pObj->GetMemberIntegerAs( context, L"rgbaShadow", ltdec.rgbaShadow ) ;
	//
	RSSmartPtr	pPoint( pObj->GetMemberAs( context, L"ptShadow" ), &context ) ;
	if ( pPoint != NULL )
	{
		ltdec.ptShadow.x = (int32_t)
			pPoint->GetMemberIntegerAs( context, L"x", ltdec.ptShadow.x ) ;
		ltdec.ptShadow.y = (int32_t)
			pPoint->GetMemberIntegerAs( context, L"y", ltdec.ptShadow.y ) ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// RectangleSprite.RectStyle クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( Rosetta::RSRectangleSpriteClass::RectStyleClass, RSStructuredPointerClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSRectangleSpriteClass::RectStyleClass::RectStyleClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSStructuredPointerClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSRectangleSpriteClass::RectStyleClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSStructuredPointer( this, NULL ) ;
	//
	AddArrayMemberAs
		( context, L"size", context.GetClassAs( L"Size" ) ) ;
	AddArrayMemberAs
		( context, L"color", context.GetBasicTypeClass(RSCodeControl::wiInt) ) ;
	//
	SGLSpriteRectangle::RectStyle *	pStyle =
		(SGLSpriteRectangle::RectStyle*)
			m_bufInit.GetArray( sizeof(SGLSpriteRectangle::RectStyle) ) ;
	*pStyle = SGLSpriteRectangle::RectStyle() ;
	m_bufInit.FinishArray() ;
}


//////////////////////////////////////////////////////////////////////////////
// RectangleSprite クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSRectangleSpriteClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSRectangleSpriteClass::RSRectangleSpriteClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// メンバ初期設定
//////////////////////////////////////////////////////////////////////////////
void RSRectangleSpriteClass::Initialize( RSContext& context )
{
	AddSuperClass( context, context.GetClassAs( L"Sprite" ) ) ;
	OverrideVirtuals( context ) ;
	m_flagInitialized = true ;
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSRectangleSpriteClass::OverrideVirtuals( RSContext& context )
{
	RectStyleClass *
		pStyleClass =
			new RectStyleClass( context.GetVM()->GetClassClass() ) ;
	pStyleClass->Initialize( context ) ;
	pStyleClass->FinishClass( context ) ;
	context.ReleaseObjectRef
		( CreateMemberAs( context, L"RectStyle", pStyleClass ) ) ;
	//
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( NULL, this ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"",
				NULL, &RSRectangleSpriteClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr,
			L"getRectStyle", L"RectangleSprite.RectStyle", L"",
			NULL, &RSRectangleSpriteClass::method_getRectStyle, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr,
			L"setRectStyle", NULL, L"RectangleSprite.RectStyle style",
			NULL, &RSRectangleSpriteClass::method_setRectStyle, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr,
			L"setRectangleSize", NULL, L"int w, int h",
			NULL, &RSRectangleSpriteClass::method_setRectangleSize, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr,
			L"setRectangleColor", NULL, L"int color",
			NULL, &RSRectangleSpriteClass::method_setRectangleColor, NULL ) ;
}

// ネイティブ型テスト
//////////////////////////////////////////////////////////////////////////////
bool RSRectangleSpriteClass::IsNativeObjectOf( ESLObject * pObj ) const
{
	return	(ESLTypeCast<SGLSpriteRectangle>(pObj) != nullptr) ;
}

// this オブジェクトを取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLSpriteRectangle *
	RSRectangleSpriteClass::GetThisSpriteRectangle( RSContext& context, RSObject* pThis )
{
	SGLSpriteRectangle *	pSprite = NULL ;
	RSNativeObject *		pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj != NULL )
	{
		pSprite = ESLTypeCast<SGLSpriteRectangle>( pNativeObj->GetObject() ) ;
	}
	if ( pSprite == NULL )
	{
		context.ThrowExceptionError( L"this が RectangleSprite ではありません" ) ;
	}
	return	pSprite ;
}

// void <init>()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRectangleSpriteClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj == NULL )
	{
		context.ThrowExceptionError
			( L"RectangleSprite.<init> の this が RectangleSprite ではありません" ) ;
		return	NULL ;
	}
	pNativeObj->SetObject( new SGLSpriteRectangle ) ;
	return	NULL ;
}

// RectangleSprite.RectStyle getRectStyle() ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRectangleSpriteClass::method_getRectStyle
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteRectangle *	pRect = GetThisSpriteRectangle( context, pThis ) ;
	if ( pRect == NULL )
	{
		return	NULL ;
	}
	const SGLSpriteRectangle::RectStyle &	style = pRect->GetRectStyle() ;
	RSObject *	pObj =
		context.new_StructuredPointer( L"RectangleSprite.RectStyle" ) ;
	if ( pObj == NULL )
	{
		return	NULL ;
	}
	RSTypedArrayPointer *
		pTypedPtr = ESLTypeCast<RSTypedArrayPointer>( pObj ) ;
	if ( pTypedPtr != NULL )
	{
		SGLSpriteRectangle::RectStyle *	pStyle =
			(SGLSpriteRectangle::RectStyle*)
				pTypedPtr->GetPointer
					( sizeof(SGLSpriteRectangle::RectStyle) ) ;
		if ( pStyle != NULL )
		{
			*pStyle = style ;
		}
	}
	return	pObj ;
}

// void setRectStyle( RectangleSprite.RectStyle style ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRectangleSpriteClass::method_setRectStyle
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteRectangle *	pRect = GetThisSpriteRectangle( context, pThis ) ;
	if ( pRect == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLSpriteRectangle::RectStyle *	pStyle =
		(SGLSpriteRectangle::RectStyle*)
			arg.PointerAt( 0, sizeof(SGLSpriteRectangle::RectStyle) ) ;
	if ( pStyle != NULL )
	{
		pRect->SetRectangleStyle( *pStyle ) ;
	}
	return	NULL ;
}

// void setRectangleSize( int w, int h )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRectangleSpriteClass::method_setRectangleSize
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteRectangle *	pRect = GetThisSpriteRectangle( context, pThis ) ;
	if ( pRect == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pRect->SetRectangleSize( (ssize_t) arg.IntAt(0), (ssize_t) arg.IntAt(1) ) ;
	return	NULL ;
}

// void setRectangleColor( int color )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRectangleSpriteClass::method_setRectangleColor
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteRectangle *	pRect = GetThisSpriteRectangle( context, pThis ) ;
	if ( pRect == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLPalette	color = (uint32_t) arg.IntAt( 0 ) ;
	pRect->SetRectangleColor( color ) ;
	return	NULL ;
}


//////////////////////////////////////////////////////////////////////////////
// TextSprite.TextStyle クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSTextSpriteClass::TextStyleClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSTextSpriteClass::TextStyleClass::TextStyleClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSTextSpriteClass::TextStyleClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSGenericObject( this ) ;
	//
	m_pPrototype->CreateMemberIntegerAs( context, L"boxAlign", 0 ) ;
	context.ReleaseObjectRef
		( m_pPrototype->CreateMemberAs
			( context, L"font",
				context.new_ObjectPointer( L"FontStyle" ) ) ) ;
	context.ReleaseObjectRef
		( m_pPrototype->CreateMemberAs
			( context, L"context",
				context.new_ObjectPointer( L"LetteringContext" ) ) ) ;
	context.ReleaseObjectRef
		( m_pPrototype->CreateMemberAs
			( context, L"decoration",
				context.new_ObjectPointer( L"LetteringDecoration" ) ) ) ;
}

// SGLSpriteText::TextStyle から変換
//////////////////////////////////////////////////////////////////////////////
void RSTextSpriteClass::TextStyleClass::ConvertToObject
	( RSContext& context, RSObject * pObj,
		const SakuraGL::SGLSpriteText::TextStyle& style )
{
	pObj->SetMemberIntegerAs( context, L"boxAlign", style.boxAlign ) ;
	//
	RSSmartPtr	pFont( pObj->GetMemberAs( context, L"font" ), &context ) ;
	if ( pFont != NULL )
	{
		RSFontStyleClass::ConvertToObject( context, pFont, style.font ) ;
	}
	RSSmartPtr	pContext( pObj->GetMemberAs( context, L"context" ), &context ) ;
	if ( pContext != NULL )
	{
		RSLetteringContextClass::ConvertToObject
						( context, pContext, style.context ) ;
	}
	RSSmartPtr	pDeco( pObj->GetMemberAs( context, L"decoration" ), &context ) ;
	if ( pDeco != NULL )
	{
		RSLetteringDecorationClass::ConvertToObject
						( context, pDeco, style.decoration ) ;
	}
}

// SGLSpriteText::TextStyle へ変換
//////////////////////////////////////////////////////////////////////////////
void RSTextSpriteClass::TextStyleClass::ConvertFromObject
	( RSContext& context,
		SakuraGL::SGLSpriteText::TextStyle& style,
		SSystem::SString& strFace,
		SSystem::SString& strProhibition, RSObject * pObj )
{
	style.boxAlign = (uint32_t)
		pObj->GetMemberIntegerAs( context, L"boxAlign", style.boxAlign ) ;
	//
	RSSmartPtr	pFont( pObj->GetMemberAs( context, L"font" ), &context ) ;
	if ( pFont != NULL )
	{
		RSFontStyleClass::ConvertFromObject
				( context, style.font, strFace, pFont ) ;
	}
	RSSmartPtr	pContext( pObj->GetMemberAs( context, L"context" ), &context ) ;
	if ( pContext != NULL )
	{
		RSLetteringContextClass::ConvertFromObject
				( context, style.context, strProhibition, pContext ) ;
	}
	RSSmartPtr	pDeco( pObj->GetMemberAs( context, L"decoration" ), &context ) ;
	if ( pDeco != NULL )
	{
		RSLetteringDecorationClass::ConvertFromObject
						( context, style.decoration, pDeco ) ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// TextSprite クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSTextSpriteClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSTextSpriteClass::RSTextSpriteClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// メンバ初期設定
//////////////////////////////////////////////////////////////////////////////
void RSTextSpriteClass::Initialize( RSContext& context )
{
	AddSuperClass( context, context.GetClassAs( L"Sprite" ) ) ;
	OverrideVirtuals( context ) ;
	m_flagInitialized = true ;
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSTextSpriteClass::OverrideVirtuals( RSContext& context )
{
	TextStyleClass *
		pStyleClass =
			new TextStyleClass( context.GetVM()->GetClassClass() ) ;
	pStyleClass->Initialize( context ) ;
	pStyleClass->FinishClass( context ) ;
	context.ReleaseObjectRef
		( CreateMemberAs( context, L"TextStyle", pStyleClass ) ) ;
	//
	CreateMemberIntegerAs
		( context, L"alignBoxLeft", SGLSpriteText::alignBoxLeft, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"alignBoxRight", SGLSpriteText::alignBoxRight, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"alignBoxCenter", SGLSpriteText::alignBoxCenter, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"alignBoxTop", SGLSpriteText::alignBoxTop, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"alignBoxBottom", SGLSpriteText::alignBoxBottom, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"alignBoxVCenter", SGLSpriteText::alignBoxVCenter, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"alignBoxHorzMask", SGLSpriteText::alignBoxHorzMask, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"alignBoxVertMask", SGLSpriteText::alignBoxVertMask, modifierConst ) ;
	//
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( NULL, this ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"",
				NULL, &RSTextSpriteClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getTextStyle", L"TextSprite.TextStyle", L"",
				NULL, &RSTextSpriteClass::method_getTextStyle,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setTextStyle", NULL, L"TextSprite.TextStyle style",
				NULL, &RSTextSpriteClass::method_setTextStyle, NULL ) ;
}

// ネイティブ型テスト
//////////////////////////////////////////////////////////////////////////////
bool RSTextSpriteClass::IsNativeObjectOf( ESLObject * pObj ) const
{
	return	(ESLTypeCast<RSTextSpriteClass>(pObj) != nullptr) ;
}

// this オブジェクトを取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLSpriteText *
	RSTextSpriteClass::GetThisSpriteText( RSContext& context, RSObject* pThis )
{
	SGLSpriteText *		pSprite = NULL ;
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj != NULL )
	{
		pSprite = ESLTypeCast<SGLSpriteText>( pNativeObj->GetObject() ) ;
	}
	if ( pSprite == NULL )
	{
		context.ThrowExceptionError( L"this が TextSprite ではありません" ) ;
	}
	return	pSprite ;
}

// void <init>()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSTextSpriteClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj == NULL )
	{
		context.ThrowExceptionError
			( L"TextSprite.<init> の this が TextSprite ではありません" ) ;
		return	NULL ;
	}
	pNativeObj->SetObject( new SGLSpriteText ) ;
	return	NULL ;
}

// TextSprite.TextStyle getTextStyle() ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSTextSpriteClass::method_getTextStyle
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteText *	pText = GetThisSpriteText( context, pThis ) ;
	if ( pText == NULL )
	{
		return	NULL ;
	}
	RSClass *	pTextSpriteClass = context.GetClassAs( L"TextSprite" ) ;
	RSObject *	pObjStyle = NULL ;
	if ( pTextSpriteClass != NULL )
	{
		RSSmartPtr	pObjClass
			( pTextSpriteClass->GetMemberAs
					( context, L"TextStyle" ), &context ) ;
		RSClass *	pClass = ESLTypeCast<RSClass>( pObjClass.Ptr() ) ;
		if ( pClass != NULL )
		{
			RSSmartPtr	pArg( context.new_Array(), &context ) ;
			pObjStyle = pClass->NewInstance( context, pArg ) ;
		}
	}
	if ( pObjStyle == NULL )
	{
		context.ThrowExceptionError
			( L"TextSprite.TextStyle インスタンスの生成に失敗しました" ) ;
		return	NULL ;
	}
	TextStyleClass::ConvertToObject
		( context, pObjStyle, pText->GetTextStyle() ) ;
	return	pObjStyle ;
}

// void setTextStyle( TextSprite.TextStyle style ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSTextSpriteClass::method_setTextStyle
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteText *	pText = GetThisSpriteText( context, pThis ) ;
	if ( pText == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSObject *	pStyle = arg.ObjectAt( 0 ) ;
	if ( pStyle != NULL )
	{
		SGLSpriteText::TextStyle	style ;
		SString	strFace, strProhib ;
		TextStyleClass::ConvertFromObject
			( context, style, strFace, strProhib, pStyle ) ;
		pText->SetTextStyle( style ) ;
	}
	return	NULL ;
}


//////////////////////////////////////////////////////////////////////////////
// ButtonSprite.ButtonStyle クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( Rosetta::RSButtonSpriteClass::ButtonStyleClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSButtonSpriteClass::ButtonStyleClass::ButtonStyleClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSButtonSpriteClass::ButtonStyleClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSGenericObject( this ) ;
	//
	m_pPrototype->CreateMemberIntegerAs( context, L"typeButton", 0 ) ;
	context.ReleaseObjectRef
		( m_pPrototype->CreateMemberAs
			( context, L"flagHitRect", context.new_Boolean( false ) ) ) ;
	m_pPrototype->CreateMemberIntegerAs( context, L"maskStatus", 0 ) ;
	//
	context.ReleaseObjectRef
		( m_pPrototype->CreateMemberAs
			( context, L"imgMask",
				context.new_Pointer( NULL, context.GetClassAs( L"Image" ) ) ) ) ;
	context.ReleaseObjectRef
		( m_pPrototype->CreateMemberAs
			( context, L"imgButton",
				context.new_Array
					( SGLSpriteButton::statusCount,
						context.GetClassAs( L"Image" ) ) ) ) ;
	context.ReleaseObjectRef
		( m_pPrototype->CreateMemberAs
			( context, L"textStyle",
				context.new_Array
					( SGLSpriteButton::statusCount,
						context.GetClassAs( L"TextSprite.TextStyle" ) ) ) ) ;
	context.ReleaseObjectRef
		( m_pPrototype->CreateMemberAs
			( context, L"rgbaBackColor",
				context.new_Array
					( SGLSpriteButton::statusCount,
						context.GetBasicTypeClass( RSCodeControl::wiInt ) ) ) ) ;
}

// SGLSpriteText::TextStyle から変換
//////////////////////////////////////////////////////////////////////////////
void RSButtonSpriteClass::ButtonStyleClass::ConvertToObject
	( RSContext& context, RSObject * pObj,
		const SakuraGL::SGLSpriteButton::ButtonStyle& style )
{
	pObj->SetMemberIntegerAs( context, L"typeButton", style.typeButton ) ;
	context.ReleaseObjectRef
		( pObj->SetMemberAs
			( context, L"flagHitRect",
				context.new_Boolean( style.flagHitRect ) ) ) ;
	pObj->SetMemberIntegerAs( context, L"maskStatus", style.maskStatus ) ;
	//
	if ( style.imgdscMask.pImage != NULL )
	{
		context.ReleaseObjectRef
			( pObj->SetMemberAs( context, L"imgMask",
				RSImageClass::NewImage
					( context, style.imgdscMask.pImage->
								NewReference( style.imgdscMask.pRect ) ) ) ) ;
	}
	else
	{
		context.ReleaseObjectRef
			( pObj->SetMemberAs( context, L"imgMask", NULL ) ) ;
	}
	//
	RSSmartPtr
		sptrImages( pObj->GetMemberAs( context, L"imgButton" ), &context ) ;
	RSSmartPtr
		sptrTextStyles( pObj->GetMemberAs( context, L"textStyle" ), &context ) ;
	RSSmartPtr
		sptrBackColors( pObj->GetMemberAs( context, L"rgbaBackColor" ), &context ) ;
	//
	for ( int i = 0; i < SGLSpriteButton::statusCount; i ++ )
	{
		if ( sptrImages != NULL )
		{
			if ( style.imgdscButton[i].pImage != NULL )
			{
				context.ReleaseObjectRef
					( sptrImages->SetElementAt
						( context, i, RSImageClass::NewImage
							( context, style.imgdscButton[i].pImage->
								NewReference( style.imgdscButton[i].pRect ) ) ) ) ;
			}
			else
			{
				context.ReleaseObjectRef
					( sptrImages->SetElementAt( context, i, NULL ) ) ;
			}
		}
		if ( sptrTextStyles != NULL )
		{
			RSObject *	pStyle = context.new_Object( L"TextSprite.TextStyle" ) ;
			ESLAssert( pStyle != NULL ) ;
			RSTextSpriteClass::TextStyleClass::ConvertToObject
						( context, pStyle, style.textStyle[i] ) ;
			context.ReleaseObjectRef
				( sptrImages->SetElementAt( context, i, pStyle ) ) ;
		}
		if ( sptrBackColors != NULL )
		{
			sptrBackColors->SetElementIntegerAt
				( context, i, style.rgbaBackColor[i].ui32 ) ;
		}
	}
}

// SGLSpriteText::TextStyle へ変換
//////////////////////////////////////////////////////////////////////////////
void RSButtonSpriteClass::ButtonStyleClass::ConvertFromObject
	( RSContext& context,
		SakuraGL::SGLSpriteButton::ButtonStyle& style,
		SSystem::SString * pstrFontFaces,
		SSystem::SString * pstrProhibitions, RSObject * pObj )
{
	style.typeButton =
		(SGLSpriteButton::ButtonType)
			pObj->GetMemberIntegerAs
				( context, L"typeButton", style.typeButton ) ;
	style.flagHitRect =
		(pObj->GetMemberIntegerAs
			( context, L"flagHitRect", style.flagHitRect ) != 0) ;
	style.maskStatus =
		(uint32_t) pObj->GetMemberIntegerAs
					( context, L"maskStatus", style.maskStatus ) ;
	//
	RSSmartPtr	sptrMask( pObj->GetMemberAs( context, L"imgMask" ), &context ) ;
	style.imgdscMask.pImage =
			RSImageClass::ImageFromObject( context, sptrMask ) ;
	style.imgdscMask.pRect = NULL ;
	//
	RSSmartPtr
		sptrImages( pObj->GetMemberAs( context, L"imgButton" ), &context ) ;
	RSSmartPtr
		sptrTextStyles( pObj->GetMemberAs( context, L"textStyle" ), &context ) ;
	RSSmartPtr
		sptrBackColors( pObj->GetMemberAs( context, L"rgbaBackColor" ), &context ) ;
	//
	for ( int i = 0; i < SGLSpriteButton::statusCount; i ++ )
	{
		if ( sptrImages != NULL )
		{
			RSSmartPtr	sptrImage( sptrImages->GetElementAt( context, i ) ) ;
			style.imgdscButton[i].pImage =
					RSImageClass::ImageFromObject( context, sptrImage ) ;
			style.imgdscButton[i].pRect = NULL ;
		}
		if ( sptrTextStyles != NULL )
		{
			RSSmartPtr	sptrStyle( sptrTextStyles->GetElementAt( context, i ) ) ;
			if ( sptrStyle != NULL )
			{
				RSTextSpriteClass::TextStyleClass::ConvertFromObject
					( context, style.textStyle[i],
						pstrFontFaces[i], pstrProhibitions[i], sptrStyle ) ;
			}
		}
		if ( sptrBackColors != NULL )
		{
			style.rgbaBackColor[i] =
				(uint32_t) sptrBackColors->GetElementIntegerAt( context, i ) ;
		}
	}
}


//////////////////////////////////////////////////////////////////////////////
// ButtonSprite.Listener オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2
	( Rosetta::RSButtonSpriteClass::Listener,
				RSGenericObject, SGLSpriteButtonListener )

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
RSButtonSpriteClass::Listener::~Listener( void )
{
}

// 複製（参照の複製を含む）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSButtonSpriteClass::Listener::DuplicateObject( RSContext& context ) const
{
	ESLAssert( m_pClass != NULL ) ;
	Listener *	pListener = new Listener( context.GetVM(), m_pClass ) ;
	pListener->m_gcmMembers.DuplicateAllMembers
				( context, m_gcmMembers.m_members ) ;
	return	pListener ;
}

// 複製（実体も可能な限り複製）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSButtonSpriteClass::Listener::CloneObject( RSContext& context ) const
{
	ESLAssert( m_pClass != NULL ) ;
	Listener *	pListener = new Listener( context.GetVM(), m_pClass ) ;
	pListener->m_gcmMembers.CloneAllMembers
				( context, m_gcmMembers.m_members ) ;
	return	pListener ;
}

// ボタンが押された
//////////////////////////////////////////////////////////////////////////////
bool RSButtonSpriteClass::Listener::OnButtonPushed( SakuraGL::SGLSpriteButton& button, bool fRepeat )
{
	ESLAssert( m_vm != NULL ) ;
	RSContext	context( m_vm ) ;
	RSFunctionObject *	pFunc =
			m_pClass->GetVirtualMemberAs( context, L"onButtonPushed" ) ;
	bool	fResult = false ;
	if ( pFunc != NULL )
	{
		RSObject *	pArgs[2] ;
		pArgs[0] = new RSNativeObject
					( &button, context.GetClassAs( L"ButtonSprite" ) ) ;
		pArgs[1] = context.new_Boolean( fRepeat ) ;
		//
		RSSmartPtr	pResult
			( context.CallFunction( *pFunc, this, &pArgs[0], 2, false ) ) ;
		if ( pResult != NULL )
		{
			fResult = pResult->AsBoolean() ;
		}
		//
		RSObject::ReleaseRef( pArgs[0] ) ;
		RSObject::ReleaseRef( pArgs[1] ) ;
		RSObject::ReleaseRef( pFunc ) ;
	}
	return	fResult ;
}

// ボタンのステータスが変化した
//////////////////////////////////////////////////////////////////////////////
bool RSButtonSpriteClass::Listener::OnChangedButtonStatus( SakuraGL::SGLSpriteButton& button )
{
	ESLAssert( m_vm != NULL ) ;
	RSContext	context( m_vm ) ;
	RSFunctionObject *	pFunc =
			m_pClass->GetVirtualMemberAs( context, L"onChangedButtonStatus" ) ;
	bool	fResult = false ;
	if ( pFunc != NULL )
	{
		RSObject *	pArgs[1] ;
		pArgs[0] = new RSNativeObject
					( &button, context.GetClassAs( L"ButtonSprite" ) ) ;
		//
		RSSmartPtr	pResult
			( context.CallFunction( *pFunc, this, &pArgs[0], 1, false ) ) ;
		if ( pResult != NULL )
		{
			fResult = pResult->AsBoolean() ;
		}
		//
		RSObject::ReleaseRef( pArgs[0] ) ;
		RSObject::ReleaseRef( pFunc ) ;
	}
	return	fResult ;
}

// ドラッグが開始した
//////////////////////////////////////////////////////////////////////////////
void RSButtonSpriteClass::Listener::OnBeginDrag
	( SakuraGL::SGLSpriteButton& button, double xOffset, double yOffset )
{
	ESLAssert( m_vm != NULL ) ;
	RSContext	context( m_vm ) ;
	RSFunctionObject *	pFunc =
			m_pClass->GetVirtualMemberAs( context, L"onBeginDrag" ) ;
	if ( pFunc != NULL )
	{
		RSObject *	pArgs[3] ;
		pArgs[0] = new RSNativeObject
					( &button, context.GetClassAs( L"ButtonSprite" ) ) ;
		pArgs[1] = context.new_Number( xOffset ) ;
		pArgs[2] = context.new_Number( yOffset ) ;
		//
		RSSmartPtr	pResult
			( context.CallFunction( *pFunc, this, &pArgs[0], 3, false ) ) ;
		//
		RSObject::ReleaseRef( pArgs[0] ) ;
		RSObject::ReleaseRef( pArgs[1] ) ;
		RSObject::ReleaseRef( pArgs[2] ) ;
		RSObject::ReleaseRef( pFunc ) ;
	}
}


//////////////////////////////////////////////////////////////////////////////
// ButtonSprite.Listener クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO
	( Rosetta::RSButtonSpriteClass::ListenerClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSButtonSpriteClass::ListenerClass::ListenerClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSButtonSpriteClass::ListenerClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new Listener( context.GetVM(), this ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"onButtonPushed",
			L"boolean", L"ButtonSprite button, boolean fRepeat",
			NULL, &ListenerClass::method_onButtonPushed, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"onChangedButtonStatus",
			L"boolean", L"ButtonSprite button",
			NULL, &ListenerClass::method_onChangedButtonStatus, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"onBeginDrag",
			L"boolean", L"ButtonSprite button, double xOffset, double yOffset",
			NULL, &ListenerClass::method_onBeginDrag, NULL ) ;
}

// boolean onButtonPushed
//	( ButtonSprite button, boolean fRepeat )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSButtonSpriteClass::ListenerClass::method_onButtonPushed
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	return	context.new_Boolean( false ) ;
}

// boolean onChangedButtonStatus( ButtonSprite button )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSButtonSpriteClass::ListenerClass::method_onChangedButtonStatus
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	return	context.new_Boolean( false ) ;
}

// void onBeginDrag
//	( ButtonSprite button, double xOffset, double yOffset )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSButtonSpriteClass::ListenerClass::method_onBeginDrag
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	return	NULL ;
}



//////////////////////////////////////////////////////////////////////////////
// ButtonSprite クラス
/////////////////////////////////////////////////////////////////////////////

// クラス情報
/////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSButtonSpriteClass, RSClass )

// 構築関数
/////////////////////////////////////////////////////////////////////////////
RSButtonSpriteClass::RSButtonSpriteClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// メンバ初期設定
/////////////////////////////////////////////////////////////////////////////
void RSButtonSpriteClass::Initialize( RSContext& context )
{
	AddSuperClass( context, context.GetClassAs( L"Sprite" ) ) ;
	OverrideVirtuals( context ) ;
	m_flagInitialized = true ;
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
/////////////////////////////////////////////////////////////////////////////
void RSButtonSpriteClass::OverrideVirtuals( RSContext& context )
{
	ButtonStyleClass *
		pStyleClass =
			new ButtonStyleClass( context.GetVM()->GetClassClass() ) ;
	pStyleClass->Initialize( context ) ;
	pStyleClass->FinishClass( context ) ;
	context.ReleaseObjectRef
		( CreateMemberAs( context, L"ButtonStyle", pStyleClass ) ) ;
	//
	ListenerClass *
		pListenerClass =
			new ListenerClass( context.GetVM()->GetClassClass() ) ;
	pListenerClass->Initialize( context ) ;
	pListenerClass->FinishClass( context ) ;
	context.ReleaseObjectRef
		( CreateMemberAs( context, L"Listener", pListenerClass ) ) ;
	//
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( NULL, this ) ;
	//
	CreateMemberIntegerAs
		( context, L"typeNormal", SGLSpriteButton::typeNormal, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"typeCheck", SGLSpriteButton::typeCheck, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"typeRadio", SGLSpriteButton::typeRadio, modifierConst ) ;
	//
	CreateMemberIntegerAs
		( context, L"statusInvalid", SGLSpriteButton::statusInvalid, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"statusNormal", SGLSpriteButton::statusNormal, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"statusFocus", SGLSpriteButton::statusFocus, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"statusPushed", SGLSpriteButton::statusPushed, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"statusPushedFocus", SGLSpriteButton::statusPushedFocus, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"statusActive", SGLSpriteButton::statusActive, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"statusPushedActive", SGLSpriteButton::statusPushedActive, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"statusDisabled", SGLSpriteButton::statusDisabled, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"statusPushDisabled", SGLSpriteButton::statusPushDisabled, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"statusCount", SGLSpriteButton::statusCount, modifierConst ) ;
	//
	CreateMemberIntegerAs
		( context, L"flagNormal", SGLSpriteButton::flagNormal, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"flagFocus", SGLSpriteButton::flagFocus, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"flagPushed", SGLSpriteButton::flagPushed, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"flagPushedFocus", SGLSpriteButton::flagPushedFocus, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"flagActive", SGLSpriteButton::flagActive, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"flagPushedActive", SGLSpriteButton::flagPushedActive, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"flagDisabled", SGLSpriteButton::flagDisabled, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"flagPushDisabled", SGLSpriteButton::flagPushDisabled, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"flagNormal4Buttons", SGLSpriteButton::flagNormal4Buttons, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"flagAllStatus", SGLSpriteButton::flagAllStatus, modifierConst ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"",
				NULL, &RSButtonSpriteClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"attachButtonListener",
			NULL, L"ButtonSprite.Listener listener",
			NULL, &RSButtonSpriteClass::method_attachButtonListener, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"attachSoundEffect",
			NULL, L"AudioPlayer seFocus, AudioPlayer sePushed",
			NULL, &RSButtonSpriteClass::method_attachSoundEffect, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"createSimpleButton", L"boolean",
				L"Image[] pImages, boolean flagHitRect, Image pHitMask,"
				L"Size sizeButton, TextSprite.TextStyle textStyle,"
				L"int[] pTextColors, int[] pBackColors,"
				L"int maskStatus = ButtonSprite.flagNormal4Buttons,"
				L"int typeButton = ButtonSprite.typeNormal",
			NULL, &RSButtonSpriteClass::method_createSimpleButton, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"createSimpleImageButton", L"boolean",
				L"Image[] pImages, boolean flagHitRect = false,"
				L"Image pHitMask = null,"
				L"int maskStatus = ButtonSprite.flagNormal4Buttons,"
				L"int typeButton = ButtonSprite.typeNormal",
			NULL, &RSButtonSpriteClass::method_createSimpleImageButton, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"createSimpleTextButton", L"boolean",
				L"TextSprite.TextStyle textStyle,"
				L"Size sizeTextExt,"
				L"int[] pTextColors, int[] pBackColors,"
				L"int maskStatus = ButtonSprite.flagNormal4Buttons,"
				L"int typeButton = ButtonSprite.typeNormal",
			NULL, &RSButtonSpriteClass::method_createSimpleTextButton, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"createSimpleRectButton", L"boolean",
				L"Size sizeRect, int[] pTextColors,"
				L"int maskStatus = ButtonSprite.flagNormal4Buttons,"
				L"int typeButton = ButtonSprite.typeNormal",
			NULL, &RSButtonSpriteClass::method_createSimpleRectButton, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getButtonStyle",
			L"ButtonSprite.ButtonStyle", L"",
			NULL, &RSButtonSpriteClass::method_getButtonStyle,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setButtonStyle",
			NULL, L"ButtonSprite.ButtonStyle style",
			NULL, &RSButtonSpriteClass::method_setButtonStyle, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setButtonSize",
			NULL, L"int w, int h",
			NULL, &RSButtonSpriteClass::method_setButtonSize, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getButtonStatus",
			L"int", L"",
			NULL, &RSButtonSpriteClass::method_getButtonStatus,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setButtonStatus",
			NULL, L"int status",
			NULL, &RSButtonSpriteClass::method_setButtonStatus, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"newButtonImageReference",
			L"Image", L"int status",
			NULL, &RSButtonSpriteClass::method_newButtonImageReference,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setButtonRepeat",
			NULL, L"boolean flagRepeat, int msecBefore, int msecInterval",
			NULL, &RSButtonSpriteClass::method_setButtonRepeat, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setRightClickNotify",
			NULL, L"boolean flagRightClick, long nRightClickParam",
			NULL, &RSButtonSpriteClass::method_setRightClickNotify, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setStatusNotification",
			NULL, L"boolean flagNotify, long nNotifyParam",
			NULL, &RSButtonSpriteClass::method_setStatusNotification, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"enableDrag",
			NULL, L"boolean flagDraggable, int nThreshold = 8",
			NULL, &RSButtonSpriteClass::method_enableDrag, NULL ) ;
}

// ネイティブ型テスト
/////////////////////////////////////////////////////////////////////////////
bool RSButtonSpriteClass::IsNativeObjectOf( ESLObject * pObj ) const
{
	return	(ESLTypeCast<SGLSpriteButton>(pObj) != nullptr) ;
}

// this オブジェクトを取得
/////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLSpriteButton *
	RSButtonSpriteClass::GetThisSpriteButton( RSContext& context, RSObject* pThis )
{
	SGLSpriteButton *	pSprite = NULL ;
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj != NULL )
	{
		pSprite = ESLTypeCast<SGLSpriteButton>( pNativeObj->GetObject() ) ;
	}
	if ( pSprite == NULL )
	{
		context.ThrowExceptionError( L"this が ButtonSprite ではありません" ) ;
	}
	return	pSprite ;
}

// void <init>()
/////////////////////////////////////////////////////////////////////////////
RSObject * RSButtonSpriteClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj == NULL )
	{
		context.ThrowExceptionError
			( L"ButtonSprite.<init> の this が ButtonSprite ではありません" ) ;
		return	NULL ;
	}
	pNativeObj->SetObject( (SGLSprite*) new SGLSpriteButton ) ;
	return	NULL ;
}

// void attachButtonListener( ButtonSprite.Listener listener )
/////////////////////////////////////////////////////////////////////////////
RSObject * RSButtonSpriteClass::method_attachButtonListener
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteButton *	pButton = GetThisSpriteButton( context, pThis ) ;
	if ( pButton == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	Listener *	pListener = ESLTypeCast<Listener>( arg.NativeObjectAt( 0 ) ) ;
	pButton->AttachButtonListener( pListener ) ;
	return	NULL ;
}

// void attachSoundEffect( AudioPlayer seFocus, AudioPlayer sePushed )
/////////////////////////////////////////////////////////////////////////////
RSObject * RSButtonSpriteClass::method_attachSoundEffect
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteButton *	pButton = GetThisSpriteButton( context, pThis ) ;
	if ( pButton == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLAudioPlayerInterface *
		pFocusAudio = ESLTypeCast<SGLAudioPlayerInterface>
									( arg.NativeObjectAt( 0 ) ) ;
	SGLAudioPlayerInterface *
		pPushedAudio = ESLTypeCast<SGLAudioPlayerInterface>
									( arg.NativeObjectAt( 1 ) ) ;
	//
	pButton->AttachSoundEffect( pFocusAudio, pPushedAudio ) ;
	//
	return	NULL ;
}

// boolean createSimpleButton
//	( Image[] pImages, boolean flagHitRect, Image pHitMask,
//		Size sizeButton, TextSprite.TextStyle textStyle,
//		int[] pTextColors, int[] pBackColors,
//		int maskStatus = ButtonSprite.flagNormal4Buttons,
//		int typeButton = ButtonSprite.typeNormal )
/////////////////////////////////////////////////////////////////////////////
RSObject * RSButtonSpriteClass::method_createSimpleButton
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteButton *	pButton = GetThisSpriteButton( context, pThis ) ;
	if ( pButton == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SPointerArray<SGLImageObject>	lstImages ;
	SArray<SGLPalette>				lstTextColors ;
	SArray<SGLPalette>				lstBackColors ;
	//
	RSSmartPtr	sptrImages = arg.ObjectAt( 0 ) ;
	if ( sptrImages != NULL )
	{
		size_t	nCount = sptrImages->GetElementCount() ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			RSSmartPtr	sptrImage =
				sptrImages->GetElementAt( context, (int) i ) ;
			SGLImageObject *	pImage =
				RSImageClass::ImageFromObject( context, sptrImage ) ;
			lstImages.SetAt( i, pImage ) ;
		}
	}
	RSSmartPtr			sptrHitMask = arg.ObjectAt( 2 ) ;
	SGLImageObject *	pHitMask = NULL ;
	if ( sptrHitMask != NULL )
	{
		pHitMask = RSImageClass::ImageFromObject( context, sptrHitMask ) ;
	}
	SGLSize *	pSizeButton =
			(SGLSize*) arg.PointerAt( 3, sizeof(SGLSize) ) ;
	if ( pSizeButton == NULL )
	{
		context.ThrowExceptionError
			( L"sizeButton 引数が null です", L"NullPointerException" ) ;
		return	NULL ;
	}
	RSSmartPtr	sptrTextStyle = arg.ObjectAt( 4 ) ;
	if ( sptrTextStyle == NULL )
	{
		context.ThrowExceptionError
			( L"textStyle 引数が null です", L"NullPointerException" ) ;
		return	NULL ;
	}
	SGLSpriteText::TextStyle	textStyle ;
	SString	strFace, strProhibition ;
	RSTextSpriteClass::TextStyleClass::ConvertFromObject
		( context, textStyle, strFace, strProhibition, sptrTextStyle ) ;
	//
	RSSmartPtr	sptrTextColors = arg.ObjectAt( 5 ) ;
	if ( sptrTextColors != NULL )
	{
		size_t	nCount = sptrTextColors->GetElementCount() ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			lstTextColors.SetAt
				( i, SGLPalette( (uint32_t)
					sptrTextColors->
						GetElementIntegerAt( context, (int) i ) ) ) ;
		}
	}
	RSSmartPtr	sptrBackColors = arg.ObjectAt( 6 ) ;
	if ( sptrBackColors != NULL )
	{
		size_t	nCount = sptrBackColors->GetElementCount() ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			lstBackColors.SetAt
				( i, SGLPalette( (uint32_t)
					sptrBackColors->
						GetElementIntegerAt( context, (int) i ) ) ) ;
		}
	}
	//
	return	context.new_Integer
		( pButton->CreateSimpleButton
			( lstImages.GetArray(), arg.BooleanAt(1), pHitMask,
				*pSizeButton, textStyle,
				lstTextColors.GetConstArray(), lstBackColors.GetConstArray(),
				(uint32_t) arg.IntAt( 7 ),
				(SGLSpriteButton::ButtonType) arg.IntAt( 8 ) ) ) ;
}

// boolean createSimpleImageButton
//	( Image[] pImages, boolean flagHitRect = false,
//		Image pHitMask = null,
//		int maskStatus = ButtonSprite.flagNormal4Buttons,
//		int typeButton = ButtonSprite.typeNormal )
/////////////////////////////////////////////////////////////////////////////
RSObject * RSButtonSpriteClass::method_createSimpleImageButton
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteButton *	pButton = GetThisSpriteButton( context, pThis ) ;
	if ( pButton == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SPointerArray<SGLImageObject>	lstImages ;
	//
	RSSmartPtr	sptrImages = arg.ObjectAt( 0 ) ;
	if ( sptrImages != NULL )
	{
		size_t	nCount = sptrImages->GetElementCount() ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			RSSmartPtr	sptrImage =
				sptrImages->GetElementAt( context, (int) i ) ;
			SGLImageObject *	pImage =
				RSImageClass::ImageFromObject( context, sptrImage ) ;
			lstImages.SetAt( i, pImage ) ;
		}
	}
	RSSmartPtr			sptrHitMask = arg.ObjectAt( 2 ) ;
	SGLImageObject *	pHitMask = NULL ;
	if ( sptrHitMask != NULL )
	{
		pHitMask = RSImageClass::ImageFromObject( context, sptrHitMask ) ;
	}
	//
	return	context.new_Integer
		( pButton->CreateSimpleImageButton
			( lstImages.GetArray(), arg.BooleanAt(1), pHitMask,
				(uint32_t) arg.IntAt( 3 ),
				(SGLSpriteButton::ButtonType) arg.IntAt( 4 ) ) ) ;
}

// boolean createSimpleTextButton
//	( TextSprite.TextStyle textStyle,
//		Size sizeTextExt,
//		int[] pTextColors, int[] pBackColors,
//		int maskStatus = ButtonSprite.flagNormal4Buttons,
//		int typeButton = ButtonSprite.typeNormal )
/////////////////////////////////////////////////////////////////////////////
RSObject * RSButtonSpriteClass::method_createSimpleTextButton
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteButton *	pButton = GetThisSpriteButton( context, pThis ) ;
	if ( pButton == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SArray<SGLPalette>	lstTextColors ;
	SArray<SGLPalette>	lstBackColors ;
	//
	RSSmartPtr	sptrTextStyle = arg.ObjectAt( 0 ) ;
	if ( sptrTextStyle == NULL )
	{
		context.ThrowExceptionError
			( L"textStyle 引数が null です", L"NullPointerException" ) ;
		return	NULL ;
	}
	SGLSpriteText::TextStyle	textStyle ;
	SString	strFace, strProhibition ;
	RSTextSpriteClass::TextStyleClass::ConvertFromObject
		( context, textStyle, strFace, strProhibition, sptrTextStyle ) ;
	//
	SGLSize *	pSizeTextExt =
			(SGLSize*) arg.PointerAt( 1, sizeof(SGLSize) ) ;
	if ( pSizeTextExt == NULL )
	{
		context.ThrowExceptionError
			( L"sizeTextExt 引数が null です", L"NullPointerException" ) ;
		return	NULL ;
	}
	//
	RSSmartPtr	sptrTextColors = arg.ObjectAt( 2 ) ;
	if ( sptrTextColors != NULL )
	{
		size_t	nCount = sptrTextColors->GetElementCount() ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			lstTextColors.SetAt
				( i, SGLPalette( (uint32_t)
					sptrTextColors->
						GetElementIntegerAt( context, (int) i ) ) ) ;
		}
	}
	RSSmartPtr	sptrBackColors = arg.ObjectAt( 3 ) ;
	if ( sptrBackColors != NULL )
	{
		size_t	nCount = sptrBackColors->GetElementCount() ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			lstBackColors.SetAt
				( i, SGLPalette( (uint32_t)
					sptrBackColors->
						GetElementIntegerAt( context, (int) i ) ) ) ;
		}
	}
	//
	return	context.new_Integer
		( pButton->CreateSimpleTextButton
			( textStyle, *pSizeTextExt,
				lstTextColors.GetConstArray(),
				lstBackColors.GetConstArray(),
				(uint32_t) arg.IntAt( 4 ),
				(SGLSpriteButton::ButtonType) arg.IntAt( 5 ) ) ) ;
}

// boolean createSimpleRectButton
//	( Size sizeRect, int[] pTextColors,
//		int maskStatus = ButtonSprite.flagNormal4Buttons,
//		int typeButton = ButtonSprite.typeNormal )
/////////////////////////////////////////////////////////////////////////////
RSObject * RSButtonSpriteClass::method_createSimpleRectButton
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteButton *	pButton = GetThisSpriteButton( context, pThis ) ;
	if ( pButton == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SArray<SGLPalette>	lstColors ;
	//
	SGLSize *	pSizeRect =
			(SGLSize*) arg.PointerAt( 0, sizeof(SGLSize) ) ;
	if ( pSizeRect == NULL )
	{
		context.ThrowExceptionError
			( L"sizeRect 引数が null です", L"NullPointerException" ) ;
		return	NULL ;
	}
	//
	RSSmartPtr	sptrColors = arg.ObjectAt( 1 ) ;
	if ( sptrColors != NULL )
	{
		size_t	nCount = sptrColors->GetElementCount() ;
		for ( size_t i = 0; i < nCount; i ++ )
		{
			lstColors.SetAt
				( i, SGLPalette( (uint32_t)
					sptrColors->
						GetElementIntegerAt( context, (int) i ) ) ) ;
		}
	}
	//
	return	context.new_Integer
		( pButton->CreateSimpleRectButton
			( *pSizeRect, lstColors.GetConstArray(),
				(uint32_t) arg.IntAt( 2 ),
				(SGLSpriteButton::ButtonType) arg.IntAt( 3 ) ) ) ;
}

// const ButtonSprite.ButtonStyle getButtonStyle()
/////////////////////////////////////////////////////////////////////////////
RSObject * RSButtonSpriteClass::method_getButtonStyle
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteButton *	pButton = GetThisSpriteButton( context, pThis ) ;
	if ( pButton == NULL )
	{
		return	NULL ;
	}
	RSObject *	pObjStyle = context.new_Object( L"ButtonSprite.ButtonStyle" ) ;
	if ( pObjStyle == NULL )
	{
		return	pObjStyle ;
	}
	ButtonStyleClass::ConvertToObject
		( context, pObjStyle, pButton->GetButtonStyle() ) ;
	return	pObjStyle ;
}

// void setButtonStyle( ButtonSprite.ButtonStyle style )
/////////////////////////////////////////////////////////////////////////////
RSObject * RSButtonSpriteClass::method_setButtonStyle
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteButton *	pButton = GetThisSpriteButton( context, pThis ) ;
	if ( pButton == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSSmartPtr	sptrStyle = arg.ObjectAt( 0 ) ;
	if ( sptrStyle == NULL )
	{
		return	NULL ;
	}
	SGLSpriteButton::ButtonStyle	style ;
	SString	strFontFace[SGLSpriteButton::statusCount] ;
	SString	strProhibition[SGLSpriteButton::statusCount] ;
	//
	ButtonStyleClass::ConvertFromObject
		( context, style, strFontFace, strProhibition, sptrStyle ) ;
	//
	pButton->SetButtonStyle( style ) ;
	//
	return	NULL ;
}

// void setButtonSize( int w, int h )
/////////////////////////////////////////////////////////////////////////////
RSObject * RSButtonSpriteClass::method_setButtonSize
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteButton *	pButton = GetThisSpriteButton( context, pThis ) ;
	if ( pButton == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pButton->SetButtonSize( SGLSize( arg.IntAt(0), arg.IntAt(1) ) ) ;
	return	NULL ;
}

// const int getButtonStatus()
/////////////////////////////////////////////////////////////////////////////
RSObject * RSButtonSpriteClass::method_getButtonStatus
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteButton *	pButton = GetThisSpriteButton( context, pThis ) ;
	if ( pButton == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( pButton->GetButtonStatus() ) ;
}

// void setButtonStatus( int status )
/////////////////////////////////////////////////////////////////////////////
RSObject * RSButtonSpriteClass::method_setButtonStatus
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteButton *	pButton = GetThisSpriteButton( context, pThis ) ;
	if ( pButton == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pButton->SetButtonStatus
		( (SGLSpriteButton::ButtonStatus) arg.IntAt(0) ) ;
	return	NULL ;
}

// const Image newButtonImageReference( int status )
/////////////////////////////////////////////////////////////////////////////
RSObject * RSButtonSpriteClass::method_newButtonImageReference
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteButton *	pButton = GetThisSpriteButton( context, pThis ) ;
	if ( pButton == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLImageObject *	pImage =
		pButton->NewButtonImageReference
			( (SGLSpriteButton::ButtonStatus) arg.IntAt(0) ) ;
	if ( pImage == NULL )
	{
		return	NULL ;
	}
	return	RSImageClass::NewImage( context, pImage ) ;
}

// void setButtonRepeat
//	( boolean flagRepeat, int msecBefore, int msecInterval )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSButtonSpriteClass::method_setButtonRepeat
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteButton *	pButton = GetThisSpriteButton( context, pThis ) ;
	if ( pButton == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pButton->SetButtonRepeat
		( arg.BooleanAt(0),
			(uint32_t) arg.IntAt(1), (uint32_t) arg.IntAt(2) ) ;
	return	NULL ;
}

// void setRightClickNotify( boolean flagRightClick, long nRightClickParam )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSButtonSpriteClass::method_setRightClickNotify
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteButton *	pButton = GetThisSpriteButton( context, pThis ) ;
	if ( pButton == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pButton->SetRightClickNotify
		( arg.BooleanAt(0), arg.LongAt(1) ) ;
	return	NULL ;
}

// void setStatusNotification( boolean flagNotify, long nNotifyParam )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSButtonSpriteClass::method_setStatusNotification
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteButton *	pButton = GetThisSpriteButton( context, pThis ) ;
	if ( pButton == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pButton->SetStatusNotification
		( arg.BooleanAt(0), arg.LongAt(1) ) ;
	return	NULL ;
}

// void enableDrag( boolean flagDraggable, int nThreshold = 8 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSButtonSpriteClass::method_enableDrag
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteButton *	pButton = GetThisSpriteButton( context, pThis ) ;
	if ( pButton == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pButton->EnableDrag
		( arg.BooleanAt(0), (int32_t) arg.IntAt(1, 8) ) ;
	return	NULL ;
}


//////////////////////////////////////////////////////////////////////////////
// MessageSprite.ViewActionStyle クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSMessageSpriteClass::ViewActionStyleClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSMessageSpriteClass::ViewActionStyleClass::ViewActionStyleClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSMessageSpriteClass::ViewActionStyleClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSGenericObject( this ) ;
	//
	m_pPrototype->CreateMemberIntegerAs( context, L"msecPerChar", 0 ) ;
	m_pPrototype->CreateMemberIntegerAs( context, L"msecFade", 0 ) ;
	context.ReleaseObjectRef
		( m_pPrototype->CreateMemberAs
			( context, L"vMove",
				context.new_ObjectPointer( L"Vector2D" ) ) ) ;
	context.ReleaseObjectRef
		( m_pPrototype->CreateMemberAs
			( context, L"vZoom",
				context.new_ObjectPointer( L"Vector2D" ) ) ) ;
	m_pPrototype->CreateMemberNumberAs( context, L"zRotation", 0 ) ;
}

// SGLSpriteMessage::ViewActionStyle から変換
//////////////////////////////////////////////////////////////////////////////
void RSMessageSpriteClass::ViewActionStyleClass::ConvertToObject
	( RSContext& context, RSObject * pObj,
		const SakuraGL::SGLSpriteMessage::ViewActionStyle& style )
{
	pObj->SetMemberIntegerAs( context, L"msecPerChar", style.msecPerChar ) ;
	pObj->SetMemberIntegerAs( context, L"msecFade", style.msecFade ) ;
	//
	RSSmartPtr	pMove( pObj->GetMemberAs( context, L"vMove" ), &context ) ;
	if ( pMove != NULL )
	{
		pMove->SetMemberNumberAs( context, L"x", style.vMove.x ) ;
		pMove->SetMemberNumberAs( context, L"y", style.vMove.y ) ;
	}
	RSSmartPtr	pZoom( pObj->GetMemberAs( context, L"vZoom" ), &context ) ;
	if ( pZoom != NULL )
	{
		pZoom->SetMemberNumberAs( context, L"x", style.vZoom.x ) ;
		pZoom->SetMemberNumberAs( context, L"y", style.vZoom.y ) ;
	}
	pObj->SetMemberNumberAs( context, L"zRotation", style.zRotation ) ;
}

// SGLSpriteMessage::ViewActionStyle へ変換
//////////////////////////////////////////////////////////////////////////////
void RSMessageSpriteClass::ViewActionStyleClass::ConvertFromObject
	( RSContext& context,
		SakuraGL::SGLSpriteMessage::ViewActionStyle& style,
		RSObject * pObj )
{
	style.msecPerChar = (uint32_t)
		pObj->GetMemberIntegerAs( context, L"msecPerChar", style.msecPerChar ) ;
	style.msecFade = (uint32_t)
		pObj->GetMemberIntegerAs( context, L"msecFade", style.msecFade ) ;
	//
	RSSmartPtr	pMove( pObj->GetMemberAs( context, L"vMove" ), &context ) ;
	if ( pMove != NULL )
	{
		style.vMove.x = (float32_t)
				pMove->GetMemberNumberAs( context, L"x", style.vMove.x ) ;
		style.vMove.y = (float32_t)
				pMove->GetMemberNumberAs( context, L"y", style.vMove.y ) ;
	}
	RSSmartPtr	pZoom( pObj->GetMemberAs( context, L"vZoom" ), &context ) ;
	if ( pZoom != NULL )
	{
		style.vZoom.x = (float32_t)
				pZoom->GetMemberNumberAs( context, L"x", style.vZoom.x ) ;
		style.vZoom.y = (float32_t)
				pZoom->GetMemberNumberAs( context, L"y", style.vZoom.y ) ;
	}
	style.zRotation = (float32_t)
		pObj->GetMemberNumberAs( context, L"zRotation", style.zRotation ) ;
}



//////////////////////////////////////////////////////////////////////////////
// MessageSprite クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSMessageSpriteClass, RSClass )
// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSMessageSpriteClass::RSMessageSpriteClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// メンバ初期設定
//////////////////////////////////////////////////////////////////////////////
void RSMessageSpriteClass::Initialize( RSContext& context )
{
	AddSuperClass( context, context.GetClassAs( L"Sprite" ) ) ;
	OverrideVirtuals( context ) ;
	m_flagInitialized = true ;
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSMessageSpriteClass::OverrideVirtuals( RSContext& context )
{
	ViewActionStyleClass *
		pStyleClass =
			new ViewActionStyleClass( context.GetVM()->GetClassClass() ) ;
	pStyleClass->Initialize( context ) ;
	pStyleClass->FinishClass( context ) ;
	context.ReleaseObjectRef
		( CreateMemberAs( context, L"ViewActionStyle", pStyleClass ) ) ;
	//
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( NULL, this ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"",
				NULL, &RSMessageSpriteClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getTextStyle", L"TextSprite.TextStyle", L"",
				NULL, &RSMessageSpriteClass::method_getTextStyle,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setTextStyle", NULL, L"TextSprite.TextStyle style",
				NULL, &RSMessageSpriteClass::method_setTextStyle, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getRubyFontStyle", L"FontStyle", L"",
				NULL, &RSMessageSpriteClass::method_getRubyFontStyle,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setRubyFontStyle", NULL, L"FontStyle style",
				NULL, &RSMessageSpriteClass::method_setRubyFontStyle, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getViewActionStyle", L"MessageSprite.ViewActionStyle", L"",
				NULL, &RSMessageSpriteClass::method_getViewActionStyle,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setViewActionStyle", NULL, L"MessageSprite.ViewActionStyle style",
				NULL, &RSMessageSpriteClass::method_setViewActionStyle, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getAttachedSkin", L"SkinManager", L"",
				NULL, &RSMessageSpriteClass::method_getAttachedSkin,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"attachSkin", NULL, L"SkinManager skin",
				NULL, &RSMessageSpriteClass::method_attachSkin, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"clearMessage", NULL, L"",
				NULL, &RSMessageSpriteClass::method_clearMessage, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"flushMessage", NULL, L"",
				NULL, &RSMessageSpriteClass::method_flushMessage, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"isMessagePending", L"boolean", L"",
				NULL, &RSMessageSpriteClass::method_isMessagePending,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setMessageSpeedRatio", NULL, L"int fxSpeedRatio",
				NULL, &RSMessageSpriteClass::method_setMessageSpeedRatio, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"addMessageText", NULL, L"String strText",
				NULL, &RSMessageSpriteClass::method_addMessageText, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"addMessageXML", NULL, L"String strXML",
				NULL, &RSMessageSpriteClass::method_addMessageXML, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getNextMessagePoint", L"Point", L"",
				NULL, &RSMessageSpriteClass::method_getNextMessagePoint,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getCircumscribedRect", L"boolean", L"Rect rectMsg",
				NULL, &RSMessageSpriteClass::method_getCircumscribedRect,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getMessageCharacterCount", L"int", L"",
				NULL, &RSMessageSpriteClass::method_getMessageCharacterCount,
				NULL, RSFunctionPrototype::flagConstant ) ;
}

// ネイティブ型テスト
//////////////////////////////////////////////////////////////////////////////
bool RSMessageSpriteClass::IsNativeObjectOf( ESLObject * pObj ) const
{
	return	(ESLTypeCast<SGLSpriteMessage>(pObj) != nullptr) ;
}

// this オブジェクトを取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLSpriteMessage *
	RSMessageSpriteClass::GetThisSpriteMessage( RSContext& context, RSObject* pThis )
{
	SGLSpriteMessage *	pSprite = NULL ;
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj != NULL )
	{
		pSprite = ESLTypeCast<SGLSpriteMessage>( pNativeObj->GetObject() ) ;
	}
	if ( pSprite == NULL )
	{
		context.ThrowExceptionError( L"this が MessageSprite ではありません" ) ;
	}
	return	pSprite ;
}

// void <init>()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMessageSpriteClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj == NULL )
	{
		context.ThrowExceptionError
			( L"MessageSprite.<init> の this が MessageSprite ではありません" ) ;
		return	NULL ;
	}
	pNativeObj->SetObject( (SGLSprite*) new SGLSpriteMessage ) ;
	return	NULL ;
}

// TextSprite.TextStyle getTextStyle() ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMessageSpriteClass::method_getTextStyle
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteMessage *	pMsg = GetThisSpriteMessage( context, pThis ) ;
	if ( pMsg == NULL )
	{
		return	NULL ;
	}
	RSClass *	pTextSpriteClass = context.GetClassAs( L"TextSprite" ) ;
	RSObject *	pObjStyle = NULL ;
	if ( pTextSpriteClass != NULL )
	{
		RSSmartPtr	pObjClass
			( pTextSpriteClass->GetMemberAs
					( context, L"TextStyle" ), &context ) ;
		RSClass *	pClass = ESLTypeCast<RSClass>( pObjClass.Ptr() ) ;
		if ( pClass != NULL )
		{
			RSSmartPtr	pArg( context.new_Array(), &context ) ;
			pObjStyle = pClass->NewInstance( context, pArg ) ;
		}
	}
	if ( pObjStyle == NULL )
	{
		context.ThrowExceptionError
			( L"TextSprite.TextStyle インスタンスの生成に失敗しました" ) ;
		return	NULL ;
	}
	RSTextSpriteClass::TextStyleClass::ConvertToObject
		( context, pObjStyle, pMsg->GetTextStyle() ) ;
	return	pObjStyle ;
}

// void setTextStyle( TextSprite.TextStyle style ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMessageSpriteClass::method_setTextStyle
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteMessage *	pMsg = GetThisSpriteMessage( context, pThis ) ;
	if ( pMsg == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSObject *	pStyle = arg.ObjectAt( 0 ) ;
	if ( pStyle != NULL )
	{
		SGLSpriteText::TextStyle	style ;
		SString	strFace, strProhib ;
		RSTextSpriteClass::TextStyleClass::ConvertFromObject
			( context, style, strFace, strProhib, pStyle ) ;
		pMsg->SetTextStyle( style ) ;
	}
	return	NULL ;
}

// FontStyle getRubyFontStyle() ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMessageSpriteClass::method_getRubyFontStyle
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteMessage *	pMsg = GetThisSpriteMessage( context, pThis ) ;
	if ( pMsg == NULL )
	{
		return	NULL ;
	}
	RSObject *	pObjStyle = context.new_Object( L"FontStyle" ) ;
	if ( pObjStyle == NULL )
	{
		context.ThrowExceptionError
			( L"FontStyle インスタンスの生成に失敗しました" ) ;
		return	NULL ;
	}
	RSFontStyleClass::ConvertToObject
		( context, pObjStyle, pMsg->GetRubyFontStyle() ) ;
	return	pObjStyle ;
}

// void setRubyFontStyle( FontStyle style ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMessageSpriteClass::method_setRubyFontStyle
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteMessage *	pMsg = GetThisSpriteMessage( context, pThis ) ;
	if ( pMsg == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSObject *	pStyle = arg.ObjectAt( 0 ) ;
	if ( pStyle != NULL )
	{
		SGLFontStyle	style ;
		SString			strFace ;
		RSFontStyleClass::ConvertFromObject
			( context, style, strFace, pStyle ) ;
		pMsg->SetRubyFontStyle( style ) ;
	}
	return	NULL ;
}

// MessageSprite.ViewActionStyle getViewActionStyle() ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMessageSpriteClass::method_getViewActionStyle
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteMessage *	pMsg = GetThisSpriteMessage( context, pThis ) ;
	if ( pMsg == NULL )
	{
		return	NULL ;
	}
	RSClass *	pMsgSpriteClass = context.GetClassAs( L"MessageSprite" ) ;
	RSObject *	pObjStyle = NULL ;
	if ( pMsgSpriteClass != NULL )
	{
		RSSmartPtr	pObjClass
			( pMsgSpriteClass->GetMemberAs
					( context, L"ViewActionStyle" ), &context ) ;
		RSClass *	pClass = ESLTypeCast<RSClass>( pObjClass.Ptr() ) ;
		if ( pClass != NULL )
		{
			RSSmartPtr	pArg( context.new_Array(), &context ) ;
			pObjStyle = pClass->NewInstance( context, pArg ) ;
		}
	}
	if ( pObjStyle == NULL )
	{
		context.ThrowExceptionError
			( L"MessageSprite.ViewActionStyle インスタンスの生成に失敗しました" ) ;
		return	NULL ;
	}
	ViewActionStyleClass::ConvertToObject
		( context, pObjStyle, pMsg->GetViewActionStyle() ) ;
	return	pObjStyle ;
}

// void setViewActionStyle( MessageSprite.ViewActionStyle style ) ;
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMessageSpriteClass::method_setViewActionStyle
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteMessage *	pMsg = GetThisSpriteMessage( context, pThis ) ;
	if ( pMsg == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSObject *	pStyle = arg.ObjectAt( 0 ) ;
	if ( pStyle != NULL )
	{
		SGLSpriteMessage::ViewActionStyle	style ;
		ViewActionStyleClass::ConvertFromObject( context, style, pStyle ) ;
		pMsg->SetViewActionStyle( style ) ;
	}
	return	NULL ;
}

// SkinManager getAttachedSkin()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMessageSpriteClass::method_getAttachedSkin
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteMessage *	pMsg = GetThisSpriteMessage( context, pThis ) ;
	if ( pMsg == NULL )
	{
		return	NULL ;
	}
	SGLSkinManager *	pSkin = pMsg->GetAttachedSkin() ;
	if ( pSkin == NULL )
	{
		return	NULL ;
	}
	return	new RSNativeObject
				( (SGLObject*) pSkin, context.GetClassAs( L"SkinManager" ) ) ;
}

// void attachSkin( SkinManager skin )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMessageSpriteClass::method_attachSkin
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteMessage *	pMsg = GetThisSpriteMessage( context, pThis ) ;
	if ( pMsg == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLSkinManager *
		pSkin = ESLTypeCast<SGLSkinManager>( arg.NativeObjectAt( 0 ) ) ;
	pMsg->AttachSkin( pSkin ) ;
	return	NULL ;
}

// void clearMessage()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMessageSpriteClass::method_clearMessage
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteMessage *	pMsg = GetThisSpriteMessage( context, pThis ) ;
	if ( pMsg == NULL )
	{
		return	NULL ;
	}
	pMsg->ClearMessage() ;
	return	NULL ;
}

// void flushMessage()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMessageSpriteClass::method_flushMessage
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteMessage *	pMsg = GetThisSpriteMessage( context, pThis ) ;
	if ( pMsg == NULL )
	{
		return	NULL ;
	}
	pMsg->FlushMessage() ;
	return	NULL ;
}

// boolean isMessagePending()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMessageSpriteClass::method_isMessagePending
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteMessage *	pMsg = GetThisSpriteMessage( context, pThis ) ;
	if ( pMsg == NULL )
	{
		return	NULL ;
	}
	return	context.new_Boolean( pMsg->IsMessagePending() ) ;
}

// void setMessageSpeedRatio( int fxSpeedRatio )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMessageSpriteClass::method_setMessageSpeedRatio
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteMessage *	pMsg = GetThisSpriteMessage( context, pThis ) ;
	if ( pMsg == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pMsg->SetMessageSpeedRatio( (uint32_t) arg.IntAt( 0 ) ) ;
	return	NULL ;
}

// void addMessageText( String strText )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMessageSpriteClass::method_addMessageText
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteMessage *	pMsg = GetThisSpriteMessage( context, pThis ) ;
	if ( pMsg == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pMsg->AddMessageText( arg.StringAt( 0 ) ) ;
	return	NULL ;
}

// void addMessageXML( String strXML )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMessageSpriteClass::method_addMessageXML
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteMessage *	pMsg = GetThisSpriteMessage( context, pThis ) ;
	if ( pMsg == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pMsg->AddMessageXML( arg.StringAt( 0 ) ) ;
	return	NULL ;
}

// Point getNextMessagePoint()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMessageSpriteClass::method_getNextMessagePoint
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteMessage *	pMsg = GetThisSpriteMessage( context, pThis ) ;
	if ( pMsg == NULL )
	{
		return	NULL ;
	}
	RSObject *	pPoint = context.new_Object( L"Point" ) ;
	SGLPoint	ptNext = pMsg->GetNextMessagePoint() ;
	pPoint->SetMemberIntegerAs( context, L"x", ptNext.x ) ;
	pPoint->SetMemberIntegerAs( context, L"y", ptNext.y ) ;
	return	pPoint ;
}

// boolean getCircumscribedRect( Rect rectMsg )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMessageSpriteClass::method_getCircumscribedRect
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteMessage *	pMsg = GetThisSpriteMessage( context, pThis ) ;
	if ( pMsg == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SGLImageRect *	pRectMsg =
		(SGLImageRect*) arg.PointerAt( 0, sizeof(SGLImageRect) ) ;
	if ( pRectMsg == NULL )
	{
		return	context.new_Boolean( false ) ;
	}
	SGLRect	rectMsg ;
	if ( !pMsg->GetCircumscribedRect( rectMsg ) )
	{
		return	context.new_Boolean( false ) ;
	}
	*pRectMsg = rectMsg ;
	return	context.new_Boolean( true ) ;
}

// int getMessageCharacterCount()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMessageSpriteClass::method_getMessageCharacterCount
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteMessage *	pMsg = GetThisSpriteMessage( context, pThis ) ;
	if ( pMsg == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( pMsg->GetMessageCharacterCount() ) ;
}



//////////////////////////////////////////////////////////////////////////////
// MovieSprite クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSMovieSpriteClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSMovieSpriteClass::RSMovieSpriteClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// メンバ初期設定
//////////////////////////////////////////////////////////////////////////////
void RSMovieSpriteClass::Initialize( RSContext& context )
{
	AddSuperClass( context, context.GetClassAs( L"Sprite" ) ) ;
	OverrideVirtuals( context ) ;
	m_flagInitialized = true ;
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSMovieSpriteClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( NULL, this ) ;
	//
	CreateMemberIntegerAs
		( context, L"playDrawDirect",
			SGLSpriteMovie::playDrawDirect, modifierConst ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"",
				NULL, &RSMovieSpriteClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"openMovieFile",
				L"boolean", L"String path",
				NULL, &RSMovieSpriteClass::method_openMovieFile, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"closeMovieFile",
				L"boolean", L"",
				NULL, &RSMovieSpriteClass::method_closeMovieFile, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"playMovie",
				L"boolean", L"long nFlags = 0",
				NULL, &RSMovieSpriteClass::method_playMovie, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"stopMovie",
				L"boolean", L"",
				NULL, &RSMovieSpriteClass::method_stopMovie, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setMovieLoop",
				L"boolean",
				L"boolean flagLoop = true, long nStart = -1, long nEnd = -1",
				NULL, &RSMovieSpriteClass::method_setMovieLoop, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"pauseMovie",
				L"boolean", L"",
				NULL, &RSMovieSpriteClass::method_pauseMovie, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"restartMovie",
				L"boolean", L"",
				NULL, &RSMovieSpriteClass::method_restartMovie, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getVolume",
				L"boolean", L"float[] pVolumes, int nChannels",
				NULL, &RSMovieSpriteClass::method_getVolume,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setVolume",
				L"boolean", L"float[] pVolumes, int nChannels",
				NULL, &RSMovieSpriteClass::method_setVolume, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setVolumeLine",
				L"boolean", L"int iLine",
				NULL, &RSMovieSpriteClass::method_setVolumeLine, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"isMoviePlaying",
				L"boolean", L"",
				NULL, &RSMovieSpriteClass::method_isMoviePlaying,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"isMoviePaused",
				L"boolean", L"",
				NULL, &RSMovieSpriteClass::method_isMoviePaused,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getMovieFrequency",
				L"int", L"",
				NULL, &RSMovieSpriteClass::method_getMovieFrequency,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getMovieLength",
				L"long", L"",
				NULL, &RSMovieSpriteClass::method_getMovieLength,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getMoviePosition",
				L"long", L"",
				NULL, &RSMovieSpriteClass::method_getMoviePosition,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"seekMovie",
				NULL, L"long nPos",
				NULL, &RSMovieSpriteClass::method_seekMovie, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getMovieSize",
				L"Size", L"",
				NULL, &RSMovieSpriteClass::method_getMovieSize,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"hasEndedOfDuration",
				L"boolean", L"",
				NULL, &RSMovieSpriteClass::method_hasEndedOfDuration,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"resetEndOfDuration",
				NULL, L"",
				NULL, &RSMovieSpriteClass::method_resetEndOfDuration,
				NULL, RSFunctionPrototype::flagConstant ) ;
}

// ネイティブ型テスト
//////////////////////////////////////////////////////////////////////////////
bool RSMovieSpriteClass::IsNativeObjectOf( ESLObject * pObj ) const
{
	return	(ESLTypeCast<SGLSpriteMovie>(pObj) != nullptr) ;
}

// this オブジェクトを取得
//////////////////////////////////////////////////////////////////////////////
SakuraGL::SGLSpriteMovie *
	RSMovieSpriteClass::GetThisSpriteMovie( RSContext& context, RSObject* pThis )
{
	SGLSpriteMovie *	pSprite = NULL ;
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj != NULL )
	{
		pSprite = ESLTypeCast<SGLSpriteMovie>( pNativeObj->GetObject() ) ;
	}
	if ( pSprite == NULL )
	{
		context.ThrowExceptionError( L"this が MovieSprite ではありません" ) ;
	}
	return	pSprite ;
}

// void <init>()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMovieSpriteClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj == NULL )
	{
		context.ThrowExceptionError
			( L"MovieSprite.<init> の this が MovieSprite ではありません" ) ;
		return	NULL ;
	}
	pNativeObj->SetObject( (SGLSprite*) new SGLSpriteMovie ) ;
	return	NULL ;
}

// boolean openMovieFile( String path )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMovieSpriteClass::method_openMovieFile
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteMovie *	pMovie = GetThisSpriteMovie( context, pThis ) ;
	if ( pMovie == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Boolean
		( pMovie->OpenMovieFile( arg.StringAt(0) ) == sglErrSuccess ) ;
}

// boolean closeMovieFile()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMovieSpriteClass::method_closeMovieFile
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteMovie *	pMovie = GetThisSpriteMovie( context, pThis ) ;
	if ( pMovie == NULL )
	{
		return	NULL ;
	}
	return	context.new_Boolean
		( pMovie->CloseMovieFile() == sglErrSuccess ) ;
}

// boolean playMovie( long nFlags = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMovieSpriteClass::method_playMovie
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteMovie *	pMovie = GetThisSpriteMovie( context, pThis ) ;
	if ( pMovie == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Boolean
		( pMovie->PlayMovie( (uint64_t) arg.LongAt(0) ) == sglErrSuccess ) ;
}

// boolean stopMovie()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMovieSpriteClass::method_stopMovie
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteMovie *	pMovie = GetThisSpriteMovie( context, pThis ) ;
	if ( pMovie == NULL )
	{
		return	NULL ;
	}
	return	context.new_Boolean
		( pMovie->StopMovie() == sglErrSuccess ) ;
}

// boolean setMovieLoop
//	( boolean flagLoop = true, long nStart = -1, long nEnd = -1 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMovieSpriteClass::method_setMovieLoop
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteMovie *	pMovie = GetThisSpriteMovie( context, pThis ) ;
	if ( pMovie == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Boolean
		( pMovie->SetMovieLoop
			( arg.BooleanAt(0),
				arg.LongAt(1,-1), arg.LongAt(2,-1) ) == sglErrSuccess ) ;
}

// boolean pauseMovie()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMovieSpriteClass::method_pauseMovie
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteMovie *	pMovie = GetThisSpriteMovie( context, pThis ) ;
	if ( pMovie == NULL )
	{
		return	NULL ;
	}
	return	context.new_Boolean
		( pMovie->PauseMovie() == sglErrSuccess ) ;
}

// boolean restartMovie()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMovieSpriteClass::method_restartMovie
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteMovie *	pMovie = GetThisSpriteMovie( context, pThis ) ;
	if ( pMovie == NULL )
	{
		return	NULL ;
	}
	return	context.new_Boolean
		( pMovie->RestartMovie() == sglErrSuccess ) ;
}

// boolean getVolume( float[] pVolumes, int nChannels )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMovieSpriteClass::method_getVolume
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteMovie *	pMovie = GetThisSpriteMovie( context, pThis ) ;
	if ( pMovie == NULL )
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
	SGLError	err = pMovie->GetVolume( pVolumes, nChannels ) ;
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
RSObject * RSMovieSpriteClass::method_setVolume
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteMovie *	pMovie = GetThisSpriteMovie( context, pThis ) ;
	if ( pMovie == NULL )
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
	SGLError	err = pMovie->SetVolume( pVolumes, nChannels ) ;
	aVolumes.FinishArray() ;
	return	context.new_Boolean( err == sglErrSuccess ) ;
}

// void setVolumeLine( int iLine )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMovieSpriteClass::method_setVolumeLine
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteMovie *	pMovie = GetThisSpriteMovie( context, pThis ) ;
	if ( pMovie == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pMovie->SetVolumeLine( (size_t) arg.LongAt( 0 ) ) ;
	return	NULL ;
}

// const boolean isMoviePlaying()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMovieSpriteClass::method_isMoviePlaying
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteMovie *	pMovie = GetThisSpriteMovie( context, pThis ) ;
	if ( pMovie == NULL )
	{
		return	NULL ;
	}
	return	context.new_Boolean( pMovie->IsMoviePlaying() ) ;
}

// const boolean isMoviePaused()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMovieSpriteClass::method_isMoviePaused
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteMovie *	pMovie = GetThisSpriteMovie( context, pThis ) ;
	if ( pMovie == NULL )
	{
		return	NULL ;
	}
	return	context.new_Boolean( pMovie->IsMoviePaused() ) ;
}

// const int getMovieFrequency()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMovieSpriteClass::method_getMovieFrequency
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteMovie *	pMovie = GetThisSpriteMovie( context, pThis ) ;
	if ( pMovie == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( pMovie->GetMovieFrequency() ) ;
}

// const long getMovieLength()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMovieSpriteClass::method_getMovieLength
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteMovie *	pMovie = GetThisSpriteMovie( context, pThis ) ;
	if ( pMovie == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( (uint64_t) pMovie->GetMovieLength() ) ;
}

// const long getMoviePosition()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMovieSpriteClass::method_getMoviePosition
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteMovie *	pMovie = GetThisSpriteMovie( context, pThis ) ;
	if ( pMovie == NULL )
	{
		return	NULL ;
	}
	return	context.new_Integer( (uint64_t) pMovie->GetMoviePosition() ) ;
}

// void seekMovie( long nPos )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMovieSpriteClass::method_seekMovie
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteMovie *	pMovie = GetThisSpriteMovie( context, pThis ) ;
	if ( pMovie == NULL )
	{
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pMovie->SeekMovie( (uint64_t) arg.LongAt(0) ) ;
	return	NULL ;
}

// const Size getMovieSize()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMovieSpriteClass::method_getMovieSize
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteMovie *	pMovie = GetThisSpriteMovie( context, pThis ) ;
	if ( pMovie == NULL )
	{
		return	NULL ;
	}
	SGLSize	sizeMovie = pMovie->GetMovieSize() ;
	//
	RSObject *	pObjSize = context.new_Object( L"Size" ) ;
	if ( pObjSize != NULL )
	{
		pObjSize->SetMemberIntegerAs( context, L"w", sizeMovie.w ) ;
		pObjSize->SetMemberIntegerAs( context, L"h", sizeMovie.h ) ;
	}
	return	pObjSize ;
}

// const boolean hasEndedOfDuration()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMovieSpriteClass::method_hasEndedOfDuration
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteMovie *	pMovie = GetThisSpriteMovie( context, pThis ) ;
	if ( pMovie == NULL )
	{
		return	NULL ;
	}
	return	context.new_Boolean( pMovie->HasEndedOfDuration() ) ;
}

// const void resetEndOfDuration()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSMovieSpriteClass::method_resetEndOfDuration
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SGLSpriteMovie *	pMovie = GetThisSpriteMovie( context, pThis ) ;
	if ( pMovie == NULL )
	{
		return	NULL ;
	}
	pMovie->ResetEndOfDuration() ;
	return	NULL ;
}


//////////////////////////////////////////////////////////////////////////////
// RenderableSprite.Sprite
//////////////////////////////////////////////////////////////////////////////

ESL_IMPLEMENT_CLASS_INFO2
	( Rosetta::RSRenderableSpriteClass::Sprite, SGLSprite, ObjectListener )

RSRenderableSpriteClass::Sprite::Sprite
	( RSVirtualMachine * vm, RSSmartPtr pObject )
		: m_refObject( pObject.Ptr() ), m_errMode(errorIgnore), m_pRectBuf( NULL )
{
	m_context = new RSContext( vm ) ;

	RSStructuredPointer *	pStruct = m_context->new_StructuredPointer( L"Rect" ) ;
	ESLAssert( pStruct != NULL ) ;
	m_pTempRect = pStruct ;
	if ( pStruct != NULL )
	{
		m_pRectBuf =
			(SGLImageRect*) pStruct->GetPointer( sizeof(SGLImageRect) ) ;
	}
}

RSRenderableSpriteClass::Sprite::~Sprite( void )
{
	DetachSyncTimeout( 100 ) ;
}

// オブジェクト削除前
void RSRenderableSpriteClass::Sprite::OnRelease( RSNativeObject * pNObj )
{
	if ( m_refObject.GetReference() != nullptr )
	{
		ESLAssert( m_refObject.GetReference() == pNObj ) ;
		m_refObject = nullptr ;
		//
		DetachSyncTimeout( 200 ) ;
	}
}

void RSRenderableSpriteClass::Sprite::OnDetach( RSNativeObject * pNObj )
{
	m_refObject = nullptr ;
}

// 時間経過処理
void RSRenderableSpriteClass::Sprite::AdvanceTime( uint32_t msecPast )
{
	SGLSprite::AdvanceTime( msecPast ) ;

	RSSmartPtr	pObject( RSObject::AddRef( m_refObject ), m_context ) ;
	if ( pObject == nullptr )
	{
		return ;
	}
	RSArray *	pArg = m_context->new_Array() ;
	pArg->AddIntegerElement( *m_context, msecPast ) ;
	m_context->ReleaseObjectRef
		( m_context->CallMethod
			( pObject, L"onTimer",
				pArg->GetConstArrayPointer(),
				pArg->GetElementCount() ) ) ;
	ClearException() ;
	m_context->ReleaseObjectRef( pArg ) ;
}

// 外接矩形取得
bool RSRenderableSpriteClass::Sprite::GetRectangle( SGLRect& rectExt ) const
{
	bool	flagRect = SGLSprite::GetRectangle( rectExt ) ;

	RSSmartPtr	pObject( RSObject::AddRef( m_refObject ), m_context ) ;
	if ( (pObject == nullptr) || (m_pRectBuf == nullptr) )
	{
		return	flagRect ;
	}
	RSArray *	pArg = m_context->new_Array() ;
	pArg->AddObjectElement( m_pTempRect.AddRef() ) ;
	//
	RSSmartPtr	pResult
		( m_context->CallMethod
			( pObject, L"getRenderRect",
				pArg->GetConstArrayPointer(),
				pArg->GetElementCount() ), m_context ) ;
	//
	ClearException() ;
	m_context->ReleaseObjectRef( pArg ) ;
	//
	if ( (pResult != nullptr) && pResult->AsBoolean() )
	{
		if ( flagRect )
		{
			rectExt |= SGLRect( *m_pRectBuf ) ;
		}
		else
		{
			rectExt = SGLRect( *m_pRectBuf ) ;
			flagRect = true ;
		}
	}
	return	flagRect ;
}

// ヒット判定
bool RSRenderableSpriteClass::Sprite::IsHitSprite( double x, double y ) const
{
	if ( SGLSprite::IsHitSprite( x, y ) )
	{
		return	true ;
	}
	RSSmartPtr	pObject( RSObject::AddRef( m_refObject ), m_context ) ;
	if ( pObject == nullptr )
	{
		return	false ;
	}
	RSArray *	pArg = m_context->new_Array() ;
	pArg->AddNumberElement( *m_context, x ) ;
	pArg->AddNumberElement( *m_context, y ) ;

	RSSmartPtr	pResult
		( m_context->CallMethod
			( pObject, L"isHitTest",
				pArg->GetConstArrayPointer(),
				pArg->GetElementCount() ), m_context ) ;

	ClearException() ;
	m_context->ReleaseObjectRef( pArg ) ;
	//
	return	(pResult != NULL) && pResult->AsBoolean() ;
}

// 子スプライトを描画
void RSRenderableSpriteClass::Sprite::DrawChildren
	( S3DRenderContextInterface& render, SGLSprite::Stereo3DView s3dView ) const
{
	SGLSprite::DrawChildren( render, s3dView ) ;

	RSSmartPtr	pObject( RSObject::AddRef( m_refObject ), m_context ) ;
	if ( pObject == nullptr )
	{
		return ;
	}
	RSArray *	pArg = m_context->new_Array() ;
	pArg->AddObjectElement
		( new RSNativeObject
			( (SGLPaintContextInterface*) &render,
					m_context->GetClassAs( L"RenderContext" ) ) ) ;
	pArg->AddIntegerElement( *m_context, s3dView ) ;
	//
	m_context->ReleaseObjectRef
		( m_context->CallMethod
			( pObject, L"onRender",
				pArg->GetConstArrayPointer(),
				pArg->GetElementCount() ) ) ;

	ClearException() ;
	m_context->ReleaseObjectRef( pArg ) ;
}

// 例外クリア
void RSRenderableSpriteClass::Sprite::ClearException( void ) const
{
	if ( m_errMode == errorStdOut )
	{
		SParserErrorLogger	perrLog ;
		#if	defined(__DEBUG__)
		perrLog.EnableDebugTrace() ;
		#endif
		m_context->OutputExceptionError( perrLog ) ;
		//
		for ( size_t i = 0; i < perrLog.GetErrorCount(); i ++ )
		{
			SParserErrorLogger::ErrorLog *	pLog = perrLog.GetErrorLogAt( i ) ;
			if ( pLog == nullptr )
			{
				continue ;
			}
			cout << L"exception:" << pLog->m_strError << L"\r\n" ;
			cout << pLog->m_strFilePath
					<< L"(" << (int64_t) pLog->m_nLineNum
					<< L"):" << pLog->m_strLine << L"\r\n" ;
		}
	}
	else
	{
	#if	defined(__DEBUG__)
		SParserErrorTracer	perrTracer ;
		m_context->OutputExceptionError( perrTracer ) ;
	#else
		m_context->ClearException() ;
	#endif
	}
}



//////////////////////////////////////////////////////////////////////////////
// RenderableSprite クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSRenderableSpriteClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSRenderableSpriteClass::RSRenderableSpriteClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// メンバ初期設定
//////////////////////////////////////////////////////////////////////////////
void RSRenderableSpriteClass::Initialize( RSContext& context )
{
	AddSuperClass( context, context.GetClassAs( L"Sprite" ) ) ;
	OverrideVirtuals( context ) ;
	m_flagInitialized = true ;
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSRenderableSpriteClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSNativeObject( NULL, this ) ;
	//
	CreateMemberIntegerAs
		( context, L"s3dMonoview", SGLSprite::s3dMonoview, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"s3dRightView", SGLSprite::s3dRightView, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"s3dLeftView", SGLSprite::s3dLeftView, modifierConst ) ;
	//
	CreateMemberIntegerAs
		( context, L"errorIgnore", errorIgnore, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"errorStdOut", errorStdOut, modifierConst ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"",
			NULL, &RSRenderableSpriteClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setErrorMode", NULL, L"int error",
			NULL, &RSRenderableSpriteClass::method_setErrorMode,
			NULL, RSFunctionPrototype::flagAccessProtected ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"onTimer", NULL, L"int msecPast",
			NULL, &RSRenderableSpriteClass::method_onTimer,
			NULL, RSFunctionPrototype::flagAccessProtected ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getRenderRect", L"boolean", L"Rect rect",
				NULL, &RSRenderableSpriteClass::method_getRenderRect,
				NULL, RSFunctionPrototype::flagAccessProtected ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"isHitTest",
			L"boolean", L"double xLocal, double yLocal",
			NULL, &RSRenderableSpriteClass::method_isHitTest,
			NULL, RSFunctionPrototype::flagAccessProtected ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"onRender",
			NULL, L"RenderContext render, int s3dView",
			NULL, &RSRenderableSpriteClass::method_onRender,
			NULL, RSFunctionPrototype::flagAccessProtected ) ;
}

// ネイティブ型テスト
//////////////////////////////////////////////////////////////////////////////
bool RSRenderableSpriteClass::IsNativeObjectOf( ESLObject * pObj ) const
{
	return	(ESLTypeCast<RSRenderableSpriteClass::Sprite>(pObj) != nullptr) ;
}

// this オブジェクトを取得
//////////////////////////////////////////////////////////////////////////////
RSRenderableSpriteClass::Sprite *
	RSRenderableSpriteClass::GetThisSprite( RSContext& context, RSObject* pThis )
{
	return	RSNativeObject::GetNative<Sprite>( pThis ) ;
}

// void <init>()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderableSpriteClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSNativeObject *	pNativeObj = ESLTypeCast<RSNativeObject>( pThis ) ;
	if ( pNativeObj == NULL )
	{
		context.ThrowExceptionError
			( L"RenderableSprite.<init> の this が RenderableSprite ではありません" ) ;
		return	NULL ;
	}
	pNativeObj->SetObject
		( (Sprite::ESLClassPointer)
				new Sprite( context.GetVM(),
					RSSmartPtr( RSObject::AddRef( pNativeObj ) ) ) ) ;
	return	NULL ;
}

// void setErrorMode( int error )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderableSpriteClass::method_setErrorMode
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	Sprite *	pSprite = GetThisSprite( context, pThis ) ;
	if ( pSprite != nullptr )
	{
		RSContext::SArgList	arg( ppArg, count ) ;
		pSprite->m_errMode = (ErrorMode) arg.IntAt(0) ;
	}
	return	nullptr ;
}

// void onTimer( int msecPast )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderableSpriteClass::method_onTimer
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	return	NULL ;
}

// boolean getRenderRect( Rect rect )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderableSpriteClass::method_getRenderRect
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	return	context.new_Boolean( false ) ;
}

// boolean isHitTest( double xLocal, double yLocal )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderableSpriteClass::method_isHitTest
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	return	context.new_Boolean( false ) ;
}

// void onRender( RenderContext render, int s3dView )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSRenderableSpriteClass::method_onRender
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	return	NULL ;
}

