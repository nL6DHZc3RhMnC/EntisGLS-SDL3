
//////////////////////////////////////////////////////////////////////////////
// Entis Cotopha Script 外部モジュールインターフェース
//////////////////////////////////////////////////////////////////////////////

#if	!defined(__CTSCRIPT_PLUGIN_H__)
#define	__CTSCRIPT_PLUGIN_H__	1

#if	!defined(__EGL2D_H__)
#include <egl.h>
#endif

enum	CSOperatorType
{
	csotNop	= -1,
	csotAdd,	csotSub,	csotMul,	csotDiv,	csotMod,
	csotAnd,	csotOr,		csotXor,
	csotLogicalAnd,			csoutLogicalOr,
	/* extended 2.0 */
	csotShiftRight,			csotShiftLeft,
	csotMax
} ;

enum	CSUnaryOperatorType
{
	csuotPlus,	csuotNegate,	csuotBitNot,	csuotLogicalNot,
	/* extended 2.3 */
	/* 命令コードとしては csuotIncrement, csuotDecrement のみ使用 */
	csuotIncrement, csuotDecrement,
	csuotIncrementAfter, csuotDecrementAfter,
	csuotMax,
} ;

enum	CSExtraUniOperatorType	/* extended 2.0 */
{
	csxuotDeselect,	csxuotBoolean,
	csxuotSizeOf,	csxuotTypeOf,
	csxuotStaticCast,	csxuotDynamicCast,
	csxuotDuplicate,
	/* extended 2.3 */
	/* 命令コードとしては csxuotDelete のみ使用 */
	/* それ以外はコンパイル時に使用 */
	csxuotDelete,		csxuotDeleteArray,
	csxuotLoadAddress,	csxuotRefAddress,
	csxuotMax,
} ;

enum	CSExtraOperatorType	/* extended 2.0 */
{
	csxotArrayDim,	csxotHashContainer,
	csxotMoveReference,
	csxotMax,
} ;

enum	CSCompareType
{
	csctNotEqual,			csctEqual,
	csctLessThan,			csctLessEqual,
	csctGreaterThan,		csctGreaterEqual,
	/* extended 2.0 */
	csctNotEqualPointer,	csctEqualPointer,
	csctMax,
	csctPointerComparatorFirst	= csctNotEqualPointer,
} ;

enum	CSVariableType
{
	csvtObject,					// システム定義オブジェクト
	csvtReference,				// 参照型
	csvtArray,					// 配列型
	csvtHash,					// 参照配列型
	csvtInteger,				// 整数型
	csvtReal,					// 実数型
	csvtString,					// 文字列型
	csvtMax,
	/* extended 2.0 */
	csvtInteger64 = csvtMax,	// 整数（64ビット即値用）
	csvtPointer,				// ポインター型
	csvtClassObject,			// ユーザー定義クラスオブジェクト生成用
	csvtBoolean,				// Integer 派生型インスタンス生成用
	csvtInt8,
	csvtUint8,
	csvtInt16,
	csvtUint16,
	csvtInt32,
	csvtUint32,
	csvtArrayDimension,			// 多次元配列型保存用
								//（クラス情報内の静的な型情報）
								// ※動的な多次元配列生成には
								// 　CSExtraOperatorType の
								// 　csxotArrayDim 演算子を使用
	csvtHashContainer,			// ハッシュコンテナ型保存用
								//（クラス情報内の静的な型情報）
								// ※動的なハッシュコンテナの生成には
								// 　CSExtraOperatorType の
								// csxotHashContainer 演算子を使用
	/* extended 2.3 */
	csvtReal32,					// Pointer からロードする時のための識別子
	csvtReal64,
	csvtPointerReference,		// メモリポインタ参照型
	csvtExTypeMax,
	csvtBuffer = csvtExTypeMax,	// バッファ
	csvtFunction,				// 関数ポインタインスタンス生成用（未使用）
	csvtValidMax,
	csvtInvalid = -1
} ;


//////////////////////////////////////////////////////////////////////////////
// プラグインエントリポイント
//////////////////////////////////////////////////////////////////////////////

#if	!defined(ECS_EXPORT)
#define	ECS_EXPORT	extern "C" __declspec( dllexport )
#endif

struct	ECS_PLUGIN_ENTRY_TABLE
{
	DWORD	dwVersion ;
	void (__stdcall *pfnStartup)( struct ECS_CONTEXT * context ) ;
	void (__stdcall *pfnShutdown)( struct ECS_CONTEXT * context ) ;
	struct ECS_OBJECT * (__stdcall *pfnCreateObject)
		( struct ECS_CONTEXT * context, const wchar_t * pwszType ) ;
} ;

typedef	ECS_PLUGIN_ENTRY_TABLE * (*PECS_PLUGIN_ENTRYPOINT)( void ) ;


//////////////////////////////////////////////////////////////////////////////
//	ファイルインターフェース
//////////////////////////////////////////////////////////////////////////////

struct	ECS_FILE
{
	enum	OpenFlag
	{
		modeCreateFlag	= 0x0001 ,
		modeCreate		= 0x0005 ,
		modeRead		= 0x0002 ,
		modeWrite		= 0x0004 ,
		modeReadWrite	= 0x0006 ,
		shareRead		= 0x0010 ,
		shareWrite		= 0x0020
	} ;
	enum	SeekOrigin
	{
		FromBegin	= FILE_BEGIN,
		FromCurrent	= FILE_CURRENT,
		FromEnd		= FILE_END
	} ;
	void (__stdcall *pfnRelease)( ECS_FILE * pfile ) ;
	unsigned long int (__stdcall *pfnRead)
		( ECS_FILE * pfile, void * ptrBuffer, unsigned long int nBytes ) ;
	unsigned long int (__stdcall *pfnWrite)
		( ECS_FILE * pfile, const void * ptrBuffer, unsigned long int nBytes ) ;
	unsigned long int (__stdcall *pfnGetLength)( ECS_FILE * pfile ) ;
	unsigned long int (__stdcall *pfnSeek)
		( ECS_FILE * pfile, long int nOffsetPos, SeekOrigin fSeekFrom ) ;
	unsigned long int (__stdcall *pfnGetPosition)( ECS_FILE * pfile ) ;
	ESLError (__stdcall *pfnSetEndOfFile)( ECS_FILE * pfile ) ;
	const char * (__stdcall *pfnGetFilePath)( ECS_FILE * pfile ) ;

	void Release( void )
		{	pfnRelease( this ) ;	}
	unsigned long int Read( void * ptrBuffer, unsigned long int nBytes )
		{	return	pfnRead( this, ptrBuffer, nBytes ) ;	}
	unsigned long int Write( const void * ptrBuffer, unsigned long int nBytes )
		{	return	pfnWrite( this, ptrBuffer, nBytes ) ;	}
	unsigned long int GetLength( void )
		{	return	pfnGetLength( this ) ;	}
	unsigned long int Seek( long int nOffsetPos, SeekOrigin fSeekFrom )
		{	return	pfnSeek( this, nOffsetPos, fSeekFrom ) ;	}
	unsigned long int GetPosition( void )
		{	return	pfnGetPosition( this ) ;	}
	ESLError SetEndOfFile( void )
		{	return	pfnSetEndOfFile( this ) ;	}
	const char * GetFilePath( void )
		{	return	pfnGetFilePath( this ) ;	}
} ;

class	ECSPIFileInterface	: public	ESLFileObject
{
public:
	ECS_FILE *	m_pfile ;

public:
	// 構築関数
	ECSPIFileInterface
		( ECS_FILE * pfile = NULL,
			int nAttrFlags = modeReadWrite )
		: m_pfile(pfile) { SetAttribute( nAttrFlags ) ; }
	// ファイルオブジェクトを複製する
	virtual ESLFileObject * Duplicate( void ) const
		{
			return	new ECSPIFileInterface( m_pfile ) ;
		}
	// ファイルから読み込む
	virtual unsigned long int Read
			( void * ptrBuffer, unsigned long int nBytes )
		{
			ESLAssert( m_pfile != NULL ) ;
			return	m_pfile->Read( ptrBuffer, nBytes ) ;
		}
	// ファイルへ書き出す
	virtual unsigned long int Write
			( const void * ptrBuffer, unsigned long int nBytes )
		{
			ESLAssert( m_pfile != NULL ) ;
			return	m_pfile->Write( ptrBuffer, nBytes ) ;
		}
	// ファイルの長さを取得
	virtual unsigned long int GetLength( void ) const
		{
			ESLAssert( m_pfile != NULL ) ;
			return	m_pfile->GetLength( ) ;
		}
	// ファイルポインタを移動
	virtual unsigned long int Seek
			( long int nOffsetPos, SeekOrigin fSeekFrom )
		{
			ESLAssert( m_pfile != NULL ) ;
			return	m_pfile->Seek
				( nOffsetPos, (ECS_FILE::SeekOrigin) fSeekFrom ) ;
		}
	// ファイルポインタを取得
	virtual unsigned long int GetPosition( void ) const
		{
			ESLAssert( m_pfile != NULL ) ;
			return	m_pfile->GetPosition( ) ;
		}
	// ファイルの終端を現在の位置に設定する
	virtual ESLError SetEndOfFile( void )
		{
			ESLAssert( m_pfile != NULL ) ;
			return	m_pfile->SetEndOfFile( ) ;
		}

} ;


//////////////////////////////////////////////////////////////////////////////
//	実行コンテキスト抽象インターフェース
//////////////////////////////////////////////////////////////////////////////

struct	ECS_OBJECT ;
struct	ECS_CONTEXT
{
	enum	ExecutionStatus
	{
		xsHalt,				// 実行完了／未実行
		xsExecution,		// 実行中
		xsSuspend,			// 実行一時停止
		xsInterrupt,		// 割り込み
		xsMask	= 0xFFFFFFFF
	} ;

	ExecutionStatus (__stdcall *pfnGetStatus)( ECS_CONTEXT * context ) ;
	ExecutionStatus (__stdcall *pfnSetStatus)
		( ECS_CONTEXT * context, ExecutionStatus status ) ;
	unsigned long int
		(__stdcall *pfnGetInstructionPointer)( ECS_CONTEXT * context ) ;
	void (__stdcall *pfnSetInstructionPointer)
		( ECS_CONTEXT * context, unsigned long int ip ) ;
	ESLError (__stdcall *pfnLock)
		( ECS_CONTEXT * context, DWORD dwTimeout ) ;
	ESLError (__stdcall *pfnUnlock)( ECS_CONTEXT * context ) ;
	ESLError (__stdcall *pfnLockExecution)
		( ECS_CONTEXT * context, DWORD dwTimeout ) ;
	ESLError (__stdcall *pfnUnlockExecution)( ECS_CONTEXT * context ) ;
	ESLError (__stdcall *pfnCallFunction)
		( ECS_CONTEXT * context, DWORD dwFuncAddr,
			ECS_OBJECT * const* pArg, int nArgCount ) ;
	ESLError (__stdcall *pfnPushObject)
		( ECS_CONTEXT * context, ECS_OBJECT * pObj ) ;
	ECS_OBJECT * (__stdcall *pfnPopObject)( ECS_CONTEXT * context ) ;
	ECS_FILE * (__stdcall *pfnOpenFile)
		( ECS_CONTEXT * context,
			const wchar_t * pwszFileName, long int nOpenFlags ) ;
	ESLError (__stdcall *pfnSave)
		( ECS_CONTEXT * context, ECS_FILE * pfile ) ;
	ESLError (__stdcall *pfnLoad)
		( ECS_CONTEXT * context, ECS_FILE * pfile ) ;
	ESLError (__stdcall *pfnSaveObject)
		( ECS_CONTEXT * context, ECS_FILE * pfile, ECS_OBJECT * pObj ) ;
	ESLError (__stdcall *pfnLoadObject)
		( ECS_CONTEXT * context, ECS_FILE * pfile, ECS_OBJECT ** pObj ) ;
	ECS_OBJECT * (__stdcall *pfnCreateObject)
		( ECS_CONTEXT * context, CSVariableType csvtType,
			const wchar_t * pwszType, DWORD * pdwFuncAddr ) ;
	ECS_OBJECT * (__stdcall *pfnGetStack)( ECS_CONTEXT * context ) ;
	ECS_OBJECT * (__stdcall *pfnGetGlobal)( ECS_CONTEXT * context ) ;
	ECS_OBJECT * (__stdcall *pfnGetStatic)( ECS_CONTEXT * context ) ;
	ECS_OBJECT * (__stdcall *pfnCreateReference)
		( ECS_CONTEXT * context, ECS_OBJECT * pRef ) ;
	ECS_OBJECT * (__stdcall *pfnCreateInteger)
		( ECS_CONTEXT * context, long int nInitVal ) ;
	ECS_OBJECT * (__stdcall *pfnCreateReal)
		( ECS_CONTEXT * context, double rInitVal ) ;
	ECS_OBJECT * (__stdcall *pfnCreateString)
		( ECS_CONTEXT * context, const wchar_t * pwszInitVal ) ;
	ECS_OBJECT * (__stdcall *pfnCreateAbstractObject)( ECS_CONTEXT * context ) ;
	ESLObject * (__stdcall *pfnGetWaveOutputDevice)( ECS_CONTEXT * context ) ;
	ESLObject * (__stdcall *pfnGetDrawImageObject)( ECS_CONTEXT * context ) ;
	HESLHEAP (__stdcall *pfnGetHeapHandle)( ECS_CONTEXT * context ) ;

