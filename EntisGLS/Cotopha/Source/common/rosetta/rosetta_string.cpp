
#include <rosetta/rosetta.h>
#include <rosetta/rosetta_string.h>
#include <rosetta/rosetta_reference.h>

using namespace	SSystem ;
using namespace	Rosetta ;


//////////////////////////////////////////////////////////////////////////////
// 文字列
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSString, RSObject )

// 型名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * RSString::GetTypeName( void ) const
{
	return	L"String" ;
}

// 型テスト
//////////////////////////////////////////////////////////////////////////////
RSObject * RSString::InstanceOf( const wchar_t * pwszType )
{
	if ( SString::Compare( pwszType, L"String" ) == 0 )
	{
		return	this ;
	}
	return	NULL ;
}

// 文字列型か？
//////////////////////////////////////////////////////////////////////////////
bool RSString::IsStringType( void ) const
{
	return	true ;
}

// オブジェクト型か？
//////////////////////////////////////////////////////////////////////////////
bool RSString::IsObjectType( void ) const
{
	return	true ;
}

// 整数値取得
//////////////////////////////////////////////////////////////////////////////
bool RSString::AsInteger( int64_t& number ) const
{
	number = m_strValue.AsInteger() ;
	return	true ;
}

// 実数値取得
//////////////////////////////////////////////////////////////////////////////
bool RSString::AsRealNumber( double& number ) const
{
	number = m_strValue.AsReal() ;
	return	true ;
}

// ブール判定
//////////////////////////////////////////////////////////////////////////////
bool RSString::AsBoolean( void ) const
{
	return	(m_strValue.GetLength() != 0) ;
}

// 文字列変換
//////////////////////////////////////////////////////////////////////////////
bool RSString::AsString( SSystem::SString& strValue ) const
{
	strValue = m_strValue ;
	return	true ;
}

// 同定判定
//////////////////////////////////////////////////////////////////////////////
bool RSString::IsEqualObject( RSObject * pObj ) const
{
	if ( pObj == NULL )
	{
		return	(this == pObj) ;
	}
	SString	strObj ;
	if ( !pObj->AsString( strObj ) )
	{
		return	false ;
	}
	return	(m_strValue == strObj) ;
}

// デバッグ用ダンプ文字列
//////////////////////////////////////////////////////////////////////////////
void RSString::ToDebugDump
	( SSystem::SFileInterface& dump,
			size_t nPtrNest, const wchar_t * pwszIndent )
{
	SString	strDump = L"\"" ;
	strDump += m_strValue ;
	strDump += L"\"" ;
	dump.WriteEncodedString( strDump ) ;
}

// 値設定
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSString::SetIntegerAs( int64_t nValue )
{
	m_strValue.FromInteger( nValue ) ;
	return	errSuccess ;
}

SSystem::SError RSString::SetNumberAs( double nValue )
{
	m_strValue.FromReal( nValue ) ;
	return	errSuccess ;
}

SSystem::SError RSString::SetStringAs( const wchar_t * pwszValue )
{
	m_strValue = pwszValue ;
	return	errSuccess ;
}

// 内部リソース解放
//////////////////////////////////////////////////////////////////////////////
void RSString::DisposeObject( RSContext& context )
{
	m_strValue.FreeArray() ;
	RSObject::DisposeObject( context ) ;
}

// 複製（実体も可能な限り複製）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSString::CloneObject( RSContext& context ) const
{
	return	context.new_String( m_strValue ) ;
}

// 単項演算子
//////////////////////////////////////////////////////////////////////////////
RSObject * RSString::OperatorPlus( RSContext& context ) const
{
	return	context.new_String( m_strValue ) ;
}

RSObject * RSString::OperatorNegate( RSContext& context ) const
{
	SString	strValue = m_strValue ;
	strValue.Reverse() ;
	return	context.new_String( strValue ) ;
}

// 二項演算子
//////////////////////////////////////////////////////////////////////////////
RSObject * RSString::OperatorMul( RSContext& context, RSObject * pObj ) const
{
	int64_t	number ;
	if ( pObj->AsInteger( number ) )
	{
		SString	strValue = m_strValue * (int) number ;
		return	context.new_String( strValue ) ;
	}
	return	NULL ;
}

RSObject * RSString::OperatorAdd( RSContext& context, RSObject * pObj ) const
{
	SString	strValue ;
	if ( pObj->AsString( strValue ) )
	{
		return	context.new_String( m_strValue + strValue ) ;
	}
	return	NULL ;
}

RSObject * RSString::OperatorCompareEQ( RSContext& context, RSObject * pObj ) const
{
	RSObject *	pEntity = pObj->GetEntityObject() ;
	if ( pEntity != NULL )
	{
		SString	strValue ;
		if ( pEntity->AsString( strValue ) )
		{
			return	context.new_Boolean( m_strValue == strValue ) ;
		}
	}
	return	context.new_Boolean( false ) ;
}

RSObject * RSString::OperatorCompareNE( RSContext& context, RSObject * pObj ) const
{
	RSObject *	pEntity = pObj->GetEntityObject() ;
	if ( pEntity != NULL )
	{
		SString	strValue ;
		if ( pEntity->AsString( strValue ) )
		{
			return	context.new_Boolean( m_strValue != strValue ) ;
		}
	}
	return	context.new_Boolean( true ) ;
}

RSObject * RSString::OperatorCompareGE( RSContext& context, RSObject * pObj ) const
{
	SString	strValue ;
	if ( pObj->AsString( strValue ) )
	{
		return	context.new_Boolean( m_strValue >= strValue ) ;
	}
	return	context.new_Boolean( false ) ;
}

RSObject * RSString::OperatorCompareGT( RSContext& context, RSObject * pObj ) const
{
	SString	strValue ;
	if ( pObj->AsString( strValue ) )
	{
		return	context.new_Boolean( m_strValue > strValue ) ;
	}
	return	context.new_Boolean( false ) ;
}

RSObject * RSString::OperatorCompareLE( RSContext& context, RSObject * pObj ) const
{
	SString	strValue ;
	if ( pObj->AsString( strValue ) )
	{
		return	context.new_Boolean( m_strValue <= strValue ) ;
	}
	return	context.new_Boolean( false ) ;
}

RSObject * RSString::OperatorCompareLT( RSContext& context, RSObject * pObj ) const
{
	SString	strValue ;
	if ( pObj->AsString( strValue ) )
	{
		return	context.new_Boolean( m_strValue < strValue ) ;
	}
	return	context.new_Boolean( false ) ;
}

// 代入演算子
//////////////////////////////////////////////////////////////////////////////
RSObject * RSString::OperatorMove( RSContext& context, RSObject * pObj )
{
	if ( pObj->AsString( m_strValue ) )
	{
		AddRef() ;
		return	this ;
	}
	context.ThrowExceptionError( L"文字列へ変換できません" ) ;
	return	NULL ;
}

// シリアライズ
//////////////////////////////////////////////////////////////////////////////
RSObject * RSString::SerializeObject( RSContext& context )
{
	return	CloneObject( context ) ;
}

SSystem::SError RSString::SerializeBinary
		( RSContext& context, SSystem::SFileInterface& file )
{
	return	file.WriteString( m_strValue ) ;
}

SSystem::SError RSString::MakeXMLDocument
		( RSContext& context, SSystem::SXMLDocument& xmlDoc )
{
	xmlDoc.SetTextElement( m_strValue ) ;
	return	errSuccess ;
}

// 復元
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSString::RestoreObject
		( RSContext& context, RSObject * pObj )
{
	context.ReleaseObjectRef( OperatorMove( context, pObj ) ) ;
	return	errSuccess ;
}

SSystem::SError RSString::RestoreBinary
		( RSContext& context, SSystem::SFileInterface& file )
{
	return	file.ReadString( m_strValue ) ;
}

SSystem::SError RSString::RestoreXMLDocument
		( RSContext& context, const SSystem::SXMLDocument& xmlDoc )
{
	SString *	pstrText = xmlDoc.GetTextElement() ;
	if ( pstrText != NULL )
	{
		m_strValue = *pstrText ;
	}
	else
	{
		m_strValue = L"" ;
	}
	return	errSuccess ;
}


