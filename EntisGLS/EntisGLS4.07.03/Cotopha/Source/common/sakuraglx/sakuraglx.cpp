
#include <loquaty.h>
#include <sakuraglx/sakuraglx.h>

#if	!defined(__COTOPHA__)
#include <sakura/ssys_heap_memory.h>
#include <sakuraglx/sglx3d_render.h>
#include <sakuraglx/sglx_sprite_lib.h>
#include <sakuraglx/sprite/sglx_resource_manager.h>
#include <sakuraglx/sprite/sglx_sprite_formed.h>
#include <sakuraglx/sprite/sglx_sprite_filter.h>
#include <sakuraglx/sprite/sglx_sprite_rectangle.h>
#include <sakuraglx/sprite/sglx_sprite_frame.h>
#include <sakuraglx/sprite/sglx_sprite_text.h>
#include <sakuraglx/sprite/sglx_sprite_edit.h>
#include <sakuraglx/sprite/sglx_sprite_progress_bar.h>
#include <sakuraglx/sprite/sglx_sprite_button.h>
#include <sakuraglx/sprite/sglx_sprite_scroll_bar.h>
#include <sakuraglx/sprite/sglx_sprite_scroller.h>
#include <sakuraglx/sprite/sglx_sprite_message.h>
#include <sakuraglx/sprite/sglx_sprite_movie.h>
#include <sakuraglx/sprite/sglx_sprite_list.h>
#include <sakuraglx/sprite/sglx_virtual_input.h>
#include <sakuraglx/sglx_platform_ui.h>
#include <sakuraglx/render/sglx3d_scene_composer.h>
#include <sakuraglx/render/sglx3d_scene_item.h>
#include <sakuraglx/render/sglx3d_scene_vr_item.h>
#include <sakuraglx/render/sglx3d_scene_item_curve.h>
#include <sakuraglx/render/sglx3d_scene_edit_mesh.h>
#include <sakuraglx/render/sglx3d_scene_effector.h>
#include <sakuraglx/render/sglx3d_scene_rsrc_proc.h>
#include <antirrhinum/antirrhinum.h>

static struct
{
	const wchar_t *	pwszClassName ;
	ulong_ptr_t		pfnNewObject ;
}	g_tableNewObjectFunc[] =
{
#include <sakuraglx/sglx_new_object_entries.h>
} ;

ESL_DLL_DECL( SakuraGL::SGLObject::ObjectCreatorPair*
				SakuraGL::SGLObject::s_pFirstObjectCreatorPair = nullptr ) ;

#endif

using namespace SSystem ;
using namespace SakuraGL ;



//////////////////////////////////////////////////////////////////////////////
// 基底クラス
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
SGL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLObject, SObject )