	ExecutionStatus GetStatus( void )
		{	return	pfnGetStatus( this ) ;	}
	ExecutionStatus SetStatus( ExecutionStatus status )
		{	return	pfnSetStatus( this, status ) ;	}
	unsigned long int GetInstructionPointer( void )
		{	return	pfnGetInstructionPointer( this ) ;	}
	void SetInstructionPointer( unsigned long int ip )
		{	pfnSetInstructionPointer( this, ip ) ;	}
	ESLError Lock( DWORD dwTimeout )
		{	return	pfnLock( this, dwTimeout ) ;	}
	ESLError Unlock( void )
		{	return	pfnUnlock( this ) ;	}
	ESLError LockExecution( DWORD dwTimeout )
		{	return	pfnLockExecution( this, dwTimeout ) ;	}
	ESLError UnlockExecution( void )
		{	return	pfnUnlockExecution( this ) ;	}
	ESLError CallFunction
		( DWORD dwFuncAddr, ECS_OBJECT * const* pArg, int nArgCount )
		{	return	pfnCallFunction( this, dwFuncAddr, pArg, nArgCount ) ;	}
	ESLError PushObject( ECS_OBJECT * pObj )
		{	return	pfnPushObject( this, pObj ) ;	}
	ECS_OBJECT * PopObject( void )
		{	return	pfnPopObject( this ) ;	}
	ECS_FILE * OpenFile
		( const wchar_t * pwszFileName,
			long int nOpenFlags = ECS_FILE::modeRead | ECS_FILE::shareRead )
		{	return	pfnOpenFile( this, pwszFileName, nOpenFlags ) ;	}
	ESLError Save( ECS_FILE * pfile )
		{	return	pfnSave( this, pfile ) ;	}
	ESLError Load( ECS_FILE * pfile )
		{	return	pfnLoad( this, pfile ) ;	}
	ESLError SaveObject( ECS_FILE * pfile, ECS_OBJECT * pObj )
		{	return	pfnSaveObject( this, pfile, pObj ) ;	}
	ESLError LoadObject( ECS_FILE * pfile, ECS_OBJECT ** pObj )
		{	return	pfnLoadObject( this, pfile, pObj ) ;	}
	ECS_OBJECT * CreateObject
		( CSVariableType csvtType,
			const wchar_t * pwszType, DWORD * pdwFuncAddr = NULL )
		{	return	pfnCreateObject( this, csvtType, pwszType, pdwFuncAddr ) ;	}
	ECS_OBJECT * GetStack( void )
		{	return	pfnGetStack( this ) ;	}
	ECS_OBJECT * GetGlobal( void )
		{	return	pfnGetGlobal( this ) ;	}
	ECS_OBJECT * GetStatic( void )
		{	return	pfnGetStatic( this ) ;	}
	ECS_OBJECT * CreateReference( ECS_OBJECT * pRef = NULL )
		{	return	pfnCreateReference( this, pRef ) ;	}
	ECS_OBJECT * CreateInteger( long int nInitVal = 0 )
		{	return	pfnCreateInteger( this, nInitVal ) ;	}
	ECS_OBJECT * CreateReal( double rInitVal = 0.0 )
		{	return	pfnCreateReal( this, rInitVal ) ;	}
	ECS_OBJECT * CreateString( const wchar_t * pwszInitVal = NULL )
		{	return	pfnCreateString( this, pwszInitVal ) ;	}
	ECS_OBJECT * CreateAbstractObject( void )
		{	return	pfnCreateAbstractObject( this ) ;	}
	ESLObject * GetWaveOutputDevice( void )
		{	return	pfnGetWaveOutputDevice( this ) ;	}
	ESLObject * GetDrawImageObject( void )
		{	return	pfnGetDrawImageObject( this ) ;	}
	ECS_OBJECT * GetObjectElement
			( ECS_OBJECT * pObj, const wchar_t * pwszName ) ;
	ESLError SetObjectElement
			( ECS_OBJECT * pObj,
				const wchar_t * pwszName, ECS_OBJECT * pElement ) ;

} ;


//////////////////////////////////////////////////////////////////////////////
//	オブジェクト抽象インターフェース
//////////////////////////////////////////////////////////////////////////////

struct	ECS_REFERENCE_INTERFACE ;
struct	ECS_INTEGER_INTERFACE ;
struct	ECS_REAL_INTERFACE ;
struct	ECS_STRING_INTERFACE ;
struct	ECS_ARRAY_INTERFACE ;
struct	ECS_HASH_INTERFACE ;
struct	ECS_RESOURCE_INTERFACE ;
struct	ECS_SPRITE_INTERFACE ;
struct	ECS_WINDOW_INTERFACE ;
struct	ECS_INPUT_FILTER_INTERFACE ;
struct	ECS_FILE_INTERFACE ;
struct	ECS_OBJECT
{
	// オブジェクトインスタンス
	void *		ptrInstance ;
	// オブジェクト破棄
	void (__stdcall * pfnRelease)( ECS_OBJECT * instance ) ;
	// インターフェース取得
	void * (__stdcall *pfnQueryInterface)
		( ECS_OBJECT * instance, const wchar_t * pwszType ) ;
	// スクリプトオブジェクトインターフェース
	const wchar_t * (__stdcall *pfnGetTypeName)( ECS_OBJECT * instance ) ;
	ECS_OBJECT * (__stdcall *pfnGetTypeOf)
		( ECS_OBJECT * instance, const wchar_t * pwszTypeName ) ;
	ECS_OBJECT * (__stdcall *pfnDuplicate)( ECS_OBJECT * instance ) ;
	ESLError (__stdcall *pfnMove)
		( ECS_OBJECT * instance, ECS_CONTEXT * context,
			const ECS_OBJECT * obj ) ;
	ESLError (__stdcall *pfnUnaryOperate)
		( ECS_OBJECT * instance, ECS_CONTEXT * context,
			CSUnaryOperatorType csuopType ) ;
	ESLError (__stdcall *pfnOperate)
		( ECS_OBJECT * instance, ECS_CONTEXT * context,
			CSOperatorType csopType, ECS_OBJECT * obj ) ;
	ESLError (__stdcall *pfnCompare)
		( ECS_OBJECT * instance, ECS_CONTEXT * context,
			int * pResult, CSCompareType cscpType, ECS_OBJECT * obj ) ;
	ESLError (__stdcall *pfnGetVariableIndexInt)
		( ECS_OBJECT * instance, int * pIndex, int iElement ) ;
	ESLError (__stdcall *pfnGetVariableIndexStr)
		( ECS_OBJECT * instance, int * pIndex, const wchar_t * pwszElement ) ;
	ECS_OBJECT * (__stdcall *pfnGetVariableAt)
		( ECS_OBJECT * instance, int nIndex ) ;
	ECS_OBJECT * (__stdcall *pfnSetVariableAt)
		( ECS_OBJECT * instance, int nIndex, ECS_OBJECT * obj ) ;
	ESLError (__stdcall *pfnGetFunction)
		( ECS_OBJECT * instance, ECS_CONTEXT * context,
			int * pIndex, const wchar_t * pwszName ) ;
	ESLError (__stdcall *pfnCallFunction)
		( ECS_OBJECT * instance, ECS_CONTEXT * context,
			int nIndex, ECS_OBJECT * const* pArg, int nArgCount ) ;
	// オブジェクトシリアル化インターフェース
	void (__stdcall *pfnIndexAllMember)( ECS_OBJECT * instance ) ;
	void (__stdcall *pfnCleanupAllReference)
		( ECS_OBJECT * instance, ECS_CONTEXT * context ) ;
	ESLError (__stdcall *pfnCommitAllReference)
		( ECS_OBJECT * instance, ECS_CONTEXT * context ) ;
	ESLError (__stdcall *pfnSave)
		( ECS_OBJECT * instance,
			ECS_FILE * pfile, ECS_CONTEXT * context ) ;
	ESLError (__stdcall *pfnLoad)
		( ECS_OBJECT * instance,
			ECS_FILE * pfile, ECS_CONTEXT * context ) ;
	ESLError (__stdcall *pfnDumpObject)
		( ECS_OBJECT * instance, ECS_FILE * pfile,
			int nIndent, ECS_CONTEXT * context ) ;
	// 予約領域
	void *	ptrReserved[11] ;

	void Release( void )
		{	pfnRelease( this ) ;	}
	const wchar_t * GetTypeName( void )
		{	return	pfnGetTypeName( this ) ;	}
	ECS_OBJECT * GetTypeOf( const wchar_t * pwszTypeName )
		{	return	pfnGetTypeOf( this, pwszTypeName ) ;	}
	ECS_OBJECT * Duplicate( void )
		{	return	pfnDuplicate( this ) ;	}
	ESLError Move( ECS_CONTEXT * context, const ECS_OBJECT * obj )
		{	return	pfnMove( this, context, obj ) ;	}
	ESLError UnaryOperate
			( ECS_CONTEXT * context, CSUnaryOperatorType csuopType )
		{	return	pfnUnaryOperate( this, context, csuopType ) ;	}
	ESLError Operate
			( ECS_CONTEXT * context,
				CSOperatorType csopType, ECS_OBJECT * obj )
		{	return	pfnOperate( this, context, csopType, obj ) ;	}
	ESLError Compare
			( ECS_CONTEXT * context,
				int * pResult, CSCompareType cscpType, ECS_OBJECT * obj )
		{	return	pfnCompare( this, context, pResult, cscpType, obj ) ;	}
	ESLError GetVariableIndex( int * pIndex, int iElement )
		{	return	pfnGetVariableIndexInt( this, pIndex, iElement ) ;	}
	ESLError GetVariableIndex( int * pIndex, const wchar_t * pwszElement )
		{	return	pfnGetVariableIndexStr( this, pIndex, pwszElement ) ;	}
	ECS_OBJECT * GetVariableAt( int nIndex )
		{	return	pfnGetVariableAt( this, nIndex ) ;	}
	ECS_OBJECT * SetVariableAt( int nIndex, ECS_OBJECT * obj )
		{	return	pfnSetVariableAt( this, nIndex, obj ) ;	}
	ESLError GetFunction
			( ECS_CONTEXT * context,
				int * pIndex, const wchar_t * pwszName )
		{	return	pfnGetFunction( this, context, pIndex, pwszName ) ;	}
	ESLError CallFunction
			( ECS_CONTEXT * context,
				int nIndex, ECS_OBJECT * const* pArg, int nArgCount )
		{	return	pfnCallFunction( this, context, nIndex, pArg, nArgCount ) ;	}
	ESLError CallFunctionAs
			( ECS_CONTEXT * context,
				const wchar_t * pwszName,
				ECS_OBJECT * const* pArg, int nArgCount )
		{
			int			nIndex ;
			ESLError	err = GetFunction( context, &nIndex, pwszName ) ;
			if ( !err )
			{
				err = CallFunction
					( context, nIndex, pArg, nArgCount ) ;
			}
			return	err ;
		}
	void IndexAllMember( void )
		{	pfnIndexAllMember( this ) ;	}
	void CleanupAllReference( ECS_CONTEXT * context )
		{	pfnCleanupAllReference( this, context ) ;	}
	ESLError CommitAllReference( ECS_CONTEXT * context )
		{	return	pfnCommitAllReference( this, context ) ;	}
	ESLError Save( ECS_FILE * pfile, ECS_CONTEXT * context )
		{	return	pfnSave( this, pfile, context ) ;	}
	ESLError Load( ECS_FILE * pfile, ECS_CONTEXT * context )
		{	return	pfnLoad( this, pfile, context ) ;	}
	ESLError DumpObject
			( ECS_FILE * pfile, int nIndent, ECS_CONTEXT * context )
		{	return	pfnDumpObject( this, pfile, nIndent, context ) ;	}

