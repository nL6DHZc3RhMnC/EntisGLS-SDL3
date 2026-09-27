
#include <loquaty.h>
#include "EntisGLS4_SpriteMouseListener.h"

using namespace Loquaty ;


// uint getButtonID( ulong nFlags )
IMPL_LOQUATY_FUNC(EntisGLS4_SpriteMouseListener_getButtonID)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_ARG_ULONG( nFlags ) ;

	LUint32	valRet ;
	// valRet = getButtonID(...) ;

	LQT_RETURN_UINT( valRet ) ;
}

// uint getMouseID( ulong nFlags )
IMPL_LOQUATY_FUNC(EntisGLS4_SpriteMouseListener_getMouseID)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_ARG_ULONG( nFlags ) ;

	LUint32	valRet ;
	// valRet = getMouseID(...) ;

	LQT_RETURN_UINT( valRet ) ;
}

// boolean isFromTouch( ulong nFlags )
IMPL_LOQUATY_FUNC(EntisGLS4_SpriteMouseListener_isFromTouch)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_ARG_ULONG( nFlags ) ;

	LBoolean	valRet ;
	// valRet = isFromTouch(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// SpriteMouseListener( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_SpriteMouseListener)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ( LEntisGLS4_SpriteMouseListener, pThis, () ) ;

	// pThis->Initialize() ;

	LQT_RETURN_VOID() ;
}

// uint getPointerCount( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SpriteMouseListener_getPointerCount)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SpriteMouseListener, pThis ) ;

	LUint32	valRet ;
	// valRet = pThis->getPointerCount(...) ;

	LQT_RETURN_UINT( valRet ) ;
}

// int findMouseIndexById( uint idMouse ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SpriteMouseListener_findMouseIndexById)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SpriteMouseListener, pThis ) ;
	LQT_FUNC_ARG_UINT( idMouse ) ;

	LInt32	valRet ;
	// valRet = pThis->findMouseIndexById(...) ;

	LQT_RETURN_INT( valRet ) ;
}

// boolean getMousePointAt( ulong index, Vector2d* vPos ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SpriteMouseListener_getMousePointAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SpriteMouseListener, pThis ) ;
	LQT_FUNC_ARG_ULONG( index ) ;
	LQT_FUNC_ARG_STRUCT( LVector2d, vPos ) ;
	LQT_VERIFY_NULL_PTR( vPos ) ;

	LBoolean	valRet ;
	// valRet = pThis->getMousePointAt(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean getMousePointAs( uint idMouse, Vector2d* vPos ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SpriteMouseListener_getMousePointAs)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SpriteMouseListener, pThis ) ;
	LQT_FUNC_ARG_UINT( idMouse ) ;
	LQT_FUNC_ARG_STRUCT( LVector2d, vPos ) ;
	LQT_VERIFY_NULL_PTR( vPos ) ;

	LBoolean	valRet ;
	// valRet = pThis->getMousePointAs(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean isLButtonDownAt( ulong index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SpriteMouseListener_isLButtonDownAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SpriteMouseListener, pThis ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	LBoolean	valRet ;
	// valRet = pThis->isLButtonDownAt(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean isRButtonDownAt( ulong index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SpriteMouseListener_isRButtonDownAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SpriteMouseListener, pThis ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	LBoolean	valRet ;
	// valRet = pThis->isRButtonDownAt(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// uint getLDownPointsCount( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SpriteMouseListener_getLDownPointsCount)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SpriteMouseListener, pThis ) ;

	LUint32	valRet ;
	// valRet = pThis->getLDownPointsCount(...) ;

	LQT_RETURN_UINT( valRet ) ;
}

// const Vector2d* enumerateLDownPoints( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SpriteMouseListener_enumerateLDownPoints)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SpriteMouseListener, pThis ) ;

	LVector2d	valRet ;
	// valRet = pThis->enumerateLDownPoints(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}



