
/*****************************************************************************
                          Sakura2 Library
 ****************************************************************************/

#include <sakura/sakura.h>

using namespace SSystem ;


//////////////////////////////////////////////////////////////////////////////
// ライブラリ初期化
//////////////////////////////////////////////////////////////////////////////

PLATFORM_INFORMATION	SSystem::g_infoPlatform ;
SSystem::CPU_Family		SSystem::g_cpuFamily ;
uint64_t				SSystem::g_cpuFeatures ;
unsigned int			SSystem::g_cpuLogicalCount ;

static atomic_int_t	g_countRefSystem = 0 ;


void SSystem::Initialize( void )
{
	if ( AtomicAdd( &g_countRefSystem, 1 ) == 1 )
	{
		GetPlatformInformation( g_infoPlatform ) ;
		g_cpuFamily = GetCPUFamily() ;
		g_cpuFeatures = GetCPUFeatures() ;
		g_cpuLogicalCount = GetLogicalProcessorCount() ;
		SFileOpener::SetDefaultOpener( &g_defURLOpener ) ;
	}
}

void SSystem::Finalize( void )
{
	if ( AtomicSub( &g_countRefSystem, 1 ) == 0 )
	{
		SThread::ExitAllStockedThread() ;
		g_defURLOpener.UnregisterAllScheme() ;
	}
}


