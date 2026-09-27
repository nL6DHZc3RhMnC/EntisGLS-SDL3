
#include <loquaty/gls4_loquaty.h>
#include <loquaty/EntisGLS4_Sprite.h>


// Sprite( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_Sprite)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ
		( LEntisGLS4_Sprite, pThis, ( new SSmartObject( new SGLSprite ) ) ) ;

	LQT_RETURN_VOID() ;
}

// EntisGLS4.Sprite duplicate( )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_duplicate)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	SGLSprite *	pDup = ESLSmartCast<SGLSprite>( pSprite->DuplicateObject() ) ;
	if ( pDup == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}

	LPtr<LNativeObj>	valRet( new LNativeObj( GetSpriteClass( _context.VM(), pDup ) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_Sprite>( new SSmartObject( pDup ) ) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// const EntisGLS4.Sprite.Parameter* getParameter( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	const SGLSprite::Parameter&	param = pSprite->GetParameter() ;
	std::shared_ptr<LArrayBufAlias>	buf =
		std::make_shared<LArrayBufAlias>
			( (uint8_t*) &param, sizeof(LEntisGLS4_Sprite_Parameter) ) ;

	LQT_RETURN_POINTER_BUF( buf ) ;
}

// uint getDrawParamFlags( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getDrawParamFlags)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_UINT( pSprite->GetDrawParamFlags() ) ;
}

// uint getSpriteFlags( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getSpriteFlags)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_UINT( pSprite->GetSpriteFlags() ) ;
}

// const Vector3d* getPosition( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getPosition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	std::shared_ptr<LArrayBufAlias>	buf =
		std::make_shared<LArrayBufAlias>
			( (uint8_t*) &(pSprite->GetPosition()), sizeof(LVector3d) ) ;

	LQT_RETURN_POINTER_BUF( buf ) ;
}

// const Vector2d* getPosition2D( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getPosition2D)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LVector2d	valRet = pSprite->GetPosition2D() ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// const Vector2d* getCenterPosition( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getCenterPosition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	std::shared_ptr<LArrayBufAlias>	buf =
		std::make_shared<LArrayBufAlias>
			( (uint8_t*) &(pSprite->GetCenterPosition()), sizeof(LVector2d) ) ;

	LQT_RETURN_POINTER_BUF( buf ) ;
}

// const Vector2d* getImageCenter( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getImageCenter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	std::shared_ptr<LArrayBufAlias>	buf =
		std::make_shared<LArrayBufAlias>
			( (uint8_t*) &(pSprite->GetImageCenter()), sizeof(LVector2d) ) ;

	LQT_RETURN_POINTER_BUF( buf ) ;
}

// const Vector2d* getZoom( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getZoom)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	std::shared_ptr<LArrayBufAlias>	buf =
		std::make_shared<LArrayBufAlias>
			( (uint8_t*) &(pSprite->GetZoom()), sizeof(LVector2d) ) ;

	LQT_RETURN_POINTER_BUF( buf ) ;
}

// double getRotation( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getRotation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_DOUBLE( pSprite->GetRotation() ) ;
}

// uint getTransparency( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getTransparency)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_UINT( pSprite->GetTransparency() ) ;
}

// int getFilterParameter( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getFilterParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_INT( pSprite->GetFilterParameter() ) ;
}

// int getFilter2Parameter( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getFilter2Parameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_INT( pSprite->GetFilter2Parameter() ) ;
}

// void setParameter( const EntisGLS4.Sprite.Parameter* param )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_Sprite_Parameter, param ) ;
	LQT_VERIFY_NULL_PTR( param ) ;

	SGLSprite::Parameter	paramTemp = pSprite->GetParameter() ;
	param->ToSGLSpriteParameter( paramTemp ) ;
	pSprite->SetParameter( paramTemp ) ;

	LQT_RETURN_VOID() ;
}

// void modifyDrawFlags( uint nAddFlags, uint nRemoveFlags )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_modifyDrawFlags)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_UINT( nAddFlags ) ;
	LQT_FUNC_ARG_UINT( nRemoveFlags ) ;

	pSprite->ModifyDrawFlags( nAddFlags, nRemoveFlags ) ;

	LQT_RETURN_VOID() ;
}

// void modifySpriteFlags( uint nAddFlags, uint nRemoveFlags )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_modifySpriteFlags)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_UINT( nAddFlags ) ;
	LQT_FUNC_ARG_UINT( nRemoveFlags ) ;

	pSprite->ModifySpriteFlags( nAddFlags, nRemoveFlags ) ;

	LQT_RETURN_VOID() ;
}

// void setPosition( double x, double y )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setPosition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_DOUBLE( x ) ;
	LQT_FUNC_ARG_DOUBLE( y ) ;

	pSprite->SetPosition( x, y ) ;

	LQT_RETURN_VOID() ;
}

// void setPosition3D( double x, double y, double z )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setPosition3D)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_DOUBLE( x ) ;
	LQT_FUNC_ARG_DOUBLE( y ) ;
	LQT_FUNC_ARG_DOUBLE( z ) ;

	pSprite->SetPosition3D( S3DDVector( x, y, z ) ) ;

	LQT_RETURN_VOID() ;
}

