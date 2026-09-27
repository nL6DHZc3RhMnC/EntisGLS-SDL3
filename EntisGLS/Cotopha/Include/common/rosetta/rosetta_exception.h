
#if	!defined(__ROSETTA_EXCEPTION_H__)
#define	__ROSETTA_EXCEPTION_H__

#include <rosetta/rosetta_string.h>

namespace	Rosetta
{
	//////////////////////////////////////////////////////////////////////////
	// 例外エラー
	//////////////////////////////////////////////////////////////////////////

	class	RSException	: public RSGenericObject
	{
	public:
		SSystem::SString		m_strMessage ;
		const RSParenthesis *	m_pGenParenthesis ;
		size_t					m_iGenStatement ;

	public:
		// クラス情報
		ESL_DECLARE_CLASS_INFO( RSException, RSGenericObject )
		// 構築関数
		RSException
			( RSClass * pClass, const wchar_t * pwszStr,
				const RSParenthesis * pParenthesis, size_t iGenStatement )
			: RSGenericObject(pClass), m_strMessage(pwszStr),
				m_pGenParenthesis(pParenthesis), m_iGenStatement(iGenStatement) {}

	public:	// 型情報
		// 型名
		virtual const wchar_t * GetTypeName( void ) const ;
		// 型テスト
		virtual RSObject * InstanceOf( const wchar_t * pwszType ) ;
		// 文字列変換
		virtual bool AsString( SSystem::SString& strValue ) const ;

	} ;

}

#endif
