
/*****************************************************************************
             Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
	Copyright (c) 2002-2012 Leshade Entis, Entis-soft. All rights reserved.
 ****************************************************************************/


#include <gls.h>


//////////////////////////////////////////////////////////////////////////////
// Sakura2 実行コンテキスト
//////////////////////////////////////////////////////////////////////////////

// 128 bit アライメント new
//////////////////////////////////////////////////////////////////////////////
void * ECSContext::operator new ( size_t nBytes )
{
	size_t	offsetContext =
		offsetof(ECSContext,m_regset)
			- offsetof(ECSSakura2Processor::Context,m_regset) ;
	BYTE *	pbytBuf =
		(BYTE*) ::eslHeapAllocate( NULL, nBytes + 0x20 + offsetContext, 0 ) ;
	size_t	modContext = ((DWORD_PTR) (pbytBuf + offsetContext)) & 0x0F ;
	if ( modContext != 0 )
	{
		modContext = 0x10 - modContext ;
	}
	BYTE *	pbytObj = pbytBuf + (0x10 + modContext) ;
	((void**)pbytObj)[-1] = pbytBuf ;
	return	pbytObj ;
}

void ECSContext::operator delete ( void * pObj )
{
	::eslHeapFree( NULL, ((void**)pObj)[-1], 0 ) ;
}

// Sakura2 processor 初期化
//////////////////////////////////////////////////////////////////////////////
void ECSContext::InitializeSakuraProcessor( void )
{
	//
	// スタック割り当て
	//
	if ( m_vaNakedStack == 0 )
	{
		ESLAssert( m_bufNakedStack == NULL ) ;
		DWORD	dwInitStackSize =
					(m_pcsxi->m_exiHeader.nStackSize + 0x3FF) & ~0x3FF ;
		if ( dwInitStackSize == 0 )
		{
			dwInitStackSize = 0x400 ;
		}
		m_bufNakedStack = new ECSBuffer ;
		m_bufNakedStack->CreateBuffer
				( dwInitStackSize, Sakura2StackLimit - dwInitStackSize ) ;
		ECSSakura2Processor::AssertLock() ;
		m_vaNakedStack =
			m_pcsxi->AllocateHeapObjectAddress( m_bufNakedStack ) ;
		ECSSakura2Processor::AssertUnlock() ;
	}
	//
	// コンテキスト初期化
	//
	InitializeProcessor( m_vaNakedStack + Sakura2StackLimit ) ;
	//
	m_pSakura2VM = m_pcsxi ;
}

// スタック拡張
//////////////////////////////////////////////////////////////////////////////
DWORD ECSContext::HandleExceptionExtendStack( DWORD maskException )
{
	m_maskException &= ~ECSSakura2Processor::exceptionExtendStack ;
	maskException &= ~ECSSakura2Processor::exceptionExtendStack ;
	//
	if ( (DWORD) m_regset[ECSSakura2Processor::regSP].l32 <= Sakura2StackLimit )
	{
		ECSSakura2Processor::AssertLock() ;
		m_pcsxi->Lock() ;
		//
		int	sp = (DWORD) m_regset[ECSSakura2Processor::regSP].l32 & ~0x0FFF ;
		int	nSize = Sakura2StackLimit - sp ;
		ESLAssert( nSize >= 0 ) ;
		//
		int	nOldSize = m_bufNakedStack->GetLength() ;
		int	nNewSize = nOldSize << 1 ;
		while ( nNewSize < nSize )
		{
			nNewSize <<= 1 ;
		}
		int	nExpandOffset = nNewSize - nOldSize ;
		m_bufNakedStack->ResizeBuffer
				( nNewSize, Sakura2StackLimit - nNewSize ) ;
		//
		BYTE *	pbytBuf = m_bufNakedStack->GetBuffer() ;
		eslMoveMemory( pbytBuf + nExpandOffset, pbytBuf, nOldSize ) ;
		//
		m_pcsxi->Unlock() ;
		ECSSakura2Processor::AssertUnlock() ;
	}
	else
	{
		// スタック・オーバーフロー例外
		m_maskException |= ECSSakura2Processor::exceptionStackOverflow ;
		maskException |= ECSSakura2Processor::exceptionStackOverflow ;
	}
	return	maskException ;
}

