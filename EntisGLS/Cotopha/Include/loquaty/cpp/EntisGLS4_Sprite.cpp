
#include <loquaty.h>
#include "EntisGLS4_Sprite.h"

using namespace Loquaty ;


// Sprite( )
IMPL_LOQUATY_CONSTRUCTOR(EntisGLS4_Sprite)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_INIT_NOBJ( LEntisGLS4_Sprite, pThis, () ) ;

	// pThis->Initialize() ;

	LQT_RETURN_VOID() ;
}

// EntisGLS4.Sprite duplicate( )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_duplicate)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.Sprite) ) ) ;
	// valRet = pThis->duplicate(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_Sprite> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_Sprite>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// const EntisGLS4.Sprite.Parameter* getParameter( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	LEntisGLS4_Sprite_Parameter	valRet ;
	// valRet = pThis->getParameter(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// EntisGLS4.PaintParam.PaintFlag getDrawParamFlags( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getDrawParamFlags)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	LUint32	valRet ;
	// valRet = pThis->getDrawParamFlags(...) ;

	LQT_RETURN_UINT( valRet ) ;
}

// EntisGLS4.Sprite.SpriteFlag getSpriteFlags( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getSpriteFlags)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	LUint32	valRet ;
	// valRet = pThis->getSpriteFlags(...) ;

	LQT_RETURN_UINT( valRet ) ;
}

// const Vector3d* getPosition( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getPosition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	LVector3d	valRet ;
	// valRet = pThis->getPosition(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// const Vector2d* getPosition2D( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getPosition2D)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	LVector2d	valRet ;
	// valRet = pThis->getPosition2D(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// const Vector2d* getCenterPosition( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getCenterPosition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	LVector2d	valRet ;
	// valRet = pThis->getCenterPosition(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// const Vector2d* getImageCenter( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getImageCenter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	LVector2d	valRet ;
	// valRet = pThis->getImageCenter(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// const Vector2d* getZoom( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getZoom)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	LVector2d	valRet ;
	// valRet = pThis->getZoom(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// double getRotation( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getRotation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	LDouble	valRet ;
	// valRet = pThis->getRotation(...) ;

	LQT_RETURN_DOUBLE( valRet ) ;
}

// uint getTransparency( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getTransparency)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	LUint32	valRet ;
	// valRet = pThis->getTransparency(...) ;

	LQT_RETURN_UINT( valRet ) ;
}

// int getFilterParameter( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getFilterParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	LInt32	valRet ;
	// valRet = pThis->getFilterParameter(...) ;

	LQT_RETURN_INT( valRet ) ;
}

// int getFilter2Parameter( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getFilter2Parameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	LInt32	valRet ;
	// valRet = pThis->getFilter2Parameter(...) ;

	LQT_RETURN_INT( valRet ) ;
}

// void setParameter( const EntisGLS4.Sprite.Parameter* param )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_Sprite_Parameter, param ) ;
	LQT_VERIFY_NULL_PTR( param ) ;

	// pThis->setParameter(...) ;

	LQT_RETURN_VOID() ;
}

// void modifyDrawFlags( EntisGLS4.PaintParam.PaintFlag nAddFlags, EntisGLS4.PaintParam.PaintFlag nRemoveFlags )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_modifyDrawFlags)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_UINT( nAddFlags ) ;
	LQT_FUNC_ARG_UINT( nRemoveFlags ) ;

	// pThis->modifyDrawFlags(...) ;

	LQT_RETURN_VOID() ;
}

// void modifySpriteFlags( EntisGLS4.Sprite.SpriteFlag nAddFlags, EntisGLS4.Sprite.SpriteFlag nRemoveFlags )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_modifySpriteFlags)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_UINT( nAddFlags ) ;
	LQT_FUNC_ARG_UINT( nRemoveFlags ) ;

	// pThis->modifySpriteFlags(...) ;

	LQT_RETURN_VOID() ;
}

// void setPosition( double x, double y )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setPosition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_DOUBLE( x ) ;
	LQT_FUNC_ARG_DOUBLE( y ) ;

	// pThis->setPosition(...) ;

	LQT_RETURN_VOID() ;
}

// void setPosition3D( double x, double y, double z )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setPosition3D)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_DOUBLE( x ) ;
	LQT_FUNC_ARG_DOUBLE( y ) ;
	LQT_FUNC_ARG_DOUBLE( z ) ;

	// pThis->setPosition3D(...) ;

	LQT_RETURN_VOID() ;
}