// void setPosition3D( const Vector3d* vPos )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setPosition3D_1)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vPos ) ;
	LQT_VERIFY_NULL_PTR( vPos ) ;

	pSprite->SetPosition3D( *vPos ) ;

	LQT_RETURN_VOID() ;
}

// void setCenterPosition( double x, double y )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setCenterPosition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_DOUBLE( x ) ;
	LQT_FUNC_ARG_DOUBLE( y ) ;

	pSprite->SetCenterPosition( x, y ) ;

	LQT_RETURN_VOID() ;
}

// void setZoom( double x, double y )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setZoom)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_DOUBLE( x ) ;
	LQT_FUNC_ARG_DOUBLE( y ) ;

	pSprite->SetZoom( x, y ) ;

	LQT_RETURN_VOID() ;
}

// void setRotation( double zAngle )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setRotation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_DOUBLE( zAngle ) ;

	pSprite->SetRotation( zAngle ) ;

	LQT_RETURN_VOID() ;
}

// void setTransparency( uint transparency )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setTransparency)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_UINT( transparency ) ;

	pSprite->SetTransparency( transparency ) ;

	LQT_RETURN_VOID() ;
}

// void setFilterParameter( int paramFilter )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setFilterParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_INT( paramFilter ) ;

	pSprite->SetFilterParameter( paramFilter ) ;

	LQT_RETURN_VOID() ;
}

// void setFilter2Parameter( int paramFilter )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setFilter2Parameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_INT( paramFilter ) ;

	pSprite->SetFilter2Parameter( paramFilter ) ;

	LQT_RETURN_VOID() ;
}

// ulong getUIFlag( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getUIFlag)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_ULONG( pSprite->GetUIFlag() ) ;
}

// ulong modifyUIFlag( ulong nAddFlags, ulong nRemoveFlags )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_modifyUIFlag)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_ULONG( nAddFlags ) ;
	LQT_FUNC_ARG_ULONG( nRemoveFlags ) ;

	LQT_RETURN_ULONG( pSprite->ModifyUIFlag( nAddFlags, nRemoveFlags ) ) ;
}

// const ImageRect* getClickableRect( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getClickableRect)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LImageRect	valRet = pSprite->GetClickableRect() ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// void setClickableRect( const ImageRect* rect )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setClickableRect)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, rect ) ;
	LQT_VERIFY_NULL_PTR( rect ) ;

	pSprite->SetClickableRect( *rect ) ;

	LQT_RETURN_VOID() ;
}

// boolean isVisible( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_isVisible)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_BOOL( pSprite->IsVisible() ) ;
}

// void setVisible( boolean visible )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setVisible)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_BOOL( visible ) ;

	pSprite->SetVisible( visible ) ;

	LQT_RETURN_VOID() ;
}

// int getPriority( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getPriority)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_INT( pSprite->GetPriority() ) ;
}

// void changePriority( int priority )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_changePriority)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_INT( priority ) ;

	pSprite->ChangePriority( priority ) ;

	LQT_RETURN_VOID() ;
}

// String getID( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getID)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_STRING( pSprite->GetID() ) ;
}

// void setID( String id )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setID)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_STRING( id ) ;

	pSprite->SetID( id.c_str() ) ;

	LQT_RETURN_VOID() ;
}

// boolean getRectangle( ImageRect* rectExt ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getRectangle)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, rectExt ) ;
	LQT_VERIFY_NULL_PTR( rectExt ) ;

	SGLRect		rect ;
	LBoolean	valRet = pSprite->GetRectangle( rect ) ;
	if ( valRet )
	{
		*rectExt = rect ;
	}
	LQT_RETURN_BOOL( valRet ) ;
}

// boolean localToGlobal( Vector2d* vPos ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_localToGlobal)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_STRUCT( LVector2d, vPos ) ;
	LQT_VERIFY_NULL_PTR( vPos ) ;

	LQT_RETURN_BOOL( pSprite->LocalToGlobal( *vPos ) ) ;
}

// boolean globalToLocal( Vector2d* vPos ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_globalToLocal)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_STRUCT( LVector2d, vPos ) ;
	LQT_VERIFY_NULL_PTR( vPos ) ;

	LQT_RETURN_BOOL( pSprite->GlobalToLocal( *vPos ) ) ;
}

// void postUpdate( ImageRect* pUpdate )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_postUpdate)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, pUpdate ) ;

	if ( pUpdate != nullptr )
	{
		SGLRect	rect = *pUpdate ;
		pSprite->PostUpdate( &rect ) ;
	}
	else
	{
		pSprite->PostUpdate( nullptr ) ;
	}

	LQT_RETURN_VOID() ;
}

// boolean hasUpdate( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_hasUpdate)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_BOOL( pSprite->HasUpdate() ) ;
}

// void freezeFrameUpdate( )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_freezeFrameUpdate)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	pSprite->FreezeFrameUpdate() ;

	LQT_RETURN_VOID() ;
}

// boolean defrostFrameUpdate( )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_defrostFrameUpdate)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_BOOL( pSprite->DefrostFrameUpdate() ) ;
}

