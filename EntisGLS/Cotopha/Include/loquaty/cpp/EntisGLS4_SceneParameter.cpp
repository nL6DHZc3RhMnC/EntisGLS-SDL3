
#include <loquaty.h>
#include "EntisGLS4_SceneParameter.h"

using namespace Loquaty ;


// uint colorFromVector( const Vector3d* vec )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneParameter_colorFromVector)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vec ) ;
	LQT_VERIFY_NULL_PTR( vec ) ;

	LUint32	valRet ;
	// valRet = colorFromVector(...) ;

	LQT_RETURN_UINT( valRet ) ;
}

// const Vector3d* vectorFromColor( uint rgb )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneParameter_vectorFromColor)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_ARG_UINT( rgb ) ;

	LVector3d	valRet ;
	// valRet = vectorFromColor(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// const Matrix3d* getMatrixParameter( ulong iParam ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneParameter_getMatrixParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneParameter, pThis ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;

	LMatrix3d	valRet ;
	// valRet = pThis->getMatrixParameter(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// const Vector3d* getVectorParameter( ulong iParam ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneParameter_getVectorParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneParameter, pThis ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;

	LVector3d	valRet ;
	// valRet = pThis->getVectorParameter(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// double getScalarParameter( ulong iParam ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneParameter_getScalarParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneParameter, pThis ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;

	LDouble	valRet ;
	// valRet = pThis->getScalarParameter(...) ;

	LQT_RETURN_DOUBLE( valRet ) ;
}

// int getIntegerParameter( ulong iParam ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneParameter_getIntegerParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneParameter, pThis ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;

	LInt32	valRet ;
	// valRet = pThis->getIntegerParameter(...) ;

	LQT_RETURN_INT( valRet ) ;
}

// boolean getBooleanParameter( ulong iParam ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneParameter_getBooleanParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneParameter, pThis ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;

	LBoolean	valRet ;
	// valRet = pThis->getBooleanParameter(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// String getCommandParameter( ulong iParam ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneParameter_getCommandParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneParameter, pThis ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;

	LString	valRet ;
	// valRet = pThis->getCommandParameter(...) ;

	LQT_RETURN_STRING( valRet ) ;
}

// const Vector2d* getVector2dParameter( ulong iParam ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneParameter_getVector2dParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneParameter, pThis ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;

	LVector2d	valRet ;
	// valRet = pThis->getVector2dParameter(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// const Vector4d* getVector4dParameter( ulong iParam ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneParameter_getVector4dParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneParameter, pThis ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;

	LVector4d	valRet ;
	// valRet = pThis->getVector4dParameter(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// const Matrix4d* getMatrix4dParameter( ulong iParam ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneParameter_getMatrix4dParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneParameter, pThis ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;

	LMatrix4d	valRet ;
	// valRet = pThis->getMatrix4dParameter(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// ulong getBinaryParameter( void* pDst, ulong nBufBytes, ulong iParam ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneParameter_getBinaryParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneParameter, pThis ) ;
	LQT_FUNC_ARG_POINTER( void, pDst ) ;
	LQT_VERIFY_NULL_PTR( pDst ) ;
	LQT_FUNC_ARG_ULONG( nBufBytes ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;

	LUint64	valRet ;
	// valRet = pThis->getBinaryParameter(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// void setMatrixParameter( ulong iParam, const Matrix3d* mat )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneParameter_setMatrixParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneParameter, pThis ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix3d, mat ) ;
	LQT_VERIFY_NULL_PTR( mat ) ;

	// pThis->setMatrixParameter(...) ;

	LQT_RETURN_VOID() ;
}

// void setVectorParameter( ulong iParam, const Vector3d* vec )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneParameter_setVectorParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneParameter, pThis ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vec ) ;
	LQT_VERIFY_NULL_PTR( vec ) ;

	// pThis->setVectorParameter(...) ;

	LQT_RETURN_VOID() ;
}

// void setScalarParameter( ulong iParam, double s )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneParameter_setScalarParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneParameter, pThis ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;
	LQT_FUNC_ARG_DOUBLE( s ) ;

	// pThis->setScalarParameter(...) ;

	LQT_RETURN_VOID() ;
}

// void setIntegerParameter( ulong iParam, int n )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneParameter_setIntegerParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneParameter, pThis ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;
	LQT_FUNC_ARG_INT( n ) ;

	// pThis->setIntegerParameter(...) ;

	LQT_RETURN_VOID() ;
}

// void setBooleanParameter( ulong iParam, boolean b )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneParameter_setBooleanParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneParameter, pThis ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;
	LQT_FUNC_ARG_BOOL( b ) ;

	// pThis->setBooleanParameter(...) ;

	LQT_RETURN_VOID() ;
}

// void setCommandParameter( ulong iParam, String cmd )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneParameter_setCommandParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneParameter, pThis ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;
	LQT_FUNC_ARG_STRING( cmd ) ;

	// pThis->setCommandParameter(...) ;

	LQT_RETURN_VOID() ;
}

// void setVector2dParameter( ulong iParam, const Vector2d* vec )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneParameter_setVector2dParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneParameter, pThis ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;
	LQT_FUNC_ARG_STRUCT( LVector2d, vec ) ;
	LQT_VERIFY_NULL_PTR( vec ) ;

	// pThis->setVector2dParameter(...) ;

	LQT_RETURN_VOID() ;
}

// void setVector4dParameter( ulong iParam, const Vector4d* vec )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneParameter_setVector4dParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneParameter, pThis ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;
	LQT_FUNC_ARG_STRUCT( LVector4d, vec ) ;
	LQT_VERIFY_NULL_PTR( vec ) ;

	// pThis->setVector4dParameter(...) ;

	LQT_RETURN_VOID() ;
}

// void setMatrix4dParameter( ulong iParam, const Matrix4d* mat )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneParameter_setMatrix4dParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneParameter, pThis ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;
	LQT_FUNC_ARG_STRUCT( LMatrix4d, mat ) ;
	LQT_VERIFY_NULL_PTR( mat ) ;

	// pThis->setMatrix4dParameter(...) ;

	LQT_RETURN_VOID() ;
}

// ulong setBinaryParameter( ulong iParam, const void* pSrc, ulong nBufBytes )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneParameter_setBinaryParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneParameter, pThis ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;
	LQT_FUNC_ARG_POINTER( void, pSrc ) ;
	LQT_VERIFY_NULL_PTR( pSrc ) ;
	LQT_FUNC_ARG_ULONG( nBufBytes ) ;

	LUint64	valRet ;
	// valRet = pThis->setBinaryParameter(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// boolean enumerateStringSet( ulong iParam, String[] strSet )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneParameter_enumerateStringSet)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneParameter, pThis ) ;
	LQT_FUNC_ARG_ULONG( iParam ) ;
	LQT_FUNC_ARG_OBJECT( LArrayObj, strSet ) ;
	LQT_VERIFY_NULL_PTR( strSet ) ;

	LBoolean	valRet ;
	// valRet = pThis->enumerateStringSet(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}