// 複製（可能なら）
//////////////////////////////////////////////////////////////////////////////
SGLObject * SGLObject::DuplicateObject( void )
{
	return	NULL ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
SGLError SGLObject::OnSave( SSystem::SFileInterface& file )
{
	return	sglErrSuccess ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SGLError SGLObject::OnRestore( SSystem::SFileInterface& file )
{
	return	sglErrSuccess ;
}

// 復元後処理
//////////////////////////////////////////////////////////////////////////////
SGLError SGLObject::OnAfterRestore( void )
{
	return	sglErrSuccess ;
}

// 実行時クラス名からクラスオブジェクト生成
//////////////////////////////////////////////////////////////////////////////
ESLObject * SGLObject::NewObject( const wchar_t * pszType )
{
#if	defined(__COTOPHA__)
	SString	strFuncName = pszType ;
	strFuncName += L"::NewESLObject" ;
	ulong_ptr_t	ptrNewFunc = GetModuleExportFunction( strFuncName ) ;
#else
	const size_t	countClasses =
			sizeof(g_tableNewObjectFunc) / sizeof(g_tableNewObjectFunc[0]) ;
	ssize_t		iFirst = 0 ;
	ssize_t		iEnd = (ssize_t) countClasses - 1 ;
	ulong_ptr_t	ptrNewFunc = 0 ;
	while ( iFirst < iEnd )
	{
		const ssize_t	iMiddle = ((iFirst + iEnd) >> 1) ;
		const wchar_t *	pwszClassName =
							g_tableNewObjectFunc[iMiddle].pwszClassName ;
		//
		int	nCompare = SString::Compare( pwszClassName, pszType ) ;
		if ( nCompare > 0 )
		{
			iEnd = iMiddle - 1 ;
		}
		else if ( nCompare < 0 )
		{
			iFirst = iMiddle + 1 ;
		}
		else
		{
			ptrNewFunc = g_tableNewObjectFunc[iMiddle].pfnNewObject ;
			break ;
		}
	}
	if ( ptrNewFunc == 0 )
	{
		const ObjectCreatorPair *	pPair = s_pFirstObjectCreatorPair ;
		while ( pPair != nullptr )
		{
			if ( SString(pPair->pszType) == pszType )
			{
				ptrNewFunc = (ulong_ptr_t) pPair->pfnNewObj ;
				break ;
			}
			pPair = pPair->pNext ;
		}
	}
#endif
	if ( ptrNewFunc == 0 )
	{
		return	NULL ;
	}
	ESLRuntimeClass::PFUNC_NEW_OBJECT
		pfnNewFunc = (ESLRuntimeClass::PFUNC_NEW_OBJECT) ptrNewFunc ;
	return	pfnNewFunc() ;
}

// クラスオブジェクト保存
//////////////////////////////////////////////////////////////////////////////
SGLError SGLObject::SaveObject
	( SGLObject * pObj, SSystem::SFileInterface& file )
{
	//
	// クラス名保存
	//
	SString	strClassName ;
	if ( pObj == NULL )
	{
		return	(SGLError) file.WriteString( strClassName ) ;
	}
	strClassName = pObj->GetESLClassName() ;
	SGLError	err = (SGLError) file.WriteString( strClassName ) ;
	if ( err )
	{
		return	err ;
	}
	//
	// オブジェクト保存
	//
	int64_t		nFilePos = file.GetPosition() ;
	uint32_t	nObjBytes = 0x80000000 ;
	file.Write( &nObjBytes, sizeof(uint32_t) ) ;
	//
	err = pObj->OnSave( file ) ;
	//
	int64_t		nEndOfData = file.GetPosition() ;
	if ( (nEndOfData - nFilePos < 0x80000000)
			&& (nEndOfData >= 0) && (nFilePos >= 0) )
	{
		if ( file.Seek( nFilePos ) == nFilePos )
		{
			nObjBytes = (uint32_t) (nEndOfData - nFilePos - sizeof(uint32_t)) ;
			file.Write( &nObjBytes, sizeof(uint32_t) ) ;
		}
		file.Seek( nEndOfData ) ;
	}
	return	err ;
}

// クラスオブジェクト保存
//////////////////////////////////////////////////////////////////////////////
SGLObject * SGLObject::LoadObject( SSystem::SFileInterface& file )
{
	//
	// クラス名読み込み
	//
	SString	strClassName ;
	if ( file.ReadString( strClassName ) )
	{
		return	NULL ;
	}
	if ( strClassName.IsEmpty() )
	{
		return	NULL ;
	}
	//
	// オブジェクト生成
	//
	ESLObject *	pObj = NewObject( strClassName ) ;
	if ( pObj == NULL )
	{
		#if	defined(__DEBUG__)
		Trace( "failed new object \'%s\' class.\n",
					strClassName.ToCharArray().GetConstArray() ) ;
		#endif
		return	NULL ;
	}
	SGLObject *	pSGLObj = ESLTypeCast<SGLObject>( pObj ) ;
	if ( pSGLObj == NULL )
	{
		#if	defined(__DEBUG__)
		SArray<char>	bufClassName ;
		Trace( "%s is not SGLObject class.\n",
					strClassName.EncodeDefaultTo(bufClassName) ) ;
		#endif
		delete	pObj ;
		return	NULL ;
	}
	//
	// オブジェクト復元
	//
	uint32_t	nObjBytes = 0x80000000 ;
	if ( file.Read( &nObjBytes, sizeof(uint32_t) ) < sizeof(uint32_t) )
	{
		ESLTrace( "failed to read SGLObject data bytes.\n" ) ;
		return	NULL ;
	}
	int64_t	nFilePos = file.GetPosition() ;
	//
	if ( pSGLObj->OnRestore( file ) )
	{
		#if	defined(__DEBUG__)
		SArray<char>	bufClassName ;
		Trace( "failed to restore object of %s class.\n",
					strClassName.EncodeDefaultTo(bufClassName) ) ;
		#endif
		delete	pSGLObj ;
		return	NULL ;
	}
	if ( !(nObjBytes & 0x80000000) && (nFilePos >= 0) )
	{
		file.Seek( nFilePos + nObjBytes ) ;
	}
	return	pSGLObj ;
}

#if	!defined(__COTOPHA__)
// 実行時オブジェクト生成関数登録
//////////////////////////////////////////////////////////////////////////////
#if !defined(ENTISGLS4_DLL_IMPORT)
ESLRuntimeClass::PFUNC_NEW_OBJECT
	SGLObject::RegisterObjectCreator
		( const char * pszType,
			ESLRuntimeClass::PFUNC_NEW_OBJECT pfnNewObj )
{
	ObjectCreatorPair *	pPair =
			(ObjectCreatorPair*) malloc( sizeof(ObjectCreatorPair) ) ;
	pPair->pszType = pszType ;
	pPair->pfnNewObj = pfnNewObj ;
	pPair->pNext = s_pFirstObjectCreatorPair ;
	pPair->pfnFreePair = &free ;

	s_pFirstObjectCreatorPair = pPair ;

	return	pfnNewObj ;
}
#endif

// 実行時オブジェクト生成関数登録情報削除
//////////////////////////////////////////////////////////////////////////////
void SGLObject::UnregisterAllObjectCreator( void )
{
	ObjectCreatorPair *	pPair = s_pFirstObjectCreatorPair ;
	while ( pPair != nullptr )
	{
		ObjectCreatorPair *	pNext = pPair->pNext ;
		(pPair->pfnFreePair)( pPair ) ;
		pPair = pNext ;
	}
	s_pFirstObjectCreatorPair = nullptr ;
}

#endif


//////////////////////////////////////////////////////////////////////////////
// ポインタ保存用マッパー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( SakuraGL::SGLObjectSavingMapper, SObject )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
SGLObjectSavingMapper::SGLObjectSavingMapper( void )
{
}

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
SGLObjectSavingMapper::~SGLObjectSavingMapper( void )
{
	if ( GetCurrent() == this )
	{
		DetachCurrentThread() ;
	}
}

// ポインタ登録
//////////////////////////////////////////////////////////////////////////////
SGLError SGLObjectSavingMapper::RegisterObject
				( const wchar_t * pwszID, ESLObject * pObj )
{
	size_t	count = m_mapper.GetLength() ;
	for ( size_t i = 0; i < count; i ++ )
	{
		Entry *	e = m_mapper.GetAt( i ) ;
		if ( e != NULL )
		{
			if ( e->m_pPointer == pObj )
			{
				e->m_strID = pwszID ;
				return	sglErrSuccess ;
			}
			if ( e->m_strID == pwszID )
			{
				e->m_pPointer = pObj ;
				return	sglErrSuccess ;
			}
		}
	}
	Entry *	e = new Entry ;
	e->m_strID = pwszID ;
	e->m_pPointer = pObj ;
	m_mapper.Add( e ) ;
	return	sglErrSuccess ;
}

// 登録識別子検索
//////////////////////////////////////////////////////////////////////////////
const wchar_t * SGLObjectSavingMapper::GetIdentityOf( ESLObject * pObj ) const
{
	size_t	count = m_mapper.GetLength() ;
	for ( size_t i = 0; i < count; i ++ )
	{
		Entry *	e = m_mapper.GetAt( i ) ;
		if ( e != NULL )
		{
			if ( e->m_pPointer == pObj )
			{
				return	e->m_strID ;
			}
		}
	}
	return	NULL ;
}

// 登録ポインタ検索
//////////////////////////////////////////////////////////////////////////////
ESLObject * SGLObjectSavingMapper::GetObjectOf( const wchar_t * pwszID ) const
{
	size_t	count = m_mapper.GetLength() ;
	for ( size_t i = 0; i < count; i ++ )
	{
		Entry *	e = m_mapper.GetAt( i ) ;
		if ( e != NULL )
		{
			if ( e->m_strID == pwszID )
			{
				return	e->m_pPointer ;
			}
		}
	}
	return	NULL ;
}

// クラスオブジェクト保存
//////////////////////////////////////////////////////////////////////////////
SGLError SGLObjectSavingMapper::SaveObject
	( SSystem::SFileInterface& file, SGLObject * pObj, bool fEnableNewObject )
{
	SString	strID = GetIdentityOf( pObj ) ;
	file.WriteString( strID ) ;
	//
	if ( strID.IsEmpty() )
	{
		if ( fEnableNewObject )
		{
			return	SGLObject::SaveObject( pObj, file ) ;
		}
		if ( pObj != NULL )
		{
			return	sglErrFailed ;
		}
	}
	return	sglErrSuccess ;
}

// クラスオブジェクト復元
//////////////////////////////////////////////////////////////////////////////
SGLObject * SGLObjectSavingMapper::LoadObject
		( SSystem::SFileInterface& file, bool fEnableNewObject )
{
	SString	strID ;
	if ( file.ReadString( strID ) )
	{
		return	NULL ;
	}
	if ( !strID.IsEmpty() )
	{
		SGLObject *	pObj = ESLTypeCast<SGLObject>( GetObjectOf( strID ) ) ;
		return	pObj ;
	}
	if ( fEnableNewObject )
	{
		return	SGLObject::LoadObject( file ) ;
	}
	return	NULL ;
}

// 現在のスレッドに関連付けられたマッパー取得
//////////////////////////////////////////////////////////////////////////////
SGLObjectSavingMapper * SGLObjectSavingMapper::GetCurrent( void )
{
	return	ESLTypeCast<SGLObjectSavingMapper>
				( SThread::GetLocalStorageAs( L"SGLObjectSavingMapper" ) ) ;
}

// 現在のスレッドに関連付ける
//////////////////////////////////////////////////////////////////////////////
void SGLObjectSavingMapper::AttachCurrentThread( void )
{
	SThread::SetLocalStorageAs( L"SGLObjectSavingMapper", this ) ;
}

// 現在のスレッドから解除する
//////////////////////////////////////////////////////////////////////////////
void SGLObjectSavingMapper::DetachCurrentThread( void )
{
	SThread::SetLocalStorageAs( L"SGLObjectSavingMapper", NULL ) ;
}

