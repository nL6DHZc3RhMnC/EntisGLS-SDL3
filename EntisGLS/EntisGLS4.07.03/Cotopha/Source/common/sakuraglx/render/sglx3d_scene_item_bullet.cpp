
#include <sakuraglx/sakuraglx.h>
#include <sakuraglx/render/sglx3d_scene_item.h>

using namespace SSystem ;
using namespace SakuraGL ;
using namespace Rosetta ;
using namespace Loquaty ;


//////////////////////////////////////////////////////////////////////////////
// 弾幕制御基底
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_NV_CLASS_INFO( SakuraGL::S3DBulletItemInterface )
ESL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DBulletItemInterface::BulletListener, SObject )
ESL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::S3DBulletItemInterface::BulletController, Controller, BulletListener )

// BulletController 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DBulletItemInterface::BulletController::BulletController( const wchar_t * pwszClassID )
	: Controller( pwszClassID )
{
}

// BulletListener 禁止状態
//////////////////////////////////////////////////////////////////////////////
bool S3DBulletItemInterface::BulletController::IsBulletDisabled( void ) const
{
	return	IsControllerDisabled() ;
}


// 複数当たり判定
//////////////////////////////////////////////////////////////////////////////
S3DBulletItemInterface::MultiHitInstance::MultiHitInstance( S3DBulletItemInterface * pbii )
	: m_pbii( pbii ), m_iCurTrack( 0 )
{
}

void S3DBulletItemInterface::MultiHitInstance::ResetInstance( void )
{
	RemoveAll() ;
	m_iCurTrack = 0 ;
}

bool S3DBulletItemInterface::MultiHitInstance::IsEmpty( void ) const
{
	return	GetLength() == 0 ;
}

void S3DBulletItemInterface::MultiHitInstance::SetRay( const S3DVector& vRay )
{
	m_vRay = vRay.Normalized() ;
}

void S3DBulletItemInterface::MultiHitInstance::SetCurrentTrack( size_t iTrack )
{
	m_iCurTrack = iTrack ;
}

S3DCollision::HitColliderCallback
	S3DBulletItemInterface::MultiHitInstance::Callback_OnHitSphereCollider
		( const S3DCollisionResult& rsHit,
			const S3DVector& vHitPos, const S3DVector& vHitNormal,
			const S3DCollision::MeshCollision * pMesh, size_t iPolygon )
{
	MultiHitInstance *		pmhi = (MultiHitInstance*) rsHit.ptrOnHitInstance ;
	const HitBulletEntry *	phbEnties = pmhi->GetConstArray() ;
	const size_t			nEntries = pmhi->GetLength() ;
	//
	S3DCollision::HitColliderGlobalInfo	hcgi ;
	rsHit.GetHitColliderGlobalInfo( hcgi, pMesh ) ;
	//
	S3DVector	vGlobalNormal = hcgi.matToGlobal * vHitNormal ;
	vGlobalNormal.Normalize() ;
	if ( pmhi->m_vRay.InnerProduct( vGlobalNormal ) >= 0.1f )
	{
		// 裏面ヒットは通過する
		return	S3DCollision::hitColliderNext ;
	}
	//
	for ( size_t i = 0; i < nEntries; i ++ )
	{
		if ( phbEnties[i].pmcHitGlobal == hcgi.pGlobalMesh )
		{
			return	S3DCollision::hitColliderNextMesh ;
		}
	}
	HitBulletEntry	hbe ;
	hbe.pmcHitPrimitive = pMesh ;
	hbe.pmcHitGlobal = hcgi.pGlobalMesh ;
	hbe.vHitGlobalPos = hcgi.matToGlobal * vHitPos + hcgi.vToGlobal ;
	hbe.vHitGlobalNormal = hcgi.matToGlobal * vHitNormal ;
	hbe.iHitTrack = pmhi->m_iCurTrack ;
	pmhi->InsertAt( 0, hbe ) ;
	//
	if ( pmhi->m_pbii->m_nBulletFlags & flagMultiHit )
	{
		return	S3DCollision::hitColliderNextMesh ;
	}
	return	S3DCollision::hitColliderReturn ;
}

S3DCollision::HitColliderCallback
	S3DBulletItemInterface::MultiHitInstance::Callback_OnHitRayCollider
		( const S3DCollisionResult& rsHit,
			const S3DVector& vHitPos, const S3DVector& vHitNormal,
			const S3DCollision::MeshCollision * pMesh, size_t iPolygon )
{
	MultiHitInstance *	pmhi = (MultiHitInstance*) rsHit.ptrOnHitInstance ;
	HitBulletEntry *	phbEnties = pmhi->GetArray() ;
	const size_t		nEntries = pmhi->GetLength() ;
	//
	S3DCollision::HitColliderGlobalInfo	hcgi ;
	rsHit.GetHitColliderGlobalInfo( hcgi, pMesh ) ;
	//
	S3DVector	vGlobalNormal = hcgi.matToGlobal * vHitNormal ;
	if ( pmhi->m_vRay.InnerProduct( vGlobalNormal ) >= 0.0f )
	{
		// 裏面ヒットは通過する
		return	S3DCollision::hitColliderNext ;
	}
	//
	for ( size_t i = 0; i < nEntries; i ++ )
	{
		HitBulletEntry&	hbe = phbEnties[i] ;
		if ( hbe.pmcHitGlobal == hcgi.pGlobalMesh )
		{
			hbe.vHitGlobalPos = hcgi.matToGlobal * vHitPos + hcgi.vToGlobal ;
			hbe.vHitGlobalNormal = hcgi.matToGlobal * vHitNormal ;
			pmhi->FinishArray() ;
			return	S3DCollision::hitColliderReturn ;
		}
	}
	pmhi->FinishArray() ;
	//
	HitBulletEntry	hbe ;
	hbe.pmcHitPrimitive = pMesh ;
	hbe.pmcHitGlobal = hcgi.pGlobalMesh ;
	hbe.vHitGlobalPos = hcgi.matToGlobal * vHitPos + hcgi.vToGlobal ;
	hbe.vHitGlobalNormal = hcgi.matToGlobal * vHitNormal ;
	hbe.iHitTrack = pmhi->m_iCurTrack ;
	//
	if ( pmhi->m_pbii->m_nBulletFlags & flagMultiHit )
	{
		pmhi->InsertAt( 0, hbe ) ;
	}
	else
	{
		pmhi->SetAt( 0, hbe ) ;
	}
	return	S3DCollision::hitColliderReturn ;
}



// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DBulletItemInterface::S3DBulletItemInterface( void )
{
	m_idBulletNext = 1 ;
	m_nBulletFlags = 0 ;
	m_nMaxTrackCount = 2 ;
	m_nMaxColliderCount = 1 ;
	m_fpColliderRadius = 0.1f ;
	m_maskColliderClasses = S3DCollision::colliderPhysItem ;
	m_maskCollisionClasses = S3DCollision::colliderHit ;
	m_secBulletMaxLife = 2.0 ;
	m_secBulletFadeout = 0.5 ;
	m_secLaserPrepareDuration = 0.0 ;
	m_fpLaserPrepareThickness = 0.2 ;
	m_fpLaserPrepareAlpha = 0.5 ;
	m_secLaserFlashDuration = 0.3 ;
	m_fpLaserFlashThickness = 2.0 ;
	m_secNoHitInterval = 0.033 ;
	m_fpLaserMaxLength = 100.0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DBulletItemInterface::~S3DBulletItemInterface( void )
{
	DeleteAllBullets() ;
}

// 動作フラグ
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DBulletItemInterface::GetBulletFlags( void ) const
{
	return	m_nBulletFlags ;
}

void S3DBulletItemInterface::SetBulletFlags( uint32_t nFlags )
{
	m_nBulletFlags = nFlags ;
}

// 最大軌跡
//////////////////////////////////////////////////////////////////////////////
size_t S3DBulletItemInterface::GetMaxTrackCount( void ) const
{
	return	m_nMaxTrackCount ;
}

void S3DBulletItemInterface::SetMaxTrackCount( size_t nCount )
{
	if ( (m_nMaxTrackCount != nCount) && (nCount >= 1) )
	{
		m_nMaxTrackCount = nCount ;
		//
		Lock() ;
		DeleteAllBullets() ;
		Unlock() ;
	}
}

// 軌跡当たり判定数
//////////////////////////////////////////////////////////////////////////////
size_t S3DBulletItemInterface::GetTrackColliderCount( void ) const
{
	return	m_nMaxColliderCount ;
}

void S3DBulletItemInterface::SetTrackColliderCount( size_t nCount )
{
	m_nMaxColliderCount = nCount ;
}

// 当たり判定半径
//////////////////////////////////////////////////////////////////////////////
float32_t S3DBulletItemInterface::GetColliderRadius( void ) const
{
	return	m_fpColliderRadius ;
}

void S3DBulletItemInterface::SetColliderRadius( float32_t fpRadius )
{
	m_fpColliderRadius = fpRadius ;
}

// 当たり判定対象
//////////////////////////////////////////////////////////////////////////////
void S3DBulletItemInterface::SetColliderTargetClasses( uint32_t maskClasses )
{
	m_maskColliderClasses = maskClasses ;
}

uint32_t S3DBulletItemInterface::GetColliderTargetClasses( void ) const
{
	return	m_maskColliderClasses ;
}

// 被当たり判定設定
//////////////////////////////////////////////////////////////////////////////
void S3DBulletItemInterface::SetCollisionClasses( uint32_t maskClasses )
{
	m_maskCollisionClasses = maskClasses ;
}

uint32_t S3DBulletItemInterface::GetCollisionClasses( void ) const
{
	return	m_maskCollisionClasses ;
}

// 寿命
//////////////////////////////////////////////////////////////////////////////
double S3DBulletItemInterface::GetBulletMaxLife( void ) const
{
	return	m_secBulletMaxLife ;
}

double S3DBulletItemInterface::GetBulletFadeout( void ) const
{
	return	m_secBulletFadeout ;
}

void S3DBulletItemInterface::SetBulletMaxLife( double sec )
{
	m_secBulletMaxLife = sec ;
}

void S3DBulletItemInterface::SetBulletFadeout( double sec )
{
	m_secBulletFadeout = sec ;
}

// レーザー最大長
//////////////////////////////////////////////////////////////////////////////
double S3DBulletItemInterface::GetLaserMaxLength( void ) const
{
	return	m_fpLaserMaxLength ;
}

void S3DBulletItemInterface::SetLaserMaxLength( double fpLength )
{
	m_fpLaserMaxLength = fpLength ;
}

// レーザー予備動作
//////////////////////////////////////////////////////////////////////////////
double S3DBulletItemInterface::GetLaserPrepareDuration( void ) const
{
	return	m_secLaserPrepareDuration ;
}

double S3DBulletItemInterface::GetLaserPrepareThickness( void ) const
{
	return	m_fpLaserPrepareThickness ;
}

double S3DBulletItemInterface::GetLaserPrepareAlpha( void ) const
{
	return	m_fpLaserPrepareAlpha ;
}

void S3DBulletItemInterface::SetLaserPrepareDuration( double sec )
{
	m_secLaserPrepareDuration = sec ;
}

void S3DBulletItemInterface::SetLaserPrepareThickness( double fpThickness )
{
	m_fpLaserPrepareThickness = fpThickness ;
}

void S3DBulletItemInterface::SetLaserPrepareAlpha( double alpha )
{
	m_fpLaserPrepareAlpha = alpha ;
}

// レーザー発射直後閃光
//////////////////////////////////////////////////////////////////////////////
double S3DBulletItemInterface::GetLaserFlashDuration( void ) const
{
	return	m_secLaserFlashDuration ;
}

double S3DBulletItemInterface::GetLaserFlashThickness( void ) const
{
	return	m_fpLaserFlashThickness ;
}

void S3DBulletItemInterface::SetLaserFlashDuration( double sec )
{
	m_secLaserFlashDuration = sec ;
}

void S3DBulletItemInterface::SetLaserFlashThickness( double fpThickness )
{
	m_fpLaserFlashThickness = fpThickness ;
}

// 貫通弾当たり判定インターバル
//////////////////////////////////////////////////////////////////////////////
double S3DBulletItemInterface::GetBulletHitInterval( void ) const
{
	return	m_secNoHitInterval ;
}

void S3DBulletItemInterface::SetBulletHitInterval( double sec )
{
	m_secNoHitInterval = sec ;
}

// 弾幕生成（大域座標）
//////////////////////////////////////////////////////////////////////////////
void S3DBulletItemInterface::GenerateBullets
	( const S3DDVector * pvPositions,
		const S3DDVector * pvSpeeds,
		const float32_t * pThickness,
		Rosetta::RSObject ** ppObjects, size_t nCount )
{
	SSmartLock<SCriticalSection>	lock( &m_csBullets ) ;
	//
	if ( m_bullets.GetLength() + nCount > m_bullets.GetLimit() )
	{
		m_bullets.SetLimit
			( ((m_bullets.GetLength() + nCount) + 0x3F) & ~0x3F ) ;
	}
	for ( size_t i = 0; i < nCount; i ++ )
	{
		GenerateBullet
			( pvPositions[i], pvSpeeds[i], pThickness[i], ppObjects[i] ) ;
	}
}

S3DBulletItemInterface::Bullet *
	S3DBulletItemInterface::GenerateBullet
		( const S3DDVector& vPosition,
			const S3DDVector& vSpeed,
			float32_t fpThickness,
			Rosetta::RSObject * pObject,
			Loquaty::LObject * pLObject )
{
	m_csBullets.Lock() ;
	//
	Bullet *	pBullet = NewBullet() ;
	//
	pBullet->vPos = vPosition ;
	pBullet->vTrack[0] = vPosition ;
	pBullet->vSpeed = vSpeed ;
	pBullet->secMaxLife = (float32_t) GetBulletMaxLife() ;
	pBullet->fpBaseThickness = fpThickness ;
	pBullet->fpThickness = fpThickness ;
	pBullet->fpLength = 0 ;
	pBullet->nAlpha = 0xFF ;
	pBullet->pUserObj = pObject ;
	pBullet->pUserLObj = pLObject ;
	RSObject::AddRef( pObject ) ;
	//
	if ( (m_nBulletFlags & flagLaser) && (m_secLaserPrepareDuration > 0.0) )
	{
		pBullet->nStateFlags |= statePrepareLaser ;
		pBullet->fpThickness *= (float32_t) m_fpLaserPrepareThickness ;
		pBullet->nAlpha = (uint32_t) (m_fpLaserPrepareAlpha * 0xFF) ;
	}
	//
	m_bullets.Add( pBullet ) ;
	//
	if ( !(pBullet->nStateFlags & statePrepareLaser) )
	{
		CallOnFireBullet( pBullet ) ;
	}
	m_csBullets.Unlock() ;
	//
	return	pBullet ;
}

// OnFireBullet 呼び出し
//////////////////////////////////////////////////////////////////////////////
void S3DBulletItemInterface::CallOnFireBullet
		( S3DBulletItemInterface::Bullet * pBullet )
{
	for ( size_t i = 0; i < m_aListeners.GetLength(); i ++ )
	{
		BulletListener *	pListener = m_aListeners.GetAt( i ) ;
		if ( pListener != NULL )
		{
			pListener->OnFireBullet( this, *pBullet ) ;
		}
	}
}

// 弾幕取得
//////////////////////////////////////////////////////////////////////////////
bool S3DBulletItemInterface::IsValidBullet
		( S3DBulletItemInterface::Bullet * pBullet ) const
{
	SSmartLock<const SCriticalSection>	lock( &m_csBullets ) ;
	return	(m_bullets.FindPtr( pBullet ) >= 0) ;
}

// 新規 Bullet S3DBulletItemInterface::生成
//////////////////////////////////////////////////////////////////////////////
S3DBulletItemInterface::Bullet *
	S3DBulletItemInterface::NewBullet( void )
{
	const size_t	nObjBytes = sizeof(Bullet)
								+ m_nMaxTrackCount * sizeof(S3DDVector) ;
	Bullet *		pBullet = (Bullet*) esl_malloc ( nObjBytes ) ;
	if ( m_idBulletNext == 0 )
	{
		m_idBulletNext = 1 ;
	}
	eslFillMemory( pBullet, 0, nObjBytes ) ;
	pBullet->idBullet = m_idBulletNext ++ ;
	return	pBullet ;
}

// Bullet 解放
//////////////////////////////////////////////////////////////////////////////
void S3DBulletItemInterface::DeleteBullet
	( S3DBulletItemInterface::Bullet * pBullet )
{
	if ( pBullet != NULL )
	{
		for ( size_t i = 0; i < pBullet->nInstanceCount; i ++ )
		{
			delete	pBullet->pCtrlInstance[i] ;
		}
		RSObject::ReleaseRef( pBullet->pUserObj ) ;
		LObject::ReleaseRef( pBullet->pUserLObj ) ;
		//
		esl_free( pBullet ) ;
	}
}

void S3DBulletItemInterface::DeleteAllBullets( void )
{
	SSmartLock<const SCriticalSection>	lock( &m_csBullets ) ;
	Bullet *const*	ppBullet = m_bullets.GetConstArray() ;
	size_t			nCount = m_bullets.GetLength() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		ESLAssert( ppBullet[i] != NULL ) ;
		DeleteBullet( ppBullet[i] ) ;
	}
	m_bullets.RemoveAll() ;
}

// Bullet にインスタンス追加
//////////////////////////////////////////////////////////////////////////////
bool S3DBulletItemInterface::AddBulletInstance
	( S3DBulletItemInterface::Bullet& bullet, ESLObject * pObj )
{
	if ( (bullet.nInstanceCount >= countMaxInstance) || (pObj == NULL) )
	{
		return	false ;
	}
	bullet.pCtrlInstance[bullet.nInstanceCount ++] = pObj ;
	return	true ;
}

// Bullet インスタンス取得
//////////////////////////////////////////////////////////////////////////////
ESLObject * S3DBulletItemInterface::GetBulletInstance
	( const S3DBulletItemInterface::Bullet& bullet, const ESLRuntimeClass& rtClass )
{
	for ( size_t i = 0; i < bullet.nInstanceCount; i ++ )
	{
		ESLAssert( bullet.pCtrlInstance[i] != NULL ) ;
		if ( bullet.pCtrlInstance[i]->IsKindOf( rtClass ) )
		{
			return	bullet.pCtrlInstance[i] ;
		}
	}
	return	NULL ;
}

// Bullet　デフォルトの移動処理
//////////////////////////////////////////////////////////////////////////////
S3DBulletItemInterface::MoveBulletResult
	S3DBulletItemInterface::MoveBulletDefault
		( S3DBulletItemInterface::Bullet& bullet, double secPast ) const
{
	if ( bullet.nStateFlags & (stateControlMoved | stateControlDestroy) )
	{
		return	resultNoMove ;
	}
	if ( bullet.nStateFlags & stateHitDestroy )
	{
		if ( bullet.nTrackCount <= 3 )
		{
			// 削除
			return	resultDelete ;
		}
		bullet.nStateFlags |= stateMovedTrackTail ;
		bullet.nTrackCount -- ;
	}
	else if ( m_nBulletFlags & flagLaser )
	{
		if ( m_nMaxTrackCount >= 1 )
		{
			S3DDVector	vDir = bullet.vSpeed ;
			vDir.Normalize() ;
			//
			bullet.nTrackCount = 1 ;
			if ( !(bullet.nStateFlags & stateNoHitConditions) )
			{
				bullet.fpLength = (float32_t) m_fpLaserMaxLength ;
			}
			bullet.vTrack[1] = bullet.vPos ;
			bullet.vTrack[0] = bullet.vPos + vDir * bullet.fpLength ;
			//
			bullet.nStateFlags |= stateMovedTrack ;
		}
		if ( bullet.secLife >= m_secLaserPrepareDuration )
		{
			if ( bullet.secLife < m_secLaserPrepareDuration + m_secLaserFlashDuration )
			{
				double	t = (bullet.secLife - m_secLaserPrepareDuration)
												/ m_secLaserFlashDuration ;
				bullet.fpThickness = bullet.fpBaseThickness
					* (float32_t) ((m_fpLaserFlashThickness - 1.0) * (1.0 - t) + 1.0) ;
			}
			else
			{
				bullet.fpThickness = bullet.fpBaseThickness ;
			}
			bullet.nAlpha = 0xFF ;
		}
		else
		{
			bullet.nStateFlags |= statePrepareLaser ;
			bullet.fpThickness = bullet.fpBaseThickness
								* (float32_t) m_fpLaserPrepareThickness ;
			bullet.nAlpha = (uint32_t) (m_fpLaserPrepareAlpha * 0xFF) ;
		}
	}
	else if ( m_nBulletFlags & flagDirection )
	{
		if ( m_nMaxTrackCount >= 1 )
		{
			S3DDVector	vDir = bullet.vSpeed ;
			float32_t	fpSpeed = (float32_t) vDir.Absolute() ;
			if ( fpSpeed > 0.0f )
			{
				bullet.fpLength += fpSpeed ;
				vDir *= 1.0f / fpSpeed ;
				//
				bullet.nTrackCount = 1 ;
				bullet.vTrack[1] = bullet.vPos ;
				bullet.vTrack[0] = bullet.vPos + vDir * bullet.fpLength ;
				//
				bullet.nStateFlags |= stateMovedTrack ;
			}
		}
	}
	else
	{
		S3DDVector	vNewPos = bullet.vPos + bullet.vSpeed * secPast ;
		MoveBullet( bullet, vNewPos ) ;
	}
	bullet.nStateFlags |= stateControlMoved ;
	return	resultMoved ;
}

// Bullet をフェードアウト開始
//////////////////////////////////////////////////////////////////////////////
void S3DBulletItemInterface::MakeBulletFadeout
	( S3DBulletItemInterface::Bullet& bullet ) const
{
	if ( !(bullet.nStateFlags & stateLifeFadeout) )
	{
		bullet.nStateFlags =
			(bullet.nStateFlags & ~stateNoHitTimer) | stateLifeFadeout ;
		bullet.secTimer = 0.0f ;
	}
}

// Bullet を当たり判定インターバル開始
//////////////////////////////////////////////////////////////////////////////
void S3DBulletItemInterface::MakeBulletNotHitInterval
	( S3DBulletItemInterface::Bullet& bullet ) const
{
	if ( !(bullet.nStateFlags & (stateLifeFadeout | stateNoHitTimer)) )
	{
		bullet.nStateFlags |= stateNoHitTimer ;
		bullet.secTimer = 0.0f ;
	}
}

// Bullet を衝突削除設定
//////////////////////////////////////////////////////////////////////////////
void S3DBulletItemInterface::MakeBulletHitDestroy
	( S3DBulletItemInterface::Bullet& bullet ) const
{
	bullet.nStateFlags |= stateHitDestroy ;
}

// Bullet を削除設定
//////////////////////////////////////////////////////////////////////////////
void S3DBulletItemInterface::MakeBulletDestroy
	( S3DBulletItemInterface::Bullet& bullet ) const
{
	bullet.nStateFlags |= stateControlDestroy ;
}

// Bullet に新しい座標を追加
//////////////////////////////////////////////////////////////////////////////
void S3DBulletItemInterface::MoveBullet
	( S3DBulletItemInterface::Bullet& bullet, const S3DDVector& vPos ) const
{
	if ( ++ bullet.nTrackCount > m_nMaxTrackCount )
	{
		bullet.nTrackCount = (uint32_t) m_nMaxTrackCount ;
		bullet.nStateFlags |= stateMovedTrackTail ;
	}
	for ( int i = (int) bullet.nTrackCount; i >= 1; i -- )
	{
		bullet.vTrack[i] = bullet.vTrack[i - 1] ;
	}
	bullet.nStateFlags |= stateControlMoved ;
	bullet.vPos = vPos ;
	bullet.vTrack[0] = bullet.vPos ;
}

// リスナ追加
//////////////////////////////////////////////////////////////////////////////
void S3DBulletItemInterface::AddBulletListener
	( S3DBulletItemInterface::BulletListener * pListener )
{
	m_csBullets.Lock() ;
	m_aListeners.Add( pListener ) ;
	m_csBullets.Unlock() ;
}

// リスナ削除
//////////////////////////////////////////////////////////////////////////////
void S3DBulletItemInterface::DetachBulletListener
	( S3DBulletItemInterface::BulletListener * pListener )
{
	m_csBullets.Lock() ;
	ssize_t	i = m_aListeners.FindPtr( pListener ) ;
	if ( i >= 0 )
	{
		m_aListeners.RemoveAt( (size_t) i ) ;
	}
	m_csBullets.Unlock() ;
}

// リスナ数取得
//////////////////////////////////////////////////////////////////////////////
size_t S3DBulletItemInterface::GetBulletListenerCount( void ) const
{
	return	m_aListeners.GetLength() ;
}

S3DBulletItemInterface::BulletListener *
	S3DBulletItemInterface::GetBulletListenerAt( size_t i ) const
{
	BulletListener *	pListener ;
	m_csBullets.Lock() ;
	pListener = m_aListeners.GetAt( i ) ;
	m_csBullets.Unlock() ;
	return	pListener ;
}