// void setPosition3D( const Vector3d* vPos )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setPosition3D_1)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vPos ) ;
	LQT_VERIFY_NULL_PTR( vPos ) ;

	// pThis->setPosition3D(...) ;

	LQT_RETURN_VOID() ;
}

// void setCenterPosition( double x, double y )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setCenterPosition)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_DOUBLE( x ) ;
	LQT_FUNC_ARG_DOUBLE( y ) ;

	// pThis->setCenterPosition(...) ;

	LQT_RETURN_VOID() ;
}

// void setZoom( double x, double y )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setZoom)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_DOUBLE( x ) ;
	LQT_FUNC_ARG_DOUBLE( y ) ;

	// pThis->setZoom(...) ;

	LQT_RETURN_VOID() ;
}

// void setRotation( double zAngle )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setRotation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_DOUBLE( zAngle ) ;

	// pThis->setRotation(...) ;

	LQT_RETURN_VOID() ;
}

// void setTransparency( uint transparency )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setTransparency)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_UINT( transparency ) ;

	// pThis->setTransparency(...) ;

	LQT_RETURN_VOID() ;
}

// void setFilterParameter( int paramFilter )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setFilterParameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_INT( paramFilter ) ;

	// pThis->setFilterParameter(...) ;

	LQT_RETURN_VOID() ;
}

// void setFilter2Parameter( int paramFilter )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setFilter2Parameter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_INT( paramFilter ) ;

	// pThis->setFilter2Parameter(...) ;

	LQT_RETURN_VOID() ;
}

// EntisGLS4.Sprite.UIFlag getUIFlag( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getUIFlag)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	LUint64	valRet ;
	// valRet = pThis->getUIFlag(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// ulong modifyUIFlag( EntisGLS4.Sprite.UIFlag nAddFlags, EntisGLS4.Sprite.UIFlag nRemoveFlags )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_modifyUIFlag)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_ULONG( nAddFlags ) ;
	LQT_FUNC_ARG_ULONG( nRemoveFlags ) ;

	LUint64	valRet ;
	// valRet = pThis->modifyUIFlag(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// const ImageRect* getClickableRect( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getClickableRect)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	LImageRect	valRet ;
	// valRet = pThis->getClickableRect(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// void setClickableRect( const ImageRect* rect )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setClickableRect)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, rect ) ;
	LQT_VERIFY_NULL_PTR( rect ) ;

	// pThis->setClickableRect(...) ;

	LQT_RETURN_VOID() ;
}

// boolean isVisible( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_isVisible)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->isVisible(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void setVisible( boolean visible )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setVisible)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_BOOL( visible ) ;

	// pThis->setVisible(...) ;

	LQT_RETURN_VOID() ;
}

// int getPriority( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getPriority)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	LInt32	valRet ;
	// valRet = pThis->getPriority(...) ;

	LQT_RETURN_INT( valRet ) ;
}

// void changePriority( int priority )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_changePriority)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_INT( priority ) ;

	// pThis->changePriority(...) ;

	LQT_RETURN_VOID() ;
}

// String getID( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getID)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	LString	valRet ;
	// valRet = pThis->getID(...) ;

	LQT_RETURN_STRING( valRet ) ;
}

// void setID( String id )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setID)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_STRING( id ) ;

	// pThis->setID(...) ;

	LQT_RETURN_VOID() ;
}

// boolean getRectangle( ImageRect* rectExt ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getRectangle)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, rectExt ) ;
	LQT_VERIFY_NULL_PTR( rectExt ) ;

	LBoolean	valRet ;
	// valRet = pThis->getRectangle(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean localToGlobal( Vector2d* vPos ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_localToGlobal)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LVector2d, vPos ) ;
	LQT_VERIFY_NULL_PTR( vPos ) ;

	LBoolean	valRet ;
	// valRet = pThis->localToGlobal(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean globalToLocal( Vector2d* vPos ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_globalToLocal)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LVector2d, vPos ) ;
	LQT_VERIFY_NULL_PTR( vPos ) ;

	LBoolean	valRet ;
	// valRet = pThis->globalToLocal(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void postUpdate( ImageRect* pUpdate )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_postUpdate)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, pUpdate ) ;
	LQT_VERIFY_NULL_PTR( pUpdate ) ;

	// pThis->postUpdate(...) ;

	LQT_RETURN_VOID() ;
}

// boolean hasUpdate( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_hasUpdate)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->hasUpdate(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void freezeFrameUpdate( )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_freezeFrameUpdate)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	// pThis->freezeFrameUpdate(...) ;

	LQT_RETURN_VOID() ;
}

