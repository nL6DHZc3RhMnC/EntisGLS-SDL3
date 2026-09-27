
/*****************************************************************************
				詞葉環境・Sakura2 仮想マシン
 *****************************************************************************/

#if	!defined(__GLSCS_SAKURA2_OBJECT_VM_H__)
#define	__GLSCS_SAKURA2_OBJECT_VM_H__

namespace	ECSSakura2
{
	//////////////////////////////////////////////////////////////////////////
	// Sakura2 仮想マシンオブジェクト
	//////////////////////////////////////////////////////////////////////////

	class	Sakura2VMObject	: public EnvironmentVM
	{
	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( Sakura2VMObject, EnvironmentVM )
		// 構築関数
		Sakura2VMObject( void ) ;
		// 消滅関数
		virtual ~Sakura2VMObject( void ) ;

	public:
		enum	PackageType
		{
			packageInvalid	= -1,
			packageFile,
			packageArchive,
		} ;
	protected:
		PackageType				m_typePackage ;
		SSystem::SString		m_strPackageFile ;
		SSystem::SString		m_strMainArg ;
		SSystem::SSignalEvent	m_signalExit ;
		int64_t					m_codeExit ;

	public:
		// 仮想マシンを起動する
		virtual SSystem::SError OpenVM
			( const wchar_t * pwszFilePath,
				const wchar_t * pwszArg = NULL,
				PackageType typePackage = packageFile ) ;
		// 仮想マシンを閉じる
		virtual void ReleaseVM( void ) ;
		// 仮想マシンの終了コードを取得する
		virtual SSystem::SError GetExitCode
			( int64_t& codeExit,
				int64_t msecTimeout = SSystem::Synchronism::Infinite ) ;

	protected:
		// 仮想マシンを読み込む（main を実行はしない）
		virtual SSystem::SError LoadVM
			( const wchar_t * pwszFilePath,
						PackageType typePackage ) ;
		// main 関数スレッド開始
		SSystem::SError BeginVMMain( const wchar_t * pwszArg ) ;
		// main 関数スレッド継続
		SSystem::SError ContinueVMMain( void ) ;
		// スレッド関数
		static void VMMainThreadProc( void * pInstance ) ;
		static void VMMainContinueThreadProc( void * pInstance ) ;
		void VMMainProc( void ) ;
		void VMMainContinueProc( void ) ;

	public:	// Object オーバーライド
		// 保存処理
		virtual SError SaveStatic
			( SFileInterface * file,
				VirtualMachine * vm, Context * context ) ;
		virtual SError SaveDynamic
			( SFileInterface * file,
				VirtualMachine * vm, Context * context ) ;
		// 復元処理
		virtual SError LoadStatic
			( SFileInterface * file,
				VirtualMachine * vm, Context * context ) ;
		virtual SError LoadDynamic
			( SFileInterface * file,
				VirtualMachine * vm, Context * context ) ;
		// 復元後の後のスクリプト処理
		virtual SError OnLoadedDynamic
			( VirtualMachine * vm, Context * context ) ;

	protected:
		// パッケージ保存処理
		SError SaveVMPackageFile( SFileInterface * file ) ;
		// パッケージ復元処理
		SError LoadVMPackageFile( SFileInterface * file ) ;

	} ;

}


// new SSystem::Sakura2VM
ECS_LIB_DECLARE_EXPORT_NEW_OBJECT(SSystem_Sakura2VM) ;

// SSystem::SError SSystem::Sakura2VM::OpenVM
//	( const char * pszFilePath,
//		const char * pszArg = NULL,
//		PackageType typePackage = packageFile ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_Sakura2VM_OpenVM) ;

// void SSystem::Sakura2VM::ReleaseVM( void ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_Sakura2VM_ReleaseVM) ;

// SSystem::SError SSystem::Sakura2VM::GetExitCode
//  ( int64_t& codeExit,
//		int64_t msecTimeout = SSystem::Synchronism::Infinite ) ;
ECS_LIB_DECLARE_EXPORT_SYSCALL(SSystem_Sakura2VM_GetExitCode) ;


#endif
