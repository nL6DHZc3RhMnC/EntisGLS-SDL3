
#include <loquaty.h>
#include "EntisGLS4_SceneMeshBuffer.h"

using namespace Loquaty ;


// SceneMeshBuffer( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_SceneMeshBuffer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ( LEntisGLS4_SceneMeshBuffer, pThis, () ) ;

	// pThis->Initialize() ;

	LQT_RETURN_VOID() ;
}

// EntisGLS4.VertexBuffer[] lockMeshBuffer( )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneMeshBuffer_lockMeshBuffer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneMeshBuffer, pThis ) ;

	LObjPtr	valRet ;
	// valRet = pThis->lockMeshBuffer(...) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// void unlockMeshBuffer( )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneMeshBuffer_unlockMeshBuffer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneMeshBuffer, pThis ) ;

	// pThis->unlockMeshBuffer(...) ;

	LQT_RETURN_VOID() ;
}