// 当たり判定
//////////////////////////////////////////////////////////////////////////////
void S3DBulletItemInterface::TestHitToCollider
	( S3DBulletItemInterface::MultiHitInstance& mhiHits,
		const S3DCollider& collider,
		const S3DBulletItemInterface::Bullet& bullet,
		const S3DDMatrix& matSpace,
		const S3DDVector& vSpace, float32_t fpHitRadius ) const
{
	mhiHits.ResetInstance() ;
	mhiHits.SetCurrentTrack( 0 ) ;
	//
	if ( bullet.nTrackCount >= 1 )
	{
		S3DCollisionResult	rsCross ;
		S3DDVector	vPos0 = matSpace * bullet.vTrack[1] + vSpace ;
		S3DDVector	vPos1 = matSpace * bullet.vTrack[0] + vSpace ;
		rsCross.SetInclusionSceneFlags( 0 ) ;
		rsCross.SetInclusionUserFlags( m_maskColliderClasses ) ;
		rsCross.fpDistance = (float32_t) (vPos1 - vPos0).Absolute() ;
		rsCross.pfnOnHitCollider = &S3DBulletItemInterface::MultiHitInstance::Callback_OnHitRayCollider ;
		rsCross.ptrOnHitInstance = &mhiHits ;
		mhiHits.SetRay( S3DVector( vPos1 - vPos0 ) ) ;
		//
		collider.IsSegmentCrossing
			( vPos0, vPos1, fpHitRadius, rsCross ) ;
	}
	size_t	nTestCount = bullet.nTrackCount ;
	if ( nTestCount > m_nMaxColliderCount )
	{
		nTestCount = m_nMaxColliderCount ;
	}
	for ( size_t j = 0; j < nTestCount; j ++ )
	{
		if ( !(m_nBulletFlags & flagMultiHit) && !mhiHits.IsEmpty() )
		{
			break ;
		}
		ESLAssert( j + 1 <= bullet.nTrackCount ) ;
		S3DDVector	vPos0 = matSpace * bullet.vTrack[j + 1] + vSpace ;
		S3DDVector	vPos1 = matSpace * bullet.vTrack[j] + vSpace ;
		mhiHits.SetCurrentTrack( j ) ;
		mhiHits.SetRay( S3DVector( vPos1 - vPos0 ) ) ;
		//
		S3DCollisionResult	rsHit ;
		rsHit.SetInclusionSceneFlags( 0 ) ;
		rsHit.SetInclusionUserFlags( m_maskColliderClasses ) ;
		rsHit.pfnOnHitCollider = &S3DBulletItemInterface::MultiHitInstance::Callback_OnHitSphereCollider ;
		rsHit.ptrOnHitInstance = &mhiHits ;
		//
		collider.IsHitAgainstSphere( vPos1, fpHitRadius, rsHit ) ;
	}
}

// OnHitBullet 呼び出し
//////////////////////////////////////////////////////////////////////////////
void S3DBulletItemInterface::CallOnMultiHitBullet
	( const S3DBulletItemInterface::MultiHitInstance& mhi,
		const S3DMatrix& matISpace, const S3DVector& vSpace,
		S3DBulletItemInterface::Bullet& bullet )
{
	for ( size_t i = 0; i < mhi.GetLength(); i ++ )
	{
		const HitBulletEntry&	hbe = mhi.At(i) ;
		S3DVector	vHitPos = matISpace * (hbe.vHitGlobalPos - vSpace) ;
		S3DVector	vHitNormal = matISpace * hbe.vHitGlobalNormal ;
		CallOnHitBullet
			( hbe.pmcHitGlobal,
				vHitPos, vHitNormal, bullet, hbe.iHitTrack ) ;
	}
}

void S3DBulletItemInterface::CallOnHitBullet
	( const S3DCollision::MeshCollision * pmcHitMesh,
		const S3DVector& vHitPos, const S3DVector& vHitNormal,
		S3DBulletItemInterface::Bullet& bullet, size_t iHitTrack )
{
	S3DSceneComposer::ItemSerializer *	pHitItem =
		ESLTypeCast<S3DSceneComposer::ItemSerializer>
								( pmcHitMesh->pUserData ) ;
	if ( pHitItem == nullptr )
	{
		S3DPhysicsScene::Object *	pPhysObj =
			ESLTypeCast<S3DPhysicsScene::Object>( pmcHitMesh->pUserData ) ;
		if ( pPhysObj != nullptr )
		{
			pHitItem = ESLTypeCast<S3DSceneComposer::ItemSerializer>
										( pPhysObj->GetOwnerObject() ) ;
		}
	}
	for ( size_t j = 0; j < m_aListeners.GetLength(); j ++ )
	{
		BulletListener *	pListener = m_aListeners.GetAt( j ) ;
		if ( pListener != NULL )
		{
			pListener->OnHitBullet
				( this, pHitItem, pmcHitMesh,
					vHitPos, vHitNormal, bullet, iHitTrack ) ;
		}
	}
}

// Bullet 移動処理
//////////////////////////////////////////////////////////////////////////////
void S3DBulletItemInterface::OnBulletTimer
	( S3DCollider& collider,
		const S3DDMatrix& matSpace,
		const S3DDVector& vSpace, float32_t secPast )
{
	SSmartLock<SCriticalSection>	lock( &m_csBullets ) ;
	//
	S3DDMatrix	matBulletBase( 1, 1, 1 ) ;
	S3DDVector	vBulletBase( 0, 0, 0 ) ;
	if ( m_nBulletFlags & flagTransform )
	{
		matBulletBase = matSpace ;
		vBulletBase = vSpace ;
	}
	Bullet **	ppBullet = m_bullets.GetArray() ;
	size_t		nCount = m_bullets.GetLength() ;
	//
	for ( size_t i = 0; i < m_aListeners.GetLength(); i ++ )
	{
		BulletListener *	pListener = m_aListeners.GetAt( i ) ;
		if ( pListener != NULL )
		{
			pListener->BeforeBulletTimer( this, secPast, ppBullet, nCount ) ;
			ppBullet = m_bullets.GetArray() ;
			nCount = m_bullets.GetLength() ;
		}
	}
	for ( size_t i = 0; i < m_aListeners.GetLength(); i ++ )
	{
		BulletListener *	pListener = m_aListeners.GetAt( i ) ;
		if ( pListener != NULL )
		{
			pListener->OnBulletTimer( this, secPast, ppBullet, nCount ) ;
			ppBullet = m_bullets.GetArray() ;
			nCount = m_bullets.GetLength() ;
		}
	}
	MultiHitInstance	mhiHits( this ) ;
	bool				flagDeleteAny = false ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		Bullet *	pBullet = ppBullet[i] ;
		ESLAssert( ppBullet[i] != NULL ) ;
		if ( !(pBullet->nStateFlags
				& (stateControlMoved | stateControlDestroy)) )
		{
			//
			// デフォルト移動
			//
			MoveBulletResult	mbr = MoveBulletDefault( *pBullet, secPast ) ;
			if ( mbr == resultDelete )
			{
				// 削除
				DeleteBullet( pBullet ) ;
				ppBullet[i] = NULL ;
				flagDeleteAny = true ;
				continue ;
			}
		}
		else if ( pBullet->nStateFlags & stateControlDestroy )
		{
			//
			// 削除
			//
			DeleteBullet( pBullet ) ;
			ppBullet[i] = NULL ;
			flagDeleteAny = true ;
			continue ;
		}
		else if ( pBullet->nStateFlags & stateHitDestroy )
		{
			if ( pBullet->nTrackCount <= 3 )
			{
				// 削除
				DeleteBullet( pBullet ) ;
				ppBullet[i] = NULL ;
				flagDeleteAny = true ;
				continue ;
			}
			pBullet->nStateFlags |= stateMovedTrackTail ;
			pBullet->nTrackCount -- ;
		}
		//
		if ( !(pBullet->nStateFlags & stateNoHitConditions)
			&& (m_nBulletFlags & flagCollider) )
		{
			//
			// 当たり判定
			//
			TestHitToCollider
				( mhiHits, collider, *pBullet,
					matBulletBase, vBulletBase,
					m_fpColliderRadius * pBullet->fpThickness ) ;
			//
			if ( !mhiHits.IsEmpty() )
			{
				//
				// 障害物にヒット
				//
				if ( m_nBulletFlags & (flagPierce | flagLaser) )
				{
					MakeBulletNotHitInterval( *pBullet ) ;
				}
				else
				{
					MakeBulletHitDestroy( *pBullet ) ;
				}
				S3DMatrix	matISpace = matBulletBase.Inverse() ;
				S3DVector	vsSpace = vBulletBase ;
				//
				CallOnMultiHitBullet
					( mhiHits, matISpace, vsSpace, *pBullet ) ;
				//
				if ( m_nBulletFlags & (flagDirection | flagLaser) )
				{
					//
					// 指向性ビーム・レーザーの場合には長さを調整
					//
					S3DDVector	vDir = pBullet->vSpeed ;
					vDir.Normalize() ;
					//
					S3DDVector	vDelta = S3DDVector(mhiHits.At(0).vHitGlobalPos) - pBullet->vPos ;
					pBullet->fpLength = (float32_t) vDelta.InnerProduct( vDir ) ;
					pBullet->vTrack[0] = pBullet->vPos + vDir * pBullet->fpLength ;
				}
				//
				ppBullet = m_bullets.GetArray() ;
				nCount = m_bullets.GetLength() ;
			}
		}
		//
		// 時間経過処理
		//
		pBullet->nStateFlags &= ~stateAllMovedFlags ;
		pBullet->secLife += secPast ;
		pBullet->secTimer += secPast ;
		//
		if ( (pBullet->nStateFlags & statePrepareLaser)
			&& (pBullet->secLife >= m_secLaserPrepareDuration) )
		{
			pBullet->nStateFlags &= ~statePrepareLaser ;
			pBullet->fpThickness = pBullet->fpBaseThickness
									* (float32_t) m_fpLaserFlashThickness ;
			pBullet->nAlpha = 0xFF ;
			CallOnFireBullet( pBullet ) ;
		}
		if ( (pBullet->nStateFlags & stateNoHitTimer)
			&& (pBullet->secTimer >= m_secNoHitInterval) )
		{
			pBullet->nStateFlags &= ~stateNoHitTimer ;
		}
		if ( !(pBullet->nStateFlags & stateLifeFadeout) )
		{
			if ( pBullet->secLife >= pBullet->secMaxLife )
			{
				MakeBulletFadeout( *pBullet ) ;
			}
		}
		if ( pBullet->nStateFlags & stateLifeFadeout )
		{
			if ( pBullet->secTimer >= m_secBulletFadeout )
			{
				DeleteBullet( pBullet ) ;
				ppBullet[i] = NULL ;
				flagDeleteAny = true ;
				continue ;
			}
			pBullet->nAlpha =
				(uint32_t) ((1.0 - pBullet->secTimer / m_secBulletFadeout) * 0xFF) ;
		}
	}
	m_bullets.FinishArray() ;
	if ( flagDeleteAny )
	{
		m_bullets.TrimEmpty() ;
	}
}

// Bullet 当たり判定追加
//////////////////////////////////////////////////////////////////////////////
void S3DBulletItemInterface::AddBulletCollision
	( const S3DScene& scene, S3DCollision& collision ) const
{
	if ( !(m_nBulletFlags & flagCollision) || (m_nMaxColliderCount == 0) )
	{
		return ;
	}
	SSmartLock<const SCriticalSection>	lock( &m_csBullets ) ;
	//
	S3DDMatrix	matI( 1, 1, 1 ) ;
	S3DDVector	vZero( 0, 0, 0 ) ;
	collision.PushTransformation() ;
	if ( !(m_nBulletFlags & flagTransform) )
	{
		collision.SetMatrixTransformation( matI, vZero ) ;
		collision.SetUserClassesMask( m_maskCollisionClasses ) ;
	}
	//
	SArray<S3DVector4>	bufVertex ;
	Bullet *const*		ppBullet = m_bullets.GetConstArray() ;
	size_t				nCount = m_bullets.GetLength() ;
	//
	for ( size_t i = 0; i < m_aListeners.GetLength(); i ++ )
	{
		BulletListener *	pListener = m_aListeners.GetAt( i ) ;
		if ( pListener != NULL )
		{
			pListener->RenderBulletCollision
				( scene, collision, this, ppBullet, nCount ) ;
			ppBullet = m_bullets.GetConstArray() ;
			nCount = m_bullets.GetLength() ;
		}
	}
	for ( size_t i = 0; i < nCount; i ++ )
	{
		Bullet *	pBullet = ppBullet[i] ;
		if ( pBullet == NULL )
		{
			continue ;
		}
		if ( pBullet->nStateFlags & statePreventCollider )
		{
			pBullet->nStateFlags &= ~statePreventCollider ;
			continue ;
		}
		if ( pBullet->nStateFlags & (stateNoHitConditions | statePrepareLaser) )
		{
			continue ;
		}
		size_t	nTracks = pBullet->nTrackCount ;
		if ( nTracks > m_nMaxColliderCount )
		{
			nTracks = m_nMaxColliderCount ;
		}
		if ( nTracks >= 2 )
		{
			S3DVector4 *	pvVertex = bufVertex.GetArray( nTracks ) ;
			for ( size_t j = 0; j < nTracks; j ++ )
			{
				pvVertex[j] = pBullet->vTrack[j] ;
			}
			collision.SetCurrentThickness( m_fpColliderRadius * pBullet->fpThickness ) ;
			collision.AddIndexedPrimitiveList
				( NULL, 0, primitiveLineStrip, 0,
					nTracks, pvVertex, NULL, NULL, NULL, NULL ) ;
			bufVertex.FinishArray() ;
		}
		else
		{
			S3DVector	vPos = pBullet->vPos ;
			collision.SetCurrentThickness( 0.0f ) ;
			collision.AddSolidSphere
				( vPos, m_fpColliderRadius * pBullet->fpThickness, i ) ;
		}
	}
	collision.PopTransformation() ;
}

// Bullet 描画
//////////////////////////////////////////////////////////////////////////////
void S3DBulletItemInterface::RenderBulletTracks( S3DScene& scene ) const
{
	SSmartLock<const SCriticalSection>	lock( &m_csBullets ) ;
	//
	for ( size_t i = 0; i < m_aListeners.GetLength(); i ++ )
	{
		BulletListener *	pListener = m_aListeners.GetAt( i ) ;
		if ( pListener != NULL )
		{
			pListener->RenderBulletTracks
				( scene, this, m_bullets.GetConstArray(), m_bullets.GetLength() ) ;
		}
	}
}



//////////////////////////////////////////////////////////////////////////////
// 弾幕発射アイテム
//////////////////////////////////////////////////////////////////////////////

const S3DSceneComposer::ParamEntry
	S3DBulletItemSerializer::m_paramEntries
		[S3DBulletItemSerializer::paramBulletCount] =
{
	{ L"enable_collider",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant1,
		L"当たり判定", L"当たり判定を行います。" },
	{ L"collider_classes",
		S3DSceneComposer::typeInteger,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrFlagSetInteger,
		L"衝突フラグ",
		L"当たり判定の対象を判別するためのビット集合を指定する。\n"
		L"システム既定値として 0x01 が形状、0x02 が移動障壁として定義済み。\n"
		L"0x04 は当たり判定（敵）、0x08 は（敵）攻撃当たり判定、0x10 はイベント発生判定として推奨。\n"
		L"0x10～0x80 は未定義の予約領域で、0x0100 以上がユーザー領域である。" },
	{ L"enable_collision",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant1,
		L"被当たり判定", L"被当たり判定を設定します" },
	{ L"collision_classes",
		S3DSceneComposer::typeInteger,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrFlagSetInteger,
		L"被衝突フラグ",
		L"当たり判定の対象を判別するためのビット集合を指定する。\n"
		L"システム既定値として 0x01 が形状、0x02 が移動障壁として定義済み。\n"
		L"0x04 は当たり判定（敵）、0x08 は（敵）攻撃当たり判定、0x10 はイベント発生判定として推奨。\n"
		L"0x10～0x80 は未定義の予約領域で、0x0100 以上がユーザー領域である。" },
	{ L"collider_track_count",
		S3DSceneComposer::typeInteger,
		S3DSceneComposer::attrConstant1,
		L"当たり判定軌跡長", L"当たり判定に使用する軌跡頂点数" },
	{ L"collider_radius",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant1,
		L"当たり判定半径", L"当たり判定の半径を指定します" },
	{ L"enable_pierce",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant1,
		L"貫通弾", L"貫通弾として処理します" },
	{ L"enable_direction",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant1,
		L"指向ビーム", L"指向ビームとして処理します\n発射元の方向は発射後に変更できます" },
	{ L"enable_laser",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant1,
		L"レーザー", L"レーザーとして処理します\n速度が無限大の指向ビームです" },
	{ L"enable_multi_hit",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant1,
		L"同時多数当たり判定", L"同時に多数のアイテムに当たり判定できます" },
	{ L"enable_transform",
		S3DSceneComposer::typeBoolean,
		S3DSceneComposer::attrConstant1,
		L"座標変換あり", L"弾の座標をローカル座標として座標変換を行います" },
	{ L"max_track_count",
		S3DSceneComposer::typeInteger,
		S3DSceneComposer::attrConstant1,
		L"軌跡最大数", L"軌跡描画のための記録する頂点数" },
	{ L"bullet_life",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant1,
		L"寿命", L"最大継続時間 [秒]" },
	{ L"beam_max_length",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant1,
		L"レーザー最大長", L"レーザーの最大到達距離" },
	{ L"laser_pre_time",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant1,
		L"レーザー予備時間", L"レーザー発射前のガイド表示時間 [秒]" },
	{ L"laser_pre_thickness",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrUIScalarSlider,
		L"レーザー発射前幅", L"レーザー発射前のガイド表示幅（比率）", 0.0, 1.0 },
	{ L"laser_pre_alpha",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrUIScalarSlider,
		L"レーザー発射前α", L"レーザー発射前の表示不透明度", 0.0, 1.0 },
	{ L"laser_flash_time",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant1,
		L"レーザー発射閃光時間", L"レーザー発射直後の閃光時間 [秒]" },
	{ L"laser_flash_thickness",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant1
		| S3DSceneComposer::attrUIScalarSlider,
		L"レーザー発射閃光幅", L"レーザー発射直後の閃光表示幅（比率）", 0.0, 1.0 },
	{ L"bullet_fadeout",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant1,
		L"フェードアウト時間", L"フェードアウト時間 [秒]" },
	{ L"hit_interval",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrConstant1,
		L"衝突インターバル", L"貫通弾の当たり判定インターバル [秒]" },
} ;

const S3DSceneComposer::ParamSetClass
	S3DBulletItemSerializer::m_pscClass =
{
	&ItemBasicSerializer::m_pscClass,
	S3DBulletItemSerializer::paramBulletCount,
	&S3DBulletItemSerializer::m_paramEntries[0]
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO2
	( SakuraGL::S3DBulletItemSerializer,
			ItemBasicSerializer, S3DBulletItemInterface )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DBulletItemSerializer, bullet )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DBulletItemSerializer::S3DBulletItemSerializer( void )
	: ItemBasicSerializer
		( m_ItemClassDescriptor.pwszClassID,
				&S3DBulletItemSerializer::m_pscClass )
{
	m_flagsBehavior |= S3DScene::itemTimer ;
	m_classItem = S3DScene::classDynamicItem1 ;
	m_maskClasses |= (1 << S3DScene::classPreRender) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DBulletItemSerializer::~S3DBulletItemSerializer( void )
{
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
double S3DBulletItemSerializer::GetScalarParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramColliderRadius:
		return	GetColliderRadius() ;
	case	paramBulletMaxLife:
		return	GetBulletMaxLife() ;
	case	paramBeamMaxLength:
		return	GetLaserMaxLength() ;
	case	paramLaserPrepareDuration:
		return	GetLaserPrepareDuration() ;
	case	paramLaserPrepareThickness:
		return	GetLaserPrepareThickness() ;
	case	paramLaserPrepareAlpha:
		return	GetLaserPrepareAlpha() ;
	case	paramLaserFlashDuration:
		return	GetLaserFlashDuration() ;
	case	paramLaserFlashThickness:
		return	GetLaserFlashThickness() ;
	case	paramBulletFadeout:
		return	GetBulletFadeout() ;
	case	paramBulletHitInterval:
		return	GetBulletHitInterval() ;
	}
	return	ItemBasicSerializer::GetScalarParameter( i ) ;
}

int32_t S3DBulletItemSerializer::GetIntegerParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramColliderClasses:
		return	(int32_t) GetColliderTargetClasses() ;
	case	paramCollisionClasses:
		return	(int32_t) GetCollisionClasses() ;
	case	paramColliderTrackCount:
		return	(int32_t) GetTrackColliderCount() ;
	case	paramBulletTrackCount:
		return	(int32_t) GetMaxTrackCount() ;
	}
	return	ItemBasicSerializer::GetIntegerParameter( i ) ;
}

bool S3DBulletItemSerializer::GetBooleanParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramEnableCollider:
		return	(GetBulletFlags() & flagCollider) != 0 ;
	case	paramEnableCollision:
		return	(GetBulletFlags() & flagCollision) != 0 ;
	case	paramEnablePierce:
		return	(GetBulletFlags() & flagPierce) != 0 ;
	case	paramEnableDirection:
		return	(GetBulletFlags() & flagDirection) != 0 ;
	case	paramEnableLaser:
		return	(GetBulletFlags() & flagLaser) != 0 ;
	case	paramEnableMultiHit:
		return	(GetBulletFlags() & flagMultiHit) != 0 ;
	case	paramEnableTransform:
		return	(GetBulletFlags() & flagTransform) != 0 ;
	}
	return	ItemBasicSerializer::GetBooleanParameter( i ) ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DBulletItemSerializer::SetScalarParameter( size_t i, double s )
{
	switch ( i )
	{
	case	paramColliderRadius:
		SetColliderRadius( (float32_t) s ) ;
		return ;
	case	paramBulletMaxLife:
		SetBulletMaxLife( s ) ;
		return ;
	case	paramBeamMaxLength:
		SetLaserMaxLength( s ) ;
		return ;
	case	paramLaserPrepareDuration:
		SetLaserPrepareDuration( s ) ;
		return ;
	case	paramLaserPrepareThickness:
		SetLaserPrepareThickness( s ) ;
		return ;
	case	paramLaserPrepareAlpha:
		SetLaserPrepareAlpha( s ) ;
		return ;
	case	paramLaserFlashDuration:
		SetLaserFlashDuration( s ) ;
		return ;
	case	paramLaserFlashThickness:
		SetLaserFlashThickness( s ) ;
		return ;
	case	paramBulletFadeout:
		SetBulletFadeout( s ) ;
		return ;
	case	paramBulletHitInterval:
		SetBulletHitInterval( s ) ;
		return ;
	}
	ItemBasicSerializer::SetScalarParameter( i, s ) ;
}

void S3DBulletItemSerializer::SetIntegerParameter( size_t i, int32_t n )
{
	switch ( i )
	{
	case	paramColliderClasses:
		SetColliderTargetClasses( (uint32_t) n ) ;
		return ;
	case	paramCollisionClasses:
		SetCollisionClasses( (uint32_t) n ) ;
		return ;
	case	paramColliderTrackCount:
		SetTrackColliderCount( (size_t) n ) ;
		return ;
	case	paramBulletTrackCount:
		SetMaxTrackCount( (size_t) n ) ;
		return ;
	}
	ItemBasicSerializer::SetIntegerParameter( i, n ) ;
}

void S3DBulletItemSerializer::SetBooleanParameter( size_t i, bool b )
{
	switch ( i )
	{
	case	paramEnableCollider:
		SetBulletFlags
			( (GetBulletFlags() & ~flagCollider)
				| (b ? flagCollider : 0) ) ;
		return ;
	case	paramEnableCollision:
		SetBulletFlags
			( (GetBulletFlags() & ~flagCollision)
				| (b ? flagCollision : 0) ) ;
		m_flagsBehavior =
			(m_flagsBehavior & ~S3DScene::itemCollision)
				| (b ? S3DScene::itemCollision : 0) ;
		return ;
	case	paramEnablePierce:
		SetBulletFlags
			( (GetBulletFlags() & ~flagPierce)
				| (b ? flagPierce : 0) ) ;
		return ;
	case	paramEnableDirection:
		SetBulletFlags
			( (GetBulletFlags() & ~flagDirection)
				| (b ? flagDirection : 0) ) ;
		return ;
	case	paramEnableLaser:
		SetBulletFlags
			( (GetBulletFlags() & ~flagLaser)
				| (b ? flagLaser : 0) ) ;
		return ;
	case	paramEnableMultiHit:
		SetBulletFlags
			( (GetBulletFlags() & ~flagMultiHit)
				| (b ? flagMultiHit : 0) ) ;
		return ;
	case	paramEnableTransform:
		SetBulletFlags
			( (GetBulletFlags() & ~flagTransform)
				| (b ? flagTransform : 0) ) ;
		return ;
	}
	ItemBasicSerializer::SetBooleanParameter( i, b ) ;
}

