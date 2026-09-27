
//////////////////////////////////////////////////////////////////////////////
// Entis Cotopha Script 実行イメージ・コンパイラ
//////////////////////////////////////////////////////////////////////////////

class	ECSExecutionImageCompiler	: public	ECSExecutionImageLinker
{
public:
	// 構築関数
	ECSExecutionImageCompiler
		( ECSExecutionImageCompiler * pcsxiMaster = NULL ) ;
	// 消滅関数
	virtual ~ECSExecutionImageCompiler( void ) ;
	// クラス情報
	DECLARE_CLASS_INFO( ECSExecutionImageCompiler, ECSExecutionImageLinker )

protected:
	ECSExecutionImageCompiler *		m_pcsxiMaster ;

	// コンパイル時の最終出力命令情報
	DWORD				m_dwLastInstructionPos ;
	ECSSakura2Processor::InstructionCode
						m_icLastInstruction ;

	// m_icLastInstruction == codeLoadImm64 の時
	bool				m_flagLastImmediateReal ;

	// reg, reg 形式の場合
	int					m_regLastDst, m_regLastSrc ;

	// imm32 パラメータ
	int					m_immLastSrc32 ;

public:
	// レジスタ割り当て情報
	struct	LocalAssignRegister
	{
		bool	flagLoaded ;		// 使用中フラグ
		bool	flagUnloaded ;		// 要ロード
		bool	flagModified ;		// 要ライトバック
		int		countLocked ;		// ロックフラグ（割り当て解放不可）
		int		lastAccess ;		// 他のレジスタにアクセスした回数
		int		offsetLocal ;		// 割り当てられたローカル変数アドレス
		ECSSakura2Processor::DataType
				typeLocal ;			// 割り当てられたローカル変数の型

