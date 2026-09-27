
#include <loquaty.h>
#include "EntisGLS4_SceneProperty.h"

using namespace Loquaty ;


// String getItemIdentity( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneProperty_getItemIdentity)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneProperty, pThis ) ;

	LString	valRet ;
	// valRet = pThis->getItemIdentity(...) ;

	LQT_RETURN_STRING( valRet ) ;
}

// void setItemIdentity( String id )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneProperty_setItemIdentity)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneProperty, pThis ) ;
	LQT_FUNC_ARG_STRING( id ) ;

	// pThis->setItemIdentity(...) ;

	LQT_RETURN_VOID() ;
}

// ulong getParameterCount( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneProperty_getParameterCount)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneProperty, pThis ) ;

	LUint64	valRet ;
	// valRet = pThis->getParameterCount(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// String getParameterID( ulong iParam ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneProperty_getParameterID)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneProperty, pThis ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;

	LString	valRet ;
	// valRet = pThis->getParameterID(...) ;

	LQT_RETURN_STRING( valRet ) ;
}

// String getParameterFriendlyName( ulong iParam ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneProperty_getParameterFriendlyName)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneProperty, pThis ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;

	LString	valRet ;
	// valRet = pThis->getParameterFriendlyName(...) ;

	LQT_RETURN_STRING( valRet ) ;
}

// String getParameterDescription( ulong iParam ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneProperty_getParameterDescription)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneProperty, pThis ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;

	LString	valRet ;
	// valRet = pThis->getParameterDescription(...) ;

	LQT_RETURN_STRING( valRet ) ;
}

// long findParameterID( String id ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneProperty_findParameterID)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneProperty, pThis ) ;
	LQT_FUNC_ARG_STRING( id ) ;

	LInt64	valRet ;
	// valRet = pThis->findParameterID(...) ;

	LQT_RETURN_LONG( valRet ) ;
}

// EntisGLS4.SceneProperty.ParameterType getParameterType( ulong iParam ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneProperty_getParameterType)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneProperty, pThis ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;

	LInt32	valRet ;
	// valRet = pThis->getParameterType(...) ;

	LQT_RETURN_INT( valRet ) ;
}

// EntisGLS4.SceneProperty.ParameterAttribute getParameterAttributes( ulong iParam ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneProperty_getParameterAttributes)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneProperty, pThis ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;

	LUint32	valRet ;
	// valRet = pThis->getParameterAttributes(...) ;

	LQT_RETURN_UINT( valRet ) ;
}

// boolean getParameterScalarRange( ulong iParam, double* pMin, double* pMax ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneProperty_getParameterScalarRange)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneProperty, pThis ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;
	LQT_FUNC_ARG_POINTER( LDouble, pMin ) ;
	LQT_VERIFY_NULL_PTR( pMin ) ;
	LQT_FUNC_ARG_POINTER( LDouble, pMax ) ;
	LQT_VERIFY_NULL_PTR( pMax ) ;

	LBoolean	valRet ;
	// valRet = pThis->getParameterScalarRange(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean isParameterValidation( ulong iParam ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneProperty_isParameterValidation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneProperty, pThis ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;

	LBoolean	valRet ;
	// valRet = pThis->isParameterValidation(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// String getParameterCategoryName( ulong iCategory ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneProperty_getParameterCategoryName)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneProperty, pThis ) ;
	LQT_FUNC_ARG_ULONG( iCategory ) ;

	LString	valRet ;
	// valRet = pThis->getParameterCategoryName(...) ;

	LQT_RETURN_STRING( valRet ) ;
}

// EntisGLS4.SceneSequencer getParameterSequencer( ulong iParam ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneProperty_getParameterSequencer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneProperty, pThis ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.SceneSequencer) ) ) ;
	// valRet = pThis->getParameterSequencer(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_SceneSequencer> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_SceneSequencer>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.SceneSequencer createParameterSequencer( ulong iParam )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneProperty_createParameterSequencer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneProperty, pThis ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.SceneSequencer) ) ) ;
	// valRet = pThis->createParameterSequencer(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_SceneSequencer> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_SceneSequencer>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// void removeParameterSequencer( ulong iParam )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneProperty_removeParameterSequencer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneProperty, pThis ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;

	// pThis->removeParameterSequencer(...) ;

	LQT_RETURN_VOID() ;
}

// EntisGLS4.ModelPoseLibrary getPoseLibraryChain( )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneProperty_getPoseLibraryChain)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneProperty, pThis ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.ModelPoseLibrary) ) ) ;
	// valRet = pThis->getPoseLibraryChain(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_ModelPoseLibrary> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_ModelPoseLibrary>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}



