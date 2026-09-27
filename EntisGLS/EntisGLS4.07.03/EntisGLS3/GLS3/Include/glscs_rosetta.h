
#if	!defined(__GLSCS_ROSETTA_H__)
#define	__GLSCS_ROSETTA_H__

#include <rosetta/rosetta.h>
/*
#if	!defined(__CTSCRIPT_PLUGIN_H__)
class	ECSObject ;
enum	CSOperatorType ;
enum	CSUnaryOperatorType ;
enum	CSCompareType ;
#endif
*/

namespace	Rosetta
{
	//////////////////////////////////////////////////////////////////////////
	// 詞葉 → Rosetta インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	RSCotophaObject	: public RSObject
	{
	protected:
		ECSContext *	m_pContext ;
		ECSObject *		m_pObject ;
		bool			m_flagOwner ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSCotophaObject, RSObject )
		// 構築関数
		RSCotophaObject
			( RSClass * pClass,
				ECSContext * pContext,
				ECSObject * pObject, bool flagOwner = true ) ;
		// 消滅関数
		virtual ~RSCotophaObject( void ) ;
		// オブジェクト分離
		ECSObject * DetachObject( void ) ;

	public:
		// 型名
		virtual const wchar_t * GetTypeName( void ) const ;
		// 型テスト
		virtual RSObject * InstanceOf( const wchar_t * pwszType ) ;
		// 整数型か？
		virtual bool IsIntegerType( void ) const ;
		// 浮動小数点型か？
		virtual bool IsFloatType( void ) const ;
		// 文字列型か？
		virtual bool IsStringType( void ) const ;
		// オブジェクト型か？
		virtual bool IsObjectType( void ) const ;
		// 整数値取得
		virtual bool AsInteger( int64_t& number ) const ;
		// 実数値取得
		virtual bool AsRealNumber( double& number ) const ;
		// ブール判定
		virtual bool AsBoolean( void ) const ;
		// 文字列変換
		virtual bool AsString( SSystem::SString& strValue ) const ;
		// 同定判定
		virtual bool IsEqualObject( RSObject * pObj ) const ;
		// デバッグ用ダンプ文字列
		virtual void ToDebugDump
			( SSystem::SFileInterface& dump,
					size_t nPtrNest = 10,
					const wchar_t * pwszIndent = NULL ) ;
		// 複製（実体も可能な限り複製）
		virtual RSObject * CloneObject( RSContext& context ) const ;

	public:
		// 要素取得
		virtual RSObject * GetElementAt
			( RSContext& context, int nIndex ) const ;
		// 要素名取得
		virtual const wchar_t * GetElementNameAt( int nIndex ) const ;
		// 要素取得
		virtual RSObject * SetElementAt
			( RSContext& context, int nIndex, RSObject * pObj ) ;
		// 要素数取得
		virtual size_t GetElementCount( void ) const ;
		// 要素最大数取得
		virtual size_t GetElementLimit( void ) const ;
		// メンバ取得
		virtual RSObject * GetMemberAs
			( RSContext& context, const wchar_t * pwszName ) const ;
		// メンバ設定
		virtual RSObject * SetMemberAs
			( RSContext& context, const wchar_t * pwszName, RSObject * pObj ) ;
		// メンバ新規作成
		virtual RSObject * CreateMemberAs
			( RSContext& context, const wchar_t * pwszName, RSObject * pObj ) ;
		// メンバ削除
		virtual RSObject * RemoveMemberAs
			( RSContext& context, const wchar_t * pwszName ) ;
		// メソッド取得
		virtual METHOD_ENTRY * GetMethodAs
			( RSContext& context,
				const wchar_t * pwszName, METHOD_ENTRY& method ) const ;

	public:
		static RSObject * proc_CallFunction
			( RSContext& context, void * pInstacce,
				RSObject* pThis, RSObject** ppArg, size_t count ) ;

		// Rosetta -> 詞葉オブジェクト変換
		ECSObject * CotophaFromRosettaObject( RSObject * pObj ) const ;

		// 詞葉 -> Rosetta 型変換
		static RSFunctionPrototype * RosettaFromCotophaProto
			( RSContext& context, const ECSPrototypeInfo& protoCotopha ) ;
		static RSClass * RosettaFromCotophaType
			( RSContext& context, const ECSTypeInfo& typeCotopha ) ;

	public:
		// 単項演算子
		RSObject * UnaryOperator( RSContext& context, CSUnaryOperatorType uotType ) const ;
		virtual RSObject * OperatorPlus( RSContext& context ) const ;
		virtual RSObject * OperatorNegate( RSContext& context ) const ;
		virtual RSObject * OperatorBitNot( RSContext& context ) const ;
		virtual RSObject * OperatorIncrement( RSContext& context ) ;
		virtual RSObject * OperatorDecrement( RSContext& context ) ;
		// 二項演算子
		RSObject * BinaryOperator
			( RSContext& context, CSOperatorType opType, RSObject * pObj ) const ;
		RSObject * BinaryComparator
			( RSContext& context, CSCompareType cpType, RSObject * pObj ) const ;
		virtual RSObject * OperatorMul( RSContext& context, RSObject * pObj ) const ;
		virtual RSObject * OperatorDiv( RSContext& context, RSObject * pObj ) const ;
		virtual RSObject * OperatorMod( RSContext& context, RSObject * pObj ) const ;
		virtual RSObject * OperatorAdd( RSContext& context, RSObject * pObj ) const ;
		virtual RSObject * OperatorSub( RSContext& context, RSObject * pObj ) const ;
		virtual RSObject * OperatorShiftLeft( RSContext& context, RSObject * pObj ) const ;
		virtual RSObject * OperatorShiftRight( RSContext& context, RSObject * pObj ) const ;
		virtual RSObject * OperatorBitShiftRight( RSContext& context, RSObject * pObj ) const ;
		virtual RSObject * OperatorBitAnd( RSContext& context, RSObject * pObj ) const ;
		virtual RSObject * OperatorBitOr( RSContext& context, RSObject * pObj ) const ;
		virtual RSObject * OperatorBitXor( RSContext& context, RSObject * pObj ) const ;
		virtual RSObject * OperatorCompareEQ( RSContext& context, RSObject * pObj ) const ;
		virtual RSObject * OperatorCompareNE( RSContext& context, RSObject * pObj ) const ;
		virtual RSObject * OperatorCompareGE( RSContext& context, RSObject * pObj ) const ;
		virtual RSObject * OperatorCompareGT( RSContext& context, RSObject * pObj ) const ;
		virtual RSObject * OperatorCompareLE( RSContext& context, RSObject * pObj ) const ;
		virtual RSObject * OperatorCompareLT( RSContext& context, RSObject * pObj ) const ;
		// 代入演算子
		virtual RSObject * OperatorMove( RSContext& context, RSObject * pObj ) ;

	} ;


	//////////////////////////////////////////////////////////////////////////
	// 詞葉 → Rosetta 関数インターフェース
	//////////////////////////////////////////////////////////////////////////

	class	RSCotophaFuncObject	: public RSFunctionObject
	{
	protected:
		ECSContext *	m_pContext ;
		ECSObject *		m_pObject ;
		int				m_nFuncIndex ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSCotophaFuncObject, RSFunctionObject )
		// 構築関数
		RSCotophaFuncObject
			( RSClass * pFuncClass,
				RSContext& context,
				ECSContext * pContext,
				ECSObject * pObject,
				int nFuncIndex,
				const wchar_t * pwszFuncName ) ;
		// 消滅関数
		virtual ~RSCotophaFuncObject( void ) ;

	} ;

}

#endif

