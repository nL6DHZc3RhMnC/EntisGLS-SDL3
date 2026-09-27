
#include <gls.h>

using namespace SSystem ;


// 開始エントリ
//////////////////////////////////////////////////////////////////////////////
int WINAPI WinMain
	( HINSTANCE hInstance,
		HINSTANCE hPrevInstance,
		LPSTR lpCmdLine, int nCmdShow )
{
	int	codeExit = 0 ;
#if	!defined(DISABLE_ENTIS_GLS3)
	::glsInitializeLibrary() ;
	ECotophaScript::Initialize() ;
#endif
	SakuraGL::Initialize() ;
	{
		SString	strCmdLine = lpCmdLine ;
		//
		SetMemoryAllocationMode( mallocModeShared ) ;
		if ( sglStaticInitialize() == SakuraGL::sglErrSuccess )
		{
			SetMemoryAllocationMode( mallocModeGlobal ) ;
			codeExit = sglMain( strCmdLine ) ;
			//
			sglStaticFinalize() ;
		}
	}
	SakuraGL::Finalize() ;
#if	!defined(DISABLE_ENTIS_GLS3)
	ECotophaScript::Release() ;
	::glsCloseLibrary() ;
#endif
	return	codeExit ;
}

