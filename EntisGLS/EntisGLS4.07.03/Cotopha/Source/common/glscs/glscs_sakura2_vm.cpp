
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/


#include <sakuraglx/sakuraglx.h>
#include <sakura/ssys_module.h>

using	namespace SSystem ;
using	namespace ECSSakura2 ;
using	namespace ECSSakura2Processor ;


//////////////////////////////////////////////////////////////////////////////
// ネイティブ・オブジェクト基底
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ECSSakura2::Object, SObject )

// メモリマッピング
//////////////////////////////////////////////////////////////////////////////
LinearAddressCache *
	Object::GetSegmentBuffer( LinearAddressCache & seg )
{
	return	NULL ;
}

BYTE * Object::GetSegmentShadowBuffer( int iShadow )
{
	return	NULL ;
}

// 破棄処理
//////////////////////////////////////////////////////////////////////////////
void Object::OnDestruction
	( VirtualMachine * vm, Context * context )
{
}

// 保存準備処理
//////////////////////////////////////////////////////////////////////////////
SError Object::PrepareSave
	( VirtualMachine * vm, Context * context )
{
	return	errSuccess ;
}

// 保存処理
//////////////////////////////////////////////////////////////////////////////
SError Object::SaveStatic
	( SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	return	errSuccess ;
}

SError Object::SaveDynamic
	( SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	return	SaveStatic( file, vm, context ) ;
}

// 復元処理
//////////////////////////////////////////////////////////////////////////////
SError Object::LoadStatic
	( SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	return	errSuccess ;
}

SError Object::LoadDynamic
	( SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	return	LoadStatic( file, vm, context ) ;
}

// 復元後処理
//////////////////////////////////////////////////////////////////////////////
SError Object::CommitAfterLoad
	( VirtualMachine * vm, Context * context )
{
	return	errSuccess ;
}

// 復元後の後のスクリプト処理
//////////////////////////////////////////////////////////////////////////////
SError Object::OnLoadedDynamic
	( VirtualMachine * vm, Context * context )
{
	return	errSuccess ;
}

// オブジェクトの上位アドレス取得
//////////////////////////////////////////////////////////////////////////////
DWORD Object::GetHighAddressOf( const ESLObject * pObj )
{
	const Object *	pSakuraObj = ESLConstTypeCast<Object>( pObj ) ;
	if ( pSakuraObj != NULL )
	{
		return	pSakuraObj->m_dwHighAddr ;
	}
	return	0 ;
}



//////////////////////////////////////////////////////////////////////////////
// ネイティブ・オブジェクト・参照（マッピング・ゲート）
//////////////////////////////////////////////////////////////////////////////

#if	!defined(ENTISGLS4_DLL_IMPORT)

// new SSystem::Reference
//////////////////////////////////////////////////////////////////////////////
ECS_LIB_IMPLEMENT_EXPORT_NEW_OBJECT(SSystem_Reference,context,cls_id)
{
	return	new ECSSakura2::ReferenceObject ;
}

#endif

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO2_CAST
	( ECSSakura2::ReferenceObject, ECSSakura2::Object, SSyncReference, m_pReference )

// 全ての参照が解除された
//////////////////////////////////////////////////////////////////////////////
void ReferenceObject::OnReleaseFromReference( void )
{
}

// メモリマッピング
//////////////////////////////////////////////////////////////////////////////
LinearAddressCache *
		ReferenceObject::GetSegmentBuffer( LinearAddressCache & seg )
{
	ECSSakura2::Object *	pRef = ESLTypeCast<ECSSakura2::Object>( m_pReference ) ;
	if ( pRef != NULL )
	{
		return	pRef->GetSegmentBuffer( seg ) ;
	}
	return	NULL ;
}

BYTE * ReferenceObject::GetSegmentShadowBuffer( int iShadow )
{
	ECSSakura2::Object *	pRef = ESLTypeCast<ECSSakura2::Object>( m_pReference ) ;
	if ( pRef != NULL )
	{
		return	pRef->GetSegmentShadowBuffer( iShadow ) ;
	}
	return	NULL ;
}