//////////////////////////////////////////////////////////////////////////////
// 文字列型
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSStringClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSStringClass::RSStringClass( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSStringClass::OverrideVirtuals( RSContext& context )
{
	CreateMemberIntegerAs
		( context, L"encodingUnknown", Charset::encodingUnknown, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"encodingShiftJIS", Charset::encodingShiftJIS, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"encodingUTF8", Charset::encodingUTF8, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"encodingISO2022JP", Charset::encodingISO2022JP, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"encodingEUCJP", Charset::encodingEUCJP, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"encodingUTF16", Charset::encodingUTF16, modifierConst ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"String value",
				NULL, &RSStringClass::method_init2, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"Uint16Pointer value",
				NULL, &RSStringClass::method_init3, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>",
				NULL, L"Uint16Pointer value, int count",
				NULL, &RSStringClass::method_init3, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"char[] value",
				NULL, &RSStringClass::method_init1, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>",
				NULL, L"char[] value, int offset, int count",
				NULL, &RSStringClass::method_init1, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setString", NULL, L"String str",
			NULL, &RSStringClass::method_setString, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"length", L"int", L"",
			NULL, &RSStringClass::method_length,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"indexOf", L"int",
			L"String str, int first = 0",
			NULL, &RSStringClass::method_indexOf,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"charAt", L"String",
			L"int index",
			NULL, &RSStringClass::method_charAt,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"charCodeAt", L"char",
			L"int index",
			NULL, &RSStringClass::method_charCodeAt,
			NULL, RSFunctionPrototype::flagConstant,
			L"　文字コードを取得します。index が範囲外の場合 0 を返します。" ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"charLastCodeAt", L"char",
			L"int index",
			NULL, &RSStringClass::method_charLastCodeAt,
			NULL, RSFunctionPrototype::flagConstant,
			L"　文字コードを取得します。index が範囲外の場合 0 を返します。\n"
			L"<param name=\"index\">文字終端を 0 として前方向への指標</param>" ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"compareTo", L"int",
			L"String str",
			NULL, &RSStringClass::method_compareTo,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"compareToIgnoreCase", L"int",
			L"String str",
			NULL, &RSStringClass::method_compareToIgnoreCase,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"replace", L"String",
			L"String oldStr, String newStr",
			NULL, &RSStringClass::method_replace,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"substring", L"String",
			L"int begin",
			NULL, &RSStringClass::method_substring,
			NULL, RSFunctionPrototype::flagConstant,
			L"　begin から終端までの文字列を取得します。\n"
			L"　begin が負数の場合先頭から、begin が文字列長を超えている場合空の文字列を返します。" ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"substring", L"String",
			L"int begin, int end",
			NULL, &RSStringClass::method_substring,
			NULL, RSFunctionPrototype::flagConstant,
			L"　begin から end までの文字列を取得します。\n"
			L"　end に指定された指標の文字は含みません。\n"
			L"　end に負数が指定された場合、文字列終端までの文字列を返します。" ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"toCharArray", L"char[]", L"",
			NULL, &RSStringClass::method_toCharArray,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"toUpperCase", L"String", L"",
			NULL, &RSStringClass::method_toUpperCase,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"toLowerCase", L"String", L"",
			NULL, &RSStringClass::method_toLowerCase,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"chopRight", L"String", L"int count = 1",
			NULL, &RSStringClass::method_chopRight,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"trim", L"String", L"",
			NULL, &RSStringClass::method_trim,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"trimRight", L"String", L"",
			NULL, &RSStringClass::method_trimRight,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"trimLeft", L"String", L"",
			NULL, &RSStringClass::method_trimLeft,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"asInteger", L"long", L"",
			NULL, &RSStringClass::method_asInteger,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"asNumber", L"double", L"",
			NULL, &RSStringClass::method_asNumber,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"parseInt", L"long", L"int radix = 10",
			NULL, &RSStringClass::method_parseInt,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"format", L"String", L"String fmt, ...",
				NULL, &RSStringClass::method_format, NULL ) ;
	//
	AddVirtualDescriptiveAs
		( context, perr, L"mappingFilter",
			L"String", L"HashMap map",
			NULL, &RSStringClass::method_mappingFilter,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getFileNamePart",
			L"String", L"char sep = \'\\\\\'",
			NULL, &RSStringClass::method_getFileNamePart,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getFileExtensionPart",
			L"String", L"char sep = \'\\\\\'",
			NULL, &RSStringClass::method_getFileExtensionPart,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getFileDirectoryPart",
			L"String", L"char sep = \'\\\\\'",
			NULL, &RSStringClass::method_getFileDirectoryPart,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getFileTitlePart",
			L"String", L"char sep = \'\\\\\'",
			NULL, &RSStringClass::method_getFileTitlePart,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getFileDrivePart",
			L"String", L"char sep = \'\\\\\'",
			NULL, &RSStringClass::method_getFileDrivePart,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"offsetFilePath",
			L"String", L"String sOffsetPath, char sep = \'\\\\\'",
			NULL, &RSStringClass::method_offsetFilePath,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"relativeFilePath",
			L"String", L"String sFullPath, int nAscendLimit = 5, char sep = \'\\\\\'",
			NULL, &RSStringClass::method_relativeFilePath,
			NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"normalizeFilePath",
			L"String", L"char sep = \'\\\\\'",
			NULL, &RSStringClass::method_normalizeFilePath,
			NULL, RSFunctionPrototype::flagConstant ) ;
	//
	AddFunctionDescriptiveAs
		( context, perr, L"getEncodingName", L"String", L"int encoding",
				NULL, &RSStringClass::method_getEncodingName, NULL ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"getEncodingType", L"int", L"String encoding",
				NULL, &RSStringClass::method_getEncodingType, NULL ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"analyzeEncoding",
				L"int", L"Uint8Pointer ptrSrc, int nLength = -1",
				NULL, &RSStringClass::method_analyzeEncoding, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"encodeTo",
				L"Uint8Pointer", L"int encoding",
				NULL, &RSStringClass::method_encodeTo,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"decode",
				L"String", L"int encoding, Uint8Pointer ptrSrc, int nLength = -1",
				NULL, &RSStringClass::method_decode, NULL ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"encodeBase64",
				L"String", L"Uint8Pointer ptrSrc, int nLength = -1",
				NULL, &RSStringClass::method_encodeBase64, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"decodeBase64",
				L"Uint8Pointer", L"",
				NULL, &RSStringClass::method_decodeBase64,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"encodeHex",
				L"String", L"Uint8Pointer ptrSrc, int nLength = -1",
				NULL, &RSStringClass::method_encodeHex, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"decodeHex",
				L"Uint8Pointer", L"",
				NULL, &RSStringClass::method_decodeHex,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"decodeCLangString",
				L"String", L"",
				NULL, &RSStringClass::method_decodeCLangString,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"encodeCLangString",
				L"String", L"",
				NULL, &RSStringClass::method_encodeCLangString,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"decodeCSVString",
				L"String", L"",
				NULL, &RSStringClass::method_decodeCSVString,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"encodeCSVString",
				L"String", L"",
				NULL, &RSStringClass::method_encodeCSVString,
				NULL, RSFunctionPrototype::flagConstant ) ;
}

// インスタンス生成
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringClass::NewInstance( RSContext& context, RSObject * pArg )
{
	if ( (pArg != NULL) && (pArg->GetElementCount() >= 1) )
	{
		if ( m_pConstructor != NULL )
		{
			RSObject *	pObj = context.new_String( NULL ) ;
			bool		fArgMatchResult ;
			RSObject *	pRetInstance =
				context.CallFunction
					( *m_pConstructor, pObj, *pArg, false, &fArgMatchResult ) ;
			if ( !fArgMatchResult )
			{
				if ( pArg->GetElementCount() > 0 )
				{
					context.ThrowExceptionError
						( L"構築関数の呼び出しで引数がプロトタイプに一致しません" ) ;
				}
			}
			if ( pRetInstance != NULL )
			{
				context.ReleaseObjectRef( pObj ) ;
				pObj = pRetInstance ;
			}
			return	pObj ;
		}
		if ( pArg->GetElementCount() >= 2 )
		{
			context.ThrowExceptionError
				( m_strClassName + L" の構築引数が多すぎます" ) ;
			return	NULL ;
		}
		RSObject *	pObjArg = pArg->GetElementAt( context, 0 ) ;
		if ( pObjArg != NULL )
		{
			SString	str ;
			if ( pObjArg->AsString( str ) )
			{
				return	context.new_String( str ) ;
			}
		}
		context.ThrowExceptionError
			( m_strClassName + L" の初期値が不正です" ) ;
		return	NULL ;
	}
	return	context.new_String( NULL ) ;
}

// 変数インスタンス生成
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringClass::NewVariable( RSContext& context )
{
	return	context.new_Pointer( NULL, this ) ;
}

// キャスト処理
//////////////////////////////////////////////////////////////////////////////
bool RSStringClass::TestCastInstance
	( RSObject * pObj, RSClass::CastMethod castMethod )
{
	if ( castMethod == castForce )
	{
		return	true ;
	}
	return	RSClass::TestCastInstance( pObj, castMethod ) ;
}

RSObject * RSStringClass::CastInstance
	( RSContext& context, RSObject * pObj, RSClass::CastMethod castMethod )
{
	if ( castMethod == castForce )
	{
		if ( pObj != NULL )
		{
			SString	strValue ;
			if ( pObj->AsString( strValue ) )
			{
				return	context.new_String( strValue ) ;
			}
			return	NULL ;
		}
		return	context.new_String( L"null" ) ;
	}
	return	RSClass::CastInstance( context, pObj, castMethod ) ;
}

// void <init>( char[] value )
// void <init>( char[] value, int offset, int count )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringClass::method_init1
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSString *	pObj = ESLTypeCast<RSString>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"String 構築関数の this が String ではありません" ) ;
		return	NULL ;
	}
	RSObject *	pValue = arg.ObjectAt( 0 ) ;
	if ( pValue != NULL )
	{
		int		iOffset = arg.IntAt( 1, 0 ) ;
		int		nCount = arg.IntAt( 2, -1 ) ;
		size_t	nLength = pValue->GetElementCount() ;
		if ( nCount == -1 )
		{
			nCount = (int) nLength - iOffset ;
		}
		if ( (iOffset < 0) || ((size_t) iOffset > nLength)
			|| (nCount < 0) || ((size_t) (iOffset + nCount) > nLength) )
		{
			context.ThrowExceptionError
				( L"指標が範囲を超えています",
						L"IndexOutOfBoundsException" ) ;
			return	NULL ;
		}
		uint16_t *	pwBuf = pObj->m_strValue.LockBuffer( (size_t) nCount ) ;
		for ( int i = 0; i < nCount; i ++ )
		{
			RSObject *	pChar =
					pValue->GetElementAt( context, iOffset + i ) ;
			int64_t	num ;
			if ( (pChar != NULL) && pChar->AsInteger( num ) )
			{
				pwBuf[i] = (uint16_t) num ;
			}
		}
		pObj->m_strValue.UnlockBuffer( (ssize_t) nCount ) ;
	}
	return	NULL ;
}

// void <init>( String value )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringClass::method_init2
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSString *	pObj = ESLTypeCast<RSString>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"String 構築関数の this が String ではありません" ) ;
		return	NULL ;
	}
	pObj->m_strValue = arg.StringAt( 0 ) ;
	return	NULL ;
}

// void <init>( Uint16Pointer value )
// void <init>( Uint16Pointer value, int count )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringClass::method_init3
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSString *	pObj = ESLTypeCast<RSString>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"String 構築関数の this が String ではありません" ) ;
		return	NULL ;
	}
	size_t		nBytes ;
	uint16_t *	pwszStr = (uint16_t*) arg.PointerAt( 0, &nBytes ) ;
	ssize_t		nLength = (ssize_t) arg.IntAt( 1, -1 ) ;
	if ( (nLength < 0) || (nLength * sizeof(uint16_t) > nBytes) )
	{
		size_t	nLimit = nBytes / sizeof(uint16_t) ;
		nLength = (ssize_t) nLimit ;
		for ( size_t i = 0; i < nLimit; i ++ )
		{
			if ( pwszStr[i] == 0 )
			{
				nLength = (ssize_t) i ;
			}
		}
	}
	pObj->m_strValue.SetString( pwszStr, nLength ) ;
	return	NULL ;
}

// void setString( String str )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringClass::method_setString
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSString *	pObj = ESLTypeCast<RSString>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"String.setString 関数の this が String ではありません" ) ;
		return	NULL ;
	}
	pObj->m_strValue = arg.StringAt( 0 ) ;
	return	NULL ;
}

// int length()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringClass::method_length
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSString *	pObj = ESLTypeCast<RSString>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"String.length 関数の this が String ではありません" ) ;
		return	NULL ;
	}
	return	context.new_Integer( pObj->m_strValue.GetLength() ) ;
}

// int indexOf( String str, int first = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringClass::method_indexOf
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSString *	pObj = ESLTypeCast<RSString>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"String.indexOf 関数の this が String ではありません" ) ;
		return	NULL ;
	}
	return	context.new_Integer
		( pObj->m_strValue.Find( arg.StringAt(0), arg.IntAt(1) ) ) ;
}

// String charAt( int index )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringClass::method_charAt
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSString *	pObj = ESLTypeCast<RSString>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"String.charAt 関数の this が String ではありません" ) ;
		return	NULL ;
	}
	int	index = arg.IntAt( 0 ) ;
	if ( (index < 0) || ((size_t) index >= pObj->m_strValue.GetLength()) )
	{
		context.ThrowExceptionError
			( L"指標が範囲を超えています",
					L"IndexOutOfBoundsException" ) ;
		return	NULL ;
	}
	return	context.new_String
		( pObj->m_strValue.Middle( index, 1 ) ) ;
}

// char charCodeAt( int index )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringClass::method_charCodeAt
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSString *	pObj = ESLTypeCast<RSString>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"String.charCodeAt 関数の this が String ではありません" ) ;
		return	NULL ;
	}
	int	index = arg.IntAt( 0 ) ;
	return	context.new_Integer( pObj->m_strValue.GetAt( (size_t) index ) ) ;
}

// char charLastCodeAt( int index )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringClass::method_charLastCodeAt
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSString *	pObj = ESLTypeCast<RSString>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"String.charCodeAt 関数の this が String ではありません" ) ;
		return	NULL ;
	}
	int	index = arg.IntAt( 0 ) ;
	return	context.new_Integer( pObj->m_strValue.GetLastAt( (size_t) index ) ) ;
}

// int compareTo( String str )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringClass::method_compareTo
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSString *	pObj = ESLTypeCast<RSString>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"String.compareTo 関数の this が String ではありません" ) ;
		return	NULL ;
	}
	return	context.new_Integer
				( pObj->m_strValue.Compare( arg.StringAt(0) ) ) ;
}

// int compareToIgnoreCase( String str )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringClass::method_compareToIgnoreCase
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSString *	pObj = ESLTypeCast<RSString>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"String.compareToIgnoreCase 関数の this が String ではありません" ) ;
		return	NULL ;
	}
	return	context.new_Integer
				( pObj->m_strValue.CompareNoCase( arg.StringAt(0) ) ) ;
}

