
#if	!defined(__SAKURA2_FRAME_THREAD_H__)
#define	__SAKURA2_FRAME_THREAD_H__

#include <sakura/ssys_reference_array.h>


namespace	SSystem
{
	class	SFrameThreadGroup ;


	//////////////////////////////////////////////////////////////////////////
	// フレーム・スレッド
	//////////////////////////////////////////////////////////////////////////

	class	SFrameThread	: public SObject, public SThread
	{
	protected:
		SSyncReference	m_refGroup ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO2( SFrameThread, SObject, SThread )
		// 構築関数
		SFrameThread( SFrameThreadGroup * pGroup ) ;
		// 消滅関数
		virtual ~SFrameThread( void ) ;

	public:
		// フレーム駆動スレッド起動
		SError Begin( SProcedure * pProc, size_t nInitStack = 0 ) ;
		// 即時スレッド終了
		SError Terminate( void ) ;
		// フレーム駆動
		SError Continue( void ) ;
		SError SyncContinue( void ) ;
		// フレーム同期
		SError Sync( void ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// フレーム・スレッド・グループ
	//////////////////////////////////////////////////////////////////////////

	class	SFrameThreadGroup	: public SObject
	{
	protected:
		SCriticalSection				m_csOnFrame ;
		SCriticalSection				m_csSync ;
		SReferenceArray<SFrameThread>	m_raThreads ;
		SObjectArray<SFrameThread>		m_aTrashThreads ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( SFrameThreadGroup, SObject )
		// 構築関数
		SFrameThreadGroup( void ) ;
		// 消滅関数
		virtual ~SFrameThreadGroup( void ) ;

	public:
		// スレッド作成
		SSmartRef<SFrameThread>
			BeginThread( SProcedure * pProc, size_t nInitStack = 0 ) ;
		// スレッドを遅延終了させる
		bool DelayTerminate( SFrameThread * pThread ) ;
		// フレーム駆動
		SError SyncFrame( void ) ;
		// グループから離脱する
		bool Detach( SFrameThread * pThread ) ;
		// フレーム駆動中排他処理
		void LockFrame( void ) ;
		void UnlockFrame( void ) ;

	} ;

}

#endif