// パラメーター有効性
//////////////////////////////////////////////////////////////////////////////
bool S3DBulletItemSerializer::IsParameterValidation( size_t i ) const
{
	switch ( i )
	{
	case	paramZoom:
	case	paramTransparency:
	case	paramColorMul:
	case	paramColorAdd:
	case	paramForceToon:
	case	paramForceBorder:
	case	paramFreeToon:
	case	paramFreeBorder:
	case	paramUseCollision:
	case	paramHideNear:
	case	paramHideFar:
		return	false ;
	}
	return	ItemBasicSerializer::IsParameterValidation( i ) ;
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DBulletItemSerializer::GetParameterCategoryName( size_t iCategory ) const
{
	switch ( iCategory )
	{
	case	0:
		return	L"基本設定" ;
	case	1:
		return	L"弾設定" ;
	}
	return	ItemBasicSerializer::GetParameterCategoryName( iCategory ) ;
}

// Loquaty クラス名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DBulletItemSerializer::GetLQClassName( void ) const
{
	return	L"EntisGLS4.SceneBulleteItem" ;
}

// コントローラー通知
//////////////////////////////////////////////////////////////////////////////
void S3DBulletItemSerializer::OnAddController
	( S3DSceneComposer::Controller * pController )
{
	S3DBulletItemInterface::BulletListener *	pListener =
		ESLTypeCast<S3DBulletItemInterface::BulletListener>( pController ) ;
	if ( pListener != NULL )
	{
		AddBulletListener( pListener ) ;
	}
	ItemBasicSerializer::OnAddController( pController ) ;
}

void S3DBulletItemSerializer::BeforeDetachController
	( S3DSceneComposer::Controller * pController )
{
	S3DBulletItemInterface::BulletListener *	pListener =
		ESLTypeCast<S3DBulletItemInterface::BulletListener>( pController ) ;
	if ( pListener != NULL )
	{
		DetachBulletListener( pListener ) ;
	}
	ItemBasicSerializer::BeforeDetachController( pController ) ;
}

// タイマ処理
//////////////////////////////////////////////////////////////////////////////
void S3DBulletItemSerializer::OnTimer( S3DScene& scene, uint32_t msecPast )
{
	S3DDMatrix	matSpace( 1, 1, 1 ) ;
	S3DDVector	vSpace( 0, 0, 0 ) ;
	GetGlobalTransformation( matSpace, vSpace ) ;
	OnBulletTimer
		( scene, matSpace, vSpace, (float32_t) msecPast / 1000.0f ) ;
	//
	ItemBasicSerializer::OnTimer( scene, msecPast ) ;
}

// 当たり判定追加
//////////////////////////////////////////////////////////////////////////////
void S3DBulletItemSerializer::OnItemRenderCollision
	( const S3DScene& scene, S3DCollision& render )
{
	AddBulletCollision( scene, render ) ;
	//
	ItemBasicSerializer::OnItemRenderCollision( scene, render ) ;
}

// レンダリング前後処理（全視点共通）
//////////////////////////////////////////////////////////////////////////////
void S3DBulletItemSerializer::OnItemRenderEvent
	( S3DScene& scene, S3DScene::ItemClass clsItem )
{
	if ( clsItem == S3DScene::classPreRender )
	{
		RenderBulletTracks( scene ) ;
	}
	ItemBasicSerializer::OnItemRenderEvent( scene, clsItem ) ;
}

// 空間行列取得
//////////////////////////////////////////////////////////////////////////////
void S3DBulletItemSerializer::GetGlobalTransformation
	( S3DDMatrix& matGlobal, S3DDVector& vGlobalPos ) const
{
	ItemBasicSerializer::GetGlobalTransformation( matGlobal, vGlobalPos ) ;
}



//////////////////////////////////////////////////////////////////////////////
// 砲台アイテム
//////////////////////////////////////////////////////////////////////////////

const S3DSceneComposer::ParamEntry
	S3DBatteryItemSerializer::m_paramEntries
		[S3DBatteryItemSerializer::paramBatteryCount] =
{
	{ L"bullet_item",
		S3DSceneComposer::typeSelector,
		S3DSceneComposer::attrCategory1
		| S3DSceneComposer::attrStringEnumeration,
		L"弾アイテム", L"発射する弾アイテムを指定します" },
	{ L"interval",
		S3DSceneComposer::typeInteger,
		S3DSceneComposer::attrCategory1,
		L"発射間隔", L"発射する間隔[frame]を指定します" },
	{ L"fire_count",
		S3DSceneComposer::typeInteger,
		S3DSceneComposer::attrCategory1,
		L"発射数", L"発射する数を指定します" },
	{ L"scatter_type",
		S3DSceneComposer::typeSelector,
		S3DSceneComposer::attrCategory1
		| S3DSceneComposer::attrStringEnumeration
		| S3DSceneComposer::attrUIOnlyEnumeration
		| S3DSceneComposer::attrDynamicValidation,
		L"撒布タイプ", L"弾の撒き方を指定します" },
	{ L"direction",
		S3DSceneComposer::typeDirection,
		S3DSceneComposer::attrCategory1,
		L"方向", L"発射する方向を指定します" },
	{ L"sub_direction",
		S3DSceneComposer::typeDirection,
		S3DSceneComposer::attrCategory1,
		L"副方向", L"平面上に複数発射する場合の発射角と平面を指定します" },
	{ L"speed",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1,
		L"速度", L"発射する弾の速度を指定します" },
	{ L"thickness",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1
		| S3DSceneComposer::attrUIScalarSlider,
		L"太さ", L"発射する弾の太さの比率を指定します", 0.0, 1.0 },
	{ L"scatter_angle",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1,
		L"撒布範囲角", L"弾をばら撒く範囲を指定します [deg]" },
	{ L"cone_angle",
		S3DSceneComposer::typeScalar,
		S3DSceneComposer::attrCategory1,
		L"円錐角", L"弾を円錐状にばら撒く時の角度を指定します [deg]" },
} ;

const S3DSceneComposer::ParamSetClass
	S3DBatteryItemSerializer::m_pscClass =
{
	&ItemBasicSerializer::m_pscClass,
	S3DBatteryItemSerializer::paramBatteryCount,
	&S3DBatteryItemSerializer::m_paramEntries[0]
} ;

const wchar_t *	S3DBatteryItemSerializer::m_pwszScatterType
					[S3DBatteryItemSerializer::scatterCount] =
{
	L"1way", L"1way_beam", L"odd_x_way", L"event_x_way",
	L"2d_random", L"3d_random", L"cone_3d_random"
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO
	( SakuraGL::S3DBatteryItemSerializer, ItemBasicSerializer )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DBatteryItemSerializer, battery )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DBatteryItemSerializer::S3DBatteryItemSerializer( void )
	: ItemBasicSerializer
		( m_ItemClassDescriptor.pwszClassID,
				&S3DBatteryItemSerializer::m_pscClass )
{
	m_flagsBehavior |= S3DScene::itemTimer ;
	//
	m_nFireInterval = 10 ;
	m_fpIntervalCounter = 0.0 ;
	m_fpLastFrame = 0.0 ;
	m_nFireCount = 1 ;
	m_typeScatter = scatter1Way ;
	m_vDirection = S3DDVector( 0, 0, 1 ) ;
	m_vSubDirection = S3DDVector( 1, 0, 0 ) ;
	m_fpBulletSpeed = 10.0 ;
	m_fpBulletThickness = 1.0 ;
	m_degScatterAngle = 3.0 ;
	m_degConeAngle = 15.0 ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DBatteryItemSerializer::~S3DBatteryItemSerializer( void )
{
}

// ターゲットを設定
//////////////////////////////////////////////////////////////////////////////
void S3DBatteryItemSerializer::AttachBulletItem
	( S3DBulletItemSerializer * pItem, const wchar_t * pwszID )
{
	m_refBulletItem.SetReference
		( (S3DSceneComposer::ItemSerializer*) pItem ) ;
	m_strBulletItem = pwszID ;
}

bool S3DBatteryItemSerializer::UpdateBulletReference( void )
{
	if ( m_strBulletItem.IsEmpty() )
	{
		m_refBulletItem.SetReference( NULL ) ;
		return	true ;
	}
	S3DSceneComposer::Composition *	pComp = GetComposition() ;
	if ( pComp != NULL )
	{
		S3DSceneComposer::ItemSerializer *
			pItem = pComp->GetSceneItemAs( m_strBulletItem ) ;
		m_refBulletItem.SetReference( pItem ) ;
		//
		return	(pItem != NULL) ;
	}
	return	false ;
}

// 発射間隔 [frame]
//////////////////////////////////////////////////////////////////////////////
size_t S3DBatteryItemSerializer::GetFireInterval( void ) const
{
	return	m_nFireInterval ;
}

void S3DBatteryItemSerializer::SetFireInterval( size_t nInterval )
{
	m_nFireInterval = nInterval ;
}

// 発射弾数
//////////////////////////////////////////////////////////////////////////////
size_t S3DBatteryItemSerializer::GetFireCount( void ) const
{
	return	m_nFireCount ;
}

void S3DBatteryItemSerializer::SetFireCount( size_t nCount )
{
	m_nFireCount = nCount ;
}

// ばら撒き方
//////////////////////////////////////////////////////////////////////////////
S3DBatteryItemSerializer::ScatterType
	S3DBatteryItemSerializer::GetScatterType( void ) const
{
	return	m_typeScatter ;
}

void S3DBatteryItemSerializer::SetScatterType( S3DBatteryItemSerializer::ScatterType type )
{
	m_typeScatter = type ;
}

// 方向（ローカル空間）
//////////////////////////////////////////////////////////////////////////////
const S3DDVector& S3DBatteryItemSerializer::GetDirection( void ) const
{
	return	m_vDirection ;
}

void S3DBatteryItemSerializer::SetDirection( const S3DDVector& vDir )
{
	m_vDirection = vDir ;
}

// 副方向（ローカル空間）
//////////////////////////////////////////////////////////////////////////////
const S3DDVector& S3DBatteryItemSerializer::GetSubDirection( void ) const
{
	return	m_vSubDirection ;
}

void S3DBatteryItemSerializer::SetSubDirection( const S3DDVector& vSubDir )
{
	m_vSubDirection = vSubDir ;
}

// 弾速度
//////////////////////////////////////////////////////////////////////////////
double S3DBatteryItemSerializer::GetBulletSpeed( void ) const
{
	return	m_fpBulletSpeed ;
}

void S3DBatteryItemSerializer::SetBulletSpeed( double fpSpeed )
{
	m_fpBulletSpeed = fpSpeed ;
}

// 弾太さ比率
//////////////////////////////////////////////////////////////////////////////
double S3DBatteryItemSerializer::GetBulletThickness( void ) const
{
	return	m_fpBulletThickness ;
}

void S3DBatteryItemSerializer::SetBulletThickness( double fpThickness )
{
	m_fpBulletThickness = fpThickness ;
}

// ばら撒き範囲角 [deg]
//////////////////////////////////////////////////////////////////////////////
double S3DBatteryItemSerializer::GetScatterAngle( void ) const
{
	return	m_degScatterAngle ;
}

void S3DBatteryItemSerializer::SetScatterAngle( double degAngle )
{
	m_degScatterAngle = degAngle ;
}

// 円錐角 [deg]
//////////////////////////////////////////////////////////////////////////////
double S3DBatteryItemSerializer::GetConeAngle( void ) const
{
	return	m_degConeAngle ;
}

void S3DBatteryItemSerializer::SetConeAngle( double degAngle )
{
	m_degConeAngle = degAngle ;
}

// 弾発射
//////////////////////////////////////////////////////////////////////////////
void S3DBatteryItemSerializer::FireBullet( size_t nCount )
{
	S3DBulletItemSerializer *	pBulletItem =
				ESLTypeCast<S3DBulletItemSerializer>
					( m_refBulletItem.GetReference() ) ;
	if ( pBulletItem == NULL )
	{
		return ;
	}
	S3DDMatrix	matItem ;
	S3DDVector	vItem ;
	GetGlobalTransformation( matItem, vItem ) ;
	//
	S3DCompositionManager *	pManager = GetManager() ;
	if ( pManager != NULL )
	{
		switch ( m_typeScatter )
		{
		case	scatter2DRandom:
		case	scatter3DRandom:
		case	scatter3DRandomCone:
			return ;
		default:
			break ;
		}
	}
	S3DDVector	vSpeed = matItem * m_vDirection ;
	vSpeed.Normalize() ;
	vSpeed *= m_fpBulletSpeed ;
	//
	float32_t	fpThickness = (float32_t) m_fpBulletThickness ;
	//
	S3DDMatrix	matWayRot( 1, 1, 1 ), matIWayRot( 1, 1, 1 ) ;
	S3DDVector	vRSpeed, vXaxis, vZaxis ;
	size_t		i ;
	switch ( m_typeScatter )
	{
	case	scatter1Way:
		pBulletItem->GenerateBullet
			( vItem, vSpeed, fpThickness, NULL ) ;
		break ;

	case	scatter1WayBeam:
		m_aBeamBullet.Add
			( pBulletItem->GenerateBullet
				( vItem, vSpeed,
					(float32_t) m_fpBulletThickness, NULL ) ) ;
		break ;

	case	scatterOddNWay:
		matWayRot.VectorRotationOf( m_vDirection, m_vSubDirection ) ;
		matIWayRot.InverseOf( matWayRot ) ;
		//
		pBulletItem->GenerateBullet
			( vItem, vSpeed, fpThickness, NULL ) ;
		//
		matWayRot = matItem * matWayRot ;
		matIWayRot = matItem * matIWayRot ;
		vRSpeed = vSpeed ;
		for ( i = 1; i + 2 <= nCount; i += 2 )
		{
			matWayRot.RevolveVector( vSpeed ) ;
			matIWayRot.RevolveVector( vRSpeed ) ;
			//
			pBulletItem->GenerateBullet
				( vItem, vSpeed, fpThickness, NULL ) ;
			pBulletItem->GenerateBullet
				( vItem, vRSpeed, fpThickness, NULL ) ;
		}
		break ;

	case	scatterEvenNWay:
		matWayRot.VectorRotationOf( m_vDirection, m_vSubDirection ) ;
		matIWayRot.InverseOf( matWayRot ) ;
		//
		matWayRot = matItem * matWayRot ;
		matIWayRot = matItem * matIWayRot ;
		vRSpeed = vSpeed ;
		//
		matWayRot.RevolveVector( vSpeed ) ;
		matIWayRot.RevolveVector( vRSpeed ) ;
		//
		for ( i = 0; i + 2 <= nCount; i += 2 )
		{
			pBulletItem->GenerateBullet
				( vItem, vSpeed, fpThickness, NULL ) ;
			pBulletItem->GenerateBullet
				( vItem, vRSpeed, fpThickness, NULL ) ;
			//
			matWayRot.RevolveVector( vSpeed ) ;
			matWayRot.RevolveVector( vSpeed ) ;
			matIWayRot.RevolveVector( vRSpeed ) ;
			matIWayRot.RevolveVector( vRSpeed ) ;
		}
		break ;

	case	scatter2DRandom:
		vXaxis = (m_vDirection * m_vSubDirection) * m_vDirection ;
		vZaxis = m_vDirection ;
		vXaxis.Normalize() ;
		vZaxis.Normalize() ;
		vXaxis = matItem * vXaxis ;
		vZaxis = matItem * vZaxis ;
		//
		for ( i = 0; i < nCount; i ++ )
		{
			double	rad = pManager->Randomizer().QuickRandomDouble
								( m_degScatterAngle * (PI / 180.0) ) ;
			vSpeed = vZaxis * cos(rad) + vXaxis * sin(rad) ;
			vSpeed *= m_fpBulletSpeed ;
			//
			pBulletItem->GenerateBullet
				( vItem, vSpeed, fpThickness, NULL ) ;
		}
		break ;

	case	scatter3DRandom:
		matWayRot.RevolveForAngle( vSpeed ) ;
		//
		for ( i = 0; i < nCount; i ++ )
		{
			double	rad = pManager->Randomizer().QuickRandomDouble
								( m_degScatterAngle * (PI / 180.0) ) ;
			double	z = cos(rad) * m_fpBulletSpeed ;
			double	s = sin(rad) * m_fpBulletSpeed ;
			rad = pManager->Randomizer().QuickRandomDouble( PI * 2.0 ) ;
			//
			vSpeed = matWayRot
						* S3DDVector( s * cos(rad), s * sin(rad), z ) ;
			pBulletItem->GenerateBullet
				( vItem, vSpeed, fpThickness, NULL ) ;
		}
		break ;

	case	scatter3DRandomCone:
		matWayRot.RevolveForAngle( vSpeed ) ;
		//
		for ( i = 0; i < nCount; i ++ )
		{
			double	rad = m_degConeAngle * PI / 180.0
							+ pManager->Randomizer().QuickRandomDouble
									( m_degScatterAngle * (PI / 180.0) ) ;
			double	z = cos(rad) * m_fpBulletSpeed ;
			double	s = sin(rad) * m_fpBulletSpeed ;
			rad = pManager->Randomizer().QuickRandomDouble( PI * 2.0 ) ;
			//
			vSpeed = matWayRot
						* S3DDVector( s * cos(rad), s * sin(rad), z ) ;
			pBulletItem->GenerateBullet
				( vItem, vSpeed, fpThickness, NULL ) ;
		}
		break ;

	default:
		break ;
	}
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDVector S3DBatteryItemSerializer::GetVectorParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramDirection:
		return	m_vDirection ;
	case	paramSubDirection:
		return	m_vSubDirection ;
	}
	return	ItemBasicSerializer::GetVectorParameter( i ) ;
}

double S3DBatteryItemSerializer::GetScalarParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramSpeed:
		return	m_fpBulletSpeed ;
	case	paramThickness:
		return	m_fpBulletThickness ;
	case	paramScatterAngle:
		return	m_degScatterAngle ;
	case	paramConeAngle:
		return	m_degConeAngle ;
	}
	return	ItemBasicSerializer::GetScalarParameter( i ) ;
}

int32_t S3DBatteryItemSerializer::GetIntegerParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramInterval:
		return	(int32_t) m_nFireInterval ;
	case	paramFireCount:
		return	(int32_t) m_nFireCount ;
	}
	return	ItemBasicSerializer::GetIntegerParameter( i ) ;
}

const wchar_t * S3DBatteryItemSerializer::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramBulletItem:
		return	m_strBulletItem ;
	case	paramScatterType:
		return	m_pwszScatterType[m_typeScatter] ;
	}
	return	ItemBasicSerializer::GetCommandParameter( i ) ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DBatteryItemSerializer::SetVectorParameter( size_t i, const S3DDVector& vec )
{
	switch ( i )
	{
	case	paramDirection:
		m_vDirection = vec ;
		return ;
	case	paramSubDirection:
		m_vSubDirection = vec ;
		return ;
	}
	ItemBasicSerializer::SetVectorParameter( i, vec ) ;
}

void S3DBatteryItemSerializer::SetScalarParameter( size_t i, double s )
{
	switch ( i )
	{
	case	paramSpeed:
		m_fpBulletSpeed = s ;
		return ;
	case	paramThickness:
		m_fpBulletThickness = s ;
		return ;
	case	paramScatterAngle:
		m_degScatterAngle = s ;
		return ;
	case	paramConeAngle:
		m_degConeAngle = s ;
		return ;
	}
	ItemBasicSerializer::SetScalarParameter( i, s ) ;
}

void S3DBatteryItemSerializer::SetIntegerParameter( size_t i, int32_t n )
{
	switch ( i )
	{
	case	paramInterval:
		m_nFireInterval = (size_t) n ;
		return ;
	case	paramFireCount:
		m_nFireCount = (size_t) n ;
		return ;
	}
	ItemBasicSerializer::SetIntegerParameter( i, n ) ;
}

void S3DBatteryItemSerializer::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	switch ( i )
	{
	case	paramBulletItem:
		if ( m_strBulletItem != pwszCmd )
		{
			m_strBulletItem = pwszCmd ;
			UpdateBulletReference() ;
		}
		return ;
	case	paramScatterType:
		{
			for ( size_t j = 0; j < scatterCount; j ++ )
			{
				if ( SString::Compare( m_pwszScatterType[j], pwszCmd ) == 0 )
				{
					m_typeScatter = (ScatterType) j ;
					break ;
				}
			}
		}
		return ;
	}
	ItemBasicSerializer::SetCommandParameter( i, pwszCmd ) ;
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DBatteryItemSerializer::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	S3DSceneComposer::Composition *	pComp ;
	switch ( i )
	{
	case	paramBulletItem:
		pComp = GetComposition() ;
		if ( pComp != NULL )
		{
			pComp->EnumerateItemIDsAs
				( aStrSet, ESL_RUNTIME_CLASS(S3DBulletItemSerializer) ) ;
		}
		return	true ;

	case	paramScatterType:
		{
			for ( size_t j = 0; j < scatterCount; j ++ )
			{
				aStrSet.Add( new SString(m_pwszScatterType[j]) ) ;
			}
		}
		return	true ;
	}
	return	ItemBasicSerializer::EnumerateStringSet( i, aStrSet ) ;
}

// パラメーター有効性
//////////////////////////////////////////////////////////////////////////////
bool S3DBatteryItemSerializer::IsParameterValidation( size_t i ) const
{
	switch ( i )
	{
	case	paramZoom:
	case	paramTransparency:
	case	paramColorMul:
	case	paramColorAdd:
	case	paramVisible:
	case	paramForceToon:
	case	paramForceBorder:
	case	paramFreeToon:
	case	paramFreeBorder:
	case	paramUseCollision:
	case	paramHideNear:
	case	paramHideFar:
	case	paramItemClass:
		return	false ;

	case	paramSubDirection:
		return	(m_typeScatter == scatterOddNWay)
				|| (m_typeScatter == scatterEvenNWay)
				|| (m_typeScatter == scatter2DRandom) ;

	case	paramScatterAngle:
		return	(m_typeScatter == scatter2DRandom)
				|| (m_typeScatter == scatter3DRandom)
				|| (m_typeScatter == scatter3DRandomCone) ;

	case	paramConeAngle:
		return	(m_typeScatter == scatter3DRandomCone) ;
	}
	return	ItemBasicSerializer::IsParameterValidation( i ) ;
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DBatteryItemSerializer::GetParameterCategoryName( size_t iCategory ) const
{
	switch ( iCategory )
	{
	case	0:
		return	L"基本設定" ;
	case	1:
		return	L"砲台設定" ;
	}
	return	NULL ;
}

// タイマ処理
//////////////////////////////////////////////////////////////////////////////
void S3DBatteryItemSerializer::OnTimer( S3DScene& scene, uint32_t msecPast )
{
	ItemBasicSerializer::OnTimer( scene, msecPast ) ;
	//
	if ( m_nFireCount > 0 )
	{
		S3DSceneComposer::Composition *	pComp= GetComposition() ;
		if ( pComp != NULL )
		{
			const S3DSceneComposer::CompositionInfo *
								pci = pComp->GetCompositionInfo() ;
			ESLAssert( pci != NULL ) ;
			double	fpFrame = pci->FrameIndexFromSecond( msecPast / 1000.0 ) ;
			double	fpError = pci->FrameIndexFromSecond( 0.0015 ) ;
			int	nFrame = (int) eslRoundR64ToLInt( fpFrame ) ;
			if ( fabs( fpFrame - nFrame ) < fpError )
			{
				m_fpIntervalCounter -= nFrame ;
			}
			else
			{
				m_fpIntervalCounter -= fpFrame ;
			}
		}
		else
		{
			m_fpIntervalCounter -= 1.0 ;
		}
		if ( m_fpIntervalCounter <= 0.0 )
		{
			m_fpIntervalCounter += m_nFireInterval ;
			if ( m_fpIntervalCounter < 0.0 )
			{
				m_fpIntervalCounter = 0.0 ;
			}
			FireBullet( m_nFireCount ) ;
		}
	}
	else
	{
		m_fpIntervalCounter = 0.0 ;
	}
	//
	if ( m_typeScatter == scatter1WayBeam )
	{
		S3DBulletItemSerializer *	pBulletItem =
					ESLTypeCast<S3DBulletItemSerializer>
						( m_refBulletItem.GetReference() ) ;
		if ( (pBulletItem != NULL)
			&& (pBulletItem->GetBulletFlags()
					& S3DBulletItemInterface::flagDirection) )
		{
			S3DDMatrix	matTrans ;
			S3DDVector	vTrans ;
			pBulletItem->GetTransformationFrom( matTrans, vTrans, this ) ;
			//
			S3DDVector	vSpeed = matTrans * m_vDirection ;
			vSpeed.Normalize() ;
			vSpeed *= m_fpBulletSpeed ;
			//
			for ( size_t i = 0; i < m_aBeamBullet.GetLength(); i ++ )
			{
				S3DBulletItemInterface::Bullet *
							pBullet = m_aBeamBullet.GetAt( i ) ;
				if ( pBulletItem->IsValidBullet( pBullet ) )
				{
					pBullet->vPos = vTrans ;
					pBullet->vSpeed = vSpeed ;
				}
				else
				{
					m_aBeamBullet.SetAt( i, NULL ) ;
				}
			}
			m_aBeamBullet.TrimEmpty() ;
		}
		else
		{
			m_aBeamBullet.RemoveAll() ;
		}
	}
}

// アイテムプロパティのリソース等の参照を更新する
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DBatteryItemSerializer::UpdatePropertyReference
	( S3DSceneComposer::Composition& comp, uint32_t nFlags )
{
	uint32_t	nResFlags =
		ItemBasicSerializer::UpdatePropertyReference( comp, nFlags ) ;
	//
	if ( nFlags & S3DSceneComposer::updateRefItem )
	{
		if ( !UpdateBulletReference() )
		{
			nResFlags |= S3DSceneComposer::updateRefItem ;
		}
	}
	return	nResFlags ;
}



//////////////////////////////////////////////////////////////////////////////
// 弾描画コントローラー（先端）
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DBulletDrawController, BulletController )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DBulletDrawController, bullet_drawer )

