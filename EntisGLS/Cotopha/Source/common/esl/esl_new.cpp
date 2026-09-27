
/*****************************************************************************
                      operator new / delete 関数
 ****************************************************************************/


#ifndef	PLATFORM_ANDROID

#include <esl/esl_new.h>
#include <sakura/sakura.h>
#include <sakura/ssys_heap_memory.h>


void * operator new ( size_t nBytes )
{
	ESLAssert( SSystem::g_pStdHeap != NULL ) ;
	if ( SSystem::g_pStdHeap == NULL )
	{
		SSystem::eslHeapInitialize() ;
	}
	return	SSystem::eslHeapAllocate( nBytes ) ;
}

void * operator new ( size_t nBytes, const char * pszFileName, int nLine )
{
	ESLAssert( SSystem::g_pStdHeap != NULL ) ;
	if ( SSystem::g_pStdHeap == NULL )
	{
		SSystem::eslHeapInitialize() ;
	}
	return	SSystem::eslHeapAllocate( nBytes ) ;
}

void operator delete ( void * ptrObj )
{
	if ( ptrObj != NULL )
	{
		ESLAssert( SSystem::g_pStdHeap != NULL ) ;
		if ( SSystem::g_pStdHeap != NULL )
		{
			SSystem::eslHeapFree( ptrObj ) ;
		}
	}
}

#endif


