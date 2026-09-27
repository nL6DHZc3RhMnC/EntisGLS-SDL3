
#if	!defined(__GLS4_LOQUATY_DEF_H__)
#define	__GLS4_LOQUATY_DEF_H__

#if !defined(__LOQUATY_H__)
// 最小限の Loquaty ヘッダ
#include <loquaty_stddefs.h>
#endif

namespace	Loquaty
{
	//////////////////////////////////////////////////////////////////////////
	// 数式／文・評価コンテキスト
	//////////////////////////////////////////////////////////////////////////

	class	LInstantEvaluator	: public SSystem::SObject
	{
	public:
		enum	Type
		{
			typeAuto		= -1,
			typeVoid,
			typeBoolean,
			typeInteger,
			typeDouble,
			typeString,
			typeObject,
		} ;

	protected:
		LVirtualMachine *	m_vm ;
		LPtr<LFunctionObj>	m_pFunc ;
		SSystem::SString	m_strErrMsg ;
		LObjPtr				m_pException ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( LInstantEvaluator, SObject )
		// 構築関数
		LInstantEvaluator( void ) : m_vm( nullptr ) {}
		LInstantEvaluator( LVirtualMachine& vm ) : m_vm( &vm ) {}

		// 仮想マシン関連付け
		void AttachVM( LVirtualMachine& vm ) ;

		// 数式コンパイル
		bool MakeExpression
			( const wchar_t * pwszExpr,
				LClass * pThisClass = nullptr,
				Type type = typeAuto, LClass * pRetTypeClass = nullptr ) ;
		// 文コンパイル
		bool MakeStatement
			( const wchar_t * pwszStatement, LClass * pThisClass = nullptr ,
				Type type = typeVoid, LClass * pRetTypeClass = nullptr ) ;

		// エラー出力取得
		const SSystem::SString& GetErrorMessages( void ) const ;
		// コンパイル済みか？
		bool IsCompiled( void ) const ;
		// コンパイル済み関数取得
		const LPtr<LFunctionObj>& GetFunction( void ) const ;
		// this 取得
		LClass * GetThisClass( void ) const ;

		// 式／文評価
		LValue EvaluateValue
			( const LPtr<LTaskObj>& pTask, const LObjPtr& pThis ) ;
		bool EvaluateAsBool
			( const LPtr<LTaskObj>& pTask, const LObjPtr& pThis ) ;
		int64_t EvaluateAsLong
			( const LPtr<LTaskObj>& pTask, const LObjPtr& pThis ) ;
		double EvaluateAsDouble
			( const LPtr<LTaskObj>& pTask, const LObjPtr& pThis ) ;
		SSystem::SString EvaluateAsString
			( const LPtr<LTaskObj>& pTask, const LObjPtr& pThis ) ;
		LObjPtr EvaluateAsObject
			( const LPtr<LTaskObj>& pTask, const LObjPtr& pThis ) ;

		// 文実行 (void)
		void Execute
			( const LPtr<LTaskObj>& pTask, const LObjPtr& pThis ) ;
		// 非同期実行開始
		bool BeginAsync
			( const LPtr<LTaskObj>& pTask, const LObjPtr& pThis ) ;

		// 例外取得
		const LObjPtr& GetException( void ) const ;
	} ;

}

#endif