		LocalAssignRegister( void )
		{
			flagLoaded = false ;
			flagUnloaded = false ;
			flagModified = false ;
			countLocked = 0 ;
			lastAccess = 0 ;
			offsetLocal = 0 ;
			typeLocal = ECSSakura2Processor::dataInt64 ;
		}
	} ;
	enum	Register
	{
		regAssignMax = 8,
	} ;
	// レジスタ割り当てコンテキスト
	struct	RegisterContext
	{
		LocalAssignRegister	regAssigns[regAssignMax] ;
	public:
		// レジスタのメモリへの書き出しが必要
		bool IsModifiedRegister( void ) const ;
		// マージ可能か？（レジスタ割り当てが一致しているか？）
		bool IsMergableContextFrom( const RegisterContext & context ) ;
		// マージ（レジスタの変更フラグを優先して複製）
		void MergeContextFrom( const RegisterContext & context ) ;
	} ;

protected:
	LocalAssignRegister	m_regAssigns[regAssignMax] ;

public:
	// 実行イメージを消去
	virtual void DeleteImage( void ) ;
	// クラスを検索
	virtual int GetClassInfoIndex( const wchar_t * pwszClassName ) const ;

public:
	// 最後に出力した命令コードをリセットする
	void FenceInstruction( void ) ;
	// 最後に出力した命令コードを取得
	ECSSakura2Processor::InstructionCode GetLastWrittenInstruction( void ) const
		{
			return	m_icLastInstruction ;
		}
	// 最後に出力した命令コードを無効化
	void FlushLastWrittenInstruction( void )
		{
			m_icLastInstruction = ECSSakura2Processor::codeInvalid ;
		}

public:
	// レジスタ割り当てを初期化
	void InitializeAllRegisterAssigns( void ) ;
	// レジスタ割り当てを解除
	void ResetAllRegisterAssigns( void ) ;
	void ResetLocalBoundsAssignedRegisters( int offsetFirst, int offsetEnd ) ;
	// unloaded 状態のレジスタを再ロード
	void ReloadAllRegisterAssigns( void ) ;
	// レジスタへの変更をメモリに反映させる
	void FlushAllRegisterAssigns( void ) ;
	void FlushLocalBoundsAssignedRegisters( int offsetFirst, int offsetEnd ) ;
	void FlushRegisterAssign( int regNum ) ;
	// レジスタ・コンテキスト取得
	void GetRegisterContext( RegisterContext & context ) ;
	// レジスタ・コンテキスト復元
	void RestoreRegisterContext( const RegisterContext & context ) ;
	// レジスタ・コンテキストをマージ可能か？
	bool IsMergableContextTo( const RegisterContext & context ) const ;
	// レジスタ・コンテキストをマージするためのメモリ処理
	bool FlushMergableContextTo( RegisterContext & context ) ;

public:
	// 割り当てられたレジスタを取得
	int FindAssignedRegister
		( int offseLocal, ECSSakura2Processor::DataType typeLocal ) const ;
	// レジスタにローカル変数が割り当てられているか？
	bool IsRegisterAssigned( int regNum ) const ;
	// レジスタの割り当てローカルアドレスを取得
	int GetAssignedLocalOffset( int regNum ) const ;
	// レジスタにローカル変数を割り当てる
	//（必要であれば古いレジスタはライトバックする）
	int AssignLocalToRegister
		( int offsetLocal, ECSSakura2Processor::DataType typeLocal ) ;
	// レジスタのアクセス履歴を更新する
	void AccessAssignedRegister( int regNum ) ;
	// レジスタの割り当てを解放する（ライトバックはしない）
	void FreeAssignLocalToRegister
		( int offsetLocal, ECSSakura2Processor::DataType typeLocal ) ;
	// レジスタの割り当てをロックする
	void LockAssignedRegister( int regNum ) ;
	// レジスタの割り当てのロックを1回アンロックする
	void UnlockAssignedRegister( int regNum ) ;

public:
	//////////////////////////////////////////////////////////////////////////
	// 仮想マシン用コード出力
	//////////////////////////////////////////////////////////////////////////
	// １バイト書き出し
	void WriteByteCode( BYTE nCode ) ;
	void WriteInstructionCode( CSInstructionCode icode ) ;
	void WriteObjectModeCode( CSObjectMode objmode )
		{	WriteByteCode( objmode ) ;	}
	void WriteVariableTypeCode( CSVariableType vartype )
		{	WriteByteCode( vartype ) ;	}
	void WriteOperatorTypeCode( CSOperatorType optype )
		{	WriteByteCode( optype ) ;	}
	void WriteUniOperatorTypeCode( CSUnaryOperatorType uoptype )
		{	WriteByteCode( uoptype ) ;	}
	void WriteCompareTypeCode( CSCompareType cmptype )
		{	WriteByteCode( cmptype ) ;	}
	void WriteExOperatorTypeCode( CSExtraOperatorType xoptype )
		{	WriteByteCode( xoptype ) ;	}
	void WriteExUniOperatorTypeCode( CSExtraUniOperatorType xuoptype )
		{	WriteByteCode( xuoptype ) ;	}
	// 文字列書き出し
	void WriteConstantString( const ECSWideString & wstrData ) ;
	// クラスインデックス書き出し
	void WriteClassIndex( DWORD dwClassIndex ) ;
	// ネイティブ関数インデックス書き出し
	void WriteNativeFunctionIndex( const wchar_t * pwszFuncName ) ;
	void WriteNakedNativeFunctionIndex( const wchar_t * pwszFuncName ) ;
	// ネイティブ関数インデックス生成
	int MakeNakedNativeFunctionIndex( const wchar_t * pwszFuncName ) ;
	// 関数アドレス（参照）書き出し
	void WriteFunctionAddress( const wchar_t * pwszGlobalFuncName ) ;
	// n バイト書き出し
	void WriteCodeData( const void * ptrData, unsigned int nBytes ) ;
	// イメージを確定する
	void CommitImage( void ) ;