const SSystem::SXMLDocument::AttrInteger
	S3DBulletDrawController::m_aiRotateAxis
			[S3DBulletDrawController::rotateAxisCount+1] =
{
	{ L"x", S3DBulletDrawController::rotateOnX },
	{ L"y", S3DBulletDrawController::rotateOnY },
	{ L"z", S3DBulletDrawController::rotateOnZ },
	{ L"w", S3DBulletDrawController::rotateOnW },
	{ NULL, 0 },
} ;

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DBulletDrawController::S3DBulletDrawController( void )
	: BulletController( m_ItemClassDescriptor.pwszClassID ),
		m_vRotAngle( 0, 0, 1 )
{
	m_fpDrawPosition = 0.0 ;
	m_fpZoomScale = 1.0 ;
	m_degRotSpeed = 0.0 ;
	m_rotAxis = rotateOnW ;
	m_flagEnableRotation = false ;
	m_flagRotDirection = false ;
	//
	AddParameterEntry
		( L"render_target", S3DSceneComposer::typeCommand,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrStringEnumeration,
			L"描画ターゲット", L"描画ターゲットアイテムを指定します" ) ;
	AddParameterEntry
		( L"render_position", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant,
			L"描画位置", L"軌道上の描画位置を指定します" ) ;
	AddParameterEntry
		( L"render_zoom", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant,
			L"描画拡大率", L"拡大比率を指定します" ) ;
	AddParameterEntry
		( L"enable_rot", S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrConstant,
			L"回転有効", L"回転処理を有効にします" ) ;
	AddParameterEntry
		( L"rot_for_dir", S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrConstant,
			L"進行方向へ回転", NULL ) ;
	AddParameterEntry
		( L"rot_for_angle",
			S3DSceneComposer::typeDirection,
			S3DSceneComposer::attrConstant,
			L"回転基準方向", L"基準となる回転方向" ) ;
	AddParameterEntry
		( L"rot_axis",
			S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrStringEnumeration
			| S3DSceneComposer::attrUIOnlyEnumeration,
			L"回転軸", L"回転速度で動的に回転させる場合の回転軸" ) ;
	AddParameterEntry
		( L"rot_speed", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant,
			L"回転速度", L"回転速度 [deg/sec]" ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DBulletDrawController::~S3DBulletDrawController( void )
{
}

// 描画ターゲット設定
//////////////////////////////////////////////////////////////////////////////
void S3DBulletDrawController::AttachRenderTarget
	( S3DParticleSerializer::RenderTarget * pTarget, const wchar_t * pwszID )
{
	m_refDrawTarget.SetReference( pTarget ) ;
	m_strDrawTarget = pwszID ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
double S3DBulletDrawController::GetScalarParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramDrawPosition:
		return	m_fpDrawPosition ;
	case	paramDrawZoom:
		return	m_fpZoomScale ;
	case	paramRotateSpeed:
		return	m_degRotSpeed ;
	}
	return	0 ;
}

bool S3DBulletDrawController::GetBooleanParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramRotateEnable:
		return	m_flagEnableRotation ;
	case	paramRotateDir:
		return	m_flagRotDirection ;
	}
	return	false ;
}

const wchar_t * S3DBulletDrawController::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramDrawTarget:
		return	m_strDrawTarget ;
	case	paramRotateAxis:
		return	SXMLDocument::GetSymbolAsIntegerOf( m_aiRotateAxis, m_rotAxis ) ;
	}
	return	NULL ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DBulletDrawController::SetScalarParameter( size_t i, double s )
{
	switch ( i )
	{
	case	paramDrawPosition:
		m_fpDrawPosition = s ;
		return ;
	case	paramDrawZoom:
		m_fpZoomScale = s ;
		return ;
	case	paramRotateSpeed:
		m_degRotSpeed = s ;
		return ;
	}
}

void S3DBulletDrawController::SetBooleanParameter( size_t i, bool b )
{
	switch ( i )
	{
	case	paramRotateEnable:
		m_flagEnableRotation = b ;
		return ;
	case	paramRotateDir:
		m_flagRotDirection = b ;
		return ;
	}
}

void S3DBulletDrawController::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	switch ( i )
	{
	case	paramDrawTarget:
		if ( m_strDrawTarget != pwszCmd )
		{
			AttachRenderTarget
				( ESLTypeCast<S3DParticleSerializer::RenderTarget>
							( GetSceneItemAs( pwszCmd ) ), pwszCmd ) ;
		}
		break ;
	case	paramRotateAxis:
		m_rotAxis = (RotaionAxis) SXMLDocument::GetIntegerAsSymbolOf
								( m_aiRotateAxis, pwszCmd, m_rotAxis ) ;
		return ;
	}
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DBulletDrawController::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	S3DSceneComposer::Composition *	pComp ;
	int	j ;
	switch ( i )
	{
	case	paramDrawTarget:
		pComp = GetComposition() ;
		if ( pComp != NULL )
		{
			pComp->EnumerateItemIDsAs
				( aStrSet,
					ESL_RUNTIME_CLASS(S3DParticleSerializer::RenderTarget) ) ;
		}
		return	true ;
	case	paramRotateAxis:
		for ( j = 0; m_aiRotateAxis[j].pszSymbol != NULL; j ++ )
		{
			aStrSet.Add( new SString(m_aiRotateAxis[j].pszSymbol) ) ;
		}
		return	true ;
	}
	return	Controller::EnumerateStringSet( i, aStrSet ) ;
}

// アイテムプロパティのリソース等の参照を更新する
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DBulletDrawController::UpdatePropertyReference
	( S3DSceneComposer::Composition& comp,
		S3DSceneComposer::ItemSerializer * pItem, uint32_t nFlags )
{
	uint32_t	nResFlags =
		Controller::UpdatePropertyReference( comp, pItem, nFlags ) ;
	//
	if ( (nFlags & S3DSceneComposer::updateRefItem)
		&& !m_strDrawTarget.IsEmpty() )
	{
		S3DParticleSerializer::RenderTarget *	pTarget =
			ESLTypeCast<S3DParticleSerializer::RenderTarget>
							( GetSceneItemAs( m_strDrawTarget ) ) ;
		AttachRenderTarget( pTarget, m_strDrawTarget ) ;
		//
		if ( pTarget == NULL )
		{
			nResFlags |= S3DSceneComposer::updateRefItem ;
		}
	}
	return	nResFlags ;
}

// 発射時処理
//////////////////////////////////////////////////////////////////////////////
void S3DBulletDrawController::OnFireBullet
	( S3DBulletItemInterface * pItem,
		S3DBulletItemInterface::Bullet& bullet )
{
}

// 着弾時処理
//////////////////////////////////////////////////////////////////////////////
void S3DBulletDrawController::OnHitBullet
	( S3DBulletItemInterface * pItem,
		S3DSceneComposer::ItemSerializer * pHitItem,
		const S3DCollision::MeshCollision * pmcHitMesh,
		const S3DVector& vHitPos, const S3DVector& vHitNormal,
		S3DBulletItemInterface::Bullet& bullet, size_t iHitTrack )
{
}

// タイマー前処理
//////////////////////////////////////////////////////////////////////////////
void S3DBulletDrawController::BeforeBulletTimer
	( S3DBulletItemInterface * pItem, float32_t secPast,
			S3DBulletItemInterface::Bullet*const* pBullets, size_t nCount )
{
}

// タイマー処理
//////////////////////////////////////////////////////////////////////////////
void S3DBulletDrawController::OnBulletTimer
	( S3DBulletItemInterface * pItem, float32_t secPast,
		S3DBulletItemInterface::Bullet*const* pBullets, size_t nCount )
{
}

// 当たり判定追加処理
//////////////////////////////////////////////////////////////////////////////
void S3DBulletDrawController::RenderBulletCollision
	( const S3DScene& scene,
		S3DCollision& collision,
		const S3DBulletItemInterface * pItem,
		S3DBulletItemInterface::Bullet*const* pBullets, size_t nCount )
{
}

// 弾軌跡描画（追加処理）
//////////////////////////////////////////////////////////////////////////////
void S3DBulletDrawController::RenderBulletTracks
	( S3DScene& scene, const S3DBulletItemInterface * pItem,
		const S3DBulletItemInterface::Bullet*const* pBullets, size_t nCount )
{
	S3DParticleSerializer::RenderTarget *
					pRenderTarget = m_refDrawTarget.GetReference() ;
	if ( pRenderTarget == NULL )
	{
		return ;
	}
	const bool		flagIndexedParticle =
						pRenderTarget->IsUsingIndexedParticles() ;
	const size_t	nMaxTrackCount =
						flagIndexedParticle ? pItem->GetMaxTrackCount() : 0 ;
	S3DVector4 *	pvPoints = m_bufPoints.GetArray( nCount * (nMaxTrackCount + 1) ) ;
	size_t *		pFrames = m_bufFrames.GetArray( nCount ) ;
	S3DColor *		pColors = m_bufColors.GetArray( nCount ) ;
	float32_t *		pZooms = NULL ;
	S3DMatrix *		pMatrixs = NULL ;
	//
	S3DParticleSerializer::ParticleIndex *	pIndexes = NULL ;
	if ( flagIndexedParticle )
	{
		pIndexes = m_bufParticleIndex.GetArray( nCount ) ;
		pMatrixs = m_bufMatrixs.GetArray( nCount ) ;
	}
	else
	{
		pZooms = m_bufZooms.GetArray( nCount ) ;
	}
	//
	// 変換座標取得
	//
	S3DDMatrix	matdITargetSpace( 1, 1, 1 ) ;
	S3DDVector	vdITargetSpace( 0, 0, 0 ) ;
	pRenderTarget->GetTargetSpaceTransformation
						( matdITargetSpace, vdITargetSpace ) ;
	if ( pItem->GetBulletFlags() & S3DBulletItemInterface::flagTransform )
	{
		S3DDMatrix	matdItem( 1, 1, 1 ) ;
		S3DDVector	vItem( 0, 0, 0 ) ;
		pItem->GetGlobalTransformation( matdItem, vItem ) ;
		//
		vdITargetSpace += matdITargetSpace * vItem ;
		matdITargetSpace *= matdItem ;
	}
	//
	// アニメーション情報取得
	//
	double	secLength ;
	size_t	nTotalFrames = 1 ;
	if ( pRenderTarget->GetTargetAnimationLength( secLength ) )
	{
		nTotalFrames = pRenderTarget->GetTargetAnimationFrames() ;
		if ( nTotalFrames == 0 )
		{
			nTotalFrames = 1 ;
		}
	}
	//
	// カメラ回転取得
	//
	S3DDVector	vdFaceDir( 0, 0, -1 ) ;
	S3DMatrix	matRotate( m_fpZoomScale, m_fpZoomScale, m_fpZoomScale ) ;
	if ( flagIndexedParticle )
	{
		if ( !m_flagEnableRotation )
		{
			S3DScene::Camera *	pCamera = scene.GetCurrentCamera() ;
			if ( pCamera == NULL )
			{
				pCamera = scene.GetMainCamera() ;
			}
			if ( pCamera != NULL )
			{
				S3DDMatrix	matLinkCamera ;
				S3DDVector	vLinkCamera ;
				pCamera->CalcItemLinkTransformation
								( matLinkCamera, vLinkCamera ) ;
				//
				S3DDVector	vCamera =
						matLinkCamera * pCamera->GetCameraPosition() ;
				S3DDVector	vTarget =
						matLinkCamera * pCamera->GetCameraTarget() ;
				//
				S3DDVector	vViewDir = vTarget - vCamera ;
				vViewDir.Normalize() ;
				//
				vdFaceDir = vViewDir ;
			}
			matdITargetSpace.RevolveVector( vdFaceDir ) ;
			matRotate.RevolveForAngle( S3DVector(vdFaceDir) ) ;
		}
	}
	//
	size_t	iDst = 0 ;
	size_t	iPoint = 0 ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		const S3DBulletItemInterface::Bullet *	pBullet = pBullets[i] ;
		if ( (pBullet == NULL)
			|| (pBullet->nStateFlags
					& S3DBulletItemInterface::stateHitDestroy) )
		{
			continue ;
		}
		S3DVector	vPos ;
		if ( m_fpDrawPosition < 1.0e-5 )
		{
			vPos = matdITargetSpace * pBullet->vPos + vdITargetSpace ;
		}
		else
		{
			const int	iTrackPos = (int) m_fpDrawPosition ;
			double		fpDecimal = m_fpDrawPosition - iTrackPos ;
			S3DDVector	vdPos ;
			if ( (uint32_t) iTrackPos >= pBullet->nTrackCount )
			{
				vdPos = pBullet->vTrack[pBullet->nTrackCount] ;
			}
			else
			{
				vdPos = pBullet->vTrack[iTrackPos] * (1.0 - fpDecimal)
						+ pBullet->vTrack[iTrackPos + 1] * fpDecimal ;
			}
			vPos = matdITargetSpace * vdPos + vdITargetSpace ;
		}
		size_t		iFrame = 0 ;
		S3DColor	clrBullet( 0xFFFFFFFF, 0 ) ;
		//
		if ( (nTotalFrames > 1) && (secLength > 0.0) )
		{
			iFrame = (size_t) eslRoundR64ToLInt
							( pBullet->secLife / secLength * nTotalFrames)
															% nTotalFrames ;
		}
		clrBullet.rgbMul.argb.Alpha = (uint8_t) esl_min( pBullet->nAlpha, 0xFF ) ;
		//
		pvPoints[iPoint] = vPos ;
		pFrames[iDst] = iFrame ;
		pColors[iDst] = clrBullet ;
		//
		if ( flagIndexedParticle )
		{
			if ( m_flagEnableRotation )
			{
				S3DMatrix	matRot = matRotate ;
				if ( m_flagRotDirection )
				{
					matRot.RevolveForAngle( S3DVector(pBullet->vSpeed) ) ;
				}
				double	rad = m_degRotSpeed * pBullet->secLife * (PI / 180.0) ;
				switch ( m_rotAxis )
				{
				case	rotateOnX:
					matRot.RevolveOnX( sin(rad), cos(rad) ) ;
					break ;
				case	rotateOnY:
					matRot.RevolveOnY( sin(rad), cos(rad) ) ;
					break ;
				case	rotateOnZ:
					matRot.RevolveOnZ( sin(rad), cos(rad) ) ;
					break ;
				default:
					break ;
				}
				matRot.RevolveForAngle( S3DVector(m_vRotAngle) ) ;
				pMatrixs[iDst] = matRot ;
			}
			else
			{
				pMatrixs[iDst] = matRotate ;
			}
			pMatrixs[iDst] *= pBullet->fpThickness ;
			//
			S3DParticleSerializer::ParticleIndex&	pi = pIndexes[iDst] ;
			const int	iTrackPos = (int) m_fpDrawPosition ;
			pi.nIndex = iPoint ;
			if ( (uint32_t) iTrackPos >= pBullet->nTrackCount )
			{
				pi.nBlurCount = 1 ;
			}
			else
			{
				pi.nBlurCount = pBullet->nTrackCount + 1 - iTrackPos ;
			}
			pi.nIdentity = 0 ;
			//
			for ( size_t j = iTrackPos + 1; j <= pBullet->nTrackCount; j ++ )
			{
				pvPoints[iPoint + j] = pBullet->vTrack[j] ;
			}
			iPoint += pi.nBlurCount ;
		}
		else
		{
			pZooms[iDst] = (float32_t) m_fpZoomScale * pBullet->fpThickness ;
			iPoint ++ ;
		}
		iDst ++ ;
	}
	ESLAssert( iPoint <= nCount * (nMaxTrackCount + 1) ) ;
	ESLAssert( iDst <= nCount ) ;
	if ( iDst > 0 )
	{
		if ( flagIndexedParticle )
		{
			pRenderTarget->AddIndexedParticles
				( iDst, pvPoints, pIndexes, pFrames, pColors, pMatrixs ) ;
		}
		else
		{
			pRenderTarget->AddParticles
				( iDst, pvPoints, pFrames, pColors, pZooms ) ;
		}
	}
	m_bufPoints.FinishArray() ;
	m_bufFrames.FinishArray() ;
	m_bufColors.FinishArray() ;
	m_bufParticleIndex.FinishArray() ;
	m_bufMatrixs.FinishArray() ;
	m_bufZooms.FinishArray() ;
}



//////////////////////////////////////////////////////////////////////////////
// 弾描画コントローラー（軌跡）
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DBulletRenderController, BulletController )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DBulletRenderController, beam_renderer )
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DBulletRenderController::ThunderInstance, ESLObject )

