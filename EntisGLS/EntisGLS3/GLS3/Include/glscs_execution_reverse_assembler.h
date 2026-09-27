
//////////////////////////////////////////////////////////////////////////////
// Entis Cotopha Script 実行イメージ・逆アセンブラ
//////////////////////////////////////////////////////////////////////////////

class	ECSExecutionReverseAssembler	: public ESLObject
{
public:
	// 構築関数
	ECSExecutionReverseAssembler( ECSExecutionImage * pcsxi ) ;
	// 消滅関数
	virtual ~ECSExecutionReverseAssembler( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( ECSExecutionReverseAssembler, ESLObject )

public:
	// 命令情報
	struct	InstructionInfo	: public ECSSakura2Processor::InstructionInfo
	{
		int		nAddress ;
		EString	strMnemonic ;
		EString	strOperand ;
	} ;

protected:
	ECSExecutionImage *	m_pcsxi ;

	typedef	void (ECSExecutionReverseAssembler::*PFUNC_REVERSE_ASSEMBLE)
			( InstructionInfo & inf,
				const BYTE * pbytCode, int addrCode, bool fMnemonic ) ;
	static const PFUNC_REVERSE_ASSEMBLE	m_pfnReverseAssemble[csicMax] ;

public:
	// 逆アセンブル
	void ReverseAssemble
		( InstructionInfo & inf,
			const BYTE * pbytCode, int addrCode,  bool fMnemonic ) ;

protected:
	// 記憶クラス文字列変換
	static const char * GetMemoryClassName( CSObjectMode csomType ) ;
	// 型名文字列変換
	static const char * GetVariableTypeName( CSVariableType csvtType ) ;
	// 演算文字列変換
	static const char * GetOperatorTypeName( CSOperatorType csotType ) ;
	static const char * GetUniOperatorTypeName( CSUnaryOperatorType csuotType ) ;
	static const char * GetCompareTypeName( CSCompareType csctType ) ;
	// クラス名取得
	const wchar_t * GetClassName( const BYTE * pImage, int& ip ) ;
	const wchar_t * GetClassNameFromIndex( DWORD dwClassIndex ) ;
	// 文字列リテラル取得
	EWideString GetStringLiteral( const BYTE * pImage, int& ip ) ;
	// 関数名取得
	EWideString GetFunctionName( DWORD dwFuncAddr ) ;

protected:
	/* cotopha 1.0 */
	void ReverseAssembleNew( InstructionInfo & inf, const BYTE  * pbytCode, int addrCode, bool fMnemonic ) ;
	void ReverseAssembleFree( InstructionInfo & inf, const BYTE  * pbytCode, int addrCode, bool fMnemonic ) ;
	void ReverseAssembleLoad( InstructionInfo & inf, const BYTE  * pbytCode, int addrCode, bool fMnemonic ) ;
	void ReverseAssembleStore( InstructionInfo & inf, const BYTE  * pbytCode, int addrCode, bool fMnemonic ) ;
	void ReverseAssembleEnter( InstructionInfo & inf, const BYTE  * pbytCode, int addrCode, bool fMnemonic ) ;
	void ReverseAssembleLeave( InstructionInfo & inf, const BYTE  * pbytCode, int addrCode, bool fMnemonic ) ;
	void ReverseAssembleJump( InstructionInfo & inf, const BYTE  * pbytCode, int addrCode, bool fMnemonic ) ;
	void ReverseAssembleCJump( InstructionInfo & inf, const BYTE  * pbytCode, int addrCode, bool fMnemonic ) ;
	void ReverseAssembleCall( InstructionInfo & inf, const BYTE  * pbytCode, int addrCode, bool fMnemonic ) ;
	void ReverseAssembleReturn( InstructionInfo & inf, const BYTE  * pbytCode, int addrCode, bool fMnemonic ) ;
	void ReverseAssembleElement( InstructionInfo & inf, const BYTE  * pbytCode, int addrCode, bool fMnemonic ) ;
	void ReverseAssembleElementIndirect( InstructionInfo & inf, const BYTE  * pbytCode, int addrCode, bool fMnemonic ) ;
	void ReverseAssembleOperate( InstructionInfo & inf, const BYTE  * pbytCode, int addrCode, bool fMnemonic ) ;
	void ReverseAssembleUniOperate( InstructionInfo & inf, const BYTE  * pbytCode, int addrCode, bool fMnemonic ) ;
	void ReverseAssembleCompare( InstructionInfo & inf, const BYTE  * pbytCode, int addrCode, bool fMnemonic ) ;
	/* extended 2.0 */
	void ReverseAssembleExOperate( InstructionInfo & inf, const BYTE  * pbytCode, int addrCode, bool fMnemonic ) ;
	void ReverseAssembleExUniOperate( InstructionInfo & inf, const BYTE  * pbytCode, int addrCode, bool fMnemonic ) ;
	void ReverseAssembleExCall( InstructionInfo & inf, const BYTE  * pbytCode, int addrCode, bool fMnemonic ) ;
	void ReverseAssembleExReturn( InstructionInfo & inf, const BYTE  * pbytCode, int addrCode, bool fMnemonic ) ;
	void ReverseAssembleCallMember( InstructionInfo & inf, const BYTE  * pbytCode, int addrCode, bool fMnemonic ) ;
	void ReverseAssembleCallNativeMember( InstructionInfo & inf, const BYTE  * pbytCode, int addrCode, bool fMnemonic ) ;
	void ReverseAssembleSwap( InstructionInfo & inf, const BYTE  * pbytCode, int addrCode, bool fMnemonic ) ;
	/* extended 2.3 */
	void ReverseAssembleCreateBuffer( InstructionInfo & inf, const BYTE  * pbytCode, int addrCode, bool fMnemonic ) ;
	void ReverseAssembleCreateBufferVSize( InstructionInfo & inf, const BYTE  * pbytCode, int addrCode, bool fMnemonic ) ;
	void ReverseAssemblePointerToObject( InstructionInfo & inf, const BYTE  * pbytCode, int addrCode, bool fMnemonic ) ;
	void ReverseAssemblePointerToAddress( InstructionInfo & inf, const BYTE  * pbytCode, int addrCode, bool fMnemonic ) ;
	void ReverseAssembleReferenceForPointer( InstructionInfo & inf, const BYTE  * pbytCode, int addrCode, bool fMnemonic ) ;
	void ReverseAssembleReferenceForObjPointer( InstructionInfo & inf, const BYTE  * pbytCode, int addrCode, bool fMnemonic ) ;
	void ReverseAssembleCallFunctionPointer( InstructionInfo & inf, const BYTE  * pbytCode, int addrCode, bool fMnemonic ) ;
	void ReverseAssembleCallNativeFunction( InstructionInfo & inf, const BYTE  * pbytCode, int addrCode, bool fMnemonic ) ;

} ;

