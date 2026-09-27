
#include <gls.h>

using namespace SSystem ;


// 開始エントリ
//////////////////////////////////////////////////////////////////////////////
int main( const wchar_t * pwszArg )
{
	SetMemoryAllocationMode( mallocModeGlobal ) ;
	int	codeExit = sglMain( pwszArg ) ;
	sglStaticFinalize() ;
	SakuraGL::Finalize() ;
	return	codeExit ;
}

// 初期化処理
//////////////////////////////////////////////////////////////////////////////
SError StaticInitialize( void )
{
	SakuraGL::Initialize() ;
	//
	SakuraGL::SGLError	err ;
	SetMemoryAllocationMode( mallocModeShared ) ;
	err = sglStaticInitialize() ;
	SetMemoryAllocationMode( mallocModeGlobal ) ;
	//
	return	(SError) err ;
}

