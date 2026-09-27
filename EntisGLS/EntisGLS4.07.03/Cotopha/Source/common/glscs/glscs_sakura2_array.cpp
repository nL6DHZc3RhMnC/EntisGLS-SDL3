
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/


#include <sakuraglx/sakuraglx.h>
#include <sakura/ssys_module.h>

using	namespace SSystem ;
using	namespace ECSSakura2 ;
using	namespace ECSSakura2Processor ;


// 配列長設定
//////////////////////////////////////////////////////////////////////////////
BYTE * SSystem_Array::AllocateArray
	( size_t nLength, size_t nElement, VirtualMachine * vm )
{
	if ( m_ptrArray != 0 )
	{
		m_ptrArray =
			vm->ReallocateHeapMemory
				( m_ptrArray, (DWORD) (nLength * nElement) ) ;
	}
	else
	{
		m_ptrArray = vm->AllocateHeapMemory( (DWORD) (nLength * nElement) ) ;
	}
	m_nLength = (DWORD) nLength ;
	m_nBufSize = (DWORD) nLength ;
	return	vm->TranslateAddress( m_ptrArray, nLength * nElement ) ;
}


