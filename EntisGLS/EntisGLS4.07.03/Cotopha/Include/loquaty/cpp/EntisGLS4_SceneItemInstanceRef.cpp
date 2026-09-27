
#include <loquaty.h>
#include "EntisGLS4_SceneItemInstanceRef.h"

using namespace Loquaty ;


// SceneItemInstanceRef( const EntisGLS4.SceneItemInstanceRef ref )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_SceneItemInstanceRef)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_OBJ( LNativeObj, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneItemInstanceRef, ref ) ;
	LQT_VERIFY_NULL_PTR( ref ) ;

	pThis->SetNative
		( std::make_shared<LEntisGLS4_SceneItemInstanceRef>( /* construction-arg-list */ ) ) ;

	LQT_RETURN_VOID() ;
}

// SceneItemInstanceRef( EntisGLS4.SceneItem item, ulong instance )
IMPL_LOQUATY_CONSTRUCTOR_N(EntisGLS4_SceneItemInstanceRef,1)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_OBJ( LNativeObj, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneItem, item ) ;
	LQT_VERIFY_NULL_PTR( item ) ;
	LQT_FUNC_ARG_ULONG( instance ) ;

	pThis->SetNative
		( std::make_shared<LEntisGLS4_SceneItemInstanceRef>( /* construction-arg-list */ ) ) ;

	LQT_RETURN_VOID() ;
}

// EntisGLS4.SceneItemInstanceRef operator :=( const EntisGLS4.SceneItemInstanceRef ref )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItemInstanceRef_operator_smov)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItemInstanceRef, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneItemInstanceRef, ref ) ;
	LQT_VERIFY_NULL_PTR( ref ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.SceneItemInstanceRef) ) ) ;
	// valRet = pThis->operator :=(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_SceneItemInstanceRef> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_SceneItemInstanceRef>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// boolean isEmpty( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItemInstanceRef_isEmpty)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItemInstanceRef, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->isEmpty(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean isEqual( const EntisGLS4.SceneItemInstanceRef ref ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItemInstanceRef_isEqual)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItemInstanceRef, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneItemInstanceRef, ref ) ;
	LQT_VERIFY_NULL_PTR( ref ) ;

	LBoolean	valRet ;
	// valRet = pThis->isEqual(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void releaseRef( )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItemInstanceRef_releaseRef)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItemInstanceRef, pThis ) ;

	// pThis->releaseRef(...) ;

	LQT_RETURN_VOID() ;
}

// EntisGLS4.SceneItem getRefItem( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItemInstanceRef_getRefItem)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItemInstanceRef, pThis ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.SceneItem) ) ) ;
	// valRet = pThis->getRefItem(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_SceneItem> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_SceneItem>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// boolean isInstance( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItemInstanceRef_isInstance)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItemInstanceRef, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->isInstance(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// long getInstanceIndex( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItemInstanceRef_getInstanceIndex)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItemInstanceRef, pThis ) ;

	LInt64	valRet ;
	// valRet = pThis->getInstanceIndex(...) ;

	LQT_RETURN_LONG( valRet ) ;
}

// boolean correctInstance( )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItemInstanceRef_correctInstance)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItemInstanceRef, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->correctInstance(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean getInstanceMatrix( Matrix4* matrix, EntisGLS4.ColorMulAdd* color )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItemInstanceRef_getInstanceMatrix)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItemInstanceRef, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix4, matrix ) ;
	LQT_VERIFY_NULL_PTR( matrix ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_ColorMulAdd, color ) ;
	LQT_VERIFY_NULL_PTR( color ) ;

	LBoolean	valRet ;
	// valRet = pThis->getInstanceMatrix(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean deleteInstance( )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneItemInstanceRef_deleteInstance)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneItemInstanceRef, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->deleteInstance(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}