// void addChild( EntisGLS4.Sprite sprite )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_addChild)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pThisSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pThisSprite ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Sprite, sprite ) ;
	LQT_VERIFY_NULL_PTR( sprite ) ;
	SGLSprite *	pAddSprite = sprite->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pAddSprite ) ;

	pThisSprite->AddChild( pAddSprite ) ;

	LQT_RETURN_VOID() ;
}

// boolean removeChild( EntisGLS4.Sprite sprite )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_removeChild)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pThisSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pThisSprite ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Sprite, sprite ) ;
	LQT_VERIFY_NULL_PTR( sprite ) ;
	SGLSprite *	pRemoveSprite = sprite->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pRemoveSprite ) ;

	LQT_RETURN_BOOL( pThisSprite->RemoveChild( pRemoveSprite ) ) ;
}

// void removeAllChildren( )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_removeAllChildren)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	pSprite->RemoveAllChildren() ;

	LQT_RETURN_VOID() ;
}

// ulong getChildCount( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getChildCount)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_ULONG( pSprite->GetChildCount() ) ;
}

// EntisGLS4.Sprite getChildAt( ulong index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getChildAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	SGLSprite *	pChild = pSprite->GetChildAt( (size_t) index ) ;
	if ( pChild == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( GetSpriteClass( _context.VM(), pChild ) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_Sprite>(pChild) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.Sprite getParent( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getParent)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	SGLSprite *	pParent = pSprite->GetParent() ;
	if ( pParent == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( GetSpriteClass( _context.VM(), pParent ) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_Sprite>(pParent) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.Sprite getItemAs( String id ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getItemAs)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_STRING( id ) ;

	SGLSprite *	pChild = pSprite->GetItemAs( id.c_str() ) ;
	if ( pChild == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( GetSpriteClass( _context.VM(), pChild ) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_Sprite>(pChild) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.Sprite getHitSpriteAt( Vector2d* vPos ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getHitSpriteAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_STRUCT( LVector2d, vPos ) ;
	LQT_VERIFY_NULL_PTR( vPos ) ;

	SGLSprite *	pChild = pSprite->GetHitSpriteAt( *vPos ) ;
	if ( pChild == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( GetSpriteClass( _context.VM(), pChild ) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_Sprite>(pChild) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// boolean isHitSprite( double x, double y ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_isHitSprite)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_DOUBLE( x ) ;
	LQT_FUNC_ARG_DOUBLE( y ) ;

	LQT_RETURN_BOOL( pSprite->IsHitSprite( x, y ) ) ;
}

// boolean loadImage( String path )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_loadImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_STRING( path ) ;

	LQT_RETURN_BOOL( pSprite->LoadImage( path.c_str() ) == sglErrSuccess ) ;
}

// void attachAnimation( EntisGLS4.Image image, const ImageRect* pClip )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_attachAnimation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, image ) ;
	LQT_VERIFY_NULL_PTR( image ) ;
	SGLImageObject *	pImage = image->GetRef<SGLImageObject>() ;
	LQT_VERIFY_NULL_PTR( pImage ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, pClip ) ;
	LQT_VERIFY_NULL_PTR( pClip ) ;

	pSprite->AttachAnimation( pImage, pClip ) ;

	LQT_RETURN_VOID() ;
}

// void beginAnimation( long countLoop, ulong iLoopStart, ulong iLoopEnd, ulong iStartFrame, ulong msecDuration )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_beginAnimation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_LONG( countLoop ) ;
	LQT_FUNC_ARG_ULONG( iLoopStart ) ;
	LQT_FUNC_ARG_ULONG( iLoopEnd ) ;
	LQT_FUNC_ARG_ULONG( iStartFrame ) ;
	LQT_FUNC_ARG_ULONG( msecDuration ) ;

	pSprite->BeginAnimation
		( (ssize_t) countLoop,
			(size_t) iLoopStart, (size_t) iLoopEnd,
			(size_t) iStartFrame, (size_t) msecDuration ) ;

	LQT_RETURN_VOID() ;
}

// void setLoopAnimation( long countLoop, ulong iLoopStart, ulong iLoopEnd )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setLoopAnimation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_LONG( countLoop ) ;
	LQT_FUNC_ARG_ULONG( iLoopStart ) ;
	LQT_FUNC_ARG_ULONG( iLoopEnd ) ;

	pSprite->SetLoopAnimation
		( (ssize_t) countLoop, (size_t) iLoopStart, (size_t) iLoopEnd ) ;

	LQT_RETURN_VOID() ;
}

// void attachImage( EntisGLS4.Image image, EntisGLS4.Image leftImage )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_attachImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, image ) ;
	SGLImageObject *	pImage = nullptr ;
	if ( image != nullptr )
	{
		pImage = image->GetRef<SGLImageObject>() ;
	}
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, leftImage ) ;
	SGLImageObject *	pLeftImage = nullptr ;
	if ( leftImage != nullptr )
	{
		pLeftImage = leftImage->GetRef<SGLImageObject>() ;
	}

	pSprite->AttachImage( pImage, pLeftImage ) ;

	LQT_RETURN_VOID() ;
}

// EntisGLS4.Image getAttachedImage( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getAttachedImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	SGLImageObject *	pImage = pSprite->GetAttachedImage() ;
	if ( pImage == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.Image) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_Image>(pImage) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.Image getAttachedLeftImage( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getAttachedLeftImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	SGLImageObject *	pImage = pSprite->GetAttachedLeftImage() ;
	if ( pImage == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.Image) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_Image>(pImage) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// Size* getImageSize( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getImageSize)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LSize	valRet = pSprite->GetImageSize() ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// boolean getImageInfo( EntisGLS4.ImageInfo* imginf ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getImageInfo)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_ImageInfo, imginf ) ;
	LQT_VERIFY_NULL_PTR( imginf ) ;

	LQT_RETURN_BOOL( pSprite->GetImageInfo( *imginf ) == sglErrSuccess ) ;
}

// boolean createBuffer( uint width, uint height, uint format, uint depth, ulong nBufFlags, boolean flagZBuffer, boolean flagStereo3D )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_createBuffer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_UINT( width ) ;
	LQT_FUNC_ARG_UINT( height ) ;
	LQT_FUNC_ARG_UINT( format ) ;
	LQT_FUNC_ARG_UINT( depth ) ;
	LQT_FUNC_ARG_ULONG( nBufFlags ) ;
	LQT_FUNC_ARG_BOOL( flagZBuffer ) ;
	LQT_FUNC_ARG_BOOL( flagStereo3D ) ;

	LQT_RETURN_BOOL
		( pSprite->CreateBuffer
			( width, height, format, depth,
				nBufFlags, flagZBuffer, flagStereo3D ) == sglErrSuccess ) ;
}

// void releaseBuffer( )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_releaseBuffer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	pSprite->ReleaseBuffer() ;

	LQT_RETURN_VOID() ;
}

