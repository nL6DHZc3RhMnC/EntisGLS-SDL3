
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_RenderableSprite.h>


// RenderableSprite( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_RenderableSprite)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ
		( LEntisGLS4_RenderableSprite, pThis,
			( new SSmartObject
				( new LRenderableSprite( _context.VM(), LQT_ARG_OBJECT(0) ) ) ) ) ;

	LQT_RETURN_VOID() ;
}




// RenderableSprite
//////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Loquaty::LRenderableSprite, SGLSprite )

// 構築関数
//////////////////////////////////////////////////////////////////////////
LRenderableSprite::LRenderableSprite( LVirtualMachine& vm, LObjPtr pObj )
{
	m_pTask = vm.new_Task();
	m_pObj = pObj.Ptr() ;
	m_pRenderClass = vm.GetClassPathAs( L"EntisGLS4.RenderContext" ) ;
}

// フレーム描画（視点に関係しない）共通処理
//////////////////////////////////////////////////////////////////////////
void LRenderableSprite::PrepareDrawFrame( void )
{
	CallbackOnDrawFrame( L"onPrepareFrame" ) ;

	SGLSprite::PrepareDrawFrame() ;
}

// フレーム描画完了後処理
//////////////////////////////////////////////////////////////////////////
void LRenderableSprite::FinishDrawFrame( void )
{
	CallbackOnDrawFrame( L"onFinishFrame" ) ;

	SGLSprite::FinishDrawFrame() ;
}

// 子スプライトを描画
//////////////////////////////////////////////////////////////////////////
void LRenderableSprite::DrawChildren
	( S3DRenderContextInterface& render, Stereo3DView s3dView ) const
{
	LPtr<LNativeObj>	pRender( new LNativeObj( m_pRenderClass ) ) ;
	pRender->SetNative
		( std::make_shared<LEntisGLS4_RenderContext>
				( (SGLPaintContextInterface*) &render ) ) ;

	LValue	valArg[2] ;
	valArg[0] = LValue( pRender ) ;
	valArg[1] = LValue( LType::typeInt32, LValue::MakeUint64(s3dView) ) ;

	auto	[valRet, except] =
		m_pTask->SyncCallFunctionAs
			( LObjPtr(LObject::AddRef(m_pObj)), L"onRender", valArg, 2 ) ;
	if ( except != nullptr )
	{
		LString	lstr ;
		except->AsString( lstr ) ;

		std::string	str = lstr.ToString() ;
		LTrace( "onRender:exception:%s\n", str.c_str() ) ;
	}

	SGLSprite::DrawChildren( render, s3dView ) ;
}

// コールバック
//////////////////////////////////////////////////////////////////////////
void LRenderableSprite::CallbackOnDrawFrame( const wchar_t * pwszFunc )
{
	auto	[valRet, except] =
		m_pTask->SyncCallFunctionAs
			( LObjPtr(LObject::AddRef(m_pObj)), pwszFunc, nullptr, 0 ) ;
	if ( except != nullptr )
	{
		LString	lstrFunc = pwszFunc ;
		LString	lstrErr ;
		except->AsString( lstrErr ) ;

		std::string	strFunc = lstrFunc.ToString() ;
		std::string	strErr = lstrErr.ToString() ;
		LTrace( "%s:exception:%s\n", strFunc.c_str(), strErr.c_str() ) ;
	}
}


