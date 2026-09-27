
#include <loquaty.h>
#include "EntisGLS4_SceneInstancingItem.h"

using namespace Loquaty ;


// const Quaterniond* getBaseRotation( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneInstancingItem_getBaseRotation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneInstancingItem, pThis ) ;

	LQuaterniond	valRet ;
	// valRet = pThis->getBaseRotation(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// void setBaseRotation( const Quaterniond* rot )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneInstancingItem_setBaseRotation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneInstancingItem, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LQuaterniond, rot ) ;
	LQT_VERIFY_NULL_PTR( rot ) ;

	// pThis->setBaseRotation(...) ;

	LQT_RETURN_VOID() ;
}

// const Vector3d* getBaseZoom( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneInstancingItem_getBaseZoom)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneInstancingItem, pThis ) ;

	LVector3d	valRet ;
	// valRet = pThis->getBaseZoom(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// void setBaseZoom( const Vector3d* zoom )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneInstancingItem_setBaseZoom)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneInstancingItem, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, zoom ) ;
	LQT_VERIFY_NULL_PTR( zoom ) ;

	// pThis->setBaseZoom(...) ;

	LQT_RETURN_VOID() ;
}

// void addTemporaryInstances( const Matrix4* matrixes, const EntisGLS4.ColorMulAdd* colors, ulong count, EntisGLS4.SceneItem itemFrom )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneInstancingItem_addTemporaryInstances)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneInstancingItem, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix4, matrixes ) ;
	LQT_VERIFY_NULL_PTR( matrixes ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_ColorMulAdd, colors ) ;
	LQT_VERIFY_NULL_PTR( colors ) ;
	LQT_FUNC_ARG_ULONG( count ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneItem, itemFrom ) ;
	LQT_VERIFY_NULL_PTR( itemFrom ) ;

	// pThis->addTemporaryInstances(...) ;

	LQT_RETURN_VOID() ;
}

// ulong getStaticInstanceCount( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneInstancingItem_getStaticInstanceCount)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneInstancingItem, pThis ) ;

	LUint64	valRet ;
	// valRet = pThis->getStaticInstanceCount(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// boolean getStaticInstanceAt( ulong index, Matrix4* matrix, EntisGLS4.ColorMulAdd* color ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneInstancingItem_getStaticInstanceAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneInstancingItem, pThis ) ;
	LQT_FUNC_ARG_ULONG( index ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix4, matrix ) ;
	LQT_VERIFY_NULL_PTR( matrix ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_ColorMulAdd, color ) ;
	LQT_VERIFY_NULL_PTR( color ) ;

	LBoolean	valRet ;
	// valRet = pThis->getStaticInstanceAt(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void insertStaticInstanceAt( ulong index, const Matrix4* matrix, const EntisGLS4.ColorMulAdd* color )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneInstancingItem_insertStaticInstanceAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneInstancingItem, pThis ) ;
	LQT_FUNC_ARG_ULONG( index ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix4, matrix ) ;
	LQT_VERIFY_NULL_PTR( matrix ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_ColorMulAdd, color ) ;
	LQT_VERIFY_NULL_PTR( color ) ;

	// pThis->insertStaticInstanceAt(...) ;

	LQT_RETURN_VOID() ;
}

// void removeStaticInstanceAt( ulong index )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneInstancingItem_removeStaticInstanceAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneInstancingItem, pThis ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	// pThis->removeStaticInstanceAt(...) ;

	LQT_RETURN_VOID() ;
}

// void lockInstancing( )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneInstancingItem_lockInstancing)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneInstancingItem, pThis ) ;

	// pThis->lockInstancing(...) ;

	LQT_RETURN_VOID() ;
}

// void unlockInstancing( )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneInstancingItem_unlockInstancing)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneInstancingItem, pThis ) ;

	// pThis->unlockInstancing(...) ;

	LQT_RETURN_VOID() ;
}