// boolean defrostFrameUpdate( )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_defrostFrameUpdate)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->defrostFrameUpdate(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void addChild( EntisGLS4.Sprite sprite )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_addChild)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Sprite, sprite ) ;
	LQT_VERIFY_NULL_PTR( sprite ) ;

	// pThis->addChild(...) ;

	LQT_RETURN_VOID() ;
}

// boolean removeChild( EntisGLS4.Sprite sprite )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_removeChild)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Sprite, sprite ) ;
	LQT_VERIFY_NULL_PTR( sprite ) ;

	LBoolean	valRet ;
	// valRet = pThis->removeChild(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void removeAllChildren( )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_removeAllChildren)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	// pThis->removeAllChildren(...) ;

	LQT_RETURN_VOID() ;
}

// ulong getChildCount( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getChildCount)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	LUint64	valRet ;
	// valRet = pThis->getChildCount(...) ;

	LQT_RETURN_ULONG( valRet ) ;
}

// EntisGLS4.Sprite getChildAt( ulong index ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getChildAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_ULONG( index ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.Sprite) ) ) ;
	// valRet = pThis->getChildAt(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_Sprite> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_Sprite>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.Sprite getParent( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getParent)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.Sprite) ) ) ;
	// valRet = pThis->getParent(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_Sprite> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_Sprite>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.Sprite getItemAs( String id ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getItemAs)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_STRING( id ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.Sprite) ) ) ;
	// valRet = pThis->getItemAs(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_Sprite> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_Sprite>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.Sprite getHitSpriteAt( Vector2d* vPos ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getHitSpriteAt)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LVector2d, vPos ) ;
	LQT_VERIFY_NULL_PTR( vPos ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.Sprite) ) ) ;
	// valRet = pThis->getHitSpriteAt(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_Sprite> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_Sprite>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// boolean isHitSprite( double x, double y ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_isHitSprite)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_DOUBLE( x ) ;
	LQT_FUNC_ARG_DOUBLE( y ) ;

	LBoolean	valRet ;
	// valRet = pThis->isHitSprite(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean loadImage( String path )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_loadImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_STRING( path ) ;

	LBoolean	valRet ;
	// valRet = pThis->loadImage(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void attachAnimation( EntisGLS4.Image image, const ImageRect* pClip )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_attachAnimation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, image ) ;
	LQT_VERIFY_NULL_PTR( image ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, pClip ) ;
	LQT_VERIFY_NULL_PTR( pClip ) ;

	// pThis->attachAnimation(...) ;

	LQT_RETURN_VOID() ;
}

// void beginAnimation( long countLoop, ulong iLoopStart, ulong iLoopEnd, ulong iStartFrame, ulong msecDuration )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_beginAnimation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_LONG( countLoop ) ;
	LQT_FUNC_ARG_ULONG( iLoopStart ) ;
	LQT_FUNC_ARG_ULONG( iLoopEnd ) ;
	LQT_FUNC_ARG_ULONG( iStartFrame ) ;
	LQT_FUNC_ARG_ULONG( msecDuration ) ;

	// pThis->beginAnimation(...) ;

	LQT_RETURN_VOID() ;
}

// void setLoopAnimation( long countLoop, ulong iLoopStart, ulong iLoopEnd )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setLoopAnimation)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_LONG( countLoop ) ;
	LQT_FUNC_ARG_ULONG( iLoopStart ) ;
	LQT_FUNC_ARG_ULONG( iLoopEnd ) ;

	// pThis->setLoopAnimation(...) ;

	LQT_RETURN_VOID() ;
}

// void attachImage( EntisGLS4.Image image, EntisGLS4.Image leftImage )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_attachImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, image ) ;
	LQT_VERIFY_NULL_PTR( image ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Image, leftImage ) ;
	LQT_VERIFY_NULL_PTR( leftImage ) ;

	// pThis->attachImage(...) ;

	LQT_RETURN_VOID() ;
}