// boolean isBuffered( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_isBuffered)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_BOOL( pSprite->IsBuffered() ) ;
}

// boolean setRenderDevice( EntisGLS4.RenderDevice device )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setRenderDevice)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_RenderDevice, device ) ;
	LQT_VERIFY_NULL_PTR( device ) ;
	S3DRenderDevice *	pDevice = device->GetRef<S3DRenderDevice>() ;
	LQT_VERIFY_NULL_PTR( pDevice ) ;

	LQT_RETURN_BOOL( pSprite->SetRenderDevice( pDevice ) == sglErrSuccess ) ;
}

// EntisGLS4.RenderDevice getBufferRenderDevice( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getBufferRenderDevice)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	S3DRenderDevice *	pDevice = pSprite->GetBufferRenderDevice() ;
	if ( pDevice == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.RenderDevice) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_RenderDevice>(pDevice) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// boolean getFillBackColor( uint* argbFill ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getFillBackColor)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_POINTER( LUint32, argbFill ) ;
	LQT_VERIFY_NULL_PTR( argbFill ) ;

	LQT_RETURN_BOOL( pSprite->GetFillBackColor( *argbFill ) ) ;
}

// boolean setFillBackColor( uint argbFill, boolean flagFillBack )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setFillBackColor)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_UINT( argbFill ) ;
	LQT_FUNC_ARG_BOOL( flagFillBack ) ;

	LQT_RETURN_BOOL
		( pSprite->SetFillBackColor( argbFill, flagFillBack ) == sglErrSuccess ) ;
}

// void addFilter( EntisGLS4.SpriteFilter filter )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_addFilter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SpriteFilter, filter ) ;
	LQT_VERIFY_NULL_PTR( filter ) ;
	SGLSpriteFilter *	pFilter = filter->GetRef<SGLSpriteFilter>() ;
	LQT_VERIFY_NULL_PTR( pFilter ) ;

	pSprite->AddReferenceFilter( pFilter ) ;

	LQT_RETURN_VOID() ;
}

// void removeFilter( EntisGLS4.SpriteFilter filter )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_removeFilter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_SpriteFilter, filter ) ;
	LQT_VERIFY_NULL_PTR( filter ) ;
	SGLSpriteFilter *	pFilter = filter->GetRef<SGLSpriteFilter>() ;
	LQT_VERIFY_NULL_PTR( pFilter ) ;

	pSprite->RemoveFilter( pFilter ) ;

	LQT_RETURN_VOID() ;
}

// void removeAllFilters( )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_removeAllFilters)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	pSprite->RemoveAllFilter() ;

	LQT_RETURN_VOID() ;
}

// String getText( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getText)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_STRING( pSprite->GetText() ) ;
}

// void setText( String text )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setText)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_STRING( text ) ;

	pSprite->SetText( text.c_str() ) ;

	LQT_RETURN_VOID() ;
}

// void setTextFont( String font, int size )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setTextFont)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_STRING( font ) ;
	LQT_FUNC_ARG_INT( size ) ;

	pSprite->SetTextFont( font.c_str(), size ) ;

	LQT_RETURN_VOID() ;
}

// boolean isEnabled( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_isEnabled)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_BOOL( pSprite->IsEnabled() ) ;
}

// void setEnable( boolean flagEnable )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setEnable)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_BOOL( flagEnable ) ;

	pSprite->SetEnable( flagEnable ) ;

	LQT_RETURN_VOID() ;
}

// int getScrollPos( int scrlDir )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getScrollPos)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_INT( scrlDir ) ;

	LQT_RETURN_INT( pSprite->GetScrollPos( (SGLSprite::ScrollDirection) scrlDir ) ) ;
}