	void * QueryInterface( const wchar_t * pwszType )
		{
			if ( this == NULL )	return	NULL ;
			return	pfnQueryInterface( this, pwszType ) ;
		}
	ECS_REFERENCE_INTERFACE * QueryReferenceInterface( void )
		{
			if ( this == NULL )	return	NULL ;
			return	(ECS_REFERENCE_INTERFACE*)
				pfnQueryInterface( this, L"ECS_REFERENCE_INTERFACE" ) ;
		}
	ECS_INTEGER_INTERFACE * QueryIntegerInterface( void )
		{
			if ( this == NULL )	return	NULL ;
			return	(ECS_INTEGER_INTERFACE*)
				pfnQueryInterface( this, L"ECS_INTEGER_INTERFACE" ) ;
		}
	ECS_REAL_INTERFACE * QueryRealInterface( void )
		{
			if ( this == NULL )	return	NULL ;
			return	(ECS_REAL_INTERFACE*)
				pfnQueryInterface( this, L"ECS_REAL_INTERFACE" ) ;
		}
	ECS_STRING_INTERFACE * QueryStringInterface( void )
		{
			if ( this == NULL )	return	NULL ;
			return	(ECS_STRING_INTERFACE*)
				pfnQueryInterface( this, L"ECS_STRING_INTERFACE" ) ;
		}
	ECS_ARRAY_INTERFACE * QueryArrayInterface( void )
		{
			if ( this == NULL )	return	NULL ;
			return	(ECS_ARRAY_INTERFACE*)
				pfnQueryInterface( this, L"ECS_ARRAY_INTERFACE" ) ;
		}
	ECS_HASH_INTERFACE * QueryHashInterface( void )
		{
			if ( this == NULL )	return	NULL ;
			return	(ECS_HASH_INTERFACE*)
				pfnQueryInterface( this, L"ECS_HASH_INTERFACE" ) ;
		}
	ECS_RESOURCE_INTERFACE * QueryResourceInterface( void )
		{
			if ( this == NULL )	return	NULL ;
			return	(ECS_RESOURCE_INTERFACE*)
				pfnQueryInterface( this, L"ECS_RESOURCE_INTERFACE" ) ;
		}
	ECS_SPRITE_INTERFACE * QuerySpriteInterface( void )
		{
			if ( this == NULL )	return	NULL ;
			return	(ECS_SPRITE_INTERFACE*)
				pfnQueryInterface( this, L"ECS_SPRITE_INTERFACE" ) ;
		}
	ECS_WINDOW_INTERFACE * QueryWindowInterface( void )
		{
			if ( this == NULL )	return	NULL ;
			return	(ECS_WINDOW_INTERFACE*)
				pfnQueryInterface( this, L"ECS_WINDOW_INTERFACE" ) ;
		}
	ECS_INPUT_FILTER_INTERFACE * QuertInputFilterInterface( void )
		{
			if ( this == NULL )	return	NULL ;
			return	(ECS_INPUT_FILTER_INTERFACE*)
				pfnQueryInterface( this, L"ECS_INPUT_FILTER_INTERFACE" ) ;
		}
	ECS_FILE_INTERFACE * QueryFileInterface( void )
		{
			if ( this == NULL )	return	NULL ;
			return	(ECS_FILE_INTERFACE*)
				pfnQueryInterface( this, L"ECS_FILE_INTERFACE" ) ;
		}
	class ECSObject * QueryECSObject( void )
		{
			if ( this == NULL )	return	NULL ;
			return	(ECSObject*) pfnQueryInterface( this, L"ECSObject" ) ;
		}

	inline ECS_OBJECT * GetObjectEntity( void ) ;
	inline long int GetInteger( int nDefValue = 0 ) ;
	inline void SetInteger( int nValue ) ;
	inline INT64 GetInteger64( INT64 nDefValue = 0 ) ;
	inline void SetInteger64( INT64 nValue ) ;
	inline double GetReal( double rDefValue = 0 ) ;
	inline void SetReal( double rValue ) ;
	inline const wchar_t * GetString( const wchar_t * pwszDefValue = NULL ) ;
	inline void SetString
		( const wchar_t * pwszValue, unsigned int nLength = 0 ) ;

} ;

inline ECS_OBJECT * ECS_CONTEXT::GetObjectElement
		( ECS_OBJECT * pObj, const wchar_t * pwszName )
{
	ECS_OBJECT *	pElement = NULL ;
	int				nIndex ;
	if ( !pObj->GetVariableIndex( &nIndex, pwszName ) )
	{
		pElement = pObj->GetVariableAt( nIndex ) ;
	}
	return	pElement ;
}

inline ESLError ECS_CONTEXT::SetObjectElement
	( ECS_OBJECT * pObj, const wchar_t * pwszName, ECS_OBJECT * pElement )
{
	ESLError	err ;
	int			nIndex ;
	err = pObj->GetVariableIndex( &nIndex, pwszName ) ;
	if ( !err )
	{
		pObj->SetVariableAt( nIndex, pElement ) ;
	}
	return	err ;
}


//////////////////////////////////////////////////////////////////////////////
//	参照型
//////////////////////////////////////////////////////////////////////////////

struct	ECS_REFERENCE_INTERFACE
{
	ECS_OBJECT * (__stdcall *pfnGetObjectEntity)
		( ECS_REFERENCE_INTERFACE * instance ) ;
	void (__stdcall *pfnSetReference)
		( ECS_REFERENCE_INTERFACE * instance,
			ECS_OBJECT * pRef, ECS_CONTEXT * context ) ;

	ECS_OBJECT * GetObjectEntity( void )
		{	return	pfnGetObjectEntity( this ) ;	}
	void SetReference( ECS_OBJECT * pRef, ECS_CONTEXT * context )
		{	pfnSetReference( this, pRef, context ) ;	}
} ;

inline ECS_OBJECT * ECS_OBJECT::GetObjectEntity( void )
{
	if ( this == NULL )
	{
		return	NULL ;
	}
	ECS_REFERENCE_INTERFACE *
		pri = (ECS_REFERENCE_INTERFACE*)
			pfnQueryInterface( this, L"ECS_REFERENCE_INTERFACE" ) ;
	if ( pri != NULL )
	{
		return	pri->GetObjectEntity( ) ;
	}
	return	this ;
}


//////////////////////////////////////////////////////////////////////////////
//	整数型
//////////////////////////////////////////////////////////////////////////////

struct	ECS_INTEGER_INTERFACE
{
	long int (__stdcall *pfnGetInteger)
		( ECS_INTEGER_INTERFACE * instance ) ;
	void (__stdcall *pfnSetInteger)
		( ECS_INTEGER_INTERFACE * instance, long int nVal ) ;
	INT64 (__stdcall *pfnGetInteger64)
		( ECS_INTEGER_INTERFACE * instance ) ;
	void (__stdcall *pfnSetInteger64)
		( ECS_INTEGER_INTERFACE * instance, INT64 nVal ) ;

	long int GetInteger( void )
		{	return	pfnGetInteger( this ) ;	}
	INT64 GetInteger64( void )
		{	return	pfnGetInteger64( this ) ;	}
	void SetInteger( long int nVal )
		{	pfnSetInteger( this, nVal ) ;	}
	void SetInteger64( INT64 nVal )
		{	pfnSetInteger64( this, nVal ) ;	}
} ;

inline long int ECS_OBJECT::GetInteger( int nDefValue )
{
	ECS_OBJECT *	pObj = GetObjectEntity( ) ;
	if ( pObj != NULL )
	{
		ECS_INTEGER_INTERFACE *	piObj = pObj->QueryIntegerInterface( ) ;
		if ( piObj != NULL )
		{
			return	piObj->GetInteger( ) ;
		}
	}
	return	nDefValue ;
}

inline void ECS_OBJECT::SetInteger( int nValue )
{
	ECS_OBJECT *	pObj = GetObjectEntity( ) ;
	if ( pObj != NULL )
	{
		ECS_INTEGER_INTERFACE *	piObj = pObj->QueryIntegerInterface( ) ;
		if ( piObj != NULL )
		{
			piObj->SetInteger( nValue ) ;
		}
	}
}

inline INT64 ECS_OBJECT::GetInteger64( INT64 nDefValue )
{
	ECS_OBJECT *	pObj = GetObjectEntity( ) ;
	if ( pObj != NULL )
	{
		ECS_INTEGER_INTERFACE *	piObj = pObj->QueryIntegerInterface( ) ;
		if ( piObj != NULL )
		{
			return	piObj->GetInteger64( ) ;
		}
	}
	return	nDefValue ;
}

inline void ECS_OBJECT::SetInteger64( INT64 nValue )
{
	ECS_OBJECT *	pObj = GetObjectEntity( ) ;
	if ( pObj != NULL )
	{
		ECS_INTEGER_INTERFACE *	piObj = pObj->QueryIntegerInterface( ) ;
		if ( piObj != NULL )
		{
			piObj->SetInteger64( nValue ) ;
		}
	}
}


//////////////////////////////////////////////////////////////////////////////
//	実数型
//////////////////////////////////////////////////////////////////////////////

struct	ECS_REAL_INTERFACE
{
	double (__stdcall *pfnGetReal)
		( ECS_REAL_INTERFACE * instance ) ;
	void (__stdcall *pfnSetReal)
		( ECS_REAL_INTERFACE * instance, double rVal ) ;

	double GetReal( void )
		{	return	pfnGetReal( this ) ;	}
	void SetReal( double rVal )
		{	pfnSetReal( this, rVal ) ;	}
} ;

inline double ECS_OBJECT::GetReal( double rDefValue )
{
	ECS_OBJECT *	pObj = GetObjectEntity( ) ;
	if ( pObj != NULL )
	{
		ECS_REAL_INTERFACE *	prObj = pObj->QueryRealInterface( ) ;
		if ( prObj != NULL )
		{
			return	prObj->GetReal( ) ;
		}
	}
	return	rDefValue ;
}

inline void ECS_OBJECT::SetReal( double rValue )
{
	ECS_OBJECT *	pObj = GetObjectEntity( ) ;
	if ( pObj != NULL )
	{
		ECS_REAL_INTERFACE *	prObj = pObj->QueryRealInterface( ) ;
		if ( prObj != NULL )
		{
			prObj->SetReal( rValue ) ;
		}
	}
}


//////////////////////////////////////////////////////////////////////////////
//	文字列型
//////////////////////////////////////////////////////////////////////////////

struct	ECS_STRING_INTERFACE
{
	const wchar_t * (__stdcall *pfnGetString)
		( ECS_STRING_INTERFACE * instance ) ;
	wchar_t * (__stdcall *pfnGetBuffer)
		( ECS_STRING_INTERFACE * instance, unsigned int nLength ) ;
	void (__stdcall *pfnReleaseBuffer)
		( ECS_STRING_INTERFACE * instance, int nLength ) ;

	const wchar_t * GetString( void )
		{	return	pfnGetString( this ) ;	}
	wchar_t * GetBuffer( unsigned int nLength )
		{	return	pfnGetBuffer( this, nLength ) ;	}
	void ReleaseBuffer( int nLength = -1 )
		{	pfnReleaseBuffer( this, nLength ) ;	}
	void SetString( const wchar_t * pwszValue, unsigned int nLength = 0 )
		{
			if ( (pwszValue != NULL) && (nLength == 0) )
			{
				while ( pwszValue[nLength] )
					nLength ++ ;
			}
			wchar_t *	pwszBuf = GetBuffer( nLength ) ;
			for ( unsigned int i = 0; i < nLength; i ++ )
			{
				pwszBuf[i] = pwszValue[i] ;
			}
			ReleaseBuffer( nLength ) ;
		}
} ;

