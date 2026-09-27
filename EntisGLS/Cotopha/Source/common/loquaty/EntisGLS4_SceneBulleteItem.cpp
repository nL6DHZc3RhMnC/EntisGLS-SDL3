
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_SceneBulleteItem.h>


// SceneBulleteItem( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_SceneBulleteItem)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ( LEntisGLS4_SceneBulleteItem, pThis, () ) ;

	LQT_RETURN_VOID() ;
}

// EntisGLS4.SceneBulleteItem.Bullet generateBullet( const Vector3d* vPos, const Vector3d* vSpeed, float thickness, Object objUserInstance )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneBulleteItem_generateBullet)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneBulleteItem, pThis ) ;
	S3DBulletItemSerializer *	pBulletItem = pThis->GetRef<S3DBulletItemSerializer>() ;
	LQT_VERIFY_NULL_PTR( pBulletItem ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vPos ) ;
	LQT_VERIFY_NULL_PTR( vPos ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vSpeed ) ;
	LQT_VERIFY_NULL_PTR( vSpeed ) ;
	LQT_FUNC_ARG_FLOAT( thickness ) ;
	LQT_FUNC_ARG_OBJECT( LObject, objUserInstance ) ;

	S3DBulletItemInterface::Bullet *	pBullet =
		pBulletItem->GenerateBullet
			( *vPos, *vSpeed, thickness, nullptr, objUserInstance.Get() ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.SceneBulleteItem.Bullet) ) ) ;
	valRet->SetNative
		( std::make_shared<LEntisGLS4_SceneBulleteItem_Bullet>
				( new SceneBulleteItemBulletInstance( pBullet) ) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// void removeAllBullets( )
IMPL_LOQUATY_FUNC(EntisGLS4_SceneBulleteItem_removeAllBullets)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneBulleteItem, pThis ) ;
	S3DBulletItemSerializer *	pBulletItem = pThis->GetRef<S3DBulletItemSerializer>() ;
	LQT_VERIFY_NULL_PTR( pBulletItem ) ;

	pBulletItem->DeleteAllBullets() ;

	LQT_RETURN_VOID() ;
}

// boolean isValidBullet( EntisGLS4.SceneBulleteItem.Bullet bullet ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneBulleteItem_isValidBullet)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneBulleteItem, pThis ) ;
	S3DBulletItemSerializer *	pBulletItem = pThis->GetRef<S3DBulletItemSerializer>() ;
	LQT_VERIFY_NULL_PTR( pBulletItem ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SceneBulleteItem_Bullet, bullet ) ;
	LQT_VERIFY_NULL_PTR( bullet ) ;
	SceneBulleteItemBulletInstance *
			pBullet = bullet->GetRef<SceneBulleteItemBulletInstance>() ;
	if ( pBullet == nullptr )
	{
		LQT_RETURN_BOOL( false ) ;
	}

	LQT_RETURN_BOOL( pBulletItem->IsValidBullet( pBullet->m_pBullet ) ) ;
}



