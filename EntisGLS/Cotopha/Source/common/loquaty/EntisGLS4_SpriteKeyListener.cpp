
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_SpriteKeyListener.h>


// SpriteKeyListener( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_SpriteKeyListener)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ
		( LEntisGLS4_SpriteKeyListener, pThis,
			( new SSmartObject
				( new LSpriteKeyListener
					( _context.VM(), LQT_ARG_OBJECT(0) ) ) ) ) ;

	// pThis->Initialize() ;

	LQT_RETURN_VOID() ;
}


// SpriteKeyListener
//////////////////////////////////////////////////////////////////////////

// クラス情報
ESL_IMPLEMENT_CLASS_INFO( Loquaty::LSpriteKeyListener, SGLSpriteKeyListener )

// 構築関数
LSpriteKeyListener::LSpriteKeyListener( LVirtualMachine& vm, LObjPtr pObj )
{
	m_pTask = vm.new_Task() ;
	m_pObj = pObj.Ptr() ;
	m_pSpriteClass = vm.GetClassPathAs( L"EntisGLS4.Sprite" ) ;
	m_pStringClass = vm.GetStringClass() ;
	m_pInputStartCompositionClass = vm.GetClassPathAs( L"EntisGLS4.InputStartComposition" ) ;
	assert( m_pInputStartCompositionClass != nullptr ) ;
	m_pInputCompositionStringClass = vm.GetClassPathAs( L"EntisGLS4.InputCompositionString" ) ;
	assert( m_pInputCompositionStringClass != nullptr ) ;
}

// キー入力
bool LSpriteKeyListener::OnKeyDown
	( SGLSprite& sprite, int64_t nVirtKey, int64_t nFlags )
{
	return	CallbackOnKey( L"onKeyDown", sprite, nVirtKey, nFlags ) ;
}

bool LSpriteKeyListener::OnKeyUp
	( SGLSprite& sprite, int64_t nVirtKey, int64_t nFlags )
{
	return	CallbackOnKey( L"onKeyUp", sprite, nVirtKey, nFlags ) ;
}

// 文字入力
bool LSpriteKeyListener::OnChar( SGLSprite& sprite, uint16_t codeChar )
{
	LPtr<LNativeObj>	pSprite( new LNativeObj( m_pSpriteClass ) ) ;
	pSprite->SetNative( std::make_shared<LEntisGLS4_Sprite>( &sprite ) ) ;

	LValue	valArg[2] ;
	valArg[0] = LValue( pSprite ) ;
	valArg[1] = LValue( LType::typeUint32, LValue::MakeLong(codeChar) ) ;

	auto	[valRet, except] =
		m_pTask->SyncCallFunctionAs
			( LObjPtr(LObject::AddRef(m_pObj)), L"onChar", valArg, 2 ) ;
	if ( except != nullptr )
	{
		LString	lstr ;
		except->AsString( lstr ) ;

		std::string	str = lstr.ToString() ;
		LTrace( "onChar:exception:%s\n", str.c_str() ) ;
		return	true ;
	}
	return	valRet.AsBoolean() ;
}

// コンポジション開始
bool LSpriteKeyListener::OnStartComposition
	( SGLSprite& sprite, SGLInputStartComposition& iscForm )
{
	if ( m_pInputStartCompositionClass == nullptr )
	{
		return	false ;
	}
	LPtr<LNativeObj>	pSprite( new LNativeObj( m_pSpriteClass ) ) ;
	pSprite->SetNative( std::make_shared<LEntisGLS4_Sprite>( &sprite ) ) ;

	LObjPtr	piscForm( m_pInputStartCompositionClass->CreateInstance() ) ;

	LValue	valArg[2] ;
	valArg[0] = LValue( pSprite ) ;
	valArg[1] = LValue( piscForm ) ;

	auto	[valRet, except] =
		m_pTask->SyncCallFunctionAs
			( LObjPtr(LObject::AddRef(m_pObj)), L"onStartComposition", valArg, 2 ) ;
	if ( except != nullptr )
	{
		LString	lstrMsg ;
		except->AsString( lstrMsg ) ;

		std::string	strMsg = lstrMsg.ToString() ;
		LTrace( "onStartComposition:exception:%s\n", strMsg.c_str() ) ;
		return	false ;
	}
	if ( valRet.AsBoolean() )
	{
		iscForm.nFlags = 0 ;

		LPoint *	pptStart =
			(LPoint*) piscForm->GetElementPointerAs( L"ptStart", sizeof(LPoint) ) ;
		if ( pptStart != nullptr )
		{
			iscForm.nFlags |= SGLInputStartComposition::flagPosition ;
			iscForm.ptStart = *pptStart ;
		}
		LImageRect *	prctArea =
			(LImageRect*) piscForm->GetElementPointerAs( L"rctArea", sizeof(LImageRect) ) ;
		if ( prctArea != nullptr )
		{
			iscForm.nFlags |= SGLInputStartComposition::flagRectangle ;
			iscForm.rctArea = *prctArea ;
		}
		LPtr<LEntisGLS4_FontStyle>
				pFontStyle( piscForm->GetElementAs( L"fsFontStyle" ) ) ;
		if ( pFontStyle != nullptr )
		{
			iscForm.nFlags |= SGLInputStartComposition::flagFont ;
			GetLFontStyle( iscForm.fsFontStyle, m_strFontFace, pFontStyle ) ;
		}
		return	true ;
	}
	return	false ;
}