inline const wchar_t * ECS_OBJECT::GetString( const wchar_t * pwszDefValue )
{
	ECS_OBJECT *	pObj = GetObjectEntity( ) ;
	if ( pObj != NULL )
	{
		ECS_STRING_INTERFACE *	psObj = pObj->QueryStringInterface( ) ;
		if ( psObj != NULL )
		{
			return	psObj->GetString( ) ;
		}
	}
	return	pwszDefValue ;
}

inline void ECS_OBJECT::SetString
	( const wchar_t * pwszValue, unsigned int nLength )
{
	ECS_OBJECT *	pObj = GetObjectEntity( ) ;
	if ( pObj != NULL )
	{
		ECS_STRING_INTERFACE *	psObj = pObj->QueryStringInterface( ) ;
		if ( psObj != NULL )
		{
			psObj->SetString( pwszValue, nLength ) ;
		}
	}
}


//////////////////////////////////////////////////////////////////////////////
// 配列型
//////////////////////////////////////////////////////////////////////////////

struct	ECS_ARRAY_INTERFACE
{
	unsigned int (__stdcall *pfnGetLength)( ECS_ARRAY_INTERFACE * instance ) ;
	int (__stdcall *pfnIsEmpty)( ECS_ARRAY_INTERFACE * instance, int nIndex ) ;
	void (__stdcall *pfnSwap)
		( ECS_ARRAY_INTERFACE * instance, int nIndex1, int nIndex2 ) ;
	void (__stdcall *pfnInsert)
		( ECS_ARRAY_INTERFACE * instance, int nIndex, ECS_OBJECT * pObj ) ;
	void (__stdcall *pfnRemove)
		( ECS_ARRAY_INTERFACE * instance, int nIndex, int nCount ) ;
	ECS_OBJECT * (__stdcall *pfnDetach)
		( ECS_ARRAY_INTERFACE * instance, int nIndex ) ;

	unsigned int GetLength( void )
		{	return	pfnGetLength( this ) ;	}
	int IsEmpty( int nIndex )
		{	return	pfnIsEmpty( this, nIndex ) ;	}
	void Swap( int nIndex1, int nIndex2 )
		{	pfnSwap( this, nIndex1, nIndex2 ) ;	}
	void Insert( int nIndex, ECS_OBJECT * pObj )
		{	pfnInsert( this, nIndex, pObj ) ;	}
	void Remove( int nIndex = 0, int nCount = -1 )
		{	pfnRemove( this, nIndex, nCount ) ;	}
	ECS_OBJECT * Detach( int nIndex )
		{	return	pfnDetach( this, nIndex ) ;	}

} ;


//////////////////////////////////////////////////////////////////////////////
// ハッシュ型
//////////////////////////////////////////////////////////////////////////////

struct	ECS_HASH_INTERFACE
{
	unsigned int (__stdcall *pfnGetLength)( ECS_HASH_INTERFACE * instance ) ;
	ECS_OBJECT * (__stdcall *pfnGetElement)
		( ECS_HASH_INTERFACE * instance, const wchar_t * pwszTag ) ;
	ECS_OBJECT * (__stdcall *pfnGetTagName)
		( ECS_HASH_INTERFACE * instance, int nIndex ) ;
	void (__stdcall *pfnRemove)
		( ECS_HASH_INTERFACE * instance, const wchar_t * pwszTag ) ;
	void (__stdcall *pfnRemoveAll)( ECS_HASH_INTERFACE * instance ) ;
	void (__stdcall *pfnSetDefaultElement)
		( ECS_HASH_INTERFACE * instance, ECS_OBJECT * pDefault ) ;

	unsigned int GetLength( void )
		{	return	pfnGetLength( this ) ;	}
	ECS_OBJECT * GetElement( const wchar_t * pwszTag )
		{	return	pfnGetElement( this, pwszTag ) ;	}
	ECS_OBJECT * GetTagName( int nIndex )
		{	return	pfnGetTagName( this, nIndex ) ;	}
	void Remove( const wchar_t * pwszTag )
		{	pfnRemove( this, pwszTag ) ;	}
	void RemoveAll( void )
		{	pfnRemoveAll( this ) ;	}
	void SetDefaultElement( ECS_OBJECT * pDefault )
		{	pfnSetDefaultElement( this, pDefault ) ;	}

	long int GetElementAsInteger( const wchar_t * pwszTag, long int nDefValue = 0 )
		{
			ECS_OBJECT *	pElement = GetElement( pwszTag ) ;
			return	pElement ? pElement->GetInteger( nDefValue ) : nDefValue ;
		}
	double GetElementAsReal( const wchar_t * pwszTag, double rDefValue = 0 )
		{
			ECS_OBJECT *	pElement = GetElement( pwszTag ) ;
			return	pElement ? pElement->GetReal( rDefValue ) : rDefValue ;
		}
	const wchar_t * GetElementAsString
			( const wchar_t * pwszTag, const wchar_t * pwszDefValue = NULL )
		{
			ECS_OBJECT *	pElement = GetElement( pwszTag ) ;
			return	pElement ? pElement->GetString( pwszDefValue ) : pwszDefValue ;
		}
	void SetElementAsInteger( const wchar_t * pwszTag, long int nValue )
		{
			ECS_OBJECT *	pElement = GetElement( pwszTag ) ;
			if ( pElement != NULL )
			{
				pElement->SetInteger( nValue ) ;
			}
		}
	void SetElementAsReal( const wchar_t * pwszTag, double rValue )
		{
			ECS_OBJECT *	pElement = GetElement( pwszTag ) ;
			if ( pElement != NULL )
			{
				pElement->SetReal( rValue ) ;
			}
		}
	void SetElementAsString
			( const wchar_t * pwszTag,
				const wchar_t * pwszValue, unsigned int nLength = 0 )
		{
			ECS_OBJECT *	pElement = GetElement( pwszTag ) ;
			if ( pElement != NULL )
			{
				pElement->SetString( pwszValue, nLength ) ;
			}
		}

} ;



//////////////////////////////////////////////////////////////////////////////
// リソースインターフェース
//////////////////////////////////////////////////////////////////////////////

struct	ECS_RESOURCE_INTERFACE
{
	enum	PlayTypeFlag
	{
		ptfDevice	= 0x80000000,
		ptfNothing	= -1,
		ptfMusic,	ptfSound,	ptfVoice,	ptfSystem,
		ptfMax,
	} ;
	enum	ResourceOwnFlag
	{
		rofNothing	= 0,
		rofImage,	rofSound,	rofMidi
	} ;
	ESLError (__stdcall *pfnLoadImageFile)
		( ECS_RESOURCE_INTERFACE * instance,
			const wchar_t * pwszFilePath, ECS_CONTEXT * pContext ) ;
	ESLError (__stdcall *pfnLoadSoundFile)
		( ECS_RESOURCE_INTERFACE * instance,
			const wchar_t * pwszFilePath,
			unsigned int nThreshold, ECS_CONTEXT * pContext ) ;
	ESLError (__stdcall *pfnLoadMidiFile)
		( ECS_RESOURCE_INTERFACE * instance,
			const wchar_t * pwszFilePath, ECS_CONTEXT * pContext ) ;
	ESLError (__stdcall *pfnAttachSound)
		( ECS_RESOURCE_INTERFACE * instance, ECS_RESOURCE_INTERFACE * pRsrc ) ;
	void (__stdcall *pfnRelease)
		( ECS_RESOURCE_INTERFACE * instance ) ;
	ESLError (__stdcall *pfnPlayFrom)
		( ECS_RESOURCE_INTERFACE * instance,
			unsigned int nStartPos, unsigned int nEndPos,
			int fRepeat, unsigned int nRewindPos, int fPlayType ) ;
	ESLError (__stdcall *pfnStop)
		( ECS_RESOURCE_INTERFACE * instance ) ;
	ESLError (__stdcall *pfnPause)
		( ECS_RESOURCE_INTERFACE * instance ) ;
	ESLError (__stdcall *pfnRestart)
		( ECS_RESOURCE_INTERFACE * instance ) ;
	ESLError (__stdcall *pfnGetVolume)
		( ECS_RESOURCE_INTERFACE * instance,
			REAL32 & rLeftVol, REAL32 & rRightVol ) ;
	ESLError (__stdcall *pfnSetVolume)
		( ECS_RESOURCE_INTERFACE * instance,
			REAL32 rLeftVol, REAL32 rRightVol ) ;
	int (__stdcall *pfnIsPlaying)
		( ECS_RESOURCE_INTERFACE * instance ) ;
	ESLError (__stdcall *pfnSetVolumeEnvelope)
		( ECS_RESOURCE_INTERFACE * instance,
			const E3D_VECTOR_2D * bezier, unsigned int nDurationTime ) ;
	void (__stdcall *pfnCancelVolumeEnvelope)
		( ECS_RESOURCE_INTERFACE * instance ) ;
	int (__stdcall *pfnIsPendingEnvelope)
		( ECS_RESOURCE_INTERFACE * instance ) ;
	REAL32 (__stdcall *pfnGetTotalVolume)
		( ECS_RESOURCE_INTERFACE * instance, int fPlayType ) ;
	ESLError (__stdcall *pfnSetTotalVolume)
		( ECS_RESOURCE_INTERFACE * instance, int fPlayType, REAL32 rVolume ) ;
	ESLError (__stdcall *pfnSetResource)
		( ECS_RESOURCE_INTERFACE * instance, ESLObject * pRsrc,
			ResourceOwnFlag rofType, const wchar_t * pwszFileName ) ;
	ESLObject * (__stdcall *pfnGetResource)
		( ECS_RESOURCE_INTERFACE * instance ) ;

	// 画像リソースを読み込む
	ESLError LoadImageFile
		( const wchar_t * pwszFilePath, ECS_CONTEXT * pContext = NULL )
		{	return	pfnLoadImageFile( this, pwszFilePath, pContext ) ;	}
	// 音声リソースを読み込む
	ESLError LoadSoundFile
		( const wchar_t * pwszFilePath,
			unsigned int nThreshold, ECS_CONTEXT * pContext = NULL )
		{	return	pfnLoadSoundFile( this, pwszFilePath, nThreshold, pContext ) ;	}
	// MIDI リソースを読み込む
	ESLError LoadMidiFile
		( const wchar_t * pwszFilePath, ECS_CONTEXT * pContext = NULL )
		{	return	pfnLoadMidiFile( this, pwszFilePath, pContext ) ;	}
	// 音声関連付け
	ESLError AttachSound( ECS_RESOURCE_INTERFACE * pRsrc )
		{	return	pfnAttachSound( this, pRsrc ) ;	}
	// リソースを解放する
	void Release( void )
		{	pfnRelease( this ) ;	}
	// 音声リソースを再生する
	ESLError PlayFrom
		( unsigned int nStartPos = 0, unsigned int nEndPos = -1, 
			bool fRepeat = false, unsigned int nRewindPos = -1,
			int fPlayType = ptfMusic )
		{	return	pfnPlayFrom
				( this, nStartPos, nEndPos, fRepeat, nRewindPos, fPlayType ) ;	}
	// 音声リソースの再生を停止する
	ESLError Stop( void )
		{	return	pfnStop( this ) ;	}
	// 音声リソースの再生を一時停止する
	ESLError Pause( void )
		{	return	pfnPause( this ) ;	}
	// 音声リソースの再生を再開する
	ESLError Restart( void )
		{	return	pfnRestart( this ) ;	}
	// 音量を取得する
	ESLError GetVolume( REAL32 & rLeftVol, REAL32 & rRightVol )
		{	return	pfnGetVolume( this, rLeftVol, rRightVol ) ;	}
	// 音量を設定する
	ESLError SetVolume( REAL32 rLeftVol, REAL32 rRightVol )
		{	return	pfnSetVolume( this, rLeftVol, rRightVol ) ;	}
	// 再生中か調べる
	int IsPlaying( void )
		{	return	pfnIsPlaying( this ) ;	}
	// 音量エンベロープを設定する
	ESLError SetVolumeEnvelope
		( const E3D_VECTOR_2D * bezier, unsigned int nDurationTime )
		{	return	pfnSetVolumeEnvelope( this, bezier, nDurationTime ) ;	}
	// 音量エンベロープをキャンセルする
	void CancelVolumeEnvelope( void )
		{	pfnCancelVolumeEnvelope( this ) ;	}
	// 音量エンベロープ実行中か調べる
	int IsPendingEnvelope( void )
		{	return	pfnIsPendingEnvelope( this ) ;	}
	// 全体音量を取得する
	REAL32 GetTotalVolume( int fPlayType )
		{	return	pfnGetTotalVolume( this, fPlayType ) ;	}
	// 全体音量を設定する
	ESLError SetTotalVolume( int fPlayType, REAL32 rVolume )
		{	return	pfnSetTotalVolume( this, fPlayType, rVolume ) ;	}
	// リソースを設定する
	ESLError SetResource
		( ESLObject * pRsrc,
			ResourceOwnFlag rofType, const wchar_t * pwszFileName )
		{	return	pfnSetResource( this, pRsrc, rofType, pwszFileName ) ;	}
	// リソースを取得する
	ESLObject * GetResource( void )
		{	return	pfnGetResource( this ) ;	}

} ;