// String replace( String oldStr, String newStr )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringClass::method_replace
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSString *	pObj = ESLTypeCast<RSString>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"String.replace 関数の this が String ではありません" ) ;
		return	NULL ;
	}
	SString		strOld = arg.StringAt( 0 ) ;
	ssize_t		iNext = pObj->m_strValue.Find( strOld ) ;
	if ( iNext < 0 )
	{
		pObj->AddRef() ;
		return	pObj ;
	}
	RSString *	pStrResult =
					ESLSmartCast<RSString>
						( context.new_String
							( pObj->m_strValue.Left( (size_t) iNext ) ) ) ;
	SString		strNew = arg.StringAt( 1 ) ;
	size_t		iLast = (size_t) iNext + strOld.GetLength() ;
	//
	pStrResult->m_strValue += strNew ;
	//
	for ( ; ; )
	{
		iNext = pObj->m_strValue.Find( strOld, iLast ) ;
		if ( iNext < 0 )
		{
			pStrResult->m_strValue += pObj->m_strValue.Middle( iLast ) ;
			break ;
		}
		pStrResult->m_strValue +=
				pObj->m_strValue.
					Middle( iLast, (ssize_t) (iNext - iLast) ) ;
		pStrResult->m_strValue += strNew ;
		iLast = (size_t) iNext + strOld.GetLength() ;
	}
	return	pStrResult ;
}

// String substring( int begin )
// String substring( int begin, int end )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringClass::method_substring
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	RSString *	pObj = ESLTypeCast<RSString>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"String.substring 関数の this が String ではありません" ) ;
		return	NULL ;
	}
	int	iBegin = arg.IntAt( 0 ) ;
	int	iEnd = arg.IntAt( 1, -1 ) ;
	if ( iBegin < 0 )
	{
		iBegin = 0 ;
	}
	else if ( iBegin >= (int) pObj->m_strValue.GetLength() )
	{
		return	context.new_String( NULL ) ;
	}
	if ( (iEnd < 0) || (iEnd > (int) pObj->m_strValue.GetLength()) )
	{
		iEnd = (int) pObj->m_strValue.GetLength() ;
	}
	if ( iEnd < iBegin )
	{
		context.ThrowExceptionError
			( L"指標が範囲外です", L"IndexOutOfBoundsException" ) ;
		return	NULL ;
	}
	return	context.new_String
				( pObj->m_strValue.Middle( iBegin, iEnd - iBegin ) ) ;
}

// char[] toCharArray()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringClass::method_toCharArray
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSString *	pObj = ESLTypeCast<RSString>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"String.toCharArray 関数の this が String ではありません" ) ;
		return	NULL ;
	}
	RSObject *	pArray =
		context.new_Array
			( 0x7FFFFFFF,
				context.GetBasicTypeClass( RSCodeControl::wiChar ) ) ;
	size_t	nLength = pObj->m_strValue.GetLength() ;
	for ( size_t i = 0; i < nLength; i ++ )
	{
		pArray->SetElementIntegerAt
			( context, (int) i, pObj->m_strValue.GetAt(i) ) ;
	}
	return	pArray ;
}

// String toUpperCase()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringClass::method_toUpperCase
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SString	strOrg ;
	if ( (pThis == nullptr) || !pThis->AsString( strOrg ) )
	{
		context.ThrowExceptionError
			( L"String.toUpperCase 関数の this が String ではありません" ) ;
		return	NULL ;
	}
	strOrg.MakeUpper() ;
	return	context.new_String( strOrg ) ;
}

// String toLowerCase()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringClass::method_toLowerCase
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SString	strOrg ;
	if ( (pThis == nullptr) || !pThis->AsString( strOrg ) )
	{
		context.ThrowExceptionError
			( L"String.toLowerCase 関数の this が String ではありません" ) ;
		return	NULL ;
	}
	strOrg.MakeLower() ;
	return	context.new_String( strOrg ) ;
}

// String chopRight( int count = 1 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringClass::method_chopRight
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SString	strOrg ;
	if ( (pThis == nullptr) || !pThis->AsString( strOrg ) )
	{
		context.ThrowExceptionError
			( L"String.chopRight 関数の this が String ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	strOrg.ChopRight( (size_t) arg.IntAt( 0, 1 ) ) ;
	return	context.new_String( strOrg ) ;
}

// String trim()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringClass::method_trim
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SString	strOrg ;
	if ( (pThis == nullptr) || !pThis->AsString( strOrg ) )
	{
		context.ThrowExceptionError
			( L"String.trim 関数の this が String ではありません" ) ;
		return	NULL ;
	}
	strOrg.TrimRight() ;
	strOrg.TrimLeft() ;
	return	context.new_String( strOrg ) ;
}

// String trimRight()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringClass::method_trimRight
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SString	strOrg ;
	if ( (pThis == nullptr) || !pThis->AsString( strOrg ) )
	{
		context.ThrowExceptionError
			( L"String.trimRight 関数の this が String ではありません" ) ;
		return	NULL ;
	}
	strOrg.TrimRight() ;
	return	context.new_String( strOrg ) ;
}

// String trimLeft()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringClass::method_trimLeft
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SString	strOrg ;
	if ( (pThis == nullptr) || !pThis->AsString( strOrg ) )
	{
		context.ThrowExceptionError
			( L"String.trimLeft 関数の this が String ではありません" ) ;
		return	NULL ;
	}
	strOrg.TrimLeft() ;
	return	context.new_String( strOrg ) ;
}

// long asInteger()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringClass::method_asInteger
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	int64_t	num = 0 ;
	if ( (pThis == nullptr) || !pThis->AsInteger( num ) )
	{
		context.ThrowExceptionError
			( L"String を Integer に変換できません" ) ;
		return	NULL ;
	}
	return	context.new_Integer( num ) ;
}

// double asNumber()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringClass::method_asNumber
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	double	num = 0 ;
	if ( (pThis == nullptr) || !pThis->AsRealNumber( num ) )
	{
		context.ThrowExceptionError
			( L"String を Number に変換できません" ) ;
		return	NULL ;
	}
	return	context.new_Number( num ) ;
}