// EntisGLS4.Image getAttachedImage( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getAttachedImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.Image) ) ) ;
	// valRet = pThis->getAttachedImage(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_Image> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_Image>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// EntisGLS4.Image getAttachedLeftImage( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getAttachedLeftImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.Image) ) ) ;
	// valRet = pThis->getAttachedLeftImage(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_Image> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_Image>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// Size* getImageSize( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getImageSize)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	LSize	valRet ;
	// valRet = pThis->getImageSize(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// boolean getImageInfo( EntisGLS4.ImageInfo* imginf ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getImageInfo)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LEntisGLS4_ImageInfo, imginf ) ;
	LQT_VERIFY_NULL_PTR( imginf ) ;

	LBoolean	valRet ;
	// valRet = pThis->getImageInfo(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean createBuffer( uint width, uint height, uint format, uint depth, ulong nBufFlags, boolean flagZBuffer, boolean flagStereo3D )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_createBuffer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_UINT( width ) ;
	LQT_FUNC_ARG_UINT( height ) ;
	LQT_FUNC_ARG_UINT( format ) ;
	LQT_FUNC_ARG_UINT( depth ) ;
	LQT_FUNC_ARG_ULONG( nBufFlags ) ;
	LQT_FUNC_ARG_BOOL( flagZBuffer ) ;
	LQT_FUNC_ARG_BOOL( flagStereo3D ) ;

	LBoolean	valRet ;
	// valRet = pThis->createBuffer(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void releaseBuffer( )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_releaseBuffer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	// pThis->releaseBuffer(...) ;

	LQT_RETURN_VOID() ;
}

// boolean isBuffered( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_isBuffered)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->isBuffered(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean setRenderDevice( EntisGLS4.RenderDevice device )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setRenderDevice)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_RenderDevice, device ) ;
	LQT_VERIFY_NULL_PTR( device ) ;

	LBoolean	valRet ;
	// valRet = pThis->setRenderDevice(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// EntisGLS4.RenderDevice getBufferRenderDevice( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getBufferRenderDevice)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.RenderDevice) ) ) ;
	// valRet = pThis->getBufferRenderDevice(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_RenderDevice> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_RenderDevice>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// boolean getFillBackColor( uint* argbFill ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getFillBackColor)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_POINTER( LUint32, argbFill ) ;
	LQT_VERIFY_NULL_PTR( argbFill ) ;

	LBoolean	valRet ;
	// valRet = pThis->getFillBackColor(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean setFillBackColor( uint argbFill, boolean flagFillBack )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setFillBackColor)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_UINT( argbFill ) ;
	LQT_FUNC_ARG_BOOL( flagFillBack ) ;

	LBoolean	valRet ;
	// valRet = pThis->setFillBackColor(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void addFilter( EntisGLS4.SpriteFilter filter )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_addFilter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SpriteFilter, filter ) ;
	LQT_VERIFY_NULL_PTR( filter ) ;

	// pThis->addFilter(...) ;

	LQT_RETURN_VOID() ;
}

// void removeFilter( EntisGLS4.SpriteFilter filter )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_removeFilter)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SpriteFilter, filter ) ;
	LQT_VERIFY_NULL_PTR( filter ) ;

	// pThis->removeFilter(...) ;

	LQT_RETURN_VOID() ;
}

// void removeAllFilters( )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_removeAllFilters)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	// pThis->removeAllFilters(...) ;

	LQT_RETURN_VOID() ;
}

// String getText( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getText)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	LString	valRet ;
	// valRet = pThis->getText(...) ;

	LQT_RETURN_STRING( valRet ) ;
}

// void setText( String text )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setText)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_STRING( text ) ;

	// pThis->setText(...) ;

	LQT_RETURN_VOID() ;
}

// void setTextFont( String font, int size )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setTextFont)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_STRING( font ) ;
	LQT_FUNC_ARG_INT( size ) ;

	// pThis->setTextFont(...) ;

	LQT_RETURN_VOID() ;
}

// boolean isEnabled( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_isEnabled)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->isEnabled(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void setEnable( boolean flagEnable )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setEnable)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_BOOL( flagEnable ) ;

	// pThis->setEnable(...) ;

	LQT_RETURN_VOID() ;
}

// int getScrollPos( EntisGLS4.Sprite.ScrollDirection scrlDir )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getScrollPos)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_INT( scrlDir ) ;

	LInt32	valRet ;
	// valRet = pThis->getScrollPos(...) ;

	LQT_RETURN_INT( valRet ) ;
}

// void setScrollPos( int nPos, EntisGLS4.Sprite.ScrollDirection scrlDir )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setScrollPos)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_INT( nPos ) ;
	LQT_FUNC_ARG_INT( scrlDir ) ;

	// pThis->setScrollPos(...) ;

	LQT_RETURN_VOID() ;
}