//////////////////////////////////////////////////////////////////////////////
//	スプライトインターフェース
//////////////////////////////////////////////////////////////////////////////

struct	ECS_SPRITE_INTERFACE
{
	enum	ActionType
	{
		actNormal,	actLoop, actTurnLoop,
	} ;
	// 基本情報
	PCEGL_IMAGE_INFO (__stdcall *pfnGetImageBuffer)
		( ECS_SPRITE_INTERFACE * instance ) ;
	HWND (__stdcall *pfnGetWindow)( ECS_SPRITE_INTERFACE * instance ) ;
	ESLError (__stdcall *pfnAttachImage)
		( ECS_SPRITE_INTERFACE * instance, ECS_OBJECT * pImage ) ;
	ESLError (__stdcall *pfnSetImageView)
		( ECS_SPRITE_INTERFACE * instance,
			PEGL_IMAGE_INFO pImage, PCEGL_RECT pViewRect ) ;
	ESLError (__stdcall *pfnCreateSprite)
		( ECS_SPRITE_INTERFACE * instance,
			DWORD fdwFormat, int nWidth, int nHeight ) ;
	void (__stdcall *pfnRelease)
		( ECS_SPRITE_INTERFACE * instance ) ;
	void (__stdcall *pfnSetBackColor)
		( ECS_SPRITE_INTERFACE * instance,
			EGL_PALETTE rgbBack, int fEnableBack ) ;
	int (__stdcall * pfnEnableDynamicMode)
		( ECS_SPRITE_INTERFACE * instance, int fDynamicMode ) ;
	PCEGL_IMAGE_INFO (__stdcall *pfnGetZBuffer)
		( ECS_SPRITE_INTERFACE * instance ) ;
	void (__stdcall *pfnCreateZBuffer)
		( ECS_SPRITE_INTERFACE * instance ) ;
	void (__stdcall *pfnDeleteZBuffer)
		( ECS_SPRITE_INTERFACE * instance ) ;
	const E3D_VECTOR * (__stdcall *pfnGetScreenPosition)
		( ECS_SPRITE_INTERFACE * instance ) ;
	void (__stdcall *pfnSetScreenPosition)
		( ECS_SPRITE_INTERFACE * instance, const E3D_VECTOR * vScreen ) ;
	DWORD (__stdcall *pfnGetDrawFunctionFlags)
		( ECS_SPRITE_INTERFACE * instance ) ;
	DWORD (__stdcall *pfnGetRenderFunctionFlags)
		( ECS_SPRITE_INTERFACE * instance ) ;
	void (__stdcall *pfnSetDrawFunctionFlags)
		( ECS_SPRITE_INTERFACE * instance, DWORD dwFlags ) ;
	void (__stdcall *pfnSetRenderFunctionFlags)
		( ECS_SPRITE_INTERFACE * instance, DWORD dwFlags ) ;
	ECS_OBJECT * (__stdcall *pfnGetParent)
		( ECS_SPRITE_INTERFACE * instance ) ;
	int (__stdcall *pfnIsVisible)
		( ECS_SPRITE_INTERFACE * instance ) ;
	void (__stdcall *pfnSetVisible)
		( ECS_SPRITE_INTERFACE * instance, int fVisible ) ;
	void (__stdcall *pfnGetRectangle)
		( ECS_SPRITE_INTERFACE * instance, EGL_RECT * rect ) ;
	void (__stdcall *pfnGetPosition)
		( ECS_SPRITE_INTERFACE * instance, EGL_POINT * pos ) ;
	void (__stdcall *pfnMovePosition)
		( ECS_SPRITE_INTERFACE * instance, long int xPos, long int yPos ) ;
	unsigned int (__stdcall *pfnGetTransparency)
		( ECS_SPRITE_INTERFACE * instance ) ;
	void (__stdcall *pfnSetTransparency)
		( ECS_SPRITE_INTERFACE * instance, unsigned int nTransparency ) ;
	void (__stdcall *pfnGetParameter)
		( ECS_SPRITE_INTERFACE * instance,
			EImageSprite::PARAMETER * param ) ;
	void (__stdcall *pfnSetParameter)
		( ECS_SPRITE_INTERFACE * instance,
			const EImageSprite::PARAMETER * param ) ;
	// 表示制御
	void (__stdcall *pfnUpdateRect)
		( ECS_SPRITE_INTERFACE * instance,
			const EGL_RECT * pUpdateRect ) ;
	void (__stdcall *pfnRefresh)
		( ECS_SPRITE_INTERFACE * instance ) ;
	int (__stdcall *pfnGetPriority)
		( ECS_SPRITE_INTERFACE * instance ) ;
	void (__stdcall *pfnChangePriority)
		( ECS_SPRITE_INTERFACE * instance, int nPriority ) ;
	void (__stdcall *pfnAddSprite)
		( ECS_SPRITE_INTERFACE * instance,
			int nPriority, ECS_SPRITE_INTERFACE * pChild ) ;
	ESLError (__stdcall *pfnDetachSprite)
		( ECS_SPRITE_INTERFACE * instance, ECS_SPRITE_INTERFACE * pChild ) ;
	ESLError (__stdcall *pfnDetachAllSprite)
		( ECS_SPRITE_INTERFACE * instance ) ;
	// アニメーション
	DWORD (__stdcall *pfnModifyAnimationFlags)
		( ECS_SPRITE_INTERFACE * instance,
			DWORD dwAddFlags, DWORD dwRemoveFlags ) ;
	ESLError (__stdcall *pfnBeginAnimation)
		( ECS_SPRITE_INTERFACE * instance,
			unsigned long int nLoopCount, unsigned long int nBeginFrame,
			unsigned long int nAnimationTime,
			unsigned long int nRewindSequence, unsigned long int nTurnSequence ) ;
	ESLError (__stdcall *pfnEndAnimation)
		( ECS_SPRITE_INTERFACE * instance ) ;
	int (__stdcall *pfnIsDuringAnimation)
		( ECS_SPRITE_INTERFACE * instance ) ;
	ESLError (__stdcall *pfnSetAlphaImage)
		( ECS_SPRITE_INTERFACE * instance,
			PEGL_IMAGE_INFO pBlendAlpha, unsigned int nAlphaRange ) ;
	unsigned int (__stdcall *pfnGetBlendDegree)
		( ECS_SPRITE_INTERFACE * instance ) ;
	void (__stdcall *pfnSetBlendDegree)
		( ECS_SPRITE_INTERFACE * instance, unsigned int nDegree ) ;
	ESLError (__stdcall *pfnSetBlendingEnvelope)
		( ECS_SPRITE_INTERFACE * instance,
			const double * pbzEnvelope, int nCount ) ;
	ESLError (__stdcall *pfnSetBezierCurve)
		( ECS_SPRITE_INTERFACE * instance,
			const E3D_VECTOR_2D * pbzCurve, int nCurveCount,
			const double * bzRev, int nRevCount,
			const E3D_VECTOR_2D * bzZoom, int nZoomCount ) ;
	ESLError (__stdcall *pfnBeginActivation)
		( ECS_SPRITE_INTERFACE * instance,
			const unsigned int nDurationTime[],
				int nDurationCount, int nActionType ) ;
	ESLError (__stdcall *pfnFlushActivation)
		( ECS_SPRITE_INTERFACE * instance ) ;
	ESLError (__stdcall *pfnCancelActivation)
		( ECS_SPRITE_INTERFACE * instance ) ;
	int (__stdcall *pfnIsActivation)
		( ECS_SPRITE_INTERFACE * instance ) ;

