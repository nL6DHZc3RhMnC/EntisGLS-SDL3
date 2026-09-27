
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/

#if	!defined(__GLSCS_SAKURA2_OBJECT_SOCKET_H__)
#define	__GLSCS_SAKURA2_OBJECT_SOCKET_H__

namespace	ECSSakura2
{
	//////////////////////////////////////////////////////////////////////////
	// ソケット・オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	SocketObject	: public FileObject
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SocketObject, FileObject )
		// 構築関数
		SocketObject( void ) ;

	public:
		// 実行時型名
		virtual const wchar_t * GetTypeName( void ) const ;
	} ;

}


//////////////////////////////////////////////////////////////////////////////
// SSystem::Socket スタブ
//////////////////////////////////////////////////////////////////////////////

// new SSystem::Socket
ECS_LIB_DECLARE_EXPORT_NEW_OBJECT(SSystem_Socket) ;

// SError Create
//	( uint32_t nPort = 0, int64_t nFlags = 0,
//			const wchar_t * pwszAddress = NULL ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_Socket_Create) ;

// void Close( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_Socket_Close) ;

// SError Connect
//	( const wchar_t * pwszHostAddress, uint32_t nHostPort ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_Socket_Connect) ;

// SError Listen( int nConnectionBacklog = 5 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_Socket_Listen) ;

// SError Accept( Socket * socket ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_Socket_Accept) ;

// int64_t Poll( int64_t nFlags = 0, int64_t msecTimeout = 0 ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_Socket_Poll) ;

// size_t ReceiveFrom
//	( void * ptrBuf, size_t nBytes,
//			void * ptrAddrFrom, uint32_t& nAddrBytes ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_Socket_ReceiveFrom) ;

// size_t SendTo
//	( void * ptrBuf, size_t nBytes,
//			void * ptrAddrTo, size_t nAddrBytes ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_Socket_SendTo) ;

// SError GetAcceptedCleintIP( SArray<uint16_t> & strAddress ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_Socket_GetAcceptedCleintIP) ;

// static SError GetLocalMachineIP( SArray<uint16_t> & strAddrIP ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_Socket_GetLocalMachineIP) ;

#endif