// 例外エラーメッセージを取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSContext::GetExceptionErrorMessage( DWORD maskException )
{
	if ( maskException & ECSSakura2Processor::exceptionObjectMode )
	{
		return	NULL ;
	}
	return	ContextShell::GetExceptionErrorMessage( maskException ) ;
}

// naked メモリ上の文字列を取得する
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSContext::AtomicLoadString
		( EWideString & wstrBuf, INT64 nAddress )
{
	const wchar_t *	pwszStr = NULL ;
	ECotophaScript::Lock() ;
	int			iOffset ;
	ECSSakura2::Object *
			pObj = GetObjectFromLinearAddress( nAddress, iOffset ) ;
	pwszStr = AtomicLoadString( wstrBuf, pObj, iOffset ) ;
	ECotophaScript::Unlock() ;
	return	pwszStr ;
}

const wchar_t * ECSContext::AtomicLoadString
		( EWideString & wstrBuf, INT64 nAddress, int nLength )
{
	const wchar_t *	pwszStr = NULL ;
	ECotophaScript::Lock() ;
	int			iOffset ;
	ECSSakura2::Object *
			pObj = GetObjectFromLinearAddress( nAddress, iOffset ) ;
	pwszStr = AtomicLoadString( wstrBuf, pObj, iOffset, nLength ) ;
	ECotophaScript::Unlock() ;
	return	pwszStr ;
}

const wchar_t * ECSContext::AtomicLoadString
		( EWideString & wstrBuf, ECSSakura2::Object * pObj, int iOffset )
{
	const wchar_t *	pwszStr = NULL ;
	if ( pObj != NULL )
	{
		ECSSakura2Processor::LinearAddressCache	seg ;
		if ( pObj->GetSegmentBuffer( seg ) != NULL )
		{
			//
			// メモリ参照を取得できる場合
			//
			iOffset -= seg.baseOffset ;
			if ( (unsigned int) iOffset < seg.limitSegment )
			{
				const wchar_t *	pwszSrc =
					(const wchar_t *) (seg.pbytBuffer + iOffset) ;
				int	nLimit = (seg.limitSegment - iOffset) >> 1 ;
				for ( int i = 0; i < nLimit; i ++ )
				{
					if ( pwszSrc[i] == L'\0' )
					{
						wstrBuf = pwszSrc ;
						pwszStr = wstrBuf ;
						nLimit = i ;
						break ;
					}
				}
				if ( pwszStr != NULL )
				{
					wstrBuf = EWideString( pwszSrc, nLimit ) ;
					pwszStr = wstrBuf ;
				}
			}
			else
			{
				wstrBuf = L"" ;
			}
		}
		else
		{
			//
			// 汎用処理
			//
			ECSObject *	pcsObj = ESLTypeCast<ECSObject>( pObj ) ;
			if ( pcsObj != NULL )
			{
				wstrBuf = L"" ;
				for ( int i = 0; true ; i ++, iOffset += sizeof(wchar_t) )
				{
					wchar_t *	pwBuf =
						(wchar_t*) pcsObj->GetBuffer
								( iOffset, sizeof(wchar_t), false ) ;
					if ( pwBuf != NULL )
					{
						wchar_t	wch = *pwBuf ;
						pcsObj->FlushBuffer
							( iOffset, sizeof(wchar_t), pwBuf, false ) ;
						//
						if ( wch != 0 )
						{
							wstrBuf += wch ;
						}
						else
						{
							break ;
						}
					}
					else
					{
						break ;
					}
				}
				pwszStr = wstrBuf ;
			}
		}
	}
	return	pwszStr ;
}