	// 画像バッファ取得
	PCEGL_IMAGE_INFO GetImageBuffer( void )
		{	return	pfnGetImageBuffer( this ) ;	}
	// ウィンドウハンドル取得
	HWND GetWindow( void )
		{	return	pfnGetWindow( this ) ;	}
	// 画像バッファ関連付け
	ESLError AttachImage( ECS_OBJECT * pImage )
		{	return	pfnAttachImage( this, pImage ) ;	}
	ESLError SetImageView( PEGL_IMAGE_INFO pImage, PCEGL_RECT pViewRect = NULL )
		{	return	pfnSetImageView( this, pImage, pViewRect ) ;	}
	// 画像バッファ作成
	ESLError CreateSprite( DWORD fdwFormat, int nWidth, int nHeight )
		{	return	pfnCreateSprite( this, fdwFormat, nWidth, nHeight ) ;	}
	// リソース解放
	void Release( void )
		{	pfnRelease( this ) ;	}
	// 背景色設定
	void SetBackColor( EGL_PALETTE rgbBack, bool fEnableBack )
		{	pfnSetBackColor( this, rgbBack, fEnableBack ) ;	}
	// 動的スプライトモード設定
	int EnableDynamicMode( bool fDynamicMode )
		{	return	pfnEnableDynamicMode( this, fDynamicMode ) ;	}
	// Z バッファ取得
	PCEGL_IMAGE_INFO GetZBuffer( void )
		{	return	pfnGetZBuffer( this ) ;	}
	// Z バッファ作成
	void CreateZBuffer( void )
		{	pfnCreateZBuffer( this ) ;	}
	// Z バッファ削除
	void DeleteZBuffer( void )
		{	pfnDeleteZBuffer( this ) ;	}
	// スクリーン座標取得
	const E3D_VECTOR & GetScreenPosition( void )
		{	return	*(pfnGetScreenPosition( this )) ;	}
	// スクリーン座標を設定
	void SetScreenPosition( const E3D_VECTOR & vScreen )
		{	pfnSetScreenPosition( this, &vScreen ) ;	}
	// 描画機能フラグ取得
	DWORD GetDrawFunctionFlags( void )
		{	return	pfnGetDrawFunctionFlags( this ) ;	}
	DWORD GetRenderFunctionFlags( void )
		{	return	pfnGetRenderFunctionFlags( this ) ;	}
	// 描画機能フラグ設定
	void SetDrawFunctionFlags( DWORD dwFlags )
		{	pfnSetDrawFunctionFlags( this, dwFlags ) ;	}
	void SetRenderFunctionFlags( DWORD dwFlags )
		{	pfnSetRenderFunctionFlags( this, dwFlags ) ;	}
	// 親スプライト取得
	ECS_OBJECT * GetParent( void )
		{	return	pfnGetParent( this ) ;	}
	// 表示状態取得
	int IsVisible( void )
		{	return	pfnIsVisible( this ) ;	}
	// 表示状態設定
	void SetVisible( bool fVisible )
		{	pfnSetVisible( this, fVisible ) ;	}
	// 表示外接矩形取得
	void GetRectangle( EGL_RECT & rect )
		{	pfnGetRectangle( this, &rect ) ;	}
	// 表示基準座標取得
	void GetPosition( EGL_POINT & pos )
		{	pfnGetPosition( this, &pos ) ;	}
	// 表示基準座標設定
	void MovePosition( long int xPos, long int yPos )
		{	pfnMovePosition( this, xPos, yPos ) ;	}
	// 透明度取得
	unsigned int GetTransparency( void )
		{	return	pfnGetTransparency( this ) ;	}
	// 透明度設定
	void SetTransparency( unsigned int nTransparency )
		{	pfnSetTransparency( this, nTransparency ) ;	}
	// 表示パラメータ取得
	void GetParameter( EImageSprite::PARAMETER & param )
		{	pfnGetParameter( this, &param ) ;	}
	// 表示パラメータ設定
	void SetParameter( const EImageSprite::PARAMETER & param )
		{	pfnSetParameter( this, &param ) ;	}
	// 更新領域通知
	void UpdateRect( const EGL_RECT * pUpdateRect = NULL )
		{	pfnUpdateRect( this, pUpdateRect ) ;	}
	// 更新領域を再描画
	void Refresh( void )
		{	pfnRefresh( this ) ;	}
	// 表示優先度取得
	int GetPriority( void )
		{	return	pfnGetPriority( this ) ;	}
	// 表示優先度変更
	void ChangePriority( int nPriority )
		{	pfnChangePriority( this, nPriority ) ;	}
	// 子スプライト追加
	void AddSprite( int nPriority, ECS_SPRITE_INTERFACE * pChild )
		{	pfnAddSprite( this, nPriority, pChild ) ;	}
	// スプライトを分離
	ESLError DetachSprite( ECS_SPRITE_INTERFACE * pChild )
		{	return	pfnDetachSprite( this, pChild ) ;	}
	ESLError DetachAllSprite( void )
		{	return	pfnDetachAllSprite( this ) ;	}
	// アニメーション機能フラグを設定する
	DWORD ModifyAnimationFlags
		( DWORD dwAddFlags = 0, DWORD dwRemoveFlags = 0 )
		{	return	pfnModifyAnimationFlags( this, dwAddFlags, dwRemoveFlags ) ;	}
	DWORD GetAnimationFlags( void )
		{	return	pfnModifyAnimationFlags( this, 0, 0 ) ;	}
	void SetAnimationFlags( DWORD dwFlags )
		{	pfnModifyAnimationFlags( this, dwFlags, ~dwFlags ) ;	}
	// アニメーション任意回数再生
	ESLError BeginAnimation
		( unsigned long int nLoopCount = 1,
			unsigned long int nBeginFrame = 0,
			unsigned long int nAnimationTime = -1,
			unsigned long int nRewindSequence = 0,
			unsigned long int nTurnSequence = -1 )
		{	return	pfnBeginAnimation
				( this, nLoopCount, nBeginFrame,
					nAnimationTime, nRewindSequence, nTurnSequence ) ;	}
	// アニメーション停止
	ESLError EndAnimation( void )
		{	return	pfnEndAnimation( this ) ;	}
	// アニメーション中か？
	int IsDuringAnimation( void )
		{	return	pfnIsDuringAnimation( this ) ;	}
	// αチャネル設定
	ESLError SetAlphaImage
		( PEGL_IMAGE_INFO pBlendAlpha, unsigned int nAlphaRange )
		{	return	pfnSetAlphaImage( this, pBlendAlpha, nAlphaRange ) ;	}
	// 現在の合成マスクの度合いを取得
	unsigned int GetBlendDegree( void )
		{	return	pfnGetBlendDegree( this ) ;	}
	// 合成マスクの度合いを設定
	void SetBlendDegree( unsigned int nDegree )
		{	pfnSetBlendDegree( this, nDegree ) ;	}
	// フェード処理設定
	ESLError SetBlendingEnvelope( const double * bzEnvelope, int nCount )
		{	return	pfnSetBlendingEnvelope( this, bzEnvelope, nCount ) ;	}
	// 移動パラメータ設定
	ESLError SetBezierCurve
		( const E3D_VECTOR_2D * pbzCurve = NULL, int nCurveCount = 0,
			const double * pbzRev = NULL, int nRevCount = 0,
			const E3D_VECTOR_2D * pbzZoom = NULL, int nZoomCount = 0 )
		{	return	pfnSetBezierCurve
			( this, pbzCurve, nCurveCount, pbzRev, nRevCount, pbzZoom, nZoomCount ) ;	}
	// フェード処理・移動処理開始
	ESLError BeginActivation
		( const unsigned int nDurationTime[],
			int nDurationCount, int nActionType = actNormal )
		{	return	pfnBeginActivation
			( this, nDurationTime, nDurationCount, nActionType ) ;	}
	// フェード処理を即時完了させる
	ESLError FlushActivation( void )
		{	return	pfnFlushActivation( this ) ;	}
	// フェード処理をキャンセルする
	ESLError CancelActivation( void )
		{	return	pfnCancelActivation( this ) ;	}
	// フェード処理中か？
	int IsActivation( void )
		{	return	pfnIsActivation( this ) ;	}

} ;

struct	ECS_SPRITE_USER_INTERFACE
{
	// スプライト表示インターフェース
	EGL_RECT * (__stdcall *pfnGetRectangle)
		( ECS_SPRITE_USER_INTERFACE * instance, EGL_RECT * pRect ) ;
	bool (__stdcall *pfnGetHiddenRectangle)
		( ECS_SPRITE_USER_INTERFACE * instance, EGL_RECT * pHiddenRect ) ;
	void (__stdcall *pfnDraw)
		( ECS_SPRITE_USER_INTERFACE * instance, HEGL_RENDER_POLYGON hRenderPoly ) ;
	bool (__stdcall *pfnUpdateRect)
		( ECS_SPRITE_USER_INTERFACE * instance, EGL_RECT * pUpdateRect ) ;
	// スプライト共通インターフェース
	void (__stdcall *pfnEnable)
		( ECS_SPRITE_USER_INTERFACE * instance, bool fEnable ) ;
	bool (__stdcall *IsEnabled)( ECS_SPRITE_USER_INTERFACE * instance ) ;
	bool (__stdcall *pfnOnCommand)
		( ECS_SPRITE_USER_INTERFACE * instance,
			const wchar_t * pwszID, long int nNotification, long int nParameter ) ;
	long int (__stdcall *pfnSendCommand)
		( ECS_SPRITE_USER_INTERFACE * instance,
			const wchar_t * pwszParam, EWideString * pwstrResult ) ;
	// スプライト入力インターフェース
	bool (__stdcall *pfnIsHitSprite)
		( ECS_SPRITE_USER_INTERFACE * instance, int xPos, int yPos ) ;
	void (__stdcall *pfnOnMouseMove)
		( ECS_SPRITE_USER_INTERFACE * instance, UINT nFlags, int xPos, int yPos ) ;
	void (__stdcall *pfnOnMouseLeave)
		( ECS_SPRITE_USER_INTERFACE * instance, UINT nFlags, int xPos, int yPos ) ;
	bool (__stdcall *pfnOnSetCursor)
		( ECS_SPRITE_USER_INTERFACE * instance, int xPos, int yPos ) ;
	void (__stdcall *pfnOnMouseWheel)
		( ECS_SPRITE_USER_INTERFACE * instance,
			UINT nFlags, short int zDelta, int xPos, int yPos ) ;
	bool (__stdcall *pfnOnLButtonDown)
		( ECS_SPRITE_USER_INTERFACE * instance, UINT nFlags, int xPos, int yPos ) ;
	bool (__stdcall *pfnOnLButtonUp)
		( ECS_SPRITE_USER_INTERFACE * instance, UINT nFlags, int xPos, int yPos ) ;
	bool (__stdcall *pfnOnLButtonDblClk)
		( ECS_SPRITE_USER_INTERFACE * instance, UINT nFlags, int xPos, int yPos ) ;
	bool (__stdcall *pfnOnRButtonDown)
		( ECS_SPRITE_USER_INTERFACE * instance, UINT nFlags, int xPos, int yPos ) ;
	bool (__stdcall *pfnOnRButtonUp)
		( ECS_SPRITE_USER_INTERFACE * instance, UINT nFlags, int xPos, int yPos ) ;
	bool (__stdcall *pfnOnRButtonDblClk)
		( ECS_SPRITE_USER_INTERFACE * instance, UINT nFlags, int xPos, int yPos ) ;
	bool (__stdcall *pfnOnTimer)
		( ECS_SPRITE_USER_INTERFACE * instance, UINT nEventID ) ;
	bool (__stdcall *pfnMessageProc)
		( ECS_SPRITE_USER_INTERFACE * instance,
			HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam ) ;
	void (__stdcall *pfnOnSetFocus)( ECS_SPRITE_USER_INTERFACE * instance ) ;
	void (__stdcall *pfnOnKillFocus)( ECS_SPRITE_USER_INTERFACE * instance ) ;
	void (__stdcall *pfnSetCapture)( ECS_SPRITE_USER_INTERFACE * instance ) ;
	void (__stdcall *pfnReleaseCapture)( ECS_SPRITE_USER_INTERFACE * instance ) ;

	EGL_RECT * GetRectangle( EGL_RECT * pRect )
		{
			return	pfnGetRectangle( this, pRect ) ;
		}
	bool GetHiddenRectangle( EGL_RECT * pHiddenRect )
		{
			return	pfnGetHiddenRectangle( this, pHiddenRect ) ;
		}
	void Draw( HEGL_RENDER_POLYGON hRenderPoly )
		{
			pfnDraw( this, hRenderPoly ) ;
		}
	bool UpdateRect( EGL_RECT * pUpdateRect = NULL )
		{
			return	pfnUpdateRect( this, pUpdateRect ) ;
		}
} ;


//////////////////////////////////////////////////////////////////////////////
// ウィンドウインターフェース
//////////////////////////////////////////////////////////////////////////////

struct	ECS_WINDOW_INTERFACE
{
	class	WndCommand
	{
	public:
		ECS_OBJECT *	m_pID ;
		ECS_OBJECT *	m_pFullID ;
		long int		m_nNotification ;
		long int		m_nParameter ;
	public:
		WndCommand( void ) : m_pID(NULL), m_pFullID(NULL) { }
		~WndCommand( void )
			{
				if ( m_pID )	m_pID->Release( ) ;
				if ( m_pFullID )	m_pFullID->Release( ) ;
			}
	} ;
	// ウィンドウハンドル取得
	HWND (__stdcall *pfnGetWindow)( ECS_WINDOW_INTERFACE * instance ) ;
	// ウィンドウ生成
	ESLError (__stdcall *pfnCreateDisplay)
		( ECS_WINDOW_INTERFACE * instance, 
			const char * pszWindowName, int fCooperationLevel,
			unsigned int nWidth, unsigned int nHeight,
			unsigned int nBitsPerPixel, unsigned int nFrequency ) ;
	// ウィンドウを閉じる
	void (__stdcall *pfnCloseDisplay)( ECS_WINDOW_INTERFACE * instance ) ;
	// オプショナル機能フラグを取得する
	unsigned int (__stdcall *pfnGetOptionalFuncFlag)
						( ECS_WINDOW_INTERFACE * instance ) ;
	// オプショナル機能フラグを設定する
	void (__stdcall *pfnSetOptionalFuncFlag)
		( ECS_WINDOW_INTERFACE * instance, unsigned int nFlags ) ;
	// 協調レベルを変更
	ESLError (__stdcall *pfnChangeCooperationLevel)
		( ECS_WINDOW_INTERFACE * instance, int fCooperationLevel ) ;
	// ウィンドウサイズ変更
	ESLError (__stdcall *pfnChangeDisplaySize)
		( ECS_WINDOW_INTERFACE * instance,
			unsigned int nWidth, unsigned int nHeight,
			unsigned int nBitsPerPixel, unsigned int nFrequency ) ;
	// ウィンドウサイズ取得
	void (__stdcall *pfnGetDisplaySize)
		( ECS_WINDOW_INTERFACE * instance, SIZE * pDisplaySize ) ;
	// ウィンドウ更新
	void (__stdcall *pfnUpdateWindow)( ECS_WINDOW_INTERFACE * instance ) ;
	// ウィンドウはアクティブか？
	int (__stdcall *pfnIsWindowActive)( ECS_WINDOW_INTERFACE * instance ) ;
	// コマンド待ち行列を有効化する
	void (__stdcall *pfnEnableCommandQueue)
		( ECS_WINDOW_INTERFACE * instance, int fQueueCommand ) ;
	// コマンド待ち行列を初期化する
	void (__stdcall *pfnFlushCommandQueue)
		( ECS_WINDOW_INTERFACE * instance, int fQueueCommand ) ;
	// 待ち行列からコマンドを取得
	ESLError (__stdcall *pfnGetCommand)
		( ECS_WINDOW_INTERFACE * instance,
			WndCommand * pCmd, DWORD dwTimeout, int fRemove ) ;
	// コマンドを待ち行列に追加
	void (__stdcall *pfnQueueCommand)
		( ECS_WINDOW_INTERFACE * instance,
			const wchar_t * pwszID, long int nNotification, long int nParameter ) ;
	// マウス座標通知
	void (__stdcall *pfnCallMouseMove)( ECS_WINDOW_INTERFACE * instance ) ;
	// スレッド排他処理
	ESLError (__stdcall *pfnLock)
		( ECS_WINDOW_INTERFACE * instance, DWORD dwTimeout ) ;
	void (__stdcall *pfnUnlock)( ECS_WINDOW_INTERFACE * instance ) ;
	// スレッド同期描画処理
	ESLError (__stdcall *pfnSyncTimePaint)
		( ECS_WINDOW_INTERFACE * instance, DWORD dwTimeout ) ;
	void (__stdcall *pfnAsyncTimePaint)( ECS_WINDOW_INTERFACE * instance ) ;
	// カーソル表示設定
	void (__stdcall *pfnShowCursor)
		( ECS_WINDOW_INTERFACE * instance, int fShow ) ;
	// カーソル表示状態取得
	int (__stdcall *pfnIsShowCursor)( ECS_WINDOW_INTERFACE * instance ) ;
	// ウィンドウスレッドから関数を呼び出す
	typedef	LRESULT (__stdcall *PFUNC_PROCEDURE)( void * pInstance ) ;
	ESLError (__stdcall *pfnProcedureOnWindowThread)
		( ECS_WINDOW_INTERFACE * instance,
			PFUNC_PROCEDURE pfnProc,
				void * pInstance, LRESULT * pResult, int fAsync ) ;

