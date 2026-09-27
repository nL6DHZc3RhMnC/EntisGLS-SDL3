
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_SpriteMouseListener.h>


// uint getButtonID( ulong nFlags )
IMPL_LOQUATY_FUNC(EntisGLS4_SpriteMouseListener_getButtonID)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_ARG_ULONG( nFlags ) ;

	LQT_RETURN_UINT( SGLSpriteMouseListener::GetButtonID( nFlags ) ) ;
}

// uint getMouseID( ulong nFlags )
IMPL_LOQUATY_FUNC(EntisGLS4_SpriteMouseListener_getMouseID)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_ARG_ULONG( nFlags ) ;

	LQT_RETURN_UINT( SGLSpriteMouseListener::GetMouseID( nFlags ) ) ;
}

// boolean isFromTouch( ulong nFlags )
IMPL_LOQUATY_FUNC(EntisGLS4_SpriteMouseListener_isFromTouch)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_ARG_ULONG( nFlags ) ;

	LQT_RETURN_BOOL( SGLSpriteMouseListener::IsFromTouch( nFlags ) ) ;
}

// SpriteMouseListener( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_SpriteMouseListener)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ
		( LEntisGLS4_SpriteMouseListener, pThis,
			( new SSmartObject
				( new LSpriteMouseListener
					( _context.VM(), LQT_ARG_OBJECT(0) ) ) ) ) ;

	LQT_RETURN_VOID() ;
}

// uint getPointerCount( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SpriteMouseListener_getPointerCount)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SpriteMouseListener, pThis ) ;
	LSpriteMouseListener *	pListener = pThis->GetRef<LSpriteMouseListener>() ;
	LQT_VERIFY_NULL_PTR( pListener ) ;

	LQT_RETURN_UINT( (LUint) pListener->GetPointerCount() ) ;
}

// int findMouseIndexById( uint idMouse ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SpriteMouseListener_findMouseIndexById)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SpriteMouseListener, pThis ) ;
	LSpriteMouseListener *	pListener = pThis->GetRef<LSpriteMouseListener>() ;
	LQT_VERIFY_NULL_PTR( pListener ) ;
	LQT_FUNC_ARG_UINT( idMouse ) ;

	LQT_RETURN_INT( pListener->FindMouseIndexById( idMouse ) ) ;
}

// boolean getMousePointAt( ulong index, Vector2d* vPos ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SpriteMouseListener_getMousePointAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SpriteMouseListener, pThis ) ;
	LSpriteMouseListener *	pListener = pThis->GetRef<LSpriteMouseListener>() ;
	LQT_VERIFY_NULL_PTR( pListener ) ;
	LQT_FUNC_ARG_ULONG( index ) ;
	LQT_FUNC_ARG_STRUCT( LVector2d, vPos ) ;
	LQT_VERIFY_NULL_PTR( vPos ) ;

	LQT_RETURN_BOOL( pListener->GetMousePointAt( (size_t) index, *vPos ) ) ;
}

// boolean getMousePointAs( uint idMouse, Vector2d* vPos ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SpriteMouseListener_getMousePointAs)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SpriteMouseListener, pThis ) ;
	LSpriteMouseListener *	pListener = pThis->GetRef<LSpriteMouseListener>() ;
	LQT_VERIFY_NULL_PTR( pListener ) ;
	LQT_FUNC_ARG_UINT( idMouse ) ;
	LQT_FUNC_ARG_STRUCT( LVector2d, vPos ) ;
	LQT_VERIFY_NULL_PTR( vPos ) ;

	LBoolean	valRet ;
	ssize_t		index = pListener->FindMouseIndexById( idMouse ) ;
	valRet = (index >= 0) ;
	if ( valRet )
	{
		valRet = pListener->GetMousePointAt( (size_t) index, *vPos ) ;
	}
	LQT_RETURN_BOOL( valRet ) ;
}

// boolean isLButtonDownAt( ulong index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SpriteMouseListener_isLButtonDownAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SpriteMouseListener, pThis ) ;
	LSpriteMouseListener *	pListener = pThis->GetRef<LSpriteMouseListener>() ;
	LQT_VERIFY_NULL_PTR( pListener ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	LQT_RETURN_BOOL( pListener->IsLButtonDownAt( (size_t) index ) ) ;
}

