
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_SceneBulleteItem_Bullet.h>


// Object getUserObject( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_SceneBulleteItem_Bullet_getUserObject)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SceneBulleteItem_Bullet, pThis ) ;

	SceneBulleteItemBulletInstance *
			pBullet = pThis->GetRef<SceneBulleteItemBulletInstance>() ;
	LQT_VERIFY_NULL_PTR( pBullet ) ;
	if ( pBullet->m_pBullet == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}

	LObjPtr	valRet( LObject::AddRef( pBullet->m_pBullet->pUserLObj ) ) ;
	LQT_RETURN_OBJECT( valRet ) ;
}



