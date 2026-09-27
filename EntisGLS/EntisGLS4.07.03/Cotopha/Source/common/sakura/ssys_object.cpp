
#include <sakura/sakura.h>

using namespace SSystem ;


//////////////////////////////////////////////////////////////////////////////
// バックリンク付きオブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SObject, ESLObject )

// 参照を解除
//////////////////////////////////////////////////////////////////////////////
void SObject::DetachFromReference( void )
{
	if ( m_pRefBackLink != NULL )
	{
		m_pRefBackLink->DetachAllReference() ;
	}
}

// 全ての参照が解除された
//////////////////////////////////////////////////////////////////////////////
void SObject::OnReleaseFromReference( void )
{
}


//////////////////////////////////////////////////////////////////////////////
// 参照オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO_CAST( SSystem::SReference, SObject, m_pReference )

// 参照を設定
//////////////////////////////////////////////////////////////////////////////
void SReference::SetReference( SObject * pObj )
{
	if ( pObj == m_pReference )
	{
		return ;
	}
	if ( m_pReference != NULL )
	{
		ReleaseReference() ;
	}
	ESLAssert( m_pReference == NULL ) ;
	ESLAssert( m_pPrevRef == NULL ) ;
	ESLAssert( m_pNextRef == NULL ) ;
	if ( pObj != NULL )
	{
		SReference *	pOldBackLink = pObj->m_pRefBackLink ;
		if ( pOldBackLink != NULL )
		{
			ESLAssert( pOldBackLink->m_pPrevRef == NULL ) ;
			pOldBackLink->m_pPrevRef = this ;
			m_pNextRef = pOldBackLink ;
		}
		m_pReference = pObj ;
		pObj->m_pRefBackLink = this ;
	}
}

// 参照を解除
//////////////////////////////////////////////////////////////////////////////
void SReference::ReleaseReference( void )
{
	SObject *	pFreeRef = GetReleaseReference() ;
	if ( pFreeRef != NULL )
	{
		pFreeRef->OnReleaseFromReference() ;
	}
}

// 参照を解除（すべての参照が解除された場合のみ参照先オブジェクトを返却）
//////////////////////////////////////////////////////////////////////////////
SObject * SReference::GetReleaseReference( void )
{
	SObject *	pRef = m_pReference ;
	if ( pRef != NULL )
	{
		SReference *	pPrevRef = m_pPrevRef ;
		SReference *	pNextRef = m_pNextRef ;
		if ( pPrevRef != NULL )
		{
			pPrevRef->m_pNextRef = pNextRef ;
			m_pPrevRef = NULL ;
		}
		else
		{
			ESLAssert( pRef->m_pRefBackLink == this ) ;
			pRef->m_pRefBackLink = pNextRef ;
			//
			if ( pNextRef == NULL )
			{
				m_pReference = NULL ;
				return	pRef ;
			}
		}
		if ( pNextRef != NULL )
		{
			ESLAssert( pNextRef->m_pPrevRef == this ) ;
			pNextRef->m_pPrevRef = pPrevRef ;
			m_pNextRef = NULL ;
		}
		m_pReference = NULL ;
	}
	return	NULL ;
}

// SObject 側から参照の解除要求
//////////////////////////////////////////////////////////////////////////////
void SReference::DetachAllReference( void )
{
	SReference *	pNextRef = this ;
	while ( pNextRef != NULL )
	{
		SReference *	pRef = pNextRef ;
		pNextRef = pRef->m_pNextRef ;
		pRef->m_pReference = NULL ;
		pRef->m_pPrevRef = NULL ;
		pRef->m_pNextRef = NULL ;
	}
}



//////////////////////////////////////////////////////////////////////////////
// 自動消滅オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO_CAST( SSystem::SSmartObject, SObject, m_pObject )

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SSmartObject::~SSmartObject( void )
{
	if ( m_pObject != NULL )
	{
		delete	m_pObject ;
		m_pObject = NULL ;
	}
}

// 全ての参照が解除された
//////////////////////////////////////////////////////////////////////////////
void SSmartObject::OnReleaseFromReference( void )
{
	delete	this ;
}



//////////////////////////////////////////////////////////////////////////////
// 同期参照オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SSyncReference, SReference )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SSyncReference::SSyncReference( const SSyncReference& refSrc )
{
	QuickLock() ;
	SSyncReference::SetReference( refSrc.m_pReference ) ;
	QuickUnlock() ;
}

// 参照を設定
//////////////////////////////////////////////////////////////////////////////
void SSyncReference::SetReference( SObject * pObj )
{
	QuickLock() ;
	SReference::SetReference( pObj ) ;
	QuickUnlock() ;
}

const SSyncReference & SSyncReference::operator = ( const SSyncReference & ref )
{
	QuickLock() ;
	SReference::SetReference( ref.m_pReference ) ;
	QuickUnlock() ;
	return	*this ;
}