// long parseInt( int radix = 10 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringClass::method_parseInt
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	SString	strOrg ;
	if ( (pThis == nullptr) || !pThis->AsString( strOrg ) )
	{
		context.ThrowExceptionError
			( L"String.parseInt 関数の this が String ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	const int	nRadix = arg.IntAt( 0, 10 ) ;
	return	context.new_Integer( strOrg.AsInteger ( nRadix ) ) ;
}

// static String format( String fmt, ... )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringClass::method_format
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	if ( count < 1 )
	{
		context.ThrowExceptionError
			( L"String.format 関数の引数が少なすぎます" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SString				strFormat = arg.StringAt(0) ;
	SString				strResult ;
	context.FormatStringVlist
			( strResult, strFormat, ppArg + 1, count - 1 ) ;
	return	context.new_String( strResult ) ;
}

// String mappingFilter( HashMap map )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringClass::method_mappingFilter
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSString *	pStrThis = ESLTypeCast<RSString>( pThis ) ;
	if ( pStrThis == NULL )
	{
		context.ThrowExceptionError
			( L"String.mappingFilter 関数の this が String ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSDynamicObject *
			pObj = ESLTypeCast<RSDynamicObject>( arg.ObjectAt(0) ) ;
	//
	SArray<SString::FILTER_ENTRY>	arrFilter ;
	size_t	nCount = pObj->GetElementCount() ;
	for ( size_t i = 0; i < nCount; i ++ )
	{
		SString::FILTER_ENTRY	fe ;
		fe.pwszSource = pObj->GetElementNameAt( (int) i ) ;
		//
		RSSmartPtr	pElement
			( pObj->GetElementAt( context, (int) i ), &context ) ;
		RSString *	pStr = NULL ;
		if ( pElement != NULL )
		{
			pStr = ESLTypeCast<RSString>( pElement->GetEntityObject() ) ;
			if ( pStr != NULL )
			{
				fe.pwszTarget = pStr->m_strValue ;
			}
		}
		if ( fe.pwszSource && fe.pwszTarget )
		{
			arrFilter.Add( fe ) ;
		}
	}
	SString::FILTER_ENTRY *	pfeFilter = arrFilter.GetArray() ;
	size_t					nFilters = arrFilter.GetLength() ;
	SString::PrepareFilter( pfeFilter, nFilters ) ;
	RSObject *	pResult = 
		context.new_String
			( pStrThis->m_strValue.MappingFilter( pfeFilter, nFilters ) ) ;
	arrFilter.FinishArray() ;
	return	pResult ;
}

// String getFileNamePart( char sep = '\\' )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringClass::method_getFileNamePart
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSString *	pStrThis = ESLTypeCast<RSString>( pThis ) ;
	if ( pStrThis == NULL )
	{
		context.ThrowExceptionError
			( L"String.getFileNamePart 関数の this が String ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_String
		( SString( pStrThis->m_strValue.
					GetFileNamePart( (wchar_t) arg.IntAt(0, L'\\') ) ) ) ;
}

// String getFileExtensionPart( char sep = '\\' )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringClass::method_getFileExtensionPart
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSString *	pStrThis = ESLTypeCast<RSString>( pThis ) ;
	if ( pStrThis == NULL )
	{
		context.ThrowExceptionError
			( L"String.getFileExtensionPart 関数の this が String ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_String
		( SString( pStrThis->m_strValue.
					GetFileExtensionPart( (wchar_t) arg.IntAt(0, L'\\') ) ) ) ;
}

// String getFileDirectoryPart( char sep = '\\' )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringClass::method_getFileDirectoryPart
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSString *	pStrThis = ESLTypeCast<RSString>( pThis ) ;
	if ( pStrThis == NULL )
	{
		context.ThrowExceptionError
			( L"String.getFileDirectoryPart 関数の this が String ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	wchar_t	wchSep = (wchar_t) arg.IntAt( 0, L'\\' ) ;
	SString	strDirPart = pStrThis->m_strValue.GetFileDirectoryPart( wchSep ) ;
	while ( (strDirPart.GetLastAt(0) == L'\\')
				|| (strDirPart.GetLastAt(0) == L'/')
				|| (strDirPart.GetLastAt(0) == wchSep) )
	{
		strDirPart.ChopRight( 1 ) ;
	}
	return	context.new_String( strDirPart ) ;
}

// String getFileTitlePart( char sep = '\\' )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringClass::method_getFileTitlePart
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSString *	pStrThis = ESLTypeCast<RSString>( pThis ) ;
	if ( pStrThis == NULL )
	{
		context.ThrowExceptionError
			( L"String.getFileTitlePart 関数の this が String ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_String
		( pStrThis->m_strValue.
				GetFileTitlePart( (wchar_t) arg.IntAt(0, L'\\') ) ) ;
}

// String getFileDrivePart( char sep = '\\' )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringClass::method_getFileDrivePart
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSString *	pStrThis = ESLTypeCast<RSString>( pThis ) ;
	if ( pStrThis == NULL )
	{
		context.ThrowExceptionError
			( L"String.getFileDrivePart 関数の this が String ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_String
		( pStrThis->m_strValue.
				GetFileDrivePart( (wchar_t) arg.IntAt(0, L'\\') ) ) ;
}

// String offsetFilePath( String sOffsetPath, char sep = '\\' )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringClass::method_offsetFilePath
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSString *	pStrThis = ESLTypeCast<RSString>( pThis ) ;
	if ( pStrThis == NULL )
	{
		context.ThrowExceptionError
			( L"String.offsetFilePath 関数の this が String ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_String
		( pStrThis->m_strValue.OffsetFilePath
				( arg.StringAt(0), (wchar_t) arg.IntAt(1, L'\\') ) ) ;
}

// String relativeFilePath( String sFullPath, int nAscendLimit = 5, char sep = '\\' )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringClass::method_relativeFilePath
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSString *	pStrThis = ESLTypeCast<RSString>( pThis ) ;
	if ( pStrThis == NULL )
	{
		context.ThrowExceptionError
			( L"String.relativeFilePath 関数の this が String ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_String
		( pStrThis->m_strValue.RelativeFilePath
				( arg.StringAt(0), arg.IntAt(1,5), (wchar_t) arg.IntAt(2, L'\\') ) ) ;
}

// String normalizeFilePath( char sep = '\\' )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringClass::method_normalizeFilePath
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSString *	pStrThis = ESLTypeCast<RSString>( pThis ) ;
	if ( pStrThis == NULL )
	{
		context.ThrowExceptionError
			( L"String.normalizeFilePath 関数の this が String ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SString	strFilePath = pStrThis->m_strValue ;
	return	context.new_String
		( strFilePath.NormalizeFilePath( (wchar_t) arg.IntAt(0, L'\\') ) ) ;
}

// static String getEncodingName( int encoding )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringClass::method_getEncodingName
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_String
		( Charset::GetEncodingName( (Charset::EncodingType) arg.IntAt(0) ) ) ;
}

// static int getEncodingType( String encoding )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringClass::method_getEncodingType
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Integer( Charset::GetEncodingType( arg.StringAt(0) ) ) ;
}

// static int analyzeEncoding
//	( Uint8Pointer ptrSrc, int nLength = -1 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringClass::method_analyzeEncoding
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	size_t		nPtrBounds = 0 ;
	uint8_t *	pbytSrc = arg.PointerAt( 0, &nPtrBounds ) ;
	if ( pbytSrc == NULL )
	{
		return	context.new_Integer( Charset::encodingUnknown ) ;
	}
	ssize_t		nLength = (ssize_t) arg.IntAt( 1, -1 ) ;
	if ( nLength < 0 )
	{
		nLength = (ssize_t) nPtrBounds ;
		for ( size_t i = 0; i < nPtrBounds; i ++ )
		{
			if ( pbytSrc[i] == 0 )
			{
				nLength = (ssize_t) i ;
				break ;
			}
		}
	}
	else if ( (size_t) nLength > nPtrBounds )
	{
		context.ThrowExceptionError
			( L"引数がバッファ長を超えています", L"IndexOutOfBoundsException" ) ;
		return	NULL ;
	}
	return	context.new_Integer
				( Charset::AnalyzeEncoding( pbytSrc, nLength ) ) ;
}

// Uint8Pointer encodeTo( int encoding )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringClass::method_encodeTo
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSString *	pStrThis = ESLTypeCast<RSString>( pThis ) ;
	if ( pStrThis == NULL )
	{
		context.ThrowExceptionError
			( L"String.encodeTo 関数の this が String ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SArray<uint8_t>	bufDst ;
	Charset::Encode
		( bufDst, (Charset::EncodingType) arg.IntAt(0),
			pStrThis->m_strValue,
			(ssize_t) pStrThis->m_strValue.GetLength() ) ;
	//
	RSArrayBuffer *	pBuffer = new RSArrayBuffer ;
	pBuffer->AllocateBuffer( bufDst.GetLength() ) ;
	eslMoveMemory
		( pBuffer->m_ptrBuf, bufDst.GetConstArray(), bufDst.GetLength() ) ;
	//
	return	context.new_PointerNumber
				( pBuffer, RSReferenceNumber::typeUint8 ) ;
}

// static String decode
//	( int encoding, Uint8Pointer ptrSrc, int nLength = -1 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringClass::method_decode
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	size_t		nPtrBounds = 0 ;
	uint8_t *	pbytSrc = arg.PointerAt( 1, &nPtrBounds ) ;
	if ( pbytSrc == NULL )
	{
		return	context.new_String() ;
	}
	ssize_t		nLength = (ssize_t) arg.IntAt( 2, -1 ) ;
	if ( nLength < 0 )
	{
		nLength = (ssize_t) nPtrBounds ;
		for ( size_t i = 0; i < nPtrBounds; i ++ )
		{
			if ( pbytSrc[i] == 0 )
			{
				nLength = (ssize_t) i ;
				break ;
			}
		}
	}
	else if ( (size_t) nLength > nPtrBounds )
	{
		context.ThrowExceptionError
			( L"引数がバッファ長を超えています", L"IndexOutOfBoundsException" ) ;
		return	NULL ;
	}
	SString	strDst ;
	Charset::Decode
		( strDst, (Charset::EncodingType) arg.IntAt(0), pbytSrc, nLength ) ;
	return	context.new_String( strDst ) ;
}

// static String encodeBase64( Uint8Pointer ptrSrc, int nLength = -1 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringClass::method_encodeBase64
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	size_t		nPtrBounds = 0 ;
	uint8_t *	pbytSrc = arg.PointerAt( 0, &nPtrBounds ) ;
	if ( pbytSrc == NULL )
	{
		return	context.new_String() ;
	}
	ssize_t		nLength = (ssize_t) arg.IntAt( 1, -1 ) ;
	if ( nLength < 0 )
	{
		nLength = (ssize_t) nPtrBounds ;
		for ( size_t i = 0; i < nPtrBounds; i ++ )
		{
			if ( pbytSrc[i] == 0 )
			{
				nLength = (ssize_t) i ;
				break ;
			}
		}
	}
	else if ( (size_t) nLength > nPtrBounds )
	{
		context.ThrowExceptionError
			( L"引数がバッファ長を超えています", L"IndexOutOfBoundsException" ) ;
		return	NULL ;
	}
	SString	strDst ;
	Charset::EncodeBase64( strDst, pbytSrc, nLength ) ;
	return	context.new_String( strDst ) ;
}

// Uint8Pointer decodeBase64()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringClass::method_decodeBase64
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSString *	pStrThis = ESLTypeCast<RSString>( pThis ) ;
	if ( pStrThis == NULL )
	{
		context.ThrowExceptionError
			( L"String.encodeTo 関数の this が String ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SArray<uint8_t>	bufDst ;
	Charset::DecodeBase64
		( bufDst, pStrThis->m_strValue,
			(ssize_t) pStrThis->m_strValue.GetLength() ) ;
	//
	RSArrayBuffer *	pBuffer = new RSArrayBuffer ;
	pBuffer->AllocateBuffer( bufDst.GetLength() ) ;
	eslMoveMemory
		( pBuffer->m_ptrBuf, bufDst.GetConstArray(), bufDst.GetLength() ) ;
	//
	return	context.new_PointerNumber
				( pBuffer, RSReferenceNumber::typeUint8 ) ;
}

// static String encodeHex( Uint8Pointer ptrSrc, int nLength = -1 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringClass::method_encodeHex
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	size_t		nPtrBounds = 0 ;
	uint8_t *	pbytSrc = arg.PointerAt( 0, &nPtrBounds ) ;
	if ( pbytSrc == NULL )
	{
		return	context.new_String() ;
	}
	ssize_t		nLength = (ssize_t) arg.IntAt( 1, -1 ) ;
	if ( nLength < 0 )
	{
		nLength = (ssize_t) nPtrBounds ;
		for ( size_t i = 0; i < nPtrBounds; i ++ )
		{
			if ( pbytSrc[i] == 0 )
			{
				nLength = (ssize_t) i ;
				break ;
			}
		}
	}
	else if ( (size_t) nLength > nPtrBounds )
	{
		context.ThrowExceptionError
			( L"引数がバッファ長を超えています", L"IndexOutOfBoundsException" ) ;
		return	NULL ;
	}
	SString	strDst ;
	SStringParser::EncodeHexString( strDst, pbytSrc, nLength ) ;
	return	context.new_String( strDst ) ;
}

// Uint8Pointer decodeHex()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringClass::method_decodeHex
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSString *	pStrThis = ESLTypeCast<RSString>( pThis ) ;
	if ( pStrThis == NULL )
	{
		context.ThrowExceptionError
			( L"String.decodeHex 関数の this が String ではありません" ) ;
		return	NULL ;
	}
	SArray<uint8_t>	bufDst ;
	SStringParser::DecodeHexString
		( bufDst, pStrThis->m_strValue,
			(ssize_t) pStrThis->m_strValue.GetLength() ) ;
	//
	RSArrayBuffer *	pBuffer = new RSArrayBuffer ;
	pBuffer->AllocateBuffer( bufDst.GetLength() ) ;
	eslMoveMemory
		( pBuffer->m_ptrBuf, bufDst.GetConstArray(), bufDst.GetLength() ) ;
	//
	return	context.new_PointerNumber
				( pBuffer, RSReferenceNumber::typeUint8 ) ;
}

// String decodeCLangString()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringClass::method_decodeCLangString
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSString *	pStrThis = ESLTypeCast<RSString>( pThis ) ;
	if ( pStrThis == NULL )
	{
		context.ThrowExceptionError
			( L"String.decodeCLangString 関数の this が String ではありません" ) ;
		return	NULL ;
	}
	SString	strDst ;
	SStringParser::DecodeCLangString( strDst, pStrThis->m_strValue ) ;
	//
	return	context.new_String( strDst ) ;
}

// String encodeCLangString()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringClass::method_encodeCLangString
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSString *	pStrThis = ESLTypeCast<RSString>( pThis ) ;
	if ( pStrThis == NULL )
	{
		context.ThrowExceptionError
			( L"String.decodeCLangString 関数の this が String ではありません" ) ;
		return	NULL ;
	}
	SString	strDst ;
	SStringParser::EncodeCLangString( strDst, pStrThis->m_strValue ) ;
	//
	return	context.new_String( strDst ) ;
}

// String decodeCSVString()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringClass::method_decodeCSVString
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSString *	pStrThis = ESLTypeCast<RSString>( pThis ) ;
	if ( pStrThis == NULL )
	{
		context.ThrowExceptionError
			( L"String.decodeCSVString 関数の this が String ではありません" ) ;
		return	NULL ;
	}
	SString	strDst = pStrThis->m_strValue ;
	SStringParser::DecodeCSVString( strDst ) ;
	//
	return	context.new_String( strDst ) ;
}

// String encodeCSVString()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringClass::method_encodeCSVString
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSString *	pStrThis = ESLTypeCast<RSString>( pThis ) ;
	if ( pStrThis == NULL )
	{
		context.ThrowExceptionError
			( L"String.encodeCSVString 関数の this が String ではありません" ) ;
		return	NULL ;
	}
	SString	strDst ;
	bool	flagNeedDQuote ;
	SStringParser::EncodeCSVString
		( strDst, pStrThis->m_strValue, flagNeedDQuote ) ;
	if ( flagNeedDQuote )
	{
		strDst = SString(L"\"") + strDst + L"\"" ;
	}
	return	context.new_String( strDst ) ;
}



//////////////////////////////////////////////////////////////////////////////
// 文字列パーサー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSStringParser, RSObject )

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
RSStringParser::~RSStringParser( void )
{
	RSObject::ReleaseRef( m_pRefString ) ;
	m_pRefString = NULL ;
}

// 型名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * RSStringParser::GetTypeName( void ) const
{
	return	L"StringParser" ;
}

// 型テスト
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParser::InstanceOf( const wchar_t * pwszType )
{
	if ( SString::Compare( pwszType, L"StringParser" ) == 0 )
	{
		return	this ;
	}
	return	NULL ;
}

// 文字列型か？
//////////////////////////////////////////////////////////////////////////////
bool RSStringParser::IsStringType( void ) const
{
	return	true ;
}

// 即値型か？
//////////////////////////////////////////////////////////////////////////////
bool RSStringParser::IsObjectType( void ) const
{
	return	true ;
}

// ブール判定
//////////////////////////////////////////////////////////////////////////////
bool RSStringParser::AsBoolean( void ) const
{
	return	(m_parser.GetLength() != 0) ;
}

// 文字列変換
//////////////////////////////////////////////////////////////////////////////
bool RSStringParser::AsString( SSystem::SString& strValue ) const
{
	strValue = m_parser.SubString( 0 ) ;
	return	true ;
}

// 同定判定
//////////////////////////////////////////////////////////////////////////////
bool RSStringParser::IsEqualObject( RSObject * pObj ) const
{
	if ( pObj == NULL )
	{
		return	(this == pObj) ;
	}
	SString	strObj ;
	if ( !pObj->AsString( strObj ) )
	{
		return	false ;
	}
	return	(m_parser.SubString(0) == strObj) ;
}

// デバッグ用ダンプ文字列
//////////////////////////////////////////////////////////////////////////////
void RSStringParser::ToDebugDump
	( SSystem::SFileInterface& dump,
			size_t nPtrNest, const wchar_t * pwszIndent )
{
	SString	strDump = L"\"" ;
	strDump += m_parser.SubString(0) ;
	strDump += L"\", index=" ;
	strDump += SString( m_parser.GetIndex() ) ;
	dump.WriteEncodedString( strDump ) ;
}

// 内部リソース解放
//////////////////////////////////////////////////////////////////////////////
void RSStringParser::DisposeObject( RSContext& context )
{
	m_parser.ReleaseString() ;
	context.ReleaseObjectRef( m_pRefString ) ;
	m_pRefString = NULL ;
	RSObject::DisposeObject( context ) ;
}

// 複製（実体も可能な限り複製）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParser::CloneObject( RSContext& context ) const
{
	RSStringParser *	pObj = new RSStringParser( m_pClass ) ;
	if ( m_pRefString != NULL )
	{
		pObj->m_parser.AttachString( m_pRefString->m_strValue ) ;
		pObj->m_parser.SeekIndex( m_parser.GetIndex() ) ;
		pObj->m_pRefString = m_pRefString ;
		RSObject::AddRef( m_pRefString ) ;
	}
	else
	{
		pObj->m_parser = m_parser ;
	}
	return	pObj ;
}

// 代入演算子
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParser::OperatorMove( RSContext& context, RSObject * pObj )
{
	context.ReleaseObjectRef( m_pRefString ) ;
	m_pRefString = NULL ;
	//
	RSString *	pStr = ESLTypeCast<RSString>( pObj ) ;
	if ( pStr != NULL )
	{
		m_parser.AttachString( pStr->m_strValue ) ;
		pStr->AddRef() ;
		m_pRefString = pStr ;
		//
		AddRef() ;
		return	this ;
	}
	else
	{
		SString	strValue ;
		if ( pObj->AsString( strValue ) )
		{
			m_parser = strValue ;
			//
			AddRef() ;
			return	this ;
		}
	}
	context.ThrowExceptionError( L"文字列へ変換できません" ) ;
	return	NULL ;
}



//////////////////////////////////////////////////////////////////////////////
// 文字列パーサー型
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSStringParserClass, RSClass )
// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSStringParserClass::RSStringParserClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSStringParserClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSStringParser( this ) ;
	//
	CreateMemberIntegerAs
		( context, L"tokenInvalid", SStringParser::tokenInvalid, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"tokenNormal", SStringParser::tokenNormal, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"tokenPunctuation", SStringParser::tokenPunctuation, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"tokenSpecialMark", SStringParser::tokenSpecialMark, modifierConst ) ;
	//
	CreateMemberIntegerAs
		( context, L"ctrlNoEscInQuote", SStringParser::ctrlNoEscInQuote, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"ctrlNoEscInDQuote", SStringParser::ctrlNoEscInDQuote, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"ctrlNoRadixPostfix", SStringParser::ctrlNoRadixPostfix, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"ctrlCStyleNumber", SStringParser::ctrlCStyleNumber, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"ctrlEscCSVInDQuote", SStringParser::ctrlEscCSVInDQuote, modifierConst ) ;
	//
	CreateMemberIntegerAs
		( context, L"numberInvalid", SStringParser::numberInvalid, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"numberDefault", SStringParser::numberDefault, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"numberInteger", SStringParser::numberInteger, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"numberRadix2", SStringParser::numberRadix2, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"numberRadix8", SStringParser::numberRadix8, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"numberRadix10", SStringParser::numberRadix10, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"numberRadix16", SStringParser::numberRadix16, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"numberStyleOct", SStringParser::numberStyleOct, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"numberStyleHex", SStringParser::numberStyleHex, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"numberRadixMask", SStringParser::numberRadixMask, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"numberFlagReal", SStringParser::numberFlagReal, modifierConst ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"String value",
				NULL, &RSStringParserClass::method_init1, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"StringParser value",
				NULL, &RSStringParserClass::method_init2, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"attachString", NULL, L"String str",
				NULL, &RSStringParserClass::method_attachString, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"releaseString", NULL, L"",
				NULL, &RSStringParserClass::method_releaseString, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"loadTextFile", L"boolean",
			L"String sFilePath, int encoding = String.encodingUnknown",
				NULL, &RSStringParserClass::method_loadTextFile, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"readTextFile", L"boolean",
			L"InputStream is, int encoding = String.encodingUnknown",
				NULL, &RSStringParserClass::method_readTextFile, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"readTextFile", L"boolean",
			L"RandomAccessFile raf, int encoding = String.encodingUnknown",
				NULL, &RSStringParserClass::method_readTextFile, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getLength", L"int", L"",
				NULL, &RSStringParserClass::method_getLength,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getIndex", L"int", L"",
				NULL, &RSStringParserClass::method_getIndex,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"seekIndex", L"int", L"int index",
				NULL, &RSStringParserClass::method_seekIndex, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"isIndexOverflow", L"boolean", L"",
				NULL, &RSStringParserClass::method_isIndexOverflow,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"seekString", L"boolean", L"String str",
			NULL, &RSStringParserClass::method_seekString, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"seekAnyCharacters", L"boolean", L"String strChars",
				NULL, &RSStringParserClass::method_seekAnyCharacters, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"markIndex", NULL, L"",
				NULL, &RSStringParserClass::method_markIndex, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"seekToMark", NULL, L"",
				NULL, &RSStringParserClass::method_seekToMark, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"releaseMark", NULL, L"",
				NULL, &RSStringParserClass::method_releaseMark, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"subString",
			L"String", L"int iStart, int nCount = -1",
			NULL, &RSStringParserClass::method_subString, NULL, 0,
			L"　iStart から nCount 文字分の部分文字列を取得します。" ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"subStringFromMark", L"String", L"",
			NULL, &RSStringParserClass::method_subStringFromMark, NULL, 0,
			L"　記憶指標から現在の指標までの部分文字列を取得します。\n"
			L"　現在の指標の文字は含みません。" ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"subStringFrom", L"String", L"int iStart",
			NULL, &RSStringParserClass::method_subStringFrom, NULL, 0,
			L"　iStart 位置から現在の指標までの部分文字列を取得します。\n"
			L"　現在の指標の文字は含みません。" ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getLineNumberOf",
				L"int", L"int index, int[] pGetLineIndex = null",
				NULL, &RSStringParserClass::method_getLineNumberOf, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"currentCharacter", L"char", L"",
			NULL, &RSStringParserClass::method_currentCharacter, NULL, 0,
			L"　現在の指標の文字を取得します。指標が範囲を超えている場合 0 を返します。" ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"offsetAt", L"char", L"int iOffset",
			NULL, &RSStringParserClass::method_offsetAt, NULL, 0,
			L"　現在の指標から iOffset 番目の文字を取得します。\n"
			L"　指標が範囲を超えている場合、0 を返します。" ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"isCharacterSpace", L"boolean", L"char c",
				NULL, &RSStringParserClass::method_isCharacterSpace, NULL ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"isPunctuation", L"boolean", L"char c",
				NULL, &RSStringParserClass::method_isPunctuation, NULL ) ;
	AddFunctionDescriptiveAs
		( context, perr, L"isSpecialMark", L"boolean", L"char c",
				NULL, &RSStringParserClass::method_isSpecialMark, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"passSpace", L"boolean", L"",
			NULL, &RSStringParserClass::method_passSpace, NULL, 0,
			L"　現在の指標から空白文字の間、指標を進めます。\n"
			L"　空白以外の文字を発見した場合、true を返します。" ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"seekToNextLine", L"int", L"",
				NULL, &RSStringParserClass::method_seekToNextLine, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"passString", NULL, L"",
				NULL, &RSStringParserClass::method_passString, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"passEnclosedString", NULL,
				L"char chCloser, int nCtrlFlags = 0",
				NULL, &RSStringParserClass::method_passEnclosedString, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"passToken", L"int", L"",
				NULL, &RSStringParserClass::method_passToken, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"passExpressionTerm", NULL, L"int nCtrlFlags = 0",
				NULL, &RSStringParserClass::method_passExpressionTerm, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"passExpression", NULL,
				L"String strCloses, int nCtrlFlags = 0",
				NULL, &RSStringParserClass::method_passExpression, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getCharacter", L"char", L"",
			NULL, &RSStringParserClass::method_getCharacter, NULL, 0,
			L"　現在の指標の文字を取得し、指標を次の文字へ進めます。" ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getLine", L"String", L"int[] pRetCode = null",
			NULL, &RSStringParserClass::method_getLine, NULL, 0,
			L"　現在の指標から行末までの文字列を取得します。\n"
			L"　文字列には行末の改行記号が含まれます。\n"
			L"　pRetCode が null でない場合、pRetCode[0] に改行コードが返されます。\n"
			L"　改行コードが CRLF の場合には 0x0D0A が返されます。" ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getString", L"String", L"",
				NULL, &RSStringParserClass::method_getString, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getEnclosedString", L"String",
			L"char chCloser, int nCtrlFlags = 0, char[] pGetClosed = null",
			NULL, &RSStringParserClass::method_getEnclosedString, NULL, 0,
			L"　現在の指標から chCloser 文字を見つけるまでの区間の文字列を取得します。\n"
			L"　文字列に終端の chCloser は含まれませんが。指標は chCloser の次へ移動します。\n"
			L"　pGetClosed が null でない場合 pGetClosed[0] に chCloser が返されます。\n"
			L"　chCloser が見つからなかった場合、終端までの文字列が返されます。" ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getToken", L"String",
				L"int[] pGetType = null",
				NULL, &RSStringParserClass::method_getToken, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getStringTerm", L"String",
				L"int nCtrlFlags = 0",
				NULL, &RSStringParserClass::method_getStringTerm, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getExpressionTerm", L"String",
				L"int nCtrlFlags = 0",
				NULL, &RSStringParserClass::method_getExpressionTerm, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"getExpression", L"String",
			L"String strCloses, int nCtrlFlags = 0, char[] pGetClosed = null",
				NULL, &RSStringParserClass::method_getExpression, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"hasToComeChar",
			L"char", L"String strNext",
			NULL, &RSStringParserClass::method_hasToComeChar, NULL, 0,
			L"　現在の指標から空白を除く次の文字が strNext に指定した文字列の"
			L"要素文字のいずれかだった場合、その文字コードを返し、"
			L"指標をその次の文字へ移動します。\n"
			L"　次の文字が指定文字列のいずれの文字でもなかった場合 0 を返し"
			L"指標は移動しません。" ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"hasToComeString",
				L"boolean", L"String strNext",
				NULL, &RSStringParserClass::method_hasToComeString, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"hasToComeNoCaseString",
				L"boolean", L"String strNext",
				NULL, &RSStringParserClass::method_hasToComeNoCaseString, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"hasToComeToken",
				L"boolean", L"String strNext",
				NULL, &RSStringParserClass::method_hasToComeToken, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"isNextNumber",
				L"int", L"int nCtrlFlags = 0",
				NULL, &RSStringParserClass::method_isNextNumber, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"nextInteger",
				L"long", L"int type = StringParser.numberInteger",
				NULL, &RSStringParserClass::method_nextInteger, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"nextRealNumber",
				L"double", L"int type = StringParser.numberDefault",
				NULL, &RSStringParserClass::method_nextRealNumber, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"parseIntegerArray", L"int",
				L"long[] pNumbers, int nCount, "
				L"int nCtrlFlags = 0, String strSeparator = null",
				NULL, &RSStringParserClass::method_parseIntegerArray, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"parseHexIntegerArray", L"int",
			L"long[] pNumbers, int nCount, String strSeparator = null",
				NULL, &RSStringParserClass::method_parseHexIntegerArray, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"parseNumberArray", L"int",
				L"double[] pNumbers, int nCount, "
				L"int nCtrlFlags = 0, String strSeparator = null",
				NULL, &RSStringParserClass::method_parseNumberArray, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"parseCommaSeparatedValues", L"int",
				L"String[] aValues",
				NULL, &RSStringParserClass::method_parseCommaSeparatedValues, NULL ) ;
}

