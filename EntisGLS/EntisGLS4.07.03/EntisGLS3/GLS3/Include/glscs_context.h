
//////////////////////////////////////////////////////////////////////////////
// Entis Cotopha Script 実行コンテキスト
//////////////////////////////////////////////////////////////////////////////

class	ECSContext	: public ECSSakura2Processor::ContextShell
{
public:
	// 構築関数
	ECSContext( void ) ;
	// 消滅関数
	virtual ~ECSContext( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( ECSContext, ECSSakura2Processor::ContextShell )

public:
//	ECSEnvironment *		m_pEnv ;		// 環境設定オブジェクト
	HMODULE					m_module ;		// EXE モジュールハンドル
	ECSExecutionImage *		m_pcsxi ;		// 実行イメージ
	ECSStack				m_stack ;		// 実行スタック
	INT64					m_vaStack ;		// 実行スタック仮想アドレス
	ECSArray				m_arg ;			// 関数引数
//	DWORD					m_ip ;			// 命令ポインタ
	ECSObject *				m_pRetObj ;		// 返り値
//	ExecutionStatus			m_status ;		// 実行ステータス
	EString					m_strErrMsg ;	// エラーメッセージ

	// naked モード用スタック
	ECSBuffer *				m_bufNakedStack ;
	INT64					m_vaNakedStack ;


	struct	PLUGIN_CONTEXT_HEADER
	{
		ECSContext *	pBackLink ;
		void *			ptrReserved ;
	} ;
	struct	PLUGIN_CONTEXT
		: public PLUGIN_CONTEXT_HEADER, public ECS_CONTEXT { } ;
	PLUGIN_CONTEXT *	m_ppic ;	// プラグイン用インターフェース

	static ECSContext * ContextFromPlugin( ECS_CONTEXT * context )
	{
		PLUGIN_CONTEXT *	ppic = (PLUGIN_CONTEXT*) context ;
		ESLAssert( ppic->pBackLink->m_ppic == ppic ) ;
		return	ppic->pBackLink ;
	}

protected:
	UINT64					m_nLastTickTime ;	// 最近のタイムカウンタ
	DWORD					m_dwLastBaseTime ;	// ベース時間

	ECSString				m_strThis ;			// "this" 固定文字列
	HANDLE					m_hAbortEvent ;		// 中断イベント
	HANDLE					m_hSuspended ;		// 実行が一時停止されているか？
	HANDLE					m_hStatusEvent ;	// 実行ステータスが変更された
	HANDLE					m_hExecutionMutex ;	// 実行コンテキスト排他処理権
	LONG					m_nLockedCount ;	// 実行コンテキストがロックされた回数

	ECSThread *				m_pThreadList ;		// 実行中のスレッドリスト

	template <class T> class ETemporaryStack : public EObjArray<T>
	{
	public:
		T * Create( void )
			{
				T *	p = Pop() ;
				if ( p == NULL )
				{
					p = new T ;
				}
				return	p ;
			}
		void Delete( T * p )
			{
				if ( GetSize() >= 128 )
				{
					delete	p ;
				}
				else
				{
					p->ReleaseBackLink() ;
					Push( p ) ;
				}
			}
	} ;
	ETemporaryStack<ECSReference>			m_tsbufReference ;
	ETemporaryStack<ECSInteger>				m_tsbufInteger ;
	ETemporaryStack<ECSReal>				m_tsbufReal ;
	ETemporaryStack<ECSString>				m_tsbufString ;
	ETemporaryStack<ECSPointer>				m_tsbufPointer ;
	ETemporaryStack<ECSPointerReference>	m_tsbufPointerRef ;

	ETagSortArray<ECSWideString,ECSString>	m_wstaConstStrBuf ;

public:
	// 実行コンテキスト初期化
	virtual ESLError InitializeContext
		( ECSExecutionImage * pcsxi, bool fInitializeGlobal = true ) ;
	// 初期化関数呼び出し
	virtual ESLError CallPrologueFunctions( void ) ;
	// 実行コンテキストのリソース解放
	virtual void ReleaseContext( bool fReleaseGlobal = true ) ;
	// エピローグ関数呼び出し
	virtual ESLError CallEpilogueFunctions( void ) ;
	// システムコンテキストを取得
	ECSContext * GetSystemContext( void )
		{
			if ( m_pcsxi != NULL )
			{
				return	m_pcsxi->GetSystemContext() ;
			}
			return	this ;
		}
	// 関数呼び出し
	virtual ESLError CallFunction
		( DWORD dwFuncAddr, const ECSObjArray<ECSObject> & lstArg ) ;
	virtual ESLError CallNakedFunction
		( DWORD dwFuncAddr, const INT64 *pArg, int nArgCount ) ;
	// 実行継続
	virtual ESLError ResumeExecution
		( ExecutionStatus xsStatus = xsExecution ) ;
	// １命令実行
	virtual ESLError ExecuteInstruction( void ) ;
	// スタックへオブジェクトをプッシュ
	ESLError PushObject( ECSObject * pObj )
		{
			return	m_stack.PushObject( pObj ) ;
		}
	// スタックからオブジェクトをポップ
	ECSObject * PopObject( void )
		{
			return	m_stack.PopObject() ;
		}
	// スタックのトップオブジェクトを取得
	ECSObject * GetStackTop( void ) const
		{
			return	m_stack.m_varArray.GetLastAt( 0 ) ;
		}
	// スクリプトからファイルを開く
	virtual ESLFileObject * OpenFileOnScript
		( const wchar_t * pwszFileName,
			long int nOpenFlags = ESLFileObject::modeRead
									| ESLFileObject::shareRead ) ;

protected:
	// 実行イメージ上型フォーマット情報から静的なオブジェクトを生成
	ESLError CreateInstanceFromTypeInfo
				( ECSObject *& pObj, const ECSObject * pType ) ;

public:
	// 実行ステータスを取得
	ExecutionStatus GetStatus( void ) const
		{
			return	m_status ;
		}
	// 実行ステータスを設定
	virtual ExecutionStatus SetStatus( ExecutionStatus status ) ;
	// 返り値取得
	ECSObject * GetReturnValue( void ) const
		{
			return	m_pRetObj ;
		}
	// 実行環境取得
	ECSEnvironment * GetEnvironment( void ) const
		{
			if ( m_pcsxi != NULL )
			{
				return	m_pcsxi->m_pEnv ;
			}
			return	NULL ;
		}
	// 実行が一時停止されるまで待機する
	ESLError WaitForSuspended( DWORD dwTimeout ) ;
	// 実行コンテキストの処理権を取得する
	ESLError Lock( DWORD dwTimeout ) ;
	// 実行コンテキストの処理権を解放する
	ESLError Unlock( void ) ;
	// 実行中のコンテキストを一時停止させ処理権を取得する
	//	（別スレッドからの同期処理用）
	ESLError LockExecution( DWORD dwTimeout ) ;
	// LockExecution で取得した処理権を解放する
	ESLError UnlockExecution( void ) ;

public:
	// イベント待機フラグ
	enum	WaitFlag
	{
		wfAbortFlag		= 0x0001
	} ;
	// イベント待機
	virtual ESLError WaitUntilEvent
		( HANDLE hEvent, DWORD dwTimeout, DWORD dwFlags = wfAbortFlag ) ;
	// イベント待機を中止する
	virtual ESLError AbortWatingEvent( void ) ;
	// イベント待機の中止を解除する
	virtual ESLError ResetAbortWatingEvent( void ) ;

public:
	// 実行コンテキストを保存する
	virtual ESLError Save( ESLFileObject & file ) ;
	// 実行コンテキストを保存する（プロセッサコンテキスト）
	ESLError SaveProcessorContext( ESLFileObject & file ) ;
	// 自由領域を保存する
	ESLError SaveHeapMemory( ESLFileObject & file ) ;
	// 実行コンテキストを復元する
	virtual ESLError Load( ESLFileObject & file ) ;
	// 実行コンテキストを復元する（プロセッサコンテキスト）
	ESLError LoadProcessorContext( ESLFileObject & file ) ;
	// 実行コンテキストの復元処理を確定する（プロセッサコンテキスト）
	ESLError CommitLoadedProcessorContext( void ) ;
	// 自由領域を復元する
	ESLError LoadHeapMemory( ESLFileObject & file ) ;
public:
	// オブジェクトを保存する
	ESLError SaveObject( ESLFileObject & file, ECSObject * pObj ) ;
	// オブジェクトを復元する
	ESLError LoadObject( ESLFileObject & file, ECSObject *& pObj ) ;
protected:
	// 拡張コンテキストデータを保存する
	virtual ESLError SaveExtendedData( EMCFile & file ) ;
	// 拡張コンテキストデータを復元する
	virtual ESLError LoadExtendedData( EMCFile & file ) ;

public:
	// オブジェクトを生成する
	virtual ECSObject * CreateObject
		( CSVariableType csvtType,
			const wchar_t * pwszType, DWORD * pdwFuncAddr = NULL ) ;
	// 拡張型オブジェクトを生成する
	virtual ECSObject * CreateExtendedObject
		( CSVariableType csvtType,
			const wchar_t * pwszType, DWORD * pdwFuncAddr = NULL ) ;
	// クラス情報からオブジェクトを生成する
	virtual ECSObject * CreateClassObject( const ECSClassInfo & clsinf ) ;
	// 列挙型オブジェクトを生成する
	ECSObject * CreateEnumeratorObject( const ECSClassInfo & clsinf ) ;
	// ユーザー定義クラスのオブジェクトを生成する
	ECSStructureInterface *
		CreateUserClassObject( const ECSClassInfo & clsinf ) ;
	ESLError BuildNakedUserClassObject
			( ECSBufferStructure & obj, const ECSClassInfo & clsinf ) ;
	ESLError BuildUserClassObject
			( ECSStructure & obj, const ECSClassInfo & clsinf ) ;
	// Sakura2 共通オブジェクト生成インターフェース
	static ECSObject * Sakura2DefaultNewObject
			( ECSSakura2Processor::Context * context, int cls_id ) ;
	// 拡張ネイティブ関数を取得する
	virtual API_ECS_FUNC GetImportNativeFunction( const wchar_t * pwszFuncName ) ;
	// グローバル関数を呼び出す
	virtual ESLError CallGlobalFunction( const wchar_t * pwszFuncName ) ;
	// DLL 関数を検索する
	FARPROC FindPluginedFunction( const char * pszFuncName ) ;
	// Windows API を呼び出す
	ESLError CallFunctionWin32x86API
			( FARPROC pfnFunc, ECSObjArray<ECSObject> & lstArg ) ;
	// クラス情報取得
	ECSClassInfo * GetClassInfoAs( const wchar_t * pwszClassName ) const
		{
			if ( m_pcsxi != NULL )
			{
				return	m_pcsxi->GetClassInfoAs( pwszClassName ) ;
			}
			return	NULL ;
		}

public:
	// コンテキストをダンプする
	void DumpContext( EStreamBuffer & buf ) ;
	// 全ての変数にインデックスを振る
	void IndexAllMember( void ) ;

public:
	// スレッドリストに追加する
	void AddThreadList( ECSThread * pThread ) ;
	// スレッドリストから削除する
	void RemoveThreadList( ECSThread * pThread ) ;
	// 全てのサブスレッドを一時停止する
	void SuspendAllThread( void ) ;
	// 全てのサブスレッドの実行を再開する
	void ResumeAllThread( void ) ;

public:
	// 関数の引数の数をチェックする
	ESLError VerifyArgumentCount
		( ECSObjArray<ECSObject> & lstArg, int nArgMin, int nArgMax = 0 ) ;
	// 関数の引数を型を特定して取得する
	ESLError GetArgumentAsInt
		( int & nValue,
			ECSObjArray<ECSObject> & lstArg, int iArg, int nDefValue ) ;
	ESLError GetArgumentAsInt64
		( INT64 & nValue,
			ECSObjArray<ECSObject> & lstArg, int iArg, INT64 nDefValue ) ;
	ESLError GetArgumentAsReal
		( double & rValue,
			ECSObjArray<ECSObject> & lstArg, int iArg, double rDefValue ) ;
	ESLError GetArgumentAsStr
		( ECSWideString & wstrValue,
			ECSObjArray<ECSObject> & lstArg,
			int iArg, const wchar_t * pwszDefValue ) ;
	ECSObject * GetArgumentObjectAs
		( ECSObjArray<ECSObject> & lstArg,
				int iArg, const wchar_t * pwszTypeName ) ;
	// ユーザー定義構造体・クラスを生成する
	ECSStructure * CreateUserStructureObject( const wchar_t * pwszName ) ;
	ECSStructureInterface * CreateUserStructure( const wchar_t * pwszName ) ;

public:
	// リニアアドレスからオブジェクトを取得する
	inline ECSSakura2::Object * GetObjectFromLinearAddress
			( INT64 nAddress, int & iNakedOffset )
		{
			const int	nSel = (int) (nAddress >> 56) & 0xFF ;
			ECSSakura2::Object *	pObj ;
			iNakedOffset = (int) nAddress ;
			pObj = m_pcsxi->m_pAddressRootDirectory[nSel]->GetAt
								( (int) (nAddress >> 32) & 0x00FFFFFF ) ;
			return	pObj ;
		}

protected:
	// 命令実行関数ポインタ
	typedef ESLError (ECSContext::*PFUNC_EXECUTE_INSTRUCTION)( void ) ;
	static const PFUNC_EXECUTE_INSTRUCTION	m_pfnExecute[csicMax] ;

	// 実行関数
	ESLError ExecuteNew( void ) ;
	ESLError ExecuteFree( void ) ;
	ESLError ExecuteLoad( void ) ;
	ESLError ExecuteStore( void ) ;
	ESLError ExecuteEnter( void ) ;
	ESLError ExecuteLeave( void ) ;
	ESLError ExecuteJump( void ) ;
	ESLError ExecuteCJump( void ) ;
	ESLError ExecuteCall( void ) ;
	ESLError ExecuteReturn( void ) ;
	ESLError ExecuteElement( void ) ;
	ESLError ExecuteElementIndirect( void ) ;
	ESLError ExecuteOperate( void ) ;
	ESLError ExecuteUniOperate( void ) ;
	ESLError ExecuteCompare( void ) ;
	/* extended 2.0 */
	ESLError ExecuteExOperate( void ) ;
	ESLError ExecuteExUniOperate( void ) ;
	ESLError ExecuteExCall( void ) ;
	ESLError ExecuteExReturn( void ) ;
	ESLError ExecuteCallMember( void ) ;
	ESLError ExecuteCallNativeMember( void ) ;
	ESLError ExecuteSwap( void ) ;
	/* extended 2.3 */
	ESLError ExecuteCreateBuffer( void ) ;
	ESLError ExecuteCreateBufferVSize( void ) ;
	ESLError ExecutePointerToObject( void ) ;
	ESLError ExecutePointerToAddress( void ) ;
	ESLError ExecuteReferenceForPointer( void ) ;
	ESLError ExecuteReferenceForObjPointer( void ) ;
	ESLError ExecuteCallFunctionPointer( void ) ;
	ESLError ExecuteCallNativeFunction( void ) ;

public:
	// オブジェクト代入操作
	ESLError MoveObject( ECSObject * pDstObj, ECSObject * pSrcObj )
		{
			ECSObject *	pSrcEntity = ECSObject::GetEntity( pSrcObj ) ;
			if ( pSrcEntity == NULL )
			{
				if ( pDstObj->m_vtType == csvtReference )
				{
					((ECSReference*)pDstObj)->SetReference( NULL, this ) ;
					delete_CSObject( pSrcObj ) ;
					return	eslErrSuccess ;
				}
			}
			else if ( pDstObj->m_vtType == pSrcEntity->m_vtType )
			{
				switch ( pDstObj->m_vtType )
				{
				case	csvtInteger:
					((ECSInteger*)pDstObj)->SetValue
						( ((ECSInteger*)pSrcEntity)->GetValue() ) ;
					delete_CSObject( pSrcObj ) ;
					return	eslErrSuccess ;
				case	csvtReal:
					((ECSReal*)pDstObj)->m_varReal =
						((ECSReal*)pSrcEntity)->m_varReal ;
					delete_CSObject( pSrcObj ) ;
					return	eslErrSuccess ;
				case	csvtString:
					((ECSString*)pDstObj)->m_varStr =
						((ECSString*)pSrcEntity)->m_varStr ;
					delete_CSObject( pSrcObj ) ;
					return	eslErrSuccess ;
				case	csvtObject:
				case	csvtReference:
				case	csvtArray:
				case	csvtHash:
				default:
					break ;
				}
			}
			return	pDstObj->Move( *this, pSrcObj ) ;
		}
	// 静的文字列バッファポインタを取得する
	ECSWideString * GetConstantString( const wchar_t * pwszString ) ;

protected:
	// 現在の命令ポインタから文字列リテラルを取得してポインタを進める
	// （解放する必要のない静的なバッファを返す）
	ECSString * GetStringLiteralObject( void ) ;
	ECSWideString * GetStringLiteral( void ) ;
	// 現在の命令ポインタから文字列リテラルを取得してポインタを進める
	void LoadStringLiteral( ECSSourceStream & wstrBuf ) ;
public:
	// 関数の引数を object スタックから引数配列へセット
	ESLError ExecuteExCallArguments( int nArgCount ) ;
	// this オブジェクトを取得
	ECSObject * GetThisObject( void ) ;
	// 関数呼び出し名前空間をマークする
	void MarkCallStackFlag( void ) ;
	// 現在のスタック上のローカルな名前空間を削除
	void ReleaseLocalStackBlock( void ) ;
	// 指定オブジェクトが関数のローカルな変数か判定
	bool IsLocalObject( ECSObject * pObj ) ;
	// 例外を発行する
	ESLError ThrowExpression( void ) ;

public:
	// システム関数プロトタイプ
	typedef	ESLError (ECSContext::*PFUNC_CALL)( ECSObjArray<ECSObject> & lstArg ) ;
	// メンバ関数ポインタ
	static ECSStrTagArray *	m_staFuncName ;
	static const wchar_t *	m_pwszFuncName[14] ;
	static const PFUNC_CALL	m_pfnCallFunc[13] ;
	// メンバ関数
	ESLError Call_GetSystemPerformance( ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_SetSystemPerformance( ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetCurrentTime( ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetLocalTime( ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_GetMemoryStatus( ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_AddModule( ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_OpenToAddArchiveFile( ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_EnableArchiveFilePath( ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Sleep( ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Suspend( ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Exit( ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_Trace( ECSObjArray<ECSObject> & lstArg ) ;
	ESLError Call_memmove( ECSObjArray<ECSObject> & lstArg ) ;

public:
	// native 関数ゲート
	class	NativeCallGate
	{
	public:
		virtual ESLError Call
			( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) = 0 ;
	} ;
	//
	class	NativeCotophaCallGate	: public NativeCallGate
	{
		PFUNC_CALL	m_pfnCall ;
	public:
		NativeCotophaCallGate( PFUNC_CALL pfnCall )
			: m_pfnCall( pfnCall ) { }
		virtual ESLError Call
			( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	} ;
	class	NativePluginCallGate	: public NativeCallGate
	{
		API_ECS_FUNC	m_pfnCall ;
	public:
		NativePluginCallGate( API_ECS_FUNC pfnCall )
			: m_pfnCall( pfnCall ) { }
		virtual ESLError Call
			( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	} ;
	class	NativeStdcallCallGate	: public NativeCallGate
	{
		FARPROC	m_pfnCall ;
	public:
		NativeStdcallCallGate( FARPROC pfnCall )
			: m_pfnCall( pfnCall ) { }
		virtual ESLError Call
			( ECSContext & context, ECSObjArray<ECSObject> & lstArg ) ;
	} ;
	EObjArray<NativeCallGate>	m_lstNativeCallGate ;

public:
	// オブジェクトを生成する
	ECSReference * new_CSReference( ECSObject * pRef = NULL )
		{
			ECSReference *	pObj = m_tsbufReference.Create() ;
			if ( pRef != NULL )
			{
				pObj->SetReference( pRef, this ) ;
			}
			return	pObj ;
		}
	ECSArray * new_CSArray( void )
		{
			return	new ECSArray ;
		}
	ECSHash * new_CSHash( void )
		{
			return	new ECSHash ;
		}
	ECSInteger * new_CSInteger( INT64 nInitVal = 0, INT64 nMask = -1 )
		{
			ECSInteger *	pObj = m_tsbufInteger.Create() ;
			pObj->m_varInt = nInitVal ;
			pObj->m_varMask = nMask ;
			return	pObj ;
		}
	ECSReal * new_CSReal( double rInitVal = 0 )
		{
			ECSReal *	pObj = m_tsbufReal.Create() ;
			pObj->m_varReal = rInitVal ;
			return	pObj ;
		}
	ECSString * new_CSString( void )
		{
			return	m_tsbufString.Create() ;
		}
	ECSString * new_CSString( const wchar_t * pwszInit )
		{
			ECSString *	pStr = m_tsbufString.Create() ;
			pStr->m_varStr = pwszInit ;
			return	pStr ;
		}
	ECSPointer * new_CSPointer( ECSPointer & ptr )
		{
			ECSPointer *	pObj = m_tsbufPointer.Create() ;
			pObj->CopyPointerFrom( this, ptr ) ;
			return	pObj ;
		}
	ECSPointer * new_CSPointer( ECSObject * pRef = NULL, int iOffset = 0 )
		{
			ECSPointer *	pObj = m_tsbufPointer.Create() ;
			if ( pRef != NULL )
			{
				pObj->SetReference( pRef, this ) ;
			}
			pObj->SetOffset( iOffset ) ;
			return	pObj ;
		}
	ECSPointerReference * new_CSPointerReference
			( ECSObject * pRef = NULL,
				int iOffset = 0, CSVariableType csvtType = csvtObject )
		{
			ECSPointerReference *	pObj = m_tsbufPointerRef.Create() ;
			if ( pRef != NULL )
			{
				pObj->SetReference( pRef, this ) ;
			}
			pObj->SetOffset( iOffset ) ;
			pObj->SetReferenceType( csvtType ) ;
			return	pObj ;
		}
	ECSPointerReference * new_CSPointerReference( ECSPointerReference & ptr )
		{
			ECSPointerReference *	pObj = m_tsbufPointerRef.Create() ;
			pObj->CopyPointerFrom( this, ptr ) ;
			return	pObj ;
		}
	// オブジェクトを破棄する
	void delete_CSObject( ECSObject * pObj ) ;

public:
	//////////////////////////////////////////////////////////////////////////
	// Sakura2 実行コンテキスト
	//////////////////////////////////////////////////////////////////////////

	// 128 bit アライメント new
	void * operator new ( size_t nBytes ) ;
	void operator delete ( void * pObj ) ;

	// 定数
	enum
	{
		Sakura2StackLimit	= 0x1000000,		// 16MB
	} ;

	// Sakura2 processor 初期化
	void InitializeSakuraProcessor( void ) ;
	// スタック拡張
	virtual DWORD HandleExceptionExtendStack( DWORD maskException ) ;
	// 例外エラーメッセージを取得
	virtual const wchar_t * GetExceptionErrorMessage( DWORD maskException ) ;


	// naked native 関数用メモリアクセス・ユーティリティ関数
	//////////////////////////////////////////////////////////////////////////

	// メモリロード
	inline INT64 AtomicLoadMemory
		( ECSSakura2Processor::DataType type, INT64 nAddress )
	{
		INT64	nValue ;
		ECotophaScript::Lock() ;
		nValue = (ECSSakura2Processor::pfnSakuraLoad[type])( this, nAddress ) ;
		ECotophaScript::Unlock() ;
		return	nValue ;
	}
	const wchar_t * AtomicLoadString
			( EWideString & wstrBuf, INT64 nAddress ) ;
	const wchar_t * AtomicLoadString
			( EWideString & wstrBuf, INT64 nAddress, int nLength ) ;
	const wchar_t * AtomicLoadString
			( EWideString & wstrBuf,
				ECSSakura2::Object * pObj, int iOffset ) ;
	const wchar_t * AtomicLoadString
			( EWideString & wstrBuf,
				ECSSakura2::Object * pObj, int iOffset, int nLength ) ;
	inline INT64 AtomicLoadInt64( INT64 nAddress )
	{
		return	AtomicLoadMemory( ECSSakura2Processor::dataInt64, nAddress ) ;
	}
	inline INT64 AtomicLoadInt32( INT64 nAddress )
	{
		return	AtomicLoadMemory( ECSSakura2Processor::dataInt32, nAddress ) ;
	}
	inline INT64 AtomicLoadInt16( INT64 nAddress )
	{
		return	AtomicLoadMemory( ECSSakura2Processor::dataInt16, nAddress ) ;
	}
	inline INT64 AtomicLoadInt8( INT64 nAddress )
	{
		return	AtomicLoadMemory( ECSSakura2Processor::dataInt8, nAddress ) ;
	}
	inline INT64 AtomicLoadUInt32( INT64 nAddress )
	{
		return	AtomicLoadMemory( ECSSakura2Processor::dataUint32, nAddress ) ;
	}
	inline INT64 AtomicLoadUInt16( INT64 nAddress )
	{
		return	AtomicLoadMemory( ECSSakura2Processor::dataUint16, nAddress ) ;
	}
	inline INT64 AtomicLoadUInt8( INT64 nAddress )
	{
		return	AtomicLoadMemory( ECSSakura2Processor::dataUint8, nAddress ) ;
	}
	inline double AtomicLoadReal32( INT64 nAddress )
	{
		INT64	nValue ;
		ECotophaScript::Lock() ;
		nValue = (ECSSakura2Processor::pfnSakuraLoad[ECSSakura2Processor::dataInt64])( this, nAddress ) ;
		ECotophaScript::Unlock() ;
		return	*((REAL64*)&nValue) ;
	}
	inline double AtomicLoadReal64( INT64 nAddress )
	{
		INT64	nValue ;
		ECotophaScript::Lock() ;
		nValue = (ECSSakura2Processor::pfnSakuraLoad[ECSSakura2Processor::dataFloat])( this, nAddress ) ;
		ECotophaScript::Unlock() ;
		return	*((REAL64*)&nValue) ;
	}

	// メモリストア
	inline void AtomicStoreMemory
		( ECSSakura2Processor::DataType type,
						INT64 nAddress, INT64 nData )
	{
		ECotophaScript::Lock() ;
		(ECSSakura2Processor::pfnSakuraStore[type])( this, nAddress, nData ) ;
		ECotophaScript::Unlock() ;
	}
	inline void AtomicStoreInt64( INT64 nAddress, INT64 nData )
	{
		AtomicStoreMemory
			( ECSSakura2Processor::dataInt64, nAddress, nData ) ;
	}
	inline void AtomicStoreInt32( INT64 nAddress, int nData )
	{
		AtomicStoreMemory
			( ECSSakura2Processor::dataInt32, nAddress, nData ) ;
	}
	inline void AtomicStoreInt16( INT64 nAddress, int nData )
	{
		AtomicStoreMemory
			( ECSSakura2Processor::dataInt16, nAddress, nData ) ;
	}
	inline void AtomicStoreInt8( INT64 nAddress, int nData )
	{
		AtomicStoreMemory
			( ECSSakura2Processor::dataInt8, nAddress, nData ) ;
	}
	inline void AtomicStoreUInt32( INT64 nAddress, unsigned int nData )
	{
		AtomicStoreMemory
			( ECSSakura2Processor::dataUint32, nAddress, nData ) ;
	}
	inline void AtomicStoreUInt16( INT64 nAddress, unsigned int nData )
	{
		AtomicStoreMemory
			( ECSSakura2Processor::dataUint16, nAddress, nData ) ;
	}
	inline void AtomicStoreUInt8( INT64 nAddress, unsigned int nData )
	{
		AtomicStoreMemory
			( ECSSakura2Processor::dataUint8, nAddress, nData ) ;
	}


public:
	//////////////////////////////////////////////////////////////////////////
	// プラグインインターフェース
	//////////////////////////////////////////////////////////////////////////

	// プラグイン用インターフェースを取得する
	PLUGIN_CONTEXT * GetContextInterface( void ) ;

protected:
	static ECS_CONTEXT::ExecutionStatus
		__stdcall PIC_GetStatus( ECS_CONTEXT * context ) ;
	static ECS_CONTEXT::ExecutionStatus __stdcall PIC_SetStatus
		( ECS_CONTEXT * context, ECS_CONTEXT::ExecutionStatus status ) ;
	static unsigned long int
		__stdcall PIC_GetInstructionPointer( ECS_CONTEXT * context ) ;
	static void __stdcall PIC_SetInstructionPointer
		( ECS_CONTEXT * context, unsigned long int ip ) ;
	static ESLError __stdcall PIC_Lock
		( ECS_CONTEXT * context, DWORD dwTimeout ) ;
	static ESLError __stdcall PIC_Unlock( ECS_CONTEXT * context ) ;
	static ESLError __stdcall PIC_LockExecution
		( ECS_CONTEXT * context, DWORD dwTimeout ) ;
	static ESLError __stdcall PIC_UnlockExecution( ECS_CONTEXT * context ) ;
	static ESLError __stdcall PIC_CallFunction
		( ECS_CONTEXT * context, DWORD dwFuncAddr,
			ECS_OBJECT * const* pArg, int nArgCount ) ;
	static ESLError __stdcall PIC_PushObject
		( ECS_CONTEXT * context, ECS_OBJECT * pObj ) ;
	static ECS_OBJECT * __stdcall PIC_PopObject( ECS_CONTEXT * context ) ;
	static ECS_FILE * __stdcall PIC_OpenFile
		( ECS_CONTEXT * context,
			const wchar_t * pwszFileName, long int nOpenFlags ) ;
	static ESLError __stdcall PIC_Save
		( ECS_CONTEXT * context, ECS_FILE * pfile ) ;
	static ESLError __stdcall PIC_Load
		( ECS_CONTEXT * context, ECS_FILE * pfile ) ;
	static ESLError __stdcall PIC_SaveObject
		( ECS_CONTEXT * context, ECS_FILE * pfile, ECS_OBJECT * pObj ) ;
	static ESLError __stdcall PIC_LoadObject
		( ECS_CONTEXT * context, ECS_FILE * pfile, ECS_OBJECT ** pObj ) ;
	static ECS_OBJECT * __stdcall PIC_CreateObject
		( ECS_CONTEXT * context, CSVariableType csvtType,
			const wchar_t * pwszType, DWORD * pdwFuncAddr ) ;
	static ECS_OBJECT * __stdcall PIC_GetStack( ECS_CONTEXT * context ) ;
	static ECS_OBJECT * __stdcall PIC_GetGlobal( ECS_CONTEXT * context ) ;
	static ECS_OBJECT * __stdcall PIC_GetStatic( ECS_CONTEXT * context ) ;
	static ECS_OBJECT * __stdcall PIC_CreateReference
		( ECS_CONTEXT * context, ECS_OBJECT * pRef ) ;
	static ECS_OBJECT * __stdcall PIC_CreateInteger
		( ECS_CONTEXT * context, long int nInitVal ) ;
	static ECS_OBJECT * __stdcall PIC_CreateReal
		( ECS_CONTEXT * context, double rInitVal ) ;
	static ECS_OBJECT * __stdcall PIC_CreateString
		( ECS_CONTEXT * context, const wchar_t * pwszInitVal ) ;
	static ECS_OBJECT *
		__stdcall PIC_CreateAbstractObject( ECS_CONTEXT * context ) ;
	static ESLObject * __stdcall PIC_GetWaveOutputDevice( ECS_CONTEXT * context ) ;
	static ESLObject * __stdcall PIC_GetDrawImageObject( ECS_CONTEXT * context ) ;
	static HESLHEAP __stdcall PIC_GetHeapHandle( ECS_CONTEXT * context ) ;

	friend	ECSThread ;
	friend	ECSExecutionImage ;

} ;


// naked native 関数
//////////////////////////////////////////////////////////////////////////////

/*
// void * malloc( int bytes )
ECS_EXPORT const wchar_t * ecs_nakedcall_malloc
	( ECSSakura2Processor::Context * context,
			const ECSSakura2Processor::Register * pArg ) ;
// void * realloc( void * memblock, int bytes )
ECS_EXPORT const wchar_t * ecs_nakedcall_realloc
	( ECSSakura2Processor::Context * context,
			const ECSSakura2Processor::Register * pArg ) ;
// void free( void * memblock )
ECS_EXPORT const wchar_t * ecs_nakedcall_free
	( ECSSakura2Processor::Context * context,
			const ECSSakura2Processor::Register * pArg ) ;

// void * object_new( int cls_id )
ECS_EXPORT const wchar_t * ecs_nakedcall_object_new
	( ECSSakura2Processor::Context * context,
			const ECSSakura2Processor::Register * pArg ) ;
// void object_delete( void * obj )
ECS_EXPORT const wchar_t * ecs_nakedcall_object_delete
	( ECSSakura2Processor::Context * context,
			const ECSSakura2Processor::Register * pArg ) ;
*/
//
// void object_push_int( int value )
ECS_EXPORT const wchar_t * ecs_nakedcall_object_push_int
	( ECSSakura2Processor::Context * context,
			const ECSSakura2Processor::Register * pArg ) ;
// void object_push_double( double value )
ECS_EXPORT const wchar_t * ecs_nakedcall_object_push_double
	( ECSSakura2Processor::Context * context,
			const ECSSakura2Processor::Register * pArg ) ;
// void object_push_string( const uint16 * str )
ECS_EXPORT const wchar_t * ecs_nakedcall_object_push_string
	( ECSSakura2Processor::Context * context,
			const ECSSakura2Processor::Register * pArg ) ;
// void object_push_reference( void * obj )
ECS_EXPORT const wchar_t * ecs_nakedcall_object_push_reference
	( ECSSakura2Processor::Context * context,
			const ECSSakura2Processor::Register * pArg ) ;
// void object_push_pointer( void * obj )
ECS_EXPORT const wchar_t * ecs_nakedcall_object_push_pointer
	( ECSSakura2Processor::Context * context,
			const ECSSakura2Processor::Register * pArg ) ;
// void object_push_type( int type_id )
ECS_EXPORT const wchar_t * ecs_nakedcall_object_push_type
	( ECSSakura2Processor::Context * context,
			const ECSSakura2Processor::Register * pArg ) ;
// void object_push_new( int cls_id )
ECS_EXPORT const wchar_t * ecs_nakedcall_object_push_new
	( ECSSakura2Processor::Context * context,
			const ECSSakura2Processor::Register * pArg ) ;
// int object_pop_int( void )
ECS_EXPORT const wchar_t * ecs_nakedcall_object_pop_int
	( ECSSakura2Processor::Context * context,
			const ECSSakura2Processor::Register * pArg ) ;
// double object_pop_double( void )
ECS_EXPORT const wchar_t * ecs_nakedcall_object_pop_double
	( ECSSakura2Processor::Context * context,
			const ECSSakura2Processor::Register * pArg ) ;
// void * object_pop_new( void )
ECS_EXPORT const wchar_t * ecs_nakedcall_object_pop_new
	( ECSSakura2Processor::Context * context,
			const ECSSakura2Processor::Register * pArg ) ;
// void object_stack_free( int count )
ECS_EXPORT const wchar_t * ecs_nakedcall_object_stack_free
	( ECSSakura2Processor::Context * context,
			const ECSSakura2Processor::Register * pArg ) ;
// void * object_get_stack_at( int index )
ECS_EXPORT const wchar_t * ecs_nakedcall_object_get_stack_at
	( ECSSakura2Processor::Context * context,
			const ECSSakura2Processor::Register * pArg ) ;
// void * object_get_stack_last( int index )
ECS_EXPORT const wchar_t * ecs_nakedcall_object_get_stack_last
	( ECSSakura2Processor::Context * context,
			const ECSSakura2Processor::Register * pArg ) ;
//
// bool object_get_boolean( void * obj )
ECS_EXPORT const wchar_t * ecs_nakedcall_object_get_boolean
	( ECSSakura2Processor::Context * context,
			const ECSSakura2Processor::Register * pArg ) ;
// int object_get_int( void * obj )
ECS_EXPORT const wchar_t * ecs_nakedcall_object_get_int
	( ECSSakura2Processor::Context * context,
			const ECSSakura2Processor::Register * pArg ) ;
// double object_get_double( void * obj )
ECS_EXPORT const wchar_t * ecs_nakedcall_object_get_double
	( ECSSakura2Processor::Context * context,
			const ECSSakura2Processor::Register * pArg ) ;
// int object_size_of( void * obj )
ECS_EXPORT const wchar_t * ecs_nakedcall_object_size_of
	( ECSSakura2Processor::Context * context,
			const ECSSakura2Processor::Register * pArg ) ;
//
// void object_move_pop( int optype )
ECS_EXPORT const wchar_t * ecs_nakedcall_object_move_pop
	( ECSSakura2Processor::Context * context,
			const ECSSakura2Processor::Register * pArg ) ;
// void object_operate_pop( int optype )
ECS_EXPORT const wchar_t * ecs_nakedcall_object_operate_pop
	( ECSSakura2Processor::Context * context,
			const ECSSakura2Processor::Register * pArg ) ;
// void object_uni_operate( int optype )
ECS_EXPORT const wchar_t * ecs_nakedcall_object_uni_operate_pop
	( ECSSakura2Processor::Context * context,
			const ECSSakura2Processor::Register * pArg ) ;
// bool object_compare_pop( int cmptype )
ECS_EXPORT const wchar_t * ecs_nakedcall_object_compare_pop
	( ECSSakura2Processor::Context * context,
			const ECSSakura2Processor::Register * pArg ) ;
// bool object_push_element( void * obj, int index )
ECS_EXPORT const wchar_t * ecs_nakedcall_object_push_element
	( ECSSakura2Processor::Context * context,
			const ECSSakura2Processor::Register * pArg ) ;
// bool object_push_element_pop( void * obj )
ECS_EXPORT const wchar_t * ecs_nakedcall_object_push_element_pop
	( ECSSakura2Processor::Context * context,
			const ECSSakura2Processor::Register * pArg ) ;
// bool object_set_element_int_pop( void * obj, int index )
ECS_EXPORT const wchar_t * ecs_nakedcall_object_set_element_int_pop
	( ECSSakura2Processor::Context * context,
			const ECSSakura2Processor::Register * pArg ) ;
// bool object_set_element_pop_pop( void * obj )
ECS_EXPORT const wchar_t * ecs_nakedcall_object_set_element_pop_pop
	( ECSSakura2Processor::Context * context,
			const ECSSakura2Processor::Register * pArg ) ;
//
// void object_static_cast
//		( int var_offset, int var_bounds, int func_offset )
ECS_EXPORT const wchar_t * ecs_nakedcall_object_static_cast
	( ECSSakura2Processor::Context * context,
			const ECSSakura2Processor::Register * pArg ) ;
// void object_dynamic_cast( int cstr_id )
ECS_EXPORT const wchar_t * ecs_nakedcall_object_dynamic_cast
	( ECSSakura2Processor::Context * context,
			const ECSSakura2Processor::Register * pArg ) ;