// コンポジション終了
bool LSpriteKeyListener::OnEndComposition( SGLSprite& sprite )
{
	LPtr<LNativeObj>	pSprite( new LNativeObj( m_pSpriteClass ) ) ;
	pSprite->SetNative( std::make_shared<LEntisGLS4_Sprite>( &sprite ) ) ;

	LValue	valArg[1] ;
	valArg[0] = LValue( pSprite ) ;

	auto	[valRet, except] =
		m_pTask->SyncCallFunctionAs
			( LObjPtr(LObject::AddRef(m_pObj)), L"onEndComposition", valArg, 1 ) ;
	if ( except != nullptr )
	{
		LString	lstrMsg ;
		except->AsString( lstrMsg ) ;

		std::string	strMsg = lstrMsg.ToString() ;
		LTrace( "onEndComposition:exception:%s\n", strMsg.c_str() ) ;
		return	false ;
	}
	return	valRet.AsBoolean() ;
}

// コンポジション文字列
bool LSpriteKeyListener::OnCompositionString
	( SGLSprite& sprite, const SGLInputCompositionString& icsComp )
{
	if ( m_pInputCompositionStringClass == nullptr )
	{
		return	false ;
	}
	LPtr<LNativeObj>	pSprite( new LNativeObj( m_pSpriteClass ) ) ;
	pSprite->SetNative( std::make_shared<LEntisGLS4_Sprite>( &sprite ) ) ;

	LObjPtr	picsComp( m_pInputCompositionStringClass->CreateInstance() ) ;
	LString	strComp( icsComp.pszComposition + icsComp.nStart, icsComp.nCount ) ;
	picsComp->SetElementLongAs( L"nFlags", icsComp.nFlags ) ;
	picsComp->SetElementStringAs( L"strComposition", strComp.c_str() ) ;

	LValue	valArg[2] ;
	valArg[0] = LValue( pSprite ) ;
	valArg[1] = LValue( picsComp ) ;

	auto	[valRet, except] =
		m_pTask->SyncCallFunctionAs
			( LObjPtr(LObject::AddRef(m_pObj)), L"onCompositionString", valArg, 2 ) ;
	if ( except != nullptr )
	{
		LString	lstrMsg ;
		except->AsString( lstrMsg ) ;

		std::string	strMsg = lstrMsg.ToString() ;
		LTrace( "onCompositionString:exception:%s\n", strMsg.c_str() ) ;
		return	false ;
	}
	return	valRet.AsBoolean() ;
}

// コマンド
bool LSpriteKeyListener::OnCommand
	( SGLSprite& sprite, const wchar_t * pszCmd,
		int64_t nParam, int64_t nCode, int nPriority, bool fOverwritable )
{
	LPtr<LNativeObj>	pSprite( new LNativeObj( m_pSpriteClass ) ) ;
	pSprite->SetNative( std::make_shared<LEntisGLS4_Sprite>( &sprite ) ) ;

	LPtr<LStringObj>	pStrCmd( new LStringObj( m_pStringClass, pszCmd ) ) ;

	LValue	valArg[6] ;
	valArg[0] = LValue( pSprite ) ;
	valArg[1] = LValue( pStrCmd ) ;
	valArg[2] = LValue( LType::typeInt64, LValue::MakeLong(nParam) ) ;
	valArg[3] = LValue( LType::typeInt64, LValue::MakeLong(nCode) ) ;
	valArg[4] = LValue( LType::typeInt32, LValue::MakeLong(nPriority) ) ;
	valArg[5] = LValue( LType::typeBoolean, LValue::MakeBool(fOverwritable) ) ;

	auto	[valRet, except] =
		m_pTask->SyncCallFunctionAs
			( LObjPtr(LObject::AddRef(m_pObj)), L"onCommand", valArg, 6 ) ;
	if ( except != nullptr )
	{
		LString	lstrMsg ;
		except->AsString( lstrMsg ) ;

		std::string	strMsg = lstrMsg.ToString() ;
		LTrace( "onCommand:exception:%s\n", strMsg.c_str() ) ;
		return	false ;
	}
	return	valRet.AsBoolean() ;
}

// コールバック
bool LSpriteKeyListener::CallbackOnKey
	( const wchar_t * pwszName,
		SGLSprite& sprite, int64_t nVirtKey, int64_t nFlags )
{
	LPtr<LNativeObj>	pSprite( new LNativeObj( m_pSpriteClass ) ) ;
	pSprite->SetNative( std::make_shared<LEntisGLS4_Sprite>( &sprite ) ) ;

	LValue	valArg[3] ;
	valArg[0] = LValue( pSprite ) ;
	valArg[1] = LValue( LType::typeInt64, LValue::MakeLong(nVirtKey) ) ;
	valArg[2] = LValue( LType::typeUint64, LValue::MakeUint64(nFlags) ) ;

	auto	[valRet, except] =
		m_pTask->SyncCallFunctionAs
			( LObjPtr(LObject::AddRef(m_pObj)), pwszName, valArg, 3 ) ;
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


