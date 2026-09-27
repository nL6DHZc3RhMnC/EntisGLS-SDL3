#pragma once
// Thread object declarations ported from the locally supplied EntisGLS3 SDK.
// Copyright (c) 2003-2012 Leshade Entis, Entis-soft. All rights reserved.
#include <sakura/ssys_thread.h>
#include <mutex>

// Host thread transport; script suspend/abort remains cooperative ECSContext logic.
class LegacyThreadHost : public SSystem::SThread {
public:
    DECLARE_CLASS_INFO(LegacyThreadHost, SSystem::SThread)
    LegacyThreadHost();
    virtual ~LegacyThreadHost();
    ESLError BeginThread();
    ESLError CloseThread();
    HANDLE Handle() const { return m_completion; }
protected:
    virtual DWORD ThreadProc() = 0;
private:
    class Procedure : public SSystem::SProcedure {
    public:
        explicit Procedure(LegacyThreadHost* owner) : m_owner(owner) {}
        void Run() override;
    private:
        LegacyThreadHost* m_owner;
    };
    HANDLE m_completion;
    Procedure m_procedure;
};

class	ECSThread	: public ECSObject, public LegacyThreadHost
{
public:
	// 構築関数
	ECSThread( void ) ;
	ECSThread( ECSContext & context ) ;
	// 消滅関数
	virtual ~ECSThread( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO2( ECSThread, ECSObject, LegacyThreadHost )

protected:
	ECSContext *	m_pPrimaryContext ;
	std::recursive_mutex m_controlMutex;
	bool m_abortRequested = false;
	class ThreadContext final : public ECSContext {
	public:
		explicit ThreadContext(ECSThread& owner) : m_owner(owner) {}
		ExecutionStatus SetStatus(ExecutionStatus status) override;
		ESLError ExecuteInstruction() override;
	private:
		ECSThread& m_owner;
	};
	ThreadContext	m_context ;
	DWORD			m_dwSuspendCount ;
	EWideString		m_wstrExceptionFunc ;

	ECSReference	m_refThreadProc ;

	ECSThread *		m_pPrevThread ;
	ECSThread *		m_pNextThread ;

	struct	SAVE_DATA
	{
		ECSContext::ExecutionStatus	nStatus ;
//		DWORD						dwIP ;
		DWORD						dwSuspendCount ;
		DWORD						dwExceptionFuncLen ;
	} ;
	bool		m_fLoadedContext ;
	SAVE_DATA	m_sdRestore ;

public:
	// 実行イメージ関連付け
	virtual ESLError Initialize( ECSContext & context ) ;
	// スレッド関数呼び出し
	virtual ESLError BeginThread
		( DWORD dwFuncAddr, const ECSObjArray<ECSObject> & lstArg ) ;
	ESLError BeginThread
		( const wchar_t * pwszFuncName, const ECSObjArray<ECSObject> & lstArg ) ;
	ESLError BeginThread( ECSObject * pThreadProc ) ;
	// 実行を強制終了させる
	virtual ESLError AbortThread( DWORD dwTimeout = INFINITE ) ;
	// スレッドは実行中か？
	bool IsThreadRunning( void ) const ;
	DWORD GetSuspendCount() const { return __atomic_load_n(&m_dwSuspendCount, __ATOMIC_SEQ_CST); }
	// スクリプトの実行を一時停止する
	virtual ESLError SuspendThread( DWORD dwTimeout = INFINITE ) ;
	// 一時停止中のスクリプトを再開する
	virtual ESLError ResumeThread( void ) ;
	// コンテキスト取得
	ECSContext & GetContext( void )
		{
			return	m_context ;
		}
	// 例外エラー発生時の呼び出し関数を設定する
	void SetExceptionFunction( const wchar_t * pwszFuncName )
		{
			m_wstrExceptionFunc = pwszFuncName ;
		}
	// 例外エラー発生時に呼び出す関数名を取得する
	const wchar_t * GetExceptionFunction( void ) const
		{
			return	m_wstrExceptionFunc ;
		}

protected:
	// スレッド関数
	virtual DWORD ThreadProc( void ) ;

public:
	// オブジェクトの型名を取得する
	virtual const wchar_t * GetTypeName( void ) const ;
	virtual ECSObject * GetTypeOf( const wchar_t * pwszTypeName ) ;
	// オブジェクトを複製
	virtual ECSObject * Duplicate( void ) ;
	// オブジェクトを代入
	virtual ESLError Move( ECSContext & context, ECSObject * obj ) ;
	// 単項演算子
	virtual ESLError UnaryOperate
		( ECSContext & context, CSUnaryOperatorType csuopType ) ;
	// 二項演算子
	virtual ESLError Operate
		( ECSContext & context, CSOperatorType csopType, ECSObject * obj ) ;
	// 比較演算子
	virtual ESLError Compare
		( ECSContext & context, int & nResult,
			CSCompareType cscpType, ECSObject & obj ) ;
	// メンバ変数インデックス取得
//	virtual ESLError GetVariableIndex( int & nIndex, ECSObject & obj ) ;
	virtual ESLError GetVariableIndex( int & nIndex, int iMember ) ;
	virtual ESLError GetVariableIndex
					( int & nIndex, const wchar_t * pwszMember ) ;
	// メンバ変数取得
	virtual ECSObject * GetVariableAt( int nIndex ) ;
	// メンバ変数設定
	virtual ECSObject * SetVariableAt( int nIndex, ECSObject * obj ) ;
	// メンバ関数インデックス取得
	virtual ESLError GetFunction
		( ECSContext & context, int & nIndex, const wchar_t * pwszName ) ;
	// メンバ関数呼び出し
	virtual ESLError CallFunction
		( ECSContext & context,
			int nIndex, ECSObjArray<ECSObject> & lstArg ) ;

public:		// シリアル化のための関数（システムによって必要）
	// 全てのメンバ変数にインデックスを振る
	virtual void IndexAllMember( void ) ;
	// 全てのメンバ変数の参照を解消する
	virtual void CleanupAllReference( ECSContext & context ) ;
	// 全てのメンバ変数の参照を解決する
	virtual ESLError CommitAllReference( ECSContext & context ) ;
	// データを保存
	virtual ESLError Save( ESLFileObject & file, ECSContext & context ) ;
	// データを復元
	virtual ESLError Load( ESLFileObject & file, ECSContext & context ) ;
	// セーブ処理を開始する
	virtual void OnBeginningSave( ECSContext & context ) ;
	// セーブ処理が全て完了した
	virtual void OnFinishedSave( ECSContext & context ) ;
	// ロード処理を開始する
	virtual void OnBeginningLoad( ECSContext & context ) ;
	// ロード処理が全て完了した
	virtual void OnFinishedLoad( ECSContext & context ) ;
	// データをダンプ
	virtual ESLError DumpObject
		( EStreamBuffer & buf, int nIndent, ECSContext & context ) ;

public:
	// メンバ関数プロトタイプ
	typedef	ESLError (ECSThread::*PFUNC_CALL)
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	// メンバ関数ポインタ
	static ECSStrTagArray *	m_staFuncName ;
	static const wchar_t *	m_pwszFuncName[5] ;
	static const PFUNC_CALL	m_pfnCallFunc[4] ;
	// メンバ関数
	ESLError Call_BeginThread
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_IsThreadRunning
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetThreadResult
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetExceptionHandler
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;

	friend class ECSContext ;

} ;


//////////////////////////////////////////////////////////////////////////////
// スレッド待機イベント
//////////////////////////////////////////////////////////////////////////////

class	ECSThreadEvent	: public	ECSObject
{
public:
	// 構築関数
	ECSThreadEvent( void ) ;
	// 消滅関数
	virtual ~ECSThreadEvent( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( ECSThreadEvent, ECSObject )

protected:
	HANDLE		m_hEvent ;
	SDWORD	m_nEvent ;

public:
	// イベントオブジェクト作成
	virtual ESLError CreateEvent( bool fInitState ) ;
	// イベントオブジェクト削除
	virtual ESLError DeleteEvent( void ) ;
	// イベントセット
	virtual void SetEvent( void ) ;
	// イベントリセット
	virtual void ResetEvent( void ) ;
	// イベント待機
	virtual ESLError WaitEvent( DWORD dwTimeout, ECSContext & context ) ;
	// イベント取得
	HANDLE GetEventHandle( void ) const
		{
			return	m_hEvent ;
		}
	SDWORD GetEventState( void ) const
		{
			return	m_nEvent ;
		}

public:
	// オブジェクトの型名を取得する
	virtual const wchar_t * GetTypeName( void ) const ;
	virtual ECSObject * GetTypeOf( const wchar_t * pwszTypeName ) ;
	// オブジェクトを複製
	virtual ECSObject * Duplicate( void ) ;
	// オブジェクトを代入
	virtual ESLError Move( ECSContext & context, ECSObject * obj ) ;
	// 単項演算子
	virtual ESLError UnaryOperate
		( ECSContext & context, CSUnaryOperatorType csuopType ) ;
	// 二項演算子
	virtual ESLError Operate
		( ECSContext & context, CSOperatorType csopType, ECSObject * obj ) ;
	// 比較演算子
	virtual ESLError Compare
		( ECSContext & context, int & nResult,
			CSCompareType cscpType, ECSObject & obj ) ;
	// メンバ関数インデックス取得
	virtual ESLError GetFunction
		( ECSContext & context, int & nIndex, const wchar_t * pwszName ) ;
	// メンバ関数呼び出し
	virtual ESLError CallFunction
		( ECSContext & context,
			int nIndex, ECSObjArray<ECSObject> & lstArg ) ;

public:		// シリアル化のための関数（システムによって必要）
	// データを保存
	virtual ESLError Save( ESLFileObject & file, ECSContext & context ) ;
	// データを復元
	virtual ESLError Load( ESLFileObject & file, ECSContext & context ) ;
	// データをダンプ
	virtual ESLError DumpObject
		( EStreamBuffer & buf, int nIndent, ECSContext & context ) ;

public:
	// メンバ関数プロトタイプ
	typedef	ESLError (ECSThreadEvent::*PFUNC_CALL)
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	// メンバ関数ポインタ
	static ECSStrTagArray *	m_staFuncName ;
	static const wchar_t *	m_pwszFuncName[7] ;
	static const PFUNC_CALL	m_pfnCallFunc[6] ;
	// メンバ関数
	ESLError Call_Create
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Delete
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Wait
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Set
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Reset
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Value
		( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
// スレッド待機（ミューテックス）イベント
//////////////////////////////////////////////////////////////////////////////

class	ECSThreadMutex	: public	ECSThreadEvent
{
public:
	// 構築関数
	ECSThreadMutex( void ) ;
	// 消滅関数
	virtual ~ECSThreadMutex( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( ECSThreadMutex, ECSThreadEvent )

protected:
	ECSReference	m_refOwnerThread ;

public:
	// イベントオブジェクト作成
	virtual ESLError CreateEvent( bool fInitState ) ;
	// イベントオブジェクト削除
	virtual ESLError DeleteEvent( void ) ;
	// イベントセット
	virtual void SetEvent( void ) ;
	// イベントリセット
	virtual void ResetEvent( void ) ;
	// イベント待機
	virtual ESLError WaitEvent( DWORD dwTimeout, ECSContext & context ) ;

public:
	// オブジェクトの型名を取得する
	virtual const wchar_t * GetTypeName( void ) const ;
	virtual ECSObject * GetTypeOf( const wchar_t * pwszTypeName ) ;
	// オブジェクトを複製
	virtual ECSObject * Duplicate( void ) ;
	// オブジェクトを代入
	virtual ESLError Move( ECSContext & context, ECSObject * obj ) ;

public:		// シリアル化のための関数（システムによって必要）
	// 全てのメンバ変数にインデックスを振る
	virtual void IndexAllMember( void ) ;
	// 全てのメンバ変数の参照を解消する
	virtual void CleanupAllReference( ECSContext & context ) ;
	// 全てのメンバ変数の参照を解決する
	virtual ESLError CommitAllReference( ECSContext & context ) ;
	// データを保存
	virtual ESLError Save( ESLFileObject & file, ECSContext & context ) ;
	// データを復元
	virtual ESLError Load( ESLFileObject & file, ECSContext & context ) ;
	// データをダンプ
	virtual ESLError DumpObject
		( EStreamBuffer & buf, int nIndent, ECSContext & context ) ;

} ;

