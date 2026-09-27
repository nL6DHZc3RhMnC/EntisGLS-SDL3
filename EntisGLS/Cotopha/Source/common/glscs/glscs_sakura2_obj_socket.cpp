
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/


#include <sakuraglx/sakuraglx.h>
#include <sakura/ssys_socket.h>
#include <sakura/ssys_module.h>
#include <glscs/glscs_sakura2_obj_file.h>
#include <glscs/glscs_sakura2_obj_socket.h>

using	namespace SSystem ;
using	namespace ECSSakura2 ;
using	namespace ECSSakura2Processor ;


//////////////////////////////////////////////////////////////////////////////
// ソケット・オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ECSSakura2::SocketObject, FileObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SocketObject::SocketObject( void )
	: FileObject( new SSocket, true )
{
}

// 実行時型名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * SocketObject::GetTypeName( void ) const
{
	return	L"SSystem::Socket" ;
}


//////////////////////////////////////////////////////////////////////////////
// SSystem::Socket スタブ
//////////////////////////////////////////////////////////////////////////////

#if	!defined(ENTISGLS4_DLL_IMPORT)

// new SSystem::Socket
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_NEW_OBJECT( SSystem_Socket, context, cls_id )
{
	return	new SocketObject ;
}

// SError Create
//	( uint32_t nPort = 0, int64_t nFlags = 0,
//			const wchar_t * pwszAddress = NULL ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SSystem_Socket_Create,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SSocket, pSocket, arg, Socket::Create ) ;
	const uint16_t *	pszAddr =
		(const uint16_t*) context->AtomicTranslateAddress( arg[3].i ) ;
	//
	const wchar_t *	pwszAddr = NULL ;
	SString			strAddr ;
	if ( pszAddr != NULL )
	{
		strAddr = pszAddr ;
		pwszAddr = strAddr ;
	}
	//
	context->m_regset[regAcc].i =
		pSocket->Create( (uint32_t) arg[1].i, arg[2].i, pwszAddr ) ;
	//
	return	NULL ;
}

// void Close( void ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SSystem_Socket_Close,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SSocket, pSocket, arg, Socket::Close ) ;
	//
	pSocket->Close() ;
	//
	return	NULL ;
}

// SError Connect
//	( const wchar_t * pwszHostAddress, uint32_t nHostPort ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SSystem_Socket_Connect,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SSocket, pSocket, arg, Socket::Connect ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, const uint16_t, pszAddr,
				arg[1].i, pwszHostAddress at Socket::Connect ) ;
	//
	SString			strAddr = pszAddr ;
	context->m_regset[regAcc].i =
		pSocket->Connect( strAddr, (uint32_t) arg[2].i ) ;
	//
	return	NULL ;
}

// SError Listen( int nConnectionBacklog = 5 ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SSystem_Socket_Listen,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SSocket, pSocket, arg, Socket::Listen ) ;
	//
	context->m_regset[regAcc].i = pSocket->Listen( (int) arg[1].i ) ;
	//
	return	NULL ;
}

// SError Accept( Socket * socket ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SSystem_Socket_Accept,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SSocket, pSocket, arg, Socket::Accept ) ;
	ECS_DECLARE_SYSCALL_OBJECT
		( vm, SSocket, pAcceptSocket, arg[1].i, Socket::Accept ) ;
	//
	context->m_regset[regAcc].i = pSocket->Accept( *pAcceptSocket ) ;
	//
	return	NULL ;
}

// int64_t Poll( int64_t nFlags = 0, int64_t msecTimeout = 0 ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SSystem_Socket_Poll,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SSocket, pSocket, arg, Socket::Poll ) ;
	//
	context->m_regset[regAcc].i = pSocket->Poll( arg[1].i, arg[2].i ) ;
	//
	return	NULL ;
}