// void <init>( String value )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParserClass::method_init1
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSStringParser *	pObj = ESLTypeCast<RSStringParser>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser 構築関数の this が StringParser ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pObj->m_parser = arg.StringAt( 0 ) ;
	return	NULL ;
}

// void <init>( StringParser value )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParserClass::method_init2
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSStringParser *	pObjThis = ESLTypeCast<RSStringParser>( pThis ) ;
	if ( pObjThis == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser 構築関数の this が StringParser ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSStringParser *
			pObj = ESLTypeCast<RSStringParser>( arg.ObjectAt( 0 ) ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser 構築関数の引数が null です", L"NullPointerException" ) ;
		return	NULL ;
	}
	pObjThis->m_parser = pObj->m_parser ;
	return	NULL ;
}

// void attachString( String str )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParserClass::method_attachString
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSStringParser *	pObjThis = ESLTypeCast<RSStringParser>( pThis ) ;
	if ( pObjThis == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser.attachString 関数の this が StringParser ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSString *	pObj = ESLTypeCast<RSString>( arg.ObjectAt( 0 ) ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser.attachString 関数の引数が null です", L"NullPointerException" ) ;
		return	NULL ;
	}
	pObjThis->m_parser.AttachString( pObj->m_strValue ) ;
	context.ReleaseObjectRef( pObjThis->m_pRefString ) ;
	pObjThis->m_pRefString = pObj ;
	RSObject::AddRef( pObj ) ;
	return	NULL ;
}