// boolean isRButtonDownAt( ulong index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SpriteMouseListener_isRButtonDownAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SpriteMouseListener, pThis ) ;
	LSpriteMouseListener *	pListener = pThis->GetRef<LSpriteMouseListener>() ;
	LQT_VERIFY_NULL_PTR( pListener ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	LQT_RETURN_BOOL( pListener->IsRButtonDownAt( (size_t) index ) ) ;
}

// uint getLDownPointsCount( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SpriteMouseListener_getLDownPointsCount)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SpriteMouseListener, pThis ) ;
	LSpriteMouseListener *	pListener = pThis->GetRef<LSpriteMouseListener>() ;
	LQT_VERIFY_NULL_PTR( pListener ) ;

	LQT_RETURN_UINT( (LUint) pListener->GetLDownPointsCount() ) ;
}

// const Vector2d* enumerateLDownPoints( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SpriteMouseListener_enumerateLDownPoints)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SpriteMouseListener, pThis ) ;
	LSpriteMouseListener *	pListener = pThis->GetRef<LSpriteMouseListener>() ;
	LQT_VERIFY_NULL_PTR( pListener ) ;

	SArray<S2DDVector>	arrPoints ;
	if ( pListener->EnumerateLDownPoints( arrPoints ) == 0 )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}

	LQT_RETURN_POINTER
		( arrPoints.GetConstArray(),
			arrPoints.GetLength() * sizeof(S2DDVector) ) ;
}



// SpriteMouseListener
//////////////////////////////////////////////////////////////////////////

// クラス情報
ESL_IMPLEMENT_CLASS_INFO( Loquaty::LSpriteMouseListener, SGLSpriteMouseStateListener )

// 構築関数
LSpriteMouseListener::LSpriteMouseListener( LVirtualMachine& vm, LObjPtr pObj )
{
	m_pTask = vm.new_Task() ;
	m_pObj = pObj.Ptr() ;
	m_pSpriteClass = vm.GetClassPathAs( L"EntisGLS4.Sprite" ) ;
}

// マウス移動
bool LSpriteMouseListener::OnMouseMove
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	SGLSpriteMouseStateListener::OnMouseMove( sprite, xPos, yPos, nFlags ) ;

	return	CallbackOnMouse( L"onMouseMove", sprite, xPos, yPos, nFlags ) ;
}

void LSpriteMouseListener::OnMouseLeave( SGLSprite& sprite, int64_t nFlags )
{
	SGLSpriteMouseStateListener::OnMouseLeave( sprite, nFlags ) ;

	LPtr<LNativeObj>	pSprite( new LNativeObj( m_pSpriteClass ) ) ;
	pSprite->SetNative( std::make_shared<LEntisGLS4_Sprite>( &sprite ) ) ;

	LValue	valArg[2] ;
	valArg[0] = LValue( pSprite ) ;
	valArg[1] = LValue( LType::typeUint64, LValue::MakeUint64(nFlags) ) ;

	auto	[valRet, except] =
		m_pTask->SyncCallFunctionAs
			( LObjPtr(LObject::AddRef(m_pObj)), L"onMouseLeave", valArg, 2 ) ;
	if ( except != nullptr )
	{
		LString	lstr ;
		except->AsString( lstr ) ;

		std::string	str = lstr.ToString() ;
		LTrace( "onMouseLeave:exception:%s\n", str.c_str() ) ;
	}
}

