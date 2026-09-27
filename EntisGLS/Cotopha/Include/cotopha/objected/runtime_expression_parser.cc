
#include <cotopha.h>
#include <objected/runtime_expression_parser.h>


//////////////////////////////////////////////////////////////////////////////
// 実行時式パーサー（詞葉オブジェクトベース）
//////////////////////////////////////////////////////////////////////////////

// 定数テーブル関連付け
//////////////////////////////////////////////////////////////////////////////
void RuntimeExpressionParser::AttachConstantTable( Global& rConstant )
{
	m_refConstant ::= rConstant ;
}

// 定数設定
//////////////////////////////////////////////////////////////////////////////
void RuntimeExpressionParser::DefineConstant( String sName, Reference rValue )
{
	m_mapConstant.Remove( sName ) ;
	m_mapConstant[sName] = rValue ;
}

// グローバル変数設定
//////////////////////////////////////////////////////////////////////////////
void RuntimeExpressionParser::AttachGlobalVariable( String sName, Reference rVariable )
{
	m_mapGlobal[sName] ::= rVariable ;
}

// 実行時式評価
//////////////////////////////////////////////////////////////////////////////
Reference RuntimeExpressionParser::EvaluateExpression
	( String& strExpr, int nPriority, String sCloser, bool fAutoCloserNext )
{
	if ( sCloser != "" )
	{
		int	nIndex = strExpr.GetIndex() ;
		if ( strExpr.NextNeedChar( sCloser ) != "" )
		{
			if ( !fAutoCloserNext )
			{
				strExpr.SeekIndex( nIndex ) ;
			}
			return	null ;
		}
	}
	//
	// 第一項
	//
	Reference	rObj ;
	String	sToken = strExpr.NextToken() ;
	if ( sToken == "" )
	{
		return	null ;
	}
	if ( sToken == "(" )
	{
		rObj ::= EvaluateExpression( strExpr, 0, ")", true ) ;
	}
	else if ( (sToken.Left(1) >= "0") && (sToken.Left(1) <= "9") )
	{
		rObj ::= Integer( sToken ) ;
	}
	else if ( sToken == "\"" )
	{
		rObj ::= strExpr.NextEnclosedString("\"").GetCDecoded() ;
	}
	else if ( sToken == "\'" )
	{
		rObj ::= strExpr.NextEnclosedString("\'") ;
	}
	else
	{
		int	nUOpPri = GetUnaryOperatorPriority( sToken ) ;
		if ( nUOpPri >= 0 )
		{
			rObj ::= EvaluateExpression( strExpr, nUOpPri, "" ) ;
			switch ( sToken )
			{
			case	"-":
				rObj ::= - rObj ;
				break ;
			case	"+":
				rObj ::= + rObj ;
				break ;
			case	"--":
				rObj ::= -- rObj ;
				break ;
			case	"++":
				rObj ::= ++ rObj ;
				break ;
			case	"!":
				rObj ::= ! rObj ;
				break ;
			case	"~":
				rObj ::= ~ rObj ;
				break ;
			default:
				OutputError
					( "\'" + sToken + "\' は定義されていない単項演算子です" ) ;
				return	null ;
			}
		}
		else if ( (m_refConstant !== null)
					&& !m_refConstant.IsEmpty( sToken ) )
		{
			rObj ::= m_refConstant[sToken] ;
			if ( typeof(rObj) == "Integer" )
			{
				rObj ::= int( rObj ) ;
			}
			else if ( typeof(rObj) == "Real" )
			{
				rObj ::= double( rObj ) ;
			}
			else if ( typeof(rObj) == "String" )
			{
				rObj ::= String( rObj ) ;
			}
		}
		else if ( !m_mapConstant.IsEmpty( sToken ) )
		{
			rObj ::= m_mapConstant[sToken] ;
			if ( typeof(rObj) == "Integer" )
			{
				rObj ::= int( rObj ) ;
			}
			else if ( typeof(rObj) == "Real" )
			{
				rObj ::= double( rObj ) ;
			}
			else if ( typeof(rObj) == "String" )
			{
				rObj ::= String( rObj ) ;
			}
		}
		else if ( !m_mapGlobal.IsEmpty( sToken ) )
		{
			rObj ::= m_mapGlobal[sToken] ;
		}
		else
		{
			OutputError
				( "\'" + sToken + "\' は定義されていないシンボルです" ) ;
			return	null ;
		}
	}
	while ( !strExpr.SeekNext() )
	{
		//
		// 終端判定
		//
		if ( sCloser != "" )
		{
			int	nIndex = strExpr.GetIndex() ;
			String	sClosed = strExpr.NextNeedChar( sCloser ) ;
			if ( sClosed != "" )
			{
				if ( !fAutoCloserNext )
				{
					strExpr.SeekIndex( nIndex ) ;
				}
				return	rObj ;
			}
		}
		//
		// 二項演算子
		//
		String	sOp ;
		int		nOpPri ;
		for ( ; ; )
		{
			int	nIndex = strExpr.GetIndex() ;
			sOp = strExpr.NextToken() ;
			if ( (sOp == ">") || (sOp == "<") )
			{
				for ( ; ; )
				{
					int	iTemp = strExpr.GetIndex() ;
					String	sNext = strExpr.NextChar() ;
					if ( (sNext == "=") || (sNext == sOp) )
					{
						sOp += sNext ;
						if ( sNext != sOp )
						{
							break ;
						}
					}
					else
					{
						strExpr.SeekIndex( iTemp ) ;
						break;
					}
				}
			}
			nOpPri = GetOperatorPriority( sOp ) ;
			if ( nOpPri < 0 )
			{
				if ( sOp == "++" )
				{
					rObj ::= rObj ++ ;
				}
				else if ( sOp == "--" )
				{
					rObj ::= rObj -- ;
				}
				else
				{
					OutputError
						( "\'" + sOp + "\' は演算子ではありません" ) ;
					strExpr.SeekIndex( nIndex ) ;
					return	rObj ;
				}
			}
			if ( nOpPri <= nPriority )
			{
				strExpr.SeekIndex( nIndex ) ;
				return	rObj ;
			}
			else
			{
				break ;
			}
		}
		if ( sOp == "." )
		{
			String	sMember = strExpr.NextToken() ;
			rObj ::= rObj[sMember] ;
			continue ;
		}
		else if ( sOp == "[" )
		{
			Reference	rObjIndex =
							EvaluateExpression( strExpr, 0, "]", true ) ;
			if ( rObjIndex === null )
			{
				return	rObj ;
			}
			rObj ::= rObj[rObjIndex] ;
			continue ;
		}
		//
		// 第二項評価
		//
		Reference	rObj2nd =
				EvaluateExpression( strExpr, nOpPri, sCloser, false ) ;
		if ( rObj2nd === null )
		{
			return	rObj ;
		}
		//
		// 演算子
		//
		switch ( sOp )
		{
		case	"+":
			rObj ::= rObj + rObj2nd ;
			break ;
		case	"-":
			rObj ::= rObj - rObj2nd ;
			break ;
		case	"*":
			rObj ::= rObj * rObj2nd ;
			break ;
		case	"/":
			rObj ::= rObj / rObj2nd ;
			break ;
		case	"%":
			rObj ::= rObj % rObj2nd ;
			break ;
		case	"&":
			rObj ::= rObj & rObj2nd ;
			break ;
		case	"|":
			rObj ::= rObj | rObj2nd ;
			break ;
		case	"^":
			rObj ::= rObj ^ rObj2nd ;
			break ;
		case	"<<":
			rObj ::= rObj << rObj2nd ;
			break ;
		case	">>":
			rObj ::= rObj >> rObj2nd ;
			break ;
		case	"&&":
			rObj ::= rObj && rObj2nd ;
			break ;
		case	"||":
			rObj ::= rObj || rObj2nd ;
			break ;
		case	"=":
			rObj = rObj2nd ;
			break ;
		case	"+=":
			rObj += rObj2nd ;
			break ;
		case	"-=":
			rObj -= rObj2nd ;
			break ;
		case	"*=":
			rObj *= rObj2nd ;
			break ;
		case	"/=":
			rObj /= rObj2nd ;
			break ;
		case	"%=":
			rObj %= rObj2nd ;
			break ;
		case	"&=":
			rObj &= rObj2nd ;
			break ;
		case	"|=":
			rObj |= rObj2nd ;
			break ;
		case	"^=":
			rObj ^= rObj2nd ;
			break ;
		case	"<<=":
			rObj <<= rObj2nd ;
			break ;
		case	">>=":
			rObj >>= rObj2nd ;
			break ;
		case	"==":
			rObj ::= (rObj == rObj2nd) ;
			break ;
		case	"!=":
			rObj ::= (rObj != rObj2nd) ;
			break ;
		case	"<":
			rObj ::= (rObj < rObj2nd) ;
			break ;
		case	"<=":
			rObj ::= (rObj <= rObj2nd) ;
			break ;
		case	">":
			rObj ::= (rObj > rObj2nd) ;
			break ;
		case	">=":
			rObj ::= (rObj >= rObj2nd) ;
			break ;
		default:
			OutputError
				( "\'" + sOp + "\' は定義されていない二項演算子です" ) ;
			return	rObj ;
		}
	}
	return	rObj ;
}

