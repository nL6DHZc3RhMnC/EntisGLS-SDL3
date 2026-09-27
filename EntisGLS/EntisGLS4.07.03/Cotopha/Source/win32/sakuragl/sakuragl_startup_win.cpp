
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
	return	codeExit ;
}

