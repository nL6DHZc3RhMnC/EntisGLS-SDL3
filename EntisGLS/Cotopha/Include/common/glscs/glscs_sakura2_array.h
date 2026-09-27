
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/

#if	!defined(__GLSCS_SAKURA2_ARRAY_H__)
#define	__GLSCS_SAKURA2_ARRAY_H__

namespace	ECSSakura2
{
	//////////////////////////////////////////////////////////////////////////
	// SSystem::Array
	//////////////////////////////////////////////////////////////////////////

	struct	SSystem_Array
	{
	public:
		INT64	m_ptrArray ;
		DWORD	m_nLength ;
		DWORD	m_nBufSize ;

	public:
		// 配列長設定
		BYTE * AllocateArray
			( size_t nLength, size_t nElement, VirtualMachine * vm ) ;
	} ;

}

#endif