// 演算子優先度（演算子でない場合には -1）
//////////////////////////////////////////////////////////////////////////////
int RuntimeExpressionParser::GetOperatorPriority( String sOp )
{
	switch ( sOp )
	{
	case	".":
		return	21 ;

	case	"[":
		return	20 ;

	case	"*":
	case	"/":
	case	"%":
		return	11 ;

	case	"+":
	case	"-":
		return	10 ;

	case	"&":
		return	9 ;

	case	"|":
		return	8 ;

	case	"^":
		return	7 ;

	case	">>":
		return	6 ;

	case	"<<":
		return	5 ;

	case	"==":
	case	"!=":
	case	"<":
	case	"<=":
	case	">":
	case	">=":
		return	4 ;

	case	"&&":
		return	3 ;

	case	"||":
		return	2 ;

	case	"=":
	case	"+=":
	case	"-=":
	case	"*=":
	case	"/=":
	case	"%=":
	case	"&=":
	case	"|=":
	case	"^=":
	case	"<<=":
	case	">>=":
		return	1 ;
	}
	return	-1 ;
}

int RuntimeExpressionParser::GetUnaryOperatorPriority( String sOp )
{
	switch ( sOp )
	{
	case	"--":
		return	17 ;

	case	"++":
		return	16 ;

	case	"-":
	case	"+":
	case	"!":
	case	"~":
		return	15 ;
	}
	return	-1 ;
}

// エラー出力
//////////////////////////////////////////////////////////////////////////////
void RuntimeExpressionParser::OutputError( String sErr )
{
}