	HWND GetWindow( void )
		{	return	pfnGetWindow( this ) ;	}
	ESLError CreateDisplay
		( const char * pszWindowName, int fCooperationLevel,
			unsigned int nWidth, unsigned int nHeight,
			unsigned int nBitsPerPixel = 0, unsigned int nFrequency = 0 )
		{
			return	pfnCreateDisplay
				( this, pszWindowName, fCooperationLevel,
					nWidth, nHeight, nBitsPerPixel, nFrequency ) ;
		}
	void CloseDisplay( void )
		{	pfnCloseDisplay( this ) ;	}
	unsigned int GetOptionalFuncFlag( void )
		{	return	pfnGetOptionalFuncFlag( this ) ;	}
	void SetOptionalFuncFlag( unsigned int nFlags )
		{	pfnSetOptionalFuncFlag( this, nFlags ) ;	}
	ESLError ChangeCooperationLevel( int fCooperationLevel )
		{	return	pfnChangeCooperationLevel( this, fCooperationLevel ) ;	}
	ESLError ChangeDisplaySize
		( unsigned int nWidth, unsigned int nHeight,
			unsigned int nBitsPerPixel = 0, unsigned int nFrequency = 0 )
		{
			return	pfnChangeDisplaySize
				( this, nWidth, nHeight, nBitsPerPixel, nFrequency ) ;
		}
	void GetDisplaySize( SIZE * pDisplaySize )
		{	pfnGetDisplaySize( this, pDisplaySize ) ;	}
	void UpdateWindow( void )
		{	pfnUpdateWindow( this ) ;	}
	int IsWindowActive( void )
		{	return	pfnIsWindowActive( this ) ;	}
	void EnableCommandQueue( int fQueueCommand )
		{	pfnEnableCommandQueue( this, fQueueCommand ) ;	}
	void FlushCommandQueue( int fQueueCommand )
		{	pfnFlushCommandQueue( this, fQueueCommand ) ;	}
	ESLError GetCommand
		( WndCommand & wcmd, DWORD dwTimeout, bool fRemove = true )
		{	return	pfnGetCommand( this, &wcmd, dwTimeout, fRemove ) ;	}
	void QueueCommand
		( const wchar_t * pwszID, long int nNotification, long int nParameter )
		{	pfnQueueCommand( this, pwszID, nNotification, nParameter ) ;	}
	void CallMouseMove( void )
		{	pfnCallMouseMove( this ) ;	}
	ESLError Lock( DWORD dwTimeout = INFINITE )
		{	return	pfnLock( this, dwTimeout ) ;	}
	void Unlock( void )
		{	pfnUnlock( this ) ;	}
	ESLError SyncTimePaint( DWORD dwTimeout )
		{	return	pfnSyncTimePaint( this, dwTimeout ) ;	}
	void AsyncTimePaint( void )
		{	pfnAsyncTimePaint( this ) ;	}
	void ShowCursor( int fShow )
		{	pfnShowCursor( this, fShow ) ;	}
	int IsShowCursor( void )
		{	return	pfnIsShowCursor( this ) ;	}
	ESLError ProcedureOnWindowThread
		( PFUNC_PROCEDURE pfnProc,
			void * pInstance, LRESULT * pResult, bool fAsync )
		{
			return	pfnProcedureOnWindowThread
					( this, pfnProc, pInstance, pResult, fAsync ) ;
		}

} ;


//////////////////////////////////////////////////////////////////////////////
// 入力フィルタインターフェース
//////////////////////////////////////////////////////////////////////////////

struct	ECS_INPUT_FILTER_INTERFACE
{
	enum	InputDevice
	{
		idKeyboard,	idMouse,	idJoyStick,	idCommand
	} ;
	enum	JoyButton
	{
		jbUp,	jbDown,	jbLeft,	jbRight,
		jbButton1,	jbButton2,	jbButton3,	jbButton4,
		jbButtonMax	= jbButton1 + 32
	} ;
	struct	INPUT_EVENT
	{
		InputDevice	idType ;		// デバイスの種類
		int			iDevNum ;		// デバイスの番号（ジョイスティック）
		int			iKeyNum ;		// 仮想キーコード／ジョイボタン番号
	} ;
	ESLError (__stdcall *pfnLoadInputFilter)
		( ECS_INPUT_FILTER_INTERFACE * instance,
			const wchar_t * pwszFileName, ECS_CONTEXT * pContext ) ;
	void (__stdcall *pfnDeleteInputFilter)
		( ECS_INPUT_FILTER_INTERFACE * instance ) ;
	ESLError (__stdcall *pfnOpenFilter)
		( ECS_INPUT_FILTER_INTERFACE * instance,
						int nType, ECS_OBJECT * pWnd ) ;
	ESLError (__stdcall *pfnCloseFilter)
		( ECS_INPUT_FILTER_INTERFACE * instance ) ;
	ESLError (__stdcall *pfnGetInputEvent)
		( ECS_INPUT_FILTER_INTERFACE * instance,
			INPUT_EVENT & ieEvent, DWORD dwTimeout ) ;
	ESLError (__stdcall *pfnFlushInputQueue)
		( ECS_INPUT_FILTER_INTERFACE * instance, int nLimit ) ;
	DWORD (__stdcall *pfnGetCapturedJoyStick)
		( ECS_INPUT_FILTER_INTERFACE * instance ) ;
	ESLError (__stdcall *pfnGetStickPosition)
		( ECS_INPUT_FILTER_INTERFACE * instance,
					E3D_VECTOR & vPos, int iDevNum ) ;
	int (__stdcall *pfnIsJoyButtonPushing)
		( ECS_INPUT_FILTER_INTERFACE * instance,
						int iKeyNum, int iDevNum ) ;
	int (__stdcall *pfnGetJoyButtonPushed)
		( ECS_INPUT_FILTER_INTERFACE * instance,
						int iKeyNum, int iDevNum ) ;
	ESLError (__stdcall *pfnFlushJoyButtonPushed)
		( ECS_INPUT_FILTER_INTERFACE * instance, int iDevNum, int iKeyNum ) ;
	void (__stdcall *pfnGetCursorPos)
		( ECS_INPUT_FILTER_INTERFACE * instance, EGL_POINT & ptCursor ) ;
	void (__stdcall *pfnMoveCursorPos)
		( ECS_INPUT_FILTER_INTERFACE * instance,
						long int xPos, long int yPos ) ;

	// フィルタファイル読み込み
	ESLError LoadInputFilter
		( const wchar_t * pwszFileName, ECS_CONTEXT * pContext = NULL )
		{	return	pfnLoadInputFilter( this, pwszFileName, pContext ) ;	}
	// フィルタの内容を初期化する
	void DeleteInputFilter( void )
		{	pfnDeleteInputFilter( this ) ;	}
	// フィルター処理開始
	ESLError OpenFilter( int nType, ECS_OBJECT * pWnd )
		{	return	pfnOpenFilter( this, nType, pWnd ) ;	}
	// フィルター処理終了
	ESLError CloseFilter( void )
		{	return	pfnCloseFilter( this ) ;	}
	// 入力イベントを取得
	ESLError GetInputEvent
		( INPUT_EVENT & ieEvent, DWORD dwTimeout = INFINITE )
		{	return	pfnGetInputEvent( this, ieEvent, dwTimeout ) ;	}
	// 入力イベント待ち行列を初期化
	ESLError FlushInputQueue( int nLimit )
		{	return	pfnFlushInputQueue( this, nLimit ) ;	}
	// 現在使用中のジョイスティックを取得
	DWORD GetCapturedJoyStick( void )
		{	return	pfnGetCapturedJoyStick( this ) ;	}
	// 現在のスティック座標を取得
	ESLError GetStickPosition( E3D_VECTOR & vPos, int iDevNum = 0 )
		{	return	pfnGetStickPosition( this, vPos, iDevNum ) ;	}
	// 仮想ジョイスティックのボタンの現在の状態を取得
	int IsJoyButtonPushing( int iKeyNum, int iDevNum = 0 )
		{	return	pfnIsJoyButtonPushing( this, iKeyNum, iDevNum ) ;	}
	// 仮想ジョイスティックのボタンが押された回数を取得
	int GetJoyButtonPushed( int iKeyNum, int iDevNum = 0 )
		{	return	pfnGetJoyButtonPushed( this, iKeyNum, iDevNum ) ;	}
	// 仮想ジョイスティックのボタンの押下回数をリセット
	ESLError FlushJoyButtonPushed( int iDevNum = 0, int iKeyNum = -1 )
		{	return	pfnFlushJoyButtonPushed( this, iDevNum, iKeyNum ) ;	}
	// マウスカーソル座標取得
	void GetCursorPos( EGL_POINT & ptCursor )
		{	pfnGetCursorPos( this, ptCursor ) ;	}
	// マウスカーソル座標移動
	void MoveCursorPos( long int xPos, long int yPos )
		{	pfnMoveCursorPos( this, xPos, yPos ) ;	}

} ;


//////////////////////////////////////////////////////////////////////////////
// ファイルオブジェクトインターフェース
//////////////////////////////////////////////////////////////////////////////

