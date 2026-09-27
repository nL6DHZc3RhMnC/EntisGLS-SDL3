
#include <gls.h>

using namespace SSystem ;


// 開始エントリ
//////////////////////////////////////////////////////////////////////////////
int main( int argc, char *argv[], char *envp[] )
{
	int	codeExit = 0 ;
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
	return	codeExit ;
}