const wchar_t *	S3DBulletRenderController::m_pwszShapeType
					[S3DBulletRenderController::shapeCount] =
{
	L"band", L"tube",
} ;

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DBulletRenderController::S3DBulletRenderController( void )
	: BulletController( m_ItemClassDescriptor.pwszClassID )
{
	m_shape = shapeBand ;
	m_fpUScale = 1.0 ;
	m_fpVScale = 1.0 ;
	m_fpVSpeed = 0.0 ;
	m_fpZBais = 0.0 ;
	m_fpTopAlpha = 1.0 ;
	m_fpTailAlpha = 0.0 ;
	m_fpThickness[0] = 0.0 ;
	m_iThickIndex[0] = 0 ;
	m_fpThickness[1] = 1.0 ;
	m_iThickIndex[1] = 1 ;
	m_fpThickness[2] = 0.8 ;
	m_iThickIndex[2] = 4 ;
	m_fpThickness[3] = 0.0 ;
	m_iThickIndex[3] = 10 ;
	//
	m_iThunderOffset = 0 ;
	m_nThunderCount = 0 ;
	m_fpThunderJointEffect = 0.5 ;
	m_fpThunderJointLen = 0.5 ;
	m_fpThunderWaveEffect = 1.0 ;
	m_fpThunderWaveLen = 5.0 ;
	//
	m_flagTubeHeadCap = true ;
	m_flagTubeTailCap = true ;
	m_nTubeHDivision = 16 ;
	m_nTubeVDivision = 4 ;
	m_colorBase = S3DColor( 0xFFFFFFFF, 0x00808080 ) ;
	//
	m_random.InitializeSeed() ;
	//
	m_aColorDiv.Add( S3DColor( 0xFFFFFFFF, 0x00FFFFFF ) ) ;
	m_aThickDiv.Add( 0.0f ) ;
	m_aColorDiv.Add( S3DColor( 0xFFFFFFFF, 0x0000FFFF ) ) ;
	m_aThickDiv.Add( 0.25f ) ;
	m_aColorDiv.Add( S3DColor( 0xFFFFFFFF, 0x000000FF ) ) ;
	m_aThickDiv.Add( 0.5f ) ;
	m_aColorDiv.Add( S3DColor( 0x00FFFFFF, 0x00000000 ) ) ;
	m_aThickDiv.Add( 1.0f ) ;
	//
	PrepareParameterEntryCount
		( paramColorEntryFirst + paramColorElementCount * 4 ) ;
	AddParameterEntry
		( L"render_target", S3DSceneComposer::typeCommand,
			S3DSceneComposer::attrStringEnumeration,
			L"描画ターゲット", L"描画ターゲットメッシュを指定します" ) ;
	AddParameterEntry
		( L"shape_type", S3DSceneComposer::typeCommand,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrStringEnumeration
			| S3DSceneComposer::attrDynamicValidation,
			L"形状種別", L"描画形状を指定します" ) ;
	AddParameterEntry
		( L"u_scale", S3DSceneComposer::typeScalar, 0,
			L"Ｕスケール", L"テクスチャマップｘ座標スケール（ビームを1.0とする）" ) ;
	AddParameterEntry
		( L"v_scale", S3DSceneComposer::typeScalar, 0,
			L"Ｖスケール", L"テクスチャマップｙ座標スケール（空間距離比）" ) ;
	AddParameterEntry
		( L"v_speed", S3DSceneComposer::typeScalar, 0,
			L"Ｖスクロール速度", L"テクスチャマップｙ座標スクロール速度" ) ;
	AddParameterEntry
		( L"z_bias", S3DSceneComposer::typeScalar, 0,
			L"ｚバイアス" ) ;
	AddParameterEntry
		( L"top_cap", S3DSceneComposer::typeBoolean, 0,
			L"先端半球", L"先端に半球を付加する" ) ;
	AddParameterEntry
		( L"tail_cap", S3DSceneComposer::typeBoolean, 0,
			L"末端半球", L"末端に半球を付加する" ) ;
	AddParameterEntry
		( L"h_division", S3DSceneComposer::typeInteger, 0,
			L"周囲分割数", L"円柱水平方向の分割数" ) ;
	AddParameterEntry
		( L"cap_division", S3DSceneComposer::typeInteger, 0,
			L"半球分割数", L"半球の垂直方向の分割数" ) ;
	AddParameterEntry
		( L"top_alpha", S3DSceneComposer::typeScalar, 0,
			L"先端不透明度", L"ビーム先端の不透明度" ) ;
	AddParameterEntry
		( L"tail_alpha", S3DSceneComposer::typeScalar, 0,
			L"末端不透明度", L"ビーム末端の不透明度" ) ;
	AddParameterEntry
		( L"top_thickness", S3DSceneComposer::typeScalar, 0,
			L"ビーム先端太さ", L"ビーム先端の太さを指定します" ) ;
	AddParameterEntry
		( L"head_thickness", S3DSceneComposer::typeScalar, 0,
			L"ビーム頭太さ", L"ビームの頭位置の太さを指定します" ) ;
	AddParameterEntry
		( L"head_index", S3DSceneComposer::typeInteger, 0,
			L"ビーム頭位置", L"ビームの頭位置のインデックス（軌跡頂点）を指定します" ) ;
	AddParameterEntry
		( L"tail_thickness", S3DSceneComposer::typeScalar, 0,
			L"テイル太さ", L"ビームテイルの付け根位置の太さを指定します" ) ;
	AddParameterEntry
		( L"tail_index", S3DSceneComposer::typeInteger, 0,
			L"テイル位置", L"ビームテイルの開始位置のインデックス（軌跡頂点）を指定します" ) ;
	AddParameterEntry
		( L"end_thickness", S3DSceneComposer::typeScalar, 0,
			L"末端太さ", L"ビームテイルの末端位置の太さを指定します" ) ;
	AddParameterEntry
		( L"end_index", S3DSceneComposer::typeInteger, 0,
			L"ビーム最大長", L"ビームの最大長（軌跡頂点数）を指定します" ) ;
	AddParameterEntry
		( L"thunder_count", S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrCategory1,
			L"雷本数", L"稲妻効果で描画するビーム本数を指定します" ) ;
	AddParameterEntry
		( L"thunder_joint_amp", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrCategory1,
			L"雷ブレ幅", L"稲妻効果によるジグザグ振幅を指定します" ) ;
	AddParameterEntry
		( L"thunder_joint", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrCategory1,
			L"雷節長", L"最小のジグザグの節分割長を指定します" ) ;
	AddParameterEntry
		( L"thunder_wave_amp", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrCategory1,
			L"雷振幅", L"稲妻効果による振幅を指定します" ) ;
	AddParameterEntry
		( L"thunder_wave_len", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrCategory1,
			L"雷波長", L"より長いジグザグの効果の波長を指定します" ) ;
	AddParameterEntry
		( L"base_color_alpha", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant2
			| S3DSceneComposer::attrUIScalarSlider,
			L"不透明度", NULL, 0.0, 1.0 ) ;
	AddParameterEntry
		( L"base_color_mul", S3DSceneComposer::typeColor,
			S3DSceneComposer::attrConstant2,
			L"色乗算", NULL ) ;
	AddParameterEntry
		( L"base_color_add", S3DSceneComposer::typeColor,
			S3DSceneComposer::attrConstant2,
			L"色加算", NULL ) ;
	AddParameterEntry
		( L"color_division", S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrDynamicValidation
			| S3DSceneComposer::attrConstant2,
			L"色指定分割数", NULL ) ;
	//
	SetColorDivCount( 4 ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DBulletRenderController::~S3DBulletRenderController( void )
{
}

// 色分割数設定
//////////////////////////////////////////////////////////////////////////////
void S3DBulletRenderController::SetColorDivCount( size_t nCount )
{
	ChopParameterEntryLastAt( paramColorDivision ) ;
	//
	m_aColorDiv.SetLength( nCount ) ;
	m_aThickDiv.SetLength( nCount ) ;
	//
	SString	strID, strName ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		SString *	pstrID =
			GetColorDivPropID
				( i * paramColorElementCount, L"color_level%d", i ) ;
		SString *	pstrName =
			GetColorDivPropName
				( i * paramColorElementCount, L"分割位置[%d]", i ) ;
		AddParameterEntry
			( *pstrID, S3DSceneComposer::typeScalar,
				S3DSceneComposer::attrConstant2
				| S3DSceneComposer::attrUIScalarSlider,
				*pstrName, NULL, 0.0, 1.0 ) ;
		//
		pstrID = GetColorDivPropID
				( i * paramColorElementCount + 1, L"color_alpha%d", i ) ;
		pstrName = GetColorDivPropName
				( i * paramColorElementCount + 1, L"不透明度[%d]", i ) ;
		AddParameterEntry
			( *pstrID, S3DSceneComposer::typeScalar,
				S3DSceneComposer::attrConstant2
				| S3DSceneComposer::attrUIScalarSlider,
				*pstrName, NULL, 0.0, 1.0 ) ;
		//
		pstrID = GetColorDivPropID
				( i * paramColorElementCount + 2, L"color_mul%d", i ) ;
		pstrName = GetColorDivPropName
				( i * paramColorElementCount + 2, L"色乗算[%d]", i ) ;
		AddParameterEntry
			( *pstrID, S3DSceneComposer::typeColor,
				S3DSceneComposer::attrConstant2,
				*pstrName, NULL ) ;
		//
		pstrID = GetColorDivPropID
				( i * paramColorElementCount + 3, L"color_add%d", i ) ;
		pstrName = GetColorDivPropName
				( i * paramColorElementCount + 3, L"色加算[%d]", i ) ;
		AddParameterEntry
			( *pstrID, S3DSceneComposer::typeColor,
				S3DSceneComposer::attrConstant2,
				*pstrName, NULL ) ;
	}
}

SSystem::SString *
	S3DBulletRenderController::GetColorDivPropID
		( size_t iProp, const wchar_t * pwszFormat, size_t i )
{
	SString *	pstrID = m_aColorDivIDs.GetAt( iProp ) ;
	if ( pstrID == NULL )
	{
		pstrID = new SString ;
		m_aColorDivIDs.SetAt( iProp, pstrID ) ;
		pstrID->Format( pwszFormat, i ) ;
	}
	return	pstrID ;
}

SSystem::SString *
	S3DBulletRenderController::GetColorDivPropName
		( size_t iProp, const wchar_t * pwszFormat, size_t i )
{
	SString *	pstrName = m_aColorDivNames.GetAt( iProp ) ;
	if ( pstrName == NULL )
	{
		pstrName = new SString ;
		m_aColorDivNames.SetAt( iProp, pstrName ) ;
		pstrName->Format( pwszFormat, i ) ;
	}
	return	pstrName ;
}

// 描画ターゲット設定
//////////////////////////////////////////////////////////////////////////////
void S3DBulletRenderController::AttachRenderTarget
	( S3DMeshBufferItemSerializer * pTarget, const wchar_t * pwszID )
{
	m_refRenderTarget.SetReference
				( (S3DSceneComposer::ItemSerializer*) pTarget ) ;
	m_strRenderTarget = pwszID ;
}

bool S3DBulletRenderController::UpdateRenderTarget( void )
{
	if ( m_strRenderTarget.IsEmpty() )
	{
		m_refRenderTarget.SetReference( NULL ) ;
		return	true ;
	}
	S3DSceneComposer::Composition *	pComp = GetComposition() ;
	if ( pComp != NULL )
	{
		S3DSceneComposer::ItemSerializer *
			pItem = pComp->GetSceneItemAs( m_strRenderTarget ) ;
		m_refRenderTarget.SetReference( pItem ) ;
		//
		return	(pItem != NULL) ;
	}
	return	false ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDVector S3DBulletRenderController::GetVectorParameter( size_t i ) const
{
	size_t	iColor = 0 ;
	if ( i >= paramColorEntryFirst )
	{
		iColor = (i - paramColorEntryFirst) / paramColorElementCount ;
		i -= iColor * paramColorElementCount ;
		if ( iColor >= m_aColorDiv.GetLength() )
		{
			return	S3DDVector(0,0,0) ;
		}
	}
	switch ( i )
	{
	case	paramColorMul:
		return	VectorFromColor( m_colorBase.rgbMul ) ;
	case	paramColorAdd:
		return	VectorFromColor( m_colorBase.rgbAdd ) ;
	case	paramColorMul0:
		return	VectorFromColor( m_aColorDiv.At(iColor).rgbMul ) ;
	case	paramColorAdd0:
		return	VectorFromColor( m_aColorDiv.At(iColor).rgbAdd ) ;
	}
	return	S3DDVector(0,0,0) ;
}

double S3DBulletRenderController::GetScalarParameter( size_t i ) const
{
	size_t	iColor = 0 ;
	if ( i >= paramColorEntryFirst )
	{
		iColor = (i - paramColorEntryFirst) / paramColorElementCount ;
		i -= iColor * paramColorElementCount ;
		if ( iColor >= m_aColorDiv.GetLength() )
		{
			return	0.0 ;
		}
	}
	switch ( i )
	{
	case	paramUScale:
		return	m_fpUScale ;
	case	paramVScale:
		return	m_fpVScale ;
	case	paramVSpeed:
		return	m_fpVSpeed ;
	case	paramZBais:
		return	m_fpZBais ;
	case	paramTopAlpha:
		return	m_fpTopAlpha ;
	case	paramTailAlpha:
		return	m_fpTailAlpha ;
	case	paramThickness0:
		return	m_fpThickness[0] ;
	case	paramThickness1:
		return	m_fpThickness[1] ;
	case	paramThickness2:
		return	m_fpThickness[2] ;
	case	paramThickness3:
		return	m_fpThickness[3] ;
	case	paramThunderJointEffect:
		return	m_fpThunderJointEffect ;
	case	paramThunderJointLen:
		return	m_fpThunderJointLen ;
	case	paramThunderWaveEffect:
		return	m_fpThunderWaveEffect ;
	case	paramThunderWaveLen:
		return	m_fpThunderWaveLen ;
	case	paramColorAlpha:
		return	m_colorBase.rgbMul.argb.Alpha / 255.0 ;
	case	paramColorLevel0:
		return	m_aThickDiv.At( iColor ) ;
	case	paramColorAlpha0:
		return	m_aColorDiv.At( iColor ).rgbMul.argb.Alpha / 255.0 ;
	}
	return	0.0 ;
}

int32_t S3DBulletRenderController::GetIntegerParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramHorzDivision:
		return	(int32_t) m_nTubeHDivision ;
	case	paramCapDivision:
		return	(int32_t) m_nTubeVDivision ;
	case	paramThickIndex1:
		return	(int32_t) m_iThickIndex[1] ;
	case	paramThickIndex2:
		return	(int32_t) m_iThickIndex[2] ;
	case	paramThickIndex3:
		return	(int32_t) m_iThickIndex[3] ;
	case	paramThunderCount:
		return	(int32_t) m_nThunderCount ;
	case	paramColorDivision:
		return	(int32_t) m_aColorDiv.GetLength() ;
	}
	return	0 ;
}

bool S3DBulletRenderController::GetBooleanParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramTopCap:
		return	m_flagTubeHeadCap ;
	case	paramTailCap:
		return	m_flagTubeTailCap ;
	}
	return	false ;
}

const wchar_t * S3DBulletRenderController::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramRenderTarget:
		return	m_strRenderTarget ;
	case	paramShapeType:
		return	m_pwszShapeType[m_shape] ;
	}
	return	NULL ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DBulletRenderController::SetVectorParameter( size_t i, const S3DDVector& vec )
{
	size_t	iColor = 0 ;
	if ( i >= paramColorEntryFirst )
	{
		iColor = (i - paramColorEntryFirst) / paramColorElementCount ;
		i -= iColor * paramColorElementCount ;
		if ( iColor >= m_aColorDiv.GetLength() )
		{
			return ;
		}
	}
	SGLPalette	rgba ;
	switch ( i )
	{
	case	paramColorMul:
		rgba = ColorFromVector( vec ) ;
		rgba.argb.Alpha = m_colorBase.rgbMul.argb.Alpha ;
		m_colorBase.rgbMul = rgba ;
		return ;
	case	paramColorAdd:
		m_colorBase.rgbAdd = ColorFromVector( vec ) ;
		return ;
	case	paramColorMul0:
		rgba = ColorFromVector( vec ) ;
		rgba.argb.Alpha = m_aColorDiv.At(iColor).rgbMul.argb.Alpha ;
		m_aColorDiv.At(iColor).rgbMul = rgba ;
		return ;
	case	paramColorAdd0:
		m_aColorDiv.At(iColor).rgbAdd = ColorFromVector( vec ) ;
		return ;
	}
}

void S3DBulletRenderController::SetScalarParameter( size_t i, double s )
{
	size_t	iColor = 0 ;
	if ( i >= paramColorEntryFirst )
	{
		iColor = (i - paramColorEntryFirst) / paramColorElementCount ;
		i -= iColor * paramColorElementCount ;
		if ( iColor >= m_aColorDiv.GetLength() )
		{
			return ;
		}
	}
	switch ( i )
	{
	case	paramUScale:
		m_fpUScale = s ;
		return ;
	case	paramVScale:
		m_fpVScale = s ;
		return ;
	case	paramVSpeed:
		m_fpVSpeed = s ;
		return ;
	case	paramZBais:
		m_fpZBais = s ;
		return ;
	case	paramTopAlpha:
		m_fpTopAlpha = esl_fclamp( s, 0.0, 1.0 ) ;
		return ;
	case	paramTailAlpha:
		m_fpTailAlpha = esl_fclamp( s, 0.0, 1.0 ) ;
		return ;
	case	paramThickness0:
		m_fpThickness[0] = s ;
		return ;
	case	paramThickness1:
		m_fpThickness[1] = s ;
		return ;
	case	paramThickness2:
		m_fpThickness[2] = s ;
		return ;
	case	paramThickness3:
		m_fpThickness[3] = s ;
		return ;
	case	paramThunderJointEffect:
		m_fpThunderJointEffect = s ;
		return ;
	case	paramThunderJointLen:
		m_fpThunderJointLen = s ;
		return ;
	case	paramThunderWaveEffect:
		m_fpThunderWaveEffect = s ;
		return ;
	case	paramThunderWaveLen:
		m_fpThunderWaveLen = s ;
		return ;
	case	paramColorAlpha:
		m_colorBase.rgbMul.argb.Alpha =
			(uint8_t) esl_clampi( (int) eslRoundR64ToLInt( s * 255.0 ), 0, 255 ) ;
		return ;
	case	paramColorLevel0:
		m_aThickDiv.SetAt( iColor, (float32_t) s ) ;
		return ;
	case	paramColorAlpha0:
		m_aColorDiv.At( iColor ).rgbMul.argb.Alpha =
			(uint8_t) esl_clampi( (int) eslRoundR64ToLInt( s * 255.0 ), 0, 255 ) ;
		return ;
	}
	return ;
}

void S3DBulletRenderController::SetIntegerParameter( size_t i, int32_t n )
{
	switch ( i )
	{
	case	paramHorzDivision:
		m_nTubeHDivision = (size_t) n ;
		return ;
	case	paramCapDivision:
		m_nTubeVDivision = (size_t) n ;
		return ;
	case	paramThickIndex1:
		m_iThickIndex[1] = (size_t) n ;
		return ;
	case	paramThickIndex2:
		m_iThickIndex[2] = (size_t) n ;
		return ;
	case	paramThickIndex3:
		m_iThickIndex[3] = (size_t) n ;
		return ;
	case	paramThunderCount:
		m_nThunderCount = (size_t) n ;
		return ;
	case	paramColorDivision:
		if ( m_aColorDiv.GetLength() != (size_t) n )
		{
			SetColorDivCount( (size_t) n ) ;
		}
		return ;
	}
}

void S3DBulletRenderController::SetBooleanParameter( size_t i, bool b )
{
	switch ( i )
	{
	case	paramTopCap:
		m_flagTubeHeadCap = b ;
		return ;
	case	paramTailCap:
		m_flagTubeTailCap = b ;
		return ;
	}
}

void S3DBulletRenderController::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	int	j ;
	switch ( i )
	{
	case	paramRenderTarget:
		if ( m_strRenderTarget != pwszCmd )
		{
			m_strRenderTarget = pwszCmd ;
			UpdateRenderTarget() ;
		}
		return ;
	case	paramShapeType:
		for ( j = 0; j < shapeCount; j ++ )
		{
			if ( SString::Compare( m_pwszShapeType[j], pwszCmd ) == 0 )
			{
				m_shape = (ShapeTypeIndex) j ;
				break ;
			}
		}
		return ;
	}
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DBulletRenderController::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	switch ( i )
	{
	case	paramRenderTarget:
		{
			S3DSceneComposer::Composition *	pComp = GetComposition() ;
			if ( pComp != NULL )
			{
				pComp->EnumerateItemIDsAs
					( aStrSet, ESL_RUNTIME_CLASS(S3DMeshBufferItemSerializer) ) ;
			}
		}
		return	true ;

	case	paramShapeType:
		{
			for ( int j = 0; j < shapeCount; j ++ )
			{
				aStrSet.Add( new SString( m_pwszShapeType[j] ) ) ;
			}
		}
		return	true ;
	}
	return	false ;
}