// int getScrollRange( EntisGLS4.Sprite.ScrollDirection scrlDir )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getScrollRange)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_INT( scrlDir ) ;

	LInt32	valRet ;
	// valRet = pThis->getScrollRange(...) ;

	LQT_RETURN_INT( valRet ) ;
}

// void setScrollRange( int nRange, EntisGLS4.Sprite.ScrollDirection scrlDir )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setScrollRange)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_INT( nRange ) ;
	LQT_FUNC_ARG_INT( scrlDir ) ;

	// pThis->setScrollRange(...) ;

	LQT_RETURN_VOID() ;
}

// int getScrollPageSize( EntisGLS4.Sprite.ScrollDirection scrlDir )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getScrollPageSize)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_INT( scrlDir ) ;

	LInt32	valRet ;
	// valRet = pThis->getScrollPageSize(...) ;

	LQT_RETURN_INT( valRet ) ;
}

// void setScrollPageSize( int nPageSize, EntisGLS4.Sprite.ScrollDirection scrlDir )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setScrollPageSize)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_INT( nPageSize ) ;
	LQT_FUNC_ARG_INT( scrlDir ) ;

	// pThis->setScrollPageSize(...) ;

	LQT_RETURN_VOID() ;
}

// boolean isButtonChecked( )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_isButtonChecked)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->isButtonChecked(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void checkButton( boolean flagCheck )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_checkButton)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_BOOL( flagCheck ) ;

	// pThis->checkButton(...) ;

	LQT_RETURN_VOID() ;
}

// boolean invokeCommands( String strXMLCommands, XMLDocument xmlResults )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_invokeCommands)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_STRING( strXMLCommands ) ;
	LQT_FUNC_ARG_NOBJ( LXMLDocument, xmlResults ) ;
	LQT_VERIFY_NULL_PTR( xmlResults ) ;

	LBoolean	valRet ;
	// valRet = pThis->invokeCommands(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void attachScrollBar( EntisGLS4.Sprite scrollBar, EntisGLS4.Sprite.ScrollDirection scrlDir )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_attachScrollBar)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Sprite, scrollBar ) ;
	LQT_VERIFY_NULL_PTR( scrollBar ) ;
	LQT_FUNC_ARG_INT( scrlDir ) ;

	// pThis->attachScrollBar(...) ;

	LQT_RETURN_VOID() ;
}

// void detachScrollBar( EntisGLS4.Sprite scrollBar, EntisGLS4.Sprite.ScrollDirection scrlDir )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_detachScrollBar)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_Sprite, scrollBar ) ;
	LQT_VERIFY_NULL_PTR( scrollBar ) ;
	LQT_FUNC_ARG_INT( scrlDir ) ;

	// pThis->detachScrollBar(...) ;

	LQT_RETURN_VOID() ;
}

// boolean isSpriteVisible( String id ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_isSpriteVisible)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_STRING( id ) ;

	LBoolean	valRet ;
	// valRet = pThis->isSpriteVisible(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void setSpriteVisible( String id, boolean visible )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setSpriteVisible)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_STRING( id ) ;
	LQT_FUNC_ARG_BOOL( visible ) ;

	// pThis->setSpriteVisible(...) ;

	LQT_RETURN_VOID() ;
}

// int getSpritePriority( String id ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getSpritePriority)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_STRING( id ) ;

	LInt32	valRet ;
	// valRet = pThis->getSpritePriority(...) ;

	LQT_RETURN_INT( valRet ) ;
}

// void changeSpritePriority( String id, int priority )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_changeSpritePriority)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_STRING( id ) ;
	LQT_FUNC_ARG_INT( priority ) ;

	// pThis->changeSpritePriority(...) ;

	LQT_RETURN_VOID() ;
}

// uint getSpriteTransparency( String id ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getSpriteTransparency)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_STRING( id ) ;

	LUint32	valRet ;
	// valRet = pThis->getSpriteTransparency(...) ;

	LQT_RETURN_UINT( valRet ) ;
}

// void setSpriteTransparency( String id, uint transparency )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setSpriteTransparency)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_STRING( id ) ;
	LQT_FUNC_ARG_UINT( transparency ) ;

	// pThis->setSpriteTransparency(...) ;

	LQT_RETURN_VOID() ;
}