const wchar_t * ECSContext::AtomicLoadString
	( EWideString & wstrBuf, ECSSakura2::Object * pObj, int iOffset, int nLength )
{
	const wchar_t *	pwszStr = NULL ;
	ECSObject *	pcsObj = ESLTypeCast<ECSObject>( pObj ) ;
	if ( pcsObj != NULL )
	{
		wchar_t *	pwBuf =
			(wchar_t*) pcsObj->GetBuffer
					( iOffset, nLength * sizeof(wchar_t), false ) ;
		if ( pwBuf != NULL )
		{
			wstrBuf = EWideString( pwBuf, nLength * sizeof(wchar_t) ) ;
			pwszStr = wstrBuf ;
			//
			pcsObj->FlushBuffer
				( iOffset, nLength * sizeof(wchar_t), pwBuf, false ) ;
		}
	}
	return	pwszStr ;
}


// naked native 関数
//////////////////////////////////////////////////////////////////////////////

/*
// void * malloc( int bytes )
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ecs_nakedcall_malloc
	( ECSSakura2Processor::Context * pcontext,
		const ECSSakura2Processor::Register * pArg )
{
	ECSContext *	context = (ECSContext*) pcontext ;
	ECSBuffer *	pBuf = new ECSBuffer ;
	pBuf->CreateBuffer( (int) pArg[0].l32 ) ;
	//
	ECSSakura2Processor::AssertLock() ;
	context->m_regset[0].i =
		context->m_pcsxi->AllocateHeapObjectAddress( pBuf ) ;
	ECSSakura2Processor::AssertUnlock() ;
	return	NULL ;
}

// void * realloc( void * memblock, int bytes )
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ecs_nakedcall_realloc
	( ECSSakura2Processor::Context * pcontext,
		const ECSSakura2Processor::Register * pArg )
{
	ECSBuffer *	pBuf =
		ESLTypeCast<ECSBuffer>
			( pcontext->m_pSakura2VM->AtomicObjectFromAddress( pArg[0].h32 ) ) ;
	if ( pBuf != NULL )
	{
		ECSSakura2Processor::AssertLock() ;
		pBuf->ResizeBuffer( (int) pArg[1].l32 ) ;
		pcontext->m_regset[0] = pArg[0] ;
		ECSSakura2Processor::AssertUnlock() ;
	}
	else
	{
		pcontext->m_regset[0].i = 0 ;
	}
	return	NULL ;
}

// void free( void * memblock )
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ecs_nakedcall_free
	( ECSSakura2Processor::Context * pcontext,
		const ECSSakura2Processor::Register * pArg )
{
//	ECSSakura2Processor::AssertLock() ;
	pcontext->m_pSakura2VM->FreeHeapObjectAddress( pArg[0].i, pcontext ) ;
//	ECSSakura2Processor::AssertUnlock() ;
	//
	return	NULL ;
}

// void * object_new( int cls_id )
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ecs_nakedcall_object_new
	( ECSSakura2Processor::Context * pcontext,
		const ECSSakura2Processor::Register * pArg )
{
	ECSSakura2::Object *	pObj =
			pcontext->m_pSakura2VM->
				NewObjectByIdentity( pcontext, pArg[0].l32 ) ;
	if ( pObj != NULL )
	{
		ECSSakura2Processor::AssertLock() ;
		pcontext->m_regset[0].i =
			pcontext->m_pSakura2VM->AllocateHeapObjectAddress( pObj ) ;
		ECSSakura2Processor::AssertUnlock() ;
		return	NULL ;
	}
	else
	{
		return	L"object の生成に失敗しました" ;
	}
}

// void object_delete( void * obj )
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ecs_nakedcall_object_delete
	( ECSSakura2Processor::Context * pcontext,
		const ECSSakura2Processor::Register * pArg )
{
	ECSSakura2::Object *	pObj =
		pcontext->m_pSakura2VM->AtomicObjectFromAddress( pArg[0].h32 ) ;
	if ( pObj != NULL )
	{
		pObj->OnDestruction( pcontext->m_pSakura2VM, pcontext ) ;
		//
//		ECSSakura2Processor::AssertLock() ;
		pcontext->m_pSakura2VM->FreeHeapObjectAddress( pArg[0].i, pcontext ) ;
//		ECSSakura2Processor::AssertUnlock() ;
	}
	return	NULL ;
}
*/

