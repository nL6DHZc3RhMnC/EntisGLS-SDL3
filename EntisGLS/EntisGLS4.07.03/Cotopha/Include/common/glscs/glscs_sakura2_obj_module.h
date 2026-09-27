
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/

#if	!defined(__GLSCS_SAKURA2_OBJECT_MODULE_H__)
#define	__GLSCS_SAKURA2_OBJECT_MODULE_H__

namespace	ECSSakura2
{
	//////////////////////////////////////////////////////////////////////////
	// ファイル・オブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	ModuleObject
				: public ECSVolatileObject, public ExecutableModule
	{
	protected:
		bool				m_flagLoaded ;
		SSystem::SString	m_strFilePath ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2
				( ModuleObject, ECSVolatileObject, ExecutableModule )
		// 構築関数
		ModuleObject( void ) ;
		// 読み込み
		SError LoadModule
			( StandardVM * vm,
				const wchar_t * pszFileName, int iModule = -1 ) ;
		SError ReadModule
			( StandardVM * vm, SSystem::SFileInterface * pFile ) ;
		// モジュールを仮想マシンにロード
		SError LoadModuleOnVM( StandardVM * vm, int iModule = -1 ) ;
		// モジュール解放
		void FreeModuleOnVM( StandardVM * vm ) ;

	public:
		// 実行時型名
		virtual const wchar_t * GetTypeName( void ) const ;
		// 破棄処理
		virtual void OnDestruction
			( VirtualMachine * vm, Context * context ) ;
		// 保存処理
		virtual SError SaveStatic
			( SFileInterface * file,
				VirtualMachine * vm, Context * context ) ;
		// 復元処理
		virtual SError LoadStatic
			( SFileInterface * file,
				VirtualMachine * vm, Context * context ) ;

	} ;

}


//////////////////////////////////////////////////////////////////////////////
// SSystem::Module スタブ
//////////////////////////////////////////////////////////////////////////////

// new SSystem::Module
ECS_LIB_DECLARE_EXPORT_NEW_OBJECT(SSystem_Module) ;

// SError Module::LoadModule( const wchar_t * pszFileName ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_Module_LoadModule) ;

// SError Module::ReadModule( File * pFile ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_Module_ReadModule) ;

// uint64_t FindFunction( const wchar_t * pszFuncName ) const ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_Module_FindFunction) ;

// uint64_t FindVariable( const wchar_t * pszVarName ) const ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_Module_FindVariable) ;


#endif