// 実行時型名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * ReferenceObject::GetTypeName( void ) const
{
	return	L"SSystem::Reference" ;
}

// 破棄処理
//////////////////////////////////////////////////////////////////////////////
void ReferenceObject::OnDestruction
	( VirtualMachine * vm, Context * context )
{
}

// 保存準備処理
//////////////////////////////////////////////////////////////////////////////
SError ReferenceObject::PrepareSave
	( VirtualMachine * vm, Context * context )
{
	ECSSakura2::Object *	pRef = ESLTypeCast<ECSSakura2::Object>( m_pReference ) ;
	if ( pRef != NULL )
	{
		vm->AddClassIdentity( pRef->GetTypeName() ) ;
		return	pRef->PrepareSave( vm, context ) ;
	}
	return	errSuccess ;
}

// 保存処理
//////////////////////////////////////////////////////////////////////////////
SError ReferenceObject::SaveStatic
	( SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	ECSSakura2::Object *	pRef = ESLTypeCast<ECSSakura2::Object>( m_pReference ) ;
	DWORD	dwRefHighAddr = 0 ;
	int	clsid = VirtualMachine::clsidInvalid ;
	if ( pRef != NULL )
	{
		dwRefHighAddr = pRef->m_dwHighAddr ;
		//
		ESLAssert( dwRefHighAddr != 0 ) ;
		if ( dwRefHighAddr == 0 )
		{
			ESLTrace( "無効な参照先 Reference の SaveStatic です" ) ;
			return	errFailed ;
		}
	}
	file->Write( &dwRefHighAddr, sizeof(DWORD) ) ;
	return	errSuccess ;
}

SError ReferenceObject::SaveDynamic
	( SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	ECSSakura2::Object *	pRef = ESLTypeCast<ECSSakura2::Object>( m_pReference ) ;
	DWORD	dwRefHighAddr = 0 ;
	if ( pRef != NULL )
	{
		dwRefHighAddr = pRef->m_dwHighAddr ;
		//
		ESLAssert( dwRefHighAddr != 0 ) ;
		if ( dwRefHighAddr == 0 )
		{
			ESLTrace( "無効な参照先 Reference の SaveDynamic です" ) ;
			return	errFailed ;
		}
	}
	file->Write( &dwRefHighAddr, sizeof(DWORD) ) ;
	return	errSuccess ;
}

// 復元処理
//////////////////////////////////////////////////////////////////////////////
SError ReferenceObject::LoadStatic
	( SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	m_dwLoadedRefHighAddr = 0 ;
	//
	file->Read( &m_dwLoadedRefHighAddr, sizeof(DWORD) ) ;
	//
	return	errSuccess ;
}

SError ReferenceObject::LoadDynamic
	( SFileInterface * file,
		VirtualMachine * vm, Context * context )
{
	m_dwLoadedRefHighAddr = 0 ;
	//
	file->Read( &m_dwLoadedRefHighAddr, sizeof(DWORD) ) ;
	//
	return	errSuccess ;
}

// 復元後処理
//////////////////////////////////////////////////////////////////////////////
SError ReferenceObject::CommitAfterLoad
	( VirtualMachine * vm, Context * context )
{
	if ( m_dwLoadedRefHighAddr != 0 )
	{
		ECSSakura2::Object *	pObj = vm->ObjectFromAddress( m_dwLoadedRefHighAddr ) ;
		if ( pObj != NULL )
		{
			SetReference( pObj ) ;
		}
		else
		{
			ESLTrace( "Reference の参照先を復元出来ませんでした "
						"(%08X)\n", (unsigned int) m_dwLoadedRefHighAddr ) ;
		}
	}
	return	errSuccess ;
}

// 復元後の後のスクリプト処理
//////////////////////////////////////////////////////////////////////////////
SError ReferenceObject::OnLoadedDynamic
	( VirtualMachine * vm, Context * context )
{
	return	errSuccess ;
}



//////////////////////////////////////////////////////////////////////////////
// 抽象仮想マシン
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( ECSSakura2::VirtualMachine, Object )