// void setScrollPos( int nPos, int scrlDir )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setScrollPos)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_INT( nPos ) ;
	LQT_FUNC_ARG_INT( scrlDir ) ;

	pSprite->SetScrollPos( nPos, (SGLSprite::ScrollDirection) scrlDir ) ;

	LQT_RETURN_VOID() ;
}

// int getScrollRange( int scrlDir )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getScrollRange)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_INT( scrlDir ) ;

	LQT_RETURN_INT( pSprite->GetScrollRange( (SGLSprite::ScrollDirection) scrlDir ) ) ;
}

// void setScrollRange( int nRange, int scrlDir )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setScrollRange)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_INT( nRange ) ;
	LQT_FUNC_ARG_INT( scrlDir ) ;

	pSprite->SetScrollRange( nRange, (SGLSprite::ScrollDirection) scrlDir ) ;

	LQT_RETURN_VOID() ;
}

// int getScrollPageSize( int scrlDir )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getScrollPageSize)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_INT( scrlDir ) ;

	LQT_RETURN_INT( pSprite->GetScrollPageSize( (SGLSprite::ScrollDirection) scrlDir ) ) ;
}

// void setScrollPageSize( int nPageSize, int scrlDir )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setScrollPageSize)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_INT( nPageSize ) ;
	LQT_FUNC_ARG_INT( scrlDir ) ;

	pSprite->SetScrollPageSize( nPageSize, (SGLSprite::ScrollDirection) scrlDir ) ;

	LQT_RETURN_VOID() ;
}

// boolean isButtonChecked( )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_isButtonChecked)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_BOOL( pSprite->IsButtonChecked() ) ;
}

// void checkButton( boolean flagCheck )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_checkButton)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_BOOL( flagCheck ) ;

	pSprite->CheckButton( flagCheck ) ;

	LQT_RETURN_VOID() ;
}

// boolean invokeCommands( String strXMLCommands, XMLDocument xmlResults )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_invokeCommands)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_STRING( strXMLCommands ) ;
	LQT_FUNC_ARG_OBJECT( LNativeObj, xmlResultsObj ) ;

	SXMLDocument	xmlResults ;
	LBoolean	valRet =
		(pSprite->InvokeCommands
			( strXMLCommands.c_str(), &xmlResults ) == sglErrSuccess) ;
	if ( valRet && (xmlResultsObj != nullptr) )
	{
		xmlResultsObj->SetNative( GetLXMLDocPtr( xmlResults ) ) ;
	}
	LQT_RETURN_BOOL( valRet ) ;
}

// void attachScrollBar( EntisGLS4.Sprite scrollBar, int scrlDir )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_attachScrollBar)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pThisSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pThisSprite ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Sprite, scrollBar ) ;
	LQT_VERIFY_NULL_PTR( scrollBar ) ;
	SGLSprite *	pBarSprite = scrollBar->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pBarSprite ) ;
	LQT_FUNC_ARG_INT( scrlDir ) ;

	pThisSprite->AttachScrollBar
		( pBarSprite, (SGLSprite::ScrollDirection) scrlDir ) ;

	LQT_RETURN_VOID() ;
}

// void detachScrollBar( EntisGLS4.Sprite scrollBar, int scrlDir )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_detachScrollBar)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pThisSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pThisSprite ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Sprite, scrollBar ) ;
	LQT_VERIFY_NULL_PTR( scrollBar ) ;
	SGLSprite *	pBarSprite = scrollBar->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pBarSprite ) ;
	LQT_FUNC_ARG_INT( scrlDir ) ;

	pThisSprite->DetachScrollBar
		( pBarSprite, (SGLSprite::ScrollDirection) scrlDir ) ;

	LQT_RETURN_VOID() ;
}

// boolean isSpriteVisible( String id ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_isSpriteVisible)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_STRING( id ) ;

	LQT_RETURN_BOOL( pSprite->IsSpriteVisible( id.c_str() ) ) ;
}

// void setSpriteVisible( String id, boolean visible )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setSpriteVisible)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_STRING( id ) ;
	LQT_FUNC_ARG_BOOL( visible ) ;

	pSprite->SetSpriteVisible( id.c_str(), visible ) ;

	LQT_RETURN_VOID() ;
}

// int getSpritePriority( String id ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getSpritePriority)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_STRING( id ) ;

	LQT_RETURN_INT( pSprite->GetSpritePriority( id.c_str() ) ) ;
}

// void changeSpritePriority( String id, int priority )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_changeSpritePriority)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_STRING( id ) ;
	LQT_FUNC_ARG_INT( priority ) ;

	pSprite->ChangeSpritePriority( id.c_str(), priority ) ;

	LQT_RETURN_VOID() ;
}

// uint getSpriteTransparency( String id ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getSpriteTransparency)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_STRING( id ) ;

	LQT_RETURN_UINT( pSprite->GetSpriteTransparency( id.c_str() ) ) ;
}

// void setSpriteTransparency( String id, uint transparency )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setSpriteTransparency)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_STRING( id ) ;
	LQT_FUNC_ARG_UINT( transparency ) ;

	pSprite->SetSpriteTransparency( id.c_str(), transparency ) ;

	LQT_RETURN_VOID() ;
}