// boolean getSpriteRectangle( String id, ImageRect* rectExt ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getSpriteRectangle)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_STRING( id ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, rectExt ) ;
	LQT_VERIFY_NULL_PTR( rectExt ) ;

	LBoolean	valRet ;
	// valRet = pThis->getSpriteRectangle(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean getSpriteTextRectangle( String id, ImageRect* rectExt ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getSpriteTextRectangle)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_STRING( id ) ;
	LQT_FUNC_ARG_STRUCT( LImageRect, rectExt ) ;
	LQT_VERIFY_NULL_PTR( rectExt ) ;

	LBoolean	valRet ;
	// valRet = pThis->getSpriteTextRectangle(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// String getSpriteText( String id ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getSpriteText)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_STRING( id ) ;

	LString	valRet ;
	// valRet = pThis->getSpriteText(...) ;

	LQT_RETURN_STRING( valRet ) ;
}

// void setSpriteText( String id, String text )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setSpriteText)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_STRING( id ) ;
	LQT_FUNC_ARG_STRING( text ) ;

	// pThis->setSpriteText(...) ;

	LQT_RETURN_VOID() ;
}

// void setSpriteTextFont( String id, String font, int size )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setSpriteTextFont)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_STRING( id ) ;
	LQT_FUNC_ARG_STRING( font ) ;
	LQT_FUNC_ARG_INT( size ) ;

	// pThis->setSpriteTextFont(...) ;

	LQT_RETURN_VOID() ;
}

// void setSpriteImage( String id, String imageID )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setSpriteImage)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_STRING( id ) ;
	LQT_FUNC_ARG_STRING( imageID ) ;

	// pThis->setSpriteImage(...) ;

	LQT_RETURN_VOID() ;
}

// boolean isSpriteEnabled( String id ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_isSpriteEnabled)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_STRING( id ) ;

	LBoolean	valRet ;
	// valRet = pThis->isSpriteEnabled(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void setSpriteEnable( String id, boolean flagEnable )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setSpriteEnable)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_STRING( id ) ;
	LQT_FUNC_ARG_BOOL( flagEnable ) ;

	// pThis->setSpriteEnable(...) ;

	LQT_RETURN_VOID() ;
}

// int getSpriteScrollPos( String id, EntisGLS4.Sprite.ScrollDirection scrlDir ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getSpriteScrollPos)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_STRING( id ) ;
	LQT_FUNC_ARG_INT( scrlDir ) ;

	LInt32	valRet ;
	// valRet = pThis->getSpriteScrollPos(...) ;

	LQT_RETURN_INT( valRet ) ;
}

// void setSpriteScrollPos( String id, int nPos, EntisGLS4.Sprite.ScrollDirection scrlDir )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setSpriteScrollPos)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_STRING( id ) ;
	LQT_FUNC_ARG_INT( nPos ) ;
	LQT_FUNC_ARG_INT( scrlDir ) ;

	// pThis->setSpriteScrollPos(...) ;

	LQT_RETURN_VOID() ;
}

// int getSpriteScrollRange( String id, EntisGLS4.Sprite.ScrollDirection scrlDir ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getSpriteScrollRange)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_STRING( id ) ;
	LQT_FUNC_ARG_INT( scrlDir ) ;

	LInt32	valRet ;
	// valRet = pThis->getSpriteScrollRange(...) ;

	LQT_RETURN_INT( valRet ) ;
}

// void setSpriteScrollRange( String id, int nRange, EntisGLS4.Sprite.ScrollDirection scrlDir )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setSpriteScrollRange)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_STRING( id ) ;
	LQT_FUNC_ARG_INT( nRange ) ;
	LQT_FUNC_ARG_INT( scrlDir ) ;

	// pThis->setSpriteScrollRange(...) ;

	LQT_RETURN_VOID() ;
}

// int getSpriteScrollPageSize( String id, EntisGLS4.Sprite.ScrollDirection scrlDir ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getSpriteScrollPageSize)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_STRING( id ) ;
	LQT_FUNC_ARG_INT( scrlDir ) ;

	LInt32	valRet ;
	// valRet = pThis->getSpriteScrollPageSize(...) ;

	LQT_RETURN_INT( valRet ) ;
}

// void setSpriteScrollPageSize( String id, int nPageSize, EntisGLS4.Sprite.ScrollDirection scrlDir )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setSpriteScrollPageSize)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_STRING( id ) ;
	LQT_FUNC_ARG_INT( nPageSize ) ;
	LQT_FUNC_ARG_INT( scrlDir ) ;

	// pThis->setSpriteScrollPageSize(...) ;

	LQT_RETURN_VOID() ;
}