// 環境設定を取得
//////////////////////////////////////////////////////////////////////////////
SSystem::SEnvironmentInterface * VirtualMachine::GetEnvironment( void ) const
{
	return	SEnvironmentInterface::GetInstance() ;
}

// デフォルトスタックサイズ取得
//////////////////////////////////////////////////////////////////////////////
size_t VirtualMachine::GetDefaultStackSize( void ) const
{
	SSystem::SEnvironmentInterface *	pEnv = GetEnvironment() ;
	ESLAssert( this != NULL ) ;
	if ( pEnv == NULL )
	{
		pEnv = SSystem::SEnvironmentInterface::GetInstance() ;
	}
	if ( pEnv == NULL )
	{
		return	0x1000 ;
	}
	return	pEnv->GetDefaultStackSize() ;
}

// デフォルトヒープサイズ取得
//////////////////////////////////////////////////////////////////////////////
size_t VirtualMachine::GetDefaultHeapSize( void ) const
{
	SSystem::SEnvironmentInterface *	pEnv = GetEnvironment() ;
	ESLAssert( this != NULL ) ;
	if ( pEnv == NULL )
	{
		pEnv = SSystem::SEnvironmentInterface::GetInstance() ;
	}
	if ( pEnv == NULL )
	{
		return	0xF000 ;
	}
	return	pEnv->GetDefaultHeapSize() ;
}

// デバッグ用例外エラー処理
//////////////////////////////////////////////////////////////////////////////
DWORD VirtualMachine::HandleExceptionEscape( Context * context, DWORD maskException )
{
	AtomicAnd( &(context->m_maskException), ~interruptEscape ) ;
	maskException &= ~interruptEscape ;
	return	maskException ;
}

// モジュールがアタッチされた（デバッグ・フック処理用）
//////////////////////////////////////////////////////////////////////////////
void VirtualMachine::OnModuleAttached( int iModule, ExecutableModule * module )
{
}

// モジュールがデタッチされる（デバッグ・フック処理用）
//////////////////////////////////////////////////////////////////////////////
void VirtualMachine::OnModuleDetached( int iModule, ExecutableModule * module )
{
}

// スレッドがアタッチされた（デバッグ・フック処理用）
//////////////////////////////////////////////////////////////////////////////
void VirtualMachine::OnThreadAttached( ThreadObject * thread )
{
}

// スレッドがデタッチされた（デバッグ・フック処理用）
//////////////////////////////////////////////////////////////////////////////
void VirtualMachine::OnThreadDetached( ThreadObject * thread )
{
}

// ファイルを開く
//////////////////////////////////////////////////////////////////////////////
SSystem::SFileInterface * VirtualMachine::NewOpenFile
		( const wchar_t * pwszFilePath, long int nOpenFlags ) const
{
	using namespace SSystem ;
	SSystem::SEnvironmentInterface *	pEnv = GetEnvironment() ;
	ESLAssert( this != NULL ) ;
	if ( pEnv == NULL )
	{
		pEnv = SSystem::SEnvironmentInterface::GetInstance() ;
	}
	if ( pEnv != NULL )
	{
		SEnvironment *	pStdEnv = ESLTypeCast<SEnvironment>( pEnv ) ;
		if ( (pStdEnv != NULL)
			&& (pStdEnv == ESLConstTypeCast<SEnvironment>( this )) )
		{
			return	pStdEnv->SEnvironment::NewOpenFile( pwszFilePath, nOpenFlags ) ;
		}
		else
		{
			return	pEnv->NewOpenFile( pwszFilePath, nOpenFlags ) ;
		}
	}
	else
	{
		return	SSystem::SFileOpener::DefaultNewOpenFile
								( pwszFilePath, nOpenFlags ) ;
	}
}