// void releaseString()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParserClass::method_releaseString
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSStringParser *	pObjThis = ESLTypeCast<RSStringParser>( pThis ) ;
	if ( pObjThis == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser.releaseString 関数の this が StringParser ではありません" ) ;
		return	NULL ;
	}
	pObjThis->m_parser.ReleaseString() ;
	context.ReleaseObjectRef( pObjThis->m_pRefString ) ;
	pObjThis->m_pRefString = NULL ;
	return	NULL ;
}

// boolean loadTextFile
//	( String sFilePath, int encoding = String::encodingUnknown )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParserClass::method_loadTextFile
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSStringParser *	pObjThis = ESLTypeCast<RSStringParser>( pThis ) ;
	if ( pObjThis == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser.loadTextFile 関数の this が StringParser ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Boolean
		( pObjThis->m_parser.LoadTextFile
			( arg.StringAt(0),
				(Charset::EncodingType)
					arg.IntAt(1,Charset::encodingUnknown) ) == errSuccess ) ;
}

// boolean readTextFile
//	( InputStream is, int encoding = String::encodingUnknown )
// boolean readTextFile
//	( RandomAccessFile raf, int encoding = String::encodingUnknown )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParserClass::method_readTextFile
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSStringParser *	pObjThis = ESLTypeCast<RSStringParser>( pThis ) ;
	if ( pObjThis == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser.readTextFile 関数の this が StringParser ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SFileInterface *
			pFile = ESLTypeCast<SFileInterface>( arg.NativeObjectAt(0) ) ;
	if ( pFile == NULL )
	{
		context.ThrowExceptionError( L"StringParser.readTextFile の引数が null です" ) ;
		return	NULL ;
	}
	return	context.new_Boolean
		( pObjThis->m_parser.ReadTextFile
			( *pFile, (Charset::EncodingType)
						arg.IntAt(1, Charset::encodingUnknown) ) == errSuccess ) ;
}

// int getLength()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParserClass::method_getLength
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSStringParser *	pObjThis = ESLTypeCast<RSStringParser>( pThis ) ;
	if ( pObjThis == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser.getLength 関数の this が StringParser ではありません" ) ;
		return	NULL ;
	}
	return	context.new_Integer( pObjThis->m_parser.GetLength() ) ;
}

// int getIndex()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParserClass::method_getIndex
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSStringParser *	pObjThis = ESLTypeCast<RSStringParser>( pThis ) ;
	if ( pObjThis == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser.getIndex 関数の this が StringParser ではありません" ) ;
		return	NULL ;
	}
	return	context.new_Integer( pObjThis->m_parser.GetIndex() ) ;
}

// int seekIndex( int index )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParserClass::method_seekIndex
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSStringParser *	pObjThis = ESLTypeCast<RSStringParser>( pThis ) ;
	if ( pObjThis == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser.getIndex 関数の this が StringParser ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Integer
				( pObjThis->m_parser.SeekIndex( (size_t) arg.IntAt(0) ) ) ;
}

// boolean isIndexOverflow()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParserClass::method_isIndexOverflow
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSStringParser *	pObjThis = ESLTypeCast<RSStringParser>( pThis ) ;
	if ( pObjThis == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser.isIndexOverflow 関数の this が StringParser ではありません" ) ;
		return	NULL ;
	}
	return	context.new_Boolean( pObjThis->m_parser.IsIndexOverflow() ) ;
}

// boolean seekString( String str )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParserClass::method_seekString
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSStringParser *	pObjThis = ESLTypeCast<RSStringParser>( pThis ) ;
	if ( pObjThis == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser.seekString 関数の this が StringParser ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Boolean
		( pObjThis->m_parser.SeekString( arg.StringAt(0) ) ) ;
}

// boolean seekAnyCharacters( String str )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParserClass::method_seekAnyCharacters
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSStringParser *	pObjThis = ESLTypeCast<RSStringParser>( pThis ) ;
	if ( pObjThis == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser.seekAnyCharacters 関数の this が StringParser ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Boolean
		( pObjThis->m_parser.SeekAnyCharacters( arg.StringAt(0) ) ) ;
}

// void markIndex()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParserClass::method_markIndex
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSStringParser *	pObjThis = ESLTypeCast<RSStringParser>( pThis ) ;
	if ( pObjThis == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser.markIndex 関数の this が StringParser ではありません" ) ;
		return	NULL ;
	}
	pObjThis->m_parser.MarkIndex() ;
	return	NULL ;
}

// void seekToMark()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParserClass::method_seekToMark
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSStringParser *	pObjThis = ESLTypeCast<RSStringParser>( pThis ) ;
	if ( pObjThis == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser.seekToMark 関数の this が StringParser ではありません" ) ;
		return	NULL ;
	}
	pObjThis->m_parser.SeekToMark() ;
	return	NULL ;
}

// void releaseMark()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParserClass::method_releaseMark
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSStringParser *	pObjThis = ESLTypeCast<RSStringParser>( pThis ) ;
	if ( pObjThis == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser.releaseMark 関数の this が StringParser ではありません" ) ;
		return	NULL ;
	}
	pObjThis->m_parser.ReleaseMark() ;
	return	NULL ;
}

// String subString( int iStart, int nCount = -1 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParserClass::method_subString
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSStringParser *	pObjThis = ESLTypeCast<RSStringParser>( pThis ) ;
	if ( pObjThis == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser.subString 関数の this が StringParser ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_String
		( pObjThis->m_parser.SubString
				( (size_t) arg.IntAt(0), (ssize_t) arg.IntAt(1,-1) ) ) ;
}

// String subStringFromMark()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParserClass::method_subStringFromMark
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSStringParser *	pObjThis = ESLTypeCast<RSStringParser>( pThis ) ;
	if ( pObjThis == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser.subStringFromMark 関数の this が StringParser ではありません" ) ;
		return	NULL ;
	}
	return	context.new_String
				( pObjThis->m_parser.SubStringFromMark() ) ;
}

// String subStringFrom( int iStart )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParserClass::method_subStringFrom
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSStringParser *	pObjThis = ESLTypeCast<RSStringParser>( pThis ) ;
	if ( pObjThis == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser.subStringFrom 関数の this が StringParser ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_String
		( pObjThis->m_parser.SubStringFrom( (size_t) arg.IntAt(0) ) ) ;
}

// int getLineNumberOf( int index, int[] pGetLineIndex = null )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParserClass::method_getLineNumberOf
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSStringParser *	pObjThis = ESLTypeCast<RSStringParser>( pThis ) ;
	if ( pObjThis == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser.getLineNumberOf 関数の this が StringParser ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	size_t	iLineIndex ;
	size_t	nLine = pObjThis->m_parser.GetLineNumberOf
						( (size_t) arg.IntAt(0), &iLineIndex ) ;
	RSObject *	pGetLineIndex = arg.ObjectAt( 1 ) ;
	if ( pGetLineIndex != NULL )
	{
		pGetLineIndex->SetElementIntegerAt( context, 0, iLineIndex ) ;
	}
	return	context.new_Integer( nLine ) ;
}

// char currentCharacter()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParserClass::method_currentCharacter
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSStringParser *	pObjThis = ESLTypeCast<RSStringParser>( pThis ) ;
	if ( pObjThis == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser.currentCharacter 関数の this が StringParser ではありません" ) ;
		return	NULL ;
	}
	return	context.new_Integer( pObjThis->m_parser.CurrentCharacter() ) ;
}

// char offsetAt( int iOffset )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParserClass::method_offsetAt
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSStringParser *	pObjThis = ESLTypeCast<RSStringParser>( pThis ) ;
	if ( pObjThis == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser.currentCharacter 関数の this が StringParser ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Integer
				( pObjThis->m_parser.OffsetAt( (size_t) arg.IntAt(0) ) ) ;
}

// static boolean isCharacterSpace( char c )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParserClass::method_isCharacterSpace
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Boolean
		( SStringParser::IsCharacterSpace( (wchar_t) arg.IntAt(0) ) ) ;
}

// static boolean isPunctuation( char c )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParserClass::method_isPunctuation
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Boolean
		( SStringParser::IsPunctuation( (wchar_t) arg.IntAt(0) ) ) ;
}

// static boolean isSpecialMark( char c )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParserClass::method_isSpecialMark
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Boolean
		( SStringParser::IsSpecialMark( (wchar_t) arg.IntAt(0) ) ) ;
}

// boolean passSpace()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParserClass::method_passSpace
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSStringParser *	pObjThis = ESLTypeCast<RSStringParser>( pThis ) ;
	if ( pObjThis == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser.passSpace 関数の this が StringParser ではありません" ) ;
		return	NULL ;
	}
	return	context.new_Boolean( pObjThis->m_parser.PassSpace() ) ;
}

// int seekToNextLine()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParserClass::method_seekToNextLine
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSStringParser *	pObjThis = ESLTypeCast<RSStringParser>( pThis ) ;
	if ( pObjThis == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser.seekToNextLine 関数の this が StringParser ではありません" ) ;
		return	NULL ;
	}
	return	context.new_Integer( pObjThis->m_parser.SeekToNextLine() ) ;
}

// void passString()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParserClass::method_passString
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSStringParser *	pObjThis = ESLTypeCast<RSStringParser>( pThis ) ;
	if ( pObjThis == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser.passString 関数の this が StringParser ではありません" ) ;
		return	NULL ;
	}
	pObjThis->m_parser.PassString() ;
	return	NULL ;
}

// void passEnclosedString( char chCloser, int nCtrlFlags = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParserClass::method_passEnclosedString
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSStringParser *	pObjThis = ESLTypeCast<RSStringParser>( pThis ) ;
	if ( pObjThis == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser.passEnclosedString 関数の this が StringParser ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pObjThis->m_parser.PassEnclosedString
					( (wchar_t) arg.IntAt(0), arg.IntAt(1) ) ;
	return	NULL ;
}

// int passToken()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParserClass::method_passToken
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSStringParser *	pObjThis = ESLTypeCast<RSStringParser>( pThis ) ;
	if ( pObjThis == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser.passToken 関数の this が StringParser ではありません" ) ;
		return	NULL ;
	}
	return	context.new_Integer( pObjThis->m_parser.PassToken() ) ;
}

// void passExpressionTerm( int nCtrlFlags = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParserClass::method_passExpressionTerm
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSStringParser *	pObjThis = ESLTypeCast<RSStringParser>( pThis ) ;
	if ( pObjThis == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser.passExpressionTerm 関数の this が StringParser ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pObjThis->m_parser.PassExpressionTerm( arg.IntAt(0) ) ;
	return	NULL ;
}

