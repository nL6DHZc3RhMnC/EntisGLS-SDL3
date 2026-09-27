
#if	!defined(__SAKURA2_MODULE_H__)
#define	__SAKURA2_MODULE_H__

#if	!defined(__COTOPHA__)
#include <glscs/glscs_sakura2_processor.h>
#include <glscs/glscs_sakura2_vm.h>
#include <glscs/glscs_sakura2_obj_volatile.h>
#include <glscs/glscs_sakura2_array.h>
#include <glscs/glscs_sakura2_buffer.h>
#include <glscs/glscs_sakura2_heap_buffer.h>
#include <glscs/glscs_sakura2_obj_heap.h>
#include <glscs/glscs_sakura2_obj_thread.h>
#include <glscs/glscs_sakura2_jit_native_compiler.h>
#include <glscs/glscs_sakura2_std_vm.h>
#include <glscs/glscs_sakura2_env_vm.h>
#include <glscs/glscs_sakura2_obj_vm.h>
#endif


namespace	SSystem
{
	//////////////////////////////////////////////////////////////////////////
	// モジュール
	//////////////////////////////////////////////////////////////////////////

	#if	defined(__COTOPHA__)
	class	native Module : public VolatileObject
	{
	public:
		// モジュールを読み込む
		native SError LoadModule( const wchar_t * pszFileName ) ;
		native SError ReadModule( File * pFile ) ;
		// 関数アドレスを取得する
		native uint64_t FindFunction
			( const wchar_t * pszFuncName,
					const void * ptrReserved = NULL ) const ;
		// 変数アドレスを取得する
		native uint64_t FindVariable
			( const wchar_t * pszVarName,
					const void * ptrReserved = NULL ) const ;
	} ;
	#endif

	class	SModule	: public ESLObject
	{
	protected:
		#if	defined(__COTOPHA__)
			Module *	m_pModule ;
		#else
			bool							m_fLoaded ;
			ECSSakura2::StandardVM *		m_pVM ;
			ECSSakura2::StandardVM *		m_pOwnerVM ;
			ECSSakura2::ExecutableModule	m_module ;
		#endif

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SModule, ESLObject )
		// 構築関数
		SModule( void ) ;
		// 消滅関数
		virtual ~SModule( void ) ;

	public:
		// モジュールを読み込む
		SError LoadModule( const wchar_t * pszFileName ) ;
		SError ReadModule( SFileInterface * pFile ) ;
		// 関数アドレスを取得する
		uint64_t FindFunction
			( const wchar_t * pszFuncName,
				const void * ptrReserved = NULL ) const ;
		// 変数アドレスを取得する
		uint64_t FindVariable
			( const wchar_t * pszVarName,
				const void * ptrReserved = NULL ) const ;
		// 関数を呼び出す
		int64_t CallFunction
			( uint64_t pfnFuncAddr,
				const int64_t * pArg, size_t nArgCount ) ;
	} ;


	#if	!defined(__COTOPHA__)
	typedef	SModule	Module ;
	#endif


	//////////////////////////////////////////////////////////////////////////
	// 仮想マシン
	//////////////////////////////////////////////////////////////////////////

	#if	defined(__COTOPHA__)
	class	native Sakura2VM
	{
	public:
		enum	PackageType
		{
			packageInvalid	= -1,
			packageFile,
			packageArchive,
		} ;
		// 仮想マシンを起動する
		native SError OpenVM
			( const wchar_t * pwszFilePath,
				const wchar_t * pwszArg = NULL,
				PackageType typePackage = packageFile ) ;
		// 仮想マシンを閉じる
		native void ReleaseVM( void ) ;
		// 仮想マシンの終了コードを取得する
		native SError GetExitCode
			( int64_t& codeExit,
				int64_t msecTimeout = Synchronism::Infinite ) ;
	} ;
	#else
	typedef	ECSSakura2::Sakura2VMObject	Sakura2VM ;
	#endif

}


#endif

