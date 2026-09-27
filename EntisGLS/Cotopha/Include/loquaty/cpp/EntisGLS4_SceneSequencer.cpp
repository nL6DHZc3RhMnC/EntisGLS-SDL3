
#include <loquaty.h>
#include "EntisGLS4_SceneSequencer.h"

using namespace Loquaty ;


// EntisGLS4.SceneSequencer.InterpolateMethod getInterpolateMethod( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSequencer_getInterpolateMethod)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSequencer, pThis ) ;

	LInt32	valRet ;
	// valRet = pThis->getInterpolateMethod(...) ;

	LQT_RETURN_INT( valRet ) ;
}

// boolean getKeyFrameRange( int* iFirst, int* iEnd ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSequencer_getKeyFrameRange)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSequencer, pThis ) ;
	LQT_FUNC_ARG_POINTER( LInt32, iFirst ) ;
	LQT_VERIFY_NULL_PTR( iFirst ) ;
	LQT_FUNC_ARG_POINTER( LInt32, iEnd ) ;
	LQT_VERIFY_NULL_PTR( iEnd ) ;

	LBoolean	valRet ;
	// valRet = pThis->getKeyFrameRange(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean getKeyFrameParameter( ulong index, EntisGLS4.SceneSequencer.KeyFrameParam* kfp ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSequencer_getKeyFrameParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSequencer, pThis ) ;
	LQT_FUNC_ARG_ULONG( index ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_SceneSequencer_KeyFrameParam, kfp ) ;
	LQT_VERIFY_NULL_PTR( kfp ) ;

	LBoolean	valRet ;
	// valRet = pThis->getKeyFrameParameter(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean setKeyFrameParameter( ulong index, const EntisGLS4.SceneSequencer.KeyFrameParam* kfp )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSequencer_setKeyFrameParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSequencer, pThis ) ;
	LQT_FUNC_ARG_ULONG( index ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_SceneSequencer_KeyFrameParam, kfp ) ;
	LQT_VERIFY_NULL_PTR( kfp ) ;

	LBoolean	valRet ;
	// valRet = pThis->setKeyFrameParameter(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// ulong getKeyFrameCount( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSequencer_getKeyFrameCount)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSequencer, pThis ) ;

	LUint64	valRet ;
	// valRet = pThis->getKeyFrameCount(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// void insertKeyFrame( ulong index, const EntisGLS4.SceneSequencer.KeyFrameParam* kfp )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSequencer_insertKeyFrame)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSequencer, pThis ) ;
	LQT_FUNC_ARG_ULONG( index ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_SceneSequencer_KeyFrameParam, kfp ) ;
	LQT_VERIFY_NULL_PTR( kfp ) ;

	// pThis->insertKeyFrame(...) ;

	LQT_RETURN_VOID() ;
}

// void removeKeyFrame( ulong index )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSequencer_removeKeyFrame)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSequencer, pThis ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	// pThis->removeKeyFrame(...) ;

	LQT_RETURN_VOID() ;
}

// void updateAllFrameValues( )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSequencer_updateAllFrameValues)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSequencer, pThis ) ;

	// pThis->updateAllFrameValues(...) ;

	LQT_RETURN_VOID() ;
}

// boolean normalizeKeyFrameOrder( )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSequencer_normalizeKeyFrameOrder)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSequencer, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->normalizeKeyFrameOrder(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void swapKeyFrame( ulong index1, ulong index2 )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSequencer_swapKeyFrame)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSequencer, pThis ) ;
	LQT_FUNC_ARG_ULONG( index1 ) ;
	LQT_FUNC_ARG_ULONG( index2 ) ;

	// pThis->swapKeyFrame(...) ;

	LQT_RETURN_VOID() ;
}

// ulong orderKeyFrameIndex( int iFrame ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSequencer_orderKeyFrameIndex)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSequencer, pThis ) ;
	LQT_FUNC_ARG_INT( iFrame ) ;

	LUint64	valRet ;
	// valRet = pThis->orderKeyFrameIndex(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// long findKeyFrame( int iFrame ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSequencer_findKeyFrame)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSequencer, pThis ) ;
	LQT_FUNC_ARG_INT( iFrame ) ;

	LInt64	valRet ;
	// valRet = pThis->findKeyFrame(...) ;

	LQT_RETURN_LONG( valRet ) ;
}

// ulong getKeyFrameProgress( double* t, int iFrame ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSequencer_getKeyFrameProgress)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSequencer, pThis ) ;
	LQT_FUNC_ARG_POINTER( LDouble, t ) ;
	LQT_VERIFY_NULL_PTR( t ) ;
	LQT_FUNC_ARG_INT( iFrame ) ;

	LUint64	valRet ;
	// valRet = pThis->getKeyFrameProgress(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// const Matrix3d* getFrameMatrix( double frame )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSequencer_getFrameMatrix)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSequencer, pThis ) ;
	LQT_FUNC_ARG_DOUBLE( frame ) ;

	LMatrix3d	valRet ;
	// valRet = pThis->getFrameMatrix(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// const Vector3d* getFrameVector( double frame )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSequencer_getFrameVector)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSequencer, pThis ) ;
	LQT_FUNC_ARG_DOUBLE( frame ) ;

	LVector3d	valRet ;
	// valRet = pThis->getFrameVector(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// double getFrameScalar( double frame )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSequencer_getFrameScalar)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSequencer, pThis ) ;
	LQT_FUNC_ARG_DOUBLE( frame ) ;

	LDouble	valRet ;
	// valRet = pThis->getFrameScalar(...) ;

	LQT_RETURN_DOUBLE( valRet ) ;
}

// int getFrameInteger( double frame )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSequencer_getFrameInteger)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSequencer, pThis ) ;
	LQT_FUNC_ARG_DOUBLE( frame ) ;

	LInt32	valRet ;
	// valRet = pThis->getFrameInteger(...) ;

	LQT_RETURN_INT( valRet ) ;
}

// boolean getFrameBoolean( double frame )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSequencer_getFrameBoolean)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSequencer, pThis ) ;
	LQT_FUNC_ARG_DOUBLE( frame ) ;

	LBoolean	valRet ;
	// valRet = pThis->getFrameBoolean(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// String getFrameCommand( double frame, EntisGLS4.SceneSequencer.SeekMethod seek )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSequencer_getFrameCommand)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSequencer, pThis ) ;
	LQT_FUNC_ARG_DOUBLE( frame ) ;
	LQT_FUNC_ARG_INT( seek ) ;

	LString	valRet ;
	// valRet = pThis->getFrameCommand(...) ;

	LQT_RETURN_STRING( valRet ) ;
}