// void passExpression( String strCloses, int nCtrlFlags = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParserClass::method_passExpression
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSStringParser *	pObjThis = ESLTypeCast<RSStringParser>( pThis ) ;
	if ( pObjThis == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser.passExpression 関数の this が StringParser ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	pObjThis->m_parser.PassExpression( arg.StringAt(0), arg.IntAt(1) ) ;
	return	NULL ;
}

// char getCharacter()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParserClass::method_getCharacter
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSStringParser *	pObjThis = ESLTypeCast<RSStringParser>( pThis ) ;
	if ( pObjThis == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser.getCharacter 関数の this が StringParser ではありません" ) ;
		return	NULL ;
	}
	return	context.new_Integer( pObjThis->m_parser.GetCharacter() ) ;
}

// String getLine( int[] pRetCode = null )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParserClass::method_getLine
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSStringParser *	pObjThis = ESLTypeCast<RSStringParser>( pThis ) ;
	if ( pObjThis == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser.getLine 関数の this が StringParser ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	uint32_t	nRetCode ;
	SString		strLine = pObjThis->m_parser.GetLine( &nRetCode ) ;
	RSObject *	pRetCode = arg.ObjectAt( 0 ) ;
	if ( pRetCode != NULL )
	{
		pRetCode->SetElementIntegerAt( context, 0, nRetCode ) ;
	}
	return	context.new_String( strLine ) ;
}

// String getString()
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParserClass::method_getString
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSStringParser *	pObjThis = ESLTypeCast<RSStringParser>( pThis ) ;
	if ( pObjThis == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser.getString 関数の this が StringParser ではありません" ) ;
		return	NULL ;
	}
	return	context.new_String( pObjThis->m_parser.GetString() ) ;
}

// String getEnclosedString
//		( char chCloser, int nCtrlFlags = 0, char[] pGetClosed = null )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParserClass::method_getEnclosedString
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSStringParser *	pObjThis = ESLTypeCast<RSStringParser>( pThis ) ;
	if ( pObjThis == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser.getEnclosedString 関数の this が StringParser ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	wchar_t		wchClosed ;
	SString		strResult =
		pObjThis->m_parser.GetEnclosedString
				( (wchar_t) arg.IntAt(0), arg.IntAt(1), &wchClosed ) ;
	RSObject *	pGetClosed = arg.ObjectAt( 2 ) ;
	if ( pGetClosed != NULL )
	{
		pGetClosed->SetElementIntegerAt( context, 0, wchClosed ) ;
	}
	return	context.new_String( strResult ) ;
}

// String getToken( int[] pGetType = null )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParserClass::method_getToken
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSStringParser *	pObjThis = ESLTypeCast<RSStringParser>( pThis ) ;
	if ( pObjThis == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser.getToken 関数の this が StringParser ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SStringParser::TokenType	type ;
	SString		strToken = pObjThis->m_parser.GetToken( &type ) ;
	RSObject *	pGetType = arg.ObjectAt( 0 ) ;
	if ( pGetType != NULL )
	{
		pGetType->SetElementIntegerAt( context, 0, type ) ;
	}
	return	context.new_String( strToken ) ;
}

// String getStringTerm( int nCtrlFlags = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParserClass::method_getStringTerm
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSStringParser *	pObjThis = ESLTypeCast<RSStringParser>( pThis ) ;
	if ( pObjThis == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser.getStringTerm 関数の this が StringParser ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_String
				( pObjThis->m_parser.GetStringTerm( arg.IntAt(0) ) ) ;
}

// String getExpressionTerm( int nCtrlFlags = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParserClass::method_getExpressionTerm
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSStringParser *	pObjThis = ESLTypeCast<RSStringParser>( pThis ) ;
	if ( pObjThis == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser.getExpressionTerm 関数の this が StringParser ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_String
				( pObjThis->m_parser.GetExpressionTerm( arg.IntAt(0) ) ) ;
}

// String getExpression
//	( String strCloses, int nCtrlFlags = 0, char[] pGetClosed = null )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParserClass::method_getExpression
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSStringParser *	pObjThis = ESLTypeCast<RSStringParser>( pThis ) ;
	if ( pObjThis == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser.getExpressionTerm 関数の this が StringParser ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	wchar_t	wchClosed ;
	SString	strExpr =
		pObjThis->m_parser.GetExpression
			( arg.StringAt(0), arg.IntAt(1), &wchClosed ) ;
	RSObject *	pGetClosed = arg.ObjectAt( 2 ) ;
	if ( pGetClosed != NULL )
	{
		pGetClosed->SetElementIntegerAt( context, 0, wchClosed ) ;
	}
	return	context.new_String( strExpr ) ;
}

// char hasToComeChar( String strNext )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParserClass::method_hasToComeChar
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSStringParser *	pObjThis = ESLTypeCast<RSStringParser>( pThis ) ;
	if ( pObjThis == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser.hasToComeChar 関数の this が StringParser ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Integer
				( pObjThis->m_parser.HasToComeChar( arg.StringAt(0) ) ) ;
}

// boolean hasToComeString( String strNext )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParserClass::method_hasToComeString
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSStringParser *	pObjThis = ESLTypeCast<RSStringParser>( pThis ) ;
	if ( pObjThis == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser.hasToComeString 関数の this が StringParser ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Boolean
				( pObjThis->m_parser.HasToComeString( arg.StringAt(0) ) ) ;
}

// boolean hasToComeNoCaseString( String strNext )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParserClass::method_hasToComeNoCaseString
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSStringParser *	pObjThis = ESLTypeCast<RSStringParser>( pThis ) ;
	if ( pObjThis == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser.hasToComeNoCaseString 関数の this が StringParser ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Boolean
				( pObjThis->m_parser.HasToComeNoCaseString( arg.StringAt(0) ) ) ;
}

// boolean hasToComeToken( String strNext )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParserClass::method_hasToComeToken
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSStringParser *	pObjThis = ESLTypeCast<RSStringParser>( pThis ) ;
	if ( pObjThis == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser.hasToComeToken 関数の this が StringParser ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Boolean
				( pObjThis->m_parser.HasToComeToken( arg.StringAt(0) ) ) ;
}

// int isNextNumber( int nCtrlFlags = 0 )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParserClass::method_isNextNumber
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSStringParser *	pObjThis = ESLTypeCast<RSStringParser>( pThis ) ;
	if ( pObjThis == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser.isNextNumber 関数の this が StringParser ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Integer
				( pObjThis->m_parser.IsNextNumber( arg.IntAt(0) ) != SStringParser::numberInvalid ) ;
}

// long nextInteger( int type = StringParser.numberInteger )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParserClass::method_nextInteger
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSStringParser *	pObjThis = ESLTypeCast<RSStringParser>( pThis ) ;
	if ( pObjThis == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser.nextInteger 関数の this が StringParser ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Integer
		( pObjThis->m_parser.NextInteger
			( arg.IntAt(0, SStringParser::numberInteger) ) ) ;
}

// double nextRealNumber( int type = StringParser.numberDefault )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParserClass::method_nextRealNumber
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSStringParser *	pObjThis = ESLTypeCast<RSStringParser>( pThis ) ;
	if ( pObjThis == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser.nextRealNumber 関数の this が StringParser ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	return	context.new_Number
		( pObjThis->m_parser.NextRealNumber
			( arg.IntAt(0, SStringParser::numberDefault) ) ) ;
}

// int parseIntegerArray
//		( long[] pNumbers, int nCount,
//			int nCtrlFlags = 0, String strSeparator = null )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParserClass::method_parseIntegerArray
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSStringParser *	pObjThis = ESLTypeCast<RSStringParser>( pThis ) ;
	if ( pObjThis == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser.parseIntegerArray 関数の this が StringParser ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SArray<int64_t>	bufNumbers ;
	const size_t	nCount = (size_t) arg.IntAt(1) ;
	int64_t *		pNumbers = bufNumbers.GetArray( nCount ) ;
	size_t			nResult =
		pObjThis->m_parser.ParseIntegerArray
			( pNumbers, nCount, arg.IntAt(2), arg.StringAt(3) ) ;
	RSObject *		pObjNumbers = arg.ObjectAt( 0 ) ;
	if ( pObjNumbers == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser.parseIntegerArray 関数の引数が null です", L"NullPointerException" ) ;
		return	NULL ;
	}
	for ( size_t i = 0; i < nResult; i ++ )
	{
		pObjNumbers->SetElementIntegerAt( context, (int) i, pNumbers[i] ) ;
	}
	bufNumbers.FinishArray() ;
	return	context.new_Integer( nResult ) ;
}

// int parseHexIntegerArray
//		( long[] pNumbers, int nCount, String strSeparator = null )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParserClass::method_parseHexIntegerArray
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSStringParser *	pObjThis = ESLTypeCast<RSStringParser>( pThis ) ;
	if ( pObjThis == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser.parseHexIntegerArray 関数の this が StringParser ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SArray<int64_t>	bufNumbers ;
	const size_t	nCount = (size_t) arg.IntAt(1) ;
	int64_t *		pNumbers = bufNumbers.GetArray( nCount ) ;
	size_t			nResult =
						pObjThis->m_parser.ParseHexIntegerArray
								( pNumbers, nCount, arg.StringAt(2) ) ;
	RSObject *		pObjNumbers = arg.ObjectAt( 0 ) ;
	if ( pObjNumbers == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser.parseHexIntegerArray 関数の引数が null です", L"NullPointerException" ) ;
		return	NULL ;
	}
	for ( size_t i = 0; i < nResult; i ++ )
	{
		pObjNumbers->SetElementIntegerAt( context, (int) i, pNumbers[i] ) ;
	}
	bufNumbers.FinishArray() ;
	return	context.new_Integer( nResult ) ;
}

// int parseNumberArray
//		( double[] pNumbers, int nCount,
//			int nCtrlFlags = 0, String strSeparator = null )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParserClass::method_parseNumberArray
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSStringParser *	pObjThis = ESLTypeCast<RSStringParser>( pThis ) ;
	if ( pObjThis == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser.parseNumberArray 関数の this が StringParser ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SArray<double>	bufNumbers ;
	const size_t	nCount = (size_t) arg.IntAt(1) ;
	double *		pNumbers = bufNumbers.GetArray( nCount ) ;
	size_t			nResult =
		pObjThis->m_parser.ParseNumberArray
			( pNumbers, nCount, arg.IntAt(2), arg.StringAt(3) ) ;
	RSObject *		pObjNumbers = arg.ObjectAt( 0 ) ;
	if ( pObjNumbers == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser.parseNumberArray 関数の引数が null です", L"NullPointerException" ) ;
		return	NULL ;
	}
	for ( size_t i = 0; i < nResult; i ++ )
	{
		pObjNumbers->SetElementNumberAt( context, (int) i, pNumbers[i] ) ;
	}
	bufNumbers.FinishArray() ;
	return	context.new_Integer( nResult ) ;
}

// int parseCommaSeparatedValues( String[] aValues )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSStringParserClass::method_parseCommaSeparatedValues
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSStringParser *	pObjThis = ESLTypeCast<RSStringParser>( pThis ) ;
	if ( pObjThis == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser.parseCommaSeparatedValues 関数の this が StringParser ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SObjectArray<SString>	aValues ;
	SError		err = pObjThis->m_parser.ParseCommaSeparatedValues( aValues ) ;
	RSObject *	pObjStrings = arg.ObjectAt( 0 ) ;
	if ( pObjStrings == NULL )
	{
		context.ThrowExceptionError
			( L"StringParser.parseCommaSeparatedValues 関数の引数が null です", L"NullPointerException" ) ;
		return	NULL ;
	}
	for ( size_t i = 0; i < aValues.GetLength(); i ++ )
	{
		SString *		pstrValue = aValues.GetAt( i ) ;
		const wchar_t *	pwszValue = L"" ;
		if ( pstrValue != NULL )
		{
			pwszValue = *pstrValue ;
		}
		pObjStrings->SetElementStringAt( context, (int) i, pwszValue ) ;
	}
	return	context.new_Integer( err ) ;
}