// void object_push_int( int value )
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ecs_nakedcall_object_push_int
	( ECSSakura2Processor::Context * pcontext,
		const ECSSakura2Processor::Register * pArg )
{
	ECSContext *	context = (ECSContext*) pcontext ;
	context->PushObject( context->new_CSInteger( pArg[0].i ) ) ;
	return	NULL ;
}

// void object_push_double( double value )
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ecs_nakedcall_object_push_double
	( ECSSakura2Processor::Context * pcontext,
		const ECSSakura2Processor::Register * pArg )
{
	ECSContext *	context = (ECSContext*) pcontext ;
	context->PushObject( context->new_CSReal( pArg[0].f ) ) ;
	return	NULL ;
}

// void object_push_string( const uint16 * str )
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ecs_nakedcall_object_push_string
	( ECSSakura2Processor::Context * pcontext,
		const ECSSakura2Processor::Register * pArg )
{
	ECSContext *	context = (ECSContext*) pcontext ;
	ECSString *	pStr = context->new_CSString() ;
	context->AtomicLoadString( pStr->m_varStr, pArg[0].i ) ;
	context->PushObject( pStr ) ;
	return	NULL ;
}

// void object_push_reference( void * obj )
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ecs_nakedcall_object_push_reference
	( ECSSakura2Processor::Context * pcontext,
		const ECSSakura2Processor::Register * pArg )
{
	ECSContext *	context = (ECSContext*) pcontext ;
	ECotophaScript::Lock() ;
	int	iOffset ;
	ECSObject *	pObj =
		ESLTypeCast<ECSObject>
			( context->GetObjectFromLinearAddress( pArg[0].i, iOffset ) ) ;
	ECotophaScript::Unlock() ;
	//
	context->PushObject( context->new_CSReference( pObj ) ) ;
	return	NULL ;
}

// void object_push_pointer( void * obj )
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ecs_nakedcall_object_push_pointer
	( ECSSakura2Processor::Context * pcontext,
		const ECSSakura2Processor::Register * pArg )
{
	ECSContext *	context = (ECSContext*) pcontext ;
	ECotophaScript::Lock() ;
	int	iOffset ;
	ECSObject *	pObj =
		ESLTypeCast<ECSObject>
			( context->GetObjectFromLinearAddress( pArg[0].i, iOffset ) ) ;
	ECotophaScript::Unlock() ;
	//
	context->PushObject( context->new_CSPointer( pObj ) ) ;
	return	NULL ;
}

// void object_push_type( int type_id )
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ecs_nakedcall_object_push_type
	( ECSSakura2Processor::Context * pcontext,
		const ECSSakura2Processor::Register * pArg )
{
	ECSContext *	context = (ECSContext*) pcontext ;
	ECSObject *	pObj =
		context->CreateObject( (CSVariableType) pArg[0].li32, NULL ) ;
	if ( pObj != NULL )
	{
		context->PushObject( pObj ) ;
		return	NULL ;
	}
	else
	{
		return	L"オブジェクトの生成に失敗しました" ;
	}
}

