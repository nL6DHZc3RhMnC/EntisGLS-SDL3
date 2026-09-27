
/*****************************************************************************
				詞葉 naked モードプロセッサ Sakura2
 *****************************************************************************/

#if	!defined(__GLSCS_SAKURA2_MODULE_MAKER_H__)
#define	__GLSCS_SAKURA2_MODULE_MAKER_H__

#include <sakura/ssys_smart_buffer.h>

namespace	Rosetta
{
	class	RSParenthesis ;
}

namespace	ECSSakura2
{
	//////////////////////////////////////////////////////////////////////////
	// 詞葉モジュール・メーカー
	//////////////////////////////////////////////////////////////////////////

	class	ExecutableModuleMaker	: public	ExecutableModule
	{
	public:
		SSystem::SSmartBuffer	m_sbufCode ;
		SSystem::SSmartBuffer	m_sbufGlobal ;
		SSystem::SSmartBuffer	m_sbufConst ;
		SSystem::SSmartBuffer	m_sbufShared ;

		struct	DebugCodeInfo
		{
			size_t		addrCode ;
			const Rosetta::RSParenthesis *
						pParenthesis ;
			size_t		indexSrc ;
		} ;
		SSystem::SArray<DebugCodeInfo>	m_arrDebugCodeInfo ;

	protected:
		SSystem::SString		m_strCurFunc ;
		FUNC_ENTRY *			m_pfeCurrent ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( ExecutableModuleMaker, ExecutableModule )
		// 構築関数
		ExecutableModuleMaker( void ) ;

	public:
		// 書き出し準備
		void InitializeMake( void ) ;
		// 書き出し完了
		void FinishMake( void ) ;
		// 関数開始
		void BeginFunction( const wchar_t * pwszFuncName ) ;
		// 関数終了
		void EndFunction( void ) ;
		// 関数情報削除
		void DeleteFunction( void ) ;

	public:
		// デバッグ情報追加
		void AddDebugCodeInfo
			( size_t addrCode,
				const Rosetta::RSParenthesis * pParenthesis, size_t indexSrc ) ;
		// コードアドレスからデバッグ情報検索
		const DebugCodeInfo * SearchDebugCodeInfo( size_t addrCode ) const ;

	public:
		// システムコールID生成／取得
		uint32_t GenSystemCallID( const wchar_t * pwszSysCall ) ;
		// システムコール参照追加
		void AddSystemCallRef( size_t nRefAddr ) ;

	public:
		// LOAD reg, mem
		void WriteCodeLoad
			( int regDst,
				ECSSakura2Processor::DataType dataType,
				int regBase, int nOffset = 0,
				int regIndex = -1, int scaleIndex = 0 ) ;
		// STORE mem, reg
		void WriteCodeStore
			( int regSrc,
				ECSSakura2Processor::DataType dataType,
				int regBase, int nOffset = 0,
				int regIndex = -1, int scaleIndex = 0 ) ;
		// MOVE reg, imm64
		void WriteCodeMoveRegImm64( int regDst, int64_t num ) ;
		void WriteCodeMoveRegInt64( int regDst, int64_t num ) ;
		void WriteCodeMoveRegFloat64( int regDst, float64_t num ) ;
		void WriteCodeMoveRegConstString( int regDst, const wchar_t * pwszString ) ;
		// MOVE reg, reg
		void WriteCodeMoveRegReg( int regDst, int regSrc ) ;
		// ADD reg, reg, imm32
		void WriteCodeAddRegRegImm32( int regDst, int regSrc, int imm32 ) ;
		// MUL reg, reg, imm32
		void WriteCodeMulRegRegImm32( int regDst, int regSrc, int imm32 ) ;
		// PUSH reg
		void WriteCodePushReg( int reg ) ;
		// PUSH reg, imm8
		void WriteCodePushRegsImm8( int reg, int imm8 ) ;
		// POP reg
		void WriteCodePopReg( int reg ) ;
		// POP reg, imm8
		void WriteCodePopRegsImm8( int reg, int imm8 ) ;
		// ADD sp, imm32
		size_t WriteCodeAddSP( int imm ) ;
		void CommitCodeAddSP( size_t addrAddSP, int imm ) ;
		// JUMP imm32
		size_t WriteCodeJump( size_t nTargetAddr = 0 ) ;
		// JUMPcc imm32
		size_t WriteCodeCJump( int reg, size_t nTargetAddr = 0 ) ;
		size_t WriteCodeNCJump( int reg, size_t nTargetAddr = 0 ) ;
		// 相対ジャンプアドレス更新
		void CommitJumpAddress( size_t nJumpOrg, size_t nTargetAddr ) ;
		// SYSCALL imm32
		void WriteCodeSyscall( const wchar_t * pwszSysCall ) ;
		// RET
		void WriteCodeReturn( void ) ;
		// 1OP 命令書き出し
		void WriteCode1OP
			( ECSSakura2Processor::InstructionCode code, int regDst ) ;
		// 2OP 命令書き出し
		void WriteCode2OP
			( ECSSakura2Processor::InstructionCode code,
									int regDst, int regSrcImm8 ) ;
		// 3OP 命令書き出し
		void WriteCode3OP
			( ECSSakura2Processor::InstructionCode code,
						int regDst, int regSrc, int regSrc2Imm8 ) ;
		// 64bit 浮動小数点命令書き出し
		void WriteCodeFloat64_2OP
			( ECSSakura2Processor::FloatInstructionCode code,
										int regDst, int regSrc ) ;
		// 64bit SIMD 2OP 命令書き出し
		void WriteCodeSIMD64_2OP
			( ECSSakura2Processor::SIMDPacked2OpInstructionCode code,
									int regDst, int regSrc ) ;
		// コード書き出し
		void WriteCode( const void * ptrCode, size_t nBytes ) ;
		void WriteCodeAt
			( size_t nAddr, const void * ptrCode, size_t nBytes ) ;
		// 次のコードアドレス
		size_t GetNextCodeAddress( void ) const ;

	} ;

}

#endif

