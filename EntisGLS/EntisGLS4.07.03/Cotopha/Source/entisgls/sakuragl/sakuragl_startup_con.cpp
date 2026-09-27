
#include <gls.h>

using namespace SSystem ;


// 開始エントリ
//////////////////////////////////////////////////////////////////////////////
int main( int argc, char *argv[], char *envp[] )
{
	int	codeExit = 0 ;
#if	!defined(DISABLE_ENTIS_GLS3)
	::glsInitializeLibrary() ;
	ECotophaScript::Initialize() ;
#endif
	SakuraGL::Initialize() ;
	{
		SStringParser	spCmdLine = GetCommandLine() ;
		spCmdLine.GetStringTerm() ;
		//
		SetMemoryAllocationMode( mallocModeShared ) ;
		if ( sglStaticInitialize() == SakuraGL::sglErrSuccess )
		{
			SetMemoryAllocationMode( mallocModeGlobal ) ;
			codeExit = sglMain( spCmdLine.SubString( spCmdLine.GetIndex() ) ) ;
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