// 参照を解除
//////////////////////////////////////////////////////////////////////////////
void SSyncReference::ReleaseReference( void )
{
	SObject *	pFreeRef ;
	QuickLock() ;
	pFreeRef = GetReleaseReference() ;
	QuickUnlock() ;
	if ( pFreeRef != NULL )
	{
		pFreeRef->OnReleaseFromReference() ;
	}
}

// SObject 側から参照の解除要求
//////////////////////////////////////////////////////////////////////////////
void SSyncReference::DetachAllReference( void )
{
	QuickLock() ;
	SReference::DetachAllReference() ;
	QuickUnlock() ;
}


//////////////////////////////////////////////////////////////////////////////
// 詞葉・揮発性オブジェクト
//////////////////////////////////////////////////////////////////////////////

#if	!defined(__COTOPHA__)

void VolatileObject::AttachVolatileInterface( SVolatileInterface * pInterface )
{
}

#endif


//////////////////////////////////////////////////////////////////////////////
// 依存オブジェクト通知インターフェース
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SDependentNotification, SObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SDependentNotification::SDependentNotification( void )
{
	m_pdnPrev = NULL ;
	m_pdnNext = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SDependentNotification::~SDependentNotification( void )
{
	SDependentNotificationServer *	pServer =
			ESLTypeCast<SDependentNotificationServer>
							( m_refDependent.GetReference() ) ;
	if ( pServer != NULL )
	{
		pServer->DetachNotification( this ) ;
	}
	else
	{
		DetachNotification() ;
	}
}

// このオブジェクトの前に挿入する
//////////////////////////////////////////////////////////////////////////////
void SDependentNotification::InsertBeforeNotification( SDependentNotification * pBefore )
{
	ESLAssert( this != NULL ) ;
	ESLAssert( pBefore != NULL ) ;
	ESLAssert( pBefore->m_pdnPrev == NULL ) ;
	ESLAssert( pBefore->m_pdnNext == NULL ) ;
	//
	SDependentNotification *	pPrev = m_pdnPrev ;
	pBefore->m_pdnNext = this ;
	pBefore->m_pdnPrev = pPrev ;
	m_pdnPrev = pBefore ;
	//
	if ( pPrev != NULL )
	{
		ESLAssert( pPrev->m_pdnNext == this ) ;
		pPrev->m_pdnNext = pBefore ;
	}
}

// このオブジェクトの後に挿入する
//////////////////////////////////////////////////////////////////////////////
void SDependentNotification::InsertAfterNotification( SDependentNotification * pAfter )
{
	ESLAssert( this != NULL ) ;
	ESLAssert( pAfter != NULL ) ;
	ESLAssert( pAfter->m_pdnPrev == NULL ) ;
	ESLAssert( pAfter->m_pdnNext == NULL ) ;
	//
	SDependentNotification *	pNext = m_pdnNext ;
	pAfter->m_pdnPrev = this ;
	pAfter->m_pdnNext = pNext ;
	m_pdnNext = pAfter ;
	//
	if ( pNext != NULL )
	{
		ESLAssert( pNext->m_pdnPrev == this ) ;
		pNext->m_pdnPrev = pAfter ;
	}
}

// このオブジェクトを分離する
//////////////////////////////////////////////////////////////////////////////
void SDependentNotification::DetachNotification( void )
{
	SDependentNotification *	pPrev = m_pdnPrev ;
	SDependentNotification *	pNext = m_pdnNext ;
	if ( pPrev != NULL )
	{
		pPrev->m_pdnNext = pNext ;
	}
	if ( pNext != NULL )
	{
		pNext->m_pdnPrev = pPrev ;
	}
	m_pdnPrev = NULL ;
	m_pdnNext = NULL ;
}

// 依存先オブジェクト生成／再生成後に呼び出される
//////////////////////////////////////////////////////////////////////////////
void SDependentNotification::OnCreateObject( SDependentNotificationServer * pObject, bool fInitialize )
{
}

// 依存先のリソースが解放される／リセットされる前等に呼び出される
//////////////////////////////////////////////////////////////////////////////
void SDependentNotification::OnReleaseObject( SDependentNotificationServer * pObject )
{
}

// 依存先オブジェクトが削除される前に呼び出される
//////////////////////////////////////////////////////////////////////////////
void SDependentNotification::OnFinalizeObject( SDependentNotificationServer * pObject )
{
}


//////////////////////////////////////////////////////////////////////////////
// 依存オブジェクト通知サーバ
//////////////////////////////////////////////////////////////////////////////

ESL_DLL_DECL(SDependentNotificationServer SSystem::g_dnsLibServ) ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SSystem::SDependentNotificationServer, SDependentNotification )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SDependentNotificationServer::SDependentNotificationServer( void )
{
	m_pdnFirstNotify = NULL ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SDependentNotificationServer::~SDependentNotificationServer( void )
{
}

// 通知オブジェクトを追加する
//////////////////////////////////////////////////////////////////////////////
void SDependentNotificationServer::AddNotification( SDependentNotification * pdnNotify )
{
	SDependentNotificationServer *	pServer =
		ESLTypeCast<SDependentNotificationServer>
					( pdnNotify->m_refDependent.GetReference() ) ;
	if ( pServer != NULL )
	{
		if ( pServer == this )
		{
			ESLAssert( (m_pdnFirstNotify == pdnNotify)
						|| (pdnNotify->m_pdnPrev != NULL)
						|| (pdnNotify->m_pdnNext != NULL) ) ;
			return ;
		}
		pServer->DetachNotification( pdnNotify ) ;
	}
	pdnNotify->m_refDependent.SetReference( this ) ;
	//
	ESLAssert( pdnNotify != NULL ) ;
	ESLAssert( pdnNotify->m_pdnNext == NULL ) ;
	ESLAssert( pdnNotify->m_pdnPrev == NULL ) ;
	//
	QuickLock() ;
	SDependentNotification *	pFirst = m_pdnFirstNotify ;
	pdnNotify->m_pdnNext = pFirst ;
	m_pdnFirstNotify = pdnNotify ;
	//
	if ( pFirst != NULL )
	{
		ESLAssert( pFirst->m_pdnPrev == NULL ) ;
		pFirst->m_pdnPrev = pdnNotify ;
	}
	QuickUnlock() ;
}

// 通知を解除する
//////////////////////////////////////////////////////////////////////////////
void SDependentNotificationServer::DetachNotification( SDependentNotification * pdnNotify )
{
	ESLAssert( pdnNotify != NULL ) ;
	SDependentNotificationServer *	pServer =
		ESLTypeCast<SDependentNotificationServer>
					( pdnNotify->m_refDependent.GetReference() ) ;
	if ( pServer == this )
	{
		QuickLock() ;
		if ( m_pdnFirstNotify == pdnNotify )
		{
			m_pdnFirstNotify = pdnNotify->m_pdnNext ;
		}
		pdnNotify->DetachNotification() ;
		QuickUnlock() ;
	}
}

// OnCreateObject を通知する
//////////////////////////////////////////////////////////////////////////////
void SDependentNotificationServer::NotifyAllOnCreate( bool fInitialize )
{
	SDependentNotification *	pNotify ;
	QuickLock() ;
	pNotify = m_pdnFirstNotify ;
	while ( pNotify != NULL )
	{
		SSmartReference<SDependentNotification>	refNext = pNotify->m_pdnNext ;
		QuickUnlock() ;
		pNotify->OnCreateObject( this, fInitialize ) ;
		QuickLock() ;
		pNotify = refNext.GetReference() ;
	}
	QuickUnlock() ;
}

// OnReleaseObject を通知する
//////////////////////////////////////////////////////////////////////////////
void SDependentNotificationServer::NotifyAllOnRelease( void )
{
	SDependentNotification *	pNotify ;
	QuickLock() ;
	pNotify = m_pdnFirstNotify ;
	while ( pNotify != NULL )
	{
		SSmartReference<SDependentNotification>	refNext = pNotify->m_pdnNext ;
		QuickUnlock() ;
		pNotify->OnReleaseObject( this ) ;
		QuickLock() ;
		pNotify = refNext.GetReference() ;
	}
	QuickUnlock() ;
}

// OnFinalizeObject を通知する
//////////////////////////////////////////////////////////////////////////////
void SDependentNotificationServer::NotifyAllOnFinalize( void )
{
	SDependentNotification *	pNotify ;
	QuickLock() ;
	pNotify = m_pdnFirstNotify ;
	while ( pNotify != NULL )
	{
		SSmartReference<SDependentNotification>	refCur = pNotify ;
		SSmartReference<SDependentNotification>	refNext = pNotify->m_pdnNext ;
		QuickUnlock() ;
		pNotify->OnFinalizeObject( this ) ;
		QuickLock() ;
		pNotify = refCur.GetReference() ;
		if ( pNotify != NULL )
		{
			refNext = pNotify->m_pdnNext ;
			DetachNotification( pNotify ) ;
		}
		pNotify = refNext.GetReference() ;
	}
	ESLAssert( m_pdnFirstNotify == NULL ) ;
	QuickUnlock() ;
}

// 依存先オブジェクト生成／再生成後に呼び出される
//////////////////////////////////////////////////////////////////////////////
void SDependentNotificationServer::OnCreateObject( SDependentNotificationServer * pObject, bool fInitialize )
{
	NotifyAllOnCreate( fInitialize ) ;
}

// 依存先のリソースが解放される／リセットされる前等に呼び出される
//////////////////////////////////////////////////////////////////////////////
void SDependentNotificationServer::OnReleaseObject( SDependentNotificationServer * pObject )
{
	NotifyAllOnRelease() ;
}

// 依存先オブジェクトが削除される前に呼び出される
//////////////////////////////////////////////////////////////////////////////
void SDependentNotificationServer::OnFinalizeObject( SDependentNotificationServer * pObject )
{
	NotifyAllOnFinalize() ;
}