//////////////////////////////////////////////////////////////////////////////
// 構文マッチャー
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSUsageMatcher, RSObject )

// 消滅関数
//////////////////////////////////////////////////////////////////////////////
RSUsageMatcher::~RSUsageMatcher( void )
{
}

// 書式設定
//////////////////////////////////////////////////////////////////////////////
SError RSUsageMatcher::SetUsage
	( const wchar_t * pwszUsage, SParserErrorInterface& perr )
{
	SError	err = m_matcher.ParseUsage( pwszUsage, perr ) ;
	if ( !err )
	{
		m_usage = pwszUsage ;
	}
	return	err ;
}

// 構文の解釈
//////////////////////////////////////////////////////////////////////////////
SSystem::SError RSUsageMatcher::IsMatchedWith
	( SSystem::SStringParser & sparsTarget,
		SSystem::SObjectArray<SSystem::SString> * parrParam,
		SSystem::SParserErrorInterface & perr ) const
{
	return	m_matcher.IsMatchedWith( sparsTarget, parrParam, perr ) ;
}

// 書式検索
//////////////////////////////////////////////////////////////////////////////
ssize_t RSUsageMatcher::FindMatchedWith
	( SSystem::SStringParser & sparsTarget,
		SSystem::SUsageMatcher::UsageType utWildCardType ) const
{
	return	m_matcher.FindMatchedWith( sparsTarget, utWildCardType ) ;
}

// 型名
//////////////////////////////////////////////////////////////////////////////
const wchar_t * RSUsageMatcher::GetTypeName( void ) const
{
	return	L"UsageMatcher" ;
}

// 文字列型か？
//////////////////////////////////////////////////////////////////////////////
bool RSUsageMatcher::IsStringType( void ) const
{
	return	true ;
}

// 文字列変換
//////////////////////////////////////////////////////////////////////////////
bool RSUsageMatcher::AsString( SSystem::SString& strValue ) const
{
	strValue = m_usage ;
	return	true ;
}

// デバッグ用ダンプ文字列
//////////////////////////////////////////////////////////////////////////////
void RSUsageMatcher::ToDebugDump
	( SSystem::SFileInterface& dump,
			size_t nPtrNest, const wchar_t * pwszIndent )
{
	SString	strDump = L"usage \"" ;
	strDump += m_usage ;
	strDump += L"\"" ;
	dump.WriteEncodedString( strDump ) ;
}

// 内部リソース解放
//////////////////////////////////////////////////////////////////////////////
void RSUsageMatcher::DisposeObject( RSContext& context )
{
	m_matcher.ReleaseUsage() ;
	m_usage.FreeArray() ;
	RSObject::DisposeObject( context ) ;
}

// 複製（実体も可能な限り複製）
//////////////////////////////////////////////////////////////////////////////
RSObject * RSUsageMatcher::CloneObject( RSContext& context ) const
{
	RSUsageMatcher *	pObj = new RSUsageMatcher( m_pClass ) ;
	if ( !m_usage.IsEmpty() )
	{
		SParserErrorInterface	perr ;
		pObj->SetUsage( m_usage, perr ) ;
	}
	return	pObj ;
}

// 代入演算子
//////////////////////////////////////////////////////////////////////////////
RSObject * RSUsageMatcher::OperatorMove( RSContext& context, RSObject * pObj )
{
	SString	strValue ;
	if ( pObj->AsString( strValue ) )
	{
		SParserErrorLogger	perrLog ;
		if ( SetUsage( strValue, perrLog ) )
		{
			SParserErrorLogger::ErrorLog *
						pLog = perrLog.GetErrorLogAt( 0 ) ;
			SString	strErr ;
			if ( pLog != NULL )
			{
				strErr = pLog->m_strError ;
			}
			context.ThrowExceptionError( strErr ) ;
		}
		AddRef() ;
		return	this ;
	}
	context.ThrowExceptionError( L"文字列へ変換できません" ) ;
	return	NULL ;
}



//////////////////////////////////////////////////////////////////////////////
// 構文マッチャー型
//////////////////////////////////////////////////////////////////////////////

// クラス情報
//////////////////////////////////////////////////////////////////////////////
ESL_IMPLEMENT_CLASS_INFO( Rosetta::RSUsageMatcherClass, RSClass )

// 構築関数
//////////////////////////////////////////////////////////////////////////////
RSUsageMatcherClass::RSUsageMatcherClass
		( RSClass * pClass, const wchar_t * pwszClassName )
	: RSClass( pClass, pwszClassName )
{
}

// クラス固有仮想関数オーバーライドと
// クラス static 変数のオーバーロード
//////////////////////////////////////////////////////////////////////////////
void RSUsageMatcherClass::OverrideVirtuals( RSContext& context )
{
	context.ReleaseObjectRef( m_pPrototype ) ;
	m_pPrototype = new RSUsageMatcher( this ) ;
	//
	CreateMemberIntegerAs
		( context, L"utWildCardToken", SUsageMatcher::utWildCardToken, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"utWildCardUsage", SUsageMatcher::utWildCardUsage, modifierConst ) ;
	CreateMemberIntegerAs
		( context, L"utWildCardExpression", SUsageMatcher::utWildCardExpression, modifierConst ) ;
	//
	SParserErrorTracer	perr ;
	AddVirtualDescriptiveAs
		( context, perr, L"<init>", NULL, L"String usage",
				NULL, &RSUsageMatcherClass::method_init, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"setUsage", NULL, L"String usage",
				NULL, &RSUsageMatcherClass::method_setUsage, NULL ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"parse",
			L"String", L"String text, String[] aParams = null",
				NULL, &RSUsageMatcherClass::method_parse,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"parseNext",
			L"String", L"StringParser parser, String[] aParams = null",
				NULL, &RSUsageMatcherClass::method_parseNext,
				NULL, RSFunctionPrototype::flagConstant ) ;
	AddVirtualDescriptiveAs
		( context, perr, L"find",
			L"int", L"StringParser parser, "
					L"int typeWildCard = UsageMatcher.utWildCardToken",
				NULL, &RSUsageMatcherClass::method_find,
				NULL, RSFunctionPrototype::flagConstant ) ;
}

// void <init>( String usage )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSUsageMatcherClass::method_init
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSUsageMatcher *	pObj = ESLTypeCast<RSUsageMatcher>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"UsageMatcher 構築関数の this が UsageMatcher ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SParserErrorTracer	perrTrace ;
	pObj->SetUsage( arg.StringAt( 0 ), perrTrace ) ;
	return	NULL ;
}

// void setUsage( String usage )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSUsageMatcherClass::method_setUsage
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSUsageMatcher *	pObj = ESLTypeCast<RSUsageMatcher>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"UsageMatcher.setUsage 関数の this が UsageMatcher ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	SParserErrorLogger	perrLog ;
	if ( pObj->SetUsage( arg.StringAt( 0 ), perrLog ) )
	{
		SString	strErr ;
		SParserErrorLogger::ErrorLog *	pLog = perrLog.GetErrorLogAt( 0 ) ;
		if ( pLog != NULL )
		{
			strErr = pLog->m_strError ;
		}
		context.ThrowExceptionError( strErr ) ;
	}
	return	NULL ;
}

// String parse( String text, String[] aParams = null )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSUsageMatcherClass::method_parse
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSUsageMatcher *	pObj = ESLTypeCast<RSUsageMatcher>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"UsageMatcher.parse 関数の this が UsageMatcher ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList		arg( ppArg, count ) ;
	SObjectArray<SString>	arrParam ;
	SParserErrorLogger		perrLog ;
	SStringParser			sparsTarget( arg.StringAt( 0 ) ) ;
	if ( pObj->IsMatchedWith( sparsTarget, &arrParam, perrLog ) )
	{
		SString	strErr ;
		SParserErrorLogger::ErrorLog *	pLog = perrLog.GetErrorLogAt( 0 ) ;
		if ( pLog != NULL )
		{
			strErr = pLog->m_strError ;
		}
		return	context.new_String( strErr ) ;
	}
	RSObject *	pObjParam = arg.ObjectAt( 1 ) ;
	if ( pObjParam != NULL )
	{
		for ( size_t i = 0; i < arrParam.GetLength(); i ++ )
		{
			SString *	pstrParam = arrParam.GetAt( i ) ;
			if ( pstrParam != NULL )
			{
				pObjParam->SetElementStringAt( context, (int) i, *pstrParam ) ;
			}
			else
			{
				context.ReleaseObjectRef
					( pObjParam->SetElementAt( context, (int) i, NULL ) ) ;
			}
		}
	}
	return	NULL ;
}

// String parseNext( StringParser parser, String[] aParams = null )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSUsageMatcherClass::method_parseNext
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSUsageMatcher *	pObj = ESLTypeCast<RSUsageMatcher>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"UsageMatcher.parse 関数の this が UsageMatcher ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList		arg( ppArg, count ) ;
	SObjectArray<SString>	arrParam ;
	SParserErrorLogger		perrLog ;
	RSStringParser *		pObjTarget = ESLTypeCast<RSStringParser>( arg.ObjectAt( 0 ) ) ;
	if ( pObjTarget == NULL )
	{
		context.ThrowExceptionError
			( L"UsageMatcher.parseNext 関数の引数が null です", L"NullPointerException" ) ;
		return	NULL ;
	}
	if ( pObj->IsMatchedWith( pObjTarget->m_parser, &arrParam, perrLog ) )
	{
		SString	strErr ;
		SParserErrorLogger::ErrorLog *	pLog = perrLog.GetErrorLogAt( 0 ) ;
		if ( pLog != NULL )
		{
			strErr = pLog->m_strError ;
		}
		return	context.new_String( strErr ) ;
	}
	RSObject *	pObjParam = arg.ObjectAt( 1 ) ;
	if ( pObjParam == NULL )
	{
		context.ThrowExceptionError
			( L"UsageMatcher.parseNext 関数の引数が null です", L"NullPointerException" ) ;
		return	NULL ;
	}
	for ( size_t i = 0; i < arrParam.GetLength(); i ++ )
	{
		SString *	pstrParam = arrParam.GetAt( i ) ;
		if ( pstrParam != NULL )
		{
			pObjParam->SetElementStringAt( context, (int) i, *pstrParam ) ;
		}
		else
		{
			context.ReleaseObjectRef
				( pObjParam->SetElementAt( context, (int) i, NULL ) ) ;
		}
	}
	return	NULL ;
}

// int find( StringParser parser, int typeWildCard = UsageMatcher.utWildCardToken )
//////////////////////////////////////////////////////////////////////////////
RSObject * RSUsageMatcherClass::method_find
	( RSContext& context, void * pInstace,
		RSObject* pThis, RSObject** ppArg, size_t count )
{
	RSUsageMatcher *	pObj = ESLTypeCast<RSUsageMatcher>( pThis ) ;
	if ( pObj == NULL )
	{
		context.ThrowExceptionError
			( L"UsageMatcher.find 関数の this が UsageMatcher ではありません" ) ;
		return	NULL ;
	}
	RSContext::SArgList	arg( ppArg, count ) ;
	RSStringParser *	pObjTarget =
							ESLTypeCast<RSStringParser>( arg.ObjectAt( 0 ) ) ;
	if ( pObjTarget == NULL )
	{
		context.ThrowExceptionError
			( L"UsageMatcher.find 関数の引数が null です", L"NullPointerException" ) ;
		return	NULL ;
	}
	ssize_t	iFind =
		pObj->FindMatchedWith
			( pObjTarget->m_parser,
				(SUsageMatcher::UsageType) arg.IntAt(1) ) ;
	return	context.new_Integer( iFind ) ;
}