struct	ECS_FILE_INTERFACE
{
	ESLError (__stdcall *pfnOpen)
		( ECS_FILE_INTERFACE * instance,
			const wchar_t * pwszFileName,
			DWORD dwOpenFlags, ECS_CONTEXT * pContext ) ;
	ESLError (__stdcall *pfnClose)( ECS_FILE_INTERFACE * instance ) ;
	ESLError (__stdcall *pfnSetCharacterEncoding)
		( ECS_FILE_INTERFACE * instance, const char * pszType ) ;
	const char * (__stdcall *pfnGetCharacterEncoding)
		( ECS_FILE_INTERFACE * instance ) ;
	unsigned long int (__stdcall *pfnGetFileLength)
		( ECS_FILE_INTERFACE * instance ) ;
	unsigned long int (__stdcall *pfnGetFilePosition)
		( ECS_FILE_INTERFACE * instance ) ;
	unsigned long int (__stdcall *pfnSeek)
		( ECS_FILE_INTERFACE * instance, long int nPos, int nSeekType ) ;
	int (__stdcall *pfnIsEndOfFile)( ECS_FILE_INTERFACE * instance ) ;
	void (__stdcall *pfnSetEndOfFile)( ECS_FILE_INTERFACE * instance ) ;
	ESLError (__stdcall *pfnReadText)
		( ECS_FILE_INTERFACE * instance, ECS_OBJECT * pObj ) ;
	unsigned long int (__stdcall *pfnWriteText)
		( ECS_FILE_INTERFACE * instance, ECS_OBJECT * pObj ) ;
	unsigned long int (__stdcall *pfnReadBinary)
		( ECS_FILE_INTERFACE * instance, ECS_OBJECT * pObj, long int nBytes ) ;
	unsigned long int (__stdcall *pfnWriteBinary)
		( ECS_FILE_INTERFACE * instance, ECS_OBJECT * pObj, long int nBytes ) ;
	ESLError (__stdcall *pfnLoadContextTitle)
		( ECS_FILE_INTERFACE * instance,
			ECS_OBJECT **pObj, ECS_CONTEXT * pContext ) ;
	ESLError (__stdcall *pfnLoadObject)
		( ECS_FILE_INTERFACE * instance,
			ECS_OBJECT **pObj, ECS_CONTEXT * pContext ) ;
	ESLError (__stdcall *pfnLoadContext)
		( ECS_FILE_INTERFACE * instance, ECS_CONTEXT * pContext ) ;
	ESLError (__stdcall *pfnSaveThumbnailImage)
		( ECS_FILE_INTERFACE * instance,
			ECS_OBJECT * pPreview, int nWidth, int nHeight ) ;
	ESLError (__stdcall *pfnSaveObject)
		( ECS_FILE_INTERFACE * instance,
			ECS_OBJECT * pObj, ECS_OBJECT * pTitle, ECS_CONTEXT * pContext ) ;
	ESLError (__stdcall *pfnSaveContext)
		( ECS_FILE_INTERFACE * instance,
			ECS_OBJECT * pTitle, ECS_CONTEXT * pContext ) ;
	ESLError (__stdcall *pfnDumpObject)
		( ECS_FILE_INTERFACE * instance,
			ECS_OBJECT * pObj, ECS_CONTEXT * pContext ) ;
	ESLError (__stdcall *pfnDumpContext)
		( ECS_FILE_INTERFACE * instance, ECS_CONTEXT * pContext ) ;
	ECS_FILE * (__stdcall *pfnGetFile)( ECS_FILE_INTERFACE * instance ) ;

	// ファイルを開く
	ESLError Open
		( const wchar_t * pwszFileName,
			DWORD dwOpenFlags, ECS_CONTEXT * pContext = NULL )
		{	return	pfnOpen( this, pwszFileName, dwOpenFlags, pContext ) ;	}
	// ファイルを閉じる
	ESLError Close( void )
		{	return	pfnClose( this ) ;	}
	// 文字エンコーディングを設定する
	ESLError SetCharacterEncoding( const char * pszType )
		{	return	pfnSetCharacterEncoding( this, pszType ) ;	}
	// 文字エンコーディングを取得する
	const char * GetCharacterEncoding( void )
		{	return	pfnGetCharacterEncoding( this ) ;	}
	// ファイル長取得
	unsigned long int GetFileLength( void )
		{	return	pfnGetFileLength( this ) ;	}
	// ファイルポインタ取得
	unsigned long int GetFilePosition( void )
		{	return	pfnGetFilePosition( this ) ;	}
	// ファイルポインタ移動
	unsigned long int Seek( long int nPos, int nSeekType )
		{	return	pfnSeek( this, nPos, nSeekType ) ;	}
	// EOF 判定
	bool IsEndOfFile( void )
		{	return	pfnIsEndOfFile( this ) != 0 ;	}
	// EOF 設定
	void SetEndOfFile( void )
		{	pfnSetEndOfFile( this ) ;	}
	// 文字列の読み込み
	ESLError ReadText( ECS_OBJECT * pObj )
		{	return	pfnReadText( this, pObj ) ;	}
	// 文字列の書き出し
	unsigned long int WriteText( ECS_OBJECT * pObj )
		{	return	pfnWriteText( this, pObj ) ;	}
	// バイナリの読み込み
	unsigned long int ReadBinary( ECS_OBJECT * pObj, long int nBytes )
		{	return	pfnReadBinary( this, pObj, nBytes ) ;	}
	// バイナリの書き出し
	unsigned long int WriteBinary( ECS_OBJECT * pObj, long int nBytes )
		{	return	pfnWriteBinary( this, pObj, nBytes ) ;	}
	// セーブファイル見出しの読み込み
	ESLError LoadContextTitle( ECS_OBJECT *&pObj, ECS_CONTEXT * pContext )
		{	return	pfnLoadContextTitle( this, &pObj, pContext ) ;	}
	// オブジェクトの読み込み
	ESLError LoadObject( ECS_OBJECT *&pObj, ECS_CONTEXT * pContext )
		{	return	pfnLoadObject( this, &pObj, pContext ) ;	}
	// コンテキストの読み込み
	ESLError LoadContext( ECS_CONTEXT * pContext )
		{	return	pfnLoadContext( this, pContext ) ;	}
	// サーブファイルサムネイル画像の書き出し
	ESLError SaveThumbnailImage
		( ECS_OBJECT * pPreview, int nWidth, int nHeight )
		{	return	pfnSaveThumbnailImage( this, pPreview, nWidth, nHeight ) ;	}
	// オブジェクトの書き出し
	ESLError SaveObject
		( ECS_OBJECT * pObj, ECS_OBJECT * pTitle, ECS_CONTEXT * pContext )
		{	return	pfnSaveObject( this, pObj, pTitle, pContext ) ;	}
	// コンテキストの読み込み
	ESLError SaveContext
		( ECS_OBJECT * pTitle, ECS_CONTEXT * pContext )
		{	return	pfnSaveContext( this, pTitle, pContext ) ;	}
	// オブジェクトをダンプする
	ESLError DumpObject( ECS_OBJECT * pObj, ECS_CONTEXT * pContext )
		{	return	pfnDumpObject( this, pObj, pContext ) ;	}
	// コンテキストをダンプする
	ESLError DumpContext( ECS_CONTEXT * pContext )
		{	return	pfnDumpContext( this, pContext ) ;	}
	// 内部ファイルへのインターフェースを取得
	ECS_FILE * GetFile( void )
		{	return	pfnGetFile( this ) ;	}

} ;


//////////////////////////////////////////////////////////////////////////////
// プラグイン固有オブジェクト
//////////////////////////////////////////////////////////////////////////////

class	ECSPISubClass	: public	ESLObject
{
public:
	ECS_OBJECT *	m_instance ;
	ECS_OBJECT		m_absobj ;
	ECS_OBJECT *	m_baseobj ;

public:
	// 構築関数
	ECSPISubClass( ECS_OBJECT * absobj = NULL, ECS_OBJECT * baseobj = NULL )
			: m_instance( NULL ), m_baseobj( NULL )
		{
			SetObjectInstance( absobj, baseobj ) ;
		}
	// 消滅関数
	virtual ~ECSPISubClass( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( ECSPISubClass, ESLObject )

public:
	// オブジェクトインスタンスを設定（オーバーライド）
	virtual void SetObjectInstance
		( ECS_OBJECT * absobj, ECS_OBJECT * baseobj = NULL ) ;

public:
	// オーバーライド可能な関数
	virtual void Release( void ) ;
	virtual void * QueryInterface( const wchar_t * pwszType ) ;
	virtual const wchar_t * GetTypeName( void ) ;
	virtual ECS_OBJECT * GetTypeOf( const wchar_t * pwszTypeName ) ;
	virtual ECS_OBJECT * Duplicate( void ) ;
	virtual ESLError Move( ECS_CONTEXT * context, const ECS_OBJECT * obj ) ;
	virtual ESLError UnaryOperate
		( ECS_CONTEXT * context, CSUnaryOperatorType csuopType ) ;
	virtual ESLError Operate
		( ECS_CONTEXT * context, CSOperatorType csopType, ECS_OBJECT * obj ) ;
	virtual ESLError Compare
		( ECS_CONTEXT * context, int * pResult,
			CSCompareType cscpType, ECS_OBJECT * obj ) ;
	virtual ESLError GetVariableIndex( int * pIndex, int iElement ) ;
	virtual ESLError GetVariableIndex( int * pIndex, const wchar_t * pwszElement ) ;
	virtual ECS_OBJECT * GetVariableAt( int nIndex ) ;
	virtual ECS_OBJECT * SetVariableAt( int nIndex, ECS_OBJECT * obj ) ;
	virtual ESLError GetFunction
		( ECS_CONTEXT * context, int * pIndex, const wchar_t * pwszName ) ;
	virtual ESLError CallFunction
		( ECS_CONTEXT * context,
			int nIndex, ECS_OBJECT * const* pArg, int nArgCount ) ;
	virtual void IndexAllMember( void ) ;
	virtual void CleanupAllReference( ECS_CONTEXT * context ) ;
	virtual ESLError CommitAllReference( ECS_CONTEXT * context ) ;
	virtual ESLError Save( ECS_FILE * pfile, ECS_CONTEXT * context ) ;
	virtual ESLError Load( ECS_FILE * pfile, ECS_CONTEXT * context ) ;
	virtual ESLError DumpObject
		( ECS_FILE * pfile, int nIndent, ECS_CONTEXT * context ) ;

protected:
	// オーバーライド関数
	static void __stdcall PI_Release( ECS_OBJECT * instance ) ;
	static void * __stdcall PI_QueryInterface
		( ECS_OBJECT * instance, const wchar_t * pwszType ) ;
	static const wchar_t * __stdcall PI_GetTypeName( ECS_OBJECT * instance ) ;
	static ECS_OBJECT * __stdcall PI_GetTypeOf
		( ECS_OBJECT * instance, const wchar_t * pwszTypeName ) ;
	static ECS_OBJECT * __stdcall PI_Duplicate( ECS_OBJECT * instance ) ;
	static ESLError __stdcall PI_Move
		( ECS_OBJECT * instance,
			ECS_CONTEXT * context, const ECS_OBJECT * obj ) ;
	static ESLError __stdcall PI_UnaryOperate
		( ECS_OBJECT * instance, ECS_CONTEXT * context,
			CSUnaryOperatorType csuopType ) ;
	static ESLError __stdcall PI_Operate
		( ECS_OBJECT * instance, ECS_CONTEXT * context,
			CSOperatorType csopType, ECS_OBJECT * obj ) ;
	static ESLError __stdcall PI_Compare
		( ECS_OBJECT * instance, ECS_CONTEXT * context,
			int * pResult, CSCompareType cscpType, ECS_OBJECT * obj ) ;
	static ESLError __stdcall PI_GetVariableIndex
		( ECS_OBJECT * instance, int * pIndex, int iElement ) ;
	static ESLError __stdcall PI_GetVariableIndex
		( ECS_OBJECT * instance, int * pIndex, const wchar_t * pwszElement ) ;
	static ECS_OBJECT * __stdcall PI_GetVariableAt
		( ECS_OBJECT * instance, int nIndex ) ;
	static ECS_OBJECT * __stdcall PI_SetVariableAt
		( ECS_OBJECT * instance, int nIndex, ECS_OBJECT * obj ) ;
	static ESLError __stdcall PI_GetFunction
		( ECS_OBJECT * instance, ECS_CONTEXT * context,
			int * pIndex, const wchar_t * pwszName ) ;
	static ESLError __stdcall PI_CallFunction
		( ECS_OBJECT * instance, ECS_CONTEXT * context,
			int nIndex, ECS_OBJECT * const* pArg, int nArgCount ) ;
	static void __stdcall PI_IndexAllMember( ECS_OBJECT * instance ) ;
	static void __stdcall PI_CleanupAllReference
		( ECS_OBJECT * instance, ECS_CONTEXT * context ) ;
	static ESLError __stdcall PI_CommitAllReference
		( ECS_OBJECT * instance, ECS_CONTEXT * context ) ;
	static ESLError __stdcall PI_Save
		( ECS_OBJECT * instance,
			ECS_FILE * pfile, ECS_CONTEXT * context ) ;
	static ESLError __stdcall PI_Load
		( ECS_OBJECT * instance,
			ECS_FILE * pfile, ECS_CONTEXT * context ) ;
	static ESLError __stdcall PI_DumpObject
		( ECS_OBJECT * instance, ECS_FILE * pfile,
			int nIndent, ECS_CONTEXT * context ) ;

} ;


#endif
