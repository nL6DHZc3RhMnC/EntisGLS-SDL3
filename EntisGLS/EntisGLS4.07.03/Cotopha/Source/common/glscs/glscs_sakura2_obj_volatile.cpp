
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/


#include <sakuraglx/sakuraglx.h>
#include <sakura/ssys_module.h>

using	namespace SSystem ;
using	namespace ECSSakura2 ;
using	namespace ECSSakura2Processor ;


//////////////////////////////////////////////////////////////////////////////
// 揮発性オブジェクト
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ECSSakura2::ECSVolatileObject, Object )

// 実行時型名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ECSVolatileObject::GetTypeName( void ) const
{
	return	L"SSystem::VolatileObject" ;
}

// 保存処理
//////////////////////////////////////////////////////////////////////////////
SSystem::SError ECSVolatileObject::SaveStatic
	( SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	file->Write( &m_addrProc, sizeof(INT64) ) ;
	return	errSuccess ;
}

// 復元処理
//////////////////////////////////////////////////////////////////////////////
SSystem::SError ECSVolatileObject::LoadStatic
	( SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	file->Read( &m_addrProc, sizeof(INT64) ) ;
	return	errSuccess ;
}

// 復元後の後のスクリプト処理
//////////////////////////////////////////////////////////////////////////////
SError ECSVolatileObject::OnLoadedDynamic
	( VirtualMachine * vm, Context * context )
{
	StandardVM *	pVM = ESLTypeCast<StandardVM>( vm ) ;
	if ( (m_addrProc != 0) && (pVM != NULL) )
	{
		pVM->CallVirtualOnSysThread
			( m_addrProc, procVectorOnLoaded,
						(const Register*) &m_addrProc, 1 ) ;
	}
	return	errSuccess ;
}

// インターフェース設定
//////////////////////////////////////////////////////////////////////////////
void ECSVolatileObject::AttachVolatileInterface( INT64 addrProc )
{
	m_addrProc = addrProc ;
}

#if	!defined(ENTISGLS4_DLL_IMPORT)

// new SSystem::VolatileObject
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_NEW_OBJECT( SSystem_VolatileObject, context, cls_id )
{
	return	new ECSSakura2::ECSVolatileObject ;
}

// void SSystem::VolatileObject::AttachVolatileInterface
//						( SVolatileInterface * pInterface ) ;
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_SYSCALL
	( SSystem_VolatileObject_AttachVolatileInterface, context, arg )
{
	ECS_DECLARE_SYSCALL_VM_THIS
		( context, vm, ECSVolatileObject, pObj,
				arg, VolatileObject::AttachVolatileInterface ) ;
	pObj->AttachVolatileInterface( arg[1].i ) ;
	return	NULL ;
}

#endif



//////////////////////////////////////////////////////////////////////////////
// ネイティブ・オブジェクト（ランタイム派生）
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO_CAST( ECSSakura2::RuntimeObject, ECSVolatileObject, m_pObj )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RuntimeObject::RuntimeObject( void )
{
	m_pObj = NULL ;
	m_pwszType = NULL ;
}

RuntimeObject::RuntimeObject( ESLObject * pObj, const wchar_t * pwszType )
{
	m_pObj = pObj ;
	m_pwszType = pwszType ;
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
RuntimeObject::~RuntimeObject( void )
{
	delete	m_pObj ;
	m_pObj = NULL ;
}

// 実行時型名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * RuntimeObject::GetTypeName( void ) const
{
	return	m_pwszType ;
}

// オブジェクト取得
//////////////////////////////////////////////////////////////////////////////
ESLObject * RuntimeObject::GetObject( void ) const
{
	return	m_pObj ;
}