// ホイール回転
bool LSpriteMouseListener::OnMouseWheel
	( SGLSprite& sprite, int32_t zDelta,
		double xPos, double yPos, int64_t nFlags )
{
	SGLSpriteMouseStateListener::OnMouseWheel( sprite, zDelta, xPos, yPos, nFlags ) ;

	LPtr<LNativeObj>	pSprite( new LNativeObj( m_pSpriteClass ) ) ;
	pSprite->SetNative( std::make_shared<LEntisGLS4_Sprite>( &sprite ) ) ;

	LValue	valArg[5] ;
	valArg[0] = LValue( pSprite ) ;
	valArg[1] = LValue( LType::typeInt32, LValue::MakeLong(zDelta) ) ;
	valArg[2] = LValue( LType::typeDouble, LValue::MakeDouble(xPos) ) ;
	valArg[3] = LValue( LType::typeDouble, LValue::MakeDouble(yPos) ) ;
	valArg[4] = LValue( LType::typeUint64, LValue::MakeUint64(nFlags) ) ;

	auto	[valRet, except] =
		m_pTask->SyncCallFunctionAs
			( LObjPtr(LObject::AddRef(m_pObj)), L"onMouseWheel", valArg, 5 ) ;
	if ( except != nullptr )
	{
		LString	lstr ;
		except->AsString( lstr ) ;

		std::string	str = lstr.ToString() ;
		LTrace( "onMouseWheel:exception:%s\n", str.c_str() ) ;
		return	false ;
	}
	return	valRet.AsBoolean() ;
}

// マウスボタン（前処理）
bool LSpriteMouseListener::OnButtonDown
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	SGLSpriteMouseStateListener::OnButtonDown( sprite, xPos, yPos, nFlags ) ;

	return	CallbackOnMouse( L"onButtonDown", sprite, xPos, yPos, nFlags ) ;
}

bool LSpriteMouseListener::OnButtonUp
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	SGLSpriteMouseStateListener::OnButtonUp( sprite, xPos, yPos, nFlags ) ;

	return	CallbackOnMouse( L"onButtonUp", sprite, xPos, yPos, nFlags ) ;
}

bool LSpriteMouseListener::OnButtonDblClk
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	SGLSpriteMouseStateListener::OnButtonDblClk( sprite, xPos, yPos, nFlags ) ;

	return	CallbackOnMouse( L"onButtonDblClk", sprite, xPos, yPos, nFlags ) ;
}

// マウスボタン（後処理）
bool LSpriteMouseListener::AfterButtonDown
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	SGLSpriteMouseStateListener::AfterButtonDown( sprite, xPos, yPos, nFlags ) ;

	return	CallbackOnMouse( L"afterButtonDown", sprite, xPos, yPos, nFlags ) ;
}

bool LSpriteMouseListener::AfterButtonUp
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	SGLSpriteMouseStateListener::AfterButtonUp( sprite, xPos, yPos, nFlags ) ;

	return	CallbackOnMouse( L"afterButtonUp", sprite, xPos, yPos, nFlags ) ;
}

bool LSpriteMouseListener::AfterButtonDblClk
	( SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	SGLSpriteMouseStateListener::AfterButtonDblClk( sprite, xPos, yPos, nFlags ) ;

	return	CallbackOnMouse( L"afterButtonDblClk", sprite, xPos, yPos, nFlags ) ;
}

// コールバック
bool LSpriteMouseListener::CallbackOnMouse
	( const wchar_t * pwszName,
		SGLSprite& sprite,
		double xPos, double yPos, int64_t nFlags )
{
	LPtr<LNativeObj>	pSprite( new LNativeObj( m_pSpriteClass ) ) ;
	pSprite->SetNative( std::make_shared<LEntisGLS4_Sprite>( &sprite ) ) ;

	LValue	valArg[4] ;
	valArg[0] = LValue( pSprite ) ;
	valArg[1] = LValue( LType::typeDouble, LValue::MakeDouble(xPos) ) ;
	valArg[2] = LValue( LType::typeDouble, LValue::MakeDouble(yPos) ) ;
	valArg[3] = LValue( LType::typeUint64, LValue::MakeUint64(nFlags) ) ;

	auto	[valRet, except] =
		m_pTask->SyncCallFunctionAs
			( LObjPtr(LObject::AddRef(m_pObj)), pwszName, valArg, 4 ) ;
	if ( except != nullptr )
	{
		LString	lstrFunc = pwszName ;
		LString	lstrMsg ;
		except->AsString( lstrMsg ) ;

		std::string	strFunc = lstrFunc.ToString() ;
		std::string	strMsg = lstrMsg.ToString() ;
		LTrace( "%s:exception:%s\n", strFunc.c_str(), strMsg.c_str() ) ;
		return	false ;
	}
	return	valRet.AsBoolean() ;
}