// void object_push_new( int cls_id )
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ecs_nakedcall_object_push_new
	( ECSSakura2Processor::Context * pcontext,
		const ECSSakura2Processor::Register * pArg )
{
	ECSContext *	context = (ECSContext*) pcontext ;
	const ECSClassInfo *
		pClassInf = context->m_pcsxi->GetClassInfoAt( (int) pArg[0].l32 ) ;
	if ( pClassInf != NULL )
	{
		ECSObject *	pObj = context->CreateClassObject( *pClassInf ) ;
		if ( pObj != NULL )
		{
			context->PushObject( pObj ) ;
			return	NULL ;
		}
		else
		{
			return	L"クラスの生成に失敗しました" ;
		}
	}
	else
	{
		return	L"クラス情報が見つかりません" ;
	}
}

// int object_pop_int( void )
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ecs_nakedcall_object_pop_int
	( ECSSakura2Processor::Context * pcontext,
		const ECSSakura2Processor::Register * pArg )
{
	ECSContext *	context = (ECSContext*) pcontext ;
	ECSObject *	pObj = context->PopObject() ;
	if ( pObj == NULL )
	{
		return	L"object スタックからヌルを取得しました" ;
	}
	INT64		nValue ;
	ESLError	err = pObj->OperateInteger( nValue ) ;
	if ( err )
	{
		context->delete_CSObject( pObj ) ;
		return	L"object を整数に変換出来ませんでした" ;
	}
	context->delete_CSObject( pObj ) ;
	context->m_regset[0].i = nValue ;
	return	NULL ;
}

// double object_pop_double( void )
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ecs_nakedcall_object_pop_double
	( ECSSakura2Processor::Context * pcontext,
		const ECSSakura2Processor::Register * pArg )
{
	ECSContext *	context = (ECSContext*) pcontext ;
	ECSObject *	pObj = context->PopObject() ;
	if ( pObj == NULL )
	{
		return	L"object スタックからヌルを取得しました" ;
	}
	REAL64		nValue ;
	ESLError	err = pObj->OperateReal( nValue ) ;
	context->delete_CSObject( pObj ) ;
	if ( err )
	{
		return	L"object を実数に変換出来ませんでした" ;
	}
	context->m_regset[0].f = nValue ;
	return	NULL ;
}

// void * object_pop_new( void )
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ecs_nakedcall_object_pop_new
	( ECSSakura2Processor::Context * pcontext,
		const ECSSakura2Processor::Register * pArg )
{
	ECSContext *	context = (ECSContext*) pcontext ;
	ECSObject *	pObj = context->PopObject() ;
	if ( pObj == NULL )
	{
		context->m_regset[0].i = 0 ;
		return	NULL ;
	}
	ECSSakura2Processor::AssertLock() ;
	//
	context->m_regset[0].i =
		context->m_pcsxi->AllocateHeapObjectAddress( pObj ) ;
	//
	ECSSakura2Processor::AssertUnlock() ;
	return	NULL ;
}

// void object_stack_free( int count )
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ecs_nakedcall_object_stack_free
	( ECSSakura2Processor::Context * pcontext,
		const ECSSakura2Processor::Register * pArg )
{
	ECSContext *	context = (ECSContext*) pcontext ;
	const int	nCount = (int) pArg[0].l32 ;
	for ( int i = 0; i < nCount; i ++ )
	{
		context->delete_CSObject( context->PopObject() ) ;
	}
	return	NULL ;
}

// void * object_get_stack_at( int index )
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ecs_nakedcall_object_get_stack_at
	( ECSSakura2Processor::Context * pcontext,
		const ECSSakura2Processor::Register * pArg )
{
	ECSContext *	context = (ECSContext*) pcontext ;
	int			iOffset ;
	ESLError	err =
		context->m_stack.GetVariableIndex( iOffset, (int) pArg[0].li32 ) ;
	if ( !err )
	{
		context->m_regset[0].i =
				context->m_vaStack | (((INT64)iOffset) << 32) ;
	}
	else
	{
		context->m_regset[0].i = 0 ;
	}
	return	NULL ;
}