// パラメーター有効性
//////////////////////////////////////////////////////////////////////////////
bool S3DBulletRenderController::IsParameterValidation( size_t i ) const
{
	if ( m_shape == shapeTube )
	{
		if ( i >= paramColorDivision )
		{
			return	false ;
		}
		switch ( i )
		{
		case	paramZBais:
		case	paramCapDivision:
			return	false ;
		}
	}
	else if ( m_shape == shapeBand )
	{
		switch ( i )
		{
		case	paramHorzDivision:
		case	paramCapDivision:
		case	paramColorAlpha:
		case	paramColorMul:
		case	paramColorAdd:
			return	false ;
		}
	}
	return	true ;
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DBulletRenderController::GetParameterCategoryName( size_t iCategory ) const
{
	switch ( iCategory )
	{
	case	0:
		return	L"形状設定" ;
	case	1:
		return	L"雷設定" ;
	case	2:
		return	L"ビーム色設定" ;
	}
	return	NULL ;
}

// アイテムプロパティのリソース等の参照を更新する
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DBulletRenderController::UpdatePropertyReference
	( S3DSceneComposer::Composition& comp,
		S3DSceneComposer::ItemSerializer * pItem, uint32_t nFlags )
{
	uint32_t	nResFlags =
		Controller::UpdatePropertyReference( comp, pItem, nFlags ) ;
	//
	if ( nFlags & S3DSceneComposer::updateRefItem )
	{
		if ( !UpdateRenderTarget() )
		{
			nResFlags |= S3DSceneComposer::updateRefItem ;
		}
	}
	return	nResFlags ;
}

// 発射時処理
//////////////////////////////////////////////////////////////////////////////
void S3DBulletRenderController::OnFireBullet
	( S3DBulletItemInterface * pItem,
		S3DBulletItemInterface::Bullet& bullet )
{
}

// 着弾時処理
//////////////////////////////////////////////////////////////////////////////
void S3DBulletRenderController::OnHitBullet
	( S3DBulletItemInterface * pItem,
		S3DSceneComposer::ItemSerializer * pHitItem,
		const S3DCollision::MeshCollision * pmcHitMesh,
		const S3DVector& vHitPos, const S3DVector& vHitNormal,
		S3DBulletItemInterface::Bullet& bullet, size_t iHitTrack )
{
}

// タイマー前処理
//////////////////////////////////////////////////////////////////////////////
void S3DBulletRenderController::BeforeBulletTimer
	( S3DBulletItemInterface * pItem, float32_t secPast,
		S3DBulletItemInterface::Bullet*const* pBullets, size_t nCount )
{
	if ( m_nThunderCount > 0 )
	{
		m_iThunderOffset = 0 ;
		for ( size_t i = 0; i < pItem->GetBulletListenerCount(); i ++ )
		{
			S3DBulletRenderController *	pRenderCtrl =
						ESLTypeCast<S3DBulletRenderController>
									( pItem->GetBulletListenerAt( i ) ) ;
			if ( pRenderCtrl == this )
			{
				break ;
			}
			if ( pRenderCtrl == nullptr )
			{
				continue ;
			}
			m_iThunderOffset += pRenderCtrl->m_nThunderCount ;
		}
	}
}

// タイマー処理
//////////////////////////////////////////////////////////////////////////////
void S3DBulletRenderController::OnBulletTimer
	( S3DBulletItemInterface * pItem, float32_t secPast,
		S3DBulletItemInterface::Bullet*const* pBullets, size_t nCount )
{
	if ( m_nThunderCount > 0 )
	{
		for ( size_t i = 0; i < nCount; i ++ )
		{
			if ( pBullets[i] == nullptr )
			{
				continue ;
			}
			S3DBulletItemInterface::Bullet&	bullet = *(pBullets[i]) ;
			S3DBulletItemInterface::MoveBulletResult
				mbr = pItem->MoveBulletDefault( bullet, secPast ) ;
			if ( !(bullet.nStateFlags & S3DBulletItemInterface::stateControlMoved) )
			{
				continue ;
			}
			//
			ThunderInstance *	pInstance =
				ESLTypeCast<ThunderInstance>
					( S3DBulletItemInterface::GetBulletInstance
						( bullet, ESL_RUNTIME_CLASS(ThunderInstance) ) ) ;
			if ( pInstance == nullptr )
			{
				pInstance = new ThunderInstance ;
				S3DBulletItemInterface::AddBulletInstance( bullet, pInstance ) ;
			}
			//
			S3DMeshShaper::ThunderParam	param ;
			param.fpJointEffect = m_fpThunderJointEffect* bullet.fpThickness ;
			param.fpJointLen = m_fpThunderJointLen * bullet.fpThickness ;
			param.fpWaveEffect = m_fpThunderWaveEffect * bullet.fpThickness ;
			param.fpWaveLen = m_fpThunderWaveLen * bullet.fpThickness ;
			//
			for ( size_t j = 0; j < m_nThunderCount; j ++ )
			{
				S3DMeshShaper::ThunderContext *
						ptc = pInstance->GetAt( m_iThunderOffset + j ) ;
				if ( ptc == nullptr )
				{
					ptc = new S3DMeshShaper::ThunderContext ;
					pInstance->SetAt( m_iThunderOffset + j, ptc ) ;
				}
				if ( bullet.nStateFlags & S3DBulletItemInterface::stateMovedTrack )
				{
					if ( bullet.nTrackCount >= 1 )
					{
						ptc->InitContext( m_random, bullet.vTrack[0], param ) ;
						for ( size_t i = 1; i <= bullet.nTrackCount; i ++ )
						{
							ptc->NextPoint( m_random, bullet.vTrack[i], param ) ;
						}
					}
				}
				else
				{
					if ( bullet.nStateFlags
							& S3DBulletItemInterface::stateMovedTrackTail )
					{
						ptc->DecreaseIndex( 1 ) ;
					}
					if ( ptc->GetNextIndex() == 0 )
					{
						ptc->InitContext
							( m_random, bullet.vTrack[bullet.nTrackCount], param ) ;
						if ( bullet.nTrackCount >= 1 )
						{
							ptc->NextPoint( m_random, bullet.vPos, param ) ;
						}
					}
					else
					{
						ptc->NextPoint( m_random, bullet.vPos, param ) ;
					}
				}
			}
		}
	}
}

// 当たり判定追加処理
//////////////////////////////////////////////////////////////////////////////
void S3DBulletRenderController::RenderBulletCollision
	( const S3DScene& scene,
		S3DCollision& collision,
		const S3DBulletItemInterface * pItem,
		S3DBulletItemInterface::Bullet*const* pBullets, size_t nCount )
{
}

// 弾軌跡描画（追加処理）
//////////////////////////////////////////////////////////////////////////////
void S3DBulletRenderController::RenderBulletTracks
	( S3DScene& scene, const S3DBulletItemInterface * pItem,
		const S3DBulletItemInterface::Bullet*const* pBullets, size_t nCount )
{
	if ( nCount == 0 )
	{
		return ;
	}
	S3DMeshBufferItemSerializer *	pMeshBuf =
		ESLTypeCast<S3DMeshBufferItemSerializer>
				( m_refRenderTarget.GetReference() ) ;
	if ( pMeshBuf == NULL )
	{
		return ;
	}
	SSmartLock<const SCriticalSection>	lock( pMeshBuf->GetInstanceLocker() ) ;
	S3DVertexBufferInterface *	pvb = pMeshBuf->GetVertexBuffer() ;
	if ( pvb == NULL )
	{
		return ;
	}
	S3DDMatrix	matI( 1, 1, 1 ) ;
	S3DDVector	vZero( 0, 0, 0 ) ;
	S3DColor	clrEffect( 0xFFFFFFFF, 0 ) ;
	pvb->PushTransformation() ;
	pvb->SetMatrixTransformation( matI, vZero, &clrEffect, 0 ) ;
	//
	if ( pItem->GetBulletFlags() & S3DBulletItemInterface::flagTransform )
	{
		pMeshBuf->GetTransformationFrom
			( m_matBulletToVB, m_vBulletToVB, GetOwnerItem() ) ;
	}
	else
	{
		pMeshBuf->GetTransformationFrom
			( m_matBulletToVB, m_vBulletToVB, nullptr ) ;
	}
	//
	m_vCameraPos = scene.GetCurrentCameraPosition() ;
	m_matICamera = scene.GetCurrentCameraIMatrix() ;
	m_vCameraRay = m_matICamera * S3DVector( 0, 0, 1 ) ;
	m_vCameraRay.Normalize() ;
	//
	if ( m_shape == shapeBand )
	{
		m_tlpBeam.nFlags = 0 ;
		m_tlpBeam.uWidth = (float32_t) m_fpUScale ;
		m_tlpBeam.vRatio = (float32_t) m_fpVScale ;
		m_tlpBeam.vOffset = 0.0f ;
		m_tlpBeam.zBais = (float32_t) m_fpZBais ;
		m_tlpBeam.pColors = m_aColorDiv.GetConstArray() ;
		m_tlpBeam.nColorDiv = m_aColorDiv.GetLength() ;
		m_tlpBeam.pThickDiv = m_aThickDiv.GetConstArray() ;
	}
	else
	{
		m_tubeBeam.nFlags = 0 ;
		if ( m_flagTubeHeadCap )
		{
			m_tubeBeam.nFlags |= S3DMeshShaper::tubeCapHead ;
		}
		if ( m_flagTubeTailCap )
		{
			m_tubeBeam.nFlags |= S3DMeshShaper::tubeCapTail ;
		}
		m_tubeBeam.fpRadius = 1.0f ;
		m_tubeBeam.hDivision = m_nTubeHDivision ;
		m_tubeBeam.vDivision = m_nTubeVDivision ;
		m_tubeBeam.uvScale.x = (float32_t) m_fpUScale ;
		m_tubeBeam.uvScale.y = (float32_t) m_fpVScale ;
		m_tubeBeam.uvOffset.x = 0.0f ;
		m_tubeBeam.uvOffset.y = 0.0f ;
		m_tubeBeam.vTerminal =
			(float32_t) (m_fpVScale
						* esl_fmax( m_fpThickness[0], m_fpThickness[3] )) ;
		m_tubeBeam.colorBase = m_colorBase ;
	}
	//
	for ( size_t i = 0; i < nCount; i ++ )
	{
		if ( pBullets[i] != NULL )
		{
			RenderBulletTrack( *pvb, pItem, *(pBullets[i]) ) ;
		}
	}
	pvb->PopTransformation() ;
}

void S3DBulletRenderController::RenderBulletTrack
	( S3DVertexBufferInterface& vb,
		const S3DBulletItemInterface * pItem,
		const S3DBulletItemInterface::Bullet& bullet )
{
	if ( bullet.nTrackCount == 0 )
	{
		return ;
	}
	const uint32_t	nAlpha = esl_min( bullet.nAlpha + 1, 0x100 ) ;
	const size_t	nTrackCount =
		(size_t) esl_min( (int) bullet.nTrackCount + 1, (int) m_iThickIndex[3] ) ;
	if ( m_nThunderCount > 0 )
	{
		ThunderInstance *	pInstance =
			ESLTypeCast<ThunderInstance>
				( S3DBulletItemInterface::GetBulletInstance
					( bullet, ESL_RUNTIME_CLASS(ThunderInstance) ) ) ;
		if ( pInstance != nullptr )
		{
			S3DMatrix	matBulletToVB = m_matBulletToVB ;
			S3DVector	vBulletToVB = m_vBulletToVB ;
			for ( size_t iInstance = 0; iInstance < m_nThunderCount; iInstance ++ )
			{
				S3DMeshShaper::ThunderContext *
						ptc = pInstance->GetAt( m_iThunderOffset + iInstance ) ;
				if ( ptc == nullptr )
				{
					continue ;
				}
				const size_t		nCount = ptc->GetPointCount() ;
				const double		fpNextIndex = (double) ptc->GetNextIndex() ;
				const S3DVector4 *	pvPoints = ptc->GetPointArray() ;
				const double *		pIndices = ptc->GetIndexArray() ;
				float32_t *			pThickness = m_aThickBuf.GetArray( nCount ) ;
				uint32_t *			pAlpha = m_aAlphaBuf.GetArray( nCount ) ;
				for ( size_t i = 0; i < nCount; i ++ )
				{
					CalcThicknessAndAlpha
						( pThickness[i], pAlpha[i],
							fpNextIndex - pIndices[i], nAlpha ) ;
					pThickness[i] *= bullet.fpThickness ;
				}
				if ( nCount >= 2 )
				{
					S3DVector4 *	pvDstPoints = m_aPointBuf.GetArray( nCount ) ;
					matBulletToVB.RevolveVectors
						( pvDstPoints, pvPoints, nCount, vBulletToVB ) ;
					//
					AddBeamInstance
						( vb, pvDstPoints,
							pThickness, pAlpha, nCount, bullet.secLife ) ;
				}
				m_aThickBuf.FinishArray() ;
				m_aAlphaBuf.FinishArray() ;
			}
		}
/*
		m_aPointBuf.SetLimit( (nTrackCount * 2 + 0x0F) & ~0x0F ) ;
		m_aThickBuf.SetLimit( (nTrackCount * 2 + 0x0F) & ~0x0F ) ;
		m_aAlphaBuf.SetLimit( (nTrackCount * 2 + 0x0F) & ~0x0F ) ;
		//
		for ( size_t i = 0; i < m_nThunderCount; i ++ )
		{
			AddThunderInstance( vb, nAlpha, bullet ) ;
		}
*/
	}
	else
	{
		S3DVector4 *	pvPoints = m_aPointBuf.GetArray( nTrackCount ) ;
		float32_t *		pThickness = m_aThickBuf.GetArray( nTrackCount ) ;
		uint32_t *		pAlpha = m_aAlphaBuf.GetArray( nTrackCount ) ;
		//
		for ( size_t i = 0; i < nTrackCount; i ++ )
		{
			pvPoints[i] = m_matBulletToVB * bullet.vTrack[i] + m_vBulletToVB ;
			//
			CalcThicknessAndAlpha
				( pThickness[i], pAlpha[i], (double) i, nAlpha ) ;
			pThickness[i] *= bullet.fpThickness ;
		}
		m_aPointBuf.FinishArray() ;
		m_aThickBuf.FinishArray() ;
		m_aAlphaBuf.FinishArray() ;
		//
		AddBeamInstance( vb, nTrackCount, bullet.secLife ) ;
	}
}

void S3DBulletRenderController::CalcThicknessAndAlpha
	( float32_t& fpThickness, uint32_t& nAlpha,
				double index, uint32_t nTotalAlpha ) const
{
	double		w = 0.0 ;
	uint32_t	a = nTotalAlpha ;
	for ( int i = 1; i < 4; i ++ )
	{
		if ( index <= m_iThickIndex[i] )
		{
			double	t = (double) (index - m_iThickIndex[i-1])
							/ (m_iThickIndex[i] - m_iThickIndex[i-1]) ;
			w = m_fpThickness[i-1]
				+ (m_fpThickness[i] - m_fpThickness[i-1]) * t ;
			if ( i == 1 )
			{
				double	b = (1.0 - m_fpTopAlpha) * t + m_fpTopAlpha ;
				a = (uint32_t) eslRoundR64ToLInt( a * b ) ;
			}
			else if ( i == 3 )
			{
				double	b = 1.0 - (1.0 - m_fpTailAlpha) * t ;
				a = (uint32_t) eslRoundR64ToLInt( a * b ) ;
			}
			break ;
		}
	}
	fpThickness = (float32_t) w ;
	nAlpha = a ;
}

void S3DBulletRenderController::AddThunderInstance
	( S3DVertexBufferInterface& vb,
		uint32_t nTotalAlpha,
		const S3DBulletItemInterface::Bullet& bullet )
{
	m_aPointBuf.RemoveAll() ;
	m_aThickBuf.RemoveAll() ;
	m_aAlphaBuf.RemoveAll() ;
	//
	double	fpNextAmpX = m_random.QuickRandomDouble( m_fpThunderWaveEffect ) ;
	double	fpNextAmpY = m_random.QuickRandomDouble( m_fpThunderWaveEffect ) ;
	double	fpLastAmpX = m_random.QuickRandomDouble( m_fpThunderWaveEffect ) ;
	double	fpLastAmpY = m_random.QuickRandomDouble( m_fpThunderWaveEffect ) ;
	double	fpPhase = 0.0 ;
	//
	const size_t	nTrackCount = bullet.nTrackCount + 1 ;
	S3DVector		vLastPos = bullet.vTrack[0] ;
	//
	m_aPointBuf.Add( vLastPos ) ;
	//
	float32_t	fpThickness ;
	uint32_t	nAlpha ;
	CalcThicknessAndAlpha( fpThickness,nAlpha, 0, nTotalAlpha ) ;
	//
	m_aThickBuf.Add( fpThickness * bullet.fpThickness ) ;
	m_aAlphaBuf.Add( nAlpha ) ;
	//
	for ( size_t i = 1; i < nTrackCount; i ++ )
	{
		S3DVector	vCurPos = bullet.vTrack[i] ;
		S3DVector	vDelta = vCurPos - vLastPos ;
		double		fpLen = vDelta.Absolute() ;
		if ( fpLen < 1.0e-7 )
		{
			continue ;
		}
		S3DMatrix	matDir( 1, 1, 1 ) ;
		matDir.RevolveForAngle( vDelta ) ;
		//
		S3DVector	vBaseX = matDir * S3DVector( 1, 0, 0 ) ;
		S3DVector	vBaseY = matDir * S3DVector( 0, 1, 0 ) ;
		//
		size_t	nDivCount =
			(size_t) eslRoundR64ToLInt( fpLen / m_fpThunderJointLen ) ;
		if ( nDivCount == 0 )
		{
			nDivCount = 1 ;
		}
		else if ( nDivCount > 16 )
		{
			nDivCount = 16 ;
		}
		double	tLast = 0.0 ;
		for ( size_t j = (i == 0) ? 0 : 1; j <= nDivCount; j ++ )
		{
			double	t = (double) j / nDivCount ;
			fpPhase += (t - tLast) * fpLen ;
			if ( fpPhase > m_fpThunderWaveLen )
			{
				fpLastAmpX = fpNextAmpX ;
				fpLastAmpY = fpNextAmpY ;
				fpNextAmpX = m_random.QuickRandomDouble( m_fpThunderWaveEffect ) ;
				fpNextAmpY = m_random.QuickRandomDouble( m_fpThunderWaveEffect ) ;
				fpPhase -= m_fpThunderWaveLen
								* floor( fpPhase / m_fpThunderWaveLen ) ;
			}
			//
			double	ax = m_random.QuickRandomDouble( m_fpThunderJointEffect ) ;
			double	ay = m_random.QuickRandomDouble( m_fpThunderJointEffect ) ;
			//
			double	tw = fpPhase / m_fpThunderWaveLen ;
			ax += (fpNextAmpX - fpLastAmpX) * tw + fpLastAmpX ;
			ay += (fpNextAmpY - fpLastAmpY) * tw + fpLastAmpY ;
			//
			S3DVector4	vPos = vLastPos ;
			vPos += vDelta * t ;
			vPos += vBaseX * ax ;
			vPos += vBaseY * ay ;
			//
			m_aPointBuf.Add( vPos ) ;
			//
			float32_t	fpThickness ;
			uint32_t	nAlpha ;
			CalcThicknessAndAlpha( fpThickness, nAlpha, i + t, nTotalAlpha ) ;
			//
			m_aThickBuf.Add( fpThickness * bullet.fpThickness ) ;
			m_aAlphaBuf.Add( nAlpha ) ;
			//
			tLast = t ;
		}
		vLastPos = vCurPos ;
	}
	if ( m_aPointBuf.GetLength() >= 2 )
	{
		AddBeamInstance( vb, m_aPointBuf.GetLength(), bullet.secLife ) ;
	}
}

void S3DBulletRenderController::AddBeamInstance
	( S3DVertexBufferInterface& vb,
		size_t nTrackCount, float32_t secLife )
{
	ESLAssert( m_aPointBuf.GetLength() >= nTrackCount ) ;
	ESLAssert( m_aThickBuf.GetLength() >= nTrackCount ) ;
	ESLAssert( m_aAlphaBuf.GetLength() >= nTrackCount ) ;
	//
	AddBeamInstance
		( vb, m_aPointBuf.GetConstArray(),
			m_aThickBuf.GetConstArray(),
			m_aAlphaBuf.GetConstArray(), nTrackCount, secLife ) ;
}

void S3DBulletRenderController::AddBeamInstance
	( S3DVertexBufferInterface& vb,
		const S3DVector4 * pvPoints,
		const float32_t * pThickness,
		const uint32_t * pAlpha,
		size_t nTrackCount, float32_t secLife )
{
	if ( m_shape == shapeBand )
	{
		m_tlpBeam.vOffset = secLife * (float32_t) m_fpVSpeed ;
		//
		double		dNearest = 1.0e+10 ;
		ssize_t		iNearest = -1 ;
		S3DVector	vCameraRay = m_vCameraRay ;
		for ( size_t i = 0; i < nTrackCount; i ++ )
		{
			double	d = (m_vCameraPos - pvPoints[i]).Absolute() ;
			if ( d < dNearest )
			{
				dNearest = d ;
				iNearest = (ssize_t) i ;
			}
		}
		if ( iNearest >= 0 )
		{
			vCameraRay += (pvPoints[iNearest] - m_vCameraPos) ;
			vCameraRay.Normalize() ;
		}
		S3DMeshShaper::AddThickLines
			( vb, vCameraRay, m_tlpBeam,
				nTrackCount, pvPoints, pThickness, pAlpha ) ;
	}
	else
	{
		S3DVector	vHandle ;
		S3DMeshShaper::CalcDefaultTubeHandle
			( vHandle, nTrackCount, m_aPointBuf.GetConstArray() ) ;
		//
		m_tubeBeam.uvOffset.y = secLife * (float32_t) m_fpVSpeed ;
		//
		S3DMeshShaper::AddTube
			( vb, m_tubeBeam, vHandle,
				nTrackCount, pvPoints, pThickness, pAlpha ) ;
	}
}



//////////////////////////////////////////////////////////////////////////////
// 弾パーティクル放出コントローラー
//////////////////////////////////////////////////////////////////////////////

const wchar_t *	S3DBulletParticleController::m_pwszTiming
					[S3DBulletParticleController::timingCount] =
{
	L"fire", L"beam_fire", L"hit", L"hit_pre", L"hit_no_pre", L"flight", L"pre_move",
} ;

const wchar_t *	S3DBulletParticleController::m_pwszDirType
					[S3DBulletParticleController::dirTypeCount] =
{
	L"default", L"front", L"back", L"hit_normal", L"hit_reflect", L"option",
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DBulletParticleController, BulletController )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DBulletParticleController, bullet_particle )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DBulletParticleController::S3DBulletParticleController( void )
	: BulletController( m_ItemClassDescriptor.pwszClassID )
{
	m_nGenCount = 0 ;
	m_nTrackCount = 1 ;
	m_timing = timingFire ;
	m_dirGeneration = dirMoveFront ;
	m_dirSpeed = dirMoveBack ;
	m_dirFace = dirDefault ;
	m_vDirOption = S3DVector( 0, -1, 0 ) ;
	m_fpGenCountByThickness = 0.0 ;
	m_fpParticleScaleByThickness = 1.0 ;
	m_fpSpeedScaleByThickness = 1.0 ;
	m_fpGenSpeed = 1.0 ;
	m_fpGenSpeedScale = 0.0 ;
	m_fpZoomScale = 1.0 ;
	m_flagUseColor = false ;
	m_clrParticle = S3DColor( 0xFFFFFFFF, 0 ) ;
	//
	PrepareParameterEntryCount( paramCount ) ;
	AddParameterEntry
		( L"particle_target", S3DSceneComposer::typeCommand,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrStringEnumeration,
			L"対象パーティクル", L"パーティクル生成アイテム" ) ;
	AddParameterEntry
		( L"gen_count", S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrConstant,
			L"生成数", L"パーティクル生成数[個]\nfight の場合には[個/秒]" ) ;
	AddParameterEntry
		( L"gen_count_by_thickness",
			S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrUIScalarSlider,
			L"生成数影響度", L"弾の太さによる生成数影響度", 0.0, 1.0 ) ;
	AddParameterEntry
		( L"timing", S3DSceneComposer::typeCommand,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrStringEnumeration,
			L"生成タイミング", L"パーティクルを生成するタイミング" ) ;
	AddParameterEntry
		( L"flight_tracks", S3DSceneComposer::typeInteger,
			S3DSceneComposer::attrConstant,
			L"有効軌跡",
			L"飛翔中にパーティクルを生成する場合、発生する軌跡の有効節数" ) ;
	AddParameterEntry
		( L"dir_type", S3DSceneComposer::typeCommand,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrStringEnumeration,
			L"生成向き", L"パーティクルを生成する方向" ) ;
	AddParameterEntry
		( L"direction", S3DSceneComposer::typeDirection,
			S3DSceneComposer::attrConstant,
			L"生成ベクトル", L"パーティクルを生成する方向ベクトル" ) ;
	AddParameterEntry
		( L"speed_dir_type", S3DSceneComposer::typeCommand,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrStringEnumeration,
			L"放出速度向き", L"パーティクルを放出する方向" ) ;
	AddParameterEntry
		( L"gen_speed", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant,
			L"放出速度", L"パーティクルを放出する速度（放出方向成分）" ) ;
	AddParameterEntry
		( L"gen_speed_scale", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant,
			L"放出弾速度比", L"パーティクルを放出する速度の弾速度比加算" ) ;
	AddParameterEntry
		( L"speed_by_thickness",
			S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrUIScalarSlider,
			L"放出速度影響度",
			L"弾の太さによる放出する速度（パーティクル固有の放射方向）の影響度", 0.0, 1.0 ) ;
	AddParameterEntry
		( L"zoom_scale", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant,
			L"パーティクル拡大率", L"パーティクルの拡大比率" ) ;
	AddParameterEntry
		( L"zoom_by_thickness",
			S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrUIScalarSlider,
			L"パーティクル拡大率影響度",
			L"弾の太さによるパーティクル拡大率影響度", 0.0, 1.0 ) ;
	AddParameterEntry
		( L"face_dir_type", S3DSceneComposer::typeCommand,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrStringEnumeration,
			L"パーティクル面向き", L"生成するパーティクルの面の向き" ) ;
	AddParameterEntry
		( L"use_color", S3DSceneComposer::typeBoolean,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrDynamicValidation,
			L"色を指定する", L"パーティクルの色を指定する" ) ;
	AddParameterEntry
		( L"color_mul", S3DSceneComposer::typeColor,
			S3DSceneComposer::attrConstant,
			L"生成色（乗算）", L"パーティクルの色（乗算）" ) ;
	AddParameterEntry
		( L"color_add", S3DSceneComposer::typeColor,
			S3DSceneComposer::attrConstant,
			L"生成色（加算）", L"パーティクルの色（加算）" ) ;
	AddParameterEntry
		( L"color_alpha", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrUIScalarSlider,
			L"生成不透明度", L"パーティクルの不透明度", 0.0, 1.0 ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DBulletParticleController::~S3DBulletParticleController( void )
{
}

// 描画ターゲット設定
//////////////////////////////////////////////////////////////////////////////
void S3DBulletParticleController::AttachParticleTarget
	( S3DParticleSerializer * pTarget, const wchar_t * pwszID )
{
	m_refParticleTarget.SetReference
			( (S3DSceneComposer::ItemSerializer*) pTarget ) ;
	m_strParticleTarget = pwszID ;
}

bool S3DBulletParticleController::UpdateParticleTarget( void )
{
	if ( m_strParticleTarget.IsEmpty() )
	{
		m_refParticleTarget.SetReference( NULL ) ;
		return	true ;
	}
	S3DSceneComposer::Composition *	pComp = GetComposition() ;
	if ( pComp != NULL )
	{
		S3DSceneComposer::ItemSerializer *
			pItem = pComp->GetSceneItemAs( m_strParticleTarget ) ;
		m_refParticleTarget.SetReference( pItem ) ;
		//
		return	(pItem != NULL) ;
	}
	return	false ;
}

// タイミング文字列解釈
//////////////////////////////////////////////////////////////////////////////
S3DBulletParticleController::TimingOfGeneration
	S3DBulletParticleController::ParseTimingOfGeneration( const wchar_t * pwszType )
{
	for ( int i = 0; i < timingCount; i ++ )
	{
		if ( SString::Compare( pwszType, m_pwszTiming[i] ) == 0 )
		{
			return	(TimingOfGeneration) i ;
		}
	}
	return	timingFire ;
}

// 方向文字列解釈
//////////////////////////////////////////////////////////////////////////////
S3DBulletParticleController::DirectionType
	S3DBulletParticleController::ParseDirectionType( const wchar_t * pwszType )
{
	for ( int i = 0; i < dirTypeCount; i ++ )
	{
		if ( SString::Compare( pwszType, m_pwszDirType[i] ) == 0 )
		{
			return	(DirectionType) i ;
		}
	}
	return	dirDefault ;
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDVector S3DBulletParticleController::GetVectorParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramDirection:
		return	S3DDVector( m_vDirOption ) ;
	case	paramColorMul:
		return	VectorFromColor( m_clrParticle.rgbMul ) ;
	case	paramColorAdd:
		return	VectorFromColor( m_clrParticle.rgbAdd ) ;
	}
	return	S3DDVector( 0, 0, 0 ) ;
}

double S3DBulletParticleController::GetScalarParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramGenCountByThickness:
		return	m_fpGenCountByThickness ;
	case	paramSpeed:
		return	m_fpGenSpeed ;
	case	paramSpeedScale:
		return	m_fpGenSpeedScale ;
	case	paramSpeedByThickness:
		return	m_fpSpeedScaleByThickness ;
	case	paramZoomScale:
		return	m_fpZoomScale ;
	case	paramZoomByThickness:
		return	m_fpParticleScaleByThickness ;
	case	paramAlpha:
		return	m_clrParticle.rgbMul.argb.Alpha / 255.0 ;
	}
	return	0.0 ;
}

int32_t S3DBulletParticleController::GetIntegerParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramGenCount:
		return	(int32_t) m_nGenCount ;
	case	paramFlightTracks:
		return	(int32_t) m_nTrackCount ;
	}
	return	0 ;
}

bool S3DBulletParticleController::GetBooleanParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramUseColor:
		return	m_flagUseColor ;
	}
	return	false ;
}

const wchar_t * S3DBulletParticleController::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramEmitterTarget:
		return	m_strParticleTarget ;
	case	paramTiming:
		return	m_pwszTiming[m_timing] ;
	case	paramDirType:
		return	m_pwszDirType[m_dirGeneration] ;
	case	paramSpeedDir:
		return	m_pwszDirType[m_dirSpeed] ;
	case	paramFaceDir:
		return	m_pwszDirType[m_dirFace] ;
	}
	return	NULL ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DBulletParticleController::SetVectorParameter( size_t i, const S3DDVector& vec )
{
	switch ( i )
	{
	case	paramDirection:
		m_vDirOption = vec ;
		return ;
	case	paramColorMul:
		{
			SGLPalette	argb = ColorFromVector( vec ) ;
			argb.argb.Alpha = m_clrParticle.rgbMul.argb.Alpha ;
			m_clrParticle.rgbMul = argb ;
		}
		return ;
	case	paramColorAdd:
		m_clrParticle.rgbAdd = ColorFromVector( vec ) ;
		return ;
	}
}

void S3DBulletParticleController::SetScalarParameter( size_t i, double s )
{
	switch ( i )
	{
	case	paramGenCountByThickness:
		m_fpGenCountByThickness = s ;
		return ;
	case	paramSpeed:
		m_fpGenSpeed = s ;
		return ;
	case	paramSpeedScale:
		m_fpGenSpeedScale = s ;
		return ;
	case	paramSpeedByThickness:
		m_fpSpeedScaleByThickness = s ;
		return ;
	case	paramZoomScale:
		m_fpZoomScale = s ;
		return ;
	case	paramZoomByThickness:
		m_fpParticleScaleByThickness = s ;
		return ;
	case	paramAlpha:
		m_clrParticle.rgbMul.argb.Alpha =
			(uint8_t) esl_clampi
				( eslRoundR32ToInt( (float32_t) s * 255.0f ), 0, 0xFF ) ;
		return ;
	}
}

void S3DBulletParticleController::SetIntegerParameter( size_t i, int32_t n )
{
	switch ( i )
	{
	case	paramGenCount:
		m_nGenCount = (size_t) n ;
		return ;
	case	paramFlightTracks:
		m_nTrackCount = (size_t) n ;
		return ;
	}
}

void S3DBulletParticleController::SetBooleanParameter( size_t i, bool b )
{
	switch ( i )
	{
	case	paramUseColor:
		m_flagUseColor = b ;
		return ;
	}
}

void S3DBulletParticleController::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	switch ( i )
	{
	case	paramEmitterTarget:
		if ( m_strParticleTarget != pwszCmd )
		{
			m_strParticleTarget = pwszCmd ;
			UpdateParticleTarget() ;
		}
		return ;
	case	paramTiming:
		m_timing = ParseTimingOfGeneration( pwszCmd ) ;
		return ;
	case	paramDirType:
		m_dirGeneration = ParseDirectionType( pwszCmd ) ;
		return ;
	case	paramSpeedDir:
		m_dirSpeed = ParseDirectionType( pwszCmd ) ;
		return ;
	case	paramFaceDir:
		m_dirFace = ParseDirectionType( pwszCmd ) ;
		return ;
	}
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DBulletParticleController::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	int	j ;
	switch ( i )
	{
	case	paramEmitterTarget:
		{
			S3DSceneComposer::Composition *	pComp = GetComposition() ;
			if ( pComp != NULL )
			{
				pComp->EnumerateItemIDsAs
					( aStrSet, ESL_RUNTIME_CLASS(S3DParticleSerializer) ) ;
			}
		}
		return	true ;

	case	paramTiming:
		for ( j = 0; j < timingCount; j ++ )
		{
			aStrSet.Add( new SString(m_pwszTiming[j]) ) ;
		}
		return	true ;

	case	paramDirType:
	case	paramSpeedDir:
	case	paramFaceDir:
		for ( j = 0; j < dirTypeCount; j ++ )
		{
			aStrSet.Add( new SString(m_pwszDirType[j]) ) ;
		}
		return	true ;
	}
	return	false ;
}

// パラメーター有効性
//////////////////////////////////////////////////////////////////////////////
bool S3DBulletParticleController::IsParameterValidation( size_t i ) const
{
	switch ( i )
	{
	case	paramColorMul:
	case	paramColorAdd:
	case	paramAlpha:
		return	m_flagUseColor ;
	}
	return	true ;
}

// パラメータカテゴリ名取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * S3DBulletParticleController::GetParameterCategoryName( size_t iCategory ) const
{
	return	L"パーティクル生成" ;
}

// アイテムプロパティのリソース等の参照を更新する
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DBulletParticleController::UpdatePropertyReference
	( S3DSceneComposer::Composition& comp,
		S3DSceneComposer::ItemSerializer * pItem, uint32_t nFlags )
{
	uint32_t	nResFlags =
		Controller::UpdatePropertyReference( comp, pItem, nFlags ) ;
	//
	if ( nFlags & S3DSceneComposer::updateRefItem )
	{
		if ( !UpdateParticleTarget() )
		{
			nResFlags |= S3DSceneComposer::updateRefItem ;
		}
	}
	return	nResFlags ;
}

// 発射時処理
//////////////////////////////////////////////////////////////////////////////
void S3DBulletParticleController::OnFireBullet
	( S3DBulletItemInterface * pItem,
		S3DBulletItemInterface::Bullet& bullet )
{
	if ( m_timing == timingFire )
	{
		S3DVector	vNormal = bullet.vSpeed ;
		GenerateParticles
			( m_nGenCount, 0.0f, pItem, bullet, NULL, &vNormal ) ;
	}
}

