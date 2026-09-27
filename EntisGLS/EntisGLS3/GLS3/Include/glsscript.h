
/*****************************************************************************
             Entis Generalized Library System version 3
 ----------------------------------------------------------------------------
	Copyright (c) 2002-2012 Leshade Entis, Entis-soft. All rights reserved.
 ****************************************************************************/


#if	!defined(__GLSSCRIPT_H__)
#define	__GLSSCRIPT_H__


//////////////////////////////////////////////////////////////////////////////
// クラス
//////////////////////////////////////////////////////////////////////////////

class	ECSStrTagArray ;
class	ECSObject ;
	class	ECSReference ;
	class	ECSInteger ;
	class	ECSReal ;
	class	ECSString ;
	class	ECSArray ;
		class	ECSStack ;
		class	ECSGlobal ;
	class	ECSHash ;
		class	ECSStructure ;
class	ECSSourceStream ;
class	ECSAssembler ;
class	ECSCompiler ;
class	ECSContext ;
class	ECSThread ;
class	ECSThreadEvent ;
	class	ECSThreadMutex ;
class	ECSSetup ;

class	ECSResource ;
	class	ECSSprite ;
		class	ECSWindow ;
		class	ECSMessageSprite ;
		class	ECSSuperSprite ;
		class	ECSMovieSprite ;
		class	ECSParticleSprite ;
class	ECSResourceManager ;
class	ECSToneFilter ;
class	ECSInputFilter ;
class	ECSFile ;
class	ECSThread ;
class	ECSThreadEvent ;



//////////////////////////////////////////////////////////////////////////////
// Entis Cotopha Script 外部モジュールインターフェース
//////////////////////////////////////////////////////////////////////////////

#include <ctscriptplugin.h>


//////////////////////////////////////////////////////////////////////////////
// Entis Cotopha Script 命令コード
//////////////////////////////////////////////////////////////////////////////

enum	CSInstructionCode
{
	// 0x00
	csicNew,		csicFree,
	csicLoad,		csicStore,
	csicEnter,		csicLeave,
	csicJump,		csicCJump,
	csicCall,		csicReturn,
	csicElement,	csicElementIndirect,
	csicOperate,	csicUniOperate,
	csicCompare,	/* extended 2.0 */	csicExOperate,
	// 0x10
	csicExUniOperate,		csicExCall,
	csicExReturn,			csicCallMember,
	csicCallNativeMember,	csicSwap,
	/* extended 2.3 */
	csicCreateBuffer,			csicCreateBufferVSize,
	csicPointerToObject,		csicPointerToAddress,
	csicReferenceForPointer,	csicReferenceForObjPointer,
	csicCallFunctionPointer,	csicCallNativeFunction,
	csicMax,
	csicInvalid = csicMax,
} ;

enum	CSObjectMode
{
	csomImmediate,
	csomStack,	csomThis,	csomGlobal,	csomData,	csomAuto,
	csomMax,
	csomArgument = csomMax,	csomHeap, csomHeapShared,
} ;


//////////////////////////////////////////////////////////////////////////////
// スクリプト初期化
//////////////////////////////////////////////////////////////////////////////

namespace	ECotophaScript
{
	extern	CRITICAL_SECTION	g_csCotopha ;

	// 初期化
	void Initialize
		( DWORD dwFlags = ESL_HEAP_NO_SERIALIZE,
						HESLHEAP hImportHeap = NULL ) ;
	// 終了
	void Release( void ) ;
	// プライマリコンテキスト設定
	void SetPrimaryContext( ECSContext * context ) ;
	// プライマリコンテキスト設定
	ECSContext * GetPrimaryContext( void ) ;
	// カレントスレッド設定
	void SetCurrentThread( ECSThread * pThread ) ;
	// カレントスレッド取得
	ECSThread * GetCurrentThread( void ) ;
	// 音声出力オブジェクト生成
	EWaveMixingServer * OpenWaveDevice
		( unsigned int nBufferingTime = 500,
			unsigned int nQuantumTime = 33,
			unsigned int nFrequency = 44100,
			unsigned int nChannels = 2,
			unsigned int nBitsPerSample = 16 ) ;
	EWaveMixingServer * OpenDirectSound
		( unsigned int nBufferingTime = 200,
			unsigned int nQuantumTime = 33,
			unsigned int nFrequency = 44100,
			unsigned int nChannels = 2,
			unsigned int nBitsPerSample = 16 ) ;
	// 音声出力オブジェクト取得
	EWaveMixingServer * GetWaveDevice( void ) ;
	// 画像描画オブジェクト生成
	EGLDrawImage * CreateDrawImage( void ) ;
	// 画像描画オブジェクト取得
	EGLDrawImage * GetDrawImage( void ) ;
	// ヒープメモリ取得
//	HESLHEAP GetHeap( void ) ;
	// スレッド排他アクセス権取得
	inline void Lock( void )
		{
			::EnterCriticalSection( &g_csCotopha ) ;
		}
	// スレッド排他アクセス権解放
	inline void Unlock( void )
		{
			::LeaveCriticalSection( &g_csCotopha ) ;
		}
	// Reference 排他アクセス用
	void MultithreadReference( bool fMultithread ) ;
	void LockReference( void ) ;
	void UnlockReference( void ) ;
} ;


//////////////////////////////////////////////////////////////////////////////
// 詞葉3.0 仮想マシン
//////////////////////////////////////////////////////////////////////////////

#include <sakura/sakura.h>
#include <sakura/ssys_module.h>


//////////////////////////////////////////////////////////////////////////////
// スクリプト基本オブジェクト
//////////////////////////////////////////////////////////////////////////////

#include <glsscriptobj.h>


//////////////////////////////////////////////////////////////////////////////
// Entis Cotopha Script 実行環境オブジェクト
//////////////////////////////////////////////////////////////////////////////

#include <glscs_environment.h>



//////////////////////////////////////////////////////////////////////////////
// Entis Cotopha Script 実行イメージ
//////////////////////////////////////////////////////////////////////////////

class	ECSExecutionImage ;
class	ECSExecutionImageLinker ;
class	ECSExecutionImageCompiler ;
class	ECSExecutionReverseAssembler ;
class	ECSExecutionOptimizer ;

#include <glscs_execution_image.h>
#include <glscs_execution_image_linker.h>
#include <glscs_execution_image_compiler.h>
#include <glscs_execution_reverse_assembler.h>
#include <glscs_execution_optimizer.h>


//////////////////////////////////////////////////////////////////////////////
// Entis Cotopha Script 実行コンテキスト
//////////////////////////////////////////////////////////////////////////////

#include <glscs_context.h>


//////////////////////////////////////////////////////////////////////////////
// Entis Cotopha Script コンパイラ
//////////////////////////////////////////////////////////////////////////////

#include <glscs_assembler.h>
#include <glscs_compiler.h>


//////////////////////////////////////////////////////////////////////////////
// Entis Cotopha Script C style コンパイラ
//////////////////////////////////////////////////////////////////////////////

#include <glscs_compiler_c_style.h>


#endif