// boolean getSpriteRectangle( String id, ImageRect* rectExt ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getSpriteRectangle)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_STRING( id ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, rectExt ) ;
	LQT_VERIFY_NULL_PTR( rectExt ) ;

	SGLRect		rect ;
	LBoolean	valRet =pSprite->GetSpriteRectangle( id.c_str(), rect ) ;
	if ( valRet )
	{
		*rectExt = rect ;
	}
	LQT_RETURN_BOOL( valRet ) ;
}

// boolean getSpriteTextRectangle( String id, ImageRect* rectExt ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getSpriteTextRectangle)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_STRING( id ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, rectExt ) ;
	LQT_VERIFY_NULL_PTR( rectExt ) ;

	SGLRect		rect ;
	LBoolean	valRet = pSprite->GetSpriteTextRectangle( id.c_str(), rect ) ;
	if ( valRet )
	{
		*rectExt = rect ;
	}
	LQT_RETURN_BOOL( valRet ) ;
}

// String getSpriteText( String id ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getSpriteText)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_STRING( id ) ;

	LQT_RETURN_STRING( pSprite->GetSpriteText( id.c_str() ) ) ;
}

// void setSpriteText( String id, String text )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setSpriteText)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_STRING( id ) ;
	LQT_FUNC_ARG_STRING( text ) ;

	pSprite->SetSpriteText( id.c_str(), text.c_str() ) ;

	LQT_RETURN_VOID() ;
}

// void setSpriteTextFont( String id, String font, int size )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setSpriteTextFont)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_STRING( id ) ;
	LQT_FUNC_ARG_STRING( font ) ;
	LQT_FUNC_ARG_INT( size ) ;

	pSprite->SetSpriteTextFont( id.c_str(), font.c_str(), size ) ;

	LQT_RETURN_VOID() ;
}

// void setSpriteImage( String id, String imageID )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setSpriteImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_STRING( id ) ;
	LQT_FUNC_ARG_STRING( imageID ) ;

	pSprite->SetSpriteImage( id.c_str(), imageID.c_str() ) ;

	LQT_RETURN_VOID() ;
}

// boolean isSpriteEnabled( String id ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_isSpriteEnabled)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_STRING( id ) ;

	LQT_RETURN_BOOL( pSprite->IsSpriteEnabled( id.c_str() ) ) ;
}

// void setSpriteEnable( String id, boolean flagEnable )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setSpriteEnable)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_STRING( id ) ;
	LQT_FUNC_ARG_BOOL( flagEnable ) ;

	pSprite->SetSpriteEnable( id.c_str(), flagEnable ) ;

	LQT_RETURN_VOID() ;
}

// int getSpriteScrollPos( String id, int scrlDir ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getSpriteScrollPos)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_STRING( id ) ;
	LQT_FUNC_ARG_INT( scrlDir ) ;

	LQT_RETURN_INT
		( pSprite->GetSpriteScrollPos
			( id.c_str(), (SGLSprite::ScrollDirection) scrlDir ) ) ;
}

// void setSpriteScrollPos( String id, int nPos, int scrlDir )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setSpriteScrollPos)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_STRING( id ) ;
	LQT_FUNC_ARG_INT( nPos ) ;
	LQT_FUNC_ARG_INT( scrlDir ) ;

	pSprite->SetSpriteScrollPos
		( id.c_str(), nPos, (SGLSprite::ScrollDirection) scrlDir ) ;

	LQT_RETURN_VOID() ;
}

// int getSpriteScrollRange( String id, int scrlDir ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getSpriteScrollRange)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_STRING( id ) ;
	LQT_FUNC_ARG_INT( scrlDir ) ;

	LQT_RETURN_INT
		( pSprite->GetSpriteScrollRange
			( id.c_str(), (SGLSprite::ScrollDirection) scrlDir ) ) ;
}

// void setSpriteScrollRange( String id, int nRange, int scrlDir )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setSpriteScrollRange)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_STRING( id ) ;
	LQT_FUNC_ARG_INT( nRange ) ;
	LQT_FUNC_ARG_INT( scrlDir ) ;

	pSprite->SetSpriteScrollRange
		( id.c_str(), nRange, (SGLSprite::ScrollDirection) scrlDir ) ;

	LQT_RETURN_VOID() ;
}

// int getSpriteScrollPageSize( String id, int scrlDir ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getSpriteScrollPageSize)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_STRING( id ) ;
	LQT_FUNC_ARG_INT( scrlDir ) ;

	LQT_RETURN_INT
		( pSprite->GetSpriteScrollPageSize
			( id.c_str(), (SGLSprite::ScrollDirection) scrlDir ) ) ;
}

// void setSpriteScrollPageSize( String id, int nPageSize, int scrlDir )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setSpriteScrollPageSize)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_STRING( id ) ;
	LQT_FUNC_ARG_INT( nPageSize ) ;
	LQT_FUNC_ARG_INT( scrlDir ) ;

	pSprite->SetSpriteScrollPageSize
		( id.c_str(), nPageSize, (SGLSprite::ScrollDirection) scrlDir ) ;

	LQT_RETURN_VOID() ;
}

// boolean isSpriteButtonChecked( String id ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_isSpriteButtonChecked)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_STRING( id ) ;

	LQT_RETURN_BOOL( pSprite->IsSpriteButtonChecked( id.c_str() ) ) ;
}