// 着弾時処理
//////////////////////////////////////////////////////////////////////////////
void S3DBulletParticleController::OnHitBullet
	( S3DBulletItemInterface * pItem,
		S3DSceneComposer::ItemSerializer * pHitItem,
		const S3DCollision::MeshCollision * pmcHitMesh,
		const S3DVector& vHitPos, const S3DVector& vHitNormal,
		S3DBulletItemInterface::Bullet& bullet, size_t iHitTrack )
{
	if ( (m_timing == timingHit)
		|| ((m_timing == timingHitPrepare)
			&& (bullet.nStateFlags & S3DBulletItemInterface::statePrepareLaser))
		|| ((m_timing == timingHitNoPrepare)
			&& !(bullet.nStateFlags & S3DBulletItemInterface::statePrepareLaser)) )
	{
		GenerateParticles
			( m_nGenCount, 0.0f, pItem, bullet, &vHitPos, &vHitNormal ) ;
	}
}

// タイマー前処理
//////////////////////////////////////////////////////////////////////////////
void S3DBulletParticleController::BeforeBulletTimer
	( S3DBulletItemInterface * pItem, float32_t secPast,
			S3DBulletItemInterface::Bullet*const* pBullets, size_t nCount )
{
}

// タイマー処理
//////////////////////////////////////////////////////////////////////////////
void S3DBulletParticleController::OnBulletTimer
	( S3DBulletItemInterface * pItem, float32_t secPast,
		S3DBulletItemInterface::Bullet*const* pBullets, size_t nCount )
{
	if ( (m_timing == timingFlight)
		|| (m_timing == timingPreMove)
		|| (m_timing == timingBeamFire) )
	{
		S3DParticleSerializer *	pParticle =
					ESLTypeCast<S3DParticleSerializer>
						( m_refParticleTarget.GetReference() ) ;
		if ( pParticle == nullptr )
		{
			return ;
		}
		double	fpCount = m_nGenCount * secPast ;
		size_t	nGenCount = pParticle->GenerateCount( fpCount ) ;
		if ( nGenCount != 0 )
		{
			for ( size_t i = 0; i < nCount; i ++ )
			{
				if ( pBullets[i] != nullptr )
				{
					S3DVector *	pvGenPos = nullptr ;
					S3DVector	vGenPos ;
					if ( m_timing == timingBeamFire )
					{
						vGenPos = pBullets[i]->vPos ;
						pvGenPos = &vGenPos ;
					}
					GenerateParticles
						( nGenCount, secPast,
							pItem, *(pBullets[i]), pvGenPos, nullptr ) ;
				}
			}
		}
	}
}

// 当たり判定追加処理
//////////////////////////////////////////////////////////////////////////////
void S3DBulletParticleController::RenderBulletCollision
	( const S3DScene& scene,
		S3DCollision& collision,
		const S3DBulletItemInterface * pItem,
		S3DBulletItemInterface::Bullet*const* pBullets, size_t nCount )
{
}

// 弾軌跡描画（追加処理）
//////////////////////////////////////////////////////////////////////////////
void S3DBulletParticleController::RenderBulletTracks
	( S3DScene& scene, const S3DBulletItemInterface * pItem,
		const S3DBulletItemInterface::Bullet*const* pBullets, size_t nCount )
{
}

// パーティクル生成
//////////////////////////////////////////////////////////////////////////////
void S3DBulletParticleController::GenerateParticles
	( size_t nCount, float32_t secPast,
		S3DBulletItemInterface * pItem,
		const S3DBulletItemInterface::Bullet& bullet,
		const S3DVector * pvHitPos, const S3DVector * pvHitNormal )
{
	if ( nCount == 0 )
	{
		return ;
	}
	S3DParticleSerializer *	pParticle =
				ESLTypeCast<S3DParticleSerializer>
					( m_refParticleTarget.GetReference() ) ;
	if ( pParticle == NULL )
	{
		return ;
	}
	nCount = pParticle->GenerateCount
			( nCount * pow( fabs(bullet.fpThickness), m_fpGenCountByThickness ) ) ;
	if ( nCount == 0 )
	{
		return ;
	}
	//
	S3DColor *	pclrParticle = NULL ;
	S3DColor	clrParticle( 0xFFFFFFFF, 0 ) ;
	if ( m_flagUseColor )
	{
		clrParticle = m_clrParticle ;
		pclrParticle = &clrParticle ;
	}
	if ( bullet.nAlpha < 0xFF )
	{
		clrParticle.rgbMul.argb.Alpha = (uint32_t) esl_min( bullet.nAlpha, 0xFF ) ;
		pclrParticle = &clrParticle ;
	}
	//
	S3DDMatrix	matdParticle( 1, 1, 1 ) ;
	S3DDVector	vdParticle( 0, 0, 0 ) ;
	if ( pParticle->GetParticleParameter().nFlags
				& S3DParticleSerializer::particleLocalSpace )
	{
		pParticle->CalcGlobalTransformation( matdParticle, vdParticle ) ;
	}
	S3DDMatrix	matdItem( 1, 1, 1 ) ;
	S3DDVector	vdItem( 0, 0, 0 ) ;
	if ( pItem->GetBulletFlags() & S3DBulletItemInterface::flagTransform )
	{
		pItem->GetGlobalTransformation( matdItem, vdItem ) ;
	}
	if ( pParticle->GetParticleParameter().nFlags
				& S3DParticleSerializer::particleLocalSpace )
	{
		S3DDMatrix	matdIParticle = matdParticle.Inverse() ;
		vdItem = matdIParticle * (vdItem - vdParticle) ;
		matdItem = matdIParticle * matdItem ;
	}
	S3DMatrix	matParticle = matdParticle ;
	//
	const S3DParticleSerializer::EmissionParam&
				emission = pParticle->GetEmissionParameter() ;
	const S3DParticleSerializer::ParticleParam&
				particle = pParticle->GetParticleParameter() ;
	S3DVector	vSpeed = matdItem * bullet.vSpeed ;
	double		fpBulletSpeed = vSpeed.Absolute() ;
	//
	S3DVector	vGenDir =
		GetParticleDirection
			( m_dirGeneration,
				matParticle * emission.vDirection,
				vSpeed, pvHitNormal ) ;
	S3DVector	vGenSpeed =
		GetParticleDirection
			( m_dirSpeed,
				matParticle * emission.vBaseSpeed,
				vSpeed, pvHitNormal ) ;
	S3DVector	vGenFace =
		GetParticleDirection
			( m_dirFace,
				matParticle * particle.vDefFaceDir,
				vSpeed, pvHitNormal ) ;
	//
	uint32_t	nTrackCount =
		(uint32_t) esl_min( (int) m_nTrackCount,
								(int) bullet.nTrackCount ) + 1 ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		uint32_t	iTrack =
			pParticle->Randomizer().QuickRandomize( nTrackCount ) ;
		S3DDVector	vPos = bullet.vTrack[iTrack] ;
		if ( pvHitPos != NULL )
		{
			vPos = *pvHitPos ;
		}
		else if ( m_timing == timingPreMove )
		{
			S3DDVector	vDelta = bullet.vSpeed * secPast ;
			if ( iTrack > 0 )
			{
				vDelta = bullet.vTrack[iTrack-1] - vPos ;
			}
			double	t =
				pParticle->Randomizer().QuickRandomize( 1000 ) / 1000.0 ;
			vPos += vDelta * t ;
			//
			vGenSpeed =
				GetParticleDirection
					( m_dirSpeed,
						matParticle * emission.vBaseSpeed,
						matdItem * vDelta, pvHitNormal ) ;
		}
		else if ( iTrack + 1 < nTrackCount )
		{
			S3DDVector	vDelta = vPos - bullet.vTrack[iTrack+1] ;
			double	t =
				pParticle->Randomizer().QuickRandomize( 1000 ) / 1000.0 ;
			vPos -= vDelta * t ;
			//
			if ( (nTrackCount >= 2)
				&& (vDelta.InnerProduct(vDelta) > 1.0e-7) )
			{
				vGenSpeed =
					GetParticleDirection
						( m_dirSpeed,
							matParticle * emission.vBaseSpeed,
							matdItem * vDelta, pvHitNormal ) ;
			}
		}
		else
		{
			S3DDVector	vDelta = bullet.vSpeed ;
			vGenSpeed =
				GetParticleDirection
					( m_dirSpeed,
						matParticle * emission.vBaseSpeed,
						matdItem * vDelta, pvHitNormal ) ;
		}
		vGenSpeed.Normalize() ;
		vGenSpeed *= (float32_t) (m_fpGenSpeed
									+ m_fpGenSpeedScale * fpBulletSpeed) ;
		//
		const float32_t	fpParticleScale =
			(float32_t) pow( fabs(bullet.fpThickness), m_fpParticleScaleByThickness ) ;
		const float32_t	fpSpeedScale =
			(float32_t) pow( fabs(bullet.fpThickness), m_fpSpeedScaleByThickness ) ;
		S3DDVector	vGenPos = matdItem * vPos + vdItem ;
		pParticle->GenerateParticlesByParam
			( 1, vGenPos, &vGenDir, &vGenSpeed, &vGenFace,
				pclrParticle, 
				(float32_t) m_fpZoomScale * fpParticleScale,
				1.0f, fpSpeedScale ) ;
	}
}

// 方向取得
//////////////////////////////////////////////////////////////////////////////
S3DVector S3DBulletParticleController::GetParticleDirection
	( S3DBulletParticleController::DirectionType dirType,
		const S3DVector& vDefault,
		const S3DVector& vSpeed,
		const S3DVector * pvHitNormal ) const
{
	switch ( dirType )
	{
	case	dirDefault:
	default:
		break ;

	case	dirMoveFront:
		return	vSpeed ;

	case	dirMoveBack:
		return	- vSpeed ;

	case	dirHitNormal:
		if ( pvHitNormal != NULL )
		{
			return	*pvHitNormal ;
		}
		break ;

	case	dirHitReflect:
		if ( pvHitNormal != NULL )
		{
			S3DVector	vNormal = *pvHitNormal ;
			vNormal.Normalize() ;
			//
			float32_t	a = vNormal.InnerProduct( vSpeed ) ;
			return	vSpeed - vNormal * (a * 2.0f) ;
		}
		break ;

	case	dirOption:
		return	m_vDirOption ;
	}
	return	vDefault ;
}



//////////////////////////////////////////////////////////////////////////////
// 弾効果音・コントローラー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DBulletSoundController::SoundInstancePtr, ESLObject )
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DBulletSoundController, BulletController )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DBulletSoundController, bullet_sound )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DBulletSoundController::S3DBulletSoundController( void )
	: BulletController( m_ItemClassDescriptor.pwszClassID ),
		m_fpFireVolume( 1.0 ), m_fpHitVolume( 1.0 ), m_fpWindNoiseVolume( 1.0 )
{
	PrepareParameterEntryCount( paramCount ) ;
	ESLVerify( paramFireSound == AddParameterEntry
		( L"fire_sound", S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrStringEnumeration,
			L"発射サウンド" ) ) ;
	ESLVerify( paramFireVolume == AddParameterEntry
		( L"fire_volume", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrUIScalarSlider,
			L"発射音量", nullptr, 0.0, 1.0 ) ) ;
	ESLVerify( paramHitSound == AddParameterEntry
		( L"hit_sound", S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrStringEnumeration,
			L"着弾サウンド" ) ) ;
	ESLVerify( paramHitVolume == AddParameterEntry
		( L"hit_volume", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrUIScalarSlider,
			L"着弾音量", nullptr, 0.0, 1.0 ) ) ;
	ESLVerify( paramWindNoiseSound == AddParameterEntry
		( L"wind_noise_sound", S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrStringEnumeration,
			L"風切り音" ) ) ;
	ESLVerify( paramWindNoiseVolume == AddParameterEntry
		( L"wind_noise_volume", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrUIScalarSlider,
			L"風切り音量", nullptr, 0.0, 1.0 ) ) ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DBulletSoundController::~S3DBulletSoundController( void )
{
}

// 参照サウンドアイテム更新
//////////////////////////////////////////////////////////////////////////////
void S3DBulletSoundController::UpdatreFireSoundRef( void )
{
	m_refFireSound.SetReference( nullptr ) ;
	if ( m_strFireSound.IsEmpty() )
	{
		return ;
	}
	S3DSceneComposer::Composition *	pComp = GetComposition() ;
	if ( pComp != nullptr )
	{
		m_refFireSound.SetReference( pComp->GetSceneItemAs( m_strFireSound ) ) ;
	}
}

void S3DBulletSoundController::UpdatreHitSoundRef( void )
{
	m_refHitSound.SetReference( nullptr ) ;
	if ( m_strHitSound.IsEmpty() )
	{
		return ;
	}
	S3DSceneComposer::Composition *	pComp = GetComposition() ;
	if ( pComp != nullptr )
	{
		m_refHitSound.SetReference( pComp->GetSceneItemAs( m_strHitSound ) ) ;
	}
}

void S3DBulletSoundController::UpdatreWindNoiseRef( void )
{
	m_refWindNoise.SetReference( nullptr ) ;
	if ( m_strWindNoise.IsEmpty() )
	{
		return ;
	}
	S3DSceneComposer::Composition *	pComp = GetComposition() ;
	if ( pComp != nullptr )
	{
		m_refWindNoise.SetReference( pComp->GetSceneItemAs( m_strWindNoise ) ) ;
	}
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
double S3DBulletSoundController::GetScalarParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramFireVolume:
		return	m_fpFireVolume ;
	case	paramHitVolume:
		return	m_fpHitVolume ;
	case	paramWindNoiseVolume:
		return	m_fpWindNoiseVolume ;
	}
	return	0.0 ;
}

const wchar_t * S3DBulletSoundController::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramFireSound:
		return	m_strFireSound ;
	case	paramHitSound:
		return	m_strHitSound ;
	case	paramWindNoiseSound:
		return	m_strWindNoise ;
	}
	return	nullptr ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DBulletSoundController::SetScalarParameter( size_t i, double s )
{
	switch ( i )
	{
	case	paramFireVolume:
		m_fpFireVolume = s ;
		return ;
	case	paramHitVolume:
		m_fpHitVolume = s ;
		return ;
	case	paramWindNoiseVolume:
		m_fpWindNoiseVolume = s ;
		return ;
	}
}

void S3DBulletSoundController::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	switch ( i )
	{
	case	paramFireSound:
		if ( m_strFireSound != pwszCmd )
		{
			m_strFireSound = pwszCmd ;
			UpdatreFireSoundRef() ;
		}
		return ;
	case	paramHitSound:
		if ( m_strHitSound != pwszCmd )
		{
			m_strHitSound = pwszCmd ;
			UpdatreHitSoundRef() ;
		}
		return ;
	case	paramWindNoiseSound:
		if ( m_strWindNoise != pwszCmd )
		{
			m_strWindNoise = pwszCmd ;
			UpdatreWindNoiseRef() ;
		}
		return ;
	}
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DBulletSoundController::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	if ( (i == paramFireSound)
		|| (i == paramHitSound)
		|| (i == paramWindNoiseSound) )
	{
		S3DSceneComposer::Composition *	pComp = GetComposition() ;
		if ( pComp != NULL )
		{
			pComp->EnumerateItemIDsAs
				( aStrSet, ESL_RUNTIME_CLASS(S3DSoundItemSerializer) ) ;
		}
		return	true ;
	}
	return	false ;
}

// アイテムプロパティのリソース等の参照を更新する
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DBulletSoundController::UpdatePropertyReference
	( S3DSceneComposer::Composition& comp,
		S3DSceneComposer::ItemSerializer * pItem, uint32_t nFlags )
{
	uint32_t	nResFlags =
		BulletController::UpdatePropertyReference( comp, pItem, nFlags ) ;
	//
	if ( nFlags & S3DSceneComposer::updateRefItem )
	{
		UpdatreFireSoundRef() ;
		UpdatreHitSoundRef() ;
		UpdatreWindNoiseRef() ;
	}
	return	nResFlags ;
}

// 発射時処理
//////////////////////////////////////////////////////////////////////////////
void S3DBulletSoundController::OnFireBullet
	( S3DBulletItemInterface * pItem,
		S3DBulletItemInterface::Bullet& bullet )
{
	S3DSoundItemSerializer *	pSound = m_refFireSound.GetRef<S3DSoundItemSerializer>() ;
	if ( (pSound != nullptr) && (m_fpFireVolume > 0.0) )
	{
		S3DDMatrix	matdSound ;
		S3DDVector	vdSound ;
		pSound->GetGlobalTransformation( matdSound, vdSound ) ;
		//
		S3DDVector	vPos = matdSound.Inverse() * (bullet.vPos - vdSound) ;
		pSound->PlayTemporary( m_fpFireVolume, nullptr, nullptr, &vPos ) ;
	}
	pSound = m_refWindNoise.GetRef<S3DSoundItemSerializer>() ;
	if ( (pSound != nullptr) && (m_fpWindNoiseVolume > 0.0) )
	{
		SoundInstancePtr *	psip =
			ESLTypeCast<SoundInstancePtr>
				( S3DBulletItemInterface::GetBulletInstance
					( bullet, ESL_RUNTIME_CLASS(SoundInstancePtr) ) ) ;
		if ( psip == nullptr )
		{
			S3DDMatrix	matdSound ;
			S3DDVector	vdSound ;
			pSound->GetGlobalTransformation( matdSound, vdSound ) ;
			//
			S3DDVector	vPos = matdSound.Inverse() * (bullet.vPos - vdSound) ;
			psip = new SoundInstancePtr
				( pSound, pSound->PlayTemporary
							( m_fpWindNoiseVolume, nullptr, nullptr, &vPos ) ) ;
			S3DBulletItemInterface::AddBulletInstance( bullet, psip ) ;
		}
	}
}

// 着弾時処理
//////////////////////////////////////////////////////////////////////////////
void S3DBulletSoundController::OnHitBullet
	( S3DBulletItemInterface * pItem,
		S3DSceneComposer::ItemSerializer * pHitItem,
		const S3DCollision::MeshCollision * pmcHitMesh,
		const S3DVector& vHitPos, const S3DVector& vHitNormal,
		S3DBulletItemInterface::Bullet& bullet, size_t iHitTrack )
{
	S3DSoundItemSerializer *	pSound = m_refHitSound.GetRef<S3DSoundItemSerializer>() ;
	if ( (pSound != nullptr) && (m_fpHitVolume > 0.0) )
	{
		S3DDMatrix	matdSound ;
		S3DDVector	vdSound ;
		pSound->GetGlobalTransformation( matdSound, vdSound ) ;
		//
		S3DDVector	vPos = matdSound.Inverse() * (bullet.vPos - vdSound) ;
		pSound->PlayTemporary( m_fpHitVolume, nullptr, nullptr, &vPos ) ;
	}
}

// タイマー前処理
//////////////////////////////////////////////////////////////////////////////
void S3DBulletSoundController::BeforeBulletTimer
	( S3DBulletItemInterface * pItem, float32_t secPast,
			S3DBulletItemInterface::Bullet*const* pBullets, size_t nCount )
{
}

// タイマー処理
//////////////////////////////////////////////////////////////////////////////
void S3DBulletSoundController::OnBulletTimer
	( S3DBulletItemInterface * pItem, float32_t secPast,
		S3DBulletItemInterface::Bullet*const* pBullets, size_t nCount )
{
	S3DSoundItemSerializer *	pSound = m_refWindNoise.GetRef<S3DSoundItemSerializer>() ;
	if ( pSound == nullptr )
	{
		return ;
	}
	ESLAssert( GetOwnerItem() != nullptr ) ;
	S3DScene *	pScene = GetOwnerItem()->GetScene() ;
	if ( pScene == nullptr )
	{
		return ;
	}
	S3DScene::Camera *	pCamera = pScene->GetMainCamera() ;
	if ( pCamera == nullptr )
	{
		return ;
	}
	S3DDMatrix	matdItem( 1, 1, 1 ) ;
	S3DDVector	vItem( 0, 0, 0 ) ;
	if ( pItem->GetBulletFlags() & S3DBulletItemInterface::flagTransform )
	{
		pItem->GetGlobalTransformation( matdItem, vItem ) ;
	}
	S3DDMatrix	matCameraSpace ;
	S3DDVector	vdCameraSpace ;
	pCamera->CalcItemLinkTransformation( matCameraSpace, vdCameraSpace ) ;
	//
	S3DDVector	vdCameraPos = matCameraSpace
								* pCamera->GetCameraPosition() + vdCameraSpace ;
	//
	for ( size_t i = 0; i < nCount; i ++ )
	{
		if ( pBullets[i] == nullptr )
		{
			continue ;
		}
		S3DBulletItemInterface::Bullet&	bullet = *(pBullets[i]) ;
		SoundInstancePtr *	psip =
			ESLTypeCast<SoundInstancePtr>
				( S3DBulletItemInterface::GetBulletInstance
					( bullet, ESL_RUNTIME_CLASS(SoundInstancePtr) ) ) ;
		pSound->LockInstance() ;
		if ( (psip == nullptr)
			|| !pSound->IsValidInstance( psip->m_pInstance ) )
		{
			pSound->UnlockInstance() ;
			continue ;
		}
		if ( (pItem->GetBulletFlags() & S3DBulletItemInterface::flagLaser)
			&& (bullet.nTrackCount >= 1) )
		{
			S3DDVector	vTrack0 = matdItem * bullet.vTrack[0] + vItem ;
			S3DDVector	vTrack1 = matdItem * bullet.vTrack[1] + vItem ;
			S3DDVector	vLaserDelta = vTrack0 - vTrack1 ;
			S3DDVector	vCameraDelta = vdCameraPos - vTrack1 ;
			double		fpLaserLen = vLaserDelta.Absolute() ;
			if ( fpLaserLen > 1.0e-5 )
			{
				S3DDVector	vLaserDir = vLaserDelta / fpLaserLen ;
				double		d = esl_fclamp( vLaserDir.InnerProduct( vCameraDelta ),
																0.0, fpLaserLen ) ;
				psip->m_pInstance->m_vPos = vTrack1 + vLaserDir * d ;
			}
			else
			{
				psip->m_pInstance->m_vPos = matdItem * bullet.vPos + vItem ;
			}
		}
		else
		{
			psip->m_pInstance->m_vPos = matdItem * bullet.vPos + vItem ;
		}
		psip->m_pInstance->m_fpVolume = (double) bullet.nAlpha / 255.0 ;
		pSound->UnlockInstance() ;
	}
}

// 当たり判定追加処理
//////////////////////////////////////////////////////////////////////////////
void S3DBulletSoundController::RenderBulletCollision
	( const S3DScene& scene,
		S3DCollision& collision,
		const S3DBulletItemInterface * pItem,
		S3DBulletItemInterface::Bullet*const* pBullets, size_t nCount )
{
}

// 弾軌跡描画（追加処理）
//////////////////////////////////////////////////////////////////////////////
void S3DBulletSoundController::RenderBulletTracks
	( S3DScene& scene, const S3DBulletItemInterface * pItem,
		const S3DBulletItemInterface::Bullet*const* pBullets, size_t nCount )
{
}


//////////////////////////////////////////////////////////////////////////////
// 弾サブコンポジション・コントローラー
//////////////////////////////////////////////////////////////////////////////

const wchar_t *	S3DBulletSubCompositionController::s_pwszMatrixOperation[S3DBulletSubCompositionController::matrixOpCount] =
{
	L"identity", L"direction",
} ;

