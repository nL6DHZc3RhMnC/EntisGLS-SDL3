
#pragma	comment( lib, "initgls.lib" )

extern "C" __declspec(dllimport) void DLL_InitGLS( void ) ;
class	classDLL_InitGLS
{
public:
	classDLL_InitGLS( void ) { DLL_InitGLS() ; }
}	g_DLL_InitGLS ;