// void checkSpriteButton( String id, boolean flagCheck )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_checkSpriteButton)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_STRING( id ) ;
	LQT_FUNC_ARG_BOOL( flagCheck ) ;

	pSprite->CheckSpriteButton( id.c_str(), flagCheck ) ;

	LQT_RETURN_VOID() ;
}

// void setProjectionScreen( const Vector3* vScreen, double zScale, double fpPixelAspect )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setProjectionScreen)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_STRUCT( LVector3, vScreen ) ;
	LQT_VERIFY_NULL_PTR( vScreen ) ;
	LQT_FUNC_ARG_DOUBLE( zScale ) ;
	LQT_FUNC_ARG_DOUBLE( fpPixelAspect ) ;

	pSprite->SetProjectionScreen( *vScreen, zScale, fpPixelAspect ) ;

	LQT_RETURN_VOID() ;
}

// boolean getProjectionScreen( Vector3* vScreen, double* zScale, double* fpPixelAspect ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getProjectionScreen)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_STRUCT( LVector3, vScreen ) ;
	LQT_VERIFY_NULL_PTR( vScreen ) ;
	LQT_FUNC_ARG_POINTER( LDouble, zScale ) ;
	LQT_VERIFY_NULL_PTR( zScale ) ;
	LQT_FUNC_ARG_POINTER( LDouble, fpPixelAspect ) ;
	LQT_VERIFY_NULL_PTR( fpPixelAspect ) ;

	LQT_RETURN_BOOL
		( pSprite->GetProjectionScreen( *vScreen, *zScale, *fpPixelAspect ) ) ;
}

// void setParallax( double xParallax, double zFocusRate, double xScreenDelta )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setParallax)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_DOUBLE( xParallax ) ;
	LQT_FUNC_ARG_DOUBLE( zFocusRate ) ;
	LQT_FUNC_ARG_DOUBLE( xScreenDelta ) ;

	pSprite->SetParallax( xParallax, zFocusRate, xScreenDelta ) ;

	LQT_RETURN_VOID() ;
}

// double getParallax( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getParallax)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_DOUBLE( pSprite->GetParallax() ) ;
}

// double getParallaxFocusRatio( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getParallaxFocusRatio)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_DOUBLE( pSprite->GetParallaxFocusRatio() ) ;
}

// double getParallaxScreenX( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getParallaxScreenX)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_DOUBLE( pSprite->GetParallaxScreenX() ) ;
}

// void setVirtualCamera( const Vector3d* vCamera, const Vector3d* vTarget )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setVirtualCamera)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vCamera ) ;
	LQT_VERIFY_NULL_PTR( vCamera ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vTarget ) ;
	LQT_VERIFY_NULL_PTR( vTarget ) ;

	pSprite->SetVirtualCamera( *vCamera, *vTarget );

	LQT_RETURN_VOID() ;
}

// const EntisGLS4.Sprite.Virtual3DParam* getVirtual3DParam( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getVirtual3DParam)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LEntisGLS4_Sprite_Virtual3DParam *	pv3p = pSprite->GetVirtual3DParam() ;
	if ( pv3p == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	std::shared_ptr<LArrayBufAlias>	buf =
		std::make_shared<LArrayBufAlias>
			( (uint8_t*) pv3p, sizeof(LEntisGLS4_Sprite_Virtual3DParam) ) ;

	LQT_RETURN_POINTER_BUF( buf ) ;
}

// void setActionLinearTo( uint msecDuration, uint transparency, const Vector2d* pos, const Vector2d* zoom, double a0, double a1 )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setActionLinearTo)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_UINT( msecDuration ) ;
	LQT_FUNC_ARG_UINT( transparency ) ;
	LQT_FUNC_ARG_STRUCT( LVector2d, pos ) ;
	LQT_FUNC_ARG_STRUCT( LVector2d, zoom ) ;
	LQT_FUNC_ARG_DOUBLE( a0 ) ;
	LQT_FUNC_ARG_DOUBLE( a1 ) ;

	pSprite->SetActionLinearTo( msecDuration, transparency, pos, zoom, a0, a1 ) ;

	LQT_RETURN_VOID() ;
}

// void flushAction( )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_flushAction)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	pSprite->FlushAction() ;

	LQT_RETURN_VOID() ;
}

// void cancelAction( )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_cancelAction)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	pSprite->CancelAction() ;

	LQT_RETURN_VOID() ;
}

// boolean isAction( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_isAction)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_BOOL( pSprite->IsAction() ) ;
}

// void pauseAllAction( )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_pauseAllAction)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	pSprite->PauseAllAction() ;

	LQT_RETURN_VOID() ;
}

// void restartAllAction( )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_restartAllAction)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	pSprite->RestartAllAction() ;

	LQT_RETURN_VOID() ;
}

// boolean postCommand( String cmd, long param, long code, int priority, boolean overwritable )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_postCommand)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_STRING( cmd ) ;
	LQT_FUNC_ARG_LONG( param ) ;
	LQT_FUNC_ARG_LONG( code ) ;
	LQT_FUNC_ARG_INT( priority ) ;
	LQT_FUNC_ARG_BOOL( overwritable ) ;

	LQT_RETURN_BOOL
		( pSprite->NotifyCommand
			( cmd.c_str(), param, code, priority, overwritable ) ) ;
}