const SSystem::SXMLDocument::AttrInteger
	S3DBulletSubCompositionController::s_aiMatrixOperation[S3DBulletSubCompositionController::matrixOpCount+1] =
{
	{ L"identity", S3DBulletSubCompositionController::matrixIdentity },
	{ L"direction", S3DBulletSubCompositionController::matrixDirection },
	{ nullptr, 0 },
} ;

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DBulletSubCompositionController, BulletController )
S3D_IMPLEMENT_COMPOSER_ITEM( SakuraGL::S3DBulletSubCompositionController, bullet_subcomp )
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::S3DBulletSubCompositionController::Container, ESLObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
S3DBulletSubCompositionController::S3DBulletSubCompositionController( void )
	: BulletController( m_ItemClassDescriptor.pwszClassID ),
		m_matrixBullet(matrixDirection), m_vBulletSize( 1, 1, 1 ),
		m_matrixFireEffect(matrixDirection), m_vFireEffectSize( 1, 1, 1 ),
		m_matrixHitEffect(matrixIdentity), m_vHitEffectSize( 1, 1, 1 ),
		m_matrixDrop( matrixDirection ), m_vDropSize( 1, 1, 1 ),
		m_secDropInterval( 0.2 ),
		m_vDropRndPos( 0, 0, 0 ), m_fpZoomRndRate( 0.0 ),
		m_vRndEulerAngles1( 0, 0, 0 ), m_vRndEulerAngles2( 0, 0, 0 )
{
	PrepareParameterEntryCount( paramCount ) ;
	ESLVerify( paramBulletSubComp == AddParameterEntry
		( L"bullet_subcomp", S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrStringEnumeration,
			L"弾頭コンポジション" ) ) ;
	ESLVerify( paramBulletMatrixOp == AddParameterEntry
		( L"bullet_matrix", S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrStringEnumeration
			| S3DSceneComposer::attrUIOnlyEnumeration,
			L"弾頭向き" ) ) ;
	ESLVerify( paramBulletSize == AddParameterEntry
		( L"bullet_size", S3DSceneComposer::typeZoom,
			S3DSceneComposer::attrConstant,
			L"弾頭拡大率" ) ) ;
	ESLVerify( paramFireEffectSubComp == AddParameterEntry
		( L"fire_effect_subcomp", S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrStringEnumeration,
			L"発射効果コンポジション" ) ) ;
	ESLVerify( paramFireEffectMatrixOp == AddParameterEntry
		( L"fire_effect_matrix", S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrStringEnumeration
			| S3DSceneComposer::attrUIOnlyEnumeration,
			L"発射効果向き" ) ) ;
	ESLVerify( paramFireEffectSize == AddParameterEntry
		( L"fire_effect_size", S3DSceneComposer::typeZoom,
			S3DSceneComposer::attrConstant,
			L"発射効果拡大率" ) ) ;
	ESLVerify( paramHitEffectSubComp == AddParameterEntry
		( L"hit_effect_subcomp", S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrStringEnumeration,
			L"着弾効果コンポジション" ) ) ;
	ESLVerify( paramHitEffectMatrixOp == AddParameterEntry
		( L"hit_effect_matrix", S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrStringEnumeration
			| S3DSceneComposer::attrUIOnlyEnumeration,
			L"着弾効果向き" ) ) ;
	ESLVerify( paramHitEffectSize == AddParameterEntry
		( L"hit_effect_size", S3DSceneComposer::typeZoom,
			S3DSceneComposer::attrConstant,
			L"着弾効果拡大率" ) ) ;
	ESLVerify( paramDropSubComp == AddParameterEntry
		( L"drop_subcomp", S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrStringEnumeration,
			L"移動軌跡コンポジション" ) ) ;
	ESLVerify( paramDropMatrixOp == AddParameterEntry
		( L"drop_matrix", S3DSceneComposer::typeSelector,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrStringEnumeration
			| S3DSceneComposer::attrUIOnlyEnumeration,
			L"移動軌跡向き" ) ) ;
	ESLVerify( paramDropSize == AddParameterEntry
		( L"drop_size", S3DSceneComposer::typeZoom,
			S3DSceneComposer::attrConstant,
			L"移動軌跡拡大率" ) ) ;
	ESLVerify( paramDropInterval == AddParameterEntry
		( L"drop_interval", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant,
			L"移動軌跡間隔", L"移動軌跡に発生するコンポジションの時間間隔[秒]" ) ) ;
	ESLVerify( paramDropRndPos == AddParameterEntry
		( L"drop_rnd_pos", S3DSceneComposer::typePosition,
			S3DSceneComposer::attrConstant,
			L"移動軌跡位置乱数" ) ) ;
	ESLVerify( paramDropRndZoom == AddParameterEntry
		( L"drop_rnd_zoom", S3DSceneComposer::typeScalar,
			S3DSceneComposer::attrConstant
			| S3DSceneComposer::attrUIScalarSlider,
			L"移動軌跡拡大乱数",
			L"移動軌跡に発生するコンポジションの拡大率の乱数影響度", 0.0, 1.0 ) ) ;
	ESLVerify( paramDropRndAngle1 == AddParameterEntry
		( L"drop_rnd_angle1", S3DSceneComposer::typePosition,
			S3DSceneComposer::attrConstant,
			L"移動軌跡回転角乱数1", L"回転のランダム成分 [deg]" ) ) ;
	ESLVerify( paramDropRndAngle2 == AddParameterEntry
		( L"drop_rnd_angle2", S3DSceneComposer::typePosition,
			S3DSceneComposer::attrConstant,
			L"移動軌跡回転角乱数2", L"回転のランダム成分 [deg]" ) ) ;
}

S3DBulletSubCompositionController::Container::Container
	( S3DSubCompositionSerializer * pOwner,
			S3DSceneComposer::Composition * pComp )
	: m_refOwner( (S3DScene::Item*) ((S3DSceneComposer::ItemBasicSerializer*) pOwner) ),
		m_pComp( pComp ), m_nDropCount( 0 )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
S3DBulletSubCompositionController::~S3DBulletSubCompositionController( void )
{
}

S3DBulletSubCompositionController::Container::~Container( void )
{
	S3DSubCompositionSerializer *
			pOwner = m_refOwner.GetRef<S3DSubCompositionSerializer>() ;
	if ( pOwner != nullptr )
	{
		pOwner->ReleaseInstance( m_pComp ) ;
	}
}

// 参照コンポジション更新
//////////////////////////////////////////////////////////////////////////////
void S3DBulletSubCompositionController::UpdatreBulletSubCompRef( void )
{
	m_refBulletSubComp.SetReference( nullptr ) ;
	if ( m_strBulletSubComp.IsEmpty() )
	{
		return ;
	}
	S3DSceneComposer::Composition *	pComp = GetComposition() ;
	if ( pComp != nullptr )
	{
		m_refBulletSubComp.SetReference( pComp->GetSceneItemAs( m_strBulletSubComp ) ) ;
	}
}

void S3DBulletSubCompositionController::UpdatreFireEffectSubCompRef( void )
{
	m_refFireEffectSubComp.SetReference( nullptr ) ;
	if ( m_strFireEffectSubComp.IsEmpty() )
	{
		return ;
	}
	S3DSceneComposer::Composition *	pComp = GetComposition() ;
	if ( pComp != nullptr )
	{
		m_refFireEffectSubComp.SetReference( pComp->GetSceneItemAs( m_strFireEffectSubComp ) ) ;
	}
}

void S3DBulletSubCompositionController::UpdatreHitEffectSubCompRef( void )
{
	m_refHitEffectSubComp.SetReference( nullptr ) ;
	if ( m_strHitEffectSubComp.IsEmpty() )
	{
		return ;
	}
	S3DSceneComposer::Composition *	pComp = GetComposition() ;
	if ( pComp != nullptr )
	{
		m_refHitEffectSubComp.SetReference( pComp->GetSceneItemAs( m_strHitEffectSubComp ) ) ;
	}
}

void S3DBulletSubCompositionController::UpdatreDropSubCompRef( void )
{
	m_refDropSubComp.SetReference( nullptr ) ;
	if ( m_strDropSubComp.IsEmpty() )
	{
		return ;
	}
	S3DSceneComposer::Composition *	pComp = GetComposition() ;
	if ( pComp != nullptr )
	{
		m_refDropSubComp.SetReference( pComp->GetSceneItemAs( m_strDropSubComp ) ) ;
	}
}

// パラメータ値取得
//////////////////////////////////////////////////////////////////////////////
S3DDVector S3DBulletSubCompositionController::GetVectorParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramBulletSize:
		return	m_vBulletSize ;
	case	paramFireEffectSize:
		return	m_vFireEffectSize ;
	case	paramHitEffectSize:
		return	m_vHitEffectSize ;
	case	paramDropSize:
		return	m_vDropSize ;
	case	paramDropRndPos:
		return	m_vDropRndPos ;
	case	paramDropRndAngle1:
		return	m_vRndEulerAngles1 ;
	case	paramDropRndAngle2:
		return	m_vRndEulerAngles2 ;
	}
	return	S3DDVector( 0, 0, 0 ) ;
}

double S3DBulletSubCompositionController::GetScalarParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramDropInterval:
		return	m_secDropInterval ;
	case	paramDropRndZoom:
		return	m_fpZoomRndRate ;
	}
	return	0.0 ;
}

const wchar_t * S3DBulletSubCompositionController::GetCommandParameter( size_t i ) const
{
	switch ( i )
	{
	case	paramBulletSubComp:
		return	m_strBulletSubComp ;
	case	paramBulletMatrixOp:
		return	s_pwszMatrixOperation[m_matrixBullet] ;

	case	paramFireEffectSubComp:
		return	m_strFireEffectSubComp ;
	case	paramFireEffectMatrixOp:
		return	s_pwszMatrixOperation[m_matrixFireEffect] ;

	case	paramHitEffectSubComp:
		return	m_strHitEffectSubComp ;
	case	paramHitEffectMatrixOp:
		return	s_pwszMatrixOperation[m_matrixHitEffect] ;

	case	paramDropSubComp:
		return	m_strDropSubComp ;
	case	paramDropMatrixOp:
		return	s_pwszMatrixOperation[m_matrixDrop] ;
	}
	return	nullptr ;
}

// パラメータ値設定
//////////////////////////////////////////////////////////////////////////////
void S3DBulletSubCompositionController::SetVectorParameter( size_t i, const S3DDVector& vec )
{
	switch ( i )
	{
	case	paramBulletSize:
		m_vBulletSize = vec ;
		return ;
	case	paramHitEffectSize:
		m_vHitEffectSize = vec ;
		return ;
	case	paramDropSize:
		m_vDropSize = vec ;
		return ;
	case	paramDropRndPos:
		m_vDropRndPos = vec ;
		return ;
	case	paramDropRndAngle1:
		m_vRndEulerAngles1 = vec ;
		return ;
	case	paramDropRndAngle2:
		m_vRndEulerAngles2 = vec ;
		return ;
	}
}

void S3DBulletSubCompositionController::SetScalarParameter( size_t i, double s )
{
	switch ( i )
	{
	case	paramDropInterval:
		m_secDropInterval = s ;
		return ;
	case	paramDropRndZoom:
		m_fpZoomRndRate = s ;
		return ;
	}
}

void S3DBulletSubCompositionController::SetCommandParameter( size_t i, const wchar_t * pwszCmd )
{
	switch ( i )
	{
	case	paramBulletSubComp:
		if ( m_strBulletSubComp != pwszCmd )
		{
			m_strBulletSubComp = pwszCmd ;
			UpdatreBulletSubCompRef() ;
		}
		return ;

	case	paramBulletMatrixOp:
		m_matrixBullet = (MatrixOperation)
			SXMLDocument::GetIntegerAsSymbolOf
				( s_aiMatrixOperation, pwszCmd, m_matrixBullet ) ;
		return ;

	case	paramFireEffectSubComp:
		if ( m_strFireEffectSubComp != pwszCmd )
		{
			m_strFireEffectSubComp = pwszCmd ;
			UpdatreFireEffectSubCompRef() ;
		}
		return ;

	case	paramFireEffectMatrixOp:
		m_matrixFireEffect = (MatrixOperation)
			SXMLDocument::GetIntegerAsSymbolOf
				( s_aiMatrixOperation, pwszCmd, m_matrixFireEffect ) ;
		return ;

	case	paramHitEffectSubComp:
		if ( m_strHitEffectSubComp != pwszCmd )
		{
			m_strHitEffectSubComp = pwszCmd ;
			UpdatreHitEffectSubCompRef() ;
		}
		return ;

	case	paramHitEffectMatrixOp:
		m_matrixHitEffect = (MatrixOperation)
			SXMLDocument::GetIntegerAsSymbolOf
				( s_aiMatrixOperation, pwszCmd, m_matrixHitEffect ) ;
		return ;

	case	paramDropSubComp:
		if ( m_strDropSubComp != pwszCmd )
		{
			m_strDropSubComp = pwszCmd ;
			UpdatreDropSubCompRef() ;
		}
		return ;

	case	paramDropMatrixOp:
		m_matrixDrop = (MatrixOperation)
			SXMLDocument::GetIntegerAsSymbolOf
				( s_aiMatrixOperation, pwszCmd, m_matrixDrop ) ;
		return ;
	}
}

// パラメータ値域列挙
//////////////////////////////////////////////////////////////////////////////
bool S3DBulletSubCompositionController::EnumerateStringSet
	( size_t i, SSystem::SStringArray& aStrSet )
{
	if ( (i == paramBulletSubComp)
		|| (i == paramFireEffectSubComp)
		|| (i == paramHitEffectSubComp)
		|| (i == paramDropSubComp) )
	{
		S3DSceneComposer::Composition *	pComp = GetComposition() ;
		if ( pComp != NULL )
		{
			pComp->EnumerateItemIDsAs
				( aStrSet, ESL_RUNTIME_CLASS(S3DSubCompositionSerializer) ) ;
		}
		return	true ;
	}
	if ( (i == paramBulletMatrixOp)
		|| (i == paramFireEffectMatrixOp)
		|| (i == paramHitEffectMatrixOp)
		|| (i == paramDropMatrixOp) )
	{
		for ( int i = 0; i < matrixOpCount; i ++ )
		{
			aStrSet.Add( new SString( s_pwszMatrixOperation[i] ) ) ;
		}
		return	true ;
	}
	return	false ;
}

// アイテムプロパティのリソース等の参照を更新する
//////////////////////////////////////////////////////////////////////////////
uint32_t S3DBulletSubCompositionController::UpdatePropertyReference
	( S3DSceneComposer::Composition& comp,
		S3DSceneComposer::ItemSerializer * pItem, uint32_t nFlags )
{
	uint32_t	nResFlags =
		BulletController::UpdatePropertyReference( comp, pItem, nFlags ) ;
	//
	if ( nFlags & S3DSceneComposer::updateRefItem )
	{
		UpdatreBulletSubCompRef() ;
		UpdatreFireEffectSubCompRef() ;
		UpdatreHitEffectSubCompRef() ;
		UpdatreDropSubCompRef() ;
	}
	return	nResFlags ;
}

// 発射時処理
//////////////////////////////////////////////////////////////////////////////
void S3DBulletSubCompositionController::OnFireBullet
	( S3DBulletItemInterface * pItem,
		S3DBulletItemInterface::Bullet& bullet )
{
	S3DDMatrix	matdItem( 1, 1, 1 ) ;
	S3DDVector	vItem( 0, 0, 0 ) ;
	if ( pItem->GetBulletFlags() & S3DBulletItemInterface::flagTransform )
	{
		pItem->GetGlobalTransformation( matdItem, vItem ) ;
	}
	S3DSubCompositionSerializer *	pSubComp =
			m_refFireEffectSubComp.GetRef<S3DSubCompositionSerializer>() ;
	if ( pSubComp != nullptr )
	{
		//
		// 発射エフェクト
		//
		S3DSceneComposer::Composition *	pInstance = pSubComp->CreateInstance() ;
		ReflectSubCompositionMatrix
			( pInstance, *pSubComp, bullet,
				matdItem, vItem, m_matrixFireEffect, m_vFireEffectSize ) ;
		//
		m_csEffects.Lock() ;
		m_aFireEffects.Add( pInstance ) ;
		m_csEffects.Unlock() ;
	}
	pSubComp = m_refBulletSubComp.GetRef<S3DSubCompositionSerializer>() ;
	if ( pSubComp != nullptr )
	{
		//
		// 弾頭設定
		//
		S3DSceneComposer::Composition *	pInstance = pSubComp->CreateInstance() ;
		ReflectSubCompositionMatrix
			( pInstance, *pSubComp, bullet,
				matdItem, vItem, m_matrixBullet, m_vBulletSize ) ;
		//
		S3DBulletItemInterface::AddBulletInstance
			( bullet, new Container( pSubComp, pInstance ) ) ;
	}
	else
	{
		S3DSubCompositionSerializer *	pDropSubComp =
				m_refDropSubComp.GetRef<S3DSubCompositionSerializer>() ;
		if ( pDropSubComp != nullptr )
		{
			S3DBulletItemInterface::AddBulletInstance
				( bullet, new Container( nullptr, nullptr ) ) ;
		}
	}
}

// 着弾時処理
//////////////////////////////////////////////////////////////////////////////
void S3DBulletSubCompositionController::OnHitBullet
	( S3DBulletItemInterface * pItem,
		S3DSceneComposer::ItemSerializer * pHitItem,
		const S3DCollision::MeshCollision * pmcHitMesh,
		const S3DVector& vHitPos, const S3DVector& vHitNormal,
		S3DBulletItemInterface::Bullet& bullet, size_t iHitTrack )
{
	S3DSubCompositionSerializer *	pSubComp =
			m_refHitEffectSubComp.GetRef<S3DSubCompositionSerializer>() ;
	if ( pSubComp != nullptr )
	{
		S3DDMatrix	matdItem( 1, 1, 1 ) ;
		S3DDVector	vItem( 0, 0, 0 ) ;
		if ( pItem->GetBulletFlags() & S3DBulletItemInterface::flagTransform )
		{
			pItem->GetGlobalTransformation( matdItem, vItem ) ;
		}
		//
		// 着弾エフェクト
		//
		S3DSceneComposer::Composition *	pInstance = pSubComp->CreateInstance() ;
		ReflectSubCompositionMatrix
			( pInstance, *pSubComp, bullet,
				matdItem, vItem, m_matrixHitEffect, m_vHitEffectSize ) ;
		//
		m_csEffects.Lock() ;
		m_aHitEffects.Add( pInstance ) ;
		m_csEffects.Unlock() ;
	}
}

// タイマー前処理
//////////////////////////////////////////////////////////////////////////////
void S3DBulletSubCompositionController::BeforeBulletTimer
	( S3DBulletItemInterface * pItem, float32_t secPast,
			S3DBulletItemInterface::Bullet*const* pBullets, size_t nCount )
{
}

// タイマー処理
//////////////////////////////////////////////////////////////////////////////
void S3DBulletSubCompositionController::OnBulletTimer
	( S3DBulletItemInterface * pItem, float32_t secPast,
		S3DBulletItemInterface::Bullet*const* pBullets, size_t nCount )
{
	SSmartLock<SCriticalSection>	lock( &m_csEffects ) ;

	S3DSubCompositionSerializer *	pDropSubComp =
			m_refDropSubComp.GetRef<S3DSubCompositionSerializer>() ;

	S3DDMatrix	matdItem( 1, 1, 1 ) ;
	S3DDVector	vItem( 0, 0, 0 ) ;
	if ( pItem->GetBulletFlags() & S3DBulletItemInterface::flagTransform )
	{
		pItem->GetGlobalTransformation( matdItem, vItem ) ;
	}
	for ( size_t i = 0; i < nCount; i ++ )
	{
		if ( pBullets[i] == nullptr )
		{
			continue ;
		}
		S3DBulletItemInterface::Bullet&	bullet = *(pBullets[i]) ;
		Container *	pContainer =
			ESLTypeCast<Container>
				( S3DBulletItemInterface::GetBulletInstance
					( bullet, ESL_RUNTIME_CLASS(Container) ) ) ;
		if ( pContainer == nullptr )
		{
			continue ;
		}
		if ( pDropSubComp != nullptr )
		{
			if ( bullet.secLife >= pContainer->m_nDropCount * m_secDropInterval )
			{
				S3DSceneComposer::Composition *	pInstance = pDropSubComp->CreateInstance() ;
				ReflectSubCompositionMatrix
					( pInstance, *pDropSubComp, bullet,
						matdItem, vItem, m_matrixDrop, m_vDropSize ) ;
				//
				S3DDMatrix	matLocal ;
				S3DDVector	vLocal ;
				pInstance->GetLocalSpacePosition( vLocal ) ;
				pInstance->GetLocalTransformation( matLocal ) ;
				//
				S3DDVector	vDropPos( m_randomizer.QuickRandomDouble(m_vDropSize.x),
										m_randomizer.QuickRandomDouble(m_vDropSize.y),
										m_randomizer.QuickRandomDouble(m_vDropSize.z) ) ;
				pInstance->SetLocalSpacePosition( matLocal * vDropPos + vLocal ) ;
				//
				S3DDVector	vRadAngle1 = m_vRndEulerAngles1 * (PI / 180.0) ;
				S3DDVector	vRadAngle2 = m_vRndEulerAngles2 * (PI / 180.0) ;
				vRadAngle1.x = m_randomizer.QuickRandomDouble( vRadAngle1.x ) ;
				vRadAngle1.y = m_randomizer.QuickRandomDouble( vRadAngle1.y ) ;
				vRadAngle1.z = m_randomizer.QuickRandomDouble( vRadAngle1.z ) ;
				vRadAngle2.x = m_randomizer.QuickRandomDouble( vRadAngle2.x ) ;
				vRadAngle2.y = m_randomizer.QuickRandomDouble( vRadAngle2.y ) ;
				vRadAngle2.z = m_randomizer.QuickRandomDouble( vRadAngle2.z ) ;
				//
				S3DDMatrix	matRndMatrix( 1, 1, 1 ) ;
				matRndMatrix.RevolveOnZ( sin(vRadAngle2.z), cos(vRadAngle2.z) ) ;
				matRndMatrix.RevolveOnY( sin(vRadAngle2.y), cos(vRadAngle2.y) ) ;
				matRndMatrix.RevolveOnX( sin(vRadAngle2.x), cos(vRadAngle2.x) ) ;
				matRndMatrix.RevolveOnZ( sin(vRadAngle1.z), cos(vRadAngle1.z) ) ;
				matRndMatrix.RevolveOnY( sin(vRadAngle1.y), cos(vRadAngle1.y) ) ;
				matRndMatrix.RevolveOnX( sin(vRadAngle1.x), cos(vRadAngle1.x) ) ;
				//
				double	fpZoom = 1.0 + m_randomizer.QuickRandomDouble(m_fpZoomRndRate ) ;
				matRndMatrix.MagnifyByVector( S3DDVector( fpZoom, fpZoom, fpZoom ) ) ;
				//
				pInstance->SetLocalTransformation( matLocal * matRndMatrix ) ;
				//
				m_csEffects.Lock() ;
				m_aDropEffects.Add( pInstance ) ;
				m_csEffects.Unlock() ;
				pContainer->m_nDropCount ++ ;
			}
		}
		if ( (pContainer->m_pComp == nullptr)
			|| (pContainer->m_refOwner.GetReference() == nullptr) )
		{
			continue ;
		}
		ReflectSubCompositionMatrix
			( pContainer->m_pComp,
				*(pContainer->m_refOwner.GetRef<S3DSubCompositionSerializer>()),
				bullet, matdItem, vItem, m_matrixBullet, m_vBulletSize ) ;
	}

	CleanupSubCompositions
		( m_refFireEffectSubComp.GetRef<S3DSubCompositionSerializer>(), m_aFireEffects ) ;

	CleanupSubCompositions
		( m_refHitEffectSubComp.GetRef<S3DSubCompositionSerializer>(), m_aHitEffects ) ;

	CleanupSubCompositions( pDropSubComp, m_aDropEffects ) ;
}

// 当たり判定追加処理
//////////////////////////////////////////////////////////////////////////////
void S3DBulletSubCompositionController::RenderBulletCollision
	( const S3DScene& scene,
		S3DCollision& collision,
		const S3DBulletItemInterface * pItem,
		S3DBulletItemInterface::Bullet*const* pBullets, size_t nCount )
{
}

// 弾軌跡描画（追加処理）
//////////////////////////////////////////////////////////////////////////////
void S3DBulletSubCompositionController::RenderBulletTracks
	( S3DScene& scene, const S3DBulletItemInterface * pItem,
		const S3DBulletItemInterface::Bullet*const* pBullets, size_t nCount )
{
}

// インスタンス・クリーンアップ
//////////////////////////////////////////////////////////////////////////////
void S3DBulletSubCompositionController::CleanupSubCompositions
	( S3DSubCompositionSerializer * pSubComp,
		SSystem::SPointerArray<S3DSceneComposer::Composition>& aEffects )
{
	if ( pSubComp != nullptr )
	{
		for ( size_t i = 0; i < aEffects.GetLength(); i ++ )
		{
			S3DSceneComposer::Composition *	pComp = aEffects.GetAt( i ) ;
			if ( (pComp != nullptr) && !pComp->IsPlayingComposition() )
			{
				ESLAssert( ESLTypeCast<S3DSubCompositionSerializer>( pComp->GetOwnerItem() ) == pSubComp ) ;
				pSubComp->ReleaseInstance( pComp ) ;
				aEffects.SetAt( i, nullptr ) ;
			}
		}
		aEffects.TrimEmpty() ;
	}
	else
	{
		aEffects.RemoveAll() ;
	}
}

// サブコンポジション位置設定
//////////////////////////////////////////////////////////////////////////////
void S3DBulletSubCompositionController::ReflectSubCompositionMatrix
	( S3DSceneComposer::Composition * pInstance,
		const S3DSubCompositionSerializer& subcomp,
		const S3DBulletItemInterface::Bullet& bullet,
		const S3DDMatrix& matBase, const S3DDVector& vBase,
		S3DBulletSubCompositionController::MatrixOperation matrixOp,
		const S3DDVector& vSize )
{
	S3DDMatrix	matSubComp ;
	S3DDVector	vSubComp ;
	subcomp.GetGlobalTransformation( matSubComp, vSubComp ) ;
	//
	S3DDMatrix	matISubComp = matSubComp.Inverse() ;
	S3DDMatrix	matTrans = matISubComp * matBase ;
	S3DDVector	vTrans = matISubComp * vBase - vSubComp ;
	//
	S3DDMatrix	matBulletDir( 1, 1, 1 ) ;
	if ( matrixOp == matrixDirection )
	{
		matBulletDir.RevolveForAngle( bullet.vSpeed ) ;
	}
	matBulletDir.MagnifyByVector( vSize * bullet.fpThickness ) ;
	//
	pInstance->SetLocalSpacePosition( matTrans * bullet.vPos + vTrans ) ;
	pInstance->SetLocalTransformation( matTrans * matBulletDir ) ;
}