// size_t ReceiveFrom
//	( void * ptrBuf, size_t nBytes,
//			void * ptrAddrFrom, uint32_t& nAddrBytes ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SSystem_Socket_ReceiveFrom,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SSocket, pSocket, arg, Socket::ReceiveFrom ) ;
	//
	context->m_regset[regAcc].i = 0 ;
	if ( arg[2].i != 0 )
	{
		size_t	nBytes = (size_t) arg[2].i ;
		void *	ptrBuf =
			context->AtomicTranslateAddress( arg[1].i, nBytes ) ;
		if ( ptrBuf == NULL )
		{
			return	L"invalid buffer pointer at Socket::ReceiveFrom" ;
		}
		uint32_t *	pAddrBytes =
			(uint32_t*) context->AtomicTranslateAddress
								( arg[4].i, sizeof(uint32_t) ) ;
		size_t	nAddrBytes = 0 ;
		void *	ptrAddrFrom = NULL ;
		if ( pAddrBytes != NULL )
		{
			nAddrBytes = *pAddrBytes ;
			ptrAddrFrom =
				context->AtomicTranslateAddress( arg[3].i, nAddrBytes ) ;
		}
		context->m_regset[regAcc].i =
			pSocket->ReceiveFrom( ptrBuf, nBytes, ptrAddrFrom, nAddrBytes ) ;
		if ( pAddrBytes != NULL )
		{
			*pAddrBytes = (uint32_t) nAddrBytes ;
		}
	}
	return	NULL ;
}

// size_t SendTo
//	( void * ptrBuf, size_t nBytes,
//			void * ptrAddrTo, size_t nAddrBytes ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SSystem_Socket_SendTo,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SSocket, pSocket, arg, Socket::SendTo ) ;
	//
	context->m_regset[regAcc].i = 0 ;
	if ( arg[2].i != 0 )
	{
		size_t	nBytes = (size_t) arg[2].i ;
		void *	ptrBuf =
			context->AtomicTranslateAddress( arg[1].i, nBytes ) ;
		if ( ptrBuf == NULL )
		{
			return	L"invalid buffer pointer at Socket::SendTo" ;
		}
		size_t	nAddrBytes = (size_t) arg[4].i ;
		void *	ptrAddrTo =
				context->AtomicTranslateAddress( arg[3].i, nAddrBytes ) ;
		//
		context->m_regset[regAcc].i =
			pSocket->SendTo( ptrBuf, nBytes, ptrAddrTo, nAddrBytes ) ;
	}
	return	NULL ;
}

// SError GetAcceptedCleintIP( SArray<uint16_t> & strAddress ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SSystem_Socket_GetAcceptedCleintIP,context,arg)
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, SSocket, pSocket, arg, Socket::GetAcceptedCleintIP ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, SSystem_Array, pAddrIP,
				arg[1].i, strAddress at Socket::GetAcceptedCleintIP ) ;
	//
	SString	strAddrIP ;
	SError	err = pSocket->GetAcceptedCleintIP( strAddrIP ) ;
	if ( !err )
	{
		size_t	nCount = strAddrIP.GetLength() ;
		uint16_t *	pStrArray =
			(uint16_t*) pAddrIP->AllocateArray
					( nCount + 1, sizeof(uint16_t), vm ) ;
		if ( pStrArray != NULL )
		{
			const uint16_t *	pszAddrIP = strAddrIP.GetConstArray() ;
			pAddrIP->m_nLength = (DWORD) nCount ;
			for ( size_t i = 0; i <= nCount; i ++ )
			{
				pStrArray[i] = pszAddrIP[i] ;
			}
		}
	}
	context->m_regset[regAcc].i = err ;
	return	NULL ;
}

// static SError GetLocalMachineIP( SArray<uint16_t> & strAddrIP ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL(SSystem_Socket_GetLocalMachineIP,context,arg)
{
	ECS_DECLARE_SYSCALL_VM( context, vm ) ;
	ECS_DECLARE_SYSCALL_PTRVAR
		( context, SSystem_Array, pAddrIP,
				arg[0].i, strAddrIP at Socket::GetLocalMachineIP ) ;
	//
	SString	strAddrIP ;
	SError	err = SSocket::GetLocalMachineIP( strAddrIP ) ;
	if ( !err )
	{
		size_t	nCount = strAddrIP.GetLength() ;
		uint16_t *	pStrArray =
			(uint16_t*) pAddrIP->AllocateArray
					( nCount + 1, sizeof(uint16_t), vm ) ;
		if ( pStrArray != NULL )
		{
			const uint16_t *	pszAddrIP = strAddrIP.GetConstArray() ;
			pAddrIP->m_nLength = (DWORD) nCount ;
			for ( size_t i = 0; i <= nCount; i ++ )
			{
				pStrArray[i] = pszAddrIP[i] ;
			}
		}
	}
	context->m_regset[regAcc].i = err ;
	return	NULL ;
}

#endif