// void * object_get_stack_last( int index )
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ecs_nakedcall_object_get_stack_last
	( ECSSakura2Processor::Context * pcontext,
		const ECSSakura2Processor::Register * pArg )
{
	ECSContext *	context = (ECSContext*) pcontext ;
	int	iOffset =
			context->m_stack.m_varArray.GetSize() - (int) pArg[0].li32 - 1 ;
	context->m_regset[0].i =
			context->m_vaStack | (((INT64)iOffset) << 32) ;
	return	NULL ;
}

// bool object_get_boolean( void * obj )
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ecs_nakedcall_object_get_boolean
	( ECSSakura2Processor::Context * pcontext,
		const ECSSakura2Processor::Register * pArg )
{
	ECSContext *	context = (ECSContext*) pcontext ;
	ECotophaScript::Lock() ;
	int	iOffset ;
	ECSObject *	pObj =
		ESLTypeCast<ECSObject>
			( context->GetObjectFromLinearAddress( pArg[0].i, iOffset ) ) ;
	ECotophaScript::Unlock() ;
	//
	if ( pObj == NULL )
	{
		return	L"null object からの読み込みです" ;
	}
	int	nBoolean ;
	ESLError	err = pObj->OperateBoolean( nBoolean ) ;
	if ( err )
	{
		return	L"object を boolean に変換出来ませんでした" ;
	}
	context->m_regset[0].i = nBoolean ;
	return	NULL ;
}

// int object_get_int( void * obj )
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ecs_nakedcall_object_get_int
	( ECSSakura2Processor::Context * pcontext,
		const ECSSakura2Processor::Register * pArg )
{
	ECSContext *	context = (ECSContext*) pcontext ;
	ECotophaScript::Lock() ;
	int	iOffset ;
	ECSObject *	pObj =
		ESLTypeCast<ECSObject>
			( context->GetObjectFromLinearAddress( pArg[0].i, iOffset ) ) ;
	ECotophaScript::Unlock() ;
	//
	if ( pObj == NULL )
	{
		return	L"null object からの読み込みです" ;
	}
	INT64		nValue ;
	ESLError	err = pObj->OperateInteger( nValue ) ;
	if ( err )
	{
		return	L"object を整数に変換出来ませんでした" ;
	}
	context->m_regset[0].i = nValue ;
	return	NULL ;
}

// double object_get_double( void * obj )
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ecs_nakedcall_object_get_double
	( ECSSakura2Processor::Context * pcontext,
		const ECSSakura2Processor::Register * pArg )
{
	ECSContext *	context = (ECSContext*) pcontext ;
	ECotophaScript::Lock() ;
	int	iOffset ;
	ECSObject *	pObj =
		ESLTypeCast<ECSObject>
			( context->GetObjectFromLinearAddress( pArg[0].i, iOffset ) ) ;
	ECotophaScript::Unlock() ;
	//
	if ( pObj == NULL )
	{
		return	L"null object からの読み込みです" ;
	}
	REAL64		nValue ;
	ESLError	err = pObj->OperateReal( nValue ) ;
	if ( err )
	{
		return	L"object を実数に変換出来ませんでした" ;
	}
	context->m_regset[0].f = nValue ;
	return	NULL ;
}

// int object_size_of( void * obj )
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ecs_nakedcall_object_size_of
	( ECSSakura2Processor::Context * pcontext,
		const ECSSakura2Processor::Register * pArg )
{
	ECSContext *	context = (ECSContext*) pcontext ;
	ECotophaScript::Lock() ;
	int	iOffset ;
	ECSObject *	pObj =
		ESLTypeCast<ECSObject>
			( context->GetObjectFromLinearAddress( pArg[0].i, iOffset ) ) ;
	ECotophaScript::Unlock() ;
	//
	if ( pObj == NULL )
	{
		return	L"null object からの読み込みです" ;
	}
	INT64		nLength ;
	ESLError	err = pObj->OperateSizeOf( nLength ) ;
	if ( err )
	{
		return	L"object の sizeof は定義されていません" ;
	}
	context->m_regset[0].i = nLength ;
	return	NULL ;
}