// boolean isSpriteButtonChecked( String id ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_isSpriteButtonChecked)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_STRING( id ) ;

	LBoolean	valRet ;
	// valRet = pThis->isSpriteButtonChecked(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void checkSpriteButton( String id, boolean flagCheck )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_checkSpriteButton)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_STRING( id ) ;
	LQT_FUNC_ARG_BOOL( flagCheck ) ;

	// pThis->checkSpriteButton(...) ;

	LQT_RETURN_VOID() ;
}

// void setProjectionScreen( const Vector3* vScreen, double zScale, double fpPixelAspect )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setProjectionScreen)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LVector3, vScreen ) ;
	LQT_VERIFY_NULL_PTR( vScreen ) ;
	LQT_FUNC_ARG_DOUBLE( zScale ) ;
	LQT_FUNC_ARG_DOUBLE( fpPixelAspect ) ;

	// pThis->setProjectionScreen(...) ;

	LQT_RETURN_VOID() ;
}

// boolean getProjectionScreen( Vector3* vScreen, double* zScale, double* fpPixelAspect ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getProjectionScreen)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LVector3, vScreen ) ;
	LQT_VERIFY_NULL_PTR( vScreen ) ;
	LQT_FUNC_ARG_POINTER( LDouble, zScale ) ;
	LQT_VERIFY_NULL_PTR( zScale ) ;
	LQT_FUNC_ARG_POINTER( LDouble, fpPixelAspect ) ;
	LQT_VERIFY_NULL_PTR( fpPixelAspect ) ;

	LBoolean	valRet ;
	// valRet = pThis->getProjectionScreen(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void setParallax( double xParallax, double zFocusRate, double xScreenDelta )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setParallax)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_DOUBLE( xParallax ) ;
	LQT_FUNC_ARG_DOUBLE( zFocusRate ) ;
	LQT_FUNC_ARG_DOUBLE( xScreenDelta ) ;

	// pThis->setParallax(...) ;

	LQT_RETURN_VOID() ;
}

// double getParallax( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getParallax)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	LDouble	valRet ;
	// valRet = pThis->getParallax(...) ;

	LQT_RETURN_DOUBLE( valRet ) ;
}

// double getParallaxFocusRatio( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getParallaxFocusRatio)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	LDouble	valRet ;
	// valRet = pThis->getParallaxFocusRatio(...) ;

	LQT_RETURN_DOUBLE( valRet ) ;
}

// double getParallaxScreenX( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getParallaxScreenX)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	LDouble	valRet ;
	// valRet = pThis->getParallaxScreenX(...) ;

	LQT_RETURN_DOUBLE( valRet ) ;
}

// void setVirtualCamera( const Vector3d* vCamera, const Vector3d* vTarget )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setVirtualCamera)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vCamera ) ;
	LQT_VERIFY_NULL_PTR( vCamera ) ;
	LQT_FUNC_ARG_STRUCT( LVector3d, vTarget ) ;
	LQT_VERIFY_NULL_PTR( vTarget ) ;

	// pThis->setVirtualCamera(...) ;

	LQT_RETURN_VOID() ;
}

// const EntisGLS4.Sprite.Virtual3DParam* getVirtual3DParam( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getVirtual3DParam)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	LEntisGLS4_Sprite_Virtual3DParam	valRet ;
	// valRet = pThis->getVirtual3DParam(...) ;

	LQT_RETURN_POINTER_STRUCT( valRet ) ;
}

// void setActionLinearTo( uint msecDuration, uint transparency, const Vector2d* pos, const Vector2d* zoom, double a0, double a1 )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setActionLinearTo)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_UINT( msecDuration ) ;
	LQT_FUNC_ARG_UINT( transparency ) ;
	LQT_FUNC_ARG_STRUCT( LVector2d, pos ) ;
	LQT_VERIFY_NULL_PTR( pos ) ;
	LQT_FUNC_ARG_STRUCT( LVector2d, zoom ) ;
	LQT_VERIFY_NULL_PTR( zoom ) ;
	LQT_FUNC_ARG_DOUBLE( a0 ) ;
	LQT_FUNC_ARG_DOUBLE( a1 ) ;

	// pThis->setActionLinearTo(...) ;

	LQT_RETURN_VOID() ;
}

// void flushAction( )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_flushAction)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	// pThis->flushAction(...) ;

	LQT_RETURN_VOID() ;
}

// void cancelAction( )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_cancelAction)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	// pThis->cancelAction(...) ;

	LQT_RETURN_VOID() ;
}

// boolean isAction( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_isAction)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->isAction(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void pauseAllAction( )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_pauseAllAction)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	// pThis->pauseAllAction(...) ;

	LQT_RETURN_VOID() ;
}