// ファイルは存在するか？
//////////////////////////////////////////////////////////////////////////////
bool VirtualMachine::IsExistingFile( const wchar_t * pwszFilePath ) const
{
	SSystem::SEnvironmentInterface *	pEnv = GetEnvironment() ;
	ESLAssert( this != NULL ) ;
	if ( pEnv == NULL )
	{
		pEnv = SSystem::SEnvironmentInterface::GetInstance() ;
	}
	if ( pEnv != NULL )
	{
		if ( pEnv->IsExistingFile( pwszFilePath ) )
		{
			return	true ;
		}
	}
	if ( SSystem::SFileOpener::GetDefaultOpener() != NULL )
	{
		return	SSystem::SFileOpener::GetDefaultOpener()->IsExisting( pwszFilePath ) ;
	}
	return	false ;
}

// ファイル状態
//////////////////////////////////////////////////////////////////////////////
SError VirtualMachine::QueryFileState
	( const wchar_t * pszFilePath, SFileOpener::State& state )
{
	SSystem::SEnvironmentInterface *	pEnv = GetEnvironment() ;
	ESLAssert( this != NULL ) ;
	if ( pEnv == NULL )
	{
		pEnv = SSystem::SEnvironmentInterface::GetInstance() ;
	}
	if ( pEnv != NULL )
	{
		if ( !pEnv->QueryFileState( pszFilePath, state ) )
		{
			return	errSuccess ;
		}
	}
	if ( SSystem::SFileOpener::GetDefaultOpener() != NULL )
	{
		return	SSystem::SFileOpener::GetDefaultOpener()->QueryState( pszFilePath, state ) ;
	}
	return	errFailed ;
}

// ファイルパス
//////////////////////////////////////////////////////////////////////////////
SSystem::SString VirtualMachine::OffsetFilePath( const wchar_t * pwszFilePath ) const
{
	SSystem::SEnvironmentInterface *	pEnv = GetEnvironment() ;
	ESLAssert( this != NULL ) ;
	if ( pEnv == NULL )
	{
		pEnv = SSystem::SEnvironmentInterface::GetInstance() ;
	}
	if ( pEnv != NULL )
	{
		return	pEnv->OffsetFilePath( pwszFilePath ) ;
	}
	return	pwszFilePath ;
}

// printf スタイル文字列
//////////////////////////////////////////////////////////////////////////////
void VirtualMachine::FormatStringVlist
	( SSystem::SString& strDst,
		const wchar_t * pwszFormat, const Register * pVarArg )
{
	SStringParser	sparsFormat = pwszFormat ;
	SFormatVarArg	va( this, pVarArg ) ;
	sparsFormat.Format( strDst, va ) ;
}


//////////////////////////////////////////////////////////////////////////////
// printf パラメータ
//////////////////////////////////////////////////////////////////////////////

// 構築関数
//////////////////////////////////////////////////////////////////////////////
VirtualMachine::SFormatVarArg::SFormatVarArg
	( VirtualMachine * vm, const Register * pVarArg )
{
	m_vm = vm ;
	m_pVarArg = pVarArg ;
	m_iNextArg = 0 ;
}

// 次の整数取得
//////////////////////////////////////////////////////////////////////////////
int64_t VirtualMachine::SFormatVarArg::NextInteger( void )
{
	return	m_pVarArg[m_iNextArg ++].i ;
}

// 次の浮動小数点取得
//////////////////////////////////////////////////////////////////////////////
double VirtualMachine::SFormatVarArg::NextDouble( void )
{
	return	m_pVarArg[m_iNextArg ++].f ;
}

// 次の文字取得
//////////////////////////////////////////////////////////////////////////////
wchar_t VirtualMachine::SFormatVarArg::NextCharacter( void )
{
	return	(wchar_t) m_pVarArg[m_iNextArg ++].i ;
}

// 次の文字列取得
//////////////////////////////////////////////////////////////////////////////
const wchar_t * VirtualMachine::SFormatVarArg::NextString( SString& strNext )
{
	uint16_t *	pwszStr =
		(uint16_t*) m_vm->TranslateAddress( m_pVarArg[m_iNextArg ++].i, 2 ) ;
	if ( pwszStr != NULL )
	{
		strNext = pwszStr ;
	}
	else
	{
		strNext = L"(null)" ;
	}
	return	strNext ;
}

