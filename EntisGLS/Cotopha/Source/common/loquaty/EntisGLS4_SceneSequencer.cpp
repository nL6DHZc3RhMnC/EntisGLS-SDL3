
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_SceneSequencer.h>


// int getInterpolateMethod( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSequencer_getInterpolateMethod)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSequencer, pThis ) ;
	S3DSceneComposer::Sequencer *	pSeq = pThis->GetRef<S3DSceneComposer::Sequencer>() ;
	LQT_VERIFY_NULL_PTR( pSeq ) ;

	LQT_RETURN_INT( pSeq->GetInterpolateMethod() ) ;
}

// boolean getKeyFrameRange( int* iFirst, int* iEnd ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSequencer_getKeyFrameRange)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSequencer, pThis ) ;
	S3DSceneComposer::Sequencer *	pSeq = pThis->GetRef<S3DSceneComposer::Sequencer>() ;
	LQT_VERIFY_NULL_PTR( pSeq ) ;
	LQT_FUNC_ARG_POINTER( LInt32, iFirst ) ;
	LQT_VERIFY_NULL_PTR( iFirst ) ;
	LQT_FUNC_ARG_POINTER( LInt32, iEnd ) ;
	LQT_VERIFY_NULL_PTR( iEnd ) ;

	LQT_RETURN_BOOL( pSeq->GetKeyFrameRange( *iFirst, *iEnd ) ) ;
}

// boolean getKeyFrameParameter( ulong index, EntisGLS4.SceneSequencer.KeyFrameParam* kfp ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSequencer_getKeyFrameParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSequencer, pThis ) ;
	S3DSceneComposer::Sequencer *	pSeq = pThis->GetRef<S3DSceneComposer::Sequencer>() ;
	LQT_VERIFY_NULL_PTR( pSeq ) ;
	LQT_FUNC_ARG_ULONG( index ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_SceneSequencer_KeyFrameParam, kfp ) ;
	LQT_VERIFY_NULL_PTR( kfp ) ;

	LQT_RETURN_BOOL( pSeq->GetKeyFrameParameter( (size_t) index, *kfp ) ) ;
}

// boolean setKeyFrameParameter( ulong index, const EntisGLS4.SceneSequencer.KeyFrameParam* kfp )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSequencer_setKeyFrameParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSequencer, pThis ) ;
	S3DSceneComposer::Sequencer *	pSeq = pThis->GetRef<S3DSceneComposer::Sequencer>() ;
	LQT_VERIFY_NULL_PTR( pSeq ) ;
	LQT_FUNC_ARG_ULONG( index ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_SceneSequencer_KeyFrameParam, kfp ) ;
	LQT_VERIFY_NULL_PTR( kfp ) ;

	LQT_RETURN_BOOL( pSeq->SetKeyFrameParameter( (size_t) index, *kfp ) ) ;
}

// ulong getKeyFrameCount( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSequencer_getKeyFrameCount)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSequencer, pThis ) ;
	S3DSceneComposer::Sequencer *	pSeq = pThis->GetRef<S3DSceneComposer::Sequencer>() ;
	LQT_VERIFY_NULL_PTR( pSeq ) ;

	LQT_RETURN_ULONG( pSeq->GetKeyFrameCount() ) ;
}

// void insertKeyFrame( ulong index, const EntisGLS4.SceneSequencer.KeyFrameParam* kfp )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSequencer_insertKeyFrame)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSequencer, pThis ) ;
	S3DSceneComposer::Sequencer *	pSeq = pThis->GetRef<S3DSceneComposer::Sequencer>() ;
	LQT_VERIFY_NULL_PTR( pSeq ) ;
	LQT_FUNC_ARG_ULONG( index ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_SceneSequencer_KeyFrameParam, kfp ) ;
	LQT_VERIFY_NULL_PTR( kfp ) ;

	pSeq->InsertKeyFrame( (size_t) index, *kfp ) ;

	LQT_RETURN_VOID() ;
}

// void removeKeyFrame( ulong index )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSequencer_removeKeyFrame)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSequencer, pThis ) ;
	S3DSceneComposer::Sequencer *	pSeq = pThis->GetRef<S3DSceneComposer::Sequencer>() ;
	LQT_VERIFY_NULL_PTR( pSeq ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	pSeq->RemoveKeyFrame( (size_t) index ) ;

	LQT_RETURN_VOID() ;
}

// void updateAllFrameValues( )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSequencer_updateAllFrameValues)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSequencer, pThis ) ;
	S3DSceneComposer::Sequencer *	pSeq = pThis->GetRef<S3DSceneComposer::Sequencer>() ;
	LQT_VERIFY_NULL_PTR( pSeq ) ;

	pSeq->UpdateAllFrameValues() ;

	LQT_RETURN_VOID() ;
}

// boolean normalizeKeyFrameOrder( )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSequencer_normalizeKeyFrameOrder)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSequencer, pThis ) ;
	S3DSceneComposer::Sequencer *	pSeq = pThis->GetRef<S3DSceneComposer::Sequencer>() ;
	LQT_VERIFY_NULL_PTR( pSeq ) ;

	LQT_RETURN_BOOL( pSeq->NormalizeKeyFrameOrder() ) ;
}