// void restartAllAction( )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_restartAllAction)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	// pThis->restartAllAction(...) ;

	LQT_RETURN_VOID() ;
}

// boolean postCommand( String cmd, long param, long code, int priority, boolean overwritable )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_postCommand)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_STRING( cmd ) ;
	LQT_FUNC_ARG_LONG( param ) ;
	LQT_FUNC_ARG_LONG( code ) ;
	LQT_FUNC_ARG_INT( priority ) ;
	LQT_FUNC_ARG_BOOL( overwritable ) ;

	LBoolean	valRet ;
	// valRet = pThis->postCommand(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean setMouseCapture( )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setMouseCapture)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->setMouseCapture(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean releaseMouseCapture( )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_releaseMouseCapture)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->releaseMouseCapture(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// EntisGLS4.Sprite getMouseCapture( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_getMouseCapture)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	LPtr<LNativeObj>	valRet( new LNativeObj( LQT_GET_CLASS(EntisGLS4.Sprite) ) ) ;
	// valRet = pThis->getMouseCapture(...) ;

	// ToDo: set std::shared_ptr<LEntisGLS4_Sprite> to return
	valRet->SetNative( std::make_shared<LEntisGLS4_Sprite>() ) ;

	LQT_RETURN_OBJECT( valRet ) ;
}

// boolean setKeyFocus( )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_setKeyFocus)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->setKeyFocus(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean killKeyFocus( )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_killKeyFocus)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->killKeyFocus(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean hasKeyFocus( ) const
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_hasKeyFocus)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->hasKeyFocus(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean moveNextKeyFocus( )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_moveNextKeyFocus)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->moveNextKeyFocus(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean movePrevKeyFocus( )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_movePrevKeyFocus)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	LBoolean	valRet ;
	// valRet = pThis->movePrevKeyFocus(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// boolean moveKeyFocusDirectionOf( const Vector2d* vDir )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_moveKeyFocusDirectionOf)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_STRUCT( LVector2d, vDir ) ;
	LQT_VERIFY_NULL_PTR( vDir ) ;

	LBoolean	valRet ;
	// valRet = pThis->moveKeyFocusDirectionOf(...) ;

	LQT_RETURN_BOOL( valRet ) ;
}

// void attachMouseListener( EntisGLS4.SpriteMouseListener listener )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_attachMouseListener)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SpriteMouseListener, listener ) ;
	LQT_VERIFY_NULL_PTR( listener ) ;

	// pThis->attachMouseListener(...) ;

	LQT_RETURN_VOID() ;
}

// void detachMouseListener( EntisGLS4.SpriteMouseListener listener )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_detachMouseListener)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SpriteMouseListener, listener ) ;
	LQT_VERIFY_NULL_PTR( listener ) ;

	// pThis->detachMouseListener(...) ;

	LQT_RETURN_VOID() ;
}

// void attachKeyListener( EntisGLS4.SpriteKeyListener listener )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_attachKeyListener)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SpriteKeyListener, listener ) ;
	LQT_VERIFY_NULL_PTR( listener ) ;

	// pThis->attachKeyListener(...) ;

	LQT_RETURN_VOID() ;
}

// void detachKeyListener( EntisGLS4.SpriteKeyListener listener )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_detachKeyListener)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SpriteKeyListener, listener ) ;
	LQT_VERIFY_NULL_PTR( listener ) ;

	// pThis->detachKeyListener(...) ;

	LQT_RETURN_VOID() ;
}

// void addTimer( EntisGLS4.SpriteTimer timer )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_addTimer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SpriteTimer, timer ) ;
	LQT_VERIFY_NULL_PTR( timer ) ;

	// pThis->addTimer(...) ;

	LQT_RETURN_VOID() ;
}

// void removeTimer( EntisGLS4.SpriteTimer timer )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_removeTimer)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;
	LQT_FUNC_ARG_NOBJ( LEntisGLS4_SpriteTimer, timer ) ;
	LQT_VERIFY_NULL_PTR( timer ) ;

	// pThis->removeTimer(...) ;

	LQT_RETURN_VOID() ;
}

// void removeAllTimers( )
IMPL_LOQUATY_FUNC(EntisGLS4_Sprite_removeAllTimers)
{
	LQT_FUNC_ARG_LIST ;
	LQT_FUNC_THIS_NOBJ( LEntisGLS4_Sprite, pThis ) ;

	// pThis->removeAllTimers(...) ;

	LQT_RETURN_VOID() ;
}



