
#if	!defined(__ESL_NEW_H__)
#define	__ESL_NEW_H__

void * operator new ( size_t nBytes ) ;
void * operator new ( size_t nBytes, const char * pszFileName, int nLine ) ;
void operator delete ( void * ptrObj ) ;

#endif