	//////////////////////////////////////////////////////////////////////////
	// 詞葉 3.0 Sakura2 仮想マシン用コード出力
	//////////////////////////////////////////////////////////////////////////
protected:
	// 命令コード出力
	void WriteSakuraInstructionCode
			( ECSSakura2Processor::InstructionCode code ) ;
public:
	// CSVariableType から ECSSakura2Processor::DataType へ変換
	static ECSSakura2Processor::DataType
			DataTypeFromVariableType( CSVariableType csvtType ) ;
	// ECSTypeInfo からアドレッシングモード取得
	static ECSSakura2Processor::AddressingMode
			AddressingModeFromTypeInfo( const ECSTypeInfo & typeVar ) ;
	// インデックス係数をスケーリング値に変換
	static int NumToIndexScale( int num )
	{
		for ( int i = 0; i < 8; i ++, num >>= 1 )
		{
			if ( num & 0x01 )
			{
				if ( !(num & ~0x01) )
				{
					return	i ;
				}
				return	-1 ;
			}
		}
		return	-1 ;
	}
	// ※このブロックの関数群による命令出力は自動的に最適化される
	// ロード・ストア命令
	ESLError WriteSakuraMoveMemory
		( bool fStore,
			ECSSakura2Processor::AddressingMode mode,
			ECSSakura2Processor::DataType type, int& regDst,
			int regBase, int offset32,
			int regIndex = 0, int scaleIndex = 0, bool fNoRegCache = false ) ;
	ESLError WriteSakuraLoadMemory
		( ECSSakura2Processor::AddressingMode mode,
			ECSSakura2Processor::DataType type, int& regDst,
			int regBase, int offset32,
			int regIndex = 0, int scaleIndex = 0, bool fNoRegCache = false )
	{
		return	WriteSakuraMoveMemory
			( false, mode, type, regDst,
				regBase, offset32, regIndex, scaleIndex, fNoRegCache ) ;
	}
	ESLError WriteSakuraStoreMemory
		( ECSSakura2Processor::AddressingMode mode,
			ECSSakura2Processor::DataType type, int& regSrc,
			int regBase, int offset32,
			int regIndex = 0, int scaleIndex = 0, bool fNoRegCache = false )
	{
		return	WriteSakuraMoveMemory
			( true, mode, type, regSrc,
				regBase, offset32, regIndex, scaleIndex, fNoRegCache ) ;
	}
	ESLError WriteSakuraLoadMemory
		( int& regDst, const ECSTypeInfo & typeVar,
			bool fRefType = false, int iOffset = 0, bool fNoRegCache = false ) ;
	ESLError WriteSakuraStoreMemory
		( int regSrc, const ECSTypeInfo & typeVar,
			bool fRefType = false, int iOffset = 0, bool fNoRegCache = false ) ;
	// ローカルメモリ命令
	ESLError WriteSakuraMoveLocal
		( bool fStore,
			ECSSakura2Processor::LocalAddressingMode mode,
			ECSSakura2Processor::DataType type, int& regDst,
			int offset32, int regIndex = 0,
			int scaleIndex = 0, bool fNoRegCache = false ) ;
	ESLError WriteSakuraLoadLocal
		( ECSSakura2Processor::LocalAddressingMode mode,
			ECSSakura2Processor::DataType type, int& regDst,
			int offset32, int regIndex = 0,
			int scaleIndex = 0, bool fNoRegCache = false )
	{
		return	WriteSakuraMoveLocal
			( false, mode, type, regDst,
				offset32, regIndex, scaleIndex, fNoRegCache ) ;
	}
	ESLError WriteSakuraStoreLocal
		( ECSSakura2Processor::LocalAddressingMode mode,
			ECSSakura2Processor::DataType type, int regSrc,
			int offset32, int regIndex = 0,
			int scaleIndex = 0, bool fNoRegCache = false )
	{
		return	WriteSakuraMoveLocal
			( true, mode, type, regSrc,
				offset32, regIndex, scaleIndex, fNoRegCache ) ;
	}
	// １オペランド（reg）形式命令
	ESLError WriteSakuraOperandReg
		( ECSSakura2Processor::InstructionCode code, int reg ) ;
	ESLError WriteSakuraPushReg( int reg ) ;
	ESLError WriteSakuraPopReg( int reg ) ;
	// ２オペランド（reg,reg）形式命令
	ESLError WriteSakuraOperandRegReg
		( ECSSakura2Processor::InstructionCode code, int dstreg, int srcreg ) ;
	ESLError WriteSakuraSIMD64OperandRegReg
		( ECSSakura2Processor::SIMDPacked2OpInstructionCode code, int dstreg, int srcreg ) ;
	ESLError WriteSakuraCvt2IntRegReg( int dstreg, int srcreg ) ;
	ESLError WriteSakuraCvt2FloatRegReg( int dstreg, int srcreg ) ;
	ESLError WriteSakuraMoveRegReg( int dstreg, int srcreg ) ;
	ESLError WriteSakuraAddRegReg( int dstreg, int srcreg ) ;
	ESLError WriteSakuraSubRegReg( int dstreg, int srcreg ) ;
	ESLError WriteSakuraAndRegReg( int dstreg, int srcreg ) ;
	ESLError WriteSakuraXorRegReg( int dstreg, int srcreg ) ;
	ESLError WriteSakuraCmpNeRegReg( int dstreg, int srcreg ) ;
	ESLError WriteSakuraCmpEqRegReg( int dstreg, int srcreg ) ;
	ESLError WriteSakuraCmpLtRegReg( int dstreg, int srcreg ) ;
	ESLError WriteSakuraCmpGtRegReg( int dstreg, int srcreg ) ;
	// ２オペランド（reg,imm8）形式命令
	ESLError WriteSakuraOperandRegImm8
		( ECSSakura2Processor::InstructionCode code, int reg, int imm8 ) ;
	ESLError WriteSakuraPushRegsImm8( int reg, int imm8 ) ;
	ESLError WriteSakuraPopRegsImm8( int reg, int imm8 ) ;
	// ３オペランド（reg,reg,imm8）形式命令
	ESLError WriteSakuraOperandRegRegImm8
		( ECSSakura2Processor::InstructionCode code,
							int dstreg, int srcreg, int imm8 ) ;
	ESLError WriteSakuraSrlRegRegImm8( int dstreg, int srcreg, int imm8 ) ;
	ESLError WriteSakuraSraRegRegImm8( int dstreg, int srcreg, int imm8 ) ;
	ESLError WriteSakuraSllRegRegImm8( int dstreg, int srcreg, int imm8 ) ;
	// ３オペランド（reg,reg,imm32）形式命令
	ESLError WriteSakuraOperandRegRegImm32
		( ECSSakura2Processor::InstructionCode code,
							int dstreg, int srcreg, int imm32 ) ;
	// ３オペランド（reg,reg,reg）形式命令
	ESLError WriteSakuraMaskMoveRegRegReg
					( int dstreg, int srcreg, int maskreg ) ;
	// 整数即値演算命令
	ESLError WriteSakuraAddRegRegImm32( int dstreg, int srcreg, int imm32 ) ;
	ESLError WriteSakuraMulRegRegImm32( int dstreg, int srcreg, int imm32 ) ;
	// スタックレジスタ加算命令
	DWORD WriteSakuraAddSP( int imm32 ) ;
	// 即値ロード命令
	DWORD WriteSakuraLoadInt64( int reg, INT64 imm64 ) ;
	DWORD WriteSakuraLoadReal64( int reg, REAL64 imm64 ) ;
	DWORD WriteSakuraLoadInt64_FuncPtr( int reg, const wchar_t * pwszFuncName ) ;
	DWORD WriteSakuraLoadInt64_ClassID( int reg, const wchar_t * pwszClassName ) ;
	DWORD WriteSakuraLoadInt64_CStrPtr( int reg, const wchar_t * pwszString ) ;
	DWORD WriteSakuraLoadInt64_VarAddr( int reg, INT64 nAddr ) ;
	DWORD WriteSakuraLoadInt64_GlobalVarAddr( int reg, const wchar_t * pwszString ) ;
	DWORD WriteSakuraLoadInt64_SharedVarAddr( int reg, const wchar_t * pwszString ) ;
	DWORD WriteSakuraLoadInt64_ConstVarAddr( int reg, const wchar_t * pwszString ) ;
	// 相対ジャンプ命令出力
	DWORD WriteSakuraJumpOffset32( int imm32 ) ;
	DWORD WriteSakuraCNJumpOffset32( int reg, int imm32 ) ;
	DWORD WriteSakuraCJumpOffset32( int reg, int imm32 ) ;
	// 間接ジャンプ命令出力
	ESLError WriteSakuraJumpReg( int reg ) ;
	// コール命令出力
	DWORD WriteSakuraCallImm32( int imm32 ) ;
	DWORD WriteSakuraCallFunction( const wchar_t * pwszFuncName ) ;
	// 間接コール命令出力
	ESLError WriteSakuraCallReg( int reg ) ;
	// システムコール命令出力
	DWORD WriteSakuraSysCallImm32( int imm32 ) ;
	DWORD WriteSakuraSysCallFunction( const wchar_t * pwszFuncName ) ;
	void WriteSakuraSysCallIndirect( int reg ) ;
	// リターン命令出力
	void WriteSakuraReturn( void ) ;

public:
	// naked 無名不変文字配列データ登録
	INT64 AllocateNakedConstString( const wchar_t * pwszString ) ;
	// naked データシンボル追加
	void AddNakedSymbolInfo
		( const wchar_t * pwszSymbol, INT64 nAddress ) ;
	// naked データ外部参照追加
	void AddCodeRefNakedGlobalAddress
		( const wchar_t * pwszSymbol, DWORD dwCodeAddr ) ;
	void AddCodeRefNakedConstAddress
		( const wchar_t * pwszSymbol, DWORD dwCodeAddr ) ;
	void AddCodeRefNakedSharedAddress
		( const wchar_t * pwszSymbol, DWORD dwCodeAddr ) ;
	// ネイティブ関数インデックス参照追加
	void AddCodeRefNakedSystemCallID( DWORD dwCodeAddr ) ;
	// クラスインデックス参照追加
	void AddCodeRefClassID( DWORD dwCodeAddr ) ;


protected:
	//////////////////////////////////////////////////////////////////////////
	// 詞葉 3.0 Sakura2 仮想マシン用コード最適化
	//////////////////////////////////////////////////////////////////////////
	DWORD	m_dwOptimizeStart ;
	DWORD	m_dwOptimizeEnd ;

public:
	// 最適化領域開始
	void BeginSakura2Optimize( void ) ;
	// 最適化領域開始
	void FinishSakura2Optimize
		( int regTempFirst = ECSSakura2Processor::regExpr0, int regTempEnd = 0x80 ) ;


	friend	ECSExecutionOptimizer ;
	friend	ECSCompiler ;
	friend	ECSContext ;

} ;