// void swapKeyFrame( ulong index1, ulong index2 )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSequencer_swapKeyFrame)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSequencer, pThis ) ;
	S3DSceneComposer::Sequencer *	pSeq = pThis->GetRef<S3DSceneComposer::Sequencer>() ;
	LQT_VERIFY_NULL_PTR( pSeq ) ;
	LQT_FUNC_ARG_ULONG( index1 ) ;
	LQT_FUNC_ARG_ULONG( index2 ) ;

	pSeq->SwapKeyFrame( (size_t) index1, (size_t) index2 ) ;

	LQT_RETURN_VOID() ;
}

// ulong orderKeyFrameIndex( int iFrame ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSequencer_orderKeyFrameIndex)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSequencer, pThis ) ;
	S3DSceneComposer::Sequencer *	pSeq = pThis->GetRef<S3DSceneComposer::Sequencer>() ;
	LQT_VERIFY_NULL_PTR( pSeq ) ;
	LQT_FUNC_ARG_INT( iFrame ) ;

	LQT_RETURN_ULONG( pSeq->OrderKeyFrameIndex( iFrame ) ) ;
}

// long findKeyFrame( int iFrame ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSequencer_findKeyFrame)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSequencer, pThis ) ;
	S3DSceneComposer::Sequencer *	pSeq = pThis->GetRef<S3DSceneComposer::Sequencer>() ;
	LQT_VERIFY_NULL_PTR( pSeq ) ;
	LQT_FUNC_ARG_INT( iFrame ) ;

	LQT_RETURN_LONG( pSeq->FindKeyFrame( iFrame ) ) ;
}

// ulong getKeyFrameProgress( double* t, int iFrame ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSequencer_getKeyFrameProgress)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSequencer, pThis ) ;
	S3DSceneComposer::Sequencer *	pSeq = pThis->GetRef<S3DSceneComposer::Sequencer>() ;
	LQT_VERIFY_NULL_PTR( pSeq ) ;
	LQT_FUNC_ARG_POINTER( LDouble, t ) ;
	LQT_VERIFY_NULL_PTR( t ) ;
	LQT_FUNC_ARG_INT( iFrame ) ;

	LQT_RETURN_ULONG( pSeq->GetKeyFrameProgress( *t, iFrame ) ) ;
}

// const Matrix3d* getFrameMatrix( double frame )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSequencer_getFrameMatrix)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSequencer, pThis ) ;
	S3DSceneComposer::Sequencer *	pSeq = pThis->GetRef<S3DSceneComposer::Sequencer>() ;
	LQT_VERIFY_NULL_PTR( pSeq ) ;
	LQT_FUNC_ARG_DOUBLE( frame ) ;

	LMatrix3d	valRet = pSeq->GetFrameMatrix( frame ) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// const Vector3d* getFrameVector( double frame )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSequencer_getFrameVector)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSequencer, pThis ) ;
	S3DSceneComposer::Sequencer *	pSeq = pThis->GetRef<S3DSceneComposer::Sequencer>() ;
	LQT_VERIFY_NULL_PTR( pSeq ) ;
	LQT_FUNC_ARG_DOUBLE( frame ) ;

	LVector3d	valRet = pSeq->GetFrameVector( frame ) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// double getFrameScalar( double frame )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSequencer_getFrameScalar)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSequencer, pThis ) ;
	S3DSceneComposer::Sequencer *	pSeq = pThis->GetRef<S3DSceneComposer::Sequencer>() ;
	LQT_VERIFY_NULL_PTR( pSeq ) ;
	LQT_FUNC_ARG_DOUBLE( frame ) ;

	LQT_RETURN_DOUBLE( pSeq->GetFrameScalar( frame ) ) ;
}

// int getFrameInteger( double frame )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSequencer_getFrameInteger)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSequencer, pThis ) ;
	S3DSceneComposer::Sequencer *	pSeq = pThis->GetRef<S3DSceneComposer::Sequencer>() ;
	LQT_VERIFY_NULL_PTR( pSeq ) ;
	LQT_FUNC_ARG_DOUBLE( frame ) ;

	LQT_RETURN_INT( pSeq->GetFrameInteger( frame ) ) ;
}

// boolean getFrameBoolean( double frame )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSequencer_getFrameBoolean)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSequencer, pThis ) ;
	S3DSceneComposer::Sequencer *	pSeq = pThis->GetRef<S3DSceneComposer::Sequencer>() ;
	LQT_VERIFY_NULL_PTR( pSeq ) ;
	LQT_FUNC_ARG_DOUBLE( frame ) ;

	LQT_RETURN_BOOL( pSeq->GetFrameBoolean( frame ) ) ;
}

// String getFrameCommand( double frame, int seek )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneSequencer_getFrameCommand)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneSequencer, pThis ) ;
	S3DSceneComposer::Sequencer *	pSeq = pThis->GetRef<S3DSceneComposer::Sequencer>() ;
	LQT_VERIFY_NULL_PTR( pSeq ) ;
	LQT_FUNC_ARG_DOUBLE( frame ) ;
	LQT_FUNC_ARG_INT( seek ) ;

	LQT_RETURN_STRING
		( pSeq->GetFrameCommand( frame, (S3DSceneComposer::SeekMethod) seek ) ) ;
}