// boolean setMouseCapture( )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setMouseCapture)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_BOOL( pSprite->SetMouseCapture() == sglErrSuccess ) ;
}

// boolean releaseMouseCapture( )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_releaseMouseCapture)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_BOOL( pSprite->ReleaseMouseCapture() == sglErrSuccess ) ;
}

// EntisGLS4.Sprite getMouseCapture( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getMouseCapture)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	SGLSprite *	pCaptured = pSprite->GetMouseCapture() ;
	if ( pCaptured == nullptr )
	{
		LQT_RETURN_OBJECT( nullptr ) ;
	}
	LPtr<LNativeObj>	valRet( new LNativeObj( GetSpriteClass( _context.VM(), pCaptured ) ) ) ;
	valRet->SetNative( std::make_shared<LEntisGLS4_Sprite>(pCaptured) ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// boolean setKeyFocus( )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setKeyFocus)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_BOOL( pSprite->SetKeyFocus() == sglErrSuccess ) ;
}

// boolean killKeyFocus( )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_killKeyFocus)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_BOOL( pSprite->KillKeyFocus() == sglErrSuccess ) ;
}

// boolean hasKeyFocus( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_hasKeyFocus)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_BOOL( pSprite->HasKeyFocus() ) ;
}

// boolean moveNextKeyFocus( )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_moveNextKeyFocus)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_BOOL( pSprite->MoveNextKeyFocus() ) ;
}

// boolean movePrevKeyFocus( )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_movePrevKeyFocus)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	LQT_RETURN_BOOL( pSprite->MovePrevKeyFocus() ) ;
}

// boolean moveKeyFocusDirectionOf( const Vector2d* vDir )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_moveKeyFocusDirectionOf)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_STRUCT( LVector2d, vDir ) ;
	LQT_VERIFY_NULL_PTR( vDir ) ;

	LQT_RETURN_BOOL( pSprite->MoveKeyFocusDirectionOf( *vDir ) ) ;
}

// void attachMouseListener( EntisGLS4.SpriteMouseListener listener )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_attachMouseListener)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SpriteMouseListener, listener ) ;
	LQT_VERIFY_NULL_PTR( listener ) ;
	LSpriteMouseListener *	pListener = listener->GetRef<LSpriteMouseListener>() ;
	LQT_VERIFY_NULL_PTR( pListener ) ;

	pSprite->AttachMouseListener( pListener ) ;

	LQT_RETURN_VOID() ;
}

// void detachMouseListener( EntisGLS4.SpriteMouseListener listener )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_detachMouseListener)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SpriteMouseListener, listener ) ;
	LQT_VERIFY_NULL_PTR( listener ) ;
	LSpriteMouseListener *	pListener = listener->GetRef<LSpriteMouseListener>() ;
	LQT_VERIFY_NULL_PTR( pListener ) ;

	pSprite->DetachMouseListener( pListener ) ;

	LQT_RETURN_VOID() ;
}

// void attachKeyListener( EntisGLS4.SpriteKeyListener listener )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_attachKeyListener)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SpriteKeyListener, listener ) ;
	LQT_VERIFY_NULL_PTR( listener ) ;
	LSpriteKeyListener *	pListener = listener->GetRef<LSpriteKeyListener>() ;
	LQT_VERIFY_NULL_PTR( pListener ) ;

	pSprite->AttachKeyListener( pListener ) ;

	LQT_RETURN_VOID() ;
}

// void detachKeyListener( EntisGLS4.SpriteKeyListener listener )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_detachKeyListener)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SpriteKeyListener, listener ) ;
	LQT_VERIFY_NULL_PTR( listener ) ;
	LSpriteKeyListener *	pListener = listener->GetRef<LSpriteKeyListener>() ;
	LQT_VERIFY_NULL_PTR( pListener ) ;

	pSprite->DetachKeyListener( pListener ) ;

	LQT_RETURN_VOID() ;
}

// void addTimer( EntisGLS4.SpriteTimer timer )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_addTimer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SpriteTimer, timer ) ;
	LQT_VERIFY_NULL_PTR( timer ) ;
	LSpriteTimer *	pTimer = timer->GetRef<LSpriteTimer>() ;
	LQT_VERIFY_NULL_PTR( pTimer ) ;

	pSprite->AddReferenceTimer( pTimer ) ;

	LQT_RETURN_VOID() ;
}

// void removeTimer( EntisGLS4.SpriteTimer timer )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_removeTimer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SpriteTimer, timer ) ;
	LQT_VERIFY_NULL_PTR( timer ) ;
	LSpriteTimer *	pTimer = timer->GetRef<LSpriteTimer>() ;
	LQT_VERIFY_NULL_PTR( pTimer ) ;

	pSprite->RemoveTimer( pTimer ) ;

	LQT_RETURN_VOID() ;
}

// void removeAllTimers( )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_removeAllTimers)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	SGLSprite *	pSprite = pThis->GetRef<SGLSprite>() ;
	LQT_VERIFY_NULL_PTR( pSprite ) ;

	pSprite->RemoveAllTimer() ;

	LQT_RETURN_VOID() ;
}



