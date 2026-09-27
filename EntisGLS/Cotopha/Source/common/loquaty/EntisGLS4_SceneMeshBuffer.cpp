
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_SceneMeshBuffer.h>


// SceneMeshBuffer( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_SceneMeshBuffer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ
		( LEntisGLS4_SceneMeshBuffer, pThis,
			( new SSmartObject
				( (S3DSceneComposer::ItemSerializer*)
						new S3DMeshBufferItemSerializer ) ) ) ;

	LQT_RETURN_VOID() ;
}

// EntisGLS4.VertexBuffer[] lockMeshBuffer( )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneMeshBuffer_lockMeshBuffer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneMeshBuffer, pThis ) ;
	S3DMeshBufferItemSerializer *	pMesh = pThis->GetRef<S3DMeshBufferItemSerializer>() ;
	LQT_VERIFY_NULL_PTR( pMesh ) ;

	LClass *	pVBClass = LQT_GET_CLASS(EntisGLS4.VertexBuffer) ;
	LQT_VERIFY_NULL_PTR( pVBClass ) ;

	pMesh->GetInstanceLocker()->Lock() ;

	size_t	nTargetCount = 0 ;
	S3DVertexBufferInterface **
			ppVBs = pMesh->GetTargetVertexBuffers( nTargetCount ) ;

	LPtr<LArrayObj>	pArray( _context.new_Array( pVBClass ) ) ;
	for ( size_t i = 0; i < nTargetCount; i ++ )
	{
		if ( ppVBs[i] != nullptr )
		{
			LPtr<LNativeObj>	pVB( new LNativeObj( pVBClass ) ) ;
			pVB->SetNative
				( std::make_shared<LEntisGLS4_VertexBuffer>
						( (S3DRenderBufferInterface*) ppVBs[i] ) ) ;
			LObject::ReleaseRef( pArray->SetElementAt( i, pVB.Get() ) ) ;
		}
	}

	LQT_RETURN_OBJECT( pArray ) ;
}

// void unlockMeshBuffer( )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneMeshBuffer_unlockMeshBuffer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneMeshBuffer, pThis ) ;
	S3DMeshBufferItemSerializer *	pMesh = pThis->GetRef<S3DMeshBufferItemSerializer>() ;
	LQT_VERIFY_NULL_PTR( pMesh ) ;

	pMesh->GetInstanceLocker()->Unlock() ;

	LQT_RETURN_VOID() ;
}



