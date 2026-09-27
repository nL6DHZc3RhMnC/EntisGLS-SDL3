
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_SpriteTimer.h>


// SpriteTimer( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_SpriteTimer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ
		( LEntisGLS4_SpriteTimer, pThis,
			( new SSmartObject
				( new LSpriteTimer
					( _context.VM(), LQT_ARG_OBJECT(0) ) ) ) ) ;

	LQT_RETURN_VOID() ;
}



// SpriteTimer
//////////////////////////////////////////////////////////////////////////

// クラス情報
ESL_IMPLEMENT_CLASS_INFO( Loquaty::LSpriteTimer, SGLSpriteTimer )

// 構築関数
LSpriteTimer::LSpriteTimer( LVirtualMachine& vm, LObjPtr pObj )
{
	m_pTask = vm.new_Task() ;
	m_pObj = pObj.Ptr() ;
	m_pSpriteClass = vm.GetClassPathAs( L"EntisGLS4.Sprite" ) ;
}

// タイマー処理（true で終了）
bool LSpriteTimer::OnTimer( SGLSprite& sprite, uint32_t msecPast )
{
	LPtr<LNativeObj>	pSprite( new LNativeObj( m_pSpriteClass ) ) ;
	pSprite->SetNative( std::make_shared<LEntisGLS4_Sprite>( &sprite ) ) ;

	LValue	valArg[2] ;
	valArg[0] = LValue( pSprite ) ;
	valArg[1] = LValue( LType::typeUint32, LValue::MakeLong(msecPast) ) ;

	auto	[valRet, except] =
		m_pTask->SyncCallFunctionAs
			( LObjPtr(LObject::AddRef(m_pObj)), L"onTimer", valArg, 2 ) ;
	if ( except != nullptr )
	{
		LString	lstr ;
		except->AsString( lstr ) ;

		std::string	str = lstr.ToString() ;
		LTrace( "onTimer:exception:%s\n", str.c_str() ) ;
		return	true ;
	}
	return	valRet.AsBoolean() ;
}


